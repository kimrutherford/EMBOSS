pkgdata_DATA = AlignApplet.java              GroutSequenceJPanel.java \
AlignJFrame.java                             Matrix.java \
ColourJFrame.java                            MatrixJFrame.java \
ColourMenu.java                              PatternJFrame.java \
ColourPanel.java                             PlotConsensus.java \
Consensus.java                               PrettyPlotJFrame.java \
ConsensusOptions.java                        PrintAlignmentImage.java \
GraphicSequenceCollection.java               PrintAlignment.java \
GroutAlignJFrameEvent.java                   Sequence.java \
GroutAlignJFrame.java                        SequenceJPanel.java \
GroutAlignJFrameListener.java                SequenceNameJButton.java \
GroutGraphicSequenceCollectionEvent.java     SequenceProperties.java \
GroutGraphicSequenceCollection.java          SequenceReader.java \
GroutGraphicSequenceCollectionListener.java  SequenceSaver.java \
GroutSequence.java IDTableJFrame.java        PrintableJTable.java \
Paste.java

pkgdatadir=$(prefix)/share/$(PACKAGE)/jemboss/org/emboss/jemboss/editor

install-exec-hook:
	@if test "${JAVA_OK}" != "yes" ; then \
	  echo "No class files to copy"; \
	else \
	  $(top_srcdir)/mkinstalldirs $(DESTDIR)$(prefix)/share/$(PACKAGE)/jemboss/org/emboss/jemboss/editor; \
	for i in *.class; do \
	  ${INSTALL_DATA} $$i $(DESTDIR)$(prefix)/share/$(PACKAGE)/jemboss/org/emboss/jemboss/editor; \
	done; \
	fi
