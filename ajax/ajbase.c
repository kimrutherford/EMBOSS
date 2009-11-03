/******************************************************************************
** @source AJAX IUB base nucleic acid functions
**
** @author Copyright (C) 1999 Alan Bleasby
** @version 1.0
** @modified Feb 28 ajb First version
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
#include <string.h>

#define IUBFILE "Ebases.iub"
#define IUBPFILE "Eresidues.iub"

typedef struct AjIUB AjIUB;
struct AjIUB
{
    AjPStr code;
    AjPStr list;
    AjPStr mnemonic;
};



AjIUB aj_base_iubS[256];	/* Base letters and their alternatives */
ajint aj_base_table[256];	/* Base letter numerical codes         */
float aj_base_prob[32][32];     /* Asym base probability matches       */

AjIUB aj_residue_iubS[256];	/* Residues and their alternatives     */
ajint aj_residue_table[256];	/* Residue numerical codes             */
float aj_residue_prob[32][32];  /* Asym base probability matches       */

const char* iubbases = "XACMGRSVTWYHKDBN";

static const char *BaseAaTable[]=
{
    "ALA","ASX","CYS","ASP","GLU","PHE","GLY","HIS",
    "ILE","---","LYS","LEU","MET","ASN","---","PRO",
    "GLN","ARG","SER","THR","---","VAL","TRP","XAA",
    "TYR","GLX" 
};


static AjBool aj_base_I = AJFALSE;
static AjBool aj_residue_I = AJFALSE;

static AjBool baseInit(void);
static AjBool residueInit(void);

/* @filesection ajbase ********************************************************
**
** @nam1rule aj Function belongs to the AJAX library.
**
*/




/* @datasection [none] Base *******************************************
**
** Function is for manipulating nucleotide base codes
** 
** @nam2rule Base 
**
*/


/* @section character conversion
**
** Functions converting binary forms of base codes
**
** @fdata      [none]
**
** @nam3rule Alpha Converts a character value
** @nam4rule AlphaCompare Compares two base codes
** @nam4rule AlphaTo Converts to a specific type
** @nam5rule AlphaToBin Converts to binary code type
**
** @argrule Alpha base [ajint] Alphabetic character as an integer
** @argrule Compare base2 [ajint] Comparison alphabetic character as a character
**
** @valrule Compare [float] Probability bases are the same
** @valrule ToBin [ajint] Binary code in range 0 to 31
**
** @fcategory cast
*/



/* @func ajBaseAlphaCompare ***************************************************
**
** Returns an element of the base match probability array
**
** @param [r] base [ajint] First base offset
** @param [r] base2 [ajint] Second base offset
**
** @return [float] Base probability value
** @@
******************************************************************************/
float  ajBaseAlphaCompare(ajint base, ajint base2)
{
    ajint b1;
    ajint b2;

    if(!aj_base_I)
        baseInit();

    b1 = base;
    b2 = base2;

    if(b1<0)b1=0;
    if(b1>31)b1=31;
    if(b2<0)b2=0;
    if(b2>31)b2=31;

    return aj_base_prob[b1][b2];
}


__deprecated float  ajBaseProb(ajint base1, ajint base2)
{
    return ajBaseAlphaCompare(base1, base2);
}



/* @func ajBaseAlphaToBin *****************************************************
**
** Returns a binary OR'd representation of an IUB base where A=1, C=2,
** G=4 and T=8
** Uses the base table set up by daseInit
**
** @param  [r] base [ajint] character to convert
**
** @return [ajint] Binary OR'd representation
******************************************************************************/

ajint ajBaseAlphaToBin(ajint base)
{
    if(!aj_base_I)
	baseInit();

    return (aj_base_table[toupper(base)]);
}


/* @obsolete ajAZToBin
** @rename ajBaseAlphaToBin
*/

__deprecated ajint ajAZToBin(ajint c)
{
    return ajBaseAlphaToBin(c);
}


/* @section character conversion
**
** Functions converting binary forms of base codes
**
** @fdata      [none]
**
** @nam3rule Alphachar Converts a character value
** @nam4rule AlphacharCompare Compares two base codes
** @nam4rule AlphacharTo Converts to a specific type
** @nam5rule AlphacharToBin Converts to binary code type
**
** @argrule Alphachar c [char] Alphabetic character as a character
** @argrule Compare c2 [char] Comparison alphabetic character as a character
**
** @valrule Compare [float] Probability of a match allowing for ambiguity codes
** @valrule ToBin [char] Binary code in range 0 to 31
**
** @fcategory cast
*/



/* @func ajBaseAlphacharCompare ***********************************************
**
** Check ambiguity codes (IUB) to estimate the distance score.
**
** @param [r] c [char] First base to compare
** @param [r] c2 [char] Second base to compare
** @return [float] estimated match
**
******************************************************************************/

float ajBaseAlphacharCompare(char c, char c2)
{
    const AjPStr b1 = NULL;
    const AjPStr b2 = NULL;
    AjPStr b = NULL;
    AjPRegexp rexp = NULL;
    AjBool pmatch = ajFalse;

    ajuint i;
    float n;
    ajint len1;
    ajint len2;

    if(!aj_base_I)
        baseInit();

    len1 = ajStrGetLen(aj_base_iubS[(int)c].list)-1;
    len2 = ajStrGetLen(aj_base_iubS[(int)c2].list)-1;

    b1 = aj_base_iubS[(int)c].list;

    b2 = aj_base_iubS[(int)c2].list;

    /*
    ** for each base code in 1 cf. base code
    ** for seq 2 to see if there is a match
    */
    for(i = 0;i < (ajuint) len1;i++)
    {
	b = ajStrNew();
	ajStrAssignSubS(&b,b1,i,i);
	rexp = ajRegComp(b);

	if(ajRegExec(rexp,b2))
	    pmatch = ajTrue;
	ajRegFree(&rexp);
	ajStrDel(&b);
    }


    if(pmatch)
	n = ((float)1./len1)*((float)1./len2);
    else
	n = 0.;

    return n;
}




/* @func ajBaseAlphacharToBin *************************************************
**
** Returns a binary OR'd representation of an IUB base where A=1, C=2,
** G=4 and T=8
** Uses the base table set up by baseInit
**
** @param  [r] c [char] character to convert
**
** @return [char] Binary OR'd representation
******************************************************************************/

char ajBaseAlphacharToBin(char c)
{
    if(!aj_base_I)
	baseInit();

    return ajSysCastItoc(aj_base_table[toupper((ajint) c)]);
}


/* @obsolete ajAZToBinC
** @rename ajBaseAlphacharToBin
*/

__deprecated char ajAZToBinC(char c)
{
    return (char)ajBaseAlphacharToBin((int)c);
}



/* @section Complement
**
** Functions complementing nucleotide base codes
**
** @fdata      [none]
**
** @nam3rule Alphachar Converts a character value
** @nam4rule Comp Complement a nucleotide base code
**
** @argrule * c [char]
**
** @valrule * [char]
**
** @fcategory use
*/

/* @func ajBaseAlphacharComp ***************************************************
**
** Complements a nucleotide base.
**
** @param [r] c [char] Base character.
** @return [char] Complementary base.
** @@
******************************************************************************/

char ajBaseAlphacharComp(char c)
{
    static char fwd[]="ACGTURYWSMKBDHVNXacgturywsmkbdhvnx";
    static char rev[]="TGCAAYRWSKMVHDBNXtgcaayrwskmvhdbnx";
    char *cp;
    char *cq;

    cp = strchr(fwd,c);
    if(cp)
    {
	cq = cp - fwd + rev;
	return *cq;
    }

    return c;
}

/* @obsolete ajBaseComp
** @rename ajBaseAlphacharComp
*/
__deprecated char ajBaseComp(char base)
{

    return ajBaseAlphacharComp(base);
}


/* @obsolete ajSeqBaseComp
** @rename ajBaseComp
*/
__deprecated char ajSeqBaseComp(char base)
{
    return ajBaseAlphacharComp(base);
}

/* @section binary conversion
**
** Functions converting binary forms of base codes
**
** @fdata      [none]
**
** @nam3rule Bin Converts a binary code value
** @nam4rule BinTo Converts to a specific type
** @nam5rule BinToAlpha Converts to alphabetic type
**
** @argrule Bin c [ajint] Binary code in range 0 to 31
**
** @valrule ToAlpha [char]
**
** @fcategory cast
*/



/* @func ajBaseBinToAlpha *****************************************************
**
** Converts a binary OR'd representation of an IUB base where A=1, C=2,
** G=4 and T=8 into an ambiguous DNA base code (uses T rather than U).
**
** Uses the base table set up by baseInit
**
** @param  [r] c [ajint] character to convert
**
** @return [char] Ambiguous DNA base code
******************************************************************************/

char ajBaseBinToAlpha(ajint c)
{
    if(c<0)
	return 'N';
    if(c>15)
	return 'N';

    return (iubbases[c]);
}


/* @obsolete ajBinToAZ
** @rename ajBaseBinToAlpha
*/

__deprecated char ajBinToAZ(ajint c)
{
    return ajBaseBinToAlpha(c);
}


/* @section query *************************************************************
**
** Functions
**
** @fdata      [none]
**
** @nam3rule Is test a base code
** @nam3rule Exists Tests code is valid
** @suffix Char Character code
** @suffix Bin Numeric base code
**
** @argrule Bin base [ajint] Binary base code in range 0 to 31
** @argrule Char c [char] Character base code
**
** @valrule Exists [AjBool] True is the code exists
**
** @fcategory use
**
******************************************************************************/

/* @func ajBaseExistsBin ******************************************************
**
** Tests whether a base code exists
**
** @param [r] base [ajint] Base code in range 0 to 31
** @return [AjBool] True if base code is known
**
******************************************************************************/

AjBool ajBaseExistsBin(ajint base)
{
    if(!aj_base_I)
	baseInit();

    if(ajStrGetLen(aj_base_iubS[base].code))
        return ajTrue;
    
    return ajFalse;
}


/* @func ajBaseExistsChar ******************************************************
**
** Tests whether a base code exists
**
** @param [r] c [char] Base character
** @return [AjBool] True if base code is known
**
******************************************************************************/

AjBool ajBaseExistsChar(char c)
{
    int itest;

    if(!aj_base_I)
	baseInit();

    itest = toupper((int)c);
    if(ajStrGetLen(aj_base_iubS[itest].code))
        return ajTrue;
    
    itest = tolower((int)c);
    if(ajStrGetLen(aj_base_iubS[itest].code))
        return ajTrue;

    return ajFalse;
}


/* @section retrieval
**
** Functions
**
** @fdata      [none]
**
** @nam3rule Get Return a value
** @nam4rule GetCodes Returns a string of matching base codes
** @nam4rule GetMnemonic Returns mnemonic string
**
** @argrule Get base [ajint] Binary base code in range 0 to 31
**
** @valrule GetCodes [const AjPStr] Matching base codes
** @valrule GetMnemonic [const AjPStr] Mnemonic for base code
**
** @fcategory use
*/



/* @func ajBaseGetCodes *******************************************************
**
** Returns a string of matching base codes
**
** @param [r] base [ajint] Original base code
**
** @return [const AjPStr] Base codes
******************************************************************************/

const AjPStr ajBaseGetCodes(ajint base)
{
    if(!aj_base_I)
	baseInit();

    return  aj_base_iubS[base].list;
}


/* @obsolete ajBaseCodes
** @rename ajBaseGetCodes
*/

__deprecated const AjPStr ajBaseCodes(ajint ibase)
{
    return ajBaseGetCodes(ibase);
}


/* @func ajBaseGetMnemonic *****************************************************
**
** Returns a string of matching base codes
**
** @param [r] base [ajint] Original base code
**
** @return [const AjPStr] Base codes
******************************************************************************/

const AjPStr ajBaseGetMnemonic(ajint base)
{
    if(!aj_base_I)
	baseInit();

    return  aj_base_iubS[base].mnemonic;
}


/* @funcstatic baseInit ******************************************************
**
** Sets up binary OR'd representation of an IUB bases in a table
** aj_base_table where A=1, C=2, G=4 and T=8
** Also sets up a match probability array aj_base_prob holding the
** probability of one IUB base matching any other.
** Uses the Ebases.iub file
** Is initialised if necessary from other AJAX functions.
**
** @return [AjBool] True on success
******************************************************************************/

static AjBool baseInit(void)
{
    AjPFile bfptr    = NULL;
    AjPStr  bfname   = NULL;
    AjPStr  line     = NULL;
    AjPStr  code     = NULL;
    AjPStr  list     = NULL;
    AjPStr  mnemonic = NULL;

    ajint i;
    ajint j;
    ajint k;

    ajint c;
    ajint qc;

    ajint l1;
    ajint l2;

    ajint x;
    ajint y;

    ajint n;
    const char *p;
    const char *q;

    if(aj_base_I)
	return ajTrue;


    for(i=0;i<256;++i)
    {
	aj_base_iubS[i].code = ajStrNewC("");
	aj_base_iubS[i].list = ajStrNewC("");
	aj_base_iubS[i].mnemonic = ajStrNewC("");
	aj_base_table[i] = 0;
    }

    code = ajStrNew();
    list = ajStrNew();
    ajStrAssignClear(&code);
    ajStrAssignC(&list,"ACGT");


    bfname = ajStrNewC(IUBFILE);
    bfptr = ajDatafileNewInNameS(bfname);
    if(!bfptr) ajFatal("%S file not found\n", bfname);


    line = ajStrNew();


    while(ajReadline(bfptr, &line))
    {
	p = ajStrGetPtr(line);
	if(*p=='#' || *p=='!' || *p=='\n')
	    continue;
	p = ajSysFuncStrtok(p," \t\r\n");
	ajStrAssignC(&code,p);
	p=ajSysFuncStrtok(NULL," \t\r\n");
	if(sscanf(p,"%d",&n)!=1)
	    ajFatal("Bad format IUB file");
	p = ajSysFuncStrtok(NULL," \t\r\n");
	ajStrAssignC(&list,p);
	p = ajSysFuncStrtok(NULL," \t\r\n");
	ajStrAssignC(&mnemonic,p);
	qc = (ajint) ajStrGetCharFirst(code);
	ajStrAssignS(&aj_base_iubS[toupper(qc)].code,code);
	ajStrAssignS(&aj_base_iubS[toupper(qc)].list,list);
	ajStrAssignS(&aj_base_iubS[toupper(qc)].mnemonic,mnemonic);
	ajStrAssignS(&aj_base_iubS[tolower(qc)].code,code);
	ajStrAssignS(&aj_base_iubS[tolower(qc)].list,list);
	ajStrAssignS(&aj_base_iubS[tolower(qc)].mnemonic,mnemonic);
	aj_base_table[toupper(qc)] = n;
	aj_base_table[tolower(qc)] = n;
    }

    ajStrDel(&code);
    ajStrDel(&list);
    ajStrDel(&line);
    ajStrDel(&mnemonic);
    ajStrDel(&bfname);

    ajFileClose(&bfptr);


    for(i=0;i<32;++i)
    {
	x = ajBasecodeFromInt(i);
	for(j=0;j<32;++j)
	{
	    y = ajBasecodeFromInt(j);
	    if(!(l1=ajStrGetLen(aj_base_iubS[x].code)))
	    {
		aj_base_prob[i][j]=0.0;
		continue;
	    }
	    if(l1!=1)
		ajFatal("Bad IUB letter");


	    p = ajStrGetPtr(aj_base_iubS[x].list);
	    q = ajStrGetPtr(aj_base_iubS[y].list);
	    l1 = strlen(p);
	    l2 = strlen(q);
	    for(k=0,c=0;k<l1;++k)
		if(strchr(q,(ajint)*(p+k))) ++c;
	    if(l2)
		aj_base_prob[i][j] = (float)c / (float)l2;
	    else
		aj_base_prob[i][j]=0.0;
	}
    }

    aj_base_I = ajTrue;

    return aj_base_I;
}




/* @section exit
**
** Functions called on exit from the program by ajExit to do
** any necessary cleanup and to report internal statistics to the debug file
**
** @fdata      [none]
**
** @nam3rule Exit Cleanup and report on exit
**
** @valrule * [void]
**
** @fcategory misc
*/


/* @func ajBaseExit ***********************************************************
**
** Cleans up sequence base and residue processing internal memory
**
** @return [void]
** @@
******************************************************************************/

void ajBaseExit(void)
{
    ajint i;

    if(aj_base_I)
        for(i=0;i<256;++i)
        {
            ajStrDel(&aj_base_iubS[i].code);
            ajStrDel(&aj_base_iubS[i].list);
            ajStrDel(&aj_base_iubS[i].mnemonic);
        }
    
    if(aj_residue_I)
        for(i=0;i<256;++i)
        {
            ajStrDel(&aj_residue_iubS[i].code);
            ajStrDel(&aj_residue_iubS[i].list);
            ajStrDel(&aj_residue_iubS[i].mnemonic);
        }
    
    return;
}

/* @datasection [none] Residue *******************************************
**
** Function is for manipulating amino acid residue codes
** 
** @nam2rule Residue 
**
*/




/* @section character conversion
**
** Functions converting binary forms of base codes
**
** @fdata      [none]
**
** @nam3rule Alpha Converts a character value
** @nam4rule AlphaCompare Compares two base codes
** @nam4rule AlphaTo Converts to a specific type
** @nam5rule AlphaToBin Converts to binary code type
**
** @argrule Alpha base [ajint] Alphabetic character as an integer
** @argrule Compare base2 [ajint] Comparison alphabetic character as a character
**
** @valrule Compare [float] Probability bases are the same
** @valrule ToBin [ajint] Binary code in range 0 to 31
**
** @fcategory cast
*/



/* @func ajResidueAlphaToBin **************************************************
**
** Returns a binary OR'd representation of an IUB residue where A=1, C=2,
** Uses the base table set up by residueInit
**
** @param  [r] base [ajint] character to convert
**
** @return [ajint] Binary OR'd representation
******************************************************************************/

ajint ajResidueAlphaToBin(ajint base)
{
    if(!aj_residue_I)
	residueInit();

    return (aj_residue_table[toupper(base)]);
}


/* @section binary conversion
**
** Functions converting binary forms of base codes
**
** @fdata      [none]
**
** @nam3rule Bin Converts a binary code value
** @nam4rule BinTo Converts to a specific type
** @nam5rule BinToAlpha Converts to alphabetic type
**
** @argrule Bin c [ajint] Binary code in range 0 to 31
**
** @valrule ToAlpha [char]
**
** @fcategory cast
*/



/* @func ajResidueBinToAlpha **************************************************
**
** Converts a binary OR'd representation of an IUB residue where A=1, C=2,
** etc into an ambiguous protein code
**
** Uses the base table set up by residueInit
**
** @param  [r] c [ajint] character to convert
**
** @return [char] Ambiguous residue code
******************************************************************************/

char ajResidueBinToAlpha(ajint c)
{
    ajuint i;

    if(!aj_residue_I)
	residueInit();

    for(i = 0;i<256;i++)
    {
        if(aj_residue_table[i] == c)
            return ajStrGetCharFirst(aj_residue_iubS[i].code);
    }
    return 'X';
}





/* @section query *************************************************************
**
** Functions
**
** @fdata      [none]
**
** @nam3rule Is test a base code
** @nam3rule Exists Tests code is valid
** @suffix Char Character code
** @suffix Bin Numeric base code
**
** @argrule Bin base [ajint] Binary base code in range 0 to 31
** @argrule Char c [char] Character base code
**
** @valrule Exists [AjBool] True is the code exists
**
** @fcategory use
**
******************************************************************************/

/* @func ajResidueExistsBin **************************************************
**
** Tests whether a residue code exists
**
** @param [r] base [ajint] Base code in range 0 to 31
** @return [AjBool] True if base code is known
**
******************************************************************************/

AjBool ajResidueExistsBin(ajint base)
{
    if(!aj_residue_I)
	residueInit();

    if(ajStrGetLen(aj_residue_iubS[base].code))
        return ajTrue;
    
    return ajFalse;
}


/* @func ajResidueExistsChar ***************************************************
**
** Tests whether a residue code exists
**
** @param [r] c [char] Base character
** @return [AjBool] True if base code is known
**
******************************************************************************/

AjBool ajResidueExistsChar(char c)
{
    int itest;

    if(!aj_residue_I)
	residueInit();

    itest = toupper((int)c);
    if(ajStrGetLen(aj_residue_iubS[itest].code))
        return ajTrue;
    
    itest = tolower((int)c);
    if(ajStrGetLen(aj_residue_iubS[itest].code))
        return ajTrue;

    return ajFalse;
}


/* @section retrieval
**
** Functions
**
** @fdata      [none]
**
** @nam3rule Get Return a value
** @nam4rule GetCodes Returns a string of matching base codes
** @nam4rule GetMnemonic Returns mnemonic string
**
** @argrule Get base [ajint] Binary base code in range 0 to 31
**
** @valrule GetCodes [const AjPStr] Matching base codes
** @valrule GetMnemonic [const AjPStr] Mnemonic for base code
**
** @fcategory use
*/



/* @func ajResidueGetCodes ****************************************************
**
** Returns a string of matching amino acid residue codes
**
** @param [r] base [ajint] Original base code
**
** @return [const AjPStr] Base codes
******************************************************************************/

const AjPStr ajResidueGetCodes(ajint base)
{
    if(!aj_residue_I)
	residueInit();

    return  aj_residue_iubS[base].list;
}


/* @func ajResidueGetMnemonic ***********************************************
**
** Returns a string of matching amino acid residue codes
**
** @param [r] base [ajint] Original base code
**
** @return [const AjPStr] Base codes
******************************************************************************/

const AjPStr ajResidueGetMnemonic(ajint base)
{
    if(!aj_residue_I)
	residueInit();

    return  aj_residue_iubS[base].mnemonic;
}


/* @funcstatic residueInit ****************************************************
**
** Sets up binary OR'd representation of an IUB residues in a table
** aj_residue_table where A=1, C=2, etc
** Also sets up a match probability array aj_residue_prob holding the
** probability of one IUB residue matching any other.
** Uses the Eresidues.iub file
** Is initialised if necessary from other AJAX functions.
**
** @return [AjBool] True on success
******************************************************************************/

static AjBool residueInit(void)
{
    AjPFile bfptr    = NULL;
    AjPStr  bfname   = NULL;
    AjPStr  line     = NULL;
    AjPStr  code     = NULL;
    AjPStr  list     = NULL;
    AjPStr  mnemonic = NULL;

    ajint i;
    ajint j;
    ajint k;

    ajint c;
    ajint qc;

    ajint l1;
    ajint l2;

    ajint x;
    ajint y;

    ajint n;
    const char *p;
    const char *q;

    if(aj_residue_I)
	return ajTrue;


    for(i=0;i<256;++i)
    {
	aj_residue_iubS[i].code = ajStrNewC("");
	aj_residue_iubS[i].list = ajStrNewC("");
	aj_residue_table[i] = 0;
    }

    code = ajStrNew();
    list = ajStrNew();
    ajStrAssignClear(&code);
    ajStrAssignC(&list,"ABCDEFGHIJKLMNOPQRSTUVWXYZ");


    ajStrAssignC(&bfname,IUBPFILE);
    bfptr = ajDatafileNewInNameS(bfname);
    if(!bfptr) ajFatal("%S file not found\n", bfname);


    while(ajReadline(bfptr, &line))
    {
	p = ajStrGetPtr(line);
	if(*p=='#' || *p=='!' || *p=='\n')
	    continue;
	p = ajSysFuncStrtok(p," \t\r\n");
	ajStrAssignC(&code,p);
	p=ajSysFuncStrtok(NULL," \t\r\n");
	if(sscanf(p,"%d",&n)!=1)
	    ajFatal("Bad format IUB file");
	p = ajSysFuncStrtok(NULL," \t\r\n");
	ajStrAssignC(&list,p);
	p = ajSysFuncStrtok(NULL," \t\r\n");
	ajStrAssignC(&mnemonic,p);
	qc = (ajint) ajStrGetCharFirst(code);
	ajStrAssignS(&aj_residue_iubS[toupper(qc)].code,code);
	ajStrAssignS(&aj_residue_iubS[toupper(qc)].list,list);
	ajStrAssignS(&aj_residue_iubS[toupper(qc)].mnemonic,mnemonic);
	ajStrAssignS(&aj_residue_iubS[tolower(qc)].code,code);
	ajStrAssignS(&aj_residue_iubS[tolower(qc)].list,list);
	ajStrAssignS(&aj_residue_iubS[tolower(qc)].mnemonic,mnemonic);
	aj_residue_table[toupper(qc)] = n;
	aj_residue_table[tolower(qc)] = n;
    }

    ajStrDel(&code);
    ajStrDel(&list);
    ajStrDel(&line);
    ajStrDel(&mnemonic);
    ajStrDel(&bfname);

    ajFileClose(&bfptr);


    for(i=0;i<32;++i)
    {
	x = ajBasecodeFromInt(i);
	for(j=0;j<32;++j)
	{
	    y = ajBasecodeFromInt(j);
	    if(!(l1=ajStrGetLen(aj_residue_iubS[x].code)))
	    {
		aj_residue_prob[i][j]=0.0;
		continue;
	    }
	    if(l1!=1)
		ajFatal("Bad IUB letter");


	    p = ajStrGetPtr(aj_residue_iubS[x].list);
	    q = ajStrGetPtr(aj_residue_iubS[y].list);
	    l1 = strlen(p);
	    l2 = strlen(q);
	    for(k=0,c=0;k<l1;++k)
		if(strchr(q,(ajint)*(p+k))) ++c;
	    if(l2)
		aj_residue_prob[i][j] = (float)c / (float)l2;
	    else
		aj_residue_prob[i][j]=0.0;
	}
    }

    aj_residue_I = ajTrue;

    return aj_residue_I;
}

/* @section Triplet names
**
** Functions exchanging residue codes with three letter codes
**
** @fdata      [none]
**
** @nam3rule From Convert some other form to a residue
** @nam3rule To   Convert residue to another form
** @nam4rule Triplet Convert a triplet 3-letter residue name
**
** @argrule FromTriplet aa3 [const AjPStr] Triplet residue name
** @argrule From Pc [char*] Triplet residue name
** @argrule To c [char] Residue character code
** @argrule ToTriplet Paa3 [AjPStr*] Triplet residue name
**
** @valrule * [AjBool] True if code was recognized.
**
** @fcategory cast
*/






/* @func ajResidueFromTriplet *************************************************
**
** Takes a 3 character amino acid code and writes a char with the 
** corresponding single letter code.
** 
** @param [r] aa3 [const AjPStr]   AjPStr object (3 letter code)
** @param [w] Pc [char *] Resulting residue code
**
** @return [AjBool] True on success, false if triplet is not recognized
** @@
****************************************************************************/

AjBool  ajResidueFromTriplet(const AjPStr aa3, char *Pc)
{
    ajint i;
    
    for(i=0; i<26; i++)
	if(!ajStrCmpC(aa3, BaseAaTable[i]))
	{
	    *Pc = (char) (i + (int) 'A');
	    return ajTrue;
	}
    
    if(!ajStrCmpC(aa3, "UNK"))
    {
	*Pc = 'X';

	return ajTrue;
    }	
    
    *Pc='X';
    return ajFalse;
}

/* @obsolete ajBaseAa3ToAa1
** @replace ajResidueFromTriplet (1,2/2,1)
*/

__deprecated AjBool  ajBaseAa3ToAa1(char *Paa1, const AjPStr aa3)
{
    return ajResidueFromTriplet(aa3, Paa1);
}


/* @func ajResidueToTriplet ***************************************************
**
** Writes an AjPStr with an amino acid 3 letter code
**
** @param [r] c [char] Single letter identifier of amino acid
** @param [w] Paa3  [AjPStr *] AjPStr object
**
** @return [AjBool] True on succcess
** @@
******************************************************************************/
AjBool  ajResidueToTriplet(char c, AjPStr *Paa3)
{
    ajint idx;

     if((idx=ajBasecodeToInt(c))>25)
	return ajFalse;

    ajStrAssignC(Paa3, BaseAaTable[idx]);
    return ajTrue;
}


/* @obsolete ajBaseAa1ToAa3
** @rename ajResidueToTriplet
*/

__deprecated AjBool  ajBaseAa1ToAa3(char aa1, AjPStr *Paa3)
{
    return ajResidueToTriplet(aa1, Paa3);
}

/* @datasection [none] Basecode *******************************************
**
** Function is for coding the letters used for nucleotide bases and
** protein residues as integers from zero for use as array indexes.
** 
** @nam2rule Basecode  
**
*/


/* @section Conversion
**
** @fdata [none]
**
** @nam3rule From Convert from another type
** @nam3rule To Convert To another type
** @nam4rule Int Convert integer code type
**
** @argrule To c [ajint] Base code character to convert from
** @argrule FromInt n [ajint] Integer base code to convert from
**
** @valrule From [ajint] Base code character
** @valrule ToInt [ajint] Integer base code
**
** @fcategory cast
*/

/* @func ajBasecodeFromInt **************************************************
**
** Returns 'A' for 0 to  'Z' for 25
**
** @param  [r] n [ajint] character to convert
**
** @return [ajint] 0 as 'A' up to  25 as 'Z'
** @@
******************************************************************************/

ajint ajBasecodeFromInt(ajint n)
{
    if(n>25)
	return (ajint) '*';
    if(n<0)
	return (ajint) '*';

    return(n+(ajint)'A');
}



/* @obsolete ajIntToAZ
** @rename ajBasecodeFromInt
*/

__deprecated ajint ajIntToAZ(ajint n)
{
    return ajBasecodeFromInt(n);
}


/* @func ajBasecodeToInt *************************************************
**
** Returns A=0 to Z=25  or 27 otherwise
**
** @param  [r] c [ajint] character to convert
**
** @return [ajint] A=0 to Z=25 or 27 if unknown
** @@
******************************************************************************/

ajint ajBasecodeToInt(ajint c)
{
    ajint ic = toupper(c);

    if(ic >= (ajint) 'A' && ic <= (ajint) 'Z')
	return(ic - (ajint)'A');

    return 27;
}


/* @obsolete ajAZToInt
** @rename ajBasecodeToInt
*/

__deprecated ajint ajAZToInt(ajint c)
{

    return ajBasecodeToInt(c);
}


