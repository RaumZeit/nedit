# Makefile for SCO Unix
#
# Thanks to Koen D'Hondt and patrick@wombat.logica.co.uk, and Andras Porjesz
CC=cc
AR=ar
#CFLAGS=-O -I/usr/include -Dsco -DSYSV -DUSG -DMALLOC_0_RETURNS_NULL -DUSE_DIRENT
#CFLAGS=-O -Dsco -DSYSV -DUSG -DXTFUNCPROTO -DUSE_DIRENT -DMOTIF12
CFLAGS=-O -I/usr/X11R6/include -DUSE_DIRENT -DUSE_LPR_PRINT_CMD -DMAXPATHLEN=256
#LIBS= -lXm -lXt -lXext -lX11 -lc -lx -lsocket -lmalloc -lPW -lintl 
#LIBS=-lXtXm_s -lX11_s -lXmu -lXext -lsocket -lmalloc -lPW -lintl
LIBS -L/usr/X11R6/lib -L/usr/lib -lm -lXm -lXext -lXt -lX11 -lsocket

include Makefile.common
