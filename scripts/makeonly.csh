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

cd emboss
echo "make $app"
make $app
echo "install $app"
cp $app $embossinst/bin
cp acd/$app.acd $embossinst/share/EMBOSS/acd/
