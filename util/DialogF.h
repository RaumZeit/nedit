/*
 *  To use DialogF, #include <stdarg.h>
 */
 
#define DF_ERR 1			/* Error Dialog       */
#define DF_INF 2			/* Information Dialog */
#define DF_MSG 3			/* Message Dialog     */
#define DF_QUES 4			/* Question Dialog    */
#define DF_WARN 5			/* Warning Dialog     */
#define DF_PROMPT 6			/* Prompt Dialog      */

#define DF_MAX_MSG_LENGTH 2047		/* longest message length supported */
#define DF_MAX_PROMPT_LENGTH 255	/* longest prompt string supported */

unsigned DialogF(unsigned, Widget, unsigned, char*, ...);
				/* variable # arguments */
void SetDialogFPromptHistory(char **historyList, int nItems);
