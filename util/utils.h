/* $Id: utils.h,v 1.13 2004/11/09 21:58:45 yooden Exp $ */
/*******************************************************************************
*                                                                              *
* utils.h -- Nirvana Editor Utilities Header File                              *
*                                                                              *
* Copyright 2002 The NEdit Developers                                          *
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

#ifndef NEDIT_UTILS_H_INCLUDED
#define NEDIT_UTILS_H_INCLUDED

#include <sys/utsname.h>

#ifdef VMS
#include "vmsparam.h"
#else
#include <sys/param.h>
#endif /*VMS*/

extern const char *GetCurrentDir(void);
extern const char *GetHomeDir(void);
extern char *PrependHome(const char *filename, char *buf, int buflen);
const char *GetUserName(void);
const char *GetNameOfHost(void);
extern int Max(int i1, int i2);
extern int Min(int i1, int i2);
extern int Min3(int i1, int i2, int i3);
extern int Max3(int i1, int i2, int i3);
const char* GetRCFileName(int type);

/* N_FILE_TYPES must be the last entry!! This saves us from counting. */
enum {NEDIT_RC, AUTOLOAD_NM, NEDIT_HISTORY, N_FILE_TYPES};

/* If anyone knows where to get this from system include files (in a machine
   independent way), please change this (L_cuserid is apparently not ANSI) */
#define MAXUSERNAMELEN 32

/* Ditto for the maximum length for a node name.  SYS_NMLN is not available
   on most systems, and I don't know what the portable alternative is. */
#ifdef SYS_NMLN
#define MAXNODENAMELEN SYS_NMLN
#else
#define MAXNODENAMELEN (MAXPATHLEN+2)
#endif

#endif /* NEDIT_UTILS_H_INCLUDED */
