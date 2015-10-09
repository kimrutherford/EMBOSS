## Process this file with automake to produce Makefile.in

lib_LTLIBRARIES = libnucleus.la

libnucleus_la_CFLAGS  = $(WARN_CFLAGS) $(DEVWARN_CFLAGS) $(JAVA_CFLAGS)

if ISSHARED
if ISAIXIA64
libnucleus_la_CFLAGS += -Wl,-G -lm
endif
endif

libnucleus_la_CPPFLAGS  = -I$(top_srcdir)/plplot
libnucleus_la_CPPFLAGS += -I$(top_srcdir)/ajax/pcre
if !ESYSTEMLIBS
libnucleus_la_CPPFLAGS += -I$(top_srcdir)/ajax/expat
libnucleus_la_CPPFLAGS += -I$(top_srcdir)/ajax/zlib
endif
libnucleus_la_CPPFLAGS += -I$(top_srcdir)/ajax/graphics
libnucleus_la_CPPFLAGS += -I$(top_srcdir)/ajax/ensembl
libnucleus_la_CPPFLAGS += -I$(top_srcdir)/ajax/ajaxdb
libnucleus_la_CPPFLAGS += -I$(top_srcdir)/ajax/acd
libnucleus_la_CPPFLAGS += $(JAVA_CPPFLAGS)
libnucleus_la_CPPFLAGS += $(X_CFLAGS)

libnucleus_la_LDFLAGS  = -version-info 6:4:0
if ISCYGWIN
libnucleus_la_LDFLAGS += -no-undefined
libnucleus_la_LDFLAGS += -Wl,--add-stdcall-alias
libnucleus_la_LDFLAGS += -Wl,--export-all-symbols
libnucleus_la_LDFLAGS += -Wl,--enable-auto-import
libnucleus_la_LDFLAGS += -L../plplot
libnucleus_la_LDFLAGS += -L../ajax/pcre
libnucleus_la_LDFLAGS += -L../ajax/expat
libnucleus_la_LDFLAGS += -L../ajax/zlib
libnucleus_la_LDFLAGS += -L../ajax/core
libnucleus_la_LDFLAGS += -L../ajax/graphics
libnucleus_la_LDFLAGS += -L../ajax/ensembl
libnucleus_la_LDFLAGS += -L../ajax/ajaxdb
libnucleus_la_LDFLAGS += -L../ajax/acd
libnucleus_la_LDFLAGS += -lacd
libnucleus_la_LDFLAGS += -lajaxdb
libnucleus_la_LDFLAGS += -lensembl
libnucleus_la_LDFLAGS += -lajaxg
libnucleus_la_LDFLAGS += -lajax
libnucleus_la_LDFLAGS += -leexpat
libnucleus_la_LDFLAGS += -lepcre
libnucleus_la_LDFLAGS += -lezlib
libnucleus_la_LDFLAGS += -leplplot
endif

libnucleus_la_SOURCES = \
embaln.c \
embcom.c \
embcons.c \
embdata.c \
embdbi.c \
embdmx.c \
embdomain.c \
embest.c \
embexit.c \
embgroup.c \
embiep.c \
embindex.c \
embinit.c \
embmat.c \
embmisc.c \
embmol.c \
embnmer.c \
embpat.c \
embpatlist.c \
embprop.c \
embpdb.c \
embread.c \
embsig.c \
embshow.c \
embword.c

include_HEADERS = \
emboss.h \
embaln.h \
embcom.h \
embcons.h \
embdata.h \
embdbi.h \
embdmx.h \
embdomain.h \
embest.h \
embexit.h \
embgroup.h \
embiep.h \
embindex.h \
embinit.h \
embmat.h \
embmisc.h \
embmol.h \
embnmer.h \
embpat.h \
embpatlist.h \
embprop.h \
embpdb.h \
embread.h \
embsig.h \
embshow.h \
embword.h
