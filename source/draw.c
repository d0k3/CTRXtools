// Copyright 2013 Normmatt
// Licensed under GPLv2 or any later version
// Refer to the license.txt file included.

#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <stdio.h>

#include "font.h"
#include "draw.h"

int current_y = 0;

void ClearScreen(unsigned char *screen, int color)
{
    int i;
    unsigned char *screenPos = screen;
    for (i = 0; i < (SCREEN_HEIGHT * SCREEN_WIDTH); i++) {
        *(screenPos++) = color >> 16;  // B
        *(screenPos++) = color >> 8;   // G
        *(screenPos++) = color & 0xFF; // R
    }
}

void DrawCharacter(unsigned char *screen, int character, int x, int y, int color, int bgcolor)
{
    int yy, xx;
    for (yy = 0; yy < 8; yy++) {
        int xDisplacement = (x * BYTES_PER_PIXEL * SCREEN_WIDTH);
        int yDisplacement = ((SCREEN_WIDTH - (y + yy) - 1) * BYTES_PER_PIXEL);
        unsigned char *screenPos = screen + xDisplacement + yDisplacement;

        unsigned char charPos = font[character * 8 + yy];
        for (xx = 7; xx >= 0; xx--) {
            if ((charPos >> xx) & 1) {
                *(screenPos + 0) = color >> 16;  // B
                *(screenPos + 1) = color >> 8;   // G
                *(screenPos + 2) = color & 0xFF; // R
            } else {
                *(screenPos + 0) = bgcolor >> 16;  // B
                *(screenPos + 1) = bgcolor >> 8;   // G
                *(screenPos + 2) = bgcolor & 0xFF; // R
            }
            screenPos += BYTES_PER_PIXEL * SCREEN_WIDTH;
        }
    }
}

void DrawString(unsigned char *screen, const char *str, int x, int y, int color, int bgcolor)
{
    int i;
    for (i = 0; i < strlen(str); i++)
        DrawCharacter(screen, str[i], x + i * 8, y, color, bgcolor);
}

void DrawStringF(int x, int y, const char *format, ...)
{
    char str[256];
    va_list va;

    va_start(va, format);
    vsnprintf(str, 256, format, va);
    va_end(va);

    DrawString(TOP_SCREEN0, str, x, y, RGB(0, 255, 0), RGB(0, 0, 0));
    DrawString(TOP_SCREEN1, str, x, y, RGB(0, 255, 0), RGB(0, 0, 0));
}

void drawRect( int x1, int y1, int x2, int y2, char r, char g, char b, u8* screen)
{
	drawLine( x1, y1, x2, y1, r, g, b, screen);
	drawLine( x2, y1, x2, y2, r, g, b, screen);
	drawLine( x1, y2, x2, y2, r, g, b, screen);
	drawLine( x1, y1, x1, y2, r, g, b, screen);
}

void drawPixel(int x, int y, char r, char g, char b, u8* screen)
{
	int height=240;
	
	u32 v=(height-1-y+x*height)*3;
	screen[v]=b;
	screen[v+1]=g;
	screen[v+2]=r;
}

void drawLine( int x1, int y1, int x2, int y2, char r, char g, char b, u8* screen)
{

	int x, y;
	if (x1 == x2){
		if (y1<y2) for (y = y1; y < y2; y++) drawPixel(x1,y,r,g,b,screen);
		else for (y = y2; y < y1; y++) drawPixel(x1,y,r,g,b,screen);
	} else {
		if (x1<x2) for (x = x1; x < x2; x++) drawPixel(x,y1,r,g,b,screen);
		else for (x = x2; x < x1; x++) drawPixel(x,y1,r,g,b,screen);
	}
}

void drawFillRect( int x1, int y1, int x2, int y2, char r, char g, char b, u8* screen)
{
	int X1,X2,Y1,Y2,i,j;

	if (x1<x2){ 
		X1=x1;
		X2=x2;
	} else { 
		X1=x2;
		X2=x1;
	} 

	if (y1<y2){ 
		Y1=y1;
		Y2=y2;
	} else { 
		Y1=y2;
		Y2=y1;
	} 
	for(i=X1;i<=X2;i++){
		for(j=Y1;j<=Y2;j++){
			drawPixel(i,j, r, g, b, screen);
		}
	}
}

void Debug(const char *format, ...)
{
    char str[256];
    va_list va;

    va_start(va, format);
    vsnprintf(str, 256, format, va);
    va_end(va);

    if (current_y >= 240) {
        ClearScreen(TOP_SCREEN0, RGB(0, 0, 0));
        ClearScreen(TOP_SCREEN1, RGB(0, 0, 0));
        current_y = 0;
		newline(3);
		drawUI();
    }
    
    DrawString(TOP_SCREEN0, str, 10, current_y, RGB(0, 255, 0), RGB(0, 0, 0));
    DrawString(TOP_SCREEN1, str, 10, current_y, RGB(0, 255, 0), RGB(0, 0, 0));

    current_y += 10;
}
