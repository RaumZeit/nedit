/* $Id: smartIndent.h,v 1.6 2003/03/21 18:22:14 edg Exp $ */

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
int LMHasSmartIndentMacros(const char *languageMode);
void RenameSmartIndentMacros(const char *oldName, const char *newName);
void UpdateLangModeMenuSmartIndent(void);

#endif /* NEDIT_SMARTINDENT_H_INCLUDED */
