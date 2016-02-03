#include "common.h"
#include "draw.h"
#include "fs.h"
#include "menu.h"
#include "i2c.h"
#include "decryptor/game.h"
#include "decryptor/nand.h"
#include "decryptor/nandfat.h"
#include "decryptor/titlekey.h"

#define SUBMENU_START 8


MenuInfo menu[] =
{
    {
        "XORpad Generator Options", 10,
        {
            { "NCCH Padgen",                  &NcchPadgen,            0 },
            { "SD Padgen (SDinfo.bin)",       &SdPadgen,              0 },
            { "SD Padgen (SysNAND dir)",      &SdPadgenDirect,        0 },
            { "SD Padgen (EmuNAND1 dir)",     &SdPadgenDirect,        N_EMUNAND1 },
            { "SD Padgen (EmuNAND2 dir)",     &SdPadgenDirect,        N_EMUNAND2 },
            { "SD Padgen (EmuNAND3 dir)",     &SdPadgenDirect,        N_EMUNAND3 },
            { "SD Padgen (EmuNAND4 dir)",     &SdPadgenDirect,        N_EMUNAND4 },
            { "CTRNAND Padgen",               &CtrNandPadgen,         0 },
            { "TWLNAND Padgen",               &TwlNandPadgen,         0 },
            { "FIRM0FIRM1 Padgen",            &Firm0Firm1Padgen,      0 }
        }
    },
    {
        "Titlekey Decrypt Options", 6,
        {
            { "Titlekey Decrypt (file)",      &DecryptTitlekeysFile,  0 },
            { "Titlekey Decrypt (SysNAND)",   &DecryptTitlekeysNand,  0 },
            { "Titlekey Decrypt (EmuNAND1)",  &DecryptTitlekeysNand,  N_EMUNAND1 },
            { "Titlekey Decrypt (EmuNAND2)",  &DecryptTitlekeysNand,  N_EMUNAND2 },
            { "Titlekey Decrypt (EmuNAND3)",  &DecryptTitlekeysNand,  N_EMUNAND3 },
            { "Titlekey Decrypt (EmuNAND4)",  &DecryptTitlekeysNand,  N_EMUNAND4 }
        }
    },
    {
        "SysNAND Options", 8,
        {
            { "NAND Backup",                  &DumpNand,              0 },
            { "NAND Restore",                 &RestoreNand,           N_NANDWRITE },
            { "Partition Dump...",            NULL,                   SUBMENU_START + 0 },
            { "Partition Inject...",          NULL,                   SUBMENU_START + 5 },
            { "File Dump...",                 NULL,                   SUBMENU_START + 10 },
            { "File Inject...",               NULL,                   SUBMENU_START + 15 },
            { "Health&Safety Dump",           &DumpHealthAndSafety,   0 },
            { "Health&Safety Inject",         &InjectHealthAndSafety, N_NANDWRITE }
        }
    },
    {
        "EmuNAND1 Options", 9,
        {
            { "EmuNAND1 Backup",              &DumpNand,              N_EMUNAND1 },
            { "EmuNAND1 Restore",             &RestoreNand,           N_NANDWRITE | N_EMUNAND1 | N_FORCENAND },
            { "Partition Dump...",            NULL,                   SUBMENU_START + 1 },
            { "Partition Inject...",          NULL,                   SUBMENU_START + 6 },
            { "File Dump...",                 NULL,                   SUBMENU_START + 11 },
            { "File Inject...",               NULL,                   SUBMENU_START + 16 },
            { "Health&Safety Dump",           &DumpHealthAndSafety,   N_EMUNAND1 },
            { "Health&Safety Inject",         &InjectHealthAndSafety, N_NANDWRITE | N_EMUNAND1 },
            { "Update SeedDB",                &UpdateSeedDb,          N_EMUNAND1 }
        }
    },
    {
        "EmuNAND2 Options", 9,
        {
            { "EmuNAND2 Backup",              &DumpNand,              N_EMUNAND2 },
            { "EmuNAND2 Restore",             &RestoreNand,           N_NANDWRITE | N_EMUNAND2 | N_FORCENAND },
            { "Partition Dump...",            NULL,                   SUBMENU_START + 2 },
            { "Partition Inject...",          NULL,                   SUBMENU_START + 7 },
            { "File Dump...",                 NULL,                   SUBMENU_START + 12 },
            { "File Inject...",               NULL,                   SUBMENU_START + 17 },
            { "Health&Safety Dump",           &DumpHealthAndSafety,   N_EMUNAND2 },
            { "Health&Safety Inject",         &InjectHealthAndSafety, N_NANDWRITE | N_EMUNAND2 },
            { "Update SeedDB",                &UpdateSeedDb,          N_EMUNAND2 }
        }
    },
    {
        "EmuNAND3 Options", 9,
        {
            { "EmuNAND3 Backup",              &DumpNand,              N_EMUNAND3 },
            { "EmuNAND3 Restore",             &RestoreNand,           N_NANDWRITE | N_EMUNAND3 | N_FORCENAND },
            { "Partition Dump...",            NULL,                   SUBMENU_START + 3 },
            { "Partition Inject...",          NULL,                   SUBMENU_START + 8 },
            { "File Dump...",                 NULL,                   SUBMENU_START + 13 },
            { "File Inject...",               NULL,                   SUBMENU_START + 18 },
            { "Health&Safety Dump",           &DumpHealthAndSafety,   N_EMUNAND3 },
            { "Health&Safety Inject",         &InjectHealthAndSafety, N_NANDWRITE | N_EMUNAND3 },
            { "Update SeedDB",                &UpdateSeedDb,          N_EMUNAND3 }
        }
    },
    {
        "EmuNAND4 Options", 9,
        {
            { "EmuNAND4 Backup",              &DumpNand,              N_EMUNAND4 },
            { "EmuNAND4 Restore",             &RestoreNand,           N_NANDWRITE | N_EMUNAND4 | N_FORCENAND },
            { "Partition Dump...",            NULL,                   SUBMENU_START + 4 },
            { "Partition Inject...",          NULL,                   SUBMENU_START + 9 },
            { "File Dump...",                 NULL,                   SUBMENU_START + 14 },
            { "File Inject...",               NULL,                   SUBMENU_START + 19 },
            { "Health&Safety Dump",           &DumpHealthAndSafety,   N_EMUNAND4 },
            { "Health&Safety Inject",         &InjectHealthAndSafety, N_NANDWRITE | N_EMUNAND4 },
            { "Update SeedDB",                &UpdateSeedDb,          N_EMUNAND4 }
        }
    },
    {
        "Game Decryptor Options", 13,
        {
            { "NCCH/NCSD Decryptor",          &CryptGameFiles,        GC_NCCH_PROCESS },
            { "NCCH/NCSD Encryptor",          &CryptGameFiles,        GC_NCCH_PROCESS | GC_NCCH_ENCRYPT },
            { "CIA Decryptor (shallow)",      &CryptGameFiles,        GC_CIA_PROCESS },
            { "CIA Decryptor (deep)",         &CryptGameFiles,        GC_CIA_PROCESS | GC_CIA_DEEP },
            { "CIA Decryptor (CXI only)",     &CryptGameFiles,        GC_CIA_PROCESS | GC_CIA_DEEP | GC_CXI_ONLY },
            { "CIA Encryptor (NCCH)",         &CryptGameFiles,        GC_CIA_PROCESS | GC_NCCH_ENCRYPT },
            { "CIA Encryptor (CXI only)",     &CryptGameFiles,        GC_CIA_PROCESS | GC_NCCH_ENCRYPT | GC_CXI_ONLY },
            { "SD Decryptor/Encryptor",       &CryptSdFiles,          0 },
            { "SD Decryptor (SysNAND dir)",   &DecryptSdFilesDirect,  0 },
            { "SD Decryptor (EmuNAND1 dir)",  &DecryptSdFilesDirect,  N_EMUNAND1 },
            { "SD Decryptor (EmuNAND2 dir)",  &DecryptSdFilesDirect,  N_EMUNAND2 },
            { "SD Decryptor (EmuNAND3 dir)",  &DecryptSdFilesDirect,  N_EMUNAND3 },
            { "SD Decryptor (EmuNAND4 dir)",  &DecryptSdFilesDirect,  N_EMUNAND4 }
        }
    },
    // everything below is not contained in the main menu
    {
        "Partition Dump... (SysNAND)", 6, // ID 0
        {
            { "Dump TWLN Partition",          &DecryptNandPartition, P_TWLN },
            { "Dump TWLP Partition",          &DecryptNandPartition, P_TWLP },
            { "Dump AGBSAVE Partition",       &DecryptNandPartition, P_AGBSAVE },
            { "Dump FIRM0 Partition",         &DecryptNandPartition, P_FIRM0 },
            { "Dump FIRM1 Partition",         &DecryptNandPartition, P_FIRM1 },
            { "Dump CTRNAND Partition",       &DecryptNandPartition, P_CTRNAND }
        }
    },
    {
        "Partition Dump...(EmuNAND1)", 6, // ID 1
        {
            { "Dump TWLN Partition",          &DecryptNandPartition, N_EMUNAND1 | P_TWLN },
            { "Dump TWLP Partition",          &DecryptNandPartition, N_EMUNAND1 | P_TWLP },
            { "Dump AGBSAVE Partition",       &DecryptNandPartition, N_EMUNAND1 | P_AGBSAVE },
            { "Dump FIRM0 Partition",         &DecryptNandPartition, N_EMUNAND1 | P_FIRM0 },
            { "Dump FIRM1 Partition",         &DecryptNandPartition, N_EMUNAND1 | P_FIRM1 },
            { "Dump CTRNAND Partition",       &DecryptNandPartition, N_EMUNAND1 | P_CTRNAND }
        }
    },
    {
        "Partition Dump...(EmuNAND2)", 6, // ID 2
        {
            { "Dump TWLN Partition",          &DecryptNandPartition, N_EMUNAND2 | P_TWLN },
            { "Dump TWLP Partition",          &DecryptNandPartition, N_EMUNAND2 | P_TWLP },
            { "Dump AGBSAVE Partition",       &DecryptNandPartition, N_EMUNAND2 | P_AGBSAVE },
            { "Dump FIRM0 Partition",         &DecryptNandPartition, N_EMUNAND2 | P_FIRM0 },
            { "Dump FIRM1 Partition",         &DecryptNandPartition, N_EMUNAND2 | P_FIRM1 },
            { "Dump CTRNAND Partition",       &DecryptNandPartition, N_EMUNAND2 | P_CTRNAND }
        }
    },
    {
        "Partition Dump...(EmuNAND3)", 6, // ID 3
        {
            { "Dump TWLN Partition",          &DecryptNandPartition, N_EMUNAND3 | P_TWLN },
            { "Dump TWLP Partition",          &DecryptNandPartition, N_EMUNAND3 | P_TWLP },
            { "Dump AGBSAVE Partition",       &DecryptNandPartition, N_EMUNAND3 | P_AGBSAVE },
            { "Dump FIRM0 Partition",         &DecryptNandPartition, N_EMUNAND3 | P_FIRM0 },
            { "Dump FIRM1 Partition",         &DecryptNandPartition, N_EMUNAND3 | P_FIRM1 },
            { "Dump CTRNAND Partition",       &DecryptNandPartition, N_EMUNAND3 | P_CTRNAND }
        }
    },
    {
        "Partition Dump...(EmuNAND4)", 6, // ID 4
        {
            { "Dump TWLN Partition",          &DecryptNandPartition, N_EMUNAND4 | P_TWLN },
            { "Dump TWLP Partition",          &DecryptNandPartition, N_EMUNAND4 | P_TWLP },
            { "Dump AGBSAVE Partition",       &DecryptNandPartition, N_EMUNAND4 | P_AGBSAVE },
            { "Dump FIRM0 Partition",         &DecryptNandPartition, N_EMUNAND4 | P_FIRM0 },
            { "Dump FIRM1 Partition",         &DecryptNandPartition, N_EMUNAND4 | P_FIRM1 },
            { "Dump CTRNAND Partition",       &DecryptNandPartition, N_EMUNAND4 | P_CTRNAND }
        }
    },
    {
        "Partition Inject... (SysNAND)", 6, // ID 5
        {
            { "Inject TWLN Partition",        &InjectNandPartition, N_NANDWRITE | P_TWLN },
            { "Inject TWLP Partition",        &InjectNandPartition, N_NANDWRITE | P_TWLP },
            { "Inject AGBSAVE Partition",     &InjectNandPartition, N_NANDWRITE | P_AGBSAVE },
            { "Inject FIRM0 Partition",       &InjectNandPartition, N_NANDWRITE | P_FIRM0 },
            { "Inject FIRM1 Partition",       &InjectNandPartition, N_NANDWRITE | P_FIRM1 },
            { "Inject CTRNAND Partition",     &InjectNandPartition, N_NANDWRITE | P_CTRNAND }
        }
    },
    {
        "Partition Inject... (EmuNAND1)", 6, // ID 6
        {
            { "Inject TWLN Partition",        &InjectNandPartition, N_NANDWRITE | N_EMUNAND1 | P_TWLN },
            { "Inject TWLP Partition",        &InjectNandPartition, N_NANDWRITE | N_EMUNAND1 | P_TWLP },
            { "Inject AGBSAVE Partition",     &InjectNandPartition, N_NANDWRITE | N_EMUNAND1 | P_AGBSAVE },
            { "Inject FIRM0 Partition",       &InjectNandPartition, N_NANDWRITE | N_EMUNAND1 | P_FIRM0 },
            { "Inject FIRM1 Partition",       &InjectNandPartition, N_NANDWRITE | N_EMUNAND1 | P_FIRM1 },
            { "Inject CTRNAND Partition",     &InjectNandPartition, N_NANDWRITE | N_EMUNAND1 | P_CTRNAND }
        }
    },
    {
        "Partition Inject... (EmuNAND2)", 6, // ID 7
        {
            { "Inject TWLN Partition",        &InjectNandPartition, N_NANDWRITE | N_EMUNAND2 | P_TWLN },
            { "Inject TWLP Partition",        &InjectNandPartition, N_NANDWRITE | N_EMUNAND2 | P_TWLP },
            { "Inject AGBSAVE Partition",     &InjectNandPartition, N_NANDWRITE | N_EMUNAND2 | P_AGBSAVE },
            { "Inject FIRM0 Partition",       &InjectNandPartition, N_NANDWRITE | N_EMUNAND2 | P_FIRM0 },
            { "Inject FIRM1 Partition",       &InjectNandPartition, N_NANDWRITE | N_EMUNAND2 | P_FIRM1 },
            { "Inject CTRNAND Partition",     &InjectNandPartition, N_NANDWRITE | N_EMUNAND2 | P_CTRNAND }
        }
    },
    {
        "Partition Inject... (EmuNAND3)", 6, // ID 8
        {
            { "Inject TWLN Partition",        &InjectNandPartition, N_NANDWRITE | N_EMUNAND3 | P_TWLN },
            { "Inject TWLP Partition",        &InjectNandPartition, N_NANDWRITE | N_EMUNAND3 | P_TWLP },
            { "Inject AGBSAVE Partition",     &InjectNandPartition, N_NANDWRITE | N_EMUNAND3 | P_AGBSAVE },
            { "Inject FIRM0 Partition",       &InjectNandPartition, N_NANDWRITE | N_EMUNAND3 | P_FIRM0 },
            { "Inject FIRM1 Partition",       &InjectNandPartition, N_NANDWRITE | N_EMUNAND3 | P_FIRM1 },
            { "Inject CTRNAND Partition",     &InjectNandPartition, N_NANDWRITE | N_EMUNAND3 | P_CTRNAND }
        }
    },
    {
        "Partition Inject... (EmuNAND4)", 6, // ID 9
        {
            { "Inject TWLN Partition",        &InjectNandPartition, N_NANDWRITE | N_EMUNAND4 | P_TWLN },
            { "Inject TWLP Partition",        &InjectNandPartition, N_NANDWRITE | N_EMUNAND4 | P_TWLP },
            { "Inject AGBSAVE Partition",     &InjectNandPartition, N_NANDWRITE | N_EMUNAND4 | P_AGBSAVE },
            { "Inject FIRM0 Partition",       &InjectNandPartition, N_NANDWRITE | N_EMUNAND4 | P_FIRM0 },
            { "Inject FIRM1 Partition",       &InjectNandPartition, N_NANDWRITE | N_EMUNAND4 | P_FIRM1 },
            { "Inject CTRNAND Partition",     &InjectNandPartition, N_NANDWRITE | N_EMUNAND4 | P_CTRNAND }
        }
    },
    {
        "File Dump... (SysNAND)", 10, // ID 10
        {
            { "Dump ticket.db",               &DumpFile,             F_TICKET },
            { "Dump title.db",                &DumpFile,             F_TITLE },
            { "Dump import.db",               &DumpFile,             F_IMPORT },
            { "Dump certs.db",                &DumpFile,             F_CERTS },
            { "Dump SecureInfo_A",            &DumpFile,             F_SECUREINFO },
            { "Dump LocalFriendCodeSeed_B",   &DumpFile,             F_LOCALFRIEND },
            { "Dump rand_seed",               &DumpFile,             F_RANDSEED },
            { "Dump movable.sed",             &DumpFile,             F_MOVABLE },
            { "Dump nagsave.bin",             &DumpFile,             F_NAGSAVE },
            { "Dump nnidsave.bin",            &DumpFile,             F_NNIDSAVE }
        }
    },
    {
        "File Dump... (EmuNAND1)", 11, // ID 11
        {
            { "Dump ticket.db",               &DumpFile,             N_EMUNAND1 | F_TICKET },
            { "Dump title.db",                &DumpFile,             N_EMUNAND1 | F_TITLE },
            { "Dump import.db",               &DumpFile,             N_EMUNAND1 | F_IMPORT },
            { "Dump certs.db",                &DumpFile,             N_EMUNAND1 | F_CERTS },
            { "Dump SecureInfo_A",            &DumpFile,             N_EMUNAND1 | F_SECUREINFO },
            { "Dump LocalFriendCodeSeed_B",   &DumpFile,             N_EMUNAND1 | F_LOCALFRIEND },
            { "Dump rand_seed",               &DumpFile,             N_EMUNAND1 | F_RANDSEED },
            { "Dump movable.sed",             &DumpFile,             N_EMUNAND1 | F_MOVABLE },
            { "Dump seedsave.bin",            &DumpFile,             N_EMUNAND1 | F_SEEDSAVE },
            { "Dump nagsave.bin",             &DumpFile,             N_EMUNAND1 | F_NAGSAVE },
            { "Dump nnidsave.bin",            &DumpFile,             N_EMUNAND1 | F_NNIDSAVE }
        }
    },
    {
        "File Dump... (EmuNAND2)", 10, // ID 12
        {
            { "Dump ticket.db",               &DumpFile,             N_EMUNAND2 | F_TICKET },
            { "Dump title.db",                &DumpFile,             N_EMUNAND2 | F_TITLE },
            { "Dump import.db",               &DumpFile,             N_EMUNAND2 | F_IMPORT },
            { "Dump certs.db",                &DumpFile,             N_EMUNAND2 | F_CERTS },
            { "Dump SecureInfo_A",            &DumpFile,             N_EMUNAND2 | F_SECUREINFO },
            { "Dump LocalFriendCodeSeed_B",   &DumpFile,             N_EMUNAND2 | F_LOCALFRIEND },
            { "Dump rand_seed",               &DumpFile,             N_EMUNAND2 | F_RANDSEED },
            { "Dump movable.sed",             &DumpFile,             N_EMUNAND2 | F_MOVABLE },
            { "Dump seedsave.bin",            &DumpFile,             N_EMUNAND2 | F_SEEDSAVE },
            { "Dump nagsave.bin",             &DumpFile,             N_EMUNAND2 | F_NAGSAVE },
            { "Dump nnidsave.bin",            &DumpFile,             N_EMUNAND2 | F_NNIDSAVE }
        }
    },
    {
        "File Dump... (EmuNAND3)", 10, // ID 13
        {
            { "Dump ticket.db",               &DumpFile,             N_EMUNAND3 | F_TICKET },
            { "Dump title.db",                &DumpFile,             N_EMUNAND3 | F_TITLE },
            { "Dump import.db",               &DumpFile,             N_EMUNAND3 | F_IMPORT },
            { "Dump certs.db",                &DumpFile,             N_EMUNAND3 | F_CERTS },
            { "Dump SecureInfo_A",            &DumpFile,             N_EMUNAND3 | F_SECUREINFO },
            { "Dump LocalFriendCodeSeed_B",   &DumpFile,             N_EMUNAND3 | F_LOCALFRIEND },
            { "Dump rand_seed",               &DumpFile,             N_EMUNAND3 | F_RANDSEED },
            { "Dump movable.sed",             &DumpFile,             N_EMUNAND3 | F_MOVABLE },
            { "Dump seedsave.bin",            &DumpFile,             N_EMUNAND3 | F_SEEDSAVE },
            { "Dump nagsave.bin",             &DumpFile,             N_EMUNAND3 | F_NAGSAVE },
            { "Dump nnidsave.bin",            &DumpFile,             N_EMUNAND3 | F_NNIDSAVE }
        }
    },
    {
        "File Dump... (EmuNAND4)", 10, // ID 14
        {
            { "Dump ticket.db",               &DumpFile,             N_EMUNAND4 | F_TICKET },
            { "Dump title.db",                &DumpFile,             N_EMUNAND4 | F_TITLE },
            { "Dump import.db",               &DumpFile,             N_EMUNAND4 | F_IMPORT },
            { "Dump certs.db",                &DumpFile,             N_EMUNAND4 | F_CERTS },
            { "Dump SecureInfo_A",            &DumpFile,             N_EMUNAND4 | F_SECUREINFO },
            { "Dump LocalFriendCodeSeed_B",   &DumpFile,             N_EMUNAND4 | F_LOCALFRIEND },
            { "Dump rand_seed",               &DumpFile,             N_EMUNAND4 | F_RANDSEED },
            { "Dump movable.sed",             &DumpFile,             N_EMUNAND4 | F_MOVABLE },
            { "Dump seedsave.bin",            &DumpFile,             N_EMUNAND4 | F_SEEDSAVE },
            { "Dump nagsave.bin",             &DumpFile,             N_EMUNAND4 | F_NAGSAVE },
            { "Dump nnidsave.bin",            &DumpFile,             N_EMUNAND4 | F_NNIDSAVE }
        }
    },
    {
        "File Inject... (SysNAND)", 10, // ID 15
        {
            { "Inject ticket.db",             &InjectFile,           N_NANDWRITE | F_TICKET },
            { "Inject title.db",              &InjectFile,           N_NANDWRITE | F_TITLE },
            { "Inject import.db",             &InjectFile,           N_NANDWRITE | F_IMPORT },
            { "Inject certs.db",              &InjectFile,           N_NANDWRITE | F_CERTS },
            { "Inject SecureInfo_A",          &InjectFile,           N_NANDWRITE | F_SECUREINFO },
            { "Inject LocalFriendCodeSeed_B", &InjectFile,           N_NANDWRITE | F_LOCALFRIEND },
            { "Inject rand_seed",             &InjectFile,           N_NANDWRITE | F_RANDSEED },
            { "Inject movable.sed",           &InjectFile,           N_NANDWRITE | F_MOVABLE },
            { "Inject nagsave.bin",           &InjectFile,           N_NANDWRITE | F_NAGSAVE },
            { "Inject nnidsave.bin",          &InjectFile,           N_NANDWRITE | F_NNIDSAVE }
        }
    },
    {
        "File Inject... (EmuNAND1)", 10, // ID 16
        {
            { "Inject ticket.db",             &InjectFile,           N_NANDWRITE | N_EMUNAND1 | F_TICKET },
            { "Inject title.db",              &InjectFile,           N_NANDWRITE | N_EMUNAND1 | F_TITLE },
            { "Inject import.db",             &InjectFile,           N_NANDWRITE | N_EMUNAND1 | F_IMPORT },
            { "Inject certs.db",              &InjectFile,           N_NANDWRITE | N_EMUNAND1 | F_CERTS },
            { "Inject SecureInfo_A",          &InjectFile,           N_NANDWRITE | N_EMUNAND1 | F_SECUREINFO },
            { "Inject LocalFriendCodeSeed_B", &InjectFile,           N_NANDWRITE | N_EMUNAND1 | F_LOCALFRIEND },
            { "Inject rand_seed",             &InjectFile,           N_NANDWRITE | N_EMUNAND1 | F_RANDSEED },
            { "Inject movable.sed",           &InjectFile,           N_NANDWRITE | N_EMUNAND1 | F_MOVABLE },
            { "Inject seedsave.bin",          &InjectFile,           N_NANDWRITE | N_EMUNAND1 | F_SEEDSAVE },
            { "Inject nagsave.bin",           &InjectFile,           N_NANDWRITE | N_EMUNAND1 | F_NAGSAVE },
            { "Inject nnidsave.bin",          &InjectFile,           N_NANDWRITE | N_EMUNAND1 | F_NNIDSAVE }
        }
    },
    {
        "File Inject... (EmuNAND2)", 10, // ID 17
        {
            { "Inject ticket.db",             &InjectFile,           N_NANDWRITE | N_EMUNAND2 | F_TICKET },
            { "Inject title.db",              &InjectFile,           N_NANDWRITE | N_EMUNAND2 | F_TITLE },
            { "Inject import.db",             &InjectFile,           N_NANDWRITE | N_EMUNAND2 | F_IMPORT },
            { "Inject certs.db",              &InjectFile,           N_NANDWRITE | N_EMUNAND2 | F_CERTS },
            { "Inject SecureInfo_A",          &InjectFile,           N_NANDWRITE | N_EMUNAND2 | F_SECUREINFO },
            { "Inject LocalFriendCodeSeed_B", &InjectFile,           N_NANDWRITE | N_EMUNAND2 | F_LOCALFRIEND },
            { "Inject rand_seed",             &InjectFile,           N_NANDWRITE | N_EMUNAND2 | F_RANDSEED },
            { "Inject movable.sed",           &InjectFile,           N_NANDWRITE | N_EMUNAND2 | F_MOVABLE },
            { "Inject seedsave.bin",          &InjectFile,           N_NANDWRITE | N_EMUNAND2 | F_SEEDSAVE },
            { "Inject nagsave.bin",           &InjectFile,           N_NANDWRITE | N_EMUNAND2 | F_NAGSAVE },
            { "Inject nnidsave.bin",          &InjectFile,           N_NANDWRITE | N_EMUNAND2 | F_NNIDSAVE }
        }
    },
    {
        "File Inject... (EmuNAND3)", 10, // ID 18
        {
            { "Inject ticket.db",             &InjectFile,           N_NANDWRITE | N_EMUNAND3 | F_TICKET },
            { "Inject title.db",              &InjectFile,           N_NANDWRITE | N_EMUNAND3 | F_TITLE },
            { "Inject import.db",             &InjectFile,           N_NANDWRITE | N_EMUNAND3 | F_IMPORT },
            { "Inject certs.db",              &InjectFile,           N_NANDWRITE | N_EMUNAND3 | F_CERTS },
            { "Inject SecureInfo_A",          &InjectFile,           N_NANDWRITE | N_EMUNAND3 | F_SECUREINFO },
            { "Inject LocalFriendCodeSeed_B", &InjectFile,           N_NANDWRITE | N_EMUNAND3 | F_LOCALFRIEND },
            { "Inject rand_seed",             &InjectFile,           N_NANDWRITE | N_EMUNAND3 | F_RANDSEED },
            { "Inject movable.sed",           &InjectFile,           N_NANDWRITE | N_EMUNAND3 | F_MOVABLE },
            { "Inject seedsave.bin",          &InjectFile,           N_NANDWRITE | N_EMUNAND3 | F_SEEDSAVE },
            { "Inject nagsave.bin",           &InjectFile,           N_NANDWRITE | N_EMUNAND3 | F_NAGSAVE },
            { "Inject nnidsave.bin",          &InjectFile,           N_NANDWRITE | N_EMUNAND3 | F_NNIDSAVE }
        }
    },
    {
        "File Inject... (EmuNAND4)", 10, // ID 19
        {
            { "Inject ticket.db",             &InjectFile,           N_NANDWRITE | N_EMUNAND4 | F_TICKET },
            { "Inject title.db",              &InjectFile,           N_NANDWRITE | N_EMUNAND4 | F_TITLE },
            { "Inject import.db",             &InjectFile,           N_NANDWRITE | N_EMUNAND4 | F_IMPORT },
            { "Inject certs.db",              &InjectFile,           N_NANDWRITE | N_EMUNAND4 | F_CERTS },
            { "Inject SecureInfo_A",          &InjectFile,           N_NANDWRITE | N_EMUNAND4 | F_SECUREINFO },
            { "Inject LocalFriendCodeSeed_B", &InjectFile,           N_NANDWRITE | N_EMUNAND4 | F_LOCALFRIEND },
            { "Inject rand_seed",             &InjectFile,           N_NANDWRITE | N_EMUNAND4 | F_RANDSEED },
            { "Inject movable.sed",           &InjectFile,           N_NANDWRITE | N_EMUNAND4 | F_MOVABLE },
            { "Inject seedsave.bin",          &InjectFile,           N_NANDWRITE | N_EMUNAND4 | F_SEEDSAVE },
            { "Inject nagsave.bin",           &InjectFile,           N_NANDWRITE | N_EMUNAND4 | F_NAGSAVE },
            { "Inject nnidsave.bin",          &InjectFile,           N_NANDWRITE | N_EMUNAND4 | F_NNIDSAVE }
        }
    },
    {
        NULL, 0, {}, // empty menu to signal end
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


int main()
{
    ClearScreenFull(true, true);
    InitFS();

    u32 menu_exit = ProcessMenu(menu, SUBMENU_START);
    
    DeinitFS();
    (menu_exit == MENU_EXIT_REBOOT) ? Reboot() : PowerOff();
    return 0;
}
