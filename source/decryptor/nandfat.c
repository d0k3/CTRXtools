#include "fs.h"
#include "draw.h"
#include "platform.h"
#include "decryptor/sha.h"
#include "decryptor/decryptor.h"
#include "decryptor/game.h"
#include "decryptor/nand.h"
#include "decryptor/nandfat.h"

// only a subset, see http://3dbrew.org/wiki/Title_list
// regions: JPN, USA,EUR, CHN, KOR, TWN
TitleListInfo titleList[] = {
    { "System Settings"       , 0x00040010, { 0x00020000, 0x00021000, 0x00022000, 0x00026000, 0x00027000, 0x00028000 } },
    { "Download Play"         , 0x00040010, { 0x00020100, 0x00021100, 0x00022100, 0x00026100, 0x00027100, 0x00028100 } },
    { "Activity Log"          , 0x00040010, { 0x00020200, 0x00021200, 0x00022200, 0x00026200, 0x00027200, 0x00028200 } },
    { "Health&Safety"         , 0x00040010, { 0x00020300, 0x00021300, 0x00022300, 0x00026300, 0x00027300, 0x00028300 } },
    { "Health&Safety (N3DS)"  , 0x00040010, { 0x20020300, 0x20021300, 0x20022300, 0x00000000, 0x00000000, 0x00000000 } },
    { "3DS Camera"            , 0x00040010, { 0x00020400, 0x00021400, 0x00022400, 0x00026400, 0x00027400, 0x00028400 } },
    { "3DS Sound"             , 0x00040010, { 0x00020500, 0x00021500, 0x00022500, 0x00026500, 0x00027500, 0x00028500 } },
    { "Mii Maker"             , 0x00040010, { 0x00020700, 0x00021700, 0x00022700, 0x00026700, 0x00027700, 0x00028700 } },
    { "Streetpass Mii Plaza"  , 0x00040010, { 0x00020800, 0x00021800, 0x00022800, 0x00026800, 0x00027800, 0x00028800 } },
    { "3DS eShop"             , 0x00040010, { 0x00020900, 0x00021900, 0x00022900, 0x00000000, 0x00027900, 0x00028900 } }
};

NandFileInfo fileList[] = {
    { "ticket.db",             "ticket.db",             "DBS        TICKET  DB ",                P_CTRNAND },
    { "title.db",              "title.db",              "DBS        TITLE   DB ",                P_CTRNAND },
    { "import.db",             "import.db",             "DBS        IMPORT  DB ",                P_CTRNAND },
    { "certs.db",              "certs.db",              "DBS        CERTS   DB ",                P_CTRNAND },
    { "SecureInfo_A",          "SecureInfo",            "RW         SYS        SECURE~?   ",     P_CTRNAND },
    { "LocalFriendCodeSeed_B", "LocalFriendCodeSeed",   "RW         SYS        LOCALF~?   ",     P_CTRNAND },
    { "rand_seed",             "rand_seed",             "RW         SYS        RAND_S~?   ",     P_CTRNAND },
    { "movable.sed",           "movable.sed",           "PRIVATE    MOVABLE SED",                P_CTRNAND },
    { "seedsave.bin", "seedsave.bin", "DATA       ???????????SYSDATA    0001000F   00000000   ", P_CTRNAND },
    { "nagsave.bin",  "nagsave.bin",  "DATA       ???????????SYSDATA    0001002C   00000000   ", P_CTRNAND },
    { "nnidsave.bin", "nnidsave.bin", "DATA       ???????????SYSDATA    00010038   00000000   ", P_CTRNAND }
};


NandFileInfo* GetNandFileInfo(u32 file_id)
{
    u32 file_num = 0;
    for(; !(file_id & (1<<file_num)) && (file_num < 32); file_num++);
    return (file_num >= 32) ? NULL : &(fileList[file_num]);
}

u32 SeekFileInNand(u32* offset, u32* size, const char* path, PartitionInfo* partition)
{
    // poor mans NAND FAT file seeker:
    // - path must be in FAT 8+3 format, without dots or slashes
    //   example: DIR1_______DIR2_______FILENAMEEXT
    // - can't handle long filenames
    // - dirs must not exceed 1024 entries
    // - fragmentation not supported
    
    u8* buffer = BUFFER_ADDRESS;
    u32 p_size = partition->size;
    u32 p_offset = partition->offset;
    u32 fat_pos = 0;
    bool found = false;
    
    if (strnlen(path, 256) % (8+3) != 0)
        return 1;
    
    DecryptNandToMem(buffer, p_offset, NAND_SECTOR_SIZE, partition);
    
    // good FAT header description found here: http://www.compuphase.com/mbr_fat.htm
    u32 fat_start = NAND_SECTOR_SIZE * getle16(buffer + 0x0E);
    u32 fat_count = buffer[0x10];
    u32 fat_size = NAND_SECTOR_SIZE * getle16(buffer + 0x16) * fat_count;
    u32 root_size = getle16(buffer + 0x11) * 0x20;
    u32 cluster_start = fat_start + fat_size + root_size;
    u32 cluster_size = buffer[0x0D] * NAND_SECTOR_SIZE;
    
    for (*offset = p_offset + fat_start + fat_size; strnlen(path, 256) >= 8+3; path += 8+3) {
        if (*offset - p_offset > p_size)
            return 1;
        found = false;
        DecryptNandToMem(buffer, *offset, cluster_size, partition);
        for (u32 i = 0x00; i < cluster_size; i += 0x20) {
            const static char zeroes[8+3] = { 0x00 };
            // skip invisible, deleted and lfn entries
            if ((buffer[i] == '.') || (buffer[i] == 0xE5) || (buffer[i+0x0B] == 0x0F))
                continue;
            else if (memcmp(buffer + i, zeroes, 8+3) == 0)
                return 1;
            u32 p; // search for path in fat folder structure, accept '?' wildcards
            for (p = 0; (p < 8+3) && (path[p] == '?' || buffer[i+p] == path[p]); p++);
            if (p != 8+3) continue;
            // entry found, store offset and move on
            fat_pos = getle16(buffer + i + 0x1A);
            *offset = p_offset + cluster_start + (fat_pos - 2) * cluster_size;
            *size = getle32(buffer + i + 0x1C);
            found = true;
            break;
        }
        if (!found) break;
    }
    
    // check for fragmentation
    if (found && (*size > cluster_size)) {  
        if (fat_size / fat_count > 0x100000) // prevent buffer overflow
            return 1; // fishy FAT table size - should never happen
        DecryptNandToMem(buffer, p_offset + fat_start, fat_size / fat_count, partition);
        for (u32 i = 0; i < (*size - 1) / cluster_size; i++) {
            if (*(((u16*) buffer) + fat_pos + i) != fat_pos + i + 1)
                return 1;
        } // no need to check the files last FAT table entry
    }
    
    return (found) ? 0 : 1;
}

u32 DebugSeekFileInNand(u32* offset, u32* size, const char* filename, const char* path, PartitionInfo* partition)
{
    Debug("Searching for %s...", filename);
    if (SeekFileInNand(offset, size, path, partition) != 0) {
        Debug("Failed!");
        return 1;
    }
    if (*size < 1024)
        Debug("Found at %08X, size %ub", *offset, *size);
    else if (*size < 1024 * 1024)
        Debug("Found at %08X, size %ukB", *offset, *size / 1024);
    else
        Debug("Found at %08X, size %uMB", *offset, *size / (1024*1024));
    
    return 0;
}

u32 SeekTitleInNandDb(u32* tid_low, u32* tmd_id, TitleListInfo* title_info)
{
    PartitionInfo* ctrnand_info = GetPartitionInfo(P_CTRNAND);
    u8* titledb = (u8*) 0x20316000;
    
    u32 offset_db;
    u32 size_db;
    if (SeekFileInNand(&offset_db, &size_db, "DBS        TITLE   DB ", ctrnand_info) != 0)
        return 1; // database not found
    if (size_db != 0x64C00)
        return 1; // bad database size
    if (DecryptNandToMem(titledb, offset_db, size_db, ctrnand_info) != 0)
        return 1;
    
    u8* entry_table = titledb + 0x39A80;
    u8* info_data = titledb + 0x44B80;
    if ((getle32(entry_table + 0) != 2) || (getle32(entry_table + 4) != 3))
        return 1; // magic number not found
    *tid_low = 0;
    for (u32 i = 0; i < 1000; i++) {
        u8* entry = entry_table + 0xA8 + (0x2C * i);
        u8* info = info_data + (0x80 * i);
        u32 r;
        if (getle32(entry + 0xC) != title_info->tid_high) continue; // not a title id match
        if (getle32(entry + 0x4) != 1) continue; // not an active entry
        if ((getle32(entry + 0x18) - i != 0x162) || (getle32(entry + 0x1C) != 0x80) || (getle32(info + 0x08) != 0x40)) continue; // fishy title info / offset
        for (r = 0; r < 6; r++) {
            if ((title_info->tid_low[r] != 0) && (getle32(entry + 0x8) == title_info->tid_low[r])) break;
        }
        if (r >= 6) continue;
        *tmd_id = getle32(info + 0x14);
        *tid_low = title_info->tid_low[r];
        break; 
    }
    
    return (*tid_low) ? 0 : 1;
}

u32 DebugSeekTitleInNand(u32* offset_tmd, u32* size_tmd, u32* offset_app, u32* size_app, TitleListInfo* title_info, u32 max_cnt)
{
    PartitionInfo* ctrnand_info = GetPartitionInfo(P_CTRNAND);
    u8* buffer = (u8*) 0x20316000;
    u32 cnt_count = 0;
    u32 tid_low = 0;
    u32 tmd_id = 0;
    
    Debug("Searching title \"%s\"...", title_info->name);
    Debug("Method 1: Search in title.db...");
    if (SeekTitleInNandDb(&tid_low, &tmd_id, title_info) == 0) {
        char path[64];
        sprintf(path, "TITLE      %08X   %08X   CONTENT    %08XTMD", (unsigned int) title_info->tid_high, (unsigned int) tid_low, (unsigned int) tmd_id);
        if (SeekFileInNand(offset_tmd, size_tmd, path, ctrnand_info) != 0)
            tid_low = 0;
    }
    if (!tid_low) {
        Debug("Method 2: Search in file system...");
        for (u32 i = 0; i < 6; i++) {
            char path[64];
            if (title_info->tid_low[i] == 0)
                continue;
            sprintf(path, "TITLE      %08X   %08X   CONTENT    ????????TMD", (unsigned int) title_info->tid_high, (unsigned int) title_info->tid_low[i]);
            if (SeekFileInNand(offset_tmd, size_tmd, path, ctrnand_info) == 0) {
                tid_low = title_info->tid_low[i];
                break;
            }
        }
    }
    if (!tid_low) {
        Debug("Failed!");
        return 1;
    }
    Debug("Found title %08X%08X", title_info->tid_high, tid_low);
    
    Debug("TMD0 found at %08X, size %ub", *offset_tmd, *size_tmd);
    if ((*size_tmd < 0xC4 + (0x40 * 0x24)) || (*size_tmd > 0x4000)) {
        Debug("TMD has bad size!");
        return 1;
    }
    if (DecryptNandToMem(buffer, *offset_tmd, *size_tmd, ctrnand_info) != 0)
        return 1;
    u32 size_sig = (buffer[3] == 3) ? 0x240 : (buffer[3] == 4) ? 0x140 : (buffer[3] == 5) ? 0x80 : 0;         
    if ((size_sig == 0) || (memcmp(buffer, "\x00\x01\x00", 3) != 0)) {
        Debug("Unknown signature type: %08X", getbe32(buffer));
        return 1;
    }
    cnt_count = getbe16(buffer + size_sig + 0x9E);
    u32 size_tmd_expected = size_sig + 0xC4 + (0x40 * 0x24) + (cnt_count * 0x30);
    if (*size_tmd < size_tmd_expected) {
        Debug("TMD bad size (expected %ub)!", size_tmd_expected );
        return 1;
    }
    buffer += size_sig + 0xC4 + (0x40 * 0x24);
    
    for (u32 i = 0; i < cnt_count && i < max_cnt; i++) {
        char path[64];
        u32 cnt_id = getbe32(buffer + (0x30 * i));
        if (i >= max_cnt) {
            Debug("APP%i was skipped", i);
            continue;
        }
        sprintf(path, "TITLE      %08X   %08X   CONTENT    %08XAPP", (unsigned int) title_info->tid_high, (unsigned int) tid_low, (unsigned int) cnt_id);
        if (SeekFileInNand(offset_app + i, size_app + i, path, ctrnand_info) != 0) {
            Debug("APP%i not found or fragmented!", i);
            return 1;
        }
        Debug("APP%i found at %08X, size %ukB", i, offset_app[i], size_app[i] / 1024);
    }
    
    return 0;
}

u32 DumpFile(u32 param)
{
    NandFileInfo* f_info = GetNandFileInfo(param);
    PartitionInfo* p_info = GetPartitionInfo(f_info->partition_id);
    char filename[64];
    u32 offset;
    u32 size;
    
    if (DebugSeekFileInNand(&offset, &size, f_info->name_l, f_info->path, p_info) != 0)
        return 1;
    if (OutputFileNameSelector(filename, f_info->name_l, NULL, (param & N_EMUNAND)) != 0)
        return 1;
    if (DecryptNandToFile(filename, offset, size, p_info) != 0)
        return 1;
    
    return 0;
}

u32 InjectFile(u32 param)
{
    NandFileInfo* f_info = GetNandFileInfo(param);
    PartitionInfo* p_info = GetPartitionInfo(f_info->partition_id);
    char filename[64];
    u32 offset;
    u32 size;
    
    if (!(param & N_NANDWRITE)) // developer screwup protection
        return 1;
    
    if (DebugSeekFileInNand(&offset, &size, f_info->name_l, f_info->path, p_info) != 0)
        return 1;
    if (InputFileNameSelector(filename, f_info->name_s, NULL, NULL, 0, size) != 0)
        return 1;
    if (EncryptFileToNand(filename, offset, size, p_info) != 0)
        return 1;
    
    return 0;
}

u32 DumpHealthAndSafety(u32 param)
{
    PartitionInfo* ctrnand_info = GetPartitionInfo(P_CTRNAND);
    TitleListInfo* health = titleList + ((GetUnitPlatform() == PLATFORM_3DS) ? 3 : 4);
    char filename[64];
    u32 offset_app[4];
    u32 size_app[4];
    u32 offset_tmd;
    u32 size_tmd;
    
    
    if (DebugSeekTitleInNand(&offset_tmd, &size_tmd, offset_app, size_app, health, 4) != 0)
        return 1;
    if (OutputFileNameSelector(filename, "hs.app", NULL, (param & N_EMUNAND)) != 0)
        return 1;
        
    Debug("Dumping & decrypting APP0...");
    if (DecryptNandToFile(filename, offset_app[0], size_app[0], ctrnand_info) != 0)
        return 1;
    if (CryptNcch(filename, 0, 0, 0, NULL) != 0)
        return 1;
        
     return 0;
}

u32 InjectHealthAndSafety(u32 param)
{
    u8* buffer = BUFFER_ADDRESS;
    PartitionInfo* ctrnand_info = GetPartitionInfo(P_CTRNAND);
    TitleListInfo* health = titleList + ((GetUnitPlatform() == PLATFORM_3DS) ? 3 : 4);
    NcchHeader* ncch = (NcchHeader*) 0x20316000;
    char filename[64];
    u32 offset_app[4];
    u32 size_app[4];
    u32 offset_tmd;
    u32 size_tmd;
    u32 size_hs;
    
    
    if (!(param & N_NANDWRITE)) // developer screwup protection
        return 1;
    
    if (DebugSeekTitleInNand(&offset_tmd, &size_tmd, offset_app, size_app, health, 4) != 0)
        return 1;
    if (size_app[0] > 0x400000) {
        Debug("H&S system app is too big!");
        return 1;
    }
    if (DecryptNandToMem((void*) ncch, offset_app[0], 0x200, ctrnand_info) != 0)
        return 1;
    if (InputFileNameSelector(filename, NULL, "app", ncch->signature, 0x100, 0) != 0)
        return 1;
    
    if (!DebugFileOpen(filename))
        return 1;
    size_hs = FileGetSize();
    memset(buffer, 0, size_app[0]);
    if (size_hs > size_app[0]) {
        Debug("H&S inject app is too big!");
        return 1;
    }
    if (!DebugFileRead(buffer, size_hs, 0)) {
        FileClose();
        return 1;
    }
    FileClose();
    if (!DebugFileCreate("hs.enc", true))
        return 1;
    if (!DebugFileWrite(buffer, size_app[0], 0)) {
        FileClose();
        return 1;
    }
    FileClose();
    if (CryptNcch("hs.enc", 0, 0, 0, ncch->flags) != 0)
        return 1;
    
    Debug("Injecting H&S app...");
    if (EncryptFileToNand("hs.enc", offset_app[0], size_app[0], ctrnand_info) != 0)
        return 1;
    
    Debug("Fixing TMD...");
    u8* tmd_data = (u8*) 0x20316000;
    if (DecryptNandToMem(tmd_data, offset_tmd, size_tmd, ctrnand_info) != 0)
        return 1; 
    tmd_data += (tmd_data[3] == 3) ? 0x240 : (tmd_data[3] == 4) ? 0x140 : 0x80;
    u8* content_list = tmd_data + 0xC4 + (64 * 0x24);
    u32 cnt_count = getbe16(tmd_data + 0x9E);
    if (GetHashFromFile("hs.enc", 0, size_app[0], content_list + 0x10) != 0) {
        Debug("Failed!");
        return 1;
    }
    for (u32 i = 0, kc = 0; i < 64 && kc < cnt_count; i++) {
        u32 k = getbe16(tmd_data + 0xC4 + (i * 0x24) + 0x02);
        u8* chunk_hash = tmd_data + 0xC4 + (i * 0x24) + 0x04;
        sha_init(SHA256_MODE);
        sha_update(content_list + kc * 0x30, k * 0x30);
        sha_get(chunk_hash);
        kc += k;
    }
    u8* tmd_hash = tmd_data + 0xA4;
    sha_init(SHA256_MODE);
    sha_update(tmd_data + 0xC4, 64 * 0x24);
    sha_get(tmd_hash);
    tmd_data = (u8*) 0x20316000;
    if (EncryptMemToNand(tmd_data, offset_tmd, size_tmd, ctrnand_info) != 0)
        return 1; 
    
    
    return 0;
}
