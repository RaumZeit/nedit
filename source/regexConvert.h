/* $Id: regexConvert.h,v 1.4 2002/07/11 21:18:10 slobasso Exp $ */

#ifndef NEDIT_REGEXCONVERT_H_INCLUDED
#define NEDIT_REGEXCONVERT_H_INCLUDED

char *ConvertRE(const char *exp, char **errorText, char *cap_parens);
void ConvertSubstituteRE(const char *source, char *dest, int max);

#endif /* NEDIT_REGEXCONVERT_H_INCLUDED */
