/**************************************************************\
 pltermlib, v1.00
 (c) 2023 pocketlinux32, Under MPL v2.0
 plterm-basic.h: Basic routines that don't fit in plterm-base.h
\**************************************************************/

#include <plterm-core.h>

typedef struct pltibuf pltibuf_t;

void plTermSetBackground(plterm_t* termStruct, pltermcolor_t color);
void plTermPrintHeader(plterm_t* termStruct, plstring_t string, pltermcolor_t color, uint16_t y, uint16_t textOffset);
pltibuf_t* plTermCreateTextBuffer(plmt_t* mt, size_t bufferSize, bool usePLChar);
pltibuf_t* plTermPLStringToTextBuffer(plmt_t* mt, plstring_t string);
plstring_t* plTermGetRawBuffer(pltibuf_t* bufferStruct);
plchar_t plTermReadline(plterm_t* termStruct, pltibuf_t* buffer, plstring_t prompt);
