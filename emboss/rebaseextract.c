/* @source rebaseextract application
**
** Extracts restriction enzyme information from REBASE
** @author Copyright (C) Alan Bleasby (ableasby@hgmp.mrc.ac.uk)
** @@
**
** This program is free software; you can redistribute it and/or
** modify it under the terms of the GNU General Public License
** as published by the Free Software Foundation; either version 2
** of the License, or (at your option) any later version.
**
** This program is distributed in the hope that it will be useful,
** but WITHOUT ANY WARRANTY; without even the implied warranty of
** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
** GNU General Public License for more details.
**
** You should have received a copy of the GNU General Public License
** along with this program; if not, write to the Free Software
** Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
******************************************************************************/

#include "emboss.h"
#include <math.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <limits.h>


#define DATANAME  "REBASE/embossre.enz"
#define DATANAME2 "REBASE/embossre.ref"
#define DATANAME3 "REBASE/embossre.sup"
#define DATANAME4 "REBASE/embossre.equ"
#define EQUGUESS  5000
#define SUPGUESS  10000



static void rebaseextract_process_pattern(const AjPStr pattern,
					  const AjPStr code,
					  AjPFile outf, AjBool hassup);
static void rebaseextract_printEnzHeader(AjPFile outf);
static void rebaseextract_printRefHeader(AjPFile outf);
static void rebaseextract_printSuppHeader(AjPFile outf);




/* @prog rebaseextract ********************************************************
**
** Extract data from REBASE
**
******************************************************************************/

int main(int argc, char **argv)
{
    AjPFile inf   = NULL;
    AjPFile infp  = NULL;
    AjPFile outf  = NULL;
    AjPFile outf2 = NULL;
    AjPFile outf3 = NULL;

    AjPStr  line;
    AjPStr  code;
    AjPStr  pattern;
    AjPStr  isoschiz;
    AjPStr  meth;
    AjPStr  tit;
    AjPStr  sou;
    AjPStr  comm;
    AjPStr  pfname;
    AjBool  isrefm = ajFalse;
    AjBool  isref  = ajFalse;
    AjBool  hassup;

    ajint  count;
    ajlong pos;
    ajint  i;

    AjBool    doequ;
    AjPFile   oute    = NULL;
    AjPStr    isostr  = NULL;
    AjPTable  ptable  = NULL;
    AjPStr    key     = NULL;
    AjPStr    value   = NULL;
    AjPStrTok handle  = NULL;
    AjPStr    token   = NULL;
    AjPStr    line2   = NULL;
    AjPList   equ1    = NULL;
    AjPList   equ2    = NULL;
    AjPStr    equstr1 = NULL;
    AjPStr    equstr2 = NULL;
    AjPStr    hstr1   = NULL;
    AjPStr    hstr2   = NULL;
    const AjPStr htest = NULL;
    
    AjBool isproto = ajFalse;
    char c;
    const char *sptr = NULL;

    AjPTable hassuptable = NULL;
    
    
    embInit("rebaseextract",argc,argv);

    inf   = ajAcdGetInfile("infile");
    infp  = ajAcdGetInfile("protofile");
    doequ = ajAcdGetBoolean("equivalences");

    equ1 = ajListNew();
    equ2 = ajListNew();

    hassuptable = ajTablestrNew(SUPGUESS);
    

    pfname = ajStrNewC(DATANAME);
    outf = ajDatafileNewOutNameS(pfname);
    rebaseextract_printEnzHeader(outf);

    ajStrAssignC(&pfname,DATANAME2);
    outf2 = ajDatafileNewOutNameS(pfname);
    rebaseextract_printRefHeader(outf2);

    ajStrAssignC(&pfname,DATANAME3);
    outf3 = ajDatafileNewOutNameS(pfname);
    rebaseextract_printSuppHeader(outf3);

    if(doequ)
    {
	ajStrAssignC(&pfname,DATANAME4);
	oute = ajDatafileNewOutNameS(pfname);
	ptable = ajTablestrNew(EQUGUESS);
	isostr = ajStrNew();
    }
    ajStrDel(&pfname);

    line     = ajStrNew();
    line2    = ajStrNew();
    code     = ajStrNew();
    pattern  = ajStrNew();
    isoschiz = ajStrNew();
    meth     = ajStrNew();
    tit      = ajStrNew();
    sou      = ajStrNew();
    comm     = ajStrNew();
    token    = ajStrNew();

    /*
     *  Extract Supplier information
     */
    while(ajReadlineTrim(inf,&line))
    {
	if(ajStrFindC(line,"withrefm.")>=0)
	    isrefm = ajTrue;

	if(ajStrFindC(line,"withref.")>=0)
	    isref = ajTrue;

	if(strstr(ajStrGetPtr(line),"REBASE codes"))
	    break;
    }


    while(ajReadlineTrim(infp,&line))
    {
	if(ajStrFindC(line,"proto.")>=0)
	    isproto = ajTrue;

	
	if(strstr(ajStrGetPtr(line),"Rich Roberts"))
	    break;
    }

    if(!isrefm)
    {
	if(isref)
	    ajFatal("WITHREF file specified by mistake. Use WITHREFM instead");
	else
	    ajFatal("Invalid withrefm file");
    }


    if(!isproto)
	ajFatal("Invalid PROTO file specified");



    while(doequ && ajReadlineTrim(infp,&line))
    {
	if(!ajStrGetLen(line))
	    continue;
	sptr = ajStrGetPtr(line);
	c = *sptr;
	if(c>='A' && c<='Z')
	{
	    while(*sptr!=' ')
		++sptr;
	    while(*sptr==' ')
		++sptr;
	    
	    key   = ajStrNew();
	    value = ajStrNewC(sptr);
	    ajStrRemoveWhite(&value);
	    ajFmtScanS(line,"%S",&key);
	    ajTablePut(ptable,(void *)key, (void *)value);
	}
    }
    
    if(!ajReadlineTrim(inf,&line))
	ajFatal("No Supplier Information Found");

    if(!ajReadlineTrim(inf,&line))
	ajFatal("Unexpected EOF");

    while(ajStrGetLen(line))
    {
	ajStrRemoveWhiteExcess(&line);
	ajFmtPrintF(outf3,"%s\n",ajStrGetPtr(line));
	if(!ajReadlineTrim(inf,&line))
	    ajFatal("Unexpected EOF");
    }
    ajFileClose(&outf3);



    while(ajReadlineTrim(inf,&line))
    {
	/* Get RE */
	if(!ajStrPrefixC(line,"<1>"))
	    continue;

	ajStrAssignC(&code,ajStrGetPtr(line)+3);

	/* Get isoschizomers */
	if(!ajReadlineTrim(inf,&line2))
	    ajFatal("Unexpected EOF");

	if(ajStrGetLen(line2)>3)
	    ajStrAssignC(&isoschiz,ajStrGetPtr(line2)+3);
	else
	    ajStrAssignC(&isoschiz,"");


	/* Get recognition sequence */
	if(!ajReadlineTrim(inf,&line))
	    ajFatal("Unexpected EOF");

	if(ajStrGetLen(line)>3)
	{
	    ajStrAssignC(&pattern,ajStrGetPtr(line)+3);

	    if(doequ && ajStrGetLen(line2)>3)
	    {
		ajStrAssignS(&isostr,isoschiz);
		handle = ajStrTokenNewC(isostr,"\t\n>,");
	        ajStrTokenNextParse(&handle, &token);

		if((value=ajTableFetchmodS(ptable, token)))
		    if(ajStrMatchS(value,pattern))
		    {
			equstr1 = ajStrNew();
			equstr2 = ajStrNew();
			ajStrAssignS(&equstr1,code);
			ajStrAssignS(&equstr2, token);
			ajListPushAppend(equ1,(void *) equstr1);
			ajListPushAppend(equ2,(void *) equstr2);
		    }		
		ajStrTokenDel(&handle);
	    }
	}
	else
	    ajStrAssignC(&pattern,"");

	/* Get methylation */
	if(!ajReadlineTrim(inf,&line))
	    ajFatal("Unexpected EOF");

	if(ajStrGetLen(line)>3)
	    ajStrAssignC(&meth,ajStrGetPtr(line)+3);
	else
	    ajStrAssignC(&meth,"");

	/* Get title */
	if(!ajReadlineTrim(inf,&line))
	    ajFatal("Unexpected EOF");

	if(ajStrGetLen(line)>3)
	    ajStrAssignC(&tit,ajStrGetPtr(line)+3);
	else
	    ajStrAssignC(&tit,"");

	/* Get source */
	if(!ajReadlineTrim(inf,&line))
	    ajFatal("Unexpected EOF");

	if(ajStrGetLen(line)>3)
	    ajStrAssignC(&sou,ajStrGetPtr(line)+3);
	else
	    ajStrAssignC(&sou,"");

	/* Get commercial supplier */
	if(!ajReadlineTrim(inf,&line))
	    ajFatal("Unexpected EOF");

	hstr1 = ajStrNew();
	hstr2 = ajStrNew();

	hassup=ajFalse;
	if(ajStrGetLen(line)>3)
	{
	    hassup = ajTrue;
	    ajStrAssignC(&comm,ajStrGetPtr(line)+3);
	    ajStrAssignC(&hstr2,"Y");
	}
	else
	{
	    ajStrAssignC(&comm,"");
	    ajStrAssignC(&hstr2,"N");
	}

	ajStrAssignS(&hstr1,code);
	ajTablePut(hassuptable,(void *)hstr1, (void *)hstr2);

	ajFmtPrintF(outf2,"%s\n",ajStrGetPtr(code));
	ajFmtPrintF(outf2,"%s\n",ajStrGetPtr(tit));
	ajFmtPrintF(outf2,"%s\n",ajStrGetPtr(isoschiz));
	ajFmtPrintF(outf2,"%s\n",ajStrGetPtr(meth));
	ajFmtPrintF(outf2,"%s\n",ajStrGetPtr(sou));
	ajFmtPrintF(outf2,"%s\n",ajStrGetPtr(comm));

	/* Get references */
	count = -1;
	pos = ajFileResetPos(inf);
	while(ajStrGetLen(line))
	{
	    if(!ajReadlineTrim(inf,&line))
		break;
	    ++count;
	}
	ajFileSeek(inf,pos,0);


	if(!ajReadlineTrim(inf,&line))
	    ajFatal("Unexpected EOF");

	if(ajStrGetLen(line)==3)
	    ajFmtPrintF(outf2,"0\n");
	else
	{
	    ajFmtPrintF(outf2,"%d\n%s\n",count,ajStrGetPtr(line)+3);
	    for(i=1;i<count;++i)
	    {
		if(!ajReadlineTrim(inf,&line))
		    ajFatal("Unexpected EOF");
		ajFmtPrintF(outf2,"%s\n",ajStrGetPtr(line));
	    }
	}
	ajFmtPrintF(outf2,"//\n");


	rebaseextract_process_pattern(pattern,code,outf,hassup);

    }


    if(doequ)
    {
	while(ajListPop(equ1,(void **)&equstr1))
	{
	    ajListPop(equ2,(void **)&equstr2);

	    if(!(htest=ajTableFetchS(hassuptable, equstr2)))
		ajFatal("Expected supplier value not found for '%S' (enzyme '%S')",
			equstr1,equstr2);
	    if(ajStrMatchC(htest,"N"))
		ajStrAppendC(&equstr2,"*");

	    ajFmtPrintF(oute,"%S %S\n",equstr1,equstr2);
	    ajStrDel(&equstr1);
	    ajStrDel(&equstr2);
	}

	ajStrDel(&isostr);
	ajFileClose(&oute);
	ajTablestrFree(&ptable);
    }

    ajTablestrFree(&hassuptable);
    ajFileClose(&inf);
    ajFileClose(&infp);
    ajFileClose(&outf);
    ajFileClose(&outf2);


    ajStrDel(&line);
    ajStrDel(&line2);
    ajStrDel(&token);
    ajStrDel(&isoschiz);
    ajStrDel(&tit);
    ajStrDel(&meth);
    ajStrDel(&sou);
    ajStrDel(&comm);
    ajStrDel(&pattern);
    ajStrDel(&code);

    ajListFree(&equ1);
    ajListFree(&equ2);

    embExit();

    return 0;
}




/* @funcstatic rebaseextract_process_pattern **********************************
**
** Convert rebase pattern into emboss pattern
**
** @param [r] pattern [const AjPStr] rebase recognition sequence
** @param [r] code [const AjPStr] re name
** @param [u] outf [AjPFile] outfile
** @param [r] hassup [AjBool] has a supplier
** @@
******************************************************************************/

static void rebaseextract_process_pattern(const AjPStr pattern,
					  const AjPStr code,
					  AjPFile outf,
					  AjBool hassup)
{
    AjPStr temp = NULL;
    AjPStr ppat = NULL;
    AjPStrTok tokens = NULL;
    AjPStr tmppattern = NULL;

    const char *p;
    const char *q;
    char *r;
    const char *t;

    AjBool hascarat;

    ajint cut1;
    ajint cut2;
    ajint cut3;
    ajint cut4;
    ajint len;
    ajint ncuts;
    ajint nc;
    ajint i;
    AjBool blunt = ajFalse;

    tmppattern = ajStrNewS(pattern);
    ajStrFmtUpper(&tmppattern);
    temp = ajStrNew();
    ppat = ajStrNew();

    tokens=ajStrTokenNewC(tmppattern,",");

    while(ajStrTokenNextParseC(&tokens,",",&ppat))
    {
	ajFmtPrintF(outf,"%S\t",code);

	ajStrAssignS(&temp,ppat);

	hascarat = ajFalse;
	p = ajStrGetPtr(ppat);

	if(*p=='?')
	{
	    ajFmtPrintF(outf,"?\t0\t0\t0\t0\t0\t0\t0\n");
	    continue;
	}


	t = p;
	if(*p=='(')
	{
	    sscanf(p+1,"%d/%d",&cut1,&cut2);
	    q=p+1;
	    if(!(q=strchr(q,(ajint)'(')))
		ajFatal("Bad pattern %S in %S",code,pattern);
	    sscanf(q+1,"%d/%d",&cut3,&cut4);
	    cut1 *= -1;
	    cut2 *= -1;
	    --cut1;
	    --cut2;

	    if(!(p=strchr(p,(ajint)')')))
		ajFatal("%S mismatched parentheses",code);

	    p = ajStrGetPtr(ppat);
	    r = ajStrGetuniquePtr(&temp);
	    while(*p)
	    {
		if(*p>='A' && *p<='Z')
		    *r++ = *p;
		++p;
	    }
	    *r = '\0';
	    ajStrAssignC(&ppat,ajStrGetPtr(temp));
	    len=ajStrGetLen(ppat);
	    cut3 += len;
	    cut4 += len;
	    ncuts = 4;

	    if(cut1==cut2 && cut3==cut4)
		blunt = ajTrue;
	    else
		blunt = ajFalse;

	    p = t;
	}
	else
	{
	    ncuts = 2;
	    cut3 = cut4 = 0;
	    if((p=strchr(p,(ajint)'(')))
	    {
		sscanf(p+1,"%d/%d",&cut1,&cut2);
		if(cut1==cut2)
		    blunt = ajTrue;
		else
		    blunt = ajFalse;

		p = ajStrGetPtr(ppat);
		r = ajStrGetuniquePtr(&temp);
		while(*p)
		{
		    if(*p>='A' && *p<='Z')
			*r++ = *p;
		    ++p;
		}
		*r = '\0';
		ajStrAssignC(&ppat,ajStrGetPtr(temp));
		len=ajStrGetLen(ppat);
		cut1 += len;
		cut2 += len;
		if(cut1<=0)
		    --cut1;

		if(cut2<=0)
		    --cut2;
	    }
	    else			/* probably a carat */
	    {
		p = ajStrGetPtr(ppat);
		r = ajStrGetuniquePtr(&temp);
		cut1 = 0;
		hascarat = ajFalse;

		while(*p)
		{
		    if(*p>='A' && *p<='Z')
		    {
			*r++ = *p;
			if(!hascarat)
			    ++cut1;
		    }
		    if(*p=='^')
			hascarat = ajTrue;

		    ++p;
		}
		*r = '\0';
		ajStrAssignC(&ppat,ajStrGetPtr(temp));
		len = ajStrGetLen(ppat);
		if(!hascarat)
		{
		    ncuts = 0;
		    blunt = ajFalse;
		    cut1  = 0;
		    cut2  = 0;
		}
		else
		{
		    if(len==cut1*2)
		    {
			blunt = ajTrue;
			cut2  = cut1;
		    }
		    else if(!cut1)
		    {
			cut1 = -1;
			cut2 = len;
		    }
		    else
		    {
			p = ajStrGetPtr(ppat);
			if(p[cut1-1]=='N' && cut1==len)
			{
			    for(i=cut1-1,nc=0;i>-1;--i)
				if(p[i]=='N')
				    ++nc;
			    cut2 = len-cut1-nc-1;
			}
			else if(cut1==len)
			    cut2 = -1;
			else
			    cut2 = len-cut1;
			blunt = ajFalse;
		    }
		}
	    }
	}

	/* Mark RE's with no suppliers with lc sequence */
	if(!hassup)
	    ajStrFmtLower(&ppat);


	if(ncuts==4)
	    ajFmtPrintF(outf,"%s\t%d\t%d\t%d\t%d\t%d\t%d\t%d\n",
			ajStrGetPtr(ppat),len,ncuts,blunt,cut1,cut2,cut3,cut4);
	else
	    ajFmtPrintF(outf,"%s\t%d\t%d\t%d\t%d\t%d\t0\t0\n",
			ajStrGetPtr(ppat),
			len,ncuts,blunt,cut1,cut2);
    }

    ajStrDel(&tmppattern);
    ajStrDel(&temp);
    ajStrDel(&ppat);
    ajStrTokenDel(&tokens);

    return;
}




/* @funcstatic rebaseextract_printEnzHeader ***********************************
**
** print comments at start of embossre.enz
**
** @param [w] outf [AjPFile] outfile
** @@
******************************************************************************/

static void rebaseextract_printEnzHeader(AjPFile outf)
{
    ajFmtPrintF(outf,"# REBASE enzyme patterns for EMBOSS\n#\n");
    ajFmtPrintF(outf,"# Format:\n");
    ajFmtPrintF(outf,"# name<ws>pattern<ws>len<ws>ncuts<ws>");
    ajFmtPrintF(outf,"blunt<ws>c1<ws>c2<ws>c3<ws>c4\n");
    ajFmtPrintF(outf,"#\n");
    ajFmtPrintF(outf,"# Where:\n");
    ajFmtPrintF(outf,"# name = name of enzyme\n");
    ajFmtPrintF(outf,"# pattern = recognition site\n");
    ajFmtPrintF(outf,"# len = length of pattern\n");
    ajFmtPrintF(outf,"# ncuts = number of cuts made by enzyme\n");
    ajFmtPrintF(outf,"#         Zero represents unknown\n");
    ajFmtPrintF(outf,"# blunt = true if blunt end cut, false if sticky\n");
    ajFmtPrintF(outf,"# c1 = First 5' cut\n");
    ajFmtPrintF(outf,"# c2 = First 3' cut\n");
    ajFmtPrintF(outf,"# c3 = Second 5' cut\n");
    ajFmtPrintF(outf,"# c4 = Second 3' cut\n#\n# Examples:\n");
    ajFmtPrintF(outf,"# AAC^TGG -> 6 2 1 3 3 0 0\n");
    ajFmtPrintF(outf,"# A^ACTGG -> 6 2 0 1 5 0 0\n");
    ajFmtPrintF(outf,"# AACTGG  -> 6 0 0 0 0 0 0\n");
    ajFmtPrintF(outf,"# AACTGG(-5/-1) -> 6 2 0 1 5 0 0\n");
    ajFmtPrintF(outf,"# (8/13)GACNNNNNNTCA(12/7) -> 12 4 0 -9 -14 24 19\n");
    ajFmtPrintF(outf,"#\n");
    ajFmtPrintF(outf,"# i.e. cuts are always to the right of the given\n");
    ajFmtPrintF(outf,"# residue and sequences are always with reference to\n");
    ajFmtPrintF(outf,"# the 5' strand.\n");
    ajFmtPrintF(outf,"# Sequences are numbered ... -3 -2 -1 1 2 3 ... with\n");
    ajFmtPrintF(outf,"# the first residue of the pattern at base number 1.\n");
    ajFmtPrintF(outf,"#\n");

    return;
}




/* @funcstatic rebaseextract_printRefHeader ***********************************
**
** Print header to embossre.ref
**
** @param [w] outf [AjPFile] outfile
** @@
******************************************************************************/

static void rebaseextract_printRefHeader(AjPFile outf)
{
    ajFmtPrintF(outf,"# REBASE enzyme information for EMBOSS\n#\n");
    ajFmtPrintF(outf,"# Format:\n");
    ajFmtPrintF(outf,"# Line 1: Name of Enzyme\n");
    ajFmtPrintF(outf,"# Line 2: Organism\n");
    ajFmtPrintF(outf,"# Line 3: Isoschizomers\n");
    ajFmtPrintF(outf,"# Line 4: Methylation\n");
    ajFmtPrintF(outf,"# Line 5: Source\n");
    ajFmtPrintF(outf,"# Line 6: Suppliers\n");
    ajFmtPrintF(outf,"# Line 7: Number of following references\n");
    ajFmtPrintF(outf,"# Lines 8..n: References\n");
    ajFmtPrintF(outf,"# // (end of entry marker)\n");
    ajFmtPrintF(outf,"#\n");

    return;
}




/* @funcstatic rebaseextract_printSuppHeader **********************************
**
** Print header to embossre.sup
**
** @param [w] outf [AjPFile] outfile
** @@
******************************************************************************/

static void rebaseextract_printSuppHeader(AjPFile outf)
{
    ajFmtPrintF(outf,"# REBASE Supplier information for EMBOSS\n#\n");
    ajFmtPrintF(outf,"# Format:\n");
    ajFmtPrintF(outf,"# Code of Supplier<ws>Supplier name\n");
    ajFmtPrintF(outf,"#\n");

    return;
}
