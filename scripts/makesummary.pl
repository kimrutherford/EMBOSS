#!/usr/bin/perl -w

print "Make errors and warnings\n";
print "========================\n\n";

printf "%15s %6s %6s\n\n", "Package", "Errors", "Warn";

$myerr = int(`grep -c error: $ENV{HOME}/out/emboss10-emboss.out`);
$mywarn = int(`grep -c warning: $ENV{HOME}/out/emboss10-emboss.out`);
$tote = $myerr;
$totw = $mywarn;

$nodoch = $nodoct = $nosf = 0;

if ($myerr || $mywarn) {
    if (!$myerr) {$myerr = "."}
    if (!$mywarn) {$mywarn = "."}
    $link = "<a href=\"make-emboss.txt\">detail</a>";
}
else {$link=""}

printf "%15s %6s %6s %s\n\n", "EMBOSS", $myerr, $mywarn, $link;


$embassylist = `ls -1 embassy/*/Makefile.am`;
@embassy = split(/\s+/, $embassylist);

foreach $x (@embassy) {
    $x =~ /^embassy\/([^\/]+)\/Makefile.am/;
    $name = $1;
    $myerr = int(`grep -c error: $ENV{HOME}/out/emboss10-$name.out`);
    $mywarn =  int(`grep -c warning: $ENV{HOME}/out/emboss10-$name.out`);
    $tote += $myerr;
    $totw += $mywarn;
    if ($myerr || $mywarn) {
	if (!$myerr) {$myerr = "."}
	if (!$mywarn) {$mywarn = "."}
	$link = "<a href=\"make-$name.txt\">detail</a>";
    }
    else {$link=""}
    printf "%15s %6s %6s %s\n", $name, $myerr, $mywarn, $link;
}

printf "\n%15s %6d %6d\n", "TOTAL", $tote, $totw;

print "\n";
print "QA Test Results\n";
print "===============\n";

open (QA, "test/qa/qatest.out") || die "Unable to open qatest.out";

$skip = 0;
while (<QA>) {
    if (/^Tests total: +(\d+) pass: (\d+) fail: (\d+)/) {
	$tot = $1;
#	$pass = $2;
	$fail = $3;
    }
    if (/^Skipped: (\d+) check: (\d+) embassy: (\d+) requirements: (\d+)/) {
	$skip = $1;
	$skipc = $2;
	$skipe = $3;
	$skipr = $4;
    }
    if (/^No tests: (\d+)/) {$notest = $1;}
    if (/^Missing documentation html: (\d+) text: (\d+) sourceforge: (\d+)/) {
	$nodoch = $1;
	$nodoct = $2;
	$nosf   = $3
    }
}
close QA;

$ok = 1;
if ($fail) {
    print "  Tests failed $fail / $tot\n";
    $ok = 0;
}
if ($skip) {
    print "  Tests skipped: $skip check: $skipc embassy: $skipe requirements: $skipr\n";
    $ok = 0;
}

if ($notest) {
    print "  Programs with no tests: $notest\n";
    $ok = 0;
}

if ($nodoch || $nodoct) {
    print "  Missing documentation html: $nodoch text: $nodoct\n";
    $ok = 0;
}

if ($nosf) {
    print "  Missing sourceforge: $nosf\n";
    $ok = 0;
}

if ($ok) {print "OK\n"}
else {print "<a href=\"qatest.txt\">detail</a>\n"}

print "\n";


print "\n";
print "ValGrind Memory Test Results\n";
print "============================\n";

open (VG, "$ENV{HOME}/public_html/valgrind.txt") || die "Cannot open valgrind.txt";

$tot = 0;
$totok = -1;
while (<VG>) {
    if (/^Detail/) {last}
    if (/^Valgrind Tests: (\d+) OK: (\d+)/) {
	$tot = $1;
	$totok = $2;
    }
    if (/^Leaks: (\d+) Failed: (\d+) Errors: (\d+) Timeout: (\d+)/) {
	$totl = $1;
	$totf = $2;
	$tote = $3;
	$tott = $4;
    }
}

$totbad = $tot - $totok;
if ($totbad) {
    printf "  Problems:%4d /%d\n", $totbad,$tot;

    if ($totf) {printf "%10s %4d\n",  "Failed", $totf}
    if ($tote) {printf "%10s %4d\n",  "Errors", $tote}
    if ($tott) {printf "%10s %4d\n",  "Timeout", $tott}
    if ($totl) {printf "%10s %4d\n",  "Leaking", $totl}
    print "<a href=\"valgrind.txt\">detail</a>\n";
}
else {print "OK\n"}

print "\n";

print "AcdValid warnings and errors\n";
print "============================\n";

open (AV, "acdvalidreport.txt") || die "Cannot open acdvalidreport.txt";

$toto = $tote = $totw = 0;
while (<AV>) {
    if (/^ *(\d+) EMBOSS and (\d+) EMBASSY applications/) {next}
    elsif (/^\s*$/) {next}
    elsif (/^ *(\d+) [*] /) {$tote += $1}
    elsif (/^ *(\d+) /) {$totw += $1}
    else {$toto++}
}

$tot = $toto + $tote + $totw;
if ($tot) {
    printf "%10s %6d\n", "Errors", $tote;
    printf "%10s %6d\n", "Warnings", $totw;
    printf "%10s %6d\n", "Other", $toto;
    print "<a href=\"acdvalid.txt\">detail</a>\n";
}
else {print"OK\n"}

print "\n";

print "AcdSyntax documentation\n";
print "=======================\n";

open (AS, "x/acdsyntax.error") || die "Cannot open x/acdsyntax.error";

$totas = 0;
while (<AS>) {
    $totas++;
}

if ($totas) {
    print "Errors: $totas\n";
    print "<a href=\"acdsyntax.txt\">detail</a>\n";
}
else {print"OK\n"}

print "\n";

print "EFUNC documentation\n";
print "===================\n";

open (EF, "$ENV{HOME}/data/efunc/efunc.summary") || die "Cannot open efunc.summary";

$tot =  0;
while (<EF>) {
    if (/^ *(\d+) errors in (\d+) functions in (\d+) files/) {
	$tot = $1;
	$totfun = $2;
	$totfil = $3;
	if ($tot) {
	    print "Total: $tot Functions: $totfun Files: $totfil\n";
	}
    }
}
close EF;
if (!$tot) { print "OK\n"}
else {print "<a href=\"efunc-bad.txt\">detail</a>\n"}
print "\n";

print "EDATA documentation\n";
print "===================\n";

open (ED, "$ENV{HOME}/data/efunc/edata.summary") || die "Cannot open edata.summary";

$tot = 0;
while (<ED>) {
    if (/^ *(\d+) errors in (\d+) functions in (\d+) files/) {
	$tot = $1;
	$totfun = $2;
	$totfil = $3;
	if ($tot) {
	    print "\nTotal: $tot Functions: $totfun Files: $totfil\n";
	}
    }
}
close ED;
if (!$tot) { print "OK\n"}
else {print "<a href=\"edata-bad.txt\">detail</a>\n"}

print "\n";

