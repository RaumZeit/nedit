/* $Id: fileUtils.h,v 1.7 2001/11/18 19:02:58 arnef Exp $ */
int ParseFilename(const char *fullname, char *filename, char *pathname);
int ExpandTilde(char *pathname);
const char* GetTrailingPathComponents(const char* path,
                                      int noOfComponents);
int NormalizePathname(char *pathname);
int CompressPathname(char *pathname);
int ResolvePath(const char * pathIn, char * pathResolved); 
