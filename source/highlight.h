/* $Id: highlight.h,v 1.7 2002/09/26 12:37:39 ajhood Exp $ */

#ifndef NEDIT_HIGHLIGHT_H_INCLUDED
#define NEDIT_HIGHLIGHT_H_INCLUDED

#include "nedit.h"

#include <X11/Intrinsic.h>

/* Pattern flags for modifying pattern matching behavior */
#define PARSE_SUBPATS_FROM_START 1
#define DEFER_PARSING 2
#define COLOR_ONLY 4

/* Pattern specification structure */
typedef struct {
    char *name;
    char *startRE;
    char *endRE;
    char *errorRE;
    char *style;
    char *subPatternOf;
    int flags;
} highlightPattern;

/* Header for a set of patterns */
typedef struct {
    char *languageMode;
    int lineContext;
    int charContext;
    int nPatterns;
    highlightPattern *patterns;
} patternSet;

void SyntaxHighlightModifyCB(int pos, int nInserted, int nDeleted,
    	int nRestyled, char *deletedText, void *cbArg);
void StartHighlighting(WindowInfo *window, int warn);
void StopHighlighting(WindowInfo *window);
void AttachHighlightToWidget(Widget widget, WindowInfo *window);
void FreeHighlightingData(WindowInfo *window);
void RemoveWidgetHighlight(Widget widget);
void UpdateHighlightStyles(WindowInfo *window);
int TestHighlightPatterns(patternSet *patSet);
Pixel AllocColor(Widget w, const char *colorName, int *r, int *g, int *b);
void* GetHighlightInfo(WindowInfo *window, int pos);
highlightPattern *FindPatternOfWindow(WindowInfo *window, char *name);
int HighlightCodeOfPos(WindowInfo *window, int pos);
int HighlightLengthOfCodeFromPos(WindowInfo *window, int pos, int *checkCode);
char *HighlightNameOfCode(WindowInfo *window, int hCode);
char *HighlightStyleOfCode(WindowInfo *window, int hCode);
char *HighlightColorOfCode(WindowInfo *window, int hCode);
Pixel HighlightColorValueOfCode(WindowInfo *window, int hCode,
      int *r, int *g, int *b);
int HighlightCodeIsBold(WindowInfo *window, int hCode);
int HighlightCodeIsItalic(WindowInfo *window, int hCode);

#endif /* NEDIT_HIGHLIGHT_H_INCLUDED */
