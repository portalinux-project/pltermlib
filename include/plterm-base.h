#pragma once
#include <plrt.h>
#include <fcntl.h>
#include <unistd.h>
#include <termios.h>

#define PLTERM_VERSION "0.05"
#define PLTERM_API_VER 0
#define PLTERM_FEATURELVL 5
#define PLTERM_PATCHLVL 0

typedef enum pltermkey {
//	NONO_1 = 192,
//	NONO_2 = 224,
//	NONO_3 = 240,
	PLTERM_KEY_F1 = 226,
	PLTERM_KEY_F2 = 227,
	PLTERM_KEY_F3 = 228,
	PLTERM_KEY_F4 = 229,
	PLTERM_KEY_F5 = 230,
	PLTERM_KEY_F6 = 231,
	PLTERM_KEY_F7 = 232,
	PLTERM_KEY_F8 = 233,
	PLTERM_KEY_F9 = 234,
	PLTERM_KEY_F10 = 235,
	PLTERM_KEY_F11 = 236,
	PLTERM_KEY_F12 = 237,
	PLTERM_KEY_INS = 242,
	PLTERM_KEY_DEL = 243,
	PLTERM_KEY_PGUP = 244,
	PLTERM_KEY_PGDN = 245,
	PLTERM_KEY_HOME = 246,
	PLTERM_KEY_END = 247,
	PLTERM_KEY_UP = 248,
	PLTERM_KEY_DOWN = 249,
	PLTERM_KEY_RIGHT = 250,
	PLTERM_KEY_LEFT = 251,
	PLTERM_KEY_ENTER = 10,
	PLTERM_KEY_BACKSPACE = 8,
	PLTERM_KEY_ESCAPE = 27,
	PLTERM_KEY_CTRL_A = 1,
	PLTERM_KEY_CTRL_B = 2,
	PLTERM_KEY_CTRL_C = 3,
	PLTERM_KEY_CTRL_D = 4,
	PLTERM_KEY_CTRL_E = 5,
	PLTERM_KEY_CTRL_F = 6,
	PLTERM_KEY_CTRL_G = 7,
	PLTERM_KEY_CTRL_H = 8,
	PLTERM_KEY_CTRL_I = 9,
	PLTERM_KEY_CTRL_J = 10,
	PLTERM_KEY_CTRL_K = 11,
	PLTERM_KEY_CTRL_L = 12,
	PLTERM_KEY_CTRL_M = 13,
	PLTERM_KEY_CTRL_N = 14,
	PLTERM_KEY_CTRL_O = 15,
	PLTERM_KEY_CTRL_P = 16,
	PLTERM_KEY_CTRL_Q = 17,
	PLTERM_KEY_CTRL_R = 18,
	PLTERM_KEY_CTRL_S = 19,
	PLTERM_KEY_CTRL_T = 20,
	PLTERM_KEY_CTRL_U = 21,
	PLTERM_KEY_CTRL_V = 22,
	PLTERM_KEY_CTRL_W = 23,
	PLTERM_KEY_CTRL_X = 24,
	PLTERM_KEY_CTRL_Y = 25,
	PLTERM_KEY_CTRL_Z = 26
} pltermkey_t;

typedef enum pltermcolor {
	PLTERM_FONT_DEFAULT = 0,
	PLTERM_FONT_BOLD = 1,
	PLTERM_FONT_BLINKING = 5,
	PLTERM_FONT_REVERSED_COLOR = 7,
	PLTERM_FONT_FCOL_BLACK = 30,
	PLTERM_FONT_FCOL_RED = 31,
	PLTERM_FONT_FCOL_GREEN = 32,
	PLTERM_FONT_FCOL_YELLOW = 33,
	PLTERM_FONT_FCOL_BLUE = 34,
	PLTERM_FONT_FCOL_MAGENTA = 35,
	PLTERM_FONT_FCOL_CYAN = 36,
	PLTERM_FONT_FCOL_WHITE = 37,
	PLTERM_FONT_BCOL_BLACK = 40,
	PLTERM_FONT_BCOL_RED = 41,
	PLTERM_FONT_BCOL_GREEN = 42,
	PLTERM_FONT_BCOL_YELLOW = 43,
	PLTERM_FONT_BCOL_BLUE = 44,
	PLTERM_FONT_BCOL_MAGENTA = 45,
	PLTERM_FONT_BCOL_CYAN = 46,
	PLTERM_FONT_BCOL_WHITE = 47
} pltermcolor_t;

typedef enum pltermaction {
	PLTERM_SIZE = 1,
	PLTERM_POS = 2,
	PLTERM_MT = 3
} pltermaction_t;

typedef struct simpleCoord {
	uint16_t x;
	uint16_t y;
} pltermsc_t;

typedef struct plterm plterm_t;

void plTermGetAttrib(memptr_t buf, pltermaction_t attrib, plterm_t* termStruct);
void plTermToggleCursor(plterm_t* termStruct);
void plTermClearScreen(plterm_t* termStruct);
plchar_t plTermGetInput(plterm_t* termStruct);

void plTermMove(plterm_t* termStruct, uint16_t x, uint16_t y);
void plTermRelMove(plterm_t* termStruct, int x, int y);
int plTermChangeColor(pltermcolor_t color);
void plTermPrint(plterm_t* termStruct, plstring_t string);
void plTermMovePrint(plterm_t* termStruct, uint16_t x, uint16_t y, plstring_t string);

void plTermFillArea(plterm_t* termStruct, pltermcolor_t color, uint16_t xStart, uint16_t yStart, uint16_t xStop, uint16_t yStop);

plterm_t* plTermInit(plmt_t* mt, bool nonblockInput);
void plTermStop(plterm_t* termStruct);
