/*******************************************************************************
*									       *
* shell.h -- Nirvana Editor shell command execution			       *
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
* May 10, 1991								       *
*									       *
* Written by Mark Edel							       *
*									       *
*******************************************************************************/

/* sources for command input and destinations for command output */
enum inSrcs {FROM_SELECTION, FROM_WINDOW, FROM_EITHER, FROM_NONE};
enum outDests {TO_SAME_WINDOW, TO_NEW_WINDOW, TO_DIALOG};

void FilterSelection(WindowInfo *window, char *command, int fromMacro);
void ExecShellCommand(WindowInfo *window, char *command, int fromMacro);
void ExecCursorLine(WindowInfo *window, int fromMacro);
void ShellCmdToMacroString(WindowInfo *window, char *command, char *input);
void DoShellMenuCmd(WindowInfo *window, char *command, int input, int output,
	int outputReplaceInput, int saveFirst, int loadAfter, int fromMacro);
void AbortShellCommand(WindowInfo *window);
