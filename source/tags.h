/* $Id: tags.h,v 1.3 2001/02/26 23:38:03 edg Exp $ */
typedef struct _tag {
    struct _tag *next;
    char *path;
    char *name;
    char *file;
    char *searchString;
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

int AddTagsFile(char *tagSpec);
int DeleteTagsFile(char *filename);
int LookupTag(char *name, char **file, char **searchString,char **tagPath);
void FindDefinition(WindowInfo *window, Time time, char *arg);
