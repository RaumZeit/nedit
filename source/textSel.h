/* $Id: textSel.h,v 1.6 2004/11/09 21:58:45 yooden Exp $ */
/*******************************************************************************
*                                                                              *
* textSel.h -- Nirvana Editor Selection header file                            *
*                                                                              *
* Copyright 2004 The NEdit Developers                                          *
*                                                                              *
* This is free software; you can redistribute it and/or modify it under the    *
* terms of the GNU General Public License as published by the Free Software    *
* Foundation; either version 2 of the License, or (at your option) any later   *
* version. In addition, you may distribute versions of this program linked to  *
* Motif or Open Motif. See README for details.                                 *
*                                                                              *
* This software is distributed in the hope that it will be useful, but WITHOUT *
* ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or        *
* FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for    *
* more details.                                                                *
*                                                                              *
* You should have received a copy of the GNU General Public License along with *
* software; if not, write to the Free Software Foundation, Inc., 59 Temple     *
* Place, Suite 330, Boston, MA  02111-1307 USA                                 *
*                                                                              *
* Nirvana Text Editor                                                          *
* July 31, 2001                                                                *
*                                                                              *
*******************************************************************************/

#ifndef NEDIT_TEXTSEL_H_INCLUDED
#define NEDIT_TEXTSEL_H_INCLUDED

#include <X11/Intrinsic.h>
#include <X11/X.h>

void HandleXSelections(Widget w);
void StopHandlingXSelections(Widget w);
void CopyToClipboard(Widget w, Time time);
void InsertPrimarySelection(Widget w, Time time, int isColumnar);
void MovePrimarySelection(Widget w, Time time, int isColumnar);
void SendSecondarySelection(Widget w, Time time, int removeAfter);
void ExchangeSelections(Widget w, Time time);
void InsertClipboard(Widget w, int isColumnar);
void TakeMotifDestination(Widget w, Time time);

#endif /* NEDIT_TEXTSEL_H_INCLUDED */
