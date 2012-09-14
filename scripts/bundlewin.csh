#!/bin/csh

# for running under cygwin

# with EMBOSS under ~/devemboss

# and emboss-ms under ~/emboss-ms
# cd ~/

# save data files

setenv EMBOSS_DATA ~/devemboss/emboss/data

cd ~/datafiles

echo "Updating datafiles from ~/datafiles/*"

echo "AAINDEX (nakai)"
rm ~/devemboss/emboss/data/AAINDEX/[a-z]*[0-9]
aaindexextract nakai/aaindex1 -auto

echo "JASPAR"
rm -rf ~/devemboss/emboss/data/JASPAR_*/*.pfm
rm -rf ~/devemboss/emboss/data/JASPAR_*/matrix_list.txt
jaspextract jaspar/all_data/FlatFileDir -auto

echo "PRINTS"
rm ~/devemboss/emboss/data/PRINTS/PR*
rm ~/devemboss/emboss/data/PRINTS/prints.mat
printsextract prints/prints[0-9]*.dat -auto

echo "PROSITE"
rm ~/devemboss/emboss/data/PROSITE/PS*
rm ~/devemboss/emboss/data/PROSITE/prosite.lines
prosextract prosite -auto

echo "REBASE"
rm ~/devemboss/emboss/data/REBASE/emboss*.*
rebaseextract rebase/withrefm.[0-9]* rebase/proto.[0-9]* -auto

echo "TRANSFAC"
rm ~/devemboss/emboss/data/tf*
tfextract transfac/site.dat -auto

cd ~/devemboss/scripts

# make sure protection is set correctly on external .exe files

chmod ugo+x ~/devemboss/win32/exes/*.exe

# compile the bundlewin executable

gcc -o bundlewin -O2 bundlewin.c

# set the default location of Visual C++ libraries

setenv vc100files /home/emboss/emboss-ms/redist/vc100
setenv vc90files /home/emboss/emboss-ms/redist/vc90
setenv vc80files /home/emboss/emboss-ms/redist/vc80

if(-d ~/membossdev/win32build) then
  rm -rf ~/membossdev/win32build
endif

if (-s ~/membossdev/memboss-dev.zip) then
  rm ~/membossdev/memboss-dev.zip
endif

# run and accept default (see above) VC++ library locations

./bundlewin /home/emboss/devemboss /home/emboss/membossdev

echo "when ready, unpack with"
echo "cd ~/membossdev"
echo "rename or remove memboss-dev directory"
echo "unzip -q ~/membossdev/memboss-dev.zip"
echo "or unzip in windows by opening the memboss-dev.zip file"
