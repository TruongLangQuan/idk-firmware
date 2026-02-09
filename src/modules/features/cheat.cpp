#include "modules/features/cheat.h"
#include "core/ui.h"

const char* CHEAT_CATEGORIES[] = {"Toan", "Vat Ly", "Hoa Hoc", "English"};
const int CHEAT_CAT_COUNT = 4;

// Math formulas (Vietnamese) - Expanded
const char* MATH_FORMULAS[] = {
  "Hang dang thuc:",
  "(a+b)^2 = a^2+2ab+b^2",
  "(a-b)^2 = a^2-2ab+b^2",
  "a^2-b^2 = (a-b)(a+b)",
  "(a+b)^3 = a^3+3a^2b+3ab^2+b^3",
  "(a-b)^3 = a^3-3a^2b+3ab^2-b^3",
  "a^3+b^3 = (a+b)(a^2-ab+b^2)",
  "a^3-b^3 = (a-b)(a^2+ab+b^2)",
  "",
  "Phuong trinh bac 2:",
  "ax^2+bx+c=0",
  "Delta = b^2-4ac",
  "x = (-b+-sqrt(Delta))/(2a)",
  "Delta>0: 2 nghiem",
  "Delta=0: nghiem kep",
  "Delta<0: vo nghiem",
  "",
  "Dinh li Viet:",
  "x1+x2 = -b/a",
  "x1*x2 = c/a",
  "",
  "Dinh li Pythagoras:",
  "a^2+b^2=c^2",
  "",
  "Dien tich hinh:",
  "Vuong: S=a^2",
  "Chu nhat: S=ab",
  "Tam giac: S=(ah)/2",
  "Tron: S=pi*r^2",
  "Hinh thang: S=(a+b)h/2",
  "Hinh binh hanh: S=ah",
  "Hinh thoi: S=d1*d2/2",
  "",
  "The tich:",
  "Lap phuong: V=a^3",
  "Hop chu nhat: V=abc",
  "Tru: V=pi*r^2*h",
  "Non: V=(pi*r^2*h)/3",
  "Cau: V=(4*pi*r^3)/3",
  "",
  "Chu vi:",
  "Vuong: P=4a",
  "Chu nhat: P=2(a+b)",
  "Tron: P=2*pi*r",
  "",
  "Ty so luong giac:",
  "sin = doi/huyen",
  "cos = ke/huyen",
  "tan = doi/ke",
  "cot = ke/doi"
};
const int MATH_COUNT = 50;

// Physics formulas (Vietnamese) - Expanded
const char* PHYSICS_FORMULAS[] = {
  "Van toc:",
  "v = s/t",
  "v = v0 + at",
  "v^2 = v0^2 + 2as",
  "",
  "Quang duong:",
  "s = vt",
  "s = v0t + (at^2)/2",
  "s = (v0+v)t/2",
  "",
  "Gia toc:",
  "a = (v-v0)/t",
  "a = F/m",
  "",
  "Dinh luat Newton:",
  "F = ma (Newton 2)",
  "F = -F' (Newton 3)",
  "",
  "Cong suat:",
  "P = A/t",
  "P = Fv",
  "P = UI (dien)",
  "",
  "Cong:",
  "A = Fs",
  "A = Pt",
  "",
  "Dong nang:",
  "W = (mv^2)/2",
  "",
  "The nang:",
  "Wt = mgh",
  "Wt = (kx^2)/2",
  "",
  "Dinh luat Ohm:",
  "U = IR",
  "R = U/I",
  "",
  "Cong suat dien:",
  "P = UI",
  "P = I^2*R",
  "P = U^2/R",
  "",
  "Cong dien:",
  "A = UIt",
  "A = Pt",
  "",
  "Nhiet luong:",
  "Q = mc(t2-t1)",
  "Q = mL",
  "",
  "Ap suat:",
  "p = F/S",
  "p = dgh",
  "",
  "Luc day Archimedes:",
  "FA = dVg"
};
const int PHYSICS_COUNT = 45;

// Chemistry formulas (Vietnamese) - Expanded
const char* CHEMISTRY_FORMULAS[] = {
  "Can bang phuong trinh:",
  "VD: 2H2 + O2 -> 2H2O",
  "Can bang theo nguyen to",
  "",
  "Phan ung axit-bazo:",
  "HCl + NaOH -> NaCl + H2O",
  "H2SO4 + 2NaOH -> Na2SO4+2H2O",
  "",
  "Phan ung oxi hoa-khu:",
  "CuO + H2 -> Cu + H2O",
  "Fe2O3 + 3CO -> 2Fe + 3CO2",
  "",
  "Phan ung the:",
  "Fe + CuSO4 -> FeSO4 + Cu",
  "Zn + 2HCl -> ZnCl2 + H2",
  "",
  "Cau tao nguyen tu:",
  "So p = So e = Z",
  "So n = A - Z",
  "A = Z + N",
  "",
  "Cau hinh electron:",
  "Lop 1: toi da 2e",
  "Lop 2: toi da 8e",
  "Lop 3: toi da 8e",
  "Lop 4: toi da 18e",
  "",
  "Hoan vi:",
  "Hoa tri cao nhat = STT nhom",
  "Hoa tri thap = 8 - STT nhom",
  "",
  "Cong thuc tinh:",
  "n = m/M",
  "n = V/22.4 (khi)",
  "CM = n/V",
  "C% = (mct/mdd)*100%",
  "",
  "Dinh luat bao toan:",
  "m truoc = m sau",
  "So mol truoc = sau",
  "",
  "Bang tuan hoan:",
  "Chu ky: so lop",
  "Nhom: so e ngoai",
  "",
  "Lien ket hoa hoc:",
  "Ion: cho/nhan e",
  "Cong hoa tri: dung chung",
  "",
  "Dung dich:",
  "mdd = mct + mnc",
  "Vdd = Vnc (gan dung)"
};
const int CHEMISTRY_COUNT = 40;

// English tenses and irregular verbs - Expanded
const char* ENGLISH_CONTENT[] = {
  "Thi hien tai don:",
  "S + V(s/es)",
  "I go, He goes",
  "Dau hieu: always,often",
  "",
  "Thi qua khu don:",
  "S + V2/ed",
  "I went, He played",
  "Dau hieu: yesterday,ago",
  "",
  "Thi tuong lai:",
  "S + will + V",
  "I will go",
  "Dau hieu: tomorrow,next",
  "",
  "Thi hien tai tiep dien:",
  "S + am/is/are + V-ing",
  "Dau hieu: now,at the moment",
  "",
  "Thi qua khu tiep dien:",
  "S + was/were + V-ing",
  "",
  "Thi hien tai hoan thanh:",
  "S + have/has + V3",
  "Dau hieu: already,just,yet",
  "",
  "Thi qua khu hoan thanh:",
  "S + had + V3",
  "",
  "Dong tu bat quy tac:",
  "be -> was/were -> been",
  "go -> went -> gone",
  "do -> did -> done",
  "see -> saw -> seen",
  "take -> took -> taken",
  "come -> came -> come",
  "get -> got -> got",
  "make -> made -> made",
  "know -> knew -> known",
  "think -> thought -> thought",
  "give -> gave -> given",
  "write -> wrote -> written",
  "read -> read -> read",
  "speak -> spoke -> spoken",
  "break -> broke -> broken",
  "choose -> chose -> chosen",
  "drive -> drove -> driven",
  "eat -> ate -> eaten",
  "fall -> fell -> fallen",
  "find -> found -> found",
  "forget -> forgot -> forgotten",
  "have -> had -> had",
  "hear -> heard -> heard",
  "keep -> kept -> kept",
  "leave -> left -> left",
  "lose -> lost -> lost",
  "meet -> met -> met",
  "pay -> paid -> paid",
  "say -> said -> said",
  "send -> sent -> sent",
  "sit -> sat -> sat",
  "sleep -> slept -> slept",
  "spend -> spent -> spent",
  "stand -> stood -> stood",
  "teach -> taught -> taught",
  "tell -> told -> told",
  "understand -> understood",
  "wear -> wore -> worn",
  "win -> won -> won"
};
const int ENGLISH_COUNT = 70;

void runCheat(){
  int catIndex = 0;
  int scroll = 0;
  
  while (true){
    M5.update();
    if (M5.BtnPWR.wasPressed()) break;
    
    if (M5.BtnB.wasPressed()){
      catIndex = (catIndex + 1) % CHEAT_CAT_COUNT;
      scroll = 0;
    }
    
    if (M5.BtnA.wasPressed()){
      scroll += 8;
    }
    
    M5.Display.fillScreen(BLACK);
    drawStatus();
    
    // Category title
    M5.Display.setTextColor(WHITE);
    M5.Display.setCursor(6, STATUS_H + 2);
    M5.Display.print(CHEAT_CATEGORIES[catIndex]);
    
    // Content
    const char** content = nullptr;
    int contentCount = 0;
    if (catIndex == 0){
      content = MATH_FORMULAS;
      contentCount = MATH_COUNT;
    } else if (catIndex == 1){
      content = PHYSICS_FORMULAS;
      contentCount = PHYSICS_COUNT;
    } else if (catIndex == 2){
      content = CHEMISTRY_FORMULAS;
      contentCount = CHEMISTRY_COUNT;
    } else {
      content = ENGLISH_CONTENT;
      contentCount = ENGLISH_COUNT;
    }
    
    int maxShow = 8;
    int startY = STATUS_H + 16;
    for (int i=0; i<maxShow && (scroll+i) < contentCount; i++){
      int idx = scroll + i;
      M5.Display.setTextColor(COLOR_DIM);
      M5.Display.setCursor(6, startY + i*12);
      String line = content[idx];
      if (line.length() > 30) line = line.substring(0, 27) + "...";
      M5.Display.print(line);
    }
    
    // Scroll indicator
    if (scroll + maxShow < contentCount){
      M5.Display.setTextColor(COLOR_DIM);
      M5.Display.setCursor(SCREEN_W - 20, SCREEN_H - 10);
      M5.Display.print("M5=More");
    }
    
    M5.Display.setTextColor(COLOR_DIM);
    M5.Display.setCursor(6, SCREEN_H - 10);
    M5.Display.print("Next=Cat  Prev=Exit");
    
    delay(10);
  }
}
