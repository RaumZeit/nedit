/*******************************************************************************
*									       *
* smartIndent.h -- Maintain, and allow user to edit, macros for smart indent   *
*									       *
* Copyright (c) 1997 Universities Research Association, Inc.		       *
* All rights reserved.							       *
* 									       *
* This material resulted from work developed under a Government Contract and   *
* is subject to the following license:  The Government retains a paid-up,      *
* nonexclusive, irrevocable worldwide license to reproduce, prepare derivative *
* works, perform publicly and display publicly by or for the Government,       *
* including the right to distribute to other Government contractors.  Neither  *
* the United States nor the United States Department of Energy, nor any of     *
* their employees, makes any warrenty, express or implied, or assumes any      *
* legal liability or responsibility for the accuracy, completeness, or         *
* usefulness of any information, apparatus, product, or process disclosed, or  *
* represents that its use would not infringe privately owned rights.           *
*                                        				       *
* Fermilab Nirvana GUI Library						       *
* July, 1997								       *
*									       *
* Written by Mark Edel							       *
*									       *
*******************************************************************************/
void BeginSmartIndent(WindowInfo *window, int warn);
void EndSmartIndent(WindowInfo *window);
void SmartIndentCB(Widget w, XtPointer clientData, XtPointer callData);
int LoadSmartIndentString(char *inString);
int LoadSmartIndentCommonString(char *inString);
char *WriteSmartIndentString(void);
char *WriteSmartIndentCommonString(void);
int SmartIndentMacrosAvailable(char *languageMode);
void EditSmartIndentMacros(WindowInfo *window);
void EditCommonSmartIndentMacro(void);
