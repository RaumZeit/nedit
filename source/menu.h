#define PERMANENT_MENU_ITEM 1
#define TEMPORARY_MENU_ITEM 2

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
