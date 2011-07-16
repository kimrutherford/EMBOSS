#!/usr/bin/perl -w

$text = "";
while (<>) {
    s/<\/[Tt][DdRrHh]>//gosm;
    s/<\/?[Tt][Aa][Bb][Ll][Ee][^>]*>//gosm;
    $text .= $_;
}
@rows = split(/<[Tt][Rr][^>]*>\s*<[Tt][DdHh][^>]*>/osm,$text);

print "{| style=\"background:#ccccff\" border=\"2\"\n";
$i = 0;
foreach $r(@rows) {
    $i++;
    $r =~ s/\n//gosm;
    @items = split(/<[Tt][DdHh][^>]*>/,$r);
    $j = 0;

    $empty=0;
    $caption=0;
    $header=0;
    if($#items < 0){next}
    if($#items == 0){
	if($items[0] =~ /\S+ [\(][^\)]+[\)] \S+$/){$caption=1}
	elsif($items[0] =~ /Associated qualifiers$/){$caption=1}
	elsif($items[0] =~ /General qualifiers$/){$caption=1}
	elsif($items[0] =~ /\S+ associated \S+ qualifiers$/){$caption=1}
	elsif($items[0] =~ /^[\(]none[\)]$/){$empty=1}
	else {next}
    }
    if($r =~ /<[Tt][Hh]/){$header=1}

    print "|-\n";
    foreach $it (@items) {
	$it =~ s/<[Tt][Rr][^>]*>//gosm;
	$it =~ s/^\s+//gosm;
	$it =~ s/\n$//gosm;
	$it =~ s/^[-]/ -/gosm;
	$j++;
	if($caption) {print "!style=\"background:#ffffcc\" colspan=\"5\"|$it\n"}
	elsif($empty) {print "!colspan=\"5\" align=\"left\"|$it\n"}
	elsif($header) {print "!$it\n"}
	else {print "|$it\n"}
    }
}
print "|}\n";

