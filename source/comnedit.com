$ ! $Id: comnedit.com,v 1.7 2001/12/20 15:38:26 amai Exp $
$ !
$ ! VMS procedure to compile and link modules for NEdit
$ !
$ SET NOVERIFY
$ ON ERROR THEN EXIT
$ ! COMPILE := CC/DEBUG/NOOPTIMIZE
$ COMPILE := CC/DEFINE=(USE_ACCESS)
$ ! For some systems: COMPILE := CC /PREFIX_LIBRARY_ENTRIES=ALL_ENTRIES
$ DEFINE SYS DECC$LIBRARY_INCLUDE
$ DEFINE XM DECW$INCLUDE
$ DEFINE X11 DECW$INCLUDE
$ !
$ SET VERIFY
$ COMPILE selection.c
$ COMPILE file.c
$ COMPILE help.c
$ COMPILE menu.c
$ COMPILE nedit.c
$ COMPILE preferences.c
$ COMPILE regularExp.c
$ COMPILE search.c
$ COMPILE shift.c
$ COMPILE tags.c
$ COMPILE undo.c
$ COMPILE window.c
$ COMPILE userCmds.c
$ COMPILE macro.c
$ COMPILE text.c
$ COMPILE textSel.c
$ COMPILE textDisp.c
$ COMPILE textBuf.c
$ COMPILE textDrag.c
$ COMPILE server.c
$ COMPILE highlight.c
$ COMPILE highlightData.c
$ COMPILE interpret.c
$ COMPILE smartIndent.c
$ COMPILE regexconvert.c
$ COMPILE rbtree.c
$ !
$ COPY parse_noyacc.c parse.c
$ COMPILE parse.c
$ !
$ COMPILE nc.c
$ !
$ @LNKNEDIT
