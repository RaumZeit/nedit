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
* FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Lesser General Public License *
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
#include <stdio.h>
#include <stdlib.h>
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
#include <sys/param.h>
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
#include "interpret.h"
#include "parse.h"

static void nextArg(int argc, char **argv, int *argIndex);
static int checkDoMacroArg(char *macro);
void maskArgvKeywords(int argc, char **argv, char **maskArgs);
void unmaskArgvKeywords(int argc, char **argv, char **maskArgs);

WindowInfo *WindowList = NULL;
Display *TheDisplay;
char *ArgV0;
Boolean IsServer = False;

static char *fallbackResources[] = {
    "*menuBar.marginHeight: 0",
    "*menuBar.shadowThickness: 1",
    "*pane.sashHeight: 11",
    "*pane.sashWidth: 11",
    "*text.selectionArrayCount: 3",
    "*fontList:-adobe-helvetica-medium-r-normal-*-12-*-*-*-*-*-*",
    "*XmList.fontList:-adobe-courier-medium-r-normal-*-12-*-*-*-*-*-*",
    "*XmText.fontList:-adobe-courier-medium-r-normal-*-12-*-*-*-*-*-*",
    /* This should not be necessary, but some default in lesstif is
       overriding the resource above, and specifying the app-name fixes it */
    "nedit*XmText.fontList:-adobe-courier-medium-r-normal-*-12-*-*-*-*-*-*",
    /* Same with this, both Solaris Motif and Lesstif seem to have some
       very specific defaults for file selection box fonts */
    "nedit*FileSelect*XmList.fontList:-adobe-courier-medium-r-normal-*-12-*-*-*-*-*-*",
    "*XmTextField.fontList:-adobe-courier-medium-r-normal-*-12-*-*-*-*-*-*",
    "*background: #b3b3b3",
    "*foreground: black",
    "*text.lineNumForeground: #777777",
    "*text.background: #e5e5e5",
    "*text.foreground: black",
    "*text.highlightBackground: red",
    "*text.highlightForeground: black",
    "*XmText*foreground: black",
    "*XmText*background: #cccccc",
    "*helpText.background: #cccccc",
    "*helpText.foreground: black",
    "*helpText.selectBackground: #b3b3b3",
    "*helpText.font: -adobe-courier-medium-r-normal-*-12-*-*-*-*-*-*",
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
    "*XmList*foreground: black",
    "*XmList*background: #cccccc",
    "*XmTextField*background: #cccccc",
    "*XmTextField*foreground: black",
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
    "*searchMenu.replaceAgain.accelerator: Ctrl<Key>t",
    "*searchMenu.replaceAgain.acceleratorText: [Shift]Ctrl+T",
    "*searchMenu.replaceAgainShift.accelerator: Shift Ctrl<Key>t",
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
    "*preferencesMenu.overtype.accelerator: Ctrl<Key>b",
    "*preferencesMenu.overtype.acceleratorText: Ctrl+B",
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
    0
};

static char cmdLineHelp[] =
#ifndef VMS
"Usage:  nedit [-read] [-create] [-line n | +n] [-server] [-do command]\n\
	      [-tags file] [-tabs n] [-wrap] [-nowrap] [-autowrap]\n\
	      [-autoindent] [-noautoindent] [-autosave] [-noautosave]\n\
	      [-lm languagemode] [-rows n] [-columns n] [-font font]\n\
	      [-geometry geometry] [-iconic] [-noiconic] [-svrname name]\n\
	      [-display [host]:server[.screen] [-xrm resourcestring]\n\
	      [-import file] [-background color] [-foreground color]\n\
	      [file...]\n";
#else
"";
#endif /*VMS*/

int main(int argc, char **argv)
{
    int i, lineNum, nRead, fileSpecified = FALSE, editFlags = CREATE;
    int gotoLine = False, macroFileRead = False;
    int iconic = False;
    char *toDoCommand = NULL, *geometry = NULL, *langMode = NULL;
    char filename[MAXPATHLEN], pathname[MAXPATHLEN];
    XtAppContext context;
    XrmDatabase prefDB;
    static char *protectedKeywords[] = {"-iconic", "-icon", "-geometry", "-g",
	    "-rv", "-reverse", "-bd", "-bordercolor", "-borderwidth", "-bw",
	    "-title", NULL};
    
    /* Save the command which was used to invoke nedit for restart command */
    ArgV0 = argv[0];
    
#ifndef NO_XMIM
    /* Set local for C library and X, and Motif input functions */
    if (setlocale(LC_CTYPE, "") == NULL) {
	fprintf(stderr, "NEdit: Locale not supported by C library.\n");
	if(setlocale(LC_CTYPE, NULL)==NULL) {
            fprintf(stderr,"NEdit: cannot continue.\n");
            exit(0);
	} else
            fprintf(stderr,"NEdit: Using %s locale instead.\n",
            	    setlocale(LC_CTYPE, NULL));
    }
    if (!XSupportsLocale()) {
    	fprintf(stderr, "NEdit: Xlib: locale %s not supported\n.",
    	    	setlocale(LC_CTYPE, NULL));
    	exit(0);
    }
    if (XSetLocaleModifiers("") == NULL)
        fprintf(stderr,"NEdit: cannot set locale modifiers.\n");
#else
    /* Set locale for C library functions */
    setlocale(LC_CTYPE, "");
#endif
    
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
	exit(0);
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
    
    /* Install word delimiters for regular expression matching */
    SetREDefaultWordDelimiters(GetPrefDelimiters());
    
    /* Read the nedit dynamic database of files for the Open Previous
       command (and eventually other information as well) */
    ReadNEditDB();
    
    /* Process -import command line argument before others which might
       open windows (loading preferences doesn't update menu settings,
       which would then be out of sync with the real preference settings) */
    for (i=1; i<argc; i++) {
    	if (!strcmp(argv[i], "-import")) {
    	    nextArg(argc, argv, &i);
    	    ImportPrefFile(argv[i], False);
	}else if (!strcmp(argv[i], "-importold")) {
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
    	if (!strcmp(argv[i], "-tags")) {
    	    nextArg(argc, argv, &i);
    	    if (!AddTagsFile(argv[i]))
    	    	fprintf(stderr, "NEdit: Unable to load tags file\n");
    	} else if (!strcmp(argv[i], "-do")) {
    	    nextArg(argc, argv, &i);
	    if (checkDoMacroArg(argv[i]))
    	    	toDoCommand = argv[i];
    	} else if (!strcmp(argv[i], "-read")) {
    	    editFlags |= FORCE_READ_ONLY;
    	} else if (!strcmp(argv[i], "-create")) {
    	    editFlags |= SUPPRESS_CREATE_WARN;
    	} else if (!strcmp(argv[i], "-line")) {
    	    nextArg(argc, argv, &i);
	    nRead = sscanf(argv[i], "%d", &lineNum);
	    if (nRead != 1)
    		fprintf(stderr, "NEdit: argument to line should be a number\n");
    	    else
    	    	gotoLine = True;
    	} else if (*argv[i] == '+') {
    	    nRead = sscanf((argv[i]+1), "%d", &lineNum);
	    if (nRead != 1)
    		fprintf(stderr, "NEdit: argument to + should be a number\n");
    	    else
    	    	gotoLine = True;
    	} else if (!strcmp(argv[i], "-server")) {
    	    IsServer = True;
	} else if (!strcmp(argv[i], "-iconic") || !strcmp(argv[i], "-icon")) {
    	    iconic = True;
	} else if (!strcmp(argv[i], "-noiconic")) {
    	    iconic = False;
	} else if (!strcmp(argv[i], "-geometry") || !strcmp(argv[i], "-g")) {
	    nextArg(argc, argv, &i);
    	    geometry = argv[i];
	} else if (!strcmp(argv[i], "-lm")) {
	    nextArg(argc, argv, &i);
    	    langMode = argv[i];
	} else if (!strcmp(argv[i], "-import")) {
	    nextArg(argc, argv, &i); /* already processed, skip */
    	} else if (*argv[i] == '-') {
#ifdef VMS
	    *argv[i] = '/';
#endif /*VMS*/
    	    fprintf(stderr, "NEdit: Unrecognized option %s\n%s", argv[i],
    	    	    cmdLineHelp);
    	    exit(0);
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
	    ParseFilename(argv[i], filename, pathname);
	    EditExistingFile(WindowList, filename, pathname, editFlags,
		    geometry, iconic, langMode);
	    fileSpecified = TRUE;
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
    	EditNewFile(geometry, iconic, langMode);
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
    return 0;
}

static void nextArg(int argc, char **argv, int *argIndex)
{
    if (*argIndex + 1 >= argc) {
#ifdef VMS
	    *argv[*argIndex] = '/';
#endif /*VMS*/
    	fprintf(stderr, "NEdit: %s requires an argument\n%s", argv[*argIndex],
    	        cmdLineHelp);
    	exit(0);
    }
    (*argIndex)++;
}

/*
** Return True if -do macro is valid, otherwise write an error on stderr
*/
static int checkDoMacroArg(char *macro)
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
void maskArgvKeywords(int argc, char **argv, char **maskArgs)
{
    int i, k;
    for (i=1; i<argc; i++)
	for (k=0; maskArgs[k]!=NULL; k++)
	    if (!strcmp(argv[i], maskArgs[k]))
    		argv[i][0] = ' ';
}
void unmaskArgvKeywords(int argc, char **argv, char **maskArgs)
{
    int i, k;
    for (i=1; i<argc; i++)
	for (k=0; maskArgs[k]!=NULL; k++)
	    if (argv[i][0]==' ' && !strcmp(&argv[i][1], &maskArgs[k][1]))
    		argv[i][0] = '-';
}
