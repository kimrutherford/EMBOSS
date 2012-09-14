#!/usr/bin/perl -w

%author = ("ableasby" => "Bleasby",
	   "price" => "Rice",
	   "ilongden" => "Longden",
	   "mschuster" => "Schuster",
    );

%types = ("Ione" => "Short",
	  "Ihtml" => "",
	  "Itable" => "Options",
	  "Usage" => "Example",
	  "Input" => "Input",
	  "Output" => "Output",
	  "History" => "History",
	  "Comment" => "Comment",
	  "Isee" => "Seealso"
    );

%names = ("Alignformats" => "AlignFormats",
	  "Assemblyformats" => "AssemblyFormats",
	  "Codonformats" => "CodonFormats",
	  "Featformats" => "FeatFormats",
	  "Ontologyformats" => "OntologyFormats",
	  "Reportformats" => "ReportFormats",
	  "Resourceformats" => "ResourceFormats",
	  "Seqformats" => "SeqFormats",
	  "Seqfeatformats" => "SeqfeatFormats",
	  "Taxonformats" => "TaxonFormats",
	  "Textformats" => "TextFormats",
	  "Urlformats" => "UrlFormats",
	  "Variationformats" => "VariationFormats",
	  "Assemblyinformats" => "AssemblyInFormats",
	  "Codoninformats" => "CodonInFormats",
	  "Featinformats" => "FeatInFormats",
	  "Ontologyinformats" => "OntologyInFormats",
	  "Resourceinformats" => "ResourceInFormats",
	  "Seqinformats" => "SeqInFormats",
	  "Seqfeatinformats" => "SeqfeatInFormats",
	  "Taxoninformats" => "TaxonInFormats",
	  "Textinformats" => "TextInFormats",
	  "Urlinformats" => "UrlInFormats",
	  "Variationformats" => "VariationFormats",
	  "Gcfiles" => "GeneticCodeFiles",
	  "Localfiles" => "LocalFiles",
	  "Graphicsdevices" => "GraphicsDevices",
	  "Drcat" => "Drcat",
	  "Edam" => "Edam",
	  "Go" => "Go",
	  "Taxon" => "Taxon",
	  "Jaspfiles" => "JasparFiles",
	  "Refiles" => "RebaseFiles",
	  "Jison-programs" => "JonIsonPrograms",
    );

use English;

$text = "";
while (<>) {
    s/^[ \t]+//;
    $text .= $_;
}

$text =~ s/^.*BEGIN MAIN CONTENT[^>]+>//gosm;
$text =~ s/(<\/?[Hh][Tt][Mm][Ll]>)//gosm;
$text =~ s/(<\/?[Bb][Oo][Dd][Yy][^>]*?>)//gosm;
$text =~ s/(<[Aa]\s+name\s*=\s*\"([^\"]+)[^>]+>)[^<]*<\/[Aa]>//gosm;
$text =~ s/<[Aa]\s+href\s*=\s*\"([^\".]+\.html)[^>]+>([^<]+)<\/[Aa]>/[http:\/\/evolution.genetics.washington.edu\/phylip\/doc\/$1 $2]/gosm;
$text =~ s/<[Aa]\s+href\s*=\s*\"([^\"]+)[^>]+>([^<]+)<\/[Aa]>/[$1| $2]/gosm;
$text =~ s/(<[Hh]\d>)\s*\n\s*/$1/gosm;
$text =~ s/\n(<\/[Hh]\d>)/$1/gosm;
$text =~ s/<\/[Dd][Ii][Vv]>//gosm;
$text =~ s/<[Hh][Rr]>//gosm;
$text =~ s/<[Hh]1>(.*?)<\/[Hh]1>/==$1==/gosm;
$text =~ s/<[Hh]2>(.*?)<\/[Hh]2>/==$1==/gosm;
$text =~ s/<[Hh]3>(.*?)<\/[Hh]3>/===$1===/gosm;
$text =~ s/<[Hh]4>(.*?)<\/[Hh]4>/====$1====/gosm;

$text =~ s/<[Pp]>/\n/gosm;
$text =~ s/<\/?[Tt][RrDd]>//gosm;
$text =~ s/<\/[Pp]>//gosm;
$text =~ s/(<\/?[Bb]>)/'''/gosm;
$text =~ s/(<\/?[Ee][Mm]>)/'''/gosm;
$text =~ s/(<\/?[Ii]>)/''/gosm;
$text =~ s/(<\/?[Ll][Ii]>)\n/$1/gosm;

$i=0;
$pre = 0;
$head=0;
$blank=0;
$table = 0;
$list = "";
while ($text =~ /.*?\n/gos) {
    $t = $MATCH;

    if($t =~ s/<[Hh][Ee][Aa][Dd]>//gosm) {$head=1}
    if($t =~ s/<\/[Hh][Ee][Aa][Dd]>//gosm) {$head=0;next}
    if($t =~ s/<!--#include file="header1.inc" -->//gosm) {$head=1}
    if($t =~ s/<!--#include file="header2.inc" -->//gosm) {$head=0;next}
    if($head) {next}
    if($t =~ s/<\/[Tt][Aa][Bb][Ll][Ee][^>]*>//gosm) {$table = 0}
    if($t =~ s/<[Tt][Aa][Bb][Ll][Ee][^>]*>//gosm) {$table = 1}
    if($t =~ s/<\/[OoUu][Ll]>//gosm) {$list = ""}
    if($t =~ s/<[Uu][Ll]>//gosm) {$list = "u";$nlist=0}
    if($t =~ s/<[Oo][Ll]>//gosm) {$list = "o";$nlist=0}
    if($t =~ s/<[Pp][Rr][Ee]>//gosm) {$pre = 1}
    if($t =~ s/<\/[Pp][Rr][Ee]>//gosm) {$pre = 0}

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
    $i++;
    if($blank > 1){next}
    if($blank == 1 && $list ne ""){next}
#    print "$i:";
    if($list ne "") {$t =~ s/\n/ /}
    if($pre) {print " "}
    if($t =~ s/<[Ll][Ii]>//gosm) {
	if($nlist++){print "\n"}
	if($list eq "u") {print "* "}
	elsif($list eq "o") {print "# "}
    }
    if($nlist && $list eq "") {print "\n";$nlist=0;}
    print "$t";
}
