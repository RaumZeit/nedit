/* $Id: regularExp.h,v 1.3 2001/02/26 23:38:03 edg Exp $ */
/*----------------------------------------------------------------------*
 *  This is regularExp.h: NEdit Regular Expression Package Header File
 *----------------------------------------------------------------------*/

/* Number of text capturing parentheses allowed. */

#define NSUBEXP 50

/* Structure to contain the compiled form of a regular expression plus
   pointers to matched text.  `program' is the actual compiled regex code. */

typedef struct regexp {
   char *startp [NSUBEXP];  /* Captured text starting locations. */
   char *endp   [NSUBEXP];  /* Captured text ending locations. */
   char *extentp;           /* Points to the maximum extent of text scanned by
                               ExecRE to achieve a match (needed because of
                               positive look-ahead.) */
   char  match_start;       /* Internal use only. */
   char  anchor;            /* Internal use only. */
   char  program [1];       /* Unwarranted chumminess with compiler. */
} regexp;

/* Compiles a regular expression into the internal format used by `ExecRE'. */

regexp * CompileRE (
   char  *exp,        /* String containing the regex specification. */
   char **errorText); /* Text of any error message produced. */

/* Match a `regexp' structure against a string. */

int ExecRE (
   regexp *prog,                /* Compiled regex. */
   regexp *cross_regex_backref, /* Pointer to a `regexp' that was used in a
                                   previous execution of ExecRE.  Used to
                                   implement back references across regular
                                   expressions for use in syntax
                                   highlighting.*/
   char   *string,              /* Text to search within. */
   char   *end,                 /* Pointer to the end of `string'.  If NULL will
                                   scan from `string' until '\0' is found. */
   int     reverse,             /* Backward search. */
   char    prev_char,           /* Character immediately prior to `string'.  Set
                                   to '\n' or '\0' if true beginning of text. */
   char    succ_char,           /* Character immediately after `end'.  Set
                                   to '\n' or '\0' if true beginning of text. */
   char   *delimiters);         /* Word delimiters to use (NULL for default) */

/* Perform substitutions after a `regexp' match. */

void SubstituteRE (
   regexp *prog,
   char   *source,
   char   *dest,
   int     max);

/* Builds a default delimiter table that persists across `ExecRE' calls that
   is identical to `delimiters'.  Pass NULL for "default default" set of
   delimiters. */

void SetREDefaultWordDelimiters (
   char *delimiters);

/* Enable (or disable) brace counting quantifiers, e.g. `(foo){0,3}'. */

void EnableCountingQuantifier (int is_enabled);
