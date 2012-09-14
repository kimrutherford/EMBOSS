## Process this file with automake to produce Makefile.in

lib_LTLIBRARIES = libeexpat.la

libeexpat_la_LDFLAGS  = -version-info 2:0:1
if ISCYGWIN
libeexpat_la_LDFLAGS += -no-undefined
libeexpat_la_LDFLAGS += -Wl,--add-stdcall-alias
libeexpat_la_LDFLAGS += -Wl,--export-all-symbols
libeexpat_la_LDFLAGS += -Wl,--enable-auto-import
endif

libeexpat_la_SOURCES = \
xmlparse.c \
xmltok.c \
xmlrole.c \
xmltok_impl.c \
xmltok_ns.c

eexpatincludedir = $(includedir)/eexpat

eexpatinclude_HEADERS = \
expat_config.h \
ascii.h \
asciitab.h \
iasciitab.h \
internal.h \
latin1tab.h \
nametab.h \
utf8tab.h \
xmlrole.h \
xmltok.h \
xmltok_impl.h \
expat.h \
expat_external.h \
expatemboss.h
