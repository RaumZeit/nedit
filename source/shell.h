/* $Id: shell.h,v 1.5 2002/07/11 21:18:10 slobasso Exp $ */

#ifndef NEDIT_SHELL_H_INCLUDED
#define NEDIT_SHELL_H_INCLUDED

#include "nedit.h"

/* sources for command input and destinations for command output */
enum inSrcs {FROM_SELECTION, FROM_WINDOW, FROM_EITHER, FROM_NONE};
enum outDests {TO_SAME_WINDOW, TO_NEW_WINDOW, TO_DIALOG};

void FilterSelection(WindowInfo *window, const char *command, int fromMacro);
void ExecShellCommand(WindowInfo *window, const char *command, int fromMacro);
void ExecCursorLine(WindowInfo *window, int fromMacro);
void ShellCmdToMacroString(WindowInfo *window, const char *command,
        const char *input);
void DoShellMenuCmd(WindowInfo *window, const char *command, int input,
        int output, int outputReplaceInput,
	int saveFirst, int loadAfter, int fromMacro);
void AbortShellCommand(WindowInfo *window);

#endif /* NEDIT_SHELL_H_INCLUDED */
