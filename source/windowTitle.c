static const char CVSID[] = "$Id: windowTitle.c,v 1.2 2002/02/11 21:23:16 arnef Exp $";
/*******************************************************************************
*                                                                              *
* windowTitle.c -- Nirvana Editor window title customization                   *
*                                                                              *
* Copyright (C) 2001, Arne Forlie                                              *
*                                                                              *
* This is free software; you can redistribute it and/or modify it under the    *
* terms of the GNU General Public License as published by the Free Software    *
* Foundation; either version 2 of the License, or (at your option) any later   *
* version.                                                                     *
*                                                                              *
* This software is distributed in the hope that it will be useful, but WITHOUT *
* ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or        *
* FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Lesser General Public License *
* for more details.                                                            *
*                                                                              *
* You should have received a copy of the GNU General Public License along with *
* software; if not, write to the Free Software Foundation, Inc., 59 Temple     *
* Place, Suite 330, Boston, MA  02111-1307 USA                                 *
*                                                                              *
* Nirvana Text Editor                                                          *
* July 31, 2001                                                                *
*                                                                              *
* Written by Arne Forlie, http://arne.forlie.com                               *
*                                                                              *
*******************************************************************************/

#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#ifdef VMS
#include "../util/VMSparam.h"
#else
#ifndef __MVS__
#include <sys/param.h>
#endif
#endif /*VMS*/
#include <string.h>
#include <stdio.h>
#include <Xm/Xm.h>
#include <Xm/SelectioB.h>
#include <Xm/Form.h>
#include <Xm/List.h>
#include <Xm/SeparatoG.h>
#include <Xm/LabelG.h>
#include <Xm/PushBG.h>
#include <Xm/PushB.h>
#include <Xm/ToggleBG.h>
#include <Xm/ToggleB.h>
#include <Xm/RowColumn.h>
#include <Xm/CascadeBG.h>
#include <Xm/Frame.h>
#include <Xm/Text.h>
#include "../util/prefFile.h"
#include "../util/misc.h"
#include "../util/DialogF.h"
#include "../util/utils.h"
#include "../util/fileUtils.h"
#include "../util/clearcase.h"
#include "textBuf.h"
#include "nedit.h"
#include "windowTitle.h"
#include "preferences.h"
#include "help.h"

#define WINDOWTITLE_MAX_LEN 500

/* Customize window title dialog information */
static struct {
    Widget      form;
    Widget      shell;
    WindowInfo* window;
    Widget      formatW;
    Widget      pathW;
    Widget      ccViewTagW;
    Widget      serverNameW;
    Widget      fileChangedW;
    Widget      serverEqualViewW;
    char	filename[MAXPATHLEN];
    char	path[MAXPATHLEN];
    char	viewTag[MAXPATHLEN];
    char	serverName[MAXPATHLEN];
    int         isServer;
    int         filenameSet;
    int         lockReasons;
    int         fileChanged;
} etDialog = {NULL};



static char* removeSequence(char* sourcePtr, char c)
{
    while (*sourcePtr == c) {
        sourcePtr++;
    }
    return(sourcePtr);
}


/*
** Two functions for performing safe insertions into a finite
** size buffer so that we don't get any memory overruns.
*/
static char* safeStrCpy(char* dest, char* destEnd, const char* source)
{
   size_t len = strlen(source);
   if (len <= (destEnd - dest)) {
       strcpy(dest, source);
       return(dest + len);
   }
   else {
       strncpy(dest, source, destEnd - dest);
       *destEnd = '\0';
       return(destEnd);
   }
}

static char* safeCharAdd(char* dest, char* destEnd, char c)
{
   if (destEnd - dest > 0)
   {
      *dest++ = c;
      *dest = '\0';
   }
   return(dest);
}

/*
** Remove empty paranthesis pairs and multiple spaces in a row
** with one space.
** Also remove leading and trailing spaces and dashes.
*/
static void compressWindowTitle(char *title)
{
    /* Compress the title */
    int modified;
    do {
        char *sourcePtr = title;
        char *destPtr   = sourcePtr;
        char c = *sourcePtr++;

        modified = False;

        /* Remove leading spaces and dashes */
        while (c == ' ' || c == '-') {
            c= *sourcePtr++;
        }

        /* Remove empty constructs */
        while (c != '\0') {
            switch (c) {
                /* remove sequences */
                case ' ':
                case '-':
                    sourcePtr = removeSequence(sourcePtr, c);
                    *destPtr++ = c; /* leave one */
                    break;

                /* remove empty paranthesis pairs */
                case '(':
                    if (*sourcePtr == ')') {
                        modified = True;
                        sourcePtr++;
                    }
                    else *destPtr++ = c;
                    sourcePtr = removeSequence(sourcePtr, ' ');
                    break;

                case '[':
                    if (*sourcePtr == ']') {
                        modified = True;
                        sourcePtr++;
                    }
                    else *destPtr++ = c;
                    sourcePtr = removeSequence(sourcePtr, ' ');
                    break;

                case '{':
                    if (*sourcePtr == '}') {
                        modified = True;
                        sourcePtr++;
                    }
                    else *destPtr++ = c;
                    sourcePtr = removeSequence(sourcePtr, ' ');
                    break;
                    
                default:
                    *destPtr++ = c;
                    break;
            }
            c = *sourcePtr++;
            *destPtr = '\0';
        }

        /* Remove trailing spaces and dashes */
        while (destPtr-- > title) {
            if (*destPtr != ' ' && *destPtr != '-')
                break;
            *destPtr = '\0';
        }
    } while (modified == True);
}


/*
** Format the windows title using a printf like formatting string.
** The following flags are recognised:
**  %c    : ClearCase view tag
**  %s    : server name
**  %[n]d : directory, with one optional digit specifying the max number
**          of trailing directory components to display. Skipped components are
**          replaced by an ellipsis (...).
**  %f    : file name
**  %h    : host name
**  %S    : file status
**  %u    : user name
**
**  if the ClearCase view tag and server name are identical, only the first one
**  specified in the formatting string will be displayed. 
*/
char *FormatWindowTitle(const char* filename,
                        const char* path,
                        const char* clearCaseViewTag,
                        const char* serverName,
                        int isServer,
                        int filenameSet,
                        int lockReasons,
                        int fileChanged,
                        const char* titleFormat)
{
    static char title[WINDOWTITLE_MAX_LEN];
    char *titlePtr = title;
    char* titleEnd = title + WINDOWTITLE_MAX_LEN - 1;
    
    
    /* Flags to supress one of these if both are specified and they are identical */
    int serverNameSeen = False;
    int clearCaseViewTagSeen = False;

    *titlePtr = '\0';  /* always start with an empty string */

    while (*titleFormat != '\0' && titlePtr < titleEnd) {
        char c = *titleFormat++;
        if (c == '%') {
            c = *titleFormat++;
            switch (c) {
                case 'c': /* ClearCase view tag */
                    if (clearCaseViewTag != NULL) {
                        if (serverNameSeen == False ||
                            strcmp(serverName, clearCaseViewTag) != 0) {
                            titlePtr = safeStrCpy(titlePtr, titleEnd, clearCaseViewTag);
                            clearCaseViewTagSeen = True;
                        }
                    }
                    break;
                   
                case 's': /* server name */
                    if (isServer && serverName[0] != '\0') { /* only applicable for servers */ 
                        if (clearCaseViewTagSeen == False ||
                            strcmp(serverName, clearCaseViewTag) != 0) {
                            titlePtr = safeStrCpy(titlePtr, titleEnd, serverName);
                            serverNameSeen = True;
                        }
                    }
                    break;
                   
                case 'd': /* directory without any limit to no. of components */
                    if (filenameSet) {
                       titlePtr = safeStrCpy(titlePtr, titleEnd, path);
                    }
                    break;
                    
               case '0': /* directory with limited no. of components */
               case '1':
               case '2':
               case '3':
               case '4':
               case '5':
               case '6':
               case '7':
               case '8':
               case '9':
                   if (*titleFormat == 'd') {
                       int noOfComponents = c - '0';
                       titleFormat++; /* delete the argument */

                       if (filenameSet) {
                           const char* trailingPath = GetTrailingPathComponents(path,
                                                                                noOfComponents);

                           /* prefix with ellipsis if components were skipped */
                           if (trailingPath > path) {
                               titlePtr = safeStrCpy(titlePtr, titleEnd, "...");
                           }
                           titlePtr = safeStrCpy(titlePtr, titleEnd, trailingPath);
                       }
                    }
                    break;
                    
                case 'f': /* file name */
                    titlePtr = safeStrCpy(titlePtr, titleEnd, filename);
                    break;
                    
                case 'h': /* host name */
                    titlePtr = safeStrCpy(titlePtr, titleEnd, GetHostName());
                    break;
                   
                case 'S': /* file status */
                    if (IS_ANY_LOCKED_IGNORING_USER(lockReasons) && fileChanged)
                       titlePtr = safeStrCpy(titlePtr, titleEnd, "read only, modified");
                    else if (IS_ANY_LOCKED_IGNORING_USER(lockReasons))
                       titlePtr = safeStrCpy(titlePtr, titleEnd, "read only");
                    else if (IS_USER_LOCKED(lockReasons) && fileChanged)
                       titlePtr = safeStrCpy(titlePtr, titleEnd, "locked, modified");
                    else if (IS_USER_LOCKED(lockReasons))
                       titlePtr = safeStrCpy(titlePtr, titleEnd, "locked");
                    else if (fileChanged)
                       titlePtr = safeStrCpy(titlePtr, titleEnd, "modified");
                    break;
                    
                case 'u': /* user name */
                    titlePtr = safeStrCpy(titlePtr, titleEnd, GetUserName());
                    break;
                   
                default:
                    titlePtr = safeCharAdd(titlePtr, titleEnd, c);
                    break;
            }
        }
        else {
            titlePtr = safeCharAdd(titlePtr, titleEnd, c);
        }
    }
    
    compressWindowTitle(title);
    
    return(title);
}



/* a utility that sets the values of all toggle buttons */
static void setToggleButtons(void)
{
    XmToggleButtonSetState(etDialog.pathW,
    	    	etDialog.filenameSet == True, False);
    XmToggleButtonSetState(etDialog.fileChangedW,
    	    	etDialog.fileChanged == True, False);

    XmToggleButtonSetState(etDialog.ccViewTagW,
    	    	GetClearCaseViewTag() != NULL, False);
    XmToggleButtonSetState(etDialog.serverNameW,
    	    	etDialog.isServer, False);

    if (GetClearCaseViewTag() != NULL &&
        etDialog.isServer &&
        GetPrefServerName()[0] != '\0' &&  
        strcmp(GetClearCaseViewTag(), GetPrefServerName()) == 0) {
        XmToggleButtonSetState(etDialog.serverEqualViewW,
    	    	    True, False);
     } else {
        XmToggleButtonSetState(etDialog.serverEqualViewW,
    	    	    False, False);
    }
}    

static void formatChangedCB(Widget w, XtPointer clientData, XtPointer callData)
{
    char *format = XmTextGetString(etDialog.formatW);
    int  filenameSet = XmToggleButtonGetState(etDialog.pathW);
    char *title;
    const char* serverName;
    
    if (XmToggleButtonGetState(etDialog.serverEqualViewW)) {
       serverName = etDialog.viewTag;
    } else {
       serverName = XmToggleButtonGetState(etDialog.serverNameW) ?
                                       etDialog.serverName : "";
    }
    title = FormatWindowTitle(
                  etDialog.filename,
                  etDialog.filenameSet == True ?
                                   etDialog.path :
                                   "/a/very/long/path/used/as/example/",
                  XmToggleButtonGetState(etDialog.ccViewTagW) ?
                                   etDialog.viewTag : NULL,
                  serverName,
                  etDialog.isServer,
                  filenameSet,
                  etDialog.lockReasons,
                  XmToggleButtonGetState(etDialog.fileChangedW),
                  format);
    XtVaSetValues(etDialog.shell, XmNtitle, title, NULL);
}


static void ccViewTagCB(Widget w, XtPointer clientData, XtPointer callData)
{
    if (XmToggleButtonGetState(w) == False) {
        XmToggleButtonSetState(etDialog.serverEqualViewW, False, False);
    }
    formatChangedCB(w, clientData, callData);
}

static void serverNameCB(Widget w, XtPointer clientData, XtPointer callData)
{
    if (XmToggleButtonGetState(w) == False) {
        XmToggleButtonSetState(etDialog.serverEqualViewW, False, False);
    }
    etDialog.isServer = XmToggleButtonGetState(w);
    formatChangedCB(w, clientData, callData);
}
      
static void fileChangedCB(Widget w, XtPointer clientData, XtPointer callData)
{        
    etDialog.fileChanged = XmToggleButtonGetState(w);
    formatChangedCB(w, clientData, callData);
}

static void serverEqualViewCB(Widget w, XtPointer clientData, XtPointer callData)
{
    if (XmToggleButtonGetState(w) == True) {
        XmToggleButtonSetState(etDialog.ccViewTagW,  True, False);
        XmToggleButtonSetState(etDialog.serverNameW, True, False);
        etDialog.isServer = True;
    }
    formatChangedCB(w, clientData, callData);
}         


static void okCB(Widget w, XtPointer clientData, XtPointer callData)
{
    char *format = XmTextGetString(etDialog.formatW);

    /* pop down the dialog */
    XtUnmanageChild(etDialog.form);
   
    if (strcmp(format, GetPrefTitleFormat()) != 0) {
        SetPrefTitleFormat(format);
    }
}

static void dismissCB(Widget w, XtPointer clientData, XtPointer callData)
{
    /* pop down the dialog */
    XtUnmanageChild(etDialog.form);
}

static void helpCB(Widget w, XtPointer clientData, XtPointer callData)
{
    Help(etDialog.form, HELP_CUSTOM_TITLE_DIALOG);
}

static void wtDestroyCB(Widget w, XtPointer clientData, XtPointer callData)
{
    etDialog.form = NULL;
}

static void createEditTitleDialog(Widget parent, WindowInfo *window)
{
#define LEFT_MARGIN_POS 1
#define RIGHT_MARGIN_POS 99
#define H_MARGIN 5
    Widget formatForm, buttonForm, formatLbl;
    Widget overrideFrame, overrideForm, overrideBox;
    Widget okBtn, dismissBtn, helpBtn;
    XmString s1;
    Arg args[20];
    int defaultBtnOffset;
    Dimension	shadowThickness;

    int ac = 0;
    XtSetArg(args[ac], XmNautoUnmanage, False); ac++;
    etDialog.form = CreateFormDialog(parent, "customizeTitle", args, ac);
    XtAddCallback(etDialog.form, XmNdestroyCallback, wtDestroyCB, NULL);
    
    etDialog.shell = XtParent(etDialog.form);
    
    formatForm = XtVaCreateManagedWidget("formatForm", xmFormWidgetClass,
	    etDialog.form,
	    XmNleftAttachment, XmATTACH_POSITION,
	    XmNleftPosition, LEFT_MARGIN_POS,
	    XmNtopAttachment, XmATTACH_FORM,
	    XmNtopOffset, H_MARGIN,
	    XmNrightAttachment, XmATTACH_POSITION,
	    XmNrightPosition, RIGHT_MARGIN_POS, NULL);

    formatLbl = XtVaCreateManagedWidget("formatLbl", xmLabelGadgetClass,
    	    formatForm,
    	    XmNlabelString, s1=XmStringCreateSimple("Format"),
    	    XmNmnemonic, 'F',
	    XmNleftAttachment, XmATTACH_FORM,
	    XmNtopAttachment, XmATTACH_FORM,
	    XmNbottomAttachment, XmATTACH_FORM, NULL);
    XmStringFree(s1);
    etDialog.formatW = XtVaCreateManagedWidget("format", xmTextWidgetClass,
    	    formatForm,
	    XmNtopAttachment, XmATTACH_FORM,
	    XmNleftAttachment, XmATTACH_WIDGET,
	    XmNleftWidget, formatLbl,
	    XmNrightAttachment, XmATTACH_FORM,
	    XmNbottomAttachment, XmATTACH_FORM, NULL);
    RemapDeleteKey(etDialog.formatW);
    XtVaSetValues(formatLbl, XmNuserData, etDialog.formatW, NULL);
    XtAddCallback(etDialog.formatW, XmNvalueChangedCallback, formatChangedCB, NULL);

    
    overrideFrame = XtVaCreateManagedWidget("overrideFrame", xmFrameWidgetClass,
            etDialog.form,
	    XmNleftAttachment, XmATTACH_POSITION,
	    XmNleftPosition, LEFT_MARGIN_POS,
	    XmNtopAttachment, XmATTACH_WIDGET,
	    XmNtopWidget, formatForm,
	    XmNtopOffset, H_MARGIN,
	    XmNrightAttachment, XmATTACH_POSITION,
	    XmNrightPosition, RIGHT_MARGIN_POS, NULL);

    overrideForm = XtVaCreateManagedWidget("overrideForm", xmFormWidgetClass,
	    overrideFrame, NULL);
    XtVaCreateManagedWidget("overrideLbl", xmLabelGadgetClass,
            overrideFrame,
    	    XmNlabelString, s1=XmStringCreateSimple("Override current settings for test"),
	    XmNchildType, XmFRAME_TITLE_CHILD,
	    XmNchildHorizontalAlignment, XmALIGNMENT_CENTER, NULL);
    XmStringFree(s1);
    
    overrideBox = XtVaCreateManagedWidget("overrideBox", xmRowColumnWidgetClass,
    	    overrideForm,
    	    XmNorientation, XmVERTICAL,
    	    XmNpacking, XmPACK_TIGHT,
    	    XmNradioBehavior, False,
	    XmNleftAttachment, XmATTACH_POSITION,
    	    XmNleftPosition, LEFT_MARGIN_POS,
	    XmNtopAttachment, XmATTACH_FORM,
	    XmNtopOffset, H_MARGIN, NULL);

    etDialog.pathW = XtVaCreateManagedWidget("pathSet", 
    	    xmToggleButtonWidgetClass, overrideBox,
    	    XmNmarginHeight, 0,
    	    XmNlabelString, s1=XmStringCreateSimple("Test with directory"),
    	    XmNmnemonic, 'p', NULL);
    XtAddCallback(etDialog.pathW, XmNvalueChangedCallback,  formatChangedCB, NULL);
    XmStringFree(s1);

    etDialog.fileChangedW = XtVaCreateManagedWidget("fileChanged", 
    	    xmToggleButtonWidgetClass, overrideBox,
    	    XmNmarginHeight, 0,
    	    XmNlabelString, s1=XmStringCreateSimple("Test with 'File changed' status"),
    	    XmNmnemonic, 'F', NULL);
    XtAddCallback(etDialog.fileChangedW, XmNvalueChangedCallback, fileChangedCB, NULL);
    XmStringFree(s1);
    
    etDialog.ccViewTagW = XtVaCreateManagedWidget("ccViewTagSet", 
    	    xmToggleButtonWidgetClass, overrideBox,
    	    XmNmarginHeight, 0,
    	    XmNlabelString, s1=XmStringCreateSimple("Test with ClearCase view tag"),
    	    XmNset, GetClearCaseViewTag() != NULL,
    	    XmNmnemonic, 'C', NULL);
    XtAddCallback(etDialog.ccViewTagW, XmNvalueChangedCallback, ccViewTagCB, NULL);
    XmStringFree(s1);

    etDialog.serverNameW = XtVaCreateManagedWidget("servernameSet", 
    	    xmToggleButtonWidgetClass, overrideBox,
    	    XmNmarginHeight, 0,
    	    XmNlabelString, s1=XmStringCreateSimple("Test with Server name"),
    	    XmNmnemonic, 'S', NULL);
    XtAddCallback(etDialog.serverNameW, XmNvalueChangedCallback, serverNameCB, NULL);
    XmStringFree(s1);

    etDialog.serverEqualViewW = XtVaCreateManagedWidget("serverEqualView", 
    	    xmToggleButtonWidgetClass, overrideBox,
    	    XmNmarginHeight, 0,
    	    XmNlabelString, s1=XmStringCreateSimple("Test with server name equal to ClearCase view tag"),
    	    XmNmnemonic, 'q', NULL);
    XtAddCallback(etDialog.serverEqualViewW, XmNvalueChangedCallback, serverEqualViewCB, NULL);
    XmStringFree(s1);


    buttonForm = XtVaCreateManagedWidget("buttonForm", xmFormWidgetClass,
	    etDialog.form,
	    XmNleftAttachment, XmATTACH_POSITION,
	    XmNleftPosition, LEFT_MARGIN_POS,
	    XmNtopAttachment, XmATTACH_WIDGET,
	    XmNtopWidget, overrideFrame,
	    XmNtopOffset, H_MARGIN,
	    XmNrightAttachment, XmATTACH_POSITION,
	    XmNrightPosition, RIGHT_MARGIN_POS, NULL);

    okBtn = XtVaCreateManagedWidget("ok", xmPushButtonWidgetClass,
            buttonForm,
    	    XmNhighlightThickness, 2,
            XmNlabelString, s1=XmStringCreateSimple("OK"),
            XmNshowAsDefault, (short)1,
    	    XmNleftAttachment, XmATTACH_POSITION,
    	    XmNleftPosition, 10,
    	    XmNrightAttachment, XmATTACH_POSITION,
    	    XmNrightPosition, 30,
    	    XmNbottomAttachment, XmATTACH_POSITION,
    	    XmNbottomPosition, 99, NULL);
    XtAddCallback(okBtn, XmNactivateCallback, okCB, NULL);
    XmStringFree(s1);
    XtVaGetValues(okBtn, XmNshadowThickness, &shadowThickness, NULL);
    defaultBtnOffset = shadowThickness + 12;

    dismissBtn = XtVaCreateManagedWidget("dismiss", xmPushButtonWidgetClass,
            buttonForm,
    	    XmNhighlightThickness, 2,
    	    XmNlabelString, s1=XmStringCreateSimple("Dismiss"),
    	    XmNleftAttachment, XmATTACH_POSITION,
    	    XmNleftPosition, 40,
    	    XmNrightAttachment, XmATTACH_POSITION,
    	    XmNrightPosition, 60,
    	    XmNbottomAttachment, XmATTACH_POSITION,
    	    XmNbottomPosition, 99 - defaultBtnOffset, NULL);
    XtAddCallback(dismissBtn, XmNactivateCallback, dismissCB, NULL);
    XmStringFree(s1);

    helpBtn = XtVaCreateManagedWidget("help", xmPushButtonWidgetClass,
            buttonForm,
    	    XmNhighlightThickness, 2,
    	    XmNlabelString, s1=XmStringCreateSimple("Help"),
    	    XmNleftAttachment, XmATTACH_POSITION,
    	    XmNleftPosition, 70,
    	    XmNrightAttachment, XmATTACH_POSITION,
    	    XmNrightPosition, 90,
    	    XmNbottomAttachment, XmATTACH_POSITION,
    	    XmNbottomPosition, 99 - defaultBtnOffset, NULL);
    XtAddCallback(helpBtn, XmNactivateCallback, helpCB, NULL);
    XmStringFree(s1);

    /* Set initial default button */
    XtVaSetValues(etDialog.form, XmNdefaultButton, okBtn, NULL);
    XtVaSetValues(etDialog.form, XmNcancelButton, dismissBtn, NULL);

    /* Handle mnemonic selection of buttons and focus to dialog */
    AddDialogMnemonicHandler(etDialog.form, FALSE);
} 




void EditCustomTitleFormat(Widget parent, WindowInfo *window)
{
    /* copy attributes from current window so that we can use as many
     * 'real world' defaults as possible when testing the effect
     * of different formatting strings.
     */
    strcpy(etDialog.path, window->path);
    strcpy(etDialog.filename, window->filename);
    strcpy(etDialog.viewTag, GetClearCaseViewTag() != NULL ?
                             GetClearCaseViewTag() :
                             "viewtag");
    strcpy(etDialog.serverName, IsServer ?
                             GetPrefServerName() :
                             "servername");
    etDialog.isServer    = IsServer;
    etDialog.filenameSet = window->filenameSet;
    etDialog.lockReasons = window->lockReasons;
    etDialog.fileChanged = window->fileChanged;
    etDialog.window      = window;
    
    /* Create the dialog if it doesn't already exist */
    if (etDialog.form == NULL)
        createEditTitleDialog(window->shell, window);
    
    /* If the window is already up, just pop it to the top */
    if (XtIsManaged(etDialog.form)) {
	RaiseShellWindow(XtParent(etDialog.form));

        /* force update of the dialog */
        setToggleButtons();
        formatChangedCB(0, 0, 0);
	return;
    }
    
    /* set initial value of format field */
    XmTextSetString(etDialog.formatW, (char *)GetPrefTitleFormat());
        
    /* force update of the dialog */
    setToggleButtons();
    formatChangedCB(0, 0, 0);
    
    /* put up dialog and wait for user to press ok or cancel */
    ManageDialogCenteredOnPointer(etDialog.form);
}

