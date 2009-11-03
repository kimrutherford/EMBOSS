#!/usr/bin/perl -w

use English;
$pubout = "public";
$local = "local";
$infile = "";
$lib = "unknown";
@lattrs = ();


sub srsref {
    return "<a href=\"http://srs.ebi.ac.uk/srs7bin/cgi-bin/wgetz?-e+[EFUNC-ID:$_[0]]\">$_[0]</a>";
}
sub srsdref {
    return "<a href=\"http://srs.ebi.ac.uk/srs7bin/cgi-bin/wgetz?-e+[EDATA-ID:$_[0]]\">$_[0]</a>";
}

sub srsdrefa {
    return "<a href=\"http://srs.ebi.ac.uk/srs7bin/cgi-bin/wgetz?-e+[EDATA-ALIAS:$_[0]]\">$_[0]</a>";
}

if ($ARGV[0]) {$infile = $ARGV[0];}
if ($ARGV[1]) {$lib = $ARGV[1];}

$source = "";
$lasttoken = "";
$intable = 0;
$name = "";

if ($infile) {
    ($file) = ($infile =~ /([^\/]+)$/);
    (undef, $dir, $pubout, $ext) = ($infile =~ /^(([^\/.]*)\/)*([^\/.]+)(\.\S+)?$/);
    $local = $pubout;
    if ($dir) {$lib = $dir}
    if ($ext eq ".h") { $exttype = "include"}
    elsif ($ext eq ".c") { $exttype = "source"}
    else { $exttype = "other"}
    print "set pubout '$pubout' lib '$lib' type '$exttype'\n";
    open (INFILE, "$infile") || die "Cannot open $infile";
    while (<INFILE>) {$source .= $_}
    $dirfile = "$dir/$pubout$ext";
    $title = "$dirfile";
}
else {
    $file = "xxx.c";
    while (<>) {$source .= $_}
    $infile = $file;
    $title = "unknown";
}

open (HTML, ">$pubout.html");
open (HTMLB, ">$local\_static.html");
open (SRS, ">$pubout.srsdata");

$OFILE = HTML;
$countglobal=0;
$countstatic=0;

print HTML  "<html><head><title>$title</title></head><body bgcolor=\"#ffffff\">\n";
print HTMLB  "<html><head><title>$title</title></head><body bgcolor=\"#ffffff\">\n";

print HTML  "<h1>$pubout</h1>\n";
print HTMLB  "<h1>$pubout</h1>\n";

foreach $x ("alias", "attr", "cc", "iterator", "other") {
    $tables{$x} = 3;
}

foreach $x ("new", "delete", "assign", "modify", "use", "cast",
	    "input", "output", "iterate") {
    $tables{$x} = 1;
}

foreach $x ("del", "ass", "set", "mod") {
    $tables{$x} = 2;
}

foreach $x ("author", "version", "modified", "source", "plus", "funclist",
	    "prog", "macro", "func", "funcstatic", "param", "return",
	    "see", "error", "cre", "ure", "exception", "cc",
	    "category", "fcategory", "filesection", "suffix", "datasection",
	    "fdata", "fnote", "argrule", "valrule", "namrule",
	    "obsolete", "rename", "replace", "remove") {
    $functoken{$x} = 1;
}

%obsolete = ( "del" => "delete",
	      "ass" => "assign",
	      "set" => "assign",
	      "mod" => "modify");

##############################################################
## $source is the entire source file as a string with newlines
## step through each comment
## looking for extended JavaDoc style formatting
## $ccfull is the comment
## $rest is the rest of the file
##############################################################

while ($source =~ m"[\/][*][^*]*[*]+([^\/*][^*]*[*]+)*[\/]"gos) {
    $ccfull = $&;
    $rest = $POSTMATCH;

    ($cc) = ($ccfull =~ /^..\s*(.*\S)*\s*..$/gos);
    if (defined($cc)) {
	$cc =~ s/[* ]*\n[* ]*/\n/gos;
    }
    else {
	$cc = "";
    }

    $type = "";
    while ($cc =~ m/@((\S+)([^@]+))/gos) {
	$data = $1;
	$token = $2;

	if (defined($tables{$token}) && $tables{$token} == 2) {
	    print "Obsolete token $lasttoken - use $obsolete{$token}\n";
	}
	if ($token ne $lasttoken) {
	    if ($tables{$lasttoken}) {print $OFILE "</table>\n"}
	    $intable = 0;
	}

	if ($token eq "data")  {
	    $nattr = 0;
	    $OFILE = HTML;
	    $countglobal++;
	    $type = $token;
	    ($name, $frest) = ($data =~ /\S+\s+(\S+)\s*(.*)/gos);
	    if(!defined($name)) {
		print "bad data definition: not parsed\n";
		$name = "";
		next;
	    }
	    $dtypedefname = "";
	    ($dtype, $dattrs, $dtypeb, $ddefine) =
		$rest =~ /^\s*([^\{]*)\{(.*?)\n[\}]([^;]*)[;]\s*([\#]define[^\n]+)?/os;

	    print "Data type $name\n";
	    print $OFILE "<hr><h2>Data type ".srsdref($name)."</h2>\n";
	    $srest = $frest;
	    $frest =~ s/\n\n/\n<p>\n/gos;
	    print $OFILE "$frest\n";

	    print SRS "ID $name\n";
	    print SRS "TY public\n";
	    print SRS "MO $pubout\n";
	    print SRS "LB $lib\n";
	    print SRS "XX\n";

	    if ($exttype ne "include") {
		print "bad scope 'data' in $exttype file\n";
	    }
	    if (!$frest) {print "bad data type '$name', no description\n"}

	    $srest =~ s/\n\n+$/\n/gos;
	    $srest =~ s/\n\n\n+/\n\n/gos;
	    $srest =~ s/\n([^\n])/\nDE $1/gos;
	    $srest =~ s/\n\n/\nDE\n/gos;
	    print SRS "DE $srest";
	    print SRS "XX\n";

	    $icc=0;
	    @lattrs = ();
###	    print STDERR "old dattrs '$dattrs'\n";
	    $dattrs =~ s/struct\s+[\{][^\}]+[\}]\s+[^;]+;\s+//g;
	    $dattrs =~ s/union\s+[\{][^\}]+[\}]/union /g;
###	    print STDERR "mod dattrs '$dattrs'\n";
	    while ($dattrs =~ /([^;]+)[;]\s*([\/][*][^*]*[*]+([^\/*][^*]*[*]+)*[\/][^\n]*[\n])?/gos) {
		$dattr = $1;
		$dcc = $2;
		$dattr =~ s/^\s+//gos;
		$dattr =~ s/\s+$//gos;
		$dattr =~ s/\s+/ /gos;
		$dattr =~ s/ ([*]+)/$1 /gos;
		if($dattr =~ /^[\#]else /gos) {next}
		$dattr =~ s/^[\#]endif //gos;
		$dattr =~ s/^[\#]ifn?def \S+ //gos;
		push @lattrs, $dattr;
		if (defined($dcc) && $dcc ne "") {
		    $icc++;
		}
	    }
	    if ($icc) {
		print "bad data definition $name, has $icc comments\n";
	    }
	}

	elsif ($token eq "datastatic")  {
	    $nattr = 0;
	    $OFILE = HTMLB;
	    $countstatic++;
	    $type = $token;
	    ($name, $frest) = ($data =~ /\S+\s+(\S+)\s*(.*)/gos);
	    print "Static data type $name\n";
	    if(!defined($name)) {
		print "bad datastatic definition: not parsed\n";
		$name = "";
		next;
	    }
	    ($dtype, $dattrs, $dtypeb, $ddefine) =
		$rest =~ /^\s*([^\{]*)\{([^\}]+)[\}]([^;]*)[;]\s*([\#]define[^\n]+)?/os;

	    print $OFILE "<h2>Static data type $name</h2>\n";
	    $srest = $frest;
	    $frest =~ s/\n\n/\n<p>\n/gos;
	    print $OFILE "$frest\n";

	    print SRS "ID $name\n";
	    print SRS "TY static\n";
	    print SRS "MO $pubout\n";
	    print SRS "LB $lib\n";
	    print SRS "XX\n";

	    if ($exttype ne "source") {
		print "bad scope 'datastatic' in $exttype file\n";
	    }
	    if (!$frest) {print "bad data type '$name', no description\n"}

	    $srest =~ s/\n\n+$/\n/gos;
	    $srest =~ s/\n\n\n+/\n\n/gos;
	    $srest =~ s/\n([^\n])/\nDE $1/gos;
	    $srest =~ s/\n\n/\nDE\n/gos;
	    print SRS "DE $srest";
	    print SRS "XX\n";

	    $icc=0;
	    @lattrs = ();
	    while ($dattrs =~ /([^;]+)[;]\s*([\/][*][^*]*[*]+([^\/*][^*]*[*]+)*[\/][^\n]*[\n])?/gos) {
		$dattr = $1;
		$dcc = $2;
		$dattr =~ s/^\s+//gos;
		$dattr =~ s/\s+$//gos;
		$dattr =~ s/\s+/ /gos;
		$dattr =~ s/ ([*]+)/$1 /gos;
		push @lattrs, $dattr;
		if (defined($dcc) && $dcc ne "") {
		    $icc++;
		}
	    }
	    if ($icc) {
		print "bad data definition $name, has $icc comments\n";
	    }
	}

	elsif ($token eq "datatype")  {
	    $nattr = 0;
	    $OFILE = HTMLB;
	    $countstatic++;
	    $type = $token;
	    ($name, $frest) = ($data =~ /\S+\s+(\S+)\s*(.*)/gos);
	    if(!defined($name)) {
		print "bad datatype definition: not parsed\n";
		$name = "";
		next;
	    }
	    $dattrs = "";
	    $dtypeb = "";
	    $ddefine = "";
	    ($dattrtype, $dtypedefname) =
		$rest =~ /^\s*typedef\s*([^\;]+\S)\s+(\S+)\s*[;]/os;

	    print "Typedef data type $name\n";
	    print $OFILE "<h2>Typedef data type $name</h2>\n";
	    $srest = $frest;
	    $frest =~ s/\n\n/\n<p>\n/gos;
	    print $OFILE "$frest\n";

	    print SRS "ID $name\n";
	    print SRS "TY typedef\n";
	    print SRS "MO $pubout\n";
	    print SRS "LB $lib\n";
	    print SRS "XX\n";

	    if ($dtypedefname ne $name) {
		print "bad data type name '$name' typedef is '$dtypedefname'\n";
	    }

	    if (!$frest) {print "bad data type '$name', no description\n"}

	    $srest =~ s/\n\n+$/\n/gos;
	    $srest =~ s/\n\n\n+/\n\n/gos;
	    $srest =~ s/\n([^\n])/\nDE $1/gos;
	    $srest =~ s/\n\n/\nDE\n/gos;
	    print SRS "DE $srest";
	    print SRS "XX\n";

	}

	elsif (defined($tables{$token}) && $tables{$token} == 1) {
	    if ($token eq "new")  {
		if (!$intable) {
		    print $OFILE "<h3>Constructor(s)</h3>\n";
		    print $OFILE "<p><table border=3>\n";
		    print $OFILE "<tr><th>Name</th><th>Description</th></tr>\n";
		    $intable = 1;
		}
		($fname,$prest) = ($data =~ m/\S+\s*(\S*)\s*(.*)/gos);

		$drest = $prest;
		$drest =~ s/\n\n+$/\n/gos;
		$drest =~ s/\n\n\n+/\n\n/gos;
		$drest =~ s/\n([^\n])/\nND $1/gos;
		$drest =~ s/\n\n/\nND\n/gos;
		print SRS "NN $fname\n";
		print SRS "ND $drest";
		print SRS "NX\n";

		$docrest = $prest;
		$docrest =~ s/\s+/ /gos;

		if (!$prest) {print "bad new spec '$fname', no description\n"}
		print $OFILE "<tr><td>".srsref($fname)."</td><td>$prest</td></tr>\n";
	    }

	    elsif ($token eq "delete" || $token eq "del")  {
		if (!$intable) {
		    print $OFILE "<h3>Destructor(s)</h3>\n";
		    print $OFILE "<p><table border=3>\n";
		    print $OFILE "<tr><th>Name</th><th>Description</th></tr>\n";
		    $intable = 1;
		}
		($fname,$prest) = ($data =~ m/\S+\s*(\S*)\s*(.*)/gos);

		$drest = $prest;
		$drest =~ s/\n\n+$/\n/gos;
		$drest =~ s/\n\n\n+/\n\n/gos;
		$drest =~ s/\n([^\n])/\nDD $1/gos;
		$drest =~ s/\n\n/\nDD\n/gos;
		print SRS "DN $fname\n";
		print SRS "DD $drest";
		print SRS "DX\n";

		$docrest = $prest;
		$docrest =~ s/\s+/ /gos;

		if (!$prest) {print "bad delete spec '$fname', no description\n"}
		print $OFILE "<tr><td>".srsref($fname)."</td><td>$prest</td></tr>\n";
	    }

	    elsif ($token eq "use")  {
		if (!$intable) {
		    print $OFILE "<h3>Operator(s)</h3>\n";
		    print $OFILE "<p><table border=3>\n";
		    print $OFILE "<tr><th>Name</th><th>Description</th></tr>\n";
		    $intable = 1;
		}
		($fname,$prest) = ($data =~ m/\S+\s*(\S*)\s*(.*)/gos);

		$drest = $prest;
		$drest =~ s/\n\n+$/\n/gos;
		$drest =~ s/\n\n\n+/\n\n/gos;
		$drest =~ s/\n([^\n])/\nOD $1/gos;
		$drest =~ s/\n\n/\nOD\n/gos;
		print SRS "ON $fname\n";
		print SRS "OD $drest";
		print SRS "OX\n";

		$docrest = $prest;
		$docrest =~ s/\s+/ /gos;

		if (!$prest) {print "bad use spec '$fname', no description\n"}
		print $OFILE "<tr><td>".srsref($fname)."</td><td>$prest</td></tr>\n";
	    }

	    elsif ($token eq "assign" || $token eq "ass" || $token eq "set")  {
		if (!$intable) {
		    print $OFILE "<h3>Assignment(s)</h3>\n";
		    print $OFILE "<p><table border=3>\n";
		    print $OFILE "<tr><th>Name</th><th>Description</th></tr>\n";
		    $intable = 1;
		}
		($fname,$prest) = ($data =~ m/\S+\s*(\S*)\s*(.*)/gos);

		$drest = $prest;
		$drest =~ s/\n\n+$/\n/gos;
		$drest =~ s/\n\n\n+/\n\n/gos;
		$drest =~ s/\n([^\n])/\nED $1/gos;
		$drest =~ s/\n\n/\nED\n/gos;
		print SRS "EN $fname\n";
		print SRS "ED $drest";
		print SRS "EX\n";

		$docrest = $prest;
		$docrest =~ s/\s+/ /gos;

		if (!$prest) {print "bad assignment spec '$fname', no description\n"}
		print $OFILE "<tr><td>".srsref($fname)."</td><td>$prest</td></tr>\n";
	    }

	    elsif ($token eq "modify" || $token eq "mod")  {
		if (!$intable) {
		    print $OFILE "<h3>Modifier(s)</h3>\n";
		    print $OFILE "<p><table border=3>\n";
		    print $OFILE "<tr><th>Name</th><th>Description</th></tr>\n";
		    $intable = 1;
		}
		($fname,$prest) = ($data =~ m/\S+\s*(\S*)\s*(.*)/gos);

		$drest = $prest;
		$drest =~ s/\n\n+$/\n/gos;
		$drest =~ s/\n\n\n+/\n\n/gos;
		$drest =~ s/\n([^\n])/\nMD $1/gos;
		$drest =~ s/\n\n/\nMD\n/gos;
		print SRS "MN $fname\n";
		print SRS "MD $drest";
		print SRS "MX\n";

		$docrest = $prest;
		$docrest =~ s/\s+/ /gos;

		if (!$prest) {print "bad modifier spec '$fname', no description\n"}
		print $OFILE "<tr><td>".srsref($fname)."</td><td>$prest</td></tr>\n";
	    }

	    elsif ($token eq "cast")  {
		if (!$intable) {
		    print $OFILE "<h3>Cast(s)</h3>\n";
		    print $OFILE "<p><table border=3>\n";
		    print $OFILE "<tr><th>Name</th><th>Description</th></tr>\n";
		    $intable = 1;
		}
		($fname,$prest) = ($data =~ m/\S+\s*(\S*)\s*(.*)/gos);

		$drest = $prest;
		$drest =~ s/\n\n+$/\n/gos;
		$drest =~ s/\n\n\n+/\n\n/gos;
		$drest =~ s/\n([^\n])/\nCD $1/gos;
		$drest =~ s/\n\n/\nCD\n/gos;
		print SRS "CN $fname\n";
		print SRS "CD $drest";
		print SRS "CX\n";

		$docrest = $prest;
		$docrest =~ s/\s+/ /gos;

		if (!$prest) {print "bad cast spec '$fname', no description\n"}
		print $OFILE "<tr><td>".srsref($fname)."</td><td>$prest</td></tr>\n";
	    }

	    elsif ($token eq "input")  {
		if (!$intable) {
		    print $OFILE "<h3>Input</h3>\n";
		    print $OFILE "<p><table border=3>\n";
		    print $OFILE "<tr><th>Name</th><th>Description</th></tr>\n";
		    $intable = 1;
		}
		($fname,$prest) = ($data =~ m/\S+\s*(\S*)\s*(.*)/gos);

		$drest = $prest;
		$drest =~ s/\n\n+$/\n/gos;
		$drest =~ s/\n\n\n+/\n\n/gos;
		$drest =~ s/\n([^\n])/\nJD $1/gos;
		$drest =~ s/\n\n/\nJD\n/gos;
		print SRS "JN $fname\n";
		print SRS "JD $drest";
		print SRS "JX\n";

		$docrest = $prest;
		$docrest =~ s/\s+/ /gos;

		if (!$prest) {print "bad input spec '$fname', no description\n"}
		print $OFILE "<tr><td>".srsref($fname)."</td><td>$prest</td></tr>\n";
	    }

	    elsif ($token eq "output")  {
		if (!$intable) {
		    print $OFILE "<h3>Output</h3>\n";
		    print $OFILE "<p><table border=3>\n";
		    print $OFILE "<tr><th>Name</th><th>Description</th></tr>\n";
		    $intable = 1;
		}
		($fname,$prest) = ($data =~ m/\S+\s*(\S*)\s*(.*)/gos);

		$drest = $prest;
		$drest =~ s/\n\n+$/\n/gos;
		$drest =~ s/\n\n\n+/\n\n/gos;
		$drest =~ s/\n([^\n])/\nPD $1/gos;
		$drest =~ s/\n\n/\nPD\n/gos;
		print SRS "PN $fname\n";
		print SRS "PD $drest";
		print SRS "PX\n";

		$docrest = $prest;
		$docrest =~ s/\s+/ /gos;

		if (!$prest) {print "bad output spec '$fname', no description\n"}
		print $OFILE "<tr><td>".srsref($fname)."</td><td>$prest</td></tr>\n";
	    }
	    elsif ($token eq "iterate")  {
		if (!$intable) {
		    print $OFILE "<h3>Output</h3>\n";
		    print $OFILE "<p><table border=3>\n";
		    print $OFILE "<tr><th>Name</th><th>Description</th></tr>\n";
		    $intable = 1;
		}
		($fname,$prest) = ($data =~ m/\S+\s*(\S*)\s*(.*)/gos);
		if(!defined($fname)) {
		    print "bad iterate value: not parsed";
		    $fname = "";
		    next;
		}
		$drest = $prest;
		$drest =~ s/\n\n+$/\n/gos;
		$drest =~ s/\n\n\n+/\n\n/gos;
		$drest =~ s/\n([^\n])/\nLD $1/gos;
		$drest =~ s/\n\n/\nLD\n/gos;
		print SRS "LN $fname\n";
		print SRS "LD $drest";
		print SRS "LX\n";

		$docrest = $prest;
		$docrest =~ s/\s+/ /gos;

		if (!$prest) {print "bad iterate spec '$fname', no description\n"}
		print $OFILE "<tr><td>".srsref($fname)."</td><td>$prest</td></tr>\n";
	    }
	    else {
		print "bad category token '$token' - no validation\n";
	    }
	    $docrest =~ s/^\s+//gos;
	    $docrest =~ s/\s+$//gos;
	    print "category $token [$name] $fname $pubout $lib : $docrest\n";
	    print "category $token";
	    print "[$name] ";
	    print "$fname ";
	    print "$pubout ";
	    print "$lib : ";
	    print "$docrest\n";
	}

	elsif ($token eq "other")  {
	    if (!$intable) {
		print $OFILE "<h3>Other related data structure(s)</h3>\n";
		print $OFILE "<p><table border=3>\n";
		print $OFILE "<tr><th>Name</th><th>Description</th></tr>\n";
		$intable = 1;
	    }
	    ($fname,$prest) = ($data =~ m/\S+\s*(\S*)\s*(.*)/gos);

	    $drest = $prest;
	    $drest =~ s/\n\n+$/\n/gos;
	    $drest =~ s/\n\n\n+/\n\n/gos;
	    $drest =~ s/\n([^\n])/\nRD $1/gos;
	    $drest =~ s/\n\n/\nRD\n/gos;
	    $drest =~ s/^$/\n/gos;
	    print SRS "RN $fname\n";
	    print SRS "RD $drest";
	    print SRS "RX\n";

	    $docrest = $prest;
	    $docrest =~ s/\s+/ /gos;

	    if (!$prest) {print "bad other spec '$fname', no description\n"}
	    print $OFILE "<tr><td>".srsdref($fname)."</td><td>$prest</td></tr>\n";
	}

	elsif ($token eq "alias")  {
	    if (!$intable) {
		print $OFILE "<h3>Alias name(s)</h3>\n";
		print $OFILE "<p><table border=3>\n";
		print $OFILE "<tr><th>Name</th><th>Description</th></tr>\n";
		$intable = 1;
	    }
	    ($fname,$prest) = ($data =~ m/\S+\s*(\S*)\s*(.*)/gos);

	    $drest = $prest;
	    $drest =~ s/\n\n+$/\n/gos;
	    $drest =~ s/\n\n\n+/\n\n/gos;
	    $drest =~ s/\n([^\n])/\nTD $1/gos;
	    $drest =~ s/\n\n/\nTD\n/gos;
	    $drest =~ s/^$/\n/gos;
	    print SRS "TN $fname\n";
	    print SRS "TD $drest";
	    print SRS "TX\n";

	    ###if (!$prest) {print "bad alias spec '$fname', no description\n"}
	    print $OFILE "<tr><td>".srsdrefa($fname)."</td><td>$prest</td></tr>\n";
	}

	elsif ($token eq "iterator")  {
	    if (!$intable) {
		print $OFILE "<h3>Iterator name(s)</h3>\n";
		print $OFILE "<p><table border=3>\n";
		print $OFILE "<tr><th>Name</th><th>Description</th></tr>\n";
		$intable = 1;
	    }
	    ($fname,$prest) = ($data =~ m/\S+\s*(\S*)\s*(.*)/gos);

	    $drest = $prest;
	    $drest =~ s/\n\n+$/\n/gos;
	    $drest =~ s/\n\n\n+/\n\n/gos;
	    $drest =~ s/\n([^\n])/\nKD $1/gos;
	    $drest =~ s/\n\n/\nKD\n/gos;
	    $drest =~ s/^$/\n/gos;
	    print SRS "KN $fname\n";
	    print SRS "KD $drest";
	    print SRS "KX\n";

	    ###if (!$prest) {print "bad iterator spec '$fname', no description\n"}
	    print $OFILE "<tr><td>".srsdref($fname)."</td><td>$prest</td></tr>\n";
	}

	elsif ($token eq "attr")  {
	    $nattr++;
	    if (!$intable) {
		print $OFILE "<h3>Attributes</h3>\n";
		print $OFILE "<p><table border=3>\n";
		print $OFILE "<tr><th>Name</th><th>Type</th><th>Description</th></tr>\n";
		$intable = 1;
	    }
	    ($aname,$atype, $prest) = ($data =~ m/\S+\s*(\S*)\s*[\[]([\[\]A-Za-z0-9_* \(\)]+)[\]]\s*(.*)/gos);

	    if (!defined($aname)) {
		print STDERR "bad \@attr syntax:\n$data";
		$aname = "";
		$atype = "";
		next;
	    }
	    $drest = $prest;
	    $drest =~ s/\n\n+$/\n/gos;
	    $drest =~ s/\n\n\n+/\n\n/gos;
	    $drest =~ s/\n([^\n])/\nAD $1/gos;
	    $drest =~ s/\n\n/\nAD\n/gos;
	    $drest =~ s/^$/\n/gos;
	    print SRS "AN $aname\n";
	    print SRS "AT $atype\n";
	    print SRS "AD $drest";
	    print SRS "AX\n";

	    if (!$atype) {print "bad attribute spec '$aname', no type\n"}
	    if (!$prest) {print "bad attribute spec '$aname', no description\n"}
	    if ($#lattrs >= 0) {
		$dattr = shift @lattrs;
###	    print STDERR "dattr: '$dattr'\n";
		if ($dattr =~ /.*[\(][*]+[^\)]+\)/) {
		    ($dattrtype,$dattrname) = $dattr =~ /(.*\S)\s*[\(][*]+([^\)]+)\)/;
		    $dattrtype="(".$dattrtype."*)";
		}
		else {
		    ($dattrtype,$dattrname) = $dattr =~ /(.*\S)\s+(\S+)$/;
		}
	    }
	    else {
		$dattrname = "typedef";
		if ($type eq "datatype") {
		    if ($name ne $dtypedefname) {
			print "bad datatype name '$name' typedef is '$dtypedefname'\n";
		    }
		    if ($atype ne $dattrtype) {
			print "bad datatype cast <$atype> <$dattrtype>\n";
		    }
		}
	    }
	    if ($dattrname =~ /([^\[]+)([\[].*[\]])/) {
		$dattrtype .= $2;
		$dattrname = $1;
	    }
###	    print "attr: '$dattrname' type: '$dattrtype'\n";

	    if ($atype ne $dattrtype) {
		print "bad cast <$atype> <$dattrtype>\n";
	    }
	    if ($aname ne $dattrname) {
		print "bad attribute  <$aname> <$dattrname>\n";
	    }

	    print $OFILE "<tr><td>".srsdref($aname)."</td><td>$prest</td></tr>\n";
	}

	elsif ($token eq "section")  {
	    ;
	}

	elsif ($token eq "@")  {
	    last;
	}

	elsif ($token eq "header")  {
	    ;
	}

	elsif ($token eq "short")  {
	    ;
	}

	elsif ($token eq "release")  {
	    ;
	}

	elsif ($token =~ /[^a-z]/)  {# acd function
	    ;
	}

	elsif ($token eq "not")  { # acd function
	    ;
	}

	elsif ($token eq "pasteur.fr")  { # email address
	    ;
	}

	elsif ($token eq "sanger")  { # email address
	    ;
	}

	elsif (defined($functoken{$token}))  {
	    ;
	}

	else  {
	    print  "bad unrecognized token \@$token\n";
	    print STDERR "Unrecognized token \@$token in $pubout\n";
	}

	$lasttoken = $token;
    }
    if ($type) {
	print "=============================\n";
	print SRS "//\n";

	($body,undef,undef,undef) = ($rest =~ /((.*?\n([\s\{][^\n]*\n)*\S[^;]*;[^\n]*\n)([\s\n]*[\#]define\s+\S+\s+\S+[*]\s*?\n)?)/gos);
	$body =~ s/^\s+//m;
##    print "SRS:\n$body\n";
##    print "a: $a\n";
##    print "c: $c\n";

	print SRS $body;

	if (!$nattr) {
	    print "bad attributes - none found\n";
	}
	if ($#lattrs != -1) {
	    print "bad attributes - ".($#lattrs+1)." definition(s) missing\n";
	}
    }
}


if (!$countglobal) {
    open (EMPTY, ">$pubout.empty") || die "Cannot open  $pubout.empty";
    close EMPTY;
    print HTML "<p>No public datatype definitions in source file $dirfile</p>"
}
if (!$countstatic) {
    open (EMPTY, ">$local\_static.empty") || die "Cannot open $local\_static.empty";
    close EMPTY;
    print HTMLB "<p>No static datatype definitions in source file $dirfile</p>"
}
print HTML "</body></html>\n";
print HTMLB "</body></html>\n";
close HTML;
close HTMLB;
