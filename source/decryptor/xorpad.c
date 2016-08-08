#include "fs.h"
#include "draw.h"
#include "platform.h"
#include "decryptor/aes.h"
#include "decryptor/sha.h"
#include "decryptor/decryptor.h"
#include "decryptor/keys.h"
#include "decryptor/nand.h"
#include "decryptor/game.h"
#include "decryptor/xorpad.h"
#include "fatfs/sdmmc.h"


u32 CreatePad(PadInfo *info)
{
    u8* buffer = BUFFER_ADDRESS;
    u32 size_byte = (info->size_mb) ? info->size_mb * 1024*1024 : info->size_b;
    u32 result = 0;
    
    if (!DebugCheckFreeSpace(size_byte))
        return 1;
    
    if (!FileCreate(info->filename, true)) // No DebugFileCreate() here - messages are already given
        return 1;
        
    CryptBufferInfo decryptInfo = {.keyslot = info->keyslot, .setKeyY = info->setKeyY, .mode = info->mode, .buffer = buffer};
    memcpy(decryptInfo.ctr, info->ctr, 16);
    memcpy(decryptInfo.keyY, info->keyY, 16);
    for (u32 i = 0; i < size_byte; i += BUFFER_MAX_SIZE) {
        u32 curr_block_size = min(BUFFER_MAX_SIZE, size_byte - i);
        decryptInfo.size = curr_block_size;
        memset(buffer, 0x00, curr_block_size);
        ShowProgress(i, size_byte);
        CryptBuffer(&decryptInfo);
        if (!DebugFileWrite((void*)buffer, curr_block_size, i)) {
            result = 1;
            break;
        }
    }

    ShowProgress(0, 0);
    FileClose();

    return result;
}

u32 SdInfoGen(SdInfo* info, const char* base_path)
{
    char* filelist = (char*)0x20400000;
    
    // check the base path for validity
    if ((strncmp(base_path, "/Nintendo 3DS", 13) != 0 ) || (strncmp(base_path, "/Nintendo 3DS/Private/", 22) == 0) ||
        (strnlen(base_path, 255) < 13 + 33 + 33)) {
        Debug("Invalid base path given");
        return 1;
    }
        
    Debug("Generating SDinfo.bin in memory...");
    if (!GetFileList(base_path, filelist, 0x100000, true, true, false)) {
        Debug("Failed retrieving the filelist");
        return 1;
    }
    
    u32 n_entries = 0;
    SdInfoEntry* entries = info->entries;
    for (char* path = strtok(filelist, "\n"); path != NULL; path = strtok(NULL, "\n")) {
        u32 plen = strnlen(path, 255);
        // get size in MB
        if (!FileOpen(path))
            continue;
        entries[n_entries].size_mb = (FileGetSize() + (1024 * 1024) - 1) / (1024 * 1024);
        FileClose();
        // skip to relevant part of path
        path += 13 + 33 + 33; // length of ("/Nintendo 3DS" + "/<id0>" + "/<id1>")
        plen -= 13 + 33 + 33;
        if ((strncmp(path, "/dbs", 4) != 0) && (strncmp(path, "/extdata", 8) != 0) && (strncmp(path, "/title", 6) != 0))
            continue;
        // get filename
        char* filename = entries[n_entries].filename;
        filename[0] = '/';
        for (u32 i = 1; i < 180 && path[i] != 0; i++)
            filename[i] = (path[i] == '/') ? '.' : path[i];
        strncpy(filename + plen, ".xorpad", (180 - 1) - plen);
        // get AES counter
        GetSdCtr(entries[n_entries].ctr, path);
        if (++n_entries >= MAX_ENTRIES)
            break;
    }
    info->n_entries = n_entries;
    
    return (n_entries > 0) ? 0 : 1;
}

u32 NcchPadgen(u32 param)
{
    (void) (param); // param is unused here
    NcchInfo *info = (NcchInfo*)0x20316000;
    SeedInfo *seedinfo = (SeedInfo*)0x20400000;

    if (CheckKeySlot(0x25, 'X') != 0) {
        Debug("slot0x25KeyX not set up");
        Debug("7.x crypto will fail on O3DS < 7.x or A9LH");
    }
    if ((GetUnitPlatform() == PLATFORM_3DS) && (CheckKeySlot(0x18, 'X') != 0)) {
        Debug("slot0x18KeyX not set up");
        Debug("Secure3 crypto will fail");
    }
    if (CheckKeySlot(0x1B, 'X') != 0) {
        Debug("slot0x1BKeyX not set up");
        Debug("Secure4 crypto will fail");
    }
       
    if (DebugFileOpen("seeddb.bin")) {
        if (!DebugFileRead(seedinfo, 16, 0)) {
            FileClose();
            return 1;
        }
        if (!seedinfo->n_entries || seedinfo->n_entries > MAX_ENTRIES) {
            FileClose();
            Debug("Bad number of seeddb entries");
            return 1;
        }
        if (!DebugFileRead(seedinfo->entries, seedinfo->n_entries * sizeof(SeedInfoEntry), 16)) {
            FileClose();
            return 1;
        }
        FileClose();
    } else {
        Debug("9.x seed crypto will fail");
    }

    if (!DebugFileOpen("ncchinfo.bin"))
        return 1;
    if (!DebugFileRead(info, 16, 0)) {
        FileClose();
        return 1;
    }
    if (!info->n_entries || info->n_entries > MAX_ENTRIES) {
        FileClose();
        Debug("Bad number of entries in ncchinfo.bin");
        return 1;
    }
    if (info->ncch_info_version == 0xF0000004) { // ncchinfo v4
        if (!DebugFileRead(info->entries, info->n_entries * sizeof(NcchInfoEntry), 16)) {
            FileClose();
            return 1;
        }
    } else if (info->ncch_info_version == 0xF0000003) { // ncchinfo v3
        // read ncchinfo v3 entry & convert to ncchinfo v4
        for (u32 i = 0; i < info->n_entries; i++) {
            u8* entry_data = (u8*) (info->entries + i);
            if (!DebugFileRead(entry_data, 160, 16 + (160*i))) {
                FileClose();
                return 1;
            }
            memmove(entry_data + 56, entry_data + 48, 112);
            *(u64*) (entry_data + 48) = 0;
        }
    } else { // unknown file / ncchinfo version
        FileClose();
        Debug("Incompatible version ncchinfo.bin");
        return 1;
    }
    FileClose();

    Debug("Number of entries: %i", info->n_entries);

    for (u32 i = 0; i < info->n_entries; i++) { // check and fix filenames
        char* filename = info->entries[i].filename;
        if (filename[1] == 0x00) { // convert UTF-16 -> UTF-8
            for (u32 j = 1; j < (112 / 2); j++)
                filename[j] = filename[j*2];
        }
        if (memcmp(filename, "sdmc:", 5) == 0) // fix sdmc: prefix
            memmove(filename, filename + 5, 112 - 5);
    }
    
    for (u32 i = 0; i < info->n_entries; i++) {
        PadInfo padInfo = {.setKeyY = 1, .size_mb = info->entries[i].size_mb, .mode = AES_CNT_CTRNAND_MODE};
        memcpy(padInfo.ctr, info->entries[i].ctr, 16);
        memcpy(padInfo.filename, info->entries[i].filename, 112);
        Debug ("%2i: %s (%iMB)", i, info->entries[i].filename, info->entries[i].size_mb);
        
        // workaround to still be able to process old ncchinfo.bin
        if ((info->entries[i].ncchFlag7 == 0x01) && info->entries[i].ncchFlag3)
            info->entries[i].ncchFlag7 = 0x20; // this combination means seed crypto rather than FixedKey
        
        if (info->entries[i].ncchFlag7 & 0x20) { // seed crypto
            u8 keydata[32];
            memcpy(keydata, info->entries[i].keyY, 16);
            u32 found_seed = 0;
            for (u32 j = 0; j < seedinfo->n_entries; j++) {
                if (seedinfo->entries[j].titleId == info->entries[i].titleId) {
                    found_seed = 1;
                    memcpy(&keydata[16], seedinfo->entries[j].external_seed, 16);
                    break;
                }
            }
            if (!found_seed) {
                Debug("Failed to find seed in seeddb.bin");
                return 1;
            }
            u8 sha256sum[32];
            sha_quick(sha256sum, keydata, 32, SHA256_MODE);
            memcpy(padInfo.keyY, sha256sum, 16);
        } else {
            memcpy(padInfo.keyY, info->entries[i].keyY, 16);
        }
        
        if (info->entries[i].ncchFlag7 == 0x01) {
            u8 zeroKey[16] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
            u8 sysKey[16]  = {0x52, 0x7C, 0xE6, 0x30, 0xA9, 0xCA, 0x30, 0x5F, 0x36, 0x96, 0xF3, 0xCD, 0xE9, 0x54, 0x19, 0x4B};
            setup_aeskey(0x11, (info->entries[i].titleId & ((u64) 0x10 << 32)) ? sysKey : zeroKey);
            padInfo.setKeyY = 0;
            padInfo.keyslot = 0x11; // fixedKey crypto
        } else if (info->entries[i].ncchFlag3 == 0x0A) {
            padInfo.keyslot = 0x18; // Secure3 crypto, needs slot0x18KeyX.bin on O3DS
        } else if (info->entries[i].ncchFlag3 == 0x0B) {
            padInfo.keyslot = 0x1B; // Secure4 crypto, needs slot0x1BKeyX.bin
        } else if(info->entries[i].ncchFlag3 >> 8 == 0xDEC0DE) { // magic value to manually specify keyslot
            padInfo.keyslot = info->entries[i].ncchFlag3 & 0x3F;
        } else if (info->entries[i].ncchFlag3) {
            padInfo.keyslot = 0x25; // 7.x crypto
        } else {
            padInfo.keyslot = 0x2C; // standard crypto
        }
        Debug("Using keyslot: %02X", padInfo.keyslot);
        
        if (CreatePad(&padInfo) != 0)
            return 1; // this can't fail anyways
    }

    return 0;
}

u32 SdPadgen(u32 param)
{
    (void) (param); // param is unused here
    SdInfo *info = (SdInfo*) 0x20316000;

    // setup AES key from SD
    SetupSdKeyY0x34(false, NULL);
    
    if (!DebugFileOpen("SDinfo.bin"))
        return 1;
    if (!DebugFileRead(info, 4, 0)) {
        FileClose();
        return 1;
    }
    if (!info->n_entries || info->n_entries > MAX_ENTRIES) {
        FileClose();
        Debug("Bad number of entries!");
        return 1;
    }
    if (!DebugFileRead(info->entries, info->n_entries * sizeof(SdInfoEntry), 4)) {
        FileClose();
        return 1;
    }
    FileClose();
    
    Debug("Number of entries: %i", info->n_entries);
    for(u32 i = 0; i < info->n_entries; i++) {
        PadInfo padInfo = {.keyslot = 0x34, .setKeyY = 0, .size_mb = info->entries[i].size_mb, .mode = AES_CNT_CTRNAND_MODE};
        memcpy(padInfo.ctr, info->entries[i].ctr, 16);
        memcpy(padInfo.filename, info->entries[i].filename, 180);
        Debug ("%2i: %s (%iMB)", i, info->entries[i].filename, info->entries[i].size_mb);
        if (CreatePad(&padInfo) != 0)
            return 1; // this can't fail anyways
    }

    return 0;
}

u32 SdPadgenDirect(u32 param)
{
    (void) (param); // param is unused here
    SdInfo *info = (SdInfo*) 0x20316000;
    char basepath[256];
    u8 movable_keyY[16];
    
    if (SetupSdKeyY0x34(true, movable_keyY) != 0)
        return 1; // movable.sed has to be present in NAND
    
    Debug("");
    if (SdFolderSelector(basepath, movable_keyY) != 0)
        return 1;
    Debug("");
    if (SdInfoGen(info, basepath) != 0)
        return 1;
    if (!info->n_entries) {
        Debug("Nothing found in folder");
        return 1;
    }
    
    Debug("Number of entries: %i", info->n_entries);
    for(u32 i = 0; i < info->n_entries; i++) {
        PadInfo padInfo = {.keyslot = 0x34, .setKeyY = 0, .size_mb = info->entries[i].size_mb, .mode = AES_CNT_CTRNAND_MODE};
        memcpy(padInfo.ctr, info->entries[i].ctr, 16);
        memcpy(padInfo.filename, info->entries[i].filename, 180);
        Debug ("%2i: %s (%iMB)", i, info->entries[i].filename, info->entries[i].size_mb);
        if (CreatePad(&padInfo) != 0)
            return 1; // this can't fail anyways
    }

    return 0;
}

u32 AnyPadgen(u32 param)
{
    (void) (param); // param is unused here
    AnyPadInfo *info = (AnyPadInfo*) 0x20316000;
    
    // get header
    if ((FileGetData("anypad.bin", info, 16, 0) != 16) || !info->n_entries || info->n_entries > MAX_ENTRIES) {
        Debug("Corrupt or not existing: anypad.bin");
        return 1;
    }
    
    // get data
    u32 data_size = info->n_entries * sizeof(AnyPadInfoEntry);
    if (FileGetData("anypad.bin", (u8*) info + 16, data_size, 16) != data_size) {
        Debug("File is missing data: anypad.bin");
        return 1;
    }
    
    Debug("Processing anypad.bin...");
    Debug("Number of entries: %i", info->n_entries);
    for (u32 i = 0; i < info->n_entries; i++) { // this translates all entries to a standard padInfo struct
        AnyPadInfoEntry* entry = &(info->entries[i]);
        PadInfo padInfo = {.keyslot = entry->keyslot, .setKeyY = 0, .size_mb = 0, .size_b = entry->size_b, .mode = entry->mode};
        memcpy(padInfo.filename, entry->filename, 80);
        memcpy(padInfo.ctr, entry->ctr, 16);
        // process keys
        if (entry->setNormalKey)
            setup_aeskey(entry->keyslot, entry->normalKey);
        if (entry->setKeyX)
            setup_aeskeyX(entry->keyslot, entry->keyX);
        if (entry->setKeyY)
            setup_aeskeyY(entry->keyslot, entry->keyY);
        use_aeskey(entry->keyslot);
        // process flags
        if (entry->flags & (AP_USE_NAND_CTR|AP_USE_SD_CTR)) {
            u32 ctr_add = getbe32(padInfo.ctr + 12);
            u8 shasum[32];
            u8 cid[16];
            sdmmc_get_cid((entry->flags & AP_USE_NAND_CTR) ? 1 : 0, (uint32_t*) cid);
            if (entry->mode == AES_CNT_TWLNAND_MODE) {
                sha_quick(shasum, cid, 16, SHA1_MODE);
                for (u32 i = 0; i < 16; i++)
                    padInfo.ctr[i] = shasum[15-i];
            } else {
                sha_quick(shasum, cid, 16, SHA256_MODE);
                memcpy(padInfo.ctr, shasum, 16);
            }
            add_ctr(padInfo.ctr, ctr_add);
        }
        // create the pad
        Debug ("%2i: %s (%ikB)", i, entry->filename, entry->size_b / 1024);
        if (CreatePad(&padInfo) != 0)
            return 1; // this can't fail anyways
    }

    return 0;
}

u32 CtrNandPadgen(u32 param)
{
    char* filename = (param & PG_FORCESLOT4) ? "nand.fat16.slot0x04.xorpad" : "nand.fat16.xorpad";
    u32 keyslot;
    u32 nand_size;

    // legacy sizes & offset, to work with Python 3DSFAT16Tool
    if (GetUnitPlatform() == PLATFORM_3DS) {
        if (param & PG_FORCESLOT4) {
            Debug("This is a N3DS only feature");
            return 1;
        }
        keyslot = 0x4;
        nand_size = 758;
    } else {
        keyslot = (param & PG_FORCESLOT4) ? 0x4 : 0x5;
        nand_size = 1055;
    }

    Debug("Creating NAND FAT16 xorpad. Size (MB): %u", nand_size);
    Debug("Filename: %s", filename);

    PadInfo padInfo = {
        .keyslot = keyslot,
        .setKeyY = 0,
        .size_mb = nand_size,
        .mode = AES_CNT_CTRNAND_MODE
    };
    strncpy(padInfo.filename, filename, 64);
    if(GetNandCtr(padInfo.ctr, 0xB930000) != 0)
        return 1;

    return CreatePad(&padInfo);
}

u32 TwlNandPadgen(u32 param)
{
    (void) (param); // param is unused here
    PartitionInfo* twln_info = GetPartitionInfo(P_TWLN);
    u32 size_mb = (twln_info->size + (1024 * 1024) - 1) / (1024 * 1024);
    Debug("Creating TWLN FAT16 xorpad. Size (MB): %u", size_mb);
    Debug("Filename: twlnand.fat16.xorpad");

    PadInfo padInfo = {
        .keyslot = twln_info->keyslot,
        .setKeyY = 0,
        .size_mb = size_mb,
        .filename = "twlnand.fat16.xorpad",
        .mode = AES_CNT_TWLNAND_MODE
    };
    if(GetNandCtr(padInfo.ctr, twln_info->offset) != 0)
        return 1;

    return CreatePad(&padInfo);
}

u32 Firm0Firm1Padgen(u32 param)
{
    (void) (param); // param is unused here
    PartitionInfo* firm0_info = GetPartitionInfo(P_FIRM0);
    PartitionInfo* firm1_info = GetPartitionInfo(P_FIRM1);
    u32 size_mb = (firm0_info->size + firm1_info->size + (1024 * 1024) - 1) / (1024 * 1024);
    Debug("Creating FIRM0FIRM1 xorpad. Size (MB): %u", size_mb);
    Debug("Filename: firm0firm1.xorpad");

    PadInfo padInfo = {
        .keyslot = firm0_info->keyslot,
        .setKeyY = 0,
        .size_mb = size_mb,
        .filename = "firm0firm1.xorpad",
        .mode = AES_CNT_CTRNAND_MODE
    };
    if(GetNandCtr(padInfo.ctr, firm0_info->offset) != 0)
        return 1;

    return CreatePad(&padInfo);
}
