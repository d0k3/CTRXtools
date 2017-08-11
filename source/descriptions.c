
// XORpad Generator Options
const char *NcchPadgenDesc        = "Generate XORpads from the contents of "
                                    "ncchinfo.bin in the Work directory.\n\n"

                                    "You can generate this file using ncchinfo_gen.py "
                                    "in the scripts directory.",

           *SdPadgenDesc          = "Generate XORpads from the contents of sdinfo.bin "
                                    "in the Work directory.\n\n"

                                    "You can generate this file using sdinfo_gen.py in "
                                    "the scripts directory.",

           *SdPadgenDirectDesc    = "Generate XORpads for the contents under "
                                    "\"Nintendo 3DS\" based on the target NAND.",

           *AnyPadgenDesc         = "Generate XORpads from from the contents of "
                                    "anypad.bin in the Work directory.\n\n"

                                    "Refer to xorpad.h on the format of this file.",

           *CtrNandPadgenDesc     = "Generate a XORpad for the CTRNAND partition of "
                                    "this 3DS system.\n\n"

                                    "You can extract the CTRNAND partition from a "
                                    "NAND dump using 3DSFAT16Tool.",

           *CtrNandPadgen0x04Desc = "Generate a XORpad for the CTRNAND partition of "
                                    "this 3DS system.\n\n"

                                    "This feature is intended for downgraded New 3DS "
                                    "systems.\n\n"

                                    "You can extract the CTRNAND partition from a "
                                    "NAND dump using 3DSFAT16Tool.",

           *TwlNandPadgenDesc     = "Generate a XORpad for the TWLNAND partition of "
                                    "this 3DS system.\n\n"

                                    "You can extract the TWLNAND partition from a "
                                    "NAND dump using 3DSFAT16Tool.",

           *Firm0Firm1PadgenDesc  = "Generate a XORpad for the FIRM0FIRM1 partitions "
                                    "of this 3DS system.\n\n"

                                    "You can extract these partitions from a "
                                    "NAND dump using 3DSFAT16Tool.";


// Ticket/Titlekey Options
const char *CryptTitlekeysFileDesc     = "Encrypts/encrypts TitleKeys.bin in the Work "
                                         "directory",

           *DumpDecryptedTitlekeysDesc = "Dump decrypted titlekeys from the source NAND to "
                                         "the Work directory.",

           *DumpTitlekeysDesc          = "Dump encrypted titlekeys from the source NAND to "
                                         "the Work directory.",

           *DumpTicketsDesc            = "Dump tickets from the source NAND to the Work "
                                         "directory.";


// Maintenance Options
const char *SystemInfoDesc = "Displays system information.",

           *CreateSelfRef  = "Create a self-test reference for testing purposes.",

           *SelfTestDesc   = "Run self-test checks "
                             "(requires an existing self-test reference file).",

           *BuildKeyDbDesc = "Build aeskeydb.bin from currently loaded keys.",

           *CryptKeyDbDesc = "De/encrypt aeskeydb.bin.";


// Gamecart Dumper Options
const char *DumpGameCartFullDesc    = "Dump the inserted gamecart to the Game directory.",

           *DumpGameCartTrimDesc    = "Dump the inserted gamecart to the Game directory "
                                      "without padding data.",

           *DumpGameCartDecFullDesc = "Dump and decrypt the inserted gamecart to the "
                                      "Game directory.",

           *DumpGameCartDecTrimDesc = "Dump and decrypt the inserted gamecart to the "
                                      "Game directory, without padding data.",

           *DumpGameCartCIADesc     = "Dump and decrypt the inserted gamecart to the "
                                      "Game directory as a ready-to-install CIA file.",

           *DumpPrivateHeaderDesc   = "Dump the private header of the inserted gamecart "
                                      "to the Game directory, for use with flashcarts.",
                                      
           *DumpCartSaveDesc        = "Dump the savegame from the inserted gamecart.\n\n"
           
                                      "Currently only works for NDS type gamecarts.",
                                      
           *FlashCartSaveDesc       = "Flash a savegame to the inserted gamecart.\n\n"
           
                                      "Currently only works for NDS type gamecarts.";


// NDS Flashcart Options
const char *AK2iDumpDesc            = "Dump the AK2i flashcart bootrom to the Game "
                                      "directory.",

           *AK2iFlashDesc           = "Flash the Ak2i flashcart bootrom from the Game "
                                      "directory.\n\n"

                                      "AK2i 1.41 version required. If your cart is not on "
                                      "this version, upgrade or downgrade to it.";


// SysNAND/EmuNAND Backup/Restore Options
const char *DumpNandFullDesc       = "Dump the full target NAND to the Work directory.",

           *DumpNandMinDesc        = "Dump the min-size target NAND to the Work "
                                     "directory.\n\n"

                                     "This is about 1GB for an O3DS/2DS and 1.2GB for "
                                     "a N3DS",

           *RestoreNandDesc        = "Restore target NAND from a file in the Work "
                                     "directory.",

           *RestoreNandForcedDesc  = "Restore target NAND from a file in the Work "
                                     "directory, without checking the hash from a .sha "
                                     "file.",

           *RestoreNandKeepHaxDesc = "Restore target NAND from a file in the Work "
                                     "directory, without overwriting arm9loaderhax.",

           *ValidateNandDumpDesc   = "Validate a NAND dump in the Work directory using "
                                     "a .sha file.";


// SysNAND/EmuNAND Transfer Options
const char *NandTransferDesc       = "Transfer a \"ctrtransfer\" image to the target "
                                     "NAND.",
           *NandForcedTransferDesc = "Transfer a \"ctrtransfer\" image to the target "
                                     "NAND, without checking the hash from a .sha file.",
           *NandDumpTransferDesc   = "Dump a \"ctrtransfer\" image from the source NAND.",
           *NandAutoFixCtrnandDesc = "Automatically fix CMACs for movable.sed, *.db, "
                                     "and system saves in the target NAND.";


// Partition Dump/Inject... (SysNAND/EmuNAND)
const char *TWLNDesc       = "TWL-NAND FAT16 File System\n\n"

                             "Contains files for DSi software, with a similar "
                             "file tree to a Nintendo DSi system.\n"
                             "DSiWare titles are installed here.",

           *TWLPDesc       = "TWL-NAND PHOTO FAT12 File System\n\n"

                             "Contains photos used by DSiWare.",

           *AGBSAVEDesc    = "AGB_FIRM GBA savegame\n\n"

                             "Contains a copy of the last-played GBA VC savegame.",

           *FIRM0Desc      = "Firmware partition\n\n"

                             "Contains NATIVE_FIRM.\n\n"

                             "This is essential for arm9loaderhax to function "
                             "properly, and should not be tampered with.",

           *FIRM1Desc      = "Backup firmware partition\n\n"

                             "Contains NATIVE_FIRM. Used as a backup in case "
                             "FIRM0 is corrupt.\n\n"

                             "This is essential for arm9loaderhax to function "
                             "properly, and should not be tampered with.",

           *CTRNANDDesc    = "CTR-NAND FAT16 File System\n\n"

                             "Contiains the entire 3DS setup. NAND titles (besides "
                             "DSiWare) are installed here.",

           *Sector0x96Desc = "Console-unique encrypted New3DS key-storage\n\n"

                             "Contains keys used by arm9loader at boot time.\n"

                             "This is essential for arm9loaderhax to function "
                             "properly, and should not be tampered with.",

           *NANDHeaderDesc = "NCSD header\n\n"

                             "Contains a signature and a table of partitions.\n\n"

                             "Tampering with this file can cause a bootrom "
                             "error if mistakes are made.";


// System File Dump/Inject... (SysNAND/EmuNAND)
const char *TicketDBDesc   = "Contains titlekeys for installed titles.",

           *TitleDBDesc    = "Contains a database of installed titles on the "
                             "chosen NAND.",

           *ImportDBDesc   = "Contains a database of titles to be installed on "
                             "the chosen NAND.",

           *CertsDBDesc    = "Contains a database of certificates.",

           *SecureInfoDesc = "Contains the system's serial number and region.\n"
                             "Can be used to change the region of the system.",

           *LFCSeedDesc    = "Contains the system's FriendCodeSave.",

           *MovableSEDDesc = "Contains the KeyY used to decrypt the data stored "
                             "under \"Nintendo 3DS\".";


const char *SeedsaveBinDesc     = "Contains the seeds for decryption of 9.6x "
                                  "seed encrypted titles.",

           *NagsaveBinDesc      = "Contains data related to system updates.\n"
                                  "Research on this file is still in progress.",

           *NNIDSaveBinDesc     = "Contains your NNID data - this can be used "
                                  "to reset / remove the NNID from your system, "
                                  "without removing any other data.",

           *FriendSaveBinDesc   = "Contains your actual friendlist - this can "
                                  "be used to backup and restore your friendlist "
                                  "in conjunction with LocalFriendCodeSeed_B.",

           *ConfigSaveBinDesc   = "The config savegame - this contains various "
                                  "things that are set via the config menu.\n\n"
                                  "It also contains a flag telling the system that\n"
                                  "initial setup was already executed.";


const char *HealthAndSafetyDesc = "Dumps/Injects the Health & Safety contents to "
                                  "disk, allowing one to install an unsigned title.\n",

           *GbaVcSaveDesc       = "Only available on SysNAND, use this to dump the "
                                  "GBA VC Savegame from your NAND\n\n"
                                  "Requires slot0x24keyY.bin to work.",

           *SeedDbDesc          = "Use this to create or update the seeddb.bin "
                                  "file on your SD card with the seeds currently "
                                  "installed in your SysNAND/EmuNAND.",

           *CitraConfigDesc     = "Use this to dump the config file, which is "
                                  "required by the Citra emulator for certain games to run.",

           *NcchFirmsDesc       = "Use this to dump all FIRMs from your CTRNAND.",

           *FirmArm9FileDesc    = "Use this to decrypt the ARM9 binary of N3DS FIRMs.";


const char *NcchNcsdCryptoDesc  = "Files with .3DS and .APP extension are typically "
                                  "NCCH / NCSD files.\n\n"

                                  "These usually contain game or appdata.\n"
                                  "Use these options to decrypt/encrypt them\n\n"

                                  "NOTE: Decryption might require extra files "
                                  "while encryption can be done on any FW version.";


const char *CiaDecryptShallowDesc = "Decrypt all CIAs in the Game directory. Decrypts "
                                    "the titlekey layer of CIA crypto, leaves the NCCH "
                                    "untouched.",

           *CiaDecryptDeepDesc    = "Decrypt all CIAs in the Game directory. Decrypts "
                                    "the titlekey layer of CIA crypto and NCCH.\n"
                                    "May require additional files and/or seeddb.bin.",

           *CiaDecryptCXIDesc     = "Decrypt all CIAs in the Game directory. Decrypts "
                                    "the titlekey layer of CIA crypto and NCCH, but "
                                    "only for the first content.\n"
                                    "May require additional files and/or seeddb.bin.",

           *CiaEncryptDesc        = "Encrypt the NCCH containers inside of the CIA "
                                    "files in the Game directory.";


const char *BOSSCryptoDesc      = "Use this to decrypt/encrypt BOSS files.\n"
                                  "This feature will process all BOSS files "
                                  "(with a valid BOSS header) found in the folder.";


const char *CryptSdFilesDesc    = "Use this to decrypt or encrypt 'SD files'.\n"
                                  "For this feature to work, you need to copy "
                                  "the files you want to process to the Work dir.",

           *SdDecryptorDesc     = "This allows you to automatically decrypt files "
                                  "from your SD directory.",

           *SdCXIDumpDesc       = "Similar to the SD Decryptor, but only handles "
                                  "the CXI, uses title_id.cxi as file name "
                                  "and fully decrypts the NCCH.\n\n"

                                  "This is useful to create images for use "
                                  "in Citra from installed content.";

const char *NcsdNcchToCiaDesc   = "Allows you to convert any NCCH/NCSD file to an "
                                  "installable CIA file.",

           *CiaBuilderDesc      = "Allows you to directly convert "
                                  "titles on the SD card to the CIA format.\n"

                                  "It handles content from the /Nintendo 3DS/ "
                                  "subfolder belonging to SysNAND or EmuNAND.\n\n"
  
                                  "The \"orig.\" variant tries to build CIAs as "
                                  "genuine as possible, while still wiping "
                                  "identifying information from the file.\n\n"

                                  "The \"decr.\" variant fully decrypts the CIA "
                                  "files it generates for better compatibility, "
                                  "albeit at a runtime performance cost.";
