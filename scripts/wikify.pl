#!/usr/bin/perl -w

$file = $ARGV[0];

if(!defined($file)) {exit}

$outfile = $file;
$outfile =~ s/ - EMBOSS[.]htm/.wiki/g;

open (OUT, ">/cygdrive/h/wikifix/$outfile") || die "Cannot open '$outfile'";

%author = ("ableasby" => "Bleasby",
	   "price" => "Rice",
	   "ilongden" => "Longden",
	   "mschuster" => "Schuster",
    );

%types = ("Ione" => "Short",
# headers on one line	  "Ihtml" => "",
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

$text =~ s/^.*(<body class=\"mediawiki[^\"]+page-(\S+))/$1/osm; # remove all to start of mediawiki
print STDERR "Page name $2\n";
$text =~ s/^.*<h1 id=\"firstHeading\" class=\"firstHeading\">([^<]+)<\/h1>\n//osm; # remove all to start of mediawiki
print STDERR "Page true name $1\n";
$text =~ s/^.*?\n([^\n]+<span class=\"mw-headline\">)/$1/osm; # remove all to first heading
$text =~ s/<span class=noprint><span style=\"color:#[^\"]+\">//gosm;
$text =~ s/<\/span><\/span>/\n/gosm;
$text =~ s/<\!--\s*NewPP limit report[^>]*>//gosm; # remove biblio comments
$text =~ s/<\!--\s*Saved in parser cache.*//gosm; # remove all from start of google footer

$text =~ s/\s*<span class=\"mw-headline\">([^<]+)<\/span>/$1/gosm; # clean up headings

#header tages removed

$text =~ s/(<\/?[Hh][Tt][Mm][Ll]>)//gosm;
$text =~ s/(<\/?[Bb][Oo][Dd][Yy][^>]*?>)//gosm;

$text =~ s/(<[Aa]\s+name\s*=\s*\"([^\"]+)[^>]+>)[^<]*<\/[Aa]>//gosm;

# phylip doc links

$text =~ s/<[Aa]\s+href\s*=\s*\"([^\/\".]+\.html)[^>]+>([^<]+)<\/[Aa]>/[http:\/\/evolution.genetics.washington.edu\/phylip\/doc\/$1 $2]/gosm;

# other links

$text =~ s/<[Aa]\s+href\s*=\s*\"\/wiki\/([^\"]+)[^>]+>\s*([^<]+)<\/[Aa]>/[[$1| $2]]/gosm;
$text =~ s/<[Aa]\s+href\s*=\s*\"([Hh]?[FfTt][Tt][Pp]:[^\"]+)[^>]+>([^<]+)\s*<\/[Aa]>/[$1 $2]/gosm;
$text =~ s/<[Aa]\s+href\s*=\s*\"([Mm]?[Aa][Ii][Ll][Tt][Oo]:[^\"]+)[^>]+>([^<]+)\s*<\/[Aa]>/[$1 $2]/gosm;
$text =~ s/<[Aa]\s+href\s*=\s*\"([^\"]+)[^>]+>([^<]+)\s*<\/[Aa]>/[[$1| $2]]/gosm;

# headers on one line

$text =~ s/(<[Hh]\d>)\s*\n\s*/$1/gosm;
$text =~ s/\n(<\/[Hh]\d>)/$1/gosm;

# remove CSS divisions
$text =~ s/<\/[Dd][Ii][Vv]>//gosm;

# remove rulers
$text =~ s/<[Hh][Rr]>//gosm;

# header levels to wiki syntax

$text =~ s/<[Hh]1>(.*?)<\/[Hh]1>/==$1==/gosm;
$text =~ s/<[Hh]2>(.*?)<\/[Hh]2>/==$1==/gosm;
$text =~ s/<[Hh]3>(.*?)<\/[Hh]3>/===$1===/gosm;
$text =~ s/<[Hh]4>(.*?)<\/[Hh]4>/====$1====/gosm;

# paragraphs to extra newlines at start

$text =~ s/<[Pp]>/\n/gosm;

# remove table row end markings

$text =~ s/^(<\/[Tt][RrDdHh]>)+\n//gosm;
$text =~ s/<\/[Tt][RrDdHh]>//gosm;
$text =~ s/^(<\/[Cc][Aa][Pp][Tt][Ii][Oo][Nn]>)+\n//gosm;
$text =~ s/<\/[Cc][Aa][Pp][Tt][Ii][Oo][Nn]>//gosm;

# remove end-of-paragraph

$text =~ s/<\/[Pp]>//gosm;

# breaks
$text =~ s/(<\/?[Bb][Rr][^>]*>)/\n/gosm;

# bold text
$text =~ s/(<\/?[Bb][^>]*>)/'''/gosm;

# emphasis = bold

$text =~ s/(<\/?[Ee][Mm]>)/'''/gosm;

# italic

$text =~ s/(<\/?[Ii]>)/''/gosm;

# remove empty newlines after list tags

$text =~ s/(<\/?[Ll][Ii]>)\n/$1/gosm;
$text =~ s/(<\/[Ll][Ii]>)//gosm;

$text =~ s/(<\/?[Dd][DdTt]>)\n/$1/gosm;
$text =~ s/(<\/[Dd][DdTt]>)//gosm;

$text =~ s/([^\n])(<[Tt][HhDd][^>]*>)/$1\n$2/gosm;

# Now process the text line-by-line

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
    if($t =~ s/<\/[Tt][Aa][Bb][Ll][Ee][^>]*>//gosm) {
      $table = 0;
      print OUT "|}\n";
    }
    if($t =~ s/<[Tt][Aa][Bb][Ll][Ee]([^>]*)>//gosm) {
      $table = 1;
      $rest = $1;
      $resttext = "";
      if($rest =~ /sortable/) {$resttext = " sortable"}
      print OUT "\n{| class=\"wikitable$resttext\" style=\"background:#ccccff\" border=\"2\"\n";
    }
    if($t =~ s/<\/[OoUuDd][Ll]>//gosm) {$list = ""}
    if($t =~ s/<[Uu][Ll]>//gosm) {$list = "u";$nlist=0}
    if($t =~ s/<[Oo][Ll]>//gosm) {$list = "o";$nlist=0}
    if($t =~ s/<[Dd][Ll]>//gosm) {$list = "d";$nlist=0}
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
		print OUT  "==See also==\n";
	    }
	    $t = "\{\{:Appinc:$type$name\}\}\n";
	}
    }

    if(!$pre & $t =~ /^\n$/) {$blank++}
    else{$blank=0}
    $i++;
    if($blank > 1){next}
    if($blank == 1 && $list ne ""){next}
#    print OUT "$i:";
    if($list ne "") {$t =~ s/\n/ /}
    if($pre) {print OUT " "}
    if($t =~ s/<[Tt][Rr]>//gosm) {
      if($table) {
	print OUT "|-";
	$table = 2;
      }
    }
    if($t =~ s/<[Cc][Aa][Pp][Tt][Ii][Oo][Nn]>//gosm) {
      if($table) {
	print OUT "|+";
	$table = 2;
      }
    }
    if($t =~ s/<[Tt][Hh]([^>]*)>//gosm) {
      if($table) {
	$rest = $1;
	$resttext = "";
	if($rest =~ /colspan=\"(\d+)\"/) {
	  $resttext .= " colspan=\"$1\"";
	}
	if($rest =~ /class=\"unsortable"/) {
	  $resttext .= " class=\"unsortable\"";
	}
	if($rest =~ /background:(#[0-9a-fA-F]+)/) {
	  $resttext .= " style=\"background:$1\"";
	}
	if($resttext ne "") {$resttext .= "|"}
	print OUT "!$resttext ";
	$table = 4;
      }
    }
    if($t =~ s/<[Tt][Dd]>//gosm) {
      if($table) {
	print OUT "| ";
	$table = 3;
      }
    }
    if($t =~ s/<[Ll][Ii]>//gosm) {
	if($nlist++){print OUT "\n"}
	if($list eq "u") {print OUT "* "}
	elsif($list eq "o") {print OUT "# "}
    }
    if($t =~ s/<[Dd][Tt]>([^<]*)//gosm) {
	if($nlist++){print OUT "\n"}
	if($list eq "d") {print OUT "; $1"}
    }
    if($t =~ s/<[Dd][Dd]>//gosm) {
	if($nlist++){print OUT "\n"}
	if($list eq "d") {print OUT ": "}
    }
    if($nlist && $list eq "") {print OUT "\n";$nlist=0;}
    print OUT "$t";
}
