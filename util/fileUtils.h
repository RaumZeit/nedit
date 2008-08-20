/* $Id: fileUtils.h,v 1.13 2008/08/20 14:57:36 lebert Exp $ */
/*******************************************************************************
*                                                                              *
* fileUtils.h -- Nirvana Editor File Utilities Header File                     *
*                                                                              *
* Copyright 2003 The NEdit Developers                                          *
*                                                                              *
* This is free software; you can redistribute it and/or modify it under the    *
* terms of the GNU General Public License as published by the Free Software    *
* Foundation; either version 2 of the License, or (at your option) any later   *
* version. In addition, you may distribute versions of this program linked to  *
* Motif or Open Motif. See README for details.                                 *
*                                                                              *
* This software is distributed in the hope that it will be useful, but WITHOUT *
* ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or        *
* FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for    *
* more details.                                                                *
*                                                                              *
* You should have received a copy of the GNU General Public License along with *
* software; if not, write to the Free Software Foundation, Inc., 59 Temple     *
* Place, Suite 330, Boston, MA  02111-1307 USA                                 *
*                                                                              *
* Nirvana Text Editor                                                          *
* July 31, 2001                                                                *
*                                                                              *
*******************************************************************************/

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
char *ReadAnyTextFile(const char *fileName, int forceNL);

#endif /* NEDIT_FILEUTILS_H_INCLUDED */
