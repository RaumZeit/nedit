static const char CVSID[] = "$Id: utils.c,v 1.3 2001/02/26 23:38:03 edg Exp $";
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
* FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Lesser General Public License *
* for more details.*                                                           *
*                                                                              *
* You should have received a copy of the GNU General Public License along with *
* software; if not, write to the Free Software Foundation, Inc., 59 Temple     *
* Place, Suite 330, Boston, MA  02111-1307 USA		                         *
*                                                                              *
*******************************************************************************/

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/param.h>
#include <pwd.h>

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
/* return a non-NULL value for the user's home directory.
   We try really hard:
   environment var, system user database and finally some fallback */
{
    const char *ptr;
    struct passwd *passwdEntry;

    ptr=getenv("HOME");
    if (ptr) {
       return ptr;
    }
    passwdEntry = getpwuid(getuid());
    if (passwdEntry) {
       return (passwdEntry->pw_dir);
    }
    ptr=GetCurrentDir();
    return (ptr);
}
