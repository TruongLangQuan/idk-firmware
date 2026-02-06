#include "screens/ir.h"
#include "core/ui.h"
#include "modules/ir.h"

void screenIrListUpdate(){
  if (M5.BtnB.wasPressed()) { if (irCount>0) { irIndex = (irIndex + 1) % irCount; drawList("IR", irFiles, irCount, irIndex);} }
  if (M5.BtnPWR.wasPressed()) { if (irCount>0) { irIndex = (irIndex - 1 + irCount) % irCount; drawList("IR", irFiles, irCount, irIndex);} }
  if (M5.BtnPWR.pressedFor(800)) { screen = SCR_MENU; drawMenu(); }
  if (M5.BtnA.wasPressed() && irCount>0){
    parseIRFile(irFiles[irIndex]);
    cmdIndex = 0; cmdScroll = 0;
    screen = SCR_IR_CMD_LIST;
    drawIRCmdList();
  }
}

void screenIrCmdUpdate(){
  if (M5.BtnB.wasPressed()){
    cmdIndex = (cmdIndex + 1) % cmdCount;
    if (cmdIndex >= cmdScroll + 8) cmdScroll++;
    drawIRCmdList();
  }
  if (M5.BtnPWR.wasPressed()) {
    if (cmdCount > 0) {
      cmdIndex = (cmdIndex - 1 + cmdCount) % cmdCount;
      if (cmdIndex < cmdScroll) cmdScroll = cmdIndex;
      drawIRCmdList();
    }
  }
  if (M5.BtnPWR.pressedFor(800)) { screen = SCR_IR_LIST; drawList("IR", irFiles, irCount, irIndex); }
  if (M5.BtnA.wasPressed()) { sendIRCommand(cmdIndex); }
  if (M5.BtnA.pressedFor(800)) { spamAllIRCommands(); drawIRCmdList(); }
}
