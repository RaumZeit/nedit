/* $Id: file.h,v 1.12 2004/02/16 01:02:37 tksoh Exp $ */

#ifndef NEDIT_FILE_H_INCLUDED
#define NEDIT_FILE_H_INCLUDED

#include "nedit.h"

#include <X11/Intrinsic.h>

/* flags for EditExistingFile */
#define CREATE 1
#define SUPPRESS_CREATE_WARN 2
#define PREF_READ_ONLY 4

#define PROMPT_SBC_DIALOG_RESPONSE 0
#define YES_SBC_DIALOG_RESPONSE 1
#define NO_SBC_DIALOG_RESPONSE 2

WindowInfo *EditNewFile(WindowInfo *inWindow, char *geometry, int iconic,
        const char *languageMode, const char *defaultPath);
WindowInfo *EditExistingFile(WindowInfo *inWindow, const char *name,
        const char *path, int flags, char *geometry, int iconic,
	const char *languageMode, int tabbed);
void RevertToSaved(WindowInfo *window);
int SaveWindow(WindowInfo *window);
int SaveWindowAs(WindowInfo *window, const char *newName, int addWrap);
int CloseAllFilesAndWindows(void);
int CloseFileAndWindow(WindowInfo *window, int preResponse);
void PrintWindow(WindowInfo *window, int selectedOnly);
void PrintString(const char *string, int length, Widget parent, const char *jobName);
int WriteBackupFile(WindowInfo *window);
int IncludeFile(WindowInfo *window, const char *name);
int PromptForExistingFile(WindowInfo *window, char *prompt, char *fullname);
int PromptForNewFile(WindowInfo *window, char *prompt, char *fullname,
    	int *fileFormat, int *addWrap);
int CheckReadOnly(WindowInfo *window);
void RemoveBackupFile(WindowInfo *window);
void UniqueUntitledName(char *name);
void CheckForChangesToFile(WindowInfo *window);

#endif /* NEDIT_FILE_H_INCLUDED */
