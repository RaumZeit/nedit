/* $Id: calltips.h,v 1.2 2003/04/18 01:07:36 n8gray Exp $ */
#ifndef NEDIT_CALLTIPS_H_INCLUDED
#define NEDIT_CALLTIPS_H_INCLUDED

#include "nedit.h"      /* For WindowInfo */
#include "textDisp.h"   /* for textDisp */

#define NEDIT_DEFAULT_CALLTIP_FG "black"
#define NEDIT_DEFAULT_CALLTIP_BG "LemonChiffon1"


enum TipHAlignMode {TIP_LEFT, TIP_CENTER, TIP_RIGHT};
enum TipVAlignMode {TIP_ABOVE, TIP_BELOW};
enum TipAlignStrict {TIP_SLOPPY, TIP_STRICT};

int  ShowCalltip(WindowInfo *window, char *text, Boolean anchored, 
        int pos, int hAlign, int vAlign, int alignMode);
void KillCalltip(WindowInfo *window, int calltipID);
void TextDKillCalltip(textDisp *textD, int calltipID);
int  GetCalltipID(WindowInfo *window, int calltipID);
void RedrawCalltip(WindowInfo *window, int calltipID);
void TextDRedrawCalltip(textDisp *textD, int calltipID);

#endif /* ifndef NEDIT_CALLTIPS_H_INCLUDED */
