/* -*- Mode: C; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
 *
 * ***** BEGIN LICENSE BLOCK *****
 * Version: MPL 1.1/GPL 2.0/LGPL 2.1
 *
 * The contents of this file are subject to the Mozilla Public License Version
 * 1.1 (the "License"); you may not use this file except in compliance with
 * the License. You may obtain a copy of the License at
 * http://www.mozilla.org/MPL/
 *
 * Software distributed under the License is distributed on an "AS IS" basis,
 * WITHOUT WARRANTY OF ANY KIND, either express or implied. See the License
 * for the specific language governing rights and limitations under the
 * License.
 *
 * The Original Code is the Microline Widget Library, originally made available under the NPL by Neuron Data <http://www.neurondata.com>.
 *
 * The Initial Developer of the Original Code is
 * Netscape Communications Corporation.
 * Portions created by the Initial Developer are Copyright (C) 1998
 * the Initial Developer. All Rights Reserved.
 *
 * Contributor(s):
 *
 * Alternatively, the contents of this file may be used under the terms of
 * either the GNU General Public License Version 2 or later (the "GPL"), or
 * the GNU Lesser General Public License Version 2.1 or later (the "LGPL"),
 * in which case the provisions of the GPL or the LGPL are applicable instead
 * of those above. If you wish to allow use of your version of this file only
 * under the terms of either the GPL or the LGPL, and not to allow others to
 * use your version of this file under the terms of the MPL, indicate your
 * decision by deleting the provisions above and replace them with the notice
 * and other provisions required by the GPL or the LGPL. If you do not delete
 * the provisions above, a recipient may use your version of this file under
 * the terms of any one of the MPL, the GPL or the LGPL.
 *
 * In addition, as a special exception to the GNU GPL, the copyright holders
 * give permission to link the code of this program with the Motif and Open
 * Motif libraries (or with modified versions of these that use the same
 * license), and distribute linked combinations including the two. You
 * must obey the GNU General Public License in all respects for all of
 * the code used other than linking with Motif/Open Motif. If you modify
 * this file, you may extend this exception to your version of the file,
 * but you are not obligated to do so. If you do not wish to do so,
 * delete this exception statement from your version.
 *
 * ***** END LICENSE BLOCK ***** */


#include <Xm/Xm.h>
#include <Xm/Form.h>
#include <XmL/Grid.h>
#include <XmL/Tree.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <dirent.h>

void rowExpand();
void rowCollapse();
void rowDelete();
void cellSelect();

Widget grid;

main(argc, argv)
int argc;
char *argv[];
{
	XtAppContext app;
	Widget shell, form, tree;
	XmString str;

	shell = XtAppInitialize(&app, "Tree4", NULL, 0,
		&argc, argv, NULL, NULL, 0);

	form = XtVaCreateManagedWidget("form",
		xmFormWidgetClass, shell,
		XtVaTypedArg, XmNbackground, XmRString, "#C0C0C0", 8,
		XmNshadowThickness, 0,
		NULL);

	/* Add Tree to left of Form */
	tree = XtVaCreateManagedWidget("tree",
		xmlTreeWidgetClass, form,
		XmNhorizontalSizePolicy, XmCONSTANT,
		XmNautoSelect, False,
		XtVaTypedArg, XmNbackground, XmRString, "#C0C0C0", 8,
		XtVaTypedArg, XmNforeground, XmRString, "black", 6,
		XtVaTypedArg, XmNblankBackground, XmRString, "white", 6,
		XtVaTypedArg, XmNselectBackground, XmRString, "#000080", 8,
		XtVaTypedArg, XmNselectForeground, XmRString, "white", 6,
		XtVaTypedArg, XmNconnectingLineColor, XmRString, "#808080", 8,
		XmNleftAttachment, XmATTACH_FORM,
		XmNtopAttachment, XmATTACH_FORM,
		XmNbottomAttachment, XmATTACH_FORM,
		XmNrightAttachment, XmATTACH_POSITION,
		XmNrightPosition, 45,
		NULL);
	XtVaSetValues(tree,
		XmNcellDefaults, True,
		XtVaTypedArg, XmNcellBackground, XmRString, "white", 6,
		NULL);

	/* Add a single row containing the root path to the Tree */
	str = XmStringCreateSimple("/");
	XmLTreeAddRow(tree, 1, True, False, 0,
		XmUNSPECIFIED_PIXMAP, XmUNSPECIFIED_PIXMAP, str);
	XmStringFree(str);
	XtVaSetValues(tree,
		XmNrow, 0,
		XmNrowUserData, strdup("/"),
		NULL);

	XtAddCallback(tree, XmNexpandCallback, rowExpand, NULL);
	XtAddCallback(tree, XmNcollapseCallback, rowCollapse, NULL);
    XtAddCallback(tree, XmNdeleteCallback, rowDelete, NULL);
	XtAddCallback(tree, XmNselectCallback, cellSelect, NULL);

	/* Add a Grid to the right of the Form and set cell defaults */
	grid = XtVaCreateManagedWidget("grid",
		xmlGridWidgetClass, form,
		XtVaTypedArg, XmNbackground, XmRString, "#C0C0C0", 8,
		XtVaTypedArg, XmNforeground, XmRString, "black", 6,
		XtVaTypedArg, XmNblankBackground, XmRString, "white", 6,
		XtVaTypedArg, XmNselectBackground, XmRString, "#000080", 8,
		XtVaTypedArg, XmNselectForeground, XmRString, "white", 6,
		XmNcolumns, 3,
		XmNsimpleWidths, "24c 12c 10c",
		XmNsimpleHeadings, "Name|Type|Size",
		XmNvisibleColumns, 6,
		XmNallowColumnResize, True,
		XmNheadingRows, 1,
		XmNvisibleRows, 16,
		XmNhighlightRowMode, True,
		XmNleftAttachment, XmATTACH_POSITION,
		XmNleftPosition, 46,
		XmNtopAttachment, XmATTACH_FORM,
		XmNbottomAttachment, XmATTACH_FORM,
		XmNrightAttachment, XmATTACH_FORM,
		NULL);
	XtVaSetValues(grid,
		XmNcellDefaults, True,
		XtVaTypedArg, XmNcellBackground, XmRString, "white", 6,
		XmNcellTopBorderType, XmBORDER_NONE,
		XmNcellBottomBorderType, XmBORDER_NONE,
		XmNcellRightBorderType, XmBORDER_NONE,
		XmNcellLeftBorderType, XmBORDER_NONE,
		XmNcellAlignment, XmALIGNMENT_LEFT,
		NULL);
	XtVaSetValues(grid,
		XmNcellDefaults, True,
		XmNcolumn, 2,
		XmNcellAlignment, XmALIGNMENT_RIGHT,
		NULL);

	/* Invoke the select callback for the first row in the Tree */
	/* to fill the Grid with the data for the root path */
	XmLGridSelectRow(tree, 0, True);

	XtRealizeWidget(shell);
	XtAppMainLoop(app);
}

void rowExpand(w, clientData, callData)
Widget w;
XtPointer clientData, callData;
{
	XmLGridCallbackStruct *cbs;
	XmLGridRow row;
	int level, pos;
	DIR *dir;
	struct dirent *d;
	struct stat s;
	char *path, fullpath[1024];
	XmString str;

	/* Retrieve the path of the directory expanded.  This is kept */
	/* in the row's rowUserData */
	cbs = (XmLGridCallbackStruct *)callData;
	row = XmLGridGetRow(w, XmCONTENT, cbs->row);
	XtVaGetValues(w,
		XmNrowPtr, row,
		XmNrowUserData, &path,
		XmNrowLevel, &level,
		NULL);
	pos = cbs->row + 1;
	dir = opendir(path);
	if (!dir)
		return;

	/* Add the subdirectories of the directory expanded to the Tree */
	XtVaSetValues(w,
		XmNlayoutFrozen, True,
		NULL);
	while (d = readdir(dir))
	{
		if (!strcmp(d->d_name, ".") || !strcmp(d->d_name, ".."))
			continue;
		sprintf(fullpath, "%s/%s", path, d->d_name);
		if (lstat(fullpath, &s) == -1)
			continue;
		if (!S_ISDIR(s.st_mode))
			continue;
		str = XmStringCreateSimple(d->d_name);
		XmLTreeAddRow(w, level + 1, True, False, pos,
			XmUNSPECIFIED_PIXMAP, XmUNSPECIFIED_PIXMAP, str);
		XmStringFree(str);
		XtVaSetValues(w,
			XmNrow, pos,
			XmNrowUserData, strdup(fullpath),
			NULL);
		pos++;
	}
	closedir(dir);
	XtVaSetValues(w,
		XmNlayoutFrozen, False,
		NULL);
}

void rowCollapse(w, clientData, callData)
Widget w;
XtPointer clientData, callData;
{
	XmLGridCallbackStruct *cbs;
	XmLGridRow row;
	char *path;
	int i, j, level, rows;

	/* Collapse the row by deleting the rows in the tree which */
	/* are children of the collapsed row. */
	cbs = (XmLGridCallbackStruct *)callData;

    XmLTreeDeleteChildren(w, cbs->row);
}

void rowDelete(w, clientData, callData)
Widget w;
XtPointer clientData, callData;
{
    /* Free the path contained in the rowUserData of the rows deleted */

	XmLGridCallbackStruct *cbs;
	XmLGridRow row;
	char *path;

	cbs = (XmLGridCallbackStruct *)callData;

 	if (cbs->rowType != XmCONTENT || cbs->reason != XmCR_DELETE_ROW)
		return;

	row = XmLGridGetRow(w, XmCONTENT, cbs->row);

	XtVaGetValues(w,
		XmNrowPtr, row,
		XmNrowUserData, &path,
		NULL);

    if (path)
        free(path);
}

void cellSelect(w, clientData, callData)
Widget w;
XtPointer clientData, callData;
{
	XmLGridCallbackStruct *cbs;
	XmLGridRow row;
	DIR *dir;
	struct stat s;
	struct dirent *d;
	char *path, fullpath[1024], buf[256];
	int pos;

	/* Retrieve the directory selected */
	cbs = (XmLGridCallbackStruct *)callData;
 	if (cbs->rowType != XmCONTENT)
		return;
	row = XmLGridGetRow(w, XmCONTENT, cbs->row);
	XtVaGetValues(w,
		XmNrowPtr, row,
		XmNrowUserData, &path,
		NULL);
	dir = opendir(path);
	if (!dir)
		return;

	/* Add a row for each file in the directory to the Grid */
	pos = 0;
	XtVaSetValues(grid,
		XmNlayoutFrozen, True,
		NULL);
	XmLGridDeleteAllRows(grid, XmCONTENT);
	while (d = readdir(dir))
	{
		sprintf(fullpath, "%s/%s", path, d->d_name);
		if (lstat(fullpath, &s) == -1)
			continue;
		XmLGridAddRows(grid, XmCONTENT, pos, 1);
		XmLGridSetStringsPos(grid, XmCONTENT, pos, XmCONTENT, 0, d->d_name);
		if (S_ISDIR(s.st_mode))
			sprintf(buf, "Directory");
		else if (S_ISLNK(s.st_mode))
			sprintf(buf, "Link");
		else
			sprintf(buf, "File");
		XmLGridSetStringsPos(grid, XmCONTENT, pos, XmCONTENT, 1, buf);
		sprintf(buf, "%d", (int)s.st_size);
		XmLGridSetStringsPos(grid, XmCONTENT, pos, XmCONTENT, 2, buf);
		pos++;
	}
	closedir(dir);
	XtVaSetValues(grid,
		XmNlayoutFrozen, False,
		NULL);
}

