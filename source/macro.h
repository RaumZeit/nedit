/* $Id: macro.h,v 1.2 2001/02/26 23:38:03 edg Exp $ */
#define REPEAT_TO_END -1
#define REPEAT_IN_SEL -2
	
void RegisterMacroSubroutines(void);
void AddLastCommandActionHook(XtAppContext context);
void BeginLearn(WindowInfo *window);
void FinishLearn(void);
void CancelMacroOrLearn(WindowInfo *window);
void Replay(WindowInfo *window);
void SafeGC(void);
void DoMacro(WindowInfo *window, char *macro, char *errInName);
void ResumeMacroExecution(WindowInfo *window);
void AbortMacroCommand(WindowInfo *window);
int MacroWindowCloseActions(WindowInfo *window);
void RepeatDialog(WindowInfo *window);
void RepeatMacro(WindowInfo *window, char *command, int how);
int ReadMacroFile(WindowInfo *window, char *fileName, int warnNotExist);
int ReadMacroString(WindowInfo *window, char *string, char *errIn);
int CheckMacroString(Widget dialogParent, char *string, char *errIn,
	char **errPos);
char *GetReplayMacro(void);
void ReadMacroInitFile(WindowInfo *window);
void ReturnShellCommandOutput(WindowInfo *window, char *outText, int status);
