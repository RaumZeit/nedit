/* $Id: selection.h,v 1.5 2002/07/11 21:18:10 slobasso Exp $ */

#ifndef NEDIT_SELECTION_H_INCLUDED
#define NEDIT_SELECTION_H_INCLUDED

#include "nedit.h"

#include <X11/Intrinsic.h>
#include <X11/X.h>

int StringToLineAndCol(const char *text, int *lineNum, int *column );
void GotoSelectedLineNumber(WindowInfo *window, Time time);
void GotoLineNumber(WindowInfo *window);
void SelectNumberedLine(WindowInfo *window, int lineNum);
void OpenSelectedFile(WindowInfo *window, Time time);
char *GetAnySelection(WindowInfo *window);
void BeginMarkCommand(WindowInfo *window);
void BeginGotoMarkCommand(WindowInfo *window, int extend);
void AddMark(WindowInfo *window, Widget widget, char label);
void UpdateMarkTable(WindowInfo *window, int pos, int nInserted,
   	int nDeleted);
void GotoMark(WindowInfo *window, Widget w, char label, int extendSel);
void MarkDialog(WindowInfo *window);
void GotoMarkDialog(WindowInfo *window, int extend);

#endif /* NEDIT_SELECTION_H_INCLUDED */
