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
#include <Xm/Text.h>
#include <XmL/Grid.h>

/* DATABASE PROTOTYPE FUNCTIONS */

int dbTableNumRows = 14;
int dbTableNumColumns = 5;

typedef enum {
	ID, Desc, Price, Qty, UnitPrice, Buyer 
}
DbTableColumnID;

typedef struct
	{
	DbTableColumnID id;
	char label[15];
	int width;
	unsigned char cellAlignment;
	Boolean cellEditable;
} DbTableColumn;

DbTableColumn dbTableColumns[] =
{
	{ Desc,      "Description", 16, XmALIGNMENT_LEFT,  True  },
	{ Price,     "Price",       9,  XmALIGNMENT_LEFT, True  },
	{ Qty,       "Qty",         5,  XmALIGNMENT_LEFT, True  },
	{ UnitPrice, "Unit Prc",    9,  XmALIGNMENT_LEFT, False },
	{ Buyer,     "Buyer",       15, XmALIGNMENT_LEFT,  True  },
};

typedef struct
	{
	char key[10];
	char desc[20];
	float price;
	int qty;
	char buyer[20];
} DbTableRow;

DbTableRow dbTableRows[] =
{
	{ "key01", "Staples",        1.32, 100, "Tim Pick"   },
	{ "key02", "Notebooks",      1.11,   4, "Mary Miner" },
	{ "key03", "3-Ring Binders", 2.59,   2, "Mary Miner" },
	{ "key04", "Pads",           1.23,   3, "Tim Pick"   },
	{ "key05", "Scissors",       4.41,   1, "Mary Miner" },
	{ "key06", "Pens",            .29,   4, "Mary Miner" },
	{ "key07", "Pencils",         .10,   5, "Tim Pick"   },
	{ "key08", "Markers",         .95,   3, "Mary Miner" },
	{ "key09", "Fax Paper",      3.89, 100, "Bob Coal"   },
	{ "key10", "3.5\" Disks",   15.23,  30, "Tim Pick"   },
	{ "key11", "8mm Tape",      32.22,   2, "Bob Coal"   },
	{ "key12", "Toner",         35.69,   1, "Tim Pick"   },
	{ "key13", "Paper Cups",     4.25,   3, "Bob Coal"   },
	{ "key14", "Paper Clips",    2.09,   3, "Tim Pick"   },
};

DbTableRow *dbFindRow(rowKey)
char *rowKey;
{
	int i;

	for (i = 0; i < dbTableNumRows; i++)
		if (!strcmp(rowKey, dbTableRows[i].key))
			return &dbTableRows[i];
	return 0;
}

int dbCompareRowKeys(userData, l, r)
void *userData;
void *l;
void *r;
{
	DbTableRow *dbRow1, *dbRow2;
	float u1, u2;

	dbRow1 = dbFindRow(*(char **)l);
	dbRow2 = dbFindRow(*(char **)r);
	switch ((int)userData)
	{
	case Desc:
		return strcmp(dbRow1->desc, dbRow2->desc);
	case Price:
		u1 = dbRow1->price - dbRow2->price;
		if (u1 < 0)
			return -1;
		else if (u1 == 0)
			return 0;
		return 1;
	case Qty:
		return dbRow1->qty - dbRow2->qty;
	case UnitPrice:
		u1 = dbRow1->price / (float)dbRow1->qty;
		u2 = dbRow2->price / (float)dbRow2->qty;
		if (u1 < u2)
			return -1;
		else if (u1 == u2)
			return 0;
		else
			return 1;
	case Buyer:
		return strcmp(dbRow1->buyer, dbRow2->buyer);
	}
	return (int)(dbRow1 - dbRow2);
}

char **dbGetRowKeysSorted(sortColumnID)
int sortColumnID;
{
	char **keys;
	int i;

	keys = (char **)malloc(sizeof(char *) * dbTableNumRows);
	for (i = 0; i < dbTableNumRows; i++)
		keys[i] = dbTableRows[i].key;
	XmLSort(keys, dbTableNumRows, sizeof(char *),
		dbCompareRowKeys, (void *)sortColumnID);
	return keys;
}

/* GRID FUNCTIONS */

void setRowKeysInGridSorted(grid, sortColumnID)
Widget grid;
int sortColumnID;
{
	char **keys;
	int i;

	keys = dbGetRowKeysSorted(sortColumnID);
	/* Place a pointer to each row key in each rows userData */
	for (i = 0; i < dbTableNumRows; i++)
		XtVaSetValues(grid,
			XmNrow, i,
			XmNrowUserData, (XtPointer)keys[i],
			NULL);
	free((char *)keys);
}

void cellSelect(w, clientData, callData)
Widget w;
XtPointer clientData;
XtPointer callData;
{
	XmLGridCallbackStruct *cbs;
	XmLGridColumn column;
	XtPointer columnUserData;

	cbs = (XmLGridCallbackStruct *)callData;

	if (cbs->rowType != XmHEADING)
		return;

	/* Cancel any edits in progress */
	XmLGridEditCancel(w);

	column = XmLGridGetColumn(w, cbs->columnType, cbs->column);
	XtVaGetValues(w,
		XmNcolumnPtr, column,
		XmNcolumnUserData, &columnUserData,
		NULL);
	XtVaSetValues(w,
		XmNcolumn, cbs->column,
		XmNcolumnSortType, XmSORT_ASCENDING,
    	NULL);
	setRowKeysInGridSorted(w, (DbTableColumnID)columnUserData);
	XmLGridRedrawAll(w);
}

void cellDraw(w, clientData, callData)
Widget w;
XtPointer clientData;
XtPointer callData;
{
	XmLGridCallbackStruct *cbs;
	XmLGridDrawStruct *ds;
	XmLGridRow row;
	XmLGridColumn column;
	XtPointer rowUserData, columnUserData;
	DbTableRow *dbRow;
	XRectangle cellRect;
	int horizMargin, vertMargin;
	XmString str;
	char buf[50];

	cbs = (XmLGridCallbackStruct *)callData;
	if (cbs->rowType != XmCONTENT)
		return;

	ds = cbs->drawInfo;

	/* Retrieve userData from the cells row */
	row = XmLGridGetRow(w, cbs->rowType, cbs->row);
	XtVaGetValues(w,
		XmNrowPtr, row,
		XmNrowUserData, &rowUserData,
		NULL);

	/* Retrieve userData from cells column */
	column = XmLGridGetColumn(w, cbs->columnType, cbs->column);
	XtVaGetValues(w,
		XmNcolumnPtr, column,
		XmNcolumnUserData, &columnUserData,
		NULL);

	/* Retrieve the cells value from the database */
	dbRow = dbFindRow((char *)rowUserData);
	switch ((DbTableColumnID)columnUserData)
	{
	case Desc:
		sprintf(buf, "%s", dbRow->desc);
		break;
	case Price:
		sprintf(buf, "$%4.2f", dbRow->price);
		break;
	case Qty:
		sprintf(buf, "%d", dbRow->qty);
		break;
	case UnitPrice:
		sprintf(buf, "$%4.2f", dbRow->price / (float)dbRow->qty);
		break;
	case Buyer:
		sprintf(buf, "%s", dbRow->buyer);
		break;
	}

	/* Compensate for cell margins */
	cellRect = *ds->cellRect;
	horizMargin = ds->leftMargin + ds->rightMargin;
	vertMargin = ds->topMargin + ds->bottomMargin;
	if (horizMargin >= (int)cellRect.width ||
		vertMargin >= (int)cellRect.height)
		return;
	cellRect.x += ds->leftMargin;
	cellRect.y += ds->topMargin;
	cellRect.width -= horizMargin;
	cellRect.height -= vertMargin;

	/* Draw the string */
	str = XmStringCreateSimple(buf);
	if (ds->drawSelected == True)
		XSetForeground(XtDisplay(w), ds->gc, ds->selectForeground);
	else
		XSetForeground(XtDisplay(w), ds->gc, ds->foreground);
	XmLStringDraw(w, str, ds->stringDirection, ds->fontList,
		ds->alignment, ds->gc, &cellRect, cbs->clipRect);
	XmStringFree(str);
}

void cellEdit(w, clientData, callData)
Widget w;
XtPointer clientData;
XtPointer callData;
{
	XmLGridCallbackStruct *cbs;
	XmLGridRow row;
	XmLGridColumn column;
	XtPointer rowUserData, columnUserData;
	DbTableRow *dbRow;
	Widget text;
	float f;
	int i;
	char *value;
	Boolean redrawRow;

	cbs = (XmLGridCallbackStruct *)callData;

	/* For a production version, this function should also
	   handle XmCR_EDIT_INSERT by retrieving the current value
	   from the database and performing an XmTextSetString on
	   the text widget in the grid with that value.  This allows
	   a user to hit insert or F2 to modify an existing cell value */

	if (cbs->reason != XmCR_EDIT_COMPLETE)
		return;

	/* Get the value the user just typed in */
	XtVaGetValues(w,
		XmNtextWidget, &text,
		NULL);
	value = XmTextGetString(text);
	if (!value)
		return;

	/* Retrieve userData from the cells row */
	row = XmLGridGetRow(w, cbs->rowType, cbs->row);
	XtVaGetValues(w,
		XmNrowPtr, row,
		XmNrowUserData, &rowUserData,
		NULL);

	/* Retrieve userData from cells column */
	column = XmLGridGetColumn(w, cbs->columnType, cbs->column);
	XtVaGetValues(w,
		XmNcolumnPtr, column,
		XmNcolumnUserData, &columnUserData,
		NULL);

	/* Set new value in the database */
	redrawRow = False;
	dbRow = dbFindRow((char *)rowUserData);
	switch ((DbTableColumnID)columnUserData)
	{
	case Desc:
		if ((int)strlen(value) < 20)
			strcpy(dbRow->desc, value);
		break;
	case Price:
		if (sscanf(value, "%f", &f) == 1)
		{
			dbRow->price = f;
			redrawRow = True;
		}
		break;
	case Qty:
		if (sscanf(value, "%d", &i) == 1)
		{
			dbRow->qty = i;
			redrawRow = True;
		}
		break;
	case Buyer:
		if ((int)strlen(value) < 20)
			strcpy(dbRow->buyer, value);
		break;
	}

	/* Redraw the row if we need to redisplay unit price */
	if (redrawRow == True)
		XmLGridRedrawRow(w, cbs->rowType, cbs->row);

	/* Set the cellString to NULL - its is set to the value the
		   user typed in the text widget at this point */
	XtVaSetValues(w,
		XmNrow, cbs->row,
		XmNcolumn, cbs->column,
		XmNcellString, NULL,
		NULL);

	XtFree(value);
}

main(argc, argv)
int argc;
char *argv[];
{
	XtAppContext app;
	Widget shell, grid;
	XmString str;
	int i;

	shell =  XtAppInitialize(&app, "Grid6", NULL, 0,
		&argc, argv, NULL, NULL, 0);

	grid = XtVaCreateManagedWidget("grid",
		xmlGridWidgetClass, shell,
		XmNhorizontalSizePolicy, XmVARIABLE,
		XmNvisibleRows, 10,
		XmNvsbDisplayPolicy, XmSTATIC,
		XmNselectionPolicy, XmSELECT_NONE,
		XmNshadowThickness, 0,
		XtVaTypedArg, XmNbackground, XmRString, "#C0C0C0", 8,
		XtVaTypedArg, XmNforeground, XmRString, "black", 6,
		NULL);
	XtAddCallback(grid, XmNcellDrawCallback, cellDraw, NULL);
	XtAddCallback(grid, XmNeditCallback, cellEdit, NULL);
	XtAddCallback(grid, XmNselectCallback, cellSelect, NULL);

	XtVaSetValues(grid,
		XmNlayoutFrozen, True,
		NULL);

	XmLGridAddColumns(grid, XmCONTENT, -1, dbTableNumColumns);

	/* Setup columns and column cell defaults based on */
	/* database description */
	for (i = 0; i < dbTableNumColumns; i++)
	{
		/* Set the width and the id on the column */
		XtVaSetValues(grid,
			XmNcolumn, i,
			XmNcolumnUserData, (XtPointer)dbTableColumns[i].id,
			XmNcolumnWidth, dbTableColumns[i].width,
			NULL);

		/* Set the default cell alignment and editibility for */
		/* cells in the column */
		XtVaSetValues(grid,
			XmNcellDefaults, True,
			XmNcolumn, i,
			XmNcellAlignment, dbTableColumns[i].cellAlignment,
			XmNcellEditable, dbTableColumns[i].cellEditable,
			NULL);
	}

	/* Add the heading row */
	XmLGridAddRows(grid, XmHEADING, -1, 1);

	/* Set the column headings */
	for (i = 0; i < dbTableNumColumns; i++)
	{
		/* Set the column heading label */
		str = XmStringCreateSimple(dbTableColumns[i].label);
		XtVaSetValues(grid,
			XmNrowType, XmHEADING,
			XmNrow, 0,
			XmNcolumn, i,
			XmNcellString, str,
			NULL);
		XmStringFree(str);
	}

	/* Set cell defaults for content rows */
	XtVaSetValues(grid,
		XmNcellDefaults, True,
		XtVaTypedArg, XmNcellBackground, XmRString, "white", 6,
		XmNcellLeftBorderType, XmBORDER_NONE,
		XmNcellRightBorderType, XmBORDER_NONE,
		XmNcellTopBorderType, XmBORDER_NONE,
		XmNcellBottomBorderType, XmBORDER_NONE,
		XmNcellMarginLeft, 1,
		XmNcellMarginRight, 1,
		NULL);

	XmLGridAddRows(grid, XmCONTENT, -1, dbTableNumRows);

	XtVaSetValues(grid,
		XmNlayoutFrozen, False,
		NULL);

	/* Set the row keys in the rows */
	setRowKeysInGridSorted(grid, Desc);

	XtRealizeWidget(shell);
	XtAppMainLoop(app);
}
