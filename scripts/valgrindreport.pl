#!/usr/bin/perl -w

######################################################################
# 
# Name: getprogramnames
# 
# Description: 
# 	Runs wossname to get the EMBOSS programs on the path
#	together with their groups
#
# Args: 
#   *** These are all global variables ***
# 	%grpnames - hash of key=lowercase group name,
#                        value = uppercase group description
# 	%progs - hash of key=program name, value = program description
#	%groups - hash of key=lowercase group name, value = program names
# 
######################################################################

sub getprogramnames ( ) {

    my $prog;	# program name
    my $capgrp;	# uppcase group name
    my $grp;	# lowercase group name

    open (PROGS, "wossname -noembassy -auto |") ||
	die "Cannot run wossname";
    while ($prog = <PROGS>) {
	if ($prog =~ /^\s*$/) {	# ignore blank lines
	    next;
	} elsif ($prog =~ /^([A-Z0-9 ]+)$/) {	# uppcase means a group name
	    $capgrp = $1;			
	    $grp = lc($capgrp);
	    $grp =~ s/ +/_/g;		# lowercase one-word group_name
	    $grpnames{$grp} = $capgrp;
#      print "Group $grp = $capgrp\n";
	} elsif ($prog =~ /^(\S+) +(.*)/) {
	    $progs{$1} = $2;		
	    $groups{$grp} .= "$1 ";
#      print "Program in $grp = $1\n";
	}
    }
    close PROGS;

    foreach $e(@embassylist) {
	open (PROGS, "wossname -showembassy $e -auto |") ||
	    die "Cannot run wossname";
	while ($prog = <PROGS>) {
	    if ($prog =~ /^\s*$/) {	# ignore blank lines
		next;
	    } elsif ($prog =~ /^([A-Z0-9 ]+)$/) {  # uppcase means a group name
		$capgrp = $1;			
		$grp = lc($capgrp);
		$grp =~ s/ +/_/g;		# lowercase one-word group_name
		$grpnames{$grp} = $capgrp;
#      print "Group $grp = $capgrp\n";
	    } elsif ($prog =~ /^(\S+) +(.*)/) {
		$progs{$1} = $2;		
		$groups{$grp} .= "$1 ";
		$embassyprogs{$1} = $e;
#      print "Program in $grp = $1\n";
	    }
	}
	close PROGS;
    }

}

@embassylist = ("appendixd",
		"cbstools",
		"clustalomega",
		"domainatrix",
		"domalign",
		"domsearch",
		"emnu",
		"esim4",
		"hmmer",
		"hmmernew",
		"iprscan",
		"meme",
		"memenew",
		"mira",
		"mse",
		"myemboss", # we avoid documenting these examples
		"myembossdemo", # we avoid documenting these examples
		"phylip",
		"phylipnew",
		"signature",
		"structure",
		"topo",
		"vienna",	# old vienna
		"vienna2",
    );

%cmd = ();
%progs = ();
%embassyprogs = ();

$failtxt = "";
$leaktxt = "";
$errortxt = "";
$timetxt="";
$reachtxt = "";
$othertxt = "";

getprogramnames();

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
    if (/^Valgrind time /) {next}
    elsif (/^Valgrind test (\S+)/) {
	$name = $1;
	($appname) = ($name =~ /^([^-]+)/);
	$isembassy = 0;
	$embassy = "EMBOSS";
	if(!defined($progs{$appname})) {
	    if(-e "/homes/pmr/check/bin/$appname")
	      {$embassy = "EMBOSSCHECK"}
	    else
	      {print "Unknown application '$appname'\n"}
	}
	elsif(defined($embassyprogs{$appname})) {
	    $isembassy = 1;
	    $embassy = $embassyprogs{$appname};
	}
	$leak = $status = 0;
	if (/^Valgrind test (\S+) OK [\(]all clean[\)]/) {next}
	elsif (/^Valgrind test (\S+) OK [\(]still reachable +([^\)]+)/) {
	    $status = $2;
	    $reachcnt{$embassy}++;
	    $reachtxt .= "\n";
	    $reachtxt .= "$name $embassy reachable $status\n";
	    $reachtxt .= "\% $cmd{$name}\n";
	}
	elsif (/^Valgrind test (\S+) (timed out)/) {
	    $status = $2;
	    $timecnt{$embassy}++;
	    $timetxt .= "\n";
	    $timetxt .= "$name $embassy status $status\n";
	    $timetxt .= "\% $cmd{$name}\n";
	    $timetxt .= "$txt";
	}
	elsif (/^Valgrind test (\S+) returned status (\d+)/) {
	    $status = $2;
	    $failcnt{$embassy}++;
	    $failtxt .= "\n";
	    $failtxt .= "$name $embassy status $status\n";
	    $failtxt .= "\% $cmd{$name}\n";
	    $failtxt .= "$txt";
	}
	elsif (/^Valgrind test (\S+) leak ([^e]+bytes)/) {
	    $leak = $2;
	    $leakcnt{$embassy}++;
	    $leak =~ s/\(possibly 0 \[0\]\) //;
	    $leaktxt .= "\n";
	    $leaktxt .= "$name $embassy leak $leak\n";
	    $leaktxt .= "\% $cmd{$name}\n";
	}
	elsif (/^Valgrind test (\S+) errors (\S+\s+\S+)/) {
	    $err = $2;
	    $errorcnt{$embassy}++;
	    $errortxt .= "\n";
	    $errortxt .= "$name $embassy ** errors ** $err\n";
	    $errortxt .= "\% $cmd{$name}\n";
	}
	elsif (/^Valgrind test (\S+) (.*)\n/) {
	    $msg = $2;
	    $othercnt{$embassy}++;
	    $othertxt .= "\n";
	    $othertxt .= "$name $embassy ++ '$msg'\n";
	    $othertxt .= "\% $cmd{$name}\n";
	}
	$txt = "";
    }
    else {$txt .= $_}
}


if($failtxt ne "") {
    print "\nFailed\n======\n";
    foreach $e(sort(keys(%failcnt))) {
	printf "  %3d %s\n", $failcnt{$e}, $e;
    }
    print "$failtxt\n";
}

if($errortxt ne "") {
    print "\nErrors\n======\n";
    foreach $e(sort(keys(%errorcnt))) {
	printf "  %3d %s\n", $errorcnt{$e}, $e;
    }
    print "$errortxt\n";
}

if($timetxt ne "") {
    print "\nTimed out\n========\n";
    foreach $e(sort(keys(%timecnt))) {
	printf "  %3d %s\n", $timecnt{$e}, $e;
    }
    print "$timetxt\n";
}


if($othertxt ne "") {
    print "\nOther problems\n==============\n";
    foreach $e(sort(keys(%othercnt))) {
	printf "  %3d %s\n", $othercnt{$e}, $e;
    }
    print "$othertxt\n";
}

if($leaktxt ne "") {
    print "\nLeaks\n=====\n";
    foreach $e(sort(keys(%leakcnt))) {
	printf "  %3d %s\n", $leakcnt{$e}, $e;
    }
    print "$leaktxt\n";
}

if($reachtxt ne "") {
    print "\nReachable\n=========\n";
    foreach $e(sort(keys(%reachcnt))) {
	printf "  %3d %s\n", $reachcnt{$e}, $e;
    }
    print "$reachtxt\n";
}
