/* $Id: tags.h,v 1.11 2002/07/11 21:18:10 slobasso Exp $ */

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

int AddRelTagsFile(const char *tagSpec, const char *windowPath);
int AddTagsFile(const char *tagSpec);
int DeleteTagsFile(const char *filename);
int LookupTag(const char *name, const char **file, 
              const char **searchString, int * pos, const char **path);
void FindDefinition(WindowInfo *window, Time time, const char *arg);

#endif /* NEDIT_TAGS_H_INCLUDED */
