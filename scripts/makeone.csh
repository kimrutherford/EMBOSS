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
make -j 4 install |& egrep '^[^ =\[]*:' |& grep -v '^libtool: '
cd ../ajax 
echo "make AJAX"
make -j 4 install |& egrep '^[^ =\[]*:' |& grep -v '^libtool: '
cd ../nucleus
echo "make NUCLEUS"
make -j 4 install |& egrep '^[^ =\[]*:' |& grep -v '^libtool: '
cd ../emboss
echo "make $app"
make -j 4 $app |& egrep '^[^ =\[]*:' |& grep -v '^libtool: '
echo "install $app"
echo "/bin/sh ../libtool --mode=install /usr/bin/install -c $app $embossinst/$app"
/bin/sh ../libtool --mode=install /usr/bin/install  -c $app $embossinst/bin/$app
cp acd/$app.acd $embossinst/share/EMBOSS/acd/
