/*
 * CompileRE, ExecRE, and substituteRE -- regular expression parsing
 *
 * This is an altered version of Henry Spencer's regcomp and regexec code
 * adapted for NEdit.  Original copyright notice:
 *
 *	   Copyright (c) 1986 by University of Toronto.
 *	   Written by Henry Spencer.  Not derived from licensed software.
 *	 
 *	   Permission is granted to anyone to use this software for any
 *	   purpose on any computer system, and to redistribute it freely,
 *	   subject to the following restrictions:
 *	 
 *	   1. The author is not responsible for the consequences of use of
 *	 	this software, no matter how awful, even if they arise
 *	 	from defects in it.
 *	 
 *	   2. The origin of this software must not be misrepresented, either
 *	 	by explicit claim or by omission.
 *	 
 *	   3. Altered versions must be plainly marked as such, and must not
 *	 	be misrepresented as being the original software.
 *	 
 * Beware that some of this code is subtly aware of the way operator
 * precedence is structured in regular expressions.  Serious changes in
 * regular-expression syntax might require a total rethink.
 *
 * January, 1994, M.E.: Consolidated files, changed names of external
 * 			functions to avoid potential conflicts with native
 *			regcomp and regexec functions, changed error
 *			reporting to NEdit form, added multi-line and
 *			reverse searching, and added \n \t \u \U \l \L.
 * June, 1996, M.E.:	Bug in NEXT macro, didn't work for expressions
 *  	    	    	which compiled to over 256 bytes.
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "regularExp.h"

/*
 * The first byte of the regexp internal "program" is actually this magic
 * number; the start node begins in the second byte.
 */
#define MAGIC	0234

/*
 * The "internal use only" fields in regexp.h are present to pass info from
 * compile to execute that permits the execute phase to run lots faster on
 * simple cases.  They are:
 *
 * regstart char that must begin a match; '\0' if none obvious
 * reganch  is the match anchored (at beginning-of-line only)?
 * regmust  string (pointer into program) that match must include, or NULL
 * regmlen  length of regmust string
 *
 * Regstart and reganch permit very fast decisions on suitable starting points
 * for a match, cutting down the work a lot.  Regmust permits fast rejection
 * of lines that cannot possibly match.	 The regmust tests are costly enough
 * that CompileRE() supplies a regmust only if the r.e. contains something
 * potentially expensive (at present, the only such thing detected is * or +
 * at the start of the r.e., which can involve a lot of backup).  Regmlen is
 * supplied because the test in ExecRE() needs it and CompileRE() is computing
 * it anyway.
 */

/*
 * Structure for regexp "program".  This is essentially a linear encoding
 * of a nondeterministic finite-state machine (aka syntax charts or
 * "railroad normal form" in parsing technology).  Each node is an opcode
 * plus a "next" pointer, possibly plus an operand.  "Next" pointers of
 * all nodes except BRANCH implement concatenation; a "next" pointer with
 * a BRANCH on both ends of it is connecting two alternatives.	(Here we
 * have one of the subtle syntax dependencies:	an individual BRANCH (as
 * opposed to a collection of them) is never concatenated with anything
 * because of operator precedence.)  The operand of some types of node is
 * a literal string; for others, it is a node leading into a sub-FSM.  In
 * particular, the operand of a BRANCH node is the first node of the branch.
 * (NB this is *not* a tree structure:	the tail of the branch connects
 * to the thing following the set of BRANCHes.)	 The opcodes are:
 */

/* definition	number	opnd?	meaning */
#define END     0   /* no	End of program. */
#define BOL     1   /* no	Match "" at beginning of line. */
#define EOL     2   /* no	Match "" at end of line. */
#define ANY     3   /* no	Match any one character. */
#define ANYOF	4   /* str  Match any character in this string. */
#define ANYBUT	5   /* str  Match any character not in this string. */
#define BRANCH	6   /* node Match this alternative, or the next... */
#define BACK	7   /* no   Match "", "next" ptr points backward. */
#define EXACTLY 8   /* str  Match this string. */
#define NOTHING 9   /* no   Match empty string. */
#define STAR	10  /* node Match this (simple) thing 0 or more times. */
#define PLUS	11  /* node Match this (simple) thing 1 or more times. */
#define BOWORD	12  /* no   	Match "" representing word delimiter or BOL */
#define EOWORD	13  /* no   	Match "" representing word delimiter or EOL */
#define OPEN	20  /* no   Mark this point in input as start of #n. */
	    	    /*	OPEN+1 is number 1, etc. */
#define CLOSE	(OPEN + NSUBEXP)  /* no   Analogous to OPEN. */

/*
 * Opcode notes:
 *
 * BRANCH   The set of branches constituting a single choice are hooked
 *	together with their "next" pointers, since precedence prevents
 *	anything being concatenated to any individual branch.  The
 *	"next" pointer of the last BRANCH in a choice points to the
 *	thing following the whole choice.  This is also where the
 *	final "next" pointer of each individual branch points; each
 *	branch starts with the operand node of a BRANCH node.
 *
 * BACK	    Normal "next" pointers all implicitly point forward; BACK
 *	exists to make loop structures possible.
 *
 * STAR,PLUS	'?', and complex '*' and '+', are implemented as circular
 *	BRANCH structures using BACK.  Simple cases (one character
 *	per match) are implemented with STAR and PLUS for speed
 *	and to minimize recursive plunges.
 *
 * OPEN,CLOSE	...are numbered at compile time.
 */

/*
 * A node is one char of opcode followed by two chars of "next" pointer.
 * "Next" pointers are stored as two 8-bit pieces, high order first.  The
 * value is a positive offset from the opcode of the node containing it.
 * An operand, if any, simply follows the node.	 (Note that much of the
 * code generation knows about this implicit relationship.)
 *
 * Using two bytes for the "next" pointer is vast overkill for most things,
 * but allows patterns to get big without disasters.
 */
#define OP(p)	(*(p))
#define NEXT(p) (((*((p)+1)&0377)<<8) + ((*((p)+2))&0377))
#define OPERAND(p)  ((p) + 3)

/*
 * Utility definitions.
 */
#define UCHARAT(p)  ((int)*(unsigned char *)(p))
#define FAIL(m) { *errorpointer = m; return(NULL); }
#define ISMULT(c)   ((c) == '*' || (c) == '+' || (c) == '?')
#define META	"^$.[()|?+*\\<>"

/*
 * Flags to be passed up and down.
 */
#define HASWIDTH    01	/* Known never to match null string. */
#define SIMPLE	    02	/* Simple enough to be STAR/PLUS operand. */
#define SPSTART	    04	/* Starts with * or +. */
#define WORST	    0	/* Worst case. */

/*
 * Global work variables for CompileRE().
 */
static char *regparse;	    /* Input-scan pointer. */
static int regnpar;	    /* () count. */
static char regdummy;
static char *regcode;	    /* Code-emit pointer; &regdummy = don't. */
static long regsize;	    /* Code size. */
static char **errorpointer; /* Place to store error messages so they can be
			       returned by CompileRE */

/*
 * Forward declarations for CompileRE()'s friends.
 */
static char *reg(int paren, int *flagp);
static char *regbranch(int *flagp);
static char *regpiece(int *flagp);
static char *regatom(int *flagp);
static char *regnode(char op);
static char *regnext(char *p);
static void regc(char b);
static void reginsert(char op, char *opnd);
static void regtail(char *p, char *val);
static void regoptail(char *p, char *val);
static void regerror(char *s);
static char regescape(char c);

/*
 - CompileRE - compile a regular expression into internal code
 *
 * We can't allocate space until we know how big the compiled form will be,
 * but we can't compile it (and thus know how big it is) until we've got a
 * place to put the code.  So we cheat:	 we compile it twice, once with code
 * generation turned off and size counting turned on, and once "for real".
 * This also means that we don't allocate space until we are sure that the
 * thing really will compile successfully, and we never have to move the
 * code and thus invalidate pointers into it.  (Note that it has to be in
 * one piece because free() must be able to free it all.)
 *
 * Beware that the optimization-preparation code in here knows about some
 * of the structure of the compiled regexp.
 */
regexp *CompileRE(char *exp, char **errorText)
{
    register regexp *r;
    register char *scan;
    register char *longest;
    register int len;
    int flags;

    /* Set up errorText to receive failure reports */
    errorpointer = errorText;
    *errorpointer = NULL;
    
    if (exp == NULL)
	FAIL("NULL argument");

    /* First pass: determine size, legality. */
    regparse = exp;
    regnpar = 1;
    regsize = 0L;
    regcode = &regdummy;
    regc(MAGIC);
    if (reg(0, &flags) == NULL)
	return(NULL);

    /* Small enough for pointer-storage convention? */
    if (regsize >= 32767L)	/* Probably could be 65535L. */
	FAIL("regexp too big");

    /* Allocate space. */
    r = (regexp *)malloc(sizeof(regexp) + (unsigned)regsize);
    if (r == NULL)
	FAIL("out of space");

    /* Second pass: emit code. */
    regparse = exp;
    regnpar = 1;
    regcode = r->program;
    regc(MAGIC);
    if (reg(0, &flags) == NULL)
	return(NULL);

    /* Dig out information for optimizations. */
    r->regstart = '\0'; /* Worst-case defaults. */
    r->reganch = 0;
    r->regmust = NULL;
    r->regmlen = 0;
    scan = r->program+1;	    /* First BRANCH. */
    if (OP(regnext(scan)) == END) {	/* Only one top-level choice. */
	scan = OPERAND(scan);

	/* Starting-point info. */
	if (OP(scan) == EXACTLY)
	    r->regstart = *OPERAND(scan);
	else if (OP(scan) == BOL)
	    r->reganch++;

	/*
	 * If there's something expensive in the r.e., find the
	 * longest literal string that must appear and make it the
	 * regmust.  Resolve ties in favor of later strings, since
	 * the regstart check works with the beginning of the r.e.
	 * and avoiding duplication strengthens checking.  Not a
	 * strong reason, but sufficient in the absence of others.
	 */
	if (flags&SPSTART) {
	    longest = NULL;
	    len = 0;
	    for (; scan != NULL; scan = regnext(scan))
		if (OP(scan) == EXACTLY && strlen(OPERAND(scan)) >= len) {
		    longest = OPERAND(scan);
		    len = strlen(OPERAND(scan));
		}
	    r->regmust = longest;
	    r->regmlen = len;
	}
    }

    return(r);
}

/*
 - reg - regular expression, i.e. main body or parenthesized thing
 *
 * Caller must absorb opening parenthesis.
 *
 * Combining parenthesis handling with the base level of regular expression
 * is a trifle forced, but the need to tie the tails of the branches to what
 * follows makes it hard to avoid.
 */
static char *reg(int paren, int *flagp)
{
    register char *ret;
    register char *br;
    register char *ender;
    register int parno;
    int flags;

    *flagp = HASWIDTH;	/* Tentatively. */

    /* Make an OPEN node, if parenthesized. */
    if (paren) {
	if (regnpar >= NSUBEXP)
	    FAIL("too many ()");
	parno = regnpar;
	regnpar++;
	ret = regnode(OPEN+parno);
    } else
	ret = NULL;

    /* Pick up the branches, linking them together. */
    br = regbranch(&flags);
    if (br == NULL)
	return(NULL);
    if (ret != NULL)
	regtail(ret, br);   /* OPEN -> first. */
    else
	ret = br;
    if (!(flags&HASWIDTH))
	*flagp &= ~HASWIDTH;
    *flagp |= flags&SPSTART;
    while (*regparse == '|') {
	regparse++;
	br = regbranch(&flags);
	if (br == NULL)
	    return(NULL);
	regtail(ret, br);   /* BRANCH -> BRANCH. */
	if (!(flags&HASWIDTH))
	    *flagp &= ~HASWIDTH;
	*flagp |= flags&SPSTART;
    }

    /* Make a closing node, and hook it on the end. */
    ender = regnode((paren) ? CLOSE+parno : END);   
    regtail(ret, ender);

    /* Hook the tails of the branches to the closing node. */
    for (br = ret; br != NULL; br = regnext(br))
	regoptail(br, ender);

    /* Check for proper termination. */
    if (paren && *regparse++ != ')') {
	FAIL("unmatched ()");
    } else if (!paren && *regparse != '\0') {
	if (*regparse == ')') {
	    FAIL("unmatched ()");
	} else
	    FAIL("junk on end");    /* "Can't happen". */
	/* NOTREACHED */
    }

    return(ret);
}

/*
 - regbranch - one alternative of an | operator
 *
 * Implements the concatenation operator.
 */
static char *regbranch(int *flagp)
{
    register char *ret;
    register char *chain;
    register char *latest;
    int flags;

    *flagp = WORST;	/* Tentatively. */

    ret = regnode(BRANCH);
    chain = NULL;
    while (*regparse != '\0' && *regparse != '|' && *regparse != ')') {
	latest = regpiece(&flags);
	if (latest == NULL)
	    return(NULL);
	*flagp |= flags&HASWIDTH;
	if (chain == NULL)  /* First piece. */
	    *flagp |= flags&SPSTART;
	else
	    regtail(chain, latest);
	chain = latest;
    }
    if (chain == NULL)	/* Loop ran zero times. */
	(void) regnode(NOTHING);

    return(ret);
}

/*
 - regpiece - something followed by possible [*+?]
 *
 * Note that the branching code sequences used for ? and the general cases
 * of * and + are somewhat optimized:  they use the same NOTHING node as
 * both the endmarker for their branch list and the body of the last branch.
 * It might seem that this node could be dispensed with entirely, but the
 * endmarker role is not redundant.
 */
static char *regpiece(int *flagp)
{
    register char *ret;
    register char op;
    register char *next;
    int flags;

    ret = regatom(&flags);
    if (ret == NULL)
	return(NULL);

    op = *regparse;
    if (!ISMULT(op)) {
	*flagp = flags;
	return(ret);
    }

    if (!(flags&HASWIDTH) && op != '?')
	FAIL("*+ operand could be empty");
    *flagp = (op != '+') ? (WORST|SPSTART) : (WORST|HASWIDTH);

    if (op == '*' && (flags&SIMPLE))
	reginsert(STAR, ret);
    else if (op == '*') {
	/* Emit x* as (x&|), where & means "self". */
	reginsert(BRANCH, ret);		    /* Either x */
	regoptail(ret, regnode(BACK));	    /* and loop */
	regoptail(ret, ret);		    /* back */
	regtail(ret, regnode(BRANCH));	    /* or */
	regtail(ret, regnode(NOTHING));	    /* null. */
    } else if (op == '+' && (flags&SIMPLE))
	reginsert(PLUS, ret);
    else if (op == '+') {
	/* Emit x+ as x(&|), where & means "self". */
	next = regnode(BRANCH);		    /* Either */
	regtail(ret, next);
	regtail(regnode(BACK), ret);	    /* loop back */
	regtail(next, regnode(BRANCH));	    /* or */
	regtail(ret, regnode(NOTHING));	    /* null. */
    } else if (op == '?') {
	/* Emit x? as (x|) */
	reginsert(BRANCH, ret);		    /* Either x */
	regtail(ret, regnode(BRANCH));	    /* or */
	next = regnode(NOTHING);	    /* null. */
	regtail(ret, next);
	regoptail(ret, next);
    }
    regparse++;
    if (ISMULT(*regparse))
	FAIL("nested *?+");

    return(ret);
}

/*
 - regatom - the lowest level
 *
 * Optimization:  gobbles an entire sequence of ordinary characters so that
 * it can turn them into a single node, which is smaller to store and
 * faster to run.  Backslashed characters are exceptions, each becoming a
 * separate node; the code is simpler that way and it's not worth fixing.
 */
static char *regatom(int *flagp)
{
    register char *ret;
    int flags;

    *flagp = WORST;	/* Tentatively. */

    switch (*regparse++) {
    case '^':
	ret = regnode(BOL);
	break;
    case '$':
	ret = regnode(EOL);
	break;
    case '<':
	ret = regnode(BOWORD);
	break;
    case '>':
	ret = regnode(EOWORD);
	break;
    case '.':
	ret = regnode(ANY);
	*flagp |= HASWIDTH|SIMPLE;
	break;
    case '[': {
	    register int class;
	    register int classend;

	    if (*regparse == '^') { /* Complement of range. */
		ret = regnode(ANYBUT);
		regparse++;
	    } else
		ret = regnode(ANYOF);
	    if (*regparse == ']' || *regparse == '-')
		regc(*regparse++);
	    while (*regparse != '\0' && *regparse != ']') {
		if (*regparse == '-') {
		    regparse++;
		    if (*regparse == ']' || *regparse == '\0')
			regc('-');
		    else {
			class = UCHARAT(regparse-2)+1;
			classend = UCHARAT(regparse);
			if (class > classend+1)
			    FAIL("invalid [] range");
			for (; class <= classend; class++)
			    regc(class);
			regparse++;
		    }
		} else if (*regparse == '\\') {
		    regparse++;
		    if (regescape(*regparse) != '\0')
		    	regc(regescape(*regparse));
		    else
		    	FAIL("invalid escape (\\) sequence");
		    regparse++;
		} else
		    regc(*regparse++);
	    }
	    regc('\0');
	    if (*regparse != ']')
		FAIL("unmatched []");
	    regparse++;
	    *flagp |= HASWIDTH|SIMPLE;
	}
	break;
    case '(':
	ret = reg(1, &flags);
	if (ret == NULL)
	    return(NULL);
	*flagp |= flags&(HASWIDTH|SPSTART);
	break;
    case '\0':
    case '|':
    case ')':
	FAIL("internal urp");	/* Supposed to be caught earlier. */
	break;
    case '?':
    case '+':
    case '*':
	FAIL("?+* follows nothing");
	break;
    case '\\':
	ret = regnode(EXACTLY);
	if (regescape(*regparse) != '\0')
	    regc(regescape(*regparse));
	else
	    FAIL("invalid escape (\\) sequence");
	regparse++;
	regc('\0');
	*flagp |= HASWIDTH|SIMPLE;
	break;
    default: {
	    register int len;
	    register char ender;

	    regparse--;
	    len = strcspn(regparse, META);
	    if (len <= 0)
		FAIL("internal disaster");
	    ender = *(regparse+len);
	    if (len > 1 && ISMULT(ender))
		len--;	    /* Back off clear of ?+* operand. */
	    *flagp |= HASWIDTH;
	    if (len == 1)
		*flagp |= SIMPLE;
	    ret = regnode(EXACTLY);
	    while (len > 0) {
		regc(*regparse++);
		len--;
	    }
	    regc('\0');
	}
	break;
    }

    return(ret);
}

/*
 - regnode - emit a node
 */
static char *regnode(char op)
{
    register char *ret;
    register char *ptr;

    ret = regcode;
    if (ret == &regdummy) {
	regsize += 3;
	return(ret);
    }

    ptr = ret;
    *ptr++ = op;
    *ptr++ = '\0';	/* Null "next" pointer. */
    *ptr++ = '\0';
    regcode = ptr;

    return(ret);
}

/*
 - regc - emit (if appropriate) a byte of code
 */
static void regc(char b)
{
    if (regcode != &regdummy)
	*regcode++ = b;
    else
	regsize++;
}

/*
 - reginsert - insert an operator in front of already-emitted operand
 *
 * Means relocating the operand.
 */
static void reginsert(char op, char *opnd)
{
    register char *src;
    register char *dst;
    register char *place;

    if (regcode == &regdummy) {
	regsize += 3;
	return;
    }

    src = regcode;
    regcode += 3;
    dst = regcode;
    while (src > opnd)
	*--dst = *--src;

    place = opnd;	/* Op node, where operand used to be. */
    *place++ = op;
    *place++ = '\0';
    *place++ = '\0';
}

/*
 - regtail - set the next-pointer at the end of a node chain
 */
static void regtail(char *p, char *val)
{
    register char *scan;
    register char *temp;
    register int offset;

    if (p == &regdummy)
	return;

    /* Find last node. */
    scan = p;
    for (;;) {
	temp = regnext(scan);
	if (temp == NULL)
	    break;
	scan = temp;
    }

    if (OP(scan) == BACK)
	offset = scan - val;
    else
	offset = val - scan;
    *(scan+1) = (offset>>8)&0377;
    *(scan+2) = offset&0377;
}

/*
 - regescape - recognize legal escape (prefixed with backslash) characters,
 - and translate them into the corresponding ascii code
 */
static char regescape(char c)
{
    static char escape[] =  {'*','+','?','(',')','^','|','.','$','&','\\',
    	'[', ']', '<', '>', '\"', '\'', 'n','t','b','r','f', 'a', 'v', '\0'};
    static char replace[] = {'*','+','?','(',')','^','|','.','$','&','\\',
    	'[', ']', '<', '>', '\"', '\'','\n','\t','\b','\r','\f','\a','\v','\0'};
    int i;
    
    for (i=0; escape[i] != '\0'; i++)
    	if (c == escape[i])
    	    return replace[i];
    return '\0';
}


/*
 - regoptail - regtail on operand of first argument; nop if operandless
 */
static void regoptail(char *p, char *val)
{
    /* "Operandless" and "op != BRANCH" are synonymous in practice. */
    if (p == NULL || p == &regdummy || OP(p) != BRANCH)
	return;
    regtail(OPERAND(p), val);
}

/*
 * ExecRE and friends
 */

/*
 * Global work variables for ExecRE().
 */
static char *reginput;	      /* String-input pointer. */
static char *startofstring;   /* Beginning of input, for ^ and < checks. */
static int stringstartsline;  /* input begins is line boundary */
static int stringstartsword;  /* input begins at word boundary */
static char **regstartp;      /* Pointer to startp array. */
static char **regendp;	      /* Ditto for endp. */
static char defaultdelimiters[256] = {0}; /* Default table for determining
				 whether a character is a word delimiter */
static char *isdelimiter;     /* Current delimiter table */
/*
 * Forwards.
 */
static int regtry(regexp *prog, char *string);
static int regmatch(char *prog);
static int regrepeat(char *p);
static void adjustcase(char *str, int len, char chgcase);
static char *makeDelimiterTable(char *delimiters, char *table);

/*
 - ExecRE - match a regexp against a string
 *
 * If end is non-NULL, matches may not *begin* past end, but may extend past
 * it.  If reverse is true, end must be specified, and searching begins at end.
 * isbol should be set to true if the beginning of the string is the actual
 * beginning of a line (since ExecRE can't look backwards from the beginning
 * to find whether there was a newline before).  Likewise, isbow asks whether
 * the string is preceded by a word delimiter.  End of string is always treated
 * as a word and line boundary (there may be cases where it shouldn't be, in
 * which case, this should be changed).  "delimit" (if non-null) specifies
 * a null-terminated string of characters to be considered word delimiters
 * matching "<" and ">".  if "delimit" is NULL, the default delimiters (as set
 * in SetREDefaultWordDelimiters are used.
 */
int ExecRE(regexp *prog, char *string, char *end, int reverse, int isbol,
	int isbow, char *delimiters)
{
    register char *s;
    char tempDelimitTable[256];

    /* Be paranoid... */
    if (prog == NULL || string == NULL) {
	regerror("NULL parameter");
	return(0);
    }

    /* Check validity of program. */
    if (UCHARAT(prog->program) != MAGIC) {
	regerror("corrupted program");
	return(0);
    }

    /* If caller has supplied delimiters, make a delimiter table */
    if (delimiters == NULL)
    	isdelimiter = defaultdelimiters;
    else
    	isdelimiter = makeDelimiterTable(delimiters, tempDelimitTable);

    /* If there is a "must appear" string, look for it, reject if not found. */
    if (prog->regmust != NULL) {
	for (s=string; *s!='\0' && s==end; s++) { /*... This must be wrong!!! */
	    if (*s == prog->regmust[0]) {
		if (strncmp(s, prog->regmust, prog->regmlen) == 0)
		    break;	/* Found it. */
	    }
	}
	if (s == end || *s == '\0')	/* Not present. */
	    return(0);
    }

    /* remember the beginning of the string for matching BOL */
    startofstring = string;
    stringstartsline = isbol;
    stringstartsword = isbow;
    
    /* Search forward */
    if (!reverse) {
	
	/* If search is anchored at BOL */
	if (prog->reganch) {
	    if (regtry(prog, string))
	    	return(1);
	    for (s=string; *s!='\0' && s!=end; s++)
	    	if (*s == '\n')
	    	    if (regtry(prog, s+1))
	    	    	return(1);
	    return(0);
	}
	
	/* We know what char match must start with. */
	if (prog->regstart != '\0') {
	    for (s=string; *s!='\0' && s!=end; s++)
	    	if (*s == prog->regstart)
	    	    if (regtry(prog, s))
	    	    	return(1);
	    return(0);
	}
	
	/* General case */
	for (s=string; *s!='\0' && s!=end; s++)
	    if (regtry(prog, s))
		return(1);

	/* Failure. */
	return(0);
    }
    
    /* Search reverse, same as forward, but loops run backward */
	
    /* If search is anchored at BOL */
    if (prog->reganch) {
	for (s=end-1; s>=string; s--)
	    if (*s == '\n')
	    	if (regtry(prog, s+1))
	    	    return(1);
	if (regtry(prog, string))
	    return(1);
	return(0);
    }

    /* We know what char match must start with. */
    if (prog->regstart != '\0') {
	for (s=end; s>=string; s--)
	    if (*s == prog->regstart)
	    	if (regtry(prog, s))
	    	    return(1);
	return(0);
    }

    /* General case */
    for (s=end; s>=string; s--)
	if (regtry(prog, s))
	    return(1);

    /* Failure. */
    return(0);
}

/*
 - regtry - try match at specific point, returns: 0 failure, 1 success
 */
static int regtry(regexp *prog, char *string)
{
    register int i;
    register char **sp;
    register char **ep;

    reginput = string;
    regstartp = prog->startp;
    regendp = prog->endp;

    sp = prog->startp;
    ep = prog->endp;
    for (i = NSUBEXP; i > 0; i--) {
	*sp++ = NULL;
	*ep++ = NULL;
    }
    if (regmatch(prog->program + 1)) {
	prog->startp[0] = string;
	prog->endp[0] = reginput;
	return(1);
    } else
	return(0);
}

/*
 - regmatch - main matching routine
 *
 * Conceptually the strategy is simple:	 check to see whether the current
 * node matches, call self recursively to see whether the rest matches,
 * and then act accordingly.  In practice we make some effort to avoid
 * recursion, in particular by going through "ordinary" nodes (that don't
 * need to know whether the rest of the match failed) by a loop instead of
 * by recursion.  Returns 0 failure, 1 success
 */
static int regmatch(char *prog)
{
    register char *scan;    /* Current node. */
    char *next;	    /* Next node. */

    scan = prog;
    while (scan != NULL) {
	next = regnext(scan);

	switch (OP(scan)) {
	case BOL:
	    if (reginput == startofstring) {
	    	if (stringstartsline)
	    	    break;
	    } else if (*(reginput-1) == '\n')
	    	break;
	    return(0);
	case EOL:
	    if (*reginput != '\n' && *reginput != '\0')
		return(0);
	    break;
	case BOWORD:
	    if (reginput == startofstring) {
	    	if (stringstartsword)
	    	    break;
	    	else
	    	    return(0);
	    }
	    if (!isdelimiter[(unsigned char)*(reginput-1)])
	    	return(0);
	    break;
	case EOWORD:
	    if (!isdelimiter[(unsigned char)*reginput])
	    	return(0);
	    break;
	case ANY:
	    if (*reginput == '\0' || *reginput == '\n')
		return(0);
	    reginput++;
	    break;
	case EXACTLY: {
		register int len;
		register char *opnd;

		opnd = OPERAND(scan);
		/* Inline the first character, for speed. */
		if (*opnd != *reginput)
		    return(0);
		len = strlen(opnd);
		if (len > 1 && strncmp(opnd, reginput, len) != 0)
		    return(0);
		reginput += len;
	    }
	    break;
	case ANYOF:
	    if (strchr(OPERAND(scan), *reginput) == NULL)
		return(0);
	    if (*reginput == '\0')
	    	return(0);
	    reginput++;
	    break;
	case ANYBUT:
	    if (strchr(OPERAND(scan), *reginput) != NULL || *reginput == '\n')
		return(0);
	    reginput++;
	    break;
	case NOTHING:
	    break;
	case BACK:
	    break;
	case BRANCH: {
		register char *save;

		if (OP(next) != BRANCH)	    /* No choice. */
		    next = OPERAND(scan);   /* Avoid recursion. */
		else {
		    do {
			save = reginput;
			if (regmatch(OPERAND(scan)))
			    return(1);
			reginput = save;
			scan = regnext(scan);
		    } while (scan != NULL && OP(scan) == BRANCH);
		    return(0);
		    /* NOTREACHED */
		}
	    }
	    break;
	case STAR:
	case PLUS: {
		register char nextch;
		register int no;
		register char *save;
		register int min;

		/*
		 * Lookahead to avoid useless match attempts
		 * when we know what character comes next.
		 */
		nextch = '\0';
		if (OP(next) == EXACTLY)
		    nextch = *OPERAND(next);
		min = (OP(scan) == STAR) ? 0 : 1;
		save = reginput;
		no = regrepeat(OPERAND(scan));
		while (no >= min) {
		    /* If it could work, try it. */
		    if (nextch == '\0' || *reginput == nextch)
			if (regmatch(next))
			    return(1);
		    /* Couldn't or didn't -- back up. */
		    no--;
		    reginput = save + no;
		}
		return(0);
	    }
	    break;
	case END:
	    return(1);	/* Success! */
	    break;
	default:
	    if ((OP(scan) > OPEN) && (OP(scan) < OPEN + NSUBEXP)) {
		register int no;
		register char *save;

		no = OP(scan) - OPEN;
		save = reginput;

		if (regmatch(next)) {
		    /*
		     * Don't set startp if some later
		     * invocation of the same parentheses
		     * already has.
		     */
		    if (regstartp[no] == NULL)
			regstartp[no] = save;
		    return(1);
		} else
		    return(0);
	    } else if ((OP(scan) > CLOSE) && (OP(scan) < CLOSE + NSUBEXP)) {
		register int no;
		register char *save;

		no = OP(scan) - CLOSE;
		save = reginput;

		if (regmatch(next)) {
		    /*
		     * Don't set endp if some later
		     * invocation of the same parentheses
		     * already has.
		     */
		    if (regendp[no] == NULL)
			regendp[no] = save;
		    return(1);
		} else
		    return(0);
	    } else {
		regerror("memory corruption");
		return(0);
    	    }
	    break;
	}

	scan = next;
    }

    /*
     * We get here only if there's trouble -- normally "case END" is
     * the terminating point.
     */
    regerror("corrupted pointers");
    return(0);
}

/*
 - regrepeat - repeatedly match something simple, report how many
 */
static int regrepeat(char *p)
{
    register int count = 0;
    register char *scan;
    register char *opnd;

    scan = reginput;
    opnd = OPERAND(p);
    switch (OP(p)) {
    case ANY:
	while (*scan != '\0' && *scan != '\n') {
	    count++;
	    scan++;
	}
	break;
    case EXACTLY:
	while (*opnd == *scan) {
	    count++;
	    scan++;
	}
	break;
    case ANYOF:
	while (*scan != '\0' && strchr(opnd, *scan) != NULL) {
	    count++;
	    scan++;
	}
	break;
    case ANYBUT:
	while (*scan != '\0' && *scan != '\n' && strchr(opnd, *scan) == NULL) {
	    count++;
	    scan++;
	}
	break;
    default:	    /* Oh dear.	 Called inappropriately. */
	regerror("internal foulup");
	count = 0;  /* Best compromise. */
	break;
    }
    reginput = scan;

    return(count);
}

/*
 - regnext - dig the "next" pointer out of a node
 */
static char *regnext(char *p)
{
    register int offset;

    if (p == &regdummy)
	return(NULL);

    offset = NEXT(p);
    if (offset == 0)
	return(NULL);

    if (OP(p) == BACK)
	return(p-offset);
    else
	return(p+offset);
}

/*
 - SubstituteRE - perform substitutions after a regexp match
 */
void SubstituteRE(regexp *prog, char *source, char *dest, int max)
{
    register char *src;
    register char *dst;
    register char c;
    register int no;
    register int len;
    register char chgcase;

    if (prog == NULL || source == NULL || dest == NULL) {
	regerror("NULL parm to SubstituteRE");
	return;
    }
    if (UCHARAT(prog->program) != MAGIC) {
	regerror("damaged regexp fed to SubstituteRE");
	return;
    }

    src = source;
    dst = dest;
    while ((c = *src++) != '\0') {
	 if (c == '\\' && (*src=='u' || *src=='U' || *src=='l' || *src=='L')) {
	    chgcase = *src;
	    src++;
	    c = *src++;
	    if (c == '\0')
	    	break;
	} else
	    chgcase = '\0';
	if (c == '&')
	    no = 0;
	else if (c == '\\' && '0' <= *src && *src <= '9')
	    no = *src++ - '0';
	else if (c == '\\' && regescape(*src) != '\0') {
	    c = regescape(*src++);
	    no = -1;
	} else if (c == '\\') {
	    c = *src++;			/* allow any escape sequence */
	    no = -1;
	} else
	    no = -1;

	if (no < 0) { /* Ordinary character. */
	    if (dst - dest >= max-1)
	    	break;
	    else
	    	*dst++ = c;
	} else if (prog->startp[no] != NULL && prog->endp[no] != NULL) {
	    len = prog->endp[no] - prog->startp[no];
	    if (dst + len - dest >= max-1)
	    	len = max - 1 - (dst - dest);
	    (void) strncpy(dst, prog->startp[no], len);
	    adjustcase(dst, len, chgcase);
	    dst += len;
	    if (len != 0 && *(dst-1) == '\0') {     /* strncpy hit NUL. */
		regerror("damaged match string");
		return;
	    }
	}
    }
    *dst++ = '\0';
}

static void adjustcase(char *str, int len, char chgcase)
{
    int i;
    
    if (chgcase == 'u')
    	str[0] = toupper((unsigned char)str[0]);
    else if (chgcase == 'U')
    	for (i=0; i<len; i++)
    	    str[i] = toupper((unsigned char)str[i]);
    else if (chgcase == 'l')
    	str[0] = tolower((unsigned char)str[0]);
    else if (chgcase == 'L')
    	for (i=0; i<len; i++)
    	    str[i] = tolower((unsigned char)str[i]);
}

static void regerror(char *s)
{
    fprintf(stderr,
	    "NEdit: Internal error processing regular expression (%s)\n", s);
}

void SetREDefaultWordDelimiters(char *delimiters)
{
    makeDelimiterTable(delimiters, defaultdelimiters);
}

/*
** Translate a null-terminated string of delimiters into a 256 byte lookup
** table for determining whether a character is a delimiter or not.  Table
** must be allocated by the caller.  Return value is a pointer to the table.
*/
static char *makeDelimiterTable(char *delimiters, char *table)
{
    unsigned char *c;
    
    memset(table, 0, 256);
    for (c=(unsigned char*)delimiters; *c!='\0'; c++)
    	table[*c] = 1;
    table[0] = 1;
    table[' '] = 1;
    table['\t'] = 1;
    table['\n'] = 1;
    return table;
}
