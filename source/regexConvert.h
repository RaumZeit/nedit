/* $Id: regexConvert.h,v 1.3 2001/08/25 15:58:54 amai Exp $ */
char * ConvertRE (const char *exp, char **errorText, char *cap_parens);
void ConvertSubstituteRE (const char *source, char *dest, int max);
