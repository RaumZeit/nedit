/* $Id: DialogF.h,v 1.8 2003/04/07 22:51:41 yooden Exp $ */

#ifndef NEDIT_DIALOGF_H_INCLUDED
#define NEDIT_DIALOGF_H_INCLUDED

#include <X11/Intrinsic.h>

#define DF_ERR 1        /* Error Dialog       */
#define DF_INF 2        /* Information Dialog */
#define DF_MSG 3        /* Message Dialog     */
#define DF_QUES 4       /* Question Dialog    */
#define DF_WARN 5       /* Warning Dialog     */
#define DF_PROMPT 6     /* Prompt Dialog      */

#define DF_MAX_MSG_LENGTH 2047      /* longest message length supported */
#define DF_MAX_PROMPT_LENGTH 255    /* longest prompt string supported  */

unsigned DialogF(int dialog_type, Widget parent, unsigned n, const char* title,
        const char* msgstr, ...);                    /* variable # arguments */
void SetDialogFPromptHistory(char **historyList, int nItems);

#endif /* NEDIT_DIALOGF_H_INCLUDED */
