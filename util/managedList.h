/* $Id: managedList.h,v 1.3 2001/02/26 23:38:03 edg Exp $ */
Widget CreateManagedList(Widget parent, char *name, Arg *args,
    	int argC, void **itemList, int *nItems, int maxItems, int nColumns,
    	void *(*getDialogDataCB)(void *, int, int *, void *),
    	void *getDialogDataArg, void (*setDialogDataCB)(void *, void *),
    	void *setDialogDataArg, void (*freeItemCB)(void *));
Widget ManageListAndButtons(Widget listW,
	Widget deleteBtn, Widget copyBtn, Widget moveUpBtn,
	Widget moveDownBtn, void **itemList, int *nItems,
	int maxItems, void *(*getDialogDataCB)(void *, int, int *, void *),
    	void *getDialogDataArg, void (*setDialogDataCB)(void *, void *),
    	void *setDialogDataArg, void (*freeItemCB)(void *));
int UpdateManagedList(Widget listW, int explicitRequest);
int ManagedListSelectedIndex(Widget listW);
void ChangeManagedListData(Widget listW);
void SelectManagedListItem(Widget listW, int itemIndex);
void AddDeleteConfirmCB(Widget listW, int (*deleteConfirmCB)(int, void *),
    	void *deleteConfirmArg);
