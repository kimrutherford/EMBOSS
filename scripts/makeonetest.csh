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
make install |& egrep '^[^ =\[]*:' |& grep -v '^libtool: '
cd ../ajax 
echo "make AJAX"
make install |& egrep '^[^ =\[]*:' |& grep -v '^libtool: '
cd ../nucleus
echo "make NUCLEUS"
make install |& egrep '^[^ =\[]*:' |& grep -v '^libtool: '
cd ../emboss
echo "make $app"
make $app |& egrep '^[^ =\[]*:' |& grep -v '^libtool: '
if(-e ~/check/bin/$app) then
echo "install $app"
echo "/bin/sh ../libtool --mode=install /usr/bin/install  -c $app ~/check/bin/$app"
/bin/sh ../libtool --mode=install /usr/bin/install  -c $app ~/check/bin/$app
else
#echo "install $app"
#echo "/bin/sh ../libtool --mode=install /usr/bin/install -c $app $embossinst/$app"
#/bin/sh ../libtool --mode=install /usr/bin/install  -c $app $embossinst/bin/$app
endif
