/* $Id: textP.h,v 1.3 2001/02/26 23:38:03 edg Exp $ */
/*******************************************************************************
*									       *
* textP.h - Text Editing Widget	private include file			       *
*									       *
*******************************************************************************/
#ifndef TEXTP_H
#define TEXTP_H

#include "text.h"
#include <Xm/XmP.h>

enum dragStates {NOT_CLICKED, PRIMARY_CLICKED, SECONDARY_CLICKED,
	CLICKED_IN_SELECTION,  PRIMARY_DRAG, PRIMARY_RECT_DRAG, SECONDARY_DRAG,
	SECONDARY_RECT_DRAG, PRIMARY_BLOCK_DRAG, DRAG_CANCELED, MOUSE_PAN};
enum multiClickStates {NO_CLICKS, ONE_CLICK, TWO_CLICKS, THREE_CLICKS};

typedef struct _TextClassPart{
    int ignore;
} TextClassPart;

typedef struct _TextClassRec{
    CoreClassPart  core_class;
    XmPrimitiveClassPart primitive_class;
    TextClassPart  text_class;
} TextClassRec;

extern TextClassRec nTextClassRec;

typedef struct _TextPart {
    /* resources */
    Pixel selectFGPixel, selectBGPixel, highlightFGPixel, highlightBGPixel;
    Pixel cursorFGPixel, lineNumFGPixel;
    XFontStruct *fontStruct;
    Boolean pendingDelete;
    Boolean autoShowInsertPos;
    Boolean autoWrap;
    Boolean autoWrapPastedText;
    Boolean continuousWrap;
    Boolean autoIndent;
    Boolean smartIndent;
    Boolean overstrike;
    Boolean heavyCursor;
    Boolean readOnly;
    int rows, columns;
    int marginWidth, marginHeight;
    int cursorBlinkRate;
    int wrapMargin;
    int emulateTabs;
    int lineNumCols;
    char *delimiters;
    Widget hScrollBar, vScrollBar;
    XtCallbackList focusInCB;
    XtCallbackList focusOutCB;
    XtCallbackList cursorCB;
    XtCallbackList dragStartCB;
    XtCallbackList dragEndCB;
    XtCallbackList smartIndentCB;
    /* private state */
    textDisp *textD;			/* Pointer to display information */
    int anchor, rectAnchor;		/* Anchors for drag operations and
    					   rectangular drag operations */
    int dragState;			/* Why is the mouse being dragged
    					   and what is being acquired */
    int multiClickState;		/* How long is this multi-click
    					   sequence so far */
    int btnDownX, btnDownY;		/* Mark the position of last btn down
    					   action for deciding when to begin
    					   paying attention to motion actions,
    					   and where to paste columns */
    Time lastBtnDown;			/* Timestamp of last button down event
    					   for multi-click recognition */
    int mouseX, mouseY;			/* Last known mouse position in drag
    					   operation (for autoscroll) */
    int selectionOwner;			/* True if widget owns the selection */
    int motifDestOwner;			/* " " owns the motif destination */
    int emTabsBeforeCursor;		/* If non-zero, number of consecutive
    					   emulated tabs just entered.  Saved
    					   so chars can be deleted as a unit */
    XtIntervalId autoScrollProcID;	/* id of Xt timer proc for autoscroll */
    XtIntervalId cursorBlinkProcID;	/* id of timer proc for cursor blink */
    textBuffer *dragOrigBuf;	    	/* backup buffer copy used during
    	    	    	    	    	   block dragging of selections */
    int dragXOffset, dragYOffset;   	/* offsets between cursor location and
    	    	    	    	    	   actual insertion point in drag */
    int dragType;   	    	    	/* style of block drag operation */
    int dragInsertPos;	    	    	/* location where text being block
    	    	    	    	    	   dragged was last inserted */
    int dragRectStart;	    	    	/* rect. offset "" */
    int dragInserted;	    	    	/* # of characters inserted at drag
    	    	    	    	    	   destination in last drag position */
    int dragDeleted;	    	    	/* # of characters deleted "" */
    int dragSourceDeletePos;	    	/* location from which move source
    	    	    	    	    	   text was removed at start of drag */
    int dragSourceInserted; 	    	/* # of chars. inserted when move
    	    	    	    	    	   source text was deleted */
    int dragSourceDeleted;  	    	/* # of chars. deleted "" */
    int dragNLines; 	    	    	/* # of newlines in text being drag'd */
} TextPart;

typedef struct _TextRec {
   CorePart        core;
   XmPrimitivePart primitive;
   TextPart        text;
} TextRec;

#endif
