/* $Id: getfiles.h,v 1.6 2003/02/15 02:33:28 yooden Exp $ */

#ifndef NEDIT_GETFILES_H_INCLUDED
#define NEDIT_GETFILES_H_INCLUDED

#include <X11/Intrinsic.h>

#define GFN_OK		1               /* Get Filename OK constant     */
#define GFN_CANCEL	2               /* Get Filename Cancel constant */

int GetExistingFilename(Widget parent, char *promptString, char *filename);
int HandleCustomExistFileSB(Widget existFileSB, char *filename);
int HandleCustomNewFileSB(Widget newFileSB, char *filename, char *defaultName);
char *GetFileDialogDefaultDirectory(void);
char *GetFileDialogDefaultPattern(void);
void SetFileDialogDefaultDirectory(char *dir);
void SetFileDialogDefaultPattern(char *pattern);
void SetGetEFTextFieldRemoval(int state);

#endif /* NEDIT_GETFILES_H_INCLUDED */
