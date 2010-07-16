/******************************************************************************
** @source AJAX codon functions
**
** @author Copyright (C) 1999 Alan Bleasby
** @version 1.0
** @modified Aug 07 ajb First version
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

#include <math.h>
#include "ajax.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include <limits.h>

static AjPStr codReadLine = NULL;
static AjPStr codTmpLine = NULL;


static void   codCalcFraction(AjPCod thys);
static AjBool codCommentProcess(AjPCod thys, const AjPStr line);
static void   codFix(AjPCod thys);
static AjBool codGcgProcess(AjPCod thys, const AjPStr line);
static AjBool codIsAa(AjPStr* token);
static AjBool codIsAa3(AjPStr* token, ajint* idx);
static AjBool codIsCodon(AjPStr* token);
static AjBool codIsFraction(const AjPStr token);
static AjBool codIsFreq(const AjPStr token);
static AjBool codIsNumber(const AjPStr token);
static AjBool codIsNumberF(AjPStr* token);
static double codRandom(ajint NA, ajint NC, ajint NG, ajint NT, ajint len,
			const char *p);
static AjBool codReadCodehop(AjPCod thys, AjPFilebuff inbuff);
static AjBool codReadCutg(AjPCod thys, AjPFilebuff inbuff);
static AjBool codReadEmboss(AjPCod thys, AjPFilebuff inbuff);
static AjBool codReadGcg(AjPCod thys, AjPFilebuff inbuff);
static AjBool codReadSpsum(AjPCod thys, AjPFilebuff inbuff);
static AjBool codReadStaden(AjPCod thys, AjPFilebuff inbuff);
static AjBool codTripletAdd (const AjPCod thys, const char residue,
			     char triplet[4]);
static void   codTripletBases(char* triplet);
static void   codWriteCherry(const AjPCod thys, AjPFile outf);
static void   codWriteCodehop(const AjPCod thys, AjPFile outf);
static void   codWriteCutg(const AjPCod thys, AjPFile outf);
static void   codWriteCutgaa(const AjPCod thys, AjPFile outf);
static void   codWriteEmboss(const AjPCod thys, AjPFile outf);
static void   codWriteGcg(const AjPCod thys, AjPFile outf);
static void   codWriteStaden(const AjPCod thys, AjPFile outf);
static void   codWriteSpsum(const AjPCod thys, AjPFile outf);
static void   codWriteTransterm(const AjPCod thys, AjPFile outf);
static double* codGetWstat(const AjPCod thys);

#define AJCODSIZE 66
#define AJCODSTART 64
#define AJCODSTOP 65
#define AJCODEND  65
#define AJCODAMINOS 28

static const char *spsumcodons[]=
{
    "CGA","CGC","CGG","CGT","AGA","AGG","CTA","CTC",
    "CTG","CTT","TTA","TTG","TCA","TCC","TCG","TCT",
    "AGC","AGT","ACA","ACC","ACG","ACT","CCA","CCC",
    "CCG","CCT","GCA","GCC","GCG","GCT","GGA","GGC",
    "GGG","GGT","GTA","GTC","GTG","GTT","AAA","AAG",
    "AAC","AAT","CAA","CAG","CAC","CAT","GAA","GAG",
    "GAC","GAT","TAC","TAT","TGC","TGT","TTC","TTT",
    "ATA","ATC","ATT","ATG","TGG","TAA","TAG","TGA"
};

static const char *spsumaa= "RRRRRRLLLLLLSSSSSSTTTTPPPPAAAAGG"
                      "GGVVVVKKNNQQHHEEDDYYCCFFIIIMW***";




/* @datastatic CodPInFormat ***************************************************
**
** Codon usage input formats data structure
**
** @alias CodSInFormat
** @alias CodOInFormat
**
** @attr Name [const char*] Format name
** @attr Try [AjBool] If true, try for an unknown input. Duplicate names
**                    and read-anything formats are set false
** @attr Padding [ajint] Padding to alignment boundary
** @attr Desc [const char*] Format description
** @attr Read [(AjBool*)] Input function, returns ajTrue on success
** @attr Comment [const char*] Format comments
** @@
******************************************************************************/

typedef struct CodSInFormat
{
    const char *Name;
    AjBool Try;
    ajint Padding;
    const char *Desc;
    AjBool (*Read) (AjPCod thys, AjPFilebuff inbuff);
    const char *Comment;
} CodOInFormat;

#define CodPInFormat CodOInFormat*


static CodOInFormat codInFormatDef[] =
{
/* "Name",      Try      "Description" */
/*     ReadFunction      "Comment" */
  {"emboss",    AJTRUE, 0, "EMBOSS codon usage file",
       codReadEmboss,    "All numbers read, #comments for extras"},
  {"cut",       AJFALSE, 0, "EMBOSS codon usage file",
       codReadEmboss,    "Same as EMBOSS, output default format is 'cut'"},
  {"gcg",       AJTRUE,  0, "GCG codon usage file",
       codReadGcg,       "All numbers read, #comments for extras"},
  {"cutg",      AJTRUE,  0, "CUTG codon usage file",
       codReadCutg,      "All numbers (cutgaa) read or fraction calculated, extras in first line"},
  {"cutgaa",    AJFALSE, 0, "CUTG codon usage file with aminoacids",
       codReadCutg,      "Cutg with all numbers"},
  {"spsum",     AJTRUE,  0, "CUTG species summary file",
       codReadSpsum,     "Number only, species and CDSs in header"},
  {"cherry",    AJFALSE, 0, "Mike Cherry codonusage database file",
       codReadGcg,       "GCG format with species and CDSs in header"},
  {"transterm", AJFALSE, 0, "TransTerm database file",
       codReadGcg, "GCG format with no extras"},
  {"codehop",   AJTRUE,  0, "FHCRC codehop program codon usage file",
       codReadCodehop,   "Freq only, extras at end"},
  {"staden",    AJTRUE,  0, "Staden package codon usage file with numbers",
      codReadStaden,     "Number only, no extras."},
  {"numstaden", AJFALSE, 0, "Staden package codon usage file with numbers",
      codReadStaden,     "Number only, no extras. Obsolete name for 'staden'"},
  {NULL, 0, 0, NULL, NULL, NULL}
};




/* @datastatic CodPOutFormat **************************************************
**
** Codon usage output formats data structure
**
** @alias CodSOutFormat
** @alias CodOOutFormat
**
** @attr Name [const char*] Format name
** @attr Desc [const char*] Format description
** @attr Write[(void*)] Output function
** @@
******************************************************************************/

typedef struct CodSOutFormat
{
    const char *Name;
    const char *Desc;
    void (*Write) (const AjPCod thys, AjPFile outfile);
} CodOOutFormat;

#define CodPOutFormat CodOOutFormat*


static CodOOutFormat codOutFormatDef[] =
{
/* "Name",      "Description" */
/*     WriteFunction */
  {"emboss",    "EMBOSS codon usage file",
       codWriteEmboss},
  {"cut",       "EMBOSS codon usage file",
       codWriteEmboss},
  {"gcg",       "GCG codon usage file",
       codWriteGcg},
  {"cutg",      "CUTG codon usage file",
       codWriteCutg},
  {"cutgaa",    "CUTG codon usage file with aminoacids",
       codWriteCutgaa},
  {"spsum",     "CUTG species summary file",
       codWriteSpsum},
  {"cherry",    "Mike Cherry codonusage database file",
       codWriteCherry},
  {"transterm", "TransTerm database file",
       codWriteTransterm},
  {"codehop",   "FHCRC codehop program codon usage file",
       codWriteCodehop},
  {"staden",    "Staden package codon usage file with numbers",
       codWriteStaden},
  {"numstaden", "Staden package codon usage file with numbers",
       codWriteStaden},
  {NULL, NULL, NULL}
};




/* @section Codon Constructors ************************************************
**
** All constructors return a new object by pointer. It is the responsibility
** of the user to first destroy any previous object. The target pointer
** does not need to be initialised to NULL, but it is good programming practice
** to do so anyway.
**
******************************************************************************/




/* @func ajCodNew *************************************************************
**
** Default constructor for empty AJAX codon objects.
**
** @return [AjPCod] Pointer to an codon object
** @@
******************************************************************************/

AjPCod ajCodNew(void)
{
    AjPCod thys;

    AJNEW0(thys);

    thys->Name = ajStrNew();
    thys->Species = ajStrNew();
    thys->Division = ajStrNew();
    thys->Release = ajStrNew();
    thys->Desc = ajStrNew();
    AJCNEW0(thys->back, AJCODAMINOS);
    AJCNEW0(thys->aa, AJCODSIZE);
    AJCNEW0(thys->num, AJCODSIZE);
    AJCNEW0(thys->tcount, AJCODSIZE);
    AJCNEW0(thys->fraction, AJCODSIZE);

    return thys;
}




/* @func ajCodNewCodenum *******************************************************
**
** Default constructor for empty AJAX codon usage objects, with the
** amino acid assignments taken from a standard genetic code.
**
** @param [r] code [ajint] Genetic code number
** @return [AjPCod] Pointer to an codon object
** @@
******************************************************************************/

AjPCod ajCodNewCodenum(ajint code)
{
    AjPCod thys;
    AjPTrn trn = NULL;
    AjPStr aa = NULL;
    ajint i;
    ajint idx;
    ajint c;

    trn = ajTrnNewI(code);

    if(!trn)
	return NULL;

    AJNEW0(thys);

    thys->Name = ajStrNew();
    thys->Species = ajStrNew();
    thys->Division = ajStrNew();
    thys->Release = ajStrNew();
    thys->Desc = ajStrNew();
    AJCNEW0(thys->back, AJCODAMINOS);
    AJCNEW0(thys->aa, AJCODSIZE);
    AJCNEW0(thys->num, AJCODSIZE);
    AJCNEW0(thys->tcount, AJCODSIZE);
    AJCNEW0(thys->fraction, AJCODSIZE);

    for(i=0;i<64;i++)
    {
	idx = ajCodIndexC(spsumcodons[i]);
	ajStrAssignK(&aa, ajTrnCodonC(trn, spsumcodons[i]));
	c = ajBasecodeToInt((ajint)ajStrGetCharFirst(aa));

	if(c>25)
	    c=27;

	thys->aa[idx] = c;
    }

    ajTrnDel(&trn);
    ajStrDel(&aa);

    return thys;
}




/* @obsolete ajCodNewCode
** @rename ajCodNewCodenum
*/

__deprecated AjPCod ajCodNewCode(ajint code)
{
    return ajCodNewCodenum(code);
}




/* @func ajCodNewCod ***********************************************************
**
** Duplicate a codon object
**
** @param [r] thys [const AjPCod] Codon to duplicate
**
** @return [AjPCod] Pointer to an codon object
** @@
******************************************************************************/

AjPCod ajCodNewCod(const AjPCod thys)
{
    AjPCod dupcod;
    ajint  i;

    dupcod = ajCodNew();

    ajStrAssignS(&dupcod->Name,thys->Name);
    ajStrAssignS(&dupcod->Species,thys->Species);
    ajStrAssignS(&dupcod->Division,thys->Division);
    ajStrAssignS(&dupcod->Release,thys->Release);
    ajStrAssignS(&dupcod->Desc,thys->Desc);

    dupcod->CdsCount = thys->CdsCount;
    dupcod->CodonCount = thys->CodonCount;
    dupcod->GeneticCode = thys->GeneticCode;

    for(i=0;i<AJCODSIZE;++i)
    {
	dupcod->aa[i]       = thys->aa[i];
	dupcod->num[i]      = thys->num[i];
	dupcod->tcount[i]   = thys->tcount[i];
	dupcod->fraction[i] = thys->fraction[i];
    }

    for(i=0;i<AJCODAMINOS;++i)
	dupcod->back[i] = thys->back[i];

    return dupcod;
}




/* @obsolete ajCodDup
** @rename ajCodNewCod
*/

__deprecated AjPCod ajCodDup(const AjPCod thys)
{
    return ajCodNewCod(thys);
}




/* @section Codon Destructors ************************************************
**
** Destructor(s) for AjPCod objects
**
******************************************************************************/




/* @func ajCodDel *************************************************************
**
** Default destructor for AJAX codon objects.
**
** @param [w] pthys [AjPCod *] codon usage structure
**
** @return [void]
** @@
******************************************************************************/

void ajCodDel(AjPCod *pthys)
{
    AjPCod thys = *pthys;

    if(!thys)
        return;

    AJFREE(thys->fraction);
    AJFREE(thys->tcount);
    AJFREE(thys->num);
    AJFREE(thys->aa);
    AJFREE(thys->back);

    ajStrDel(&thys->Name);
    ajStrDel(&thys->Species);
    ajStrDel(&thys->Division);
    ajStrDel(&thys->Release);
    ajStrDel(&thys->Desc);

    AJFREE(*pthys);

    return;
}




/* @section Codon Functions ************************************************
**
** Function(s) for AjPCod objects
**
******************************************************************************/




/* @func ajCodBacktranslate ***************************************************
**
** Back translate a string
**
** @param [w] b [AjPStr *] back translated sequence
** @param [r] a [const AjPStr] sequence
** @param [r] thys [const AjPCod] codon usage object
**
** @return [void]
** @@
******************************************************************************/
void ajCodBacktranslate(AjPStr *b, const AjPStr a, const AjPCod thys)
{
    const char *p;
    char q;
    ajint idx;

    ajStrAssignClear(b);

    p = ajStrGetPtr(a);

    while(*p)
    {
	if(*p=='-')
	{
	    ++p;
	    continue;
	}

	if(toupper((int)*p)==(int)'X')
	{
	    ajStrAppendC(b,"NNN");
	    ++p;
	    continue;
	}

	q = *p;

	if(toupper((int)q)==(int)'B')
	  q = 'D';

	if(toupper((int)q)==(int)'Z')
	  q = 'E';

	idx = thys->back[ajBasecodeToInt(q)];
/*
	if(thys->aa[idx]==27)
	{
	    ajStrAppendC(b,"End");
	    ++p;
	    continue;
	}
*/
	ajStrAppendC(b,ajCodTriplet(idx));
	++p;
    }

    return;
}




/* @func ajCodBacktranslateAmbig **********************************************
**
** Back translate a string to a fully ambiguous nucleotide sequence as a string
**
** @param [w] b [AjPStr *] back translated sequence
** @param [r] a [const AjPStr] sequence
** @param [r] thys [const AjPCod] codon usage object
**
** @return [void]
** @@
******************************************************************************/

void ajCodBacktranslateAmbig(AjPStr *b, const AjPStr a, const AjPCod thys)
{
    const char *p;
    char q;
    char codon[4] = "NNN";

    ajStrAssignClear(b);

    ajDebug("ajCodBacktranslateAmbig '%S'\n", a);
    p = ajStrGetPtr(a);

    while(*p)
    {
	codon[0] = codon[1] = codon[2] = '\0';

	q = *p;

	if(toupper((int)q)==(int)'-')
	{
	    strcpy(codon, "---");
	}
	else if(toupper((int)q)==(int)'X')
	{
	    strcpy(codon,"NNN");
	}
	else if(toupper((int)q)==(int)'B')
	{
	    codTripletAdd(thys, 'D', codon);
	    codTripletAdd(thys, 'N', codon);
	    codTripletBases(codon);
	}
	else if(toupper((int)q)==(int)'Z')
	{
	    codTripletAdd(thys, 'E', codon);
	    codTripletAdd(thys, 'Q', codon);
	    codTripletBases(codon);
	}
	else if(toupper((int)q)==(int)'J') /* Unlikely - NMR code */
	{
	    codTripletAdd(thys, 'I', codon);
	    codTripletAdd(thys, 'L', codon);
	    codTripletBases(codon);
	}
	else if(toupper((int)q)==(int)'U') /* selenocysteine if found */
	{
	    strcpy(codon,"TGA");
	}
	else
	{
	    codTripletAdd(thys, q, codon);
	    codTripletBases(codon);
	}

	ajStrAppendC(b,codon);
	++p;
    }

    return;
}




/* @funcstatic codTripletAdd **************************************************
**
** Add all possible bases at each position to a codon for one amino acid
**
** @param [r] thys [const AjPCod] Codon usage table
** @param [r] residue [const char] Amino acid code
** @param [u] triplet [char[4]] Codon triplet
** @return [AjBool] ajTrue on success, ajFalse if the amino acid is not found..
******************************************************************************/

static AjBool codTripletAdd (const AjPCod thys,
			     const char residue, char triplet[4])
{
    AjBool ret = ajFalse;
    ajint i;
    char  aa;
    char* codon;

    aa = (char) toupper((ajint) residue);

    for (i=0;i<AJCODSTART;++i)
    {
	ajDebug("testing '%c' %d '%c' %2x%2x%2x\n",
		residue, i, ajBasecodeFromInt(thys->aa[i]),
		triplet[0], triplet[1], triplet[2]);

	if(ajBasecodeFromInt(thys->aa[i]) == aa)
	{
	    codon = ajCodTriplet(i);
	    ajDebug("codTripletAdd '%c' %d '%c' %2x%2x%2x '%s'\n",
		    residue, i, ajBasecodeFromInt(thys->aa[i]),
		    triplet[0], triplet[1], triplet[2], codon);
	    triplet[0] |= ajBaseAlphaToBin(codon[0]);
	    triplet[1] |= ajBaseAlphaToBin(codon[1]);
	    triplet[2] |= ajBaseAlphaToBin(codon[2]);
	    ajDebug("codTripletAdd now %2x%2x%2x\n",
		    triplet[0], triplet[1], triplet[2]);
	    ret = ajTrue;
	}
    }

    return ret;
}




/* @funcstatic codTripletBases ************************************************
**
** Converts a triplet of binary codes to base codes
**
** @param [u] triplet [char*] Triplet in binary a=1 C=2 G=4 T=8
** return [void]
******************************************************************************/

static void codTripletBases(char* triplet)
{
    ajint j;

    for(j=0;j<3;j++)
	triplet[j] = ajBaseBinToAlpha(triplet[j]);

    return;
}




/* @func ajCodBase ************************************************************
**
** Return one codon value given a possibly ambiguous base
**
** @param [r] c [ajint] base
**
** @return [ajint] single base value
** @@
******************************************************************************/
ajint ajCodBase(ajint c)
{
    ajint v;

    v = ajBaseAlphaToBin(c);

    if(v & 1)
        return 0;

    if(v & 2)
        return 1;

    if(v & 4)
        return 2;

    if(v & 8)
        return 3;

    return 0;
}




/* @func ajCodClear ***********************************************************
**
** Zero all entries
**
** To retain the genetics code (the amino acids for each codon) use
** ajCodClearData instead.
**
** @param [w] thys [AjPCod] codon usage structure
**
** @return [void]
** @@
******************************************************************************/
void ajCodClear(AjPCod thys)
{
    ajint i;

    ajStrAssignClear(&thys->Name);
    ajStrAssignClear(&thys->Species);
    ajStrAssignClear(&thys->Division);
    ajStrAssignClear(&thys->Release);
    ajStrAssignClear(&thys->Desc);

    thys->CdsCount = 0;
    thys->CodonCount = 0;
    thys->GeneticCode = 0;

    for(i=0;i<AJCODSIZE;++i)
    {
	thys->fraction[i] = 0.0;
	thys->tcount[i] = 0.0;
	thys->num[i] = 0;
	thys->aa[i] = 0;
    }

    for(i=0;i<AJCODAMINOS;++i)
	thys->back[i] = 0;

    return;
}




/* @func ajCodClearData *******************************************************
**
** Zero the name, number count and fraction codon entries
**
** @param [w] thys [AjPCod] codon usage structure
**
** @return [void]
** @@
******************************************************************************/
void ajCodClearData(AjPCod thys)
{
    ajint i;

    ajStrAssignClear(&thys->Name);
    ajStrAssignClear(&thys->Species);
    ajStrAssignClear(&thys->Division);
    ajStrAssignClear(&thys->Release);
    ajStrAssignClear(&thys->Desc);

    thys->CdsCount = 0;
    thys->CodonCount = 0;
    thys->GeneticCode = 0;

    for(i=0;i<AJCODSIZE;++i)
    {
	thys->fraction[i] = 0.0;
	thys->tcount[i] = 0.0;
	thys->num[i] = 0;
    }

    for(i=0;i<AJCODAMINOS;++i)
	thys->back[i] = 0;

    return;
}




/* @func ajCodSetTripletsS *****************************************************
**
** Load the num array of a codon structure
**
** Used for creating a codon usage table
**
** Skips triplets with ambiguity codes and any incomplete triplet at the end.
**
** @param [w] thys [AjPCod] Codon object
** @param [r] s [const AjPStr] dna sequence
** @param [w] c [ajint *] triplet count
**
** @return [void]
** @@
******************************************************************************/
void ajCodSetTripletsS(AjPCod thys, const AjPStr s, ajint *c)
{
    const char *p;
    ajuint  last;
    ajuint  i;
    ajint  idx;

    p = ajStrGetPtr(s);
    last = ajStrGetLen(s)-2;

    for(i=0;i<last;i+=3,p+=3,++(*c))
    {
	if((idx=ajCodIndexC(p))!=-1)
	    ++thys->num[idx];
	else
	{
	    ajDebug("ajCodSetTripletsS skipping triplet %3.3s\n", p);
	    --(*c);
	}
    }

    return;
}





/* @obsolete ajCodCountTriplets
** @rename ajCodSetTripletsS
*/

__deprecated void ajCodCountTriplets(AjPCod thys, const AjPStr s, ajint *c)
{
    ajCodSetTripletsS(thys, s, c);
    return;
}




/* @func ajCodIndex ***********************************************************
**
** Return a codon index given a three character codon
**
** Can be called with any coding sequence of 3 or more bases, and will
** use only the first 3 bases of the input.
**
** @param [r] s [const AjPStr] Codon
**
** @return [ajint] Codon index AAA=0 TTT=3f
** @@
******************************************************************************/

ajint ajCodIndex(const AjPStr s)
{
    return ajCodIndexC(ajStrGetPtr(s));
}




/* @func ajCodIndexC **********************************************************
**
** Return a codon index given a three character codon
**
** Can be called with any coding sequence of 3 or more bases, and will
** use only the first 3 bases of the input.
**
** @param [r] codon [const char *] Codon pointer
**
** @return [ajint] codon index AAA=0 TTT=3f
** @@
******************************************************************************/

ajint ajCodIndexC(const char *codon)
{
    const char *p;
    ajint  sum;
    ajint c;

    p   = codon;
    sum = 0;

    if(!(c=*(p++)))
	return -1;

    sum += (ajCodBase(toupper(c))<<4);

    if(!(c=*(p++)))
	return -1;

    sum += (ajCodBase(toupper(c))<<2);

    if(!(c=*p))
	return -1;

    sum += ajCodBase(toupper(c));

    return sum;
}




/* @func ajCodRead ************************************************************
**
** Read a codon index from a filename using a specified format.
**
** The format can be in the format argument, as a prefix format:: to the
** filename, or empty to allow all known formats to be tried.
**
** @param [w] thys [AjPCod] Codon object
** @param [r] fn [const AjPStr] filename
** @param [r] format [const AjPStr] format
**
** @return [AjBool] ajTrue on success
** @category input [AjPCod] Read codon index from a file
** @@
******************************************************************************/
AjBool ajCodRead(AjPCod thys, const AjPStr fn, const AjPStr format)
{
    AjBool ret = ajFalse;
    AjPFile inf = NULL;
    AjPFilebuff inbuff = NULL;
    AjPStr formatstr = NULL;
    AjPStr fname = NULL;
    AjPStr filename = NULL;
    ajint i;

    i = ajStrFindC(fn, "::");

    if(i == -1)
    {
	ajStrAssignS(&filename, fn);
	ajStrAssignS(&formatstr, format);
    }
    else
    {
	ajStrAssignSubS(&formatstr, fn, 0, i-1);
	ajStrAssignSubS(&fname, fn, i+1, -1);
    }

    fname = ajStrNewS(filename);

    inf = ajDatafileNewInNameS(fname);

    if(!inf)
    {
	ajStrAssignC(&fname,"CODONS/");
	ajStrAppendS(&fname,filename);
	inf = ajDatafileNewInNameS(fname);

	if(!inf)
	{
	    ajStrDel(&fname);
	    return ajFalse;
	}
    }
    ajStrDel(&fname);
    inbuff = ajFilebuffNewFromFile(inf);


    for(i=0;codInFormatDef[i].Name; i++)
    {
	if(ajStrGetLen(formatstr))
	{
	    if(!ajStrMatchC(formatstr, codInFormatDef[i].Name))
		continue;
	}
	else
	{
	    if(!codInFormatDef[i].Try)
		continue;
	}

	ajDebug("ajCodRead Try format '%s'\n", codInFormatDef[i].Name);
	ret = codInFormatDef[i].Read(thys, inbuff);

	if(ret)
	{
	    ajStrAssignS(&thys->Name, filename);
	    codFix(thys);
	    ajDebug("ajCodRead Format '%s' success\n", codInFormatDef[i].Name);
	    ajFilebuffDel(&inbuff);
	    ajStrDel(&filename);
	    ajStrDel(&formatstr);
	    return ajTrue;
	}

	ajDebug("ajCodRead Format '%s' failed\n", codInFormatDef[i].Name);
	ajCodClear(thys);
	ajFilebuffReset(inbuff);
    }

    ajFilebuffDel(&inbuff);
    ajStrDel(&filename);
    ajStrDel(&formatstr);

    return ret;
}




/* @funcstatic codReadEmboss **************************************************
**
** Read a codon index from a filename in EMBOSS format
**
** First readable line format is either the comment or the data
**
** #Codon AA Fraction Frequency Number<br>
** GCA     A            0.070      7.080   544<br>
**
** @param [w] thys [AjPCod] Codon object
** @param [u] inbuff [AjPFilebuff] Input file buffer
**
** @return [AjBool] ajTrue on success
** @category input [AjPCod] Read codon index from a file
** @@
******************************************************************************/
static AjBool codReadEmboss(AjPCod thys, AjPFilebuff inbuff)
{
    AjPStr  t;
    const char    *p;
    ajint     idx;
    ajint     c;
    ajint     num;
    double  tcount;
    double  fraction;
    AjPStr tok = NULL;
    AjPStrTok handle = NULL;

    t    = ajStrNew();

    while(ajBuffreadLine(inbuff,&codReadLine))
    {
	p=ajStrGetPtr(codReadLine);

	if(*p=='\n')
	    continue;
	else if(*p=='#')
	{
	    codCommentProcess(thys, codReadLine);
	    continue;
	}
	else if(*p=='!')
	    continue;

	/* check record format */

	if(ajStrParseCountC(codReadLine, " \t\r\n") != 5)
	    return ajFalse;

	ajStrTokenAssignC(&handle, codReadLine, " \t\r\n");
	ajStrTokenNextParse(&handle, &tok);

	if(!codIsCodon(&tok))
	    return ajFalse;

	idx = ajCodIndex(tok);

	ajStrTokenNextParse(&handle, &tok);

	if(!codIsAa(&tok))
	    return ajFalse;

	c = ajBasecodeToInt((ajint)ajStrGetCharFirst(tok));

	if(c>25)
	    c=27;			/* stop */

	ajStrTokenNextParse(&handle, &tok);

	if(!codIsFraction(tok))
	    return ajFalse;

	ajStrToDouble(tok,&fraction);

	ajStrTokenNextParse(&handle, &tok);
	if(!codIsFreq(tok))
	    return ajFalse;
	ajStrToDouble(tok,&tcount);

	ajStrTokenNextParse(&handle, &tok);

	if(!codIsNumber(tok))
	    return ajFalse;

	ajStrToInt(tok,&num);


	thys->aa[idx]       = c;
	thys->num[idx]      = num;
	thys->tcount[idx]   = tcount;
	thys->fraction[idx] = fraction;
    }

    ajStrDel(&t);
    ajStrTokenDel(&handle);
    ajStrDel(&tok);

    return ajTrue;
}




/* @funcstatic codReadStaden **********************************************
**
** Read a codon index from a filename in Staden format with counts.
**
** The first lines have the format:
**
**      ===========================================<br>
**      F TTT  170 S TCT  160 Y TAT  10 C TGT   20<br>
**
** Note that in this format the Staden package allows two codon usage tables
** in the same file. The first is the one we want as it is for the protein
** coding regions. The second is for the regions between CDSs.
**
** @param [w] thys [AjPCod] Codon object
** @param [u] inbuff [AjPFilebuff] Input file buffer
**
** @return [AjBool] ajTrue on success
** @category input [AjPCod] Read codon index from a file
** @@
******************************************************************************/

static AjBool codReadStaden(AjPCod thys, AjPFilebuff inbuff)
{
    AjPStr tok = NULL;
    AjPStrTok handle = NULL;
    ajint i;
    ajint c;
    ajint idx;
    ajint num;
    double tcount;
    ajint cdscount = 0;
    ajint icod = 0;

    while(ajBuffreadLine(inbuff,&codReadLine))
    {
	ajStrRemoveWhiteExcess(&codReadLine);

	if(!ajStrGetLen(codReadLine))
	    continue;

	if(ajStrGetCharFirst(codReadLine) == '#')
	{
	    codCommentProcess(thys, codReadLine);
	    continue;
	}

	if(ajStrGetCharFirst(codReadLine) == '!')
	    continue;

	if(ajStrFindRestC(codReadLine, "=") == -1)
	    continue;

	if(icod > 63)			/* ignore second non-coding set */
	    continue;

	/* check record format */

	if(ajStrParseCountC(codReadLine, " \t\r\n") != 12)
	    return ajFalse;

	ajStrTokenAssignC(&handle, codReadLine, " \t\r\n");
	for(i=0;i<4;i++)
	{
	    ajStrTokenNextParse(&handle, &tok);

	    if(!codIsAa(&tok))
		return ajFalse;

	    c = ajBasecodeToInt((ajint)ajStrGetCharFirst(tok));

	    if(c>25)
		c=27;			/* stop */

	    ajStrTokenNextParse(&handle, &tok);

	    if(!codIsCodon(&tok))
		return ajFalse;

	    idx = ajCodIndex(tok);

	    ajStrTokenNextParse(&handle, &tok);

	    if(!codIsNumber(tok))
		return ajFalse;

	    ajStrToDouble(tok,&tcount);
	    ajStrToInt(tok, &num);

	    thys->aa[idx]       = c;
	    cdscount += num;
	    thys->num[idx]  = num;
	    thys->tcount[idx]   = 0.0;  /* unknown - can calculate */
	    thys->fraction[idx] = 0.0;	/* unknown fraction - can calculate */

	    icod++;

	}
    }

    codCalcFraction(thys);

    ajStrTokenDel(&handle);
    ajStrDel(&tok);

    return ajTrue;
}




/* @funcstatic codReadSpsum **************************************************
**
** Read a codon index from a filename in CUTG .spsum file format
**
** @param [w] thys [AjPCod] Codon object
** @param [u] inbuff [AjPFilebuff] Input file buffer
**
** @return [AjBool] ajTrue on success
** @category input [AjPCod] Read codon index from a file
** @@
******************************************************************************/

static AjBool codReadSpsum(AjPCod thys, AjPFilebuff inbuff)
{
    AjPStrTok handle = NULL;
    AjPStr tok = NULL;
    ajint i;
    ajint j;
    ajint c;
    ajint num;

    if(!ajBuffreadLine(inbuff,&codReadLine))
	return ajFalse;

    if(ajStrParseCountC(codReadLine, ":") != 2)
	return ajFalse;

    ajStrTokenAssignC(&handle, codReadLine, ":");
    ajStrTokenNextParse(&handle, &tok);
    ajStrTrimWhite(&tok);
    ajStrAssignS(&thys->Species, tok);

    ajStrTokenNextParse(&handle, &tok);
    ajStrTrimWhite(&tok);

    if(!ajStrToInt(tok, &thys->CdsCount))
	return ajFalse;

    if(!ajBuffreadLine(inbuff,&codReadLine))
	return ajFalse;

    if(ajStrParseCountC(codReadLine, " \t\n\r") != 64)
	return ajFalse;

    ajStrTokenAssignC(&handle, codReadLine, " \t\r\n");

    for(i=0;i<64;i++)
    {
	ajStrTokenNextParse(&handle, &tok);

	if(!ajStrToInt(tok, &num))
	    return ajFalse;

	j = ajCodIndexC(spsumcodons[i]);
	thys->num[j] = num;
	c = ajBasecodeToInt((ajint)spsumaa[i]);

	if(c>25)
	    c=27;

	thys->aa[j] = c;
    }

    ajStrTokenDel(&handle);
    ajStrDel(&tok);

    return ajTrue;
}




/* @funcstatic codReadCutg **************************************************
**
** Read a codon index from a filename in CUTG web site format
**
** @param [w] thys [AjPCod] Codon object
** @param [u] inbuff [AjPFilebuff] Input file buffer
**
** @return [AjBool] ajTrue on success
** @category input [AjPCod] Read codon index from a file
** @@
******************************************************************************/
static AjBool codReadCutg(AjPCod thys, AjPFilebuff inbuff)
{
    AjPStr tok = NULL;
    AjPStrTok handle = NULL;
    ajint i;
    ajint j;
    AjBool hasaa;
    ajint ilines = 0;
    ajint c;
    ajint idx;
    ajint num;
    double  tcount;
    double  fraction;
    AjPTrn trn = NULL;

    /* species [division]: 000 CDS's (0000 codons)  */

    ajBuffreadLine(inbuff,&codReadLine);

    if(-1 == ajStrFindC(codReadLine, "CDS's"))
	return ajFalse;

    if(-1 == ajStrFindC(codReadLine, "codons)"))
	return ajFalse;

    if(-1 == ajStrFindC(codReadLine, "]:"))
	return ajFalse;

    ajStrTokenAssignC(&handle, codReadLine, "[");

    ajStrTokenNextParse(&handle, &tok);

    if(!ajStrGetLen(tok))
	return ajFalse;

    ajStrRemoveWhiteExcess(&tok);
    ajStrAssignS(&thys->Species, tok);

    ajStrTokenNextParseC(&handle, "]: ", &tok);

    if(!ajStrGetLen(tok))
	return ajFalse;

    ajStrAssignS(&thys->Division, tok);

    ajStrTokenNextParseC(&handle, "CDS's (", &tok);

    if(!codIsNumber(tok))
	return ajFalse;

    ajStrToInt(tok, &thys->CdsCount);

    ajStrTokenNextParseC(&handle, " codons)", &tok);

    if(!codIsNumber(tok))
	return ajFalse;

    ajStrToInt(tok, &thys->CodonCount);


    while(ajBuffreadLine(inbuff,&codReadLine))
    {
	ajStrTrimWhite(&codReadLine);

	if(ajStrPrefixC(codReadLine, "Coding GC "))
	    continue;
	else if(ajStrPrefixC(codReadLine, "Genetic code "))
	{
	    ajStrCutStart(&codReadLine, 13);
	    ajStrTokenAssignC(&handle, codReadLine, " :");
	    ajStrTokenNextParse(&handle, &tok);

	    if(codIsNumber(tok))
		ajStrToInt(tok, &thys->GeneticCode);

	    continue;
	}
	else
	{
	    i = ajStrParseCountC(codReadLine, " ()");

	    if(i == 20)
		hasaa = ajTrue;
	    else if(i == 12)
		hasaa = ajFalse;
	    else
		continue;

	    ajStrTokenAssignC(&handle, codReadLine, " ");

	    for(j=0;j<4;j++)
	    {
		ajStrTokenNextParse(&handle, &tok);

		if(!codIsCodon(&tok))
		    return ajFalse;

		idx = ajCodIndex(tok);

		if(hasaa)
		{
		    ajStrTokenNextParse(&handle, &tok);

		    if(!codIsAa(&tok))
			return ajFalse;

		    c = ajBasecodeToInt((ajint)ajStrGetCharFirst(tok));

		    ajStrTokenNextParse(&handle, &tok);

		    if(!codIsFraction(tok))
			return ajFalse;

		    ajStrToDouble(tok,&fraction);
		}
		else
		{
		    if(!trn)
			trn = ajTrnNewI(0);

		    ajStrAssignK(&tok, ajTrnCodonS(trn, tok));
		    c = ajBasecodeToInt((ajint)ajStrGetCharFirst(tok));
		}

		if(c>25)
		    c=27;		/* stop */

		ajStrTokenNextParseC(&handle, " (", &tok);

		if(!codIsFreq(tok))
		    return ajFalse;

		ajStrToDouble(tok,&tcount);

		ajStrTokenNextParseC(&handle, " )", &tok);

		if(!codIsNumber(tok))
		    return ajFalse;

		ajStrToInt(tok,&num);



		thys->aa[idx]       = c;
		thys->num[idx]      = num;
		thys->tcount[idx]   = tcount;

		if(hasaa)
		    thys->fraction[idx] = fraction;
	    }
	    ilines++;
	}
    }

    ajStrTokenDel(&handle);
    ajStrDel(&tok);

    if(!ilines)
	return ajFalse;

    ajTrnDel(&trn);

    return ajTrue;
}




/* @funcstatic codReadCodehop *************************************************
**
** Read a codon index from a filename in FHCRC codehop program format
**
** @param [w] thys [AjPCod] Codon object
** @param [u] inbuff [AjPFilebuff] Input file buffer
**
** @return [AjBool] ajTrue on success
** @category input [AjPCod] Read codon index from a file
** @@
******************************************************************************/

static AjBool codReadCodehop(AjPCod thys, AjPFilebuff inbuff)
{
    AjPStr  line = NULL;
    AjPStrTok handle = NULL;
    AjPStr  tok1 = NULL;
    AjPStr  tok2 = NULL;
    AjPStr  tok3 = NULL;
    AjPStr  tok4 = NULL;
    ajint i;
    ajint c;
    ajint idx;
    double fraction;
    double tcount;
    double res;
    
    AjPTrn trn = NULL;

    while(ajBuffreadLine(inbuff,&line))
    {
	if(ajStrGetCharFirst(line) == '-')
	    break;

	if(ajStrParseCountC(line, " \t\r\n[]") != 4)
	    return ajFalse;

	ajStrTokenAssignC(&handle, line, " \t\r\n[]");
	ajStrTokenNextParse(&handle, &tok1);
	ajStrTokenNextParse(&handle, &tok2);
	ajStrTokenNextParse(&handle, &tok3);
	ajStrTokenNextParse(&handle, &tok4);

	if(!codIsFreq(tok1))
	    return ajFalse;

	if(!codIsCodon(&tok4))
	    return ajFalse;

	ajStrToDouble(tok1, &fraction);
	tcount = fraction * 1000.0;

	if(!trn)
	    trn = ajTrnNewI(0);

	ajStrAssignK(&tok2, ajTrnCodonS(trn, tok4));
	c = ajBasecodeToInt((ajint)ajStrGetCharFirst(tok2));

	if(c>25)
	    c=27;			/* stop */

	idx = ajCodIndex(tok4);
	thys->aa[idx]       = c;
	thys->tcount[idx]   = tcount;
    }

    ajBuffreadLine(inbuff,&line);
    ajBuffreadLine(inbuff,&line);
    ajStrRemoveWhiteExcess(&line);

    if(ajStrPrefixC(line, "Codon usage for "))
    {
	ajStrCutStart(&line, 16);
	i = ajStrFindlastC(line, ":");

	if(i>0)
	{
	    ajStrAssignSubS(&thys->Species, line, 0, i-1);
	    ajStrAssignSubS(&tok1, line, i+1, -1);
	    ajStrRemoveWhiteExcess(&tok1);

	    if(codIsNumber(tok1))
		ajStrToInt(tok1, &thys->CdsCount);
	}
    }
    ajBuffreadLine(inbuff,&line);
    ajStrRemoveWhiteExcess(&line);

    if(ajStrPrefixC(line, "from "))
    {
	ajStrCutStart(&line, 5);
	ajStrRemoveWhiteExcess(&line);
	i = ajStrFindlastC(line, ":");

	if(i>0)
	{
	    ajStrAssignSubS(&tok1, line, 0, i-1);
	    ajStrAssignSubS(&tok2, line, i+1, -1);
	    ajStrRemoveWhiteExcess(&tok1);
	    ajStrRemoveWhiteExcess(&tok2);
	    i = ajStrFindC(tok1, "/");

	    if(i>0)
	    {
		ajStrAssignSubS(&thys->Release, tok1, 0, i-1);
		ajStrAssignSubS(&thys->Division, tok1, i+1, -1);
	    }
	    else
		ajStrAssignS(&thys->Release, tok1);

	    i = ajStrFindC(tok2, " ");

	    if(i>0)
	    {
		ajStrAssignSubS(&tok3, tok2, 0, i-1);

		if(codIsNumber(tok3))
		    ajStrToInt(tok3, &thys->CodonCount);
	    }
	}
    }

    if(thys->CodonCount)
	for(i=0;i<64;i++)
	{
	    res = ((double)thys->CodonCount* thys->tcount[i] / 1000.0 + 0.1);
	    thys->num[i] = (ajint) res;
	}

    ajStrDel(&tok1);
    ajStrDel(&tok2);
    ajStrDel(&tok3);
    ajStrDel(&tok4);
    ajStrDel(&line);
    ajStrTokenDel(&handle);
    ajTrnDel(&trn);

    return ajTrue;
}




/* @funcstatic codReadGcg *****************************************************
**
** Read a codon index from a filename in GCG format
**
** @param [w] thys [AjPCod] Codon object
** @param [u] inbuff [AjPFilebuff] Input file buffer
**
** @return [AjBool] ajTrue on success
** @category input [AjPCod] Read codon index from a file
** @@
******************************************************************************/

static AjBool codReadGcg(AjPCod thys, AjPFilebuff inbuff)
{
    AjPStr  line = NULL;
    AjBool header = ajTrue;
    AjPStr saveheader = NULL;
    AjPStr tok1 = NULL;
    ajint i;

    while(ajBuffreadLine(inbuff,&line))
    {
	if(ajStrSuffixC(line, "..\n"))
	{
	    header = ajFalse;
	    continue;
	}

	if(header)
	{
	    ajStrRemoveWhiteExcess(&line);

	    if(!ajStrGetLen(line))
		continue;

	    if(ajStrGetCharFirst(line) == '!')
		continue;

	    if(ajStrGetCharFirst(line) == '#')
	    {
		codCommentProcess(thys, line);
		continue;
	    }

	    i = ajStrFindC(line, " genes found in ");

	    if(i == -1)
		ajStrAssignS(&saveheader, line);
	    else
	    {
		ajStrAssignSubS(&tok1, line, 0, i-1);

		if(!codIsNumber(tok1))
		    continue;

		ajStrToInt(tok1, &thys->CdsCount);
		ajStrAssignSubS(&thys->Release, line, i+16, -1);
		ajStrTrimEndC(&thys->Release, ".");
		ajStrExchangeKK(&thys->Release, ' ', '_');
		ajStrAssignS(&thys->Species, saveheader);
	    }
	    continue;
	}

	codGcgProcess(thys, line);
    }

    ajStrDel(&line);
    ajStrDel(&saveheader);
    ajStrDel(&tok1);

    if(header)
	return ajFalse;

    return ajTrue;
}




/* @funcstatic codGcgProcess ***********************************************
**
** Read a codon index from a filename in GCG format
**
** @param [u] thys [AjPCod] Codon object
** @param [r] line [const AjPStr] Input line
**
** @return [AjBool] ajTrue on success
** @@
******************************************************************************/

static AjBool codGcgProcess(AjPCod thys, const AjPStr line)
{
    AjPStrTok handle = NULL;
    AjPStr tok = NULL;
    ajint idx;
    ajint c;
    ajint num;
    double tcount;
    double fraction;

    if(ajStrParseCountC(line, " \t\n\r") != 5)
	return ajFalse;

    ajStrTokenAssignC(&handle, line, " \t\r\n");

    ajStrTokenNextParse(&handle, &tok);

    if(!codIsAa3(&tok, &c))
	return ajFalse;

    ajStrTokenNextParse(&handle, &tok);

    if(!codIsCodon(&tok))
	return ajFalse;

    idx = ajCodIndex(tok);

    ajStrTokenNextParse(&handle, &tok);

    if(!codIsNumberF(&tok))
	return ajFalse;

    ajStrToInt(tok, &num);

    ajStrTokenNextParse(&handle, &tok);

    if(!codIsFreq(tok))
	return ajFalse;

    ajStrToDouble(tok, &tcount);

    ajStrTokenNextParse(&handle, &tok);

    if(!codIsFraction(tok))
	return ajFalse;

    ajStrToDouble(tok, &fraction);

    thys->aa[idx] = c;
    thys->tcount[idx] = tcount;
    thys->fraction[idx] = fraction;
    thys->num[idx] = num;

    ajStrTokenDel(&handle);
    ajStrDel(&tok);

    return ajTrue;
}




/* @funcstatic codCommentProcess **********************************************
**
** Read codon usage extra information from an EMBOSS-style comment
**
** @param [w] thys [AjPCod] Codon object
** @param [r] ccline [const AjPStr] Input line
**
** @return [AjBool] ajTrue if something was read
** @@
******************************************************************************/

static AjBool codCommentProcess(AjPCod thys, const AjPStr ccline)
{
    ajStrAssignS(&codTmpLine, ccline);

    if(ajStrPrefixC(codTmpLine, "#Species:"))
    {
	ajStrCutStart(&codTmpLine, 9);
	ajStrRemoveWhiteExcess(&codTmpLine);
	ajStrAssignS(&thys->Species, codTmpLine);

	return ajTrue;
    }
    else if(ajStrPrefixC(codTmpLine, "#Division:"))
    {
	ajStrCutStart(&codTmpLine, 10);
	ajStrRemoveWhiteExcess(&codTmpLine);
	ajStrAssignS(&thys->Division, codTmpLine);

	return ajTrue;
    }
    else if(ajStrPrefixC(codTmpLine, "#Release:"))
    {
	ajStrCutStart(&codTmpLine, 9);
	ajStrRemoveWhiteExcess(&codTmpLine);
	ajStrAssignS(&thys->Release, codTmpLine);

	return ajTrue;
    }
    else if(ajStrPrefixC(codTmpLine, "#Description:"))
    {
	ajStrCutStart(&codTmpLine, 13);
	ajStrRemoveWhiteExcess(&codTmpLine);
	ajStrAssignS(&thys->Desc, codTmpLine);

	return ajTrue;
    }
    else if(ajStrPrefixC(codTmpLine, "#CdsCount:"))
    {
	ajStrCutStart(&codTmpLine, 10);
	ajStrRemoveWhiteExcess(&codTmpLine);
	ajStrToInt(codTmpLine, &thys->CdsCount);

	return ajTrue;
    }
    else if(ajStrPrefixC(codTmpLine, "#CodonCount:"))
    {
	ajStrCutStart(&codTmpLine, 12);
	ajStrRemoveWhiteExcess(&codTmpLine);
	ajStrToInt(codTmpLine, &thys->CodonCount);

	return ajTrue;
    }
    else if(ajStrPrefixC(codTmpLine, "#GeneticCode:"))
    {
	ajStrCutStart(&codTmpLine, 13);
	ajStrRemoveWhiteExcess(&codTmpLine);
	ajStrToInt(codTmpLine, &thys->GeneticCode);
    }

    return ajFalse;
}




/* @funcstatic codIsCodon *****************************************************
**
** Checks a string is a codon, makes sure it is ACGT only
**
** @param [u] token [AjPStr*] String
**
** @return [AjBool] ajTrue if string is a valid codon sequence
** @@
******************************************************************************/

static AjBool codIsCodon (AjPStr* token)
{
    char* cp;

    if(ajStrGetLen(*token) != 3)
	return ajFalse;

    for(cp = ajStrGetuniquePtr(token); *cp; cp++)
    {
	if(islower((ajint) *cp))
	    *cp = toupper((ajint) *cp);

	if(*cp == 'U')
	    *cp = 'T';

	if(!strchr("ACGT",*cp))
	    return ajFalse;
    }

    return ajTrue;
}




/* @funcstatic codIsAa ********************************************************
**
** Checks a string is a codon, makes sure it is an amino acid or a stop (*).
**
** @param [u] token [AjPStr*] String
**
** @return [AjBool] ajTrue if string is a valid amino acid code
** @@
******************************************************************************/

static AjBool codIsAa(AjPStr* token)
{
    char* cp;

    if(ajStrGetLen(*token) != 1)
	return ajFalse;

    cp = ajStrGetuniquePtr(token);

    if(islower((ajint) *cp))
	*cp = toupper((ajint) *cp);

    if(!strchr("ACDEFGHIKLMNPQRSTUVWY*",*cp)) /* SelenoCysteine is U */
	    return ajFalse;

    return ajTrue;
}




/* @funcstatic codIsAa3 *******************************************************
**
** Checks a string is a 3 letter amino acid code
**
** @param [u] token [AjPStr*] String
** @param [w] idx [ajint*] Amino acid index number
**
** @return [AjBool] ajTrue if string is a valid amino acid 3-letter code
** @@
******************************************************************************/

static AjBool codIsAa3(AjPStr* token, ajint* idx)
{
    const char* cp;
    ajint i;

    static const char *tab[]=
    {
	"Ala","Asx","Cys","Asp","Glu","Phe","Gly","His",
	"Ile",""   ,"Lys","Leu","Met","Asn",""   ,"Pro",
	"Gln","Arg","Ser","Thr","Sel","Val","Trp",""   ,
	"Tyr",""   ,""   ,"End",NULL
    };

     if(ajStrGetLen(*token) != 3)
	return ajFalse;

    for(i=0;tab[i]; i++)
    {
	cp = tab[i];

	if(!*cp)
	    continue;

	if(ajStrMatchCaseC(*token, cp))
	{
	    if(!ajStrMatchC(*token, cp))
		ajStrAssignC(token, cp);

	    *idx = i;
	    return ajTrue;
	}
    }

    return ajFalse;
}




/* @funcstatic codIsNumber ****************************************************
**
** Checks a string is numeric.
**
** @param [r] token [const AjPStr] String
**
** @return [AjBool] ajTrue if string is a valid integer
** @@
******************************************************************************/

static AjBool codIsNumber(const AjPStr token)
{
    ajint ilen;

    ilen = ajStrGetLen(token);

    if(!ilen)
	return ajFalse;

   if(ajStrFindRestC(token, "0123456789") != -1)
       return ajFalse;

    return ajTrue;
}




/* @funcstatic codIsNumberF ***************************************************
**
** Checks a string is a number, optionally ending in .000
**
** @param [u] token [AjPStr*] String
**
** @return [AjBool] ajTrue if string is a valid floating point number
** @@
******************************************************************************/

static AjBool codIsNumberF(AjPStr * token)
{
    ajuint ilen;
    AjPStr tmpstr = NULL;

    ilen = ajStrGetLen(*token);
    if(!ilen)
	return ajFalse;

    if(ajStrFindRestC(*token, "0123456789") == -1)
        return ajTrue;

    ajStrAssignS(&tmpstr, *token);

    ilen = ajStrGetLen(tmpstr);
    ajStrTrimEndC(&tmpstr, "0");

    if(ajStrGetLen(tmpstr) < ilen)
	ilen = ajStrGetLen(tmpstr);

    ajStrTrimEndC(&tmpstr,".");

    if(ajStrGetLen(tmpstr) < ilen)
    {
	ajStrAssignS(token, tmpstr);
	ajStrDel(&tmpstr);
	return ajTrue;
    }
	
    ajStrDel(&tmpstr);

    return ajFalse;
}




/* @funcstatic codCalcFraction ************************************************
**
** Calculates the fraction values for a codon usage object
**
** @param [u] thys [AjPCod] Codon usage object
**
** @return [void]
** @@
******************************************************************************/

static void codCalcFraction(AjPCod thys)
{
    ajint count[64];
    ajint i;
    ajint j;
    ajint idx;
    ajint icount;
    double fsum = 0.0;
    const char* cp;
    const char* aa = "ACDEFGHIKLMNPQRSTUVWY*";

    cp = aa;
    while (*cp)
    {
	if(*cp == '*')
	    idx = 27;
	else
	    idx = *cp - 'A';

	icount = 0;
	fsum = 0.0;

	for(i=0;i<64;i++)
	{
	    if(thys->aa[i] == idx)
	    {
		count[icount++] = i;
		fsum += thys->tcount[i];
	    }
	}

	for(j=0;j<icount;j++)
	{
            /* so we know we have fsum > 0.0 */
	    if(!E_FPZERO(thys->tcount[count[j]],U_DEPS))
		thys->fraction[count[j]] = thys->tcount[count[j]]/fsum;
	    else
		thys->fraction[count[j]] = 0.0;
	}
	cp++;
    }

    return;
}




/* @funcstatic codIsFraction **************************************************
**
** Checks a string is a fraction between 0.0 and 1.0.
**
** @param [r] token [const AjPStr] String
**
** @return [AjBool] ajTrue if string is a valid fraction between 0.0 and 1.0
** @@
******************************************************************************/

static AjBool codIsFraction(const AjPStr token)
{
    const char* cp;
    ajuint ilen;
    AjBool StartOne = ajFalse;;

    ilen = ajStrGetLen(token);
    if(!ilen)
	return ajFalse;

    cp = ajStrGetPtr(token);

    if(*cp == '0')
    {
	cp++;
	ilen--;
    }
    else if(*cp == '1')
    {
	cp++;
	ilen--;
	StartOne = ajTrue;
    }

    if(*cp == '.')
    {
	cp++;
	ilen--;
    }

    if(!*cp)
	return ajTrue;

    if(StartOne)
    {
	if(ilen != strspn(cp, "0"))
	    return ajFalse;
    }
    else
    {
	if(ilen != strspn(cp, "0123456789"))
	    return ajFalse;
    }

    return ajTrue;
}




/* @funcstatic codIsFreq ******************************************************
**
** Checks a string is a frequency between 0.0 and 1000.0.
**
** @param [r] token [const AjPStr] String
**
** @return [AjBool] ajTrue if string is a valid frequency between
**                  0.0 and 1000.0
** @@
******************************************************************************/

static AjBool codIsFreq(const AjPStr token)
{
    const char* cp;
    float f;
    ajuint ilen;
    
    ilen = ajStrGetLen(token);
    if(!ilen)
	return ajFalse;

    cp = ajStrGetPtr(token);

    if(ilen != strspn(cp, "0123456789."))
	return ajFalse;

    if(!ajStrToFloat(token, &f))
	return ajFalse;

    if(f < 0.0)
	return ajFalse;

    if(f > 1000.0)
	return ajFalse;

    return ajTrue;
}




/* @func ajCodSetBacktranslate ************************************************
**
** Fill the codon usage object "back" element with the most commonly
** used triplet index for the amino acids
**
** @param [u] thys [AjPCod] codon usage structure
**
** @return [void]
** @@
******************************************************************************/

void ajCodSetBacktranslate(AjPCod thys)
{
    ajint i;
    ajint aa;
    double f;
    double f2;

    if(!thys)
	ajFatal("Codon usage object uninitialised");

    for(i=0;i<AJCODAMINOS;++i)
	thys->back[i] = -1;

    for(i=0;i<AJCODSTART;++i)
    {
	aa = thys->aa[i];

	if(thys->back[aa]<0)
	    thys->back[aa] = i;

	f = thys->fraction[i];
	f2 = thys->fraction[thys->back[aa]];

	if(f>f2)
	    thys->back[aa] = i;
    }

    return;
}




/* @func ajCodTriplet *********************************************************
**
** Convert triplet index to triple
**
** @param [r] idx [ajint] triplet index
**
** @return [char*] Triplet
** @@
******************************************************************************/

char* ajCodTriplet(ajint idx)
{
    static char ret[4] = "AAA";
    const char *conv = "ACGT";

    char *p;

    p=ret;

    *p++ = conv[idx>>4];
    *p++ = conv[(idx & 0xf)>>2];
    *p   = conv[idx & 0x3];

    return ret;
}




/* @func ajCodWriteOut ********************************************************
**
** Write codon structure to output file
**
** @param [r] thys  [const AjPCod]  codon usage
** @param [u] outf [AjPOutfile] output file
**
** @return [void]
** @category output [AjPCod] Write codon structure to output file
** @@
******************************************************************************/
void ajCodWriteOut(const AjPCod thys, AjPOutfile outf)
{
    ajint i;

    for(i=0;codInFormatDef[i].Name; i++)
    {
	if(ajStrMatchCaseC(ajOutfileGetFormat(outf), codOutFormatDef[i].Name))
	{
	    codOutFormatDef[i].Write(thys, ajOutfileGetFile(outf));
	    return;
	}
    }

    return;
}




/* @func ajCodWrite ********************************************************
**
** Write codon structure to output file
**
** @param [u] thys  [AjPCod]  codon usage
** @param [u] outf [AjPFile] output file
**
** @return [void]
** @category output [AjPCod] Write codon structure to output file
** @@
******************************************************************************/

void ajCodWrite(AjPCod thys, AjPFile outf)
{
    codFix(thys);
    codWriteEmboss(thys, outf);
}




/* @funcstatic codWriteEmboss *************************************************
**
** Write codon structure to output file in EMBOSS format
**
** @param [r] thys  [const AjPCod]  codon usage
** @param [u] outf [AjPFile] output file
**
** @return [void]
** @category output [AjPCod] Write codon structure to output file
** @@
******************************************************************************/

static void codWriteEmboss(const AjPCod thys, AjPFile outf)
{
    ajint i;
    ajint j;
    const char* codon;
    ajint gc[3] = {0,0,0};
    float totgc = 0.0;

    if(ajStrGetLen(thys->Species))
	ajFmtPrintF(outf, "#Species: %S\n", thys->Species);

    if(ajStrGetLen(thys->Division))
	ajFmtPrintF(outf, "#Division: %S\n", thys->Division);

    if(ajStrGetLen(thys->Release))
	ajFmtPrintF(outf, "#Release: %S\n", thys->Release);

    if(thys->GeneticCode)
	ajFmtPrintF(outf, "#GeneticCode: %d\n", thys->GeneticCode);

    if(thys->CdsCount)
	ajFmtPrintF(outf, "#CdsCount: %d\n", thys->CdsCount);

    for(j=0;j<AJCODSTART;++j)
    {
	codon = ajCodTriplet(j);

	for(i=0;i<3;i++)
	    if(codon[i] == 'C' || codon[i] == 'G')
		gc[i] += thys->num[j];
    }

    totgc = (float) (gc[0] + gc[1] + gc[2]);
    ajFmtPrintF(outf, "\n#Coding GC %5.2f%%\n",
		(100.0 * totgc/(float)thys->CodonCount/3.0));
    ajFmtPrintF(outf, "#1st letter GC %5.2f%%\n",
		(100.0 * (float)gc[0]/(float)thys->CodonCount));
    ajFmtPrintF(outf, "#2nd letter GC %5.2f%%\n",
		(100.0 * (float)gc[1]/(float)thys->CodonCount));
    ajFmtPrintF(outf, "#3rd letter GC %5.2f%%\n\n",
		(100.0 * (float)gc[2]/(float)thys->CodonCount));

    ajFmtPrintF(outf, "#Codon AA Fraction Frequency Number\n");

    for(i=0;i<AJCODAMINOS-2;++i)
	for(j=0;j<AJCODSTART;++j)
	    if(thys->aa[j]==i)
		ajFmtPrintF(outf,"%s    %c  %8.3f %9.3f %6d\n",
			    ajCodTriplet(j),
			    i+'A',thys->fraction[j],thys->tcount[j],
			    thys->num[j]);

    for(j=0;j<AJCODSTART;++j)
	if(thys->aa[j]==27)
	    ajFmtPrintF(outf,"%s    *  %8.3f %9.3f %6d\n",
			ajCodTriplet(j),
			thys->fraction[j],thys->tcount[j],thys->num[j]);

    return;
}




/* @funcstatic codWriteSpsum *************************************************
**
** Write codon structure to output file inSpsum  format
**
** @param [r] thys  [const AjPCod]  codon usage
** @param [u] outf [AjPFile] output file
**
** @return [void]
** @category output [AjPCod] Write codon structure to output file
** @@
******************************************************************************/

static void codWriteSpsum(const AjPCod thys, AjPFile outf)
{
    AjPStr species = NULL;
    ajint i;
    ajint j;


    ajStrAssignS(&species, thys->Species);
    ajStrAssignEmptyC(&species, "unknown species");

    ajFmtPrintF(outf, "%S: %d\n", species, thys->CdsCount);

    for(i=0;i<64;i++)
    {
	j = ajCodIndexC(spsumcodons[i]);

	if(i)
	    ajFmtPrintF(outf, " %d", thys->num[j]);
	else
	    ajFmtPrintF(outf, "%d", thys->num[j]);
    }

    ajFmtPrintF(outf, "\n");

    ajStrDel(&species);

    return;
}




/* @funcstatic codWriteCutg *************************************************
**
** Write codon structure to output file in Cutg format
**
** @param [r] thys  [const AjPCod]  codon usage
** @param [u] outf [AjPFile] output file
**
** @return [void]
** @category output [AjPCod] Write codon structure to output file
** @@
******************************************************************************/

static void codWriteCutg(const AjPCod thys, AjPFile outf)
{
    const char* c0;
    const char* c1;
    const char* c2;
    const char* bases = "UCAG";
    char codon[4]="GGG";
    ajint i;
    float totgc;
    ajint gc[3] = {0,0,0};
    AjPStr species = NULL;
    AjPStr division = NULL;
    AjPStr minusline = NULL;

    ajStrAssignS(&species, thys->Species);
    ajStrAssignEmptyC(&species, "unknown species");

    ajStrAssignS(&division, thys->Division);
    ajStrAssignEmptyC(&division, "unknown.division");

    ajFmtPrintF(outf, "%S [%S]: %d CDS's (%d codons)\n",
		species, division, thys->CdsCount, thys->CodonCount);

    ajStrAppendCountK(&minusline, '-', 80);
    ajFmtPrintF(outf, "%S\n", minusline);
    ajFmtPrintF(outf,
		"fields: [triplet] [frequency: per thousand] ([number])\n");
    ajFmtPrintF(outf, "%S\n\n", minusline);

    c0 = bases;

    while(*c0)
    {
	codon[0]=*c0;
	c2 = bases;

	while(*c2)
	{
	    codon[2]=*c2;
	    c1 = bases;

	    while(*c1)
	    {
		codon[1]=*c1;
		i = ajCodIndexC(codon);

		if(*c0 == 'C' || *c0 == 'G')
		    gc[0] += thys->num[i];

		if(*c1 == 'C' || *c1 == 'G')
		    gc[1] += thys->num[i];

		if(*c2 == 'C' || *c2 == 'G')
		    gc[2] += thys->num[i];
		
		ajFmtPrintF(outf, "%s %4.1f(%6d)",
			    codon, thys->tcount[i], thys->num[i]);
		c1++;

		if(*c1)
		    ajFmtPrintF(outf, "  ");
	    }

	    ajFmtPrintF(outf, "\n");
	    c2++;
	}

	c0++;

	if(*c0)
	    ajFmtPrintF(outf, "\n");
    }

    ajFmtPrintF(outf, "\n\n");
    ajFmtPrintF(outf, "%S\n", minusline);
    totgc = (float) (gc[0] + gc[1] + gc[2]);
    ajFmtPrintF(outf, "Coding GC %5.2f%% 1st letter GC %5.2f%% "
		"2nd letter GC %5.2f%% 3rd letter GC %5.2f%%\n",
		(100.0 * totgc/(float)thys->CodonCount/3.0),
		(100.0 * (float)gc[0]/(float)thys->CodonCount),
		(100.0 * (float)gc[1]/(float)thys->CodonCount),
		(100.0 * (float)gc[2]/(float)thys->CodonCount));

    ajStrDel(&species);
    ajStrDel(&division);
    ajStrDel(&minusline);

    return;
}




/* @funcstatic codWriteCutgaa *************************************************
**
** Write codon structure to output file in Cutg format with amino acids
**
** @param [r] thys  [const AjPCod]  codon usage
** @param [u] outf [AjPFile] output file
**
** @return [void]
** @category output [AjPCod] Write codon structure to output file
** @@
******************************************************************************/

static void codWriteCutgaa(const AjPCod thys, AjPFile outf)
{
    const char* c0;
    const char* c1;
    const char* c2;
    const char* bases = "UCAG";
    char codon[4]="GGG";
    ajint i;
    float totgc;
    ajint gc[3] = {0,0,0};
    AjPStr species = NULL;
    AjPStr division = NULL;
    AjPStr minusline = NULL;
    char aa;

    ajStrAssignS(&species, thys->Species);
    ajStrAssignEmptyC(&species, "unknown species");

    ajStrAssignS(&division, thys->Division);
    ajStrAssignEmptyC(&division, "unknown.division");

    ajFmtPrintF(outf, "%S [%S]: %d CDS's (%d codons)\n",
		species, division, thys->CdsCount, thys->CodonCount);

    ajStrAppendCountK(&minusline, '-', 80);
    ajFmtPrintF(outf, "%S\n", minusline);
    ajFmtPrintF(outf,
		"fields: [triplet] [frequency: per thousand] ([number])\n");
    ajFmtPrintF(outf, "%S\n\n", minusline);

    c0 = bases;

    while(*c0)
    {
	codon[0]=*c0;
	c2 = bases;

	while(*c2)
	{
	    codon[2]=*c2;
	    c1 = bases;

	    while(*c1)
	    {
		codon[1]=*c1;
		i = ajCodIndexC(codon);

		if(*c0 == 'C' || *c0 == 'G')
		    gc[0] += thys->num[i];
		if(*c1 == 'C' || *c1 == 'G')
		    gc[1] += thys->num[i];
		if(*c2 == 'C' || *c2 == 'G')
		    gc[2] += thys->num[i];
		if(thys->aa[i] > 25)
		    aa = '*';
		else
		    aa = 'A' + thys->aa[i];
		
		ajFmtPrintF(outf, "%s %c %4.2f %4.1f (%6d)",
			    codon, aa,
			    thys->fraction[i], thys->tcount[i], thys->num[i]);
		c1++;

		if(*c1)
		    ajFmtPrintF(outf, "  ");
	    }

	    ajFmtPrintF(outf, "\n");
	    c2++;
	}

	c0++;

	if(*c0)
	    ajFmtPrintF(outf, "\n");
    }

    ajFmtPrintF(outf, "\n\n");
    ajFmtPrintF(outf, "%S\n", minusline);
    totgc = (float) (gc[0] + gc[1] + gc[2]);
    ajFmtPrintF(outf, "Coding GC %5.2f%% 1st letter GC %5.2f%% "
		"2nd letter GC %5.2f%% 3rd letter GC %5.2f%%\n",
		(100.0 * totgc/(float)thys->CodonCount/3.0),
		(100.0 * (float)gc[0]/(float)thys->CodonCount),
		(100.0 * (float)gc[1]/(float)thys->CodonCount),
		(100.0 * (float)gc[2]/(float)thys->CodonCount));

    ajFmtPrintF(outf, "Genetic code %d: %S\n",
		thys->GeneticCode, ajTrnName(thys->GeneticCode));
    ajStrDel(&species);
    ajStrDel(&division);
    ajStrDel(&minusline);

    return;
}




/* @funcstatic codWriteCherry *************************************************
**
** Write codon structure to output file in Mike Cherry's codon usage database
** format.
**
** Very similar to GCG format, has a structured header comment from
** which we can extract the species, CDS count, and release.
**
** @param [r] thys  [const AjPCod]  codon usage
** @param [u] outf [AjPFile] output file
**
** @return [void]
** @category output [AjPCod] Write codon structure to output file
** @@
******************************************************************************/

static void codWriteCherry(const AjPCod thys, AjPFile outf)
{
    const char* c0;
    const char* c1;
    const char* c2;
    const char* bases = "GATC";
    char codon[4]="GGG";
    ajint i;
    AjPStr ThreeAa = NULL;
    AjPStr release = NULL;
    AjPStr species = NULL;
    AjPStr division = NULL;

    ajStrAssignS(&release, thys->Release);
    ajStrAssignEmptyC(&release, "unknowndb0.0");

    ajStrAssignS(&species, thys->Species);
    ajStrAssignEmptyC(&species, "unknown species");

    ajStrAssignS(&division, thys->Division);
    ajStrAssignEmptyC(&division, "unknown.division");

    ajFmtPrintF(outf, "\n%S\n\n", species);
    ajFmtPrintF(outf, "%d genes found in %S\n\n", thys->CdsCount, release);
    ajFmtPrintF(outf, "AmAcid  Codon     Number    /1000     Fraction   ..\n");

    c0 = bases;
    while(*c0)
    {
	codon[0]=*c0;
	c1 = bases;

	while(*c1)
	{
	    codon[1]=*c1;
	    ajFmtPrintF(outf, " \n");
	    c2 = bases;

	    while(*c2)
	    {
		codon[2]=*c2;
		i = ajCodIndexC(codon);

		if(!ajResidueToTriplet(ajBasecodeFromInt(thys->aa[i]),
                                       &ThreeAa))
		    ajStrAssignC(&ThreeAa, "End");

		ajStrFmtLower(&ThreeAa);
		ajStrFmtTitle(&ThreeAa);
		ajFmtPrintF(outf, "%S     %s  %10.2f    %6.2f      %4.2f\n",
			    ThreeAa, codon, (float) thys->num[i],
			    thys->tcount[i], thys->fraction[i]);
		c2++;
	    }

	    c1++;
	}

	c0++;
    }

    ajStrDel(&species);
    ajStrDel(&release);
    ajStrDel(&division);
    ajStrDel(&ThreeAa);

    return;
}




/* @funcstatic codWriteTransterm **********************************************
**
** Write codon structure to output file in Transterm database format
**
** Very similar to GCG format. There is a fixed first line referring to the
** transterm program where we put today's date and time, and the line format
** has slightly different spacing to our "GCG" format.
**
** @param [r] thys  [const AjPCod]  codon usage
** @param [u] outf [AjPFile] output file
**
** @return [void]
** @category output [AjPCod] Write codon structure to output file
** @@
******************************************************************************/

static void codWriteTransterm(const AjPCod thys, AjPFile outf)
{
    const char* c0;
    const char* c1;
    const char* c2;
    const char* bases = "GATC";
    char codon[4]="GGG";
    ajint i;
    AjPStr ThreeAa = NULL;
    AjPStr datestr = NULL;
    AjPStr timestr = NULL;

    ajFmtPrintS(&datestr, "%D", ajTimeRefTodayFmt("day"));
    ajFmtPrintS(&timestr, "%D", ajTimeRefTodayFmt("time"));

    ajFmtPrintF(outf, "FISH_TERM version 4.45 run at %S at  %S\n\n",
		datestr, timestr);

    ajStrDel(&datestr);
    ajStrDel(&timestr);

    ajFmtPrintF(outf, "AmAcid  Codon     Number    /1000     Fraction   ..\n");

    c0 = bases;

    while(*c0)
    {
	codon[0]=*c0;
	c1 = bases;

	while(*c1)
	{
	    codon[1]=*c1;
	    ajFmtPrintF(outf, "\n");
	    c2 = bases;

	    while(*c2)
	    {
		codon[2]=*c2;
		i = ajCodIndexC(codon);

		if(!ajResidueToTriplet(ajBasecodeFromInt(thys->aa[i]),
                                       &ThreeAa))
		    ajStrAssignC(&ThreeAa, "End");

		ajStrFmtLower(&ThreeAa);
		ajStrFmtTitle(&ThreeAa);
		ajFmtPrintF(outf, "%S     %s  %10.2f    %6.2f      %4.2f\n",
			    ThreeAa, codon, (float) thys->num[i],
			    thys->tcount[i], thys->fraction[i]);
		c2++;
	    }

	    c1++;
	}

	c0++;
    }

    ajStrDel(&ThreeAa);

    return;
}




/* @funcstatic codWriteGcg *************************************************
**
** Write codon structure to output file in Gcg format
**
** @param [r] thys  [const AjPCod]  codon usage
** @param [u] outf [AjPFile] output file
**
** @return [void]
** @category output [AjPCod] Write codon structure to output file
** @@
******************************************************************************/

static void codWriteGcg(const AjPCod thys, AjPFile outf)
{
    const char* c0;
    const char* c1;
    const char* c2;
    const char* bases = "GATC";
    char codon[4]="GGG";
    ajint i;
    AjPStr ThreeAa = NULL;

    ajFmtPrintF(outf, "!!CODON_FREQUENCY 1.0\n\n");
    ajFmtPrintF(outf, "AmAcid  Codon     Number    /1000     Fraction   ..\n");

    c0 = bases;

    while(*c0)
    {
	codon[0]=*c0;
	c1 = bases;

	while(*c1)
	{
	    codon[1]=*c1;
	    ajFmtPrintF(outf, "\n");
	    c2 = bases;

	    while(*c2)
	    {
		codon[2]=*c2;
		i = ajCodIndexC(codon);

		if(!ajResidueToTriplet(ajBasecodeFromInt(thys->aa[i]),
                                       &ThreeAa))
		    ajStrAssignC(&ThreeAa, "End");

		ajStrFmtLower(&ThreeAa);
		ajStrFmtTitle(&ThreeAa);
		ajFmtPrintF(outf, "%S     %s %10.2f   %8.2f      %4.2f\n",
			    ThreeAa, codon, (double) thys->num[i],
			    thys->tcount[i], thys->fraction[i]);
		c2++;
	    }

	    c1++;
	}

	c0++;
    }

    ajStrDel(&ThreeAa);
    return;
}




/* @funcstatic codWriteCodehop ************************************************
**
** Write codon structure to output file in Codehop program format
**
** @param [r] thys  [const AjPCod]  codon usage
** @param [u] outf [AjPFile] output file
**
** @return [void]
** @category output [AjPCod] Write codon structure to output file
** @@
******************************************************************************/

static void codWriteCodehop(const AjPCod thys, AjPFile outf)
{
    const char* c0;
    const char* c1;
    const char* c2;
    const char* bases = "ACGU";
    char codon[4]="AAA";
    ajint i;
    ajint j = 0;
    AjPStr release = NULL;
    AjPStr species = NULL;
    AjPStr division = NULL;

    ajStrAssignS(&release, thys->Release);
    ajStrAssignEmptyC(&release, "unknowndb0.0");

    ajStrAssignS(&species, thys->Species);
    ajStrAssignEmptyC(&species, "unknown species");

    ajStrAssignS(&division, thys->Division);
    ajStrAssignEmptyC(&division, "unknown.division");

    c0 = bases;

    while(*c0)
    {
	codon[0]=*c0;
	c1 = bases;

	while(*c1)
	{
	    codon[1]=*c1;
	    c2 = bases;

	    while(*c2)
	    {
		codon[2]=*c2;
		i = ajCodIndexC(codon);
		ajFmtPrintF(outf, "%8.6f\t-- [%2d] %s\n",
			    0.001 * thys->tcount[i], j++, codon);
		c2++;
	    }
	    c1++;
	}
	c0++;
    }

    ajFmtPrintF(outf, "---------------------------------------------\n");
    ajFmtPrintF(outf, "%S\n", thys->Name);
    ajFmtPrintF(outf, "Codon usage for %S: %d\n",
		species, thys->CdsCount);
    ajFmtPrintF(outf, " from %S/%S: %d codons\n",
		release, division, thys->CodonCount);

    ajStrDel(&species);
    ajStrDel(&release);
    ajStrDel(&division);

    return;
}




/* @funcstatic codWriteStaden *************************************************
**
** Write codon structure to output file in Staden format with numbers
** (as generated by nip4) instead of percentages as used by the original
** nip program.
**
** @param [r] thys  [const AjPCod]  codon usage
** @param [u] outf [AjPFile] output file
**
** @return [void]
** @category output [AjPCod] Write codon structure to output file
** @@
******************************************************************************/

static void codWriteStaden(const AjPCod thys, AjPFile outf)
{
    const char* c0;
    const char* c1;
    const char* c2;
    const char* bases = "TCAG";
    char codon[4]="TTT";
    const char* aa = "ABCDEFGHIJKLMNOPQRSTUVWXYZ.*";
    ajint i;

    c0 = bases;

    while(*c0)
    {
	codon[0]=*c0;
	ajFmtPrintF(outf,
		    "      ===========================================\n");
	c2 = bases;

	while(*c2)
	{
	    codon[2]=*c2;
	    ajFmtPrintF(outf, "     ");
	    c1 = bases;

	    while(*c1)
	    {
		codon[1]=*c1;
		i = ajCodIndexC(codon);
		ajFmtPrintF(outf, " %c %s %4d",
			    aa[thys->aa[i]], codon, thys->num[i]);
		c1++;
	    }

	    ajFmtPrintF(outf, "\n");
	    c2++;
	}

	c0++;
    }

    ajFmtPrintF(outf,
		"      ===========================================\n");

    return;
}




/* @func ajCodComp ************************************************************
**
** Calculate sequence composition
**
** @param [w] NA [ajint *] number of A's
** @param [w] NC [ajint *] number of C's
** @param [w] NG [ajint *] number of G's
** @param [w] NT [ajint *] number of T'
** @param [r] str [const char *] sequence
**
** @return [void]
** @@
******************************************************************************/

void ajCodComp(ajint *NA, ajint *NC, ajint *NG, ajint *NT, const char *str)
{
    const char *p;
    ajint c;

    p = str;

    while((c = *p))
    {
	if(c=='A')
            ++(*NA);
	else if(c=='C')
            ++(*NC);
	else if(c=='G')
            ++(*NG);
	else if(c=='T')
            ++(*NT);
	++p;
    }

    return;
}




/* @funcstatic codRandom ****************************************************
**
** Calculate expected frequency of a codon
**
** @param [r] NA [ajint] number of A's
** @param [r] NC [ajint] number of C's
** @param [r] NG [ajint] number of G's
** @param [r] NT [ajint] number of T'
** @param [r] len [ajint] sequence length
** @param [r] p [const char *] triplet
**
** @return [double] triplet frequency
** @@
******************************************************************************/

static double codRandom(ajint NA, ajint NC, ajint NG, ajint NT,
			  ajint len, const char *p)
{
    ajint i;
    double prod;
    double tot;
    ajint c;


    prod = 1;
    tot  = 1.0;

    for(i=0;i<3;++i)
    {
	c = p[i];

	if(c=='A')
	    prod = (double)NA;

	if(c=='C')
	    prod = (double)NC;

	if(c=='G')
	    prod = (double)NG;

	if(c=='T')
	    prod = (double)NT;

	tot *= prod/(double)len;
    }

    return tot;
}




/* @funcstatic codGetWstat *****************************************************
**
** Calculate codon adaptive index W values
** NAR 15:1281-1295
**
** @param [r] thys [const AjPCod] codon usage
**
** @return [double*] w value array
** @@
******************************************************************************/

static double* codGetWstat(const AjPCod thys)
{
    ajint i;
    ajint j;
    double *wk;
    ajint thisaa;
    double aamax;

    AJCNEW0(wk,AJCODSTART);


    for(i=0;i<AJCODSTART;++i)
    {
	thisaa = thys->aa[i];
	aamax = (double)INT_MIN;

	for(j=0;j<AJCODSTART;++j)
	    if(thys->aa[j]==thisaa)
		aamax = aamax > thys->tcount[j] ? aamax : thys->tcount[j];
	if(!E_FPZERO(aamax,U_DEPS))
	    wk[i] = thys->tcount[i] / aamax;
    }

    return wk;
}




/* @func ajCodCalcCaiCod *******************************************************
**
** Calculate codon adaptive index using overall codon usage data only.
**
** See equation 8 in NAR 15:1281-1295
**
** @param [r] thys [const AjPCod] codon usage
**
** @return [double] CAI
** @@
******************************************************************************/

double ajCodCalcCaiCod(const AjPCod thys)
{
    double cai;
    double max;
    double sum;
    double res;
    double xij;
    double total;
    ajint  i;
    ajint  k;

    total = (double)0.;

    for(i=0;i<AJCODAMINOS-2;++i)
    {
	max = (double)0.;
	for(k=0;k<AJCODSTART;++k)
	{
	    if(thys->aa[k]==27)
		continue;

	    if(thys->aa[k]==i)
		max = (max> thys->fraction[k]) ? max : thys->fraction[k];
	}

	sum = (double)0.;

	for(k=0;k<AJCODSTART && !E_FPZERO(max,U_DEPS);++k)
	{
	    if(thys->aa[k]==27)
		continue;

	    if(thys->aa[k]==i)
	    {
		xij = thys->fraction[k];

		if(!E_FPZERO(xij,U_DEPS))
		{
		    res = thys->tcount[k] * log(xij/max);
		    sum += res;
		}
	    }
	}

	total += sum;
    }

    total /= (double)1000.;
    cai = exp(total);

    return cai;
}




/* @func ajCodCalcCaiSeq *******************************************************
**
** Calculate codon adaptive index for a coding sequence
**
** See equation 7 in NAR 15:1281-1295
**
** @param [r] thys [const AjPCod] codon usage
** @param [r] str [const AjPStr] sequence
**
** @return [double] CAI
** @@
******************************************************************************/

double ajCodCalcCaiSeq(const AjPCod thys, const AjPStr str)
{
    double *wk;
    ajint  i;
    ajint  len;
    const char   *p;
    ajint  limit;
    ajint  idx;
    double total;

    wk = codGetWstat(thys);

    len = ajStrGetLen(str);
    p   = ajStrGetPtr(str);

    limit = len / 3;
    total = (double)0.;

    for(i=0;i<limit; ++i,p+=3)
    {
	idx = ajCodIndexC(p);

	if(!E_FPZERO(wk[idx],U_DEPS))
	    total += log(wk[idx]);
    }

    total /= (double)limit;

    AJFREE(wk);

    return exp(total);
}




/* @obsolete ajCodCalcCai
** @rename ajCodCalcCaiSeq
*/

__deprecated double ajCodCalcCai(const AjPCod thys, const AjPStr str)
{
    return ajCodCalcCaiSeq(thys, str);
}




/* @func ajCodCalcGribskov ****************************************************
**
** Calculate Gribskov statistic (count per thousand) in AjPCod internals
**
** @param [u] thys [AjPCod] codon usage for sequence
** @param [r] s [const AjPStr] sequence
**
** @return [void]
** @@
******************************************************************************/

void ajCodCalcGribskov(AjPCod thys, const AjPStr s)
{
    ajint i;
    ajint j;

    ajint NA;
    ajint NC;
    ajint NG;
    ajint NT;

    const char *p;
    ajint len;
    ajint aa;
    double fam[64];
    double frct[64];

    double x;
    double z;

    len = ajStrGetLen(s);

    for(i=0;i<AJCODSTART;++i)
	frct[i] = thys->fraction[i];

    NA = NC = NG = NT = 0;
    ajCodComp(&NA,&NC,&NG,&NT,ajStrGetPtr(s));

    /* Get expected frequencies */
    for(i=0;i<AJCODSTART;++i)
    {
	p = ajCodTriplet(i);
	thys->tcount[i] = codRandom(NA,NC,NG,NT,len,p);
    }


    /* Calculate expected family */
    for(i=0;i<AJCODSTART;++i)
    {
	x=0.0;
	aa = thys->aa[i];

	for(j=0;j<AJCODSTART;++j)
	    if(thys->aa[j]==aa) x += thys->tcount[j];

	fam[i] = x;
    }


    /* Calculate expected ratio etc */

    for(i=0;i<AJCODSTART;++i)
    {
	z = thys->tcount[i] / fam[i];

	/* Work out ln(real/expected) */
	thys->tcount[i]= log(frct[i] / z);
    }

    return;
}




/* @func ajCodCalcNc **********************************************************
**
** Calculate effective number of codons
** Wright, F. (1990) Gene 87:23-29
**
** @param [r] thys [const AjPCod] codon usage
**
** @return [double] Nc
** @@
******************************************************************************/

double ajCodCalcNc(const AjPCod thys)
{
    ajint *df = NULL;
    ajint *n  = NULL;
    ajint i;
    ajint j;
    ajint v;

    ajint max;
    ajint ndx;

    ajint    *nt = NULL;
    double *Fbar = NULL;
    double *F    = NULL;
    double sum;
    double num = 0.0;

    AJCNEW0(df, AJCODAMINOS);
    AJCNEW0(n, AJCODAMINOS);

    for(i=0;i<AJCODSTART;++i)
    {
	v = thys->aa[i];

	if(v==27)
	    continue;

	++df[v];
	n[thys->aa[i]] += thys->num[i];
    }


    for(i=0,max=INT_MIN;i<AJCODAMINOS;++i)
	max = (max > df[i]) ? max : df[i];

    AJCNEW0(Fbar, max);
    AJCNEW0(nt, max);
    AJCNEW0(F, AJCODAMINOS);

    for(i=0;i<AJCODAMINOS-2;++i)
	if(df[i])
	    ++nt[df[i]-1];

    for(i=0;i<AJCODAMINOS-2;++i)
      for(j=0;j<AJCODSTART;++j)
	{
	    if(thys->aa[j]==27)
		continue;

	    if(thys->aa[j]==i)
		F[i] += pow(thys->fraction[j],2.0);
	}


    for(i=0;i<AJCODAMINOS-2;++i)
    {
	if(n[i]-1 <1  || (num=((double)n[i]*F[i]-1.0))<0.05)
	{
	    F[i] = 0.0;

	    if(df[i])
		--nt[df[i]-1];

	    continue;
	}

	F[i]= num / ((double)n[i]-1.0);
    }


    for(i=0;i<AJCODAMINOS-2;++i)
    {
	ndx=df[i];

	if(!ndx)
	    continue;

	--ndx;
	Fbar[ndx] += F[i];
    }

    for(i=0;i<max;++i)
	if(nt[i])
	    Fbar[i] /= (double)nt[i];

    if(E_FPZERO(Fbar[2],U_DEPS))				/* Ile fix */
	Fbar[2] = (Fbar[1]+Fbar[3]) / 2.0;


    for(i=1,sum=2.0;i<max;++i)
    {
	if(E_FPZERO(Fbar[i],U_DEPS))
	    continue;

	if(i==1)
	    sum += 9.0/Fbar[i];
	else if(i==2)
	    sum += 1.0/Fbar[i];
	else if(i==3)
	    sum += 5.0/Fbar[i];
	else if(i==5)
	    sum += 3.0/Fbar[i];
    }


    AJFREE(F);
    AJFREE(nt);
    AJFREE(Fbar);
    AJFREE(n);
    AJFREE(df);

    if(sum>61.0)
	return 61.0;

    return sum;
}




/* @func ajCodCalcUsage ********************************************************
**
** Calculate fractional and thousand elements of a codon object
** Used for creating a codon usage table
**
** If no counts are stored, warns of need to first run ajCodSetTriplets
**
** @param [u] thys [AjPCod] Codon object
** @param [r] c [ajint] triplet count
**
** @return [void]
** @@
******************************************************************************/

void ajCodCalcUsage(AjPCod thys, ajint c)
{
    ajint i;
    ajint *aasum;
    ajuint totcnt = 0;

    for(i=0;i<AJCODSIZE;i++)
        totcnt += thys->num[i];
    if(!totcnt)
        ajWarn("empty codon usage table, first call ajCodSetTripletsS");

    /* Calculate thousands */
    for(i=0;i<AJCODSTART;++i)
	if(!c)
	    thys->tcount[i] = 0.0;
	else
	    thys->tcount[i] = ((double)thys->num[i] / (double)c) * 1000.0;

    /* Get number of codons per amino acid */
    AJCNEW0(aasum, AJCODAMINOS);
    for(i=0;i<AJCODSTART;++i)
	if(thys->aa[i]==27)
	    aasum[27] += thys->num[i];
	else
	    aasum[thys->aa[i]] += thys->num[i];


    /* Calculate fraction */
    for(i=0;i<AJCODSTART;++i)
	if(thys->aa[i]==27)
	{
	    if(!aasum[27])
		thys->fraction[i] = 0.0;
	    else
		thys->fraction[i] = (double)thys->num[i] /
		    (double)aasum[27];
	}
	else
	{
	    if(!aasum[thys->aa[i]])
		thys->fraction[i] = 0.0;
	    else
		thys->fraction[i] = (double)thys->num[i] /
		    (double)aasum[thys->aa[i]];
	}

    AJFREE(aasum);

    return;
}




/* @obsolete ajCodCalculateUsage
** @rename ajCodCalcUsage
*/

__deprecated void ajCodCalculateUsage(AjPCod thys, ajint c)
{
    ajCodCalcUsage(thys, c);
    return;
}




/* @func ajCodGetName *********************************************************
**
** Returns the name of a codon table
**
** @param [r] thys [const AjPCod] Codon usage object
** @return [const AjPStr] Original filename
******************************************************************************/

const AjPStr ajCodGetName(const AjPCod thys)
{
    return thys->Name;
}




/* @func ajCodGetNameC ********************************************************
**
** Returns the name of a codon table
**
** @param [r] thys [const AjPCod] Codon usage object
** @return [const char*] Original filename
******************************************************************************/

const char* ajCodGetNameC(const AjPCod thys)
{
    return ajStrGetPtr(thys->Name);
}




/* @func ajCodGetDesc *********************************************************
**
** Returns the description of a codon table
**
** @param [r] thys [const AjPCod] Codon usage object
** @return [const AjPStr] Original filename
******************************************************************************/

const AjPStr ajCodGetDesc(const AjPCod thys)
{
    return thys->Desc;
}




/* @func ajCodGetDescC ********************************************************
**
** Returns the description of a codon table
**
** @param [r] thys [const AjPCod] Codon usage object
** @return [const char*] Original filename
******************************************************************************/

const char* ajCodGetDescC(const AjPCod thys)
{
    return ajStrGetPtr(thys->Desc);
}




/* @func ajCodGetSpecies ******************************************************
**
** Returns the species of a codon table
**
** @param [r] thys [const AjPCod] Codon usage object
** @return [const AjPStr] Species
******************************************************************************/

const AjPStr ajCodGetSpecies(const AjPCod thys)
{
    return thys->Species;
}




/* @func ajCodGetSpeciesC *****************************************************
**
** Returns the species of a codon table
**
** @param [r] thys [const AjPCod] Codon usage object
** @return [const char*] Species
******************************************************************************/

const char* ajCodGetSpeciesC(const AjPCod thys)
{
    return ajStrGetPtr(thys->Species);
}




/* @func ajCodGetDivision *****************************************************
**
** Returns the division of a codon table
**
** @param [r] thys [const AjPCod] Codon usage object
** @return [const AjPStr] Division
******************************************************************************/

const AjPStr ajCodGetDivision(const AjPCod thys)
{
    return thys->Division;
}




/* @func ajCodGetDivisionC ****************************************************
**
** Returns the division of a codon table
**
** @param [r] thys [const AjPCod] Codon usage object
** @return [const char*] Division
******************************************************************************/

const char* ajCodGetDivisionC(const AjPCod thys)
{
    return ajStrGetPtr(thys->Division);
}




/* @func ajCodGetRelease ******************************************************
**
** Returns the release of a codon table
**
** @param [r] thys [const AjPCod] Codon usage object
** @return [const AjPStr] Release
******************************************************************************/

const AjPStr ajCodGetRelease(const AjPCod thys)
{
    return thys->Release;
}




/* @func ajCodGetReleaseC *****************************************************
**
** Returns the release of a codon table
**
** @param [r] thys [const AjPCod] Codon usage object
** @return [const char*] Release
******************************************************************************/

const char* ajCodGetReleaseC(const AjPCod thys)
{
    return ajStrGetPtr(thys->Release);
}




/* @func ajCodGetNumcodon *****************************************************
**
** Returns the number of codons in a codon table
**
** @param [r] thys [const AjPCod] Codon usage object
** @return [ajint] Number of codons
******************************************************************************/

ajint ajCodGetNumcodon(const AjPCod thys)
{
    return thys->CodonCount;
}




/* @func ajCodGetNumcds *****************************************************
**
** Returns the numbers of CDSs in a codon table
**
** @param [r] thys [const AjPCod] Codon usage object
** @return [ajint] Number of CDSs
******************************************************************************/

ajint ajCodGetNumcds(const AjPCod thys)
{
    return thys->CdsCount;
}




/* @func ajCodGetCode *****************************************************
**
** Returns the genetic code of a codon table
**
** @param [r] thys [const AjPCod] Codon usage object
** @return [ajint] Number of CDSs
******************************************************************************/

ajint ajCodGetCode(const AjPCod thys)
{
    return thys->GeneticCode;
}




/* @func ajCodSetCodenum *****************************************************
**
** Assigns the genetic code in a codon table
**
** @param [u] thys [AjPCod] Codon usage object
** @param [r] geneticcode [ajint] Genetic code
** @return [void]
******************************************************************************/

void ajCodSetCodenum(AjPCod thys, ajint geneticcode)
{
    thys->GeneticCode = geneticcode;
    return;
}




/* @obsolete ajCodAssCode
** @rename ajCodSetCodenum
*/

__deprecated void ajCodAssCode(AjPCod thys, ajint geneticcode)
{
    thys->GeneticCode = geneticcode;
    return;
}




/* @func ajCodSetDescC ********************************************************
**
** Assigns the description of a codon table
**
** @param [u] thys [AjPCod] Codon usage object
** @param [r] desc [const char*] Description
** @return [void]
******************************************************************************/

void ajCodSetDescC(AjPCod thys, const char* desc)
{
    ajStrAssignC(&thys->Desc, desc);
    return;
}




/* @obsolete ajCodAssDescC
** @rename ajCodSetDescC
*/

__deprecated void ajCodAssDescC(AjPCod thys, const char* desc)
{
    ajCodSetDescC(thys, desc);
    return;
}




/* @func ajCodSetDescS *********************************************************
**
** Assigns the description of a codon table
**
** @param [u] thys [AjPCod] Codon usage object
** @param [r] desc [const AjPStr] Description
** @return [void]
******************************************************************************/

void ajCodSetDescS(AjPCod thys, const AjPStr desc)
{
    ajStrAssignS(&thys->Desc,desc );
    return;
}




/* @obsolete ajCodAssDesc
** @rename ajCodSetDescS
*/

__deprecated void ajCodAssDesc(AjPCod thys, const AjPStr desc)
{
    ajCodSetDescS(thys, desc);
    return;
}




/* @func ajCodSetDivisionC ****************************************************
**
** Assigns the division of a codon table
**
** @param [u] thys [AjPCod] Codon usage object
** @param [r] division [const char*] Division
** @return [void]
******************************************************************************/

void ajCodSetDivisionC(AjPCod thys, const char* division)
{
    ajStrAssignC(&thys->Division, division);
    return;
}




/* @obsolete ajCodAssDivisionC
** @rename ajCodSetDivisionC
*/

__deprecated void ajCodAssDivisionC(AjPCod thys, const char* division)
{
    ajCodSetDivisionC(thys, division);
    return;
}




/* @func ajCodSetDivisionS *****************************************************
**
** Assigns the division of a codon table
**
** @param [u] thys [AjPCod] Codon usage object
** @param [r] division [const AjPStr] Division
** @return [void]
******************************************************************************/

void ajCodSetDivisionS(AjPCod thys, const AjPStr division)
{
    ajStrAssignS(&thys->Division, division);
    return;
}




/* @obsolete ajCodAssDivision
** @rename ajCodSetDivisionS
*/

__deprecated void ajCodAssDivision(AjPCod thys, const AjPStr division)
{
    ajCodSetDivisionS(thys, division);
    return;
}




/* @func ajCodSetNameC ********************************************************
**
** Assigns the name of a codon table
**
** @param [u] thys [AjPCod] Codon usage object
** @param [r] name [const char*] Name
** @return [void]
******************************************************************************/

void ajCodSetNameC(AjPCod thys, const char* name)
{
    ajStrAssignC(&thys->Name, name);
    return;
}




/* @obsolete ajCodAssNameC
** @rename ajCodSetNameC
*/

__deprecated void ajCodAssNameC(AjPCod thys, const char* name)
{
    ajCodSetNameC(thys, name);
    return;
}




/* @func ajCodSetNameS *********************************************************
**
** Assigns the name of a codon table
**
** @param [u] thys [AjPCod] Codon usage object
** @param [r] name [const AjPStr] Name
** @return [void]
******************************************************************************/

void ajCodSetNameS(AjPCod thys, const AjPStr name)
{
    ajStrAssignS(&thys->Name, name);
    return;
}




/* @obsolete ajCodAssName
** @rename ajCodSetNameS
*/

__deprecated void ajCodAssName(AjPCod thys, const AjPStr name)
{
    ajCodSetNameS(thys, name);
    return;
}




/* @func ajCodSetNumcds ********************************************************
**
** Assigns the number of CDSs in a codon table
**
** @param [u] thys [AjPCod] Codon usage object
** @param [r] numcds [ajint] Number of codons
** @return [void]
******************************************************************************/

void ajCodSetNumcds(AjPCod thys, ajint numcds)
{
    thys->CdsCount = numcds;
    return;
}




/* @obsolete ajCodAssNumcds
** @rename ajCodSetNumcds
*/

__deprecated void ajCodAssNumcds(AjPCod thys, ajint numcds)
{
    ajCodSetNumcds(thys, numcds);
    return;
}




/* @func ajCodSetNumcodons ****************************************************
**
** Assigns the number of codons in a codon table
**
** @param [u] thys [AjPCod] Codon usage object
** @param [r] numcodon [ajint] Number of codons
** @return [void]
******************************************************************************/

void ajCodSetNumcodons(AjPCod thys, ajint numcodon)
{
    thys->CodonCount = numcodon;
    return;
}




/* @obsolete ajCodAssNumcodon
** @rename ajCodSetNumcodons
*/

__deprecated void ajCodAssNumcodon(AjPCod thys, ajint numcodon)
{
    ajCodSetNumcodons(thys, numcodon);
    return;
}




/* @func ajCodSetReleaseC *****************************************************
**
** Assigns the  of a codon table
**
** @param [u] thys [AjPCod] Codon usage object
** @param [r] release [const char*] Release
** @return [void]
******************************************************************************/

void ajCodSetReleaseC(AjPCod thys, const char* release)
{
    ajStrAssignC(&thys->Release, release);
    return;
}




/* @obsolete ajCodAssReleaseC
** @rename ajCodSetReleaseC
*/

__deprecated void ajCodAssReleaseC(AjPCod thys, const char* release)
{
    ajStrAssignC(&thys->Release, release);
    return;
}




/* @func ajCodSetReleaseS ******************************************************
**
** Assigns the release of a codon table
**
** @param [u] thys [AjPCod] Codon usage object
** @param [r] release [const AjPStr] Release
** @return [void]
******************************************************************************/

void ajCodSetReleaseS(AjPCod thys, const AjPStr release)
{
    ajStrAssignS(&thys->Release, release);
    return;
}




/* @obsolete ajCodAssRelease
** @rename ajCodSetReleaseS
*/

__deprecated void ajCodAssRelease(AjPCod thys, const AjPStr release)
{
    ajCodSetReleaseS(thys, release);
    return;
}




/* @func ajCodSetSpeciesC *****************************************************
**
** Assigns the species of a codon table
**
** @param [u] thys [AjPCod] Codon usage object
** @param [r] species [const char*] Species
** @return [void]
******************************************************************************/

void ajCodSetSpeciesC(AjPCod thys, const char* species)
{
    ajStrAssignC(&thys->Species, species);
    return;
}




/* @obsolete ajCodAssSpeciesC
** @rename ajCodSetSpeciesC
*/

__deprecated void ajCodAssSpeciesC(AjPCod thys, const char* species)
{
    ajCodSetSpeciesC(thys, species);
    return;
}




/* @func ajCodSetSpeciesS *****************************************************
**
** Assigns the species of a codon table
**
** @param [u] thys [AjPCod] Codon usage object
** @param [r] species [const AjPStr] Species
** @return [void]
******************************************************************************/

void ajCodSetSpeciesS(AjPCod thys, const AjPStr species)
{
    ajStrAssignS(&thys->Species, species);
    return;
}




/* @obsolete ajCodAssSpecies
** @rename ajCodSetSpeciesS
*/

__deprecated void ajCodAssSpecies(AjPCod thys, const AjPStr species)
{
    ajCodSetSpeciesS(thys, species);
    return;
}




/* @func ajCodOutFormat *******************************************************
**
** Tests the output format for an outcodon ACD type
**
** @param [r] name [const AjPStr] Format name
** @return [ajint] Internal format index, of -1 if not found
** @@
******************************************************************************/

ajint ajCodOutFormat(const AjPStr name)
{
    ajint i;
    for(i=0;codOutFormatDef[i].Name; i++)
    {
	if(ajStrMatchCaseC(name, codOutFormatDef[i].Name))
	{
	    return i;
	}
    }
    return -1;
}




/* @funcstatic codFix *********************************************************
**
** Fill in missing values in a codon usage object
**
** @param [u] thys [AjPCod] Codon usage object
** @return [void]
******************************************************************************/
static void codFix(AjPCod thys)
{
    ajint i;
    ajint totnum = 0;
    ajint totcds = 0;
    double totfreq = 0.0;
    double totfrac = 0.0;
    ajint totaa = 0;
    ajint aa;
    ajint aacount[28] = {-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
			 -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1};

    for(i=0;i<64;i++)
    {
	totnum += thys->num[i];
	totfreq += thys->tcount[i];
	totfrac += thys->fraction[i];
	aa = thys->aa[i];

	if(aa == 27)
	    totcds += thys->num[i];

	if(aa > 27)
	    aa = 26;

	if(aa < 0)
	    aa = 26;

	if(aacount[aa] == -1)
	{
	    totaa++;
	    aacount[aa] = 0;
	}

	aacount[aa] += thys->num[i];
    }

    if(thys->CdsCount)
    {
	if(totcds > thys->CdsCount)	/* can be less if stops ignored */
	    ajDebug("Codon usage file '%S' has %d stop codons, CDS count %d\n",
		  thys->Name, totcds,  thys->CdsCount);
    }
    else
	thys->CdsCount = totcds;

    if(thys->CodonCount)
    {
	if(totnum != thys->CodonCount)
	    ajDebug("Codon usage file '%S' has %d codons, Codon count %d\n",
		  thys->Name, totnum,  thys->CodonCount);
    }
    else
	thys->CodonCount = totnum;

    if(E_FPZERO(totfreq,U_DEPS))
    {
	for(i=0;i<64;i++)
	{
	    thys->tcount[i] = 1000.0 * (double)thys->num[i] / (double) totnum;
	    totfreq += thys->tcount[i];
	}
    }

    if(fabs(totfreq - 1000.0) > 0.1)
	ajDebug("Codon usage file '%S' has total frequency/1000 of %.2f\n",
	       thys->Name, totfreq);

    if(E_FPZERO(totfrac,U_DEPS))
    {
	for(i=0;i<64;i++)
	{
	    aa = thys->aa[i];

	    if(aa == 27)
		totcds += thys->num[i];

	    if(aa > 27)
		aa = 26;

	    if(aa < 0)
		aa = 26;

	    if(aacount[aa])
		thys->fraction[i] = (double)thys->num[i]/(double)(aacount[aa]);

	    totfrac += thys->fraction[i];
	}
    }

    if(fabs(totfrac - (double)totaa) > 0.1)
	ajDebug("Codon usage file '%S' has total fraction of %.2f for %d "
                "amino acids\n",
	       thys->Name, totfrac, totaa);

    return;
}




/* @func ajCodPrintFormat **************************************************
**
** Reports the internal data structures
**
** @param [u] outf [AjPFile] Output file
** @param [r] full [AjBool] Full report, currently no extra details printed
** @return [void]
** @@
******************************************************************************/

void ajCodPrintFormat(AjPFile outf, AjBool full)
{
    ajint i = 0;

    if(full)
	i=0;

    ajFmtPrintF(outf, "\n");
    ajFmtPrintF(outf, "# codon usage input formats\n");
    ajFmtPrintF(outf, "# Name  Format name (or alias)\n");
    ajFmtPrintF(outf, "# Try   Test for unknown input files\n");
    ajFmtPrintF(outf, "# Desc  Format description\n");
    ajFmtPrintF(outf, "# Name         Try Description\n");
    ajFmtPrintF(outf, "\n");
    ajFmtPrintF(outf, "Format {\n");

    for(i=0; codInFormatDef[i].Name; i++)
    {
	ajFmtPrintF(outf, "  %-12s %3B \"%s\"\n",
		     codInFormatDef[i].Name,
		     codInFormatDef[i].Try,
		     codInFormatDef[i].Desc);
    }

    ajFmtPrintF(outf, "}\n\n");

    ajFmtPrintF(outf, "\n");
    ajFmtPrintF(outf, "# codon usage output formats\n");
    ajFmtPrintF(outf, "# Name  Format name (or alias)\n");
    ajFmtPrintF(outf, "# Desc  Format description\n");
    ajFmtPrintF(outf, "# Name         Description\n");
    ajFmtPrintF(outf, "\n");
    ajFmtPrintF(outf, "OFormat {\n");

    for(i=0; codOutFormatDef[i].Name; i++)
    {
	ajFmtPrintF(outf, "  %-12s \"%s\"\n",
		     codOutFormatDef[i].Name,
		     codOutFormatDef[i].Desc);
    }
    ajFmtPrintF(outf, "}\n\n");

    return;
}




/* @func ajCodGetCodonlist ****************************************************
**
** Writes codon triplets to a string list
**
** @param [r] cod [const AjPCod] Cusp file
** @param [w] list [AjPList] List with character distributions
** @return [void]
** @@
******************************************************************************/

void ajCodGetCodonlist(const AjPCod cod, AjPList list)
{
    ajint i;
    ajint j;
    AjPStr codon = NULL;

    for(i=0;i<AJCODSTART;++i)
	for(j=0;j<cod->tcount[i];j++)
	{
	    codon = ajStrNewC(ajCodTriplet(i));
	    ajListstrPushAppend(list, codon);
	    codon = NULL;
	}

    return;
}




/* @func ajCodExit ************************************************************
**
** Cleans up codon usage processing internal memory
**
** @return [void]
** @@
******************************************************************************/

void ajCodExit(void)
{
    ajStrDel(&codReadLine);
    ajStrDel(&codTmpLine);

    return;
}
