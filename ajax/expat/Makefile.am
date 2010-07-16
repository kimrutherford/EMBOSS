## Process this file with automake to produce Makefile.in

lib_LTLIBRARIES = libeexpat.la


if ISCYGWIN
CYGWIN_LDFLAGS = -no-undefined -Wl,--add-stdcall-alias -Wl,--export-all-symbols -Wl,--enable-auto-import
endif

INCLUDES =

EXPATSRC = xmlparse.c xmltok.c xmlrole.c xmltok_impl.c xmltok_ns.c

pkginclude_HEADERS = expat_config.h ascii.h asciitab.h iasciitab.h \
internal.h latin1tab.h nametab.h utf8tab.h xmlrole.h xmltok.h \
xmltok_impl.h expat.h expat_external.h expatemboss.h

pkgincludedir = $(includedir)/eexpat

libeexpat_la_SOURCES = $(EXPATSRC)

VERS_INF = -version-info 2:0:1

if ISCYGWIN
LINKFLAGS = $(VERS_INF) $(CYGWIN_LDFLAGS)
else
LINKFLAGS = $(VERS_INF)
endif

libeexpat_la_LDFLAGS = $(LINKFLAGS)
