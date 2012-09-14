#!/usr/bin/perl -w

$infile = "stdin";

if ($ARGV[0]) {
  $infile = $ARGV[0];
}

$infile =~ m"^([^.]+)"o;

# $fpref = $1;
$allsrc = "";

open (IN, $infile) || die "Cannot open input file $infile";

while (<IN>) {
  $allsrc .= $_;
}

print "\n";
print "============================\n";
print ".. File $infile\n";
print "============================\n";
@presrc = split (/^[A-Za-z][^\/\{\[;=]+[\)]\s*[\{]\s*$/osm, $allsrc);
$ip = 0;

while ($allsrc =~ /^([A-Za-z][^\/\{\[;=]+[\)]\s*[\{])\s*$/gosm) {
  $proto = $1;
  $presrc = $presrc[$ip];

  if ($proto =~ /^(static\s+)?\s*([^()]*\S)\s+([^()]*\S)\s*[\(]\s*([^{]*)[)]\s*[\{]/osm) {
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

  if($presrc =~ /^\#else\s*$/osm) {next}
  elsif($presrc =~ /^\#if\s*AJFALSE\s*$/osm) {next}
  elsif ($presrc !~ /[\n][\/][*]\s+([@]\S+)\s+(\S+)([^\/*][^*]*[*]+)*[\/]\s*$/osm) {
    print "bad docheader bad or missing for $pnam\n$proto\n";
  }
  $ip++;
}

close IN;
