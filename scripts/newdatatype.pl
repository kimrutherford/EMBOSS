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
    $text =~ s/WXYZ/$newcaps/g;
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

    $text =~ s/AJWXYZ/AjP$newtypeup/g;
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
    elsif ($opt =~ /desc=(.+)/) {$newdesc=$1}
    else {print STDERR "Unknown option '$opt'\n"}
}

if(!defined($newname)){die "name not defined"}
if(!defined($newtype)){$newtype = ucfirst($newname);$newtypeup = uc($newname);}
if(!defined($newdesc)){$newdesc = $newname}
if(!defined($newcaps)){$newcaps = uc($newname)}

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

Add:

#include \"aj$newname"."read.h\"

Add to namDbTypes

    {\"$newtype\",        \"$newname\",  \"$newdescup\",
    AJDATATYPE_$newcaps"."},

Add new datatype wherever obo appears:

in namAccessTest
        else if(namtype->DataType == AJDATATYPE_$newcaps)
            result = aj$newtype"."accessMethodTest(method);

in namInformatTest

    else if(namtype->DataType == AJDATATYPE_$newcaps)
        result = aj$newtype"."informatTest(format);

in namMethod2Qlinks

        else if(namtype->DataType == AJDATATYPE_$newcaps)
            result = aj$newtype"."accessMethodGetQlinks(method);


in namMethod2Scope

        else if(namtype->DataType == AJDATATYPE_$newcaps)
            result = aj$newtype"."accessMethodGetScope(method);


in namDatatype2Fields

        else if(namtype->DataType == AJDATATYPE_$newcaps)
            result = aj$newtype"."inTypeGetFields(method);


in namDatatype2Qlinks

        else if(namtype->DataType == AJDATATYPE_$newcaps)
            result = aj$newtype"."inTypeGetQlinks(method);


(3) Edit ajax/core/ajfile.h

Edit AjEOutfileType and add

    ** \@value $newdescup data

    ajEOutfileType$newcaps,

(4) Edit ajax/core/ajquerydata.h

Add to AjEDataType

    AJDATATYPE_$newcaps, 

(5) Edit ajax/core/ajutil.c

Add

#include \"aj$newname".".h\"

Add to ajReset

    aj$newtype"."Exit();

(6) Edit ajax/core/Makefile.am

Add:

aj$newname".".c \
aj$newname"."read.c \
aj$newname"."write.c \

Add:
aj$newname".".h \
aj$newname"."data.h \
aj$newname"."read.h \
aj$newname"."write.h \

(7) Edit ajax/ajaxdb/Makefile.am

Add:

aj$newname"."db.c \

aj$newname"."db.h \

(8) Edit nucleus/emboss.h

Insert:

#include \"aj$newname"."db.h\"

(9) Edit nucleus/embinit.c

Insert:

#include \"aj$newname"."db.h\"

Insert in initDball,

    aj$newtype"."dbInit();

(10) Edit nucleus/embexit.c

Insert:

#include \"aj$newname"."db.h\"

Insert in embExit,

    aj$newtype"."dbExit();

(11) Edit ajacd.h

Add

#include \"aj$newname"."data.h\"

Add datatype and functions, following e.g. taxon definitions

AjP$newtype   ajAcdGet$newtype(const char *token);
AjP$newtype"."all   ajAcdGet$newtype"."all(const char *token);
AjPOutfile    ajAcdGetOut$newtype(const char *token);


(12) Edit ajacd.c

Add

#include \"aj$newname"."read.h\"
#include \"aj$newname"."write.h\"

Add datatype and functions, following e.g. taxon definitions

static const AjPStr acdPrompt$newtype(AcdPAcd thys);
static const AjPStr acdPromptOut$newname(AcdPAcd thys);
static void acdHelpText$newtype(const AcdPAcd thys, AjPStr* str);
static void acdDel$newtype(void** PPval);
static void acdSet$newtype(AcdPAcd thys);
static void acdSetOut$newname(AcdPAcd thys);


AcdOAttr acdAttr$newtype"."[] =
{
    {\"entry\", VT_BOOL, AJFALSE, \"N\",
	 \"Read whole entry text\"},
    {\"minreads\", VT_INT, AJFALSE, \"1\",
	 \"Minimum number of inputs\"},
    {\"maxreads\", VT_INT, AJFALSE, \"(INT_MAX)\",
	 \"Maximum number of inputs\"},
    {\"nullok\", VT_BOOL, AJFALSE, \"N\",
	 \"Can accept a null filename as 'no file'\"},
    {NULL, VT_NULL, AJFALSE, NULL,
	 NULL}
};

AcdOAttr acdAttrOut$newname"."[] =
{
    {\"name\", VT_STR, AJFALSE, \"\",
	 \"Default file name\"},
    {\"extension\", VT_STR, AJFALSE, \"\",
	 \"Default file extension\"},
    {\"nulldefault\", VT_BOOL, AJFALSE, \"N\",
	 \"Defaults to 'no file'\"},
    {\"nullok\", VT_BOOL, AJFALSE, \"N\",
	 \"Can accept a null filename as 'no file'\"},
    {NULL, VT_NULL, AJFALSE, NULL,
	 NULL}
};



In AcdOOuttype acdOuttype[] =

    {\"out$newname\",        \"abc\",          ajEOutfileType$newname, 0,
	 acdPromptOut$newname,         aj$newtype"."outformatFind},



AcdOQual acdQual$newtype"."[] =
{
    {\"iformat\",    \"\",  \"string\",  \"Input $newdesc format\"},
    {\"iquery\",     \"\",  \"string\",  \"Input query fields or ID list\"},
    {\"ioffset\",    \"\",  \"integer\", \"Input start position offset\"},
    {\"idbname\",    \"\",  \"string\",  \"User-provided database name\"},
    {NULL, NULL, NULL, NULL}
};


AcdOQual acdQualOut$newname"."[] =
{
    {\"odirectory\", \"\",  \"string\",  \"Output directory\"},
    {\"oformat\",    \"\",  \"string\",  \"$newdescup output format\"},
    {NULL, NULL, NULL, NULL}
};


    {\"$newname\",       \"input\",     acdSecInput,
	 acdAttr$newtype,       acdQual$newtype,
	 acdSet$newtype,        acdHelpText$newtype,   acdDel$newtype,
	 AJTRUE,  AJTRUE,  acdPrompt$newtype,  &acdUseData, &acdUseIn,
	 \"$newdescup\" },

    {\"out$newname\",     \"output\",    acdSecOutput,
	 acdAttrOut$newname,    acdQualOut$newname,
	 acdSetOut$newname,     NULL,                  acdDelOutfile,
	 AJTRUE,  AJTRUE,  acdPromptOut$newname,  &acdUseOutfile, &acdUseOut,
	 \"$newdescup\" },

in section return

** \@nam4rule  Get$newtype   ACD $newdesc datatype
** \@nam4rule  Get$newtype"."all   ACD $newdesc stream datatype
** \@nam4rule  GetOut$newname   ACD $newdesc output datatype
** \@valrule   $newtype         [AjP$newtype]
** \@valrule   $newtype"."all         [AjP$newtype"."all]
** \@valrule   Out$newname      [AjPOutfile]


/* \@func ajAcdGet$newtype *****************************************************
**
** Returns an item of type $newdesc as defined in a named ACD item.
** Called by the application after all ACD values have been set,
** and simply returns what the ACD item already has.
**
** \@param [r] token [const char*] Text token name
** \@return [AjP$newtype] $newdescup object
** \@cre failure to find an item with the right name and type aborts.
**
** \@release 6.6.0
** \@@
******************************************************************************/

AjP$newtype ajAcdGet$newtype(const char *token)
{
    AjP$newtype val = acdGetValueRef(token, \"$newname\");

    return val;
}




/* \@funcstatic acdSet$newtype *************************************************
**
** Using the definition in the ACD file, and any values for the
** item or its associated qualifiers provided on the command line,
** prompts the user if necessary (and possible) and
** sets the actual value for an ACD $newdesc input
**
** Understands all attributes and associated qualifiers for this item type.
**
** \@param [u] thys [AcdPAcd] ACD item.
** \@return [void]
**
** \@release 6.6.0
** \@\@
******************************************************************************/

static void acdSet$newtype(AcdPAcd thys)
{
    AjP$newtype val;
    AjP$newtype"."in $newname"."in;

    AjBool required = ajFalse;
    AjBool ok       = ajFalse;
    AjBool nullok   = ajFalse;
    ajint itry;

    AjPStr infname = NULL;
    
    val = aj$newtype"."New();        /* set the default value */
    $newname"."in = aj$newtype"."inNew();  

    acdAttrToBool(thys, \"nullok\", ajFalse, &nullok);

    required = acdIsRequired(thys);
    acdReplyInitS(thys, infname, &acdReplyDef);
    ajStrDel(&infname);

    acdAttrToBool(thys, \"entry\", ajFalse, &$newname"."in->Input->Text);

    for(itry=acdPromptTry; itry && !ok; itry--)
    {
	ok = ajTrue;	   /* accept the default if nothing changes */

	ajStrAssignS(&acdReply, acdReplyDef);

	if(required)
	    acdUserGet(thys, &acdReply);

	if(!ajStrGetLen(acdReply) && nullok)
        {
            aj$newtype"."inDel(&$newname"."in);
	    break;
	}

	aj$newtype"."inQryS($newname"."in, acdReply);

        ok = aj$newtype"."inRead($newname"."in, val);
    }

    if(!ok)
	acdBadRetry(thys);

    acdInFileSave(acdReply, aj$newtype"."GetId(val), ajTrue);

    thys->Value = val;
    ajStrAssignS(&thys->ValStr, acdReply);
    
    aj$newtype"."inDel(&$newname"."in);

    return;
}



/* \@func ajAcdGetOut$newname **************************************************
**
** Returns an item of type Out$newname as defined in a named ACD item.
** Called by the application after all ACD values have been set,
** and simply returns what the ACD item already has.
**
** \@param [r] token [const char*] Text token name
** \@return [AjPOutfile] File object. The file was already opened by
**         acdSetOut so this just returns the pointer.
** \@cre failure to find an item with the right name and type aborts.
**
** \@release 6.6.0
** \@\@
******************************************************************************/

AjPOutfile ajAcdGetOut$newname(const char *token)
{
    return acdGetValueRef(token, \"out$newname"."\");
}




/* \@funcstatic acdSetOut$newname **********************************************
**
** Using the definition in the ACD file, and any values for the
** item or its associated qualifiers provided on the command line,
** prompts the user if necessary (and possible) and
** sets the actual value for an ACD out$newname item.
**
** Understands all attributes and associated qualifiers for this item type.
**
** The default value (if no other available) is a null string, which
** is invalid.
**
** Associated qualifiers \"-oformat\"
** are stored in the object and applied to the data on output.
**
** \@param [u] thys [AcdPAcd] ACD item.
** \@return [void]
**
** \@release 6.6.0
** \@\@
******************************************************************************/

static void acdSetOut$newname(AcdPAcd thys)
{
    acdSetOutType(thys, \"out$newname"."\");

    return;
}




/* \@funcstatic acdHelpText$newtype ********************************************
**
** Sets the help text for this ACD object to be a $newdesc description
**
** \@param [r] thys [const AcdPAcd] Current ACD object.
** \@param [w] Pstr [AjPStr*] Help text
** \@return [void]
**
** \@release 6.6.0
** \@\@
******************************************************************************/

static void acdHelpText$newtype(const AcdPAcd thys, AjPStr* Pstr)
{
    ajint maxreads;

    ajStrAssignClear(Pstr);

    acdAttrToInt(thys, \"maxreads\", 1, &maxreads);

    if(maxreads <= 1)
        ajStrAssignC(Pstr, \"$newname\");
    else
        ajStrAssignC(Pstr, \"$newname(s)\");

    ajStrFmtTitle(Pstr);

    ajStrAppendC(Pstr,
		 \" filename and optional format, or reference (input query)\");

    return;
}




/* \@funcstatic acdPrompt$newtype **********************************************
**
** Sets the default prompt for this ACD object to be a $newdesc
** prompt with \"first\", \"second\" etc. added.
**
** \@param [u] thys [AcdPAcd] Current ACD object.
** \@return [const AjPStr] Generated standard prompt
**
** \@release 6.6.0
** \@\@
******************************************************************************/

static const AjPStr acdPrompt$newtype"."(AcdPAcd thys)
{
    static ajint count=0;
    AjPStr $newname"."Prompt = NULL;
    AjPStr $newname"."PromptAlt = NULL;
    const AjPStr knowntype;
    ajint maxreads = 0;

    acdAttrToInt(thys, \"maxreads\", 1, &maxreads);

    knowntype = acdKnowntypeDesc(thys);

    $newname"."Prompt = ajStrNewRes(32);
    $newname"."PromptAlt = ajStrNewRes(32);

    if(ajStrGetLen(knowntype))
	ajFmtPrintAppS(&$newname"."PromptAlt, \"\%S \", knowntype);

    ajFmtPrintAppS(&$newname"."PromptAlt, \"$newdesc\");

    if(maxreads > 1)
	ajStrAppendC(&$newname"."PromptAlt, \"(s)\");

    ajFmtPrintS(&$newname"."Prompt, \"Input %S\", $newname"."PromptAlt);

    if(knowntype)
    {
	count++;
	acdPromptStandardS(thys, $newname"."PromptAlt);
    }
    else
	acdPromptStandardAlt(thys, ajStrGetPtr($newname"."Prompt),
			     ajStrGetPtr($newname"."PromptAlt), &count);

    if(!acdAttrTestDefined(thys, \"default\") &&
       acdAttrTestDefined(thys, \"nullok\"))
	acdPromptStandardAppend(thys, \" (optional)\");
    
    ajStrDel(&$newname"."Prompt);
    ajStrDel(&$newname"."PromptAlt);

    return thys->StdPrompt;
}



/* \@funcstatic acdPromptOut$newname"." ****************************************
**
** Sets the default prompt for this ACD object to be a simple
** prompt with \"second\", \"third\" etc. added.
**
** \@param [u] thys [AcdPAcd] Current ACD object.
** \@return [const AjPStr] Generated standard prompt
**
** \@release 6.6.0
** \@\@
******************************************************************************/

static const AjPStr acdPromptOut$newname"."(AcdPAcd thys)
{
    static ajint count=0;

    const AjPStr knowntype;

    knowntype = acdKnowntypeDesc(thys);

    if(ajStrGetLen(knowntype))
    {
	count++;
	acdPromptStandardS(thys, knowntype);
	acdPromptStandardAppend(thys, \" output file\");
    }
    else
	acdPromptStandard(thys, \"$newdesc output file\", &count);

    if(!acdAttrTestDefined(thys, \"default\") &&
       acdAttrTestDefined(thys, \"nullok\"))
	acdPromptStandardAppend(thys, \" (optional)\");
    
    return thys->StdPrompt;
}




/* \@funcstatic acdDel$newtype"." **********************************************
**
** Function with void** prototype to delete ACD $newdesc input data
**
** \@param [d] PPval [void**] Value to be deleted
** \@return [void]
**
** \@release 6.6.0
**
******************************************************************************/

static void acdDel$newtype(void** PPval)
{
    if(!*PPval)
        return;

    aj$newtype"."Del((AjP$newtype"."*)PPval);

    return;
}






";
