/*******************************************************************************
*									       *
* nc.c -- Nirvana Editor client program for nedit server processes	       *
*									       *
* Copyright (c) 1991 Universities Research Association, Inc.		       *
* All rights reserved.							       *
* 									       *
* This material resulted from work developed under a Government Contract and   *
* is subject to the following license:  The Government retains a paid-up,      *
* nonexclusive, irrevocable worldwide license to reproduce, prepare derivative *
* works, perform publicly and display publicly by or for the Government,       *
* including the right to distribute to other Government contractors.  Neither  *
* the United States nor the United States Department of Energy, nor any of     *
* their employees, makes any warrenty, express or implied, or assumes any      *
* legal liability or responsibility for the accuracy, completeness, or         *
* usefulness of any information, apparatus, product, or process disclosed, or  *
* represents that its use would not infringe privately owned rights.           *
*                                        				       *
* Fermilab Nirvana GUI Library						       *
* November, 1995							       *
*									       *
* Written by Mark Edel							       *
*									       *
*******************************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <limits.h>
#ifdef VMS
#include <lib$routines.h>
#include descrip
#include ssdef
#include syidef
#include "../util/VMSparam.h"
#include "../util/VMSutils.h"
#else
#include <sys/param.h>
#include <sys/utsname.h>
#include <sys/types.h>
#include <unistd.h>
#include <pwd.h>
#endif
#include <X11/Intrinsic.h>
#include <X11/Xatom.h>
#include "../util/fileUtils.h"
#include "../util/prefFile.h"

/* If anyone knows where to get this from system include files (in a machine
   independent way), please change this (L_cuserid is apparently not ANSI) */
#define MAXUSERNAMELEN 32

#define APP_NAME "nc"
#define APP_CLASS "NEditClient"

#if defined(VMS) || defined(linux)
#define MAXNODENAMELEN (MAXPATHLEN+2)
#elif defined(SUNOS)
#define MAXNODENAMELEN 9
#else
#define MAXNODENAMELEN SYS_NMLN
#endif

static void deadServerTimerProc(XtPointer clientData, XtIntervalId *id);
static void startServer(char *message, char *commandLine);
static char *parseCommandLine(int argc, char **argv);
static char *getUserName(void);
static void getHostName(char *hostname);
static void nextArg(int argc, char **argv, int *argIndex);

Display *TheDisplay;

static char cmdLineHelp[] =
#ifndef VMS
"Usage:  nc [-read] [-create] [-line n | +n] [-do command] [-[no]ask] \n\
           [-svrname name] [file...]\n";
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
};

int main(int argc, char **argv)
{
    XtAppContext context;
    Window rootWindow;
    int i, length = 0, commandLineLen, getFmt;
    char *commandString, *commandLine, *outPtr, *c;
    unsigned char *propValue;
    Atom dummyAtom;
    unsigned long dummyULong, nItems;
    XEvent event;
    XPropertyEvent *e = (XPropertyEvent *)&event;
    Atom serverExistsAtom, serverRequestAtom;
    char *userName, propName[24+MAXNODENAMELEN+MAXUSERNAMELEN+MAXPATHLEN];
    char hostName[MAXNODENAMELEN+1];
    char serverName[MAXPATHLEN+2];
    XrmDatabase prefDB;

    /* Initialize toolkit and get an application context */
    XtToolkitInitialize();
    context = XtCreateApplicationContext();
    
#ifdef VMS
    /* Convert the command line to Unix style */
    ConvertVMSCommandLine(&argc, &argv);
#endif /*VMS*/
    
    /* Read the preferences command line and (very) optional .nc file
       into a database */
    prefDB = CreatePreferencesDatabase(".nc", APP_CLASS, 
	    OpTable, XtNumber(OpTable), (unsigned *)&argc, argv);

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
    	    
    /* Convert command line arguments into a command string for the server */
    commandString = parseCommandLine(argc, argv);
    
    /* Reconstruct the command line in string commandLine in case we have to
       start a server (nc command line args parallel nedit's).  Include
       -svrname if nc wants a named server, so nedit will match. Special
       characters are protected from the shell by escaping EVERYTHING with \ */
    for (i=1; i<argc; i++)
    	length += 1 + strlen(argv[i])*2;
    commandLine = XtMalloc(length+1 + 9+strlen(Preferences.serverName));
    outPtr = commandLine;
    for (i=1; i<argc; i++) {
    	for (c=argv[i]; *c!='\0'; c++) {
#ifndef VMS
    	    *outPtr++ = '\\';
#endif
    	    *outPtr++ = *c;
    	}
    	*outPtr++ = ' ';
    }
    if (Preferences.serverName[0] != '\0') {
    	sprintf(outPtr, "-svrname %s", Preferences.serverName);
    	outPtr += 9+strlen(Preferences.serverName);
    }
    *outPtr = '\0';
    commandLineLen = outPtr - commandLine;
        
    /* Create server property atoms.  Atom names are generated by
       concatenating NEDIT_SERVER_REQUEST_, and NEDIT_SERVER_EXITS_
       with hostname,  user name and optional server name */
    userName = getUserName();
    getHostName(hostName);
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
    	startServer("No servers available, start one (yes)? ", commandLine);
    	return 0;
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
    startServer("No servers responding, start one (yes)? ", (char *)clientData);
    exit(0);
}

/*
** Prompt the user about starting a server, with "message", then start server
*/
static void startServer(char *message, char *commandLineArgs)
{
    char c, *commandLine;
#ifdef VMS
    int spawnFlags = 1 /* + 1<<3 */;			/* NOWAIT, NOKEYPAD */
    int spawn_sts;
    struct dsc$descriptor_s *cmdDesc;
    char *nulDev = "NL:";
    struct dsc$descriptor_s *nulDevDesc;
#endif /*VMS*/
    
    /* prompt user whether to start server */
    if (!Preferences.autoStart) {
	printf(message);
	do {
    	    c = getc(stdin);
	} while (c == ' ' || c == '\t');
	if (c != 'Y' && c != 'y' && c != '\n')
    	    return;
    }
    
    /* start the server */
    commandLine = XtMalloc(strlen(Preferences.serverCmd) +
    	    strlen(commandLineArgs) + 3);
#ifdef VMS
    sprintf(commandLine, "%s %s", Preferences.serverCmd, commandLineArgs);
    cmdDesc = NulStrToDesc(commandLine);	/* build command descriptor */
    nulDevDesc = NulStrToDesc(nulDev);		/* build "NL:" descriptor */
    spawn_sts = lib$spawn(cmdDesc, nulDevDesc, 0, &spawnFlags, 0,0,0,0,0,0,0,0);
    if (spawn_sts != SS$_NORMAL) {
	fprintf(stderr, "Error return from lib$spawn: %d\n", spawn_sts);
	fprintf(stderr, "Nedit server not started.\n");
	return;
    }
    FreeStrDesc(cmdDesc);
#else
    sprintf(commandLine, "%s %s&", Preferences.serverCmd, commandLineArgs);
    system(commandLine);
#endif
    XtFree(commandLine);
}

/*
** Converts command line into a command string suitable for passing to
** the server
*/
static char *parseCommandLine(int argc, char **argv)
{
    char name[MAXPATHLEN], path[MAXPATHLEN];
    char *toDoCommand = "";
    char *commandString, *outPtr;
    int lineNum = 0, read = 0, create = 0, length = 0;
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
    	} else if (!strcmp(argv[i], "-read")) {
    	    read = 1;
    	} else if (!strcmp(argv[i], "-create")) {
    	    create = 1;
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
    	} else if (*argv[i] == '-') {
#ifdef VMS
	    *argv[i] = '/';
#endif /*VMS*/
    	    fprintf(stderr, "nc: Unrecognized option %s\n%s", argv[i],
    	    	    cmdLineHelp);
    	    exit(0);
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
    		sprintf(outPtr, "%d %d %d %d %d\n%s\n%s\n%n", lineNum, read,
    			create, strlen(path), strlen(toDoCommand), path,
    			toDoCommand, &charsWritten);
		outPtr += charsWritten;
		free(nameList[j]);
	    }
	    if (nameList != NULL)
	    	free(nameList);
#else
    	    ParseFilename(argv[i], name, path);
    	    strcat(path, name);
    	    /* SunOS acc or acc and/or its runtime library has a bug
    	       such that %n fails (segv) if it follows a string in a
    	       printf or sprintf.  The silly code below avoids this */
    	    sprintf(outPtr, "%d %d %d %d %d\n%n", lineNum, read, create,
    	    	    strlen(path), strlen(toDoCommand), &charsWritten);
    	    outPtr += charsWritten;
    	    strcpy(outPtr, path);
    	    outPtr += strlen(path);
    	    *outPtr++ = '\n';
    	    strcpy(outPtr, toDoCommand);
    	    outPtr += strlen(toDoCommand);
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
	sprintf(outPtr, "0 0 0 0 %d\n\n%n", strlen(toDoCommand), &charsWritten);
	outPtr += charsWritten;
	strcpy(outPtr, toDoCommand);
	outPtr += strlen(toDoCommand);
	*outPtr++ = '\n';
    }
    
    *outPtr = '\0';
    return commandString;
}

/*
** Return a pointer to the username of the current user in a statically
** allocated string.
*/
static char *getUserName(void)
{
#ifdef VMS
    return cuserid(NULL);
#else
    /* This should be simple, but cuserid has apparently been dropped from
       the ansi C standard, and if strict ansi compliance is turned on (on
       Sun anyhow, maybe others), calls to cuserid fail to compile.
       Unfortunately the alternative is this weird sequence of getlogin
       followed by getpwuid.  Getlogin only works if a terminal is attached &
       there can be more than one name associated with a uid (really?).  Both
       calls return a pointer to a static area. */
    char *name;
    struct passwd *passwdEntry;
    
    name = getlogin();
    if (name == NULL || name[0] == '\0') {
    	passwdEntry = getpwuid(getuid());
    	name = passwdEntry->pw_name;
    }
    return name;
#endif
}

/*
** Writes the hostname of the current system in string "hostname".
*/
static void getHostName(char *hostname)
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

    uname(&nameStruct);
    strcpy(hostname, nameStruct.nodename);
#endif
}

static void nextArg(int argc, char **argv, int *argIndex)
{
    if (*argIndex + 1 >= argc) {
#ifdef VMS
	    *argv[*argIndex] = '/';
#endif /*VMS*/
    	fprintf(stderr, "NEdit: %s requires an argument\n%s", argv[*argIndex],
    	        cmdLineHelp);
    	exit(1);
    }
    (*argIndex)++;
}
