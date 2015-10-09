## Process this file with automake to produce Makefile.in

lib_LTLIBRARIES = libajaxg.la

libajaxg_la_CFLAGS = $(WARN_CFLAGS) $(DEVWARN_CFLAGS) $(JAVA_CFLAGS)

libajaxg_la_CPPFLAGS  = -I$(top_srcdir)/ajax/pcre
if !ESYSTEMLIBS
libajaxg_la_CPPFLAGS += -I$(top_srcdir)/ajax/expat
endif
libajaxg_la_CPPFLAGS += -I$(top_srcdir)/plplot
libajaxg_la_CPPFLAGS += $(JAVA_CPPFLAGS)

libajaxg_la_LDFLAGS  = -version-info 6:4:0
if ISCYGWIN
libajaxg_la_LDFLAGS += -no-undefined
libajaxg_la_LDFLAGS += -Wl,--add-stdcall-alias
libajaxg_la_LDFLAGS += -Wl,--export-all-symbols
libajaxg_la_LDFLAGS += -Wl,--enable-auto-import
libajaxg_la_LDFLAGS += -L../../plplot
libajaxg_la_LDFLAGS += -L../expat
libajaxg_la_LDFLAGS += -L../pcre
libajaxg_la_LDFLAGS += -L../core
libajaxg_la_LDFLAGS += -lajax
libajaxg_la_LDFLAGS += -leplplot
libajaxg_la_LDFLAGS += -leexpat
libajaxg_la_LDFLAGS += -lepcre
endif

libajaxg_la_SOURCES = ajgraph.c ajhist.c

include_HEADERS = ajgraph.h ajgraphstruct.h ajhist.h
