/* $Id: smartIndent.h,v 1.5 2002/07/11 21:18:10 slobasso Exp $ */

#ifndef NEDIT_SMARTINDENT_H_INCLUDED
#define NEDIT_SMARTINDENT_H_INCLUDED

#include "nedit.h"

#include <X11/Intrinsic.h>

void BeginSmartIndent(WindowInfo *window, int warn);
void EndSmartIndent(WindowInfo *window);
void SmartIndentCB(Widget w, XtPointer clientData, XtPointer callData);
int LoadSmartIndentString(char *inString);
int LoadSmartIndentCommonString(char *inString);
char *WriteSmartIndentString(void);
char *WriteSmartIndentCommonString(void);
int SmartIndentMacrosAvailable(char *languageMode);
void EditSmartIndentMacros(WindowInfo *window);
void EditCommonSmartIndentMacro(void);
Boolean InSmartIndentMacros(WindowInfo *window);

#endif /* NEDIT_SMARTINDENT_H_INCLUDED */
