/******************************************************************************
** @source AJAX translate functions
**
** These functions control all aspects of sequence translation
**
** These functions do not translate to the 'ambiguity' residues
** 'B' (Asn or Asp) and 'Z' (Glu or Gln). So the codons:
** RAC, RAT, RAY, RAU which could code for 'B' return 'X'
** and SAA, SAG, SAR which could code for 'Z' return 'X'.
**
** This translation table doesn't have the doubly ambiguous
** codons set up:
** YTR - L
** MGR - R
** YUR - L
**
** This should be attended to at some time.
**
** @author Copyright (C) 1999 Gary Williams
** @version 2.0
** @modified Feb 15 1999 GWW First version
** @modified April 19 1999 GWW Second version using NCBI's GC tables
** @modified April 18 2000 GWW Reorganised many of the routines
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
*******************************************************************************
**
** Example of typical usage (code fragment):
**
**
** trnTable = ajTrnNewI(table_number);
** while(ajSeqallNext(seqall, &seq))
** {
**     protein_seq = ajTrnSeqFramePep(trnObj, seq, frame)
**     write out protein_seq
**     ajSeqDel(&protein_seq);
** }
** ajTrnDel(&trnTable);
**
** or
**
** trnTable = ajTrnNewI(table_number);
** ajTrnStr(trnTable, seq, &protein_str)
** ajFmtPrintF(outfile, "protein=%S\n", protein_str);
** ajTrnDel(&trnTable);
**
**
**
** Example of typical ACD file list for getting the translation frame:
**
**   list: frame [
**         opt: Y
**         default: "1"
**         min: 1
**         max: 1
**         header: "Translation frames"
**         values:
**                 "1:1,
**                 2:2,
**                 3:3,
**                 F:Forward three frames,
**                 -1:-1,
**                 -2:-2,
**                 -3:-3,
**                 R:Reverse three frames,
**                 6:All six frames"
**         delim: ","
**         codedelim: ":"
**         prompt: "Frame(s) to translate"
**   ]
**
**
** Example of typical ACD file list for getting the genetic code table number:
**
** list: table [
**        opt: Y
**        default: "0"
**        min: 1
**        max: 1
**        header: "Genetic codes"
**        values:
**                "0:Standard;
**                1:Standard (with alternative initiation codons);
**                2:Vertebrate Mitochondrial;
**                3:Yeast Mitochondrial;
**                4:Mold, Protozoan, Coelenterate Mitochondrial
**                  and Mycoplasma/Spiroplasma;
**                5:Invertebrate Mitochondrial;
**                6:Ciliate Macronuclear and Dasycladacean;
**                9:Echinoderm Mitochondrial;
**                10:Euplotid Nuclear;
**                11:Bacterial;
**                12:Alternative Yeast Nuclear;
**                13:Ascidian Mitochondrial;
**                14:Flatworm Mitochondrial;
**                15:Blepharisma Macronuclear;
**                16:Chlorophycean Mitochondrial;
**                21:Trematode Mitochondrial"
**        delim: ";"
**        codedelim: ":"
**        prompt: "Code to use"
**  ]
**
**
**
******************************************************************************/

#include <stddef.h>
#include <stdarg.h>
#include <float.h>
#include <limits.h>

#include "ajax.h"




#define TGCFILE "EGC.0"
#define TGC "EGC."

static AjPTable trnCodes = NULL;


/* table to convert character of base to translation array element value */
static ajint trnconv[] =
{
    /* characters less than 64 */
    14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14,
    14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14,

  /*' '  !   "   #   $   %   &   '   (   )   *   +   ,   -   .   / */
    14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14,

  /* 0   1   2   3   4   5   6   7   8   9   :   ;   <   =   >   ? */
    14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14,

  /* @   A   B   C   D   E   F   G   H   I   J   K   L   M   N   O */
    14,  0, 13,  1, 12, 14, 14,  2, 11, 14, 14,  9, 14,  4, 14, 14,

  /* P   Q   R   S   T   U   V   W   X   Y   Z   [   \   ]   ^   _ */
    14, 14,  5,  7,  3,  3, 10,  6, 14,  8, 14, 14, 14, 14, 14, 14,

  /* `   a   b   c   d   e   f   g   h   i   j   k   l   m   n   o */
    14,  0, 13,  1, 12, 14, 14,  2, 11, 14, 14,  9, 14,  4, 14, 14,

  /* p   q   r   s   t   u   v   w   x   y   z   {   |   }   ~   del */
    14, 14,  5,  7,  3,  3, 10,  6, 14,  8, 14, 14, 14, 14, 14, 14
};


/*
 ** table to convert character of COMPLEMENT of base to translation array
 ** element value
 */

static ajint trncomp[] =
{
    /* characters less than 64 */
    14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14,
    14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14,

  /*' '  !   "   #   $   %   &   '   (   )   *   +   ,   -   .   / */
    14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14,

  /* 0   1   2   3   4   5   6   7   8   9   :   ;   <   =   >   ? */
    14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14,

  /* @   A   B   C   D   E   F   G   H   I   J   K   L   M   N   O*/
    14,  3, 10,  2, 11, 14, 14,  1, 12, 14, 14,  4, 14,  9, 14, 14,

  /* P   Q   R   S   T   U   V   W   X   Y   Z   [   \   ]   ^   _ */
    14, 14,  8,  7,  0,  0, 13,  6, 14,  5, 14, 14, 14, 14, 14, 14,

  /* `   a   b   c   d   e   f   g   h   i   j   k   l   m   n   o */
    14,  3, 10,  2, 11, 14, 14,  1, 12, 14, 14,  4, 14,  9, 14, 14,

  /* p   q   r   s   t   u   v   w   x   y   z   {   |   }   ~   del */
    14, 14,  8,  7,  0,  0, 13,  6, 14,  5, 14, 14, 14, 14, 14, 14
};


static AjPStr trnResidueStr = NULL;

static void trnNoComment(AjPStr* text);

static AjBool trnComplete(AjPTrn thys);




/* @func ajTrnDel *************************************************************
**
** Deletes a translation table object
**
** @param [d] pthis [AjPTrn*] Address of translation table object
** @return [void]
** @category delete [AjPTrn] Default destructor
** @@
******************************************************************************/

void ajTrnDel(AjPTrn* pthis)
{
    AjPTrn thys;

    if(!pthis)
        return;

    thys = *pthis;

    if(!thys)
        return;

    ajStrDel(&thys->FileName);
    ajStrDel(&thys->Title);

    AJFREE(*pthis);

    return;
}




/* @func ajTrnNewC ************************************************************
**
** Initialises translation. Reads a translation data file.
**
** @param [r] filename [const char*] translation table file name
** @return [AjPTrn] Translation object
** @category new [AjPTrn] Default constructor
** @@
******************************************************************************/

AjPTrn ajTrnNewC(const char * filename)
{
    AjPStr trnFileName = NULL;

    trnFileName = ajStrNewC(filename);

    return ajTrnNew(trnFileName);

}




/* @func ajTrnNewI ************************************************************
**
** Initialises translation. Reads a translation data file called 'EGC.x'
** where 'x' is supplied as an ajint parameter.
** The filename must exist somewhere in the data path.
** ajTrnDel should be called when translation has ceased.
**
** @param [r] trnFileNameInt [ajint] translation table file name number
** @return [AjPTrn] Translation object
** @category new [AjPTrn] Default constructor
** @@
******************************************************************************/

AjPTrn ajTrnNewI(ajint trnFileNameInt)
{
    AjPStr trnFileName = NULL;
    AjPStr value       = NULL;
    AjPTrn ret = NULL;

    value       = ajStrNew();
    trnFileName = ajStrNewC(TGC);

    ajStrFromInt(&value, trnFileNameInt);
    ajStrAppendS(&trnFileName, value);

    ret = ajTrnNew(trnFileName);

    ajStrDel(&value);
    ajStrDel(&trnFileName);

    return ret;
}




/* @func ajTrnNew *************************************************************
**
** Initialises translation. Reads a translation data file
** ajTrnDel should be called when translation has ceased.
**
** @param [r] trnFileName [const AjPStr] translation table file name
** @return [AjPTrn] Translation object
** @category new [AjPTrn] Default constructor
** @@
******************************************************************************/

AjPTrn ajTrnNew(const AjPStr trnFileName)
{
    AjPFile trnFile = NULL;
    AjPTrn pthis;
    ajint i;
    ajint j;
    ajint k;

    /* open the translation table file */

    /* if the file is not specified, use the standard table file */
    if(!ajStrGetLen(trnFileName))
	trnFileName = ajStrNewC(TGCFILE);


    trnFile = ajDatafileNewInNameS(trnFileName);

    if(trnFile==NULL)
	ajFatal("Translation table file '%S' not found\n", trnFileName);

    /* create and initialise the translation object */
    AJNEW0(pthis);
    pthis->FileName = ajStrNew();
    pthis->Title    = ajStrNew();

    /* initialise the GC and Starts tables */
    for(i=0; i<16; i++)
	for(j=0; j<16; j++)
	    for(k=0; k<16; k++)
	    {
		pthis->GC[i][j][k] = 'X';
		pthis->Starts[i][j][k] = '-';
	    }

    ajStrAssignS(&(pthis->FileName), trnFileName);
    ajTrnReadFile(pthis, trnFile);

    ajFileClose(&trnFile);

    return pthis;
}




/* @func ajTrnReadFile ********************************************************
**
** Reads a translation data file
**
** The destructor ajTrnDel should be called when translation has ceased.
**
** @param [w] trnObj [AjPTrn] translation table object
** @param [u] trnFile [AjPFile] translation table file handle
** @return [void]
** @category input [AjPTrn] Reads a Genetic Code file
** @@
******************************************************************************/

void ajTrnReadFile(AjPTrn trnObj, AjPFile trnFile)
{
    AjPStr trnLine    = NULL;
    AjPStr trnText    = NULL;
    AjPStr tmpstr     = NULL;
    AjPStr aaline     = NULL;
    AjPStr startsline = NULL;
    AjPStr base1line  = NULL;
    AjPStr base2line  = NULL;
    AjPStr base3line  = NULL;

    AjPStrTok tokenhandle;

    const char *aa;
    const char *starts;
    const char *base1;
    const char *base2;
    const char *base3;
    ajint dlen;
    ajint i;


    /* positions of first use of a residue in the aa line */
    /* ajint firstaa[256]; Now usused */

    /*
    ** NB '-' and '*' are valid characters,
    ** don't skip over them when parsing tokens
    */
    char white[] = " \t\n\r!@#$%^&()_+=|\\~`{[}]:;\"'<,>.?/";

    ajDebug("ajTrnReadFile %F\n", trnFile);

    while(ajReadlineTrim(trnFile, &trnLine))
    {
	trnNoComment(&trnLine);

	if(ajStrGetLen(trnLine))
	{
	    if(ajStrFindC(trnLine, "Genetic Code") == -1)
		ajFatal("The file '%S' is not a valid Genetic Code file.\n"
			"The 'Genetic Code' line was not found.",
			trnObj->FileName);
	    else
		break;
	}
    }

    /* title */
    while(ajReadlineTrim(trnFile, &trnLine))
    {
	trnNoComment(&trnLine);

	if(ajStrGetLen(trnLine))
	{
	    ajStrAssignS(&(trnObj->Title), trnLine);
	    break;
	}
    }

    /* rest */
    while(ajReadlineTrim(trnFile, &trnLine))
    {
	trnNoComment(&trnLine);

	if(ajStrGetLen(trnLine))
	{
	    ajStrAppendS(&trnText, trnLine);
	    ajStrAppendC(&trnText, " ");
	}
    }

    /* data */
    tokenhandle = ajStrTokenNewC(trnText, white);

    ajStrTokenNextParse(&tokenhandle, &tmpstr);

    if(ajStrCmpC(tmpstr, "AAs") == -1)
	ajFatal("The file '%S' is not a valid Genetic Code file.\n"
		"The 'AAs' line was not found.", trnObj->FileName);

    ajStrTokenNextParse(&tokenhandle, &aaline);
    aa = ajStrGetPtr(aaline);

    ajStrTokenNextParse(&tokenhandle, &tmpstr);

    if(ajStrCmpC(tmpstr, "Starts") == -1)
	ajFatal("The file '%S' is not a valid Genetic Code file.\n"
		"The 'Starts' line was not found.", trnObj->FileName);

    ajStrTokenNextParse(&tokenhandle, &startsline);
    starts = ajStrGetPtr(startsline);

    ajStrTokenNextParse(&tokenhandle, &tmpstr);

    if(ajStrCmpC(tmpstr, "Base1") == -1)
	ajFatal("The file '%S' is not a valid Genetic Code file.\n"
		"The 'Base1' line was not found.", trnObj->FileName);

    ajStrTokenNextParse(&tokenhandle, &base1line);
    base1 = ajStrGetPtr(base1line);

    ajStrTokenNextParse(&tokenhandle, &tmpstr);

    if(ajStrCmpC(tmpstr, "Base2") == -1)
	ajFatal("The file '%S' is not a valid Genetic Code file.\n"
		"The 'Base2' line was not found.", trnObj->FileName);

    ajStrTokenNextParse(&tokenhandle, &base2line);
    base2 = ajStrGetPtr(base2line);

    ajStrTokenNextParse(&tokenhandle, &tmpstr);

    if(ajStrCmpC(tmpstr, "Base3") == -1)
	ajFatal("The file '%S' is not a valid Genetic Code file.\n"
		"The 'Base3' line was not found.", trnObj->FileName);

    ajStrTokenNextParse(&tokenhandle, &base3line);
    base3 = ajStrGetPtr(base3line);


    ajStrTokenDel(&tokenhandle);
    ajStrDel(&tmpstr);

    /* populate the Starts(Initiation sites) table */
    dlen = ajStrGetLen(startsline);

    for(i=0; i<dlen; i++)
	trnObj->Starts[trnconv[(ajint)base1[i]]]
	              [trnconv[(ajint)base2[i]]]
		      [trnconv[(ajint)base3[i]]]
		    = starts[i];

    /* populate the GC (Genetic code) table */
    dlen = ajStrGetLen(aaline);

    /* initialise first use of aa array */
    /* Now usused 
    for(i=0; i<256; i++)
	firstaa[i] = -1;
    */

    for(i=0; i<dlen; i++)
    {
	/*
	** put the residue in the table using the unambiguous codon
	*/
	trnObj->GC[trnconv[(ajint)base1[i]]]
	    [trnconv[(ajint)base2[i]]]
		[trnconv[(ajint)base3[i]]]
		    = aa[i];
    }

    /*
    ** Check we have defined every codon
    ** Calculate wobble in base3 and populate
    ** For each amino acid (just once) calculate most ambiguous and populate
    */

    trnComplete(trnObj);

    ajStrDel(&trnText);
    ajStrDel(&startsline);
    ajStrDel(&base1line);
    ajStrDel(&base2line);
    ajStrDel(&base3line);
    ajStrDel(&aaline);
    ajStrDel(&trnLine);

    return;
}




/* @funcstatic trnNoComment ***************************************************
**
** Strips comments from a character string (a line from an trn file).
** Comments are blank lines or any text following a "#" character.
** Whitespace characters can be included in a blank line.
**
** @param [u] text [AjPStr*] Line of text from input file
** @return [void]
** @@
******************************************************************************/

static void trnNoComment(AjPStr* text)
{
    ajint i;
    char *cp;

    ajStrTrimWhite(text);
    i = ajStrGetLen(*text);

    if(!i)
	return;

    cp = strchr(ajStrGetuniquePtr(text), '#');

    if(cp)
    {
	/* comment found */
	*cp = '\0';
	ajStrSetValid(text);
    }

    return;
}




/* @func ajTrnNewPep **********************************************************
**
** Creates a new AjPSeq set up with an appropriate name and description
**
** It:
**  Creates a AjPSeq object
**  Sets it to be a protein.
**  Sets the description as being the same as that description of the nucleic
**    acid sequence it was translated from.
**  Gives it the same name as the nucleic acid sequence it is translated from.
**
** If the frame is not specified as being '0' it will then append a '_'
** and the number of the frame to form a unique name for the protein
** sequence in the event of many frames being translated.  If the frame
** number is negative, it will use a number in the range 4, 5, 6, this is
** because ID names with '-' in them were causing problems in the sequence
** reading routines.
**
** Frame 4 is the same as frame -1, 5 is -2, 6 is -3.
**
** You will have to set the sequence of this object with something like:
**  ajSeqAssignSeqS(trnPeptide, seqstr);
**
**
** @param [r] nucleicSeq [const AjPSeq] nucleic sequence being translated
** @param [r] frame [ajint] frame of translation (-3,-2,-1,0,1,2,3,4,5,6)
** @return [AjPSeq] New peptide object
** @category new [AjPSeq] Peptide object constructor
** @@
******************************************************************************/

AjPSeq ajTrnNewPep(const AjPSeq nucleicSeq, ajint frame)
{

    AjPSeq trnPeptide = NULL;
    AjPStr name       = NULL;		/* name of the translation */
    AjPStr value      = NULL;  /* value of frame of the translation */

    trnPeptide = ajSeqNew();
    ajSeqSetProt(trnPeptide);

    name  = ajStrNew();
    value = ajStrNew();

    /* name for the subsequence */
    ajStrAssignS(&name, ajSeqGetNameS(nucleicSeq));

    /*
    ** if the frame is not 0 then append the frame number to the name to
    **make it unique
    */
    if(frame != 0)
    {
	if(frame < -3)
            frame = frame + 3;

	if(frame < 0)
            frame = -frame + 3;

	ajStrAppendC(&name, "_");

	ajStrFromInt(&value, frame);
	ajStrAppendS(&name, value);
    }

    ajSeqAssignNameS(trnPeptide, name);

    ajSeqAssignDescS(trnPeptide, ajSeqGetDescS(nucleicSeq));

    ajStrDel(&name);
    ajStrDel(&value);

    return trnPeptide;
}




/* @func ajTrnCodon ***********************************************************
**
** Translates a codon
**
** @param [r] trnObj [const AjPTrn] Translation tables
** @param [r] codon [const AjPStr] codon to translate
** @return [const AjPStr] Amino acid translation
** @category use [AjPTrn] Translating a codon from an AjPStr
** @@
******************************************************************************/

const AjPStr ajTrnCodon(const AjPTrn trnObj, const AjPStr codon)
{
    const char * res;
    char store[2];

    store[1] = '\0';			/* end the char * of store */

    res = ajStrGetPtr(codon);
    store[0] = trnObj->GC[trnconv[(ajint)res[0]]]
	                 [trnconv[(ajint)res[1]]]
	                 [trnconv[(ajint)res[2]]];

    ajStrAssignC(&trnResidueStr, store);

    return trnResidueStr;
}




/* @func ajTrnRevCodon ********************************************************
**
** Translates the reverse complement of a codon
**
** @param [r] trnObj [const AjPTrn] Translation tables
** @param [r] codon [const AjPStr] codon to translate
** @return [const AjPStr] Amino acid translation
** @category use [AjPTrn] Reverse complement translating a codon
**                from an AjPStr
** @@
******************************************************************************/

const AjPStr ajTrnRevCodon(const AjPTrn trnObj, const AjPStr codon)
{
    const char * res;
    char store[2];

    store[1] = '\0';			/* end the char * of store */

    res = ajStrGetPtr(codon);
    store[0] = trnObj->GC[trncomp[(ajint)res[2]]]
	                 [trncomp[(ajint)res[1]]]
	                 [trncomp[(ajint)res[0]]];

    ajStrAssignC(&trnResidueStr, store);

    return trnResidueStr;
}




/* @func ajTrnCodonC **********************************************************
**
** Translates a const char * codon
**
** @param [r] trnObj [const AjPTrn] Translation tables
** @param [r] codon [const char *] codon to translate
**                           (these 3 characters need not be NULL-terminated)
** @return [const AjPStr] Amino acid translation
** @category use [AjPTrn] Translating a codon from a char* text
** @@
******************************************************************************/

const AjPStr ajTrnCodonC(const AjPTrn trnObj, const char *codon)
{
    char store[2];


    store[0] = trnObj->GC[trnconv[(ajint)codon[0]]]
	                 [trnconv[(ajint)codon[1]]]
	                 [trnconv[(ajint)codon[2]]];
    store[1] = '\0';

    ajStrAssignC(&trnResidueStr, store);

    return trnResidueStr;
}




/* @func ajTrnRevCodonC *******************************************************
**
** Translates the reverse complement of a const char * codon
**
** @param [r] trnObj [const AjPTrn] Translation tables
** @param [r] codon [const char *] codon to translate
**                           (these 3 characters need not be NULL-terminated)
** @return [const AjPStr] Amino acid translation
** @category use [AjPTrn] Translating a codon from a char* text
** @@
******************************************************************************/

const AjPStr ajTrnRevCodonC(const AjPTrn trnObj, const char *codon)
{
    char store[2];


    store[0] = trnObj->GC[trncomp[(ajint)codon[2]]]
	                 [trncomp[(ajint)codon[1]]]
	                 [trncomp[(ajint)codon[0]]];
    store[1] = '\0';

    ajStrAssignC(&trnResidueStr, store);

    return trnResidueStr;
}




/* @func ajTrnCodonK **********************************************************
**
** Translates a const char * codon to a char
**
** @param [r] trnObj [const AjPTrn] Translation tables
** @param [r] codon [const char *] codon to translate
**                           (these 3 characters need not be NULL-terminated)
** @return [char] Amino acid translation
** @category use [AjPTrn] Translating a codon from a char* to a
**                char
** @@
******************************************************************************/

char ajTrnCodonK(const AjPTrn trnObj, const char *codon)
{
    return trnObj->GC[trnconv[(ajint)codon[0]]]
	             [trnconv[(ajint)codon[1]]]
	             [trnconv[(ajint)codon[2]]];
}




/* @func ajTrnRevCodonK *******************************************************
**
** Translates a the reverse complement of a const char * codon to a char
**
** @param [r] trnObj [const AjPTrn] Translation tables
** @param [r] codon [const char *] codon to translate
**                           (these 3 characters need not be NULL-terminated)
** @return [char] Amino acid translation
** @category use [AjPTrn] Reverse complement translating a codon
**                from a char* to a char
** @@
******************************************************************************/

char ajTrnRevCodonK(const AjPTrn trnObj, const char *codon)
{

    return trnObj->GC[trncomp[(ajint)codon[2]]]
	             [trncomp[(ajint)codon[1]]]
	             [trncomp[(ajint)codon[0]]];

}




/* @func ajTrnC ***************************************************************
**
** Translates a sequence in a char *
**
** This routine translates in frame 1 (from the first base) to the last full
** triplet codon, (i.e. if there are 1 or 2 bases extra at the end, they are
** ignored)
**
**
** @param [r] trnObj [const AjPTrn] Translation tables
** @param [r] str [const char *] sequence string to translate
** @param [r] len [ajint] length of sequence string to translate
** @param [u] pep [AjPStr *] returned peptide translation (APPENDED TO INPUT)
**
** @return [void]
** @category use [AjPTrn] Translating a sequence from a char* text
** @@
******************************************************************************/

void ajTrnC(const AjPTrn trnObj, const char *str, ajint len, AjPStr *pep)
{
    ajint i;
    ajint lenmod3;
    const char *cp = str;
    AjPStr transtr = NULL;
    char *cq;
    ajint trnlen;

    lenmod3 = len - (len % 3);
    trnlen = lenmod3/3;

    transtr = ajStrNewRes(trnlen+1);
    cq = ajStrGetuniquePtr(&transtr);

    for(i=0; i < lenmod3; i+=3)
    {
        *cq++ =  trnObj->GC[trnconv[(ajint)*cp]]
	  [trnconv[(ajint)*(cp+1)]]
	  [trnconv[(ajint)*(cp+2)]];
	cp+=3;
    }
    ajStrSetValidLen(&transtr, trnlen);
    ajStrAppendS(pep, transtr);
    ajStrDel(&transtr);

    return;
}




/* @func ajTrnRevC ************************************************************
**
** Translates the reverse complement of a sequence in a char *.
**
** This routine translates in frame -1 (using the frame '1' codons)
** to the first full triplet codon,
** (i.e. if there are 1 or 2 bases extra at the start, they are ignored)
**
**
** @param [r] trnObj [const AjPTrn] Translation tables
** @param [r] str [const char *] sequence string to translate
** @param [r] len [ajint] length of sequence string to translate
** @param [u] pep [AjPStr *] returned peptide translation (APPENDED TO INPUT)
**
** @return [void]
** @category use [AjPTrn] Reverse complement translating a sequence
**                from a char* text
** @@
******************************************************************************/

void ajTrnRevC(const AjPTrn trnObj, const char *str, ajint len, AjPStr *pep)
{
    ajint i;
    ajint end;
    const char *cp;
    AjPStr transtr = NULL;
    char *cq;
    ajint trnlen;

    end = (len/3)*3-1;
    trnlen = (end+1)/3;
    cp = &str[end];
    transtr = ajStrNewRes(trnlen+1);
    cq = ajStrGetuniquePtr(&transtr);

    for(i=end; i>1; i-=3)
    {
	*cq++ = trnObj->GC[trncomp[(ajint)*cp]]
	  [trncomp[(ajint)*(cp-1)]]
	  [trncomp[(ajint)*(cp-2)]];
	cp-=3;
    }

    ajStrSetValidLen(&transtr, trnlen);
    ajStrAppendS(pep, transtr);
    ajStrDel(&transtr);

    return;
}




/* @func ajTrnAltRevC *********************************************************
**
** Translates the reverse complement of a sequence in a char *.
**
** This routine translates in frame -4 (from the last base) to the first full
** triplet codon, (i.e. if there are 1 or 2 bases extra at the start, they are
** ignored).
** This routine is for those people who define frame '-1' as being the
** frame starting from the first base of a reverse-complemented sequence.
**
**
** @param [r] trnObj [const AjPTrn] Translation tables
** @param [r] str [const char *] sequence string to translate
** @param [r] len [ajint] length of sequence string to translate
** @param [u] pep [AjPStr *] returned peptide translation (APPENDED TO INPUT)
**
** @return [void]
** @category use [AjPTrn] (Alt) Reverse complement translating a
**                sequence from a char* text
** @@
******************************************************************************/

void ajTrnAltRevC(const AjPTrn trnObj, const char *str, ajint len, AjPStr *pep)
{
    ajint i;

    for(i=len-1; i>1; i-=3)
	ajStrAppendK(pep, trnObj->GC[trncomp[(ajint)str[i]]]
		                 [trncomp[(ajint)str[i-1]]]
		                 [trncomp[(ajint)str[i-2]]]);

    return;
}




/* @func ajTrnStr *************************************************************
**
** Translates a sequence in a AjPStr.
**
** This routine translates in frame 1 (from the first base) to the last full
** triplet codon, (i.e. if there are 1 or 2 bases extra at the end, they are
** ignored)
**
**
** @param [r] trnObj [const AjPTrn] Translation tables
** @param [r] str [const AjPStr] sequence string to translate
** @param [u] pep [AjPStr *] returned peptide translation (APPENDED TO INPUT)
**
** @return [void]
** @category use [AjPTrn] Translating a sequence from a
**                AjPStr
** @@
******************************************************************************/

void ajTrnStr(const AjPTrn trnObj, const AjPStr str, AjPStr *pep)
{
    ajTrnC(trnObj, ajStrGetPtr(str), ajStrGetLen(str), pep);

    return;
}




/* @func ajTrnRevStr **********************************************************
**
** Translates the reverse complement of a sequence in a AjPStr.
**
** This routine translates in frame -1 (from the first base) to the last full
** triplet codon, (i.e. if there are 1 or 2 bases extra at the end, they are
** ignored)
**
**
** @param [r] trnObj [const AjPTrn] Translation tables
** @param [r] str [const AjPStr] sequence string to translate
** @param [u] pep [AjPStr *] returned peptide translation (APPENDED TO INPUT)
**
** @return [void]
** @category use [AjPTrn] Reverse complement translating a sequence
**                from a AjPStr
** @@
******************************************************************************/

void ajTrnRevStr(const AjPTrn trnObj, const AjPStr str, AjPStr *pep)
{
    ajTrnRevC(trnObj, ajStrGetPtr(str), ajStrGetLen(str), pep);

    return;
}




/* @func ajTrnAltRevStr *******************************************************
**
** Translates the reverse complement of a sequence in a AjPStr.
**
** This routine translates in frame -4 (from the last base) to the first full
** triplet codon, (i.e. if there are 1 or 2 bases extra at the start, they are
** ignored).
** This routine is for those people who define frame '-1' as being the
** frame starting from the first base of a reverse-complemented sequence.
**
**
** @param [r] trnObj [const AjPTrn] Translation tables
** @param [r] str [const AjPStr] sequence string to translate
** @param [u] pep [AjPStr *] returned peptide translation (APPENDED TO INPUT)
**
** @return [void]
** @category use [AjPTrn] (Alt) Reverse complement translating a
**                sequence from a AjPStr
** @@
******************************************************************************/

void ajTrnAltRevStr(const AjPTrn trnObj, const AjPStr str, AjPStr *pep)
{
    ajTrnAltRevC(trnObj, ajStrGetPtr(str), ajStrGetLen(str), pep);

    return;
}




/* @func ajTrnSeq *************************************************************
**
** Translates a sequence in a AjPSeq
**
** This routine translates in frame 1 (from the first base) to the last full
** triplet codon, (i.e. if there are 1 or 2 bases extra at the end, they are
** ignored)
**
**
** @param [r] trnObj [const AjPTrn] Translation tables
** @param [r] seq [const AjPSeq] sequence to translate
** @param [u] pep [AjPStr *] returned peptide translation (APPENDED TO INPUT)
**
** @return [void]
** @category use [AjPTrn] Translating a sequence from a
**                AjPSeq
** @@
******************************************************************************/

void ajTrnSeq(const AjPTrn trnObj, const AjPSeq seq, AjPStr *pep)
{
    ajTrnC(trnObj, ajSeqGetSeqC(seq), ajSeqGetLen(seq), pep);

    return;
}




/* @func ajTrnRevSeq **********************************************************
**
** Translates the reverse complement of a sequence in a AjPSeq
** The translation is APPENDED to the input peptide.
**
** This routine translates in frame 1 (from the first base) to the last full
** triplet codon, (i.e. if there are 1 or 2 bases extra at the end, they are
** ignored)
**
**
** @param [r] trnObj [const AjPTrn] Translation tables
** @param [r] seq [const AjPSeq] sequence to translate
** @param [u] pep [AjPStr *] returned peptide translation (APPENDED TO INPUT)
**
** @return [void]
** @category use [AjPTrn] Reverse complement translating a sequence
**                from a AjPSeq
** @@
******************************************************************************/

void ajTrnRevSeq(const AjPTrn trnObj, const AjPSeq seq, AjPStr *pep)
{
    ajTrnRevC(trnObj, ajSeqGetSeqC(seq), ajSeqGetLen(seq), pep);

    return;
}




/* @func ajTrnAltRevSeq *******************************************************
**
** Translates the reverse complement of a sequence in a AjPSeq
** The translation is APPENDED to the input peptide.
**
** This routine translates in frame -4 (from the last base) to the first full
** triplet codon, (i.e. if there are 1 or 2 bases extra at the start, they are
** ignored).
** This routine is for those people who define frame '-1' as being the
** frame starting from the first base of a reverse-complemented sequence.
**
**
**
** @param [r] trnObj [const AjPTrn] Translation tables
** @param [r] seq [const AjPSeq] sequence to translate
** @param [u] pep [AjPStr *] returned peptide translation (APPENDED TO INPUT)
**
** @return [void]
** @category use [AjPTrn] Reverse complement translating a sequence
**                from a AjPSeq
** @@
******************************************************************************/

void ajTrnAltRevSeq(const AjPTrn trnObj, const AjPSeq seq, AjPStr *pep)
{
    ajTrnAltRevC(trnObj, ajSeqGetSeqC(seq), ajSeqGetLen(seq), pep);

    return;
}




/* @func ajTrnCFrame **********************************************************
**
** Translates a sequence in a char * in the specified frame.
** The translation is APPENDED to the input peptide.
**
** This routine translates in the specified frame (one of:
** 1,2,3,-1,-2,-3,4,5,6,-4,-5,-6) to the last full triplet codon,
** (i.e.  if there are 1 or 2 bases extra at the end, they are ignored).
**
** Frame -1 is defined as the translation of the reverse complement
** sequence which matches the codons used in frame 1.  ie.  in the sequence
** ACGT, the first codon of frame 1 is ACG and the last codon of frame -1
** is the reverse complement of ACG (ie.  CGT).
**
** Frame -4 is defined as the translation from the last base to the first full
** triplet codon.
** This routine is for those people who define frame '-1' as being the
** frame starting from the first base of a reverse-complemented sequence.
** This is also known as the 'alternative frame -1'.
** Frame -5 starts on the penultimate base. (Alternative frame -2)
** Frame -6 starts on the ante-penultimate base. (Alternative frame -3)
**
** Frame 4 is the same as frame -1, 5 is -2, 6 is -3.
**
** @param [r] trnObj [const AjPTrn] Translation tables
** @param [r] seq [const char *] sequence string to translate
** @param [r] len [ajint] length of sequence string to translate
** @param [r] frame [ajint] frame to translate in
** @param [u] pep [AjPStr *] returned peptide translation (APPENDED TO INPUT)
**
** @return [void]
** @category use [AjPTrn] Translating a sequence from a char* in a
**                frame
** @@
******************************************************************************/

void ajTrnCFrame(const AjPTrn trnObj, const char *seq, ajint len, ajint frame,
		 AjPStr *pep)
{

    if(frame > 3)
        frame = -frame + 3;

    if(frame >= 1 && frame <= 3)
    {
	/* len = REAL length passed over */
	ajTrnC(trnObj, &seq[frame-1], len-frame+1, pep);
    }
    else if(frame >= -3 && frame <= -1)
    {
	/* len = REAL length passed over */
	ajTrnRevC(trnObj, &seq[-frame-1], len+frame+1, pep);
    }
    else if(frame >= -6 && frame <= -4)
	ajTrnAltRevC(trnObj, seq, len+frame+4 , pep);
    else
	ajFatal("Invalid frame '%d' in ajTrnCFrame()\n", frame);

    return;
}




/* @func ajTrnStrFrame ********************************************************
**
** Translates a sequence in a AjStr in the specified frame.
** The translation is APPENDED to the input peptide.
**
** This routine translates in the specified frame (one of:
** 1,2,3,-1,-2,-3,4,5,6,-4,-5,-6) to the last full triplet codon,
** (i.e.  if there are 1 or 2 bases extra at the end, they are ignored).
**
** Frame -1 is defined as the translation of the reverse complement
** sequence which matches the codons used in frame 1.  ie.  in the sequence
** ACGT, the first codon of frame 1 is ACG and the last codon of frame -1
** is the reverse complement of ACG (ie.  CGT).
**
** Frame -4 is defined as the translation from the last base to the first full
** triplet codon.
** This routine is for those people who define frame '-1' as being the
** frame starting from the first base of a reverse-complemented sequence.
** This is also known as the 'alternative frame -1'.
** Frame -5 starts on the penultimate base. (Alternative frame -2)
** Frame -6 starts on the ante-penultimate base. (Alternative frame -3)
**
** Frame 4 is the same as frame -1, 5 is -2, 6 is -3.
**
** @param [r] trnObj [const AjPTrn] Translation tables
** @param [r] seq [const AjPStr] sequence string to translate
** @param [r] frame [ajint] frame to translate in
** @param [u] pep [AjPStr *] returned peptide translation (APPENDED TO INPUT)
**
** @return [void]
** @category use [AjPTrn] Translating a sequence from a AjPStr in a
**                frame
** @@
******************************************************************************/

void ajTrnStrFrame(const AjPTrn trnObj, const AjPStr seq, ajint frame,
		   AjPStr *pep)
{
    ajTrnCFrame(trnObj, ajStrGetPtr(seq), ajStrGetLen(seq), frame, pep);

    return;
}




/* @func ajTrnSeqFrame ********************************************************
**
** Translates a sequence in a AjSeq in the specified frame.
** The translation is APPENDED to the input peptide.
**
** This routine translates in the specified frame (one of:
** 1,2,3,-1,-2,-3,4,5,6,-4,-5,-6) to the last full triplet codon,
** (i.e.  if there are 1 or 2 bases extra at the end, they are ignored).
**
** Frame -1 is defined as the translation of the reverse complement
** sequence which matches the codons used in frame 1.  ie.  in the sequence
** ACGT, the first codon of frame 1 is ACG and the last codon of frame -1
** is the reverse complement of ACG (ie.  CGT).
**
** Frame -4 is defined as the translation from the last base to the first full
** triplet codon.
** This routine is for those people who define frame '-1' as being the
** frame starting from the first base of a reverse-complemented sequence.
** This is also known as the 'alternative frame -1'.
** Frame -5 starts on the penultimate base. (Alternative frame -2)
** Frame -6 starts on the ante-penultimate base. (Alternative frame -3)
**
** Frame 4 is the same as frame -1, 5 is -2, 6 is -3.
**
**
** @param [r] trnObj [const AjPTrn] Translation tables
** @param [r] seq [const AjPSeq] sequence string to translate
** @param [r] frame [ajint] frame to translate in
** @param [u] pep [AjPStr *] returned peptide translation (APPENDED TO INPUT)
**
** @return [void]
** @category use [AjPTrn] Translating a sequence from a AjPSeq in a
**                frame
** @@
******************************************************************************/

void ajTrnSeqFrame(const AjPTrn trnObj, const AjPSeq seq, ajint frame,
		   AjPStr *pep)
{
    ajTrnCFrame(trnObj, ajSeqGetSeqC(seq), ajSeqGetLen(seq), frame, pep);

    return;
}




/* @func ajTrnSeqFramePep *****************************************************
**
** Translates a sequence in a AjSeq in the specified frame and returns a
** new peptide.
**
** This routine translates in the specified frame (one of:
** 1,2,3,-1,-2,-3,4,5,6,-4,-5,-6) to the last full triplet codon,
** (i.e.  if there are 1 or 2 bases extra at the end, they are ignored).
**
** Frame -1 is defined as the translation of the reverse complement
** sequence which matches the codons used in frame 1.  ie.  in the sequence
** ACGT, the first codon of frame 1 is ACG and the last codon of frame -1
** is the reverse complement of ACG (ie.  CGT).
**
** Frame -4 is defined as the translation from the last base to the first full
** triplet codon.
** This routine is for those people who define frame '-1' as being the
** frame starting from the first base of a reverse-complemented sequence.
** This is also known as the 'alternative frame -1'.
** Frame -5 starts on the penultimate base. (Alternative frame -2)
** Frame -6 starts on the ante-penultimate base. (Alternative frame -3)
**
** Frame 4 is the same as frame -1, 5 is -2, 6 is -3.
**
** NB.  that the naming of the output sequence is always to take
** the name of the input sequence (eg.  ECARGS) and to append an underscore
** character and the frame number 1 to 3 for forward frames and 4 to 6 for
** reverse frames regardless of the final orientation of the reverse
** frames.  (i.e.  frame -1 = ECARGS_4, frame -2 = ECARGS_5, -3 = ECARGS_6, 4 =
** ECARGS_4, 5 = ECARGS_5 6 = ECARGS_6)
**
** @param [r] trnObj [const AjPTrn] Translation tables
** @param [r] seq [const AjPSeq] sequence string to translate
** @param [r] frame [ajint] frame to translate in
**
** @return [AjPSeq] returned peptide translation
** @category use [AjPTrn] Translating a sequence from a AjPSeq in a
**                frame and returns a new peptide
** @@
******************************************************************************/

AjPSeq ajTrnSeqFramePep(const AjPTrn trnObj, const AjPSeq seq, ajint frame)
{
    AjPSeq pep = NULL;
    AjPStr trn = NULL;

    pep = ajTrnNewPep(seq, frame);
    trn = ajStrNew();

    ajTrnSeqFrame(trnObj, seq, frame, &trn);
    ajSeqAssignSeqS(pep, trn);

    ajStrDel(&trn);

    return pep;
}




/* @func ajTrnCDangle *********************************************************
**
** Translates the last 1 or two bases of a sequence in a char *
** that would not be translated if just translating complete codons
** in the specified frame.
** The translation is APPENDED to the input peptide.
**
** @param [r] trnObj [const AjPTrn] Translation tables
** @param [r] seq [const char *] sequence string to translate
** @param [r] len [ajint] sequence string length
** @param [r] frame [ajint] frame to translate in
** @param [u] pep [AjPStr *] returned peptide translation (APPENDED TO INPUT)
**
** @return [ajint] Number of dangling bases (0,1 or 2)
** @category use [AjPTrn] Translates the last 1 or two bases of a
**                sequence in a char* text
** @@
******************************************************************************/

ajint ajTrnCDangle(const AjPTrn trnObj, const char *seq, ajint len,
		   ajint frame,
		   AjPStr *pep)
{
    ajint end = 0; 	          /* end base of last complete forward codon */
    ajint dangle;		  /* number of bases at the end              */

    if(frame > 3)			/* convert frames 4,5,6 to -1,-2,-3 */
	frame = -frame + 3;

    if(frame > 0)
    {					/* forward 3 frames */
	end = frame + ((len-frame+1)/3)*3 - 1;
	dangle = len - end;
    }
    else if(frame <= -4)		/* alternative reverse frames */
	dangle = (len+frame+4)%3;
    else				/* standard reverse frames */
	dangle = -frame-1;

    /* translate any dangling pair of bases at the end */
    if(dangle == 2)
    {
	if(frame >= 1 && frame <= 3)
	    ajStrAppendK(pep, trnObj->GC[trnconv[(ajint)seq[end]]]
		                     [trnconv[(ajint)seq[end+1]]]
		                     [trnconv[0]]);
	else	/* reverse sense */
	    ajStrAppendK(pep, trnObj->GC[trncomp[(ajint)seq[1]]]
		                     [trncomp[(ajint)seq[0]]]
		                     [trncomp[0]]);
    }
    else if(dangle == 1) /* Make up single base translation */
	ajStrAppendK(pep, 'X');

    return dangle;
}




/* @func ajTrnStrDangle *******************************************************
**
** Translates the last 1 or two bases of a sequence in a AjStr
** that would not be translated if just translating complete codons
** in the specified frame.
** The translation is APPENDED to the input peptide.
**
** @param [r] trnObj [const AjPTrn] Translation tables
** @param [r] seq [const AjPStr] sequence string to translate
** @param [r] frame [ajint] frame to translate in
** @param [u] pep [AjPStr *] returned peptide translation (APPENDED TO INPUT)
**
** @return [ajint] Number of dangling bases (0,1 or 2)
**	dangle = len - end;
** @category use [AjPTrn] Translates the last 1 or two bases of a
**                sequence in a AjStr
** @@
******************************************************************************/

ajint ajTrnStrDangle(const AjPTrn trnObj, const AjPStr seq, ajint frame,
		     AjPStr *pep)
{
  return ajTrnCDangle(trnObj, ajStrGetPtr(seq), ajStrGetLen(seq), frame, pep);
}




/* @func ajTrnSeqOrig *********************************************************
**
** Translates a sequence.
**
** The frame to translate is in the range -3 to 6.
** Frames 1 to 3 give normal forward translations.
**
** Frames -3 to -1 give translations in the reverse sense.
** Frame -1 is defined as the translation of the reverse complement
** sequence which matches the codons used in frame 1.  ie.  in the sequence
** ACGT, the first codon of frame 1 is ACG and the last codon of frame -1
** is the reverse complement of ACG (ie.  CGT).
**
** Frames -4 to -6 give translations in the reverse sense.
** Frame -4 is defined as the translation of the reverse complement,
** starting the translation in the first codon of the reversed sequence.
** ie.  in the sequence ACGT, the last codon is CGT and so frame -4
** translates from the reverse complement of CGT (ie.  ACG) - this is
** for those people who define frame -1 as using the first codon when the
** sequence is reverse-complemented.
**
** Frames 4 to 6 rev-comp the DNA sequence then reverse the peptide sequence
**
** Frames 4 to 6 are therefore a reversed protein sequence useful mainly for
**  displaying beneath the original DNA sequence.
**
** NB.  that the naming of the output sequence is always to take
** the name of the input sequence (eg.  ECARGS) and to append an underscore
** character and the frame number 1 to 3 for forward frames and 4 to 6 for
** reverse frames regardless of the final orientation of the reverse
** frames.  (i.e.  frame -1 = ECARGS_4, frame -2 = ECARGS_5, -3 = ECARGS_6, 4 =
** ECARGS_4, 5 = ECARGS_5 6 = ECARGS_6)
**
** @param [r] trnObj [const AjPTrn] Translation tables
** @param [r] seq [const AjPSeq] sequence to translate
** @param [r] frame [ajint] frame to translate in (-6 to 6)
**
** @return [AjPSeq] Peptide translation
** @category use [AjPTrn] Translating a sequence
** @category new [AjPSeq] Translating a sequence
** @@
******************************************************************************/

AjPSeq ajTrnSeqOrig(const AjPTrn trnObj, const AjPSeq seq, ajint frame)
{
    AjPSeq pep = NULL;
    AjPStr trn = NULL;

    pep = ajTrnNewPep(seq, frame);
    trn = ajStrNew();

    ajTrnSeqFrame(trnObj, seq, frame, &trn);

    /*
    ** if there are any dangling bases, then attempt to
    ** translate them
    */
    ajTrnStrDangle(trnObj, ajSeqGetSeqS(seq), frame, &trn);

    /*
    ** if frame is 4, 5 or 6 then reverse the peptide for displaying beneath
    ** the original DNA sequence
    */
    if(frame > 3)
	ajStrReverse(&trn);

    ajSeqAssignSeqS(pep, trn);

    ajStrDel(&trn);

    return pep;
}




/* @func ajTrnGetTitle ********************************************************
**
** Returns the translation table description.
** Because this is a pointer to the real internal string
** the caller must take care not to change the character string in any way.
** If the string is to be changed (case for example) then it must first
** be copied.
**
** @param [r] thys [const AjPTrn] Translation object.
** @return [AjPStr] Description as a string.
** @category cast [AjPTrn] Returns description of the translation
**                table
** @@
******************************************************************************/

AjPStr ajTrnGetTitle(const AjPTrn thys)
{
  return thys->Title;
}




/* @func ajTrnGetFileName *****************************************************
**
** Returns the file that the translation table was read from.
** Because this is a pointer to the real internal string
** the caller must take care not to change the character string in any way.
** If the string is to be changed (case for example) then it must first
** be copied.
**
** @param [r] thys [const AjPTrn] Translation object.
** @return [AjPStr] File name as a string.
** @category cast [AjPTrn] Returns file name the translation table
**                was read from
** @@
******************************************************************************/

AjPStr ajTrnGetFileName(const AjPTrn thys)
{
  return thys->FileName;
}




/* @func ajTrnStartStop *******************************************************
**
** Checks whether the input codon is a Start codon, a Stop codon or
** something else
**
** @param [r] trnObj [const AjPTrn] Translation tables
** @param [r] codon [const AjPStr] codon to check
** @param [w] aa [char *] returned translated amino acid
**                        (not a NULL-terminated array of char)
** @return [ajint] 1 if it is a start codon, -1 if it is a stop codon, else 0
** @category use [AjPTrn] Checks whether the input codon is a Start
**                codon, a Stop codon or something else
** @@
******************************************************************************/

ajint ajTrnStartStop(const AjPTrn trnObj, const AjPStr codon, char *aa)
{
    const char *res;

    ajint tc1;
    ajint tc2;
    ajint tc3;

    res = ajStrGetPtr(codon);

    tc1 = trnconv[(ajint)res[0]];
    tc2 = trnconv[(ajint)res[1]];
    tc3 = trnconv[(ajint)res[2]];

    *aa = trnObj->GC[tc1][tc2][tc3];

    if(trnObj->Starts[tc1][tc2][tc3] == 'M')
	return 1;

    if(*aa == '*')
	return -1;

    return 0;
}




/* @func ajTrnStartStopC ******************************************************
**
** Checks whether a const char * codon is a Start codon, a Stop codon or
** something else
**
** @param [r] trnObj [const AjPTrn] Translation tables
** @param [r] codon [const char *] codon to translate
**                           (these 3 characters need not be NULL-terminated)
** @param [w] aa [char *] returned translated amino acid
**                        (not a NULL-terminated array of char)
** @return [ajint] 1 if it is a start codon, -1 if it is a stop codon, else 0
** @category use [AjPTrn] Checks whether a const char* codon is a
**                Start codon, a Stop codon or something else
** @@
******************************************************************************/

ajint ajTrnStartStopC(const AjPTrn trnObj, const char *codon, char *aa)
{
    ajint tc1;
    ajint tc2;
    ajint tc3;

    tc1 = trnconv[(ajint)codon[0]];
    tc2 = trnconv[(ajint)codon[1]];
    tc3 = trnconv[(ajint)codon[2]];


    *aa = trnObj->GC[tc1][tc2][tc3];

    if(trnObj->Starts[tc1][tc2][tc3] == 'M')
	return 1;

    if(*aa == '*')
	return -1;

    return 0;
}




/* @func ajTrnName ************************************************************
**
** Checks whether a const char * codon is a Start codon, a Stop codon or
** something else
**
** @param [r] trnFileNameInt [ajint] translation table file name number
** @return [const AjPStr] Genetic code description
** @@
******************************************************************************/

const AjPStr ajTrnName(ajint trnFileNameInt)
{
    const AjPStr ret = NULL;
    AjPStr unknown = NULL;
    AjPFile indexf = NULL;
    AjPStr indexfname = NULL;
    AjPStr line = NULL;
    AjPStr tmpstr = NULL;
    AjPStr tok1 = NULL;
    AjPStr tok2 = NULL;
    AjPStrTok handle = NULL;

    if(!unknown)
	unknown = ajStrNewC("unknown");

    if(!trnCodes)
    {
	if(!indexfname)
	    indexfname = ajStrNewC("EGC.index");

	trnCodes = ajTablestrNewLen(20);

	indexf = ajDatafileNewInNameS(indexfname);

	if(!indexf)
	    return unknown;

	while(ajReadlineTrim(indexf, &line))
	{
	    ajStrTrimWhite(&line);

	    if(ajStrGetCharFirst(line) == '#')
		continue;

	    ajStrTokenAssignC(&handle, line, " ");
	    ajStrTokenNextParse(&handle, &tok1);
	    ajStrTokenRestParse(&handle, &tok2);
	    ajTablePut(trnCodes, tok1, tok2);
	    tok1 = NULL;
	    tok2 = NULL;
	}

	ajFileClose(&indexf);
    }

    ajFmtPrintS(&tmpstr, "%d", trnFileNameInt);
    ret = (AjPStr) ajTableFetch(trnCodes, tmpstr);

    ajStrDel(&unknown);
    ajStrDel(&indexfname);
    ajStrDel(&tok1);
    ajStrDel(&tok2);
    ajStrDel(&line);
    ajStrDel(&tmpstr);
    ajStrTokenDel(&handle);

    if(ret)
	return ret;

    return unknown;
}




/* @funcstatic trnComplete ****************************************************
**
** Completes a translation table.
**
** Checks all basic codons are defined
**
** Sets third base wobble ambiguity codes
**
** Sets most ambiguous codon for each amino acid
**
** @param [u] thys [AjPTrn] Translation table
** @return [AjBool] ajTrue if table was valid and could be set
******************************************************************************/

static AjBool trnComplete(AjPTrn thys)
{
    ajint i;
    ajint j;
    ajint k;
    ajint jj;
    ajint kk;
    ajint ifirst = 0;
    ajint jfirst = 0;
    ajint kfirst = 0;
    ajint newkfirst;
    const char* bases = "ACGT";
    char newaa[] = "X";
    ajint codonval[4];
    char aa;
    ajint code[4] = {1, 2, 4, 8};
    ajint ambigcodes[] = { 1,  2,  4,  8,  3,  5,  6,  9,
			  10, 12,  7, 11, 13, 14, 15};
    ajint trncodes[] = {14,  0,  1,  4,  2,  5,  7, 10,
			 3,  6,  8, 11,  9, 12, 13, 14};
    char aalist[65] = "";

    for(i=0;i<4;i++)
    {
	for(j=0;j<4;j++)
	{
	    for(k=0;k<4;k++)
	    {
		if(!thys->GC[i][j][k])
		{
		    ajWarn("Incomplete genetic code definition: "
			   "no codon for '%c%c%c",
			   bases[i], bases[j], bases[k]);

		    return ajFalse;
		}
	    }

	    kfirst = 0;

	    while(kfirst < 4)
	    {
		codonval[2] = 0;
		aa = thys->GC[i][j][kfirst];
		newkfirst = 4;

		for(k=kfirst;k<4;k++)
		{
		    if(thys->GC[i][j][k] == aa)
		    {
			codonval[2] |= code[k];
		    }
		    else
		    {
			if(newkfirst == 4)
			    newkfirst = k;
		    }
		}

		for(k=4;k<15;k++)
		{
		    kk = ambigcodes[k];

		    if(((kk|codonval[2]) == codonval[2]) &&
		       (kk & codonval[2]))
			thys->GC[i][j][k] = aa;
		}

		kfirst = newkfirst;
	    }
	}
    }

    newaa[0] = 'X';
    ifirst = jfirst = kfirst = 0;
    jj = kk = 0;

    while(newaa[0])
    {
	newaa[0] = '\0';
	aa = thys->GC[ifirst][jfirst][kfirst];
	codonval[0] = codonval[1] = codonval[2] = 0;

	for(i=ifirst;i<4;i++)
	{
	    for(j=jfirst;j<4;j++)
	    {
		for(k=kfirst;k<4;k++)
		{
		    if(thys->GC[i][j][k] == aa)
		    {
			codonval[0] |= code[i];
			codonval[1] |= code[j];
			codonval[2] |= code[k];
		    }
		    else
		    {
			if(!newaa[0])
			{
			    if(!strchr(aalist, thys->GC[i][j][k]))
			    {
				newaa[0] = thys->GC[i][j][k];
				ifirst = i;
				jj = j;
				kk = k;
				strcat(aalist, newaa);
			    }
			}
		    }
		}

		kfirst = 0;
	    }

	    jfirst = 0;
	}

	jfirst = jj;
	kfirst = kk;
	thys->GC[trncodes[codonval[0]]]
	    [trncodes[codonval[1]]]
		[trncodes[codonval[2]]] = aa;
    }

    return ajTrue;
}




/* @func ajTrnExit ************************************************************
**
** Cleans up translation processing internal memory
**
** @return [void]
** @@
******************************************************************************/

void ajTrnExit(void)
{
    ajStrDel(&trnResidueStr);
    ajTablestrFree(&trnCodes);

    return;
}
