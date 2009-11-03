#!/usr/bin/perl -w

######################################################################
# 
# Name: createnewdocumentation
# 
# Description: 
# 	Asks if the user wishes to create and edit new documentation
#	for a program.
#	If so, the template file is copied and the user's favorite
#	editor is started.
#
# Args: 
# 	$thisprogram - the name of the program
#	$docdir - the location of the web pages
#	
#
# Returns:
#	1 if the document is created and edited
#	0 if no document is created
# 
######################################################################

open (VERS, "embossversion -full -auto|") || die "Cannot run embossversion";
while (<VERS>) {
#    if(/InstallDirectory: +(\S+)/) {$installtop = $1}
    if(/BaseDirectory: +(\S+)/) {$distribtop = $1}
}
close VERS;
my $doctop = "$ENV{HOME}/sfdoc";
my $thisprogram = $ARGV[0];
my $progdocdir = "$doctop/apps/cvs/emboss/apps/";
if ($#ARGV > 1) { $progdocdir =  "$doctop/apps/cvs/embassy/" . $ARGV[1]}

my $ans;
my $indexfile = "$progdocdir/index.html";

if (-e "$progdocdir/$thisprogram.html") {
    print "Documentation for $thisprogram already exists\n";
    exit();
}
# application's document is missing
    print "\n$progdocdir/$thisprogram.html =missing=\n";
    print "Create a web page for this program? (y/n) ";
####      $ans = <STDIN>;
    $ans = "skip";
    if ($ans =~ /^y/) {
        system("cp $docdir/template.html.save $progdocdir/$thisprogram.html");
        system "perl -p -i -e 's/ProgramNameToBeReplaced/$thisprogram/g;' $progdocdir/$thisprogram.html";
	chmod 0664, "$progdocdir/$thisprogram.html";
	if (defined $ENV{'EDITOR'} && $ENV{'EDITOR'} ne "") {
	    system("$ENV{'EDITOR'} $progdocdir/$thisprogram.html");
	    open (INDEX2, ">> $indexfile") || die "Cannot open $indexfile\n";
	    print INDEX2 "

<tr><td><a href=\"$thisprogram.html\">$thisprogram</a></td><td>INSTITUTE</td><td>
$progs{$thisprogram}
</td></tr>
";
	    close (INDEX2);
	    system("$ENV{'EDITOR'} $progdocdir/index.html");
	}
	else {
	    print "*********************************

YOU DO NOT HAVE AN EDITOR DEFINED
REMEMBER TO EDIT THESE FILES:
 $progdocdir/$thisprogram.html
 $indexfile\n\n\n";
	}
    }
