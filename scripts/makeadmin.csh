#!/bin/csh -f

# Uses admin.tex and emboss.sty

latex admin

# creates:
# admin.aux ? some kind of auxilliary file
# admin.dvi device independent tex output
# admin.log logfile
# admin.toc table of contents file

# create postscript file admin.ps
# stupid dvips by default sends to the printer and deletes!
dvips -o admin.ps admin

# create PDF file admin.pdf
dvipdf admin

# cd to where the source will be built
# copy admin.tex, admin.aux and emboss.perl there
# -local_icons stops it linking up, down, next to some silly install directory

latex2html -local_icons admin

# makes HTML files in the admin directory
