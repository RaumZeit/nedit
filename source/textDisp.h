/* $Id: textDisp.h,v 1.10 2002/06/20 21:32:32 slobasso Exp $ */
enum cursorStyles {NORMAL_CURSOR, CARET_CURSOR, DIM_CURSOR, BLOCK_CURSOR,
	HEAVY_CURSOR};

#define NO_HINT -1

typedef struct {
    Pixel color;
    Boolean underline;
    XFontStruct *font;
} styleTableEntry;

typedef struct graphicExposeTranslationEntry {
    int horizontal;
    int vertical;
    struct graphicExposeTranslationEntry *next;
} graphicExposeTranslationEntry;

typedef void (*unfinishedStyleCBProc)();

typedef struct _textDisp {
    Widget w;
    int top, left, width, height, lineNumLeft, lineNumWidth;
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
    int absTopLineNum;			/* In continuous wrap mode, the line
    					   number of the top line if the text
					   were not wrapped (note that this is
					   only maintained as needed). */
    int needAbsTopLineNum;		/* Externally settable flag to continue
    					   maintaining absTopLineNum even if
					   it isn't needed for line # display */
    int horizOffset;			/* Horizontal scroll pos. in pixels */
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
    Pixel lineNumFGPixel;   	    	/* Color for drawing line numbers */
    Pixel bgPixel, selectBGPixel;   	/* Background colors */
    Pixel highlightBGPixel;
    GC gc, selectGC, highlightGC;	/* GCs for drawing text */
    GC selectBGGC, highlightBGGC;	/* GCs for erasing text */
    GC cursorFGGC;			/* GC for drawing the cursor */
    GC styleGC;     	    	    	/* GC with color and font unspecified
    	    	    	    	    	   for drawing colored/styled text */
    GC lineNumGC;   	    	    	/* GC for drawing line numbers */
    
    int suppressResync;			/* Suppress resynchronization of line
                                           starts during buffer updates */
    int nLinesDeleted;			/* Number of lines deleted during
					   buffer modification (only used
				           when resynchronization is suppressed) */
    int modifyingTabDist;		/* Whether tab distance is being
    					   modified */
    Boolean pointerHidden;              /* true if the mouse pointer is hidden */
    graphicExposeTranslationEntry *graphicsExposeQueue;
} textDisp;

textDisp *TextDCreate(Widget widget, Widget hScrollBar, Widget vScrollBar,
	Position left, Position top, Position width, Position height,
	Position lineNumLeft, Position lineNumWidth, textBuffer *buffer,
	XFontStruct *fontStruct, Pixel bgPixel, Pixel fgPixel,
	Pixel selectFGPixel, Pixel selectBGPixel, Pixel highlightFGPixel,
	Pixel highlightBGPixel, Pixel cursorFGPixel, Pixel lineNumFGPixel,
	int continuousWrap, int wrapMargin);
void TextDFree(textDisp *textD);
void TextDSetBuffer(textDisp *textD, textBuffer *buffer);
textBuffer *TextDGetBuffer(textDisp *textD);
void TextDAttachHighlightData(textDisp *textD, textBuffer *styleBuffer,
    	styleTableEntry *styleTable, int nStyles, char unfinishedStyle,
    	unfinishedStyleCBProc unfinishedHighlightCB, void *cbArg);
void TextDSetFont(textDisp *textD, XFontStruct *fontStruct);
int TextDMinFontWidth(textDisp *textD, Boolean considerStyles);
int TextDMaxFontWidth(textDisp *textD, Boolean considerStyles);
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
int TextDXYToCharPos(textDisp *textD, int x, int y);
void TextDXYToUnconstrainedPosition(textDisp *textD, int x, int y, int *row,
	int *column);
int TextDLineAndColToPos(textDisp *textD, int lineNum, int column);
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
void TextDSetLineNumberArea(textDisp *textD, int lineNumLeft, int lineNumWidth,
	int textLeft);
void TextDMaintainAbsLineNum(textDisp *textD, int state);
int TextDPosOfPreferredCol(textDisp *textD, int column, int lineStartPos);
int TextDPreferredColumn(textDisp *textD, int *visLineNum, int *lineStartPos);
void TextDImposeGraphicsExposeTranslation(textDisp *textD, int *xOffset, int *yOffset);
Boolean TextDPopGraphicExposeQueueEntry(textDisp *textD);
void TextDTranlateGraphicExposeQueue(textDisp *textD, int xOffset, int yOffset, Boolean appendEntry);
