SUBDIRS = axis

clean-local:
if EHAVEANT
	rm -f jemboss.jar;
endif

jemboss.jar:../org/emboss/jemboss/server/JembossFile*Server.java
if EHAVEANT
	ant -f ../build.xml package -Demboss.data=../emboss/data -Dname=jemboss -Djar.dir=lib -Dnotformemboss=yes;
endif

pkgdata_DATA = README activation.jar jakarta-regexp-1.2.jar \
mail.jar jalviewApplet.jar jemboss.jar

pkgdatadir=$(prefix)/share/$(PACKAGE)/jemboss/lib
