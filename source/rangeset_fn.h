/* $Id: rangeset_fn.h,v 1.2 2002/11/08 20:22:45 edg Exp $ */
/*******************************************************************************
*									       *
* rangeset_fn.h	 -- Nirvana Editor rangest function header		       *
*									       *
* Copyright (C) 1999 Mark Edel						       *
*									       *
* This is free software; you can redistribute it and/or modify it under the    *
* terms of the GNU General Public License as published by the Free Software    *
* Foundation; either version 2 of the License, or (at your option) any later   *
* version.								       *
*									       *
* This software is distributed in the hope that it will be useful, but WITHOUT *
* ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or	       *
* FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License	       *
* for more details.							       *
*									       *
* You should have received a copy of the GNU General Public License along with *
* software; if not, write to the Free Software Foundation, Inc., 59 Temple     *
* Place, Suite 330, Boston, MA	02111-1307 USA				       *
*									       *
* Nirvana Text Editor							       *
* Sep 26, 2002								       *
*									       *
* Written by Tony Balinski with contributions from Andrew Hood		       *
*									       *
* Modifications:							       *
*									       *
*									       *
*******************************************************************************/
#ifndef rangeset_fn_h_DEFINED
#define rangeset_fn_h_DEFINED

#include "rangeset.h"
#include <Xm/Xm.h>

void RangesetRefreshRange(Rangeset *p, int start, int end);
void RangesetEmpty(Rangeset *r);
void RangesetInit(Rangeset *r, char label, textBuffer *buf);
int RangesetChangeModifyResponse(Rangeset *r, char *name);
int RangesetFindRangeNo(Rangeset *p, int ind, int *start, int *end);
int RangesetFindRangeOfPos(Rangeset *p, int pos, int incl_end);
int RangesetCheckRangeOfPos(Rangeset *p, int pos);
int RangesetInverse(Rangeset *p);
int RangesetAdd(Rangeset *p, Rangeset *q);
int RangesetAddBetween(Rangeset *p, int start, int end);
int RangesetRemove(Rangeset *p, Rangeset *q);
int RangesetRemoveBetween(Rangeset *p, int start, int end);
int RangesetGetNRanges(Rangeset *p);
void RangesetSetMaxpos(Rangeset *p, int maxpos);
RangesetTable *RangesetTableAlloc(textBuffer *buf);
RangesetTable *RangesetTableFree(RangesetTable *tab);
int RangesetFindIndex(RangesetTable *tab, char label, int must_be_active);
int RangesetLabelOK(char label);
Rangeset *RangesetForget(RangesetTable *tab, char label);
Rangeset *RangesetFetch(RangesetTable *tab, char label, int make_active);
void RangesetTableClearMacroVars(RangesetTable *tab);
void RangesetTableAssignMacroVars(RangesetTable *tab, Rangeset *p, int range);
char *RangesetTableGetMacroSetLabel(RangesetTable *tab);
int RangesetTableGetMacroRangeIndex(RangesetTable *tab);
int RangesetTableGetMacroRangeStart(RangesetTable *tab);
int RangesetTableGetMacroRangeEnd(RangesetTable *tab);
int RangesetTableGetMacroRangeN(RangesetTable *tab);
char *RangesetTableGetMacroRangeList(RangesetTable *tab);
char *RangesetTableGetMacroRangeColor(RangesetTable *tab);
char *RangesetTableGetMacroRangeMod(RangesetTable *tab);
void RangesetTableUpdatePos(RangesetTable *tab, int pos, int n_ins, int n_del);
void RangesetBufModifiedCB(int pos, int nInserted, int nDeleted, int nRestyled,
	char *deletedText, void *cbArg);
int RangesetIndex1ofPos(RangesetTable *tab, int pos, int needs_color);
int RangesetAssignColorName(Rangeset *p, char *color_name);
int RangesetAssignColorPixel(Rangeset *p, Pixel color, int ok);
char *RangesetGetColorName(Rangeset *p);
int RangesetGetColorValid(Rangeset *p, Pixel *color);
char *RangesetTableGetColorName(RangesetTable *tab, int ind);
int RangesetTableGetColorValid(RangesetTable *tab, int ind, Pixel *color);
int RangesetTableAssignColorPixel(RangesetTable *tab, int ind, Pixel color,
	int ok);

#endif /* rangeset_fn_h_DEFINED */
