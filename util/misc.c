static const char CVSID[] = "$Id: misc.c,v 1.14 2001/02/26 23:38:03 edg Exp $";
/*******************************************************************************
*									       *
* misc.c -- Miscelaneous Motif convenience functions			       *
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
* July 28, 1992								       *
*									       *
* Written by Mark Edel							       *
*									       *
*******************************************************************************/
#include <stdlib.h>
#include <stdarg.h>
#include <ctype.h>
#include <math.h>
#include <stdio.h>
#include <time.h>
#ifdef VMS
#include <types.h>
#include <unixio.h>
#include <file.h>
#endif /*VMS*/
#include <X11/Intrinsic.h>
#include <X11/Xatom.h>
#include <X11/keysym.h>
#include <Xm/Xm.h>
#include <Xm/Label.h>
#include <Xm/LabelG.h>
#include <Xm/ToggleB.h>
#include <Xm/PushB.h>
#include <Xm/Separator.h>
#include <Xm/RowColumn.h>
#include <Xm/CascadeB.h>
#include <Xm/AtomMgr.h>
#include <Xm/Protocols.h>
#include <Xm/Text.h>
#include <Xm/MessageB.h>
#include <Xm/DialogS.h>
#include <Xm/SelectioB.h>
#include <Xm/Form.h>
#include <Xm/FileSB.h>
#include "DialogF.h"
#include "misc.h"

/* math.h on Sun mysteriously excludes strtod and other functions when
   POSIX compliance is turned on */
extern double strtod();

/* structure for passing history-recall data to callbacks */
typedef struct {
    char ***list;
    int *nItems;
    int index;
} histInfo;

typedef Widget (*MotifDialogCreationCall)(Widget, String, ArgList, Cardinal);

/* Maximum size of a history-recall list.  Typically never invoked, since
   user must first make this many entries in the text field, limited for
   safety, to the maximum reasonable number of times user can hit up-arrow
   before carpal tunnel syndrome sets in */
#define HISTORY_LIST_TRIM_TO 1000
#define HISTORY_LIST_MAX 2000

/* Maximum length for a menu accelerator string which can be parsed by
   parseAccelString (how many modifier keys can you hold down at once?) */
#define MAX_ACCEL_LEN 100

/* flags to enable/disable delete key remapping and pointer centered dialogs */
static int RemapDeleteEnabled = True;
static int PointerCenteredDialogsEnabled = False;

/* bitmap and mask for waiting (wrist-watch) cursor */
#define watch_x_hot 7
#define watch_y_hot 7
#define watch_width 16
#define watch_height 16
static unsigned char watch_bits[] = {
   0xe0, 0x07, 0xe0, 0x07, 0xe0, 0x07, 0xe0, 0x07, 0x10, 0x08, 0x08, 0x11,
   0x04, 0x21, 0x04, 0x21, 0xe4, 0x21, 0x04, 0x20, 0x08, 0x10, 0x10, 0x08,
   0xe0, 0x07, 0xe0, 0x07, 0xe0, 0x07, 0xe0, 0x07
};
#define watch_mask_width 16
#define watch_mask_height 16
static unsigned char watch_mask_bits[] = {
   0xf0, 0x0f, 0xf0, 0x0f, 0xf0, 0x0f, 0xf0, 0x0f, 0xf8, 0x1f, 0xfc, 0x3f,
   0xfe, 0x7f, 0xfe, 0x7f, 0xfe, 0x7f, 0xfe, 0x7f, 0xfc, 0x3f, 0xf8, 0x1f,
   0xf0, 0x0f, 0xf0, 0x0f, 0xf0, 0x0f, 0xf0, 0x0f
};

static void addMnemonicGrabs(Widget addTo, Widget w);
static void mnemonicCB(Widget w, XtPointer callData, XKeyEvent *event);
static void findAndActivateMnemonic(Widget w, unsigned int keycode);
static void addAccelGrabs(Widget topWidget, Widget w);
static void addAccelGrab(Widget topWidget, Widget w);
static int parseAccelString(char *string, KeySym *keysym,
	unsigned int *modifiers);
static void lockCB(Widget w, XtPointer callData, XEvent *event,
	Boolean *continueDispatch);
static int findAndActivateAccel(Widget w, unsigned int keyCode,
	unsigned int modifiers, XEvent *event);
static void removeWhiteSpace(char *string);
static int stripCaseCmp(const char *str1, const char *str2);
static void warnHandlerCB(String message);
static void passwdCB(Widget w, char * passTxt, XmTextVerifyCallbackStruct
	*txtVerStr);
static void histDestroyCB(Widget w, XtPointer clientData, XtPointer callData);
static void histArrowKeyEH(Widget w, XtPointer callData, XEvent *event,
	Boolean *continueDispatch);
static Widget addParentVisArgsAndCall(MotifDialogCreationCall callRoutine,
	Widget parent, char *name, ArgList arglist, Cardinal  argcount);

/*
** Set up closeCB to be called when the user selects close from the
** window menu.  The close menu item usually activates f.kill which
** sends a WM_DELETE_WINDOW protocol request for the window.
*/
void AddMotifCloseCallback(Widget shell, XtCallbackProc closeCB, void *arg)
{
    static Atom wmpAtom, dwAtom = 0;
    Display *display = XtDisplay(shell);

    /* deactivate the built in delete response of killing the application */
    XtVaSetValues(shell, XmNdeleteResponse, XmDO_NOTHING, NULL);

    /* add a delete window protocol callback instead */
    if (dwAtom == 0) {
    	wmpAtom = XmInternAtom(display, "WM_PROTOCOLS", TRUE);
    	dwAtom = XmInternAtom(display, "WM_DELETE_WINDOW", TRUE);
    }
    XmAddProtocolCallback(shell, wmpAtom, dwAtom, closeCB, arg);
}

/*
** Motif still generates spurrious passive grab warnings on both IBM and SGI
** This routine suppresses them
*/
void SuppressPassiveGrabWarnings(void)
{
    XtSetWarningHandler(warnHandlerCB);
}

/*
** This routine kludges around the problem of backspace not being mapped
** correctly when Motif is used between a server with a delete key in
** the traditional typewriter backspace position and a client that
** expects a backspace key in that position.  Though there are three
** distinct levels of key re-mapping in effect when a user is running
** a Motif application, none of these is really appropriate or effective
** for eliminating the delete v.s. backspace problem.  Our solution is,
** sadly, to eliminate the forward delete functionality of the delete key
** in favor of backwards delete for both keys.  So as not to prevent the
** user or the application from applying other translation table re-mapping,
** we apply re-map the key as a post-processing step, applied after widget
** creation.  As a result, the re-mapping necessarily becomes embedded
** throughout an application (wherever text widgets are created), and
** within library routines, including the Nirvana utility library.  To
** make this remapping optional, the SetDeleteRemap function provides a
** way for an application to turn this functionality on and off.  It is
** recommended that applications that use this routine provide an
** application resource called remapDeleteKey so savvy users can get
** their forward delete functionality back.
*/
void RemapDeleteKey(Widget w)
{
    static XtTranslations table = NULL;
    static char *translations =
    	"~Shift~Ctrl~Meta~Alt<Key>osfDelete: delete-previous-character()\n";

    if (RemapDeleteEnabled) {
    	if (table == NULL)
    	    table = XtParseTranslationTable(translations);
    	XtOverrideTranslations(w, table);
    }
}

void SetDeleteRemap(int state)
{
    RemapDeleteEnabled = state;
}

/*
** This routine resolves a window manager protocol incompatibility between
** the X toolkit and several popular window managers.  Using this in place
** of XtRealizeWidget will realize the window in a way which allows the
** affected window managers to apply their own placement strategy to the
** window, as opposed to forcing the window to a specific location.
**
** One of the hints in the WM_NORMAL_HINTS protocol, PPlacement, gets set by
** the X toolkit (probably part of the Core or Shell widget) when a shell
** widget is realized to the value stored in the XmNx and XmNy resources of the
** Core widget.  While callers can set these values, there is no "unset" value
** for these resources.  On systems which are more Motif aware, a PPosition
** hint of 0,0, which is the default for XmNx and XmNy, is interpreted as,
** "place this as if no hints were specified".  Unfortunately the fvwm family
** of window managers, which are now some of the most popular, interpret this
** as "place this window at (0,0)".  This routine intervenes between the
** realizing and the mapping of the window to remove the inappropriate
** PPlacement hint.
*/ 
void RealizeWithoutForcingPosition(Widget shell)
{
    XSizeHints *hints = XAllocSizeHints();
    long suppliedHints;
    Boolean mappedWhenManaged;
    
    /* Temporarily set value of XmNmappedWhenManaged
       to stop the window from popping up right away */
    XtVaGetValues(shell, XmNmappedWhenManaged, &mappedWhenManaged, NULL);
    XtVaSetValues(shell, XmNmappedWhenManaged, False, NULL);
    
    /* Realize the widget in unmapped state */
    XtRealizeWidget(shell);
    
    /* Get rid of the incorrect WMNormal hint */
    if (XGetWMNormalHints(XtDisplay(shell), XtWindow(shell), hints,
    	    &suppliedHints)) {
	hints->flags &= ~PPosition;
	XSetWMNormalHints(XtDisplay(shell), XtWindow(shell), hints);
    }
    XFree(hints);
    
    /* Map the widget */
    XtMapWidget(shell);
    
    /* Restore the value of XmNmappedWhenManaged */
    XtVaSetValues(shell, XmNmappedWhenManaged, mappedWhenManaged, NULL);
}

/*
** Older X applications and X servers were mostly designed to operate with
** visual class PseudoColor, because older displays were at most 8 bits
** deep.  Modern X servers, however, usually support 24 bit depth and other
** color models.  Sun (and others?) still sets their default visual to
** 8-bit PseudoColor, because some of their X applications don't work
** properly with the other color models.  The problem with PseudoColor, of
** course, is that users run out of colors in the default colormap, and if
** they install additional colormaps for individual applications, colors
** flash and change weirdly when you change your focus from one application
** to another.
**
** In addition to the poor choice of default, a design flaw in Xt makes it
** impossible even for savvy users to specify the XtNvisual resource to
** switch to a deeper visual.  The problem is that the colormap resource is
** processed independently of the visual resource, and usually results in a
** colormap for the default visual rather than for the user-selected one.
**
** This routine should be called before creating a shell widget, to
** pre-process the visual, depth, and colormap resources, and return the
** proper values for these three resources to be passed to XtAppCreateShell.
** Applications which actually require a particular color model (i.e. for
** doing color table animation or dynamic color assignment) should not use
** this routine.
**
** Note that a consequence of using the "best" as opposed to the default
** visual is that some color resources are still converted with the default
** visual (particularly *background), and these must be avoided by widgets
** which are allowed to handle any visual.
*/
void FindBestVisual(Display *display, char *appName, char *appClass,
	Visual **visual, int *depth, Colormap *colormap)
{
    char rsrcName[256], rsrcClass[256], *valueString, *type, *endPtr;
    XrmValue value;
    int screen = DefaultScreen(display);
    int reqDepth = -1;
    int reqID = -1;
    int reqClass = -1;
    int installColormap = FALSE;
    int maxDepth, bestClass, bestVisual, nVis, i, j;
    XVisualInfo visTemplate, *visList = NULL;
    static Visual *cachedVisual = NULL;
    static Colormap cachedColormap;
    static int cachedDepth = 0;
    int bestClasses[] = {StaticGray, GrayScale, StaticColor, PseudoColor,
    	    DirectColor, TrueColor};

    /* If results have already been computed, just return them */
    if (cachedVisual != NULL) {
	*visual = cachedVisual;
	*depth = cachedDepth;
	*colormap = cachedColormap;
	return;
    }
    
    /* Read the visualID and installColormap resources for the application.
       visualID can be specified either as a number (the visual id as
       shown by xdpyinfo), as a visual class name, or as Best or Default. */
    sprintf(rsrcName,"%s.%s", appName, "visualID");
    sprintf(rsrcClass, "%s.%s", appClass, "VisualID");
    if (XrmGetResource(XtDatabase(display), rsrcName, rsrcClass, &type,
	    &value)) {
	valueString = value.addr;
	reqID = (int)strtol(valueString, &endPtr, 0);
	if (endPtr == valueString) {
	    reqID = -1;
	    if (stripCaseCmp(valueString, "Default"))
		reqID = DefaultVisual(display, screen)->visualid;
	    else if (stripCaseCmp(valueString, "StaticGray"))
		reqClass = StaticGray;
	    else if (stripCaseCmp(valueString, "StaticColor"))
		reqClass = StaticColor;
	    else if (stripCaseCmp(valueString, "TrueColor"))
		reqClass = TrueColor;
	    else if (stripCaseCmp(valueString, "GrayScale"))
		reqClass = GrayScale;
	    else if (stripCaseCmp(valueString, "PseudoColor"))
		reqClass = PseudoColor;
	    else if (stripCaseCmp(valueString, "DirectColor"))
		reqClass = DirectColor;
	    else if (!stripCaseCmp(valueString, "Best"))
		fprintf(stderr, "Invalid visualID resource value\n");
	}
    }
    sprintf(rsrcName,"%s.%s", appName, "installColormap");
    sprintf(rsrcClass, "%s.%s", appClass, "InstallColormap");
    if (XrmGetResource(XtDatabase(display), rsrcName, rsrcClass, &type,
	    &value)) {
	if (stripCaseCmp(value.addr, "Yes") || stripCaseCmp(value.addr, "True"))
	    installColormap = TRUE;
    }
    
    /* Generate a list of visuals to consider.  (Note, vestigial code for
       user-requested visual depth is left in, just in case that function
       might be needed again, but it does nothing)  */
    if (reqID != -1) {
	visTemplate.visualid = reqID;
	visList = XGetVisualInfo(display, VisualIDMask, &visTemplate, &nVis);
	if (visList == NULL)
	    fprintf(stderr, "VisualID resource value not valid\n");
    }
    if (visList == NULL && reqClass != -1 && reqDepth != -1) {
	visTemplate.class = reqClass;
	visTemplate.depth = reqDepth;
    	visList = XGetVisualInfo(display,
		VisualClassMask | VisualDepthMask, &visTemplate, &nVis);
    	if (visList == NULL)
	    fprintf(stderr, "Visual class/depth combination not available\n");
    }
    if (visList == NULL && reqClass != -1) {
 	visTemplate.class = reqClass;
    	visList = XGetVisualInfo(display, VisualClassMask, &visTemplate, &nVis);
    	if (visList == NULL)
	    fprintf(stderr,
		    "Visual Class from resource \"visualID\" not available\n");
    }
    if (visList == NULL && reqDepth != -1) {
	visTemplate.depth = reqDepth;
	visTemplate.depth = reqDepth;
    	visList = XGetVisualInfo(display, VisualDepthMask, &visTemplate, &nVis);
    	if (visList == NULL)
	    fprintf(stderr, "Requested visual depth not available\n");
    }
    if (visList == NULL) {
	visList = XGetVisualInfo(display, VisualNoMask, &visTemplate, &nVis);
	if (visList == NULL) {
	    fprintf(stderr, "Internal Error: no visuals available?\n");
	    *visual = DefaultVisual(display, screen);
	    *depth =  DefaultDepth(display, screen);
    	    *colormap = DefaultColormap(display, screen);
    	    return;
	}
    }
    
    /* Choose among the visuals in the candidate list.  Prefer maximum
       depth first then matching default, then largest value of bestClass
       (I'm not sure whether we actually care about class) */
    maxDepth = 0;
    bestClass = 0;
    bestVisual = 0;
    for (i=0; i < nVis; i++) {
	if (visList[i].depth > maxDepth) {
	    maxDepth = visList[i].depth;
	    bestClass = 0;
	    bestVisual = i;
	}
	if (visList[i].depth == maxDepth) {
	    if (visList[i].visual == DefaultVisual(display, screen))
		bestVisual = i;
	    if (visList[bestVisual].visual != DefaultVisual(display, screen)) {
		for (j = 0; j < XtNumber(bestClasses); j++) {
		    if (visList[i].class == bestClasses[j] && j > bestClass) {
			bestClass = j;
			bestVisual = i;
		    }
		}
	    }
	}
    }
    *visual = cachedVisual = visList[bestVisual].visual;
    *depth = cachedDepth = visList[bestVisual].depth;
    
    /* If the chosen visual is not the default, it needs a colormap allocated */
    if (*visual == DefaultVisual(display, screen) && !installColormap)
	*colormap = cachedColormap = DefaultColormap(display, screen);
    else {
	*colormap = cachedColormap = XCreateColormap(display,
		RootWindow(display, screen), cachedVisual, AllocNone);
	XInstallColormap(display, cachedColormap);
    }
    /* printf("Chose visual with depth %d, class %d, colormap %ld, id 0x%x\n",
	    visList[bestVisual].depth, visList[bestVisual].class,
	    *colormap, cachedVisual->visualid); */
}

/*
** If you want to use a non-default visual with Motif, shells all have to be
** created with that visual, depth, and colormap, even if the parent has them
** set up properly. Substituting these routines, will append visual args copied
** from the parent widget (CreatePopupMenu and CreatePulldownMenu), or from the
** best visual, obtained via FindBestVisual above (CreateShellWithBestVis).
*/
Widget CreateDialogShell(Widget parent, char *name,
	ArgList arglist, Cardinal  argcount)
{
    return addParentVisArgsAndCall(XmCreateDialogShell, parent, name, arglist,
	    argcount);
}
Widget CreatePopupMenu(Widget parent, char *name, ArgList arglist,
	Cardinal argcount)
{
    return addParentVisArgsAndCall(XmCreatePopupMenu, parent, name,
	    arglist, argcount);
}
Widget CreatePulldownMenu(Widget parent, char *name,
	ArgList arglist, Cardinal  argcount)
{
    return addParentVisArgsAndCall(XmCreatePulldownMenu, parent, name, arglist,
	    argcount);
}
Widget CreatePromptDialog(Widget parent, char *name,
	ArgList arglist, Cardinal  argcount)
{
    return addParentVisArgsAndCall(XmCreatePromptDialog, parent, name, arglist,
	    argcount);
}
Widget CreateSelectionDialog(Widget parent, char *name,
	ArgList arglist, Cardinal  argcount)
{
    return addParentVisArgsAndCall(XmCreateSelectionDialog, parent, name,
	    arglist, argcount);
}
Widget CreateFormDialog(Widget parent, char *name,
	ArgList arglist, Cardinal  argcount)
{
    return addParentVisArgsAndCall(XmCreateFormDialog, parent, name, arglist,
	    argcount);
}
Widget CreateFileSelectionDialog(Widget parent, char *name,
	ArgList arglist, Cardinal  argcount)
{
    return addParentVisArgsAndCall(XmCreateFileSelectionDialog, parent, name,
	    arglist, argcount);
}
Widget CreateQuestionDialog(Widget parent, char *name,
	ArgList arglist, Cardinal  argcount)
{
    return addParentVisArgsAndCall(XmCreateQuestionDialog, parent, name,
	    arglist, argcount);
}
Widget CreateMessageDialog(Widget parent, char *name,
	ArgList arglist, Cardinal  argcount)
{
    return addParentVisArgsAndCall(XmCreateMessageDialog, parent, name,
	    arglist, argcount);
}
Widget CreateErrorDialog(Widget parent, char *name,
	ArgList arglist, Cardinal  argcount)
{
    return addParentVisArgsAndCall(XmCreateErrorDialog, parent, name, arglist,
	    argcount);
}
Widget CreateShellWithBestVis(String appName, String appClass, 
	   WidgetClass class, Display *display, ArgList args, Cardinal nArgs)
{
    Visual *visual;
    int depth;
    Colormap colormap;
    ArgList al;
    Cardinal ac = nArgs;
    Widget result;

    FindBestVisual(display, appName, appClass, &visual, &depth, &colormap);
    al = (ArgList)XtMalloc(sizeof(Arg) * (nArgs + 3));
    if (nArgs != 0)
    	memcpy(al, args, sizeof(Arg) * nArgs);
    XtSetArg(al[ac], XtNvisual, visual); ac++;
    XtSetArg(al[ac], XtNdepth, depth); ac++;
    XtSetArg(al[ac], XtNcolormap, colormap); ac++;
    result = XtAppCreateShell(appName, appClass, class, display, al, ac);
    XtFree((char *)al);
    return result;
}

/*
** Calls one of the Motif widget creation routines, splicing in additional
** arguments for visual, colormap, and depth.
*/
static Widget addParentVisArgsAndCall(MotifDialogCreationCall createRoutine,
	Widget parent, char *name, ArgList arglist, Cardinal  argcount)
{
    Visual *visual;
    int depth;
    Colormap colormap;
    ArgList al;
    Cardinal ac = argcount;
    Widget result;
    Widget parentShell = parent;
    
    /* Find the application/dialog/menu shell at the top of the widget
       hierarchy, which has the visual resource being used */
    while (True) {
    	if (XtIsShell(parentShell))
    	    break;
    	if (parentShell == NULL) {
	    fprintf(stderr, "failed to find shell\n");
	    exit(1);
	}
    	parentShell = XtParent(parentShell);
    }

    /* Add the visual, depth, and colormap resources to the argument list */
    XtVaGetValues(parentShell, XtNvisual, &visual, XtNdepth, &depth,
	    XtNcolormap, &colormap, NULL);
    al = (ArgList)XtMalloc(sizeof(Arg) * (argcount + 3));
    if (argcount != 0)
    	memcpy(al, arglist, sizeof(Arg) * argcount);
    XtSetArg(al[ac], XtNvisual, visual); ac++;
    XtSetArg(al[ac], XtNdepth, depth); ac++;
    XtSetArg(al[ac], XtNcolormap, colormap); ac++;
    result = (*createRoutine)(parent, name, al, ac);
    XtFree((char *)al);
    return result;
}

/*
** ManageDialogCenteredOnPointer is used in place of XtManageChild for
** popping up a dialog to enable the dialog to be centered under the
** mouse pointer.  Whether it pops up the dialog centered under the pointer
** or in its default position centered over the parent widget, depends on
** the value set in the SetPointerCenteredDialogs call.
*/ 
void ManageDialogCenteredOnPointer(Widget dialogChild)
{
    Widget shell = XtParent(dialogChild);
    Window root, child;
    unsigned int mask;
    unsigned int width, height, borderWidth, depth;
    int x, y, winX, winY, maxX, maxY;
    Boolean mappedWhenManaged;

    /* If this feature is not enabled, just manage the dialog */
    if (!PointerCenteredDialogsEnabled) {
    	XtManageChild(dialogChild);
    	return;
    }
    
    /* Temporarily set value of XmNmappedWhenManaged
       to stop the dialog from popping up right away */
    XtVaGetValues(shell, XmNmappedWhenManaged, &mappedWhenManaged, NULL);
    XtVaSetValues(shell, XmNmappedWhenManaged, False, NULL);
    
    /* Manage the dialog */
    XtManageChild(dialogChild);

    /* Get the pointer position (x, y) */
    XQueryPointer(XtDisplay(shell), XtWindow(shell), &root, &child,
	    &x, &y, &winX, &winY, &mask);

    /* Translate the pointer position (x, y) into a position for the new
       window that will place the pointer at its center */
    XGetGeometry(XtDisplay(shell), XtWindow(shell), &root, &winX, &winY,
    	    &width, &height, &borderWidth, &depth);
    width += 2 * borderWidth;
    height += 2 * borderWidth;
    x -= width/2;
    y -= height/2;

    /* Ensure that the dialog remains on screen */
    maxX = XtScreen(shell)->width - width;
    maxY = XtScreen(shell)->height - height;
    if (x < 0) x = 0;
    if (x > maxX) x = maxX;
    if (y < 0) y = 0;
    if (y > maxY) y = maxY;

    /* Set desired window position in the DialogShell */
    XtVaSetValues(shell, XmNx, x, XmNy, y, NULL);
    
    /* Map the widget */
    XtMapWidget(shell);
    
    /* Restore the value of XmNmappedWhenManaged */
    XtVaSetValues(shell, XmNmappedWhenManaged, mappedWhenManaged, NULL);
}

/*
** Cause dialogs created by libNUtil.a routines (such as DialogF and
** GetNewFilename), and dialogs which use ManageDialogCenteredOnPointer
** to pop up over the pointer (state = True), or pop up in their default
** positions (state = False)
*/
void SetPointerCenteredDialogs(int state)
{
    PointerCenteredDialogsEnabled = state;
}


/*
** Raise a window to the top and give it the input focus.  Setting input focus
** is important on systems which use explict (rather than pointer) focus.
**
** The X alternatives XMapRaised, and XSetInputFocus both have problems.
** XMapRaised only gives the window the focus if it was initially not visible,
** and XSetInputFocus sets the input focus, but crashes if the window is not
** visible.
**
** This routine should also be used in the case where a dialog is popped up and
** subsequent calls to the dialog function use a flag, or the XtIsManaged, to
** decide whether to create a new instance of the dialog, because on slower
** systems, events can intervene while a dialog is still on its way up and its
** window is still invisible, causing a subtle crash potential if
** XSetInputFocus is used.
*/
void RaiseShellWindow(Widget shell)
{
    RaiseWindow(XtDisplay(shell), XtWindow(shell));
}
void RaiseWindow(Display *display, Window w)
{
    XWindowAttributes winAttr;

    XGetWindowAttributes(display, w, &winAttr);
    if (winAttr.map_state == IsViewable)
    	XSetInputFocus(display, w, RevertToParent, CurrentTime);	
    XMapRaised(display, w);
}

/*
** Add a handler for mnemonics in a dialog (Motif currently only handles
** mnemonics in menus) following the example of M.S. Windows.  To add
** mnemonics to a dialog, set the XmNmnemonic resource, as you would in
** a menu, on push buttons or toggle buttons, and call this function
** when the dialog is fully constructed.  Mnemonics added or changed
** after this call will not be noticed.  To add a mnemonic to a text field
** or list, set the XmNmnemonic resource on the appropriate label and set
** the XmNuserData resource of the label to the widget to get the focus
** when the mnemonic is typed.
*/
void AddDialogMnemonicHandler(Widget dialog)
{
    XtAddEventHandler(dialog, KeyPressMask, False,
    	    (XtEventHandler)mnemonicCB, (XtPointer)0);
    addMnemonicGrabs(dialog, dialog);
}

/*
** Removes the event handler and key-grabs added by AddDialogMnemonicHandler
*/
void RemoveDialogMnemonicHandler(Widget dialog)
{
    XtUngrabKey(dialog, AnyKey, Mod1Mask);
    XtRemoveEventHandler(dialog, KeyPressMask, False,
    	    (XtEventHandler)mnemonicCB, (XtPointer)0);
}

/*
** Patch around Motif's poor handling of menu accelerator keys.  Motif
** does not process menu accelerators when the caps lock or num lock
** keys are engaged.  To enable accelerators in these cases, call this
** routine with the completed menu bar widget as "topMenuContainer", and
** the top level shell widget as "topWidget".  It will add key grabs for
** all of the accelerators it finds in the topMenuContainer menu tree, and
** an event handler which can process dropped accelerator events by (again)
** traversing the menu tree looking for matching accelerators, and invoking
** the appropriate button actions.  Any dynamic additions to the menus
** require a call to UpdateAccelLockPatch to add the additional grabs.
** Unfortunately, these grabs can not be removed.
*/
void AccelLockBugPatch(Widget topWidget, Widget topMenuContainer)
{
    XtAddEventHandler(topWidget, KeyPressMask, False, lockCB, topMenuContainer);
    addAccelGrabs(topWidget, topMenuContainer);
}

/*
** Add additional key grabs for new menu items added to the menus, for
** patching around the Motif Caps/Num Lock problem. "topWidget" must be
** the same widget passed in the original call to AccelLockBugPatch.
*/
void UpdateAccelLockPatch(Widget topWidget, Widget newButton)
{
    addAccelGrab(topWidget, newButton);
}

/*
** PopDownBugPatch
**
** Under some circumstances, popping down a dialog and its parent in
** rapid succession causes a crash.  This routine delays and
** processs events until receiving a ReparentNotify event.
** (I have no idea why a ReparentNotify event occurs at all, but it does
** mark the point where it is safe to destroy or pop down the parent, and
** it might have something to do with the bug.)  There is a failsafe in
** the form of a ~1.5 second timeout in case no ReparentNotify arrives.
** Use this sparingly, only when real crashes are observed, and periodically
** check to make sure that it is still necessary.
*/
void PopDownBugPatch(Widget w)
{
    time_t stopTime;

    stopTime = time(NULL) + 1;
    while (time(NULL) <= stopTime) {
    	XEvent event;
    	XtAppContext context = XtWidgetToApplicationContext(w);
    	XtAppPeekEvent(context, &event);
    	if (event.xany.type == ReparentNotify)
    	    return;
    	XtAppProcessEvent(context, XtIMAll);
    }
}

/*
** Convert a compound string to a C style null terminated string.
** Returned string must be freed by the caller.
*/
char *GetXmStringText(XmString fromString)
{
    XmStringContext context;
    char *text, *toPtr, *toString, *fromPtr;
    XmStringCharSet charset;
    XmStringDirection direction;
    Boolean separator;
    
    /* Malloc a buffer large enough to hold the string.  XmStringLength
       should always be slightly longer than necessary, but won't be
       shorter than the equivalent null-terminated string */ 
    toString = XtMalloc(XmStringLength(fromString));
    
    /* loop over all of the segments in the string, copying each segment
       into the output string and converting separators into newlines */
    XmStringInitContext(&context, fromString);
    toPtr = toString;
    while (XmStringGetNextSegment(context, &text,
    	    &charset, &direction, &separator)) {
    	for (fromPtr=text; *fromPtr!='\0'; fromPtr++)
    	    *toPtr++ = *fromPtr;
    	if (separator)
    	    *toPtr++ = '\n';
    }
    
    /* terminate the string, free the context, and return the string */
    *toPtr++ = '\0';
    XmStringFreeContext(context);
    return toString;
}

/*
** Get the XFontStruct that corresponds to the default (first) font in
** a Motif font list.  Since Motif stores this, it saves us from storing
** it or querying it from the X server.
*/
XFontStruct *GetDefaultFontStruct(XmFontList font)
{
    XFontStruct *fs;
    XmFontContext context;
    XmStringCharSet charset;

    XmFontListInitFontContext(&context, font);
    XmFontListGetNextFont(context, &charset, &fs);
    XmFontListFreeFontContext(context);
    XtFree(charset);
    return fs;
}
   
/*
** Create a string table suitable for passing to XmList widgets
*/
XmString* StringTable(int count, ... )
{
    va_list ap;
    XmString *array;
    int i;
    char *str;

    va_start(ap, count);
    array = (XmString*)XtMalloc((count+1) * sizeof(XmString));
    for(i = 0;  i < count; i++ ) {
    	str = va_arg(ap, char *);
	array[i] = XmStringCreateSimple(str);
    }
    array[i] = (XmString)0;
    va_end(ap);
    return(array);
}

void FreeStringTable(XmString *table)
{
    int i;

    for(i = 0; table[i] != 0; i++)
	XmStringFree(table[i]);
    XtFree((char *)table);
}

/*
** Simulate a button press.  The purpose of this routine is show users what
** is happening when they take an action with a non-obvious side effect,
** such as when a user double clicks on a list item.  The argument is an
** XmPushButton widget to "press"
*/ 
void SimulateButtonPress(Widget widget)
{
    XEvent keyEvent;
    
    memset((char *)&keyEvent, 0, sizeof(XKeyPressedEvent));
    keyEvent.type = KeyPress;
    keyEvent.xkey.serial = 1;
    keyEvent.xkey.send_event = True;
    keyEvent.xkey.display = XtDisplay(widget);
    keyEvent.xkey.window = XtWindow(widget);

    if (XtIsSubclass(widget, xmGadgetClass))
    {
        XtCallActionProc(XtParent(widget), "ManagerGadgetSelect",
                         &keyEvent, NULL, 0);
    }                 
    else
    {
        XtCallActionProc(widget, "ArmAndActivate", &keyEvent, NULL, 0);
    }
}

/*
** Add an item to an already established pull-down or pop-up menu, including
** mnemonics, accelerators and callbacks.
*/
Widget AddMenuItem(Widget parent, char *name, char *label,
			  char mnemonic, char *acc, char *accText,
			  XtCallbackProc callback, void *cbArg)
{
    Widget button;
    XmString st1, st2;
    
    button = XtVaCreateManagedWidget(name, xmPushButtonWidgetClass, parent, 
    	XmNlabelString, st1=XmStringCreateSimple(label),
    	XmNmnemonic, mnemonic,
    	XmNacceleratorText, st2=XmStringCreateSimple(accText),
    	XmNaccelerator, acc, NULL);
    XtAddCallback(button, XmNactivateCallback, callback, cbArg);
    XmStringFree(st1);
    XmStringFree(st2);
    return button;
}

/*
** Add a toggle button item to an already established pull-down or pop-up
** menu, including mnemonics, accelerators and callbacks.
*/
Widget AddMenuToggle(Widget parent, char *name, char *label,
		 	    char mnemonic, char *acc, char *accText,
		  	    XtCallbackProc callback, void *cbArg, int set)
{
    Widget button;
    XmString st1, st2;
    
    button = XtVaCreateManagedWidget(name, xmToggleButtonWidgetClass, parent, 
    	XmNlabelString, st1=XmStringCreateSimple(label),
    	XmNmnemonic, mnemonic,
    	XmNacceleratorText, st2=XmStringCreateSimple(accText),
    	XmNaccelerator, acc,
    	XmNset, set, NULL);
    XtAddCallback(button, XmNvalueChangedCallback, callback, cbArg);
    XmStringFree(st1);
    XmStringFree(st2);
    return button;
}

/*
** Add a separator line to a menu
*/
Widget AddMenuSeparator(Widget parent, char *name)
{
    Widget button;
    
    button = XmCreateSeparator(parent, name, NULL, 0);
    XtManageChild(button);
    return button;
}

/*
** Add a sub-menu to an established pull-down or pop-up menu, including
** mnemonics, accelerators and callbacks.  Returns the menu pane of the
** new sub menu.
*/
Widget AddSubMenu(Widget parent, char *name, char *label, char mnemonic)
{
    Widget menu;
    XmString st1;
    
    menu = CreatePulldownMenu(parent, name, NULL, 0);
    XtVaCreateManagedWidget(name, xmCascadeButtonWidgetClass, parent, 
    	XmNlabelString, st1=XmStringCreateSimple(label),
    	XmNmnemonic, mnemonic,
    	XmNsubMenuId, menu, NULL);
    XmStringFree(st1);
    return menu;
}

/*
** SetIntLabel, SetFloatLabel, SetIntText, SetFloatText
**
** Set the text of a motif label or text widget to show an integer or
** floating number.
*/
void SetIntLabel(Widget label, int value)
{
    char labelString[20];
    XmString s1;
    
    sprintf(labelString, "%d", value);
    s1=XmStringCreateSimple(labelString);
    XtVaSetValues(label, XmNlabelString, s1, NULL);
    XmStringFree(s1);
}
void SetFloatLabel(Widget label, double value)
{
    char labelString[20];
    XmString s1;
    
    sprintf(labelString, "%g", value);
    s1=XmStringCreateSimple(labelString);
    XtVaSetValues(label, XmNlabelString, s1, NULL);
    XmStringFree(s1);
}
void SetIntText(Widget text, int value)
{
    char labelString[20];
    
    sprintf(labelString, "%d", value);
    XmTextSetString(text, labelString);
}
void SetFloatText(Widget text, double value)
{
    char labelString[20];
    
    sprintf(labelString, "%g", value);
    XmTextSetString(text, labelString);
}

/*
** GetIntText, GetFloatText, GetIntTextWarn, GetFloatTextWarn
**
** Get the text of a motif text widget as an integer or floating point number.
** The functions will return TEXT_READ_OK of the value was read correctly.
** If not, they will return either TEXT_IS_BLANK, or TEXT_NOT_NUMBER.  The
** GetIntTextWarn and GetFloatTextWarn will display a dialog warning the
** user that the value could not be read.  The argument fieldName is used
** in the dialog to help the user identify where the problem is.  Set
** warnBlank to true if a blank field is also considered an error.
*/
int GetFloatText(Widget text, double *value)
{
    char *strValue, *endPtr;
    int retVal;

    strValue = XmTextGetString(text);	/* Get Value */
    removeWhiteSpace(strValue);		/* Remove blanks and tabs */
    *value = strtod(strValue, &endPtr);	/* Convert string to double */
    if (strlen(strValue) == 0)		/* String is empty */
	retVal = TEXT_IS_BLANK;
    else if (*endPtr != '\0')		/* Whole string not parsed */
    	retVal = TEXT_NOT_NUMBER;
    else
	retVal = TEXT_READ_OK;
    XtFree(strValue);
    return retVal;
}

int GetIntText(Widget text, int *value)
{
    char *strValue, *endPtr;
    int retVal;

    strValue = XmTextGetString(text);		/* Get Value */
    removeWhiteSpace(strValue);			/* Remove blanks and tabs */
    *value = strtol(strValue, &endPtr, 10);	/* Convert string to long */
    if (strlen(strValue) == 0)			/* String is empty */
	retVal = TEXT_IS_BLANK;
    else if (*endPtr != '\0')			/* Whole string not parsed */
    	retVal = TEXT_NOT_NUMBER;
    else
	retVal = TEXT_READ_OK;
    XtFree(strValue);
    return retVal;
}

int GetFloatTextWarn(Widget text, double *value, char *fieldName, int warnBlank)
{
    int result;
    char *valueStr;
    
    result = GetFloatText(text, value);
    if (result == TEXT_READ_OK || (result == TEXT_IS_BLANK && !warnBlank))
    	return result;
    valueStr = XmTextGetString(text);
    if (result == TEXT_IS_BLANK)
    	DialogF (DF_ERR, text, 1, "Please supply %s value",
    		 "Dismiss", fieldName);
    else /* TEXT_NOT_NUMBER */
    	DialogF (DF_ERR, text, 1,
    		 "Can't read %s value: \"%s\"",
    		 "Dismiss", fieldName, valueStr);
    XtFree(valueStr);
    return result;
}

int GetIntTextWarn(Widget text, int *value, char *fieldName, int warnBlank)
{
    int result;
    char *valueStr;
    
    result = GetIntText(text, value);
    if (result == TEXT_READ_OK || (result == TEXT_IS_BLANK && !warnBlank))
    	return result;
    valueStr = XmTextGetString(text);
    if (result == TEXT_IS_BLANK)
    	DialogF (DF_ERR, text, 1, "Please supply a value for %s",
    		 "Dismiss", fieldName);
    else /* TEXT_NOT_NUMBER */
    	DialogF (DF_ERR, text, 1,
    		 "Can't read integer value \"%s\" in %s",
    		 "Dismiss", valueStr, fieldName);
    XtFree(valueStr);
    return result;
}

int TextWidgetIsBlank(Widget textW)
{
    char *str;
    int retVal;
    
    str = XmTextGetString(textW);
    removeWhiteSpace(str);
    retVal = *str == '\0';
    XtFree(str);
    return retVal;
}

/*
** Turn a multi-line editing text widget into a fake single line text area
** by disabling the translation for Return.  This is a way to give users
** extra space, by allowing wrapping, but still prohibiting newlines.
** (SINGLE_LINE_EDIT mode can't be used, in this case, because it forces
** the widget to be one line high).
*/
void MakeSingleLineTextW(Widget textW)
{
    static XtTranslations noReturnTable = NULL;
    static char *noReturnTranslations = "<Key>Return: activate()\n";
    
    if (noReturnTable == NULL)
    	noReturnTable = XtParseTranslationTable(noReturnTranslations);
    XtOverrideTranslations(textW, noReturnTable);
}

/*
** Add up-arrow/down-arrow recall to a single line text field.  When user
** presses up-arrow, string is cleared and recent entries are presented,
** moving to older ones as each successive up-arrow is pressed.  Down-arrow
** moves to more recent ones, final down-arrow clears the field.  Associated
** routine, AddToHistoryList, makes maintaining a history list easier.
**
** Arguments are the widget, and pointers to the history list and number of
** items, which are expected to change periodically.
*/
void AddHistoryToTextWidget(Widget textW, char ***historyList, int *nItems)
{
    histInfo *histData;
    
    /* create a data structure for passing history info to the callbacks */
    histData = (histInfo *)XtMalloc(sizeof(histInfo));
    histData->list = historyList;
    histData->nItems = nItems;
    histData->index = -1;
    
    /* Add an event handler for handling up/down arrow events */
    XtAddEventHandler(textW, KeyPressMask, False,
    	    (XtEventHandler)histArrowKeyEH, histData);
    
    /* Add a destroy callback for freeing history data structure */
    XtAddCallback(textW, XmNdestroyCallback, histDestroyCB, histData);
}

static void histDestroyCB(Widget w, XtPointer clientData, XtPointer callData)
{
    XtFree((char *)clientData);
}

static void histArrowKeyEH(Widget w, XtPointer callData, XEvent *event,
	Boolean *continueDispatch)
{
    histInfo *histData = (histInfo *)callData;
    KeySym keysym = XLookupKeysym((XKeyEvent *)event, 0);
    
    /* only process up and down arrow keys */
    if (keysym != XK_Up && keysym != XK_Down)
    	return;
    
    /* increment or decrement the index depending on which arrow was pressed */
    histData->index += (keysym == XK_Up) ? 1 : -1;

    /* if the index is out of range, beep, fix it up, and return */
    if (histData->index < -1) {
    	histData->index = -1;
	XBell(XtDisplay(w), 0);
    	return;
    }
    if (histData->index >= *histData->nItems) {
    	histData->index = *histData->nItems - 1;
	XBell(XtDisplay(w), 0);
    	return;
    }
    
    /* Change the text field contents */
    XmTextSetString(w, histData->index == -1 ? "" :
	    (*histData->list)[histData->index]);
}

/*
** Copies a string on to the end of history list, which may be reallocated
** to make room.  If historyList grows beyond its internally set boundary
** for size (HISTORY_LIST_MAX), it is trimmed back to a smaller size
** (HISTORY_LIST_TRIM_TO).  Before adding to the list, checks if the item
** is a duplicate of the last item.  If so, it is not added.
*/	
void AddToHistoryList(char *newItem, char ***historyList, int *nItems)
{
    char **newList;
    int i;
    
    if (*nItems != 0 && !strcmp(newItem, **historyList))
	return;
    if (*nItems == HISTORY_LIST_MAX) {
	for (i=HISTORY_LIST_TRIM_TO; i<HISTORY_LIST_MAX; i++)
	    XtFree((*historyList)[i]);
	*nItems = HISTORY_LIST_TRIM_TO;
    }
    newList = (char **)XtMalloc(sizeof(char *) * (*nItems + 1));
    for (i=0; i < *nItems; i++)
	newList[i+1] = (*historyList)[i];
    if (*nItems != 0 && *historyList != NULL)
	XtFree((char *)*historyList);
    (*nItems)++;
    newList[0] = XtNewString(newItem);
    *historyList = newList;
}

/*
 * PasswordText - routine to add a callback to any text widget so that all
 * 		  text typed by the user is echoed with asterisks, allowing
 *		  a password to be typed in without being seen.
 *
 * parameters: w       - text widget to add the callback to
 *             passTxt - pointer to a string created by caller of this routine.
 *		         **NOTE** The length of this string should be one 
 *			 greater than the maximum specified by XmNmaxLength.
 *			 This string is set to empty just before the callback
 *			 is added.
 */

void PasswordText(Widget w, char *passTxt)
{
    passTxt[0] = '\0';
    XtAddCallback(w, XmNmodifyVerifyCallback, (XtCallbackProc)passwdCB,passTxt);
}

/*
** BeginWait/EndWait
**
** Display/Remove a watch cursor over topCursorWidget and its descendents
*/
void BeginWait(Widget topCursorWidget)
{
    Display *display = XtDisplay(topCursorWidget);
    Pixmap pixmap;
    Pixmap maskPixmap;
    XColor xcolors[2];
    static Cursor  waitCursor = 0;
    
    /* if the watch cursor hasn't been created yet, create it */
    if (!waitCursor) {
	pixmap = XCreateBitmapFromData(display, DefaultRootWindow(display),
		(char *)watch_bits, watch_width, watch_height);

	maskPixmap = XCreateBitmapFromData(display, DefaultRootWindow(display),
		(char *)watch_mask_bits, watch_width, watch_height);

	xcolors[0].pixel = BlackPixelOfScreen(DefaultScreenOfDisplay(display));
	xcolors[1].pixel = WhitePixelOfScreen(DefaultScreenOfDisplay(display));

	XQueryColors(display, DefaultColormapOfScreen(
		DefaultScreenOfDisplay(display)), xcolors, 2);
	waitCursor = XCreatePixmapCursor(display, pixmap, maskPixmap,
		&xcolors[0], &xcolors[1], watch_x_hot, watch_y_hot);
	XFreePixmap(display, pixmap);
	XFreePixmap(display, maskPixmap);
    }

    /* display the cursor */
    XDefineCursor(display, XtWindow(topCursorWidget), waitCursor);
}

void EndWait(Widget topCursorWidget)
{
    XUndefineCursor(XtDisplay(topCursorWidget), XtWindow(topCursorWidget));
}

/*
** Create an X window geometry string from width, height, x, and y values.
** This is a complement to the X routine XParseGeometry, and uses the same
** bitmask values (XValue, YValue, WidthValue, HeightValue, XNegative, and
** YNegative) as defined in <X11/Xutil.h> and documented under XParseGeometry.
** It expects a string of at least MAX_GEOMETRY_STRING_LEN in which to write
** result.  Note that in a geometry string, it is not possible to supply a y
** position without an x position.  Also note that the X/YNegative flags
** mean "add a '-' and negate the value" which is kind of odd.
*/
void CreateGeometryString(char *string, short x, short y,
	short width, short height, int bitmask)
{
    char *ptr = string;
    int nChars;
    
    if (bitmask & WidthValue) {
    	sprintf(ptr, "%d%n", width, &nChars);
	ptr += nChars;
    }
    if (bitmask & HeightValue) {
	sprintf(ptr, "x%d%n", height, &nChars);
	ptr += nChars;
    }
    if (bitmask & XValue) {
	if (bitmask & XNegative)
    	    sprintf(ptr, "-%d%n", -x, &nChars);
	else
    	    sprintf(ptr, "+%d%n", x, &nChars);
	ptr += nChars;
    }
    if (bitmask & YValue) {
	if (bitmask & YNegative)
    	    sprintf(ptr, "-%d%n", -y, &nChars);
	else
    	    sprintf(ptr, "+%d%n", y, &nChars);
	ptr += nChars;
    }
    *ptr = '\0';
}

/*									     */
/* passwdCB: callback routine added by PasswordText routine.  This routine   */
/* 	     echoes each character typed as an asterisk (*) and a few other  */
/* 	     necessary things so that the password typed in is not visible   */
/*									     */
static void passwdCB(Widget w, char * passTxt, XmTextVerifyCallbackStruct
	*txtVerStr)

/* XmTextVerifyCallbackStruct:						      */
/*   int reason;	should be XmCR_MODIFYING_TEXT_VALUE 		      */
/*   XEvent  *event;	points to XEvent that triggered the callback	      */
/*   Boolean doit;	indicates whether action should be performed; setting */
/*			this to false negates the action		      */
/*   long currInsert, 	current position of insert cursor		      */
/*	  newInsert;	position user attempts to position the insert cursor  */
/*   long startPos,	starting position of the text to modify		      */
/*	  endPos;	ending position of the text to modify		      */
/*   XmTextBlock text;							      */

/* XmTextBlock (used to pass text around): 		    		      */
/*   char *ptr;                   points to text to be inserted 	      */
/*   int length;		  Number of bytes (length)		      */
/*   XmTextFormat format;         Representations format		      */

/* XmTextFormat: either FMT8BIT or FMT16BIT */

{
    int numCharsTyped, i, j, pos;

    /* ensure text format is 8-bit characters */
    if (txtVerStr->text->format != FMT8BIT)
	return;

    /* verify assumptions */
/*    if (txtVerStr->endPos < txtVerStr->startPos)
	fprintf(stderr, "User password callback error: endPos < startPos\n");
    if (strlen(passTxt) == 0 && txtVerStr->endPos != 0)
	fprintf(stderr, "User password callback error: no txt, but end != 0\n");

    printf("passTxt = %s, startPos = %d, endPos = %d, txtBlkAddr = %d\n",
	 passTxt, txtVerStr->startPos, txtVerStr->endPos, txtVerStr->text);
    if (txtVerStr->text != NULL && txtVerStr->text->ptr != NULL)
	printf("       string typed = %s, length = %d\n", txtVerStr->text->ptr,
		txtVerStr->text->length);
*/
    /* If necessary, expand/compress passTxt and insert any new text */
    if (txtVerStr->text != NULL && txtVerStr->text->ptr != NULL)
	numCharsTyped = txtVerStr->text->length;
    else
	numCharsTyped = 0;
    /* numCharsTyped = # chars to insert (that user typed) */
    /* j = # chars to expand (+) or compress (-) the password string */
    j = numCharsTyped - (txtVerStr->endPos - txtVerStr->startPos);
    if (j > 0) 				/* expand case: start at ending null  */
	for (pos = strlen(passTxt) + 1; pos >= txtVerStr->endPos; --pos)
	    passTxt[pos+j] = passTxt[pos];
    if (j < 0)				/* compress case */
	for (pos = txtVerStr->startPos + numCharsTyped; 
			     pos <= strlen(passTxt)+1; ++pos)
	    passTxt[pos] = passTxt[pos-j];
    /* then copy text to be inserted into passTxt */
    for (pos = txtVerStr->startPos, i = 0; i < numCharsTyped; ++i) {
	passTxt[pos+i] = *(txtVerStr->text->ptr + i);
	/* Replace text typed by user with asterisks (*) */
	*(txtVerStr->text->ptr + i) = '*';
    }
/*    printf("  Password string now = %s\n", passTxt);  */
}

/*
** Remove the white space (blanks and tabs) from a string
*/
static void removeWhiteSpace(char *string)
{
    char *outPtr = string;
    
    while (TRUE) {
    	if (*string == 0) {
	    *outPtr = 0;
	    return;
    	} else if (*string != ' ' && *string != '\t')
	    *(outPtr++) = *(string++);
	else
	    string++;
    }
}

/*
** Compares two strings and return TRUE if the two strings
** are the same, ignoring whitespace and case differences.
*/
static int stripCaseCmp(const char *str1, const char *str2)
{
    const char *c1, *c2;
    
    for (c1=str1, c2=str2; *c1!='\0' && *c2!='\0'; c1++, c2++) {
	while (*c1 == ' ' || *c1 == '\t')
	    c1++;
	while (*c2 == ' ' || *c2 == '\t')
	    c2++;
    	if (toupper((unsigned char)*c1) != toupper((unsigned char)*c2))
    	    return FALSE;
    }
    return *c1 == '\0' && *c2 == '\0';
}

static void warnHandlerCB(String message)
{
    if (strstr(message, "XtRemoveGrab"))
    	return;
    if (strstr(message, "Attempt to remove non-existant passive grab"))
    	return;
    fputs(message, stderr);
    fputc('\n', stderr);
}

/*
** Part of dialog mnemonic processing.  Search the widget tree under w
** for widgets with mnemonics.  When found, add a passive grab to the
** dialog widget for the mnemonic character, thus directing mnemonic
** events to the dialog widget.
*/
static void addMnemonicGrabs(Widget dialog, Widget w)
{
    char mneString[2];
    WidgetList children;
    Cardinal numChildren;
    int i, isMenu;
    KeySym mnemonic = '\0';
    unsigned char rowColType;
    unsigned int keyCode;
    
    if (XtIsComposite(w)) {
	if (XtClass(w) == xmRowColumnWidgetClass) {
	    XtVaGetValues(w, XmNrowColumnType, &rowColType, NULL);
	    isMenu = rowColType != XmWORK_AREA;
	} else
	    isMenu = False;
	if (!isMenu) {
	    XtVaGetValues(w, XmNchildren, &children, XmNnumChildren,
		    &numChildren, NULL);
	    for (i=0; i<numChildren; i++)
    		addMnemonicGrabs(dialog, children[i]);
    	}
    } else {
	XtVaGetValues(w, XmNmnemonic, &mnemonic, NULL);
	if (mnemonic != '\0') {
	    mneString[0] = mnemonic; mneString[1] = '\0';
	    keyCode = XKeysymToKeycode(XtDisplay(dialog),
	    	    XStringToKeysym(mneString));
	    XtGrabKey(dialog, keyCode, Mod1Mask, True,
		    GrabModeAsync, GrabModeAsync);
	    XtGrabKey(dialog, keyCode, Mod1Mask | LockMask, True,
		    GrabModeAsync, GrabModeAsync);
	    XtGrabKey(dialog, keyCode, Mod1Mask | Mod3Mask, True,
		    GrabModeAsync, GrabModeAsync);
	    XtGrabKey(dialog, keyCode, Mod1Mask | LockMask | Mod3Mask, True,
		    GrabModeAsync, GrabModeAsync);
	}
    }
}

/*
** Callback routine for dialog mnemonic processing.
*/
static void mnemonicCB(Widget w, XtPointer callData, XKeyEvent *event)
{
    findAndActivateMnemonic(w, event->keycode);
}

/*
** Look for a widget in the widget tree w, with a mnemonic matching
** keycode.  When one is found, simulate a button press on that widget
** and give it the keyboard focus.  If the mnemonic is on a label,
** look in the userData field of the label to see if it points to
** another widget, and give that the focus.  This routine is just
** sufficient for NEdit, no doubt it will need to be extended for
** mnemonics on widgets other than just buttons and text fields.
*/
static void findAndActivateMnemonic(Widget w, unsigned int keycode)
{
    WidgetList children;
    Cardinal numChildren;
    int i, isMenu;
    KeySym mnemonic = '\0';
    char mneString[2];
    Widget userData;
    unsigned char rowColType;
    
    if (XtIsComposite(w)) {
	if (XtClass(w) == xmRowColumnWidgetClass) {
	    XtVaGetValues(w, XmNrowColumnType, &rowColType, NULL);
	    isMenu = rowColType != XmWORK_AREA;
	} else
	    isMenu = False;
	if (!isMenu) {
	    XtVaGetValues(w, XmNchildren, &children, XmNnumChildren,
		    &numChildren, NULL);
	    for (i=0; i<numChildren; i++)
    		findAndActivateMnemonic(children[i], keycode);
    	}
    } else {
	XtVaGetValues(w, XmNmnemonic, &mnemonic, NULL);
	if (mnemonic != '\0') {
	    mneString[0] = mnemonic; mneString[1] = '\0';
	    if (XKeysymToKeycode(XtDisplay(XtParent(w)),
	    	    XStringToKeysym(mneString)) == keycode) {
	    	if (XtClass(w) == xmLabelWidgetClass ||
	    		XtClass(w) == xmLabelGadgetClass) {
	    	    XtVaGetValues(w, XmNuserData, &userData, NULL);
	    	    if (userData!=NULL && XtIsWidget(userData))
	    	    	XmProcessTraversal(userData, XmTRAVERSE_CURRENT);
	    	} else {
	    	    XmProcessTraversal(w, XmTRAVERSE_CURRENT);
	    	    SimulateButtonPress(w);
	    	}
	    }
	}
    }
}

/*
** Part of workaround for Motif Caps/Num Lock bug.  Search the widget tree
** under w for widgets with accelerators.  When found, add three passive
** grabs to topWidget, one for the accelerator keysym + modifiers + Caps
** Lock, one for Num Lock, and one for both, thus directing lock +
** accelerator events to topWidget.
*/
static void addAccelGrabs(Widget topWidget, Widget w)
{
    WidgetList children;
    Widget menu;
    Cardinal numChildren;
    int i;
    
    if (XtIsComposite(w)) {
	XtVaGetValues(w, XmNchildren, &children, XmNnumChildren,
		&numChildren, NULL);
	for (i=0; i<numChildren; i++)
    	    addAccelGrabs(topWidget, children[i]);
    } else if (XtClass(w) == xmCascadeButtonWidgetClass) {
	XtVaGetValues(w, XmNsubMenuId, &menu, NULL);
	if (menu != NULL)
	    addAccelGrabs(topWidget, menu);
    } else
	addAccelGrab(topWidget, w);
}

/*
** Grabs the key + modifier defined in the widget's accelerator resource,
** in combination with the Caps Lock and Num Lock accelerators.
*/
static void addAccelGrab(Widget topWidget, Widget w)
{
    char *accelString = NULL;
    KeySym keysym;
    unsigned int modifiers;
    
    XtVaGetValues(w, XmNaccelerator, &accelString, NULL);
    if (accelString == NULL || *accelString == '\0')
	return;
    
    if (!parseAccelString(accelString, &keysym, &modifiers))
	return;
    XtGrabKey(topWidget, XKeysymToKeycode(XtDisplay(topWidget), keysym),
	    modifiers | LockMask, True, GrabModeAsync, GrabModeAsync);
    XtGrabKey(topWidget, XKeysymToKeycode(XtDisplay(topWidget), keysym),
	    modifiers | Mod3Mask, True, GrabModeAsync, GrabModeAsync);
    XtGrabKey(topWidget, XKeysymToKeycode(XtDisplay(topWidget), keysym),
	    modifiers | LockMask | Mod3Mask, True, GrabModeAsync, GrabModeAsync);
}

/*
** Read a Motif accelerator string and translate it into a keysym + modifiers.
** Returns TRUE if the parse was successful, FALSE, if not.
*/
static int parseAccelString(char *string, KeySym *keySym,
	unsigned int *modifiers)
{
#define N_MODIFIERS 12
    /*... Is NumLock always Mod3? */
    static char *modifierNames[N_MODIFIERS] = {"Ctrl", "Shift", "Alt", "Mod2",
	    "Mod4", "Mod5", "Button1", "Button2", "Button3", "Button4",
	    "Button5"};
    static unsigned int modifierMasks[N_MODIFIERS] = {ControlMask, ShiftMask,
	    Mod1Mask, Mod2Mask, Mod4Mask, Mod5Mask, Button1Mask, Button2Mask,
	    Button3Mask, Button4Mask, Button5Mask};
    char modStr[MAX_ACCEL_LEN];
    char evtStr[MAX_ACCEL_LEN];
    char keyStr[MAX_ACCEL_LEN];
    char *c, *evtStart, *keyStart;
    int i;
    
    if (strlen(string) >= MAX_ACCEL_LEN)
	return FALSE;
    
    /* Get the modifier part */
    for (c = string; *c != '<'; c++)
	if (*c == '\0')
	    return FALSE;
    strncpy(modStr, string, c - string);
    modStr[c - string] = '\0';
    
    /* Verify the <key> or <keypress> part */
    evtStart = c;
    for ( ; *c != '>'; c++)
	if (*c == '\0')
	    return FALSE;
    c++;
    strncpy(evtStr, evtStart, c - evtStart);
    evtStr[c - evtStart] = '\0';
    if (!stripCaseCmp(evtStr, "<key>") && !stripCaseCmp(evtStr, "<keypress>"))
	return FALSE;
    
    /* Get the keysym part */
    keyStart = c;
    for ( ; *c != '\0'; c++);
    strncpy(keyStr, keyStart, c - keyStart);
    keyStr[c - keyStart] = '\0';
    *keySym = XStringToKeysym(keyStr);
    
    /* Parse the modifier part */
    *modifiers = 0;
    c = modStr;
    while (*c != '\0') {
	while (*c == ' ' || *c == '\t')
	    c++;
	if (*c == '\0')
	    break;
	for (i = 0; i < N_MODIFIERS; i++) {
	    if (!strncmp(c, modifierNames[i], strlen(modifierNames[i]))) {
		*modifiers |= modifierMasks[i];
	    	c += strlen(modifierNames[i]);
		break;
	    }
	}
	if (i == N_MODIFIERS)
	    return FALSE;
    }
    
    return TRUE;
}

/*
** Event handler for patching around Motif's lock + accelerator problem.
** Looks for a menu item in the patched menu hierarchy and invokes its
** ArmAndActivate action.
*/
static void lockCB(Widget w, XtPointer callData, XEvent *event,
	Boolean *continueDispatch)
{
    Widget topMenuWidget = (Widget)callData;
    *continueDispatch = TRUE;
    if (!(((XKeyEvent *)event)->state & (LockMask | Mod3Mask)))
	return;

    if (!findAndActivateAccel(topMenuWidget, ((XKeyEvent *)event)->keycode,
	    ((XKeyEvent *)event)->state & ~(LockMask | Mod3Mask), event))
	*continueDispatch = FALSE;
}

/*
** Search through menu hierarchy under w and look for a button with
** accelerator matching keyCode + modifiers, and do its action
*/
static int findAndActivateAccel(Widget w, unsigned int keyCode,
	unsigned int modifiers, XEvent *event)
{

    WidgetList children;
    Widget menu;
    Cardinal numChildren;
    int i;
    char *accelString = NULL;
    KeySym keysym;
    unsigned int mods;
    
    if (XtIsComposite(w)) {
	XtVaGetValues(w, XmNchildren, &children, XmNnumChildren,
		&numChildren, NULL);
	for (i=0; i<numChildren; i++)
    	    if (findAndActivateAccel(children[i], keyCode, modifiers, event))
		return TRUE;
    } else if (XtClass(w) == xmCascadeButtonWidgetClass) {
	XtVaGetValues(w, XmNsubMenuId, &menu, NULL);
	if (menu != NULL)
	    if (findAndActivateAccel(menu, keyCode, modifiers, event))
		return TRUE;
    } else {
	XtVaGetValues(w, XmNaccelerator, &accelString, NULL);
	if (accelString != NULL && *accelString != '\0') {
	    if (!parseAccelString(accelString, &keysym, &mods))
		return FALSE;
	    if (keyCode == XKeysymToKeycode(XtDisplay(w), keysym) &&
		    modifiers == mods) {
		if (XtIsSensitive(w)) {
		    XtCallActionProc(w, "ArmAndActivate", event, NULL, 0);
		    return TRUE;
		}
	    }
	}
    }
    return FALSE;
}
