/* $Id: utils.h,v 1.3 2001/11/18 19:02:58 arnef Exp $ */
#ifndef _UTILS_H
#define _UTILS_H

extern const char *GetCurrentDir(void);

extern const char *GetHomeDir(void);

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

const char *GetUserName(void);
const char *GetHostName(void);

#endif /* _UTILS_H */
