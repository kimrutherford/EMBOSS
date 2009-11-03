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

AjIUB aj_base_iubS[256];	/* Base letters and their alternatives */
ajint aj_base_table[256];	/* Base letter numerical codes         */
float aj_base_prob[32][32];     /* Assym base probability matches      */

const char* iubbases = "XACMGRSVTWYHKDBN";

static const char *BaseAaTable[]=
{
    "ALA","ASX","CYS","ASP","GLU","PHE","GLY","HIS",
    "ILE","---","LYS","LEU","MET","ASN","---","PRO",
    "GLN","ARG","SER","THR","---","VAL","TRP","XAA",
    "TYR","GLX" 
};


AjBool aj_base_I = 0;



/* @func ajBaseCodes *******************************************************
**
** Returns a string of matching base codes
**
** @param [r] ibase [ajint] Original base code
**
** @return [const AjPStr] Base codes
******************************************************************************/

const AjPStr ajBaseCodes(ajint ibase)
{
    if(!aj_base_I)
	ajBaseInit();

    return  aj_base_iubS[ibase].list;
}




/* @func ajAZToInt ************************************************************
**
** Returns A=0 to Z=25  or 27 otherwise
**
** @param  [r] c [ajint] character to convert
**
** @return [ajint] A=0 to Z=25 or 27 if unknown
** @@
******************************************************************************/

ajint ajAZToInt(ajint c)
{
    ajint ic = toupper(c);

    if(ic >= (ajint) 'A' && ic <= (ajint) 'Z')
	return(ic - (ajint)'A');

    return 27;
}




/* @func ajIntToAZ ************************************************************
**
** Returns 'A' for 0 to  'Z' for 25
**
** @param  [r] n [ajint] character to convert
**
** @return [ajint] 0 as 'A' up to  25 as 'Z'
** @@
******************************************************************************/

ajint ajIntToAZ(ajint n)
{
    if(n>25)
	return (ajint) '*';
    if(n<0)
	return (ajint) '*';

    return(n+(ajint)'A');
}




/* @func ajBinToAZ ************************************************************
**
** Converts a binary OR'd representation of an IUB base where A=1, C=2,
** G=4 and T=8 into an ambiguous DNA base code (uses T rather than U).
**
** Uses the base table set up by ajBaseInit
**
** @param  [r] c [ajint] character to convert
**
** @return [char] Ambiguous DNA base code
******************************************************************************/

char ajBinToAZ(ajint c)
{
    if(c<0)
	return 'N';
    if(c>15)
	return 'N';

    return (iubbases[c]);
}




/* @func ajAZToBin ************************************************************
**
** Returns a binary OR'd representation of an IUB base where A=1, C=2,
** G=4 and T=8
** Uses the base table set up by ajBaseInit
**
** @param  [r] c [ajint] character to convert
**
** @return [ajint] Binary OR'd representation
******************************************************************************/

ajint ajAZToBin(ajint c)
{
    if(!aj_base_I)
	ajBaseInit();

    return (aj_base_table[toupper(c)]);
}




/* @func ajAZToBinC ***********************************************************
**
** Returns a binary OR'd representation of an IUB base where A=1, C=2,
** G=4 and T=8
** Uses the base table set up by ajBaseInit
**
** @param  [r] c [char] character to convert
**
** @return [char] Binary OR'd representation
******************************************************************************/

char ajAZToBinC(char c)
{
    if(!aj_base_I)
	ajBaseInit();

    return ajSysCastItoc(aj_base_table[toupper((ajint) c)]);
}




/* @func ajBaseInit ***********************************************************
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

AjBool ajBaseInit(void)
{
    AjPFile bfptr  = NULL;
    AjPStr  bfname = NULL;
    AjPStr  line   = NULL;
    AjPStr  code   = NULL;
    AjPStr  list   = NULL;

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
	aj_base_table[i] = 0;
    }

    code = ajStrNew();
    list = ajStrNew();
    ajStrAssignC(&code,"");
    ajStrAssignC(&list,"ACGT");


    bfname = ajStrNew();
    ajStrAssignC(&bfname,IUBFILE);
    ajFileDataNew(bfname, &bfptr);
    if(!bfptr) ajFatal("Ebases.iub file not found\n");


    line = ajStrNew();


    while(ajFileGets(bfptr, &line))
    {
	p = ajStrGetPtr(line);
	if(*p=='#' || *p=='!' || *p=='\n')
	    continue;
	p = ajSysFuncStrtok(p," \t\r");
	ajStrAssignC(&code,p);
	p=ajSysFuncStrtok(NULL," \t\r");
	if(sscanf(p,"%d",&n)!=1)
	    ajFatal("Bad format IUB file");
	p = ajSysFuncStrtok(NULL," \t\r");
	ajStrAssignC(&list,p);
	qc = (ajint) ajStrGetCharFirst(code);
	ajStrAssignS(&aj_base_iubS[toupper(qc)].code,code);
	ajStrAssignS(&aj_base_iubS[toupper(qc)].list,list);
	ajStrAssignS(&aj_base_iubS[tolower(qc)].code,code);
	ajStrAssignS(&aj_base_iubS[tolower(qc)].list,list);
	aj_base_table[toupper(qc)] = n;
	aj_base_table[tolower(qc)] = n;
    }

    ajStrDel(&code);
    ajStrDel(&list);
    ajStrDel(&line);
    ajStrDel(&bfname);

    ajFileClose(&bfptr);


    for(i=0;i<32;++i)
    {
	x = ajIntToAZ(i);
	for(j=0;j<32;++j)
	{
	    y = ajIntToAZ(j);
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




/* @func ajBaseAa1ToAa3 *******************************************************
**
** Writes an AjPStr with an amino acid 3 letter code
**
** @param [r] aa1 [char]    Single letter identifier of amino acid
** @param [w] aa3  [AjPStr *] AjPStr object
**
** @return [AjBool] True on succcess
** @@
******************************************************************************/
AjBool  ajBaseAa1ToAa3(char aa1, AjPStr *aa3)
{
    ajint idx;

     if((idx=ajAZToInt(aa1))>25)
	return ajFalse;

    ajStrAssignC(aa3, BaseAaTable[idx]);
    return ajTrue;
}


/* @func ajBaseAa3ToAa1 *******************************************************
**
** Takes a 3 character amino acid code and writes a char with the 
** corresponding single letter code.
** 
** @param [w] aa1 [char *]   Single letter identifier of amino acid
** @param [r] aa3 [const AjPStr]   AjPStr object (3 letter code)
**
** @return [AjBool] True on success
** @@
****************************************************************************/

AjBool  ajBaseAa3ToAa1(char *aa1, const AjPStr aa3)
{
    ajint i;
    
    for(i=0; i<26; i++)
	if(!ajStrCmpC(aa3, BaseAaTable[i]))
	{
	    *aa1 = (char) (i + (int) 'A');
	    return ajTrue;
	}
    
    if(!ajStrCmpC(aa3, "UNK"))
    {
	*aa1 = 'X';

	return ajTrue;
    }	
    
    *aa1='X';
    return ajFalse;
}



/* @func ajBaseProb **********************************************************
**
** Returns an element of the base match probability array
**
** @param [r] base1 [ajint] First base offset
** @param [r] base2 [ajint] Second base offset
**
** @return [float] Base probability value
** @@
******************************************************************************/
float  ajBaseProb(ajint base1, ajint base2)
{
    ajint b1;
    ajint b2;

    b1 = base1;
    b2 = base2;

    if(b1<0)b1=0;
    if(b1>31)b1=31;
    if(b2<0)b2=0;
    if(b2>31)b2=31;

    return aj_base_prob[b1][b2];
}





/* @func ajBaseComp ********************************************************
**
** Complements a nucleotide base.
**
** @param [r] base [char] Base character.
** @return [char] Complementary base.
** @@
******************************************************************************/

char ajBaseComp(char base)
{
    static char fwd[]="ACGTURYWSMKBDHVNXacgturywsmkbdhvnx";
    static char rev[]="TGCAAYRWSKMVHDBNXtgcaayrwskmvhdbnx";
    char *cp;
    char *cq;

    cp = strchr(fwd,base);
    if(cp)
    {
	cq = cp - fwd + rev;
	return *cq;
    }

    return base;
}


/* @obsolete ajSeqBaseComp
** @rename ajBaseComp
*/
__deprecated char  ajSeqBaseComp(char base)
{
    return ajBaseComp(base);
}

/* @func ajBaseExit ***********************************************************
**
** Cleans up sequence base processing internal memory
**
** @return [void]
** @@
******************************************************************************/

void ajBaseExit(void)
{
    ajint i;

    for(i=0;i<256;++i)
    {
	ajStrDel(&aj_base_iubS[i].code);
	ajStrDel(&aj_base_iubS[i].list);
    }
    
    return;
}
