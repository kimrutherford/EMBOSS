#!/usr/bin/perl -w

#
# Test groups.standard top level topic v. tpoic relations
# need at least one match to eeach group
# or some overall group marked by 0000003 as the top level
$dotest = 1;
$testgroup = "";
foreach $test (@ARGV) {
    $testgroup = lc($test);
    $testgroup =~ s/ /_/g;
    $testgroup{$testgroup} = 1;
}

open (VERSION, "embossversion -full -filter|") ||
    die "Cannot run embossversion";

while (<VERSION>){
   if(/RootDirectory: (\S+)/) {
	$rootdir = $1;
    }
   if(/Emboss_Standard: (\S+)/) {
	$standarddir = $1;
    }
}
close VERSION;

open (GROUPS, "$rootdir/acd/groups.standard") ||
    die "Cannot find groups.standard";
while(<GROUPS>){
    if(/^#/) {next}
    if(/^EDAM:(\d+)\s+(\S+) +(.*)/){
	$term = $1;
	$group = $2;
	$desc = $3;
	$group = lc($group);
	$group =~ s/:/_/g;
	$term{$group} = $term;
    }
}
close GROUPS;

$dotest = 0;
$lastgroup = "";
open (WOSSNAME, "wossname -filter|") ||
    die "Cannot run wossname";
while(<WOSSNAME>){
    if(/^([A-Z0-9 ]+)$/) {
	$group = $1;
	$group = lc($group);
	$group =~ s/ /_/g;
	if($dotest && $lastgroup ne ""){
	    print "\nGroup: $lastgroup $nprogs ($gterm)\n";
	    if(scalar(keys(%topic))) {
	    $getanc = "{";
	    foreach $e (sort {$topic{$a} <=> $topic{$b}} (keys(%topic))){
		printf "%3d %s %s\n", $topic{$e}, $e, $edam{$e};
		$getanc .= "$e,";
	    }
#	    if($gterm ne "0000003") {
#		$getanc .= "$gterm}";
#	    } else {
		$getanc =~ s/,$/\}/;
#	    }

#	    print "ontogetcommon edam:$getanc -obsolete -filter -oformat excel\n";
	    open (ANC, "ontogetcommon 'edam:$getanc' -obsolete -filter -oformat excel|") ||
		die "Failed to run ontogetcommon \"edam:$getanc\"";
	    while(<ANC>){
		print "$_";
		if (/^$gterm/) {print "+++++++\n"}
		if (!$groupok && /^0000003/) {print "-------\n"}
	    }
	    close ANC;
	    }
	    else {print "+++No topics in group+++\n"}
#	    foreach $e (sort {$oper{$a} <=> $oper{$b}} (keys(%oper))){
#		printf "oper  %3d %s %s\n", $oper{$e}, $e, $edam{$e};
#	    }
	}
	$lastgroup = $group;
	%topic = ();
	%oper = ();
	$nprogs = 0;
	if(defined($term{$lastgroup}) && $term{$lastgroup} ne "0000003") {
	    $gterm = "$term{$lastgroup}";
	} else {
	    $gterm = "0000003";
	}
	if($testgroup ne "" && !$testgroup{$lastgroup}) {
	    $lastgroup = $group;
	    $dotest=0;
	    next;
	}

	$dotest = 1;
	$groupok = 1;
    }
    elsif(!$dotest) {next}
    elsif(/^(\S+)\s+(.*)/){
	$prog = $1;
	$desc = $2;
	$acd = "$standarddir/acd/$prog.acd";;
	open (ACD, "$acd") || die "Cannot open '$acd'";
	$nprogs++;
	$app = 0;
	$topicok = 0;
	$topicwarn = 0;
	$topicmsg = "";
	while(<ACD>){
	    if(/^ *application:/){$app=1}
	    if($app) {
		if(/^ *]/){$app=0}
		if(/relations: +\"([^\"]+)\"/) {
		    $rel = $1;
		    if($rel =~ /^EDAM:(\d+) (\S+) (.*)/) {
			$term = $1;
			$namespace = $2;
			$name = $3;
			if($namespace eq "topic"){
			    $topic{$term}++;
			    $edam{$term} = $name;
#			    print "ontogetcommon \"edam:{$gterm,$term}\"\n";
			    open (ANC, "ontogetcommon \"edam:{$gterm,$term}\" -obsolete -filter -oformat excel|") ||
				die "Failed to run ontogetcommon \"edam:{$gterm,$term}\"";
			    while(<ANC>){
				if (/^$gterm/) {
				    $topicok = 1;
#				    $topicmsg .= "$gterm $prog FOUND match to EDAM:$term $namespace $name\n"
				} else {
				    if(! /^0000003 /) {
					$topicmsg .= "$gterm $prog no match to EDAM:$term $namespace $name\n";
					$topicwarn++;
				    }
				}
			    }
			    close ANC;
			}
			elsif($namespace eq "operation"){
			    $oper{$term}++;
			    $edam{$term} = $name;
			}
		    }
		    else {
			print STDERR "$prog bad relation '$rel'\n";
		    }
		}
	    }
	}
	close ACD;
	if(!$topicok) {print "$topicmsg";$groupok = 0;}
	elsif($topicmsg ne "") {print "OK $prog but $topicwarn warnings:\n"}
    }
}

if($dotest && $lastgroup ne ""){
    print "\nGroup: $lastgroup $nprogs ($gterm)\n";
    foreach $e (sort {$topic{$a} <=> $topic{$b}} (keys(%topic))){
	printf "%3d %s %s\n", $topic{$e}, $e, $edam{$e};
    }
    $getanc = "{";
    foreach $e (sort {$topic{$a} <=> $topic{$b}} (keys(%topic))){
	printf "%3d %s %s\n", $topic{$e}, $e, $edam{$e};
	$getanc .= "$e,";
    }
#    if($gterm ne "0000003") {
#	$getanc .= "$gterm}";
#    } else {
	$getanc =~ s/,$/\}/;
#    }
#    print "ontogetcommon edam:$getanc -obsolete -filter -oformat excel\n";
    open (ANC, "ontogetcommon 'edam:$getanc' -obsolete -filter -oformat excel|") ||
	die "Failed to run ontogetcommon \"edam:$getanc\"";
    while(<ANC>){
	print "$_";
	if (/^$gterm/) {print "+++++++\n"}
	if (!$groupok && /^0000003/) {print "-------\n"}
    }
    close ANC;

#    foreach $e (sort {$oper{$a} <=> $oper{$b}} (keys(%oper))){
#	printf "oper  %3d %s %s\n", $oper{$e}, $e, $edam{$e};
#    }
}
