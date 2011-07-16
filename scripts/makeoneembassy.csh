#!/bin/csh -f


set pack = $argv[1]
set app = $argv[2]

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
cd ../embassy/$pack/src
if ($pack == 'vienna') then
  echo "make libovienna"
  make liboviennarna.la |& egrep '^[^ =\[]*:' |& grep -v '^libtool: '
  make install-libLTLIBRARIES
endif
if ($pack == 'vienna2') then
  echo "make libvienna"
  make libviennarna.la |& egrep '^[^ =\[]*:' |& grep -v '^libtool: '
  make install-libLTLIBRARIES
endif
echo "make $app"
make $app |& egrep '^[^ =\[]*:' |& grep -v '^libtool: '
if ($pack == 'myembossdemo') then
  echo "copy $app to check/bin"
  cp $app ~/check/bin/$app
else
echo "install $app"
echo "/bin/sh ../libtool --mode=install /usr/bin/install -c $app $embossinst/$app"
/bin/sh ../libtool --mode=install /usr/bin/install  -c $app $embossinst/bin/$app
cp ../emboss_acd/$app.acd $embossinst/share/EMBOSS/acd/
endif
