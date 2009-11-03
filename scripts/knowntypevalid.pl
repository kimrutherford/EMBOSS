#!/usr/bin/perl -w
$lasttype = "";
open (VERS, "embossversion -full -auto|") || die "Cannot run embossversion";
while (<VERS>) {
    if(/InstallDirectory: +(\S+)/) {
	$installtop = $1;
	$installtop =~ s/\/$//;
    }
    if(/BaseDirectory: +(\S+)/) {
	$distribtop = $1;
	$distribtop =~ s/\/$//;
    }
}
close VERS;
open (KNOWN, "$installtop/share/EMBOSS/acd/knowntypes.standard") ||
    die "Unable to open $installtop/share/EMBOSS/acd/knowntypes.standard";
while(<KNOWN>) {
    if(/^\#/) {next}
    if(/^(\S+\s+)(\S+\s+)(.*)/) {
	$kt = lc($1);
	if($kt eq $lasttype) {
	    print "$kt: Duplicate known type\n";
	}
	elsif($kt lt $lasttype) {
	    print "$kt: out of order 'follows '$lasttype'\n";
	}
	$lasttype = $kt;
	if($3 !~ /^[A-Z]/) {
	    print "$kt: Description does not start with capital letter\n";
	}
	if($3 !~ / file$/) {
	    print "$kt: Description ends with 'file'\n";
	}
    }
}
close KNOWN;

exit;
