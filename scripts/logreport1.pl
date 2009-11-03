#!/usr/bin/perl -w

%progcount=();
%proguser=();

while (<>) {
  chomp;
  if (!/^(\S+)\s+(\S+)\s+(... ... .. ..:..:.. [12][0-9][0-9][0-9])$/) {
    print STDERR "bad logfile line: '$_'\n";
    next;
  }
  $progname = $1;
  $username = $2;
###  $date = $3;
  $progcount{$progname}++;
  $proguser{$progname}{$username}++;
}

print "Program            Total        Username    Total  Pcnt    Other Users\n";
print "=======            =====        ========    =====  ====    ===== =====\n";
 foreach $prog (sort { $progcount{$b} <=> $progcount{$a} } keys (%progcount)) {
  $totuser=0;
  foreach $user (sort { $proguser{$prog}{$a} <=> $proguser{$prog}{$b} } keys (%{$proguser{$prog}})) {
    $maxcount = $proguser{$prog}{$user};
    $maxuser = $user;
    $totuser++;
  }
  $percent = 100.0 * $maxcount/$progcount{$prog};
  $rest = $progcount{$prog} - $maxcount;
  printf "%-15s %8d %15s %8d %5.1f %8d %5d\n", $prog, $progcount{$prog}, $maxuser, $maxcount, $percent, $rest, $totuser;
}
