enum undoTypes {UNDO_NOOP, ONE_CHAR_INSERT, ONE_CHAR_REPLACE, ONE_CHAR_DELETE,
		BLOCK_INSERT, BLOCK_REPLACE, BLOCK_DELETE};

void Undo(WindowInfo *window);
void Redo(WindowInfo *window);
void SaveUndoInformation(WindowInfo *window, int pos, int nInserted,
	int nDeleted, char *deletedText);
void ClearUndoList(WindowInfo *window);
void ClearRedoList(WindowInfo *window);
