## Process this file with automake to produce Makefile.in

lib_LTLIBRARIES = libepcre.la
AM_CFLAGS = -DHAVE_CONFIG_H

if ISCYGWIN
CYGWIN_LDFLAGS = -no-undefined -Wl,--add-stdcall-alias -Wl,--export-all-symbols -Wl,--enable-auto-import
endif

## PCRE stuff

POSIX_MALLOC_THRESHOLD = @POSIX_MALLOC_THRESHOLD@
PCRELIBVERSION = @PCRE_LIB_VERSION@
PCREPOSIXLIBVERSION = @PCRE_POSIXLIB_VERSION@

INCLUDES = $(POSIX_MALLOC_THRESHOLD) 

PCRESRC = pcre_chartables.c pcre_compile.c pcre_exec.c \
pcre_fullinfo.c pcre_get.c pcre_globals.c pcre_info.c pcre_newline.c \
pcreposix.c pcre_study.c pcre_tables.c pcre_try_flipped.c

pkginclude_HEADERS = pcre.h pcre_config.h pcreemboss.h \
	pcre_internal.h pcreposix.h pcre_printint.src ucp.h

pkgincludedir = $(includedir)/epcre

libepcre_la_SOURCES = $(PCRESRC)

VERS_INF = -version-info 7:9:0

if ISCYGWIN
LINKFLAGS = $(VERS_INF) $(CYGWIN_LDFLAGS)
else
LINKFLAGS = $(VERS_INF)
endif

libepcre_la_LDFLAGS = $(LINKFLAGS)
