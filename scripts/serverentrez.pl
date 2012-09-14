#!/usr/bin/perl -w

# retmode=text for text data
# rettype=(format)

# see:
# http://eutils.ncbi.nlm.nih.gov/corehtml/query/static/efetchseq_help.html
# http://eutils.ncbi.nlm.nih.gov/corehtml/query/static/efetchlit_help.html
# http://eutils.ncbi.nlm.nih.gov/corehtml/query/static/efetchtax_help.html
# 
# for table of formats for each type

%sequenceformats = ("genbank" => "gbwithparts", # resolves CON entries
		  "genpept" => "gp",
		  "fasta" => "fasta",
		  "dbest" => "est",
		  "dbgss" => "gss",
    );
%featureformats = ("genbank" => "ft",
    );

%geneformats = ();		# no text format, xml or asn.1

%snpformats = ();		# all are text

%taxformats = ();		# use report=uilist,brief,docsum,xml

%litformats = ();		# text (html) non-journals also xml,asn.1

%knownformats = ("gene" => "",
		 "snp" => "chr,flt,rsr,brief,docset",
		 "popset" => "",
		 "genome" => "",
		 "seqannot" => "",
		 "nucleotide" => "genbank,fasta",
		 "nuccore" => "genbank,fasta",
		 "nucest" => "genbank,fasta",
		 "nucgss" => "genbank,fasta",
		 "protein" => "genpept,fasta",
		 "sra" => "ftplist,asperalist",
		 "pmc" => "text",
		 "pubmed" => "text",
		 "journals" => "text",
		 "omim" => "text",
		 "structure" => "",
		 "bioproject" => "",
		 "genomeprj" => "",
		 "biosample" => "",
		 "biosystems" => "",
		 "blastdbinfo" => "",
		 "books" => "",
		 "cancerchromosomes" => "",
		 "cdd" => "",
		 "gap" => "",
		 "dbvar" => "",
		 "epigenomics" => "",
		 "gensat" => "",
		 "gds" => "",
		 "geo" => "",
		 "geoprofiles" => "",
		 "homologene" => "",
		 "mesh" => "",
		 "ncbisearch" => "",
		 "nlmcatalog" => "",
		 "omia" => "",
		 "probe" => "",
		 "proteinclusters" => "",
		 "pcassay" => "",
		 "pccompound" => "",
		 "pcsubstance" => "",
		 "taxonomy" => "",
		 "toolkit" => "",
		 "toolkitall" => "",
		 "unigene" => "",
		 "unists" => "",
		 "gencoll" => "",
		 "gcassembly" => "",
    );

%knowntypes = ("gene" => "text",
	       "snp" => "text",
	       "popset" => "text",
	       "genome" => "nucleotide",
	       "seqannot" => "text",
	       "nucleotide" => "nucleotide",
	       "nuccore" => "nucleotide",
	       "nucest" => "nucleotide",
	       "nucgss" => "nucleotide",
	       "protein" => "protein",
	       "pmc" => "text",
	       "pubmed" => "text",
	       "journals" => "text",
	       "omim" => "text",
	       "sra" => "text",
		 "structure" => "",
		 "genomeprj" => "",
		 "bioproject" => "",
		 "biosample" => "",
		 "biosystems" => "",
		 "blastdbinfo" => "",
		 "books" => "",
		 "cancerchromosomes" => "",
		 "cdd" => "",
		 "gap" => "",
		 "dbvar" => "",
		 "epigenomics" => "",
		 "gensat" => "",
		 "gds" => "",
		 "geo" => "",
		 "geoprofiles" => "",
		 "homologene" => "",
		 "mesh" => "",
		 "ncbisearch" => "",
		 "nlmcatalog" => "",
		 "omia" => "",
		 "probe" => "",
		 "proteinclusters" => "",
		 "pcassay" => "",
		 "pccompound" => "",
		 "pcsubstance" => "",
		 "taxonomy" => "",
		 "toolkit" => "",
		 "toolkitall" => "",
		 "unigene" => "",
		 "unists" => "",
		 "gencoll" => "",
		 "gcassembly" => "",
    );

open (SERVER, ">server.entrez.new") || die "Cannot open entrez.server.new";

print SERVER "# server.entrez 2011-10-05 14:07:00\n";
print SERVER "
# database names listed by the Einfo utility
# http://eutils.ncbi.nlm.nih.gov/entrez/eutils/einfo.fcgi?

# database dbxyz details listed by
# http://eutils.ncbi.nlm.nih.gov/entrez/eutils/einfo.fcgi?db=dbxyz
";

if (-e "entrez.info") {system "rm entrez.info"}
$status = system "wget -q -O entrez.info \"http://eutils.ncbi.nlm.nih.gov/entrez/eutils/einfo.fcgi?\"";

@dbname = ();
open (DBLIST, "entrez.info") || die "Unable to open 'entrez.info'";
while(<DBLIST>) {
    if(/<DbName>([^<]+)<\/DbName>/) {
	push @dbname, $1;
    }
}
close DBLIST;

foreach $db (@dbname) {
    $type = "Text";
    $format = "Text";
    if(defined($knowntypes{$db})) {$type = $knowntypes{$db};}
    else{print STDERR "Unknown type for '$db'\n"}
    if(defined($knownformats{$db})) {$format = $knownformats{$db}}
    else{print STDERR "Unknown format for '$db'\n"}

    print SERVER "
DB $db [
  type: \"$type\"
  format: \"$format\"
";
    if (-e "entrez.$db") {system "rm entrez.$db"}
    $status = system "wget -q -O entrez.$db \"http://eutils.ncbi.nlm.nih.gov/entrez/eutils/einfo.fcgi?db=$db\"";
    open (DBINFO, "entrez.$db") || die "Unable to open 'entrez.$db'";
    $isfield=0;
    $isintro = 1;
    %fullname = ();
    %fulldesc = ();
    @fields = ();
    while (<DBINFO>) {
	if(/<Field>/) {$isfield = 1;$isintro = 0}
	if(/<\/Field>/) {$isfield = 0}
	if($isintro) {
            if(/<Description>([^<]+)<\/Description>/) {
                 $dbdesc{$db} = $1;
                 print SERVER "  comment: \"$dbdesc{$db}\"\n";
            }
        }
        if(!$isfield){next}

	if(/<Name>([^<]+)<\/Name>/) {
	    $field = $1;
	    push @fields, $field;
	}
	elsif(/<FullName>([^<]+)<\/FullName>/) {
	    $fullname{$field} = $1;
            $fullname{$field} =~ s/[\s\/]/_/g;
            $fullname{$field} =~ s/\&apos;//g;
	}
	elsif(/<Description>([^<]+)<\/Description>/) {
	    $fulldesc{$field} = $1;
            $fulldesc{$field} =~ s/\&apos;/'/g;
	}
    }
    close DBINFO;
    foreach $fd (@fields) {
        print SERVER "  field: \"$fd $fullname{$fd} ! $fulldesc{$fd}\"\n";
    }
print SERVER "]\n";
}

close SERVER;

foreach $d (sort(keys(%knowntypes))){
    if(!defined($dbdesc{$d})) {
        print STDERR "Expected database '$d' not found\n";
    }
}
