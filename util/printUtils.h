/* Maximum length of an error returned by IssuePrintCommand() */
#define MAX_PRINT_ERROR_LENGTH 1024

#define DESTINATION_REMOTE 1
#define DESTINATION_LOCAL  2

void LoadPrintPreferences(XrmDatabase prefDB, char *appName, char *appClass,
	int lookForFlpr);

#ifdef VMS
void PrintFile(Widget parent, char *PrintFileName, char *jobName, int delete);
#else
void PrintFile(Widget parent, char *PrintFileName, char *jobName);
#endif /*VMS*/
