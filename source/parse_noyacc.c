
# line 3 "parse.y"
#include <string.h>
#include <stdio.h>
#include <ctype.h>
#include <X11/Intrinsic.h>
#include <Xm/Xm.h>
#ifdef VMS
#include "../util/VMSparam.h"
#else
#ifndef __MVS__
#include <sys/param.h>
#endif
#endif /*VMS*/

#include "textBuf.h"
#include "nedit.h"
#include "rbTree.h"
#include "interpret.h"
#include "parse.h"

/* Macros to add error processing to AddOp and AddSym calls */
#define ADD_OP(op) if (!AddOp(op, &ErrMsg)) return 1
#define ADD_SYM(sym) if (!AddSym(sym, &ErrMsg)) return 1
#define ADD_IMMED(val) if (!AddImmediate(val, &ErrMsg)) return 1
#define ADD_BR_OFF(to) if (!AddBranchOffset(to, &ErrMsg)) return 1
#define SET_BR_OFF(from, to) *((int *)(from)) = ((Inst *)(to)) - ((Inst *)(from))

/* Max. length for a string constant (... there shouldn't be a maximum) */
#define MAX_STRING_CONST_LEN 5000

static const char CVSID[] = "$Id: parse_noyacc.c,v 1.2 2001/12/24 09:26:35 amai Exp $";
static int yyerror(char *s);
static int yylex(void);
int yyparse(void);
static int follow(char expect, int yes, int no);
static int follow2(char expect1, int yes1, char expect2, int yes2, int no);
static int follow_non_whitespace(char expect, int yes, int no);
static Symbol *matchesActionRoutine(char **inPtr);

static char *ErrMsg;
static char *InPtr;
extern Inst *LoopStack[]; /* addresses of break, cont stmts */
extern Inst **LoopStackPtr;  /*  to fill at the end of a loop */


# line 48 "parse.y"
typedef union  {
    Symbol *sym;
    Inst *inst;
    int nArgs;
} YYSTYPE;
# define NUMBER 257
# define STRING 258
# define SYMBOL 259
# define IF 260
# define WHILE 261
# define ELSE 262
# define FOR 263
# define BREAK 264
# define CONTINUE 265
# define RETURN 266
# define IF_NO_ELSE 267
# define ADDEQ 268
# define SUBEQ 269
# define MULEQ 270
# define DIVEQ 271
# define MODEQ 272
# define ANDEQ 273
# define OREQ 274
# define CONCAT 275
# define OR 276
# define AND 277
# define GT 278
# define GE 279
# define LT 280
# define LE 281
# define EQ 282
# define NE 283
# define IN 284
# define UNARY_MINUS 285
# define NOT 286
# define DELETE 287
# define INCR 288
# define DECR 289
# define POW 290
#define yyclearin yychar = -1
#define yyerrok yyerrflag = 0
extern int yychar;
extern int yyerrflag;
#ifndef YYMAXDEPTH
#define YYMAXDEPTH 150
#endif
YYSTYPE yylval, yyval;
typedef int yytabelem;
# define YYERRCODE 256

# line 256 "parse.y"
 /* User Subroutines Section */


/*
** Parse a null terminated string and create a program from it (this is the
** parser entry point).  The program created by this routine can be
** executed using ExecuteProgram.  Returns program on success, or NULL
** on failure.  If the command failed, the error message is returned
** as a pointer to a static string in msg, and the length of the string up
** to where parsing failed in stoppedAt.
*/
Program *ParseMacro(char *expr, char **msg, char **stoppedAt)
{
    Program *prog;
    
    BeginCreatingProgram();
    
    /* call yyparse to parse the string and check for success.  If the parse
       failed, return the error message and string index (the grammar aborts
       parsing at the first error) */
    InPtr = expr;
    if (yyparse()) {
    	*msg = ErrMsg;
    	*stoppedAt = InPtr;
    	FreeProgram(FinishCreatingProgram());
    	return NULL;
    }
    
    /* get the newly created program */
    prog = FinishCreatingProgram();
    
    /* parse succeeded */
    *msg = "";
    *stoppedAt = InPtr;
    return prog;
}


static int yylex(void)
{
    int i, len;
    Symbol *s;
    static int stringConstIndex = 0;
    static DataValue value = {0, {0}};
    static char escape[] = "\\\"ntbrfav";
    static char replace[] = "\\\"\n\t\b\r\f\a\v";
    
    /* skip whitespace and backslash-newline combinations which are
       also considered whitespace */
    for (;;) {
    	if (*InPtr == '\\' && *(InPtr + 1) == '\n')
    	    InPtr += 2;
    	else if (*InPtr == ' ' || *InPtr == '\t')
    	    InPtr++;
    	else
    	    break;
    }
    
    /* skip comments */
    if (*InPtr == '#')
    	while (*InPtr != '\n' && *InPtr != '\0') InPtr++;
    
    /* return end of input at the end of the string */
    if (*InPtr == '\0') {
	return 0;
    }
    
    /* process number tokens */
    if (isdigit((unsigned char)*InPtr))  { /* number */
        char name[28];
        sscanf(InPtr, "%d%n", &value.val.n, &len);
        sprintf(name, "const %d", value.val.n);
        InPtr += len;
        value.tag = INT_TAG;
        if ((yylval.sym=LookupSymbol(name)) == NULL)
            yylval.sym = InstallSymbol(name, CONST_SYM, value);
        return NUMBER;
    }
    
    /* process symbol tokens.  "define" is a special case not handled
       by this parser, considered end of input.  Another special case
       is action routine names which are allowed to contain '-' despite
       the ambiguity, handled in matchesActionRoutine. */
    if (isalpha((unsigned char)*InPtr) || *InPtr == '$') {
        if ((s=matchesActionRoutine(&InPtr)) == NULL) {
            char symName[MAX_SYM_LEN+1], *p = symName;
            *p++ = *InPtr++;
            while (isalnum((unsigned char)*InPtr) || *InPtr=='_') {
		if (p >= symName + MAX_SYM_LEN)
		    InPtr++;
		else
		    *p++ = *InPtr++;
	    }
	    *p = '\0';
	    if (!strcmp(symName, "while")) return WHILE;
	    if (!strcmp(symName, "if")) return IF;
	    if (!strcmp(symName, "else")) return ELSE;
	    if (!strcmp(symName, "for")) return FOR;
	    if (!strcmp(symName, "break")) return BREAK;
	    if (!strcmp(symName, "continue")) return CONTINUE;
	    if (!strcmp(symName, "return")) return RETURN;
        if (!strcmp(symName, "in")) return IN;
        if (!strcmp(symName, "delete") && follow_non_whitespace('(', SYMBOL, DELETE) == DELETE) return DELETE;
	    if (!strcmp(symName, "define")) {
	    	InPtr -= 6;
	    	return 0;
	    }
	    if ((s=LookupSymbol(symName)) == NULL) {
        	s = InstallSymbol(symName, symName[0]=='$' ? 
		      	(isdigit((unsigned char)symName[1]) ?
            		ARG_SYM : GLOBAL_SYM) : LOCAL_SYM, value);
            	s->value.tag = NO_TAG;
            }
	}
	yylval.sym = s;
        return SYMBOL;
    }
    
    /* process quoted strings w/ embedded escape sequences */
    if (*InPtr == '\"') {
        char string[MAX_STRING_CONST_LEN], *p = string;
        char stringName[25];
        InPtr++;
        while (*InPtr != '\0' && *InPtr != '\"' && *InPtr != '\n') {
	    if (p >= string + MAX_STRING_CONST_LEN) {
	    	InPtr++;
	    	continue;
	    }
	    if (*InPtr == '\\') {
		InPtr++;
		if (*InPtr == '\n') {
		    InPtr++;
		    continue;
		}
		for (i=0; escape[i]!='\0'; i++) {
		    if (escape[i] == '\0') {
		    	*p++= *InPtr++;
		    	break;
		    } else if (escape[i] == *InPtr) {
		    	*p++ = replace[i];
		    	InPtr++;
		    	break;
		    }
		}
	    } else
		*p++= *InPtr++;
	}
	*p = '\0';
	InPtr++;
	if ((yylval.sym = LookupStringConstSymbol(string)) == NULL) {
		value.val.str = AllocString(p-string+1);
		strcpy(value.val.str, string);
		value.tag = STRING_TAG;
		sprintf(stringName, "string #%d", stringConstIndex++);
		yylval.sym = InstallSymbol(stringName, CONST_SYM, value);
	}
	return STRING;
    }
    
    /* process remaining two character tokens or return single char as token */
    switch(*InPtr++) {
    case '>':	return follow('=', GE, GT);
    case '<':	return follow('=', LE, LT);
    case '=':	return follow('=', EQ, '=');
    case '!':	return follow('=', NE, NOT);
    case '+':	return follow2('+', INCR, '=', ADDEQ, '+');
    case '-':	return follow2('-', DECR, '=', SUBEQ, '-');
    case '|':	return follow2('|', OR, '=', OREQ, '|');
    case '&':	return follow2('&', AND, '=', ANDEQ, '&');
    case '*':	return follow2('*', POW, '=', MULEQ, '*');
    case '/':   return follow('=', DIVEQ, '/');
    case '%':	return follow('=', MODEQ, '%');
    case '^':	return POW;
    default:	return *(InPtr-1);
    }
}

/*
** look ahead for >=, etc.
*/
static int follow(char expect, int yes, int no)
{
    if (*InPtr++ == expect)
	return yes;
    InPtr--;
    return no;
}
static int follow2(char expect1, int yes1, char expect2, int yes2, int no)
{
    char next = *InPtr++;
    if (next == expect1)
	return yes1;
    if (next == expect2)
    	return yes2;
    InPtr--;
    return no;
}

static int follow_non_whitespace(char expect, int yes, int no)
{
    char *localInPtr = InPtr;
    
    while (1) {
        if (*localInPtr == ' ' || *localInPtr == '\t') {
            ++localInPtr;
        }
        else if (*localInPtr == '\\' && *(localInPtr + 1) == '\n') {
            localInPtr += 2;
        }
        else if (*localInPtr == expect) {
            return(yes);
        }
        else {
            return(no);
        }
    }
}

/*
** Look (way) ahead for hyphenated routine names which begin at inPtr.  A
** hyphenated name is allowed if it is pre-defined in the global symbol
** table.  If a matching name exists, returns the symbol, and update "inPtr".
**
** I know this is horrible language design, but existing nedit action routine
** names contain hyphens.  Handling them here in the lexical analysis process
** is much easier than trying to deal with it in the parser itself.  (sorry)
*/
static Symbol *matchesActionRoutine(char **inPtr)
{
    char *c, *symPtr;
    int hasDash = False;
    char symbolName[MAX_SYM_LEN+1];
    Symbol *s;
    
    symPtr = symbolName;
    for (c = *inPtr; isalnum((unsigned char)*c) || *c=='_' || 
      	    ( *c=='-' && isalnum((unsigned char)(*(c+1)))); c++) {
    	if (*c == '-')
    	    hasDash = True;
    	*symPtr++ = *c;
    }
    if (!hasDash)
    	return NULL;
    *symPtr = '\0';
    s = LookupSymbol(symbolName);
    if (s != NULL)
    	*inPtr = c;
    return s;
}

/*
** Called by yacc to report errors (just stores for returning when
** parsing is aborted.  The error token action is to immediate abort
** parsing, so this message is immediately reported to the caller
** of ParseExpr)
*/
static int yyerror(char *s)
{
    ErrMsg = s;
    return 0;
}
static const yytabelem yyexca[] ={
-1, 1,
	0, -1,
	-2, 0,
-1, 137,
	269, 33,
	270, 35,
	271, 37,
	272, 39,
	273, 41,
	274, 43,
	-2, 31,
	};
# define YYNPROD 97
# define YYLAST 652
static const yytabelem yyact[]={

    75,    83,   190,    70,   171,    73,    71,   170,    72,   169,
    74,   168,   110,   167,    75,    83,    36,   136,   166,    73,
    71,    37,    72,   165,    74,   173,    75,    75,    83,   145,
    97,    73,    73,    71,    96,    72,    74,    74,    55,    54,
    75,    83,    15,    53,     3,    73,    71,     8,    72,     4,
    74,    62,   109,    75,    70,    43,    63,   148,    73,    71,
     6,    72,   138,    74,   138,   162,   138,   114,    70,     6,
    17,    18,    19,    56,   163,    64,    41,     6,   112,   111,
    70,    70,   113,    59,    17,    18,    19,    84,    27,    26,
   155,    15,     9,    20,    70,    21,    12,    13,    14,   187,
    25,    84,   114,     6,    67,    29,    42,    70,     6,   147,
    61,   157,   138,   146,    84,   137,    99,    75,    90,    17,
    18,    19,    73,    71,    31,    72,   139,    74,   154,   138,
    28,    24,   153,   152,    36,   151,    15,     9,    20,    37,
    21,    12,    13,    14,   150,   149,    15,     9,    20,    43,
    21,    12,    13,    14,    36,   176,     1,    16,    36,    37,
   101,    86,   144,    37,    17,    18,    19,    85,   172,    10,
    41,    70,    11,   159,    17,    18,    19,    46,    47,    48,
    49,    50,    51,    52,   186,     0,     0,   117,     0,     0,
     0,     0,    57,   160,     0,     0,     7,   143,    22,     0,
    42,     0,    76,     0,     0,     0,   158,   135,     0,     0,
    64,     0,     0,     0,   175,     0,     0,     7,     0,     0,
     0,     5,     0,     0,   185,     0,   140,     0,     0,     0,
     0,     0,     0,    33,    34,    35,     0,     0,     0,    89,
    88,    77,    78,    79,    80,    81,    82,    87,     0,     0,
    22,     0,     0,    76,    88,    77,    78,    79,    80,    81,
    82,    87,    38,     0,    39,    40,     0,    76,    77,    78,
    79,    80,    81,    82,    87,     0,     0,     0,     0,    76,
    76,    77,    78,    79,    80,    81,    82,    87,     0,     0,
     0,     0,     0,    76,    77,    78,    79,    80,    81,    82,
    87,     0,     0,    44,    45,     0,    76,     0,     0,    15,
     9,    20,     0,    21,    12,    13,    14,     0,    15,     9,
    20,     0,    21,    12,    13,    14,    15,     9,    20,     0,
    21,    12,    13,    14,     0,     0,     0,    17,    18,    19,
     0,     0,     0,     0,     0,     0,    17,    18,    19,   161,
     0,    33,    34,    35,    17,    18,    19,    15,     9,    20,
     0,    21,    12,    13,    14,     0,    91,    92,     7,     0,
    76,    33,    34,    35,     0,    33,    34,    35,     0,    22,
    38,     0,    39,    40,    32,    17,    18,    19,     0,     0,
     0,   189,     0,   115,     0,   193,     0,    44,    45,   195,
    38,     0,    39,    40,    38,     0,    39,    40,     0,     0,
    60,    60,     2,     0,     0,    68,     0,     0,    23,     0,
     0,     0,    94,    95,     0,     0,     0,     0,     0,     0,
     0,     0,     0,     0,     0,     0,     0,    58,     0,     0,
     0,    65,    66,     0,    69,     0,     0,     0,     0,     0,
     0,     0,     0,     0,     0,     0,   118,   119,   120,   121,
   122,   123,   124,   125,   126,   127,   128,   129,   130,   131,
   132,   133,   134,     0,     0,     0,     0,     0,    68,   100,
   116,     0,     0,    68,     0,    68,     0,    68,    68,    68,
    68,    68,    68,    68,    30,     0,     0,     0,    60,     0,
     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     0,     0,     0,     0,     0,     0,    93,     0,     0,     0,
     0,    98,     0,     0,   141,   142,   102,   103,   104,   105,
   106,   107,   108,     0,     0,     0,     0,     0,     0,     0,
     0,    68,     0,     0,     0,     0,     0,     0,     0,    68,
     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     0,     0,    68,    68,    68,    68,    68,    68,    68,     0,
     0,     0,   174,     0,     0,     0,     0,     0,     0,     0,
     0,     0,     0,     0,     0,   184,     0,     0,     0,   188,
     0,     0,     0,     0,     0,     0,     0,     0,     0,   191,
   192,     0,     0,   194,     0,     0,     0,     0,     0,     0,
     0,     0,     0,     0,     0,     0,     0,     0,   156,     0,
     0,     0,     0,     0,     0,     0,     0,     0,   164,     0,
     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     0,     0,     0,     0,     0,   177,   178,   179,   180,   181,
   182,   183 };
static const yytabelem yypact[]={

  -212, -1000,    98, -1000,  -168, -1000, -1000, -1000,   121,    60,
    49,    48,   120,    95,   114,    15,   -91,  -216,  -220,  -221,
 -1000, -1000, -1000,    67, -1000,   118,   118,  -203, -1000, -1000,
    94, -1000,   -37, -1000, -1000,    78,   118,   118,   118,  -225,
  -229,   118,   118,   118, -1000, -1000,   118,   118,   118,   118,
   118,   118,   118,   -39, -1000, -1000,  -113, -1000,    93,    38,
   -37,    37,    23,   109, -1000,    93,    93, -1000,   -37,    93,
   118,   118,   118,   118,   118,   118,   118,   118,   118,   118,
   118,   118,   118,   118,   118,   118,   118,   118, -1000, -1000,
   118, -1000, -1000,   -24,   -88,   -88, -1000, -1000,   118,    22,
   118,    85,   118,   118,   118,   118,   118,   118,   118,   118,
 -1000, -1000, -1000,   118,  -217,  -230,    93,    20,   -11,   -11,
   -88,   -88,   -88,   -88,    80,    80,    80,    80,    80,    80,
    16,     3,   -10,   -23,    80,    68, -1000,    -4,   118, -1000,
    18,    50,    50,     6, -1000,    33, -1000, -1000,   118,  -245,
  -251,  -257,  -260,  -263,  -266,  -270,   118, -1000,  -237, -1000,
 -1000, -1000,  -217, -1000,   118,   118,   118,   118,   118,   118,
   118,   118, -1000, -1000,    59,    58, -1000,   118,   118,   118,
   118,   118,   118,   118,    50,  -123, -1000, -1000,    50, -1000,
 -1000,    93,    50, -1000,    93, -1000 };
static const yytabelem yypgo[]={

     0,   116,    83,    51,   172,   169,   168,   167,   161,   157,
   156,   412,    49,   206,   193,    47,   155,   479,   145,   144,
   135,   133,   132,   128,    90,   384 };
static const yytabelem yyr1[]={

     0,    10,    10,    10,    10,    13,    13,    13,    12,    12,
    14,    14,    14,    14,    14,    16,    14,    14,    14,    14,
    14,    15,    15,    15,    15,    15,    15,    15,    15,    15,
    15,    18,    15,    19,    15,    20,    15,    21,    15,    22,
    15,    23,    15,    24,    15,    15,    15,    15,    15,    15,
     9,     3,     3,     3,     1,     1,     1,    17,    17,    25,
    25,    25,    25,    25,    25,    25,    25,    25,    25,    25,
    25,    25,    25,    25,    25,    25,    25,    25,    25,    25,
    25,    25,    25,    25,    25,    25,    25,    25,     5,     4,
     6,     2,     2,     7,     8,    11,    11 };
static const yytabelem yyr2[]={

     0,     5,    11,     9,     3,    10,     8,     2,     2,     4,
     6,    13,    19,    13,    21,     1,    19,     7,     7,     9,
     7,     7,     7,     7,     7,     7,     7,     7,     7,    11,
    13,     1,    15,     1,    15,     1,    15,     1,    15,     1,
    15,     1,    15,     1,    15,     9,     5,     5,     5,     5,
     3,     1,     3,     7,     1,     3,     7,     2,     5,     3,
     3,     3,     9,     6,     9,     7,     7,     7,     7,     7,
     7,     5,     7,     7,     7,     7,     7,     7,     7,     7,
     7,     7,     5,     5,     5,     5,     5,     7,     3,     3,
     3,     1,     3,     3,     3,     0,     4 };
static const yytabelem yychk[]={

 -1000,   -10,   -11,   256,   -12,   123,    10,   -14,   -15,   260,
    -5,    -4,   264,   265,   266,   259,    -9,   287,   288,   289,
   261,   263,   -14,   -11,    10,    40,    40,    40,    10,    10,
   -17,    10,   -25,   257,   258,   259,    40,    45,   286,   288,
   289,    61,    91,    40,   288,   289,   268,   269,   270,   271,
   272,   273,   274,   259,   259,   259,   -12,   125,   -11,    -2,
   -25,    -2,    -3,   259,   -15,   -11,   -11,    10,   -25,   -11,
    91,    43,    45,    42,    47,    37,   290,   278,   279,   280,
   281,   282,   283,    38,   124,    -7,    -8,   284,   277,   276,
    40,   288,   289,   -17,   -25,   -25,   259,   259,   -17,    -1,
   -17,    -1,   -17,   -17,   -17,   -17,   -17,   -17,   -17,    91,
   125,    41,    41,    59,    44,   284,   -11,    -1,   -25,   -25,
   -25,   -25,   -25,   -25,   -25,   -25,   -25,   -25,   -25,   -25,
   -25,   -25,   -25,   -25,   -25,    -1,    41,    93,    44,    41,
    -1,   -11,   -11,    -2,   -15,   259,    93,    41,    61,   -18,
   -19,   -20,   -21,   -22,   -23,   -24,   -17,    93,   -13,   123,
   -14,   -13,    59,    41,   -17,   268,   269,   270,   271,   272,
   273,   274,    -6,   262,   -11,    -3,   -16,   -17,   -17,   -17,
   -17,   -17,   -17,   -17,   -11,   -12,   125,    41,   -11,   -13,
   125,   -11,   -11,   -13,   -11,   -13 };
static const yytabelem yydef[]={

    95,    -2,     0,     4,     1,    95,    96,     8,     0,     0,
     0,     0,     0,     0,     0,    50,     0,     0,     0,     0,
    88,    89,     9,     0,    95,    91,    91,    51,    95,    95,
     0,    95,    57,    59,    60,    61,     0,     0,     0,     0,
     0,     0,    54,    54,    47,    49,     0,     0,     0,     0,
     0,     0,     0,     0,    46,    48,     0,     3,    10,     0,
    92,     0,     0,    50,    52,    17,    18,    95,    58,    20,
    54,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     0,     0,     0,     0,     0,     0,     0,     0,    93,    94,
    54,    84,    86,     0,    71,    82,    83,    85,    21,     0,
    55,     0,    22,    23,    24,    25,    26,    27,    28,    54,
     2,    95,    95,    91,     0,     0,    19,     0,    65,    66,
    67,    68,    69,    70,    72,    73,    74,    75,    76,    77,
    78,    79,    80,    81,    87,     0,    63,    -2,     0,    45,
     0,     0,     0,     0,    53,     0,    64,    62,     0,     0,
     0,     0,     0,     0,     0,     0,    56,    29,    11,    95,
     7,    13,    51,    15,    30,     0,     0,     0,     0,     0,
     0,     0,    95,    90,     0,     0,    95,    32,    34,    36,
    38,    40,    42,    44,     0,     0,    95,    95,     0,    12,
    95,     6,     0,    16,     5,    14 };
typedef struct { char *t_name; int t_val; } yytoktype;
#ifndef YYDEBUG
#	define YYDEBUG	0	/* don't allow debugging */
#endif

#if YYDEBUG

yytoktype yytoks[] =
{
	"NUMBER",	257,
	"STRING",	258,
	"SYMBOL",	259,
	"IF",	260,
	"WHILE",	261,
	"ELSE",	262,
	"FOR",	263,
	"BREAK",	264,
	"CONTINUE",	265,
	"RETURN",	266,
	"IF_NO_ELSE",	267,
	"=",	61,
	"ADDEQ",	268,
	"SUBEQ",	269,
	"MULEQ",	270,
	"DIVEQ",	271,
	"MODEQ",	272,
	"ANDEQ",	273,
	"OREQ",	274,
	"CONCAT",	275,
	"OR",	276,
	"AND",	277,
	"|",	124,
	"&",	38,
	"GT",	278,
	"GE",	279,
	"LT",	280,
	"LE",	281,
	"EQ",	282,
	"NE",	283,
	"IN",	284,
	"+",	43,
	"-",	45,
	"*",	42,
	"/",	47,
	"%",	37,
	"UNARY_MINUS",	285,
	"NOT",	286,
	"DELETE",	287,
	"INCR",	288,
	"DECR",	289,
	"POW",	290,
	"[",	91,
	"(",	40,
	"-unknown-",	-1	/* ends search */
};

char * yyreds[] =
{
	"-no such reduction-",
      "program : blank stmts",
      "program : blank '{' blank stmts '}'",
      "program : blank '{' blank '}'",
      "program : error",
      "block : '{' blank stmts '}' blank",
      "block : '{' blank '}' blank",
      "block : stmt",
      "stmts : stmt",
      "stmts : stmts stmt",
      "stmt : simpstmt '\n' blank",
      "stmt : IF '(' cond ')' blank block",
      "stmt : IF '(' cond ')' blank block else blank block",
      "stmt : while '(' cond ')' blank block",
      "stmt : for '(' comastmts ';' cond ';' comastmts ')' blank block",
      "stmt : for '(' SYMBOL IN SYMBOL ')'",
      "stmt : for '(' SYMBOL IN SYMBOL ')' blank block",
      "stmt : BREAK '\n' blank",
      "stmt : CONTINUE '\n' blank",
      "stmt : RETURN expr '\n' blank",
      "stmt : RETURN '\n' blank",
      "simpstmt : SYMBOL '=' expr",
      "simpstmt : evalsym ADDEQ expr",
      "simpstmt : evalsym SUBEQ expr",
      "simpstmt : evalsym MULEQ expr",
      "simpstmt : evalsym DIVEQ expr",
      "simpstmt : evalsym MODEQ expr",
      "simpstmt : evalsym ANDEQ expr",
      "simpstmt : evalsym OREQ expr",
      "simpstmt : DELETE SYMBOL '[' arglist ']'",
      "simpstmt : SYMBOL '[' arglist ']' '=' expr",
      "simpstmt : SYMBOL '[' arglist ']'",
      "simpstmt : SYMBOL '[' arglist ']' ADDEQ expr",
      "simpstmt : SYMBOL '[' arglist ']'",
      "simpstmt : SYMBOL '[' arglist ']' SUBEQ expr",
      "simpstmt : SYMBOL '[' arglist ']'",
      "simpstmt : SYMBOL '[' arglist ']' MULEQ expr",
      "simpstmt : SYMBOL '[' arglist ']'",
      "simpstmt : SYMBOL '[' arglist ']' DIVEQ expr",
      "simpstmt : SYMBOL '[' arglist ']'",
      "simpstmt : SYMBOL '[' arglist ']' MODEQ expr",
      "simpstmt : SYMBOL '[' arglist ']'",
      "simpstmt : SYMBOL '[' arglist ']' ANDEQ expr",
      "simpstmt : SYMBOL '[' arglist ']'",
      "simpstmt : SYMBOL '[' arglist ']' OREQ expr",
      "simpstmt : SYMBOL '(' arglist ')'",
      "simpstmt : INCR SYMBOL",
      "simpstmt : SYMBOL INCR",
      "simpstmt : DECR SYMBOL",
      "simpstmt : SYMBOL DECR",
      "evalsym : SYMBOL",
      "comastmts : /* empty */",
      "comastmts : simpstmt",
      "comastmts : comastmts ',' simpstmt",
      "arglist : /* empty */",
      "arglist : expr",
      "arglist : arglist ',' expr",
      "expr : numexpr",
      "expr : expr numexpr",
      "numexpr : NUMBER",
      "numexpr : STRING",
      "numexpr : SYMBOL",
      "numexpr : SYMBOL '(' arglist ')'",
      "numexpr : '(' expr ')'",
      "numexpr : numexpr '[' arglist ']'",
      "numexpr : numexpr '+' numexpr",
      "numexpr : numexpr '-' numexpr",
      "numexpr : numexpr '*' numexpr",
      "numexpr : numexpr '/' numexpr",
      "numexpr : numexpr '%' numexpr",
      "numexpr : numexpr POW numexpr",
      "numexpr : '-' numexpr",
      "numexpr : numexpr GT numexpr",
      "numexpr : numexpr GE numexpr",
      "numexpr : numexpr LT numexpr",
      "numexpr : numexpr LE numexpr",
      "numexpr : numexpr EQ numexpr",
      "numexpr : numexpr NE numexpr",
      "numexpr : numexpr '&' numexpr",
      "numexpr : numexpr '|' numexpr",
      "numexpr : numexpr and numexpr",
      "numexpr : numexpr or numexpr",
      "numexpr : NOT numexpr",
      "numexpr : INCR SYMBOL",
      "numexpr : SYMBOL INCR",
      "numexpr : DECR SYMBOL",
      "numexpr : SYMBOL DECR",
      "numexpr : numexpr IN numexpr",
      "while : WHILE",
      "for : FOR",
      "else : ELSE",
      "cond : /* empty */",
      "cond : numexpr",
      "and : AND",
      "or : OR",
      "blank : /* empty */",
      "blank : blank '\n'",
};
#endif /* YYDEBUG */
/*
 * (c) Copyright 1990, OPEN SOFTWARE FOUNDATION, INC.
 * ALL RIGHTS RESERVED
 */
/*
 * OSF/1 Release 1.0
 */
/* @(#)yaccpar	1.3  com/cmd/lang/yacc,3.1, 9/7/89 18:46:37 */
/*
** Skeleton parser driver for yacc output
*/

/*
** yacc user known macros and defines
*/
#ifdef YYSPLIT
#   define YYERROR	return(-2)
#else
#   define YYERROR	goto yyerrlab
#endif

#define YYACCEPT	return(0)
#define YYABORT		return(1)
#define YYBACKUP( newtoken, newvalue )\
{\
	if ( yychar >= 0 || ( yyr2[ yytmp ] >> 1 ) != 1 )\
	{\
		yyerror( "syntax error - cannot backup" );\
		goto yyerrlab;\
	}\
	yychar = newtoken;\
	yystate = *yyps;\
	yylval = newvalue;\
	goto yynewstate;\
}
#define YYRECOVERING()	(!!yyerrflag)
#ifndef YYDEBUG
#	define YYDEBUG	1	/* make debugging available */
#endif

/*
** user known globals
*/
int yydebug;			/* set to 1 to get debugging */

/*
** driver internal defines
*/
#define YYFLAG		(-1000)

#ifdef YYSPLIT
#   define YYSCODE { \
			extern int (*yyf[])(); \
			register int yyret; \
			if (yyf[yytmp]) \
			    if ((yyret=(*yyf[yytmp])()) == -2) \
				    goto yyerrlab; \
				else if (yyret>=0) return(yyret); \
		   }
#endif

/*
** local variables used by the parser
 * these should be static in order to support
 * multiple parsers in a single executable program. POSIX 1003.2-1993
 */
static YYSTYPE yyv[ YYMAXDEPTH ];	/* value stack */
static int yys[ YYMAXDEPTH ];		/* state stack */

static YYSTYPE *yypv;			/* top of value stack */
static YYSTYPE *yypvt;			/* top of value stack for $vars */
static int *yyps;			/* top of state stack */

static int yystate;			/* current state */
static int yytmp;			/* extra var (lasts between blocks) */

/*
** global variables used by the parser - renamed as a result of -p
*/
int yynerrs;			/* number of errors */
int yyerrflag;			/* error recovery flag */
int yychar;			/* current input token number */

/*
** yyparse - return 0 if worked, 1 if syntax error not recovered from
*/
int
yyparse()
{
	/*
	** Initialize externals - yyparse may be called more than once
	*/
	yypv = &yyv[-1];
	yyps = &yys[-1];
	yystate = 0;
	yytmp = 0;
	yynerrs = 0;
	yyerrflag = 0;
	yychar = -1;

	goto yystack;
	{
		register YYSTYPE *yy_pv;	/* top of value stack */
		register int *yy_ps;		/* top of state stack */
		register int yy_state;		/* current state */
		register int  yy_n;		/* internal state number info */

		/*
		** get globals into registers.
		** branch to here only if YYBACKUP was called.
		*/
	yynewstate:
		yy_pv = yypv;
		yy_ps = yyps;
		yy_state = yystate;
		goto yy_newstate;

		/*
		** get globals into registers.
		** either we just started, or we just finished a reduction
		*/
	yystack:
		yy_pv = yypv;
		yy_ps = yyps;
		yy_state = yystate;

		/*
		** top of for (;;) loop while no reductions done
		*/
	yy_stack:
		/*
		** put a state and value onto the stacks
		*/
#if YYDEBUG
		/*
		** if debugging, look up token value in list of value vs.
		** name pairs.  0 and negative (-1) are special values.
		** Note: linear search is used since time is not a real
		** consideration while debugging.
		*/
		if ( yydebug )
		{
			register int yy_i;

			printf( "State %d, token ", yy_state );
			if ( yychar == 0 )
				printf( "end-of-file\n" );
			else if ( yychar < 0 )
				printf( "-none-\n" );
			else
			{
				for ( yy_i = 0; yytoks[yy_i].t_val >= 0;
					yy_i++ )
				{
					if ( yytoks[yy_i].t_val == yychar )
						break;
				}
				printf( "%s\n", yytoks[yy_i].t_name );
			}
		}
#endif /* YYDEBUG */
		if ( ++yy_ps >= &yys[ YYMAXDEPTH ] )	/* room on stack? */
		{
			yyerror( "yacc stack overflow" );
			YYABORT;
		}
		*yy_ps = yy_state;
		*++yy_pv = yyval;

		/*
		** we have a new state - find out what to do
		*/
	yy_newstate:
		if ( ( yy_n = yypact[ yy_state ] ) <= YYFLAG )
			goto yydefault;		/* simple state */
#if YYDEBUG
		/*
		** if debugging, need to mark whether new token grabbed
		*/
		yytmp = yychar < 0;
#endif
		if ( ( yychar < 0 ) && ( ( yychar = yylex() ) < 0 ) )
			yychar = 0;		/* reached EOF */
#if YYDEBUG
		if ( yydebug && yytmp )
		{
			register int yy_i;

			printf( "Received token " );
			if ( yychar == 0 )
				printf( "end-of-file\n" );
			else if ( yychar < 0 )
				printf( "-none-\n" );
			else
			{
				for ( yy_i = 0; yytoks[yy_i].t_val >= 0;
					yy_i++ )
				{
					if ( yytoks[yy_i].t_val == yychar )
						break;
				}
				printf( "%s\n", yytoks[yy_i].t_name );
			}
		}
#endif /* YYDEBUG */
		if ( ( ( yy_n += yychar ) < 0 ) || ( yy_n >= YYLAST ) )
			goto yydefault;
		if ( yychk[ yy_n = yyact[ yy_n ] ] == yychar )	/*valid shift*/
		{
			yychar = -1;
			yyval = yylval;
			yy_state = yy_n;
			if ( yyerrflag > 0 )
				yyerrflag--;
			goto yy_stack;
		}

	yydefault:
		if ( ( yy_n = yydef[ yy_state ] ) == -2 )
		{
#if YYDEBUG
			yytmp = yychar < 0;
#endif
			if ( ( yychar < 0 ) && ( ( yychar = yylex() ) < 0 ) )
				yychar = 0;		/* reached EOF */
#if YYDEBUG
			if ( yydebug && yytmp )
			{
				register int yy_i;

				printf( "Received token " );
				if ( yychar == 0 )
					printf( "end-of-file\n" );
				else if ( yychar < 0 )
					printf( "-none-\n" );
				else
				{
					for ( yy_i = 0;
						yytoks[yy_i].t_val >= 0;
						yy_i++ )
					{
						if ( yytoks[yy_i].t_val
							== yychar )
						{
							break;
						}
					}
					printf( "%s\n", yytoks[yy_i].t_name );
				}
			}
#endif /* YYDEBUG */
			/*
			** look through exception table
			*/
			{
				register const int *yyxi = yyexca;

				while ( ( *yyxi != -1 ) ||
					( yyxi[1] != yy_state ) )
				{
					yyxi += 2;
				}
				while ( ( *(yyxi += 2) >= 0 ) &&
					( *yyxi != yychar ) )
					;
				if ( ( yy_n = yyxi[1] ) < 0 )
					YYACCEPT;
			}
		}

		/*
		** check for syntax error
		*/
		if ( yy_n == 0 )	/* have an error */
		{
			/* no worry about speed here! */
			switch ( yyerrflag )
			{
			case 0:		/* new error */
				yyerror( "syntax error" );
				goto skip_init;
			yyerrlab:
				/*
				** get globals into registers.
				** we have a user generated syntax type error
				*/
				yy_pv = yypv;
				yy_ps = yyps;
				yy_state = yystate;
				yynerrs++;
			skip_init:
			case 1:
			case 2:		/* incompletely recovered error */
					/* try again... */
				yyerrflag = 3;
				/*
				** find state where "error" is a legal
				** shift action
				*/
				while ( yy_ps >= yys )
				{
					yy_n = yypact[ *yy_ps ] + YYERRCODE;
					if ( yy_n >= 0 && yy_n < YYLAST &&
						yychk[yyact[yy_n]] == YYERRCODE)					{
						/*
						** simulate shift of "error"
						*/
						yy_state = yyact[ yy_n ];
						goto yy_stack;
					}
					/*
					** current state has no shift on
					** "error", pop stack
					*/
#if YYDEBUG
#	define _POP_ "Error recovery pops state %d, uncovers state %d\n"
					if ( yydebug )
						printf( _POP_, *yy_ps,
							yy_ps[-1] );
#	undef _POP_
#endif
					yy_ps--;
					yy_pv--;
				}
				/*
				** there is no state on stack with "error" as
				** a valid shift.  give up.
				*/
				YYABORT;
			case 3:		/* no shift yet; eat a token */
#if YYDEBUG
				/*
				** if debugging, look up token in list of
				** pairs.  0 and negative shouldn't occur,
				** but since timing doesn't matter when
				** debugging, it doesn't hurt to leave the
				** tests here.
				*/
				if ( yydebug )
				{
					register int yy_i;

					printf( "Error recovery discards " );
					if ( yychar == 0 )
						printf( "token end-of-file\n" );
					else if ( yychar < 0 )
						printf( "token -none-\n" );
					else
					{
						for ( yy_i = 0;
							yytoks[yy_i].t_val >= 0;
							yy_i++ )
						{
							if ( yytoks[yy_i].t_val
								== yychar )
							{
								break;
							}
						}
						printf( "token %s\n",
							yytoks[yy_i].t_name );
					}
				}
#endif /* YYDEBUG */
				if ( yychar == 0 )	/* reached EOF. quit */
					YYABORT;
				yychar = -1;
				goto yy_newstate;
			}
		}/* end if ( yy_n == 0 ) */
		/*
		** reduction by production yy_n
		** put stack tops, etc. so things right after switch
		*/
#if YYDEBUG
		/*
		** if debugging, print the string that is the user's
		** specification of the reduction which is just about
		** to be done.
		*/
		if ( yydebug )
			printf( "Reduce by (%d) \"%s\"\n",
				yy_n, yyreds[ yy_n ] );
#endif
		yytmp = yy_n;			/* value to switch over */
		yypvt = yy_pv;			/* $vars top of value stack */
		/*
		** Look in goto table for next state
		** Sorry about using yy_state here as temporary
		** register variable, but why not, if it works...
		** If yyr2[ yy_n ] doesn't have the low order bit
		** set, then there is no action to be done for
		** this reduction.  So, no saving & unsaving of
		** registers done.  The only difference between the
		** code just after the if and the body of the if is
		** the goto yy_stack in the body.  This way the test
		** can be made before the choice of what to do is needed.
		*/
		{
			/* length of production doubled with extra bit */
			register int yy_len = yyr2[ yy_n ];

			if ( !( yy_len & 01 ) )
			{
				yy_len >>= 1;
				yyval = ( yy_pv -= yy_len )[1];	/* $$ = $1 */
				yy_state = yypgo[ yy_n = yyr1[ yy_n ] ] +
					*( yy_ps -= yy_len ) + 1;
				if ( yy_state >= YYLAST ||
					yychk[ yy_state =
					yyact[ yy_state ] ] != -yy_n )
				{
					yy_state = yyact[ yypgo[ yy_n ] ];
				}
				goto yy_stack;
			}
			yy_len >>= 1;
			yyval = ( yy_pv -= yy_len )[1];	/* $$ = $1 */
			yy_state = yypgo[ yy_n = yyr1[ yy_n ] ] +
				*( yy_ps -= yy_len ) + 1;
			if ( yy_state >= YYLAST ||
				yychk[ yy_state = yyact[ yy_state ] ] != -yy_n )
			{
				yy_state = yyact[ yypgo[ yy_n ] ];
			}
		}
					/* save until reenter driver code */
		yystate = yy_state;
		yyps = yy_ps;
		yypv = yy_pv;
	}
	/*
	** code supplied by user is placed in this switch
	*/

		switch(yytmp){

case 1:
# line 81 "parse.y"
{ ADD_OP(OP_RETURN_NO_VAL); return 0; } /*NOTREACHED*/ break;
case 2:
# line 82 "parse.y"
{ ADD_OP(OP_RETURN_NO_VAL); return 0; } /*NOTREACHED*/ break;
case 3:
# line 83 "parse.y"
{ ADD_OP(OP_RETURN_NO_VAL); return 0; } /*NOTREACHED*/ break;
case 4:
# line 84 "parse.y"
{ return 1; } /*NOTREACHED*/ break;
case 11:
# line 95 "parse.y"
{ SET_BR_OFF(yypvt[-3].inst, GetPC()); } /*NOTREACHED*/ break;
case 12:
# line 97 "parse.y"
{ SET_BR_OFF(yypvt[-6].inst, (yypvt[-2].inst+1)); SET_BR_OFF(yypvt[-2].inst, GetPC()); } /*NOTREACHED*/ break;
case 13:
# line 98 "parse.y"
{ ADD_OP(OP_BRANCH); ADD_BR_OFF(yypvt[-5].inst);
                SET_BR_OFF(yypvt[-3].inst, GetPC()); FillLoopAddrs(GetPC(), yypvt[-5].inst); } /*NOTREACHED*/ break;
case 14:
# line 101 "parse.y"
{ FillLoopAddrs(GetPC()+2+(yypvt[-3].inst-(yypvt[-5].inst+1)), GetPC());
    	          SwapCode(yypvt[-5].inst+1, yypvt[-3].inst, GetPC());
    	          ADD_OP(OP_BRANCH); ADD_BR_OFF(yypvt[-7].inst); SET_BR_OFF(yypvt[-5].inst, GetPC()); } /*NOTREACHED*/ break;
case 15:
# line 105 "parse.y"
{ Symbol *iterSym = InstallIteratorSymbol();
                  ADD_OP(OP_BEGIN_ARRAY_ITER); ADD_SYM(yypvt[-1].sym); ADD_SYM(iterSym);
                  ADD_OP(OP_ARRAY_ITER); ADD_SYM(yypvt[-3].sym); ADD_SYM(iterSym);
                  ADD_BR_OFF(0); } /*NOTREACHED*/ break;
case 16:
# line 110 "parse.y"
{ FillLoopAddrs(GetPC()+2, GetPC());
                  ADD_OP(OP_BRANCH); ADD_BR_OFF(yypvt[-8].inst+3);
                  SET_BR_OFF(yypvt[-8].inst+6, GetPC());} /*NOTREACHED*/ break;
case 17:
# line 114 "parse.y"
{ ADD_OP(OP_BRANCH); ADD_BR_OFF(0); if(AddBreakAddr(GetPC()-1)) { yyerror("break outside loop"); YYERROR; } } /*NOTREACHED*/ break;
case 18:
# line 116 "parse.y"
{ ADD_OP(OP_BRANCH); ADD_BR_OFF(0); if(AddContinueAddr(GetPC()-1)) { yyerror("continue outside loop"); YYERROR; } } /*NOTREACHED*/ break;
case 19:
# line 117 "parse.y"
{ ADD_OP(OP_RETURN); } /*NOTREACHED*/ break;
case 20:
# line 118 "parse.y"
{ ADD_OP(OP_RETURN_NO_VAL); } /*NOTREACHED*/ break;
case 21:
# line 120 "parse.y"
{ ADD_OP(OP_ASSIGN); ADD_SYM(yypvt[-2].sym); } /*NOTREACHED*/ break;
case 22:
# line 121 "parse.y"
{ ADD_OP(OP_ADD); ADD_OP(OP_ASSIGN); ADD_SYM(yypvt[-2].sym); } /*NOTREACHED*/ break;
case 23:
# line 122 "parse.y"
{ ADD_OP(OP_SUB); ADD_OP(OP_ASSIGN); ADD_SYM(yypvt[-2].sym); } /*NOTREACHED*/ break;
case 24:
# line 123 "parse.y"
{ ADD_OP(OP_MUL); ADD_OP(OP_ASSIGN); ADD_SYM(yypvt[-2].sym); } /*NOTREACHED*/ break;
case 25:
# line 124 "parse.y"
{ ADD_OP(OP_DIV); ADD_OP(OP_ASSIGN); ADD_SYM(yypvt[-2].sym); } /*NOTREACHED*/ break;
case 26:
# line 125 "parse.y"
{ ADD_OP(OP_MOD); ADD_OP(OP_ASSIGN); ADD_SYM(yypvt[-2].sym); } /*NOTREACHED*/ break;
case 27:
# line 126 "parse.y"
{ ADD_OP(OP_BIT_AND); ADD_OP(OP_ASSIGN);
    	    	ADD_SYM(yypvt[-2].sym); } /*NOTREACHED*/ break;
case 28:
# line 128 "parse.y"
{ ADD_OP(OP_BIT_OR); ADD_OP(OP_ASSIGN);
    	    	ADD_SYM(yypvt[-2].sym); } /*NOTREACHED*/ break;
case 29:
# line 131 "parse.y"
{ ADD_OP(OP_ARRAY_DELETE); ADD_SYM(yypvt[-3].sym); ADD_IMMED((void *)yypvt[-1].nArgs); } /*NOTREACHED*/ break;
case 30:
# line 133 "parse.y"
{ ADD_OP(OP_ARRAY_ASSIGN); ADD_SYM(yypvt[-5].sym); ADD_IMMED((void *)yypvt[-3].nArgs); } /*NOTREACHED*/ break;
case 31:
# line 134 "parse.y"
{ ADD_OP(OP_PUSH_SYM);
                ADD_SYM(yypvt[-3].sym); ADD_OP(OP_ARRAY_REF); ADD_IMMED((void *)yypvt[-1].nArgs); } /*NOTREACHED*/ break;
case 32:
# line 137 "parse.y"
{ ADD_OP(OP_ADD); ADD_OP(OP_ARRAY_ASSIGN); ADD_SYM(yypvt[-6].sym);
                  ADD_IMMED((void *)yypvt[-4].nArgs);} /*NOTREACHED*/ break;
case 33:
# line 139 "parse.y"
{ ADD_OP(OP_PUSH_SYM); ADD_SYM(yypvt[-3].sym);
                  ADD_OP(OP_ARRAY_REF); ADD_IMMED((void *)yypvt[-1].nArgs); } /*NOTREACHED*/ break;
case 34:
# line 142 "parse.y"
{ ADD_OP(OP_SUB); ADD_OP(OP_ARRAY_ASSIGN); ADD_SYM(yypvt[-6].sym);
                  ADD_IMMED((void *)yypvt[-4].nArgs); } /*NOTREACHED*/ break;
case 35:
# line 145 "parse.y"
{ ADD_OP(OP_PUSH_SYM); ADD_SYM(yypvt[-3].sym); ADD_OP(OP_ARRAY_REF);
                  ADD_IMMED((void *)yypvt[-1].nArgs); } /*NOTREACHED*/ break;
case 36:
# line 148 "parse.y"
{ ADD_OP(OP_MUL); ADD_OP(OP_ARRAY_ASSIGN); ADD_SYM(yypvt[-6].sym);
                  ADD_IMMED((void *)yypvt[-4].nArgs); } /*NOTREACHED*/ break;
case 37:
# line 151 "parse.y"
{ ADD_OP(OP_PUSH_SYM); ADD_SYM(yypvt[-3].sym); ADD_OP(OP_ARRAY_REF);
                  ADD_IMMED((void *)yypvt[-1].nArgs); } /*NOTREACHED*/ break;
case 38:
# line 154 "parse.y"
{ ADD_OP(OP_DIV); ADD_OP(OP_ARRAY_ASSIGN); ADD_SYM(yypvt[-6].sym);
                  ADD_IMMED((void *)yypvt[-4].nArgs); } /*NOTREACHED*/ break;
case 39:
# line 157 "parse.y"
{ ADD_OP(OP_PUSH_SYM); ADD_SYM(yypvt[-3].sym); ADD_OP(OP_ARRAY_REF);
                  ADD_IMMED((void *)yypvt[-1].nArgs); } /*NOTREACHED*/ break;
case 40:
# line 160 "parse.y"
{ ADD_OP(OP_MOD); ADD_OP(OP_ARRAY_ASSIGN); ADD_SYM(yypvt[-6].sym);
                  ADD_IMMED((void *)yypvt[-4].nArgs); } /*NOTREACHED*/ break;
case 41:
# line 163 "parse.y"
{ ADD_OP(OP_PUSH_SYM); ADD_SYM(yypvt[-3].sym); ADD_OP(OP_ARRAY_REF);
                  ADD_IMMED((void *)yypvt[-1].nArgs); } /*NOTREACHED*/ break;
case 42:
# line 166 "parse.y"
{ ADD_OP(OP_BIT_AND); ADD_OP(OP_ARRAY_ASSIGN);
                ADD_SYM(yypvt[-6].sym); ADD_IMMED((void *)yypvt[-4].nArgs); } /*NOTREACHED*/ break;
case 43:
# line 169 "parse.y"
{ ADD_OP(OP_PUSH_SYM); ADD_SYM(yypvt[-3].sym); ADD_OP(OP_ARRAY_REF);
                  ADD_IMMED((void *)yypvt[-1].nArgs); } /*NOTREACHED*/ break;
case 44:
# line 172 "parse.y"
{ ADD_OP(OP_BIT_OR); ADD_OP(OP_ARRAY_ASSIGN);
                  ADD_SYM(yypvt[-6].sym); ADD_IMMED((void *)yypvt[-4].nArgs); } /*NOTREACHED*/ break;
case 45:
# line 174 "parse.y"
{ ADD_OP(OP_SUBR_CALL);
                ADD_SYM(PromoteToGlobal(yypvt[-3].sym)); ADD_IMMED((void *)yypvt[-1].nArgs); } /*NOTREACHED*/ break;
case 46:
# line 176 "parse.y"
{ ADD_OP(OP_PUSH_SYM); ADD_SYM(yypvt[-0].sym); ADD_OP(OP_INCR);
                ADD_OP(OP_ASSIGN); ADD_SYM(yypvt[-0].sym); } /*NOTREACHED*/ break;
case 47:
# line 178 "parse.y"
{ ADD_OP(OP_PUSH_SYM); ADD_SYM(yypvt[-1].sym); ADD_OP(OP_INCR);
                ADD_OP(OP_ASSIGN); ADD_SYM(yypvt[-1].sym); } /*NOTREACHED*/ break;
case 48:
# line 180 "parse.y"
{ ADD_OP(OP_PUSH_SYM); ADD_SYM(yypvt[-0].sym); ADD_OP(OP_DECR);
                ADD_OP(OP_ASSIGN); ADD_SYM(yypvt[-0].sym); } /*NOTREACHED*/ break;
case 49:
# line 182 "parse.y"
{ ADD_OP(OP_PUSH_SYM); ADD_SYM(yypvt[-1].sym); ADD_OP(OP_DECR);
                ADD_OP(OP_ASSIGN); ADD_SYM(yypvt[-1].sym); } /*NOTREACHED*/ break;
case 50:
# line 185 "parse.y"
{ yyval.sym = yypvt[-0].sym; ADD_OP(OP_PUSH_SYM); ADD_SYM(yypvt[-0].sym); } /*NOTREACHED*/ break;
case 51:
# line 187 "parse.y"
{ yyval.inst = GetPC(); } /*NOTREACHED*/ break;
case 52:
# line 188 "parse.y"
{ yyval.inst = GetPC(); } /*NOTREACHED*/ break;
case 53:
# line 189 "parse.y"
{ yyval.inst = GetPC(); } /*NOTREACHED*/ break;
case 54:
# line 191 "parse.y"
{ yyval.nArgs = 0;} /*NOTREACHED*/ break;
case 55:
# line 192 "parse.y"
{ yyval.nArgs = 1; } /*NOTREACHED*/ break;
case 56:
# line 193 "parse.y"
{ yyval.nArgs = yypvt[-2].nArgs + 1; } /*NOTREACHED*/ break;
case 58:
# line 196 "parse.y"
{ ADD_OP(OP_CONCAT); } /*NOTREACHED*/ break;
case 59:
# line 198 "parse.y"
{ ADD_OP(OP_PUSH_SYM); ADD_SYM(yypvt[-0].sym); } /*NOTREACHED*/ break;
case 60:
# line 199 "parse.y"
{ ADD_OP(OP_PUSH_SYM); ADD_SYM(yypvt[-0].sym); } /*NOTREACHED*/ break;
case 61:
# line 200 "parse.y"
{ ADD_OP(OP_PUSH_SYM); ADD_SYM(yypvt[-0].sym); } /*NOTREACHED*/ break;
case 62:
# line 201 "parse.y"
{ ADD_OP(OP_SUBR_CALL);
	    	ADD_SYM(PromoteToGlobal(yypvt[-3].sym)); ADD_IMMED((void *)yypvt[-1].nArgs);
		ADD_OP(OP_FETCH_RET_VAL);} /*NOTREACHED*/ break;
case 64:
# line 206 "parse.y"
{ ADD_OP(OP_ARRAY_REF); ADD_IMMED((void *)yypvt[-1].nArgs); } /*NOTREACHED*/ break;
case 65:
# line 207 "parse.y"
{ ADD_OP(OP_ADD); } /*NOTREACHED*/ break;
case 66:
# line 208 "parse.y"
{ ADD_OP(OP_SUB); } /*NOTREACHED*/ break;
case 67:
# line 209 "parse.y"
{ ADD_OP(OP_MUL); } /*NOTREACHED*/ break;
case 68:
# line 210 "parse.y"
{ ADD_OP(OP_DIV); } /*NOTREACHED*/ break;
case 69:
# line 211 "parse.y"
{ ADD_OP(OP_MOD); } /*NOTREACHED*/ break;
case 70:
# line 212 "parse.y"
{ ADD_OP(OP_POWER); } /*NOTREACHED*/ break;
case 71:
# line 213 "parse.y"
{ ADD_OP(OP_NEGATE); } /*NOTREACHED*/ break;
case 72:
# line 214 "parse.y"
{ ADD_OP(OP_GT); } /*NOTREACHED*/ break;
case 73:
# line 215 "parse.y"
{ ADD_OP(OP_GE); } /*NOTREACHED*/ break;
case 74:
# line 216 "parse.y"
{ ADD_OP(OP_LT); } /*NOTREACHED*/ break;
case 75:
# line 217 "parse.y"
{ ADD_OP(OP_LE); } /*NOTREACHED*/ break;
case 76:
# line 218 "parse.y"
{ ADD_OP(OP_EQ); } /*NOTREACHED*/ break;
case 77:
# line 219 "parse.y"
{ ADD_OP(OP_NE); } /*NOTREACHED*/ break;
case 78:
# line 220 "parse.y"
{ ADD_OP(OP_BIT_AND); } /*NOTREACHED*/ break;
case 79:
# line 221 "parse.y"
{ ADD_OP(OP_BIT_OR); } /*NOTREACHED*/ break;
case 80:
# line 223 "parse.y"
{ ADD_OP(OP_AND); SET_BR_OFF(yypvt[-1].inst, GetPC()); } /*NOTREACHED*/ break;
case 81:
# line 225 "parse.y"
{ ADD_OP(OP_OR); SET_BR_OFF(yypvt[-1].inst, GetPC()); } /*NOTREACHED*/ break;
case 82:
# line 226 "parse.y"
{ ADD_OP(OP_NOT); } /*NOTREACHED*/ break;
case 83:
# line 227 "parse.y"
{ ADD_OP(OP_PUSH_SYM); ADD_SYM(yypvt[-0].sym); ADD_OP(OP_INCR);
    	    	ADD_OP(OP_DUP); ADD_OP(OP_ASSIGN); ADD_SYM(yypvt[-0].sym); } /*NOTREACHED*/ break;
case 84:
# line 229 "parse.y"
{ ADD_OP(OP_PUSH_SYM); ADD_SYM(yypvt[-1].sym); ADD_OP(OP_DUP);
	    	ADD_OP(OP_INCR); ADD_OP(OP_ASSIGN); ADD_SYM(yypvt[-1].sym); } /*NOTREACHED*/ break;
case 85:
# line 231 "parse.y"
{ ADD_OP(OP_PUSH_SYM); ADD_SYM(yypvt[-0].sym); ADD_OP(OP_DECR);
	    	ADD_OP(OP_DUP); ADD_OP(OP_ASSIGN); ADD_SYM(yypvt[-0].sym); } /*NOTREACHED*/ break;
case 86:
# line 233 "parse.y"
{ ADD_OP(OP_PUSH_SYM); ADD_SYM(yypvt[-1].sym); ADD_OP(OP_DUP);
	    	ADD_OP(OP_DECR); ADD_OP(OP_ASSIGN); ADD_SYM(yypvt[-1].sym); } /*NOTREACHED*/ break;
case 87:
# line 235 "parse.y"
{ ADD_OP(OP_IN_ARRAY); } /*NOTREACHED*/ break;
case 88:
# line 237 "parse.y"
{ yyval.inst = GetPC(); StartLoopAddrList(); } /*NOTREACHED*/ break;
case 89:
# line 239 "parse.y"
{ StartLoopAddrList(); yyval.inst = GetPC(); } /*NOTREACHED*/ break;
case 90:
# line 241 "parse.y"
{ ADD_OP(OP_BRANCH); yyval.inst = GetPC(); ADD_BR_OFF(0); } /*NOTREACHED*/ break;
case 91:
# line 243 "parse.y"
{ ADD_OP(OP_BRANCH_NEVER); yyval.inst = GetPC(); ADD_BR_OFF(0); } /*NOTREACHED*/ break;
case 92:
# line 244 "parse.y"
{ ADD_OP(OP_BRANCH_FALSE); yyval.inst = GetPC(); ADD_BR_OFF(0); } /*NOTREACHED*/ break;
case 93:
# line 246 "parse.y"
{ ADD_OP(OP_DUP); ADD_OP(OP_BRANCH_FALSE); yyval.inst = GetPC();
    	    	ADD_BR_OFF(0); } /*NOTREACHED*/ break;
case 94:
# line 249 "parse.y"
{ ADD_OP(OP_DUP); ADD_OP(OP_BRANCH_TRUE); yyval.inst = GetPC();
    	    	ADD_BR_OFF(0); } /*NOTREACHED*/ break;
}


	goto yystack;		/* reset registers in driver code */
}
