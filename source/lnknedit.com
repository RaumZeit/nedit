$ ! $Id: lnknedit.com,v 1.4 2001/07/11 15:21:52 amai Exp $
$ !
$ ! DCL link procedure for NEdit
$ !
$ SET NOVERIFY
OBJS :=	nedit, file, menu, window, selection, search, undo, shift, -
	help, preferences, tags, userCmds, regularExp, macro, text, -
	textSel, textDisp, textBuf, textDrag, server, highlight, -
        highlightData, interpret, parse, smartIndent, regexconvert,
        rbTree

$ SET VERIFY
$ LINK 'OBJS', NEDIT_OPTIONS_FILE/OPT, [-.util]vmsUtils/lib, libUtil/lib
$ LINK nc, NEDIT_OPTIONS_FILE/OPT, [-.util]vmsUtils/lib, libUtil/lib
