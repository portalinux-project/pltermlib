/**************************************************************\
 pltermlib, v1.00
 (c) 2023 pocketlinux32, Under MPL v2.0
 plterm-basic.h: Basic routines that don't fit in plterm-base.h
\**************************************************************/

#include <plterm-base.h>

void plTermSetBackground(plterm_t* termStruct, pltermcolor_t color);
void plTermPrintHeader(plterm_t* termStruct, plstring_t string, pltermcolor_t color, uint16_t y, uint16_t textOffset);
void plTermTextInput(plterm_t* termStruct, plptr_t* buffer, plchar_t endKey);
