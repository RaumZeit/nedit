/* $Id: tags.h,v 1.12 2002/07/26 21:39:10 n8gray Exp $ */

#ifndef NEDIT_TAGS_H_INCLUDED
#define NEDIT_TAGS_H_INCLUDED

#include "nedit.h"

#include <X11/Intrinsic.h>
#include <X11/X.h>
#include <time.h>

typedef struct _tagFile {
    struct _tagFile *next;
    char *filename;
    time_t  date;
    Boolean loaded;
    short index;
} tagFile;

extern tagFile *TagsFileList;         /* list of loaded tags files */
extern tagFile *TipsFileList;         /* list of loaded calltips tag files */

/* file_type and search_type arguments are to select between tips and tags,
    and should be one of TAG or TIP.  TIP_FROM_TAG is for ShowTipString. */
enum mode {TAG, TIP_FROM_TAG, TIP};
int AddRelTagsFile(const char *tagSpec, const char *windowPath, 
                   int file_type);
/* tagSpec is a colon-delimited list of filenames */
int AddTagsFile(const char *tagSpec, int file_type);
int DeleteTagsFile(const char *tagSpec, int file_type);
int LookupTag(const char *name, const char **file, int *lang,
              const char **searchString, int * pos, const char **path,
              int search_type);

/* Routines for handling tags or tips from the current selection */
void FindDefinition(WindowInfo *window, Time time, const char *arg);
void FindDefCalltip(WindowInfo *window, Time time, const char *arg);

/* Go to a tag given as a string */
int ShowDefString(WindowInfo *window, char *text);
/* Display (possibly finding first) a calltip.  Search type can only be 
    TIP or TIP_FROM_TAG here. */
int ShowTipString(WindowInfo *window, char *text, Boolean anchored,
                         int pos, Boolean lookup, int tip_search_type);
void KillCalltip(WindowInfo *window, int calltipID);
int GetCalltipID(WindowInfo *window, int calltipID);

#endif /* NEDIT_TAGS_H_INCLUDED */
