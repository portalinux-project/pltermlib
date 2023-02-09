#pragma once
#include <pl32.h>
#include <unistd.h>
#include <termios.h>

#define KEY_UP 248
#define KEY_DOWN 249
#define KEY_RIGHT 250
#define KEY_LEFT 251

#define PLTERM_SIZE 1
#define PLTERM_POS 2

typedef struct plterm plterm_t;

void plTermGetAttrib(size_t* buf, int attrib, plterm_t* termStruct);
unsigned string_t plTermGetInput(plmt_t* mt);
void plTermMove(plterm_t* termStruct, uint16_t x, uint16_t y);
void plTermRelMove(plterm_t* termStruct, int x, int y);
int plTermChangeColor(uint8_t color);
void plTermPrint(plterm_t* termStruct, string_t string);
void plTermMovePrint(plterm_t* termStruct, int x, int y, string_t string);
plterm_t* plTermInit(plmt_t* mt);
void plTermStop(plterm_t* termStruct, plmt_t* mt);
