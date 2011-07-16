#!/usr/bin/perl -w

use English;

$text = "";
while (<>) {
    s/<a href=\"\#output[.]1\">.*?<\/a>/[[#Output example|Go to the output files for this example]]/gosm;
    s/<a href=\"\#output[.](\d+)\">.*?<\/a>/[[#Output example $1|Go to the output files for this example]]/gosm;
    s/<a href=\"\#input[.]1\">.*?<\/a>/[[#Input example|Go to the input files for this example]]/gosm;
    s/<a href=\"\#input[.](\d+)\">.*?<\/a>/[[#Input example $1|Go to the input files for this example]]/gosm;
    s/<a name=[^>]*>//gosm;
    s/<\/a>//gosm;
    s/<\/?[Tt][DdRrHh][^>]*>//gosm;
    s/<\/?[Tt][Aa][Bb][Ll][Ee][^>]*>//gosm;
    s/<h3>Input files for usage example <\/h3>/===Input example===/gosm;
    s/<h3>Input files for usage example (\d+)<\/h3>/===Input example $1===/gosm;
    s/<h3>Output files for usage example <\/h3>/===Output example===/gosm;
    s/<h3>Output files for usage example (\d+)<\/h3>/===Output example $1===/gosm;
    s/<h3>(File: .*?)<\/h3>/<h4>$1<\/h4>/gosm;
    s/<h3>(Database entry: .*?)<\/h3>/<h4>$1<\/h4>/gosm;
    s/<[Pp]>/\n/gosm;
    s/<\/[Pp]>//gosm;
    s/<[Bb]><\/[Bb]>//gosm;
    s/<[Ii]><\/[Ii]>//gosm;
    s/(<\/?[Bb]>)/'''/gosm;
    s/(<\/?[Ll][Ii]>)\n/$1/gosm;
     s/<[Hh]2>(.*?)<\/[Hh]2>/==$1==/gosm;
    s/<[Hh]3>(.*?)<\/[Hh]3>/===$1===/gosm;
    s/<[Hh]4>(.*?)<\/[Hh]4>/====$1====/gosm;
    $text .= $_;
}

$pre = 0;
$blank=0;
while ($text =~ /.*?\n/gos) {
    $t = $MATCH;

    if($t =~ s/<[Pp][Rr][Ee]>//gosm) {$pre = 1; if($t =~ /^\n$/){next}}
    if($t =~ s/<\/[Pp][Rr][Ee]>//gosm) {$pre = 0; if($t =~ /^\n$/){next}}

    if(!$pre & $t =~ /^\n$/) {$blank++}
    else{$blank=0}
    if($blank > 1){next}
    if($pre) {print " "}
    print "$t";
}
