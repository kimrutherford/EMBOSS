#!/usr/bin/perl -w

# creates and commits the Makefile.am for the text and html doc directories
# the default is to do the text directory.
# any parameter causes it to do the html directory

my @list = ();
my $line = "";
my $flag = 0;
my $flag2 = 0;
my $dir;  
my $html;
my $cvsdoc;

sub ingroups($$) {
    my ($a,$b) = @_;

    $a =~ s/_group[.]html//;
    $b =~ s/_group[.]html//;

    $a =~ s/groups[.]html//;
    $b =~ s/groups[.]html//;

    $ret = ($a cmp $b);

    return $ret;
}

open (VERS, "embossversion -full -auto|") || die "Cannot run embossversion";
while (<VERS>) {
    if(/BaseDirectory: +(\S+)/) {$distribtop = $1}
}
close VERS;
# where the CVS tree program doc pages are
$cvsdoc = "$distribtop/doc/programs/";

if ($#ARGV >= 0) {
    $html = 1;
    print "Doing HTML directory\n";
} else {
    $html = 0;
    print "Doing TEXT directory\n";
}


if ($html) {
    $dir = "html";
} else {
    $dir = "text";
}

chdir "$cvsdoc/$dir";

open (OLD, "Makefile.am") || die "Cannot open Makefile.am\n";
$data = 0;
while (<OLD>) {
    if(/_DATA\s*= /) {$data=1}
    if(/dir\s*= /) {$data=0}
    if($data) {
	@names = split(/\s+/, $_);
	foreach $n (@names) {
	    if($n =~ /_DATA$/) {next}
	    if($n eq "=") {next}
	    if($n eq "\\") {next}
	    $oldnames{$n}++;
	}
    }
}
close OLD;

open(M, ">make.temp") || die "Cannot open make.temp\n";

print M "## Process this file with automake to produce Makefile.in
";

# process gifs, then groups, then to lindna, then to end

if($html) {
    print M "
eimagesdir    = \$(pkgdatadir)/doc/programs/html
egroupsdir    = \$(pkgdatadir)/doc/programs/html
eprograms1dir = \$(pkgdatadir)/doc/programs/html
eprograms2dir = \$(pkgdatadir)/doc/programs/html

";
    print M "eimages\_DATA =";
    foreach $file (glob("*.jpg"), glob("*.gif")) {
	if($newnames{$file}) {next}
	$newnames{$file}++;
	if(!defined($oldnames{$file})) {
	    print "$file\n";
	}
	print M " \\\n$file";
    }
    print M "\n\n";

    print M "egroups\_DATA =";
    foreach $file (sort ingroups ("groups.html", glob("*_group.html"))) {
	if($newnames{$file}) {next}
	$newnames{$file}++;
	if(!defined($oldnames{$file})) {
	    print "$file\n";
	}
	print M " \\\n$file";
    }
    print M "\n\n";

    $firstchar="a";
    $set = 1;
    print M "eprograms$set\_DATA =";
    foreach $file (glob("*.html")) {
	if($newnames{$file}) {next}
	$newnames{$file}++;
	if(!defined($oldnames{$file})) {
	    print "$file\n";
	}
	$file =~ /^(.)/;
	if($1 ne $firstchar) {$firstchar = $1;$newfirst=1}
	if($newfirst && $firstchar ge "l" && $set == 1) {
	    ++$set;
	    print M "\n\neprograms$set\_DATA =";
	}
	print M " \\\n$file";
    }
    print M "\n\n";
}
else {
    print M "
eprograms1dir = \$(pkgdatadir)/doc/programs/text
eprograms2dir = \$(pkgdatadir)/doc/programs/text

";
    $firstchar="a";
    $set = 1;
    print M "eprograms$set\_DATA =";
    foreach $file (glob("*.txt")) {
	if($newnames{$file}) {next}
	$newnames{$file}++;
	if(!defined($oldnames{$file})) {
	    print "$file\n";
	}
	$file =~ /^(.)/;
	if($1 ne $firstchar) {$firstchar = $1;$newfirst=1}
	if($newfirst && $firstchar ge "l" && $set == 1) {
	    ++$set;
	    print M "\n\neprograms$set\_DATA =";
	}
	print M " \\\n$file";
    }
    print M "\n";
}

close (M);

# copy make.text to be Makefile.am
system("diff make.temp Makefile.am");

