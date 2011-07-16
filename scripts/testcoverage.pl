#!/usr/bin/perl -w

sub parseline($) {
    my ($line) = @_;
    my $cols;
    my $desc;
    my $i = 0;
    @cols = ();
    if($line =~ /^  (\S+[^\"]+) \"([^\"]+)\"/) {
	$cols = $1;
	$desc = $2;
	@cols = split(/\s+/, $cols);
	if(defined($cols[0])) {$name = $cols[0]}
	else { $name = $cols}
#	if($name =~ /Unknown/i) {return 0}
	while(defined($cols[$i])) {
	    if($cols[$i] eq "Yes") {$cols[$i] = 1}
	    elsif($cols[$i] eq "No") {$cols[$i] = 0}
	    $i++;
	}
	return 1 + $#cols;
    }

    return 0;
}

sub qatest($$){
    my ($id,$prog) = @_;
    if(!defined($qatest{$id})) {
	print STDERR "No test: $id\n";
	return 0;
    }
    if($qatest{$id} ne $prog) {
	print STDERR "Bad app: $qatest{$id} ($prog) for $id\n";
	return 0;
    }
    return $prog;
}

open (VERSION, "embossversion -full -filter|") ||
    die "Cannot run embossversion";

while (<VERSION>){
   if(/BaseDirectory: (\S+)/) {
	$basedir = $1;
    }
   if(/RootDirectory: (\S+)/) {
	$rootdir = $1;
    }
}
close VERSION;

open (QA, "$basedir/test/qatest.dat") ||
    die "Cannot find qatest.dat";
while(<QA>){
    if(/^ID +(\S+)/) {$id = $1}
    if(/^A[APQ] +(\S+)/) {$qatest{$id} = $1}
}
close QA;

open (ENTRAILS, "$rootdir/entrails -full -filter|") ||
    die "Cannot run entrails";

$in = "";
while (<ENTRAILS>){

    if(/^\s*\}/) {$in = ""}
    if(/^\S.*\{/) {$found{$in}++}

    if(/^# Assembly input formats/) {$in = "inassem"}
    elsif(/^# Assembly output formats/) {$in = "outassem"}

    elsif(/^# Codon usage input formats/) {$in = "incod"}
    elsif(/^# Codon usage output formats/) {$in = "outcod"}

    elsif(/^# Feature input formats/) {$in = "infeat"}
    elsif(/^# Feature output formats/) {$in = "outfeat"}

    elsif(/^# Obo term input formats/) {$in = "inobo"}
    elsif(/^# Obo term output formats/) {$in = "outobo"}

    elsif(/^# Data resource input formats/) {$in = "inresource"}
    elsif(/^# Data resource output formats/) {$in = "outresource"}

    elsif(/^# Sequence input formats/) {$in = "inseq"}
    elsif(/^# Sequence output formats/) {$in = "outseq"}

    elsif(/^# Taxonomy input formats/) {$in = "intax"}
    elsif(/^# Taxonomy output formats/) {$in = "outtax"}

    elsif(/^# Text input formats/) {$in = "intext"}
    elsif(/^# Text output formats/) {$in = "outtext"}

    elsif(/^# Url input formats/) {$in = "inurl"}
    elsif(/^# Url output formats/) {$in = "outurl"}

    elsif(/^# Variation input formats/) {$in = "invar"}
    elsif(/^# Variation output formats/) {$in = "outvar"}

    elsif(/^# Alignment output formats/) {$in = "outalign"}
    elsif(/^# Report output formats/) {$in = "outreport"}

    elsif(/^# Sequence types/) {$in = "seqtype"}

    elsif(/^# Graphics devices/) {$in = "graph"}

    elsif(/^# Assembly access methods/) {$in = "assemmethod"}
    elsif(/^# Feature access methods/) {$in = "featmethod"}
    elsif(/^# Obo term access methods/) {$in = "obomethod"}
    elsif(/^# Data resource access methods/) {$in = "resourcemethod"}
    elsif(/^# Sequence access methods/) {$in = "seqmethod"}
    elsif(/^# Taxonomy access methods/) {$in = "taxmethod"}
    elsif(/^# Text access methods/) {$in = "textmethod"}
    elsif(/^# Url access methods/) {$in = "urlmethod"}
    elsif(/^# Variation access methods/) {$in = "varmethod"}

    elsif(/^# .* access methods/) {print STDERR "Unknown: $_"}
    elsif(/^# .* formats/) {print STDERR "Unknown: $_"}

    # 10 input types
    # 12 output types
    # sequence types
    # graphics devices
    # 9 access methods

    if($in ne ""){
	if(/^\S.*\{/) {
	    $found{$in}++;
	    next;
	}

#input types

	if($in eq "inassem") {
	    if(parseline($_)){$inassem{$name}++}
	}
	elsif($in eq "incod") {
	    if(parseline($_)){$incod{$name}++}
	}
	elsif($in eq "infeat") {
	    if(parseline($_)){$infeat{$name}++}
	}
	elsif($in eq "inobo") {
	    if(parseline($_)){$inobo{$name}++}
	}
	elsif($in eq "inresource") {
	    if(parseline($_)){$inresource{$name}++}
	}
	elsif($in eq "inseq") {
	    if(parseline($_)){
		$inseq{$name}++;
		($n,$alias,$try,$nuc,$pro,$feat,$gap,$mset) = @cols;
		qatest("seq-in-$name", "seqret");
		if(!$alias) {
		    if($feat) {qatest("seq-infeat-$name", "seqret")}
		    if($mset) {qatest("seq-infeat-$name", "nthseqset")}
		    if($nuc && $pro) {
			qatest("seq-innuc-$name", "infoseq");
			qatest("seq-inprot-$name", "infoseq");
		    }
		}
		if($try) {qatest("seq-inauto-$name", "seqret")}
	    }
	}
	elsif($in eq "intax") {
	    if(parseline($_)){$intax{$name}++}
	}
	elsif($in eq "intext") {
	    if(parseline($_)){$intext{$name}++}
	}
	elsif($in eq "inurl") {
	    if(parseline($_)){$inurl{$name}++}
	}
	elsif($in eq "invar") {
	    if(parseline($_)){$invar{$name}++}
	}

	# output types
	elsif($in eq "outassem") {
	    if(parseline($_)){$outassem{$name}++}
	}
	elsif($in eq "outcod") {
	    if(parseline($_)){$outcod{$name}++}
	}
	elsif($in eq "outfeat") {
	    if(parseline($_)){$outfeat{$name}++}
	}
	elsif($in eq "outobo") {
	    if(parseline($_)){$outobo{$name}++}
	}
	elsif($in eq "outresource") {
	    if(parseline($_)){$outresource{$name}++}
	}
	elsif($in eq "outseq") {
	    if(parseline($_)){$outseq{$name}++}
	}
	elsif($in eq "outtax") {
	    if(parseline($_)){$outtax{$name}++}
	}
	elsif($in eq "outtext") {
	    if(parseline($_)){$outtext{$name}++}
	}
	elsif($in eq "outurl") {
	    if(parseline($_)){$outurl{$name}++}
	}
	elsif($in eq "outvar") {
	    if(parseline($_)){$outvar{$name}++}
	}
	elsif($in eq "outalign") {
	    if(parseline($_)){$outalign{$name}++}
	}
	elsif($in eq "outreport") {
	    if(parseline($_)){$outreport{$name}++}
	}

	# other

	elsif($in eq "seqtype") {
	    if(parseline($_)){$seqtype{$name}++}
	}
	elsif($in eq "graph") {
	    if(parseline($_)){$graph{$name}++}
	}

	# access methods

	elsif($in eq "assemmethod") {
	    if(parseline($_)){$methodassem{$name}++}
	}
	elsif($in eq "featmethod") {
	    if(parseline($_)){$methodfeat{$name}++}
	}
	elsif($in eq "obomethod") {
	    if(parseline($_)){$methodobo{$name}++}
	}
	elsif($in eq "resourcemethod") {
	    if(parseline($_)){$methodresource{$name}++}
	}
	elsif($in eq "seqmethod") {
	    if(parseline($_)){
		$methodseq{$name}++;
	    }
	}
	elsif($in eq "taxmethod") {
	    if(parseline($_)){$methodtax{$name}++}
	}
	elsif($in eq "textmethod") {
	    if(parseline($_)){$methodtext{$name}++}
	}
	elsif($in eq "urlmethod") {
	    if(parseline($_)){$methodurl{$name}++}
	}
	elsif($in eq "varmethod") {
	    if(parseline($_)){$methodvar{$name}++}
	}
    }
}
close ENTRAILS;

print "Inputs:\n";
printf " %3d assem\n", scalar(keys(%inassem));
printf " %3d codon\n", scalar(keys(%incod));
printf " %3d feat\n", scalar(keys(%infeat));
printf " %3d obo\n", scalar(keys(%inobo));
printf " %3d resource\n", scalar(keys(%inresource));
printf " %3d seq\n", scalar(keys(%inseq));
printf " %3d tax\n", scalar(keys(%intax));
printf " %3d text\n", scalar(keys(%intext));
printf " %3d url\n", scalar(keys(%inurl));
printf " %3d var\n", scalar(keys(%invar));
print "\n";

print "Outputs:\n";
printf " %3d assem\n", scalar(keys(%outassem));
printf " %3d codon\n", scalar(keys(%outcod));
printf " %3d feat\n", scalar(keys(%outfeat));
printf " %3d obo\n", scalar(keys(%outobo));
printf " %3d resource\n", scalar(keys(%outresource));
printf " %3d seq\n", scalar(keys(%outseq));
printf " %3d tax\n", scalar(keys(%outtax));
printf " %3d text\n", scalar(keys(%outtext));
printf " %3d url\n", scalar(keys(%outurl));
printf " %3d var\n", scalar(keys(%outvar));
print "\n";
printf " %3d align\n", scalar(keys(%outalign));
printf " %3d report\n", scalar(keys(%outreport));
print "\n";

print "Other:\n";
printf " %3d seqtype\n", scalar(keys(%seqtype));
printf " %3d graph\n", scalar(keys(%graph));
print "\n";

print "Methods:\n";
printf " %3d assem\n", scalar(keys(%methodassem));
printf " %3d feat\n", scalar(keys(%methodfeat));
printf " %3d obo\n", scalar(keys(%methodobo));
printf " %3d resource\n", scalar(keys(%methodresource));
printf " %3d seq\n", scalar(keys(%methodseq));
printf " %3d tax\n", scalar(keys(%methodtax));
printf " %3d text\n", scalar(keys(%methodtext));
printf " %3d url\n", scalar(keys(%methodurl));
printf " %3d var\n", scalar(keys(%methodvar));
print "\n";
