/* $Id: printUtils.h,v 1.7 2002/07/11 21:18:09 slobasso Exp $ */

#ifndef NEDIT_PRINTUTILS_H_INCLUDED
#define NEDIT_PRINTUTILS_H_INCLUDED

#include <X11/Intrinsic.h>

/* Maximum length of an error returned by IssuePrintCommand() */
#define MAX_PRINT_ERROR_LENGTH 1024

#define DESTINATION_REMOTE 1
#define DESTINATION_LOCAL  2

void LoadPrintPreferences(XrmDatabase prefDB, const char *appName,
     const char *appClass, int lookForFlpr);

#ifdef VMS
void PrintFile(Widget parent, const char *PrintFileName, const char *jobName, int delete);
#else
void PrintFile(Widget parent, const char *PrintFileName, const char *jobName);
#endif /*VMS*/

#endif /* NEDIT_PRINTUTILS_H_INCLUDED */
