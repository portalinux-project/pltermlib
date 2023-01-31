#include <plterm.h>

int main(){
	plmt_t* mt = plMTInit(0);
	plterm_t* thingie = plTermInit(mt);

	size_t thingieSize[2];
	plTermGetAttrib(thingieSize, PLTERM_SIZE, thingie);

	plTermChangeColor(47);
	plTermChangeColor(30);
	plTermMovePrint(thingie, (thingieSize[0] / 2) - 4, 1, "hewwo uwu");
	plTermChangeColor(0);
	plTermMovePrint(thingie, (thingieSize[0] / 2) - 24, thingieSize[1] / 2, "this is the size of the tewminaw uwu: ");

	char buffer[12] = "";
	snprintf(buffer, 12, "%zux%zu", thingieSize[0], thingieSize[1]);
	plTermPrint(thingie, buffer);

	plMTFree(mt, plTermGetInput(mt));

	plTermStop(thingie, mt);
	plMTStop(mt);
	return 0;
}
