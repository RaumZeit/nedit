/* $Id: window.h,v 1.13 2003/12/25 06:55:08 tksoh Exp $ */

#ifndef NEDIT_WINDOW_H_INCLUDED
#define NEDIT_WINDOW_H_INCLUDED

#include "nedit.h"
#include "textBuf.h"

#include <X11/Intrinsic.h>

WindowInfo *CreateWindow(const char *title, char *geometry, int iconic);
void CloseWindow(WindowInfo *window);
int NWindows(void);
void UpdateWindowTitle(const WindowInfo *window);
void UpdateWindowReadOnly(WindowInfo *window);
void UpdateStatsLine(WindowInfo *window);
void UpdateLineNumDisp(WindowInfo *window);
void UpdateWMSizeHints(WindowInfo *window);
void UpdateMinPaneHeights(WindowInfo *window);
void SetWindowModified(WindowInfo *window, int modified);
void MakeSelectionVisible(WindowInfo *window, Widget textPane);
int GetSelection(Widget widget, int *left, int *right);
int GetSimpleSelection(textBuffer *buf, int *left, int *right);
char *GetTextRange(Widget widget, int left, int right);
WindowInfo *FindWindowWithFile(const char *name, const char *path);
void SetAutoIndent(WindowInfo *window, int state);
void SetShowMatching(WindowInfo *window, int state);
void SetFonts(WindowInfo *window, const char *fontName, const char *italicName,
	const char *boldName, const char *boldItalicName);
void SetColors(WindowInfo *window, const char *textFg, const char *textBg,  
        const char *selectFg, const char *selectBg, const char *hiliteFg, 
        const char *hiliteBg, const char *lineNoFg, const char *cursorFg);
void SetOverstrike(WindowInfo *window, int overstrike);
void SetAutoWrap(WindowInfo *window, int state);
void SetWrapMargin(WindowInfo *window, int margin);
void SplitWindow(WindowInfo *window);
Widget GetPaneByIndex(WindowInfo *window, int paneIndex);
int WidgetToPaneIndex(WindowInfo *window, Widget w);
void ClosePane(WindowInfo *window);
void ShowBufferTabBar(WindowInfo *window, int state);
void ShowStatsLine(WindowInfo *window, int state);
void ShowISearchLine(WindowInfo *window, int state);
void TempShowISearch(WindowInfo *window, int state);
void ShowLineNumbers(WindowInfo *window, int state);
void SetModeMessage(WindowInfo *window, const char *message);
void ClearModeMessage(WindowInfo *window);
WindowInfo *WidgetToWindow(Widget w);
void AddSmallIcon(Widget shell);
void SetTabDist(WindowInfo *window, int tabDist);
void SetEmTabDist(WindowInfo *window, int emTabDist);
int CloseAllBufferInWindow(WindowInfo *window);
WindowInfo *CreateBuffer(WindowInfo *shellWindow, const char *name,
	char *geometry, int iconic);
WindowInfo *TabToWindow(Widget tab);
void RaiseBuffer(WindowInfo *window);
void RaiseBufferWindow(WindowInfo *window);
void DeleteBuffer(WindowInfo *window);
WindowInfo *MarkLastBuffer(WindowInfo *window);
WindowInfo *MarkActiveBuffer(WindowInfo *window);
void NextBuffer(WindowInfo *window);
void PreviousBuffer(WindowInfo *window);
void ToggleBuffer(WindowInfo *window);
int NBuffers(WindowInfo *window);
WindowInfo *AttachBuffer(WindowInfo *toWindow, WindowInfo *window);
WindowInfo *DetachBuffer(WindowInfo *window);
void AttachBufferDialog(Widget parent);
WindowInfo* GetTopBuffer(Widget w);
Boolean IsTopBuffer(const WindowInfo *window);
int IsIconic(WindowInfo *window);
int IsValidWindow(WindowInfo *window);
void RefreshTabState(WindowInfo *window);
void ShowWindowTabBar(WindowInfo *window);
void RefreshMenuToggleStates(WindowInfo *window);
void RefreshBufferWindowState(WindowInfo *window);
void AllWindowsBusy(const char* message);
void AllWindowsUnbusy(void);
void SetBacklightChars(WindowInfo *window, char *applyBacklightTypes);
#endif /* NEDIT_WINDOW_H_INCLUDED */
