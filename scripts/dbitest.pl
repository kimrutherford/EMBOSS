#!/usr/bin/perl -w

$first = 0;
$last = 0;
$offset = 0;

$global_recsize = 0;

%indexfiles = (
	       "division.lkp" => "division.lkp",
	       "division"     => "division.lkp",
	       "div"          => "division.lkp",
	       "lkp"          => "division.lkp",
	       "entrynam.idx" => "entrynam.idx",
	       "entrynam"     => "entrynam.idx",
	       "ent"          => "entrynam.idx",
	       "idx"          => "entrynam.idx",
	       "acnum.hit"    => "acnum.hit",
	       "hit"          => "acnum.hit",
	       "acnum.trg"    => "acnum.trg",
	       "trg"          => "acnum.trg",
);

while (@ARGV and $ARGV[0] =~ /^-/) {

  $arg = shift @ARGV;
  last if ($arg =~ /^--$/);

  if ($arg =~ /-first/) {
    $first = shift @ARGV;
  }
  elsif ($arg =~ /-last/) {
    $last = shift @ARGV;
  }
  elsif ($arg =~ /-off/) {
    $offset = shift @ARGV;
  }
  else {
    print STDERR "Unrecognized option '$arg'\n\n";
    usage ();
  }
  
}

if ($first  < 0) {$first  = 0}
if ($last   < 0) {$last   = 0}
if ($offset < 0) {$offset = 0}

if ($first && !$last) { $last = $first }

if ($#ARGV != 0) {
  usage ();
}


$shortname = $ARGV[0];

if (!defined($indexfiles{$shortname})) {
  print STDERR "Filename must be division.lkp, entryname.idx, acnum.trg or acnum.hit\n";
  exit ();
}

$filename = $indexfiles{$shortname};

open (INFILE, "$filename") || die "Cannot open $filename";

$maxrecord = header (INFILE);

print "Maximum record: $maxrecord\n";

if (!$last) {$last = $maxrecord}

if ($first) {$begin = $first}
else {$begin = 1}

if ($last) {$end=$last}
else {$end = $maxrecord}

if ($offset) {
  $x = ($offset - 300 + $global_recsize)/$global_recsize;
    $recbuff = record (INFILE, $x);
    if ($filename =~ /^div/o) { divreport($recbuff, $x)  }
    elsif ($filename =~ /^ent/o) { entreport($recbuff, $x)  }
    elsif ($filename =~ /^acnum\.hit/o) { hitreport($recbuff, $x)  }
    elsif ($filename =~ /^acnum\.trg/o) { trgreport($recbuff, $x)  }
}
else {
  for ($x=$begin;$x <= $end; $x++) {
    $recbuff = record (INFILE, $x);
    if ($filename =~ /^div/o) { divreport($recbuff, $x)  }
    elsif ($filename =~ /^ent/o) { entreport($recbuff, $x)  }
    elsif ($filename =~ /^acnum\.hit/o) { hitreport($recbuff, $x)  }
    elsif ($filename =~ /^acnum\.trg/o) { trgreport($recbuff, $x)  }
  }
}

close INFILE;

exit ();



sub usage () {
  print STDERR "Usage:

dbitest.pl [-first num] [-last num] [-offset num] filename

Options:
  -first   first record to be reported (defaults to all)
  -last    last record to be reported  (defaults to all, or first if specified)
  -offset  byte offset to report (obtained from a -debug report)
";
  exit ();
}

sub header (*) {
  my $file = shift @_;
  my $buff;
  my $fsize;
  my $nrec;
  my $recsize;
  my $dbname;
  my $release;
  my $date;
  my $padding;

  read ($file, $buff, 300, 0) == 300 || die "Failed to read header";

  ($fsize, $nrec, $recsize, $dbname, $release, $date, $padding) =
    unpack ("VVva20a10a4a256", $buff);

  ($a,$b,$c,$d) = unpack ("CCCC", $date);

  printf "File size: %11d\n", $fsize;
  printf "Record count: %8d\n", $nrec;
  printf "Record size: %9d\n", $recsize;
  printf "Database name: '%s'\n", $dbname;
  printf "Release:       '%s'\n", $release;
  printf "Date string:   '%s'\n", $date;
  printf "Date: %x/%x/%x/%x\n", $a, $b, $c, $d;
  # printf "Padding: '%s'\n", $padding;
  undef $padding;

  $global_recsize = $recsize;

  return $nrec;

}


sub record (*$) {

  my ($file,$rec) = @_;
  my $buff;

  my $recoffset = 300 + ($rec-1)*$global_recsize;
  seek ($file, $recoffset, 0);
  read ($file, $buff, $global_recsize) == $global_recsize
    || die "Failed to read $global_recsize bytes at $recoffset for record $rec";

  return $buff;
}

sub divreport ($$) {
  my ($buff, $nrecord) = @_;
  my $namelen = $global_recsize - 2;
  my $template = "va$namelen";
  my ($num,$name) = unpack ($template, $buff);
  printf "%8d: number: %d   name '%s'\n", $nrecord, $num, $name;
  return;
}

sub entreport ($$) {
  my ($buff, $nrecord) = @_;
  my $idlen = $global_recsize - 10;
  $template = "a$idlen" . "VVv";
  my ($id, $rpos, $spos,$num) = unpack ($template, $buff);
  printf "%8d: rpos: %10d (%08x) spos: %10d (%08x) num: %8d name '%s'\n",
          $nrecord, $rpos, $rpos, $spos, $spos, $num, $id;
  return;
}

sub hitreport ($$) {
  my ($buff, $nrecord) = @_;
  my $idlen = $global_recsize - 4;
  $template = "V";
  my ($num) = unpack ($template, $buff);
  printf "%8d:  num: %8d \n",
          $nrecord, $num;
  return;
}

sub trgreport ($$) {
  my ($buff, $nrecord) = @_;
  my $idlen = $global_recsize - 8;
  $template = "VVa$idlen";
  my ($count, $start, $id) = unpack ($template, $buff);
  printf "%8d: count: %6d start: %10d  name '%s'\n",
          $nrecord, $count, $start, $id;
  return;
}
