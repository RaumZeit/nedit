#define PLAIN_LANGUAGE_MODE -1

/* maximum number of language modes allowed */
#define MAX_LANGUAGE_MODES 127

/* max length of an accelerator string */
#define MAX_ACCEL_LEN 50

XrmDatabase CreateNEditPrefDB(int *argcInOut, char **argvInOut);
void RestoreNEditPrefs(XrmDatabase prefDB, XrmDatabase appDB);
void SaveNEditPrefs(Widget parent, int quietly);
void ImportPrefFile(char *filename, int convertOld);
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
void SetPrefISearchLine(int state);
int GetPrefISearchLine(void);
void SetPrefLineNums(int state);
int GetPrefLineNums(void);
void SetPrefWarnFileMods(int state);
int GetPrefWarnFileMods(void);
void SetPrefWarnExit(int state);
int GetPrefWarnExit(void);
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
void SetPrefSortOpenPrevMenu(int state);
int GetPrefSortOpenPrevMenu(void);
void SetPrefTagFile(char *tagFileName);
char *GetPrefTagFile(void);
int GetPrefSmartTags(void);
void SetPrefSmartTags(int state);
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
void SetPrefGeometry(char *geometry);
char *GetPrefGeometry(void);
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
void SetLanguageMode(WindowInfo *window, int mode, int forceNewDefaults);
int FindLanguageMode(char *languageName);
void DetermineLanguageMode(WindowInfo *window, int forceNewDefaults);
Widget CreateLanguageModeMenu(Widget parent, XtCallbackProc cbProc,
	void *cbArg);
void SetLangModeMenu(Widget optMenu, char *modeName);
Widget CreateLanguageModeSubMenu(WindowInfo *window, Widget parent, char *name,
    	char *label, char mnemonic);
