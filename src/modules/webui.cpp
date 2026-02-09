#include "modules/webui.h"
#include "core/ui.h"
#include <WebServer.h>
#include <Preferences.h>
#include <vector>
#include "core/log.h"
#include "modules/config.h"
#include "modules/backup.h"
#include "modules/wifi.h"

static WebServer server(80);
static bool webuiRunning = false;
static bool webuiApMode = false;
static String webuiUrl = "";
static Preferences webuiPrefs;
static const char* WEBUI_NS = "webui";
static const char* WEBUI_KEY_EN = "enabled";

static String fmtBytes(size_t b){
  float mb = b / (1024.0f * 1024.0f);
  char buf[16];
  snprintf(buf, sizeof(buf), "%.2fMB", mb);
  return String(buf);
}

static fs::FS* getFs(const String& fs){
  if (fs == "sd"){
    if (!sdReady) sdReady = SD.begin(configGetSdCsPin());
    if (!sdReady) return nullptr;
    return &SD;
  }
  return &SPIFFS;
}

static String normPath(const String &p){
  String s = p;
  s.replace("\\", "/");
  while (s.indexOf("//") >= 0) s.replace("//", "/");
  if (!s.startsWith("/")) s = "/" + s;
  s.replace("..", "");
  return s;
}

static bool mkdirs(fs::FS &fs, const String &path){
  if (path.length() == 0 || path == "/") return true;
  if (fs.exists(path)) return true;
  int pos = 1;
  while (true){
    pos = path.indexOf('/', pos);
    String sub = (pos < 0) ? path : path.substring(0, pos);
    if (!fs.exists(sub)) fs.mkdir(sub);
    if (pos < 0) break;
    pos++;
  }
  return fs.exists(path);
}

static size_t dirSize(fs::FS &fs, const String &path){
  File dir = fs.open(path);
  if (!dir || !dir.isDirectory()) return 0;
  size_t sum = 0;
  File f = dir.openNextFile();
  while (f){
    if (f.isDirectory()){
      sum += dirSize(fs, String(f.name()));
    } else {
      sum += f.size();
    }
    f = dir.openNextFile();
  }
  return sum;
}

static bool deletePath(fs::FS &fs, const String &path){
  File f = fs.open(path);
  if (!f) return false;
  if (!f.isDirectory()){
    f.close();
    return fs.remove(path);
  }
  File c = f.openNextFile();
  while (c){
    String name = String(c.name());
    if (c.isDirectory()){
      deletePath(fs, name);
    } else {
      fs.remove(name);
    }
    c = f.openNextFile();
  }
  f.close();
  return fs.rmdir(path);
}

static const char* WEBUI_HTML = R"HTML(
<!doctype html>
<html lang="en">
<head>
<meta charset="utf-8" />
<meta name="viewport" content="width=device-width, initial-scale=1" />
<title>IDK WebUI</title>
<style>
  :root { --bg:#0b0b0b; --fg:#f5f5f5; --muted:#a0a0a0; --blue:#2d6cff; }
  * { box-sizing: border-box; }
  body { margin:0; font-family: ui-monospace, SFMono-Regular, Menlo, Monaco, Consolas, "Liberation Mono", "Courier New", monospace; background: #fff; color:#000; }
  .wrap { max-width: 980px; margin: 0 auto; padding: 12px; }
  .top { display:flex; flex-wrap:wrap; gap:8px; background:#000; color:#fff; border:2px solid #000; padding:8px; align-items:center; justify-content:space-between; }
  .tabs { display:flex; gap:6px; flex-wrap:wrap; }
  .btn { padding:6px 10px; border:1px solid #666; background:#000; color:#fff; cursor:pointer; }
  .btn.active { background: var(--blue); border-color:#fff; }
  .bar { border-left:2px solid #000; border-right:2px solid #000; border-bottom:2px solid #000; }
  .bar .row { padding:6px 10px; font-size:14px; }
  .bar .row.sd { background: var(--blue); color:#fff; }
  .bar .row.fs { background: #fff; color:#000; border-top:2px solid #000; }
  .panel { border:2px solid #000; border-top:0; background:#000; color:#fff; min-height: 380px; }
  .path { display:flex; justify-content:space-between; align-items:center; padding:8px 10px; border-bottom:1px solid #fff; background:#111; font-size:14px; }
  .icons { display:flex; gap:6px; }
  .icon { padding:2px 6px; border:1px solid #fff; }
  .content { padding:10px; color:#ddd; font-size:14px; }
  .row { display:flex; justify-content:space-between; border-bottom:1px solid #222; padding:6px 4px; }
  .row:hover { background:#111; }
  .muted { color:#aaa; }
  .toolbar { display:flex; gap:6px; flex-wrap:wrap; margin-bottom:8px; }
  .input { background:#111; color:#fff; border:1px solid #555; padding:4px 6px; font-family:inherit; }
  .select { background:#111; color:#fff; border:1px solid #555; padding:4px 6px; }
  .danger { border-color:#a33; }
  @media (max-width: 600px){ .content{height:260px;} }
</style>
</head>
<body>
  <div class="wrap">
    <div class="top">
      <div class="tabs">
        <button class="btn" data-tab="dev">idk [dev]</button>
        <button class="btn" data-tab="serial">Serial Cmd</button>
        <button class="btn active" data-tab="nav">Navigator</button>
      </div>
      <div class="tabs">
        <button class="btn">Settings</button>
        <button class="btn">Log Out</button>
      </div>
    </div>

    <div class="bar">
      <div class="row sd">SDCard [-- / --]</div>
      <div class="row fs">LittleFS [-- / --]</div>
    </div>

    <div class="panel">
      <div class="path">
        <span id="pathLabel">/</span>
        <div class="icons">
          <span class="icon" id="btnRefresh">⟳</span>
          <span class="icon" id="btnUp">⬆</span>
          <span class="icon" id="btnNew">＋</span>
        </div>
      </div>
      <div class="content">
        <div class="toolbar">
          <select id="fsSelect" class="select">
            <option value="spiffs">LittleFS</option>
            <option value="sd">SDCard</option>
          </select>
          <button class="btn" id="btnUpload">Upload</button>
          <button class="btn" id="btnMkdir">New Folder</button>
          <button class="btn" id="btnTouch">New File</button>
          <button class="btn danger" id="btnDelete">Delete</button>
          <button class="btn" id="btnRename">Rename</button>
          <button class="btn" id="btnBackup">Backup</button>
          <button class="btn" id="btnRestore">Restore</button>
                <button class="btn" id="btnKnown">Known WiFi</button>
          <button class="btn" id="btnWebui">WebUI: On</button>
        </div>
        <div class="muted" id="memInfo">--</div>
        <div id="fileList"></div>
      </div>
    </div>
  </div>
<script>
  const buttons = document.querySelectorAll('.btn[data-tab]');
  buttons.forEach(b=>b.addEventListener('click',()=>{
    buttons.forEach(x=>x.classList.remove('active'));
    b.classList.add('active');
  }));

  let currentFs = 'spiffs';
  let currentPath = '/';
  let selected = null;

  const qs = (s)=>document.querySelector(s);
  const fileList = qs('#fileList');
  const memInfo = qs('#memInfo');
  const pathLabel = qs('#pathLabel');

  async function api(path, opts){
    const res = await fetch(path, opts);
    return await res.json();
  }

  function render(items){
    fileList.innerHTML = '';
    items.forEach(it=>{
      const row = document.createElement('div');
      row.className = 'row';
      row.textContent = '';
      const left = document.createElement('span');
      left.textContent = (it.isDir ? '📁 ' : '📄 ') + it.name;
      const right = document.createElement('span');
      right.textContent = it.size;
      right.className = 'muted';
      row.appendChild(left);
      row.appendChild(right);
      row.onclick = ()=>{ selected = it; };
      row.ondblclick = ()=>{ if (it.isDir){ currentPath = it.path; loadList(); } };
      fileList.appendChild(row);
    });
    pathLabel.textContent = currentPath;
  }

  async function updateInfo(){
    const data = await api('/api/info');
    memInfo.textContent = `FS ${data.fs} | SD ${data.sd}`;
    qs('#btnWebui').textContent = `WebUI: ${data.web ? 'On' : 'Off'}`;
  }

  async function loadList(){
    const data = await api(`/api/list?fs=${currentFs}&path=${encodeURIComponent(currentPath)}`);
    render(data.items);
    updateInfo();
  }

  qs('#fsSelect').onchange = (e)=>{ currentFs = e.target.value; currentPath = '/'; loadList(); };
  qs('#btnRefresh').onclick = ()=>loadList();
  qs('#btnUp').onclick = ()=>{
    if (currentPath === '/') return;
    const parts = currentPath.split('/').filter(Boolean);
    parts.pop();
    currentPath = '/' + parts.join('/');
    if (currentPath === '') currentPath = '/';
    loadList();
  };

  qs('#btnUpload').onclick = async ()=>{
    const input = document.createElement('input');
    input.type = 'file';
    input.multiple = true;
    input.webkitdirectory = true;
    input.onchange = async ()=>{
      for (const f of input.files){
        const form = new FormData();
        form.append('file', f, f.webkitRelativePath || f.name);
        await fetch(`/api/upload?fs=${currentFs}&path=${encodeURIComponent(currentPath)}`, { method:'POST', body: form });
      }
      loadList();
    };
    input.click();
  };

  qs('#btnMkdir').onclick = async ()=>{
    const name = prompt('Folder name');
    if (!name) return;
    await api(`/api/mkdir?fs=${currentFs}&path=${encodeURIComponent(currentPath)}&name=${encodeURIComponent(name)}`, {method:'POST'});
    loadList();
  };
  qs('#btnTouch').onclick = async ()=>{
    const name = prompt('File name');
    if (!name) return;
    await api(`/api/touch?fs=${currentFs}&path=${encodeURIComponent(currentPath)}&name=${encodeURIComponent(name)}`, {method:'POST'});
    loadList();
  };
  qs('#btnRename').onclick = async ()=>{
    if (!selected) return alert('Select a file');
    const name = prompt('New name', selected.name);
    if (!name) return;
    await api(`/api/rename?fs=${currentFs}&path=${encodeURIComponent(currentPath)}&name=${encodeURIComponent(selected.name)}&to=${encodeURIComponent(name)}`, {method:'POST'});
    loadList();
  };
  qs('#btnDelete').onclick = async ()=>{
    if (!selected) return alert('Select a file');
    if (!confirm('Delete '+selected.name+'?')) return;
    await api(`/api/delete?fs=${currentFs}&path=${encodeURIComponent(currentPath)}&name=${encodeURIComponent(selected.name)}`, {method:'POST'});
    loadList();
  };

  qs('#btnWebui').onclick = async ()=>{
    const data = await api('/api/webui', {method:'POST', body: new URLSearchParams({enable: '0'})});
    qs('#btnWebui').textContent = `WebUI: ${data.enabled ? 'On' : 'Off'}`;
  };

  qs('#btnBackup').onclick = async ()=>{
    const res = await fetch('/api/backup');
    if (!res.ok) return alert('Backup failed');
    const txt = await res.text();
    const blob = new Blob([txt], {type: 'application/json'});
    const url = URL.createObjectURL(blob);
    const a = document.createElement('a');
    a.href = url;
    a.download = 'idk-backup.json';
    document.body.appendChild(a);
    a.click();
    a.remove();
    URL.revokeObjectURL(url);
  };

  qs('#btnRestore').onclick = async ()=>{
    const input = document.createElement('input');
    input.type = 'file';
    input.accept = '.json,application/json';
    input.onchange = async ()=>{
      const f = input.files[0];
      if (!f) return;
      const txt = await f.text();
      const res = await fetch('/api/backup/restore', {method:'POST', headers: {'Content-Type':'application/json'}, body: txt});
      const j = await res.json();
      if (j.ok) alert('Restore applied — reboot recommended'); else alert('Restore failed');
    };
    input.click();
  };

  qs('#btnKnown').onclick = async ()=>{
    const data = await api('/api/known');
    // render list in fileList area
    fileList.innerHTML = '';
    const title = document.createElement('div');
    title.className = 'row';
    title.innerHTML = '<strong>Known Networks</strong>';
    fileList.appendChild(title);
    data.forEach((it, idx)=>{
      const row = document.createElement('div');
      row.className = 'row';
      const left = document.createElement('span');
      left.textContent = it.ssid + (it.star ? ' *' : '');
      const right = document.createElement('span');
      right.className = 'muted';
      const btn = document.createElement('button');
      btn.textContent = 'Forget';
      btn.className = 'btn danger';
      btn.onclick = async ()=>{
        if (!confirm('Forget '+it.ssid+'?')) return;
        const res = await fetch('/api/known/forget', {method:'POST', body: new URLSearchParams({idx: idx})});
        const j = await res.json();
        if (j.ok) { alert('Forgot'); qs('#btnKnown').click(); } else alert('Failed');
      };
      right.appendChild(btn);
      row.appendChild(left);
      row.appendChild(right);
      fileList.appendChild(row);
    });
  };

  loadList();
</script>
</body>
</html>
)HTML";

static void handleRoot(){
  server.send(200, "text/html", WEBUI_HTML);
}

static void handleList(){
  String fsName = server.arg("fs");
  String path = normPath(server.arg("path"));
  fs::FS* fs = getFs(fsName);
  if (!fs){
    LOGW("webui list: FS %s not ready", fsName.c_str());
    server.send(500, "application/json", "{\"items\":[],\"info\":\"FS not ready\"}");
    return;
  }
  File dir = fs->open(path);
  if (!dir || !dir.isDirectory()){
    server.send(400, "application/json", "{\"items\":[],\"info\":\"Bad path\"}");
    return;
  }
  String items = "[";
  File f = dir.openNextFile();
  bool first = true;
  while (f){
    String name = String(f.name());
    String itemPath = name;
    bool isDir = f.isDirectory();
    size_t size = isDir ? dirSize(*fs, itemPath) : f.size();
    if (!first) items += ",";
    first = false;
    items += "{\"name\":\"" + String(name.substring(name.lastIndexOf('/')+1)) + "\",";
    items += "\"path\":\"" + itemPath + "\",";
    items += "\"isDir\":" + String(isDir ? "true" : "false") + ",";
    items += "\"size\":\"" + fmtBytes(size) + "\"}";
    f = dir.openNextFile();
  }
  items += "]";
  size_t total = fsName == "sd" ? SD.totalBytes() : SPIFFS.totalBytes();
  size_t used  = fsName == "sd" ? SD.usedBytes() : SPIFFS.usedBytes();
  String info = (fsName == "sd" ? "SD " : "FS ");
  info += fmtBytes(used) + "/" + fmtBytes(total);
  String out = "{\"items\":" + items + ",\"info\":\"" + info + "\"}";
  server.send(200, "application/json", out);
}

static void handleInfo(){
  size_t fsTotal = SPIFFS.totalBytes();
  size_t fsUsed  = SPIFFS.usedBytes();
  bool sdOk = sdReady || SD.begin(configGetSdCsPin());
  size_t sdTotal = sdOk ? SD.totalBytes() : 0;
  size_t sdUsed  = sdOk ? SD.usedBytes() : 0;
  String out = "{";
  out += "\"fs\":\"" + fmtBytes(fsUsed) + "/" + fmtBytes(fsTotal) + "\",";
  out += "\"sd\":\"" + (sdOk ? (fmtBytes(sdUsed) + "/" + fmtBytes(sdTotal)) : String("N/A")) + "\",";
  out += "\"sdOk\":" + String(sdOk ? "true" : "false") + ",";
  out += "\"web\":" + String(webuiEnabled ? "true" : "false");
  out += "}";
  server.send(200, "application/json", out);
}

static void handleMkdir(){
  String fsName = server.arg("fs");
  fs::FS* fs = getFs(fsName);
  if (!fs){ server.send(500, "application/json", "{\"ok\":false}"); return; }
  String path = normPath(server.arg("path"));
  String name = server.arg("name");
  String full = normPath(path + "/" + name);
  bool ok = mkdirs(*fs, full);
  server.send(200, "application/json", String("{\"ok\":") + (ok?"true":"false") + "}");
}

static void handleTouch(){
  String fsName = server.arg("fs");
  fs::FS* fs = getFs(fsName);
  if (!fs){ server.send(500, "application/json", "{\"ok\":false}"); return; }
  String path = normPath(server.arg("path"));
  String name = server.arg("name");
  String full = normPath(path + "/" + name);
  File f = fs->open(full, "w");
  bool ok = (bool)f;
  if (f) f.close();
  server.send(200, "application/json", String("{\"ok\":") + (ok?"true":"false") + "}");
}

static void handleRename(){
  String fsName = server.arg("fs");
  fs::FS* fs = getFs(fsName);
  if (!fs){ server.send(500, "application/json", "{\"ok\":false}"); return; }
  String path = normPath(server.arg("path"));
  String name = server.arg("name");
  String to = server.arg("to");
  String fromPath = normPath(path + "/" + name);
  String toPath = normPath(path + "/" + to);
  bool ok = fs->rename(fromPath, toPath);
  server.send(200, "application/json", String("{\"ok\":") + (ok?"true":"false") + "}");
}

static void handleDelete(){
  String fsName = server.arg("fs");
  fs::FS* fs = getFs(fsName);
  if (!fs){ server.send(500, "application/json", "{\"ok\":false}"); return; }
  String path = normPath(server.arg("path"));
  String name = server.arg("name");
  String full = normPath(path + "/" + name);
  bool ok = deletePath(*fs, full);
  server.send(200, "application/json", String("{\"ok\":") + (ok?"true":"false") + "}");
}

static void handleUpload(){
  String fsName = server.arg("fs");
  fs::FS* fs = getFs(fsName);
  if (!fs) { LOGE("upload: FS %s not ready", fsName.c_str()); return; }
  String base = normPath(server.arg("path"));
  HTTPUpload& upload = server.upload();
  static File uploadFile;
  if (upload.status == UPLOAD_FILE_START){
    String filename = upload.filename;
    filename.replace("\\", "/");
    filename.replace("..", "");
    String full = normPath(base + "/" + filename);
    int slash = full.lastIndexOf('/');
    if (slash > 0){
      String dir = full.substring(0, slash);
      mkdirs(*fs, dir);
    }
    uploadFile = fs->open(full, "w");
    if (!uploadFile) LOGE("upload: open failed %s", full.c_str());
  } else if (upload.status == UPLOAD_FILE_WRITE){
    if (uploadFile) uploadFile.write(upload.buf, upload.currentSize);
    else LOGW("upload: write without open file");
  } else if (upload.status == UPLOAD_FILE_END){
    if (uploadFile) uploadFile.close();
  }
}

static void handleBackup(){
  String j = backupCreateJson();
  server.send(200, "application/json", j);
}

static void handleBackupRestore(){
  // expects application/json body
  String body = server.arg("plain");
  bool ok = backupRestoreFromJson(body);
  server.send(200, "application/json", String("{\"ok\":") + (ok?"true":"false") + "}");
}

static void handleKnownList(){
  Preferences p;
  p.begin("wifi", true);
  int known = p.getInt("known", 0);
  String out = "[";
  for (int i=0;i<known;i++){
    if (i) out += ",";
    String s = p.getString((String("s") + String(i)).c_str(), "");
    bool star = s.endsWith("*");
    String ss = s;
    ss.replace("\"", "\\\"");
    out += "{\"ssid\":\"" + ss + "\",\"star\":" + String(star?"true":"false") + "}";
  }
  p.end();
  out += "]";
  server.send(200, "application/json", out);
}

static void handleKnownForget(){
  String idxs = server.arg("idx");
  int idx = -1;
  if (idxs.length()) idx = atoi(idxs.c_str());
  if (idx < 0){
    // try form body
    String body = server.arg("plain");
    if (body.length()){
      // simple parse of idx
      int p = body.indexOf('=');
      if (p>0) idx = atoi(body.substring(p+1).c_str());
    }
  }
  if (idx < 0){ server.send(400, "application/json", "{\"ok\":false}"); return; }
  wifiForgetKnown(idx);
  server.send(200, "application/json", "{\"ok\":true}");
}

static void handleWebuiToggle(){
  String en = server.arg("enable");
  if (en.length()){
    bool on = (en == "1");
    webuiPersistEnabled(on);
    webuiEnabled = on;
    if (!on) webuiStop();
  }
  server.send(200, "application/json", String("{\"enabled\":") + (webuiEnabled?"true":"false") + "}");
}

void webuiStartSTA(){
  webuiStop();
  if (!webuiEnabled) return;
  if (WiFi.status() != WL_CONNECTED) return;
  WiFi.setSleep(false);
  webuiApMode = false;
  webuiUrl = String("http://") + WiFi.localIP().toString();
  server.on("/", handleRoot);
  server.on("/api/info", HTTP_GET, handleInfo);
  server.on("/api/list", HTTP_GET, handleList);
  server.on("/api/mkdir", HTTP_POST, handleMkdir);
  server.on("/api/touch", HTTP_POST, handleTouch);
  server.on("/api/rename", HTTP_POST, handleRename);
  server.on("/api/delete", HTTP_POST, handleDelete);
  server.on("/api/upload", HTTP_POST, [](){ server.send(200, "application/json", "{\"ok\":true}"); }, handleUpload);
  server.on("/api/webui", HTTP_POST, handleWebuiToggle);
  server.on("/api/backup", HTTP_GET, handleBackup);
  server.on("/api/backup/restore", HTTP_POST, handleBackupRestore);
  server.on("/api/known", HTTP_GET, handleKnownList);
  server.on("/api/known/forget", HTTP_POST, handleKnownForget);
  server.onNotFound(handleRoot);
  server.begin();
  webuiRunning = true;
  LOGI("WebUI STA started at %s", webuiUrl.c_str());
}

void webuiStartAP(){
  webuiStop();
  if (!webuiEnabled) return;
  webuiApMode = true;
  const char* ssid = "idk";
  const char* pass = "idk";
  WiFi.mode(WIFI_AP);
  WiFi.softAP(ssid, pass);
  IPAddress ip = WiFi.softAPIP();
  webuiUrl = String("http://") + ip.toString();
  server.on("/", handleRoot);
  server.on("/api/info", HTTP_GET, handleInfo);
  server.on("/api/list", HTTP_GET, handleList);
  server.on("/api/mkdir", HTTP_POST, handleMkdir);
  server.on("/api/touch", HTTP_POST, handleTouch);
  server.on("/api/rename", HTTP_POST, handleRename);
  server.on("/api/delete", HTTP_POST, handleDelete);
  server.on("/api/upload", HTTP_POST, [](){ server.send(200, "application/json", "{\"ok\":true}"); }, handleUpload);
  server.on("/api/webui", HTTP_POST, handleWebuiToggle);
  server.on("/api/backup", HTTP_GET, handleBackup);
  server.on("/api/backup/restore", HTTP_POST, handleBackupRestore);
  server.on("/api/known", HTTP_GET, handleKnownList);
  server.on("/api/known/forget", HTTP_POST, handleKnownForget);
  server.onNotFound(handleRoot);
  server.begin();
  webuiRunning = true;
  LOGI("WebUI AP started at %s", webuiUrl.c_str());
}

void webuiStop(){
  if (webuiRunning){
    server.stop();
    webuiRunning = false;
  }
}

void webuiLoop(){
  if (webuiRunning) server.handleClient();
}

String webuiGetUrl(){
  return webuiUrl;
}

void webuiShowInfo(const char* title){
  M5.Display.fillScreen(BLACK);
  drawStatus();
  M5.Display.setTextColor(WHITE);
  M5.Display.setCursor(6, STATUS_H + 10);
  M5.Display.print(title);
  M5.Display.setCursor(6, STATUS_H + 28);
  M5.Display.print(webuiApMode ? "AP: idk" : "STA Mode");
  if (webuiApMode){
    M5.Display.setCursor(6, STATUS_H + 44);
    M5.Display.print("PASS: idk");
  }
  M5.Display.setCursor(6, STATUS_H + 60);
  M5.Display.print(webuiGetUrl());
  M5.Display.setTextColor(COLOR_DIM);
  M5.Display.setCursor(6, SCREEN_H-10);
  M5.Display.print("Prev=Back");
}

bool webuiPromptOpen(){
  M5.Display.fillScreen(BLACK);
  drawStatus();
  M5.Display.setTextColor(WHITE);
  M5.Display.setCursor(6, STATUS_H + 20);
  M5.Display.print("Open WebUI?");
  M5.Display.setTextColor(COLOR_DIM);
  M5.Display.setCursor(6, SCREEN_H-10);
  M5.Display.print("M5=Yes  Prev=No");
  while (true){
    M5.update();
    if (M5.BtnA.wasPressed()) return true;
    if (M5.BtnPWR.wasPressed()) return false;
    delay(10);
  }
}

void webuiInit(){
  webuiPrefs.begin(WEBUI_NS, true);
  webuiEnabled = webuiPrefs.getBool(WEBUI_KEY_EN, true);
  webuiPrefs.end();
}

void webuiPersistEnabled(bool enabled){
  webuiPrefs.begin(WEBUI_NS, false);
  webuiPrefs.putBool(WEBUI_KEY_EN, enabled);
  webuiPrefs.end();
}
