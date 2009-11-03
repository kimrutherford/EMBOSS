## Process this file with automake to produce Makefile.in

SUBDIRS = lib

lib_LTLIBRARIES = libeplplot.la

if ISSHARED
if ISAIXIA64

if AMPNG
AM_CFLAGS = -Wl,-G -lgd -lpng -lz -lX11 -lm
else
AM_CFLAGS = -Wl,-G -lX11
endif

else
AM_CFLAGS =
endif
endif

if ISCYGWIN
AM_LDFLAGS = -no-undefined -Wl,--add-stdcall-alias -Wl,--export-all-symbols -Wl,--enable-auto-import -lX11 -lgd -lpng -lz
endif


INCLUDES = $(X_CFLAGS) -I$(srcdir)/ -I/usr/include/gd -DPREFIX=\"$(prefix)\" -DBUILD_DIR=\"$(srcdir)\" -DDRV_DIR=\"$(srcdir)\" -DEMBOSS_TOP=\"$(EMBOSS_TOP)\"
LIBS =

PLPLOTSRC = pdfutils.c plargs.c plbox.c plcont.c plcore.c \
plctrl.c plcvt.c pldtik.c plfill.c plhist.c plline.c plmap.c \
plot3d.c plpage.c plsdef.c plshade.c plsym.c pltick.c \
plvpor.c plwind.c plstripc.c hpgl.c impress.c \
ljiip.c ljii.c null.c data.c pbm.c plbuf.c \
plmeta.c ps.c tek.c xfig.c xwin.c gd.c plimage.c plstdio.c plvect.c

# removed gdpng.c (replaced by gd.c) next.c gd300.c

libeplplot_la_SOURCES = $(PLPLOTSRC)

libeplplot_la_LDFLAGS = -version-info 5:7:2

pkginclude_HEADERS = drivers.h pdf.h plcore.h plevent.h plplotP.h plxwd.h \
metadefs.h plConfig.h pldebug.h plplot.h plstrm.h plDevs.h \
disptab.h pldll.h plhershey-unicode.h ps.h \
plunicode-type1.h plfci-type1.h

pkgincludedir = $(includedir)/eplplot
