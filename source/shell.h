/* $Id: shell.h,v 1.3 2001/02/26 23:38:03 edg Exp $ */
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
