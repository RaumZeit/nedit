static const char CVSID[] = "$Id: nc.c,v 1.11 2001/08/27 09:08:07 amai Exp $";
/*******************************************************************************
*									       *
* nc.c -- Nirvana Editor client program for nedit server processes	       *
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
* November, 1995							       *
*									       *
* Written by Mark Edel							       *
*									       *
*******************************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <limits.h>
#include <string.h>
#ifdef VMS
#include <lib$routines.h>
#include descrip
#include ssdef
#include syidef
#include "../util/VMSparam.h"
#include "../util/VMSutils.h"
#else
#ifndef __MVS__
#include <sys/param.h>
#endif
#include <sys/types.h>
#include <sys/utsname.h>
#include <unistd.h>
#include <pwd.h>
#endif
#ifdef __EMX__
#include <process.h>
#endif
#include <X11/Intrinsic.h>
#include <X11/Xatom.h>
#include "../util/fileUtils.h"
#include "../util/prefFile.h"

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

#define APP_NAME "nc"
#define APP_CLASS "NEditClient"

static void deadServerTimerProc(XtPointer clientData, XtIntervalId *id);
static int startServer(const char *message, const char *commandLine);
static char *parseCommandLine(int argc, char **argv);
static const char *getUserName(void);
static const char *getHostName(void);
static void nextArg(int argc, char **argv, int *argIndex);

static Display *TheDisplay;

static const char cmdLineHelp[] =
#ifndef VMS
"Usage:  nc [-read] [-create] [-line n | +n] [-do command] [-ask] [-noask]\n\
           [-svrname name] [-svrcmd command] [-lm languagemode]\n\
           [-geometry geometry] [-iconic] [file...]\n";
#else
"";
#endif /*VMS*/

/* Structure to hold X Resource values */
static struct {
    int autoStart;
    char serverCmd[MAXPATHLEN];
    char serverName[MAXPATHLEN];
} Preferences;

/* Application resources */
static PrefDescripRec PrefDescrip[] = {
    {"autoStart", "AutoStart", PREF_BOOLEAN, "False",
      &Preferences.autoStart, NULL, True},
    {"serverCommand", "ServerCommand", PREF_STRING, "nedit -server",
      Preferences.serverCmd, (void *)sizeof(Preferences.serverCmd), False},
    {"serverName", "serverName", PREF_STRING, "", Preferences.serverName,
      (void *)sizeof(Preferences.serverName), False},
};

/* Resource related command line options */
static XrmOptionDescRec OpTable[] = {
    {"-ask", ".autoStart", XrmoptionNoArg, (caddr_t)"False"},
    {"-noask", ".autoStart", XrmoptionNoArg, (caddr_t)"True"},
    {"-svrname", ".serverName", XrmoptionSepArg, (caddr_t)NULL},
    {"-svrcmd", ".serverCommand", XrmoptionSepArg, (caddr_t)NULL},
};


int main(int argc, char **argv)
{
    XtAppContext context;
    Window rootWindow;
    int i, length = 0, getFmt;
    char *commandString, *commandLine, *outPtr, *c;
    unsigned char *propValue;
    Atom dummyAtom;
    unsigned long dummyULong, nItems;
    XEvent event;
    XPropertyEvent *e = (XPropertyEvent *)&event;
    Atom serverExistsAtom, serverRequestAtom;
    const char *userName, *hostName;
    char propName[24+MAXNODENAMELEN+MAXUSERNAMELEN+MAXPATHLEN];
    char serverName[MAXPATHLEN+2];
    XrmDatabase prefDB;

    /* Initialize toolkit and get an application context */
    XtToolkitInitialize();
    context = XtCreateApplicationContext();
    
#ifdef VMS
    /* Convert the command line to Unix style */
    ConvertVMSCommandLine(&argc, &argv);
#endif /*VMS*/
#ifdef __EMX__
    /* expand wildcards if necessary */
    _wildcard(&argc, &argv);
#endif
    
    /* Read the preferences command line and (very) optional .nc file
       into a database */
    prefDB = CreatePreferencesDatabase(".nc", APP_CLASS, 
	    OpTable, XtNumber(OpTable), (unsigned *)&argc, argv);
    
    /* Reconstruct the command line in string commandLine in case we have to
       start a server (nc command line args parallel nedit's).  Include
       -svrname if nc wants a named server, so nedit will match. Special
       characters are protected from the shell by escaping EVERYTHING with \ */
    for (i=1; i<argc; i++)
    	length += 1 + strlen(argv[i])*2;
    commandLine = XtMalloc(length+1 + 9 + MAXPATHLEN);
    outPtr = commandLine;
    for (i=1; i<argc; i++) {
    	for (c=argv[i]; *c!='\0'; c++) {
#if !defined(VMS) && !defined(__EMX__) /* Non-Unix shells don't want/need esc */
    	    *outPtr++ = '\\';
#endif
    	    *outPtr++ = *c;
    	}
    	*outPtr++ = ' ';
    }
    *outPtr = '\0';
    	    
    /* Convert command line arguments into a command string for the server */
    commandString = parseCommandLine(argc, argv);

    /* Open the display and find the root window */
    TheDisplay = XtOpenDisplay (context, NULL, APP_NAME, APP_CLASS, NULL,
    	    0, &argc, argv);
    if (!TheDisplay) {
	XtWarning ("nc: Can't open display\n");
	return 1;
    }
    rootWindow = RootWindow(TheDisplay, DefaultScreen(TheDisplay));
    
    /* Read the application resources into the Preferences data structure */
    RestorePreferences(prefDB, XtDatabase(TheDisplay), APP_NAME,
    	    APP_CLASS, PrefDescrip, XtNumber(PrefDescrip));
    
    /* For Clearcase users who have not set a server name, use the clearcase
       view name.  Clearcase views make files with the same absolute path names
       but different contents (and therefore can't be edited in the same nedit
       session). This should have no bad side-effects for non-clearcase users */
    if (Preferences.serverName[0] == '\0') {
	char *envPtr, *tagPtr;
	envPtr = getenv("CLEARCASE_ROOT");
	if (envPtr != NULL) {
	    tagPtr = strrchr(envPtr, '/');
	    if (tagPtr != NULL && strlen(tagPtr+1) < MAXPATHLEN)
		strcpy(Preferences.serverName, tagPtr+1);
	}
    }
    
    /* Add back the server name resource from the command line or resource
       database to the command line for starting the server.  If -svrcmd
       appeared on the original command line, it was removed by
       CreatePreferencesDatabase before the command line was recorded
       in commandLine */
    if (Preferences.serverName[0] != '\0') {
    	sprintf(outPtr, "-svrname %s", Preferences.serverName);
    	outPtr += 9+strlen(Preferences.serverName);
    }
    *outPtr = '\0';
        
    /* Create server property atoms.  Atom names are generated by
       concatenating NEDIT_SERVER_REQUEST_, and NEDIT_SERVER_EXITS_
       with hostname,  user name and optional server name */
    userName = getUserName();
    hostName = getHostName();
    if (Preferences.serverName[0] != '\0') {
    	serverName[0] = '_';
    	strcpy(&serverName[1], Preferences.serverName);
    } else
    	serverName[0] = '\0';
    sprintf(propName, "NEDIT_SERVER_EXISTS_%s_%s%s", hostName, userName,
    	    serverName);
    serverExistsAtom = XInternAtom(TheDisplay, propName, False);
    sprintf(propName, "NEDIT_SERVER_REQUEST_%s_%s%s", hostName, userName,
    	    serverName);
    serverRequestAtom = XInternAtom(TheDisplay, propName, False);
    
    /* See if there might be a server (not a guaranty), by translating the
       root window property NEDIT_SERVER_EXISTS_<user>_<host> */
    if (XGetWindowProperty(TheDisplay, rootWindow, serverExistsAtom, 0,
    	    INT_MAX, False, XA_STRING, &dummyAtom, &getFmt, &nItems,
    	    &dummyULong, &propValue) != Success || nItems == 0) {
	int success;

	success=startServer("No servers available, start one (yes)? ", commandLine);
	return success;
    }
    XFree(propValue);

    /* Set the NEDIT_SERVER_REQUEST_<user>_<host> property on the root
       window to activate the server */
    XChangeProperty(TheDisplay, rootWindow, serverRequestAtom, XA_STRING, 8,
    	    PropModeReplace, (unsigned char *)commandString,
    	    strlen(commandString));
    
    /* Set up a timeout proc in case the server is dead.  The standard
       selection timeout is probably a good guess at how long to wait
       for this style of inter-client communication as well */
    XtAppAddTimeOut(context, XtAppGetSelectionTimeout(context),
    	    deadServerTimerProc, commandLine);
    	    
    /* Wait for the property to be deleted to know the request was processed */
    XSelectInput(TheDisplay, rootWindow, PropertyChangeMask);
    while (TRUE) {
        XtAppNextEvent(context, &event);
        if (e->window == rootWindow && e->atom == serverRequestAtom &&
        	    e->state == PropertyDelete)
            break;
    	XtDispatchEvent(&event);
    }
    XtFree(commandString);
    return 0;
}


/*
** Xt timer procedure for timeouts on NEdit server requests
*/
static void deadServerTimerProc(XtPointer clientData, XtIntervalId *id)
{   
    int success;
    
    success=startServer("No servers responding, start one (yes)? ",
            (char *)clientData);
    if (success==0) {
       exit(EXIT_SUCCESS);
    } else {
       exit(EXIT_FAILURE);
    }
}

/*
** Prompt the user about starting a server, with "message", then start server
*/
static int startServer(const char *message, const char *commandLineArgs)
{
    char c, *commandLine;
#ifdef VMS
    int spawnFlags = 1 /* + 1<<3 */;			/* NOWAIT, NOKEYPAD */
    int spawn_sts;
    struct dsc$descriptor_s *cmdDesc;
    char *nulDev = "NL:";
    struct dsc$descriptor_s *nulDevDesc;
#else
    int sysrc;
#endif /* !VMS */
    
    /* prompt user whether to start server */
    if (!Preferences.autoStart) {
	puts(message);
	do {
    	    c = getc(stdin);
	} while (c == ' ' || c == '\t');
	if (c != 'Y' && c != 'y' && c != '\n')
    	    return 0;
    }
    
    /* start the server */
#ifdef VMS
    commandLine = XtMalloc(strlen(Preferences.serverCmd) +
    	    strlen(commandLineArgs) + 3);
    sprintf(commandLine, "%s %s", Preferences.serverCmd, commandLineArgs);
    cmdDesc = NulStrToDesc(commandLine);	/* build command descriptor */
    nulDevDesc = NulStrToDesc(nulDev);		/* build "NL:" descriptor */
    spawn_sts = lib$spawn(cmdDesc, nulDevDesc, 0, &spawnFlags, 0,0,0,0,0,0,0,0);
    XtFree(commandLine);
    if (spawn_sts != SS$_NORMAL) {
	fprintf(stderr, "Error return from lib$spawn: %d\n", spawn_sts);
	fprintf(stderr, "Nedit server not started.\n");
	return (-1);
    } else {
       FreeStrDesc(cmdDesc);
       return 0;
    }
#else
#if defined(__EMX__)  /* OS/2 */
    /* Unfortunately system() calls a shell determined by the environment
       variables COMSPEC and EMXSHELL. We have to figure out which one */
    {
    char *sh, *base;
    commandLine = XtMalloc(strlen(Preferences.serverCmd) +
	   strlen(commandLineArgs) + 12);
    sh = getenv ("EMXSHELL");
    if (sh == NULL)
      sh = getenv ("COMSPEC");
    if (sh == NULL)
      sh = strdup("cmd.exe");
    base=_getname(strdup(sh));
    _remext(base);
    if (stricmp (base, "cmd") == 0 || stricmp (base, "4os2") == 0)
       sprintf(commandLine, "start /MIN %s %s",
               Preferences.serverCmd, commandLineArgs);
    else
       sprintf(commandLine, "%s %s &", 
               Preferences.serverCmd, commandLineArgs);
    }
#else /* Unix */
    commandLine = XtMalloc(strlen(Preferences.serverCmd) +
    	    strlen(commandLineArgs) + 3);
    sprintf(commandLine, "%s %s&", Preferences.serverCmd, commandLineArgs);
#endif

    sysrc=system(commandLine);
    XtFree(commandLine);
    
    if (sysrc==0)
       return 0;
    else
       return (-1);
#endif /* !VMS */
}

/*
** Converts command line into a command string suitable for passing to
** the server
*/
static char *parseCommandLine(int argc, char **argv)
{
    char name[MAXPATHLEN], path[MAXPATHLEN];
    char *toDoCommand = "", *langMode = "", *geometry = "";
    char *commandString, *outPtr;
    int lineNum = 0, read = 0, create = 0, iconic = 0, length = 0;
    int i, lineArg, nRead, charsWritten;
    
    /* Allocate a string for output, for the maximum possible length.  The
       maximum length is calculated by assuming every argument is a file,
       and a complete record of maximum length is created for it */
    for (i=1; i<argc; i++)
    	length += 38 + strlen(argv[i]) + MAXPATHLEN;
    commandString = XtMalloc(length+1);
    
    /* Parse the arguments and write the output string */
    outPtr = commandString;
    for (i=1; i<argc; i++) {
	if (!strcmp(argv[i], "-do")) {
    	    nextArg(argc, argv, &i);
    	    toDoCommand = argv[i];
    	} else if (!strcmp(argv[i], "-lm")) {
    	    nextArg(argc, argv, &i);
    	    langMode = argv[i];
    	} else if (!strcmp(argv[i], "-g") || !strcmp(argv[i], "-geometry")) {
    	    nextArg(argc, argv, &i);
    	    geometry = argv[i];
    	} else if (!strcmp(argv[i], "-read")) {
    	    read = 1;
    	} else if (!strcmp(argv[i], "-create")) {
    	    create = 1;
    	} else if (!strcmp(argv[i], "-iconic") || !strcmp(argv[i], "-icon")) {
    	    iconic = 1;
    	} else if (!strcmp(argv[i], "-line")) {
    	    nextArg(argc, argv, &i);
	    nRead = sscanf(argv[i], "%d", &lineArg);
	    if (nRead != 1)
    		fprintf(stderr, "nc: argument to line should be a number\n");
    	    else
    	    	lineNum = lineArg;
    	} else if (*argv[i] == '+') {
    	    nRead = sscanf((argv[i]+1), "%d", &lineArg);
	    if (nRead != 1)
    		fprintf(stderr, "nc: argument to + should be a number\n");
    	    else
    	    	lineNum = lineArg;
    	} else if (!strcmp(argv[i], "-ask") || !strcmp(argv[i], "-noAsk")) {
    	    ; /* Ignore resource-based arguments which are processed later */
    	} else if (!strcmp(argv[i], "-svrname") || !strcmp(argv[i], "-xrm") ||
		!strcmp(argv[i], "-svrcmd") || !strcmp(argv[i], "-display")) {
    	    nextArg(argc, argv, &i); /* Ignore rsrc args with data */
    	} else if (*argv[i] == '-') {
#ifdef VMS
	    *argv[i] = '/';
#endif /*VMS*/
    	    fprintf(stderr, "nc: Unrecognized option %s\n%s", argv[i],
    	    	    cmdLineHelp);
    	    exit(EXIT_FAILURE);
    	} else {
#ifdef VMS
	    int numFiles, j, oldLength;
	    char **nameList = NULL, *newCommandString;
	    /* Use VMS's LIB$FILESCAN for filename in argv[i] to process */
	    /* wildcards and to obtain a full VMS file specification     */
	    numFiles = VMSFileScan(argv[i], &nameList, NULL, INCLUDE_FNF);
	    /* for each expanded file name do: */
	    for (j = 0; j < numFiles; ++j) {
	    	oldLength = outPtr-commandString;
	    	newCommandString = XtMalloc(oldLength+length+1);
	    	strncpy(newCommandString, commandString, oldLength);
	    	XtFree(commandString);
	    	commandString = newCommandString;
	    	outPtr = newCommandString + oldLength;
	    	ParseFilename(nameList[j], name, path);
    		strcat(path, name);
                /* See below for casts */
    		sprintf(outPtr, "%d %d %d %d %ld %ld %ld %ld\n%s\n%s\n%s\n%s\n%n",
			lineNum, read, create, iconic, (long) strlen(path),
			(long) strlen(toDoCommand), (long) strlen(langMode), 
                        (long) strlen(geometry),
			path, toDoCommand, langMode, geometry, &charsWritten);
		outPtr += charsWritten;
		free(nameList[j]);
	    }
	    if (nameList != NULL)
	    	free(nameList);
#else
    	    ParseFilename(argv[i], name, path);
    	    strcat(path, name);
    	    /* SunOS 4 acc or acc and/or its runtime library has a bug
    	       such that %n fails (segv) if it follows a string in a
    	       printf or sprintf.  The silly code below avoids this.
               
               The "long" cast on strlen() is necessary because size_t
               is 64 bit on Alphas, and 32-bit on most others.  There is
               no printf format specifier for "size_t", thanx, ANSI. */

    	    sprintf(outPtr, "%d %d %d %d %ld %ld %ld %ld\n%n", lineNum, read,
		    create, iconic, (long) strlen(path), (long) strlen(toDoCommand),
		    (long) strlen(langMode), (long) strlen(geometry), &charsWritten);
    	    outPtr += charsWritten;
    	    strcpy(outPtr, path);
    	    outPtr += strlen(path);
    	    *outPtr++ = '\n';
    	    strcpy(outPtr, toDoCommand);
    	    outPtr += strlen(toDoCommand);
    	    *outPtr++ = '\n';
    	    strcpy(outPtr, langMode);
    	    outPtr += strlen(langMode);
    	    *outPtr++ = '\n';
    	    strcpy(outPtr, geometry);
    	    outPtr += strlen(geometry);
    	    *outPtr++ = '\n';
	    toDoCommand = "";
#endif
    	}
    }
#ifdef VMS
    VMSFileScanDone();
#endif /*VMS*/
    
    /* If ther's an un-written -do command, write it with an empty file name */
    if (toDoCommand[0] != '\0') {
	sprintf(outPtr, "0 0 0 0 0 %d 0 0\n\n%n", (int) strlen(toDoCommand),
		&charsWritten);
	outPtr += charsWritten;
	strcpy(outPtr, toDoCommand);
	outPtr += strlen(toDoCommand);
	*outPtr++ = '\n';
	*outPtr++ = '\n';
	*outPtr++ = '\n';
    }
    
    *outPtr = '\0';
    return commandString;
}

/*
** Return a pointer to the username of the current user in a statically
** allocated string.
*/
static const char *getUserName(void)
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
    struct passwd *passwdEntry;

    passwdEntry = getpwuid(getuid());
    if (!passwdEntry) {
       perror("nc: getpwuid() failed ");
       exit(EXIT_FAILURE);
    }
    return passwdEntry->pw_name;
#endif
}

/*
** Returns the hostname of the current system in static string.
*/
static const char *getHostName(void)
{
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
    static char hostname[MAXNODENAMELEN+1];
    
    hostnameDesc = NulStrWrtDesc(hostname, MAXNODENAMELEN+1);
    syi_status = lib$getsyi(&syiItemCode, &unused, hostnameDesc, &hostnameLen,
    			    0, 0);
    if (syi_status != SS$_NORMAL) {
	fprintf(stderr, "Error return from lib$getsyi: %d", syi_status);
	strcpy(hostname, "VMS");
    } else
    	hostname[hostnameLen] = '\0';
    FreeStrDesc(hostnameDesc);
    return hostname;
#else
    struct utsname nameStruct;
    static char hostname[MAXNODENAMELEN+1];
    int rc;
    
    rc=uname(&nameStruct);
    if (rc<0) {
       /* Shouldn't ever happen, so we better exit() here */
       perror("nc: uname() failed ");
       exit(EXIT_FAILURE);
    }
    strcpy(hostname, nameStruct.nodename);
    return hostname;
#endif
}

static void nextArg(int argc, char **argv, int *argIndex)
{
    if (*argIndex + 1 >= argc) {
#ifdef VMS
	    *argv[*argIndex] = '/';
#endif /*VMS*/
    	fprintf(stderr, "nc: %s requires an argument\n%s",
	        argv[*argIndex], cmdLineHelp);
    	exit(EXIT_FAILURE);
    }
    (*argIndex)++;
}
