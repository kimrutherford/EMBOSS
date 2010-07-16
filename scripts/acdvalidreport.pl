#!/usr/bin/perl -w

%knownmsg = (
   "^Section \\S+ follows section" => "Section out of order",
   "^Documentation string \\d+ exceeds" => "Documentation string length",
   "^Calculated standard value for \\S+" => "Calculated standard value",
   "^Calculated additional value for \\S+" => "Calculated additional value",
   "^Standard qualifier '\\S+' in section '\\S+'" => "Standard badsection",
   "^Additional qualifier '\\S+' in section '\\S+'" => "Additional badsection",
   "^Advanced qualifier '\\S+' in section '\\S+'" => "Advanced badsection",
   "^First \\S+ (\\S+ )?qualifier '\\S+' is not" => "Bad first qualname",
   "^\\S+ (\\S+ )?qualifier '\\S+' is not" => "Bad qualname",
   "^No knowntype specified for" => "Knowntype missing",
   "^Pattern but no knowntype specified for" => "Pattern without knowntype",
   "^Knowntype '[^']+' not defined" => "Knowntype undefined",
   "^Knowntype '[^']+' defined for type '[^']+', used" => "Knowntype wrong type",
   "^Expected \\S+ (\\S+ )?qualifier is '\\S+' found '\\S+'" => "Bad expected qualname",
   "^Unexpected information value for" => "Unexpected information",
   "^\\S+ string for '\\S+' starts in lower case" => "String start lowercase",
   "^\\S+ string for '\\S+' starts non-alphabetic" => "String start nonalpha",
   "Section info '[^']+' expected, case mismatch" => "Section info badcase",
   "^Qualifier '\\S+' type '\\S+' not in section" => "Qualifier badsection",
   "^No section defined for qualifier '\\S+'" => "No section",
   "^Sub level section '\\S+' should be under" => "Subsection badsection",
   "Parameter defined as false" => "Parameter false",
   "Section '\\S+' not defined in sections[.]standard file" => "Section undefined",
   "First \\S+ (\\S+ )?'\\S+' is not a parameter" => "Nonparameter first",
   "Subsequent \\S+ (\\S+ )?'\\S+' is not a parameter" => "Nonparameter later",
   "Sequence set '\\S+' has no 'aligned'" => "Seqset aligned undefined",
#   "" => "",
   "^Multiple definition of parameter/standard/additional" => "Multiple definition of parameter/standard/additional",
   "Information string for '\\S+' '[^']+' not standard" => "Nonstandard info",
   "Missing standard information '\\S+' expected" => "Missing standard info",
   "Dummy message" => "dummy"
	     );

%knownexp = ();
$applcnt = 0;
$embassycnt = 0;

foreach $x (sort (keys (%knownmsg))) {
    $xname = $knownmsg{$x};
    $knownexp{$xname} = qr/$x/;
}

while (<>) {
    if (/^(Error|Warning): File ([^.]+)[.]acd line \d+: (.*)/) {
	$type = $1;
	$file = $2;
	$message = $3;
	$found=0;
	foreach $x (keys (%knownexp)) {
	    if ($message =~ /$knownexp{$x}/) {
		if ($type eq "Error") {$y = "* $x"}
		else {$y = $x}
		$countknown{$y}++;
#		print "$file: Message '$y' $countknown{$y}: $message\n";
		$found=1;
		last;
	    }
	}
	if (!$found) {
	    $countmsg{$message}++;
	    print "$file: Message '++other++' $countmsg{$message}: $message\n";
	}
	if($isembassy) {$embassy{$index} .= $_}
	else {$apps{$index} .= $_}
    }
    elsif (/^[+](\S+)\s+[\(]([a-zA-Z0-9]+)[\)]$/) {
	$embassycnt++;
	$index = "$2 $1";
	$embassy{$index} = $_;
	$isembassy=1;
    }

    elsif (/^(\S+)$/) {
	$applcnt++;
	$index = $1;
	$apps{$index} = $_;
	$isembassy=0;
    }

}

foreach $x (sort (keys ( %countknown ) ) ) {
    printf "%4d %s\n", $countknown{$x}, $x;
}

print "\n$applcnt EMBOSS and $embassycnt EMBASSY applications\n";

foreach $x(sort(keys(%apps))) {
    print "\n$apps{$x}";
}

foreach $x(sort(keys(%embassy))) {
    print "\n$embassy{$x}";
}
