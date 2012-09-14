## Process this file with automake to produce Makefile.in

lib_LTLIBRARIES = libepcre.la

# PCRE stuff

POSIX_MALLOC_THRESHOLD = @POSIX_MALLOC_THRESHOLD@
### PCRELIBVERSION = @PCRE_LIB_VERSION@
### PCREPOSIXLIBVERSION = @PCRE_POSIXLIB_VERSION@

libepcre_la_CPPFLAGS = $(POSIX_MALLOC_THRESHOLD)

libepcre_la_LDFLAGS  = -version-info 7:9:0
if ISCYGWIN
libepcre_la_LDFLAGS += -no-undefined
libepcre_la_LDFLAGS += -Wl,--add-stdcall-alias
libepcre_la_LDFLAGS += -Wl,--export-all-symbols
libepcre_la_LDFLAGS += -Wl,--enable-auto-import
endif

libepcre_la_SOURCES = \
pcre_chartables.c \
pcre_compile.c \
pcre_dfa_exec.c \
pcre_exec.c \
pcre_fullinfo.c \
pcre_get.c \
pcre_globals.c \
pcre_info.c \
pcre_newline.c \
pcreposix.c \
pcre_study.c \
pcre_tables.c \
pcre_try_flipped.c \
pcre_version.c \
pcre_xclass.c

epcreincludedir = $(includedir)/epcre

epcreinclude_HEADERS = \
pcre.h \
pcre_config.h \
pcreemboss.h \
pcre_internal.h \
pcreposix.h \
pcre_printint.src \
ucp.h
