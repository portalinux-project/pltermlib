/**************************************\
 pltermlib, v0.03
 (c) 2023 pocketlinux32, Under MPL v2.0
 plterm-ui.h: UI header file
\**************************************/
#pragma once
#include <plterm.h>

typedef enum pltdmenusel {
	PLTERM_MENU_SEL_UP,
	PLTERM_MENU_SEL_DOWN
} pltdmenusel_t;

typedef struct pltdmenuopt {
	plstring_t title;
	plstring_t subtitle;
} pltdmenuopt_t;

typedef struct pltermdiag pltermdiag_t;
typedef struct pltdmenu pltdmenu_t;


void plTermUISetBackground(plterm_t* termStruct, pltermcolor_t color);
void plTermUIPrintHeader(plterm_t* termStruct, plstring_t string, pltermcolor_t color, uint16_t y, uint16_t textOffset);

pltermdiag_t* plTermUIDiagBoxInit(plterm_t* termStruct, uint16_t width, uint16_t height, uint16_t x, uint16_t y, bool drawShadow);
void plTermUIDiagBoxStop(pltermdiag_t* dialogBox);

void plTermUIDiagRender(pltermdiag_t* dialogBox, pltermcolor_t color);
void plTermUIDiagMove(pltermdiag_t* dialogBox, uint16_t x, uint16_t y);
void plTermUIDiagMovePrint(pltermdiag_t* dialogBox, uint16_t x, uint16_t y, plstring_t string);

pltdmenu_t* plTermUIDiagMenuCreate(pltermdiag_t* dialogBox, uint16_t xPadding, uint16_t yPadding);
void plTermUIDiagMenuStop(pltdmenu_t* menu);
void plTermUIDiagMenuAddOption(pltdmenu_t* menu, plstring_t title, plstring_t subtitle);
void plTermUIDiagMenuSelectOption(pltdmenu_t* menu, pltdmenusel_t selectAction);
pltdmenuopt_t plTermUIDiagMenuGetSelectedOpt(pltdmenu_t* menu);
void plTermUIDiagMenuRender(pltdmenu_t* menu);
