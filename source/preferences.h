/*******************************************************************************
*									       *
* preferences.h -- Nirvana Editor preferences processing		       *
*									       *
* Copyright (c) 1991 Universities Research Association, Inc.		       *
* All rights reserved.							       *
* 									       *
* This material resulted from work developed under a Government Contract and   *
* is subject to the following license:  The Government retains a paid-up,      *
* nonexclusive, irrevocable worldwide license to reproduce, prepare derivative *
* works, perform publicly and display publicly by or for the Government,       *
* including the right to distribute to other Government contractors.  Neither  *
* the United States nor the United States Department of Energy, nor any of     *
* their employees, makes any warrenty, express or implied, or assumes any      *
* legal liability or responsibility for the accuracy, completeness, or         *
* usefulness of any information, apparatus, product, or process disclosed, or  *
* represents that its use would not infringe privately owned rights.           *
*                                        				       *
* Fermilab Nirvana GUI Library						       *
* April 20, 1993							       *
*									       *
* Written by Mark Edel							       *
*									       *
*******************************************************************************/

#define PLAIN_LANGUAGE_MODE -1

/* maximum number of language modes allowed */
#define MAX_LANGUAGE_MODES 40

/* max length of an accelerator string */
#define MAX_ACCEL_LEN 50

XrmDatabase CreateNEditPrefDB(int *argcInOut, char **argvInOut);
void RestoreNEditPrefs(XrmDatabase prefDB, XrmDatabase appDB);
void SaveNEditPrefs(Widget parent, int quietly);
void ImportPrefFile(char *filename);
void MarkPrefsChanged(void);
int CheckPrefsChangesSaved(Widget dialogParent);
void SetPrefWrap(int state);
int GetPrefWrap(int langMode);
void SetPrefWrapMargin(int margin);
int GetPrefWrapMargin(void);
void SetPrefSearchDlogs(int state);
int GetPrefSearchDlogs(void);
void SetPrefKeepSearchDlogs(int state);
int GetPrefKeepSearchDlogs(void);
void SetPrefStatsLine(int state);
int GetPrefStatsLine(void);
void SetPrefSearch(int searchType);
int GetPrefSearch(void);
void SetPrefAutoIndent(int state);
int GetPrefAutoIndent(int langMode);
void SetPrefAutoSave(int state);
int GetPrefAutoSave(void);
void SetPrefSaveOldVersion(int state);
int GetPrefSaveOldVersion(void);
void SetPrefRows(int nRows);
int GetPrefRows(void);
void SetPrefCols(int nCols);
int GetPrefCols(void);
void SetPrefTabDist(int tabDist);
int GetPrefTabDist(int langMode);
void SetPrefEmTabDist(int tabDist);
int GetPrefEmTabDist(int langMode);
void SetPrefInsertTabs(int state);
int GetPrefInsertTabs(void);
void SetPrefShowMatching(int state);
int GetPrefShowMatching(void);
void SetPrefHighlightSyntax(int state);
int GetPrefHighlightSyntax(void);
void SetPrefRepositionDialogs(int state);
int GetPrefRepositionDialogs(void);
void SetPrefTagFile(char *tagFileName);
char *GetPrefTagFile(void);
void SetPrefFont(char *fontName);
void SetPrefBoldFont(char *fontName);
void SetPrefItalicFont(char *fontName);
void SetPrefBoldItalicFont(char *fontName);
char *GetPrefFontName(void);
char *GetPrefBoldFontName(void);
char *GetPrefItalicFontName(void);
char *GetPrefBoldItalicFontName(void);
XmFontList GetPrefFontList(void);
XFontStruct *GetPrefBoldFont(void);
XFontStruct *GetPrefItalicFont(void);
XFontStruct *GetPrefBoldItalicFont(void);
void SetPrefShell(char *shell);
char *GetPrefShell(void);
char *GetPrefServerName(void);
char *GetPrefBGMenuBtn(void);
void RowColumnPrefDialog(Widget parent);
void TabsPrefDialog(Widget parent, WindowInfo *forWindow);
void WrapMarginDialog(Widget parent, WindowInfo *forWindow);
void SetPrefMapDelete(int state);
int GetPrefMapDelete(void);
void SetPrefStdOpenDialog(int state);
int GetPrefStdOpenDialog(void);
char *GetPrefDelimiters(void);
int GetPrefMaxPrevOpenFiles(void);
#ifdef SGI_CUSTOM
void SetPrefShortMenus(int state);
int GetPrefShortMenus(void);
#endif
void EditLanguageModes(Widget parent);
void ChooseFonts(WindowInfo *window, int forWindow);
char *LanguageModeName(int mode);
char *GetWindowDelimiters(WindowInfo *window);
int ReadNumericField(char **inPtr, int *value);
char *ReadSymbolicField(char **inPtr);
char *ReadSymbolicFieldTextWidget(Widget textW, char *fieldName, int silent);
int ReadQuotedString(char **inPtr, char **errMsg, char **string);
char *MakeQuotedString(char *string);
char *EscapeSensitiveChars(char *string);
int SkipDelimiter(char **inPtr, char **errMsg);
int ParseError(Widget toDialog, char *stringStart, char *stoppedAt,
	char *errorIn, char *message);
char *CopyAllocatedString(char *string);
int AllocatedStringsDiffer(char *s1, char *s2);
void DetermineLanguageMode(WindowInfo *window, int forceNewDefaults);
Widget CreateLanguageModeMenu(Widget parent, XtCallbackProc cbProc,
	void *cbArg);
void SetLangModeMenu(Widget optMenu, char *modeName);
Widget CreateLanguageModeSubMenu(WindowInfo *window, Widget parent, char *name,
    	char *label, char mnemonic);
