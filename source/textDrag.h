/* $Id: textDrag.h,v 1.3 2002/07/11 21:18:12 slobasso Exp $ */

#ifndef NEDIT_TEXTDRAG_H_INCLUDED
#define NEDIT_TEXTDRAG_H_INCLUDED

#include "text.h"

enum blockDragTypes {USE_LAST, DRAG_COPY, DRAG_MOVE, DRAG_OVERLAY_MOVE,
    	DRAG_OVERLAY_COPY};

void BeginBlockDrag(TextWidget tw);
void BlockDragSelection(TextWidget tw, int x, int y, int dragType);
void FinishBlockDrag(TextWidget tw);
void CancelBlockDrag(TextWidget tw);

#endif /* NEDIT_TEXTDRAG_H_INCLUDED */
