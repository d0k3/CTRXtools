// Copyright 2013 Normmatt
// Licensed under GPLv2 or any later version
// Refer to the license.txt file included.

#pragma once

#include "common.h"

#define BYTES_PER_PIXEL 3
#define SCREEN_WIDTH 240
#define SCREEN_HEIGHT 400

#define SCREEN_SIZE (BYTES_PER_PIXEL * SCREEN_WIDTH * SCREEN_HEIGHT)

#define RGB(r,g,b) (r<<24|b<<16|g<<8|r)

#define TOP_SCREEN0 (u8*)(0x20000000)
#define TOP_SCREEN1 (u8*)(0x20046500)
#define BOT_SCREEN0 (u8*)(0x2008CA00)
#define BOT_SCREEN1 (u8*)(0x200C4E00)
extern int current_y;

void ClearScreen(unsigned char *screen, int color);
void DrawCharacter(unsigned char *screen, int character, int x, int y, int color, int bgcolor);

void DrawString(unsigned char *screen, const char *str, int x, int y, int color, int bgcolor);
void DrawStringF(int x, int y, const char *format, ...);

void drawRect( int x1, int y1, int x2, int y2, char r, char g, char b, u8* screen);
void drawFillRect( int x1, int y1, int x2, int y2, char r, char g, char b, u8* screen);

void Debug(const char *format, ...);
