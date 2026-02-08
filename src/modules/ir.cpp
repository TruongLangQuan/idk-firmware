#include "modules/ir.h"
#include <IRutils.h>

// Helper: byte-swap 32-bit
static uint32_t swap32(uint32_t value) {
  return ((value & 0x000000FF) << 24) | ((value & 0x0000FF00) << 8) | ((value & 0x00FF0000) >> 8) |
         ((value & 0xFF000000) >> 24);
}

// External IRsend instance (optional) and runtime-configurable pin
static IRsend *g_external_ir = nullptr;
static IRsend *g_local_ir = nullptr;
static uint8_t g_ir_pin = IR_PIN;

static IRsend *getIrSender() {
  if (g_external_ir) return g_external_ir;
  if (!g_local_ir) {
    g_local_ir = new IRsend(g_ir_pin);
    g_local_ir->begin();
  }
  return g_local_ir;
}

void attachExternalIRsend(IRsend *sender){ g_external_ir = sender; }
void detachExternalIRsend(){ g_external_ir = nullptr; }
void setIrPin(uint8_t pin){
  g_ir_pin = pin;
  if (g_local_ir) {
    // recreate local sender with new pin
    delete g_local_ir; g_local_ir = nullptr;
  }
}
uint8_t getIrPin(){ return g_ir_pin; }

void parseIRFile(const String &path){
  cmdCount = 0;
  File f = SPIFFS.open(path, "r");
  if (!f) return;
  while(f.available() && cmdCount < MAX_CMDS){
    String line = f.readStringUntil('\n');
    line.trim();
    if (line.length() == 0) continue;
    if (line.indexOf('=') > 0) {
      int eq = line.indexOf('=');
      String name = line.substring(0, eq);
      String val = line.substring(eq+1);
      name.trim(); val.trim();
      uint32_t v = (uint32_t) strtoul(val.c_str(), NULL, 0);
      cmdName[cmdCount] = name;
      cmdHex[cmdCount] = v;
      cmdCount++;
    } else if (line.startsWith("RAW:") || line.startsWith("raw:")) {
      String raw = line.substring(line.indexOf(':')+1);
      raw.trim();
      cmdName[cmdCount] = "RAW:" + raw;
      cmdHex[cmdCount] = 0;
      cmdCount++;
    }
  }
  f.close();
}

void sendIRCommand(int idx){
  if (idx < 0 || idx >= cmdCount) return;
  if (cmdHex[idx] != 0) {
    irsend.sendNEC(cmdHex[idx], 32);
  } else {
    String rawLine = cmdName[idx].substring(4);
    std::vector<uint16_t> timings;
    int start = 0;
    while (start < (int)rawLine.length()) {
      int comma = rawLine.indexOf(',', start);
      if (comma == -1) comma = rawLine.length();
      String token = rawLine.substring(start, comma);
      token.trim();
      int v = token.toInt();
      if (v > 0) timings.push_back((uint16_t)v);
      start = comma + 1;
    }
    if (timings.size() > 0) {
      irsend.sendRaw(timings.data(), timings.size(), 38);
    }
  }
}

void spamAllIRCommands(){
  for (int i=0;i<cmdCount;i++){
    sendIRCommand(i);
    delay(120);
  }
}

// --- Extended send helpers (based on custom_ir) ---

void sendIRCommand(IRCode *code, bool hideDefaultUI){
  // Choose between raw or protocol-based
  if (!code) return;
  if (code->type.equalsIgnoreCase("raw") || code->type.equalsIgnoreCase("RAW")) {
    sendRawCommand(code->frequency, code->data, hideDefaultUI);
    return;
  }
  if (code->protocol.length() > 0 && code->data.length() > 0) {
    // Try decoded send
    sendDecodedCommand(code->protocol, code->data, code->bits, hideDefaultUI);
    return;
  }
  // Fallback: try a few protocol-specific heuristics
  if (code->protocol.equalsIgnoreCase("NEC")) sendNECCommand(code->address, code->command, hideDefaultUI);
  else if (code->protocol.equalsIgnoreCase("RC5")) sendRC5Command(code->address, code->command, hideDefaultUI);
  else if (code->protocol.equalsIgnoreCase("RC6")) sendRC6Command(code->address, code->command, hideDefaultUI);
}

void sendNECCommand(String address, String command, bool hideDefaultUI){
  IRsend *ir = getIrSender();
  uint16_t addressValue = (uint16_t)strtoul(address.substring(0, min(2, (int)address.length())).c_str(), nullptr, 16);
  uint16_t commandValue = (uint16_t)strtoul(command.substring(0, min(2, (int)command.length())).c_str(), nullptr, 16);
  uint64_t data = ir->encodeNEC(addressValue, commandValue);
  ir->sendNEC(data, 32);
  for (int i=0;i<IR_TX_REPEATS;i++) ir->sendNEC(data, 32);
}

void sendNECextCommand(String address, String command, bool hideDefaultUI){
  IRsend *ir = getIrSender();
  address.replace(" ", "");
  command.replace(" ", "");
  uint32_t addressValue = (uint32_t)strtoul(address.c_str(), nullptr, 16);
  uint32_t commandValue = (uint32_t)strtoul(command.c_str(), nullptr, 16);
  uint16_t newAddress = (addressValue >> 8) | (addressValue << 8);
  uint16_t newCommand = (commandValue >> 8) | (commandValue << 8);
  uint16_t lsbAddress = reverseBits(newAddress, 16);
  uint16_t lsbCommand = reverseBits(newCommand, 16);
  uint32_t data = ((uint32_t)lsbAddress << 16) | lsbCommand;
  ir->sendNEC(data, 32);
  for (int i=0;i<IR_TX_REPEATS;i++) ir->sendNEC(data, 32);
}

void sendRC5Command(String address, String command, bool hideDefaultUI){
  IRsend *ir = getIrSender();
  uint8_t addressValue = (uint8_t)strtoul(address.substring(0, min(2, (int)address.length())).c_str(), nullptr, 16);
  uint8_t commandValue = (uint8_t)strtoul(command.substring(0, min(2, (int)command.length())).c_str(), nullptr, 16);
  uint16_t data = ir->encodeRC5(addressValue, commandValue);
  ir->sendRC5(data, 13);
  for (int i=0;i<IR_TX_REPEATS;i++) ir->sendRC5(data, 13);
}

void sendRC6Command(String address, String command, bool hideDefaultUI){
  IRsend *ir = getIrSender();
  address.replace(" ", "");
  command.replace(" ", "");
  uint32_t addressValue = (uint32_t)strtoul(address.substring(0, min(2, (int)address.length())).c_str(), nullptr, 16);
  uint32_t commandValue = (uint32_t)strtoul(command.substring(0, min(2, (int)command.length())).c_str(), nullptr, 16);
  uint64_t data = ir->encodeRC6(addressValue, commandValue);
  ir->sendRC6(data, 20);
  for (int i=0;i<IR_TX_REPEATS;i++) ir->sendRC6(data, 20);
}

void sendSamsungCommand(String address, String command, bool hideDefaultUI){
  IRsend *ir = getIrSender();
  uint8_t addressValue = (uint8_t)strtoul(address.substring(0, min(2, (int)address.length())).c_str(), nullptr, 16);
  uint8_t commandValue = (uint8_t)strtoul(command.substring(0, min(2, (int)command.length())).c_str(), nullptr, 16);
  uint64_t data = ir->encodeSAMSUNG(addressValue, commandValue);
  ir->sendSAMSUNG(data, 32);
  for (int i=0;i<IR_TX_REPEATS;i++) ir->sendSAMSUNG(data, 32);
}

void sendSonyCommand(String address, String command, uint8_t nbits, bool hideDefaultUI){
  IRsend *ir = getIrSender();
  address.replace(" ", "");
  command.replace(" ", "");
  uint32_t addressValue = (uint32_t)strtoul(address.c_str(), nullptr, 16);
  uint32_t commandValue = (uint32_t)strtoul(command.c_str(), nullptr, 16);
  uint16_t swappedAddr = static_cast<uint16_t>(swap32(addressValue));
  uint8_t swappedCmd = static_cast<uint8_t>(swap32(commandValue));
  uint32_t data = 0;
  if (nbits == 12) data = ((swappedAddr & 0x1F) << 7) | (swappedCmd & 0x7F);
  else if (nbits == 15) data = ((swappedAddr & 0xFF) << 7) | (swappedCmd & 0x7F);
  else if (nbits == 20) data = ((swappedAddr & 0x1FFF) << 7) | (swappedCmd & 0x7F);
  else return;
  data = reverseBits(data, nbits);
  ir->sendSony(data, nbits, 2);
  for (int i=0;i<IR_TX_REPEATS;i++) ir->sendSony(data, nbits, 2);
}

void sendKaseikyoCommand(String address, String command, bool hideDefaultUI){
  IRsend *ir = getIrSender();
  address.replace(" ", "");
  command.replace(" ", "");
  uint32_t addressValue = (uint32_t)strtoul(address.c_str(), nullptr, 16);
  uint32_t commandValue = (uint32_t)strtoul(command.c_str(), nullptr, 16);
  uint32_t newAddress = swap32(addressValue);
  uint16_t newCommand = static_cast<uint16_t>(swap32(commandValue));
  uint8_t id = (newAddress >> 24) & 0xFF;
  uint16_t vendor_id = (newAddress >> 8) & 0xFFFF;
  uint8_t genre1 = (newAddress >> 4) & 0x0F;
  uint8_t genre2 = newAddress & 0x0F;
  uint16_t data = newCommand & 0x3FF;
  byte bytes[6];
  bytes[0] = vendor_id & 0xFF;
  bytes[1] = (vendor_id >> 8) & 0xFF;
  uint8_t vendor_parity = bytes[0] ^ bytes[1];
  vendor_parity = (vendor_parity & 0xF) ^ (vendor_parity >> 4);
  bytes[2] = (genre1 << 4) | (vendor_parity & 0x0F);
  bytes[3] = ((data & 0x0F) << 4) | genre2;
  bytes[4] = ((id & 0x03) << 6) | ((data >> 4) & 0x3F);
  bytes[5] = bytes[2] ^ bytes[3] ^ bytes[4];
  uint64_t lsb_data = 0;
  for (int i = 0; i < 6; i++) lsb_data |= (uint64_t)bytes[i] << (8 * i);
  uint64_t msb_data = reverseBits(lsb_data, 48);
  ir->sendPanasonic64(msb_data, 48);
  for (int i=0;i<IR_TX_REPEATS;i++) ir->sendPanasonic64(msb_data, 48);
}

bool sendDecodedCommand(String protocol, String value, uint8_t bits, bool hideDefaultUI){
  decode_type_t type = decode_type_t::UNKNOWN;
  String p = protocol;
  p.toUpperCase();
  if (p == "NEC") type = decode_type_t::NEC;
  else if (p == "SONY" || p == "SIRC") type = decode_type_t::SONY;
  else if (p == "RC5") type = decode_type_t::RC5;
  else if (p == "RC6") type = decode_type_t::RC6;
  else if (p == "PANASONIC" || p == "KASEIKYO") type = decode_type_t::PANASONIC;
  else if (p == "SAMSUNG") type = decode_type_t::SAMSUNG;
  else if (p == "UNKNOWN") type = decode_type_t::UNKNOWN;

  if (type == decode_type_t::UNKNOWN) return false;

  IRsend *ir = getIrSender();
  value.replace(" ", "");
  uint64_t value_int = strtoull(value.c_str(), nullptr, 16);
  ir->send(type, value_int, bits);
  for (int i=0;i<IR_TX_REPEATS;i++) ir->send(type, value_int, bits);
  return true;
}

void sendRawCommand(uint16_t frequency, String rawData, bool hideDefaultUI){
  IRsend *ir = getIrSender();
  uint16_t dataBufferSize = 1;
  for (int i = 0; i < rawData.length(); i++) if (rawData[i] == ' ') dataBufferSize += 1;
  uint16_t *dataBuffer = (uint16_t *)malloc((dataBufferSize) * sizeof(uint16_t));
  uint16_t count = 0;
  while (rawData.length() > 0 && count < dataBufferSize) {
    int delimiterIndex = rawData.indexOf(' ');
    if (delimiterIndex == -1) delimiterIndex = rawData.length();
    String dataChunk = rawData.substring(0, delimiterIndex);
    rawData.remove(0, delimiterIndex + 1);
    dataBuffer[count++] = (uint16_t)(dataChunk.toInt());
  }
  if (count > 0) ir->sendRaw(dataBuffer, count, frequency);
  for (int i=0;i<IR_TX_REPEATS;i++) ir->sendRaw(dataBuffer, count, frequency);
  free(dataBuffer);
}
