#!/bin/csh

# Directory should be empty before running

cd ~/sfemboss/doc/
rm -rf currentsourceforge
mkdir currentsourceforge

scp -r "peterrice@shell.sourceforge.net:emboss/*" currentsourceforge

diff -r  currentsourceforge ~/sfdoc >! ~/out/sourceforgecompare.txt
grep '^Only in currentsourceforge[/:]' ~/out/sourceforgecompare.txt >! ~/out/sourceforgeonly.txt
grep '^Only in sourceforge[/:]' ~/out/sourceforgecompare.txt | grep -v ' CVS$' >! ~/out/localonly.txt
grep '^diff'  ~/out/sourceforgecompare.txt >! ~/out/sourceforgediff.txt

