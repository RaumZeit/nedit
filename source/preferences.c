static const char CVSID[] = "$Id: preferences.c,v 1.32 2001/08/25 15:58:54 amai Exp $";
/*******************************************************************************
*									       *
* preferences.c -- Nirvana Editor preferences processing		       *
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
* FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License *
* for more details.							       *
* 									       *
* You should have received a copy of the GNU General Public License along with *
* software; if not, write to the Free Software Foundation, Inc., 59 Temple     *
* Place, Suite 330, Boston, MA  02111-1307 USA		                       *
*									       *
* Nirvana Text Editor	    						       *
* April 20, 1993							       *
*									       *
* Written by Mark Edel							       *
*									       *
*******************************************************************************/
#include <stdlib.h>
#include <ctype.h>
#ifdef VMS
#include "../util/VMSparam.h"
#else
#ifndef __MVS__
#include <sys/param.h>
#endif
#endif /*VMS*/
#include <string.h>
#include <stdio.h>
#include <Xm/Xm.h>
#include <Xm/SelectioB.h>
#include <Xm/Form.h>
#include <Xm/List.h>
#include <Xm/SeparatoG.h>
#include <Xm/LabelG.h>
#include <Xm/PushBG.h>
#include <Xm/PushB.h>
#include <Xm/ToggleBG.h>
#include <Xm/ToggleB.h>
#include <Xm/RowColumn.h>
#include <Xm/CascadeBG.h>
#include <Xm/Frame.h>
#include <Xm/Text.h>
#include "../util/prefFile.h"
#include "../util/misc.h"
#include "../util/DialogF.h"
#include "../util/managedList.h"
#include "../util/fontsel.h"
#include "textBuf.h"
#include "nedit.h"
#include "text.h"
#include "search.h"
#include "preferences.h"
#include "window.h"
#include "userCmds.h"
#include "highlight.h"
#include "highlightData.h"
#include "help.h"
#include "regularExp.h"
#include "smartIndent.h"

#define PREF_FILE_NAME ".nedit"

/* maximum number of word delimiters allowed (256 allows whole character set) */
#define MAX_WORD_DELIMITERS 256

/* maximum number of file extensions allowed in a language mode */
#define MAX_FILE_EXTENSIONS 20

/* Return values for checkFontStatus */
enum fontStatus {GOOD_FONT, BAD_PRIMARY, BAD_FONT, BAD_SIZE, BAD_SPACING};

/* enumerated type preference strings 
** The order of the elements in this array must be exactly the same
** as the order of the coresponding integers of the enum SearchType
** defined in search.h (!!)
*/
static char *SearchMethodStrings[] = {
  	"Literal", "CaseSense", "RegExp", 
	"LiteralWord", "CaseSenseWord", "RegExpNoCase", 
	NULL
};

#ifdef REPLACE_SCOPE
/* enumerated default scope for replace dialog if a selection exists when
** the dialog is popped up.
*/
static char *ReplaceDefScopeStrings[] = {
	"Window", "Selection", "Smart", NULL
};
#endif

#define N_WRAP_STYLES 3
static char *AutoWrapTypes[N_WRAP_STYLES+3] = {"None", "Newline", "Continuous",
    	"True", "False", NULL};
#define N_INDENT_STYLES 3
static char *AutoIndentTypes[N_INDENT_STYLES+3] = {"None", "Auto",
    	"Smart", "True", "False", NULL};

#define N_SHOW_MATCHING_STYLES 3
/* For backward compatibility, "False" and "True" are still accepted.
   They are internally converted to "Off" and "Delimiter" respectively. 
   NOTE: N_SHOW_MATCHING_STYLES must correspond to the number of 
         _real_ matching styles, not counting False & True. 
         False and True should also be the last ones in the list. */
static char *ShowMatchingTypes[] = {"Off", "Delimiter", "Range", 
	"False", "True", NULL};

/* suplement wrap and indent styles w/ a value meaning "use default" for
   the override fields in the language modes dialog */
#define DEFAULT_WRAP -1
#define DEFAULT_INDENT -1
#define DEFAULT_TAB_DIST -1
#define DEFAULT_EM_TAB_DIST -1

/* list of available language modes and language specific preferences */
static int NLanguageModes = 0;
typedef struct {
    char *name;
    int nExtensions;
    char **extensions;
    char *recognitionExpr;
    char *delimiters;
    int wrapStyle;	
    int indentStyle;	
    int tabDist;	
    int emTabDist;	
} languageModeRec;
static languageModeRec *LanguageModes[MAX_LANGUAGE_MODES];

/* Language mode dialog information */
static struct {
    Widget shell;
    Widget nameW;
    Widget extW;
    Widget recogW;
    Widget delimitW;
    Widget managedListW;
    Widget tabW;
    Widget emTabW;
    Widget defaultIndentW;
    Widget noIndentW;
    Widget autoIndentW;
    Widget smartIndentW;
    Widget defaultWrapW;
    Widget noWrapW;
    Widget newlineWrapW;
    Widget contWrapW;
    languageModeRec **languageModeList;
    int nLanguageModes;
} LMDialog = {NULL};

/* Font dialog information */
typedef struct {
    Widget shell;
    Widget primaryW;
    Widget fillW;
    Widget italicW;
    Widget italicErrW;
    Widget boldW;
    Widget boldErrW;
    Widget boldItalicW;
    Widget boldItalicErrW;
    WindowInfo *window;
    int forWindow;
} fontDialog;

/* Repository for simple preferences settings */
static struct prefData {
    int wrapStyle;		/* what kind of wrapping to do */
    int wrapMargin;		/* 0=wrap at window width, other=wrap margin */
    int autoIndent;		/* style for auto-indent */
    int autoSave;		/* whether automatic backup feature is on */
    int saveOldVersion;		/* whether to preserve a copy of last version */
    int searchDlogs;		/* whether to show explanatory search dialogs */
    int searchWrapBeep;     	/* 1=beep when search restarts at begin/end */
    int keepSearchDlogs;	/* whether to retain find and replace dialogs */
    int searchWraps;	/* whether to attempt search again if reach bof or eof */
    int statsLine;		/* whether to show the statistics line */
    int iSearchLine;	    	/* whether to show the incremental search line*/
    int lineNums;   	    	/* whether to show line numbers */
    int pathInWindowsMenu;   	/* whether to show path in windows menu */
    int warnFileMods;	    	/* " warn user if files externally modified */
    int warnExit;	    	/* whether to warn on exit */
    int searchMethod;		/* initial search method as a text string */
#ifdef REPLACE_SCOPE
    int replaceDefScope;	/* default replace scope if selection exists */
#endif
    int textRows;		/* initial window height in characters */
    int textCols;		/* initial window width in characters */
    int tabDist;		/* number of characters between tab stops */
    int emTabDist;		/* non-zero tab dist. if emulated tabs are on */
    int insertTabs;		/* whether to use tabs for padding */
    int showMatchingStyle;	/* how to flash matching parenthesis */
    int highlightSyntax;    	/* whether to highlight syntax by default */
    int smartTags;  	    	/* look for tag in current window first */
    int stickyCaseSenseBtn;     /* whether Case Word Btn is sticky to Regex Btn */
    int prefFileRead;	    	/* detects whether a .nedit existed */
#ifdef SGI_CUSTOM
    int shortMenus; 	    	/* short menu mode */
#endif
    char fontString[MAX_FONT_LEN]; /* names of fonts for text widget */
    char boldFontString[MAX_FONT_LEN];
    char italicFontString[MAX_FONT_LEN];
    char boldItalicFontString[MAX_FONT_LEN];
    XmFontList fontList;	/* XmFontLists corresp. to above named fonts */
    XFontStruct *boldFontStruct;
    XFontStruct *italicFontStruct;
    XFontStruct *boldItalicFontStruct;
    int repositionDialogs;	/* w. to reposition dialogs under the pointer */
    int sortOpenPrevMenu;   	/* whether to sort the "Open Previous" menu */
    int mapDelete;		/* whether to map delete to backspace */
    int stdOpenDialog;		/* w. to retain redundant text field in Open */
    char tagFile[MAXPATHLEN];	/* name of tags file to look for at startup */
    int maxPrevOpenFiles;   	/* limit to size of Open Previous menu */
    char delimiters[MAX_WORD_DELIMITERS]; /* punctuation characters */
    char shell[MAXPATHLEN];	/* shell to use for executing commands */
    char geometry[MAX_GEOM_STRING_LEN];	/* per-application geometry string,
    	    	    	    	    	   only for the clueless */
    char serverName[MAXPATHLEN];/* server name for multiple servers per disp. */
    char bgMenuBtn[MAX_ACCEL_LEN]; /* X event description for triggering
    	    	    	    	      posting of background menu */
    char fileVersion[4]; 	/* Version of nedit which wrote the .nedit
    				   file we're reading */
    int findReplaceUsesSelection; /* whether the find replace dialog is automatically
                                     loaded with the primary selection */
} PrefData;

/* Temporary storage for preferences strings which are discarded after being
   read */
static struct {
    char *shellCmds;
    char *macroCmds;
    char *bgMenuCmds;
    char *highlight;
    char *language;
    char *styles;
    char *smartIndent;
    char *smartIndentCommon;
} TempStringPrefs;

/* preference descriptions for SavePreferences and RestorePreferences. */
static PrefDescripRec PrefDescrip[] = {
    {"fileVersion", "FileVersion" , PREF_STRING, "", PrefData.fileVersion,
      (void *)sizeof(PrefData.fileVersion), True},
#ifndef VMS
#ifdef linux
    {"shellCommands", "ShellCommands", PREF_ALLOC_STRING, "spell:Alt+B:s:EX:\n\
	cat>spellTmp; xterm -e ispell -x spellTmp; cat spellTmp; rm spellTmp\n\
	wc::w:ED:\nset wc=`wc`; echo $wc[1] \"lines,\" $wc[2] \"words,\" $wc[3] \"characters\"\n\
	sort::o:EX:\nsort\nnumber lines::n:AW:\nnl -ba\nmake:Alt+Z:m:W:\nmake\n\
	expand::p:EX:\nexpand\nunexpand::u:EX:\nunexpand\n",
    	&TempStringPrefs.shellCmds, NULL, True},
#elif __FreeBSD__
    {"shellCommands", "ShellCommands", PREF_ALLOC_STRING, "spell:Alt+B:s:EX:\n\
      cat>spellTmp; xterm -e ispell -x spellTmp; cat spellTmp; rm spellTmp\n\
      wc::w:ED:\nset wc=`wc`; echo $wc[1] \"words,\" $wc[2] \"lines,\" $wc[3] \"characters\"\n\
      sort::o:EX:\nsort\nnumber lines::n:AW:\npr -tn\nmake:Alt+Z:m:W:\nmake\n\
      expand::p:EX:\nexpand\nunexpand::u:EX:\nunexpand\n",
      &TempStringPrefs.shellCmds, NULL, True},
#else
    {"shellCommands", "ShellCommands", PREF_ALLOC_STRING, "spell:Alt+B:s:ED:\n\
    	(cat;echo \"\") | spell\nwc::w:ED:\nset wc=`wc`; echo $wc[1] \"lines,\" $wc[2] \"words,\" $wc[3] \"characters\"\n\
    	\nsort::o:EX:\nsort\nnumber lines::n:AW:\nnl -ba\nmake:Alt+Z:m:W:\nmake\n\
	expand::p:EX:\nexpand\nunexpand::u:EX:\nunexpand\n",
    	&TempStringPrefs.shellCmds, NULL, True},
#endif /* linux, __FreeBSD__ */
#endif /* VMS */
    {"macroCommands", "MacroCommands", PREF_ALLOC_STRING,
	"Complete Word:Alt+D::: {\n\
		# Tuning parameters\n\
		ScanDistance = 200\n\
		\n\
		# Search back to a word boundary to find the word to complete\n\
		startScan = max(0, $cursor - ScanDistance)\n\
		endScan = min($text_length, $cursor + ScanDistance)\n\
		scanString = get_range(startScan, endScan)\n\
		keyEnd = $cursor-startScan\n\
		keyStart = search_string(scanString, \"<\", keyEnd, \"backward\", \"regex\")\n\
		if (keyStart == -1)\n\
		    return\n\
		keyString = \"<\" substring(scanString, keyStart, keyEnd)\n\
		\n\
		# search both forward and backward from the cursor position.  Note that\n\
		# using a regex search can lead to incorrect results if any of the special\n\
		# regex characters is encountered, which is not considered a delimiter\n\
		backwardSearchResult = search_string(scanString, keyString, keyStart-1, \\\n\
		    	\"backward\", \"regex\")\n\
		forwardSearchResult = search_string(scanString, keyString, keyEnd, \"regex\")\n\
		if (backwardSearchResult == -1 && forwardSearchResult == -1) {\n\
		    beep()\n\
		    return\n\
		}\n\
		\n\
		# if only one direction matched, use that, otherwise use the nearest\n\
		if (backwardSearchResult == -1)\n\
		    matchStart = forwardSearchResult\n\
		else if (forwardSearchResult == -1)\n\
		    matchStart = backwardSearchResult\n\
		else {\n\
		    if (keyStart - backwardSearchResult <= forwardSearchResult - keyEnd)\n\
		    	matchStart = backwardSearchResult\n\
		    else\n\
		    	matchStart = forwardSearchResult\n\
		}\n\
		\n\
		# find the complete word\n\
		matchEnd = search_string(scanString, \">\", matchStart, \"regex\")\n\
		completedWord = substring(scanString, matchStart, matchEnd)\n\
		\n\
		# replace it in the window\n\
		replace_range(startScan + keyStart, $cursor, completedWord)\n\
	}\n\
	Fill Sel. w/Char:::R: {\n\
		if ($selection_start == -1) {\n\
		    beep()\n\
		    return\n\
		}\n\
		\n\
		# Ask the user what character to fill with\n\
		fillChar = string_dialog(\"Fill selection with what character?\", \"OK\", \"Cancel\")\n\
		if ($string_dialog_button == 2 || $string_dialog_button == 0)\n\
		    return\n\
		\n\
		# Count the number of lines in the selection\n\
		nLines = 0\n\
		for (i=$selection_start; i<$selection_end; i++)\n\
		    if (get_character(i) == \"\\n\")\n\
		    	nLines++\n\
		\n\
		# Create the fill text\n\
		rectangular = $selection_left != -1\n\
		line = \"\"\n\
		fillText = \"\"\n\
		if (rectangular) {\n\
		    for (i=0; i<$selection_right-$selection_left; i++)\n\
			line = line fillChar\n\
		    for (i=0; i<nLines; i++)\n\
			fillText = fillText line \"\\n\"\n\
		    fillText = fillText line\n\
		} else {\n\
		    if (nLines == 0) {\n\
		    	for (i=$selection_start; i<$selection_end; i++)\n\
		    	    fillText = fillText fillChar\n\
		    } else {\n\
		    	startIndent = 0\n\
		    	for (i=$selection_start-1; i>=0 && get_character(i)!=\"\\n\"; i--)\n\
		    	    startIndent++\n\
		    	for (i=0; i<$wrap_margin-startIndent; i++)\n\
		    	    fillText = fillText fillChar\n\
		    	fillText = fillText \"\\n\"\n\
			for (i=0; i<$wrap_margin; i++)\n\
			    line = line fillChar\n\
			for (i=0; i<nLines-1; i++)\n\
			    fillText = fillText line \"\\n\"\n\
			for (i=$selection_end-1; i>=$selection_start && get_character(i)!=\"\\n\"; \\\n\
			    	i--)\n\
			    fillText = fillText fillChar\n\
		    }\n\
		}\n\
		\n\
		# Replace the selection with the fill text\n\
		replace_selection(fillText)\n\
	}\n\
	Quote Mail Reply:::: {\n\
		if ($selection_start == -1)\n\
		    replace_all(\"^.*$\", \"\\\\> &\", \"regex\")\n\
		else\n\
		    replace_in_selection(\"^.*$\", \"\\\\> &\", \"regex\")\n\
	}\n\
	Unquote Mail Reply:::: {\n\
		if ($selection_start == -1)\n\
		    replace_all(\"(^\\\\> )(.*)$\", \"\\\\2\", \"regex\")\n\
		else\n\
		    replace_in_selection(\"(^\\\\> )(.*)$\", \"\\\\2\", \"regex\")\n\
	}\n\
	C Comments>Comment Out Sel.@C@C++:::R: {\n\
		selStart = $selection_start\n\
		selEnd = $selection_end\n\
		replace_range(selStart, selEnd, \"/* \" get_selection() \" */\")\n\
		select(selStart, selEnd + 6)\n\
	}\n\
	C Comments>C Uncomment Sel.@C@C++:::R: {\n\
		sel = get_selection()\n\
		selStart = $selection_start\n\
		selEnd = $selection_end\n\
		commentStart = search_string(sel, \"/*\", 0)\n\
		if (substring(sel, commentStart+2, commentStart+3) == \" \")\n\
		    keepStart = commentStart + 3\n\
		else\n\
		    keepStart = commentStart + 2\n\
		keepEnd = search_string(sel, \"*/\", length(sel), \"backward\")\n\
		commentEnd = keepEnd + 2\n\
		if (substring(sel, keepEnd - 1, keepEnd == \" \"))\n\
		    keepEnd = keepEnd - 1\n\
		replace_range(selStart + commentStart, selStart + commentEnd, \\\n\
			substring(sel, keepStart, keepEnd))\n\
		select(selStart, selEnd - (keepStart-commentStart) - \\\n\
			(commentEnd - keepEnd))\n\
	}\n\
	C Comments>+ C++ Comment@C++:::R: {\n\
		replace_in_selection(\"^.*$\", \"// &\", \"regex\")\n\
	}\n\
	C Comments>- C++ Comment@C++:::R: {\n\
		replace_in_selection(\"(^[ \\\\t]*// ?)(.*)$\", \"\\\\2\", \"regex\")\n\
	}\n\
	C Comments>+ C Bar Comment 1@C:::R: {\n\
		if ($selection_left != -1) {\n\
		    dialog(\"Selection must not be rectangular\")\n\
		    return\n\
		}\n\
		start = $selection_start\n\
		end = $selection_end-1\n\
		origText = get_range($selection_start, $selection_end-1)\n\
		newText = \"/*\\n\" replace_in_string(get_range(start, end), \\\n\
			\"^\", \" * \", \"regex\") \"\\n */\\n\"\n\
		replace_selection(newText)\n\
		select(start, start + length(newText))\n\
	}\n\
	C Comments>- C Bar Comment 1@C:::R: {\n\
		selStart = $selection_start\n\
		selEnd = $selection_end\n\
		newText = get_range(selStart+3, selEnd-4)\n\
		newText = replace_in_string(newText, \"^ \\\\* \", \"\", \"regex\")\n\
		replace_range(selStart, selEnd, newText)\n\
		select(selStart, selStart + length(newText))\n\
	}\n\
	Make C Prototypes@C@C++:::: {\n\
		if ($selection_start == -1) {\n\
		    start = 0\n\
		    end = $text_length\n\
		} else {\n\
		    start = $selection_start\n\
		    end = $selection_end\n\
		}\n\
		string = get_range(start, end)\n\
		nDefs = 0\n\
		searchPos = 0\n\
		prototypes = \"\"\n\
		staticPrototypes = \"\"\n\
		for (;;) {\n\
		    headerStart = search_string(string, \\\n\
			    \"^[a-zA-Z]([^;#\\\"'{}=><!/]|\\n)*\\\\)[ \\t]*\\n?[ \\t]*\\\\{\", \\\n\
			    searchPos, \"regex\")\n\
		    if (headerStart == -1)\n\
			break\n\
		    headerEnd = search_string(string, \")\", $search_end,\"backward\") + 1\n\
		    prototype = substring(string, headerStart, headerEnd) \";\\n\"\n\
		    if (substring(string, headerStart, headerStart+6) == \"static\")\n\
			staticPrototypes = staticPrototypes prototype\n\
		    else\n\
    			prototypes = prototypes prototype\n\
		    searchPos = headerEnd\n\
		    nDefs++\n\
		}\n\
		if (nDefs == 0) {\n\
		    dialog(\"No function declarations found\")\n\
		    return\n\
		}\n\
		new()\n\
		focus_window(\"last\")\n\
		replace_range(0, 0, prototypes staticPrototypes)\n\
	}", &TempStringPrefs.macroCmds, NULL, True},
    {"bgMenuCommands", "BGMenuCommands", PREF_ALLOC_STRING,
       "Undo:::: {\nundo()\n}\n\
	Redo:::: {\nredo()\n}\n\
	Cut:::R: {\ncut_clipboard()\n}\n\
	Copy:::R: {\ncopy_clipboard()\n}\n\
	Paste:::: {\npaste_clipboard()\n}", &TempStringPrefs.bgMenuCmds,
	NULL, True},
#ifdef VMS
/* The VAX compiler can't compile Java-Script's definition in highlightData.c */
    {"highlightPatterns", "HighlightPatterns", PREF_ALLOC_STRING,
       "Ada:Default\n\
        Awk:Default\n\
        C++:Default\n\
        C:Default\n\
        CSS:Default\n\
        Csh:Default\n\
        Fortran:Default\n\
        Java:Default\n\
        LaTeX:Default\n\
        Lex:Default\n\
        Makefile:Default\n\
        Matlab:Default\n\
        NEdit Macro:Default\n\
        Pascal:Default\n\
        Perl:Default\n\
        PostScript:Default\n\
        Python:Default\n\
        Regex:Default\n\
        SGML HTML:Default\n\
        SQL:Default\n\
        Sh Ksh Bash:Default\n\
        Tcl:Default\n\
        VHDL:Default\n\
        Verilog:Default\n\
        XML:Default\n\
        X Resources:Default\n\
        Yacc:Default",
        &TempStringPrefs.highlight, NULL, True},
    {"languageModes", "LanguageModes", PREF_ALLOC_STRING,
#else
    {"highlightPatterns", "HighlightPatterns", PREF_ALLOC_STRING,
       "Ada:Default\n\
        Awk:Default\n\
        C++:Default\n\
        C:Default\n\
        CSS:Default\n\
        Csh:Default\n\
        Fortran:Default\n\
        Java:Default\n\
        JavaScript:Default\n\
        LaTeX:Default\n\
        Lex:Default\n\
        Makefile:Default\n\
        Matlab:Default\n\
        NEdit Macro:Default\n\
        Pascal:Default\n\
        Perl:Default\n\
        PostScript:Default\n\
        Python:Default\n\
        Regex:Default\n\
        SGML HTML:Default\n\
        SQL:Default\n\
        Sh Ksh Bash:Default\n\
        Tcl:Default\n\
        VHDL:Default\n\
        Verilog:Default\n\
        XML:Default\n\
        X Resources:Default\n\
        Yacc:Default",
        &TempStringPrefs.highlight, NULL, True},
    {"languageModes", "LanguageModes", PREF_ALLOC_STRING,
#endif /*VMS*/
#ifdef VMS
       "Ada:.ADA .AD .ADS .ADB .A::::::\n\
        Awk:.AWK::::::\n\
        C++:.CC .HH .C .H .I .CXX .HXX .CPP::::::\".,/\\`'!|@#%^&*()-=+{}[]\"\":;<>?~\"\n\
        C:.C .H::::::\".,/\\`'!|@#%^&*()-=+{}[]\"\":;<>?~\"\n\
        CSS:CSS::Auto:None:::\".,/\\`'!|@#%^&*()=+{}[]\"\":;<>?~\"\n\
        Csh:.csh .cshrc .login .logout:\"^[ \\t]*#[ \\t]*![ \\t]*/bin/csh\":::::\n\
        Fortran:.F .F77 .FOR::::::\n\
        Java:.JAVA::::::\n\
        LaTeX:.TEX .STY .CLS .DTX .INS::::::\n\
        Lex:.lex::::::\n\
        Makefile:MAKEFILE:::None:8:8:\n\
        Matlab:.m .oct .sci::::::\n\
        NEdit Macro:.NM .NEDITMACRO::::::\n\
        Pascal:.PAS .P .INT::::::\n\
        Perl:.PL .PM .P5:\"^[ \\t]*#[ \\t]*!.*perl\":Auto:None:::\".,/\\\\`'!$@#%^&*()-=+{}[]\"\":;<>?~|\"\n\
        PostScript:.ps .PS .eps .EPS .epsf .epsi:\"^%!\":::::\"/%(){}[]<>\"\n\
        Python:.PY:\"^#!.*python\":Auto:None:::\n\
        Regex:.reg .regex:\"\\(\\?[:#=!iInN].+\\)\":None:Continuous:::\n\
        SGML HTML:.sgml .sgm .html .htm:\"\\<[Hh][Tt][Mm][Ll]\\>\":::::\n\
        SQL:.sql::::::\n\
        Sh Ksh Bash:.sh .bash .ksh .profile .bashrc .bash_logout .bash_login .bash_profile:\"^[ \\t]*#[ \\t]*![ \\t]*/.*bin/(sh|ksh|bash)\":::::\n\
        Tcl:.TCL::Smart:None:::\n\
        VHDL:.VHD .VHDL .VDL::::::\n\
        Verilog:.V::::::\n\
        XML:.xml .xsl .dtd:\"\\<(?i\\?xml|!doctype)\"::None:::\"<>/=\"\"'()+*?|\"\n\
        X Resources:.XRESOURCES .XDEFAULTS .NEDIT:\"^[!#].*([Aa]pp|[Xx]).*[Dd]efaults\":::::\n\
        Yacc:.Y::::::\".,/\\`'!|@#%^&*()-=+{}[]\"\":;<>?~\"",
#else
       "Ada:.ada .ad .ads .adb .a::::::\n\
        Awk:.awk::::::\n\
        C++:.cc .hh .C .H .i .cxx .hxx .cpp::::::\".,/\\`'!|@#%^&*()-=+{}[]\"\":;<>?~\"\n\
        C:.c .h::::::\".,/\\`'!|@#%^&*()-=+{}[]\"\":;<>?~\"\n\
        CSS:css::Auto:None:::\".,/\\`'!|@#%^&*()=+{}[]\"\":;<>?~\"\n\
        Csh:.csh .cshrc .login .logout:\"^[ \\t]*#[ \\t]*![ \\t]*/bin/csh\":::::\n\
        Fortran:.f .f77 .for::::::\n\
        Java:.java::::::\n\
        JavaScript:.js::::::\n\
        LaTeX:.tex .sty .cls .dtx .ins::::::\n\
        Lex:.lex::::::\n\
        Makefile:Makefile makefile .gmk:::None:8:8:\n\
        Matlab:.m .oct .sci::::::\n\
        NEdit Macro:.nm .neditmacro::::::\n\
        Pascal:.pas .p .int::::::\n\
        Perl:.pl .pm .p5 .PL:\"^[ \\t]*#[ \\t]*!.*perl\":Auto:None:::\".,/\\\\`'!$@#%^&*()-=+{}[]\"\":;<>?~|\"\n\
        PostScript:.ps .eps .epsf .epsi:\"^%!\":::::\"/%(){}[]<>\"\n\
        Python:.py:\"^#!.*python\":Auto:None:::\n\
        Regex:.reg .regex:\"\\(\\?[:#=!iInN].+\\)\":None:Continuous:::\n\
        SGML HTML:.sgml .sgm .html .htm:\"\\<[Hh][Tt][Mm][Ll]\\>\":::::\n\
        SQL:.sql::::::\n\
        Sh Ksh Bash:.sh .bash .ksh .profile .bashrc .bash_logout .bash_login .bash_profile:\"^[ \\t]*#[ \\t]*![ \\t]*/.*bin/(sh|ksh|bash)\":::::\n\
        Tcl:.tcl .tk .itcl .itk::Smart:None:::\n\
        VHDL:.vhd .vhdl .vdl::::::\n\
        Verilog:.v::::::\n\
        XML:.xml .xsl .dtd:\"\\<(?i\\?xml|!doctype)\"::None:::\"<>/=\"\"'()+*?|\"\n\
        X Resources:.Xresources .Xdefaults .nedit:\"^[!#].*([Aa]pp|[Xx]).*[Dd]efaults\":::::\n\
        Yacc:.y::::::\".,/\\`'!|@#%^&*()-=+{}[]\"\":;<>?~\"",
#endif
	&TempStringPrefs.language, NULL, True},
    {"styles", "Styles", PREF_ALLOC_STRING, "Plain:black:Plain\n\
    	Comment:gray20:Italic\n\
    	Keyword:black:Bold\n\
    	Storage Type:brown:Bold\n\
    	Storage Type1:saddle brown:Bold\n\
    	String:darkGreen:Plain\n\
    	String1:SeaGreen:Plain\n\
    	String2:darkGreen:Bold\n\
    	Preprocessor:RoyalBlue4:Plain\n\
    	Preprocessor1:blue:Plain\n\
    	Character Const:darkGreen:Plain\n\
    	Numeric Const:darkGreen:Plain\n\
    	Identifier:brown:Plain\n\
    	Identifier1:RoyalBlue4:Plain\n\
 	Subroutine:brown:Plain\n\
	Subroutine1:chocolate:Plain\n\
   	Ada Attributes:plum:Bold\n\
	Label:red:Italic\n\
	Flag:red:Bold\n\
    	Text Comment:SteelBlue4:Italic\n\
    	Text Key:VioletRed4:Bold\n\
	Text Key1:VioletRed4:Plain\n\
    	Text Arg:RoyalBlue4:Bold\n\
    	Text Arg1:SteelBlue4:Bold\n\
	Text Arg2:RoyalBlue4:Plain\n\
    	Text Escape:gray30:Bold\n\
	LaTeX Math:darkGreen:Plain\n\
	Pointer:#660000:Bold\n\
	Regex:#009944:Bold\n\
	Warning:brown2:Italic", &TempStringPrefs.styles, NULL, True},
    {"smartIndentInit", "SmartIndentInit", PREF_ALLOC_STRING,
        "C:Default\n\
	C++:Default\n\
	Python:Default\n\
	Matlab:Default", &TempStringPrefs.smartIndent, NULL, True},
    {"smartIndentInitCommon", "SmartIndentInitCommon", PREF_ALLOC_STRING,
        "Default", &TempStringPrefs.smartIndentCommon, NULL, True},
    {"autoWrap", "AutoWrap", PREF_ENUM, "Newline",
    	&PrefData.wrapStyle, AutoWrapTypes, True},
    {"wrapMargin", "WrapMargin", PREF_INT, "0",
    	&PrefData.wrapMargin, NULL, True},
    {"autoIndent", "AutoIndent", PREF_ENUM, "Auto",
    	&PrefData.autoIndent, AutoIndentTypes, True},
    {"autoSave", "AutoSave", PREF_BOOLEAN, "True",
    	&PrefData.autoSave, NULL, True},
    {"saveOldVersion", "SaveOldVersion", PREF_BOOLEAN, "False",
    	&PrefData.saveOldVersion, NULL, True},
    {"showMatching", "ShowMatching", PREF_ENUM, "Delimiter",
 	&PrefData.showMatchingStyle, ShowMatchingTypes, True},
    {"highlightSyntax", "HighlightSyntax", PREF_BOOLEAN, "True",
    	&PrefData.highlightSyntax, NULL, True},
    {"searchDialogs", "SearchDialogs", PREF_BOOLEAN, "False",
    	&PrefData.searchDlogs, NULL, True},
    {"beepOnSearchWrap", "BeepOnSearchWrap", PREF_BOOLEAN, "False",
      &PrefData.searchWrapBeep, NULL, True},
    {"retainSearchDialogs", "RetainSearchDialogs", PREF_BOOLEAN, "False",
    	&PrefData.keepSearchDlogs, NULL, True},
    {"searchWraps", "SearchWraps", PREF_BOOLEAN, "True",
    	&PrefData.searchWraps, NULL, True},
    {"stickyCaseSenseButton", "StickyCaseSenseButton", PREF_BOOLEAN, "True",
    	&PrefData.stickyCaseSenseBtn, NULL, True},
#if XmVersion < 1002 /* Flashing is annoying in 1.1 versions */
    {"repositionDialogs", "RepositionDialogs", PREF_BOOLEAN, "False",
    	&PrefData.repositionDialogs, NULL, True},
#else
    {"repositionDialogs", "RepositionDialogs", PREF_BOOLEAN, "True",
    	&PrefData.repositionDialogs, NULL, True},
#endif
    {"sortOpenPrevMenu", "SortOpenPrevMenu", PREF_BOOLEAN, "True",
    	&PrefData.sortOpenPrevMenu, NULL, True},
    {"statisticsLine", "StatisticsLine", PREF_BOOLEAN, "False",
    	&PrefData.statsLine, NULL, True},
    {"iSearchLine", "ISearchLine", PREF_BOOLEAN, "False",
    	&PrefData.iSearchLine, NULL, True},
    {"lineNumbers", "LineNumbers", PREF_BOOLEAN, "False",
    	&PrefData.lineNums, NULL, True},
    {"pathInWindowsMenu", "PathInWindowsMenu", PREF_BOOLEAN, "True",
    	&PrefData.pathInWindowsMenu, NULL, True},
    {"warnFileMods", "WarnFileMods", PREF_BOOLEAN, "True",
    	&PrefData.warnFileMods, NULL, True},
    {"warnExit", "WarnExit", PREF_BOOLEAN, "True",
    	&PrefData.warnExit, NULL, True},
    {"searchMethod", "SearchMethod", PREF_ENUM, "Literal",
    	&PrefData.searchMethod, SearchMethodStrings, True},
#ifdef REPLACE_SCOPE
    {"replaceDefaultScope", "ReplaceDefaultAllScope", PREF_ENUM, "Smart",
    	&PrefData.replaceDefScope, ReplaceDefScopeStrings, True},
#endif
    {"textRows", "TextRows", PREF_INT, "24",
    	&PrefData.textRows, NULL, True},
    {"textCols", "TextCols", PREF_INT, "80",
    	&PrefData.textCols, NULL, True},
    {"tabDistance", "TabDistance", PREF_INT, "8",
    	&PrefData.tabDist, NULL, True},
    {"emulateTabs", "EmulateTabs", PREF_INT, "0",
    	&PrefData.emTabDist, NULL, True},
    {"insertTabs", "InsertTabs", PREF_BOOLEAN, "True",
    	&PrefData.insertTabs, NULL, True},
    {"textFont", "TextFont", PREF_STRING,
    	"-adobe-courier-medium-r-normal--12-*-*-*-*-*-*",
    	PrefData.fontString, (void *)sizeof(PrefData.fontString), True},
    {"boldHighlightFont", "BoldHighlightFont", PREF_STRING,
    	"-adobe-courier-bold-r-normal--12-*-*-*-*-*-*",
    	PrefData.boldFontString, (void *)sizeof(PrefData.boldFontString), True},
    {"italicHighlightFont", "ItalicHighlightFont", PREF_STRING,
    	"-adobe-courier-medium-o-normal--12-*-*-*-*-*-*",
    	PrefData.italicFontString,
    	(void *)sizeof(PrefData.italicFontString), True},
    {"boldItalicHighlightFont", "BoldItalicHighlightFont", PREF_STRING,
    	"-adobe-courier-bold-o-normal--12-*-*-*-*-*-*",
    	PrefData.boldItalicFontString,
    	(void *)sizeof(PrefData.boldItalicFontString), True},
    {"shell", "Shell", PREF_STRING,
#ifdef __MVS__
    	"/bin/sh"
#else
        "/bin/csh",
#endif
    	PrefData.shell, (void *)sizeof(PrefData.shell), False},
    {"geometry", "Geometry", PREF_STRING, "",
    	PrefData.geometry, (void *)sizeof(PrefData.geometry), False},
    {"remapDeleteKey", "RemapDeleteKey", PREF_BOOLEAN, "False",
    	&PrefData.mapDelete, NULL, False},
    {"stdOpenDialog", "StdOpenDialog", PREF_BOOLEAN, "False",
    	&PrefData.stdOpenDialog, NULL, False},
    {"tagFile", "TagFile", PREF_STRING,
    	"", PrefData.tagFile, (void *)sizeof(PrefData.tagFile), False},
    {"wordDelimiters", "WordDelimiters", PREF_STRING,
    	".,/\\`'!|@#%^&*()-=+{}[]\":;<>?",
    	PrefData.delimiters, (void *)sizeof(PrefData.delimiters), False},
    {"serverName", "serverName", PREF_STRING, "", PrefData.serverName,
      (void *)sizeof(PrefData.serverName), False},
    {"maxPrevOpenFiles", "MaxPrevOpenFiles", PREF_INT, "30",
    	&PrefData.maxPrevOpenFiles, NULL, False},
    {"bgMenuButton", "BGMenuButton" , PREF_STRING,
	"~Shift~Ctrl~Meta~Alt<Btn3Down>", PrefData.bgMenuBtn,
      (void *)sizeof(PrefData.bgMenuBtn), False},
    {"smartTags", "SmartTags", PREF_BOOLEAN, "True",
    	&PrefData.smartTags, NULL, True},
    {"prefFileRead", "PrefFileRead", PREF_BOOLEAN, "False",
    	&PrefData.prefFileRead, NULL, True},
#ifdef SGI_CUSTOM
    {"shortMenus", "ShortMenus", PREF_BOOLEAN, "False", &PrefData.shortMenus,
      NULL, True},
#endif
    {"findReplaceUsesSelection", "FindReplaceUsesSelection", PREF_BOOLEAN, "False",
    	&PrefData.findReplaceUsesSelection, NULL, False},
};

static XrmOptionDescRec OpTable[] = {
    {"-wrap", ".autoWrap", XrmoptionNoArg, (caddr_t)"Continuous"},
    {"-nowrap", ".autoWrap", XrmoptionNoArg, (caddr_t)"None"},
    {"-autowrap", ".autoWrap", XrmoptionNoArg, (caddr_t)"Newline"},
    {"-noautowrap", ".autoWrap", XrmoptionNoArg, (caddr_t)"None"},
    {"-autoindent", ".autoIndent", XrmoptionNoArg, (caddr_t)"Auto"},
    {"-noautoindent", ".autoIndent", XrmoptionNoArg, (caddr_t)"False"},
    {"-autosave", ".autoSave", XrmoptionNoArg, (caddr_t)"True"},
    {"-noautosave", ".autoSave", XrmoptionNoArg, (caddr_t)"False"},
    {"-rows", ".textRows", XrmoptionSepArg, (caddr_t)NULL},
    {"-columns", ".textCols", XrmoptionSepArg, (caddr_t)NULL},
    {"-tabs", ".tabDistance", XrmoptionSepArg, (caddr_t)NULL},
    {"-font", ".textFont", XrmoptionSepArg, (caddr_t)NULL},
    {"-fn", ".textFont", XrmoptionSepArg, (caddr_t)NULL},
    {"-svrname", ".serverName", XrmoptionSepArg, (caddr_t)NULL},
};

static const char HeaderText[] = "\
! Preferences file for NEdit\n\
!\n\
! This file is overwritten by the \"Save Defaults...\" command in NEdit \n\
! and serves only the interactively settable options presented in the NEdit\n\
! \"Preferences\" menu.  To modify other options, such as background colors\n\
! and key bindings, use the .Xdefaults file in your home directory (or\n\
! the X resource specification method appropriate to your system).  The\n\
! contents of this file can be moved into an X resource file, but since\n\
! resources in this file override their corresponding X resources, either\n\
! this file should be deleted or individual resource lines in the file\n\
! should be deleted for the moved lines to take effect.\n";

/* Module-global variable set when any preference changes (for asking the
   user about re-saving on exit) */	
static int PrefsHaveChanged = False;

/* Module-global variable set when user uses -import to load additional
   preferences on top of the defaults.  Contains name of file loaded */
static char *ImportedFile = NULL;

/* Module-global variables to support Initial Window Size... dialog */
static int DoneWithSizeDialog;
static Widget RowText, ColText;

/* Module-global variables for Tabs dialog */
static int DoneWithTabsDialog;
static WindowInfo *TabsDialogForWindow;
static Widget TabDistText, EmTabText, EmTabToggle, UseTabsToggle, EmTabLabel;

/* Module-global variables for Wrap Margin dialog */
static int DoneWithWrapDialog;
static WindowInfo *WrapDialogForWindow;
static Widget WrapText, WrapTextLabel, WrapWindowToggle;

static void translatePrefFormats(int convertOld);
static void setIntPref(int *prefDataField, int newValue);
static void setStringPref(char *prefDataField, const char *newValue);
static void sizeOKCB(Widget w, XtPointer clientData, XtPointer callData);
static void sizeCancelCB(Widget w, XtPointer clientData, XtPointer callData);
static void tabsOKCB(Widget w, XtPointer clientData, XtPointer callData);
static void tabsCancelCB(Widget w, XtPointer clientData, XtPointer callData);
static void tabsHelpCB(Widget w, XtPointer clientData, XtPointer callData);
static void emTabsCB(Widget w, XtPointer clientData, XtPointer callData);
static void wrapOKCB(Widget w, XtPointer clientData, XtPointer callData);
static void wrapCancelCB(Widget w, XtPointer clientData, XtPointer callData);
static void wrapWindowCB(Widget w, XtPointer clientData, XtPointer callData);
static void reapplyLanguageMode(WindowInfo *window, int mode,int forceDefaults);
static void fillFromPrimaryCB(Widget w, XtPointer clientData,
    	XtPointer callData);
static int checkFontStatus(fontDialog *fd, Widget fontTextFieldW);
static int showFontStatus(fontDialog *fd, Widget fontTextFieldW,
    	Widget errorLabelW);
static void primaryModifiedCB(Widget w, XtPointer clientData,
	XtPointer callData);
static void italicModifiedCB(Widget w, XtPointer clientData,
    	XtPointer callData);
static void boldModifiedCB(Widget w, XtPointer clientData, XtPointer callData);
static void boldItalicModifiedCB(Widget w, XtPointer clientData,
    	XtPointer callData);
static void primaryBrowseCB(Widget w, XtPointer clientData, XtPointer callData);
static void italicBrowseCB(Widget w, XtPointer clientData, XtPointer callData);
static void boldBrowseCB(Widget w, XtPointer clientData, XtPointer callData);
static void boldItalicBrowseCB(Widget w, XtPointer clientData,
    	XtPointer callData);
static void browseFont(Widget parent, Widget fontTextW);
static void fontDestroyCB(Widget w, XtPointer clientData, XtPointer callData);
static void fontOkCB(Widget w, XtPointer clientData, XtPointer callData);
static void fontApplyCB(Widget w, XtPointer clientData, XtPointer callData);
static void fontDismissCB(Widget w, XtPointer clientData, XtPointer callData);
static void updateFonts(fontDialog *fd);
static int matchLanguageMode(WindowInfo *window);
static int loadLanguageModesString(char *inString);
static char *writeLanguageModesString(void);
static char *createExtString(char **extensions, int nExtensions);
static char **readExtensionList(char **inPtr, int *nExtensions);
static void updateLanguageModeSubmenu(WindowInfo *window);
static void setLangModeCB(Widget w, XtPointer clientData, XtPointer callData);
static int modeError(languageModeRec *lm, const char *stringStart,
	const char *stoppedAt, const char *message);
static void lmDestroyCB(Widget w, XtPointer clientData, XtPointer callData);
static void lmOkCB(Widget w, XtPointer clientData, XtPointer callData);
static void lmApplyCB(Widget w, XtPointer clientData, XtPointer callData);
static void lmDismissCB(Widget w, XtPointer clientData, XtPointer callData);
static int lmDeleteConfirmCB(int itemIndex, void *cbArg);
static int updateLMList(void);
static languageModeRec *copyLanguageModeRec(languageModeRec *lm);
static void *lmGetDisplayedCB(void *oldItem, int explicitRequest, int *abort,
    	void *cbArg);
static void lmSetDisplayedCB(void *item, void *cbArg);
static languageModeRec *readLMDialogFields(int silent);
static void lmFreeItemCB(void *item);
static void freeLanguageModeRec(languageModeRec *lm);
static int lmDialogEmpty(void);
static void updatePatternsTo5dot1(void);
static void spliceString(char **intoString, char *insertString, char *atExpr);
static int regexFind(const char *inString, const char *expr);
static int regexReplace(char **inString, const char *expr,
	const char *replaceWith);

#ifdef SGI_CUSTOM
static int shortPrefToDefault(Widget parent, char *settingName, int *setDefault);
#endif

XrmDatabase CreateNEditPrefDB(int *argcInOut, char **argvInOut)
{
    return CreatePreferencesDatabase(PREF_FILE_NAME, APP_NAME, 
	    OpTable, XtNumber(OpTable), (unsigned int *)argcInOut, argvInOut);
}
    
void RestoreNEditPrefs(XrmDatabase prefDB, XrmDatabase appDB)
{
    int requiresConversion;
    
    /* Load preferences */
    RestorePreferences(prefDB, appDB, APP_NAME,
    	    APP_CLASS, PrefDescrip, XtNumber(PrefDescrip));
    
    /* If the preferences file was written by an older version of NEdit,
       warn the user that it will be converted. */
    requiresConversion = PrefData.prefFileRead &&
    	    PrefData.fileVersion[0] == '\0';
    if (requiresConversion) {
	fprintf(stderr, "NEdit: Converting .nedit file from old version.\n"
		"    To update, use Preferences -> Save Defaults\n");
	updatePatternsTo5dot1();
    }
     
    /* Do further parsing on resource types which RestorePreferences does
       not understand and reads as strings, to put them in the final form
       in which nedit stores and uses.  If the preferences file was
       written by an older version of NEdit, update regular expressions in
       highlight patterns to quote braces and use & instead of \0 */
    translatePrefFormats(requiresConversion);
}

/*
** Many of of NEdit's preferences are much more complicated than just simple
** integers or strings.  These are read as strings, but must be parsed and
** translated into something meaningful.  This routine does the translation,
** and, in most cases, frees the original string, which is no longer useful.
**
** The argument convertOld attempts a conversion from pre 5.1 format .nedit
** files (which means patterns and macros may contain regular expressions
** which are of the older syntax where braces were not quoted, and \0 was a
** legal substitution character).  Macros, so far can not be automatically
** converted, unfortunately.
*/
static void translatePrefFormats(int convertOld)
{
    XFontStruct *font;

    /* Parse the strings which represent types which are not decoded by
       the standard resource manager routines */
#ifndef VMS
    if (TempStringPrefs.shellCmds != NULL) {
	LoadShellCmdsString(TempStringPrefs.shellCmds);
	XtFree(TempStringPrefs.shellCmds);
	TempStringPrefs.shellCmds = NULL;
    }
#endif /* VMS */
    if (TempStringPrefs.macroCmds != NULL) {
	LoadMacroCmdsString(TempStringPrefs.macroCmds);
    	XtFree(TempStringPrefs.macroCmds);
	TempStringPrefs.macroCmds = NULL;
    }
    if (TempStringPrefs.bgMenuCmds != NULL) {
	LoadBGMenuCmdsString(TempStringPrefs.bgMenuCmds);
    	XtFree(TempStringPrefs.bgMenuCmds);
	TempStringPrefs.bgMenuCmds = NULL;
    }
    if (TempStringPrefs.highlight != NULL) {
	LoadHighlightString(TempStringPrefs.highlight, convertOld);
    	XtFree(TempStringPrefs.highlight);
	TempStringPrefs.highlight = NULL;
    }
    if (TempStringPrefs.styles != NULL) {
	LoadStylesString(TempStringPrefs.styles);
    	XtFree(TempStringPrefs.styles);
	TempStringPrefs.styles = NULL;
    }
    if (TempStringPrefs.language != NULL) {
	loadLanguageModesString(TempStringPrefs.language);
    	XtFree(TempStringPrefs.language);
	TempStringPrefs.language = NULL;
    }
    if (TempStringPrefs.smartIndent != NULL) {
	LoadSmartIndentString(TempStringPrefs.smartIndent);
    	XtFree(TempStringPrefs.smartIndent);
	TempStringPrefs.smartIndent = NULL;
    }
    if (TempStringPrefs.smartIndentCommon != NULL) {
	LoadSmartIndentCommonString(TempStringPrefs.smartIndentCommon);
	XtFree(TempStringPrefs.smartIndentCommon);
	TempStringPrefs.smartIndentCommon = NULL;
    }
    
    /* translate the font names into fontLists suitable for the text widget */
    font = XLoadQueryFont(TheDisplay, PrefData.fontString);
    PrefData.fontList = font==NULL ? NULL :
	    XmFontListCreate(font, XmSTRING_DEFAULT_CHARSET);
    PrefData.boldFontStruct = XLoadQueryFont(TheDisplay,
    	    PrefData.boldFontString);
    PrefData.italicFontStruct = XLoadQueryFont(TheDisplay,
    	    PrefData.italicFontString);
    PrefData.boldItalicFontStruct = XLoadQueryFont(TheDisplay,
    	    PrefData.boldItalicFontString);
    
    /* For compatability with older (4.0.3 and before) versions, the autoWrap
       and autoIndent resources can accept values of True and False.  Translate
       them into acceptable wrap and indent styles */
    if (PrefData.wrapStyle == 3) PrefData.wrapStyle = NEWLINE_WRAP;
    if (PrefData.wrapStyle == 4) PrefData.wrapStyle = NO_WRAP;
    if (PrefData.autoIndent == 3) PrefData.autoIndent = AUTO_INDENT;
    if (PrefData.autoIndent == 4) PrefData.autoIndent = NO_AUTO_INDENT;
}

void SaveNEditPrefs(Widget parent, int quietly)
{
    if (!quietly) {
	if (DialogF(DF_INF, parent, 2, ImportedFile == NULL ?
"Default preferences will be saved in the .nedit file\n\
in your home directory.  NEdit automatically loads\n\
this file each time it is started." :
"Default preferences will be saved in the .nedit\n\
file in your home directory.\n\n\
SAVING WILL INCORPORATE SETTINGS\n\
FROM FILE: %s", "OK", "Cancel", ImportedFile) == 2)
	    return;
    }    
#ifndef VMS
    TempStringPrefs.shellCmds = WriteShellCmdsString();
#endif /* VMS */
    TempStringPrefs.macroCmds = WriteMacroCmdsString();
    TempStringPrefs.bgMenuCmds = WriteBGMenuCmdsString();
    TempStringPrefs.highlight = WriteHighlightString();
    TempStringPrefs.language = writeLanguageModesString();
    TempStringPrefs.styles = WriteStylesString();
    TempStringPrefs.smartIndent = WriteSmartIndentString();
    TempStringPrefs.smartIndentCommon = WriteSmartIndentCommonString();
    strcpy(PrefData.fileVersion, "5.1");
    if (!SavePreferences(XtDisplay(parent), PREF_FILE_NAME, HeaderText,
    	    PrefDescrip, XtNumber(PrefDescrip)))
    	DialogF(DF_WARN, parent, 1,
#ifdef VMS
    		"Unable to save preferences in SYS$LOGIN:.NEDIT", "Dismiss");
#else
    		"Unable to save preferences in $HOME/.nedit", "Dismiss");
#endif /*VMS*/

#ifndef VMS
    XtFree(TempStringPrefs.shellCmds);
#endif /* VMS */
    XtFree(TempStringPrefs.macroCmds);
    XtFree(TempStringPrefs.bgMenuCmds);
    XtFree(TempStringPrefs.highlight);
    XtFree(TempStringPrefs.language);
    XtFree(TempStringPrefs.styles);
    XtFree(TempStringPrefs.smartIndent);
    XtFree(TempStringPrefs.smartIndentCommon);
    
    PrefsHaveChanged = False;
}

/*
** Load an additional preferences file on top of the existing preferences
** derived from defaults, the .nedit file, and X resources.
*/
void ImportPrefFile(const char *filename, int convertOld)
{
    XrmDatabase db;
    
    if ((db = XrmGetFileDatabase(filename)) != NULL)
    	{		
     	 OverlayPreferences(db, APP_NAME, APP_CLASS, PrefDescrip,
    	    	XtNumber(PrefDescrip));
       translatePrefFormats(convertOld);
       ImportedFile = XtNewString(filename);
      } else
      {
       fprintf(stderr, "Could not open additional preferences file: ");
       fprintf(stderr, filename);
       fprintf(stderr, "\n");
	}
}

void SetPrefWrap(int state)
{
    setIntPref(&PrefData.wrapStyle, state);
}

int GetPrefWrap(int langMode)
{
    if (langMode == PLAIN_LANGUAGE_MODE ||
	    LanguageModes[langMode]->wrapStyle == DEFAULT_WRAP)
    	return PrefData.wrapStyle;
    return LanguageModes[langMode]->wrapStyle;
}

void SetPrefWrapMargin(int margin)
{
    setIntPref(&PrefData.wrapMargin, margin);
}

int GetPrefWrapMargin(void)
{
    return PrefData.wrapMargin;
}

void SetPrefSearch(int searchType)
{
    setIntPref(&PrefData.searchMethod, searchType);
}

int GetPrefSearch(void)
{
    return PrefData.searchMethod;
}

#ifdef REPLACE_SCOPE
void SetPrefReplaceDefScope(int scope)
{
    setIntPref(&PrefData.replaceDefScope, scope);
}

int GetPrefReplaceDefScope(void)
{
    return PrefData.replaceDefScope;
}
#endif

void SetPrefAutoIndent(int state)
{
    setIntPref(&PrefData.autoIndent, state);
}

int GetPrefAutoIndent(int langMode)
{
    if (langMode == PLAIN_LANGUAGE_MODE ||
	    LanguageModes[langMode]->indentStyle == DEFAULT_INDENT)
    	return PrefData.autoIndent;
    return LanguageModes[langMode]->indentStyle;
}

void SetPrefAutoSave(int state)
{
    setIntPref(&PrefData.autoSave, state);
}

int GetPrefAutoSave(void)
{
    return PrefData.autoSave;
}

void SetPrefSaveOldVersion(int state)
{
    setIntPref(&PrefData.saveOldVersion, state);
}

int GetPrefSaveOldVersion(void)
{
    return PrefData.saveOldVersion;
}

void SetPrefSearchDlogs(int state)
{
    setIntPref(&PrefData.searchDlogs, state);
}

int GetPrefSearchDlogs(void)
{
    return PrefData.searchDlogs;
}

void SetPrefBeepOnSearchWrap(int state)
{
    setIntPref(&PrefData.searchWrapBeep, state);
}

int GetPrefBeepOnSearchWrap(void)
{
    return PrefData.searchWrapBeep;
}

void SetPrefKeepSearchDlogs(int state)
{
    setIntPref(&PrefData.keepSearchDlogs, state);
}

int GetPrefKeepSearchDlogs(void)
{
    return PrefData.keepSearchDlogs;
}

void SetPrefSearchWraps(int state)
{
    setIntPref(&PrefData.searchWraps, state);
}

int GetPrefStickyCaseSenseBtn(void)
{
    return PrefData.stickyCaseSenseBtn;
}

int GetPrefSearchWraps(void)
{
    return PrefData.searchWraps;
}

void SetPrefStatsLine(int state)
{
    setIntPref(&PrefData.statsLine, state);
}

int GetPrefStatsLine(void)
{
    return PrefData.statsLine;
}

void SetPrefISearchLine(int state)
{
    setIntPref(&PrefData.iSearchLine, state);
}

int GetPrefISearchLine(void)
{
    return PrefData.iSearchLine;
}

void SetPrefLineNums(int state)
{
    setIntPref(&PrefData.lineNums, state);
}

int GetPrefLineNums(void)
{
    return PrefData.lineNums;
}

void SetPrefShowPathInWindowsMenu(int state)
{
    setIntPref(&PrefData.pathInWindowsMenu, state);
}

int GetPrefShowPathInWindowsMenu(void)
{
    return PrefData.pathInWindowsMenu;
}

void SetPrefWarnFileMods(int state)
{
    setIntPref(&PrefData.warnFileMods, state);
}

int GetPrefWarnFileMods(void)
{
    return PrefData.warnFileMods;
}

void SetPrefWarnExit(int state)
{
    setIntPref(&PrefData.warnExit, state);
}

int GetPrefWarnExit(void)
{
    return PrefData.warnExit;
}

void SetPrefFindReplaceUsesSelection(int state)
{
    setIntPref(&PrefData.findReplaceUsesSelection, state);
}

int GetPrefFindReplaceUsesSelection(void)
{
    return PrefData.findReplaceUsesSelection;
}

void SetPrefMapDelete(int state)
{
    setIntPref(&PrefData.mapDelete, state);
}

int GetPrefMapDelete(void)
{
    return PrefData.mapDelete;
}

void SetPrefStdOpenDialog(int state)
{
    setIntPref(&PrefData.stdOpenDialog, state);
}

int GetPrefStdOpenDialog(void)
{
    return PrefData.stdOpenDialog;
}

void SetPrefRows(int nRows)
{
    setIntPref(&PrefData.textRows, nRows);
}

int GetPrefRows(void)
{
    return PrefData.textRows;
}

void SetPrefCols(int nCols)
{
   setIntPref(&PrefData.textCols, nCols);
}

int GetPrefCols(void)
{
    return PrefData.textCols;
}

void SetPrefTabDist(int tabDist)
{
    setIntPref(&PrefData.tabDist, tabDist);
}

int GetPrefTabDist(int langMode)
{
    if (langMode == PLAIN_LANGUAGE_MODE ||
	    LanguageModes[langMode]->tabDist == DEFAULT_TAB_DIST)
	return PrefData.tabDist;
    return LanguageModes[langMode]->tabDist;
 }

void SetPrefEmTabDist(int tabDist)
{
    setIntPref(&PrefData.emTabDist, tabDist);
}

int GetPrefEmTabDist(int langMode)
{
    if (langMode == PLAIN_LANGUAGE_MODE ||
	    LanguageModes[langMode]->emTabDist == DEFAULT_EM_TAB_DIST)
	return PrefData.emTabDist;
    return LanguageModes[langMode]->emTabDist;
}

void SetPrefInsertTabs(int state)
{
    setIntPref(&PrefData.insertTabs, state);
}

int GetPrefInsertTabs(void)
{
    return PrefData.insertTabs;
}

void SetPrefShowMatching(int state)
{
    setIntPref(&PrefData.showMatchingStyle, state);
}

int GetPrefShowMatching(void)
{
    /*
     * For backwards compatibility with pre-5.2 versions, the boolean 
     * False/True matching behavior is converted to NO_FLASH/FLASH_DELIMIT. 
     */
    if (PrefData.showMatchingStyle >= N_SHOW_MATCHING_STYLES) 
	PrefData.showMatchingStyle -= N_SHOW_MATCHING_STYLES;
    return PrefData.showMatchingStyle;
}

void SetPrefHighlightSyntax(int state)
{
    setIntPref(&PrefData.highlightSyntax, state);
}

int GetPrefHighlightSyntax(void)
{
    return PrefData.highlightSyntax;
}

void SetPrefRepositionDialogs(int state)
{
    setIntPref(&PrefData.repositionDialogs, state);
}

int GetPrefRepositionDialogs(void)
{
    return PrefData.repositionDialogs;
}

void SetPrefSortOpenPrevMenu(int state)
{
    setIntPref(&PrefData.sortOpenPrevMenu, state);
}

int GetPrefSortOpenPrevMenu(void)
{
    return PrefData.sortOpenPrevMenu;
}

void SetPrefTagFile(const char *tagFileName)
{
    setStringPref(PrefData.tagFile, tagFileName);
}

char *GetPrefTagFile(void)
{
    return PrefData.tagFile;
}

void SetPrefSmartTags(int state)
{
    setIntPref(&PrefData.smartTags, state);
}

int GetPrefSmartTags(void)
{
    return PrefData.smartTags;
}

char *GetPrefDelimiters(void)
{
    return PrefData.delimiters;
}

/*
** Set the font preferences using the font name (the fontList is generated
** in this call).  Note that this leaks memory and server resources each
** time the default font is re-set.  See note on SetFontByName in window.c
** for more information.
*/
void SetPrefFont(char *fontName)
{
    XFontStruct *font;
    
    setStringPref(PrefData.fontString, fontName);
    font = XLoadQueryFont(TheDisplay, fontName);
    PrefData.fontList = font==NULL ? NULL :
	    XmFontListCreate(font, XmSTRING_DEFAULT_CHARSET);
}
void SetPrefBoldFont(char *fontName)
{
    setStringPref(PrefData.boldFontString, fontName);
    PrefData.boldFontStruct = XLoadQueryFont(TheDisplay, fontName);
}
void SetPrefItalicFont(char *fontName)
{
    setStringPref(PrefData.italicFontString, fontName);
    PrefData.italicFontStruct = XLoadQueryFont(TheDisplay, fontName);
}
void SetPrefBoldItalicFont(char *fontName)
{
    setStringPref(PrefData.boldItalicFontString, fontName);
    PrefData.boldItalicFontStruct = XLoadQueryFont(TheDisplay, fontName);
}

char *GetPrefFontName(void)
{
    return PrefData.fontString;
}
char *GetPrefBoldFontName(void)
{
    return PrefData.boldFontString;
}
char *GetPrefItalicFontName(void)
{
    return PrefData.italicFontString;
}
char *GetPrefBoldItalicFontName(void)
{
    return PrefData.boldItalicFontString;
}

XmFontList GetPrefFontList(void)
{
    return PrefData.fontList;
}
XFontStruct *GetPrefBoldFont(void)
{
    return PrefData.boldFontStruct;
}
XFontStruct *GetPrefItalicFont(void)
{
    return PrefData.italicFontStruct;
}
XFontStruct *GetPrefBoldItalicFont(void)
{
    return PrefData.boldItalicFontStruct;
}

void SetPrefShell(const char *shell)
{
    setStringPref(PrefData.shell, shell);
}

char *GetPrefShell(void)
{
    return PrefData.shell;
}

void SetPrefGeometry(const char *geometry)
{
    setStringPref(PrefData.geometry, geometry);
}

char *GetPrefGeometry(void)
{
    return PrefData.geometry;
}

char *GetPrefServerName(void)
{
    return PrefData.serverName;
}

char *GetPrefBGMenuBtn(void)
{
    return PrefData.bgMenuBtn;
}

int GetPrefMaxPrevOpenFiles(void)
{
    return PrefData.maxPrevOpenFiles;
}

#ifdef SGI_CUSTOM
void SetPrefShortMenus(int state)
{
    setIntPref(&PrefData.shortMenus, state);
}

int GetPrefShortMenus(void)
{
    return PrefData.shortMenus;
}
#endif

/*
** If preferences don't get saved, ask the user on exit whether to save
*/
void MarkPrefsChanged(void)
{
    PrefsHaveChanged = True;
}

/*
** Check if preferences have changed, and if so, ask the user if he wants
** to re-save.  Returns False if user requests cancelation of Exit (or whatever
** operation triggered this call to be made).
*/
int CheckPrefsChangesSaved(Widget dialogParent)
{
    int resp;
    
    if (!PrefsHaveChanged)
	return True;
    
    resp = DialogF(DF_WARN, dialogParent, 3, ImportedFile == NULL ?
    	    "Default Preferences have changed.\nSave changes to .nedit file?" :
	    "Default Preferences have changed.  SAVING \n\
CHANGES WILL INCORPORATE ADDITIONAL\nSETTINGS FROM FILE: %s",
	    "Save", "Don't Save", "Cancel", ImportedFile);
    if (resp == 2)
	return True;
    if (resp == 3)
	return False;
    
    SaveNEditPrefs(dialogParent, True);
    return True;
}

/*
** set *prefDataField to newValue, but first check if they're different
** and update PrefsHaveChanged if a preference setting has now changed.
*/
static void setIntPref(int *prefDataField, int newValue)
{
    if (newValue != *prefDataField)
	PrefsHaveChanged = True;
    *prefDataField = newValue;
}

static void setStringPref(char *prefDataField, const char *newValue)
{
    if (strcmp(prefDataField, newValue))
	PrefsHaveChanged = True;
    strcpy(prefDataField, newValue);
}


/*
** Set the language mode for the window, update the menu and trigger language
** mode specific actions (turn on/off highlighting).  If forceNewDefaults is
** true, re-establish default settings for language-specific preferences
** regardless of whether they were previously set by the user.
*/
void SetLanguageMode(WindowInfo *window, int mode, int forceNewDefaults)
{
    Widget menu;
    WidgetList items;
    int n;
    Cardinal nItems;
    void *userData;
    
    /* Do mode-specific actions */
    reapplyLanguageMode(window, mode, forceNewDefaults);
    
    /* Select the correct language mode in the sub-menu */
    XtVaGetValues(window->langModeCascade, XmNsubMenuId, &menu, NULL);
    XtVaGetValues(menu, XmNchildren, &items, XmNnumChildren, &nItems, NULL);
    for (n=0; n<(int)nItems; n++) {
    	XtVaGetValues(items[n], XmNuserData, &userData, NULL);
    	XmToggleButtonSetState(items[n], (int)userData == mode, False);
    }
}

/*
** Lookup a language mode by name, returning the index of the language
** mode or PLAIN_LANGUAGE_MODE if the name is not found
*/
int FindLanguageMode(const char *languageName)
{
    int i;
 
    /* Compare each language mode to the one we were presented */
    for (i=0; i<NLanguageModes; i++)
	if (!strcmp(languageName, LanguageModes[i]->name))
	    return i;

    return PLAIN_LANGUAGE_MODE;
}


/*
** Apply language mode matching criteria and set window->languageMode to
** the appropriate mode for the current file, trigger language mode
** specific actions (turn on/off highlighting), and update the language
** mode menu item.  If forceNewDefaults is true, re-establish default
** settings for language-specific preferences regardless of whether
** they were previously set by the user.
*/
void DetermineLanguageMode(WindowInfo *window, int forceNewDefaults)
{
    SetLanguageMode(window, matchLanguageMode(window), forceNewDefaults);
}

/*
** Return the name of the current language mode set in "window", or NULL
** if the current mode is "Plain".
*/
char *LanguageModeName(int mode)
{
    if (mode == PLAIN_LANGUAGE_MODE)
    	return NULL;
    else
    	return LanguageModes[mode]->name;
}

/*
** Get the set of word delimiters for the language mode set in the current
** window.  Returns NULL when no language mode is set (it would be easy to
** return the default delimiter set when the current language mode is "Plain",
** or the mode doesn't have its own delimiters, but this is usually used
** to supply delimiters for RE searching, and ExecRE can skip compiling a
** delimiter table when delimiters is NULL).
*/
char *GetWindowDelimiters(WindowInfo *window)
{
    if (window->languageMode == PLAIN_LANGUAGE_MODE)
    	return NULL;
    else
    	return LanguageModes[window->languageMode]->delimiters;
}

/*
** Put up a dialog for selecting a custom initial window size
*/
void RowColumnPrefDialog(Widget parent)
{
    Widget form, selBox, topLabel;
    Arg selBoxArgs[2];
    XmString s1;

    XtSetArg(selBoxArgs[0], XmNdialogStyle, XmDIALOG_FULL_APPLICATION_MODAL);
    XtSetArg(selBoxArgs[1], XmNautoUnmanage, False);
    selBox = CreatePromptDialog(parent, "customSize", selBoxArgs, 2);
    XtAddCallback(selBox, XmNokCallback, (XtCallbackProc)sizeOKCB, NULL);
    XtAddCallback(selBox, XmNcancelCallback, (XtCallbackProc)sizeCancelCB,NULL);
    XtUnmanageChild(XmSelectionBoxGetChild(selBox, XmDIALOG_TEXT));
    XtUnmanageChild(XmSelectionBoxGetChild(selBox, XmDIALOG_SELECTION_LABEL));
    XtUnmanageChild(XmSelectionBoxGetChild(selBox, XmDIALOG_HELP_BUTTON));
    XtVaSetValues(XtParent(selBox), XmNtitle, "Initial Window Size", NULL);
    
    form = XtVaCreateManagedWidget("form", xmFormWidgetClass, selBox, NULL);

    topLabel = XtVaCreateManagedWidget("topLabel", xmLabelGadgetClass, form,
    	    XmNlabelString, s1=MKSTRING(
    	       "Enter desired size in rows\nand columns of characters:"), NULL);
    XmStringFree(s1);
 
    RowText = XtVaCreateManagedWidget("rows", xmTextWidgetClass, form,
    	    XmNcolumns, 3,
    	    XmNtopAttachment, XmATTACH_WIDGET,
    	    XmNleftAttachment, XmATTACH_POSITION,
    	    XmNrightAttachment, XmATTACH_POSITION,
    	    XmNtopWidget, topLabel,
    	    XmNleftPosition, 5,
    	    XmNrightPosition, 45, NULL);
    RemapDeleteKey(RowText);
 
    XtVaCreateManagedWidget("xLabel", xmLabelGadgetClass, form,
    	    XmNlabelString, s1=MKSTRING("x"),
    	    XmNtopAttachment, XmATTACH_WIDGET,
    	    XmNleftAttachment, XmATTACH_POSITION,
    	    XmNrightAttachment, XmATTACH_POSITION,
    	    XmNbottomAttachment, XmATTACH_OPPOSITE_WIDGET,
    	    XmNtopWidget, topLabel,
    	    XmNbottomWidget, RowText,
    	    XmNleftPosition, 45,
    	    XmNrightPosition, 55, NULL);
    XmStringFree(s1);

    ColText = XtVaCreateManagedWidget("cols", xmTextWidgetClass, form,
    	    XmNcolumns, 3,
    	    XmNtopAttachment, XmATTACH_WIDGET,
    	    XmNleftAttachment, XmATTACH_POSITION,
    	    XmNrightAttachment, XmATTACH_POSITION,
    	    XmNtopWidget, topLabel,
    	    XmNleftPosition, 55,
    	    XmNrightPosition, 95, NULL);
    RemapDeleteKey(ColText);

    /* put up dialog and wait for user to press ok or cancel */
    DoneWithSizeDialog = False;
    ManageDialogCenteredOnPointer(selBox);
    while (!DoneWithSizeDialog)
        XtAppProcessEvent (XtWidgetToApplicationContext(parent), XtIMAll);
    
    XtDestroyWidget(selBox);
}

static void sizeOKCB(Widget w, XtPointer clientData, XtPointer callData)
{
    int rowValue, colValue, stat;
    
    /* get the values that the user entered and make sure they're ok */
    stat = GetIntTextWarn(RowText, &rowValue, "number of rows", True);
    if (stat != TEXT_READ_OK)
    	return;
    stat = GetIntTextWarn(ColText, &colValue, "number of columns", True);
    if (stat != TEXT_READ_OK)
    	return;
    
    /* set the corresponding preferences and dismiss the dialog */
    SetPrefRows(rowValue);
    SetPrefCols(colValue);
    DoneWithSizeDialog = True;
}

static void sizeCancelCB(Widget w, XtPointer clientData, XtPointer callData)
{
    DoneWithSizeDialog = True;
}

/*
** Present the user a dialog for setting tab related preferences, either as
** defaults, or for a specific window (pass "forWindow" as NULL to set default
** preference, or a window to set preferences for the specific window.
*/
void TabsPrefDialog(Widget parent, WindowInfo *forWindow)
{
    Widget form, selBox;
    Arg selBoxArgs[2];
    XmString s1;
    int emulate, emTabDist, useTabs, tabDist;

    XtSetArg(selBoxArgs[0], XmNdialogStyle, XmDIALOG_FULL_APPLICATION_MODAL);
    XtSetArg(selBoxArgs[1], XmNautoUnmanage, False);
    selBox = CreatePromptDialog(parent, "customSize", selBoxArgs, 2);
    XtAddCallback(selBox, XmNokCallback, (XtCallbackProc)tabsOKCB, NULL);
    XtAddCallback(selBox, XmNcancelCallback, (XtCallbackProc)tabsCancelCB,NULL);
    XtAddCallback(selBox, XmNhelpCallback, (XtCallbackProc)tabsHelpCB,NULL);
    XtUnmanageChild(XmSelectionBoxGetChild(selBox, XmDIALOG_TEXT));
    XtUnmanageChild(XmSelectionBoxGetChild(selBox, XmDIALOG_SELECTION_LABEL));
    XtVaSetValues(XtParent(selBox), XmNtitle, "Tabs", NULL);
    
    form = XtVaCreateManagedWidget("form", xmFormWidgetClass, selBox, NULL);

    TabDistText = XtVaCreateManagedWidget("tabDistText", xmTextWidgetClass,
    	    form, XmNcolumns, 7,
    	    XmNtopAttachment, XmATTACH_FORM,
    	    XmNrightAttachment, XmATTACH_FORM, NULL);
    RemapDeleteKey(TabDistText);
    XtVaCreateManagedWidget("tabDistLabel", xmLabelGadgetClass, form,
    	    XmNlabelString, s1=XmStringCreateSimple(
    	    	"Tab spacing (for hardware tab characters)"),
	    XmNmnemonic, 'T',
    	    XmNuserData, TabDistText,
    	    XmNtopAttachment, XmATTACH_FORM,
    	    XmNleftAttachment, XmATTACH_FORM,
    	    XmNrightAttachment, XmATTACH_WIDGET,
    	    XmNrightWidget, TabDistText,
	    XmNbottomAttachment, XmATTACH_OPPOSITE_WIDGET,
	    XmNbottomWidget, TabDistText, NULL);
    XmStringFree(s1);
 
    EmTabText = XtVaCreateManagedWidget("emTabText", xmTextWidgetClass, form,
    	    XmNcolumns, 7,
    	    XmNtopAttachment, XmATTACH_WIDGET,
    	    XmNtopWidget, TabDistText,
    	    XmNrightAttachment, XmATTACH_OPPOSITE_WIDGET,
    	    XmNrightWidget, TabDistText, NULL);
    RemapDeleteKey(EmTabText);
    EmTabLabel = XtVaCreateManagedWidget("emTabLabel", xmLabelGadgetClass, form,
    	    XmNlabelString, s1=XmStringCreateSimple("Emulated tab spacing"),
	    XmNmnemonic, 's',
    	    XmNuserData, EmTabText,
    	    XmNtopAttachment, XmATTACH_WIDGET,
    	    XmNtopWidget, TabDistText,
    	    XmNrightAttachment, XmATTACH_WIDGET,
    	    XmNrightWidget, EmTabText,
    	    XmNbottomAttachment, XmATTACH_OPPOSITE_WIDGET,
	    XmNbottomWidget, EmTabText, NULL);
    XmStringFree(s1);
    EmTabToggle = XtVaCreateManagedWidget("emTabToggle",
    	    xmToggleButtonWidgetClass, form, XmNlabelString,
    	    	s1=XmStringCreateSimple("Emulate tabs"),
	    XmNmnemonic, 'E',
    	    XmNtopAttachment, XmATTACH_WIDGET,
    	    XmNtopWidget, TabDistText,
    	    XmNleftAttachment, XmATTACH_FORM,
    	    XmNbottomAttachment, XmATTACH_OPPOSITE_WIDGET,
	    XmNbottomWidget, EmTabText, NULL);
    XmStringFree(s1);
    XtAddCallback(EmTabToggle, XmNvalueChangedCallback, emTabsCB, NULL);
    UseTabsToggle = XtVaCreateManagedWidget("useTabsToggle",
    	    xmToggleButtonWidgetClass, form,
    	    XmNlabelString, s1=XmStringCreateSimple(
    	    	"Use tab characters in padding and emulated tabs"),
	    XmNmnemonic, 'U',
    	    XmNtopAttachment, XmATTACH_WIDGET,
    	    XmNtopWidget, EmTabText,
    	    XmNtopOffset, 5,
    	    XmNleftAttachment, XmATTACH_FORM, NULL);
    XmStringFree(s1);

    /* Set default values */
    if (forWindow == NULL) {
    	emTabDist = GetPrefEmTabDist(PLAIN_LANGUAGE_MODE);
    	useTabs = GetPrefInsertTabs();
    	tabDist = GetPrefTabDist(PLAIN_LANGUAGE_MODE);
    } else {
    	XtVaGetValues(forWindow->textArea, textNemulateTabs, &emTabDist, NULL);
    	useTabs = forWindow->buffer->useTabs;
    	tabDist = BufGetTabDistance(forWindow->buffer);
    }
    emulate = emTabDist != 0;
    SetIntText(TabDistText, tabDist);
    XmToggleButtonSetState(EmTabToggle, emulate, True);
    if (emulate)
    	SetIntText(EmTabText, emTabDist);
    XmToggleButtonSetState(UseTabsToggle, useTabs, False);
    XtSetSensitive(EmTabText, emulate);
    XtSetSensitive(EmTabLabel, emulate);
    
    /* Handle mnemonic selection of buttons and focus to dialog */
    AddDialogMnemonicHandler(form, FALSE);

    /* Set the widget to get focus */
#if XmVersion >= 1002
    XtVaSetValues(form, XmNinitialFocus, TabDistText, NULL);
#endif
    
    /* put up dialog and wait for user to press ok or cancel */
    TabsDialogForWindow = forWindow;
    DoneWithTabsDialog = False;
    ManageDialogCenteredOnPointer(selBox);
    while (!DoneWithTabsDialog)
        XtAppProcessEvent(XtWidgetToApplicationContext(parent), XtIMAll);
    
    XtDestroyWidget(selBox);
}

static void tabsOKCB(Widget w, XtPointer clientData, XtPointer callData)
{
    int emulate, useTabs, stat, tabDist, emTabDist;
    WindowInfo *window = TabsDialogForWindow;
    
    /* get the values that the user entered and make sure they're ok */
    emulate = XmToggleButtonGetState(EmTabToggle);
    useTabs = XmToggleButtonGetState(UseTabsToggle);
    stat = GetIntTextWarn(TabDistText, &tabDist, "tab spacing", True);
    if (stat != TEXT_READ_OK)
    	return;
    if (tabDist <= 0 || tabDist > MAX_EXP_CHAR_LEN) {
    	DialogF(DF_WARN, TabDistText, 1, "Tab spacing out of range", "Dismiss");
    	return;
    }
    if (emulate) {
	stat = GetIntTextWarn(EmTabText, &emTabDist, "emulated tab spacing",True);
	if (stat != TEXT_READ_OK)
	    return;
	if (emTabDist <= 0 || tabDist >= 1000) {
	    DialogF(DF_WARN, EmTabText, 1, "Emulated tab spacing out of range",
	    	    "Dismiss");
	    return;
	}
    } else
    	emTabDist = 0;
    
#ifdef SGI_CUSTOM
    /* Ask the user about saving as a default preference */
    if (TabsDialogForWindow != NULL) {
	int setDefault;
	if (!shortPrefToDefault(window->shell, "Tab Settings", &setDefault)) {
	    DoneWithTabsDialog = True;
    	    return;
	}
	if (setDefault) {
    	    SetPrefTabDist(tabDist);
    	    SetPrefEmTabDist(emTabDist);
    	    SetPrefInsertTabs(useTabs);
	    SaveNEditPrefs(window->shell, GetPrefShortMenus());
	}
    }
#endif

    /* Set the value in either the requested window or default preferences */
    if (TabsDialogForWindow == NULL) {
    	SetPrefTabDist(tabDist);
    	SetPrefEmTabDist(emTabDist);
    	SetPrefInsertTabs(useTabs);
    } else {
        char *params[1];
        char numStr[25];

        params[0] = numStr;
        sprintf(numStr, "%d", tabDist);
        XtCallActionProc(window->textArea, "set_tab_dist", NULL, params, 1);
        params[0] = numStr;
        sprintf(numStr, "%d", emTabDist);
        XtCallActionProc(window->textArea, "set_em_tab_dist", NULL, params, 1);
        params[0] = numStr;
        sprintf(numStr, "%d", useTabs);
        XtCallActionProc(window->textArea, "set_use_tabs", NULL, params, 1);
/*
    	setTabDist(window, tabDist);
    	setEmTabDist(window, emTabDist);
       	window->buffer->useTabs = useTabs;
*/
    }
    DoneWithTabsDialog = True;
}

static void tabsCancelCB(Widget w, XtPointer clientData, XtPointer callData)
{
    DoneWithTabsDialog = True;
}

static void tabsHelpCB(Widget w, XtPointer clientData, XtPointer callData)
{
    Help(XtParent(EmTabLabel), HELP_TABS_DIALOG);
}

static void emTabsCB(Widget w, XtPointer clientData, XtPointer callData)
{
    int state = XmToggleButtonGetState(w);
    
    XtSetSensitive(EmTabLabel, state);
    XtSetSensitive(EmTabText, state);
}

/*
** Present the user a dialog for setting wrap margin.
*/
void WrapMarginDialog(Widget parent, WindowInfo *forWindow)
{
    Widget form, selBox;
    Arg selBoxArgs[2];
    XmString s1;
    int margin;

    XtSetArg(selBoxArgs[0], XmNdialogStyle, XmDIALOG_FULL_APPLICATION_MODAL);
    XtSetArg(selBoxArgs[1], XmNautoUnmanage, False);
    selBox = CreatePromptDialog(parent, "wrapMargin", selBoxArgs, 2);
    XtAddCallback(selBox, XmNokCallback, (XtCallbackProc)wrapOKCB, NULL);
    XtAddCallback(selBox, XmNcancelCallback, (XtCallbackProc)wrapCancelCB,NULL);
    XtUnmanageChild(XmSelectionBoxGetChild(selBox, XmDIALOG_TEXT));
    XtUnmanageChild(XmSelectionBoxGetChild(selBox, XmDIALOG_SELECTION_LABEL));
    XtUnmanageChild(XmSelectionBoxGetChild(selBox, XmDIALOG_HELP_BUTTON));
    XtVaSetValues(XtParent(selBox), XmNtitle, "Wrap Margin", NULL);
    
    form = XtVaCreateManagedWidget("form", xmFormWidgetClass, selBox, NULL);

    WrapWindowToggle = XtVaCreateManagedWidget("wrapWindowToggle",
    	    xmToggleButtonWidgetClass, form, XmNlabelString,
    	    	s1=XmStringCreateSimple("Wrap and Fill at width of window"),
	    XmNmnemonic, 'W',
    	    XmNtopAttachment, XmATTACH_FORM,
    	    XmNleftAttachment, XmATTACH_FORM, NULL);
    XmStringFree(s1);
    XtAddCallback(WrapWindowToggle, XmNvalueChangedCallback, wrapWindowCB,NULL);
    WrapText = XtVaCreateManagedWidget("wrapText", xmTextWidgetClass, form,
    	    XmNcolumns, 5,
    	    XmNtopAttachment, XmATTACH_WIDGET,
    	    XmNtopWidget, WrapWindowToggle,
    	    XmNrightAttachment, XmATTACH_FORM, NULL);
    RemapDeleteKey(WrapText);
    WrapTextLabel = XtVaCreateManagedWidget("wrapMarginLabel",
    	    xmLabelGadgetClass, form,
    	    XmNlabelString, s1=XmStringCreateSimple(
    	    	"Margin for Wrap and Fill"),
	    XmNmnemonic, 'M',
    	    XmNuserData, WrapText,
    	    XmNtopAttachment, XmATTACH_WIDGET,
    	    XmNtopWidget, WrapWindowToggle,
    	    XmNleftAttachment, XmATTACH_FORM,
    	    XmNrightAttachment, XmATTACH_WIDGET,
    	    XmNrightWidget, WrapText,
	    XmNbottomAttachment, XmATTACH_OPPOSITE_WIDGET,
	    XmNbottomWidget, WrapText, NULL);
    XmStringFree(s1);

    /* Set default value */
    if (forWindow == NULL)
    	margin = GetPrefWrapMargin();
    else
    	XtVaGetValues(forWindow->textArea, textNwrapMargin, &margin, NULL);
    XmToggleButtonSetState(WrapWindowToggle, margin==0, True);
    if (margin != 0)
    	SetIntText(WrapText, margin);
    XtSetSensitive(WrapText, margin!=0);
    XtSetSensitive(WrapTextLabel, margin!=0);
    
    /* Handle mnemonic selection of buttons and focus to dialog */
    AddDialogMnemonicHandler(form, FALSE);

    /* put up dialog and wait for user to press ok or cancel */
    WrapDialogForWindow = forWindow;
    DoneWithWrapDialog = False;
    ManageDialogCenteredOnPointer(selBox);
    while (!DoneWithWrapDialog)
        XtAppProcessEvent(XtWidgetToApplicationContext(parent), XtIMAll);
    
    XtDestroyWidget(selBox);
}

static void wrapOKCB(Widget w, XtPointer clientData, XtPointer callData)
{
    int wrapAtWindow, margin, stat;
    WindowInfo *window = WrapDialogForWindow;
    
    /* get the values that the user entered and make sure they're ok */
    wrapAtWindow = XmToggleButtonGetState(WrapWindowToggle);
    if (wrapAtWindow)
    	margin = 0;
    else {
	stat = GetIntTextWarn(WrapText, &margin, "wrap Margin", True);
	if (stat != TEXT_READ_OK)
    	    return;
	if (margin <= 0 || margin >= 1000) {
    	    DialogF(DF_WARN, WrapText, 1, "Wrap margin out of range", "Dismiss");
    	    return;
	}
    }

#ifdef SGI_CUSTOM
    /* Ask the user about saving as a default preference */
    if (WrapDialogForWindow != NULL) {
	int setDefault;
	if (!shortPrefToDefault(window->shell, "Wrap Margin Settings",
	    	&setDefault)) {
	    DoneWithWrapDialog = True;
    	    return;
	}
	if (setDefault) {
    	    SetPrefWrapMargin(margin);
	    SaveNEditPrefs(window->shell, GetPrefShortMenus());
	}
    }
#endif

    /* Set the value in either the requested window or default preferences */
    if (WrapDialogForWindow == NULL)
    	SetPrefWrapMargin(margin);
    else {
        char *params[1];
        char marginStr[25];
        sprintf(marginStr, "%d", margin);
        params[0] = marginStr;
        XtCallActionProc(window->textArea, "set_wrap_margin", NULL, params, 1);
    }
    DoneWithWrapDialog = True;
}

static void wrapCancelCB(Widget w, XtPointer clientData, XtPointer callData)
{
    DoneWithWrapDialog = True;
}

static void wrapWindowCB(Widget w, XtPointer clientData, XtPointer callData)
{
    int wrapAtWindow = XmToggleButtonGetState(w);
    
    XtSetSensitive(WrapTextLabel, !wrapAtWindow);
    XtSetSensitive(WrapText, !wrapAtWindow);
}

/*
** Present a dialog for editing language mode information
*/
void EditLanguageModes(Widget parent)
{
#define LIST_RIGHT 40
#define LEFT_MARGIN_POS 1
#define RIGHT_MARGIN_POS 99
#define H_MARGIN 5
    Widget form, nameLbl, topLbl, extLbl, recogLbl, delimitLbl;
    Widget okBtn, applyBtn, dismissBtn;
    Widget overrideFrame, overrideForm, delimitForm;
    Widget tabForm, tabLbl, indentBox, wrapBox;
    XmString s1;
    int i, ac;
    Arg args[20];

    /* if the dialog is already displayed, just pop it to the top and return */
    if (LMDialog.shell != NULL) {
    	RaiseShellWindow(LMDialog.shell);
    	return;
    }
    
    LMDialog.languageModeList = (languageModeRec **)XtMalloc(
    	    sizeof(languageModeRec *) * MAX_LANGUAGE_MODES);
    for (i=0; i<NLanguageModes; i++)
    	LMDialog.languageModeList[i] = copyLanguageModeRec(LanguageModes[i]);
    LMDialog.nLanguageModes = NLanguageModes;

    /* Create a form widget in an application shell */
    ac = 0;
    XtSetArg(args[ac], XmNdeleteResponse, XmDO_NOTHING); ac++;
    XtSetArg(args[ac], XmNiconName, "Language Modes"); ac++;
    XtSetArg(args[ac], XmNtitle, "Language Modes"); ac++;
    LMDialog.shell = CreateShellWithBestVis(APP_NAME, APP_CLASS,
	    applicationShellWidgetClass, TheDisplay, args, ac);
    AddSmallIcon(LMDialog.shell);
    form = XtVaCreateManagedWidget("editLanguageModes", xmFormWidgetClass,
	    LMDialog.shell, XmNautoUnmanage, False,
	    XmNresizePolicy, XmRESIZE_NONE, NULL);
    XtAddCallback(form, XmNdestroyCallback, lmDestroyCB, NULL);
    AddMotifCloseCallback(LMDialog.shell, lmDismissCB, NULL);
    
    topLbl = XtVaCreateManagedWidget("topLabel", xmLabelGadgetClass, form,
    	    XmNlabelString, s1=MKSTRING(
"To modify the properties of an existing language mode, select the name from\n\
the list on the left.  To add a new language, select \"New\" from the list."),
	    XmNmnemonic, 'N',
	    XmNtopAttachment, XmATTACH_POSITION,
	    XmNtopPosition, 2,
	    XmNleftAttachment, XmATTACH_POSITION,
	    XmNleftPosition, LEFT_MARGIN_POS,
	    XmNrightAttachment, XmATTACH_POSITION,
	    XmNrightPosition, RIGHT_MARGIN_POS, NULL);
    XmStringFree(s1);
    
    nameLbl = XtVaCreateManagedWidget("nameLbl", xmLabelGadgetClass, form,
    	    XmNlabelString, s1=XmStringCreateSimple("Name"),
	    XmNmnemonic, 'm',
    	    XmNalignment, XmALIGNMENT_BEGINNING,
	    XmNleftAttachment, XmATTACH_POSITION,
    	    XmNleftPosition, LIST_RIGHT,
	    XmNtopAttachment, XmATTACH_WIDGET,
	    XmNtopOffset, H_MARGIN,
	    XmNtopWidget, topLbl, NULL);
    XmStringFree(s1);
 
    LMDialog.nameW = XtVaCreateManagedWidget("name", xmTextWidgetClass, form,
    	    XmNcolumns, 15,
	    XmNleftAttachment, XmATTACH_POSITION,
	    XmNleftPosition, LIST_RIGHT,
	    XmNtopAttachment, XmATTACH_WIDGET,
	    XmNtopWidget, nameLbl,
	    XmNrightAttachment, XmATTACH_POSITION,
	    XmNrightPosition, (RIGHT_MARGIN_POS + LIST_RIGHT)/2, NULL);
    RemapDeleteKey(LMDialog.nameW);
    XtVaSetValues(nameLbl, XmNuserData, LMDialog.nameW, NULL);
    
    extLbl = XtVaCreateManagedWidget("extLbl", xmLabelGadgetClass, form,
    	    XmNlabelString, 	    	
    	    	s1=XmStringCreateSimple("File extensions (separate w/ space)"),
    	    XmNmnemonic, 'F',
    	    XmNalignment, XmALIGNMENT_BEGINNING,
	    XmNleftAttachment, XmATTACH_POSITION,
    	    XmNleftPosition, LIST_RIGHT,
	    XmNtopAttachment, XmATTACH_WIDGET,
	    XmNtopOffset, H_MARGIN,
	    XmNtopWidget, LMDialog.nameW, NULL);
    XmStringFree(s1);
 
    LMDialog.extW = XtVaCreateManagedWidget("ext", xmTextWidgetClass, form,
	    XmNleftAttachment, XmATTACH_POSITION,
	    XmNleftPosition, LIST_RIGHT,
	    XmNtopAttachment, XmATTACH_WIDGET,
	    XmNtopWidget, extLbl,
	    XmNrightAttachment, XmATTACH_POSITION,
	    XmNrightPosition, RIGHT_MARGIN_POS, NULL);
    RemapDeleteKey(LMDialog.extW);
    XtVaSetValues(extLbl, XmNuserData, LMDialog.extW, NULL);
    
    recogLbl = XtVaCreateManagedWidget("recogLbl", xmLabelGadgetClass, form,
    	    XmNlabelString, s1=MKSTRING(
"Recognition regular expression (applied to first 200\n\
characters of file to determine type from content)"),
    	    XmNalignment, XmALIGNMENT_BEGINNING,
    	    XmNmnemonic, 'R',
	    XmNleftAttachment, XmATTACH_POSITION,
    	    XmNleftPosition, LIST_RIGHT,
	    XmNtopAttachment, XmATTACH_WIDGET,
	    XmNtopOffset, H_MARGIN,
	    XmNtopWidget, LMDialog.extW, NULL);
    XmStringFree(s1);
 
    LMDialog.recogW = XtVaCreateManagedWidget("recog", xmTextWidgetClass, form,
	    XmNleftAttachment, XmATTACH_POSITION,
	    XmNleftPosition, LIST_RIGHT,
	    XmNtopAttachment, XmATTACH_WIDGET,
	    XmNtopWidget, recogLbl,
	    XmNrightAttachment, XmATTACH_POSITION,
	    XmNrightPosition, RIGHT_MARGIN_POS, NULL);
    RemapDeleteKey(LMDialog.recogW);
    XtVaSetValues(recogLbl, XmNuserData, LMDialog.recogW, NULL);
	    
    okBtn = XtVaCreateManagedWidget("ok", xmPushButtonWidgetClass, form,
    	    XmNlabelString, s1=XmStringCreateSimple("OK"),
    	    XmNleftAttachment, XmATTACH_POSITION,
    	    XmNleftPosition, 10,
    	    XmNrightAttachment, XmATTACH_POSITION,
    	    XmNrightPosition, 30,
    	    XmNbottomAttachment, XmATTACH_POSITION,
    	    XmNbottomPosition, 99, NULL);
    XtAddCallback(okBtn, XmNactivateCallback, lmOkCB, NULL);
    XmStringFree(s1);

    applyBtn = XtVaCreateManagedWidget("apply", xmPushButtonWidgetClass, form,
    	    XmNlabelString, s1=XmStringCreateSimple("Apply"),
    	    XmNmnemonic, 'A',
    	    XmNleftAttachment, XmATTACH_POSITION,
    	    XmNleftPosition, 40,
    	    XmNrightAttachment, XmATTACH_POSITION,
    	    XmNrightPosition, 60,
    	    XmNbottomAttachment, XmATTACH_POSITION,
    	    XmNbottomPosition, 99, NULL);
    XtAddCallback(applyBtn, XmNactivateCallback, lmApplyCB, NULL);
    XmStringFree(s1);

    dismissBtn = XtVaCreateManagedWidget("dismiss",xmPushButtonWidgetClass,form,
    	    XmNlabelString, s1=XmStringCreateSimple("Dismiss"),
    	    XmNleftAttachment, XmATTACH_POSITION,
    	    XmNleftPosition, 70,
    	    XmNrightAttachment, XmATTACH_POSITION,
    	    XmNrightPosition, 90,
    	    XmNbottomAttachment, XmATTACH_POSITION,
    	    XmNbottomPosition, 99, NULL);
    XtAddCallback(dismissBtn, XmNactivateCallback, lmDismissCB, NULL);
    XmStringFree(s1);

    overrideFrame = XtVaCreateManagedWidget("overrideFrame",
    	    xmFrameWidgetClass, form,
	    XmNleftAttachment, XmATTACH_POSITION,
	    XmNleftPosition, LEFT_MARGIN_POS,
	    XmNrightAttachment, XmATTACH_POSITION,
	    XmNrightPosition, RIGHT_MARGIN_POS,
	    XmNbottomAttachment, XmATTACH_WIDGET,
    	    XmNbottomWidget, dismissBtn,
	    XmNbottomOffset, H_MARGIN, NULL);
    overrideForm = XtVaCreateManagedWidget("overrideForm", xmFormWidgetClass,
	    overrideFrame, NULL);
    XtVaCreateManagedWidget("overrideLbl", xmLabelGadgetClass, overrideFrame,
    	    XmNlabelString, s1=XmStringCreateSimple("Override Defaults"),
	    XmNchildType, XmFRAME_TITLE_CHILD,
	    XmNchildHorizontalAlignment, XmALIGNMENT_CENTER, NULL);
    XmStringFree(s1);
 
    delimitForm = XtVaCreateManagedWidget("delimitForm", xmFormWidgetClass,
	    overrideForm,
	    XmNleftAttachment, XmATTACH_POSITION,
	    XmNleftPosition, LEFT_MARGIN_POS,
	    XmNtopAttachment, XmATTACH_FORM,
	    XmNtopOffset, H_MARGIN,
	    XmNrightAttachment, XmATTACH_POSITION,
	    XmNrightPosition, RIGHT_MARGIN_POS, NULL);
    delimitLbl = XtVaCreateManagedWidget("delimitLbl", xmLabelGadgetClass,
    	    delimitForm,
    	    XmNlabelString, s1=XmStringCreateSimple("Word delimiters"),
    	    XmNmnemonic, 'W',
	    XmNleftAttachment, XmATTACH_FORM,
	    XmNtopAttachment, XmATTACH_FORM,
	    XmNbottomAttachment, XmATTACH_FORM, NULL);
    XmStringFree(s1);
    LMDialog.delimitW = XtVaCreateManagedWidget("delimit", xmTextWidgetClass,
    	    delimitForm,
	    XmNtopAttachment, XmATTACH_FORM,
	    XmNleftAttachment, XmATTACH_WIDGET,
	    XmNleftWidget, delimitLbl,
	    XmNrightAttachment, XmATTACH_FORM,
	    XmNbottomAttachment, XmATTACH_FORM, NULL);
    RemapDeleteKey(LMDialog.delimitW);
    XtVaSetValues(delimitLbl, XmNuserData, LMDialog.delimitW, NULL);

    tabForm = XtVaCreateManagedWidget("tabForm", xmFormWidgetClass,
	    overrideForm,
	    XmNleftAttachment, XmATTACH_POSITION,
	    XmNleftPosition, LEFT_MARGIN_POS,
	    XmNtopAttachment, XmATTACH_WIDGET,
	    XmNtopWidget, delimitForm,
	    XmNtopOffset, H_MARGIN,
	    XmNrightAttachment, XmATTACH_POSITION,
	    XmNrightPosition, RIGHT_MARGIN_POS, NULL);
    tabLbl = XtVaCreateManagedWidget("tabLbl", xmLabelGadgetClass, tabForm,
    	    XmNlabelString, s1=XmStringCreateSimple(
    	    	"Alternative hardware tab spacing"),
    	    XmNmnemonic, 't',
	    XmNleftAttachment, XmATTACH_FORM,
	    XmNtopAttachment, XmATTACH_FORM,
	    XmNbottomAttachment, XmATTACH_FORM, NULL);
    XmStringFree(s1);
    LMDialog.tabW = XtVaCreateManagedWidget("delimit", xmTextWidgetClass,
    	    tabForm,
    	    XmNcolumns, 3,
	    XmNtopAttachment, XmATTACH_FORM,
	    XmNleftAttachment, XmATTACH_WIDGET,
	    XmNleftWidget, tabLbl,
	    XmNbottomAttachment, XmATTACH_FORM, NULL);
    RemapDeleteKey(LMDialog.tabW);
    XtVaSetValues(tabLbl, XmNuserData, LMDialog.tabW, NULL);
    LMDialog.emTabW = XtVaCreateManagedWidget("delimit", xmTextWidgetClass,
    	    tabForm,
    	    XmNcolumns, 3,
	    XmNtopAttachment, XmATTACH_FORM,
	    XmNrightAttachment, XmATTACH_FORM,
	    XmNbottomAttachment, XmATTACH_FORM, NULL);
    RemapDeleteKey(LMDialog.emTabW);
    XtVaCreateManagedWidget("emTabLbl", xmLabelGadgetClass, tabForm,
    	    XmNlabelString,
    	    s1=XmStringCreateSimple("Alternative emulated tab spacing"),
    	    XmNalignment, XmALIGNMENT_END, 
    	    XmNmnemonic, 'e',
	    XmNuserData, LMDialog.emTabW,
	    XmNleftAttachment, XmATTACH_WIDGET,
	    XmNleftWidget, LMDialog.tabW,
	    XmNrightAttachment, XmATTACH_WIDGET,
	    XmNrightWidget, LMDialog.emTabW,
	    XmNtopAttachment, XmATTACH_FORM,
	    XmNbottomAttachment, XmATTACH_FORM, NULL);
    XmStringFree(s1);

    indentBox = XtVaCreateManagedWidget("indentBox", xmRowColumnWidgetClass,
    	    overrideForm,
    	    XmNorientation, XmHORIZONTAL,
    	    XmNpacking, XmPACK_TIGHT,
    	    XmNradioBehavior, True,
	    XmNleftAttachment, XmATTACH_POSITION,
    	    XmNleftPosition, LEFT_MARGIN_POS,
	    XmNtopAttachment, XmATTACH_WIDGET,
	    XmNtopWidget, tabForm,
	    XmNtopOffset, H_MARGIN, NULL);
    LMDialog.defaultIndentW = XtVaCreateManagedWidget("defaultIndent", 
    	    xmToggleButtonWidgetClass, indentBox,
    	    XmNset, True,
    	    XmNmarginHeight, 0,
    	    XmNlabelString, s1=XmStringCreateSimple("Default indent style"),
    	    XmNmnemonic, 'D', NULL);
    XmStringFree(s1);
    LMDialog.noIndentW = XtVaCreateManagedWidget("noIndent", 
    	    xmToggleButtonWidgetClass, indentBox,
    	    XmNmarginHeight, 0,
    	    XmNlabelString, s1=XmStringCreateSimple("No automatic indent"),
    	    XmNmnemonic, 'N', NULL);
    XmStringFree(s1);
    LMDialog.autoIndentW = XtVaCreateManagedWidget("autoIndent", 
    	    xmToggleButtonWidgetClass, indentBox,
    	    XmNmarginHeight, 0,
    	    XmNlabelString, s1=XmStringCreateSimple("Auto-indent"),
    	    XmNmnemonic, 'A', NULL);
    XmStringFree(s1);
    LMDialog.smartIndentW = XtVaCreateManagedWidget("smartIndent", 
    	    xmToggleButtonWidgetClass, indentBox,
    	    XmNmarginHeight, 0,
    	    XmNlabelString, s1=XmStringCreateSimple("Smart-indent"),
    	    XmNmnemonic, 'S', NULL);
    XmStringFree(s1);

    wrapBox = XtVaCreateManagedWidget("wrapBox", xmRowColumnWidgetClass,
    	    overrideForm,
    	    XmNorientation, XmHORIZONTAL,
    	    XmNpacking, XmPACK_TIGHT,
    	    XmNradioBehavior, True,
	    XmNleftAttachment, XmATTACH_POSITION,
    	    XmNleftPosition, LEFT_MARGIN_POS,
	    XmNtopAttachment, XmATTACH_WIDGET,
	    XmNtopWidget, indentBox,
	    XmNtopOffset, H_MARGIN,
	    XmNbottomAttachment, XmATTACH_FORM,
	    XmNbottomOffset, H_MARGIN, NULL);
    LMDialog.defaultWrapW = XtVaCreateManagedWidget("defaultWrap", 
    	    xmToggleButtonWidgetClass, wrapBox,
    	    XmNset, True,
    	    XmNmarginHeight, 0,
    	    XmNlabelString, s1=XmStringCreateSimple("Default wrap style"),
    	    XmNmnemonic, 'D', NULL);
    XmStringFree(s1);
    LMDialog.noWrapW = XtVaCreateManagedWidget("noWrap", 
    	    xmToggleButtonWidgetClass, wrapBox,
    	    XmNmarginHeight, 0,
    	    XmNlabelString, s1=XmStringCreateSimple("No wrapping"),
    	    XmNmnemonic, 'N', NULL);
    XmStringFree(s1);
    LMDialog.newlineWrapW = XtVaCreateManagedWidget("newlineWrap", 
    	    xmToggleButtonWidgetClass, wrapBox,
    	    XmNmarginHeight, 0,
    	    XmNlabelString, s1=XmStringCreateSimple("Auto newline wrap"),
    	    XmNmnemonic, 'A', NULL);
    XmStringFree(s1);
    LMDialog.contWrapW = XtVaCreateManagedWidget("contWrap", 
    	    xmToggleButtonWidgetClass, wrapBox,
    	    XmNmarginHeight, 0,
    	    XmNlabelString, s1=XmStringCreateSimple("Continuous wrap"),
    	    XmNmnemonic, 'C', NULL);
    XmStringFree(s1);

    XtVaCreateManagedWidget("stretchForm", xmFormWidgetClass, form,
	    XmNtopAttachment, XmATTACH_WIDGET,
	    XmNtopWidget, LMDialog.recogW,
	    XmNleftAttachment, XmATTACH_POSITION,
	    XmNleftPosition, LIST_RIGHT,
	    XmNrightAttachment, XmATTACH_POSITION,
	    XmNrightPosition, RIGHT_MARGIN_POS,
	    XmNbottomAttachment, XmATTACH_WIDGET,
	    XmNbottomWidget, overrideFrame,
	    XmNbottomOffset, H_MARGIN*2, NULL);
    
    ac = 0;
    XtSetArg(args[ac], XmNtopAttachment, XmATTACH_WIDGET); ac++;
    XtSetArg(args[ac], XmNtopOffset, H_MARGIN); ac++;
    XtSetArg(args[ac], XmNtopWidget, topLbl); ac++;
    XtSetArg(args[ac], XmNleftAttachment, XmATTACH_POSITION); ac++;
    XtSetArg(args[ac], XmNleftPosition, LEFT_MARGIN_POS); ac++;
    XtSetArg(args[ac], XmNrightAttachment, XmATTACH_POSITION); ac++;
    XtSetArg(args[ac], XmNrightPosition, LIST_RIGHT-1); ac++;
    XtSetArg(args[ac], XmNbottomAttachment, XmATTACH_WIDGET); ac++;
    XtSetArg(args[ac], XmNbottomWidget, overrideFrame); ac++;
    XtSetArg(args[ac], XmNbottomOffset, H_MARGIN*2); ac++;
    LMDialog.managedListW = CreateManagedList(form, "list", args, ac,
    	    (void **)LMDialog.languageModeList, &LMDialog.nLanguageModes,
    	    MAX_LANGUAGE_MODES, 15, lmGetDisplayedCB, NULL, lmSetDisplayedCB,
    	    NULL, lmFreeItemCB);
    AddDeleteConfirmCB(LMDialog.managedListW, lmDeleteConfirmCB, NULL);
    XtVaSetValues(topLbl, XmNuserData, LMDialog.managedListW, NULL);
    	
    /* Set initial default button */
    XtVaSetValues(form, XmNdefaultButton, okBtn, NULL);
    XtVaSetValues(form, XmNcancelButton, dismissBtn, NULL);
    
    /* Handle mnemonic selection of buttons and focus to dialog */
    AddDialogMnemonicHandler(form, FALSE);

    /* Realize all of the widgets in the new dialog */
    RealizeWithoutForcingPosition(LMDialog.shell);
}

static void lmDestroyCB(Widget w, XtPointer clientData, XtPointer callData)
{
    int i;
    
    for (i=0; i<LMDialog.nLanguageModes; i++)
    	freeLanguageModeRec(LMDialog.languageModeList[i]);
    XtFree((char *)LMDialog.languageModeList);
}

static void lmOkCB(Widget w, XtPointer clientData, XtPointer callData)
{
    if (!updateLMList())
    	return;

    /* pop down and destroy the dialog */
    XtDestroyWidget(LMDialog.shell);
    LMDialog.shell = NULL;
}

static void lmApplyCB(Widget w, XtPointer clientData, XtPointer callData)
{
    updateLMList();
}

static void lmDismissCB(Widget w, XtPointer clientData, XtPointer callData)
{
    /* pop down and destroy the dialog */
    XtDestroyWidget(LMDialog.shell);
    LMDialog.shell = NULL;
}

static int lmDeleteConfirmCB(int itemIndex, void *cbArg)
{
    int i;
    
    /* Allow duplicate names to be deleted regardless of dependencies */
    for (i=0; i<LMDialog.nLanguageModes; i++)
	if (i != itemIndex && !strcmp(LMDialog.languageModeList[i]->name,
		LMDialog.languageModeList[itemIndex]->name))
	    return True;
    
    /* don't allow deletion if data will be lost */
    if (LMHasHighlightPatterns(LMDialog.languageModeList[itemIndex]->name)) {
    	DialogF(DF_WARN, LMDialog.shell, 1,
"This language mode has syntax highlighting\n\
patterns defined.  Please delete the patterns\n\
first, in Preferences -> Default Settings ->\n\
Syntax Highlighting, before proceeding here.", "Dismiss");
	return False;
    }
    return True;
}

/*
** Apply the changes that the user has made in the language modes dialog to the
** stored language mode information for this NEdit session (the data array
** LanguageModes)
*/
static int updateLMList(void)
{
    WindowInfo *window;
    char *oldModeName, *newDelimiters;
    int i, j;
    
    /* Get the current contents of the dialog fields */
    if (!UpdateManagedList(LMDialog.managedListW, True))
    	return False;

    /* Fix up language mode indecies in all open windows (which may change
       if the currently selected mode is deleted or has changed position),
       and update word delimiters */
    for (window=WindowList; window!=NULL; window=window->next) {
	if (window->languageMode != PLAIN_LANGUAGE_MODE) {
    	    oldModeName = LanguageModes[window->languageMode]->name;
    	    window->languageMode = PLAIN_LANGUAGE_MODE;
    	    for (i=0; i<LMDialog.nLanguageModes; i++) {
    		if (!strcmp(oldModeName, LMDialog.languageModeList[i]->name)) {
    	    	    newDelimiters = LMDialog.languageModeList[i]->delimiters;
    	    	    if (newDelimiters == NULL)
    	    	    	newDelimiters = GetPrefDelimiters();
    	    	    XtVaSetValues(window->textArea, textNwordDelimiters,
    	    	    	    newDelimiters, NULL);
    	    	    for (j=0; j<window->nPanes; j++)
    	    	    	XtVaSetValues(window->textPanes[j],
    	    	    	    	textNwordDelimiters, newDelimiters, NULL);
    	    	    window->languageMode = i;
    	    	    break;
    		}
    	    }
	}
    }
    
    /* If there were any name changes, re-name dependent highlight patterns
       and fix up the weird rename-format names */
    for (i=0; i<LMDialog.nLanguageModes; i++) {
    	if (strchr(LMDialog.languageModeList[i]->name, ':') != NULL) {
    	    char *newName = strrchr(LMDialog.languageModeList[i]->name, ':')+1;
    	    *strchr(LMDialog.languageModeList[i]->name, ':') = '\0';
    	    RenameHighlightPattern(LMDialog.languageModeList[i]->name, newName);
    	    memmove(LMDialog.languageModeList[i]->name, newName,
    	    	    strlen(newName) + 1);
    	    ChangeManagedListData(LMDialog.managedListW);
    	}
    }
    
    /* Replace the old language mode list with the new one from the dialog */
    for (i=0; i<NLanguageModes; i++)
    	freeLanguageModeRec(LanguageModes[i]);
    for (i=0; i<LMDialog.nLanguageModes; i++)
    	LanguageModes[i] = copyLanguageModeRec(LMDialog.languageModeList[i]);
    NLanguageModes = LMDialog.nLanguageModes;
    
    /* Update the menus in the window menu bars */
    for (window=WindowList; window!=NULL; window=window->next)
    	updateLanguageModeSubmenu(window);
    
    /* If a syntax highlighting dialog is up, update its menu */
    UpdateLanguageModeMenu();
    
    /* Note that preferences have been changed */
    MarkPrefsChanged();

    return True;
}

static void *lmGetDisplayedCB(void *oldItem, int explicitRequest, int *abort,
    	void *cbArg)
{
    languageModeRec *lm, *oldLM = (languageModeRec *)oldItem;
    char *tempName;
    int i, nCopies, oldLen;
    
    /* If the dialog is currently displaying the "new" entry and the
       fields are empty, that's just fine */
    if (oldItem == NULL && lmDialogEmpty())
    	return NULL;
    
    /* Read the data the user has entered in the dialog fields */
    lm = readLMDialogFields(True);

    /* If there was a name change of a non-duplicate language mode, modify the
       name to the weird format of: ":old name:new name".  This signals that a
       name change is necessary in lm dependent data such as highlight
       patterns.  Duplicate language modes may be re-named at will, since no
       data will be lost due to the name change. */
    if (lm != NULL && oldLM != NULL && strcmp(oldLM->name, lm->name)) {
    	nCopies = 0;
	for (i=0; i<LMDialog.nLanguageModes; i++)
	    if (!strcmp(oldLM->name, LMDialog.languageModeList[i]->name))
		nCopies++;
	if (nCopies <= 1) {
    	    oldLen = strchr(oldLM->name, ':') == NULL ? strlen(oldLM->name) :
    	    	    strchr(oldLM->name, ':') - oldLM->name;
    	    tempName = XtMalloc(oldLen + strlen(lm->name) + 2);
    	    strncpy(tempName, oldLM->name, oldLen);
    	    sprintf(&tempName[oldLen], ":%s", lm->name);
    	    XtFree(lm->name);
    	    lm->name = tempName;
	}
    }
    
    /* If there are no problems reading the data, just return it */
    if (lm != NULL)
    	return (void *)lm;
    
    /* If there are problems, and the user didn't ask for the fields to be
       read, give more warning */
    if (!explicitRequest) {
	if (DialogF(DF_WARN, LMDialog.shell, 2,
    		"Discard incomplete entry\nfor current language mode?", "Keep",
    		"Discard") == 2) {
     	    return oldItem == NULL ? NULL :
     	    	    (void *)copyLanguageModeRec((languageModeRec *)oldItem);
	}
    }

    /* Do readLMDialogFields again without "silent" mode to display warning */
    lm = readLMDialogFields(False);
    *abort = True;
    return NULL;
}

static void lmSetDisplayedCB(void *item, void *cbArg)
{
    languageModeRec *lm = (languageModeRec *)item;
    char *extStr;

    if (item == NULL) {
    	XmTextSetString(LMDialog.nameW, "");
    	XmTextSetString(LMDialog.extW, "");
    	XmTextSetString(LMDialog.recogW, "");
    	XmTextSetString(LMDialog.delimitW, "");
    	XmTextSetString(LMDialog.tabW, "");
    	XmTextSetString(LMDialog.emTabW, "");
    	XmToggleButtonSetState(LMDialog.defaultIndentW, True, True);
    	XmToggleButtonSetState(LMDialog.defaultWrapW, True, True);
    } else {
    	XmTextSetString(LMDialog.nameW, strchr(lm->name, ':') == NULL ?
    	    	lm->name : strchr(lm->name, ':')+1);
    	extStr = createExtString(lm->extensions, lm->nExtensions);
    	XmTextSetString(LMDialog.extW, extStr);
    	XtFree(extStr);
    	XmTextSetString(LMDialog.recogW, lm->recognitionExpr);
    	XmTextSetString(LMDialog.delimitW, lm->delimiters);
    	if (lm->tabDist == DEFAULT_TAB_DIST)
    	    XmTextSetString(LMDialog.tabW, "");
    	else
    	    SetIntText(LMDialog.tabW, lm->tabDist);
    	if (lm->emTabDist == DEFAULT_EM_TAB_DIST)
    	    XmTextSetString(LMDialog.emTabW, "");
    	else
    	    SetIntText(LMDialog.emTabW, lm->emTabDist);
    	XmToggleButtonSetState(LMDialog.defaultIndentW,
    	    	lm->indentStyle == DEFAULT_INDENT, False);
    	XmToggleButtonSetState(LMDialog.noIndentW,
    	    	lm->indentStyle == NO_AUTO_INDENT, False);
    	XmToggleButtonSetState(LMDialog.autoIndentW,
    	    	lm->indentStyle == AUTO_INDENT, False);
    	XmToggleButtonSetState(LMDialog.smartIndentW,
    	    	lm->indentStyle == SMART_INDENT, False);
    	XmToggleButtonSetState(LMDialog.defaultWrapW,
    	    	lm->wrapStyle == DEFAULT_WRAP, False);
    	XmToggleButtonSetState(LMDialog.noWrapW,
    	    	lm->wrapStyle == NO_WRAP, False);
    	XmToggleButtonSetState(LMDialog.newlineWrapW,
    	    	lm->wrapStyle == NEWLINE_WRAP, False);
    	XmToggleButtonSetState(LMDialog.contWrapW,
    	    	lm->wrapStyle == CONTINUOUS_WRAP, False);
    }
}

static void lmFreeItemCB(void *item)
{
    freeLanguageModeRec((languageModeRec *)item);
}

static void freeLanguageModeRec(languageModeRec *lm)
{
    int i;
    
    XtFree(lm->name);
    if (lm->recognitionExpr != NULL)
    	XtFree(lm->recognitionExpr);
    if (lm->delimiters != NULL)
    	XtFree(lm->delimiters);
    for (i=0; i<lm->nExtensions; i++)
    	XtFree(lm->extensions[i]);
    if (lm->nExtensions != 0)
	XtFree((char *)lm->extensions);
    XtFree((char *)lm);
}

/*
** Copy a languageModeRec data structure and all of the allocated data it contains
*/
static languageModeRec *copyLanguageModeRec(languageModeRec *lm)
{
    languageModeRec *newLM;
    int i;
    
    newLM = (languageModeRec *)XtMalloc(sizeof(languageModeRec));
    newLM->name = XtMalloc(strlen(lm->name)+1);
    strcpy(newLM->name, lm->name);
    newLM->nExtensions = lm->nExtensions;
    newLM->extensions = (char **)XtMalloc(sizeof(char *) * lm->nExtensions);
    for (i=0; i<lm->nExtensions; i++) {
    	newLM->extensions[i] = XtMalloc(strlen(lm->extensions[i]) + 1);
    	strcpy(newLM->extensions[i], lm->extensions[i]);
    }
    if (lm->recognitionExpr == NULL)
    	newLM->recognitionExpr = NULL;
    else {
	newLM->recognitionExpr = XtMalloc(strlen(lm->recognitionExpr)+1);
	strcpy(newLM->recognitionExpr, lm->recognitionExpr);
    }
    if (lm->delimiters == NULL)
    	newLM->delimiters = NULL;
    else {
	newLM->delimiters = XtMalloc(strlen(lm->delimiters)+1);
	strcpy(newLM->delimiters, lm->delimiters);
    }
    newLM->wrapStyle = lm->wrapStyle;
    newLM->indentStyle = lm->indentStyle;
    newLM->tabDist = lm->tabDist;
    newLM->emTabDist = lm->emTabDist;
    return newLM;
}

/*
** Read the fields in the language modes dialog and create a languageModeRec data
** structure reflecting the current state of the selected language mode in the dialog.
** If any of the information is incorrect or missing, display a warning dialog and
** return NULL.  Passing "silent" as True, suppresses the warning dialogs.
*/
static languageModeRec *readLMDialogFields(int silent)
{
    languageModeRec *lm;
    regexp *compiledRE;
    char *compileMsg, *extStr, *extPtr;

    /* Allocate a language mode structure to return, set unread fields to
       empty so everything can be freed on errors by freeLanguageModeRec */
    lm = (languageModeRec *)XtMalloc(sizeof(languageModeRec));
    lm->nExtensions = 0;
    lm->recognitionExpr = NULL;
    lm->delimiters = NULL;

    /* read the name field */
    lm->name = ReadSymbolicFieldTextWidget(LMDialog.nameW,
    	    "language mode name", silent);
    if (lm->name == NULL) {
    	XtFree((char *)lm);
    	return NULL;
    }
    if (*lm->name == '\0') {
    	if (!silent) {
    	    DialogF(DF_WARN, LMDialog.shell, 1,
    		    "Please specify a name\nfor the language mode", "Dismiss");
    	    XmProcessTraversal(LMDialog.nameW, XmTRAVERSE_CURRENT);
    	}
    	freeLanguageModeRec(lm);
   	return NULL;
    }
    
    /* read the extension list field */
    extStr = extPtr = XmTextGetString(LMDialog.extW);
    lm->extensions = readExtensionList(&extPtr, &lm->nExtensions);
    XtFree(extStr);
    
    /* read recognition expression */
    lm->recognitionExpr = XmTextGetString(LMDialog.recogW);
    if (*lm->recognitionExpr == '\0') {
    	XtFree(lm->recognitionExpr);
    	lm->recognitionExpr = NULL;
    } else {
	compiledRE = CompileRE(lm->recognitionExpr, &compileMsg, REDFLT_STANDARD);
	if (compiledRE == NULL) {
   	    if (!silent) {
   		DialogF(DF_WARN, LMDialog.shell, 1, "Recognition expression:\n%s",
   	    		"Dismiss", compileMsg);
     		XmProcessTraversal(LMDialog.recogW, XmTRAVERSE_CURRENT);
	    }
 	    XtFree((char *)compiledRE);
 	    freeLanguageModeRec(lm);
 	    return NULL;    
	}
	XtFree((char *)compiledRE);
    }
    
    /* read tab spacing field */
    if (TextWidgetIsBlank(LMDialog.tabW))
    	lm->tabDist = DEFAULT_TAB_DIST;
    else {
    	if (GetIntTextWarn(LMDialog.tabW, &lm->tabDist, "tab spacing", False)
    	    	!= TEXT_READ_OK) {
   	    freeLanguageModeRec(lm);
    	    return NULL;
	}
	if (lm->tabDist <= 0 || lm->tabDist > 100) {
   	    if (!silent) {
		DialogF(DF_WARN, LMDialog.shell, 1, "Invalid tab spacing: %d",
	    		"Dismiss", lm->tabDist);
		XmProcessTraversal(LMDialog.tabW, XmTRAVERSE_CURRENT);
	    }
	    freeLanguageModeRec(lm);
    	    return NULL;
	}
    }
    
    /* read emulated tab field */
    if (TextWidgetIsBlank(LMDialog.emTabW))
    	lm->emTabDist = DEFAULT_EM_TAB_DIST;
    else {
    	if (GetIntTextWarn(LMDialog.emTabW, &lm->emTabDist,
    	    	"emulated tab spacing", False) != TEXT_READ_OK) {
    	    freeLanguageModeRec(lm);
    	    return NULL;
	}
	if (lm->emTabDist < 0 || lm->emTabDist > 100) {
   	    if (!silent) {
		DialogF(DF_WARN, LMDialog.shell, 1,
	    		"Invalid emulated tab spacing: %d", "Dismiss",
			lm->emTabDist);
		XmProcessTraversal(LMDialog.emTabW, XmTRAVERSE_CURRENT);
	    }
	    freeLanguageModeRec(lm);
    	    return NULL;
	}
    }
    
    /* read delimiters string */
    lm->delimiters = XmTextGetString(LMDialog.delimitW);
    if (*lm->delimiters == '\0') {
    	XtFree(lm->delimiters);
    	lm->delimiters = NULL;
    }
    
    /* read indent style */
    if (XmToggleButtonGetState(LMDialog.noIndentW))
    	 lm->indentStyle = NO_AUTO_INDENT;
    else if (XmToggleButtonGetState(LMDialog.autoIndentW))
    	 lm->indentStyle = AUTO_INDENT;
    else if (XmToggleButtonGetState(LMDialog.smartIndentW))
    	 lm->indentStyle = SMART_INDENT;
    else
    	 lm->indentStyle = DEFAULT_INDENT;
    
    /* read wrap style */
    if (XmToggleButtonGetState(LMDialog.noWrapW))
    	 lm->wrapStyle = NO_WRAP;
    else if (XmToggleButtonGetState(LMDialog.newlineWrapW))
    	 lm->wrapStyle = NEWLINE_WRAP;
    else if (XmToggleButtonGetState(LMDialog.contWrapW))
    	 lm->wrapStyle = CONTINUOUS_WRAP;
    else
    	 lm->wrapStyle = DEFAULT_WRAP;
    return lm;
}

/*
** Return True if the language mode dialog fields are blank (unchanged from the "New"
** language mode state).
*/
static int lmDialogEmpty(void)
{
    return TextWidgetIsBlank(LMDialog.nameW) &&
 	    TextWidgetIsBlank(LMDialog.extW) &&
	    TextWidgetIsBlank(LMDialog.recogW) &&
	    TextWidgetIsBlank(LMDialog.delimitW) &&
	    TextWidgetIsBlank(LMDialog.tabW) &&
	    TextWidgetIsBlank(LMDialog.emTabW) &&
	    XmToggleButtonGetState(LMDialog.defaultIndentW) &&
	    XmToggleButtonGetState(LMDialog.defaultWrapW);
}   	

/*
** Present a dialog for changing fonts (primary, and for highlighting).
*/
void ChooseFonts(WindowInfo *window, int forWindow)
{
#define MARGIN_SPACING 10
#define BTN_TEXT_OFFSET 3
    Widget form, primaryLbl, primaryBtn, italicLbl, italicBtn;
    Widget boldLbl, boldBtn, boldItalicLbl, boldItalicBtn;
    Widget primaryFrame, primaryForm, highlightFrame, highlightForm;
    Widget okBtn, applyBtn, dismissBtn;
    fontDialog *fd;
    XmString s1;
    int ac;
    Arg args[20];

    /* if the dialog is already displayed, just pop it to the top and return */
    if (window->fontDialog != NULL) {
    	RaiseShellWindow(((fontDialog *)window->fontDialog)->shell);
    	return;
    }
    
    /* Create a structure for keeping track of dialog state */
    fd = (fontDialog *)XtMalloc(sizeof(fontDialog));
    fd->window = window;
    fd->forWindow = forWindow;
    window->fontDialog = (void*)fd;
    
    /* Create a form widget in a dialog shell */
    ac = 0;
    XtSetArg(args[ac], XmNautoUnmanage, False); ac++;
    XtSetArg(args[ac], XmNresizePolicy, XmRESIZE_NONE); ac++;
    form = CreateFormDialog(window->shell, "choose Fonts", args, ac);
    XtVaSetValues(form, XmNshadowThickness, 0, NULL);
    fd->shell = XtParent(form);
    XtVaSetValues(fd->shell, XmNtitle, "Fonts", NULL);
    AddMotifCloseCallback(XtParent(form), fontDismissCB, fd);
    XtAddCallback(form, XmNdestroyCallback, fontDestroyCB, fd);

    primaryFrame = XtVaCreateManagedWidget("primaryFrame", xmFrameWidgetClass,
    	    form, XmNmarginHeight, 3,
	    XmNtopAttachment, XmATTACH_POSITION,
	    XmNtopPosition, 2,
	    XmNleftAttachment, XmATTACH_POSITION,
	    XmNleftPosition, 1,
	    XmNrightAttachment, XmATTACH_POSITION,
	    XmNrightPosition, 99, NULL);
    primaryForm = XtVaCreateManagedWidget("primaryForm", xmFormWidgetClass,
	    primaryFrame, NULL);
    primaryLbl = XtVaCreateManagedWidget("primaryFont", xmLabelGadgetClass,
    	    primaryFrame,
    	    XmNlabelString, s1=XmStringCreateSimple("Primary Font"),
    	    XmNmnemonic, 'P',
	    XmNchildType, XmFRAME_TITLE_CHILD,
	    XmNchildHorizontalAlignment, XmALIGNMENT_CENTER, NULL);
    XmStringFree(s1);

    primaryBtn = XtVaCreateManagedWidget("primaryBtn",
    	    xmPushButtonWidgetClass, primaryForm,
    	    XmNlabelString, s1=XmStringCreateSimple("Browse..."),
    	    XmNmnemonic, 'r',
	    XmNtopAttachment, XmATTACH_POSITION,
	    XmNtopPosition, 2,
	    XmNtopOffset, BTN_TEXT_OFFSET,
	    XmNleftAttachment, XmATTACH_POSITION,
	    XmNleftPosition, 1, NULL);
    XtAddCallback(primaryBtn, XmNactivateCallback, primaryBrowseCB, fd);

    fd->primaryW = XtVaCreateManagedWidget("primary", xmTextWidgetClass,
    	    primaryForm,
    	    XmNcolumns, 70,
    	    XmNmaxLength, MAX_FONT_LEN,
	    XmNleftAttachment, XmATTACH_WIDGET,
	    XmNleftWidget, primaryBtn,
	    XmNtopAttachment, XmATTACH_POSITION,
	    XmNtopPosition, 2,
	    XmNrightAttachment, XmATTACH_POSITION,
	    XmNrightPosition, 99, NULL);
    RemapDeleteKey(fd->primaryW);
    XtAddCallback(fd->primaryW, XmNvalueChangedCallback,
    	    primaryModifiedCB, fd);
    XtVaSetValues(primaryLbl, XmNuserData, fd->primaryW, NULL);

    highlightFrame = XtVaCreateManagedWidget("highlightFrame",
    	    xmFrameWidgetClass, form,
	    XmNmarginHeight, 3,
	    XmNnavigationType, XmTAB_GROUP,
	    XmNtopAttachment, XmATTACH_WIDGET,
	    XmNtopWidget, primaryFrame,
	    XmNtopOffset, 20,
	    XmNleftAttachment, XmATTACH_POSITION,
	    XmNleftPosition, 1,
	    XmNrightAttachment, XmATTACH_POSITION,
	    XmNrightPosition, 99, NULL);
    highlightForm = XtVaCreateManagedWidget("highlightForm", xmFormWidgetClass,
    	    highlightFrame, NULL);
    XtVaCreateManagedWidget("highlightFonts", xmLabelGadgetClass,
    	    highlightFrame,
    	    XmNlabelString,
    	    	s1=XmStringCreateSimple("Fonts for Syntax Highlighting"),
	    XmNchildType, XmFRAME_TITLE_CHILD,
	    XmNchildHorizontalAlignment, XmALIGNMENT_CENTER, NULL);
    XmStringFree(s1);

    fd->fillW = XtVaCreateManagedWidget("fillBtn",
    	    xmPushButtonWidgetClass, highlightForm,
    	    XmNlabelString,
    	    	s1=XmStringCreateSimple("Fill Highlight Fonts from Primary"),
    	    XmNmnemonic, 'F',
    	    XmNtopAttachment, XmATTACH_POSITION,
	    XmNtopPosition, 2,
	    XmNtopOffset, BTN_TEXT_OFFSET,
	    XmNleftAttachment, XmATTACH_POSITION,
	    XmNleftPosition, 1, NULL);
    XtAddCallback(fd->fillW, XmNactivateCallback, fillFromPrimaryCB, fd);

    italicLbl = XtVaCreateManagedWidget("italicLbl", xmLabelGadgetClass,
    	    highlightForm,
    	    XmNlabelString, s1=XmStringCreateSimple("Italic Font"),
    	    XmNmnemonic, 'I',
    	    XmNalignment, XmALIGNMENT_BEGINNING,
	    XmNtopAttachment, XmATTACH_WIDGET,
	    XmNtopWidget, fd->fillW,
	    XmNtopOffset, MARGIN_SPACING,
	    XmNleftAttachment, XmATTACH_POSITION,
	    XmNleftPosition, 1, NULL);
    XmStringFree(s1);

    fd->italicErrW = XtVaCreateManagedWidget("italicErrLbl",
    	    xmLabelGadgetClass, highlightForm,
    	    XmNlabelString, s1=XmStringCreateSimple(
    	    	"(vvv  spacing is inconsistent with primary font  vvv)"),
    	    XmNalignment, XmALIGNMENT_END,
	    XmNtopAttachment, XmATTACH_WIDGET,
	    XmNtopWidget, fd->fillW,
	    XmNtopOffset, MARGIN_SPACING,
	    XmNleftAttachment, XmATTACH_WIDGET,
	    XmNleftWidget, italicLbl,
	    XmNrightAttachment, XmATTACH_POSITION,
	    XmNrightPosition, 99, NULL);
    XmStringFree(s1);

    italicBtn = XtVaCreateManagedWidget("italicBtn",
    	    xmPushButtonWidgetClass, highlightForm,
    	    XmNlabelString, s1=XmStringCreateSimple("Browse..."),
    	    XmNmnemonic, 'o',
    	    XmNtopAttachment, XmATTACH_WIDGET,
	    XmNtopWidget, italicLbl,
	    XmNtopOffset, BTN_TEXT_OFFSET,
	    XmNleftAttachment, XmATTACH_POSITION,
	    XmNleftPosition, 1, NULL);
    XtAddCallback(italicBtn, XmNactivateCallback, italicBrowseCB, fd);

    fd->italicW = XtVaCreateManagedWidget("italic", xmTextWidgetClass,
    	    highlightForm,
    	    XmNmaxLength, MAX_FONT_LEN,
	    XmNleftAttachment, XmATTACH_WIDGET,
	    XmNleftWidget, italicBtn,
	    XmNtopAttachment, XmATTACH_WIDGET,
	    XmNtopWidget, italicLbl,
	    XmNrightAttachment, XmATTACH_POSITION,
	    XmNrightPosition, 99, NULL);
    RemapDeleteKey(fd->italicW);
    XtAddCallback(fd->italicW, XmNvalueChangedCallback,
    	    italicModifiedCB, fd);
    XtVaSetValues(italicLbl, XmNuserData, fd->italicW, NULL);

    boldLbl = XtVaCreateManagedWidget("boldLbl", xmLabelGadgetClass,
    	    highlightForm,
    	    XmNlabelString, s1=XmStringCreateSimple("Bold Font"),
    	    XmNmnemonic, 'B',
    	    XmNalignment, XmALIGNMENT_BEGINNING,
	    XmNtopAttachment, XmATTACH_WIDGET,
	    XmNtopWidget, italicBtn,
	    XmNtopOffset, MARGIN_SPACING,
	    XmNleftAttachment, XmATTACH_POSITION,
	    XmNleftPosition, 1, NULL);
    XmStringFree(s1);

    fd->boldErrW = XtVaCreateManagedWidget("boldErrLbl",
    	    xmLabelGadgetClass, highlightForm,
    	    XmNlabelString, s1=XmStringCreateSimple(""),
    	    XmNalignment, XmALIGNMENT_END,
	    XmNtopAttachment, XmATTACH_WIDGET,
	    XmNtopWidget, italicBtn,
	    XmNtopOffset, MARGIN_SPACING,
	    XmNleftAttachment, XmATTACH_WIDGET,
	    XmNleftWidget, boldLbl,
	    XmNrightAttachment, XmATTACH_POSITION,
	    XmNrightPosition, 99, NULL);
    XmStringFree(s1);

    boldBtn = XtVaCreateManagedWidget("boldBtn",
    	    xmPushButtonWidgetClass, highlightForm,
    	    XmNlabelString, s1=XmStringCreateSimple("Browse..."),
    	    XmNmnemonic, 'w',
    	    XmNtopAttachment, XmATTACH_WIDGET,
	    XmNtopWidget, boldLbl,
	    XmNtopOffset, BTN_TEXT_OFFSET,
	    XmNleftAttachment, XmATTACH_POSITION,
	    XmNleftPosition, 1, NULL);
    XtAddCallback(boldBtn, XmNactivateCallback, boldBrowseCB, fd);

    fd->boldW = XtVaCreateManagedWidget("bold", xmTextWidgetClass,
    	    highlightForm,
    	    XmNmaxLength, MAX_FONT_LEN,
	    XmNleftAttachment, XmATTACH_WIDGET,
	    XmNleftWidget, boldBtn,
	    XmNtopAttachment, XmATTACH_WIDGET,
	    XmNtopWidget, boldLbl,
	    XmNrightAttachment, XmATTACH_POSITION,
	    XmNrightPosition, 99, NULL);
    RemapDeleteKey(fd->boldW);
    XtAddCallback(fd->boldW, XmNvalueChangedCallback,
    	    boldModifiedCB, fd);
    XtVaSetValues(boldLbl, XmNuserData, fd->boldW, NULL);

    boldItalicLbl = XtVaCreateManagedWidget("boldItalicLbl", xmLabelGadgetClass,
    	    highlightForm,
    	    XmNlabelString, s1=XmStringCreateSimple("Bold Italic Font"),
    	    XmNmnemonic, 'l',
    	    XmNalignment, XmALIGNMENT_BEGINNING,
	    XmNtopAttachment, XmATTACH_WIDGET,
	    XmNtopWidget, boldBtn,
	    XmNtopOffset, MARGIN_SPACING,
	    XmNleftAttachment, XmATTACH_POSITION,
	    XmNleftPosition, 1, NULL);
    XmStringFree(s1);

    fd->boldItalicErrW = XtVaCreateManagedWidget("boldItalicErrLbl",
    	    xmLabelGadgetClass, highlightForm,
    	    XmNlabelString, s1=XmStringCreateSimple(""),
    	    XmNalignment, XmALIGNMENT_END,
	    XmNtopAttachment, XmATTACH_WIDGET,
	    XmNtopWidget, boldBtn,
	    XmNtopOffset, MARGIN_SPACING,
	    XmNleftAttachment, XmATTACH_WIDGET,
	    XmNleftWidget, boldItalicLbl,
	    XmNrightAttachment, XmATTACH_POSITION,
	    XmNrightPosition, 99, NULL);
    XmStringFree(s1);

    boldItalicBtn = XtVaCreateManagedWidget("boldItalicBtn",
    	    xmPushButtonWidgetClass, highlightForm,
    	    XmNlabelString, s1=XmStringCreateSimple("Browse..."),
    	    XmNmnemonic, 's',
    	    XmNtopAttachment, XmATTACH_WIDGET,
	    XmNtopWidget, boldItalicLbl,
	    XmNtopOffset, BTN_TEXT_OFFSET,
	    XmNleftAttachment, XmATTACH_POSITION,
	    XmNleftPosition, 1, NULL);
    XtAddCallback(boldItalicBtn, XmNactivateCallback, boldItalicBrowseCB, fd);

    fd->boldItalicW = XtVaCreateManagedWidget("boldItalic",
    	    xmTextWidgetClass, highlightForm,
    	    XmNmaxLength, MAX_FONT_LEN,
	    XmNleftAttachment, XmATTACH_WIDGET,
	    XmNleftWidget, boldItalicBtn,
	    XmNtopAttachment, XmATTACH_WIDGET,
	    XmNtopWidget, boldItalicLbl,
	    XmNrightAttachment, XmATTACH_POSITION,
	    XmNrightPosition, 99, NULL);
    RemapDeleteKey(fd->boldItalicW);
    XtAddCallback(fd->boldItalicW, XmNvalueChangedCallback,
    	    boldItalicModifiedCB, fd);
    XtVaSetValues(boldItalicLbl, XmNuserData, fd->boldItalicW, NULL);

    okBtn = XtVaCreateManagedWidget("ok", xmPushButtonWidgetClass, form,
    	    XmNlabelString, s1=XmStringCreateSimple("OK"),
    	    XmNtopAttachment, XmATTACH_WIDGET,
	    XmNtopWidget, highlightFrame,
	    XmNtopOffset, MARGIN_SPACING,
    	    XmNleftAttachment, XmATTACH_POSITION,
    	    XmNleftPosition, forWindow ? 13 : 26,
    	    XmNrightAttachment, XmATTACH_POSITION,
    	    XmNrightPosition, forWindow ? 27 : 40, NULL);
    XtAddCallback(okBtn, XmNactivateCallback, fontOkCB, fd);
    XmStringFree(s1);

    if (forWindow) {
	applyBtn = XtVaCreateManagedWidget("apply",xmPushButtonWidgetClass,form,
    		XmNlabelString, s1=XmStringCreateSimple("Apply"),
    		XmNmnemonic, 'A',
    		XmNtopAttachment, XmATTACH_WIDGET,
		XmNtopWidget, highlightFrame,
		XmNtopOffset, MARGIN_SPACING,
    		XmNleftAttachment, XmATTACH_POSITION,
    		XmNleftPosition, 43,
    		XmNrightAttachment, XmATTACH_POSITION,
    		XmNrightPosition, 57, NULL);
	XtAddCallback(applyBtn, XmNactivateCallback, fontApplyCB, fd);
	XmStringFree(s1);
    }
    
    dismissBtn = XtVaCreateManagedWidget("dismiss",xmPushButtonWidgetClass,form,
    	    XmNlabelString, s1=XmStringCreateSimple("Dismiss"),
    	    XmNtopAttachment, XmATTACH_WIDGET,
	    XmNtopWidget, highlightFrame,
	    XmNtopOffset, MARGIN_SPACING,
    	    XmNleftAttachment, XmATTACH_POSITION,
    	    XmNleftPosition, forWindow ? 73 : 59,
    	    XmNrightAttachment, XmATTACH_POSITION,
    	    XmNrightPosition, forWindow ? 87 : 73, NULL);
    XtAddCallback(dismissBtn, XmNactivateCallback, fontDismissCB, fd);
    XmStringFree(s1);
 
    /* Set initial default button */
    XtVaSetValues(form, XmNdefaultButton, okBtn, NULL);
    XtVaSetValues(form, XmNcancelButton, dismissBtn, NULL);
    
    /* Set initial values */
    if (forWindow) {
	XmTextSetString(fd->primaryW, window->fontName);
	XmTextSetString(fd->boldW, window->boldFontName);
	XmTextSetString(fd->italicW, window->italicFontName);
	XmTextSetString(fd->boldItalicW, window->boldItalicFontName);
    } else {
    	XmTextSetString(fd->primaryW, GetPrefFontName());
	XmTextSetString(fd->boldW, GetPrefBoldFontName());
	XmTextSetString(fd->italicW, GetPrefItalicFontName());
	XmTextSetString(fd->boldItalicW, GetPrefBoldItalicFontName());
    }
    
    /* Handle mnemonic selection of buttons and focus to dialog */
    AddDialogMnemonicHandler(form, FALSE);
    
    /* put up dialog */
    ManageDialogCenteredOnPointer(form);
}

static void fillFromPrimaryCB(Widget w, XtPointer clientData,
    	XtPointer callData)
{
    fontDialog *fd = (fontDialog *)clientData;
    char *primaryName, *errMsg, modifiedFontName[MAX_FONT_LEN];
    char *searchString = "(-[^-]*-[^-]*)-([^-]*)-([^-]*)-(.*)";
    char *italicReplaceString = "\\1-\\2-o-\\4";
    char *boldReplaceString = "\\1-bold-\\3-\\4";
    char *boldItalicReplaceString = "\\1-bold-o-\\4";
    regexp *compiledRE;

    /* Match the primary font agains RE pattern for font names.  If it
       doesn't match, we can't generate highlight font names, so return */
    compiledRE = CompileRE(searchString, &errMsg, REDFLT_STANDARD);
    primaryName = XmTextGetString(fd->primaryW);
    if (!ExecRE(compiledRE, NULL, primaryName, NULL, False, '\0', '\0', NULL)) {
    	XBell(XtDisplay(fd->shell), 0);
    	free(compiledRE);
    	XtFree(primaryName);
    	return;
    }
    
    /* Make up names for new fonts based on RE replace patterns */
    SubstituteRE(compiledRE, italicReplaceString, modifiedFontName,
    	    MAX_FONT_LEN);
    XmTextSetString(fd->italicW, modifiedFontName);
    SubstituteRE(compiledRE, boldReplaceString, modifiedFontName,
    	    MAX_FONT_LEN);
    XmTextSetString(fd->boldW, modifiedFontName);
    SubstituteRE(compiledRE, boldItalicReplaceString, modifiedFontName,
    	    MAX_FONT_LEN);
    XmTextSetString(fd->boldItalicW, modifiedFontName);
    XtFree(primaryName);
    free(compiledRE);
}

static void primaryModifiedCB(Widget w, XtPointer clientData,
	XtPointer callData)
{
    fontDialog *fd = (fontDialog *)clientData;

    showFontStatus(fd, fd->italicW, fd->italicErrW);
    showFontStatus(fd, fd->boldW, fd->boldErrW);
    showFontStatus(fd, fd->boldItalicW, fd->boldItalicErrW);
}
static void italicModifiedCB(Widget w, XtPointer clientData, XtPointer callData)
{
    fontDialog *fd = (fontDialog *)clientData;

    showFontStatus(fd, fd->italicW, fd->italicErrW);
}
static void boldModifiedCB(Widget w, XtPointer clientData, XtPointer callData)
{
    fontDialog *fd = (fontDialog *)clientData;

    showFontStatus(fd, fd->boldW, fd->boldErrW);
}
static void boldItalicModifiedCB(Widget w, XtPointer clientData,
    	XtPointer callData)
{
    fontDialog *fd = (fontDialog *)clientData;

    showFontStatus(fd, fd->boldItalicW, fd->boldItalicErrW);
}

static void primaryBrowseCB(Widget w, XtPointer clientData, XtPointer callData)
{
    fontDialog *fd = (fontDialog *)clientData;

    browseFont(fd->shell, fd->primaryW);
}
static void italicBrowseCB(Widget w, XtPointer clientData, XtPointer callData)
{
    fontDialog *fd = (fontDialog *)clientData;

    browseFont(fd->shell, fd->italicW);
}
static void boldBrowseCB(Widget w, XtPointer clientData, XtPointer callData)
{
    fontDialog *fd = (fontDialog *)clientData;

    browseFont(fd->shell, fd->boldW);
}
static void boldItalicBrowseCB(Widget w, XtPointer clientData,
    	XtPointer callData)
{
   fontDialog *fd = (fontDialog *)clientData;

   browseFont(fd->shell, fd->boldItalicW);
}

static void fontDestroyCB(Widget w, XtPointer clientData, XtPointer callData)
{
    fontDialog *fd = (fontDialog *)clientData;
    
    fd->window->fontDialog = NULL;
    XtFree((char *)fd);
}

static void fontOkCB(Widget w, XtPointer clientData, XtPointer callData)
{
    fontDialog *fd = (fontDialog *)clientData;

    updateFonts(fd);

    /* pop down and destroy the dialog */
    XtDestroyWidget(fd->shell);
}

static void fontApplyCB(Widget w, XtPointer clientData, XtPointer callData)
{
    fontDialog *fd = (fontDialog *)clientData;

    updateFonts(fd);
}

static void fontDismissCB(Widget w, XtPointer clientData, XtPointer callData)
{
    fontDialog *fd = (fontDialog *)clientData;

    /* pop down and destroy the dialog */
    XtDestroyWidget(fd->shell);
}

/*
** Check over a font name in a text field to make sure it agrees with the
** primary font in height and spacing.
*/
static int checkFontStatus(fontDialog *fd, Widget fontTextFieldW)
{
    char *primaryName, *testName;
    XFontStruct *primaryFont, *testFont;
    Display *display = XtDisplay(fontTextFieldW);
    int primaryWidth, primaryHeight, testWidth, testHeight;
    
    /* Get width and height of the font to check.  Note the test for empty
       name: X11R6 clients freak out X11R5 servers if they ask them to load
       an empty font name, and kill the whole application! */
    testName = XmTextGetString(fontTextFieldW);
    if (testName[0] == '\0') {
	XtFree(testName);
    	return BAD_FONT;
    }
    testFont = XLoadQueryFont(display, testName);
    if (testFont == NULL) {
    	XtFree(testName);
    	return BAD_FONT;
    }
    XtFree(testName);
    testWidth = testFont->min_bounds.width;
    testHeight = testFont->ascent + testFont->descent;
    XFreeFont(display, testFont);
    
    /* Get width and height of the primary font */
    primaryName = XmTextGetString(fd->primaryW);
    if (primaryName[0] == '\0') {
	XtFree(primaryName);
    	return BAD_FONT;
    }
    primaryFont = XLoadQueryFont(display, primaryName);
    if (primaryFont == NULL) {
    	XtFree(primaryName);
    	return BAD_PRIMARY;
    }
    XtFree(primaryName);
    primaryWidth = primaryFont->min_bounds.width;
    primaryHeight = primaryFont->ascent + primaryFont->descent;
    XFreeFont(display, primaryFont);
    
    /* Compare font information */
    if (testWidth != primaryWidth)
    	return BAD_SPACING;
    if (testHeight != primaryHeight)
    	return BAD_SIZE;
    return GOOD_FONT;
}

/*
** Update the error label for a font text field to reflect its validity and degree
** of agreement with the currently selected primary font
*/
static int showFontStatus(fontDialog *fd, Widget fontTextFieldW,
    	Widget errorLabelW)
{
    int status;
    XmString s;
    char *msg;
    
    status = checkFontStatus(fd, fontTextFieldW);
    if (status == BAD_PRIMARY)
    	msg = "(font below may not match primary font)";
    else if (status == BAD_FONT)
    	msg = "(xxx font below is invalid xxx)";
    else if (status == BAD_SIZE)
    	msg = "(height of font below does not match primary)";
    else if (status == BAD_SPACING)
    	msg = "(spacing of font below does not match primary)";
    else
    	msg = "";
    
    XtVaSetValues(errorLabelW, XmNlabelString, s=XmStringCreateSimple(msg),
	    NULL);
    XmStringFree(s);
    return status;
}

/*
** Put up a font selector panel to set the font name in the text widget "fontTextW"
*/
static void browseFont(Widget parent, Widget fontTextW)
{
    char *origFontName, *newFontName;
    
    origFontName = XmTextGetString(fontTextW);
    newFontName = FontSel(parent, PREF_FIXED, origFontName);
    XtFree(origFontName);
    if (newFontName == NULL)
    	return;
    XmTextSetString(fontTextW, newFontName);
    XtFree(newFontName);
}

/*
** Accept the changes in the dialog and set the fonts regardless of errors
*/
static void updateFonts(fontDialog *fd)
{
    char *fontName, *italicName, *boldName, *boldItalicName;
    
    fontName = XmTextGetString(fd->primaryW);
    italicName = XmTextGetString(fd->italicW);
    boldName = XmTextGetString(fd->boldW);
    boldItalicName = XmTextGetString(fd->boldItalicW);
    
    if (fd->forWindow) {
        char *params[4];
        params[0] = fontName;
        params[1] = italicName;
        params[2] = boldName;
        params[3] = boldItalicName;
        XtCallActionProc(fd->window->textArea, "set_fonts", NULL, params, 4);
/*
    	SetFonts(fd->window, fontName, italicName, boldName, boldItalicName);
*/
    }
    else {
    	SetPrefFont(fontName);
    	SetPrefItalicFont(italicName);
    	SetPrefBoldFont(boldName);
    	SetPrefBoldItalicFont(boldItalicName);
    }
    XtFree(fontName);
    XtFree(italicName);
    XtFree(boldName);
    XtFree(boldItalicName);
}

/*
** Change the language mode to the one indexed by "mode", reseting word
** delimiters, syntax highlighting and other mode specific parameters
*/
static void reapplyLanguageMode(WindowInfo *window, int mode, int forceDefaults)
{
    char *delimiters;
    int i, wrapMode, indentStyle, tabDist, emTabDist, highlight, oldEmTabDist;
    int wrapModeIsDef, tabDistIsDef, emTabDistIsDef, indentStyleIsDef;
    int highlightIsDef, haveHighlightPatterns, haveSmartIndentMacros;
    int oldMode = window->languageMode;
    
    /* If the mode is the same, and changes aren't being forced (as might
       happen with Save As...), don't mess with already correct settings */
    if (window->languageMode == mode && !forceDefaults)
	return;
    
    /* Change the mode name stored in the window */
    window->languageMode = mode;
    
    /* Set delimiters for all text widgets */
    if (mode == PLAIN_LANGUAGE_MODE || LanguageModes[mode]->delimiters == NULL)
    	delimiters = GetPrefDelimiters();
    else
    	delimiters = LanguageModes[mode]->delimiters;
    XtVaSetValues(window->textArea, textNwordDelimiters, delimiters, NULL);
    for (i=0; i<window->nPanes; i++)
    	XtVaSetValues(window->textPanes[i], textNautoIndent, delimiters, NULL);
    
    /* Decide on desired values for language-specific parameters.  If a
       parameter was set to its default value, set it to the new default,
       otherwise, leave it alone */
    wrapModeIsDef = window->wrapMode == GetPrefWrap(oldMode);
    tabDistIsDef = BufGetTabDistance(window->buffer) == GetPrefTabDist(oldMode);
    XtVaGetValues(window->textArea, textNemulateTabs, &oldEmTabDist, NULL);
    emTabDistIsDef = oldEmTabDist == GetPrefEmTabDist(oldMode);
    indentStyleIsDef = window->indentStyle == GetPrefAutoIndent(oldMode) ||
	    (GetPrefAutoIndent(oldMode) == SMART_INDENT &&
	     window->indentStyle == AUTO_INDENT &&
	     !SmartIndentMacrosAvailable(LanguageModeName(oldMode)));
    highlightIsDef = window->highlightSyntax == GetPrefHighlightSyntax()
	    || (GetPrefHighlightSyntax() &&
		 FindPatternSet(LanguageModeName(oldMode)) == NULL);
    wrapMode = wrapModeIsDef || forceDefaults ?
    	    GetPrefWrap(mode) : window->wrapMode;
    tabDist = tabDistIsDef || forceDefaults ?
	    GetPrefTabDist(mode) : BufGetTabDistance(window->buffer);
    emTabDist = emTabDistIsDef || forceDefaults ?
	    GetPrefEmTabDist(mode) : oldEmTabDist;
    indentStyle = indentStyleIsDef || forceDefaults ?
    	    GetPrefAutoIndent(mode) : window->indentStyle;
    highlight = highlightIsDef || forceDefaults ? 
	    GetPrefHighlightSyntax() : window->highlightSyntax;
	     
    /* Dim/undim smart-indent and highlighting menu items depending on
       whether patterns/macros are available */
    haveHighlightPatterns = FindPatternSet(LanguageModeName(mode)) != NULL;
    haveSmartIndentMacros = SmartIndentMacrosAvailable(LanguageModeName(mode));
    XtSetSensitive(window->highlightItem, haveHighlightPatterns);
    XtSetSensitive(window->smartIndentItem, haveSmartIndentMacros);
    
    /* Turn off requested options which are not available */
    highlight = haveHighlightPatterns && highlight;
    if (indentStyle == SMART_INDENT && !haveSmartIndentMacros)
	indentStyle = AUTO_INDENT;

    /* Change highlighting */
    window->highlightSyntax = highlight;
    XmToggleButtonSetState(window->highlightItem, highlight, False);
    StopHighlighting(window);
    if (highlight)
    	StartHighlighting(window, False);

    /* Force a change of smart indent macros (SetAutoIndent will re-start) */
    if (window->indentStyle == SMART_INDENT) {
	EndSmartIndent(window);
	window->indentStyle = AUTO_INDENT;
    }
    
    /* set requested wrap, indent, and tabs */
    SetAutoWrap(window, wrapMode);
    SetAutoIndent(window, indentStyle);
    SetTabDist(window, tabDist);
    SetEmTabDist(window, emTabDist);
    
    /* Add/remove language specific menu items */
#ifndef VMS
    UpdateShellMenu(window);
#endif
    UpdateMacroMenu(window);
    UpdateBGMenu(window);
}

/*
** Find and return the name of the appropriate languange mode for
** the file in "window".  Returns a pointer to a string, which will
** remain valid until a change is made to the language modes list.
*/
static int matchLanguageMode(WindowInfo *window)
{
    char *ext, *first200;
    int i, j, fileNameLen, extLen, beginPos, endPos, start;

    /*... look for an explicit mode statement first */
    
    /* Do a regular expression search on for recognition pattern */
    first200 = BufGetRange(window->buffer, 0, 200);
    for (i=0; i<NLanguageModes; i++) {
    	if (LanguageModes[i]->recognitionExpr != NULL) {
    	    if (SearchString(first200, LanguageModes[i]->recognitionExpr,
    	    	    SEARCH_FORWARD, SEARCH_REGEX, False, 0, &beginPos,
    	    	    &endPos, NULL, NULL))
    	    	return i;
    	}
    }
    XtFree(first200);
    
    /* Look at file extension ("@@/" starts a ClearCase version extended path,
       which gets appended after the file extension, and therefore must be
       stripped off to recognize the extension to make ClearCase users happy) */
    fileNameLen = strlen(window->filename);
#ifdef VMS
    if (strchr(window->filename, ';') != NULL)
    	fileNameLen = strchr(window->filename, ';') - window->filename;
#endif
    if (strstr(window->filename, "@@/") != NULL)
      	fileNameLen = strstr(window->filename, "@@/") - window->filename;
    for (i=0; i<NLanguageModes; i++) {
    	for (j=0; j<LanguageModes[i]->nExtensions; j++) {
    	    ext = LanguageModes[i]->extensions[j];
    	    extLen = strlen(ext);
    	    start = fileNameLen - extLen;
#if defined(__VMS) && (__VMS_VER >= 70200000) 
          /* VMS v7.2 has case-preserving filenames */
            if (start >= 0 && !strncasecmp(&window->filename[start], ext, extLen))
               return i;
#else
            if (start >= 0 && !strncmp(&window->filename[start], ext, extLen))  
                return i;
#endif
    	}
    }

    /* no appropriate mode was found */
    return PLAIN_LANGUAGE_MODE;
}

static int loadLanguageModesString(char *inString)
{    
    char *errMsg, *styleName, *inPtr = inString;
    languageModeRec *lm;
    int i;

    for (;;) {
   	
	/* skip over blank space */
	inPtr += strspn(inPtr, " \t\n");
    	
	/* Allocate a language mode structure to return, set unread fields to
	   empty so everything can be freed on errors by freeLanguageModeRec */
	lm = (languageModeRec *)XtMalloc(sizeof(languageModeRec));
	lm->nExtensions = 0;
	lm->recognitionExpr = NULL;
	lm->delimiters = NULL;

	/* read language mode name */
	lm->name = ReadSymbolicField(&inPtr);
	if (lm->name == NULL) {
    	    XtFree((char *)lm);
    	    return modeError(NULL,inString,inPtr,"language mode name required");
	}
	if (!SkipDelimiter(&inPtr, &errMsg))
    	    return modeError(lm, inString, inPtr, errMsg);
    	
    	/* read list of extensions */
    	lm->extensions = readExtensionList(&inPtr,
    	    	&lm->nExtensions);
	if (!SkipDelimiter(&inPtr, &errMsg))
    	    return modeError(lm, inString, inPtr, errMsg);
    	
	/* read the recognition regular expression */
	if (*inPtr == '\n' || *inPtr == '\0' || *inPtr == ':')
    	    lm->recognitionExpr = NULL;
    	else if (!ReadQuotedString(&inPtr, &errMsg, &lm->recognitionExpr))
    	    return modeError(lm, inString,inPtr, errMsg);
	if (!SkipDelimiter(&inPtr, &errMsg))
    	    return modeError(lm, inString, inPtr, errMsg);
    	
    	/* read the indent style */
    	styleName = ReadSymbolicField(&inPtr);
	if (styleName == NULL)
    	    lm->indentStyle = DEFAULT_INDENT;
    	else {
	    for (i=0; i<N_INDENT_STYLES; i++) {
	    	if (!strcmp(styleName, AutoIndentTypes[i])) {
	    	    lm->indentStyle = i;
	    	    break;
	    	}
	    }
	    XtFree(styleName);
	    if (i == N_INDENT_STYLES)
	    	return modeError(lm,inString,inPtr,"unrecognized indent style");
	}
	if (!SkipDelimiter(&inPtr, &errMsg))
    	    return modeError(lm, inString, inPtr, errMsg);
    	
    	/* read the wrap style */
    	styleName = ReadSymbolicField(&inPtr);
	if (styleName == NULL)
    	    lm->wrapStyle = DEFAULT_WRAP;
    	else {
	    for (i=0; i<N_WRAP_STYLES; i++) {
	    	if (!strcmp(styleName, AutoWrapTypes[i])) {
	    	    lm->wrapStyle = i;
	    	    break;
	    	}
	    }
	    XtFree(styleName);
	    if (i == N_WRAP_STYLES)
	    	return modeError(lm, inString, inPtr,"unrecognized wrap style");
	}
	if (!SkipDelimiter(&inPtr, &errMsg))
    	    return modeError(lm, inString, inPtr, errMsg);
    	
    	/* read the tab distance */
	if (*inPtr == '\n' || *inPtr == '\0' || *inPtr == ':')
    	    lm->tabDist = DEFAULT_TAB_DIST;
    	else if (!ReadNumericField(&inPtr, &lm->tabDist))
    	    return modeError(lm, inString, inPtr, "bad tab spacing");
	if (!SkipDelimiter(&inPtr, &errMsg))
    	    return modeError(lm, inString, inPtr, errMsg);
    	
    	/* read emulated tab distance */
    	if (*inPtr == '\n' || *inPtr == '\0' || *inPtr == ':')
    	    lm->emTabDist = DEFAULT_EM_TAB_DIST;
    	else if (!ReadNumericField(&inPtr, &lm->emTabDist))
    	    return modeError(lm, inString, inPtr, "bad emulated tab spacing");
	if (!SkipDelimiter(&inPtr, &errMsg))
    	    return modeError(lm, inString, inPtr, errMsg);
    	
	/* read the delimiters string */
	if (*inPtr == '\n' || *inPtr == '\0')
    	    lm->delimiters = NULL;
    	else if (!ReadQuotedString(&inPtr, &errMsg, &lm->delimiters))
    	    return modeError(lm, inString, inPtr, errMsg);
    	
   	/* pattern set was read correctly, add/replace it in the list */
   	for (i=0; i<NLanguageModes; i++) {
	    if (!strcmp(LanguageModes[i]->name, lm->name)) {
		freeLanguageModeRec(LanguageModes[i]);
		LanguageModes[i] = lm;
		break;
	    }
	}
	if (i == NLanguageModes) {
	    LanguageModes[NLanguageModes++] = lm;
   	    if (NLanguageModes > MAX_LANGUAGE_MODES)
   		return modeError(NULL, inString, inPtr,
   	    		"maximum allowable number of language modes exceeded");
	}
    	
    	/* if the string ends here, we're done */
   	inPtr += strspn(inPtr, " \t\n");
    	if (*inPtr == '\0')
    	    return True;
    }
}

static char *writeLanguageModesString(void)
{
    int i;
    char *outStr, *escapedStr, *str, numBuf[25];
    textBuffer *outBuf;
    
    outBuf = BufCreate();
    for (i=0; i<NLanguageModes; i++) {
    	BufInsert(outBuf, outBuf->length, "\t");
    	BufInsert(outBuf, outBuf->length, LanguageModes[i]->name);
    	BufInsert(outBuf, outBuf->length, ":");
    	BufInsert(outBuf, outBuf->length, str = createExtString(
    	    	LanguageModes[i]->extensions, LanguageModes[i]->nExtensions));
    	XtFree(str);
    	BufInsert(outBuf, outBuf->length, ":");
    	if (LanguageModes[i]->recognitionExpr != NULL) {
    	    BufInsert(outBuf, outBuf->length,
    	    	    str=MakeQuotedString(LanguageModes[i]->recognitionExpr));
    	    XtFree(str);
    	}
    	BufInsert(outBuf, outBuf->length, ":");
    	if (LanguageModes[i]->indentStyle != DEFAULT_INDENT)
    	    BufInsert(outBuf, outBuf->length,
    	    	    AutoIndentTypes[LanguageModes[i]->indentStyle]);
    	BufInsert(outBuf, outBuf->length, ":");
    	if (LanguageModes[i]->wrapStyle != DEFAULT_WRAP)
    	    BufInsert(outBuf, outBuf->length,
    	    	    AutoWrapTypes[LanguageModes[i]->wrapStyle]);
    	BufInsert(outBuf, outBuf->length, ":");
    	if (LanguageModes[i]->tabDist != DEFAULT_TAB_DIST) {
    	    sprintf(numBuf, "%d", LanguageModes[i]->tabDist);
    	    BufInsert(outBuf, outBuf->length, numBuf);
    	}
    	BufInsert(outBuf, outBuf->length, ":");
    	if (LanguageModes[i]->emTabDist != DEFAULT_EM_TAB_DIST) {
    	    sprintf(numBuf, "%d", LanguageModes[i]->emTabDist);
    	    BufInsert(outBuf, outBuf->length, numBuf);
    	}
    	BufInsert(outBuf, outBuf->length, ":");
    	if (LanguageModes[i]->delimiters != NULL) {
    	    BufInsert(outBuf, outBuf->length,
    	    	    str=MakeQuotedString(LanguageModes[i]->delimiters));
    	    XtFree(str);
    	}
    	BufInsert(outBuf, outBuf->length, "\n");
    }
    
    /* Get the output, and lop off the trailing newline */
    outStr = BufGetRange(outBuf, 0, outBuf->length - 1);
    BufFree(outBuf);
    escapedStr = EscapeSensitiveChars(outStr);
    XtFree(outStr);
    return escapedStr;
}

static char *createExtString(char **extensions, int nExtensions)
{
    int e, length = 1;
    char *outStr, *outPtr;

    for (e=0; e<nExtensions; e++)
    	length += strlen(extensions[e]) + 1;
    outStr = outPtr = XtMalloc(length);
    for (e=0; e<nExtensions; e++) {
    	strcpy(outPtr, extensions[e]);
    	outPtr += strlen(extensions[e]);
    	*outPtr++ = ' ';
    }
    if (nExtensions == 0)
    	*outPtr = '\0';
    else
    	*(outPtr-1) = '\0';
    return outStr;
}

static char **readExtensionList(char **inPtr, int *nExtensions)
{
    char *extensionList[MAX_FILE_EXTENSIONS];
    char **retList, *strStart;
    int i, len;
    
    /* skip over blank space */
    *inPtr += strspn(*inPtr, " \t");
    
    for (i=0; i<MAX_FILE_EXTENSIONS && **inPtr!=':' && **inPtr!='\0'; i++) {
    	*inPtr += strspn(*inPtr, " \t");
	strStart = *inPtr;
	while (**inPtr!=' ' && **inPtr!='\t' && **inPtr!=':' && **inPtr!='\0')
	    (*inPtr)++;
    	len = *inPtr - strStart;
    	extensionList[i] = XtMalloc(len + 1);
    	strncpy(extensionList[i], strStart, len);
    	extensionList[i][len] = '\0';
    }
    *nExtensions = i;
    if (i == 0)
    	return NULL;
    retList = (char **)XtMalloc(sizeof(char *) * i);
    memcpy(retList, extensionList, sizeof(char *) * i);
    return retList;
}

int ReadNumericField(char **inPtr, int *value)
{
    int charsRead;
    
    /* skip over blank space */
    *inPtr += strspn(*inPtr, " \t");
    
    if (sscanf(*inPtr, "%d%n", value, &charsRead) != 1)
    	return False;
    *inPtr += charsRead;
    return True;
}

/*
** Parse a symbolic field, skipping initial and trailing whitespace,
** stops on first invalid character or end of string.  Valid characters
** are letters, numbers, _, -, +, $, #, and internal whitespace.  Internal
** whitespace is compressed to single space characters.
*/
char *ReadSymbolicField(char **inPtr)
{
    char *outStr, *outPtr, *strStart, *strPtr;
    int len;
    
    /* skip over initial blank space */
    *inPtr += strspn(*inPtr, " \t");
    
    /* Find the first invalid character or end of string to know how
       much memory to allocate for the returned string */
    strStart = *inPtr;
    while (isalnum(**inPtr) || **inPtr=='_' || **inPtr=='-' ||  **inPtr=='+' ||
    	    **inPtr=='$' || **inPtr=='#' || **inPtr==' ' || **inPtr=='\t')
    	(*inPtr)++;
    len = *inPtr - strStart;
    if (len == 0)
    	return NULL;
    outStr = outPtr = XtMalloc(len + 1);
    
    /* Copy the string, compressing internal whitespace to a single space */
    strPtr = strStart;
    while (strPtr - strStart < len) {
    	if (*strPtr == ' ' || *strPtr == '\t') {
    	    strPtr += strspn(strPtr, " \t");
    	    *outPtr++ = ' ';
    	} else
    	    *outPtr++ = *strPtr++;
    }
    
    /* If there's space on the end, take it back off */
    if (outPtr > outStr && *(outPtr-1) == ' ')
    	outPtr--;
    if (outPtr == outStr) {
    	XtFree(outStr);
    	return NULL;
    }
    *outPtr = '\0';
    return outStr;
}

/*
** parse an individual quoted string.  Anything between
** double quotes is acceptable, quote characters can be escaped by "".
** Returns allocated string "string" containing
** argument minus quotes.  If not successful, returns False with
** (statically allocated) message in "errMsg".
*/
int ReadQuotedString(char **inPtr, char **errMsg, char **string)
{
    char *outPtr, *c;
    
    /* skip over blank space */
    *inPtr += strspn(*inPtr, " \t");
    
    /* look for initial quote */
    if (**inPtr != '\"') {
    	*errMsg = "expecting quoted string";
    	return False;
    }
    (*inPtr)++;
    
    /* calculate max length and allocate returned string */
    for (c= *inPtr; ; c++) {
    	if (*c == '\0') {
    	    *errMsg = "string not terminated";
    	    return False;
    	} else if (*c == '\"') {
    	    if (*(c+1) == '\"')
    	    	c++;
    	    else
    	    	break;
    	}
    }
    
    /* copy string up to end quote, transforming escaped quotes into quotes */
    *string = XtMalloc(c - *inPtr + 1);
    outPtr = *string;
    while (True) {
    	if (**inPtr == '\"') {
    	    if (*(*inPtr+1) == '\"')
    	    	(*inPtr)++;
    	    else
    	    	break;
    	}
    	*outPtr++ = *(*inPtr)++;
    }
    *outPtr = '\0';

    /* skip end quote */
    (*inPtr)++;
    return True;
}

/*
** Replace characters which the X resource file reader considers control
** characters, such that a string will read back as it appears in "string".
** (So far, newline characters are replaced with with \n\<newline> and
** backslashes with \\.  This has not been tested exhaustively, and
** probably should be.  It would certainly be more asthetic if other
** control characters were replaced as well).
**
** Returns an allocated string which must be freed by the caller with XtFree.
*/
char *EscapeSensitiveChars(const char *string)
{
    const char *c;
    char *outStr, *outPtr;
    int length = 0;

    /* calculate length and allocate returned string */
    for (c=string; *c!='\0'; c++) {
    	if (*c == '\\')
    	    length++;
    	else if (*c == '\n')
    	    length += 3;
    	length++;
    }
    outStr = XtMalloc(length + 1);
    outPtr = outStr;
    
    /* add backslashes */
    for (c=string; *c!='\0'; c++) {
    	if (*c == '\\')
    	    *outPtr++ = '\\';
    	else if (*c == '\n') {
    	    *outPtr++ = '\\';
    	    *outPtr++ = 'n';
    	    *outPtr++ = '\\';
    	}
    	*outPtr++ = *c;
    }
    *outPtr = '\0';
    return outStr;
}

/*
** Adds double quotes around a string and escape existing double quote
** characters with two double quotes.  Enables the string to be read back
** by ReadQuotedString.
*/
char *MakeQuotedString(const char *string)
{
    const char *c;
    char *outStr, *outPtr;
    int length = 0;

    /* calculate length and allocate returned string */
    for (c=string; *c!='\0'; c++) {
    	if (*c == '\"')
    	    length++;
    	length++;
    }
    outStr = XtMalloc(length + 3);
    outPtr = outStr;
    
    /* add starting quote */
    *outPtr++ = '\"';
    
    /* copy string, escaping quotes with "" */
    for (c=string; *c!='\0'; c++) {
    	if (*c == '\"')
    	    *outPtr++ = '\"';
    	*outPtr++ = *c;
    }
    
    /* add ending quote */
    *outPtr++ = '\"';

    /* terminate string and return */
    *outPtr = '\0';
    return outStr;
}

/*
** Read a dialog text field containing a symbolic name (language mode names,
** style names, highlight pattern names, colors, and fonts), clean the
** entered text of leading and trailing whitespace, compress all
** internal whitespace to one space character, and check it over for
** colons, which interfere with the preferences file reader/writer syntax.
** Returns NULL on error, and puts up a dialog if silent is False.  Returns
** an empty string if the text field is blank.
*/
char *ReadSymbolicFieldTextWidget(Widget textW, const char *fieldName, int silent)
{
    char *string, *stringPtr, *parsedString;
    
    /* read from the text widget */
    string = stringPtr = XmTextGetString(textW);
    
    /* parse it with the same routine used to read symbolic fields from
       files.  If the string is not read entirely, there are invalid
       characters, so warn the user if not in silent mode. */
    parsedString = ReadSymbolicField(&stringPtr);
    if (*stringPtr != '\0') {
    	if (!silent) {
    	    *(stringPtr+1) = '\0';
    	    DialogF(DF_WARN, textW, 1,"Invalid character \"%s\" in %s",
    	    	    "Dismiss", stringPtr, fieldName);
    	    XmProcessTraversal(textW, XmTRAVERSE_CURRENT);
    	}
    	XtFree(string);
    	if (parsedString != NULL)
    	    XtFree(parsedString);
    	return NULL;
    }
    XtFree(string);
    if (parsedString == NULL) {
    	parsedString = XtMalloc(1);
    	*parsedString = '\0';
    }
    return parsedString;
}

/*
** Create a pulldown menu pane with the names of the current language modes.
** XmNuserData for each item contains the language mode name.
*/
Widget CreateLanguageModeMenu(Widget parent, XtCallbackProc cbProc, void *cbArg)
{
    Widget menu, btn;
    int i;
    XmString s1;

    menu = CreatePulldownMenu(parent, "languageModes", NULL, 0);
    for (i=0; i<NLanguageModes; i++) {
        btn = XtVaCreateManagedWidget("languageMode", xmPushButtonGadgetClass,
        	menu,
        	XmNlabelString, s1=XmStringCreateSimple(LanguageModes[i]->name),
		XmNmarginHeight, 0,
    		XmNuserData, (void *)LanguageModes[i]->name, NULL);
        XmStringFree(s1);
	XtAddCallback(btn, XmNactivateCallback, cbProc, cbArg);
    }
    return menu;
}

/*
** Set the language mode menu in option menu "optMenu" to
** show a particular language mode
*/
void SetLangModeMenu(Widget optMenu, const char *modeName)
{
    int i;
    Cardinal nItems;
    WidgetList items;
    Widget pulldown, selectedItem;
    char *itemName;

    XtVaGetValues(optMenu, XmNsubMenuId, &pulldown, NULL);
    XtVaGetValues(pulldown, XmNchildren, &items, XmNnumChildren, &nItems, NULL);
    if (nItems == 0)
    	return;
    selectedItem = items[0];
    for (i=0; i<(int)nItems; i++) {
    	XtVaGetValues(items[i], XmNuserData, &itemName, NULL);
    	if (!strcmp(itemName, modeName)) {
    	    selectedItem = items[i];
    	    break;
    	}
    }
    XtVaSetValues(optMenu, XmNmenuHistory, selectedItem,NULL);
}

/*
** Create a submenu for chosing language mode for the current window.
*/
Widget CreateLanguageModeSubMenu(WindowInfo *window, Widget parent, char *name,
    	char *label, char mnemonic)
{
    XmString s1=XmStringCreateSimple(label);

    window->langModeCascade = XtVaCreateManagedWidget(name,
    	    xmCascadeButtonGadgetClass, parent, XmNlabelString,
    	    s1, XmNmnemonic, mnemonic,
    	    XmNsubMenuId, NULL, NULL);
    XmStringFree(s1);
    updateLanguageModeSubmenu(window);
    return window->langModeCascade;
}

/*
** Re-build the language mode sub-menu using the current data stored
** in the master list: LanguageModes.
*/
static void updateLanguageModeSubmenu(WindowInfo *window)
{
    int i;
    XmString s1;
    Widget menu, btn;
    Arg args[1] = {{XmNradioBehavior, (XtArgVal)True}};
    
    /* Destroy and re-create the menu pane */
    XtVaGetValues(window->langModeCascade, XmNsubMenuId, &menu, NULL);
    if (menu != NULL)
    	XtDestroyWidget(menu);
    menu = CreatePulldownMenu(XtParent(window->langModeCascade),
    	    "languageModes", args, 1);
    btn = XtVaCreateManagedWidget("languageMode",
            xmToggleButtonGadgetClass, menu,
            XmNlabelString, s1=XmStringCreateSimple("Plain"),
    	    XmNuserData, (void *)PLAIN_LANGUAGE_MODE,
    	    XmNset, window->languageMode==PLAIN_LANGUAGE_MODE, NULL);
    XmStringFree(s1);
    XtAddCallback(btn, XmNvalueChangedCallback, setLangModeCB, window);
    for (i=0; i<NLanguageModes; i++) {
        btn = XtVaCreateManagedWidget("languageMode",
            	xmToggleButtonGadgetClass, menu,
            	XmNlabelString, s1=XmStringCreateSimple(LanguageModes[i]->name),
 	    	XmNmarginHeight, 0,
   		XmNuserData, (void *)i,
    		XmNset, window->languageMode==i, NULL);
        XmStringFree(s1);
	XtAddCallback(btn, XmNvalueChangedCallback, setLangModeCB, window);
    }
    XtVaSetValues(window->langModeCascade, XmNsubMenuId, menu, NULL);
}

static void setLangModeCB(Widget w, XtPointer clientData, XtPointer callData)
{
    WindowInfo *window = (WindowInfo *)clientData;
    char *params[1];
    void *mode;
    
    if (!XmToggleButtonGetState(w))
    	return;
    	
    /* get name of language mode stored in userData field of menu item */
    XtVaGetValues(w, XmNuserData, &mode, NULL);
    
    /* If the mode didn't change, do nothing */
    if (window->languageMode == (int)mode)
    	return;
    
    /* redo syntax highlighting word delimiters, etc. */
/*
    reapplyLanguageMode(window, (int)mode, False);
*/
    params[0] = (((int)mode) == PLAIN_LANGUAGE_MODE) ? "" : LanguageModes[(int)mode]->name;
    XtCallActionProc(window->textArea, "set_language_mode", NULL, params, 1);
}

/*
** Skip a delimiter and it's surrounding whitespace
*/
int SkipDelimiter(char **inPtr, char **errMsg)
{
    *inPtr += strspn(*inPtr, " \t");
    if (**inPtr != ':') {
    	*errMsg = "syntax error";
    	return False;
    }
    (*inPtr)++;
    *inPtr += strspn(*inPtr, " \t");
    return True;
}

/*
** Short-hand error processing for language mode parsing errors, frees
** lm (if non-null), prints a formatted message explaining where the
** error is, and returns False;
*/
static int modeError(languageModeRec *lm, const char *stringStart,
        const char *stoppedAt, const char *message)
{
    if (lm != NULL)
    	freeLanguageModeRec(lm);
    return ParseError(NULL, stringStart, stoppedAt,
    	    "language mode specification", message);
}

/*
** Report parsing errors in resource strings or macros, formatted nicely so
** the user can tell where things became botched.  Errors can be sent either
** to stderr, or displayed in a dialog.  For stderr, pass toDialog as NULL.
** For a dialog, pass the dialog parent in toDialog.
*/
int ParseError(Widget toDialog, const char *stringStart, const char *stoppedAt,
	const char *errorIn, const char *message)
{
    int len, nNonWhite = 0;
    const char *c;
    char *errorLine;
    
    for (c=stoppedAt; c>=stringStart; c--) {
    	if (c == stringStart)
    	    break;
    	else if (*c == '\n' && nNonWhite >= 5)
    	    break;
    	else if (*c != ' ' && *c != '\t')
    	    nNonWhite++;
    }
    len = stoppedAt - c + (*stoppedAt == '\0' ? 0 : 1);
    errorLine = XtMalloc(len+4);
    strncpy(errorLine, c, len);
    errorLine[len++] = '<';
    errorLine[len++] = '=';
    errorLine[len++] = '=';
    errorLine[len] = '\0';
    if (toDialog == NULL)
    	fprintf(stderr, "NEdit: %s in %s:\n%s\n", message, errorIn, errorLine);
    else
    	DialogF(DF_WARN, toDialog, 1, "%s in %s:\n%s", "Dismiss", message,
    	    	errorIn, errorLine);
    XtFree(errorLine);
    return False;
}

/*
** Make a new copy of a string, if NULL, return NULL
*/
char *CopyAllocatedString(const char *string)
{
    char *newString;
    
    if (string == NULL)
    	return NULL;
    newString = XtMalloc(strlen(string)+1);
    strcpy(newString, string);
    return newString;
}

/*
** Compare two strings which may be NULL
*/
int AllocatedStringsDiffer(const char *s1, const char *s2)
{
    if (s1 == NULL && s2 == NULL)
    	return False;
    if (s1 == NULL || s2 == NULL)
    	return True;
    return strcmp(s1, s2);
}

static void updatePatternsTo5dot1(void)
{
    char *htmlDefaultExpr = "^[ \t]*HTML[ \t]*:[ \t]*Default[ \t]*$";
    char *vhdlAnchorExpr = "^[ \t]*VHDL:";
    
    /* Add new patterns if there aren't already existing patterns with
       the same name.  If possible, insert before VHDL in language mode
       list.  If not, just add to end */
    if (!regexFind(TempStringPrefs.highlight, "^[ \t]*PostScript:"))
	spliceString(&TempStringPrefs.highlight, "PostScript:Default",
		vhdlAnchorExpr);
    if (!regexFind(TempStringPrefs.language, "^[ \t]*PostScript:"))
	spliceString(&TempStringPrefs.language,
		"PostScript:.ps .PS .eps .EPS .epsf .epsi::::::",
		vhdlAnchorExpr);
    if (!regexFind(TempStringPrefs.highlight, "^[ \t]*Lex:"))
	spliceString(&TempStringPrefs.highlight, "Lex:Default",
		vhdlAnchorExpr);
    if (!regexFind(TempStringPrefs.language, "^[ \t]*Lex:"))
	spliceString(&TempStringPrefs.language, "Lex:.lex::::::",
		vhdlAnchorExpr);
    if (!regexFind(TempStringPrefs.highlight, "^[ \t]*SQL:"))
	spliceString(&TempStringPrefs.highlight, "SQL:Default",
		vhdlAnchorExpr);
    if (!regexFind(TempStringPrefs.language, "^[ \t]*SQL:"))
	spliceString(&TempStringPrefs.language, "SQL:.sql::::::",
		vhdlAnchorExpr);
    if (!regexFind(TempStringPrefs.highlight, "^[ \t]*Matlab:"))
	spliceString(&TempStringPrefs.highlight, "Matlab:Default",
		vhdlAnchorExpr);
    if (!regexFind(TempStringPrefs.language, "^[ \t]*Matlab:"))
	spliceString(&TempStringPrefs.language, "Matlab:..m .oct .sci::::::",
		vhdlAnchorExpr);
    if (!regexFind(TempStringPrefs.smartIndent, "^[ \t]*Matlab:"))
	spliceString(&TempStringPrefs.smartIndent, "Matlab:Default", NULL);
    if (!regexFind(TempStringPrefs.styles, "^[ \t]*Label:"))
	spliceString(&TempStringPrefs.styles, "Label:red:Italic",
		"^[ \t]*Flag:");
    if (!regexFind(TempStringPrefs.styles, "^[ \t]*Storage Type1:"))
	spliceString(&TempStringPrefs.styles, "Storage Type1:saddle brown:Bold",
		"^[ \t]*String:");

    /* Replace html pattern with sgml html pattern, as long as there
       isn't an existing html pattern which will be overwritten */
    if (regexFind(TempStringPrefs.highlight, htmlDefaultExpr)) {
	regexReplace(&TempStringPrefs.highlight, htmlDefaultExpr,
	    	"SGML HTML:Default");
    	if (!regexReplace(&TempStringPrefs.language, "^[ \t]*HTML:.*$",
	    	"SGML HTML:.sgml .sgm .html .htm:\"\\<(?ihtml)\\>\":::::\n")) {
	    spliceString(&TempStringPrefs.language,
		    "SGML HTML:.sgml .sgm .html .htm:\"\\<(?ihtml)\\>\":::::\n",
		    vhdlAnchorExpr);
	}
    }
}

/*
** Inserts a string into intoString, reallocating it with XtMalloc.  If
** regular expression atExpr is found, inserts the string before atExpr
** followed by a newline.  If atExpr is not found, inserts insertString
** at the end, PRECEDED by a newline.
*/
static void spliceString(char **intoString, char *insertString, char *atExpr)
{
    int beginPos, endPos;
    int intoLen = strlen(*intoString);
    int insertLen = strlen(insertString);
    char *newString = XtMalloc(intoLen + insertLen + 2);
    
    if (atExpr != NULL && SearchString(*intoString, atExpr,
	    SEARCH_FORWARD, SEARCH_REGEX, False, 0, &beginPos, &endPos,
	    NULL, NULL)) {
	strncpy(newString, *intoString, beginPos);
    	strncpy(&newString[beginPos], insertString, insertLen);
	newString[beginPos+insertLen] = '\n';
	strncpy(&newString[beginPos+insertLen+1],
		&((*intoString)[beginPos]), intoLen - beginPos);
    } else {
	strncpy(newString, *intoString, intoLen);
	newString[intoLen] = '\n';
	strncpy(&newString[intoLen+1], insertString, insertLen);
    }
    newString[intoLen + insertLen + 1] = '\0';
    XtFree(*intoString);
    *intoString = newString;
}

/*
** Simplified regular expression search routine which just returns true
** or false depending on whether inString matches expr
*/
static int regexFind(const char *inString, const char *expr)
{
    int beginPos, endPos;
    return SearchString(inString, expr, SEARCH_FORWARD, SEARCH_REGEX, False,
	    0, &beginPos, &endPos, NULL, NULL);
}

/*
** Simplified regular expression replacement routine which replaces the
** first occurence of expr in inString with replaceWith, reallocating
** inString with XtMalloc.  If expr is not found, does nothing and
** returns false.
*/
static int regexReplace(char **inString, const char *expr, const char *replaceWith)
{
    int beginPos, endPos, newLen;
    char *newString;
    int replaceLen = strlen(replaceWith);
    int inLen = strlen(*inString);
    
    if (!SearchString(*inString, expr, SEARCH_FORWARD, SEARCH_REGEX, False,
	    0, &beginPos, &endPos, NULL, NULL))
	return FALSE;
    newLen = inLen + replaceLen - (endPos-beginPos);
    newString = XtMalloc(newLen + 1);
    strncpy(newString, *inString, beginPos);
    strncpy(&newString[beginPos], replaceWith, replaceLen);
    strncpy(&newString[beginPos+replaceLen],
	    &((*inString)[endPos]), inLen - endPos);
    newString[newLen] = '\0';
    XtFree(*inString);
    *inString = newString;
    return TRUE;
}

#ifdef SGI_CUSTOM
/*
** Present the user a dialog for specifying whether or not a short
** menu mode preference should be applied toward the default setting.
** Return False (function value) if operation was canceled, return True
** in setDefault if requested to reset the default value.
*/
static int shortPrefToDefault(Widget parent, char *settingName, int *setDefault)
{
    char msg[100] = "";
    
    if (!GetPrefShortMenus()) {
    	*setDefault = False;
    	return True;
    }
    
    sprintf(msg, "%s\nSave as default for future windows as well?",settingName);
    switch(DialogF (DF_QUES, parent, 3, msg, "Yes", "No", "Cancel")) {
        case 1: /* yes */
            *setDefault = True;
            return True;
        case 2: /* no */
            *setDefault = False;
            return True;
        case 3: /* cancel */
            return False;
    }
    return False; /* not reached */
}
#endif
