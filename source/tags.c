#include <stdio.h>
#include <stdlib.h>
#ifdef VMS
#include "../util/VMSparam.h"
#else
#include <sys/param.h>
#endif /*VMS*/
#include <Xm/Xm.h>
#include <X11/Xatom.h>
#include "../util/DialogF.h"
#include "../util/fileUtils.h"
#include "textBuf.h"
#include "text.h"
#include "nedit.h"
#include "window.h"
#include "file.h"
#include "search.h"
#include "selection.h"
#include "tags.h"

#define MAXLINE 512
#define MAX_TAG_LEN 80

enum searchDirection {FORWARD, BACKWARD};

typedef struct {
    char *name;
    char *file;
    char *searchString;
} tag;

static void findDefCB(Widget widget, WindowInfo *window, Atom *sel,
    	Atom *type, char *value, int *length, int *format);
static void findDef(Widget dialogParent, char *string);
static void setTag(tag *t, char *name, char *file, char *searchString);
static void freeTagList(tag *tags);
static int fakeRegExSearch(WindowInfo *window, char *searchString, 
	int *start, int *end);

/* Parsed list of tags read by LoadTagsFile.  List is terminated by a tag
   structure with the name field == NULL */
static tag *Tags = NULL;
static char TagPath[MAXPATHLEN];

int LoadTagsFile(char *filename)
{
    FILE *fp = NULL;
    char line[MAXLINE], name[MAXLINE], file[MAXLINE], searchString[MAXLINE];
    char unused[MAXPATHLEN];
    char *charErr;
    tag *tags;
    int i, nTags, nRead;
    WindowInfo *w;

    /* Open the file */
    if ((fp = fopen(filename, "r")) == NULL)
    	return FALSE;
	
    /* Read it once to see how many lines there are */
    for (nTags=0; TRUE; nTags++) {
    	charErr = fgets(line, MAXLINE, fp);
    	if (charErr == NULL) {
    	    if (feof(fp))
    	    	break;
    	    else
    	    	return FALSE;
    	}
    }
    
    /* Allocate zeroed memory for list so that it is automatically terminated
       and can be freed by freeTagList at any stage in its construction*/
    tags = (tag *)calloc(nTags + 1, sizeof(tag));
    
    /* Read the file and store its contents */
    rewind(fp);
    for (i=0; i<nTags; i++) {
    	charErr = fgets(line, MAXLINE, fp);
    	if (charErr == NULL) {
    	    if (feof(fp))
    	    	break;
    	    else {
    	    	freeTagList(tags);
    	    	return FALSE;
    	    }
    	}
    	nRead = sscanf(line, "%s\t%s\t%[^\n]", name, file, searchString);
    	if (nRead != 3) {
    	    freeTagList(tags);
    	    return FALSE;
    	}
	setTag(&tags[i], name, file, searchString);
    }
    
    /* Make sure everything was read */
    if (i != nTags) {
    	freeTagList(tags);
    	return FALSE;
    }
    
    /* Replace current tags data and path for retrieving files */
    if (Tags != NULL)
    	freeTagList(Tags);
    Tags = tags;
    ParseFilename(filename, unused, TagPath);
    
    /* Undim the "Find Definition" menu item in the existing windows */
    for (w=WindowList; w!=NULL; w=w->next)
    	XtSetSensitive(w->findDefItem, TRUE);
    return TRUE;
}

int TagsFileLoaded(void)
{
    return Tags != NULL;
}

/*
** Given a name, lookup a file, search string.  Returned strings are pointers
** to internal storage which are valid until the next LoadTagsFile call.
*/
int LookupTag(char *name, char **file, char **searchString)
{
    int i;
    tag *t;
    
    if (!TagsFileLoaded())
    	return FALSE;
    	
    for (i=0, t=Tags; t->name!=NULL; i++, t++) {
 	if (!strcmp(t->name, name)) {
 	    *file = t->file;
 	    *searchString = t->searchString;
 	    return TRUE;
	}
    }
    return FALSE;
}

/*
** Lookup the definition for the current primary selection the currently
** loaded tags file and bring up the file and line that the tags file
** indicates.
*/
void FindDefinition(WindowInfo *window, Time time)
{
    XtGetSelectionValue(window->textArea, XA_PRIMARY, XA_STRING,
    	    (XtSelectionCallbackProc)findDefCB, window, time);
}

static void findDefCB(Widget widget, WindowInfo *window, Atom *sel,
    	Atom *type, char *value, int *length, int *format)
{
    char tagText[MAX_TAG_LEN+1];
    
    /* skip if we can't get the selection data, or it's obviously too long */
    if (*type == XT_CONVERT_FAIL || value == NULL) {
    	XBell(TheDisplay, 0);
	return;
    }
    if (*length > MAX_TAG_LEN) {
    	XBell(TheDisplay, 0);
	XtFree(value);
	return;
    }
    /* should be of type text??? */
    if (*format != 8) {
    	fprintf(stderr, "NEdit: Can't handle non 8-bit text\n");
    	XBell(TheDisplay, 0);
	XtFree(value);
	return;
    }
    
    /* Copy the string just to make space for the null character (this may
       not be necessary, XLib documentation claims a NULL is already added,
       but the Xt documentation for this routine makes no such claim) */
    strncpy(tagText, value, *length);
    tagText[*length] = '\0';
    
    /* Lookup the tag and put up the window */
    findDef(window->textArea, tagText);
    
    /* The selection requstor is required to free the memory passed
       to it via value */
    XtFree((char *)value);
}

/*
** Lookup the definition for "string" in the currently loaded tags file
** and bring up the file and line that the tags file indicates
*/
static void findDef(Widget dialogParent, char *string)
{
    int startPos, endPos, found, lineNum, rows;
    char *fileToSearch, *searchString, *eptr;
    WindowInfo *windowToSearch;
    char filename[MAXPATHLEN], pathname[MAXPATHLEN], temp[MAXPATHLEN];

    /* verify that the string is reasonable as a tag */
    if (*string == '\0') {
	XBell(TheDisplay, 0);
	return;
    }
    if (strlen(string) > MAX_TAG_LEN) {
    	XBell(TheDisplay, 0);
	return;
    }
    
    /* lookup the name in the tags file */
    found = LookupTag(string, &fileToSearch, &searchString);
    if (!found) {
    	DialogF(DF_WARN, dialogParent, 1, "%s not found in tags file", "OK",
    		string);
    	return;
    }
    
    /* if the path is not absolute, qualify file path with directory
       from which tags file was loaded */
    if (fileToSearch[0] == '/')
    	strcpy(temp, fileToSearch);
    else {
    	strcpy(temp, TagPath);
    	strcat(temp, fileToSearch);
    }
    ParseFilename(temp, filename, pathname);
    
    /* open the file containing the definition */
    EditExistingFile(WindowList, filename, pathname, FALSE);
    windowToSearch = FindWindowWithFile(filename, pathname);
    if (windowToSearch == NULL) {
    	DialogF(DF_WARN, dialogParent, 1, "File %s not found", 
    		"OK", fileToSearch);
    	return;
    }
    
    /* if the search string is a number, select the numbered line */
    lineNum = strtol(searchString, &eptr, 10);
    if (eptr != searchString) {
    	SelectNumberedLine(windowToSearch, lineNum);
    	return;
    }
    
    /* search for the tags file search string in the newly opened file */
    found = fakeRegExSearch(windowToSearch, searchString ,&startPos, &endPos);
    if (!found) {
    	DialogF(DF_WARN, windowToSearch->shell, 1,"Definition for %s\nnot found in %s", 
    		"OK", string, fileToSearch);
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

static void setTag(tag *t, char *name, char *file, char *searchString)
{
    t->name = (char *)malloc(sizeof(char) * strlen(name) + 1);
    strcpy(t->name, name);
    t->file = (char *)malloc(sizeof(char) * strlen(file) + 1);
    strcpy(t->file, file);
    t->searchString = (char *)malloc(sizeof(char) * strlen(searchString) + 1);
    strcpy(t->searchString, searchString);
}

static void freeTagList(tag *tags)
{
    int i;
    tag *t;
    
    for (i=0, t=tags; t->name!=NULL; i++, t++) {
    	free(t->name);
    	free(t->file);
    	free(t->searchString);
    }
    free(tags);
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
    		SEARCH_CASE_SENSE, False, 0, &startPos, &endPos, NULL);
    else
    	found = SearchString(fileString, searchSubs, SEARCH_BACKWARD,
    		SEARCH_CASE_SENSE, False, fileLen, &startPos, &endPos, NULL);
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
