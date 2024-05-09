/****************************************************\
 pltermlib, v1.00
 (c) 2023 pocketlinux32, Under MPL v2.0
 plterm-basic.c: Basic terminal routines, source file
\****************************************************/
#include <plterm-basic.h>

bool isCharEqual(plchar_t char1, plchar_t char2){
	for(int i = 0; i < 4; i++){
		if(char1.bytes[i] != char2.bytes[i])
			return false;
	}

	return true;
}

void plTermSetBackground(plterm_t* termStruct, pltermcolor_t color){
	if(color < 40 && color > 47)
		return;

	size_t terminalSize[2];
	plTermGetAttrib(terminalSize, PLTERM_SIZE, termStruct);
	plTermFillArea(termStruct, color, 1, 1, terminalSize[0], terminalSize[1]);
}

void plTermPrintHeader(plterm_t* termStruct, plstring_t string, pltermcolor_t color, uint16_t y, uint16_t textOffset){
	size_t terminalSize[2];
	plTermGetAttrib(terminalSize, PLTERM_SIZE, termStruct);
	plTermFillArea(termStruct, color, 1, y, terminalSize[0], y);

	if(color > 7 && color != PLTERM_FONT_BCOL_BLACK){
		plTermChangeColor(PLTERM_FONT_FCOL_BLACK);
	}
	plTermMovePrint(termStruct, textOffset, y, string);
	plTermChangeColor(PLTERM_FONT_DEFAULT);
}

pltermsc_t plTermTILeftRight(plterm_t* termStruct, plptr_t buffer, size_t* bufSeekbyte, plchar_t inputKey, uint16_t tabWidth){
	pltermsc_t currentPos, terminalSize;
	int16_t movementUnits = 1;
	plTermGetAttrib(&currentPos, PLTERM_POS, termStruct);
	plTermGetAttrib(&terminalSize, PLTERM_SIZE, termStruct);

	if(*bufSeekbyte > 0 && inputKey.bytes[0] == PLTERM_KEY_LEFT){
		if(((plchar_t*)buffer.pointer)[*bufSeekbyte - 1].bytes[0] == '\t')
			movementUnits = tabWidth;

		if(currentPos.x - movementUnits == 0 || currentPos.x - movementUnits > currentPos.x)
			plTermMove(termStruct, terminalSize.x, currentPos.y - 1);
		else
			plTermRelMove(termStruct, -movementUnits, 0);

		(*bufSeekbyte)--;
	}else if(*bufSeekbyte + 1 < buffer.size && inputKey.bytes[0] == PLTERM_KEY_RIGHT){
		if(((plchar_t*)buffer.pointer)[*bufSeekbyte].bytes[0] == '\t')
			movementUnits = tabWidth;

		if(currentPos.x + movementUnits >= terminalSize.x)
			plTermMove(termStruct, 1, currentPos.y + 1);
		else
			plTermRelMove(termStruct, movementUnits, 0);

		(*bufSeekbyte)++;
	}

	plTermGetAttrib(&currentPos, PLTERM_POS, termStruct);
	return currentPos;
}

pltermsc_t plTermTIEditing(plterm_t* termStruct, plptr_t* buffer, size_t* bufSeekbyte, plchar_t inputKey, uint16_t tabWidth){
	if(*bufSeekbyte + 1 < buffer->size)
		memmove(buffer->pointer + *bufSeekbyte + 1, buffer->pointer + *bufSeekbyte, buffer->size - *bufSeekbyte);

	((plchar_t*)buffer->pointer)[*bufSeekbyte] = inputKey;
	buffer->size++;

	return plTermTILeftRight(termStruct, *buffer, bufSeekbyte, inputKey, tabWidth);
}

void plTermReadline(plterm_t* termStruct, plptr_t* buffer){
	size_t bufferSizeLimit = buffer->size;
	size_t bufferSeekbyte = 0;
	buffer->size = strlen(buffer->pointer);

	bool directionalKey = false;
	plchar_t inputKey = plTermGetInput(termStruct);
}

void plTermTextbox(plterm_t* termStruct, plptr_t* buffer){

}
