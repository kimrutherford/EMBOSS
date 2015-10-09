## Process this file with automake to produce Makefile.in

lib_LTLIBRARIES = libensembl.la

libensembl_la_CFLAGS = $(WARN_CFLAGS) $(DEVWARN_CFLAGS) $(JAVA_CFLAGS)

libensembl_la_CPPFLAGS  = -I$(top_srcdir)/ajax/pcre
if !ESYSTEMLIBS
libensembl_la_CPPFLAGS += -I$(top_srcdir)/ajax/expat
libensembl_la_CPPFLAGS += -I$(top_srcdir)/ajax/zlib
endif
libensembl_la_CPPFLAGS += -I$(top_srcdir)/plplot
libensembl_la_CPPFLAGS += $(JAVA_CPPFLAGS)

libensembl_la_LDFLAGS  = -version-info 6:4:0
if ISCYGWIN
libensembl_la_LDFLAGS += -no-undefined
libensembl_la_LDFLAGS += -Wl,--add-stdcall-alias
libensembl_la_LDFLAGS += -Wl,--export-all-symbols
libensembl_la_LDFLAGS += -Wl,--enable-auto-import
libensembl_la_LDFLAGS += -L../../plplot
libensembl_la_LDFLAGS += -L../core
libensembl_la_LDFLAGS += -L../expat
libensembl_la_LDFLAGS += -L../pcre
libensembl_la_LDFLAGS += -lajax
libensembl_la_LDFLAGS += -leplplot
libensembl_la_LDFLAGS += -leexpat
libensembl_la_LDFLAGS += -lepcre
endif

libensembl_la_SOURCES = \
ensalign.c \
ensanalysis.c \
ensassembly.c \
ensassemblyexception.c \
ensassemblymapper.c \
ensattribute.c \
ensbaseadaptor.c \
enscache.c \
enscoordsystem.c \
ensdatabaseadaptor.c \
ensdatabaseconnection.c \
ensdatabaseentry.c \
ensdensity.c \
ensditag.c \
ensexon.c \
ensexternaldatabase.c \
ensfeature.c \
ensgene.c \
ensgvallele.c \
ensgvattribute.c \
ensgvbaseadaptor.c \
ensgvdatabaseadaptor.c \
ensgvgenotype.c \
ensgvindividual.c \
ensgvpopulation.c \
ensgvsample.c \
ensgvsource.c \
ensgvsynonym.c \
ensgvvariation.c \
ensintron.c \
enskaryotype.c \
ensmapper.c \
ensmarker.c \
ensmetacoordinate.c \
ensmetainformation.c \
ensmiscellaneous.c \
ensontology.c \
ensprediction.c \
ensprojectionsegment.c \
ensprotein.c \
ensqc.c \
ensqcalignment.c \
ensqcdasfeature.c \
ensqcdatabase.c \
ensqcsequence.c \
ensqcsubmission.c \
ensqcvariation.c \
ensregistry.c \
ensrepeat.c \
ensseqregion.c \
enssequence.c \
enssequenceedit.c \
enssimple.c \
ensslice.c \
enssoftware.c \
ensstorable.c \
enstable.c \
enstranscript.c \
enstranslation.c \
ensutility.c \
ensvariation.c

include_HEADERS = \
ensalign.h \
ensanalysis.h \
ensassemblyexception.h \
ensassembly.h \
ensassemblymapper.h \
ensattribute.h \
ensbaseadaptor.h \
enscache.h \
enscoordsystem.h \
ensdatabaseadaptor.h \
ensdatabaseconnection.h \
ensdatabaseentry.h \
ensdata.h \
ensdensity.h \
ensditag.h \
ensembl.h \
ensexon.h \
ensexternaldatabase.h \
ensfeature.h \
ensgene.h \
ensgvallele.h \
ensgvattribute.h \
ensgvbaseadaptor.h \
ensgvdata.h \
ensgvdatabaseadaptor.h \
ensgvgenotype.h \
ensgvindividual.h \
ensgvpopulation.h \
ensgvsample.h \
ensgvsource.h \
ensgvsynonym.h \
ensgvvariation.h \
ensintron.h \
enskaryotype.h \
ensmapper.h \
ensmarker.h \
ensmetacoordinate.h \
ensmetainformation.h \
ensmiscellaneous.h \
ensontology.h \
ensprediction.h \
ensprojectionsegment.h \
ensprotein.h \
ensqc.h \
ensqcalignment.h \
ensqcdasfeature.h \
ensqcdatabase.h \
ensqcsequence.h \
ensqcsubmission.h \
ensqcvariation.h \
ensregistry.h \
ensrepeat.h \
ensseqregion.h \
enssequenceedit.h \
enssequence.h \
enssimple.h \
ensslice.h \
enssoftware.h \
ensstorable.h \
enstable.h \
enstranscript.h \
enstranslation.h \
ensutility.h \
ensvariation.h
