#
# Makefile for NEdit text editor
#
# targets: sgi_nedit, hp_nedit, ultrix_nedit, ibm_nedit, sunos_nedit,
#	   solaris_nedit, dec_nedit, linux_nedit
#
# Builds an intermediate library in util directory, then builds
# the nedit executable in the source directory
#

all:
	@echo "Please specify target"

sgi_nedit:
	(cd util; $(MAKE) -f Makefile.sgi libNUtil.a)
	(cd source; $(MAKE) -f Makefile.sgi nedit nc)

hp_nedit:
	(cd util; $(MAKE) -f Makefile.hp libNUtil.a)
	(cd source; $(MAKE) -f Makefile.hp nedit nc)

ultrix_nedit:
	(cd util; $(MAKE) -f Makefile.ultrix libNUtil.a)
	(cd source; $(MAKE) -f Makefile.ultrix nedit nc)

ibm_nedit:
	(cd util; $(MAKE) -f Makefile.ibm libNUtil.a)
	(cd source; $(MAKE) -f Makefile.ibm nedit nc)

sunos_nedit:
	(cd util; $(MAKE) -f Makefile.sunos libNUtil.a)
	(cd source; $(MAKE) -f Makefile.sunos nedit nc)

solaris_nedit:
	(cd util; $(MAKE) -f Makefile.solaris libNUtil.a)
	(cd source; $(MAKE) -f Makefile.solaris nedit nc)

dec_nedit:
	(cd util; $(MAKE) -f Makefile.dec libNUtil.a)
	(cd source; $(MAKE) -f Makefile.dec nedit nc)

linux_nedit:
	(cd util; $(MAKE) -f Makefile.linux libNUtil.a)
	(cd source; $(MAKE) -f Makefile.linux nedit nc)
