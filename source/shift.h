/* $Id: shift.h,v 1.4 2002/07/11 21:18:10 slobasso Exp $ */

#ifndef NEDIT_SHIFT_H_INCLUDED
#define NEDIT_SHIFT_H_INCLUDED

#include "nedit.h"

enum ShiftDirection {SHIFT_LEFT, SHIFT_RIGHT};

void ShiftSelection(WindowInfo *window, int direction, int byTab);
void UpcaseSelection(WindowInfo *window);
void DowncaseSelection(WindowInfo *window);
void FillSelection(WindowInfo *window);
char *ShiftText(char *text, int direction, int tabsAllowed, int tabDist,
	int nChars, int *newLen);

#endif /* NEDIT_SHIFT_H_INCLUDED */
