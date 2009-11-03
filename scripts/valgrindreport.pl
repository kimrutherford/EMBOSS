#!/usr/bin/perl -w

%cmd = ();

$failtxt = "";
$leaktxt = "";
$errortxt = "";
$timetxt="";
$reachtxt = "";
$othertxt = "";

open (DAT, "../memtest.dat") || die "Cannot open file memtest.dat";
while (<DAT>) {
    if (/^[\#]/) {next}
    if (/^(\S+) = (.*)/) {$cmd{$1} = $2};
    if (/^(\S+) =test= (.*)/) {$cmd{$1} = $2};
}
close DAT;

open (DAT, "qatestcmd.dat") || die "Cannot open file qatestcmd.dat";
while (<DAT>) {
    if (/^[\#]/) {next}
    if (/^(\S+) = (.*)/) {$cmd{$1} = $2};
    if (/^(\S+) =test= (.*)/) {$cmd{$1} = $2};
}
close DAT;

while (<>) {
    if (/^Valgrind test (\S+)/) {
	$name = $1;
	$leak = $status = 0;
	if (/^Valgrind test (\S+) OK [\(]all clean[\)]/) {next}
	elsif (/^Valgrind test (\S+) OK [\(]still reachable +([^\)]+)/) {
	    $status = $2;
	    $reachtxt .= "\n";
	    $reachtxt .= "$name reachable $status\n";
	    $reachtxt .= "\% $cmd{$name}\n";
	}
	elsif (/^Valgrind test (\S+) (timed out)/) {
	    $status = $2;
	    $timetxt .= "\n";
	    $timetxt .= "$name status $status\n";
	    $timetxt .= "\% $cmd{$name}\n";
	    $timetxt .= "$txt";
	}
	elsif (/^Valgrind test (\S+) returned status (\d+)/) {
	    $status = $2;
	    $failtxt .= "\n";
	    $failtxt .= "$name status $status\n";
	    $failtxt .= "\% $cmd{$name}\n";
	    $failtxt .= "$txt";
	}
	elsif (/^Valgrind test (\S+) leak ([^e]+bytes)/) {
	    $leak = $2;
	    $leak =~ s/\(possibly 0 \[0\]\) //;
	    $leaktxt .= "\n";
	    $leaktxt .= "$name leak $leak\n";
	    $leaktxt .= "\% $cmd{$name}\n";
	}
	elsif (/^Valgrind test (\S+) errors (\S+\s+\S+)/) {
	    $err = $2;
	    $errortxt .= "\n";
	    $errortxt .= "$name ** errors ** $err\n";
	    $errortxt .= "\% $cmd{$name}\n";
	}
	elsif (/^Valgrind test (\S+) (.*)\n/) {
	    $msg = $2;
	    $othertxt .= "\n";
	    $othertxt .= "$name ++ '$msg'\n";
	    $othertxt .= "\% $cmd{$name}\n";
	}
	$txt = "";
    }
    else {$txt .= $_}
}


if($failtxt ne "") {
    print "\nFailed\n======\n$failtxt\n";
}

if($errortxt ne "") {
    print "\nErrors\n======\n$errortxt\n";
}

if($timetxt ne "") {
    print "\nTimed out\n========\n$timetxt\n";
}


if($othertxt ne "") {
    print "\nOther problems\n==============\n$othertxt\n";
}

if($leaktxt ne "") {
    print "\nLeaks\n=====\n$leaktxt\n";
}

if($reachtxt ne "") {
    print "\nReachable\n=========\n$reachtxt\n";
}
