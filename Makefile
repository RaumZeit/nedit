SHELL=/bin/sh
# Makefile for NEdit text editor
#
# Targets are the sufixes of the system-specific makefiles in directory
# "makefiles".  For example, to build NEdit for Solaris, give the command
#
#   make solaris
#
# Builds an intermediate library in util directory, then builds
# the nedit executable in the source directory.
#

all:
	@echo "Please specify target"
	@ls -C makefiles | sed -e 's/Makefile.//g'

.DEFAULT:
	@- (cd util;   if [ -f ../makefiles/Makefile.$@ -a ! -f ./Makefile.$@ ];\
	   then ln -s ../makefiles/Makefile.$@ .; fi)
	@- (cd source; if [ -f ../makefiles/Makefile.$@ -a ! -f ./Makefile.$@ ];\
	   then ln -s ../makefiles/Makefile.$@ .; fi)
	(cd util;   $(MAKE) -f Makefile.$@ libNUtil.a)
	(cd source; $(MAKE) -f Makefile.$@ nedit nc)

clean:
	(cd util; $(MAKE) -f Makefile.common clean)
	(cd source; $(MAKE) -f Makefile.common clean)
