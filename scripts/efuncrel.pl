#!/usr/bin/perl -w

$filename = shift @ARGV;
#print "$filename\n";
$mod = 0;
$alltext = "";
open (IN, "$filename") || die "Cannot read from $filename";
while (<IN>) {
    if(/EFUNC[^R]/) {
	$mod++;
	s/EFUNC([^R])/EFUNCREL$1/g;
    }
    if(/EDATA[^R]/) {
	$mod++;
	s/EDATA([^R])/EDATAREL$1/g;
    }
    $alltext .= $_;
}
close IN;

if($mod) {
    open (OUT, ">$filename") || die "Cannot write to $filename";
    print OUT $alltext;
    close OUT;
    print "$mod lines changed: $filename\n";
}

exit();
