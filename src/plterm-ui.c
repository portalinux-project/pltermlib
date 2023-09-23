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

	plTermChangeColor(PLTERM_FONT_FCOL_BLACK);
	plTermMovePrint(termStruct, textOffset, y, string);
	plTermChangeColor(PLTERM_FONT_DEFAULT);
}

pltermdiag_t* plTermUIDiagBoxInit(plterm_t* termStruct, uint16_t width, uint16_t height, uint16_t x, uint16_t y, bool drawShadow){
	plmt_t* mt;
	plTermGetAttrib(&mt, PLTERM_MT, termStruct);

	pltermdiag_t* retStruct = plMTAlloc(mt, sizeof(pltermdiag_t));
	retStruct->terminal = termStruct;
	retStruct->position[0] = x;
	retStruct->position[1] = y;
	retStruct->dimensions[0] = width;
	retStruct->dimensions[1] = height;
	retStruct->drawShadow = drawShadow;

	return retStruct;
}

void plTermUIDiagBoxStop(pltermdiag_t* dialogBox){
	if(dialogBox->drawShadow){
		dialogBox->dimensions[0]++;
		dialogBox->dimensions[1]++;
	}

	plmt_t* mt;
	plTermGetAttrib(&mt, PLTERM_MT, dialogBox->terminal);
	plTermFillArea(dialogBox->terminal, currentBColor, dialogBox->position[0], dialogBox->position[1], dialogBox->dimensions[0], dialogBox->dimensions[1]);
	plMTFree(mt, dialogBox);
}

void plTermUIDiagRender(pltermdiag_t* dialogBox, uint8_t color){
	if(dialogBox->drawShadow)
		plTermFillArea(dialogBox->terminal, PLTERM_FONT_BCOL_BLACK, dialogBox->position[0] + 1, dialogBox->position[1] + 1, dialogBox->position[0] + dialogBox->dimensions[0], dialogBox->position[1] + dialogBox->dimensions[1]);

	plTermFillArea(dialogBox->terminal, color, dialogBox->position[0], dialogBox->position[1], dialogBox->position[0] + dialogBox->dimensions[0] - 1, dialogBox->position[1] + dialogBox->dimensions[1] - 1);
}

void plTermUIDiagMove(pltermdiag_t* dialogBox, uint16_t x, uint16_t y){
	if(x > dialogBox->dimensions[0])
		x = dialogBox->dimensions[0];
	if(x > dialogBox->dimensions[1])
		y = dialogBox->dimensions[1];

	plTermMove(dialogBox->terminal, dialogBox->position[0] + x - 1, dialogBox->position[1] + y - 1);
}

void plTermUIDiagMovePrint(pltermdiag_t* dialogBox, uint16_t x, uint16_t y, plstring_t string){
	plTermUIDiagMove(dialogBox, x, y);
	plTermPrint(dialogBox->terminal, string);
}
