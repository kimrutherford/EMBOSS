#!/usr/bin/perl -w

#############################################################################
#
# Processing new @section blocks for ajstr etc.
#
# Controls
#
# @filesection
# Expected to contain @nam1rule and perhaps @nam2rule
# which overwrite all later name rules
#
# @datasection
# subdivides for multiple types in one file e.g. ajstr.c
# expected to have @nam2rule
# which overwrites all later name rules
#
# @fdata [Datatype]
# should automatically pick up rules from an @datasection block
#
# @nam*rule Name Descriptive text
# describes a name element and its level
# if attached always to a lower level name, include both e.g. NewRes
# (or they could be simply nested if the name can appear anywhere)
#
# @suffix Name
# single letter suffix appended to any function name
# defined globally in @filesection or @datasection
# or just for a single section
#
# @argrule Name Argname [Argtype] Descriptive text
# attached to a name from @namrule or @suffix
# the argument name must appear in the order specified in the rules
# Name can (should) be * to apply to all functions in a section.
#
# @valrule Name [Valtype] Descriptive text
# The return value for a named set of functions.
# Name can (should) be * to apply to all functions in a section.
#############################################################################

use English;

sub nametowords($) {
    my ($name) = @_;
    my $fname = $name;
    $name =~ s/([A-Z])/ $1/go;
    my @nameparts = split(' ', $name);
#    print LOG "sub function $fname parts $#nameparts\n";
    return @nameparts;
}

sub nametorules($\@) {
    my ($name,$rules) = @_;
    my $fname = $name;
    my $ok = 1;
    if (!($name =~ s/^M//)) {return 0}

    print LOG "nametorules $fname\n";
    my $ilevel = 0;
    my $irule = 0;
    my $urule = "";
    my $nname = $name;
    my @nametorules = ();

    foreach $rulelevel (@$rules) {
	$ok = 1;
	$ilevel++;
	print LOG "nametorules level $ilevel\n";
	$irule = 0;
	$nname = $name;
	foreach $currule (@$rulelevel) {
	    $irule++;
	    print LOG "nametorules level $ilevel rule $irule\n";
	    $currule =~ s/([A-Z])/ $1/gos;
	    @ruleparts = split(' ', $currule);
	    $rule = pop(@ruleparts);
	    $urule = uc($rule);
	    print LOG "nametorules rule '$rule'\n";
	    if($nname =~ s/^$urule//) {
		print LOG "nametorules matched name: '...$nname'\n";
		$ok = 1;
		push(@nametorules, $rule);
		if($nname eq "") {last}
		next;
	    }
	    else {
		print LOG "nametorules no match\n";
		$ok = 0;
	    }
	}
	if($ok) {
	    if ($nname eq "") {
		print LOG "nametorules success\n";
		return @nametorules;
	    }
	    else {
		print LOG "nametorules matched up to: '...$nname'\n";
		$name = $nname;
	    }
	}
	else {
	    print LOG "nametorules not found '...$nname'\n";
	}
    }

    print LOG "nametorules failed $fname ok:$ok name: '$nname'\n";

    return 0;
}

sub testorder($@$) {
    my($lastname, @newparts, $type) = @_;
    print LOG "testorder '$lastname' '$name'\n";
    if($lastname eq "") {return 1}
    $lastname =~ s/([A-Z])/ $1/go;
    my @oldparts = split(' ', $lastname);
    my $o;
    foreach $o (@oldparts) {
	if($#newparts < 0) {return 0}
	$n = shift(@newparts);
	if($o =~ /^[A-Z]$/) {	# last name within suffix list
	    print LOG "testorder suffix '$n' '$o'\n";
	    if($n =~ /^[A-Z]$/) {
		if($n lt $o) {return 0}
		if($n gt $o) {return 1}
	    }
	    else {return 1}	# new name level
	}
	else {
	    print LOG "testorder name '$n' '$o'\n";
	    if($n lt $o) {return 0}
	    if($n gt $o) {return 1}
	}
    }
    if($#newparts >= 0) {return 1}
    # oops - names seem to be the same
    print LOG "testorder fail: identity\n";
    if($type eq "macro") {return 1} # macro can follow function of same name
    return 0;
}

sub issuffix($@) {
    my ($name,@suffixes) = @_;
    my $s;
    if($#suffixes < 0) {return 0}

    foreach $s (@suffixes) {
#	print LOG "issuffix '$name' '$s'\n";
	if ($name eq $s) {return 1}
    }

#    print LOG "issuffix failed\n";
    return 0;
}

sub isnamrule($\@@) {
    my ($i, $rules, @nameparts) = @_;
    my $j = $i-1;
#    print LOG "isnamrule ++ i: $i rules $#{$rules} names $#nameparts '$nameparts[$i]'\n";
    if($i > $#nameparts) {
#	print LOG "isnamrule i: $i names $#nameparts\n";
	return 0;
    }
    my $rule;
    my $r;
    my @ruleparts;
    my $ok;
    foreach $currule (@$rules) {
#	print LOG "isnamrule: rule '$currule'\n";
	$rule = $currule;
	$rule =~ s/([A-Z])/ $1/gos;
	@ruleparts = split(' ', $rule);
	$j = $i - $#ruleparts;
	if($j < 0) {next}
	$ok = 1;
	foreach $r (@ruleparts) {
#	    print LOG "isnamrule $j name: '$nameparts[$j]' rule '$r'\n";
	    if($nameparts[$j] ne $r) {$ok=0;last}
	    $j++;
	}
	if(!$ok) {next}
#	print LOG "isnamrule OK\n";
	return 1;
    }
#    print LOG "isnamrule all rules failed\n";
    return 0;
}

sub matchargname($$@) {
    my ($aname, $anum, @nameparts) = @_;
    my $j = $#nameparts;
    my $argname = $aname;
    $argname =~ s/^[*]//go;
    $argname =~ s/([A-Z])/ $1/go;
    my @argparts = split(' ', $argname);
    my $k = $#argparts;
    if($j < $k) {return 0} 	# argname longer than function name!
    my $curarg;
    my $ok;
    my $imax = $j - $k;
    my $i;
    my $ii;
    my $kk;
    my $n = "";
    my $sufcnt = 0;
    print LOG "matchargname '$aname' <$anum> '$fname' imax:$imax\n";
    print LOG "matchargname parts: \n";
    foreach $n (@nameparts) { print LOG " '$n'"}
    print LOG "\n";
    for ($i=0;$i<=$imax; $i++) {
	$ok = 1;
	$aname = "";
	$sufcnt = 0;
	for ($ii=0; $ii < $i; $ii++) {
	    if($nameparts[$ii] =~ /^[A-Z]$/) {
		print LOG "i:$i suffix '$nameparts[$ii]'\n";
		$sufcnt++;
	    }
	}
	print LOG "i:$i sufcnt: $sufcnt\n";
	for ($kk=0;$kk<=$k;$kk++) {
	    $ii = $i+$kk;
	    print LOG "matchargname test $nameparts[$ii] $argparts[$kk]\n";
	    if($nameparts[$ii] =~ /^[A-Z]$/) {$sufcnt++}
	    if($nameparts[$ii] ne $argparts[$kk]) {
		print LOG "matchargname reject $nameparts[$ii] $argparts[$kk]\n";
		$ok = 0;
		last;
	    }
	    $aname .= $nameparts[$ii];
	    print LOG "matchargname OK so far: $aname\n";
	}
	if($ok) {
	    print LOG "matchargname: matched i:$i '$aname' $sufcnt/$anum\n";
	    if($anum && ($sufcnt != $anum)) {next}
	    return 1;
	}
    }
    print LOG "matchargname failed\n";
    return 0;
}

sub srsref {
    return "<a href=\"http://srs.ebi.ac.uk/srs7bin/cgi-bin/wgetz?-e+[EFUNC-ID:$_[0]]\">$_[0]</a>";
}
sub srsdref {
    return "<a href=\"http://srs.ebi.ac.uk/srs7bin/cgi-bin/wgetz?-e+[EDATA-ID:$_[0]]\">$_[0]</a>";
}

sub secttest($$) {
    my ($sect, $ctype) = @_;
    my $stype = "";
    if ($sect =~ /Constructors$/) {$stype = "new"}
    elsif ($sect =~ /Destructors$/) {$stype = "delete"}
    elsif ($sect =~ /Assignments$/) {$stype = "assign"}
    elsif ($sect =~ /Iterators$/) {$stype = "iterate"}
    elsif ($sect =~ /Modifiers$/) {$stype = "modify"}
    elsif ($sect =~ /Casts$/) {$stype = "cast"}
    elsif ($sect =~ /Input$/) {$stype = "input"}
    elsif ($sect =~ /Output$/) {$stype = "output"}
    elsif ($sect =~ /Miscellaneous$/) {$stype = "misc"}
    if ($stype eq "") {return $stype}
    if ($stype ne $ctype) {
	print "bad category '$ctype' (expected '$stype') in section '$sect'\n";
    }
    return $stype;
}

sub testvar($) {
    my ($tvar) = @_;
    if (defined($cppreserved{$tvar})) {
	print "bad variable '$tvar' - reserved word in C++, use '$cppreserved{$tvar}'\n";
    }
}

sub testnew($$) {
    my ($tdata, $ttype) = @_;
    if ($tdata ne $ttype) {
	print "bad category new - return type '$ttype' datatype '$tdata'\n";
    }
}

sub testdelete($$\@\@) {
    my ($tdata, $ttype, $tcast, $tcode) = @_;
    if ($ttype ne "void") {
	print "bad category delete - return type '$ttype' non-void\n";
    }
    if ($#{$tcast} < 0) {
	print "bad category delete - parameter missing\n";
	return 0;
    }
    $tx = ${$tcode}[0];
    if ($#{$tcast} > 0) {
	print "bad category delete - only one parameter allowed\n";
	return 0;
    }
    if (${$tcast}[0] !~ /$tdata\*+/) {
	$tc = ${$tcast}[0];
	print "bad category delete - only parameter '$tc' must be '$tdata\*'\n";
    }
    if ($tx !~ /[d]/) {
	print "bad category delete - code1 '$tx' not 'd'\n";
    }
}

sub testassign($$\@\@) {
    my ($tdata, $ttype, $tcast, $tcode) = @_;
    if ($#{$tcast} < 0) {
	print "bad category assign - no parameters\n";
    }
    $tc = ${$tcast}[0];
    $tx = ${$tcode}[0];
    if ($tc ne "$tdata\*") {
	print "bad category assign - parameter1 '$tc' not '$tdata\*'\n";
    }
    if ($tx !~ /[w]/) {
	print "bad category assign - code1 '$tx' not 'w'\n";
    }
#    if ($tx !~ /[D]/) {
#	print "bad category assign - code1 '$tx' not 'D'\n";
#    }
}

sub testmodify($$\@\@) {
    my ($tdata, $ttype, $tcast, $tcode) = @_;
    if ($#{$tcast} < 0) {
	print "bad category modify - no parameters\n";
    }
    $tc = ${$tcast}[0];
    $tx = ${$tcode}[0];
    if(!defined($tc)) {
    print "testmodify tc undefined for $fname $pubout\n";
    }
    if ($tc ne "$tdata" && $tc ne "$tdata\*") {
	print "bad category modify - parameter1 '$tc' not '$tdata' or '$tdata\*'\n";
    }
    if ($tx !~ /[wu]/) {
	print "bad category modify - code1 '$tx' not 'w' or 'u'\n";
    }
}

sub testcast($$\@\@) {
    my ($tdata, $ttype, $tcast, $tcode) = @_;
    if ($#{$tcast} < 0) {
	print "bad category cast - no parameters\n";
	return 0;
    }
    if ($#{$tcast} == 0 && $ttype eq "void") {
	print "bad category cast - one parameter and returns void\n";
    }
    $tc = ${$tcast}[0];
    $tx = ${$tcode}[0];
    if ($tc ne "const $tdata") {
	print "bad category cast - parameter1 '$tc' not 'const $tdata'\n";
    }
    if ($tx !~ /[r]/) {
	print "bad category cast - code1 '$tx' not 'r'\n";
    }
}

sub testderive($$\@\@) {
    my ($tdata, $ttype, $tcast, $tcode) = @_;
    if ($#{$tcast} < 0) {
	print "bad category derive - no parameters\n";
	return 0;
    }
    if ($#{$tcast} == 0 && $ttype eq "void") {
	print "bad category derive - one parameter and returns void\n";
    }
    $tc = ${$tcast}[0];
    $tx = ${$tcode}[0];
    if ($tc ne "const $tdata") {
	print "bad category derive - parameter1 '$tc' not 'const $tdata'\n";
    }
    if ($tx !~ /[r]/) {
	print "bad category derive - code1 '$tx' not 'r'\n";
    }
}

sub testuse($\@\@) {
    my ($tdata, $tcast, $tcode) = @_;
    if ($#{$tcast} < 0) {
	print "bad category use - no parameters\n";
	return 0;
    }
    $qpat = qr/^const $tdata[*]*$/;
    $qpat2 = qr/^$tdata[*]* const[ *]*$/;
    $tc = ${$tcast}[0];
    $tx = ${$tcode}[0];
    $tc =~ s/^CONST /const /go;
    if ($tc !~ $qpat && $tc !~ $qpat2 && $tc ne "const void*") {
	print "bad category use - parameter1 '$tc' not 'const $tdata'\n";
    }
    if ($tx !~ /[r]/) {
	print "bad category use - code1 '$tx' not 'r'\n";
    }
}

sub testiterate($$$\@) {
    my ($tdata, $ttype, $tdesc, $tcast, $tcode) = @_;
    my ($itertype) = ($tdesc =~ /(^\S+)\s+iterator/);
    if (!$itertype) {
	print "bad category iterator - no type in description\n";
    }
    else {
	$tc = ${$tcast}[0];
	if ($ttype ne $itertype &&
	    $tc ne "$itertype" &&
	    $tc ne "$itertype\*") {
	    print "bad category iterate - type '$itertype' not referenced\n";
	}
    }
}

sub testinput($\@\@) {
    my ($tdata, $tcast, $tcode) = @_;
    my $ok = 0;
    my $i = 0;
    if ($#{$tcast} < 0) {
	print "bad category input - no parameters\n";
	return 0;
    }
	
    for ($i=0; $i <= $#{$tcast}; $i++) {
	$tc = ${$tcast}[$i];
	$tx = ${$tcode}[$i];
	if (($tc eq "$tdata" || $tc eq "$tdata*")&& ($tx =~ /[wu]/)) {
	    $ok = 1;
	}
    }
    if (!$ok) {
	print "bad category input - no parameter '$tdata' with code 'w' or 'u'\n";
    }
}

sub testoutput($\@\@) {
    my ($tdata, $tcast, $tcode) = @_;
    my $ok = 0;
    my $i = 0;
    if ($#{$tcast} < 0) {
	print "bad category output - no parameters\n";
	return 0;
    }
    for ($i=0; $i <= $#{$tcast}; $i++) {
	$tc = ${$tcast}[$i];
	$tx = ${$tcode}[$i];
	if ($tc eq "$tdata" || $tc eq "const $tdata") {
	    if  ($tx =~ /[ru]/) {
		$ok = 1;
	    }
	}
    }
    if (!$ok) {
	print "bad category output - no parameter (const) '$tdata' and code 'r' or 'u'\n";
    }
}

sub testmisc($\@\@) {
    my ($tdata, $tcast, $tcode) = @_;
    my $ok = 0;
    my $i = 0;
#    if ($#{$tcast} < 0) {
#	print "bad category misc - no parameters\n";
#	return 0;
#    }
#    for ($i=0; $i <= $#{$tcast}; $i++) {
#	$tc = ${$tcast}[$i];
#	$tx = ${$tcode}[$i];
#	if ($tc eq "$tdata" || $tc eq "const $tdata") {
#	    if  ($tx =~ /[ru]/) {
#		$ok = 1;
#	    }
#	}
#    }
#    if (!$ok) {
#	print "bad category misc - no parameter (const) '$tdata' and code 'r' or 'u'\n";
#    }
}

sub printsect($$) {
    my ($mysect,$mysrest) = @_;
    if ($mysect ne $lastfsect) {
	if(defined($dataname)) {
	    printdata($dataname,$datadesc);
	}
	if(${$ostr} =~ /\.\.\.\.lastsect\.\.\.\./) {
	    if(!$dosecttest) {$sectstr = ""}
	    elsif($sectstr !~ /[^ ]$/) {$sectstr = ""}
	    ${$ostr} =~ s/[.]+lastsect[.]+/$sectstr\n/;
	}
	my $mysname = $mysect;
	$mysname =~ s/\s+/_/;
	${$ostr} .= "<hr><h3><a name=\"sec_$mysname\">\n";
	${$ostr} .= "Section: $mysect</a></h3>\n";
	${$ostr} .= "$mysrest\n";
	${$ostr} .= "....lastsect....";
	$lastfsect = $mysect;

	$datastr .= " <a href=#sec_$mysname>$mysect</a>";
    }
}

sub printsectstatic($$) {
    my ($mysect, $mysrest) = @_;
    if ($mysect ne $laststatfsect) {
	if(defined($dataname)) {
	    printdatastatic($dataname,$datadesc);
	}
	if(${$ostr} =~ /\.\.\.\.lastsect\.\.\.\./) {
	    if(!$dosecttest) {$sectstrstatic = ""}
	    elsif($sectstrstatic !~ /[^ ]$/) {$sectstrstatic = ""}
	    ${$ostr} =~ s/[.]+lastsect[.]+/$sectstrstatic\n/;
	}
	my $mysname = $mysect;
	$mysname =~ s/\s+/_/;
	${$ostr} .= "<hr><h3><a name=\"sec_$mysname\">\n";
	${$ostr} .= "Section: $mysect</a></h3>\n";
	${$ostr} .= "$mysrest\n";
	${$ostr} .= "....lastsect....";
	$laststatfsect = $mysect;

	$datastrstatic .= " <a href=#sec_$mysname>$mysect</a>";
    }
}

sub printdata($$) {
    my ($mydata,$mydrest) = @_;
    if ($mydata ne $lastdsect) {
	if(${$ostr} =~ /\.\.\.\.lastdata\.\.\.\./) {
	    if(!$dosecttest) {$datastr = ""}
	    elsif($datastr !~ /[^ ]$/) {$datastr = ""}
	    ${$ostr} =~ s/[.]+lastdata[.]+/$datastr\n/;
	}
	my $mydname = $mydata;
	$mydname =~ s/\s+/_/;
	${$ostr} .= "<hr><h2><a name=\"data_$mydname\">\n";
	${$ostr} .= "Datatype: $mydata</a></h2>\n";
	${$ostr} .= "$mydrest\n";
	${$ostr} .= "....lastdata....";
	$lastdsect = $mydata;

	$filestr .= " <a href=#data_$mydname>$mydata</a>";
    }
}

sub printdatastatic($$) {
    my ($mydata, $mydrest) = @_;
    if ($mydata ne $laststatdsect) {
	if(${$ostr} =~ /\.\.\.\.lastdata\.\.\.\./) {
	    if(!$dosecttest) {$datastrstatic = ""}
	    elsif($datastrstatic !~ /[^ ]$/) {$datastrstatic = ""}
	    ${$ostr} =~ s/[.]+lastdata[.]+/$datastrstatic\n/;
	}
	my $mydname = $mydata;
	$mydname =~ s/\s+/_/;
	${$ostr} .= "<hr><h2><a name=\"data_$mydname\">\n";
	${$ostr} .= "Datatype: $mydata</a></h2>\n";
	${$ostr} .= "$mydrest\n";
	${$ostr} .= "....lastdata....";
	$laststatdsect = $mydata;

	$filestrstatic .= " <a href=#data_$mydname>$mydata</a>";
    }
}

$pubout = "public";
$local = "local";
$infile = "";
$lib = "unknown";
$countglobal=0;
$countstatic=0;
$countsection = 0;

@namrules = ();
@sufname = ();
@datalist = ();
$namrulesfilecount=$#namrules;
$namrulesdatacount=$#namrules;
$suffixfilecount=$#sufname;
$suffixdatacount=$#sufname;

$dosecttest = 0;
$datatype="undefined";
$unused = "";
$flastname = 0;

$filestr = "<p><b>Datatypes:</b> ";
$filestrstatic = "<p><b>Datatypes:</b> ";

$ftable = "";

$lastfname = "";

### cppreserved is a list of C++ reserved words not to be used as param names.
### test is whether to test the return etc.
### body is whether to print the body code

%cppreserved = ("this" => "thys", "bool" => "boule", "string" => "strng");
%test = ("func" => 1, "funcstatic" => 1, "funclist" => 0, "prog" => 0);
%body = ("func" => 1, "funcstatic" => 1, "funclist" => 1, "prog" => 1);

%categs = ("new" => 1, "delete" => 1, "assign" => 1, "modify" => 1,
	   "cast" => 1, "derive" => 1, "use" => 1, "iterate" => 1,
	   "input" => 1, "output" => 1, "misc" => 1);
%ctot = ();
if ($ARGV[0]) {$infile = $ARGV[0];}
if ($ARGV[1]) {$lib = $ARGV[1];}

foreach $x ("short", "int", "long", "float", "double", "char",
	    "size_t", "time_t",
	    "unsigned", "unsigned char",
	    "unsigned short", "unsigned int",
	    "unsigned long", "unsigned long int") {
    $simpletype{$x} = 1;
}

foreach $x ("ajshort", "ajint", "ajuint", "ajlong", "ajulong",
	    "jobject", "jstring", "jboolean", "jclass", "jint", "jbyteArray",
	    "AjBool", "AjStatus", "BOOL", "AjEnum", "PLFLT", "PLINT",
	    "VALIST") {
    $simpletype{$x} = 1;
}

foreach $x ("CallFunc", "AjMessVoidRoutine", "AjMessOutRoutine") {
    $functype{$x} = 1;
}

foreach $x ("datastatic", "alias", "attr") {
    $datatoken{$x} = 1;
}

foreach $x("plus") {
    $ignore{$x} = 1;
}

$source = "";

if ($infile) {
    (undef, $dir, $pubout) = ($infile =~ /^(([^\/.]*)\/)*([^\/.]+)(\.\S+)?$/);
##    ($dummy, $dir, $pubout) = ($infile =~ /(([^\/.]+)\/)?([^\/.]+)(\.\S+)?$/);
    $local = $pubout;
    if ($dir) {$lib = $dir}
    print "set pubout '$pubout' lib '$lib'\n";
    open (INFILE, "$infile") || die "Cannot open $infile";
    while (<INFILE>) {$source .= $_}
}
else {
    while (<>) {$source .= $_}
}

open (OBS, ">>deprecated.new");
print OBS "#$pubout\n";
open (HTML, ">$pubout.html");
open (HTMLB, ">$local\_static.html");
open (SRS, ">$pubout.srs");
open (LOG, ">$local.log");
$file = "$pubout\.c";
$title = "$file";
$out="";
$outstatic="";
$out .=  "<html><head><title>$title</title></head>\n";
$out .=  "<body bgcolor=\"#ffffff\">\n";
$outstatic .= "<html><head><title>$title</title></head>\n";
$outstatic .= "<body bgcolor=\"#ffffff\">\n";

$out .=  "<h1>$file</h1>\n...lastfile...";
$outstatic .= "<h1>$file</h1>\n...lastfile...";

$sect = $lastfsect = $laststatfsect = "";
$datasect = $lastdsect = $laststatdsect = "";
$mainprog = 0;
$functot = 0;
$datanum=0;
$secnum=0;
$datastr = " ";
$datastrstatic = " ";
$sectstr = " ";
$sectstrstatic = " ";
$fnum=0;
$ostr = \$out;
$datatitle = "";


##############################################################
## $source is the entire source file as a string with newlines
## step through each comment
## looking for extended JavaDoc style formatting
## $ccfull is the comment
## $rest is the rest of the file
##############################################################

# Process an entire block
# We process each part below

$fdata = "";

while ($source =~ m"[/][*][^*]*[*]+([^/*][^*]*[*]+)*[/]"gos) {
    $partnum=0;
    $mastertoken="undefined";
    $ccfull = $&;
    $rest = $POSTMATCH;

    ($cc) = ($ccfull =~ /^..\s*(.*\S)*\s*..$/gos);
    if (defined($cc)) {
	$cc =~ s/[* ]*\n[* ]*/\n/gos;
	$cc = " ".$cc;
    }
    else {
	$cc = "";
    }
    $type = "";
    $acnt = 0;
    $rtype = "";
    $ismacro = 0;
    $isprog = 0;
    $islist = 0;
    @largs = ();
    @savecode = ();
    @savevar = ();
    @savecast = ();
    $inputargs = "";
    $outputargs = "";
    $modifyargs = "";
    $returnargs = "";
    $longdesc = "";
    $shortdesc = "";
    $usetext = "See source code";
    $exampletext = "In preparation";
    $errtext = "See source code";
    $dependtext = "See source code";
    $othertext = "See other functions in this section";
    $availtext = "In release 5.0.0";
    $ctype = "";

    while ($cc =~ m/\s@((\S+)\s+([^@]*[^@\s]))/gos) {
	$data = $1;
	$token = $2;
	#print "<$token>\n";
	#print "$data\n";

	if(!$partnum) {$mastertoken = $token}
	$partnum++;
	if ($token eq "section")  {
	    $secnum++;
	    if($out =~ /\.\.\.\.lastsect\.\.\.\./) {
		if($sectstr !~ /[^ ]$/) {$sectstr = ""}
		$out =~ s/\.\.\.\.lastsect\.\.\.\./$sectstr\n/;
	    }
	    if($outstatic =~ /\.\.\.\.lastsect\.\.\.\./) {
		if($sectstrstatic !~ /[^ ]$/) {$sectstrstatic = ""}
		$outstatic =~ s/\.\.\.\.lastsect\.\.\.\./$sectstrstatic\n/;
	    }
	    $sectstr = "<p><b>Functions:</b> ";
	    $sectstrstatic = "<p><b>Functions:</b> ";

	    $ostr = \$out;
	    $countglobal++;
	    if($dosecttest && $sect ne "") {
		if($countsection == 0 && $countstatic == 0) {
		    print "bad section: '$sect' has no public or static functions\n";
		}
	    }
	    $countsection = 0;
	    ($sect, $srest) = ($data =~ /\S+\s+([^*\n]+)\s*(.*)/gos);
	    if(!defined($sect)) {
		print "bad section: cannot parse '$data'\n";
	    }
	    $sect =~ s/\s+/ /gos;
	    $sect =~ s/^ //gos;
	    $sect =~ s/ $//gos;
	    $srest =~ s/>/\&gt;/gos;
	    $srest =~ s/</\&lt;/gos;
	    $srest =~ s/\n\n/\n<p>\n/gos;
	    $srest =~ s/{([^\}]+)}/<a href="#$1">$1<\/a>/gos;
	    print "\nSection $sect\n";
	    print "-----------------------------\n";

	    push (@{$datasect{$datatitle}}, $sect);
	    $datasub = "$datatitle - $sect";
	    @{$datafunc{$datasub}} = ();

	    if($dosecttest) {
		@argnumb = ();
		@argpref = ();
		@argname = ();
		@argtype = ();
		@argdesc = ();
		@valname = ();
		@valtype = ();
		$lastfname = "";
		$fdata = "";
		$ctype = "";
		splice(@namrules, 1+$namrulesdatacount);
		splice(@namdescs, 1+$namrulesdatacount);
		splice(@sufname, 1+$suffixdatacount);
		splice(@sufdesc, 1+$suffixdatacount);
	    }
	}

	elsif ($token eq "fdata")  {
	    $dosecttest = 1;
	    if($mastertoken ne "section") {
		print "bad syntax \@$token must be in \@section\n";
	    }
	    ($fdata) =
		($data =~ /^\S+\s+[\[]([^\]]+)[\]]\s*(.*)/gos);
	    if(!defined($fdata)) {
		print "bad fdata: $data\n";
	    }
	    if($fdata ne $datatype) {
		print "bad fdata <$fdata> <$datatype>\n";
	    }
	}

	elsif ($token eq "datasection")  {
	    $datanum++;
	    if($out =~ /\.\.\.\.lastdata\.\.\.\./) {
		if($datastr !~ /[^ ]$/) {$datastr = ""}
		$out =~ s/\.\.\.\.lastdata\.\.\.\./$datastr\n/;
	    }
	    if($outstatic =~ /\.\.\.\.lastdata\.\.\.\./) {
		if($datastrstatic !~ /[^ ]$/) {$datastrstatic = ""}
		$outstatic =~ s/\.\.\.\.lastdata\.\.\.\./$datastrstatic\n/;
	    }
	    $datastr = "<p><b>Sections:</b> ";
	    $datastrstatic = "<p><b>Sections:</b> ";

	    $secnum=0;
	    $fnum=0;
	    $dosecttest = 1;
	    if($partnum != 1) {
		print "bad syntax \@$token must be at start\n";
	    }
	    $flastname = "";
	    ($datatype, $datadesc) =
		($data =~ /\S+\s+[\[]([^\]]+)[\]]\s*([^*\n]+)(.*)/gos);
	    if(!defined($datadesc)) {
		print "bad datasection: $data\n";
		next;
	    }
	    $dataname = $1;
	    $datadesc = $2;
	    $dataname =~ s/\s+/ /gos;
	    $dataname =~ s/^ //gos;
	    $dataname =~ s/ $//gos;
	    $datadesc =~ s/\s+/ /gos;
	    $datadesc =~ s/^ //gos;
	    $datadesc =~ s/ $//gos;

	    $datatitle = "$dataname: $datadesc";
	    push (@datalist, "$datatitle");

	    @{ $datasect{$datatitle} } = ();
	    $datastr = "<p><b>Sections:</b> ";
	    $datastrstatic = "<p><b>Sections:</b> ";

	    splice(@namrules, 1+$namrulesfilecount);
	    splice(@namdescs, 1+$namrulesfilecount);
	    splice(@sufname, 1+$suffixfilecount);
	    splice(@sufdesc, 1+$suffixfilecount);
	}

	elsif ($token eq "filesection")  {
	    $dosecttest = 1;
	    if($partnum != 1) {
		print "bad syntax \@$token must be at start\n";
	    }
	    $flastname = "";
	    splice (@namrules, 0);

	}

	elsif ($token eq "fnote")  {
	    if($mastertoken ne "section") {
		print "bad syntax \@$token must be in \@section\n";
	    }
	}

	elsif ($token eq "suffix")  {
	    # can be on its own or in a block?
	    ($sufname,$sufdesc) =
		($data =~ /\S+\s+(\S+)\s+(.*)/gos);
	    push(@sufname, $sufname);
	    push(@sufdesc, $sufdesc);
	}

	elsif ($token =~ /^nam([1-9])rule$/)  {
	    if($mastertoken ne "section" &&
	       $mastertoken ne "filesection" &&
	       $mastertoken ne "datasection") {
		print "bad syntax \@$token must be in \@filesection, \@datasection or \@section\n";
	    }
	    $i = $1 - 1;
	    ($namrule, $namdesc) = ($data =~ /\S+\s+(\S+)\s*(.*)/gos);
	    if(!defined($namdesc)) {
		print "bad namrule: $data\n";
		next;
	    }
	    print LOG "defined nam$i"."rule '$namrule'\n";
	    $namdesc =~ s/\n//;
	    $namdesc =~ s/[.]$//;
	    push(@{$namrules[$i]},$namrule);
	    push(@{$namdescs[$i]},$namdesc);
	}

	elsif ($token eq "valrule")  {
	    if($mastertoken ne "section") {
		print "bad syntax \@$token must be in \@section\n";
	    }
	    ($valname,$valtype,$valdesc) =
		($data =~ /\S+\s+(\S+)\s+[\[]([^\]]+)[\]]\s*(.*)/gos);
	    if(!defined($valdesc)) {
		print "bad valrule: $data\n";
		next;
	    }
	    $valdesc =~ s/\n//;
	    $valdesc =~ s/[.]$//;
	    push (@valname, $valname);
	    push (@valtype, $valtype);
	    push (@valdesc, $valdesc);
	}

	elsif ($token =~ /^arg(\d?)rule$/)  {
	    if($mastertoken ne "section") {
		print "bad syntax \@$token must be in \@section\n";
	    }
	    $argnumb = $1;
	    if ($argnumb ne "") {
		print LOG "$token argnumb: $argnumb\n";
	    }
	    ($argpref, $argname, $argtype, $argdesc) =
		($data =~ /\S+\s+(\S+)\s+(\S+)\s+[\[]([^\]]+[\]]?)[\]]\s*(.*)/gos);
	    if(!defined($argdesc)) {
		print "bad argrule: $data\n";
		next;
	    }
	    $argdesc =~ s/\n//;
	    $argdesc =~ s/[.]$//;
	    push (@argnumb, $argnumb);
	    push (@argpref, $argpref);
	    push (@argname, $argname);
	    push (@argtype, $argtype);
	    push (@argdesc, $argdesc);
	}

	elsif (!$dosecttest && $token eq "section")  {
	    if($partnum != 1) {
		print "bad syntax \@$token must be at start\n";
	    }
	    $out = \$out;
	    $countglobal++;
	    ($sect, $srest) = ($data =~ /\S+\s+([^*\n]+)\s*(.*)/gos);
	    $sect =~ s/\s+/ /gos;
	    $sect =~ s/^ //gos;
	    $sect =~ s/ $//gos;
	    $srest =~ s/>/\&gt;/gos;
	    $srest =~ s/</\&lt;/gos;
	    $srest =~ s/\n\n/\n<p>\n/gos;
	    $srest =~ s/{([^\}]+)}/<a href="#$1">$1<\/a>/gos;
	    print "Section $sect\n";
	}

	elsif ($token eq "func" || $token eq "prog")  {
	    if($partnum != 1) {
		print "bad syntax \@$token must be at start\n";
	    }
	    $ismacro = 0;
	    $isprog = 0;
	    $fnum++;
	    if ($token eq "prog") {
		$isprog = 1;
		$mainprog=1;
		if($functot) {
		    print "bad ordering - main program should come first\n";
		}
	    }
	    if($mainprog && !$isprog) {
		print "bad function prototype: not static after main program\n";
	    }
	    $ostr = \$out;
	    $countglobal++;
	    $functot++;
	    if($sect ne "") {$countsection++;}

	    printsect($sect,$srest);

	    $type = $token;
	    ($name, $frest) = ($data =~ /\S+\s+(\S+)\s*(.*)/gos);
	    ($ftype,$fname, $fargs) =
		$rest =~ /^\s*([^\(\)]*\S)\s+(\S+)\s*[\(]\s*([^{]*)[)]\s*[\{]/os;
	    $sectstr .= " <a href=#$name>$name</a>";
	    $ftype =~ s/^__noreturn +//;
	    if($isprog) {$progname = $name}
	    elsif(defined($datasub)) {
		push(@{$datafunc{$datasub}}, "$name");
	    }
	    print "Function $name\n";
	    ${$ostr} .= "<hr><h4><a name=\"$name\">\n";
	    ${$ostr} .= "Function</a> ".srsref($name)."</h4>\n";
	    if(!defined($fargs)) {
		print "bad function prototype: not parsed\n";
		$ftype = "unknown";
		$fname = "unknown";
		next;
	    }
	    if ($isprog && $fname eq "main") {$fname = $pubout}
	    $trest = $frest;
	    #if($frest =~ /<(.*)>/) {print "bad HTML tag <$1>\n"}
	    $frest =~ s/>/\&gt;/gos;
	    $frest =~ s/</\&lt;/gos;
	    $frest =~ s/\n\n/\n<p>\n/gos;
	    #${$ostr} .= "$frest\n";
	    $shortdesc = $frest;
	    $longdesc = $frest;

	    print SRS "ID $name\n";
	    print SRS "TY public\n";
	    print SRS "MO $pubout\n";
	    print SRS "LB $lib\n";
	    print SRS "XX\n";

	    $ftype =~ s/\s+/ /gos;
	    $ftype =~ s/ \*/\*/gos;
	    $fname =~ s/^[\(]//gos;
	    $fname =~ s/[\)]$//gos;
	    if ($fname =~ /^Java_org.*Ajax_([^_]+)$/) {
		$fname = "Ajax.".$1;
		if ($ftype =~ /JNIEXPORT+\s+(\S+)\s+JNICALL/) {
		    $ftype = $1;
		}
	    }
	    if ($isprog && $ftype ne "int") {print "bad main type (not int)\n"}
	    if (!$ftype) {print "bad function definition\n"}
	    if ($fname ne $name) {print "bad function name <$name> <$fname>\n"}
	    if (!$frest) {print "bad function '$name', no description\n"}
	
	    $trest =~ s/\n\n+$/\n/gos;
	    $trest =~ s/\n\n\n+/\n\n/gos;
	    $trest =~ s/\n([^\n])/\nDE $1/gos;
	    $trest =~ s/\n\n/\nDE\n/gos;
	    $trest =~ s/>/\&gt;/gos;
	    $trest =~ s/</\&lt;/gos;
	    chomp $trest;
	    print SRS "DE $trest\n";
	    print SRS "XX\n";

	    $fargs =~ s/\s+/ /gos;    # all whitespace is one space
	    $fargs =~ s/ ,/,/gos;   # no space before comma
	    $fargs =~ s/, /,/gos;   # no space after comma
	    $fargs =~ s/ *(\w+) *((\[[^\]]*\])+)/$2 $1/gos;   # [] before name
	    $fargs =~ s/(\*+)(\S)/$1 $2/g;  # put space after run of *
	    $fargs =~ s/ \*/\*/gos;         # no space before run of *
	    $fargs =~ s/ [\(]\* (\w+)[\)]/* $1/gos;  # remove fn arguments
	    $fargs =~ s/(\w+)\s?[\(][^\)]+[\)],/function $1,/gos; # ditto
	    $fargs =~ s/(\w+)\s?[\(][^\)]+[\)]$/function $1/gos;  # ditto
	    $fargs =~ s/\s*\(\*(\w+)[^\)]*\)/\* $1/gs;
#           print "**functype <$ftype> fname <$fname> fargs <$fargs>\n";
	    @largs = split(/,/, $fargs);
#           foreach $x (@largs) {
#	        print "<$x> ";
#           }
#           print "\n";
#           print "-----------------------------\n";
	}

	elsif ($token eq "funcstatic")  {
	    if($partnum != 1) {
		print "bad syntax \@$token must be at start\n";
	    }
	    $ismacro = 0;
	    $isprog = 0;
	    $fnum++;
	    $ostr = \$outstatic;
	    $countstatic++;

	    printsectstatic($sect, $srest);

	    $type = $token;
	    ($name, $frest) = ($data =~ /\S+\s+(\S+)\s*(.*)/gos);
	    ($unused,$ftype,$fname, $fargs) =
		$rest =~ /^\s*(__noreturn\s*)?static\s+([^\(\)]*\S)\s+(\S+)\s*[\(]\s*([^{]*)[)]\s*[\{]/os;
	    print "Static function $name\n";
	    $sectstrstatic .= " <a href=#$name>$name</a>";
	    ${$ostr} .= "<hr><h4><a name=\"$name\">\n";
	    ${$ostr} .= "Static function</a> ".srsref($name)."</h4>\n";
	    if(!defined($ftype)){
		print "bad static function prototype: not parsed\n";
		next;
	    }
	    if($mainprog) {
		if($name !~ /^$progname[_A-Z]/) {
		    print "bad name expected prefix '$progname\_'\n";
		}
	    }
	    $trest = $frest;
	    #if($frest =~ /<(.*)>/) {print "bad HTML tag <$1>\n"}
	    $frest =~ s/>/\&gt;/gos;
	    $frest =~ s/</\&lt;/gos;
	    $frest =~ s/\n\n/\n<p>\n/gos;
	    #${$ostr} .= "$frest\n";
	    $shortdesc = $frest;
	    $longdesc = $frest;

	    print SRS "ID $name\n";
	    print SRS "TY static\n";
	    print SRS "MO $pubout\n";
	    print SRS "LB $lib\n";
	    print SRS "XX\n";

	    if ($fname ne $name) {print "bad function name <$name> <$fname>\n"}
	    if (!$frest) {print "bad function '$name', no description\n"}

	    $ftype =~ s/\s+/ /gos;
	    $ftype =~ s/ \*/\*/gos;

	    $trest =~ s/\n\n+$/\n/gos;
	    $trest =~ s/\n\n\n+/\n\n/gos;
	    $trest =~ s/\n([^\n])/\nDE $1/gos;
	    $trest =~ s/\n\n/\nDE\n/gos;
	    $trest =~ s/>/\&gt;/gos;
	    $trest =~ s/</\&lt;/gos;
	    chomp $trest;
	    print SRS "DE $trest\n";
	    print SRS "XX\n";


	    $fargs =~ s/\s+/ /gos;    # all whitespace is one space
	    $fargs =~ s/ ,/,/gos;   # no space before comma
	    $fargs =~ s/, /,/gos;   # no space after comma
	    $fargs =~ s/ *(\w+) *((\[[^\]]*\])+)/$2 $1/gos;   # [] before name
	    $fargs =~ s/(\*+)(\S)/$1 $2/g;  # put space after run of *
	    $fargs =~ s/ \*/\*/gos;         # no space before run of *
	    $fargs =~ s/ [\(]\* (\w+)[\)]/* $1/gos;  # remove fn arguments
	    $fargs =~ s/(\w+)\s?[\(][^\)]+[\)],/function $1,/gos;  # ditto
	    $fargs =~ s/(\w+)\s?[\(][^\)]+[\)]$/function $1/gos;  # ditto
	    $fargs =~ s/\s*\(\*(\w+)[^\)]*\)/\* $1/gs;
	    @largs = split(/,/, $fargs);
	}

	elsif ($token eq "macro")  {
	    if($partnum != 1) {
		print "bad syntax \@$token must be at start\n";
	    }
	    $fnum++;
	    $ismacro = 1;
	    $ostr = \$out;
	    $countglobal++;
	    if($sect ne "") {$countsection++;}
	    
	    printsect($sect,$srest);

	    $type = $token; 
	    ($name, $mrest) = ($data =~ /\S+\s+(\S+)\s*(.*)/gos);
	    $fname = $name;
	    print "Macro $name\n";
	    $sectstr .= " <a href=#$name>$name</a>";
	    ### print "args '$margs'\n";
	    ${$ostr} .= "<hr><h4><a name=\"$name\">\n";
	    ${$ostr} .= "Macro</a> ".srsref($name)."</h4>\n";
	    $trest = $mrest;
	    #if($mrest =~ /<(.*)>/) {print "bad HTML tag <$1>\n"}
	    $mrest =~ s/>/\&gt;/gos;
	    $mrest =~ s/</\&lt;/gos;
	    $mrest =~ s/\n\n/\n<p>\n/gos;
	    #${$ostr} .= "$mrest\n";
	    $shortdesc = $mrest;
	    $longdesc = $mrest;

	    print SRS "ID $name\n";
	    print SRS "TY macro\n";
	    print SRS "MO $pubout\n";
	    print SRS "LB $lib\n";
	    print SRS "XX\n";

#           $ftype =~ s/\s+/ /gos;
#           $ftype =~ s/ \*/\*/gos;
#           if (!$ftype) {print "bad macro definition\n"}
#           if ($fname ne $name) {print "bad macro name <$name> <$fname>\n"}
#           if (!$frest) {print "bad macro '$name', no description\n"}

	    $trest =~ s/\n\n+$/\n/gos;
	    $trest =~ s/\n\n\n+/\n\n/gos;
	    $trest =~ s/\n([^\n])/\nDE $1/gos;
	    $trest =~ s/\n\n/\nDE\n/gos;
	    $trest =~ s/>/\&gt;/gos;
	    $trest =~ s/</\&lt;/gos;
	    chomp $trest;
	    print SRS "DE $trest\n";
	    print SRS "XX\n";
	}

	elsif ($token eq "funclist")  {
	    if($partnum != 1) {
		print "bad syntax \@$token must be at start\n";
	    }
	    $fnum++;
	    $ismacro = 0;
	    $isprog = 0;
	    $islist = 1;
	    $ostr = \$outstatic;
	    $countstatic++;

	    printsectstatic($sect, $srest);

	    $type = $token;
	    ($name, $mrest) = ($data =~ /\S+\s+(\S+)\s*(.*)/gos);
	    print "Function list $name\n";
	    $sectstrstatic .= " <a href=#$name>$name</a>";
	    ${$ostr} .= "<hr><h4><a name=\"$name\">\n";
	    ${$ostr} .= "Function list</a> ".srsref($name)."</h4>\n";
	    $trest = $mrest;
	    #if($mrest =~ /<(.*)>/) {print "bad HTML tag <$1>\n"}
	    $mrest =~ s/>/\&gt;/gos;
	    $mrest =~ s/</\&lt;/gos;
	    $mrest =~ s/\n\n/\n<p>\n/gos;
	    #${$ostr} .= "$mrest\n";
	    $shortdesc = $mrest;
	    $longdesc = $mrest;

	    print SRS "ID $name\n";
	    print SRS "TY list\n";
	    print SRS "MO $pubout\n";
	    print SRS "LB $lib\n";
	    print SRS "XX\n";

	    $trest =~ s/\n\n+$/\n/gos;
	    $trest =~ s/\n\n\n+/\n\n/gos;
	    $trest =~ s/\n([^\n])/\nDE $1/gos;
	    $trest =~ s/\n\n/\nDE\n/gos;
	    $trest =~ s/>/\&gt;/gos;
	    $trest =~ s/</\&lt;/gos;
	    chomp $trest;
	    print SRS "DE $trest\n";
	    print SRS "XX\n";
	}

	elsif ($token eq "param")  {
	    if($mastertoken ne "func" &&
	       $mastertoken ne "funcstatic" &&
	       $mastertoken ne "macro" &&
	       $mastertoken ne "funclist") {
		print "bad syntax \@$token must be in \@func, funcstatic, funclist or macro\n";
	    }
	    if (!$intable) {
		$ftable = "<p><table border=3>\n";
		$ftable .= "<tr><th>Type</th><th>Name</th><th>Read/Write</th><th>Description</th></tr>\n";
		$intable = 1;
	    }
	    ($code,$var,$cast, $prest) = ($data =~ m/[\[]([^\]]+)[\]]\s*(\S*)\s*[\[]([^\]]+[\]]?)[\]]\s*(.*)/gos);
	    if (!defined($code)) {
		print "bad \@param syntax:\n$data";
		next;
	    }

	    if($prest =~ /([^\{]+)[\{]([^\}]+)[\}]/) {
		if($usetext eq "See source code") {$usetext = ""}
		else {$usetext .= "<p>\n"}
		$usetext .= "<b>$var:</b> $2\n";
		$prest = $1;
	    }

#           print "code: <$code> var: <$var> cast: <$cast>\n";
#           print "-----------------------------\n";
	    $cast =~ s/ \*/\*/gos;         # no space before run of *
	    $cast =~ s/\{/\[/gos;	# brackets fixed
	    $cast =~ s/\}/\]/gos;	# brackets fixed

	    if ($code !~ /^[rwufdvo?][CENP]*$/) { # deleted OSU (all unused)
		print "bad code <$code> var: <$var>\n";
	    }

	    if($code =~ /^[rfv]/) {
		if($code =~ /^r/) {$codename = "Input"}
		elsif($code =~ /^f/) {$codename = "Function"}
		elsif($code =~ /^v/) {$codename = "Vararg"}
		$inputargs .= "<tr><td><b>$var:</b></td><td>($codename)</td><td>$prest</td></tr>";
	    }
	    elsif($code =~ /[wd]/) {
		if($code =~ /^w/) {$codename = "Output"}
		elsif($code =~ /^d/) {$codename = "Delete"}
		$outputargs .= "<tr><td><b>$var:</b></td><td>($codename)</td><td>$prest</td></tr>";
	    }
	    elsif($code =~ /[u]/) {
		if($code =~ /^u/) {$codename = "Modify"}
		$modifyargs .= "<tr><td><b>$var:</b></td><td>($codename)</td><td>$prest</td></tr>";
	    }
	    else {$codename = "Unknown"}

	    testvar($var);
	    if ($ismacro) {               # No code to test for macros
	    }
	    else {
		$curarg = $largs[$acnt];
		if (!defined($curarg)) {
		    print "bad argument \#$acnt not found in prototype for <$var>\n";
		}
		else {
		    ($tcast,$tname) = ($curarg =~ /(\S.*\S)\s+(\S+)/);
		    if (!defined($tname)) {
			$tcast = $curarg;
			if (!$var) {
			    if($curarg eq "...") {
				$var = $tname = "vararg";
			    }
			    else {
				print "bad argument \#$acnt parsing failed for '$curarg'\n";
				$var = "unknown";
				$tname = "undefined";
			    }
			}
			else {
				print "bad argument \#$acnt parsing failed for '$curarg'\n";
				$tname = "undefined";
			}
		    }
		    $castfix = $cast;
		    $castfix =~ s/^CONST +//go;
		    if (!$isprog && ($castfix ne $tcast)) {
			print "bad cast for $tname <$cast> <$tcast>\n";
		    }
		    if (!$isprog && ($var ne $tname)) {
			print "bad var <$var> <$tname>\n";
		    }
		}
	    }
	    $acnt++;

	    push @savecode, $code;
	    push @savevar,  $var;
	    push @savecast, $cast;

	    $drest = $prest;
	    $drest =~ s/\n\n+$/\n/gos;
	    $drest =~ s/\n\n\n+/\n\n/gos;
	    $drest =~ s/\n([^\n])/\nPD $1/gos;
	    $drest =~ s/\n\n/\nPD\n/gos;
	    $drest =~ s/>/\&gt;/gos;
	    $drest =~ s/</\&lt;/gos;
	    chomp $drest;
	    print SRS "PN [$acnt]\n";
	    print SRS "PA $code $var $cast\n";
	    print SRS "PD $drest\n";
	    print SRS "PX\n";

	    if (!$prest) {print "bad \@param '$var', no description\n"}
	    $ftable .= "<tr><td>$cast</td><td>$var</td><td>$codename</td><td>$prest</td></tr>\n";

	    if ($simpletype{$cast}) {
# Simple C types (not structs)
# and EMBOSS types that resolve to simple types
		if ($code !~ /r/) {
		    print "bad \@param '$var' pass by value, code '$code'\n";
		}
	    }
	    elsif ($functype{$cast}) {
# Known function types - C and EMBOSS-specific
		if ($code !~ /f/) {
		    print "bad \@param '$var' function type '$cast', code '$code'\n";
		}
	    }
	    elsif ($cast =~ / function$/) {
# other function types
		if ($code !~ /f/) {
		    print "bad \@param '$var' function type '$cast', code '$code'\n";
		}
	    }
	    elsif ($cast =~ /^const .*[*][*]/) {
# Tricky - we can be read-only
# or we can set to any const char* string (for example)
# e.g. pcre error pointers
# but can be d (e.g. in ajTableMapDel functions)
		if ($code !~ /[rwud]/) {
		    print "bad \@param '$var' const ** but code '$code'\n";
		}
	    }
	    elsif ($cast =~ /^const /) {
#If it starts const - except const type ** (see above) - it is const
# One exception: pcre has a "const int*" array that is set
		if ($cast =~ /const[^a-z].*[*]/)
		{
		    if ($code !~ /[rwud]/) {
			print "bad \@param '$var' const($cast) but code '$code'\n";
		    }
		}
		elsif ($code !~ /r/) {
		    print "bad \@param '$var' const but code '$code'\n";
		}
	    }
	    elsif ($cast =~ / const[^a-z]/) {
# also if it has an internal const
# For example char* const argv[] is "char* const[]"
# One exception: pcre has a "register const uschar*" array that is set
		if ($cast =~ / const[^a-z].*[*]/)
		{
		    if ($code !~ /[rwud]/) {
			print "bad \@param '$var' const($cast) but code '$code'\n";
		    }
		}
		elsif ($code !~ /r/) {
			print "bad \@param '$var' const($cast) but code '$code'\n";
		}
	    }
	    elsif ($cast =~ / const$/) {
# For char* const (so far no examples)
# There could be exceptions - but not yet!
		if ($code !~ /r/) {
		    print "bad \@param '$var' const($cast) but code '$code'\n";
		}
	    }
	    elsif ($cast =~ /^[.][.][.]$/) {
# varargs can be ...
		if ($code !~ /v/) {
		    print "bad \@param '$var' type '...' but code '$code'\n";
		}
	    }
	    elsif ($cast =~ /^va_list$/) {
# varargs can also be va_list down the list
# we did use 'a' for this instead of 'v' but it is too confusing
		if ($code !~ /v/) {
		    print "bad \@param '$var' type '$cast' but code '$code'\n";
		}
	    }
	    elsif ($cast =~ /^void[*]$/) {
# hard to check - can be read, write, update or delete
		if ($code =~ /[?]/) {
		    print "bad \@param '$var' code '$code'\n";
		}
	    }
	    elsif ($cast =~ /^void[*]+$/) {
# hard to check - can be read, write, update or delete
# Note: maybe we can put a placeholder in the @param cast
		if ($code =~ /[?]/) {
		    print "bad \@param '$var' code '$code'\n";
		}
	    }
	    elsif ($cast =~ /[\]]$/) {
# hard to check - can be read, write, update or delete
# because we can't use const for these
# Note: maybe we can put a placeholder in the @param cast
		if ($code =~ /[?]/) {
		    print "bad \@param '$var' code '$code'\n";
		}
		if ($code =~ /r/) {
		    if ($cast =~ /^CONST +/) {
			$cast =~ s/^CONST +//o;
		    }
		    else
		    {
			print "bad \@param '$var' code '$code' but '$cast'\n";
		    }
		}
	    }
	    elsif ($cast =~ /[*]+$/) {
# hard to check - can be read, write, update or delete
# because we can't use const for these
# Note: maybe we can put a placeholder in the @param cast
		if ($code =~ /[?]/) {
		    print "bad \@param '$var' code '$code'\n";
		}
		if ($code =~ /r/) {
		    if ($cast =~ /^CONST +/) {
			$cast =~ s/^CONST +//o;
		    }
		    else
		    {
			print "bad \@param '$var' code '$code' but '$cast'\n";
		    }
		}
	    }
	    else {
# Standard checks for anything else
		if ($code =~ /r/) {
		    print "bad \@param '$var' code '$code' but not const\n";
		}
		if ($code =~ /[?]/) {
		    print "bad \@param '$var' code '$code'\n";
		}
	    }
	}

	elsif ($token eq "return")  {
	    if($mastertoken ne "func" &&
	       $mastertoken ne "funcstatic" &&
	       $mastertoken ne "macro" &&
	       $mastertoken ne "funclist") {
		print "bad syntax \@$token must be in \@func, funcstatic, funclist or macro\n";
	    }
	    if (!$intable) {
		$ftable = "<p><table border=3>\n";
		$ftable .= "<tr><th>Type</th><th>Name</th><th>Read/Write</th><th>Description</th></tr>\n";
		$intable = 1;
	    }
	    ($rtype, $rrest) = ($data =~ /\S+\s+\[([^\]]+)\]\s*(.*)/gos);
	    if(!defined($rtype)) {
		print "bad return definition: not parsed\n";
		next;
	    }
	    if(!defined($ftype)) {$ftype = "unknown";}
	    if (!$ismacro && !$isprog && $rtype ne $ftype) {
		print "bad return type <$rtype> <$ftype>\n";
	    }
	    if (!$rrest && $rtype ne "void") {
		print "bad \@return [$rtype], no description\n";
	    }

	    if($rtype eq "void") {
		$returnargs = "<tr><td><b>$rtype:</b></td><td>No return value</td></tr>";
	    }
	    else {
		$returnargs = "<tr><td><b>$rtype:</b></td><td>$rrest</td></tr>";
	    }
	    $rrest =~ s/>/\&gt;/gos;
	    $rrest =~ s/</\&lt;/gos;
	    $ftable .= "<tr><td>$rtype</td><td>\&nbsp;</td><td>RETURN</td><td>$rrest</td></tr>\n";
	    $ftable .= "</table><p>\n";
	    $intable = 0;

	    $drest = $rrest;
	    $drest =~ s/^$/\n/gos;  # make sure we have something
	    $drest =~ s/\n\n+$/\n/gos;
	    $drest =~ s/\n\n\n+/\n\n/gos;
	    $drest =~ s/\n([^\n])/\nRD $1/gos;
	    $drest =~ s/\n\n/\nRD\n/gos;
	    $drest =~ s/>/\&gt;/gos;
	    $drest =~ s/</\&lt;/gos;
	    chomp $drest;
	    print SRS "RT $rtype\n";
	    print SRS "RD $drest\n";
	    print SRS "RX\n";
	}

	elsif ($token eq "fcategory")  {
	    if($mastertoken ne "section") {
		print "bad syntax \@fcategory must be in \@section\n";
	    }
	    ($ctype, $crest) = ($data =~ /\S+\s+(\S+)\s*(.*)/gos);
	    if ($crest) {
		print "bad \@$token [$ctype], extra text\n";
	    }

	    $ctot{$ctype}++;
	    secttest($sect,$ctype);
	    if (!defined($categs{$ctype})) {
		print "bad \@fcategory $ctype - unknown type\n";
	    }
	}

	elsif ($token eq "category")  {
	    if($mastertoken ne "func" &&
		   $mastertoken ne "funcstatic" &&
	       $mastertoken ne "macro") {
		print "bad syntax \@category must be in \@func, funcstatic, or macro\n";
	    }
	    ($ctype, $cdata, $crest) = ($data =~ /\S+\s+(\S+)\s+\[([^\]]+)\]\s*(.*)/gos);
	    if (!$crest) {
		print "bad \@$token [$ctype], no description\n";
	    }

	    $crest =~ s/\s+/ /gos;
	    $crest =~ s/^ //gos;
	    $crest =~ s/ $//gos;
	    $crest =~ s/>/\&gt;/gos;
	    $crest =~ s/</\&lt;/gos;

	    $drest = $crest;
	    $drest =~ s/^$/\n/gos;  # make sure we have something
	    $drest =~ s/\n\n+$/\n/gos;
	    $drest =~ s/\n\n\n+/\n\n/gos;
	    $drest =~ s/\n([^\n])/\nCD $1/gos;
	    $drest =~ s/\n\n/\nCD\n/gos;
	    $drest =~ s/>/\&gt;/gos;
	    $drest =~ s/</\&lt;/gos;
	    chomp $drest;
	    print SRS "CA $ctype\n";
	    print SRS "CT $cdata\n";
	    print SRS "CD $drest\n";
	    print SRS "CX\n";

###	    print "category $ctype [$cdata] $fname $pubout $lib : $crest\n";
	    $ctot{$ctype}++;
	    secttest($sect,$ctype);

	    if ($dosecttest && $fdata ne "") {
		$cdata = $fdata;
	    }
	    if (!defined($categs{$ctype})) {
		print "bad \@$type [$ctype], unknown type\n";
	    }
	    elsif ($ctype eq "new") {
		testnew($cdata,$rtype);
	    }
	    elsif  ($ctype eq "delete") {
		testdelete($cdata, $rtype,@savecast,@savecode);
	    }
	    elsif  ($ctype eq "assign") {
		testassign($cdata,$rtype,@savecast,@savecode);
	    }
	    elsif  ($ctype eq "modify") {
		testmodify($cdata,$rtype,@savecast,@savecode);
	    }
	    elsif  ($ctype eq "cast") {
		testcast($cdata,$rtype,@savecast,@savecode);
	    }
	    elsif  ($ctype eq "derive") {
		testderive($cdata,$rtype,@savecast,@savecode);
	    }
	    elsif  ($ctype eq "use") {
		testuse($cdata,@savecast,@savecode);
	    }
	    elsif  ($ctype eq "iterate") {
		testiterate($cdata,$rtype,$crest,@savecast);
	    }
	    elsif  ($ctype eq "input") {
		testinput($cdata,@savecast,@savecode);
	    }
	    elsif  ($ctype eq "output") {
		testoutput($cdata,@savecast,@savecode);
	    }
	    elsif  ($ctype eq "misc") {
		testmisc($cdata,@savecast,@savecode);
	    }
	    else {
		print "bad category type '$ctype' - no validation\n";
	    }
	}

	elsif ($token eq "header")  {
	    if($partnum != 1) {
		print "bad syntax \@$token must be at start\n";
	    }
	    next;
	}

	elsif ($token eq "short")  {
	    if($mastertoken ne "func" &&
	       $mastertoken ne "funcstatic" &&
	       $mastertoken ne "macro") {
		print "bad syntax \@$token must be in \@func, funcstatic, or macro\n";
	    }
	    ($shortdesc) = ($data =~ /\S+\s+(.*)/);
	    $shortdesc =~ s/>/\&gt;/gos;
	    $shortdesc =~ s/</\&lt;/gos;
	    $shortdesc =~ s/\n\n/\n<p>\n/gos;
	}

	elsif ($token eq "release")  {
	    if($mastertoken ne "func" &&
	       $mastertoken ne "funcstatic" &&
	       $mastertoken ne "macro") {
		print "bad syntax \@$token must be in \@func, funcstatic, or macro\n";
	    }
	    ($availtext) = ($data =~ /\S+\s+(.*)/);
	    $availtext =~ s/\s+$//gos;
	    if($availtext =~ /^(\d+[.][.\d]+)$/) {
		$availtext = "EMBOSS $1";
	    }
	    $availtext =~ s/>/\&gt;/gos;
	    $availtext =~ s/</\&lt;/gos;
	    $availtext =~ s/\n\n/\n<p>\n/gos;
	}

	elsif ($token eq "cc")  {
	    if($mastertoken ne "func" &&
	       $mastertoken ne "funcstatic" &&
	       $mastertoken ne "macro") {
		print "bad syntax \@$token must be in \@func, funcstatic, or macro\n";
	    }
	    next;
	}

	elsif ($token eq "obsolete")  {
	    if($partnum != 1) {
		print "bad syntax \@$token must be at start\n";
	    }
	    ($oname, $norest) =
		($data =~ /\S+\s+(\S+)\s*(.*)/gos);
	    if($norest) {
		print "bad obsolete $oname - extra text\n"
	    }
	    $replaces = "";
	    if ($rest =~ /^\s*__deprecated\s+([^\(\)]*\S)\s+(\S+)\s*[\(]\s*([^{]*)[)]\s*[\{]/os) {
		$ofname = $2;
		$ofname =~ s/^[*]+//;
		if ($oname ne $ofname) {
		    print "bad obsolete function name <$ofname> <$oname>\n";
		}
	    }
	    else {
		print "bad obsolete function $oname - not __deprecated\n";
	    }
	    next;
	}

	elsif ($token eq "rename")  {
	    if($mastertoken ne "obsolete") {
		print "bad syntax \@$token must be in \@obsolete\n";
	    }
	    if($partnum == 1) {
		print "bad syntax \@$token cannot be the start\n";
	    }
	    ($rename, $norest) =
		($data =~ /\S+\s+(\S+)\s*(.*)/gos);
	    if($norest) {
		print "bad rename $oname $rename - extra text\n";
		next;
	    }
	    print OBS "$oname $rename\n";
	    next;
	}

	elsif ($token eq "replace")  {
	    if($mastertoken ne "obsolete") {
		print "bad syntax \@$token must be in \@obsolete\n";
	    }
	    if($partnum == 1) {
		print "bad syntax \@$token cannot be the start\n";
	    }
	    ($replace, $repargs, $norest) =
		($data =~ /\S+\s+(\S+)\s+[\(]([^\)]+)[\)]\s*(.*)/gos);
	    if(!defined($repargs)){
		print "bad replace $oname value: failed to parse\n";
		next;
	    }
	    if($repargs ne "") {
		($repold, $repnew) = split('/', $repargs);
		@repold = split(',', $repold);
		@repnew = split(',', $repnew);
		print OBS "$oname =$replace $repold $repnew\n";
	    }
	    else {
		print "bad replace $oname $replace - no arguments\n";
		next;
	    }
	    if($norest) {
		print "bad replace $oname $replace - extra text\n";
		next;
	    }

	    if($replaces ne "") {
		$replaces .= "_or_\@$replace";
	    }
	    else {
		$replaces = "\@$replace";
	    }
	    next;
	}

	elsif ($token eq "remove")  {
	    if($mastertoken ne "obsolete") {
		print "bad syntax \@$token must be in \@obsolete\n";
	    }
	    if($partnum == 1) {
		print "bad syntax \@$token cannot be the start\n";
	    }
	    ($delrest) =
		($data =~ /\S+\s*(.*)/gos);
	    if(!$delrest) {
		print "bad remove $oname - no explanation\n";
		next;
	    }
	    print OBS "$oname -\n";
	    next;
	}

	elsif ($token eq "source")  {
	    if($partnum != 1) {
		print "bad syntax \@$token must be at start\n";
	    }
	    next;
	}

	elsif ($token eq "author")  {
	    if($mastertoken ne "source") {
		print "bad syntax \@$token must be in \@source\n";
	    }
	    next;
	}

	elsif ($token eq "version")  {
	    if($mastertoken ne "source") {
		print "bad syntax \@$token must be in \@source\n";
	    }
	    next;
	}

	elsif ($token eq "modified")  {
	    if($mastertoken ne "source") {
		print "bad syntax \@$token must be in \@source\n";
	    }
	    next;
	}

	elsif ($token eq "error")  {
	    if($mastertoken ne "func" &&
	       $mastertoken ne "funcstatic" &&
	       $mastertoken ne "macro") {
		print "bad syntax \@$token must be in \@func, funcstatic, or macro\n";
	    }
	    next;
	}

	elsif ($token eq "cre")  {
	    if($mastertoken ne "func" &&
	       $mastertoken ne "funcstatic" &&
	       $mastertoken ne "macro") {
		print "bad syntax \@$token must be in \@func, funcstatic, or macro\n";
	    }
	    next;
	}

	elsif ($token eq "see")  {
	    if($mastertoken ne "func" &&
	       $mastertoken ne "funcstatic" &&
	       $mastertoken ne "macro") {
		print "bad syntax \@$token must be in \@func, funcstatic, or macro\n";
	    }
	    next;
	}

	elsif ($token eq "ure")  {
	    if($mastertoken ne "func" &&
	       $mastertoken ne "funcstatic" &&
	       $mastertoken ne "macro") {
		print "bad syntax \@$token must be in \@func, funcstatic, or macro\n";
	    }
	    next;
	}

	elsif ($datatoken{$token}) {
	}
	elsif ($categs{$token}) {
	}
	elsif ($ignore{$token}) {
	}
	elsif ($token eq "@")  {
	    if($partnum == 1) {
		print "bad syntax \@$token cannot be the start\n";
	    }
	    last;
	}
	else {
	    print "Unknown tag '\@$token\n";
	}
    }

# Whole block read.
# Post-processing

    if($dosecttest) {
	if($mastertoken eq "obsolete") {
	    if($replaces ne "") {
		print OBS "$oname $replaces\n";
	    }
	}

	if($mastertoken eq "filesection") {
	    $namrulesfilecount=$#namrules;
	    $suffixfilecount=$#sufname;
	}
	if($mastertoken eq "datasection") {
	    $namrulesdatacount=$#namrules;
	    $suffixdatacount=$#sufname;
	}

	if($mastertoken eq "section") {
	    if($fdata eq "") {
		print "bad section: '$sect' no fdata $datatype assumed\n";
	    }
	    if($ctype eq "") {
		print "bad section: '$sect' no fcategory\n";
	    }
	}
    }

    if ($type) {
#       print "acnt: $acnt largs: $#largs\n";
#       print "type $type test $test{$type}\n";

	if ($dosecttest && $type eq "func") { # not funcstatic or funclist
	    if($type eq "macro") {
		@nameparts = nametorules($fname, @namrules);
	    }
	    else {
		@nameparts = nametowords($fname);
	    }
	    if(!testorder($lastfname, @nameparts, $type)) {
		print "bad order: Function $fname follows $lastfname\n";
	    }
	    if($type eq "macro") {
		$lastfname = "";
		foreach $n(@nameparts) {
		    $lastfname .= $n;
		}
		print LOG "Macro lastfname '$lastfname'\n";
	    }
	    else {
		$lastfname = $fname;
	    }
	    print LOG "function $fname ...\n";

# Function name compared to naming rules

	    $i=0;
	    foreach $f (@nameparts) {
		$j = $i+1;
#		print LOG "name $j '$f'\n";
		if(defined($namrules[$i]) && ($f eq $namrules[$i])) {
#		    print LOG "namecheck OK\n";
		}
		elsif(issuffix($f,@sufname)) {
#		    print LOG "namecheck OK suffix\n";
		}
		else {
		    if(defined($namrules[$i])) {
#			print LOG "calling isnamrule i: $i rules $#{$namrules[$i]} names $#nameparts\n";
			if(!isnamrule($i, @{$namrules[$i]}, @nameparts)) {
			    print "bad name $fname: '$f' not found\n";
			    last;
			}
		    }
		    else {
			print "bad name $fname: '$f' beyond last rule\n";
			last;
		    }
		}
		$i++;
	    }

# parameters compared to argument rules

# First we use the name to generate a list of arguments

	    @genargname=();
	    @genargtype=();
	    @genvalname=();
	    @genvaltype=();
	    $i=0;
	    foreach $a (@argpref) {
		print LOG "argrule '$a' $argnumb[$i] testing $fname\n";
		$j = $i+1;
#		print LOG "argrule $j '$a' [$argtype[$i]] '$argdesc[$i]'\n";
		if(($a eq "*") || matchargname($a, $argnumb[$i], @nameparts)) {
#		    print LOG "argrule used: '$a' $argname[$i] [$argtype[$i]]\n";
		    push (@genargname, $argname[$i]);
		    push (@genargtype, $argtype[$i]);
		}
		$i++;
	    }

### return value = "*" for default, may also have a specific value

	    $valtypeall = "";
	    $i=0;
	    foreach $v (@valname) {
		$vv = $v;
#		print LOG "valrule '$v' testing $fname\n";
		$j = $i+1;
#		print LOG "valrule $j '$v' [$valtype[$i]]'\n";
		if(matchargname($v, 0, @nameparts)) {
#		    print LOG "valrule used: '$vv' [$valtype[$i]]\n";
		    if($vv =~ /^[*](.+)/) {
			$vv = $1;
			@genvalname = ();
			@genvaltype = ();
		    }
		    push (@genvaltype, $valtype[$i]);
		    push (@genvalname, $vv);
		}
		if($vv eq "*") {
		    $valtypeall = $valtype[$i];
		}
		$i++;
	    }
	    if($valtypeall ne "") {
		print LOG "valrule * [$valtypeall]\n";
		if(!defined($genvaltype[0])) {
#		    print LOG " valrule * [$valtypeall] used\n";
		    push (@genvaltype, $valtypeall);
		}
	    }
	    $i=0;
	    foreach $x (@genargname) {
		if(!defined($savevar[$i])) {
		    print LOG "++ arg '$x' [$genargtype[$i]] ... <undefined>\n";
		}
		else {
		    print LOG "++ arg '$x' [$genargtype[$i]] ... $savevar[$i] [$savecast[$i]]\n";
		    if($x ne $savevar[$i]) {
			print "bad param name <$savevar[$i]> rule <$x> \n";
		    }
		    if($genargtype[$i] ne $savecast[$i]) {
			print "bad param type <$savevar[$i]> [$savecast[$i]] rule <$x> [$genargtype[$i]]\n";
		    }
		}
		$i++;
	    }
#
	    $isave = $#savevar + 1;
	    $igen=$#genargname + 1;
	    if($igen < $isave) {
		print "bad argrule: $igen/$isave params defined\n";
	    }
	    elsif($igen > $isave) {
		print "bad argrule: expected $isave params, found $igen\n";
	    }

	    if($#genvaltype <0) {
		print "bad valrule: no matching rule\n"
	    }
	    elsif($#genvaltype >0) {
		$igenvaltype = $#genvaltype+1;
		print "bad valrule: $igenvaltype matching rules:";
		foreach $g(@genvalname) {
		    print "<$g>";
		}
		print "\n";
	    }
	    else {
		print LOG "++ val [$genvaltype[0]] ... [$rtype]\n";
		if($rtype ne $genvaltype[0]) {
		    print "bad return <$rtype> rule <$genvaltype[0]>\n";
		}
	    }
	    if($dosecttest && $fdata ne "") {
		$cdata = $fdata;
	    }
	    if ($ctype eq "") {
		# already an error above
	    }
	    elsif ($ctype eq "new") {
		testnew($fdata,$rtype);
	    }
	    elsif  ($ctype eq "delete") {
		testdelete($fdata, $rtype,@savecast,@savecode);
	    }
	    elsif  ($ctype eq "assign") {
		testassign($fdata,$rtype,@savecast,@savecode);
	    }
	    elsif  ($ctype eq "modify") {
		testmodify($fdata,$rtype,@savecast,@savecode);
	    }
	    elsif  ($ctype eq "cast") {
		testcast($fdata,$rtype,@savecast,@savecode);
	    }
	    elsif  ($ctype eq "derive") {
		testderive($fdata,$rtype,@savecast,@savecode);
	    }
	    elsif  ($ctype eq "use") {
		testuse($fdata,@savecast,@savecode);
	    }
	    elsif  ($ctype eq "iterate") {
		testiterate($fdata,$rtype,$crest,@savecast);
	    }
	    elsif  ($ctype eq "input") {
		testinput($fdata,@savecast,@savecode);
	    }
	    elsif  ($ctype eq "output") {
		testoutput($fdata,@savecast,@savecode);
	    }
	    elsif  ($ctype eq "misc") {
		testmisc($fdata,@savecast,@savecode);
	    }
	    else {
		print "bad category type '$ctype' - no validation\n";
	    }
	}

	if ($test{$type}) {
	    if ($acnt == $#largs) {
		if ($largs[$#largs] ne "void") {
		    print "bad last argument: $largs[$#largs]\n";
		    if(!$acnt) {
			for ($ii=0;$ii<=$#largs;$ii++) {
			    ($itcast,$itname) = ($largs[$ii] =~ /(\S.*\S)\s+(\S+)/);
			    if($itcast =~ /[*]/)
			    {
				print "** \@param [u] $itname [$itcast] Undocumented\n";
			    }
			    else
			    {
				print "** \@param [r] $itname [$itcast] Undocumented\n";
			    }
			}
		    }
		}
	    }
	    if ($acnt < $#largs) {   # allow one remaining
		$w=$#largs+1;
		print "bad \@param list $acnt found $w wanted\n";
		if(!$acnt) {
		    for ($ii=0;$ii<=$#largs;$ii++) {
			($itcast,$itname) = ($largs[$ii] =~ /(\S.*\S)\s+(\S+)/);
			if($itcast =~ /[*]/)
			{
			    print "** \@param [u] $itname [$itcast] Undocumented\n";
			}
			else
			{
			    print "** \@param [r] $itname [$itcast] Undocumented\n";
			}
		    }
		}
	    }
	    if(!defined($ftype)) {$ftype = "unknown"}
	    if (!$rtype && $ftype ne "void") {print "bad missing \@return\n"}
	    print "=============================\n";
	}
	print SRS "//\n";

	if($shortdesc) {
	    ${$ostr} .= "$shortdesc\n";
	}

##############################################################
## do we want to save what follows the comment?
## Yes, for functions (and static functions) and main programs
## $rest is what follows the comment
##############################################################

	if (defined($body{$type}) && $body{$type} == 1) {

# body is the code up to a '}' at the start of a line

	    ($body) = ($rest =~ /(.*?\n\}[^\n]*\n)/os);
	    if(!defined($body)) {
		print "bad code body, closing brace not found\n";
		$body = "\n";
	    }
	    print SRS $body;

	    if(defined($fname)) {
		${$ostr} .= "<h4>Synopsis</h4>";
		${$ostr} .= "<h5>Prototype</h5><pre>";
		${$ostr} .= "\n$ftype $fname (";
		$firstarg = 1;
		foreach $a (@largs) {
		    if($firstarg) {
			${$ostr} .= "\n      $a";
		    }
		    else {
			${$ostr} .= ",\n      $a";
		    }
		    $firstarg = 0;
		}
		if($firstarg) {
		    ${$ostr} .= "void);\n</pre>\n";
		}
		else {
		    ${$ostr} .= "\n);\n</pre>\n";
		}
		if($ftable ne "") {
		    ${$ostr} .= $ftable;
		    $ftable = "";
		}
	    }
	}

	if (defined($test{$type}) && $test{$type} == 2) {

# body is the code up to a line that doesn't end with '\'

	    ($body) = ($rest =~ /\s*(\n\#define\s+[^(\n]+\s*[(][^)\n]*[)].*?[^\\])$/os);
	    print SRS "==FUNCLIST\n$body\n==ENDLIST\n";
	    print SRS "==REST\n$rest\n==ENDREST\n";
	}
	if($inputargs) {
	    ${$ostr} .= "<h5>Input</h5>\n";
	    ${$ostr} .= "<table>$inputargs</table>\n";
	}
	if($outputargs) {
	    ${$ostr} .= "<h5>Output</h5>\n";
	    ${$ostr} .= "<table>$outputargs</table>\n";
	}
	if($modifyargs) {
	    ${$ostr} .= "<h5>Input \&amp; Output</h5>\n";
	    ${$ostr} .= "<table>$modifyargs</table>\n";
	}
	if($returnargs) {
	    ${$ostr} .= "<h5>Returns</h5>\n";
	    ${$ostr} .= "<table>$returnargs</table>\n";
	}
	if($longdesc) {
	    ${$ostr} .= "<h4>Description</h4>\n";
	    ${$ostr} .= "$longdesc\n";
	}
	if($usetext) {
	    ${$ostr} .= "<h4>Usage</h4>\n";
	    ${$ostr} .= "$usetext\n";
	}
	if($exampletext) {
	    ${$ostr} .= "<h4>Example</h4>\n";
	    ${$ostr} .= "$exampletext\n";
	}
	if($errtext) {
	    ${$ostr} .= "<h4>Errors</h4>\n";
	    ${$ostr} .= "$errtext\n";
	}
	if($dependtext) {
	    ${$ostr} .= "<h4>Dependencies</h4>\n";
	    ${$ostr} .= "$dependtext\n";
	}
	if($othertext) {
	    ${$ostr} .= "<h4>See Also</h4>\n";
	    ${$ostr} .= "$othertext\n";
	}
	if($availtext) {
	    ${$ostr} .= "<h4>Availability</h4>\n";
	    ${$ostr} .= "$availtext\n";
	}
    }
}

if($dosecttest && $sect ne "") {
    if($countsection == 0) {
	print "bad section: '$sect' has no public functions\n";
    }
}

if (!$countglobal) {
    open (EMPTY, ">$pubout.empty") || die "Cannot open  $pubout.empty";
    close EMPTY;
    $out .= "<p>No public functions in source file $infile</p>"
}
if (!$countstatic) {
    open (EMPTY, ">$local\_static.empty") || die "Cannot open $local\_static.empty";
    close EMPTY;
    $outstatic .= "<p>No static functions in source file $infile</p>"
}

if($sectstr !~ /[^ ]$/) {$sectstr = ""}
$out =~ s/[.]+lastsect[.]+/$sectstr\n/;

if($sectstrstatic !~ /[^ ]$/) {$sectstrstatic = ""}
$outstatic =~ s/[.]+lastsect[.]+/$sectstrstatic\n/;

if($datastr !~ /[^ ]$/) {$datastr = ""}
$out =~ s/[.]+lastdata[.]+/$datastr\n/;

if($datastrstatic !~ /[^ ]$/) {$datastrstatic = ""}
$outstatic =~ s/[.]+lastdata[.]+/$datastrstatic\n/;

if($filestr !~ /[^ ]$/) {$filestr = ""}
$out =~ s/[.]+lastfile[.]+/$filestr\n/;

if($filestrstatic !~ /[^ ]$/) {$filestrstatic = ""}
$outstatic =~ s/[.]+lastfile[.]+/$filestrstatic\n/;

$out .= "</body></html>\n";
$outstatic .= "</body></html>\n";

print HTML "$out";
print HTMLB "$outstatic";
close HTML;
close HTMLB;

exit ();

foreach $x (@datalist) {
    print STDERR "$x\n";

    foreach $y (@{$datasect{$x}}) {
	print STDERR "    $y\n";
	$d = "$x - $y";
	foreach $f (@{$datafunc{$d}}) {
	    print STDERR "        $f\n";
	}
	print STDERR "\n";
    }
    print STDERR "\n";
}

