/* $Id: smartIndent.h,v 1.4 2001/08/09 18:03:10 slobasso Exp $ */
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
