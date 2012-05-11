#!/bin/sh -e
# Copyleft 2007 Charles Plessy, do what you want with this trivial file.

PACKAGE=$(dpkg-parsechangelog -ldebian/changelog | grep Source: | cut -f2 -d' ')
VERSION=$(dpkg-parsechangelog -ldebian/changelog | grep Version: | cut -f2 -d' ' | cut -f1 -d-)
ORIGTARGZ=../${PACKAGE}_${VERSION}.orig.tar.gz

rm -f debian/$PACKAGE.manpages

for FILE in $(ls emboss/acd/*.acd)
  do
  ACD=$(basename $FILE .acd)
  ./debian/acd2docbook.pl $FILE ${VERSION} \
  | xsltproc --nonet --param man.charmap.use.subset 0 -o debian/manpages/ -
  echo "debian/manpages/$ACD.1e" >> debian/$PACKAGE.manpages
done
