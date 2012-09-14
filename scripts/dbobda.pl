#!/usr/bin/perl -w

use Cwd;

%dbformats = ("embl" => "embl",
	      "genbank" => "genbank",
	      "swiss" => "swissprot",
	      "wormpep" => "fasta");

%dbfiles = ("embl" => "*.dat",
	    "genbank" => "*.seq",
	    "swiss" => "*.dat",
	    "wormpep" => "wormpep");

$distribtop = "/home/pmr/devemboss";

#open (VERS, "embossversion -full -auto|") || die "Cannot run embossversion";
#while (<VERS>) {
#    if(/InstallDirectory: +(\S+)/) {
#	$installtop = $1;
#	$installtop =~ s/\/$//;
#    }
#    if(/BaseDirectory: +(\S+)/) {
#	$distribtop = $1;
#	$distribtop =~ s/\/$//;
#    }
#}
#close VERS;

foreach $db (sort(keys(%dbformats))) {
    chdir ("$distribtop/test/$db");
    printf "$db: %s\n", cwd(); ;
    $location = "$db";
    if($db eq "wormpep") {$location = "worm"}
    elsif($db eq "swiss") {$location = "swissprot"}
    $format = $dbformats{$db};
    $dbfiles = $dbfiles{$db};
    system "mv $location/CVS ./savecvs";
    print STDERR "~/devemboss/scripts/bioflat_index_obda.pl --create --indextype flat --dbname $location --location . --format $format $dbfiles\n";
    system "~/devemboss/scripts/bioflat_index_obda.pl --create --indextype flat --dbname $location --location . --format $format $dbfiles";
    system "mv ./savecvs $location/CVS";
}
