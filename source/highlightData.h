/* $Id: highlightData.h,v 1.11 2003/05/09 17:43:45 edg Exp $ */

#ifndef NEDIT_HIGHLIGHTDATA_H_INCLUDED
#define NEDIT_HIGHLIGHTDATA_H_INCLUDED

#include "nedit.h"
#include "highlight.h"

#include <X11/Intrinsic.h>
#include <X11/Xlib.h>

patternSet *FindPatternSet(const char *langModeName);
int LoadHighlightString(char *inString, int convertOld);
char *WriteHighlightString(void);
int LoadStylesString(char *inString);
char *WriteStylesString(void);
void EditHighlightStyles(const char *initialStyle);
void EditHighlightPatterns(WindowInfo *window);
void UpdateLanguageModeMenu(void);
int LMHasHighlightPatterns(const char *languageMode);
XFontStruct *FontOfNamedStyle(WindowInfo *window, const char *styleName);
int FontOfNamedStyleIsBold(char *styleName);
int FontOfNamedStyleIsItalic(char *styleName);
char *ColorOfNamedStyle(const char *styleName);
char *BgColorOfNamedStyle(const char *styleName);
int IndexOfNamedStyle(const char *styleName);
int NamedStyleExists(const char *styleName);
void RenameHighlightPattern(const char *oldName, const char *newName);

#endif /* NEDIT_HIGHLIGHTDATA_H_INCLUDED */
