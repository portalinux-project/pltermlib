#include <plterm.h>
#include <plterm-ui.h>

int main(){
	plmt_t* mt = plMTInit(0);
	plterm_t* thingie = plTermInit(mt);

	size_t thingieSize[2];
	plTermGetAttrib(thingieSize, PLTERM_SIZE, thingie);

	pltermdiag_t* thingieDiag = plTermUIDialogBoxInit(thingie, 52, 3, (thingieSize[0] / 2) - 26, thingieSize[1] / 2);

	plTermUISetBackgroundColor(thingie, PLTERM_FONT_BCOL_MAGENTA);
	plTermUIPrintHeader(thingie, "hewwo uwu", PLTERM_FONT_BCOL_WHITE, 1, (thingieSize[0] / 2) - 4);
	plTermUIPrintHeader(thingie, "pwess any key to exit uwu", PLTERM_FONT_BCOL_WHITE, thingieSize[1], (thingieSize[0] / 2) - 16);

	plTermUIDiagSetBackgroundColor(thingieDiag, PLTERM_FONT_BCOL_WHITE, true);
	plTermChangeColor(PLTERM_FONT_FCOL_BLACK);
	plTermUIDiagMovePrint(thingieDiag, 1, 1, "this is the size of the tewminaw uwu: ");

	char buffer[12] = "";
	snprintf(buffer, 12, "%zux%zu", thingieSize[0], thingieSize[1]);
	plTermPrint(thingie, buffer);

	plMTFree(mt, plTermGetInput(thingie));

	plTermStop(thingie);
	plMTStop(mt);
	return 0;
}
