/*******************************************************************************
*									       *
* help.h -- Nirvana Editor help display					       *
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
* September 10, 1991							       *
*									       *
* Written by Mark Edel							       *
*									       *
*******************************************************************************/
enum HelpTopic {HELP_VERSION, HELP_START, HELP_SEARCH, HELP_SELECT,
	HELP_CLIPBOARD, HELP_INDENT, HELP_TABS, HELP_PROGRAMMER, HELP_TAGS,
	HELP_MOUSE, HELP_KEYBOARD, HELP_FILL, HELP_SYNTAX, HELP_RECOVERY,
	HELP_PREFERENCES, HELP_SHELL, HELP_REGEX, HELP_COMMAND_LINE,
	HELP_SERVER, HELP_CUSTOMIZE, HELP_RESOURCES, HELP_BINDING, HELP_LEARN,
	HELP_MACRO_LANG, HELP_MACRO_SUBRS, HELP_ACTIONS, HELP_PATTERNS,
	HELP_SMART_INDENT, HELP_BUGS, HELP_MAILING_LIST, HELP_DISTRIBUTION,
	HELP_TABS_DIALOG};
#define NUM_TOPICS 32

void Help(Widget parent, enum HelpTopic topic);
