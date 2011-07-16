#!/usr/bin/perl -w

# Makes the apps/index.html web page
# Creates the command lines include files: .ione, .ihelp, .itable
# Warns of missing documentation pages
# Updates the doc/programs/{text,html} files in the CVS tree

###############################################################################
#
# To be done
# ==========
#
# programs/html version to have separate embassy directories
# emboss_doc/master with inc subdirectory for templates (or programs/master)
# emboss_doc/html and emboss_doc/text for the outputs
# to be installed as /emboss/apps and /embassy/*/
# and on sourceforge as /apps/release/N.N/emboss/apps etc. or /apps/cvs/
#
# Use these as the master copy to create the ~/sfdoc versions
#
# need to update included html for emboss and for embassy packages
#
# check emboss.cvs for files that need to be added - do not assume
# the cvs add will be run
#
# Why did domainatrix files get updated as zero length when the tests failed?
#
# Skeleton version for sourceforge apps directory (for Jemboss)
#
# embassy docs - /apps/ redirect (checked)
#     and embassy/*/ main pages (not yet checked?)
#
# plotorf copies plotorf.gif 3 times in 2 tests
#
# need to check new docs are created when new applications appear with
# master docs
#
#
# Changes needed for SourceForge version
# ======================================
# need to be better at identifying and fixing embassy missing documentation
# Need to check for EFUNC and EDATA HTML file updates with main server
#     including the index.html pages
###############################################################################

##################################################################
# 
# Definitions and global variables
# 
##################################################################


my %progdone = ();  # key=program name,
                    # value = set to 1 if documentation exists
my %progdir = ();   # key=program name,
                    # value = EMBASSY name if EMBASSY program

my $embassy;	    # name of current EMBASSY directory being done,
                    # "" if not an EMBASSY program
my $docdir;	    # name of directory holding the set of EMBASSY programs
                    # being done for local distribution

# read in from the EMBOSS application 'wossname'
# group names, application name and which application is in which groups
my %grpnames;	    # hash of key=lowercase group name,
                    # value = uppercase group description
my %progs;	    # hash of key=program name, value = description
my %groups;	    # hash of key=lowercase group name, value = program names
my %embassyprogs;   # hash of embassy package names for each program
my %missingdoc;     # hash of missing program documentation.
                    # Value is the directory it should be in

# where the URL for the html pages is
my $url = "http://emboss.sourceforge.net/apps/cvs";
my $urlembassy = "http://emboss.sourceforge.net/embassy";

# where the original distribution lives
my $distribtop = "./";

# where the installed package lives
my $installtop = "./";

open (VERS, "embossversion -full -auto|") || die "Cannot run embossversion";
while (<VERS>) {
    if(/InstallDirectory: +(\S+)/) {
	$installtop = $1;
	$installtop =~ s/\/$//;
    }
    if(/BaseDirectory: +(\S+)/) {
	$distribtop = $1;
	$distribtop =~ s/\/$//;
    }
}
close VERS;

# where the CVS tree program doc pages are
my $cvsdoc = "$distribtop/doc/programs";
my $cvsedoc = "$distribtop/embassy";

# where the CVS tree scripts are
my $scripts = "$distribtop/scripts";
 
# where the web pages live
my $doctop = "$cvsdoc/master/emboss/apps";
my $sfdoctop = "$ENV{HOME}/sfdoc/apps/cvs";

my @embassylist = ("appendixd",
		   "cbstools",
		   "clustalomega",
		   "domainatrix",
		   "domalign",
		   "domsearch",
		   "emnu",
		   "esim4",
		   "hmmer",
		   "hmmernew",
		   "iprscan",
		   "meme",
		   "memenew",
		   "mira",
		   "mse",
		   "myemboss", # we avoid documenting these examples
		   "myembossdemo", # we avoid documenting these examples
		   "phylip",
		   "phylipnew",
		   "signature",
		   "structure",
		   "topo",
		   "vienna",	# old vienna
		   "vienna2",
		   );

# the directories containing web pages - EMBOSS and EMBASSY
my @doclist = (
	       "$doctop"
	       );

# Filenames for cvs add and commit commands.
# These hold a list of the names of files to be added/committed
# to the text or html documentation directories
# This is done at the end of the script
my $cvsdochtmladd = '';
my $cvsdochtmlcommit = '';
my $cvsdoctextadd = '';
my $cvsdoctextcommit = '';
my $badlynx = 0;
my $badsrc = 0;

######################################################################
######################################################################
# 
# SUBROUTINES
# 
######################################################################
######################################################################

######################################################################
# 
# Name: usage
# 
# Description:
#       Reports command line options
#
######################################################################

sub usage () {
  print STDERR "Usage:\n";
  print STDERR "  autodoc.pl -create -embassy=package [appname]\n";
  print STDERR "\n";
  print STDERR "Default is to produce docuemntation for all applications\n";
  exit();
}
######################################################################
# 
# Name: htmlsource
# 
# Description: 
#	resolves #include directives from template
#       to make a simple html file
# 
# Args: 
# 	$tfile - HTML template filename
# 	$edir  - EMBASSY or emboss application dir for includes
# 
# Warning: 
#	uses temporary filename 'x.x'
# 
######################################################################
sub htmlsource ( $$ ) {
    my($tfile, $edir) = @_;
    @giffiles = ();
    open (X, ">x.x") || die "Cannot upen temporary file x.x";

    open (H, "$edir/$tfile") ||
	die "Cannot open HTML template file '$edir/$tfile'";

    while (<H>) {
	if(/^\s*Function\s*$/) {
	    print X "Wiki\n";
	    print X "</H2>\n";
	    print X "\n";
	    print X "The master copies of EMBOSS documentation are available\n";
	    print X "at <a href=\"http://emboss.open-bio.org/wiki/Appdocs\">\n";
	    print X "http://emboss.open-bio.org/wiki/Appdocs</a>\n";
	    print X "on the EMBOSS Wiki.\n";
	    print X "\n";
	    print X "<p>\n";
	    print X "Please help by correcting and extending the Wiki pages.\n";
	    print X "\n";
	    print X "<H2>\n";
	}
	if (/[<][\!][-][-][\#]include file=\"([^\"]+)\" +[-][-][>]/) {
	    $ifile = $1;
	    if(-e "$edir/$ifile") {
		open (I, "$edir/$ifile") ||
		    die "Cannot open include file '$edir/$ifile'";
		while (<I>) {
		    print X;
		    if(/<p><img src=\"([^\"]+)\" alt=\"\[\S+ results\]\">/){
			push @giffiles, $1;
		    }
		}
		close I;
	    }
	    else {
		print STDERR "Cannot open include file '$edir/$ifile'\n";
		print X "[an error has occurred processing this directive]\n";
	    }
	    next;
	}
	elsif (/[<][\!][-][-][\#]include virtual=\"\/apps\/([^\"]+)\" +[-][-][>]/) {
	    $ifile = $1;
	    if(-e "$cvsdoc/master/emboss/apps/$ifile") {
		open (I, "$cvsdoc/master/emboss/apps/$ifile") ||
		    die "Cannot open include file '$cvsdoc/master/emboss/apps/$ifile'";
		while (<I>) {print X}
		close I;
	    }
	    else {
		print STDERR "Cannot open virtual file '$cvsdoc/master/emboss/apps/$ifile'\n";
		print X "[an error has occurred processing this directive]\n";
	    }
	    next;
	}
	else {print X}
    }
    close X;
    close H;
    return 1;
}

######################################################################
# 
# Name: cleantext
# 
# Description: 
#	removes unwanted acdtable output from text documentation
#	makes sure seealso output is reasonably spaced
# 
# Args: 
# 	$afile - first filename
#
# Returns:
# Warning: 
#	uses temporary filename 'z.z'
# 
######################################################################
sub cleantext ( $ ) {
    my $seealso = 0;
    my ($afile) = @_;
    if(-e $afile) {
	open (X, $afile);
	open (Z, ">z.z");
	my $acdtable = 0;
	while (<X>) {
	    if(/^See also$/) {
		    $seealso = 1;
		}
	    if(/^Author[\(s\)]$/) {
		    $seealso = 0;
		}
	    if($acdtable) {
		if(/^Input file format$/) {
		    $acdtable = 0;
		}
		if(/^Output file format$/) {
		    $acdtable = 0;
		}

		if(/^Data files$/) {
		    $acdtable = 0;
		}

	    }
	    else {
		if(/^\s+Qualifier Type Description Allowed values Default/) {
		    $acdtable = 1;
		}
	    }
	    if($seealso) {
		if(/   Program name Description/) {
		    $seealso = 2;
		    s/Program name Description/Program name     Description/;
		}
		else {
		    if(/^(\s+)(\S+) ([A-Z])/) {
			$name = sprintf "%-16s", $2;
			s/^(\s+)(\S+) ([A-Z])/$1$name $3/;
		    }
		    else {
			s/^  /                   /;
		    }
		}
		if(/^$/ && $seealso == 2) {$seealso = 0}
	    }
	
	    if(!$acdtable) { print Z }
	}
    }
    close X;
    close Z;
    open (X, ">$afile");
    open (Z, "z.z");
    while (<Z>) {
	print X;
    }
    close X;
    close Z;

}

######################################################################
# 
# Name: filediff
# 
# Description: 
#	runs diff on two files and returns 1 if they differ
# 
# Args: 
# 	$afile - first filename
# 	$bfile - second filename
#
# Returns:
#       "replaced" file differed and was replaced
#       "created"  file did not exist and was created
#       "same"     files were identical in size and content
# Warning: 
#	uses temporary filename 'z.z'
# 
######################################################################
sub filediff ( $$ ) {
    my ($afile, $bfile) = @_;
    my $s = 0;
    my $action = "";

    if(-e $afile) {
	if ((-s $afile) !=  (-s $bfile)) {
	    print LOG "$afile " . (-s $afile) . ", $bfile " . (-s $bfile) . "\n";
	}
	system ("diff -b $afile $bfile > z.z");
	$s = (-s "z.z");
	if ($s) {
	    print LOG "$afile ** differences ** size:$s\n";
	    open (DIF, "z.z") || die "cannot open diff output file";
	    while (<DIF>) { print LOG "> $_";}
	    close DIF;
	    $action = "replaced";
	}
	unlink "z.z";
    }
    else {
	$s = (-s $bfile);
	$action = "created";
	$cvsdochtmladd .= " $afile";
    }

    if($s) {
	system "cp $bfile $afile";
	chmod 0664, "$afile";
	unlink "$bfile";
	print "$afile *$action*\n";
	print LOG "$afile *$action*\n";
	$cvsdochtmlcommit .= " $afile";
    }
    return;
}


######################################################################
# 
# Name: header1
# 
# Description: 
#	prints out the first part of the HTML header text (before title)
# 
# Args: 
# 	*OUT - filehandle to print to
# 
# 
######################################################################
sub header1 (*) {
    local (*OUT) = @_;

    print OUT "
<HTML>

<HEAD>
  <TITLE>
  EMBOSS
  </TITLE>
</HEAD>
<BODY BGCOLOR=\"#FFFFFF\" text=\"#000000\">


<!--#include file=\"header1.inc\" -->
";

}


######################################################################
# 
# Name: header2
# 
# Description: 
#	prints out the second part of the HTML header text (after title)
# 
# Args: 
# 	*OUT - filehandle to print to
# 
# 
######################################################################
sub header2 (*) {
    local (*OUT) = @_;

    print OUT "
<!--#include file=\"header2.inc\" -->

<!--END OF HEADER-->




";
}



######################################################################
# 
# Name: footer
# 
# Description: 
#	ends an HTML page
# 
# Args: 
# 	*OUT - filehandle to print to
# 
# 
######################################################################
sub footer (*) {
    local (*OUT) = @_;
    print OUT "

</BODY>
</HTML>
";
}


######################################################################
# 
# Name: indexheader
# 
# Description: 
# 	prints out the header and text at the start of the file
#	containing the table of applications.
#
# Args: 
#	*OUT - filehandle to print to 
# 
# 
######################################################################
sub indexheader (*) {
    local (*OUT) = @_;

    print OUT "
<HTML>

<HEAD>
  <TITLE>
  EMBOSS: The Applications (programs)
  </TITLE>
</HEAD>
<BODY BGCOLOR=\"#FFFFFF\" text=\"#000000\">



<table align=center border=0 cellspacing=0 cellpadding=0>
<tr><td valign=top>
<A HREF=\"http://emboss.sourceforge.net/\" ONMOUSEOVER=\"self.status='Go to the EMBOSS home page';return true\">
<img border=0 src=\"/emboss_icon.jpg\" alt=\"\" width=150 height=48></a>
</td>
<td align=left valign=middle>
<b><font size=\"+6\"> 
The Applications (programs)
</font></b>
</td></tr>
</table>
<br>&nbsp;
<p>




The programs are listed in alphabetical order, Look at the individual
applications or go to the 
<a href=\"groups.html\">GROUPS</a>
page to search by category. 
<p>

<a href=\"../../embassy/index.html\">EMBASSY applications</a>
are described in separate documentation for each package.

<h3><A NAME=\"current\">Applications</A> in the <a
href=\"ftp://emboss.open-bio.org/pub/EMBOSS/\">current release</a></h3>

<table border cellpadding=4 bgcolor=\"#FFFFF0\">

<tr>
<th>Program name</th>
<th>Description</th>
</tr>

";

}

######################################################################
# 
# Name: indexfooter
# 
# Description: 
# 	print out the end of the table for the file
#       containing the table of applications. 
#
# Args: 
# 	*OUT - filehandle to print to
# 
# 
######################################################################
sub indexfooter (*) {
    local (*OUT) = @_;

    print OUT "

</table>




</BODY>
</HTML>
";
}

######################################################################
# 
# Name: getprogramnames
# 
# Description: 
# 	Runs wossname to get the EMBOSS programs on the path
#	together with their groups
#
# Args: 
#   *** These are all global variables ***
# 	%grpnames - hash of key=lowercase group name,
#                        value = uppercase group description
# 	%progs - hash of key=program name, value = program description
#	%groups - hash of key=lowercase group name, value = program names
# 
######################################################################

sub getprogramnames ( ) {

    my $prog;	# program name
    my $capgrp;	# uppcase group name
    my $grp;	# lowercase group name

    open (PROGS, "wossname -noembassy -auto |") ||
	die "Cannot run wossname";
    while ($prog = <PROGS>) {
	if ($prog =~ /^\s*$/) {	# ignore blank lines
	    next;
	} elsif ($prog =~ /^([A-Z0-9 ]+)$/) {	# uppcase means a group name
	    $capgrp = $1;			
	    $grp = lc($capgrp);
	    $grp =~ s/ +/_/g;		# lowercase one-word group_name
	    $grpnames{$grp} = $capgrp;
#      print "Group $grp = $capgrp\n";
	} elsif ($prog =~ /^(\S+) +(.*)/) {
	    $progs{$1} = $2;		
	    $groups{$grp} .= "$1 ";
#      print "Program in $grp = $1\n";
	}
    }
    close PROGS;

    foreach $e(@embassylist) {
	open (PROGS, "wossname -showembassy $e -auto |") ||
	    die "Cannot run wossname";
	while ($prog = <PROGS>) {
	    if ($prog =~ /^\s*$/) {	# ignore blank lines
		next;
	    } elsif ($prog =~ /^([A-Z0-9 ]+)$/) {  # uppcase means a group name
		$capgrp = $1;			
		$grp = lc($capgrp);
		$grp =~ s/ +/_/g;		# lowercase one-word group_name
		$grpnames{$grp} = $capgrp;
#      print "Group $grp = $capgrp\n";
	    } elsif ($prog =~ /^(\S+) +(.*)/) {
		$progs{$1} = $2;		
		$groups{$grp} .= "$1 ";
		$embassyprogs{$1} = $e;
#      print "Program in $grp = $1\n";
	    }
	}
	close PROGS;
    }

}


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

sub createnewdocumentation ( $$$ ) {

    my ($thisprogram, $progdocdir, $sfprogdocdir) = @_;
    my $ans;
    my $indexfile = "$progdocdir/index.html";

# application's document is missing
    print LOG "createnewdocumentation missing $progdocdir/$thisprogram.html\n";
    print "\n$progdocdir/$thisprogram.html =missing=\n";
    print STDERR "\n$progdocdir/$thisprogram.html =missing=\n";
    if ($doccreate) {
	print STDERR "Create a web page for this program? (y/n) ";
	$ans = <STDIN>;
    }
    else {
	$ans = "skip";
	print STDERR "Create later - run with create on commandline\n";
    }
    if ($ans =~ /^y/) {
        system("cp $progdocdir/template.html.save $progdocdir/$thisprogram.html");
        system "perl -p -i -e 's/ProgramNameToBeReplaced/$thisprogram/g;' $progdocdir/$thisprogram.html";
	chmod 0664, "$progdocdir/$thisprogram.html";
	if (defined $ENV{'EDITOR'} && $ENV{'EDITOR'} ne "") {
	    print STDERR "Generated $thisprogram.html:
	    Fill in the description section
	    Describe the input and output
	    Add notes, references
";
	    system("$ENV{'EDITOR'} $progdocdir/$thisprogram.html");
	    open (INDEX2, ">> $indexfile") || die "Cannot open $indexfile\n";
	    print INDEX2 "

<tr><td><a href=\"$thisprogram.html\">$thisprogram</a></td><td>
$progs{$thisprogram}
</td></tr>

";
	    close (INDEX2);
	    print STDERR "Edit $progdocdir/index.html:
	    Look for $thisprogram line (at end)
	    Move $thisprogram line to be in alphabetic order
";
	    system("$ENV{'EDITOR'} $progdocdir/index.html");
	}
	else {
	    print "*********************************

YOU DO NOT HAVE AN EDITOR DEFINED
REMEMBER TO EDIT THESE FILES:
 $progdocdir/$thisprogram.html
 $indexfile\n\n\n";
	}
        return 1;
    }
    else {
	$missingdoc{$thisprogram} = $progdocdir;
	return 0;
    }
}

######################################################################
# 
# Name: createnewdocumentationembassy
# 
# Description: 
# 	Asks if the user wishes to create and edit new documentation
#	for an embassy program.
#	If so, the top level redirect is created, and then 
#       the embassy template file is copied and the user's favorite
#	editor is started.
#
# Args: 
# 	$thisprogram - the name of the program
#	$progdocdir - the location of the web pages
#	
#
# Returns:
#	1 if the document is created and edited
#	0 if no document is created
# 
######################################################################

sub createnewdocumentationembassy ( $$$ ) {

    my ($thisprogram, $progdocdir, $sfprogdocdir) = @_;
    my $ans;
    my $indexfile = "$progdocdir/index.html";

# application's document is missing
    print LOG "createnewdocumentationembassy missing $progdocdir/$thisprogram.html\n";
    print "\n$progdocdir/$thisprogram.html =missing=\n";
    print STDERR "\n$progdocdir/$thisprogram.html =missing=\n";
    print STDERR "Create a web page for this program? (y/n) ";
    if ($doccreate) {
	print STDERR "Create a web page for this program? (y/n) ";
	$ans = <STDIN>;
    }
    else {
	$ans = "skip";
	print STDERR "Create later - run with create on commandline\n";
    }
    if ($ans =~ /^y/) {
        system("cp $progdocdir/template.html.save $progdocdir/$thisprogram.html");
        system "perl -p -i -e 's/ProgramNameToBeReplaced/$thisprogram/g;' $progdocdir/$thisprogram.html";
	chmod 0664, "$progdocdir/$thisprogram.html";
	if (defined $ENV{'EDITOR'} && $ENV{'EDITOR'} ne "") {
	    print STDERR "Generated $thisprogram.html:
	    Fill in the description section
	    Describe the input and output
	    Add notes, references
";
	    system("$ENV{'EDITOR'} $progdocdir/$thisprogram.html");
	    open (INDEX2, ">> $indexfile") || die "Cannot open $indexfile\n";
	    print INDEX2 "

<tr>
<td><a href=\"$thisprogram.html\">$thisprogram</a></td>
<td>
$progs{$thisprogram}
</td>
</tr>
";
	    close (INDEX2);
	    print STDERR "Edit $progdocdir/index.html:
	    Look for $thisprogram line (at end)
";
	    system("$ENV{'EDITOR'} $progdocdir/index.html");
	}
	else {
	    print "*********************************

YOU DO NOT HAVE AN EDITOR DEFINED
REMEMBER TO EDIT THESE FILES:
 $progdocdir/$thisprogram.html
 $indexfile\n\n\n";
	}
        return 1;
    }
    else {
	$missingdoc{$thisprogram} = $progdocdir;
	return 0;
    }
}

######################################################################
# 
# Name: checkincludefile
# 
# Description: 
# 	This checks for the existence of one of several types of include file
#	If the file doesn't exist, it is created from the 'x.x' file.
#	If the file exists, a new one is created and checked to see if it
#	is different to the old one, if different, it is updated
#	This assumes that the file 'x.x' has just been set up with the
#	new include file contents.
#
# Args: 
# 	$thisprogram - the name of the program
#	$docdir - the location of the web pages
#	$ext - extension of the include file
# 
# 
######################################################################

sub checkincludefile ( $$$ ) {

    my ($thisprogram, $docdir, $ext) = @_;


# check to see if the include file has changed
    filediff ("$docdir/inc/$thisprogram.$ext", "x.x");
}



######################################################################
# 
# Name: checkhistoryfile
# 
# Description: 
# 	This checks for the existence of a history file.
#       If no file is found, creates a blank file..
#
# Args: 
# 	$thisprogram - the name of the program
#	$docdir - the location of the web pages
# 
# 
######################################################################

sub checkhistoryfile ( $$ ) {

    my ($thisprogram, $docdir) = @_;

    my $histfile = "$docdir/inc/$thisprogram.history";

# check to see if the include file has changed
    if (! -e "$histfile") {
	open (HIST, ">$histfile") || die "Cannot open distribution $histfile";
	print HIST "\n";
	close HIST;
    }
}



######################################################################
# 
# Name: checkcommentfile
# 
# Description: 
# 	This checks for the existence of a coment file.
#       If no file is found, creates a file with the test 'None'..
#
# Args: 
# 	$thisprogram - the name of the program
#	$docdir - the location of the web pages
# 
# 
######################################################################

sub checkcommentfile ( $$ ) {

    my ($thisprogram, $docdir) = @_;

    my $commentfile = "$docdir/inc/$thisprogram.comment";

# check to see if the include file has changed
    if (! -e "$commentfile") {
	open (HIST, ">$commentfile") || die "Cannot open distribution $commentfile";
	print HIST "None\n";
	close HIST;
    }
}



##################################################################
##################################################################
#
# Main routine
#
##################################################################
##################################################################

foreach $x (@embassylist) {
    push @doclist, "$distribtop/embassy/$x";
}

$doccreate = "";

foreach $test (@ARGV) {
    if ($test =~ /^-(.*)/) {
	$opt=$1;
	if ($opt eq "create") {$doccreate = "Y"}
	elsif ($opt =~ /embassy[=](.*)/) {
	    $singlepackage=$1;
	    ###print STDERR "Singlepackage '$singlepackage'\n";
	}
	else {print STDERR "+++ unknown option '$opt'\n";usage()}
    }
    elsif(!defined($singleapp)) {
	$singleapp = $test;
	###print STDERR "Singleapp '$singleapp'\n";
    }
    else {print STDERR "+++ only one application name allowed\n;usage()"}
}

$cvscommit = $doccreate;
@giffiles=();
open(LOGEX, ">makeexample.log") || die "Cannot open makeexample.log";
close (LOGEX);

open(LOG, ">autodoc.log") || die "Cannot open autodoc.log";

# get the program and group names
getprogramnames();

# open the file 'i.i'
# This will be copied to the file 'index.html' at the end of the script
# if all goes well.
# 'index.html' is the file we will be putting in the distribution.
if(!defined($singlepackage) && !defined($singleapp)) {
    open (INDEX, "> i.i") || die "Cannot open i.i\n";
    indexheader(INDEX);
}


# main loop
# look at all directories in our documentation list
foreach $docdir (@doclist) {
    if ($docdir =~ /embassy\/(.*)/) {
	$embassy = $1;
	$sfdocdir = "$sfdoctop/embassy/$embassy/";
	if(defined($singlepackage) && $embassy ne $singlepackage) {next}
	print LOG "embassy $embassy\n";
	$eindex =  "$cvsedoc/$embassy/emboss_doc/master/inc/apps.itable";
	if(!defined($singleapp)) {
	    open (EINDEX, ">e.e") || die "Cannot open 'e.e'";
#	embassyindexheader(EINDEX, $embassy);
	    print EINDEX "<h3><A NAME=\"$embassy\">Applications</A> in the <a
href=\"ftp://emboss.open-bio.org/pub/EMBOSS/\">current $embassy release</a></h3>

<table border cellpadding=4 bgcolor=\"#FFFFF0\">

<tr>
<th>Program name</th>
<th>Description</th>
</tr>

";
	}
    }
    else {			# main EMBOSS applications
	$sfdocdir = "$sfdoctop/emboss/apps/";
	if(defined($singlepackage)) {next}
	$embassy = "";
	$eindex = "";
    }

# look at all applications alphabetically
    foreach $thisprogram (sort (keys %progs)) {
	if(defined($singleapp) && $thisprogram ne $singleapp) {next}
	if ($embassy eq "") {
	    if (defined($embassyprogs{$thisprogram})) {next}
	}
	elsif (!defined($embassyprogs{$thisprogram})) {next}
	else {
	    if ($embassyprogs{$thisprogram} ne $embassy) {next}
	}
	print "\n$thisprogram '$progs{$thisprogram}'\n";
	print LOG "\n$thisprogram '$progs{$thisprogram}'\n";
	
# if this is a non-EMBASSY program then add it to the index.html file
	if (!defined($embassyprogs{$thisprogram})) {
	    $progdocdir = $docdir;
	    $sfprogdocdir = $sfdocdir;
	    if(!defined($singleapp)) {
		print INDEX
"<tr>
<td><a href=\"$thisprogram.html\">$thisprogram</a></td>
<td>
$progs{$thisprogram}
</td>
</tr>\n";
	    }
	}
	else {
# update the embassy index here -
# or just use the %embassyprogs array to make a list?
	    $progdocdir = "$cvsedoc/$embassyprogs{$thisprogram}/emboss_doc/master";
	    $sfprogdocdir = "$sfdoctop/embassy/$embassyprogs{$thisprogram}";
	    if(!defined($singleapp)) {
		print EINDEX
"<tr>
<td><a href=\"$thisprogram.html\">$thisprogram</a></td>
<td>
$progs{$thisprogram}
</td>
</tr>\n";
	    }
	}

# check the documentation for this file exists and is not a symbolic link 
# if this is an EMBASSY document, note which EMBASSY directory it is in
	if (!defined($embassyprogs{$thisprogram})) {
	    if (-e "$cvsdoc/master/emboss/apps/$thisprogram.html") {
###	  print "$progdocdir/$thisprogram.html found\n";
		if (-e "$sfprogdocdir/$thisprogram.html") {
		    system("diff -b $cvsdoc/html/$thisprogram.html $sfprogdocdir/$thisprogram.html > z.z");
		    $s = (-s "z.z");
		    if ($s) {
			print LOG "** $sfprogdocdir/$thisprogram.html differences ** size:$s ($cvsdoc/html/$thisprogram.html)\n";
			system "cp  $cvsdoc/html/$thisprogram.html $sfprogdocdir/$thisprogram.html";
			chmod 0664, "$sfprogdocdir/$thisprogram.html";
		    }
		}
		else {
			print LOG "** $sfprogdocdir/$thisprogram.html copied\n";
			system "cp  $cvsdoc/html/$thisprogram.html $sfprogdocdir/$thisprogram.html";
			chmod 0664, "$sfprogdocdir/$thisprogram.html";
		}
	    }
	    else {
# optionally create the documentation and edit it,
#	  or abort and do the next program
###	  print "$progdocdir/$thisprogram.html missing - EMBOSS\n";
		if (!createnewdocumentation($thisprogram, $progdocdir, $sfprogdocdir)) {next;}
	    }
	    $progdone{$thisprogram} = 1;
	}
	else {
	    $progdir{$thisprogram} = $embassyprogs{$thisprogram};
	    $edir = "$cvsedoc/$progdir{$thisprogram}/emboss_doc/html";
	    $sfedir = "$sfdoctop/embassy/$progdir{$thisprogram}";
	    if(-e "$edir/$thisprogram.html") {
###	  print "$edir/$thisprogram.html found\n";
		if (-e "$sfedir/$thisprogram.html") {
		    system("diff -b $edir/$thisprogram.html $sfedir/$thisprogram.html > z.z");
		    $s = (-s "z.z");
		    if ($s) {
			print LOG "** $sfedir/$thisprogram.html differences ** size:$s ($edir/$thisprogram.html)\n";
			system "cp  $edir/$thisprogram.html $sfedir/$thisprogram.html";
			chmod 0664, "$sfedir/$thisprogram.html";
		    }
		}
		else {
			print LOG "** $edir/$thisprogram.html copied\n";
			system "cp  $edir/$thisprogram.html $sfedir/$thisprogram.html";
			chmod 0664, "$sfedir/$thisprogram.html";
		}
	    }
	    else {
###	  print "$progdocdir/$thisprogram.html missing - EMBASSY $embassyprogs{$thisprogram}\n";
		print STDERR "Missing embassy documentation $edir/$thisprogram.html\n";
		print STDERR "docdir: $docdir\n";
		print STDERR "progdocdir: $progdocdir\n";
		print STDERR "embassyprogs: $embassyprogs{$thisprogram}\n";
		if (!createnewdocumentationembassy($thisprogram, $progdocdir, $sfedir)) {next;}
	    }
	    $progdone{$thisprogram} = 1;
	}

# note whether we now have a documentation file or not
	if (!$progdone{$thisprogram}) {
	    print "++ Missing main docs: $thisprogram ++ \n";
	}

# check on the existence of the one-line description include file
# for this application
	open(FH, ">x.x") || die "Can't open file x.x\n";
	print FH $progs{$thisprogram};
	close(FH);
	checkincludefile($thisprogram, $progdocdir, 'ione');

# check on the existence of the '-help' include file for this application
	system "acdc $thisprogram -help -verbose 2> x.x";
	checkincludefile($thisprogram, $progdocdir, 'ihelp');


# check to see if the command table include file exists
	system "acdtable $thisprogram -verbose 2> x.x";
	checkincludefile($thisprogram, $progdocdir, 'itable');


# check to see if the comment file exists
	checkcommentfile($thisprogram, $progdocdir);

# check to see if the comment file exists
	checkhistoryfile($thisprogram, $progdocdir);


# check on the existence of the 'seealso' include file for this application
# if this is not an EMBASSY program, then we don't want to include EMBASSY
# programs in the SEE ALSO file
	if (!defined($embassyprogs{$thisprogram})) {
	    system "seealso $thisprogram -auto -noembassy -html -out x.x";
	    open (X, "x.x") || die "Cannot open x.x";
	    $text = "";
	    while (<X>) {
		if (/\"([^\/.]+)\.html/) {
		    $app = $1;
		    if (defined($embassyprogs{$app})) {
			$apppack = $embassyprogs{$app};
			s/\"([^\/.]+)\.html/\"..\/..\/embassy\/$apppack\/$app.html/;
		    }
		}
		$text .= $_;
	    }
	    close (X);
	    open (X, ">x.x") || die "Cannot open x.x for output";
	    print X $text;
	    close X;
	}
	else {
	    system "seealso $thisprogram -auto -html -out x.x";
	    open (X, "x.x") || die "Cannot open x.x";
	    $text = "";
	    while (<X>) {
		if (/\"([^\/.]+)\.html/) {
		    $app = $1;
		    if (defined($embassyprogs{$app})) {
			$apppack = $embassyprogs{$app};
			if ($apppack ne $embassyprogs{$thisprogram}) {
			    s/\"([^\/.]+)\.html/\"..\/..\/$apppack\/$app.html/;
			}
		    }
		    else {
			s/\"([^\/.]+)\.html/\"..\/..\/apps\/$app.html/;
		    }
		}
		$text .= $_;
	    }
	    close (X);
	    open (X, ">x.x") || die "Cannot open x.x for output";
	    print X $text;
	    close X;
	}
	system "perl -p -i -e 's/SEE ALSO/See also/g;' x.x";
	checkincludefile($thisprogram, $progdocdir, 'isee');

# create the '.usage', '.input' and '.output' include files
	if ($embassy eq "") {
	    $docurl = $url;
	    $mkstatus = system "$scripts/makeexample.pl $thisprogram";
	    $docmaster = "$cvsdoc/master/emboss/apps";
	    $dochtml   = "$cvsdoc/html";
	    $doctext   = "$cvsdoc/text";
	}
	else {
	    $docurl = "$urlembassy/$embassy";
	    $mkstatus = system "$scripts/makeexample.pl $thisprogram $embassy";
	    $docmaster = "$cvsedoc/$embassy/emboss_doc/master";
	    $dochtml   = "$cvsedoc/$embassy/emboss_doc/html";
	    $doctext   = "$cvsedoc/$embassy/emboss_doc/text";
	}
	if ($mkstatus) {
	    print STDERR "$thisprogram: makeexample.pl status $mkstatus\n";
	}

# check to see if the CVS tree copy of the html documentation needs updating
# check to see if the html file has changed
	$status = htmlsource("$thisprogram.html","$docmaster");
#	    $status = system "lynx -source $docurl/$thisprogram.html > x.x";
	if (!$status) {
	    $badsrc++;
	    print "htmlsource error $status $docmaster/$thisprogram.html";
	}
	else {
	    if($#giffiles >= 0) {
		foreach $gf (@giffiles) {
		    system("cp $docmaster/$gf g.g");
		    filediff("$dochtml/$gf", "g.g");
		    system("cp $docmaster/$gf g.g");
		    filediff("$sfprogdocdir/$gf", "g.g");
		}
	    }
# edit the HTML file
# change ../emboss_icon.jpg and ../index.html to current directory
	    system "perl -p -i -e 's#\.\.\/index.html#index.html#g;' x.x";
	    system "perl -p -i -e 's#/images/emboss_icon.jpg#emboss_icon.jpg#g;' x.x";
	    $diff = filediff ("$dochtml/$thisprogram.html", "x.x");
	}

# check to see if the CVS tree copy of the text documentation needs updating
# check to see if the text has changed
	$status = system "lynx -dump -nolist $dochtml/$thisprogram.html > x.x";
	if ($status) {
	    $badlynx++;
	    print "lynx error $status $dochtml/$thisprogram.html";
	}
	else {
	    cleantext("x.x");
	    filediff ("$doctext/$thisprogram.txt", "x.x");
	}
    }
    if($embassy ne "") {
	if(!defined($singleapp)) {
	    print EINDEX "</table>\n";
	    close EINDEX;
# check to see if the index.html file has changed
	    $diff = filediff ("$eindex", "e.e");

	    $status = htmlsource("index.html", "$docmaster");
	    if (!$status) {
		$badsrc++;
		print "htmlsource error $status $docmaster/index.html";
	    }
	    else {
# change ../emboss_icon.jpg and ../index.html to current directory
		system "perl -p -i -e 's#\.\.\/index.html#index.html#g;' x.x";
		system "perl -p -i -e 's#/images/emboss_icon.jpg#emboss_icon.jpg#g;' x.x";
		filediff ("$dochtml/index.html", "x.x");
	    }
	}
    }
}
# main loop over all directories completed.

# end the index.html file
if(defined($singleapp)) {exit()}

if(defined($singlepackage)) {exit()}
indexfooter(INDEX);
close(INDEX);

# check to see if the index.html file has changed
filediff ("$cvsdoc/html/index.html", "i.i");   

# look at all applications and report the ones with missing documentation
foreach $thisprogram (sort (keys %progs)) {
    if ($progdone{$thisprogram}) {next}
    print "$thisprogram.html =missing=\n";
}


#############################
#
# NOW PROCESS THE GROUPS FILE
#
#############################

open (GRPSTD, "$installtop/share/EMBOSS/acd/groups.standard") ||
    die "Cannot open $installtop/share/EMBOSS/acd/groups.standard";
while (<GRPSTD>) {
    if (/^\#/) {next}
    if (/^([^ ]+) (\S+) (.*)/) {
#	$gterm = $1;
	$gname = $2;
	$gdesc = ucfirst(lc($3));
	$gname =~ s/:/ /g;
	$gname =~ s/_/ /g;
	$gname = ucfirst(lc($gname));
	$gname =~ s/[Dd]na/DNA/;
	$gname =~ s/[Rr]na/RNA/;
	$gname =~ s/[Cc]pg/CpG/;
	$gname =~ s/Hmm/HMM/i;
	$gdesc =~ s/[Dd]na/DNA/;
	$gdesc =~ s/[Rr]na/RNA/;
	$gdesc =~ s/[Cc]pg/CpG/;
	$gdesc =~ s/Hmm/HMM/i;
	$grpdef{$gname} = $gdesc;
    }
}
close GRPSTD;

$docdir = "$cvsdoc/master/emboss/apps";
$sfdocdir = "$sfdoctop/apps/cvs";

open ( SUM, ">g.g") || die "cannot open temporary groups summary file";

header1(SUM);
print SUM "Application Groups";
header2(SUM);


print SUM "
<table border cellpadding=4 bgcolor=\"#FFFFF0\">

<tr><th>Group</th><th>Description</th></tr>

";

foreach $g (sort (keys %groups)) {
# change the capitalisation on a few group names - most are lowercase
    $name = $g;
    $name =~ s/_/ /g;
    $name = ucfirst(lc($name));
    $name =~ s/[Dd]na/DNA/;
    $name =~ s/[Rr]na/RNA/;
    $name =~ s/Cpg/CpG/i;
    $name =~ s/Hmm/HMM/i;
    if(!defined($grpdef{$name})){
	print STDERR "Unknown group '$name'\n";
	$grpdef{$name} = $name;
    }
    $desc = $grpdef{$name};

# this group's name is too long for the Makefile 
    $filename = $g;
    $filename =~ s/restriction_enzymes/re/;


    print SUM "<tr>
<td><A HREF=\"$filename\_group.html\">$name</A></td>
<td>$desc</td>
</tr>\n";
# print "$filename '$groups{$g}' '$grpnames{$g}'\n";

    open ( GRP, ">y.y") || die "cannot open temporary group file";

    header1 (GRP);
    print GRP "$grpnames{$g}";
    header2 (GRP);

    print GRP "
$desc
<p>

<table border cellpadding=4 bgcolor=\"#FFFFF0\">

<tr><th>Program name</th><th>Description</th></tr>
";

    foreach $p (split (/\s+/, $groups{$g})) {
#   print "$g : '$p'\n";
	if ($progdir{$p}) {
	    print GRP "
<tr>
<td><a href=\"../embassy/$progdir{$p}/$p.html\">$p</a></td>
<td>
$progs{$p}
</td>
</tr>
";

	}
	else {
	    print GRP "
<tr>
<td><a href=\"$p.html\">$p</a></td>
<td>
$progs{$p}
</td>
</tr>
";
	}
    }

    print GRP "

</table>

";

    footer (GRP);

    close GRP;

    filediff ("$docdir/$filename\_group.html", "y.y");

# check to see if the CVS tree copy of the html program group documentation
# needs updating
    $status = htmlsource("$filename\_group.html","$cvsdoc/master/emboss/apps");
#	$status = system "lynx -source $url/$filename\_group.html > x.x";
    if (!$status) {
	$badsrc++;
	print "htmlsource error $status $url/$filename\_group.html";
    }
    else {
# change ../emboss_icon.jpg and ../index.html to current directory
	system "perl -p -i -e 's#\.\.\/index.html#index.html#g;' x.x";
	system "perl -p -i -e 's#\/images/emboss_icon.jpg#emboss_icon.jpg#g;' x.x";
	filediff ("$cvsdoc/html/$filename\_group.html", "x.x");
    }
}

print SUM "

</table>
";

footer(SUM);

close SUM;

filediff("$docdir/groups.html", "g.g");

# check to see if the CVS tree copy of the html group documentation needs updating
$status = htmlsource("groups.html","$cvsdoc/master/emboss/apps");
#    $status = system "lynx -source $url/groups.html > x.x";
if (!$status) {
    $badsrc++;
    print "htmlsource error $status $url/groups.html";
}
else {
# change ../emboss_icon.jpg and ../index.html to current directory
    system "perl -p -i -e 's#\.\.\/index.html#index.html#g;' x.x";
    system "perl -p -i -e 's#\/images/emboss_icon.jpg#emboss_icon.jpg#g;' x.x";
    filediff ("$cvsdoc/html/groups.html", "x.x");
}

######################################################################
# OK - we have updated all our files, now CVS add and CVS commit them 
######################################################################

chdir "$cvsdoc/html";

if ($cvsdochtmladd ne "") {
    print "cvs add -m'documentation added' $cvsdochtmladd\n";
    print STDERR "cvs add -m'documentation added' $cvsdochtmladd\n";
    if ($cvscommit) {
	system "cvs add -m'documentation added' $cvsdochtmladd";
    }
}
if ($cvsdochtmlcommit ne "") {
    print "cvs commit -m'documentation committed' $cvsdochtmlcommit\n";
    print STDERR "cvs commit -m'documentation committed' $cvsdochtmlcommit\n";
    if ($cvscommit) {
	system "cvs commit -m'documentation committed' $cvsdochtmlcommit";
    }
}
else {
    print STDERR "HTML docs unchanged\n";
}

chdir "$cvsdoc/text";

if ($cvsdoctextadd ne "") {
    print "cvs add -m'documentation added' $cvsdoctextadd\n";
    print STDERR "cvs add -m'documentation added' $cvsdoctextadd\n";
    if ($cvscommit) {
	system "cvs add -m'documentation added' $cvsdoctextadd";
    }
}
if ($cvsdoctextcommit ne "") {
    print "cvs commit -m'documentation committed' $cvsdoctextcommit\n";
    print STDERR "cvs commit -m'documentation committed' $cvsdoctextcommit\n";
    if ($cvscommit) {
	system "cvs commit -m'documentation committed' $cvsdoctextcommit";
    }
}
else {
    print STDERR "TEXT docs unchanged\n";
}

# No need to make these makefiles ... now they use wildcards
#print "Create make files\n";
#system("$scripts/makeMake.pl");	       # no parameter == do text
#system("$scripts/makeMake.pl html");   # any parameter == do html

print "\n";
print LOG "\n";
print "==================================\n";
print LOG "==================================\n";
print "Lynx errors: $badlynx\n";
print LOG "Lynx errors: $badlynx\n";
print "HTML source errors: $badsrc\n";
print LOG "HTML source errors: $badsrc\n";
foreach $x (sort(keys(%missingdoc))) {
    print "Missing: $missingdoc{$x}/$x.html\n";
    print LOG "Missing: $missingdoc{$x}/$x.html\n";
}
