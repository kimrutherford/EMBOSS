## Process this file with automake to produce Makefile.in

lib_LTLIBRARIES = libezlib.la

libezlib_la_LDFLAGS  = -version-info 2:0:1
if ISCYGWIN
libezlib_la_LDFLAGS += -no-undefined
libezlib_la_LDFLAGS += -Wl,--add-stdcall-alias
libezlib_la_LDFLAGS += -Wl,--export-all-symbols
libezlib_la_LDFLAGS += -Wl,--enable-auto-import
endif

libezlib_la_SOURCES = \
adler32.c \
compress.c \
crc32.c \
deflate.c \
gzclose.c \
gzlib.c \
gzread.c \
gzwrite.c \
infback.c \
inffast.c \
inflate.c \
inftrees.c \
trees.c \
uncompr.c \
zutil.c

ezlibincludedir = $(includedir)/ezlib

ezlibinclude_HEADERS = \
crc32.h \
deflate.h \
gzguts.h \
inffast.h \
inffixed.h \
inflate.h \
inftrees.h \
trees.h \
zconf.h \
zlibemboss.h \
zlib.h \
zutil.h
