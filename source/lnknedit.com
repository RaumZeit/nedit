$ ! $Id: lnknedit.com,v 1.7 2002/11/08 20:22:45 edg Exp $
$ !
$ ! DCL link procedure for NEdit
$ !
$ ON ERROR THEN GOTO THE_END
$ VERIFY = 'F$VERIFY (1)'
$ OBJS :=	nedit, file, menu, window, selection, search, undo, shift, -
	help, preferences, tags, userCmds, regularExp, macro, text, -
	textSel, textDisp, textBuf, textDrag, server, highlight, -
        highlightData, interpret, parse, smartIndent, regexconvert, -
        rbTree, windowtitle, linkdate, server_common, calltips, rangeset

$ LINK 'OBJS', NEDIT_OPTIONS_FILE/OPT, [-.util]vmsUtils/lib, libUtil/lib
$ LINK nc, server_common.obj, NEDIT_OPTIONS_FILE/OPT, [-.util]vmsUtils/lib, libUtil/lib
$THE_END:
$ EXIT (F$VERIFY(VERIFY))
