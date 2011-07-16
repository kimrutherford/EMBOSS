#!/usr/bin/perl -w

%xrefs = (
    "SP_explicit" => "swissprot",
    "SP_implicit" => "swissprot",
    "SP_CC" => "swissprot",
    "SP_FT" => "swissprot",
    "SP_lit" => "swissprot",
    "EMBL_DR" => "embl",
    "EMBL_explicit" => "embl",
    "Other" => "Tax_id, EC_number, etc.",
    );

%knownfmt = (
    "Text" => 0,
    "HTML" => 0,
    "XML" => 0,
    );

@required = ("ID", "Name", "Desc");
@preferred = ("URL", "Query", "Example", "Email", "Status");


sub checkid() {
    my $qid;
    my $e;
    my $qcnt = 0;
    foreach $qid (sort(keys(%qid))){
	$qcnt++;
	if(!defined($xid{$qid})) {
	    print STDERR "$iline: QRYID-NOEXAMPLE ($qid{$qid}): '$qid'\n";
	    $noexample{$qid} += $qid{$qid};
	}
    }
    foreach $e (sort(keys(%edamdat))){
	if(!defined($qdat{$e})) {
	    print STDERR "$iline: EDAMDAT-UNUSED: '$e'\n";
	}
    }
    foreach $e (sort(keys(%edamid))){
	if(!defined($qid{$e}) && !defined($xid{$e})) {
	    print STDERR "$iline: EDAMID-UNUSED: '$e'\n";
	}
    }
    foreach $e (sort(keys(%edamfmt))){
	if(!defined($qfmt{$e})) {
	    print STDERR "$iline: EDAMFMT-UNUSED: '$e'\n";
	}
    }
    # missing lines
    if(!keys(%qid)) {
	print STDERR "$iline: NOLINE-QUERY no query for: $did\n";
    }
    if(!keys(%xid)) {
	print STDERR "$iline: NOLINE-EXAMPLE no example for: $did\n";
    }
    if(!keys(%oid)) {
	print STDERR "$iline: NOLINE-TAXON no taxon for: $did\n";
    }
    if(keys(%rid)) {
	if(!$qcnt) {
	    print STDERR "$iline: XREF-NO-QUERY xref but no query for: $did\n";
	}
    }
}

foreach $p (@preferred) {$preferred{$p}=1}
foreach $r (@required)  {$required{$r}=1}

open(EDAM, "/homes/pmr/devemboss/emboss/data/EDAM.obo") || die "Cannot open EDAM.obo";

$isterm = 0;
$nterms = 0;
while (<EDAM>){
    if(/^[\[]([^\]]+)/) {
	if($1 eq "Term") {$isterm = 1}
	else {$isterm = 0}
    }
    if(!$isterm) {next}

    if(/^id: EDAM:(\d+)/) {
	$id = $1;
	$ntrms++;
    }
    elsif(/^alt_id: EDAM:(\d+)/) {
	$altid = $1;
	$trueid{$altid} = $id;
    }
    elsif(/^name: ([^\n\!]+)/) {
	$name = $1;
	$name =~ s/\s+$//g;
	$edam{$id} = $name;
    }
    elsif(/^namespace: ([^\n\!]+)/) {
	$namespace = $1;
	$namespace =~ s/\s+$//g;
	$edamspace{$id} = $namespace;
    }
    elsif(/^is_obsolete: (\S+)/) {
	$obs = $1;
	if($obs eq "true") {$isobs{$id}=1}
	else {print STDERR "obsolete '$obs' for id '$id'\n"}
    }
}
close EDAM;

open(DBXREF, "/homes/pmr/devemboss/emboss/data/dbxref.txt") || die "Cannot open dbxref.txt";

$isdata = 0;
while(<DBXREF>){
    if($isdata){
	if(/^[-]+$/) {$isdata = 0;next}
	if(/^$/) {next}
	elsif(/^AC    : (\S+)/) {
	    $xrac = $1;
	}
	elsif(/^Abbrev: (.*)$/) {
	    $xrid = $1;
	    $xrid{$xrid} = $xrac;
	    $xrac{$xrac} = $xrid;
	}
	elsif(/^LinkTp: (.*)$/) {
	    $xlink{$xrid} = $1;
	}
	elsif(/^Server: (.*)$/) {
	    $xserver{$xrid} = $1;
	}
	elsif(/^Db_URL: (.*)$/) {
	    $xurl{$xrid} = $1;
	}
	elsif(/^Cat   : (.*)$/) {
	    $xurl{$xrid} = $1;
	}
	elsif(/^Name  : (.*)$/) {
	    $xname{$xrid} = $1;
	}
	elsif(/^Ref   : (.*)$/) {
	    $xcit{$xrid} = $1;
	}
	elsif(/^Note  : (.*)$/) {
	    $nore{$xrid} = $1;
	}
	elsif(/^        (\S.*)$/) {
	    $continue{$xrid} = $1;
	}
	else {
	    print STDERR "DBXREF-BADLINE: $_";
	}
    }
    if(/^[_]+$/) {$isdata = 1}
}

close DBXREF;

open(DRNEW, ">DRCAT.new") || die "Cannot open DRCAT.new";
open(DRCAT, "/homes/pmr/devemboss/emboss/data/DRCAT.dat") || die "Cannot open DRCAT.dat";

open(DRNEW, ">DRCAT.new") || die "Cannot open DRCAT.new";

$line = 0;
$iline = 0;
while (<DRCAT>) {
    $line++;
    $keep = 1;
    if(/^[\#]/){print DRNEW; next}
    if(/^\s*$/){print DRNEW; next}

    if(/^(\S+) +([^\n]+)/){
	$pref = $1;
	$rest = $2;
	if($rest eq "None" || $rest eq "Unknown" ) {
	    if($required{$pref}) {
		print STDERR "$line:  REQUIRED: $_";
		$keep = 0;
	    }
	    elsif($preferred{$pref}) {
		print STDERR "$line: PREFERRED: $_";
		$keep = 0;
	    }
	    else {
		if($rest eq "None" && $pref eq "Taxon") {
		    $oid{$did}++;
		}
		else {
		    print STDERR "$line:     EMPTY: $_";
		}
		$keep = 0;
	    }
	}
	else {
	    if($rest =~ /\s\s\s+$/) {
		print STDERR "$line: SPACES: $_";
	    }
	    if($pref =~ /^ID$/) {
		if($iline) {checkid()}
		$did = $rest;
		%edamid = ();
		%edamdat = ();
		%edamfmt = ();
		%edamtpc = ();
		%qdat = ();
		%qfmt = ();
		%qid = ();
		%oid = ();
		%rid = ();
		%xid = ();
		$contact = "unknown-contact";
		$email = "unknown-email";
		$iline = $line;
	    }
	    elsif($pref =~ /^Taxon/) {
		$oid{$did}++;
	    }
	    elsif($pref =~ /^Acc/) {
		$dac = $rest;
		$catid{$did} = $dac;
		$catac{$dac} = $did;
	    }
	    elsif($pref =~ /^Contact/) {
		$contact = $rest;
	    }
	    elsif($pref =~ /^Email/) {
		$email = $rest;
	    }
	    elsif($pref =~ /^Desc/) {
		if($rest =~ /[\|]/) {
		    print STDERR "$line: BAD-DESC: $_";
		}
	    }
	    elsif($pref =~ /^Xref/) {
		($type, $ids) = ($rest =~ /^(\S+) [\|] (.*)/);
		if(!defined($xrefs{$type})){
		    print STDERR "$line: XREF-TYPE $type: $_";
		}
		elsif($type eq "SP_explicit") {
		    if($ids =~ /[Nn]one/) {
		    print STDERR "$line: XREF-NONE ($did)\n";
		    }
		}
		$rid{$type}++;
	    }
	    elsif($pref =~ /^Query/) {
		$rest =~ s/ [\{][^\}]*[\}]//g;
		($data, $fmt, $ids, $url, $xtra) = split(/ [\| ] /, $rest);
		if($data eq "Unknown" && $id eq "Unknown" && $fmt eq "Unknown" && $url eq "Unknown") {
		    $keep = 0;
		    print STDERR "$line: skip\n";
		}
		if(!defined($url) || defined($xtra)){
		    print STDERR "$line: BAD-QUERY: $_";
		}

		$qdat{$data}++;
		if(!keys(%edamdat)) {
			print STDERR "$line: QRY EDAMdat MISSING '$data': $_";
		}
		elsif(!defined($edamdat{$data})) {
			print STDERR "$line: QRY DATA '$data': $_";
		}

		$qfmt{$fmt}++;
		if(!defined($edamfmt{$fmt}) && !defined($knownfmt{$fmt})) {
			print STDERR "$line: QRY FORMAT '$fmt': $_";
		}

		@ids = split(/;/, $ids);
		foreach $qid (@ids) {
		    if(!defined($edamid{$qid})) {
			if(defined($edamdat{$qid})) {
			    print STDERR "$line: QRY ID=DAT '$qid': $_";
			}
			elsif ($qid eq "None") {}
			else {
			    print STDERR "$line: QRY ID '$qid': $_";
			    $badqid{$qid}++;
			}
		    }
		    $qid{$qid}++;
		}
	    }
	    elsif($pref =~ /^Example/) {
		$rest =~ s/ [\{][^\}]*[\}]//g;
		($ids, $values) = split(/ [\| ] /, $rest);
		if($data eq "Unknown" && $id eq "Unknown" && $fmt eq "Unknown" && $url eq "Unknown") {
		    $keep = 0;
		    print STDERR "$line: skip\n";
		}
		if(!defined($values)){
		    print STDERR "$line: BAD-QUERY: $_";
		}

		@ids = split(/;/, $ids);
		foreach $xid (@ids) {
		    if(!defined($qid{$xid})) {
			print STDERR "$line: EXAMPLE ID NO QRY '$xid': $_";
		    }
		    elsif(!defined($edamid{$xid})) {
			if(defined($edamdat{$xid})) {
			    print STDERR "$line: EXAMPLE ID=DAT '$xid': $_";
			}
			else {
			    print STDERR "$line: EXAMPLE ID '$xid': $_";
			    $badxid{xqid}++;
			}
		    }
		    $xid{$xid}++;
		}
		@values = split(/;/, $values);
		if($#values != $#ids) {
		    print STDERR "$line: EXAMPLE $#ids ids $#values values: $_";
		}
		foreach $xval (@values) {
		}
	    }
	    elsif($pref =~ /^EDAM/) {
		($term, $name) = ($rest =~ /^(\d+) [\|] (.*)/);
		if(!defined($term)) {
		    print STDERR "$line: BAD LINE: $_";
		}
		elsif(defined($trueid{$term})) {
		    print STDERR "$line: EDAM-ALTID $trueid{$term} $_";
		}
		elsif ($pref eq "EDAMtpc") {
		    $edamtpc{$name} = $term;
		    if(!defined($edam{$term})) {
			print STDERR "$line: NOT-IN-EDAM: $_";
		    }
		    elsif($edamspace{$term} ne "topic") {
			print STDERR "$line: EDAM-NAMESPACE $edamspace{$term}: $_";
		    }
		}
		elsif ($pref eq "EDAMfmt") {
		    $edamfmt{$name} = $term;
		    if(!defined($edam{$term})) {
			print STDERR "$line: NOT-IN-EDAM: $_";
		    }
		    elsif($edamspace{$term} ne "format") {
			print STDERR "$line: EDAM-NAMESPACE $edamspace{$term}: $_";
		    }
		}
		elsif ($pref eq "EDAMdat") {
		    $edamdat{$name} = $term;
		    if(!defined($edam{$term})) {
			print STDERR "$line: NOT-IN-EDAM: $_";
		    }
		    elsif($edamspace{$term} ne "data") {
			print STDERR "$line: EDAM-NAMESPACE $edamspace{$term}: $_";
		    }
		}
		elsif ($pref eq "EDAMid") {
		    $edamid{$name} = $term;
		    if(!defined($edam{$term})) {
			print STDERR "$line: NOT-IN-EDAM: $_";
		    }
		    elsif($edamspace{$term} ne "identifier") {
			print STDERR "$line: EDAM-NAMESPACE $edamspace{$term}: $_";
		    }
		}
		elsif ($pref eq "EDAMres") {
		    print STDERR "$line: EDAM-RESOURCE $edamspace{$term}: $_";
		}
		if($isobs{$term}) {
		    print STDERR "$line: EDAM-OBSOLETE: $_";
		}
		if($edam{$term} ne $name) {
		    print STDERR "$line: EDAM-NAME '$edam{$term}': $_";
		}
	    }
	}
    }
    if($keep) {print DRNEW}
}

close DRCAT;
checkid();

sub numqid()
{
    my $ret = ($badqid{$a} <=> $badqid{$b});
    if(!$ret) {$ret = $a cmp $b};
    $ret;
}
sub numxqid()
{
    my $ret = ($noexample{$a} <=> $noexample{$b});
    if(!$ret) {$ret = $a cmp $b};
    $ret;
}
print STDERR "\nSummary:\n\n";
print STDERR "\nUndefined query id:\n\n";
foreach $b (sort numqid (keys(%badqid))) {
    printf STDERR "%6d %s\n", $badqid{$b}, $b;
}
print STDERR "\nNo example for query id:\n\n";
foreach $qi (sort numxqid (keys(%noexample))) {
    printf STDERR "%6d %s\n", $noexample{$qi}, $qi;
}

foreach $x(sort(keys(%catac))) {
    if(!defined($xrac{$x})) {
	print STDERR "DB-BADAC: $x $catac{$x}\n";
    }
}
foreach $x(sort(keys(%xrac))) {
    if(!defined($catac{$x})) {
	print STDERR "NEWDB-AC: $x $xrac{$x}\n";
    }
}
foreach $x(sort(keys(%xrid))) {
    if(!defined($catid{$x})) {
	print STDERR "NEWDB-ID: $xrid{$x} $x\n";
    }
}
