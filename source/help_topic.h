/*******************************************************************************
*                                                                              *
* help_topic.h --  Nirvana Editor help display                                 *
*                                                                              *
                 Generated on Jul 9, 2002 (Do NOT edit!)
                 Source of content from file help.etx
*                                                                              *
* Copyright (c) 1999-2002 Mark Edel                                            *
*                                                                              *
* This is free software; you can redistribute it and/or modify it under the    *
* terms of the GNU General Public License as published by the Free Software    *
* Foundation; either version 2 of the License, or (at your option) any later   *
* version.                                                                     *
*                                                                              *
* This software is distributed in the hope that it will be useful, but WITHOUT *
* ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or        *
* FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License        *
* for more details.                                                            *
*                                                                              *
* You should have received a copy of the GNU General Public License along with *
* software; if not, write to the Free Software Foundation, Inc., 59 Temple     *
* Place, Suite 330, Boston, MA  02111-1307 USA                                 *
*                                                                              *
* Nirvana Text Editor                                                          *
* September 10, 1991                                                           *
*                                                                              *
* Written by Mark Edel                                                         *
*                                                                              *
*******************************************************************************/

#define MAX_HEADING   3
#define STL_HD        16+1
#define STL_LINK      16
#define STL_NM_HEADER 'R'
#define STL_NM_LINK   'Q'
#define STYLE_MARKER  '\01'
#define STYLE_PLAIN   'A'
#define TKN_LIST_SIZE 4

enum HelpTopic {
    HELP_START,
    HELP_SELECT,
    HELP_SEARCH,
    HELP_CLIPBOARD,
    HELP_MOUSE,
    HELP_KEYBOARD,
    HELP_FILL,
    HELP_FORMAT,
    HELP_PROGRAMMER,
    HELP_TABS,
    HELP_INDENT,
    HELP_SYNTAX,
    HELP_TAGS,
    HELP_BASICSYNTAX,
    HELP_ESCAPESEQUENCES,
    HELP_PARENCONSTRUCTS,
    HELP_ADVANCEDTOPICS,
    HELP_EXAMPLES,
    HELP_SHELL,
    HELP_LEARN,
    HELP_MACRO_LANG,
    HELP_MACRO_SUBRS,
    HELP_ACTIONS,
    HELP_CUSTOMIZE,
    HELP_PREFERENCES,
    HELP_RESOURCES,
    HELP_BINDING,
    HELP_PATTERNS,
    HELP_SMART_INDENT,
    HELP_COMMAND_LINE,
    HELP_SERVER,
    HELP_RECOVERY,
    HELP_VERSION,
    HELP_DISTRIBUTION,
    HELP_MAILING_LIST,
    HELP_DEFECTS,
    HELP_TABS_DIALOG,
    HELP_CUSTOM_TITLE_DIALOG,
    HELP_LAST_ENTRY,
    HELP_none = 0x7fffffff  /* Illegal topic */ 
};

#define NUM_TOPICS HELP_LAST_ENTRY

