/* $Id: fileUtils.h,v 1.8 2002/06/26 23:39:21 slobasso Exp $ */

#ifndef FILEUTILS_H_INCLUDED
#define FILEUTILS_H_INCLUDED

int ParseFilename(const char *fullname, char *filename, char *pathname);
int ExpandTilde(char *pathname);
const char* GetTrailingPathComponents(const char* path,
                                      int noOfComponents);
int NormalizePathname(char *pathname);
int CompressPathname(char *pathname);
int ResolvePath(const char * pathIn, char * pathResolved); 

#endif /* FILEUTILS_H_INCLUDED */
