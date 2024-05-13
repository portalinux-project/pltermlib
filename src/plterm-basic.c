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

bool plTermIsNoise(plchar_t inputKey){
	switch(inputKey.bytes[0]){
		case PLTERM_KEY_F1:
		case PLTERM_KEY_F2:
		case PLTERM_KEY_F3:
		case PLTERM_KEY_F4:
		case PLTERM_KEY_F5:
		case PLTERM_KEY_F6:
		case PLTERM_KEY_F7:
		case PLTERM_KEY_F8:
		case PLTERM_KEY_F9:
		case PLTERM_KEY_F10:
		case PLTERM_KEY_F11:
		case PLTERM_KEY_F12:
		case PLTERM_KEY_INS:
		case PLTERM_KEY_PGUP:
		case PLTERM_KEY_PGDN:
		case PLTERM_KEY_HOME:
		case PLTERM_KEY_END:
		case PLTERM_KEY_ENTER:
		case PLTERM_KEY_UP:
		case PLTERM_KEY_DOWN:
		case PLTERM_KEY_LEFT:
		case PLTERM_KEY_RIGHT:
		case PLTERM_KEY_ESCAPE:
		case PLTERM_KEY_CTRL_A:
		case PLTERM_KEY_CTRL_B:
		case PLTERM_KEY_CTRL_C:
		case PLTERM_KEY_CTRL_D:
		case PLTERM_KEY_CTRL_E:
		case PLTERM_KEY_CTRL_F:
		case PLTERM_KEY_CTRL_G:
		case PLTERM_KEY_CTRL_H:
		case PLTERM_KEY_CTRL_I:
		case PLTERM_KEY_CTRL_J:
		case PLTERM_KEY_CTRL_K:
		case PLTERM_KEY_CTRL_L:
		case PLTERM_KEY_CTRL_M:
		case PLTERM_KEY_CTRL_N:
		case PLTERM_KEY_CTRL_O:
		case PLTERM_KEY_CTRL_P:
		case PLTERM_KEY_CTRL_Q:
		case PLTERM_KEY_CTRL_R:
		case PLTERM_KEY_CTRL_S:
		case PLTERM_KEY_CTRL_T:
		case PLTERM_KEY_CTRL_U:
		case PLTERM_KEY_CTRL_V:
		case PLTERM_KEY_CTRL_W:
		case PLTERM_KEY_CTRL_X:
		case PLTERM_KEY_CTRL_Y:
		case PLTERM_KEY_CTRL_Z:
			return true;
	}

	return false;
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
	if(inputKey.bytes[0] == PLTERM_KEY_BACKSPACE || inputKey.bytes[0] == PLTERM_KEY_DEL){
		memmove(buffer->pointer + *bufSeekbyte, buffer->pointer + *bufSeekbyte + 1, buffer->size - *bufSeekbyte - 1);
		buffer->size--;
		if(inputKey.bytes[0] == PLTERM_KEY_BACKSPACE)
			inputKey.bytes[0] = PLTERM_KEY_LEFT;
	}else{
		if(*bufSeekbyte + 1 < buffer->size){
			memmove(buffer->pointer + *bufSeekbyte + 1, buffer->pointer + *bufSeekbyte, buffer->size - *bufSeekbyte);
		((plchar_t*)buffer->pointer)[*bufSeekbyte] = inputKey;
		buffer->size++;
		inputKey.bytes[0] = PLTERM_KEY_RIGHT;
	}

	return plTermTILeftRight(termStruct, *buffer, bufSeekbyte, inputKey, tabWidth);
}

void plTermReadline(plterm_t* termStruct, plstring_t* buffer, plstring_t prompt){
	pltermsc_t currentPos;
	size_t bufferSizeLimit = buffer->data.size;
	size_t bufferSeekbyte = 0;
	if(!buffer->isplChar)
		plRTPanic("plTermReadline", PLRT_ERROR | PLRT_NOT_PLCHAR, true);

	plTermPrint(termStruct, prompt);
	plTermGetAttrib(&currentPos, PLTERM_POS, termStruct);
	plchar_t inputKey = plTermGetInput(termStruct);

	if(inputKey.bytes[0] == PLTERM_KEY_LEFT || inputKey.bytes[0] == PLTERM_KEY_RIGHT)
		currentPos = plTermTILeftRight(termStruct, buffer->data, &bufferSeekbyte, inputKey, 8);
	else if(!plTermIsNoise(inputKey))
		currentPos = plTermTIEditing(termStruct, &buffer->data, &bufferSeekbyte, inputKey, 8);

	
}

void plTermTextbox(plterm_t* termStruct, plptr_t* buffer){
	//TODO: Implement textbox
}
