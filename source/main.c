#include <string.h>
#include <stdlib.h>
#include <stdio.h>

#include "common.h"
#include "draw.h"
#include "fs.h"
#include "hid.h"
#include "i2c.h"
#include "decryptor/padgen.h"
#include "decryptor/titlekey.h"

void ClearTop()
{
    ClearScreen(TOP_SCREEN0, RGB(0, 0, 0));
    ClearScreen(TOP_SCREEN1, RGB(0, 0, 0));
    current_y = 0;
}

void Reboot()
{
    i2cWriteRegister(I2C_DEV_MCU, 0x20, 1 << 2);
    while(true);
}

void newline(int count) {
	int i;
	for(i=0; i<count; i++)
		Debug("");
}
	
void drawUI()
{	
	drawRect(2, 2, 398, 238, 102, 0, 255, TOP_SCREEN0);
	drawRect(2, 2, 398, 238, 102, 0, 255, TOP_SCREEN1);
	drawRect(6, 6, 394, 20, 102, 0, 255, TOP_SCREEN0);
	drawRect(6, 6, 394, 20, 102, 0, 255, TOP_SCREEN1);
	DrawString(TOP_SCREEN0, "Decrypt9 - Bootstrap-MOD", 110, 10, 65280, 0);
	DrawString(TOP_SCREEN1, "Decrypt9 - Bootstrap-MOD", 110, 10, 65280, 0);
}

void menu()
{
	DrawString(TOP_SCREEN0, "A: NCCH Padgen", 110, 60, 65280, 0);
	DrawString(TOP_SCREEN1, "A: NCCH Padgen", 110, 60, 65280, 0);
	DrawString(TOP_SCREEN0, "B: SD Padgen", 110, 70, 65280, 0);
	DrawString(TOP_SCREEN1, "B: SD Padgen", 110, 70, 65280, 0);
	DrawString(TOP_SCREEN0, "X: Titlekey Decryption", 110, 80, 65280, 0);
	DrawString(TOP_SCREEN1, "X: Titlekey Decryption", 110, 80, 65280, 0);
	DrawString(TOP_SCREEN0, "Y: NAND Padgen", 110, 90, 65280, 0);
	DrawString(TOP_SCREEN1, "Y: NAND Padgen", 110, 90, 65280, 0);
	drawRect(6, 50, 394, 110, 102, 0, 255, TOP_SCREEN0);
	drawRect(6, 50, 394, 110, 102, 0, 255, TOP_SCREEN1);
}

void space()
{
	drawRect(6, 234, 394, 214, 102, 0, 255, TOP_SCREEN0);
	drawRect(6, 234, 394, 214, 102, 0, 255, TOP_SCREEN1);
	newline(22);
	Debug("     Remaining SD storage space: %llu MiB", RemainingStorageSpace() / 1024 / 1024);
}

int main()
{
    ClearTop();
    InitFS();
	drawUI();
	menu();
	space();
	
    while (true) {
        u32 pad_state = InputWait();
        if (pad_state & BUTTON_A) {
            ClearTop();
			drawUI();
            Debug("NCCH Padgen: %s!", NcchPadgen() == 0 ? "succeeded" : "failed");
            break;
        } else if (pad_state & BUTTON_B) {
            ClearTop();
			drawUI();
            Debug("SD Padgen: %s!", SdPadgen() == 0 ? "succeeded" : "failed");
            break;
        } else if (pad_state & BUTTON_X) {
            ClearTop();
			drawUI();
            Debug("Titlekey Decryption: %s!", DecryptTitlekeys() == 0 ? "succeeded" : "failed");
            break;
        } else if (pad_state & BUTTON_Y) {
            ClearTop();
			drawUI();
            Debug("NAND Padgen: %s!", NandPadgen() == 0 ? "succeeded" : "failed");
            break;
        } else if (pad_state & BUTTON_START) {
            goto reboot;
        }
    }

    Debug("");
    Debug("Press START to reboot to home.");
    while(true) {
        if (InputWait() & BUTTON_START)
            break;
    }

reboot:
    DeinitFS();
    Reboot();
    return 0;
}
