#!/usr/bin/perl -w

$checkinst = "/homes/pmr/check/bin/";

open (VERSION, "embossversion -full -auto|") || die "Unable to run embossversion";
while(<VERSION>) {
    if(/^InstallDirectory:\s+(\S+)/) {
	$embossinst = "$1/bin/";
    }
}
close VERSION;

%ignore = ("emnu" => "stdin input",
	   "mse" => "stdin input",
           "fretree" => "stdin input"
);

%count = ();

%unknownapps = ();

while (<>) {
    if(/^[\#]/){
	next;
    }
    else {
	if(/^(\S+)/) {$app = $1}
	else {$app = "qatest"}
	if(defined($ignore{$app})) {next}
	$num = ++$count{$app};
	if(-e "$embossinst/$app") {
	    s/^(\S+) +/$1-qa$num = $1 /;
	}
	elsif (-e "$checkinst/$app") {
	    s/^(\S+) +/$1-qa$num =test= $1 /;
	}
	else {
	    if(!defined($unknownapps{$app})) {
		print STDERR "Unknown application $app\n";
		$unknownapps{$app}=0;
	    }
	    $unknownapps{$app}++;
	}
	s/[\[](-[^\]]+)[\]]//gos;
	s/([ ,:\@])\.\.\/([^.])/$1..\/..\/qa\/$2/gos;
#	s/([ ,:\@])\.\.\/\.\.\/([^.])/$1..\/$2/gos;
	s/  / /gos;
    }
    s/ -auto Y//;
    s/ -auto//;
    s/$/ -auto/;
    print;
}

foreach $a (sort(keys(%unknownapps))) {
    print STDERR "Unknown $unknownapps{$a} times '$a'\n";
}
