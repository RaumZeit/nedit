$ ! $Id: comnedit.com,v 1.4 2001/02/26 23:38:03 edg Exp $
$ !
$ ! VMS procedure to compile and link modules for NEdit
$ !
$ SET NOVERIFY
$ ON ERROR THEN EXIT
$ ! COMPILE := CC/DEBUG/NOOPTIMIZE
$ COMPILE := CC/DEFINE=(NO_FCHMOD,USE_ACCESS)
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
$ !
$ COPY parse.c_noyacc parse.c
$ COMPILE parse.c
$ !
$ COMPILE nc.c
$ !
$ @LNKNEDIT
