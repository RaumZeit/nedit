$ !
$ ! DCL link procedure for NEdit
$ !
$ SET NOVERIFY
OBJS :=	nedit, file, menu, window, selection, search, undo, shift, -
	help, preferences, tags, userCmds, regularExp, macro, text, -
	textSel, textDisp, textBuf, textDrag, server, highlight, -
        highlightData, interpret, parse, smartIndent

$ SET VERIFY
$ LINK 'OBJS', NEDIT_OPTIONS_FILE/OPT, [-.util]vmsUtils/lib, libUtil/lib
$ LINK nc, NEDIT_OPTIONS_FILE/OPT, [-.util]vmsUtils/lib, libUtil/lib
