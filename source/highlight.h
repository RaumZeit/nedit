/*******************************************************************************
*									       *
* highlight.h -- Nirvana Editor syntax highlighting (text coloring and font    *
*   	    	 selected by file content				       *
*									       *
* Copyright (c) 1996 Universities Research Association, Inc.		       *
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
* June 24, 1996								       *
*									       *
* Written by Mark Edel							       *
*									       *
*******************************************************************************/

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
