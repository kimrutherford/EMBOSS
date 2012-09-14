#!/bin/csh -f

set file = $argv[1]
set name = $file:t:r

embossdatadoc.pl $file > ! $name.outdata
embossdocreport.pl $name.outdata
mv embossdocreport.log datadocsingle.report

rm $name\_static.html
rm $name.srsdata
rm $name.outdata
# rm $name.log
rm $name.html
# rm $name.book
