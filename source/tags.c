/*******************************************************************************
*									       *
* tags.c -- Nirvana editor tag file handling        	    	    	       *
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
* July, 1993								       *
*									       *
* Written by Mark Edel							       *
*									       *
*******************************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <sys/types.h>
#include <sys/stat.h>
#ifdef VMS
#include "../util/VMSparam.h"
#include <unistd.h>
#else
#include <sys/param.h>
#endif /*VMS*/
#include <Xm/Xm.h>
#include <Xm/SelectioB.h>
#include <X11/Xatom.h>
#include "../util/DialogF.h"
#include "../util/fileUtils.h"
#include "../util/misc.h"
#include "textBuf.h"
#include "text.h"
#include "nedit.h"
#include "window.h"
#include "file.h"
#include "preferences.h"
#include "search.h"
#include "selection.h"
#include "tags.h"

#define MAXLINE 2048
#define MAX_TAG_LEN 256
#define MAXDUPTAGS 100

#define STRSAVE(a)  ((a)?strcpy(malloc(strlen(a)+1),(a)):strcpy(malloc(1),""))
 
enum searchDirection {FORWARD, BACKWARD};

static int loadTagsFile(char *tagSpec, int index);
static void findDefCB(Widget widget, WindowInfo *window, Atom *sel,
	Atom *type, char *value, int *length, int *format);
static void setTag(tag *t, char *name, char *file, char *searchString);
static int fakeRegExSearch(WindowInfo *window, char *searchString, 
    int *start, int *end);
static unsigned hashAddr(char *key);
static int addTag(char *name,char *file,char *search,char *path,int index);
static int delTag(char *name,char *file,char *search,int index);
static tag *getTag(char *name);
static void findAllDialogAP(Widget dialogParent, char *string);
static void findAllCB(Widget parent, XtPointer client_data, XtPointer call_data);
static Widget createSelectMenu(Widget parent, char *name, char *label,int nArgs, char *args[]);
static char *normalizePathname(char *);

/* Parsed list of tags read by LoadTagsFile.  List is terminated by a tag
   structure with the name field == NULL */
static tag **Tags = NULL;
static int DefTagHashSize = 10000;

static char *tagMark;
static int nTags = 0;
static char *tagName;
static WindowInfo *currentWindow;
static char tagFiles[MAXDUPTAGS][MAXPATHLEN];
static char tagSearch[MAXDUPTAGS][MAXPATHLEN];

tagFile *TagsFileList = NULL;       /* list of loaded tags files */

/*	Compute hash address from a string key */
static unsigned hashAddr(char *key)
{
    unsigned s=strlen(key);
    unsigned a=0,x=0,i;
    
    for (i=0; (i+3)<s; i += 4) {
	strncpy((char*)&a,&key[i],4);
	x += a;
    }
    
    for (a=1; i<(s+1); i++, a *= 256)
	x += key[i] * a;
	
    return x;
}

/*	Retrieve a tag structure from the hash table */
static tag *getTag(char *name)
{
    static char lastName[MAXLINE];
    static tag *t;
    static int addr;
    
    if (Tags == NULL) return NULL;
    
    if (name) {
	addr = hashAddr(name) % DefTagHashSize;
	t = Tags[addr];
	strcpy(lastName,name);
    }
    else if (t) {
	name = lastName;
	t = t->next;
    }
    else return NULL;
    
    for (;t; t = t->next) 
	if (!strcmp(name,t->name)) return t;
    return NULL;
}

/*	Add a tag specification to the hash table */
static int addTag(char *name,char *file,char *search,char *path,int index)
{
    int addr = hashAddr(name) % DefTagHashSize;
    tag *t;
    
    char newfile[MAXPATHLEN];
    if (*file == '/')
        strcpy(newfile,file);
    else
        sprintf(newfile,"%s%s",path,file);
    normalizePathname(newfile);
   
    if (Tags == NULL) Tags = (tag **)calloc(DefTagHashSize, sizeof(tag*));
	
    for (t = Tags[addr]; t; t = t->next) {
	if (strcmp(name,t->name)) continue;
	if (strcmp(search,t->searchString)) continue;
	if (*t->file == '/' && strcmp(newfile,t->file)) continue;
	if (*t->file != '/') {
	    char tmpfile[MAXPATHLEN];
	    sprintf(tmpfile,"%s%s",t->path,t->file);
	    normalizePathname(tmpfile);
	    if (strcmp(newfile,tmpfile)) continue;
	}
	return FALSE;
    }
	
    t = (tag *) malloc(sizeof(tag));
    setTag(t,name,file,search);
    t->index = index;
    t->path = path;
    t->next = Tags[addr];
    Tags[addr] = t;
    return TRUE;
}

/*  Delete a tag from the cache.  
 *  Search is limited to valid matches of 'name','file', 'search', and 'index'.
 *  EX: delete all tags matching index 2 ==> delTag(tagname,NULL,NULL,2);
 */
static int delTag(char *name,char *file,char *search,int index)
{
    tag *t, *last;
    int start,finish,i,del=0;
    
    if (Tags == NULL) return FALSE;
    if (name)
	start = finish = hashAddr(name) % DefTagHashSize;
    else {
	start = 0;
	finish = DefTagHashSize;
    }
    for (i = start; i<finish; i++) {
	for (last = NULL, t = Tags[i]; t; last = t, t = t?t->next:Tags[i]) {
	    if (name && strcmp(name,t->name)) continue;
	    if (index && index != t->index) continue;
	    if (file && strcmp(file,t->file)) continue;
	    if (search && strcmp(search,t->searchString)) continue;
	    if (last)
		last->next = t->next;
	    else
		Tags[i] = t->next;
	    free(t->name);
	    free(t->file);
	    free(t->searchString);
	    free(t);
	    t = NULL;
	    del++;
	}
    }
    return del>0;
}

/*  AddTagsFile():  Add a file spec to the list of tag files to manage
 */
int AddTagsFile(char *tagSpec)
{
    static int fileIndex = 0;
    tagFile *t;
    int added=0;
    struct stat statbuf;
    char *filename;
    char tBuf[MAXPATHLEN];
    char pathName[MAXPATHLEN];

    tagSpec = strcpy(tBuf,tagSpec);
    for (filename = strtok(tagSpec,":"); filename; filename = strtok(NULL,":")) {
	if (*filename != '/') {
	    if (!getcwd(pathName,MAXPATHLEN)) {
		fprintf(stderr, "NEdit: failed to get working directory\n");
		return FALSE;
	    }
	    strcat(pathName,"/");
	    strcat(pathName,filename);
	} else
	    strcpy(pathName,filename);
	normalizePathname(pathName);
	for (t = TagsFileList; t && strcmp(t->filename,pathName); t = t->next);
	if (t) {
	    added=1;
	    continue;
	}
	if (stat(pathName,&statbuf) != 0)
	    continue;
	t = (tagFile *) malloc(sizeof(tag));
	t->filename = STRSAVE(pathName);
	t->loaded = 0;
	t->date = statbuf.st_mtime;
	t->index = ++fileIndex;
	t->next = TagsFileList;
	TagsFileList = t;
	added=1;
    }
    if (added)
	return TRUE;
    return FALSE;
}

/*	Un-manage a tags file */
int DeleteTagsFile(char *filename)
{           
    tagFile *t, *last;

    for (last=NULL,t = TagsFileList; t; last = t,t = t->next) {
	if (strcmp(t->filename,filename))
	    continue;
	if (t->loaded)
	    delTag(NULL,NULL,NULL,t->index);
	if (last) last->next = t->next;
	else TagsFileList = t->next;
	free(t->filename);
	free(t);
	return TRUE;
    }
    return FALSE;
}

/*	Load the tags file into the hash table */
static int loadTagsFile(char *tagSpec, int index)
{
    FILE *fp = NULL;
    char line[MAXLINE], name[MAXLINE], file[MAXPATHLEN], searchString[MAXLINE];
    char unused[MAXPATHLEN], tmpPath[MAXPATHLEN];
    char *filename;
    char *tagPath=NULL;
    int nRead;
    WindowInfo *w;
    
/*
 *  This modification allows multiple tags files to be loaded.  Multiple tags files can be
 *  specified by separating them with colons.  The .Xdefaults would look like this:
 *  Nedit.tags: <tagfile>:<tagfile>
 */
    
    for (filename = strtok(tagSpec,":"); filename; filename = strtok(NULL,":")) {

	/* Open the file */
	if ((fp = fopen(filename, "r")) == NULL) continue;

	/* BUG?  tagPath is shared between tags.  I don't think it
	   gets deallocated when the file is unloaded */
	ParseFilename(filename, unused, tmpPath);
	tagPath = (char *)malloc(strlen(tmpPath)+1);
	strcpy(tagPath, tmpPath);
	
	/* Read the file and store its contents */
	while (fgets(line, MAXLINE, fp)) {
	    nRead = sscanf(line, "%s\t%s\t%[^\n;]", name, file, searchString);
	    if (nRead != 3) 
		continue;
	    if ( *name == '!' )
		continue;
	    addTag(name,file,searchString,tagPath,index);
	}
	fclose(fp);
    }
    if (tagPath == NULL) return FALSE;
    
    /* Undim the "Find Definition" and "Clear All Tags Data" menu item in the existing windows */
    for (w=WindowList; w!=NULL; w=w->next) {
	XtSetSensitive(w->findDefItem, TRUE);
    }
    return TRUE;
}

/*
** Given a name, lookup a file, search string.  Returned strings are pointers
** to internal storage which are valid until the next loadTagsFile call.
*/
int LookupTag(char *name, char **file, char **searchString,char **path)
{
    tag *t;
    tagFile *tf;
    struct stat statbuf;
    for (tf = TagsFileList;tf; tf = tf->next) {
	if (!tf->loaded)
	    continue;
	if (stat(tf->filename,&statbuf) != 0)
	    fprintf(stderr, "NEdit: Error getting status for tag file %s\n",
		    tf->filename);
	if (tf->date == statbuf.st_mtime)
	    continue;
	delTag(NULL,NULL,NULL,tf->index);
	tf->loaded = 0;
    }
    for (tf = TagsFileList; (t = getTag(name))==NULL && tf; tf = tf->next) {
	if (tf->loaded)
	    continue;
	loadTagsFile(tf->filename, tf->index);
	tf->loaded = 1;
	if (stat(tf->filename,&statbuf) != 0)
	    fprintf(stderr, "NEdit: Error getting status for tag file %s\n",
		    tf->filename);
	tf->date = statbuf.st_mtime;
    }
    if (!t)     
	return FALSE;
    *file = t->file;
    *searchString = t->searchString;
    *path = t->path;
    return TRUE;
}

/*
** Lookup the definition for the current primary selection the currently
** loaded tags file and bring up the file and line that the tags file
** indicates.
*/
void FindDefinition(WindowInfo *window, Time time,char *arg)
{
    tagMark = arg;
    currentWindow = window;
    XtGetSelectionValue(window->textArea, XA_PRIMARY, XA_STRING,
	    (XtSelectionCallbackProc)findDefCB, window, time);
}

/*	Callback function for FindDefinition */
static void findDefCB(Widget widget, WindowInfo *window, Atom *sel,
	Atom *type, char *value, int *length, int *format)
{
    static char tagText[MAX_TAG_LEN+1],*p;
    int l,ml;
    
    if (tagMark == NULL) tagMark = value;
    /* skip if we can't get the selection data, or it's obviously too long */
    if (*type == XT_CONVERT_FAIL || tagMark == NULL) {
	XBell(TheDisplay, 0);
	XtFree(value);
	return;
    }
    l = strlen(tagMark);
    if (l > MAX_TAG_LEN) {
	fprintf(stderr, "NEdit: Tag Length too long.\n");
	XBell(TheDisplay, 0);
	XtFree(value);
	return;
    }
    /* should be of type text??? */
    for (p = tagMark; *p && isascii(*p); p++);
    if (*p) {
	fprintf(stderr, "NEdit: Can't handle non 8-bit text\n");
	XBell(TheDisplay, 0);
	XtFree(value);
	return;
    }
    ml = (l<MAX_TAG_LEN?l:MAX_TAG_LEN);
    strncpy(tagText, tagMark, ml);
    tagText[ml] = '\0';
    findAllDialogAP(window->textArea, tagText);
}

/*	store all of the info into a pre-allocated tags struct */
static void setTag(tag *t, char *name, char *file, char *searchString)
{
    t->name = (char *)malloc(sizeof(char) * strlen(name) + 1);
    strcpy(t->name, name);
    t->file = (char *)malloc(sizeof(char) * strlen(file) + 1);
    strcpy(t->file, file);
    t->searchString = (char *)malloc(sizeof(char) * strlen(searchString) + 1);
    strcpy(t->searchString, searchString);
}

/*
** regex searching is not available on all platforms.  To use built in
** case sensitive searching, this routine fakes enough to handle the
** search characters presented in ctags files
*/
static int fakeRegExSearch(WindowInfo *window, char *searchString, 
    int *start, int *end)
{
    int startPos, endPos, found=FALSE, hasBOL, hasEOL, fileLen, searchLen, dir;
    char *fileString, searchSubs[MAXLINE];
    
    /* get the entire (sigh) text buffer from the text area widget */
    fileString = BufGetAll(window->buffer);
    fileLen = window->buffer->length;

    /* remove / .. / or ? .. ? and substitute ^ and $ with \n */
    searchLen = strlen(searchString);
    if (searchString[0] == '/')
	dir = FORWARD;
    else if (searchString[0] == '?')
	dir = BACKWARD;
    else {
	fprintf(stderr, "NEdit: Error parsing tag file search string");
	return FALSE;
    }
    searchLen -= 2;
    strncpy(searchSubs, &searchString[1], searchLen);
    searchSubs[searchLen] = '\0';
    hasBOL = searchSubs[0] == '^';
    hasEOL = searchSubs[searchLen-1] == '$';
    if (hasBOL) searchSubs[0] = '\n';
    if (hasEOL) searchSubs[searchLen-1] = '\n';

    /* search for newline-substituted string in the file */
    if (dir==FORWARD)
	found = SearchString(fileString, searchSubs, SEARCH_FORWARD,
	    SEARCH_CASE_SENSE, False, 0, &startPos, &endPos, NULL, NULL);
    else
	found = SearchString(fileString, searchSubs, SEARCH_BACKWARD,
	    SEARCH_CASE_SENSE, False, fileLen, &startPos, &endPos, NULL, NULL);
    if (found) {
	if (hasBOL) startPos++;
	if (hasEOL) endPos--;
    }
    
    /* if not found: ^ may match beginning of file, $ may match end */
    if (!found && hasBOL) {
	found = strncmp(&searchSubs[1], fileString, searchLen-1);
	if (found) {
	    startPos = 0;
	    endPos = searchLen - 2;
	}
    }
    if (!found && hasEOL) {     
	found = strncmp(searchSubs, fileString+fileLen-searchLen+1,
	     searchLen-1);
	if (found) {
	    startPos = fileLen-searchLen+2;
	    endPos = fileLen;
	}
    }

    /* free the text buffer copy returned from XmTextGetString */
    XtFree(fileString);
    
    /* return the result */
    if (!found) {
	XBell(TheDisplay, 0);
    return FALSE;
    }
    *start = startPos;
    *end = endPos;
    return TRUE;
}

/*	Handles tag "collisions". Prompts user with a list of collided
	tags in the hash table and allows the user to select the correct
	one. */
static void findAllDialogAP(Widget dialogParent, char *string)
{
    char filename[MAXPATHLEN], pathname[MAXPATHLEN], temp[MAXPATHLEN];
    char *fileToSearch, *searchString, *tagPath,**dupTagsList,*eptr;
    int startPos, endPos, lineNum, rows, i,nTag=0,samePath=0;
    WindowInfo *windowToSearch;

    /* verify that the string is reasonable as a tag */
    if (*string == '\0' || strlen(string) > MAX_TAG_LEN) {
	XBell(TheDisplay, 0);
	return;
    }
    nTags=0;
    tagName=string;
    while (LookupTag(string, &fileToSearch, &searchString,&tagPath)) {
	if (*fileToSearch == '/') 
	    strcpy(tagFiles[nTags], fileToSearch);
	else 
	    sprintf(tagFiles[nTags],"%s%s",tagPath,fileToSearch);
	strcpy(tagSearch[nTags],searchString);
	ParseFilename(tagFiles[nTags], filename, pathname);
	if (GetPrefSmartTags() && !strcmp(currentWindow->filename,filename)
				      && !strcmp(currentWindow->path,pathname)) {
	    if (nTags) {
		strcpy(tagFiles[0],tagFiles[nTags]);
		strcpy(tagSearch[0],tagSearch[nTags]);
	    }
	    nTags = 1;
	    break;
	}
	if (!strcmp(currentWindow->path,pathname)) {
	    samePath++;
	    nTag=nTags;
	}
	if (++nTags >= MAXDUPTAGS) {
	    DialogF(DF_WARN, dialogParent,1,"Too many duplicate tags, first %d shown","OK",MAXDUPTAGS);
	    break;
	}
	string = NULL;
    }
    if (GetPrefSmartTags() && samePath == 1 && nTags > 1) {
	strcpy(tagFiles[0],tagFiles[nTag]);
	strcpy(tagSearch[0],tagSearch[nTag]);
	nTags = 1;
    }
    if (!nTags) {
	DialogF(DF_WARN, dialogParent, 1, "%s not found in tags file", "OK",
	    tagName);
	return;
    }
    /*  If all of the tag entries are the same file, just use the first.
     */
    if (GetPrefSmartTags()) {
	for (i=1; i<nTags; i++) 
	    if (strcmp(tagFiles[i],tagFiles[i-1]))
		break;
	if (i==nTags) 
	    nTags = 1;
    }
    
    if (nTags>1) {
	if (!(dupTagsList = (char **) malloc(sizeof(char *) * nTags))) {
	    printf("findDef(): out of heap space!\n");
	    XBell(TheDisplay, 0);
	    return;
	}
	for (i=0; i<nTags; i++) {
	    ParseFilename(tagFiles[i], filename, pathname);
	    if ((i<nTags-1 && !strcmp(tagFiles[i],tagFiles[i+1])) ||
		    (i>0 && !strcmp(tagFiles[i],tagFiles[i-1])))
		sprintf(temp,"%2d. %s%s %s",i+1,pathname,filename,tagSearch[i]);
	    else 
		sprintf(temp,"%2d. %s%s",i+1,pathname,filename);
	    if (!(dupTagsList[i] = (char *) malloc(strlen(temp) + 1))) {
		printf("findDef(): out of heap space!\n");
		XBell(TheDisplay, 0);
		return;
	    }
	    strcpy(dupTagsList[i],temp);
	}
	createSelectMenu(dialogParent,"tagList","Duplicate Tags",nTags,
		dupTagsList);
	for (i=0; i<nTags; i++)
	    free(dupTagsList[i]);
	free(dupTagsList);
	return;
    }
    /*
    **  No need for a dialog list, there is only one tag matching --
    **  Go directly to the tag
    */
    ParseFilename(tagFiles[0],filename,pathname);
    /* open the file containing the definition */
    EditExistingFile(WindowList, filename, pathname, 0, NULL, False, NULL);
    windowToSearch = FindWindowWithFile(filename, pathname);
    if (windowToSearch == NULL) {
	DialogF(DF_WARN, dialogParent, 1, "File %s not found", 
	    "OK", tagFiles[0]);
	return;
    }

    /* if the search string is a number, select the numbered line */
    lineNum = strtol(tagSearch[0], &eptr, 10);
    if (eptr != tagSearch[0]) {
	SelectNumberedLine(windowToSearch, lineNum);
	return;
    }

    /* search for the tags file search string in the newly opened file */
    if (!fakeRegExSearch(windowToSearch, tagSearch[0] ,&startPos, &endPos)) {
	DialogF(DF_WARN, windowToSearch->shell, 1,"Definition for %s\nnot found in %s", 
	    "OK", tagName, tagFiles[0]);
	return;
    }

    /* select the matched string */
    BufSelect(windowToSearch->buffer, startPos, endPos);

    /* Position it nicely in the window, about 1/4 of the way down from the
       top */
    lineNum = BufCountLines(windowToSearch->buffer, 0, startPos);
    XtVaGetValues(windowToSearch->lastFocus, textNrows, &rows, 0);
    TextSetScroll(windowToSearch->lastFocus, lineNum - rows/4, 0);
    TextSetCursorPos(windowToSearch->lastFocus, endPos);
    
}

/*	Callback function for the FindAll widget. Process the users response. */
static void findAllCB(Widget parent, XtPointer client_data, XtPointer call_data)
{
    int startPos, endPos, lineNum, rows, i;
    char filename[MAXPATHLEN], pathname[MAXPATHLEN];
    char *eptr;
    WindowInfo *windowToSearch;
    
    XmSelectionBoxCallbackStruct *cbs = (XmSelectionBoxCallbackStruct *) call_data;
    if (cbs->reason == XmCR_NO_MATCH)
	return;
    if (cbs->reason == XmCR_CANCEL) {
	XtDestroyWidget(XtParent(parent));
	return;
    }
    
    XmStringGetLtoR(cbs->value,XmFONTLIST_DEFAULT_TAG,&eptr);
    if ((i = atoi(eptr)-1) < 0) {
	XBell(TheDisplay, 0);
	return;
    }
    ParseFilename(tagFiles[i],filename,pathname);
    /* open the file containing the definition */
    EditExistingFile(WindowList, filename, pathname, 0, NULL, False, NULL);
    windowToSearch = FindWindowWithFile(filename, pathname);
    if (windowToSearch == NULL) {
	DialogF(DF_WARN, parent, 1, "File %s not found", 
	    "OK", tagFiles[i]);
	return;
    }

    /* if the search string is a number, select the numbered line */
    lineNum = strtol(tagSearch[i], &eptr, 10);
    if (eptr != tagSearch[i]) {
	SelectNumberedLine(windowToSearch, lineNum);
	if (cbs->reason == XmCR_OK)
	    XtDestroyWidget(XtParent(parent));
	return;
    }

    /* search for the tags file search string in the newly opened file */
    if (!fakeRegExSearch(windowToSearch, tagSearch[i] ,&startPos, &endPos)) {
	DialogF(DF_WARN, windowToSearch->shell, 1,
		"Definition for %s\nnot found in %s", "OK", tagName,
		tagFiles[i]);
	return;
    }

    /* select the matched string */
    BufSelect(windowToSearch->buffer, startPos, endPos);

    /* Position it nicely in the window, about 1/4 of the way down from the
       top */
    lineNum = BufCountLines(windowToSearch->buffer, 0, startPos);
    XtVaGetValues(windowToSearch->lastFocus, textNrows, &rows, 0);
    TextSetScroll(windowToSearch->lastFocus, lineNum - rows/4, 0);
    TextSetCursorPos(windowToSearch->lastFocus, endPos);
    if (cbs->reason == XmCR_OK)
	XtDestroyWidget(XtParent(parent));
}

/*	Window manager close-box callback for tag-collision dialog */
static void findAllCloseCB(Widget parent, XtPointer client_data,
	XtPointer call_data)
{
    XtDestroyWidget(parent);
}

/*	Create a Menu for user to select from the collided tags */
static Widget createSelectMenu(Widget parent, char *name, char *label,
	int nArgs, char *args[])
{
    int i;
    char tmpStr[100];
    Widget menu;
    XmStringTable list;
    XmString popupTitle;
    int ac;
    Arg csdargs[20];
    
    list = (XmStringTable) XtMalloc(nArgs * sizeof(XmString *));
    for (i=0; i<nArgs; i++)
	list[i] = XmStringCreateSimple(args[i]);
    sprintf(tmpStr,"Select File With TAG: %s",tagName);
    popupTitle = XmStringCreateSimple(tmpStr);
    ac = 0;
    XtSetArg(csdargs[ac], XmNlistLabelString, popupTitle); ac++;
    XtSetArg(csdargs[ac], XmNlistItems, list); ac++;
    XtSetArg(csdargs[ac], XmNlistItemCount, nArgs); ac++;
    XtSetArg(csdargs[ac], XmNvisibleItemCount, 12); ac++;
    XtSetArg(csdargs[ac], XmNautoUnmanage, False); ac++;
    menu = XmCreateSelectionDialog(parent,label,csdargs,ac);
    XtUnmanageChild(XmSelectionBoxGetChild(menu, XmDIALOG_TEXT));
    XtUnmanageChild(XmSelectionBoxGetChild(menu, XmDIALOG_HELP_BUTTON));
    XtUnmanageChild(XmSelectionBoxGetChild(menu, XmDIALOG_SELECTION_LABEL));
    XtAddCallback(menu, XmNokCallback, (XtCallbackProc)findAllCB, menu);
    XtAddCallback(menu, XmNapplyCallback, (XtCallbackProc)findAllCB, menu);
    XtAddCallback(menu, XmNcancelCallback, (XtCallbackProc)findAllCB, menu);
    AddMotifCloseCallback(XtParent(menu), findAllCloseCB, NULL);
    for (i=0; i<nArgs; i++)
	XmStringFree(list[i]);
    XtFree((char *)list);
    XmStringFree(popupTitle);
    ManageDialogCenteredOnPointer(menu);
    return menu;
}

/* remove './' '../' & '//' from pathnames */
static char *normalizePathname(char *str)
{
    char *r,*p=str;
    char result[MAXPATHLEN+1];
    r = result;

    while (*p) {
	if (!strncmp(p,"../",3))
	    p += 3;
	else if (!strncmp(p,"./",2))
	    p += 2;
	else if (!strncmp(p,"//",2))
	    p += 2;
	else
	    *r = *p;
	r++; p++;
    }
    *r = '\0';
    return strcpy(str,result);
}
