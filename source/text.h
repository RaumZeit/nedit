/* $Id: text.h,v 1.4 2001/03/09 16:58:59 slobasso Exp $ */
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
#define textNlineNumForeground "lineNumForeground"
#define textCLineNumForeground "LineNumForeground"
#define textNpendingDelete "pendingDelete"
#define textCPendingDelete "PendingDelete"
#define textNhScrollBar "hScrollBar"
#define textCHScrollBar "HScrollBar"
#define textNvScrollBar "vScrollBar"
#define textCVScrollBar "VScrollBar"
#define textNlineNumCols "lineNumCols"
#define textCLineNumCols "LineNumCols"
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
int TextGetMinFontWidth(Widget w, Boolean considerStyles);
int TextGetMaxFontWidth(Widget w, Boolean considerStyles);
void TextHandleXSelections(Widget w);
void TextStopHandlingSelections(Widget w);
void TextPasteClipboard(Widget w, Time time);
void TextColPasteClipboard(Widget w, Time time);
void TextCopyClipboard(Widget w, Time time);
void TextCutClipboard(Widget w, Time time);
int TextFirstVisibleLine(Widget w);
int TextNumVisibleLines(Widget w);
int TextVisibleWidth(Widget w);
void TextInsertAtCursor(Widget w, char *chars, XEvent *event,
    	int allowPendingDelete, int allowWrap);
int TextFirstVisiblePos(Widget w);
int TextLastVisiblePos(Widget w);
char *TextGetWrapped(Widget w, int startPos, int endPos, int *length);
XtActionsRec *TextGetActions(int *nActions);
#endif
