/* $Id: highlight.h,v 1.3 2001/02/26 23:38:03 edg Exp $ */
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
