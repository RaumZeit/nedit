static const char CVSID[] = "$Id: help.c,v 1.25 2001/03/13 16:48:22 slobasso Exp $";
/*******************************************************************************
*									       *
* help.c -- Nirvana Editor help display					       *
*									       *
* Copyright (C) 1999 Mark Edel						       *
*									       *
* This is free software; you can redistribute it and/or modify it under the    *
* terms of the GNU General Public License as published by the Free Software    *
* Foundation; either version 2 of the License, or (at your option) any later   *
* version.							               *
* 									       *
* This software is distributed in the hope that it will be useful, but WITHOUT *
* ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or        *
* FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Lesser General Public License *
* for more details.							       *
* 									       *
* You should have received a copy of the GNU General Public License along with *
* software; if not, write to the Free Software Foundation, Inc., 59 Temple     *
* Place, Suite 330, Boston, MA  02111-1307 USA		                       *
*									       *
* Nirvana Text Editor	    						       *
* September 10, 1991							       *
*									       *
* Written by Mark Edel							       *
*									       *
*******************************************************************************/
#ifdef VMS
#include "../util/VMSparam.h"
#else
#ifndef __MVS__
#include <sys/param.h>
#endif
#endif /*VMS*/
#include <Xm/Xm.h>
#include <Xm/Form.h>
#include <Xm/ScrolledW.h>
#include <Xm/ScrollBar.h>
#include <Xm/PushB.h>
#include "../util/misc.h"
#include "../util/DialogF.h"
#include "textBuf.h"
#include "text.h"
#include "textSel.h"
#include "nedit.h"
#include "search.h"
#include "window.h"
#include "preferences.h"
#include "help.h"
#include "file.h"

static char *HelpTitles[NUM_TOPICS] = {
"Version",
"Getting Started",
"Finding and Replacing Text",
"Selecting Text",
"Cut and Paste",
"Auto / Smart Indent",
"Tabs / Emulated Tabs",
"Programming with NEdit",
"Finding Declarations (ctags)",
"Using the Mouse",
"Keyboard Shortcuts",
"Shifting and Filling",
"File Format",
"Syntax Highlighting",
"Crash Recovery",
"Preferences",
"Shell Commands/Filters",
"Regular Expressions: Basic Syntax",
"Regular Expressions: Escape Sequences",
"Regular Expressions: Parenthetical Constructs",
"Regular Expressions: Advanced Topics",
"Regular Expressions: Examples",
"NEdit Command Line",
"Server Mode and nc",
"Customizing NEdit",
"X Resources",
"Key Binding",
"Learn/Replay",
"Macro Language",
"Macro Subroutines",
"Action Routines",
"Highlighting Patterns",
"Smart Indent Macros",
"Problems/Bugs",
"Mailing Lists",
"Distribution Policy",
"Tabs Dialog"};

static char *HelpText[NUM_TOPICS] = {
"NEdit Version 5.2 DEVELOPMENT version\n\
March, 2001\n\
\n\
NEdit was written by Mark Edel, Joy Kyriakopulos, Christopher Conrad, \
Jim Clark, Arnulfo Zepeda-Navratil, \
Suresh Ravoor, Tony Balinski, Max Vohlken, Yunliang Yu, and Donna Reid.\n\
\n\
The regular expression matching routines used in NEdit are adapted (with \
permission) from original code written by Henry Spencer at the \
University of Toronto.\n\
\n\
Syntax highlighting patterns and smart indent macros were contributed by: \
Simon T. MacDonald,  Maurice Leysens, Matt Majka, Alfred Smeenk, \
Alain Fargues, Christopher Conrad, Scott Markinson, Konrad Bernloehr, \
Ivan Herman, Patrice Venant, Christian Denat, Philippe Couton, \
Max Vohlken, Markus Schwarzenberg, Himanshu Gohel, Steven C. Kapp, \
Michael Turomsha, John Fieber, Chris Ross, Nathaniel Gray, Joachim Lous, \
Mike Duigou, and Seak, Teng-Fong.\n\
\n\
NEdit sources, executables, additional documentation, and contributed \
software are available from the NEdit web site at http://nedit.org.\n\
\n\
This program is free software; you can redistribute it and/or \
modify it under the terms of the GNU General Public License \
as published by the Free Software Foundation; either version 2 \
of the License, or (at your option) any later version.\n\
\n\
This program is distributed in the hope that it will be useful, \
but WITHOUT ANY WARRANTY; without even the implied warranty of \
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the \
GNU General Public License in the Help section \"Distribution \
Policy\" for more details.",

"Welcome to NEdit!\n\
\n\
NEdit is a standard GUI (Graphical User Interface) style text editor for \
programs and plain-text files.  Users of Macintosh and MS Windows based \
text editors should find NEdit a familiar and comfortable environment.  \
NEdit provides all of the standard menu, dialog, editing, and mouse \
support, as well as all of the standard shortcuts to which the users \
of modern GUI based environments are accustomed.  For users of older \
style Unix editors, welcome to the world of mouse-based editing!\n\
\n\
Help sections of interest to new users are listed under the \"Basic \
Operation\" heading in the top-level Help menu:\n\
\n\
	Selecting Text\n\
	Finding and Replacing Text\n\
	Cut and Paste\n\
	Using the Mouse\n\
	Keyboard Shortcuts\n\
	Shifting and Filling\n\
\n\
Programmers should also read the introductory section under the \
\"Features for Programming\" section:\n\
\n\
	Programming with NEdit\n\
\n\
If you get into trouble, the Undo command in the Edit menu can reverse \
any modifications that you make.  NEdit does not change the file you are \
editing until you tell it to Save.\n\
\n\
Editing an Existing File\n\
\n\
To open an existing file, choose Open... from the file menu. Select the \
file that you want to open in the pop-up dialog that appears and click on \
OK.  You may open any number of files at the same time.  Each file will \
appear in its own editor window.  Using Open... rather than re-typing the \
NEdit command and running additional copies of NEdit, will give you quick \
access to all of the files you have open via the Windows menu, and ensure \
that you don't accidentally open the same file twice.  NEdit has no \
\"main\" window.  It remains running as long as at least one editor window \
is open.\n\
\n\
Creating a New File\n\
\n\
If you already have an empty (Untitled) window displayed, just begin \
typing in the window.  To create a new Untitled window, choose New from \
the File menu.  To give the file a name and save its contents to the \
disk, choose Save or Save As... from the File menu.\n\
\n\
Backup Files\n\
\n\
NEdit maintains periodic backups of the file you are editing so that you \
can recover the file in the event of a problem such as a system crash, \
network failure, or X server crash.  These files are saved under the name \
~filename (on Unix) or _filename (on VMS), where filename is the name of \
the file you were editing.  If an NEdit process is killed, some of these \
backup files may remain in your directory.  (To remove one of these files \
on Unix, you may have to prefix the ~ (tilde) character with a \
(backslash) to prevent the shell from interpreting it as a special \
character.)\n\
\n\
Shortcuts\n\
\n\
As you become more familiar with NEdit, substitute the control and \
function keys shown on the right side of the menus for pulling down menus \
with the mouse.\n\
\n\
Dialogs are also streamlined so you can enter information quickly and \
without using the mouse*.  To move the keyboard focus around a dialog, \
use the tab and arrow keys.  One of the buttons in a dialog is usually \
drawn with a thick, indented, outline.  This button can be activated by \
pressing Return or Enter.  The Cancel or Dismiss button can be activated \
by pressing escape.  For example, to replace the string \"thing\" with \
\"things\" type:\n\
\n\
    <ctrl-r>thing<tab>things<return>\n\
\n\
To open a file named \"whole_earth.c\", type:\n\
\n\
    <ctrl-o>who<return>\n\
\n\
(how much of the filename you need to type depends on the other files in \
the directory).  See the section called Keyboard Shortcuts for more \
details.\n\
\n\
* Users who have set their keyboard focus mode to \"pointer\" should set \
\"Popups Under Pointer\" in the Default Settings menu to avoid the \
additional step of moving the mouse into the dialog.",

"The Search menu contains a number of commands for finding and replacing text.\n\
\n\
The Find... and Replace... commands present dialogs for entering text for \
searching and replacing.  These dialogs also allow you to choose whether you \
want the search to be sensitive to upper and lower case, or whether to use the \
standard Unix pattern matching characters (regular expressions).  Searches \
begin at the current text insertion position.\n\
\n\
Find Again and Replace Again repeat the last find or replace command without \
prompting for search strings.  To selectively replace text, use the two \
commands in combination: Find Again, then Replace Again if \
the highlighted string should be replaced, or Find Again again to go to the \
next string.\n\
\n\
Find Selection searches for the text contained in the current primary \
selection (see Selecting Text).  The selected text does not have to \
be in the current editor window, it may even be in another program.  For \
example, if the word dog appears somewhere in a window on your screen, and \
you want to find it in the file you are editing, select the \
word dog by dragging the mouse across it, switch to your NEdit window and \
choose Find Selection from the Search menu.\n\
\n\
Find Incremental is yet another variation on searching, where every \
character typed triggers a new search.  Incremental searching is \
generally the quickest way to find something in a file, because it \
gives you the immediate feedback of seeing how your search is \
progressing, so you never need to type more than the minimally \
sufficient search string to reach your target.\n\
\n\
Searching Backwards\n\
\n\
Holding down the shift key while choosing any of the search or replace \
commands from the menu (or using the keyboard shortcut), will search in \
the reverse direction.  Users who have set the search direction using \
the buttons in the search dialog, may find it a bit confusing that Find \
Again and Replace Again don't continue in the same direction as the \
original search (for experienced users, consistency \
of the direction implied by the shift key is more important).\n\
\n\
Selective Replacement\n\
\n\
To replace only some occurrences of a string within a file, choose Replace... \
from the Search menu, enter the string to search for and the string to \
substitute, and finish by pressing the Find button.  When the first \
occurrence is highlighted, use either Replace Again (^T) to replace it, or \
Find Again (^G) to move to the next occurrence without replacing it, and \
continue in such a manner through all occurrences of interest.\n\
\n\
To replace all occurrences of a string within some range of text, \
select the range (see Selecting Text), choose Replace... from the \
Search menu, type the string to search for and the string to substitute, and \
press the \"R. in Selection\" button in the dialog.  Note that selecting text \
in the Replace... dialog will unselect the text in the window.",

"NEdit \
has two general types of selections, primary (highlighted text), and \
secondary (underlined text). Selections can cover either \
a simple range of text between two points in the file, or they can \
cover a rectangular area of the file.  Rectangular selections \
are only useful with non-proportional (fixed spacing) fonts.\n\
\n\
To select text for copying, deleting, or replacing, press the left \
mouse button with the pointer at one end of the text you want to select, \
and drag it to the other end.  The text will become highlighted.  To \
select a whole word, double click (click twice quickly in succession).  \
Double clicking and then dragging the mouse will select a number of words.  \
Similarly, you can select \
a whole line or a number of lines by triple clicking or triple clicking and \
dragging.  Quadruple clicking selects the whole file.  \
After releasing the mouse button, you can still adjust a selection \
by holding down the shift key and dragging on either end of the selection.  \
To delete the selected text, press delete or backspace.  To replace it, \
begin typing.\n\
\n\
To select a rectangle or column of text, hold the Ctrl key while dragging \
the mouse.  Rectangular selections can be used in any context that \
normal selections can be used, including cutting and pasting, filling, \
shifting, dragging, and searching.  Operations on rectangular selections \
automatically fill in tabs and spaces to maintain alignment of text \
within and to the right of the selection.  Note that the interpretation \
of rectangular selections by Fill Paragraph is slightly different from that \
of other commands, the section titled \"Shifting and Filling\" has details.\n\
\n\
The middle mouse button can be used to make an \
additional selection (called the secondary selection).  As soon as the \
button is released, the contents of this selection will be \
copied to the insert position of the window where the mouse was last \
clicked (the destination window).  This position is marked by a caret \
shaped cursor when the \
mouse is outside of the destination window.  \
If there is a (primary) selection, adjacent to the cursor \
in the window, \
the new text will replace the selected text.  Holding the shift key \
while making the secondary selection will move the text, deleting it \
at the site of the secondary selection, rather than \
copying it.\n\
\n\
Selected text can also be dragged to a new location in the file using the \
middle mouse button.  Holding the shift key while \
dragging the text will copy the selected text, leaving the original \
text in place.  Holding the control key will drag the text in overlay \
mode.\n\
\n\
Normally, dragging moves text by removing it from the \
selected position at the start of the drag, and inserting it at a \
new position relative to to the mouse.  Dragging a block of text \
over existing characters, displaces the characters to \
the end of the selection.  In overlay mode, characters which are \
occluded by blocks of text being dragged are simply removed.  When \
dragging non-rectangular selections, overlay mode also converts the \
selection to rectangular form, allowing it to be dragged outside of \
the bounds of the existing text.\n\
\n\
The section \"Using the Mouse\" sumarizes the mouse commands for making \
primary and secondary selections.  Primary selections can also be made \
via keyboard commands, see \"Keyboard Shortcuts\".",

"The easiest way to copy and move text around in your file or between \
windows, is to use the clipboard, an imaginary area that temporarily stores \
text and data.  The Cut command removes the \
selected text (see Selecting Text) from your file and places it in the \
clipboard.  Once text is in \
the clipboard, the Paste command will copy it to the insert position in the \
current window.  For example, to move some text from one place to another, \
select it by \
dragging the mouse over it, choose Cut to remove it, \
click the pointer to move the insert point where you want the text inserted, \
then choose Paste to insert it.  Copy copies text to the clipboard without \
deleting it from your file.  You can also use the clipboard to transfer text \
to and from other Motif programs and X programs which make proper use of \
the clipboard.\n\
\n\
There are many other methods for copying and moving text within \
NEdit windows and between NEdit and other programs.  The most common \
such method is clicking the middle mouse button to copy the primary \
selection (to the clicked position).  Copying the selection by clicking \
the middle mouse button in many cases is the only way to transfer data \
to and from many X programs.  Holding the Shift key while clicking the \
middle mouse button moves the text, deleting it from its original \
position, rather than copying it.  Other methods for transferring text \
include secondary selections, primary selection dragging, keyboard-based \
selection copying, and drag and drop.  These are described in detail in \
the sections: Selecting Text, Using the Mouse, and Keyboard Shortcuts.",


"Programmers who use structured languages usually require some form of \
automatic indent, so that they don't have to continually re-type the \
sequences of tabs and/or spaces needed to maintain lengthy running indents.  \
Version 5.0 of NEdit is the first release of NEdit to offer \"smart\" \
indent, at least experimentally, in addition to the traditional automatic \
indent which simply lines up the cursor position with the previous line.\n\
\n\
Smart Indent\n\
\n\
Smart Indent in this release must still be considered somewhat \
experimental.  Smart indent macros are only available by default for C \
and C++, and while these can easily be configured for different default \
indentation distances, they may not conform to everyone's exact C \
programming style.  Smart indent is programmed in terms of macros in the \
NEdit macro language which can be entered in: Preferences -> Default \
Settings -> Indent -> Program Smart Indent.  Hooks are provided for \
intervening at the point that a newline is entered, either via the user \
pressing the Enter key, or through auto-wrapping; and for arbitrary \
type-in to act on specific characters typed.\n\
\n\
To type a newline character without invoking smart-indent when operating \
in smart-indent mode, hold the Ctrl key while pressing the Return or \
Enter key.\n\
\n\
Auto-Indent\n\
\n\
With Indent set to Auto (the default), NEdit keeps a running indent.  \
When you press the Return or Enter key, \
spaces and tabs are inserted to line up the insert point under the start of \
the previous line.  Ctrl+Return in auto-indent mode acts like a normal \
Return, With auto-indent turned off, Ctrl+Return does indentation.\n\
\n\
Block Indentation Adjustment\n\
\n\
The Shift Left and Shift Right commands as well as rectangular dragging \
can be used to adjust the \
indentation for several lines at once.  To shift a block of text one \
character \
to the right, select the text, then choose Shift Right from the Edit menu.  \
Note that the accelerator keys for these menu items are Ctrl+9 and Ctrl+0, \
which correspond to  the right and left parenthesis on most keyboards.  \
Remember them as adjusting the text in the direction pointed to by the \
parenthesis character.  Holding the Shift key while selecting either \
Shift Left or Shift Right will shift the text by one tab stop (or by one \
emulated tab stop if tab emulation is turned on).  The help section \
\"Shifting and Filling\" under \"Basic Operation\" has details.",


"Changing the Tab Distance\n\
\n\
Tabs are important for programming in languages which use indentation \
to show nesting, as short-hand for producing white-space for leading \
indents.  As a programmer, you have to decide how to use indentation, \
and how or whether tab characters map to your indentation scheme.\n\
\n\
Ideally, tab characters map directly to the amount of indent that you \
use to distinguish nesting levels in your code.  Unfortunately, the \
Unix standard for interpretation of tab characters is eight characters \
(probably dating back to mechanical capabilities of the original \
teletype), which is usually too coarse for a single indent.\n\
\n\
Most text editors, NEdit included, allow you to change the \
interpretation of the tab character, and many programmers take \
advantage of this, and set their tabs to 3 or 4 characters to match \
their programming style.  In NEdit you set the hardware tab distance in \
Preferences -> Tabs... for the current window, or Preferences -> \
Default Settings -> Tabs... (general), or Preferences -> Default \
Settings -> Language Modes... (language-specific) to change the \
defaults for future windows.\n\
\n\
Changing the meaning of the tab character makes programming much easier \
while you're in the editor, but can cause you headaches outside of the \
editor, because there is no way to pass along the tab setting as part \
of a plain-text file.  All of the other tools which display, print, and \
otherwise process your source code have to be made aware of how the \
tabs are set, and must be able to handle the change.  Non-standard tabs \
can also confuse other programmers, or make editing your code difficult \
for them if their text editors don't support changes in tab distance.\n\
\n\
Emulated Tabs\n\
\n\
An alternative to changing the interpretation of the tab character is \
tab emulation.  In the Tabs... dialog(s), turning on Emulated Tabs \
causes the Tab key to insert the correct number of spaces and/or tabs \
to bring the cursor the next emulated tab stop, as if tabs were set at \
the emulated tab distance rather than the hardware tab distance. \
Backspacing immediately after entering an emulated tab will delete the \
fictitious tab as a unit, but as soon as you move the cursor away from \
the spot, NEdit will forget that the collection of spaces and tabs is a \
tab, and will treat it as separate characters.  To enter a real tab \
character with \"Emulate Tabs\" turned on, use Ctrl+Tab.\n\
\n\
It is also possible to tell NEdit not to insert ANY tab characters at \
all in the course of processing emulated tabs, and in shifting and \
rectangular insertion/deletion operations, for programmers who worry \
about the misinterpretation of tab characters on other systems.",

"Though general in appearance, NEdit has many \
features intended specifically for programmers.  Major \
programming-related topics are listed in separate sections under the heading: \
\"Features for Programming\": Syntax Highlighting, \
Tabs and Tab Emulation, ctags support, and Automatic Indent.  Minor topics \
related to programming are discussed below:\n\
\n\
Language Modes\n\
\n\
When NEdit initially reads a file, it attempts to determine whether the \
file is in one of the computer languages that it knows about.  Knowing \
what language a file is written in allows NEdit to assign highlight \
patterns and smart indent macros, and to set language specific \
preferences like word delimiters, tab emulation, and auto-indent.  \
Language mode can be recognized from both the file name and from the \
first 200 characters of content.  Language mode recognition and \
language-specific preferences are configured \
in: Preferences -> Default Settings -> Language Modes....\n\
\n\
You can set the language mode manually for a window, by selecting \
it from the menu: Preferences -> Language Modes.\n\
\n\
Line Numbers\n\
\n\
To find a particular line in a source file by line number, choose Goto \
Line #... from the Search menu.  You can also directly select the line \
number text in the compiler message in the terminal emulator window \
(xterm, decterm, winterm, etc.) where you ran the compiler, and choose \
Goto Selected from the Search menu.\n\
\n\
To find out the line number of a particular line in your file, turn on \
Statistics Line in the Preferences menu and position the insertion point \
anywhere on the line.  The statistics line continuously updates the line \
number of the line containing the cursor.\n\
\n\
Matching Parentheses\n\
\n\
To help you inspect nested parentheses, brackets, braces, quotes, and other \
characters, NEdit has both an automatic parenthesis matching mode, and a \
Goto Matching command.  Automatic parenthesis matching is activated when \
you type, or move the insertion cursor after a parenthesis, bracket, or \
brace.  It momentarily highlights the matching character if that character \
is visible in the window.  To find a matching character anywhere in \
the file, select it or position the cursor after it, and choose Goto \
Matching from the Search menu.  If \
the character matches itself, such as a quote or slash, select the first \
character of the pair.  NEdit will match {, (, [, <, \", \', `, /, and \\. \
Holding the Shift key while typing the accelerator key (Shift+Ctrl+M, by \
default),will select all of the text between the matching characters.\n\
\n\
Opening Included Files\n\
\n\
The Open Selected command in the File menu understands the C \
preprocessor's #include \
syntax, so selecting an #include line and invoking Open Selected will \
generally find the file referred to, unless doing so depends on the \
settings of compiler switches or other information not available to NEdit.\n\
\n\
Interface to Programming Tools\n\
\n\
Integrated software development environments such as SGI's CaseVision and \
Centerline Software's Code Center, can be interfaced \
directly with NEdit via the client server interface.  These \
tools allow you to click directly on compiler and runtime error messages \
and request NEdit to open files, and select lines of interest.  \
The easiest method is usually to use \
the tool's interface for character-based editors like vi, to invoke nc, \
but programmatic interfaces can also be derived using the source code \
for nc.\n\
\n\
There are also some simple compile/review, grep, ctree, and ctags browsers \
available in the NEdit contrib directory on ftp.nedit.org.",

"NEdit can process tags files generated using the Unix ctags command or \
the Exuberant Ctags program.  \
Ctags creates index files correlating names of functions and declarations \
with their locations in C, Fortran, or Pascal source code files. (See the \
ctags manual page for more information).  Ctags produces a file called \
\"tags\" which can be loaded by NEdit.  NEdit can manage any number of \
tags files simultaneously.  Tag collisions are handled with a popup menu \
to let the user decide which tag to use.  In 'Smart' mode NEdit will \
automatically choose the desired tag based on the scope of the file or \
module. Once loaded, the information in \
the tags file enables NEdit to go directly to the declaration of a \
highlighted function or data structure name with a single command.  To \
load a tags file, select \"Load Tags File\" from the File menu and choose \
a tags file to load, or specify the name of the tags file on the NEdit \
command line:\n\
\n\
    nedit -tags tags\n\
\n\
NEdit can also be set to load a tags file automatically when it starts up.  \
Setting the X resource nedit.tagFile to the name of a tag file tells NEdit \
to look for that file at startup time (see Customizing NEdit).  The file name \
can be either a complete path name, in which case NEdit will always load \
the same tags file, or a file name without a path or with a relative path, \
in which case NEdit will load it starting from the current directory.  The \
second option allows you to have different tags files for different \
projects, each automatically loaded depending on the directory you're in \
when you start NEdit.  Setting the name to \"tags\" is an obvious choice \
since this is the name that ctags uses.\n\
\n\
To unload a tags file, select \"Un-load Tags File\" from the File menu \
and choose from the list of tags files.  NEdit will keep track of tags \
file updates by checking the timestamp on the files, and automatically \
update the tags cache. \n\
\n\
To find the definition of a function or data structure once a tags file is \
loaded, select the name anywhere it appears in your program (see Selecting \
Text) and choose \"Find Definition\" from the Search menu.",

"Mouse-based \
editing is what NEdit is all about, and learning to use the more advanced \
features like secondary selections and primary selection \
dragging will be well worth your while.\n\
\n\
If you don't have time to learn everything, you can get by adequately with \
just the left mouse button:  Clicking the left button \
moves the cursor.  \
Dragging with the left button makes a selection.  Holding the shift \
key while clicking extends the existing selection, or begins a selection \
between the cursor and the mouse.  Double or triple clicking selects a \
whole word or a whole line.\n\
\n\
This section will make more sense if you also read the section called, \
\"Selecting Text\", which explains the terminology of selections, i.e. \
what is meant by primary, secondary, rectangular, etc.\n\
\n\
\n\
GENERAL\n\
\n\
General meaning of mouse buttons and modifier keys:\n\
\n\
Buttons\n\
\n\
  Button 1 (left)    Cursor position and primary selection\n\
\n\
  Button 2 (middle)  Secondary selections, and dragging and\n\
                     copying the primary selection\n\
\n\
  Button 3 (right)   Quick-access programmable menu and pan\n\
                     scrolling\n\
\n\
Modifier keys\n\
\n\
  Shift   On primary selections, (left mouse button):\n\
             Extends selection to the mouse pointer\n\
          On secondary and copy operations, (middle):\n\
             Toggles between move and copy\n\
\n\
  Ctrl    Makes selection rectangular or insertion\n\
          columnar\n\
\n\
  Alt*    (on release) Exchange primary and secondary\n\
          selections.\n\
\n\
\n\
Left Mouse Button\n\
\n\
The left mouse button is used to position the cursor and to make primary \
selections\n\
\n\
  Click 	Moves the cursor\n\
 \n\
  Double Click	Selects a whole word\n\
\n\
  Triple Click	Selects a whole line\n\
\n\
  Quad Click	Selects the whole file\n\
\n\
  Shift Click	Adjusts (extends or shrinks) the\n\
 		selection, or if there is no existing\n\
 		selection, begins a new selection\n\
 		between the cursor and the mouse.\n\
\n\
  Ctrl+Shift+	Adjusts (extends or shrinks) the\n\
  Click 	selection rectangularly.\n\
\n\
  Drag		Selects text between where the mouse\n\
 		was pressed and where it was released.\n\
\n\
  Ctrl+Drag	Selects rectangle between where the\n\
 		mouse was pressed and where it was\n\
 		released.\n\
\n\
\n\
Right Mouse Button\n\
\n\
The right mouse button posts a programmable menu for frequently used \
commands.\n\
\n\
  Click/Drag   Pops up the background menu (programmed\n\
               from Preferences -> Default Settings ->\n\
	       Customize Menus -> Window Background).\n\
\n\
  Ctrl+Drag    Pan scrolling.  Scrolls the window\n\
               both vertically and horizontally, as if\n\
	       you had grabbed it with your mouse.\n\
\n\
\n\
Middle Mouse Button\n\
\n\
The middle mouse button is for making secondary selections, and copying and \
dragging the primary selection\n\
\n\
  Click        Copies the primary selection to the\n\
 	       clicked position.\n\
\n\
  Shift+Click  Moves the primary selection to the\n\
 	       clicked position, deleting it from its\n\
 	       original position.\n\
\n\
  Drag	       1) Outside of the primary selection:\n\
 	           Begins a secondary selection.\n\
 	       2) Inside of the primary selection:\n\
 	           Moves the selection by dragging.\n\
\n\
  Ctrl+Drag    1) Outside of the primary selection:\n\
 	           Begins a rectangular secondary\n\
 	           selection.\n\
 	       2) Inside of the primary selection:\n\
 	           Drags the selection in overlay\n\
 	           mode (see below).\n\
\n\
When the mouse button is released after creating a secondary selection:\n\
\n\
  No Modifiers	If there is a primary selection,\n\
 		replaces it with the secondary\n\
 		selection.  Otherwise, inserts the\n\
 		secondary selection at the cursor\n\
 		position.\n\
\n\
  Shift 	Move the secondary selection, deleting\n\
 		it from its original position.  If\n\
 		there is a primary selection, the move\n\
 		will replace the primary selection\n\
 		with the secondary selection.\n\
 		Otherwise, moves the secondary\n\
 		selection to to the cursor position.\n\
\n\
  Alt*		Exchange the primary and secondary\n\
 		selections.\n\
\n\
\n\
While moving the primary selection by dragging with the middle mouse button:\n\
\n\
  Shift   Leaves a copy of the original\n\
 	  selection in place rather than\n\
 	  removing it or blanking the area.\n\
 \n\
  Ctrl	  Changes from insert mode to overlay\n\
 	  mode (see below).\n\
\n\
  Escape  Cancels drag in progress.\n\
\n\
Overlay Mode: Normally, dragging moves text by removing it from the \
selected position at the start of the drag, and inserting it at a \
new position relative to to the mouse. When you drag a block of text \
over existing characters, the existing characters are displaced to \
the end of the selection.  In overlay mode, characters which are \
occluded by blocks of text being dragged are simply removed.  When \
dragging non-rectangular selections, overlay mode also converts the \
selection to rectangular form, allowing it to be dragged outside of \
the bounds of the existing text.\n\
\n\
\n\
* The Alt key may be labeled Meta or Compose-Character on some \
keyboards.  Some window managers, including default configurations \
of mwm, bind combinations of the Alt key and mouse buttons to window \
manager operations.  In NEdit, Alt is only used on button release, \
so regardless of the window manager bindings for Alt-modified mouse \
buttons, you can still do the corresponding NEdit operation by using \
the Alt key AFTER the initial mouse press, so that Alt is held while \
you release the mouse button.  If you find this difficult or \
annoying, you can re-configure most window managers to skip this \
binding, or you can re-configure NEdit to use a different key \
combination.",

"Most of the keyboard shortcuts in NEdit are shown on the right hand sides \
of the pull-down menus.  However, there are more which are not as obvious.  \
These include; dialog button shortcuts; menu and \
dialog mnemonics; labeled keyboard keys, such as the arrows, page-up, \
page-down, and home; and optional Shift modifiers on accelerator keys, like \
[Shift]Ctrl+F.\n\
\n\
\n\
Menu Accelerators\n\
\n\
Pressing the key combinations shown on the right of the menu items is a \
shortcut for selecting the menu item with the mouse.  Some items have the \
shift key enclosed in brackets, such as [Shift]Ctrl+F.  This indicates \
that the shift key is optional.  In search commands, including the shift \
key reverses the direction of the search.  In Shift commands, it makes the \
command shift the selected text by a whole tab stop rather than by \
single characters.\n\
\n\
\n\
Menu Mnemonics\n\
\n\
Pressing the Alt key in combination with one of the underlined characters \
in the menu bar pulls down that menu.  Once the menu is pulled down, \
typing the underlined characters in a menu item (without the Alt key) \
activates that item.  With a menu pulled down, you can also use the arrow \
keys to select menu items, and the Space or Enter keys to activate them.\n\
\n\
\n\
Keyboard Shortcuts within Dialogs\n\
\n\
One button in a dialog is usually marked with a thick indented outline.  \
Pressing the Return or Enter key activates this button.\n\
\n\
All dialogs have either a Cancel or Dismiss button.  This button can \
be activated by pressing the Escape (or Esc) key.\n\
\n\
Pressing the tab key moves the keyboard focus to the next item in a \
dialog.  Within an associated group of buttons, the arrow keys move \
the focus among the buttons.  Shift+Tab moves backward through the items.\n\
\n\
Most items in dialogs have an underline under one character in their name.  \
Pressing the Alt key along with this character, activates a button as if you \
had pressed it with the mouse, or moves the keyboard focus to the associated \
text field or list.\n\
\n\
You can select items from a list by using the arrow keys to move the \
selection and space to select.\n\
\n\
In file selection dialogs, you can type the beginning characters of the \
file name or directory in the list to select files\n\
\n\
\n\
Labeled Function Keys \n\
\n\
The labeled function keys on standard workstation and PC keyboards, like \
the arrows, and page-up and page-down, are active in NEdit, though not \
shown in the pull-down menus.\n\
\n\
Holding down the control key while pressing a named key extends the \
scope of the action that it performs.  For example, Home normally moves \
the insert cursor the beginning of a line.  Ctrl+Home moves it to the \
beginning of the file. Backspace deletes one character, Ctrl+Backspace \
deletes one word.\n\
\n\
Holding down the shift key while pressing a named key begins or extends \
a selection.  Combining the shift and control keys combines their \
actions.  For example, to select a word without using the mouse, \
position the cursor at the beginning of the word and press \
Ctrl+Shift+RightArrow.  The Alt key modifies selection commands to make \
the selection rectangular.\n\
\n\
Under X and Motif, there are several levels of translation between \
keyboard keys and the actions they perform in a program.  The \
\"Customizing NEdit\", and \"X Resources\" sections of the Help \
menu have more information on this subject.  Because of all of this \
configurability, and since keyboards and standards for the meaning of \
some keys vary from machine to machine, the mappings may be changed from \
the defaults listed below.\n\
\n\
Modifier Keys (in general)\n\
\n\
  Ctrl	 Extends the scope of the action that the key\n\
 	 would otherwise perform.  For example, Home\n\
 	 normally moves the insert cursor to the beginning\n\
 	 of a line. Ctrl+Home moves it to the beginning of\n\
 	 the file.  Backspace deletes one character, Ctrl+\n\
 	 Backspace deletes one word.\n\
\n\
  Shift  Extends the selection to the cursor position. If\n\
 	 there's no selection, begins one between the old\n\
 	 and new cursor positions.\n\
\n\
  Alt	 When modifying a selection, makes the selection\n\
 	 rectangular.\n\
\n\
(For the effects of modifier keys on mouse button presses, see \
the section titled \"Using the Mouse\")\n\
\n\
All Keyboards\n\
\n\
  Escape	Cancels operation in progress: menu\n\
  		selection, drag, selection, etc.  Also\n\
  		equivalent to cancel button in dialogs.\n\
\n\
  Backspace	Delete the character before the cursor\n\
\n\
  Ctrl+BS	Delete the word before the cursor\n\
\n\
  Arrows\n\
\n\
    Left	Move the cursor to the left one character\n\
\n\
    Ctrl+Left   Move the cursor backward one word\n\
    		(Word delimiters are settable, see\n\
    		Customizing NEdit, and X Resources)\n\
\n\
    Right	Move the cursor to the right one character\n\
\n\
    Ctrl+Right  Move the cursor forward one word\n\
\n\
    Up  	Move the cursor up one line\n\
\n\
    Ctrl+Up	Move the cursor up one paragraph.\n\
    		(Paragraphs are delimited by blank lines)\n\
\n\
    Down	Move the cursor down one line.\n\
\n\
    Ctrl+Down	Move the cursor down one paragraph.\n\
\n\
  Ctrl+Return	Return with automatic indent, regardless\n\
  		of the setting of Auto Indent.\n\
\n\
  Shift+Return	Return without automatic indent,\n\
  		regardless of the setting of Auto Indent.\n\
\n\
  Ctrl+Tab	Insert an ascii tab character, without\n\
  		processing emulated tabs.\n\
\n\
  Alt+Ctrl+<c>	Insert the control-code equivalent of\n\
  		a key <c>\n\
\n\
  Ctrl+/	Select everything (same as Select\n\
     		All menu item or ^A)\n\
\n\
  Ctrl+\\	Unselect\n\
\n\
  Ctrl+U	Delete to start of line\n\
\n\
PC Standard Keyboard\n\
\n\
  Ctrl+Insert	Copy the primary selection to the\n\
 		clipboard (same as Copy menu item or ^C)\n\
 		for compatibility with Motif standard key\n\
 		binding\n\
  Shift+Ctrl+\n\
  Insert	Copy the primary selection to the cursor\n\
 		location.\n\
\n\
  Delete	Delete the character before the cursor.\n\
 		(Can be configured to delete the character\n\
 		after the cursor, see Customizing NEdit,\n\
 		and X Resources)\n\
\n\
  Ctrl+Delete	Delete to end of line.\n\
\n\
  Shift+Delete	Cut, remove the currently selected text\n\
 		and place it in the clipboard. (same as\n\
 		Cut menu item or ^X) for compatibility\n\
 		with Motif standard key binding\n\
  Shift+Ctrl+\n\
  Delete	Cut the primary selection to the cursor\n\
 		location.\n\
\n\
  Home		Move the cursor to the beginning of the\n\
                line\n\
\n\
  Ctrl+Home	Move the cursor to the beginning of the\n\
                file\n\
\n\
  End		Move the cursor to the end of the line\n\
\n\
  Ctrl+End	Move the cursor to the end of the file\n\
\n\
  PageUp	Scroll and move the cursor up by one page.\n\
\n\
  Ctrl+PageUp   Scroll and move the cursor left by one\n\
  		page.\n\
  PageDown	Scroll and move the cursor down by one\n\
  		page.\n\
\n\
  Ctrl+PageDown Scroll and move the cursor right by one\n\
  		page.\n\
\n\
  F10		Make the menu bar active for keyboard\n\
  		input (Arrow Keys, Return, Escape,\n\
  		and the Space Bar)\n\
\n\
Specialty Keyboards \n\
\n\
On machines with different styles of keyboards, generally, text \
editing actions are properly matched to the labeled keys, such as \
Remove, Next-screen, etc..  If you prefer different key bindings, see \
the section titled \"Key Binding\" under the Customizing \
heading in the Help menu.",

"Shifting and Filling\n\
\n\
\n\
Shift Left, Shift Right \n\
\n\
While shifting blocks of text is most important for programmers (See \
Features for Programming), it is also useful for other tasks, such as \
creating indented paragraphs.\n\
\n\
To shift a block of text one tab stop to the right, select the text, \
then choose Shift Right from the Edit menu.  Note that the accelerator \
keys for these menu items are Ctrl+9 and Ctrl+0, which correspond to \
the right and left parenthesis on most keyboards.  Remember them as \
adjusting the text in the direction pointed to by the parenthesis \
character.  Holding the Shift key while selecting either Shift Left or \
Shift Right will shift the text by one character.\n\
\n\
It is also possible to shift blocks of text by selecting the text \
rectangularly, and dragging it left or right (and up or down as well).  \
Using a rectangular selection also causes tabs within the selection to \
be recalculated and substituted, such that the non-whitespace characters \
remain stationary with respect to the selection.\n\
\n\
\n\
Filling \n\
\n\
Text filling using the Fill Paragraph command in the Edit menu is one \
of the most important concepts in NEdit.  And it will be well worth \
your while to understand how to use it properly.\n\
\n\
In plain text files, unlike word-processor files, there is no way to \
tell which lines are continuations of other lines, and which lines are \
meant to be separate, because there is no distinction in meaning \
between newline characters which separate lines in a paragraph, and \
ones which separate paragraphs from other text.  This makes it \
impossible for a text editor like NEdit to tell parts of the text \
which belong together as a paragraph from carefully arranged \
individual lines.\n\
\n\
In continuous wrap mode (Preferences -> Wrap -> Continuous), lines \
automatically wrap and unwrap themselves to line up properly at the \
right margin.  In this mode, you simply omit the newlines within \
paragraphs and let NEdit make the line breaks as needed.  Unfortunately, \
continuous wrap mode is not appropriate in the majority of situations, \
because files with extremely long lines are not common under Unix and \
may not be compatible with all tools, and because you can't achieve \
effects like indented sections, columns, or program comments, and still \
take advantage of the automatic wrapping.\n\
\n\
Without continuous wrapping, paragraph filling is not entirely \
automatic.  Auto-Newline wrapping keeps paragraphs lined up as you \
type, but once entered, NEdit can no longer distinguish newlines which \
join wrapped text, and newlines which must be preserved.  Therefore, \
editing in the middle of a paragraph will often leave the right margin \
messy and uneven.\n\
\n\
Since NEdit can't act automatically to keep your text lined up, you \
need to tell it explicitly where to operate, and that is what Fill \
Paragraph is for.  It arranges lines to fill the space between two \
margins, wrapping the lines neatly at word boundaries.  Normally, the \
left margin for filling is inferred from the text being filled.  The \
first line of each paragraph is considered special, and its left \
indentation is maintained separately from the remaining lines (for \
leading indents, bullet points, numbered paragraphs, etc.).  Otherwise, \
the left margin is determined by the furthest left non-whitespace \
character.  The right margin is either the Wrap Margin, set in the \
preferences menu (by default, the right edge of the window), or can \
also be chosen on the fly by using a rectangular selection (see below).\n\
\n\
There are three ways to use Fill Paragraph.  The simplest is, while you \
are typing text, and there is no selection, simply select Fill Paragraph \
(or type Ctrl+J), and NEdit will arrange the text in the paragraph \
adjacent to the cursor.  A paragraph, in this case, means an area of \
text delimited by blank lines.\n\
\n\
The second way to use Fill Paragraph is with a selection.  If you select \
a range of text and then chose Fill Paragraph, all of the text in the \
selection will be filled.  Again, continuous text between blank lines is \
interpreted as paragraphs and filled individually, respecting leading \
indents and blank lines.\n\
\n\
The third, and most versitile, way to use Fill Paragraph is with a \
rectangular selection.  Fill Paragraph treats rectangular selections \
differently from other commands.  Instead of simply filling the text \
inside the rectangular selection, NEdit interprets the right edge of \
the selection as the requested wrap margin.  Text to the left of the \
selection is not disturbed (the usual interpretation of a rectangular \
selection), but text to the right of the selection is included in the \
operation and is pulled in to the selected region.  This method enables you \
to fill text to an arbitrary right margin, without going back and forth \
to the wrap-margin dialog, as well as to exclude text to the left of \
the selection such as comment bars or other text columns.",

"While plain-text is probably the simplest and most interchangeable \
file format in the computer world, there is still variation in what \
plain-text means from system to system.  Plain-text files can differ \
in character set, line termination, and wrapping.\n\
\n\
While character set differences are the most obvious and pose the \
most challenge to portability, they affect NEdit only indirectly via \
the same font and localization mechanisms common to all X \
applications.  If your system is set up properly, you will probably \
never see character-set related problems in NEdit.  NEdit can not \
display Unicode text files, or any multi-byte character set.\n\
\n\
The primary difference between an MS DOS format file and a Unix \
format file, is how the lines are terminated.  Unix uses a single \
newline character.  MS DOS uses a carriage-return and a newline.  \
NEdit can read and write both file formats, but internally, it uses \
the single character Unix standard.  NEdit auto-detects MS DOS format \
files based on the line termination at the start of the file.  Files \
are judged to be DOS format if all of the first five line terminators, \
within a maximum range, are DOS-style.  To change the format in which \
NEdit writes a file from DOS to Unix or visa versa, use the Save \
As... command and check or un-check the MS DOS Format button.\n\
\n\
Wrapping within text files can vary among individual users, as \
well as from system to system.  Both Windows and MacOS make \
frequent use of plain \
text files with no implicit right margin.  In these files, wrapping \
is determined by the tool which displays them.  Files of this style \
also exist on Unix systems, despite the fact that they are not \
supported by all Unix utilities.  To display this kind of file \
properly in NEdit, you have to select the wrap style called \
Continuous.  Wrapping modes are discussed in the sections: \
Customizing -> Preferences, and Basic Operation -> Shifting and \
Filling.\n\
\n\
The last and most minute of format differences is the terminating \
newline.  NEdit, like vi and approximately half of Unix editors, \
enforces a final terminating newline on all of the files that it \
writes.  NEdit does this because some Unix compilers and utilities \
require it, and fail in various ways on files which do not have it.  \
Emacs does not enforce this rule.  Users are divided on which \
is best.",

"Syntax Highlighting means using colors and fonts to help distinguish \
language elements in programming languages and other types of \
structured files.  Programmers use syntax highlighting to understand \
code faster and better, and to spot many kinds of syntax errors more \
quickly.\n\
\n\
To use syntax highlighting in NEdit, select Highlight Syntax in the \
Preferences menu.  If NEdit recognizes the computer language that you \
are using, and highlighting rules (patterns) are available for that \
language, it will highlight your text, and maintain the highlighting, \
automatically, as you type.\n\
\n\
If NEdit doesn't correctly recognize the type of the file you are \
editing, you can manually select a language mode from Language Modes in \
the Preferences menu.  You can also program the method that NEdit uses \
to recognize language modes in Preferences -> Default Settings -> \
Language Modes....\n\
\n\
If no highlighting patterns are available for the language that you \
want to use, you can create new patterns relatively quickly.  The \
Help section \"Highlighting Patterns\" under \"Customizing\", has details.\n\
\n\
If you are satisfied with what NEdit is highlighting, but would like it \
to use different colors or fonts, you can change these by selecting \
Preferences -> Default Settings -> Syntax Highlighting -> Text Drawing \
Styles.  Highlighting patterns are \
connected with font and color information through a common set of \
styles so that colorings defined for one language will be similar \
across others, and patterns within the same language which are meant to \
appear identical can be changed in the same place.  To understand which \
styles are used to highlight the language you are interested in, you \
may need to look at \"Highlighting Patterns\" section, as well.\n\
\n\
Syntax highlighting is CPU intensive, and under some circumstances can \
affect NEdit's responsiveness.  If you have a particularly slow system, \
or work with very large files, you may not want to use it all of the \
time.  Syntax highlighting introduces two kinds of delays.  The first is \
an initial parsing delay, proportional to the size of the file.  This \
delay is also incurred when pasting large sections of text, filtering \
text through shell commands, and other circumstances involving changes \
to large amounts of text.  The second kind of delay happens when text which \
has not previously been visible is scrolled in to view.  Depending on \
your system, and the highlight patterns you are using, this may or may \
not be noticeable.  A typing delay is also possible, but unlikely if \
you are only using the built-in patterns.",

"If a system crash, network failure, X server crash, or program error should \
happen while you are editing a file, you can still recover most of your work.  \
NEdit maintains a backup file which it updates periodically (every 8 editing \
operations or 80 characters typed).  This file is has the same name as the \
file that you are editing, but with the character \"~\" (tilde) on Unix or \
\"_\" (underscore) on VMS prefixed \
to the name.  To recover a file after a crash, simply rename the file to \
remove the tilde or underscore character, replacing the older version of the \
file.  \
(Because several of the Unix shells consider the tilde to be a special \
character, you may have to prefix the character with a \"\\\" (backslash) \
when you move or delete an NEdit backup file.)\n\
\n\
Example, to recover the file called \"help.c\" on Unix type the command:\n\
\n\
    mv \\~help.c help.c\n\
\n\
A minor caveat, is that if the file you were editing was in MS DOS \
format, the backup file will be in Unix format, and you will need to \
open the backup file in NEdit and change the file format back \
to MS DOS via the Save As... dialog (or use the Unix \
unix2dos command outside of NEdit).",

"The Preferences menu allows you to set options for both the current \
editing window, and default values for newly created windows and future \
NEdit sessions.  Options in the Preferences menu itself (not in the \
Default Settings sub-menu) \
take effect immediately and refer to the current window only.  Options \
in the Default Settings sub-menu have no effect on the current window, \
but instead provide initial settings for future windows created using \
the New or Open commands.  Preferences set in the Default Settings \
sub-menu can also be saved in a file \
that is automatically read by NEdit at startup time, by selecting \
Save Defaults.\n\
\n\
Preferences Menu\n\
\n\
    Default Settings -- Menu of initial settings for\n\
        future windows.  Generally the same as the\n\
        options in the main part of the menu, but apply\n\
        as defaults for future windows created during\n\
        this NEdit session.  These settings can be saved\n\
        using the Save Defaults command below, to be\n\
        loaded automatically each time NEdit is started.\n\
\n\
    Save Defaults -- Save the default options as set\n\
        under Default Settings for future NEdit sessions.\n\
\n\
    Statistics Line -- Show the full file name, line\n\
        number, and length of the file being edited.\n\
\n\
    Incremental Search Line -- Keep the incremental search\n\
        bar (Search -> Find Incremental) permanently\n\
	displayed at the top of the window.\n\
\n\
    Show Line Numbers -- Display line numbers to the right\n\
        of the text.\n\
\n\
    Language Mode -- Tells NEdit what language (if any) to\n\
        assume, for selecting language-specific features\n\
	such as highlight patterns and smart indent macros,\n\
	and setting language specific preferences like word\n\
	delimiters, tab emulation, and auto-indent.  See\n\
	Features for Programming -> Programming With NEdit\n\
	for more information.\n\
\n\
    Auto Indent -- Setting Auto Indent \"on\" maintains a\n\
        running indent (pressing the Return key will line\n\
	up the cursor with the indent level of the previous\n\
	line).  If smart indent macros are available for\n\
	the current language mode, smart indent can be\n\
	selected and NEdit will attempt to guess proper\n\
	language indentation for each new line.  See\n\
	Help -> Features for Programming -> Automatic\n\
	Indent for more information.\n\
\n\
    Wrap -- Choose between two styles of automatic wrapping\n\
        or none.  Auto Newline wrap, wraps text at word\n\
	boundaries when the cursor reaches the right margin,\n\
	by replacing the space or tab at the last word\n\
	boundary with a newline character.  Continuous Wrap\n\
	wraps long lines which extend past the right margin.\n\
	Continuous Wrap mode is typically used to produce\n\
	files where newlines are ommitted within paragraphs,\n\
	to make text filling automatic (a kind of poor-man's\n\
	word processor).  Text of this style is common on\n\
	Macs and PCs but is not necessarily supported very\n\
	well under Unix (except in programs which deal with\n\
	e-mail, for which it is often the format of choice).\n\
\n\
	Wrap Margin -- Set margin for Auto Newline Wrap,\n\
            Continuous Wrap, and Fill Paragraph.  Lines may,\n\
            be wrapped at the right margin of the window, or\n\
            the margin can be set at a specific column.\n\
\n\
    Tabs -- Set the tab distance (number of characters\n\
        between tab stops) for tab characters, and\n\
        control tab emulation and use of tab characters\n\
        in padding and emulated tabs.\n\
\n\
    Text Font... -- Change the font(s) used to display\n\
	text (fonts for menus and dialogs must be set\n\
	using X resources for the text area of the window).\n\
	See below for more information.\n\
\n\
    Highlight Syntax -- If NEdit recognizes the language\n\
        being edited, and highlighting patterns are\n\
	available for that language, use fonts and colors\n\
	to enhance viewing of the file.  (See Help ->\n\
	Features for Programming -> Syntax Highlighting\n\
	for more information.\n\
\n\
    Make Backup Copy -- On Save, write a backup copy of\n\
    	the file as it existed before the Save command\n\
    	with the extension .bck (Unix only).\n\
\n\
    Incremental Backup -- Periodically make a backup copy\n\
        of the file being edited under the name ~filename\n\
        on Unix or _filename on VMS (see Crash Recovery).\n\
\n\
    Show Matching (..) -- Momentarily highlight matching\n\
        parenthesis, brackets, and braces when one of\n\
        these characters is typed, or when the insertion\n\
        cursor is positioned after it.\n\
\n\
    Overtype -- In overtype mode, new characters entered\n\
        replace the characters in front of the insertion\n\
        cursor, rather than being inserted before them.\n\
\n\
    Read Only -- Lock the file against accidental\n\
        modification.  This temporarily prevents the\n\
        file from being modified in this NEdit session.\n\
        Note that this is diferent from setting the file\n\
        protection.\n\
\n\
Preferences -> Default Settings Menu\n\
\n\
Options in the Preferences -> Default Settings menu have the same \
meaning as those in the top-level Preferences menu, except that they \
apply to future NEdit windows and future NEdit sessions if saved \
with the Save Defaults command.  Additional options which appear in \
this menu are:\n\
\n\
    Language Modes -- Define language recognition\n\
        information (for determining language mode from\n\
	file name or content) and set language specific\n\
	preferences.\n\
\n\
    Tag Collisions -- How to react to multiple tags for\n\
        the same name.  Tags are described in the section:\n\
	Features for Programmers -> Finding Declarations\n\
	(ctags).  In Show All mode, all matching tags are\n\
	displayed in a dialog.  In Smart mode, if one of\n\
	the matching tags is in the current window, that\n\
	tag is chosen, without displaying the dialog.\n\
\n\
    Customize Menus -- Add/remove items from the Shell,\n\
        Macro, and window background menus (see below).\n\
\n\
    Searching -- Options for controlling the behavior of\n\
        Find and Replace commands:\n\
\n\
	    Verbose - Presents search results in dialog\n\
	        form, asks before wrapping a search back\n\
		around the beginning (or end) of the file.\n\
\n\
	    Wrap Around - Search and Replace operations wrap\n\
	        around the beginning (or end) of the file.\n\
\n\
	    Keep Dialogs Up - Don't pop down Replace and\n\
	        Find boxes after searching.\n\
\n\
	    Default Search Style - Initial setting for\n\
	        search type in Find and Replace dialogs.\n\
\n\
    Syntax Highlighting -- Program and configure enhanced\n\
        text display for new or supported languages (See\n\
	Features for Programming -> Syntax Highlighting).\n\
\n\
    Sort Open Prev. Menu -- Option to order the File ->\n\
        Open Previous menu alphabetically, versus in order\n\
	of last access.\n\
\n\
    Popups Under Pointer -- Display pop-up dialogs\n\
        centered on the current mouse position, as opposed\n\
	to centered on the parent window.  This generally\n\
	speeds interaction, and is essential for users who\n\
	users who set their window managers so keyboard\n\
	focus follows the mouse.\n\
\n\
    Modification Warnings -- Pop up a warning dialog when\n\
        files get changed external to NEdit.\n\
\n\
    Exit Warnings -- Ask before exiting when two or more\n\
        files are open in an NEdit session.\n\
\n\
    Initial Window Size -- Default size for new windows.\n\
\n\
Changing Font(s)\n\
\n\
The font used to display text in NEdit is set under Preferences -> \
Text Font (for the current window), or Preferences -> Default Settings \
Text Font (for future windows).  These dialogs also allow you to set \
fonts for syntax highlighting.  If you don't intend to use syntax \
highlighting, you can ignore most of the dialog, and just set the \
field labeled Primary Font.\n\
\n\
Unless you are absolutely certain about the types of files that you \
will be editing with NEdit, you should choose a fixed-spacing font.  \
Many, if not most, plain-text files are written expecting to be viewed \
with fixed character spacing, and will look wrong with proportional spacing.  \
NEdit's filling, wrapping, and rectangular operations will also work strangely \
if you choose a proportional font.\n\
\n\
Note that in the font browser (the dialog \
brought up by the Browse... button), the subset of fonts which are shown is \
narrowed depending on the characteristics already selected.  It is \
therefore important to know that you can unselect characteristics \
from the lists by clicking on the selected items a second time.\n\
\n\
Fonts for syntax highlighting should ideally match the primary font \
in both height and spacing.  A mismatch in spacing will result in similar \
distortions as choosing a proportional font: column alignment will \
sometimes look wrong, and rectangular operations, wrapping, and filling \
will behave strangely.  A mismatch in height will cause windows to re-size \
themselves slightly when syntax highlighting is turned on or off, and \
increase the \
inter- line spacing of the text.  Unfortunately, on some systems it is \
hard to find sets of fonts which match exactly in height.\n\
\n\
Customizing Menus\n\
\n\
You can add or change items in the Shell, Macro, and window background \
menus under Preferences -> Default Settings -> Customize Menus.  When \
you choose one of these, you will see a dialog with a list of the \
current user-configurable items from the menu on the left.  \
To change an existing item, select it from the list, and its properties \
will appear in the remaining fields of the dialog, where you may change \
them.  Selecting the item \"New\" from the list allows you to enter new \
items in the menu.\n\
\n\
Hopefully most of the characteristics are self explanatory, but here are \
a few things to note:\n\
\n\
Accelerator keys are keyboard shortcuts which appear on the right hand \
side of the menus, and allow you avoid pulling down the menu and activate \
the command with a single keystroke.  Enter accelerators by typing the keys \
exactly as you would to activate the command.\n\
\n\
Mnemonics are a single letter which should be part of the menu item name, \
which allow users to traverse and activate menu items by typing keys when \
the menu is pulled down.\n\
\n\
In the Shell Command field of the Shell Commands dialog, the % character \
expands to the name (including directory path) of the file in the window.  \
To include a % character in the command, use %%.\n\
\n\
The Menu Entry field can contain special \
characters for constructing hierarchical sub-menus, and for making items \
which appear only in certain language modes.  The right angle bracket \
character \">\" creates a sub-menu.  The name of the item itself should \
be the last element of the path formed from successive sub-menu names \
joined with \">\".  Menu panes are called in to existence simply by \
naming them as part of a Menu Entry name.  To put several items in the same \
sub-menu, repeat the same hierarchical sequence for each.  For \
example, in the Macro Commands dialog, two items with menu entries: a>b>c and \
a>b>d would create a single sub menu under the macro menu called \"a\", \
which would contain a single sub-menu, b, holding the actual items, c and d:\n\
\n\
    +---++---++---+\n\
    |a >||b >||c  |\n\
    +---++---+|d  |\n\
    	      +---+\n\
\n\
To qualify a menu entry with a language mode, simply add an at-sign \"@\" \
at the end of the menu command, followed (no space) by a language mode \
name.  To make a menu item which appears in several language \
modes, append additional @s and language mode names.  For example, \
an item with the menu entry:\n\
\n\
  Make C Prototypes@C@C++\n\
\n\
would appear only in C and C++ language modes, and:\n\
\n\
  Make Class Template@C++\n\
\n\
would appear only in C++ mode.\n\
\n\
Menu items with no qualification appear in all language modes.\n\
\n\
If a menu item is followed by the single language qualification \"@*\", \
that item will appear only if there are no applicable language-specific items \
of the same name in the same submenu.  For example, if you have the following \
three entries in the same menu:\n\
\n\
  Make Prototypes@C@C++\n\
  Make Prototypes@Java\n\
  Make Prototypes@*\n\
\n\
The first will be available when the language mode is C or C++, the second \
when the language mode is Java, and for all other language modes (including \
the \"Plain\" non-language mode).  If the entry:\n\
\n\
  Make Prototypes\n\
\n\
also exists, this will always appear, meaning that the menu will always \
have two \"Make Prototypes\" entries, whatever the language mode.\n\
\n\
Sharing Customizations with Other NEdit Users\n\
\n\
If you have written macro or shell menu commands, highlight patterns, \
or smart-indent macros that you want to share with other NEdit users, you \
can make a file which they can load into their NEdit environment.\n\
\n\
To load such a file, start NEdit with the command:\n\
\n\
   nedit -import <file>\n\
\n\
In the new NEdit session, verify that the imported patterns or macros do \
what you want, then select Preferences -> Save Defaults.  \
Saving incorporates the changes into your own .nedit file, so the next \
time you run NEdit, you will not have to import the distribution file.\n\
\n\
Loading a customization file is automated, but creating one is not.  \
To produce a file to be imported by other users, you must make a copy of \
your own .nedit file, and edit it, by hand, to remove everything but the \
few items of interest to the recipient.  Leave only the individual \
resource(s), and within those resources, only the particular macro, \
pattern, style, etc, that you wish to exchange.  For example, to share a \
highlighting pattern set, you would include the patterns, any new styles \
you added, and language mode information only if the patterns are \
intended to support a new language rather than updating an existing one. \
For example:\n\
\n\
   nedit.highlightPatterns:\\\n\
	My Language:1:0{\\n\\\n\
		Comment:\"#\":\"$\"::Comment::\\n\\\n\
		Loop Header:\"^[ \\\\t]*loop:\":::Loop::\\n\\\n\
	}\n\
   nedit.languageModes: My Language:.my::::::\n\
   nedit.styles: Loop:blue:Bold\n\
\n\
Resources are in the format of X resource files, but the format of text \
within multiple-item resources like highlight patterns, language modes, \
macros, styles, etc., are private to NEdit.  Each resource is a string \
which ends at the first newline character not escaped with \\, so you must \
be careful about how you treat ends of lines.  While you can generally \
just cut and paste indented sections, if something which was originally \
in the middle of a resource string is now at the end, you must remove the \
\\ line continuation character(s) so it will not join the next line into \
the resource.  Conversely, if something which was originally at the end \
of a resource is now in the middle, you'll have to add continuation \
character(s) to make sure that the resource string is properly continued \
from beginning to end, and possibly newline character(s) (\\n) to make \
sure that it is properly separated from the next item.",

"The Shell menu (Unix versions only) allows you to execute Unix shell commands \
from within NEdit.  You can add items to the menu to extend NEdit's command \
set or to incorporate custom automatic editing features using shell commands \
or editing languages like awk and \
sed.  To add items to the menu, select Preferences -> Default Settings \
Customize Menus -> Shell Menu.  NEdit comes pre-configured with \
a few useful Unix commands like spell and sort, but we encourage you to \
add your own custom extensions.\n\
\n\
Filter Selection... prompts you for a Unix command to \
use to process the currently selected text.  The output from this \
command replaces the contents of the selection.\n\
\n\
Execute Command... prompts you for a Unix command and replaces the \
current selection with the output of the command.  If there is no \
selection, it deposits the output at the current insertion point.\n\
\n\
Execute Command Line uses the position of the cursor in the window \
to indicate a line to execute as a shell command line.  The cursor may \
be positioned anywhere on the line.  This command allows you to use \
an NEdit window as an editable command window for saving output \
and saving commands for re-execution.\n\
\n\
The X resource called nedit.shell (See Customizing NEdit) determines \
which Unix shell is used to execute commands.  The default value for \
this resource is /bin/csh.",

"REGEX BASICS\n\
\n\
Regular expressions (regex's) are useful as a way to match inexact \
sequences of characters.  They can be used in the `Find...' and \
`Replace...' search dialogs and are at the core of Color Syntax \
Highlighting patterns.  To specify a regular expression in a search \
dialog, simply click on the `Regular Expression' radio button in the \
dialog.\n\
\n\
A regex is a specification of a pattern to be matched in the searched \
text.  This pattern consists of a sequence of tokens, each being able \
to match a single character or a sequence of characters in the text, \
or assert that a specific position within the text has been reached \
(the latter is called an anchor.)  Tokens (also called atoms) can be \
modified by adding one of a number of special quantifier tokens \
immediately after the token.  A quantifier token specifies how many \
times the previous token must be matched (see below.)\n\
\n\
Tokens can be grouped together using one of a number of grouping \
constructs, the most common being plain parentheses.  Tokens that are \
grouped in this way are also collectively considered to be a regex \
atom, since this new larger atom may also be modified by a quantifier.\n\
\n\
A regex can also be organized into a list of alternatives by \
separating each alternative with pipe characters, `|'.  This is called \
alternation.  A match will be attempted for each alternative listed, \
in the order specified, until a match results or the list of \
alternatives is exhausted (see \"Alternation\" below.)\n\
\n\
The Dot Meta Character\n\
\n\
If an un-escaped dot (`.') appears in a regex, it means to match any \
character exactly once.  By default dot will not match a newline \
character, but this behavior can be changed (see help topic \
\"Grouping\", item \"Matching Newlines\".)\n\
\n\
Character Classes\n\
\n\
A character class, or range, matches exactly one character of text, \
but the candidates for matching are limited to those specified by the \
class.  Classes come in two flavors as described below:\n\
\n\
   [...]   Regular class, match only characters listed.\n\
   [^...]  Negated class, match only characters NOT listed.\n\
\n\
As with the dot token, by default negated character classes do not \
match newline, but can be made to do so.\n\
\n\
The characters that are considered special within a class \
specification are different than the rest of regex syntax as follows. \
If the first character in a class is the `]' character (second \
character if the first character is `^') it is a literal character and \
part of the class character set.  This also applies if the first or \
last character is `-'.  Outside of these rules, two characters \
separated by `-' form a character range which includes all the \
characters between the two characters as well.  For example, `[^f-j]' \
is the same as `[^fghij]' and means to match any character that is not \
`f', `g', `h', `i', or `j'.\n\
\n\
Anchors\n\
\n\
Anchors are assertions that you are at a very specific position within \
the search text.  NEdit regular expressions support the following \
anchor tokens:\n\
\n\
   ^    Beginning of line\n\
   $    End of line\n\
   <    Left word boundary\n\
   >    Right word boundary\n\
   \\B   Not a word boundary\n\
\n\
Note that the \\B token ensures that the left and right characters are \
both delimiter characters, or that both left and right characters are \
non-delimiter characters.  Currently word anchors check only one \
character, e.g. the left word anchor `<' only asserts that the left \
character is a word delimiter character.  Similarly the right word \
anchor checks the right character.\n\
\n\
Quantifiers\n\
\n\
Quantifiers specify how many times the previous regular expression atom \
may be matched in the search text.  Some quantifiers can produce a \
large performance penalty, and can in some instances completely lock up \
NEdit.  To prevent this, avoid nested quantifiers, especially those of \
the maximal matching type (see below.)\n\
\n\
The following quantifiers are maximal matching, or \"greedy\", in that \
they match as much text as possible.\n\
\n\
   *   Match zero or more\n\
   +   Match one  or more\n\
   ?   Match zero or one\n\
\n\
The following quantifiers are minimal matching, or \"lazy\", in that they \
match as little text as possible.\n\
\n\
   *?   Match zero or more\n\
   +?   Match one  or more\n\
   ??   Match zero or one\n\
\n\
One final quantifier is the counting quantifier, or brace quantifier. \
It takes the following basic form:\n\
\n\
   {min,max}  Match from `min' to `max' times the\n\
              previous regular expression atom.\n\
\n\
If `min' is omitted, it is assumed to be zero.  If `max' is omitted, \
it is assumed to be infinity.  Whether specified or assumed, `min' \
must be less than or equal to `max'.  Note that both `min' and `max' \
are limited to 65535.  If both are omitted, then the construct is the \
same as `*'.   Note that `{,}' and `{}' are both valid brace \
constructs.  A single number appearing without a comma, e.g. `{3}' is \
short for the `{min,min}' construct, or to match exactly `min' number \
of times.\n\
\n\
The quantifiers `{1}' and `{1,1}' are accepted by the syntax, but are \
optimized away since they mean to match exactly once, which is \
redundant information.  Also, for efficiency, certain combinations of \
`min' and `max' are converted to either `*', `+', or `?' as follows:\n\
\n\
   {} {,} {0,}    *\n\
   {1,}           +\n\
   {,1} {0,1}     ?\n\
\n\
Note that {0} and {0,0} are meaningless and will generate an error \
message at regular expression compile time.\n\
\n\
Brace quantifiers can also be \"lazy\".  For example {2,5}? would try to \
match 2 times if possible, and will only match 3, 4, or 5 times if that \
is what is necessary to achieve an overall match.\n\
\n\
Alternation\n\
\n\
A series of alternative patterns to match can be specified by \
separating them with vertical pipes, `|'.  An example of alternation \
would be `a|be|sea'.  This will match `a', or `be', or `sea'.  Each \
alternative can be an arbitrarily complex regular expression.  The \
alternatives are attempted in the order specified.  An empty \
alternative can be specified if desired, e.g. `a|b|'.  Since an empty \
alternative can match nothingness (the empty string), this guarantees \
that the expression will match.\n\
\n\
Comments\n\
\n\
Comments are of the form `(?#<comment text>)' and can be inserted \
anywhere and have no effect on the execution of the regular \
expression.  They can be handy for documenting very complex regular \
expressions.  Note that a comment begins with `(?#' and ends at the \
first occurrence of an ending parenthesis, or the end of the regular \
expression... period.  Comments do not recognize any escape sequences.",

"ESCAPE SEQUENCES\n\
\n\
Special Control Characters\n\
\n\
In a regex, most ordinary characters match themselves.  For example, `ab%' \
would match anywhere `a' followed by `b' followed by `%' appeared in the \
text.  However, there are some special characters that are difficult or \
impossible to type.  Many of these characters have escape sequences \
(simple characters preceded by `\\') assigned to represent them.  NEdit \
recognizes the following special character escape sequences:\n\
\n\
   \\a  alert (bell)\n\
   \\b  backspace\n\
   \\e  ASCII escape character (***)\n\
   \\f  form feed (new page)\n\
   \\n  newline\n\
   \\r  carriage return\n\
   \\t  horizontal tab\n\
   \\v  vertical tab\n\
\n\
   *** For environments that use the EBCDIC character set,\n\
       when compiling NEdit set the EBCDIC_CHARSET compiler\n\
       symbol to get the EBCDIC equivalent escape\n\
       character.)\n\
\n\
Escaped Meta Characters\n\
\n\
Characters that have special meaning to the regex syntax are called meta \
characters.  NEdit provides the following escape sequences so that \
characters that are used by the regex syntax can be specified as ordinary \
characters and not interpreted as meta characters.\n\
\n\
   \\(  \\)  \\-  \\[  \\]  \\<  \\>  \\{  \\}\n\
   \\.  \\|  \\^  \\$  \\*  \\+  \\?  \\&  \\\\\n\
\n\
Octal and Hex Escape Sequences\n\
\n\
Any ASCII (or EBCDIC) character, except null, can be specified by using \
either an octal escape or a hexadecimal escape, each beginning with \\0 or \
\\x (or \\X) respectively.  For example, \\052 and \\X2A both specify the `*' \
character.  Escapes for null (\\00 or \\x0) are not valid and will generate \
an error message.  Also, any escape that exceeds \\0377 or \\xFF will either \
cause an error or have any additional character(s) interpreted literally. \
For example, \\0777 will be interpreted as \\077 (a `?' character) followed \
by `7' since \\0777 is greater than \\0377.\n\
\n\
An invalid digit will also end an octal or hexadecimal escape.  For \
example, \\091 will cause an error since `9' is not within an octal \
escape's range of allowable digits (0-7) and truncation before the `9' \
yields \\0 which is invalid.\n\
\n\
Shortcut Escapes\n\
\n\
NEdit defines some escape sequences that are handy shortcuts for commonly \
used character classes.\n\
\n\
   \\d  digits            0-9\n\
   \\l  letters           a-z and A-Z\n\
   \\s  whitespace        \\t, \\r, \\v, \\f, and space\n\
   \\w  word characters   a-z, A-Z, 0-9, and underscore, `_'\n\
\n\
\\D, \\L, \\S, and \\W are the same as the lowercase versions except that the \
resulting character class is negated.  For example, \\d is equivalent to \
`[0-9]', while \\D is equivalent to `[^0-9]'.\n\
\n\
These escape sequences can also be used within a character class.  For \
example, `[\\l_]' is the same as `[a-zA-Z_]'.  The escape sequences for \
special characters, and octal and hexadecimal escapes are also valid \
within a class.\n\
\n\
Word Delimiter Tokens\n\
\n\
Although not strictly a character class, the following escape sequences \
behave similarly to character classes:\n\
\n\
   \\y   Word delimiter character\n\
   \\Y   Not a word delimiter character\n\
\n\
The `\\y' token matches any single character that is one of the characters \
that NEdit recognizes as a word delimiter character, while the `\\Y' token \
matches any character that is NOT a word delimiter character.  Word \
delimiter characters are dynamic in nature, meaning that the user can \
change them through preference settings.  For this reason, they must be \
handled differently by the regular expression engine.  As a consequence of \
this, `\\y' and `\\Y' can not be used within a character class \
specification.",

"PARENTHETICAL CONSTRUCTS\n\
\n\
Capturing Parentheses\n\
\n\
Capturing Parentheses are of the form `(<regex>)' and can be used to group \
arbitrarily complex regular expressions.  Parentheses can be nested, but \
the total number of parentheses, nested or otherwise, is limited to 50 \
pairs.  The text that is matched by the regular expression between a \
matched set of parentheses is captured and available for text \
substitutions and backreferences (see below.)  Capturing parentheses carry \
a fairly high overhead both in terms of memory used and execution speed, \
especially if quantified by `*' or `+'.\n\
\n\
Non-Capturing Parentheses\n\
\n\
Non-Capturing Parentheses are of the form `(?:<regex>)' and facilitate \
grouping only and do not incur the overhead of normal capturing \
parentheses.  They should not be counted when determining numbers for \
capturing parentheses which are used with backreferences and \
substitutions.  Because of the limit on the number of capturing \
parentheses allowed in a regex, it is advisable to use non-capturing \
parentheses when possible.\n\
\n\
Positive Look-Ahead\n\
\n\
Positive look-ahead constructs are of the form `(?=<regex>)' and implement \
a zero width assertion of the enclosed regular expression.  In other \
words, a match of the regular expression contained in the positive \
look-ahead construct is attempted.  If it succeeds, control is passed to \
the next regular expression atom, but the text that was consumed by the \
positive look-ahead is first unmatched (backtracked) to the place in the \
text where the positive look-ahead was first encountered.\n\
\n\
One application of positive look-ahead is the manual implementation of a \
first character discrimination optimization.  You can include a positive \
look-ahead that contains a character class which lists every character \
that the following (potentially complex) regular expression could possibly \
start with.  This will quickly filter out match attempts that can not \
possibly succeed.\n\
\n\
Negative Look-Ahead\n\
\n\
Negative look-ahead takes the form `(?!<regex>)' and is exactly the same \
as positive look-ahead except that the enclosed regular expression must \
NOT match.  This can be particularly useful when you have an expression \
that is general, and you want to exclude some special cases.  Simply \
precede the general expression with a negative look-ahead that covers the \
special cases that need to be filtered out.\n\
\n\
Case Sensitivity\n\
\n\
There are two parenthetical constructs that control case sensitivity:\n\
\n\
   (?i<regex>)   Case insensitive; `AbcD' and `aBCd' are\n\
                 equivalent.\n\
\n\
   (?I<regex>)   Case sensitive;   `AbcD' and `aBCd' are\n\
                 different.\n\
\n\
Regular expressions are case sensitive by default, i.e `(?I<regex>)' is \
assumed.  All regular expression token types respond appropriately to case \
insensitivity including character classes and backreferences.  There is \
some extra overhead involved when case insensitivity is in effect, but \
only to the extent of converting each character compared to lower case.\n\
\n\
Matching Newlines\n\
\n\
NEdit regular expressions by default handle the matching of newlines in a \
way that should seem natural for most editing tasks.  There are \
situations, however, that require finer control over how newlines are \
matched by some regular expression tokens.\n\
\n\
By default, NEdit regular expressions will NOT match a newline character \
for the following regex tokens: dot (`.'); a negated character class \
(`[^...]'); and the following shortcuts for character classes:\n\
\n\
   `\\d', `\\D', `\\l', `\\L', `\\s', `\\S', `\\w', `\\W', `\\Y'\n\
\n\
The matching of newlines can be controlled for the `.' token, negated \
character classes, and the `\\s' and `\\S' shortcuts by using one of the \
following parenthetical constructs:\n\
\n\
   (?n<regex>)  `.', `[^...]', `\\s', `\\S' match newlines\n\
\n\
   (?N<regex>)  `.', `[^...]', `\\s', `\\S' don't match\n\
                                          newlines\n\
\n\
`(?N<regex>)' is the default behavior.\n\
\n\
Notes on New Parenthetical Constructs\n\
\n\
Except for plain parentheses, none of the parenthetical constructs capture \
text.  If that is desired, the construct must be wrapped with capturing \
parentheses, e.g. `((?i<regex))'.\n\
\n\
All parenthetical constructs can be nested as deeply as desired, except \
for capturing parentheses which have a limit of 50 sets of parentheses, \
regardless of nesting level.\n\
\n\
Back References\n\
\n\
Backreferences allow you to match text captured by a set of capturing \
parenthesis at some later position in your regular expression.  A \
backreference is specified using a single backslash followed by a single \
digit from 1 to 9 (example: \\3).  Backreferences have similar syntax to \
substitutions (see below), but are different from substitutions in that \
they appear within the regular expression, not the substitution string. \
The number specified with a backreference identifies which set of text \
capturing parentheses the backreference is associated with. The text that \
was most recently captured by these parentheses is used by the \
backreference to attempt a match.  As with substitutions, open parentheses \
are counted from left to right beginning with 1.  So the backreference \
`\\3' will try to match another occurrence of the text most recently \
matched by the third set of capturing parentheses.  As an example, the \
regular expression `(\\d)\\1' could match `22', `33', or `00', but wouldn't \
match `19' or `01'.\n\
\n\
A backreference must be associated with a parenthetical expression that is \
complete.  The expression `(\\w(\\1))' contains an invalid backreference \
since the first set of parentheses are not complete at the point where the \
backreference appears.\n\
\n\
Substitution\n\
\n\
Substitution strings are used to replace text matched by a set of \
capturing parentheses.  The substitution string is mostly interpreted as \
ordinary text except as follows.\n\
\n\
The escape sequences described above for special characters, and octal and \
hexadecimal escapes are treated the same way by a substitution string. \
When the substitution string contains the `&' character, NEdit will \
substitute the entire string that was matched by the `Find...' operation. \
Any of the first nine sub-expressions of the match string can also be \
inserted into the replacement string.  This is done by inserting a `\\' \
followed by a digit from 1 to 9 that represents the string matched by a \
parenthesized expression within the regular expression.  These expressions \
are numbered left-to-right in order of their opening parentheses.\n\
\n\
The capitalization of text inserted by `&' or `\\1', `\\2', ... `\\9' can be \
altered by preceding them with `\\U', `\\u', `\\L', or `\\l'.  `\\u' and `\\l' \
change only the first character of the inserted entity, while `\\U' and \
`\\L' change the entire entity to upper or lower case, respectively.",

"ADVANCED REGEX TOPICS\n\
\n\
Substitutions\n\
\n\
Regular expression substitution can be used to program automatic editing \
operations.  For example, the following are search and replace strings to \
find occurrences of the `C' language subroutine `get_x', reverse the first \
and second parameters, add a third parameter of NULL, and change the name \
to `new_get_x':\n\
\n\
   Search string:   `get_x *\\( *([^ ,]*), *([^\\)]*)\\)'\n\
   Replace string:  `new_get_x(\\2, \\1, NULL)'\n\
\n\
Ambiguity\n\
\n\
If a regular expression could match two different parts of the text, it \
will match the one which begins earliest.  If both begin in the same place \
but match different lengths, or match the same length in different ways, \
life gets messier, as follows.\n\
\n\
In general, the possibilities in a list of alternatives are considered in \
left-to-right order.  The possibilities for `*', `+', and `?' are \
considered longest-first, nested constructs are considered from the \
outermost in, and concatenated constructs are considered leftmost-first. \
The match that will be chosen is the one that uses the earliest \
possibility in the first choice that has to be made.  If there is more \
than one choice, the next will be made in the same manner (earliest \
possibility) subject to the decision on the first choice.  And so forth.\n\
\n\
For example, `(ab|a)b*c' could match `abc' in one of two ways.  The first \
choice is between `ab' and `a'; since `ab' is earlier, and does lead to a \
successful overall match, it is chosen.  Since the `b' is already spoken \
for, the `b*' must match its last possibility, the empty string, since it \
must respect the earlier choice.\n\
\n\
In the particular case where no `|'s are present and there is only one \
`*', `+', or `?', the net effect is that the longest possible match will \
be chosen.  So `ab*', presented with `xabbbby', will match `abbbb'.  Note \
that if `ab*' is tried against `xabyabbbz', it will match `ab' just after \
`x', due to the begins-earliest rule.  (In effect, the decision on where \
to start the match is the first choice to be made, hence subsequent \
choices must respect it even if this leads them to less-preferred \
alternatives.)\n\
\n\
References\n\
\n\
An excellent book on the care and feeding of regular expressions is\n\
\n\
	\"Mastering Regular Expressions\"\n\
	Jeffrey E. F. Friedl\n\
	(c) 1997, O'Reilly & Associates\n\
	ISBN 1-56592-257-3",

"EXAMPLES\n\
\n\
o  Entire line.\n\
\n\
  ^.*$\n\
\n\
o  Blank lines.\n\
\n\
  ^$\n\
\n\
o  Whitespace on a line.\n\
\n\
  \\s+\n\
\n\
o  Whitespace across lines.\n\
\n\
  (?n\\s+)\n\
\n\
o  Whitespace that spans at least two lines.  Note minimal\n\
   matching `*?' quantifier.\n\
\n\
  (?n\\s*?\\n\\s*)\n\
\n\
o  IP address (not robust.)\n\
\n\
  (?:\\d{1,3}(?:\\.\\d{1,3}){3})\n\
\n\
o  Two character US Postal state abbreviations (includes\n\
   territories.)\n\
\n\
  [ACDF-IK-PR-W][A-Z]\n\
\n\
o  Web addresses\n\
\n\
  (?:http://)?www\\.\\S+\n\
\n\
o  Case insensitive double words across line breaks.\n\
\n\
  (?i(?n<(\\S+)\\s+\\1>))\n\
\n\
o  Upper case words with possible punctuation.\n\
\n\
  <[A-Z][^a-z\\s]*>",

#ifndef VMS
"nedit [-read] [-create] [-line n | +n] [-server]\n\
    [-do command] [-tags file] [-tabs n] [-wrap]\n\
    [-nowrap] [-autowrap] [-autoindent] [-noautoindent]\n\
    [-autosave] [-noautosave] [-rows n] [-columns n]\n\
    [-font font] [-lm languagemode] [-geometry geometry]\n\
    [-iconic] [-noiconic] [-display [host]:server[.screen]\n\
    [-xrm resourcestring] [-svrname name] [-import file]\n\
    [-background color] [-foreground color] [file...]\n\
\n\
    -read -- Open the file Read Only regardless of\n\
    	the actual file protection.\n\
\n\
    -create -- Don't warn about file creation when\n\
    	a file doesn't exist.\n\
\n\
    -line n (or +n) -- Go to line number n\n\
\n\
    -server -- Designate this session as an NEdit\n\
        server, for processing commands from the nc\n\
        program.  nc can be used to interface NEdit to\n\
        code development environments, mailers, etc.,\n\
        or just as a quick way to open files from the\n\
        shell command line without starting a new NEdit\n\
        session.\n\
\n\
    -do command -- Execute an NEdit macro or action.\n\
        On each file following the -do argument on the\n\
        command line.  -do is particularly useful from\n\
        the nc program, where nc -do can remotely\n\
        execute commands in an nedit -server session.\n\
\n\
    -tags file -- Load a file of directions for finding\n\
        definitions of program subroutines and data\n\
        objects.  The file must be of the format gen-\n\
        erated by Exuberant Ctags, or the standard Unix\n\
        ctags command.\n\
\n\
    -tabs n -- Set tab stops every n characters.\n\
\n\
    -wrap, -nowrap -- Wrap long lines at the right edge\n\
        of the window rather than continuing them past\n\
        it.  (Continuous Wrap mode)\n\
\n\
    -autowrap, -noautowrap -- Wrap long lines when the\n\
        cursor reaches the right edge of the window by\n\
        inserting newlines at word boundaries.  (Auto\n\
        Newline Wrap mode)\n\
\n\
    -autoindent, -noautoindent -- Maintain a running\n\
        indent.\n\
\n\
    -autosave, -noautosave -- Maintain a backup copy of\n\
        the file being edited under the name ~filename \n\
        (on Unix) or _filename (on VMS).\n\
\n\
    -lm languagemode -- Initial language mode used for\n\
        editing succeeding files.\n\
\n\
    -rows n -- Default height in characters for an editing\n\
        window.\n\
\n\
    -columns n -- Default width in characters for an\n\
        editing window.\n\
\n\
    -font font (or -fn font) -- Font for text being\n\
        edited (Font for menus and dialogs can be set\n\
        with -xrm \"*fontList:font\").\n\
\n\
    -geometry geometry (or -g geometry) -- The initial\n\
        size and/or location of editor windows.  The\n\
        argument geometry has the form:\n\
\n\
        [<width>x<height>][+|-][<xoffset>[+|-]<yoffset>]\n\
\n\
        where <width> and <height> are the desired width\n\
        and height of the window, and <xoffset> and\n\
        <yoffset> are the distance from the edge of the\n\
        screen to the window, + for top or left, - for\n\
        bottom or right.  -geometry can be specified\n\
	for individual files on the command line.\n\
\n\
    -iconic, -noiconic -- Initial window state for\n\
        succeeding files.\n\
\n\
    -display [host]:server[.screen] -- The name of the\n\
        X server to use.  host specifies the machine,\n\
        server specifies the display server number, and\n\
        screen specifies the screen number.  host or\n\
        screen can be omitted and default to the local\n\
        machine, and screen 0.\n\
\n\
    -background color (or -bg color) -- Background color.\n\
        (background color for text can be set separately\n\
        with -xrm \"nedit*text.background: color\").\n\
\n\
    -foreground color (or -fg color) -- Foreground color.\n\
        (foreground color for text can be set separately\n\
        with -xrm \"nedit*text.foreground: color\").\n\
\n\
    -xrm resourcestring -- Set the value of an X resource\n\
        to override a default value (see Customizing NEdit).\n\
\n\
    -svrname name -- When starting nedit in server mode,\n\
        name the server, such that it responds to requests\n\
        only when nc is given a corresponding -svrname\n\
        argument.  By naming servers, you can run several\n\
        simultaneously, and direct files and commands\n\
        specifically to any one.\n\
\n\
    -import file -- loads an additional preferences file\n\
        on top of the existing defaults saved in your\n\
	.nedit file.  To incorporate macros, language\n\
	modes, and highlight patterns and styles written\n\
	by other users, run nedit with -import <file>, then\n\
	re-save your .nedit file with Preferences -> Save\n\
	Defaults.",
#else
"Command Format:\n\
\n\
    NEDIT [filespec[,...]]\n\
\n\
The following qualifiers are accepted:\n\
\n\
    /read -- Open the file Read Only regardless of\n\
    	the actual file protection.\n\
\n\
    /create -- Don't warn about file creation when\n\
    	a file doesn't exist.\n\
\n\
    /line=n -- Go to line #n\n\
\n\
    /server -- Designate this session as an NEdit\n\
        server for processing commands from the nc\n\
        program.  nc can be used to interface NEdit to\n\
        code development environments, mailers, etc.,\n\
        or just as a quick way to open files from the\n\
        shell command line without starting a new NEdit\n\
        session.\n\
\n\
    /do=command -- Execute an NEdit action routine.\n\
        on each file following the /do argument on the\n\
        command line.  /do is particularly useful from\n\
        the nc program, where nc /do can remotely\n\
        execute commands in an nedit /server session.\n\
    /tags=file -- Load a file of directions for finding\n\
        definitions of program subroutines and data\n\
        objects.  The file must be of the format gen-\n\
        erated by the Unix ctags command.\n\
\n\
    /wrap, /nowrap --  Wrap long lines at the right edge\n\
        of the window rather than continuing them past\n\
        it.  (Continuous Wrap mode)\n\
\n\
    /autowrap, /noautowrap -- Wrap long lines when the\n\
        cursor reaches the right edge of the window by\n\
        inserting newlines at word boundaries.  (Auto\n\
        Newline Wrap mode)\n\
\n\
    /autoindent, /noautoindent -- Maintain a running\n\
        indent.\n\
\n\
    /autosave, /noautosave -- Maintain a backup copy of\n\
        the file being edited under the name ~filename \n\
        (on Unix) or _filename (on VMS).\n\
\n\
    /rows=n -- Default width in characters for an editing\n\
        window.\n\
\n\
    /columns=n -- Default height in characters for an\n\
        editing window.\n\
\n\
    /font=font (or /fn=font) -- Font for text being\n\
        edited (Font for menus and dialogs can be set\n\
        with -xrm \"*fontList:font\").\n\
\n\
    /display [host]:server[.screen] -- The name of the\n\
        X server to use.  host specifies the machine,\n\
        server specifies the display server number, and\n\
        screen specifies the screen number.  host or\n\
        screen can be omitted and default to the local\n\
        machine, and screen 0.\n\
\n\
    /geometry=geometry (or /g=geometry) -- The initial\n\
        size and/or location of editor windows.  The\n\
        argument geometry has the form:\n\
\n\
        [<width>x<height>][+|-][<xoffset>[+|-]<yoffset>]\n\
\n\
        where <width> and <height> are the desired width\n\
        and height of the window, and <xoffset> and\n\
        <yoffset> are the distance from the edge of the\n\
        screen to the window, + for top or left, - for\n\
        bottom or right.\n\
\n\
    /background=color (or /bg=color) -- Background color.\n\
        (background color for text can be set separately\n\
        with /xrm \"nedit*text:background color\").\n\
\n\
    /foreground=color (or /fg=color) -- Foreground color.\n\
        (foreground color for text can be set separately\n\
        with -xrm \"nedit*text:foreground color\").\n\
\n\
    /xrm=resourcestring -- Set the value of an X resource\n\
        to override a default value (see Customizing NEdit).\n\
\n\
    /svrname=name -- When starting nedit in server mode,\n\
        name the server, such that it responds to requests\n\
        only when nc is given a correspoding -svrname\n\
        argument.  By naming servers, you can run several\n\
        simultaneously, and direct files and commands\n\
        specifically to any one.\n\
\n\
    /import=filename -- loads an additional preferences\n\
        file on top of the existing defaults saved in your\n\
	.nedit file.  To incorporate macros, language\n\
	modes, and highlight patterns and styles written\n\
	by other users, run nedit with /import=<file>, then\n\
	re-save your .nedit file with Preferences -> Save\n\
	Defaults.\n\
\n\
Unix-style command lines (but not file names) are also acceptable:\n\
\n\
    nedit -rows 20 -wrap file1.c file2.c\n\
\n\
is equivalent to:\n\
\n\
    nedit /rows=20/wrap file1.c, file2.c",
#endif /*VMS*/

"NEdit can be operated on its own, or as a two-part client/server \
application.  Client/server mode is useful for \
integrating NEdit with software \
development environments, mailers, and other programs; or just as a \
quick way to open files from the shell command line without starting \
a new NEdit session.\n\
\n\
To run NEdit in server mode, type:\n\
\n\
    nedit -server\n\
\n\
NEdit can also be started in server mode via the nc program when \
no servers are available.\n\
\n\
The nc (for NEdit Client) program, which is distributed along with NEdit, sends \
commands to an nedit server to open files, select lines, or execute editor \
actions.  It accepts a limited set of the nedit command line options: -read, \
-create, -line (or +n), -do, and a list of file names.  Listing a file on the \
nc command line means, open it if it is not already open and bring the window \
to the front.  -read and -create affect only newly opened files, but -line and \
-do can also be used on files which are already open (See  \
\"NEdit Command Line\" for more information).\n\
\n\
In typical Unix style, arguments affect the files which follow them on the \
command line, for example:\n\
\n\
    incorrect:   nc file.c -line 25\n\
    correct:     nc -line 25 file.c\n\
\n\
-read, -create, and -line affect all of the files which follow them on \
the command line.  The -do macro is executed only once, on the next file on \
the line.  -do without a file following it on the command line, executes \
the macro on the first available window (presumably when you give a -do \
command without a corresponding file or window, you intend it to do something \
independent of the window in which it happens execute).\n\
\n\
nc also accepts one command line option of its own, -noask (or -ask), \
which instructs it whether to automatically start a server if one is not \
available.  This is also settable via the X resource, nc.autoStart \
(See X Resources).\n\
\n\
Sometimes it is useful to have more than one NEdit server running, for \
example to keep mail and programming work separate.  The option, -svrname, \
to both nedit and nc, allows you to start, and communicate with, separate \
named servers.  A named server responds only to requests with the \
corresponding -svrname argument.  If you use ClearCase and are within \
a ClearCase view, the server name will default to the name of the view (based \
on the value of the CLEARCASE_ROOT environment variable).\n\
\n\
Communication between nc and nedit is through the X display. So as long as X \
windows is set up and working properly, nc will will work \
properly as well.  nc uses the DISPLAY environment variable, the machine name \
and your user name to find the appropriate server, meaning, if you have several \
machines sharing a common file system, nc will not be able to find a server \
that is running on a machine with a different host name, even though it may be \
perfectly appropriate for editing a given file.\n\
\n\
The command which nc uses to start an nedit server is settable \
via the X resource nc.serverCommand, by default, \
\"nedit -server\".",

"NEdit can be customized many different ways.  The most important \
user-settable options are presented in the Preferences menu, including \
all options that users might need to change during an editing session.  \
Options set in the Default Settings sub-menu of the Preferences menu \
can be preserved between sessions by selecting Save Defaults, \
which writes a file called .nedit in the user's \
home directory.  See the section titled \"Preferences\" for more details.\n\
\n\
User defined commands can be added to NEdit's Shell, Macro, and window \
background menus.  Dialogs for creating items in these menus can be found \
under Customize Menus in the Default Settings sub menu of the Preferences \
menu.\n\
\n\
For users who depend on NEdit every day and want to tune \
every excruciating detail, there are also X resources for tuning a vast number \
of such details, down to the color of each individual button.  See \
the section \"X Resources\" for more information, as well as a list of \
selected resources.\n\
\n\
The most common reason customizing your X resources for NEdit, however, \
is key binding.  While limited key binding can be done through Preferences \
settings (Preferences -> Default Settings -> Customize Menus), you can really \
only add keys this way, and each key must have a corresponding menu item.  \
Any significant changes to key binding should be made via the Translations \
resource and menu \
accelerator resources.  The sections titled \"Key Binding\" and \
\"X Resources\" have more information.",

"NEdit has additional options to those provided in the Preferences menu \
which are set using X resources.  \
Like most other X programs, NEdit can be customized to vastly unnecessary \
proportions, from initial window positions down to the font and shadow \
colors of each individual button (A complete discussion of how to do \
this is left to books on the X Windows System).  Key binding (see \
\"Key Binding\" \
is one of the most useful of these resource settable options.\n\
\n\
X resources are usually specified in a file called .Xdefaults or \
.Xresources in your \
home directory (on VMS this is sys$login:decw$xdefaults.dat).  On some \
systems, this file is read and its information \
attached to the X server (your screen) when you start X.  On other \
systems, the .Xdefaults file is read each time you run an X program.  \
When X resource values are attached to the X server, changes to the \
resource file are not available to application programs until you either run \
the xrdb program with the appropriate file as input, or re-start \
the X server.\n\
\n\
The .nedit File\n\
\n\
The .nedit (saved preferences) file is in the same format as an X resource \
file, and its contents can be moved into your X resource file.  One reason \
for doing so would be to attach server specific preferences, such as \
a default font to a particular X server.  Another reason for moving \
preferences into the X resource file would be to keep preferences menu \
options and resource settable options together in one place. Though \
the files are the same format, additional resources should not be added \
to the .nedit file, they will not be read, and NEdit modifies this file \
by overwriting it completely.  Note also that the contents of the .nedit \
file take precedence over the values of X resources.  Using \
Save Defaults after moving the contents of your .nedit file to your \
.Xdefaults file will re-create the .nedit file, interfering with \
the options that you have moved.\n\
\n\
Selected X Resource Names\n\
\n\
The following are selected NEdit resource names and default values \
for NEdit options not settable via the Preferences menu (for preference \
resource names, see your .nedit file):\n\
\n\
    nedit.tagFile: (not defined) -- The name of a file\n\
        of the type produced by Exuberant Ctags or the\n\
	Unix ctags command, which NEdit will load at\n\
	startup time (see Features for Programmers).  The\n\
	tag file provides a database from which NEdit can\n\
	automatically open files containing the definition\n\
	of a particular subroutine or data type.\n\
\n\
    nedit.shell: /bin/csh -- (Unix systems only) The Unix\n\
        shell (command interpreter) to use for executing\n\
        commands from the Shell menu\n\
\n\
    nedit.wordDelimiters: .,/\\\\`'!@#%^&*()-=+{}[]\":;<>?\n\
        -- The characters, in addition to blanks and tabs,\n\
        which mark the boundaries between words for the\n\
        move-by-word (Ctrl+Arrow) and select-word (double\n\
        click) commands.  Note that this default value may\n\
	be overridden by the setting in Preferences ->\n\
	Default Settings -> Language Modes....\n\
\n\
    nedit.remapDeleteKey: False -- Setting this resource\n\
        to True forcibly maps the delete key to backspace.\n\
	This can be helpful on systems where the bindings\n\
	have become tangled, and in environments which mix\n\
	systems with PC style keyboards and systems with\n\
	DEC and Macintosh keyboards.  Theoretically, these\n\
	bindings should be made using the standard X/Motif\n\
	mechanisms, outside of nedit.  In practice, some\n\
	environments where users access several different\n\
	systems remotely, can be very hard to configure.\n\
	If you've given up and are using a backspace key\n\
	halfway off the keyboard because you can't figure\n\
	out the bindings, set this to True.\n\
\n\
    nedit.stdOpenDialog: False -- Setting this resource\n\
        to True restores the standard Motif style of\n\
        Open dialog.  NEdit file open dialogs are missing\n\
        a text field at the bottom of the dialog, where\n\
        the file name can be entered as a string.  The\n\
        field is removed in NEdit to encourage users to\n\
        type file names in the list, a non-standard, but\n\
        much faster method for finding files.\n\
\n\
    nedit.bgMenuButton: ~Shift~Ctrl~Meta~Alt<Btn3Down> --\n\
        Specification for mouse button / key combination\n\
	to post the background menu (in the form of an X\n\
	translation table event specification).  The event\n\
	specification should be as specific as possible,\n\
	since it will override less specific translation\n\
    	table entries.\n\
\n\
    nedit.maxPrevOpenFiles: 30 -- Number of files listed\n\
        in the Open Previous sub-menu of the File menu.\n\
        Setting this to zero disables the Open Previous\n\
        menu item and maintenance of the .neditdb file.\n\
\n\
    nedit.printCommand: (system specific) -- Command used\n\
        by the print dialog to print a file, i.e. lp,\n\
        lpr, etc..  The command must be capable of\n\
        accepting input via stdin (standard input).\n\
\n\
    nedit.printCopiesOption: (system specific) -- Option\n\
        name used to specify multiple copies to the print\n\
        command.  If the option should be separated from\n\
        its argument by a space, leave a trailing space.\n\
        If blank, no \"Number of Copies\" item will\n\
        appear in the print dialog.\n\
\n\
    nedit.printQueueOption: (system specific) -- Option\n\
        name used to specify a print queue to the print\n\
        command.  If the option should be separated from\n\
        its argument by a space, leave a trailing space.\n\
        If blank, no \"Queue\" item will appear in the\n\
        print dialog.\n\
\n\
    nedit.printNameOption: (system specific) -- Option\n\
        name used to specify a job name to the print\n\
        command.  If the option should be separated from\n\
        its argument by a space, leave a trailing space.\n\
        If blank, no job or file name will be attached\n\
        to the print job or banner page.\n\
\n\
    nedit.printHostOption: (system specific) -- Option\n\
        name used to specify a host name to the print\n\
        command.  If the option should be separated from\n\
        its argument by a space, leave a trailing space.\n\
        If blank, no \"Host\" item will appear in the\n\
        print dialog.\n\
\n\
    nedit.printDefaultQueue: (system specific) -- The\n\
        name of the default print queue.  Used only to\n\
        display in the print dialog, and has no effect on\n\
        printing.\n\
\n\
    nedit.printDefaultHost: (system specific) -- The\n\
        node name of the default print host.  Used only\n\
        to display in the print dialog, and has no effect\n\
        on printing.\n\
\n\
    nedit.visualID: (Best) -- If your screen supports\n\
        multiple visuals (color mapping models), this\n\
	resource allows you to manually choose among\n\
	them.  The default value of \"Best\" chooses\n\
	the deepest (most colors) visual available.\n\
	Since NEdit does not depend on the specific\n\
	characteristics of any given color model, Best\n\
	probably IS the best choice for everyone, and\n\
	the only reason for setting this resource would\n\
	be to patch around some kind of X server problem.\n\
	The resource may also be set to \"Default\",\n\
	which chooses the screen's default visual (often\n\
	a color-mapped, PseudoColor, visual for\n\
	compatibility with older X applications).  It\n\
	may also be set to a numeric visual-id value\n\
	(use xdpyinfo to see the list of visuals\n\
	supported by your display), or a visual class\n\
	name: PseudoColor, DirectColor, TrueColor, etc..\n\
\n\
    nedit.installColormap (False) -- Force the\n\
        installation of a private colormap.  If you have\n\
	a humble 8-bit color display, and netscape is\n\
	hogging all of the color cells, you may want to\n\
	try turning this on.  On most systems, this will\n\
	result in colors flashing wildly when you switch\n\
	between NEdit and other applications.  But a few\n\
	systems (SGI) have hardware support for multiple\n\
	simultaneous colormaps, and applications with\n\
	installed colormaps are well behaved.\n\
\n\
    nedit.findReplaceUsesSelection: False -- Controls if\n\
        the Find and Replace dialogs are automatically\n\
        loaded with the contents of the primary selection.\n\
\n\
    nedit.multiClickTime: (system specific) -- Maximum\n\
        time in milliseconds allowed between mouse clicks\n\
        within double and triple click actions.\n\
\n\
    nedit*scrollBarPlacement: BOTTOM_LEFT -- How scroll\n\
        bars are placed in NEdit windows, as well as\n\
        various lists and text fields in the program.\n\
        Other choices are: BOTTOM_RIGHT, TOP_LEFT, or\n\
        TOP_RIGHT.\n\
\n\
    nedit*text.autoWrapPastedText: False -- When Auto-\n\
        Newline Wrap is turned on, apply automatic\n\
	wrapping (which normally only applies to typed\n\
	text) to pasted text as well.\n\
\n\
    nedit*text.heavyCursor: False -- For monitors with\n\
        poor resolution or users who have difficulty\n\
        seeing the cursor, makes the cursor in the text\n\
        editing area of the window heavier and darker.\n\
\n\
    nedit*text.foreground: black -- Foreground color of\n\
        the text editing area of the NEdit window.\n\
\n\
    nedit*text.background: white -- Background color of\n\
        the text editing area of the NEdit window.\n\
\n\
    nedit*text.selectForeground: black -- Foreground\n\
    	(text) color for selections in the text editing\n\
    	area of the NEdit window.\n\
\n\
    nedit*text.selectBackground: gray80 -- Color for\n\
    	selections in the text editing area of the NEdit\n\
    	window.\n\
\n\
    nedit*text.highlightForeground: white -- Foreground\n\
    	(text) color for highlights (parenthesis\n\
    	flashing) in the text editing area of the NEdit\n\
    	window.\n\
\n\
    nedit*text.highlightBackground: red -- Color for\n\
    	highlights (parenthesis flashing) in the text\n\
    	editing area of the NEdit window.\n\
\n\
    nedit*text.cursorForeground: black -- Color for\n\
    	text cursor in the text editing area of the\n\
    	NEdit window.\n\
\n\
    nedit*text.lineNumForeground: gray47 -- Color for\n\
    	displaying line numbers in the NEdit window.\n\
\n\
    nedit*text.blinkRate: 600 -- Blink rate of the text\n\
        insertion cursor in milliseconds.  Set to zero\n\
        to stop blinking.\n\
\n\
    nedit*text.Translations: -- Modifies key bindings\n\
        (see below).\n\
\n\
    nedit*foreground: black -- Default foreground color\n\
        for menus, dialogs, scroll bars, etc..\n\
\n\
    nedit*background: gray70 -- Default background color\n\
        for menus, dialogs, scroll bars, etc..\n\
\n\
    nedit*fontList: helvetica-bold-14 -- Default font\n\
        for menus, dialogs, scroll bars, etc..\n\
\n\
    nc.autoStart: False -- Whether the nc program should\n\
        automatically start an NEdit server (without\n\
        prompting the user) if an appropriate server is\n\
        not found.\n\
\n\
    nc.serverCommand: nedit -server -- Command used by\n\
        the nc program to start an NEdit server.\n\
\n\
Selected widget names (to which you may append .background .foreground, \
.fontList, etc., to change colors, fonts and other characteristics):\n\
\n\
    nedit*statsForm -- Statistics line and incremental\n\
        search bar.  Use this to set statistics line\n\
	background color.  To set attributes affecting\n\
	both the statistics line and the incremental\n\
	search bar, use '*' rather than '.' to separate\n\
	the resource name.  For example, to set the\n\
	foreground color: nedit*statsForm*foreground.\n\
\n\
    nedit*menuBar -- Top-of-window menu-bar\n\
\n\
    nedit*textHorScrollBar -- Horizontal scroll bar\n\
\n\
    nedit*textVertScrollBar -- Vertical scroll bar\n\
\n\
    nedit*helpText -- Help window text",
	
"There are several ways to change key bindings in NEdit.  The easiest \
way to add a new key binding in NEdit is to define a macro in Preferences -> \
Default Settings -> Customize Menus -> Macro Menu.  \
However, if you want to change existing bindings or add a significant \
number of new key bindings you will need to do so via X resources.\n\
\n\
Before reading this section, \
you must understand how to set X resources (see the help section \"X \
Resources\").  Since setting X resources is tricky, it is also helpful \
when working on key-binding, to set some easier-to-verify resource at the \
same time, \
as a simple check that the NEdit program is actually seeing your changes.  \
The appres program is also very helpful in checking that the resource \
settings that you make, actually reach the program for which they are \
intended in the correct form.\n\
\n\
Key Binding in General\n\
\n\
Keyboard commands are associated with editor action routines through \
two separate mechanisms in NEdit.  Commands which appear in pull-down \
menus have individual resources designating a keyboard equivalent to \
the menu command, called an accelerator key.  Commands which do not have \
an associated menu item are bound to keys via the X toolkit translation \
mechanism.  The methods for changing these two kinds of bindings are \
quite different.\n\
\n\
Key Binding Via Translations\n\
\n\
The most general way to bind actions to keys in NEdit is to use the \
translation table associated with the text widget.  To add a binding to \
Alt+Y to insert the string \"Hi!\", for example, add lines similar to the \
following to your X resource file:\n\
\n\
  NEdit*text.Translations: #override \\n\\\n\
    Alt<Key>y: insert_string(\"Hi!\") \\n\n\
\n\
The Help topic \"Action \
Routines\" lists the actions available to be bound.\n\
\n\
Translation tables map key and mouse presses, \
window operations, and other kinds of events, to actions.   \
The syntax for translation tables is simplified here, and you.  \
may need to refer to a book on the X window system for more detailed \
information.\n\
\n\
Note that accelerator resources (discussed below) override \
translations, and that most Ctrl+letter and Alt+letter combinations are \
already bound to an accelerator key.  To use one of these combinations from \
a translation table, therefore, you must first un-bind the original menu \
accelerator.\n\
\n\
A resource for changing a translation table \
consists of a keyword; #override, #augment, or #replace; \
followed by lines (separated by newline characters) pairing events with \
actions.  Events begin with modifiers, like Ctrl, Shift, or Alt, \
followed by the event type in <>.  BtnDown, Btn1Down, Btn2Down, Btn1Up, \
Key, KeyUp are valid event types.  For key presses, the \
event type is followed by the name of the key.  You can specify a \
combination of events, such as a sequence of key presses, by separating \
them with commas.  The other half of the event/action pair is a set \
of actions.  These are separated from the event specification by a colon \
and from each other by spaces.  Actions \
are names followed by parentheses, optionally \
containing one or more parameters separated by comas.\n\
\n\
Changing Menu Accelerator Keys\n\
\n\
The menu shortcut keys shown at the right of NEdit menu items can also \
be changed via X resources.  Each menu item has two \
resources associated with it, accelerator, the \
event to trigger the menu item; and acceleratorText, the string \
shown in the menu.  \
The form of the accelerator resource is the same as events for translation \
table entries discussed above, though multiple keys and other subtleties \
are not allowed.  \
The resource name for a menu is the title in lower case, followed by \
\"Menu\", the resource name of menu item is the name in lower case, run \
together, with words separated by caps, and all punctuation removed.  \
For example, to change Cut to Ctrl+X, \
you would add the following to your .Xdefaults file:\n\
\n\
    nedit*editMenu.cut.accelerator: Ctrl<Key>x\n\
    nedit*editMenu.cut.acceleratorText: Ctrl+X\n\
\n\
Accelerator keys with optional shift key modifiers, like Find..., have an \
additional accelerator resource with Shift appended to the name.  For \
example:\n\
\n\
    nedit*searchMenu.find.acceleratorText: [Shift]Alt+F\n\
    nedit*searchMenu.find.accelerator: Alt<Key>f\n\
    nedit*searchMenu.findShift.accelerator: Shift Alt<Key>f",

"Learn/Replay\n\
\n\
Selecting Learn Keystrokes from the Macro menu puts NEdit in learn mode.  \
In learn mode, keystrokes and menu commands are recorded, to be played \
back later, using the Replay Keystrokes command, or pasted into a \
macro in the Macro Commands dialog of the Default Settings menu in \
Preferences.\n\
\n\
Note that only keyboard and menu commands are recorded, not mouse clicks \
or mouse movements since these have no absolute point of reference, such \
as cursor or selection position.  When you do a mouse-based operation in \
learn mode, NEdit will beep (repeatedly) to remind you that the operation \
was not recorded.\n\
\n\
Learn mode is also the quickest and easiest method for writing macros.  The \
dialog for creating macro commands contains a button labeled \"Paste Learn / \
Replay Macro\", which will deposit the last sequence learned into the body \
of the macro.\n\
\n\
Repeating Actions and Learn/Replay Sequences\n\
\n\
You can repeat the last (keyboard-based) command, or learn/replay sequence \
with the Repeat... command in the Macro menu.  To repeat an action, first \
do the action (i.e. insert a character, do a \
search, move the cursor), then select Repeat..., \
decide how or how many times you want it repeated, and click OK.  For \
example, to move down 30 lines through a file, you could type: <Down Arrow> \
Ctrl+, 29 <Return>.  To repeat a \
learn/replay sequence, first learn it, then select Repeat..., click on \
Learn/Replay and how you want it repeated, then click OK.\n\
\n\
If the commands \
you are repeating advance the cursor through the file, you can also repeat \
them within a range of characters, or \
from the current cursor position to the end of the file.  To iterate over \
a range of characters, use the primary \
selection (drag the left mouse button over the text) to mark the range \
you want to operate on, \
and select \"In Selection\" in the Repeat dialog.\n\
\n\
When using In \"Selection\" or \"To End\" with a learned sequence, try \
to do cursor movement as the last step in the sequence, since testing of \
the cursor position is only done at the end of the sequence execution.  \
If you do cursor movement first, for example searching for a particular \
word then doing a modification, the position of the cursor won't be \
checked until the sequence has potentially gone far beyond the end of \
your desired range.\n\
\n\
It's easy for a repeated command to get out of hand, and you can easily \
generate an infinite loop by using range iteration on a command which \
doesn't progress.  To cancel a repeating command in progress, type Ctrl+. \
(period), or select Cancel Macro from the Macro menu.",

"Macros can be called from Macro menu commands, window background menu \
commands, within the smart-indent \
framework, and from the .neditmacro file.  Macro menu and window \
background menu commands are defined under Preferences -> \
Default Settings -> Customize Menus.  Help on creating items in these menus \
can be found in the section, Help -> Customizing -> Preferences.  \
The .neditmacro file is a file of macro commands and definitions which you \
can create in your home directory, and \
which NEdit will automatically load when it is first started.\n\
\n\
NEdit's macro language is a simple interpreter with integer \
arithmetic, dynamic strings, and C-style looping constructs \
(very similar to the procedural portion of the Unix awk program).  \
From the macro language, you can call the same action routines \
which are bound to keyboard keys and menu items, as well \
additional subroutines for accessing and manipulating editor data, \
which are specific to the macro language (these are listed in the \
sections titled Macro Subroutines, and Actions).\n\
\n\
\n\
SYNTAX\n\
\n\
An NEdit macro language program consists of a list of statements, \
each terminated by a newline.  Groups of statements which are \
executed together conditionally, such as the body of a loop, are \
surrounded by curly braces \"{}\".\n\
\n\
Blank lines and comments are also allowed.  Comments begin with a \
\"#\" and end with a newline, and can appear either on a line by \
themselves, or at the end of a statement.\n\
\n\
Statements which are too long to fit on a single line may be \
split across several lines, by placing a backslash \"\\\" character \
at the end of each line to be continued.\n\
\n\
\n\
DATA TYPES\n\
\n\
The NEdit macro language recognizes only three data types, dynamic \
character strings, integer values and associative arrays.  In general \
strings and integers can be used interchangeably.  If a string \
represents an integer value, it can be used as an integer.  Integers \
can be compared and concatenated with strings.  Arrays may contain \
integers, strings, or arrays. Arrays are stored key/value pairs. Keys \
are always stored as strings.\n\
\n\
Integer Constants\n\
\n\
Integers are non-fractional numbers in the range of -2147483647 to \
2147483647.  Integer constants must be in decimal.  For example:\n\
\n\
  a = -1\n\
  b = 1000\n\
\n\
Character String Constants\n\
\n\
Character string constants are enclosed in \
double quotes.  For example:\n\
\n\
   a = \"a string\"\n\
   dialog(\"Hi there!\", \"Dismiss\")\n\
\n\
Strings may also include C-language style escape sequences:\n\
\n\
   \\\\ Backslash      \\t Tab		  \\f Form feed\n\
   \\\" Double quote   \\b Backspace	  \\a Alert\n\
   \\n Newline	     \\r Carriage return   \\v Vertical tab\n\
\n\
For example, to send output to the terminal \
from which nedit was started, a newline character is neccessary \
because, like printf, t_print requires explicit newlines, and \
also buffers its output on a per-line basis:\n\
\n\
   t_print(\"a = \" a \"\\n\")\n\
\n\
\n\
VARIABLES\n\
\n\
Variable names must begin either with a letter (local variables), or \
a $ (global variables).  Beyond the first character, variables may \
also contain numbers and underscores `_'.  Variables are called in to \
existence just by setting them (no explicit declarations are necessary).\n\
\n\
Local variables are limited in scope to \
the subroutine (or menu item definition) in which they appear.  \
Global variables are accessible from all routines, and their values \
persist beyond the call which created them, until reset.\n\
\n\
Built-in Variables\n\
\n\
NEdit has a number of permanently defined variables, which are used \
to access global editor information and information about the the \
window in which the macro is executing.  These are listed along with \
the built in functions in the section titled Macro Subroutines.\n\
\n\
\n\
FUNCTIONS and SUBROUTINES\n\
\n\
The syntax of a function or subroutine call is:\n\
\n\
   function_name(arg1, arg2, ...)\n\
\n\
where arg1, arg2, etc. represent up to 9 argument values which are \
passed to the routine being called.  A function or subroutine call \
can be on a line by itself, as above, or if it \
returns a value, can be invoked within a character or numeric \
expression:\n\
\n\
   a = fn1(b, c) + fn2(d)\n\
   dialog(\"fn3 says: \" fn3())\n\
\n\
Arguments are passed by value.  This means that you can not return \
values via the argument list, only through the function value or \
indirectly through agreed-upon global variables.\n\
\n\
Built-in Functions\n\
\n\
NEdit has a wide range of built in functions which can be called from \
the macro language.  These routines are divided into two classes, \
macro-language functions, and editor action routines.  Editor action \
routines are more flexible, in that they may be called either from \
the macro language, or bound directly to keys via translation tables.  \
They are also limited, however, in that they can not return values.  \
Macro language routines can return values, but can not be bound to \
keys in translation tables.\n\
\n\
Nearly all of the built-in subroutines operate on an implied window, \
which is initially the window from which the macro was started.  To \
manipulate the contents of other windows, use the focus_window \
subroutine to change the focus to the ones you wish to modify.  \
focus_window can also be used to iterate over all of the currently open \
windows, using the special keyword names, \"last\" and \"next\".\n\
\n\
For backwards compatibility, hyphenated action routine names are \
allowed, and most of the existing action routines names which contain \
underscores have an equivalent version containing hyphens ('-') \
instead of underscores.  Use of these names is discouraged.  The macro \
parser resolves the ambiguity between '-' as the subtraction/negation \
operator, and - as part of an action routine name by assuming \
subtraction unless the symbol specifically matches an action routine \
name.\n\
\n\
User Defined Functions\n\
\n\
Users can define their own macro subroutines, using the define \
keyword:\n\
\n\
   define subroutine_name {\n\
      < body of subroutine >\n\
   }\n\
\n\
Macro definitions can not appear within other definitions, or within macro \
menu item definitions (usually they are found in the .neditmacro file).\n\
\n\
The arguments with which a user-defined subroutine or function was \
invoked, are presented as $1, $2, ... , $9.  The number of arguments \
can be read from $n_args.\n\
\n\
To return a value from a subroutine, and/or to exit from the subroutine \
before the end of the subroutine body, use the return statement:\n\
\n\
   return <value to return>\n\
\n\
\n\
OPERATORS AND EXPRESSIONS\n\
\n\
Operators have the same meaning and precedence that they do in C, \
except for ^, which raises a number to a power (y^x means y to the x \
power), rather than bitwise exclusive OR.  The table below lists \
operators in decreasing order of precedence.\n\
\n\
   Operators		    Associativity\n\
   ()\n\
   ^			    right to left\n\
   - ! ++ --		    (unary)\n\
   * / %		    left to right\n\
   + -  		    left to right\n\
   > >= < <= == !=	    left to right\n\
   &			    left to right\n\
   |			    left to right\n\
   &&			    left to right\n\
   ||			    left to right\n\
   (concatenation)	    left to right\n\
   = += -= *= /= %=, &= |=  right to left\n\
\n\
The order in which operands are evaluated in an expression is \
undefined, except for && and ||, which like C, evaluate operands left \
to right, but stop when further evaluation would no longer change the \
result.\n\
\n\
Numerical Operators\n\
\n\
The numeric operators supported by the NEdit macro language are listed \
below:\n\
\n\
   + addition\n\
   - subtraction or negation\n\
   * multiplication\n\
   / division\n\
   % modulo\n\
   ^ power\n\
   & bitwise and\n\
   | bitwise or\n\
\n\
Increment (++) and decrement (--) operators can also be appended or \
prepended to variables within an expression.  Prepended increment/decrement \
operators act before the variable is evaulated.  Appended increment/decrement \
operators act after the variable is evaluated.\n\
\n\
Logical and Comparison Operators\n\
\n\
Logical operations produce a result of 0 (for false) or 1 (for true).  \
In a logical operation, any non-zero value is recognized to mean true.  \
The logical and comparison operators allowed in the NEdit macro \
language are listed below:\n\
\n\
   && logical and\n\
   || logical or\n\
   !  not\n\
   >  greater\n\
   <  less\n\
   >= greater or equal\n\
   <= less or equal\n\
   == equal (integers and/or strings)\n\
   != not equal (integers and/or strings)\n\
\n\
Character String Operators\n\
\n\
The \"operator\" for concatenating two strings is the absence of an \
operator.  Adjoining character strings with no operator in between \
means concatenation:\n\
\n\
   d = a b \"string\" c\n\
   t_print(\"the value of a is: \" a)\n\
\n\
Comparison between character strings is done with the == and != \
operators, (as with integers).  There are a number of useful \
built-in routines for working with character strings, which are \
listed in the section called Macro Subroutines.\n\
\n\
Arrays and Array Operators\n\
\n\
Arrays may contain either strings, integers, or other arrays. Arrays \
are associative, which means that they relate two pieces of information, \
the key and the value. The key is always a string, although you can use \
integers and strings, they are always converted to strings. Array keys \
can also contain multiple sub-scripts:\n\
\n\
    x[1,2,3] = \"string\"\n\
    x[1 $sub_sep 2 $sub_sep 3] = \"string\"\n\
    x[\"1\" $sub_sep \"2\" $sub_sep \"3\"] = \"string\"\n\
\n\
The sub-scripts are concatenated with $sub_sep. The above assignments \
are eqivalent. To determine if a given key is in an array, use the in \
keyword.\n\
\n\
    if (\"6\" in x) <body>\n\
\n\
If the left side of the in keyword is an array, the result is true if \
every key in the left array is in the right array. Array values are \
not compared.\n\
\n\
To iterate through all the keys of an array use the for looping \
construct. Keys are not guarranteed in any particular order:\n\
\n\
    for (aKey in x) <body>\n\
\n\
Elements can be removed from an array using the delete command:\n\
\n\
    delete x[3] # deletes element with key 3\n\
    delete x[] # deletes all elements\n\
\n\
The number of elements in an array can be determined by referencing \
the array with no sub-scripts:\n\
\n\
    dialog(\"array x has \" x[] \" elemnts\")\n\
\n\
Arrays can be combined with some operators. All the following operators \
only compare the keys of the arrays.\n\
\n\
   + merge arrays\n\
   - remove keys\n\
   & common keys\n\
   | all keys that are not shared\n\
\n\
When duplicate keys are encountered using the + and & operators, the \
values from the array on the right side of the operators are used \
for the result. All of the above operators are array only, meaning \
both the left and right sides of the operator must be arrays. The \
results are also arrays.\n\
\n\
\n\
LOOPING AND CONDITIONALS\n\
\n\
NEdit supports looping constructs: for and while, and conditional \
statements: if and else, with essentially the same syntax as C:\n\
\n\
   for (<init>, ...; <condition>; <increment>, ...) <body>\n\
\n\
   while (<condition>) <body>\n\
\n\
   if (<condition>) <body>\n\
\n\
   if (<condition>) <body> else <body>\n\
\n\
<body>, as in C, can be a single statement, or a list of statements \
enclosed in curly braces ({}).  <condition> is an expression which \
must evaluate to true for the statements in <body> to be executed.  \
for loops may also contain initialization statements, <init>, \
executed once at the beginning of the loop, and increment/decrement \
statements (or any arbitrary statement), which are executed at the end \
of the loop, before the condition is evaluated again.\n\
\n\
Examples:\n\
\n\
  for (i=0; i<100; i++)\n\
     j = i * 2\n\
\n\
  for (i=0, j=20; i<20; i++, j--) {\n\
     k = i * j\n\
     t_print(i, j, k)\n\
  }\n\
\n\
  while (k > 0)\n\
  {\n\
     k = k - 1\n\
     t_print(k)\n\
  }\n\
\n\
  for (;;) {\n\
     if (i-- < 1)\n\
         break\n\
  }\n\
\n\
Loops may contain break and continue statements.  A break statement \
causes an exit from the innermost loop, a continue statement transfers \
control to the end of the loop.",

"Built in Variables\n\
\n\
$active_pane -- Index of the current pane.\n\
\n\
$auto_indent -- Contains the current preference for auto indent. Can be \
\"off\", \"on\" or \"auto\".\n\
\n\
$cursor -- Position of the cursor in the current window.\n\
\n\
$column -- Column number of the cursor position in the current window.\n\
\n\
$display_width -- Width of the current pane in pixels.\n\
\n\
$em_tab_dist -- If tab emulation is turned on in the Tabs... dialog of \
the Preferences menu, value is the distance between emulated tab stops.  If \
tab emulation is turned off, value is -1.\n\
\n\
$file_format -- Current newline format that the file will be saved with. Can \
be \"unix\", \"dos\" or \"macintosh\".\n\
\n\
$file_name -- Name of the file being edited in the current window, \
stripped of directory component.\n\
\n\
$file_path -- Directory component of file being edited in the \
current window.\n\
\n\
$font_name -- Contains the current plain text font name.\n\
\n\
$font_name_bold -- Contains the current bold text font name.\n\
\n\
$font_name_bold_italic -- Contains the current bold-italic text font name.\n\
\n\
$font_name_italic -- Contains the current italic text font name.\n\
\n\
$highlight_syntax -- Whether syntax highlighting is turned on.\n\
\n\
$incremental_backup -- Contains 1 if incremental auto saving is on, \
otherwise 0.\n\
\n\
$incremental_search_line -- Has a value of 1 if the preference is \
selected to always show the incremental search line, otherwise 0.\n\
\n\
$language_mode -- Name of language mode set in the current window.\n\
\n\
$line -- Line number of the cursor position in the current window.\n\
\n\
$locked -- True if the file has been locked by the user.\n\
\n\
$make_backup_copy -- Has a value of 1 if original file is kept in a \
backup file on save, otherwise 0.\n\
\n\
$max_font_width -- The maximum font width of all the active styles. \
Syntax highlighting styles are only considered if syntax highlighting \
is turned on.\n\
\n\
$min_font_width -- The minimum font width of all the active styles. \
Syntax highlighting styles are only considered if syntax highlighting \
is turned on.\n\
\n\
$modified -- True if the file in the current window has been modified \
and the modifications have not yet been saved.\n\
\n\
$n_display_lines -- The number of lines visible in the currently active \
pane.\n\
\n\
$n_panes -- The number of panes in the current window.\n\
\n\
$overtype_mode -- True if in Overtype mode.\n\
\n\
$read_only -- True if the file is read only.\n\
\n\
$selection_start, $selection_end -- Beginning and ending positions of \
the primary selection in the current window, or -1 if there is no \
text selected in the current window.\n\
\n\
$selection_left, $selection_right -- Left and right character offsets of \
the rectangular (primary) selection in the current window, or -1 if \
there is no selection or it is not rectangular.\n\
\n\
$show_line_numbers -- Whether line numbers are shown next to the text.\n\
\n\
$show_matching -- Contains 1 if matching items are highlighted, such as \
\"[]\" and \"{}\" pairs, otherwise 0.\n\
\n\
$statistics_line -- Has a value of 1 if the statistics line is shown, \
otherwise 0.\n\
\n\
$sub_sep -- Contains the value of the array sub-script separation string.\n\
\n\
$tab_dist -- The distance between tab stops for a hardware tab \
character, as set in the Tabs... dialog of the Preferences menu.\n\
\n\
$text_length -- The length of the text in the current window.\n\
\n\
$top_line -- The line number of the top line of the currently active \
pane.\n\
\n\
$use_tabs -- Whether the user is allowing the NEdit to insert tab \
characters to maintain spacing in tab emulation and rectangular \
dragging operations. (The setting of the \"Use tab characters in \
padding and emulated tabs\" button in the Tabs... dialog of the \
Preferences menu.)\n\
\n\
$wrap_margin -- The right margin in the current window for text \
wrapping and filling.\n\
\n\
$wrap_text -- The current wrap text mode. Values are \"none\", \
\"auto\" or \"continuous\".\n\
\n\
\n\
Built-in Subroutines\n\
\n\
append_file(string, filename) -- Appends a string to \
a named file.  Returns 1 on successful write, or 0 \
if unsuccessful.\n\
\n\
beep() -- Ring the bell\n\
\n\
clipboard_to_string() -- Returns the contents of the clipboard \
as a macro string.  Returns empty string on error.\n\
\n\
dialog(message, btn_1_label, btn_2_label, ...) -- Pop up a dialog \
for querying and presenting information to the user.  First \
argument is a string to show in the message area of the dialog.  Up \
to eight additional optional arguments represent labels for buttons \
to appear along the bottom of the dialog.  Returns the number of \
the button pressed (the first button is number 1), or 0 if the \
user closed the dialog via the window close box.\n\
\n\
focus_window(window_name) -- Sets the window on which subsequent \
macro commands operate.  window_name can be either a fully qualified \
file name, or one of \"last\" for the last window created, or \"next\" \
for the next window in the chain from the currently focused window (the \
first window being the one returned from calling focus_window(\"last\").  \
Returns the name of the newly-focused window, or an empty string \
if the requested window was not found.\n\
\n\
get_character(position) -- Returns the single character at the \
position indicated by the first argument to the routine from the \
current window.\n\
\n\
get_range(start, end) -- Returns the text between a starting and \
ending position from the current window.\n\
\n\
get_selection() -- Returns a string containing the text currently \
selected by the primary selection either from the current window (no \
keyword), or from anywhere on the screen (keyword \"any\").\n\
\n\
getenv(name) -- Gets the value of an environment variable.\n\
\n\
length(string) -- Returns the length of a string\n\
\n\
list_dialog(message, text, btn_1_label, btn_2_label, ...) -- Pop up a \
dialog for prompting the user to choose a line from the given text \
string.  The first argument is a message string to be used as a title \
for the fixed text describing the list.  The second string provides \
the list data: this is a text string in which list entries are \
separated by newline characters.  Up to seven additional optional \
arguments represent labels for buttons to appear along the bottom of \
the dialog.  Returns the line of text selected by the user as the \
function value (without any newline separator) or the empty string if \
none was selected, and number of the button pressed (the first button \
is number 1), in $list_dialog_button.  If the user closes the dialog \
via the window close box, the function returns the empty string, and \
$list_dialog_button returns 0.\n\
\n\
max(n1, n2, ...) -- Returns the maximum value of all of its \
arguments\n\
\n\
min(n1, n2, ...) -- Returns the minimum value of all of its \
arguments\n\
\n\
read_file(filename) -- Reads the contents of a text file into a \
string.  On success, returns 1 in $read_status, and the contents of \
the file as a string in the subroutine return value.  On failure, \
returns the empty string \"\" and an 0 $read_status.\n\
\n\
replace_in_string(string, search_for, replace_with, [type]) -- \
Replaces all occurrences of a search string in a string with a \
replacement string.  Arguments are 1: string to search in, 2: \
string to search for, 3: replacement string.  Argument 4 is an \
optional search type, one of \"literal\", \"case\" or \"regex\".  The \
default search type is \"literal\".  Returns a new string with all of \
the replacements done, or an empty string (\"\") if no occurrences \
were found.\n\
\n\
replace_range(start, end, string) -- Replaces all of the text in the \
current window between two positions\n\
\n\
replace_selection(string) -- Replaces the primary-selection \
selected text in the current window.\n\
\n\
replace_substring(string, start, end, replace_with) -- Replacing a \
substring between two positions in a string within another string.\n\
\n\
search(search_for, start, [search_type, wrap, direction]) -- \
Searches silently in a window without dialogs, beeps, or changes to \
the selection.  Arguments are: 1: string to search for, 2: starting \
position. Optional arguments may include the strings: \"wrap\" to \
make the search wrap around the beginning or end of the string, \
\"backward\" or \"forward\" to change the search direction (\"forward\" \
is the default), \"literal\", \"case\" or \"regex\" to change the search \
type (default is \"literal\").  Returns the starting position of the \
match, or -1 if nothing matched. also returns the ending position \
of the match in search_end\n\
\n\
search_string(string, search_for, start, [search_type, direction]) \
-- Built-in macro subroutine for searching a string.  Arguments are \
1: string to search in, 2: string to search for, 3: starting \
position.  Optional arguments may include the strings: \"wrap\" to \
make the search wrap around the beginning or end of the string, \
\"backward\" or \"forward\" to change the search direction (\"forward\" \
is the default), \"literal\", \"case\" or \"regex\" to change the search \
type (default is \"literal\").  Returns the starting position of the \
match, or -1 if nothing matched.  Also returns the ending position \
of the match in $search_end\n\
\n\
select(start, end) -- Selects (with the primary selection) text in \
the current buffer between a starting and ending position.\n\
\n\
select_rectangle(start, end, left, right) -- Selects a rectangular \
area of text between a starting and ending position, and confined \
horizontally to characters displayed between positions \"left\", and \
\"right\".\n\
\n\
set_cursor_pos(pos) -- Set the cursor position for the current window.\n\
\n\
shell_command(command, input_string) -- executes a shell command, feeding \
it input from input_string.  On completion, output from the command \
is returned as the function value, and the command's exit status is \
returned in the global variable $shell_cmd_status.\n\
\n\
split(string, separation_string [, search_type]) -- Splits a string \
using the separator specified. Optionally the search_type argument \
can specify how the separation_string is interpreted. The default \
is \"literal\". The returned value is an array with keys beginning \
at 0.\n\
\n\
string_dialog(message, btn_1_label, btn_2_label, ...) -- Pop up a \
dialog prompting the user to enter information.  The first \
argument is a string to show in the message area of the dialog.  Up \
to nine additional optional arguments represent labels for buttons \
to appear along the bottom of the dialog.  Returns the string \
entered by the user as the function value, and number of the button \
pressed (the first button is number 1), in $string_dialog_button.  \
If the user closes the dialog via the window close box, the function \
returns the empty string, and $string_dialog_button returns 0.\n\
\n\
string_compare(string1, string2 [, consider-case]) -- Compare two \
strings and return 0 if they are equal, -1 if string1 is less than \
string2 or 1 if string1 is greater than string2. The default is to \
do a case sensitive comparison. Optionally a \"nocase\" argument \
can be supplied to force the comparison to ingore case differences. \
\n\
string_to_clipboard(string) -- Copy the contents of a macro string \
to the clipboard.\n\
\n\
substring(string, start, end) -- Returns the portion of a string \
between a starting and ending position.\n\
\n\
t_print(string1, string2, ...) -- Writes strings to the terminal \
(stdout) from which NEdit was started.\n\
\n\
tolower(string) -- Return an all lower-case version of string.\n\
\n\
toupper(string) -- Return an all upper-case version of string.\n\
\n\
write_file(string, filename) -- Writes a string (parameter 1) to a \
file named in parameter 2.  Returns 1 on successful write, or 0 if \
unsuccessful.",

"All of the editing capabilities of NEdit are represented as a special type of \
subroutine, called an action routine, which can be invoked from both \
macros and translation table entries (see \"Binding Keys to Actions\" \
in the X Resources section of the Help menu).\n\
\n\
\n\
Actions Representing Menu Commands:\n\
\n\
    File Menu		      Search Menu\n\
    ---------------------     -----------------------\n\
    new()		      find()\n\
    open()		      find_dialog()\n\
    open_dialog()	      find_again()\n\
    open_selected()	      find_selection()\n\
    close()		      replace()\n\
    save()		      replace_dialog()\n\
    save_as()		      replace_all()\n\
    save_as_dialog()	      replace_in_selection()\n\
    revert_to_saved()	      replace_again()\n\
    include_file()	      goto_line_number()\n\
    include_file_dialog()     goto_line_number_dialog()\n\
    load_tags_file()	      goto_selected()\n\
    load_tags_file_dialog()   mark()\n\
    load_macro_file()         mark_dialog()\n\
    load_macro_file_dialog()  goto_mark()\n\
    print()		      goto_mark_dialog()\n\
    print_selection()	      goto_matching()\n\
    unload_tags_file()        select_to_matching()\n\
    exit()		      find_definition()\n\
 			      split_window()\n\
    			      close_pane()\n\
    Edit Menu		      \n\
    ---------------------     Shell Menu\n\
    undo()		      -----------------------\n\
    redo()		      filter_selection_dialog()\n\
    delete_selection()        filter_selection()\n\
    select_all()	      execute_command()\n\
    shift_left()	      execute_command_dialog()\n\
    shift_left_by_tab()       execute_command_line()\n\
    shift_right()	      shell_menu_command()\n\
    shift_right_by_tab()      \n\
    uppercase() 	      Macro Menu\n\
    lowercase() 	      -----------------------\n\
    fill_paragraph()	      macro_menu_command()\n\
    control_code_dialog()     repeat_macro()\n\
   			      repeat_dialog()\n\
\n\
The actions representing menu commands are named the same as the menu item \
with punctuation removed, all lower case, and underscores \
replacing spaces.  Without the \
_dialog suffix, commands which normally prompt the user for information, \
instead take the information from the routine's arguments (see below).  To \
present a dialog and ask the user for input, rather than supplying it in via \
arguments, use the actions with the _dialog suffix.\n\
\n\
Menu Action Routine Arguments:\n\
\n\
Arguments are text strings enclosed in quotes.  Below are the menu action \
routines which take arguments.  Optional arguments are enclosed in [].\n\
\n\
  open(filename)\n\
\n\
  save_as(filename)\n\
\n\
  include(filename)\n\
\n\
  load_tags_file(filename)\n\
\n\
  unload_tags_file(filename)\n\
\n\
  find_dialog([search_direction] [, search-type]\n\
	  [, keep-dialog])\n\
\n\
  find(search_string [, search-direction] [, search-type]\n\
	  [, search-wrap])\n\
\n\
  find_again([search-direction] [, search-wrap])\n\
\n\
  find_selection([search-direction] [, search-wrap]\n\
	  [, non-regex-search-type])\n\
\n\
  replace_dialog([search-direction] [, search-type]\n\
	  [, keep-dialog])\n\
\n\
  replace(search-string, replace-string,\n\
	  [, search-direction] [, search-type]\n\
	  [, search-wrap])\n\
\n\
  replace_in_selection(search-string, replace-string\n\
	  [, search-type])\n\
\n\
  replace_again([search-direction] [, search-wrap])\n\
\n\
  goto_line_number([line-number])\n\
\n\
  mark(mark-letter)\n\
\n\
  goto_mark(mark-letter)\n\
\n\
  find_definition([tag-name])\n\
\n\
  filter_selection(shell-command)\n\
\n\
  execute_command(shell-command)\n\
\n\
  shell_menu_command(shell-menu-item-name)\n\
\n\
  macro_menu_command(macro-menu-item-name)\n\
\n\
Some notes on argument types above:\n\
\n\
  filename	    Path names are interpreted relative\n\
 		    to the directory from which NEdit was\n\
 		    started, wildcards and ~ are not\n\
 		    expanded.\n\
 		    \n\
  search-direction  Either \"forward\" or \"backward\"\n\
\n\
  search-type	    Either \"literal\", \"case\", or \"regex\"\n\
\n\
  non-regex-search-type	    Either \"literal\" or \"case\"\n\
\n\
  search-wrap	    Either \"wrap\" or \"nowrap\"\n\
\n\
  consider-case	 Either \"case\" or \"nocase\"\n\
\n\
  keep-dialog	    Either \"keep\" or \"nokeep\"\n\
\n\
  mark-letter	    The mark command limits users to\n\
 		    single letters.  Inside of macros,\n\
 		    numeric marks are allowed, which won't\n\
 		    interfere with marks set by the user.\n\
\n\
  (macro or shell)  Name of the command exactly as\n\
  -menu-item-name   specified in the Shell Menu or\n\
 		    Macro Menu dialogs\n\
\n\
\n\
Window Preferneces Actions\n\
\n\
set_auto_indent(\"off\" | \"on\" | \"smart\")\n\
Set auto indent mode for the current window.\n\
\n\
set_em_tab_dist(em-tab-distance)\n\
Set the emulated tab size. An em-tab-distance value of \
0 or -1 translates to no emulated tabs. Em-tab-distance must \
be smaller than 1000.\n\
\n\
set_fonts(font-name, italic-font-name, bold-font-name, bold-italic-font-name)\n\
Set all the fonts used for the current window.\n\
\n\
set_highlight_syntax([0 | 1])\n\
Set syntax highlighting mode for the current window. \
A value of 0 turns it off and a value of 1 turns it on. \
If no parameters are supplied the option is toggled.\n\
\n\
set_incremental_backup([0 | 1])\n\
Set incremental backup mode for the current window. \
A value of 0 turns it off and a value of 1 turns it on. \
If no parameters are supplied the option is toggled.\n\
\n\
set_incremental_search_line([0 | 1])\n\
Show or hide the incremental search line for the current window. \
A value of 0 turns it off and a value of 1 turns it on. \
If no parameters are supplied the option is toggled.\n\
\n\
set_language_mode(language-mode)\n\
Set the language mode for the current window. If the language mode is \
\"\" or unrecognized, it will be set to Plain.\n\
\n\
set_locked([0 | 1])\n\
This only affects the locked status of a file, not it's read-only \
status. Permissions are NOT changed. \
A value of 0 turns it off and a value of 1 turns it on. \
If no parameters are supplied the option is toggled.\n\
\n\
set_make_backup_copy([0 | 1])\n\
Set whether backup copies are made during saves for the current window. \
A value of 0 turns it off and a value of 1 turns it on. \
If no parameters are supplied the option is toggled.\n\
\n\
set_overtype_mode([0 | 1])\n\
Set overtype mode for the current window. \
A value of 0 turns it off and a value of 1 turns it on. \
If no parameters are supplied the option is toggled.\n\
\n\
set_show_line_numbers([0 | 1])\n\
Show or hide line numbers for the current window. \
A value of 0 turns it off and a value of 1 turns it on. \
If no parameters are supplied the option is toggled.\n\
\n\
set_show_matching([0 | 1])\n\
Set show matching (...) mode for the current window. \
A value of 0 turns it off and a value of 1 turns it on. \
If no parameters are supplied the option is toggled.\n\
\n\
set_statistics_line([0 | 1])\n\
Show or hide the statistics line for the current window. \
A value of 0 turns it off and a value of 1 turns it on. \
If no parameters are supplied the option is toggled.\n\
\n\
set_tab_dist(tab-distance)\n\
Set the size of hardware tab spacing. Tab-distance must \
must be a value greater than 0 and no greater than 20.\n\
\n\
set_use_tabs([0 | 1])\n\
Set whether tabs are used for the current window. \
A value of 0 turns it off and a value of 1 turns it on. \
If no parameters are supplied the option is toggled.\n\
\n\
set_wrap_margin(wrap-width)\n\
Set the wrap width for text wrapping of the current window. A value \
of 0 means to wrap at window width.\n\
\n\
set_wrap_text(\"none\" | \"auto\" | \"continuous\")\n\
Set wrap text mode for the current window.\n\
\n\
\n\
Keyboard-Only Actions\n\
\n\
backward_character([\"nobell\"])\n\
Moves the cursor one character to the left.\n\
\n\
backward_paragraph([\"nobell\"])\n\
Moves the cursor to the beginning of the paragraph, or if the \
cursor is already at the beginning of a paragraph, moves the cursor \
to the beginning of the previous paragraph.  Paragraphs are \
defined as regions of text delimited by one or more blank lines.\n\
\n\
backward_word([\"nobell\"])\n\
Moves the cursor to the beginning of a word, or, if the \
cursor is already at the beginning of a word, moves the \
cursor to the beginning of the previous word.  Word delimiters \
are user-settable, and defined by the X resource wordDelimiters.\n\
\n\
beginning_of_file([\"scrollbar\"])\n\
Moves the cursor to the beginning of the file.\n\
\n\
beginning_of_line()\n\
Moves the cursor to the beginning of the line.\n\
\n\
beginning_of_selection()\n\
Moves the cursor to the beginning of the selection \
without disturbing the selection.\n\
\n\
copy_clipboard()\n\
Copies the current selection to the clipboard.\n\
\n\
copy_primary()\n\
Copies the primary selection to the cursor.\n\
\n\
copy_to()\n\
If a secondary selection exists, copies the secondary selection to the \
cursor.  If no secondary selection exists, copies the primary \
selection to the \
pointer location.\n\
\n\
copy_to_or_end_drag()\n\
Completes either a secondary selection operation, or a primary \
drag.  If the user is dragging the mouse to adjust a secondary selection, \
the selection is copied and either inserted at the cursor location, \
or, if pending-delete is on and a primary selection exists in the window, \
replaces the primary selection.  If the user is dragging a block of \
text (primary selection), completes the drag operation and leaves the \
text at it's current location.\n\
\n\
cut_clipboard()\n\
Deletes the text in the primary selection and places it in the clipboard.\n\
\n\
cut_primary()\n\
Copies the primary selection to the cursor and deletes it \
at its original location.\n\
\n\
delete_selection()\n\
Deletes the contents of the primary selection.\n\
\n\
delete_next_character([\"nobell\"])\n\
If a primary selection exists, deletes its contents.  Otherwise, \
deletes the character following the cursor.\n\
\n\
delete_previous_character([\"nobell\"])\n\
If a primary selection exists, deletes its contents.  Otherwise, \
deletes the character before the cursor.\n\
\n\
delete_next_word([\"nobell\"])\n\
If a primary selection exists, deletes its contents.  Otherwise, \
deletes the word following the cursor.\n\
\n\
delete_previous_word([\"nobell\"])\n\
If a primary selection exists, deletes its contents.  Otherwise, \
deletes the word before the cursor.\n\
\n\
delete_to_start_of_line([\"nobell\"])\n\
If a primary selection exists, deletes its contents.  Otherwise, \
deletes the characters between the cursor \
and the start of the line.\n\
\n\
delete_to_end_of_line([\"nobell\"])\n\
If a primary selection exists, deletes its contents.  Otherwise, \
deletes the characters between the cursor \
and the end of the line.\n\
\n\
deselect_all()\n\
De-selects the primary selection.\n\
\n\
end_of_file([\"scrollbar\"])\n\
Moves the cursor to the end of the file.\n\
\n\
end_of_line()\n\
Moves the cursor to the end of the line.\n\
\n\
end_of_selection()\n\
Moves the cursor to the end of the selection \
without disturbing the selection.\n\
\n\
exchange([\"nobell\"])\n\
Exchange the primary and secondary selections.\n\
\n\
extend_adjust()\n\
Attached mouse-movement events to begin a selection between the \
cursor and the mouse, or extend the primary selection to the \
mouse position.\n\
\n\
extend_end()\n\
Completes a primary drag-selection operation.\n\
\n\
extend_start()\n\
Begins a selection between the \
cursor and the mouse.  A drag-selection operation can be started with \
either extend_start or grab_focus.\n\
\n\
focus_pane([relative-pane] | [positive-index] | [negative-index])\n\
Move the focus to the requested pane.\n\
Possible arguments are:\n\
  relative-pane   Either  \"first\", \"last\", \"next\", \"previous\"\n\
  positive-index are numbers greater than 0. 1 is the same as \"first\".\n\
  negative-index are numbers less than 0. -1 is the as \"last\".\n\
\n\
forward_character([\"nobell\"])\n\
Moves the cursor one character to the right.\n\
\n\
forward_paragraph([\"nobell\"])\n\
Moves the cursor to the beginning of the next paragraph.  Paragraphs are \
defined as regions of text delimited by one or more blank lines.\n\
\n\
forward_word([\"tail\"] [\"nobell\"])\n\
Moves the cursor to the beginning of the next word.  Word delimiters \
are user-settable, and defined by the X resource wordDelimiters.\
If the \"tail\" argument is supplied the cursor will be moved to \
the end of the current word or the end of the next word, if the \
cursor is between words.\n\
\n\
grab_focus()\n\
Moves the cursor to the mouse pointer location, and prepares for \
a possible drag-selection operation (bound to extend_adjust), or \
multi-click operation (a further grab_focus action).  If a second \
invocation of grab focus follows immediately, it selects a whole word, \
or a third, a whole line.\n\
\n\
insert_string(\"string\") \n\
If pending delete is on and the cursor is inside the selection, replaces \
the selection with \"string\".  Otherwise, inserts \"string\" at the \
cursor location.\n\
\n\
key_select(\"direction\" [, \"nobell\"])\n\
Moves the cursor one character in \
\"direction\" (\"left\", \"right\", \"up\", or \"down\") and extends the \
selection.  Same as forward/backward-character(\"extend\"), or \
process-up/down(\"extend\"), for compatibility with previous versions.\n\
\n\
move-destination()\n\
Moves the cursor to the pointer location without disturbing the selection.  \
(This is an unusual way of working.  We left it in for compatibility with \
previous versions, but if you actually use this capability, please send us some \
mail, otherwise it is likely to disappear in the future.\n\
\n\
move_to()\n\
If a secondary selection exists, deletes the contents of the secondary \
selection and inserts it at the cursor, or if pending-delete is on and there is \
a primary selection, replaces the primary selection.  If no secondary selection \
exists, moves the primary selection to the pointer location, deleting it \
from its original position.\n\
\n\
move_to_or_end_drag()\n\
Completes either a secondary selection operation, or a primary \
drag.  If the user is dragging the mouse to adjust a secondary selection, \
the selection is deleted and either inserted at the cursor location, \
or, if pending-delete is on and a primary selection exists in the window, \
replaces the primary selection.  If the user is dragging a block of \
text (primary selection), completes the drag operation and deletes the \
text from it's current location.\n\
\n\
newline()\n\
Inserts a newline character.  If Auto Indent is on, lines up the indentation \
of the cursor with the current line.\n\
\n\
newline_and_indent()\n\
Inserts a newline character and lines up the indentation \
of the cursor with the current line, regardless of the setting of Auto Indent.\n\
\n\
newline_no_indent()\n\
Inserts a newline character, without automatic indentation, regardless of \
the setting of Auto Indent.\n\
\n\
next_page([\"stutter\"] [\"column\"] [\"scrollbar\"] [\"nobell\"])\n\
Moves the cursor and scroll forward one page.\
The paramater \"stutter\" moves the cursor to the bottom of the display,\
unless it is already there, otherwise it will page down.\
The parameter \"column\" will maintain the preferred column while\
moving the cursor.\n\
\n\
page_left([\"scrollbar\"] [\"nobell\"])\n\
Move the cursor and scroll left one page.\n\
\n\
page_right([\"scrollbar\"] [\"nobell\"])\n\
Move the cursor and scroll right one page.\n\
\n\
paste_clipboard()\n\
Insert the contents of the clipboard at the cursor, or if pending delete \
is on, replace the primary selection with the contents of the clipboard.\n\
\n\
previous_page([\"stutter\"] [\"column\"] [\"scrollbar\"] [\"nobell\"])\n\
Moves the cursor and scroll backward one page.\
The paramater \"stutter\" moves the cursor to the top of the display,\
unless it is already there, otherwise it will page up.\
The parameter \"column\" will maintain the preferred column while\
moving the cursor.\n\
\n\
process_bdrag()\n\
Same as secondary_or_drag_start for compatibility with previous versions.\n\
\n\
process_cancel()\n\
Cancels the current extend_adjust, secondary_adjust, or \
secondary_or_drag_adjust in progress.\n\
\n\
process_down([\"nobell\"])\n\
Moves the cursor down one line.\n\
\n\
process_return()\n\
Same as newline for compatibility with previous versions.\n\
\n\
process_shift_down([\"nobell\"])\n\
Same as process_down(\"extend\") for compatibility with previous versions.\n\
\n\
process_shift_up([\"nobell\"])\n\
Same as process_up(\"extend\") for compatibility with previous versions.\n\
\n\
process_tab()\n\
If tab emulation is turned on, inserts an emulated tab, otherwise inserts \
a tab character.\n\
\n\
process_up([\"nobell\"])\n\
Moves the cursor up one line.\n\
\n\
raise_window([relative-window] | [positive-index] | [negative-index])\n\
Raise the current focused window to the front if no argument is supplied. \
Possible arguments are \n\
  relative-window   Either  \"first\", \"last\", \"next\", \"previous\"\n\
  positive-index are numbers greater than 0. 1 is the same as \"last\".\n\
  negative-index are numbers less than 0. -1 is the as \"first\".\n\
\n\
scroll_down(nLines)\n\
Scroll the display down (towards the end of the file) by nLines.\n\
\n\
scroll_left(nPixels)\n\
Scroll the display left by nPixels.\n\
\n\
scroll_right(nPixels)\n\
Scroll the display right by nPixels.\n\
\n\
scroll_up(nLines)\n\
Scroll the display up (towards the beginning of the file) by nLines.\n\
\n\
scroll_to_line(lineNum)\n\
Scroll to position line number lineNum at the top of the \
pane.  The first line of a file is line 1.\n\
\n\
secondary_adjust()\n\
Attached mouse-movement events to extend the secondary selection to the \
mouse position.\n\
\n\
secondary_or_drag_adjust()\n\
Attached mouse-movement events to extend the secondary selection, or \
reposition the primary text being dragged.  Takes two optional arguments, \
\"copy\", and \"overlay\".  \"copy\" leaves a copy of the dragged text \
at the site at which the drag began.  \"overlay\" does the drag in overlay \
mode, meaning the dragged text is laid on top of the existing text, \
obscuring and ultimately deleting it when the drag is complete.\n\
\n\
secondary_or_drag_start()\n\
To be attached to a mouse down event.  Begins drag selecting a secondary \
selection, or dragging the contents of the primary selection, depending on \
whether the mouse is pressed inside of an existing primary selection.\n\
\n\
secondary_start()\n\
To be attached to a mouse down event.  Begin drag selecting a secondary \
selection.\n\
\n\
select_all()\n\
Select the entire file.\n\
\n\
self_insert()\n\
To be attached to a key-press event, inserts the character equivalent \
of the key pressed.\n\
\n\
Arguments to Keyboard Action Routines \n\
\n\
In addition to the arguments listed in the call descriptions, any routine \
involving cursor movement can take the argument \"extend\", meaning, adjust \
the primary selection to the new cursor position.  Routines which take \
the \"extend\" argument as well as mouse dragging operations for both \
primary and secondary selections can take the optional keyword \"rect\", \
meaning, make the selection rectangular. Any routine that accepts the \
\"scrollbar\" argument will move the display but not the cursor or \
selection. Routines that accept the \"nobell\" argument will fail silently \
without beeping, if that argument is supplied.",

"WRITING SYNTAX HIGHLIGHTING PATTERNS\n\
\n\
Patterns are the mechanism by which syntax highlighting (see Syntax \
Highlighting under the heading of Features for Programming) \
is programmed in NEdit, that is, how it decides what to highlight in a given \
language.  \
To create syntax highlighting patterns for a new language, or to \
modify existing patterns, select \"Recognition Patterns\" from \
\"Syntax Highlighting\" sub-section of the \"Default Settings\" sub-menu \
of the \"Preferences\" menu.\n\
\n\
First, a word of caution.  As with regular expression matching in \
general, it is quite possible to write patterns which are so \
inefficient that they essentially lock up the editor as they \
recursively re-examine the entire contents of the file thousands of \
times.  With the multiplicity of patterns, the possibility of a lock-up \
is significantly increased in syntax highlighting.  When working \
on highlighting patterns, be sure to save your work frequently.\n\
\n\
NEdit's syntax highlighting is unusual in that it works in real-time \
(as you type), and yet is completely programmable using standard \
regular expression notation.  Other syntax highlighting editors usually fall \
either into the category of fully programmable but unable to keep up in \
real-time, or real-time but limited programmability.  The additional \
burden that NEdit places on pattern writers in order to achieve this \
speed/flexibility mix, is to force them to state self-imposed \
limitations on the amount of context that patterns may examine when \
re-parsing after a change.  While the \"Pattern Context Requirements\" \
heading is near the end of this section, it is not optional, and must be \
understood before making any any serious effort at pattern writing.\n\
\n\
In its simplest form, a highlight pattern consists of a regular \
expression to match, along with a style representing the font an color \
for displaying any text which matches that expression.  To bold the \
word, \"highlight\", wherever it appears the text, the regular expression \
simply would be the word \"highlight\".  The style (selected from the menu \
under the heading of \"Highlight Style\") determines how the text will be \
drawn.  To bold the text, either select an existing style, such as \
\"Keyword\", which bolds text, or create a new style and select it \
under Highlight Style.\n\
\n\
The full range of regular expression capabilities can be applied in \
such a pattern, with the single caveat that the expression must \
conclusively match or not match, within the pre-defined context \
distance (as discussed below under Pattern Context Requirements).\n\
\n\
To match longer ranges of text, particularly any constructs which \
exceed the requested context, you must use a pattern which highlights \
text between a starting and ending regular expression match.  To do so, \
select \"Highlight text between starting and ending REs\" under \
\"Matching\", and enter both a starting and ending regular expression.  \
For example, to highlight everything between double quotes, you would \
enter a double quote character in both the starting and ending regular \
expression fields.  Patterns with both a beginning and ending expression span \
all characters between the two expressions, including newlines.\n\
\n\
Again, the limitation for automatic parsing to operate properly is that \
both expressions must match within the context distance stated for the \
pattern set.\n\
\n\
With the ability to span large distances, comes the responsibility to \
recover when things go wrong.  Remember that syntax highlighting is \
called upon to parse incorrect or incomplete syntax as often as correct \
syntax.  To stop a pattern short of matching its end expression, you can \
specify an error expression, which stops the pattern from gobbling up \
more than it should.  For example, if the text between double quotes \
shouldn't contain newlines, the error expression might be \"$\".  As with \
both starting and ending expressions, error expressions must also match \
within the requested context distance.\n\
\n\
Coloring Sub-Expressions\n\
\n\
It is also possible to color areas of text within a regular \
expression match.  A pattern of this type associates a style with \
sub-expressions references of the parent pattern (as used in regular \
expression substitution patterns, see the NEdit Help menu item on \
Regular Expressions).  Sub-expressions of both the starting and ending \
patterns may be colored.  For example, if the parent pattern has a \
starting expression \"\\<\", and end expression \"\\>\", (for highlighting \
all of the text contained within angle brackets), a sub-pattern using \
\"&\" in both the starting and ending expression fields could color the \
brackets differently from the intervening text.  A quick shortcut to \
typing in pattern names in the Parent Pattern field is to use the \
middle mouse button to drag them from the Patterns list.\n\
\n\
Hierarchical Patterns\n\
\n\
A hierarchical sub-pattern, is identical to a top level pattern, but is \
invoked only between the beginning and ending expression matches of its \
parent pattern.  Like the sub-expression coloring patterns discussed \
above, it is associated with a parent pattern using the Parent Pattern \
field in the pattern specification.  Pattern names can be dragged from \
the pattern list with the middle mouse button to the Parent Pattern \
field.\n\
\n\
After the start expression of the parent pattern matches, the syntax \
highlighting parser searches for either the parent's end pattern or a \
matching sub-pattern.  When a sub-pattern matches, control is not \
returned to the parent pattern until the entire sub-pattern has been \
parsed, regardless of whether the parent's end pattern appears in the \
text matched by the sub-pattern.\n\
\n\
The most common use for this capability is for coloring sub-structure \
of language constructs (smaller patterns embedded in larger patterns).  \
Hierarchical patterns can also simplify parsing by having sub-patterns \
\"hide\" special syntax from parent patterns, such as special escape \
sequences or internal comments.\n\
\n\
There is no depth limit in nesting hierarchical sub-patterns, but \
beyond the third level of nesting, automatic re-parsing will sometimes \
have to re-parse more than the requested context distance to guarantee \
a correct parse (which can slow down the maximum rate at which the user \
can type if large sections of text are matched only by deeply nested \
patterns).\n\
\n\
While this is obviously not a complete hierarchical language parser it \
is still useful in many text coloring situations.  As a pattern writer, \
your goal is not to completely cover the language syntax, but to \
generate colorings that are useful to the programmer.  Simpler patterns \
are usually more efficient and also more robust when applied to \
incorrect code.\n\
\n\
Deferred (Pass-2) Parsing\n\
\n\
NEdit does pattern matching for syntax highlighting in two passes.  The \
first pass is applied to the entire file when syntax highlighting is \
first turned on, and to new ranges of text when they are initially read \
or pasted in.  The second pass is applied only as needed when text is \
exposed (scrolled in to view).\n\
\n\
If you have a particularly complex set of patterns, and parsing is \
beginning to add a noticeable delay to opening files or operations \
which change large regions of text, you can defer some of that parsing \
from startup time, to when it is actually needed for viewing the text.  \
Deferred parsing can only be used with single expression patterns, or \
begin/end patterns which match entirely within the requested context \
distance.  To defer the parsing of a pattern to when \
the text is exposed, click on the Pass-2 pattern type button in the \
highlight patterns dialog.\n\
\n\
Sometimes a pattern can't be deferred, not because of context \
requirements, but because it must run concurrently with pass-1 \
(non-deferred) patterns.  If they didn't run concurrently, a pass-1 \
pattern might incorrectly match some of the characters which would \
normally be hidden inside of a sequence matched by the deferred \
pattern.  For example, C has character constants enclosed in single \
quotes.  These typically do not cross line boundaries, meaning they can \
be parsed entirely within the context distance of the C pattern set and \
should be good candidates for deferred parsing.  However, they can't be \
deferred because they can contain sequences of characters which can \
trigger pass-one patterns. Specifically, the sequence, '\\\"', contains \
a double quote character, which would be matched by the string pattern \
and interpreted as introducing a string.\n\
\n\
Pattern Context Requirements\n\
\n\
The context requirements of a pattern set state how much additional \
text around any change must be examined to guarantee that \
the patterns will match what they are intended to match.  Context \
requirements are a promise by NEdit to the pattern writer, that the \
regular expressions in his/her patterns will be matched against at \
least <line context> lines and <character context> characters, around \
any modified text.  Combining line and character requirements guarantee \
that both will be met.\n\
\n\
Automatic re-parsing happens on EVERY KEYSTROKE, so the amount of \
context which must be examined is very critical to typing efficiency.  \
The more complicated your patterns, the more critical the context \
becomes.  To cover all of the keywords in a typical language, without \
affecting the maximum rate at which users can enter text, you may be \
limited to just a few lines and/or a few hundred characters of context.\n\
\n\
The default context distance is 1 line, with no minimum character \
requirement.  There are several benefits to sticking with this \
default.  One is simply that it is easy to understand and to comply \
with.  Regular expression notation is designed around single line \
matching.  To span lines in a regular expression, you must explicitly \
mention the newline character \"\\n\", and matches which are restricted to \
a single line are virtually immune to lock-ups.  Also, if you can code \
your patterns to work within a single line of context, without an \
additional character-range context requirement, the parser can take \
advantage the fact that patterns don't cross line boundaries, and \
nearly double its efficiency over a one-line and 1-character context \
requirement.  (In a single line context, you are allowed to match \
newlines, but only as the first and/or last character.)",

"Smart indent macros can be written for any language, but are usually \
more difficult to write than highlighting patterns.  A good place to start, \
of course, is to look at the existing macros for C and C++.\n\
\n\
Smart indent macros for a language mode consist of standard NEdit macro \
language code attached to any or all of the following three activation \
conditions: \
1) When smart indent is first turned on for a text window containing code \
of the language, 2) When a newline is typed and smart indent is expected, 3) \
after any character is typed.  To attach macro code to any of these code \
\"hooks\", enter it in the appropriate section in the Preferences -> \
Default Settings -> Auto Indent -> Program Smart Indent dialog.\n\
\n\
Typically most of the code should go in the initialization section, because \
that is the appropriate place for subroutine definitions, and smart indent \
macros are complicated enough that you are not likely to want to write them \
as one monolithic run of code.  You may also put code in the Common/Shared \
Initialization section (accessible through the button in the upper left \
corner of the dialog).  Unfortunately, since the C/C++ macros also reside \
in the common/shared section, when you add code there, you run some risk of \
missing out on future \
upgrades to these macros, because your changes will override the built-in \
defaults.\n\
\n\
The newline macro is invoked after the user types a newline, but before the \
newline is entered in the buffer.  It takes a single argument ($1) which is \
the position at which the newline will be inserted.  It must return the \
number of characters of indentation the line should have, or -1.  A \
return value of -1 means to do a standard auto-indent.  You must supply \
a newline macro, but the code: \"return -1\" (auto-indent), or \"return 0\" \
(no indent) is sufficient.\n\
\n\
The type-in macro takes two arguments.  $1 is the insert position, and $2 is \
the character just inserted, and does not return a value.  You can do just \
about anything here, but keep in mind that this macro is executed for every \
keystroke typed, so if you try to get too fancy, you may degrade performance.",

"SOLUTIONS TO COMMON PROBLEMS\n\
\n\
For a much more comprehensive list of common problems and solutions, \
see the NEdit FAQ.  The latest \
version of the FAQ can always be found on the NEdit web site at:\n\
\n\
    http://nedit.org.\n\
\n\
P: No files are shown in the \"Files\" list in the Open... dialog.\n\
S: When you use the \"Filter\" field, include the file specification or a \
complete directory specification, including the trailing \"/\" on Unix.  \
(See Help in the Open... dialog).\n\
\n\
P: Find Again and Replace Again don't continue in the same direction \
as the original Find or Replace.\n\
S: Find Again and Replace Again don't use the direction of the original \
search.  The Shift key controls the direction: Ctrl+G means \
forward, Shift+Ctrl+G means backward.\n\
\n\
P: Preferences specified in the Preferences menu don't seem to get saved \
when I select Save Defaults.\n\
S: NEdit has two kinds of preferences: 1) per-window preferences, in the \
Preferences menu, and 2) default settings for preferences in newly created \
windows, in the Default \
Settings sub-menu of the Preferences menu.  Per-window preferences are not \
saved by Save Defaults, only Default Settings.\n\
\n\
P: Columns and indentation don't line up.\n\
S: NEdit is using a proportional width font.  Set the font to a fixed style \
(see Preferences).\n\
\n\
P: NEdit performs poorly on very large files.\n\
S: Turn off Incremental Backup.  With Incremental Backup on, NEdit \
periodically writes a full copy of the file to disk.\n\
\n\
P: Commands added to the Shell Commands menu (Unix only) don't output \
anything until they are finished executing.\n\
S: If the command output is \
directed to a dialog, or the input is from a selection, output is collected \
together and held until the command completes.  De-select both of the options \
and the output will be shown incrementally as the command executes.\n\
\n\
P: Dialogs don't automatically get keyboard focus when they pop up.\n\
S: Most X Window managers allow you to choose between two categories of \
keyboard focus models: pointer focus, and explicit focus.  \
Pointer focus means that as you move the mouse around the screen, the window \
under the mouse automatically gets the keyboard focus.  NEdit users who use \
this focus model should set \"Popups Under Pointer\" in the Default Settings \
sub menu of the preferences menu in NEdit.  Users with the explicit \
focus model, in some cases, may have problems with certain dialogs, such as \
Find and Replace.  In MWM this is caused by the mwm resource startupKeyFocus \
being set to False (generally a bad choice for explicit focus users).  \
NCDwm users should use the focus model \"click\" \
instead of \"explicit\", again, unless you have set it that way to correct \
specific problems, this is the appropriate setting for most \
explicit focus users.\n\
\n\
P: The Backspace key doesn't work, or deletes forward rather than backward.\n\
S: While this is an X/Motif binding problem, and should be solved outside \
of NEdit in the Motif virtual binding layer (or possibly xmodmap or \
translations), NEdit provides an out.  If you set the resource: \
nedit.remapDeleteKey to True, NEdit will forcibly map the delete key \
to backspace.  The default setting of this resource recently changed, so \
users who have been depending on this remapping will now have to set \
it explicitly (or fix their bindings).\n\
\n\
P: NEdit crashes when I try to paste text in to a text field in a dialog \
(like Find or Replace) on my SunOS system.\n\
S: On many SunOS systems, you have to set up an nls directory before \
various inter-client communication features of Motif will function \
properly.  There are instructions in README.sun in \
/pub/v5_0_2/individual/README.sun on \
ftp.nedit.org, as well as a tar file containg a complete nls \
directory: ftp://ftp.nedit.org/pub/v5_0_2/nls.tar. \
README.sun contains directions for setting up an nls directory, which is \
required by Motif for handling copy and paste to Motif text fields. \
\n\
\n\
KNOWN BUGS\n\
\n\
Below is the list of known bugs which affect NEdit.  The bugs your copy \
of NEdit will exhibit depend on which system you are running and with which \
Motif libraries it was built. Note that there are now Motif 1.2 and/or 2.0 \
libraries available on ALL supported platforms, and as you can see below \
there are far fewer bugs in Motif 1.2, so it is in your best interest to \
upgrade your system.\n\
\n\
All Versions\n\
\n\
BUG: Operations between rectangular selections on overlapping lines \
do nothing.\n\
WORKAROUND: None.  These operations are \
very complicated and rarely used.\n\
\n\
BUG: Cut and Paste menu items fail, or possibly crash, for very large \
(multi-megabyte) selections.\n\
WORKAROUND: Use selection copy (middle mouse button click) for transferring \
larger quantities of data.  Cut and Paste save the copied text in server \
memory, which is usually limited.\n\
\n\
\n\
REPORTING BUGS\n\
\n\
The NEdit developers subscribe to both discuss@nedit.org and \
develop@nedit.org, either of which may be used for reporting bugs.  If \
you're not sure, or you think the report might be of interest to the \
general NEdit user community, send the report to discuss@nedit.org.  If \
it's something obvious and boring, like we misspelled \"anemometer\" in \
the on-line help, send it to develop.  If you don't want to subscribe to \
these lists, please add a note to your mail about cc'ing you on \
responses.",

"There are two separate mailing lists for NEdit users, and one for developers. \
Users may post to the developer mailing list to report bugs and communicate \
with the NEdit developers.  Remember that NEdit is entirely a volunteer effort, \
so please ask questions first to the discussion list, and do your share to \
answer other users questions as well.\n\
\n\
  discuss@nedit.org  -- General discussion, questions and\n\
    	    	    	answers among NEdit users and\n\
			developers.\n\
\n\
  announce@nedit.org -- A low-volume mailing list for\n\
                        announcing new versions.\n\
\n\
  develop@nedit.org  -- Communication among and with NEdit\n\
                        developers.  Developers should also\n\
			subscribe to the discuss list.\n\
\n\
To subscribe, send mail to <majordomo@nedit.org> with one or more of the \
following in the body of the message:\n\
\n\
    subscribe announce\n\
    subscribe discuss\n\
    subscribe develop",

"GNU GENERAL PUBLIC LICENSE\n\
\n\
Version 2, June 1991\n\
\n\
Copyright (C) 1989, 1991 Free Software Foundation, Inc. 675 Mass Ave, \
Cambridge, MA 02139, USA. Everyone is permitted to copy and distribute \
verbatim copies of this license document, but changing it is not \
allowed.\n\
\n\
Preamble\n\
\n\
The licenses for most software are designed to take away your freedom \
to share and change it. By contrast, the GNU General Public License is \
intended to guarantee your freedom to share and change free \
software--to make sure the software is free for all its users. This \
General Public License applies to most of the Free Software \
Foundation's software and to any other program whose authors commit to \
using it. (Some other Free Software Foundation software is covered by \
the GNU Library General Public License instead.) You can apply it to \
your programs, too.\n\
\n\
When we speak of free software, we are referring to freedom, not \
price. Our General Public Licenses are designed to make sure that you \
have the freedom to distribute copies of free software (and charge for \
this service if you wish), that you receive source code or can get it \
if you want it, that you can change the software or use pieces of it \
in new free programs; and that you know you can do these things.\n\
\n\
To protect your rights, we need to make restrictions that forbid \
anyone to deny you these rights or to ask you to surrender the rights. \
These restrictions translate to certain responsibilities for you if \
you distribute copies of the software, or if you modify it.\n\
\n\
For example, if you distribute copies of such a program, whether \
gratis or for a fee, you must give the recipients all the rights that \
you have. You must make sure that they, too, receive or can get the \
source code. And you must show them these terms so they know their \
rights.\n\
\n\
We protect your rights with two steps: (1) copyright the software, and \
(2) offer you this license which gives you legal permission to copy, \
distribute and/or modify the software.\n\
\n\
Also, for each author's protection and ours, we want to make certain \
that everyone understands that there is no warranty for this free \
software. If the software is modified by someone else and passed on, \
we want its recipients to know that what they have is not the \
original, so that any problems introduced by others will not reflect \
on the original authors' reputations.\n\
\n\
Finally, any free program is threatened constantly by software \
patents. We wish to avoid the danger that redistributors of a free \
program will individually obtain patent licenses, in effect making the \
program proprietary. To prevent this, we have made it clear that any \
patent must be licensed for everyone's free use or not licensed at \
all.\n\
\n\
The precise terms and conditions for copying, distribution and \
modification follow.\n\
\n\
GNU GENERAL PUBLIC LICENSE TERMS AND CONDITIONS FOR COPYING, \
DISTRIBUTION AND MODIFICATION\n\
\n\
0. This License applies to any program or other work which contains a \
notice placed by the copyright holder saying it may be distributed \
under the terms of this General Public License. The \"Program\", below, \
refers to any such program or work, and a \"work based on the Program\" \
means either the Program or any derivative work under copyright law: \
that is to say, a work containing the Program or a portion of it, \
either verbatim or with modifications and/or translated into another \
language. (Hereinafter, translation is included without limitation in \
the term \"modification\".) Each licensee is addressed as \"you\".\n\
\n\
Activities other than copying, distribution and modification are not \
covered by this License; they are outside its scope. The act of \
running the Program is not restricted, and the output from the Program \
is covered only if its contents constitute a work based on the Program \
(independent of having been made by running the Program). Whether that \
is true depends on what the Program does.\n\
\n\
1. You may copy and distribute verbatim copies of the Program's source \
code as you receive it, in any medium, provided that you conspicuously \
and appropriately publish on each copy an appropriate copyright notice \
and disclaimer of warranty; keep intact all the notices that refer to \
this License and to the absence of any warranty; and give any other \
recipients of the Program a copy of this License along with the \
Program.\n\
\n\
You may charge a fee for the physical act of transferring a copy, and \
you may at your option offer warranty protection in exchange for a \
fee.\n\
\n\
2. You may modify your copy or copies of the Program or any portion of \
it, thus forming a work based on the Program, and copy and distribute \
such modifications or work under the terms of Section 1 above, \
provided that you also meet all of these conditions:\n\
\n\
a) You must cause the modified files to carry prominent notices \
stating that you changed the files and the date of any change.\n\
\n\
b) You must cause any work that you distribute or publish, that in \
whole or in part contains or is derived from the Program or any part \
thereof, to be licensed as a whole at no charge to all third parties \
under the terms of this License.\n\
\n\
c) If the modified program normally reads commands interactively when \
run, you must cause it, when started running for such interactive use \
in the most ordinary way, to print or display an announcement \
including an appropriate copyright notice and a notice that there is \
no warranty (or else, saying that you provide a warranty) and that \
users may redistribute the program under these conditions, and telling \
the user how to view a copy of this License. (Exception: if the \
Program itself is interactive but does not normally print such an \
announcement, your work based on the Program is not required to print \
an announcement.)\n\
\n\
These requirements apply to the modified work as a whole. If \
identifiable sections of that work are not derived from the Program, \
and can be reasonably considered independent and separate works in \
themselves, then this License, and its terms, do not apply to those \
sections when you distribute them as separate works. But when you \
distribute the same sections as part of a whole which is a work based \
on the Program, the distribution of the whole must be on the terms of \
this License, whose permissions for other licensees extend to the \
entire whole, and thus to each and every part regardless of who wrote \
it.\n\
\n\
Thus, it is not the intent of this section to claim rights or contest \
your rights to work written entirely by you; rather, the intent is to \
exercise the right to control the distribution of derivative or \
collective works based on the Program.\n\
\n\
In addition, mere aggregation of another work not based on the Program \
with the Program (or with a work based on the Program) on a volume of \
a storage or distribution medium does not bring the other work under \
the scope of this License.\n\
\n\
3. You may copy and distribute the Program (or a work based on it, \
under Section 2) in object code or executable form under the terms of \
Sections 1 and 2 above provided that you also do one of the following:\n\
\n\
a) Accompany it with the complete corresponding machine-readable \
source code, which must be distributed under the terms of Sections 1 \
and 2 above on a medium customarily used for software interchange; or,\n\
\n\
b) Accompany it with a written offer, valid for at least three years, \
to give any third party, for a charge no more than your cost of \
physically performing source distribution, a complete machine-readable \
copy of the corresponding source code, to be distributed under the \
terms of Sections 1 and 2 above on a medium customarily used for \
software interchange; or,\n\
\n\
c) Accompany it with the information you received as to the offer to \
distribute corresponding source code. (This alternative is allowed \
only for noncommercial distribution and only if you received the \
program in object code or executable form with such an offer, in \
accord with Subsection b above.)\n\
\n\
The source code for a work means the preferred form of the work for \
making modifications to it. For an executable work, complete source \
code means all the source code for all modules it contains, plus any \
associated interface definition files, plus the scripts used to \
control compilation and installation of the executable. However, as a \
special exception, the source code distributed need not include \
anything that is normally distributed (in either source or binary \
form) with the major components (compiler, kernel, and so on) of the \
operating system on which the executable runs, unless that component \
itself accompanies the executable.\n\
\n\
If distribution of executable or object code is made by offering \
access to copy from a designated place, then offering equivalent \
access to copy the source code from the same place counts as \
distribution of the source code, even though third parties are not \
compelled to copy the source along with the object code.\n\
\n\
4. You may not copy, modify, sublicense, or distribute the Program \
except as expressly provided under this License. Any attempt otherwise \
to copy, modify, sublicense or distribute the Program is void, and \
will automatically terminate your rights under this License. However, \
parties who have received copies, or rights, from you under this \
License will not have their licenses terminated so long as such \
parties remain in full compliance.\n\
\n\
5. You are not required to accept this License, since you have not \
signed it. However, nothing else grants you permission to modify or \
distribute the Program or its derivative works. These actions are \
prohibited by law if you do not accept this License. Therefore, by \
modifying or distributing the Program (or any work based on the \
Program), you indicate your acceptance of this License to do so, and \
all its terms and conditions for copying, distributing or modifying \
the Program or works based on it.\n\
\n\
6. Each time you redistribute the Program (or any work based on the \
Program), the recipient automatically receives a license from the \
original licensor to copy, distribute or modify the Program subject to \
these terms and conditions. You may not impose any further \
restrictions on the recipients' exercise of the rights granted herein. \
You are not responsible for enforcing compliance by third parties to \
this License.\n\
\n\
7. If, as a consequence of a court judgment or allegation of patent \
infringement or for any other reason (not limited to patent issues), \
conditions are imposed on you (whether by court order, agreement or \
otherwise) that contradict the conditions of this License, they do not \
excuse you from the conditions of this License. If you cannot \
distribute so as to satisfy simultaneously your obligations under this \
License and any other pertinent obligations, then as a consequence you \
may not distribute the Program at all. For example, if a patent \
license would not permit royalty-free redistribution of the Program by \
all those who receive copies directly or indirectly through you, then \
the only way you could satisfy both it and this License would be to \
refrain entirely from distribution of the Program.\n\
\n\
If any portion of this section is held invalid or unenforceable under \
any particular circumstance, the balance of the section is intended to \
apply and the section as a whole is intended to apply in other \
circumstances.\n\
\n\
It is not the purpose of this section to induce you to infringe any \
patents or other property right claims or to contest validity of any \
such claims; this section has the sole purpose of protecting the \
integrity of the free software distribution system, which is \
implemented by public license practices. Many people have made \
generous contributions to the wide range of software distributed \
through that system in reliance on consistent application of that \
system; it is up to the author/donor to decide if he or she is willing \
to distribute software through any other system and a licensee cannot \
impose that choice.\n\
\n\
This section is intended to make thoroughly clear what is believed to \
be a consequence of the rest of this License.\n\
\n\
8. If the distribution and/or use of the Program is restricted in \
certain countries either by patents or by copyrighted interfaces, the \
original copyright holder who places the Program under this License \
may add an explicit geographical distribution limitation excluding \
those countries, so that distribution is permitted only in or among \
countries not thus excluded. In such case, this License incorporates \
the limitation as if written in the body of this License.\n\
\n\
9. The Free Software Foundation may publish revised and/or new \
versions of the General Public License from time to time. Such new \
versions will be similar in spirit to the present version, but may \
differ in detail to address new problems or concerns.\n\
\n\
Each version is given a distinguishing version number. If the Program \
specifies a version number of this License which applies to it and \
\"any later version\", you have the option of following the terms and \
conditions either of that version or of any later version published by \
the Free Software Foundation. If the Program does not specify a \
version number of this License, you may choose any version ever \
published by the Free Software Foundation.\n\
\n\
10. If you wish to incorporate parts of the Program into other free \
programs whose distribution conditions are different, write to the \
author to ask for permission. For software which is copyrighted by the \
Free Software Foundation, write to the Free Software Foundation; we \
sometimes make exceptions for this. Our decision will be guided by the \
two goals of preserving the free status of all derivatives of our free \
software and of promoting the sharing and reuse of software generally.\n\
\n\
NO WARRANTY\n\
\n\
11. BECAUSE THE PROGRAM IS LICENSED FREE OF CHARGE, THERE IS NO \
WARRANTY FOR THE PROGRAM, TO THE EXTENT PERMITTED BY APPLICABLE LAW. \
EXCEPT WHEN OTHERWISE STATED IN WRITING THE COPYRIGHT HOLDERS AND/OR \
OTHER PARTIES PROVIDE THE PROGRAM \"AS IS\" WITHOUT WARRANTY OF ANY \
KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING, BUT NOT LIMITED TO, THE \
IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR \
PURPOSE. THE ENTIRE RISK AS TO THE QUALITY AND PERFORMANCE OF THE \
PROGRAM IS WITH YOU. SHOULD THE PROGRAM PROVE DEFECTIVE, YOU ASSUME \
THE COST OF ALL NECESSARY SERVICING, REPAIR OR CORRECTION.\n\
\n\
12. IN NO EVENT UNLESS REQUIRED BY APPLICABLE LAW OR AGREED TO IN \
WRITING WILL ANY COPYRIGHT HOLDER, OR ANY OTHER PARTY WHO MAY MODIFY \
AND/OR REDISTRIBUTE THE PROGRAM AS PERMITTED ABOVE, BE LIABLE TO YOU \
FOR DAMAGES, INCLUDING ANY GENERAL, SPECIAL, INCIDENTAL OR \
CONSEQUENTIAL DAMAGES ARISING OUT OF THE USE OR INABILITY TO USE THE \
PROGRAM (INCLUDING BUT NOT LIMITED TO LOSS OF DATA OR DATA BEING \
RENDERED INACCURATE OR LOSSES SUSTAINED BY YOU OR THIRD PARTIES OR A \
FAILURE OF THE PROGRAM TO OPERATE WITH ANY OTHER PROGRAMS), EVEN IF \
SUCH HOLDER OR OTHER PARTY HAS BEEN ADVISED OF THE POSSIBILITY OF SUCH \
DAMAGES.\n\
\n\
END OF TERMS AND CONDITIONS",

"The Tabs dialog controls both the operation of the Tab \
key, and the interpretation of tab characters \
within a file.\n\
\n\
The first field, Tab Spacing, controls how  NEdit \
responds to tab characters in a file.  On most Unix and \
VMS systems the conventional interpretation of a tab \
character is to advance the text position to the nearest \
multiple of eight characters (a tab spacing of 8).  \
However, many programmers of C and other structured \
languages, when given the choice, prefer a tab spacing \
of 3 or 4 characters.  Setting a three or four character \
hardware tab spacing is useful and convenient as long as \
your other software tools support it.  Unfortunately, on \
Unix and VMS systems, system utilities, such as more, \
and printing software can't always properly display \
files with other than eight character tabs.\n\
\n\
Selecting \"Emulate Tabs\" will cause the Tab key to \
insert the correct number of spaces or tabs to reach the \
next tab stop, as if the tab spacing were set at the \
value in the \"Emulated tab spacing\" field.  \
Backspacing immediately after entering an emulated tab \
will delete it as a unit, but as soon as you move the \
cursor away from the spot, NEdit will forget that the \
collection of spaces and tabs is a tab, and will treat \
it as separate characters.  To enter a real tab \
character with \"Emulate Tabs\" turned on, use Ctrl+Tab.\n\
\n\
In generating emulated tabs, and in Shift Left, Paste \
Column, and some rectangular selection operations, NEdit \
inserts blank characters (spaces or tabs) to preserve \
the alignment of non-blank characters.  The bottom \
toggle button in the Tabs dialog instructs NEdit whether \
to insert tab characters as padding in such situations.  \
Turning this off, will keep NEdit from automatically \
inserting tabs.  Some software developers prefer to keep \
their source code free of tabs to avoid its \
misinterpretation on systems with different tab \
character conventions."
};

static Widget HelpWindows[NUM_TOPICS] = {NULL}; 
static Widget HelpTextPanes[NUM_TOPICS] = {NULL};

/* Information on the last search for search-again */
static char LastSearchString[DF_MAX_PROMPT_LENGTH] = "";
static int LastSearchTopic = -1;
static int LastSearchPos = 0;
static int LastSearchWasAllTopics = False;

static Widget createHelpPanel(Widget parent, int topic);
static void dismissCB(Widget w, XtPointer clientData, XtPointer callData);
static void searchHelpCB(Widget w, XtPointer clientData, XtPointer callData);
static void searchHelpAgainCB(Widget w, XtPointer clientData,
	XtPointer callData);
static void printCB(Widget w, XtPointer clientData, XtPointer callData);
static void searchHelpText(Widget parent, int parentTopic, char *searchFor,
	int allSections, int startPos, int startTopic);
static int findTopicFromShellWidget(Widget shellWidget);

void Help(Widget parent, enum HelpTopic topic)
{
    if (HelpWindows[topic] != NULL)
    	RaiseShellWindow(HelpWindows[topic]);
    else
    	HelpWindows[topic] = createHelpPanel(parent, topic);
}

static Widget createHelpPanel(Widget parent, int topic)
{
    Arg al[50];
    int ac;
    Widget appShell, form, btn, dismissBtn;
    Widget sw, hScrollBar, vScrollBar;
    XmString st1;
    
    ac = 0;
    XtSetArg(al[ac], XmNtitle, HelpTitles[topic]); ac++;
    XtSetArg(al[ac], XmNdeleteResponse, XmDO_NOTHING); ac++;
    XtSetArg(al[ac], XmNiconName, HelpTitles[topic]); ac++;
    appShell = CreateShellWithBestVis(APP_NAME, APP_CLASS,
	    applicationShellWidgetClass, TheDisplay, al, ac);
    AddSmallIcon(appShell);
    form = XtVaCreateManagedWidget("helpForm", xmFormWidgetClass, appShell, NULL);
    XtVaSetValues(form, XmNshadowThickness, 0, NULL);
    
    btn = XtVaCreateManagedWidget("find", xmPushButtonWidgetClass, form,
    	    XmNlabelString, st1=XmStringCreateSimple("Find..."),
    	    XmNmnemonic, 'F',
    	    XmNhighlightThickness, 0,
    	    XmNbottomAttachment, XmATTACH_FORM,
	    XmNleftAttachment, XmATTACH_POSITION,
	    XmNleftPosition, 3,
	    XmNrightAttachment, XmATTACH_POSITION,
	    XmNrightPosition, 25, NULL);
    XtAddCallback(btn, XmNactivateCallback, searchHelpCB, appShell);
    XmStringFree(st1);

    btn = XtVaCreateManagedWidget("findAgain", xmPushButtonWidgetClass, form,
    	    XmNlabelString, st1=XmStringCreateSimple("Find Again"),
    	    XmNmnemonic, 'A',
    	    XmNhighlightThickness, 0,
    	    XmNbottomAttachment, XmATTACH_FORM,
	    XmNleftAttachment, XmATTACH_POSITION,
	    XmNleftPosition, 27,
	    XmNrightAttachment, XmATTACH_POSITION,
	    XmNrightPosition, 49, NULL);
    XtAddCallback(btn, XmNactivateCallback, searchHelpAgainCB, appShell);
    XmStringFree(st1);

    btn = XtVaCreateManagedWidget("print", xmPushButtonWidgetClass, form,
    	    XmNlabelString, st1=XmStringCreateSimple("Print..."),
    	    XmNmnemonic, 'P',
    	    XmNhighlightThickness, 0,
    	    XmNbottomAttachment, XmATTACH_FORM,
	    XmNleftAttachment, XmATTACH_POSITION,
	    XmNleftPosition, 51,
	    XmNrightAttachment, XmATTACH_POSITION,
	    XmNrightPosition, 73, NULL);
    XtAddCallback(btn, XmNactivateCallback, printCB, appShell);
    XmStringFree(st1);

    dismissBtn = XtVaCreateManagedWidget("dismiss", xmPushButtonWidgetClass,
	    form, XmNlabelString, st1=XmStringCreateSimple("Dismiss"),
    	    XmNhighlightThickness, 0,
    	    XmNbottomAttachment, XmATTACH_FORM,
	    XmNleftAttachment, XmATTACH_POSITION,
	    XmNleftPosition, 75,
	    XmNrightAttachment, XmATTACH_POSITION,
	    XmNrightPosition, 97, NULL);
    XtAddCallback(dismissBtn, XmNactivateCallback, dismissCB, appShell);
    XmStringFree(st1);
            
    /* Create a text widget inside of a scrolled window widget */
    sw = XtVaCreateManagedWidget("sw", xmScrolledWindowWidgetClass,
    	    form, XmNspacing, 0, XmNhighlightThickness, 0,
	    XmNshadowThickness, 2,
	    XmNtopAttachment, XmATTACH_FORM,
	    XmNleftAttachment, XmATTACH_FORM,
    	    XmNrightAttachment, XmATTACH_FORM,
	    XmNbottomAttachment, XmATTACH_WIDGET,
	    XmNbottomWidget, dismissBtn, NULL);
    hScrollBar = XtVaCreateManagedWidget("hScrollBar",
    	    xmScrollBarWidgetClass, sw, XmNorientation, XmHORIZONTAL, 
    	    XmNrepeatDelay, 10, NULL);
    vScrollBar = XtVaCreateManagedWidget("vScrollBar",
    	    xmScrollBarWidgetClass, sw, XmNorientation, XmVERTICAL,
    	    XmNrepeatDelay, 10, NULL);
    HelpTextPanes[topic] = XtVaCreateManagedWidget("helpText",
	    textWidgetClass, sw, textNrows, 30, textNcolumns, 60,
    	    textNhScrollBar, hScrollBar, textNvScrollBar, vScrollBar,
	    textNreadOnly, True, textNcontinuousWrap, True,
	    textNautoShowInsertPos, True, NULL);
    XtVaSetValues(sw, XmNworkWindow, HelpTextPanes[topic],
	    XmNhorizontalScrollBar, hScrollBar,
    	    XmNverticalScrollBar, vScrollBar, NULL);
    BufSetAll(TextGetBuffer(HelpTextPanes[topic]), HelpText[topic]);
    
    /* This shouldn't be necessary (what's wrong in text.c?) */
    HandleXSelections(HelpTextPanes[topic]);
    
    /* Process dialog mnemonic keys */
    AddDialogMnemonicHandler(form);
    
    /* Set the default button */
    XtVaSetValues(form, XmNdefaultButton, dismissBtn, NULL);
    XtVaSetValues(form, XmNcancelButton, dismissBtn, NULL);
    
    /* realize all of the widgets in the new window */
    RealizeWithoutForcingPosition(appShell);

    /* Make close command in window menu gracefully prompt for close */
    AddMotifCloseCallback(appShell, (XtCallbackProc)dismissCB, appShell);
    
    return appShell;
}

static void dismissCB(Widget w, XtPointer clientData, XtPointer callData)
{
    int topic;
    
    if ((topic = findTopicFromShellWidget((Widget)clientData)) == -1)
    	return;
    
    /* I don't understand the mechanism by which this can be called with
       HelpWindows[topic] as NULL, but it has happened */
    XtDestroyWidget(HelpWindows[topic]);
    HelpWindows[topic] = NULL;
}

static void searchHelpCB(Widget w, XtPointer clientData, XtPointer callData)
{
    char promptText[DF_MAX_PROMPT_LENGTH];
    int response, topic;
    static char **searchHistory = NULL;
    static int nHistoryStrings = 0;
    
    if ((topic = findTopicFromShellWidget((Widget)clientData)) == -1)
    	return; /* shouldn't happen */
    SetDialogFPromptHistory(searchHistory, nHistoryStrings);
    response = DialogF(DF_PROMPT, HelpWindows[topic], 3,
	    "Search for:    (use up arrow key to recall previous)",
    	    promptText, "This Section", "All Sections", "Cancel");
    if (response == 3)
    	return;
    AddToHistoryList(promptText, &searchHistory, &nHistoryStrings);
    searchHelpText(HelpWindows[topic], topic, promptText, response == 2, 0, 0);
}

static void searchHelpAgainCB(Widget w, XtPointer clientData,
	XtPointer callData)
{
    int topic;
    
    if ((topic = findTopicFromShellWidget((Widget)clientData)) == -1)
    	return; /* shouldn't happen */
    searchHelpText(HelpWindows[topic], topic, LastSearchString,
	    LastSearchWasAllTopics, LastSearchPos, LastSearchTopic);
}

static void printCB(Widget w, XtPointer clientData, XtPointer callData)
{
    int topic, helpStringLen;
    char *helpString;
    
    if ((topic = findTopicFromShellWidget((Widget)clientData)) == -1)
    	return; /* shouldn't happen */
    helpString = TextGetWrapped(HelpTextPanes[topic], 0,
	    TextGetBuffer(HelpTextPanes[topic])->length, &helpStringLen);
    PrintString(helpString, helpStringLen, HelpWindows[topic],
	    HelpTitles[topic]);
}

static void searchHelpText(Widget parent, int parentTopic, char *searchFor,
	int allSections, int startPos, int startTopic)
{    
    int topic, beginMatch, endMatch;
    int found = False;
    
    /* Search for the string */
    for (topic=startTopic; topic<NUM_TOPICS; topic++) {
	if (!allSections && topic != parentTopic)
	    continue;
	if (SearchString(HelpText[topic], searchFor, SEARCH_FORWARD,
		SEARCH_LITERAL, False, topic == startTopic ? startPos : 0,
		&beginMatch, &endMatch, NULL, GetPrefDelimiters())) {
	    found = True;
	    break;
	}
    }
    if (!found) {
	if (startPos != 0 || (allSections && startTopic != 0)) { /* Wrap search */
	    searchHelpText(parent, parentTopic, searchFor, allSections, 0, 0);
	    return;
    	}
	DialogF(DF_INF, parent, 1, "String Not Found", "Dismiss");
	return;
    }
    
    /* If the appropriate window is already up, bring it to the top, if not,
       make the parent window become this topic */
    if (HelpWindows[topic] == NULL) {
	XtVaSetValues(HelpWindows[parentTopic], XmNtitle, HelpTitles[topic], NULL);
	BufSetAll(TextGetBuffer(HelpTextPanes[parentTopic]), HelpText[topic]);
	HelpWindows[topic] = HelpWindows[parentTopic];
	HelpTextPanes[topic] = HelpTextPanes[parentTopic];
	HelpWindows[parentTopic] = NULL;
	HelpTextPanes[parentTopic] = NULL;
    } else if (topic != parentTopic)
	RaiseShellWindow(HelpWindows[topic]);
    BufSelect(TextGetBuffer(HelpTextPanes[topic]), beginMatch, endMatch);
    TextSetCursorPos(HelpTextPanes[topic], endMatch);
    
    /* Save the search information for search-again */
    strcpy(LastSearchString, searchFor);
    LastSearchTopic = topic;
    LastSearchPos = endMatch;
    LastSearchWasAllTopics = allSections;
}

static int findTopicFromShellWidget(Widget shellWidget)
{
    int i;
    
    for (i=0; i<NUM_TOPICS; i++)
	if (shellWidget == HelpWindows[i])
	    return i;
    return -1;
}
