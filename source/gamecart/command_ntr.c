// Copyright 2014 Normmatt
// Licensed under GPLv2 or any later version
// Refer to the license.txt file included.
//
// modifyed by osilloscopion (2 Jul 2016)
//

#include "command_ntr.h"
#include "protocol_ntr.h"
#include "card_ntr.h"
#include "delay.h"


u32 ReadDataFlags = 0;

void NTR_CmdReset(void)
{
    cardReset ();
    ioDelay(0xF000);
}

u32 NTR_CmdGetCartId(void)
{
    return cardReadID (0);
}

void NTR_CmdEnter16ByteMode(void)
{
    static const u32 enter16bytemode_cmd[2] = { 0x3E000000, 0x00000000 };
    NTR_SendCommand(enter16bytemode_cmd, 0x0, 0, NULL);
}

void NTR_CmdReadHeader (void* buffer)
{
    cardReadHeader (buffer);
}

void NTR_CmdReadData (u32 offset, void* buffer)
{
    cardParamCommand (NTRCARD_CMD_DATA_READ, offset, ReadDataFlags | NTRCARD_ACTIVATE | NTRCARD_nRESET | NTRCARD_BLK_SIZE(1), (u32*)buffer, 0x200 / 4);
}


