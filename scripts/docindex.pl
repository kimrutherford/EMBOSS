#!/usr/bin/perl -w

print "<div id=\"table\">
<table cellpadding =\"5\">
<tr><th>LIBRARY</th>  <th>DATATYPES</th>  <th>FUNCTIONS</th> <th>NOTES</th> </tr>
";

%ignorefiles = (
    "index" => "indexfile",
    "ajgraphxml" => "old graph XML code",
    "pcre" => "PCRE library",
    "pcre_study" => "PCRE library",
    "pcre_chartables" => "PCRE library",
    "pcre_printint" => "PCRE library",
    "pcre_get" => "PCRE library",
    "pcreposix" => "PCRE library",
    );


%knownfiles = (
    "ajacd" => "AJAX Command Definitions",
    "ajalign" => "Alignments",
    "ajarr" => "Array handling",
    "ajassert" => "Assert function",
    "ajbase" => "Base code functions",
    "ajcall" => "Call registration functions",
    "ajcod" => "Codon analysis functions",
    "ajdan" => "DNA melting functions",
    "ajdmx" => "Domainatrix package functions",
    "ajdom" => "DOM parser for XML",
    "ajdomain" => "Domain package functions",
    "ajexcept" => "Exception handling",
    "ajfeat" => "Feature table functions",
    "ajfeatdata" => "Feature table data",
    "ajfile" => "<i>File handling</i>",
    "ajfmt" => "Formatted input and output",
    "ajgraph" => "Graphical output",
    "ajgraphstruct" => "Graphical output data structures",
    "ajhist" => "Histogram graphical data",
    "ajindex" => "Data indexing functions",
    "ajjava" => "Java API functions",
    "ajlist" => "Lists",
    "ajmath" => "Maths utilities",
    "ajmatrices" => "Comparison matrix functions",
    "ajmem" => "Memory allocation",
    "ajmess" => "Message handling",
    "ajnam" => "Names: variables and database definitions",
    "ajnexus" => "Nexus data format",
    "ajpat" => "Pattern matching functions",
    "ajpdb" => "Protein databank data format",
    "ajpdbio" => "Protein databank I/O",
    "ajphylo" => "Phylogenetic data functions",
    "ajrange" => "Sequence range specifications",
    "ajreg" => "Regular expressions",
    "ajreport" => "Feature report output",
    "ajseq" => "Sequence handling functions",
    "ajseqabi" => "Sequence ABI traces",
    "ajseqdata" => "Sequence data",
    "ajseqdb" => "Sequence data access methods",
    "ajseqread" => "Sequence input functions",
    "ajseqtype" => "Sequence type checking",
    "ajseqwrite" => "Sequence output functions",
    "ajsort" => "Sort utility functions",
    "ajstr" => "<i>String manipulation</i>",
    "ajsys" => "System utilities",
    "ajtable" => "Tables",
    "ajtime" => "Time functions",
    "ajtranslate" => "Sequence translation",
    "ajtree" => "Phylogenetic trees",
    "ajutil" => "Utilities",
    "ajvector" => "Cloning vectors",
);

opendir(DIR, ".");
while ($file = readdir(DIR)) {
    if($file =~ /^[.]/) {next}
    if(-d $file) {next}
    if($file =~ /[~]$/) {next}
    if($file =~ /[.]inc$/) {next}
    if($file =~ /^(.*)_static[.]html$/) {next}
    elsif($file =~ /^(.*)[.]html$/) {
	$fpref=$1;
	if($ignorefiles{$fpref}) {next}
    }
    else {
	print "+++ unknown file type $file\n";
	next;
    }
    $empty=0;
    open (HTML, "$file");
    while (<HTML>) {
	if (/No public datatype definitions in source file/) {$empty=1}
    }
#    print "$file ($fpref)\n";
    close (HTML);

    open (DD, "../datadef/$file");
    $emptyd = 0;
    while (<DD>) {
	if(/No public datatype definitions in source file/) {$emptyd=1}
    }
    close (DD);

    if(!defined($knownfiles{$fpref})) {
	    print STDERR "Unknown file: $fpref\n";
    }
    else {
	$out = "\n<tr><td><b><i>$knownfiles{$fpref}</b></i></td>\n";
	if(!$emptyd) {
	    $out .= "<td><a href=\"../datadef/$fpref.html\">Datatypes</a></td>\n";
	}
	else {
	    $out .= "<td>&nbsp;</td>\n";
	}
	if(!$empty) {
	    $out .= "<td><a href=\"$fpref.html\">Functions</a></td>\n";
	}
	else {
	    $out .= "<td>&nbsp;</td>\n";
	}
	$label = uc($fpref);
	$label =~ s/^AJ//;

	$out .= "<td><a href=\"#$label\">Notes</a></td></tr>\n";
	$out{$fpref} = $out;
    }
}
closedir(DIR);


foreach $x(keys(%knownfiles)) {
    if(defined($out{$x})) {
	print $out{$x};
    }
    else {
	$fpref = $x;
	open (DD, "../datadef/$fpref.html");
	$emptyd = 0;
	while (<DD>) {
	    if(/No public datatype definitions in source file/) {$emptyd=1}
	}
	close (DD);

	$out = print "<tr><td><b><i>$knownfiles{$fpref}</b></i></td>\n";
	if(!$emptyd) {
	    $out .= "<td><a href=\"../datadef/$fpref.html\">Datatypes</a></td>\n";
	}
	else {
	    $out .= "<td>&nbsp;</td>\n";
	}

# we know there are no documented public functions!

	$out .= "<td>&nbsp;</td>\n";

	$label = uc($fpref);
	$label =~ s/^AJ//;

	$out .= "<td><a href=\"#$label\">Notes</a></td></tr>\n";
	$out{$fpref} = $out;
	print $out{$x};
    }
}
print "</table>\n";

