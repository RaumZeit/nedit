/* $Id: undo.h,v 1.6 2002/08/27 05:39:27 n8gray Exp $ */

#ifndef NEDIT_UNDO_H_INCLUDED
#define NEDIT_UNDO_H_INCLUDED

#include "nedit.h"

enum undoTypes {UNDO_NOOP, ONE_CHAR_INSERT, ONE_CHAR_REPLACE, ONE_CHAR_DELETE,
		BLOCK_INSERT, BLOCK_REPLACE, BLOCK_DELETE};

void Undo(WindowInfo *window);
void Redo(WindowInfo *window);
void SaveUndoInformation(WindowInfo *window, int pos, int nInserted,
	int nDeleted, const char *deletedText);
void ClearUndoList(WindowInfo *window);
void ClearRedoList(WindowInfo *window);
void DisableUnmodified(WindowInfo *window);

#endif /* NEDIT_UNDO_H_INCLUDED */
