/* $Id: fileUtils.h,v 1.9 2002/07/11 21:18:09 slobasso Exp $ */

#ifndef NEDIT_FILEUTILS_H_INCLUDED
#define NEDIT_FILEUTILS_H_INCLUDED

int ParseFilename(const char *fullname, char *filename, char *pathname);
int ExpandTilde(char *pathname);
const char* GetTrailingPathComponents(const char* path,
                                      int noOfComponents);
int NormalizePathname(char *pathname);
int CompressPathname(char *pathname);
int ResolvePath(const char * pathIn, char * pathResolved); 

#endif /* NEDIT_FILEUTILS_H_INCLUDED */
