/* $Id: parse.h,v 1.5 2002/07/11 21:18:10 slobasso Exp $ */

#ifndef NEDIT_PARSE_H_INCLUDED
#define NEDIT_PARSE_H_INCLUDED

#include "interpret.h"

Program *ParseMacro(char *expr, char **msg, char **stoppedAt);

#endif /* NEDIT_PARSE_H_INCLUDED */
