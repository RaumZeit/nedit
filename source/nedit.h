/* $Id: nedit.h,v 1.10 2001/04/03 22:59:38 edg Exp $ */
/*******************************************************************************
*									       *
* nedit.h -- Nirvana Editor common include file				       *
*									       *
*******************************************************************************/

/* Tuning parameters */
#define SEARCHMAX 511		/* Maximum length of search/replace strings */
#define MAX_SEARCH_HISTORY 100	/* Maximum length of search string history */
#define MAX_PANES 6		/* Max # of ADDITIONAL text editing panes
				   that can be added to a window */
#ifndef VMS
#define AUTOSAVE_CHAR_LIMIT 30	/* number of characters user can type before
				   NEdit generates a new backup file */
#else
#define AUTOSAVE_CHAR_LIMIT 80	/* set higher on VMS becaus saving is slower */
#endif /*VMS*/
#define AUTOSAVE_OP_LIMIT 8	/* number of distinct editing operations user
				   can do before NEdit gens. new backup file */
#define MAX_FONT_LEN 100	/* maximum length for a font name */
#define MAX_MARKS 36	    	/* max. # of bookmarks (one per letter & #) */
#define MIN_LINE_NUM_COLS 4 	/* Min. # of columns in line number display */
#define APP_NAME "nedit"	/* application name for loading resources */
#define APP_CLASS "NEdit"	/* application class for loading resources */
#ifdef SGI_CUSTOM
#define SGI_WINDOW_TITLE "nedit: " /* part of window title string for sgi */
#define SGI_WINDOW_TITLE_LEN 7     /* length of SGI_WINDOW_TITLE */
#endif

/* The accumulated list of undo operations can potentially consume huge
   amounts of memory.  These tuning parameters determine how much undo infor-
   mation is retained.  Normally, the list is kept between UNDO_OP_LIMIT and
   UNDO_OP_TRIMTO in length (when the list reaches UNDO_OP_LIMIT, it is
   trimmed to UNDO_OP_TRIMTO then allowed to grow back to UNDO_OP_LIMIT).
   When there are very large amounts of saved text held in the list,
   UNDO_WORRY_LIMIT and UNDO_PURGE_LIMIT take over and cause the list to
   be trimmed back further to keep its size down. */
#define UNDO_PURGE_LIMIT 15000000 /* If undo list gets this large (in bytes),
				     trim it to length of UNDO_PURGE_TRIMTO */
#define UNDO_PURGE_TRIMTO 1	  /* Amount to trim the undo list in a purge */
#define UNDO_WORRY_LIMIT 2000000  /* If undo list gets this large (in bytes),
				     trim it to length of UNDO_WORRY_TRIMTO */
#define UNDO_WORRY_TRIMTO 5	  /* Amount to trim the undo list when memory
				     use begins to get serious */
#define UNDO_OP_LIMIT 400	  /* normal limit for length of undo list */
#define UNDO_OP_TRIMTO 200	  /* size undo list is normally trimmed to
				     when it exceeds UNDO_OP_TRIMTO in length */
#ifdef SGI_CUSTOM
#define MAX_SHORTENED_ITEMS 40    /* max. number of items excluded in short- */
#endif	    	    	    	  /*     menus mode */

enum indentStyle {NO_AUTO_INDENT, AUTO_INDENT, SMART_INDENT};
enum wrapStyle {NO_WRAP, NEWLINE_WRAP, CONTINUOUS_WRAP};
enum fileFormats {UNIX_FILE_FORMAT, DOS_FILE_FORMAT, MAC_FILE_FORMAT};

#define CHARSET (XmStringCharSet)XmSTRING_DEFAULT_CHARSET

#define MKSTRING(string) \
	XmStringCreateLtoR(string, XmSTRING_DEFAULT_CHARSET)
	
#define SET_ONE_RSRC(widget, name, newValue) \
{ \
    static Arg args[1] = {{name, (XtArgVal)0}}; \
    args[0].value = (XtArgVal)newValue; \
    XtSetValues(widget, args, 1); \
}	

#define GET_ONE_RSRC(widget, name, valueAddr) \
{ \
    static Arg args[1] = {{name, (XtArgVal)0}}; \
    args[0].value = (XtArgVal)valueAddr; \
    XtGetValues(widget, args, 1); \
}

/* Record on undo list */
typedef struct _UndoInfo {
    struct _UndoInfo *next;		/* pointer to the next undo record */
    int		type;
    int		startPos;
    int		endPos;
    int 	oldLen;
    char	*oldText;
    char	inUndo;			/* flag to indicate undo command on
    					   this record in progress.  Redirects
    					   SaveUndoInfo to save the next mod-
    					   ifications on the redo list instead
    					   of the undo list. */
    char	restoresToSaved;	/* flag to indicate undoing this
    	    	    	    	    	   operation will restore file to
    	    	    	    	    	   last saved (unmodified) state */
} UndoInfo;

/* Element in bookmark table */
typedef struct {
    char label;
    int cursorPos;
    selection sel;
} Bookmark;

typedef struct _WindowInfo {
    struct _WindowInfo *next;
    Widget	shell;			/* application shell of window */
    Widget	splitPane;		/* paned win. for splitting text area */
    Widget	textArea;		/* the first text editing area widget */
    Widget	textPanes[MAX_PANES];	/* additional ones created on demand */
    Widget	lastFocus;		/* the last pane to have kbd. focus */
    Widget	statsLine;		/* file stats information display */
    Widget  	iSearchForm;	    	/* incremental search line widgets */
    Widget  	iSearchText;
    Widget  	iSearchRegexToggle;
    Widget  	iSearchCaseToggle;
    Widget  	iSearchRevToggle;
    Widget	menuBar;    	    	/* the main menu bar */
    Widget	replaceDlog;		/* replace dialog */
    Widget	replaceText;		/* replace dialog settable widgets... */
    Widget	replaceWithText;
    Widget    	replaceCaseToggle;
    Widget	replaceWordToggle;    
    Widget	replaceRegexToggle;    
    Widget	replaceRevToggle;
    Widget	replaceKeepBtn;
    Widget	replaceBtns;
    Widget	replaceBtn;
#ifdef REPLACE_SCOPE
    Widget	replaceAllBtn;
#else
    Widget	replaceInSelBtn;
#endif
    Widget	replaceSearchTypeBox;
    Widget	replaceFindBtn;
    Widget	replaceAndFindBtn;
    Widget	findDlog;		/* find dialog */
    Widget	findText;		/* find dialog settable widgets... */
    Widget      findCaseToggle;
    Widget      findWordToggle;    
    Widget      findRegexToggle;    
    Widget	findRevToggle;
    Widget	findKeepBtn;
    Widget	findBtns;
    Widget	findBtn;
    Widget	findSearchTypeBox;
    Widget	replaceMultiFileDlog;	/* Replace in multiple files */
    Widget	replaceMultiFileList;
    Widget	replaceMultiFilePathBtn;
    Widget	fontDialog;		/* NULL, unless font dialog is up */
    Widget	readOnlyItem;		/* menu bar settable widgets... */
    Widget	autoSaveItem;
    Widget	saveLastItem;
    Widget	closeItem;
    Widget	printSelItem;
    Widget	undoItem;
    Widget	redoItem;
    Widget	cutItem;
    Widget	copyItem;
    Widget	langModeCascade;
    Widget	findDefItem;
    Widget	autoIndentOffItem;
    Widget	autoIndentItem;
    Widget	smartIndentItem;
    Widget  	noWrapItem;
    Widget  	newlineWrapItem;
    Widget  	continuousWrapItem;
    Widget  statsLineItem;
    Widget  iSearchLineItem;
    Widget  lineNumsItem;
    Widget  showMatchingItem;
    Widget  overtypeModeItem;
    Widget  highlightItem;
    Widget	windowMenuPane;
    Widget	shellMenuPane;
    Widget	macroMenuPane;
    Widget  	bgMenuPane;
    Widget  	prevOpenMenuPane;
    Widget  	prevOpenMenuItem;
    Widget  	unloadTagsMenuPane;
    Widget  	unloadTagsMenuItem;
    Widget	filterItem;
    Widget	autoIndentOffDefItem;
    Widget	autoIndentDefItem;
    Widget	smartIndentDefItem;
    Widget	autoSaveDefItem;
    Widget	saveLastDefItem;
    Widget	noWrapDefItem;
    Widget	newlineWrapDefItem;
    Widget	contWrapDefItem;
    Widget	showMatchingDefItem;
    Widget	highlightOffDefItem;
    Widget	highlightDefItem;
    Widget	searchDlogsDefItem;
    Widget      beepOnSearchWrapDefItem;
    Widget	keepSearchDlogsDefItem;
    Widget	searchWrapsDefItem;
    Widget	sortOpenPrevDefItem;
    Widget	allTagsDefItem;
    Widget	smartTagsDefItem;
    Widget	reposDlogsDefItem;
    Widget	statsLineDefItem;
    Widget	iSearchLineDefItem;
    Widget	lineNumsDefItem;
    Widget  	modWarnDefItem;
    Widget  	exitWarnDefItem;
    Widget	searchLiteralDefItem;
    Widget	searchCaseSenseDefItem;
    Widget	searchLiteralWordDefItem;
    Widget	searchCaseSenseWordDefItem;
    Widget	searchRegexNoCaseDefItem;
    Widget	searchRegexDefItem;
#ifdef REPLACE_SCOPE
    Widget	replScopeWinDefItem;
    Widget	replScopeSelDefItem;
    Widget	replScopeSmartDefItem;
#endif
    Widget	size24x80DefItem;
    Widget	size40x80DefItem;
    Widget	size60x80DefItem;
    Widget	size80x80DefItem;
    Widget	sizeCustomDefItem;
    Widget	cancelShellItem;
    Widget	learnItem;
    Widget	finishLearnItem;
    Widget	cancelMacroItem;
    Widget	replayItem;
    Widget	repeatItem;
    Widget	splitWindowItem;
    Widget	closePaneItem;
    Widget  	bgMenuUndoItem;
    Widget  	bgMenuRedoItem;
#ifdef SGI_CUSTOM
    Widget	shortMenusDefItem;
    Widget	toggleShortItems[MAX_SHORTENED_ITEMS]; /* Menu items to be
    	    	    	    	    	   managed and unmanaged to toggle
    	    	    	    	    	   short menus on and off */
    int     	nToggleShortItems;
#endif
    char	filename[MAXPATHLEN];	/* name component of file being edited*/
    char	path[MAXPATHLEN];	/* path component of file being edited*/
    int		fileMode;		/* permissions of file being edited */
    int     	fileFormat; 	    	/* whether to save the file straight
    	    	    	    	    	   (Unix format), or convert it to
					   MS DOS style with \r\n line breaks */
    time_t    	lastModTime; 	    	/* time of last modification to file */
    UndoInfo	*undo;			/* info for undoing last operation */
    UndoInfo	*redo;			/* info for redoing last undone op */
    textBuffer	*buffer;		/* holds the text being edited */
    int		nPanes;			/* number of additional text editing
    					   areas, created by splitWindow */
    int		autoSaveCharCount;	/* count of single characters typed
    					   since last backup file generated */
    int		autoSaveOpCount;	/* count of editing operations "" */
    int		undoOpCount;		/* count of stored undo operations */
    int		undoMemUsed;		/* amount of memory (in bytes)
    					   dedicated to the undo list */
    char	fontName[MAX_FONT_LEN];	/* names of the text fonts in use */
    char	italicFontName[MAX_FONT_LEN];
    char	boldFontName[MAX_FONT_LEN];
    char	boldItalicFontName[MAX_FONT_LEN];
    XmFontList	fontList;		/* fontList for the primary font */
    XFontStruct *italicFontStruct;	/* fontStructs for highlighting fonts */
    XFontStruct *boldFontStruct;
    XFontStruct *boldItalicFontStruct;
    XtIntervalId flashTimeoutID;	/* timer procedure id for getting rid
    					   of highlighted matching paren.  Non-
    					   zero val. means highlight is drawn */
    int		flashPos;		/* position saved for erasing matching
    					   paren highlight (if one is drawn) */
    int 	wasSelected;		/* last selection state (for dim/undim
    					   of selection related menu items */
    Boolean	filenameSet;		/* is the window still "Untitled"? */ 
    Boolean	fileChanged;		/* has window been modified? */
    Boolean	readOnly;		/* is current file read only? */
    Boolean	lockWrite;		/* is Read Only selected in menu? */
    Boolean	autoSave;		/* is autosave turned on? */
    Boolean	saveOldVersion;		/* keep old version in filename.bck */
    char	indentStyle;		/* whether/how to auto indent */
    char	wrapMode;		/* line wrap style: NO_WRAP,
    	    	    	    	    	   NEWLINE_WRAP or CONTINUOUS_WRAP */
    Boolean	overstrike;		/* is overstrike mode turned on ? */
    Boolean	showMatching;		/* is paren matching mode on? */
    Boolean	showStats;		/* is stats line supposed to be shown */
    Boolean 	showISearchLine;    	/* is incr. search line to be shown */
    Boolean 	showLineNumbers;    	/* is the line number display shown */
    Boolean	highlightSyntax;	/* is syntax highlighting turned on? */
    Boolean	modeMessageDisplayed;	/* special stats line banner for learn
    					   and shell command executing modes */
    Boolean	ignoreModify;		/* ignore modifications to text area */
    Boolean	windowMenuValid;	/* is window menu is up to date? */
    Boolean	prevOpenMenuValid;	/* Prev. Opened Files menu up to date?*/
    int		rHistIndex, fHistIndex;	/* history placeholders for */
    int     	iSearchHistIndex;	/*   find and replace dialogs */
    int     	iSearchStartPos;    	/* start pos. of current incr. search */
    int     	nMarks;     	    	/* number of active bookmarks */
    XtIntervalId markTimeoutID;	    	/* backup timer for mark event handler*/
    Bookmark	markTable[MAX_MARKS];	/* marked locations in window */
    void    	*highlightData; 	/* info for syntax highlighting */
    void    	*shellCmdData;  	/* when a shell command is executing,
    	    	    	    	    	   info. about it, otherwise, NULL */
    void    	*macroCmdData;  	/* same for macro commands */
    void    	*smartIndentData;   	/* compiled macros for smart indent */
    int    	languageMode;	    	/* identifies language mode currently
    	    	    	    	    	   selected in the window */
    Boolean	multiFileReplSelected;	/* selected during last multi-window 
					   replacement operation (history) */
    struct _WindowInfo**		/* temporary list of writable windows */
		writableWindows;	/* used during multi-file replacements */
    int		nWritableWindows;	/* number of elements in the list */
    Bool 	multiFileBusy;		/* suppresses multiple beeps/dialogs
					   during multi-file replacements */
    Bool 	replaceFailed;		/* flags replacements failures during
					   multi-file replacements */
    Bool	replaceLastRegexCase;   /* last state of the case sense button
                                           in regex mode for replace dialog */
    Bool	replaceLastLiteralCase; /* idem, for literal mode */
    Bool	iSearchLastRegexCase;   /* idem, for regex mode in 
                                           incremental search bar */
    Bool	iSearchLastLiteralCase; /* idem, for literal mode */
    Bool	findLastRegexCase; 	/* idem, for regex mode in find dialog */
    Bool	findLastLiteralCase;    /* idem, for literal mode */
    
#ifdef REPLACE_SCOPE
    int		replaceScope;		/* Current scope for replace dialog */
    Widget	replaceScopeWinToggle;	/* Scope for replace = window */
    Widget	replaceScopeSelToggle;	/* Scope for replace = selection */
    Widget	replaceScopeMultiToggle;/* Scope for replace = multiple files */
#endif
} WindowInfo;

extern WindowInfo *WindowList;
extern Display *TheDisplay;
extern char *ArgV0;
Boolean IsServer;
