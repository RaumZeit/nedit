/*******************************************************************************
*									       *
* file.c -- Nirvana Editor file i/o					       *
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
#include <sys/param.h>
#include <fcntl.h>
#endif /*VMS*/
#include <Xm/Xm.h>
#include <Xm/ToggleB.h>
#include <Xm/FileSB.h>
#include "../util/misc.h"
#include "../util/DialogF.h"
#include "../util/fileUtils.h"
#include "../util/getfiles.h"
#include "../util/printUtils.h"
#include "textBuf.h"
#include "text.h"
#include "nedit.h"
#include "window.h"
#include "preferences.h"
#include "undo.h"
#include "menu.h"
#include "file.h"

static int doSave(WindowInfo *window);
static int doOpen(WindowInfo *window, char *name, char *path, int flags);
static void backupFileName(WindowInfo *window, char *name);
static int writeBckVersion(WindowInfo *window);
static int bckError(WindowInfo *window, char *errString, char *file);
static char *errorString(void);
static void addWrapNewlines(WindowInfo *window);
static void addWrapCB(Widget w, XtPointer clientData, XtPointer callData);
#ifdef VMS
void removeVersionNumber(char *fileName);
#endif /*VMS*/

void EditNewFile(void)
{
    char name[MAXPATHLEN];
    WindowInfo *window;
    
    /*... test for creatability? */
    
    /* Find a (relatively) unique name for the new file */
    UniqueUntitledName(name);

    /* create the window */
    window = CreateWindow(name);
    strcpy(window->filename, name);
    strcpy(window->path, "");
    window->filenameSet = FALSE;
    SetWindowModified(window, FALSE);
    window->readOnly = FALSE;
    window->lockWrite = FALSE;
    UpdateWindowReadOnly(window);
    UpdateStatsLine(window);
    DetermineLanguageMode(window, True);
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
**	FORCE_READ_ONLY		Make the file read-only regardless
**
*/
WindowInfo *EditExistingFile(WindowInfo *inWindow, char *name, char *path,
	int flags)
{
    WindowInfo *window;
    char fullname[MAXPATHLEN];
    
    /* first look to see if file is already displayed in a window */
    window = FindWindowWithFile(name, path);
    if (window != NULL) {
    	RaiseShellWindow(window->shell);
	return window;
    }
    
    /* If an existing window isn't specified, or the window is already
       in use (not Untitled or Untitled and modified), create the window */
    if (inWindow == NULL || inWindow->filenameSet || inWindow->fileChanged)
	window = CreateWindow(name);
    else
    	window = inWindow;
    	
    /* Open the file */
    if (!doOpen(window, name, path, flags)) {
    	CloseWindow(window);
    	return NULL;
    }
    
    /* Bring the title bar and statistics line up to date, doOpen does
       not necessarily set the window title or read-only status */
    if (inWindow != NULL) {
    	UpdateWindowTitle(window);
    	UpdateWindowReadOnly(window);
    }
    UpdateStatsLine(window);
    
    /* Add the name to the convenience menu of previously opened files */
    strcpy(fullname, path);
    strcat(fullname, name);
    AddToPrevOpenMenu(fullname);
    
    /* Decide what language mode to use, trigger language specific actions */
    DetermineLanguageMode(window, True);
    return window;
}

void RevertToSaved(WindowInfo *window)
{
    char name[MAXPATHLEN], path[MAXPATHLEN];
    int i;
    int insertPositions[MAX_PANES], topLines[MAX_PANES];
    int horizOffsets[MAX_PANES];
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
    if (!doOpen(window, name, path, 0)) {
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

static int doOpen(WindowInfo *window, char *name, char *path, int flags)
{
    char fullname[MAXPATHLEN];
    struct stat statbuf;
    int fileLen, readLen;
    char *fileString, *c;
    FILE *fp = NULL;
    int fd;
    int readOnly = FALSE;
    int resp;
    
    /* Update the window data structure */
    strcpy(window->filename, name);
    strcpy(window->path, path);
    window->filenameSet = TRUE;

    /* Get the full name of the file */
    strcpy(fullname, path);
    strcat(fullname, name);
    
    /* Open the file */
    if ((fp = fopen(fullname, "r+")) == NULL) {
    	/* Error opening file or file is not writeable */
	if ((fp = fopen(fullname, "r")) != NULL) {
	    /* File is read only */
	    readOnly = TRUE;
	} else if (flags & CREATE && errno == ENOENT) {
	    /* Give option to create (or to exit if this is the only window) */
	    if (!(flags & SUPPRESS_CREATE_WARN)) {
		if (WindowList == window && window->next == NULL)
	    	    resp = DialogF(DF_WARN, window->shell, 3,
	    	    	    "Can't open %s:\n%s", "Create", "Cancel",
	    	    	    "Exit NEdit", fullname, errorString());
		else
	    	    resp = DialogF(DF_WARN, window->shell, 2,
	    	    	    "Can't open %s:\n%s", "Create", "Cancel",
	    	    	    fullname, errorString());
		if (resp == 2)
	    	    return FALSE;
		else if (resp == 3)
	    	    exit(0);
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
		close(fd);
#endif
	        remove(fullname);
	    }
	    SetWindowModified(window, FALSE);
	    window->readOnly = FALSE;
	    window->lockWrite = flags & FORCE_READ_ONLY;
	    UpdateWindowReadOnly(window);
	    return TRUE;
	} else {
	    /* A true error */
	    DialogF(DF_ERR, window->shell, 1, "Could not open %s%s:\n%s",
	    	    "Dismiss", path, name, errorString());
	    return FALSE;
	}
    }
    
    /* Get the length of the file and the protection mode */
    if (fstat(fileno(fp), &statbuf) != 0) {
	DialogF(DF_ERR, window->shell, 1, "Error opening %s", "Dismiss", name);
	return FALSE;
    }
    fileLen = statbuf.st_size;
    window->fileMode = statbuf.st_mode;
    
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
	    readOnly = TRUE;
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
    window->lockWrite = flags & FORCE_READ_ONLY;
    if (readOnly) {
	window->readOnly = TRUE;
	window->fileChanged = FALSE;
	UpdateWindowTitle(window);
    } else {
	window->readOnly = FALSE;
	SetWindowModified(window, FALSE);
	if (window->lockWrite)
	    UpdateWindowTitle(window);
    }
    UpdateWindowReadOnly(window);
    
    return TRUE;
}   

int IncludeFile(WindowInfo *window, char *name)
{
    struct stat statbuf;
    int fileLen, readLen;
    char *fileString;
    FILE *fp = NULL;

    /* Open the file */
    fp = fopen(name, "r");
    if (fp == NULL) {
	DialogF(DF_ERR, window->shell, 1, "Could not open %s:\n%s",
	    	"Dismiss", name, errorString());
	return FALSE;
    }
    
    /* Get the length of the file */
    if (fstat(fileno(fp), &statbuf) != 0) {
	DialogF(DF_ERR, window->shell, 1, "Error openinig %s", "Dismiss", name);
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
    	if (!CloseFileAndWindow(WindowList))
    	    return FALSE;
    }
    return TRUE;
}

int CloseFileAndWindow(WindowInfo *window)
{ 
    int response, stat;
    
    /* Make sure that the window is not in iconified state */
    RaiseShellWindow(window->shell);

    /* if window is modified, ask about saving, otherwise just close */
    if (!window->fileChanged) {
       CloseWindow(window);
       /* up-to-date windows don't have outstanding backup files to close */
    } else {
	response = DialogF(DF_WARN, window->shell, 3,
		"Save %s before closing?", "Yes", "No", "Cancel",
		window->filename);
	if (response == 1) {
	    /* Save */
	    stat = SaveWindow(window);
	    if (stat)
	    	CloseWindow(window);
	} else if (response == 2) {
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
    
    if (!window->fileChanged || window->lockWrite)
    	return TRUE;
    if (!window->filenameSet)
    	return SaveWindowAs(window, NULL, False);
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
    
int SaveWindowAs(WindowInfo *window, char *newName, int addWrap)
{
    int response, retVal;
    char fullname[MAXPATHLEN], filename[MAXPATHLEN], pathname[MAXPATHLEN];
    WindowInfo *otherWindow;
    
    /* Get the new name for the file */
    if (newName == NULL) {
	response = PromptForNewFile(window, "Save File As:", fullname,&addWrap);
	if (response != GFN_OK)
    	    return FALSE;
    } else
    	strcpy(fullname, newName);
    
    /* Format conversion: Add newlines if requested */
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
	    if (!CloseFileAndWindow(otherWindow))
	    	return FALSE;
    }
    
    /* Change the name of the file and save it under the new name */
    RemoveBackupFile(window);
    strcpy(window->filename, filename);
    strcpy(window->path, pathname);
    window->filenameSet = TRUE;
    window->readOnly = FALSE;
    window->fileMode = 0;
    window->lockWrite = FALSE;
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
    if ((fp = fopen(fullname, "w", "rfm = stmlf")) == NULL) {
#else
    if ((fp = fopen(fullname, "w")) == NULL) {
#endif /* VMS */
    	DialogF(DF_WARN, window->shell, 1, "Unable to save %s:\n%s", "Dismiss",
		window->filename, errorString());
        return FALSE;
    }

#ifdef VMS
    /* get the complete name of the file including the new version number */
    fgetname(fp, fullname);
        
    /* set the protection for the file */
    if (window->fileMode)
    	chmod(fullname, window->fileMode);
#else /* Unix */
    
    /* set the protection for the file */
    if (window->fileMode)
    	fchmod(fileno(fp), window->fileMode);
#endif /*VMS/Unix*/

    /* get the text buffer contents and its length */
    fileString = BufGetAll(window->buffer);
    fileLen = window->buffer->length;
    
    /* If null characters are substituted for, put them back */
    BufUnsubstituteNullChars(fileString, window->buffer);

    /* add a terminating newline if the file doesn't already have one */
    if (fileLen != 0 && fileString[fileLen-1] != '\n')
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
    int fileLen;
    
    /* Generate a name for the autoSave file */
    backupFileName(window, name);

#ifdef VMS
    /* remove the old backup file because we reuse the same version number */
    remove(name);
#endif /*VMS*/
    
    /* open the file */
#ifdef VMS
    if ((fp = fopen(name, "w", "rfm = stmlf")) == NULL) {
#else
    if ((fp = fopen(name, "w")) == NULL) {
#endif /* VMS */
    	DialogF(DF_WARN, window->shell, 1,
    	       "Unable to save backup for %s:\n%s\nAutomatic backup is now off",
    	       "Dismiss", window->filename, errorString());
        window->autoSave = FALSE;
        XmToggleButtonSetState(window->autoSaveItem, FALSE, FALSE);
        return FALSE;
    }

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
    	sprintf(name, "%s/~%s", getenv("HOME"), window->filename);
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
    int fileLen;
    char *fileString;

    /* Do only if version backups are turned on */
    if (!window->saveOldVersion)
    	return False;
    
    /* Get the full name of the file */
    strcpy(fullname, window->path);
    strcat(fullname, window->filename);
    
    /* Generate name for old version */
    if (strlen(fullname) + 5 > MAXPATHLEN)
    	return bckError(window, "file name too long", window->filename);
    sprintf(bckname, "%s.bck", fullname);

    /* open the file being edited.  If there are problems with the
       old file, don't bother the user, just skip the backup */
    if ((inFP = fopen(fullname, "r")) == NULL)
    	return FALSE;
    
    /* find the length of the file */
    if (fstat(fileno(inFP), &statbuf) != 0)
	return FALSE;
    fileLen = statbuf.st_size;
    
    /* open the file to receive a copy of the old version */
    if ((outFP = fopen(bckname, "w")) == NULL) {
    	fclose(inFP);
    	return bckError(window, errorString(), bckname);
    }
    
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
    
    /* set the protection for the backup file */
    if (window->fileMode)
    	fchmod(fileno(outFP), window->fileMode);
   
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
static int bckError(WindowInfo *window, char *errString, char *file)
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
void PrintString(char *string, int length, Widget parent, char *jobName)
{
    char tmpFileName[L_tmpnam];    /* L_tmpnam defined in stdio.h */
    FILE *fp;

    /* Generate a temporary file name */
    tmpnam(tmpFileName);

    /* open the temporary file */
#ifdef VMS
    if ((fp = fopen(tmpFileName, "w", "rfm = stmlf")) == NULL) {
#else
    if ((fp = fopen(tmpFileName, "w")) == NULL) {
#endif /* VMS */
    	DialogF(DF_WARN, parent, 1, "Unable to write file for printing:\n%s",
		"Dismiss", errorString());
        return;
    }
    
    /* write to the file */
#ifdef IBM_FWRITE_BUG
    write(fileno(fp), string, length);
#else
    fwrite(string, sizeof(char), length, fp);
#endif
    if (ferror(fp)) {
    	DialogF(DF_ERR, parent, 1, "%s not printed:\n%s", "Dismiss", 
		jobName, errorString());
	fclose(fp);
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
    	int *addWrap)
{
    int n, retVal;
    Arg args[20];
    XmString s1, s2;
    Widget fileSB, wrapToggle;
    char *savedDefaultDir;
    
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
    fileSB = XmCreateFileSelectionDialog(window->shell,"FileSelect",args,n);
    XmStringFree(s1);
    XmStringFree(s2);
    if (window->wrapMode == CONTINUOUS_WRAP) {
	wrapToggle = XtVaCreateManagedWidget("addWrap",
	    	xmToggleButtonWidgetClass, fileSB, XmNlabelString,
	    	s1=XmStringCreateSimple("Add line breaks where wrapped"),
    		XmNmarginHeight, 0, XmNalignment, XmALIGNMENT_BEGINNING, 0);
	XtAddCallback(wrapToggle, XmNvalueChangedCallback, addWrapCB,
    	    	addWrap);
	XmStringFree(s1);
    }
    *addWrap = False;
    XtVaSetValues(XmFileSelectionBoxGetChild(fileSB,
    	    XmDIALOG_FILTER_LABEL), XmNmnemonic, 'l', XmNuserData,
    	    XmFileSelectionBoxGetChild(fileSB, XmDIALOG_FILTER_TEXT), 0);
    XtVaSetValues(XmFileSelectionBoxGetChild(fileSB,
    	    XmDIALOG_DIR_LIST_LABEL), XmNmnemonic, 'D', XmNuserData,
    	    XmFileSelectionBoxGetChild(fileSB, XmDIALOG_DIR_LIST), 0);
    XtVaSetValues(XmFileSelectionBoxGetChild(fileSB,
    	    XmDIALOG_LIST_LABEL), XmNmnemonic, 'F', XmNuserData,
    	    XmFileSelectionBoxGetChild(fileSB, XmDIALOG_LIST), 0);
    XtVaSetValues(XmFileSelectionBoxGetChild(fileSB,
    	    XmDIALOG_SELECTION_LABEL), XmNmnemonic,
    	    prompt[strspn(prompt, "lFD")], XmNuserData,
    	    XmFileSelectionBoxGetChild(fileSB, XmDIALOG_TEXT), 0);
    AddDialogMnemonicHandler(fileSB);
    RemapDeleteKey(XmFileSelectionBoxGetChild(fileSB, XmDIALOG_FILTER_TEXT));
    RemapDeleteKey(XmFileSelectionBoxGetChild(fileSB, XmDIALOG_TEXT));
    retVal = HandleCustomNewFileSB(fileSB, fullname);

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
** Check the read-only or locked status of the window and beep and return
** false if the window should not be written in.
*/
int CheckReadOnly(WindowInfo *window)
{
    if (window->readOnly || window->lockWrite) {
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
