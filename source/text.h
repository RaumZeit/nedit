/*******************************************************************************
*									       *
* text.h - Text Editing Widget						       *
*									       *
* Copyright (c) 1995 Universities Research Association, Inc.		       *
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

#ifndef  Text_H
#define  Text_H

/* Resource strings */
#define textNfont "font"
#define textCFont "Font"
#define textNrows "rows"
#define textCRows "Rows"
#define textNcolumns "columns"
#define textCColumns "Columns"
#define textNmarginWidth "marginWidth"
#define textCMarginWidth "MarginWidth"
#define textNmarginHeight "marginHeight"
#define textCMarginHeight "MarginHeight"
#define textNselectForeground "selectForeground"
#define textCSelectForeground "SelectForeground"
#define textNselectBackground "selectBackground"
#define textCSelectBackground "SelectBackground"
#define textNhighlightForeground "highlightForeground"
#define textCHighlightForeground "HighlightForeground"
#define textNhighlightBackground "highlightBackground"
#define textCHighlightBackground "HighlightBackground"
#define textNcursorForeground "cursorForeground"
#define textCCursorForeground "CursorForeground"
#define textNpendingDelete "pendingDelete"
#define textCPendingDelete "PendingDelete"
#define textNhScrollBar "hScrollBar"
#define textCHScrollBar "HScrollBar"
#define textNvScrollBar "vScrollBar"
#define textCVScrollBar "VScrollBar"
#define textNautoShowInsertPos "autoShowInsertPos"
#define textCAutoShowInsertPos "AutoShowInsertPos"
#define textNautoWrapPastedText "autoWrapPastedText"
#define textCAutoWrapPastedText "AutoWrapPastedText"
#define textNwordDelimiters "wordDelimiters"
#define textCWordDelimiters "WordDelimiters"
#define textNblinkRate "blinkRate"
#define textCBlinkRate "BlinkRate"
#define textNfocusCallback "focusCallback"
#define textCFocusCallback "FocusCallback"
#define textNlosingFocusCallback "losingFocusCallback"
#define textCLosingFocusCallback "LosingFocusCallback"
#define textNcursorMovementCallback "cursorMovementCallback"
#define textCCursorMovementCallback "CursorMovementCallback"
#define textNdragStartCallback "dragStartCallback"
#define textCDragStartCallback "DragStartCallback"
#define textNdragEndCallback "dragEndCallback"
#define textCDragEndCallback "DragEndCallback"
#define textNsmartIndentCallback "smartIndentCallback"
#define textCSmartIndentCallback "SmartIndentCallback"
#define textNautoWrap "autoWrap"
#define textCAutoWrap "AutoWrap"
#define textNcontinuousWrap "continuousWrap"
#define textCContinuousWrap "ContinuousWrap"
#define textNwrapMargin "wrapMargin"
#define textCWrapMargin "WrapMargin"
#define textNautoIndent "autoIndent"
#define textCAutoIndent "AutoIndent"
#define textNsmartIndent "smartIndent"
#define textCSmartIndent "SmartIndent"
#define textNoverstrike "overstrike"
#define textCOverstrike "Overstrike"
#define textNheavyCursor "heavyCursor"
#define textCHeavyCursor "HeavyCursor"
#define textNreadOnly "readOnly"
#define textCReadOnly "ReadOnly"
#define textNemulateTabs "emulateTabs"
#define textCEmulateTabs "EmulateTabs"

extern WidgetClass textWidgetClass;

typedef struct _TextClassRec *TextWidgetClass;
typedef struct _TextRec *TextWidget;

typedef struct {
    int startPos;
    int nCharsDeleted;
    int nCharsInserted;
    char *deletedText;
} dragEndCBStruct;

enum smartIndentCallbackReasons {NEWLINE_INDENT_NEEDED, CHAR_TYPED};
typedef struct {
    int reason;
    int pos;
    int indentRequest;
    char *charsTyped;
} smartIndentCBStruct;

/* User callable routines */
void TextSetBuffer(Widget w, textBuffer *buffer);
textBuffer *TextGetBuffer(Widget w);
int TextPosToLineAndCol(Widget w, int pos, int *lineNum, int *column);
int TextPosToXY(Widget w, int pos, int *x, int *y);
int TextGetCursorPos(Widget w);
void TextSetCursorPos(Widget w, int pos);
void TextGetScroll(Widget w, int *topLineNum, int *horizOffset);
void TextSetScroll(Widget w, int topLineNum, int horizOffset);
void TextHandleXSelections(Widget w);
void TextStopHandlingSelections(Widget w);
void TextPasteClipboard(Widget w, Time time);
void TextColPasteClipboard(Widget w, Time time);
void TextCopyClipboard(Widget w, Time time);
void TextCutClipboard(Widget w, Time time);
void TextInsertAtCursor(Widget w, char *chars, XEvent *event,
    	int allowPendingDelete, int allowWrap);
int TextFirstVisiblePos(Widget w);
int TextLastVisiblePos(Widget w);
char *TextGetWrapped(Widget w, int startPos, int endPos, int *length);
XtActionsRec *TextGetActions(int *nActions);
#endif
