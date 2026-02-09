#include "modules/cheat.h"
#include "core/ui.h"

const char* CHEAT_CATEGORIES[] = {"Toan", "Vat Ly", "Hoa Hoc", "English"};
const int CHEAT_CAT_COUNT = 4;

// Math formulas (Vietnamese)
const char* MATH_FORMULAS[] = {
  "Hang dang thuc:",
  "(a+b)^2 = a^2+2ab+b^2",
  "(a-b)^2 = a^2-2ab+b^2",
  "a^2-b^2 = (a-b)(a+b)",
  "(a+b)^3 = a^3+3a^2b+3ab^2+b^3",
  "",
  "Phuong trinh bac 2:",
  "ax^2+bx+c=0",
  "Delta = b^2-4ac",
  "x = (-b+-sqrt(Delta))/(2a)",
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
  "",
  "The tich:",
  "Lap phuong: V=a^3",
  "Hop chu nhat: V=abc",
  "Tru: V=pi*r^2*h",
  "Non: V=(pi*r^2*h)/3"
};
const int MATH_COUNT = 25;

// Physics formulas (Vietnamese)
const char* PHYSICS_FORMULAS[] = {
  "Van toc:",
  "v = s/t",
  "v = v0 + at",
  "",
  "Quang duong:",
  "s = vt",
  "s = v0t + (at^2)/2",
  "",
  "Gia toc:",
  "a = (v-v0)/t",
  "",
  "Dinh luat Newton 2:",
  "F = ma",
  "",
  "Cong suat:",
  "P = A/t",
  "P = Fv",
  "",
  "Dong nang:",
  "W = (mv^2)/2",
  "",
  "The nang:",
  "Wt = mgh",
  "",
  "Dinh luat Ohm:",
  "U = IR",
  "",
  "Cong suat dien:",
  "P = UI",
  "P = I^2*R"
};
const int PHYSICS_COUNT = 22;

// Chemistry formulas (Vietnamese)
const char* CHEMISTRY_FORMULAS[] = {
  "Can bang phuong trinh:",
  "VD: 2H2 + O2 -> 2H2O",
  "",
  "Phan ung axit-bazo:",
  "HCl + NaOH -> NaCl + H2O",
  "",
  "Phan ung oxi hoa-khu:",
  "CuO + H2 -> Cu + H2O",
  "",
  "Cau tao nguyen tu:",
  "So p = So e = Z",
  "So n = A - Z",
  "",
  "Cau hinh electron:",
  "Lop 1: toi da 2e",
  "Lop 2: toi da 8e",
  "Lop 3: toi da 8e",
  "",
  "Hoan vi:",
  "Hoa tri cao nhat = STT nhom",
  "",
  "Cong thuc tinh:",
  "n = m/M",
  "n = V/22.4 (khi)",
  "CM = n/V"
};
const int CHEMISTRY_COUNT = 20;

// English tenses and irregular verbs
const char* ENGLISH_CONTENT[] = {
  "Thi hien tai don:",
  "S + V(s/es)",
  "I go, He goes",
  "",
  "Thi qua khu don:",
  "S + V2/ed",
  "I went, He played",
  "",
  "Thi tuong lai:",
  "S + will + V",
  "I will go",
  "",
  "Thi hien tai tiep dien:",
  "S + am/is/are + V-ing",
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
  "think -> thought -> thought"
};
const int ENGLISH_COUNT = 25;

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
