/* $Id: fileUtils.h,v 1.5 2001/08/17 10:56:46 amai Exp $ */
int ParseFilename(const char *fullname, char *filename, char *pathname);
int ExpandTilde(char *pathname);
int NormalizePathname(char *pathname);
int CompressPathname(char *pathname);
