/* $Id: fileUtils.h,v 1.10 2003/04/03 19:05:37 jlous Exp $ */

#ifndef NEDIT_FILEUTILS_H_INCLUDED
#define NEDIT_FILEUTILS_H_INCLUDED

enum fileFormats {UNIX_FILE_FORMAT, DOS_FILE_FORMAT, MAC_FILE_FORMAT};

int ParseFilename(const char *fullname, char *filename, char *pathname);
int ExpandTilde(char *pathname);
const char* GetTrailingPathComponents(const char* path,
                                      int noOfComponents);
int NormalizePathname(char *pathname);
int CompressPathname(char *pathname);
int ResolvePath(const char * pathIn, char * pathResolved); 

int FormatOfFile(const char *fileString);
void ConvertFromDosFileString(char *inString, int *length, 
     char* pendingCR);
void ConvertFromMacFileString(char *fileString, int length);
int ConvertToDosFileString(char **fileString, int *length);
void ConvertToMacFileString(char *fileString, int length);
char *ReadAnyTextFile(const char *fileName);

#endif /* NEDIT_FILEUTILS_H_INCLUDED */
