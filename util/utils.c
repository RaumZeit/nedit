static const char CVSID[] = "$Id: utils.c,v 1.11 2002/03/14 01:25:24 amai Exp $";
/*******************************************************************************
*                                                                              *
* utils.c -- miscellaneous non-GUI routines                                    *
*                                                                              *
*                                                                              *
* This is free software; you can redistribute it and/or modify it under the    *
* terms of the GNU General Public License as published by the Free Software    *
* Foundation; either version 2 of the License, or (at your option) any later   *
* version.							                               *
*                                                                              *
* This software is distributed in the hope that it will be useful, but WITHOUT *
* ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or        *
* FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License        *
* for more details.*                                                           *
*                                                                              *
* You should have received a copy of the GNU General Public License along with *
* software; if not, write to the Free Software Foundation, Inc., 59 Temple     *
* Place, Suite 330, Boston, MA  02111-1307 USA		                         *
*                                                                              *
*******************************************************************************/

#ifdef HAVE_CONFIG_H
#include "../config.h"
#endif

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#ifdef VMS
#include <lib$routines.h>
#include ssdef
#include syidef
#include "../util/VMSparam.h"
#include "../util/VMSutils.h"
#endif
#include <sys/types.h>
#include <sys/utsname.h>
#ifdef VMS
#include "vmsparam.h"
#else
#include <sys/param.h>
#endif /*VMS*/
#include <pwd.h>

/* just to get 'Boolean' types defined: */
#include <X11/Xlib.h>

#include "utils.h"


extern const char
*GetCurrentDir(void)
/* return non-NULL value for the current working directory.
   If system call fails, provide a fallback value */
{
  static char curdir[MAXPATHLEN];

  if (!getcwd(curdir, MAXPATHLEN)) {
     perror("NEdit: getcwd() fails");
     strcpy(curdir, ".");
  }
  return (curdir);
}


extern const char
*GetHomeDir(void)
/* return a non-NULL value for the user's home directory,
   without trailing slash.
   We try the  environment var and the system user database. */
{
    const char *ptr;
    static char homedir[MAXPATHLEN]="";
    struct passwd *passwdEntry;
    int len;

    if (*homedir) {
       return homedir;
    }
    ptr=getenv("HOME");
    if (!ptr) {
       passwdEntry = getpwuid(getuid());
       if (passwdEntry && *(passwdEntry->pw_dir)) {
          ptr= passwdEntry->pw_dir;
       } else {
          /* This is really serious, so just exit. */
          perror("NEdit/nc: getpwuid() failed ");
          exit(EXIT_FAILURE);
       }
    }
    strncpy(homedir, ptr, sizeof(homedir)-1);
    homedir[sizeof(homedir)-1]='\0';
    /* Fix trailing slash */
    len=strlen(homedir);
    if (len>1 && homedir[len-1]=='/') {
       homedir[len-1]='\0';
    }
    return homedir;
}

/*
** Return a pointer to the username of the current user in a statically
** allocated string.
*/
const char
*GetUserName(void)
{
#ifdef VMS
    return cuserid(NULL);
#else
    /* cuserid has apparently been dropped from the ansi C standard, and if
       strict ansi compliance is turned on (on Sun anyhow, maybe others), calls
       to cuserid fail to compile.  Older versions of nedit try to use the
       getlogin call first, then if that fails, use getpwuid and getuid.  This
       results in the user-name of the original terminal being used, which is
       not correct when the user uses the su command.  Now, getpwuid only: */

    const struct passwd *passwdEntry;
    static char *userName=NULL;
    
    if (userName)
       return userName;
    
    passwdEntry = getpwuid(getuid());
    if (!passwdEntry) {
       /* This is really serious, so just exit. */
       perror("NEdit/nc: getpwuid() failed ");
       exit(EXIT_FAILURE);
    }
    userName=malloc(strlen(passwdEntry->pw_name)+1);
    strcpy(userName, passwdEntry->pw_name);
    return userName;
#endif /* VMS */
}


/*
** Writes the hostname of the current system in string "hostname".
*/
const char
*GetHostName(void)
{
    static char hostname[MAXNODENAMELEN+1];
    static int  hostnameFound = False;
    
    if (!hostnameFound) {
#ifdef VMS
        /* This should be simple, but uname is not supported in the DEC C RTL and
           gethostname on VMS depends either on Multinet or UCX.  So use uname 
           on Unix, and use LIB$GETSYI on VMS. Note the VMS hostname will
           be in DECNET format with trailing double colons, e.g. "FNALV1::".    */
        int syi_status;
        struct dsc$descriptor_s *hostnameDesc;
        unsigned long int syiItemCode = SYI$_NODENAME;	/* get Nodename */
        unsigned long int unused = 0;
        unsigned short int hostnameLen = MAXNODENAMELEN+1;

        hostnameDesc = NulStrWrtDesc(hostname, MAXNODENAMELEN+1);
        syi_status = lib$getsyi(&syiItemCode, &unused, hostnameDesc, &hostnameLen,
    			        0, 0);
        if (syi_status != SS$_NORMAL) {
	    fprintf(stderr, "Error return from lib$getsyi: %d", syi_status);
	    strcpy(hostname, "VMS");
        } else
    	    hostname[hostnameLen] = '\0';
        FreeStrDesc(hostnameDesc);
#else
        struct utsname nameStruct;
        int rc = uname(&nameStruct);
        if (rc<0) {
            /* Shouldn't ever happen, so we better exit() here */
           perror("NEdit/nc: uname() failed ");
           exit(EXIT_FAILURE);
        }
        strcpy(hostname, nameStruct.nodename);
#endif /* VMS */
        hostnameFound = True;
    }
    return hostname;
}


/*
** Create a path: $HOME/filename
** Return "" if it doesn't fit into the buffer
*/
char 
*PrependHome(const char *filename, char *buf, int buflen)
{
    const char *homedir;
    int home_len, file_len;
    
    homedir=GetHomeDir();
    home_len=strlen(homedir);
    file_len=strlen(filename);
    if ( (home_len+1+file_len)>=buflen ) {
       buf[0]='\0';
    }
    else {
       strcpy(buf, homedir);
       strcat(buf, "/");
       strcat(buf, filename);
    }
    return buf;
}

extern int Max(int i1, int i2)
{
    return i1 >= i2 ? i1 : i2;
}

extern int Min(int i1, int i2)
{
    return i1 <= i2 ? i1 : i2;
}

extern int Min3(int i1, int i2, int i3)
{
    if (i1 <= i2 && i1 <= i3)
    	return i1;
    return i2 <= i3 ? i2 : i3;
}

extern int Max3(int i1, int i2, int i3)
{
    if (i1 >= i2 && i1 >= i3)
    	return i1;
    return i2 >= i3 ? i2 : i3;
}
