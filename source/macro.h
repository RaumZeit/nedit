/* $Id: macro.h,v 1.5 2004/01/08 06:19:27 tksoh Exp $ */

#ifndef NEDIT_MACRO_H_INCLUDED
#define NEDIT_MACRO_H_INCLUDED

#include "nedit.h"

#include <X11/Intrinsic.h>

#define REPEAT_TO_END -1
#define REPEAT_IN_SEL -2
	
void RegisterMacroSubroutines(void);
void AddLastCommandActionHook(XtAppContext context);
void BeginLearn(WindowInfo *window);
void FinishLearn(void);
void CancelMacroOrLearn(WindowInfo *window);
void Replay(WindowInfo *window);
void SafeGC(void);
void DoMacro(WindowInfo *window, const char *macro, const char *errInName);
void ResumeMacroExecution(WindowInfo *window);
void AbortMacroCommand(WindowInfo *window);
int MacroWindowCloseActions(WindowInfo *window);
void RepeatDialog(WindowInfo *window);
void RepeatMacro(WindowInfo *window, const char *command, int how);
int ReadMacroFile(WindowInfo *window, const char *fileName, int warnNotExist);
int ReadMacroString(WindowInfo *window, char *string, const char *errIn);
int CheckMacroString(Widget dialogParent, char *string, const char *errIn,
	char **errPos);
char *GetReplayMacro(void);
void ReadMacroInitFile(WindowInfo *window);
void ReturnShellCommandOutput(WindowInfo *window, const char *outText, int status);
void MakeMacroBanner(WindowInfo *window);

#endif /* NEDIT_MACRO_H_INCLUDED */
