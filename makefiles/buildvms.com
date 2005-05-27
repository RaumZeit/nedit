$! $Id: buildvms.com,v 1.1 2005/05/27 16:49:04 edg Exp $
$!
$! VMS procedure to compile and link modules for NEdit
$!
$! Subroutine to compile a source file
$compile: subroutine
$ pwd = F$DIRECTORY()
$ crdt = F$CVTIME(F$FILE_ATTRIBUTES("[-.''p1']''p2'.c","RDT"))
$ ordt = F$CVTIME(F$FILE_ATTRIBUTES("[-.''p1']''p2'.obj","RDT"))
$ if "''crdt'" .gts. "''ordt'"
$ then
$ write sys$output "Compiling ", p2
$ cc/define=(use_access)/include="[-.MICROLINE],[-.XLT],[-.MICROLINE.XML],[-.'p1']"/object=[-.'p1']'p2' [-.'p1']'p2'
$ 	libname = p1
$ 	if libname .eqs. "MICROLINE.XML"
$	then
$		libname = "MicrolineXmL"
$	endif
$ 	library/replace/object [-.'p1']'libname'.olb [-.'p1']'p2'
$ endif
$ exit
$ endsubroutine
$!
$ goto 'P1'
$ write sys$output "Please specify one of: clean, all, distbin"
$ write sys$output "clean: delete generated files and rebuild"
$ write sys$output "all: compile out of date files and rebuild"
$ write sys$output "distbin: build a binary distrubution"
$ exit
$!
$! delete generated files and rebuild
$clean:
$ delete/noconfirm/log [-.util]*.obj;*
$ delete/noconfirm/log [-.util]*.olb;*
$ delete/noconfirm/log [-.Xlt]*.obj;*
$ delete/noconfirm/log [-.Xlt]*.olb;*
$ delete/noconfirm/log [-.Microline.XmL]*.obj;*
$ delete/noconfirm/log [-.Microline.XmL]*.olb;*
$ delete/noconfirm/log [-.source]*.obj;*
$ delete/noconfirm/log [-.source]*.olb;*
$ delete/noconfirm/log [-.source]*.exe;*
$ library/create/object [-.util]util.olb
$ library/create/object [-.Xlt]Xlt.olb
$ library/create/object [-.Microline.XmL]MicrolineXmL.olb
$ library/create/object [-.source]source.olb
$!
$! Build nedit
$all:
$ set symbol/scope=noglobal
$ on error then goto the_end
$ define sys decc$library_include
$ define xm decw$include
$ define x11 decw$include
$!
$! utils
$ call compile util dialogf
$ call compile util fileutils
$ call compile util getfiles
$ call compile util misc        
$ call compile util preffile
$ call compile util printutils 
$ call compile util fontsel
$ call compile util managedlist
$ call compile util utils
$ call compile util vmsutils
$!
$! Xlt
$ call compile Xlt bubblebutton 
$ call compile Xlt slidec	
$!
$! Microline/XmL
$ call compile Microline.XmL folder
$ call compile Microline.XmL grid
$ call compile Microline.XmL gridutil
$ call compile Microline.XmL progress
$ call compile Microline.XmL tree
$ call compile Microline.XmL xml
$!
$! source
$ call compile source selection    
$ call compile source server_common
$ call compile source file	   
$ call compile source help	   
$ call compile source menu	   
$ call compile source preferences  
$ call compile source regularexp   
$ call compile source search	   
$ call compile source shift	   
$ call compile source tags	   
$ call compile source undo	   
$ call compile source window	   
$ call compile source usercmds     
$ call compile source macro	   
$ call compile source text	   
$ call compile source textsel	   
$ call compile source textdisp     
$ call compile source textbuf	   
$ call compile source textdrag     
$ call compile source server	   
$ call compile source highlight    
$ call compile source highlightdata
$ call compile source interpret    
$ call compile source smartindent  
$ call compile source regexconvert 
$ call compile source rbtree	   
$ call compile source windowtitle  
$ call compile source linkdate     
$ call compile source calltips     
$ call compile source rangeset     
$ call compile source shell     
$!
$! delete parse.c;*
$ copy [-.source]parse_noyacc.c [-.source]parse.c
$!
$ call compile source parse
$!
$ cc/define=(use_access)/object=[-.source]nedit [-.source]nedit
$ cc/define=(use_access)/object=[-.source]nc [-.source]nc
$!
$! Linking nc and nedit
$ link -
	/executable=[-.source]nedit.exe -
	[-.source]nedit.obj, -
	[-.source]source.olb/library, -
	[-.source]NEDIT_OPTIONS_FILE.OPT/opt, -
	[-.util]util.olb/library, -
	[-.Xlt]Xlt.olb/library, -
	[-.Microline.XmL]MicrolineXmL.olb/library
$!
$ link -
	/executable=[-.source]nc.exe -
	[-.source]server_common.obj, -
	[-.source]NEDIT_OPTIONS_FILE.OPT/opt, -
	[-.source]nc.obj, -
	[-.util]util.olb/library, -
	[-.Xlt]Xlt.olb/library, -
	[-.Microline.XmL]MicrolineXmL.olb/library
$the_end:
$ exit
$
$distbin:
$ if "a''p2'" .eqs. "a"
$ then
$ 	write sys$output "Second argument must be nedit version"
$ 	exit
$ endif
$ on error then continue
$ purge/log [-...]
$ delete [-]'p2'-vms-alpha.bck;*
$ create/directory [-.'p2']
$ set protection=w:rwed [-.'p2'...]*.*;*
$ delete [-.'p2'...]*.*;*
$ delete [-.'p2'...]*.*;*
$ delete [-.'p2'...]*.*;*
$ delete [-.'p2'...]*.*;*
$ delete [-.'p2'...]*.*;*
$ delete [-.'p2']*.*;*
$ create/directory [-.'p2']
$ create/directory [-.'p2']
$ copy [-.source]nedit.exe [-.'p2']
$ copy [-.source]nc.exe [-.'p2']
$ copy [-]README [-.'p2']
$ copy [-]COPYRIGHT [-.'p2']
$ copy [-]ReleaseNotes [-.'p2']
$ copy [-.doc]NEDIT.DOC [-.'p2']
$ copy [-.doc]NEDIT.HTML [-.'p2']
$ copy [-.doc]NEDIT.MAN [-.'p2']
$ copy [-.doc]NC.MAN [-.'p2']
$ copy [-.doc]FAQ.TXT [-.'p2']
$ backup/log [-.'p2'...] 'p2'-vms-alpha.bck/save_set
