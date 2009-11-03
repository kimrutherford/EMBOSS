#!/usr/bin/perl -w

# This is a utility to aid prople who wish to edit the html
# documentation for an application.  It is somewhat more friendly than
# 'autodoc.pl' in that it only works with the specified application's
# documentation and will confirm any action with the user. 

###################################################################
#
# Some useful definitions
#
###################################################################
# where the URL for the html pages is
$url = "http://www.uk.embnet.org/Software/EMBOSS/Apps/";

# where the CVS tree program doc pages are
$cvsdoc = "/packages/emboss_dev/$ENV{'USER'}/emboss/emboss/doc/programs/";

# where the CVS tree scripts are
$scripts = "/packages/emboss_dev/$ENV{'USER'}/emboss/emboss/scripts";

# where the web pages live
$docdir = "/data/www/Software/EMBOSS/Apps";


###################################################################
# check that we are on the CVS machine
#require 'hostname.pl';
#if (hostname() ne "tin") {
#	die "This script should be executed on the CVS machine 'tin'\n";
#}


###################################################################
# get an editor
###################################################################
$editor = $ENV{'EDITOR'};
if (!defined $editor || $editor eq "") {
    print "What is your favourite editor [emacs] >";
    $editor = <STDIN>;
    chomp $editor;
    if ($editor eq "") {$editor = 'emacs';}
}

print "Utility to edit an application's html documentation\n\n";

if ($#ARGV != 0) {
	print "Name of the program >";
	$application = <STDIN>;
} else {
	$application = $ARGV[0];
}
chomp $application;
if (!defined $application || $application eq "") {die "No program specified\n";}

###################################################################
# get details of the program from wossname
###################################################################

# read in from the EMBOSS application 'wossname'
# group names, application name and which application is in which groups

open (PROGS, "wossname -auto -search $application |") || die "Cannot run wossname\n";
$grp = "";
while (<PROGS>) {
  if (/^\s*$/) {next}
  if (/^([A-Z ]+)$/) {
    $capgrp = $1;
    $grp = lc($capgrp);
    $grp =~ s/ +/_/g;
    next;
  }
  if (/^(\S+) +(.*)/) {
    $progs{$1} = $2;
  } 
}
close PROGS;


# look at all applications alphabetically
foreach $x (sort (keys %progs)) {
  if ($x eq $application) {
	print "Description of this application according to wossname:\n$x '$progs{$x}'\n";
	$desc = $progs{$x};
  }
}
if (!defined $desc) {die "wossname can't find $application\nExiting.\n";}


###################################################################
# check to see if this application has been committed yet
###################################################################

# we need to cd to the directory where the application was committed
chdir "/packages/emboss_dev/$ENV{'USER'}/emboss/emboss/emboss/";
$result = system "cvs log $application.c  >/dev/null";
$result /= 256;
if ($result == 1) {
	print "WARNING: This program has not yet been committed to CVS.\n";
	print "This is not a problem because users won't see the web page\nuntil it is added to the index page.\n";
	print "Continue (y/n) >";
###	$ans = <STDIN>;
	$ans = "skip";
	$ans = lc($ans);
	if ($ans !~ /^y/) {die "Exiting.\n";}
}



###################################################################
# check to see if there is a web page already
###################################################################
if (!-e "$docdir/$application.html") {
    print "Create a web page for this program? (y/n) ";
###    $ans = <STDIN>;
    $ans = "skip";
    $ans=lc($ans);
    if ($ans !~ /^y/) {
	die "Exiting.\n";
    } else {
	system("cp $docdir/template.html.save $docdir/$application.html");
        system "perl -p -i -e 's/ProgramNameToBeReplaced/$application/g;' $docdir/$application.html";
	chmod 0664, "$docdir/$application.html";
    }
}
system("$editor $docdir/$application.html");

###################################################################
# make the include files
###################################################################

# create the new one-line description output
open(FH, ">$docdir/inc/$application.ione") || die "Can't open file $docdir/inc/$application.ione\n";
print FH $desc;
close(FH);
chmod 0664, "$docdir/inc/$application.ione";
print "$application.ione *created*\n";

# create the new '-help' output
system "acdc $application -help -verbose 2> $docdir/inc/$application.ihelp";
chmod 0664, "$docdir/inc/$application.ihelp";
print "$application.ihelp *created*\n";

# create the new command table include file
system "acdtable $application 2> $docdir/inc/$application.itable";
chmod 0664, "$docdir/inc/$application.itable";
print "$application.itable *created*\n";

# create the new 'seealso' output
system "seealso $application -auto -html -post '.html' -out $docdir/inc/$application.isee";
system "perl -p -i -e 's/SEE ALSO/See also/g;' $docdir/inc/$application.isee";
chmod 0664, "$docdir/inc/$application.isee";
print "$application.isee *created*\n";

# create the '.usage', '.input' and '.output' include files
system "$scripts/makeexample.pl $application";

###################################################################
# edit the index.html file
###################################################################
print "\n\n
To make the documentation available to the public you will have to
manually edit the index.html file for the applications directory There
will be an entry at the end of the index page for you to move into
position. 
\n\n";
print "Edit the index web page (y/n) >";
###$ans = <STDIN>;
$ans = "skip";
$ans=lc($ans);
if ($ans =~ /^y/) {  

    open (INDEX, ">> $docdir/index.html") || die "Can't open $docdir/index.html\n";
    print INDEX "

<tr><td><a href=\"$application.html\">$application</a></td><td>HGMP</td><td>
$desc
</td></tr>
";
    close (INDEX);
    system("$editor $docdir/index.html");
}

###################################################################
# CVS commit text and html copies in the EMBOSS package
###################################################################

print "CVS commit text and html copies in the EMBOSS package (y/n) >";
###$ans = <STDIN>;
$ans = "skip";
$ans=lc($ans);
if ($ans !~ /^y/) {  
    die "Exiting.\n";   
} else {

    chdir $cvsdoc;

# create the new text output
    if (!-e "text/$application.txt") {
      system "lynx -dump -nolist $url/$application.html > text/$application.txt";
      chmod 0664, "text/$application.txt";
      system "cvs add -m'documentation created' text/$application.txt";
    } else {
      system "lynx -dump -nolist $url/$application.html > text/$application.txt";
      chmod 0664, "text/$application.txt";
    }
    system "cvs commit -m'documentation created' text/$application.txt";
    print "$application.txt *created*\n";

# create the new html output
    if (!-e "html/$application.html") {
      system "lynx -source $url/$application.html > html/$application.html";
      chmod 0664, "html/$application.html";
      system "cvs add -m'documentation created' html/$application.html";
    } else {
      system "lynx -source $url/$application.html > html/$application.html";
      chmod 0664, "html/$application.html";
    }
# change ../emboss_icon.jpg and ../index.html to current directory
    system "perl -p -i -e 's#\.\.\/index.html#index.html#g;' html/$application.html";
    system "perl -p -i -e 's#\.\.\/emboss_icon.jpg#emboss_icon.jpg#g;' html/$application.html";
    system "cvs commit -m'documentation created' html/$application.html";
    print "$application.html *created*\n";
}

print "Create make files\n";
#chdir "/packages/emboss_dev/$ENV{'USER'}/emboss/emboss/scripts";
system("$scripts/makeMake.pl");        # no parameter == do text
system("$scripts/makeMake.pl html");

print "Done.\n";
    
