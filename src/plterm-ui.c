/**************************************\
 pltermlib, v0.03
 (c) 2023 pocketlinux32, Under MPL v2.0
 plterm-ui.c: UI source file
\**************************************/
#include <plterm-ui.h>

uint8_t currentBColor = PLTERM_FONT_DEFAULT;

struct pltermdiag {
	plterm_t* terminal;
	uint16_t position[2];
	uint16_t dimensions[2];
	bool drawShadow;
};

void plTermUISetBackground(plterm_t* termStruct, uint8_t color){
	if(color < 40 && color > 47)
		return;

	size_t terminalSize[2];
	plTermGetAttrib(terminalSize, PLTERM_SIZE, termStruct);
	plTermFillArea(termStruct, color, 1, 1, terminalSize[0], terminalSize[1]);
}

void plTermUIPrintHeader(plterm_t* termStruct, plstring_t string, uint8_t color, uint16_t y, uint16_t textOffset){
	size_t terminalSize[2];
	plTermGetAttrib(terminalSize, PLTERM_SIZE, termStruct);
	plTermFillArea(termStruct, color, 1, y, terminalSize[0], y);

	plTermSetColor(PLTERM_FONT_FCOL_BLACK);
	plTermMovePrint(termStruct, 1, y, string);
	plTermSetColor(PLTERM_FONT_DEFAULT);
}
