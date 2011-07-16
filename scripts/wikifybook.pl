#!/usr/bin/perl -w

use English;

$text = "";

while (<>) {
    s/^[ \t]+//;
    s/\r//;
    $text .= $_;
}
$text =~ s/<section[^>]+>//gosm;
$text =~ s/<info>.*?<\/info>//gosm;
$text =~ s/<remark>.*?<\/remark>//gosm;
$text =~ s/(<[Hh]\d>)\s*\n\s*/$1/gosm;
$text =~ s/\n(<\/[Hh]\d>)/$1/gosm;
$text =~ s/<[Hh]2>(.*?)<\/[Hh]2>/==$1==/gosm;
$text =~ s/<[Hh]3>(.*?)<\/[Hh]3>/===$1===/gosm;
$text =~ s/<[Hh]4>(.*?)<\/[Hh]4>/====$1====/gosm;
$text =~ s/<formalpara><title>(.*?)<\/title>/====$1====/gosm;
$text =~ s/<section><title>(.*?)<\/title>/===$1===/gosm;
$text =~ s/<title>(.*?)<\/title>/==$1==/gosm;
$text =~ s/<programlisting>/\n<programlisting>/gosm;
$text =~ s/<\/programlisting>/\n<\/programlisting>/gosm;
$text =~ s/<para>/\n/gosm;
$text =~ s/<\/para>/\n/gosm;
$text =~ s/<formalpara>/\n/gosm;
$text =~ s/<\/formalpara>/\n/gosm;
$text =~ s/<\/?thead>//gosm;
$text =~ s/<\/?tbody>//gosm;
$text =~ s/<\/[Pp]>//gosm;
$text =~ s/(<\/?[Bb]>)/'''/gosm;
$text =~ s/(<\/?systemitem>)/'''/gosm;
$text =~ s/(<\/?emphasis>)/'''/gosm;
$text =~ s/(<\/?database>)/'''/gosm;
$text =~ s/(<\/?package[^>]*>)/''/gosm;
$text =~ s/(<\/?replaceable[^>]*>)/''/gosm;
$text =~ s/(<envar[^>]*>)/<tt>/gosm;
$text =~ s/(<\/envar[^>]*>)/<\/tt>/gosm;
$text =~ s/(<filename[^>]*>)/<tt>/gosm;
$text =~ s/(<\/filename[^>]*>)/<\/tt>/gosm;
$text =~ s/(<code[^>]*>)/<tt>/gosm;
$text =~ s/(<\/code[^>]*>)/<\/tt>/gosm;
$text =~ s/(<option[^>]*>)/<tt>/gosm;
$text =~ s/(<\/option[^>]*>)/<\/tt>/gosm;
$text =~ s/(<command[^>]*>)/<tt>/gosm;
$text =~ s/(<\/command[^>]*>)/<\/tt>/gosm;
$text =~ s/^\s*<varlistentry><term>/;/gosm;
$text =~ s/^\s*<variablelist>/\n<variablelist>/gosm;
$text =~ s/^\s*<\/listitem><\/varlistentry>/\n/gosm;
$text =~ s/\s*<link [^>]+>([^<]+)<\/link>\s*/ $1 /gosm;
$text =~ s/\s*\(<xref linkend=\"FormatsAlignmentDesc\" \/>//gosm;
$text =~ s/ \(see <xref linkend=\"FormatsAlignmentDesc\" \/>\)//gosm;
$text =~ s/<xref linkend=\"([^\"]+)\"[^>]*>/\[\[$1\]\]/gosm;
$text =~ s/<application role=\"emboss[^>]+>([^<]+)<\/application[^>]*>/"\[\[Appdoc:".ucfirst($1)."|$1\]\]"/gosme;
    
$text =~ s/(<\/?application[^>]*>)/'''/gosm;
$text =~ s/(<\/?[Ii]>)/''/gosm;
$text =~ s/(<\/?[Ll][Ii]>)\n/$1/gosm;
$text =~ s/<bridgehead>([^<]+)<\/bridgehead>/===$1===/gosm;

$pre = 0;
$blank=0;
$table = 0;
$list = "";
while ($text =~ /.*?\n/gos) {
    $t = $MATCH;
    $t =~ s/\r//gosm;

    if($t =~ s/^\s*<informaltable[^>]*>/\{| border=\"2\"/gosm) {$table=1}
    if($t =~ s/^\s*<\/informaltable[^>]*>/|\}/gosm) {$table=0}
    $t =~ s/^\s*<tr[^>]*>/|-/gosm;
    $t =~ s/^\s*<td>/|/gosm;
    $t =~ s/^\s*<th>/!/gosm;
    $t =~ s/<\/t[dhr]>//gosm;
    $t =~ s/<\/term><listitem>/:/gosm;

    if($t =~ s/<\/[Tt][Aa][Bb][Ll][Ee][^>]*>//gosm) {$table = 0}
    if($t =~ s/<[Tt][Aa][Bb][Ll][Ee][^>]*>//gosm) {$table = 1}
    if($t =~ s/<\/[OoUu][Ll]>//gosm) {$list = ""}
    if($t =~ s/<variablelist>//gosm) {$list = "d";$nlist=0}
    if($t =~ s/<\/variablelist>//gosm) {$list = ""}
    if($t =~ s/<[Oo][Ll]>//gosm) {$list = "o";$nlist=0}
    if($t =~ s/<programlisting[^>]*>//gosm) {$pre = 1}
    if($t =~ s/<\/programlisting>//gosm) {$pre = 0}
    if($t =~ s/<\/section>[^\n]*\n//gosm) {next}

    if($t =~ /<[!]--[\#]include file=\"([^\"]+)\" -->/gosm){
	$ifile = $1;
	if($ifile =~ /inc\/([^.]+)[.]address/){
	    $aname = $1;
	    if(defined($author{$aname})){$aname = $author{$aname}}
	    else {$aname = ucfirst($aname)}
	    $t = "\{\{:Appinc:Address$aname\}\}\n";
	}
	elsif($ifile =~ /inc\/target([^.]*)[.]itxt/){
	    $tgtname = ucfirst($1);
	    $t = "\{\{:Appinc:Target$tgtname\}\}\n";
	}
	elsif($ifile =~ /inc\/([^.]+)[.]ihelp/){
	    $t = "";
	}
	elsif($ifile =~ /inc\/([^.]+)[.](.*)/){
	    $name = ucfirst($1);
	    $type=ucfirst($2);
	    if(defined($names{$name})){$name = $names{$name}}
	    if(defined($types{$type})){$type = $types{$type}}
	    if($type eq "Seealso") {
		print "==See also==\n";
	    }
	    $t = "\{\{:Appinc:$type$name\}\}\n";
	}
    }

    if(!$pre & $t =~ /^\n$/) {$blank++}
    else{$blank=0}
    if($blank > 1){next}
    if($blank == 1 && $table){next}
#    print "$i:";
    if($pre) {print " "}
    if($t =~ s/<[Ll][Ii]>//gosm) {
	if($nlist++){print "\n"}
	if($list eq "u") {print "* "}
	elsif($list eq "o") {print "# "}
    }
    if($nlist && $list eq "") {print "\n";$nlist=0;}
    print "$t";
}
