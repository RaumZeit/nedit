static const char CVSID[] = "$Id: file.c,v 1.35 2001/11/08 16:05:12 amai Exp $";
/*******************************************************************************
*									       *
* file.c -- Nirvana Editor file i/o					       *
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
*******************************************************************************/
#include <stdio.h>
#include <errno.h>
#include <limits.h>
#include <string.h>
#include <stdlib.h>
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
#include <unistd.h>
#include <fcntl.h>
#endif /*VMS*/

#include <Xm/Xm.h>
#include <Xm/ToggleB.h>
#include <Xm/FileSB.h>
#include <Xm/RowColumn.h>
#include <Xm/Form.h>
#include <Xm/Label.h>
#include "../util/misc.h"
#include "../util/DialogF.h"
#include "../util/fileUtils.h"
#include "../util/getfiles.h"
#include "../util/printUtils.h"
#include "../util/utils.h"
#include "textBuf.h"
#include "text.h"
#include "nedit.h"
#include "window.h"
#include "preferences.h"
#include "undo.h"
#include "menu.h"
#include "file.h"
#include "tags.h"


/* Parameters to algorithm used to auto-detect DOS format files.  NEdit will
   scan up to the lesser of FORMAT_SAMPLE_LINES lines and FORMAT_SAMPLE_CHARS
   characters of the beginning of the file, checking that all newlines are
   paired with carriage returns.  If even a single counterexample exists,
   the file is judged to be in Unix format. */
#define FORMAT_SAMPLE_LINES 5
#define FORMAT_SAMPLE_CHARS 2000

/* Maximum frequency in miliseconds of checking for external modifications.
   The periodic check is only performed on buffer modification, and the check
   interval is only to prevent checking on every keystroke in case of a file
   system which is slow to process stat requests (which I'm not sure exists) */
#define MOD_CHECK_INTERVAL 3000

static int doSave(WindowInfo *window);
static int doOpen(WindowInfo *window, const char *name, const char *path,
     int flags);
static void backupFileName(WindowInfo *window, char *name);
static int writeBckVersion(WindowInfo *window);
static int bckError(WindowInfo *window, const char *errString, const char *file);
static int fileWasModifiedExternally(WindowInfo *window);
static char *errorString(void);
static void addWrapNewlines(WindowInfo *window);
static void setFormatCB(Widget w, XtPointer clientData, XtPointer callData);
static void addWrapCB(Widget w, XtPointer clientData, XtPointer callData);
static int formatOfFile(const char *fileString);
static void convertFromDosFileString(char *inString, int *length);
static void convertFromMacFileString(char *fileString, int length);
static int convertToDosFileString(char **fileString, int *length);
static void convertToMacFileString(char *fileString, int length);
#ifdef VMS
void removeVersionNumber(char *fileName);
#endif /*VMS*/

void EditNewFile(char *geometry, int iconic, const char *languageMode,
	const char *defaultPath)
{
    char name[MAXPATHLEN];
    WindowInfo *window;
    
    /*... test for creatability? */
    
    /* Find a (relatively) unique name for the new file */
    UniqueUntitledName(name);

    /* create the window */
    window = CreateWindow(name, geometry, iconic);
    strcpy(window->filename, name);
    strcpy(window->path, defaultPath ? defaultPath : "");
    window->filenameSet = FALSE;
    window->fileFormat = UNIX_FILE_FORMAT;
    window->lastModTime = 0;
    SetWindowModified(window, FALSE);
    CLEAR_ALL_LOCKS(window->lockReasons);
    UpdateWindowReadOnly(window);
    UpdateStatsLine(window);
    UpdateWindowTitle(window);
    if (languageMode == NULL) 
    	DetermineLanguageMode(window, True);
    else
	SetLanguageMode(window, FindLanguageMode(languageMode), True);
}

/*
** Open an existing file specified by name and path.  Use the window inWindow
** unless inWindow is NULL or points to a window which is already in use
** (displays a file other than Untitled, or is Untitled but modified).  Flags
** can be any of:
**
**	CREATE: 		If file is not found, (optionally) prompt the
**				user whether to create
**	SUPPRESS_CREATE_WARN	When creating a file, don't ask the user
**	PREF_READ_ONLY		Make the file read-only regardless
**
** If languageMode is passed as NULL, it will be determined automatically
** from the file extension or file contents.
*/
WindowInfo *EditExistingFile(WindowInfo *inWindow, const char *name,
        const char *path, int flags, char *geometry, int iconic,
	const char *languageMode)
{
    WindowInfo *window;
    char fullname[MAXPATHLEN];
    
    /* first look to see if file is already displayed in a window */
    window = FindWindowWithFile(name, path);
    if (window != NULL) {
    	RaiseShellWindow(window->shell);
	return window;
    }
    
    /* If an existing window isn't specified; or the window is already
       in use (not Untitled or Untitled and modified), or is currently
       busy running a macro; create the window */
    if (inWindow == NULL || inWindow->filenameSet || inWindow->fileChanged ||
	    inWindow->macroCmdData != NULL)
	window = CreateWindow(name, geometry, iconic);
    else {
    	window = inWindow;
        RaiseShellWindow(window->shell);
    }
    	
    /* Open the file */
    if (!doOpen(window, name, path, flags)) {
    	CloseWindow(window);
    	return NULL;
    }
    
    /* Bring the title bar and statistics line up to date, doOpen does
       not necessarily set the window title or read-only status */
    UpdateWindowTitle(window);
    UpdateWindowReadOnly(window);
    UpdateStatsLine(window);
    
    /* Add the name to the convenience menu of previously opened files */
    strcpy(fullname, path);
    strcat(fullname, name);
    if(GetPrefAlwaysCheckRelativeTagsSpecs())
      	AddRelTagsFile(GetPrefTagFile(), path);
    AddToPrevOpenMenu(fullname);
    
    /* Decide what language mode to use, trigger language specific actions */
    if (languageMode == NULL) 
    	DetermineLanguageMode(window, True);
    else
	SetLanguageMode(window, FindLanguageMode(languageMode), True);
    return window;
}

void RevertToSaved(WindowInfo *window)
{
    char name[MAXPATHLEN], path[MAXPATHLEN];
    int i;
    int insertPositions[MAX_PANES], topLines[MAX_PANES];
    int horizOffsets[MAX_PANES];
    int openFlags = 0;
    Widget text;
    
    /* Can't revert untitled windows */
    if (!window->filenameSet) {
    	DialogF(DF_WARN, window->shell, 1,
    		"Window was never saved, can't re-read", "Dismiss");
    	return;
    }
    
    /* save insert & scroll positions of all of the panes to restore later */
    for (i=0; i<=window->nPanes; i++) {
    	text = i==0 ? window->textArea : window->textPanes[i-1];
    	insertPositions[i] = TextGetCursorPos(text);
    	TextGetScroll(text, &topLines[i], &horizOffsets[i]);
    }

    /* re-read the file, update the window title if new file is different */
    strcpy(name, window->filename);
    strcpy(path, window->path);
    RemoveBackupFile(window);
    ClearUndoList(window);
    openFlags |= IS_USER_LOCKED(window->lockReasons) ? PREF_READ_ONLY : 0;
    if (!doOpen(window, name, path, openFlags)) {
    	CloseWindow(window);
    	return;
    }
    UpdateWindowTitle(window);
    UpdateWindowReadOnly(window);
    
    /* restore the insert and scroll positions of each pane */
    for (i=0; i<=window->nPanes; i++) {
    	text = i==0 ? window->textArea : window->textPanes[i-1];
	TextSetCursorPos(text, insertPositions[i]);
	TextSetScroll(text, topLines[i], horizOffsets[i]);
    }
}

static int doOpen(WindowInfo *window, const char *name, const char *path,
     int flags)
{
    char fullname[MAXPATHLEN];
    struct stat statbuf;
    int fileLen, readLen;
    char *fileString, *c;
    FILE *fp = NULL;
    int fd;
    int resp;
    
    /* initialize lock reasons */
    CLEAR_ALL_LOCKS(window->lockReasons);
    
    /* Update the window data structure */
    strcpy(window->filename, name);
    strcpy(window->path, path);
    window->filenameSet = TRUE;

    /* Get the full name of the file */
    strcpy(fullname, path);
    strcat(fullname, name);
    
    /* Open the file */
#ifdef USE_ACCESS  /* The only advantage of this is if you use clearcase,
    	    	      which messes up the mtime of files opened with r+,
		      even if they're never actually written. */
    {
	if ((fp = fopen(fullname, "r")) != NULL) {
    	    if(access(fullname, W_OK) != 0)
                SET_PERM_LOCKED(window->lockReasons, TRUE);
#else
    fp = fopen(fullname, "rb+");
    if (fp == NULL) {
    	/* Error opening file or file is not writeable */
	fp = fopen(fullname, "rb");
	if (fp != NULL) {
	    /* File is read only */
            SET_PERM_LOCKED(window->lockReasons, TRUE);
#endif
	} else if (flags & CREATE && errno == ENOENT) {
	    /* Give option to create (or to exit if this is the only window) */
	    if (!(flags & SUPPRESS_CREATE_WARN)) {
		if (WindowList == window && window->next == NULL)
	    	    resp = DialogF(DF_WARN, window->shell, 3,
	    	    	    "Can't open %s:\n%s", "New File", "Cancel",
	    	    	    "Exit NEdit", fullname, errorString());
		else
	    	    resp = DialogF(DF_WARN, window->shell, 2,
	    	    	    "Can't open %s:\n%s", "New File", "Cancel",
	    	    	    fullname, errorString());
		if (resp == 2)
	    	    return FALSE;
		else if (resp == 3)
	    	    exit(EXIT_SUCCESS);
	    }
	    /* Test if new file can be created */
	    if ((fd = creat(fullname, 0666)) == -1) {
    		DialogF(DF_ERR, window->shell, 1, "Can't create %s:\n%s",
    			"Dismiss", fullname, errorString());
        	return FALSE;
	    } else {
#ifdef VMS
		/* get correct version number and close before removing */
		getname(fd, fullname);
#endif
		close(fd);
	        remove(fullname);
	    }
	    SetWindowModified(window, FALSE);
            if ((flags & PREF_READ_ONLY) != 0) {
                SET_USER_LOCKED(window->lockReasons, TRUE);
            }
	    UpdateWindowReadOnly(window);
	    return TRUE;
	} else {
	    /* A true error */
	    DialogF(DF_ERR, window->shell, 1, "Could not open %s%s:\n%s",
	    	    "Dismiss", path, name, errorString());
	    return FALSE;
	}
    }
    
    /* Get the length of the file, the protection mode, and the time of the
       last modification to the file */
    if (fstat(fileno(fp), &statbuf) != 0) {
	DialogF(DF_ERR, window->shell, 1, "Error opening %s", "Dismiss", name);
	return FALSE;
    }
    if (S_ISDIR(statbuf.st_mode))  {
	DialogF(DF_ERR, window->shell, 1, "Can't open directory %s", "Dismiss", name);
	return FALSE;
    }
    fileLen = statbuf.st_size;
    window->fileMode = statbuf.st_mode;
    window->lastModTime = statbuf.st_mtime;
    
    /* Allocate space for the whole contents of the file (unfortunately) */
    fileString = (char *)malloc(fileLen+1);  /* +1 = space for null */
    if (fileString == NULL) {
	DialogF(DF_ERR, window->shell, 1, "File is too large to edit",
	    	"Dismiss");
	return FALSE;
    }

    /* Read the file into fileString and terminate with a null */
    readLen = fread(fileString, sizeof(char), fileLen, fp);
    if (ferror(fp)) {
    	DialogF(DF_ERR, window->shell, 1, "Error reading %s:\n%s", "Dismiss",
    		name, errorString());
	free(fileString);
	return FALSE;
    }
    fileString[readLen] = 0;
 
    /* Close the file */
    if (fclose(fp) != 0) {
    	/* unlikely error */
	DialogF(DF_WARN, window->shell, 1, "Unable to close file", "Dismiss");
	/* we read it successfully, so continue */
    }
    
    /* Detect and convert DOS and Macintosh format files */
    window->fileFormat = formatOfFile(fileString);
    if (window->fileFormat == DOS_FILE_FORMAT)
	convertFromDosFileString(fileString, &readLen);
    else if (window->fileFormat == MAC_FILE_FORMAT)
	convertFromMacFileString(fileString, readLen);
    
    /* Display the file contents in the text widget */
    window->ignoreModify = True;
    BufSetAll(window->buffer, fileString);
    window->ignoreModify = False;
    
    /* Check that the length that the buffer thinks it has is the same
       as what we gave it.  If not, there were probably nuls in the file.
       Substitute them with another character.  If that is impossible, warn
       the user, make the file read-only, and force a substitution */
    if (window->buffer->length != readLen) {
    	if (!BufSubstituteNullChars(fileString, readLen, window->buffer)) {
	    resp = DialogF(DF_ERR, window->shell, 2,
"Too much binary data in file.  You may view\n\
it, but not modify or re-save its contents.", "View", "Cancel");
	    if (resp == 2)
		return FALSE;
            SET_TMBD_LOCKED(window->lockReasons, TRUE);
	    for (c=fileString; c<&fileString[readLen]; c++)
    		if (*c == '\0')
    		    *c = 0xfe;
	    window->buffer->nullSubsChar = 0xfe;
	}
	window->ignoreModify = True;
	BufSetAll(window->buffer, fileString);
	window->ignoreModify = False;
    }

    /* Release the memory that holds fileString */
    free(fileString);

    /* Set window title and file changed flag */
    if ((flags & PREF_READ_ONLY) != 0) {
        SET_USER_LOCKED(window->lockReasons, TRUE);
    }
    if (IS_PERM_LOCKED(window->lockReasons)) {
	window->fileChanged = FALSE;
	UpdateWindowTitle(window);
    } else {
	SetWindowModified(window, FALSE);
	if (IS_ANY_LOCKED(window->lockReasons)) {
	    UpdateWindowTitle(window);
        }
    }
    UpdateWindowReadOnly(window);
    
    return TRUE;
}   

int IncludeFile(WindowInfo *window, const char *name)
{
    struct stat statbuf;
    int fileLen, readLen;
    char *fileString;
    FILE *fp = NULL;

    /* Open the file */
    fp = fopen(name, "rb");
    if (fp == NULL) {
	DialogF(DF_ERR, window->shell, 1, "Could not open %s:\n%s",
	    	"Dismiss", name, errorString());
	return FALSE;
    }
    
    /* Get the length of the file */
    if (fstat(fileno(fp), &statbuf) != 0) {
	DialogF(DF_ERR, window->shell, 1, "Error opening %s", "Dismiss", name);
	return FALSE;
    }
    if (S_ISDIR(statbuf.st_mode))  {
	DialogF(DF_ERR, window->shell, 1, "Can't open directory %s", "Dismiss", name);
	return FALSE;
    }
    fileLen = statbuf.st_size;
 
    /* allocate space for the whole contents of the file */
    fileString = (char *)malloc(fileLen+1);  /* +1 = space for null */
    if (fileString == NULL) {
	DialogF(DF_ERR, window->shell, 1, "File is too large to include",
	    	"Dismiss");
	return FALSE;
    }

    /* read the file into fileString and terminate with a null */
    readLen = fread(fileString, sizeof(char), fileLen, fp);
    if (ferror(fp)) {
    	DialogF(DF_ERR, window->shell, 1, "Error reading %s:\n%s", "Dismiss",
    		name, errorString());
	fclose(fp);
	free(fileString);
	return FALSE;
    }
    fileString[readLen] = 0;
    
    /* Detect and convert DOS and Macintosh format files */
    switch (formatOfFile(fileString)) {
        case DOS_FILE_FORMAT:
	    convertFromDosFileString(fileString, &readLen); break;
        case MAC_FILE_FORMAT:
	    convertFromMacFileString(fileString, readLen); break;
    }
    
    /* If the file contained ascii nulls, re-map them */
    if (!BufSubstituteNullChars(fileString, readLen, window->buffer))
	DialogF(DF_ERR, window->shell, 1, "Too much binary data in file",
	    	"Dismiss");
 
    /* close the file */
    if (fclose(fp) != 0) {
    	/* unlikely error */
	DialogF(DF_WARN, window->shell, 1, "Unable to close file", "Dismiss");
	/* we read it successfully, so continue */
    }
    
    /* insert the contents of the file in the selection or at the insert
       position in the window if no selection exists */
    if (window->buffer->primary.selected)
    	BufReplaceSelected(window->buffer, fileString);
    else
    	BufInsert(window->buffer, TextGetCursorPos(window->lastFocus),
    		fileString);

    /* release the memory that holds fileString */
    free(fileString);

    return TRUE;
}

/*
** Close all files and windows, leaving one untitled window
*/
int CloseAllFilesAndWindows(void)
{
    while (WindowList->next != NULL || 
    		WindowList->filenameSet || WindowList->fileChanged) {
    	if (!CloseFileAndWindow(WindowList, PROMPT_SBC_DIALOG_RESPONSE))
    	    return FALSE;
    }
    return TRUE;
}

int CloseFileAndWindow(WindowInfo *window, int preResponse)
{ 
    int response, stat;
    
    /* Make sure that the window is not in iconified state */
    RaiseShellWindow(window->shell);

    /* if window is modified, ask about saving, otherwise just close */
    if (!window->fileChanged) {
       CloseWindow(window);
       /* up-to-date windows don't have outstanding backup files to close */
    } else {
        if (preResponse == PROMPT_SBC_DIALOG_RESPONSE) {
	    response = DialogF(DF_WARN, window->shell, 3,
		    "Save %s before closing?", "Yes", "No", "Cancel",
		    window->filename);
        }
        else {
            response = preResponse;
        }
	if (response == YES_SBC_DIALOG_RESPONSE) {
	    /* Save */
	    stat = SaveWindow(window);
	    if (stat)
	    	CloseWindow(window);
	} else if (response == NO_SBC_DIALOG_RESPONSE) {
	    /* Don't Save */
	    RemoveBackupFile(window);
	    CloseWindow(window);
	} else /* 3 == Cancel */
	    return FALSE;
    }
    return TRUE;
}

int SaveWindow(WindowInfo *window)
{
    int stat;
    
    if (!window->fileChanged || IS_ANY_LOCKED_IGNORING_PERM(window->lockReasons))
    	return TRUE;
    if (!window->filenameSet)
    	return SaveWindowAs(window, NULL, False);

    /* Check for external modifications and warn the user */
    if (GetPrefWarnFileMods() && fileWasModifiedExternally(window)) {
    	if (DialogF(DF_WARN, window->shell, 2,
		"%s has been modified by another program.\n\n"
		"Continuing this operation will overwrite any external\n"
		"modifications to the file since it was opened in NEdit,\n"
		"and your work or someone else's may potentially be lost.\n\n"
		"To preserve the modified file, cancel this operation and\n"
		"use Save As... to save this file under a different name,\n"
		"or Revert to Saved to revert to the modified version.",
		"Continue", "Cancel", window->filename) != 1) {
	    window->lastModTime = 0;	/* Don't warn again */
	    return FALSE;
	}
    }
    
#ifdef VMS
    RemoveBackupFile(window);
    stat = doSave(window);
#else
    if (writeBckVersion(window))
    	return FALSE;
    stat = doSave(window);
    RemoveBackupFile(window);
#endif /*VMS*/
    return stat;
}
    
int SaveWindowAs(WindowInfo *window, const char *newName, int addWrap)
{
    int response, retVal, fileFormat;
    char fullname[MAXPATHLEN], filename[MAXPATHLEN], pathname[MAXPATHLEN];
    WindowInfo *otherWindow;
    
    /* Get the new name for the file */
    if (newName == NULL) {
	response = PromptForNewFile(window, "Save File As:", fullname,
		&fileFormat, &addWrap);
	if (response != GFN_OK)
    	    return FALSE;
	window->fileFormat = fileFormat;
    } else
    	strcpy(fullname, newName);
    
    /* Add newlines if requested */
    if (addWrap)
    	addWrapNewlines(window);
    
    /* If the requested file is this file, just save it and return */
    ParseFilename(fullname, filename, pathname);
    if (!strcmp(window->filename, filename) &&
    	    !strcmp(window->path, pathname)) {
	if (writeBckVersion(window))
    	    return FALSE;
	return doSave(window);
    }
    
    /* If the file is open in another window, make user close it.  Note that
       it is possible for user to close the window by hand while the dialog
       is still up, because the dialog is not application modal, so after
       doing the dialog, check again whether the window still exists. */
    otherWindow = FindWindowWithFile(filename, pathname);
    if (otherWindow != NULL) {
	response = DialogF(DF_WARN, window->shell, 2,
		"%s is open in another NEdit window", "Cancel",
		"Close Other Window", filename);
	if (response == 1)
	    return FALSE;
	if (otherWindow == FindWindowWithFile(filename, pathname))
	    if (!CloseFileAndWindow(otherWindow, PROMPT_SBC_DIALOG_RESPONSE))
	    	return FALSE;
    }
    
    /* Change the name of the file and save it under the new name */
    RemoveBackupFile(window);
    strcpy(window->filename, filename);
    strcpy(window->path, pathname);
    window->filenameSet = TRUE;
    window->fileMode = 0;
    CLEAR_ALL_LOCKS(window->lockReasons);
    retVal = doSave(window);
    UpdateWindowTitle(window);
    UpdateWindowReadOnly(window);
    
    /* Add the name to the convenience menu of previously opened files */
    AddToPrevOpenMenu(fullname);
    
    /* If name has changed, language mode may have changed as well */
    DetermineLanguageMode(window, False);

    return retVal;
}

static int doSave(WindowInfo *window)
{
    char *fileString = NULL;
    char fullname[MAXPATHLEN];
    struct stat statbuf;
    FILE *fp;
    int fileLen;
    
    /* Get the full name of the file */
    strcpy(fullname, window->path);
    strcat(fullname, window->filename);

#ifdef VMS
    /* strip the version number from the file so VMS will begin a new one */
    removeVersionNumber(fullname);
#endif

    /* open the file */
#ifdef VMS
    fp = fopen(fullname, "w", "rfm = stmlf");
#else
    fp = fopen(fullname, "wb");
#endif /* VMS */
    if (fp == NULL) {
    	DialogF(DF_WARN, window->shell, 1, "Unable to save %s:\n%s", "Dismiss",
		window->filename, errorString());
        return FALSE;
    }

#ifdef VMS
    /* get the complete name of the file including the new version number */
    fgetname(fp, fullname);
#endif
    
    /* get the text buffer contents and its length */
    fileString = BufGetAll(window->buffer);
    fileLen = window->buffer->length;
    
    /* If null characters are substituted for, put them back */
    BufUnsubstituteNullChars(fileString, window->buffer);
    
    /* If the file is to be saved in DOS or Macintosh format, reconvert */
    if (window->fileFormat == DOS_FILE_FORMAT) {
	if (!convertToDosFileString(&fileString, &fileLen)) {
	    DialogF(DF_ERR, window->shell, 1, "Out of memory!  Try\n"
		    "saving in Unix format", "Dismiss");
	    return FALSE;
	}
    } else if (window->fileFormat == MAC_FILE_FORMAT)
	convertToMacFileString(fileString, fileLen);
 	
    /* add a terminating newline if the file doesn't already have one for
       Unix utilities which get confused otherwise */
    if (window->fileFormat == UNIX_FILE_FORMAT && fileLen != 0 &&
	    fileString[fileLen-1] != '\n')
    	fileString[fileLen++] = '\n'; 	 /* null terminator no longer needed */
    
    /* write to the file */
#ifdef IBM_FWRITE_BUG
    write(fileno(fp), fileString, fileLen);
#else
    fwrite(fileString, sizeof(char), fileLen, fp);
#endif
    if (ferror(fp)) {
    	DialogF(DF_ERR, window->shell, 1, "%s not saved:\n%s", "Dismiss", 
		window->filename, errorString());
	fclose(fp);
	remove(fullname);
        XtFree(fileString);
	return FALSE;
    }
    
    /* close the file */
    if (fclose(fp) != 0) {
    	DialogF(DF_ERR, window->shell,1,"Error closing file:\n%s", "Dismiss",
		errorString());
        XtFree(fileString);
	return FALSE;
    }

    /* free the text buffer copy returned from XmTextGetString */
    XtFree(fileString);
    
#ifdef VMS
    /* reflect the fact that NEdit is now editing a new version of the file */
    ParseFilename(fullname, window->filename, window->path);
#endif /*VMS*/

    /* success, file was written */
    SetWindowModified(window, FALSE);
    
    /* update the modification time */
    if (stat(fullname, &statbuf) == 0)
	window->lastModTime = statbuf.st_mtime;
    else
	window->lastModTime = 0;

    return TRUE;
}

/*
** Create a backup file for the current window.  The name for the backup file
** is generated using the name and path stored in the window and adding a
** tilde (~) on UNIX and underscore (_) on VMS to the beginning of the name.  
*/
int WriteBackupFile(WindowInfo *window)
{
    char *fileString = NULL;
    char name[MAXPATHLEN];
    FILE *fp;
    int fd, fileLen;
    
    /* Generate a name for the autoSave file */
    backupFileName(window, name);

    /* remove the old backup file.
       Well, this might fail - we'll notice later however. */
    remove(name);
    
    /* open the file, set more restrictive permissions (using default
        permissions was somewhat of a security hole, because permissions were
        independent of those of the original file being edited */
#ifdef VMS
    if ((fp = fopen(name, "w", "rfm = stmlf")) == NULL) {
#else
    if ((fd = open(name, O_CREAT|O_EXCL|O_WRONLY, S_IRUSR | S_IWUSR)) < 0
        || (fp = fdopen(fd, "w")) == NULL) {
#endif /* VMS */
    	DialogF(DF_WARN, window->shell, 1,
    	       "Unable to save backup for %s:\n%s\nAutomatic backup is now off",
    	       "Dismiss", window->filename, errorString());
        window->autoSave = FALSE;
        XmToggleButtonSetState(window->autoSaveItem, FALSE, FALSE);
        return FALSE;
    }

    /* Set VMS permissions */
#ifdef VMS
    chmod(name, S_IRUSR | S_IWUSR);
#endif

    /* get the text buffer contents and its length */
    fileString = BufGetAll(window->buffer);
    fileLen = window->buffer->length;
    
    /* If null characters are substituted for, put them back */
    BufUnsubstituteNullChars(fileString, window->buffer);
    
    /* add a terminating newline if the file doesn't already have one */
    if (fileLen != 0 && fileString[fileLen-1] != '\n')
    	fileString[fileLen++] = '\n'; 	 /* null terminator no longer needed */
    
    /* write out the file */
#ifdef IBM_FWRITE_BUG
    write(fileno(fp), fileString, fileLen);
#else
    fwrite(fileString, sizeof(char), fileLen, fp);
#endif
    if (ferror(fp)) {
    	DialogF(DF_ERR, window->shell, 1,
    	   "Error while saving backup for %s:\n%s\nAutomatic backup is now off",
    	   "Dismiss", window->filename, errorString());
	fclose(fp);
	remove(name);
        XtFree(fileString);
        window->autoSave = FALSE;
	return FALSE;
    }
    
    /* close the backup file */
    if (fclose(fp) != 0) {
	XtFree(fileString);
	return FALSE;
    }

    /* Free the text buffer copy returned from XmTextGetString */
    XtFree(fileString);

    return TRUE;
}

/*
** Remove the backup file associated with this window
*/
void RemoveBackupFile(WindowInfo *window)
{
    char name[MAXPATHLEN];
    
    backupFileName(window, name);
    remove(name);
}

/*
** Generate the name of the backup file for this window from the filename
** and path in the window data structure & write into name
*/
static void backupFileName(WindowInfo *window, char *name)
{
#ifdef VMS
    if (window->filenameSet)
    	sprintf(name, "%s_%s", window->path, window->filename);
    else
    	sprintf(name, "%s_%s", "SYS$LOGIN:", window->filename);
#else
    if (window->filenameSet)
    	sprintf(name, "%s~%s", window->path, window->filename);
    else
    	sprintf(name, "%s/~%s", GetHomeDir(), window->filename);
#endif /*VMS*/
}

/*
** If saveOldVersion is on, copies the existing version of the file to
** <filename>.bck in anticipation of a new version being saved.  Returns
** True if backup fails and user requests that the new file not be written.
*/
static int writeBckVersion(WindowInfo *window)
{
#ifndef VMS
    char fullname[MAXPATHLEN], bckname[MAXPATHLEN];
    struct stat statbuf;
    FILE *inFP, *outFP;
    int fd, fileLen;
    char *fileString;

    /* Do only if version backups are turned on */
    if (!window->saveOldVersion) {
    	return False;
    }
    
    /* Get the full name of the file */
    strcpy(fullname, window->path);
    strcat(fullname, window->filename);
    
    /* Generate name for old version */ {
    if ((int)(strlen(fullname) + 5) > (int)MAXPATHLEN)
    	return bckError(window, "file name too long", window->filename);
    }
    sprintf(bckname, "%s.bck", fullname);

    /* Delete the old backup file */
    /* Errors are ignored; we'll notice them later. */
    unlink(bckname);

    /* open the file being edited.  If there are problems with the
       old file, don't bother the user, just skip the backup */
    inFP = fopen(fullname, "rb");
    if (inFP == NULL) {
    	return FALSE;
    }

    /* find the length of the file */
    if (fstat(fileno(inFP), &statbuf) != 0) {
	return FALSE;
    }
    fileLen = statbuf.st_size;

    /* open the file exclusive and with restrictive permissions. */
#ifdef VMS
    if ((outFP = fopen(bckname, "w", "rfm = stmlf")) == NULL) {
#else
    if ((fd = open(bckname, O_CREAT|O_EXCL|O_WRONLY, S_IRUSR | S_IWUSR)) < 0
        || (outFP = fdopen(fd, "wb")) == NULL) {
#endif /* VMS */
    	fclose(inFP);
        return bckError(window, "Error open backup file", bckname);
    }
#ifdef VMS
    chmod(bckname, S_IRUSR | S_IWUSR);    
#endif
    
    /* Allocate space for the whole contents of the file */
    fileString = (char *)malloc(fileLen);
    if (fileString == NULL) {
    	fclose(inFP);
    	fclose(outFP);
	return bckError(window, "out of memory", bckname);
    }
    
    /* read the file into fileString */
    fread(fileString, sizeof(char), fileLen, inFP);
    if (ferror(inFP)) {
    	fclose(inFP);
    	fclose(outFP);
    	free(fileString);
    	return FALSE;
    }
 
    /* close the input file, ignore any errors */
    fclose(inFP);

    /* write to the file */
#ifdef IBM_FWRITE_BUG
    write(fileno(outFP), fileString, fileLen);
#else
    fwrite(fileString, sizeof(char), fileLen, outFP);
#endif
    if (ferror(outFP)) {
	fclose(outFP);
	remove(bckname);
        XtFree(fileString);
	return bckError(window, errorString(), bckname);
    }
    XtFree(fileString);
    
    /* close the file */
    if (fclose(outFP) != 0)
	return bckError(window, errorString(), bckname);
#endif /* VMS */
	
    return FALSE;
}

/*
** Error processing for writeBckVersion, gives the user option to cancel
** the subsequent save, or continue and optionally turn off versioning
*/
static int bckError(WindowInfo *window, const char *errString, const char *file)
{
    int resp;

    resp = DialogF(DF_ERR, window->shell, 3,
    	    "Couldn't write .bck (last version) file.\n%s: %s",
    	    "Cancel Save", "Turn off Backups", "Continue", file, errString);
    if (resp == 1)
    	return TRUE;
    if (resp == 2) {
    	window->saveOldVersion = FALSE;
    	XmToggleButtonSetState(window->saveLastItem, FALSE, FALSE);
    }
    return FALSE;
}

void PrintWindow(WindowInfo *window, int selectedOnly)
{
    textBuffer *buf = window->buffer;
    selection *sel = &buf->primary;
    char *fileString = NULL;
    int fileLen;
    
    /* get the contents of the text buffer from the text area widget.  Add
       wrapping newlines if necessary to make it match the displayed text */
    if (selectedOnly) {
    	if (!sel->selected) {
    	    XBell(TheDisplay, 0);
	    return;
	}
	if (sel->rectangular) {
    	    fileString = BufGetSelectionText(buf);
    	    fileLen = strlen(fileString);
    	} else
    	    fileString = TextGetWrapped(window->textArea, sel->start, sel->end,
    	    	    &fileLen);
    } else
    	fileString = TextGetWrapped(window->textArea, 0, buf->length, &fileLen);
    
    /* If null characters are substituted for, put them back */
    BufUnsubstituteNullChars(fileString, buf);

        /* add a terminating newline if the file doesn't already have one */
    if (fileLen != 0 && fileString[fileLen-1] != '\n')
    	fileString[fileLen++] = '\n'; 	 /* null terminator no longer needed */
    
    /* Print the string */
    PrintString(fileString, fileLen, window->shell, window->filename);

    /* Free the text buffer copy returned from XmTextGetString */
    XtFree(fileString);
}

/*
** Print a string (length is required).  parent is the dialog parent, for
** error dialogs, and jobName is the print title.
*/
void PrintString(const char *string, int length, Widget parent, const char *jobName)
{
    char tmpFileName[L_tmpnam];    /* L_tmpnam defined in stdio.h */
    FILE *fp;
    int fd;

    /* Generate a temporary file name */
    /*  If the glibc is used, the linker issues a warning at this point. This is
	very thoughtful of him, but does not apply to NEdit. The recommended
	replacement mkstemp(3) uses the same algorithm as NEdit, namely
	    1. Create a filename
	    2. Open the file with the O_CREAT|O_EXCL flags
	So all an attacker can do is a DoS on the print function. */
    tmpnam(tmpFileName);

    /* open the temporary file */
#ifdef VMS
    if ((fp = fopen(tmpFileName, "w", "rfm = stmlf")) == NULL)
#else
    if ((fd = open(tmpFileName, O_CREAT|O_EXCL|O_WRONLY, S_IRUSR | S_IWUSR)) < 0 || (fp = fdopen(fd, "w")) == NULL)
#endif /* VMS */
    {
    	DialogF(DF_WARN, parent, 1, "Unable to write file for printing:\n%s",
		"Dismiss", errorString());
        return;
    }

#ifdef VMS
    chmod(tmpFileName, S_IRUSR | S_IWUSR);    
#endif
    
    /* write to the file */
#ifdef IBM_FWRITE_BUG
    write(fileno(fp), string, length);
#else
    fwrite(string, sizeof(char), length, fp);
#endif
    if (ferror(fp)) {
    	DialogF(DF_ERR, parent, 1, "%s not printed:\n%s", "Dismiss", 
		jobName, errorString());
	fclose(fp); /* should call close(fd) in turn! */
    	remove(tmpFileName);
	return;
    }
    
    /* close the temporary file */
    if (fclose(fp) != 0) {
    	DialogF(DF_ERR, parent, 1, "Error closing temp. print file:\n%s",
		"Dismiss", errorString());
    	remove(tmpFileName);
	return;
    }

    /* Print the temporary file, then delete it and return success */
#ifdef VMS
    strcat(tmpFileName, ".");
    PrintFile(parent, tmpFileName, jobName, True);
#else
    PrintFile(parent, tmpFileName, jobName);
    remove(tmpFileName);
#endif /*VMS*/
    return;
}

/*
** Wrapper for GetExistingFilename which uses the current window's path
** (if set) as the default directory.
*/
int PromptForExistingFile(WindowInfo *window, char *prompt, char *fullname)
{
    char *savedDefaultDir;
    int retVal;
    
    /* Temporarily set default directory to window->path, prompt for file,
       then, if the call was unsuccessful, restore the original default
       directory */
    savedDefaultDir = GetFileDialogDefaultDirectory();
    if (*window->path != '\0')
    	SetFileDialogDefaultDirectory(window->path);
    retVal = GetExistingFilename(window->shell, prompt, fullname);
    if (retVal != GFN_OK)
    	SetFileDialogDefaultDirectory(savedDefaultDir);
    if (savedDefaultDir != NULL)
    	XtFree(savedDefaultDir);
    return retVal;
}

/*
** Wrapper for GetNewFilename which uses the current window's path
** (if set) as the default directory, and asks about embedding newlines
** to make wrapping permanent.
*/
int PromptForNewFile(WindowInfo *window, char *prompt, char *fullname,
    	int *fileFormat, int *addWrap)
{
    int n, retVal;
    Arg args[20];
    XmString s1, s2;
    Widget fileSB, wrapToggle;
    Widget formatForm, formatBtns, unixFormat, dosFormat, macFormat;
    char *savedDefaultDir;
    
    *fileFormat = window->fileFormat;
    
    /* Temporarily set default directory to window->path, prompt for file,
       then, if the call was unsuccessful, restore the original default
       directory */
    savedDefaultDir = GetFileDialogDefaultDirectory();
    if (*window->path != '\0')
    	SetFileDialogDefaultDirectory(window->path);
    
    /* Present a file selection dialog with an added field for requesting
       long line wrapping to become permanent via inserted newlines */
    n = 0;
    XtSetArg(args[n], XmNselectionLabelString, 
    	    s1=XmStringCreateSimple(prompt)); n++;     
    XtSetArg(args[n], XmNdialogStyle, XmDIALOG_FULL_APPLICATION_MODAL); n++;
    XtSetArg(args[n], XmNdialogTitle, s2=XmStringCreateSimple(" ")); n++;
    XtSetArg(args[n], XmNresizePolicy, XmRESIZE_GROW); n++;
    fileSB = CreateFileSelectionDialog(window->shell,"FileSelect",args,n);
    XmStringFree(s1);
    XmStringFree(s2);
    formatForm = XtVaCreateManagedWidget("formatBtns", xmFormWidgetClass,
	    fileSB, NULL);
    formatBtns = XtVaCreateManagedWidget("formatBtns", xmRowColumnWidgetClass,
	    formatForm,
	    XmNradioBehavior, XmONE_OF_MANY,
	    XmNorientation, XmHORIZONTAL,
	    XmNpacking, XmPACK_TIGHT,
	    XmNtopAttachment, XmATTACH_FORM,
	    XmNleftAttachment, XmATTACH_FORM, NULL);
    XtVaCreateManagedWidget("formatBtns", xmLabelWidgetClass, formatBtns,
	    XmNlabelString, s1=XmStringCreateSimple("Format:"), NULL);
    XmStringFree(s1);
    unixFormat = XtVaCreateManagedWidget("unixFormat",
	    xmToggleButtonWidgetClass, formatBtns, XmNlabelString,
	    s1=XmStringCreateSimple("Unix"),
	    XmNset, *fileFormat == UNIX_FILE_FORMAT,
	    XmNuserData, UNIX_FILE_FORMAT,
    	    XmNmarginHeight, 0, XmNalignment, XmALIGNMENT_BEGINNING,
	    XmNmnemonic, 'U', NULL);
    XmStringFree(s1);
    XtAddCallback(unixFormat, XmNvalueChangedCallback, setFormatCB,
    	    fileFormat);
    dosFormat = XtVaCreateManagedWidget("dosFormat",
	    xmToggleButtonWidgetClass, formatBtns, XmNlabelString,
	    s1=XmStringCreateSimple("DOS"),
	    XmNset, *fileFormat == DOS_FILE_FORMAT,
	    XmNuserData, DOS_FILE_FORMAT,
    	    XmNmarginHeight, 0, XmNalignment, XmALIGNMENT_BEGINNING,
	    XmNmnemonic, 'D', NULL);
    XmStringFree(s1);
    XtAddCallback(dosFormat, XmNvalueChangedCallback, setFormatCB,
    	    fileFormat);
    macFormat = XtVaCreateManagedWidget("macFormat",
	    xmToggleButtonWidgetClass, formatBtns, XmNlabelString,
	    s1=XmStringCreateSimple("Macintosh"),
	    XmNset, *fileFormat == MAC_FILE_FORMAT,
	    XmNuserData, MAC_FILE_FORMAT,
    	    XmNmarginHeight, 0, XmNalignment, XmALIGNMENT_BEGINNING,
	    XmNmnemonic, 'M', NULL);
    XmStringFree(s1);
    XtAddCallback(macFormat, XmNvalueChangedCallback, setFormatCB,
    	    fileFormat);
    if (window->wrapMode == CONTINUOUS_WRAP) {
	wrapToggle = XtVaCreateManagedWidget("addWrap",
	    	xmToggleButtonWidgetClass, formatForm, XmNlabelString,
	    	s1=XmStringCreateSimple("Add line breaks where wrapped"),
    		XmNalignment, XmALIGNMENT_BEGINNING,
		XmNmnemonic, 'A',
		XmNtopAttachment, XmATTACH_WIDGET,
		XmNtopWidget, formatBtns,
		XmNleftAttachment, XmATTACH_FORM, NULL);
	XtAddCallback(wrapToggle, XmNvalueChangedCallback, addWrapCB,
    	    	addWrap);
	XmStringFree(s1);
    }
    *addWrap = False;
    XtVaSetValues(XmFileSelectionBoxGetChild(fileSB,
    	    XmDIALOG_FILTER_LABEL), XmNmnemonic, 'l', XmNuserData,
    	    XmFileSelectionBoxGetChild(fileSB, XmDIALOG_FILTER_TEXT), NULL);
    XtVaSetValues(XmFileSelectionBoxGetChild(fileSB,
    	    XmDIALOG_DIR_LIST_LABEL), XmNmnemonic, 'D', XmNuserData,
    	    XmFileSelectionBoxGetChild(fileSB, XmDIALOG_DIR_LIST), NULL);
    XtVaSetValues(XmFileSelectionBoxGetChild(fileSB,
    	    XmDIALOG_LIST_LABEL), XmNmnemonic, 'F', XmNuserData,
    	    XmFileSelectionBoxGetChild(fileSB, XmDIALOG_LIST), NULL);
    XtVaSetValues(XmFileSelectionBoxGetChild(fileSB,
    	    XmDIALOG_SELECTION_LABEL), XmNmnemonic,
    	    prompt[strspn(prompt, "lFD")], XmNuserData,
    	    XmFileSelectionBoxGetChild(fileSB, XmDIALOG_TEXT), NULL);
    AddDialogMnemonicHandler(fileSB, FALSE);
    RemapDeleteKey(XmFileSelectionBoxGetChild(fileSB, XmDIALOG_FILTER_TEXT));
    RemapDeleteKey(XmFileSelectionBoxGetChild(fileSB, XmDIALOG_TEXT));
    retVal = HandleCustomNewFileSB(fileSB, fullname,
    	    window->filenameSet ? window->filename : NULL);

    if (retVal != GFN_OK)
    	SetFileDialogDefaultDirectory(savedDefaultDir);
    if (savedDefaultDir != NULL)
    	XtFree(savedDefaultDir);
    return retVal;
}

/*
** Find a name for an untitled file, unique in the name space of in the opened
** files in this session, i.e. Untitled or Untitled_nn, and write it into
** the string "name".
*/
void UniqueUntitledName(char *name)
{
    WindowInfo *w;
    int i;

   for (i=0; i<INT_MAX; i++) {
    	if (i == 0)
    	    sprintf(name, "Untitled");
    	else
    	    sprintf(name, "Untitled_%d", i);
	for (w=WindowList; w!=NULL; w=w->next)
     	    if (!strcmp(w->filename, name))
    	    	break;
    	if (w == NULL)
    	    break;
    }
}

/*
** Check if the file in the window was changed by an external source.
** and put up a warning dialog if it has.
*/
void CheckForChangesToFile(WindowInfo *window)
{
    static WindowInfo *lastCheckWindow;
    static Time lastCheckTime = 0;
    char fullname[MAXPATHLEN];
    struct stat statbuf;
    Time timestamp;
    FILE *fp;
    int resp;
    
    if(!window->filenameSet)
        return;
    
    /* If last check was very recent, don't impact performance */
    timestamp = XtLastTimestampProcessed(XtDisplay(window->shell));
    if (window == lastCheckWindow &&
	    timestamp - lastCheckTime < MOD_CHECK_INTERVAL)
    	return;
    lastCheckWindow = window;
    lastCheckTime = timestamp;

    /* Get the file mode and modification time */
    strcpy(fullname, window->path);
    strcat(fullname, window->filename);
    if (stat(fullname, &statbuf) != 0)
        return;
    
    /* Check that the file's read-only status is still correct (but
       only if the file can still be opened successfully in read mode) */
    if (window->fileMode != statbuf.st_mode) {
	window->fileMode = statbuf.st_mode;
	if ((fp = fopen(fullname, "r")) != NULL) {
	    int readOnly;
	    fclose(fp);
#ifdef USE_ACCESS
    	    readOnly = access(fullname, W_OK) != 0;
#else
	    if (((fp = fopen(fullname, "r+")) != NULL)) {
		readOnly = FALSE;
		fclose(fp);
	    } else
		readOnly = TRUE;
#endif
            if (IS_PERM_LOCKED(window->lockReasons) != readOnly) {
                SET_PERM_LOCKED(window->lockReasons, readOnly);
		UpdateWindowTitle(window);
		UpdateWindowReadOnly(window);
	    }
	}
    }

    /* Warn the user if the file has been modified, unless checking is
       turned off or the user has already been warned.  Popping up a dialog
       from a focus callback (which is how this routine is usually called)
       seems to catch Motif off guard, and if the timing is just right, the
       dialog can be left with a still active pointer grab from a Motif menu
       which is still in the process of popping down.  The workaround, below,
       of calling XUngrabPointer is inelegant but seems to fix the problem. */
    if (window->lastModTime != 0 && window->lastModTime != statbuf.st_mtime &&
	    GetPrefWarnFileMods()) {
    	window->lastModTime = 0;	/* Inhibit further warnings */
	XUngrabPointer(XtDisplay(window->shell), timestamp);
	if (window->fileChanged)
	    resp = DialogF(DF_WARN, window->shell, 2,
		    "%s has been modified by another program.  Reload\n"
		    "and discard changes made in this edit session?",
		    "Reload", "Dismiss", window->filename);
	else
	    resp = DialogF(DF_WARN, window->shell, 2,
		    "%s has been modified by another\nprogram.  Reload?",
		    "Reload", "Dismiss", window->filename);
	if (resp == 1)
	    RevertToSaved(window);
    }
}

/*
** Return true if the file displayed in window has been modified externally
** to nedit
*/
static int fileWasModifiedExternally(WindowInfo *window)
{    
    char fullname[MAXPATHLEN];
    struct stat statbuf;
    
    if(!window->filenameSet)
        return FALSE;
    if (window->lastModTime == 0)
	return FALSE;
    strcpy(fullname, window->path);
    strcat(fullname, window->filename);
    if (stat(fullname, &statbuf) != 0)
        return FALSE;
    return window->lastModTime != statbuf.st_mtime;
}

/*
** Check the read-only or locked status of the window and beep and return
** false if the window should not be written in.
*/
int CheckReadOnly(WindowInfo *window)
{
    if (IS_ANY_LOCKED(window->lockReasons)) {
    	XBell(TheDisplay, 0);
	return True;
    }
    return False;
}

/*
** Wrapper for strerror so all the calls don't have to be ifdef'd for VMS.
*/
static char *errorString(void)
{
#ifdef VMS
    return strerror(errno, vaxc$errno);
#else
    return strerror(errno);
#endif
}

#ifdef VMS
/*
** Removing the VMS version number from a file name (if has one).
*/
void removeVersionNumber(char *fileName)
{
    char *versionStart;
    
    versionStart = strrchr(fileName, ';');
    if (versionStart != NULL)
    	*versionStart = '\0';
}
#endif /*VMS*/

/*
** Callback procedure for File Format toggle buttons.  Format is stored
** in userData field of widget button
*/
static void setFormatCB(Widget w, XtPointer clientData, XtPointer callData)
{
    if (XmToggleButtonGetState(w))
	XtVaGetValues(w, XmNuserData, clientData, NULL);
}

/*
** Callback procedure for toggle button requesting newlines to be inserted
** to emulate continuous wrapping.
*/
static void addWrapCB(Widget w, XtPointer clientData, XtPointer callData)
{
    int resp;
    int *addWrap = (int *)clientData;
    
    if (XmToggleButtonGetState(w)) {
    	resp = DialogF(DF_WARN, w, 2,
"This operation adds permanent line breaks to\n\
match the automatic wrapping done by the\n\
Continuous Wrap mode Preferences Option.\n\
\n\
      *** This Option is Irreversable ***\n\
\n\
Once newlines are inserted, continuous wrapping\n\
will no longer work automatically on these lines", "OK", "Cancel");
    	if (resp == 2) {
    	    XmToggleButtonSetState(w, False, False);
    	    *addWrap = False;
    	} else
    	    *addWrap = True;
    } else
    	*addWrap = False;
}

/*
** Change a window created in NEdit's continuous wrap mode to the more
** conventional Unix format of embedded newlines.  Indicate to the user
** by turning off Continuous Wrap mode.
*/
static void addWrapNewlines(WindowInfo *window)
{
    int fileLen, i, insertPositions[MAX_PANES], topLines[MAX_PANES];
    int horizOffset;
    Widget text;
    char *fileString;
	
    /* save the insert and scroll positions of each pane */
    for (i=0; i<=window->nPanes; i++) {
    	text = i==0 ? window->textArea : window->textPanes[i-1];
    	insertPositions[i] = TextGetCursorPos(text);
    	TextGetScroll(text, &topLines[i], &horizOffset);
    }

    /* Modify the buffer to add wrapping */
    fileString = TextGetWrapped(window->textArea, 0,
    	    window->buffer->length, &fileLen);
    BufSetAll(window->buffer, fileString);
    XtFree(fileString);

    /* restore the insert and scroll positions of each pane */
    for (i=0; i<=window->nPanes; i++) {
    	text = i==0 ? window->textArea : window->textPanes[i-1];
	TextSetCursorPos(text, insertPositions[i]);
	TextSetScroll(text, topLines[i], 0);
    }

    /* Show the user that something has happened by turning off
       Continuous Wrap mode */
    XmToggleButtonSetState(window->continuousWrapItem, False, True);
}

/*
** Samples up to a maximum of FORMAT_SAMPLE_LINES lines and FORMAT_SAMPLE_CHARS
** characters, to determine whether fileString represents a MS DOS or Macintosh
** format file.  If there's ANY ambiguity (a newline in the sample not paired
** with a return in an otherwise DOS looking file, or a newline appearing in
** the sampled portion of a Macintosh looking file), the file is judged to be
** Unix format.
*/
static int formatOfFile(const char *fileString)
{
    const char *p;
    int nNewlines = 0, nReturns = 0;
    
    for (p=fileString; *p!='\0' && p < fileString + FORMAT_SAMPLE_CHARS; p++) {
	if (*p == '\n') {
	    nNewlines++;
	    if (p == fileString || *(p-1) != '\r')
		return UNIX_FILE_FORMAT;
	    if (nNewlines >= FORMAT_SAMPLE_LINES)
		return DOS_FILE_FORMAT;
	} else if (*p == '\r')
	    nReturns++;
    }
    if (nNewlines > 0)
	return DOS_FILE_FORMAT;
    if (nReturns > 1)
	return MAC_FILE_FORMAT;
    return UNIX_FILE_FORMAT;
}

/*
** Converts a string (which may represent the entire contents of the file)
** from DOS or Macintosh format to Unix format.  Conversion is done in-place.
** In the DOS case, the length will be shorter, and passed length will be
** modified to reflect the new length.
*/
static void convertFromDosFileString(char *fileString, int *length)
{
    char *outPtr = fileString;
    char *inPtr = fileString;
    while (inPtr < fileString + *length) {
    	if (*inPtr == '\r' && *(inPtr + 1) == '\n')
	    inPtr++;
	*outPtr++ = *inPtr++;
    }
    *outPtr = '\0';
    *length = outPtr - fileString;
}
static void convertFromMacFileString(char *fileString, int length)
{
    char *inPtr = fileString;
    while (inPtr < fileString + length) {
        if (*inPtr == '\r' )
            *inPtr = '\n';
        inPtr++;
    }
}

/*
** Converts a string (which may represent the entire contents of the file) from
** Unix to DOS format.  String is re-allocated (with malloc), and length is
** modified.  If allocation fails, which it may, because this can potentially
** be a huge hunk of memory, returns FALSE and no conversion is done.
**
** This could be done more efficiently by asking doSave to allocate some
** extra memory for this, and only re-allocating if it wasn't enough.  If
** anyone cares about the performance or the potential for running out of
** memory on a save, it should probably be redone.
*/
static int convertToDosFileString(char **fileString, int *length)
{
    char *outPtr, *outString;
    char *inPtr = *fileString;
    int inLength = *length;
    int outLength = 0;

    /* How long a string will we need? */
    while (inPtr < *fileString + inLength) {
    	if (*inPtr == '\n')
	    outLength++;
	inPtr++;
	outLength++;
    }
    
    /* Allocate the new string */
    outString = (char *)malloc(outLength + 1);
    if (outString == NULL)
	return FALSE;
    
    /* Do the conversion, free the old string */
    inPtr = *fileString;
    outPtr = outString;
    while (inPtr < *fileString + inLength) {
    	if (*inPtr == '\n')
	    *outPtr++ = '\r';
	*outPtr++ = *inPtr++;
    }
    *outPtr = '\0';
    free(*fileString);
    *fileString = outString;
    *length = outLength;
    return TRUE;
}

/*
** Converts a string (which may represent the entire contents of the file)
** from Unix to Macintosh format.
*/
static void convertToMacFileString(char *fileString, int length)
{
    char *inPtr = fileString;
    
    while (inPtr < fileString + length) {
	if (*inPtr == '\n' )
            *inPtr = '\r';
	inPtr++;
    }
}
