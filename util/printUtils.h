/* $Id: printUtils.h,v 1.6 2002/06/26 23:39:21 slobasso Exp $ */

#ifndef PRINTUTILS_H_INCLUDED
#define PRINTUTILS_H_INCLUDED

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

#endif /* PRINTUTILS_H_INCLUDED */
