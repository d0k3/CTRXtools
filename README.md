# Decrypt9
_Open source decryption tools for the Nintendo 3DS console_

## Decrypt9 WIP (work-in-progress) by d0k3 

This is a work in progress fork of Archshifts original Decrypt9, including bleeding edge new features. Note that the names of the exectuable files for this are Decrypt9WIP.* instead of Decrypt9.*. New features introduced in this will eventually get pulled into [Archshifts repo](https://github.com/archshift/Decrypt9).

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

## How to run this / entry points

Decrypt9 can be built to run from a number of entry points, descriptions are below. Note that you need to be on or below 3DS firmware version v9.2 for any of these to work. 
* __Gateway Browser Exploit__: Copy Launcher.dat to your SD card root and run this via http://go.gateway-3ds.com/ from your 3DS browser. Build this with `make gateway`.
* __A9LH, Brahma & Bootstrap__: Copy Decrypt9.bin to somewhere on your SD card and run it via either [Brahma](https://github.com/delebile/Brahma2) or [Bootstrap](https://github.com/shinyquagsire23/bootstrap). Brahma derivatives / loaders such as [BrahmaLoader](https://gbatemp.net/threads/release-easily-load-payloads-in-hb-launcher-via-brahma-2-mod.402857/), [BootCTR](https://gbatemp.net/threads/re-release-bootctr-a-simple-boot-manager-for-3ds.401630/) and [CTR Boot Manager](https://gbatemp.net/threads/ctrbootmanager-3ds-boot-manager-loader-homemenuhax.398383/) will also work with this. Build this with `make bootstrap`.
* __Homebrew Launcher__: Copy Decrypt9.3dsx & Decrypt9.smdh into /3DS/Decrypt9 on your SD card. Run this via [Smealums Homebrew Launcher](http://smealum.github.io/3ds/), [Mashers Grid Launcher](https://gbatemp.net/threads/release-homebrew-launcher-with-grid-layout.397527/) or any other compatible software. Build this with `make brahma`.
* __CakeHax Browser__: Copy Decrypt9.dat to the root of your SD card. For MSET also copy Decrypt9.nds to your SD card. You can then run it via http://dukesrg.github.io/?Decrypt9.dat from your 3DS browser. Build this via `make cakehax`.
* __CakeHax MSET__: Copy Decrypt9.dat to the root of your SD card and Decrypt9.nds to anywhere on the SD card. You can then run it either via MSET and Decrypt9.nds. Build this via `make cakerop`.

If you are a developer and you are building this, you may also just run `make release` to build all files at once. If you are a user, all files are already included in the release archive.

## Working folders

Basically every input file for Decrypt9 can be placed into the SD card root and output files can be written there, too. Working folders are mostly optional. However, using them is recommended and even required for some of the Decrypt9 features to work. These two folders (on the root of your SD card) are used:
* __`/D9Game/`__: NCCH (.3DS), CIA and SD card files (from the '/Nintendo 3DS/' folder) go here and are decrypted in place by the respective features.
* __`/Decrypt9/`__: Everything that doesn't go into `/D9Game/` goes here, and this is also the standard output folder. If `/D9Game/` does not exist, NCCH, CIA and SD card files are also processed in this folder.

Decryption of game files (NCCH, CIA, SD) needs at least one of these two folders to exist. Input files are first searched in `/Decrypt9/` and (if not found) then in the SD card root.

## Support files

Depending on the environment, Decrypt9 is ran from, you may need support files to have full functionality. Support files are placed into either the root folder, or the work folder (`/Decrypt9/`). Here's a list of support files used in Decrypt9, when you need them and what they are used for:
* __`slot0x05keyY.bin`__: This file is needed for access to CTRNAND features (which is basically everything) on N3DS when running from A9LH.
* __`slot0x25keyX.bin`__: This file is needed to decrypt 7x crypto NCCHs and CIAs on O3DS < 7.0 and on A9LH.
* __`slot0x18keyX.bin`__: This file is needed to decrypt Secure 3 crypto NCCHs and CIAs on O3DS and on A9LH.
* __`slot0x1BkeyX.bin`__: This file is needed to decrypt Secure 4 crypto NCCHs and CIAs in every environment.
* __`seeddb.bin`__: This file is needed to decrypt seed crypto NCCHs and CIAs. Note that your seeddb.bin must also contain the seed for the specific game you need to decrypt.
* __`movable.sed`__: This is dumpable by Decrypt9. It is needed to decrypt SD files from another 3DS, or from another installation on your own 3DS. It is not needed when decrypting your own SysNAND / EmuNAND SD files.

## Decrypt9 features description

Features in Decrypt9 are categorized into 5 main categories, see the descriptions of each below.

### XORpad Generator Options
This category includes all features that generate XORpads. XORpads are not useful on their own, but they can be used (with additional tools) to decrypt things on your PC. Most, if not all, of the functionality provided by these features can now be achieved in Decrypt9 in a more comfortable way by newer dump/decrypt features, but these are still useful for following older tutorials and to work with other tools.
* __NCCH Padgen__: This generates XORpads for NCCH/NCSD files ('.3DS' f.e.) from `ncchinfo.bin` files. Generate the `ncchinfo.bin` via the included Python script `ncchinfo_gen.py` (or `ncchinfo_tgen.py` for theme packs) and place it into the `/Decrypt9/` work folder. Use [Archshift's XORer](https://github.com/archshift/xorer) to apply XORpads to .3DS files. NCCH Padgen is also used in conjunction with [Riku's 3DS Simple CIA Converter](https://gbatemp.net/threads/release-3ds-simple-cia-converter.384559/). Important Note: Depending on you 3DS console type / FW version and the encryption in your NCCH/NCSD files you may need additional files `slot0x??KeyX.bin` and / or `seeddb.bin`. You're on your own for `slot0x25KeyX.bin`, but for `seeddb.bin` [this](http://tinivi.net/seeddb/) will help you.
* __SD Padgen (SDinfo.bin)__: This generates XORpads for files installed into the '/Nintendo 3DS/' folder of your SD card. Use the included Python script `sdinfo_gen.py` and place the the resulting `sdinfo.bin` into your `/Decrypt9/` work folder. If the SD files to generate XORpads for are from a different NAND (different console, f.e.), you also need the respective `movable.sed` file (dumpable via Dercrypt9) to generate valid XORpads. By now, this feature should only make sense when decrypting stuff from another 3DS - use one of the two features below or the SD Decryptor instead. Use [padXORer by xerpi](https://github.com/polaris-/3ds_extract) to apply XORpads.
* __SD Padgen (SysNAND dir)__: This is basically an improved version of the above feature. For typical users, there are two folders in '/Nintendo 3DS/' on the SD card, one belonging to the SysNAND, the other to EmuNAND. This feature will generate XORpads for encrypted content inside the folder belonging to the SysNAND. It won't touch your SysNAND, thus it is not a dangerous feature. A folder selection prompt will allow you to specify exactly the XORpads you want to be generated (use the arrow keys to select). Generating all of them at once is not recommended, because this can lead to several GBs of data and very long processing time.
* __SD Padgen (EmuNAND dir)__: This is the same as the above feature, but utilizing the EmuNAND folder below '/Nintendo 3DS/' on the SD card. The EmuNAND folder is typically a lot larger than the SysNAND folder, so be careful when selecting the content for which to generate XORpads for.
* __CTRNAND Padgen__: This generates a XORpad for the CTRNAND partition inside your 3DS console flash memory. Use this with a NAND (SysNAND/EmuNAND) dump from your console and [3DSFAT16Tool](https://gbatemp.net/threads/port-release-3dsfat16tool-c-rewrite-by-d0k3.390942/) to decrypt and re-encrypt the CTRNAND partition on PC. This is useful for any modification you might want to do to the main file system of your 3DS.
* __TWLNAND Padgen__: This generates a XORpad for the TWLNAND partition inside your 3DS console flash memory. Use this with a NAND (SysNAND/EmuNAND) dump from your console and [3DSFAT16Tool](https://gbatemp.net/threads/port-release-3dsfat16tool-c-rewrite-by-d0k3.390942/) to decrypt and re-encrypt the TWLNAND partition on PC. This can be used, f.e. to set up the [SudokuHax exploit](https://gbatemp.net/threads/tutorial-new-installing-sudokuhax-on-3ds-4-x-9-2.388621/).
* __FIRM0FIRM1 Padgen__: This generates the combined XORpad for the FIRM0 and FIRM1 partitions inside your 3DS console flash memory. Use this with a NAND (SysNAND/EmuNAND) dump from your console and [3DSFAT16Tool](https://gbatemp.net/threads/port-release-3dsfat16tool-c-rewrite-by-d0k3.390942/) to decrypt and re-encrypt the FIRM0 / FIRM1 partition on PC. This is useful f.e. for manual installation of the [arm9loaderhax exploit](https://github.com/delebile/arm9loaderhax).

### Titlekey Decrypt Options
This category includes all titlekey decryption features. Decrypted titlekeys (`decTitleKeys.bin`) are used to download software from CDN via the included Python script `cdn_download.py`. You may also view the (encrypted or decrypted) titlekeys via `print_ticket_keys.py`.
* __Titlekey Decrypt (file)__: First, generate the `encTitleKeys.bin` via the included Python script `dump_ticket_keys.py` and place it into the `/Decrypt9/` work folder. This feature will decrypt the file and generate the `decTitleKeys.bin`, containing the decrypted titlekeys.
* __Titlekey Decrypt (SysNAND)__: This will find and decrypt all the titlekeys contained on your SysNAND, without the need for additional tools. The `decTitleKeys.bin` will be generated on your SD card.
* __Titlekey Decrypt (EmuNAND)__: This will find and decrypt all the titlekeys contained on your EmuNAND, without the need for additional tools. The `decTitleKeys_emu.bin` will be generated on your SD card.

### SysNAND / EmuNAND Options
This is actually two categories in the main menu, but the functionality provided the same (for SysNAND / EmuNAND respectively). These categories include all features that dump, inject, modify or extract information from/to the SysNAND/EmuNAND. For functions that output files to the SD card, the user can choose a filename from a predefined list. For functions that use files from the SD card for input, the user can choose among all candidates existing on the SD card. For an extra layer of safety, critical(!) features - meaning all features that actually introduce change to the NAND - are protected by a warning message and an unlock sequence that the user has to enter. Caution is adviced with these protected features. They should only be used by the informed user.
* __(Emu)NAND Backup__: Dumps the `NAND.bin` file from your SysNAND or the `ÈmuNAND.bin` file from your EmuNAND. This is a full backup of your 3DS System NAND and can be used to restore your 3DS SysNAND / EmuNAND to a previous state or for modifications.
* __(Emu)NAND Restore(!)__: This fully restores your SysNAND or EmuNAND from the provided `NAND.bin` file (needs to be in the `/Decrypt9/` work folder). Be careful not to restore a corrupted NAND.bin file. Also note that you won't have access to this feature if your SysNAND is too messed up or on a too high FW version to even start Decrypt9 (should be self explanatory).
* __Partition Dump...__: This allows you to dump & decrypt any of the partitions inside your NANDs (TWLN / TWLP / AGBSAVE / FIRM0 / FIRM1 / CTRNAND). Partitions with a file system (TWLN / TWLP / CTRNAND) can easily be mounted, viewed and edited on Windows via [OSFMount](http://www.osforensics.com/tools/mount-disk-images.html). These partitions are included in your NAND and can be dumped by this feature:
  * __TWLN__: _TWL-NAND FAT16 File System_ - this is the same as on a Nintendo DSi console. Installed DSiWare titles reside in this partition. This partition can be used, f.e. to set up [SudokuHax](https://gbatemp.net/threads/tutorial-new-installing-sudokuhax-on-3ds-4-x-9-2.388621/).
  * __TWLP__: _TWL-NAND PHOTO FAT12 File System_ - this is a Nintendo DSi specific partition for storing photos.
  * __AGBSAVE__: _AGB_FIRM GBA savegame_ - this contains a temporary copy of the current GBA games savegame.
  * __FIRM0__: _Firmware partition_ - this is here for development purposes only and should not be changed by users.
  * __FIRM1__: _Firmware partition backup_ - usually an exact copy of FIRM0.
  * __CTRNAND__: _CTR-NAND FAT16 File System_ - this contains basically you complete 3DS setup. Titles installed to the NAND reside here, and you can extract basically any file of interest from this partition.
* __Partition Inject...(!)__: This allows you to reencrypt & inject any of the partitions on your NAND from the respective files (`TWLN.bin` / `TWLP.bin` / `AGBSAVE.bin` / `FIRM0.bin` / `FIRM1.bin` / `CTRNAND.bin`) (see above). Only use this if you know exactly what you're doing and be careful. While there are some safety clamps in place, they won't protect you from a major messup caused by yourself.
* __File Dump...__: This allows you to directly dump & decrypt various files of interest from your SysNAND and EmuNAND. These files are included in this feature:
  * __ticket.db__: _Contains titlekeys for installed titles_ - use this with [Cearps FunkyCIA](https://gbatemp.net/threads/release-funkycia2-build-cias-from-your-eshop-content-super-easy-and-fast-2-1-fix.376941/) to download installed (legit, purchased) titles directly to your PCs ard drive.
  * __title.db__: _A database of installed titles_ - apart from informative purposes this doesn't serve a direct purpose for most users at the moment.
  * __import.db__: _A database of titles to be installed_ - this can be used to get rid of the FW update nag at a later time. Read more on it in this [GBAtemp thread](https://gbatemp.net/threads/poc-removing-update-nag-on-emunand.399460/).
  * __certs.db__: _A database of certificates_ - any practical use for this is unknown at the moment.
  * __SecureInfo_A__: _This contains your region and an ASCII serial number_ - this can be used to temporarily change your 3DS region. The dump / inject options in Decrypt9 simplify [the tutorial found here](https://gbatemp.net/threads/release-3ds-nand-secureinfo-tool-for-region-change.383792/).
  * __LocalFriendCodeSeed_B__: _This contains your FriendCodeSeed_ - in theory this can be used to import your friend list to another 3DS.
  * __rand_seed__: _Most likely containing some random data_ - any practical use for this is unknown at the moment.
  * __movable.sed__: _This contains the keyY for decryption of data on the SD card_ - Decrypt9 itself uses this in the SD Decryptor / Encryptor and in SD padgen. 
  * __seedsave.bin__: _Contains the seeds for decryption of 9.6x seed encrypted titles_ - only the seeds for installed (legit, purchased) titles are included in this. Use [SEEDconv](https://gbatemp.net/threads/download-seedconv-seeddb-bin-generator-for-use-with-decrypt9.392856/) (recommended) or the included Python script `seeddb_gen.py` to extract the seeds from this into the Decrypt9 readable `seeddb.bin`.
  * __updtsave.bin__: _Contains some data relating to system updates_ - it is possible to block automatic system updates (ie. the 'update nag') with this file. Research is still in progress. [Read this](https://gbatemp.net/threads/poc-removing-update-nag-on-emunand.399460/page-5#post-5863332) and the posts after it for more information.
  * __nnidsave.bin__: _Contains your NNID data_ - this can be used to reset / remove the NNID from your system, without removing any other data. See [here](https://gbatemp.net/threads/download-decrypt9-open-source-decryption-tools-wip.388831/page-89#post-6000951) for instructions.
* __File Inject...(!)__: This allows you to directly encrypt & inject various files of interest into the SysNAND and EmuNAND. For more information check out the list above.
* __Health&Safety Dump__: This allows you to to dump the decrypted Health and Safety system app to your SD card. The dumped H&S app can be used to [create injectable files for any homebrew software](https://gbatemp.net/threads/release-inject-any-app-into-health-safety-o3ds-n3ds-cfw-only.402236/).
* __Health&Safety Inject(!)__: This is used to inject any app into your Health & Safety system app (as long as it is smaller than the original H&S app). Multiple safety clamps are in place, and this is a pretty safe feature. Users are still adviced to be cautious using this and only use eiter the original hs.app or inject apps created with the [Universal Inject Generator](https://gbatemp.net/threads/release-inject-any-app-into-health-safety-o3ds-n3ds-cfw-only.402236/). This feature will detect all injectable apps on the SD card and let the user choose which one to inject.
* __Update SeedDB__: Use this to create or update the ´seeddb.bin´ file on your SD card with the seeds currently installed in your EmuNAND. Only new seeds will get added to `seeddb.bin` and all ones are untouched.

### Game Decryptor Options
This category includes all features that allow the decryption (and encryption) of external and internal game files. Game files are any files that contain games and other software. Typical game file extensions are .CIA, .3DS and .APP. Game files are directly processed - the encrypted versions are overwritten with the decrypted ones and vice versa, so keep backups. The standard work folder for game files is `/D9Game/`, but if that does not exist, game files are processed inside the `/Decrypt9/` work folder.
* __NCCH/NCSD Decryptor__: Use this to fully decrypt all NCCH / NCSD files in the folder. Files with .3DS and .APP extension are typically NCCH / NCSD files. A full decryption of a .3DS file is otherwise also known as _cryptofixing_. Important Note: Depending on you 3DS console type / FW version and the encryption in your NCCH/NCSD files you may need additional files `slot0x25KeyX.bin` and / or `seeddb.bin`. You're on your own for `slot0x25KeyX.bin`, but for `seeddb.bin` [this](http://tinivi.net/seeddb/) will help you.
* __NCCH/NCSD Encryptor__: Use this to (re-)encrypt all NCCH / NCSD files in the folder using standard encryption (f.e. after decrypting them). Standard encryption can be processed on any 3DS, starting from the lowest firmware versions. On some hardware, .3DS files might need to be encrypted for compatibility.
* __CIA Decryptor (shallow)__: Use this to decrypt, for all CIA files in the folder, the first layer of CIA decryption. The internal NCCH encryption is left untouched.
* __CIA Decryptor (deep)__:  Use this to fully decrypt all CIA files in the folder. This also processes the internal NCCH encryption. Deep decryption of a CIA file is otherwise known as _cryptofixing_. This also may need additional files `slot0x25KeyX.bin` and / or `seeddb.bin`, see above.
* __CIA Decryptor (CXI only)__: This is the same as CIA Decryptor (deep), but it does not process the 'deep' NCCH encryption for anything but the first CXI content. On some hardware, fully deep decrypted CIA files might not be installable, but CIA files processed with feature will work.
* __CIA Encryptor (NCCH)__: Use this to encrypt the NCCH containers inside of the CIA files in the folder. NCCH encryption is required, for example, for system CIA files to be installable.
* __CIA Encryptor (CXI only)__: This is the same as CIA Decryptor (NCCH), but it does not process anything but the first CXI content.
* __SD Decryptor/Encryptor__: Use this to decrypt or encrypt 'SD files'. SD files are titles, extdata and databases found inside the `/Nintendo 3DS/<id0>/<id1>/` folder. For this feature to work, you need to manually copy the file(s) you want to process. Copy them with their full folder structure (that's everything _after_ `/Nintendo 3DS/<id0>/<id1>/`) to the work / game folder. This feature should by now only be useful to encrypt content, decryption is much easier handled by the two features below.
* __SD Decryptor (SysNAND dir)__: An improved version of the feature above. This allows you to select content from '/Nintendo 3DS/' (more specifically from the subfolder belonging to SysNAND) to be directly copied to your work / game folder and then decrypted from there.
* __SD Decryptor (EmuNAND dir)__: This has the same functionality as the feature above, but handles the content of the '/Nintendo 3DS/' subfolder belonging to the EmuNAND instead.

### Selftest Options
This category includes two special features which allow you to test Decrypt9 internal functionality. The selftest is used, for example, to assure that everything works as intended on a new entrypoint.
* __Create Selftest Reference__: Run this first on a known working entrypoint to generate the selftest reference data. This will create a file called `d9_selftest.ref` inside your SD card root or work folder.
* __Run Selftest__: Run the actual selftest (must have created the reference data before). This will create or update a file called `d9_selftest.lst` on your SD card root or work folder. Note: on O3DS failed `ncch_sec3_key`, `ncch_sec4_key` and `nand_ctrn_key` tests are normal and expected. On O3DS <= FW 7.0, `ncch_7x_key` may fail. On N3DS, failed `ncch_sec4_key` may fail.

## Credits by Archshift
* Roxas75 for the method of ARM9 code injection
* Cha(N), Kane49, and all other FatFS contributors for FatFS
* Normmatt for `sdmmc.c` as well as project infrastructure (Makefile, linker setup, etc)
* Relys, sbJFn5r for the decryptor

## Credits by d0k3
* Everyone mentioned by Archshift above
* Archshift for starting this project and being a great project maintainer
* patois, delebile, SteveIce10 for Brahma and it's updates
* mid-kid for CakeHax
* Shadowtrance, dark_samus3, Syphurith for being of great help developing this
* profi200 for helpful hints that first made developing some features possible
* Datalogger, zoogie, atkfromabove, mixups, key1340, k8099 and countless others from the GBAtemp forums for testing, feedback and helpful hints
* Everyone I forgot about - if you think you deserve to be mentioned, just contact me
