/* $Id: textSel.h,v 1.2 2001/02/26 23:38:03 edg Exp $ */
void HandleXSelections(Widget w);
void StopHandlingXSelections(Widget w);
void CopyToClipboard(Widget w, Time time);
void InsertPrimarySelection(Widget w, Time time, int isColumnar);
void MovePrimarySelection(Widget w, Time time, int isColumnar);
void SendSecondarySelection(Widget w, Time time, int removeAfter);
void ExchangeSelections(Widget w, Time time);
void InsertClipboard(Widget w, Time time, int isColumnar);
void TakeMotifDestination(Widget w, Time time);
