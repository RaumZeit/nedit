enum ShiftDirection {SHIFT_LEFT, SHIFT_RIGHT};

void ShiftSelection(WindowInfo *window, int direction, int byTab);
void UpcaseSelection(WindowInfo *window);
void DowncaseSelection(WindowInfo *window);
void FillSelection(WindowInfo *window);
char *ShiftText(char *text, int direction, int tabsAllowed, int tabDist,
	int nChars, int *newLen);
