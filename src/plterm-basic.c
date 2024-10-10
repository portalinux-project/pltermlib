/****************************************************\
 pltermlib, v1.00
 (c) 2023 pocketlinux32, Under MPL v2.0
 plterm-basic.c: Basic terminal routines, source file
\****************************************************/
#define PLRT_ENABLE_HANDLER
#include <plterm-basic.h>

typedef struct pltibuf {
	plstring_t buffer;
	size_t currentUsage;
	size_t offset;
	pltermsc_t startPos;
	bool initialized;
} pltibuf_t;

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

pltibuf_t* plTermCreateTextBuffer(plmt_t* mt, size_t bufferSize, bool usePLChar){
	if(mt == NULL)
		plRTPanic("plTermCreateTextBuffer", PLRT_ERROR | PLRT_NULL_PTR, true);

	pltibuf_t* retStruct = plMTAlloc(mt, sizeof(pltibuf_t));
	retStruct->buffer.data.pointer = NULL;
	retStruct->buffer.data.size = 0;
	retStruct->currentUsage = 1;
	retStruct->offset = 0;
	retStruct->initialized = false;

	if(bufferSize > 0){
		size_t sizeOfEntry = 1;
		if(usePLChar){
			sizeOfEntry = 4;
			retStruct->buffer.isplChar = true;
		}

		retStruct->buffer.data.pointer = plMTAlloc(mt, bufferSize * sizeOfEntry);
		retStruct->buffer.data.size = bufferSize;
	}

	return retStruct;
}

pltibuf_t* plTermPLStringToTextBuffer(plmt_t* mt, plstring_t string){
	if(mt == NULL)
		plRTPanic("plTermCreateTextBuffer", PLRT_ERROR | PLRT_NULL_PTR, true);

	pltibuf_t* retStruct = plTermCreateTextBuffer(mt, 0, false);
	retStruct->buffer = string;
	return retStruct;
}

plstring_t* plTermGetRawBuffer(pltibuf_t* bufferStruct){
	if(bufferStruct == NULL || bufferStruct->buffer.data.pointer == NULL)
		plRTPanic("plTermCreateTextBuffer", PLRT_ERROR | PLRT_NULL_PTR, true);

	return &bufferStruct->buffer;
}

bool plTermIsNoise(plchar_t inputKey){
	switch(inputKey.bytes[0]){
		case 0:
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
		case PLTERM_KEY_UP:
		case PLTERM_KEY_DOWN:
		case PLTERM_KEY_LEFT:
		case PLTERM_KEY_RIGHT:
		case PLTERM_KEY_ESCAPE:
		case PLTERM_KEY_BACKSPACE:
		case PLTERM_KEY_CTRL_A:
		case PLTERM_KEY_CTRL_B:
		case PLTERM_KEY_CTRL_C:
		case PLTERM_KEY_CTRL_D:
		case PLTERM_KEY_CTRL_E:
		case PLTERM_KEY_CTRL_F:
		case PLTERM_KEY_CTRL_G:
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

int16_t plTermDetermineTabWidth(pltibuf_t* bufferStruct, pltermsc_t currentPos, uint8_t direction){
	pltermsc_t workPos = bufferStruct->startPos;
	plchar_t* bufferArray = bufferStruct->buffer.data.pointer;
	size_t workOffset = 0;
	int16_t mvUnits = 1;
	while(workOffset + 1 < bufferStruct->offset){
		mvUnits = 1;
		if(bufferArray[workOffset].bytes[0] == '\t')
			mvUnits = 8 - (workPos.x % 8) + 1;

		workPos.x += mvUnits;
		workOffset++;
	}

	return currentPos.x - workPos.x;
}

pltermsc_t plTermTILeftRight(plterm_t* termStruct, pltibuf_t* bufferStruct, plchar_t inputKey, bool moveOffset){
	pltermsc_t currentPos, terminalSize;
	int16_t movementUnits = 1;
	plTermGetAttrib(&currentPos, PLTERM_POS, termStruct);
	plTermGetAttrib(&terminalSize, PLTERM_SIZE, termStruct);

	if(bufferStruct->offset > 0 && inputKey.bytes[0] == PLTERM_KEY_LEFT){
		if(((plchar_t*)bufferStruct->buffer.data.pointer)[bufferStruct->offset - 1].bytes[0] == '\t')
			movementUnits = plTermDetermineTabWidth(bufferStruct, currentPos, PLTERM_KEY_LEFT);

		if(currentPos.x - movementUnits == 0 || currentPos.x - movementUnits > currentPos.x)
			plTermMove(termStruct, terminalSize.x, currentPos.y - 1);
		else
			plTermRelMove(termStruct, -movementUnits, 0);

		if(moveOffset)
			bufferStruct->offset--;
	}else if(bufferStruct->offset + 1 < bufferStruct->currentUsage && inputKey.bytes[0] == PLTERM_KEY_RIGHT){
		if(((plchar_t*)bufferStruct->buffer.data.pointer)[bufferStruct->offset].bytes[0] == '\t')
			movementUnits = 8 - (currentPos.x % 8) + 1;

		if(currentPos.x + movementUnits >= terminalSize.x)
			plTermMove(termStruct, 1, currentPos.y + 1);
		else
			plTermRelMove(termStruct, movementUnits, 0);

		if(moveOffset)
			bufferStruct->offset++;
	}

	plTermGetAttrib(&currentPos, PLTERM_POS, termStruct);
	return currentPos;
}

pltermsc_t plTermTIDelete(plterm_t* termStruct, pltibuf_t* bufferStruct, plchar_t inputKey){
	pltermsc_t currentPos;
	plTermGetAttrib(&currentPos, PLTERM_POS, termStruct);

	switch(inputKey.bytes[0]){
		case PLTERM_KEY_DEL:
			if(bufferStruct->offset + 1 < bufferStruct->currentUsage)
				memmove(bufferStruct->buffer.data.pointer + (bufferStruct->offset * sizeof(plchar_t)), bufferStruct->buffer.data.pointer + ((bufferStruct->offset + 1) * sizeof(plchar_t)), (bufferStruct->currentUsage - bufferStruct->offset) * sizeof(plchar_t));
			break;
		case PLTERM_KEY_BACKSPACE:
			if(bufferStruct->offset == 0)
				return currentPos;

			memmove(bufferStruct->buffer.data.pointer + ((bufferStruct->offset - 1) * sizeof(plchar_t)), bufferStruct->buffer.data.pointer + (bufferStruct->offset * sizeof(plchar_t)), (bufferStruct->currentUsage - bufferStruct->offset) * sizeof(plchar_t));
			inputKey.bytes[0] = PLTERM_KEY_LEFT;
			plTermTILeftRight(termStruct, bufferStruct, inputKey, false);

			inputKey.bytes[0] = ' ';
			plTermPrintChar(termStruct, inputKey);

			inputKey.bytes[0] = PLTERM_KEY_LEFT;
			currentPos = plTermTILeftRight(termStruct, bufferStruct, inputKey, true);
			inputKey.bytes[0] = PLTERM_KEY_BACKSPACE;
			break;
	}

	(bufferStruct->currentUsage)--;
	return currentPos;
}

pltermsc_t plTermTIInsert(plterm_t* termStruct, pltibuf_t* bufferStruct, plchar_t inputKey){
	if(bufferStruct->offset + 1 < bufferStruct->currentUsage)
		memmove(bufferStruct->buffer.data.pointer + ((bufferStruct->offset + 1) * sizeof(plchar_t)), bufferStruct->buffer.data.pointer + (bufferStruct->offset * sizeof(plchar_t)), (bufferStruct->currentUsage - bufferStruct->offset) * sizeof(plchar_t));

	((plchar_t*)bufferStruct->buffer.data.pointer)[bufferStruct->offset] = inputKey;
	(bufferStruct->currentUsage)++;
	(bufferStruct->offset)++;
	plTermPrintChar(termStruct, inputKey);

	pltermsc_t currentPos;
	plTermGetAttrib(&currentPos, PLTERM_POS, termStruct);
	return currentPos;
}

plchar_t plTermReadline(plterm_t* termStruct, pltibuf_t* bufferStruct, plstring_t prompt){
	pltermsc_t currentPos;
	size_t oldUsage = bufferStruct->currentUsage;
	plmt_t* mt;

	if(!bufferStruct->initialized){
		if(!bufferStruct->buffer.isplChar)
			plRTPanic("plTermReadline", PLRT_ERROR | PLRT_NOT_PLCHAR, true);

		if(prompt.data.pointer != NULL && prompt.data.size > 0){
			plTermPrint(termStruct, prompt);
			plTermRelMove(termStruct, 1, 0);
		}

		plTermGetAttrib(&bufferStruct->startPos, PLTERM_POS, termStruct);
		bufferStruct->initialized = true;
	}

	plTermGetAttrib(&currentPos, PLTERM_POS, termStruct);
	plTermGetAttrib(&mt, PLTERM_MT, termStruct);
	plchar_t inputKey = plTermGetInput(termStruct);

	if(inputKey.bytes[0] == PLTERM_KEY_LEFT || inputKey.bytes[0] == PLTERM_KEY_RIGHT)
		currentPos = plTermTILeftRight(termStruct, bufferStruct, inputKey, true);
	else if((inputKey.bytes[0] == PLTERM_KEY_BACKSPACE || inputKey.bytes[0] == PLTERM_KEY_DEL) && bufferStruct->currentUsage > 1)
		currentPos = plTermTIDelete(termStruct, bufferStruct, inputKey);
	else if(!plTermIsNoise(inputKey) && inputKey.bytes[0] != PLTERM_KEY_ENTER && bufferStruct->currentUsage < bufferStruct->buffer.data.size)
		currentPos = plTermTIInsert(termStruct, bufferStruct, inputKey);

	if(inputKey.bytes[0] != PLTERM_KEY_ENTER && (!plTermIsNoise(inputKey) || inputKey.bytes[0] == PLTERM_KEY_BACKSPACE) && oldUsage != bufferStruct->currentUsage && bufferStruct->offset + 1 < bufferStruct->currentUsage){
		plptr_t tempBuf = bufferStruct->buffer.data;
		tempBuf.pointer = &((plchar_t*)bufferStruct->buffer.data.pointer)[bufferStruct->offset];
		tempBuf.size = (bufferStruct->currentUsage - bufferStruct->offset);

		for(int i = 0; i < tempBuf.size; i++)
			plTermPrintChar(termStruct, ((plchar_t*)tempBuf.pointer)[i]);

		plchar_t space = { .bytes = { ' ', '\0', '\0', '\0' } };
		plTermPrintChar(termStruct, space);
		plTermMove(termStruct, currentPos.x, currentPos.y);
	}

	return inputKey;
}

plchar_t plTermReadscroll(plterm_t* termStruct, pltibuf_t* bufferStruct, ssize_t maxLength){
	pltermsc_t currentPos;
	pltermsc_t maxXPos;
	size_t oldUsage = bufferStruct->currentUsage;
	plmt_t* mt;

	if(!bufferStruct->initialized){
		if(!bufferStruct->buffer.isplChar)
			plRTPanic("plTermReadline", PLRT_ERROR | PLRT_NOT_PLCHAR, true);

		plTermGetAttrib(&bufferStruct->startPos, PLTERM_POS, termStruct);
		bufferStruct->initialized = true;
	}

	//TODO: implement scrolling readline
}
