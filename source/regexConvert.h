/* $Id: regexConvert.h,v 1.2 2001/02/26 23:38:03 edg Exp $ */
char * ConvertRE (char *exp, char **errorText, char *cap_parens);
void ConvertSubstituteRE (char *source, char *dest, int max);
