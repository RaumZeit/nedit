# $Id: Makefile,v 1.7 2002/06/08 13:56:49 tringali Exp $
SHELL=/bin/sh
#
# Makefile for NEdit text editor
#
# Targets are the suffixes of the system-specific makefiles in
# the makefiles/ directory.
# For example, to build NEdit for Solaris, give the command
#
#   make solaris
#
# This builds an intermediate library in the util/ directory,
# then builds the nedit and nc executables in the source/ directory.
#

all:
	@echo "Please specify target:"
	@echo "(For example, type \"make linux\" for a Linux system.)"
	@(cd makefiles && ls -C Makefile* | sed -e 's/Makefile.//g')

.DEFAULT:
	@- (cd util;   if [ -f ../makefiles/Makefile.$@ -a ! -f ./Makefile.$@ ];\
	   then ln -s ../makefiles/Makefile.$@ .; fi)
	@- (cd source; if [ -f ../makefiles/Makefile.$@ -a ! -f ./Makefile.$@ ];\
	   then ln -s ../makefiles/Makefile.$@ .; fi)
	(cd util;   $(MAKE) -f Makefile.$@ libNUtil.a)
	(cd source; $(MAKE) -f Makefile.$@ nedit nc)

clean:
	(cd util;   $(MAKE) -f Makefile.common clean)
	(cd source; $(MAKE) -f Makefile.common clean)

#
# The following is for creating binary packages of NEdit.
#
RELEASE=nedit-5.3
BINDIST-FILES=source/nedit source/nc README COPYRIGHT ReleaseNotes doc/nedit.doc doc/nedit.html doc/nedit.man doc/nc.man doc/faq.txt

dist-bin: $(RELEASE-FILES)
	rm -rf $(RELEASE)
	mkdir -p $(RELEASE)
	cp $(BINDIST-FILES) $(RELEASE)
	strip $(RELEASE)/nedit $(RELEASE)/nc
	chmod 555 $(RELEASE)/nedit $(RELEASE)/nc
	tar cf $(RELEASE).tar $(RELEASE)
	compress $(RELEASE).tar
	mv $(RELEASE).tar.Z $(RELEASE)-`uname -s`-`uname -p`.tar.Z
	rm -rf $(RELEASE)
