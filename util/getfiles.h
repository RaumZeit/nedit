/* $Id: getfiles.h,v 1.4 2002/06/26 23:39:21 slobasso Exp $ */

#ifndef GETFILES_H_INCLUDED
#define GETFILES_H_INCLUDED

#include <X11/Intrinsic.h>

#define GFN_OK		1               /* Get Filename OK constant     */
#define GFN_CANCEL	2               /* Get Filename Cancel constant */

int GetNewFilename(Widget parent, char *promptString, char *filename);
int GetExistingFilename(Widget parent, char *promptString, char *filename);
int HandleCustomExistFileSB(Widget existFileSB, char *filename);
int HandleCustomNewFileSB(Widget newFileSB, char *filename, char *defaultName);
char *GetFileDialogDefaultDirectory(void);
char *GetFileDialogDefaultPattern(void);
void SetFileDialogDefaultDirectory(char *dir);
void SetFileDialogDefaultPattern(char *pattern);
void SetGetEFTextFieldRemoval(int state);

#endif /* GETFILES_H_INCLUDED */
