/*******************************************************************************
*                                                                              *
*     FontSel ()                                                               *
*                                                                              *
*                                                                              *
*            Function to put up a modal font selection dialog box. The purpose *
*            of this routine is to allow the user to interactively view sample *
*            fonts and to choose a font for current use.                       *
*                                                                              *
*     Arguments:                                                               *
*                                                                              *
*            Widget	parent 		- parent widget ID                     *
*                                                                              *
*            int        showPropFont    - ONLY_FIXED : shows only fixed fonts  *
*                                                      doesn't show prop font  *
*                                                      toggle button also.     *
*                                         PREF_FIXED : can select either fixed *
*                                                      or proportional fonts;  *
*                                                      but starting option is  *
*                                                      Fixed fonts.            *
*                                         PREF_PROP  : can select either fixed *
*                                                      or proportional fonts;  *
*                                                      but starting option is  *
*                                                      proportional fonts.     *
*                                                                              *
*           char *	currFont        - ASCII string that contains the name  *
*                                         of the currently selected font.      *
*                                                                              *
*     Returns:                                                                 *
*                                                                              *
*           pointer to an ASCII character string that contains the name of     *
*           the selected font (in X format for naming fonts); it is the users  *
*           responsibility to free the space allocated to this string.         *
*                                                                              *
*     Comments:                                                                *
*                                                                              *
*           The calling function has to call the appropriate routines to set   *
*           the current font to the one represented by the returned string.    *
*                                                                              *
*******************************************************************************/

/* constant values for controlling the proportional font toggle */

#define ONLY_FIXED	0
#define PREF_FIXED	1
#define PREF_PROP	2


/* function prototype */

char    *FontSel(Widget parent, int showPropFont, char *currFont);
