/* $Id: utils.h,v 1.8 2002/07/05 22:28:11 uid71894 Exp $ */

#ifndef UTILS_H_INCLUDED
#define UTILS_H_INCLUDED

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

#endif /* UTILS_H_INCLUDED */
