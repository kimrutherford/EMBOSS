#!/usr/bin/perl -w



sub testfile($$$$$) {
    my($cvse,$master,$compare,$dir,$file) = @_;
    if(!(-e "$cvse/$master/$dir/$file")) {
	print "No such file for master $cvse/$master/$dir/$file\n";
	return 0;
    }
    if(!(-e "$cvse/$compare/$dir/$file")) {
	print "No such file for '$compare' $cvse/$master/$dir/$file\n";
	return 0;
    }
    system "diff $cvse/$master/$dir/$file $cvse/$compare/$dir/$file > x.test";
    if(-s "x.test") {
	$ok = 1;
	open(X, "x.test");
	$block = "";
	$savefile = "";
	$badblock = 0;
	while(<X>) {
	    if(/File \S+ differs:/) {next}
	    if(/^\d+(,\d+)?([acd])\d+(,\d+)?/) {
		if($badblock) {$savefile .= $block}
		$type = $2;
		$badblock = 0;
		$block = "";
	    }
	    elsif(/^[<>] AC_INIT[\(](source|src)\/\w+[.][ch][\)]$/) {
	    }
	    elsif(/^[<>] AM_INIT_AUTOMAKE[\(]\w+, [\d.]+[a-z]?[\)]$/) {
	    }
	    elsif(/^[<>]\s+$/ && $type eq "c") {
	    }
	    elsif(/^---$/) {
	    }
	    elsif(/^[<>] AC_OUTPUT[\(]Makefile .*$/) {
	    }
	    else {
		$ok = 0;
		$badblock = 1;
#		print "bad block $_";
	    }
	    $block .= $_;
	}
	if($badblock) {$savefile .= $block}
	close X;
	if(!$ok) {
	    print "File $file differs:\n";
	    print $savefile;
	    return 0;
	}
    }
    system "rm x.test";
    return 1;
}

# where the original distribution lives
my $distribtop = "./";

# where the installed package lives
my $installtop = "./";

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

my $cvse = "$distribtop/embassy";

my $masterembassy = "appendixd";
my @embassylist = ("appendixd",
		   "domainatrix",
		   "domalign",
		   "domsearch",
		   "emnu",
		   "esim4",
		   "hmmer",
		   "hmmernew",
		   "meme",
		   "memenew",
		   "mse",
		   "myemboss", # we avoid documenting these examples
		   "phylip",
		   "phylipnew",
		   "signature",
		   "structure",
		   "topo",
		   "vienna",
		   );


foreach $e (@embassylist) {
    if($e eq $masterembassy) {next}
    print "$e:\n";
    testfile($cvse, $masterembassy, $e, "m4", "amd64.m4");
    testfile($cvse, $masterembassy, $e, "m4", "general.m4");
    testfile($cvse, $masterembassy, $e, "m4", "grout.m4");
    testfile($cvse, $masterembassy, $e, "m4", "java.m4");
    testfile($cvse, $masterembassy, $e, "m4", "pngdriver.m4");
    testfile($cvse, $masterembassy, $e, "m4", "sgi.m4");
    testfile($cvse, $masterembassy, $e, "", "configure.in");
}
