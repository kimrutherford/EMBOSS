pkgdata_DATA = Block.java   PrintDNAImage.java \
DNADraw.java                RestrictionEnzyme.java \
EmbossCirdnaReader.java     Ticks.java \
GeneticMarker.java          Wizard.java \
LineAttribute.java

pkgdatadir=$(prefix)/share/$(PACKAGE)/jemboss/org/emboss/jemboss/draw

install-exec-hook:
	@if test "${JAVA_OK}" != "yes" ; then \
	  echo "No class files to copy"; \
	else \
	  $(top_srcdir)/mkinstalldirs $(DESTDIR)$(prefix)/share/$(PACKAGE)/jemboss/org/emboss/jemboss/draw; \
	for i in *.class; do \
	  ${INSTALL_DATA} $$i $(DESTDIR)$(prefix)/share/$(PACKAGE)/jemboss/org/emboss/jemboss/draw; \
	done; \
	fi
