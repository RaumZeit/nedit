/*******************************************************************************
*									       *
* search.h -- Nirvana Editor search and replace functions		       *
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
enum SearchType {SEARCH_LITERAL, SEARCH_CASE_SENSE, SEARCH_REGEX};
enum SearchDirection {SEARCH_FORWARD, SEARCH_BACKWARD};

void DoReplaceDlog(WindowInfo *window, int direction);
void DoFindDlog(WindowInfo *window, int direction);
int SearchAndSelect(WindowInfo *window, int direction, char *searchString,
	int searchType);
int SearchAndSelectSame(WindowInfo *window, int direction);
void SearchForSelected(WindowInfo *window, int direction, Time time);
int SearchAndReplace(WindowInfo *window, int direction, char *searchString,
	char *replaceString, int searchType);
int ReplaceSame(WindowInfo *window, int direction);
int ReplaceAll(WindowInfo *window, char *searchString, char *replaceString,
	int searchType);
int ReplaceInSelection(WindowInfo *window, char *searchString,
	char *replaceString, int searchType);
int SearchWindow(WindowInfo *window, int direction, char *searchString,
	int searchType, int beginPos, int *startPos, int *endPos);
int SearchString(char *string, char *searchString, int direction,
	int searchType, int wrap, int beginPos, int *startPos,
	int *endPos, char *delimiters);
char *ReplaceAllInString(char *inString, char *searchString,
	char *replaceString, int searchType, int *copyStart,
	int *copyEnd, int *replacementLength, char *delimiters);
void FlashMatching(WindowInfo *window, Widget textW);
void MatchSelectedCharacter(WindowInfo *window);
