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

$lastn = "";
open (OLD, "Makefile.am") || die "Cannot open Makefile.am\n";
$data = 0;
while (<OLD>) {
    if(/pkgdata_DATA = /) {$data=1}
    if(/pkgdatadir/) {$data=0}
    if($data) {
	@names = split(/\s+/, $_);
	foreach $n (@names) {
	    if($n eq "pkgdata_DATA") {next}
	    if($n eq "pkgdata2_DATA") {next}
	    if($n eq "=") {next}
	    if($n eq "\\") {$newline{$lastn}=1;next}
	    $oldnames{$n}++;
	    $lastn = $n;
	}
    }
}
close OLD;

open(M, ">make.temp") || die "Cannot open make.temp\n";

$lastn = "";
$firstchar="a";
foreach $file (glob("*.gif"), glob("*_group.html"), glob("*.html"), glob("*.txt"), glob("*.jpg")) {  
#print ">$file<\n";
    if($newnames{$file}) {next}
    $newnames{$file}++;
    $file =~ /^(.)/;
    if($1 ne $firstchar) {$firstchar = $1;$newfirst=1}
    else{$newfirst=0}
    if(!defined($oldnames{$file})) {
	print "$file\n";
    }
    if ($newline{$lastn} || $newfirst || (length($line) + length($file) +1 > 70)) {
	if ($flag) {
	    print M " \\\n\t";
	} else {
	    if ($flag2) {
		print M "\npkgdata2_DATA = ";
		$flag = 1;
	    } else {
		print M "pkgdata_DATA = ";
		$flag = 1;
		$flag2 = 1;
	    }
	}
	print M "$line";
	$line = $file;
    } else {
	$line .= " $file";
    } 
    if ($file =~ /^lindna.html/ || $file =~ /^lindna.txt/) {
#    print "Found lindna - breaking in half here\n";
	$flag = 0;
    }
    $lastn = $file;
}
print M " \\\n\t$line\n\n";
print M "pkgdatadir=\$(prefix)/share/\$(PACKAGE)/doc/programs/$dir
pkgdata2dir=\$(prefix)/share/\$(PACKAGE)/doc/programs/$dir\n";


close (M);

# copy make.text to be Makefile.am
system("diff make.temp Makefile.am");

