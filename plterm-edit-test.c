#include <plterm-basic.h>

int main(int argc, char* argv[]){
	plmt_t* mt = plMTInit(8 * 1024 * 1024);
	plterm_t* term = plTermInit(mt, false);
	pltibuf_t* buffer = plTermCreateTextBuffer(mt, 4096, true);
	plchar_t keypress = plTermReadline(term, buffer, plRTStrFromCStr("test-shell>", NULL));

	while(keypress.bytes[0] != PLTERM_KEY_ENTER)
		keypress = plTermReadline(term, buffer, plRTStrFromCStr("test-shell>", NULL));

	plTermStop(term);
	plstring_t* stringPtr = plTermGetRawBuffer(buffer);
	plRTStrCompress(stringPtr, mt);
	fputc('\n', stdout);
	fputs(stringPtr->data.pointer, stdout);
	fputc('\n', stdout);
	return 0;
}
