/* $Id: window.h,v 1.11 2002/09/26 12:37:40 ajhood Exp $ */

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
void SetOverstrike(WindowInfo *window, int overstrike);
void SetAutoWrap(WindowInfo *window, int state);
void SetWrapMargin(WindowInfo *window, int margin);
void SplitWindow(WindowInfo *window);
Widget GetPaneByIndex(WindowInfo *window, int paneIndex);
int WidgetToPaneIndex(WindowInfo *window, Widget w);
void ClosePane(WindowInfo *window);
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
void AllWindowsBusy(const char* message);
void AllWindowsUnbusy(void);
void SetBacklightChars(WindowInfo *window, char *applyBacklightTypes);
#endif /* NEDIT_WINDOW_H_INCLUDED */
