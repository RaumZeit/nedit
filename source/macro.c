static const char CVSID[] = "$Id: macro.c,v 1.15 2001/03/12 15:15:14 slobasso Exp $";
/*******************************************************************************
*									       *
* macro.c -- Macro file processing, learn/replay, and built-in macro	       *
*            subroutines			    	    	    	       *
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
* April, 1997								       *
*									       *
* Written by Mark Edel							       *
*									       *
*******************************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <errno.h>
#ifdef VMS
#include "../util/VMSparam.h"
#include <types.h>
#include <stat.h>
#include <unixio.h>
#else
#include <sys/types.h>
#include <sys/stat.h>
#ifndef __MVS__
#include <sys/param.h>
#endif
#include <fcntl.h>
#endif /*VMS*/
#include <X11/Intrinsic.h>
#include <Xm/Xm.h>
#include <Xm/CutPaste.h>
#include <Xm/Form.h>
#include <Xm/RowColumn.h>
#include <Xm/LabelG.h>
#include <Xm/List.h>
#include <Xm/ToggleB.h>
#include <Xm/DialogS.h>
#include <Xm/MessageB.h>
#include <Xm/SelectioB.h>
#include <Xm/PushB.h>
#include <Xm/Text.h>
#include <Xm/Separator.h>
#include "../util/DialogF.h"
#include "../util/misc.h"
#include "textBuf.h"
#include "text.h"
#include "nedit.h"
#include "window.h"
#include "macro.h"
#include "preferences.h"
#include "rbTree.h"
#include "interpret.h"
#include "parse.h"
#include "search.h"
#include "shell.h"
#include "userCmds.h"
#include "selection.h"
#include "../util/utils.h"

#define AUTO_LOAD_MACRO_FILE_NAME ".neditmacro"
	
/* Maximum number of actions in a macro and args in 
   an action (to simplify the reader) */
#define MAX_MACRO_ACTIONS 1024
#define MAX_ACTION_ARGS 40

/* How long to wait (msec) before putting up Macro Command banner */
#define BANNER_WAIT_TIME 6000

/* Data attached to window during shell command execution with
   information for controling and communicating with the process */
typedef struct {
    XtIntervalId bannerTimeoutID;
    XtWorkProcId continueWorkProcID;
    char bannerIsUp;
    char closeOnCompletion;
    Program *program;
    RestartData *context;
    Widget dialog;
} macroCmdInfo;

/* Widgets and global data for Repeat dialog */
typedef struct {
    WindowInfo *forWindow;
    char *lastCommand;
    Widget shell, repeatText, lastCmdToggle;
    Widget inSelToggle, toEndToggle;
} repeatDialog;

static void cancelLearn(void);
static void runMacro(WindowInfo *window, Program *prog);
static void finishMacroCmdExecution(WindowInfo *window);
static void repeatOKCB(Widget w, XtPointer clientData, XtPointer callData);
static void repeatApplyCB(Widget w, XtPointer clientData, XtPointer callData);
static int doRepeatDialogAction(repeatDialog *rd, XEvent *event);
static void repeatCancelCB(Widget w, XtPointer clientData, XtPointer callData);
static void repeatDestroyCB(Widget w, XtPointer clientData, XtPointer callData);
static void learnActionHook(Widget w, XtPointer clientData, String actionName,
	XEvent *event, String *params, Cardinal *numParams);
static void lastActionHook(Widget w, XtPointer clientData, String actionName,
	XEvent *event, String *params, Cardinal *numParams);
char *actionToString(char *actionName, XEvent *event, String *params,
	Cardinal numParams);
static int isMouseAction(char *action);
static int isRedundantAction(char *action);
static int isIgnoredAction(char *action);
static int readCheckMacroString(Widget dialogParent, char *string,
	WindowInfo *runWindow, char *errIn, char **errPos);
static void bannerTimeoutProc(XtPointer clientData, XtIntervalId *id);
static Boolean continueWorkProc(XtPointer clientData);
static int escapeStringChars(char *fromString, char *toString);
static int escapedStringLength(char *string);
static int lengthMS(WindowInfo *window, DataValue *argList, int nArgs,
    	DataValue *result, char **errMsg);
static int minMS(WindowInfo *window, DataValue *argList, int nArgs,
    	DataValue *result, char **errMsg);
static int maxMS(WindowInfo *window, DataValue *argList, int nArgs,
    	DataValue *result, char **errMsg);
static int focusWindowMS(WindowInfo *window, DataValue *argList, int nArgs,
      DataValue *result, char **errMsg);
static int getRangeMS(WindowInfo *window, DataValue *argList, int nArgs,
    	DataValue *result, char **errMsg);
static int getCharacterMS(WindowInfo *window, DataValue *argList, int nArgs,
    	DataValue *result, char **errMsg);
static int replaceRangeMS(WindowInfo *window, DataValue *argList, int nArgs,
    	DataValue *result, char **errMsg);
static int replaceSelectionMS(WindowInfo *window, DataValue *argList, int nArgs,
    	DataValue *result, char **errMsg);
static int getSelectionMS(WindowInfo *window, DataValue *argList, int nArgs,
    	DataValue *result, char **errMsg);
static int replaceInStringMS(WindowInfo *window, DataValue *argList, int nArgs,
    	DataValue *result, char **errMsg);
static int replaceSubstringMS(WindowInfo *window, DataValue *argList, int nArgs,
    	DataValue *result, char **errMsg);
static int readFileMS(WindowInfo *window, DataValue *argList, int nArgs,
    	DataValue *result, char **errMsg);
static int writeFileMS(WindowInfo *window, DataValue *argList, int nArgs,
    	DataValue *result, char **errMsg);
static int appendFileMS(WindowInfo *window, DataValue *argList, int nArgs,
    	DataValue *result, char **errMsg);
static int writeOrAppendFile(int append, WindowInfo *window,
    	DataValue *argList, int nArgs, DataValue *result, char **errMsg);
static int substringMS(WindowInfo *window, DataValue *argList, int nArgs,
    	DataValue *result, char **errMsg);
static int toupperMS(WindowInfo *window, DataValue *argList, int nArgs,
    	DataValue *result, char **errMsg);
static int tolowerMS(WindowInfo *window, DataValue *argList, int nArgs,
    	DataValue *result, char **errMsg);
static int stringToClipboardMS(WindowInfo *window, DataValue *argList, int nArgs,
    	DataValue *result, char **errMsg);
static int clipboardToStringMS(WindowInfo *window, DataValue *argList, int nArgs,
    	DataValue *result, char **errMsg);
static int searchMS(WindowInfo *window, DataValue *argList, int nArgs,
    	DataValue *result, char **errMsg);
static int searchStringMS(WindowInfo *window, DataValue *argList, int nArgs,
    	DataValue *result, char **errMsg);
static int setCursorPosMS(WindowInfo *window, DataValue *argList, int nArgs,
    	DataValue *result, char **errMsg);
static int beepMS(WindowInfo *window, DataValue *argList, int nArgs,
    	DataValue *result, char **errMsg);
static int selectMS(WindowInfo *window, DataValue *argList, int nArgs,
    	DataValue *result, char **errMsg);
static int selectRectangleMS(WindowInfo *window, DataValue *argList, int nArgs,
    	DataValue *result, char **errMsg);
static int tPrintMS(WindowInfo *window, DataValue *argList, int nArgs,
    	DataValue *result, char **errMsg);
static int getenvMS(WindowInfo *window, DataValue *argList, int nArgs,
    	DataValue *result, char **errMsg);
static int shellCmdMS(WindowInfo *window, DataValue *argList, int nArgs,
    	DataValue *result, char **errMsg);
static int dialogMS(WindowInfo *window, DataValue *argList, int nArgs,
    	DataValue *result, char **errMsg);
static void dialogBtnCB(Widget w, XtPointer clientData, XtPointer callData);
static void dialogCloseCB(Widget w, XtPointer clientData, XtPointer callData);
static int stringDialogMS(WindowInfo *window, DataValue *argList, int nArgs,
    	DataValue *result, char **errMsg);
static void stringDialogBtnCB(Widget w, XtPointer clientData,
    	XtPointer callData);
static void stringDialogCloseCB(Widget w, XtPointer clientData,
    	XtPointer callData);
/* T Balinski */
static int listDialogMS(WindowInfo *window, DataValue *argList, int nArgs,
	DataValue *result, char **errMsg);
static void listDialogBtnCB(Widget w, XtPointer clientData,
	XtPointer callData);
static void listDialogCloseCB(Widget w, XtPointer clientData,
	XtPointer callData);
/* T Balinski End */
static int stringCompareMS(WindowInfo *window, DataValue *argList, int nArgs,
    	DataValue *result, char **errMsg);
static int splitMS(WindowInfo *window, DataValue *argList, int nArgs,
    	DataValue *result, char **errMsg);
static int cursorMV(WindowInfo *window, DataValue *argList, int nArgs,
    	DataValue *result, char **errMsg);
static int lineMV(WindowInfo *window, DataValue *argList, int nArgs,
        DataValue *result, char **errMsg);
static int columnMV(WindowInfo *window, DataValue *argList, int nArgs,
        DataValue *result, char **errMsg);
static int fileNameMV(WindowInfo *window, DataValue *argList, int nArgs,
    	DataValue *result, char **errMsg);
static int filePathMV(WindowInfo *window, DataValue *argList, int nArgs,
    	DataValue *result, char **errMsg);
static int lengthMV(WindowInfo *window, DataValue *argList, int nArgs,
    	DataValue *result, char **errMsg);
static int selectionStartMV(WindowInfo *window, DataValue *argList, int nArgs,
    	DataValue *result, char **errMsg);
static int selectionEndMV(WindowInfo *window, DataValue *argList, int nArgs,
    	DataValue *result, char **errMsg);
static int selectionLeftMV(WindowInfo *window, DataValue *argList, int nArgs,
    	DataValue *result, char **errMsg);
static int selectionRightMV(WindowInfo *window, DataValue *argList, int nArgs,
    	DataValue *result, char **errMsg);
static int statisticsLineMV(WindowInfo *window, DataValue *argList, int nArgs,
    DataValue *result, char **errMsg);
static int incSearchLineMV(WindowInfo *window, DataValue *argList, int nArgs,
    DataValue *result, char **errMsg);
static int showLineNumbersMV(WindowInfo *window, DataValue *argList, int nArgs,
    DataValue *result, char **errMsg);
static int autoIndentMV(WindowInfo *window, DataValue *argList, int nArgs,
    DataValue *result, char **errMsg);
static int wrapTextMV(WindowInfo *window, DataValue *argList, int nArgs,
    DataValue *result, char **errMsg);
static int highlightSyntaxMV(WindowInfo *window, DataValue *argList, int nArgs,
    DataValue *result, char **errMsg);
static int makeBackupCopyMV(WindowInfo *window, DataValue *argList, int nArgs,
    DataValue *result, char **errMsg);
static int incBackupMV(WindowInfo *window, DataValue *argList, int nArgs,
    DataValue *result, char **errMsg);
static int showMatchingMV(WindowInfo *window, DataValue *argList, int nArgs,
    DataValue *result, char **errMsg);
static int overTypeModeMV(WindowInfo *window, DataValue *argList, int nArgs,
    DataValue *result, char **errMsg);
static int readOnlyMV(WindowInfo *window, DataValue *argList, int nArgs,
    DataValue *result, char **errMsg);
static int lockedMV(WindowInfo *window, DataValue *argList, int nArgs,
    DataValue *result, char **errMsg);
static int fileFormatMV(WindowInfo *window, DataValue *argList, int nArgs,
    DataValue *result, char **errMsg);
static int fontNameMV(WindowInfo *window, DataValue *argList, int nArgs,
    DataValue *result, char **errMsg);
static int fontNameItalicMV(WindowInfo *window, DataValue *argList, int nArgs,
    DataValue *result, char **errMsg);
static int fontNameBoldMV(WindowInfo *window, DataValue *argList, int nArgs,
    DataValue *result, char **errMsg);
static int fontNameBoldItalicMV(WindowInfo *window, DataValue *argList, int nArgs,
    DataValue *result, char **errMsg);
static int subscriptSepMV(WindowInfo *window, DataValue *argList, int nArgs,
    DataValue *result, char **errMsg);
static int minFontWidthMV(WindowInfo *window, DataValue *argList, int nArgs,
    DataValue *result, char **errMsg);
static int maxFontWidthMV(WindowInfo *window, DataValue *argList, int nArgs,
    DataValue *result, char **errMsg);
static int wrapMarginMV(WindowInfo *window, DataValue *argList, int nArgs,
    	DataValue *result, char **errMsg);
static int topLineMV(WindowInfo *window, DataValue *argList, int nArgs,
    DataValue *result, char **errMsg);
static int numDisplayLinesMV(WindowInfo *window, DataValue *argList, int nArgs,
    DataValue *result, char **errMsg);
static int displayWidthMV(WindowInfo *window, DataValue *argList, int nArgs,
    DataValue *result, char **errMsg);
static int activePaneMV(WindowInfo *window, DataValue *argList, int nArgs,
    DataValue *result, char **errMsg);
static int nPanesMV(WindowInfo *window, DataValue *argList, int nArgs,
    DataValue *result, char **errMsg);
static int tabDistMV(WindowInfo *window, DataValue *argList, int nArgs,
    	DataValue *result, char **errMsg);
static int emTabDistMV(WindowInfo *window, DataValue *argList, int nArgs,
    	DataValue *result, char **errMsg);
static int useTabsMV(WindowInfo *window, DataValue *argList, int nArgs,
    	DataValue *result, char **errMsg);
static int modifiedMV(WindowInfo *window, DataValue *argList, int nArgs,
    	DataValue *result, char **errMsg);
static int languageModeMV(WindowInfo *window, DataValue *argList, int nArgs,
    	DataValue *result, char **errMsg);
static int readSearchArgs(DataValue *argList, int nArgs, int*searchDirection,
	int *searchType, int *wrap, char **errMsg);
static int wrongNArgsErr(char **errMsg);
static int tooFewArgsErr(char **errMsg);
static int strCaseCmp(char *str1, char *str2);
static int readIntArg(DataValue dv, int *result, char **errMsg);
static int readStringArg(DataValue dv, char **result, char *stringStorage,
    	char **errMsg);

/* Built-in subroutines and variables for the macro language */
#define N_MACRO_SUBRS 33
static BuiltInSubr MacroSubrs[N_MACRO_SUBRS] = {lengthMS, getRangeMS, tPrintMS,
    	dialogMS, stringDialogMS, replaceRangeMS, replaceSelectionMS,
    	setCursorPosMS, getCharacterMS, minMS, maxMS, searchMS,
    	searchStringMS, substringMS, replaceSubstringMS, readFileMS,
    	writeFileMS, appendFileMS, beepMS, getSelectionMS,
	replaceInStringMS, selectMS, selectRectangleMS, focusWindowMS,
	shellCmdMS, stringToClipboardMS, clipboardToStringMS, toupperMS,
	tolowerMS, listDialogMS, getenvMS,
    stringCompareMS, splitMS};
static char *MacroSubrNames[N_MACRO_SUBRS] = {"length", "get_range", "t_print",
    	"dialog", "string_dialog", "replace_range", "replace_selection",
    	"set_cursor_pos", "get_character", "min", "max", "search",
        "search_string", "substring", "replace_substring", "read_file",
        "write_file", "append_file", "beep", "get_selection",
	"replace_in_string", "select", "select_rectangle", "focus_window",
	"shell_command", "string_to_clipboard", "clipboard_to_string",
	"toupper", "tolower", "list_dialog", "getenv",
    "string_compare", "split"};
#define N_SPECIAL_VARS 41
static BuiltInSubr SpecialVars[N_SPECIAL_VARS] = {cursorMV, lineMV, columnMV,
	fileNameMV, filePathMV, lengthMV, selectionStartMV, selectionEndMV,
    	selectionLeftMV, selectionRightMV, wrapMarginMV, tabDistMV,
    	emTabDistMV, useTabsMV, languageModeMV, modifiedMV,
        statisticsLineMV, incSearchLineMV, showLineNumbersMV,
        autoIndentMV, wrapTextMV, highlightSyntaxMV,
        makeBackupCopyMV, incBackupMV, showMatchingMV,
        overTypeModeMV, readOnlyMV, lockedMV, fileFormatMV,
        fontNameMV, fontNameItalicMV,
        fontNameBoldMV, fontNameBoldItalicMV, subscriptSepMV,
        minFontWidthMV, maxFontWidthMV, topLineMV, numDisplayLinesMV,
        displayWidthMV, activePaneMV, nPanesMV};
static char *SpecialVarNames[N_SPECIAL_VARS] = {"$cursor", "$line", "$column",
	"$file_name", "$file_path", "$text_length", "$selection_start",
	"$selection_end", "$selection_left", "$selection_right",
	"$wrap_margin", "$tab_dist", "$em_tab_dist", "$use_tabs",
	"$language_mode", "$modified",
    "$statistics_line", "$incremental_search_line", "$show_line_numbers",
    "$auto_indent", "$wrap_text", "$highlight_syntax",
    "$make_backup_copy", "$incremental_backup", "$show_matching",
    "$overtype_mode", "$read_only", "$locked", "$file_format",
    "$font_name", "$font_name_italic",
    "$font_name_bold", "$font_name_bold_italic", "$sub_sep",
    "$min_font_width", "$max_font_width", "$top_line", "$n_display_lines",
    "$display_width", "$active_pane", "$n_panes"};

/* Global symbols for returning values from built-in functions */
#define N_RETURN_GLOBALS 5
enum retGlobalSyms {STRING_DIALOG_BUTTON, SEARCH_END, READ_STATUS,
	SHELL_CMD_STATUS, LIST_DIALOG_BUTTON};
static char *ReturnGlobalNames[N_RETURN_GLOBALS] = {"$string_dialog_button",
    	"$search_end", "$read_status", "$shell_cmd_status",
	"$list_dialog_button"};
static Symbol *ReturnGlobals[N_RETURN_GLOBALS];

/* List of actions not useful when learning a macro sequence (also see below) */
static char* IgnoredActions[] = {"focusIn", "focusOut"};

/* List of actions intended to be attached to mouse buttons, which the user
   must be warned can't be recorded in a learn/replay sequence */
static char* MouseActions[] = {"grab_focus", "extend_adjust", "extend_start",
	"extend_end", "secondary_or_drag_adjust", "secondary_adjust",
	"secondary_or_drag_start", "secondary_start", "move_destination",
	"move_to", "move_to_or_end_drag", "copy_to", "copy_to_or_end_drag",
	"exchange", "process_bdrag", "mouse_pan"};

/* List of actions to not record because they 
   generate further actions, more suitable for recording */
static char* RedundantActions[] = {"open_dialog", "save_as_dialog",
	"include_file_dialog", "load_tags_file_dialog", "find_dialog",
	"replace_dialog", "goto_line_number_dialog", "control_code_dialog",
	"filter_selection_dialog", "execute_command_dialog", "repeat_dialog",
	"revert_to_saved_dialog", "start_incremental_find"};

/* The last command executed (used by the Repeat command) */
static char *LastCommand = NULL;

/* The current macro to execute on Replay command */
static char *ReplayMacro = NULL;

/* Buffer where macro commands are recorded in Learn mode */
static textBuffer *MacroRecordBuf = NULL;

/* Action Hook id for recording actions for Learn mode */
static XtActionHookId MacroRecordActionHook = 0;

/* Window where macro recording is taking place */
static WindowInfo *MacroRecordWindow = NULL;

/* Arrays for translating escape characters in escapeStringChars */
static char ReplaceChars[] = "\\\"ntbrfav";
static char EscapeChars[] = "\\\"\n\t\b\r\f\a\v";

/*
** Install built-in macro subroutines and special variables for accessing
** editor information
*/
void RegisterMacroSubroutines(void)
{
    static DataValue subrPtr = {NO_TAG, {0}}, noValue = {NO_TAG, {0}};
    int i;
    
    /* Install symbols for built-in routines and variables, with pointers
       to the appropriate c routines to do the work */
    for (i=0; i<N_MACRO_SUBRS; i++) {
    	subrPtr.val.ptr = (void *)MacroSubrs[i];
    	InstallSymbol(MacroSubrNames[i], C_FUNCTION_SYM, subrPtr);
    }
    for (i=0; i<N_SPECIAL_VARS; i++) {
    	subrPtr.val.ptr = (void *)SpecialVars[i];
    	InstallSymbol(SpecialVarNames[i], PROC_VALUE_SYM, subrPtr);
    }
    
    /* Define global variables used for return values, remember their
       locations so they can be set without a LookupSymbol call */
    for (i=0; i<N_RETURN_GLOBALS; i++)
    	ReturnGlobals[i] = InstallSymbol(ReturnGlobalNames[i], GLOBAL_SYM,
    	    	noValue);
}

void BeginLearn(WindowInfo *window)
{
    WindowInfo *win;
    XmString s;
    
    /* If we're already in learn mode, return */
    if (MacroRecordActionHook != 0)
    	return;
    
    /* dim the inappropriate menus and items, and undim finish and cancel */
    for (win=WindowList; win!=NULL; win=win->next)
	XtSetSensitive(win->learnItem, False);
    XtSetSensitive(window->finishLearnItem, True);
    XtVaSetValues(window->cancelMacroItem, XmNlabelString,
    	    s=XmStringCreateSimple("Cancel Learn"), NULL);
    XmStringFree(s);
    XtSetSensitive(window->cancelMacroItem, True);
    
    /* Mark the window where learn mode is happening */
    MacroRecordWindow = window;
    
    /* Allocate a text buffer for accumulating the macro strings */
    MacroRecordBuf = BufCreate();
    
    /* Add the action hook for recording the actions */
    MacroRecordActionHook =
    	    XtAppAddActionHook(XtWidgetToApplicationContext(window->shell),
    	    learnActionHook, window);
    
    /* Put up the learn-mode banner */
    SetModeMessage(window,
    	    "Learn Mode -- Press Alt+K to finish, Ctrl+. to cancel");
}

void AddLastCommandActionHook(XtAppContext context)
{
    XtAppAddActionHook(context, lastActionHook, NULL);
}

void FinishLearn(void)
{
    WindowInfo *win;
    
    /* If we're not in learn mode, return */
    if (MacroRecordActionHook == 0)
    	return;
    
    /* Remove the action hook */
    XtRemoveActionHook(MacroRecordActionHook);
    MacroRecordActionHook = 0;
    
    /* Free the old learn/replay sequence */
    if (ReplayMacro != NULL)
    	XtFree(ReplayMacro);
    
    /* Store the finished action for the replay menu item */
    ReplayMacro = BufGetAll(MacroRecordBuf);
    
    /* Free the buffer used to accumulate the macro sequence */
    BufFree(MacroRecordBuf);
    
    /* Undim the menu items dimmed during learn */
    for (win=WindowList; win!=NULL; win=win->next)
	XtSetSensitive(win->learnItem, True);
    XtSetSensitive(MacroRecordWindow->finishLearnItem, False);
    XtSetSensitive(MacroRecordWindow->cancelMacroItem, False);
    
    /* Undim the replay and paste-macro buttons */
    for (win=WindowList; win!=NULL; win=win->next)
    	XtSetSensitive(win->replayItem, True);
    DimPasteReplayBtns(True);
    
    /* Clear learn-mode banner */
    ClearModeMessage(MacroRecordWindow);
}

/*
** Cancel Learn mode, or macro execution (they're bound to the same menu item)
*/
void CancelMacroOrLearn(WindowInfo *window)
{
    if (MacroRecordActionHook != 0)
    	cancelLearn();
    else if (window->macroCmdData != NULL)
    	AbortMacroCommand(window);
}

static void cancelLearn(void)
{
    WindowInfo *win;
    
    /* If we're not in learn mode, return */
    if (MacroRecordActionHook == 0)
    	return;

    /* Remove the action hook */
    XtRemoveActionHook(MacroRecordActionHook);
    MacroRecordActionHook = 0;
    
    /* Free the macro under construction */
    BufFree(MacroRecordBuf);
    
    /* Undim the menu items dimmed during learn */
    for (win=WindowList; win!=NULL; win=win->next)
	XtSetSensitive(win->learnItem, True);
    XtSetSensitive(MacroRecordWindow->finishLearnItem, False);
    XtSetSensitive(MacroRecordWindow->cancelMacroItem, False);
    
    /* Clear learn-mode banner */
    ClearModeMessage(MacroRecordWindow);
}

/*
** Execute the learn/replay sequence stored in "window"
*/
void Replay(WindowInfo *window)
{
    Program *prog;
    char *errMsg, *stoppedAt;
    
    if (ReplayMacro == NULL)
    	return;
    
    /* Parse the replay macro (it's stored in text form) and compile it into
       an executable program "prog" */
    prog = ParseMacro(ReplayMacro, &errMsg, &stoppedAt);
    if (prog == NULL) {
    	fprintf(stderr,
    		"NEdit internal error, learn/replay macro syntax error: %s\n",
    		errMsg);
    	return;
    }

    /* run the executable program */
    runMacro(window, prog);
}

/*
** Read the .neditmacro file if one exists
*/
void ReadMacroInitFile(WindowInfo *window)
{
    char fullName[MAXPATHLEN];
    
#ifdef VMS
    sprintf(fullName, "%s%s", "SYS$LOGIN:", AUTO_LOAD_MACRO_FILE_NAME);
#else
    sprintf(fullName, "%s/%s", GetHomeDir(), AUTO_LOAD_MACRO_FILE_NAME);
#endif /*VMS*/
    ReadMacroFile(window, fullName, False);
}

/*
** Read an NEdit macro file.  Extends the syntax of the macro parser with
** define keyword, and allows intermixing of defines with immediate actions.
*/
int ReadMacroFile(WindowInfo *window, char *fileName, int warnNotExist)
{
    struct stat statbuf;
    FILE *fp;
    int fileLen, readLen, result;
    char *fileString;
    
    /* Read the whole file into fileString */
    if ((fp = fopen(fileName, "r")) == NULL) {
    	if (warnNotExist)
	    DialogF(DF_ERR, window->shell, 1, "Can't open macro file %s",
    	    	    "dismiss", fileName);
    	return False;
    }
    if (fstat(fileno(fp), &statbuf) != 0) {
	DialogF(DF_ERR, window->shell, 1, "Can't read macro file %s",
    	    	"dismiss", fileName);
	fclose(fp);
	return False;
    }
    fileLen = statbuf.st_size;
    fileString = XtMalloc(fileLen+1);  /* +1 = space for null */
    readLen = fread(fileString, sizeof(char), fileLen, fp);
    if (ferror(fp)) {
    	DialogF(DF_ERR, window->shell, 1, "Error reading macro file %s: %s",
    	    	"dismiss", fileName,
#ifdef VMS
    	    	strerror(errno, vaxc$errno));
#else
    	    	strerror(errno));
#endif
	XtFree(fileString);
	fclose(fp);
	return False;
    }
    fclose(fp);
    fileString[readLen] = 0;

    /* Parse fileString */
    result = readCheckMacroString(window->shell, fileString, window, fileName,
	    NULL);
    XtFree(fileString);
    return result;
}

/*
** Parse and execute a macro string including macro definitions.  Report
** parsing errors in a dialog posted over window->shell.
*/
int ReadMacroString(WindowInfo *window, char *string, char *errIn)
{   
    return readCheckMacroString(window->shell, string, window, errIn, NULL);
}  

/*
** Check a macro string containing definitions for errors.  Returns True
** if macro compiled successfully.  Returns False and puts up
** a dialog explaining if macro did not compile successfully.
*/  
int CheckMacroString(Widget dialogParent, char *string, char *errIn,
	char **errPos)
{
    return readCheckMacroString(dialogParent, string, NULL, errIn, errPos);
}    

/*
** Parse and optionally execute a macro string including macro definitions.
** Report parsing errors in a dialog posted over dialogParent, using the
** string errIn to identify the entity being parsed (filename, macro string,
** etc.).  If runWindow is specified, runs the macro against the window.  If
** runWindow is passed as NULL, does parse only.  If errPos is non-null, 
** returns a pointer to the error location in the string.
*/
static int readCheckMacroString(Widget dialogParent, char *string,
	WindowInfo *runWindow, char *errIn, char **errPos)
{
    char *stoppedAt, *inPtr, *namePtr, *errMsg;
    char subrName[MAX_SYM_LEN];
    Program *prog;
    Symbol *sym;
    DataValue subrPtr;
    
    inPtr = string;
    while (*inPtr != '\0') {
    	
    	/* skip over white space and comments */
	while (*inPtr==' ' || *inPtr=='\t' || *inPtr=='\n'|| *inPtr=='#') {
	    if (*inPtr == '#')
	    	while (*inPtr != '\n' && *inPtr != '\0') inPtr++;
	    else
	    	inPtr++;
	}
	if (*inPtr == '\0')
	    break;
	
	/* look for define keyword, and compile and store defined routines */
	if (!strncmp(inPtr, "define", 6) && (inPtr[6]==' ' || inPtr[6]=='\t')) {
	    inPtr += 6;
	    inPtr += strspn(inPtr, " \t\n");
	    namePtr = subrName;
	    while (isalnum(*inPtr) || *inPtr == '_')
	    	*namePtr++ = *inPtr++;
	    *namePtr = '\0';
	    inPtr += strspn(inPtr, " \t\n");
	    if (*inPtr != '{') {
	    	if (errPos != NULL) *errPos = stoppedAt;
		return ParseError(dialogParent, string, inPtr,
	    	    	errIn, "expected '{'");
	    }
	    prog = ParseMacro(inPtr, &errMsg, &stoppedAt);
	    if (prog == NULL) {
	    	if (errPos != NULL) *errPos = stoppedAt;
	    	return ParseError(dialogParent, string, stoppedAt,
	    	    	errIn, errMsg);
	    }
	    if (runWindow != NULL) {
		sym = LookupSymbol(subrName);
		if (sym == NULL) {
		    subrPtr.val.ptr = prog;
		    subrPtr.tag = NO_TAG;
		    sym = InstallSymbol(subrName, MACRO_FUNCTION_SYM, subrPtr);
		} else {
	    	    if (sym->type == MACRO_FUNCTION_SYM)
		    	FreeProgram((Program *)sym->value.val.ptr);
		    else
			sym->type = MACRO_FUNCTION_SYM;
	    	    sym->value.val.ptr = prog;
		}
	    }
	    inPtr = stoppedAt;
	
	/* Parse and execute immediate (outside of any define) macro commands
	   and WAIT for them to finish executing before proceeding.  Note that
	   the code below is not perfect.  If you interleave code blocks with
	   definitions in a file which is loaded from another macro file, it
	   will probably run the code blocks in reverse order! */
	} else {
	    prog = ParseMacro(inPtr, &errMsg, &stoppedAt);
	    if (prog == NULL) {
	    	if (errPos != NULL) *errPos = stoppedAt;
    	    	return ParseError(dialogParent, string, stoppedAt,
	    	    	errIn, errMsg);
	    }
	    if (runWindow != NULL) {
    	    	XEvent nextEvent;	 
		if (runWindow->macroCmdData == NULL) {
	            runMacro(runWindow, prog);
		    while (runWindow->macroCmdData != NULL) {
			XtAppNextEvent(XtWidgetToApplicationContext(
				runWindow->shell),  &nextEvent);
			XtDispatchEvent(&nextEvent);
		    }
		} else
    		    RunMacroAsSubrCall(prog);
	    }
	    inPtr = stoppedAt;
    	}
    }
    return True;
}

/*
** Run a pre-compiled macro, changing the interface state to reflect that
** a macro is running, and handling preemption, resumption, and cancellation.
** frees prog when macro execution is complete;
*/
static void runMacro(WindowInfo *window, Program *prog)
{
    DataValue result;
    char *errMsg;
    int stat;
    macroCmdInfo *cmdData;
    XmString s;
    
    /* If a macro is already running, just call the program as a subroutine,
       instead of starting a new one, so we don't have to keep a separate
       context, and the macros will serialize themselves automatically */
    if (window->macroCmdData != NULL) {
    	RunMacroAsSubrCall(prog);
	return;
    }
    
    /* put up a watch cursor over the waiting window */
    BeginWait(window->shell);
    
    /* enable the cancel menu item */
    XtVaSetValues(window->cancelMacroItem, XmNlabelString,
    	    s=XmStringCreateSimple("Cancel Macro"), NULL);
    XmStringFree(s);
    XtSetSensitive(window->cancelMacroItem, True);

    /* Create a data structure for passing macro execution information around
       amongst the callback routines which will process i/o and completion */
    cmdData = (macroCmdInfo *)XtMalloc(sizeof(macroCmdInfo));
    window->macroCmdData = cmdData;
    cmdData->bannerIsUp = False;
    cmdData->closeOnCompletion = False;
    cmdData->program = prog;
    cmdData->context = NULL;
    cmdData->continueWorkProcID = 0;
    cmdData->dialog = NULL;
    
    /* Set up timer proc for putting up banner when macro takes too long */
    cmdData->bannerTimeoutID = XtAppAddTimeOut(
    	    XtWidgetToApplicationContext(window->shell), BANNER_WAIT_TIME,
    	    bannerTimeoutProc, window);
    
    /* Begin macro execution */
    stat = ExecuteMacro(window, prog, 0, NULL, &result, &cmdData->context,
    	    &errMsg);
    if (stat == MACRO_ERROR) {
    	finishMacroCmdExecution(window);
    	DialogF(DF_ERR, window->shell, 1, "Error executing macro: %s",
    	    	"Dismiss", errMsg);
    	return;
    }
    if (stat == MACRO_DONE) {
    	finishMacroCmdExecution(window);
    	return;
    }
    if (stat == MACRO_TIME_LIMIT) {
	ResumeMacroExecution(window);
	return;
    }
    /* (stat == MACRO_PREEMPT) Macro was preempted */
}

/*
** Continue with macro execution after preemption.  Called by the routines
** whose actions cause preemption when they have completed their lengthy tasks.
** Re-establishes macro execution work proc.  Window must be the window in
** which the macro is executing (the window to which macroCmdData is attached),
** and not the window to which operations are focused.
*/
void ResumeMacroExecution(WindowInfo *window)
{
    macroCmdInfo *cmdData = (macroCmdInfo *)window->macroCmdData;
    
    if (cmdData != NULL)
	cmdData->continueWorkProcID = XtAppAddWorkProc(
	    	XtWidgetToApplicationContext(window->shell),
	    	continueWorkProc, window);
}

/*
** Cancel the macro command in progress (user cancellation via GUI)
*/
void AbortMacroCommand(WindowInfo *window)
{
    if (window->macroCmdData == NULL)
    	return;
    
    /* If there's both a macro and a shell command executing, the shell command
       must have been called from the macro.  When called from a macro, shell
       commands don't put up cancellation controls of their own, but rely
       instead on the macro cancellation mechanism (here) */
#ifndef VMS
    if (window->shellCmdData != NULL)
    	AbortShellCommand(window);
#endif
    
    /* Free the continuation */
    FreeRestartData(((macroCmdInfo *)window->macroCmdData)->context);
    
    /* Kill the macro command */
    finishMacroCmdExecution(window);
}

/*
** Call this before closing a window, to clean up macro references to the
** window, stop any macro which might be running from it, free associated
** memory, and check that a macro is not attempting to close the window from
** which it is run.  If this is being called from a macro, and the window
** this routine is examining is the window from which the macro was run, this
** routine will return False, and the caller must NOT CLOSE THE WINDOW. 
** Instead, empty it and make it Untitled, and let the macro completion
** process close the window when the macro is finished executing.
*/
int MacroWindowCloseActions(WindowInfo *window)
{
    macroCmdInfo *mcd, *cmdData = window->macroCmdData;
    WindowInfo *w;

    /* If no macro is executing in the window, allow the close, but check
       if macros executing in other windows have it as focus.  If so, set
       their focus back to the window from which they were originally run */
    if (cmdData == NULL) {
    	for (w=WindowList; w!=NULL; w=w->next) {
	    mcd = (macroCmdInfo *)w->macroCmdData;
	    if (w == MacroRunWindow() && MacroFocusWindow() == window)
		SetMacroFocusWindow(MacroRunWindow());
	    else if (mcd != NULL && mcd->context->focusWindow == window)
		mcd->context->focusWindow = mcd->context->runWindow;
	}
    	return True;
    }
    
    /* If the macro currently running (and therefore calling us, because
       execution must otherwise return to the main loop to execute any
       commands), is running in this window, tell the caller not to close,
       and schedule window close on completion of macro */
    if (window == MacroRunWindow()) {
    	cmdData->closeOnCompletion = True;
	return False;
    }
    
    /* Free the continuation */
    FreeRestartData(cmdData->context);
    
    /* Kill the macro command */
    finishMacroCmdExecution(window);
    return True;
}

/*
** Clean up after the execution of a macro command: free memory, and restore
** the user interface state.
*/
static void finishMacroCmdExecution(WindowInfo *window)
{
    macroCmdInfo *cmdData = window->macroCmdData;
    int closeOnCompletion = cmdData->closeOnCompletion;
    XmString s;
    XClientMessageEvent event;

    /* Cancel pending timeout and work proc */
    if (cmdData->bannerTimeoutID != 0)
    	XtRemoveTimeOut(cmdData->bannerTimeoutID);
    if (cmdData->continueWorkProcID != 0)
    	XtRemoveWorkProc(cmdData->continueWorkProcID);
    
    /* Clean up waiting-for-macro-command-to-complete mode */
    EndWait(window->shell);
    XtVaSetValues(window->cancelMacroItem, XmNlabelString,
    	    s=XmStringCreateSimple("Cancel Learn"), NULL);
    XmStringFree(s);
    XtSetSensitive(window->cancelMacroItem, False);
    if (cmdData->bannerIsUp)
    	ClearModeMessage(window);

    /* If a dialog was up, get rid of it */
    if (cmdData->dialog != NULL)
    	XtDestroyWidget(XtParent(cmdData->dialog));

    /* Free execution information */
    FreeProgram(cmdData->program);
    XtFree((char *)cmdData);
    window->macroCmdData = NULL;
    
    /* If macro closed its own window, window was made empty and untitled,
       but close was deferred until completion.  This is completion, so if
       the window is still empty, do the close */
    if (closeOnCompletion && !window->filenameSet && !window->fileChanged) {
    	CloseWindow(window);
	window = NULL;
    }

    /* If no other macros are executing, do garbage collection */
    SafeGC();
    
    /* In processing the .neditmacro file (and possibly elsewhere), there
       is an event loop which waits for macro completion.  Send an event
       to wake up that loop, otherwise execution will stall until the user
       does something to the window. */
    if (!closeOnCompletion) {
	event.format = 8;
	event.type = ClientMessage;
	XSendEvent(XtDisplay(window->shell), XtWindow(window->shell), False,
		NoEventMask, (XEvent *)&event);
    }
}

/*
** Do garbage collection of strings if there are no macros currently
** executing.  NEdit's macro language GC strategy is to call this routine
** whenever a macro completes.  If other macros are still running (preempted
** or waiting for a shell command or dialog), this does nothing and therefore
** defers GC to the completion of the last macro out.
*/
void SafeGC(void)
{
    WindowInfo *win;
    
    for (win=WindowList; win!=NULL; win=win->next)
	if (win->macroCmdData != NULL)
	    return;
    GarbageCollectStrings();
}

/*
** Executes macro string "macro" using the lastFocus pane in "window".
** Reports errors via a dialog posted over "window", integrating the name
** "errInName" into the message to help identify the source of the error.
*/
void DoMacro(WindowInfo *window, char *macro, char *errInName)
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
    
    /* Parse the macro and report errors if it fails */
    prog = ParseMacro(tMacro, &errMsg, &stoppedAt);
    if (prog == NULL) {
    	ParseError(window->shell, tMacro, stoppedAt, errInName, errMsg);
	XtFree(tMacro);
    	return;
    }
    XtFree(tMacro);

    /* run the executable program (prog is freed upon completion) */
    runMacro(window, prog);
}

/*
** Get the current Learn/Replay macro in text form.  Returned string is a
** pointer to the stored macro and should not be freed by the caller (and
** will cease to exist when the next replay macro is installed)
*/
char *GetReplayMacro(void)
{
    return ReplayMacro;
}

/*
** Present the user a dialog for "Repeat" command
*/
void RepeatDialog(WindowInfo *window)
{
    Widget form, selBox, radioBox, timesForm;
    repeatDialog *rd;
    Arg selBoxArgs[1];
    char *lastCmdLabel, *parenChar;
    XmString s1;
    int cmdNameLen;

    if (LastCommand == NULL) {
    	DialogF(DF_WARN, window->shell, 1,
	    	"No previous commands or learn/\nreplay sequences to repeat",
		"Dismiss");
	return;
    }
    
    /* Remeber the last command, since the user is allowed to work in the
       window while the dialog is up */
    rd = (repeatDialog *)XtMalloc(sizeof(repeatDialog));
    rd->lastCommand = XtNewString(LastCommand);
    
    /* make a label for the Last command item of the dialog, which includes
       the last executed action name */
    parenChar = strchr(LastCommand, '(');
    if (parenChar == NULL)
	return;
    cmdNameLen = parenChar-LastCommand;
    lastCmdLabel = XtMalloc(16 + cmdNameLen);
    strcpy(lastCmdLabel, "Last Command (");
    strncpy(&lastCmdLabel[14], LastCommand, cmdNameLen);
    strcpy(&lastCmdLabel[14 + cmdNameLen], ")");
    
    XtSetArg(selBoxArgs[0], XmNautoUnmanage, False);
    selBox = CreatePromptDialog(window->shell, "repeat", selBoxArgs, 1);
    rd->shell = XtParent(selBox);
    XtAddCallback(rd->shell, XmNdestroyCallback, repeatDestroyCB, rd);
    XtAddCallback(selBox, XmNokCallback, repeatOKCB, rd);
    XtAddCallback(selBox, XmNapplyCallback, repeatApplyCB, rd);
    XtAddCallback(selBox, XmNcancelCallback, repeatCancelCB, rd);
    XtUnmanageChild(XmSelectionBoxGetChild(selBox, XmDIALOG_TEXT));
    XtUnmanageChild(XmSelectionBoxGetChild(selBox, XmDIALOG_SELECTION_LABEL));
    XtUnmanageChild(XmSelectionBoxGetChild(selBox, XmDIALOG_HELP_BUTTON));
    XtUnmanageChild(XmSelectionBoxGetChild(selBox, XmDIALOG_APPLY_BUTTON));
    XtVaSetValues(XtParent(selBox), XmNtitle, "Repeat", NULL);
    AddMotifCloseCallback(XtParent(selBox), repeatCancelCB, rd);
    
    form = XtVaCreateManagedWidget("form", xmFormWidgetClass, selBox, NULL);

    radioBox = XtVaCreateManagedWidget("cmdSrc", xmRowColumnWidgetClass, form,
    	    XmNradioBehavior, True,
	    XmNorientation, XmHORIZONTAL,
	    XmNpacking, XmPACK_TIGHT,
	    XmNtopAttachment, XmATTACH_FORM,
    	    XmNleftAttachment, XmATTACH_FORM, NULL);
    rd->lastCmdToggle = XtVaCreateManagedWidget("lastCmdToggle",
    	    xmToggleButtonWidgetClass, radioBox, XmNset, True,
	    XmNlabelString, s1=XmStringCreateSimple(lastCmdLabel),
	    XmNmnemonic, 'C', NULL);
    XmStringFree(s1);
    XtFree(lastCmdLabel);
    XtVaCreateManagedWidget("learnReplayToggle",
    	    xmToggleButtonWidgetClass, radioBox, XmNset, False,
	    XmNlabelString,
	    	s1=XmStringCreateSimple("Learn/Replay"),
	    XmNmnemonic, 'L',
	    XmNsensitive, ReplayMacro != NULL, NULL);
    XmStringFree(s1);

    timesForm = XtVaCreateManagedWidget("form", xmFormWidgetClass, form,
	    XmNtopAttachment, XmATTACH_WIDGET,
    	    XmNtopWidget, radioBox,
	    XmNtopOffset, 10,
    	    XmNleftAttachment, XmATTACH_FORM, NULL);
    radioBox = XtVaCreateManagedWidget("method", xmRowColumnWidgetClass,
	    timesForm,
    	    XmNradioBehavior, True,
	    XmNorientation, XmHORIZONTAL,
	    XmNpacking, XmPACK_TIGHT,
	    XmNtopAttachment, XmATTACH_FORM,
    	    XmNbottomAttachment, XmATTACH_FORM, 
    	    XmNleftAttachment, XmATTACH_FORM, NULL);
    rd->inSelToggle = XtVaCreateManagedWidget("inSelToggle",
    	    xmToggleButtonWidgetClass, radioBox, XmNset, False,
	    XmNlabelString, s1=XmStringCreateSimple("In Selection"),
	    XmNmnemonic, 'I', NULL);
    XmStringFree(s1);
    rd->toEndToggle = XtVaCreateManagedWidget("toEndToggle",
    	    xmToggleButtonWidgetClass, radioBox, XmNset, False,
	    XmNlabelString, s1=XmStringCreateSimple("To End"),
	    XmNmnemonic, 'T', NULL);
    XmStringFree(s1);
    XtVaCreateManagedWidget("nTimesToggle",
    	    xmToggleButtonWidgetClass, radioBox, XmNset, True,
	    XmNlabelString, s1=XmStringCreateSimple("N Times"),
	    XmNmnemonic, 'N',
	    XmNset, True, NULL);
    XmStringFree(s1);
    rd->repeatText = XtVaCreateManagedWidget("repeatText", xmTextWidgetClass,
	    timesForm,
    	    XmNcolumns, 5,
    	    XmNtopAttachment, XmATTACH_FORM,
	    XmNbottomAttachment, XmATTACH_FORM,
    	    XmNleftAttachment, XmATTACH_WIDGET,
    	    XmNleftWidget, radioBox, NULL);
    RemapDeleteKey(rd->repeatText);

    /* Handle mnemonic selection of buttons and focus to dialog */
    AddDialogMnemonicHandler(form);

    /* Set initial focus */
#if XmVersion >= 1002
    XtVaSetValues(form, XmNinitialFocus, timesForm, NULL);
    XtVaSetValues(timesForm, XmNinitialFocus, rd->repeatText, NULL);
#endif
    
    /* put up dialog */
    rd->forWindow = window;
    ManageDialogCenteredOnPointer(selBox);
}

static void repeatOKCB(Widget w, XtPointer clientData, XtPointer callData)
{
    repeatDialog *rd = (repeatDialog *)clientData;

    if (doRepeatDialogAction(rd, ((XmAnyCallbackStruct *)callData)->event))
    	XtDestroyWidget(rd->shell);
}

/* Note that the apply button is not managed in the repeat dialog.  The dialog
   itself is capable of non-modal operation, but to be complete, it needs
   to dynamically update last command, dimming of learn/replay, possibly a
   stop button for the macro, and possibly in-selection with selection */
static void repeatApplyCB(Widget w, XtPointer clientData, XtPointer callData)
{
    doRepeatDialogAction((repeatDialog *)clientData,
	    ((XmAnyCallbackStruct *)callData)->event);
}

static int doRepeatDialogAction(repeatDialog *rd, XEvent *event)
{
    int nTimes;
    char nTimesStr[25];
    char *params[2];
    
    /* Find out from the dialog how to repeat the command */
    if (XmToggleButtonGetState(rd->inSelToggle)) {
	if (!rd->forWindow->buffer->primary.selected) {
	    DialogF(DF_WARN, rd->shell, 1,
	    	    "No selection in window to repeat within", "Dismiss");
	    XmProcessTraversal(rd->inSelToggle, XmTRAVERSE_CURRENT);
	    return False;
	}
	params[0] = "in_selection";
    } else if (XmToggleButtonGetState(rd->toEndToggle)) {
	params[0] = "to_end";
    } else {
	if (GetIntTextWarn(rd->repeatText, &nTimes, "number of times", True) != 
		TEXT_READ_OK) {
   	    XmProcessTraversal(rd->repeatText, XmTRAVERSE_CURRENT);
	    return False;
	}
	sprintf(nTimesStr, "%d", nTimes);
	params[0] = nTimesStr;
    }
    
    /* Figure out which command user wants to repeat */
    if (XmToggleButtonGetState(rd->lastCmdToggle))
	params[1] = CopyAllocatedString(rd->lastCommand);
    else {
	if (ReplayMacro == NULL)
	    return False;
	params[1] = CopyAllocatedString(ReplayMacro);
    }

    /* call the action routine repeat_macro to do the work */
    XtCallActionProc(rd->forWindow->lastFocus, "repeat_macro", event, params,2);
    XtFree(params[1]);
    return True;
}

static void repeatCancelCB(Widget w, XtPointer clientData, XtPointer callData)
{
    repeatDialog *rd = (repeatDialog *)clientData;

    XtDestroyWidget(rd->shell);
}

static void repeatDestroyCB(Widget w, XtPointer clientData, XtPointer callData)
{
    repeatDialog *rd = (repeatDialog *)clientData;
    
    XtFree(rd->lastCommand);
    XtFree((char *)rd);
}

/*
** Dispatches a macro to which repeats macro command in "command", either
** an integer number of times ("how" == positive integer), or within a
** selected range ("how" == REPEAT_IN_SEL), or to the end of the window
** ("how == REPEAT_TO_END).
**
** Note that as with most macro routines, this returns BEFORE the macro is
** finished executing
*/
void RepeatMacro(WindowInfo *window, char *command, int how)
{
    Program *prog;
    char *errMsg, *stoppedAt, *loopMacro, *loopedCmd;

    if (command == NULL)
	return;
    
    /* Wrap a for loop and counter/tests around the command */
    if (how == REPEAT_TO_END)
	loopMacro = "lastCursor=-1\nstartPos=$cursor\n\
while($cursor>=startPos&&$cursor!=lastCursor){\nlastCursor=$cursor\n%s\n}\n";
    else if (how == REPEAT_IN_SEL)
	loopMacro = "selStart = $selection_start\nif (selStart == -1)\nreturn\n\
selEnd = $selection_end\nset_cursor_pos(selStart)\nselect(0,0)\n\
boundText = get_range(selEnd, selEnd+10)\n\
while($cursor >= selStart && $cursor < selEnd && \\\n\
get_range(selEnd, selEnd+10) == boundText) {\n\
startLength = $text_length\n%s\n\
selEnd += $text_length - startLength\n}\n";
    else
    	loopMacro = "for(i=0;i<%d;i++){\n%s\n}\n";
    loopedCmd = XtMalloc(strlen(command) + strlen(loopMacro) + 25);
    if (how == REPEAT_TO_END || how == REPEAT_IN_SEL)
	sprintf(loopedCmd, loopMacro, command);
    else
	sprintf(loopedCmd, loopMacro, how, command);
    
    /* Parse the resulting macro into an executable program "prog" */
    prog = ParseMacro(loopedCmd, &errMsg, &stoppedAt);
    if (prog == NULL) {
	fprintf(stderr, "NEdit internal error, repeat macro syntax wrong: %s\n",
    		errMsg);
    	return;
    }
    XtFree(loopedCmd);

    /* run the executable program */
    runMacro(window, prog);
}

/*
** Macro recording action hook for Learn/Replay, added temporarily during
** learn.
*/
static void learnActionHook(Widget w, XtPointer clientData, String actionName,
	XEvent *event, String *params, Cardinal *numParams)
{
    WindowInfo *window;
    int i;
    char *actionString;
    
    /* Select only actions in text panes in the window for which this
       action hook is recording macros (from clientData). */
    for (window=WindowList; window!=NULL; window=window->next) {
	if (window->textArea == w)
	    break;
	for (i=0; i<window->nPanes; i++) {
    	    if (window->textPanes[i] == w)
    	    	break;
	}
	if (i < window->nPanes)
	    break;
    }
    if (window == NULL || window != (WindowInfo *)clientData)
    	return;
    
    /* beep on un-recordable operations which require a mouse position, to
       remind the user that the action was not recorded */
    if (isMouseAction(actionName)) {
    	XBell(XtDisplay(w), 0);
    	return;
    }
    
    /* Record the action and its parameters */
    actionString = actionToString(actionName, event, params, *numParams);
    if (actionString != NULL) {
	BufInsert(MacroRecordBuf, MacroRecordBuf->length, actionString);
	XtFree(actionString);
    }
}

/*
** Permanent action hook for remembering last action for possible replay
*/
static void lastActionHook(Widget w, XtPointer clientData, String actionName,
	XEvent *event, String *params, Cardinal *numParams)
{
    WindowInfo *window;
    int i;
    char *actionString;
    
    /* Find the window to which this action belongs */
    for (window=WindowList; window!=NULL; window=window->next) {
	if (window->textArea == w)
	    break;
	for (i=0; i<window->nPanes; i++) {
    	    if (window->textPanes[i] == w)
    	    	break;
	}
	if (i < window->nPanes)
	    break;
    }
    if (window == NULL)
    	return;

    /* The last action is recorded for the benefit of repeating the last
       action.  Don't record repeat_macro and wipe out the real action */
    if (!strcmp(actionName, "repeat_macro"))
	return;
        
    /* Record the action and its parameters */
    actionString = actionToString(actionName, event, params, *numParams);
    if (actionString != NULL) {
	if (LastCommand != NULL)
	    XtFree(LastCommand);
	LastCommand = actionString;
    }
}

/*
** Create a macro string to represent an invocation of an action routine.
** Returns NULL for non-operational or un-recordable actions.
*/
char *actionToString(char *actionName, XEvent *event, String *params,
	Cardinal numParams)
{
    char chars[20], *charList[1], *outStr, *outPtr;
    KeySym keysym;
    int i, nChars, nParams, length, nameLength;
    
    if (isIgnoredAction(actionName) || isRedundantAction(actionName) ||
	    isMouseAction(actionName))
    	return NULL;
    
    /* Convert self_insert actions, to insert_string */
    if (!strcmp(actionName, "self_insert") ||
    	    !strcmp(actionName, "self-insert")) {
    	actionName = "insert_string";
	nChars = XLookupString((XKeyEvent *)event, chars, 19, &keysym, NULL);
	if (nChars == 0)
	    return NULL;
    	chars[nChars] = '\0';
    	charList[0] = chars;
    	params = charList;
    	nParams = 1;
    } else
    	nParams = numParams;
    	
    /* Figure out the length of string required */
    nameLength = strlen(actionName);
    length = nameLength + 3;
    for (i=0; i<nParams; i++)
	length += escapedStringLength(params[i]) + 4;
    
    /* Allocate the string and copy the information to it */
    outPtr = outStr = XtMalloc(length + 1);
    strcpy(outPtr, actionName);
    outPtr += nameLength;
    *outPtr++ = '(';
    for (i=0; i<nParams; i++) {
	*outPtr++ = '\"';
	outPtr += escapeStringChars(params[i], outPtr);
	*outPtr++ = '\"'; *outPtr++ = ','; *outPtr++ = ' ';
    }
    if (nParams != 0)
	outPtr -= 2;
    *outPtr++ = ')'; *outPtr++ = '\n'; *outPtr++ = '\0';
    return outStr;
}

static int isMouseAction(char *action)
{
    int i;
    
    for (i=0; i<XtNumber(MouseActions); i++)
    	if (!strcmp(action, MouseActions[i]))
    	    return True;
    return False;
}

static int isRedundantAction(char *action)
{
    int i;
    
    for (i=0; i<XtNumber(RedundantActions); i++)
    	if (!strcmp(action, RedundantActions[i]))
    	    return True;
    return False;
}

static int isIgnoredAction(char *action)
{
    int i;
    
    for (i=0; i<XtNumber(IgnoredActions); i++)
    	if (!strcmp(action, IgnoredActions[i]))
    	    return True;
    return False;
}

/*
** Timer proc for putting up the "Macro Command in Progress" banner if
** the process is taking too long.
*/
static void bannerTimeoutProc(XtPointer clientData, XtIntervalId *id)
{
    WindowInfo *window = (WindowInfo *)clientData;
    macroCmdInfo *cmdData = window->macroCmdData;
    
    cmdData->bannerIsUp = True;
    SetModeMessage(window,
    	    "Macro Command in Progress -- Press Ctrl+. to Cancel");
    cmdData->bannerTimeoutID = 0;
}

/*
** Work proc for continuing execution of a preempted macro.
**
** Xt WorkProcs are designed to run first-in first-out, which makes them
** very bad at sharing time between competing tasks.  For this reason, it's
** usually bad to use work procs anywhere where their execution is likely to
** overlap.  Using a work proc instead of a timer proc (which I usually
** prefer) here means macros will probably share time badly, but we're more
** interested in making the macros cancelable, and in continuing other work
** than having users run a bunch of them at once together.
*/
static Boolean continueWorkProc(XtPointer clientData)
{
    WindowInfo *window = (WindowInfo *)clientData;
    macroCmdInfo *cmdData = window->macroCmdData;
    char *errMsg;
    int stat;
    DataValue result;
    
    stat = ContinueMacro(cmdData->context, &result, &errMsg);
    if (stat == MACRO_ERROR) {
    	finishMacroCmdExecution(window);
    	DialogF(DF_ERR, window->shell, 1, "Error executing macro: %s",
    	    	"Dismiss", errMsg);
    	return True;
    } else if (stat == MACRO_DONE) {
    	finishMacroCmdExecution(window);
    	return True;
    } else if (stat == MACRO_PREEMPT) {
	cmdData->continueWorkProcID = 0;
    	return True;
    }
    
    /* Macro exceeded time slice, re-schedule it */
    if (stat != MACRO_TIME_LIMIT)
    	return True; /* shouldn't happen */
    return False;
}

/*
** Copy fromString to toString replacing special characters in strings, such
** that they can be read back by the macro parser's string reader.  i.e. double
** quotes are replaced by \", backslashes are replaced with \\, C-std control
** characters like \n are replaced with their backslash counterparts.  This
** routine should be kept reasonably in sync with yylex in parse.y.  Companion
** routine escapedStringLength predicts the length needed to write the string
** when it is expanded with the additional characters.  Returns the number
** of characters to which the string expanded.
*/
static int escapeStringChars(char *fromString, char *toString)
{
    char *e, *c, *outPtr = toString;
    
    /* substitute escape sequences */
    for (c=fromString; *c!='\0'; c++) {
    	for (e=EscapeChars; *e!='\0'; e++) {
    	    if (*c == *e) {
    		*outPtr++ = '\\';
    		*outPtr++ = ReplaceChars[e-EscapeChars];
		break;
	    }
	}
	if (*e == '\0')
    	   *outPtr++ = *c;
    }
    *outPtr = '\0';
    return outPtr - toString;
}

/*
** Predict the length of a string needed to hold a copy of "string" with
** special characters replaced with escape sequences by escapeStringChars.
*/
static int escapedStringLength(char *string)
{
    char *c, *e;
    int length = 0;

    /* calculate length and allocate returned string */
    for (c=string; *c!='\0'; c++) {
    	for (e=EscapeChars; *e!='\0'; e++) {
	    if (*c == *e) {
    		length++;
		break;
	    }
	}
    	length++;
    }
    return length;
}

/*
** Built-in macro subroutine for getting the length of a string
*/
static int lengthMS(WindowInfo *window, DataValue *argList, int nArgs,
    	DataValue *result, char **errMsg)
{
    char *string, stringStorage[25];
    
    if (nArgs != 1)
    	return wrongNArgsErr(errMsg);
    if (!readStringArg(argList[0], &string, stringStorage, errMsg))
	return False;
    result->tag = INT_TAG;
    result->val.n = strlen(string);
    return True;
}

/*
** Built-in macro subroutines for min and max
*/
static int minMS(WindowInfo *window, DataValue *argList, int nArgs,
    	DataValue *result, char **errMsg)
{
    int minVal, value, i;
    
    if (nArgs == 1)
    	return tooFewArgsErr(errMsg);
    if (!readIntArg(argList[0], &minVal, errMsg))
    	return False;
    for (i=0; i<nArgs; i++) {
	if (!readIntArg(argList[i], &value, errMsg))
    	    return False;
    	minVal = value < minVal ? value : minVal;
    }
    result->tag = INT_TAG;
    result->val.n = minVal;
    return True;
}
static int maxMS(WindowInfo *window, DataValue *argList, int nArgs,
    	DataValue *result, char **errMsg)
{
    int maxVal, value, i;
    
    if (nArgs == 1)
    	return tooFewArgsErr(errMsg);
    if (!readIntArg(argList[0], &maxVal, errMsg))
    	return False;
    for (i=0; i<nArgs; i++) {
	if (!readIntArg(argList[i], &value, errMsg))
    	    return False;
    	maxVal = value > maxVal ? value : maxVal;
    }
    result->tag = INT_TAG;
    result->val.n = maxVal;
    return True;
}

static int focusWindowMS(WindowInfo *window, DataValue *argList, int nArgs,
      DataValue *result, char **errMsg)
{
    char stringStorage[25], *string;
    WindowInfo *w;
    char fullname[MAXPATHLEN];

    /* Read the argument representing the window to focus to, and translate
       it into a pointer to a real WindowInfo */
    if (nArgs != 1)
    	return wrongNArgsErr(errMsg);
    if (!readStringArg(argList[0], &string, stringStorage, errMsg))
    	return False;
    else if (!strcmp(string, "last"))
	w = WindowList;
    else if (!strcmp(string, "next"))
	w = window->next;
    else {
	for (w=WindowList; w != NULL; w = w->next) {
	    sprintf(fullname, "%s%s", w->path, w->filename);
	    if (!strcmp(string, fullname))
		break;
	}
    }
    
    /* If no matching window was found, return empty string and do nothing */
    if (w == NULL) {
	result->tag = STRING_TAG;
	result->tag = STRING_TAG;
	result->val.str = AllocString(1);
	result->val.str[0] = '\0';
	return True;
    }

    /* Change the focused window to the requested one */
    SetMacroFocusWindow(w);

    /* Return the name of the window */
    result->tag = STRING_TAG;
    result->val.str = AllocString(strlen(w->path)+strlen(w->filename)+1);
    sprintf(result->val.str, "%s%s", w->path, w->filename);
    return True;
}

/*
** Built-in macro subroutine for getting text from the current window's text
** buffer
*/
static int getRangeMS(WindowInfo *window, DataValue *argList, int nArgs,
    	DataValue *result, char **errMsg)
{
    int from, to;
    textBuffer *buf = window->buffer;
    char *rangeText;
    
    /* Validate arguments and convert to int */
    if (nArgs != 2)
    	return wrongNArgsErr(errMsg);
    if (!readIntArg(argList[0], &from, errMsg))
    	return False;
    if (!readIntArg(argList[1], &to, errMsg))
	return False;
    if (from < 0) from = 0;
    if (from > buf->length) from = buf->length;
    if (to < 0) to = 0;
    if (to > buf->length) to = buf->length;
    if (from > to) {int temp = from; from = to; to = temp;}
    
    /* Copy text from buffer (this extra copy could be avoided if textBuf.c
       provided a routine for writing into a pre-allocated string) */
    result->tag = STRING_TAG;
    result->val.str = AllocString(to - from + 1);
    rangeText = BufGetRange(buf, from, to);
    BufUnsubstituteNullChars(rangeText, buf);
    strcpy(result->val.str, rangeText);
    XtFree(rangeText);
    return True;
}

/*
** Built-in macro subroutine for getting a single character at the position
** given, from the current window
*/
static int getCharacterMS(WindowInfo *window, DataValue *argList, int nArgs,
    	DataValue *result, char **errMsg)
{
    int pos;
    textBuffer *buf = window->buffer;
    
    /* Validate argument and convert it to int */
    if (nArgs != 1)
    	return wrongNArgsErr(errMsg);
    if (!readIntArg(argList[0], &pos, errMsg))
    	return False;
    if (pos < 0) pos = 0;
    if (pos > buf->length) pos = buf->length;
    
    /* Return the character in a pre-allocated string) */
    result->tag = STRING_TAG;
    result->val.str =  AllocString(2);
    result->val.str[0] = BufGetCharacter(buf, pos);
    result->val.str[1] = '\0';
    BufUnsubstituteNullChars(result->val.str, buf);
    return True;
}

/*
** Built-in macro subroutine for replacing text in the current window's text
** buffer
*/
static int replaceRangeMS(WindowInfo *window, DataValue *argList, int nArgs,
    	DataValue *result, char **errMsg)
{
    int from, to;
    char stringStorage[25], *string;
    textBuffer *buf = window->buffer;
    
    /* Validate arguments and convert to int */
    if (nArgs != 3)
    	return wrongNArgsErr(errMsg);
    if (!readIntArg(argList[0], &from, errMsg))
    	return False;
    if (!readIntArg(argList[1], &to, errMsg))
	return False;
    if (!readStringArg(argList[2], &string, stringStorage, errMsg))
    	return False;
    if (from < 0) from = 0;
    if (from > buf->length) from = buf->length;
    if (to < 0) to = 0;
    if (to > buf->length) to = buf->length;
    if (from > to) {int temp = from; from = to; to = temp;}
    
    /* Don't allow modifications if the window is read-only */
    if (window->readOnly || window->lockWrite) {
	XBell(XtDisplay(window->shell), 0);
	result->tag = NO_TAG;
	return True;
    }
    
    /* There are no null characters in the string (because macro strings
       still have null termination), but if the string contains the
       character used by the buffer for null substitution, it could
       theoretically become a null.  In the highly unlikely event that
       all of the possible substitution characters in the buffer are used
       up, stop the macro and tell the user of the failure */
    if (!BufSubstituteNullChars(string, strlen(string), window->buffer)) {
	*errMsg = "Too much binary data in file";
	return False;
    }

    /* Do the replace */
    BufReplace(buf, from, to, string);
    result->tag = NO_TAG;
    return True;
}

/*
** Built-in macro subroutine for replacing the primary-selection selected
** text in the current window's text buffer
*/
static int replaceSelectionMS(WindowInfo *window, DataValue *argList, int nArgs,
    	DataValue *result, char **errMsg)
{
    char stringStorage[25], *string;
    
    /* Validate argument and convert to string */
    if (nArgs != 1)
    	return wrongNArgsErr(errMsg);
    if (!readStringArg(argList[0], &string, stringStorage, errMsg))
    	return False;
    
    /* Don't allow modifications if the window is read-only */
    if (window->readOnly || window->lockWrite) {
	XBell(XtDisplay(window->shell), 0);
	result->tag = NO_TAG;
	return True;
    }
     
    /* There are no null characters in the string (because macro strings
       still have null termination), but if the string contains the
       character used by the buffer for null substitution, it could
       theoretically become a null.  In the highly unlikely event that
       all of the possible substitution characters in the buffer are used
       up, stop the macro and tell the user of the failure */
    if (!BufSubstituteNullChars(string, strlen(string), window->buffer)) {
	*errMsg = "Too much binary data in file";
	return False;
    }
    
    /* Do the replace */
    BufReplaceSelected(window->buffer, string);
    result->tag = NO_TAG;
    return True;
}

/*
** Built-in macro subroutine for getting the text currently selected by
** the primary selection in the current window's text buffer, or in any
** part of screen if "any" argument is given
*/
static int getSelectionMS(WindowInfo *window, DataValue *argList, int nArgs,
    	DataValue *result, char **errMsg)
{
    char *selText;

    /* Read argument list to check for "any" keyword, and get the appropriate
       selection */
    if (nArgs != 0 && nArgs != 1)
      	return wrongNArgsErr(errMsg);
    if (nArgs == 1) {
        if (argList[0].tag != STRING_TAG || strcmp(argList[0].val.str, "any")) {
	    *errMsg = "unrecognized argument to %s";
	    return False;
    	}
	selText = GetAnySelection(window);
	if (selText == NULL)
	    selText = XtNewString("");
    } else {
	selText = BufGetSelectionText(window->buffer);
    	BufUnsubstituteNullChars(selText, window->buffer);
    }
	
    /* Return the text as an allocated string */
    result->tag = STRING_TAG;
    result->val.str = AllocString(strlen(selText) + 1);
    strcpy(result->val.str, selText);
    XtFree(selText);
    return True;
}

/*
** Built-in macro subroutine for replacing a substring within another string
*/
static int replaceSubstringMS(WindowInfo *window, DataValue *argList, int nArgs,
    	DataValue *result, char **errMsg)
{
    int from, to, length, replaceLen, outLen;
    char stringStorage[2][25], *string, *replStr;
    
    /* Validate arguments and convert to int */
    if (nArgs != 4)
    	return wrongNArgsErr(errMsg);
    if (!readStringArg(argList[0], &string, stringStorage[1], errMsg))
    	return False;
    if (!readIntArg(argList[1], &from, errMsg))
    	return False;
    if (!readIntArg(argList[2], &to, errMsg))
	return False;
    if (!readStringArg(argList[3], &replStr, stringStorage[1], errMsg))
    	return False;
    length = strlen(string);
    if (from < 0) from = 0;
    if (from > length) from = length;
    if (to < 0) to = 0;
    if (to > length) to = length;
    if (from > to) {int temp = from; from = to; to = temp;}
    
    /* Allocate a new string and do the replacement */
    replaceLen = strlen(replStr);
    outLen = length - (to - from) + replaceLen;
    result->tag = STRING_TAG;
    result->val.str = AllocString(outLen+1);
    strncpy(result->val.str, string, from);
    strncpy(&result->val.str[from], replStr, replaceLen);
    strncpy(&result->val.str[from + replaceLen], &string[to], length - to);
    result->val.str[outLen] = '\0';
    return True;
}

/*
** Built-in macro subroutine for getting a substring of a string
*/
static int substringMS(WindowInfo *window, DataValue *argList, int nArgs,
    	DataValue *result, char **errMsg)
{
    int from, to, length;
    char stringStorage[25], *string;
    
    /* Validate arguments and convert to int */
    if (nArgs != 3)
    	return wrongNArgsErr(errMsg);
    if (!readStringArg(argList[0], &string, stringStorage, errMsg))
    	return False;
    if (!readIntArg(argList[1], &from, errMsg))
    	return False;
    if (!readIntArg(argList[2], &to, errMsg))
	return False;
    length = strlen(string);
    if (from < 0) from = 0;
    if (from > length) from = length;
    if (to < 0) to = 0;
    if (to > length) to = length;
    if (from > to) {int temp = from; from = to; to = temp;}
    
    /* Allocate a new string and copy the sub-string into it */
    result->tag = STRING_TAG;
    result->val.str = AllocString(to - from + 1);
    strncpy(result->val.str, &string[from], to - from);
    result->val.str[to - from] = '\0';
    return True;
}

static int toupperMS(WindowInfo *window, DataValue *argList, int nArgs,
    	DataValue *result, char **errMsg)
{
    int i, length;
    char stringStorage[25], *string;
    
    /* Validate arguments and convert to int */
    if (nArgs != 1)
    	return wrongNArgsErr(errMsg);
    if (!readStringArg(argList[0], &string, stringStorage, errMsg))
    	return False;
    length = strlen(string);
    
    /* Allocate a new string and copy an uppercased version of the string it */
    result->tag = STRING_TAG;
    result->val.str = AllocString(length + 1);
    for (i=0; i<length; i++)
    	result->val.str[i] = toupper((unsigned char)string[i]);
    result->val.str[length] = '\0';
    return True;
}

static int tolowerMS(WindowInfo *window, DataValue *argList, int nArgs,
    	DataValue *result, char **errMsg)
{
    int i, length;
    char stringStorage[25], *string;
    
    /* Validate arguments and convert to int */
    if (nArgs != 1)
    	return wrongNArgsErr(errMsg);
    if (!readStringArg(argList[0], &string, stringStorage, errMsg))
    	return False;
    length = strlen(string);
    
    /* Allocate a new string and copy an lowercased version of the string it */
    result->tag = STRING_TAG;
    result->val.str = AllocString(length + 1);
    for (i=0; i<length; i++)
    	result->val.str[i] = tolower((unsigned char)string[i]);
    result->val.str[length] = '\0';
    return True;
}

static int stringToClipboardMS(WindowInfo *window, DataValue *argList, int nArgs,
    	DataValue *result, char **errMsg)
{
    long itemID = 0;
    XmString s;
    int stat;
    char stringStorage[25], *string;
    
    /* Get the string argument */
    if (nArgs != 1)
    	return wrongNArgsErr(errMsg);
    if (!readStringArg(argList[0], &string, stringStorage, errMsg))
    	return False;
    
    /* Use the XmClipboard routines to copy the text to the clipboard.
       If errors occur, just give up.  */
    result->tag = NO_TAG;
    stat = XmClipboardStartCopy(TheDisplay, XtWindow(window->textArea),
    	  s=XmStringCreateSimple("NEdit"), XtLastTimestampProcessed(TheDisplay),
	  window->textArea, NULL, &itemID);
    XmStringFree(s);
    if (stat != ClipboardSuccess)
    	return True;
    if (XmClipboardCopy(TheDisplay, XtWindow(window->textArea), itemID, "STRING",
    	    string, strlen(string), 0, NULL) != ClipboardSuccess)
    	return True;
    XmClipboardEndCopy(TheDisplay, XtWindow(window->textArea), itemID);
    return True;
}

static int clipboardToStringMS(WindowInfo *window, DataValue *argList, int nArgs,
    	DataValue *result, char **errMsg)
{
    unsigned long length, retLength;
    long id = 0;

    /* Should have no arguments */
    if (nArgs != 0)
    	return wrongNArgsErr(errMsg);
    
    /* Ask if there's a string in the clipboard, and get its length */
    if (XmClipboardInquireLength(TheDisplay, XtWindow(window->shell), "STRING",
    	    &length) != ClipboardSuccess) {
    	result->tag = STRING_TAG;
    	result->val.str = AllocString(1);
	result->val.str[0] = '\0';
	return True;
    }

    /* Allocate a new string to hold the data */
    result->tag = STRING_TAG;
    result->val.str = AllocString(length + 1);

    /* Copy the clipboard contents to the string */
    if (XmClipboardRetrieve(TheDisplay, XtWindow(window->shell), "STRING",
    	    result->val.str, length, &retLength, &id) != ClipboardSuccess)
    	retLength = 0;
    result->val.str[retLength] = '\0';

    return True;
}


/*
** Built-in macro subroutine for reading the contents of a text file into
** a string.  On success, returns 1 in $readStatus, and the contents of the
** file as a string in the subroutine return value.  On failure, returns
** the empty string "" and an 0 $readStatus.
*/
static int readFileMS(WindowInfo *window, DataValue *argList, int nArgs,
    	DataValue *result, char **errMsg)
{
    char stringStorage[25], *name;
    struct stat statbuf;
    FILE *fp;
    int readLen;
    
    /* Validate arguments and convert to int */
    if (nArgs != 1)
    	return wrongNArgsErr(errMsg);
    if (!readStringArg(argList[0], &name, stringStorage, errMsg))
    	return False;
    
    /* Read the whole file into an allocated string */
    if ((fp = fopen(name, "r")) == NULL)
    	goto errorNoClose;
    if (fstat(fileno(fp), &statbuf) != 0)
    	goto error;
    result->tag = STRING_TAG;
    result->val.str = AllocString(statbuf.st_size+1);
    readLen = fread(result->val.str, sizeof(char), statbuf.st_size, fp);
    if (ferror(fp))
	goto error;
    result->val.str[readLen] = '\0';
    fclose(fp);
    
    /* Return the results */
    ReturnGlobals[READ_STATUS]->value.tag = INT_TAG;
    ReturnGlobals[READ_STATUS]->value.val.n = True;
    return True;

error:
    fclose(fp);

errorNoClose:
    ReturnGlobals[READ_STATUS]->value.tag = INT_TAG;
    ReturnGlobals[READ_STATUS]->value.val.n = False;
    result->tag = STRING_TAG;
    result->val.str = AllocString(1);
    result->val.str[0] = '\0';
    return True;
}

/*
** Built-in macro subroutines for writing or appending a string (parameter $1)
** to a file named in parameter $2. Returns 1 on successful write, or 0 if
** unsuccessful.
*/
static int writeFileMS(WindowInfo *window, DataValue *argList, int nArgs,
    	DataValue *result, char **errMsg)
{
    return writeOrAppendFile(False, window, argList, nArgs, result, errMsg);
}
static int appendFileMS(WindowInfo *window, DataValue *argList, int nArgs,
    	DataValue *result, char **errMsg)
{
    return writeOrAppendFile(True, window, argList, nArgs, result, errMsg);
}
static int writeOrAppendFile(int append, WindowInfo *window,
    	DataValue *argList, int nArgs, DataValue *result, char **errMsg)
{
    char stringStorage[2][25], *name, *string;
    FILE *fp;
    
    /* Validate argument */
    if (nArgs != 2)
    	return wrongNArgsErr(errMsg);
    if (!readStringArg(argList[0], &string, stringStorage[1], errMsg))
    	return False;
    if (!readStringArg(argList[1], &name, stringStorage[0], errMsg))
    	return False;
    
    /* open the file */
    if ((fp = fopen(name, append ? "a" : "w")) == NULL) {
	result->tag = INT_TAG;
	result->val.n = False;
	return True;
    }
    
    /* write the string to the file */
    fwrite(string, sizeof(char), strlen(string), fp);
    if (ferror(fp)) {
	fclose(fp);
	result->tag = INT_TAG;
	result->val.n = False;
	return True;
    }
    fclose(fp);
    
    /* return the status */
    result->tag = INT_TAG;
    result->val.n = True;
    return True;
}

/*
** Built-in macro subroutine for searching silently in a window without
** dialogs, beeps, or changes to the selection.  Arguments are: $1: string to
** search for, $2: starting position. Optional arguments may include the
** strings: "wrap" to make the search wrap around the beginning or end of the
** string, "backward" or "forward" to change the search direction ("forward" is
** the default), "literal", "case" or "regex" to change the search type
** (default is "literal").
**
** Returns the starting position of the match, or -1 if nothing matched.
** also returns the ending position of the match in $searchEndPos
*/
static int searchMS(WindowInfo *window, DataValue *argList, int nArgs,
    	DataValue *result, char **errMsg)
{
    DataValue newArgList[9];
    int retVal;
    
    /* Use the search string routine, by adding the buffer contents as
       the string argument */
    if (nArgs > 8)
    	return wrongNArgsErr(errMsg);
    newArgList[0].tag = STRING_TAG;
    newArgList[0].val.str = BufGetAll(window->buffer);
    memcpy(&newArgList[1], argList, nArgs * sizeof(DataValue));
    retVal = searchStringMS(window, newArgList, nArgs+1, result, errMsg);
    XtFree(newArgList[0].val.str);
    return retVal;
}

/*
** Built-in macro subroutine for searching a string.  Arguments are $1:
** string to search in, $2: string to search for, $3: starting position.
** Optional arguments may include the strings: "wrap" to make the search
** wrap around the beginning or end of the string, "backward" or "forward"
** to change the search direction ("forward" is the default), "literal",
** "case" or "regex" to change the search type (default is "literal").
**
** Returns the starting position of the match, or -1 if nothing matched.
** also returns the ending position of the match in $searchEndPos
*/
static int searchStringMS(WindowInfo *window, DataValue *argList, int nArgs,
    	DataValue *result, char **errMsg)
{
    int beginPos, wrap, direction, found, foundStart, foundEnd, type;
    char stringStorage[2][25], *string, *searchStr;
    
    /* Validate arguments and convert to proper types */
    if (nArgs < 3)
    	return tooFewArgsErr(errMsg);
    if (!readStringArg(argList[0], &string, stringStorage[0], errMsg))
    	return False;
    if (!readStringArg(argList[1], &searchStr, stringStorage[1], errMsg))
    	return False;
    if (!readIntArg(argList[2], &beginPos, errMsg))
    	return False;
    if (!readSearchArgs(&argList[3], nArgs-3, &direction, &type, &wrap, errMsg))
    	return False;
    
    /* Do the search */
    found = SearchString(string, searchStr, direction, type, wrap, beginPos,
	    &foundStart, &foundEnd, NULL, GetWindowDelimiters(window));
    
    /* Return the results */
    ReturnGlobals[SEARCH_END]->value.tag = INT_TAG;
    ReturnGlobals[SEARCH_END]->value.val.n = found ? foundEnd : 0;
    result->tag = INT_TAG;
    result->val.n = found ? foundStart : -1;
    return True;
}

/*
** Built-in macro subroutine for replacing all occurences of a search string in
** a string with a replacement string.  Arguments are $1: string to search in,
** $2: string to search for, $3: replacement string. Argument $4 is an optional
** search type: one of "literal", "case" or "regex" (default is "literal").
**
** Returns a new string with all of the replacements done, or an empty string
** ("") if no occurences were found.
*/
static int replaceInStringMS(WindowInfo *window, DataValue *argList, int nArgs,
    	DataValue *result, char **errMsg)
{
    char stringStorage[3][25], *string, *searchStr, *replaceStr;
    char *argStr, *replacedStr;
    int searchType = SEARCH_LITERAL, copyStart, copyEnd;
    int replacedLen, replaceEnd;
    
    /* Validate arguments and convert to proper types */
    if (nArgs < 3 || nArgs > 5)
    	return wrongNArgsErr(errMsg);
    if (!readStringArg(argList[0], &string, stringStorage[0], errMsg))
    	return False;
    if (!readStringArg(argList[1], &searchStr, stringStorage[1], errMsg))
    	return False;
    if (!readStringArg(argList[2], &replaceStr, stringStorage[2], errMsg))
    	return False;
    if (nArgs == 4) {
	if (!readStringArg(argList[3], &argStr, stringStorage[2], errMsg))
    	    return False;
    	if (!strcmp(argStr, "literal"))
    	    searchType = SEARCH_LITERAL;
    	else if (!strcmp(argStr, "case"))
    	    searchType = SEARCH_CASE_SENSE;
    	else if (!strcmp(argStr, "regex"))
    	    searchType = SEARCH_REGEX;
    	else {
    	    *errMsg = "unrecognized argument to %s";
    	    return False;
    	}
    }
    
    /* Do the replace */
    replacedStr = ReplaceAllInString(string, searchStr, replaceStr, searchType,
	    &copyStart, &copyEnd, &replacedLen, GetWindowDelimiters(window));
    
    /* Return the results */
    result->tag = STRING_TAG;
    if (replacedStr == NULL) {
    	result->val.str = AllocString(1);
	result->val.str[0] = '\0';
    } else {
	replaceEnd = copyStart + replacedLen;
	result->val.str = AllocString(replaceEnd + strlen(&string[copyEnd])+1);
	strncpy(result->val.str, string, copyStart);
	strcpy(&result->val.str[copyStart], replacedStr);
	strcpy(&result->val.str[replaceEnd], &string[copyEnd]);
	XtFree(replacedStr);
    }
    return True;
}

static int readSearchArgs(DataValue *argList, int nArgs, int *searchDirection,
	int *searchType, int *wrap, char **errMsg)
{
    int i;
    char *argStr, stringStorage[9][25];
    
    *wrap = False;
    *searchDirection = SEARCH_FORWARD;
    *searchType = SEARCH_LITERAL;
    for (i=0; i<nArgs; i++) {
    	if (!readStringArg(argList[i], &argStr, stringStorage[i], errMsg))
    	    return False;
    	else if (!strcmp(argStr, "wrap"))
    	    *wrap = True;
    	else if (!strcmp(argStr, "nowrap"))
    	    *wrap = False;
    	else if (!strcmp(argStr, "backward"))
    	    *searchDirection = SEARCH_BACKWARD;
    	else if (!strcmp(argStr, "forward"))
    	    *searchDirection = SEARCH_FORWARD;
    	else if (!strcmp(argStr, "literal"))
    	    *searchType = SEARCH_LITERAL;
    	else if (!strcmp(argStr, "case"))
    	    *searchType = SEARCH_CASE_SENSE;
    	else if (!strcmp(argStr, "regex"))
    	    *searchType = SEARCH_REGEX;
    	else {
    	    *errMsg = "unrecognized argument to %s";
    	    return False;
    	}
    }
    return True;
}

static int setCursorPosMS(WindowInfo *window, DataValue *argList, int nArgs,
    	DataValue *result, char **errMsg)
{
    int pos;

    /* Get argument and convert to int */
    if (nArgs != 1)
    	return wrongNArgsErr(errMsg);
    if (!readIntArg(argList[0], &pos, errMsg))
    	return False;
    
    /* Set the position */
    TextSetCursorPos(window->lastFocus, pos);
    result->tag = NO_TAG;
    return True;
}

static int selectMS(WindowInfo *window, DataValue *argList, int nArgs,
    	DataValue *result, char **errMsg)
{
    int start, end, startTmp;

    /* Get arguments and convert to int */
    if (nArgs != 2)
    	return wrongNArgsErr(errMsg);
    if (!readIntArg(argList[0], &start, errMsg))
    	return False;
    if (!readIntArg(argList[1], &end, errMsg))
    	return False;
    
    /* Verify integrity of arguments */
    if (start > end) {
    	startTmp = start;
	start = end;
	end = startTmp;
    }
    if (start < 0) start = 0;
    if (start > window->buffer->length) start = window->buffer->length;
    if (end < 0) end = 0;
    if (end > window->buffer->length) end = window->buffer->length;
    
    /* Make the selection */
    BufSelect(window->buffer, start, end);
    result->tag = NO_TAG;
    return True;
}

static int selectRectangleMS(WindowInfo *window, DataValue *argList, int nArgs,
    	DataValue *result, char **errMsg)
{
    int start, end, left, right;

    /* Get arguments and convert to int */
    if (nArgs != 4)
    	return wrongNArgsErr(errMsg);
    if (!readIntArg(argList[0], &start, errMsg))
    	return False;
    if (!readIntArg(argList[1], &end, errMsg))
    	return False;
    if (!readIntArg(argList[2], &left, errMsg))
    	return False;
    if (!readIntArg(argList[3], &right, errMsg))
    	return False;
    
    /* Make the selection */
    BufRectSelect(window->buffer, start, end, left, right);
    result->tag = NO_TAG;
    return True;
}

/*
** Macro subroutine to ring the bell
*/
static int beepMS(WindowInfo *window, DataValue *argList, int nArgs,
    	DataValue *result, char **errMsg)
{
    if (nArgs != 0)
    	return wrongNArgsErr(errMsg);
    XBell(XtDisplay(window->shell), 0);
    result->tag = NO_TAG;
    return True;
}

static int tPrintMS(WindowInfo *window, DataValue *argList, int nArgs,
    	DataValue *result, char **errMsg)
{
    char stringStorage[25], *string;
    int i;
    
    if (nArgs == 0)
    	return tooFewArgsErr(errMsg);
    for (i=0; i<nArgs; i++) {
	if (!readStringArg(argList[i], &string, stringStorage, errMsg))
	    return False;
	printf("%s%s", string, i==nArgs-1 ? "" : " ");
    }
    result->tag = NO_TAG;
    return True;
}

/*
** Built-in macro subroutine for getting the value of an environment variable
*/
static int getenvMS(WindowInfo *window, DataValue *argList, int nArgs,
    	DataValue *result, char **errMsg)
{
    char *value;

    /* Get name of variable to get */
    if (nArgs != 1)
      	return wrongNArgsErr(errMsg);
    if (argList[0].tag != STRING_TAG) {
	*errMsg = "argument to %s must be a string";
	return False;
    }
    value = getenv(argList[0].val.str);
    if (value == NULL)
	value = "";
	
    /* Return the text as an allocated string */
    result->tag = STRING_TAG;
    result->val.str = AllocString(strlen(value) + 1);
    strcpy(result->val.str, value);
    return True;
}

static int shellCmdMS(WindowInfo *window, DataValue *argList, int nArgs,
    	DataValue *result, char **errMsg)
{
    char stringStorage[2][25], *cmdString, *inputString;

    if (nArgs != 2)
    	return wrongNArgsErr(errMsg);
    if (!readStringArg(argList[0], &cmdString, stringStorage[0], errMsg))
    	return False;
    if (!readStringArg(argList[1], &inputString, stringStorage[1], errMsg))
    	return False;
    
    /* Shell command execution requires that the macro be suspended, so
       this subroutine can't be run if macro execution can't be interrupted */
    if (MacroRunWindow()->macroCmdData == NULL) {
      *errMsg = "%s can't be called from non-suspendable context";
       return False;
    }
	
#ifdef VMS
    *errMsg = "Shell commands not supported under VMS";
    return False;
#else
    ShellCmdToMacroString(window, cmdString, inputString);
    result->tag = INT_TAG;
    result->val.n = 0;
    return True;
#endif /*VMS*/
}

/*
** Method used by ShellCmdToMacroString (called by shellCmdMS), for returning
** macro string and exit status after the execution of a shell command is
** complete.  (Sorry about the poor modularity here, it's just not worth
** teaching other modules about macro return globals, since other than this,
** they're not used outside of macro.c)
*/
void ReturnShellCommandOutput(WindowInfo *window, char *outText, int status)
{
    DataValue retVal;
    macroCmdInfo *cmdData = window->macroCmdData;
    
    if (cmdData == NULL)
    	return;
    retVal.tag = STRING_TAG;
    retVal.val.str = AllocString(strlen(outText)+1);
    strcpy(retVal.val.str, outText);
    ModifyReturnedValue(cmdData->context, retVal);
    ReturnGlobals[SHELL_CMD_STATUS]->value.tag = INT_TAG;
    ReturnGlobals[SHELL_CMD_STATUS]->value.val.n = status;
}

static int dialogMS(WindowInfo *window, DataValue *argList, int nArgs,
    	DataValue *result, char **errMsg)
{
    macroCmdInfo *cmdData;
    char stringStorage[9][25], *btnLabels[8], *message;
    Arg al[20];
    int ac;
    Widget dialog, btn;
    int i, nBtns;
    XmString s1, s2;
    
    /* Ignore the focused window passed as the function argument and put
       the dialog up over the window which is executing the macro */
    window = MacroRunWindow();
    cmdData = window->macroCmdData;
    
    /* Dialogs require macro to be suspended and interleaved with other macros.
       This subroutine can't be run if macro execution can't be interrupted */
    if (!cmdData) {
      *errMsg = "%s can't be called from non-suspendable context";
       return False;
    }

    /* Read and check the arguments.  The first being the dialog message,
       and the rest being the button labels */
    if (nArgs == 0) {
    	*errMsg = "%s subroutine called with no arguments";
    	return False;
    }
    if (!readStringArg(argList[0], &message, stringStorage[0], errMsg))
	return False;
    for (i=1; i<nArgs; i++)
	if (!readStringArg(argList[i], &btnLabels[i-1], stringStorage[i],
	    	errMsg))
	    return False;
    if (nArgs == 1) {
    	btnLabels[0] = "Dismiss";
    	nBtns = 1;
    } else
    	nBtns = nArgs - 1;

    /* Create the message box dialog widget and its dialog shell parent */
    ac = 0;
    XtSetArg(al[ac], XmNtitle, " "); ac++;
    XtSetArg(al[ac], XmNmessageString, s1=MKSTRING(message)); ac++;
    XtSetArg(al[ac], XmNokLabelString, s2=XmStringCreateSimple(btnLabels[0]));
    	    ac++;
    dialog = CreateMessageDialog(window->shell, "macroDialog", al, ac);
    XmStringFree(s1);
    XmStringFree(s2);
    AddMotifCloseCallback(XtParent(dialog), dialogCloseCB, window);
    XtAddCallback(dialog, XmNokCallback, dialogBtnCB, window);
    XtVaSetValues(XmMessageBoxGetChild(dialog, XmDIALOG_OK_BUTTON),
    	    XmNuserData, (XtPointer)1, NULL);
    cmdData->dialog = dialog;

    /* Unmanage default buttons, except for "OK" */
    XtUnmanageChild(XmMessageBoxGetChild(dialog, XmDIALOG_CANCEL_BUTTON));
    XtUnmanageChild(XmMessageBoxGetChild(dialog, XmDIALOG_HELP_BUTTON));
    
    /* Make callback for the unmanaged cancel button (which can
       still get executed via the esc key) activate close box action */
    XtAddCallback(XmMessageBoxGetChild(dialog, XmDIALOG_CANCEL_BUTTON),
	    XmNactivateCallback, dialogCloseCB, window);

    /* Add user specified buttons (1st is already done) */
    for (i=1; i<nBtns; i++) {
    	btn = XtVaCreateManagedWidget("mdBtn", xmPushButtonWidgetClass, dialog,
    	    	XmNlabelString, s1=XmStringCreateSimple(btnLabels[i]),
    	    	XmNuserData, (XtPointer)(i+1), NULL);
    	XtAddCallback(btn, XmNactivateCallback, dialogBtnCB, window);
    	XmStringFree(s1);
    }
    
    /* Put up the dialog */
    ManageDialogCenteredOnPointer(dialog);
    
    /* Stop macro execution until the dialog is complete */
    PreemptMacro();
    
    /* Return placeholder result.  Value will be changed by button callback */
    result->tag = INT_TAG;
    result->val.n = 0;
    return True;
}

static void dialogBtnCB(Widget w, XtPointer clientData, XtPointer callData)
{
    WindowInfo *window = (WindowInfo *)clientData;
    macroCmdInfo *cmdData = window->macroCmdData;
    XtPointer userData;
    DataValue retVal;
    
    /* Return the index of the button which was pressed (stored in the userData
       field of the button widget).  The 1st button, being a gadget, is not
       returned in w. */
    if (cmdData == NULL)
    	return; /* shouldn't happen */
    if (XtClass(w) == xmPushButtonWidgetClass) {
	XtVaGetValues(w, XmNuserData, &userData, NULL);
	retVal.val.n = (int)userData;
    } else
    	retVal.val.n = 1;
    retVal.tag = INT_TAG;
    ModifyReturnedValue(cmdData->context, retVal);

    /* Pop down the dialog */
    XtDestroyWidget(XtParent(cmdData->dialog));
    cmdData->dialog = NULL;

    /* Continue preempted macro execution */
    ResumeMacroExecution(window);
}

static void dialogCloseCB(Widget w, XtPointer clientData, XtPointer callData)
{
    WindowInfo *window = (WindowInfo *)clientData;
    macroCmdInfo *cmdData = window->macroCmdData;
    DataValue retVal;
    
    /* Return 0 to show that the dialog was closed via the window close box */
    retVal.val.n = 0;
    retVal.tag = INT_TAG;
    ModifyReturnedValue(cmdData->context, retVal);

    /* Pop down the dialog */
    XtDestroyWidget(XtParent(cmdData->dialog));
    cmdData->dialog = NULL;

    /* Continue preempted macro execution */
    ResumeMacroExecution(window);
}

static int stringDialogMS(WindowInfo *window, DataValue *argList, int nArgs,
    	DataValue *result, char **errMsg)
{
    macroCmdInfo *cmdData;
    char stringStorage[9][25], *btnLabels[8], *message;
    Widget dialog, btn;
    int i, nBtns;
    XmString s1, s2;
    Arg al[20];
    int ac;
    
    /* Ignore the focused window passed as the function argument and put
       the dialog up over the window which is executing the macro */
    window = MacroRunWindow();
    cmdData = window->macroCmdData;
    
    /* Dialogs require macro to be suspended and interleaved with other macros.
       This subroutine can't be run if macro execution can't be interrupted */
    if (!cmdData) {
      *errMsg = "%s can't be called from non-suspendable context";
       return False;
    }

    /* Read and check the arguments.  The first being the dialog message,
       and the rest being the button labels */
    if (nArgs == 0) {
    	*errMsg = "%s subroutine called with no arguments";
    	return False;
    }
    if (!readStringArg(argList[0], &message, stringStorage[0], errMsg))
	return False;
    for (i=1; i<nArgs; i++)
	if (!readStringArg(argList[i], &btnLabels[i-1], stringStorage[i],
	    	errMsg))
	    return False;
    if (nArgs == 1) {
    	btnLabels[0] = "Dismiss";
    	nBtns = 1;
    } else
    	nBtns = nArgs - 1;

    /* Create the selection box dialog widget and its dialog shell parent */
    ac = 0;
    XtSetArg(al[ac], XmNtitle, " "); ac++;
    XtSetArg(al[ac], XmNselectionLabelString, s1=MKSTRING(message)); ac++;
    XtSetArg(al[ac], XmNokLabelString, s2=XmStringCreateSimple(btnLabels[0]));
    	    ac++;
    dialog = CreatePromptDialog(window->shell, "macroStringDialog", al, ac);
    XmStringFree(s1);
    XmStringFree(s2);
    AddMotifCloseCallback(XtParent(dialog), stringDialogCloseCB, window);
    XtAddCallback(dialog, XmNokCallback, stringDialogBtnCB, window);
    XtVaSetValues(XmSelectionBoxGetChild(dialog, XmDIALOG_OK_BUTTON),
    	    XmNuserData, (XtPointer)1, NULL);
    cmdData->dialog = dialog;

    /* Unmanage unneded widgets */
    XtUnmanageChild(XmSelectionBoxGetChild(dialog, XmDIALOG_CANCEL_BUTTON));
    XtUnmanageChild(XmSelectionBoxGetChild(dialog, XmDIALOG_HELP_BUTTON));
    
    /* Make callback for the unmanaged cancel button (which can
       still get executed via the esc key) activate close box action */
    XtAddCallback(XmSelectionBoxGetChild(dialog, XmDIALOG_CANCEL_BUTTON),
	    XmNactivateCallback, stringDialogCloseCB, window);

    /* Add user specified buttons (1st is already done).  Selection box
       requires a place-holder widget to be added before buttons can be
       added, that's what the separator below is for */
    XtVaCreateWidget("x", xmSeparatorWidgetClass, dialog, NULL);
    for (i=1; i<nBtns; i++) {
    	btn = XtVaCreateManagedWidget("mdBtn", xmPushButtonWidgetClass, dialog,
    	    	XmNlabelString, s1=XmStringCreateSimple(btnLabels[i]),
    	    	XmNuserData, (XtPointer)(i+1), NULL);
    	XtAddCallback(btn, XmNactivateCallback, stringDialogBtnCB, window);
    	XmStringFree(s1);
    }
    
    /* Put up the dialog */
    ManageDialogCenteredOnPointer(dialog);
    
    /* Stop macro execution until the dialog is complete */
    PreemptMacro();
    
    /* Return placeholder result.  Value will be changed by button callback */
    result->tag = INT_TAG;
    result->val.n = 0;
    return True;
}

static void stringDialogBtnCB(Widget w, XtPointer clientData,
    	XtPointer callData)
{
    WindowInfo *window = (WindowInfo *)clientData;
    macroCmdInfo *cmdData = window->macroCmdData;
    XtPointer userData;
    DataValue retVal;
    char *text;
    int btnNum;

    /* shouldn't happen, but would crash if it did */
    if (cmdData == NULL)
    	return; 

    /* Return the string entered in the selection text area */
    text = XmTextGetString(XmSelectionBoxGetChild(cmdData->dialog,
    	    XmDIALOG_TEXT));
    retVal.tag = STRING_TAG;
    retVal.val.str = AllocString(strlen(text)+1);
    strcpy(retVal.val.str, text);
    XtFree(text);
    ModifyReturnedValue(cmdData->context, retVal);
    
    /* Find the index of the button which was pressed (stored in the userData
       field of the button widget).  The 1st button, being a gadget, is not
       returned in w. */
    if (XtClass(w) == xmPushButtonWidgetClass) {
	XtVaGetValues(w, XmNuserData, &userData, NULL);
	btnNum = (int)userData;
    } else
    	btnNum = 1;
    
    /* Return the button number in the global variable $string_dialog_button */
    ReturnGlobals[STRING_DIALOG_BUTTON]->value.tag = INT_TAG;
    ReturnGlobals[STRING_DIALOG_BUTTON]->value.val.n = btnNum;

    /* Pop down the dialog */
    XtDestroyWidget(XtParent(cmdData->dialog));
    cmdData->dialog = NULL;

    /* Continue preempted macro execution */
    ResumeMacroExecution(window);
}

static void stringDialogCloseCB(Widget w, XtPointer clientData,
    	XtPointer callData)
{
    WindowInfo *window = (WindowInfo *)clientData;
    macroCmdInfo *cmdData = window->macroCmdData;
    DataValue retVal;

    /* shouldn't happen, but would crash if it did */
    if (cmdData == NULL)
    	return; 

    /* Return an empty string */
    retVal.tag = STRING_TAG;
    retVal.val.str = AllocString(1);
    retVal.val.str[0] = '\0';
    ModifyReturnedValue(cmdData->context, retVal);
    
    /* Return button number 0 in the global variable $string_dialog_button */
    ReturnGlobals[STRING_DIALOG_BUTTON]->value.tag = INT_TAG;
    ReturnGlobals[STRING_DIALOG_BUTTON]->value.val.n = 0;

    /* Pop down the dialog */
    XtDestroyWidget(XtParent(cmdData->dialog));
    cmdData->dialog = NULL;

    /* Continue preempted macro execution */
    ResumeMacroExecution(window);
}

/* T Balinski */
static int listDialogMS(WindowInfo *window, DataValue *argList, int nArgs,
      DataValue *result, char **errMsg)
{
    macroCmdInfo *cmdData;
    char stringStorage[9][25], *btnLabels[8], *message, *text;
    Widget dialog, btn;
    int i, nBtns;
    XmString s1, s2;
    long nlines = 0;
    char *p, *old_p, **text_lines, *tmp;
    int tmp_len;
    int n, is_last;
    XmString *test_strings;
    int tabDist;
    Arg al[20];
    int ac;

  
    /* Ignore the focused window passed as the function argument and put
       the dialog up over the window which is executing the macro */
    window = MacroRunWindow();
    cmdData = window->macroCmdData;
    
    /* Dialogs require macro to be suspended and interleaved with other macros.
       This subroutine can't be run if macro execution can't be interrupted */
    if (!cmdData) {
      *errMsg = "%s can't be called from non-suspendable context";
       return False;
    }

    /* Read and check the arguments.  The first being the dialog message,
       and the rest being the button labels */
    if (nArgs < 2) {
      *errMsg = "%s subroutine called with no message, string or arguments";
      return False;
    }

    if (!readStringArg(argList[0], &message, stringStorage[0], errMsg))
      return False;

    if (!readStringArg(argList[1], &text, stringStorage[0], errMsg))
      return False;

    if (!text || text[0] == '\0') {
      *errMsg = "%s subroutine called with empty list data";
      return False;
    }

    for (i=2; i<nArgs; i++)
      if (!readStringArg(argList[i], &btnLabels[i-2], stringStorage[i],
              errMsg))
          return False;
    if (nArgs == 2) {
      btnLabels[0] = "Dismiss";
      nBtns = 1;
    } else
      nBtns = nArgs - 2;

    /* count the lines in the text - add one for unterminated last line */
    nlines = 1;
    for (p = text; *p; p++)
      if (*p == '\n')
          nlines++;

    /* now set up arrays of pointers to lines */
    /*   test_strings to hold the display strings (tab expanded) */
    /*   text_lines to hold the original text lines (without the '\n's) */
    test_strings = (XmString *) XtMalloc(sizeof(XmString) * nlines);
    text_lines = (char **)XtMalloc(sizeof(char *) * (nlines + 1));
    for (n = 0; n < nlines; n++) {
      test_strings[n] = (XmString)0;
      text_lines[n] = (char *)0;
    }
    text_lines[n] = (char *)0;        /* make sure this is a null-terminated table */

    /* pick up the tabDist value */
    tabDist = window->buffer->tabDist;

    /* load the table */
    n = 0;
    is_last = 0;
    p = old_p = text;
    tmp_len = 0;      /* current allocated size of temporary buffer tmp */
    tmp = malloc(1);  /* temporary buffer into which to expand tabs */
    do {
      is_last = (*p == '\0');
      if (*p == '\n' || is_last) {
          *p = '\0';
          if (strlen(old_p) > 0) {    /* only include non-empty lines */
              char *s, *t;
              int l;

              /* save the actual text line in text_lines[n] */
              text_lines[n] = (char *)XtMalloc(strlen(old_p) + 1);
              strcpy(text_lines[n], old_p);

              /* work out the tabs expanded length */
              for (s = old_p, l = 0; *s; s++)
                  l += (*s == '\t') ? tabDist - (l % tabDist) : 1;

              /* verify tmp is big enough then tab-expand old_p into tmp */
              if (l > tmp_len)
                  tmp = realloc(tmp, (tmp_len = l) + 1);
              for (s = old_p, t = tmp, l = 0; *s; s++) {
                  if (*s == '\t') {
                      for (i = tabDist - (l % tabDist); i--; l++)
                          *t++ = ' ';
                  }
                  else {
                    *t++ = *s;
                    l++;
                  }
              }
              *t = '\0';
              /* that's it: tmp is the tab-expanded version of old_p */
              test_strings[n] = MKSTRING(tmp);
              n++;
          }
          old_p = p + 1;
          if (!is_last)
              *p = '\n';              /* put back our newline */
      }
      p++;
    } while (!is_last);

    free(tmp);                /* don't need this anymore */
    nlines = n;
    if (nlines == 0) {
      test_strings[0] = MKSTRING("");
      nlines = 1;
    }

    /* Create the selection box dialog widget and its dialog shell parent */
    ac = 0;
    XtSetArg(al[ac], XmNtitle, " "); ac++;
    XtSetArg(al[ac], XmNlistLabelString, s1=MKSTRING(message)); ac++;
    XtSetArg(al[ac], XmNlistItems, test_strings); ac++;
    XtSetArg(al[ac], XmNlistItemCount, nlines); ac++;
    XtSetArg(al[ac], XmNlistVisibleItemCount, (nlines > 10) ? 10 : nlines); ac++;
    XtSetArg(al[ac], XmNokLabelString, s2=XmStringCreateSimple(btnLabels[0])); ac++;
    dialog = CreateSelectionDialog(window->shell, "macroListDialog", al, ac);
    AddMotifCloseCallback(XtParent(dialog), listDialogCloseCB, window);
    XtAddCallback(dialog, XmNokCallback, listDialogBtnCB, window);
    XtVaSetValues(XmSelectionBoxGetChild(dialog, XmDIALOG_OK_BUTTON),
          XmNuserData, (XtPointer)1, NULL);
    XmStringFree(s1);
    XmStringFree(s2);
    cmdData->dialog = dialog;

    /* forget lines stored in list */
    while (n--)
      XmStringFree(test_strings[n]);
    XtFree((char *)test_strings);

    /* modify the list */
    XtVaSetValues(XmSelectionBoxGetChild(dialog, XmDIALOG_LIST),
                XmNselectionPolicy, XmSINGLE_SELECT,
                XmNuserData, (XtPointer)text_lines, NULL);

    /* Unmanage unneeded widgets */
    XtUnmanageChild(XmSelectionBoxGetChild(dialog, XmDIALOG_APPLY_BUTTON));
    XtUnmanageChild(XmSelectionBoxGetChild(dialog, XmDIALOG_CANCEL_BUTTON));
    XtUnmanageChild(XmSelectionBoxGetChild(dialog, XmDIALOG_HELP_BUTTON));
    XtUnmanageChild(XmSelectionBoxGetChild(dialog, XmDIALOG_TEXT));
    XtUnmanageChild(XmSelectionBoxGetChild(dialog, XmDIALOG_SELECTION_LABEL));
    
    /* Make callback for the unmanaged cancel button (which can
       still get executed via the esc key) activate close box action */
    XtAddCallback(XmSelectionBoxGetChild(dialog, XmDIALOG_CANCEL_BUTTON),
	    XmNactivateCallback, listDialogCloseCB, window);

    /* Add user specified buttons (1st is already done).  Selection box
       requires a place-holder widget to be added before buttons can be
       added, that's what the separator below is for */
    XtVaCreateWidget("x", xmSeparatorWidgetClass, dialog, NULL);
    for (i=1; i<nBtns; i++) {
      btn = XtVaCreateManagedWidget("mdBtn", xmPushButtonWidgetClass, dialog,
              XmNlabelString, s1=XmStringCreateSimple(btnLabels[i]),
              XmNuserData, (XtPointer)(i+1), NULL);
      XtAddCallback(btn, XmNactivateCallback, listDialogBtnCB, window);
      XmStringFree(s1);
    }
    
    /* Put up the dialog */
    ManageDialogCenteredOnPointer(dialog);
    
    /* Stop macro execution until the dialog is complete */
    PreemptMacro();
    
    /* Return placeholder result.  Value will be changed by button callback */
    result->tag = INT_TAG;
    result->val.n = 0;
    return True;
}

static void listDialogBtnCB(Widget w, XtPointer clientData,
      XtPointer callData)
{
    WindowInfo *window = (WindowInfo *)clientData;
    macroCmdInfo *cmdData = window->macroCmdData;
    XtPointer userData;
    DataValue retVal;
    char *text;
    char **text_lines;
    int btnNum;
    int n_sel, *seltable, sel_index;
    Widget theList;

    /* shouldn't happen, but would crash if it did */
    if (cmdData == NULL)
      return; 

    theList = XmSelectionBoxGetChild(cmdData->dialog, XmDIALOG_LIST);
    /* Return the string selected in the selection list area */
    XtVaGetValues(theList, XmNuserData, &text_lines, NULL);
    if (!XmListGetSelectedPos(theList, &seltable, &n_sel)) {
      n_sel = 0;
    }
    else {
      sel_index = seltable[0] - 1;
      XtFree((XtPointer)seltable);
    }

    if (!n_sel) {
      text = AllocString(1);
      text[0] = '\0';
    }
    else {
      text = AllocString(strlen((char *)text_lines[sel_index]) + 1);
      strcpy(text, text_lines[sel_index]);
    }

    /* don't need text_lines anymore: free it */
    for (sel_index = 0; text_lines[sel_index]; sel_index++)
      XtFree((XtPointer)text_lines[sel_index]);
    XtFree((XtPointer)text_lines);

    retVal.tag = STRING_TAG;
    retVal.val.str = text;
    ModifyReturnedValue(cmdData->context, retVal);
    
    /* Find the index of the button which was pressed (stored in the userData
       field of the button widget).  The 1st button, being a gadget, is not
       returned in w. */
    if (XtClass(w) == xmPushButtonWidgetClass) {
      XtVaGetValues(w, XmNuserData, &userData, NULL);
      btnNum = (int)userData;
    } else
      btnNum = 1;
    
    /* Return the button number in the global variable $list_dialog_button */
    ReturnGlobals[LIST_DIALOG_BUTTON]->value.tag = INT_TAG;
    ReturnGlobals[LIST_DIALOG_BUTTON]->value.val.n = btnNum;

    /* Pop down the dialog */
    XtDestroyWidget(XtParent(cmdData->dialog));
    cmdData->dialog = NULL;

    /* Continue preempted macro execution */
    ResumeMacroExecution(window);
}

static void listDialogCloseCB(Widget w, XtPointer clientData,
      XtPointer callData)
{
    WindowInfo *window = (WindowInfo *)clientData;
    macroCmdInfo *cmdData = window->macroCmdData;
    DataValue retVal;
    char **text_lines;
    int sel_index;
    Widget theList;

    /* shouldn't happen, but would crash if it did */
    if (cmdData == NULL)
      return; 

    /* don't need text_lines anymore: retrieve it then free it */
    theList = XmSelectionBoxGetChild(cmdData->dialog, XmDIALOG_LIST);
    XtVaGetValues(theList, XmNuserData, &text_lines, NULL);
    for (sel_index = 0; text_lines[sel_index]; sel_index++)
      XtFree((XtPointer)text_lines[sel_index]);
    XtFree((XtPointer)text_lines);

    /* Return an empty string */
    retVal.tag = STRING_TAG;
    retVal.val.str = AllocString(1);
    retVal.val.str[0] = '\0';
    ModifyReturnedValue(cmdData->context, retVal);
    
    /* Return button number 0 in the global variable $list_dialog_button */
    ReturnGlobals[LIST_DIALOG_BUTTON]->value.tag = INT_TAG;
    ReturnGlobals[LIST_DIALOG_BUTTON]->value.val.n = 0;

    /* Pop down the dialog */
    XtDestroyWidget(XtParent(cmdData->dialog));
    cmdData->dialog = NULL;

    /* Continue preempted macro execution */
    ResumeMacroExecution(window);
}
/* T Balinski End */

static int stringCompareMS(WindowInfo *window, DataValue *argList, int nArgs,
    	DataValue *result, char **errMsg)
{
    char stringStorage[3][25];
    char *leftStr, *rightStr, *argStr;
    int considerCase = True;
    int i;
    int compareResult;
    
    if (nArgs < 2) {
        return(wrongNArgsErr(errMsg));
    }
    if (!readStringArg(argList[0], &leftStr, stringStorage[0], errMsg))
        return False;
    if (!readStringArg(argList[1], &rightStr, stringStorage[1], errMsg))
        return False;
    for (i = 2; i < nArgs; ++i) {
    	if (!readStringArg(argList[i], &argStr, stringStorage[2], errMsg))
    	    return False;
    	else if (!strcmp(argStr, "case"))
    	    considerCase = True;
    	else if (!strcmp(argStr, "nocase"))
    	    considerCase = False;
    	else {
    	    *errMsg = "unrecognized argument to %s";
    	    return False;
    	}
    }
    if (considerCase) {
        compareResult = strcmp(leftStr, rightStr);
        compareResult = (compareResult > 0) ? 1 : ((compareResult < 0) ? -1 : 0);
    }
    else {
        compareResult = strCaseCmp(leftStr, rightStr);
    }
    result->tag = INT_TAG;
    result->val.n = compareResult;
    return True;
}

/*
** This function is intended to split strings into an array of substrings
** Importatnt note: It should always return at least one entry with key 0
** split("", ",") result[0] = ""
** split("1,2", ",") result[0] = "1" result[1] = "2"
** split("1,2,", ",") result[0] = "1" result[1] = "2" result[2] = ""
** 
** This behavior is specifically important when used to break up
** array sub-scripts
*/

static int splitMS(WindowInfo *window, DataValue *argList, int nArgs,
    	DataValue *result, char **errMsg)
{
    char stringStorage[3][25];
    char *sourceStr, *splitStr, *typeSplitStr;
    int searchType, beginPos, foundStart, foundEnd, strLength;
    int found, elementEnd, indexNum;
    char indexStr[28], *allocIndexStr;
    DataValue element;
    int elementLen;
    
    if (nArgs < 2) {
        return(wrongNArgsErr(errMsg));
    }
    if (!readStringArg(argList[0], &sourceStr, stringStorage[0], errMsg)) {
        *errMsg = "first argument must be a string: %s";
        return(False);
    }
    if (!readStringArg(argList[1], &splitStr, stringStorage[1], errMsg)) {
        splitStr = NULL;
    }
    else {
        if (splitStr[0] == 0) {
            splitStr = NULL;
        }
    }
    if (splitStr == NULL) {
        *errMsg = "second argument must be a non-empty string: %s";
        return(False);
    }
    if (nArgs > 2 && readStringArg(argList[2], &typeSplitStr, stringStorage[2], errMsg)) {
    	if (!strcmp(typeSplitStr, "literal")) {
    	    searchType = SEARCH_LITERAL;
        }
    	else if (!strcmp(typeSplitStr, "case")) {
            searchType = SEARCH_CASE_SENSE;
        }
    	else if (!strcmp(typeSplitStr, "regex")) {
            searchType = SEARCH_REGEX;
        }
        else {
            *errMsg = "unrecognized argument to %s";
            return(False);
        }
    }
    else {
    	searchType = SEARCH_LITERAL;
    }
    
    result->tag = ARRAY_TAG;
    result->val.arrayPtr = NULL;

    beginPos = 0;
    indexNum = 0;
    strLength = strlen(sourceStr);
    found = 1;
    while (found && beginPos < strLength) {
        sprintf(indexStr, "%d", indexNum);
        allocIndexStr = AllocString(strlen(indexStr) + 1);
        if (!allocIndexStr) {
            *errMsg = "array element failed to allocate key: %s";
            return(False);
        }
        strcpy(allocIndexStr, indexStr);
        found = SearchString(sourceStr, splitStr, SEARCH_FORWARD, searchType,
            False, beginPos, &foundStart, &foundEnd,
	        NULL, GetWindowDelimiters(window));
        elementEnd = found ? foundStart : strLength;
        elementLen = elementEnd - beginPos;
        element.tag = STRING_TAG;
        element.val.str = AllocString(elementLen + 1);
        if (!element.val.str) {
            *errMsg = "failed to allocate element value: %s";
            return(False);
        }
        strncpy(element.val.str, &sourceStr[beginPos], elementLen);
        element.val.str[elementLen] = 0;

        if (!arrayInsert(result, allocIndexStr, &element)) {
            *errMsg = "array element failed to insert: %s";
            return(False);
        }

        beginPos = found ? foundEnd : strLength;
        ++indexNum;
    }
    if (found) {
        sprintf(indexStr, "%d", indexNum);
        allocIndexStr = AllocString(strlen(indexStr) + 1);
        if (!allocIndexStr) {
            *errMsg = "array element failed to allocate key: %s";
            return(False);
        }
        strcpy(allocIndexStr, indexStr);
        element.tag = STRING_TAG;
        element.val.str = AllocString(1);
        if (!element.val.str) {
            *errMsg = "failed to allocate element value: %s";
            return(False);
        }
        element.val.str[0] = 0;

        if (!arrayInsert(result, allocIndexStr, &element)) {
            *errMsg = "array element failed to insert: %s";
            return(False);
        }
    }
    return(True);
}

static int cursorMV(WindowInfo *window, DataValue *argList, int nArgs,
    	DataValue *result, char **errMsg)
{
    result->tag = INT_TAG;
    result->val.n = TextGetCursorPos(window->lastFocus);
    return True;
}

static int lineMV(WindowInfo *window, DataValue *argList, int nArgs,
        DataValue *result, char **errMsg)
{
    int line, cursorPos, colNum;

    result->tag = INT_TAG;
    cursorPos = TextGetCursorPos(window->lastFocus);
    if (!TextPosToLineAndCol(window->lastFocus, cursorPos, &line, &colNum))
    	line = BufCountLines(window->buffer, 0, cursorPos) + 1;
    result->val.n = line;
    return True;
}

static int columnMV(WindowInfo *window, DataValue *argList, int nArgs,
        DataValue *result, char **errMsg)
{
    textBuffer *buf = window->buffer;
    int cursorPos;

    result->tag = INT_TAG;
    cursorPos = TextGetCursorPos(window->lastFocus);
    result->val.n = BufCountDispChars(buf, BufStartOfLine(buf, cursorPos),
	    cursorPos);
    return True;
}

static int fileNameMV(WindowInfo *window, DataValue *argList, int nArgs,
    	DataValue *result, char **errMsg)
{
    result->tag = STRING_TAG;
    result->val.str = AllocString(strlen(window->filename) + 1);
    strcpy(result->val.str, window->filename);
    return True;
}

static int filePathMV(WindowInfo *window, DataValue *argList, int nArgs,
    	DataValue *result, char **errMsg)
{
    result->tag = STRING_TAG;
    result->val.str = AllocString(strlen(window->path) + 1);
    strcpy(result->val.str, window->path);
    return True;
}

static int lengthMV(WindowInfo *window, DataValue *argList, int nArgs,
    	DataValue *result, char **errMsg)
{
    result->tag = INT_TAG;
    result->val.n = window->buffer->length;
    return True;
}

static int selectionStartMV(WindowInfo *window, DataValue *argList, int nArgs,
    	DataValue *result, char **errMsg)
{
    result->tag = INT_TAG;
    result->val.n = window->buffer->primary.selected ?
    	    window->buffer->primary.start : -1;
    return True;
}

static int selectionEndMV(WindowInfo *window, DataValue *argList, int nArgs,
    	DataValue *result, char **errMsg)
{
    result->tag = INT_TAG;
    result->val.n = window->buffer->primary.selected ?
    	    window->buffer->primary.end : -1;
    return True;
}

static int selectionLeftMV(WindowInfo *window, DataValue *argList, int nArgs,
    	DataValue *result, char **errMsg)
{
    selection *sel = &window->buffer->primary;
    
    result->tag = INT_TAG;
    result->val.n = sel->selected && sel->rectangular ? sel->rectStart : -1;
    return True;
}

static int selectionRightMV(WindowInfo *window, DataValue *argList, int nArgs,
    	DataValue *result, char **errMsg)
{
    selection *sel = &window->buffer->primary;
    
    result->tag = INT_TAG;
    result->val.n = sel->selected && sel->rectangular ? sel->rectEnd : -1;
    return True;
}

static int wrapMarginMV(WindowInfo *window, DataValue *argList, int nArgs,
    	DataValue *result, char **errMsg)
{
    int margin, nCols;
    
    XtVaGetValues(window->textArea, textNcolumns, &nCols,
    	    textNwrapMargin, &margin, NULL);
    result->tag = INT_TAG;
    result->val.n = margin == 0 ? nCols : margin;
    return True;
}

static int statisticsLineMV(WindowInfo *window, DataValue *argList, int nArgs,
    DataValue *result, char **errMsg)
{
    result->tag = INT_TAG;
    result->val.n = window->showStats ? 1 : 0;
    return True;
}

static int incSearchLineMV(WindowInfo *window, DataValue *argList, int nArgs,
    DataValue *result, char **errMsg)
{
    result->tag = INT_TAG;
    result->val.n = window->showISearchLine ? 1 : 0;
    return True;
}

static int showLineNumbersMV(WindowInfo *window, DataValue *argList, int nArgs,
    DataValue *result, char **errMsg)
{
    result->tag = INT_TAG;
    result->val.n = window->showLineNumbers ? 1 : 0;
    return True;
}

static int autoIndentMV(WindowInfo *window, DataValue *argList, int nArgs,
    DataValue *result, char **errMsg)
{
    char *indentStyleStr = "";
    
    switch (window->indentStyle) {
        case NO_AUTO_INDENT:
            indentStyleStr = "off";
        break;
        case AUTO_INDENT:
            indentStyleStr = "on";
        break;
        case SMART_INDENT:
            indentStyleStr = "smart";
        break;
    }
    result->tag = STRING_TAG;
    result->val.str = AllocString(strlen(indentStyleStr) + 1);
    strcpy(result->val.str, indentStyleStr);
    return True;
}

static int wrapTextMV(WindowInfo *window, DataValue *argList, int nArgs,
    DataValue *result, char **errMsg)
{
    char *wrapStyleStr = "";
    
    switch (window->wrapMode) {
        case NO_WRAP:
            wrapStyleStr = "none";
        break;
        case NEWLINE_WRAP:
            wrapStyleStr = "auto";
        break;
        case CONTINUOUS_WRAP:
            wrapStyleStr = "continuous";
        break;
    }
    result->tag = STRING_TAG;
    result->val.str = AllocString(strlen(wrapStyleStr) + 1);
    strcpy(result->val.str, wrapStyleStr);
    return True;
}

static int highlightSyntaxMV(WindowInfo *window, DataValue *argList, int nArgs,
    DataValue *result, char **errMsg)
{
    result->tag = INT_TAG;
    result->val.n = window->highlightSyntax ? 1 : 0;
    return True;
}

static int makeBackupCopyMV(WindowInfo *window, DataValue *argList, int nArgs,
    DataValue *result, char **errMsg)
{
    result->tag = INT_TAG;
    result->val.n = window->saveOldVersion ? 1 : 0;
    return True;
}

static int incBackupMV(WindowInfo *window, DataValue *argList, int nArgs,
    DataValue *result, char **errMsg)
{
    result->tag = INT_TAG;
    result->val.n = window->autoSave ? 1 : 0;
    return True;
}

static int showMatchingMV(WindowInfo *window, DataValue *argList, int nArgs,
    DataValue *result, char **errMsg)
{
    result->tag = INT_TAG;
    result->val.n = window->showMatching ? 1 : 0;
    return True;
}

static int overTypeModeMV(WindowInfo *window, DataValue *argList, int nArgs,
    DataValue *result, char **errMsg)
{
    result->tag = INT_TAG;
    result->val.n = window->overstrike ? 1 : 0;
    return True;
}

static int readOnlyMV(WindowInfo *window, DataValue *argList, int nArgs,
    DataValue *result, char **errMsg)
{
    result->tag = INT_TAG;
    result->val.n = (window->readOnly || window->lockWrite) ? 1 : 0;
    return True;
}

static int lockedMV(WindowInfo *window, DataValue *argList, int nArgs,
    DataValue *result, char **errMsg)
{
    result->tag = INT_TAG;
    result->val.n = window->lockWrite ? 1 : 0;
    return True;
}

static int fileFormatMV(WindowInfo *window, DataValue *argList, int nArgs,
    DataValue *result, char **errMsg)
{
    char *linefeedStyleStr = "";
    
    switch (window->fileFormat) {
        case UNIX_FILE_FORMAT:
            linefeedStyleStr = "unix";
        break;
        case DOS_FILE_FORMAT:
            linefeedStyleStr = "dos";
        break;
        case MAC_FILE_FORMAT:
            linefeedStyleStr = "macintosh";
        break;
    }
    result->tag = STRING_TAG;
    result->val.str = AllocString(strlen(linefeedStyleStr) + 1);
    strcpy(result->val.str, linefeedStyleStr);
    return True;
}

static int fontNameMV(WindowInfo *window, DataValue *argList, int nArgs,
    DataValue *result, char **errMsg)
{
    result->tag = STRING_TAG;
    result->val.str = AllocString(strlen(window->fontName) + 1);
    strcpy(result->val.str, window->fontName);
    return True;
}

static int fontNameItalicMV(WindowInfo *window, DataValue *argList, int nArgs,
    DataValue *result, char **errMsg)
{
    result->tag = STRING_TAG;
    result->val.str = AllocString(strlen(window->italicFontName) + 1);
    strcpy(result->val.str, window->italicFontName);
    return True;
}

static int fontNameBoldMV(WindowInfo *window, DataValue *argList, int nArgs,
    DataValue *result, char **errMsg)
{
    result->tag = STRING_TAG;
    result->val.str = AllocString(strlen(window->boldFontName) + 1);
    strcpy(result->val.str, window->boldFontName);
    return True;
}

static int fontNameBoldItalicMV(WindowInfo *window, DataValue *argList, int nArgs,
    DataValue *result, char **errMsg)
{
    result->tag = STRING_TAG;
    result->val.str = AllocString(strlen(window->boldItalicFontName) + 1);
    strcpy(result->val.str, window->boldItalicFontName);
    return True;
}

static int subscriptSepMV(WindowInfo *window, DataValue *argList, int nArgs,
    DataValue *result, char **errMsg)
{
    static char subSepStr[sizeof(ARRAY_DIM_SEP)+2];
    
    result->tag = STRING_TAG;
    strcpy(&subSepStr[1], ARRAY_DIM_SEP);
     /* This allows grabage collection to think this is allocated */
     /* but since it isn't, it won't get deleted */
    result->val.str = &subSepStr[1];
    return True;
}

static int minFontWidthMV(WindowInfo *window, DataValue *argList, int nArgs,
    DataValue *result, char **errMsg)
{
    result->tag = INT_TAG;
    result->val.n = TextGetMinFontWidth(window->textArea, window->highlightSyntax);
    return True;
}

static int maxFontWidthMV(WindowInfo *window, DataValue *argList, int nArgs,
    DataValue *result, char **errMsg)
{
    result->tag = INT_TAG;
    result->val.n = TextGetMaxFontWidth(window->textArea, window->highlightSyntax);
    return True;
}

static int topLineMV(WindowInfo *window, DataValue *argList, int nArgs,
    DataValue *result, char **errMsg)
{
    result->tag = INT_TAG;
    result->val.n = TextFirstVisibleLine(window->lastFocus);
    return True;
}

static int numDisplayLinesMV(WindowInfo *window, DataValue *argList, int nArgs,
    DataValue *result, char **errMsg)
{
    result->tag = INT_TAG;
    result->val.n = TextNumVisibleLines(window->lastFocus);
    return True;
}

static int displayWidthMV(WindowInfo *window, DataValue *argList, int nArgs,
    DataValue *result, char **errMsg)
{
    result->tag = INT_TAG;
    result->val.n = TextVisibleWidth(window->lastFocus);
    return True;
}

static int activePaneMV(WindowInfo *window, DataValue *argList, int nArgs,
    DataValue *result, char **errMsg)
{
    result->tag = INT_TAG;
    result->val.n = WidgetToPaneIndex(window, window->lastFocus) + 1;
    return True;
}

static int nPanesMV(WindowInfo *window, DataValue *argList, int nArgs,
    DataValue *result, char **errMsg)
{
    result->tag = INT_TAG;
    result->val.n = window->nPanes + 1;
    return True;
}

static int tabDistMV(WindowInfo *window, DataValue *argList, int nArgs,
    	DataValue *result, char **errMsg)
{
    result->tag = INT_TAG;
    result->val.n = window->buffer->tabDist;
    return True;
}

static int emTabDistMV(WindowInfo *window, DataValue *argList, int nArgs,
    	DataValue *result, char **errMsg)
{
    int dist;
    
    XtVaGetValues(window->textArea, textNemulateTabs, &dist, NULL);
    result->tag = INT_TAG;
    result->val.n = dist == 0 ? -1 : dist;
    return True;
}

static int useTabsMV(WindowInfo *window, DataValue *argList, int nArgs,
    	DataValue *result, char **errMsg)
{
    result->tag = INT_TAG;
    result->val.n = window->buffer->useTabs;
    return True;
}

static int modifiedMV(WindowInfo *window, DataValue *argList, int nArgs,
    	DataValue *result, char **errMsg)
{
    result->tag = INT_TAG;
    result->val.n = window->fileChanged;
    return True;
}

static int languageModeMV(WindowInfo *window, DataValue *argList, int nArgs,
    	DataValue *result, char **errMsg)
{
    char *lmName = LanguageModeName(window->languageMode);
    
    if (lmName == NULL)
    	lmName = "Plain";
    result->tag = STRING_TAG;
    result->val.str = AllocString(strlen(lmName) + 1);
    strcpy(result->val.str, lmName);
    return True;
}

static int wrongNArgsErr(char **errMsg)
{
    *errMsg = "wrong number of arguments to function %s";
    return False;
}

static int tooFewArgsErr(char **errMsg)
{
    *errMsg = "too few arguments to function %s";
    return False;
}

/*
** strCaseCmp compares its arguments and returns 0 if the two strings
** are equal IGNORING case differences.  Otherwise returns 1 or -1
** depending on relative comparison.
*/
static int strCaseCmp(char *str1, char *str2)
{
    char *c1, *c2;

    for (c1 = str1, c2 = str2; (*c1 != '\0' && *c2 != '\0') && 
        toupper((unsigned char)*c1) == toupper((unsigned char)*c2); ++c1, ++c2) {
    }
    if (((unsigned char)toupper((unsigned char)*c1)) > ((unsigned char)toupper((unsigned char)*c2))) {
        return(1);
    }
    else if (((unsigned char)toupper((unsigned char)*c1)) < ((unsigned char)toupper((unsigned char)*c2))) {
        return(-1);
    }
    else {
        return(0);
    }
}

/*
** Get an integer value from a tagged DataValue structure.  Return True
** if conversion succeeded, and store result in *result, otherwise
** return False with an error message in *errMsg.
*/
static int readIntArg(DataValue dv, int *result, char **errMsg)
{
    char *c;
    
    if (dv.tag == INT_TAG) {
    	*result = dv.val.n;
    	return True;
    } else if (dv.tag == STRING_TAG) {
	for (c=dv.val.str; *c != '\0'; c++) {
    	    if (!(isdigit(*c) || *c == ' ' || *c == '\t')) {
    		goto typeError;
    	    }
    	}
	sscanf(dv.val.str, "%d", result);
	return True;
    }
    
typeError:
    *errMsg = "%s called with non-integer argument";
    return False;
}

/*
** Get an string value from a tagged DataValue structure.  Return True
** if conversion succeeded, and store result in *result, otherwise
** return False with an error message in *errMsg.  If an integer value
** is converted, write the string in the space provided by "stringStorage",
** which must be large enough to handle ints of the maximum size.
*/
static int readStringArg(DataValue dv, char **result, char *stringStorage,
    	char **errMsg)
{
    if (dv.tag == STRING_TAG) {
    	*result = dv.val.str;
    	return True;
    } else if (dv.tag == INT_TAG) {
	sprintf(stringStorage, "%d", dv.val.n);
	*result = stringStorage;
	return True;
    }
    *errMsg = "%s called with unknown object";
    return False;
}
