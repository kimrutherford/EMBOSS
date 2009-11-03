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
 
open(M, ">make.temp") || die "Can't open make.temp\n";

$firstchar="a";
foreach $file (glob("*.gif"), glob("*.html"), glob("*.txt"), glob("*.jpg")) {  
#print ">$file<\n";
  $file =~ /^(.)/;
  if($1 ne $firstchar) {$firstchar = $1;$newfirst=1}
  else{$newfirst=0}
  if ($newfirst || (length($line) + length($file) +1 > 60)) {
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
}
print M " \\\n\t$line\n\n";
print M "pkgdatadir=\$(prefix)/share/\$(PACKAGE)/doc/programs/$dir
pkgdata2dir=\$(prefix)/share/\$(PACKAGE)/doc/programs/$dir\n";


close (M);

# copy make.text to be Makefile.am
system("diff make.temp Makefile.am");

# cvs commit it
print "cvs commit -m'new makefile' Makefile.am\n";
#system("cvs commit -m'new makefile' Makefile.am");


