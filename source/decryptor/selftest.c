#include "fs.h"
#include "draw.h"
#include "platform.h"
#include "decryptor/aes.h"
#include "decryptor/sha.h"
#include "decryptor/selftest.h"
#include "decryptor/decryptor.h"
#include "decryptor/keys.h"
#include "decryptor/nandfat.h"
#include "decryptor/titlekey.h"
#include "fatfs/sdmmc.h"

// Selftest subtest defines
#define ST_NAND_CID_HARD    1
#define ST_NAND_CID_MEM     2
#define ST_SHA              3
#define ST_AES_MODE         4
#define ST_AES_KEYSLOT      5
#define ST_AES_KEYSLOT_Y    6
#define ST_TITLEKEYS        7

typedef struct {
    char name[16];
    u32 size;
    u32 type;
    u32 param;
} SubTestInfo;

SubTestInfo TestList[] = {
    { "nand_cid_hard", 16, ST_NAND_CID_HARD, 0 },
    { "nand_cid_mem", 16, ST_NAND_CID_MEM, 0 },
    { "sha256", 32, ST_SHA, SHA256_MODE },
    { "sha1", 20, ST_SHA, SHA1_MODE },
    { "aes_cnt_ctr", 16, ST_AES_MODE, AES_CNT_CTRNAND_MODE },
    { "aes_cnt_twl", 16, ST_AES_MODE, AES_CNT_TWLNAND_MODE },
    { "aes_ttk_enc", 16, ST_AES_MODE, AES_CNT_TITLEKEY_DECRYPT_MODE },
    { "aes_ttk_dec", 16, ST_AES_MODE, AES_CNT_TITLEKEY_ENCRYPT_MODE },
    { "ncch_std_key", 16, ST_AES_KEYSLOT_Y, 0x2C },
    { "ncch_7x_key", 16, ST_AES_KEYSLOT_Y, 0x25 },
    { "ncch_sec3_key", 16, ST_AES_KEYSLOT_Y, 0x18 },
    { "ncch_sec4_key", 16, ST_AES_KEYSLOT_Y, 0x1B },
    { "nand_twl_key", 16, ST_AES_KEYSLOT, 0x03 },
    { "nand_ctro_key", 16, ST_AES_KEYSLOT, 0x04 },
    { "nand_ctrn_key", 16, ST_AES_KEYSLOT, 0x05 },
    { "nand_agb_key", 16, ST_AES_KEYSLOT, 0x06 },
    { "nand_frm_key", 16, ST_AES_KEYSLOT, 0x07 },
    { "boss_key", 16, ST_AES_KEYSLOT, 0x38 },
    { "sd_key", 16, ST_AES_KEYSLOT_Y, 0x34 },
    { "titlekey", 6*16, ST_TITLEKEYS, 0 },
};

u32 SelfTest(u32 param)
{
    u8* test_data = (u8*) 0x20316000;
    const u8 teststr[16] = { 'D', '9', ' ', 'S', 'E', 'L', 'F', 'T', 'E', 'S', 'T', ' ', ' ', ' ', ' ' };
    const u8 zeroes[16] = { 0x00 };
    bool selftest = !(param & ST_REFERENCE);
    
    // check keyslots
    Debug("Checking keyslots...");
    Debug("0x05 KeyY: %s", (CheckKeySlot(0x05, 'Y') == 0) ? "set up" : "not set up");
    Debug("0x25 KeyX: %s", (CheckKeySlot(0x25, 'X') == 0) ? "set up" : "not set up");
    Debug("0x18 KeyX: %s", (CheckKeySlot(0x18, 'X') == 0) ? "set up" : "not set up");
    Debug("0x1B KeyX: %s", (CheckKeySlot(0x1B, 'X') == 0) ? "set up" : "not set up");
    Debug("");
    
    Debug((selftest) ? "Running selftest..." : "Creating selftest reference data...");
    
    // process all subtests
    u32 num_tests = sizeof(TestList) / sizeof(SubTestInfo);
    u8* test_ptr = test_data;
    u32 fsize_test = 0;
    for (u32 i = 0; i < num_tests; i++) {
        u32 size = TestList[i].size;
        u32 size_a = align(size, 16);
        u32 type = TestList[i].type;
        u32 tparam = TestList[i].param;
        
        memset(test_ptr, 0x00, 16 + size_a);
        strncpy((char*) test_ptr, TestList[i].name, 16);
        test_ptr += 16;
        
        if (type == ST_NAND_CID_HARD) {
            sdmmc_get_cid(1, (uint32_t*) test_ptr);
        } else if (type == ST_NAND_CID_MEM) {
            memcpy(test_ptr, (void*) 0x01FFCD84, 16);
        } else if (type == ST_SHA) {
            sha_quick(test_ptr, teststr, 16, tparam);
        } else if ((type == ST_AES_MODE) || (type == ST_AES_KEYSLOT) || (type == ST_AES_KEYSLOT_Y)) {
            CryptBufferInfo info = {.setKeyY = 0, .size = 16, .buffer = test_ptr};
            if (type == ST_AES_MODE) {
                info.mode = tparam;
                info.keyslot = 0x11;
                setup_aeskey(0x11, (void*) zeroes);
            } else {
                if (type == ST_AES_KEYSLOT_Y) {
                    info.setKeyY = 1;
                    memcpy(info.keyY, zeroes, 16);
                }
                info.mode = AES_CNT_CTRNAND_MODE;
                info.keyslot = tparam;
            }
            memset(info.ctr, 0x00, 16);
            memcpy(test_ptr, teststr, 16);
            CryptBuffer(&info);
        } else if (type == ST_TITLEKEYS) {
            TitleKeyEntry titlekey;
            memset(&titlekey, 0x00, sizeof(TitleKeyEntry));
            for (titlekey.commonKeyIndex = 0; titlekey.commonKeyIndex < 6; titlekey.commonKeyIndex++) {
                memset(titlekey.titleId, 0x00, 8);
                memset(titlekey.titleKey, 0x00, 16);
                CryptTitlekey(&titlekey, false);
                memcpy(test_ptr + (titlekey.commonKeyIndex * 16), titlekey.titleKey, 16);
            }     
        }
        
        test_ptr += size_a;
        fsize_test += 16 + size_a;
    }
    
    // run actual self test
    char filename[32];
    snprintf(filename, 31, "d9_selftest.ref");
    if (selftest) {
        u8* ref_ptr = test_data + fsize_test;
        if (FileGetData(filename, ref_ptr, fsize_test, 0) != fsize_test) {
            Debug("No valid reference data available!");
            return 1;
        }
        for (u32 chk = 0; chk < 2; chk++) {
            u32 count = 0;
            Debug("");
            Debug((chk) ? "Failed tests:" : "Passed tests:");
            test_ptr = test_data;
            ref_ptr = test_data + fsize_test;
            for (u32 i = 0; i < num_tests; i++) {
                u32 size = TestList[i].size;
                u32 size_a = align(size, 16);
                if (chk && (strncmp((char*) test_ptr, (char*) ref_ptr, 16) != 0)) {
                    Debug("%s (bad ref data)", TestList[i].name);
                    count++;
                    test_ptr += 16 + size_a;
                    ref_ptr += 16 + size_a;
                    continue;
                }
                test_ptr += 16;
                ref_ptr += 16;
                if ((!chk && memcmp(test_ptr, ref_ptr, size) == 0) || (chk && memcmp(test_ptr, ref_ptr, size) != 0)) {
                    Debug(TestList[i].name);
                    count++;
                }
                test_ptr += size_a;
                ref_ptr += size_a;
            }
            Debug("%u of %u tests %s", count, num_tests, (chk) ? "failed" : "passed");
        }
        snprintf(filename, 31, "d9_selftest.lst");
    }
    
    // write test data to file
    if (FileDumpData(filename, test_data, fsize_test) != fsize_test) {
        Debug("");
        Debug("Error writing test data");
        return 1;
    }
    
    return 0;
}


u32 SystemInfo(u32 param)
{
    // this is not intended to be run from EmuNAND, uses various info from 3dbrew
    // see: https://3dbrew.org/wiki/Nandrw/sys/SecureInfo_A
    // see: https://3dbrew.org/wiki/Nand/private/movable.sed
    // see: https://www.3dbrew.org/wiki/Memory_layout#ARM9_ITCM 
    (void) (param); // param is unused here
    const char* emunandstr[] = { "not ready", "not set up", "GW EmuNAND", "RedNAND" };
    const char* regionstr[] = { "JPN", "USA", "EUR", "AUS", "CHN", "KOR", "TWN", "UNK" };
    PartitionInfo* ctrnand_info = GetPartitionInfo(P_CTRNAND);
    bool isDevkit = (GetUnitKeysType() == KEYS_DEVKIT);
    bool isN3ds = (GetUnitPlatform() == PLATFORM_N3DS);
    bool isA9lh = ((*(u32*) 0x101401C0) == 0);
    bool isSighax = (!((*(vu8*)0x10000000) & 0x2)) && isA9lh;
    char sd_base_id0[64]; // fill this later
    char sd_base_id1[64]; // fill this later
    u32 key_state = (!CheckKeySlot(0x05, 'Y') << 3) | (!CheckKeySlot(0x25, 'X') << 2) |
        (!CheckKeySlot(0x18, 'X') << 1) | (!CheckKeySlot(0x1B, 'X') << 0);
    u64 nand_size = (u64) getMMCDevice(0)->total_size * 0x200;
    u64 sd_size_total = (u64) getMMCDevice(1)->total_size * 0x200;
    u64 sd_size_fat = TotalStorageSpace();
    u64 sd_size_fat_free = RemainingStorageSpace();
    u64 sd_size_hidden = NumHiddenSectors() * 0x200;
    u32 emunand_state = CheckEmuNand();
    u8* secureInfo = (u8*) 0x20316000;
    u8* serial = secureInfo + 0x102;
    u8* region = secureInfo + 0x100;
    u8* movable = (u8*) 0x20316000 + 0x200;
    u8* slot0x34keyY = movable + 0x110;
    u8* nandcid = (u8*) 0x20316000 + 0x400;
    u8* sdcid = (u8*) 0x20316000 + 0x410;
    u8* twlcustid = (u8*) 0x01FFB808;
    u8* mfg_date = (u8*) 0x01FFB81A;
    
    // Get NAND / SD CID
    sdmmc_get_cid(1, (uint32_t*) nandcid);
    sdmmc_get_cid(0, (uint32_t*) sdcid);
    
    // get data from secureInfo_A & movable_sed
    u32 offset, size;
    if ((SeekFileInNand(&offset, &size, "RW         SYS        SECURE~?   ", ctrnand_info) != 0) ||
        (DecryptNandToMem(secureInfo, offset, size, ctrnand_info) != 0)) {
        Debug("SecureInfo_A not found!");
        return 1; // this should never happen
    }
    if ((SeekFileInNand(&offset, &size, "PRIVATE    MOVABLE SED", ctrnand_info) != 0) ||
        (DecryptNandToMem(movable, offset, size, ctrnand_info) != 0)) {
        Debug("movable.sed not found!");
        return 1; // this should never happen
    }
    
    // build base path <id0> / <id1>
    unsigned int sha256sum[8];
    sha_quick(sha256sum, slot0x34keyY, 16, SHA256_MODE);
    snprintf(sd_base_id0, 63, "%08X%08X%08X%08X", sha256sum[0], sha256sum[1], sha256sum[2], sha256sum[3]);
    snprintf(sd_base_id1, 63, "%08X%08X%08X%08X", (unsigned int) getle32(sdcid+0), (unsigned int) getle32(sdcid+4),
        (unsigned int) getle32(sdcid+8), (unsigned int) getle32(sdcid+12));
    
    // NAND stuff output here
    Debug("Mfg. Date: %u/%02u/%02u", *(MFDATE) + 1900, *(MFDATE + 1), *(MFDATE + 2)); //the next 3 bytes are hours, minutes, seconds but those were ommitted due to being superfluous info
    Debug("NAND type / size: %s %s / %lluMB", (isDevkit) ? "Devkit" : "Retail", (isN3ds) ? "N3DS" : "O3DS", nand_size / 0x100000);
    Debug("Serial / region: %.15s / %s", (char*) serial, (*region < 7) ? regionstr[*region] : regionstr[7]);
    Debug("Manufacturing date: %u/%02u/%02u", *(mfg_date) + 1900, *(mfg_date + 1), *(mfg_date + 2));
    // the next 3 bytes are hours, minutes, seconds but those were ommitted due to being superfluous info
    Debug("NAND CID: %08X%08X%08X%08X", getbe32(nandcid+0), getbe32(nandcid+4), getbe32(nandcid+8), getbe32(nandcid+12));
    Debug("TWL customer ID: %08X%08X", getbe32(twlcustid+0), getbe32(twlcustid+4));
    Debug("SysNAND SD path <id0> / <id1>:");
    Debug(sd_base_id0);
    Debug(sd_base_id1);
    Debug("");
    
    // current setup stuff here
    Debug("Running from hax: %s", (isSighax) ? "sighax" : (isA9lh) ? "a9lh" : "no");
    Debug("Keys set:%s%s%s%s", (!key_state) ? " none" : (key_state & 0x8) ? " 0x05Y" : "",
        (key_state & 0x4) ? " 0x25X" : "", (key_state & 0x2) ? " 0x18X" : "", (key_state & 0x1) ? " 0x1BX" : "");
    Debug("");
    
    // SD stuff output here
    Debug("SD size hidden / total: %lluMB / %lluMB", sd_size_hidden / 0x100000, sd_size_total / 0x100000);
    Debug("SD FAT free / total: %lluMB / %lluMB", sd_size_fat_free / 0x100000, sd_size_fat / 0x100000);
    Debug("SD CID: %08X%08X%08X%08X", getbe32(sdcid+0), getbe32(sdcid+4), getbe32(sdcid+8), getbe32(sdcid+12));
    if ((emunand_state > 0) && (emunand_state <= 3)) {
        Debug("Installed EmuNAND: %s", emunandstr[emunand_state]);
    } else if (emunand_state > 3) {
        for (u32 i = 0; emunand_state; i++, emunand_state >>= 2)
            Debug("Installed EmuNAND #%u: %s", i, emunandstr[emunand_state&0x3]);
    }
    Debug("");
    
    return 0;
}
