/* $Id: tags.h,v 1.10 2001/12/13 13:27:00 amai Exp $ */
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
