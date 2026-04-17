#pragma once

#include <Arduino.h>
#include <Stream.h>

struct SdFileHandle;
struct SdDirHandle;

struct SdDirEntry {
  String name;
  bool isDir;
  uint64_t size;
};

bool sdCompatBegin(uint8_t csPin);
bool sdCompatReady();

uint64_t sdCompatTotalBytes();
uint64_t sdCompatUsedBytes();
uint32_t sdCompatCardErrorCode();
uint32_t sdCompatCardErrorData();

bool sdCompatExists(const String &path);
bool sdCompatMkdir(const String &path);
bool sdCompatMkdirs(const String &path);
bool sdCompatRmdir(const String &path);
bool sdCompatRemove(const String &path);
bool sdCompatRename(const String &from, const String &to);

bool sdCompatIsDirPath(const String &path);

uint64_t sdCompatDirSize(const String &path);
bool sdCompatDeletePath(const String &path);

SdDirHandle* sdCompatOpenDir(const String &path);
bool sdCompatDirNext(SdDirHandle *dir, SdDirEntry &out);
void sdCompatDirRewind(SdDirHandle *dir);
void sdCompatDirClose(SdDirHandle *dir);

SdFileHandle* sdCompatOpenRead(const String &path);
SdFileHandle* sdCompatOpenWrite(const String &path, bool truncate);
bool sdCompatIsDir(SdFileHandle *file);
uint64_t sdCompatFileSize(SdFileHandle *file);
bool sdCompatAvailable(SdFileHandle *file);
int sdCompatRead(SdFileHandle *file, uint8_t *buf, size_t len);
int sdCompatReadByte(SdFileHandle *file);
size_t sdCompatWrite(SdFileHandle *file, const uint8_t *buf, size_t len);
size_t sdCompatPrint(SdFileHandle *file, const String &s);
bool sdCompatSeek(SdFileHandle *file, uint32_t pos);
uint32_t sdCompatPosition(SdFileHandle *file);
String sdCompatReadStringUntil(SdFileHandle *file, char delim);
Stream* sdCompatStream(SdFileHandle *file);
void sdCompatClose(SdFileHandle *file);
