/*******************************************************************************
*									       *
* highlightData.c -- Maintain, and allow user to edit, highlight pattern list  *
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
#include <stdio.h>
#include <limits.h>
#ifdef VMS
#include "../util/VMSparam.h"
#else
#include <sys/param.h>
#endif /*VMS*/
#include <Xm/Xm.h>
#include <Xm/Form.h>
#include <Xm/Frame.h>
#include <Xm/Text.h>
#include <Xm/LabelG.h>
#include <Xm/PushB.h>
#include <Xm/ToggleB.h>
#include <Xm/RowColumn.h>
#include <Xm/SeparatoG.h>
#include "../util/misc.h"
#include "../util/DialogF.h"
#include "../util/managedList.h"
#include "textBuf.h"
#include "nedit.h"
#include "highlight.h"
#include "regularExp.h"
#include "preferences.h"
#include "help.h"
#include "window.h"
#include "highlightData.h"

/* Maximum allowed number of styles (also limited by representation of
   styles as a byte - 'b') */
#define MAX_HIGHLIGHT_STYLES 128

/* Maximum number of patterns allowed in a pattern set (regular expression
   limitations are probably much more restrictive).  */
#define MAX_PATTERNS 127

/* maximum number of pattern sets allowed (we wish) */
#define MAX_PATTERN_SETS 50

/* Names for the fonts that can be used for syntax highlighting */
#define N_FONT_TYPES 4
enum fontTypes {PLAIN_FONT, ITALIC_FONT, BOLD_FONT, BOLD_ITALIC_FONT};
char *FontTypeNames[N_FONT_TYPES] = {"Plain", "Italic", "Bold", "Bold Italic"};

typedef struct {
    char *name;
    char *color;
    int font;
} highlightStyleRec;

static int styleError(char *stringStart, char *stoppedAt, char *message);
static int lookupNamedStyle(char *styleName);
static highlightPattern *readHighlightPatterns(char **inPtr, int withBraces,
    	char **errMsg, int *nPatterns);
static int readHighlightPattern(char **inPtr, char **errMsg,
    	highlightPattern *pattern);
static patternSet *readDefaultPatternSet(char *langModeName);
static int isDefaultPatternSet(patternSet *patSet);
static patternSet *readPatternSet(char **inPtr);
static patternSet *highlightError(char *stringStart, char *stoppedAt,
    	char *message);
static char *intToStr(int i);
static char *createPatternsString(patternSet *patSet, char *indentStr);
static void setStyleByName(char *style);
static void hsDestroyCB(Widget w, XtPointer clientData, XtPointer callData);
static void hsOkCB(Widget w, XtPointer clientData, XtPointer callData);
static void hsApplyCB(Widget w, XtPointer clientData, XtPointer callData);
static void hsDismissCB(Widget w, XtPointer clientData, XtPointer callData);
static highlightStyleRec *copyHighlightStyleRec(highlightStyleRec *hs);
static void *hsGetDisplayedCB(void *oldItem, int explicitRequest, int *abort,
    	void *cbArg);
static void hsSetDisplayedCB(void *item, void *cbArg);
static highlightStyleRec *readHSDialogFields(int silent);
static void hsFreeItemCB(void *item);
static void freeHighlightStyleRec(highlightStyleRec *hs);
static int hsDialogEmpty(void);
static int updateHSList(void);
static void updateHighlightStyleMenu(void);
static Widget createHighlightStylesMenu(Widget parent);
static void destroyCB(Widget w, XtPointer clientData, XtPointer callData);
static void langModeCB(Widget w, XtPointer clientData, XtPointer callData);
static void lmDialogCB(Widget w, XtPointer clientData, XtPointer callData);
static void styleDialogCB(Widget w, XtPointer clientData, XtPointer callData);
static void patTypeCB(Widget w, XtPointer clientData, XtPointer callData);
static void matchTypeCB(Widget w, XtPointer clientData, XtPointer callData);
static int checkHighlightDialogData(void);
static void updateLabels(void);
static void okCB(Widget w, XtPointer clientData, XtPointer callData);
static void applyCB(Widget w, XtPointer clientData, XtPointer callData);
static void checkCB(Widget w, XtPointer clientData, XtPointer callData);
static void restoreCB(Widget w, XtPointer clientData, XtPointer callData);
static void deleteCB(Widget w, XtPointer clientData, XtPointer callData);
static void dismissCB(Widget w, XtPointer clientData, XtPointer callData);
static void helpCB(Widget w, XtPointer clientData, XtPointer callData);
static void *getDisplayedCB(void *oldItem, int explicitRequest, int *abort,
    	void *cbArg);
static void setDisplayedCB(void *item, void *cbArg);
static void setStyleMenu(char *styleName);
static highlightPattern *readDialogFields(int silent);
static int dialogEmpty(void);
static int updatePatternSet(void);
static patternSet *getDialogPatternSet(void);
static int patternSetsDiffer(patternSet *patSet1, patternSet *patSet2);
static highlightPattern *copyPatternSrc(highlightPattern *pat,
    	highlightPattern *copyTo);
static void freeNonNull(void *ptr);
static void freeItemCB(void *item);
static void freePatternSrc(highlightPattern *pat, int freeStruct);
static void freePatternSet(patternSet *p);

/* list of available highlight styles */
static int NHighlightStyles = 0;
static highlightStyleRec *HighlightStyles[MAX_HIGHLIGHT_STYLES];

/* Highlight styles dialog information */
static struct {
    Widget shell;
    Widget nameW;
    Widget colorW;
    Widget recogW;
    Widget plainW, boldW, italicW, boldItalicW;
    Widget managedListW;
    highlightStyleRec **highlightStyleList;
    int nHighlightStyles;
} HSDialog = {NULL};

/* Highlight dialog information */
static struct {
    Widget shell;
    Widget lmOptMenu;
    Widget lmPulldown;
    Widget styleOptMenu;
    Widget stylePulldown;
    Widget nameW;
    Widget topLevelW;
    Widget deferredW;
    Widget subPatW;
    Widget colorPatW;
    Widget simpleW;
    Widget rangeW;
    Widget parentW;
    Widget startW;
    Widget endW;
    Widget errorW;
    Widget lineContextW;
    Widget charContextW;
    Widget managedListW;
    Widget parentLbl;
    Widget startLbl;
    Widget endLbl;
    Widget errorLbl;
    Widget matchLbl;
    char *langModeName;
    int nPatterns;
    highlightPattern **patterns;
} HighlightDialog = {NULL, NULL, NULL};

/* Pattern sources loaded from the .nedit file or set by the user */
static int NPatternSets = 0;
static patternSet *PatternSets[MAX_PATTERN_SETS];

#ifdef VMS
#define N_DEFAULT_PATTERN_SETS 20
#else
#define N_DEFAULT_PATTERN_SETS 21
#endif /*VMS*/

static char *DefaultPatternSets[N_DEFAULT_PATTERN_SETS] = {
    "C:1:0 {\n\
    	comment:\"/\\*\":\"\\*/\"::Comment::\n\
    	string:\"\"\"\":\"\"\"\":\"\\n\":String::\n\
    	preprocessor line:\"^[ \t]*#\":\"$\"::Preprocessor::\n\
    	string escape chars:\"\\\\(.|\\n)\":::String1:string:\n\
    	preprocessor esc chars:\"\\\\(.|\\n)\":::Preprocessor1:preprocessor line:\n\
    	preprocessor comment:\"/\\*\":\"\\*/\"::Comment:preprocessor line:\n\
    	character constant:\"'\":\"'\":\"[^\\\\][^']\":Character Const::\n\
	numeric constant:\"<((0(x|X)[0-9a-fA-F]*)|(([0-9]+\\.?[0-9]*)|(\\.[0-9]+))((e|E)(\\+|-)?[0-9]+)?)(L|l|UL|ul|u|U|F|f)?>\":::Numeric Const::D\n\
    	storage keyword:\"<(const|extern|auto|register|static|unsigned|signed|volatile|char|double|float|int|long|short|void|typedef|struct|union|enum)>\":::Storage Type::D\n\
    	keyword:\"<(return|goto|if|else|case|default|switch|break|continue|while|do|for|sizeof)>\":::Keyword::D\n\
    	braces:\"[{}]\":::Keyword::D}",
    "C++:1:0 {\n\
    	comment:\"/\\*\":\"\\*/\"::Comment::\n\
    	cplus comment:\"//\":\"$\"::Comment::\n\
    	string:\"\"\"\":\"\"\"\":\"\\n\":String::\n\
    	preprocessor line:\"^[ \t]*#\":\"$\"::Preprocessor::\n\
    	string escape chars:\"\\\\(.|\\n)\":::String1:string:\n\
    	preprocessor esc chars:\"\\\\(.|\\n)\":::Preprocessor1:preprocessor line:\n\
    	preprocessor comment:\"/\\*\":\"\\*/\"::Comment:preprocessor line:\n\
    	preproc cplus comment:\"//\":\"$\"::Comment:preprocessor line:\n\
    	character constant:\"'\":\"'\":\"[^\\\\][^']\":Character Const::\n\
	numeric constant:\"<((0(x|X)[0-9a-fA-F]*)|(([0-9]+\\.?[0-9]*)|(\\.[0-9]+))((e|E)(\\+|-)?[0-9]+)?)(L|l|UL|ul|u|U|F|f)?>\":::Numeric Const::D\n\
    	storage keyword:\"<(class|typename|typeid|template|friend|virtual|inline|explicit|operator|overload|public|private|protected|const|extern|auto|register|static|mutable|unsigned|signed|volatile|char|double|float|int|long|short|bool|wchar_t|void|typedef|struct|union|enum)>\":::Storage Type::D\n\
    	keyword:\"<(new|delete|this|return|goto|if|else|case|default|switch|break|continue|while|do|for|catch|throw|sizeof|true|false|namespace|using|dynamic_cast|static_cast|reinterpret_cast)>\":::Keyword::D\n\
    	braces:\"[{}]\":::Keyword::D}",
    "HTML:1:0 {\n\
	special chars:\"\\&[-.a-zA-Z0-9#]*;?\":::Text Escape::\n\
	comment:\"\\<!--\":\"--\\>\"::Text Comment::\n\
	element:\"(\\<)(/|!)?[-.a-zA-Z0-9]*\":\"\\>\":\"[^-.a-zA-Z0-9 \\t\\n=\"\"'%]\":Text Key::\n\
	double quote string:\"\"\"\":\"\"\"\":\"[<>]\":Text Arg1:element:\n\
	single quote string:\"'\":\"'\":\"[<>]\":Text Arg1:element:\n\
	attribute:\"[^'\"\"]|\\n\":::Text Arg:element:\n\
	brackets:\"\\1\":\"\\0\"::Text Arg:element:C}",
     "Java:1:0 {\n\
	comment:\"/\\*\":\"\\*/\"::Comment::\n\
	cplus comment:\"//\":\"$\"::Comment::\n\
	string:\"\"\"\":\"\"\"\":\"\\n\":String::\n\
	single quoted:\"'\":\"'\":\"[^\\\\][^']\":String::\n\
	numeric const:\"<((0(x|X)[0-9a-fA-F]*)|[0-9.]+((e|E)(\\+|-)?)?[0-9]*)(L|l|UL|ul|u|U|F|f)?>\":::Numeric Const::\n\
	include:\"<(import|package)>\":\"$\"::Preprocessor::\n\
	storage keyword:\"<(abstract|boolean|byte|char|class|double|extends|final|float|int|interface|long|native|private|protected|public|short|static|transient|synchronized|void|volatile|implements)>\":::Storage Type::\n\
	keyword:\"<(break|case|catch|continue|default|do|else|false|finally|for|if|instanceof|new|null|return|super|switch|this|throw|throws|true|try|while)>\":::Keyword::\n\
	braces and parens:\"[{()}]\":::Keyword::\n\
	string escape chars:\"\\\\(.|\\n)\":::String1:string:\n\
	include esc chars:\"\\\\(.|\\n)\":::Preprocessor1:include:\n\
	include comment:\"/\\*\":\"\\*/\"::Comment:include:}",
#ifndef VMS
/* The VAX C compiler cannot compile this definition */
     "JavaScript:1:0{\n\
        DSComment:\"//\":\"$\"::Comment::\n\
        MLComment:\"/\\*\":\"\\*/\"::Comment::\n\
	DQColors:\"aliceblue|antiquewhite|aqua|aquamarine|azure|beige|bisque|black|blanchedalmond|blue|blueviolet|brown|burlywood|cadetblue|chartreuse|chocolate|coral|cornflowerblue|cornsilk|crimson|cyan|darkblue|darkcyan|darkgoldenrod|darkgray|darkgreen|darkkhaki|darkmagenta|darkolivegreen|darkorange|darkorchid|darkred|darksalmon|darkseagreen|darkslateblue|darkslategray|darkturquoise|darkviolet|deeppink|deepskyblue|dimgray|dodgerblue|firebrick|floralwhite|forestgreen|fuchsia|gainsboro|ghostwhite|gold|goldenrod|gray|green|greenyellow|honeydew|hotpink|indianred|indigo|ivory|khaki|lavender|lavenderblush|lawngreen|lemonchiffon|lightblue|lightcoral|lightcyan|lightgoldenrodyellow|lightgreen|lightgrey|lightpink|lightsalmon|lightseagreen|lightskyblue|lightslategray|lightsteelblue|lightyellow|lime|limegreen|linen|magenta|maroon|mediumaquamarine|mediumblue|mediumorchid|mediumpurple|mediumseagreen|mediumslateblue|mediumspringgreen|mediumturquoise|mediumvioletred|midnightblue|mintcream|mistyrose|moccasin|navajowhite|navy|oldlace|olive|olivedrab|orange|orangered|orchid|palegoldenrod|palegreen|paleturquoise|palevioletred|papayawhip|peachpuff|peru|pink|plum|powderblue|purple|red|rosybrown|royalblue|saddlebrown|salmon|sandybrown|seagreen|seashell|sienna|silver|skyblue|slateblue|slategray|snow|springgreen|steelblue|tan|teal|thistle|tomato|turquoise|violet|wheat|white|whitesmoke|yellow|yellowgreen|#[A-Fa-f0-9][A-Fa-f0-9][A-Fa-f0-9][A-Fa-f0-9][A-Fa-f0-9][A-Fa-f0-9]\":::Text Arg1:DQStrings:\n\
	SQColors:\"aliceblue|antiquewhite|aqua|aquamarine|azure|beige|bisque|black|blanchedalmond|blue|blueviolet|brown|burlywood|cadetblue|chartreuse|chocolate|coral|cornflowerblue|cornsilk|crimson|cyan|darkblue|darkcyan|darkgoldenrod|darkgray|darkgreen|darkkhaki|darkmagenta|darkolivegreen|darkorange|darkorchid|darkred|darksalmon|darkseagreen|darkslateblue|darkslategray|darkturquoise|darkviolet|deeppink|deepskyblue|dimgray|dodgerblue|firebrick|floralwhite|forestgreen|fuchsia|gainsboro|ghostwhite|gold|goldenrod|gray|green|greenyellow|honeydew|hotpink|indianred|indigo|ivory|khaki|lavender|lavenderblush|lawngreen|lemonchiffon|lightblue|lightcoral|lightcyan|lightgoldenrodyellow|lightgreen|lightgrey|lightpink|lightsalmon|lightseagreen|lightskyblue|lightslategray|lightsteelblue|lightyellow|lime|limegreen|linen|magenta|maroon|mediumaquamarine|mediumblue|mediumorchid|mediumpurple|mediumseagreen|mediumslateblue|mediumspringgreen|mediumturquoise|mediumvioletred|midnightblue|mintcream|mistyrose|moccasin|navajowhite|navy|oldlace|olive|olivedrab|orange|orangered|orchid|palegoldenrod|palegreen|paleturquoise|palevioletred|papayawhip|peachpuff|peru|pink|plum|powderblue|purple|red|rosybrown|royalblue|saddlebrown|salmon|sandybrown|seagreen|seashell|sienna|silver|skyblue|slateblue|slategray|snow|springgreen|steelblue|tan|teal|thistle|tomato|turquoise|violet|wheat|white|whitesmoke|yellow|yellowgreen|(#)[A-Fa-f0-9][A-Fa-f0-9][A-Fa-f0-9][A-Fa-f0-9][A-Fa-f0-9][A-F-af0-9]\":::Text Arg1:SQStrings:\n\
	Numeric:\"<((0(x|X)[0-9a-fA-F]*)|[0-9.]+((e|E)(\\+|-)?)?[0-9]*)(L|l|UL|ul|u|U|F|f)?>\":::Numeric Const::\n\
	Events:\"<(onAbort|onBlur|onClick|onChange|onDblClick|onDragDrop|onError|onFocus|onKeyDown|onKeyPress|onLoad|onMouseDown|onMouseMove|onMouseOut|onMouseOver|onMouseUp|onMove|onResize|onSelect|onSubmit|onUnload)>\":::Keyword::\n\
        Braces:\"[{}]\":::Keyword::\n\
	Statements:\"<(break|continue|else|for|if|in|new|return|this|typeof|var|while|with)>\":::Keyword::\n\
        Function:\"function[\\t ]+([a-zA-Z0-9_]+)[\\t \\(]+\":\"[\\n{]\"::Keyword::\n\
        FunctionName:\"\\1\":\"\"::Storage Type:Function:C\n\
        FunctionArgs:\"\\(\":\"\\)\"::Text Arg:Function:\n\
        Parentheses:\"[\\(\\)]\":::Plain::\n\
	BuiltInObjectType:\"<(anchor|Applet|Area|Array|button|checkbox|Date|document|elements|FileUpload|form|frame|Function|hidden|history|Image|link|location|Math|navigator|Option|password|Plugin|radio|reset|select|string|submit|text|textarea|window)>\":::Storage Type::\n\
        SQStrings:\"'\":\"'\":\"\\n\":String::\n\
        DQStrings:\"\"\"\":\"\"\"\":\"\\n\":String::\n\
	EventCapturing:\"captureEvents|releaseEvents|routeEvent|handleEvent\":\"\\)\":\"\\n\":Keyword::\n\
	PredefinedMethods:\"<(abs|acos|alert|anchor|asin|atan|atan2|back|big|blink|blur|bold|ceil|charAt|clear|clearTimeout|click|close|confirm|cos|escape|eval|exp|fixed|floor|focus|fontcolor|fontsize|forward|getDate|getDay|getHours|getMinutes|getMonth|getSeconds|getTime|getTimezoneOffset|getYear|go|indexOf|isNaN|italics|javaEnabled|join|lastIndexOf|link|log|max|min|open|parse|parseFloat|parseInt|pow|prompt|random|reload|replace|reset|reverse|round|scroll|select|setDate|setHours|setMinutes|setMonth|setSeconds|setTimeout|setTime|setYear|sin|small|sort|split|sqrt|strike|sub|submit|substring|sup|taint|tan|toGMTString|toLocaleString|toLowerCase|toString|toUpperCase|unescape|untaint|UTC|write|writeln)>\":::Keyword::\n\
	Properties:\"<(action|alinkColor|anchors|appCodeName|appName|appVersion|bgColor|border|checked|complete|cookie|defaultChecked|defaultSelected|defaultStatus|defaultValue|description|E|elements|enabledPlugin|encoding|fgColor|filename|forms|frames|hash|height|host|hostname|href|hspace|index|lastModified|length|linkColor|links|LN2|LN10|LOG2E|LOG10E|lowsrc|method|name|opener|options|parent|pathname|PI|port|protocol|prototype|referrer|search|selected|selectedIndex|self|SQRT1_2|SQRT2|src|status|target|text|title|top|type|URL|userAgent|value|vlinkColor|vspace|width|window)>\":::Storage Type::\n\
        Operators:\"[= ; ->]|[/]|&|\\|\":::Preprocessor::}",
#endif /*VMS*/
     "Perl:1:0 {\n\
	comments:\"#\":\"$\"::Comment::\n\
	double quote strings:\"\"\"\":\"\"\"\"::String::\n\
	dq string esc chars:\"\\\\(.|\\n)\":::String1:double quote strings:\n\
	single quote strings:\"'\":\"'\"::String::\n\
	sq string esc chars:\"\\\\(.|\\n)\":::String1:single quote strings:\n\
	subroutine header:\"sub[\\t ]+([a-zA-Z0-9_]+)[\\t ]+{\":::Keyword::\n\
	subr header coloring:\"\\1\":::Flag:subroutine header:C\n\
	ignore escaped chars:\"\\\\[#\"\"'\\$msytq]\":::Plain::\n\
	re matching:\"<((m|q|qq)?/)(\\\\/|[^/])*(/[gimsox]?)>\":::String::\n\
	re match coloring:\"\\1\\4\":::String2:re matching:C\n\
	re substitution:\"<((s|y|tr)/)(\\\\/|[^/])*(/)[^/]*(/[gimsox]?)\":::String::\n\
	re subs coloring:\"\\1\\4\\5\":::String2:re substitution:C\n\
	keywords:\"<(my|local|new|if|until|while|elsif|else|eval|unless|for|foreach|continue|exit|die|last|goto|next|redo|return|local|exec|do|use|require|package|eval|BEGIN|END|eq|ne|not|\\|\\||\\&\\&|and|or)>\":::Keyword::D\n\
	library fns:\"<(abs|accept|alarm|atan2|bind|binmode|bless|caller|chdir|chmod|chomp|chop|chr|chroot|chown|closedir|close|connect|cos|crypt|dbmclose|dbmopen|defined|delete|die|dump|each|endgrent|endhostent|endnetent|endprotoent|endpwent|endservent|eof|exec|exists|exp|fctnl|fileno|flock|fork|format|formline|getc|getgrent|getgrgid|getgrnam|gethostbyaddr|gethostbyname|gethostent|getlogin|getnetbyaddr|getnetbyname|getnetent|getpeername|getpgrp|getppid|getpriority|getprotobyname|getprotobynumber|getprotoent|getpwent|getpwnam|getpwuid|getservbyname|getservbyport|getservent|getsockname|getsockopt|glob|gmtime|grep|hex|import|index|int|ioctl|join|keys|kill|lcfirst|lc|length|link|listen|log|localtime|lstat|map|mkdir|msgctl|msgget|msgrcv|no|oct|opendir|open|ord|pack|pipe|pop|pos|printf|print|push|quotemeta|rand|readdir|read|readlink|recv|ref|rename|reset|reverse|rewinddir|rindex|rmdir|scalar|seekdir|seek|select|semctl|semget|semop|send|setgrent|sethostent|setnetent|setpgrp|setpriority|setprotoent|setpwent|setsockopt|shift|shmctl|shmget|shmread|shmwrite|shutdown|sin|sleep|socket|socketpair|sort|splice|split|sprintf|sqrt|srand|stat|study|substr|symlink|syscall|sysopen|sysread|system|syswrite|telldir|tell|tie|tied|time|times|truncate|uc|ucfirst|umask|undef|unlink|unpack|unshift|untie|utime|values|vec|wait|waitpid|wantarray|warn|write|qw|-[rwxoRWXOezsfdlpSbctugkTBMAC])>\":::Subroutine::D\n\
	variables:\"[$@%]({[^}]*}|[^a-zA-Z0-9_ /\\t\\n\\.,\\\\[\\\\{\\\\(]|[0-9]+|[a-zA-Z_][a-zA-Z0-9_]*)?\":::Identifier1::\n\
	variables in strings:\"[$@%&]({[^}]*}|[^a-zA-Z0-9_ /\\t\\n\\.,\\\\[\\\\{\\\\(]|[0-9]+|[a-zA-Z_][a-zA-Z0-9_]*)?\":::Identifier1:double quote strings:\n\
	subroutine call:\"&[a-zA-Z0-9_]+\":::Subroutine::\n\
	braces and parens:\"[\\[\\]{}\\(\\)]\":::Keyword::}",
     "Ada:1:0{\n\
        Comments:\"--\":\"$\"::Comment::\n\
        String Literals:\"\"\"\":\"\"\"\":\"\\n\":String::\n\
        Character Literals:\"'\":\"'\":\"[^\\\\][^']\":Character Const::\n\
        Ada Attributes:\"'[a-zA-Z][a-zA-Z_]+\":::Ada Attributes::\n\
        Numeric Literals:\"<(((2|8|10|16)#[_0-9a-fA-F]*#)|[0-9.]+)>\":::Numeric Const::\n\
        Withs Pragmas Use:\"(([wW]ith|WITH|[pP]ragma|PRAGMA|[uU]se|USE)[ \\t\\n\\f\\r]+[a-zA-Z0-9_.]+;)+\":::Preprocessor::\n\
        Predefined Types:\"<([bB]oolean|BOOLEAN|[cC]haracter|CHARACTER|[cC]ount|COUNT|[dD]uration|DURATION|[fF]loat|FLOAT|[iI]nteger|INTEGER|[lL]ong_[fF]loat|LONG_FLOAT|[lL]ong_[iI]nteger|LONG_INTEGER|[pP]riority|PRIORITY|[sS]hort_[fF]loat|SHORT_FLOAT|[sS]hort_[iI]nteger|SHORT_INTEGER|[sS]tring|STRING)>\":::Storage Type::D\n\
        Predefined Subtypes:\"<([fF]ield|FIELD|[nN]atural|NATURAL|[nN]umber_[bB]ase|NUMBER_BASE|[pP]ositive|POSITIVE|[pP]riority|PRIORITY)>\":::Storage Type::D\n\
        Reserved Words:\"<([aA]bort|ABORT|[aA]bs|ABS|[aA]ccept|ACCEPT|[aA]ccess|ACCESS|[aA]nd|AND|[aA]rray|ARRAY|[aA][tT]|[bB]egin|BEGIN|[bB]ody|BODY|[cC]ase|CASE|[cC]onstant|CONSTANT|[dD]eclare|DECLARE|[dD]elay|DELAY|[dD]elta|DELTA|[dD]igits|DIGITS|[dD][oO]|[eE]lse|ELSE|[eE]lsif|ELSIF|[eE]nd|END|[eE]ntry|ENTRY|[eE]xception|EXCEPTION|[eE]xit|EXIT|[fF]or|FOR|[fF]unction|FUNCTION|[gG]eneric|GENERIC|[gG]oto|GOTO|[iI][fF]|[iI][nN]|[iI][sS]|[lL]imited|LIMITED|[lL]oop|LOOP|[mM]od|MOD|[nN]ew|NEW|[nN]ot|NOT|[nN]ull|NULL|[oO][fF]|[oO][rR]|[oO]thers|OTHERS|[oO]ut|OUT|[pP]ackage|PACKAGE|[pP]ragma|PRAGMA|[pP]rivate|PRIVATE|[pP]rocedure|PROCEDURE|[rR]aise|RAISE|[rR]ange|RANGE|[rR]ecord|RECORD|[rR]em|REM|[rR]enames|RENAMES|[rR]eturn|RETURN|[rR]everse|REVERSE|[sS]elect|SELECT|[sS]eparate|SEPARATE|[sS]ubtype|SUBTYPE|[tT]ask|TASK|[tT]erminate|TERMINATE|[tT]hen|THEN|[tT]ype|TYPE|[uU]se|USE|[wW]hen|WHEN|[wW]hile|WHILE|[wW]ith|WITH|[xX]or|XOR)>\":::Keyword::D\n\
        Ada 95 Only:\"<([aA]bstract|ABSTRACT|[tT]agged|TAGGED|[aA]ll|ALL|[pP]rotected|PROTECTED|[aA]liased|ALIASED|[rR]equeue|REQUEUE|[uU]ntil|UNTIL)>\":::Keyword::\n\
        Identifiers:\"<([a-zA-Z][a-zA-Z0-9_]*)>\":::Identifier::D\n\
        Dot All:\"\\\\.[aA][lL][lL]>\":::Storage Type::D}",
     "Fortran:2:0 {\n\
        Comment:\"^[Cc*!]\":\"$\"::Comment::\n\
	Bang Comment:\"!\":\"$\"::Comment::\n\
	Debug Line:\"^D\":\"$\"::Preprocessor::\n\
	String:\"'\":\"'\":\"\\n([^ \\t]| [^ \\t]|  [^ \\t]|   [^ \\t]|    [^ \\t]|     [ \\t0]| *\\t[^1-9])\":String::\n\
        Keywords:\"<(ACCEPT|[Aa]ccept|AUTOMATIC|[Aa]utomatic|BACKSPACE|[Bb]ackspace|BLOCK|[Bb]lock|CALL|[Cc]all|CLOSE|[Cc]lose|COMMON|[Cc]ommon|CONTINUE|[Cc]ontinue|DATA|[Dd]ata|DECODE|[Dd]ecode|DELETE|[Dd]elete|DIMENSION|[Dd]imension|DO|[Dd]o|ELSE|[Ee]lse|ELSEIF|[Ee]lseif|ENCODE|[Ee]ncode|ENDDO|[Ee]nd[Dd]o|END ?FILE|[Ee]nd *[fF]ile|ENDIF|[Ee]nd[iI]f|END|[Ee]nd|ENTRY|[Ee]ntry|EQUIVALENCE|[Ee]quivalence|EXIT|[Ee]xit|EXTERNAL|[Ee]xternal|FORMAT|[Ff]ormat|FUNCTION|[Ff]unction|GO *TO|[Gg]o *[tT]o|IF|[Ii]f|IMPLICIT|[Ii]mplicit|INCLUDE|[Ii]nclude|INQUIRE|[Ii]nquire|INTRINSIC|[Ii]ntrinsic|LOGICAL|[Ll]ogical|MAP|[Mm]ap|NONE|[Nn]one|ON|[Oo]n|OPEN|[Oo]pen|PARAMETER|[Pp]arameter|PAUSE|[Pp]ause|POINTER|[Pp]ointer|PRINT|[Pp]rint|PROGRAM|[Pp]rogram|READ|[Rr]ead|RECORD|[Rr]ecord|RETURN|[Rr]eturn|REWIND|[Rr]ewind|SAVE|[Ss]ave|STATIC|[Ss]tatic|STOP|[Ss]top|STRUCTURE|[Ss]tructure|SUBROUTINE|[Ss]ubroutine|SYSTEM|[Ss]ystem|THEN|[Tt]hen|TYPE|[Tt]ype|UNION|[Uu]nion|UNLOCK|[Uu]nlock|VIRTUAL|[Vv]irtual|VOLATILE|[Vv]olatile|WHILE|[Ww]hile|WRITE|[Ww]rite)>\":::Keyword::D\n\
      	Data Types:\"<(BYTE|[Bb]yte|CHARACTER|[Cc]haracter|COMPLEX|[Cc]omplex|DOUBLE *COMPLEX|[Dd]ouble *[Cc]omplex|DOUBLE *PRECISION|[Dd]ouble *[Pp]recision|DOUBLE|[Dd]ouble|INTEGER|[Ii]nteger|REAL|[Rr]eal)(\\*[0-9]+)?>\":::Keyword::D\n\
      	F90 Keywords:\"<(ALLOCATABLE|[Aa]llocatable|ALLOCATE|[Aa]llocate|CASE|[Cc]ase|CASE|[Cc]ase|CYCLE|[Cc]ycle|DEALLOCATE|[Dd]eallocate|ELSEWHERE|[Ee]lsewhere|NAMELIST|[Nn]amelist|REWRITE|[Rr]ewrite|SELECT|[Ss]elect|WHERE|[Ww]here|INTENT|[Ii]ntent|OPTIONAL|[Oo]ptional)>\":::Keyword::D\n\
      	Continuation:\"^(     [^ \\t0]|( |  |   |    )?\\t[1-9])\":::Flag::\n\
      	Continuation in String:\"\\n(     [^ \\t0]|( |  |   |    )?\\t[1-9])\":::Flag:String:}",
     "Tcl:1:0 {\n\
	Comment:\"#\":\"$\"::Comment::\n\
	Double Quote String:\"\"\"\":\"\"\"\":\"\\n\":String::\n\
	Single Quote String:\"'\":\"'\":\"[^\\\\][^']\":String::\n\
	Ignore Escaped Chars:\"\\\\(.|\\n)\":::Plain::\n\
	Keywords:\"<(if|else|elseif|set|unset|incr|append|global|glob|upvar|uplevel|proc|catch|error|while|eof|foreach|for|return|break|continue|eval|source|switch|list|lindex|regexp|regsub|pwd|gets|puts|eof|pid|exit|open|close|expr|exec|concat|join|lappend|linsert|llength|lrange|lreplace|lsearch|lsort|split|format|scan|trace|cd|flush|read|seek|tell|string *(compare|first|index|last|length|match|range|tolower|toupper|trim|trimleft|trimright)|file[ \\t]+(atime|dirname|executable|exists|extension|isdirectory|isfile|lstat|mtime|owned|readable|readlink|rootname|size|stat|tail|type|writable)|array[ \\t]+(anymore|donesearch|names|nextelement|size|startsearch)|auto_mkindex|info[ \\t]+(args|body|cmdcount|commands|default|exists|globals|level|library|locals|procs|script|tclversion|vars)|rename|time|trace[ \\t]+(variable|vdelete|vinfo)|unknown|history|history[ \\t]+(keep|nextid|redo|substitute))>\":::Keyword::D\n\
	Variable Ref:\"\\$[0-9a-zA-Z_]+|\\${[^}]*}|\\$\":::Identifier1::\n\
	Braces and Brackets:\"[\\[\\]{}]\":::Keyword::D\n\
	DQ String Esc Chars:\"\\\\(.|\\n)\":::String1:Double Quote String:\n\
	SQ String Esc Chars:\"\\\\(.|\\n)\":::String1:Single Quote String:\n\
	Variable in String:\"\\$[0-9a-zA-Z_]+|\\${[^}]*}|\\$\":::Identifier1:Double Quote String:}",
     "Pascal:1:0 {\n\
	Comment 1:\"<(/\\*|\\(\\*)>\":\"<(\\*/|\\*\\))>\"::Comment::\n\
	Comment 2:\"{\":\"}\"::Comment::\n\
	String:\"'\":\"'\":\"\\n\":String::\n\
	Preprocessor Line:\"^[ \\t]*#\":\"$\"::Preprocessor::\n\
	String Esc Char:\"\\\\(.|\\n)\":::String1:String:\n\
	Preprocessor Esc Char:\"\\\\(.|\\n)\":::Preprocessor1:Preprocessor Line:\n\
	Preprocessor Comment 1:\"<(/\\*|\\(\\*)>\":\"<(\\*/|\\*\\))>\"::Comment:Preprocessor Line:\n\
	Preprocessor Comment 2:\"{\":\"}\"::Comment:Preprocessor Line:\n\
	Numeric Const:\"<((0(x|X)[0-9a-fA-F]*)|[0-9.]+((e|E)(\\+|-)?)?[0-9]*)(L|l|UL|ul|u|U|F|f)?>\":::Numeric Const::D\n\
	Storage and Ops:\"<(and|AND|array|const|div|export|file|function|import|in|IN|label|mod|module|nil|not|NOT|only|or|OR|packed|pow|pragma|procedure|program|protected|qualified|record|restricted|set|type|var)>\":::Storage Type::D\n\
	Keywords:\"<(begin|case|do|downto|else|end|for|goto|if|of|otherwise|then|to|until|while|with)>\":::Keyword::D}",
     "Yacc:1:0{\n\
    	comment:\"/\\*\":\"\\*/\"::Comment::\n\
    	string:\"\"\"\":\"\"\"\":\"\\n\":String::\n\
    	preprocessor line:\"^[ \t]*#\":\"$\"::Preprocessor::\n\
    	string escape chars:\"\\\\(.|\\n)\":::String1:string:\n\
    	preprocessor esc chars:\"\\\\(.|\\n)\":::Preprocessor1:preprocessor line:\n\
    	preprocessor comment:\"/\\*\":\"\\*/\"::Comment:preprocessor line:\n\
    	character constant:\"'\":\"'\":\"[^\\\\][^']\":Character Const::\n\
	numeric constant:\"<((0(x|X)[0-9a-fA-F]*)|(([0-9]+\\.?[0-9]*)|(\\.[0-9]+))((e|E)(\\+|-)?[0-9]+)?)(L|l|UL|ul|u|U|F|f)?>\":::Numeric Const::D\n\
    	storage keyword:\"<(const|extern|auto|register|static|unsigned|signed|volatile|char|double|float|int|long|short|void|typedef|struct|union|enum)>\":::Storage Type::D\n\
	rule:\"^[ \\t]*[A-Za-z_][A-Za-z0-9_]*[ \\t]*:\":::Preprocessor1::D\n\
    	keyword:\"<(return|goto|if|else|case|default|switch|break|continue|while|do|for|sizeof)>\":::Keyword::D\n\
	yacc keyword:\"<(error|YYABORT|YYACCEPT|YYBACKUP|YYERROR|YYINITDEPTH|YYLTYPE|YYMAXDEPTH|YYRECOVERING|YYSTYPE|yychar|yyclearin|yydebug|yyerrok|yyerror|yylex|yylval|yylloc|yynerrs|yyparse)>\":::Text Arg::D\n\
	percent keyword:\"<(%left|%nonassoc|%prec|%right|%start|%token|%type|%union)>([ \\t]*\\<.*\\>)?\":::Text Arg::D\n\
    	braces:\"[{}]\":::Keyword::D\n\
	markers:\"<(%{|%}|%%)>\":::Flag::D\n\
	percent sub-expr:\"\\2\":""::Text Arg2:percent keyword:DC}",
     "Makefile:8:0{\n\
	Comment:\"#\":\"$\"::Comment::\n\
	Assignment:\"^( *| [ \\t]*)[A-Za-z0-9_+]*[ \\t]*(\\+|:)?=\":::Preprocessor::\n\
	Dependency Line:\"^ *([A-Za-z0-9./$(){} _%+-]*)::?\":\"$|;\"::Text Key1::\n\
	Dep Target:\"\\1\":\"\"::Text Key:Dependency Line:C\n\
	Dep Continuation:\"\\\\\\n\":::Keyword:Dependency Line:\n\
	Dep Comment:\"#\":\"$\"::Comment:Dependency Line:\n\
	Dep Macro:\"\\$([A-Za-z0-9_]|\\([^)]*\\)|{[^}]*})\":::Preprocessor:Dependency Line:\n\
	Dep Internal Macro:\"\\$([<@*?%]|\\$@)\":::Preprocessor1:Dependency Line:\n\
	Continuation:\"\\\\$\":::Keyword::\n\
	Macro:\"\\$([A-Za-z0-9_]|\\([^)]*\\)|{[^}]*})\":::Preprocessor::\n\
	Internal Macro:\"\\$([<@*?%]|\\$@)\":::Preprocessor1::\n\
	Escaped Dollar:\"\\$\\$\":::Comment::\n\
	Include:\"^include[ \\t]\":::Keyword::}",
     "Sh Ksh Bash:1:0{\n\
        escaped special characters:\"\\\\[\\\\\"\"$`']\":::Keyword::\n\
        single quoted string:\"'\":\"'\"::String1::\n\
        double quoted string:\"\"\"\":\"\"\"\"::String::\n\
        double quoted escape:\"\\\\[\\\\\"\"$`]\":::String2:double quoted string:\n\
        dq command sub:\"`\":\"`\":\"\"\"\":Subroutine:double quoted string:\n\
        dq arithmetic expansion:\"\\$\\(\\(\":\"\\)\\)\":\"\"\"\":String:double quoted string:\n\
        dq new command sub:\"\\$\\(\":\"\\)\":\"\"\"\":Subroutine:double quoted string:\n\
        dq variables:\"\\$([-*@#?$!0-9]|[a-zA-Z_][0-9a-zA-Z_]*)\":::Identifier1:double quoted string:\n\
        dq variables2:\"\\${\":\"}\":\"\\n\":Identifier1:double quoted string:\n\
        arithmetic expansion:\"\\$\\(\\(\":\"\\)\\)\"::String::\n\
        ae escapes:\"\\\\[\\\\$`\"\"']\":::String2:arithmetic expansion:\n\
        ae single quoted string:\"'\":\"'\":\"\\)\\)\":String1:arithmetic expansion:\n\
        ae command sub:\"`\":\"`\":\"\\)\\)\":Subroutine:arithmetic expansion:\n\
        ae arithmetic expansion:\"\\$\\(\\(\":\"\\)\\)\"::String:arithmetic expansion:\n\
        ae new command sub:\"\\$\\(\":\"\\)\":\"\\)\\)\":Subroutine:arithmetic expansion:\n\
        ae variables:\"\\$([-*@#?$!0-9]|[a-zA-Z_][0-9a-zA-Z_]*)\":::Identifier1:arithmetic expansion:\n\
        ae variables2:\"\\${\":\"}\":\"\\)\\)\":Identifier1:arithmetic expansion:\n\
        comments:\"^[ \\t]*#\":\"$\"::Comment::\n\
        command substitution:\"`\":\"`\"::Subroutine::\n\
        cs escapes:\"\\\\[\\\\$`\"\"']\":::Subroutine1:command substitution:\n\
        cs single quoted string:\"'\":\"'\":\"`\":String1:command substitution:\n\
        cs variables:\"\\$([-*@#?$!0-9]|[a-zA-Z_][0-9a-zA-Z_]*)\":::Identifier1:command substitution:\n\
        cs variables2:\"\\${\":\"}\":\"`\":Identifier1:command substitution:\n\
        new command substitution:\"\\$\\(\":\"\\)\"::Subroutine::\n\
        ncs escapes:\"\\\\[\\\\$`\"\"']\":::Subroutine1:new command substitution:\n\
        ncs single quoted string:\"'\":\"'\"::String1:new command substitution:\n\
        ncs variables:\"\\$([-*@#?$!0-9]|[a-zA-Z_][0-9a-zA-Z_]*)\":::Identifier1:new command substitution:\n\
        ncs variables2:\"\\${\":\"}\":\"\\)\":Identifier1:new command substitution:\n\
        assignment:\"[a-zA-Z_][0-9a-zA-Z_]*[ \\t]*=\":::Identifier1::\n\
        variables:\"\\$([-*@#?$!0-9_]|[a-zA-Z_][0-9a-zA-Z_]*)\":::Identifier1::\n\
        variables2:\"\\${\":\"}\"::Identifier1::\n\
        comments in line:\"#\":\"$\"::Comment::\n\
        numbers:\"<((0(x|X)[0-9a-fA-F]*)|[0-9.]+((e|E)(\\+|-)?)?[0-9]*)(L|l|UL|ul|u|U|F|f)?>\":::Numeric Const::D\n\
        keywords:\"<(if|fi|then|else|elif|case|esac|while|for|do|done|in|select|until|function|continue|break|return|exit)>\":::Keyword::D\n\
        command options:\"[ \\t]-[^ \\t{}[\\],.()'\"\"~!@#$%^&*|\\\\<>?]+\":::Identifier::D\n\
        delimiters:\"<[.]>|[{};<>&~=!|^%[\\]+*|]\":::Text Key::D\n\
        built ins:\"<(:|\\\\.|\\\\[\\\\[|]]|source|alias|bg|bind|builtin|cd|chdir|command|declare|dirs|echo|enable|eval|exec|export|fc|fg|getopts|hash|help|history|jobs|kill|let|local|logout|popd|print|pushd|pwd|read|readonly|set|shift|stop|suspend|test|times|trap|type|typeset|ulimit|umask|unalias|unset|wait|whence)>\":::Subroutine1::D}",
     "Csh:1:0 {\n\
	Comment:\"#\":\"$\"::Comment::\n\
	Single Quote String:\"'\":\"([^\\\\]'|^')\":\"\\n\":String::\n\
	SQ String Esc Char:\"\\\\([bcfnrt$\\n\\\\]|[0-9][0-9]?[0-9]?)\":::String1:Single Quote String:\n\
	Double Quote String:\"\"\"\":\"\"\"\":\"\\n\":String::\n\
	DQ String Esc Char:\"\\\\([bcfnrt\\n\\\\]|[0-9][0-9]?[0-9]?)\":::String1:Double Quote String:\n\
	Keywords:\"(^|[`;()])[ \t]*(return|if|endif|then|else|switch|endsw|while|end|foreach|do|done)>\":::Keyword::D\n\
	Variable Ref:\"\\$([<$0-9\\*]|[#a-zA-Z_?][0-9a-zA-Z_[\\]]*(:([ehqrtx]|gh|gt|gr))?|{[#0-9a-zA-Z_?][a-zA-Z0-9_[\\]]*(:([ehqrtx]|gh|gt|gr))?})\":::Identifier1::\n\
	Variable in String:\"\\$([<$0-9\\*]|[#a-zA-Z_?][0-9a-zA-Z_[\\]]*(:([ehqrtx]|gh|gt|gr))?|{[#0-9a-zA-Z_?][a-zA-Z0-9_[\\]]*(:([ehqrtx]|gh|gt|gr))?})\":::Identifier1:Double Quote String:\n\
	Naked Variable Cmds:\"<(unset|set|setenv|shift)[ \\t]+[0-9a-zA-Z_]*(\\[.+\\])?\":::Identifier1::\n\
	Recolor Naked Cmd:\"\\1\":::Keyword:Naked Variable Cmds:C\n\
	Built In Cmds:\"(^|\\|&|[\\|`;()])[ \t]*(alias|bg|break|breaksw|case|cd|chdir|continue|default|echo|eval|exec|exit|fg|goto|glob|hashstat|history|jobs|kill|limit|login|logout|nohup|notify|nice|onintr|popd|pushd|printenv|read|rehash|repeat|set|setenv|shift|source|suspend|time|umask|unalias|unhash|unlimit|unset|unsetenv|wait)>\":::Keyword::D\n\
        Tcsh Built In Cmds:\"(^|\\|&|[\\|`;()])[ \t]*(alloc|bindkey|builtins|complete|echotc|filetest|hup|log|sched|settc|setty|stop|telltc|uncomplete|where|which|dirs|ls-F)>\":::Keyword::D\n\
	Special Chars:\"([-{};.,<>&~=!|^%[\\]\\+\\*\\|()])\":::Keyword::D}",
     "Python:1:0{\n\
        Comment:\"#\":\"$\"::Comment::\n\
        String3:\"\"\"\"\"\"\"\":\"\"\"\"\"\"\"\"::String1::\n\
        String:\"\"\"\":\"\"\"\":\"\\n\":String1::\n\
        String2:\"'\":\"'\":\"\\n\":String1::\n\
        Numeric const:\"<((0(x|X)[0-9a-fA-F]*)|[0-9.]+((e|E)(\\+|-)?)?[0-9]*)>\":::Numeric Const::\n\
        include:\"<(import|from)>\":\"$\"::Preprocessor::\n\
        Storage keyword:\"<(class|global|lambda)>\":::Storage Type::\n\
        Keyword:\"<(access|del|return|and|elif|not|try|break|else|if|or|while|except|pass|continue|finally|in|print|def|for|is|raise)>\":::Keyword::\n\
        Braces and parens:\"[{()}]\":::Keyword::\n\
        String escape chars:\"\\\\(.|\\n|\\r|\\b)\":::String1::}",
     "Awk:2:0{\n\
        Comment:\"#\":\"$\"::Comment::\n\
        Pattern:\"/(\\\\.|([[][]]?[^]]+[]])|[^/])+/\":::Preprocessor::\n\
        Keyword:\"<(return|print|printf|if|else|while|for|in|do|break|continue|next|exit|close|system|getline)>\":::Keyword::D\n\
        String:\"\"\"\":\"\"\"\":\"\\n\":String1::\n\
        String escape:\"\\\\(.|\\n)\":::String1:String:\n\
        Builtin functions:\"<(atan2|cos|exp|int|log|rand|sin|sqrt|srand|gsub|index|length|match|split|sprintf|sub|substr)>\":::Keyword::D\n\
        Gawk builtin functions:\"<(fflush|gensub|tolower|toupper|systime|strftime)>\":::Text Key1::D\n\
        Builtin variables:\"<(ARGC|ARGV|FILENAME|FNR|FS|NF|NR|OFMT|OFS|ORS|RLENGTH|RS|RSTART|SUBSEP)>\":::Storage Type::D\n\
        Gawk builtin variables:\"\"\"<(ARGIND|ERRNO|RT|IGNORECASE|FIELDWIDTHS)>\"\"\":::Storage Type::D\n\
        Field:\"\\$[0-9a-zA-Z_]+|\\$[ \\t]*\\([^,;]*\\)\":::Storage Type::D\n\
        BeginEnd:\"<(BEGIN|END)>\":::Preprocessor1::D\n\
        Numeric constant:\"<((0(x|X)[0-9a-fA-F]*)|[0-9.]+((e|E)(\\+|-)?)?[0-9]*)(L|l|UL|ul|u|U|F|f)?>\":::Numeric Const::D\n\
        String pattern:\"~[ \\t]*\"\"\":\"\"\"\":\"\\n\":Preprocessor::\n\
        String pattern escape:\"\\\\(.|\\n)\":::Preprocessor:String pattern:\n\
        newline escape:\"\\\\$\":::Preprocessor1::\n\
        Function:\"function\":::Preprocessor1::D}",
     "LaTeX:1:0{\n\
	Comment:\"%\":\"$\"::Text Comment::\n\
	Parameter:\"#[0-9]*\":::Text Arg::\n\
	Special Chars:\"[{}&]\":::Keyword::\n\
	Escape Chars:\"\\\\[$&%#_{}]\":::Text Escape::\n\
	Super Sub 1 Char:\"(\\^|_)[^{]\":::Text Arg2::\n\
	Verbatim Begin End:\"\\\\begin{verbatim\\*?}\":\"\\\\end{verbatim\\*?}\"::Plain::\n\
	Verbatim Color:\"\\0\":\"\\0\"::Keyword:Verbatim Begin End:C\n\
	Verbatim 1:\"\\\\verb\\*?`\":\"`\"::Plain::\n\
	Verbatim 1 Color:\"\\0\":\"\\0\"::Keyword:Verbatim 1:C\n\
	Verbatim 2:\"\\\\verb\\*?#\":\"#\"::Plain::\n\
	Verbatim 2 Color:\"\\0\":\"\\0\"::Keyword:Verbatim 2:C\n\
	Verbatim 3 Color:\"\\0\":\"\\0\"::Keyword:Verbatim 3:C\n\
	Verbatim 3:\"\\\\verb\\*?/\":\"/\"::Plain::\n\
	Inline Math:\"\\$\":\"\\$\":\"\\n\\n\":LaTeX Math::\n\
	Math Color:\"\\0\":\"\\0\"::Keyword:Inline Math:C\n\
	Inline Math1:\"\\\\\\(\":\"\\\\\\)\"::LaTeX Math::\n\
	Math1 Color:\"\\0\":\"\\0\"::Keyword:Inline Math1:C\n\
	Math Escape Chars:\"\\\\\\$\":::Text Escape:Inline Math:\n\
	No Arg Command:\"\\\\(left|right)[\\[\\]{}()]\":::Text Key::\n\
	Command:\"[_^]|[\\\\@](a'|a`|a=|[A-Za-z]+\\*?|\\\\\\*|[-@_='`^\"\"|\\[\\]*:!+<>/~.,\\\\ ])\":\"nevermatch\":\"[^{[(]\":Text Key::\n\
	Cmd Brace Args:\"{\":\"}\"::Text Arg2:Command:\n\
	Brace Color:\"\\0\":\"\\0\"::Text Arg:Cmd Brace Args:C\n\
	Cmd Paren Args:\"\\(\":\"\\)\":\"$\":Text Arg2:Command:\n\
	Paren Color:\"\\0\":\"\\0\"::Text Arg:Cmd Paren Args:C\n\
	Cmd Bracket Args:\"\\[\":\"\\]\":\"$\":Text Arg2:Command:\n\
	Bracket Color:\"\\0\":\"\\0\"::Text Arg:Cmd Bracket Args:C\n\
	Sub Command:\"([_^]|([\\\\@]([A-Za-z]+\\*?|[^A-Za-z$&%#{}~\\\\ \\t])))\":::Text Key1:Cmd Brace Args:\n\
	Sub Brace:\"{\":\"}\"::Text Arg2:Cmd Brace Args:\n\
	Sub Sub Brace:\"{\":\"}\"::Text Arg2:Sub Brace:\n\
	Sub Sub Sub Brace:\"{\":\"}\"::Text Arg2:Sub Sub Brace:\n\
	Sub Sub Sub Sub Brace:\"{\":\"}\"::Text Arg2:Sub Sub Sub Brace:\n\
	Sub Paren:\"\\(\":\"\\)\":\"$\":Text Arg2:Cmd Paren Args:\n\
	Sub Sub Paren:\"\\(\":\"\\)\":\"$\":Text Arg2:Sub Paren:\n\
	Sub Sub Sub Paren:\"\\(\":\"\\)\":\"$\":Text Arg2:Sub Sub Paren:\n\
	Sub Parameter:\"#[0-9]*\":::Text Arg:Cmd Brace Args:\n\
	Sub Spec Chars:\"[{}$&]\":::Text Arg:Cmd Brace Args:\n\
	Sub Esc Chars:\"\\\\[$&%#_{}~^\\\\]\":::Text Arg1:Cmd Brace Args:}",
     "VHDL:1:0{\n\
        Comments:\"--\":\"$\"::Comment::\n\
        String Literals:\"\"\"\":\"\"\"\":\"\\n\":String::\n\
        Vhdl Attributes:\"'[a-zA-Z][a-zA-Z_]+\":::Ada Attributes::\n\
        Character Literals:\"'\":\"'\":\"[^\\\\][^']\":Character Const::\n\
        Numeric Literals:\"<(((2#|8#|10#|16#)[_0-9a-fA-F]*#)|[0-9.]+)>\":::Numeric Const::\n\
	Predefined Types:\"<([aA]lias|ALIAS|[cC]onstant|CONSTANT|[sS]ignal|SIGNAL|[vV]ariable|VARIABLE|[sS]ubtype|SUBTYPE|[tT]ype|TYPE|[rR]esolved|RESOLVED|[bB]oolean|BOOLEAN|[sS]tring|STRING|[iI]nteger|INTEGER|[nN]atural|NATURAL|[tT]ime|TIME)>\":::Storage Type::D\n\
	Predefined SubTypes:\"<([sS]td_[lL]ogic|STD_LOGIC|[sS]td_[lL]ogic_[vV]ector|STD_LOGIC_VECTOR|[sS]td_[uU][lL]ogic|STD_ULOGIC|[sS]td_[uU][lL]ogic_[vV]ector|STD_ULOGIC_VECTOR|[bB]it|BIT|[bB]it_[vV]ector|BIT_VECTOR)>\":::Storage Type::D\n\
        Reserved WordsA:\"<([aA]bs|[aA]ccess|[aA]fter|[aA]ll|[aA]nd|[aA]rchitecture|[aA]rray|[aA]ssert|[aA]ttribute|[bB]egin|[bB]lock|[bB]ody|[bB]uffer|[bB]us|[cC]ase|[cC]omponent|[cC]onfiguration|[dD]isconnect|[dD]ownto|[eE]lse|[eE]lsif|[eE]nd|[eE]ntity|[eE]rror|[eE]xit|[fF]ailure|[fF]ile|[fF]or|[fF]unction|[gG]enerate|[gG]eneric|[gG]uarded|[iI]f|[iI]n|[iI]nout|[iI]s|[lL]abel|[lL]ibrary|[lL]inkage|[lL]oop|[mM]ap|[mM]od|[nN]and|[nN]ew|[nN]ext|[nN]or|[nN]ot|[nN]ote|[nN]ull|[oO]f|[oO]n|[oO]pen|[oO]r|[oO]thers|[oO]ut|[pP]ackage|[pP]ort|[pP]rocedure|[pP]rocess|[rR]ange|[rR]ecord|[rR]egister|[rR]em|[rR]eport|[rR]eturn|[sS]elect|[sS]everity|[tT]hen|[tT]o|[tT]ransport|[uU]nits|[uU]ntil|[uU]se|[wW]ait|[wW]arning|[wW]hen|[wW]hile|[wW]ith|[xX]or|[gG]roup|[iI]mpure|[iI]nertial|[lL]iteral|[pP]ostponed|[pP]ure|[rR]eject|[rR]ol|[rR]or|[sS]hared|[sS]la|[sS]ll|[sS]ra|[sS]rl|[uU]naffected|[xX]nor)>\":::Keyword::D\n\
        Reserved WordsB:\"<(ABS|ACCESS|AFTER|ALL|AND|ARCHITECTURE|ARRAY|ASSERT|ATTRIBUTE|BEGIN|BLOCK|BODY|BUFFER|BUS|CASE|COMPONENT|CONFIGURATION|DISCONNECT|DOWNTO|ELSE|ELSIF|END|ENTITY|ERROR|EXIT|FAILURE|FILE|FOR|FUNCTION|GENERATE|GENERIC|GUARDED|IF|IN|INOUT|IS|LABEL|LIBRARY|LINKAGE|LOOP|MAP|MOD|NAND|NEW|NEXT|NOR|NOT|NOTE|NULL|OF|ON|OPEN|OR|OTHERS|OUT|PACKAGE|PORT|PROCEDURE|PROCESS|RANGE|RECORD|REGISTER|REM|REPORT|RETURN|SELECT|SEVERITY|THEN|TO|TRANSPORT|UNITS|UNTIL|USE|WAIT|WARNING|WHEN|WHILE|WITH|XOR|GROUP|IMPURE|INERTIAL|LITERAL|POSTPONED|PURE|REJECT|ROL|ROR|SHARED|SLA|SLL|SRA|SRL|UNAFFECTED|XNOR)>\":::Keyword::D\n\
        Identifiers:\"<([a-zA-Z][a-zA-Z0-9_]*)>\":::Identifier::D\n\
        Flag Special Comments:\"--\\<[^a-zA-Z0-9]+\\>\":::Flag:Comments:}",
     "Verilog:1:0{\n\
        Comment: \"/\\*\":\"\\*/\"::Comment::\n\
        cplus comment: \"//\":\"$\"::Comment::\n\
        String Literals: \"\"\"\":\"\"\"\":\"\\n\":String::\n\
        preprocessor line: \"^[  ]*`\":\"$\"::Preprocessor::\n\
        Reserved WordsA: \"<(module|endmodule|parameter|specify|endspecify|begin|end|initial|always|if|else|task|endtask|force|release|attribute|case|case[xz]|default|endattribute|endcase|endfunction|endprimitive|endtable|for|forever|function|primitive|table|while)>\":::Keyword::D\n\
        Predefined Types: \"<(and|assign|buf|bufif[01]|cmos|deassign|defparam|disable|edge|event|force|fork|highz[01]|initial|inout|input|integer|join|large|macromodule|medium|nand|negedge|nmos|nor|not|notif[01]|or|output|parameter|pmos|posedge|pullup|rcmos|real|realtime|reg|release|repeat|rnmos|rpmos|rtran|rtranif[01]|scalered|signed|small|specparam|strength|strong[01]|supply[01]|time|tran|tranif[01]|tri[01]?|triand|trior|trireg|unsigned|vectored|wait|wand|weak[01]|wire|wor|xnor|xor)>\":::Storage Type::D\n\
        System Functions:\"<(\\$[a-z_]+)>\":::Subroutine::D\n\
        Numeric Literals: \"<([0-9]*'[dD][0-9xz\\\\?_]+|[0-9]*'[hH][0-9a-fxz\\\\?_]+|[0-9]*'[oO][0-7xz\\\\?_]+|[0-9]*'[bB][01xz\\\\?_]+|[0-9.]+((e|E)(\\\\+|-)?)?[0-9]*|[0-9]+)>\":::Numeric Const::\n\
        Delay word: \"<((#\\(.*\\))|(#[0-9]*))>\":::Ada Attributes::D\n\
        Identifiers: \"<([a-zA-Z][a-zA-Z0-9_]*)>\":::Identifier::D}",
     "X Resources:1:0{\n\
	Preprocessor:\"^[ \\t]*#\":\"$\"::Preprocessor::\n\
	Preprocessor Wrap:\"\\\\\\n\":::Preprocessor1:Preprocessor:\n\
	Comment:\"^[ \\t]*!\":\"$\"::Comment::\n\
	Comment Wrap:\"\\\\\\n\":::Comment:Comment:\n\
	Resource:\"^[ \\t]*[^: \t]+[ \\t]*:\":\"$\"::Plain::\n\
	Resource Esc Chars:\"\\\\.\":::Text Arg2:Resource:\n\
	Resource Space Warning:\"[^ \\t]+[ \\t]+$\":::Flag:Resource:\n\
	Resource Name:\"\\0\":\"\"::Storage Type:Resource:C\n\
	Resource Wrap:\"\\\\\\n\":::Text Arg1:Resource:\n\
	Free Text:\"^.*$\":::Flag::}",
     "NEdit Macro:1:0{\n\
	Comment:\"#\":\"$\"::Comment::\n\
	Built-in Vars:\"<\\$([1-9]|n_args|cursor|file_name|file_path|text_length|selection_start|selection_end|selection_left|selection_right|wrap_margin|tab_dist|em_tab_dist|use_tabs|language_mode|string_dialog_button|search_end|read_status|shell_cmd_status)>\":::Subroutine1::\n\
	Built-in Subrs:\"<(focus_window|shell_command|length|get_range|t_print|dialog|string_dialog|replace_range|replace_selection|set_cursor_pos|get_character|min|max|search|search_string|substring|replace_substring|read_file|write_file|append_file|beep|get_selection|replace_in_string|select|select_rectangle|toupper|tolower|string_to_clipboard|clipboard_to_string)>\":::Subroutine::D\n\
	Menu Actions:\"<(new|open|open-dialog|open_dialog|open-selected|open_selected|close|save|save-as|save_as|save-as-dialog|save_as_dialog|revert-to-saved|revert_to_saved|revert_to_saved_dialog|include-file|include_file|include-file-dialog|include_file_dialog|load-macro-file|load_macro_file|load-macro-file-dialog|load_macro_file_dialog|load-tags-file|load_tags_file|load-tags-file-dialog|load_tags_file_dialog|print|print-selection|print_selection|exit|undo|redo|delete|select-all|select_all|shift-left|shift_left|shift-left-by-tab|shift_left_by_tab|shift-right|shift_right|shift-right-by-tab|shift_right_by_tab|find|find-dialog|find_dialog|find-again|find_again|find-selection|find_selection|replace|replace-dialog|replace_dialog|replace-all|replace_all|replace-in-selection|replace_in_selection|replace-again|replace_again|goto-line-number|goto_line_number|goto-line-number-dialog|goto_line_number_dialog|goto-selected|goto_selected|mark|mark-dialog|mark_dialog|goto-mark|goto_mark|goto-mark-dialog|goto_mark_dialog|match|find-definition|find_definition|split-window|split_window|close-pane|close_pane|uppercase|lowercase|fill-paragraph|fill_paragraph|control-code-dialog|control_code_dialog|filter-selection-dialog|filter_selection_dialog|filter-selection|filter_selection|execute-command|execute_command|execute-command-dialog|execute_command_dialog|execute-command-line|execute_command_line|shell-menu-command|shell_menu_command|macro-menu-command|macro_menu_command|bg_menu_command|post_window_bg_menu|beginning-of-selection|beginning_of_selection|end-of-selection|end_of_selection|repeat_macro|repeat_dialog)>\":::Subroutine::D\n\
	Text Actions:\"<(self-insert|self_insert|grab-focus|grab_focus|extend-adjust|extend_adjust|extend-start|extend_start|extend-end|extend_end|secondary-adjust|secondary_adjust|secondary-or-drag-adjust|secondary_or_drag_adjust|secondary-start|secondary_start|secondary-or-drag-start|secondary_or_drag_start|process-bdrag|process_bdrag|move-destination|move_destination|move-to|move_to|move-to-or-end-drag|move_to_or_end_drag|end_drag|copy-to|copy_to|copy-to-or-end-drag|copy_to_or_end_drag|exchange|process-cancel|process_cancel|paste-clipboard|paste_clipboard|copy-clipboard|copy_clipboard|cut-clipboard|cut_clipboard|copy-primary|copy_primary|cut-primary|cut_primary|newline|newline-and-indent|newline_and_indent|newline-no-indent|newline_no_indent|delete-selection|delete_selection|delete-previous-character|delete_previous_character|delete-next-character|delete_next_character|delete-previous-word|delete_previous_word|delete-next-word|delete_next_word|delete-to-start-of-line|delete_to_start_of_line|delete-to-end-of-line|delete_to_end_of_line|forward-character|forward_character|backward-character|backward_character|key-select|key_select|process-up|process_up|process-down|process_down|process-shift-up|process_shift_up|process-shift-down|process_shift_down|process-home|process_home|forward-word|forward_word|backward-word|backward_word|forward-paragraph|forward_paragraph|backward-paragraph|backward_paragraph|beginning-of-line|beginning_of_line|end-of-line|end_of_line|beginning-of-file|beginning_of_file|end-of-file|end_of_file|next-page|next_page|previous-page|previous_page|page-left|page_left|page-right|page_right|toggle-overstrike|toggle_overstrike|scroll-up|scroll_up|scroll-down|scroll_down|scroll-to-line|scroll_to_line|select-all|select_all|deselect-all|deselect_all|focusIn|focusOut|process-return|process_return|process-tab|process_tab|insert-string|insert_string|mouse_pan)>\":::Subroutine::D\n\
	Keyword:\"<(while|if|else|for|break|continue|return|define)>\":::Keyword::D\n\
	Braces:\"[{}]\":::Keyword::\n\
	Global Variable:\"\\$[A-Za-z0-9_]*\":::Identifier1::\n\
	String:\"\"\"\":\"\"\"\":\"\\n\":String::\n\
	String Escape Char:\"\\\\(.|\\n)\":::String1:String:\n\
	Numeric Const:\"<-?[0-9]+>\":::Numeric Const::}"
    };

/*
** Read a string (from the  value of the styles resource) containing highlight
** styles information, parse it, and load it into the stored highlight style
** list (HighlightStyles) for this NEdit session.
*/
int LoadStylesString(char *inString)
{    
    char *errMsg, *fontStr, *inPtr = inString;
    highlightStyleRec *hs;
    int i;

    for (;;) {
   	
	/* skip over blank space */
	inPtr += strspn(inPtr, " \t");

	/* Allocate a language mode structure in which to store the info. */
	hs = (highlightStyleRec *)XtMalloc(sizeof(highlightStyleRec));

	/* read style name */
	hs->name = ReadSymbolicField(&inPtr);
	if (hs->name == NULL)
    	    return styleError(inString,inPtr, "style name required");
	if (!SkipDelimiter(&inPtr, &errMsg)) {
	    XtFree(hs->name);
	    XtFree((char *)hs);
    	    return styleError(inString,inPtr, errMsg);
    	}
    	
    	/* read color */
	hs->color = ReadSymbolicField(&inPtr);
	if (hs->color == NULL) {
	    XtFree(hs->name);
	    XtFree((char *)hs);
    	    return styleError(inString,inPtr, "color name required");
	}
	if (!SkipDelimiter(&inPtr, &errMsg)) {
	    freeHighlightStyleRec(hs);
    	    return styleError(inString,inPtr, errMsg);
    	}
    	
	/* read the font type */
	fontStr = ReadSymbolicField(&inPtr);
	for (i=0; i<N_FONT_TYPES; i++) {
	    if (!strcmp(FontTypeNames[i], fontStr)) {
	    	hs->font = i;
	    	break;
	    }
	}
	if (i == N_FONT_TYPES) {
	    XtFree(fontStr);
	    freeHighlightStyleRec(hs);
	    return styleError(inString, inPtr, "unrecognized font type");
	}
	XtFree(fontStr);

   	/* pattern set was read correctly, add/change it in the list */
   	for (i=0; i<NHighlightStyles; i++) {
	    if (!strcmp(HighlightStyles[i]->name, hs->name)) {
		freeHighlightStyleRec(HighlightStyles[i]);
		HighlightStyles[i] = hs;
		break;
	    }
	}
	if (i == NHighlightStyles) {
	    HighlightStyles[NHighlightStyles++] = hs;
   	    if (NHighlightStyles > MAX_HIGHLIGHT_STYLES)
   		return styleError(inString, inPtr,
   	    		"maximum allowable number of styles exceeded");
	}
	
    	/* if the string ends here, we're done */
   	inPtr += strspn(inPtr, " \t\n");
    	if (*inPtr == '\0')
    	    return True;
    }
}

/*
** Create a string in the correct format for the styles resource, containing
** all of the highlight styles information from the stored highlight style
** list (HighlightStyles) for this NEdit session.
*/
char *WriteStylesString(void)
{
    int i;
    char *outStr;
    textBuffer *outBuf;
    highlightStyleRec *style;
    
    outBuf = BufCreate();
    for (i=0; i<NHighlightStyles; i++) {
    	style = HighlightStyles[i];
    	BufInsert(outBuf, outBuf->length, "\t");
    	BufInsert(outBuf, outBuf->length, style->name);
    	BufInsert(outBuf, outBuf->length, ":");
    	BufInsert(outBuf, outBuf->length, style->color);
    	BufInsert(outBuf, outBuf->length, ":");
    	BufInsert(outBuf, outBuf->length, FontTypeNames[style->font]);
    	BufInsert(outBuf, outBuf->length, "\\n\\\n");
    }
    
    /* Get the output, and lop off the trailing newlines */
    outStr = BufGetRange(outBuf, 0, outBuf->length - (i==1?0:4));
    BufFree(outBuf);
    return outStr;
}

/*
** Read a string representing highlight pattern sets and add them
** to the PatternSets list of loaded highlight patterns.  Note that the
** patterns themselves are not parsed until they are actually used.
*/
int LoadHighlightString(char *inString)
{
    char *inPtr = inString;
    patternSet *patSet;
    int i;
    
    for (;;) {
   	
   	/* Read each pattern set, abort on error */
   	patSet = readPatternSet(&inPtr);
   	if (patSet == NULL)
   	    return False;
   	
	/* Add/change the pattern set in the list */
	for (i=0; i<NPatternSets; i++) {
	    if (!strcmp(PatternSets[i]->languageMode, patSet->languageMode)) {
		freePatternSet(PatternSets[i]);
		PatternSets[i] = patSet;
		break;
	    }
	}
	if (i == NPatternSets) {
	    PatternSets[NPatternSets++] = patSet;
   	    if (NPatternSets > MAX_PATTERN_SETS)
   		return False;
	}
	
    	/* if the string ends here, we're done */
   	inPtr += strspn(inPtr, " \t\n");
    	if (*inPtr == '\0')
    	    return True;
    }
}

/*
** Create a string in the correct format for the highlightPatterns resource,
** containing all of the highlight pattern information from the stored
** highlight pattern list (PatternSets) for this NEdit session.
*/
char *WriteHighlightString(void)
{
    char *outStr, *str, *escapedStr;
    textBuffer *outBuf;
    int psn, written = False;
    patternSet *patSet;
    
    outBuf = BufCreate();
    for (psn=0; psn<NPatternSets; psn++) {
    	patSet = PatternSets[psn];
    	if (patSet->nPatterns == 0)
    	    continue;
    	written = True;
    	BufInsert(outBuf, outBuf->length, patSet->languageMode);
    	BufInsert(outBuf, outBuf->length, ":");
    	if (isDefaultPatternSet(patSet))
    	    BufInsert(outBuf, outBuf->length, "Default\n\t");
    	else {
    	    BufInsert(outBuf, outBuf->length, intToStr(patSet->lineContext));
    	    BufInsert(outBuf, outBuf->length, ":");
    	    BufInsert(outBuf, outBuf->length, intToStr(patSet->charContext));
    	    BufInsert(outBuf, outBuf->length, "{\n");
    	    BufInsert(outBuf, outBuf->length,
    	    	    str = createPatternsString(patSet, "\t\t"));
    	    XtFree(str);
    	    BufInsert(outBuf, outBuf->length, "\t}\n\t");
    	}
    }
    
    /* Get the output string, and lop off the trailing newline and tab */
    outStr = BufGetRange(outBuf, 0, outBuf->length - (written?2:0));
    BufFree(outBuf);
    
    /* Protect newlines and backslashes from translation by the resource
       reader */
    escapedStr = EscapeSensitiveChars(outStr);
    XtFree(outStr);
    return escapedStr;
}

/*
** Find the font (font struct) associated with a named style.
** This routine must only be called with a valid styleName (call
** NamedStyleExists to find out whether styleName is valid).
*/
XFontStruct *FontOfNamedStyle(WindowInfo *window, char *styleName)
{
    int fontNum = HighlightStyles[lookupNamedStyle(styleName)]->font;
    XFontStruct *font;
    
    if (fontNum == BOLD_FONT)
    	font = window->boldFontStruct;
    else if (fontNum == ITALIC_FONT)
    	font = window->italicFontStruct;
    else if (fontNum == BOLD_ITALIC_FONT)
    	font = window->boldItalicFontStruct;
    else /* fontNum == PLAIN_FONT */
    	font = GetDefaultFontStruct(window->fontList);
    
    /* If font isn't loaded, silently substitute primary font */
    return font == NULL ? GetDefaultFontStruct(window->fontList) : font;
}

/*
** Find the color associated with a named style.  This routine must only be
** called with a valid styleName (call NamedStyleExists to find out whether
** styleName is valid).
*/
char *ColorOfNamedStyle(char *styleName)
{
    return HighlightStyles[lookupNamedStyle(styleName)]->color;
}

/*
** Determine whether a named style exists
*/
int NamedStyleExists(char *styleName)
{
    return lookupNamedStyle(styleName) != -1;
}

/*
** Look through the list of pattern sets, and find the one for a particular
** language.  Returns NULL if not found.
*/
patternSet *FindPatternSet(char *langModeName)
{
    int i;
    
    if (langModeName == NULL)
    	return NULL;
	
    for (i=0; i<NPatternSets; i++)
    	if (!strcmp(langModeName, PatternSets[i]->languageMode))
    	    return PatternSets[i];
    return NULL;
    
}

/*
** Returns True if there are highlight patterns, or potential patterns
** not yet committed in the syntax highlighting dialog for a language mode,
*/
int LMHasHighlightPatterns(char *languageMode)
{
    if (FindPatternSet(languageMode) != NULL)
    	return True;
    return HighlightDialog.shell!=NULL && !strcmp(HighlightDialog.langModeName,
    	    languageMode) && HighlightDialog.nPatterns != 0;
}

/*
** Change the language mode name of pattern sets for language "oldName" to
** "newName" in both the stored patterns, and the pattern set currently being
** edited in the dialog.
*/
void RenameHighlightPattern(char *oldName, char *newName)
{
    int i;
    
    for (i=0; i<NPatternSets; i++) {
    	if (!strcmp(oldName, PatternSets[i]->languageMode)) {
    	    XtFree(PatternSets[i]->languageMode);
    	    PatternSets[i]->languageMode = CopyAllocatedString(newName);
    	}
    }
    if (HighlightDialog.shell != NULL) {
    	if (!strcmp(HighlightDialog.langModeName, oldName)) {
    	    XtFree(HighlightDialog.langModeName);
    	    HighlightDialog.langModeName = CopyAllocatedString(newName);
    	}
    }
}

/*
** Create a pulldown menu pane with the names of the current highlight styles.
** XmNuserData for each item contains a pointer to the name.
*/
static Widget createHighlightStylesMenu(Widget parent)
{
    Widget menu;
    int i;
    XmString s1;

    menu = XmCreatePulldownMenu(parent, "highlightStyles", NULL, 0);
    for (i=0; i<NHighlightStyles; i++) {
        XtVaCreateManagedWidget("highlightStyles", xmPushButtonWidgetClass,menu,
    	      XmNlabelString, s1=XmStringCreateSimple(HighlightStyles[i]->name),
    	      XmNuserData, (void *)HighlightStyles[i]->name, 0);
        XmStringFree(s1);
    }
    return menu;
}

static char *createPatternsString(patternSet *patSet, char *indentStr)
{
    char *outStr, *str;
    textBuffer *outBuf;
    int pn;
    highlightPattern *pat;
    
    outBuf = BufCreate();
    for (pn=0; pn<patSet->nPatterns; pn++) {
    	pat = &patSet->patterns[pn];
    	BufInsert(outBuf, outBuf->length, indentStr);
    	BufInsert(outBuf, outBuf->length, pat->name);
    	BufInsert(outBuf, outBuf->length, ":");
    	if (pat->startRE != NULL) {
    	    BufInsert(outBuf, outBuf->length,
    	    	    str=MakeQuotedString(pat->startRE));
    	    XtFree(str);
    	}
    	BufInsert(outBuf, outBuf->length, ":");
    	if (pat->endRE != NULL) {
    	    BufInsert(outBuf, outBuf->length, str=MakeQuotedString(pat->endRE));
    	    XtFree(str);
    	}
    	BufInsert(outBuf, outBuf->length, ":");
    	if (pat->errorRE != NULL) {
    	    BufInsert(outBuf, outBuf->length,
    	    	    str=MakeQuotedString(pat->errorRE));
    	    XtFree(str);
    	}
    	BufInsert(outBuf, outBuf->length, ":");
    	BufInsert(outBuf, outBuf->length, pat->style);
    	BufInsert(outBuf, outBuf->length, ":");
    	if (pat->subPatternOf != NULL)
    	    BufInsert(outBuf, outBuf->length, pat->subPatternOf);
    	BufInsert(outBuf, outBuf->length, ":");
    	if (pat->flags & DEFER_PARSING)
    	    BufInsert(outBuf, outBuf->length, "D");
    	if (pat->flags & PARSE_SUBPATS_FROM_START)
    	    BufInsert(outBuf, outBuf->length, "R");
    	if (pat->flags & COLOR_ONLY)
    	    BufInsert(outBuf, outBuf->length, "C");
    	BufInsert(outBuf, outBuf->length, "\n");
    }
    outStr = BufGetAll(outBuf);
    BufFree(outBuf);
    return outStr;
}

/*
** Read in a pattern set character string, and advance *inPtr beyond it.
** Returns NULL and outputs an error to stderr on failure.
*/
static patternSet *readPatternSet(char **inPtr)
{
    char *errMsg, *stringStart = *inPtr;
    patternSet patSet, *retPatSet;

    /* remove leading whitespace */
    *inPtr += strspn(*inPtr, " \t\n");

    /* read language mode field */
    patSet.languageMode = ReadSymbolicField(inPtr);
    if (patSet.languageMode == NULL)
    	return highlightError(stringStart, *inPtr,
    	    	"language mode must be specified");
    if (!SkipDelimiter(inPtr, &errMsg))
    	return highlightError(stringStart, *inPtr, errMsg);

    /* look for "Default" keyword, and if it's there, return the default
       pattern set */
    if (!strncmp(*inPtr, "Default", 7)) {
    	*inPtr += 7;
    	retPatSet = readDefaultPatternSet(patSet.languageMode);
    	XtFree(patSet.languageMode);
    	if (retPatSet == NULL)
    	    return highlightError(stringStart, *inPtr,
    	    	    "No default pattern set");
    	return retPatSet;
    }
    	
    /* read line context field */
    if (!ReadNumericField(inPtr, &patSet.lineContext))
	return highlightError(stringStart, *inPtr,
	    	"unreadable line context field");
    if (!SkipDelimiter(inPtr, &errMsg))
    	return highlightError(stringStart, *inPtr, errMsg);

    /* read character context field */
    if (!ReadNumericField(inPtr, &patSet.charContext))
	return highlightError(stringStart, *inPtr,
	    	"unreadable character context field");

    /* read pattern list */
    patSet.patterns = readHighlightPatterns(inPtr,
   	    True, &errMsg, &patSet.nPatterns);
    if (patSet.patterns == NULL)
	return highlightError(stringStart, *inPtr, errMsg);

    /* pattern set was read correctly, make an allocated copy and return */
    retPatSet = (patternSet *)XtMalloc(sizeof(patternSet));
    memcpy(retPatSet, &patSet, sizeof(patternSet));
    return retPatSet;
}

/*
** Parse a set of highlight patterns into an array of highlightPattern
** structures, and a language mode name.  If unsuccessful, returns NULL with
** (statically allocated) message in "errMsg".
*/
static highlightPattern *readHighlightPatterns(char **inPtr, int withBraces,
    	char **errMsg, int *nPatterns)
{    
    highlightPattern *pat, *returnedList, patternList[MAX_PATTERNS];
   
    /* skip over blank space */
    *inPtr += strspn(*inPtr, " \t\n");
    
    /* look for initial brace */
    if (withBraces) {
	if (**inPtr != '{') {
    	    *errMsg = "pattern list must begin with \"{\"";
    	    return False;
	}
	(*inPtr)++;
    }
    
    /*
    ** parse each pattern in the list
    */
    pat = patternList;
    while (True) {
    	*inPtr += strspn(*inPtr, " \t\n");
    	if (**inPtr == '\0') {
    	    if (withBraces) {
    		*errMsg = "end of pattern list not found";
    		return NULL;
    	    } else
    	    	break;
	} else if (**inPtr == '}') {
	    (*inPtr)++;
    	    break;
    	}
    	if (!readHighlightPattern(inPtr, errMsg, pat++))
    	    return NULL;
    	if (pat - patternList > MAX_PATTERNS) {
    	    *errMsg = "max number of patterns exceeded\n";
    	    return NULL;
    	}
    }
    
    /* allocate a more appropriately sized list to return patterns */
    *nPatterns = pat - patternList;
    returnedList = (highlightPattern *)XtMalloc(
    	    sizeof(highlightPattern) * *nPatterns);
    memcpy(returnedList, patternList, sizeof(highlightPattern) * *nPatterns);
    return returnedList;
}

static int readHighlightPattern(char **inPtr, char **errMsg,
    	highlightPattern *pattern)
{
    /* read the name field */
    pattern->name = ReadSymbolicField(inPtr);
    if (pattern->name == NULL) {
    	*errMsg = "pattern name is required";
    	return False;
    }
    if (!SkipDelimiter(inPtr, errMsg))
    	return False;
    
    /* read the start pattern */
    if (!ReadQuotedString(inPtr, errMsg, &pattern->startRE))
    	return False;
    if (!SkipDelimiter(inPtr, errMsg))
    	return False;
    
    /* read the end pattern */
    if (**inPtr == ':')
    	pattern->endRE = NULL;
    else if (!ReadQuotedString(inPtr, errMsg, &pattern->endRE))
    	return False;
    if (!SkipDelimiter(inPtr, errMsg))
    	return False;
    
    /* read the error pattern */
    if (**inPtr == ':')
    	pattern->errorRE = NULL;
    else if (!ReadQuotedString(inPtr, errMsg, &pattern->errorRE))
    	return False;
    if (!SkipDelimiter(inPtr, errMsg))
    	return False;
    
    /* read the style field */
    pattern->style = ReadSymbolicField(inPtr);
    if (pattern->style == NULL) {
    	*errMsg = "style field required in pattern";
    	return False;
    }
    if (!SkipDelimiter(inPtr, errMsg))
    	return False;
    
    /* read the sub-pattern-of field */
    pattern->subPatternOf = ReadSymbolicField(inPtr);
    if (!SkipDelimiter(inPtr, errMsg))
    	return False;
    	
    /* read flags field */
    pattern->flags = 0;
    for (; **inPtr != '\n' && **inPtr != '}'; (*inPtr)++) {
	if (**inPtr == 'D')
	    pattern->flags |= DEFER_PARSING;
	else if (**inPtr == 'R')
	    pattern->flags |= PARSE_SUBPATS_FROM_START;
	else if (**inPtr == 'C')
	    pattern->flags |= COLOR_ONLY;
	else if (**inPtr != ' ' && **inPtr != '\t') {
	    *errMsg = "unreadable flag field";
	    return False;
	}
    }
    return True;
}

/*
** Given a language mode name, determine if there is a default (built-in)
** pattern set available for that language mode, and if so, read it and
** return a new allocated copy of it.  The returned pattern set should be
** freed by the caller with freePatternSet()
*/
static patternSet *readDefaultPatternSet(char *langModeName)
{
    int i, modeNameLen;
    char *strPtr;
    
    modeNameLen = strlen(langModeName);
    for (i=0; i<N_DEFAULT_PATTERN_SETS; i++) {
    	if (!strncmp(langModeName, DefaultPatternSets[i], modeNameLen) &&
    	    	DefaultPatternSets[i][modeNameLen] == ':') {
    	    strPtr = DefaultPatternSets[i];
    	    return readPatternSet(&strPtr);
    	}
    }
    return NULL;
}

/*
** Return True if patSet exactly matches one of the default pattern sets
*/
static int isDefaultPatternSet(patternSet *patSet)
{
    patternSet *defaultPatSet;
    int retVal;
    
    defaultPatSet = readDefaultPatternSet(patSet->languageMode);
    if (defaultPatSet == NULL)
    	return False;
    retVal = !patternSetsDiffer(patSet, defaultPatSet);
    freePatternSet(defaultPatSet);
    return retVal;
}

/*
** Short-hand functions for formating and outputing errors for
*/
static patternSet *highlightError(char *stringStart, char *stoppedAt,
    	char *message)
{
    ParseError(NULL, stringStart, stoppedAt, "highlight pattern", message);
    return NULL;
}
static int styleError(char *stringStart, char *stoppedAt, char *message)
{
    ParseError(NULL, stringStart, stoppedAt, "style specification", message);
    return False;
}

/*
** Present a dialog for editing highlight style information
*/
void EditHighlightStyles(Widget parent, char *initialStyle)
{
#define HS_LIST_RIGHT 60
#define HS_LEFT_MARGIN_POS 1
#define HS_RIGHT_MARGIN_POS 99
#define HS_H_MARGIN 10
    Widget form, nameLbl, topLbl, colorLbl, fontLbl;
    Widget fontBox, sep1, okBtn, applyBtn, dismissBtn;
    XmString s1;
    int i, ac;
    Arg args[20];

    /* if the dialog is already displayed, just pop it to the top and return */
    if (HSDialog.shell != NULL) {
	if (initialStyle != NULL)
	    setStyleByName(initialStyle);
    	RaiseShellWindow(HSDialog.shell);
    	return;
    }
    
    /* Copy the list of highlight style information to one that the user
       can freely edit (via the dialog and managed-list code) */
    HSDialog.highlightStyleList = (highlightStyleRec **)XtMalloc(
    	    sizeof(highlightStyleRec *) * MAX_HIGHLIGHT_STYLES);
    for (i=0; i<NHighlightStyles; i++)
    	HSDialog.highlightStyleList[i] =
    	copyHighlightStyleRec(HighlightStyles[i]);
    HSDialog.nHighlightStyles = NHighlightStyles;
    
    /* Create a form widget in an application shell */
    ac = 0;
    XtSetArg(args[ac], XmNdeleteResponse, XmDO_NOTHING); ac++;
    XtSetArg(args[ac], XmNiconName, "Highlight Styles"); ac++;
    XtSetArg(args[ac], XmNtitle, "Highlight Styles"); ac++;
    HSDialog.shell = XtAppCreateShell(APP_NAME, APP_CLASS,
	    applicationShellWidgetClass, TheDisplay, args, ac);
    AddSmallIcon(HSDialog.shell);
    form = XtVaCreateManagedWidget("editHighlightStyles", xmFormWidgetClass,
	    HSDialog.shell, XmNautoUnmanage, False,
	    XmNresizePolicy, XmRESIZE_NONE, 0);
    XtAddCallback(form, XmNdestroyCallback, hsDestroyCB, NULL);
    AddMotifCloseCallback(HSDialog.shell, hsDismissCB, NULL);
        
    topLbl = XtVaCreateManagedWidget("topLabel", xmLabelGadgetClass, form,
    	    XmNlabelString, s1=MKSTRING(
"To modify the properties of an existing highlight style, select the name\n\
from the list on the left.  Select \"New\" to add a new style to the list."),
	    XmNmnemonic, 'N',
	    XmNtopAttachment, XmATTACH_POSITION,
	    XmNtopPosition, 2,
	    XmNleftAttachment, XmATTACH_POSITION,
	    XmNleftPosition, HS_LEFT_MARGIN_POS,
	    XmNrightAttachment, XmATTACH_POSITION,
	    XmNrightPosition, HS_RIGHT_MARGIN_POS, 0);
    XmStringFree(s1);
    
    nameLbl = XtVaCreateManagedWidget("nameLbl", xmLabelGadgetClass, form,
    	    XmNlabelString, s1=XmStringCreateSimple("Name"),
    	    XmNmnemonic, 'm',
    	    XmNalignment, XmALIGNMENT_BEGINNING,
	    XmNleftAttachment, XmATTACH_POSITION,
    	    XmNleftPosition, HS_LIST_RIGHT,
	    XmNtopAttachment, XmATTACH_WIDGET,
	    XmNtopOffset, HS_H_MARGIN,
	    XmNtopWidget, topLbl, 0);
    XmStringFree(s1);
 
    HSDialog.nameW = XtVaCreateManagedWidget("name", xmTextWidgetClass, form,
	    XmNleftAttachment, XmATTACH_POSITION,
	    XmNleftPosition, HS_LIST_RIGHT,
	    XmNtopAttachment, XmATTACH_WIDGET,
	    XmNtopWidget, nameLbl,
	    XmNrightAttachment, XmATTACH_POSITION,
	    XmNrightPosition, HS_RIGHT_MARGIN_POS, 0);
    RemapDeleteKey(HSDialog.nameW);
    XtVaSetValues(nameLbl, XmNuserData, HSDialog.nameW, 0);
    
    colorLbl = XtVaCreateManagedWidget("colorLbl", xmLabelGadgetClass, form,
    	    XmNlabelString, s1=XmStringCreateSimple("Color"),
    	    XmNmnemonic, 'C',
    	    XmNalignment, XmALIGNMENT_BEGINNING,
	    XmNleftAttachment, XmATTACH_POSITION,
    	    XmNleftPosition, HS_LIST_RIGHT,
	    XmNtopAttachment, XmATTACH_WIDGET,
	    XmNtopOffset, HS_H_MARGIN,
	    XmNtopWidget, HSDialog.nameW, 0);
    XmStringFree(s1);
 
    HSDialog.colorW = XtVaCreateManagedWidget("color", xmTextWidgetClass, form,
	    XmNleftAttachment, XmATTACH_POSITION,
	    XmNleftPosition, HS_LIST_RIGHT,
	    XmNtopAttachment, XmATTACH_WIDGET,
	    XmNtopWidget, colorLbl,
	    XmNrightAttachment, XmATTACH_POSITION,
	    XmNrightPosition, HS_RIGHT_MARGIN_POS, 0);
    RemapDeleteKey(HSDialog.colorW);
    XtVaSetValues(colorLbl, XmNuserData, HSDialog.colorW, 0);
    
    fontLbl = XtVaCreateManagedWidget("fontLbl", xmLabelGadgetClass, form,
    	    XmNlabelString, s1=XmStringCreateSimple("Font"),
    	    XmNalignment, XmALIGNMENT_BEGINNING,
	    XmNleftAttachment, XmATTACH_POSITION,
    	    XmNleftPosition, HS_LIST_RIGHT,
	    XmNtopAttachment, XmATTACH_WIDGET,
	    XmNtopOffset, HS_H_MARGIN,
	    XmNtopWidget, HSDialog.colorW, 0);
    XmStringFree(s1);

    fontBox = XtVaCreateManagedWidget("fontBox", xmRowColumnWidgetClass, form,
    	    XmNpacking, XmPACK_COLUMN,
    	    XmNnumColumns, 2,
    	    XmNradioBehavior, True,
	    XmNleftAttachment, XmATTACH_POSITION,
    	    XmNleftPosition, HS_LIST_RIGHT,
	    XmNtopAttachment, XmATTACH_WIDGET,
	    XmNtopWidget, fontLbl, 0);
    HSDialog.plainW = XtVaCreateManagedWidget("plain", 
    	    xmToggleButtonWidgetClass, fontBox,
    	    XmNset, True,
    	    XmNlabelString, s1=XmStringCreateSimple("Plain"),
    	    XmNmnemonic, 'P', 0);
    XmStringFree(s1);
    HSDialog.boldW = XtVaCreateManagedWidget("bold", 
    	    xmToggleButtonWidgetClass, fontBox,
    	    XmNlabelString, s1=XmStringCreateSimple("Bold"),
    	    XmNmnemonic, 'B', 0);
    XmStringFree(s1);
    HSDialog.italicW = XtVaCreateManagedWidget("italic", 
    	    xmToggleButtonWidgetClass, fontBox,
    	    XmNlabelString, s1=XmStringCreateSimple("Italic"),
    	    XmNmnemonic, 'I', 0);
    XmStringFree(s1);
    HSDialog.boldItalicW = XtVaCreateManagedWidget("boldItalic", 
    	    xmToggleButtonWidgetClass, fontBox,
    	    XmNlabelString, s1=XmStringCreateSimple("Bold Italic"),
    	    XmNmnemonic, 'o', 0);
    XmStringFree(s1);
    	    
    okBtn = XtVaCreateManagedWidget("ok",xmPushButtonWidgetClass,form,
    	    XmNlabelString, s1=XmStringCreateSimple("OK"),
    	    XmNleftAttachment, XmATTACH_POSITION,
    	    XmNleftPosition, 10,
    	    XmNrightAttachment, XmATTACH_POSITION,
    	    XmNrightPosition, 30,
    	    XmNbottomAttachment, XmATTACH_POSITION,
    	    XmNbottomPosition, 99, 0);
    XtAddCallback(okBtn, XmNactivateCallback, hsOkCB, NULL);
    XmStringFree(s1);

    applyBtn = XtVaCreateManagedWidget("apply",xmPushButtonWidgetClass,form,
    	    XmNlabelString, s1=XmStringCreateSimple("Apply"),
    	    XmNmnemonic, 'A',
    	    XmNleftAttachment, XmATTACH_POSITION,
    	    XmNleftPosition, 40,
    	    XmNrightAttachment, XmATTACH_POSITION,
    	    XmNrightPosition, 60,
    	    XmNbottomAttachment, XmATTACH_POSITION,
    	    XmNbottomPosition, 99, 0);
    XtAddCallback(applyBtn, XmNactivateCallback, hsApplyCB, NULL);
    XmStringFree(s1);

    dismissBtn = XtVaCreateManagedWidget("dismiss",xmPushButtonWidgetClass,form,
    	    XmNlabelString, s1=XmStringCreateSimple("Dismiss"),
    	    XmNleftAttachment, XmATTACH_POSITION,
    	    XmNleftPosition, 70,
    	    XmNrightAttachment, XmATTACH_POSITION,
    	    XmNrightPosition, 90,
    	    XmNbottomAttachment, XmATTACH_POSITION,
    	    XmNbottomPosition, 99, 0);
    XtAddCallback(dismissBtn, XmNactivateCallback, hsDismissCB, NULL);
    XmStringFree(s1);
    
    sep1 = XtVaCreateManagedWidget("sep1", xmSeparatorGadgetClass, form,
	    XmNleftAttachment, XmATTACH_FORM,
	    XmNtopAttachment, XmATTACH_WIDGET,
	    XmNtopWidget, fontBox,
	    XmNtopOffset, HS_H_MARGIN,
 	    XmNrightAttachment, XmATTACH_FORM,
	    XmNbottomAttachment, XmATTACH_WIDGET,
    	    XmNbottomWidget, dismissBtn, 0,
	    XmNbottomOffset, HS_H_MARGIN, 0);
    
    ac = 0;
    XtSetArg(args[ac], XmNtopAttachment, XmATTACH_WIDGET); ac++;
    XtSetArg(args[ac], XmNtopOffset, HS_H_MARGIN); ac++;
    XtSetArg(args[ac], XmNtopWidget, topLbl); ac++;
    XtSetArg(args[ac], XmNleftAttachment, XmATTACH_POSITION); ac++;
    XtSetArg(args[ac], XmNleftPosition, HS_LEFT_MARGIN_POS); ac++;
    XtSetArg(args[ac], XmNrightAttachment, XmATTACH_POSITION); ac++;
    XtSetArg(args[ac], XmNrightPosition, HS_LIST_RIGHT-1); ac++;
    XtSetArg(args[ac], XmNbottomAttachment, XmATTACH_WIDGET); ac++;
    XtSetArg(args[ac], XmNbottomWidget, sep1); ac++;
    XtSetArg(args[ac], XmNbottomOffset, HS_H_MARGIN); ac++;
    HSDialog.managedListW = CreateManagedList(form, "list", args, ac,
    	    (void **)HSDialog.highlightStyleList, &HSDialog.nHighlightStyles,
    	    MAX_HIGHLIGHT_STYLES, 20, hsGetDisplayedCB, NULL, hsSetDisplayedCB,
    	    NULL, hsFreeItemCB);
    XtVaSetValues(topLbl, XmNuserData, HSDialog.managedListW, 0);
 
    /* Set initial default button */
    XtVaSetValues(form, XmNdefaultButton, okBtn, 0);
    XtVaSetValues(form, XmNcancelButton, dismissBtn, 0);
    
    /* If there's a suggestion for an initial selection, make it */
    if (initialStyle != NULL)
	setStyleByName(initialStyle);
    
    /* Handle mnemonic selection of buttons and focus to dialog */
    AddDialogMnemonicHandler(form);
    
    /* Realize all of the widgets in the new dialog */
    XtRealizeWidget(HSDialog.shell);
}

static void hsDestroyCB(Widget w, XtPointer clientData, XtPointer callData)
{
    int i;
    
    for (i=0; i<HSDialog.nHighlightStyles; i++)
    	freeHighlightStyleRec(HSDialog.highlightStyleList[i]);
    XtFree((char *)HSDialog.highlightStyleList);
}

static void hsOkCB(Widget w, XtPointer clientData, XtPointer callData)
{
    if (!updateHSList())
    	return;

    /* pop down and destroy the dialog */
    XtDestroyWidget(HSDialog.shell);
    HSDialog.shell = NULL;
}

static void hsApplyCB(Widget w, XtPointer clientData, XtPointer callData)
{
    updateHSList();
}

static void hsDismissCB(Widget w, XtPointer clientData, XtPointer callData)
{
    /* pop down and destroy the dialog */
    XtDestroyWidget(HSDialog.shell);
    HSDialog.shell = NULL;
}

static void *hsGetDisplayedCB(void *oldItem, int explicitRequest, int *abort,
    	void *cbArg)
{
    highlightStyleRec *hs;
    
    /* If the dialog is currently displaying the "new" entry and the
       fields are empty, that's just fine */
    if (oldItem == NULL && hsDialogEmpty())
    	return NULL;
    
    /* If there are no problems reading the data, just return it */
    hs = readHSDialogFields(True);
    if (hs != NULL)
    	return (void *)hs;
    
    /* If there are problems, and the user didn't ask for the fields to be
       read, give more warning */
    if (!explicitRequest) {
	if (DialogF(DF_WARN, HSDialog.shell, 2,
    		"Discard incomplete entry\nfor current highlight style?",
    		"Keep",
    		"Discard") == 2) {
     	    return oldItem == NULL ? NULL :
     	    	    (void *)copyHighlightStyleRec((highlightStyleRec *)oldItem);
	}
    }

    /* Do readHSDialogFields again without "silent" mode to display warning */
    hs = readHSDialogFields(False);
    *abort = True;
    return NULL;
}

static void hsSetDisplayedCB(void *item, void *cbArg)
{
    highlightStyleRec *hs = (highlightStyleRec *)item;

    if (item == NULL) {
    	XmTextSetString(HSDialog.nameW, "");
    	XmTextSetString(HSDialog.colorW, "");
    	XmToggleButtonSetState(HSDialog.plainW, True, False);
    	XmToggleButtonSetState(HSDialog.boldW, False, False);
    	XmToggleButtonSetState(HSDialog.italicW, False, False);
    	XmToggleButtonSetState(HSDialog.boldItalicW, False, False);
    } else {
    	XmTextSetString(HSDialog.nameW, hs->name);
    	XmTextSetString(HSDialog.colorW, hs->color);
    	XmToggleButtonSetState(HSDialog.plainW, hs->font==PLAIN_FONT, False);
    	XmToggleButtonSetState(HSDialog.boldW, hs->font==BOLD_FONT, False);
    	XmToggleButtonSetState(HSDialog.italicW, hs->font==ITALIC_FONT, False);
    	XmToggleButtonSetState(HSDialog.boldItalicW, hs->font==BOLD_ITALIC_FONT,
    	        False);
    }
}

static void hsFreeItemCB(void *item)
{
    freeHighlightStyleRec((highlightStyleRec *)item);
}

static highlightStyleRec *readHSDialogFields(int silent)
{
    highlightStyleRec *hs;
    Display *display = XtDisplay(HSDialog.shell);
    int screenNum = XScreenNumberOfScreen(XtScreen(HSDialog.shell));
    XColor rgb;

    /* Allocate a language mode structure to return */
    hs = (highlightStyleRec *)XtMalloc(sizeof(highlightStyleRec));

    /* read the name field */
    hs->name = ReadSymbolicFieldTextWidget(HSDialog.nameW,
    	    "highlight style name", silent);
    if (hs->name == NULL) {
    	XtFree((char *)hs);
    	return NULL;
    }
    if (*hs->name == '\0') {
    	if (!silent) {
    	    DialogF(DF_WARN, HSDialog.shell, 1,
    		   "Please specify a name\nfor the highlight style", "Dismiss");
    	    XmProcessTraversal(HSDialog.nameW, XmTRAVERSE_CURRENT);
    	}
    	XtFree(hs->name);
    	XtFree((char *)hs);
   	return NULL;
    }

    /* read the color field */
    hs->color = ReadSymbolicFieldTextWidget(HSDialog.colorW, "color", silent);
    if (hs->color == NULL) {
    	XtFree(hs->name);
    	XtFree((char *)hs);
    	return NULL;
    }
    if (*hs->color == '\0') {
    	if (!silent) {
    	    DialogF(DF_WARN, HSDialog.shell, 1,
    		   "Please specify a color\nfor the highlight style",
    		   "Dismiss");
    	    XmProcessTraversal(HSDialog.colorW, XmTRAVERSE_CURRENT);
    	}
    	XtFree(hs->name);
    	XtFree(hs->color);
    	XtFree((char *)hs);
   	return NULL;
    }

    /* Verify that the color is a valid X color spec */
    if (!XParseColor(display, DefaultColormap(display, screenNum), 
	    hs->color, &rgb)) {
	if (!silent) {
	    DialogF(DF_WARN, HSDialog.shell, 1,
		  "Invalid X color specification: %s\n",  "Dismiss", hs->color);
    	    XmProcessTraversal(HSDialog.colorW, XmTRAVERSE_CURRENT);
    	}
    	XtFree(hs->name);
    	XtFree(hs->color);
    	XtFree((char *)hs);
	return NULL;;
    }
    
    /* read the font buttons */
    if (XmToggleButtonGetState(HSDialog.boldW))
    	hs->font = BOLD_FONT;
    else if (XmToggleButtonGetState(HSDialog.italicW))
    	hs->font = ITALIC_FONT;
    else if (XmToggleButtonGetState(HSDialog.boldItalicW))
    	hs->font = BOLD_ITALIC_FONT;
    else
    	hs->font = PLAIN_FONT;

    return hs;
}

/*
** Copy a highlightStyleRec data structure, and all of the allocated memory
** it contains.
*/
static highlightStyleRec *copyHighlightStyleRec(highlightStyleRec *hs)
{
    highlightStyleRec *newHS;
    
    newHS = (highlightStyleRec *)XtMalloc(sizeof(highlightStyleRec));
    newHS->name = XtMalloc(strlen(hs->name)+1);
    strcpy(newHS->name, hs->name);
    if (hs->color == NULL)
    	newHS->color = NULL;
    else {
	newHS->color = XtMalloc(strlen(hs->color)+1);
	strcpy(newHS->color, hs->color);
    }
    newHS->font = hs->font;
    return newHS;
}

/*
** Free all of the allocated data in a highlightStyleRec, including the
** structure itself.
*/
static void freeHighlightStyleRec(highlightStyleRec *hs)
{
    XtFree(hs->name);
    if (hs->color != NULL)
    	XtFree(hs->color);
    XtFree((char *)hs);
}

/*
** Select a particular style in the highlight styles dialog
*/
static void setStyleByName(char *style)
{
    int i;
    
    for (i=0; i<HSDialog.nHighlightStyles; i++) {
    	if (!strcmp(HSDialog.highlightStyleList[i]->name, style)) {
    	    SelectManagedListItem(HSDialog.managedListW, i);
    	    break;
    	}
    }
}

/*
** Return True if the fields of the highlight styles dialog are consistent
** with a blank "New" style in the dialog.
*/
static int hsDialogEmpty(void)
{
    return TextWidgetIsBlank(HSDialog.nameW) &&
 	    TextWidgetIsBlank(HSDialog.colorW) &&
	    XmToggleButtonGetState(HSDialog.plainW);
}   	

/*
** Apply the changes made in the highlight styles dialog to the stored
** highlight style information in HighlightStyles
*/
static int updateHSList(void)
{
    WindowInfo *window;
    int i;
    
    /* Get the current contents of the dialog fields */
    if (!UpdateManagedList(HSDialog.managedListW, True))
    	return False;
    
    /* Replace the old highlight styles list with the new one from the dialog */
    for (i=0; i<NHighlightStyles; i++)
    	freeHighlightStyleRec(HighlightStyles[i]);
    for (i=0; i<HSDialog.nHighlightStyles; i++)
    	HighlightStyles[i] =
    	    	copyHighlightStyleRec(HSDialog.highlightStyleList[i]);
    NHighlightStyles = HSDialog.nHighlightStyles;
    
    /* If a syntax highlighting dialog is up, update its menu */
    updateHighlightStyleMenu();
    
    /* Redisplay highlighted windows which use changed style(s) */
    for (window=WindowList; window!=NULL; window=window->next)
    	UpdateHighlightStyles(window);
    
    /* Note that preferences have been changed */
    MarkPrefsChanged();

    return True;
}

/*
** Present a dialog for editing highlight pattern information
*/
void EditHighlightPatterns(WindowInfo *window)
{
#define BORDER 4
#define LIST_RIGHT 41
    Widget form, lmOptMenu, lmLbl, patternsForm, patternsFrame, patternsLbl;
    Widget lmForm, contextFrame, contextForm, contextLbl, styleLbl, styleBtn;
    Widget okBtn, applyBtn, checkBtn, deleteBtn, dismissBtn, helpBtn;
    Widget restoreBtn, nameLbl, typeLbl, typeBox, lmBtn, matchBox;
    patternSet *patSet;
    XmString s1;
    int i, n, nPatterns;
    Arg args[20];

    /* if the dialog is already displayed, just pop it to the top and return */
    if (HighlightDialog.shell != NULL) {
    	RaiseShellWindow(HighlightDialog.shell);
    	return;
    }
    
    if (LanguageModeName(0) == NULL) {
    	DialogF(DF_WARN, window->shell, 1, "No Language Modes available \
for syntax highlighting\nAdd language modes under Preferenses->Language Modes",
		"Dismiss");
    	return;
    }
    
    /* Decide on an initial language mode */
    HighlightDialog.langModeName = CopyAllocatedString(
    	    LanguageModeName(window->languageMode == PLAIN_LANGUAGE_MODE ? 0 :
    	    window->languageMode));

    /* Find the associated pattern set (patSet) to edit */
    patSet = FindPatternSet(HighlightDialog.langModeName);
    
    /* Copy the list of patterns to one that the user can freely edit */
    HighlightDialog.patterns = (highlightPattern **)XtMalloc(
    	    sizeof(highlightPattern *) * MAX_PATTERNS);
    nPatterns = patSet == NULL ? 0 : patSet->nPatterns;
    for (i=0; i<nPatterns; i++)
    	HighlightDialog.patterns[i] = copyPatternSrc(&patSet->patterns[i],NULL);
    HighlightDialog.nPatterns = nPatterns;


    /* Create a form widget in an application shell */
    n = 0;
    XtSetArg(args[n], XmNdeleteResponse, XmDO_NOTHING); n++;
    XtSetArg(args[n], XmNiconName, "Highlight Patterns"); n++;
    XtSetArg(args[n], XmNtitle, "Syntax Highlighting Patterns"); n++;
    HighlightDialog.shell = XtAppCreateShell(APP_NAME, APP_CLASS,
	    applicationShellWidgetClass, TheDisplay, args, n);
    AddSmallIcon(HighlightDialog.shell);
    form = XtVaCreateManagedWidget("editHighlightPatterns", xmFormWidgetClass,
	    HighlightDialog.shell, XmNautoUnmanage, False,
	    XmNresizePolicy, XmRESIZE_NONE, 0);
    XtAddCallback(form, XmNdestroyCallback, destroyCB, NULL);
    AddMotifCloseCallback(HighlightDialog.shell, dismissCB, NULL);

    lmForm = XtVaCreateManagedWidget("lmForm", xmFormWidgetClass,
    	    form,
	    XmNleftAttachment, XmATTACH_POSITION,
	    XmNleftPosition, 1,
	    XmNtopAttachment, XmATTACH_POSITION,
	    XmNtopPosition, 1,
	    XmNrightAttachment, XmATTACH_POSITION,
	    XmNrightPosition, 99, 0);
 
    HighlightDialog.lmPulldown = CreateLanguageModeMenu(lmForm, langModeCB,
    	    NULL);
    n = 0;
    XtSetArg(args[n], XmNspacing, 0); n++;
    XtSetArg(args[n], XmNmarginWidth, 0); n++;
    XtSetArg(args[n], XmNtopAttachment, XmATTACH_FORM); n++;
    XtSetArg(args[n], XmNleftAttachment, XmATTACH_POSITION); n++;
    XtSetArg(args[n], XmNleftPosition, 50); n++;
    XtSetArg(args[n], XmNsubMenuId, HighlightDialog.lmPulldown); n++;
    lmOptMenu = XmCreateOptionMenu(lmForm, "langModeOptMenu", args, n);
    XtManageChild(lmOptMenu);
    HighlightDialog.lmOptMenu = lmOptMenu;
    
    lmLbl = XtVaCreateManagedWidget("lmLbl", xmLabelGadgetClass, lmForm,
    	    XmNlabelString, s1=XmStringCreateSimple("Language Mode:"),
    	    XmNmnemonic, 'M',
    	    XmNuserData, XtParent(HighlightDialog.lmOptMenu),
    	    XmNalignment, XmALIGNMENT_END,
	    XmNrightAttachment, XmATTACH_POSITION,
	    XmNrightPosition, 50,
	    XmNtopAttachment, XmATTACH_FORM,
	    XmNbottomAttachment, XmATTACH_OPPOSITE_WIDGET,
	    XmNbottomWidget, lmOptMenu, 0);
    XmStringFree(s1);
    
    lmBtn = XtVaCreateManagedWidget("lmBtn", xmPushButtonWidgetClass, lmForm,
    	    XmNlabelString, s1=MKSTRING("Add / Modify\nLanguage Mode..."),
    	    XmNmnemonic, 'A',
    	    XmNrightAttachment, XmATTACH_FORM,
    	    XmNtopAttachment, XmATTACH_FORM, 0);
    XtAddCallback(lmBtn, XmNactivateCallback, lmDialogCB, NULL);
    XmStringFree(s1);
    
    okBtn = XtVaCreateManagedWidget("ok", xmPushButtonWidgetClass, form,
    	    XmNlabelString, s1=XmStringCreateSimple("OK"),
    	    XmNleftAttachment, XmATTACH_POSITION,
    	    XmNleftPosition, 1,
    	    XmNrightAttachment, XmATTACH_POSITION,
    	    XmNrightPosition, 13,
    	    XmNbottomAttachment, XmATTACH_FORM,
    	    XmNbottomOffset, BORDER, 0);
    XtAddCallback(okBtn, XmNactivateCallback, okCB, NULL);
    XmStringFree(s1);
    
    applyBtn = XtVaCreateManagedWidget("apply", xmPushButtonWidgetClass, form,
    	    XmNlabelString, s1=XmStringCreateSimple("Apply"),
    	    XmNmnemonic, 'y',
    	    XmNleftAttachment, XmATTACH_POSITION,
    	    XmNleftPosition, 13,
    	    XmNrightAttachment, XmATTACH_POSITION,
    	    XmNrightPosition, 26,
    	    XmNbottomAttachment, XmATTACH_FORM,
    	    XmNbottomOffset, BORDER, 0);
    XtAddCallback(applyBtn, XmNactivateCallback, applyCB, NULL);
    XmStringFree(s1);
    
    checkBtn = XtVaCreateManagedWidget("check", xmPushButtonWidgetClass, form,
    	    XmNlabelString, s1=XmStringCreateSimple("Check"),
    	    XmNmnemonic, 'k',
    	    XmNleftAttachment, XmATTACH_POSITION,
    	    XmNleftPosition, 26,
    	    XmNrightAttachment, XmATTACH_POSITION,
    	    XmNrightPosition, 39,
    	    XmNbottomAttachment, XmATTACH_FORM,
    	    XmNbottomOffset, BORDER, 0);
    XtAddCallback(checkBtn, XmNactivateCallback, checkCB, NULL);
    XmStringFree(s1);
    
    deleteBtn = XtVaCreateManagedWidget("delete", xmPushButtonWidgetClass, form,
    	    XmNlabelString, s1=XmStringCreateSimple("Delete"),
    	    XmNmnemonic, 'D',
    	    XmNleftAttachment, XmATTACH_POSITION,
    	    XmNleftPosition, 39,
    	    XmNrightAttachment, XmATTACH_POSITION,
    	    XmNrightPosition, 52,
    	    XmNbottomAttachment, XmATTACH_FORM,
    	    XmNbottomOffset, BORDER, 0);
    XtAddCallback(deleteBtn, XmNactivateCallback, deleteCB, NULL);
    XmStringFree(s1);
    
    restoreBtn = XtVaCreateManagedWidget("restore", xmPushButtonWidgetClass, form,
    	    XmNlabelString, s1=XmStringCreateSimple("Restore Defaults"),
    	    XmNmnemonic, 'f',
    	    XmNleftAttachment, XmATTACH_POSITION,
    	    XmNleftPosition, 52,
    	    XmNrightAttachment, XmATTACH_POSITION,
    	    XmNrightPosition, 73,
    	    XmNbottomAttachment, XmATTACH_FORM,
    	    XmNbottomOffset, BORDER, 0);
    XtAddCallback(restoreBtn, XmNactivateCallback, restoreCB, NULL);
    XmStringFree(s1);
    
    dismissBtn = XtVaCreateManagedWidget("dismiss", xmPushButtonWidgetClass,
    	    form,
    	    XmNlabelString, s1=XmStringCreateSimple("Dismiss"),
    	    XmNleftAttachment, XmATTACH_POSITION,
    	    XmNleftPosition, 73,
    	    XmNrightAttachment, XmATTACH_POSITION,
    	    XmNrightPosition, 86,
    	    XmNbottomAttachment, XmATTACH_FORM,
    	    XmNbottomOffset, BORDER, 0);
    XtAddCallback(dismissBtn, XmNactivateCallback, dismissCB, NULL);
    XmStringFree(s1);
    
    helpBtn = XtVaCreateManagedWidget("help", xmPushButtonWidgetClass,
    	    form,
    	    XmNlabelString, s1=XmStringCreateSimple("Help"),
    	    XmNmnemonic, 'H',
    	    XmNleftAttachment, XmATTACH_POSITION,
    	    XmNleftPosition, 86,
    	    XmNrightAttachment, XmATTACH_POSITION,
    	    XmNrightPosition, 99,
    	    XmNbottomAttachment, XmATTACH_FORM,
    	    XmNbottomOffset, BORDER, 0);
    XtAddCallback(helpBtn, XmNactivateCallback, helpCB, NULL);
    XmStringFree(s1);
    
    contextFrame = XtVaCreateManagedWidget("contextFrame", xmFrameWidgetClass,
    	    form,
	    XmNleftAttachment, XmATTACH_POSITION,
	    XmNleftPosition, 1,
	    XmNrightAttachment, XmATTACH_POSITION,
	    XmNrightPosition, 99,
	    XmNbottomAttachment, XmATTACH_WIDGET,
    	    XmNbottomWidget, okBtn,
    	    XmNbottomOffset, BORDER, 0);
    contextForm = XtVaCreateManagedWidget("contextForm", xmFormWidgetClass,
	    contextFrame, 0);
    contextLbl = XtVaCreateManagedWidget("contextLbl",
    	    xmLabelGadgetClass, contextFrame,
    	    XmNlabelString, s1=XmStringCreateSimple(
    	      "Context requirements for incremental re-parsing after changes"),
	    XmNchildType, XmFRAME_TITLE_CHILD, 0);
    XmStringFree(s1);
    
    HighlightDialog.lineContextW = XtVaCreateManagedWidget("lineContext",
    	    xmTextWidgetClass, contextForm,
	    XmNcolumns, 5,
	    XmNmaxLength, 12,
	    XmNleftAttachment, XmATTACH_POSITION,
	    XmNleftPosition, 15,
	    XmNrightAttachment, XmATTACH_POSITION,
	    XmNrightPosition, 25, 0);
    RemapDeleteKey(HighlightDialog.lineContextW);
    
    XtVaCreateManagedWidget("lineContLbl",
    	    xmLabelGadgetClass, contextForm,
    	    XmNlabelString, s1=XmStringCreateSimple("lines"),
    	    XmNmnemonic, 'l',
    	    XmNuserData, HighlightDialog.lineContextW,
    	    XmNalignment, XmALIGNMENT_BEGINNING,
	    XmNleftAttachment, XmATTACH_WIDGET,
	    XmNleftWidget, HighlightDialog.lineContextW,
	    XmNtopAttachment, XmATTACH_OPPOSITE_WIDGET,
	    XmNtopWidget, HighlightDialog.lineContextW,
	    XmNbottomAttachment, XmATTACH_OPPOSITE_WIDGET,
    	    XmNbottomWidget, HighlightDialog.lineContextW, 0);
    XmStringFree(s1);

    HighlightDialog.charContextW = XtVaCreateManagedWidget("charContext",
    	    xmTextWidgetClass, contextForm,
	    XmNcolumns, 5,
	    XmNmaxLength, 12,
	    XmNleftAttachment, XmATTACH_POSITION,
	    XmNleftPosition, 58,
	    XmNrightAttachment, XmATTACH_POSITION,
	    XmNrightPosition, 68, 0);
    RemapDeleteKey(HighlightDialog.lineContextW);
    
    XtVaCreateManagedWidget("charContLbl",
    	    xmLabelGadgetClass, contextForm,
    	    XmNlabelString, s1=XmStringCreateSimple("characters"),
    	    XmNmnemonic, 'c',
    	    XmNuserData, HighlightDialog.charContextW,
    	    XmNalignment, XmALIGNMENT_BEGINNING,
	    XmNleftAttachment, XmATTACH_WIDGET,
	    XmNleftWidget, HighlightDialog.charContextW,
	    XmNtopAttachment, XmATTACH_OPPOSITE_WIDGET,
	    XmNtopWidget, HighlightDialog.charContextW,
	    XmNbottomAttachment, XmATTACH_OPPOSITE_WIDGET,
    	    XmNbottomWidget, HighlightDialog.charContextW, 0);
    XmStringFree(s1);
    
    patternsFrame = XtVaCreateManagedWidget("patternsFrame", xmFrameWidgetClass,
    	    form,
	    XmNleftAttachment, XmATTACH_POSITION,
	    XmNleftPosition, 1,
	    XmNtopAttachment, XmATTACH_WIDGET,
	    XmNtopWidget, lmForm,
	    XmNrightAttachment, XmATTACH_POSITION,
	    XmNrightPosition, 99,
	    XmNbottomAttachment, XmATTACH_WIDGET,
    	    XmNbottomWidget, contextFrame,
    	    XmNbottomOffset, BORDER, 0);
    patternsForm = XtVaCreateManagedWidget("patternsForm", xmFormWidgetClass,
	    patternsFrame, 0);
    patternsLbl = XtVaCreateManagedWidget("patternsLbl", xmLabelGadgetClass,
    	    patternsFrame,
    	    XmNlabelString, s1=XmStringCreateSimple("Patterns"),
    	    XmNmnemonic, 'P',
    	    XmNmarginHeight, 0,
	    XmNchildType, XmFRAME_TITLE_CHILD, 0);
    XmStringFree(s1);
    
    typeLbl = XtVaCreateManagedWidget("typeLbl", xmLabelGadgetClass,
    	    patternsForm,
    	    XmNlabelString, s1=XmStringCreateSimple("Pattern Type:"),
    	    XmNmarginHeight, 0,
    	    XmNalignment, XmALIGNMENT_BEGINNING,
	    XmNleftAttachment, XmATTACH_POSITION,
    	    XmNleftPosition, LIST_RIGHT,
	    XmNtopAttachment, XmATTACH_FORM, 0);
    XmStringFree(s1);

    typeBox = XtVaCreateManagedWidget("typeBox", xmRowColumnWidgetClass,
    	    patternsForm,
    	    XmNpacking, XmPACK_COLUMN,
    	    XmNradioBehavior, True,
	    XmNleftAttachment, XmATTACH_POSITION,
    	    XmNleftPosition, LIST_RIGHT,
	    XmNtopAttachment, XmATTACH_WIDGET,
	    XmNtopWidget, typeLbl, 0);
    HighlightDialog.topLevelW = XtVaCreateManagedWidget("top", 
    	    xmToggleButtonWidgetClass, typeBox,
    	    XmNset, True,
    	    XmNmarginHeight, 0,
    	    XmNlabelString, s1=XmStringCreateSimple(
    	        "Pass-1 (applied to all text when loaded or modified)"),
    	    XmNmnemonic, '1', 0);
    XmStringFree(s1);
    XtAddCallback(HighlightDialog.topLevelW, XmNvalueChangedCallback,
    	    patTypeCB, NULL);
    HighlightDialog.deferredW = XtVaCreateManagedWidget("deferred", 
    	    xmToggleButtonWidgetClass, typeBox,
    	    XmNmarginHeight, 0,
    	    XmNlabelString, s1=XmStringCreateSimple(
    	        "Pass-2 (parsing is deferred until text is exposed)"),
    	    XmNmnemonic, '2', 0);
    XmStringFree(s1);
    XtAddCallback(HighlightDialog.deferredW, XmNvalueChangedCallback,
    	    patTypeCB, NULL);
    HighlightDialog.subPatW = XtVaCreateManagedWidget("subPat", 
    	    xmToggleButtonWidgetClass, typeBox,
    	    XmNmarginHeight, 0,
    	    XmNlabelString, s1=XmStringCreateSimple(
    	    	"Sub-pattern (processed within start & end of parent)"),
    	    XmNmnemonic, 'u', 0);
    XmStringFree(s1);
    XtAddCallback(HighlightDialog.subPatW, XmNvalueChangedCallback,
    	    patTypeCB, NULL);
    HighlightDialog.colorPatW = XtVaCreateManagedWidget("color", 
    	    xmToggleButtonWidgetClass, typeBox,
    	    XmNmarginHeight, 0,
    	    XmNlabelString, s1=XmStringCreateSimple(
    	    	"Coloring for sub-expressions of parent pattern"),
    	    XmNmnemonic, 'g', 0);
    XmStringFree(s1);
    XtAddCallback(HighlightDialog.colorPatW, XmNvalueChangedCallback,
    	    patTypeCB, NULL);

    HighlightDialog.matchLbl = XtVaCreateManagedWidget("matchLbl",
    	    xmLabelGadgetClass, patternsForm,
    	    XmNlabelString, s1=XmStringCreateSimple("Matching:"),
    	    XmNmarginHeight, 0,
    	    XmNalignment, XmALIGNMENT_BEGINNING,
	    XmNleftAttachment, XmATTACH_POSITION,
    	    XmNleftPosition, LIST_RIGHT,
	    XmNtopAttachment, XmATTACH_WIDGET,
	    XmNtopOffset, BORDER,
	    XmNtopWidget, typeBox, 0);
    XmStringFree(s1);

    matchBox = XtVaCreateManagedWidget("matchBox", xmRowColumnWidgetClass,
    	    patternsForm,
    	    XmNpacking, XmPACK_COLUMN,
    	    XmNradioBehavior, True,
	    XmNleftAttachment, XmATTACH_POSITION,
    	    XmNleftPosition, LIST_RIGHT,
	    XmNtopAttachment, XmATTACH_WIDGET,
	    XmNtopWidget, HighlightDialog.matchLbl, 0);
    HighlightDialog.simpleW = XtVaCreateManagedWidget("simple", 
    	    xmToggleButtonWidgetClass, matchBox,
    	    XmNset, True,
    	    XmNmarginHeight, 0,
    	    XmNlabelString, s1=XmStringCreateSimple(
    	    	"Highlight text matching regular expression"),
    	    XmNmnemonic, 'x', 0);
    XmStringFree(s1);
    XtAddCallback(HighlightDialog.simpleW, XmNvalueChangedCallback,
    	    matchTypeCB, NULL);
    HighlightDialog.rangeW = XtVaCreateManagedWidget("range", 
    	    xmToggleButtonWidgetClass, matchBox,
    	    XmNmarginHeight, 0,
    	    XmNlabelString, s1=XmStringCreateSimple(
    	    	"Highlight text between starting and ending REs"),
    	    XmNmnemonic, 'b', 0);
    XmStringFree(s1);
    XtAddCallback(HighlightDialog.rangeW, XmNvalueChangedCallback,
    	    matchTypeCB, NULL);

    nameLbl = XtVaCreateManagedWidget("nameLbl", xmLabelGadgetClass,
    	    patternsForm,
    	    XmNlabelString, s1=XmStringCreateSimple("Pattern Name"),
    	    XmNmnemonic, 'N',
    	    XmNrows, 20,
    	    XmNalignment, XmALIGNMENT_BEGINNING,
	    XmNleftAttachment, XmATTACH_POSITION,
    	    XmNleftPosition, LIST_RIGHT,
	    XmNtopAttachment, XmATTACH_WIDGET,
	    XmNtopWidget, matchBox,
	    XmNtopOffset, BORDER, 0);
    XmStringFree(s1);
 
    HighlightDialog.nameW = XtVaCreateManagedWidget("name", xmTextWidgetClass,
    	    patternsForm,
	    XmNleftAttachment, XmATTACH_POSITION,
	    XmNleftPosition, LIST_RIGHT,
	    XmNtopAttachment, XmATTACH_WIDGET,
	    XmNtopWidget, nameLbl,
	    XmNrightAttachment, XmATTACH_POSITION,
	    XmNrightPosition, (99 + LIST_RIGHT)/2, 0);
    RemapDeleteKey(HighlightDialog.nameW);
    XtVaSetValues(nameLbl, XmNuserData, HighlightDialog.nameW, 0);

    HighlightDialog.parentLbl = XtVaCreateManagedWidget("parentLbl",
    	    xmLabelGadgetClass, patternsForm,
    	    XmNlabelString, s1=XmStringCreateSimple("Parent Pattern"),
    	    XmNmnemonic, 't',
    	    XmNrows, 20,
    	    XmNalignment, XmALIGNMENT_BEGINNING,
	    XmNleftAttachment, XmATTACH_POSITION,
    	    XmNleftPosition, (99 + LIST_RIGHT)/2 + 1,
	    XmNtopAttachment, XmATTACH_WIDGET,
	    XmNtopWidget, matchBox,
	    XmNtopOffset, BORDER, 0);
    XmStringFree(s1);
 
    HighlightDialog.parentW = XtVaCreateManagedWidget("parent",
    	    xmTextWidgetClass, patternsForm,
	    XmNleftAttachment, XmATTACH_POSITION,
	    XmNleftPosition, (99 + LIST_RIGHT)/2 + 1,
	    XmNtopAttachment, XmATTACH_WIDGET,
	    XmNtopWidget, HighlightDialog.parentLbl,
	    XmNrightAttachment, XmATTACH_POSITION,
	    XmNrightPosition, 99, 0);
    RemapDeleteKey(HighlightDialog.parentW);
    XtVaSetValues(HighlightDialog.parentLbl, XmNuserData,
    	    HighlightDialog.parentW, 0);

    HighlightDialog.startLbl = XtVaCreateManagedWidget("startLbl",
    	    xmLabelGadgetClass, patternsForm,
    	    XmNalignment, XmALIGNMENT_BEGINNING,
	    XmNmnemonic, 'R',
	    XmNtopAttachment, XmATTACH_WIDGET,
	    XmNtopWidget, HighlightDialog.parentW,
	    XmNtopOffset, BORDER,
	    XmNleftAttachment, XmATTACH_POSITION,
    	    XmNleftPosition, 1, 0);
 
    HighlightDialog.errorW = XtVaCreateManagedWidget("error",
    	    xmTextWidgetClass, patternsForm,
	    XmNleftAttachment, XmATTACH_POSITION,
	    XmNleftPosition, 1,
	    XmNrightAttachment, XmATTACH_POSITION,
	    XmNrightPosition, 99,
	    XmNbottomAttachment, XmATTACH_POSITION,
	    XmNbottomPosition, 99, 0);
    RemapDeleteKey(HighlightDialog.errorW);

    HighlightDialog.errorLbl = XtVaCreateManagedWidget("errorLbl",
    	    xmLabelGadgetClass, patternsForm,
    	    XmNlabelString, s1=XmStringCreateSimple(
    	    	"Regular Expression Indicating Error in Match (Optional)"),
    	    XmNmnemonic, 'o',
    	    XmNuserData, HighlightDialog.errorW,
    	    XmNalignment, XmALIGNMENT_BEGINNING,
	    XmNleftAttachment, XmATTACH_POSITION,
    	    XmNleftPosition, 1,
	    XmNbottomAttachment, XmATTACH_WIDGET,
	    XmNbottomWidget, HighlightDialog.errorW, 0);
    XmStringFree(s1);
 
    HighlightDialog.endW = XtVaCreateManagedWidget("end",
    	    xmTextWidgetClass, patternsForm,
	    XmNleftAttachment, XmATTACH_POSITION,
	    XmNleftPosition, 1,
	    XmNbottomAttachment, XmATTACH_WIDGET,
	    XmNbottomWidget, HighlightDialog.errorLbl,
	    XmNbottomOffset, BORDER,
	    XmNrightAttachment, XmATTACH_POSITION,
	    XmNrightPosition, 99, 0);
    RemapDeleteKey(HighlightDialog.endW);

    HighlightDialog.endLbl = XtVaCreateManagedWidget("endLbl",
    	    xmLabelGadgetClass, patternsForm,
    	    XmNmnemonic, 'E',
    	    XmNuserData, HighlightDialog.endW,
    	    XmNalignment, XmALIGNMENT_BEGINNING,
	    XmNleftAttachment, XmATTACH_POSITION,
    	    XmNleftPosition, 1,
	    XmNbottomAttachment, XmATTACH_WIDGET,
	    XmNbottomWidget, HighlightDialog.endW, 0);

    n = 0;
    XtSetArg(args[n], XmNeditMode, XmMULTI_LINE_EDIT); n++;
    XtSetArg(args[n], XmNscrollHorizontal, False); n++;
    XtSetArg(args[n], XmNwordWrap, True); n++;
    XtSetArg(args[n], XmNrows, 3); n++;
    XtSetArg(args[n], XmNbottomAttachment, XmATTACH_WIDGET); n++;
    XtSetArg(args[n], XmNbottomWidget, HighlightDialog.endLbl); n++;
    XtSetArg(args[n], XmNbottomOffset, BORDER); n++;
    XtSetArg(args[n], XmNtopAttachment, XmATTACH_WIDGET); n++;
    XtSetArg(args[n], XmNtopWidget, HighlightDialog.startLbl); n++;
    XtSetArg(args[n], XmNleftAttachment, XmATTACH_POSITION); n++;
    XtSetArg(args[n], XmNleftPosition, 1); n++;
    XtSetArg(args[n], XmNrightAttachment, XmATTACH_POSITION); n++;
    XtSetArg(args[n], XmNrightPosition, 99); n++;
    HighlightDialog.startW = XmCreateScrolledText(patternsForm, "start",args,n);
    XtManageChild(HighlightDialog.startW);
    MakeSingleLineTextW(HighlightDialog.startW);
    RemapDeleteKey(HighlightDialog.startW);
    XtVaSetValues(HighlightDialog.startLbl,
    		XmNuserData,HighlightDialog.startW, 0);

    styleBtn = XtVaCreateManagedWidget("styleLbl", xmPushButtonWidgetClass,
    	    patternsForm,
    	    XmNlabelString, s1=MKSTRING("Add / Modify\nStyle..."),
    	    XmNmnemonic, 'i',
	    XmNrightAttachment, XmATTACH_POSITION,
    	    XmNrightPosition, LIST_RIGHT-1,
	    XmNbottomAttachment, XmATTACH_OPPOSITE_WIDGET,
	    XmNbottomWidget, HighlightDialog.parentW, 0);
    XmStringFree(s1);
    XtAddCallback(styleBtn, XmNactivateCallback, styleDialogCB, NULL);

    HighlightDialog.stylePulldown = createHighlightStylesMenu(patternsForm);
    n = 0;
    XtSetArg(args[n], XmNspacing, 0); n++;
    XtSetArg(args[n], XmNmarginWidth, 0); n++;
    XtSetArg(args[n], XmNbottomAttachment, XmATTACH_OPPOSITE_WIDGET); n++;
    XtSetArg(args[n], XmNbottomWidget, HighlightDialog.parentW); n++;
    XtSetArg(args[n], XmNleftAttachment, XmATTACH_POSITION); n++;
    XtSetArg(args[n], XmNleftPosition, 1); n++;
    XtSetArg(args[n], XmNrightAttachment, XmATTACH_WIDGET); n++;
    XtSetArg(args[n], XmNrightWidget, styleBtn); n++;
    XtSetArg(args[n], XmNsubMenuId, HighlightDialog.stylePulldown); n++;
    HighlightDialog.styleOptMenu = XmCreateOptionMenu(patternsForm,
    		"styleOptMenu", args, n);
    XtManageChild(HighlightDialog.styleOptMenu);

    styleLbl = XtVaCreateManagedWidget("styleLbl", xmLabelGadgetClass,
    	    patternsForm,
    	    XmNlabelString, s1=XmStringCreateSimple("Highlight Style"),
    	    XmNmnemonic, 'S',
    	    XmNuserData, XtParent(HighlightDialog.styleOptMenu),
    	    XmNalignment, XmALIGNMENT_BEGINNING,
	    XmNleftAttachment, XmATTACH_POSITION,
    	    XmNleftPosition, 1,
	    XmNbottomAttachment, XmATTACH_WIDGET,
	    XmNbottomWidget, HighlightDialog.styleOptMenu, 0);
    XmStringFree(s1);

    n = 0;
    XtSetArg(args[n], XmNtopAttachment, XmATTACH_FORM); n++;
    XtSetArg(args[n], XmNleftAttachment, XmATTACH_POSITION); n++;
    XtSetArg(args[n], XmNleftPosition, 1); n++;
    XtSetArg(args[n], XmNrightAttachment, XmATTACH_POSITION); n++;
    XtSetArg(args[n], XmNrightPosition, LIST_RIGHT-1); n++;
    XtSetArg(args[n], XmNbottomAttachment, XmATTACH_WIDGET); n++;
    XtSetArg(args[n], XmNbottomWidget, styleLbl); n++;
    XtSetArg(args[n], XmNbottomOffset, BORDER); n++;
    HighlightDialog.managedListW = CreateManagedList(patternsForm, "list", args,
    	    n, (void **)HighlightDialog.patterns, &HighlightDialog.nPatterns,
    	    MAX_PATTERNS, 18, getDisplayedCB, NULL, setDisplayedCB,
    	    NULL, freeItemCB);
    XtVaSetValues(patternsLbl, XmNuserData, HighlightDialog.managedListW, 0);

    /* Set initial default button */
    XtVaSetValues(form, XmNdefaultButton, okBtn, 0);
    XtVaSetValues(form, XmNcancelButton, dismissBtn, 0);
    
    /* Handle mnemonic selection of buttons and focus to dialog */
    AddDialogMnemonicHandler(form);
    
    /* Fill in the dialog information for the selected language mode */
    SetIntText(HighlightDialog.lineContextW, patSet==NULL ? 1 :
    	    patSet->lineContext);
    SetIntText(HighlightDialog.charContextW, patSet==NULL ? 0 :
    	    patSet->charContext);
    SetLangModeMenu(HighlightDialog.lmOptMenu, HighlightDialog.langModeName);
    updateLabels();
    
    /* Realize all of the widgets in the new dialog */
    XtRealizeWidget(HighlightDialog.shell);
}

/*
** If a syntax highlighting dialog is up, ask to have the option menu for
** chosing highlight styles updated (via a call to createHighlightStylesMenu)
*/
static void updateHighlightStyleMenu(void)
{
    Widget oldMenu;
    int patIndex;
    
    if (HighlightDialog.shell == NULL)
    	return;
    
    oldMenu = HighlightDialog.stylePulldown;
    HighlightDialog.stylePulldown = createHighlightStylesMenu(
    	    XtParent(XtParent(oldMenu)));
    XtVaSetValues(XmOptionButtonGadget(HighlightDialog.styleOptMenu),
    	    XmNsubMenuId, HighlightDialog.stylePulldown, 0);
    patIndex = ManagedListSelectedIndex(HighlightDialog.managedListW);
    if (patIndex == -1)
    	setStyleMenu("Plain");
    else
    	setStyleMenu(HighlightDialog.patterns[patIndex]->style);
    
    XtDestroyWidget(oldMenu);
}

/*
** If a syntax highlighting dialog is up, ask to have the option menu for
** chosing language mode updated (via a call to CreateLanguageModeMenu)
*/
void UpdateLanguageModeMenu(void)
{
    Widget oldMenu;

    if (HighlightDialog.shell == NULL)
    	return;

    oldMenu = HighlightDialog.lmPulldown;
    HighlightDialog.lmPulldown = CreateLanguageModeMenu(
    	    XtParent(XtParent(oldMenu)), langModeCB, NULL);
    XtVaSetValues(XmOptionButtonGadget(HighlightDialog.lmOptMenu),
    	    XmNsubMenuId, HighlightDialog.lmPulldown, 0);
    SetLangModeMenu(HighlightDialog.lmOptMenu, HighlightDialog.langModeName);

    XtDestroyWidget(oldMenu);
}

static void destroyCB(Widget w, XtPointer clientData, XtPointer callData)
{
    int i;
    
    freeNonNull(HighlightDialog.langModeName);
    for (i=0; i<HighlightDialog.nPatterns; i++)
     	freePatternSrc(HighlightDialog.patterns[i], True);
    HighlightDialog.shell = NULL;
}

static void langModeCB(Widget w, XtPointer clientData, XtPointer callData)
{
    char *modeName;
    patternSet *oldPatSet, *newPatSet;
    patternSet emptyPatSet = {NULL, 1, 0, 0, NULL};
    int i, resp;
    
    /* Get the newly selected mode name.  If it's the same, do nothing */
    XtVaGetValues(w, XmNuserData, &modeName, 0);
    if (!strcmp(modeName, HighlightDialog.langModeName))
    	return;
    
    /* Look up the original version of the patterns being edited */
    oldPatSet = FindPatternSet(HighlightDialog.langModeName);
    if (oldPatSet == NULL)
    	oldPatSet = &emptyPatSet;
    
    /* Get the current information displayed by the dialog.  If it's bad,
       give the user the chance to throw it out or go back and fix it.  If
       it has changed, give the user the chance to apply discard or cancel. */
    newPatSet = getDialogPatternSet();
    if (newPatSet == NULL) {
	if (DialogF(DF_WARN, HighlightDialog.shell, 2,
    		"Discard incomplete entry\nfor current language mode?",
    		"Keep", "Discard") == 1) {
    	    SetLangModeMenu(HighlightDialog.lmOptMenu,
	    	    HighlightDialog.langModeName);
     	    return;
     	}
    } else if (patternSetsDiffer(oldPatSet, newPatSet)) {
    	resp = DialogF(DF_WARN, HighlightDialog.shell, 3,
    		"Apply changes for language mode %s?", "Apply Changes",
    		"Discard Changes", "Cancel", HighlightDialog.langModeName);
    	if (resp == 3) {
    	    SetLangModeMenu(HighlightDialog.lmOptMenu,
	    	    HighlightDialog.langModeName);
     	    return;
     	}
     	if (resp == 1)
     	    updatePatternSet();
    }
    if (newPatSet != NULL)
    	freePatternSet(newPatSet);

    /* Free the old dialog information */
    freeNonNull(HighlightDialog.langModeName);
    for (i=0; i<HighlightDialog.nPatterns; i++)
     	freePatternSrc(HighlightDialog.patterns[i], True);
    
    /* Fill the dialog with the new language mode information */
    HighlightDialog.langModeName = CopyAllocatedString(modeName);
    newPatSet = FindPatternSet(modeName);
    if (newPatSet == NULL) {
    	HighlightDialog.nPatterns = 0;
    	SetIntText(HighlightDialog.lineContextW, 1);
    	SetIntText(HighlightDialog.charContextW, 0);
    } else {
	for (i=0; i<newPatSet->nPatterns; i++)
    	    HighlightDialog.patterns[i] =
    		    copyPatternSrc(&newPatSet->patterns[i], NULL);
	HighlightDialog.nPatterns = newPatSet->nPatterns;
    	SetIntText(HighlightDialog.lineContextW, newPatSet->lineContext);
    	SetIntText(HighlightDialog.charContextW, newPatSet->charContext);
    }
    ChangeManagedListData(HighlightDialog.managedListW);
}

static void lmDialogCB(Widget w, XtPointer clientData, XtPointer callData)
{
    EditLanguageModes(HighlightDialog.shell);
}

static void styleDialogCB(Widget w, XtPointer clientData, XtPointer callData)
{
    Widget selectedItem;
    char *style;
    
    XtVaGetValues(HighlightDialog.styleOptMenu, XmNmenuHistory,&selectedItem,0);
    XtVaGetValues(selectedItem, XmNuserData, &style, 0);
    EditHighlightStyles(HighlightDialog.shell, style);
}

static void okCB(Widget w, XtPointer clientData, XtPointer callData)
{
    /* change the patterns */
    if (!updatePatternSet())
    	return;
    
    /* pop down and destroy the dialog */
    XtDestroyWidget(HighlightDialog.shell);
}

static void applyCB(Widget w, XtPointer clientData, XtPointer callData)
{
    /* change the patterns */
    updatePatternSet();
}
	
static void checkCB(Widget w, XtPointer clientData, XtPointer callData)
{
    if (checkHighlightDialogData())
	DialogF(DF_INF, HighlightDialog.shell, 1,
    		"Patterns compiled without error", "Dismiss");
}
	
static void restoreCB(Widget w, XtPointer clientData, XtPointer callData)
{
    patternSet *defaultPatSet;
    int i, psn;
    
    defaultPatSet = readDefaultPatternSet(HighlightDialog.langModeName);
    if (defaultPatSet == NULL) {
    	DialogF(DF_WARN, HighlightDialog.shell, 1,
 		"There is no default pattern set\nfor language mode %s",
 		"Dismiss", HighlightDialog.langModeName);
    	return;
    }
    
    if (DialogF(DF_WARN, HighlightDialog.shell, 2,
"Are you sure you want to discard\n\
all changes to syntax highlighting\n\
patterns for language mode %s?", "Discard", "Cancel",
	    HighlightDialog.langModeName) == 2)
    	return;
    
    /* if a stored version of the pattern set exists, replace it, if it
       doesn't, add a new one */
    for (psn=0; psn<NPatternSets; psn++)
    	if (!strcmp(HighlightDialog.langModeName,
    	    	PatternSets[psn]->languageMode))
    	    break;
    if (psn < NPatternSets) {
     	freePatternSet(PatternSets[psn]);
   	PatternSets[psn] = defaultPatSet;
    } else
    	PatternSets[NPatternSets++] = defaultPatSet;

    /* Free the old dialog information */
    for (i=0; i<HighlightDialog.nPatterns; i++)
     	freePatternSrc(HighlightDialog.patterns[i], True);
    
    /* Update the dialog */
    HighlightDialog.nPatterns = defaultPatSet->nPatterns;
    for (i=0; i<defaultPatSet->nPatterns; i++)
    	HighlightDialog.patterns[i] =
    		copyPatternSrc(&defaultPatSet->patterns[i], NULL);
    	SetIntText(HighlightDialog.lineContextW, defaultPatSet->lineContext);
    	SetIntText(HighlightDialog.charContextW, defaultPatSet->charContext);
    ChangeManagedListData(HighlightDialog.managedListW);
}
	
static void deleteCB(Widget w, XtPointer clientData, XtPointer callData)
{
    int i, psn;
    
    if (DialogF(DF_WARN, HighlightDialog.shell, 2,
"Are you sure you want to delete\n\
syntax highlighting patterns for\n\
language mode %s?", "Yes, Delete", "Cancel", HighlightDialog.langModeName) == 2)
    	return;
    
    /* if a stored version of the pattern set exists, delete it from the list */
    for (psn=0; psn<NPatternSets; psn++)
    	if (!strcmp(HighlightDialog.langModeName,
    	    	PatternSets[psn]->languageMode))
    	    break;
    if (psn < NPatternSets) {
     	freePatternSet(PatternSets[psn]);
   	memmove(&PatternSets[psn], &PatternSets[psn+1],
   	    	(NPatternSets-1 - psn) * sizeof(patternSet *));
    	NPatternSets--;
    }

    /* Free the old dialog information */
    for (i=0; i<HighlightDialog.nPatterns; i++)
     	freePatternSrc(HighlightDialog.patterns[i], True);
    
    /* Clear out the dialog */
    HighlightDialog.nPatterns = 0;
    SetIntText(HighlightDialog.lineContextW, 1);
    SetIntText(HighlightDialog.charContextW, 0);
    ChangeManagedListData(HighlightDialog.managedListW);
}

static void dismissCB(Widget w, XtPointer clientData, XtPointer callData)
{
    /* pop down and destroy the dialog */
    XtDestroyWidget(HighlightDialog.shell);
}

static void helpCB(Widget w, XtPointer clientData, XtPointer callData)
{
    Help(w, HELP_PATTERNS);
}

static void patTypeCB(Widget w, XtPointer clientData, XtPointer callData)
{
    updateLabels();
}

static void matchTypeCB(Widget w, XtPointer clientData, XtPointer callData)
{
    updateLabels();
}

static void *getDisplayedCB(void *oldItem, int explicitRequest, int *abort,
    	void *cbArg)
{
    highlightPattern *pat;
    
    /* If the dialog is currently displaying the "new" entry and the
       fields are empty, that's just fine */
    if (oldItem == NULL && dialogEmpty())
    	return NULL;
    
    /* If there are no problems reading the data, just return it */
    pat = readDialogFields(True);
    if (pat != NULL)
    	return (void *)pat;
    
    /* If there are problems, and the user didn't ask for the fields to be
       read, give more warning */
    if (!explicitRequest) {
	if (DialogF(DF_WARN, HighlightDialog.shell, 2,
    		"Discard incomplete entry\nfor current pattern?",
    		"Keep", "Discard") == 2) {
     	    return oldItem == NULL ? NULL : (void *)copyPatternSrc(
     	    	    (highlightPattern *)oldItem, NULL);
	}
    }

    /* Do readDialogFields again without "silent" mode to display warning */
    pat = readDialogFields(False);
    *abort = True;
    return NULL;
}

static void setDisplayedCB(void *item, void *cbArg)
{
    highlightPattern *pat = (highlightPattern *)item;
    int isSubpat, isDeferred, isColorOnly, isRange;

    if (item == NULL) {
    	XmTextSetString(HighlightDialog.nameW, "");
    	XmTextSetString(HighlightDialog.parentW, "");
    	XmTextSetString(HighlightDialog.startW, "");
    	XmTextSetString(HighlightDialog.endW, "");
    	XmTextSetString(HighlightDialog.errorW, "");
    	XmToggleButtonSetState(HighlightDialog.topLevelW, True, False);
    	XmToggleButtonSetState(HighlightDialog.deferredW, False, False);
    	XmToggleButtonSetState(HighlightDialog.subPatW, False, False);
    	XmToggleButtonSetState(HighlightDialog.colorPatW, False, False);
    	XmToggleButtonSetState(HighlightDialog.simpleW, True, False);
    	XmToggleButtonSetState(HighlightDialog.rangeW, False, False);
    	setStyleMenu("Plain");
    } else {
    	isSubpat = pat->subPatternOf != NULL;
    	isDeferred = pat->flags & DEFER_PARSING;
    	isColorOnly = pat->flags & COLOR_ONLY;
    	isRange = pat->endRE != NULL;
    	XmTextSetString(HighlightDialog.nameW, pat->name);
    	XmTextSetString(HighlightDialog.parentW, pat->subPatternOf);
    	XmTextSetString(HighlightDialog.startW, pat->startRE);
    	XmTextSetString(HighlightDialog.endW, pat->endRE);
    	XmTextSetString(HighlightDialog.errorW, pat->errorRE);
    	XmToggleButtonSetState(HighlightDialog.topLevelW,
    	    	!isSubpat && !isDeferred, False);
    	XmToggleButtonSetState(HighlightDialog.deferredW,
    	    	!isSubpat && isDeferred, False);
    	XmToggleButtonSetState(HighlightDialog.subPatW,
    	    	isSubpat && !isColorOnly, False);
    	XmToggleButtonSetState(HighlightDialog.colorPatW,
    	    	isSubpat && isColorOnly, False);
    	XmToggleButtonSetState(HighlightDialog.simpleW, !isRange, False);
    	XmToggleButtonSetState(HighlightDialog.rangeW, isRange, False);
    	setStyleMenu(pat->style);
    }
    updateLabels();
}

static void freeItemCB(void *item)
{
    freePatternSrc((highlightPattern *)item, True);
}

/*
** Do a test compile of the patterns currently displayed in the highlight
** patterns dialog, and display warning dialogs if there are problems
*/
static int checkHighlightDialogData(void)
{
    patternSet *patSet;
    int result;
    
    /* Get the pattern information from the dialog */
    patSet = getDialogPatternSet();
    if (patSet == NULL)
    	return False;
     
    /* Compile the patterns  */
    result = patSet->nPatterns == 0 ? True : TestHighlightPatterns(patSet);
    freePatternSet(patSet);
    return result;
}

/*
** Update the text field labels and sensitivity of various fields, based on
** the settings of the Pattern Type and Matching radio buttons in the highlight
** patterns dialog.
*/
static void updateLabels(void)
{
    char *startLbl, *endLbl;
    int endSense, errSense, matchSense, parentSense;
    XmString s1;
    
    if (XmToggleButtonGetState(HighlightDialog.colorPatW)) {
	startLbl =  "Sub-expressions to Highlight in Parent's Starting \
Regular Expression (\\1 \\2 etc.)";
	endLbl = "Sub-expressions to Highlight in Parent Pattern's Ending \
Regular Expression";
    	endSense = True;
    	errSense = False;
    	matchSense = False;
    	parentSense = True;
    } else {
    	endLbl = "Ending Regular Expression";
    	matchSense = True;
    	parentSense = XmToggleButtonGetState(HighlightDialog.subPatW);
    	if (XmToggleButtonGetState(HighlightDialog.simpleW)) {
    	    startLbl = "Regular Expression to Match";
    	    endSense = False;
    	    errSense = False;
    	} else {
    	    startLbl = "Starting Regular Expression";
    	    endSense = True;
    	    errSense = True;
	}
    }
    
    XtSetSensitive(HighlightDialog.parentLbl, parentSense);
    XtSetSensitive(HighlightDialog.parentW, parentSense);
    XtSetSensitive(HighlightDialog.endW, endSense);
    XtSetSensitive(HighlightDialog.endLbl, endSense);
    XtSetSensitive(HighlightDialog.errorW, errSense);
    XtSetSensitive(HighlightDialog.errorLbl, errSense);
    XtSetSensitive(HighlightDialog.errorLbl, errSense);
    XtSetSensitive(HighlightDialog.simpleW, matchSense);
    XtSetSensitive(HighlightDialog.rangeW, matchSense);
    XtSetSensitive(HighlightDialog.matchLbl, matchSense);
    XtVaSetValues(HighlightDialog.startLbl, XmNlabelString,
    	    s1=XmStringCreateSimple(startLbl), 0);
    XmStringFree(s1);
    XtVaSetValues(HighlightDialog.endLbl, XmNlabelString,
    	    s1=XmStringCreateSimple(endLbl), 0);
    XmStringFree(s1);
}

/*
** Set the styles menu in the currently displayed highlight dialog to show
** a particular style
*/
static void setStyleMenu(char *styleName)
{
    int i;
    Cardinal nItems;
    WidgetList items;
    Widget selectedItem;
    char *itemStyle;

    XtVaGetValues(HighlightDialog.stylePulldown, XmNchildren, &items,
    	    XmNnumChildren, &nItems, 0);
    if (nItems == 0)
    	return;
    selectedItem = items[0];
    for (i=0; i<nItems; i++) {
    	XtVaGetValues(items[i], XmNuserData, &itemStyle, 0);
    	if (!strcmp(itemStyle, styleName)) {
    	    selectedItem = items[i];
    	    break;
    	}
    }
    XtVaSetValues(HighlightDialog.styleOptMenu, XmNmenuHistory, selectedItem,0);
}

/*
** Read the pattern fields of the highlight dialog, and produce an allocated
** highlightPattern structure reflecting the contents, or pop up dialogs
** telling the user what's wrong (Passing "silent" as True, suppresses these
** dialogs).  Returns NULL on error.
*/ 
static highlightPattern *readDialogFields(int silent)
{
    highlightPattern *pat;
    char *inPtr, *outPtr, *style;
    Widget selectedItem;
    int colorOnly;

    /* Allocate a pattern source structure to return, zero out fields
       so that the whole pattern can be freed on error with freePatternSrc */
    pat = (highlightPattern *)XtMalloc(sizeof(highlightPattern));
    pat->endRE = NULL;
    pat->errorRE = NULL;
    pat->style = NULL;
    pat->subPatternOf = NULL;
    
    /* read the type buttons */
    pat->flags = 0;
    colorOnly = XmToggleButtonGetState(HighlightDialog.colorPatW);
    if (XmToggleButtonGetState(HighlightDialog.deferredW))
    	pat->flags |= DEFER_PARSING;
    else if (colorOnly)
    	pat->flags = COLOR_ONLY;

    /* read the name field */
    pat->name = ReadSymbolicFieldTextWidget(HighlightDialog.nameW,
    	    "highlight pattern name", silent);
    if (pat->name == NULL) {
    	XtFree((char *)pat);
    	return NULL;
    }
    if (*pat->name == '\0') {
    	if (!silent) {
    	    DialogF(DF_WARN, HighlightDialog.shell, 1,
    		   "Please specify a name\nfor the pattern", "Dismiss");
    	    XmProcessTraversal(HighlightDialog.nameW, XmTRAVERSE_CURRENT);
    	}
    	XtFree(pat->name);
    	XtFree((char *)pat);
   	return NULL;
    }
    
    /* read the startRE field */
    pat->startRE = XmTextGetString(HighlightDialog.startW);
    if (*pat->startRE == '\0') {
    	if (!silent) {
    	    DialogF(DF_WARN, HighlightDialog.shell, 1,
    		   "Please specify a regular\nexpression to match", "Dismiss");
    	    XmProcessTraversal(HighlightDialog.startW, XmTRAVERSE_CURRENT);
    	}
    	freePatternSrc(pat, True);
    	return NULL;
    }
    
    /* Make sure coloring patterns contain only sub-expression references
       and put it in replacement regular-expression form */
    if (colorOnly) {
	for (inPtr=pat->startRE, outPtr=pat->startRE; *inPtr!='\0'; inPtr++)
    	    if (*inPtr!=' ' && *inPtr!='\t')
    		*outPtr++ = *inPtr;
	*outPtr = '\0';
    	if (strspn(pat->startRE, "\\0123456789 \t") != strlen(pat->startRE) ||
    	    	*pat->startRE != '\\' || strstr(pat->startRE, "\\\\") != NULL) {
    	    if (!silent) {
    		DialogF(DF_WARN, HighlightDialog.shell, 1,
"The expression field in patterns which specify highlighting for\n\
a parent, must contain only sub-expression references in regular\n\
expression replacement form (\\1\\2 etc.).  See Help -> Regular\n\
Expressions and Help -> Syntax Highlighting for more information", "Dismiss");
    		XmProcessTraversal(HighlightDialog.startW, XmTRAVERSE_CURRENT);
    	    }
    	    freePatternSrc(pat, True);
    	    return NULL;
    	}
    }
    	
    /* read the parent field */
    if (XmToggleButtonGetState(HighlightDialog.subPatW) || colorOnly) {
	if (TextWidgetIsBlank(HighlightDialog.parentW)) {
    	    if (!silent) {
    		DialogF(DF_WARN, HighlightDialog.shell, 1,
    		       "Please specify a parent parent pattern", "Dismiss");
    		XmProcessTraversal(HighlightDialog.parentW, XmTRAVERSE_CURRENT);
    	    }
    	    freePatternSrc(pat, True);
    	    return NULL;
	}
	pat->subPatternOf = XmTextGetString(HighlightDialog.parentW);
    }
    
    /* read the styles option menu */
    XtVaGetValues(HighlightDialog.styleOptMenu, XmNmenuHistory,&selectedItem,0);
    XtVaGetValues(selectedItem, XmNuserData, &style, 0);
    pat->style = XtMalloc(strlen(style) + 1);
    strcpy(pat->style, style);
    
    	
    /* read the endRE field */
    if (colorOnly || XmToggleButtonGetState(HighlightDialog.rangeW)) {
	pat->endRE = XmTextGetString(HighlightDialog.endW);
	if (!colorOnly && *pat->endRE == '\0') {
            if (!silent) {
    		DialogF(DF_WARN, HighlightDialog.shell, 1,
    		       "Please specify an ending\nregular expression", "Dismiss");
    		XmProcessTraversal(HighlightDialog.endW, XmTRAVERSE_CURRENT);
    	    }
    	    freePatternSrc(pat, True);
    	    return NULL;
	}
    }
    
    /* read the errorRE field */
    if (XmToggleButtonGetState(HighlightDialog.rangeW)) {
	pat->errorRE = XmTextGetString(HighlightDialog.errorW);
	if (*pat->errorRE == '\0') {
            XtFree(pat->errorRE);
            pat->errorRE = NULL;
	}
    }
    return pat;
}

/*
** Returns true if the pattern fields of the highlight dialog are set to
** the default ("New" pattern) state.
*/
static int dialogEmpty(void)
{
    return TextWidgetIsBlank(HighlightDialog.nameW) &&
	    XmToggleButtonGetState(HighlightDialog.topLevelW) &&
	    XmToggleButtonGetState(HighlightDialog.simpleW) &&
	    TextWidgetIsBlank(HighlightDialog.parentW) &&
	    TextWidgetIsBlank(HighlightDialog.startW) &&
	    TextWidgetIsBlank(HighlightDialog.endW) &&
	    TextWidgetIsBlank(HighlightDialog.errorW);
}   	

/*
** Update the pattern set being edited in the Syntax Highlighting dialog
** with the information that the dialog is currently displaying, and
** apply changes to any window which is currently using the patterns.
*/
static int updatePatternSet(void)
{
    patternSet *patSet;
    WindowInfo *window;
    int psn;
    	
    /* Make sure the patterns are valid and compile */
    if (!checkHighlightDialogData())
    	return False;
    
    /* Get the current data */
    patSet = getDialogPatternSet();
    if (patSet == NULL)
    	return False;
    
    /* Find the pattern being modified */
    for (psn=0; psn<NPatternSets; psn++)
    	if (!strcmp(HighlightDialog.langModeName,
    	    	PatternSets[psn]->languageMode))
    	    break;
    
    /* If it's a new pattern, add it at the end, otherwise free the
       existing pattern set and replace it */
    if (psn == NPatternSets) {
    	PatternSets[NPatternSets++] = patSet;
    } else {
	freePatternSet(PatternSets[psn]);
	PatternSets[psn] = patSet;
    }
    
    /* Find windows that are currently using this pattern set and
       re-do the highlighting */
    for (window=WindowList; window!=NULL; window=window->next) {
    	if (window->highlightSyntax &&
    		window->languageMode != PLAIN_LANGUAGE_MODE) {
    	    if (!strcmp(LanguageModeName(window->languageMode),
    	    	    patSet->languageMode)) {
    	    	StopHighlighting(window);
    	    	StartHighlighting(window, True);
    	    }
    	}
    }
    
    /* Note that preferences have been changed */
    MarkPrefsChanged();

    return True;
}

/*
** Get the current information that the user has entered in the syntax
** highlighting dialog.  Return NULL if the data is currently invalid
*/
static patternSet *getDialogPatternSet(void)
{
    int i, lineContext, charContext;
    patternSet *patSet;
    
    /* Get the current contents of the "patterns" dialog fields */
    if (!UpdateManagedList(HighlightDialog.managedListW, True))
    	return NULL;
    
    /* Get the line and character context values */
    if (GetIntTextWarn(HighlightDialog.lineContextW, &lineContext,
    	    "context lines", True) != TEXT_READ_OK)
    	return NULL;
    if (GetIntTextWarn(HighlightDialog.charContextW, &charContext,
    	    "context lines", True) != TEXT_READ_OK)
    	return NULL;
    
    /* Allocate a new pattern set structure and copy the fields read from the
       dialog, including the modified pattern list into it */
    patSet = (patternSet *)XtMalloc(sizeof(patternSet));
    patSet->languageMode = CopyAllocatedString(HighlightDialog.langModeName);
    patSet->lineContext = lineContext;
    patSet->charContext = charContext;
    patSet->nPatterns = HighlightDialog.nPatterns;
    patSet->patterns = (highlightPattern *)XtMalloc(sizeof(highlightPattern) *
    	    HighlightDialog.nPatterns);
    for (i=0; i<HighlightDialog.nPatterns; i++)
    	copyPatternSrc(HighlightDialog.patterns[i], &patSet->patterns[i]);
    return patSet;
}

/*
** Return True if "patSet1" and "patSet2" differ
*/
static int patternSetsDiffer(patternSet *patSet1, patternSet *patSet2)
{
    int i;
    highlightPattern *pat1, *pat2;
    
    if (patSet1->lineContext != patSet2->lineContext)
    	return True;
    if (patSet1->charContext != patSet2->charContext)
    	return True;
    if (patSet1->nPatterns != patSet2->nPatterns)
    	return True;
    for (i=0; i<patSet2->nPatterns; i++) {
    	pat1 = &patSet1->patterns[i];
    	pat2 = &patSet2->patterns[i];
    	if (pat1->flags != pat2->flags)
    	    return True;
    	if (AllocatedStringsDiffer(pat1->name, pat2->name))
    	    return True;
    	if (AllocatedStringsDiffer(pat1->startRE, pat2->startRE))
    	    return True;
    	if (AllocatedStringsDiffer(pat1->endRE, pat2->endRE))
    	    return True;
    	if (AllocatedStringsDiffer(pat1->errorRE, pat2->errorRE))
    	    return True;
    	if (AllocatedStringsDiffer(pat1->style, pat2->style))
    	    return True;
    	if (AllocatedStringsDiffer(pat1->subPatternOf, pat2->subPatternOf))
    	    return True;
    }
    return False;
}

/*
** Copy a highlight pattern data structure and all of the allocated data
** it contains.  If "copyTo" is non-null, use that as the top-level structure,
** otherwise allocate a new highlightPattern structure and return it as the
** function value.
*/
static highlightPattern *copyPatternSrc(highlightPattern *pat,
    	highlightPattern *copyTo)
{
    highlightPattern *newPat;
    
    if (copyTo == NULL)
    	newPat = (highlightPattern *)XtMalloc(sizeof(highlightPattern));
    else
    	newPat = copyTo;
    newPat->name = CopyAllocatedString(pat->name);
    newPat->startRE = CopyAllocatedString(pat->startRE);
    newPat->endRE = CopyAllocatedString(pat->endRE);
    newPat->errorRE = CopyAllocatedString(pat->errorRE);
    newPat->style = CopyAllocatedString(pat->style);
    newPat->subPatternOf = CopyAllocatedString(pat->subPatternOf);
    newPat->flags = pat->flags;    
    return newPat;
}

static void freeNonNull(void *ptr)
{
    if (ptr != NULL)
    	XtFree((char *)ptr);
}

/*
** Free the allocated memory contained in a highlightPattern data structure
** If "freeStruct" is true, free the structure itself as well.
*/
static void freePatternSrc(highlightPattern *pat, int freeStruct)
{
    XtFree(pat->name);
    freeNonNull(pat->startRE);
    freeNonNull(pat->endRE);
    freeNonNull(pat->errorRE);
    freeNonNull(pat->style);
    freeNonNull(pat->subPatternOf);
    if (freeStruct)
    	XtFree((char *)pat);
}

/*
** Free the allocated memory contained in a patternSet data structure
** If "freeStruct" is true, free the structure itself as well.
*/
static void freePatternSet(patternSet *p)
{
    int i;
    
    for (i=0; i<p->nPatterns; i++)
    	freePatternSrc(&p->patterns[i], False);
    XtFree(p->languageMode);
    XtFree((char *)p->patterns);
    XtFree((char *)p);
}

/*
** Find the index into the HighlightStyles array corresponding to "styleName".
** If styleName is not found, return -1.
*/
static int lookupNamedStyle(char *styleName)
{
    int i;
    
    for (i=0; i<NHighlightStyles; i++)
    	if (!strcmp(styleName, HighlightStyles[i]->name))
    	    return i;
    return -1;
}

/*
** Write the string representation of int "i" to a static area, and
** return a pointer to it.
*/
static char *intToStr(int i)
{
    static char outBuf[12];
    
    sprintf(outBuf, "%d", i);
    return outBuf;
}
