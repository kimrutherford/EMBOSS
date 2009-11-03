#!/usr/bin/perl -w

$errfile = 0;
$errfunc = 0;
$errcount = 0;
$errtotcount = 0;
$totcount = 0;
$totfile = 0;
$filelib = "unknown";
$filename = "unknown";
$funcline = "Function 'undefined'\n";

open (LOG, ">embossdocreport.log") || die "Cannot open embossdocreport.log";

%badfiles = ();
%badtotfiles = ();

while (<>) {
    $newfunc = 0;
    $newfile = 0;
    if (/^Function (\S+)/) {
	$funcline = $_;
	$funcname = $1;
	$newfunc = 1;
    }
    elsif (/^Static function (\S+)/) {
	$funcline = $_;
	$funcname = $1;
	$newfunc = 1;
    }
    elsif (/^Macro (\S+)/) {
	$funcline = $_;
	$funcname = $1;
	$newfunc = 1;
    }
    elsif (/^Data type (\S+)/) {
	$funcline = $_;
	$funcname = $1;
	$newfunc = 1;
    }
    elsif (/^Static data type (\S+)/) {
	$funcline = $_;
	$funcname = $1;
	$newfunc = 1;
    }

    elsif (/^Typedef data type (\S+)/) {
	$funcline = $_;
	$funcname = $1;
	$newfunc = 1;
    }

    elsif (/^set pubout \'([^\']+)\' lib \'([^\']+)\'/) {
	$newfileline = $_;
	$newfilename = $1;
	$newfilelib = $2;
	$newfile = 1;
	if (/ type \'([^\']+)\'/) {
	    $newfiletype = $1;
	}
	else {
	    $newfiletype = "";
	}
    }

    elsif (/^bad or missing docheader for (\S+)/) {
	$funcname = $1;
	$newfunc = 1;
    }

    elsif (/^bad docheader for (\S+) precedes (\S+)/) {
	$funcname = $2;
	$newfunc = 1;
    }

    elsif (/^bad/) {
	if (!$errcount) {
	    if (!$errfile) {
		print LOG "=============================\n";
		print LOG $newfileline;
	    }
	    print LOG "  ".$funcline;
	}
	$errcount++;
	print LOG "    ".$_;
    }
    elsif (/^[.][.] File \/\S+\/([^\/]+)\/([^\/.]+)[.]([^\/.]+)$/) {
	$newfileline = $_;
	$newfilename = $2;
	$newfilelib = $1;
	$newfile = 1;
	if ($3 eq 'c') {
	    $newfiletype = "source";
	}
	elsif ($3 eq 'h') {
	    $newfiletype = "include";
	}
	else {
	    $newfiletype = "";
	}
	$funcline = "";
	$funcname = "unknown";
    }

    if ($newfunc || $newfile) {
	if ($errcount) {
	    $errtotcount += $errcount;
	    $totcount += $errcount;
	    $errfunc++;
	    $errfile++;
	$errcount = 0;
	}
    }
    if ($newfile) {
	if ($errfile) {
	    $badfiles{$filelib."_".$filename}=$errfile;
	    $badtotfiles{$filelib."_".$filename}=$errtotcount;
	    $totfile++;
	}
	$errfile = 0;
	$errtotcount = 0;
	$filename = $newfilename;
	if ($newfiletype ne "") {
	    if ($newfiletype eq "include") {$filename .= ".h"}
	    elsif ($newfiletype eq "source") {$filename .= ".c"}
	    else {$filename .= ".$newfiletype"}
	}
	$filelib = $newfilelib;
    }


}

if ($errcount) {
    $totcount += $errcount;
    $errtotcount += $errcount;
    $errfunc++;
    if (!$errfile) {
	$totfile++;
    }
    $errfile++;
}

if ($errfile) {
    $badfiles{$filelib."_".$filename}=$errfile;
    $badtotfiles{$filelib."_".$filename}=$errtotcount;
    $totfile++;
}

foreach $x (sort (keys (%badfiles))) {
    printf "%4d %4d %s\n", $badtotfiles{$x}, $badfiles{$x}, $x;
}

print STDERR "$totcount errors in $errfunc functions in $totfile files\n";
close LOG;
