static const char CVSID[] = "$Id: tags.c,v 1.20 2001/10/04 10:16:15 amai Exp $";
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
* FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License        *
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
#include <unistd.h>
#ifdef VMS
#include "../util/VMSparam.h"
#else
#ifndef __MVS__
#include <sys/param.h>
#endif
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
#include "../util/utils.h"

#define MAXLINE 2048
#define MAX_TAG_LEN 256
#define MAXDUPTAGS 100

#define STRSAVE(a)  ((a!=NULL)?strcpy(malloc(strlen(a)+1),(a)):strcpy(malloc(1),""))
 
 
enum searchDirection {FORWARD, BACKWARD};

static int loadTagsFile(const char *tagSpec, int index);
static void findDefCB(Widget widget, WindowInfo *window, Atom *sel,
	Atom *type, char *value, int *length, int *format);
static void setTag(tag *t, const char *name, const char *file,
                   const char *searchString, const char * tag);
static int fakeRegExSearch(WindowInfo *window, const char *searchString, 
    int *startPos, int *endPos);
static unsigned hashAddr(const char *key);
static int addTag(const char *name, const char *file, const char *search,
                  const  char *path, int index);
static int delTag(const char *name, const char *file, const char *search,
                  int index);
static tag *getTag(const char *name);
static void findAllDialogAP(Widget dialogParent, const char *string);
static void findAllCB(Widget parent, XtPointer client_data, XtPointer call_data);
static Widget createSelectMenu(Widget parent, const char *name,
                               char *label, int nArgs, char *args[]);

/* Parsed list of tags read by LoadTagsFile.  List is terminated by a tag
   structure with the name field == NULL */
static tag **Tags = NULL;
static int DefTagHashSize = 10000;

static char *tagMark;
static int nTags = 0;
static const char *tagName;
static WindowInfo *currentWindow;
static char tagFiles[MAXDUPTAGS][MAXPATHLEN];
static char tagSearch[MAXDUPTAGS][MAXPATHLEN];
static const char *rcs_strdup(const char *str);
static void rcs_free(const char *str);

tagFile *TagsFileList = NULL;       /* list of loaded tags files */


/*	Compute hash address from a string key */
static unsigned hashAddr(const char *key)
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
static tag *getTag(const char *name)
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
static int addTag(const char *name, const char *file, const char *search,
                  const char *path, int index)
{
    int addr = hashAddr(name) % DefTagHashSize;
    tag *t;
    char newfile[MAXPATHLEN];

    if (*file == '/')
        strcpy(newfile,file);
    else
        sprintf(newfile,"%s%s", path, file);
    
    NormalizePathname(newfile);
    CompressPathname(newfile);
   
    if (Tags == NULL) Tags = (tag **)calloc(DefTagHashSize, sizeof(tag*));
	
    for (t = Tags[addr]; t; t = t->next) {
	if (strcmp(name,t->name)) continue;
	if (strcmp(search,t->searchString)) continue;
	if (*t->file == '/' && strcmp(newfile,t->file)) continue;
	if (*t->file != '/') {
	    char tmpfile[MAXPATHLEN];
	    sprintf(tmpfile, "%s%s", t->path, t->file);
	    NormalizePathname(tmpfile);
	    CompressPathname(tmpfile);
	    if (strcmp(newfile, tmpfile)) continue;
	}
	return FALSE;
    }
	
    t = (tag *) malloc(sizeof(tag));
    setTag(t, name, file, search, path);
    t->index = index;
    t->next = Tags[addr];
    Tags[addr] = t;
    return TRUE;
}

/*  Delete a tag from the cache.  
 *  Search is limited to valid matches of 'name','file', 'search', and 'index'.
 *  EX: delete all tags matching index 2 ==> delTag(tagname,NULL,NULL,2);
 */
static int delTag(const char *name, const char *file, const char *search,
                  int index)
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
	    rcs_free(t->name);
	    rcs_free(t->file);
	    rcs_free(t->searchString);
	    rcs_free(t->path);
	    free(t);
	    t = NULL;
	    del++;
	}
    }
    return del>0;
}

/* used  in AddRelTagsFile and AddTagsFile */
static int tagFileIndex = 0; 

/*  
** AddRelTagsFile():  Rescan tagSpec for relative tag file specs 
** (not starting with [/~]) and extend tag files list if in
** windowPath a tags file matching the relative spec has been found.
*/
int AddRelTagsFile(const char *tagSpec, const char *windowPath) 
{
    tagFile *t;
    int added=0;
    struct stat statbuf;
    char *filename;
    char pathName[MAXPATHLEN];
    char *tmptagSpec;

    tmptagSpec = (char *) malloc(strlen(tagSpec)+1);
    strcpy(tmptagSpec, tagSpec);
    for (filename = strtok(tmptagSpec, ":"); filename; filename = strtok(NULL, ":")){
      	if (*filename == '/' || *filename == '~')
	    continue;
	if (windowPath && *windowPath) {
	    strcpy(pathName, windowPath);
	}
	else {
	    strcpy(pathName, GetCurrentDir());
	 }   
	strcat(pathName, "/");
	strcat(pathName, filename);
	NormalizePathname(pathName);
      	CompressPathname(pathName);

	for (t = TagsFileList; t && strcmp(t->filename, pathName); t = t->next);
	if (t) {
	    added=1;
	    continue;
	}
	if (stat(pathName, &statbuf) != 0)
	    continue;
	t = (tagFile *) malloc(sizeof(tag));
	t->filename = STRSAVE(pathName);
	t->loaded = 0;
	t->date = statbuf.st_mtime;
	t->index = ++tagFileIndex;
	t->next = TagsFileList;
	TagsFileList = t;
	added=1;
    }
    free(tmptagSpec);
    if (added)
	return TRUE;
    else
        return FALSE;
} 

/*  AddTagsFile():  Add a file spec to the list of tag files to manage
 */
int AddTagsFile(const char *tagSpec)
{
    tagFile *t;
    int added=0;
    struct stat statbuf;
    char *filename;
    char pathName[MAXPATHLEN];
    char *tmptagSpec;

    tmptagSpec = (char *) malloc(strlen(tagSpec)+1);
    strcpy(tmptagSpec, tagSpec);
    for (filename = strtok(tmptagSpec,":"); filename; filename = strtok(NULL,":")) {
	if (*filename != '/') {
          strcpy(pathName, GetCurrentDir());
	    strcat(pathName,"/");
	    strcat(pathName,filename);
	} else {
	    strcpy(pathName,filename);
	}
	NormalizePathname(pathName);
      	CompressPathname(pathName);

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
	t->index = ++tagFileIndex;
	t->next = TagsFileList;
	TagsFileList = t;
	added=1;
    }
    free(tmptagSpec);
    if (added)
	return TRUE;
    else
       return FALSE;
}

/*	Un-manage a tags file */
int DeleteTagsFile(const char *filename)
{   
    tagFile *t, *last;

    for (last=NULL,t = TagsFileList; t; last = t,t = t->next) {
	if (strcmp(t->filename, filename))
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

/*  
** Load tags file(s) into the hash table. Multiple tags files can be loaded. 
** They can be specified by separating them with colons.  
** The .Xdefaults would look like this:
**   Nedit.tags: <tagfile>:<tagfile>
*/
static int loadTagsFile(const char *tagSpec, int index)
{
    FILE *fp = NULL;
    char line[MAXLINE], name[MAXLINE], searchString[MAXLINE];
    char file[MAXPATHLEN], unused[MAXPATHLEN], tmpPath[MAXPATHLEN];
    char *filename;
    char *tagPath=tmpPath;
    char *tmpTagSpec;
    char *posTagREEnd, *posTagRENull;
    int nRead;
    WindowInfo *w;
    
    
    
    
    /*  tagSpec has to be copied, because otherwise strtok places 
    **  NULL-characters in it!
    */
    tmpTagSpec=malloc(strlen(tagSpec)+1);
    strcpy(tmpTagSpec, tagSpec);
    for (filename = strtok(tmpTagSpec,":"); filename; filename = strtok(NULL,":")) {

	/* Open the file */
	if ((fp = fopen(filename, "r")) == NULL) {
	   continue;
	}

	ParseFilename(filename, unused, tagPath);
	
	/* Read the file and store its contents */
	while (fgets(line, MAXLINE, fp)) {
	    nRead = sscanf(line, "%s\t%s\t%[^\n]", name, file, searchString);
	    if (nRead != 3) 
		continue;
	    if ( *name == '!' )
		continue;
	    
	    /* 
	    ** Guess the end of searchString:
	    ** Try to handle original ctags and exuberant ctags format: 
	    */
	    if(searchString[0] == '/' || searchString[0] == '?') {
	      
		/* Situations: /<ANY expr>/\0     
		**             ?<ANY expr>?\0          --> original ctags 
		**             /<ANY expr>/;"  <flags>
		**	       ?<ANY expr>?;"  <flags> --> exuberant ctags 
		*/
		posTagREEnd = strrchr(searchString, ';');
		posTagRENull = strchr(searchString, 0); 
		if(!posTagREEnd || (posTagREEnd[1] != '"') || 
	      	    (posTagRENull[-1] == searchString[0])) {
		    /*	-> original ctags format = exuberant ctags format 1 */
		    posTagREEnd = posTagRENull;
                } else {
		    /* looks like exuberant ctags format 2 */
		    *posTagREEnd = 0;
		}

		/* 
		** Hide the last delimiter:
		**   /<expression>/    becomes   /<expression>
		**   ?<expression>?    becomes   ?<expression>
		** This will save a little work in fakeRegExSearch.
		*/
		if(posTagREEnd > (searchString+2)) {
	      	    posTagREEnd--;
		    if(searchString[0] == *posTagREEnd)
			*posTagREEnd=0;
		}
	    }
	    addTag(name, file, searchString, tagPath, index);
	}
	fclose(fp);
    }
    free(tmpTagSpec);
    if (tagPath == NULL) {
       /* Nothing read in?! */
       return FALSE;
    }
    /* Undim the "Find Definition" and "Clear All Tags Data" menu item 
    ** in the existing windows 
    */
    for (w=WindowList; w!=NULL; w=w->next) {
	XtSetSensitive(w->findDefItem, TRUE);
    }
    return TRUE;
}

/*
** Given a name, lookup a file, search string.  Returned strings are pointers
** to internal storage which are valid until the next loadTagsFile call.
*/
int LookupTag(const char *name, const char **file, 
              const char **searchString, const char **path)
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
    XtFree(value);
}

/*	store all of the info into a pre-allocated tags struct */
static void setTag(tag *t, const char *name, const char *file,
                   const char *searchString, const char *path)
{
    t->name         = rcs_strdup(name);
    t->file         = rcs_strdup(file);
    t->searchString = rcs_strdup(searchString);
    t->path         = rcs_strdup(path);
}

/*
** ctags search expressions are literal strings with a search direction flag, 
** line starting "^" and ending "$" delimiters. This routine translates them 
** into NEdit compatible regular expressions and does the search.
*/
static int fakeRegExSearch(WindowInfo *window, const char *searchString, 
    int *startPos, int *endPos)
{
    int found, searchStartPos, dir;
    char *fileString, searchSubs[3*MAXLINE+3], *outPtr;
    const char *inPtr;
    
    /* determine search direction and start position */
    if (searchString[0] == '/') {
	dir = SEARCH_FORWARD;
	searchStartPos = 0;
    } else if (searchString[0] == '?') {
	dir = SEARCH_BACKWARD;
	searchStartPos = window->buffer->length;
    } else {
	fprintf(stderr, "NEdit: Error parsing tag file search string");
	return FALSE;
    }

    /* get the entire (sigh) text buffer from the text area widget */
    fileString = BufGetAll(window->buffer);
        
    /* Build the search regex. */
    inPtr=searchString+1; /* searchString[0] is / or ? --> search direction */
    outPtr=searchSubs; 
    if(*inPtr == '^') {
      	/* If the first char is a caret then it's a RE line start delimiter */
      	*outPtr++ = *inPtr++;
    }
    while(*inPtr) {
      	if(strchr("()-[]<>{}.|^*+?&\\", *inPtr) || (*inPtr == '$' && inPtr[1])){
	    /* Escape RE Meta Characters to match them literally.
	       Don't escape $ if it's the last charcter of the search expr.
	     */
	    *outPtr++ = '\\';
	    *outPtr++ = *inPtr++;
      	} else if (isspace(*inPtr)) { /* collect multiple spaces  */
	    *outPtr++ = '\\';
	    *outPtr++ = 's';
	    *outPtr++ = '+';
	    do { inPtr++ ; } while(isspace(*inPtr));
	} else {              /* simply copy all other characters */
	    *outPtr++ = *inPtr++;
	}
    }
    *outPtr=0; /* Terminate searchSubs */
    
    found = SearchString(fileString, searchSubs, dir, SEARCH_REGEX, 
      	    False, searchStartPos, startPos, endPos, NULL, NULL);

    /* free the text buffer copy returned from XmTextGetString */
    XtFree(fileString);
    
    /* return the result */
    if (found) {
        /* *startPos and *endPos are set in SearchString*/
	return TRUE;
    } else {
        /* startPos, endPos left untouched by SearchString if search failed. */
	XBell(TheDisplay, 0);
      	return FALSE;
    }
}

/*	Handles tag "collisions". Prompts user with a list of collided
	tags in the hash table and allows the user to select the correct
	one. */
static void findAllDialogAP(Widget dialogParent, const char *string)
{
    char filename[MAXPATHLEN], pathname[MAXPATHLEN], temp[MAXPATHLEN];
    const char *fileToSearch, *searchString, *tagPath;
    char **dupTagsList,*eptr;
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
			       && !strcmp(currentWindow->path,pathname)   ) {
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
	    DialogF(DF_WARN, dialogParent,1,
	      	    "Too many duplicate tags, first %d shown","OK",MAXDUPTAGS);
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
    XtVaGetValues(windowToSearch->lastFocus, textNrows, &rows, NULL);
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
    XtVaGetValues(windowToSearch->lastFocus, textNrows, &rows, NULL);
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
static Widget createSelectMenu(Widget parent, const char *name,
         char *label, int nArgs, char *args[])
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
    menu = CreateSelectionDialog(parent,label,csdargs,ac);
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

/*--------------------------------------------------------------------------

   Reference-counted string hack; SJT 4/2000

   This stuff isn't specific to tags, so it should be in it's own file.
   However, I'm leaving it in here for now to reduce the diffs.
   
   This could really benefit from using a real hash table.
*/

#define RCS_SIZE 10000

struct rcs;

struct rcs_stats
{
    int talloc, tshar, tgiveup, tbytes, tbyteshared;
};

struct rcs
{
    struct rcs *next;
    char       *string;
    int         usage;
};

static struct rcs       *Rcs[RCS_SIZE];
static struct rcs_stats  RcsStats;

/*
** Take a normal string, create a shared string from it if need be,
** and return pointer to that shared string.
**
** Returned strings are const because they are shared.  Do not modify them!
*/

static const char *rcs_strdup(const char *str)
{
    int bucket;
    size_t len;
    struct rcs *rp;
    struct rcs *prev = NULL;
  
    char *newstr = NULL;
    
    if (str == NULL)
        return NULL;
        
    bucket = hashAddr(str) % RCS_SIZE;
    len = strlen(str);
    
    RcsStats.talloc++;

#if 0  
    /* Don't share if it won't save space.
    
       Doesn't save anything - if we have lots of small-size objects,
       it's beneifical to share them.  We don't know until we make a full
       count.  My tests show that it's better to leave this out.  */
    if (len <= sizeof(struct rcs))
    {
        new_str = strdup(str);
        RcsStats.tgiveup++;
        return;
    }
#endif

    /* Find it in hash */
    for (rp = Rcs[bucket]; rp; rp = rp->next)
    {
        if (!strcmp(str, rp->string))
            break;
        prev = rp;
    }

    if (rp)  /* It exists, return it and bump ref ct */
    {
        rp->usage++;
        newstr = rp->string;

        RcsStats.tshar++;
        RcsStats.tbyteshared += len;
    }
    else     /* Doesn't exist, conjure up a new one. */
    {
        struct rcs *newrcs = malloc(sizeof(struct rcs));
        newrcs->string = malloc(len+1);
        strcpy(newrcs->string, str);
        newrcs->usage = 1;
        newrcs->next = NULL;

        if (Rcs[bucket])
            prev->next = newrcs;
        else
            Rcs[bucket] = newrcs;
            
        newstr = newrcs->string;
    }

    RcsStats.tbytes += len;
    return newstr;
}

/*
** Decrease the reference count on a shared string.  When the reference
** count reaches zero, free the master string.
*/

static void rcs_free(const char *rcs_str)
{
    int bucket;
    struct rcs *rp;
    struct rcs *prev = NULL;

    if (rcs_str == NULL)
        return;
        
    bucket = hashAddr(rcs_str) % RCS_SIZE;

    /* find it in hash */
    for (rp = Rcs[bucket]; rp; rp = rp->next)
    {
        if (rcs_str == rp->string)
            break;
        prev = rp;
    }

    if (rp)  /* It's a shared string, decrease ref count */
    {
        rp->usage--;
        
        if (rp->usage < 0) /* D'OH! */
        {
            fprintf(stderr, "NEdit: internal error deallocating shared string.");
            return;
        }

        if (rp->usage == 0)  /* Last one- free the storage */
        {
            free(rp->string);
            if (prev)
                prev->next = rp->next;
            else
                Rcs[bucket] = rp->next;
            free(rp);
        }
    }
    else    /* Doesn't appear to be a shared string */
    {
        fprintf(stderr, "NEdit: attempt to free a non-shared string.");
        return;
    }
}
