/**************************************\
 pltermlib, v0.03
 (c) 2023 pocketlinux32, Under MPL v2.0
 plterm-ui.c: UI source file
\**************************************/
#include <plterm-ui.h>

pltermcolor_t currentBColor = PLTERM_FONT_DEFAULT;

struct pltermdiag {
	plterm_t* terminal;
	uint16_t position[2];
	uint16_t dimensions[2];
	bool drawShadow;
};

typedef struct pltdmenuopt {
	plstring_t title;
	plstring_t subtitle;
} pltdmenuopt_t;

struct pltdmenu {
	pltermdiag_t* dialogBox;
	plptr_t menuEntries;
	uint16_t selectedEntry;
	uint16_t padding[2];
};

void plTermUISetBackground(plterm_t* termStruct, pltermcolor_t color){
	if(color < 40 && color > 47)
		return;

	size_t terminalSize[2];
	plTermGetAttrib(terminalSize, PLTERM_SIZE, termStruct);
	plTermFillArea(termStruct, color, 1, 1, terminalSize[0], terminalSize[1]);
}

void plTermUIPrintHeader(plterm_t* termStruct, plstring_t string, pltermcolor_t color, uint16_t y, uint16_t textOffset){
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

void plTermUIDiagRender(pltermdiag_t* dialogBox, pltermcolor_t color){
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
	if(y <= dialogBox->dimensions[1]){
		if(x + string.data.size > dialogBox->dimensions[0]){
			string.data.size -= (x + string.data.size) - dialogBox->dimensions[0];
		}

		plTermUIDiagMove(dialogBox, x, y);
		plTermPrint(dialogBox->terminal, string);
	}
}

pltdmenu_t* plTermUIDiagMenuCreate(pltermdiag_t* dialogBox, uint16_t xPadding, uint16_t yPadding){
	if(dialogBox == NULL)
		plRTPanic("plTermUIDiagMenuCreate", PLRT_ERROR | PLRT_NULL_PTR, true);

	plmt_t* mt;
	plTermGetAttrib(&mt, PLTERM_MT, dialogBox->terminal);

	pltdmenu_t* retStruct = plMTAlloc(mt, sizeof(pltdmenu_t));
	retStruct->dialogBox = dialogBox;
	retStruct->padding[0] = xPadding;
	retStruct->padding[1] = yPadding;

	retStruct->menuEntries.pointer = plMTAlloc(mt, 2 * sizeof(pltdmenuopt_t));
	retStruct->menuEntries.size = 0;
	retStruct->selectedEntry = 0;
}

void plTermUIDiagMenuStop(pltdmenu_t* menu){
	plmt_t* mt;
	plTermGetAttrib(&mt, PLTERM_MT, menu->dialogBox->terminal);

	plTermUIDiagBoxStop(menu->dialogBox);
	plMTFree(mt, menu);
}

void plTermUIDiagMenuAddOption(pltdmenu_t* menu, plstring_t title, plstring_t subtitle){
	if(title.data.pointer == NULL)
		plRTPanic("plTermUIDiagMenuAddOption", PLRT_ERROR | PLRT_NULL_PTR, true);

	plmt_t* mt;
	plTermGetAttrib(&mt, PLTERM_MT, menu->dialogBox->terminal);

	if(menu->menuEntries.size >= 2){
		memptr_t tempPtr = plMTRealloc(mt, menu->menuEntries.pointer, (menu->menuEntries.size + 1) * sizeof(pltdmenuopt_t));
		if(tempPtr == NULL)
			plRTPanic("plTermUIDiagMenuAddOption", PLRT_ERROR | PLRT_FAILED_ALLOC, false);

		menu->menuEntries.pointer = tempPtr;
	}

	pltdmenuopt_t* menuEntries = menu->menuEntries.pointer;
	menuEntries[menu->menuEntries.size].title = title;
	menuEntries[menu->menuEntries.size].subtitle = subtitle;

	menu->menuEntries.size++;
}

void plTermUIDiagMenuSelectOption(pltdmenu_t* menu, pltdmenusel_t selectAction){
	if(selectAction == PLTERM_MENU_SEL_UP && menu->selectedEntry < menu->menuEntries.size - 1)
		menu->selectedEntry++;
	else if(selectAction == PLTERM_MENU_SEL_DOWN && menu->selectedEntry != 0)
		menu->selectedEntry--;
}

void plTermUIDiagMenuRender(pltdmenu_t* menu){
	pltdmenuopt_t holderEntry;
	for(int i = 0; i < menu->menuEntries.size; i++){
		memcpy(&holderEntry, menu->menuEntries.pointer, sizeof(pltdmenuopt_t));
		if(menu->padding[1] + i < menu->dialogBox->dimensions[1]){
			if(i == menu->selectedEntry)
				plTermChangeColor(PLTERM_FONT_REVERSED_COLOR);

			plTermUIDiagMovePrint(menu->dialogBox, menu->padding[0], menu->padding[1] + i, holderEntry.title);
			if(holderEntry.subtitle.data.pointer != NULL){
				plTermPrint(menu->dialogBox->terminal, plRTStrFromCStr("\t", NULL));
				plTermPrint(menu->dialogBox->terminal, holderEntry.subtitle);
			}
		}
	}
}
