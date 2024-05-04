#include <plterm-base.h>

struct plterm {
	struct termios original;
	struct termios current;
	pltermsc_t size;
	pltermsc_t pos;
	bool displayCursor;
	plmt_t* mt;
};

void plTermGetAttrib(memptr_t buf, pltermaction_t attrib, plterm_t* termStruct){
	switch(attrib){
		case PLTERM_SIZE:
			*((pltermsc_t*)buf) = termStruct->size;
			break;
		case PLTERM_POS:
			*((pltermsc_t*)buf) = termStruct->pos;
			break;
		case PLTERM_MT:
			*((plmt_t**)buf) = termStruct->mt;
			break;
	}
}

void plTermToggleCursor(plterm_t* termStruct){
	char commandConstant[6] = "\x1b[?25";
	if(termStruct->displayCursor){
		commandConstant[5] = 'l';
		termStruct->displayCursor = false;
	}else{
		commandConstant[5] = 'h';
		termStruct->displayCursor = true;
	}

	write(STDOUT_FILENO, commandConstant, 6);
}

void plTermClearScreen(plterm_t* termStruct){
	termStruct->pos.x = 1;
	termStruct->pos.y = 1;
	write(STDOUT_FILENO, "\x1b[2J\x1b[1;1H", 10);
}

pltermsc_t plTermGetPosition(plterm_t* termStruct){
	char tempBuf[16] = "";
	pltermsc_t retPos;
	write(STDOUT_FILENO, "\x1b[6n\0", 5);
	ssize_t offset = read(STDIN_FILENO, tempBuf, 16);
	if(offset < 0){
		tcsetattr(STDOUT_FILENO, 0, &(termStruct->original));
		write(STDOUT_FILENO, "\r", 1);
		perror("plTermGetPosition");
		abort();
	}

	char* startPos = tempBuf + 2;
	char* midPos = strchr(startPos, ';');
	char* endPos = strchr(startPos, 'R');
	char secondTempBuf[5];
	char* junk;

	memcpy(secondTempBuf, startPos, midPos - startPos);
	secondTempBuf[midPos - startPos + 1] = '\0';
	retPos.y = strtol(secondTempBuf, &junk, 10);

	memcpy(secondTempBuf, midPos + 1, endPos - midPos);
	secondTempBuf[endPos - (midPos + 1)] = '\0';
	retPos.x = strtol(secondTempBuf, &junk, 10);

	return retPos;
}

void plTermUpdateSize(plterm_t* termStruct){
	char tempBuf[16] = "";
	write(STDOUT_FILENO, "\x1b[9999;9999H", 12);
	termStruct->size = plTermGetPosition(termStruct);
	snprintf(tempBuf, 16, "\x1b[%d;%dH", termStruct->pos.y, termStruct->pos.x);
	write(STDOUT_FILENO, tempBuf, 16);
}

void plTermInputDriver(plchar_t* characterBuffer){
	if(characterBuffer->bytes[0] == PLTERM_KEY_ESCAPE && characterBuffer->bytes[1] != 0){
		switch(characterBuffer->bytes[2]){
			case 'A':
				characterBuffer->bytes[0] = PLTERM_KEY_UP;
				break;
			case 'B':
				characterBuffer->bytes[0] = PLTERM_KEY_DOWN;
				break;
			case 'C':
				characterBuffer->bytes[0] = PLTERM_KEY_RIGHT;
				break;
			case 'D':
				characterBuffer->bytes[0] = PLTERM_KEY_LEFT;
				break;
		}
		for(int i = 1; i < 4; i++)
			characterBuffer->bytes[i] = 0;
	}
}

plchar_t plTermGetInput(plterm_t* termStruct){
	plchar_t retVal = {
			.bytes = {0, 0, 0, 0}
	};
	ssize_t offset = 0;

	offset = read(STDIN_FILENO, retVal.bytes, 4);
	if(offset == 0)
		return retVal;

	plTermInputDriver(&retVal);
	return retVal;
}

void plTermMove(plterm_t* termStruct, uint16_t x, uint16_t y){
	char tempStr[16] = "";
	snprintf(tempStr, 16, "\x1b[%d;%dH", y, x);
	write(STDOUT_FILENO, tempStr, strlen(tempStr));
	if(x == 0)
		termStruct->pos.x = 1;
	else
		termStruct->pos.x = x;

	if(y == 0)
		termStruct->pos.y = 1;
	else
		termStruct->pos.y = y;
}

void plTermRelMove(plterm_t* termStruct, int x, int y){
	char tempStr[8] = "";

	if(x != 0){
		if(x < 0)
			snprintf(tempStr, 8, "\x1b[%dD", -x);
		else
			snprintf(tempStr, 8, "\x1b[%dC", x);
		write(STDOUT_FILENO, tempStr, strlen(tempStr));
	}

	if(y != 0){
		if(y < 0)
			snprintf(tempStr, 8, "\x1b[%dA", -y);
		else
			snprintf(tempStr, 8, "\x1b[%dB", y);
		write(STDOUT_FILENO, tempStr, strlen(tempStr));
	}

	termStruct->pos = plTermGetPosition(termStruct);
}

int plTermChangeColor(pltermcolor_t color){
	bool forecolorOutOfRange = color < 30 || color > 37;
	bool backcolorOutOfRange = color < 40 || color > 47;
	bool miscFontStyleOutOfRange = color > 7;

	if(forecolorOutOfRange && backcolorOutOfRange && miscFontStyleOutOfRange)
		return 1;

	char colorStr[6];
	snprintf(colorStr, 6, "\x1b[%dm", color);
	write(STDOUT_FILENO, colorStr, strlen(colorStr));
	return 0;
}

void plTermPrint(plterm_t* termStruct, plstring_t string){
	if(string.data.pointer == NULL)
		plRTPanic("plTermPrint", PLRT_ERROR | PLRT_NULL_PTR, true);
	if(string.isplChar)
		plRTPanic("plTermPrint", PLRT_ERROR | PLRT_NOT_COMPRESSED, true);

	if(string.data.size == 0)
		return;

	write(STDOUT_FILENO, string.data.pointer, string.data.size);

	plchar_t newline = { .bytes = { '\n', '\0', '\0', '\0' } };
	if(plRTStrchr(string, newline, 0) == -1){
		termStruct->pos.x += string.data.size;
		if(termStruct->pos.x > termStruct->size.x){
			termStruct->pos.x -= termStruct->size.x;
			termStruct->pos.y++;
		}
	}else{
		termStruct->pos = plTermGetPosition(termStruct);
	}
}

void plTermMovePrint(plterm_t* termStruct, uint16_t x, uint16_t y, plstring_t string){
	plTermMove(termStruct, x, y);
	plTermPrint(termStruct, string);
}

void plTermFillArea(plterm_t* termStruct, pltermcolor_t color, uint16_t xStart, uint16_t yStart, uint16_t xStop, uint16_t yStop){
	size_t termPos[2] = {termStruct->pos.x, termStruct->pos.y};

	if(xStart > termStruct->size.x)
		xStart = termStruct->size.x - 1;
	if(yStart > termStruct->size.y)
		yStart = termStruct->size.y - 1;
	if(xStop > termStruct->size.x)
		xStop = termStruct->size.x;
	if(yStop > termStruct->size.y)
		yStop = termStruct->size.y;

	plTermChangeColor(color);
	plTermMove(termStruct, xStart, yStart);
	while(yStart <= yStop){
		while(termStruct->pos.x < xStop)
			plTermPrint(termStruct, plRTStrFromCStr(" ", NULL));
		plTermPrint(termStruct, plRTStrFromCStr(" ", NULL));

		if(termStruct->pos.y == yStart)
			plTermMove(termStruct, xStart, termStruct->pos.y + 1);
		yStart++;
	}

	plTermMove(termStruct, termPos[0], termPos[1]);
}

plterm_t* plTermInit(plmt_t* mt, bool nonblockInput){
	plterm_t* retStruct = plMTAlloc(mt, sizeof(plterm_t));
	struct termios* og = &(retStruct->original);
	struct termios* cur = &(retStruct->current);

	tcgetattr(STDIN_FILENO, og);

	cur->c_oflag |= ONLCR;
	cur->c_iflag |= ICRNL;
	cur->c_cflag |= CS8;
	cur->c_cc[VMIN] = 1;
	cur->c_cc[VTIME] = 0;
	tcsetattr(STDIN_FILENO, TCSANOW, cur);
	if(nonblockInput)
		fcntl(STDIN_FILENO, F_SETFL, fcntl(STDIN_FILENO, F_GETFL) | O_NONBLOCK);

	retStruct->mt = mt;
	retStruct->pos = plTermGetPosition(retStruct);
	retStruct->displayCursor = true;
	plTermUpdateSize(retStruct);
	return retStruct;
}

void plTermStop(plterm_t* termStruct){
	plTermChangeColor(PLTERM_FONT_DEFAULT);
	write(STDOUT_FILENO, "\x1b[?25h", 6);
	plTermMove(termStruct, 1, termStruct->pos.y);
	tcsetattr(STDIN_FILENO, 0, &(termStruct->original));
	tcsetattr(STDOUT_FILENO, 0, &(termStruct->original));

	plMTFree(termStruct->mt, termStruct);
}
