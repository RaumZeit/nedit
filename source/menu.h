/* $Id: menu.h,v 1.6 2002/04/19 16:22:52 slobasso Exp $ */
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
