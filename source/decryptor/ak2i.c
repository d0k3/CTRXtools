#include "fs.h"
#include "draw.h"
#include "hid.h"
#include "platform.h"
#include "gamecart/protocol.h"
#include "gamecart/command_ctr.h"
#include "gamecart/command_ntr.h"
#include "gamecart/command_ak2i.h"
#include "decryptor/aes.h"
#include "decryptor/sha.h"
#include "decryptor/decryptor.h"
#include "decryptor/hashfile.h"
#include "decryptor/keys.h"
#include "decryptor/titlekey.h"
#include "decryptor/nandfat.h"
#include "decryptor/nand.h"
#include "decryptor/game.h"
#include "decryptor/ak2i.h"

#define AK2I_81_BOOTROM_LENGTH 0x1000000
#define AK2I_44_BOOTROM_LENGTH 0x200000
#define AK2I_PATCH_LENGTH 0xA0000

u32 DumpAk2iCart(u32 param)
{
    (void) param; // unused here

    // check if cartridge inserted
    if (REG_CARDCONF2 & 0x1) {
        Debug("Cartridge was not detected");
        return 1;
    }

    Cart_Init();

    u32 cartId = Cart_GetID();
    if (cartId != 0xFC2) {
        Debug("Cartridge is not AK2i");
        return 1;
    }

    u32 hwVer = AK2I_CmdGetHardwareVersion();
    if (hwVer != 0x81 && hwVer != 0x44 ) {
        Debug("Cartridge is not AK2i");
        return 1;
    }

    char filename[64];
    if (OutputFileNameSelector(filename, "ak2i_flash.bin", NULL) != 0)
        return 1;

    if (!DebugFileCreate(filename, true))
        return 1;

    Debug("Cart: 0x%04X HW: %x", cartId, hwVer);

    u32 length = hwVer == 0x81 ? AK2I_81_BOOTROM_LENGTH : AK2I_44_BOOTROM_LENGTH;

    u8* mem = BUFFER_ADDRESS;

    //Debug("Reading rom... (length 0x4000)");
    AK2I_CmdReadRom(0, mem, 0x4000);

    //Debug("Dump rom...");
    //if (FileDumpData("ak2i_rom_0x4000.bin", mem, 0x4000) != 0x4000) {
    //    Debug("Failed writing ak2i_rom_0x4000.bin");
    //}

    AK2I_CmdSetMapTableAddress(AK2I_MTN_NOR_OFFSET, 0);

    // setup flash to 1681
    if (hwVer == 0x81) {
        Debug("Setup flash to 1681");
        AK2I_CmdSetFlash1681_81();
    }

    Debug("Active FAT MAP");
    // have to do this on ak2i before making fatmap, or the first 128k flash data will be screwed up.
    AK2I_CmdActiveFatMap();

    //Debug("Unlock flash");
    //// this funcion write enable only above 0x40000 on ak2i, write enable all on ak2
    //AK2I_CmdUnlockFlash();

    //Debug("Unlock ASIC");
    //// unlock 0x30000 for save map, see definition of NOR_FAT2_START above
    //AK2I_CmdUnlockASIC();

    Debug("Dump flash... (length 0x%08X)", length);

    u8 *tmp = mem;
    for( u32 i = 0, offset = 0, read = 0; i < length; i += 512 ) {
        ShowProgress(i, length);
        u32 toRead = 512;
        if( toRead > length - i )
            toRead = length - i;
        memset(tmp, 0, toRead);
        AK2I_CmdReadFlash(i, tmp, toRead);
        read += toRead;
        tmp += 512;
        if (toRead < 512 || (read % 0x8000) == 0) {
            DebugFileWrite(mem, read, offset);
            offset += read;
            read = 0;
            tmp = mem;
        }
    }
    ShowProgress(0, 0);

    FileClose();

    Debug("Done");

    return 0;
}

u32 FlashAk2iCart(u32 param)
{
    (void) param; // unused here
    
    // check if cartridge inserted
    if (REG_CARDCONF2 & 0x1) {
        Debug("Cartridge was not detected");
        return 1;
    }

    Cart_Init();

    u32 cartId = Cart_GetID();
    if (cartId != 0xFC2) {
        Debug("Cartridge is not AK2i");
        return 1;
    }

    u32 hwVer = AK2I_CmdGetHardwareVersion();
    if (hwVer != 0x81 && hwVer != 0x44 ) {
        Debug("Cartridge is not AK2i");
        return 1;
    }

    Debug("Cart: 0x%04X HW: %x", cartId, hwVer);

    char filename[64];
    if (InputFileNameSelector(filename, "ak2i_flash.bin", NULL, NULL, 0, AK2I_PATCH_LENGTH, true) != 0)
        return 1;

    if (!DebugFileOpen(filename))
        return 1;

    u8* buffer = BUFFER_ADDRESS;
    memset(buffer, 0x41, AK2I_PATCH_LENGTH);

    Debug("Load data");

    if (!DebugFileRead(buffer, AK2I_PATCH_LENGTH, 0)) {
        FileClose();
        return 1;
    };
    FileClose();

    //DebugFileCreate("ak2i_ntrcardhax.bin", true);
    //DebugFileWrite(buffer, AK2I_PATCH_LENGTH, 0);
    //FileClose();
    //return 1;

    AK2I_CmdSetMapTableAddress(AK2I_MTN_NOR_OFFSET, 0);

    // setup flash to 1681
    if (hwVer == 0x81) {
        Debug("Setup flash to 1681");
        AK2I_CmdSetFlash1681_81();
    }

    Debug("Active FAT MAP");
    // have to do this on ak2i before making fatmap, or the first 128k flash data will be screwed up.
    AK2I_CmdActiveFatMap();

    Debug("Unlock flash");
    // this funcion write enable only above 0x40000 on ak2i, write enable all on ak2
    AK2I_CmdUnlockFlash();

    Debug("Unlock ASIC");
    // unlock 0x30000 for save map, see definition of NOR_FAT2_START above
    AK2I_CmdUnlockASIC();

    Debug("Erase flash");
    for (u32 i = 0; i < AK2I_PATCH_LENGTH; i += 64 * 1024) {
        if (hwVer == 0x81) {
            AK2I_CmdEraseFlashBlock_81(i);
        } else {
            AK2I_CmdEraseFlashBlock_44(i);
        }
    }
    //ioAK2EraseFlash( 0, CHIP_ERASE );

    //buffer[0x200C] = 'B';

    Debug("Writing...");

    for( u32 i = 0; i < AK2I_PATCH_LENGTH; i += 512 ) {
        ShowProgress(i, AK2I_PATCH_LENGTH);
        if (hwVer == 0x81) {
            AK2I_CmdWriteFlash_81(i, buffer + i, 512);
        } else {
            AK2I_CmdWriteFlash_44(i, buffer + i, 512);
        }

        if (!AK2I_CmdVerifyFlash(buffer + i, i, 512)) {
            Debug("verify failed at %08X", i);
        }
    }
    ShowProgress(0, 0);

    Debug("Lock flash");
    AK2i_CmdLockFlash();

//    AK2I_CmdSetMapTableAddress(AK2I_MTN_NOR_OFFSET, 0);
//
//    //u32 acek_sign = 0x4B454341; // "ACEK"
//    //u32 ak2k_sign = 0x4B324B41; // "AK2K"
//
//    //ioAK2WriteFlash( NOR_FAT2_START, &ak2k_sign, 4 );
//
//    //end writting
//
//    u8* mem = BUFFER_ADDRESS + 0x10000;
//    memset(mem, 0x41, 0x10000);
//
//    u32 length = hwVer == 0x81 ? AK2I_81_BOOTROM_LENGTH : AK2I_44_BOOTROM_LENGTH;
//    Debug("Dump patched flash... (length 0x%08X)", length);
//
//    DebugFileCreate("ak2i_flash_patched.bin", true);
//    u8 *tmp = mem;
//    for( u32 i = 0, offset = 0, read = 0; i < length; i += 512 ) {
//        ShowProgress(i, length);
//        u32 toRead = 512;
//        if( toRead > length - i )
//            toRead = length - i;
//        memset(tmp, 0, toRead);
//        AK2I_CmdReadFlash(i, tmp, toRead);
//        read += toRead;
//        tmp += 512;
//        if (toRead < 512 || (read % 0x8000) == 0) {
//            DebugFileWrite(mem, read, offset);
//            offset += read;
//            read = 0;
//            tmp = mem;
//        }
//    }
//
//    FileClose();
//    ShowProgress(0, 0);
//
    Debug("Done");

    return 0;
}
