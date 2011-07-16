## Process this file with automake to produce Makefile.in

lib_LTLIBRARIES = libezlib.la


if ISCYGWIN
CYGWIN_LDFLAGS = -no-undefined -Wl,--add-stdcall-alias -Wl,--export-all-symbols -Wl,--enable-auto-import
endif

AM_CPPFLAGS = -I$(top_srcdir)/ajax/core

ZLIBSRC = adler32.c compress.c crc32.c deflate.c gzio.c infback.c inffast.c \
inflate.c inftrees.c trees.c uncompr.c zutil.c

# Uncomment and amend when there are some real headers
pkginclude_HEADERS = crc32.h deflate.h inffast.h inffixed.h inflate.h \
inftrees.h trees.h zconf.h zlibemboss.h zlib.h zutil.h

pkgincludedir = $(includedir)/ezlib

libezlib_la_SOURCES = $(ZLIBSRC)

VERS_INF = -version-info 2:0:1

if ISCYGWIN
LINKFLAGS = $(VERS_INF) $(CYGWIN_LDFLAGS)
else
LINKFLAGS = $(VERS_INF)
endif

libezlib_la_LDFLAGS = $(LINKFLAGS)
