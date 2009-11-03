/******************************************************************************
** @source AJAX seqtype functions
**
** @author Copyright (C) 2002 Peter Rice
** @version 1.0
** @@
**
** This library is free software; you can redistribute it and/or
** modify it under the terms of the GNU Library General Public
** License as published by the Free Software Foundation; either
** version 2 of the License, or (at your option) any later version.
**
** This library is distributed in the hope that it will be useful,
** but WITHOUT ANY WARRANTY; without even the implied warranty of
** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
** Library General Public License for more details.
**
** You should have received a copy of the GNU Library General Public
** License along with this library; if not, write to the
** Free Software Foundation, Inc., 59 Temple Place - Suite 330,
** Boston, MA  02111-1307, USA.
******************************************************************************/

#include "ajax.h"




/* @datastatic SeqPType *******************************************************
**
** Sequence types data structure, used to test input sequence against
** a defined sequence type
**
** @alias SeqSType
** @alias SeqOType
**
** @attr Name [const char*] sequence type name
** @attr Gaps [AjBool] allow gap characters
** @attr Ambig [AjBool] True if ambiguity codes are allowed
** @attr Type [ajuint] enumerated ISANY=0 ISNUC=1 ISPROT=2 
** @attr Padding [ajint] Padding to alignment boundary
** @attr ConvertFrom [const char*] Convert each of these characters to the
**                           ConvertTo equivalent
** @attr ConvertTo [const char*] Equivalent for each sequence character in
**                         ConvertFrom
** @attr Badchars [(AjPRegexp*)] Test function
** @attr Goodchars [(AjPStr*)] Test function
** @attr Desc [const char*] Description for documentation purposes
** @@
******************************************************************************/

typedef struct SeqSType
{
    const char *Name;
    AjBool Gaps;
    AjBool Ambig;
    ajuint Type;
    ajint Padding;
    const char *ConvertFrom;
    const char *ConvertTo;
    AjPRegexp (*Badchars) (void);
    AjPStr (*Goodchars) (void);
    const char *Desc;
} SeqOType;

#define SeqPType SeqOType*




enum ProtNuc {ISANY=0, ISNUC=1, ISPROT=2};

static char* seqNewGapChars = NULL;



/*
** gaps only allowed if it says so
** gap conversion is a separate attribute, along with case convserion
*/

static AjBool     seqFindType(const AjPStr type_name, ajint* typenum);
static void       seqGapSL(AjPStr* seq, char gapc, char padc, ajuint ilen);
static AjBool     seqTypeFix(AjPSeq thys, ajint itype);
static AjBool     seqTypeFixReg(AjPSeq thys, ajint itype, char fixchar);
static void       seqTypeSet(AjPSeq thys, const AjPStr Type);
static AjBool     seqTypeStopTrimS(AjPStr* pthys);
static char       seqTypeTest(const AjPStr thys, AjPRegexp badchars);
static AjBool     seqTypeTestI(AjPSeq thys, ajint itype);
static char       seqTypeTestS(const AjPStr thys, const AjPStr goodchars);

static AjPRegexp  seqTypeCharAny(void);
static AjPRegexp  seqTypeCharAnyGap(void);
static AjPRegexp  seqTypeCharNuc(void);
static AjPRegexp  seqTypeCharNucGap(void);
static AjPRegexp  seqTypeCharNucGapPhylo(void);
static AjPRegexp  seqTypeCharNucPure(void);
static AjPRegexp  seqTypeCharProt(void);
static AjPRegexp  seqTypeCharProtGap(void);
static AjPRegexp  seqTypeCharProtGapPhylo(void);
static AjPRegexp  seqTypeCharProtPure(void);
static AjPRegexp  seqTypeCharProtStop(void);
static AjPRegexp  seqTypeCharProtStopGap(void);

static AjPStr  seqTypeStrAny(void);
static AjPStr  seqTypeStrAnyGap(void);
static AjPStr  seqTypeStrDnaGap(void);
static AjPStr  seqTypeStrNuc(void);
static AjPStr  seqTypeStrNucGap(void);
static AjPStr  seqTypeStrNucGapPhylo(void);
static AjPStr  seqTypeStrNucPure(void);
static AjPStr  seqTypeStrProt(void);
static AjPStr  seqTypeStrProtAny(void);
static AjPStr  seqTypeStrProtGap(void);
static AjPStr  seqTypeStrProtGapPhylo(void);
static AjPStr  seqTypeStrProtPure(void);
static AjPStr  seqTypeStrProtStop(void);
static AjPStr  seqTypeStrProtStopGap(void);
static AjPStr  seqTypeStrRnaGap(void);

static AjPRegexp seqtypeRegAny          = NULL;
static AjPRegexp seqtypeRegAnyGap       = NULL;
static AjPRegexp seqtypeRegDnaGap       = NULL;
static AjPRegexp seqtypeRegNuc          = NULL;
static AjPRegexp seqtypeRegNucGap       = NULL;
static AjPRegexp seqtypeRegNucGapPhylo  = NULL;
static AjPRegexp seqtypeRegNucPure      = NULL;
static AjPRegexp seqtypeRegProt         = NULL;
static AjPRegexp seqtypeRegProtAny      = NULL;
static AjPRegexp seqtypeRegProtGap      = NULL;
static AjPRegexp seqtypeRegProtGapPhylo = NULL;
static AjPRegexp seqtypeRegProtPure     = NULL;
static AjPRegexp seqtypeRegProtStop     = NULL;
static AjPRegexp seqtypeRegProtStopGap  = NULL;
static AjPRegexp seqtypeRegRnaGap       = NULL;

static AjPStr seqtypeCharsetAny          = NULL;
static AjPStr seqtypeCharsetAnyGap       = NULL;
static AjPStr seqtypeCharsetDnaGap       = NULL;
static AjPStr seqtypeCharsetNuc          = NULL;
static AjPStr seqtypeCharsetNucGap       = NULL;
static AjPStr seqtypeCharsetNucGapPhylo  = NULL;
static AjPStr seqtypeCharsetNucPure      = NULL;
static AjPStr seqtypeCharsetProt         = NULL;
static AjPStr seqtypeCharsetProtAny      = NULL;
static AjPStr seqtypeCharsetProtGap      = NULL;
static AjPStr seqtypeCharsetProtGapPhylo = NULL;
static AjPStr seqtypeCharsetProtPure     = NULL;
static AjPStr seqtypeCharsetProtStop     = NULL;
static AjPStr seqtypeCharsetProtStopGap  = NULL;
static AjPStr seqtypeCharsetRnaGap       = NULL;




/*
** gap characters known are:
**
** . GCG and most others
** - Phylip and some alignment output
** ~ GCG for gaps at ends
** * Staden for DNA but stop for protein (fix on input?)
** O Phylip (fix on input?) - no longer possible: O is pyrrolysine in proteins
*/



/*
char seqCharProt[]  = "ACDEFGHIKLMNPQRSTVWYacdefghiklmnpqrstvwyBUXZbuxz*?";
*/
char seqCharProtPure[]  = "ACDEFGHIKLMNPQRSTVWY";
char seqCharProtAmbig[] = "BJOUXZ?"; /* convert unwanted ones to Xx */
char seqCharProtStop[]  = "*";
char seqCharNuc[]       = "ACGTUBDHKMNRSVWXY?";
char seqCharNucPure[]   = "ACGTU";
char seqCharNucAmbig[]  = "BDHKMNRSVWXY?";
char seqCharGap[]       = ".~-";	/* phylip used O in old versions */
char seqCharNucDna[]    = "ACGTBDHKMNRSVWXY?";
char seqCharNucRna[]    = "ACGUBDHKMNRSVWXY?";
char seqCharGapany[]    = ".~-";	/* phylip used O in old versions*/
char seqCharGapdash[]   = "-";
char seqCharGapdot[]    = ".";
char seqGap = '-';		/* the (only) EMBOSS gap character */
char seqCharGapTest[]   = " .~-";   /* phylip used O - don't forget space */
char seqCharPhylo[]       = "?";	/* phylip uses ? for unknown or gap */




/* @funclist seqType **********************************************************
**
** Functions to test each sequence type
**
******************************************************************************/

static SeqOType seqType[] =
{
/*   "name"            Gaps     Ambig    Type    Padding CvtFrom CvtTo
         BadcharsFunction GoodCharsFunction Description */
    {"any",            AJFALSE, AJTRUE,  ISANY,  0, "?",    "X",
	 seqTypeCharAny,
	 seqTypeStrAny,
	 "any valid sequence"},		/* reset type */
    {"gapany",         AJTRUE,  AJTRUE,  ISANY,  0, "?",    "X",
	 seqTypeCharAnyGap,
	 seqTypeStrAnyGap,
	 "any valid sequence with gaps"}, /* reset type */
    {"dna",            AJFALSE, AJTRUE,  ISNUC,  0, "?XxUu", "NNnTt",
	 seqTypeCharNuc,
	 seqTypeStrNuc,
	 "DNA sequence"},
    {"puredna",        AJFALSE, AJFALSE, ISNUC,  0, "Uu", "Tt",
	 seqTypeCharNucPure,
	 seqTypeStrNucPure,
	 "DNA sequence, bases ACGT only"},
    {"gapdna",         AJTRUE,  AJTRUE,  ISNUC,  0, "?XxUu", "NNnTt",
	 seqTypeCharNucGap,
	 seqTypeStrNucGap,
	 "DNA sequence with gaps"},
    {"gapdnaphylo",    AJTRUE,  AJTRUE,  ISNUC,  0, "Uu",  "Tt",
	 seqTypeCharNucGapPhylo,
	 seqTypeStrNucGapPhylo,
	 "DNA sequence with gaps and queries"},
    {"rna",            AJFALSE, AJTRUE,  ISNUC,  0, "?XxTt", "NNnUu",
	 seqTypeCharNuc,
	 seqTypeStrNuc,
	 "RNA sequence"},
    {"purerna",        AJFALSE, AJFALSE, ISNUC,  0, "Tt", "Uu",
	 seqTypeCharNucPure,
	 seqTypeStrNucPure,
	 "RNA sequence, bases ACGU only"},
    {"gaprna",         AJTRUE,  AJTRUE,  ISNUC,  0, "?XxTt", "NNnUu",
	 seqTypeCharNucGap,
	 seqTypeStrNucGap,
	 "RNA sequence with gaps"},
    {"gaprnaphylo",     AJTRUE,  AJTRUE,  ISNUC, 0, "Tt",  "Uu",
	 seqTypeCharNucGapPhylo,
	 seqTypeStrNucGapPhylo,
	 "RNA sequence with gaps and queries"},
    {"nucleotide",     AJFALSE, AJTRUE,  ISNUC,  0, "?Xx",   "NNn",
	 seqTypeCharNuc,
	 seqTypeStrNuc,
	 "nucleotide sequence"},
    {"purenucleotide", AJFALSE, AJFALSE, ISNUC,  0, NULL,  NULL,
	 seqTypeCharNucPure,
	 seqTypeStrNucPure,
	 "nucleotide sequence, bases ACGTU only"},
    {"gapnucleotide",  AJTRUE,  AJTRUE,  ISNUC,  0, "?Xx",   "NNn",
	 seqTypeCharNucGap,
	 seqTypeStrNucGap,
	 "nucleotide sequence with gaps"},
    {"gapnucleotidephylo",  AJTRUE,  AJTRUE,  ISNUC,  0, NULL,  NULL,
	 seqTypeCharNucGapPhylo,
	 seqTypeStrNucGapPhylo,
	 "nucleotide sequence with gaps and queries"},
    {"gapnucleotidesimple",AJTRUE, AJTRUE , ISNUC,  0,
                     "BbDdHhKkMmRrSsVvWwXxYy?", "NnNnNnNnNnNnNnNnNnNnNnN",
	 seqTypeCharNucGap,
	 seqTypeStrNucGap,
	 "nucleotide sequence with gaps but only N for ambiguity"},
    {"protein",        AJFALSE, AJTRUE,  ISPROT, 0, "?*",  "XX",
	 seqTypeCharProt,
	 seqTypeStrProt,
	 "protein sequence"},
    {"pureprotein",    AJFALSE, AJFALSE, ISPROT, 0, NULL,  NULL,
	 seqTypeCharProtPure,
	 seqTypeStrProtPure,
	 "protein sequence without BZ U X or *"},
    {"stopprotein",    AJFALSE, AJTRUE,  ISPROT, 0, "?",   "X",
	 seqTypeCharProtStop,
	 seqTypeStrProtStop,
	 "protein sequence with possible stops"},
    {"gapprotein",     AJTRUE,  AJTRUE,  ISPROT, 0, "?*",  "XX",
	 seqTypeCharProtGap,
	 seqTypeStrProtGap,
	 "protein sequence with gaps"},
    {"gapstopprotein", AJTRUE,  AJTRUE,  ISPROT, 0, "?",  "X",
	 seqTypeCharProtStopGap,
	 seqTypeStrProtStopGap,
	 "protein sequence with gaps and possible stops"},
    {"gapproteinphylo", AJTRUE,  AJTRUE,  ISPROT, 0, NULL,  NULL,
	 seqTypeCharProtGapPhylo,
	 seqTypeStrProtGapPhylo,
	 "protein sequence with gaps, stops and queries"},
    {"proteinstandard",AJFALSE, AJTRUE,  ISPROT, 0, "?*UuJjOo", "XXXxXxXx",
	 seqTypeCharProt,
	 seqTypeStrProt,
	 "protein sequence with no selenocysteine"},
    {"stopproteinstandard",AJFALSE, AJTRUE, ISPROT, 0, "?UuJjOo", "XXxXxXx",
	 seqTypeCharProtStop,
	 seqTypeStrProtStop,
	 "protein sequence with a possible stop but no selenocysteine"},
    {"gapproteinstandard", AJTRUE,  AJTRUE, ISPROT, 0, "?*UuJjOo", "XXXxXxXx",
	 seqTypeCharProtGap,
	 seqTypeStrProtGap,
	 "protein sequence with gaps but no selenocysteine"},
    {"gapproteinsimple", AJTRUE,  AJTRUE, ISPROT, 0,
                                              "?*BbZzUuJjOo", "XXXxXxXxXxXx",
	 seqTypeCharProtGap,
	 seqTypeStrProtGap,
	 "protein sequence with gaps but no selenocysteine"},
    {NULL,             AJFALSE, AJTRUE,  ISANY,  0, NULL,  NULL,
	 NULL,
	 NULL,
	 NULL}
};




/* @funcstatic seqTypeTestI ***************************************************
**
** Tests the type of a sequence is compatible with a defined type.
** If the type can have gaps, also tests for gap characters.
** Used only for testing, so never writes any error message
**
** @param [u] thys [AjPSeq] Sequence object
** @param [r] itype [ajint] Sequence type index
** @return [AjBool] ajTrue if compatible.
** @@
******************************************************************************/

static AjBool seqTypeTestI(AjPSeq thys, ajint itype)
{

    /*
     ** We have a known type, now we need to either show the sequence
     ** matches it, or fix it so it does (or, of course, give up)
     */

    /*
     ** First we test the type - predefined by a database,
     ** or by checking the sequence characters
     */

    if(seqType[itype].Gaps)
    {
	ajDebug("Convert gaps to '-'\n");
	ajSeqGap(thys, seqGap, 0);
    }
    else
    {
	ajDebug("Remove all gaps\n");
	ajStrRemoveGap(&thys->Seq);
    }

    if(seqType[itype].Type == ISPROT && !ajSeqIsProt(thys))
    {
	ajDebug("Sequence is not a protein\n");
	return ajFalse;
    }

    if(seqType[itype].Type == ISNUC && !ajSeqIsNuc(thys))
    {
	ajDebug("Sequence is not nucleic\n");
	return ajFalse;
    }

    if(ajStrIsCharsetCaseS(thys->Seq, seqType[itype].Goodchars()))
    {
	if(seqType[itype].ConvertFrom)
	{
	    ajDebug("Convert '%s' to '%s'\n",
		    seqType[itype].ConvertFrom,
		    seqType[itype].ConvertTo);
	    ajStrExchangeSetCC(&thys->Seq,
			   seqType[itype].ConvertFrom,
			   seqType[itype].ConvertTo);
	}
	return ajTrue;
    }

    ajDebug("seqTypeTestI: Sequence must be %s: found bad character\n",
	    seqType[itype].Desc);

    return ajFalse;
}




/* @funcstatic seqTypeFix *****************************************************
**
** Fixes (if possible) unacceptable sequence characters by removing gaps
** (if no gaps are allowed) and by setting ambiguity codes (if they
** are allowed).
**
** @param [u] thys [AjPSeq] Sequence object
** @param [r] itype [ajint] Sequence type index
** @return [AjBool] ajTrue if the type can be fixed
** @@
******************************************************************************/

static AjBool seqTypeFix(AjPSeq thys, ajint itype)
{
    ajDebug("seqTypeFix '%s' '%S'\n", seqType[itype].Name, thys->Seq);

    /*
     ** if ungapped, remove any gap characters
     */

    if(!seqType[itype].Gaps)
	ajStrRemoveGap(&thys->Seq);

    if (ajCharMatchC(seqType[itype].Name, "pureprotein"))
	seqTypeStopTrimS(&thys->Seq);

    if(seqType[itype].Ambig)
    {
	/*
	 ** list the bad characters, change to 'X' or 'N'
	 */
	switch(seqType[itype].Type)
	{
	case ISPROT:
	    if (ajCharMatchC(seqType[itype].Name, "protein"))
		seqTypeStopTrimS(&thys->Seq);
	    seqTypeFixReg(thys, itype, 'X');
	    break;
	case ISNUC:
	    seqTypeFixReg(thys, itype, 'N');
	    break;
	case ISANY:
	    if(ajSeqIsNuc(thys))
		seqTypeFixReg(thys, itype, 'N');
	    else
		seqTypeFixReg(thys, itype, 'X');
	    break;
	default:
	    ajDie("Unknown sequence type code for '%s'", seqType[itype].Name);
	    return ajFalse;
	}
    }

    if (ajCharMatchC(seqType[itype].Name, "pureprotein"))
	seqTypeStopTrimS(&thys->Seq);

    ajDebug("seqTypeFix done  '%S'\n", thys->Seq);
    return seqTypeTestI(thys, itype);
}




/* @funcstatic seqTypeFixReg **************************************************
**
** Fixes (if possible) unacceptable sequence characters by removing gaps
** (if no gaps are allowed) and by setting ambiguity codes (if they
** are allowed).
**
** @param [u] thys [AjPSeq] Sequence object
** @param [r] itype [ajint] Sequence type index
** @param [r] fixchar [char] Character to replace with
** @return [AjBool] ajTrue if the type can be fixed
** @@
******************************************************************************/

static AjBool seqTypeFixReg(AjPSeq thys, ajint itype, char fixchar)
{
    ajDebug("seqTypeFixReg '%s' '%S'\n", seqType[itype].Name, thys->Seq);
    /*ajDebug("Seq old '%S'\n", thys->Seq);*/

    return ajStrExchangeSetRestSK(&thys->Seq,
				  seqType[itype].Goodchars(), fixchar);
}




/* @funcstatic seqTypeSet *****************************************************
**
** Sets the sequence type. Uses the first character of the type
** which can be N or P
**
** @param [u] thys [AjPSeq] Sequence object
** @param [r] Type [const AjPStr] Sequence type
** @return [void]
** @@
******************************************************************************/

static void seqTypeSet(AjPSeq thys, const AjPStr Type)
{
    const char* cp;

    ajDebug("seqTypeSet '%S'\n", Type);

    cp = ajStrGetPtr(Type);

    switch(*cp)
    {
    case 'P':
    case 'p':
	ajSeqSetProt(thys);
	break;
    case 'N':
    case 'n':
	ajSeqSetNuc(thys);
	break;
    case '\0':
	break;
    default:
	ajDie("Unknown sequence type '%c'", *cp);
    }

    return;
}




/* @func ajSeqTypeCheckS ******************************************************
**
** Tests the type of a sequence is compatible with a defined type.
** If the type can have gaps, also tests for gap characters.
** Used for input validation - writes error message if the type check fails
**
** @param [u] pthys [AjPStr*] Sequence string
** @param [r] type_name [const AjPStr] Sequence type
** @return [AjBool] ajTrue if compatible.
** @@
******************************************************************************/

AjBool ajSeqTypeCheckS(AjPStr* pthys, const AjPStr type_name)
{
    /*    AjPStr tmpstr = NULL; */
    ajint itype = -1;

    /* ajDebug("ajSeqTypeCheckS type '%S' seq '%S'\n", type_name, *pthys); */

    if(!ajStrGetLen(type_name))	   /* nothing given - anything goes */
    {
	ajSeqGapS(pthys, seqGap);
	return ajTrue;
    }

    if(!seqFindType(type_name, &itype))
    {
	ajDie("Sequence type '%S' unknown", type_name);
	return ajFalse;
    }

    ajDebug("ajSeqTypeCheckS type '%s' found (%s)\n",
	    seqType[itype].Name, seqType[itype].Desc);

    if(seqType[itype].Gaps)
    {
	ajDebug("Convert gaps to '-'\n");
	ajSeqGapS(pthys, seqGap);
    }
    else
    {
	ajDebug("Remove all gaps\n");
	ajStrRemoveGap(pthys);
    }

    /* no need to test sequence type, we will test every character below */

    if(ajStrIsCharsetCaseS(*pthys, seqType[itype].Goodchars()))
    {
	if(seqType[itype].ConvertFrom)
	{
	    ajDebug("Convert '%s' to '%s'\n",
		    seqType[itype].ConvertFrom,
		    seqType[itype].ConvertTo);
	    ajStrExchangeSetCC(pthys,
			   seqType[itype].ConvertFrom,
			   seqType[itype].ConvertTo);
	}
	return ajTrue;
    }

    return ajTrue;
}




/* @func ajSeqTypeCheckIn *****************************************************
**
** Tests the type of a sequence is compatible with a defined type.
** If the type can have gaps, also tests for gap characters.
** Used for input validation - writes error message if the type check fails
**
** @param [u] thys [AjPSeq] Sequence object
** @param [r] seqin [const AjPSeqin] Sequence input object
** @return [AjBool] ajTrue if compatible.
** @@
******************************************************************************/

AjBool ajSeqTypeCheckIn(AjPSeq thys, const AjPSeqin seqin)
{    
    ajint itype = -1;
    AjPStr Type;
    ajint i;
    
    ajDebug("testing sequence '%s' '%S' type '%S' IsNuc %B IsProt %B\n",
	    ajSeqGetNameC(thys), thys->Seq,
	    seqin->Inputtype, seqin->IsNuc, seqin->IsProt);

    Type = seqin->Inputtype; /* ACD file had a predefined seq type */
    
    if(seqin->IsNuc)
	ajSeqSetNuc(thys);
    
    if(seqin->IsProt)
	ajSeqSetProt(thys);
    
    if(seqin->Query && ajStrGetLen(seqin->Query->DbType))
	seqTypeSet(thys, seqin->Query->DbType);

    
    if(!ajStrGetLen(Type))		   /* nothing given - anything goes */
    {
	ajSeqGap(thys, seqGap, 0);
	ajDebug("ajSeqTypeCheckIn: OK - no type, gaps converted to '-'\n");
	return ajTrue;
    }
    
    if(!seqFindType(Type, &itype))
    {
	ajDebug("ajSeqTypeCheckIn: rejected - unknown type\n");
	ajDie("Sequence type '%S' unknown", Type);
	return ajFalse;
    }

    ajDebug("ajSeqTypeCheckIn type '%s' found (%s)\n",
	    seqType[itype].Name, seqType[itype].Desc);

    if(seqType[itype].Gaps)
    {
	ajDebug("Convert gaps to '-'\n");
	ajSeqGap(thys, seqGap, 0);
    }
    else
    {
	ajDebug("Remove all gaps\n");
	ajStrRemoveGap(&thys->Seq);
    }

    if(seqType[itype].Type == ISPROT)
    {
	if (ajSeqIsProt(thys))
	{
	    ajSeqSetProt(thys);
	}
	else
	{
	    ajErr("Sequence is not a protein\n");
	    ajDebug("ajSeqTypeCheckIn: rejected - not a protein\n");
	    return ajFalse;
	}
    }

    if(seqType[itype].Type == ISNUC)
    {
	if (ajSeqIsNuc(thys))
	{
	    ajSeqSetNuc(thys);
	}
	else
	{
	    ajErr("Sequence is not nucleic\n");
	    ajDebug("ajSeqTypeCheckIn: rejected - not nucleic\n");
	    return ajFalse;
	}
    }

    if(ajStrIsCharsetCaseS(thys->Seq, seqType[itype].Goodchars()))
    {
	ajDebug("ajSeqTypeCheckIn: bad characters test passed, convert\n");
	if(seqType[itype].ConvertFrom)
	{
	    ajDebug("Convert '%s' to '%s'\n",
		    seqType[itype].ConvertFrom,
		    seqType[itype].ConvertTo);
	    ajStrExchangeSetCC(&thys->Seq,
			   seqType[itype].ConvertFrom,
			   seqType[itype].ConvertTo);
	}
	ajDebug("ajSeqTypeCheckIn: OK - no badchars\n");
	return ajTrue;
    }

    if(seqTypeFix(thys, itype))		/* this will reuse badchars */
    {
	ajDebug("ajSeqTypeCheckIn: OK - type fixed\n");
	return ajTrue;
    }

    i = ajStrFindRestCaseS(thys->Seq, seqType[itype].Goodchars());
    if(i >= 0)
    {
	ajErr("ajSeqTypeCheckIn: Sequence must be %s: "
	      "found bad character '%c'",
	      seqType[itype].Desc, ajStrGetCharPos(thys->Seq, i));
	ajDebug("ajSeqTypeCheckIn: rejected - still had badchars\n");
	return ajFalse;
    }

    ajDebug("ajSeqTypeCheckIn: OK - fixed finally\n");
    ajDebug("Final sequence '%S' type '%S' IsNuc %B IsProt %B\n",
	    thys->Seq, seqin->Inputtype, seqin->IsNuc, seqin->IsProt);
    return ajTrue;
}





/* @func ajSeqTypeNucS *****************************************************
**
** Checks sequence type for nucleotide without gaps.
**
** RNA and DNA codes are accepted as is.
**
** @param [r] thys [const AjPStr] Sequence string (unchanged at present)
** @return [char] invalid character if any.
** @@
******************************************************************************/

char ajSeqTypeNucS(const AjPStr thys)
{
    char ret;
    ajDebug("ajSeqTypeNucS test\n");

    ret = seqTypeTestS(thys, seqTypeStrNuc());
    if (ret)
	return ret;

    return seqTypeTestS(thys, seqTypeStrNucGap());
}




/* @func ajSeqTypeDnaS *****************************************************
**
** Checks sequence type for DNA without gaps.
**
** RNA and DNA codes are accepted as is.
**
** @param [r] thys [const AjPStr] Sequence string (unchanged at present)
** @return [char] invalid character if any.
** @@
******************************************************************************/

char ajSeqTypeDnaS(const AjPStr thys)
{
    char ret;
    ajDebug("ajSeqTypeDnaS test\n");

    ret = seqTypeTestS(thys, seqTypeStrNuc());
    if (ret)
	return ret;

    return seqTypeTestS(thys, seqTypeStrDnaGap());
}




/* @func ajSeqTypeRnaS *****************************************************
**
** Checks sequence type for Rna without gaps
**
** RNA codes are accepted as is.
**
** @param [r] thys [const AjPStr] Sequence string (unchanged at present)
** @return [char] invalid character if any.
** @@
******************************************************************************/

char ajSeqTypeRnaS(const AjPStr thys)
{
    char ret;
    ajDebug("ajSeqTypeRnaS test\n");

    ret = seqTypeTestS(thys, seqTypeStrNuc());
    if (ret)
	return ret;

    return seqTypeTestS(thys, seqTypeStrRnaGap());
}




/* @func ajSeqTypeGapdnaS *****************************************************
**
** Checks sequence type for Dna with gaps
**
** DNA codes are accepted as is.
**
** @param [r] thys [const AjPStr] Sequence string (unchanged at present)
** @return [char] invalid character if any.
** @@
******************************************************************************/

char ajSeqTypeGapdnaS(const AjPStr thys)
{
    char ret;
    ajDebug("ajSeqTypeGapdnaS test\n");

    ret = seqTypeTestS(thys, seqTypeStrNucGap());
    if (ret)
	return ret;

    return seqTypeTestS(thys, seqTypeStrDnaGap());
}




/* @func ajSeqTypeGaprnaS *****************************************************
**
** Checks sequence type for Rna with gaps
**
** RNA codes are accepted as is.
**
** @param [r] thys [const AjPStr] Sequence string (unchanged at present)
** @return [char] invalid character if any.
** @@
******************************************************************************/

char ajSeqTypeGaprnaS(const AjPStr thys)
{
    char ret;
    ajDebug("ajSeqTypeGaprnaS test\n");

    ret = seqTypeTestS(thys, seqTypeStrNucGap());
    if (ret)
	return ret;

    return seqTypeTestS(thys, seqTypeStrRnaGap());
}




/* @func ajSeqTypeGapnucS *****************************************************
**
** Checks sequence type for nucleotide with gaps.
**
** RNA and DNA codes are accepted as is.
**
** @param [r] thys [const AjPStr] Sequence string (unchanged at present)
** @return [char] invalid character if any.
** @@
******************************************************************************/

char ajSeqTypeGapnucS(const AjPStr thys)
{
    ajDebug("ajSeqTypeGapnucS test\n");

    return seqTypeTestS(thys, seqTypeStrNucGap());
}




/* @func ajSeqTypeAnyprotS ****************************************************
**
** Checks sequence type for anything that can be in a protein sequence
**
** Stop codes are replaced with gaps.
**
** @param [r] thys [const AjPStr] Sequence string (unchanged at present)
** @return [char] invalid character if any.
** @@
******************************************************************************/

char ajSeqTypeAnyprotS(const AjPStr thys)
{
    ajDebug("ajSeqTypeAnyprotS test\n");

    return seqTypeTestS(thys, seqTypeStrProtAny());
}




/* @func ajSeqTypeProtS ****************************************************
**
** Checks sequence type for anything that can be in a protein sequence
**
** Stop codes are replaced with gaps.
**
** @param [r] thys [const AjPStr] Sequence string (unchanged at present)
** @return [char] invalid character if any.
** @@
******************************************************************************/

char ajSeqTypeProtS(const AjPStr thys)
{
    ajDebug("ajSeqTypeProtS test\n");

    return seqTypeTestS(thys, seqTypeStrProt());
}




/* @func ajSeqTypeGapanyS *****************************************************
**
** Checks sequence type for any sequence with gaps.
**
** Stops ('*') are allowed so this could be a 3 frame translation of DNA.
**
** @param [r] thys [const AjPStr] Sequence string (unchanged at present)
** @return [char] invalid character if any.
** @@
******************************************************************************/

char ajSeqTypeGapanyS(const AjPStr thys)
{
    ajDebug("ajSeqTypeGapanyS test\n");

    return seqTypeTestS(thys, seqTypeStrAnyGap());
}




/* @func ajSeqGap *************************************************************
**
** Sets non-sequence characters to valid gap characters,
** and pads with extra gaps if necessary to a specified length
**
** @param [u] thys [AjPSeq] Sequence
** @param [r] gapc [char] Standard gap character
** @param [r] padc [char] Gap character for ends of sequence
** @return [void]
** @@
******************************************************************************/

void ajSeqGap(AjPSeq thys, char gapc, char padc)
{
    seqGapSL(&thys->Seq, gapc, padc, 0);

    return;
}




/* @func ajSeqGapLen **********************************************************
**
** Sets non-sequence characters to valid gap characters,
** and pads with extra gaps if necessary to a specified length
**
** @param [u] thys [AjPSeq] Sequence
** @param [r] gapc [char] Standard gap character
** @param [r] padc [char] Gap character for ends of sequence
** @param [r] ilen [ajint] Sequence length. Expanded if longer than
**                       current length
** @return [void]
** @@
******************************************************************************/

void ajSeqGapLen(AjPSeq thys, char gapc, char padc, ajint ilen)
{
    seqGapSL(&thys->Seq, gapc, padc, ilen);

    return;
}

/* @func ajSeqGapS ************************************************************
**
** Sets non-sequence characters to valid gap characters,
** and pads with extra gaps if necessary to a specified length
**
** @param [u] seq [AjPStr*] Sequence
** @param [r] gapc [char] Standard gap character
** @return [void]
** @@
******************************************************************************/

void ajSeqGapS(AjPStr* seq, char gapc)
{
    seqGapSL(seq, gapc, 0, 0);

    return;
}

/* @funcstatic seqGapSL *******************************************************
**
** Sets non-sequence characters in a string to valid gap characters,
** and pads with extra gaps if necessary to a specified length
**
** @param [u] seq [AjPStr*] String of sequence characters
** @param [r] gapc [char] Standard gap character
** @param [r] padc [char] Gap character for ends of sequence
** @param [r] ilen [ajuint] Sequence length. Expanded if longer than
**                       current length
** @return [void]
** @@
******************************************************************************/

static void seqGapSL(AjPStr* seq, char gapc, char padc, ajuint ilen)
{
    ajuint i;
    static ajuint igap;
    char* cp;
    char endc = gapc;
    
    igap = strlen(seqCharGapTest);
    if(!seqNewGapChars)
    {
	seqNewGapChars = ajCharNewRes(igap);
	seqNewGapChars[0] = '\0';
    }
    
    /* Set the seqNewGapChars string to match gapc */
    
    if(*seqNewGapChars != gapc)
    {
	for(i=0; i < igap; i++)
	    seqNewGapChars[i] = gapc;
	seqNewGapChars[igap] = '\0';
    }
    
    
    if(ilen)
	ajStrSetRes(seq, ilen+1);
    else
	ajStrGetuniqueStr(seq);
    
    ajStrExchangeSetCC(seq, seqCharGapTest, seqNewGapChars);
    
    if(padc)
    {				/* start and end characters updated */
	endc = padc;
	/* pad start */
	for(cp = ajStrGetuniquePtr(seq); strchr(seqCharGapTest, *cp); cp++)
	    *cp = padc;

	cp = ajStrGetuniquePtr(seq);
	for(i=ajStrGetLen(*seq) - 1; i && strchr(seqCharGapTest, cp[i]);  i--)
	    cp[i] = padc;
    }
    
    if(ajStrGetLen(*seq) < ilen)	   /* ilen can be zero to skip this */
    {
	cp = ajStrGetuniquePtr(seq);
	for(i=ajStrGetLen(*seq); i < ilen; i++)
	    cp[i] = endc;
	cp[ilen] = '\0';
	ajStrSetValid(seq);
    }
    
    /*  ajDebug("seqGapSL after  '%S'\n", *seq); */

    return;
}




/* @funcstatic seqTypeStopTrimS ***********************************************
**
** Removes a trailing stop (asterisk) from a protein sequence
**
** @param [u] pthys [AjPStr*] Sequence string
** @return [AjBool] ajTrue if a stop was removed.
** @@
******************************************************************************/

static AjBool seqTypeStopTrimS(AjPStr* pthys)
{
    if(strchr(seqCharProtStop,ajStrGetCharLast(*pthys)))
    {
	ajDebug("Trailing stop removed %c\n", ajStrGetCharLast(*pthys));
	ajStrCutEnd(pthys, 1);
	return ajTrue;
    }

    return ajFalse;
}




/* @func ajSeqSetNuc **********************************************************
**
** Sets a sequence type to "nucleotide"
**
** @param [u] thys [AjPSeq] Sequence object
** @return [void]
 ** @category modify [AjPSeq] Sets sequence to be nucleotide
** @@
******************************************************************************/

void ajSeqSetNuc(AjPSeq thys)
{
    if(ajStrMatchC(thys->Type, "N"))
      return;

    ajStrAssignC(&thys->Type, "N");
    if(thys->Fttable)
	ajFeattableSetNuc(thys->Fttable);

    /* set N as the ambiguity code */
    ajStrExchangeSetCC(&thys->Seq,
		       "xX",
		       "nN");

    return;
}




/* @func ajSeqSetProt *********************************************************
**
** Sets a sequence type to "protein"
**
** @param [u] thys [AjPSeq] Sequence object
** @return [void]
** @category modify [AjPSeq] Sets sequence to be protein
** @@
******************************************************************************/

void ajSeqSetProt(AjPSeq thys)
{
    if(ajStrMatchC(thys->Type, "P"))
      return;

    ajStrAssignC(&thys->Type, "P");
    if(thys->Fttable)
	ajFeattableSetProt(thys->Fttable);

    return;
}




/* @func ajSeqsetSetNuc *******************************************************
**
** Sets a sequence set type to "nucleotide"
**
** @param [u] thys [AjPSeqset] Sequence set object
** @return [void]
** @@
******************************************************************************/

void ajSeqsetSetNuc(AjPSeqset thys)
{
    ajStrAssignC(&thys->Type, "N");

    return;
}




/* @func ajSeqsetSetProt ******************************************************
**
** Sets a sequence set type to "protein"
**
** @param [u] thys [AjPSeqset] Sequence set object
** @return [void]
** @@
******************************************************************************/

void ajSeqsetSetProt(AjPSeqset thys)
{
    ajStrAssignC(&thys->Type, "P");

    return;
}




/* @func ajSeqType ************************************************************
**
** Sets the type of a sequence if it has not yet been defined.
**
** @param [u] thys [AjPSeq] Sequence object
** @return [void]
** @category modify [AjPSeq] Sets the sequence type
** @@
******************************************************************************/

void ajSeqType(AjPSeq thys)
{
    ajDebug("ajSeqType current: %S\n", thys->Type);

    if(ajStrGetLen(thys->Type))
	return;

    if(ajSeqIsNuc(thys))
    {
	ajSeqSetNuc(thys);
	ajDebug("ajSeqType nucleotide: %S\n", thys->Type);
	return;
    }

    if(ajSeqIsProt(thys))
    {
	ajSeqSetProt(thys);
	ajDebug("ajSeqType protein: %S\n", thys->Type);
	return;
    }

    ajDebug("ajSeqType unknown: %S\n", thys->Type);

    return;
}




/* @func ajSeqPrintType *******************************************************
**
** Prints the seqType definitions.
** For EMBOSS entrails output
**
** @param [u] outf [AjPFile] Output file
** @param [r] full [AjBool] Full output
** @return [void]
******************************************************************************/

void ajSeqPrintType(AjPFile outf, AjBool full)
{
    ajuint i;
    AjPStr tmpstr = NULL;
    ajuint maxtmp = 0;

    const char* typeName[] = {"ANY", "NUC", "PRO"};


    (void) full;	    /* make used - no extra detail reported */

    ajFmtPrintF(outf, "\n# Sequence Types\n");
    ajFmtPrintF(outf, "# Name                 Gap Ambig N/P "
		"From     To       Description\n");
    ajFmtPrintF(outf, "seqType {\n");
    for(i=0; seqType[i].Name; i++)
    {
	if (seqType[i].ConvertFrom)
	{
	    ajFmtPrintF(outf, "  %-20s %3B   %3B %3s",
			seqType[i].Name, seqType[i].Gaps,
			seqType[i].Ambig, typeName[seqType[i].Type]);
	    ajFmtPrintS(&tmpstr, "\"%s\"", seqType[i].ConvertFrom);
	    if(maxtmp > ajStrGetLen(tmpstr))
	       maxtmp = ajStrGetLen(tmpstr);
	    ajFmtPrintF(outf, " %-8S", tmpstr);
	    ajFmtPrintS(&tmpstr, "\"%s\"", seqType[i].ConvertTo);
	    if(maxtmp > ajStrGetLen(tmpstr))
	       maxtmp = ajStrGetLen(tmpstr);
	    ajFmtPrintF(outf, " %-8S", tmpstr);
	    ajFmtPrintF(outf, " \"%s\"\n", seqType[i].Desc);
	}
	else
	{
	    ajFmtPrintF(outf, "  %-20s %3B   %3B %s \"\"       \"\"       "
			"\"%s\"\n",
			seqType[i].Name, seqType[i].Gaps,
			seqType[i].Ambig, typeName[seqType[i].Type],
			seqType[i].Desc);
	}
    }
    ajFmtPrintF(outf, "}\n");
    if(maxtmp > 8) ajWarn("ajSeqPrintType max tmpstr len %d",
			maxtmp);	      
    ajStrDel(&tmpstr);

    return;
}




/* @funcstatic seqTypeTest ****************************************************
**
** Checks sequence contains only expected characters.
**
** Returns an invalid character for failure, or a null character for success.
**
** @param [r] thys [const AjPStr] Sequence string
** @param [u] badchars [AjPRegexp] Regular expression for
**                                 sequence characters disallowed
** @return [char] invalid character if any.
******************************************************************************/

static char seqTypeTest(const AjPStr thys, AjPRegexp badchars)
{
    AjPStr tmpstr = NULL;
    char ret = '\0';

    if(!ajStrGetLen(thys))
	return ret;

    /*ajDebug("seqTypeTest, Sequence '%S'\n", thys);*/
    if(!ajRegExec(badchars, thys))
	return ret;

    ajRegSubI(badchars, 1, &tmpstr);
    ret = ajStrGetCharFirst(tmpstr);
    ajDebug("seqTypeTest, Sequence had bad character '%c' (%x) "
	    "at %d of %d/%d\n '%S'\n",
	    ret, ret,
	    ajRegOffset(badchars),
	    ajStrGetLen(thys), strlen(ajStrGetPtr(thys)), tmpstr);

    ajStrDel(&tmpstr);

    return ret;
}




/* @funcstatic seqTypeTestS ***************************************************
**
** Checks sequence contains only expected characters.
**
** Returns an invalid character for failure, or a null character for success.
**
** @param [r] thys [const AjPStr] Sequence string
** @param [r] goodchars [const AjPStr] String of
**                                 sequence characters allowed
** @return [char] invalid character if any.
******************************************************************************/

static char seqTypeTestS(const AjPStr thys, const AjPStr goodchars)
{
    char ret = '\0';
    ajint i;

    if(!ajStrGetLen(thys))
	return ret;

    ajDebug("seqTypeTest, len %d goodchars '%S'\n",
	    ajStrGetLen(thys), goodchars);

    if(ajStrIsCharsetCaseS(thys, goodchars))
      return ret;

    i = ajStrFindRestCaseS(thys, goodchars);
    if (i < 0)
      return ret;

    ret = ajStrGetCharPos(thys, i);
    ajDebug("seqTypeTest, Sequence had bad character '%c' (%x) "
	    "at %d of %d/%d\n",
	    ret, ret,
	    i,
	    ajStrGetLen(thys), strlen(ajStrGetPtr(thys)));


    return ret;
}




/* @funcstatic seqTypeCharAny *************************************************
**
** Returns regular expression to test for type Any
**
** @return [AjPRegexp] valid characters
******************************************************************************/

static AjPRegexp seqTypeCharAny(void)
{
    AjPStr regstr = NULL;
    AjPStr tmpstr = NULL;

    if(!seqtypeRegAny)
    {
	regstr = ajStrNewRes(256);
	tmpstr = ajStrNewS(seqTypeStrAny());

	ajStrAppendC(&regstr, "([^");
	ajStrKeepSetAlpha(&tmpstr);
	ajStrFmtLower(&tmpstr);
	ajStrAppendS(&regstr, tmpstr);
	ajStrAppendS(&regstr, seqTypeStrAny());
	ajStrAppendC(&regstr, "+])");

	seqtypeRegAny = ajRegComp(regstr);

	ajStrDel(&regstr);
	ajStrDel(&tmpstr);
    }

    return seqtypeRegAny;
}

/* @funcstatic seqTypeCharAnyGap **********************************************
**
** Returns regular expression to test for type Any with gaps
**
** @return [AjPRegexp] valid characters
******************************************************************************/

static AjPRegexp seqTypeCharAnyGap(void)
{
    AjPStr regstr = NULL;
    AjPStr tmpstr = NULL;

    if(!seqtypeRegAnyGap)
    {
	regstr = ajStrNewRes(256);
	tmpstr = ajStrNewS(seqTypeStrAnyGap());

	ajStrAppendC(&regstr, "([^");
	ajStrAppendS(&regstr, tmpstr);
	ajStrKeepSetAlpha(&tmpstr);
	ajStrFmtLower(&tmpstr);
	ajStrAppendS(&regstr, tmpstr);
	ajStrAppendC(&regstr, "+])");

	seqtypeRegAnyGap = ajRegComp(regstr);

	ajStrDel(&regstr);
	ajStrDel(&tmpstr);
    }

    return seqtypeRegAnyGap;
}




/* @funcstatic seqTypeCharNuc *************************************************
**
** Returns regular expression to test for nucleotide bases
**
** @return [AjPRegexp] valid characters
******************************************************************************/

static AjPRegexp seqTypeCharNuc(void)
{
    AjPStr regstr = NULL;
    AjPStr tmpstr = NULL;

    if(!seqtypeRegNuc)
    {
	regstr = ajStrNewRes(256);
	tmpstr = ajStrNewS(seqTypeStrNuc());

	ajStrAppendC(&regstr, "([^");
	ajStrKeepSetAlpha(&tmpstr);
	ajStrFmtLower(&tmpstr);
	ajStrAppendS(&regstr, tmpstr);
	ajStrAppendS(&regstr, seqTypeStrNuc());
	ajStrAppendC(&regstr, "+])");

	seqtypeRegNuc = ajRegComp(regstr);

	ajStrDel(&regstr);
	ajStrDel(&tmpstr);
     }

    return seqtypeRegNuc;
}




/* @funcstatic seqTypeCharNucGap **********************************************
**
** Returns regular expression to test for nucleotide bases with gaps
**
** @return [AjPRegexp] valid characters
******************************************************************************/

static AjPRegexp seqTypeCharNucGap(void)
{
    AjPStr regstr = NULL;
    AjPStr tmpstr = NULL;

    if(!seqtypeRegNucGap)
    {
	regstr = ajStrNewRes(256);
	tmpstr = ajStrNewS(seqTypeStrNucGap());

	ajStrAppendC(&regstr, "([^");
	ajStrAppendS(&regstr, tmpstr);
	ajStrKeepSetAlpha(&tmpstr);
	ajStrFmtLower(&tmpstr);
	ajStrAppendS(&regstr, tmpstr);
	ajStrAppendC(&regstr, "+])");

	seqtypeRegNucGap = ajRegComp(regstr);

	ajStrDel(&regstr);
	ajStrDel(&tmpstr);
    }

    return seqtypeRegNucGap;
}




/* @funcstatic seqTypeCharNucGapPhylo *****************************************
**
** Returns regular expression to test for nucleotide bases with gaps
** and queries
**
** @return [AjPRegexp] valid characters
******************************************************************************/

static AjPRegexp seqTypeCharNucGapPhylo(void)
{
    AjPStr regstr = NULL;
    AjPStr tmpstr = NULL;

    if(!seqtypeRegNucGapPhylo)
    {
	regstr = ajStrNewRes(256);
	tmpstr = ajStrNewS(seqTypeStrNucGapPhylo());

	ajStrAppendC(&regstr, "([^");
	ajStrKeepSetAlpha(&tmpstr);
	ajStrFmtLower(&tmpstr);
	ajStrAppendS(&regstr, tmpstr);
	ajStrAppendS(&regstr, seqTypeStrNucGapPhylo());
	ajStrAppendC(&regstr, "+])");

	seqtypeRegNucGapPhylo = ajRegComp(regstr);

	ajStrDel(&regstr);
	ajStrDel(&tmpstr);
    }

    return seqtypeRegNucGapPhylo;
}




/* @funcstatic seqTypeCharNucPure *********************************************
**
** Returns regular expression to test for nucleotide bases
** with no ambiguity
**
** @return [AjPRegexp] valid characters
******************************************************************************/

static AjPRegexp seqTypeCharNucPure(void)
{
    AjPStr regstr = NULL;
    AjPStr tmpstr = NULL;

    if(!seqtypeRegNucPure)
    {
	regstr = ajStrNewRes(256);
	tmpstr = ajStrNewS(seqTypeStrNucPure());

	ajStrAppendC(&regstr, "([^");
	ajStrKeepSetAlpha(&tmpstr);
	ajStrFmtLower(&tmpstr);
	ajStrAppendS(&regstr, tmpstr);
	ajStrAppendS(&regstr, seqTypeStrNucPure());
	ajStrAppendC(&regstr, "+])");

	seqtypeRegNucPure = ajRegComp(regstr);

	ajStrDel(&regstr);
	ajStrDel(&tmpstr);
    }

    return seqtypeRegNucPure;
}




/* @funcstatic seqTypeCharProt ************************************************
**
** Returns regular expression to test for protein residues
**
** @return [AjPRegexp] valid characters
******************************************************************************/

static AjPRegexp seqTypeCharProt(void)
{
    AjPStr regstr = NULL;
    AjPStr tmpstr = NULL;

    if(!seqtypeRegProt)
    {
	regstr = ajStrNewRes(256);
	tmpstr = ajStrNewS(seqTypeStrProt());

	ajStrAppendC(&regstr, "([^");
	ajStrKeepSetAlpha(&tmpstr);
	ajStrFmtLower(&tmpstr);
	ajStrAppendS(&regstr, tmpstr);
	ajStrAppendS(&regstr, seqTypeStrProt());
	ajStrAppendC(&regstr, "+])");

	seqtypeRegProt = ajRegComp(regstr);

	ajStrDel(&regstr);
	ajStrDel(&tmpstr);
    }

    return seqtypeRegProt;
}




/* @funcstatic seqTypeCharProtGap *********************************************
**
** Returns regular expression to test for protein residues or gaps
**
** @return [AjPRegexp] valid characters
******************************************************************************/

static AjPRegexp seqTypeCharProtGap(void)
{
    AjPStr regstr = NULL;
    AjPStr tmpstr = NULL;

    if(!seqtypeRegProtGap)
    {
	regstr = ajStrNewRes(256);
	tmpstr = ajStrNewS(seqTypeStrProtGap());

	ajStrAppendC(&regstr, "([^");
	ajStrKeepSetAlpha(&tmpstr);
	ajStrFmtLower(&tmpstr);
	ajStrAppendS(&regstr, tmpstr);
	ajStrAppendS(&regstr, seqTypeStrProtGap());
	ajStrAppendC(&regstr, "+])");

	seqtypeRegProtGap = ajRegComp(regstr);

	ajStrDel(&regstr);
	ajStrDel(&tmpstr);
    }

    return seqtypeRegProtGap;
}




/* @funcstatic seqTypeCharProtGapPhylo ****************************************
**
** Returns regular expression to test for protein residues or gaps
** stops and queries
**
** @return [AjPRegexp] valid characters
******************************************************************************/

static AjPRegexp seqTypeCharProtGapPhylo(void)
{
    AjPStr regstr = NULL;
    AjPStr tmpstr = NULL;

    if(!seqtypeRegProtGapPhylo)
    {
	regstr = ajStrNewRes(256);
	tmpstr = ajStrNewS(seqTypeStrProtGapPhylo());

	ajStrAppendC(&regstr, "([^");
	ajStrKeepSetAlpha(&tmpstr);
	ajStrFmtLower(&tmpstr);
	ajStrAppendS(&regstr, tmpstr);
	ajStrAppendS(&regstr, seqTypeStrProtGapPhylo());
	ajStrAppendC(&regstr, "+])");

	seqtypeRegProtGapPhylo = ajRegComp(regstr);

	ajStrDel(&regstr);
	ajStrDel(&tmpstr);
    }

    return seqtypeRegProtGapPhylo;
}




/* @funcstatic seqTypeCharProtPure ********************************************
**
** Returns regular expression to test for protein residues
** with no ambiguity
**
** @return [AjPRegexp] valid characters
******************************************************************************/

static AjPRegexp seqTypeCharProtPure(void)
{
    AjPStr regstr = NULL;
    AjPStr tmpstr = NULL;

    if(!seqtypeRegProtPure)
    {
	regstr = ajStrNewRes(256);
	tmpstr = ajStrNewS(seqTypeStrProtPure());

	ajStrAppendC(&regstr, "([^");
	ajStrKeepSetAlpha(&tmpstr);
	ajStrFmtLower(&tmpstr);
	ajStrAppendS(&regstr, tmpstr);
	ajStrAppendS(&regstr, seqTypeStrProtPure());
	ajStrAppendC(&regstr, "+])");

	seqtypeRegProtPure = ajRegComp(regstr);

	ajStrDel(&regstr);
	ajStrDel(&tmpstr);
    }

    return seqtypeRegProtPure;
}




/* @funcstatic seqTypeCharProtStop ********************************************
**
** Returns regular expression to test for protein residues or stop codons
**
** @return [AjPRegexp] valid characters
******************************************************************************/

static AjPRegexp seqTypeCharProtStop(void)
{
    AjPStr regstr = NULL;
    AjPStr tmpstr = NULL;

    if(!seqtypeRegProtStop)
    {
	regstr = ajStrNewRes(256);
	tmpstr = ajStrNewS(seqTypeStrProtStop());

	ajStrAppendC(&regstr, "([^");
	ajStrKeepSetAlpha(&tmpstr);
	ajStrFmtLower(&tmpstr);
	ajStrAppendS(&regstr, tmpstr);
	ajStrAppendS(&regstr, seqTypeStrProtStop());
	ajStrAppendC(&regstr, "+])");

	seqtypeRegProtStop = ajRegComp(regstr);

	ajStrDel(&regstr);
	ajStrDel(&tmpstr);
    }

    return seqtypeRegProtStop;
}




/* @funcstatic seqTypeCharProtStopGap *****************************************
**
** Returns regular expression to test for protein residues or stop codons
** or gap characters
**
** @return [AjPRegexp] valid characters
******************************************************************************/

static AjPRegexp seqTypeCharProtStopGap(void)
{
    AjPStr regstr = NULL;
    AjPStr tmpstr = NULL;

    if(!seqtypeRegProtStopGap)
    {
	regstr = ajStrNewRes(256);
	tmpstr = ajStrNewS(seqTypeStrProtStopGap());

	ajStrAppendC(&regstr, "([^");
	ajStrKeepSetAlpha(&tmpstr);
	ajStrFmtLower(&tmpstr);
	ajStrAppendS(&regstr, tmpstr);
	ajStrAppendS(&regstr, seqTypeStrProtStopGap());
	ajStrAppendC(&regstr, "+])");

	seqtypeRegProtStopGap = ajRegComp(regstr);

	ajStrDel(&regstr);
	ajStrDel(&tmpstr);
    }

    return seqtypeRegProtStopGap;
}




/* @funcstatic seqTypeStrAny **************************************************
**
** Returns string of valid characters to test for type Any
**
** @return [AjPStr] valid characters
******************************************************************************/

static AjPStr seqTypeStrAny(void)
{
    if(!seqtypeCharsetAny)
	ajFmtPrintS(&seqtypeCharsetAny, "%s%s%s%s%s",
		    seqCharProtPure,
		    seqCharProtAmbig,
		    seqCharProtStop,
		    seqCharNucPure,
		    seqCharNucAmbig);

    return seqtypeCharsetAny;
}

/* @funcstatic seqTypeStrAnyGap ***********************************************
**
** Returns string of valid characters to test for type Anygap
**
** @return [AjPStr] valid characters
******************************************************************************/

static AjPStr seqTypeStrAnyGap(void)
{
    if(!seqtypeCharsetAnyGap)
	ajFmtPrintS(&seqtypeCharsetAnyGap, "%s%s%s%s%s%s",
		    seqCharProtPure,
		    seqCharProtAmbig,
		    seqCharProtStop,
		    seqCharNucPure,
		    seqCharNucAmbig,
		    seqCharGap);

    return seqtypeCharsetAnyGap;
}


/* @funcstatic seqTypeStrDnaGap ***********************************************
**
** Returns string of valid characters to test for type Dnagap
**
** @return [AjPStr] valid characters
******************************************************************************/

static AjPStr seqTypeStrDnaGap(void)
{
    if(!seqtypeCharsetDnaGap)
	ajFmtPrintS(&seqtypeCharsetDnaGap, "%s%s%s",
		    seqCharNucPure,
		    seqCharNucAmbig,
		    seqCharGap);

    return seqtypeCharsetDnaGap;
}


/* @funcstatic seqTypeStrNuc **************************************************
**
** Returns string of valid characters to test for type Nuc
**
** @return [AjPStr] valid characters
******************************************************************************/

static AjPStr seqTypeStrNuc(void)
{
    if(!seqtypeCharsetNuc)
	ajFmtPrintS(&seqtypeCharsetNuc, "%s%s",
		    seqCharNucPure,
		    seqCharNucAmbig);

    return seqtypeCharsetNuc;
}


/* @funcstatic seqTypeStrNucGap ***********************************************
**
** Returns string of valid characters to test for type Nucgap
**
** @return [AjPStr] valid characters
******************************************************************************/

static AjPStr seqTypeStrNucGap(void)
{
    if(!seqtypeCharsetNucGap)
	ajFmtPrintS(&seqtypeCharsetNucGap, "%s%s%s",
		    seqCharNucPure,
		    seqCharNucAmbig,
		    seqCharGap);

    return seqtypeCharsetNucGap;
}


/* @funcstatic seqTypeStrNucGapPhylo ******************************************
**
** Returns string of valid characters to test for type Nucgapphylo
**
** @return [AjPStr] valid characters
******************************************************************************/

static AjPStr seqTypeStrNucGapPhylo(void)
{
    if(!seqtypeCharsetNucGapPhylo)
	ajFmtPrintS(&seqtypeCharsetNucGapPhylo, "%s%s%s%s",
		    seqCharNucPure,
		    seqCharNucAmbig,
		    seqCharPhylo,
		    seqCharGap);

    return seqtypeCharsetNucGapPhylo;
}


/* @funcstatic seqTypeStrNucPure **********************************************
**
** Returns string of valid characters to test for type Nucpure
**
** @return [AjPStr] valid characters
******************************************************************************/

static AjPStr seqTypeStrNucPure(void)
{
    if(!seqtypeCharsetNucPure)
	ajFmtPrintS(&seqtypeCharsetNucPure, "%s",
		    seqCharNucPure);

    return seqtypeCharsetNucPure;
}


/* @funcstatic seqTypeStrProt *************************************************
**
** Returns string of valid characters to test for type Prot
**
** @return [AjPStr] valid characters
******************************************************************************/

static AjPStr seqTypeStrProt(void)
{
    if(!seqtypeCharsetProt)
	ajFmtPrintS(&seqtypeCharsetProt, "%s%s",
		    seqCharProtPure,
		    seqCharProtAmbig);

    return seqtypeCharsetProt;
}


/* @funcstatic seqTypeStrProtAny **********************************************
**
** Returns string of valid characters to test for type Protany
**
** @return [AjPStr] valid characters
******************************************************************************/

static AjPStr seqTypeStrProtAny(void)
{
    if(!seqtypeCharsetProtAny)
	ajFmtPrintS(&seqtypeCharsetProtAny, "%s%s%s%s%s",
		    seqCharProtPure,
		    seqCharProtAmbig,
		    seqCharProtStop,
		    seqCharPhylo,
		    seqCharGap);

    return seqtypeCharsetProtAny;
}


/* @funcstatic seqTypeStrProtGap **********************************************
**
** Returns string of valid characters to test for type Protgap
**
** @return [AjPStr] valid characters
******************************************************************************/

static AjPStr seqTypeStrProtGap(void)
{
    if(!seqtypeCharsetProtGap)
	ajFmtPrintS(&seqtypeCharsetProtGap, "%s%s%s",
		    seqCharProtPure,
		    seqCharProtAmbig,
		    seqCharGap);

    return seqtypeCharsetProtGap;
}


/* @funcstatic seqTypeStrProtGapPhylo *****************************************
**
** Returns string of valid characters to test for type Protgapphylo
**
** @return [AjPStr] valid characters
******************************************************************************/

static AjPStr seqTypeStrProtGapPhylo(void)
{
    if(!seqtypeCharsetProtGapPhylo)
	ajFmtPrintS(&seqtypeCharsetProtGapPhylo, "%s%s%s%s%s",
		    seqCharProtPure,
		    seqCharProtAmbig,
		    seqCharProtStop,
		    seqCharPhylo,
		    seqCharGap);

    return seqtypeCharsetProtGapPhylo;
}


/* @funcstatic seqTypeStrProtPure *********************************************
**
** Returns string of valid characters to test for type Protpure
**
** @return [AjPStr] valid characters
******************************************************************************/

static AjPStr seqTypeStrProtPure(void)
{
    if(!seqtypeCharsetProtPure)
	ajFmtPrintS(&seqtypeCharsetProtPure, "%s",
		    seqCharProtPure);

    return seqtypeCharsetProtPure;
}


/* @funcstatic seqTypeStrProtStop *********************************************
**
** Returns string of valid characters to test for type Protstop
**
** @return [AjPStr] valid characters
******************************************************************************/

static AjPStr seqTypeStrProtStop(void)
{
    if(!seqtypeCharsetProtStop)
	ajFmtPrintS(&seqtypeCharsetProtStop, "%s%s%s",
		    seqCharProtPure,
		    seqCharProtAmbig,
		    seqCharProtStop);

    return seqtypeCharsetProtStop;
}


/* @funcstatic seqTypeStrProtStopGap ******************************************
**
** Returns string of valid characters to test for type Protstopgap
**
** @return [AjPStr] valid characters
******************************************************************************/

static AjPStr seqTypeStrProtStopGap(void)
{
    if(!seqtypeCharsetProtStopGap)
	ajFmtPrintS(&seqtypeCharsetProtStopGap, "%s%s%s%s",
		    seqCharProtPure,
		    seqCharProtAmbig,
		    seqCharProtStop,
		    seqCharGap);

    return seqtypeCharsetProtStopGap;
}


/* @funcstatic seqTypeStrRnaGap ***********************************************
**
** Returns string of valid characters to test for type Rnagap
**
** @return [AjPStr] valid characters
******************************************************************************/

static AjPStr seqTypeStrRnaGap(void)
{
    if(!seqtypeCharsetRnaGap)
	ajFmtPrintS(&seqtypeCharsetRnaGap, "%s%s",
		    seqCharNucRna,
		    seqCharGap);

    return seqtypeCharsetRnaGap;
}


/* @funcstatic seqFindType ****************************************************
**
** Returns sequence type index and ajTrue if type was found
**
** @param [r] type_name [const AjPStr] Sequence type
** @param [w] typenum [ajint*] Sequence type index
** @return [AjBool] ajTrue if sequence type was found
**
******************************************************************************/

static AjBool seqFindType(const AjPStr type_name, ajint* typenum)
{
    ajint i;
    ajint itype = -1;

    for(i = 0; seqType[i].Name; i++)
	if(ajStrMatchCaseC(type_name, seqType[i].Name))
	{
	    itype = i;
	    break;
	}

    if(itype <0)
    {
	*typenum = i;
	return ajFalse;
    }

    *typenum = itype;

    return ajTrue;
}




/* @func ajSeqTypeIsProt ******************************************************
**
** Returns ajTrue is sequence type can be a protein (or 'any')
**
** @param [r] type_name [const AjPStr] Sequence type
** @return [AjBool] ajTrue if sequence can be protein
**
******************************************************************************/

AjBool ajSeqTypeIsProt(const AjPStr type_name)
{
    ajint itype;
    if(seqFindType(type_name, &itype))
	switch(seqType[itype].Type)
	{
	case ISNUC:
	    return ajFalse;
	default:
	    return ajTrue;
	}

    return ajFalse;
}




/* @func ajSeqTypeIsNuc *******************************************************
**
** Returns ajTrue is sequence type can be a nucleotide (or 'any')
**
** @param [r] type_name [const AjPStr] Sequence type
** @return [AjBool] ajTrue if sequence can be nucleotide
**
******************************************************************************/

AjBool ajSeqTypeIsNuc(const AjPStr type_name)
{
    ajint itype;

    if(seqFindType(type_name, &itype))
	switch(seqType[itype].Type)
	{
	case ISPROT:
	    return ajFalse;
	default:
	    return ajTrue;
	}

    return ajFalse;
}




/* @func ajSeqTypeIsAny *******************************************************
**
** Returns ajTrue is sequence type can be a protein or nucleotide
**
** @param [r] type_name [const AjPStr] Sequence type
** @return [AjBool] ajTrue if sequence can be protein or nucleotide
**
******************************************************************************/

AjBool ajSeqTypeIsAny(const AjPStr type_name)
{
    ajint itype;

    if(seqFindType(type_name, &itype))
	switch(seqType[itype].Type)
	{
	case ISNUC:
	    return ajFalse;
	case ISPROT:
	    return ajFalse;
	default:
	    return ajTrue;
	}

    return ajFalse;
}




/* @func ajSeqTypeSummary *****************************************************
**
** Returns ajTrue is sequence type can be a protein or nucleotide
**
** @param [r] type_name [const AjPStr] Sequence type
** @param [w] Ptype [AjPStr*] Sequence type 'protein' 'nucleotide' or 'any'
** @param [w] gaps [AjBool*] True if gap characters are preserved
** @return [AjBool] ajTrue if sequence can be protein or nucleotide
**
******************************************************************************/

AjBool ajSeqTypeSummary(const AjPStr type_name, AjPStr* Ptype, AjBool* gaps)
{
    ajint itype;

    if(seqFindType(type_name, &itype))
    {
	*gaps = seqType[itype].Gaps;
	switch(seqType[itype].Type)
	{
	case ISNUC:
	    ajStrAssignC(Ptype, "nucleotide");
	    break;
	case ISPROT:
	    ajStrAssignC(Ptype, "protein");
	    break;
	default:
	    ajStrAssignClear(Ptype);
	    break;
	}
	return ajTrue;
    }

    return ajFalse;
}




/* @func ajSeqTypeExit ********************************************************
**
** Cleans up sequence type processing internal memory
**
** @return [void]
** @@
******************************************************************************/

void ajSeqTypeExit(void)
{
    ajRegFree(&seqtypeRegAny);
    ajRegFree(&seqtypeRegAnyGap);
    ajRegFree(&seqtypeRegDnaGap);
    ajRegFree(&seqtypeRegNuc);
    ajRegFree(&seqtypeRegNucGap);
    ajRegFree(&seqtypeRegNucPure);
    ajRegFree(&seqtypeRegProt);
    ajRegFree(&seqtypeRegProtAny);
    ajRegFree(&seqtypeRegProtGap);
    ajRegFree(&seqtypeRegProtPure);
    ajRegFree(&seqtypeRegProtStop);
    ajRegFree(&seqtypeRegRnaGap);

    ajStrDel(&seqtypeCharsetAny);
    ajStrDel(&seqtypeCharsetAnyGap);
    ajStrDel(&seqtypeCharsetDnaGap);
    ajStrDel(&seqtypeCharsetNuc);
    ajStrDel(&seqtypeCharsetNucGap);
    ajStrDel(&seqtypeCharsetNucGapPhylo);
    ajStrDel(&seqtypeCharsetNucPure);
    ajStrDel(&seqtypeCharsetProt);
    ajStrDel(&seqtypeCharsetProtAny);
    ajStrDel(&seqtypeCharsetProtGap);
    ajStrDel(&seqtypeCharsetProtPure);
    ajStrDel(&seqtypeCharsetProtStop);
    ajStrDel(&seqtypeCharsetProtStopGap);
    ajStrDel(&seqtypeCharsetRnaGap);

    ajCharDel(&seqNewGapChars);
    return;
}



/* @func ajSeqTypeUnused ******************************************************
**
** Dummy function to catch all unused functions defined in the ajseqtype
** source file.
**
** @return [void]
**
******************************************************************************/

void ajSeqTypeUnused(void)
{
    AjPStr ajpstr=NULL;
    AjPRegexp ajpregexp = NULL;

    seqTypeTest(ajpstr, ajpregexp);
    return;
}
