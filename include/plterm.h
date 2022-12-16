#pragma once
#include <pl32.h>
#include <unistd.h>
#include <termios.h>

#define KEY_UP 248
#define KEY_DOWN 249
#define KEY_RIGHT 250
#define KEY_LEFT 251

typedef struct plterm {
	struct termios original;
	struct termios current;
	uint16_t xSize;
	uint16_t ySize;
	uint16_t xPos;
	uint16_t yPos;
} plterm_t;

void plTermGetSize(plterm_t* termStruct);
void plTermInputDriver(unsigned char** bufferPointer, char* inputBuffer, plmt_t* mt);
unsigned char* plTermGetInput(plmt_t* mt);
void plTermMove(plterm_t* termStruct, int x,  int y);
int plTermChangeColor(uint8_t color);
void plTermPrint(plterm_t* termStruct, char* string);
void plTermMovePrint(plterm_t* termStruct, int x, int y, char* string);
plterm_t* plTermInit(plmt_t* mt);
void plTermStop(plterm_t* termStruct, plmt_t* mt);
