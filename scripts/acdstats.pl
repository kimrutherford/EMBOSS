#!/usr/bin/perl -w

use English;

$installdir = "/usr/local";
%acdtypes = ();

%defattr = (
	    "default" => "",
	    "information" => "",
	    "prompt" => "Y",
	    "code" => "Y",
	    "help" => "",
	    "parameter" => "",
	    "standard" => "",
	    "additional" => "",
	    "missing" => "",
	    "valid" => "",
	    "expected" => "",
	    "needed" => "Y",
	    "knowntype" => "",
	    "relations" => "",
	    "style" => "Y",
	    "qualifier" => "Y",
	    "template" => "Y",
	    "comment" => "Y"
	);

sub numerically { $acdtypes{$ty}{$b} <=> $acdtypes{$ty}{$a} }

sub acdparse ($$) {
    my ($name, $file) = @_;

    open (FILE, $file) || die "Cannot open ACD file $file";
    my $acd = "";
    while (<FILE>) {
        s/[\n]/ /;
	s/[ \\ ]/ /;
	$acd .= $_;
    }
    close FILE;

    while ($acd =~ /(\S+): +(\S+) +([^\[ ]+ +)?[\[]([^\]]+)[\]]/gs) {
	$acdtype = $1;
	$acdname = $2;
	$attr = $4;
	$acd = $POSTMATCH;

	if (!defined($acdtypes{$acdtype})) {
	    $acdtypes{$acdtype} = {};
	}

	$acdcount{$acdtype}++;

#	print "$acdtype: $acdname\n";
	while ($attr =~ /([a-z]+): (\"[^\"]*(\"\S[^\"]*)*\"|\S+)/gs) {
#	    print "attr=$1\n";
	    $aname = $1;
	    $acdtypes{$acdtype}{$aname}++;
	    if ($aname eq "prompt") {
		print "+++prompt+++ $aname $name $acdname\n";		 
	    }
	    if ($aname eq "missing") {
		print "+++missing+++ $aname $name $acdname\n";		 
	    }
	    $attr = $POSTMATCH;
	}
    }
}

open (EMBOSSVERSION, "embossversion -full -auto -filter|") ||
    die "Cannot run embossversion";

while (<EMBOSSVERSION>) {
    if (/InstallDirectory: +(\S+)/) { $installdir = $1 }
}

close EMBOSSVERSION;
$installdir .= "/share/EMBOSS/acd";

opendir(ACDDIR, $installdir);

@acdfiles = readdir(ACDDIR);

foreach $file (@acdfiles) {
    if ($file !~ /^([^.]+)[.]acd$/) {next}
    $name = $1;
#    print "++ $name $file\n";
    acdparse($name, "$installdir/$file");
}

closedir ACDDIR;

foreach $ty (sort (keys ( %acdtypes ) ) ) {
	print "$ty $acdcount{$ty}\n";
    foreach $at (sort numerically keys (%{$acdtypes{$ty}}) ) {
	if (defined($defattr{$at})) {
	    if ($defattr{$at} eq "Y") {
		print "     ** $at $acdtypes{$ty}{$at}\n";
	    }
	    else {
#		print ".   $at $acdtypes{$ty}{$at}\n";
	    }
	}
	else {
		print "        $at $acdtypes{$ty}{$at}\n";
	}
    }
}
