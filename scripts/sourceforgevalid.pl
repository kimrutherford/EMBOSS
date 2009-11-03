#!/usr/bin/perl -w


%validhref = ("www.sanger.ac.uk/" => "Sanger",
	      "www.bioinformatics.ac.uk/" => "CCP11",
	      "/Jemboss/jnlp/Jemboss.jnlp" => "Jemboss.jnlp",
	      "/Jemboss/jnlp_axis/DNADraw.jnlp" => "DNADraw.jnlp",
	      "/Jemboss/jnlp_axis/Jemboss_Alignment.jnlp" => "Jemboss_Alignment.jnlp",
	      "/Jemboss/statistics" => "Jemboss stats",
	      "/Jemboss/statistics/hits.html" => "Jemboss hits",
	      "srs.ebi.ac.uk/" => "SRS ebi",
	      "www.sanger.ac.uk/Software/Wise2/" => "Sanger Wise2",
	      "www.sanger.ac.uk/Software/Pfam/" => "Sanger Pfam",
	      "www.sanger.ac.uk/Software/formats/GFF/" => "Sanger GFF",
	      "www.sanger.ac.uk/" => "Sanger",
	      "http://www.dl.ac.uk/CCP/CCP11/newsletter/vol1_1/DISguISE.html" => "DISguiSE",
	      "http://www.es.embnet.org/~bioinfo/Colimate/Extra_Docs/" => "CoLiMate",
	      "http://aig.cs.man.ac.uk/research/utopia/cinema/cinema.php" => "CINEMA"
#	      "www.hgmp.mrc.ac.uk/" => "HGMP home"
	      );

%fixhref = ("" => "",
	    "www.sanger.ac.uk/srs/" => "srs.ebi.ac.uk/",
	    "www.sanger.ac.uk/srs6/" => "srs.ebi.ac.uk/",
	    "srs.rfcgr.mrc.ac.uk/" => "srs.ebi.ac.uk/",
	    "srs.hgmp.mrc.ac.uk/" => "srs.ebi.ac.uk/",
	    "www.sanger.ac.uk/Users/pmr/" => "www.ebi.ac.uk/~pmr/",
	    "www.hgmp.mrc.ac.uk/Software/EMBOSS/Apps/index.html" => "emboss.sourceforge.net/apps/",
	    "www.rfcgr.mrc.ac.uk/Software/EMBOSS/Apps/index.html" => "emboss.sourceforge.net/apps/",
	    "www.rfcgr.mrc.ac.uk/Software/EMBOSS/" => "/",
	    "www.hgmp.mrc.ac.uk/Software/EMBOSS/" => "/",
	    "www.sanger.ac.uk/Software/EMBOSS/" => "/",
	    "www.rfcgr.mrc.ac.uk/CCP11/index.jsp" => "www.bioinformatics.ac.uk/",
	    );

%ignorelref = ("serializedForm" => "JavaApi",
		"serialized_methods" => "JavaApi",
		"readObject(java.io.ObjectInputStream)" => "JavaApi",
		"writeObject(java.io.ObjectOutputStream)" => "JavaApi",
		);
sub processfile($$) {
    my ($filename,$path) = @_;
    my $fullname = "$path/$filename";
    #print "Process file $fullname\n";
    $fileprint .= "    Process file $fullname\n";
    my $ferr = 0;
    my $lpath = $path;

    if ($path =~ /(.*)\/inc\/?/) {$lpath = $1}

    my %iref=();
    my %lref=();

    open (IN, "$fullname") || die "Cannot open $fullname";
    $head = 0;
    while (<IN>) {
	if (/<head/) {$head=1}
	if (/<\/head/) {$head=0}
	if (/name=\"([^\"]+)\"/ig && !$head) {
	    #print "  Local name '$1'\n";
	    if (defined($lref{$1}) && !defined($ignorelref{$1})) {
		if(!$ferr){$ferr++;print $fileprint;}
		print "+ Duplicate name \#$1\n";
	    }
	    $lref{$1}=1;
	}
	if (/(href|src)=\"http:\/\/([^\"]+)\"/ig) {
	    $href = $2;
	    #print "  Remote href: '$href'\n";
	    $ok=1;
	    if ($href=~ /[.]rfcgr[.]/ig) {$ok=0}
	    if ($href=~ /[.]hgmp[.]/ig) {$ok=0}
	    if ($href=~ /[.]sanger[.]/ig) {$ok=0}
	    if ($href=~ /[^w]emboss[.]org/ig) {$ok=0}
	    if ($href=~ /embnet[.]org/ig) {$ok=0}
	    if ($validhref{$href}) {$ok=1}
	    if (!$ok) {
		if ($fixhref{$href}) {
		    if(!$ferr){$ferr++;print $fileprint;}
		    print "+  Fix remote href: '$href'\n";
		    print "+                => '$fixhref{$href}'\n";
		}
		elsif ($href=~ /^srs.[hgmprfc]+.mrc.ac.uk\/srs7bin\/cgi-bin\/wgetz(.*)/){
		    if(!$ferr){$ferr++;print $fileprint;}
		    print "+  Fix remote href: '$href'\n";
		    print "+                => 'srs.ebi.ac.uk/srs7bin/cgi-bin/wgetz$1'\n";
		}
		elsif ($href=~ /^srs.ebi.ac.uk\/srs7bin\/cgi-bin\/wgetz/){
		}
		else {
		    $badremote{$href}++;
		    if(!$ferr){$ferr++;print $fileprint;}
		    print "+  Bad remote href: '$href'\n";
		}
	    }
	}
	elsif (/href=\"?\#([^:\"\>]+)\"/ig) {
	    $href = $1;
	    #print "  Internal href: '$href'\n";
	    $iref{$href}++;
	}
	elsif (/(href|src)=\"([^:\"\#]+)\"/ig) {
	    $href = $2;
	    #print "  Local href: '$href'\n";
	    $ok = 0;
	    if ($href =~ /^\//) {
		if (-e ".$href") {$ok=1}
	    }
	    else {
		if (-e "$lpath/$href") {$ok=1}
	    }
	    if (!$ok) {
		if(!$ferr){$ferr++;print $fileprint;}
		print "+  Bad local href: '$href'\n";
		if ($href =~ /^\//) {
		    $fullpath = ".$href";
		}
		else {
		    $fullpath = "$lpath/$href";
		    $fullpath =~ s/\/[^\/]+\/[.].//g;
		}
		if(!$ferr){$ferr++;print $fileprint;}
		print "+       Not found: '$fullpath'\n";
		$badlocal{$fullpath}++;
	    }
	}
    }
    close IN;
    foreach $x (keys(%iref)) {
	if (!defined($lref{$x})) {
	    if ($filename =~ /[.]usage$/) {
		    if($x =~ /^input[.]\d+$/) {next}
		    if($x =~ /^output[.]\d+$/) {next}
	    }
	    if(!$ferr){$ferr++;print $fileprint;}
	    print "+ Bad internal href: '\#$x'\n";
	    $badinternal{"$path/$filename\#$x"}++;
	}
    }
}

sub processdir($$) {

    my ($dirname,$path) = @_;
    my $file = "";
    my $mypath = $dirname;
    local *DIR;
    if ($path ne "") {$mypath = "$path/$dirname"}
    opendir(DIR, $mypath);

    print "Directory: $mypath\n";
    while ($file = readdir(DIR)) {
	if ($file =~ /~$/) {next}
	if ($file =~ /^[.]/) {next}
	if ($file =~ /[.]tar$/) {next}
	if ($file =~ /[.]gif$/) {next}
	if ($file =~ /[.]png$/) {next}
	if ($file =~ /[.]jpg$/) {next}
	if ($file =~ /[.]jpeg$/) {next}
	if ($file =~ /[.]ps$/) {next}
	if ($file =~ /[.]pdf$/) {next}
	if ($file =~ /[.]gz$/) {next}
	if ($file =~ /[.]Z$/) {next}
	if (-d "$mypath/$file") {
	    #print "  Directory $file\n";
	    $fileprint = "";
	    &processdir($file,$mypath);	#& avoids prototype recursion warning
	}
	else {
	    #print "    File $file\n";
	    $fileprint = "";
	    &processfile($file, $mypath); # &avoids prototype recursion warning
	}
    }
    print "  Closing directory $mypath\n";
    closedir(DIR);
}

open (VERS, "embossversion -full -auto|") || die "Cannot run embossversion";
while (<VERS>) {
#    if(/InstallDirectory: +(\S+)/) {$installtop = $1}
    if(/BaseDirectory: +(\S+)/) {$distribtop = $1}
}
close VERS;

$fileprint = "";

chdir("$ENV{HOME}sfdoc");
processdir(".","");

print "\n";
print "Bad Remote Hrefs\n";
print "================\n";
foreach $x(sort(keys(%badremote))) {
    printf "%5d %s\n", $badremote{$x}, $x;
}

print "\n";
print "Bad Local Hrefs\n";
print "===============\n";
foreach $x(sort(keys(%badlocal))) {
    printf "%5d %s\n", $badlocal{$x}, $x;
}

print "\n";
print "Bad Internal Hrefs\n";
print "================\n";
foreach $x(sort(keys(%badinternal))) {
    printf "%5d %s\n", $badinternal{$x}, $x;
}
