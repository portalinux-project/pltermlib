#include <plterm-basic.h>

void readlineMode(plterm_t* termStruct, pltibuf_t* bufferStruct){
	plchar_t keypress = plTermReadline(termStruct, bufferStruct, plRTStrFromCStr("test-shell>", NULL));
	while(keypress.bytes[0] != PLTERM_KEY_ENTER)
		keypress = plTermReadline(termStruct, bufferStruct, plRTStrFromCStr("test-shell>", NULL));
}

void textboxMode(plterm_t* termStruct, pltibuf_t* bufferStruct){
	plchar_t keypress; // = plTermTextBox(termStruct, bufferStruct);
}

int main(int argc, char* argv[]){
	plmt_t* mt = plMTInit(8 * 1024 * 1024);
	plterm_t* term = plTermInit(mt, false);
	pltibuf_t* buffer = plTermCreateTextBuffer(mt, 4096, true);

	if(argc > 1)
		textboxMode(term, buffer);
	else
		readlineMode(term, buffer);

	plTermStop(term);
	plstring_t* stringPtr = plTermGetRawBuffer(buffer);
	plRTStrCompress(stringPtr, mt);
	fputc('\n', stdout);
	fputs(stringPtr->data.pointer, stdout);
	fputc('\n', stdout);
	return 0;
}
