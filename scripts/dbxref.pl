#!/usr/bin/perl -w

sub idsort{
    $cat{$a} cmp $cat{$b}
    or
    $a cmp $b
}

$isdata = 0;
$line=0;
$id = $cat = $name = $ref = $link = $server = $dburl = $note = "";
while (<>) {
    chomp;
    s/\s+$//;
    $line++;
    if($isdata){
	if(/^--------/) {$isdata = 0}
    }
    if(!$isdata){
	if(/^Abbrev: /) {$isdata = 1}
    }
    if(!$isdata) {next}
    # "$abbrev\|\| \|\| \|\| Swiss-Prot \|\| Name \|\| Citation \|\| "
    #	    "\|\| 
    if(/^$/) {
	$cat{$id} = $cat;
	$name{$id} = $name;
	$ref{$id} = $ref;
	$link{$id} = $link;
	$server{$id} = $server;
	$dburl{$id} = $dburl;
	$note{$id} = $note;
	$id = $cat = $name = $ref = $link = $server = $dburl = $note = "";
    }
    elsif(/^Abbrev: (.*)/) {$last="abbrev";$id=$1}
    elsif(/^Name  : (.*)/) {$last="name";$name=$1}
    elsif(/^Ref   : (.*)/) {
	$last="ref";
	$txt = $1;
	$txt =~ s/<\/A>//g;
	$txt =~ s/<A HREF[^>]+>//g;
	$ref=$txt;
    }
    elsif(/^LinkTp: (.*)/) {
	$last="link";
	$txt = $1;
	if($txt =~ /Explicit/) {
	    $txt =~ s/<A HREF=\"([^\"]+)\">Explicit<\/A>/$1/;
	}
	$link=$txt;
    }
    elsif(/^Server: (.*)/) {
	$last="server";
	$txt = $1;
	$txt =~ s/<\/A>//g;
	$txt =~ s/<A HREF[^>]+>//g;
	$server=$txt;
    }
    elsif(/^Db_URL: (.*)/) {
	$last="dburl";
	$txt = $1;
	$txt =~ s/<\/A>//g;
	$txt =~ s/<A HREF[^>]+>//g;
	if($txt =~ /^http:/){$dburl="$txt";}
	else {$dburl = "http://$txt"}
    }
    elsif(/^Note  : (.*)/) {$last="note";$note=$1}
    elsif(/^Cat   : (.*)/) {$last="cat";$cat=$1}
    elsif(/^(\S+) *:/) {print STDERR "$line: Unexpected tag '$1\n"}
    elsif(/^\s*(.+)/) {
	if($last eq "abbrev") {$id .= " $1"}
	if($last eq "name") {$name .= " $1"}
	if($last eq "ref") {$ref .= " $1"}
	if($last eq "link") {$link .= " $1"}
	if($last eq "server") {
	    $txt = $1;
	    $txt =~ s/<\/A>//g;
	    $txt =~ s/<A HREF[^>]+>//g;
	    $server .= " $txt";
	}
	if($last eq "dburl") {$dburl .= " $1"}
	if($last eq "cat") {$cat .= " $1"}
	if($last eq "note") {$note .= " $1"}
    }
}

$lastcat = "";
foreach $d (sort idsort (keys(%cat))) {
    if($cat{$d} ne $lastcat) {
	if($lastcat ne "") {print "|}\n";}
	$lastcat = $cat{$d};
	print "\n====Swiss-Prot $lastcat====\n\n";
	print "{| class=\"wikitable\" border=\"1\"\n";
	print "\|-\n";
	print "!DbName\n";
	print "!Primary\n";
	print "!Secondary\n";
	print "!UsedIn\n";
	print "!Name\n";
	print "!Citation\n";
	print "!Link\n";
	print "!Server\n";
	print "!URL\n";
    }
    print "\|-\n";
    print "|$d \|\| \|\| \|\| Swiss-Prot\n";
    print "\|$name{$d} \|\| $ref{$d}\n";
    print "\|$link{$d} \|\| $server{$d}\n";
    print "\|$dburl{$d} \|\| $note{$d}\n";
}

print "|}\n";

print "\n\n";

$lastcat = "";
$n = 0;
foreach $c (sort (values(%cat))) {
    if($c eq $lastcat) {$n++;next}
    $lastcat = $c;
    if($n) {print " ($n)\n"}
    print "$c";
    $n = 1;
}
print " ($n)\n";
