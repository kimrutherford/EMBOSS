/* @source helixturnhelix application
**
** Reports nucleic acid binding domains
** @author Copyright (C) Alan Bleasby (ableasby@hgmp.mrc.ac.uk)
** @@
**
** Original program "HELIXTURNHELIX" by Peter Rice (EGCG 1990)
** This program uses the method of Dodd and Egan (1987) J. Mol. Biol.
** 194:557-564 to determine the significance of possible helix-turn-helix
** matches in protein sequences
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




#define HTHFILE "Ehth.dat"
#define HTH87FILE "Ehth87.dat"




typedef struct DNAB DNAB;
struct DNAB
{
    AjPStr name;
    AjPStr seq;
    ajint pos;
    float sd;
    ajint wt;
    char Padding[4];
};



static ajint helixturnhelix_readNab(AjPInt2d *matrix,AjBool eightyseven);
static void helixturnhelix_print_hits(AjPList ajb,
				      ajint n, ajint lastcol,
				      AjBool eightyseven, AjPFile outf);
static void helixturnhelix_report_hits(AjPList ajb, ajint lastcol,
				       AjPFeattable TabRpt);




/* @prog helixturnhelix *******************************************************
**
** Report nucleic acid binding motifs
**
******************************************************************************/

int main(int argc, char **argv)
{
    AjPSeqall seqall;
    AjPSeq    seq  = NULL;
    AjPFile   outf = NULL;
    AjPReport report = NULL;
    AjPList   ajb = NULL;
    AjPStr    strand = NULL;
    AjPStr    substr = NULL;
    AjBool    eightyseven = ajFalse;
    float     mean;
    float     sd;
    float     minsd;
    static DNAB *lp;

    AjPInt2d matrix = NULL;
    AjPStr tmpStr   = NULL;
    AjPFeattable TabRpt = NULL;

    ajint begin;
    ajint end;
    ajint len;

    const char *p;
    char *q;

    ajint i;
    ajint j;
    ajint cols;
    ajint lastcol;

    ajint n = 0;

    ajint sp;
    ajint se;
    ajint weight;

    float minscore;
    float thissd;

    embInit("helixturnhelix",argc,argv);

    seqall = ajAcdGetSeqall("sequence");
    report = ajAcdGetReport("outfile");
    mean   = ajAcdGetFloat("mean");
    sd     = ajAcdGetFloat("sd");
    minsd  = ajAcdGetFloat("minsd");

    /* obsolete. Can be uncommented in acd file and here to reuse */

    /* outf      = ajAcdGetOutfile("originalfile"); */

    substr = ajStrNew();
    matrix = ajInt2dNew();

    eightyseven = ajAcdGetBoolean("eightyseven");

    cols = helixturnhelix_readNab(&matrix,eightyseven);
    ajDebug("cols = %d\n",cols);

    lastcol = cols-3;

    minscore = mean + (minsd*sd);

    ajb=ajListNew();

    ajFmtPrintAppS(&tmpStr,"Hits above +%.2f SD (%.2f)", minsd, minscore);
    ajReportSetHeader(report, tmpStr);

    while(ajSeqallNext(seqall, &seq))
    {
	n = 0;
	begin = ajSeqallGetseqBegin(seqall);
	end   = ajSeqallGetseqEnd(seqall);


	strand = ajSeqGetSeqCopyS(seq);
	ajStrFmtUpper(&strand);

	ajStrAssignSubC(&substr,ajStrGetPtr(strand),begin-1,end-1);
	len = ajStrGetLen(substr);

	TabRpt = ajFeattableNewSeq(seq);

	q = ajStrGetuniquePtr(&substr);
	for(i=0;i<len;++i,++q)
	    *q = (char) ajBasecodeToInt(*q);

	p = ajStrGetPtr(substr);

	se = (len-lastcol)+1;
	for(i=0;i<se;++i)
	{
	    weight = 0;
	    for(j=0;j<lastcol;++j)
		weight+=ajInt2dGet(matrix,(ajint)*(p+i+j),j);
	    thissd=((float)weight-mean)/sd;
	    if(thissd>minsd)
	    {
		AJNEW(lp);
		lp->name = ajStrNewC(ajSeqGetNameC(seq));
		lp->seq  = ajStrNew();
		sp = begin - 1 + i;
		lp->pos = sp+1;
		ajStrAssignSubC(&lp->seq,ajStrGetPtr(strand),sp,sp+lastcol-1);
		lp->sd = thissd;
		lp->wt = weight;
		ajListPush(ajb,(void *)lp);
		++n;
	    }
	}
	helixturnhelix_report_hits(ajb, lastcol, TabRpt);

	ajReportWrite(report, TabRpt, seq);
	ajFeattableDel(&TabRpt);
	ajStrDel(&strand);
    }


    if(!n)
    {
	if(outf)
	    ajFmtPrintF(outf,"\nNo hits above +%.2f SD (%.2f)\n",
			minsd,minscore);
    }
    else
        if(outf)
	{
	    ajFmtPrintF(outf, "\nHELIXTURNHELIX: Nucleic Acid Binding "
			"Domain search\n\n");
	    ajFmtPrintF(outf,"\nHits above +%.2f SD (%.2f)\n",minsd,minscore);
	    helixturnhelix_print_hits(ajb, n, lastcol,
				      eightyseven, outf);
	}
    
    ajInt2dDel(&matrix);
    
    ajSeqDel(&seq);
    ajStrDel(&substr);
    ajListFree(&ajb);
    
    if(outf)
	ajFileClose(&outf);
    
    ajReportClose(report);

    ajReportDel(&report);
    ajSeqallDel(&seqall);
    ajFeattableDel(&TabRpt);
    ajStrDel(&strand);
    ajStrDel(&tmpStr);

    embExit();

    return 0;
}




/* @funcstatic helixturnhelix_readNab *****************************************
**
** Undocumented.
**
** @param [w] matrix [AjPInt2d*] Undocumented
** @param [r] eightyseven [AjBool] Undocumented
** @return [ajint] Undocumented
** @@
******************************************************************************/


static ajint helixturnhelix_readNab(AjPInt2d *matrix,AjBool eightyseven)
{
    AjPFile mfptr = NULL;
    AjPStr  line  = NULL;
    AjPStr  delim = NULL;
    AjBool  pass;

    const char *p;
    const char *q;

    ajint xcols = 0;
    ajint cols  = 0;

    float sample;
    float expected;
    float pee;
    float exptot;
    ajint rt;

    ajuint i;
    ajuint j;
    ajuint maxi;
    ajuint maxj;
    ajint c = 0;
    ajint v;

    ajuint d1;
    ajuint d2;

    ajint **mat;

    if(eightyseven)
	mfptr = ajDatafileNewInNameC(HTH87FILE);
    else
	mfptr = ajDatafileNewInNameC(HTHFILE);
    if(!mfptr)
	ajFatal("HTH file not found\n");

    line  = ajStrNew();
    delim = ajStrNewC(" :\t\n");

    pass = ajTrue;

    while(ajReadline(mfptr, &line))
    {
	p = ajStrGetPtr(line);

	if(*p=='#' || *p=='!' || *p=='\n')
	    continue;

	if(ajStrPrefixC(line,"Sample:"))
	{
	    if(sscanf(p,"%*s%f",&sample)!=1)
		ajFatal("No sample size given");
	    continue;
	}

	while((*p!='\n') && (*p<'A' || *p>'Z'))
	    ++p;

	cols = ajStrParseCountC(line,ajStrGetPtr(delim));

	if(pass)
	{
	    pass  = ajFalse;
	    xcols = cols;
	}
	else
	    if(xcols!=cols)
		ajFatal("Assymetric table");

	d1 = ajBasecodeToInt((char)toupper((ajint)*p));

	q = ajStrGetPtr(line);
	c = 0;
	q = ajSysFuncStrtok(q,ajStrGetPtr(delim));

	while((q=ajSysFuncStrtok(NULL,ajStrGetPtr(delim))))
	{
	    sscanf(q,"%d",&v);
	    ajInt2dPut(matrix,d1,c++,v);
	}

	if(c>1)
	    maxi = c-2;
	else
	    maxi=0;
	for(i=0,rt=0;i<maxi;++i)
	    rt += ajInt2dGet(*matrix,d1,i);

	if(rt!=ajInt2dGet(*matrix,d1,c-2))
	    ajFatal("Row didn't match total");
    }


    mat = ajInt2dInt(*matrix);
    ajInt2dLen(*matrix,&d1,&d2);


    for(j=0;j<d2-2;++j)
    {
	rt = 0;

	for(i=0;i<d1;++i)
	{
	    if(!mat[i][d2-1])
		continue;
	    rt += mat[i][j];
	}

	if(rt!=(ajint)sample)
	    ajFatal("Column doesn't match sample size");
    }

    exptot = 0.0;
    for(i=0;i<d1;++i)
    {
	if(!mat[i][d2-1])
	    continue;

	expected = (float)mat[i][c-1];
	expected = (float) ((double)expected * 0.0001);
	exptot += expected;

	if(c>1)
	    maxj = c-2;
	else
	    maxj=0;
	for(j=0;j<maxj;++j)
	{
	    if(!mat[i][j])
		pee=((float)1.0<(float)1.0/((sample+(float)1.0)*expected)) ?
		    (float)1.0 : (float)1.0/((sample+(float)1.0)*expected);
	    else
		pee = ((float)mat[i][j])/(sample*expected);
	    mat[i][j]=(ajint)((double)100.0*log(pee));
	}
    }

    if((float)fabs((double)(1.0-exptot)) > 0.05)
	ajFatal("Expected column total != 1.0");

    for(i=0;i<d1;++i)
	for(j=0;j<d2;++j)
	    ajInt2dPut(matrix,i,j,mat[i][j]);

    for(i=0;i<d1;++i)
	AJFREE(mat[i]);
    AJFREE(mat);

    ajStrDel(&line);
    ajStrDel(&delim);
    ajFileClose(&mfptr);

    return cols;
}




/* @funcstatic helixturnhelix_print_hits **************************************
**
** Undocumented.
**
** @param [u] ajb [AjPList] Undocumented
** @param [r] n [ajint] Undocumented
** @param [r] lastcol [ajint] Undocumented
** @param [r] eightyseven [AjBool] Undocumented
** @param [u] outf [AjPFile] Undocumented
** @@
******************************************************************************/


static void helixturnhelix_print_hits(AjPList ajb, ajint n,
				      ajint lastcol,
				      AjBool eightyseven, AjPFile outf)
{
    DNAB     **lp;

    AjPUint hp    = NULL;
    AjPFloat hsd = NULL;

    ajint   i;

    AJCNEW(lp, n);

    hp  = ajUintNew();
    hsd = ajFloatNew();

    for(i=0;i<n;++i)
    {
	if(!ajListPop(ajb,(void **)&lp[i]))
	    ajFatal("List ended prematurely");
	ajUintPut(&hp,i,i);
	ajFloatPut(&hsd,i,lp[i]->sd);
    }
    ajSortFloatDecI(ajFloatFloat(hsd),ajUintUint(hp),n);
    ajFloatDel(&hsd);

    for(i=0;i<n;++i)
    {
	ajFmtPrintF(outf,"\nScore %d (+%.2f SD) in %s at residue %d\n",
		   lp[ajUintGet(hp,i)]->wt,lp[ajUintGet(hp,i)]->sd,
		    ajStrGetPtr(lp[ajUintGet(hp,i)]->name),
		   lp[ajUintGet(hp,i)]->pos);
	ajFmtPrintF(outf,"\n Sequence:  %s\n",
		    ajStrGetPtr(lp[ajUintGet(hp,i)]->seq));

	if(eightyseven)
	{
	    ajFmtPrintF(outf,"            |                  |\n");
	    ajFmtPrintF(outf,"%13d                  %d\n",
			lp[ajUintGet(hp,i)]->pos,
			lp[ajUintGet(hp,i)]->pos+lastcol-1);
	}
	else
	{
	    ajFmtPrintF(outf,"            |                    |\n");
	    ajFmtPrintF(outf,"%13d                    %d\n",
			lp[ajUintGet(hp,i)]->pos,
			lp[ajUintGet(hp,i)]->pos+lastcol-1);
	}
    }


    for(i=0;i<n;++i)
    {
	ajStrDel(&lp[i]->name);
	ajStrDel(&lp[i]->seq);
    }
    AJFREE(lp);
    ajUintDel(&hp);

    return;
}




/* @funcstatic helixturnhelix_report_hits *************************************
**
** Undocumented.
**
** @param [u] ajb [AjPList] List of hits - which are deleted at the end
** @param [r] lastcol [ajint] Undocumented
** @param [u] TabRpt [AjPFeattable] Undocumented
** @return [void]
** @@
******************************************************************************/

static void helixturnhelix_report_hits(AjPList ajb,
				       ajint lastcol, AjPFeattable TabRpt)
{
    DNAB     **lp = NULL;

    AjPUint   hp  = NULL;
    AjPFloat hsd = NULL;

    ajint n;
    ajint i;
    AjPFeature gf = NULL;

    AjPStr tmpStr = NULL;
    AjPStr fthit = NULL;
    struct DNAB *dnab;

    if(!fthit)
	ajStrAssignC(&fthit, "hit");

    hp  = ajUintNew();
    hsd = ajFloatNew();

    n = ajListToarray(ajb, (void***) &lp);

    if(!n)
    {
	ajUintDel(&hp);
	ajFloatDel(&hsd);
	return;
    }

    for(i=0;i<n;++i)
    {
	ajUintPut(&hp,i,i);
	ajFloatPut(&hsd,i,lp[i]->sd);
    }
    ajSortFloatDecI(ajFloatFloat(hsd),ajUintUint(hp),n);
    ajFloatDel(&hsd);

    for(i=0;i<n;++i)
    {
        gf = ajFeatNewProt(TabRpt, NULL, fthit,
			   lp[ajUintGet(hp,i)]->pos,
			   lp[ajUintGet(hp,i)]->pos+lastcol-1,
			   (float) lp[ajUintGet(hp,i)]->wt);
	ajFmtPrintS(&tmpStr, "*pos %d", lp[ajUintGet(hp,i)]->pos);
	ajFeatTagAdd(gf, NULL, tmpStr);
	ajFmtPrintS(&tmpStr, "*sd %.2f", lp[ajUintGet(hp,i)]->sd);
	ajFeatTagAdd(gf, NULL, tmpStr);

    }

    while(ajListPop(ajb,(void **)&dnab))
    {
	ajStrDel(&dnab->name);
	ajStrDel(&dnab->seq);
	AJFREE(dnab);
    }


    AJFREE(lp);
    ajUintDel(&hp);

    ajStrDel(&fthit);
    ajStrDel(&tmpStr);

    return;
}
