#pragma once
#include "app/state.h"
#include <FS.h>
#include <IRremoteESP8266.h>
#include <IRsend.h>

// Simple configurable repeat count (0 = no repeats)
#ifndef IR_TX_REPEATS
#define IR_TX_REPEATS 0
#endif

struct IRCode {
	IRCode(String protocol = "", String address = "", String command = "", String data = "", uint8_t bits = 32)
		: protocol(protocol), address(address), command(command), data(data), bits(bits) {}

	IRCode(IRCode *code) {
		name = String(code->name);
		type = String(code->type);
		protocol = String(code->protocol);
		address = String(code->address);
		command = String(code->command);
		frequency = code->frequency;
		bits = code->bits;
		data = String(code->data);
		filepath = String(code->filepath);
	}

	String protocol = "";
	String address = "";
	String command = "";
	String data = "";
	uint8_t bits = 32;
	String name = "";
	String type = "";
	uint16_t frequency = 0;
	String filepath = "";
};

// Existing simple API
void parseIRFile(const String &path);
void sendIRCommand(int idx);
void spamAllIRCommands();

// Extended API (compatible with custom_ir.h)
void sendIRCommand(IRCode *code, bool hideDefaultUI = false);
void sendRawCommand(uint16_t frequency, String rawData, bool hideDefaultUI = false);
void sendNECCommand(String address, String command, bool hideDefaultUI = false);
void sendNECextCommand(String address, String command, bool hideDefaultUI = false);
void sendRC5Command(String address, String command, bool hideDefaultUI = false);
void sendRC6Command(String address, String command, bool hideDefaultUI = false);
void sendSamsungCommand(String address, String command, bool hideDefaultUI = false);
void sendSonyCommand(String address, String command, uint8_t nbits, bool hideDefaultUI = false);
void sendKaseikyoCommand(String address, String command, bool hideDefaultUI = false);
bool sendDecodedCommand(String protocol, String value, uint8_t bits = 32, bool hideDefaultUI = false);

// External IR transmitter attachment
void attachExternalIRsend(IRsend *sender);
void detachExternalIRsend();
void setIrPin(uint8_t pin);
uint8_t getIrPin();

