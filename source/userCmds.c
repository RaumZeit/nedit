static const char CVSID[] = "$Id: userCmds.c,v 1.10 2001/03/05 15:00:13 slobasso Exp $";
/*******************************************************************************
*									       *
* userCmds.c -- Nirvana Editor shell and macro command dialogs 		       *
*									       *
* Copyright (C) 1999 Mark Edel						       *
*									       *
* This is free software; you can redistribute it and/or modify it under the    *
* terms of the GNU General Public License as published by the Free Software    *
* Foundation; either version 2 of the License, or (at your option) any later   *
* version.							               *
* 									       *
* This software is distributed in the hope that it will be useful, but WITHOUT *
* ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or        *
* FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Lesser General Public License *
* for more details.							       *
* 									       *
* You should have received a copy of the GNU General Public License along with *
* software; if not, write to the Free Software Foundation, Inc., 59 Temple     *
* Place, Suite 330, Boston, MA  02111-1307 USA		                       *
*									       *
* Nirvana Text Editor	    						       *
* April, 1997								       *
*									       *
* Written by Mark Edel							       *
*									       *
*******************************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#ifdef VMS
#include "../util/VMSparam.h"
#else
#ifndef __MVS__
#include <sys/param.h>
#endif
#endif /*VMS*/
#include <Xm/Xm.h>
#include <X11/keysym.h>
#include <Xm/Text.h>
#include <Xm/Form.h>
#include <Xm/List.h>
#include <Xm/LabelG.h>
#include <Xm/PushB.h>
#include <Xm/ToggleB.h>
#include <Xm/SelectioB.h>
#include <Xm/RowColumn.h>
#include <Xm/CascadeB.h>
#include "../util/DialogF.h"
#include "../util/misc.h"
#include "../util/managedList.h"
#include "textBuf.h"
#include "text.h"
#include "nedit.h"
#include "preferences.h"
#include "window.h"
#include "menu.h"
#include "shell.h"
#include "macro.h"
#include "file.h"
#include "rbTree.h"
#include "interpret.h"
#include "parse.h"

/* max number of user programmable menu commands allowed per each of the
   macro, shell, and bacground menus */
#define MAX_ITEMS_PER_MENU 400

/* major divisions (in position units) in User Commands dialogs */
#define LEFT_MARGIN_POS 1
#define RIGHT_MARGIN_POS 99
#define LIST_RIGHT 45
#define SHELL_CMD_TOP 70
#define MACRO_CMD_TOP 40

/* types of current dialog and/or menu */
enum dialogTypes {SHELL_CMDS, MACRO_CMDS, BG_MENU_CMDS};

/* Structure representing a menu item for both the shell and macro menus */
typedef struct {
    char *name;
    unsigned int modifiers;
    KeySym keysym;
    char mnemonic;
    char input;
    char output;
    char repInput;
    char saveFirst;
    char loadAfter;
    char *cmd;
} menuItemRec;

/* Structure for widgets and flags associated with both shell command
   and macro command editing dialogs */
typedef struct {
    int dialogType;
    WindowInfo *window;
    Widget nameTextW, accTextW, mneTextW, cmdTextW, saveFirstBtn;
    Widget loadAfterBtn, selInpBtn, winInpBtn, eitherInpBtn, noInpBtn;
    Widget repInpBtn, sameOutBtn, dlogOutBtn, winOutBtn, dlogShell;
    Widget managedList;
    menuItemRec **menuItemsList;
    int nMenuItems;
} userCmdDialog;

/* Structure for keeping track of hierarchical sub-menus durring user-menu
   creation */
typedef struct {
    char *name;
    Widget menuPane;
} menuTreeItem;

/* Descriptions of the current user programmed menu items for re-generating
   menus and processing shell, macro, and background menu selections */
static menuItemRec *ShellMenuItems[MAX_ITEMS_PER_MENU];
static int NShellMenuItems = 0;
static menuItemRec *MacroMenuItems[MAX_ITEMS_PER_MENU];
static int NMacroMenuItems = 0;
static menuItemRec *BGMenuItems[MAX_ITEMS_PER_MENU];
static int NBGMenuItems = 0;

/* Top level shells of the user-defined menu editing dialogs */
static Widget ShellCmdDialog = NULL;
static Widget MacroCmdDialog = NULL;
static Widget BGMenuCmdDialog = NULL;

/* Paste learn/replay sequence buttons in user defined menu editing dialogs
   (for dimming and undimming externally when replay macro is available) */
static Widget MacroPasteReplayBtn = NULL;
static Widget BGMenuPasteReplayBtn = NULL;

static void editMacroOrBGMenu(WindowInfo *window, int dialogType);
static void dimSelDepItemsInMenu(Widget menuPane, menuItemRec **menuList,
	int nMenuItems, int sensitive);
static void updateMenus(int menuType);
static Widget findInMenuTree(menuTreeItem *menuTree, int nTreeEntries,
	char *hierName);
static char *copySubstring(char *string, int length);
static char *findStripLanguageMode(char *menuItemName, int languageMode,
	int *isDefaultLM);
static Widget createUserMenuItem(Widget menuPane, char *name, menuItemRec *f,
	int index, XtCallbackProc cbRtn, XtPointer cbArg);
static Widget createUserSubMenu(Widget parent, char *label);
static void removeMenuItems(Widget menuPane);
static void updateMenu(WindowInfo *window, int menuType);
static void okCB(Widget w, XtPointer clientData, XtPointer callData);
static void applyCB(Widget w, XtPointer clientData, XtPointer callData);
static void checkCB(Widget w, XtPointer clientData, XtPointer callData);
static int checkMacro(userCmdDialog *ucd);
static int checkMacroText(char *macro, Widget errorParent, Widget errFocus);
static int applyDialogChanges(userCmdDialog *ucd);
static void dismissCB(Widget w, XtPointer clientData, XtPointer callData);
static void pasteReplayCB(Widget w, XtPointer clientData, XtPointer callData);
static void destroyCB(Widget w, XtPointer clientData, XtPointer callData);
static void accKeyCB(Widget w, XtPointer clientData, XKeyEvent *event);
static void sameOutCB(Widget w, XtPointer clientData, XtPointer callData);
static void shellMenuCB(Widget w, WindowInfo *window, XtPointer callData);
static void macroMenuCB(Widget w, WindowInfo *window, XtPointer callData);
static void bgMenuCB(Widget w, WindowInfo *window, XtPointer callData) ;
static void accFocusCB(Widget w, XtPointer clientData, XtPointer callData);
static void accLoseFocusCB(Widget w, XtPointer clientData,
	XtPointer callData);
static void updateDialogFields(menuItemRec *f, userCmdDialog *ucd);
static menuItemRec *readDialogFields(userCmdDialog *ucd, int silent);
static menuItemRec *copyMenuItemRec(menuItemRec *item);
static void freeMenuItemRec(menuItemRec *item);
static void *getDialogDataCB(void *oldItem, int explicitRequest, int *abort,
    	void *cbArg);
static void setDialogDataCB(void *item, void *cbArg);
static void freeItemCB(void *item);
static int dialogFieldsAreEmpty(userCmdDialog *ucd);
static void disableTextW(Widget textW);
static char *writeMenuItemString(menuItemRec **menuItems, int nItems,
	int listType);
static int loadMenuItemString(char *inString, menuItemRec **menuItems,
	int *nItems, int listType);
static void generateAcceleratorString(char *text, unsigned int modifiers,
	KeySym keysym);
static void genAccelEventName(char *text, unsigned int modifiers,
	KeySym keysym);
static int parseAcceleratorString(char *string, unsigned int *modifiers,
	KeySym *keysym);
static int parseError(char *message);
static char *copyMacroToEnd(char **inPtr, char *itemName);
static void addTerminatingNewline(char **string);

/*
** Present a dialog for editing the user specified commands in the shell menu
*/
void EditShellMenu(WindowInfo *window)
{
    Widget form, accLabel, inpLabel, inpBox, outBox, outLabel;
    Widget nameLabel, cmdLabel, okBtn, applyBtn, dismissBtn;
    userCmdDialog *ucd;
    XmString s1;
    int ac, i;
    Arg args[20];

    /* if the dialog is already displayed, just pop it to the top and return */
    if (ShellCmdDialog != NULL) {
    	RaiseShellWindow(ShellCmdDialog);
    	return;
    }

    /* Create a structure for keeping track of dialog state */
    ucd = (userCmdDialog *)XtMalloc(sizeof(userCmdDialog));
    ucd->window = window;
    
    /* Set the dialog to operate on the Shell menu */
    ucd->menuItemsList = (menuItemRec **)XtMalloc(sizeof(menuItemRec *) *
    	    MAX_ITEMS_PER_MENU);
    for (i=0; i<NShellMenuItems; i++)
    	ucd->menuItemsList[i] = copyMenuItemRec(ShellMenuItems[i]);
    ucd->nMenuItems = NShellMenuItems;
    ucd->dialogType = SHELL_CMDS;
    
    ac = 0;
    XtSetArg(args[ac], XmNdeleteResponse, XmDO_NOTHING); ac++;
    XtSetArg(args[ac], XmNiconName, "Shell Commands"); ac++;
    XtSetArg(args[ac], XmNtitle, "Shell Commands"); ac++;
    ucd->dlogShell = CreateShellWithBestVis(APP_NAME, APP_CLASS,
	    applicationShellWidgetClass, TheDisplay, args, ac);
    AddSmallIcon(ucd->dlogShell);
    form = XtVaCreateManagedWidget("editShellCommands", xmFormWidgetClass,
	    ucd->dlogShell, XmNautoUnmanage, False,
	    XmNresizePolicy, XmRESIZE_NONE, NULL);
    ShellCmdDialog = ucd->dlogShell;
    XtAddCallback(form, XmNdestroyCallback, destroyCB, ucd);
    AddMotifCloseCallback(ucd->dlogShell, dismissCB, ucd);
 
    ac = 0;
    XtSetArg(args[ac], XmNtopAttachment, XmATTACH_POSITION); ac++;
    XtSetArg(args[ac], XmNtopPosition, 2); ac++;
    XtSetArg(args[ac], XmNleftAttachment, XmATTACH_POSITION); ac++;
    XtSetArg(args[ac], XmNleftPosition, LEFT_MARGIN_POS); ac++;
    XtSetArg(args[ac], XmNrightAttachment, XmATTACH_POSITION); ac++;
    XtSetArg(args[ac], XmNrightPosition, LIST_RIGHT-1); ac++;
    XtSetArg(args[ac], XmNbottomAttachment, XmATTACH_POSITION); ac++;
    XtSetArg(args[ac], XmNbottomPosition, SHELL_CMD_TOP); ac++;
    ucd->managedList = CreateManagedList(form, "list", args, ac,
    	    (void **)ucd->menuItemsList, &ucd->nMenuItems, MAX_ITEMS_PER_MENU,
    	    20, getDialogDataCB, ucd, setDialogDataCB, ucd, freeItemCB);

    ucd->loadAfterBtn = XtVaCreateManagedWidget("loadAfterBtn",
    	    xmToggleButtonWidgetClass, form,
    	    XmNlabelString, s1=MKSTRING("Re-load file after executing command"),
    	    XmNmnemonic, 'R',
    	    XmNalignment, XmALIGNMENT_BEGINNING,
    	    XmNset, False,
	    XmNleftAttachment, XmATTACH_POSITION,
	    XmNleftPosition, LIST_RIGHT,
	    XmNrightAttachment, XmATTACH_POSITION,
	    XmNrightPosition, RIGHT_MARGIN_POS,
	    XmNbottomAttachment, XmATTACH_POSITION,
	    XmNbottomPosition, SHELL_CMD_TOP, NULL);
    XmStringFree(s1);
    ucd->saveFirstBtn = XtVaCreateManagedWidget("saveFirstBtn",
    	    xmToggleButtonWidgetClass, form,
    	    XmNlabelString, s1=MKSTRING("Save file before executing command"),
    	    XmNmnemonic, 'f',
    	    XmNalignment, XmALIGNMENT_BEGINNING,
    	    XmNset, False,
	    XmNleftAttachment, XmATTACH_POSITION,
	    XmNleftPosition, LIST_RIGHT,
	    XmNrightAttachment, XmATTACH_POSITION,
	    XmNrightPosition, RIGHT_MARGIN_POS,
	    XmNbottomAttachment, XmATTACH_WIDGET,
	    XmNbottomWidget, ucd->loadAfterBtn, NULL);
    XmStringFree(s1);
    ucd->repInpBtn = XtVaCreateManagedWidget("repInpBtn",
    	    xmToggleButtonWidgetClass, form,
    	    XmNlabelString, s1=MKSTRING("Output replaces input"),
    	    XmNmnemonic, 'f',
    	    XmNalignment, XmALIGNMENT_BEGINNING,
    	    XmNset, False,
	    XmNleftAttachment, XmATTACH_POSITION,
	    XmNleftPosition, LIST_RIGHT,
	    XmNrightAttachment, XmATTACH_POSITION,
	    XmNrightPosition, RIGHT_MARGIN_POS,
	    XmNbottomAttachment, XmATTACH_WIDGET,
	    XmNbottomWidget, ucd->saveFirstBtn, NULL);
    XmStringFree(s1);
    outBox = XtVaCreateManagedWidget("outBox", xmRowColumnWidgetClass, form,
	    XmNpacking, XmPACK_TIGHT,
	    XmNorientation, XmHORIZONTAL,
	    XmNradioBehavior, True,
	    XmNradioAlwaysOne, True,
	    XmNleftAttachment, XmATTACH_POSITION,
	    XmNleftPosition, LIST_RIGHT + 2,
	    XmNrightAttachment, XmATTACH_POSITION,
	    XmNrightPosition, RIGHT_MARGIN_POS,
	    XmNbottomAttachment, XmATTACH_WIDGET,
	    XmNbottomWidget, ucd->repInpBtn,
	    XmNbottomOffset, 4, NULL);
    ucd->sameOutBtn = XtVaCreateManagedWidget("sameOutBtn",
    	    xmToggleButtonWidgetClass, outBox,
    	    XmNlabelString, s1=MKSTRING("same window"),
    	    XmNmnemonic, 'm',
    	    XmNalignment, XmALIGNMENT_BEGINNING,
    	    XmNmarginHeight, 0,
    	    XmNset, True, NULL);
    XmStringFree(s1);
    XtAddCallback(ucd->sameOutBtn, XmNvalueChangedCallback, sameOutCB, ucd);
    ucd->dlogOutBtn = XtVaCreateManagedWidget("dlogOutBtn",
    	    xmToggleButtonWidgetClass, outBox,
    	    XmNlabelString, s1=MKSTRING("dialog"),
    	    XmNmnemonic, 'g',
    	    XmNalignment, XmALIGNMENT_BEGINNING,
    	    XmNmarginHeight, 0,
    	    XmNset, False, NULL);
    XmStringFree(s1);
    ucd->winOutBtn = XtVaCreateManagedWidget("winOutBtn", xmToggleButtonWidgetClass,
    	    outBox,
    	    XmNlabelString, s1=MKSTRING("new window"),
    	    XmNmnemonic, 'n',
    	    XmNalignment, XmALIGNMENT_BEGINNING,
    	    XmNmarginHeight, 0,
    	    XmNset, False, NULL);
    XmStringFree(s1);
    outLabel = XtVaCreateManagedWidget("outLabel", xmLabelGadgetClass, form,
    	    XmNlabelString, s1=MKSTRING("Command Output:"),
    	    XmNalignment, XmALIGNMENT_BEGINNING,
    	    XmNmarginTop, 5,
	    XmNleftAttachment, XmATTACH_POSITION,
	    XmNleftPosition, LIST_RIGHT,
	    XmNrightAttachment, XmATTACH_POSITION,
	    XmNrightPosition, RIGHT_MARGIN_POS,
	    XmNbottomAttachment, XmATTACH_WIDGET,
	    XmNbottomWidget, outBox, NULL);
    XmStringFree(s1);

    inpBox = XtVaCreateManagedWidget("inpBox", xmRowColumnWidgetClass, form,
	    XmNpacking, XmPACK_TIGHT,
	    XmNorientation, XmHORIZONTAL,
	    XmNradioBehavior, True,
	    XmNradioAlwaysOne, True,
	    XmNleftAttachment, XmATTACH_POSITION,
	    XmNleftPosition, LIST_RIGHT + 2,
	    XmNrightAttachment, XmATTACH_POSITION,
	    XmNrightPosition, RIGHT_MARGIN_POS,
	    XmNbottomAttachment, XmATTACH_WIDGET,
	    XmNbottomWidget, outLabel, NULL);
    ucd->selInpBtn = XtVaCreateManagedWidget("selInpBtn", xmToggleButtonWidgetClass,
    	    inpBox,
    	    XmNlabelString, s1=MKSTRING("selection"),
    	    XmNmnemonic, 's',
    	    XmNalignment, XmALIGNMENT_BEGINNING,
    	    XmNmarginHeight, 0,
    	    XmNset, True, NULL);
    XmStringFree(s1);
    ucd->winInpBtn = XtVaCreateManagedWidget("winInpBtn",
    	    xmToggleButtonWidgetClass, inpBox,
    	    XmNlabelString, s1=MKSTRING("window"),
    	    XmNmnemonic, 'w',
    	    XmNalignment, XmALIGNMENT_BEGINNING,
    	    XmNmarginHeight, 0,
    	    XmNset, False, NULL);
    XmStringFree(s1);
    ucd->eitherInpBtn = XtVaCreateManagedWidget("eitherInpBtn",
    	    xmToggleButtonWidgetClass, inpBox,
    	    XmNlabelString, s1=MKSTRING("either"),
    	    XmNmnemonic, 't',
    	    XmNalignment, XmALIGNMENT_BEGINNING,
    	    XmNmarginHeight, 0,
    	    XmNset, False, NULL);
    XmStringFree(s1);
    ucd->noInpBtn = XtVaCreateManagedWidget("noInpBtn",
    	    xmToggleButtonWidgetClass, inpBox,
    	    XmNlabelString, s1=MKSTRING("none"),
    	    XmNmnemonic, 'o',
    	    XmNalignment, XmALIGNMENT_BEGINNING,
    	    XmNmarginHeight, 0,
    	    XmNset, False, NULL);
    XmStringFree(s1);
    inpLabel = XtVaCreateManagedWidget("inpLabel", xmLabelGadgetClass, form,
    	    XmNlabelString, s1=MKSTRING("Command Input:"),
    	    XmNalignment, XmALIGNMENT_BEGINNING,
    	    XmNmarginTop, 5,
	    XmNleftAttachment, XmATTACH_POSITION,
	    XmNleftPosition, LIST_RIGHT,
	    XmNrightAttachment, XmATTACH_POSITION,
	    XmNrightPosition, RIGHT_MARGIN_POS,
	    XmNbottomAttachment, XmATTACH_WIDGET,
	    XmNbottomWidget, inpBox, NULL);
    XmStringFree(s1);
 
    ucd->mneTextW = XtVaCreateManagedWidget("mne", xmTextWidgetClass, form,
	    XmNcolumns, 1,
	    XmNmaxLength, 1,
	    XmNleftAttachment, XmATTACH_POSITION,
	    XmNleftPosition, RIGHT_MARGIN_POS-10,
	    XmNrightAttachment, XmATTACH_POSITION,
	    XmNrightPosition, RIGHT_MARGIN_POS,
	    XmNbottomAttachment, XmATTACH_WIDGET,
    	    XmNbottomWidget, inpLabel, NULL);
    RemapDeleteKey(ucd->mneTextW);

    ucd->accTextW = XtVaCreateManagedWidget("acc", xmTextWidgetClass, form,
    	    XmNcolumns, 12,
    	    XmNmaxLength, MAX_ACCEL_LEN-1,
    	    XmNcursorPositionVisible, False,
	    XmNleftAttachment, XmATTACH_POSITION,
	    XmNleftPosition, LIST_RIGHT,
	    XmNrightAttachment, XmATTACH_POSITION,
	    XmNrightPosition, RIGHT_MARGIN_POS-15,
	    XmNbottomAttachment, XmATTACH_WIDGET,
    	    XmNbottomWidget, inpLabel, NULL);
    XtAddEventHandler(ucd->accTextW, KeyPressMask, False,
    	    (XtEventHandler)accKeyCB, ucd);
    XtAddCallback(ucd->accTextW, XmNfocusCallback, accFocusCB, ucd);
    XtAddCallback(ucd->accTextW, XmNlosingFocusCallback, accLoseFocusCB, ucd);
    accLabel = XtVaCreateManagedWidget("accLabel", xmLabelGadgetClass, form,
    	    XmNlabelString, s1=MKSTRING("Accelerator"),
    	    XmNmnemonic, 'l',
    	    XmNuserData, ucd->accTextW,
    	    XmNalignment, XmALIGNMENT_BEGINNING,
    	    XmNmarginTop, 5,
    	    XmNleftAttachment, XmATTACH_POSITION,
    	    XmNleftPosition, LIST_RIGHT,
    	    XmNrightAttachment, XmATTACH_POSITION,
    	    XmNrightPosition, LIST_RIGHT + 24,
    	    XmNbottomAttachment, XmATTACH_WIDGET,
    	    XmNbottomWidget, ucd->mneTextW, NULL);
    XmStringFree(s1);

    XtVaCreateManagedWidget("mneLabel", xmLabelGadgetClass, form,
    	    XmNlabelString, s1=MKSTRING("Mnemonic"),
    	    XmNmnemonic, 'i',
    	    XmNuserData, ucd->mneTextW,
    	    XmNalignment, XmALIGNMENT_END,
    	    XmNmarginTop, 5,
    	    XmNleftAttachment, XmATTACH_POSITION,
    	    XmNleftPosition, LIST_RIGHT + 24,
    	    XmNrightAttachment, XmATTACH_POSITION,
    	    XmNrightPosition, RIGHT_MARGIN_POS,
    	    XmNbottomAttachment, XmATTACH_WIDGET,
    	    XmNbottomWidget, ucd->mneTextW, NULL);
    XmStringFree(s1);
    
    ucd->nameTextW = XtVaCreateManagedWidget("name", xmTextWidgetClass, form,
	    XmNleftAttachment, XmATTACH_POSITION,
	    XmNleftPosition, LIST_RIGHT,
	    XmNrightAttachment, XmATTACH_POSITION,
	    XmNrightPosition, RIGHT_MARGIN_POS,
	    XmNbottomAttachment, XmATTACH_WIDGET,
    	    XmNbottomWidget, accLabel, NULL);
    RemapDeleteKey(ucd->nameTextW);
 
    nameLabel = XtVaCreateManagedWidget("nameLabel", xmLabelGadgetClass, form,
    	    XmNlabelString, s1=MKSTRING("Menu Entry"),
    	    XmNmnemonic, 'y',
    	    XmNuserData, ucd->nameTextW,
    	    XmNalignment, XmALIGNMENT_BEGINNING,
    	    XmNmarginTop, 5,
    	    XmNleftAttachment, XmATTACH_POSITION,
    	    XmNleftPosition, LIST_RIGHT,
    	    XmNbottomAttachment, XmATTACH_WIDGET,
    	    XmNbottomWidget, ucd->nameTextW, NULL);
    XmStringFree(s1);
 
    XtVaCreateManagedWidget("nameNotes", xmLabelGadgetClass, form,
    	    XmNlabelString, s1=MKSTRING("(> for sub-menu, @ language mode)"),
    	    XmNalignment, XmALIGNMENT_END,
    	    XmNmarginTop, 5,
    	    XmNleftAttachment, XmATTACH_WIDGET,
    	    XmNleftWidget, nameLabel,
    	    XmNrightAttachment, XmATTACH_POSITION,
    	    XmNrightPosition, RIGHT_MARGIN_POS,
    	    XmNbottomAttachment, XmATTACH_WIDGET,
    	    XmNbottomWidget, ucd->nameTextW, NULL);
    XmStringFree(s1);

    XtVaCreateManagedWidget("topLabel", xmLabelGadgetClass, form,
    	    XmNlabelString, s1=MKSTRING(
"Select a shell menu item from the list at left.\n\
Select \"New\" to add a new command to the menu."),
	    XmNtopAttachment, XmATTACH_POSITION,
	    XmNtopPosition, 2,
	    XmNleftAttachment, XmATTACH_POSITION,
	    XmNleftPosition, LIST_RIGHT,
	    XmNrightAttachment, XmATTACH_POSITION,
	    XmNrightPosition, RIGHT_MARGIN_POS,
	    XmNbottomAttachment, XmATTACH_WIDGET,
    	    XmNbottomWidget, nameLabel, NULL);
    XmStringFree(s1);
 
    cmdLabel = XtVaCreateManagedWidget("cmdLabel", xmLabelGadgetClass, form,
    	    XmNlabelString, s1=MKSTRING("Shell Command to Execute"),
    	    XmNmnemonic, 'x',
    	    XmNalignment, XmALIGNMENT_BEGINNING,
    	    XmNmarginTop, 5,
    	    XmNtopAttachment, XmATTACH_POSITION,
    	    XmNtopPosition, SHELL_CMD_TOP,
    	    XmNleftAttachment, XmATTACH_POSITION,
    	    XmNleftPosition, LEFT_MARGIN_POS, NULL);
    XmStringFree(s1);
    XtVaCreateManagedWidget("cmdLabel", xmLabelGadgetClass, form,
    	    XmNlabelString, s1=MKSTRING("(% expands to current filename)"),
    	    XmNalignment, XmALIGNMENT_END,
    	    XmNmarginTop, 5,
    	    XmNtopAttachment, XmATTACH_POSITION,
    	    XmNtopPosition, SHELL_CMD_TOP,
    	    XmNleftAttachment, XmATTACH_WIDGET,
    	    XmNleftWidget, cmdLabel,
    	    XmNrightAttachment, XmATTACH_POSITION,
    	    XmNrightPosition, RIGHT_MARGIN_POS, NULL);
    XmStringFree(s1);

    okBtn = XtVaCreateManagedWidget("ok",xmPushButtonWidgetClass,form,
    	    XmNlabelString, s1=MKSTRING("OK"),
    	    XmNleftAttachment, XmATTACH_POSITION,
    	    XmNleftPosition, 13,
    	    XmNrightAttachment, XmATTACH_POSITION,
    	    XmNrightPosition, 29,
    	    XmNbottomAttachment, XmATTACH_POSITION,
    	    XmNbottomPosition, 99, NULL);
    XtAddCallback(okBtn, XmNactivateCallback, okCB, ucd);
    XmStringFree(s1);

    applyBtn = XtVaCreateManagedWidget("apply",xmPushButtonWidgetClass,form,
    	    XmNlabelString, s1=MKSTRING("Apply"),
    	    XmNmnemonic, 'A',
    	    XmNleftAttachment, XmATTACH_POSITION,
    	    XmNleftPosition, 42,
    	    XmNrightAttachment, XmATTACH_POSITION,
    	    XmNrightPosition, 58,
    	    XmNbottomAttachment, XmATTACH_POSITION,
    	    XmNbottomPosition, 99, NULL);
    XtAddCallback(applyBtn, XmNactivateCallback, applyCB, ucd);
    XmStringFree(s1);

    dismissBtn = XtVaCreateManagedWidget("dismiss",xmPushButtonWidgetClass,form,
    	    XmNlabelString, s1=MKSTRING("Dismiss"),
    	    XmNleftAttachment, XmATTACH_POSITION,
    	    XmNleftPosition, 71,
    	    XmNrightAttachment, XmATTACH_POSITION,
    	    XmNrightPosition, 87,
    	    XmNbottomAttachment, XmATTACH_POSITION,
    	    XmNbottomPosition, 99, NULL);
    XtAddCallback(dismissBtn, XmNactivateCallback, dismissCB, ucd);
    XmStringFree(s1);
    
    ac = 0;
    XtSetArg(args[ac], XmNeditMode, XmMULTI_LINE_EDIT); ac++;
    XtSetArg(args[ac], XmNscrollHorizontal, False); ac++;
    XtSetArg(args[ac], XmNwordWrap, True); ac++;
    XtSetArg(args[ac], XmNtopAttachment, XmATTACH_WIDGET); ac++;
    XtSetArg(args[ac], XmNtopWidget, cmdLabel); ac++;
    XtSetArg(args[ac], XmNleftAttachment, XmATTACH_POSITION); ac++;
    XtSetArg(args[ac], XmNleftPosition, LEFT_MARGIN_POS); ac++;
    XtSetArg(args[ac], XmNrightAttachment, XmATTACH_POSITION); ac++;
    XtSetArg(args[ac], XmNrightPosition, RIGHT_MARGIN_POS); ac++;
    XtSetArg(args[ac], XmNbottomAttachment, XmATTACH_WIDGET); ac++;
    XtSetArg(args[ac], XmNbottomWidget, okBtn); ac++;
    XtSetArg(args[ac], XmNbottomOffset, 5); ac++;
    ucd->cmdTextW = XmCreateScrolledText(form, "name", args, ac);
    XtManageChild(ucd->cmdTextW);
    MakeSingleLineTextW(ucd->cmdTextW);
    RemapDeleteKey(ucd->cmdTextW);
    XtVaSetValues(cmdLabel, XmNuserData, ucd->cmdTextW, NULL); /* for mnemonic */
   
    /* Disable text input for the accelerator key widget, let the
       event handler manage it instead */
    disableTextW(ucd->accTextW);

    /* initializs the dialog fields to match "New" list item */
    updateDialogFields(NULL, ucd); 
    
    /* Set initial default button */
    XtVaSetValues(form, XmNdefaultButton, okBtn, NULL);
    XtVaSetValues(form, XmNcancelButton, dismissBtn, NULL);
    
    /* Handle mnemonic selection of buttons and focus to dialog */
    AddDialogMnemonicHandler(form);
    
    /* realize all of the widgets in the new window */
    RealizeWithoutForcingPosition(ucd->dlogShell);
}

/*
** Present a dialogs for editing the user specified commands in the Macro
** and background menus
*/
void EditMacroMenu(WindowInfo *window)
{
    editMacroOrBGMenu(window, MACRO_CMDS);
}
void EditBGMenu(WindowInfo *window)
{
    editMacroOrBGMenu(window, BG_MENU_CMDS);
}

static void editMacroOrBGMenu(WindowInfo *window, int dialogType)
{   
    Widget form, accLabel, pasteReplayBtn;
    Widget nameLabel, cmdLabel, okBtn, applyBtn, dismissBtn;
    userCmdDialog *ucd;
    char *title;
    XmString s1;
    int ac, i;
    Arg args[20];

    /* if the dialog is already displayed, just pop it to the top and return */
    if (dialogType == MACRO_CMDS && MacroCmdDialog != NULL) {
    	RaiseShellWindow(MacroCmdDialog);
    	return;
    }
    if (dialogType == BG_MENU_CMDS && BGMenuCmdDialog != NULL) {
    	RaiseShellWindow(BGMenuCmdDialog);
    	return;
    }

    /* Create a structure for keeping track of dialog state */
    ucd = (userCmdDialog *)XtMalloc(sizeof(userCmdDialog));
    ucd->window = window;

    /* Set the dialog to operate on the Macro menu */
    ucd->menuItemsList = (menuItemRec **)XtMalloc(sizeof(menuItemRec **) *
    	    MAX_ITEMS_PER_MENU);
    if (dialogType == MACRO_CMDS) {
	for (i=0; i<NMacroMenuItems; i++)
    	    ucd->menuItemsList[i] = copyMenuItemRec(MacroMenuItems[i]);
	ucd->nMenuItems = NMacroMenuItems;
    } else { /* BG_MENU_CMDS */
	for (i=0; i<NBGMenuItems; i++)
    	    ucd->menuItemsList[i] = copyMenuItemRec(BGMenuItems[i]);
	ucd->nMenuItems = NBGMenuItems;
    }
    ucd->dialogType = dialogType;
    
    title = dialogType == MACRO_CMDS ? "Macro Commands" :
	    "Window Background Menu";
    ac = 0;
    XtSetArg(args[ac], XmNdeleteResponse, XmDO_NOTHING); ac++;
    XtSetArg(args[ac], XmNiconName, title); ac++;
    XtSetArg(args[ac], XmNtitle, title); ac++;
    ucd->dlogShell = CreateShellWithBestVis(APP_NAME, APP_CLASS,
	    applicationShellWidgetClass, TheDisplay, args, ac);
    AddSmallIcon(ucd->dlogShell);
    form = XtVaCreateManagedWidget("editMacroCommands", xmFormWidgetClass,
	    ucd->dlogShell, XmNautoUnmanage, False,
	    XmNresizePolicy, XmRESIZE_NONE, NULL);
    XtAddCallback(form, XmNdestroyCallback, destroyCB, ucd);
    AddMotifCloseCallback(ucd->dlogShell, dismissCB, ucd);
 
    ac = 0;
    XtSetArg(args[ac], XmNtopAttachment, XmATTACH_POSITION); ac++;
    XtSetArg(args[ac], XmNtopPosition, 2); ac++;
    XtSetArg(args[ac], XmNleftAttachment, XmATTACH_POSITION); ac++;
    XtSetArg(args[ac], XmNleftPosition, LEFT_MARGIN_POS); ac++;
    XtSetArg(args[ac], XmNrightAttachment, XmATTACH_POSITION); ac++;
    XtSetArg(args[ac], XmNrightPosition, LIST_RIGHT-1); ac++;
    XtSetArg(args[ac], XmNbottomAttachment, XmATTACH_POSITION); ac++;
    XtSetArg(args[ac], XmNbottomPosition, MACRO_CMD_TOP); ac++;
    ucd->managedList = CreateManagedList(form, "list", args, ac,
    	(void **)ucd->menuItemsList, &ucd->nMenuItems, MAX_ITEMS_PER_MENU, 20,
    	getDialogDataCB, ucd, setDialogDataCB, ucd, freeItemCB);
    
    ucd->selInpBtn = XtVaCreateManagedWidget("selInpBtn",
	    xmToggleButtonWidgetClass, form,
    	    XmNlabelString, s1=MKSTRING("Requires Selection"),
    	    XmNmnemonic, 'R',
    	    XmNalignment, XmALIGNMENT_BEGINNING,
    	    XmNmarginHeight, 0,
    	    XmNset, False,
	    XmNleftAttachment, XmATTACH_POSITION,
	    XmNleftPosition, LIST_RIGHT,
	    XmNbottomAttachment, XmATTACH_POSITION,
    	    XmNbottomPosition, MACRO_CMD_TOP, NULL);
    XmStringFree(s1);
 
    ucd->mneTextW = XtVaCreateManagedWidget("mne", xmTextWidgetClass, form,
	    XmNcolumns, 1,
	    XmNmaxLength, 1,
	    XmNleftAttachment, XmATTACH_POSITION,
	    XmNleftPosition, RIGHT_MARGIN_POS-21-5,
	    XmNrightAttachment, XmATTACH_POSITION,
	    XmNrightPosition, RIGHT_MARGIN_POS-21,
	    XmNbottomAttachment, XmATTACH_WIDGET,
    	    XmNbottomWidget, ucd->selInpBtn,
	    XmNbottomOffset, 5, NULL);
    RemapDeleteKey(ucd->mneTextW);

    ucd->accTextW = XtVaCreateManagedWidget("acc", xmTextWidgetClass, form,
    	    XmNcolumns, 12,
    	    XmNmaxLength, MAX_ACCEL_LEN-1,
    	    XmNcursorPositionVisible, False,
	    XmNleftAttachment, XmATTACH_POSITION,
	    XmNleftPosition, LIST_RIGHT,
	    XmNrightAttachment, XmATTACH_POSITION,
	    XmNrightPosition, RIGHT_MARGIN_POS-20-10,
	    XmNbottomAttachment, XmATTACH_WIDGET,
    	    XmNbottomWidget, ucd->selInpBtn,
	    XmNbottomOffset, 5, NULL);
    XtAddEventHandler(ucd->accTextW, KeyPressMask, False,
    	    (XtEventHandler)accKeyCB, ucd);
    XtAddCallback(ucd->accTextW, XmNfocusCallback, accFocusCB, ucd);
    XtAddCallback(ucd->accTextW, XmNlosingFocusCallback, accLoseFocusCB, ucd);
 
    accLabel = XtVaCreateManagedWidget("accLabel", xmLabelGadgetClass, form,
    	    XmNlabelString, s1=MKSTRING("Accelerator"),
    	    XmNmnemonic, 'l',
    	    XmNuserData, ucd->accTextW,
    	    XmNalignment, XmALIGNMENT_BEGINNING,
    	    XmNmarginTop, 5,
    	    XmNleftAttachment, XmATTACH_POSITION,
    	    XmNleftPosition, LIST_RIGHT,
    	    XmNrightAttachment, XmATTACH_POSITION,
    	    XmNrightPosition, LIST_RIGHT + 22,
    	    XmNbottomAttachment, XmATTACH_WIDGET,
    	    XmNbottomWidget, ucd->mneTextW, NULL);
    XmStringFree(s1);

    XtVaCreateManagedWidget("mneLabel", xmLabelGadgetClass, form,
    	    XmNlabelString, s1=MKSTRING("Mnemonic"),
    	    XmNmnemonic, 'i',
    	    XmNuserData, ucd->mneTextW,
    	    XmNalignment, XmALIGNMENT_END,
    	    XmNmarginTop, 5,
    	    XmNleftAttachment, XmATTACH_POSITION,
    	    XmNleftPosition, LIST_RIGHT + 22,
    	    XmNrightAttachment, XmATTACH_POSITION,
    	    XmNrightPosition, RIGHT_MARGIN_POS-21,
    	    XmNbottomAttachment, XmATTACH_WIDGET,
    	    XmNbottomWidget, ucd->mneTextW, NULL);
    XmStringFree(s1);

    pasteReplayBtn = XtVaCreateManagedWidget("pasteReplay",
    	    xmPushButtonWidgetClass, form,
    	    XmNlabelString, s1=MKSTRING("Paste Learn/\nReplay Macro"),
    	    XmNmnemonic, 'P',
    	    XmNsensitive, GetReplayMacro() != NULL,
    	    XmNtopAttachment, XmATTACH_OPPOSITE_WIDGET,
	    XmNtopWidget, ucd->accTextW,
     	    XmNleftAttachment, XmATTACH_POSITION,
   	    XmNleftPosition, RIGHT_MARGIN_POS-20,
    	    XmNrightAttachment, XmATTACH_POSITION,
    	    XmNrightPosition, RIGHT_MARGIN_POS,
	    XmNbottomAttachment, XmATTACH_POSITION,
    	    XmNbottomPosition, MACRO_CMD_TOP, NULL);
    XtAddCallback(pasteReplayBtn, XmNactivateCallback,
    	    pasteReplayCB, ucd);
    XmStringFree(s1);
    
    ucd->nameTextW = XtVaCreateManagedWidget("name", xmTextWidgetClass, form,
	    XmNleftAttachment, XmATTACH_POSITION,
	    XmNleftPosition, LIST_RIGHT,
	    XmNrightAttachment, XmATTACH_POSITION,
	    XmNrightPosition, RIGHT_MARGIN_POS,
	    XmNbottomAttachment, XmATTACH_WIDGET,
    	    XmNbottomWidget, accLabel, NULL);
    RemapDeleteKey(ucd->nameTextW);
 
    nameLabel = XtVaCreateManagedWidget("nameLabel", xmLabelGadgetClass, form,
    	    XmNlabelString, s1=MKSTRING("Menu Entry"),
    	    XmNmnemonic, 'y',
    	    XmNuserData, ucd->nameTextW,
    	    XmNalignment, XmALIGNMENT_BEGINNING,
    	    XmNmarginTop, 5,
    	    XmNleftAttachment, XmATTACH_POSITION,
    	    XmNleftPosition, LIST_RIGHT,
    	    XmNbottomAttachment, XmATTACH_WIDGET,
    	    XmNbottomWidget, ucd->nameTextW, NULL);
    XmStringFree(s1);
 
    XtVaCreateManagedWidget("nameNotes", xmLabelGadgetClass, form,
    	    XmNlabelString, s1=MKSTRING("(> for sub-menu, @ language mode)"),
    	    XmNalignment, XmALIGNMENT_END,
    	    XmNmarginTop, 5,
    	    XmNleftAttachment, XmATTACH_WIDGET,
    	    XmNleftWidget, nameLabel,
    	    XmNrightAttachment, XmATTACH_POSITION,
    	    XmNrightPosition, RIGHT_MARGIN_POS,
    	    XmNbottomAttachment, XmATTACH_WIDGET,
    	    XmNbottomWidget, ucd->nameTextW, NULL);
    XmStringFree(s1);

    XtVaCreateManagedWidget("topLabel", xmLabelGadgetClass, form,
    	    XmNlabelString, s1=MKSTRING(
"Select a macro menu item from the list at left.\n\
Select \"New\" to add a new command to the menu."),
	    XmNtopAttachment, XmATTACH_POSITION,
	    XmNtopPosition, 2,
	    XmNleftAttachment, XmATTACH_POSITION,
	    XmNleftPosition, LIST_RIGHT,
	    XmNrightAttachment, XmATTACH_POSITION,
	    XmNrightPosition, RIGHT_MARGIN_POS,
	    XmNbottomAttachment, XmATTACH_WIDGET,
    	    XmNbottomWidget, nameLabel, NULL);
    XmStringFree(s1);
 
    cmdLabel = XtVaCreateManagedWidget("cmdLabel", xmLabelGadgetClass, form,
    	    XmNlabelString, s1=MKSTRING("Macro Command to Execute"),
    	    XmNmnemonic, 'x',
    	    XmNalignment, XmALIGNMENT_BEGINNING,
    	    XmNmarginTop, 5,
    	    XmNtopAttachment, XmATTACH_POSITION,
    	    XmNtopPosition, MACRO_CMD_TOP,
    	    XmNleftAttachment, XmATTACH_POSITION,
    	    XmNleftPosition, LEFT_MARGIN_POS, NULL);
    XmStringFree(s1);

    okBtn = XtVaCreateManagedWidget("ok",xmPushButtonWidgetClass,form,
    	    XmNlabelString, s1=MKSTRING("OK"),
    	    XmNleftAttachment, XmATTACH_POSITION,
    	    XmNleftPosition, 8,
    	    XmNrightAttachment, XmATTACH_POSITION,
    	    XmNrightPosition, 23,
    	    XmNbottomAttachment, XmATTACH_POSITION,
    	    XmNbottomPosition, 99, NULL);
    XtAddCallback(okBtn, XmNactivateCallback, okCB, ucd);
    XmStringFree(s1);

    applyBtn = XtVaCreateManagedWidget("apply",xmPushButtonWidgetClass,form,
    	    XmNlabelString, s1=MKSTRING("Apply"),
    	    XmNmnemonic, 'A',
    	    XmNleftAttachment, XmATTACH_POSITION,
    	    XmNleftPosition, 31,
    	    XmNrightAttachment, XmATTACH_POSITION,
    	    XmNrightPosition, 46,
    	    XmNbottomAttachment, XmATTACH_POSITION,
    	    XmNbottomPosition, 99, NULL);
    XtAddCallback(applyBtn, XmNactivateCallback, applyCB, ucd);
    XmStringFree(s1);

    applyBtn = XtVaCreateManagedWidget("check",xmPushButtonWidgetClass,form,
    	    XmNlabelString, s1=MKSTRING("Check"),
    	    XmNmnemonic, 'C',
    	    XmNleftAttachment, XmATTACH_POSITION,
    	    XmNleftPosition, 54,
    	    XmNrightAttachment, XmATTACH_POSITION,
    	    XmNrightPosition, 69,
    	    XmNbottomAttachment, XmATTACH_POSITION,
    	    XmNbottomPosition, 99, NULL);
    XtAddCallback(applyBtn, XmNactivateCallback, checkCB, ucd);
    XmStringFree(s1);

    dismissBtn = XtVaCreateManagedWidget("dismiss",xmPushButtonWidgetClass,form,
    	    XmNlabelString, s1=MKSTRING("Dismiss"),
    	    XmNleftAttachment, XmATTACH_POSITION,
    	    XmNleftPosition, 77,
    	    XmNrightAttachment, XmATTACH_POSITION,
    	    XmNrightPosition, 92,
    	    XmNbottomAttachment, XmATTACH_POSITION,
    	    XmNbottomPosition, 99, NULL);
    XtAddCallback(dismissBtn, XmNactivateCallback, dismissCB, ucd);
    XmStringFree(s1);
    
    ac = 0;
    XtSetArg(args[ac], XmNeditMode, XmMULTI_LINE_EDIT); ac++;
    XtSetArg(args[ac], XmNtopAttachment, XmATTACH_WIDGET); ac++;
    XtSetArg(args[ac], XmNtopWidget, cmdLabel); ac++;
    XtSetArg(args[ac], XmNleftAttachment, XmATTACH_POSITION); ac++;
    XtSetArg(args[ac], XmNleftPosition, LEFT_MARGIN_POS); ac++;
    XtSetArg(args[ac], XmNrightAttachment, XmATTACH_POSITION); ac++;
    XtSetArg(args[ac], XmNrightPosition, RIGHT_MARGIN_POS); ac++;
    XtSetArg(args[ac], XmNbottomAttachment, XmATTACH_WIDGET); ac++;
    XtSetArg(args[ac], XmNbottomWidget, okBtn); ac++;
    XtSetArg(args[ac], XmNbottomOffset, 5); ac++;
    ucd->cmdTextW = XmCreateScrolledText(form, "name", args, ac);
    XtManageChild(ucd->cmdTextW);
    RemapDeleteKey(ucd->cmdTextW);
    XtVaSetValues(cmdLabel, XmNuserData, ucd->cmdTextW, NULL); /* for mnemonic */
   
    /* Disable text input for the accelerator key widget, let the
       event handler manage it instead */
    disableTextW(ucd->accTextW);

    /* initializs the dialog fields to match "New" list item */
    updateDialogFields(NULL, ucd); 
      
    /* Set initial default button */
    XtVaSetValues(form, XmNdefaultButton, okBtn, NULL);
    XtVaSetValues(form, XmNcancelButton, dismissBtn, NULL);
    
    /* Handle mnemonic selection of buttons and focus to dialog */
    AddDialogMnemonicHandler(form);
    
    /* Make widgets for top level shell and paste-replay buttons available
       to other functions */
    if (dialogType == MACRO_CMDS) {
    	MacroCmdDialog = ucd->dlogShell;
	MacroPasteReplayBtn = pasteReplayBtn;
    } else {
	BGMenuCmdDialog = ucd->dlogShell;
    	BGMenuPasteReplayBtn = pasteReplayBtn;
    }
    
    /* Realize all of the widgets in the new dialog */
    RealizeWithoutForcingPosition(ucd->dlogShell);
}

/*
** Update the Shell, Macro, and window background menus menu of window "window"
** from the currently loaded command descriptions.
*/
void UpdateShellMenu(WindowInfo *window)
{
    updateMenu(window, SHELL_CMDS);
}
void UpdateMacroMenu(WindowInfo *window)
{
    updateMenu(window, MACRO_CMDS);
}
void UpdateBGMenu(WindowInfo *window)
{
    updateMenu(window, BG_MENU_CMDS);
}

/*
** Dim/undim buttons for pasting replay macros into macro and bg menu dialogs
*/
void DimPasteReplayBtns(int sensitive)
{
    if (MacroCmdDialog != NULL)
    	XtSetSensitive(MacroPasteReplayBtn, sensitive);
    if (BGMenuCmdDialog != NULL)
    	XtSetSensitive(BGMenuPasteReplayBtn, sensitive);
}

/*
** Dim/undim user programmable menu items which depend on there being
** a selection in their associated window.
*/
void DimSelectionDepUserMenuItems(WindowInfo *window, int sensitive)
{
#ifndef VMS
    dimSelDepItemsInMenu(window->shellMenuPane, ShellMenuItems,
	    NShellMenuItems, sensitive);
#endif /*VMS*/
    dimSelDepItemsInMenu(window->macroMenuPane, MacroMenuItems,
	    NMacroMenuItems, sensitive);
    dimSelDepItemsInMenu(window->bgMenuPane, BGMenuItems,
	    NBGMenuItems, sensitive);
}

static void dimSelDepItemsInMenu(Widget menuPane, menuItemRec **menuList,
	int nMenuItems, int sensitive)
{
    WidgetList items;
    Widget subMenu;
    XtPointer userData;
    int n, index;
    Cardinal nItems;
    
    XtVaGetValues(menuPane, XmNchildren, &items, XmNnumChildren, &nItems, NULL);
    for (n=0; n<nItems; n++) {
	XtVaGetValues(items[n], XmNuserData, &userData, NULL);
    	if (userData != PERMANENT_MENU_ITEM) {
    	    if (XtClass(items[n]) == xmCascadeButtonWidgetClass) {
	    	XtVaGetValues(items[n], XmNsubMenuId, &subMenu, NULL);
		dimSelDepItemsInMenu(subMenu, menuList, nMenuItems, sensitive);
	    } else {
		index = (int)userData - 10;
		if (index <0 || index >= nMenuItems)
    		    return;
		if (menuList[index]->input == FROM_SELECTION)
		    XtSetSensitive(items[n], sensitive);
	    }
    	}
    }
}

/*
** Harmless kludge for making undo/redo menu items in background menu properly
** sensitive (even though they're programmable) along with real undo item
** in the Edit menu
*/
void SetBGMenuUndoSensitivity(WindowInfo *window, int sensitive)
{
    if (window->bgMenuUndoItem != NULL)
    	XtSetSensitive(window->bgMenuUndoItem, sensitive);
}
void SetBGMenuRedoSensitivity(WindowInfo *window, int sensitive)
{
    if (window->bgMenuRedoItem != NULL)
	XtSetSensitive(window->bgMenuRedoItem, sensitive);
}

/*
** Generate a text string for the preferences file describing the contents
** of the shell cmd list.  This string is not exactly of the form that it
** can be read by LoadShellCmdsString, rather, it is what needs to be written
** to a resource file such that it will read back in that form.
*/
char *WriteShellCmdsString(void)
{
    return writeMenuItemString(ShellMenuItems, NShellMenuItems,
    	    SHELL_CMDS);
}

/*
** Generate a text string for the preferences file describing the contents of
** the macro menu and background menu commands lists.  These strings are not
** exactly of the form that it can be read by LoadMacroCmdsString, rather, it
** is what needs to be written to a resource file such that it will read back
** in that form.
*/
char *WriteMacroCmdsString(void)
{
    return writeMenuItemString(MacroMenuItems, NMacroMenuItems, MACRO_CMDS);
}
char *WriteBGMenuCmdsString(void)
{
    return writeMenuItemString(BGMenuItems, NBGMenuItems, BG_MENU_CMDS);
}

/*
** Read a string representing shell command menu items and add them to the
** internal list used for constructing shell menus
*/
int LoadShellCmdsString(char *inString)
{
    return loadMenuItemString(inString, ShellMenuItems, &NShellMenuItems,
    	    SHELL_CMDS);
}

/*
** Read strings representing macro menu or background menu command menu items
** and add them to the internal lists used for constructing menus
*/
int LoadMacroCmdsString(char *inString)
{
    return loadMenuItemString(inString, MacroMenuItems, &NMacroMenuItems,
    	    MACRO_CMDS);
}
int LoadBGMenuCmdsString(char *inString)
{
    return loadMenuItemString(inString, BGMenuItems, &NBGMenuItems,
    	    BG_MENU_CMDS);
}

/*
** Search through the shell menu and execute the first command with menu item
** name "itemName".  Returns True on successs and False on failure.
*/
#ifndef VMS
int DoNamedShellMenuCmd(WindowInfo *window, char *itemName, int fromMacro)
{
    int i;
    
    for (i=0; i<NShellMenuItems; i++) {
    	if (!strcmp(ShellMenuItems[i]->name, itemName)) {
    	    if (ShellMenuItems[i]->output == TO_SAME_WINDOW &&
    	    	    CheckReadOnly(window))
    	    	return False;
    	    DoShellMenuCmd(window, ShellMenuItems[i]->cmd,
    		ShellMenuItems[i]->input, ShellMenuItems[i]->output,
    		ShellMenuItems[i]->repInput, ShellMenuItems[i]->saveFirst,
    		ShellMenuItems[i]->loadAfter, fromMacro);
    	    return True;
    	}
    }
    return False;
}
#endif /*VMS*/

/*
** Search through the Macro or background menu and execute the first command
** with menu item name "itemName".  Returns True on successs and False on
** failure.
*/
int DoNamedMacroMenuCmd(WindowInfo *window, char *itemName)
{
    int i;
    
    for (i=0; i<NMacroMenuItems; i++) {
    	if (!strcmp(MacroMenuItems[i]->name, itemName)) {
    	    DoMacro(window, MacroMenuItems[i]->cmd, "macro menu command");
    	    return True;
    	}
    }
    return False;
}
int DoNamedBGMenuCmd(WindowInfo *window, char *itemName)
{
    int i;
    
    for (i=0; i<NBGMenuItems; i++) {
    	if (!strcmp(BGMenuItems[i]->name, itemName)) {
    	    DoMacro(window, BGMenuItems[i]->cmd, "background menu macro");
    	    return True;
    	}
    }
    return False;
}

/*
** Update all of the Shell or Macro menus of all editor windows.
*/
static void updateMenus(int menuType)
{
    WindowInfo *w;

    for (w=WindowList; w!=NULL; w=w->next)
	updateMenu(w, menuType);
}

/*
** Updates either the Shell menu or the Macro menu of "window", depending on
** value of "menuType"
*/
static void updateMenu(WindowInfo *window, int menuType)
{
    Widget btn, menuPane, subPane, newSubPane;
    int nListItems, n;
    menuItemRec *f, **itemList;
    menuTreeItem *menuTree;
    int i, nTreeEntries, isDefaultLM;
    char *hierName, *namePtr, *subMenuName, *subSep, *strippedName, *name;
    
    /* Fetch the appropriate menu pane and item list for this menu type */
    if (menuType == SHELL_CMDS) {
    	menuPane = window->shellMenuPane;
    	itemList = ShellMenuItems;
    	nListItems = NShellMenuItems;
    } else if (menuType == MACRO_CMDS) {
    	menuPane = window->macroMenuPane;
    	itemList = MacroMenuItems;
    	nListItems = NMacroMenuItems;
    } else { /* BG_MENU_CMDS */
    	menuPane = window->bgMenuPane;
    	itemList = BGMenuItems;
    	nListItems = NBGMenuItems;
    }
    
    /* Remove all of the existing user commands from the menu */
    removeMenuItems(menuPane);
    
    /* Allocate storage for structures to help find sub-menus */
    menuTree = (menuTreeItem *)XtMalloc(sizeof(menuTreeItem) * nListItems);
    nTreeEntries = 0;
    
    /* Harmless kludge: undo and redo items are marked specially if found
       in the background menu, and used to dim/undim with edit menu */
    window->bgMenuUndoItem = NULL;
    window->bgMenuRedoItem = NULL;
    
    /*
    ** Add items to the list, creating hierarchical sub-menus as necessary,
    ** and skipping items not intended for this language mode
    */
    for (n=0; n<nListItems; n++) {
    	f = itemList[n];
	
	/* Eliminate items meant for other language modes, strip @ sign parts.
	   If the language mode is "*", scan the list for an item with the
	   same name and a language mode specified.  If one is found, skip
	   the item in favor of the exact match. */
	strippedName = findStripLanguageMode(f->name, window->languageMode,
		&isDefaultLM);
    	if (strippedName == NULL)
	    continue;		/* not a valid entry for the language */
	if (isDefaultLM) {
	    for (i=0; i<nListItems; i++) {
		name = findStripLanguageMode(itemList[i]->name,
			window->languageMode, &isDefaultLM);
		if (name!=NULL && !isDefaultLM && !strcmp(name, strippedName)) {
		    XtFree(name); /* item with matching language overrides */
		    break;
		}
		XtFree(name);
	    }
	    if (i != nListItems) {
		XtFree(strippedName);
		continue;
	    }
	}
	
	/* create/find sub-menus, stripping off '>' until item name is
	   reached, then create the menu item */
	namePtr = strippedName;
	subPane = menuPane;
	for (;;) {
	    subSep = strchr(namePtr, '>');
	    if (subSep == NULL) {
		btn = createUserMenuItem(subPane, namePtr, f, n,
			(XtCallbackProc)(menuType == SHELL_CMDS ? shellMenuCB :
			(menuType == MACRO_CMDS ? macroMenuCB : bgMenuCB)),
			(XtPointer)window);
		if (menuType == BG_MENU_CMDS && !strcmp(f->cmd, "undo()\n"))
		    window->bgMenuUndoItem = btn;
		else if (menuType == BG_MENU_CMDS && !strcmp(f->cmd,"redo()\n"))
		    window->bgMenuRedoItem = btn;
		UpdateAccelLockPatch(window->shell, btn);
		break;
	    }
	    hierName = copySubstring(strippedName, subSep - strippedName);
	    newSubPane = findInMenuTree(menuTree, nTreeEntries, hierName);
	    if (newSubPane == NULL) {
		subMenuName = copySubstring(namePtr, subSep - namePtr);
	    	newSubPane = createUserSubMenu(subPane, subMenuName);
		XtFree(subMenuName);
		menuTree[nTreeEntries].name = hierName;
		menuTree[nTreeEntries++].menuPane = newSubPane;
	    } else
		XtFree(hierName);
	    subPane = newSubPane;
	    namePtr = subSep + 1;
	}
	XtFree(strippedName);
    }
    
    /* Free the structure used to keep track of sub-menus durring creation */
    for (i=0; i<nTreeEntries; i++)
	XtFree(menuTree[i].name);
    XtFree((char *)menuTree);
    
    /* Set the proper sensitivity of items which may be dimmed */
    SetBGMenuUndoSensitivity(window, XtIsSensitive(window->undoItem));
    SetBGMenuRedoSensitivity(window, XtIsSensitive(window->redoItem));
    DimSelectionDepUserMenuItems(window, window->buffer->primary.selected);
}

/*
** Find the widget corresponding to a hierarchical menu name (a>b>c...)
*/
static Widget findInMenuTree(menuTreeItem *menuTree, int nTreeEntries,
	char *hierName)
{
    int i;
    
    for (i=0; i<nTreeEntries; i++)
	if (!strcmp(hierName, menuTree[i].name))
	    return menuTree[i].menuPane;
    return NULL;
}

static char *copySubstring(char *string, int length)
{
    char *retStr = XtMalloc(length + 1);
    
    strncpy(retStr, string, length);
    retStr[length] = '\0';
    return retStr;
}

/*
** Look for at signs (@) in the string menuItemName, and match them
** against the current language mode.  If there are no @ signs, just
** return an allocated copy of menuItemName.  If there are @ signs, match
** the following text against languageMode, and return NULL if none match,
** or an allocated copy of menuItemName stripped of @ parts.  If the
** language name is "*", sets isDefaultLM to true.
*/
static char *findStripLanguageMode(char *menuItemName, int languageMode,
	int *isDefaultLM)
{
    char *atPtr, *firstAtPtr, *endPtr;
    int lmNameLen;
    
    atPtr = firstAtPtr = strchr(menuItemName, '@');
    *isDefaultLM = False;
    if (atPtr == NULL)
	return CopyAllocatedString(menuItemName);
    if (!strcmp(atPtr+1, "*")) {
	/* only language is "*": this is for all but language specific macros */
	*isDefaultLM = True;
	return copySubstring(menuItemName, firstAtPtr-menuItemName);
    }
    if (languageMode == PLAIN_LANGUAGE_MODE)
	return NULL;
    for (;;) {
	for(endPtr=atPtr+1; isalnum(*endPtr) || *endPtr=='_' || *endPtr=='-' || 
		*endPtr==' ' || *endPtr=='+' || *endPtr=='$' || *endPtr=='#';
		endPtr++);
	lmNameLen = endPtr-atPtr-1;
	if (!strncmp(LanguageModeName(languageMode), atPtr+1, lmNameLen) &&
		LanguageModeName(languageMode)[lmNameLen] == '\0')
	    return copySubstring(menuItemName, firstAtPtr-menuItemName);
	atPtr = strchr(atPtr+1, '@');
	if (atPtr == NULL)
	    return NULL;
    }
}    	
	
static Widget createUserMenuItem(Widget menuPane, char *name, menuItemRec *f,
	int index, XtCallbackProc cbRtn, XtPointer cbArg)
{
    XmString st1, st2;
    char accText[MAX_ACCEL_LEN], accKeys[MAX_ACCEL_LEN+5];
    Widget btn;
    
    generateAcceleratorString(accText, f->modifiers, f->keysym);
    genAccelEventName(accKeys, f->modifiers, f->keysym);
    btn = XtVaCreateManagedWidget("cmd", xmPushButtonWidgetClass, menuPane, 
    	    XmNlabelString, st1=XmStringCreateSimple(name),
    	    XmNacceleratorText, st2=XmStringCreateSimple(accText),
    	    XmNaccelerator, accKeys,
    	    XmNmnemonic, f->mnemonic,
    	    XmNuserData, index+10, NULL);
    XtAddCallback(btn, XmNactivateCallback, cbRtn, cbArg);
    XmStringFree(st1);
    XmStringFree(st2);
    return btn;
}

/*
** Add a user-defined sub-menu to an established pull-down menu, marking
** it's userData field with TEMPORARY_MENU_ITEM so it can be found and
** removed later if the menu is redefined.  Returns the menu pane of the
** new sub menu.
*/
static Widget createUserSubMenu(Widget parent, char *label)
{
    Widget menu;
    XmString st1;
    static Arg args[1] = {{XmNuserData, (XtArgVal)TEMPORARY_MENU_ITEM}};
   
    menu = CreatePulldownMenu(parent, "userPulldown", args, 1);
    XtVaCreateManagedWidget("userCascade", xmCascadeButtonWidgetClass, parent, 
    	    XmNlabelString, st1=XmStringCreateSimple(label),
    	    XmNsubMenuId, menu, XmNuserData, TEMPORARY_MENU_ITEM, NULL);
    XmStringFree(st1);
    return menu;
}

static void removeMenuItems(Widget menuPane)
{
    WidgetList items, itemList;
    Widget subMenuID;
    XtPointer userData;
    int n;
    Cardinal nItems;
    
    /* Fetch the list of children from the menu pane, and make a copy
       (because the widget alters this list as you delete widgets) */
    XtVaGetValues(menuPane, XmNchildren, &itemList, XmNnumChildren, &nItems,
	    NULL);
    items = (WidgetList)XtMalloc(sizeof(Widget) * nItems);
    memcpy(items, itemList, sizeof(Widget) * nItems);
    
    /* Delete all of the widgets not marked as PERMANENT_MENU_ITEM */
    for (n=0; n<nItems; n++) {
	XtVaGetValues(items[n], XmNuserData, &userData, NULL);
    	if (userData != PERMANENT_MENU_ITEM) {
    	    if (XtClass(items[n]) == xmCascadeButtonWidgetClass) {
		XtVaGetValues(items[n], XmNsubMenuId, &subMenuID, NULL);
		removeMenuItems(subMenuID);
#if XmVersion < 2000  /* Skipping this creates a memory and server resource
		   leak (though both are reclaimed on window closing).  In
		   Motif 2.0 (and beyond?) there is a potential crash during
		   phase 2 widget destruction in "SetCascadeField", and in
		   Motif 1.2 there are free-memory reads.  I would really like
		   to be able to destroy this. */
		XtDestroyWidget(subMenuID);
#endif
	    } else /* remove accel. before destroy or lose it forever */
    		XtVaSetValues(items[n], XmNaccelerator, NULL, NULL);
    	    /* unmanaging before destroying stops parent from displaying */
    	    XtUnmanageChild(items[n]);
    	    XtDestroyWidget(items[n]);
    	}
    }
    XtFree((char *)items);
}

static void dismissCB(Widget w, XtPointer clientData, XtPointer callData)
{
    userCmdDialog *ucd = (userCmdDialog *)clientData;
    
    /* Mark that there's no longer a (macro, bg, or shell) dialog up */
    if (ucd->dialogType == SHELL_CMDS)
    	ShellCmdDialog = NULL;
    else if (ucd->dialogType == MACRO_CMDS)
    	MacroCmdDialog = NULL;
    else
	BGMenuCmdDialog = NULL;

    /* pop down and destroy the dialog (memory for ucd is freed in the
       destroy callback) */
    XtDestroyWidget(ucd->dlogShell);
}

static void okCB(Widget w, XtPointer clientData, XtPointer callData)
{
    userCmdDialog *ucd = (userCmdDialog *)clientData;
    
    /* Read the dialog fields, and update the menus */
    if (!applyDialogChanges(ucd))
    	return;
    
    /* Mark that there's no longer a (macro, bg, or shell) dialog up */
    if (ucd->dialogType == SHELL_CMDS)
    	ShellCmdDialog = NULL;
    else if (ucd->dialogType == MACRO_CMDS)
    	MacroCmdDialog = NULL;
    else
	BGMenuCmdDialog = NULL;

    /* pop down and destroy the dialog (memory for ucd is freed in the
       destroy callback) */
    XtDestroyWidget(ucd->dlogShell);
}

static void applyCB(Widget w, XtPointer clientData, XtPointer callData)
{
    applyDialogChanges((userCmdDialog *)clientData);
}

static void checkCB(Widget w, XtPointer clientData, XtPointer callData)
{
    userCmdDialog *ucd = (userCmdDialog *)clientData;
    
    if (checkMacro(ucd))
	DialogF(DF_INF, ucd->dlogShell, 1, "Macro compiled without error",
	    "Dismiss");
}

static int checkMacro(userCmdDialog *ucd)
{
    menuItemRec *f;
    
    f = readDialogFields(ucd, False);
    if (f == NULL)
	return False;
    if (!checkMacroText(f->cmd, ucd->dlogShell, ucd->cmdTextW)) {
	freeMenuItemRec(f);
	return False;
    }
    return True;
}

static int checkMacroText(char *macro, Widget errorParent, Widget errFocus)
{
    Program *prog;
    char *errMsg, *stoppedAt;

    prog = ParseMacro(macro, &errMsg, &stoppedAt);
    if (prog == NULL) {
	if (errorParent != NULL) {
	    ParseError(errorParent, macro, stoppedAt, "macro", errMsg);
    	    XmTextSetInsertionPosition(errFocus, stoppedAt - macro);
 	    XmProcessTraversal(errFocus, XmTRAVERSE_CURRENT);
	}
	return False;
    }
    FreeProgram(prog);
    if (*stoppedAt != '\0') {
	if (errorParent != NULL) {
	    ParseError(errorParent, macro, stoppedAt,"macro","syntax error");
    	    XmTextSetInsertionPosition(errFocus, stoppedAt - macro);
	    XmProcessTraversal(errFocus, XmTRAVERSE_CURRENT);
	}
	return False;
    }
    return True;
}

static int applyDialogChanges(userCmdDialog *ucd)
{
    int i;
    
    /* Get the current contents of the dialog fields */
    if (!UpdateManagedList(ucd->managedList, True))
    	return False;
    
    /* Test compile the macro */
    if (ucd->dialogType == MACRO_CMDS)
	if (!checkMacro(ucd))
	    return False;
    
    /* Update the menu information */
    if (ucd->dialogType == SHELL_CMDS) {
    	for (i=0; i<NShellMenuItems; i++)
    	    freeMenuItemRec(ShellMenuItems[i]);
    	for (i=0; i<ucd->nMenuItems; i++)
    	    ShellMenuItems[i] = copyMenuItemRec(ucd->menuItemsList[i]);
    	NShellMenuItems = ucd->nMenuItems;
    } else if (ucd->dialogType == MACRO_CMDS) {
    	for (i=0; i<NMacroMenuItems; i++)
    	    freeMenuItemRec(MacroMenuItems[i]);
    	for (i=0; i<ucd->nMenuItems; i++)
    	    MacroMenuItems[i] = copyMenuItemRec(ucd->menuItemsList[i]);
    	NMacroMenuItems = ucd->nMenuItems;
    } else { /* BG_MENU_CMDS */
    	for (i=0; i<NBGMenuItems; i++)
    	    freeMenuItemRec(BGMenuItems[i]);
    	for (i=0; i<ucd->nMenuItems; i++)
    	    BGMenuItems[i] = copyMenuItemRec(ucd->menuItemsList[i]);
    	NBGMenuItems = ucd->nMenuItems;
    }
    
    /* Update the menus themselves in all of the NEdit windows */
    updateMenus(ucd->dialogType);
    
    /* Note that preferences have been changed */
    MarkPrefsChanged();
    return True;
}

static void pasteReplayCB(Widget w, XtPointer clientData, XtPointer callData)
{
    userCmdDialog *ucd = (userCmdDialog *)clientData;
    
    if (GetReplayMacro() == NULL)
    	return;
    
    XmTextInsert(ucd->cmdTextW, XmTextGetInsertionPosition(ucd->cmdTextW),
    	    GetReplayMacro());
}

static void destroyCB(Widget w, XtPointer clientData, XtPointer callData)
{
    userCmdDialog *ucd = (userCmdDialog *)clientData;
    int i;
    
    for (i=0; i<ucd->nMenuItems; i++)
    	freeMenuItemRec(ucd->menuItemsList[i]);
    XtFree((char *)ucd->menuItemsList);
    XtFree((char *)ucd);
}

static void accFocusCB(Widget w, XtPointer clientData, XtPointer callData)
{
    userCmdDialog *ucd = (userCmdDialog *)clientData;

    RemoveDialogMnemonicHandler(XtParent(ucd->accTextW));
}

static void accLoseFocusCB(Widget w, XtPointer clientData, XtPointer callData)
{
    userCmdDialog *ucd = (userCmdDialog *)clientData;

    AddDialogMnemonicHandler(XtParent(ucd->accTextW));
}

static void accKeyCB(Widget w, XtPointer clientData, XKeyEvent *event)
{
    userCmdDialog *ucd = (userCmdDialog *)clientData;
    KeySym keysym = XLookupKeysym(event, 0);
    char outStr[MAX_ACCEL_LEN];
    
    /* Accept only real keys, not modifiers alone */
    if (IsModifierKey(keysym))
    	return;
    
    /* Tab key means go to the next field, don't enter */
    if (keysym == XK_Tab)
    	return;
    
    /* Beep and return if the modifiers are buttons or ones we don't support */
    if (event->state & ~(ShiftMask | LockMask | ControlMask | Mod1Mask |
    		Mod2Mask | Mod3Mask | Mod4Mask | Mod5Mask)) {
	XBell(TheDisplay, 0);
	return;
    }
    
    /* Delete or backspace clears field */
    if (keysym == XK_Delete || keysym == XK_BackSpace) {
    	XmTextSetString(ucd->accTextW, "");
    	return;
    }
    
    /* generate the string to use in the dialog field */
    generateAcceleratorString(outStr, event->state, keysym);

    /* Reject single character accelerators (a very simple way to eliminate
       un-modified letters and numbers)  The goal is give users a clue that
       they're supposed to type the actual keys, not the name.  This scheme
       is not rigorous and still allows accelerators like Comma. */
    if (strlen(outStr) == 1) {
    	XBell(TheDisplay, 0);
	return;
    }
    
    /* fill in the accelerator field in the dialog */
    XmTextSetString(ucd->accTextW, outStr);
}

static void sameOutCB(Widget w, XtPointer clientData, XtPointer callData)
{
    XtSetSensitive(((userCmdDialog *)clientData)->repInpBtn,
    	    XmToggleButtonGetState(w));
}

static void shellMenuCB(Widget w, WindowInfo *window, XtPointer callData) 
{
    XtArgVal userData;
    int index;
    char *params[1];

    /* get the index of the shell command and verify that it's in range */
    XtVaGetValues(w, XmNuserData, &userData, NULL);
    index = userData - 10;
    if (index <0 || index >= NShellMenuItems)
    	return;
    
    params[0] = ShellMenuItems[index]->name;
    XtCallActionProc(window->lastFocus, "shell_menu_command",
    	    ((XmAnyCallbackStruct *)callData)->event, params, 1);
}

static void macroMenuCB(Widget w, WindowInfo *window, XtPointer callData) 
{
    XtArgVal userData;
    int index;
    char *params[1];

    /* Don't allow users to execute a macro command from the menu (or accel)
       if there's already a macro command executing.  NEdit can't handle
       running multiple, independent uncoordinated, macros in the same
       window.  Macros may invoke macro menu commands recursively via the
       macro_menu_command action proc, which is important for being able to
       repeat any operation, and to embed macros within eachother at any
       level, however, a call here with a macro running means that THE USER
       is explicitly invoking another macro via the menu or an accelerator. */
    if (window->macroCmdData != NULL)
	return;
    
    /* get the index of the macro command and verify that it's in range */
    XtVaGetValues(w, XmNuserData, &userData, NULL);
    index = userData - 10;
    if (index <0 || index >= NMacroMenuItems)
    	return;
    
    params[0] = MacroMenuItems[index]->name;
    XtCallActionProc(window->lastFocus, "macro_menu_command",
    	    ((XmAnyCallbackStruct *)callData)->event, params, 1);
}

static void bgMenuCB(Widget w, WindowInfo *window, XtPointer callData) 
{
    XtArgVal userData;
    int index;
    char *params[1];

    /* get the index of the macro command and verify that it's in range */
    XtVaGetValues(w, XmNuserData, &userData, NULL);
    index = userData - 10;
    if (index <0 || index >= NBGMenuItems)
    	return;
    
    params[0] = BGMenuItems[index]->name;
    XtCallActionProc(window->lastFocus, "bg_menu_command",
    	    ((XmAnyCallbackStruct *)callData)->event, params, 1);
}

/*
** Update the name, accelerator, mnemonic, and command fields in the shell
** command or macro dialog to agree with the currently selected item in the
** menu item list.
*/
static void updateDialogFields(menuItemRec *f, userCmdDialog *ucd)
{
    char mneString[2], accString[MAX_ACCEL_LEN];
    
    /* fill in the name, accelerator, mnemonic, and command fields of the
       dialog for the newly selected item, or blank them if "New" is selected */
    if (f == NULL) {
    	XmTextSetString(ucd->nameTextW, "");
	XmTextSetString(ucd->cmdTextW, "");
	XmTextSetString(ucd->accTextW, "");
	XmTextSetString(ucd->mneTextW, "");
	if (ucd->dialogType == SHELL_CMDS) {
	    XmToggleButtonSetState(ucd->selInpBtn, True, True);
	    XmToggleButtonSetState(ucd->sameOutBtn, True, True);
	    XmToggleButtonSetState(ucd->repInpBtn, False, False);
	    XtSetSensitive(ucd->repInpBtn, True);
	    XmToggleButtonSetState(ucd->saveFirstBtn, False, False);
	    XmToggleButtonSetState(ucd->loadAfterBtn, False, False);
	}
    } else {
	mneString[0] = f->mnemonic;
	mneString[1] = '\0';
	generateAcceleratorString(accString, f->modifiers, f->keysym);
	XmTextSetString(ucd->nameTextW, f->name);
	XmTextSetString(ucd->cmdTextW, f->cmd);
	XmTextSetString(ucd->accTextW, accString);
	XmTextSetString(ucd->mneTextW, mneString);
	XmToggleButtonSetState(ucd->selInpBtn, f->input==FROM_SELECTION, False);
	if (ucd->dialogType == SHELL_CMDS) {
	    XmToggleButtonSetState(ucd->winInpBtn, f->input == FROM_WINDOW,
	    	    False);
	    XmToggleButtonSetState(ucd->eitherInpBtn, f->input == FROM_EITHER,
	    	    False);
	    XmToggleButtonSetState(ucd->noInpBtn, f->input == FROM_NONE,
	    	    False);
	    XmToggleButtonSetState(ucd->sameOutBtn, f->output==TO_SAME_WINDOW,
	    	    False);
	    XmToggleButtonSetState(ucd->winOutBtn, f->output==TO_NEW_WINDOW,
	    	    False);
	    XmToggleButtonSetState(ucd->dlogOutBtn, f->output==TO_DIALOG,
	    	    False);
	    XmToggleButtonSetState(ucd->repInpBtn, f->repInput, False);
	    XtSetSensitive(ucd->repInpBtn, f->output==TO_SAME_WINDOW);
	    XmToggleButtonSetState(ucd->saveFirstBtn, f->saveFirst, False);
	    XmToggleButtonSetState(ucd->loadAfterBtn, f->loadAfter, False);
	}
    }
}    

/*
** Read the name, accelerator, mnemonic, and command fields from the shell or
** macro commands dialog into a newly allocated menuItemRec.  Returns a
** pointer to the new menuItemRec structure as the function value, or NULL on
** failure.
*/
static menuItemRec *readDialogFields(userCmdDialog *ucd, int silent)
{
    char *nameText, *cmdText, *mneText, *accText;
    menuItemRec *f;

    nameText = XmTextGetString(ucd->nameTextW);
    if (*nameText == '\0') {
    	if (!silent) {
    	    DialogF(DF_WARN, ucd->dlogShell, 1,
    		    "Please specify a name\nfor the menu item", "Dismiss");
    	    XmProcessTraversal(ucd->nameTextW, XmTRAVERSE_CURRENT);
    	}
    	XtFree(nameText);
    	return NULL;
    }
    if (strchr(nameText, ':')) {
    	if (!silent) {
    	    DialogF(DF_WARN, ucd->dlogShell, 1,
    		    "Menu item names may not\ncontain colon (:) characters",
    		    "Dismiss");
    	    XmProcessTraversal(ucd->nameTextW, XmTRAVERSE_CURRENT);
    	}
    	XtFree(nameText);
    	return NULL;
    }
    cmdText = XmTextGetString(ucd->cmdTextW);
    if (cmdText == NULL || *cmdText == '\0') {
    	if (!silent) {
    	    DialogF(DF_WARN, ucd->dlogShell, 1, "Please specify %s to execute",
    	    	    "Dismiss", ucd->dialogType == SHELL_CMDS ? "shell command" :
    	    	    "macro command(s)");
    	    XmProcessTraversal(ucd->cmdTextW, XmTRAVERSE_CURRENT);
    	}
    	XtFree(nameText);
    	if (cmdText!=NULL)
    	    XtFree(cmdText);
    	return NULL;
    }
    if (ucd->dialogType == MACRO_CMDS || ucd->dialogType == BG_MENU_CMDS) {
    	addTerminatingNewline(&cmdText);
	if (!checkMacroText(cmdText, silent ? NULL : ucd->dlogShell,
		ucd->cmdTextW)) {
	    XtFree(nameText);
	    XtFree(cmdText);
	    return NULL;
	}
    }
    f = (menuItemRec *)XtMalloc(sizeof(menuItemRec));
    f->name = nameText;
    f->cmd = cmdText;
    if ((mneText = XmTextGetString(ucd->mneTextW)) != NULL) {
    	f->mnemonic = mneText==NULL ? '\0' : mneText[0];
    	XtFree(mneText);
    	if (f->mnemonic == ':')		/* colons mess up string parsing */
    	    f->mnemonic = '\0';
    }
    if ((accText = XmTextGetString(ucd->accTextW)) != NULL) {
    	parseAcceleratorString(accText, &f->modifiers, &f->keysym);
    	XtFree(accText);
    }
    if (ucd->dialogType == SHELL_CMDS) {
	if (XmToggleButtonGetState(ucd->selInpBtn))
    	    f->input = FROM_SELECTION;
	else if (XmToggleButtonGetState(ucd->winInpBtn))
    	    f->input = FROM_WINDOW;
	else if (XmToggleButtonGetState(ucd->eitherInpBtn))
    	    f->input = FROM_EITHER;
	else
    	    f->input = FROM_NONE;
	if (XmToggleButtonGetState(ucd->winOutBtn))
    	    f->output = TO_NEW_WINDOW;
	else if (XmToggleButtonGetState(ucd->dlogOutBtn))
    	    f->output = TO_DIALOG;
	else
    	    f->output = TO_SAME_WINDOW;
	f->repInput = XmToggleButtonGetState(ucd->repInpBtn);
	f->saveFirst = XmToggleButtonGetState(ucd->saveFirstBtn);
	f->loadAfter = XmToggleButtonGetState(ucd->loadAfterBtn);
    } else {
    	f->input = XmToggleButtonGetState(ucd->selInpBtn) ? FROM_SELECTION :
		FROM_NONE;
    	f->output = TO_SAME_WINDOW;
    	f->repInput = False;
    	f->saveFirst = False;
    	f->loadAfter = False;
    }
    return f;
}

/*
** Copy a menu item record, and its associated memory
*/
static menuItemRec *copyMenuItemRec(menuItemRec *item)
{
    menuItemRec *newItem;
    
    newItem = (menuItemRec *)XtMalloc(sizeof(menuItemRec));
    *newItem = *item;
    newItem->name = XtMalloc(strlen(item->name)+1);
    strcpy(newItem->name, item->name);
    newItem->cmd = XtMalloc(strlen(item->cmd)+1);
    strcpy(newItem->cmd, item->cmd);
    return newItem;
}

/*
** Free a menu item record, and its associated memory
*/
static void freeMenuItemRec(menuItemRec *item)
{
    XtFree(item->name);
    XtFree(item->cmd);
    XtFree((char *)item);
}

/*
** Callbacks for managed-list operations
*/
static void *getDialogDataCB(void *oldItem, int explicitRequest, int *abort,
    	void *cbArg)
{
    userCmdDialog *ucd = (userCmdDialog *)cbArg;
    menuItemRec *currentFields;

    /* If the dialog is currently displaying the "new" entry and the
       fields are empty, that's just fine */
    if (oldItem == NULL && dialogFieldsAreEmpty(ucd))
    	return NULL;
    
    /* If there are no problems reading the data, just return it */
    currentFields = readDialogFields(ucd, True);
    if (currentFields != NULL)
    	return (void *)currentFields;

    /* If user might not be expecting fields to be read, give more warning */
    if (!explicitRequest) {
	if (DialogF(DF_WARN, ucd->dlogShell, 2,
    		"Discard incomplete entry\nfor current menu item?", "Keep",
    		"Discard") == 2) {
     	    return oldItem == NULL ? NULL :
     	    	    (void *)copyMenuItemRec((menuItemRec *)oldItem);
	}
    }
    
    /* Do readDialogFields again without "silent" mode to display warning(s) */
    readDialogFields(ucd, False);
    *abort = True;
    return NULL;
}


static void setDialogDataCB(void *item, void *cbArg)    
{
    updateDialogFields((menuItemRec *)item, (userCmdDialog *)cbArg);
}

static int dialogFieldsAreEmpty(userCmdDialog *ucd)
{
    return TextWidgetIsBlank(ucd->nameTextW) &&
 	    TextWidgetIsBlank(ucd->cmdTextW) &&
	    TextWidgetIsBlank(ucd->accTextW) &&
	    TextWidgetIsBlank(ucd->mneTextW) &&
	    (ucd->dialogType != SHELL_CMDS || (
	    	XmToggleButtonGetState(ucd->selInpBtn) &&
	    	XmToggleButtonGetState(ucd->sameOutBtn) &&
	    	!XmToggleButtonGetState(ucd->repInpBtn) &&
	    	!XmToggleButtonGetState(ucd->saveFirstBtn) &&
	    	!XmToggleButtonGetState(ucd->loadAfterBtn)));
}   	

static void freeItemCB(void *item)
{
    freeMenuItemRec((menuItemRec *)item);
}

/*
** Gut a text widget of it's ability to process input
*/
static void disableTextW(Widget textW)
{
    static XtTranslations emptyTable = NULL;
    static char *emptyTranslations = "\
    	<EnterWindow>:	enter()\n\
	<Btn1Down>:	grab-focus()\n\
	<Btn1Motion>:	extend-adjust()\n\
	<Btn1Up>:	extend-end()\n\
	Shift<Key>Tab:	prev-tab-group()\n\
	Ctrl<Key>Tab:	next-tab-group()\n\
	<Key>Tab:	next-tab-group()\n\
	<LeaveWindow>:	leave()\n\
	<FocusIn>:	focusIn()\n\
	<FocusOut>:	focusOut()\n\
	<Unmap>:	unmap()\n";

    /* replace the translation table with the slimmed down one above */
    if (emptyTable == NULL)
    	emptyTable = XtParseTranslationTable(emptyTranslations);
    XtVaSetValues(textW, XmNtranslations, emptyTable, NULL);
}

static char *writeMenuItemString(menuItemRec **menuItems, int nItems,
	int listType)
{
    char *outStr, *outPtr, *c, accStr[MAX_ACCEL_LEN];
    menuItemRec *f;
    int i, length;
    
    /* determine the max. amount of memory needed for the returned string
       and allocate a buffer for composing the string */
    length = 0;
    for (i=0; i<nItems; i++) {
    	f = menuItems[i];
    	generateAcceleratorString(accStr, f->modifiers, f->keysym);
    	length += strlen(f->name);
    	length += strlen(accStr);
    	length += strlen(f->cmd) * 4;	/* allow for \n & \\ expansions */
    	length += 21;			/* number of characters added below */
    }
    length++;				/* terminating null */
    outStr = XtMalloc(length);
    
    /* write the string */
    outPtr = outStr;
    *outPtr++ = '\\';
    *outPtr++ = '\n';
    for (i=0; i<nItems; i++) {
    	f = menuItems[i];
    	generateAcceleratorString(accStr, f->modifiers, f->keysym);
    	*outPtr++ = '\t';
    	strcpy(outPtr, f->name);
    	outPtr += strlen(f->name);
    	*outPtr++ = ':';
    	strcpy(outPtr, accStr);
    	outPtr += strlen(accStr);
    	*outPtr++ = ':';
    	if (f->mnemonic != '\0')
    	    *outPtr++ = f->mnemonic;
    	*outPtr++ = ':';
    	if (listType == SHELL_CMDS) {
    	    if (f->input == FROM_SELECTION)
   		*outPtr++ = 'I';
    	    else if (f->input == FROM_WINDOW)
   		*outPtr++ = 'A';
    	    else if (f->input == FROM_EITHER)
   		*outPtr++ = 'E';
    	    if (f->output == TO_DIALOG)
    		*outPtr++ = 'D';
    	    else if (f->output == TO_NEW_WINDOW)
    		*outPtr++ = 'W';
    	    if (f->repInput)
    		*outPtr++ = 'X';
    	    if (f->saveFirst)
    		*outPtr++ = 'S';
    	    if (f->loadAfter)
    		*outPtr++ = 'L';
    	    *outPtr++ = ':';
    	} else {
    	    if (f->input == FROM_SELECTION)
   		*outPtr++ = 'R';
	    *outPtr++ = ':';
    	    *outPtr++ = ' ';
    	    *outPtr++ = '{';
    	}
    	*outPtr++ = '\\';
    	*outPtr++ = 'n';
    	*outPtr++ = '\\';
    	*outPtr++ = '\n';
    	*outPtr++ = '\t';
    	*outPtr++ = '\t';
    	for (c=f->cmd; *c!='\0'; c++) { /* Copy the command string, changing */
    	    if (*c == '\\') {	    	/* backslashes to double backslashes */
    	    	*outPtr++ = '\\';	/* and newlines to backslash-n's,    */
 	    	*outPtr++ = '\\';	/* followed by real newlines and tab */
 	    } else if (*c == '\n') {
 	    	*outPtr++ = '\\';
 	    	*outPtr++ = 'n';
 	    	*outPtr++ = '\\';
 	    	*outPtr++ = '\n';
 	    	*outPtr++ = '\t';
 	    	*outPtr++ = '\t';
 	    } else
 	    	*outPtr++ = *c;
    	}
    	if (listType == MACRO_CMDS || listType == BG_MENU_CMDS) {
    	    if (*(outPtr-1) == '\t') outPtr--;
    	    *outPtr++ = '}';
    	}
    	*outPtr++ = '\\';
    	*outPtr++ = 'n';
    	*outPtr++ = '\\';
    	*outPtr++ = '\n';
    }
    --outPtr;
    *--outPtr = '\0';
    return outStr;
}

static int loadMenuItemString(char *inString, menuItemRec **menuItems,
	int *nItems, int listType)
{
    menuItemRec *f;
    char *cmdStr, *inPtr = inString;
    char *nameStr, accStr[MAX_ACCEL_LEN], mneChar;
    KeySym keysym;
    unsigned int modifiers;
    int i, input, output, saveFirst, loadAfter, repInput;
    int nameLen, accLen, mneLen, cmdLen;
    
    for (;;) {
   	
   	/* remove leading whitespace */
   	while (*inPtr == ' ' || *inPtr == '\t')
   	    inPtr++;
   	
   	/* read name field */
   	nameLen = strcspn(inPtr, ":");
	if (nameLen == 0)
    	    return parseError("no name field");
    	nameStr = XtMalloc(nameLen+1);
    	strncpy(nameStr, inPtr, nameLen);
    	nameStr[nameLen] = '\0';
    	inPtr += nameLen;
	if (*inPtr == '\0')
	    return parseError("end not expected");
	inPtr++;
	
	/* read accelerator field */
	accLen = strcspn(inPtr, ":");
	if (accLen >= MAX_ACCEL_LEN)
	    return parseError("accelerator field too long");
    	strncpy(accStr, inPtr, accLen);
    	accStr[accLen] = '\0';
    	inPtr += accLen;
	if (*inPtr == '\0')
	    return parseError("end not expected");
    	inPtr++;
    	
    	/* read menemonic field */
    	mneLen = strcspn(inPtr, ":");
    	if (mneLen > 1)
    	    return parseError("mnemonic field too long");
    	if (mneLen == 1)
    	    mneChar = *inPtr++;
    	else
    	    mneChar = '\0';
    	inPtr++;
    	if (*inPtr == '\0')
	    return parseError("end not expected");
	
	/* read flags field */
	input = FROM_NONE;
	output = TO_SAME_WINDOW;
	repInput = False;
	saveFirst = False;
	loadAfter = False;
	for (; *inPtr != ':'; inPtr++) {
	    if (listType == SHELL_CMDS) {
		if (*inPtr == 'I')
	    	    input = FROM_SELECTION;
		else if (*inPtr == 'A')
	    	    input = FROM_WINDOW;
		else if (*inPtr == 'E')
	    	    input = FROM_EITHER;
		else if (*inPtr == 'W')
	    	    output = TO_NEW_WINDOW;
		else if (*inPtr == 'D')
	    	    output = TO_DIALOG;
		else if (*inPtr == 'X')
	    	    repInput = True;
		else if (*inPtr == 'S')
	    	    saveFirst = True;
		else if (*inPtr == 'L')
	    	    loadAfter = True;
		else
	    	    return parseError("unreadable flag field");
	    } else {
		if (*inPtr == 'R')
	    	    input = FROM_SELECTION;
		else
	    	    return parseError("unreadable flag field");
	    }
	}
	inPtr++;
	
	/* read command field */
	if (listType == SHELL_CMDS) {
	    if (*inPtr++ != '\n')
		return parseError("command must begin with newline");
   	    while (*inPtr == ' ' || *inPtr == '\t') /* leading whitespace */
   	    	inPtr++;
	    cmdLen = strcspn(inPtr, "\n");
	    if (cmdLen == 0)
    		return parseError("shell command field is empty");
    	    cmdStr = XtMalloc(cmdLen+1);
    	    strncpy(cmdStr, inPtr, cmdLen);
    	    cmdStr[cmdLen] = '\0';
    	    inPtr += cmdLen;
	} else {
	    cmdStr = copyMacroToEnd(&inPtr, nameStr);
	    if (cmdStr == NULL)
	    	return False;
	}
   	while (*inPtr == ' ' || *inPtr == '\t' || *inPtr == '\n')
   	    inPtr++; /* skip trailing whitespace & newline */

    	/* parse the accelerator field */
    	if (!parseAcceleratorString(accStr, &modifiers, &keysym))
    	    return parseError("couldn't read accelerator field");
    	
    	/* create a menu item record */
    	f = (menuItemRec *)XtMalloc(sizeof(menuItemRec));
	f->name = nameStr;
	f->cmd = cmdStr;
	f->mnemonic = mneChar;
	f->modifiers = modifiers;
	f->input = input;
	f->output = output;
	f->repInput = repInput;
	f->saveFirst = saveFirst;
	f->loadAfter = loadAfter;
	f->keysym = keysym;
    	
   	/* add/replace menu record in the list */
   	for (i=0; i < *nItems; i++) {
	    if (!strcmp(menuItems[i]->name, f->name)) {
		freeMenuItemRec(menuItems[i]);
		menuItems[i] = f;
		break;
	    }
	}
	if (i == *nItems)
	    menuItems[(*nItems)++] = f;
    	
    	/* end of string in proper place */
    	if (*inPtr == '\0')
    	    return True;
    }
}

static int parseError(char *message)
{
    fprintf(stderr, "NEdit: Parse error in user defined menu item, %s\n",
    	    message);
    return False;
}

/*
** Create a text string representing an accelerator for the dialog,
** the shellCommands or macroCommands resource, and for the menu item.
*/
static void generateAcceleratorString(char *text, unsigned int modifiers,
	KeySym keysym)
{
    char *shiftStr = "", *lockStr = "", *ctrlStr = "", *altStr = "";
    char *mod2Str  = "", *mod3Str = "", *mod4Str = "", *mod5Str = "";
    char keyName[20];

    /* if there's no accelerator, generate an empty string */
    if (keysym == NoSymbol) {
    	*text = '\0';
    	return;
    }

    /* translate the modifiers into strings */
    if (modifiers & ShiftMask)
    	shiftStr = "Shift+";
    if (modifiers & LockMask)
    	lockStr = "Lock+";
    if (modifiers & ControlMask)
    	ctrlStr = "Ctrl+";
    if (modifiers & Mod1Mask)
    	altStr = "Alt+";
    if (modifiers & Mod2Mask)
        mod2Str = "Mod2+";
    if (modifiers & Mod3Mask)
        mod3Str = "Mod3+";
    if (modifiers & Mod4Mask)
        mod4Str = "Mod4+";
    if (modifiers & Mod5Mask)
        mod5Str = "Mod5+";
    
    /* for a consistent look to the accelerator names in the menus,
       capitalize the first letter of the keysym */
    strcpy(keyName, XKeysymToString(keysym));
    *keyName = toupper(*keyName);
    
    /* concatenate the strings together */
    sprintf(text, "%s%s%s%s%s%s%s%s%s", shiftStr, lockStr, ctrlStr, altStr, 
            mod2Str, mod3Str, mod4Str, mod5Str, keyName);
}

/*
** Create a translation table event description string for the menu
** XmNaccelerator resource.
*/
static void genAccelEventName(char *text, unsigned int modifiers,
	KeySym keysym)
{
    char *shiftStr = "", *lockStr = "", *ctrlStr = "", *altStr  = "";
    char *mod2Str  = "", *mod3Str = "", *mod4Str = "", *mod5Str = "";

    /* if there's no accelerator, generate an empty string */
    if (keysym == NoSymbol) {
    	*text = '\0';
    	return;
    }
    
    /* translate the modifiers into strings */
    if (modifiers & ShiftMask)
    	shiftStr = "Shift ";
    if (modifiers & LockMask)
    	lockStr = "Lock ";
    if (modifiers & ControlMask)
    	ctrlStr = "Ctrl ";
    if (modifiers & Mod1Mask)
    	altStr = "Alt ";
    if (modifiers & Mod2Mask)
    	mod2Str = "Mod2 ";
    if (modifiers & Mod3Mask)
    	mod3Str = "Mod3 ";
    if (modifiers & Mod4Mask)
    	mod4Str = "Mod4 ";
    if (modifiers & Mod5Mask)
    	mod5Str = "Mod5 ";
    
    /* put the modifiers together with the key name */
    sprintf(text, "%s%s%s%s%s%s%s%s<Key>%s", 
            shiftStr, lockStr, ctrlStr, altStr, 
            mod2Str,  mod3Str, mod4Str, mod5Str,
            XKeysymToString(keysym));
}

/*
** Read an accelerator name and put it into the form of a modifier mask
** and a KeySym code.  Returns false if string can't be read
** ... does not handle whitespace in string (look at scanf)
*/
static int parseAcceleratorString(char *string, unsigned int *modifiers,
	KeySym *keysym)
{
    int i, nFields, inputLength = strlen(string);
    char fields[10][MAX_ACCEL_LEN];
    
    /* a blank field means no accelerator */
    if (inputLength == 0) {
    	*modifiers = 0;
    	*keysym = NoSymbol;
    	return True;
    }
    
    /* limit the string length so no field strings will overflow */
    if (inputLength > MAX_ACCEL_LEN)
    	return False;
    
    /* divide the input into '+' separated fields */
    nFields = sscanf(string, "%[^+]+%[^+]+%[^+]+%[^+]+%[^+]+%[^+]+%[^+]+%[^+]+%[^+]+%[^+]",
    	    fields[0], fields[1], fields[2], fields[3], fields[4], fields[5],
    	    fields[6], fields[7], fields[8], fields[9]);
    if (nFields == 0)
    	return False;
    
    /* get the key name from the last field and translate it to a keysym.
       If the name is capitalized, try it lowercase as well, since some
       of the keysyms are "prettied up" by generateAcceleratorString */
    *keysym = XStringToKeysym(fields[nFields-1]);
    if (*keysym == NoSymbol) {
    	*fields[nFields-1] = tolower(*fields[nFields-1]);
    	*keysym = XStringToKeysym(fields[nFields-1]);
    	if (*keysym == NoSymbol)
    	    return False;
    }
    	
    /* parse the modifier names from the rest of the fields */
    *modifiers = 0;
    for (i=0; i<nFields-1; i++) {
    	if (!strcmp(fields[i], "Shift"))
    	    *modifiers |= ShiftMask;
    	else if (!strcmp(fields[i], "Lock"))
    	    *modifiers |= LockMask;
    	else if (!strcmp(fields[i], "Ctrl"))
    	    *modifiers |= ControlMask;
    	/* comparision with "Alt" for compatibility with old .nedit files*/
    	else if (!strcmp(fields[i], "Alt"))
    	    *modifiers |= Mod1Mask;
    	else if (!strcmp(fields[i], "Mod2"))
    	    *modifiers |= Mod2Mask;
    	else if (!strcmp(fields[i], "Mod3"))
    	    *modifiers |= Mod3Mask;
    	else if (!strcmp(fields[i], "Mod4"))
    	    *modifiers |= Mod4Mask;
    	else if (!strcmp(fields[i], "Mod5"))
    	    *modifiers |= Mod5Mask;
    	else
    	    return False;
    }
    
    /* all fields successfully parsed */
    return True;
}

/*
** Scan text from "*inPtr" to the end of macro input (matching brace),
** advancing inPtr, and return macro text as function return value.
**
** This is kind of wastefull in that it throws away the compiled macro,
** to be re-generated from the text as needed, but compile time is
** negligible for most macros.
*/
static char *copyMacroToEnd(char **inPtr, char *itemName)
{
    char *retStr, *errMsg, *stoppedAt, *p, *retPtr;
    Program *prog;
    
    /* Skip over whitespace to find make sure there's a beginning brace
       to anchor the parse (if not, it will take the whole file) */
    *inPtr += strspn(*inPtr, " \t\n");
    if (**inPtr != '{') {
    	ParseError(NULL, *inPtr, *inPtr-1, "macro menu item", "expecting '{'");
    	return NULL;
    }

    /* Parse the input */
    prog = ParseMacro(*inPtr, &errMsg, &stoppedAt);
    if (prog == NULL) {
    	ParseError(NULL, *inPtr, stoppedAt, "macro menu item", errMsg);
    	return NULL;
    }
    FreeProgram(prog);
    
    /* Copy and return the body of the macro, stripping outer braces and
       extra leading tabs added by the writer routine */
    (*inPtr)++;
    *inPtr += strspn(*inPtr, " \t");
    if (**inPtr == '\n') (*inPtr)++;
    if (**inPtr == '\t') (*inPtr)++;
    if (**inPtr == '\t') (*inPtr)++;
    retPtr = retStr = XtMalloc(stoppedAt - *inPtr + 1);
    for (p = *inPtr; p < stoppedAt - 1; p++) {
    	if (!strncmp(p, "\n\t\t", 3)) {
    	    *retPtr++ = '\n';
    	    p += 2;
    	} else
    	    *retPtr++ = *p;
    }
    if (*(retPtr-1) == '\t') retPtr--;
    *retPtr = '\0';
    *inPtr = stoppedAt;
    return retStr;
}

/*
** If "*string" is not terminated with a newline character, reallocate the
** string and add one.  (The macro language requires newline terminators for
** statements, but the text widget doesn't force it like the NEdit text buffer
** does, so this might avoid some confusion.)
*/
static void addTerminatingNewline(char **string)
{
    char *newString;
    int length;
    
    length = strlen(*string);
    if ((*string)[length-1] != '\n') {
    	newString = XtMalloc(length + 2);
    	strcpy(newString, *string);
    	newString[length] = '\n';
    	newString[length+1] = '\0';
    	XtFree(*string);
    	*string = newString;
    }
}
