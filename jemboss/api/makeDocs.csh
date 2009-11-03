#!/bin/csh
/bin/rm Package-org *html stylesheet.css org/emboss/jemboss/* org/emboss/jemboss/gui/* org/emboss/jemboss/soap/* org/emboss/jemboss/programs/* org/emboss/jemboss/parser/* org/emboss/jemboss/gui/form/* org/emboss/jemboss/gui/sequenceChooser/* org/emboss/jemboss/parser/acd/* org/emboss/jemboss/gui/filetree/* org/emboss/jemboss/gui/startup/*  org/emboss/jemboss/editor/*

setenv LIB ../lib

javadoc -author -version -windowtitle "Jemboss API" -classpath $LIB/axis/axis.jar:$LIB/axis/jaxrpc.jar:$LIB/axis/saaj.jar:$LIB/axis/commons-logging.jar:$LIB/axis/commons-discovery.jar:$LIB/jakarta-regexp-1.2.jar:$LIB/jalview.jar:$LIB/jnet.jar:$LIB/jcert.jar:$LIB/jsse.jar:../:$LIB/axis/servlet.jar:$LIB/xerces.jar:$LIB/grout.jar -private \
 org.emboss.jemboss org.emboss.jemboss.editor org.emboss.jemboss.gui \
 org.emboss.jemboss.gui.filetree org.emboss.jemboss.gui.form \
 org.emboss.jemboss.gui.sequenceChooser org.emboss.jemboss.gui.startup \
 org.emboss.jemboss.parser org.emboss.jemboss.parser.acd \
 org.emboss.jemboss.programs org.emboss.jemboss.server \
 org.emboss.jemboss.soap

