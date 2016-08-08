#pragma once

#include "common.h"

#define KEY_ENCRYPT (1<<0)
#define KEY_DECRYPT (1<<1)

#define KEYS_UNKNOWN 0
#define KEYS_RETAIL  1
#define KEYS_DEVKIT  2

typedef struct {
    u8   slot; // keyslot, 0x00...0x3F 
    char type; // type 'X' / 'Y' / 'N' for normalKey
    char id[10]; // key ID for special keys, all zero for standard keys
    u8   reserved[2]; // reserved space
    u8   isDevkitKey; // 0 for retail units / 1 for DevKit units
    u8   isEncrypted; // 0 if not / anything else if it is
    u8   key[16];
} __attribute__((packed)) AesKeyInfo;

u32 SetupCommonKeyY0x3D(u32 commonKeyIndex);
u32 SetupSdKeyY0x34(bool from_nand, u8* movable_key);
u32 SetupSector0x96Key0x11(void);
u32 SetupSecretKey0x11(u32 keynum);
u32 SetupTwlKey0x03(void);
u32 SetupCtrNandKeyY0x05(void);
u32 GetUnitKeysType(void);
u32 LoadKeyFromFile(u32 keyslot, char type, char* id);
u32 CheckKeySlot(u32 keyslot, char type);

// --> FEATURE FUNCTIONS <--
u32 BuildKeyDb(u32 param);
u32 CryptKeyDb(u32 param);
