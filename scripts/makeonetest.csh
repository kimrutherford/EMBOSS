#!/bin/csh -f


set app = $argv[1]

#set embosshome = ~/cvsemboss
#set embossinst = ~/local

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
make install |& egrep '^[^ =\[]*:' |& egrep 'error:'
cd ../nucleus
echo "make NUCLEUS"
make install |& egrep '^[^ =\[]*:' |& egrep 'error:'
cd ../emboss
echo "make $app"
make $app |& egrep '^[^ =\[]*:' |& egrep 'error:|undefined'
if(-e ~/check/bin/$app) then
echo "install $app"
echo "/bin/sh ../libtool --mode=install /usr/bin/install  -c $app ~/check/bin/$app"
  cp $app ~/check/bin/$app
else
echo "install $app"
echo "/bin/sh ../libtool --mode=install /usr/bin/install -c $app $embossinst/$app"
/bin/sh ../libtool --mode=install /usr/bin/install  -c $app $embossinst/bin/$app
endif
cp acd/$app.acd $embossinst/share/EMBOSS/acd/
