/* $Id: file.h,v 1.9 2001/10/15 17:28:16 slobasso Exp $ */
/* flags for EditExistingFile */
#define CREATE 1
#define SUPPRESS_CREATE_WARN 2
#define PREF_READ_ONLY 4

#define PROMPT_SBC_DIALOG_RESPONSE 0
#define YES_SBC_DIALOG_RESPONSE 1
#define NO_SBC_DIALOG_RESPONSE 2

void EditNewFile(char *geometry, int iconic, const char *languageMode,
    	const char *defaultPath);
WindowInfo *EditExistingFile(WindowInfo *inWindow, const char *name,
        const char *path, int flags, char *geometry, int iconic,
	const char *languageMode);
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
