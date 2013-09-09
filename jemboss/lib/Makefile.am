## Process this file with automake to produce Makefile.in

SUBDIRS = axis

jembosslibdir = $(pkgdatadir)/jemboss/lib

jembosslib_DATA = \
README \
activation.jar \
jakarta-regexp-1.2.jar \
mail.jar \
jalviewApplet.jar \
jemboss.jar

jemboss.jar:../org/emboss/jemboss/server/JembossFile*Server.java
if JAVA_BUILD
	$(ANT) -f ../build.xml package -Demboss.data=../emboss/data -Dname=jemboss -Djar.dir=lib -Dnotformemboss=yes;
endif
