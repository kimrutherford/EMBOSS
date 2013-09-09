#!/usr/bin/perl -w

$edamurl = "http://edamontology.org";

@inputs = ("assembly",
	   "codon",
	   "cpdb",
	   "datafile",
	   "directory",
	   "dirlist",
	   "discretestates",
	   "features",
	   "filelist",
	   "frequencies",
	   "infile",
	   "matrix",
	   "matrixf",
	   "obo",
	   "pattern",
	   "properties",
	   "refseq",
	   "regexp",
	   "resource",
	   "scop",
	   "sequence",
	   "seqall",
	   "seqset",
	   "seqsetall",
	   "taxon",
	   "text",
	   "tree",
	   "url",
	   "variation",
	   "xml",
    );

@outputs = ("align",
	    "featout",
	    "graph",
	    "outassembly",
	    "outcodon",
	    "outcpdb",
	    "outdata",
	    "outdir",
	    "outdiscrete",
	    "outdistance",
	    "outfile",
	    "outfreq",
	    "outmatrix",
	    "outmatrixf",
	    "outobo",
	    "outproperties",
+	    "outrefseq",
	    "outresource",
	    "outscop",
	    "outtaxon",
	    "outtext",
	    "outtree",
	    "outurl",
	    "outvariation",
	    "outxml",
	    "report",
	    "seqout",
	    "seqoutall",
	    "seqoutset",
	    "xygraph",
    );

@embassylist = (
    "appendixd",
    "cbstools",
    "clustalomega",
    "domainatrix",
    "domalign",
    "domsearch",
    "emnu",
    "esim4",
#   "hmmer",
    "hmmernew",
    "iprscan",
    "meme",
    "memenew",
    "mira",
    "mse",
    "myemboss", # we avoid documenting these examples
    "myembossdemo", # we avoid documenting these examples
#   "phylip",
    "phylipnew",
    "signature",
    "structure",
    "topo",
#   "vienna",	# old vienna
    "vienna2",
    );

foreach $i (@inputs) {$inputtype{$i} = 1}
foreach $i (@outputs) {$outputtype{$i} = 1}

sub parseacd($){
    my ($pname) = @_;
    @function = ();		# EDAM_operation
    @topic = ();		# EDAM_topic
    @input = ();		# EDAM_data input terms
    @output = ();		# EDAM_data output terms

    system "acdpretty $pname"|| die "Failed to find ACD file $pname.acd";
    open (ACD, "$pname.acdpretty") || die "Failed to open $pname.acdpretty";
    my $acdtext = "";
    while ($acd = <ACD>) {
	$acdtext .= $acd;
    }
    close ACD;
    unlink "$pname.acdpretty";
    if($acdtext =~ m/application: \S+ [\[](.*?)\n\s*[\]]/gos) {
	$appdoc = $1;
	while($appdoc =~ /relations:\s+\"EDAM_operation:(\d+) ([^\"]+)\"/gos){
	    $id = $1;
	    $f = $2;
	    $f =~ s/\n\s*/ /go;
	    $f .= "#$edamurl/operation_$id";
#	    print STDERR "$pname function: '$f'\n";
	    push (@function, $f);
	}
	while($appdoc =~ /relations:\s+\"EDAM_topic:(\d+) ([^\"]+)\"/gos){
	    $id = $1;
	    $t = $2;
	    $t =~ s/\n\s*/ /go;
	    $t .= "#$edamurl/topic_$id";
#	    print STDERR "$pname function: '$f'\n";
	    push (@topic, $t);
	}
    }
    else {die "$pname.acd failed to find application definition"}

    while($acdtext =~ /\n\s*([a-z]+): \S+ [\[](.*?)\n\s*[\]]/gos) {
	$acdtype = $1;
	$acdrest = $2;
	if(defined($inputtype{$acdtype})) {
	    while($acdrest =~ /relations:\s+\"EDAM_data:(\d+) ([^\"]+)\"/gos){
		$id = $1;
		$i = $2;
		$i =~ s/\n\s*/ /go;
		$i .= "#$edamurl/data_$id";
#	    print STDERR "$pname function: '$f'\n";
		push (@input, $i);
	    }
	}
	if(defined($outputtype{$acdtype})) {
	    while($acdrest =~ /relations:\s+\"EDAM_data:(\d+) ([^\"]+)\"/gos){
		$id = $1;
		$i = $2;
		$i =~ s/\n\s*/ /go;
		$i .= "#$edamurl/data_$id";
#	    print STDERR "$pname function: '$f'\n";
		push (@output, $i);
	    }
	}
    }

    %done=();
    foreach $f (@function) {
	if(!defined($done{$f})){
	    $done{$f} = 1;
	    print OUT "<Function>$f</Function>\n";
	}
    }
    %done=();
    foreach $t (@topic) {
	if(!defined($done{$f})){
	    $done{$f} = 1;
	    print OUT "<Topic>$t</Topic>\n";
	}
    }
    %done=();
    print OUT "<Taxon></Taxon>\n";
    foreach $i (@input) {
#	if(!defined($done{$i})){
#	    $done{$i} = 1;
	    print OUT "<Input>$i</Input>\n";
#	}
    }
    %done=();
    foreach $o (@output) {
#	if(!defined($done{$o})){
#	    $done{$o} = 1;
	    print OUT "<Output>$o</Output>\n";
#	}
    }
}

open (OUT, ">acdtoelixir.xml") || die "Unable to open 'acdtoelixir.xml'";

print OUT "<Tools>\n";

$toolid = 0;

#
# Find install directory to read ACD files
#

open (PROGS, "wossname -noembassy -alpha -auto |") ||
    die "Cannot run wossname";

$plic = "GPL";
$iformat = "";
$oformat = "";
$pauth = "EMBOSS";

while ($prog = <PROGS>) {
    if($prog =~ /^ALPHABETIC LIST/) {next}
    if($prog =~ /^\n/) {next}
    if ($prog =~ /^(\S+) +(.*)/) {
	$pname = $1;
	$pdesc = $2;
	$toolid++;
	print OUT "<Tool toolid=\"$toolid\">\n";
	print OUT "<Name>$pname</Name>\n";
	parseacd($pname);
	print OUT "<Provider>EMBOSS</Provider>\n";
	print OUT "<Developer>$pauth</Developer>\n";
	print OUT "<Home>http://emboss.open-bio.org/rel/rel6/apps/$pname.html</Home>\n";
	print OUT "<Contact>mailto:emboss-bug\@emboss.open-bio.org</Contact>\n";
	print OUT "<Type>Command-line tool</Type>\n";
	print OUT "<Description>$pdesc</Description>\n";
	print OUT "<Collection>EMBOSS</Collection>\n";
	print OUT "<InputFormat>$iformat</InputFormat>\n";
	print OUT "<OutputFormat>$oformat</OutputFormat>\n";
	print OUT "<WorkPackage></WorkPackage>\n";
	print OUT "<Infrastructure>ELIXIR</Infrastructure>\n";
	print OUT "<License>$plic</License>\n";
	print OUT "<TermsOfUse>http://emboss.open-bio.org/html/dev/ch01s01.html</TermsOfUse>\n";
	print OUT "<Downloads></Downloads>\n";
	print OUT "<Available></Available>\n";
	print OUT "</Tool>\n";
    }
    else { die "Bad record in wossname output: $prog"}
}
close PROGS;

foreach $e(@embassylist) {
    open (PROGS, "wossname -showembassy $e -alpha -auto |") ||
	die "Cannot run wossname";

    $plic = "";
    while ($prog = <PROGS>) {
	if($prog =~ /^ALPHABETIC LIST/) {next}
	if($prog =~ /^\n/) {next}
	if ($prog =~ /^(\S+) +(.*)/) {
	    $pname = $1;
	    $pdesc = $2;
	    $toolid++;
	    print OUT "<Tool toolid=\"$toolid\">\n";
	    print OUT "<Name>$pname</Name>\n";
	    parseacd($pname);
	    print OUT "<Provider>EMBOSS</Provider>\n";
	    print OUT "<Home>http://emboss.open-bio.org/rel/rel6/embassy/$e/$pname.html</Home>\n";
	    print OUT "<Contact>mailto:emboss-bug\@emboss.open-bio.org</Contact>\n";
	    print OUT "<Type>Command-line tools </Type>\n";
	    print OUT "<Desc>$pdesc</Desc>\n";
	    print OUT "<Package>EMBOSS</Package>\n";

	    print OUT "</Tool>\n";
	}
	else { die "Bad record in wossname -showembassy $e output: $prog"}
    }
    close PROGS;
}


print OUT "</Tools>\n";
close OUT;
