/* $Id: search.h,v 1.6 2001/03/10 15:37:08 arnef Exp $ */
enum SearchType {SEARCH_LITERAL, SEARCH_CASE_SENSE, SEARCH_REGEX};
enum SearchDirection {SEARCH_FORWARD, SEARCH_BACKWARD};

void DoFindReplaceDlog(WindowInfo *window, int direction, int searchType,
    int keepDialogs, Time time);
void DoFindDlog(WindowInfo *window, int direction, int searchType,
    int keepDialogs, Time time);
int SearchAndSelect(WindowInfo *window, int direction, char *searchString,
	int searchType, int searchWrap);
int SearchAndSelectSame(WindowInfo *window, int direction, int searchWrap);
int SearchAndSelectIncremental(WindowInfo *window, int direction,
	char *searchString, int searchType, int searchWrap, int continued);
void SearchForSelected(WindowInfo *window, int direction, int searchWrap,
    int searchType, Time time);
int SearchAndReplace(WindowInfo *window, int direction, char *searchString,
	char *replaceString, int searchType, int searchWrap);
int ReplaceAndSearch(WindowInfo *window, int direction, char *searchString,
	char *replaceString, int searchType, int searchWrap);
int ReplaceFindSame(WindowInfo *window, int direction, int searchWrap);
int ReplaceSame(WindowInfo *window, int direction, int searchWrap);
int ReplaceAll(WindowInfo *window, char *searchString, char *replaceString,
	int searchType);
int ReplaceInSelection(WindowInfo *window, char *searchString,
	char *replaceString, int searchType);
int SearchWindow(WindowInfo *window, int direction, char *searchString,
	int searchType, int searchWrap, int beginPos, int *startPos, int *endPos, int *extent);
int SearchString(char *string, char *searchString, int direction,
       int searchType, int wrap, int beginPos, int *startPos, int *endPos,
       int *searchExtent, char *delimiters);
char *ReplaceAllInString(char *inString, char *searchString,
	char *replaceString, int searchType, int *copyStart,
	int *copyEnd, int *replacementLength, char *delimiters);
void BeginISearch(WindowInfo *window, int direction);
void EndISearch(WindowInfo *window);
void SetISearchTextCallbacks(WindowInfo *window);
void FlashMatching(WindowInfo *window, Widget textW);
void SelectToMatchingCharacter(WindowInfo *window);
void GotoMatchingCharacter(WindowInfo *window);
void RemoveFromMultiFileReplaceDialogLists(WindowInfo *window);
