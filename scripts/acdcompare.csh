#!/bin/csh

cd ~/devemboss/emboss/acd
foreach x (*.acd)
    echo "CVS $x:r"
    grep ':.*\[' $x | grep -v 'section:' >&! ~/acdcompare/$x:r.cvs
end

cd /ebi/industry/pmr/EMBOSS-2.6.0/emboss/acd
foreach x (*.acd)
    echo "260 $x:r"
    grep ':.*\[' $x | grep -v 'section:' | ~/devemboss/acdfix.pl >&! ~/acdcompare/$x:r.260
    diff -w ~/acdcompare/$x:r.cvs ~/acdcompare/$x:r.260 >&! ~/acdcompare/$x:r.diff
    cat ~/acdcompare/$x:r.diff
end
