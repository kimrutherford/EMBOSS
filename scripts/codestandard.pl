#!/usr/bin/perl -w

$cnt = 0;
$mt="\n";
while (<>) {
    $cnt++;
    if (/^\s*$/ && $lastbrack) {
	printf "%5d< %s", $cnt, $_;
    }
    if (/^\s*[\}]$/ && $lastempty) {
	printf "%5d> %s", $cnt-1, $mt;
    }
    if (/^\s*[\{]\s*$/) {$lastbrack=1}
    else {$lastbrack=0}

    if (/^\s*\/\//) {next}
    if (/\S.*[\{]/) {
	if (/\S.*[\{].*[\}]/) {next}
	if (/\".*[\{]/) {next}
	if (/[=]\s*[\{]/) {next}
	printf "%5d: %s", $cnt, $_;
    }
    if (/^\s*$/) {$lastempty=1;$mt=$_;}
    else {$lastempty=0}
}
