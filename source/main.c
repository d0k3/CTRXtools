#include "common.h"
#include "draw.h"
#include "fs.h"
#include "hid.h"
#include "menu.h"
#include "platform.h"
#include "i2c.h"
#include "decryptor/keys.h"
#include "decryptor/game.h"
#include "decryptor/nand.h"
#include "decryptor/nandfat.h"
#include "decryptor/titlekey.h"
#include "decryptor/selftest.h"
#include "decryptor/xorpad.h"

#define SUBMENU_START 7

MenuInfo menu[] =
{
    {
        "XORpad Generator Options", 9,
        {
            { "NCCH Padgen",                  &NcchPadgen,            0 },
            { "SD Padgen (SDinfo.bin)",       &SdPadgen,              0 },
            { "SD Padgen (SysNAND dir)",      &SdPadgenDirect,        0 },
            { "SD Padgen (EmuNAND dir)",      &SdPadgenDirect,        N_EMUNAND },
            { "Any Padgen (anypad.bin)",      &AnyPadgen,             0 },
            { "CTRNAND Padgen",               &CtrNandPadgen,         0 },
            { "CTRNAND Padgen (slot0x4)",     &CtrNandPadgen,         PG_FORCESLOT4 },
            { "TWLNAND Padgen",               &TwlNandPadgen,         0 },
            { "FIRM0FIRM1 Padgen",            &Firm0Firm1Padgen,      0 }
        }
    },
    {
        "Titlekey Options", 6,
        {
            { "Titlekey Decrypt (file)",      &CryptTitlekeysFile,    0 },
            { "Titlekey Decrypt (SysNAND)",   &DumpTitlekeysNand,     0 },
            { "Titlekey Decrypt (EmuNAND)",   &DumpTitlekeysNand,     N_EMUNAND },
            { "Titlekey Encrypt (file)",      &CryptTitlekeysFile,    TK_ENCRYPTED },
            { "Titlekey Dump (SysNAND)",      &DumpTitlekeysNand,     TK_ENCRYPTED },
            { "Titlekey Dump (EmuNAND)",      &DumpTitlekeysNand,     N_EMUNAND | TK_ENCRYPTED }
        }
    },
    {
        "SysNAND Options", 6,
        {
            { "SysNAND Backup/Restore...",    NULL,                   SUBMENU_START +  0 },
            { "Partition Dump...",            NULL,                   SUBMENU_START +  2 },
            { "Partition Inject...",          NULL,                   SUBMENU_START +  4 },
            { "File Dump...",                 NULL,                   SUBMENU_START +  6 },
            { "File Inject...",               NULL,                   SUBMENU_START +  8 },
            { "Miscellaneous...",             NULL,                   SUBMENU_START + 10 }
        }
    },
    {
        "EmuNAND Options", 6,
        {
            { "EmuNAND Backup/Restore...",    NULL,                   SUBMENU_START +  1 },
            { "Partition Dump...",            NULL,                   SUBMENU_START +  3 },
            { "Partition Inject...",          NULL,                   SUBMENU_START +  5 },
            { "File Dump...",                 NULL,                   SUBMENU_START +  7 },
            { "File Inject...",               NULL,                   SUBMENU_START +  9 },
            { "Miscellaneous...",             NULL,                   SUBMENU_START + 11 }
        }
    },
    {
        "Content Decryptor Options", 4,
        {
            { "NCCH/NCSD File Options...",    NULL,                   SUBMENU_START + 12 },
            { "CIA File Options...",          NULL,                   SUBMENU_START + 13 },
            { "BOSS File Options...",         NULL,                   SUBMENU_START + 14 },
            { "SD File Options...",           NULL,                   SUBMENU_START + 15 }
        }
    },
    {
        "Gamecart Dumper Options", 6,
        {
            { "Dump Cart (full)",             &DumpGameCart,          0 },
            { "Dump Cart (trim)",             &DumpGameCart,          CD_TRIM },
            { "Dump & Decrypt Cart (full)",   &DumpGameCart,          CD_DECRYPT },
            { "Dump & Decrypt Cart (trim)",   &DumpGameCart,          CD_DECRYPT | CD_TRIM },
            { "Dump Cart to CIA",             &DumpGameCart,          CD_DECRYPT | CD_MAKECIA },
            { "Dump Private Header",          &DumpPrivateHeader,     0 }
        }
    },
    {
        "Maintenance Options", 5,
        {
            { "System Info",                  &SystemInfo,            0 },
            { "Create Selftest Reference",    &SelfTest,              ST_REFERENCE },
            { "Run Selftest",                 &SelfTest,              0 },
            { "Build Key Database",           &BuildKeyDb,            KEY_ENCRYPT },
            { "De/Encrypt Key Database",      &CryptKeyDb,            0 }
        }
    },
    // everything below is not contained in the main menu
    {
        "SysNAND Backup/Restore Options", 6, // ID 0
        {
            { "NAND Backup",                  &DumpNand,              0 },
            { "NAND Backup (min size)",       &DumpNand,              NB_MINSIZE },
            { "NAND Restore",                 &RestoreNand,           N_NANDWRITE | N_A9LHWRITE },
            { "NAND Restore (forced)",        &RestoreNand,           N_NANDWRITE | N_A9LHWRITE | NR_NOCHECKS },
            { "NAND Restore (keep a9lh)",     &RestoreNand,           N_NANDWRITE | NR_KEEPA9LH },
            { "Validate NAND Dump",           &ValidateNandDump,      0 }
        }
    },
    {
        "EmuNAND Backup/Restore Options", 5, // ID 1
        {
            { "NAND Backup",                  &DumpNand,              N_EMUNAND },
            { "NAND Backup (min size)",       &DumpNand,              N_EMUNAND | NB_MINSIZE },
            { "NAND Restore",                 &RestoreNand,           N_NANDWRITE | N_EMUNAND | N_FORCEEMU },
            { "NAND Restore (forced)",        &RestoreNand,           N_NANDWRITE | N_EMUNAND | N_FORCEEMU | NR_NOCHECKS },
            { "Validate NAND Dump",           &ValidateNandDump,      0 } // same as the one in SysNAND backup & restore
        }
    },
    {
        "Partition Dump... (SysNAND)", 8, // ID 2
        {
            { "Dump TWLN Partition",          &DecryptNandPartition, P_TWLN },
            { "Dump TWLP Partition",          &DecryptNandPartition, P_TWLP },
            { "Dump AGBSAVE Partition",       &DecryptNandPartition, P_AGBSAVE },
            { "Dump FIRM0 Partition",         &DecryptNandPartition, P_FIRM0 },
            { "Dump FIRM1 Partition",         &DecryptNandPartition, P_FIRM1 },
            { "Dump CTRNAND Partition",       &DecryptNandPartition, P_CTRNAND },
            { "Dump Sector 0x96",             &DecryptSector0x96,    0 },
            { "Dump NAND Header",             &DumpNandHeader,       0 }
        }
    },
    {
        "Partition Dump...(EmuNAND)", 8, // ID 3
        {
            { "Dump TWLN Partition",          &DecryptNandPartition, N_EMUNAND | P_TWLN },
            { "Dump TWLP Partition",          &DecryptNandPartition, N_EMUNAND | P_TWLP },
            { "Dump AGBSAVE Partition",       &DecryptNandPartition, N_EMUNAND | P_AGBSAVE },
            { "Dump FIRM0 Partition",         &DecryptNandPartition, N_EMUNAND | P_FIRM0 },
            { "Dump FIRM1 Partition",         &DecryptNandPartition, N_EMUNAND | P_FIRM1 },
            { "Dump CTRNAND Partition",       &DecryptNandPartition, N_EMUNAND | P_CTRNAND },
            { "Dump Sector 0x96",             &DecryptSector0x96,    N_EMUNAND },
            { "Dump NAND Header",             &DumpNandHeader,       N_EMUNAND }
        }
    },
    {
        "Partition Inject... (SysNAND)", 8, // ID 4
        {
            { "Inject TWLN Partition",        &InjectNandPartition, N_NANDWRITE | P_TWLN },
            { "Inject TWLP Partition",        &InjectNandPartition, N_NANDWRITE | P_TWLP },
            { "Inject AGBSAVE Partition",     &InjectNandPartition, N_NANDWRITE | P_AGBSAVE },
            { "Inject FIRM0 Partition",       &InjectNandPartition, N_NANDWRITE | N_A9LHWRITE | P_FIRM0 },
            { "Inject FIRM1 Partition",       &InjectNandPartition, N_NANDWRITE | N_A9LHWRITE | P_FIRM1 },
            { "Inject CTRNAND Partition",     &InjectNandPartition, N_NANDWRITE | P_CTRNAND },
            { "Inject Sector 0x96",           &InjectSector0x96,    N_NANDWRITE | N_A9LHWRITE },
            { "Restore NAND Header",          &RestoreNandHeader,   N_NANDWRITE | N_A9LHWRITE }
        }
    },
    {
        "Partition Inject... (EmuNAND)", 8, // ID 5
        {
            { "Inject TWLN Partition",        &InjectNandPartition, N_NANDWRITE | N_EMUNAND | P_TWLN },
            { "Inject TWLP Partition",        &InjectNandPartition, N_NANDWRITE | N_EMUNAND | P_TWLP },
            { "Inject AGBSAVE Partition",     &InjectNandPartition, N_NANDWRITE | N_EMUNAND | P_AGBSAVE },
            { "Inject FIRM0 Partition",       &InjectNandPartition, N_NANDWRITE | N_EMUNAND | P_FIRM0 },
            { "Inject FIRM1 Partition",       &InjectNandPartition, N_NANDWRITE | N_EMUNAND | P_FIRM1 },
            { "Inject CTRNAND Partition",     &InjectNandPartition, N_NANDWRITE | N_EMUNAND | P_CTRNAND },
            { "Inject Sector0x96",            &InjectSector0x96,    N_NANDWRITE | N_EMUNAND },
            { "Restore NAND Header",          &RestoreNandHeader,   N_NANDWRITE | N_EMUNAND }
        }
    },
    {
        "File Dump... (SysNAND)", 11, // ID 6
        {
            { "Dump ticket.db",               &DumpFile,             F_TICKET },
            { "Dump title.db",                &DumpFile,             F_TITLE },
            { "Dump import.db",               &DumpFile,             F_IMPORT },
            { "Dump certs.db",                &DumpFile,             F_CERTS },
            { "Dump SecureInfo_A",            &DumpFile,             F_SECUREINFO },
            { "Dump LocalFriendCodeSeed_B",   &DumpFile,             F_LOCALFRIEND },
            { "Dump movable.sed",             &DumpFile,             F_MOVABLE },
            { "Dump seedsave.bin",            &DumpFile,             F_SEEDSAVE },
            { "Dump nagsave.bin",             &DumpFile,             F_NAGSAVE },
            { "Dump nnidsave.bin",            &DumpFile,             F_NNIDSAVE },
            { "Dump friendsave.bin",          &DumpFile,             F_FRIENDSAVE }
        }
    },
    {
        "File Dump... (EmuNAND)", 11, // ID 7
        {
            { "Dump ticket.db",               &DumpFile,             N_EMUNAND | F_TICKET },
            { "Dump title.db",                &DumpFile,             N_EMUNAND | F_TITLE },
            { "Dump import.db",               &DumpFile,             N_EMUNAND | F_IMPORT },
            { "Dump certs.db",                &DumpFile,             N_EMUNAND | F_CERTS },
            { "Dump SecureInfo_A",            &DumpFile,             N_EMUNAND | F_SECUREINFO },
            { "Dump LocalFriendCodeSeed_B",   &DumpFile,             N_EMUNAND | F_LOCALFRIEND },
            { "Dump movable.sed",             &DumpFile,             N_EMUNAND | F_MOVABLE },
            { "Dump seedsave.bin",            &DumpFile,             N_EMUNAND | F_SEEDSAVE },
            { "Dump nagsave.bin",             &DumpFile,             N_EMUNAND | F_NAGSAVE },
            { "Dump nnidsave.bin",            &DumpFile,             N_EMUNAND | F_NNIDSAVE },
            { "Dump friendsave.bin",          &DumpFile,             N_EMUNAND | F_FRIENDSAVE }
        }
    },
    {
        "File Inject... (SysNAND)", 11, // ID 8
        {
            { "Inject ticket.db",             &InjectFile,           N_NANDWRITE | F_TICKET },
            { "Inject title.db",              &InjectFile,           N_NANDWRITE | F_TITLE },
            { "Inject import.db",             &InjectFile,           N_NANDWRITE | F_IMPORT },
            { "Inject certs.db",              &InjectFile,           N_NANDWRITE | F_CERTS },
            { "Inject SecureInfo_A",          &InjectFile,           N_NANDWRITE | F_SECUREINFO },
            { "Inject LocalFriendCodeSeed_B", &InjectFile,           N_NANDWRITE | F_LOCALFRIEND },
            { "Inject movable.sed",           &InjectFile,           N_NANDWRITE | F_MOVABLE },
            { "Inject seedsave.bin",          &InjectFile,           N_NANDWRITE | F_SEEDSAVE },
            { "Inject nagsave.bin",           &InjectFile,           N_NANDWRITE | F_NAGSAVE },
            { "Inject nnidsave.bin",          &InjectFile,           N_NANDWRITE | F_NNIDSAVE },
            { "Inject friendsave.bin",        &InjectFile,           N_NANDWRITE | F_FRIENDSAVE }
        }
    },
    {
        "File Inject... (EmuNAND)", 11, // ID 9
        {
            { "Inject ticket.db",             &InjectFile,           N_NANDWRITE | N_EMUNAND | F_TICKET },
            { "Inject title.db",              &InjectFile,           N_NANDWRITE | N_EMUNAND | F_TITLE },
            { "Inject import.db",             &InjectFile,           N_NANDWRITE | N_EMUNAND | F_IMPORT },
            { "Inject certs.db",              &InjectFile,           N_NANDWRITE | N_EMUNAND | F_CERTS },
            { "Inject SecureInfo_A",          &InjectFile,           N_NANDWRITE | N_EMUNAND | F_SECUREINFO },
            { "Inject LocalFriendCodeSeed_B", &InjectFile,           N_NANDWRITE | N_EMUNAND | F_LOCALFRIEND },
            { "Inject movable.sed",           &InjectFile,           N_NANDWRITE | N_EMUNAND | F_MOVABLE },
            { "Inject seedsave.bin",          &InjectFile,           N_NANDWRITE | N_EMUNAND | F_SEEDSAVE },
            { "Inject nagsave.bin",           &InjectFile,           N_NANDWRITE | N_EMUNAND | F_NAGSAVE },
            { "Inject nnidsave.bin",          &InjectFile,           N_NANDWRITE | N_EMUNAND | F_NNIDSAVE },
            { "Inject friendsave.bin",        &InjectFile,           N_NANDWRITE | N_EMUNAND | F_FRIENDSAVE }
        }
    },
    {
        "Miscellaneous... (SysNAND)", 8, // ID 10
        {
            
            { "Health&Safety Dump",           &DumpHealthAndSafety,   0 },
            { "Health&Safety Inject",         &InjectHealthAndSafety, N_NANDWRITE },
            { "GBA VC Save Dump",             &DumpGbaVcSave,         0 },
            { "GBA VC Save Inject",           &InjectGbaVcSave,       N_NANDWRITE },
            { "Update SeedDB",                &UpdateSeedDb,          0 },
            { "Autofix CMACs",                &AutoFixCmacs,          N_NANDWRITE },
            { "NCCH FIRMs Dump",              &DumpNcchFirms,         0 },
            { "FIRM ARM9 Decryptor",          &DecryptFirmArm9File,   0 }
        }
    },
    {
        "Miscellaneous... (EmuNAND)", 6, // ID 11
        {
            
            { "Health&Safety Dump",           &DumpHealthAndSafety,   N_EMUNAND },
            { "Health&Safety Inject",         &InjectHealthAndSafety, N_NANDWRITE | N_EMUNAND },
            { "Update SeedDB",                &UpdateSeedDb,          N_EMUNAND },
            { "Autofix CMACs",                &AutoFixCmacs,          N_NANDWRITE | N_EMUNAND },
            { "NCCH FIRMs Dump",              &DumpNcchFirms,         N_EMUNAND },
            { "FIRM ARM9 Decryptor",          &DecryptFirmArm9File,   0 }
        }
    },
    {
        "NCCH/NCSD File Options", 3, // ID 12
        {
            { "NCCH/NCSD Decryptor",          &CryptGameFiles,        GC_NCCH_PROCESS },
            { "NCCH/NCSD Encryptor",          &CryptGameFiles,        GC_NCCH_PROCESS | GC_NCCH_ENC0x2C },
            { "NCCH/NCSD to CIA Converter",   &ConvertNcsdNcchToCia,  0 }
        }
    },
    {
        "CIA File Options", 4, // ID 13
        {
            { "CIA Decryptor (shallow)",      &CryptGameFiles,        GC_CIA_PROCESS },
            { "CIA Decryptor (deep)",         &CryptGameFiles,        GC_CIA_PROCESS | GC_CIA_DEEP },
            { "CIA Decryptor (CXI only)",     &CryptGameFiles,        GC_CIA_PROCESS | GC_CIA_DEEP | GC_CXI_ONLY },
            { "CIA Encryptor (NCCH)",         &CryptGameFiles,        GC_CIA_PROCESS | GC_NCCH_ENC0x2C }
        }
    },
    {
        "BOSS File Options", 2, // ID 14
        {
            { "BOSS Decryptor",               &CryptGameFiles,        GC_BOSS_PROCESS },
            { "BOSS Encryptor",               &CryptGameFiles,        GC_BOSS_PROCESS | GC_BOSS_ENCRYPT }
        }
    },
    {
        "SD File Options", 3, // ID 15
        {
            { "SD Decryptor/Encryptor",       &CryptSdFiles,          0 },
            { "SD Decryptor (SysNAND dir)",   &DecryptSdFilesDirect,  0 },
            { "SD Decryptor (EmuNAND dir)",   &DecryptSdFilesDirect,  N_EMUNAND }
        }
    },
    {
        NULL, 0, { { 0 } } // empty menu to signal end
    }
};


void Reboot()
{
    i2cWriteRegister(I2C_DEV_MCU, 0x20, 1 << 2);
    while(true);
}


void PowerOff()
{
    i2cWriteRegister(I2C_DEV_MCU, 0x20, 1 << 0);
    while (true);
}


u32 InitializeD9()
{
    u32 errorlevel = 0; // 0 -> none, 1 -> autopause, 2 -> critical
    
    ClearScreenFull(true, true);
    DebugClear();
    #ifndef BUILD_NAME
    Debug("-- Decrypt9 --");
    #else
    Debug("-- %s --", BUILD_NAME);
    #endif
    
    // a little bit of information about the current menu
    if (sizeof(menu)) {
        u32 n_submenus = 0;
        u32 n_features = 0;
        for (u32 m = 0; menu[m].n_entries; m++) {
            n_submenus = m;
            for (u32 e = 0; e < menu[m].n_entries; e++)
                n_features += (menu[m].entries[e].function) ? 1 : 0;
        }
        Debug("Counting %u submenus and %u features", n_submenus, n_features);
    }
    
    Debug("Initializing, hold L+R to pause");
    Debug("");    
    
    if (InitFS()) {
        Debug("Initializing SD card... success");
        FileGetData("d9logo.bin", BOT_SCREEN0, 320 * 240 * 3, 0);
        memcpy(BOT_SCREEN1, BOT_SCREEN0, 320 * 240 * 3);
        Debug("Work directory: %s", GetWorkDir());
        Debug("Game directory: %s", GetGameDir());
        SetupSector0x96Key0x11(); // Sector0x96 key - no effect on error level
        if (SetupTwlKey0x03() != 0) // TWL KeyX / KeyY
            errorlevel = 2;
        if ((GetUnitPlatform() == PLATFORM_N3DS) && (SetupCtrNandKeyY0x05() != 0))
            errorlevel = 2; // N3DS CTRNAND KeyY
        if (LoadKeyFromFile(0x25, 'X', NULL)) // NCCH 7x KeyX
            errorlevel = (errorlevel < 1) ? 1 : errorlevel;
        if (GetUnitKeysType() != KEYS_DEVKIT) { // don't even try 0x18 / 0x1B on devkits
            if (LoadKeyFromFile(0x18, 'X', NULL)) // NCCH Secure3 KeyX
                errorlevel = (errorlevel < 1) ? 1 : errorlevel;
            if (LoadKeyFromFile(0x1B, 'X', NULL)) // NCCH Secure4 KeyX
                errorlevel = (errorlevel < 1) ? 1 : errorlevel;
        }
        if (LoadKeyFromFile(0x24, 'Y', NULL)) // AGBSAVE CMAC KeyY
            errorlevel = (errorlevel < 1) ? 1 : errorlevel;
        Debug("Finalizing Initialization...");
        RemainingStorageSpace();
    } else {
        Debug("Initializing SD card... failed");
            errorlevel = 2;
    }
    Debug("");
    Debug("Initialization: %s", (errorlevel == 0) ? "success!" : (errorlevel == 1) ? "partially failed" : "failed!");
    
    if (((~HID_STATE & BUTTON_L1) && (~HID_STATE & BUTTON_R1)) || (errorlevel > 1)) {
        Debug("(A to %s)", (errorlevel > 1) ? "exit" : "continue");
        while (!(InputWait() & BUTTON_A));
    }
    
    return errorlevel;
}


int main()
{
    u32 menu_exit = MENU_EXIT_REBOOT;
    
    if (InitializeD9() <= 1) {
        menu_exit = ProcessMenu(menu, SUBMENU_START);
    }
    DeinitFS();
    
    (menu_exit == MENU_EXIT_REBOOT) ? Reboot() : PowerOff();
    return 0;
}
