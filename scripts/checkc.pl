#!/usr/bin/perl -w

$showline=0;
if ($#ARGV > 1) {$showline = 1}
$i=0;
while (<>) {
  $i++;
  chomp;
  $len = length($_);
  if (/\s$/) {
    print STDERR "Trailing space line: $i\n";
   if ($showline) {print STDERR "$_"}
    s/\s+$//;
  }
  if (/[*][*][*]$/) {
    if ($len != 79) {
      print STDERR "Comment line $len not 79 bytes, line: $i\n";
      if ($showline) {print STDERR "$_"}
      $_ = substr("$_******************************************************************************",0,79);
      print STDOUT "$_\n";
      next;
    }
  }
  if (/^[*][*][*][*]+[\/]$/) {
    if ($len != 79) {
     print STDERR "Comment end $len not 79 bytes, line: $i\n";
      if ($showline) {print STDERR "$_"}
     print STDOUT "******************************************************************************/\n";
      next;
    }
  }
  if ($len > 79) {
    print STDERR "+++ Too long ($len) line: $i\n";
    if ($showline) {print STDERR "$_"}
    print STDOUT "$_\n";
    next;
  }
  print STDOUT "$_\n";
}
