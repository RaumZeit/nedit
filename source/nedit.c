static const char CVSID[] = "$Id: nedit.c,v 1.32 2002/06/08 13:56:51 tringali Exp $";
/*******************************************************************************
*									       *
* nedit.c -- Nirvana Editor main program				       *
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
* May 10, 1991								       *
*									       *
* Written by Mark Edel							       *
*									       *
* Modifications:							       *
*									       *
*	8/18/93 - Mark Edel & Joy Kyriakopulos - Ported to VMS		       *
*									       *
*******************************************************************************/

#ifdef HAVE_CONFIG_H
#include "../config.h"
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifndef NO_XMIM
#include <X11/Xlocale.h>
#else
#include <locale.h>
#endif
#include <X11/Intrinsic.h>
#include <Xm/Xm.h>
#if XmVersion >= 1002
#include <Xm/RepType.h>
#endif
#ifdef VMS
#include <rmsdef.h>
#include "../util/VMSparam.h"
#include "../util/VMSUtils.h"
#else
#ifndef __MVS__
#include <sys/param.h>
#endif
#endif /*VMS*/

#include "../util/misc.h"
#include "../util/printUtils.h"
#include "../util/fileUtils.h"
#include "../util/getfiles.h"
#include "textBuf.h"
#include "nedit.h"
#include "file.h"
#include "preferences.h"
#include "regularExp.h"
#include "selection.h"
#include "tags.h"
#include "menu.h"
#include "macro.h"
#include "server.h"
#include "rbTree.h"
#include "interpret.h"
#include "parse.h"
#include "help.h"

#ifdef HAVE_DEBUG_H
#include "../debug.h"
#endif


static void nextArg(int argc, char **argv, int *argIndex);
static int checkDoMacroArg(const char *macro);
static void maskArgvKeywords(int argc, char **argv, const char **maskArgs);
static void unmaskArgvKeywords(int argc, char **argv, const char **maskArgs);

WindowInfo *WindowList = NULL;
Display *TheDisplay = NULL;
char *ArgV0 = NULL;
Boolean IsServer = False;

/* iso8859 appears to be necessary for newer versions of XFree86 that
   default to Unicode encoding, which doesn't quite work with Motif.
   Otherwise Motif puts up garbage (square blocks).

   (This of course, is a stupid default because there are far more iso8859
   apps than Unicode apps.  But the X folks insist it's a client bug.  Hah.) */

#define NEDIT_DEFAULT_FONT "-*-helvetica-medium-r-normal-*-*-120-*-*-*-iso8859-*"
#define NEDIT_FIXED_FONT   "-*-courier-medium-r-normal-*-*-120-*-*-*-iso8859-*"

static char *fallbackResources[] = {
    "*menuBar.marginHeight: 0",
    "*menuBar.shadowThickness: 1",
    "*pane.sashHeight: 11",
    "*pane.sashWidth: 11",
    "*text.selectionArrayCount: 3",
    "nedit*fontList: " NEDIT_DEFAULT_FONT,
    "nedit*XmList.fontList: " NEDIT_FIXED_FONT,
    /* This should not be necessary, but some default in LessTif is
       overriding the resource above, and specifying the app-name fixes it */
    "nedit*XmText.fontList: " NEDIT_FIXED_FONT,
    /* Same with this, both Solaris Motif and LessTif seem to have some
       very specific defaults for file selection box fonts */
    "nedit*XmFileSelectionBox*XmList.fontList: " NEDIT_FIXED_FONT,
    "nedit*XmTextField.fontList: " NEDIT_FIXED_FONT,
    "nedit*background: #b3b3b3",
    "nedit*foreground: black",
    "nedit*text.lineNumForeground: #777777",
    "nedit*text.background: #e5e5e5",
    "nedit*text.foreground: black",
    "nedit*text.highlightBackground: red",
    "nedit*text.highlightForeground: black",
    "nedit*XmText*foreground: black",
    "nedit*XmText*background: #cccccc",
    "nedit*helpText.background: #cccccc",
    "nedit*helpText.foreground: black",
    "nedit*helpText.selectBackground: #b3b3b3",
    "nedit*statsLine.background: #b3b3b3",
    "nedit*statsLine.fontList: " NEDIT_DEFAULT_FONT,
    "nedit*helpText.font: " NEDIT_FIXED_FONT,
    "*XmText.translations: #override \
Ctrl~Alt~Meta<KeyPress>v: paste-clipboard()\\n\
Ctrl~Alt~Meta<KeyPress>c: copy-clipboard()\\n\
Ctrl~Alt~Meta<KeyPress>x: cut-clipboard()\\n\
Ctrl~Alt~Meta<KeyPress>u: delete-to-start-of-line()\\n",
    "*XmTextField.translations: #override \
Ctrl~Alt~Meta<KeyPress>v: paste-clipboard()\\n\
Ctrl~Alt~Meta<KeyPress>c: copy-clipboard()\\n\
Ctrl~Alt~Meta<KeyPress>x: cut-clipboard()\\n\
Ctrl~Alt~Meta<KeyPress>u: delete-to-start-of-line()\\n",
    "nedit*XmList*foreground: black",
    "nedit*XmList*background: #cccccc",
    "nedit*XmTextField*background: #cccccc",
    "nedit*XmTextField*foreground: black",
    "*iSearchForm*highlightThickness: 1",
    "*fileMenu.tearOffModel: XmTEAR_OFF_ENABLED",
    "*editMenu.tearOffModel: XmTEAR_OFF_ENABLED",
    "*searchMenu.tearOffModel: XmTEAR_OFF_ENABLED",
    "*preferencesMenu.tearOffModel: XmTEAR_OFF_ENABLED",
    "*windowsMenu.tearOffModel: XmTEAR_OFF_ENABLED",
    "*shellMenu.tearOffModel: XmTEAR_OFF_ENABLED",
    "*macroMenu.tearOffModel: XmTEAR_OFF_ENABLED",
    "*helpMenu.tearOffModel: XmTEAR_OFF_ENABLED",
    "*fileMenu.mnemonic: F",
    "*fileMenu.new.accelerator: Ctrl<Key>n",
    "*fileMenu.new.acceleratorText: Ctrl+N",
    "*fileMenu.open.accelerator: Ctrl<Key>o",
    "*fileMenu.open.acceleratorText: Ctrl+O",
    "*fileMenu.openSelected.accelerator: Ctrl<Key>y",
    "*fileMenu.openSelected.acceleratorText: Ctrl+Y",
    "*fileMenu.close.accelerator: Ctrl<Key>w",
    "*fileMenu.close.acceleratorText: Ctrl+W",
    "*fileMenu.save.accelerator: Ctrl<Key>s",
    "*fileMenu.save.acceleratorText: Ctrl+S",
    "*fileMenu.includeFile.accelerator: Alt<Key>i",
    "*fileMenu.includeFile.acceleratorText: Alt+I",
    "*fileMenu.print.accelerator: Ctrl<Key>p",
    "*fileMenu.print.acceleratorText: Ctrl+P",
    "*fileMenu.exit.accelerator: Ctrl<Key>q",
    "*fileMenu.exit.acceleratorText: Ctrl+Q",
    "*editMenu.mnemonic: E",
    "*editMenu.undo.accelerator: Ctrl<Key>z",
    "*editMenu.undo.acceleratorText: Ctrl+Z",
    "*editMenu.redo.accelerator: Shift Ctrl<Key>z",
    "*editMenu.redo.acceleratorText: Shift+Ctrl+Z",
    "*editMenu.cut.accelerator: Ctrl<Key>x",
    "*editMenu.cut.acceleratorText: Ctrl+X",
    "*editMenu.copy.accelerator: Ctrl<Key>c",
    "*editMenu.copy.acceleratorText: Ctrl+C",
    "*editMenu.paste.accelerator: Ctrl<Key>v",
    "*editMenu.paste.acceleratorText: Ctrl+V",
    "*editMenu.pasteColumn.accelerator: Shift Ctrl<Key>v",
    "*editMenu.pasteColumn.acceleratorText: Ctrl+Shift+V",
    "*editMenu.delete.acceleratorText: Del",
    "*editMenu.selectAll.accelerator: Ctrl<Key>a",
    "*editMenu.selectAll.acceleratorText: Ctrl+A",
    "*editMenu.shiftLeft.accelerator: Ctrl<Key>9",
    "*editMenu.shiftLeft.acceleratorText: [Shift]Ctrl+9",
    "*editMenu.shiftLeftShift.accelerator: Shift Ctrl<Key>9",
    "*editMenu.shiftRight.accelerator: Ctrl<Key>0",
    "*editMenu.shiftRight.acceleratorText: [Shift]Ctrl+0",
    "*editMenu.shiftRightShift.accelerator: Shift Ctrl<Key>0",
    "*editMenu.upperCase.accelerator: Ctrl<Key>6",
    "*editMenu.upperCase.acceleratorText: Ctrl+6",
    "*editMenu.lowerCase.accelerator: Shift Ctrl<Key>6",
    "*editMenu.lowerCase.acceleratorText: Shift+Ctrl+6",
    "*editMenu.fillParagraph.accelerator: Ctrl<Key>j",
    "*editMenu.fillParagraph.acceleratorText: Ctrl+J",
    "*editMenu.insertFormFeed.accelerator: Alt Ctrl<Key>l",
    "*editMenu.insertFormFeed.acceleratorText: Alt+Ctrl+L",
    "*editMenu.insertCtrlCode.accelerator: Alt Ctrl<Key>i",
    "*editMenu.insertCtrlCode.acceleratorText: Alt+Ctrl+I",
    "*searchMenu.mnemonic: S",
    "*searchMenu.find.accelerator: Ctrl<Key>f",
    "*searchMenu.find.acceleratorText: [Shift]Ctrl+F",
    "*searchMenu.findShift.accelerator: Shift Ctrl<Key>f",
    "*searchMenu.findAgain.accelerator: Ctrl<Key>g",
    "*searchMenu.findAgain.acceleratorText: [Shift]Ctrl+G",
    "*searchMenu.findAgainShift.accelerator: Shift Ctrl<Key>g",
    "*searchMenu.findSelection.accelerator: Ctrl<Key>h",
    "*searchMenu.findSelection.acceleratorText: [Shift]Ctrl+H",
    "*searchMenu.findSelectionShift.accelerator: Shift Ctrl<Key>h",
    "*searchMenu.findIncremental.accelerator: Ctrl<Key>i",
    "*searchMenu.findIncrementalShift.accelerator: Shift Ctrl<Key>i",
    "*searchMenu.findIncremental.acceleratorText: [Shift]Ctrl+I",
    "*searchMenu.replace.accelerator: Ctrl<Key>r",
    "*searchMenu.replace.acceleratorText: [Shift]Ctrl+R",
    "*searchMenu.replaceShift.accelerator: Shift Ctrl<Key>r",
    "*searchMenu.findReplace.accelerator: Ctrl<Key>r",
    "*searchMenu.findReplace.acceleratorText: [Shift]Ctrl+R",
    "*searchMenu.findReplaceShift.accelerator: Shift Ctrl<Key>r",
    "*searchMenu.replaceFindAgain.accelerator: Ctrl<Key>t",
    "*searchMenu.replaceFindAgain.acceleratorText: [Shift]Ctrl+T",
    "*searchMenu.replaceFindAgainShift.accelerator: Shift Ctrl<Key>t",
    "*searchMenu.replaceAgain.accelerator: Alt<Key>t",
    "*searchMenu.replaceAgain.acceleratorText: [Shift]Alt+T",
    "*searchMenu.replaceAgainShift.accelerator: Shift Alt<Key>t",
    "*searchMenu.gotoLineNumber.accelerator: Ctrl<Key>l",
    "*searchMenu.gotoLineNumber.acceleratorText: Ctrl+L",
    "*searchMenu.gotoSelected.accelerator: Ctrl<Key>e",
    "*searchMenu.gotoSelected.acceleratorText: Ctrl+E",
    "*searchMenu.mark.accelerator: Alt<Key>m",
    "*searchMenu.mark.acceleratorText: Alt+M a-z",
    "*searchMenu.gotoMark.accelerator: Alt<Key>g",
    "*searchMenu.gotoMark.acceleratorText: [Shift]Alt+G a-z",
    "*searchMenu.gotoMarkShift.accelerator: Shift Alt<Key>g",
    "*searchMenu.gotoMatching.accelerator: Ctrl<Key>m",
    "*searchMenu.gotoMatching.acceleratorText: [Shift]Ctrl+M",
    "*searchMenu.gotoMatchingShift.accelerator: Shift Ctrl<Key>m",
    "*searchMenu.findDefinition.accelerator: Ctrl<Key>d",
    "*searchMenu.findDefinition.acceleratorText: Ctrl+D",
    "*preferencesMenu.mnemonic: P",
    "*preferencesMenu.statisticsLine.accelerator: Alt<Key>a",
    "*preferencesMenu.statisticsLine.acceleratorText: Alt+A",
    "*preferencesMenu.overtype.acceleratorText: Insert",
    "*shellMenu.mnemonic: l",
    "*shellMenu.filterSelection.accelerator: Alt<Key>r",
    "*shellMenu.filterSelection.acceleratorText: Alt+R",
    "*shellMenu.executeCommand.accelerator: Alt<Key>x",
    "*shellMenu.executeCommand.acceleratorText: Alt+X",
    "*shellMenu.executeCommandLine.accelerator: <Key>KP_Enter",
    "*shellMenu.executeCommandLine.acceleratorText: KP Enter",
    "*shellMenu.cancelShellCommand.accelerator: Ctrl<Key>period",
    "*shellMenu.cancelShellCommand.acceleratorText: Ctrl+.",
    "*macroMenu.mnemonic: c",
    "*macroMenu.learnKeystrokes.accelerator: Alt<Key>k",
    "*macroMenu.learnKeystrokes.acceleratorText: Alt+K",
    "*macroMenu.finishLearn.accelerator: Alt<Key>k",
    "*macroMenu.finishLearn.acceleratorText: Alt+K",
    "*macroMenu.cancelLearn.accelerator: Ctrl<Key>period",
    "*macroMenu.cancelLearn.acceleratorText: Ctrl+.",
    "*macroMenu.replayKeystrokes.accelerator: Ctrl<Key>k",
    "*macroMenu.replayKeystrokes.acceleratorText: Ctrl+K",
    "*macroMenu.repeat.accelerator: Ctrl<Key>comma",
    "*macroMenu.repeat.acceleratorText: Ctrl+,",
    "*windowsMenu.mnemonic: W",
    "*windowsMenu.splitWindow.accelerator: Ctrl<Key>2",
    "*windowsMenu.splitWindow.acceleratorText: Ctrl+2",
    "*windowsMenu.closePane.accelerator: Ctrl<Key>1",
    "*windowsMenu.closePane.acceleratorText: Ctrl+1",
    "*helpMenu.mnemonic: H",
    "nedit.helpForm.sw.helpText*translations: #override\
<Key>Tab:help-focus-buttons()\\n\
<Key>Return:help-button-action(\"dismiss\")\\n\
<KeyPress>osfCancel:help-button-action(\"dismiss\")\\n\
~Meta~Ctrl~Shift<Btn1Down>:\
    grab-focus() help-hyperlink()\\n\
~Meta~Ctrl~Shift<Btn1Up>:\
    help-hyperlink(\"current\", \"process-cancel\", \"extend-end\")\\n\
~Meta~Ctrl~Shift<Btn2Down>:\
    process-bdrag() help-hyperlink()\\n\
~Meta~Ctrl~Shift<Btn2Up>:\
    help-hyperlink(\"new\", \"process-cancel\", \"copy-to\")",
    NULL
};

static const char cmdLineHelp[] =
#ifndef VMS
"Usage:  nedit [-read] [-create] [-line n | +n] [-server] [-do command]\n\
	      [-tags file] [-tabs n] [-wrap] [-nowrap] [-autowrap]\n\
	      [-autoindent] [-noautoindent] [-autosave] [-noautosave]\n\
	      [-lm languagemode] [-rows n] [-columns n] [-font font]\n\
	      [-geometry geometry] [-iconic] [-noiconic] [-svrname name]\n\
	      [-display [host]:server[.screen] [-xrm resourcestring]\n\
	      [-import file] [-background color] [-foreground color]\n\
	      [-V|-version] [--]\n\
	      [file...]\n";
#else
"";
#endif /*VMS*/

int main(int argc, char **argv)
{
    int i, lineNum, nRead, fileSpecified = FALSE, editFlags = CREATE;
    int gotoLine = False, macroFileRead = False, opts = True;
    int iconic = False;
    char *toDoCommand = NULL, *geometry = NULL, *langMode = NULL;
    char filename[MAXPATHLEN], pathname[MAXPATHLEN];
    XtAppContext context;
    XrmDatabase prefDB;
    static const char *protectedKeywords[] = {"-iconic", "-icon", "-geometry",
            "-g", "-rv", "-reverse", "-bd", "-bordercolor", "-borderwidth",
	    "-bw", "-title", NULL};
    
    /* Save the command which was used to invoke nedit for restart command */
    ArgV0 = argv[0];

    /* Set locale for C library, X, and Motif input functions. 
       Reverts to "C" if requested locale not available. */
    XtSetLanguageProc(NULL, NULL, NULL);
 
    /* Initialize X toolkit (does not open display yet) */
    XtToolkitInitialize();
    context = XtCreateApplicationContext();
    
    /* Set up a warning handler to trap obnoxious Xt grab warnings */
    SuppressPassiveGrabWarnings();

    /* Set up default resources if no app-defaults file is found */
    XtAppSetFallbackResources(context, fallbackResources);
    
#if XmVersion >= 1002
    /* Allow users to change tear off menus with X resources */
    XmRepTypeInstallTearOffModelConverter();
#endif
    
#ifdef VMS
    /* Convert the command line to Unix style (This is not an ideal solution) */
    ConvertVMSCommandLine(&argc, &argv);
#endif /*VMS*/
#ifdef __EMX__
    /* expand wildcards if necessary */
    _wildcard(&argc, &argv);
#endif
    
    /* Read the preferences file and command line into a database */
    prefDB = CreateNEditPrefDB(&argc, argv);

    /* Open the display and read X database and remaining command line args.
       XtOpenDisplay must be allowed to process some of the resource arguments
       with its inaccessible internal option table, but others, like -geometry
       and -iconic are per-window and it should not be allowed to consume them,
       so we temporarily masked them out. */
    maskArgvKeywords(argc, argv, protectedKeywords);
    TheDisplay = XtOpenDisplay (context, NULL, APP_NAME, APP_CLASS,
	    NULL, 0, &argc, argv);
    unmaskArgvKeywords(argc, argv, protectedKeywords);
    if (!TheDisplay) {
	XtWarning ("NEdit: Can't open display\n");
	exit(EXIT_FAILURE);
    }
    
    /* Initialize global symbols and subroutines used in the macro language */
    InitMacroGlobals();
    RegisterMacroSubroutines();

    /* Store preferences from the command line and .nedit file, 
       and set the appropriate preferences */
    RestoreNEditPrefs(prefDB, XtDatabase(TheDisplay));
    LoadPrintPreferences(XtDatabase(TheDisplay), APP_NAME, APP_CLASS, True);
    SetDeleteRemap(GetPrefMapDelete());
    SetPointerCenteredDialogs(GetPrefRepositionDialogs());
    SetGetEFTextFieldRemoval(!GetPrefStdOpenDialog());
    
    /* Set up action procedures for menu item commands */
    InstallMenuActions(context);
    
    /* Add Actions for following hyperlinks in the help window */
    InstallHelpLinkActions(context);
    
    /* Install word delimiters for regular expression matching */
    SetREDefaultWordDelimiters(GetPrefDelimiters());
    
    /* Read the nedit dynamic database of files for the Open Previous
       command (and eventually other information as well) */
    ReadNEditDB();
    
    /* Process -import command line argument before others which might
       open windows (loading preferences doesn't update menu settings,
       which would then be out of sync with the real preference settings) */
    for (i=1; i<argc; i++) {
      	if(!strcmp(argv[i], "--")) {
	    break; /* treat all remaining arguments as filenames */
	} else if (!strcmp(argv[i], "-import")) {
    	    nextArg(argc, argv, &i);
    	    ImportPrefFile(argv[i], False);
	} else if (!strcmp(argv[i], "-importold")) {
    	    nextArg(argc, argv, &i);
    	    ImportPrefFile(argv[i], True);
	}
    }
    
    /* Load the default tags file. Don't complain if it doesn't load, the tag
       file resource is intended to be set and forgotten.  Running nedit in a
       directory without a tags should not cause it to spew out errors. */
    if (*GetPrefTagFile() != '\0')
    	AddTagsFile(GetPrefTagFile());

    /* Process any command line arguments (-tags, -do, -read, -create,
       +<line_number>, -line, -server, and files to edit) not already
       processed by RestoreNEditPrefs. */
    fileSpecified = FALSE;
    for (i=1; i<argc; i++) {
        if (opts && !strcmp(argv[i], "--")) { 
    	    opts = False; /* treat all remaining arguments as filenames */
	    continue;
	} else if (opts && !strcmp(argv[i], "-tags")) {
    	    nextArg(argc, argv, &i);
    	    if (!AddTagsFile(argv[i]))
    	    	fprintf(stderr, "NEdit: Unable to load tags file\n");
    	} else if (opts && !strcmp(argv[i], "-do")) {
    	    nextArg(argc, argv, &i);
	    if (checkDoMacroArg(argv[i]))
    	    	toDoCommand = argv[i];
    	} else if (opts && !strcmp(argv[i], "-read")) {
    	    editFlags |= PREF_READ_ONLY;
    	} else if (opts && !strcmp(argv[i], "-create")) {
    	    editFlags |= SUPPRESS_CREATE_WARN;
    	} else if (opts && !strcmp(argv[i], "-line")) {
    	    nextArg(argc, argv, &i);
	    nRead = sscanf(argv[i], "%d", &lineNum);
	    if (nRead != 1)
    		fprintf(stderr, "NEdit: argument to line should be a number\n");
    	    else
    	    	gotoLine = True;
    	} else if (opts && (*argv[i] == '+')) {
    	    nRead = sscanf((argv[i]+1), "%d", &lineNum);
	    if (nRead != 1)
    		fprintf(stderr, "NEdit: argument to + should be a number\n");
    	    else
    	    	gotoLine = True;
    	} else if (opts && !strcmp(argv[i], "-server")) {
    	    IsServer = True;
	} else if (opts && (!strcmp(argv[i], "-iconic") || 
	                    !strcmp(argv[i], "-icon"))) {
    	    iconic = True;
	} else if (opts && !strcmp(argv[i], "-noiconic")) {
    	    iconic = False;
	} else if (opts && (!strcmp(argv[i], "-geometry") || 
	                    !strcmp(argv[i], "-g"))) {
	    nextArg(argc, argv, &i);
    	    geometry = argv[i];
	} else if (opts && !strcmp(argv[i], "-lm")) {
	    nextArg(argc, argv, &i);
    	    langMode = argv[i];
	} else if (opts && !strcmp(argv[i], "-import")) {
	    nextArg(argc, argv, &i); /* already processed, skip */
	} else if (opts && (!strcmp(argv[i], "-V") || 
	                    !strcmp(argv[i], "-version"))) {
	    PrintVersion();
	    exit(EXIT_SUCCESS);
	} else if (opts && (*argv[i] == '-')) {
#ifdef VMS
	    *argv[i] = '/';
#endif /*VMS*/
    	    fprintf(stderr, "NEdit: Unrecognized option %s\n%s", argv[i],
    	    	    cmdLineHelp);
    	    exit(EXIT_FAILURE);
    	} else {
#ifdef VMS
	    int numFiles, j;
	    char **nameList = NULL;
	    /* Use VMS's LIB$FILESCAN for filename in argv[i] to process */
	    /* wildcards and to obtain a full VMS file specification     */
	    numFiles = VMSFileScan(argv[i], &nameList, NULL, INCLUDE_FNF);
	    /* for each expanded file name do: */
	    for (j = 0; j < numFiles; ++j) {
	    	ParseFilename(nameList[j], filename, pathname);
		EditExistingFile(WindowList, filename, pathname, editFlags,
			geometry, iconic, langMode);
		if (!macroFileRead) {
		    ReadMacroInitFile(WindowList);
		    macroFileRead = True;
		}
		if (toDoCommand != NULL)
	    	    DoMacro(WindowList, toDoCommand, "-do macro");
	    	if (gotoLine)
	    	    SelectNumberedLine(WindowList, lineNum);
		fileSpecified = TRUE;
		free(nameList[j]);
	    }
	    if (nameList != NULL)
	    	free(nameList);
#else
	    if (ParseFilename(argv[i], filename, pathname) == 0 ) {
  	        EditExistingFile(WindowList, filename, pathname, editFlags,
		                 geometry, iconic, langMode);
		fileSpecified = TRUE;
	    } else {
    	        fprintf(stderr, "NEdit: file name too long: %s", argv[i]);
	    }
	    if (!macroFileRead) {
		ReadMacroInitFile(WindowList);
		macroFileRead = True;
	    }
	    if (toDoCommand != NULL)
	    	DoMacro(WindowList, toDoCommand, "-do macro");
	    if (gotoLine)
	    	SelectNumberedLine(WindowList, lineNum);
#endif /*VMS*/
	    toDoCommand = NULL;
	}
    }
#ifdef VMS
    VMSFileScanDone();
#endif /*VMS*/
    
    /* If no file to edit was specified, open a window to edit "Untitled" */
    if (!fileSpecified) {
    	EditNewFile(geometry, iconic, langMode, NULL);
	ReadMacroInitFile(WindowList);
	if (toDoCommand != NULL)
	    DoMacro(WindowList, toDoCommand, "-do macro");
    }
    
    /* Begin remembering last command invoked for "Repeat" menu item */
    AddLastCommandActionHook(context);

    /* Set up communication port and write ~/.nedit_server_process file */
    if (IsServer)
    	InitServerCommunication();

    /* Process events. */
    if (IsServer)
    	ServerMainLoop(context);
    else
    	XtAppMainLoop(context);

    /* Not reached but this keeps some picky compilers happy */
    return EXIT_SUCCESS;
}

static void nextArg(int argc, char **argv, int *argIndex)
{
    if (*argIndex + 1 >= argc) {
#ifdef VMS
	    *argv[*argIndex] = '/';
#endif /*VMS*/
    	fprintf(stderr, "NEdit: %s requires an argument\n%s", argv[*argIndex],
    	        cmdLineHelp);
    	exit(EXIT_FAILURE);
    }
    (*argIndex)++;
}

/*
** Return True if -do macro is valid, otherwise write an error on stderr
*/
static int checkDoMacroArg(const char *macro)
{
    Program *prog;
    char *errMsg, *stoppedAt, *tMacro;
    int macroLen;
    
    /* Add a terminating newline (which command line users are likely to omit
       since they are typically invoking a single routine) */
    macroLen = strlen(macro);
    tMacro = XtMalloc(strlen(macro)+2);
    strncpy(tMacro, macro, macroLen);
    tMacro[macroLen] = '\n';
    tMacro[macroLen+1] = '\0';
    
    /* Do a test parse */
    prog = ParseMacro(tMacro, &errMsg, &stoppedAt);
    if (prog == NULL) {
    	ParseError(NULL, tMacro, stoppedAt, "argument to -do", errMsg);
	return False;
    }
    FreeProgram(prog);
    return True;
}

/*
** maskArgvKeywords and unmaskArgvKeywords mangle selected keywords by
** replacing the '-' with a space, for the purpose of hiding them from
** XtOpenDisplay's option processing.  Why this silly scheme?  XtOpenDisplay
** really needs to see command line arguments, particularly -display, but
** there's no way to change the option processing table it uses, to keep
** it from consuming arguments which are meant to apply per-window, like
** -geometry and -iconic.
*/
static void maskArgvKeywords(int argc, char **argv, const char **maskArgs)
{
    int i, k;

    for (i=1; i<argc; i++)
	for (k=0; maskArgs[k]!=NULL; k++)
	    if (!strcmp(argv[i], maskArgs[k]))
    		argv[i][0] = ' ';
}


static void unmaskArgvKeywords(int argc, char **argv, const char **maskArgs)
{
    int i, k;

    for (i=1; i<argc; i++)
	for (k=0; maskArgs[k]!=NULL; k++)
	    if (argv[i][0]==' ' && !strcmp(&argv[i][1], &maskArgs[k][1]))
    		argv[i][0] = '-';
}
