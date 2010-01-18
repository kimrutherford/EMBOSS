#!/bin/csh -f

if ($#argv != 3) then
  echo "usage:"
  echo "embossdoc.csh srctop wwwtop booktop"
endif

set edir = $argv[1]
set wdir = $argv[2]
set bdir = $argv[3]


\rm -rf x/
mkdir x
cd x

echo >! ../efunc.out
echo >! ../efunc.check
echo >! ../edata.out
echo >! ../edata.check

touch $wdir/ajax/dummy.html
foreach x ($wdir/ajax/*html $wdir/nucleus/*html \
           $wdir/datadef/*html $wdir/datadef/*empty $wdir/appsource/*html)
#    echo "'$x:t'"
    if ($x:t == "index.html") then
        echo "Preserve index $x"
    else if ($x:t == "other.html") then
        echo "Preserve other $x"
    else
        \rm $x
    endif
end

#echo "Ajax functions"
# ... skip pcre
foreach x ($edir/ajax/core/*.c $edir/ajax/graphics/*.c $edir/ajax/ajaxdb/*.c  $edir/ajax/acd/*.c $edir/ajax/ensembl/*.c)
  embossdoccheck.pl $x >> ../efunc.check
  embossdoc.pl $x >> ../efunc.out
end
cat *.srs >! ../efunc.dat
\cp *html $wdir/ajax/

foreach x (*.book)
  if(-s $x) then
    \cp $x $bdir/ajax/
  else
    echo "$x empty"
    \rm $x
  endif
end

\rm *html
\rm *.srs

#echo "Ajax static datatypes"
foreach x ($edir/ajax/core/*.c $edir/ajax/graphics/*.c $edir/ajax/ajaxdb/*.c  $edir/ajax/acd/*.c $edir/ajax/ensembl/*.c)
  embossdatacheck.pl $x >> ../edata.check
  embossdatadoc.pl $x >> ../edata.out
end
cat *.srsdata >! ../edata.dat
foreach x (*html)
  if (-e $wdir/datadef/$x) then
    echo "Warning: Ajax overwriting $wdir/datadef/$x"
  endif
  \cp $x $wdir/datadef/
end
cp *.empty $wdir/datadef/

\rm *html
\rm *.srsdata
\rm *.empty

#echo "Nucleus functions"
foreach x ($edir/nucleus/*.c)
  embossdoccheck.pl $x >> ../efunc.check
  embossdoc.pl $x >> ../efunc.out
end
cat *.srs >> ../efunc.dat
\cp *html $wdir/nucleus/

\rm *html
\rm *.srs

#echo "Nucleus static datatypes"
foreach x ($edir/nucleus/*.c)
  embossdatacheck.pl $x >> ../edata.check
  embossdatadoc.pl $x >> ../edata.out
end
cat *.srsdata >> ../edata.dat
foreach x (*html)
  if (-e $wdir/datadef/$x) then
    echo "Warning: Nucleus overwriting $wdir/datadef/$x"
  endif
  \cp $x $wdir/datadef/
end
cp *.empty $wdir/datadef/

\rm *html
\rm *.srsdata
\rm *.empty

#echo "Emboss functions"
foreach x ($edir/emboss/*.c)
  embossdoccheck.pl $x >> ../efunc.check
  embossdoc.pl $x >> ../efunc.out
end
cat *.srs >> ../efunc.dat
\cp *html $wdir/appsource/

\rm *html
\rm *.srs

#echo "Emboss static datatypes"
foreach x ($edir/emboss/*.c)
  embossdatacheck.pl $x >> ../edata.check
  embossdatadoc.pl $x >> ../edata.out
end
cat *.srsdata >> ../edata.dat
foreach x (*html)
  if (-e $wdir/datadef/$x) then
    echo "Warning: Emboss overwriting $wdir/datadef/$x"
  endif
  \cp $x $wdir/datadef/
end

\rm *html
\rm *.srsdata

#echo "Ajax datatypes"
foreach x ($edir/ajax/core/*.h $edir/ajax/graphics/*.h $edir/ajax/ajaxdb/*.h  $edir/ajax/acd/*.h $edir/ajax/ensembl/*.h)
  embossdatacheck.pl $x >> ../edata.check
  embossdatadoc.pl $x >> ../edata.out
end
cat *.srsdata >> ../edata.dat
foreach x (*html)
  if (-e $wdir/datadef/$x) then
    if (-e $wdir/datadef/$x:r.empty) then
#      echo "Note: Ajax headers overwriting $wdir/datadef/$x is empty"
      \cp $x $wdir/datadef/
    else if  (-e $x:r.empty) then
#      echo "Note: Ajax header $x is empty - keep previous"
#      ls -al $x $x:r.empty $wdir/datadef/$x:r.empty $wdir/datadef/$x
    else
      echo "Warning: Ajax headers overwriting $wdir/datadef/$x"
      \cp $x $wdir/datadef/
    endif
  else
    \cp $x $wdir/datadef/
  endif
end

\rm *html
\rm *.srsdata
\rm *.empty

#echo "Nucleus datatypes"
foreach x ($edir/nucleus/*.h)
  embossdatacheck.pl $x >> ../edata.check
  embossdatadoc.pl $x >> ../edata.out
end
foreach x (*html)
  if (-e $wdir/datadef/$x) then
    if (-e $wdir/datadef/$x:r.empty) then
#      echo "Note: Nucleus headers overwriting $wdir/datadef/$x is empty"
      \cp $x $wdir/datadef/
    else if  (-e $x:r.empty) then
#      echo "Note: Nucleus header $x is empty - keep previous"
#      ls -al $x $x:r.empty $wdir/datadef/$x:r.empty $wdir/datadef/$x
    else
      echo "Warning: Nucleus headers overwriting $wdir/datadef/$x"
      \cp $x $wdir/datadef/
    endif
  else
    \cp $x $wdir/datadef/
  endif
end
cat *.srsdata >> ../edata.dat

\rm *html
\rm *.srsdata
\rm *.empty

cp deprecated.new ~/devemboss/deprecated.txt
cd ..
\rm -rf x/

# Check the categories are all defined

embossdoccategories.pl efunc.out edata.out >! ecat.out

#source ~/srsfunc/etc/prep_srs
#
#srsbuild efunc -nn
#srsbuild efunc -rel '2.9.0'
#
#srsbuild edata -nn
#srsbuild edata -rel '2.9.0'
#
#srsbuild -l efunc
#srsbuild -l edata
