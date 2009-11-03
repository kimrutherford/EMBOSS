#!/usr/bin/perl -w

sub runtest ($) {
  my ($name) = @_;
  print "Test $name\n";

  if (defined($tests{$name})) {
  print "Running $purepath$tests{$name}\n";
    $status = system ("$purepath$tests{$name}");
    
    if ($status) {
      print STDERR "Purify test $name returned status $status\n";
    }
    else {
      print STDERR "Purify test $name OK\n";
    }
  return $status;
  }
  else {
      print STDERR "ERROR: Unknown test $name \n";
      return -1;
  }

}

%tests = ();

open (MEMTEST, "../memtest.dat");
while (<MEMTEST>) {
    if (/(\S+) += +(\S.*)/) {
	$tests{$1}=$2;
    }
}
close MEMTEST;

if (defined($ENV{EPURE})) {
  $purepath = "$ENV{EPURE}/";
}
else {
  $purepath = "";
}

@dotest = @ARGV;

foreach $name (@dotest) {
  if ($name =~ /^-(\S+)$/) {
    $arg = $1;
    if ($arg eq "all") {
      foreach $x (sort (keys (%tests))) {
	runtest($x);
      }
      exit;
    }
    elsif ($arg eq "list") {
      foreach $x (sort (keys (%tests))) {
	printf "%-15s %s\n", $x, $tests{$x};
      }
      exit;
    }
    elsif ($arg =~ /block=(\d+)/) {
      $block=$1;
      $i=0;
      $blocksize=10;
      $blockcount=0;
      foreach $x (sort (keys (%tests))) {
	if (!$i) {
	  $blockcount++;
	}
	$i++;
	if ($i >= $blocksize) {$i=0}
	if ($blockcount == $block) {
	  runtest($x);
	}
      }
      exit;
    }
    else {
      print STDERR "Invalid argument $name (ignored)\n";
      next;
    }
  }
  runtest ($name);
}


exit();
