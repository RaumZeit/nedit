#ifndef lint
static char const 
yyrcsid[] = "$FreeBSD: src/usr.bin/yacc/skeleton.c,v 1.28 2000/01/17 02:04:06 bde Exp $";
#endif
#include <stdlib.h>
#define YYBYACC 1
#define YYMAJOR 1
#define YYMINOR 9
#define YYLEX yylex()
#define YYEMPTY -1
#define yyclearin (yychar=(YYEMPTY))
#define yyerrok (yyerrflag=0)
#define YYRECOVERING() (yyerrflag!=0)
static int yygrowstack();
#define YYPREFIX "yy"
#line 3 "parse.y"
#include "parse.h"
#include "textBuf.h"
#include "nedit.h"
#include "rbTree.h"
#include "interpret.h"

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

/* Macros to add error processing to AddOp and AddSym calls */
#define ADD_OP(op) if (!AddOp(op, &ErrMsg)) return 1
#define ADD_SYM(sym) if (!AddSym(sym, &ErrMsg)) return 1
#define ADD_IMMED(val) if (!AddImmediate(val, &ErrMsg)) return 1
#define ADD_BR_OFF(to) if (!AddBranchOffset(to, &ErrMsg)) return 1
#define SET_BR_OFF(from, to) *((int *)(from)) = ((Inst *)(to)) - ((Inst *)(from))

/* Max. length for a string constant (... there shouldn't be a maximum) */
#define MAX_STRING_CONST_LEN 5000

static const char CVSID[] = "$Id: parse_noyacc.c,v 1.4 2002/09/05 23:17:25 slobasso Exp $";
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

#line 48 "parse.y"
typedef union {
    Symbol *sym;
    Inst *inst;
    int nArgs;
} YYSTYPE;
#line 67 "y.tab.c"
#define YYERRCODE 256
#define NUMBER 257
#define STRING 258
#define SYMBOL 259
#define IF 260
#define WHILE 261
#define ELSE 262
#define FOR 263
#define BREAK 264
#define CONTINUE 265
#define RETURN 266
#define IF_NO_ELSE 267
#define ADDEQ 268
#define SUBEQ 269
#define MULEQ 270
#define DIVEQ 271
#define MODEQ 272
#define ANDEQ 273
#define OREQ 274
#define CONCAT 275
#define OR 276
#define AND 277
#define GT 278
#define GE 279
#define LT 280
#define LE 281
#define EQ 282
#define NE 283
#define IN 284
#define UNARY_MINUS 285
#define NOT 286
#define DELETE 287
#define INCR 288
#define DECR 289
#define POW 290
const short yylhs[] = {                                        -1,
    0,    0,    0,    0,   13,   13,   13,   12,   12,   14,
   14,   14,   14,   14,   16,   14,   14,   14,   14,   14,
   15,   15,   15,   15,   15,   15,   15,   15,   15,   15,
   15,   15,   15,   15,   15,   15,   15,   15,   15,   15,
   15,   15,   15,   15,   15,   15,   10,    3,    3,    3,
    1,    1,    1,   17,   17,   19,   19,   18,   18,    9,
   20,   20,   20,   20,   20,   20,   20,   20,   20,   20,
   20,   20,   20,   20,   20,   20,   20,   20,   20,   20,
   20,   20,   20,   20,   20,   20,   20,   20,   20,    5,
    4,    6,    2,    2,    7,    8,   11,   11,
};
const short yylen[] = {                                         2,
    2,    5,    4,    1,    5,    4,    1,    1,    2,    3,
    6,    9,    6,   10,    0,    9,    3,    3,    4,    3,
    3,    3,    3,    3,    3,    3,    3,    3,    5,    6,
    6,    6,    6,    6,    6,    6,    6,    5,    5,    5,
    5,    4,    2,    2,    2,    2,    1,    0,    1,    3,
    0,    1,    3,    1,    2,    1,    4,    1,    4,    1,
    1,    1,    1,    4,    3,    4,    3,    3,    3,    3,
    3,    3,    2,    3,    3,    3,    3,    3,    3,    3,
    3,    3,    3,    2,    2,    2,    2,    2,    3,    1,
    1,    1,    0,    1,    1,    1,    0,    2,
};
const short yydefred[] = {                                      0,
    4,    0,    0,    0,    0,   90,   91,    0,    0,    0,
    0,    0,    0,   97,   98,    0,    0,    0,    0,    8,
    0,    0,    0,   44,   46,    0,    0,   97,   97,   61,
   62,    0,    0,    0,    0,    0,    0,   97,    0,    0,
   58,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    9,   97,    0,    0,
    0,    0,    0,    0,    0,    0,   86,   88,    0,    0,
    0,   85,   87,    0,    0,   97,    0,   96,   95,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    3,    0,    0,    0,   49,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,   42,    0,   97,    0,   65,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    2,    0,    0,    0,   97,    0,    0,    0,
   64,   66,    0,    0,    0,    0,    0,    0,   50,    0,
    0,    0,    0,    0,    0,    0,    0,    0,   38,   39,
   97,    0,    7,   15,    0,   13,    0,    0,    0,    0,
    0,    0,    0,    0,    0,   92,   97,   97,    0,   97,
    0,    0,    0,   97,    0,   97,   12,   16,    0,    0,
   14,
};
const short yydgoto[] = {                                       2,
   61,   63,  104,   16,   17,  187,   96,   97,  156,   18,
    3,   19,  172,  173,   21,  188,   62,   42,   22,   40,
};
const short yysindex[] = {                                   -215,
    0,    0,  718,  -33,   21,    0,    0,   35,   44,  176,
 -191, -182, -178,    0,    0,   62,   68,  412, 1139,    0,
   80,   13,  380,    0,    0,  380,  380,    0,    0,    0,
    0,    3,  380,  380, -146, -144,  380,    0,  504,  -32,
    0,   28,    0,   33,    0,   38,  222, -122,  380,  380,
  380,  380,  380,  380,  380,  380,    0,    0,  380,  380,
   14,  380,   92,  -32,  137,  137,    0,    0,  380,  -91,
  -91,    0,    0,  301,  137,    0,  -32,    0,    0,  380,
  380,  380,  380,  380,  380,  380,  380,  380,  380,  380,
  380,  380,  380,  380,  380,  380,  380,  380,  380,  380,
    0, 1121,   11,  -26,    0,  110,  380,  380,  380,  380,
  380,  380,  380,  137,  -40,    0,  380,    0,   84,    0,
  137, 1271, 1285,   -5,   -5,   -5,   -5,   -5,   -5,   -5,
   -3,   -3,  -91,  -91,  -91,  -91,  -30, 1253, 1216,  -19,
  -17,  -15,    0,  380,  380,  -65,    0,  698,  380,  879,
    0,    0,    0,    0,    0,  111,  -32,  109,    0,  879,
  380,  380,  380,  380,  380,  380,  380,  380,    0,    0,
    0,  -79,    0,    0,  -65,    0,  380,  380,  380,  380,
  380,  380,  380,  380,  923,    0,    0,    0,  105,    0,
 1131,  879,  879,    0,  137,    0,    0,    0,  879,  137,
    0,
};
const short yyrindex[] = {                                    954,
    0,    0,    0,  -34,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,  185,    0,
    0,    0,    0,    0,    0,  128,  146,    0,    0,    0,
    0,  117,    0,    0,    0,    0,    0,    0,    0,   91,
    0,    0,    6,    0,   25,    0,    0,  -13,  146,    0,
    0,    0,    0,    0,    0,    0,    0,    0,  -14,   52,
    0,  -22,    0,   41,    1,    9,    0,    0,  128,  160,
  368,    0,    0,    0,   17,    0, 1181,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,  -14,    0,    0,  -14,  -14,  -14,
    0,    0,  -34,    0,    0,    0,   77,   79,  104,  112,
  133,  134,  147,   48,    0,    0,    0,    0,    0,    0,
   56, 1035,  993,  665,  741,  781,  826,  872,  917,  950,
  574,  620,  413,  453,  489,  529,    0, 1078,  464,    0,
    0,    0,    0,    0,  136,    0,    0,  118,  -18,    0,
    0,    0,   26,   39,   50,    0,  171,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,  103,    0,    0,  138,    0,  170,  329,  497,  506,
  542,  551,  552,  565,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,   64,    0,    0,    0,    0,   95,
    0,
};
const short yygindex[] = {                                      0,
  420,  -46,   40,    0,    0,    0,    0,    0,    0,    0,
 1331,  -45,  -87,    5,  -36,    0, 1296,    0,    8, 1543,
};
#define YYTABLESIZE 1727
const short yytable[] = {                                      95,
   17,  102,  106,  117,   93,   81,   26,   20,   18,   91,
   89,  105,   90,  117,   92,   43,   20,  146,   52,   44,
   46,   52,   53,   57,  117,   53,  117,   23,  117,   51,
   48,   93,  145,   93,   45,   29,   91,   89,   91,   90,
    1,   92,   69,   92,   28,   48,   43,   10,   40,   43,
   26,   20,  148,   29,  116,   19,   56,  117,   95,   41,
   27,   21,  152,    6,   43,   45,   29,   41,   45,   29,
   52,   23,  176,  153,   53,  154,   43,  155,   51,   40,
   45,   94,   40,   45,   29,   95,   22,   95,   23,   58,
   41,   80,   21,   41,    5,   21,   56,   40,  158,   94,
   54,   48,   11,   59,  197,  198,   57,   49,   41,  159,
   21,  201,   72,   24,   73,   56,   59,   22,   98,   23,
   22,   25,   23,   99,  151,   17,   63,  117,  100,   57,
   54,   54,  118,   18,   54,   22,  103,   23,  105,  191,
   57,   20,   26,   27,   24,  194,   15,   24,  146,   54,
  147,  174,   25,   63,   63,   25,   28,   63,   63,   63,
   63,   63,   24,   63,   11,   12,   13,  175,   51,   73,
   25,   51,   10,   26,   27,   63,   26,   27,   48,   30,
   19,   48,  186,   54,    1,   38,   93,   28,    6,   20,
   28,   26,   27,    4,   93,   57,   73,   73,   94,   73,
   73,   73,   73,   73,   73,   28,   73,   63,   57,   63,
   30,   60,    0,   30,  189,   37,    0,    0,   73,    5,
   33,   11,   12,   13,    0,    0,    0,   11,   30,    0,
    0,   15,    0,   47,   47,   47,   47,   47,   47,   47,
   63,    0,    0,   78,   79,   82,   83,   84,   85,   86,
   87,   88,   73,    0,   24,   25,    0,   94,    0,   17,
   17,   17,   17,   17,   17,   17,   17,   18,   18,   18,
   18,   18,   18,   18,   18,   20,   20,   20,   20,   20,
   20,   20,   20,   73,   94,    0,   94,   17,   17,   17,
   67,   68,    0,    0,  144,   18,   18,   18,   24,   25,
    0,    0,    0,   20,   20,   20,   10,   10,   10,   10,
   10,   10,   10,   10,   19,   19,   19,   19,   19,   19,
   19,   19,    6,    6,    6,    6,    6,    6,    6,    6,
    0,    0,    0,    0,   10,   10,   10,    0,   31,    0,
   37,  120,   19,   19,   19,   33,  101,   54,   54,   54,
    6,    6,    6,    5,    5,    5,    5,    5,    5,    5,
    5,   11,   11,   11,    0,   11,   11,   11,   11,   31,
    0,    0,   31,   63,   63,   63,   54,   84,   54,   54,
    0,    5,    5,    5,    0,    0,    0,   31,    0,   11,
   11,   11,   63,   63,   63,   63,   63,   63,   63,   63,
   63,    0,   63,    0,   84,   84,   63,   84,   84,   84,
   84,   84,   84,    0,   84,    0,   73,   73,   73,   37,
    0,    0,   69,    0,   33,    0,   84,    0,    0,    0,
    0,    0,   30,   31,   32,   73,   73,   73,   73,   73,
   73,   73,   73,   73,    0,   73,    0,   73,   73,   69,
   69,    0,   69,   69,   69,   69,   69,   69,    0,   69,
   84,   34,   70,   35,   36,    0,    0,    0,    0,    0,
    0,   69,    0,   83,    0,    0,    0,    0,  115,    0,
    4,    5,    6,    0,    7,    8,    9,   10,  119,   70,
   70,   84,   70,   70,   70,   70,   70,   70,   71,   70,
    0,    0,    0,   83,   83,   69,   32,   83,   11,   12,
   13,   70,    0,   76,  137,   33,    0,  140,  141,  142,
    0,    0,   83,    0,    0,   71,   71,    0,   71,   71,
   71,   71,   71,   71,    0,   71,   69,   32,   72,    0,
   32,    0,    0,   37,    0,   70,   33,   71,   33,   33,
    0,   34,    0,    0,    0,   32,   83,   30,   31,   32,
   35,   36,    0,    0,   33,   72,   72,    0,   72,   72,
   72,   72,   72,   72,   37,   72,   70,    0,    0,    0,
    0,   71,   34,   67,    0,   34,   34,   72,   35,   36,
    0,   35,   36,    0,   35,   36,    0,    0,    0,    0,
   34,    0,    0,    0,    0,   37,    0,    0,   37,   35,
   36,   67,   71,   67,   67,    0,   67,   67,   67,    0,
    0,   72,    0,   37,   84,   84,   84,    0,    0,   68,
    0,    0,   67,    0,    0,    0,   30,   31,   32,    0,
    0,    0,    0,   84,   84,   84,   84,   84,   84,   84,
   84,   84,   72,   84,    0,   84,   84,   68,    0,   68,
   68,    0,   68,   68,   68,   34,   67,   35,   36,   69,
   69,   69,    0,    0,   74,    0,    0,    0,   68,   50,
   51,   52,   53,   54,   55,   56,    0,    0,   69,   69,
   69,   69,   69,   69,   69,   69,   69,   67,   69,    0,
   69,   69,   74,    0,   74,   74,    0,    0,   74,   70,
   70,   70,   68,    0,    0,    0,    0,    0,    0,    0,
   83,   83,   83,   74,    0,    0,    0,   15,   70,   70,
   70,   70,   70,   70,   70,   70,   70,    0,   70,   83,
   70,   70,    0,   68,    0,   71,   71,   71,    0,   83,
   75,   83,   83,    0,    0,    0,    0,   74,  161,    0,
   30,   31,   32,    0,   71,   71,   71,   71,   71,   71,
   71,   71,   71,    0,   71,    0,   71,   71,   75,    0,
   75,   75,    0,    0,   75,   72,   72,   72,   74,   34,
   76,   35,   36,    0,    0,    0,    0,    0,    0,   75,
    0,    0,    0,    0,   72,   72,   72,   72,   72,   72,
   72,   72,   72,    0,   72,    0,   72,   72,   76,    0,
   76,   76,    0,    0,   76,    0,    0,    0,    0,    0,
   67,   67,   67,   75,    0,   77,    0,    0,    0,   76,
   14,    0,    0,    0,    0,    0,    0,    0,    0,   67,
   67,   67,   67,   67,   67,   67,   67,   67,    0,   67,
    0,   67,   67,   77,   75,   77,   77,    0,    0,   77,
    0,    0,    0,   76,    0,    0,   68,   68,   68,    0,
    0,   78,    0,    0,   77,    0,    0,    0,   15,    0,
    0,    0,    0,    0,    0,   68,   68,   68,   68,   68,
   68,   68,   68,   68,   76,   68,    0,   68,   68,   78,
    0,   78,   78,    0,    0,   78,    0,    0,   77,    0,
    0,   74,   74,   74,    0,    0,   79,    0,    0,    0,
   78,    0,   15,    0,    0,    0,    0,    0,    0,    0,
   74,   74,   74,   74,   74,   74,   74,   74,   74,   77,
   74,    0,   74,   74,   79,    0,   79,   79,    0,   89,
   79,    0,    0,   97,   78,  162,  163,  164,  165,  166,
  167,  168,    0,    0,    0,   79,    4,    5,    6,    0,
    7,    8,    9,   10,    0,  169,  170,   89,    0,   89,
   89,    0,    0,   89,    0,   78,    0,   75,   75,   75,
    0,  171,   80,    0,   11,   12,   13,    0,   89,   79,
    0,    0,    0,    0,    0,    0,   75,   75,   75,   75,
   75,   75,   75,   75,   75,    0,   75,    0,   75,   75,
   80,    0,   80,   80,    0,    0,   80,   76,   76,   76,
   79,    0,   89,    0,   81,    0,    0,  190,    0,    0,
    0,   80,    0,    0,    0,    0,   76,   76,   76,   76,
   76,   76,   76,   76,   76,    0,   76,    0,   76,   76,
    0,    0,    0,   89,   81,   81,   97,    0,   81,    0,
    0,    0,   77,   77,   77,   80,    0,   82,    0,    0,
    0,    0,    0,   81,    0,    0,    0,    0,    0,    0,
    0,   77,   77,   77,   77,   77,   77,   77,   77,   77,
    0,   77,    0,   77,   77,    0,   80,   82,   82,    0,
    0,   82,    0,    0,    0,    0,    0,   81,   78,   78,
   78,    0,    0,    0,    0,    0,   82,    4,    5,    6,
    0,    7,    8,    9,   10,    0,    0,   78,   78,   78,
   78,   78,   78,   78,   78,   78,    0,   78,   81,   78,
   78,    0,    0,    0,    0,   11,   12,   13,    0,    0,
   82,    0,    0,   79,   79,   79,    0,    0,    0,    0,
    0,    4,    5,    6,    0,    7,    8,    9,   10,    0,
   55,    0,   79,   79,   79,   79,   79,   79,   79,   79,
   79,    0,   79,    0,   79,   79,   89,   89,   89,   11,
   12,   13,   97,   97,   97,    0,   97,   97,   97,   97,
   55,   55,    0,    0,   55,   89,   89,   89,   89,   89,
   89,   89,   89,   89,    0,   89,    0,   89,   89,   55,
   97,   97,   97,    0,    0,  143,    0,    0,    0,   80,
   80,   80,   93,   81,    0,  196,    0,   91,   89,    0,
   90,    0,   92,    0,    0,    0,    0,    0,   80,   80,
    0,    0,    0,   55,    0,    0,    0,    0,   80,    0,
   80,   80,    0,    0,    0,    0,    0,    0,    0,   93,
   81,   81,   81,   81,   91,   89,    0,   90,    0,   92,
    0,    0,    0,    0,    0,   39,   95,   93,   81,    0,
   81,   81,   91,   89,    0,   90,    0,   92,   60,    0,
   81,   93,   81,   81,    0,    0,   91,   89,    0,   90,
    0,   92,   74,    0,   82,   82,   82,    0,    0,   80,
    0,    0,    0,   95,   47,  107,  108,  109,  110,  111,
  112,  113,    0,   82,   82,    0,    0,    0,   65,   66,
    0,   95,    0,   82,    0,   82,   82,    0,   75,    0,
    0,    0,    0,    0,    0,   95,   80,    0,    0,    4,
    5,    6,    0,    7,    8,    9,   10,    0,  114,    4,
    5,    6,    0,    7,    8,    9,   10,    4,    5,    6,
    0,    7,    8,    9,   10,    0,  121,   11,   12,   13,
    0,    0,  149,    0,    0,    0,    0,   11,   12,   13,
    0,    0,    0,    0,    0,   11,   12,   13,    0,    0,
    0,    0,    0,    0,    0,    0,    0,   55,   55,   55,
    0,    0,    0,    0,    0,    0,    0,    0,  150,    0,
    0,    0,    0,    0,    0,    0,  177,  178,  179,  180,
  181,  182,  183,  184,    0,    0,   55,    0,   55,   55,
    0,    0,    0,    0,    0,    0,    0,  160,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,   79,   82,   83,   84,   85,   86,   87,   88,
    0,  185,    0,    0,    0,   94,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,  192,  193,    0,
  195,    0,    0,    0,  199,    0,  200,    0,    0,    0,
   82,   83,   84,   85,   86,   87,   88,    0,    0,    0,
    0,    0,   94,    0,    0,    0,    0,    0,   82,   83,
   84,   85,   86,   87,   88,    0,    0,    0,    0,    0,
   94,    0,   82,   83,   84,   85,   86,   87,   88,   64,
    0,    0,    0,    0,   94,   70,   71,    0,    0,    0,
    0,   77,    0,    0,    0,    0,    0,    0,    0,    0,
    0,   64,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,   77,    0,   77,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,   77,    0,    0,    0,
    0,    0,  122,  123,  124,  125,  126,  127,  128,  129,
  130,  131,  132,  133,  134,  135,  136,    0,  138,  139,
    0,    0,    0,    0,    0,    0,    0,    0,    0,   77,
   77,   77,   77,   77,   77,   77,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,  157,   64,    0,    0,
    0,   77,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,   77,
   77,   77,   77,   77,   77,   77,   77,
};
const short yycheck[] = {                                      91,
    0,   47,   49,   44,   37,   38,   40,    3,    0,   42,
   43,   48,   45,   44,   47,   10,    0,   44,   41,   12,
   13,   44,   41,   19,   44,   44,   44,   61,   44,   44,
   44,   37,   59,   37,   10,   10,   42,   43,   42,   45,
  256,   47,   40,   47,   10,   59,   41,    0,   10,   44,
   40,   47,   93,   10,   41,    0,   91,   44,   91,   10,
   40,   10,   93,    0,   59,   41,   41,  259,   44,   44,
   93,   61,  160,   93,   93,   93,  259,   93,   93,   41,
  259,   41,   44,   59,   59,   91,   10,   91,   10,   10,
   41,  124,   41,   44,    0,   44,   91,   59,  145,   59,
   10,   40,    0,   91,  192,  193,  102,   40,   59,  146,
   59,  199,  259,   10,  259,   91,   91,   41,   91,   41,
   44,   10,   44,   91,   41,  125,   10,   44,   91,   91,
   40,   41,   41,  125,   44,   59,  259,   59,  175,  185,
   91,  125,   10,   10,   41,   41,   10,   44,   44,   59,
   41,   41,   41,   37,   38,   44,   10,   41,   42,   43,
   44,   45,   59,   47,  287,  288,  289,   59,   41,   10,
   59,   44,  125,   41,   41,   59,   44,   44,   41,   10,
  125,   44,  262,   93,    0,   10,   41,   41,  125,  185,
   44,   59,   59,  259,   59,  191,   37,   38,  290,   40,
   41,   42,   43,   44,   45,   59,   47,   91,   91,   93,
   41,   41,   -1,   44,  175,   40,   -1,   -1,   59,  125,
   45,  287,  288,  289,   -1,   -1,   -1,  125,   59,   -1,
   -1,   10,   -1,  268,  269,  270,  271,  272,  273,  274,
  124,   -1,   -1,  276,  277,  278,  279,  280,  281,  282,
  283,  284,   93,   -1,  288,  289,   -1,  290,   -1,  259,
  260,  261,  262,  263,  264,  265,  266,  259,  260,  261,
  262,  263,  264,  265,  266,  259,  260,  261,  262,  263,
  264,  265,  266,  124,  290,   -1,  290,  287,  288,  289,
  288,  289,   -1,   -1,  284,  287,  288,  289,  288,  289,
   -1,   -1,   -1,  287,  288,  289,  259,  260,  261,  262,
  263,  264,  265,  266,  259,  260,  261,  262,  263,  264,
  265,  266,  259,  260,  261,  262,  263,  264,  265,  266,
   -1,   -1,   -1,   -1,  287,  288,  289,   -1,   10,   -1,
   40,   41,  287,  288,  289,   45,  125,  257,  258,  259,
  287,  288,  289,  259,  260,  261,  262,  263,  264,  265,
  266,  259,  260,  261,   -1,  263,  264,  265,  266,   41,
   -1,   -1,   44,  257,  258,  259,  286,   10,  288,  289,
   -1,  287,  288,  289,   -1,   -1,   -1,   59,   -1,  287,
  288,  289,  276,  277,  278,  279,  280,  281,  282,  283,
  284,   -1,  286,   -1,   37,   38,  290,   40,   41,   42,
   43,   44,   45,   -1,   47,   -1,  257,  258,  259,   40,
   -1,   -1,   10,   -1,   45,   -1,   59,   -1,   -1,   -1,
   -1,   -1,  257,  258,  259,  276,  277,  278,  279,  280,
  281,  282,  283,  284,   -1,  286,   -1,  288,  289,   37,
   38,   -1,   40,   41,   42,   43,   44,   45,   -1,   47,
   93,  286,   10,  288,  289,   -1,   -1,   -1,   -1,   -1,
   -1,   59,   -1,   10,   -1,   -1,   -1,   -1,   59,   -1,
  259,  260,  261,   -1,  263,  264,  265,  266,   69,   37,
   38,  124,   40,   41,   42,   43,   44,   45,   10,   47,
   -1,   -1,   -1,   40,   41,   93,   10,   44,  287,  288,
  289,   59,   -1,   10,   95,   10,   -1,   98,   99,  100,
   -1,   -1,   59,   -1,   -1,   37,   38,   -1,   40,   41,
   42,   43,   44,   45,   -1,   47,  124,   41,   10,   -1,
   44,   -1,   -1,   40,   -1,   93,   41,   59,   45,   44,
   -1,   10,   -1,   -1,   -1,   59,   93,  257,  258,  259,
   10,   10,   -1,   -1,   59,   37,   38,   -1,   40,   41,
   42,   43,   44,   45,   10,   47,  124,   -1,   -1,   -1,
   -1,   93,   41,   10,   -1,   44,  286,   59,  288,  289,
   -1,   41,   41,   -1,   44,   44,   -1,   -1,   -1,   -1,
   59,   -1,   -1,   -1,   -1,   41,   -1,   -1,   44,   59,
   59,   38,  124,   40,   41,   -1,   43,   44,   45,   -1,
   -1,   93,   -1,   59,  257,  258,  259,   -1,   -1,   10,
   -1,   -1,   59,   -1,   -1,   -1,  257,  258,  259,   -1,
   -1,   -1,   -1,  276,  277,  278,  279,  280,  281,  282,
  283,  284,  124,  286,   -1,  288,  289,   38,   -1,   40,
   41,   -1,   43,   44,   45,  286,   93,  288,  289,  257,
  258,  259,   -1,   -1,   10,   -1,   -1,   -1,   59,  268,
  269,  270,  271,  272,  273,  274,   -1,   -1,  276,  277,
  278,  279,  280,  281,  282,  283,  284,  124,  286,   -1,
  288,  289,   38,   -1,   40,   41,   -1,   -1,   44,  257,
  258,  259,   93,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
  257,  258,  259,   59,   -1,   -1,   -1,   10,  276,  277,
  278,  279,  280,  281,  282,  283,  284,   -1,  286,  276,
  288,  289,   -1,  124,   -1,  257,  258,  259,   -1,  286,
   10,  288,  289,   -1,   -1,   -1,   -1,   93,   61,   -1,
  257,  258,  259,   -1,  276,  277,  278,  279,  280,  281,
  282,  283,  284,   -1,  286,   -1,  288,  289,   38,   -1,
   40,   41,   -1,   -1,   44,  257,  258,  259,  124,  286,
   10,  288,  289,   -1,   -1,   -1,   -1,   -1,   -1,   59,
   -1,   -1,   -1,   -1,  276,  277,  278,  279,  280,  281,
  282,  283,  284,   -1,  286,   -1,  288,  289,   38,   -1,
   40,   41,   -1,   -1,   44,   -1,   -1,   -1,   -1,   -1,
  257,  258,  259,   93,   -1,   10,   -1,   -1,   -1,   59,
  123,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,  276,
  277,  278,  279,  280,  281,  282,  283,  284,   -1,  286,
   -1,  288,  289,   38,  124,   40,   41,   -1,   -1,   44,
   -1,   -1,   -1,   93,   -1,   -1,  257,  258,  259,   -1,
   -1,   10,   -1,   -1,   59,   -1,   -1,   -1,   10,   -1,
   -1,   -1,   -1,   -1,   -1,  276,  277,  278,  279,  280,
  281,  282,  283,  284,  124,  286,   -1,  288,  289,   38,
   -1,   40,   41,   -1,   -1,   44,   -1,   -1,   93,   -1,
   -1,  257,  258,  259,   -1,   -1,   10,   -1,   -1,   -1,
   59,   -1,   10,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
  276,  277,  278,  279,  280,  281,  282,  283,  284,  124,
  286,   -1,  288,  289,   38,   -1,   40,   41,   -1,   10,
   44,   -1,   -1,   10,   93,  268,  269,  270,  271,  272,
  273,  274,   -1,   -1,   -1,   59,  259,  260,  261,   -1,
  263,  264,  265,  266,   -1,  288,  289,   38,   -1,   40,
   41,   -1,   -1,   44,   -1,  124,   -1,  257,  258,  259,
   -1,  123,   10,   -1,  287,  288,  289,   -1,   59,   93,
   -1,   -1,   -1,   -1,   -1,   -1,  276,  277,  278,  279,
  280,  281,  282,  283,  284,   -1,  286,   -1,  288,  289,
   38,   -1,   40,   41,   -1,   -1,   44,  257,  258,  259,
  124,   -1,   93,   -1,   10,   -1,   -1,  125,   -1,   -1,
   -1,   59,   -1,   -1,   -1,   -1,  276,  277,  278,  279,
  280,  281,  282,  283,  284,   -1,  286,   -1,  288,  289,
   -1,   -1,   -1,  124,   40,   41,  123,   -1,   44,   -1,
   -1,   -1,  257,  258,  259,   93,   -1,   10,   -1,   -1,
   -1,   -1,   -1,   59,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,  276,  277,  278,  279,  280,  281,  282,  283,  284,
   -1,  286,   -1,  288,  289,   -1,  124,   40,   41,   -1,
   -1,   44,   -1,   -1,   -1,   -1,   -1,   93,  257,  258,
  259,   -1,   -1,   -1,   -1,   -1,   59,  259,  260,  261,
   -1,  263,  264,  265,  266,   -1,   -1,  276,  277,  278,
  279,  280,  281,  282,  283,  284,   -1,  286,  124,  288,
  289,   -1,   -1,   -1,   -1,  287,  288,  289,   -1,   -1,
   93,   -1,   -1,  257,  258,  259,   -1,   -1,   -1,   -1,
   -1,  259,  260,  261,   -1,  263,  264,  265,  266,   -1,
   10,   -1,  276,  277,  278,  279,  280,  281,  282,  283,
  284,   -1,  286,   -1,  288,  289,  257,  258,  259,  287,
  288,  289,  259,  260,  261,   -1,  263,  264,  265,  266,
   40,   41,   -1,   -1,   44,  276,  277,  278,  279,  280,
  281,  282,  283,  284,   -1,  286,   -1,  288,  289,   59,
  287,  288,  289,   -1,   -1,  125,   -1,   -1,   -1,  257,
  258,  259,   37,   38,   -1,  125,   -1,   42,   43,   -1,
   45,   -1,   47,   -1,   -1,   -1,   -1,   -1,  276,  277,
   -1,   -1,   -1,   93,   -1,   -1,   -1,   -1,  286,   -1,
  288,  289,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   37,
   38,  257,  258,  259,   42,   43,   -1,   45,   -1,   47,
   -1,   -1,   -1,   -1,   -1,   10,   91,   37,   38,   -1,
  276,  277,   42,   43,   -1,   45,   -1,   47,   23,   -1,
  286,   37,  288,  289,   -1,   -1,   42,   43,   -1,   45,
   -1,   47,   37,   -1,  257,  258,  259,   -1,   -1,  124,
   -1,   -1,   -1,   91,   14,   50,   51,   52,   53,   54,
   55,   56,   -1,  276,  277,   -1,   -1,   -1,   28,   29,
   -1,   91,   -1,  286,   -1,  288,  289,   -1,   38,   -1,
   -1,   -1,   -1,   -1,   -1,   91,  124,   -1,   -1,  259,
  260,  261,   -1,  263,  264,  265,  266,   -1,   58,  259,
  260,  261,   -1,  263,  264,  265,  266,  259,  260,  261,
   -1,  263,  264,  265,  266,   -1,   76,  287,  288,  289,
   -1,   -1,  117,   -1,   -1,   -1,   -1,  287,  288,  289,
   -1,   -1,   -1,   -1,   -1,  287,  288,  289,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,  257,  258,  259,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,  118,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,  161,  162,  163,  164,
  165,  166,  167,  168,   -1,   -1,  286,   -1,  288,  289,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,  147,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,  277,  278,  279,  280,  281,  282,  283,  284,
   -1,  171,   -1,   -1,   -1,  290,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,  187,  188,   -1,
  190,   -1,   -1,   -1,  194,   -1,  196,   -1,   -1,   -1,
  278,  279,  280,  281,  282,  283,  284,   -1,   -1,   -1,
   -1,   -1,  290,   -1,   -1,   -1,   -1,   -1,  278,  279,
  280,  281,  282,  283,  284,   -1,   -1,   -1,   -1,   -1,
  290,   -1,  278,  279,  280,  281,  282,  283,  284,   27,
   -1,   -1,   -1,   -1,  290,   33,   34,   -1,   -1,   -1,
   -1,   39,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   49,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   60,   -1,   62,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   74,   -1,   -1,   -1,
   -1,   -1,   80,   81,   82,   83,   84,   85,   86,   87,
   88,   89,   90,   91,   92,   93,   94,   -1,   96,   97,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,  107,
  108,  109,  110,  111,  112,  113,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,  144,  145,   -1,   -1,
   -1,  149,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,  177,
  178,  179,  180,  181,  182,  183,  184,
};
#define YYFINAL 2
#ifndef YYDEBUG
#define YYDEBUG 0
#endif
#define YYMAXTOKEN 290
#if YYDEBUG
const char * const yyname[] = {
"end-of-file",0,0,0,0,0,0,0,0,0,"'\\n'",0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,"'%'","'&'",0,"'('","')'","'*'","'+'","','","'-'",0,"'/'",0,0,0,0,0,
0,0,0,0,0,0,"';'",0,"'='",0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
0,0,"'['",0,"']'",0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
"'{'","'|'","'}'",0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,"NUMBER","STRING","SYMBOL","IF","WHILE",
"ELSE","FOR","BREAK","CONTINUE","RETURN","IF_NO_ELSE","ADDEQ","SUBEQ","MULEQ",
"DIVEQ","MODEQ","ANDEQ","OREQ","CONCAT","OR","AND","GT","GE","LT","LE","EQ",
"NE","IN","UNARY_MINUS","NOT","DELETE","INCR","DECR","POW",
};
const char * const yyrule[] = {
"$accept : program",
"program : blank stmts",
"program : blank '{' blank stmts '}'",
"program : blank '{' blank '}'",
"program : error",
"block : '{' blank stmts '}' blank",
"block : '{' blank '}' blank",
"block : stmt",
"stmts : stmt",
"stmts : stmts stmt",
"stmt : simpstmt '\\n' blank",
"stmt : IF '(' cond ')' blank block",
"stmt : IF '(' cond ')' blank block else blank block",
"stmt : while '(' cond ')' blank block",
"stmt : for '(' comastmts ';' cond ';' comastmts ')' blank block",
"$$1 :",
"stmt : for '(' SYMBOL IN arrayexpr ')' $$1 blank block",
"stmt : BREAK '\\n' blank",
"stmt : CONTINUE '\\n' blank",
"stmt : RETURN expr '\\n' blank",
"stmt : RETURN '\\n' blank",
"simpstmt : SYMBOL '=' expr",
"simpstmt : evalsym ADDEQ expr",
"simpstmt : evalsym SUBEQ expr",
"simpstmt : evalsym MULEQ expr",
"simpstmt : evalsym DIVEQ expr",
"simpstmt : evalsym MODEQ expr",
"simpstmt : evalsym ANDEQ expr",
"simpstmt : evalsym OREQ expr",
"simpstmt : DELETE arraylv '[' arglist ']'",
"simpstmt : initarraylv '[' arglist ']' '=' expr",
"simpstmt : initarraylv '[' arglist ']' ADDEQ expr",
"simpstmt : initarraylv '[' arglist ']' SUBEQ expr",
"simpstmt : initarraylv '[' arglist ']' MULEQ expr",
"simpstmt : initarraylv '[' arglist ']' DIVEQ expr",
"simpstmt : initarraylv '[' arglist ']' MODEQ expr",
"simpstmt : initarraylv '[' arglist ']' ANDEQ expr",
"simpstmt : initarraylv '[' arglist ']' OREQ expr",
"simpstmt : initarraylv '[' arglist ']' INCR",
"simpstmt : initarraylv '[' arglist ']' DECR",
"simpstmt : INCR initarraylv '[' arglist ']'",
"simpstmt : DECR initarraylv '[' arglist ']'",
"simpstmt : SYMBOL '(' arglist ')'",
"simpstmt : INCR SYMBOL",
"simpstmt : SYMBOL INCR",
"simpstmt : DECR SYMBOL",
"simpstmt : SYMBOL DECR",
"evalsym : SYMBOL",
"comastmts :",
"comastmts : simpstmt",
"comastmts : comastmts ',' simpstmt",
"arglist :",
"arglist : expr",
"arglist : arglist ',' expr",
"expr : numexpr",
"expr : expr numexpr",
"initarraylv : SYMBOL",
"initarraylv : initarraylv '[' arglist ']'",
"arraylv : SYMBOL",
"arraylv : arraylv '[' arglist ']'",
"arrayexpr : numexpr",
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
"cond :",
"cond : numexpr",
"and : AND",
"or : OR",
"blank :",
"blank : blank '\\n'",
};
#endif
#if YYDEBUG
#include <stdio.h>
#endif
#ifdef YYSTACKSIZE
#undef YYMAXDEPTH
#define YYMAXDEPTH YYSTACKSIZE
#else
#ifdef YYMAXDEPTH
#define YYSTACKSIZE YYMAXDEPTH
#else
#define YYSTACKSIZE 10000
#define YYMAXDEPTH 10000
#endif
#endif
#define YYINITSTACKSIZE 200
int yydebug;
int yynerrs;
int yyerrflag;
int yychar;
short *yyssp;
YYSTYPE *yyvsp;
YYSTYPE yyval;
YYSTYPE yylval;
short *yyss;
short *yysslim;
YYSTYPE *yyvs;
int yystacksize;
#line 424 "parse.y"
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
    case '>':   return follow('=', GE, GT);
    case '<':   return follow('=', LE, LT);
    case '=':   return follow('=', EQ, '=');
    case '!':   return follow('=', NE, NOT);
    case '+':   return follow2('+', INCR, '=', ADDEQ, '+');
    case '-':   return follow2('-', DECR, '=', SUBEQ, '-');
    case '|':   return follow2('|', OR, '=', OREQ, '|');
    case '&':   return follow2('&', AND, '=', ANDEQ, '&');
    case '*':   return follow2('*', POW, '=', MULEQ, '*');
    case '/':   return follow('=', DIVEQ, '/');
    case '%':   return follow('=', MODEQ, '%');
    case '^':   return POW;
    default:    return *(InPtr-1);
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
#line 965 "y.tab.c"
/* allocate initial stack or double stack size, up to YYMAXDEPTH */
static int yygrowstack()
{
    int newsize, i;
    short *newss;
    YYSTYPE *newvs;

    if ((newsize = yystacksize) == 0)
        newsize = YYINITSTACKSIZE;
    else if (newsize >= YYMAXDEPTH)
        return -1;
    else if ((newsize *= 2) > YYMAXDEPTH)
        newsize = YYMAXDEPTH;
    i = yyssp - yyss;
    newss = yyss ? (short *)realloc(yyss, newsize * sizeof *newss) :
      (short *)malloc(newsize * sizeof *newss);
    if (newss == NULL)
        return -1;
    yyss = newss;
    yyssp = newss + i;
    newvs = yyvs ? (YYSTYPE *)realloc(yyvs, newsize * sizeof *newvs) :
      (YYSTYPE *)malloc(newsize * sizeof *newvs);
    if (newvs == NULL)
        return -1;
    yyvs = newvs;
    yyvsp = newvs + i;
    yystacksize = newsize;
    yysslim = yyss + newsize - 1;
    return 0;
}

#define YYABORT goto yyabort
#define YYREJECT goto yyabort
#define YYACCEPT goto yyaccept
#define YYERROR goto yyerrlab

#ifndef YYPARSE_PARAM
#if defined(__cplusplus) || __STDC__
#define YYPARSE_PARAM_ARG void
#define YYPARSE_PARAM_DECL
#else	/* ! ANSI-C/C++ */
#define YYPARSE_PARAM_ARG
#define YYPARSE_PARAM_DECL
#endif	/* ANSI-C/C++ */
#else	/* YYPARSE_PARAM */
#ifndef YYPARSE_PARAM_TYPE
#define YYPARSE_PARAM_TYPE void *
#endif
#if defined(__cplusplus) || __STDC__
#define YYPARSE_PARAM_ARG YYPARSE_PARAM_TYPE YYPARSE_PARAM
#define YYPARSE_PARAM_DECL
#else	/* ! ANSI-C/C++ */
#define YYPARSE_PARAM_ARG YYPARSE_PARAM
#define YYPARSE_PARAM_DECL YYPARSE_PARAM_TYPE YYPARSE_PARAM;
#endif	/* ANSI-C/C++ */
#endif	/* ! YYPARSE_PARAM */

int
yyparse (YYPARSE_PARAM_ARG)
    YYPARSE_PARAM_DECL
{
    register int yym, yyn, yystate;
#if YYDEBUG
    register const char *yys;

    if ((yys = getenv("YYDEBUG")))
    {
        yyn = *yys;
        if (yyn >= '0' && yyn <= '9')
            yydebug = yyn - '0';
    }
#endif

    yynerrs = 0;
    yyerrflag = 0;
    yychar = (-1);

    if (yyss == NULL && yygrowstack()) goto yyoverflow;
    yyssp = yyss;
    yyvsp = yyvs;
    *yyssp = yystate = 0;

yyloop:
    if ((yyn = yydefred[yystate])) goto yyreduce;
    if (yychar < 0)
    {
        if ((yychar = yylex()) < 0) yychar = 0;
#if YYDEBUG
        if (yydebug)
        {
            yys = 0;
            if (yychar <= YYMAXTOKEN) yys = yyname[yychar];
            if (!yys) yys = "illegal-symbol";
            printf("%sdebug: state %d, reading %d (%s)\n",
                    YYPREFIX, yystate, yychar, yys);
        }
#endif
    }
    if ((yyn = yysindex[yystate]) && (yyn += yychar) >= 0 &&
            yyn <= YYTABLESIZE && yycheck[yyn] == yychar)
    {
#if YYDEBUG
        if (yydebug)
            printf("%sdebug: state %d, shifting to state %d\n",
                    YYPREFIX, yystate, yytable[yyn]);
#endif
        if (yyssp >= yysslim && yygrowstack())
        {
            goto yyoverflow;
        }
        *++yyssp = yystate = yytable[yyn];
        *++yyvsp = yylval;
        yychar = (-1);
        if (yyerrflag > 0)  --yyerrflag;
        goto yyloop;
    }
    if ((yyn = yyrindex[yystate]) && (yyn += yychar) >= 0 &&
            yyn <= YYTABLESIZE && yycheck[yyn] == yychar)
    {
        yyn = yytable[yyn];
        goto yyreduce;
    }
    if (yyerrflag) goto yyinrecovery;
#if defined(lint) || defined(__GNUC__)
    goto yynewerror;
#endif
yynewerror:
    yyerror("syntax error");
#if defined(lint) || defined(__GNUC__)
    goto yyerrlab;
#endif
yyerrlab:
    ++yynerrs;
yyinrecovery:
    if (yyerrflag < 3)
    {
        yyerrflag = 3;
        for (;;)
        {
            if ((yyn = yysindex[*yyssp]) && (yyn += YYERRCODE) >= 0 &&
                    yyn <= YYTABLESIZE && yycheck[yyn] == YYERRCODE)
            {
#if YYDEBUG
                if (yydebug)
                    printf("%sdebug: state %d, error recovery shifting\
 to state %d\n", YYPREFIX, *yyssp, yytable[yyn]);
#endif
                if (yyssp >= yysslim && yygrowstack())
                {
                    goto yyoverflow;
                }
                *++yyssp = yystate = yytable[yyn];
                *++yyvsp = yylval;
                goto yyloop;
            }
            else
            {
#if YYDEBUG
                if (yydebug)
                    printf("%sdebug: error recovery discarding state %d\n",
                            YYPREFIX, *yyssp);
#endif
                if (yyssp <= yyss) goto yyabort;
                --yyssp;
                --yyvsp;
            }
        }
    }
    else
    {
        if (yychar == 0) goto yyabort;
#if YYDEBUG
        if (yydebug)
        {
            yys = 0;
            if (yychar <= YYMAXTOKEN) yys = yyname[yychar];
            if (!yys) yys = "illegal-symbol";
            printf("%sdebug: state %d, error recovery discards token %d (%s)\n",
                    YYPREFIX, yystate, yychar, yys);
        }
#endif
        yychar = (-1);
        goto yyloop;
    }
yyreduce:
#if YYDEBUG
    if (yydebug)
        printf("%sdebug: state %d, reducing by rule %d (%s)\n",
                YYPREFIX, yystate, yyn, yyrule[yyn]);
#endif
    yym = yylen[yyn];
    yyval = yyvsp[1-yym];
    switch (yyn)
    {
case 1:
#line 81 "parse.y"
{
                ADD_OP(OP_RETURN_NO_VAL); return 0;
            }
break;
case 2:
#line 84 "parse.y"
{
                ADD_OP(OP_RETURN_NO_VAL); return 0;
            }
break;
case 3:
#line 87 "parse.y"
{
                ADD_OP(OP_RETURN_NO_VAL); return 0;
            }
break;
case 4:
#line 90 "parse.y"
{
                return 1;
            }
break;
case 11:
#line 102 "parse.y"
{
                SET_BR_OFF(yyvsp[-3].inst, GetPC());
            }
break;
case 12:
#line 105 "parse.y"
{
                SET_BR_OFF(yyvsp[-6].inst, (yyvsp[-2].inst+1)); SET_BR_OFF(yyvsp[-2].inst, GetPC());
            }
break;
case 13:
#line 108 "parse.y"
{
                ADD_OP(OP_BRANCH); ADD_BR_OFF(yyvsp[-5].inst);
                SET_BR_OFF(yyvsp[-3].inst, GetPC()); FillLoopAddrs(GetPC(), yyvsp[-5].inst);
            }
break;
case 14:
#line 112 "parse.y"
{
                FillLoopAddrs(GetPC()+2+(yyvsp[-3].inst-(yyvsp[-5].inst+1)), GetPC());
                SwapCode(yyvsp[-5].inst+1, yyvsp[-3].inst, GetPC());
                ADD_OP(OP_BRANCH); ADD_BR_OFF(yyvsp[-7].inst); SET_BR_OFF(yyvsp[-5].inst, GetPC());
            }
break;
case 15:
#line 117 "parse.y"
{
                Symbol *iterSym = InstallIteratorSymbol();
                ADD_OP(OP_BEGIN_ARRAY_ITER); ADD_SYM(iterSym);
                ADD_OP(OP_ARRAY_ITER); ADD_SYM(yyvsp[-3].sym); ADD_SYM(iterSym); ADD_BR_OFF(0);
            }
break;
case 16:
#line 122 "parse.y"
{
                    ADD_OP(OP_BRANCH); ADD_BR_OFF(yyvsp[-4].inst+2);
                    SET_BR_OFF(yyvsp[-4].inst+5, GetPC());
                    FillLoopAddrs(GetPC(), yyvsp[-4].inst+2);
            }
break;
case 17:
#line 127 "parse.y"
{
                ADD_OP(OP_BRANCH); ADD_BR_OFF(0);
                if (AddBreakAddr(GetPC()-1)) {
                    yyerror("break outside loop"); YYERROR;
                }
            }
break;
case 18:
#line 133 "parse.y"
{
                ADD_OP(OP_BRANCH); ADD_BR_OFF(0);
                if (AddContinueAddr(GetPC()-1)) {
                    yyerror("continue outside loop"); YYERROR;
                }
            }
break;
case 19:
#line 139 "parse.y"
{
                ADD_OP(OP_RETURN);
            }
break;
case 20:
#line 142 "parse.y"
{
                ADD_OP(OP_RETURN_NO_VAL);
            }
break;
case 21:
#line 146 "parse.y"
{
                ADD_OP(OP_ASSIGN); ADD_SYM(yyvsp[-2].sym);
            }
break;
case 22:
#line 149 "parse.y"
{
                ADD_OP(OP_ADD); ADD_OP(OP_ASSIGN); ADD_SYM(yyvsp[-2].sym);
            }
break;
case 23:
#line 152 "parse.y"
{
                ADD_OP(OP_SUB); ADD_OP(OP_ASSIGN); ADD_SYM(yyvsp[-2].sym);
            }
break;
case 24:
#line 155 "parse.y"
{
                ADD_OP(OP_MUL); ADD_OP(OP_ASSIGN); ADD_SYM(yyvsp[-2].sym);
            }
break;
case 25:
#line 158 "parse.y"
{
                ADD_OP(OP_DIV); ADD_OP(OP_ASSIGN); ADD_SYM(yyvsp[-2].sym);
            }
break;
case 26:
#line 161 "parse.y"
{
                ADD_OP(OP_MOD); ADD_OP(OP_ASSIGN); ADD_SYM(yyvsp[-2].sym);
            }
break;
case 27:
#line 164 "parse.y"
{
                ADD_OP(OP_BIT_AND); ADD_OP(OP_ASSIGN); ADD_SYM(yyvsp[-2].sym);
            }
break;
case 28:
#line 167 "parse.y"
{
                ADD_OP(OP_BIT_OR); ADD_OP(OP_ASSIGN); ADD_SYM(yyvsp[-2].sym);
            }
break;
case 29:
#line 170 "parse.y"
{
                ADD_OP(OP_ARRAY_DELETE); ADD_IMMED((void *)yyvsp[-1].nArgs);
            }
break;
case 30:
#line 173 "parse.y"
{
                ADD_OP(OP_ARRAY_ASSIGN); ADD_IMMED((void *)yyvsp[-3].nArgs);
            }
break;
case 31:
#line 176 "parse.y"
{
                ADD_OP(OP_ARRAY_REF_ASSIGN_SETUP); ADD_IMMED((void *)1); ADD_IMMED((void *)yyvsp[-3].nArgs);
                ADD_OP(OP_ADD);
                ADD_OP(OP_ARRAY_ASSIGN); ADD_IMMED((void *)yyvsp[-3].nArgs);
            }
break;
case 32:
#line 181 "parse.y"
{
                ADD_OP(OP_ARRAY_REF_ASSIGN_SETUP); ADD_IMMED((void *)1); ADD_IMMED((void *)yyvsp[-3].nArgs);
                ADD_OP(OP_SUB);
                ADD_OP(OP_ARRAY_ASSIGN); ADD_IMMED((void *)yyvsp[-3].nArgs);
            }
break;
case 33:
#line 186 "parse.y"
{
                ADD_OP(OP_ARRAY_REF_ASSIGN_SETUP); ADD_IMMED((void *)1); ADD_IMMED((void *)yyvsp[-3].nArgs);
                ADD_OP(OP_MUL);
                ADD_OP(OP_ARRAY_ASSIGN); ADD_IMMED((void *)yyvsp[-3].nArgs);
            }
break;
case 34:
#line 191 "parse.y"
{
                ADD_OP(OP_ARRAY_REF_ASSIGN_SETUP); ADD_IMMED((void *)1); ADD_IMMED((void *)yyvsp[-3].nArgs);
                ADD_OP(OP_DIV);
                ADD_OP(OP_ARRAY_ASSIGN); ADD_IMMED((void *)yyvsp[-3].nArgs);
            }
break;
case 35:
#line 196 "parse.y"
{
                ADD_OP(OP_ARRAY_REF_ASSIGN_SETUP); ADD_IMMED((void *)1); ADD_IMMED((void *)yyvsp[-3].nArgs);
                ADD_OP(OP_MOD);
                ADD_OP(OP_ARRAY_ASSIGN); ADD_IMMED((void *)yyvsp[-3].nArgs);
            }
break;
case 36:
#line 201 "parse.y"
{
                ADD_OP(OP_ARRAY_REF_ASSIGN_SETUP); ADD_IMMED((void *)1); ADD_IMMED((void *)yyvsp[-3].nArgs);
                ADD_OP(OP_BIT_AND);
                ADD_OP(OP_ARRAY_ASSIGN); ADD_IMMED((void *)yyvsp[-3].nArgs);
            }
break;
case 37:
#line 206 "parse.y"
{
                ADD_OP(OP_ARRAY_REF_ASSIGN_SETUP); ADD_IMMED((void *)1); ADD_IMMED((void *)yyvsp[-3].nArgs);
                ADD_OP(OP_BIT_OR);
                ADD_OP(OP_ARRAY_ASSIGN); ADD_IMMED((void *)yyvsp[-3].nArgs);
            }
break;
case 38:
#line 211 "parse.y"
{
                ADD_OP(OP_ARRAY_REF_ASSIGN_SETUP); ADD_IMMED((void *)0); ADD_IMMED((void *)yyvsp[-2].nArgs);
                ADD_OP(OP_INCR);
                ADD_OP(OP_ARRAY_ASSIGN); ADD_IMMED((void *)yyvsp[-2].nArgs);
            }
break;
case 39:
#line 216 "parse.y"
{
                ADD_OP(OP_ARRAY_REF_ASSIGN_SETUP); ADD_IMMED((void *)0); ADD_IMMED((void *)yyvsp[-2].nArgs);
                ADD_OP(OP_DECR);
                ADD_OP(OP_ARRAY_ASSIGN); ADD_IMMED((void *)yyvsp[-2].nArgs);
            }
break;
case 40:
#line 221 "parse.y"
{
                ADD_OP(OP_ARRAY_REF_ASSIGN_SETUP); ADD_IMMED((void *)0); ADD_IMMED((void *)yyvsp[-1].nArgs);
                ADD_OP(OP_INCR);
                ADD_OP(OP_ARRAY_ASSIGN); ADD_IMMED((void *)yyvsp[-1].nArgs);
            }
break;
case 41:
#line 226 "parse.y"
{
                ADD_OP(OP_ARRAY_REF_ASSIGN_SETUP); ADD_IMMED((void *)0); ADD_IMMED((void *)yyvsp[-1].nArgs);
                ADD_OP(OP_DECR);
                ADD_OP(OP_ARRAY_ASSIGN); ADD_IMMED((void *)yyvsp[-1].nArgs);
            }
break;
case 42:
#line 231 "parse.y"
{
                ADD_OP(OP_SUBR_CALL);
                ADD_SYM(PromoteToGlobal(yyvsp[-3].sym)); ADD_IMMED((void *)yyvsp[-1].nArgs);
            }
break;
case 43:
#line 235 "parse.y"
{
                ADD_OP(OP_PUSH_SYM); ADD_SYM(yyvsp[0].sym); ADD_OP(OP_INCR);
                ADD_OP(OP_ASSIGN); ADD_SYM(yyvsp[0].sym);
            }
break;
case 44:
#line 239 "parse.y"
{
                ADD_OP(OP_PUSH_SYM); ADD_SYM(yyvsp[-1].sym); ADD_OP(OP_INCR);
                ADD_OP(OP_ASSIGN); ADD_SYM(yyvsp[-1].sym);
            }
break;
case 45:
#line 243 "parse.y"
{
                ADD_OP(OP_PUSH_SYM); ADD_SYM(yyvsp[0].sym); ADD_OP(OP_DECR);
                ADD_OP(OP_ASSIGN); ADD_SYM(yyvsp[0].sym);
            }
break;
case 46:
#line 247 "parse.y"
{
                ADD_OP(OP_PUSH_SYM); ADD_SYM(yyvsp[-1].sym); ADD_OP(OP_DECR);
                ADD_OP(OP_ASSIGN); ADD_SYM(yyvsp[-1].sym);
            }
break;
case 47:
#line 252 "parse.y"
{
                yyval.sym = yyvsp[0].sym; ADD_OP(OP_PUSH_SYM); ADD_SYM(yyvsp[0].sym);
            }
break;
case 48:
#line 256 "parse.y"
{
                yyval.inst = GetPC();
            }
break;
case 49:
#line 259 "parse.y"
{
                yyval.inst = GetPC();
            }
break;
case 50:
#line 262 "parse.y"
{
                yyval.inst = GetPC();
            }
break;
case 51:
#line 266 "parse.y"
{
                yyval.nArgs = 0;
            }
break;
case 52:
#line 269 "parse.y"
{
                yyval.nArgs = 1;
            }
break;
case 53:
#line 272 "parse.y"
{
                yyval.nArgs = yyvsp[-2].nArgs + 1;
            }
break;
case 55:
#line 277 "parse.y"
{
                ADD_OP(OP_CONCAT);
            }
break;
case 56:
#line 281 "parse.y"
{
                    ADD_OP(OP_PUSH_ARRAY_SYM); ADD_SYM(yyvsp[0].sym); ADD_IMMED((void *)1);
                }
break;
case 57:
#line 284 "parse.y"
{
                    ADD_OP(OP_ARRAY_REF); ADD_IMMED((void *)yyvsp[-1].nArgs);
                }
break;
case 58:
#line 288 "parse.y"
{
                ADD_OP(OP_PUSH_ARRAY_SYM); ADD_SYM(yyvsp[0].sym); ADD_IMMED((void *)0);
            }
break;
case 59:
#line 291 "parse.y"
{
                ADD_OP(OP_ARRAY_REF); ADD_IMMED((void *)yyvsp[-1].nArgs);
            }
break;
case 60:
#line 295 "parse.y"
{
                yyval.inst = GetPC();
            }
break;
case 61:
#line 299 "parse.y"
{
                ADD_OP(OP_PUSH_SYM); ADD_SYM(yyvsp[0].sym);
            }
break;
case 62:
#line 302 "parse.y"
{
                ADD_OP(OP_PUSH_SYM); ADD_SYM(yyvsp[0].sym);
            }
break;
case 63:
#line 305 "parse.y"
{
                ADD_OP(OP_PUSH_SYM); ADD_SYM(yyvsp[0].sym);
            }
break;
case 64:
#line 308 "parse.y"
{
                ADD_OP(OP_SUBR_CALL);
                ADD_SYM(PromoteToGlobal(yyvsp[-3].sym)); ADD_IMMED((void *)yyvsp[-1].nArgs);
                ADD_OP(OP_FETCH_RET_VAL);
            }
break;
case 66:
#line 314 "parse.y"
{
                ADD_OP(OP_ARRAY_REF); ADD_IMMED((void *)yyvsp[-1].nArgs);
            }
break;
case 67:
#line 317 "parse.y"
{
                ADD_OP(OP_ADD);
            }
break;
case 68:
#line 320 "parse.y"
{
                ADD_OP(OP_SUB);
            }
break;
case 69:
#line 323 "parse.y"
{
                ADD_OP(OP_MUL);
            }
break;
case 70:
#line 326 "parse.y"
{
                ADD_OP(OP_DIV);
            }
break;
case 71:
#line 329 "parse.y"
{
                ADD_OP(OP_MOD);
            }
break;
case 72:
#line 332 "parse.y"
{
                ADD_OP(OP_POWER);
            }
break;
case 73:
#line 335 "parse.y"
{
                ADD_OP(OP_NEGATE);
            }
break;
case 74:
#line 338 "parse.y"
{
                ADD_OP(OP_GT);
            }
break;
case 75:
#line 341 "parse.y"
{
                ADD_OP(OP_GE);
            }
break;
case 76:
#line 344 "parse.y"
{
                ADD_OP(OP_LT);
            }
break;
case 77:
#line 347 "parse.y"
{
                ADD_OP(OP_LE);
            }
break;
case 78:
#line 350 "parse.y"
{
                ADD_OP(OP_EQ);
            }
break;
case 79:
#line 353 "parse.y"
{
                ADD_OP(OP_NE);
            }
break;
case 80:
#line 356 "parse.y"
{
                ADD_OP(OP_BIT_AND);
            }
break;
case 81:
#line 359 "parse.y"
{
                ADD_OP(OP_BIT_OR);
            }
break;
case 82:
#line 362 "parse.y"
{
                ADD_OP(OP_AND); SET_BR_OFF(yyvsp[-1].inst, GetPC());
            }
break;
case 83:
#line 365 "parse.y"
{
                ADD_OP(OP_OR); SET_BR_OFF(yyvsp[-1].inst, GetPC());
            }
break;
case 84:
#line 368 "parse.y"
{
                ADD_OP(OP_NOT);
            }
break;
case 85:
#line 371 "parse.y"
{
                ADD_OP(OP_PUSH_SYM); ADD_SYM(yyvsp[0].sym); ADD_OP(OP_INCR);
                ADD_OP(OP_DUP); ADD_OP(OP_ASSIGN); ADD_SYM(yyvsp[0].sym);
            }
break;
case 86:
#line 375 "parse.y"
{
                ADD_OP(OP_PUSH_SYM); ADD_SYM(yyvsp[-1].sym); ADD_OP(OP_DUP);
                ADD_OP(OP_INCR); ADD_OP(OP_ASSIGN); ADD_SYM(yyvsp[-1].sym);
            }
break;
case 87:
#line 379 "parse.y"
{
                ADD_OP(OP_PUSH_SYM); ADD_SYM(yyvsp[0].sym); ADD_OP(OP_DECR);
                ADD_OP(OP_DUP); ADD_OP(OP_ASSIGN); ADD_SYM(yyvsp[0].sym);
            }
break;
case 88:
#line 383 "parse.y"
{
                ADD_OP(OP_PUSH_SYM); ADD_SYM(yyvsp[-1].sym); ADD_OP(OP_DUP);
                ADD_OP(OP_DECR); ADD_OP(OP_ASSIGN); ADD_SYM(yyvsp[-1].sym);
            }
break;
case 89:
#line 387 "parse.y"
{
                ADD_OP(OP_IN_ARRAY);
            }
break;
case 90:
#line 391 "parse.y"
{
            yyval.inst = GetPC(); StartLoopAddrList();
        }
break;
case 91:
#line 395 "parse.y"
{
            StartLoopAddrList(); yyval.inst = GetPC();
        }
break;
case 92:
#line 399 "parse.y"
{
            ADD_OP(OP_BRANCH); yyval.inst = GetPC(); ADD_BR_OFF(0);
        }
break;
case 93:
#line 403 "parse.y"
{
            ADD_OP(OP_BRANCH_NEVER); yyval.inst = GetPC(); ADD_BR_OFF(0);
        }
break;
case 94:
#line 406 "parse.y"
{
            ADD_OP(OP_BRANCH_FALSE); yyval.inst = GetPC(); ADD_BR_OFF(0);
        }
break;
case 95:
#line 410 "parse.y"
{
            ADD_OP(OP_DUP); ADD_OP(OP_BRANCH_FALSE); yyval.inst = GetPC();
            ADD_BR_OFF(0);
        }
break;
case 96:
#line 415 "parse.y"
{
            ADD_OP(OP_DUP); ADD_OP(OP_BRANCH_TRUE); yyval.inst = GetPC();
            ADD_BR_OFF(0);
        }
break;
#line 1736 "y.tab.c"
    }
    yyssp -= yym;
    yystate = *yyssp;
    yyvsp -= yym;
    yym = yylhs[yyn];
    if (yystate == 0 && yym == 0)
    {
#if YYDEBUG
        if (yydebug)
            printf("%sdebug: after reduction, shifting from state 0 to\
 state %d\n", YYPREFIX, YYFINAL);
#endif
        yystate = YYFINAL;
        *++yyssp = YYFINAL;
        *++yyvsp = yyval;
        if (yychar < 0)
        {
            if ((yychar = yylex()) < 0) yychar = 0;
#if YYDEBUG
            if (yydebug)
            {
                yys = 0;
                if (yychar <= YYMAXTOKEN) yys = yyname[yychar];
                if (!yys) yys = "illegal-symbol";
                printf("%sdebug: state %d, reading %d (%s)\n",
                        YYPREFIX, YYFINAL, yychar, yys);
            }
#endif
        }
        if (yychar == 0) goto yyaccept;
        goto yyloop;
    }
    if ((yyn = yygindex[yym]) && (yyn += yystate) >= 0 &&
            yyn <= YYTABLESIZE && yycheck[yyn] == yystate)
        yystate = yytable[yyn];
    else
        yystate = yydgoto[yym];
#if YYDEBUG
    if (yydebug)
        printf("%sdebug: after reduction, shifting from state %d \
to state %d\n", YYPREFIX, *yyssp, yystate);
#endif
    if (yyssp >= yysslim && yygrowstack())
    {
        goto yyoverflow;
    }
    *++yyssp = yystate;
    *++yyvsp = yyval;
    goto yyloop;
yyoverflow:
    yyerror("yacc stack overflow");
yyabort:
    return (1);
yyaccept:
    return (0);
}
