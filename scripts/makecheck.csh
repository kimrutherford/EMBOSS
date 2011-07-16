#!/bin/csh -f


set app = $argv[1]

foreach embosshome (`embossversion -full -auto| grep '^BaseDirectory'`)
#  echo "$embosshome"
end

foreach embossinst (`embossversion -full -auto| grep '^InstallDirectory'`)
#  echo "$embossinst"
end
#echo "install: $embossinst"
#echo "   home: $embosshome"

cd $embosshome

cd plplot
echo "make PLPLOT"
make install |& egrep '^[^ =\[]*:'
cd ../ajax 
echo "make AJAX"
make install |& egrep '^[^ =\[]*:'
cd ../nucleus
echo "make NUCLEUS"
make install |& egrep '^[^ =\[]*:'
cd ../emboss
echo "make $app"
make $app |& egrep '^[^ =\[]*:'
echo "install $app"
/bin/sh ../libtool --mode=install /usr/bin/install -c $app /homes/pmr/check/bin/$app
cp acd/$app.acd $embossinst/share/EMBOSS/acd/
