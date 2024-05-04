/**************************************\
 pltermlib, v1.00
 (c) 2023 pocketlinux32, Under MPL v2.0
 plterm-ui.c: UI source file
\**************************************/
#include <plterm-ui.h>

struct pltermdiag {
	plterm_t* terminal;
	uint16_t position[2];
	uint16_t dimensions[2];
	bool drawShadow;
};

struct pltdmenu {
	pltermdiag_t* dialogBox;
	plptr_t menuEntries;
	uint16_t selectedEntry;
	uint16_t padding[2];
	bool stringsPadded;
};

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
	plTermFillArea(dialogBox->terminal, 0, dialogBox->position[0], dialogBox->position[1], dialogBox->dimensions[0], dialogBox->dimensions[1]);
	plMTFree(mt, dialogBox);
}

void plTermUIDiagRender(pltermdiag_t* dialogBox, pltermcolor_t color){
	if(dialogBox->drawShadow)
		plTermFillArea(dialogBox->terminal, PLTERM_FONT_BCOL_BLACK, dialogBox->position[0] + 1, dialogBox->position[1] + 1, dialogBox->position[0] + dialogBox->dimensions[0], dialogBox->position[1] + dialogBox->dimensions[1]);

	if(color != 0)
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

	return retStruct;
}

void plTermUIDiagMenuStop(pltdmenu_t* menu){
	plmt_t* mt;
	plTermGetAttrib(&mt, PLTERM_MT, menu->dialogBox->terminal);

	if(menu->menuEntries.size != 0){
		for(int i = 0; i < menu->menuEntries.size; i++){
			pltdmenuopt_t* menuEntries = menu->menuEntries.pointer;
			plMTFree(mt, menuEntries[menu->menuEntries.size].title.data.pointer);
			plMTFree(mt, menuEntries[menu->menuEntries.size].subtitle.data.pointer);
		}
	}

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
	menu->stringsPadded = false;
}

void plTermUIDiagMenuSelectOption(pltdmenu_t* menu, pltdmenusel_t selectAction){
	if(selectAction == PLTERM_MENU_SEL_DOWN && menu->selectedEntry < menu->menuEntries.size - 1)
		menu->selectedEntry++;
	else if(selectAction == PLTERM_MENU_SEL_UP && menu->selectedEntry != 0)
		menu->selectedEntry--;
}

void plTermUIDiagMenuPadStr(pltdmenu_t* menu){
	plmt_t* mt;
	plTermGetAttrib(&mt, PLTERM_MT, menu->dialogBox->terminal);

	pltdmenuopt_t* menuEntries = menu->menuEntries.pointer;
	size_t menuEntriesSize = menu->menuEntries.size;
	size_t stringLength[2] = {0, 0};

	for(int i = 0; i < menuEntriesSize; i++){
		if(menuEntries[i].title.data.size > stringLength[0])
			stringLength[0] = menuEntries[i].title.data.size;

		if(menuEntries[i].subtitle.data.size > stringLength[1])
			stringLength[1] = menuEntries[i].subtitle.data.size;
	}

	stringLength[0] += 1;
	stringLength[1] += 1;

	for(int i = 0; i < menuEntriesSize; i++){
		memptr_t tempPtr = NULL;
		if(menuEntries[i].title.mt == NULL)
			tempPtr = plMTAlloc(mt, stringLength[0]);
		else
			tempPtr = plMTRealloc(mt, menuEntries[i].title.data.pointer, stringLength[0]);

		if(tempPtr == NULL)
			plRTPanic("plTermUIDiagMenuPadStr", PLRT_ERROR | PLRT_FAILED_ALLOC, true);

		memcpy(tempPtr, menuEntries[i].title.data.pointer, menuEntries[i].title.data.size);
		for(int j = menuEntries[i].title.data.size; j < stringLength[0]; j++)
			((char*)tempPtr)[j] = ' ';

		menuEntries[i].title.data.pointer = tempPtr;
		menuEntries[i].title.data.size = stringLength[0];
		menuEntries[i].title.mt = mt;

		if(menuEntries[i].subtitle.mt == NULL)
			tempPtr = plMTAlloc(mt, stringLength[1]);
		else
			tempPtr = plMTRealloc(mt, menuEntries[i].subtitle.data.pointer, stringLength[1]);

		if(tempPtr == NULL)
			plRTPanic("plTermUIDiagMenuPadStr", PLRT_ERROR | PLRT_FAILED_ALLOC, true);

		memcpy(tempPtr, menuEntries[i].subtitle.data.pointer, menuEntries[i].subtitle.data.size);
		for(int j = menuEntries[i].subtitle.data.size; j < stringLength[1]; j++)
			((char*)tempPtr)[j] = ' ';

		menuEntries[i].subtitle.data.pointer = tempPtr;
		menuEntries[i].subtitle.data.size = stringLength[1];
		menuEntries[i].subtitle.mt = mt;
	}

	menu->stringsPadded = true;
}

pltdmenuopt_t plTermUIDiagMenuGetSelectedOpt(pltdmenu_t* menu){
	return ((pltdmenuopt_t*)menu->menuEntries.pointer)[menu->selectedEntry];
}

void plTermUIDiagMenuRender(pltdmenu_t* menu){
	if(!menu->stringsPadded)
		plTermUIDiagMenuPadStr(menu);

	pltdmenuopt_t holderEntry;
	for(int i = 0; i < menu->menuEntries.size; i++){
		memcpy(&holderEntry, menu->menuEntries.pointer + (sizeof(pltdmenuopt_t) * i), sizeof(pltdmenuopt_t));
		if(menu->padding[1] + i < menu->dialogBox->dimensions[1]){
			if(i == menu->selectedEntry)
				plTermChangeColor(PLTERM_FONT_REVERSED_COLOR);

			plTermUIDiagMovePrint(menu->dialogBox, menu->padding[0], menu->padding[1] + i, holderEntry.title);
			if(holderEntry.subtitle.data.pointer != NULL){
				plTermPrint(menu->dialogBox->terminal, plRTStrFromCStr("\t", NULL));
				plTermPrint(menu->dialogBox->terminal, holderEntry.subtitle);
			}

			if(i == menu->selectedEntry)
				plTermChangeColor(PLTERM_FONT_DEFAULT);
		}
	}
}
