#!/usr/bin/perl -w

use English;

$basefile = $ARGV[0];
$basefile =~ s/[.]c$//;

open (DEP, "$ENV{HOME}/devemboss/deprecated.txt") || die "Cannot open deprecated.txt";
open (SRC, "$basefile.c") || die "Cannot open $basefile.c";
open (OLDSRC, ">$basefile.save") || die "Cannot open $basefile.save";
#open (DBG, ">fixdeprecated.dbg") || die "Cannot open fixdeprecated.dbg";

$patcnt=0;
$notecnt=0;
$gonecnt=0;
$subcnt=0;
$subcntall=0;
$new=0;

%gone=();
%note=();
%argtest=();
%pat=();
$ok = 1;
while (<DEP>){
    if (/^[\#](\S+)/) {
	if($1 eq $basefile) {$ok = 0}
	else {$ok = 1}
	next;
    }
    if(!$ok) {next}
    if(/^(\S+)\s+[-]$/) {
	$oldname = "$1\\s*[\\(]";
	$gone{$oldname} = 1;
	$gonecnt++;
    }
    elsif(/^(\S+)\s+[=](\S+)\s*(\S+)\s+(\S+)$/) {
	$oldname = "$1";
	$oldpat = "$1\\s*[\\(]([^\\(\\)]*([\\(][^\\)]*[\\)][^\\(\\)]*)*)[\\)]";
	$newname = "$2";
	$oldargs = $3;
	$newargs = $4;
	$subcntall++;
	if(defined($argtest{$oldpat})) {
	    $argtest{$oldpat} .= ";$oldname $newname $oldargs $newargs";
	}
	else {
	    $subcnt++;
	    $argtest{$oldpat} = "$oldname $newname $oldargs $newargs";
	}
    }
    elsif(/^(\S+)\s+[@](\S+)\s*$/) {
	$oldname = "$1\\s*[\\(]";
	$newname = "$2";
	$note{$oldname} = $newname;
	$notecnt++;
    }
    elsif(/^(\S+)\s+(\S+)\s*$/) {
	$oldname = "$1\\s*[\\(]";
	$newname = "$2(";
	$pat{$oldname} = $newname;
	$patcnt++;
    }
}

close DEP;

print "$basefile.c: Using $patcnt patterns, $subcntall edits for $subcnt functions and $gonecnt removals\n";

$savesrc = "";
$cnt=0;
while (<SRC>) {
    $cnt++;
    $savesrc .= $_;
    foreach $g (keys(%gone)) {
	while(/$g/g) {
	    ($gout) = ($g =~ /([^\\]+)/);
	    print "$basefile.c: No replacement for obsolete $gout in line $cnt\n";
	}
    }
}
close SRC;
print OLDSRC $savesrc;

close OLDSRC;

open (NEWSRC, ">$basefile.c") || die "Cannot open $basefile.c for writing";

$savepos = 0;
foreach $n (sort (keys ( %argtest))) {
    pos($savesrc) = $savepos;
    if($savesrc =~ /$n/gs) {
	$savepos = pos($savesrc);
#	print DBG "a matched $n at '$&'\n";
	@subarg = split(/;/, $argtest{$n});
	foreach $x (@subarg) {
	    ($oldn, $newn, $olda, $newa) = split(/ /, $x);
#	    print DBG "oldn '$oldn' newn '$newn' olda '$olda' newa '$newa'\n";
	    $nkey = "$oldn\_$newn";
	    @olda = split(/,/, $olda);
	    @newa = split(/,/, $newa);
	    $arga = "";
	    $i = 0;
	    foreach $a (@olda) {
#		print DBG "Building '$a'\n";
		if($i) {$arga .= ","}
		if($a eq "n") {
		    $arga .= "\\s*NULL\\s*";
		}
		elsif ($a =~ s/^'(.*)'/$1/) {
		    $fix = $1;
		    $fix =~ s/^-/\[-\]/;
		    $arga .= "\\s*$fix\\s*";
		}
		else {
		    $arga .= "[^\\),]*([\\(][^\\)]*[\\)][^\\),]*)*";
		}
#		print DBG "So far '$arga'\n";
		$i++;
	    }

	    $pata = "($oldn"."\\s*[\\(])(".$arga.")[\\)]";
#	    print DBG "b testing $pata\n";
	    pos($savesrc) = 0;	# search from the start
	    while($savesrc =~ /$pata/gs) {
#		print DBG "b matched $pata at '$&'\n";
		$savepre = $PREMATCH;
		$savepost = $POSTMATCH;
		$arglist = $2;
#		print DBG "$oldn '$arglist'\n";
		$i = 0;
		$pat = "[^\\(\\),]*([\\(][^\\)]*[\\)][^,]*)*[^,]*,";
		while ($arglist =~ /$pat/g) {
		    $p = $POSTMATCH;
		    $ai = $&;
		    $ai =~ s/^(\s*)//;
		    $apre[$i] = $1;
		    $ai =~ s/([\s]*)$//;
		    $apost[$i] = $1;
		    $ai =~ s/,$//;
#		    print DBG "arg[$i] '$ai'\n";
		    $ao[$i++] = $ai;
		    $ai = $p;
		}
		$ai =~ s/^(\s*)//;
		$apre[$i] = $1;
		$ai =~ s/([,\s]*)$//;
		$apost[$i] = $1;
		$ao[$i] = $ai;
#		print DBG "Remaining '$ai'\n";
		$newtext = "$newn(";
#		for ($i=0; $i <= $#ao; $i++) {
#		    print DBG "saved ai[$i] '$apre[$i]' '$ao[$i]' '$apost[$i]'\n";
#		}
		$ok = 1;
#		print DBG "Processing newa '$newa' $#newa\n";
		for ($i=0; $i <= $#newa; $i++) {
		    if($newa[$i] =~ /^\d+$/) {
			$j = $newa[$i] - 1;
			$newtext .= $apre[$i];
			$newtext .= $ao[$j];
			if($i < $#newa) {$newtext .= ","}
			$newtext .= $apost[$i];
		    }
		    elsif($newa[$i] =~ /\[(\d+)\]/) {
			$j = $1 - 1;
			$pre = $PREMATCH;
			$post = $POSTMATCH;
			$x = $newa[$i];
			$newtext .= $apre[$i];
			$newtext .= $pre;
			$newtext .= "(";
			$newtext .= $ao[$j];
			$newtext .= ")";
			$newtext .= $post;
			$newtext .= $apost[$i];
		    }
		    elsif($newa[$i] =~ /[*]/) {
#			print DBG "** Cannot define arg $i '$newa[$i]': edit by hand";
			$ok = 0;
		    }
		    else {
#			print DBG "** Cannot understand arg $i '$newa[$i]': edit by hand";
			$ok = 0;
			next;
		    }
#		    print DBG "+ newa[$i] '$newa[$i]' '$newtext'\n";
		}
#		print DBG "ok:$ok newtext '$newtext'\n";
		if($ok) {
		    $savesrc = $savepre . $newtext . ")" . $savepost;
		    pos($savesrc) = length($savepre) + length($newtext);
		    $subdone{$nkey}++;
		}
	    }
	}
    }
#	if(!defined($notedone{$n})) {
#	    ($nout) = ($n =~ /([^\\]+)/);
#	    print "$basefile.c: Replace $nout with $note{$n}\n";
#	}
}

foreach $p (sort (keys ( %pat))) {
    $repcnt=0;
    while($savesrc =~ /$p/g) {$repcnt++}
    if($repcnt) {
	$savesrc =~ s/$p/$pat{$p}/g;
	($pa) = ($p =~ /(^[^\\]+)/);
	($pb) = ($pat{$p} =~ /(^[^\(]+)/);
	print "$basefile.c: Rename ($repcnt times) $pa to $pb\n";
	$new+=$repcnt;
    }
}

print NEWSRC $savesrc;

close NEWSRC;

foreach $n (sort (keys ( %subdone))) {
    ($na,$nb) = ($n =~ /([^_]+)_([^_]+)/);
    $new+=$subdone{$n};
    print "$basefile.c: Replace ($subdone{$n} times) $na with $nb\n";
}
print "$new lines replaced\n";
#close DBG;
