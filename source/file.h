/* $Id: file.h,v 1.5 2001/04/06 09:49:55 amai Exp $ */
/* flags for EditExistingFile */
#define CREATE 1
#define SUPPRESS_CREATE_WARN 2
#define FORCE_READ_ONLY 4

void EditNewFile(char *geometry, int iconic, char *languageMode,
    	char *defaultPath);
WindowInfo *EditExistingFile(WindowInfo *inWindow, char *name, char *path,
	int flags, char *geometry, int iconic, char *languageMode);
void RevertToSaved(WindowInfo *window);
int SaveWindow(WindowInfo *window);
int SaveWindowAs(WindowInfo *window, char *newName, int addWrap);
int CloseAllFilesAndWindows(void);
int CloseFileAndWindow(WindowInfo *window);
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
