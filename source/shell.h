/* $Id: shell.h,v 1.6 2004/01/08 06:19:27 tksoh Exp $ */

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
void MakeShellBanner(WindowInfo *window);

#endif /* NEDIT_SHELL_H_INCLUDED */
