static const char CVSID[] = "$Id: fileUtils.c,v 1.23 2002/03/14 17:41:04 amai Exp $";
/*******************************************************************************
*									       *
* fileUtils.c -- File utilities for Nirvana applications		       *
*									       *
* Copyright (C) 1999 Mark Edel						       *
*									       *
* This is free software; you can redistribute it and/or modify it under the    *
* terms of the GNU General Public License as published by the Free Software    *
* Foundation; either version 2 of the License, or (at your option) any later   *
* version.							               *
* 									       *
* This software is distributed in the hope that it will be useful, but WITHOUT *
* ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or        *
* FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License        *
* for more details.							       *
* 									       *
* You should have received a copy of the GNU General Public License along with *
* software; if not, write to the Free Software Foundation, Inc., 59 Temple     *
* Place, Suite 330, Boston, MA  02111-1307 USA		                       *
*									       *
* Nirvana Text Editor	    						       *
* July 28, 1992								       *
*									       *
* Written by Mark Edel							       *
*									       *
* Modified by:	DMR - Ported to VMS (1st stage for Histo-Scope)		       *
*									       *
*******************************************************************************/

#ifdef HAVE_CONFIG_H
#include "../config.h"
#endif

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#ifdef VAXC
#define NULL (void *) 0
#endif /*VAXC*/
#ifdef VMS
#include "vmsparam.h"
#include <stat.h>
#else
#include <sys/types.h>
#ifndef __MVS__
#include <sys/param.h>
#endif
#include <sys/stat.h>
#include <unistd.h>
#include <pwd.h>
#endif /*VMS*/

#include "fileUtils.h"
#include "utils.h"

#ifdef HAVE_DEBUG_H
#include "../debug.h"
#endif

#ifndef MAXSYMLINKS  /* should be defined in <sys/param.h> */
#define MAXSYMLINKS 20
#endif

#define TRUE 1
#define FALSE 0

static char *nextSlash(char *ptr);
static char *prevSlash(char *ptr);
static int compareThruSlash(const char *string1, const char *string2);
static void copyThruSlash(char **toString, char **fromString);

/*
** Decompose a Unix file name into a file name and a path.
** Return non-zero value if it fails, zero else.
** For now we assume that filename and pathname are at
** least MAXPATHLEN chars long.
** To skip setting filename or pathname pass NULL for that argument.
*/
int
ParseFilename(const char *fullname, char *filename, char *pathname)
{
    int fullLen = strlen(fullname);
    int i, pathLen, fileLen;
	    
#ifdef VMS
    /* find the last ] or : */
    for (i=fullLen-1; i>=0; i--) {
    	if (fullname[i] == ']' || fullname[i] == ':')
	    break;
    }
#else  /* UNIX */
    char *viewExtendPath;
    int scanStart;
    
    /* For clearcase version extended paths, slash characters after the "@@/"
       should be considered part of the file name, rather than the path */
    if ((viewExtendPath = strstr(fullname, "@@/")) != NULL)
      	scanStart = viewExtendPath - fullname - 1;
    else
      	scanStart = fullLen - 1;
    
    /* find the last slash */
    for (i=scanStart; i>=0; i--) {
        if (fullname[i] == '/')
	    break;
    }
#endif

    /* move chars before / (or ] or :) into pathname,& after into filename */
    pathLen = i + 1;
    fileLen = fullLen - pathLen;
    if (pathname) {
      	if (pathLen > MAXPATHLEN) {
            return 1;
	}
      	strncpy(pathname, fullname, pathLen);
      	pathname[pathLen] = 0;
    }
    if (filename) {
      	if (fileLen > MAXPATHLEN) {
      	    return 2;
      	}
      	strncpy(filename, &fullname[pathLen], fileLen);
      	filename[fileLen] = 0;
    }

#ifdef VMS
    return 0;
#else     /* UNIX specific... Modify at a later date for VMS */
    if(pathname)
      	return NormalizePathname(pathname);
    else
      	return 0;
#endif
}

#ifndef VMS


/*
** Expand tilde characters which begin file names as done by the shell
** If it doesn't work just out leave pathname unmodified.
** This implementation is neither fast, nor elegant, nor ...
*/
int
ExpandTilde(char *pathname)
{
    struct passwd *passwdEntry;
    char username[MAXPATHLEN], temp[MAXPATHLEN];
    char *nameEnd;
    int len_left;
    
    if (pathname[0] != '~')
	return TRUE;
    nameEnd = strchr(&pathname[1], '/');
    if (nameEnd == NULL) {
	nameEnd = pathname + strlen(pathname);
    }
    strncpy(username, &pathname[1], nameEnd - &pathname[1]);
    username[nameEnd - &pathname[1]] = '\0';
    /* We might consider to re-use the GetHomeDir() function,
       but to keep the code more similar for both cases ... */
    if (username[0] == '\0') {
    	passwdEntry = getpwuid(getuid());
	if ((passwdEntry == NULL) || (*(passwdEntry->pw_dir)== '\0')) {
  	   /* This is really serious, so just exit. */
           perror("NEdit/nc: getpwuid() failed ");
           exit(EXIT_FAILURE);
	}
    }
    else {
    	passwdEntry = getpwnam(username);
        if ((passwdEntry == NULL) || (*(passwdEntry->pw_dir)== '\0')) {
           /* username was just an input by the user, this is no
	      indication for some (serious) problems */
           return FALSE;
	}
    }

    strcpy(temp, passwdEntry->pw_dir);
    strcat(temp, "/");
    len_left= sizeof(temp)-strlen(temp)-1;
    if (len_left < strlen(nameEnd)) {
      /* It won't work out */
       return FALSE;
    }
    strcat(temp, nameEnd);
    strcpy(pathname, temp);
    return TRUE;
}

/*
 * Resolve symbolic links (if any) for the absolute path given in pathIn 
 * and place the resolved absolute path in pathResolved. 
 * -  pathIn must contain an absolute path spec.
 * -  pathResolved must point to a buffer of minimum size MAXPATHLEN.
 *
 * Returns:
 *   TRUE  if pathResolved contains a valid resolved path
 *         OR pathIn is not a symlink (pathResolved will have the same
 *	      contents like pathIn)
 *
 *   FALSE an error occured while trying to resolve the symlink, i.e.
 *         pathIn was no absolute path or the link is a loop.
 */
int
ResolvePath(const char * pathIn, char * pathResolved) 
{
    char resolveBuf[MAXPATHLEN], pathBuf[MAXPATHLEN];
    char *pathEnd;
    int rlResult, loops;

#ifdef NO_READLINK
    strncpy(pathResolved, pathIn, MAXPATHLEN);
    /* If there are no links at all, it's a valid "resolved" path */
    return TRUE;
#else
    /* !! readlink does NOT recognize loops, i.e. links like file -> ./file */
    for(loops=0; loops<MAXSYMLINKS; loops++) {
#ifdef UNICOS
	rlResult=readlink((char *)pathIn, resolveBuf, MAXPATHLEN-1);
#else
	rlResult=readlink(pathIn, resolveBuf, MAXPATHLEN-1);
#endif
	if(rlResult<0) {
	    if(errno==EINVAL) { 
		/* It's not a symlink - we are done*/
		strncpy(pathResolved, pathIn, MAXPATHLEN);
		return TRUE;
	    } else {
	      	return FALSE; 
	    }
	} else if (rlResult==0) {
	    return FALSE;
	}

	resolveBuf[rlResult]=0;

	if(resolveBuf[0]!='/') {
	    strncpy(pathBuf, pathIn, MAXPATHLEN);
	    pathEnd=strrchr(pathBuf, '/');
	    if(!pathEnd)
	      	return FALSE;
	    strcpy(pathEnd+1, resolveBuf);
	} else {
	    strcpy(pathBuf, resolveBuf);
	}
	NormalizePathname(pathBuf);
	pathIn=pathBuf;
    }
    return FALSE;
#endif /* NO_READLINK */
}


/*
** Return 0 if everything's fine. In fact it always return 0 ...
** Capable to handle arbitrary path length (>MAXPATHLEN)!
*/
int
NormalizePathname(char *pathname)
{
    /* if this is a relative pathname, prepend current directory */
#ifdef __EMX__
    /* OS/2, ...: welcome to the world of drive letters ... */
    if (!_fnisabs(pathname)) {
#else
    if (pathname[0] != '/') {
#endif
        char *oldPathname;
        size_t len;

        /* make a copy of pathname to work from */
	oldPathname=(char *)malloc(strlen(pathname)+1);
	strcpy(oldPathname, pathname);
	/* get the working directory and prepend to the path */
	strcpy(pathname, GetCurrentDir());
	/* check for trailing slash, or pathname being root dir "/":
	   don't add a second '/' character as this may break things
	   on non-un*x systems */
	len=strlen(pathname); /* GetCurrentDir() returns non-NULL value */
	if ( len==0 ? 1 : pathname[len-1] != '/' ) {
	   strcat(pathname, "/");
	}
	strcat(pathname, oldPathname);
	free(oldPathname);
    }

    /* compress out .. and . */
    return CompressPathname(pathname);
}


/*
** Return 0 if everything's fine, 1 else.
*/
int
CompressPathname(char *pathname)
{
    char *buf, *inPtr, *outPtr;
    struct stat statbuf;

    /* (Added by schwarzenberg)
    ** replace multiple slashes by a single slash 
    */
    inPtr=pathname;
    buf=(char *)malloc(strlen(pathname)+2);
    outPtr=buf;
    while (*inPtr) {
      	*outPtr=*inPtr++;
      	if(*outPtr=='/') {
	    while(*inPtr=='/')
	      	inPtr++;
      	}
      	outPtr++;
    }
    *outPtr=0;
    strcpy(pathname, buf);
    
    /* compress out . and .. */
    inPtr = pathname;
    outPtr = buf;
    /* copy initial / */
    copyThruSlash(&outPtr, &inPtr);
    while (inPtr != NULL) {
	/* if the next component is "../", remove previous component */
	if (compareThruSlash(inPtr, "../")) {
		*outPtr = 0;
	    /* If the ../ is at the beginning, or if the previous component
	       is a symbolic link, preserve the ../.  It is not valid to
	       compress ../ when the previous component is a symbolic link
	       because ../ is relative to where the link points.  If there's
	       no S_ISLNK macro, assume system does not do symbolic links. */
#ifdef S_ISLNK
	    if(outPtr-1 == buf || (lstat(buf, &statbuf) == 0 &&
		    S_ISLNK(statbuf.st_mode))) {
		copyThruSlash(&outPtr, &inPtr);
	    } else
#endif	    
	    {
		/* back up outPtr to remove last path name component */
		outPtr = prevSlash(outPtr);
		inPtr = nextSlash(inPtr);
	    }
	} else if (compareThruSlash(inPtr, "./")) {
	    /* don't copy the component if it's the redundant "./" */
	    inPtr = nextSlash(inPtr);
	} else {
	    /* copy the component to outPtr */
	    copyThruSlash(&outPtr, &inPtr);
	}
    }
    /* updated pathname with the new value */
    if (strlen(buf)>MAXPATHLEN) {
       fprintf(stderr, "NEdit: CompressPathname(): file name too long %s\n",
               pathname);
       free(buf);
       return 1;
    }
    else {
       strcpy(pathname, buf);
       free(buf);
       return 0;
    }
}

static char
*nextSlash(char *ptr)
{
    for(; *ptr!='/'; ptr++) {
    	if (*ptr == '\0')
	    return NULL;
    }
    return ptr + 1;
}

static char
*prevSlash(char *ptr)
{
    for(ptr -= 2; *ptr!='/'; ptr--);
    return ptr + 1;
}

static int
compareThruSlash(const char *string1, const char *string2)
{
    while (TRUE) {
    	if (*string1 != *string2)
	    return FALSE;
	if (*string1 =='\0' || *string1=='/')
	    return TRUE;
	string1++;
	string2++;
    }
}

static void
copyThruSlash(char **toString, char **fromString)
{
    char *to = *toString;
    char *from = *fromString;
    
    while (TRUE) {
        *to = *from;
        if (*from =='\0') {
            *fromString = NULL;
            return;
        }
	if (*from=='/') {
	    *toString = to + 1;
	    *fromString = from + 1;
	    return;
	}
	from++;
	to++;
    }
}
#endif /* UNIX */

/*
** Return the trailing 'n' no. of path components
*/
const char
*GetTrailingPathComponents(const char* path,
                                      int noOfComponents)
{
    /* Start from the rear */
    const char* ptr = path + strlen(path);
    int count = 0;

    while (--ptr > path) {
        if (*ptr == '/') {
            if (count++ == noOfComponents) {
                break;
            }
        }
    }
    return(ptr);
} 
