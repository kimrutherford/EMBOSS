#!/usr/bin/perl -w

sub filecopyc($$) {
    my($infile, $outfile) = @_;

    my $text = "";

    print "'$infile' => '$outfile'\n";
    open (IN, "$infile" || die "Cannot find '$infile'");
    while(<IN>) {$text .= $_}
    close IN;

    $text =~ s/AjPWxyz/AjP$newtype/g;
    $text =~ s/AjSWxyz/AjS$newtype/g;
    $text =~ s/AjOWxyz/AjO$newtype/g;
    $text =~ s/Wxyz(Data)/$newtype$1/g;
    $text =~ s/ajWxyz/aj$newtype/g;
    $text =~ s/Wxyzdesc/$newdescup/g;
    $text =~ s/wxyzdesc/$newdesc/g;
    $text =~ s/Wxyz/$newtype/g;
    $text =~ s/wxyz/$newname/g;
    if($newdescup =~ /^[AEUIOY]/){
	$text =~ s/a $newdesc/an $newdesc/g;
    }

    if(-e "$outfile") {die "$outfile exists"}
    open(OUT, ">$outfile" || die "Cannot open '$outfile'");
    print OUT $text;
    close OUT;

    return 0;
}

sub filecopyh($$) {
    my($infile, $outfile) = @_;

    my $text = "";

    print "'$infile' => '$outfile'\n";
    open (IN, "$infile" || die "Cannot find '$infile'");
    while(<IN>) {$text .= $_}
    close IN;

    $text =~ s/AjPWxyz/AjP$newtype/g;
    $text =~ s/AjSWxyz/AjS$newtype/g;
    $text =~ s/AjOWxyz/AjO$newtype/g;
    $text =~ s/Wxyz(Data)/$newtype$1/g;
    $text =~ s/ajWxyz/aj$newtype/g;
    $text =~ s/Wxyzdesc/$newdescup/g;
    $text =~ s/wxyzdesc/$newdesc/g;
    $text =~ s/Wxyz/$newtype/g;
    $text =~ s/wxyz/$newname/g;
    if($newdescup =~ /^[AEUIOY]/){
	$text =~ s/a $newdesc/an $newdesc/g;
    }

    if(-e "$outfile") {die "$outfile exists"}
    open(OUT, ">$outfile" || die "Cannot open '$outfile'");
    print OUT $text;
    close OUT;

    return 0;
}

#$newname = "abcd";
#$newtype = "Abcd"; # ucfirst
#$newdesc = "abcd"; # same name or longer (alignment, sequence, assembly)

foreach $opt (@ARGV) {
    if ($opt =~ /name=(\S+)/) {$newname=$1}
    elsif ($opt =~ /type=(\S+)/) {$newtype=ucfirst($1)}
    elsif ($opt =~ /desc=(\S+)/) {$newdesc=$1}
    else {print STDERR "Unnkown option '$opt'\n"}
}

if(!defined($newname)){die "name not defined"}
if(!defined($newtype)){$newtype = ucfirst($newname)}
if(!defined($newdesc)){$newdesc = $newname}
if(!defined($newcaps)){$newuapc = uc($newname)}

$newdescup = ucfirst($newdesc);

chdir;

chdir "devemboss/ajax/core";

$infile = "ajwxyzdata.header";
$outfile = "aj$newname"."data.h";

filecopyh($infile, $outfile);

$infile = "ajwxyz".".header";
$outfile = "aj$newname".".h";

filecopyh($infile, $outfile);

$infile = "ajwxyz"."read.header";
$outfile = "aj$newname"."read.h";

filecopyh($infile, $outfile);

$infile = "ajwxyz"."write.header";
$outfile = "aj$newname"."write.h";

filecopyh($infile, $outfile);

$infile = "ajwxyz".".source";
$outfile = "aj$newname".".c";

filecopyc($infile, $outfile);

$infile = "ajwxyz"."read.source";
$outfile = "aj$newname"."read.c";

filecopyc($infile, $outfile);

$infile = "ajwxyz"."write.source";
$outfile = "aj$newname"."write.c";

filecopyc($infile, $outfile);


#In each file, a standard set of changes
# implemented by reading all into text and then replacing strings throughout
# and testing for lines too long or short in the @func and @data records

# edit ajax.h

# add the 4 core/ajwxyz*.h files in an appropriate places - 
# ajwxyzdata.h after the other data files (before a comment)
# the others together after ajassem*.h (before a comment)


# edit ajutil.c ajReset function

# edit Makefile.am
# add new lines for the .h and .c files

chdir "../ajaxdb";

$infile = "ajwxyz"."db.header";
$outfile = "aj$newname"."db.h";

filecopyh($infile, $outfile);

$infile = "ajwxyz"."db.source";
$outfile = "aj$newname"."db.c";

filecopyc($infile, $outfile);



# edits

# edit Makefile.am

chdir "../../nucleus";

# edit embinit.c
# 2 places to add ajWxyzdbInit();


print "
New datatype $newtype defined.

(1) Edit ajax/core/ajax.h

Insert line in /* input and output data types */:

include \"aj$newname"."data.h\"

Insert lines in /* input and output data types I/O */:

#include \"aj$newname".".h\"
#include \"aj$newname"."read.h\"
#include \"aj$newname"."write.h\"

(2) Edit ajax/core/ajnam.c

Add to namDbTypes

    {\"$newtype\",        \"$newname\",  \"$newdescup\",
    AJDATATYPE_$newcaps"."},

Add new datatype wherever obo appears:

in namAccessTest
        else if(ajCharMatchC(namtype->Scope, \"$newname\"))
            result = aj$newtype"."accessMethodTest(method);

in namInformatTest

    else if(ajCharMatchC(namtype->Scope, \"$newname\"))
        result = aj$newtype"."informatTest(format);

in namMethod2Scope

        else if(ajCharMatchC(namtype->Scope, \"$newname\"))
            result = aj$newtype"."accessMethodGetScope(method);


(3) Edit ajax/core/ajutil.c

Add to ajReset

    aj$newtype"."Exit();

(4) Edit nucleus/emboss.h

Insert:

#include \"aj$newname"."db.h\"

(5) Edit nucleus/embinit.c

Insert in initDball,

    aj$newtype"."dbInit();

(6) Edit nucleus/embexit.c

Insert in embExit,

    aj$newtype"."dbExit();

(7) Edit ajacd.h and ajacd.c

Add datatype and functions, following e.g. taxon definitions
";
