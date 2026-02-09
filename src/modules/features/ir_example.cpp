#include "modules/features/ir.h"

// Example usage helper — not invoked automatically. Put this in your init or call from a button handler.
void exampleUseIrFile(const String &path){
  // parseIRFile expects SPIFFS path (as created by scanSPIFFS), e.g. "/ir/Samsung_TV_Full.ir"
  parseIRFile(path);
  // If you want to send the first parsed command:
  if (cmdCount > 0) {
    sendIRCommand(0); // sends the first command
  }
  // Or spam all commands in the file:
  // spamAllIRCommands();
}

// Convenience helper for the Optoma example file included in /data/ir
void exampleUseOptomaIr(){
  exampleUseIrFile("/ir/Optoma.ir");
}
