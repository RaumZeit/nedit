/* $Id: vmsUtils.h,v 1.4 2002/06/26 23:39:21 slobasso Exp $ */

#ifndef VMSUTILS_H_INCLUDED
#define VMSUTILS_H_INCLUDED

#ifdef VMS
#ifndef __DESCRIP_LOADED
#include descrip
#endif /*__DESCRIP_LOADED*/

#define INCLUDE_FNF 0
#define EXCLUDE_FNF 1
#define NOT_ERR_FNF 2

char *StrDescToNul(struct dsc$descriptor_s *vmsString);
struct dsc$descriptor_s *NulStrToDesc(char *nulTString);
struct dsc$descriptor_s *NulStrWrtDesc(char *nulTString, int strLen);
void FreeNulStr(char *nulTString);
void FreeStrDesc(struct dsc$descriptor_s *vmsString);
double rint(double dnum);
void ConvertVMSCommandLine(int *argc, char **argv[]);
int VMSFileScan(char *dirname, char *(*namelist[]), int (*select)(), int fnf);
void VMSFileScanDone(void);
int ProcAlive(const unsigned int pID);

#endif /*VMS*/

#endif /* VMSUTILS_H_INCLUDED */
