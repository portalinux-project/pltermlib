#include <plterm.h>
#include <plterm-ui.h>

int main(){
	plmt_t* mt = plMTInit(0);
	plterm_t* thingie = plTermInit(mt, false);
	plTermClearScreen(thingie);
	plTermToggleCursor(thingie);

	pltermsc_t thingieSize;
	plTermGetAttrib(&thingieSize, PLTERM_SIZE, thingie);

	pltermdiag_t* thingieDiag = plTermUIDiagBoxInit(thingie, 52, 3, (thingieSize.x / 2) - 26, thingieSize.y / 2, true);

	plTermSetBackground(thingie, PLTERM_FONT_BCOL_MAGENTA);
	plTermPrintHeader(thingie, plRTStrFromCStr("hewwo uwu", NULL), PLTERM_FONT_BCOL_WHITE, 1, (thingieSize.x / 2) - 4);
	plTermPrintHeader(thingie, plRTStrFromCStr("pwess any key to exit uwu", NULL), PLTERM_FONT_BCOL_WHITE, thingieSize.y, (thingieSize.x / 2) - 10);

	plTermUIDiagRender(thingieDiag, PLTERM_FONT_BCOL_WHITE);
	plTermChangeColor(PLTERM_FONT_FCOL_BLACK);
	plTermUIDiagMovePrint(thingieDiag, 2, 2, plRTStrFromCStr("this is the size of the tewminaw uwu: ", NULL));

	char buffer[12] = "";
	snprintf(buffer, 12, "%zux%zu", thingieSize.x, thingieSize.y);
	plTermPrint(thingie, plRTStrFromCStr(buffer, NULL));

	plchar_t input = plTermGetInput(thingie);
	while(input.bytes[0] != PLTERM_KEY_ENTER && input.bytes[0] != PLTERM_KEY_ESCAPE)
		input = plTermGetInput(thingie);

	plTermUIDiagBoxStop(thingieDiag);
	plTermClearScreen(thingie);
	plTermStop(thingie);
	plMTStop(mt);
	return 0;
}
