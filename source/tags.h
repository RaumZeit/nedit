/* $Id: tags.h,v 1.6 2001/09/06 09:37:54 amai Exp $ */
typedef struct _tag {
    struct _tag *next;
    const char *path;
    const char *name;
    const char *file;
    const char *searchString;
    short index;
} tag;

typedef struct _tagFile {
    struct _tagFile *next;
    char *filename;
    time_t  date;
    Boolean loaded;
    short index;
} tagFile;

extern tagFile *TagsFileList;         /* list of loaded tags files */

int AddRelTagsFile(char *tagSpec, const char *windowPath);
int AddTagsFile(char *tagSpec);
int DeleteTagsFile(char *filename);
int LookupTag(const char *name, const char **file, const char **searchString, const char **tagPath);
void FindDefinition(WindowInfo *window, Time time, char *arg);
