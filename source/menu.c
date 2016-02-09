#include "menu.h"
#include "draw.h"
#include "hid.h"
#include "fs.h"
#ifdef USE_THEME
#include "theme.h"
#endif
#include "decryptor/nand.h"


#ifdef LOG_FILE
u32 ScrollOutput()
{
    u32 log_start = LogWrite(NULL);
    
    // careful, these areas are used by other functions in Decrypt9
    char** logptr = (char**) 0x20316000;
    char* logtext = (char*)  0x20400000;
    u32 log_size = 0; // log size
    u32 l_total = 0; // total lines
    u32 l_curr = 0; // current line
    
    if (!FileOpen(LOG_FILE))
        return 0;
    log_size = FileGetSize();
    if ((log_size <= log_start) || (log_size - log_start >= 1024 * 1024)) {
        FileClose();
        return 0; // allow 1MB of text max
    }
    log_size -= log_start;
    if (!FileRead(logtext, log_size, log_start)) {
        FileClose();
        return 0;
    }
    FileClose();
    
    // read lines
    logtext[log_size - 1] = '\0';
    for (char* line = logtext; line != NULL && l_total < 4000; line = strchr(line, '\n')) {
        *line = '\0';
        logptr[l_total++] = ++line; 
    }
    if (l_total >= 4000) // allow 4000 lines of text max
        return 0;
    for (; l_total < DBG_N_CHARS_Y; logptr[l_total++] = logtext + log_size - 1);
    
    // here, the actual output starts
    l_curr = l_total - DBG_N_CHARS_Y ;
    if (l_curr > 0) l_curr--; // start at the line before the last
    while (true) {
        DebugSet((const char**) logptr + l_curr);
        u32 pad_state = InputWait();
        if (pad_state & BUTTON_X) {
            Screenshot(NULL);
        } else if ((pad_state & BUTTON_UP) && (l_curr > 0)) {
            l_curr--;
        } else if ((pad_state & BUTTON_DOWN) && (l_curr < l_total - DBG_N_CHARS_Y)) {
            l_curr++;
        } else if (pad_state & (BUTTON_B | BUTTON_START)) {
            return pad_state;
        }
    }
    
    return 0;
}
#endif

u32 UnmountSd()
{
    u32 pad_state;
    
    DebugClear();
    Debug("Unmounting SD card...");
    #ifdef USE_THEME
    LoadThemeGfx(GFX_UNMOUNT, false);
    #endif
    DeinitFS();
    Debug("SD is unmounted, you may remove it now.");
    Debug("Put the SD card back in before pressing B!");
    Debug("");
    Debug("(B to return, START to reboot)");
    while (true) {
        pad_state = InputWait();
        if (((pad_state & BUTTON_B) && InitFS()) || (pad_state & BUTTON_START))
            break;
    }
    
    return pad_state;
}

void DrawMenu(MenuInfo* currMenu, u32 index, bool fullDraw, bool subMenu)
{
    u32 emunand_state = CheckEmuNand();
    bool top_screen = true;
    u32 menublock_x0 = (top_screen) ? 76 : 36;
    u32 menublock_x1 = (top_screen) ? 52 : 12;
    u32 menublock_y0 = 40;
    u32 menublock_y1 = menublock_y0 + currMenu->n_entries * 10;
    
    if (fullDraw) { // draw full menu
        ClearScreenFull(true, !top_screen);
        DrawStringF(menublock_x0, menublock_y0 - 20, top_screen, "%s", currMenu->name);
        DrawStringF(menublock_x0, menublock_y0 - 10, top_screen, "==============================");
        DrawStringF(menublock_x0, menublock_y1 +  0, top_screen, "==============================");
        DrawStringF(menublock_x0, menublock_y1 + 10, top_screen, (subMenu) ? "A: Choose  B: Return" : "A: Choose");
        DrawStringF(menublock_x0, menublock_y1 + 20, top_screen, "SELECT: Unmount SD");
        DrawStringF(menublock_x0, menublock_y1 + 30, top_screen, "START:  Reboot");
        DrawStringF(menublock_x1, SCREEN_HEIGHT - 20, top_screen, "SD card: %lluMB/%lluMB & %s", RemainingStorageSpace() / 1024 / 1024, TotalStorageSpace() / 1024 / 1024, (emunand_state == EMUNAND_READY) ? "EmuNAND ready" : (emunand_state == EMUNAND_GATEWAY) ? "GW EmuNAND" : (emunand_state == EMUNAND_REDNAND) ? "RedNAND" : "no EmuNAND");
        DrawStringF(menublock_x1, SCREEN_HEIGHT - 30, top_screen, "Game directory: %s", GAME_DIR);
        if (DirOpen(WORK_DIR)) {
            DrawStringF(menublock_x1, SCREEN_HEIGHT - 40, top_screen, "Work directory: %s", WORK_DIR);
            DirClose();
        }
    }
    
    if (!top_screen)
        DrawStringF(10, 10, true, "Selected: %-*.*s", 32, 32, currMenu->entries[index].name);
        
    for (u32 i = 0; i < currMenu->n_entries; i++) { // draw menu entries / selection []
        char* name = currMenu->entries[i].name;
        DrawStringF(menublock_x0, menublock_y0 + (i*10), top_screen, (i == index) ? "[%s]" : " %s ", name);
    }
}

u32 ProcessEntry(MenuEntry* entry)
{
    bool emunand    = entry->param & N_EMUNAND; 
    bool nand_force = entry->param & N_FORCENAND;
    bool warning    = entry->param & N_NANDWRITE;
    
    u32 pad_state;
    u32 res = 0;
 
    // unlock sequence for dangerous features
    if (warning) {
        u32 unlockSequenceEmu[] = { BUTTON_LEFT, BUTTON_RIGHT, BUTTON_DOWN, BUTTON_UP, BUTTON_A };
        u32 unlockSequenceSys[] = { BUTTON_LEFT, BUTTON_UP, BUTTON_RIGHT, BUTTON_UP, BUTTON_A };
        u32 unlockLvlMax = ((emunand) ? sizeof(unlockSequenceEmu) : sizeof(unlockSequenceSys)) / sizeof(u32);
        u32* unlockSequence = (emunand) ? unlockSequenceEmu : unlockSequenceSys;
        u32 unlockLvl = 0;
        #ifdef USE_THEME
        LoadThemeGfx((emunand) ? GFX_DANGER_E : GFX_DANGER_S, false);
        #endif
        DebugClear();
        Debug("You selected \"%s\".", entry->name);
        Debug("This feature writes to the %s.", (emunand) ? "EmuNAND" : "SysNAND");
        Debug("Doing this is potentially dangerous!");
        Debug("");
        Debug("If you wish to proceed, enter:");
        Debug((emunand) ? "<Left>, <Right>, <Down>, <Up>, <A>" : "<Left>, <Up>, <Right>, <Up>, <A>");
        Debug("");
        Debug("(B to return, START to reboot)");
        while (true) {
            ShowProgress(unlockLvl, unlockLvlMax);
            if (unlockLvl == unlockLvlMax)
                break;
            pad_state = InputWait();
            if (!(pad_state & BUTTON_ANY))
                continue;
            else if (pad_state & unlockSequence[unlockLvl])
                unlockLvl++;
            else if (pad_state & (BUTTON_B | BUTTON_START))
                break;
            else if (unlockLvl == 0 || !(pad_state & unlockSequence[unlockLvl-1]))
                unlockLvl = 0;
        }
        ShowProgress(0, 0);
        if (unlockLvl < unlockLvlMax)
            return pad_state;
    }

    if (emunand) {
        pad_state = EmuNandSelector();
        if (pad_state & (BUTTON_B | BUTTON_START)){
            return pad_state;
        }
    }
    
    // execute this entries function
    #ifdef USE_THEME
    LoadThemeGfx(GFX_PROGRESS, false);
    #endif
    DebugClear();
    res = (SetNand(emunand, nand_force) == 0) ? (*(entry->function))(entry->param) : 1;
    Debug("%s: %s!", entry->name, (res == 0) ? "succeeded" : "failed");
    Debug("");
    Debug("Press B to return, START to reboot.");
    #ifdef USE_THEME
    LoadThemeGfx((res == 0) ? GFX_DONE : GFX_FAILED, false);
    #endif
    while(!((pad_state = InputWait()) & (BUTTON_B | BUTTON_START))) {
        if (pad_state & BUTTON_X) Screenshot(NULL);
        #ifdef LOG_FILE
        else if (pad_state & BUTTON_UP) {
            pad_state = ScrollOutput();
            break;
        }
        #endif
    }
    
    // returns the last known pad_state
    return pad_state;
}

void BatchScreenshot(MenuInfo* info, bool full_batch)
{
    for (u32 idx_m = 0; info[idx_m].name != NULL; idx_m++) {
        for (u32 idx_s = 0; idx_s < ((full_batch) ? info[idx_m].n_entries : 1); idx_s++) {
            char filename[16];
            snprintf(filename, 16, "menu%04lu.bmp", (idx_m * 100) + idx_s);
            #ifndef USE_THEME
            DrawMenu(info + idx_m, idx_s, true, true);
            #else
            LoadThemeGfxLogo();
            LoadThemeGfxMenu((idx_m * 100) + idx_s);
            #endif
            Screenshot(filename);
        }
    }
}

u32 ProcessMenu(MenuInfo* info, u32 n_entries_main)
{
    MenuInfo mainMenu;
    MenuInfo* currMenu;
    MenuInfo* prevMenu[MENU_MAX_DEPTH];
    u32 prevIndex[MENU_MAX_DEPTH];
    u32 menuLvlMin;
    u32 menuLvl;
    u32 index = 0;
    u32 result = MENU_EXIT_REBOOT;
    
    #ifndef USE_THEME
    if (n_entries_main > 1) {
        // build main menu structure from submenus
        if (n_entries_main > MENU_MAX_ENTRIES) // limit number of entries
            n_entries_main = MENU_MAX_ENTRIES;
        memset(&mainMenu, 0x00, sizeof(MenuInfo));
        for (u32 i = 0; i < n_entries_main; i++) {
            mainMenu.entries[i].name = info[i].name;
            mainMenu.entries[i].function = NULL;
            mainMenu.entries[i].param = i;
        }
        mainMenu.n_entries = n_entries_main;
        #ifndef BUILD_NAME
        mainMenu.name = "Decrypt9 Main Menu";
        #else
        mainMenu.name = BUILD_NAME;
        #endif
        currMenu = &mainMenu;
        menuLvlMin = 0;
    } else {
        currMenu = info;
        menuLvlMin = 1;
    }
    DrawMenu(currMenu, 0, true, false);
    #else
    currMenu = info;
    menuLvlMin = 1;
    LoadThemeGfxLogo();
    LoadThemeGfxMenu(0);
    #endif
    menuLvl = menuLvlMin;
    
    // main processing loop
    while (true) {
        bool full_draw = true;
        u32 pad_state = InputWait();
        if ((pad_state & BUTTON_A) && (currMenu->entries[index].function == NULL)) {
            if (menuLvl < MENU_MAX_DEPTH) {
                prevMenu[menuLvl] = currMenu;
                prevIndex[menuLvl] = index;
                menuLvl++;
            }
            currMenu = info + currMenu->entries[index].param;
            index = 0;
        } else if (pad_state & BUTTON_A) {
            pad_state = ProcessEntry(currMenu->entries + index);
        } else if ((pad_state & BUTTON_B) && (menuLvl > menuLvlMin)) {
            menuLvl--;
            currMenu = prevMenu[menuLvl];
            index = prevIndex[menuLvl];
        } else if (pad_state & BUTTON_DOWN) {
            index = (index == currMenu->n_entries - 1) ? 0 : index + 1;
            full_draw = false;
        } else if (pad_state & BUTTON_UP) {
            index = (index == 0) ? currMenu->n_entries - 1 : index - 1;
            full_draw = false;
        } else if ((pad_state & BUTTON_R1) && (menuLvl == 1)) {
            if (++currMenu - info >= n_entries_main) currMenu = info;
            index = 0;
        } else if ((pad_state & BUTTON_L1) && (menuLvl == 1)) {
            if (--currMenu < info) currMenu = info + n_entries_main - 1;
            index = 0;
        } else if (pad_state & BUTTON_SELECT) {
            pad_state = UnmountSd();
        } else if (pad_state & BUTTON_X) {
            (pad_state & (BUTTON_LEFT | BUTTON_RIGHT)) ?
                BatchScreenshot(info, pad_state & BUTTON_RIGHT) : Screenshot(NULL);
        } else {
            full_draw = false;
        }
        if (pad_state & BUTTON_START) {
            result = (pad_state & BUTTON_LEFT) ? MENU_EXIT_POWEROFF : MENU_EXIT_REBOOT;
            break;
        }
        #ifndef USE_THEME
        DrawMenu(currMenu, index, full_draw, menuLvl > menuLvlMin);
        #else
        if (full_draw) LoadThemeGfxLogo();
        LoadThemeGfxMenu(((currMenu - info) * 100) + index);
        #endif
    }
    
    return result;
}
