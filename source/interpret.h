/* $Id: interpret.h,v 1.14 2003/12/19 23:23:30 slobasso Exp $ */

#ifndef NEDIT_INTERPRET_H_INCLUDED
#define NEDIT_INTERPRET_H_INCLUDED

#include "nedit.h"
#include "rbTree.h"

#define MAX_ARGS 9  	    	/* Maximum number of subroutine arguments */
#define STACK_SIZE 1024		/* Maximum stack size */
#define MAX_SYM_LEN 100 	/* Max. symbol name length */
#define MACRO_EVENT_MARKER 2 	/* Special value for the send_event field of
    	    	    	    	   events passed to action routines.  Tells
    	    	    	    	   them that they were called from a macro */

enum symTypes {CONST_SYM, GLOBAL_SYM, LOCAL_SYM, ARG_SYM, PROC_VALUE_SYM,
    	C_FUNCTION_SYM, MACRO_FUNCTION_SYM, ACTION_ROUTINE_SYM};
#define N_OPS 43
enum operations {OP_RETURN_NO_VAL, OP_RETURN, OP_PUSH_SYM, OP_DUP, OP_ADD,
    OP_SUB, OP_MUL, OP_DIV, OP_MOD, OP_NEGATE, OP_INCR, OP_DECR, OP_GT, OP_LT,
    OP_GE, OP_LE, OP_EQ, OP_NE, OP_BIT_AND, OP_BIT_OR, OP_AND, OP_OR, OP_NOT,
    OP_POWER, OP_CONCAT, OP_ASSIGN, OP_SUBR_CALL, OP_FETCH_RET_VAL, OP_BRANCH,
    OP_BRANCH_TRUE, OP_BRANCH_FALSE, OP_BRANCH_NEVER, OP_ARRAY_REF,
    OP_ARRAY_ASSIGN, OP_BEGIN_ARRAY_ITER, OP_ARRAY_ITER, OP_IN_ARRAY,
    OP_ARRAY_DELETE, OP_PUSH_ARRAY_SYM, OP_ARRAY_REF_ASSIGN_SETUP, OP_PUSH_ARG,
    OP_PUSH_ARG_COUNT, OP_PUSH_ARG_ARRAY};

enum typeTags {NO_TAG, INT_TAG, STRING_TAG, ARRAY_TAG};

enum execReturnCodes {MACRO_TIME_LIMIT, MACRO_PREEMPT, MACRO_DONE, MACRO_ERROR};

#define ARRAY_DIM_SEP "\034"

struct DataValueTag;
struct ProgramTag;

typedef int (*Inst)(void);

typedef int (*BuiltInSubr)(WindowInfo *window, struct DataValueTag *argList, 
        int nArgs, struct DataValueTag *result, char **errMsg);

typedef struct DataValueTag {
    enum typeTags tag;
    union {
        int n;
        char *str;
        BuiltInSubr subr;
        struct ProgramTag* prog;
        XtActionProc xtproc;
        Inst* inst;
        struct DataValueTag* dataval;
        struct SparseArrayEntry *arrayPtr;
    } val;
} DataValue;

typedef struct {
    rbTreeNode nodePtrs; /* MUST BE FIRST ENTRY */
    char *key;
    DataValue value;
} SparseArrayEntry;

/* symbol table entry */
typedef struct SymbolRec {
    char *name;
    enum symTypes type;
    DataValue value;
    struct SymbolRec *next;     /* to link to another */  
} Symbol;

typedef struct ProgramTag {
    Symbol *localSymList;
    Inst *code;
} Program;

/* Information needed to re-start a preempted macro */
typedef struct {
    DataValue *stack;
    DataValue *stackP;
    DataValue *frameP;
    Inst *pc;
    WindowInfo *runWindow;
    WindowInfo *focusWindow;
} RestartData;

void InitMacroGlobals(void);

SparseArrayEntry *arrayIterateFirst(DataValue *theArray);
SparseArrayEntry *arrayIterateNext(SparseArrayEntry *iterator);
struct SparseArrayEntry *ArrayNew(void);
int ArrayInsert(DataValue *theArray, char *keyStr, DataValue *theValue);
void ArrayDelete(DataValue *theArray, char *keyStr);
void ArrayDeleteAll(DataValue *theArray);
int ArraySize(DataValue *theArray);
int ArrayGet(DataValue *theArray, char *keyStr, DataValue *theValue);
int ArrayCopy(DataValue *dstArray, DataValue *srcArray);

/* Routines for creating a program, (accumulated beginning with
   BeginCreatingProgram and returned via FinishCreatingProgram) */
void BeginCreatingProgram(void);
int AddOp(int op, char **msg);
int AddSym(Symbol *sym, char **msg);
int AddImmediate(void *value, char **msg);
int AddBranchOffset(Inst *to, char **msg);
Inst *GetPC(void);
Symbol *InstallIteratorSymbol();
Symbol *LookupStringConstSymbol(const char *value);
Symbol *InstallStringConstSymbol(const char *str);
Symbol *LookupSymbol(const char *name);
Symbol *InstallSymbol(const char *name, enum symTypes type, DataValue value);
Program *FinishCreatingProgram(void);
void SwapCode(Inst *start, Inst *boundary, Inst *end);
void StartLoopAddrList(void);
int AddBreakAddr(Inst *addr);
int AddContinueAddr(Inst *addr);
void FillLoopAddrs(Inst *breakAddr, Inst *continueAddr);

/* create a permanently allocated static string (only for use with static strings) */
#define PERM_ALLOC_STR(xStr) (((char *)("\001" xStr)) + 1)

/* Routines for executing programs */
int ExecuteMacro(WindowInfo *window, Program *prog, int nArgs, DataValue *args,
    	DataValue *result, RestartData **continuation, char **msg);
int ContinueMacro(RestartData *continuation, DataValue *result, char **msg);
void RunMacroAsSubrCall(Program *prog);
void PreemptMacro(void);
char *AllocString(int length);
char *AllocStringNCpy(const char *s, int length);
char *AllocStringCpy(const char *s);
void GarbageCollectStrings(void);
void FreeRestartData(RestartData *context);
Symbol *PromoteToGlobal(Symbol *sym);
void FreeProgram(Program *prog);
void ModifyReturnedValue(RestartData *context, DataValue dv);
WindowInfo *MacroRunWindow(void);
WindowInfo *MacroFocusWindow(void);
void SetMacroFocusWindow(WindowInfo *window);
/* function used for implicit conversion from string to number */
int StringToNum(const char *string, int *number);

#endif /* NEDIT_INTERPRET_H_INCLUDED */
