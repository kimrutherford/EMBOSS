## Process this file with automake to produce Makefile.in

lib_LTLIBRARIES = libajaxg.la

AM_CFLAGS = $(WARN_CFLAGS) $(DEVWARN_CFLAGS)

if ISCYGWIN
CYGWIN_LDFLAGS = -no-undefined -Wl,--add-stdcall-alias -Wl,--export-all-symbols -Wl,--enable-auto-import
CYGWIN_LDAJAXG = -L../../plplot -L../expat -L../pcre -L../core -lajax -leplplot -leexpat -lepcre
endif

if !ESYSTEMLIBS
EXTRA_INCLUDES = -I$(top_srcdir)/ajax/expat
endif


AM_CPPFLAGS = -I$(top_srcdir)/ajax/core \
-I$(top_srcdir)/plplot \
-I$(top_srcdir)/ajax/pcre \
$(EXTRA_INCLUDES)

AJAXGSRC = ajgraph.c ajhist.c

include_HEADERS = ajgraph.h ajgraphstruct.h  ajhist.h

libajaxg_la_SOURCES = $(AJAXGSRC)

VERS_INF = -version-info 6:4:0

if ISCYGWIN
LINKFLAGS = $(VERS_INF) $(CYGWIN_LDFLAGS) $(CYGWIN_LDAJAXG)
else
LINKFLAGS = $(VERS_INF)
endif

libajaxg_la_LDFLAGS = $(LINKFLAGS)
