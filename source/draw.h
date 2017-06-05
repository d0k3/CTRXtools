// Copyright 2013 Normmatt
// Licensed under GPLv2 or any later version
// Refer to the license.txt file included.

#pragma once

#include "common.h"

#define BYTES_PER_PIXEL 3
#define SCREEN_HEIGHT 240
#define SCREEN_WIDTH_TOP 400
#define SCREEN_WIDTH_BOT 320
#ifdef FONT_6X10 // special font width
#define FONT_WIDTH_EXT 6
#elif defined FONT_GB // special font width
#define FONT_WIDTH_EXT 7
#else
#define FONT_WIDTH_EXT 8
#endif

#define RGB(r,g,b) (b<<16|g<<8|r)

#define COLOR_BLACK         RGB(0x00, 0x00, 0x00)
#define COLOR_WHITE         RGB(0xFF, 0xFF, 0xFF)
#define COLOR_RED           RGB(0xFF, 0x00, 0x00)
#define COLOR_GREEN         RGB(0x00, 0xFF, 0x00)
#define COLOR_BLUE          RGB(0x00, 0x00, 0xFF)
#define COLOR_CYAN          RGB(0x00, 0xFF, 0xFF)
#define COLOR_MAGENTA       RGB(0xFF, 0x00, 0xFF)
#define COLOR_YELLOW        RGB(0xFF, 0xFF, 0x00)
#define COLOR_GREY          RGB(0x77, 0x77, 0x77)
#define COLOR_TRANSPARENT   RGB(0xFF, 0x00, 0xEF) // otherwise known as 'super fuchsia'

#define COLOR_GREYBLUE      RGB(0xA0, 0xA0, 0xFF)
#define COLOR_GREYGREEN     RGB(0xA0, 0xFF, 0xA0)
#define COLOR_GREYRED       RGB(0xFF, 0xA0, 0xA0)
#define COLOR_GREYCYAN      RGB(0xA0, 0xFF, 0xFF)
#define COLOR_TINTEDRED     RGB(0xFF, 0x60, 0x60)
#define COLOR_LIGHTGREY     RGB(0xA0, 0xA0, 0xA0)

#define COLOR_ASK           COLOR_GREYGREEN
#define COLOR_SELECT        COLOR_LIGHTGREY
#define COLOR_ACCENT        COLOR_GREEN

#ifndef USE_THEME
#define STD_COLOR_BG   COLOR_BLACK
#define STD_COLOR_FONT COLOR_WHITE

#define DBG_COLOR_BG   COLOR_BLACK
#define DBG_COLOR_FONT COLOR_WHITE

#define DBG_START_Y 10
#define DBG_END_Y   (SCREEN_HEIGHT - 10)
#define DBG_START_X 10
#define DBG_END_X   (SCREEN_WIDTH_TOP - 10)
#define DBG_STEP_Y  10
#endif

#define DBG_N_CHARS_Y ((DBG_END_Y - DBG_START_Y) / DBG_STEP_Y)
#define DBG_N_CHARS_X (((DBG_END_X - DBG_START_X) / FONT_WIDTH) + 1)

#define TOP_SCREEN top_screen
#define BOT_SCREEN bottom_screen

#define ScreenWidth(x)       (((x) == (TOP_SCREEN) ? 400 : 320))
#define IsCharPartOfWord(x)  (((x) >= 'a' && (x) <= 'z') || ((x) >= '0' && (x) <= '9') || ((x) >= 'A' && (x) <= 'Z'))

extern u8 *top_screen, *bottom_screen;

void ClearScreen(unsigned char *screen, int width, int color);
void ClearScreenFull(bool clear_top, bool clear_bottom);

void DrawCharacter(unsigned char *screen, int character, int x, int y, int color, int bgcolor);
int  DrawString(unsigned char *screen, const char *str, int x, int y, int color, int bgcolor);
void DrawStringF(int x, int y, bool use_top, const char *format, ...);
void DrawStringFC(int x, int y, bool use_top, u32 color, const char *format, ...);

void Screenshot(const char* path);
void DebugClear();
void DebugSet(const char **strs);
void DebugColor(u32 color, const char *format, ...);
void Debug(const char *format, ...);

void ShowProgress(u64 current, u64 total);
