SUBDIRS = axis

# The install-exec-hook is for the case where a client.jar has
# been created by the 'ant' packaging for a standalone Jemboss.
# In that case it needs to be copied to the install dir.

install-exec-hook:
	if test -f client.jar ; then \
	    (if test -d $(DESTDIR)$(prefix)/share/$(PACKAGE)/jemboss/lib ; then \
	        echo "jemboss/lib exists"; \
	     else \
	        mkdir $(DESTDIR)$(prefix)/share/$(PACKAGE)/jemboss/lib; \
	     fi); \
	cp client.jar $(DESTDIR)$(prefix)/share/$(PACKAGE)/jemboss/lib; \
	fi;


pkgdata_DATA = README activation.jar jakarta-regexp-1.2.jar \
mail.jar jalview.jar \
JembossPrintAlignment.jar grout.jar

pkgdatadir=$(prefix)/share/$(PACKAGE)/jemboss/lib
