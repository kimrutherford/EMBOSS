#!/usr/bin/perl -w

@wantid = (1,
	   2759, 6072, 7711, 7742, 7776, 8287,
	   9347, 9443, 9526, 9593, 9597, 9598,
	   9600, 9601, 9603, 9604, 9605, 9606,
	   32523, 32524, 32525, 33154, 33208, 33213, 33316, 33511,
	   40674, 89593, 
	   117570, 117571, 131567,
	   207598,
	   314146, 314293, 314295, 317028, 376913,
	   445518, 445518, 502961, 741158, 
    );

foreach $w (@wantid) {$wantid{$w} = 1}

open (MERGED, "emboss/data/TAXONOMY/merged.dmp");
open (NODES, "emboss/data/TAXONOMY/nodes.dmp");
open (NAMES, "emboss/data/TAXONOMY/names.dmp");

open (OUTMERGED, ">test/data/taxonomy/mergded.dmp");
open (OUTNODES, ">test/data/taxonomy/nodes.dmp");
open (OUTNAMES, ">test/data/taxonomy/names.dmp");

while (<MERGED>) {
    if(/\d+\t\|\t(\d+)/) {
	if($wantid{$1}) {print OUTMERGED}
    }
}
close MERGED;
close OUTMERGED;

while (<NODES>) {
    if(/^(\d+)/) {
	if($wantid{$1}) {print OUTNODES}
    }
}
close NODES;
close OUTNODES;

while (<NAMES>) {
    if(/^(\d+)/) {
	if($wantid{$1}) {print OUTNAMES}
    }
}
close NAMES;
close OUTNAMES;
