/*******************************************************************************
*									       *
* selection.h - (some) Nirvana Editor commands operating the primary selection *
*									       *
* Copyright (c) 1991 Universities Research Association, Inc.		       *
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
* May 10, 1991								       *
*									       *
* Written by Mark Edel							       *
*									       *
*******************************************************************************/
void GotoSelectedLineNumber(WindowInfo *window, Time time);
void GotoLineNumber(WindowInfo *window);
void SelectNumberedLine(WindowInfo *window, int lineNum);
void OpenSelectedFile(WindowInfo *window, Time time);
char *GetAnySelection(WindowInfo *window);
void BeginMarkCommand(WindowInfo *window);
void BeginGotoMarkCommand(WindowInfo *window, int extend);
void AddMark(WindowInfo *window, Widget widget, char label);
void UpdateMarkTable(WindowInfo *window, int pos, int nInserted,
   	int nDeleted);
void GotoMark(WindowInfo *window, Widget w, char label, int extendSel);
void MarkDialog(WindowInfo *window);
void GotoMarkDialog(WindowInfo *window, int extend);
