#include <3ds.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <malloc.h>
#include "brahma.h"
#include "hid.h"
#include "menus.h"
#include "sochlp.h"

void interact_with_user (void) {
	s32 menuidx = 0;
	
	while (aptMainLoop()) {
		gspWaitForVBlank();

		menuidx = print_main_menu(menuidx, &g_main_menu);		

		u32 kDown = wait_key(); 
		
		if (kDown & KEY_B) {
			break;
		}
		else if (kDown & KEY_A) {
			consoleClear();
			printf("\n");
			if (menu_execute_function(menuidx, &g_main_menu, 0))
				wait_any_key();
		}
		else if (kDown & KEY_UP) {
			menuidx--;
		}
		else if (kDown & KEY_DOWN) {
			menuidx++;
		}
		gfxFlushBuffers();
		gfxSwapBuffers(); 
	}

	return;
}

s32 quick_boot_firm (s32 load_from_disk) {
	if (load_from_disk)
		load_arm9_payload("");
	firm_reboot();	
}

s32 main (void) {
	// Initialize services
	srvInit();
	aptInit();
	hidInit(NULL);
	gfxInitDefault();
	fsInit();
	sdmcInit();
	hbInit();
	qtmInit();
	
	consoleInit(GFX_BOTTOM, NULL);
	if (brahma_init()) {
		hidScanInput();
		u32 kHeld = hidKeysHeld();
	 
		if (1) {                                                       //CHANGE 1 is always TRUE - lets just directly load arm9 payload and not mess with other stuff
			/* load default payload from dosk and run exploit */       //CHANGE no patois, it loads from RAM now, not 'dosk'
			quick_boot_firm(1);
			printf("[!] Quickload failed\n");
			wait_any_key();
		} else if (kHeld & KEY_RIGHT) {
			/* reboot only */
			quick_boot_firm(0);
		}
	
		soc_init();	
		interact_with_user();
		soc_exit();
		brahma_exit();

	} else {
		printf("* BRAHMA *\n\n[!]Not enough memory\n");
		wait_any_key();
	}

	hbExit();
	sdmcExit();
	fsExit();
	gfxExit();
	hidExit();
	aptExit();
	srvExit();
	// Return to hbmenu
	return 0;
}
