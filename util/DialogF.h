/* $Id: DialogF.h,v 1.9 2003/04/24 11:47:24 edg Exp $ */

#ifndef NEDIT_DIALOGF_H_INCLUDED
#define NEDIT_DIALOGF_H_INCLUDED

#include <X11/Intrinsic.h>
#ifdef VMS
#include "../util/VMSparam.h"
#else
#ifndef __MVS__
#include <sys/param.h>
#endif
#endif /*VMS*/

#define DF_ERR 1        /* Error Dialog       */
#define DF_INF 2        /* Information Dialog */
#define DF_MSG 3        /* Message Dialog     */
#define DF_QUES 4       /* Question Dialog    */
#define DF_WARN 5       /* Warning Dialog     */
#define DF_PROMPT 6     /* Prompt Dialog      */

/* longest message length supported. Note that dialogs may contains a file 
   name, which is only limited in length by MAXPATHLEN, so DF_MAX_MSG_LENGTH
   must be sufficiently larger than MAXPATHLEN. */
#define DF_MAX_MSG_LENGTH (2047 + MAXPATHLEN) 
#define DF_MAX_PROMPT_LENGTH 255            /* longest prompt string supported  */

unsigned DialogF(int dialog_type, Widget parent, unsigned n, const char* title,
        const char* msgstr, ...);                    /* variable # arguments */
void SetDialogFPromptHistory(char **historyList, int nItems);

#endif /* NEDIT_DIALOGF_H_INCLUDED */
