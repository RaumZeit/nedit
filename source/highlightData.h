/*******************************************************************************
*									       *
* highlightData.h -- Maintain, and allow user to edit, highlight pattern list  *
*		     used for syntax highlighting			       *
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
* April, 1997								       *
*									       *
* Written by Mark Edel							       *
*									       *
*******************************************************************************/

patternSet *FindPatternSet(char *langModeName);
int LoadHighlightString(char *inString);
char *WriteHighlightString(void);
int LoadStylesString(char *inString);
char *WriteStylesString(void);
void EditHighlightStyles(Widget parent, char *initialStyle);
void EditHighlightPatterns(WindowInfo *window);
void UpdateLanguageModeMenu(void);
int LMHasHighlightPatterns(char *languageMode);
XFontStruct *FontOfNamedStyle(WindowInfo *window, char *styleName);
char *ColorOfNamedStyle(char *styleName);
int NamedStyleExists(char *styleName);
void RenameHighlightPattern(char *oldName, char *newName);
