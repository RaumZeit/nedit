/* $Id: highlightData.h,v 1.5 2001/08/25 15:24:52 amai Exp $ */
patternSet *FindPatternSet(const char *langModeName);
int LoadHighlightString(char *inString, int convertOld);
char *WriteHighlightString(void);
int LoadStylesString(char *inString);
char *WriteStylesString(void);
void EditHighlightStyles(Widget parent, const char *initialStyle);
void EditHighlightPatterns(WindowInfo *window);
void UpdateLanguageModeMenu(void);
int LMHasHighlightPatterns(char *languageMode);
XFontStruct *FontOfNamedStyle(WindowInfo *window, const char *styleName);
char *ColorOfNamedStyle(const char *styleName);
int NamedStyleExists(const char *styleName);
void RenameHighlightPattern(const char *oldName, const char *newName);
