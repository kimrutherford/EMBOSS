#!/usr/bin/perl -w


sub processdir($$) {
    my ($dirname,$path) = @_;
    my $file = "";
    my $mypath = $dirname;
    local *DIR;
    if ($path ne "") {$mypath = "$path/$dirname"}
    opendir(DIR, $mypath);
    my %dirfiles = ();
    my %dirdirs = ();
    my @dirlines = ();
    my $hasmake = 0;
    my $ilines = 0;

    my %mysource = ();
    my %mybinary = ();
    my %myfile = ();
    my $f;

    if(-s "$mypath/Makefile.am") {
	$hasmake = 1;
	print "   File $file\n";
	$fileprint = "";
	@dirlines = &processfile("Makefile.am", $mypath); # &avoids prototype recursion warning
	my $name = $prename =  $postname = $x = "";
	foreach $x (@dirlines) {
	    $ilines++;
	    print "$ilines: $x";
	    if($x =~ /^\s*([^_\s]+)_([^_\s]+)\s+[=]\s+(.*)/) {
		$prename = $1;
		$postname = $2;
		@filenames = split(/\s+/, $3);
		if($postname eq "PROGRAMS") {
		    foreach $f (@filenames) {$mybinary{$f}++}
		}
		elsif($postname eq "SOURCES") {
		    foreach $f (@filenames) {$mysource{$f}++}
		}
		elsif($postname eq "HEADERS") {
		    foreach $f (@filenames) {$mysource{$f}++}
		}
		elsif($postname eq "DATA") {
		    foreach $f (@filenames) {$myfile{$f}++}
		}
		elsif($prename eq "EXTRA" && $postname eq "DIST") {
		    foreach $f (@filenames) {$myfile{$f}++}
		}
		elsif($prename eq "AM" && $postname eq "CFLAGS") {
		}
		else {
		    print ".. ignored line: [$prename]_[$postname] $line\n";
		}
		my $n = scalar @filenames;
#		print "$ilines: [$n] $x";
	    }
	    elsif($x =~ /^\s*([^_\s]+)\s+[=]\s+(.*)/) {
		$name = $1;
		@filenames = split(/\s+/, $2);
		if($name eq "SUBDIRS") {
		    foreach $f (@filenames) {$mydir{$f}++}
		}
		elsif($name =~ /SRC$/) {
		    foreach $f (@filenames) {$mysource{$f}++}
		}
	    }
	}
    }

    while ($file = readdir(DIR)) {
	if(($file =~ /^[.]/) && (-d "$mypath/$file")) {next}  # skip .* hidden files
	if($file =~ /^CVS$/) {next}  # skip CVS directory
	if($file =~ /^Makefile([.]in)?$/) {next}  # skip Makefile, Makefile.in
	if($file =~ /[~]$/) {next}  # skip *~ recover files
	if($file =~ /^[.][\#]/) {next}  # skip .# recover files
	if($file =~ /^[\#].*[\#]/) {next}  # skip #xxx# recover files
	if($file =~ /[.]o$/) {next} # skip .o object files
	if($file =~ /[.]l[oa]$/) {next} # skip .o object files
	if($file eq "Makefile.am") {next}
	if (-d "$mypath/$file") {
	    if(-e "$mypath/$file/Makefile.am") {
		#print "  Directory $file\n";
		$fileprint = "";
		&processdir($file,$mypath); #& avoids prototype warning
		$dirdirs{$file} = "$mypath/$file";
	    }
	    else {
		print ".. $mypath/$file no Makefile.am\n";
	    }
	}
	else {
	    $dirfiles{$file} = "$mypath/$file";
	}
    }
    closedir(DIR);

    print "   Directory: $mypath\n";
    $f = scalar keys %dirfiles;

    if(!$hasmake) {
	print "++ no Makefile.am found in $mypath\n";
    }
    elsif(!scalar @dirlines) {
	print "++ no '=' lines found in $mypath\n";
    }

    else {
	my $nsrc = scalar keys %mysource;
	my $nbin = scalar keys %mybinary;
	my $nfil = scalar keys %myfile;
	print ".. $ilines lines $nsrc sources $nbin binaries $nfil other\n";
	my @unknown = ();
	foreach $fname (keys (%dirfiles)) {
	    if($fname =~ /[.][ch]$/) {
		if(!defined($mysource{$fname})) {push @unknown, $fname}
	    }
	    elsif($fname =~ /[.]/) {
		if(!defined($myfile{$fname})) {push @unknown, $fname}
	    }
	    else {
		if(!defined($myfile{$fname}) &&
		   !defined($mybinary{$fname})) {push @unknown, $fname}
	    }
	}
	if(scalar @unknown) {
	    my $uname = "";
	    my $nunk = scalar @unknown;
	    print "++ $nunk unknown files\n";
	    foreach $uname (@unknown) {
		print "   $uname\n";
	    }
	}
    }
    my $dname;
    foreach $dname (keys (%dirdirs)) {
	if(!defined($mydir{$fname})) {
	    print "++ $dname not in SUBDIRS\n"}
    }
    print "   Done directory: $mypath $f files\n\n";
}


sub processfile($$) {
    my ($filename,$path) = @_;
    my $fullname = "$path/$filename";
    print "   Process file $fullname\n";
    $fileprint .= "    Process file $fullname\n";
    my @lines = ();
#
# read file
#
# concatenate all lines ending with '\'
#
# then process any of the special strings
#
# we only need lines starting word = list

    $line = "";
    $wantline = 0;
    open (IN, "$fullname") || die "Cannot open $fullname";
    while (<IN>) {
	if(/^\s*$/) {next}
	if(/^[\#]$/) {next}
	if(/^\s*([^_\s])+_([^_\s]+)\s+[=]/) {
	    $wantline = 1;
	}
	if(/\s*SUBDIRS\s+[=]/) {
	    $wantline = 1;
	}
	if(/\s*[A-Z]+SRC\s+[=]/) {
	    $wantline = 1;
	}
	if(/^(.*)[\\]\s*$/) {	# has a continuation
	    if($wantline) {$line .= $1}
	}
	elsif($wantline) {	# complete line - process it
	    $line .= $_;
	    push @lines, $line;
	    $line = "";
	    $wantline = 0;
	}
    }
    return @lines;
}



processdir(".","");

if ((-e "embassy") && (-d "embassy")) {
    local *EDIR;
    opendir(EDIR, "embassy");
    while ($file = readdir(EDIR)) {
	if(($file =~ /^[.]/) && (-d $file)) {next}  # skip .* hidden files
	if($file =~ /^CVS$/) {next}  # skip CVS directory
	processdir(".","embassy/$file");
    }
    closedir(EDIR);
}
