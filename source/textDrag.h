enum blockDragTypes {USE_LAST, DRAG_COPY, DRAG_MOVE, DRAG_OVERLAY_MOVE,
    	DRAG_OVERLAY_COPY};

void BeginBlockDrag(TextWidget tw);
void BlockDragSelection(TextWidget tw, int x, int y, int dragType);
void FinishBlockDrag(TextWidget tw);
void CancelBlockDrag(TextWidget tw);
