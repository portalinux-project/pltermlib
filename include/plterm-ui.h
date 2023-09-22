/**************************************\
 pltermlib, v0.03
 (c) 2023 pocketlinux32, Under MPL v2.0
 plterm-ui.h: UI header file
\**************************************/
#pragma once
#include <plterm.h>

typedef struct pltermdiag pltermdiag_t;

void plTermUISetBackgroundColor(plterm_t* termStruct, uint8_t color);
void plTermUIPrintHeader(plterm_t* termStruct, plstring_t string, uint8_t color, uint16_t y, uint16_t textOffset);

pltermdiag_t* plTermUIDiagBoxInit(plterm_t* termStruct, uint16_t width, uint16_t height, uint16_t x, uint16_t y, bool drawShadow);
void plTermUIDiagBoxStop(pltermdiag_t* dialogBox);

void plTermUIDiagRender(pltermdiag_t* dialogBox, uint8_t color);
void plTermUIDiagPrint(pltermdiag_t* dialogBox, plstring_t string);
void plTermUIDiagMovePrint(pltermdiag_t* dialogBox, uint16_t x, uint16_t y, plstring_t string);
