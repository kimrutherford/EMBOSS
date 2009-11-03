#!/usr/bin/perl -w

$name = shift @ARGV;
open(ACD, "$ENV{HOME}/local/share/EMBOSS/acd/$name.acd") || die "Cannot open $name.acd";
while(<ACD>) {
    if(/^\s+embassy:\s+"([^\"]+)"\s*$/) {$embassyname = $1;}
}
close ACD;

$cnt=0;
while (<>) {
    if (!$cnt++) {
	if(defined($embassyname)) {print "+$name ($embassyname)\n"}
	else {print "$name\n"}
    }
    s/ \S+\/([^\/.]+[.]acd) / $1 /;
    print;
}

if ($cnt) {print "\n"}
