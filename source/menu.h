/* $Id: menu.h,v 1.7 2002/07/11 21:18:10 slobasso Exp $ */

#ifndef NEDIT_MENU_H_INCLUDED
#define NEDIT_MENU_H_INCLUDED

#include "nedit.h"

#include <X11/Intrinsic.h>
#include <X11/Xlib.h>

#define PERMANENT_MENU_ITEM (XtPointer)1
#define TEMPORARY_MENU_ITEM (XtPointer)2

Widget CreateMenuBar(Widget parent, WindowInfo *window);
void InstallMenuActions(XtAppContext context);
XtActionsRec *GetMenuActions(int *nActions);
void InvalidateWindowMenus(void);
void CheckCloseDim(void);
void AddToPrevOpenMenu(const char *filename);
void WriteNEditDB(void);
void ReadNEditDB(void);
Widget CreateBGMenu(WindowInfo *window);
void AddBGMenuAction(Widget widget);
void HidePointerOnKeyedEvent(Widget w, XEvent *event);

#endif /* NEDIT_MENU_H_INCLUDED */
