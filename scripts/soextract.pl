#!/usr/bin/perl -w
$id="";
$name="";

sub doprint() {
    $def =~ s/\\n/\n# /g;
    print "# $def";
    if($cc ne "") {
	$cc =~ s/;$//;
	$ =~ s/;/\n#   cc: /g;
	print "#   cc: $cc\n";
    }
    if($synonym ne "") {
	$synonym =~ s/;$//;
	$synonym =~ s/;/\n#   synonym: /g;
	print "#   synonym: $synonym\n";
    }
    if($subset ne "") {
	$subset =~ s/;$//;
	$subset =~ s/;/\n#   subset: /g;
	print "#   subset: $subset\n";
    }
    if($isa ne "") {
	$isa =~ s/;$//;
	$isa =~ s/;/\n#   isa: /g;
	print "#   isa: $isa\n";
    }
    if($altid ne "") {
	$altid =~ s/;$//;
	$altid =~ s/;/\n#   altid: /g;
	print "#   altid: $altid\n";
    }
    print "\n";
    print "$name $id\n";
    print "\n";
    $id=$name=$def=$cc=$synonym=$altid=$isa=$subset="";
}

while(<>) {
    if(/^id: (\S+)/) {
	doprint();
	$id = $1;
    }
    elsif(/^name: (\S+)/) {$name=$1;}
    elsif(/^alt_id: (\S+)/) {$altid=$1;}
    elsif(/^def: (.*)/) {$def=$1;}
    elsif(/^comment: (.*)/) {$cc=$1;}
    elsif(/^subset: ()/) {$ubset.="$1;"}
    elsif(/^synonym: ()/) {$synonym.="$1;"}
    elsif(/^is_a: (\S+)/) {$isa.="$1;"}
    elsif(/^[\[]Term[\]/) {next}
    elsif(/^$/) {next}
    else {print "Unknown line type: $_\n"}
}

doprint();
