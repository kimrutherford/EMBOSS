#!/usr/bin/perl -w

$infile = "stdin";

if ($ARGV[0]) {
  $infile = $ARGV[0];
}

$infile =~ m"^([^.]+)"o;

# $fpref = $1;

open (IN, $infile) || die "Cannot open input file $infile";

while (<IN>) {
  $allsrc .= $_;
}

print "\n";
print "============================\n";
print ".. File $infile\n";
print "============================\n";
@presrc = split (m"^[A-Za-z][^/\{\[;=]+[\)]\s*[\{]\s*$"osm, $allsrc);
$ip = 0;

while ($allsrc =~ m"^([A-Za-z][^/\{\[;=]+[\)]\s*[\{])\s*$"gosm) {
  $proto = $1;
  $presrc = $presrc[$ip];

  if ($proto =~ m"^(static\s+)?\s*([^()]*\S)\s+([^()]*\S)\s*[\(]\s*([^{]*)[)]\s*[\{]"osm) {
    if (defined($2)){$ptyp = $2}
    else {$ptyp = ""}
    $pnam = $3;
    if ($pnam =~ /^([*]+)/) {
      $ptyp .= $1;
      $pnam =~ s/^[*]+//;
    }
  }

###  print "============================\n";
###  print "$ip $pnam\n";
###  print "============================\n";
###  print "$presrc\n";
  if ($presrc !~ m"[\n][/][*]\s+([@]\S+)\s+(\S+)([^/*][^*]*[*]+)*[/]\s*$"osm) {
    print "bad or missing docheader for $pnam\n$proto\n";
  }
  $ip++;
}

close IN;
