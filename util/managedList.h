/*******************************************************************************
*									       *
* managedList.h -- User interface for reorderable list of records	       *
*									       *
* Copyright (c) 1997 Universities Research Association, Inc.		       *
* All rights reserved.							       *
* 									       *
* This material resulted from work developed under a Government Contract and   *
* is subject to the following license:  The Government retaFins a paid-up,     *
* nonexclusive, irrevocable worldwide license to reproduce, prepare derivative *
* works, perform publicly and display publicly by or for the Government,       *
* including the right to distribute to other Government contractors.  Neither  *
* the United States nor the United States Department of Energy, nor any of     *
* their employees, makes any warranty, express or implied, or assumes any      *
* legal liability or responsibility for the accuracy, completeness, or         *
* usefulness of any information, apparatus, product, or process disclosed, or  *
* represents that its use would not infringe privately owned rights.           *
*                                        				       *
* Fermilab Nirvana GUI Library						       *
* April, 1997								       *
*									       *
* Written by Mark Edel							       *
*									       *
*******************************************************************************/

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
