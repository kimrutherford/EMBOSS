#!/usr/bin/perl -w


if ($ARGV[0]) {$funcfile = $ARGV[0]}
else {$funcfile = "efunc.out"}
if ($ARGV[1]) {$datafile = $ARGV[1]}
else {$datafile = "edata.out"}

open (FUNC, $funcfile) || die "Cannot open $funcfile";
open (DATA, $datafile) || die "Cannot open $datafile";
%efunc = ();
%edata = ();
%efuncdoc = ();
%edatadoc = ();

$dtot=0;
$ftot=0;

while (<FUNC>) {
    if (($ctype, $cdata, $cfunc, $cmod, $clib, $cdesc) =
	/^category +(\S+) +[\[]([^\]]+)\] +(\S+) +(\S+) +(\S+) +: +(.*)/) {
	$ftot++;
	$efunc{"$cdata\_$cfunc"} = $ctype;
	$efuncdoc{"$cdata\_$cfunc"} = $cdesc;
	$efuncsrc{"$cdata\_$cfunc"} = $cmod;
###	print "efunc $cdata\_$cfunc '$ctype'\n";
    }
}


while (<DATA>) {
    if (($ctype, $cdata, $cfunc, $cmod, $clib, $cdesc) =
	/^category +(\S+) +[\[]([^\]]+)\] +(\S+) +(\S+) +(\S+) +: +(.*)/) {
	$dtot++;
	$edata{"$cdata\_$cfunc"} = $ctype;
	$edatadoc{"$cdata\_$cfunc"} = $cdesc;
###	print "edata $cdata\_$cfunc '$ctype'\n";
	if (!defined($efunc{"$cdata\_$cfunc"})) {
	    print "\n";
	    print "$clib/$cmod $cdata bad category $ctype $cfunc (undefined)\n";
	    $outstr = "Expected:\n** \@category $ctype [$cdata] $cdesc\n";
	    $outstr =~ s/^([^\n]{65,75}) ([^ \n]{3})/$1\n**                $2/om;
	    $outstr =~ s/[\n]([^\n]{65,75}) ([^ \n]{3})/\n$1\n**                $2/om;
	    $outstr =~ s/[\n]([^\n]{65,75}) ([^ \n]{3})/\n$1\n**                $2/om;
	    $outstr =~ s/[\n]([^\n]{65,75}) ([^ \n]{3})/\n$1\n**                $2/om;
	    $outstr =~ s/[\n]([^\n]{65,75}) ([^ \n]{3})/\n$1\n**                $2/om;
	    $outstr =~ s/[\n][*][*] +([^\n]{1,10})\n/ $1\n/gom;
	    print $outstr;
	}
	elsif ($efunc{"$cdata\_$cfunc"} ne $ctype) {
	    $etype = $efunc{"$cdata\_$cfunc"};
	    $esrc = $efuncsrc{"$cdata\_$cfunc"};
	    print "\n";
	    print "$clib/$cmod $cdata $cfunc bad category type $ctype defined as $etype in $esrc\n"
	}
	elsif ($efuncdoc{"$cdata\_$cfunc"} ne $cdesc) {
	    $edesc = $efuncdoc{"$cdata\_$cfunc"};
	    $esrc = $efuncsrc{"$cdata\_$cfunc"};
	    print "\n";
	    print "$clib/$cmod $cdata $cfunc bad category type $ctype described as '$cdesc' not '$edesc' in $esrc\n"
	}
    }
}

print "\nTested $dtot data and $ftot functions\n"


