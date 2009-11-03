#!/bin/csh

../emboss/entrails -auto -full entrails-full.txt 
grep 'ajNamGetValue' ../ajax/*.c >! acdsyntax.getvalue
../scripts/acdstats.pl >! acdsyntax.acdstats 
(../scripts/acdsyntax.pl entrails-full.txt >! acdsyntax.out) \
	>&! acdsyntax.error
cp test.html ../doc/manuals/acd/syntax.html
cp content.html ../doc/manuals/acd/content.html
#rm acdsyntax.getvalue
