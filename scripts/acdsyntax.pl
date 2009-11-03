#!/usr/bin/perl -w

use English;

open (EMBOSSVERSION, "embossversion -full -auto -filter|") ||
    die "Cannot run embossversion";

while (<EMBOSSVERSION>) {
    if (/BaseDirectory: +(\S+)/) { $basedir = $1 }
}

close EMBOSSVERSION;

open (ACDSTATS, "acdsyntax.acdstats") ||
    die "Cannot open acdsyntax.acdstats";

while (<ACDSTATS>) {
    if (/^(\S+)\s+(\d+)/) {
	$t=$1;
	$acdstats{$t} = $2;
	$acdstatsdefa{$t} = {};
	$acdstatsattr{$t} = {};
    }
    elsif (/^\s+[*][*]\s+(\S+)\s+(\d+)/) { $acdstatsdefa{$t}{$1} = $2 }
    elsif (/^\s+(\S+)\s+(\d+)/) { $acdstatsattr{$t}{$1} = $2 }
}

close ACDSTATS;

open (TABLE, ">test.html");
print TABLE "<HTML><HEAD></HEAD><BODY>\n";

open (CONTENTS, ">content.html");

print CONTENTS "<html><head><title>
EMBOSS: AJAX Command Definition (ACD files)</title></head>

<body bgcolor=\"\#ffffff\" text=\"\#000000\">



<table align=\"center\" border=\"0\" cellspacing=\"0\" cellpadding=\"0\">
<tbody><tr><td valign=\"top\">
<a href=\"index.html\" <img border=\"0\"
  src=\"/images/emboss_icon.jpg\" alt=\"\"
   width=\"150\" height=\"48\"></a>
</td>
<td align=\"left\" valign=\"middle\">
<b><font size=\"+6\">

AJAX Command Definition (ACD files)
</font></b>
</td></tr>
</tbody></table>

<br>&nbsp;

<h3><a href=\"syntax.html\">ACD </a></h3>

";

$contentlevel=0;

# read the entrails full output

$itable = 0;

%section = ();
%acdtypes = ();
%acdattr = ();
%acdcalc = ();
%acdqual = ();
%acddefdefa = ();
%acddefcomm = ();
%acddeftype = ();
%acdcomm = ();
%stages = (
	   "ACD Types" => "TYPES",
	   "ACD Application Qualifiers" => "QUALS",
	   "ACD Default attributes" => "DEFATTR",
	   "ACD Calculated attributes" => "CALCATTR",
	   "sequence input formats" => "SEQINPUT",
	   "sequence output formats" => "SEQOUTPUT",
	   "Sequence Types" => "SEQTYPE",
	   "sequence access methods" => "SEQACCESS",
	   "Database attributes" => "DBATTR",
	   "Resource attributes" => "RESATTR",
	   "alignment output formats" => "ALIGNOUTPUT",
	   "report output formats" => "REPORTOUTPUT",
	   "Graphics Devices" => "GRAPHDEV",
	   "Other" => "OTHER"
	   );

%sectionnames = (
	     "simple" => "Simple",
	     "input" => "Input",
	     "selection" => "Selection lists",
	     "output" => "Output",
	     "graph" => "Graphics",
	     "other" => ""
	     );

%typedef = (
	    "boolean" => "Y/N",
	    );

%seqtype = (
	    "ANY" => "Nucleotide or protein",
	    "NUC" => "Nucleotide only",
	    "PRO" => "Protein only"
	    );

sub doacdtypes() {
    my $attr;
    my $type;
    my $default;
    my $section;
    my $comment;

    $attr = 0;
    while ($line) {
	if ($line =~ /^[\}]/) {return 1}
	if ($line =~ /^    attributes [\{]$/) {$attr=1}
	if ($line =~ /^    qualifiers [\{]$/) {$attr=0}
	if ($line =~ /^  (\S+)\s+(\S+)\s+\"([^\"]*)\"$/) {
	    $acd = $1;
	    $section = $2;
	    $comment = $3;
	    $acdtypes{$acd} = $section;
#	    print "\nacd '$acd' section '$section' comment '$comment'\n";
	    if (defined($section{$section})) {
		$section{$section} = "$section{$section} $acd";
	    }
	    else {
		$section{$section} = "$acd";
	    }
	    $acdcomm{$acd} = $comment;
	}
	if ($attr && $line =~ /^      (\S+)\s+(\S+)\s+\"([^\"]*)\"\s+\"([^\"]*)\"$/) {
	    $attr = $1;
	    $type = $2;
	    $default = $3;
	    $comment = $4;
	    if ($comment =~ /default:(.+)$/) {
		$default = $1;
		$comment = $PREMATCH;
	    }
#	    print "attr '$attr' type '$type' default '$default' comment '$comment'\n";
	    if (defined($acdattr{$acd})) {
		$acdattr{$acd} = "$acdattr{$acd} $attr";
	    }
	    else {
		$acdattr{$acd} = "$attr";
	    }
	    $acdattrtype{"$acd\_$attr"} = $type;
	    $acdattrcomm{"$acd\_$attr"} = $comment;
	    $acdattrdefa{"$acd\_$attr"} = $default;
	}
	if (!$attr && $line =~ /^      (\S+)\s+(\S+)\s+\"([^\"]*)\"\s+\"([^\"]*)\"$/) {
	    $qual = $1;
	    $type = $2;
	    $default = $3;
	    $comment = $4;
#	    print "qual '$qual' type '$type' default '$default' comment '$comment'\n";
	    if (defined($acdqual{$acd})) {
		$acdqual{$acd} = "$acdqual{$acd} $qual";
	    }
	    else {
		$acdqual{$acd} = "$qual";
	    }
	    $acdqualtype{"$acd\_$qual"} = $type;
	    $acdqualcomm{"$acd\_$qual"} = $comment;
	    $acdqualdefa{"$acd\_$qual"} = $default;
	}
	$line = <> ;
   }
}

sub doacddefattr() {
    my $attr;
    my $type;
    my $default;
    my $comment;

    while ($line) {
	if ($line =~ /^$/) {return 1}
	if ($line =~ /^  (\S+)\s+(\S+)\s+\"([^\"]*)\"\s+\"([^\"]*)\"$/) {
	    $attr = $1;
	    $type = $2;
	    $default = $3;
	    $comment = $4;
#	    print "\ndefattr '$attr' type $type default '$default' comment '$comment'\n";
	    $acddeftype{$attr} = "$type";
	    $acddefdefa{$attr} = "$default";
	    $acddefcomm{$attr} = "$comment";
	}
	$line = <> ;
    }
}

sub doacdcalcattr() {
    my $acd;
    my $attr;
    my $type;
    my $default;
    my $comment;

    while ($line) {
	if ($line =~ /^$/) {return 1}
	if ($line =~ /^  (\S+)$/) {
	    $acd = $1;
#	    print "\ncalcattr '$acd'\n";
	}
	if ($line =~ /^      (\S+)\s+(\S+)\s+\"([^\"]*)\"\s+\"([^\"]*)\"$/) {
	    $attr = $1;
	    $type = $2;
	    $default = $3;
	    $comment = $4;
#	    print "\ncalcattr '$acd' attr '$attr' type $type default '$default' comment '$comment'\n";
	    if (defined($acdcalc{$acd})) {
		$acdcalc{$acd} = "$acdcalc{$acd} $attr";
	    }
	    else {
		$acdcalc{$acd} = "$attr";
	    }
	    $acdcalctype{"$acd\_$attr"} = "$type";
	    $acdcalcdefa{"$acd\_$attr"} = "$default";
	    $acdcalccomm{"$acd\_$attr"} = "$comment";
	}
	$line = <> ;
    }
}

sub doseqtypes() {
    my $attr;
    my $type;
    my $default;
    my $section;
    my $comment;
    my $cnt = 0;

    $attr = 0;
    while ($line) {
	if ($line =~ /^[\}]/) {return 1}
	if ($line =~ /^  (\S+)\s+(\S+)\s+(\S+)\s+(\S+)\s+\"([^\"]*)\"\s+\"([^\"]*)\"\s+\"([^\"]*)\"$/) {
	    $type = $1;
	    $gap = $2;
	    $ambig = $3;
	    $nucprot = $4;
	    $from = $5;
	    $to = $6;
	    $comment = $7;
	    push @seqtype, $type;
	    $seqgap{$type} = $gap;
	    $seqambig{$type} = $ambig;
	    $seqnucprot{$type} = $nucprot;
	    $seqfrom{$type} = $from;
	    $seqto{$type} = $to;
	    $seqcomm{$type} = $comment;
	}
	$line = <> ;
    }
}

sub doacdquals() {
    my $attr;
    my $type;
    my $default;
    my $section;
    my $comment;
    my $cnt = 0;

    $attr = 0;
    while ($line) {
	if ($line =~ /^$/) {return 1}
	if ($line =~ /^(\S+)\s+(\S+)\s+\"([^\"]*)\"\s+\"([^\"]*)\"$/) {
	    $qual = $1;
	    $type = $2;
	    $default = $3;
	    $comment = $4;
	    push @qual, $qual;
	    $qualtype{$qual} = $type;
	    $qualdefa{$qual} = $default;
	    $qualcomm{$qual} = $comment;
	}
	$line = <> ;
    }
}

sub dotabletypes($) {
    my ($title) = @_;
    $itable++;
    print TABLE "
<table border=1 cellpadding=0>
";
    print TABLE "
 <tr>
  <td>
  <p><b>Data type / Object</b></p>
  </td>
  <td>
  <p><b>Description</b></p>
  </td>
  <td>
  <p><b>Calculated Attributes</b></p>
  </td>
  <td>
  <p><b>Specific Attributes</b></p>
  </td>
  <td>
  <p><b>Command Line Qualifiers</b></p>
  </td>
 </tr>
";

    print TABLE "
 <tr>
  <td colspan=5>
  <p><b>All data types</b></p>
  </td>
 </tr>
";

### The default attributes section
    print TABLE "
 <tr>
";
    print TABLE "
  <td>
  <p>&nbsp;</p>
  </td>
  <td>
  <p>All data types</p>
  </td>
  <td>
  <p>&nbsp;</p>
  </td>
";
    print TABLE "
 <td>
 <p>
";
    foreach $x (sort(keys(%acddefdefa))) {
	$defa = $acddefdefa{$x};
	$comm = $acddefcomm{$x};
	if ($defa =~ /[(](.*)[)]/) {$defa = "<i>$1</i>"}
	else {$defa = "\"$defa\""}
	print TABLE "$x: $defa<br>\n";
    }

    print TABLE "
 </p>
 </td>
";

    print TABLE "
  <td>
  <p>&nbsp;</p>
  </td>
 </tr>
";

### The sections, in order

    foreach $s ("simple", "input", "selection", "output", "graph", "other") {
	if (!defined($section{$s})) {next}
	print TABLE "
 <tr>
  <td colspan=5>
  <p><b>$sectionnames{$s} types</b></p>
  </td>
 </tr>
";

	foreach $a (split(" ", $section{$s})) {
	    $comm = $acdcomm{$a};
	   print TABLE "
 <tr>
  <td>
  <p><b>$a</b></p>
  </td>
  <td>
  <p>$comm</p>
  </td>
";

### Calculated attributes

	   print TABLE "
  <td>
  <p>
";
	    if (defined($acdcalc{$a})) {
		foreach $y (split(" ", $acdcalc{$a})) {
		    $defa = $acdcalcdefa{"$a\_$y"};
		    $type = $acdcalctype{"$a\_$y"};
		    $comm = $acdcalccomm{"$a\_$y"};
		    print TABLE "$y ($type)<br>\n";
		}
	    }
	    else {print TABLE "&nbsp;\n";}
	    print TABLE "
 </p>
 </td>
";

### Specific attributes

	   print TABLE "
  <td>
  <p>
";
	    if (defined($acdattr{$a})) {
		foreach $y (split(" ", $acdattr{$a})) {
		    $defa = $acdattrdefa{"$a\_$y"};
		    $comm = $acdattrcomm{"$a\_$y"};
		    $type = $acdattrtype{"$a\_$y"};
		    if ($defa =~ /[(](.*)[)]/) {$defa = "<i>$defa</i>"}
		    elsif ($type eq "string") {$defa = "\"$defa\""}
		    print TABLE "$y: $defa<br>\n";
		}
	    }
	    else {print TABLE "&nbsp;\n";}

	    print TABLE "
 </p>
 </td>
";

### Associated qualifiers

	   print TABLE "
  <td>
  <p>
";
	    if (defined($acdqual{$a})) {
		foreach $y (split(" ", $acdqual{$a})) {
		    $defa = $acdqualdefa{"$a\_$y"};
		    $comm = $acdqualcomm{"$a\_$y"};
		    if ($defa =~ /[(](.*)[)]/) {$defa = "<i>$1</i>"}
		    else {$defa = "\"$defa\""}
		    print TABLE "$y: $defa<br>\n";
		}
	    }
	    else {print TABLE "&nbsp;\n";}

	    print TABLE "
 </p>
 </td>
";
	}
	    print TABLE "
 </tr>
";
    }
	    print TABLE "
 </table>

<a name=table$itable>&nbsp;</a>
<p><b>Table $itable. $title</b></p>

";
}

sub dotabletypeattr($$) {

    my ($section,$title) = @_;
    my $a;

    $isubtable++;

    print TABLE "
<table border=1 cellpadding=0>
";
    print TABLE "
 <tr>
  <td>
  <p><b>Data type</b></p>
  </td>
  <td>
  <p><b>Attribute definition</b></p>
  </td>
  <td>
  <p><b>Description</b></p>
  </td>
 </tr>
";

    foreach $t (split(" ", $section{$s})) {
	$typename = $t;

	    if (defined($acdattr{$t})) {
		foreach $a (split(" ", $acdattr{$t})) {
		    $defa = $acdattrdefa{"$t\_$a"};
		    $comm = $acdattrcomm{"$t\_$a"};
		    $type = $acdattrtype{"$t\_$a"};
		    if (defined($typedef{$type})) {
			$typestring = "<i>$typedef{$type}<i>";
		    }
		    else {
			$typestring = "<i>$type<i>";
		    }
		    if ($defa =~ /[(](.*)[)]/) {$defa = "<i>$defa</i>"}
		    elsif ($type eq "string") {$defa = "\"$defa\""}
	print TABLE "
 <tr>
  <td>
  <p>$typename</p>
  </td>
  <td>
  <p>$a: $typestring</p>
  </td>
  <td>
  <p>$comm<br>Default: $defa</p>
  </td>
 </tr>
";
		    $typename = "&nbsp;";
		}
	    }

    }


    print TABLE "
</table>

<a name=table$itable$isubtable>&nbsp;</a>
<p><b>Table $itable\.$isubtable. $title</b></p>

";
}

sub dotablecalc($) {

    my ($title) = @_;
    my $a;

    $itable++;

    print TABLE "
<table border=1 cellpadding=0>
";
    print TABLE "
 <tr>
  <td>
  <p><b>Data type</b></p>
  </td>
  <td>
  <p><b>Calculated attributes</b></p>
  </td>
  <td>
  <p><b>Description</b></p>
  </td>
 </tr>
";

    foreach $t (sort(keys(%acdattr))) {
	$typename = $t;
	if (defined($acdcalc{$t})) {
	    foreach $a (split(" ", $acdcalc{$t})) {
		$defa = $acdcalcdefa{"$t\_$a"};
		$comm = $acdcalccomm{"$t\_$a"};
		$type = $acdcalctype{"$t\_$a"};
		if (defined($typedef{$type})) {
		    $typestring = "<i>$typedef{$type}<i>";
		}
		else {
		    $typestring = "<i>$type<i>";
		}
		if ($defa =~ /[(](.*)[)]/) {$defa = "<i>$defa</i>"}
		elsif ($type eq "string") {$defa = "\"$defa\""}
		print TABLE "
 <tr>
  <td>
  <p>$typename</p>
  </td>
  <td>
  <p>$a: $typestring</p>
  </td>
  <td>
  <p>$comm<br>Default: $defa</p>
  </td>
 </tr>
";
		$typename = "&nbsp;";
	    }
	}
    }


    print TABLE "
</table>

<a name=table$itable>&nbsp;</a>
<p><b>Table $itable. $title</b></p>

";
}

sub dotableattrqual($$) {

    my ($s,$title) = @_;
    my $a;
    my $havequal;

    if (!defined($section{$s})) {return 0}
    $havequal = 0;
    foreach $t (split(" ", $section{$s})) {
	$typename = $t;
	
	if (defined($acdqual{$t})) {
	    $havequal = 1;
	}
    }
    if (!$havequal) {return 0}
    $isubtable++;
    print TABLE "
<table border=1 cellpadding=0>
";
    print TABLE "
 <tr>
  <td>
  <p><b>Data type</b></p>
  </td>
  <td>
  <p><b>Qualifier definition</b></p>
  </td>
  <td>
  <p><b>Description</b></p>
  </td>
 </tr>
";

    foreach $t (split(" ", $section{$s})) {
	$typename = $t;

	if (defined($acdqual{$t})) {
	    foreach $q (split(" ", $acdqual{$t})) {
		$type = $acdqualtype{"$t\_$q"};
		$comm = $acdqualcomm{"$t\_$q"};
		$defa = $acdqualdefa{"$t\_$q"};
 
		if (defined($typedef{$type})) {
		    $typestring = "<i>$typedef{$type}<i>";
		}
		else {
		    $typestring = "<i>$type<i>";
		}
		if ($defa =~ /[(](.*)[)]/) {$defa = "<i>$defa</i>"}
		elsif ($type eq "string") {$defa = "\"$defa\""}
		print TABLE "
 <tr>
  <td>
  <p>$typename</p>
  </td>
  <td>
  <p>-$q: $typestring</p>
  </td>
  <td>
  <p>$comm<br>Default: $defa</p>
  </td>
 </tr>
";
		$typename = "&nbsp;";
	    }
	}
    }
    print TABLE "
</table>

<a name=table$itable$isubtable>&nbsp;</a>
<p><b>Table $itable\.$isubtable. $title</b></p>

";


}

sub dotableseq($) {

    my ($title) = @_;
    my $a;

    $itable++;

    print TABLE "
<table border=1 cellpadding=0>
";
    print TABLE "
 <tr>
  <td>
  <p><b>Value</b></p>
  </td>
  <td>
  <p><b>Type(s)</b></p>
  </td>
  <td>
  <p><b>Gaps</b></p>
  </td>
  <td>
  <p><b>Ambiguity codes</b></p>
  </td>
  <td>
  <p><b>Conversions</b></p>
  </td>
  <td>
  <p><b>Description</b></p>
  </td>
 </tr>
";

    foreach $t (@seqtype) {
	$gap = $seqgap{$t};
	$ambig = $seqambig{$t};
	$nucprot = $seqnucprot{$t};
	$from = $seqfrom{$t};
	$to = $seqto{$t};
	$comm = $seqcomm{"$t"};
	if ($gap eq "Yes") {$gaptext = "Kept"}
	else {$gaptext = "Removed"}

	$converttext = "&nbsp;";
	if ($from ne "") {
	    $converttext = "";
	    @convfrom = split(//, $from);
	    @convto = split(//, $to);
	    while ($cf = shift(@convfrom)) {
		$ct = shift(@convto);
		if ($cf !~ /[a-z]/) {
		    if ($converttext ne "") {$converttext .= "<br>"}
		    $converttext .= " '$cf'=&gt;'$ct'";
		}
	    }
	}
	if (defined($seqtype{$nucprot})) {$nucprot = $seqtype{$nucprot}};
	print TABLE "
 <tr>
  <td>
  <p>$t</p>
  </td>
  <td>
  <p>$nucprot</p>
  </td>
  <td>
  <p>$gaptext</p>
  </td>
  <td>
  <p>$ambig</p>
  </td>
  <td>
  <p>$converttext</p>
  </td>
  <td>
  <p>$comm</p>
  </td>
 </tr>
";
    }


    print TABLE "
</table>

<a name=table$itable>&nbsp;</a>
<p><b>Table $itable. $title</b></p>

";
}

sub dotablegroups($) {

    my ($title) = @_;
    my $a;

    $itable++;

    print TABLE "
<table border=1 cellpadding=0>
";
    print TABLE "
 <tr>
  <td>
  <p><b>Top Level</b></p>
  </td>
  <td>
  <p><b>Second Level</b></p>
  </td>
  <td>
  <p><b>Description</b></p>
  </td>
 </tr>
";

    foreach $g (sort (keys ( %groups ) ) ) {
	$grouplist = $groups{"$g"};
	if ($grouplist eq "") {	# top level group
		$comm = $groupcomm{"$g"};
		print TABLE "
 <tr>
  <td>
  <p>$g</p>
  </td>
  <td>
  <p>&nbsp;</p>
  </td>
  <td>
  <p>$comm</p>
  </td>
 </tr>
";
	}
	else {			# second level group list
	    $gtop = $g;
	    foreach $gg (split(" ", $groups{"$g"})) {
		$comm = $groupcomm{"$g:$gg"};
		print TABLE "
 <tr>
  <td>
  <p>$gtop</p>
  </td>
  <td>
  <p>$gg</p>
  </td>
  <td>
  <p>$comm</p>
  </td>
 </tr>
";
		$gtop = "&nbsp;";
	    }
	}
    }

    print TABLE "
</table>

<a name=table$itable>&nbsp;</a>
<p><b>Table $itable. $title</b></p>

";
}

sub dotablequals($) {

    my ($title) = @_;
    my $a;

    $itable++;

    print TABLE "
<table border=1 cellpadding=0>
";
    print TABLE "
 <tr>
  <td>
  <p><b>Qualifier definition</b></p>
  </td>
  <td>
  <p><b>Description</b></p>
  </td>
 </tr>
";

    foreach $q (@qual) {
	$defa = $qualdefa{"$q"};
	$comm = $qualcomm{"$q"};
	$type = $qualtype{"$q"};
	if (defined($typedef{$type})) {
	    $typestring = "<i>$typedef{$type}<i>";
	}
	else {
	    $typestring = "<i>$type<i>";
	}
	if ($defa =~ /[(](.*)[)]/) {$defa = "<i>$defa</i>"}
	elsif ($type eq "string") {$defa = "\"$defa\""}
	print TABLE "
 <tr>
  <td>
  <p>-$q $typestring</p>
  </td>
  <td>
  <p>$comm<br>Default: $defa</p>
  </td>
 </tr>
";
    }


    print TABLE "
</table>

<a name=table$itable>&nbsp;</a>
<p><b>Table $itable. $title</b></p>

";
}

sub dotablequalvars($) {

    my ($title) = @_;
    my $a;

    $itable++;

    print TABLE "
<table border=1 cellpadding=0>
";
    print TABLE "
 <tr>
  <td>
  <p><b>Environment variable</b></p>
  </td>
  <td>
  <p><b>Global qualifier</b></p>
  </td>
  <td>
  <p><b>Description</b></p>
  </td>
 </tr>
";

    foreach $q (@qual) {
	$uqual = uc($q);
	$defa = $qualdefa{"$q"};
	$comm = $qualcomm{"$q"};
	$type = $qualtype{"$q"};
	if (defined($typedef{$type})) {
	    $typestring = "<i>$typedef{$type}<i>";
	}
	else {
	    $typestring = "<i>$type<i>";
	}
	if ($defa =~ /[(](.*)[)]/) {$defa = "<i>$defa</i>"}
	elsif ($type eq "string") {$defa = "\"$defa\""}
	print TABLE "
 <tr>
  <td>
  <p>EMBOSS_$uqual</p>
  </td>
  <td>
  <p>-$q</p>
  </td>
  <td>
  <p>$comm<br>Default: $defa</p>
  </td>
 </tr>
";
    }


    print TABLE "
</table>

<a name=table$itable>&nbsp;</a>
<p><b>Table $itable. $title</b></p>

";
}

sub dotableothervars($) {

    my ($title) = @_;
    my $a;

    $itable++;

    print TABLE "
<table border=1 cellpadding=0>
";
    print TABLE "
 <tr>
  <td>
  <p><b>Environment variable</b></p>
  </td>
  <td>
  <p><b>Type</b></p>
  </td>
  <td>
  <p><b>Description</b></p>
  </td>
 </tr>
";

    foreach $q (@qual) {
	$uqual = uc($q);
	$getvalue{$uqual} = "ACD";
    }

    foreach $v (sort(keys(%getvalue))) {
	if ($getvalue{$v} eq "ACD") {
	    $uv = lc($v);
	    $comm = $qualcomm{$uv};
	    $type = $qualtype{$uv};
	    $defa = $qualdefa{$uv};
	}
	else {
	    $comm = $valuecomm{$v};
	    $type = $valuetype{$v};
	    $defa = $valuedefa{$v};
	}
	if (!defined($type)) {
	    print STDERR "Variable EMBOSS_$v undocumented in acdsyntax.getvalue\n";
	    $type = "unknown";
	}
	if (!defined($defa)) {$defa = "unknown"}
	if (!defined($comm)) {$comm = "undocumented"}
	if (defined($typedef{$type})) {
	    $typestring = "<i>$typedef{$type}<i>";
	}
	else {
	    $typestring = "<i>$type<i>";
	}
	if ($defa =~ /[(](.*)[)]/) {$defa = "<i>$defa</i>"}
	elsif ($type eq "string") {$defa = "\"$defa\""}
	print TABLE "
 <tr>
  <td>
  <p>EMBOSS_$v</p>
  </td>
  <td>
  <p>$type</p>
  </td>
  <td>
  <p>$comm<br>Default: $defa</p>
  </td>
 </tr>
";
    }


    print TABLE "
</table>

<a name=table$itable>&nbsp;</a>
<p><b>Table $itable. $title</b></p>

";
}

sub attrnumerically { $acdstatsattr{$t}{$b} <=> $acdstatsattr{$t}{$a} }
sub defanumerically { $acdstatsdefa{$t}{$b} <=> $acdstatsdefa{$t}{$a} }

##############################################
# Main body
##############################################

$stage = "";
while ($line = <>) {
    chomp $line;
    $stage = "";
    if ($line =~ /^\#\s*(\S.*)/) {
	if ($stages{$1}) {
	    $stage = $stages{$1};
	}
	else {
	    $stage = "OTHER";
	}
	while ($line =~ /^\#/) {
	    $line = <>;
	}
        print "Reading $stage\n";
    }

    if ($stage eq "TYPES") {doacdtypes();print "doacdtypes done\n"}
    if ($stage eq "DEFATTR") {doacddefattr();print "doacddefattr done\n"}
    if ($stage eq "CALCATTR") {doacdcalcattr();print "doacdcalcattr done\n"}
    if ($stage eq "SEQTYPE") {doseqtypes();print "doseqtypes done\n"}
    if ($stage eq "QUALS") {doacdquals();print "doacdquals done\n"}
  
}

open (GETVAL, "acdsyntax.getvalue") || die "Cannot open acdsyntax.getvalue";

while (<GETVAL>) {
    if (/^(\S+).*ajNamGetValueC?\s*[\(]\"([^\"]+)\"/) {
	$name=uc($2);
        $file = $1;
	$file =~ /\/([^\/.]+)[.]c:/;
	$module = $1;
	$getvalue{$name} = $module;
  }
}
close GETVAL;

open (GETVALDEF, "$basedir/emboss/acd/variables.standard")
    || die "Cannot open $basedir/emboss/acd/variables.standard";

while (<GETVALDEF>) {
    if (/^(\S+)\s+(\S+)\s+\"([^\"]*)\"\s+\"([^\"]*)\"/) {
	$name=uc($1);
	$type = $2;
	$default = $3;
	$comment = $4;
	$valuetype{$name} = $type;
	$valuedefa{$name} = $default;
	$valuecomm{$name} = $comment;
  }
}
close GETVALDEF;

open (GETGROUPS, "$basedir/emboss/acd/groups.standard")
    || die "Cannot open $basedir/emboss/acd/groups.standard";

while (<GETGROUPS>) {
    if (/^[\#]/) {next}
    if (/^(\S+)\s+(.*)/) {
	$group=$1;
	$comment = $2;
	if ($group =~ /([^:]+):(.*)/) {
	    $gtop = $1;
	    $gname = $2;
	    if (defined($groups{$gtop})) {$groups{$gtop} .= " $gname"}
	    else {$groups{$gtop} = "$gname"}
	}
	else {$groups{$group} = ""}
	$groupcomm{$group} = $comment;
  }
}
close GETGROUPS;

#foreach $x (sort(keys(%section))) {
#    print "Section $x $section{$x}\n";
#}

#foreach $x (sort(keys(%acddefdefa))) {
#    print "Default $x $acddeftype{$x} $acddefdefa{$x} $acddefcomm{$x}\n";
#}

#foreach $x (sort(keys(%acdattr))) {
#    print "ACD $x\n";
#    if (defined($acdcalc{$x})) {
#	print "  calc $acdcalc{$x}\n";
#    }
#    if (defined($acdattr{$x})) {
#	print "  attributes:\n";
#	foreach $y (split(" ", $acdattr{$x})) {
#	    $type = $acdattrtype{"$x\_$y"};
#	    $defa = $acdattrdefa{"$x\_$y"};
#	    $comm = $acdattrcomm{"$x\_$y"};
#	    print "    $y $type $defa $comm\n"; 
#	}
#    }
#    if (defined($acdqual{$x})) {
#	print "  qualifiers:\n";
#	foreach $y (split(" ", $acdqual{$x})) {
#	    $type = $acdqualtype{"$x\_$y"};
#	    $defa = $acdqualdefa{"$x\_$y"};
#	    $comm = $acdqualcomm{"$x\_$y"};
#	    print "    $y $type $defa $comm\n"; 
#	}
#    }
#}

open (TEMPLATE, "$basedir/doc/manuals/acdsyntax-template.html")
    || die "Cannot open $basedir/doc/manuals/acdsyntax-template.html";

$lcnt=0;
while (<TEMPLATE>) {

    $lcnt++;
    unless (/^<!-- *[\#](\S+) -->/) {
	if  (/^<!-- *[^\#]/) {
	    print STDERR "Unidentified comment in line $lcnt: $_";
	}

	if (/^<a name=([^>]+)>/)  {
	    $contentname = $1;
	    $contentname =~ s/\"//gos;
	}
	if (/^<h([1-3])>([^<]+)<\/h/) {
	    $level=$1;
	    while ($contentlevel > $level) {
		print CONTENTS "</ul>\n"; 
		$contentlevel--;
	    }
	    while ($contentlevel < $level) {
		print CONTENTS "<ul>\n"; 
		$contentlevel++;
	    }
	    print CONTENTS "<a href=\"syntax.html#$contentname\">$2</a><br>\n"
	}
	if ($testacdsect) {$testacdsect{$stype} .= $_}
	if ($testacdtype) {$testacdtype{$ttype} .= $_}
	if ($testattrsect) {$testattrsect{$sattr} .= $_}
	if ($testacdattr) {$testacdattr{$tattr} .= $_}
	print TABLE;
	next;
    }

    $insert = $1;
    $testacdsect = 0;
    $testacdtype = 0;
    $testattrsect = 0;
    $testacdattr = 0;

    if ($insert eq "tabletypes") {
	dotabletypes("Available Data Types/Objects in ACD.");
	$itable++;		# Table 2. Recommended naming conventions
	$itable++;		# For Table 3
	$isubtable=0;
    }
    elsif ($insert =~ /^acdtype-section-(\S+)/) {
	$stype = $1;
#  ("simple", "input", "selection", "output", "graph", "other") {
	if (!defined($section{$stype})) {
	    print STDERR "Line $lcnt: No acd types defined for section $stype\n";
	    next;
	}
	$donesection{$stype} = 1;
	$testacdsect = 1;
    }
    elsif ($insert =~ /^acdtype-(\S+)/) {
	$ttype = $1;
	if (!defined($acdtypes{$ttype})) {
	    print STDERR "Line $lcnt: Acd type $ttype not defined\n";
	    next;
	}
	$doneacdtype{$ttype} = 1;
	$testacdtype = 1;
    }
    elsif ($insert =~ /^acdattr-section-(\S+)/) {
	$sattr = $1;
#  ("simple", "input", "selection", "output", "graph", "other") {
	if (!defined($section{$sattr})) {
	    print STDERR "Line $lcnt: No acd types defined for section $sattr\n";
	    next;
	}
	$doneattrsection{$sattr} = 1;
	$testattrsect = 1;
    }
    elsif ($insert =~ /^acdattr-(\S+)/) {
	$tattr = $1;
	if (!defined($acdtypes{$tattr})) {
	    print STDERR "Line $lcnt: Acd attributes for type $tattr not defined\n";
	    next;
	}
	$doneacdattr{$tattr} = 1;
	$testacdattr = 1;
    }
    elsif ($insert =~ /^section-(\S+)/) {
	$s = $1;
#  ("simple", "input", "selection", "output", "graph", "other") {
	if (!defined($section{$s})) {
	    print STDERR "Line $lcnt: No attributes defined for section $s\n";
	    next;
	}
	$ts = ucfirst($s);
	dotabletypeattr($s, "$ts data types - attributes.");
    }

    elsif ($insert eq "tablegroups") {
	dotablegroups("Standard application groups");
    }
    elsif ($insert eq "tablecalc") {
	dotablecalc("Data type-specific calculated attributes.");
    }
    elsif ($insert eq "tableseq") {
	dotableseq("Possible values for the <i>type:</i> attribute in input sequence data types.");
    }
    elsif ($insert eq "tablequals") {
	dotablequals("Global qualifiers.");
    }
    elsif ($insert eq "tablequalvars") {
	dotablequalvars("Environment variables associated with global qualifiers.");
	$itable++;		# For Table 3
	$isubtable=0;
    }
    elsif ($insert =~ /^qual-(\S+)/) {
	$s = $1;
	$ts = ucfirst($s);
	dotableattrqual($s,"$ts qualifiers.");
    }
    elsif ($insert eq "tableothervars") {
	dotableothervars("Environment variables.");
    }
    else {
	print STDERR "Line $lcnt: Unknown instruction $insert\n";
    }
}

close TEMPLATE;

print TABLE "</BODY></HTML>\n";

close TABLE;
print CONTENTS "</BODY></HTML>\n";

close CONTENTS;

foreach $s (sort (keys ( %section))) {
    if (!defined($donesection{$s})) {
	print STDERR "Section $s acd types not documented\n";
    }
    if (!defined($doneattrsection{$s})) {
	print STDERR "Section $s attributes for acd types not documented\n";
    }
    $us = $sectionnames{$s};
    $spat = qr/^\s*<h4>2[.]3[.]1[.][0-9]\s+$us\s*(<a.*a>)?<.h4>/;
    if ($testacdsect{$s} !~ /$spat/g) {
	print STDERR "++ failed to find header for section $us\n";
###	print STDERR $testacdsect{$s};
    }
    $sapat = qr/^\s*<h5>2[.]4[.]1[.]2[.][0-9]\s+$us\s*(<a.*a>)?<.h5>/;
    if ($testattrsect{$s} !~ /$sapat/g) {
	print STDERR "++ failed to find header for attributes section $us\n";
###	print STDERR $testattrsect{$s};
    }
}

foreach $t (sort (keys ( %acdtypes))) {
    if (!defined($doneacdtype{$t})) {
	print STDERR "Acd type $t ($acdtypes{$t}) not documented\n";
    }
    if (!defined($doneacdattr{$t})) {
	print STDERR "Acd attributes for type $t ($acdtypes{$t}) not documented\n";
    }
    $ua = ucfirst($t);
    $tpat = qr/^\s*<h5>$ua<.h5>.*?<p>..*?<.p>/;
    if (defined($testacdtype{$t})) {
	$test = $testacdtype{$t};
	$test =~ s/\n/ /g;
	if ($test !~ /$tpat/gs) {
	    print STDERR "++ failed to find documentation for acd type $t ($acdtypes{$t})\n";
###	    print STDERR $testacdtype{$t};
	}
    }
    $tapat = qr/^\s*<h5>$ua<.h5>.*?<p>..*?<.p>/;
    if (defined($testacdattr{$t})) {
	$test = $testacdattr{$t};
	$test =~ s/\n/ /g;
	if ($test !~ /$tapat/gs) {
	    print STDERR "++ failed to find attribute documentation for acd type $t ($acdtypes{$t})\n";
###	    print STDERR $testacdattr{$t};
	}
	foreach $at ( sort attrnumerically  keys ( %{$acdstatsattr{$t}}) ) {
	    $tattr = qr/<i>$at:<.i>/;
	    if ($test !~ /$tattr/s) {
		print STDERR "++ failed to find attribute '$at' documented for acd type $t ($acdtypes{$t}) used $acdstatsattr{$t}{$at} / $acdstats{$t} times\n";
	    }
	}
	foreach $dt ( sort defanumerically  keys ( %{$acdstatsdefa{$t}}) ) {
	    $tattr = qr/<i>$dt:<.i>/;
	    if ($test !~ /$tattr/s) {
		print STDERR "++ failed to find default attribute '$dt' documented for acd type $t ($acdtypes{$t}) used $acdstatsdefa{$t}{$dt} / $acdstats{$t} times\n";
	    }
	}
    }
}

exit();

