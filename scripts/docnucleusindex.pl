#!/usr/bin/perl -w

print "<div id=\"table\">
<table cellpadding =\"5\">
<tr><th>LIBRARY</th>  <th>DATATYPES</th>  <th>FUNCTIONS</th> <th>NOTES</th> </tr>
";

%ignorefiles = (
    "index" => "indexfile",
    );


%knownfiles = (
    "embaln" => "Alignments",
    "embcom" => "Complexity algorithms",
    "embcons" => "Consensus sequences",
    "embdata" => "Data file utilities",
    "embdbi" => "Database indexing using EMBL-CD indices",
    "embdmx" => "Domainatrix package algorithms",
    "embdomain" => "Domain algorithms",
    "embest" => "EST algorithms",
    "embexit" => "Exit calls for applications",
    "embgroup" => "Groups of applications",
    "embiep" => "Isoelectric point utilities",
    "embindex" => "Indexing databases using B+ trees",
    "embinit" => "Initialising internals",
    "embmat" => "Comparison matrices",
    "embmisc" => "Miscellaneos methods",
    "embmol" => "Molecular fragment algorithms",
    "embnmer" => "N-mer (word) algorithms",
    "embpat" => "Pattern matching algorithms",
    "embpatlist" => "Pattern list file reading",
    "embpdb" => "Protein databank (PDB) structure file utilities",
    "embprop" => "Properties of protein sequences",
    "embread" => "Reading data files",
    "embshow" => "Showing sequences in pretty formats",
    "embsig" => "Signatures of protein domains",
    "embword" => "Word matching algorithms",
    "embxyz" => "XYZ 3D coordinate algorithms",
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

