/* $Id: tags.h,v 1.8 2001/10/22 08:50:19 amai Exp $ */
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
int LookupTag(const char *name, const char **file, const char **searchString, const char **tagPath);
void FindDefinition(WindowInfo *window, Time time, char *arg);
