/* $Id: regexConvert.h,v 1.5 2003/05/09 17:43:47 edg Exp $ */

#ifndef NEDIT_REGEXCONVERT_H_INCLUDED
#define NEDIT_REGEXCONVERT_H_INCLUDED

char *ConvertRE(const char *exp, char **errorText);
void ConvertSubstituteRE(const char *source, char *dest, int max);

#endif /* NEDIT_REGEXCONVERT_H_INCLUDED */
