%{
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
#include "interpret.h"
#include "parse.h"

/* Macros to add error processing to AddOp and AddSym calls */
#define ADD_OP(op) if (!AddOp(op, &ErrMsg)) return 1
#define ADD_SYM(sym) if (!AddSym(sym, &ErrMsg)) return 1
#define ADD_IMMED(val) if (!AddImmediate(val, &ErrMsg)) return 1
#define ADD_BR_OFF(to) if (!AddBranchOffset(to, &ErrMsg)) return 1
#define SET_BR_OFF(from, to) *((int *)from) = to - from

/* Max. length for a string constant (... there shouldn't be a maximum) */
#define MAX_STRING_CONST_LEN 5000

static int yylex(void);
static int follow(char expect, int yes, int no);
static int follow2(char expect1, int yes1, char expect2, int yes2, int no);
static Symbol *matchesActionRoutine(char **inPtr);

static char *ErrMsg;
static char *InPtr;

%}

%union {
    Symbol *sym;
    Inst *inst;
    int nArgs;
}
%token <sym> NUMBER STRING SYMBOL
%token IF WHILE ELSE FOR BREAK CONTINUE RETURN
%type <nArgs> arglist
%type <inst> cond comastmts while else and or
%type <sym> evalsym

%nonassoc IF_NO_ELSE
%nonassoc ELSE

%nonassoc SYMBOL
%right	  '=' ADDEQ SUBEQ MULEQ DIVEQ MODEQ ANDEQ OREQ
%left     CONCAT
%left	  OR
%left	  AND
%left	  '|'
%left	  '&'
%left	  GT GE LT LE EQ NE
%left	  '+' '-'
%left	  '*' '/' '%'
%nonassoc UNARY_MINUS NOT
%nonassoc INCR DECR
%right	  POW
%nonassoc '('

%%	/* Rules */

program:  blank stmts { ADD_OP(OP_RETURN_NO_VAL); return 0; }
    	| blank '{' blank stmts '}' { ADD_OP(OP_RETURN_NO_VAL); return 0; }
	| blank '{' blank '}' { ADD_OP(OP_RETURN_NO_VAL); return 0; }
	| error { return 1; }
	;
block:   '{' blank stmts '}' blank
    	| '{' blank '}' blank
    	| stmt
    	;
stmts:    stmt
	| stmts stmt
	;
stmt:     simpstmt '\n' blank
    	| IF '(' cond ')' blank block %prec IF_NO_ELSE
	    	{ SET_BR_OFF((Inst *)$3, GetPC()); }
    	| IF '(' cond ')' blank block else blank block %prec ELSE
    	    	{ SET_BR_OFF($3, ($7+1)); SET_BR_OFF($7, GetPC()); }
    	| while '(' cond ')' blank block { ADD_OP(OP_BRANCH); ADD_BR_OFF($1);
    	    	SET_BR_OFF($3, GetPC()); FillLoopAddrs(GetPC(), $1); }
    	| for '(' comastmts ';' cond ';' comastmts ')' blank block
    	    	{ FillLoopAddrs(GetPC()+2+($7-($5+1)), GetPC());
    	    	  SwapCode($5+1, $7, GetPC());
    	    	  ADD_OP(OP_BRANCH); ADD_BR_OFF($3); SET_BR_OFF($5, GetPC()); }
    	| BREAK '\n' blank
    	    	{ ADD_OP(OP_BRANCH); ADD_BR_OFF(0); AddBreakAddr(GetPC()-1); }
    	| CONTINUE '\n' blank
    	    	{ ADD_OP(OP_BRANCH); ADD_BR_OFF(0); AddContinueAddr(GetPC()-1); }
	| RETURN expr '\n' blank { ADD_OP(OP_RETURN); }
	| RETURN '\n' blank { ADD_OP(OP_RETURN_NO_VAL); }
	; 
simpstmt: SYMBOL '=' expr { ADD_OP(OP_ASSIGN); ADD_SYM($1); }
    	| evalsym ADDEQ expr { ADD_OP(OP_ADD); ADD_OP(OP_ASSIGN); ADD_SYM($1); }
    	| evalsym SUBEQ expr { ADD_OP(OP_SUB); ADD_OP(OP_ASSIGN); ADD_SYM($1); }
    	| evalsym MULEQ expr { ADD_OP(OP_MUL); ADD_OP(OP_ASSIGN); ADD_SYM($1); }
    	| evalsym DIVEQ expr { ADD_OP(OP_DIV); ADD_OP(OP_ASSIGN); ADD_SYM($1); }
    	| evalsym MODEQ expr { ADD_OP(OP_MOD); ADD_OP(OP_ASSIGN); ADD_SYM($1); }
    	| evalsym ANDEQ expr { ADD_OP(OP_BIT_AND); ADD_OP(OP_ASSIGN);
    	    	ADD_SYM($1); }
    	| evalsym OREQ expr { ADD_OP(OP_BIT_OR); ADD_OP(OP_ASSIGN);
    	    	ADD_SYM($1); }
	| SYMBOL '(' arglist ')' { ADD_OP(OP_SUBR_CALL);
	    	ADD_SYM(PromoteToGlobal($1)); ADD_IMMED((void *)$3); }
    	| INCR SYMBOL { ADD_OP(OP_PUSH_SYM); ADD_SYM($2); ADD_OP(OP_INCR);
    	    	ADD_OP(OP_ASSIGN); ADD_SYM($2); }
	| SYMBOL INCR { ADD_OP(OP_PUSH_SYM); ADD_SYM($1); ADD_OP(OP_INCR);
		ADD_OP(OP_ASSIGN); ADD_SYM($1); }
	| DECR SYMBOL { ADD_OP(OP_PUSH_SYM); ADD_SYM($2); ADD_OP(OP_DECR);
	    	ADD_OP(OP_ASSIGN); ADD_SYM($2); }
	| SYMBOL DECR { ADD_OP(OP_PUSH_SYM); ADD_SYM($1); ADD_OP(OP_DECR);
		ADD_OP(OP_ASSIGN); ADD_SYM($1); }
	;
evalsym: SYMBOL { $$ = $1; ADD_OP(OP_PUSH_SYM); ADD_SYM($1); }
    	;
comastmts: /* nothing */ { $$ = GetPC(); }
    	| simpstmt { $$ = GetPC(); }
    	| comastmts ',' simpstmt { $$ = GetPC(); }
    	;
arglist:  /* nothing */ { $$ = 0;}
    	| expr { $$ = 1; }
    	| arglist ',' expr { $$ = $1 + 1; }
    	;
expr:     numexpr %prec CONCAT
    	| expr numexpr %prec CONCAT { ADD_OP(OP_CONCAT); }
    	;
numexpr:  NUMBER { ADD_OP(OP_PUSH_SYM); ADD_SYM($1); }
    	| STRING { ADD_OP(OP_PUSH_SYM); ADD_SYM($1); }
	| SYMBOL { ADD_OP(OP_PUSH_SYM); ADD_SYM($1); }
	| SYMBOL '(' arglist ')' { ADD_OP(OP_SUBR_CALL);
	    	ADD_SYM(PromoteToGlobal($1)); ADD_IMMED((void *)$3);
		ADD_OP(OP_FETCH_RET_VAL);}
	| '(' numexpr ')'
	| numexpr '+' numexpr { ADD_OP(OP_ADD); }
	| numexpr '-' numexpr { ADD_OP(OP_SUB); }
	| numexpr '*' numexpr { ADD_OP(OP_MUL); }
	| numexpr '/' numexpr { ADD_OP(OP_DIV); }
	| numexpr '%' numexpr { ADD_OP(OP_MOD); }
	| numexpr POW numexpr { ADD_OP(OP_POWER); }
	| '-' numexpr %prec UNARY_MINUS  { ADD_OP(OP_NEGATE); }
	| numexpr GT numexpr  { ADD_OP(OP_GT); }
	| numexpr GE numexpr  { ADD_OP(OP_GE); }
	| numexpr LT numexpr  { ADD_OP(OP_LT); }
	| numexpr LE numexpr  { ADD_OP(OP_LE); }
	| numexpr EQ numexpr  { ADD_OP(OP_EQ); }
	| numexpr NE numexpr  { ADD_OP(OP_NE); }
	| numexpr '&' numexpr { ADD_OP(OP_BIT_AND); }
	| numexpr '|' numexpr  { ADD_OP(OP_BIT_OR); } 
	| numexpr and numexpr %prec AND
	    	{ ADD_OP(OP_AND); SET_BR_OFF($2, GetPC()); }
	| numexpr or numexpr %prec OR
	    	{ ADD_OP(OP_OR); SET_BR_OFF($2, GetPC()); } 
	| NOT numexpr         { ADD_OP(OP_NOT); }
	| INCR SYMBOL { ADD_OP(OP_PUSH_SYM); ADD_SYM($2); ADD_OP(OP_INCR);
    	    	ADD_OP(OP_DUP); ADD_OP(OP_ASSIGN); ADD_SYM($2); }
	| SYMBOL INCR { ADD_OP(OP_PUSH_SYM); ADD_SYM($1); ADD_OP(OP_DUP);
	    	ADD_OP(OP_INCR); ADD_OP(OP_ASSIGN); ADD_SYM($1); }
	| DECR SYMBOL { ADD_OP(OP_PUSH_SYM); ADD_SYM($2); ADD_OP(OP_DECR);
	    	ADD_OP(OP_DUP); ADD_OP(OP_ASSIGN); ADD_SYM($2); }
	| SYMBOL DECR { ADD_OP(OP_PUSH_SYM); ADD_SYM($1); ADD_OP(OP_DUP);
	    	ADD_OP(OP_DECR); ADD_OP(OP_ASSIGN); ADD_SYM($1); }
    	;
while:	 WHILE { $$ = GetPC(); StartLoopAddrList(); }
    	;
for: 	 FOR { StartLoopAddrList(); }
    	;
else:	 ELSE { ADD_OP(OP_BRANCH); $$ = GetPC(); ADD_BR_OFF(0); }
    	;
cond:	  /* nothing */ { ADD_OP(OP_BRANCH_NEVER); $$ = GetPC(); ADD_BR_OFF(0); }
	| numexpr { ADD_OP(OP_BRANCH_FALSE); $$ = GetPC(); ADD_BR_OFF(0); }
    	;
and:	AND { ADD_OP(OP_DUP); ADD_OP(OP_BRANCH_FALSE); $$ = GetPC();
    	    	ADD_BR_OFF(0); }
    	;
or:	OR { ADD_OP(OP_DUP); ADD_OP(OP_BRANCH_TRUE); $$ = GetPC();
    	    	ADD_BR_OFF(0); }
    	;
blank:	  /* nothing */
	| blank '\n'
	;
	
%% /* User Subroutines Section */

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
    if (isdigit(*InPtr))  { /* number */
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
    if (isalpha(*InPtr) || *InPtr == '$') {
        if ((s=matchesActionRoutine(&InPtr)) == NULL) {
            char symName[MAX_SYM_LEN+1], *p = symName;
            *p++ = *InPtr++;
            while (isalnum(*InPtr) || *InPtr=='_') {
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
	    if (!strcmp(symName, "define")) {
	    	InPtr -= 6;
	    	return 0;
	    }
	    if ((s=LookupSymbol(symName)) == NULL) {
        	s = InstallSymbol(symName, symName[0]=='$' ? (isdigit(symName[1]) ?
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
    for (c = *inPtr; isalnum(*c) || *c=='_' || (*c=='-'&&isalnum(*(c+1))); c++){
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
int yyerror(char *s)
{
    ErrMsg = s;
    return 0;
}
