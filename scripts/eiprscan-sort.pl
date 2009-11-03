#!/usr/bin/perl -w

$filename = shift @ARGV;

open (IN, $filename) || die "failed to open '$filename'";

@outfields = ("BlastProDom", "Coil", "Gene3D", "HMMPanther",
	      "HMMPfam", "HMMSmart", "HMMTigr", "FPrintScan", "ScanRegExp",
	      "ProfileScan", "superfamily", "Seg", "SignalP", "TmHmm");

while (<IN>) {
    @fields = split (/\t/);
    $text{$fields[3]} .= $_;
}
close IN;

open (OUT, ">$filename")  || die "failed to open '$filename'";
foreach $f (@outfields) {
    if(defined($text{$f})) {
	print OUT "$text{$f}";
    }
    $text{$f} = "";
}

foreach $f (sort(keys(%text))) {
    if($text{$f} ne "") {
	print OUT "$text{$f}";
    }
}
close OUT;
