/*******************************************************************************
*									       *
* textDisp.h - Display text from a text buffer				       *
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
* June 15, 1995								       *
*									       *
* Written by Mark Edel							       *
*									       *
*******************************************************************************/
enum cursorStyles {NORMAL_CURSOR, CARET_CURSOR, DIM_CURSOR, BLOCK_CURSOR,
	HEAVY_CURSOR};

#define NO_HINT -1

typedef struct {
    Pixel color;
    XFontStruct *font;
} styleTableEntry;

typedef void (*unfinishedStyleCBProc)();

typedef struct _textDisp {
    Widget w;
    int top, left, width, height;
    int cursorPos;
    int cursorOn;
    int cursorX, cursorY;		/* X, Y pos. of cursor for blanking */
    int cursorToHint;			/* Tells the buffer modified callback
    					   where to move the cursor, to reduce
    					   the number of redraw calls */
    int cursorStyle;			/* One of enum cursorStyles above */
    int cursorPreferredCol;		/* Column for vert. cursor movement */
    int nVisibleLines;			/* # of visible (displayed) lines */
    int nBufferLines;			/* # of newlines in the buffer */
    textBuffer *buffer;     	    	/* Contains text to be displayed */
    textBuffer *styleBuffer;   	    	/* Optional parallel buffer containing
    	    	    	    	    	   color and font information */
    int firstChar, lastChar;		/* Buffer positions of first and last
    					   displayed character (lastChar points
    					   either to a newline or one character
    					   beyond the end of the buffer) */
    int continuousWrap;     	    	/* Wrap long lines when displaying */
    int wrapMargin; 	    	    	/* Margin in # of char positions for
    	    	    	    	    	   wrapping in continuousWrap mode */
    int *lineStarts;
    int topLineNum;			/* Line number of top displayed line
    					   of file (first line of file is 1) */
    int horizOffset;			/* Horizontal scroll pos. in pixels */
    int visibility;			/* Window visibility (see XVisibility
    					   event) */
    int nStyles;			/* Number of entries in styleTable */
    styleTableEntry *styleTable;    	/* Table of fonts and colors for
    	    	    	    	    	   coloring/syntax-highlighting */
    char unfinishedStyle;   	    	/* Style buffer entry which triggers
    	    	    	    	    	   on-the-fly reparsing of region */
    unfinishedStyleCBProc		/* Callback to parse "unfinished" */
    	    unfinishedHighlightCB;  	/*     regions */
    void *highlightCBArg;   	    	/* Arg to unfinishedHighlightCB */
    XFontStruct *fontStruct;		/* Font structure for primary font */
    int ascent, descent;		/* Composite ascent and descent for
    					   primary font + all-highlight fonts */
    int fixedFontWidth;			/* Font width if all current fonts are
    					   fixed and match in width, else -1 */
    Widget hScrollBar, vScrollBar;
    Pixel bgPixel, selectBGPixel;   	/* Background colors */
    Pixel highlightBGPixel;
    GC gc, selectGC, highlightGC;	/* GCs for drawing text */
    GC selectBGGC, highlightBGGC;	/* GCs for erasing text */
    GC cursorFGGC;			/* GC for drawing the cursor */
    GC styleGC;     	    	    	/* GC with color and font unspecified
    	    	    	    	    	   for drawing colored/styled text */
} textDisp;

textDisp *TextDCreate(Widget widget, Widget hScrollBar, Widget vScrollBar,
	Position left, Position top, Position width, Position height,
	textBuffer *buffer, XFontStruct *fontStruct, Pixel bgPixel,
	Pixel fgPixel, Pixel selectFGPixel, Pixel selectBGPixel,
	Pixel highlightFGPixel, Pixel highlightBGPixel, Pixel cursorFGPixel,
	int continuousWrap, int wrapMargin);
void TextDFree(textDisp *textD);
void TextDSetBuffer(textDisp *textD, textBuffer *buffer);
textBuffer *TextDGetBuffer(textDisp *textD);
void TextDAttachHighlightData(textDisp *textD, textBuffer *styleBuffer,
    	styleTableEntry *styleTable, int nStyles, char unfinishedStyle,
    	unfinishedStyleCBProc unfinishedHighlightCB, void *cbArg);
void TextDSetFont(textDisp *textD, XFontStruct *fontStruct);
void TextDResize(textDisp *textD, int width, int height);
void TextDRedisplayRect(textDisp *textD, int left, int top, int width,
	int height);
void TextDRedisplayRange(textDisp *textD, int start, int end);
void TextDSetScroll(textDisp *textD, int topLineNum, int horizOffset);
void TextDGetScroll(textDisp *textD, int *topLineNum, int *horizOffset);
void TextDInsert(textDisp *textD, char *text);
void TextDOverstrike(textDisp *textD, char *text);
void TextDSetInsertPosition(textDisp *textD, int newPos);
int TextDGetInsertPosition(textDisp *textD);
int TextDXYToPosition(textDisp *textD, int x, int y);
void TextDXYToUnconstrainedPosition(textDisp *textD, int x, int y, int *row,
	int *column);
int TextDOffsetWrappedColumn(textDisp *textD, int row, int column);
int TextDOffsetWrappedRow(textDisp *textD, int row);
int TextDPositionToXY(textDisp *textD, int pos, int *x, int *y);
int TextDPosToLineAndCol(textDisp *textD, int pos, int *lineNum, int *column);
int TextDInSelection(textDisp *textD, int x, int y);
void TextDMakeInsertPosVisible(textDisp *textD);
int TextDMoveRight(textDisp *textD);
int TextDMoveLeft(textDisp *textD);
int TextDMoveUp(textDisp *textD);
int TextDMoveDown(textDisp *textD);
void TextDBlankCursor(textDisp *textD);
void TextDUnblankCursor(textDisp *textD);
void TextDSetCursorStyle(textDisp *textD, int style);
void TextDSetWrapMode(textDisp *textD, int wrap, int wrapMargin);
int TextDEndOfLine(textDisp *textD, int pos, int startPosIsLineStart);
int TextDStartOfLine(textDisp *textD, int pos);
int TextDCountForwardNLines(textDisp *textD, int startPos, int nLines,
    	int startPosIsLineStart);
int TextDCountBackwardNLines(textDisp *textD, int startPos, int nLines);
int TextDCountLines(textDisp *textD, int startPos, int endPos,
    	int startPosIsLineStart);
