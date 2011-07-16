#!/bin/csh -f


set app = $argv[1]
set script = `which makeone.csh`
#set embosshome = $script:h/..

foreach embosshome (`embossversion -full -auto| grep '^BaseDirectory'`)
#  echo "$embosshome"
end

foreach embosssystem (`embossversion -full -auto| grep '^System:'`)
#  echo "$embosssystem"
end

set rundir = `which embossversion`
set embossint = $rundir:h

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
if ($embosssystem == "CYGWIN_NT-5.1") then
make -j 4 $app.exe |& egrep '^[^ =\[]*:' |& grep -v '^libtool: '
else
make -j 4 $app |& egrep '^[^ =\[]*:' |& grep -v '^libtool: '
endif
echo "install $app"
echo "/bin/sh ../libtool --mode=install /usr/bin/install -c $app $embossinst/$app"
/bin/sh ../libtool --mode=install /usr/bin/install  -c $app $embossinst/bin/$app
cp acd/$app.acd $embossinst/share/EMBOSS/acd/
