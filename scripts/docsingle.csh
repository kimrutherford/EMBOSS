#!/bin/csh -f

set file = $argv[1]
set name = $file:t:r

embossdoc.pl $file > ! $name.out
embossdocreport.pl $name.out
mv embossdocreport.log docsingle.report

rm $name\_static.html
rm $name.srs
rm $name.out
rm $name.log
rm $name.html
rm $name.book
rm deprecated.new

