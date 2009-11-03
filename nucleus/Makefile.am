## Process this file with automake to produce Makefile.in

lib_LTLIBRARIES = libnucleus.la

if ISSHARED
if ISAIXIA64
AM_CFLAGS = -Wl,-G -lm -L../ajax/.libs -lajax
else
AM_CFLAGS =
endif
endif

if ISCYGWIN
AM_LDFLAGS = -no-undefined -Wl,--add-stdcall-alias -Wl,--export-all-symbols -Wl,--enable-auto-import -L../plplot -L../ajax -leplplot -lajax
endif


INCLUDES = -I$(top_srcdir)/plplot $(X_CFLAGS) -I$(srcdir)/ -I$(top_srcdir)/ajax

LIBS =

NUCLSRC =  embaln.c embcom.c embcons.c \
	   embdata.c embdbi.c embdmx.c embdomain.c \
	   embest.c embexit.c embgroup.c \
	   embiep.c embindex.c embinit.c \
	   embmat.c embmisc.c embmol.c \
	   embnmer.c embpat.c embpatlist.c embprop.c embpdb.c \
           embread.c embsig.c \
	   embshow.c embword.c

libnucleus_la_SOURCES = $(NUCLSRC)

libnucleus_la_LDFLAGS =  -version-info 5:0:0

include_HEADERS = emboss.h \
		 embaln.h embcom.h embcons.h \
		 embdata.h embdbi.h embdmx.h embdomain.h \
		 embest.h embexit.h embgroup.h \
		 embiep.h embindex.h embinit.h \
		 embmat.h embmisc.h embmol.h \
		 embnmer.h embpat.h embpatlist.h embprop.h embpdb.h \
                 embread.h embsig.h \
                 embshow.h embword.h 
