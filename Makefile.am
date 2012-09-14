## Process this file with automake to produce Makefile.in

ACLOCAL_AMFLAGS = -I m4

AUTOMAKE_OPTIONS = gnits

EXTRA_DIST = \
COMPAT \
FAQ \
KNOWN_BUGS \
LICENSE \
ONEWS \
PROBLEMS

SUBDIRS = \
plplot \
ajax \
nucleus \
emboss \
test \
doc \
jemboss

# tar to pick up the other directories
# then remove any CVS subdirectories

dist-hook:
	tar cBf - scripts | ( cd $(distdir); tar xBf - ; find scripts -name CVS | xargs rm -rf )
	tar cBf - test | ( cd $(distdir); tar xBf - ; rm -rf test/CVS test/*/CVS test/*/*/CVS )
###	tar cBf - emboss/acd | ( cd $(distdir); tar xBf - ; cd emboss/acd; rm -rf CVS )
###	tar cBf - emboss/data | ( cd $(distdir); tar xBf - ; cd emboss/data; rm -rf CVS */CVS */*/CVS )
###	tar cBf - plplot/lib | ( cd $(distdir); tar xBf - ; find plplot -name CVS | xargs rm -rf )
	tar cBf - doc | ( cd $(distdir); tar xBf - ; find doc -name CVS | xargs rm -rf; )
	tar cBf - jemboss | ( cd $(distdir); tar xBf - ; find jemboss -name CVS | xargs rm -rf; find jemboss -name Makefile | xargs rm -rf; find jemboss -name .cvsignore | xargs rm -rf )

install-exec-hook:
	echo ""
	echo "Running $(bindir)/embossupdate"
	$(bindir)/embossupdate