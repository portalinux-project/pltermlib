#include <plterm.h>
#include <plterm-ui.h>

int main(int argc, const char* argv[]){
	plmt_t* mt = plMTInit(8 * 1024 * 1024);
	plterm_t* thingie = plTermInit(mt, false);
	plTermClearScreen(thingie);
	plTermToggleCursor(thingie);

	pltermsc_t thingieSize;
	plTermGetAttrib(&thingieSize, PLTERM_SIZE, thingie);

	pltermdiag_t* thingieDiag = plTermUIDiagBoxInit(thingie, 72, 24, (thingieSize.x / 2) - 35, (thingieSize.y / 2) - 12, false);
	pltdmenu_t* thingieMenu = plTermUIDiagMenuCreate(thingieDiag, 3, 2);

	plTermUIDiagMenuAddOption(thingieMenu, plRTStrFromCStr("option 1", NULL), plRTStrFromCStr("funny text", NULL));
	plTermUIDiagMenuAddOption(thingieMenu, plRTStrFromCStr("option 2", NULL), plRTStrFromCStr("funny text 2 electric bogaloo", NULL));
	plTermUIDiagMenuAddOption(thingieMenu, plRTStrFromCStr("option 3", NULL), plRTStrFromCStr("bottom text", NULL));
	plTermUIDiagMenuAddOption(thingieMenu, plRTStrFromCStr("option 4", NULL), plRTStrFromCStr("bazinga", NULL));

	plTermPrintHeader(thingie, plRTStrFromCStr("hewwo ^w^", NULL), PLTERM_FONT_REVERSED_COLOR, 1, (thingieSize.x / 2) - 3);
	plTermUIDiagRender(thingieDiag, PLTERM_FONT_REVERSED_COLOR);
	plTermUIDiagMenuRender(thingieMenu);

	plchar_t input = plTermGetInput(thingie);
	while(input.bytes[0] != PLTERM_KEY_ENTER){
		switch(input.bytes[0]){
			case PLTERM_KEY_UP:
				plTermUIDiagMenuSelectOption(thingieMenu, PLTERM_MENU_SEL_UP);
				break;
			case PLTERM_KEY_DOWN:
				plTermUIDiagMenuSelectOption(thingieMenu, PLTERM_MENU_SEL_DOWN);
				break;
		}

		if(input.bytes[0] != 0)
			plTermUIDiagMenuRender(thingieMenu);

		input = plTermGetInput(thingie);
	}
	pltdmenuopt_t selectedOption = plTermUIDiagMenuGetSelectedOpt(thingieMenu);

	plTermUIDiagMenuStop(thingieMenu);
	plTermClearScreen(thingie);
	plTermStop(thingie);

	write(STDOUT_FILENO, "you selected: ", 14);
	write(STDOUT_FILENO, selectedOption.title.data.pointer, selectedOption.title.data.size);
	write(STDOUT_FILENO, "\n", 1);
	plMTStop(mt);
	return 0;
}
