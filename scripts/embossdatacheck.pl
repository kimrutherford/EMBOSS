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

close IN;

print "\n";
print "============================\n";
print ".. File $infile\n";
print "============================\n";

################
# typedef struct
################

@presrc = split (/^typedef\s+struct\s+\S*\s*[{][^}]+[}]\s*[^;]+;\s*$/osm, $allsrc);
$ip = 0;
$dalias="";

while ($allsrc =~ /^typedef\s+struct\s+(\S*)\s*[{][^}]+[}]\s*([^;]+);\s*(\#define\s+(\S+)\s+\S+[*])?$/gosm) {
  $dnam = $1;
  $dalias = $2;
  $ddefine = $4;
  @anam = (split(/[ \t,*]+/, $dalias));
  if (defined($ddefine)) {
    push @anam, $ddefine;
  }
  if (!defined($dnam) || $dnam eq "") {
    $dnam = $anam[$#anam];
  }
  $presrc = $presrc[$ip];

  if ($presrc =~ /[\n][\/][*]\s+[@]data[static]*\s+(\S+)([^\/*][^*]*[*]+)*[\/]\s*$/osm) {
    $hnam = $1;
    $ok = 0;
    foreach $nam (@anam) {
      if ($hnam eq $nam) {$ok = 1}
    }
    if (!$ok && $dnam ne $hnam) {
      print "bad docheader for $hnam precedes $dnam\n";
    }
  }
  else {
    print "bad or missing docheader for $dnam\n";
  }
  $ip++;
}

################
# typedef enum
################

@presrc = split (/^typedef\s+enum\s+\S*\s*[{][^}]+[}]\s*[^;]+;\s*$/osm, $allsrc);
$ip = 0;
$dalias="";

while ($allsrc =~ /^typedef\s+enum\s+(\S*)\s*[{][^}]+[}]\s*([^;]+);\s*?$/gosm) {
  $ealias = $1;
  $enam = $2;
  if (!defined($enam) || $enam eq "") {
    $enam = $ealias;
  }
  @anam = (split(/[ \t,*]+/, $enam));

  $presrc = $presrc[$ip];

  if ($presrc =~ /[\n][\/][*]\s+[@]enum[static]*\s+(\S+)([^\/*][^*]*[*]+)*[\/]\s*$/osm) {
    $hnam = $1;
    $ok = 0;
    foreach $nam (@anam) {
      if ($hnam eq $nam) {$ok = 1}
    }
    if (!$ok && $enam ne $hnam) {
      print "bad enumheader for $hnam precedes $enam\n";
    }
  }
  else {
    print "bad enumheader bad or missing docheader for $enam\n";
  }
  $ip++;
}

################
# const
################

@presrc = split (/^[sc][tatic ]*onst\s+[^*=;\(\)]*\s[^*]\S+\s*[=][^;]+;\s*?$/osm, $allsrc);
$ip = 0;
$dalias="";

while ($allsrc =~ /^[sc][tatic ]*onst\s+[^*=;\(\)]*\s([^*]\S+)\s*[=]([^;]+);\s*?$/gosm) {
  $cnam = $1;
  $cnam =~ s/[\[][0-9]*[\]]$//;
  if (!defined($cnam) || $cnam eq "") {
    $cnam = "unknown";
  }

  $presrc = $presrc[$ip];

#  print "PRESRC $ip\n$presrc\n====================\n";
  if ($presrc =~ /[\n][\/][*]\s+[@]const(static)?\s+(\S+)([^\/*][^*]*[*]+)*[\/]\s*$/osm) {
    $hnam = $2;
#    if ($cnam ne $hnam) {
#      print "bad constheader for $hnam precedes $cnam\n";
#    }
  }
  elsif ($presrc =~ /^\s*$/) {}
  else {
    print "bad constheader bad or missing docheader for $cnam\n";
  }
  $ip++;
}
