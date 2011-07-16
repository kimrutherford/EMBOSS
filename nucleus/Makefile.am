## Process this file with automake to produce Makefile.in

lib_LTLIBRARIES = libnucleus.la

if ISSHARED
if ISAIXIA64
AIX_CFLAGS = -Wl,-G -lm
endif
endif

AM_CFLAGS = $(AIX_CFLAGS) $(WARN_CFLAGS) $(DEVWARN_CFLAGS)

if ISCYGWIN
CYGWIN_LDFLAGS = -no-undefined -Wl,--add-stdcall-alias \
	-Wl,--export-all-symbols -Wl,--enable-auto-import
CYGWIN_LIBS = -L../plplot -L../ajax/pcre -L../ajax/expat -L../ajax/zlib \
-L../ajax/core -L../ajax/graphics -L../ajax/ensembl -L../ajax/ajaxdb \
-L../ajax/acd -lacd -lajaxdb -lensembl -lajaxg -lajax -leexpat -lepcre \
-lezlib -leplplot
else
if !ESYSTEMLIBS
EXTRA_INCLUDES = -I$(top_srcdir)/ajax/expat -I$(top_srcdir)/ajax/zlib
endif
endif


AM_CPPFLAGS =  -I$(top_srcdir)/ajax/core -I$(top_srcdir)/plplot \
	$(X_CFLAGS) -I$(srcdir)/ \
	-I$(top_srcdir)/ajax -I$(top_srcdir)/ajax/pcre \
	$(EXTRA_INCLUDES) \
	-I$(top_srcdir)/ajax/graphics \
	-I$(top_srcdir)/ajax/ensembl -I$(top_srcdir)/ajax/ajaxdb \
	-I$(top_srcdir)/ajax/acd

NUCLSRC =  embaln.c embcom.c embcons.c embdata.c embdbi.c embdmx.c \
	embdomain.c embest.c embexit.c embgroup.c embiep.c embindex.c \
        embinit.c embmat.c embmisc.c embmol.c embnmer.c embpat.c \
        embpatlist.c embprop.c embpdb.c embread.c embsig.c embshow.c \
        embword.c

include_HEADERS = emboss.h embaln.h embcom.h embcons.h \
	embdata.h embdbi.h embdmx.h embdomain.h \
	embest.h embexit.h embgroup.h embiep.h embindex.h embinit.h \
	embmat.h embmisc.h embmol.h \
	embnmer.h embpat.h embpatlist.h embprop.h embpdb.h \
        embread.h embsig.h embshow.h embword.h

libnucleus_la_SOURCES = $(NUCLSRC)

VERS_INF =  -version-info 6:4:0

if ISCYGWIN
LINKFLAGS =  $(VERS_INF) $(CYGWIN_LDFLAGS) $(CYGWIN_LIBS)
else
LINKFLAGS =  $(VERS_INF)
endif

libnucleus_la_LDFLAGS =  $(LINKFLAGS)
