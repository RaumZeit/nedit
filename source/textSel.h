/* $Id: textSel.h,v 1.4 2003/05/09 17:43:48 edg Exp $ */

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
