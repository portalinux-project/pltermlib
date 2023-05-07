/**************************************\
 pltermlib, v0.02
 (c) 2023 pocketlinux32, Under MPL v2.0
 plterm.h: Main header file
\**************************************/
#pragma once
#include <pl32.h>
#include <unistd.h>
#include <termios.h>

#define PLTERM_KEY_UP 248
#define PLTERM_KEY_DOWN 249
#define PLTERM_KEY_RIGHT 250
#define PLTERM_KEY_LEFT 251

#define PLTERM_FONT_DEFAULT 0
#define PLTERM_FONT_BOLD 1
#define PLTERM_FONT_FCOL_BLACK 30
#define PLTERM_FONT_FCOL_RED 31
#define PLTERM_FONT_FCOL_GREEN 32
#define PLTERM_FONT_FCOL_YELLOW 33
#define PLTERM_FONT_FCOL_BLUE 34
#define PLTERM_FONT_FCOL_MAGENTA 35
#define PLTERM_FONT_FCOL_CYAN 36
#define PLTERM_FONT_FCOL_WHITE 37
#define PLTERM_FONT_BCOL_BLACK 40
#define PLTERM_FONT_BCOL_RED 41
#define PLTERM_FONT_BCOL_GREEN 42
#define PLTERM_FONT_BCOL_YELLOW 43
#define PLTERM_FONT_BCOL_BLUE 44
#define PLTERM_FONT_BCOL_MAGENTA 45
#define PLTERM_FONT_BCOL_CYAN 46
#define PLTERM_FONT_BCOL_WHITE 47

#define PLTERM_SIZE 1
#define PLTERM_POS 2
#define PLTERM_MT 3

typedef struct plterm plterm_t;

void plTermGetAttrib(memptr_t buf, int attrib, plterm_t* termStruct);
byte_t* plTermGetInput(plterm_t* termStruct);
void plTermMove(plterm_t* termStruct, uint16_t x, uint16_t y);
void plTermRelMove(plterm_t* termStruct, int x, int y);
int plTermChangeColor(uint8_t color);
void plTermPrint(plterm_t* termStruct, string_t string);
void plTermMovePrint(plterm_t* termStruct, uint16_t x, uint16_t y, string_t string);
void plTermFillArea(plterm_t* termStruct, uint8_t color, uint16_t xStart, uint16_t yStart, uint16_t xStop, uint16_t yStop);
plterm_t* plTermInit(plmt_t* mt);
void plTermStop(plterm_t* termStruct);
