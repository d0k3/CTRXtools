#pragma once

#include "common.h"

bool InitFS();
void DeinitFS();

/** Opens existing files */
bool FileOpen(const char* path);

/** Opens new files (and creates them if they don't already exist) */
bool FileCreate(const char* path, bool truncate);

/** Reads contents of the opened file */
size_t FileRead(void* buf, size_t size, size_t foffset);

/** Writes to the opened file */
size_t FileWrite(void* buf, size_t size, size_t foffset);

/** Gets the size of the opened file */
size_t FileGetSize();

/** Gets remaining space on SD card in bytes */
uint64_t RemainingStorageSpace();

void FileClose();
