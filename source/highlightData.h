/* $Id: highlightData.h,v 1.3 2001/02/26 23:38:03 edg Exp $ */
patternSet *FindPatternSet(char *langModeName);
int LoadHighlightString(char *inString, int convertOld);
char *WriteHighlightString(void);
int LoadStylesString(char *inString);
char *WriteStylesString(void);
void EditHighlightStyles(Widget parent, char *initialStyle);
void EditHighlightPatterns(WindowInfo *window);
void UpdateLanguageModeMenu(void);
int LMHasHighlightPatterns(char *languageMode);
XFontStruct *FontOfNamedStyle(WindowInfo *window, char *styleName);
char *ColorOfNamedStyle(char *styleName);
int NamedStyleExists(char *styleName);
void RenameHighlightPattern(char *oldName, char *newName);
