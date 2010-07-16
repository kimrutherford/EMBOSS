## Process this file with automake to produce Makefile.in

lib_LTLIBRARIES = libacd.la

AM_CFLAGS = $(WARN_CFLAGS) $(DEVWARN_CFLAGS)

if ISCYGWIN
CYGWIN_LDFLAGS = -no-undefined -Wl,--add-stdcall-alias \
-Wl,--export-all-symbols -Wl,--enable-auto-import
CYGWIN_LDACD = -L../../plplot -L../pcre -L../expat -L../zlib -L../core \
-L../graphics -L../ensembl -L../ajaxdb -lajaxdb -lensembl -lajaxg -lajax \
-leplplot -leexpat -lepcre -lezlib
endif

if !ESYSTEMLIBS
EXTRA_INCLUDES = -I$(top_srcdir)/ajax/expat -I$(top_srcdir)/ajax/zlib
endif

INCLUDES = -I$(top_srcdir)/plplot -I$(top_srcdir)/ajax/pcre \
$(EXTRA_INCLUDES) \
-I$(top_srcdir)/ajax/core \
-I$(top_srcdir)/ajax/graphics \
-I$(top_srcdir)/ajax/ensembl \
-I$(top_srcdir)/ajax/ajaxdb

ACDSRC = ajacd.c

include_HEADERS = ajacd.h

libacd_la_SOURCES = $(ACDSRC)

VERS_INF = -version-info 6:3:0

if ISCYGWIN
LINKFLAGS = $(VERS_INF) $(CYGWIN_LDFLAGS) $(CYGWIN_LDACD)
else
LINKFLAGS = $(VERS_INF)
endif

libacd_la_LDFLAGS = $(LINKFLAGS)
