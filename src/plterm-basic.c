/****************************************************\
 pltermlib, v1.00
 (c) 2023 pocketlinux32, Under MPL v2.0
 plterm-basic.c: Basic terminal routines, source file
\****************************************************/
#define PLRT_ENABLE_HANDLER
#include <plterm-basic.h>

typedef struct pltibuf {
	plstring_t buffer;	// Actual buffer
	size_t currentUsage;	// Current total usage of buffer
	size_t offset;		// Current position in buffer (in bytes from starting pointer)
	pltermsc_t startPos;	// Starting position
	pltermsc_t maxPos;	// Maximum position before scrolling/wrapping
	uint16_t tabWidth;	// Width of tabs. Used for printing '\t' chars as well as determining how far to move the cursor
	bool tabDeleted;	// Flag to fix render bug for tab deletion
	bool textbox;		// When wrapping, start at startPos,x instead of 0, as well as treat maxPos.y as a limiter
	bool initialized;	// Buffer has been utilized by one of the Text Input (plTermTI*) routines
} pltibuf_t;

void plTermSetBackground(plterm_t* termStruct, pltermcolor_t color){
	if(color < 40 && color > 47)
		return;

	pltermsc_t terminalSize;
	plTermGetAttrib(&terminalSize, PLTERM_SIZE, termStruct);
	plTermFillArea(termStruct, color, 1, 1, terminalSize.x, terminalSize.y);
}

void plTermPrintHeader(plterm_t* termStruct, plstring_t string, pltermcolor_t color, uint16_t y, uint16_t textOffset){
	pltermsc_t terminalSize;
	plTermGetAttrib(&terminalSize, PLTERM_SIZE, termStruct);
	plTermFillArea(termStruct, color, 1, y, terminalSize.x, y);

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

bool plTermTIIsNoise(plchar_t inputKey){
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
		case PLTERM_KEY_ENTER:
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

pltermsc_t plTermTIMakeRelative(pltermsc_t startOffset, pltermsc_t currentPos){
	currentPos.x -= (startOffset.x - 1);
	currentPos.y -= (startOffset.y - 1);
}

int16_t plTermTIDetermineTabMovLeft(pltibuf_t* bufferStruct, pltermsc_t currentPos){
	pltermsc_t textAreaSize = bufferStruct->maxPos;
	pltermsc_t relCurrentPos = currentPos;
	pltermsc_t relWorkPos = bufferStruct->startPos;
	bool loopWhenLessThanTabWidth = true;
	if(bufferStruct->textbox){
		textAreaSize = plTermTIMakeRelative(bufferStruct->startPos, bufferStruct->maxPos);
		relCurrentPos = plTermTIMakeRelative(bufferStruct->startPos, currentPos);
		relWorkPos.x = 1;
		relWorkPos.y = 1;
		loopWhenLessThanTabWidth = false;
	}

	plchar_t* bufferArray = bufferStruct->buffer.data.pointer;
	size_t bufArrayOffset = 0, workOffset = 0, currentOffset = (relCurrentPos.y * textAreaSize.x) + relCurrentPos.x;
	int16_t mvUnits;


	while(bufArrayOffset + 1 < bufferStruct->offset){
		mvUnits = 1;
		if(bufferArray[bufArrayOffset].bytes[0] == '\t')
			mvUnits = relCurrentPos.x % bufferStruct->tabWidth == 0 ? bufferStruct->tabWidth : bufferStruct->tabWidth - (relWorkPos.x % bufferStruct->tabWidth);

		relWorkPos.x += mvUnits;
		if(relWorkPos.x > textAreaSize.x){
			relWorkPos.y++;
			relWorkPos.x -= textAreaSize.x;
		}

		bufArrayOffset++;
	}

	workOffset = (relWorkPos.y * textAreaSize.x) + relWorkPos.x;
	return (int16_t)(currentOffset - workOffset);
}

int16_t plTermTIDetermineTabMovRight(pltibuf_t* bufferStruct, pltermsc_t currentPos){
	pltermsc_t textAreaSize = bufferStruct->maxPos;
	pltermsc_t relCurrentPos = currentPos;
	if(bufferStruct->textbox){
		textAreaSize = plTermTIMakeRelative(bufferStruct->startPos, bufferStruct->maxPos);
		relCurrentPos = plTermTIMakeRelative(bufferStruct->startPos, currentPos);
	}

	return relCurrentPos.x % bufferStruct->tabWidth == 0 ? (int16_t)(bufferStruct->tabWidth) : (int16_t)(bufferStruct->tabWidth - (relCurrentPos.x % bufferStruct->tabWidth));
}

void plTermTIPrintChar(plterm_t* termStruct, pltibuf_t* bufferStruct, plchar_t ch){
	pltermsc_t currentPos;
	plTermGetAttrib(&currentPos, PLTERM_POS, termStruct);
	if(ch.bytes[0] == '\t'){
		int16_t tabLength = plTermTIDetermineTabMovRight(bufferStruct, currentPos);
		if(currentPos.x + tabLength > bufferStruct->maxPos.x && currentPos.y == bufferStruct->maxPos.y)
			bufferStruct->startPos.y--;

		ch.bytes[0] = ' ';
		for(int i = 0; i < tabLength; i++)
			plTermPrintChar(termStruct, ch);
	}else{
		if(currentPos.x + 1 > bufferStruct->maxPos.x && currentPos.y == bufferStruct->maxPos.y)
			bufferStruct->startPos.y--;

		plTermPrintChar(termStruct, ch);
	}

	/*
	if(currentPos.x >= bufferStruct->maxPos.x){
		if(currentPos.y == bufferStruct->maxPos.y){
			ch.bytes[0] = ' ';
			plTermPrintChar(termStruct, ch);
			plTermMove(termStruct, 1, currentPos.y);
			bufferStruct->startPos.y--;
		}else{
			plTermMove(termStruct, 1, currentPos.y + 1);
		}
	}*/
}

pltermsc_t plTermTILeftRight(plterm_t* termStruct, pltibuf_t* bufferStruct, plchar_t inputKey, bool moveOffset){
	pltermsc_t currentPos;
	int16_t movementUnits = 1;
	uint16_t startingPoint = 1;
	if(bufferStruct->textbox)
		startingPoint = bufferStruct->startPos.x;
	plTermGetAttrib(&currentPos, PLTERM_POS, termStruct);

	if(bufferStruct->offset > 0 && inputKey.bytes[0] == PLTERM_KEY_LEFT){
		if(((plchar_t*)bufferStruct->buffer.data.pointer)[bufferStruct->offset - 1].bytes[0] == '\t' || bufferStruct->tabDeleted)
			movementUnits = plTermTIDetermineTabMovLeft(bufferStruct, currentPos);

		if(currentPos.x - startingPoint < movementUnits){
			plTermMove(termStruct, bufferStruct->maxPos.x, currentPos.y - 1);
			movementUnits -= (currentPos.x - startingPoint);
			if(movementUnits > 0)
				plTermRelMove(termStruct, -(movementUnits - 1), 0);
		}else{
			plTermRelMove(termStruct, -movementUnits, 0);
		}

		if(moveOffset)
			bufferStruct->offset--;
	}else if(bufferStruct->offset + 1 < bufferStruct->currentUsage && inputKey.bytes[0] == PLTERM_KEY_RIGHT){
		if(((plchar_t*)bufferStruct->buffer.data.pointer)[bufferStruct->offset].bytes[0] == '\t')
			movementUnits = plTermTIDetermineTabMovRight(bufferStruct, currentPos);

		if(currentPos.x + movementUnits > bufferStruct->maxPos.x)
			plTermMove(termStruct, (currentPos.x + movementUnits) - bufferStruct->maxPos.x, currentPos.y + 1);
		else
			plTermRelMove(termStruct, movementUnits, 0);

		if(moveOffset)
			bufferStruct->offset++;
	}

	plTermGetAttrib(&currentPos, PLTERM_POS, termStruct);
	return currentPos;
}

pltermsc_t plTermTIRenderAction(plterm_t* termStruct, pltibuf_t* bufferStruct, plchar_t inputKey){
	pltermsc_t currentPos;
	plTermGetAttrib(&currentPos, PLTERM_POS, termStruct);

	switch(inputKey.bytes[0]){
		case PLTERM_KEY_BACKSPACE:
			inputKey.bytes[0] = PLTERM_KEY_LEFT;
			currentPos = plTermTILeftRight(termStruct, bufferStruct, inputKey, true);
			inputKey.bytes[0] = PLTERM_KEY_BACKSPACE;
			break;
		case PLTERM_KEY_DEL:
			plptr_t tempBuf = bufferStruct->buffer.data;
			tempBuf.pointer = &((plchar_t*)bufferStruct->buffer.data.pointer)[bufferStruct->offset];
			tempBuf.size = (bufferStruct->currentUsage - bufferStruct->offset);

			for(int i = 0; i < tempBuf.size; i++)
				plTermTIPrintChar(termStruct, bufferStruct, ((plchar_t*)tempBuf.pointer)[i]);

			plchar_t clearChar = { .bytes = { ' ', '\0', '\0', '\0' } };
			for(int i = 0; i < bufferStruct->tabWidth; i++)
				plTermTIPrintChar(termStruct, bufferStruct, clearChar);

			plTermMove(termStruct, currentPos.x, currentPos.y);
			break;
		default:
			plTermTIPrintChar(termStruct, bufferStruct, inputKey);
			plTermGetAttrib(&currentPos, PLTERM_POS, termStruct);
	}

	return currentPos;
}

void plTermTIDelete(plterm_t* termStruct, pltibuf_t* bufferStruct, plchar_t inputKey){
	bool charDeleted = false;

	switch(inputKey.bytes[0]){
		case PLTERM_KEY_DEL:
			if(bufferStruct->offset + 1 < bufferStruct->currentUsage){
				if(((plchar_t*)bufferStruct->buffer.data.pointer)[bufferStruct->offset].bytes[0] == '\t')
					bufferStruct->tabDeleted = true;

				memmove(bufferStruct->buffer.data.pointer + (bufferStruct->offset * sizeof(plchar_t)), bufferStruct->buffer.data.pointer + ((bufferStruct->offset + 1) * sizeof(plchar_t)), (bufferStruct->currentUsage - bufferStruct->offset) * sizeof(plchar_t));
				charDeleted = true;
			}
			break;
		case PLTERM_KEY_BACKSPACE:
			if(bufferStruct->offset > 0){
				if(((plchar_t*)bufferStruct->buffer.data.pointer)[bufferStruct->offset - 1].bytes[0] == '\t')
					bufferStruct->tabDeleted = true;

				memmove(bufferStruct->buffer.data.pointer + ((bufferStruct->offset - 1) * sizeof(plchar_t)), bufferStruct->buffer.data.pointer + (bufferStruct->offset * sizeof(plchar_t)), (bufferStruct->currentUsage - bufferStruct->offset) * sizeof(plchar_t));
				charDeleted = true;
			}
			break;
	}

	if(charDeleted)
		(bufferStruct->currentUsage)--;
}

void plTermTIInsert(plterm_t* termStruct, pltibuf_t* bufferStruct, plchar_t inputKey){
	if(bufferStruct->offset + 1 < bufferStruct->currentUsage)
		memmove(bufferStruct->buffer.data.pointer + ((bufferStruct->offset + 1) * sizeof(plchar_t)), bufferStruct->buffer.data.pointer + (bufferStruct->offset * sizeof(plchar_t)), (bufferStruct->currentUsage - bufferStruct->offset) * sizeof(plchar_t));

	((plchar_t*)bufferStruct->buffer.data.pointer)[bufferStruct->offset] = inputKey;
	(bufferStruct->currentUsage)++;
	(bufferStruct->offset)++;
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
		plTermGetAttrib(&bufferStruct->maxPos, PLTERM_SIZE, termStruct);
		bufferStruct->initialized = true;
		bufferStruct->tabWidth = 8;
		bufferStruct->textbox = false;
	}

	plTermGetAttrib(&currentPos, PLTERM_POS, termStruct);
	plTermGetAttrib(&mt, PLTERM_MT, termStruct);
	plchar_t inputKey = plTermGetInput(termStruct);

	if(inputKey.bytes[0] == PLTERM_KEY_LEFT || inputKey.bytes[0] == PLTERM_KEY_RIGHT){
		currentPos = plTermTILeftRight(termStruct, bufferStruct, inputKey, true);
	}else if((inputKey.bytes[0] == PLTERM_KEY_BACKSPACE || inputKey.bytes[0] == PLTERM_KEY_DEL) && bufferStruct->currentUsage > 1){
		plTermTIDelete(termStruct, bufferStruct, inputKey);
		if(inputKey.bytes[0] == PLTERM_KEY_BACKSPACE && oldUsage != bufferStruct->currentUsage)
			currentPos = plTermTIRenderAction(termStruct, bufferStruct, inputKey);
	}else if(!plTermTIIsNoise(inputKey) && bufferStruct->currentUsage < bufferStruct->buffer.data.size){
		plTermTIInsert(termStruct, bufferStruct, inputKey);
		currentPos = plTermTIRenderAction(termStruct, bufferStruct, inputKey);
	}

	if((!plTermTIIsNoise(inputKey) || inputKey.bytes[0] == PLTERM_KEY_BACKSPACE) && (oldUsage > bufferStruct->currentUsage || (oldUsage < bufferStruct->currentUsage && bufferStruct->offset + 1 < bufferStruct->currentUsage))){
		uint8_t keyHolder = inputKey.bytes[0];
		inputKey.bytes[0] = PLTERM_KEY_DEL;
		plTermTIRenderAction(termStruct, bufferStruct, inputKey);
		inputKey.bytes[0] = keyHolder;
	}

	bufferStruct->tabDeleted = false;
	return inputKey;
}
