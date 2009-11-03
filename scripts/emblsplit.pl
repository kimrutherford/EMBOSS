#!/usr/bin/perl -w

$file = shift @ARGV;

$s = (-s $file);

$maxs = 1900000000;
if($s < $maxs) {
    print "File '$file' size $s is under $maxs\n";
    exit();
}

($fpre,$fpost) = ($file =~ /(.*)[.]([^.]+)/);

$cnt=1;
open (IN, $file) || die "Cannot open input file";
open (OUT, ">$fpre$cnt.$fpost") || die "cannot open output file $cnt";

$entry = "";
while (<IN>) {
    if(/^ID   /) {
	if ($entry ne "") {
	    if(tell(OUT) > $maxs) {
		close(OUT);
		$cnt++;
		open (OUT, ">$fpre$cnt.$fpost") ||
		    die "cannot open output file $cnt";
	    }
	    print OUT "$entry";
	    $entry = "";
	}
    }
    $entry .= $_;
}
print OUT "$entry";
close OUT;
