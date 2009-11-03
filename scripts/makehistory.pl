#!/usr/bin/perl -w

# This is a utility to create the history
# include files for the EMBOSS application documentation.

# GWW 24 Jan 2003



###################################################################
#
# Some useful definitions
#
###################################################################
# where the web pages include files live
$incdir = "/data/www/Software/EMBOSS/Apps/inc";

# colours for backgrounds of usage examples, input and output files
$outputcolour = "#FFFFCC"; # light yellow



###################################################################
# run cvs to create the logs for all applications
    my $apps = "app.log";
    my $acds = "acd.log";

    my $app;
    my $acd;
    my @files;
    my $line;
    my ($date, $author, $desc);

    chdir "/packages/emboss_dev/$ENV{'USER'}/emboss/emboss/emboss/";
    system("cvs log *.c >$apps");

    parse($apps);
    unlink $apps;

    chdir "/packages/emboss_dev/$ENV{'USER'}/emboss/emboss/emboss/acd/";
    system("cvs log *.acd >$acds");

    parse($acds);
    unlink $acds;

# merge the two sets of files together
    chdir $incdir;
    @files = glob("*.applog");
    foreach $app (@files) {
        ($file) = ($app =~ /(\S+)\.applog/);
        $acd = $file . ".acdlog";
        if (-f $acd) {
            system("cat $acd >> $app");
            system("sort $app > $app.sorted");
            if (open(SORT, "< $app.sorted")) {
                open (OUT, "> $file.history") || die "Can't open history file\n";
                print OUT "<table border cellspacing=0 cellpadding=3 bgcolor=\"$outputcolour\">\n";
                print OUT "<tr><th>Date</th><th>Editor</th><th>Comment</th></tr>\n";
                while ($line = <SORT>) {
                    ($date, $author, $desc) = ($line =~ /(\S+)\s+(\S+)\s+(.+)/);
                    print OUT "<tr><td>$date</td><td>$author</td><td>$desc</td></tr>\n";
                }
                print OUT "</table>\n";
                close(SORT);
                close(OUT);
                chmod 0664, "$file.history";    # rw-rw-r-- 
            }
            unlink "$app.sorted";
            unlink $acd;
        }
        unlink $app;
    }

exit(0);

###################################################################
# Name:		parse
# Function:	Gets history data from log files.
#		Produces intermediate data file.
# Args:		
#		file - file name to parse
# Returns:	parsed data returned in 'data'
###################################################################
sub parse {
    my ($file) = @_;
    my $line;
    my $desc;
    my $outfile = "";	# initialise it to blank to indicate we have no open files

    open (LOG, "< $file") || die "Couldn't open file $file";

    while ($line = <LOG>) {
# get the name of the next output file
        if ($line =~ "RCS file:") {
            ($outfile, $extension) = ($line =~ /RCS file: \/home\/repository\/emboss\/emboss\/emboss\/emboss\/(\S+)\.(\S)/);
# close previous file
            if ($outfile ne "") {
                close (OUT);
            }
# open new file
#print "ext=$extension\n";
            if ($extension eq "c") {	# .c file
#print "open $outfile.applog\n";
                open (OUT, "> $incdir/$outfile.applog");
		$extra = "Program:";
            } else {			# .acd file
                $outfile =~ s/acd\///;	# remove the 'acd/' from the file name
#print "open $outfile.acdlog\n";
                open (OUT, "> $incdir/$outfile.acdlog");
		$extra = "ACD file:";
            }
        }
        if ($line =~ "date") {
            ($date, $author) = ($line =~ /\S+\s+(\S+)\s+\S+\s+\S+\s+(\S+);/);

            $desc = <LOG>;
            chomp $desc;
# change any ampersands or angle brackets to HTML codes
            $desc =~ s/&/&amp;/g;
            $desc =~ s/</&lt;/g;
            $desc =~ s/>/&gt;/g;

# write data out
	    print OUT "$date\t$author\t$extra $desc\n"; 
        }

    }

    close (OUT);
    close (LOG);

}



