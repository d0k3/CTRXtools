#include "titlekey.h"
#include "fs.h"
#include "draw.h"
#include "crypto.h"

// From https://github.com/profi200/Project_CTR/blob/master/makerom/pki/prod.h#L19
static const u8 common_keyy[6][16] = {
    {0xD0, 0x7B, 0x33, 0x7F, 0x9C, 0xA4, 0x38, 0x59, 0x32, 0xA2, 0xE2, 0x57, 0x23, 0x23, 0x2E, 0xB9} , // 0 - eShop Titles
    {0x0C, 0x76, 0x72, 0x30, 0xF0, 0x99, 0x8F, 0x1C, 0x46, 0x82, 0x82, 0x02, 0xFA, 0xAC, 0xBE, 0x4C} , // 1 - System Titles
    {0xC4, 0x75, 0xCB, 0x3A, 0xB8, 0xC7, 0x88, 0xBB, 0x57, 0x5E, 0x12, 0xA1, 0x09, 0x07, 0xB8, 0xA4} , // 2
    {0xE4, 0x86, 0xEE, 0xE3, 0xD0, 0xC0, 0x9C, 0x90, 0x2F, 0x66, 0x86, 0xD4, 0xC0, 0x6F, 0x64, 0x9F} , // 3
    {0xED, 0x31, 0xBA, 0x9C, 0x04, 0xB0, 0x67, 0x50, 0x6C, 0x44, 0x97, 0xA3, 0x5B, 0x78, 0x04, 0xFC} , // 4
    {0x5E, 0x66, 0x99, 0x8A, 0xB4, 0xE8, 0x93, 0x16, 0x06, 0x85, 0x0F, 0xD7, 0xA1, 0x6D, 0xD7, 0x55} , // 5
};

u32 DecryptTitlekeys(void)
{
	EncKeysInfo *info = (EncKeysInfo*)0x20316000;
	newline(3);
	Debug("Opening encTitleKeys.bin ...");
	if(!FileOpen("/encTitleKeys.bin"))
	{
		Debug("Could not open encTitleKeys.bin!");
		return 1;
	}
	FileRead(info, 16, 0);
	
	if (!info->n_entries || info->n_entries > MAXENTRIES) {
		Debug("Too many/few entries specified: %i", info->n_entries);
		FileClose();
		return 1;
	}
	
	Debug("Number of entries: %i", info->n_entries);
	
	FileRead(info->entries, info->n_entries * sizeof(TitleKeyEntry), 16);
	FileClose();
	
	Debug("Decrypting Title Keys...");
	
	u8 ctr[16] __attribute__((aligned(32)));
	u8 keyY[16] __attribute__((aligned(32)));
	u32 i;
	for(i = 0; i < info->n_entries; i++) {
		memset(ctr, 0, 16);
		memcpy(ctr, info->entries[i].titleId, 8);
		set_ctr(AES_BIG_INPUT|AES_NORMAL_INPUT, ctr);
		memcpy(keyY, (void *)common_keyy[info->entries[i].commonKeyIndex], 16);
		setup_aeskey(0x3D, AES_BIG_INPUT|AES_NORMAL_INPUT, keyY);
		use_aeskey(0x3D);
		aes_decrypt(info->entries[i].encryptedTitleKey, info->entries[i].encryptedTitleKey, ctr, 1, AES_CBC_DECRYPT_MODE);
	}

	if(!FileCreate("/decTitleKeys.bin", true))
		return 1;

    FileWrite(info, info->n_entries * sizeof(TitleKeyEntry) + 16, 0);
	FileClose();
	
	Debug("Done!");
	
	return 0;
}
