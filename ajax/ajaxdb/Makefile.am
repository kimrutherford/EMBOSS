## Process this file with automake to produce Makefile.in

lib_LTLIBRARIES = libajaxdb.la

AM_CFLAGS = $(WARN_CFLAGS) $(DEVWARN_CFLAGS)


if ISCYGWIN
CYGWIN_LDFLAGS = -no-undefined -Wl,--add-stdcall-alias \
-Wl,--export-all-symbols -Wl,--enable-auto-import
CYGWIN_LDAJAXDB = -L../../plplot -L../pcre -L../expat -L../zlib -L../core \
-L../ensembl -lensembl lajax -leplplot -leexpat -lepcre -lezlib
endif

if !ESYSTEMLIBS
EXTRA_INCLUDES = -I$(top_srcdir)/ajax/expat -I$(top_srcdir)/ajax/zlib
endif


AM_CPPFLAGS = -I$(top_srcdir)/ajax/core \
-I$(top_srcdir)/plplot \
-I$(top_srcdir)/ajax/pcre \
$(EXTRA_INCLUDES) \
$(AXIS2C_CPPFLAGS) \
-I$(top_srcdir)/ajax/ensembl


AJAXDBSRC = ajassemdb.c ajdas.c ajfeatdb.c ajmart.c \
	ajobodb.c ajresourcedb.c ajseqdb.c ajsoap.c ajtaxdb.c ajtextdb.c \
	ajurldb.c ajvardb.c

# Uncomment and amend when there are some real headers
include_HEADERS = ajassemdb.h ajdas.h ajfeatdb.h ajmart.h \
	ajobodb.h ajresourcedb.h ajseqdb.h ajsoap.h ajtaxdb.h ajtextdb.h \
	ajurldb.h ajvardb.h

libajaxdb_la_SOURCES = $(AJAXDBSRC)

VERS_INF = -version-info 6:4:0

if ISCYGWIN
LINKFLAGS = $(VERS_INF) $(CYGWIN_LDFLAGS) $(CYGWIN_LDAJAXDB)
else
LINKFLAGS = $(VERS_INF)
endif

libajaxdb_la_LDFLAGS = $(LINKFLAGS) $(AXIS2C_LDFLAGS)
