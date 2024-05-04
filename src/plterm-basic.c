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

pltermsc_t plTermTILeftRight(plterm_t* termStruct, size_t bufSize, size_t* bufSeekbyte, plchar_t inputKey){
	pltermsc_t currentPos, terminalSize;
	plTermGetAttrib(&currentPos, PLTERM_POS, termStruct);
	plTermGetAttrib(&terminalSize, PLTERM_SIZE, termStruct);

	if(*bufSeekbyte > 0 && inputKey.bytes[0] == PLTERM_KEY_LEFT){
		if(currentPos.x == 1)
			plTermMove(termStruct, terminalSize.x, currentPos.y - 1);
		else
			plTermRelMove(termStruct, -1, 0);

		(*bufSeekbyte)--;
	}else if(*bufSeekbyte + 1 < bufSize && inputKey.bytes[0] == PLTERM_KEY_RIGHT){
		if(currentPos.x == terminalSize.x)
			plTermMove(termStruct, 1, currentPos.y + 1);
		else
			plTermRelMove(termStruct, 1, 0);

		(*bufSeekbyte)++;
	}

	plTermGetAttrib(&currentPos, PLTERM_POS, termStruct);
	return currentPos;
}

pltermsc_t plTermTIEditing(plterm_t* termStruct, plptr_t* buffer, size_t* bufSeekbyte, plchar_t inputKey){
	pltermsc_t currentPos, terminalSize;
	plTermGetAttrib(&currentPos, PLTERM_POS, termStruct);
	plTermGetAttrib(&terminalSize, PLTERM_SIZE, termStruct);

	plptr_t writeArray = {
		.pointer = inputKey.bytes,
		.size = 1
	};
	while(inputKey.bytes[writeArray.size] != 0 && writeArray.size < 4)
		writeArray.size++;

	memcpy(buffer->pointer + *bufSeekbyte, writeArray.pointer, writeArray.size);
	*bufSeekbyte += writeArray.size;
	if(*bufSeekbyte + 1 > buffer->size)
		buffer->size = *bufSeekbyte + 1;

	plTermPrint(termStruct, plRTStrFromPLPtr(writeArray, NULL, false, false));
	currentPos.x += 1;
	if(currentPos.x > terminalSize.x || inputKey.bytes[0] == '\n'){
		currentPos.y++;
		currentPos.x = 1;
	}

	return currentPos;
}

void plTermReadline(plterm_t* termStruct, plptr_t* buffer){
	size_t bufferSizeLimit = buffer->size;
	size_t bufferSeekbyte = 0;
	buffer->size = strlen(buffer->pointer);

	bool directionalKey = false;
	plchar_t inputKey = plTermGetInput(termStruct);
}

void plTermTextbox(plterm_t* termStruct, plptr_t* buffer){
	pltermsc_t startPos, currentPos, terminalSize;
	size_t bufferSizeLimit = buffer->size;
	size_t bufferSeekbyte = 0;
	buffer->size = strlen(buffer->pointer);
	plTermGetAttrib(&startPos, PLTERM_POS, termStruct);
	plTermGetAttrib(&terminalSize, PLTERM_SIZE, termStruct);
	currentPos = startPos;
	if(buffer->size > 0)
		plTermPrint(termStruct, plRTStrFromPLPtr(*buffer, NULL, false, false));

	bool directionalKey = false;
	plchar_t inputKey = plTermGetInput(termStruct);
	switch(inputKey.bytes[0]){
		case PLTERM_KEY_UP:
			if(currentPos.y == startPos.y){
				if(currentPos.x != startPos.x){
					plTermMove(termStruct, startPos.x, startPos.y);
					currentPos = startPos;
					bufferSeekbyte = 0;
				}
			}else{
				plTermRelMove(termStruct, 0, -1);
				currentPos.y--;
				bufferSeekbyte -= terminalSize.x;
			}
			directionalKey = true;
			break;
		case PLTERM_KEY_DOWN:
			if((bufferSeekbyte + 1) + terminalSize.x < buffer->size){
				plTermRelMove(termStruct, 0, 1);
				currentPos.y++;
				bufferSeekbyte += terminalSize.x;
			}else if(bufferSeekbyte + 1 != buffer->size){
				if(currentPos.x + (buffer->size - (bufferSeekbyte + 1)) < terminalSize.x)
					plTermRelMove(termStruct, buffer->size - (bufferSeekbyte + 1), 0);
				else
					plTermMove(termStruct, buffer->size - ((currentPos.y) * terminalSize.x), currentPos.y + 1);

				plTermGetAttrib(&currentPos, PLTERM_POS, termStruct);
				bufferSeekbyte = buffer->size - 1;
			}
			directionalKey = true;
			break;
		case PLTERM_KEY_LEFT:
		case PLTERM_KEY_RIGHT:
			currentPos = plTermTILeftRight(termStruct, buffer->size, &bufferSeekbyte, inputKey);
		case PLTERM_KEY_BACKSPACE:
			directionalKey = true;
	}

	if(buffer->size < bufferSizeLimit && !directionalKey)
		currentPos = plTermTIEditing(termStruct, buffer, &bufferSeekbyte, inputKey);
}
