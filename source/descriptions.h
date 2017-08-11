#pragma once

// XORpad Generator Options
extern char *NcchPadgenDesc,
            *SdPadgenDesc,
            *SdPadgenDirectDesc,
            *AnyPadgenDesc,
            *CtrNandPadgenDesc,
            *CtrNandPadgen0x04Desc,
            *TwlNandPadgenDesc,
            *Firm0Firm1PadgenDesc;

// Ticket/Titlekey Options
extern char *CryptTitlekeysFileDesc,
            *DumpDecryptedTitlekeysDesc,
            *DumpTitlekeysDesc,
            *DumpTicketsDesc;

// Maintenance Options
extern char *SystemInfoDesc,
            *CreateSelfRef,
            *SelfTestDesc,
            *BuildKeyDbDesc,
            *CryptKeyDbDesc;

// Gamecart Dumper Options
extern char *DumpGameCartFullDesc,
            *DumpGameCartTrimDesc,
            *DumpGameCartDecFullDesc,
            *DumpGameCartDecTrimDesc,
            *DumpGameCartCIADesc,
            *DumpPrivateHeaderDesc,
            *DumpCartSaveDesc,
            *FlashCartSaveDesc;

// NDS Flashcart Options
extern char *AK2iDumpDesc,
            *AK2iFlashDesc;

// SysNAND/EmuNAND Backup/Restore Options
extern char *DumpNandFullDesc,
            *DumpNandMinDesc,
            *RestoreNandDesc,
            *RestoreNandForcedDesc,
            *RestoreNandKeepHaxDesc,
            *ValidateNandDumpDesc;

// SysNAND/EmuNAND Transfer Options
extern char *NandTransferDesc,
            *NandForcedTransferDesc,
            *NandDumpTransferDesc,
            *NandAutoFixCtrnandDesc;

// Partition Dump/Inject... (SysNAND/EmuNAND)
extern char *TWLNDesc,
            *TWLPDesc,
            *AGBSAVEDesc,
            *FIRM0Desc,
            *FIRM1Desc,
            *CTRNANDDesc,
            *Sector0x96Desc,
            *NANDHeaderDesc;

// System File Dump/Inject... (SysNAND/EmuNAND)
extern char *TicketDBDesc,
            *TitleDBDesc,
            *ImportDBDesc,
            *CertsDBDesc,
            *SecureInfoDesc,
            *LFCSeedDesc,
            *MovableSEDDesc;


extern char *SeedsaveBinDesc,
            *NagsaveBinDesc,
            *NNIDSaveBinDesc,
            *FriendSaveBinDesc,
            *ConfigSaveBinDesc;

extern char *HealthAndSafetyDesc,
            *GbaVcSaveDesc,
            *SeedDbDesc,
            *CitraConfigDesc,
            *NcchFirmsDesc,
            *FirmArm9FileDesc;

extern char *NcchNcsdCryptoDesc;

extern char *CiaDecryptShallowDesc,
            *CiaDecryptDeepDesc,
            *CiaDecryptCXIDesc,
            *CiaEncryptDesc;

extern char *BOSSCryptoDesc;

extern char *CryptSdFilesDesc,
            *SdDecryptorDesc,
            *SdCXIDumpDesc;

extern char *NcsdNcchToCiaDesc,
            *CiaBuilderDesc;
