#!/usr/bin/perl -w

$last = "unknown";
$mtot=$mbad=$mdesc=$mundef=0;
$tot =$tbad=$tdesc=$tundef=0;

while (<>) {
    if (/^[^\/]+[\/]([^ \/]+)/) {
	$mod=$1;
	if ($mod ne $last) {
	    if ($mtot) {
		if (!$tot) {
		printf "Bad Des Und Module\n";
		}
		printf "%3d %3d %3d %s\n", $mbad, $mdesc, $mundef, $last;
	    }
	    $tot+= $mtot;
	    $tbad += $mbad;
	    $tdesc += $mdesc;
	    $tundef += $mundef;
	    $mtot=$mbad=$mdesc=$mundef=0;
	    $last = $mod;
	}
	if (/[\(]undefined[\)]/) {$mundef++}
	elsif (/bad category type \S+ described as/) {$mdesc++}
	else {$mbad++}
	$mtot++;
    }

}

printf "%3d %3d %3d TOTAL\n", $tbad, $tdesc, $tundef;
