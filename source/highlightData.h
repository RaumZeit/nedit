/* $Id: highlightData.h,v 1.6 2001/08/25 15:58:54 amai Exp $ */
patternSet *FindPatternSet(const char *langModeName);
int LoadHighlightString(char *inString, int convertOld);
char *WriteHighlightString(void);
int LoadStylesString(char *inString);
char *WriteStylesString(void);
void EditHighlightStyles(Widget parent, const char *initialStyle);
void EditHighlightPatterns(WindowInfo *window);
void UpdateLanguageModeMenu(void);
int LMHasHighlightPatterns(const char *languageMode);
XFontStruct *FontOfNamedStyle(WindowInfo *window, const char *styleName);
char *ColorOfNamedStyle(const char *styleName);
int NamedStyleExists(const char *styleName);
void RenameHighlightPattern(const char *oldName, const char *newName);
