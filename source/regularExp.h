/*
 * Definitions etc. for regexp(3) routines.
 *
 * Caveat:  this is V8 regexp(3) [actually, a reimplementation thereof],
 * not the System V one.
 */
#define NSUBEXP  40
typedef struct regexp {
    char *startp[NSUBEXP];
    char *endp[NSUBEXP];
    char regstart;      /* Internal use only. */
    char reganch;       /* Internal use only. */
    char *regmust;      /* Internal use only. */
    int regmlen;        /* Internal use only. */
    char program[1];    /* Unwarranted chumminess with compiler. */
} regexp;

regexp *CompileRE(char *exp, char **errorText);
int ExecRE(regexp *prog, char *string, char *end, int reverse, int isbol,
    	int isbow, char *delimiters);
void SubstituteRE(regexp *prog, char *source, char *dest, int max);
void SetREDefaultWordDelimiters(char *delimiters);
