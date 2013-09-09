#!/usr/bin/perl -w

# retmode=text for text data
# rettype=(format)

# see:
# http://eutils.ncbi.nlm.nih.gov/corehtml/query/static/efetchseq_help.html
# http://eutils.ncbi.nlm.nih.gov/corehtml/query/static/efetchlit_help.html
# http://eutils.ncbi.nlm.nih.gov/corehtml/query/static/efetchtax_help.html
# http://www.ncbi.nlm.nih.gov/books/NBK3837/ (Entrez Utils help book)
#
# for table of formats (retmode and rettype) for each db
# http://www.ncbi.nlm.nih.gov/books/NBK25499/table/chapter4.chapter4_table1/?report=objectonly

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

# to fill in:
# examples (required for testing access and formats)
# EDAM terms

%knownformats = (
    "gene" => "text", # ()=>(asn.1) ()=>xml gene_table=>text
    "snp" => "chr fasta flt rsr brief docset ssexemplar genxml uilist",
    "genome" => "",
    "seqannot" => "",
    "nucleotide" => "genbank fasta", # 
    "nuccore" => "genbank fasta",    # genbank:gb=>text also gb=>xml gbc=>xml ft=>text
    "nucest" => "genbank fasta", # genbank:gb=>text also gb=>xml gbc=>xml
    "nucgss" => "genbank fasta", # genbank:gb=>text also gb=>xml gbc=>xml
    "protein" => "genpept fasta", # ft=>text genpept:gp also gp=>xml gpc=>xml
    "popset" => "genbank fasta",# genbank:gb=>text also gb=>xml gbc=>xml
    "sra" => "text xml"			# (full)=>(xml)
    "pmc" => "text xml",	# ()=>(xml) medline=>text
    "pubmed" => "text",
    "journals" => "text",
    "omim" => "text",
    "structure" => "",
    "bioproject" => "",
    "genomeprj" => "",
    "biosample" => "text xml", # (full)=>(xml) (full)=>text
    "biosystems" => "xml", # (xml)=>(xml)
    "blastdbinfo" => "",
    "books" => "",
#   "cancerchromosomes" => "",
    "cdd" => "",
    "gap" => "",
    "dbvar" => "",
    "epigenomics" => "",
#   "gensat" => "",
    "gds" => "Text",	# (summary)=>(text)
    "geo" => "",
    "geoprofiles" => "",
    "homologene" => "text fasta xml", # ()=>(asn.1) ()=>xml alignmentscores=>text fasta->text homologene->text
    "mesh" => "text",		  # (full)=>(text)
    "ncbisearch" => "",
    "nlmcatalog" => "text xml",	# ()=>(text) ()=>xml
    "omia" => "",
    "probe" => "",
    "proteinclusters" => "",
    "pcassay" => "",
    "pccompound" => "",
    "pcsubstance" => "",
    "taxonomy" => "text xml",	# ()=>xml uilist=>text uilist=>xml
    "toolkit" => "",
    "toolkitall" => "",
    "unigene" => "",
    "unists" => "",
    "gencoll" => "",
    "gcassembly" => "",
    "assembly" => "",
    "clone" => "",
    "gapplus" => "",
    "pubmedhealth" => "",
    );

%knowntypes = (
    "gene" => "text",
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
#   "cancerchromosomes" => "",
    "cdd" => "",
    "gap" => "",
    "dbvar" => "",
    "epigenomics" => "",
#   "gensat" => "",
    "gds" => "text",
    "geo" => "",
    "geoprofiles" => "",
    "homologene" => "text sequence",
    "mesh" => "text",
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
    "assembly" => "",
    "clone" => "",
    "gapplus" => "",
    "pubmedhealth" => "",
    );

%dblist = (
    "gene" => "",
    "snp" => "",
    "popset" => "",
    "genome" => "",
    "seqannot" => "",
    "nucleotide" => "",
    "nuccore" => "",
    "nucest" => "",
    "nucgss" => "",
    "protein" => "",
    "pmc" => "",
    "pubmed" => "",
    "journals" => "",
    "omim" => "",
    "sra" => "",
    "structure" => "",
    "genomeprj" => "",
    "bioproject" => "",
    "biosample" => "",
    "biosystems" => "",
    "blastdbinfo" => "",
    "books" => "",
    "cdd" => "",
    "gap" => "",
    "dbvar" => "",
    "epigenomics" => "",
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
    "assembly" => "",
    "clone" => "",
    "gapplus" => "",
    "pubmedhealth" => "",
    );

%examples = (
    "assembly" => "GCA_000001735.1",
    "bioproject" => "171810",
    "biosample" => "839651",
    "biosystems" => "bsid82991",
    "books" => "NBK1918",
    "cdd" => "118125",		# pfam09589
    "clone" => "23875610",
    "dbvar" => "nsv810341",
    "epigenomics" => "4041",
    "gap" => "phs000262.v2.p1",
    "gds" => "GDS3610",		# GEO datasets
    "gene" => "877798",		# amiC PA3364
    "genome" => "4",
    "geo" => "GSM401290",
    "geoprofiles" => "11403194",
    "homologene" => "31384",
    "mesh" => "68009923",
    "nlmcatalog" => "101587112",
    "nuccore" => "CP002688.1 NC_003075.7", # genbankid, refseqid
    "nucest" => "170179497",    # FE739931.1
    "nucgss" => "269911359",	# GS815963.1
    "nucleotide" => "45269",		   # X13776 same as nuccore?
    "omia" => "1920",
    "omim" => "108390",
    "pcassay" => "925",		# pubchem bioassay
    "pccompound" => "358063",		# pubchem compound
    "pcsubstance" => "78180210",	# pubchem substance
    "pmc" => "PMC3029258",
    "popset" => "401672041",
    "probe" => "6103371",
    "protein" => "112986",	# acc P12345
    "proteinclusters" => "PRK15614",
    "pubmed" => "22908228",
    "snp" => "rs2834167",
    "sra" => "SRX042485",
    "structure" => "50791",
    "taxonomy" => "9606",
    "toolkit" => "245792",
    "unigene" => "Mm.15621",
    "unists" => "44811",
    "blastdbinfo" => "",
    "genomeprj" => "",		# same as bioproject?
    "gapplus" => "",
    "gcassembly" => "",		# same as assembly?
    "gencoll" => "",		# genome perhaps?
    "journals" => "",
    "ncbisearch" => "",		# search returns URLs?
    "pubmedhealth" => "",
    "seqannot" => "",
    "toolkitall" => "",		# book or doc page links?
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
DBNAME $db [
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
