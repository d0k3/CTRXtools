# Decrypt9
_Multipurpose content dumper and decryptor for the Nintendo 3DS_

## Decrypt9 WIP (work-in-progress) by d0k3 

This is a work in progress fork of Archshifts original Decrypt9, including bleeding edge new features. Note that the names of the executable files for this are Decrypt9WIP.* instead of Decrypt9.*. New features introduced in this will eventually get pulled into [Archshifts repo](https://github.com/archshift/Decrypt9).

You may discuss this work in progress version, help testing, participate in the development and on [GBAtemp](https://gbatemp.net/threads/download-decrypt9-wip-3dsx-launcher-dat.388831/).

## Decrypt9, Decrypt9WIP, Decrypt9UI - which one to use?

There are at the present time, three main versions of Decrypt9 available:
* __[Decrypt9 by Archshift](https://github.com/archshift/Decrypt9)__: This is the original version of Decrypt9 by Archshift. New features are pulled into this once they are thoroughly tested. This is as stable as it gets, but may also miss some of the newer features.
* __[Decrypt9 WIP by d0k3](https://github.com/d0k3/Decrypt9)__: This is the work in progress fork of Archshifts original Decrypt9. It contains the newest features and is always up to date. Releases in here can be considered tested beta versions.
* __[Decrypt9 UI by Shadowtrance](https://github.com/shadowtrance/Decrypt9)__: This is a themed version of Decrypt9 WIP created by Shadowtrance. It contains a nice graphical user interface (instead of text only as the other two versions), but may not be up to date at all times

## What can I do with this?

See this incomplete list, more detailed descriptions are found further below.
* Create XORpads for decryption of NCCH ('.3DS') files
* Create XORpads for decryption of files in the '/Nintendo 3DS' folder
* Create XORpads for decryption of the TWLN and CTRNAND partitions
* Decrypt Titlekeys, either from a file or directly from SysNAND / EmuNAND
* Backup & restore your SysNAND and EmuNAND
* Dump & Inject any partition from your SysNAND and EmuNAND
* Dump & Inject a number of files (ticket.db, ..) from your SysNAND and EmuNAND
* Inject any app into the Health & Safety app
* Create and update the SeedDB file
* Directly decrypt (_cryptofix_) NCCH ('.3DS') and CIA files
* Directly decrypt files from the '/Nintendo 3DS/' folder
* Dump retail game cartridges
* ... and a lot more

## How to run this / entry points

Decrypt9 can be built to run from a number of entry points, descriptions are below. Note that you need to be on or below 3DS firmware version v9.2 for any of these to work. 
* __A9LH & Brahma__: Copy `Decrypt9.bin` to somewhere on your SD card and run it via either [Brahma](https://github.com/delebile/Brahma2) or [arm9loaderhax](https://github.com/Plailect/Guide/wiki). Brahma derivatives / loaders (such as [BrahmaLoader](https://gbatemp.net/threads/release-easily-load-payloads-in-hb-launcher-via-brahma-2-mod.402857/), [BootCTR](https://gbatemp.net/threads/re-release-bootctr-a-simple-boot-manager-for-3ds.401630/) and [CTR Boot Manager](https://gbatemp.net/threads/ctrbootmanager-3ds-boot-manager-loader-homemenuhax.398383/)) and A9LH chainloaders (such as [Luma3DS](https://github.com/AuroraWright/Luma3DS) and [BootCTR9](https://github.com/hartmannaf/BootCtr9)) will work with this as well. Build this with `make a9lh`.
* __Homebrew Launcher__: Copy Decrypt9.3dsx & Decrypt9.smdh into /3DS/Decrypt9 on your SD card. Run this via [Smealums Homebrew Launcher](http://smealum.github.io/3ds/), [Mashers Grid Launcher](https://gbatemp.net/threads/release-homebrew-launcher-with-grid-layout.397527/) or any other compatible software. Build this with `make brahma`.
* __CakeHax Browser__: Copy Decrypt9.dat to the root of your SD card. For MSET also copy Decrypt9.nds to your SD card. You can then run it via http://dukesrg.github.io/?Decrypt9.dat from your 3DS browser. Build this via `make cakehax`.
* __CakeHax MSET__: Copy Decrypt9.dat to the root of your SD card and Decrypt9.nds to anywhere on the SD card. You can then run it either via MSET and Decrypt9.nds. Build this via `make cakerop`.
* __Gateway Browser Exploit__: Copy Launcher.dat to your SD card root and run this via http://go.gateway-3ds.com/ from your 3DS browser. Build this with `make gateway`. Please note: __this entrypoint is deprecated__. While it may still work at the present time with little to no problems, bugs will no more be fixed and it may be completely removed at a later time. Use CakeHax instead.

If you are a developer and you are building this, you may also just run `make release` to build all files at once. If you are a user, all files are already included in the release archive. When building this, you may also select to compile with one of four available fonts by appending FONT=ORIG/6X10/ACORN/GB to the make command line parameters.

## Working folders

Basically every input file for Decrypt9 can be placed into the SD card root and output files can be written there, too. Working folders are mostly optional. However, using them is recommended and even required for some of the Decrypt9 features to work. These two folders (on the root of your SD card) are used:
* __`/files9/D9Game/`__: NCCH (.3DS), CIA, BOSS, SD card files (from the '/Nintendo 3DS/' folder) go here and are decrypted in place by the respective features. The cart dumper uses this directory as output directory.
* __`/files9/`__: Everything that doesn't go into `/files9/D9Game/` goes here, and this is also the standard output folder. If `/files9/D9Game/` does not exist, NCCH, CIA, BOSS and SD card files are also processed in this folder.

Decryption of game files (NCCH, CIA, BOSS, SD) needs at least one of these two folders to exist. Input files are first searched in `/files9/` and (if not found) then in the SD card root.

## Support files

Depending on the environment, Decrypt9 is ran from, you may need support files to have full functionality. Support files are placed into either the root folder, or the work folder (`/files9/`). Here's a list of support files used in Decrypt9, when you need them and what they are used for:
* __`slot0x05keyY.bin`__: This file was previously needed for access to CTRNAND features (which is basically everything) on N3DS. At the moment it is not needed on any entrypoint.
* __`slot0x25keyX.bin`__: This file is needed to decrypt 7x crypto NCCHs and CIAs on O3DS < 7.0.
* __`slot0x18keyX.bin`__: This file is needed to decrypt Secure 3 crypto NCCHs and CIAs on O3DS without A9LH.
* __`slot0x1BkeyX.bin`__: This file is needed to decrypt Secure 4 crypto NCCHs and CIAs in every environment.
* __`slot0x24keyY.bin`__: This file is needed to properly dump & inject GBA VC savegames.
* __`aeskeydb.bin`__: This is an alternative to the four `slot0x??key?.bin` files mentioned above. It can contain multiple keys. It can be created from your existing `slot0x??key?.bin`files in Decrypt9 via the 'Build Key Database' feature.
* __`seeddb.bin`__: Decrypt9 can create and update this file from the seeds installed in your system. This file is needed to decrypt seed crypto NCCHs and CIAs. Note that your seeddb.bin must also contain the seed for the specific game you need to decrypt.
* __`otp.bin`__: This file is console-unique and is required - on entrypoints other than A9LH - for decryption of the 'secret' sector 0x96 on N3DS (and O3DS with a9lh installed). Refer to [this guide](https://github.com/Plailect/Guide/wiki) for instructions on how to get your own `otp.bin` file.
* __`movable.sed`__: This is dumpable by Decrypt9. It is needed to decrypt SD files from another 3DS, or from another installation on your own 3DS. It is not needed when decrypting your own SysNAND / EmuNAND SD files.
* __`secret_sector.bin`__: A copy of the decrypted, untouched (non-a9lh) `sector0x96.bin`. This is required for decryption of the encrypted ARM9 section of N3DS FIRMs. It is not required for anything else. As an alternative to this you can also provide `slot0x11key95.bin` and `slot0x11key96.bin`.
* __`d9logo.bin`__: Contains a logo (or, in fact, anything) to be displayed on the bottom screen. A `d9logo.bin` file is included in the release archive. If you want to create one yourself, install [ImageMagick](http://www.imagemagick.org/) and run `convert [your image] -rotate 90 bgr:d9logo.bin`, where [your image] is any image of 320x240px resolution.

## Decrypt9 controls

The most important controls are displayed on screen, here is a list of all:
* __DOWN__/__UP__ - Navigate menus, scroll output, select between options.
* __A__ - Enter submenu or confirm action.
* __B__ - Depending on location, leave submenu or cancel.
* __L/R__ - Switch between submenus (first level submenu only).
* __X__ - Make a screenshot. Works in menu and on console output, after a feature finishes.
* __X + LEFT/RIGHT__ - Batch screenshot all submenus / entries (only on menu)
* __SELECT__ - Unmount SD card (only on menu).
* __HOME__ - Reboot the console.
* __POWER__ - Poweroff the console.
* __START (+ LEFT)__ - Reboot (START only) / Poweroff (with LEFT) the console.

There are some features (NAND backup and restore, f.e.), that require the user to choose a file or a directory. In these cases, use the arrow keys (that includes LFET/RIGHT) to select and A / B to confirm and cancel. Also, most file write operations (NAND writes excluded) can be cancelled by holding B.

## Decrypt9 features description

Features in Decrypt9 are categorized into 7 main categories, see the descriptions of each below.

### XORpad Generator Options
This category includes all features that generate XORpads. XORpads are not useful on their own, but they can be used (with additional tools) to decrypt things on your PC. Most, if not all, of the functionality provided by these features can now be achieved in Decrypt9 in a more comfortable way by newer dump/decrypt features, but these are still useful for following older tutorials and to work with other tools.
* __NCCH Padgen__: This generates XORpads for NCCH/NCSD files ('.3DS' f.e.) from `ncchinfo.bin` files. Generate the `ncchinfo.bin` via the included Python script `ncchinfo_gen.py` (or `ncchinfo_tgen.py` for theme packs) and place it into the `/files9/` work folder. Use [Archshift's XORer](https://github.com/archshift/xorer) to apply XORpads to .3DS files. NCCH Padgen is also used in conjunction with [Riku's 3DS Simple CIA Converter](https://gbatemp.net/threads/release-3ds-simple-cia-converter.384559/). Important Note: Depending on you 3DS console type / FW version and the encryption in your NCCH/NCSD files you may need additional files (see 'Support files' above) and / or `seeddb.bin`.
* __SD Padgen (SDinfo.bin)__: This generates XORpads for files installed into the '/Nintendo 3DS/' folder of your SD card. Use the included Python script `sdinfo_gen.py` and place the the resulting `sdinfo.bin` into your `/files9/` work folder. If the SD files to generate XORpads for are from a different NAND (different console, f.e.), you also need the respective `movable.sed` file (dumpable via Dercrypt9) to generate valid XORpads. By now, this feature should only make sense when decrypting stuff from another 3DS - use one of the two features below or the SD Decryptor instead. Use [padXORer by xerpi](https://github.com/polaris-/3ds_extract) to apply XORpads.
* __SD Padgen (SysNAND dir)__: This is basically an improved version of the above feature. For typical users, there are two folders in '/Nintendo 3DS/' on the SD card, one belonging to the SysNAND, the other to EmuNAND. This feature will generate XORpads for encrypted content inside the folder belonging to the SysNAND. It won't touch your SysNAND, thus it is not a dangerous feature. A folder selection prompt will allow you to specify exactly the XORpads you want to be generated (use the arrow keys to select). Generating all of them at once is not recommended, because this can lead to several GBs of data and very long processing time.
* __SD Padgen (EmuNAND dir)__: This is the same as the above feature, but utilizing the EmuNAND folder below '/Nintendo 3DS/' on the SD card. The EmuNAND folder is typically a lot larger than the SysNAND folder, so be careful when selecting the content for which to generate XORpads for.
* __Any Padgen (anypad.bin)__: This feature is a more versatile alternative to various other padgen features. It uses the anypad.bin file as base. For information on the format of this file, refer to `xorpad.h`. A few pointers to get you started: If setNormalKey, setKeyX, setKeyY are non zero, the respective keys in the struct are used, if zero, the keys are unused. ctr array is the initialization vector, or, if either AP_USE_NAND_CTR or AP_USE_SD_CTR the offset of the initialization vector. mode is the AES mode, refer to either `aes.h` or to [3DBrew](https://www.3dbrew.org/wiki/AES_Registers).
* __CTRNAND Padgen__: This generates a XORpad for the CTRNAND partition inside your 3DS console flash memory. Use this with a NAND (SysNAND/EmuNAND) dump from your console and [3DSFAT16Tool](https://gbatemp.net/threads/port-release-3dsfat16tool-c-rewrite-by-d0k3.390942/) to decrypt and re-encrypt the CTRNAND partition on PC. This is useful for any modification you might want to do to the main file system of your 3DS.
* __CTRNAND Padgen (slot0x4)__: This is an N3DS only feature. It is the same as the above option, but forces to use slot0x04 when generating the XORpad. Slot0x04 XORpads are required for decryption and encryption of the CTRNAND partition from downgraded N3DS NAND (SysNAND / EmuNAND) dumps.
* __TWLNAND Padgen__: This generates a XORpad for the TWLNAND partition inside your 3DS console flash memory. Use this with a NAND (SysNAND/EmuNAND) dump from your console and [3DSFAT16Tool](https://gbatemp.net/threads/port-release-3dsfat16tool-c-rewrite-by-d0k3.390942/) to decrypt and re-encrypt the TWLNAND partition on PC. This can be used, f.e. to set up the [SudokuHax exploit](https://gbatemp.net/threads/tutorial-new-installing-sudokuhax-on-3ds-4-x-9-2.388621/).
* __FIRM0FIRM1 Padgen__: This generates the combined XORpad for the FIRM0 and FIRM1 partitions inside your 3DS console flash memory. Use this with a NAND (SysNAND/EmuNAND) dump from your console and [3DSFAT16Tool](https://gbatemp.net/threads/port-release-3dsfat16tool-c-rewrite-by-d0k3.390942/) to decrypt and re-encrypt the FIRM0 / FIRM1 partition on PC. This is useful f.e. for manual installation of the [arm9loaderhax exploit](https://github.com/delebile/arm9loaderhax).

### Titlekey Options
This category includes all titlekey related features. Decrypted titlekeys (`decTitleKeys.bin`) are used to download software from CDN via the included Python script `cdn_download.py` and [PlaiCDN](https://github.com/Plailect/PlaiCDN). Encrypted titlekeys are used, for the same purpose, by [FunKeyCIA](https://github.com/llakssz/FunKeyCIA/). You may also view the (encrypted or decrypted) titlekeys via `print_ticket_keys.py`.
* __Titlekey Decrypt (file)__: First, generate the `encTitleKeys.bin` via the included Python script `dump_ticket_keys.py` and place it into the `/files9/` work folder. This feature will decrypt the file and generate the `decTitleKeys.bin`, containing the decrypted titlekeys.
* __Titlekey Encrypt (file)__: This feature takes a `decTitleKeys.bin` file and encrypts it to `encTitleKeys.bin`. This is useful to convert between the two formats, to make sure you have the right format for the tools you use.
* __Titlekey Decrypt (SysNAND)__: This will find and decrypt all the titlekeys contained on your SysNAND, without the need for additional tools. The `decTitleKeys.bin` file will be generated on your SD card.
* __Titlekey Decrypt (EmuNAND)__: This will find and decrypt all the titlekeys contained on your EmuNAND, without the need for additional tools. The `decTitleKeys_emu.bin` file will be generated on your SD card.
* __Titlekey Dump (SysNAND)__: This will find all the titlekeys contained on your SysNAND and dump them, without the additional step of decryption, to `encTitleKeys.bin`.
* __Titlekey Dump (EmuNAND)__: This will find all the titlekeys contained on your EmuNAND and dump them, without the additional step of decryption, to `encTitleKeys_emu.bin`.
* __Ticket Dump (SysNAND)__: Use this to dump all tickets found inside your SysNAND ticket.db file. Naming scheme for tickets is (commonkey index)-(console id)-(title id).tik. Commonkey index is typically 0 for eShop titles and 1 for system titles, a console id of zero for eShop titles typically means a forged (= non genuine) ticket. Dumped tickets are installable via [FBI](https://github.com/Steveice10/FBI/releases).
* __Ticket Dump (EmuNAND)__: Same as above, but uses the ticket.db file from your EmuNAND. 

### SysNAND / EmuNAND Options
This is actually two categories in the main menu, but the functionality provided the same (for SysNAND / EmuNAND respectively). These categories include all features that dump, inject, modify or extract information from/to the SysNAND/EmuNAND. For functions that output files to the SD card, the user can choose a filename from a predefined list. For functions that use files from the SD card for input, the user can choose among all candidates existing on the SD card. For an extra layer of safety, critical(!) features - meaning all features that actually introduce change to the NAND - are protected by a warning message and an unlock sequence that the user has to enter. Caution is adviced with these protected features. They should only be used by the informed user.
* __(Sys/Emu)NAND Backup & Restore...__: This contains multiple options to backup or restore your SysNAND or EmuNAND. The submenu contains the following entries:
  * __NAND Backup__: Dumps the `NAND.bin` file from your SysNAND or the `ÈmuNAND.bin` file from your EmuNAND. This is a full backup of your 3DS System NAND and can be used to restore your 3DS SysNAND / EmuNAND to a previous state or for modifications. 
  * __NAND Backup (minsize)__: Same as the above option, but only dumps the actually used size of the NAND (the remainder is only unused data). Use this instead of the above to save some space on your SD card.
  * __NAND Restore(!)__: This fully restores your SysNAND or EmuNAND from the provided `NAND.bin` file (needs to be in the `/files9/` work folder or in the SD card root). Although backups will be checked before restoring, be careful not to restore a corrupted NAND.bin file. Also note that you won't have access to this feature if your SysNAND is too messed up or on a too high FW version to even start Decrypt9 (should be self explanatory).
  * __NAND Restore (forced)(!)__: Same as the above option, but skips most safety checks. This is not recommended to be used without being properly informed. Keep in mind that, if the above option stops you from restoring a NAND backup, it is normally with good reason and means a prevented brick.
  * __NAND Restore (keep a9lh)(!)__: Only available on SysNAND, this is the same as the standard (unforced) restore option, but keeps all your arm9loaderhax files intact. Only use if you actually have arm9loaderhax installed.
  * __Validate NAND Dump__: Use this to check and verify NAND dumps on your SD card. If this check passes on a NAND dump, you will also be able to restore it via the standard restore option.
* __CTRNAND Transfer...__: This menu contains various options to enable transfer of CTRNAND partitions between consoles.
  * __Auto CTRNAND Transfer__: Automatically transfer a transferable CTRNAND image to this consoles NAND. Without A9LH installed, this will overwrite the FIRM0, FIRM1, CTRNAND. With A9LH installed, this will only overwrite CTRNAND. O3DS images can be transferred into N3DS consoles, but the NCSD header of the NAND may be overwritten.
  * __Dump transferable CTRNAND__: Dump a CTRNAND image for later use in the feature above. Transferables images can be shared between consoles.
  * __Autofix CTRNAND__: Use this to automatically fixes the CMACs for `movable.sed`, `*.db` and system saves inside the CTRNAND. It will also fix the &lt;id0&gt; inside the data folder. This is useful f.e. when a CTRNAND from another console was previously injected the regular way.
* __Partition Dump...__: This allows you to dump & decrypt any of the partitions inside your NANDs (TWLN / TWLP / AGBSAVE / FIRM0 / FIRM1 / CTRNAND / Sector0x96). Partitions with a file system (TWLN / TWLP / CTRNAND) can easily be mounted, viewed and edited on Windows via [OSFMount](http://www.osforensics.com/tools/mount-disk-images.html). These partitions are included in your NAND and can be dumped by this feature:
  * __TWLN__: _TWL-NAND FAT16 File System_ - this is the same as on a Nintendo DSi console. Installed DSiWare titles reside in this partition. This partition can be used, f.e. to set up [SudokuHax](https://gbatemp.net/threads/tutorial-new-installing-sudokuhax-on-3ds-4-x-9-2.388621/).
  * __TWLP__: _TWL-NAND PHOTO FAT12 File System_ - this is a Nintendo DSi specific partition for storing photos.
  * __AGBSAVE__: _AGB_FIRM GBA savegame_ - this contains a temporary copy of the current GBA games savegame.
  * __FIRM0__: _Firmware partition_ - this is here for development purposes only and should not be changed by users.
  * __FIRM1__: _Firmware partition backup_ - usually an exact copy of FIRM0.
  * __CTRNAND__: _CTR-NAND FAT16 File System_ - this contains basically you complete 3DS setup. Titles installed to the NAND reside here, and you can extract basically any file of interest from this partition.
  * __Sector0x96__: _Console-unique encrypted New3DS key-storage_ - this contains N3DS keys, access to it is required for A9LH installation.
  * __NAND header__: _NCSD header_ - the header of your NAND, this is is console-unique and contains the offsets/sizes of all partitions. It also contains the encrypted TWL MBR partition table.
* __Partition Inject...(!)__: This allows you to reencrypt & inject any of the partitions on your NAND from the respective files (`TWLN.bin` / `TWLP.bin` / `AGBSAVE.bin` / `FIRM0.bin` / `FIRM1.bin` / `CTRNAND.bin` / `Sector0x96.bin`) (see above). Only use this if you know exactly what you're doing and be careful. While there are some safety clamps in place, they won't protect you from a major messup caused by yourself.
* __System File Dump...__: This allows you to directly dump & decrypt various files of interest from your SysNAND and EmuNAND. These files are included in this feature:
  * __ticket.db__: _Contains titlekeys for installed titles_ - use this with [Cearps FunkyCIA](https://gbatemp.net/threads/release-funkycia2-build-cias-from-your-eshop-content-super-easy-and-fast-2-1-fix.376941/) to download installed (legit, purchased) titles directly to your PCs ard drive.
  * __title.db__: _A database of installed titles_ - apart from informative purposes this doesn't serve a direct purpose for most users at the moment.
  * __import.db__: _A database of titles to be installed_ - this can be used to get rid of the FW update nag at a later time. Read more on it in this [GBAtemp thread](https://gbatemp.net/threads/poc-removing-update-nag-on-emunand.399460/).
  * __certs.db__: _A database of certificates_ - any practical use for this is unknown at the moment.
  * __SecureInfo_A__: _This contains your region and an ASCII serial number_ - this can be used to temporarily change your 3DS region. The dump / inject options in Decrypt9 simplify [the tutorial found here](https://gbatemp.net/threads/release-3ds-nand-secureinfo-tool-for-region-change.383792/).
  * __LocalFriendCodeSeed_B__: _This contains your FriendCodeSeed_ - in theory this can be used to import your friend list to another 3DS.
  * __movable.sed__: _This contains the keyY for decryption of data on the SD card_ - Decrypt9 itself uses this in the SD Decryptor / Encryptor and in SD padgen.
* __System File Inject...(!)__: This allows you to directly encrypt & inject various files of interest into the SysNAND and EmuNAND. For more information check out the list above.
* __System Save Dump...__: This allows you to directly dump & decrypt various system saves from your SysNAND and EmuNAND. These files are included in this feature: 
  * __seedsave.bin__: _Contains the seeds for decryption of 9.6x seed encrypted titles_ - only the seeds for installed (legit, purchased) titles are included in this. Use [SEEDconv](https://gbatemp.net/threads/download-seedconv-seeddb-bin-generator-for-use-with-decrypt9.392856/) (recommended) or the included Python script `seeddb_gen.py` to extract the seeds from this into the Decrypt9 readable `seeddb.bin`.
  * __nagsave.bin__: _Contains some data relating to system updates_ - it is possible to block automatic system updates (ie. the 'update nag') with this file. Research is still in progress. [Read this](https://gbatemp.net/threads/poc-removing-update-nag-on-emunand.399460/page-5#post-5863332) and the posts after it for more information.
  * __nnidsave.bin__: _Contains your NNID data_ - this can be used to reset / remove the NNID from your system, without removing any other data. See [here](https://gbatemp.net/threads/download-decrypt9-open-source-decryption-tools-wip.388831/page-89#post-6000951) for instructions.
  * __friendsave.bin__: _Contains your actual friendlist_ - this can be used to backup and restore your friendlist in conjunction with `LocalFriendCodeSeed_B`. Also see [here](http://gbatemp.net/threads/download-decrypt9-open-source-decryption-tools-wip.388831/page-167#post-6294331).
  * __configsave.bin__: _The config savegame_ - this contains various things that are set via the config menu. It also contains a flag telling the system that initial setup was already executed.
* __System Save Inject...(!)__: This allows you to directly encrypt & inject various system saves into the SysNAND and EmuNAND. For more information check out the list above.
* __Miscellaneous..__: This section contains various features that don't fit into any of the other categories.
  * __Health&Safety Dump__: This allows you to to dump the decrypted Health and Safety system app to your SD card. The dumped H&S app can be used to [create injectable files for any homebrew software](https://gbatemp.net/threads/release-inject-any-app-into-health-safety-o3ds-n3ds-cfw-only.402236/).
  * __Health&Safety Inject(!)__: This is used to inject any app into your Health & Safety system app (as long as it is smaller than the original H&S app). Multiple safety clamps are in place, and this is a pretty safe feature. Users are still adviced to be cautious using this and only use eiter the original hs.app or inject apps created with the [Universal Inject Generator](https://gbatemp.net/threads/release-inject-any-app-into-health-safety-o3ds-n3ds-cfw-only.402236/). This feature will detect all injectable apps on the SD card and let the user choose which one to inject.
  * __GBA VC Save Dump__: Only available on SysNAND, use this to dump the GBA VC Savegame from your NAND. Other than the headered `AGBSAVE.bin` format, this allows usage in emulators. `slot0x24keyY.bin` is required for this to work. For info on how to use this and the below feature, see [here](https://gbatemp.net/threads/download-decrypt9-open-source-decryption-tools-wip.388831/page-196#post-6615811) or [here](http://gbatemp.net/threads/download-decrypt9-open-source-decryption-tools-wip.388831/page-214#post-6910891).
  * __GBA VC Save Inject__: Only available on SysNAND, use this to inject back a GBA VC Savegame (f.e. after manual editing) to your NAND. Same as above, `slot0x24keyY.bin` is required for this to work.
  * __Update SeedDB__: Use this to create or update the `seeddb.bin` file on your SD card with the seeds currently installed in your Sys/EmuNAND. Only new seeds will get added to `seeddb.bin`, seeds already in the database stay untouched.
  * __Dump Config (for Citra)__: Use this to dump the `config` file, which is required by the [Citra emulator](https://citra-emu.org/) for certain games. Also see [here](https://github.com/citra-emu/citra/wiki/Home-Folder).
  * __NCCH FIRMs Dump__: Use this to dump NATIVE_FIRM, SAFE_MODE_FIRM, TWL_FIRM and AGB_FIRM from your NAND. For N3DS FIRMs, the ARM9 section will be decrypted as well. This feature is at the moment only useful for research.
  * __FIRM ARM9 Decryptor__: Use this to decrypt the ARM9 section of N3DS FIRMs. This feature is at the moment only useful for research.

### Content Decryptor Options
This category includes all features that allow the decryption (and encryption) of external and internal content files. Content files are directly processed - the encrypted versions are overwritten with the decrypted ones and vice versa, so keep backups. The standard work folder for content files is `/files9/D9Game/`, but if that does not exist, content files are processed inside the `/files9/` work folder.
* __NCCH/NCSD File Options__: Files with .3DS and .APP extension are typically NCCH / NCSD files. NCCH/NCSD typically contain game or appdata.
  * __NCCH/NCSD Decryptor__: Use this to fully decrypt all NCCH / NCSD files in the folder. A full decryption of a .3DS file is otherwise also known as _cryptofixing_. Important Note: Depending on you 3DS console type / FW version and the encryption in your NCCH/NCSD files you may need additional files key files (see 'Support files' above) and / or `seeddb.bin`.
  * __NCCH/NCSD Encryptor__: Use this to (re-)encrypt all NCCH / NCSD files in the folder using standard encryption (f.e. after decrypting them). Standard encryption can be processed on any 3DS, starting from the lowest firmware versions. On some hardware, .3DS files might need to be encrypted for compatibility.
* __CIA File Options__: CIA files are 'Content Installable Files', this entry contains all related features.
  * __CIA Decryptor (shallow)__: Use this to decrypt, for all CIA files in the folder, the titlekey layer of CIA decryption. The internal NCCH encryption is left untouched.
  * __CIA Decryptor (deep)__:  Use this to fully decrypt all CIA files in the folder. This also processes the internal NCCH encryption. Deep decryption of a CIA file is otherwise known as _cryptofixing_. This also may need additional key files and / or `seeddb.bin`, see 'Support files' above.
  * __CIA Decryptor (CXI only)__: This is the same as CIA Decryptor (deep), but it does not process the 'deep' NCCH encryption for anything but the first CXI content. On some hardware, fully deep decrypted CIA files might not be installable, but CIA files processed with this feature will work.
  * __CIA Encryptor (NCCH)__: Use this to encrypt the NCCH containers inside of the CIA files in the folder. NCCH encryption is required, for example, for system CIA files to be installable.
* __BOSS File Options__: [BOSS files](http://3dbrew.org/wiki/SpotPass#Content_Container) are typically received via Spotpass, this entry contains all related features.
  * __BOSS Decryptor__: Use this to decrypt BOSS files. This feature will decrypt all encrypted BOSS files (with a valid BOSS header) found in the folder.
  * __BOSS Encryptor__: Use this to encrypt BOSS files. This feature will encrypt all unencrypted BOSS files (with a valid BOSS header) found in the folder.
* __SD File Options__: SD files are titles, extdata and databases found inside the /Nintendo 3DS/<id0>/<id1>/ folder. This entry contains all related features.
  * __SD Decryptor/Encryptor__: Use this to decrypt or encrypt 'SD files'. For this feature to work, you need to manually copy the file(s) you want to process. Copy them with their full folder structure (that's everything _after_ `/Nintendo 3DS/<id0>/<id1>/`) to the work / game folder. This feature should by now only be useful to encrypt content, decryption is much easier handled by the two features below.
  * __SD Decryptor (SysNAND dir)__: An improved version of the feature above. This allows you to select content from `/Nintendo 3DS/` (more specifically from the subfolder belonging to SysNAND) to be directly copied to your work / game folder and then decrypted from there.
  * __SD Decryptor (EmuNAND dir)__: This has the same functionality as the feature above, but handles the content of the `/Nintendo 3DS/` subfolder belonging to the EmuNAND instead.
  * __SD CXI Dumper (SysNAND dir)__: This feature is similar to the SD Decryptor, but only handles the CXI (CTR eXecutable Image), uses `title_id.cxi` as file name and fully decrypts the NCCH. This is useful to create images for use in [Citra](https://citra-emu.org/) from installed content. This handles content from the `/Nintendo 3DS/` subfolder belonging to SysNAND.
  * __SD CXI Dumper (EmuNAND dir)__: This has the same functionality as the feature above, but handles the content of the `/Nintendo 3DS/` subfolder belonging to the EmuNAND instead.
* __CIA Builder Options__: This subsection contains various features that allow you to build CIAs from files installed to / stored on your SD card.
  * __Build CIA from NCCH/NCSD__:  This was previously called 'NCCH/NCSD to CIA Converter'. It allows you to convert any NCCH/NCSD file (that means .3DS files, too) to an installable (on a signature patched system) CIA file. The CIA file will be written to `filename.ext.cia`.
  * __CIA Builder (SysNAND/orig.)__: This was previously called 'Content to CIA (SysNAND dir)'. This feature allows you to directly convert content installed to the SD card to a CIA file. It handles content from the `/Nintendo 3DS/` subfolder belonging to SysNAND. This variant of the feature tries to build CIAs as genuine as possible, while still wiping identifying information from the file. If you need similar functionality for titles installed to NAND, use [ctrnand-title-cia-gen](https://github.com/ihaveamac/ctrnand-title-cia-gen) by ihaveamac.
  * __CIA Builder (EmuNAND/orig.)__: This was previously called 'Content to CIA (EmuNAND dir)__' and has the same functionality as the feature above, but handles the content of the `/Nintendo 3DS/` subfolder belonging to the EmuNAND instead.
  * __CIA Builder (SysNAND/decr.)__: This feature has the same functionality as the features above. In addition, it fully decrypts the CIA files it generates for better compatibility. Take note that the additional decryption and recalculation of checksums mean longer processing times. This handles content from the `/Nintendo 3DS/` subfolder belonging to SysNAND.
  * __CIA Builder (EmuNAND/decr.)__: This has the same functionality as the feature above, but handles the content of the `/Nintendo 3DS/` subfolder belonging to the EmuNAND instead.

### Gamecart Dumper Options
This category includes all features handling dumping of content from external cartridges. Cartridge dumps are also known as .3ds files.
* __Dump Cart (full)__: This feature dumps the full, unaltered data from the inserted cartridge. For 4GB cartridges, the last sector is silently discarded, because the FAT32 file system can't handle files equal or above 4GB. This feature also handles NTR/TWL cartridges (aka. NDS and DSi crtridges).
* __Dump Cart (trim)__: Same as the above feature, but discards the unused padding for smaller output and faster processing. Using this is recommended unless the padding is required for digital preservation purposes.
* __Dump & Decrypt Cart (full)__: Same as 'Dump Cart (full)', but also decrypts the cartridge data on-the-fly. Decrypted cartridge data is required for emulators and recommended for CIA conversion. The recommended CIA conversion tool is [3dsconv](https://github.com/ihaveamac/3dsconv). NTR/TWL cartridges are not encrypted and thus won't be decrypted.
* __Dump & Decrypt Cart (trim)__: Same as above, but discards the unused padding for smaller output and faster processing. This is recommended over the above feature.
* __Dump Cart to CIA__: Use this to directly dump an inserted cartridge to a fully decrypted CIA file, which can be installed to a patched system using CIA installer software like [FBI](https://github.com/Steveice10/FBI/releases). For most users, this type of dump will be the most convenient. NTR/TWL cartridges can't be dumped to a CIA file.
* __Dump Private Header__: Dumps the cartridge unique private header from the inserted cartridge.
* __Flash Savegame to Cart__: Flash a savegame file to a retail game cartridge. This currently only works for NTR/TWL carts. The savegame to flash must have a filename of ndscart*.sav.

### NDS Flashcart Options
This category includes special features for certain NDS type flashcarts (currently only the AK2i).
* __Auto NTRCARDHAX to AK2i__: This performs an automatic NTRCARDHAX injection to the AK2i flashcart.
* __Dump AK2i__: Use this to dump the AK2i flashcart bootrom to the game directory.
* __Inject AK2i__: Use this to inject the AK2i flashcart bootrom from the game directory.
* __Inject NTRCARDHAX to AK2i__: Patch and inject the NTRCARDHAX payload to the AK2i flashcart. AK2i 1.41 version is required. If your AK2i is not on this version, upgrade or downgrade to it.
* __Restore AK2i bootrom__: Restore the AK2i flashcart original bootrom.

### Maintenance Options
This category includes special features which allow you to test and manage Decrypt9 internal functionality. 
* __System Info__: Displays various information about your 3DS and SD card on screen. Used for informational purposes and to test if information is available.
* __Create Selftest Reference__: Run this first on a known working entrypoint to generate the selftest reference data. This will create a file called `d9_selftest.ref` inside your SD card root or work folder.
* __Run Selftest__: Run the actual selftest (must have created the reference data before). This will create or update a file called `d9_selftest.lst` on your SD card root or work folder. Note: on O3DS failed `ncch_sec3_key`, `ncch_sec4_key` and `nand_ctrn_key` tests are normal and expected. On O3DS <= FW 7.0, `ncch_7x_key` may fail. On N3DS and O3DS `ncch_sec4_key` may fail. With all key files (see 'Support files') available, no test should fail. This is used, for example, to assure that everything works as intended on a new entrypoint.
* __Build Key Database__: This is used to build the `aeskeydb.bin` file from all available `slot0x??key?.bin` files. It will also process files that are not used by Decrypt9. With the `aeskeydb.bin`available, Decrypt9 can load keys from it and doesn't need the `slot0x??key?.bin` files anymore.
* __De/Encrypt Key Database__: By default, the `aeskeydb.bin` created in Decrypt9 is encrypted, and the keys in it are not readable via a Hex Editor. Use this to either decrypt an encrypted database, or to encrypt a decrypted one. Note that Decrypt9 can load keys from both. 

## License
You may use this under the terms of the GNU General Public License GPL v2 or under the terms of any later revisions of the GPL. Refer to the provided `LICENSE.txt` file for further information.

## Credits by Archshift
* Roxas75 for the method of ARM9 code injection
* Cha(N), Kane49, and all other FatFS contributors for FatFS
* Normmatt for `sdmmc.c` as well as project infrastructure (Makefile, linker setup, etc)
* Relys, sbJFn5r for the decryptor

## Credits by d0k3
* Everyone mentioned by Archshift above
* Archshift for starting this project and being a great project maintainer
* b1l1s, Normmatt for their 'behind-the-scenes' work and for making arm9loaderhax support possible
* Gelex for various code improvements and useful advice throughout D9 development
* ihaveamac for first developing the simple CIA generation method and for being of great help in porting it
* patois, delebile, SteveIce10 for Brahma and it's updates
* mid-kid for CakeHax and for hosting freenode #Cakey
* Shadowtrance, Syphurith, AuroraWright for being of great help developing various features
* dark_samus3 and Plailect for making CTRNAND transfers a possibility
* osilloscorpion and idgrepthat for enabling NTR cart dumps
* profi200 for helpful hints that first made developing some features possible
* Al3x_10m for helping me with countless hours of testing and useful advice
* Shadowhand for being awesome and [hosting my nightlies](https://d0k3.secretalgorithm.com/)
* SciresM and Reisyukaku for helping me allow devkit compatibility
* liomajor, Datalogger, zoogie, atkfromabove, mixups, key1340, k8099, Supster131, stbinan, Wolfvak, imanoob, Stary2001, kasai07 and countless others from freenode #Cakey and the GBAtemp forums for testing, feedback and helpful hints
* Everyone I forgot about - if you think you deserve to be mentioned, just contact me
