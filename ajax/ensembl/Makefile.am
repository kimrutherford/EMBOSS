## Process this file with automake to produce Makefile.in

lib_LTLIBRARIES = libensembl.la

AM_CFLAGS = $(WARN_CFLAGS) $(DEVWARN_CFLAGS)

if ISCYGWIN
CYGWIN_LDFLAGS = -no-undefined -Wl,--add-stdcall-alias -Wl,--export-all-symbols -Wl,--enable-auto-import
CYGWIN_LDENSEMBL = -L../../plplot -L../expat -L../pcre -L../core -lajax -leplplot -leexpat -lepcre
endif

INCLUDES = -I$(top_srcdir)/plplot -I$(top_srcdir)/ajax/expat \
-I$(top_srcdir)/ajax/pcre -I$(top_srcdir)/ajax/core

ENSEMBLSRC = ensanalysis.c ensassembly.c ensassemblyexception.c \
ensassemblymapper.c ensattribute.c ensbaseadaptor.c enscache.c \
enscoordsystem.c ensdatabaseadaptor.c ensdatabaseconnection.c \
ensdatabaseentry.c ensdensity.c ensditag.c ensexon.c \
ensexternaldatabase.c ensfeature.c ensgene.c ensgvindividual.c \
ensgvpopulation.c ensgvsample.c ensintron.c \
enskaryotype.c ensmapper.c ensmarker.c ensmetacoordinate.c \
ensmetainformation.c ensmiscellaneous.c ensprediction.c \
ensprojectionsegment.c ensqcalignment.c ensqc.c ensqcdatabase.c \
ensqcsequence.c ensregistry.c ensrepeat.c ensseqregion.c \
enssequence.c enssequenceedit.c ensslice.c ensstorable.c \
enstable.c enstranscript.c enstranslation.c ensutility.c \
ensvariation.c

include_HEADERS = ensanalysis.h ensassemblyexception.h ensassembly.h \
ensassemblymapper.h ensattribute.h ensbaseadaptor.h enscache.h \
enscoordsystem.h ensdatabaseadaptor.h ensdatabaseconnection.h \
ensdatabaseentry.h ensdata.h ensdensity.h ensditag.h ensembl.h \
ensexon.h ensexternaldatabase.h ensfeature.h ensgene.h ensgvdata.h \
ensgvindividual.h ensgvpopulation.h ensgvsample.h ensintron.h \
enskaryotype.h ensmapper.h ensmarker.h ensmetacoordinate.h \
ensmetainformation.h ensmiscellaneous.h ensprediction.h \
ensprojectionsegment.h ensqcalignment.h ensqcdatabase.h \
ensqc.h ensqcsequence.h ensregistry.h ensrepeat.h ensseqregion.h \
enssequenceedit.h enssequence.h ensslice.h ensstorable.h \
enstable.h enstranscript.h enstranslation.h ensutility.h \
ensvariation.h

libensembl_la_SOURCES = $(ENSEMBLSRC)

VERS_INF = -version-info 6:2:0

if ISCYGWIN
LINKFLAGS = $(VERS_INF) $(CYGWIN_LDFLAGS) $(CYGWIN_LDENSEMBL)
else
LINKFLAGS = $(VERS_INF)
endif

libensembl_la_LDFLAGS = $(LINKFLAGS)
