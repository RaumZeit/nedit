/* $Id: misc.h,v 1.7 2001/04/13 17:50:50 tringali Exp $ */
#define TEXT_READ_OK 0
#define TEXT_IS_BLANK 1
#define TEXT_NOT_NUMBER 2

/* maximum length for a window geometry string */
#define MAX_GEOM_STRING_LEN 24

void AddMotifCloseCallback(Widget shell, XtCallbackProc closeCB, void *arg);
void SuppressPassiveGrabWarnings(void);
void PopDownBugPatch(Widget w);
void RemapDeleteKey(Widget w);
void SetDeleteRemap(int state);
void RealizeWithoutForcingPosition(Widget shell);
void ManageDialogCenteredOnPointer(Widget dialogChild);
void SetPointerCenteredDialogs(int state);
void RaiseShellWindow(Widget shell);
void RaiseWindow(Display *display, Window w);
void AddDialogMnemonicHandler(Widget dialog, int unmodifiedToo);
void RemoveDialogMnemonicHandler(Widget dialog);
void AccelLockBugPatch(Widget topWidget, Widget topMenuContainer);
void UpdateAccelLockPatch(Widget topWidget, Widget newButton);
char *GetXmStringText(XmString fromString);
XFontStruct *GetDefaultFontStruct(XmFontList font);
XmString* StringTable(int count, ...);
void FreeStringTable(XmString *table);
void SimulateButtonPress(Widget widget);
Widget AddMenuItem(Widget parent, char *name, char *label, char mnemonic,
	char *acc, char *accText, XtCallbackProc callback, void *cbArg);
Widget AddMenuToggle(Widget parent, char *name, char *label, char mnemonic,
	char *acc, char *accText, XtCallbackProc callback, void *cbArg,int set);
Widget AddMenuSeparator(Widget parent, char *name);
Widget AddSubMenu(Widget parent, char *name, char *label, char mnemonic);
void SetIntLabel(Widget label, int value);
void SetFloatLabel(Widget label, double value);
void SetIntText(Widget text, int value);
void SetFloatText(Widget text, double value);
int GetFloatText(Widget text, double *value);
int GetIntText(Widget text, int *value);
int GetFloatTextWarn(Widget text, double *value, char *fieldName,int warnBlank);
int GetIntTextWarn(Widget text, int *value, char *fieldName, int warnBlank);
int TextWidgetIsBlank(Widget textW);
void MakeSingleLineTextW(Widget textW);
void BeginWait(Widget topCursorWidget);
void EndWait(Widget topCursorWidget);
void PasswordText(Widget w, char *passTxt);
void AddHistoryToTextWidget(Widget textW, char ***historyList, int *nItems);
void AddToHistoryList(char *newItem, char ***historyList, int *nItems);
void CreateGeometryString(char *string, short x, short y,
	short width, short height, int bitmask);
void FindBestVisual(Display *display, const char *appName, char *appClass,
	Visual **visual, int *depth, Colormap *colormap);
Widget CreateDialogShell(Widget parent, char *name, ArgList arglist,
	Cardinal  argcount);
Widget CreatePopupMenu(Widget parent, char *name, ArgList arglist,
	Cardinal argcount);
Widget CreatePulldownMenu(Widget parent, char *name, ArgList arglist,
	Cardinal  argcount);
Widget CreatePromptDialog(Widget parent, char *name, ArgList arglist,
	Cardinal  argcount);
Widget CreateSelectionDialog(Widget parent, char *name, ArgList arglist,
	Cardinal  argcount);
Widget CreateFormDialog(Widget parent, char *name, ArgList arglist,
    	Cardinal  argcount);
Widget CreateFileSelectionDialog(Widget parent, char *name, ArgList arglist,
	Cardinal  argcount);
Widget CreateQuestionDialog(Widget parent, char *name, ArgList arglist,
	Cardinal  argcount);
Widget CreateMessageDialog(Widget parent, char *name, ArgList arglist,
	Cardinal  argcount);
Widget CreateErrorDialog(Widget parent, char *name, ArgList arglist,
	Cardinal  argcount);
Widget CreateShellWithBestVis(String appName, String appClass, 
	WidgetClass class, Display *display, ArgList args, Cardinal nArgs);
