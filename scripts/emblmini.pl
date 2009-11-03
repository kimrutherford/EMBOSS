#!/usr/bin/perl -w

opendir (DIR, ".");
while ($file = readdir(DIR)) {
    if($file !~ /^(.*)[.]dat$/) {next}
    open (IN, "$file") || die "Unable to read $file";
    open (OUT, ">$1.mini");
    print STDERR "$file\n";
    while (<IN>) {
	if(/^ID/) {print OUT}
	if(/^AC/) {print OUT}
	if(/^DE/) {print OUT}
	if(/^KW/) {print OUT}
	if(/^OC/) {print OUT}
	if(/^OS/) {print OUT}
	if(/^OG/) {print OUT}
	if(/^SQ/) {$doseq=1;$seqstr1 = "";}
	if(/^  / && $doseq) {
	    if($seqstr1 eq "") {$seqstr1 = $_}
	    $seqstr2 = $_;
	}
	if(/^\/\//) {
	    $doseq=0;
	    $seq = "$seqstr1$seqstr2";
	    %bases=();
	    $bases{"a"} = $bases{"c"} = $bases{"g"} = $bases{"t"} = 0;
	    $tot = 0;
	    while ($seq =~ /([a-z])/go) {
		$bases{$1}++;
		$tot++;
	    }
	    $other = $tot-$bases{"a"}-$bases{"c"}-$bases{"g"}-$bases{"t"};
	    printf OUT "SQ   Sequence %d BP; %d A; %d C; %d G; %d T; %d other;\n", $tot, $bases{"a"}, $bases{"c"}, $bases{"g"}, $bases{"t"}, $other;
	    print OUT $seq;
	    print OUT "//\n";
	}
    }
}
