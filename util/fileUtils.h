/* $Id: fileUtils.h,v 1.6 2001/11/12 13:46:54 amai Exp $ */
int ParseFilename(const char *fullname, char *filename, char *pathname);
int ExpandTilde(char *pathname);
int NormalizePathname(char *pathname);
int CompressPathname(char *pathname);
int ResolvePath(const char * pathIn, char * pathResolved); 
