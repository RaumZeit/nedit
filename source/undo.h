/*******************************************************************************
*									       *
* undo.h -- Nirvana Editor undo command					       *
*									       *
* Copyright (c) 1991 Universities Research Association, Inc.		       *
* All rights reserved.							       *
* 									       *
* This material resulted from work developed under a Government Contract and   *
* is subject to the following license:  The Government retains a paid-up,      *
* nonexclusive, irrevocable worldwide license to reproduce, prepare derivative *
* works, perform publicly and display publicly by or for the Government,       *
* including the right to distribute to other Government contractors.  Neither  *
* the United States nor the United States Department of Energy, nor any of     *
* their employees, makes any warrenty, express or implied, or assumes any      *
* legal liability or responsibility for the accuracy, completeness, or         *
* usefulness of any information, apparatus, product, or process disclosed, or  *
* represents that its use would not infringe privately owned rights.           *
*                                        				       *
* Fermilab Nirvana GUI Library						       *
* June 11, 1991								       *
*									       *
* Written by Mark Edel							       *
*									       *
*******************************************************************************/
enum undoTypes {UNDO_NOOP, ONE_CHAR_INSERT, ONE_CHAR_REPLACE, ONE_CHAR_DELETE,
		BLOCK_INSERT, BLOCK_REPLACE, BLOCK_DELETE};

void Undo(WindowInfo *window);
void Redo(WindowInfo *window);
void SaveUndoInformation(WindowInfo *window, int pos, int nInserted,
	int nDeleted, char *deletedText);
void ClearUndoList(WindowInfo *window);
void ClearRedoList(WindowInfo *window);
