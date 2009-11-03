#!/usr/bin/perl -w

# This is a utility to create the usage, input and output
# example HTML include files for the EMBOSS application documentation.
#
# Can now run from any directory, with pre and post processing of the test
# environment done by using the standard test script
#
# Uses the qatest.dat file to find and run tests. No need to create
# all the test results first
#
# Cleans up the qa test directory on success
#
# Skips bad tests with a log message - these should be fixed!
#
# The 'scripts/qatest.pl' script will be run with the argument '-kk' 
# to create the example usage, input and output files.

use File::Basename;


open (LOG,">>makeexample.log") || die "Cannot append to makeexample.log";

###################################################################
#
# Some useful definitions
#
###################################################################

open (VERS, "embossversion -full -auto|") || die "Cannot run embossversion";
while (<VERS>) {
#   if(/InstallDirectory: +(\S+)/) {$installtop = $1}
    if(/BaseDirectory: +(\S+)/) {$distribtop = $1}
}
close VERS;

# where the qa directories are
$qatop = "$distribtop/test";
$scripts = "$distribtop/scripts";
$embassy = "";

###################################################################
# get the name of the application
if ($#ARGV < 0) {
    print "Name of the program >";
    $application = <STDIN>;
} else {
    $application = $ARGV[0];
    if ($#ARGV > 0) {$embassy = $ARGV[1]}
}
chomp $application;
if (!defined $application || $application eq "") {
    die "No program specified\n";
}

# where the web pages and include files live
$doctop = "$ENV{HOME}/cvsemboss";
if ($embassy eq "") {
    $docdir = "$doctop/doc/programs/master/emboss/apps";
    $incdir = "$docdir/inc";
}
else {
    $docdir = "$doctop/embassy/$embassy/emboss_doc/master";
    $incdir = "$docdir/inc";
}


# some HTML
$p = "<p>";
$bold = "<b>";
$unbold = "</b>";

# maximum number of lines of a file to be displayed
$MaxLines = 100;
%testok = ();
%testkeep = ();

# names of test databases
@testdbs = (
	    'tsw',
	    'tswnew',
	    'twp',
	    'tembl',
	    'tpir',
	    );

# colours for backgrounds of usage examples, input and output files
# these colours should work on all PCs/Macs/UNIX web browsers
$usagecolour = "#CCFFFF"; # light cyan
$inputcolour = "#FFCCFF"; # light violet
$outputcolour = "#CCFFCC"; # light green



###################################################################
# initialise the outputs
$USAGE = "";
$INPUT = "";
$OUTPUT = "";
$hasinput = 0;
$haskeystrokes = 1;
$hasoutkeystrokes = "";

@testdbsoutput = ();
@inputfilesshown = ();
@outputfilesshown = ();

# debug
#open (OUT, "> /people/gwilliam/jjj.html") || die "Can't open debug OUT\n";


print LOG "\n";
print LOG "Doing $application\n";

###################################################################
# check for any qa '*-ex' example directories

@dirs=();

open (QATEST, "$qatop/qatest.dat") || die "Cannot open $qatop/qatest.dat";
$expex = qr/^$application[-]ex/;
$expcheck = qr/^$application[-]check/;
$expkeep = qr/^$application[-]keep$/;
while (<QATEST>) {
    if (/^ID +(\S+)/) {
	$idqa = $1;
	if ($idqa =~ /$expex/) {push @dirs, $idqa};
	if ($idqa =~ /$expcheck/) {push @dirs, $idqa};
	if ($idqa =~ /$expkeep/) {push @dirs, $idqa};
    }
}

# are there any results directories?
if ($#dirs == -1) {
    errorexit("No qa tests were found for $application");
}

# sort the directory names
@dirs = sort @dirs;

chdir ("$qatop/qa");

###################################################################
# get next example directory
$count = 0;
foreach $dotest (@dirs) {
    $dir = "$dotest";
    print "Doing test $dotest\n";
    print LOG "Doing test $dotest\n";
    # run the test with -kk
    open (TEST,"$scripts/qatest.pl -kk -logfile=qatest.doclog $dotest 2>&1|");
    $testok{$dotest} = 0;
    $testkeep{$dotest} = 0;
    while (<TEST>) {
	if (/^Tests total: 1 pass: 1 fail: 0$/) {$testok{$dotest}=1}
    }
    if (!$testok{$dotest}) {
	print LOG "Test $dotest failed\n";
	print "Test $dotest failed\n";
	if ($embassy eq "") {
	    print STDERR "Test $dotest failed, ignored\n";
	}
	else {
	    print STDERR "Test ($embassy) $dotest failed, ignored\n";
	}
	next;
    }
    $count++;

###################################################################
# initialise some output messages for this example
    $testdbcomment = "";
    $commandline = "";
    $usagecomment = "";
    $inputcomment = "";
    $outputcomment = "";

###################################################################
# read in 'testdef' file of qa commands
    $testfile = "$dir/testdef";
    open (TESTDEF, "< $testfile") || errorexit("Couldn't open file $testfile");
    @testdef = <TESTDEF>;
    close (TESTDEF);

###################################################################
# parse 'CL, UC, IC, OC' lines from 'testdef'
    foreach $line (@testdef) {
        if ($line =~ /^CL\s+(.+)/) {$commandline .= "$1 ";}
        if ($line =~ /^UC\s+(.+)/) {$usagecomment .= "$1 ";}
        if ($line =~ /^IC\s+(.+)/) {$inputcomment .= "$1 ";}
        if ($line =~ /^FI\s+(.+)/) {push(@outfiles, "$dir/$1");}
        if ($line =~ /^FK\s+(.+)/) {$hasoutkeystrokes = $1;}
        if ($line =~ /^DI\s+(.+)/) {push(@outfiles, "$dir/$1");$savedir = $1;}
        if ($line =~ /^DF\s+(.+)/) {$outdirfiles{$savedir} .= "$1 ";
				push(@outfiles, "$dir/$savedir/$1");}
        if ($line =~ /^OC\s+(.+)/) {$outputcomment .= "$1 ";}
        if ($line =~ /^DL\s+keep/) {$testkeep{$dotest} = 1;}
        if ($line =~ /^AB\s+(\S+)/) {
	    $embassypackage = "$1";
	    $docdir = "$doctop/embassy/$embassypackage/emboss_doc/master";
	    $incdir = "$docdir/inc";
	}
        if ($line =~ /^IN\s+/) {$hasinput = 1;}
        if ($line =~ /^IK\s+/) {$haskeystrokes = 1;}
    }

# remove stderr, stdin, stdout, testdef, testlog from this list
    @outfiles = grep !/stdin|stderr|stdout|testdef|testlog/, @outfiles;

###################################################################
# read in 'stderr' file of prompts
# Watch out for acdtrace which has extra lines
# in all other cases, @prompts = <PROMPTS> would be correct and simpler.
    $promptfile = "$dir/stderr";
    open (PROMPTS, "< $promptfile" ) || errorexit("Couldn't open file $promptfile");
    $plines="";
    while (<PROMPTS>) {
	s/Trace:.*\n//;
	s/\n/\n\r/;
	$plines .= $_;
    }
    close (PROMPTS);
    (@prompts) = split(/\r/, $plines);

###################################################################
# read in 'stdin' file of responses to prompts
    if($hasinput) {
	$answerfile = "$dir/stdin";
	open (ANSWERS, "< $answerfile") || errorexit("Couldn't open file $answerfile");
	@answers = <ANSWERS>;
	close (ANSWERS);
    }
    else {
	@answers = ();
	# if $haskeystrokes could put comment about input keystrokes here
    }
    @saveanswers = @answers;

###################################################################
# read in 'stdout' file of results written to screen
    if($hasoutkeystrokes eq "stdout") {
	@results = ();
# could put some comment about the output keystrokes here
    }
    else {
	$resultsfile = "$dir/stdout";
	open (RESULTS, "< $resultsfile") || errorexit("Couldn't open file $resultsfile");
	@results = <RESULTS>;
	close (RESULTS);
    }

###################################################################
# change any ampersands or angle brackets to HTML codes
    $commandline =~ s/&/&amp;/g;
    $commandline =~ s/</&lt;/g;
    $commandline =~ s/>/&gt;/g;

    $usagecomment =~ s/&/&amp;/g;
    $usagecomment =~ s/</&lt;/g;
    $usagecomment =~ s/>/&gt;/g;

    $inputcomment =~ s/&/&amp;/g;
    $inputcomment =~ s/</&lt;/g;
    $inputcomment =~ s/>/&gt;/g;

    $outputcomment =~ s/&/&amp;/g;
    $outputcomment =~ s/</&lt;/g;
    $outputcomment =~ s/>/&gt;/g;

###################################################################
# change newlines to <br>
    $commandline =~ s/\n/<br>\n/g;
    $usagecomment =~ s/\n/<br>\n/g;
    $inputcomment =~ s/\n/<br>\n/g;
    $outputcomment =~ s/\n/<br>\n/g;

###################################################################
# see if any of the arguments of CL are real input files
    @infiles = ();
    @inusas = ();

# split on space, >, <, = - all are possible before a file name
    foreach $f (split /\s|\>|\<|\=/, $commandline) {
        chomp $f;
# ignore qualifiers - words starting with a '-'
        if ($f =~ /^-/) {next;}
# split on '::' to get files embedded in a format::file USA
        if ($f =~ /\:\:/) {
	    print "CL line=$f\n";
            @fm = split /\:\:/, $f;
            $f = pop @fm;
	    print "CL f=$f\n";
        }
# deal with '@' in list files
        $f =~ s/\@//;
# check to see if this is an output file existing in the -ex directory
        if (grep /^$f$/, @outfiles) {
            next;
        }

# check to see if it is a real file (not a directory)
        if (-f "$dir/$f" && ! -d "$dir/$f") {
# we assume all files in the *-ex directory are output files
# check for '.' or '/' at start of path
            if ($f =~ /^\./ || $f =~ m#^\/#)  {
#print "displaying infile=$f\n";
                push @infiles, $f;
            }
        }
	else {
	    push @inusas, $f; 
        }
    }

###################################################################
# see if any of the answers to prompts are real files
# split on '::' to get files embedded in a format::file USA
    foreach $line (@answers) {
        chomp $line;
	if (length $line) {
#print "line=$line\n";
            @usas = split /\:\:/, $line;
            $f = pop @usas;
#print "f=$f\n";
# deal with '@' in list files
            $f =~ s/\@//;
# check to see if this is an output file existing in the -ex directory
            if (grep /^$f$/, @outfiles) {
                next;
            }
# check to see if it is a real file (not a directory)
#print "test for file: $dir/$f\n";
            if (-f "$dir/$f" && ! -d "$dir/$f") {
# we assume all files in the *-ex directory are output files
# check for '.' or '/' at start of path
                if ($f =~ /^\./ || $f =~ m#^\/#)  {
#print "displaying infile=$f\n";
                    push @infiles, $f;
                }
            }
	    else {
		push @inusas, $line; 
            }
        }
    }

###################################################################
# see if any of the answers to prompts are lists of real files
# split on ',' to get files embedded in a file list 'file1,file2,file3'
    foreach $line (@answers) {
        chomp $line;
	if (length $line) {
            @filelist = split /,\s*/, $line;
            foreach $f (@filelist) {
# deal with '@' in list files
                $f =~ s/\@//;
# if ':' in it then ignore - not a file
                if ($f =~ /\:/) {
#print "ignoring $f in list of input files\n";
                    next;
                }
# check to see if it is a real file (not a directory)
                if (-f "$dir/$f" && ! -d "$dir/$f") {
# we assume all files in the *-ex directory are output files
# check for '.' or '/' at start of path
                    if ($f =~ /^\./ || $f =~ m#^\/#)  {
			push @infiles, $f;
		    }
		}
		else {
		    push @inusas, $line; 
		}
	    }
	}
    }

###################################################################
# see if we use the test databases anywhere in the command line or answers
    foreach $f (@answers, split (/\s|\>|\<|\=/, $commandline)) {
	chomp $f;
	@d = split (/\:/, $f);
	foreach $d (@d) {
	    if ($d =~ /\*/) {next;}
	    if ($d =~ /[\[]/ && $d !~ /[\[].*[\]]/ ) {next}
	    if (grep /^$d$/, @testdbs) {
# check we have not made any other comments about this test database
		if (! grep /$d/, @testdbsoutput) {
		    if ($d eq "tembl") {
			$type = "nucleic acid";
		    }
		    else {
			$type = "protein";
		    }
		    $testdbcomment .= "\n'$f' is a sequence entry in the example $type database '$d'\n$p\n";
		    push @testdbsoutput, $d;
		}
	    }
	}
    }


###################################################################
# usage documentation
###################################################################

# example count
    if ($count == 1) {
	$USAGE .= qq|<b>Here is a sample session with $application</b>\n$p\n|;
    }
    else {
	$USAGE .= qq|$p\n<b>Example $count</b>\n$p\n|;
    }

# usage comment
    if ($usagecomment ne "") {
	$USAGE .= qq|$usagecomment\n$p\n|;
    }

# blank line
    $USAGE .= qq|\n$p\n|;

# start table (light cyan)
    $USAGE .= qq|<table width="90%"><tr><td bgcolor="$usagecolour"><pre>\n\n|;

# application name and command line (we just use the test results)
#    print "Doing: \% $application $commandline\n";
    $usecommandline = $commandline;
    $usecommandline =~ s/[.][.]\/..\/data\/[^\/]+\///; # ../../data/(embassy)/
    $usecommandline =~ s/[.][.]\/..\/data\///;	       # ../../data/
    $usecommandline =~ s/[.][.]\/..\///;               # ../../embl (etc.)
    $USAGE .= qq|% $bold$application $usecommandline$unbold\n|;

# intercalate prompts and answers
    @pr = ();
    foreach $line (@prompts) {
	$line =~ s/[.][.]\/..\/data\/[^\/]+\///;       # ../../data/(embassy)/
	$line =~ s/[.][.]\/..\/data\///;	       # ../../data/
	$line =~ s/[.][.]\/..\///;		       # ../../embl (etc.)
	push @pr, split /([^\s]+: )/, $line;
    }
    foreach $line (@pr) {
	$USAGE .= qq|$line|;
# change ':'s in warning messages so that they don't look like prompts
# although if we get a warning, things are probably going badly wrong anyway
#print "prompt=$line\n";
	$line =~ s/Warning:/Warning :/;
	$line =~ s/Error:/Error :/;
	$line =~ s/Fatal:/Fatal :/;

# We also get warnings from blastpgp in some domainatrix apps
	$line =~ s/WARNING:/WARNING :/;
	$line =~ s/posFindAlignmentDimensions:/posFindAlignmentDimensions :/;
	$line =~ s/posProcessAlignment:/posProcessAlignment :/;
# print the answer in bold
#print "$line\n";
	if ($line =~ /[^\s]: $/) {
	    $ans = shift @answers;
#print "prompt=$line\n";
#print "ans=$ans\n";
	    if (!defined($ans)) {
		$ians = $#saveanswers - $#answers;
		print STDERR "application '$application' \@answers[$ians] undefined\n";
#		$ians = 0;
#		foreach $sa (@saveanswers) {
#		    $ians++;
#		    print STDERR "\$saveanswers[$ians] = '$sa'\n";
#		}
		$ans = "\n";
	    }
	    $ans =~ s/[.][.]\/..\/data\/[^\/]+\///; # ../../data/(embassy)/
	    $ans =~ s/[.][.]\/..\/data\///;	   # ../../data/
	    $ans =~ s/[.][.]\/..\///;		   # ../../embl (etc.)
	    $USAGE .= "$bold$ans$unbold\n";
	}
    }

# have we used all of our answers?
    if ($#answers != -1) {
	print STDERR "WARNING **** 
application '$application' example $count test $dotest 
hasn't used ", $#answers+1, " answers\n";
	print LOG "WARNING **** 
test $dotest hasn't used ", $#answers+1, " answers\n";
    }

# display any results found in the 'stdout' file
    $USAGE .= qq|\n|;
    foreach $r (@results) {$USAGE .= qq|$r|;}
# end stdout results with another blank line
    if ($#results > -1) {$USAGE .= qq|\n|;}

# end table
    $USAGE .= qq|</pre></td></tr></table>$p\n|;

# blank line
    $USAGE .= qq|$p\n|;

###################################################################
# input documentation
###################################################################

# If the command line of answers contain tembl, tsw etc.
# add a comment about these being test databases.
# only add this comment once
    $I = $testdbcomment;

# input comment
    $I .= $inputcomment;

# foreach input file
    foreach $file (@infiles) {
#print "input file=$file\n";
	$shortfile = $file;
	$shortfile =~ s/[.][.]\/..\/data\/[^\/]+\///; # ../../data/(embassy)/
	$shortfile =~ s/[.][.]\/..\/data\///;	  # ../../data/
	$shortfile =~ s/[.][.]\/..\///;		  # ../../embl (etc.)

# if this some sort of binary file?
	if (checkBinary("$dir/$file")) {
	    $I .= $p . "<h3>File: $shortfile</h3>\n";
	    $I .= $p . "This file contains non-printing characters and so cannot be displayed here.\n";

# normal file that can be displayed
	}
	else {

# has the name has been used before
	    if (! grep /^$file$/, @inputfilesshown) { 
		push @inputfilesshown, $file;

# no - contruct new display of file
# add example number to the list of examples that use this file
		$I .= displayFile($shortfile, "$dir/$file", $inputcolour);
	    }
	}
    }

# the @inusas list of inputs are possible USAs, but have not been checked.
# see if they have a ':' in them and display using entret.
# If it has an '*' then ignore it,
    foreach $f (@inusas) {
	if ($f =~ /\:/ && $f !~ /\*/ && $f !~ /http\:/) {
# has the name has been used before
	    if (! grep /^$f$/, @inputfilesshown) { 
		push @inputfilesshown, $f;
		$I .= displayEntry($f, $inputcolour);
	    }
	}
    }


# If any new files were output for this example, 
    if (length $I) {
	$INPUT .= qq|\n<a name="input.$count"></a>\n<h3>Input files for usage example |;
	if ($count > 1) {$INPUT .= "$count";}
	$INPUT .= "</h3>\n";
	$INPUT .= $I;

# add a link from the usage
	$USAGE .= qq|<a href="#input.$count">Go to the input files for this example</a><br>|;
    }



###################################################################
# output documentation
###################################################################

    $O = "";

# output comment
    $O .= $outputcomment;

# foreach output file
    foreach $path (@outfiles) {
#print "output file=$path\n";

#	$basefile = basename($path);

	if ($path !~ /^$dir\//) {next} # multiple tests

	$file = $path;
	$file =~ s/^$dir\///;

# if this a .gif, .ps or .png graphics file?
	if ($file =~ /\.gif$|\.ps$|\.png$/) {

# convert .ps file to gif
	    $giffile = "";
	    $origfile = $file;
	    if ($file =~ /\.ps$/) {
		$giffile = $file;
		$giffile =~ s/\.ps/.gif/;
# add -delay to see the first page of an animated gif for 10 mins
		system("2>&1 convert -delay 65535 -rotate '-90<' $path $giffile >/dev/null");
		$file = $giffile;
		$path = $giffile;
	    }

# rename file to be unique - application name . example count . given file name
	    $newfile = "$application.$count.$file";
	    system("cp $path $docdir/$newfile");
	    chmod 0664, "$docdir/$newfile";

# tidy up
	    if ($giffile ne "") {
		unlink $giffile;
	    }

# display the graphics file
	    $O .= $p . "<h3>Graphics File: $origfile</h3>\n";
	    $O .= $p . qq|<img src="$newfile" alt="[$application results]">\n|;

# if this some other binary file?
	}
	elsif (-d $path) {
	    $O .= $p . "<h3>Directory: $file</h3>\n";
	    $O .= $p . "This directory contains output files";
	    if(defined($outdirfiles{$file})) {
		$outtmp = $outdirfiles{$file};
		$outtmp =~ s/ (\S+) $/ and $1/;
		$outtmp =~ s/ $//;
		$O .= ", for example $outtmp\.\n";
	    }
	    else {
		$O .= ".\n$p\n";
	    }
	}
	elsif (checkBinary($path)) {
	    $O .= $p . "<h3>File: $file</h3>\n";
	    $O .= $p . "This file contains non-printing characters and so cannot be displayed here.\n";

# normal file that can be displayed
	}
	else {

# has the name has been used before (match to end of path in @outputfilesshown)
	    @o = grep /$file$/, @outputfilesshown;
	    if ($#o == -1) { 
		push @outputfilesshown, $path;

# no - contruct new display of file
#print "displaying $file\n";
		$O .= displayFile($file, $path, $outputcolour);
	    }
	    else {
# do a diff of the two files
		$ofile = pop @o;
		system ("diff  $ofile $path> diff.tmp");
		if ( -s "diff.tmp" ) {
# it is a different file
		    $O .= displayFile($file, $path, $outputcolour);
		}
		unlink "diff.tmp";
	    }
	}
    }

# If any new files were output for this example, 
    if (length $O) {
	$OUTPUT .= qq|\n<a name="output.$count"></a>\n<h3>Output files for usage example |;
	if ($count > 1) {$OUTPUT .= "$count";}
	$OUTPUT .= "</h3>\n";
	$OUTPUT .= $O;

# add a link from the usage
	$USAGE .= qq|<a href="#output.$count">Go to the output files for this example</a>$p|;
    }

# force a blank line to be at the end of the usage
    $USAGE .= "$p\n";
}

$testsfailed = "";
$nfailed = 0;
foreach $dotest (@dirs) {
    if (!$testok{$dotest}) {
	if ($nfailed) {$testsfailed .= ", "}
	$testsfailed .= "$dotest";
	$nfailed++;
    }
}
if ($nfailed) {
    if($nfailed == 1) {
	errorexit("Test $testsfailed failed");
    }
    else {
	errorexit("Tests $testsfailed failed");
    }
}


###################################################################
# create the usage, input and output documentation files

writeUsage($USAGE);
writeInput($INPUT);
writeOutput($OUTPUT);

# debug
#print OUT "USAGE=\n$USAGE\n";
#print OUT "INPUT = \n$INPUT";
#print OUT "OUTPUT = \n$OUTPUT";
#close(OUT);

foreach $dotest (@dirs) {
    if ($testok{$dotest}) {
	if (!$testkeep{$dotest}) {
	    system ("rm -rf $dotest");
	}
    }
}

close LOG;
exit(0);

###################################################################
###################################################################



###################################################################
# Name:		errorexit
# Function:	leaves output files unchanged and exits with an error message
# Args:		string - error message
# Returns:	exits - no return
###################################################################
sub errorexit {
    my $msg = $_[0];

    my $usage = "Error: $msg";
    my $input = "Error: $msg";
    my $output = "Error: $msg";

#    writeUsage($usage);
#    writeInput($input);
#    writeOutput($output);
    
    if ($embassy eq "") {
	print STDERR "Error: $msg\n";
	print "Error: $msg\n";
    }
    else {
	print STDERR "Error: ($embassy) $msg\n";
	print "Error: ($embassy) $msg\n";
    }
    exit 1;
}

###################################################################
# Name:        writeUsage
# Function:    writes usage file
# Args:        string - example usage
# Returns:    
###################################################################
sub writeUsage {
    my $usage = $_[0];

    my $out = "$incdir/$application.usage";
    open (OUT, "> $out") || die "Can't open $out";
    $usage =~ s/\/homes\/pmr\/cvsemboss/\/homes\/user/go;
    $usage =~ s/(Guide tree +file created: +)\[[A-Z0-9]+\]/$1\[12345678A]/go;
    $usage =~ s/(GCG-Alignment file created +)\[[A-Z0-9]+\]/$1\[12345678A]/go;
    $usage =~ s/domainalign\-[0-9]+[.][0-9]+/domainalign-1234567890.1234/go;
    $usage =~ s/domainrep\-[0-9]+[.][0-9]+[.]/domainrep-1234567890.1234./go;
    $usage =~ s/domainrep\-[0-9]+[.][0-9]+ /domainrep-1234567890.1234 /go;
    $usage =~ s/pdbplus\-[0-9]+[.][0-9]+ /pdbplus-1234567890.1234 /go;
    $usage =~ s/seqalign\-[0-9]+[.][0-9]+[.]/seqalign-1234567890.1234./go;
    $usage =~ s/seqsearch\-[0-9]+[.][0-9]+[.]/seqsearch-1234567890.1234./go;
    $usage =~ s/hmmalign\-[0-9]+[.][0-9]+/hmmalign-1234567890.1234/go;
    $usage =~ s/hmmpfam\-[0-9]+[.][0-9]+/hmmpfam-1234567890.1234/go;

    print OUT $usage;
    close(OUT);
    chmod 0664, $out;	# rw-rw-r--
}

###################################################################
# Name:		writeInput
# Function:	writes input file
# Args:		string - example input
# Returns:	
###################################################################
sub writeInput {
    my $input = $_[0];

    my $out = "$incdir/$application.input";
    open (OUT, "> $out") || die "Can't open $out";
    $input =~ s/DATE  [A-Z][a-z][a-z] [A-Z][a-z][a-z] +[0-9]+ [0-9:]+ 200[5-9]/DATE  Sun Jul 15 12:00:00 2007/go;
    print OUT $input;
    close(OUT);
    chmod 0664, $out;	# rw-rw-r--
}

###################################################################
# Name:		writeOutput
# Function:	writes output file
# Args:		string - example output
# Returns:	
###################################################################
sub writeOutput {
    my $output = $_[0];

    my $out = "$incdir/$application.output";
    open (OUT, "> $out") || die "Can't open $out";
    $output =~ s/\/homes\/pmr\/cvsemboss/\/homes\/user/go;
    $output =~ s/DATE  [A-Z][a-z][a-z] [A-Z][a-z][a-z] +[0-9]+ [0-9:]+ 200[5-9]/DATE  Sun Jul 15 12:00:00 2007/go;
    $output =~ s/CreationDate: ... ... +\d+ [0-9:]+ 2[0-9][0-9][0-9]$/CreationDate: Sun Jul 15 12:00:00 2007/gom;
    $output =~ s/Rundate: ... ... +\d+ 2[0-9][0-9][0-9] [0-9:]+$/Rundate: Sun Jul 15 2007 12:00:00/gom;
    $output =~ s/\#\#date 2[0-9][0-9][0-9][-][0-9][0-9][-][0-9][0-9]$/\#\#date 2007-07-15/gom;
    $output =~ s/domainalign\-[0-9]+[.][0-9]+[.]/domainalign-1234567890.1234./go;
    $output =~ s/domainrep\-[0-9]+[.][0-9]+[.]/domainrep-1234567890.1234./go;
    $output =~ s/seqalign\-[0-9]+[.][0-9]+[.]/seqalign-1234567890.1234./go;
    $output =~ s/seqsearch\-[0-9]+[.][0-9]+[.]/seqsearch-1234567890.1234./go;
    $output =~ s/hmmalign\-[0-9]+[.][0-9]+/hmmalign-1234567890.1234/go;
    $output =~ s/hmmpfam\-[0-9]+[.][0-9]+/hmmpfam-1234567890.1234/go;
    $output =~ s/Time 0\.00[1-5][0-9][0-9][0-9] secs\./Time 0.001999 secs./go;
    print OUT $output;
    close(OUT);
    chmod 0664, $out;	# rw-rw-r--
}

###################################################################
# Name:		displayEntry
# Function:	returns a string used to display a database entry
# Args:		string - USA
#		string - colour
# Returns:	string - HTML formatted entry contents
###################################################################
sub displayEntry {
    my $usa = $_[0];
    my $colour = $_[1];
    my $res = "";

#print "In displayEntry($usa)\n";

# if the 'file name' contains 'http:' then ignore it :-)
    if ($usa =~ /http\:/) {
	return $res;
    }

# if the USA has a single ':', use entret, else it is a file and we use seqret
    if ($usa !~ /\:\:/ && $usa =~ /\S\:/) {
	system ("entret $usa z.z -auto");
    }
    elsif ($usa =~ /\:\:/) {
	system ("seqret $usa z.z -auto");
    }

    $res = displayFile($usa, "z.z", $colour);

    unlink "z.z";

    return $res;
}

###################################################################
# Name:		displayFile
# Function:	returns a string used to display a file
# Args:		string - file name as used in the example usage
#		string - path to file
#		string - colour
# Returns:	string - HTML formatted files contents
###################################################################
sub displayFile {
    my $file = $_[0];
    my $path = $_[1];
    my $colour = $_[2];

    my $result = "";
    my @lines;
    my $count;

#print "In displayFile($file)\n";

# if the 'file name' contains 'http:' then ignore it :-)
    if ($file =~ /http\:/) {
	return $result;
    }

# if the file has the name 'z.z' (used by displayEntry) then say in the
# title that it is a database entry rather than a file
    if ($path eq "z.z") {
        $result = $p . "<h3>Database entry: $file</h3>\n";
    }
    else {
        $result = $p . "<h3>File: $file</h3>\n";
    }

# start table
    $result .= qq|<table width="90%"><tr><td bgcolor="$colour">\n|;

# if not an .html file, put it in a PRE block
    if ($file !~ /.html$/) {
        $result .= qq|<pre>\n|;
    }

    open (F, "< $path") || die "Can't open input file $path";
    @lines = <F>;
    close (F);

# convert <, >, & to HTML codes if the file is not a .html file
    if ($path !~ /.html$/) {
        foreach my $l (@lines) {
            $l =~ s/[&]/&amp;/g;
            $l =~ s/[<]/&lt;/g;
            $l =~ s/[>]/&gt;/g;
        }
    }

# if file is too long, cut out the middle bit;
    if ($#lines > $MaxLines) {
        for ($count = 0; $count < $MaxLines/2; $count++) {
	    if ($path =~ /.html$/) {
		$lines[$count] =~ s/[&]/&amp;/g;
		$lines[$count] =~ s/[<]/&lt;/g;
		$lines[$count] =~ s/[>]/&gt;/g;
	    }
            $result .= $lines[$count];
        }
        $result .= "\n\n<font color=red>  [Part of this file has been deleted for brevity]</font>\n\n";
        for ($count = $#lines - $MaxLines/2; $count <= $#lines; $count++) {
	    if ($path =~ /.html$/) {
		$lines[$count] =~ s/[&]/&amp;/g;
		$lines[$count] =~ s/[<]/&lt;/g;
		$lines[$count] =~ s/[>]/&gt;/g;
	    }
            $result .= $lines[$count];
        }
    }
    else {
        $result .= join "", @lines;
    }

# if not an .html file, put it in a PRE block
    if ($file !~ /.html$/) {
        $result .= qq|</pre>\n|;
    }

# end table
    $result .= qq|</td></tr></table>$p\n|;

    return $result;
}

###################################################################
# Name:		checkBinary
# Function:	checks to see if a file has non-printing characters in
# Args:		string - path to file
# Returns:	bool - true if file is binary
###################################################################
sub checkBinary {
    my $file = $_[0];

    my $count;
    my $buf;
    my $c;

    open(B, "< $file") || die "Can't open file $file\n";
# get the first 1Kb characters
    read B, $buf, 1024;
    close(B);

#my @n = unpack('C*', $buf);
#for ($count = 0; $count <= $#n; $count++) {
#    $c = pop @n;
#print "c=$c\n"; 
#    if ($c >= 32 && $c <= 126) {next;}
#    if ($c == ord("\t") || $c == ord("\n") || $c == ord("\r")) {next;}
#
#print "Binary\n";
#        return 1;
#    }
#
#print "Not Binary\n";
#    return 0;
	    

    for ($count = 0; $count < length $buf; $count++) {
        $c = ord(substr($buf, $count, 1));
        if ($c >= 32 && $c <= 126) {next;}
	if ($c == ord("\t") || $c == ord("\n") || $c == ord("\r")) {next;}
        return 1;
    }

    return 0;
}
