/* @source antigenic application
**
** Displays antigenic sites in proteins
** @author Copyright (C) Alan Bleasby (ableasby@hgmp.mrc.ac.uk)
** @@
** Original program "ANTIGENIC" by Peter Rice (EGCG 1991)
** Prediction of antigenic regions of protein sequences by method of:
** Kolaskar AS and Tongaonkar PC (1990) FEBS Letters 276:172-174
** "A semi-emipirical method for prediction of antigenic determinants
** on protein antigens"
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


#define DATAFILE "Eantigenic.dat"




static void antigenic_readAnti(AjPFloat *agp);
static void antigenic_padit(AjPFile outf, ajint b, ajint e);
static void antigenic_dumptoFeat(ajint nhits, const AjPUint hp,
				 const AjPUint hpos, const AjPUint hlen,
				 const AjPFloat thisap, const AjPFloat hwt,
				 AjPFeattabOut featout,
				 const char *seqname,ajint begin);




/* @prog antigenic ************************************************************
**
** Finds antigenic sites in proteins
**
******************************************************************************/

int main(int argc, char **argv)
{
    AjPSeqall seqall;
    AjPSeq    seq    = NULL;
    AjPFile   outf   = NULL;
    AjPReport report = NULL;
    AjPStr    strand = NULL;
    AjPStr    sstr   = NULL;
    AjPStr    stmp   = NULL;
    AjPStr    substr = NULL;

    AjPStr    tmpHeadStr = NULL;
    AjPStr    tmpFeatStr = NULL;
    AjPFeattable TabRpt  = NULL;
    AjPFeature gf = NULL;

    AjPStr fthit  = NULL;

    AjPFeattabOut featout = NULL;

    ajuint begin;
    ajuint end;
    ajuint len;
    ajuint start;
    ajuint stop;
    char *p;
    char *q;
    ajint ii;
    ajuint i;
    ajuint j;
    ajint k;
    ajint m;
    ajuint fpos;
    ajuint lpos;
    ajuint maxlen;
    ajuint maxpos;
    ajuint minlen;
    ajuint lenap;
    ajuint istart;
    ajint iend;
    ajint nhits;

    AjPFloat thisap = NULL;
    AjPFloat hwt    = NULL;
    AjPUint   hpos   = NULL;
    AjPUint   hp     = NULL;
    AjPUint   hlen   = NULL;

    float resap;
    float totap;
    float averap;
    float minap;
    float score;
    float v;
    ajint maxscorepos;

    AjPFloat agp = NULL;

    embInit("antigenic", argc, argv);

    thisap = ajFloatNew();
    hwt    = ajFloatNew();
    hpos   = ajUintNew();
    hp     = ajUintNew();
    hlen   = ajUintNew();

    agp = ajFloatNew();

    antigenic_readAnti(&agp);

    seqall    = ajAcdGetSeqall("sequence");
    minlen    = ajAcdGetInt("minlen");
    report    = ajAcdGetReport("outfile");

    substr = ajStrNew();
    sstr   = ajStrNew();
    stmp   = ajStrNew();
    ajStrAssignC(&fthit, "SO:0001018");

    while(ajSeqallNext(seqall, &seq))
    {
	begin = ajSeqallGetseqBegin(seqall);
	end   = ajSeqallGetseqEnd(seqall);
	start = begin-1;
	stop  = end-1;

	TabRpt = ajFeattableNewSeq(seq);
	strand = ajSeqGetSeqCopyS(seq);

	ajStrFmtUpper(&strand);
	ajStrAssignSubC(&substr,ajStrGetPtr(strand),start,stop);
	ajStrAssignSubC(&sstr,ajStrGetPtr(strand),start,stop);
	len  = ajStrGetLen(substr);

	q = p = ajStrGetuniquePtr(&substr);
	for(i=0;i<len;++i,++p)
	    *p = (char) ajBasecodeToInt(*p);

	totap = 0.0;
	fpos  = 0;
	lpos  = len-7;

	for(i=0;i<len;++i)
	    ajFloatPut(&thisap,i,0.0);

	p = q;
	for(i=0;i<len;++i)
	{
	    resap = ajFloatGet(agp,(ajint)*(p+i));
	    totap += resap;
	    if((i>=fpos) && (i<=lpos))
	    {
		ajFloatPut(&thisap,i+3,resap);
		for(j=i+1;j<=i+6;++j)
		    ajFloatPut(&thisap,i+3,ajFloatGet(thisap,i+3) +
			       ajFloatGet(agp,(ajint) *(p+j)));
		ajFloatPut(&thisap,i+3,ajFloatGet(thisap,i+3)/(float)7.0);
	    }
	}

	averap = totap/(float)len;
	minap  = (averap < (float)1.0) ? averap : (float)1.0;
	lenap  = nhits = maxlen = maxpos = 0;

	for(i=fpos+3;i<=lpos+4;++i)
	    if(ajFloatGet(thisap,i) >= minap)
		++lenap;
	    else
	    {
		if(lenap >= minlen)
		{
		    score = 0.0;
		    for(j=i-lenap;j<=i-1;++j)
		    {
			v = ajFloatGet(thisap,j);
			score = (score > v) ? score : v;
		    }
		    ajUintPut(&hp,nhits,nhits);
		    ajUintPut(&hpos,nhits,i-lenap);
		    ajFloatPut(&hwt,nhits,score);
		    ajUintPut(&hlen,nhits++,lenap);
		}

		if(lenap>maxlen)
		{
		    maxlen=lenap;
		    maxpos=i-lenap;
		}
		lenap=0;
	    }

	ajReportSetHeaderS(report, tmpHeadStr);

	if(outf)
	{
	  ajFmtPrintF(outf,"ANTIGENIC of %s  from: %d  to: %d\n\n",
		      ajSeqGetNameC(seq),begin,end);
	  ajFmtPrintF(outf,"Length %d residues, score calc from %d to %d\n",
		      ajSeqGetLen(seq),fpos+3+begin,lpos+3+begin);
	  ajFmtPrintF(outf,"Reporting all peptides over %d residues\n\n",
		      minlen);
	  ajFmtPrintF(outf,
		      "Found %d hits scoring over %.2f (true average %.2f)\n",
		      nhits,minap,averap);
	}

	istart = maxpos;
	iend   = maxpos+maxlen-1;

	if(outf)
	{
	  ajFmtPrintF(outf,"Maximum length %d at residues %d->%d\n\n", maxlen,
		      istart+begin, iend+begin);
	  ajStrAssignSubC(&stmp,ajStrGetPtr(sstr),istart,iend);
	  ajFmtPrintF(outf," Sequence:  %S\n",stmp);
	  ajFmtPrintF(outf,"            |");
	  antigenic_padit(outf,istart,iend);
	  ajFmtPrintF(outf,"|\n");
	  ajFmtPrintF(outf,"%13d",istart+begin);
	  antigenic_padit(outf,istart,iend);
	  ajFmtPrintF(outf,"%d\n",iend+begin);
	}

	if(nhits)
	{
	    ajSortFloatIncI(ajFloatFloat(hwt),ajUintUint(hp),nhits);
	    if(outf)
	      ajFmtPrintF(outf,
			  "\nEntries in score order, max score at \"*\"\n\n");

	    for(ii=nhits-1,j=0;ii>-1;--ii)
	    {
		k = ajUintGet(hp,ii);
		istart = ajUintGet(hpos,k);

		iend = istart + ajUintGet(hlen,k) -1;
		if(outf)
		{
		  ajFmtPrintF(outf,
			      "\n[%d] Score %.3f length %d at "
			      "residues %d->%d\n",
			      ++j,ajFloatGet(hwt,k),ajUintGet(hlen,k),
			      istart+begin,iend+begin);
		  ajFmtPrintF(outf,"            ");
		  for(m=istart;m<=iend;++m)
		    if(ajFloatGet(thisap,m) == ajFloatGet(hwt,k))
		      ajFmtPrintF(outf,"*");
		    else
		      ajFmtPrintF(outf," ");
		  ajFmtPrintF(outf,"\n");
		  ajStrAssignSubC(&stmp,ajStrGetPtr(sstr),istart,iend);
		  ajFmtPrintF(outf," Sequence:  %S\n",stmp);
		  ajFmtPrintF(outf,"            |");
		  antigenic_padit(outf,istart,iend);
		  ajFmtPrintF(outf,"|\n");
		  ajFmtPrintF(outf,"%13d",istart+begin);
		  antigenic_padit(outf,istart,iend);
		  ajFmtPrintF(outf,"%d\n",iend+begin);
		}

		gf = ajFeatNewProt(TabRpt, NULL, fthit,
				   istart+begin, iend+begin,
				   ajFloatGet(hwt,k));

		/* highest scoring position */

		maxscorepos=iend+1+1; /* 1 beyond end as a safe default */

		for(m=istart;m<=iend;++m)
		{
		    if(ajFloatGet(thisap,m) == ajFloatGet(hwt,k))
		    {
			maxscorepos=m+1;
			break;
		    }
		}
		ajFmtPrintS(&tmpFeatStr, "*pos %d", maxscorepos);
		ajFeatTagAdd (gf, NULL, tmpFeatStr);
	    }

	    /* obsolete - use -rf with a feature format to get this output */

	    if (featout)
		antigenic_dumptoFeat(nhits,hp,hpos,hlen,thisap,hwt,featout,
				     ajSeqGetNameC(seq),begin);
	}

	ajReportWrite(report, TabRpt, seq);

	ajStrDel(&strand);
	ajFeattableDel(&TabRpt);
    }

    ajStrDel(&stmp);
    ajStrDel(&sstr);
    ajSeqDel(&seq);
    ajSeqallDel(&seqall);
    ajReportDel(&report);

    ajStrDel(&strand);

    if(outf)
	ajFileClose(&outf);

    ajFloatDel(&thisap);
    ajFloatDel(&hwt);
    ajFloatDel(&agp);

    ajUintDel(&hpos);
    ajUintDel(&hp);
    ajUintDel(&hlen);


    ajStrDel(&fthit);
    ajStrDel(&substr);
    ajStrDel(&tmpFeatStr);

    embExit();

    return 0;
}




/* @funcstatic antigenic_readAnti ********************************************
**
** Reads the antigenicity index data file
**
** @param [w] agp [AjPFloat*] Data values from antigenicity index data file
** @return [void]
** @@
******************************************************************************/


static void antigenic_readAnti(AjPFloat *agp)
{
    AjPFile mfptr = NULL;
    AjPStr  line  = NULL;
    ajint Etot;
    ajint Stot;
    ajint Ptot;
    ajint n;

    ajint v1;
    ajint v2;
    ajint v3;
    float vf1;
    float vf2;
    float vf3;

    ajint deltae;
    ajint deltas;
    ajint deltap;
    float deltaaf;
    float deltasf;

    char *p;
    char *q;

    mfptr = ajDatafileNewInNameC(DATAFILE);
    if(!mfptr)
	ajFatal("Antigenicity file '%s' not found\n", DATAFILE);

    line=ajStrNew();

    deltae = deltas = deltap =0;
    deltaaf =deltasf = 0.0;

    while(ajReadline(mfptr, &line))
    {
	p = ajStrGetuniquePtr(&line);
	if(*p=='#' || *p=='!' || *p=='\n')
	    continue;

	if(strstr(p,"Total"))
	{
	    if(sscanf(p,"%*s%d%d%d",&Etot,&Stot,&Ptot) != 3)
		ajErr("Wrong number of fields in totals\n%s",
			ajStrGetPtr(line));
	    continue;
	}

	ajCharFmtUpper(p);
	q = p;
	q = ajSysFuncStrtok(q," \t");
	n = ajBasecodeToInt(*q);

	if(sscanf(p,"%*s%d%d%d%f%f%f",&v1,&v2,&v3,&vf1,&vf2,&vf3)!=6)
	{
	    ajErr("Error in table: %s",p);
	    embExitBad();
	}

	ajFloatPut(agp,n,vf3);

	if(*q != 'B' && *q != 'X' && *q != 'Z')
	{
	    deltae  += v1;
	    deltas  += v2;
	    deltap  += v3;
	    deltaaf += vf1;
	    deltasf += vf2;
	    if(fabs((double) (vf2-vf1 / vf3)) > 0.02)
		ajErr("propensity != afreq/sfreq in line\n%s",
		      p);
	}

    }


    if(deltae != Etot)
	ajErr("epitope total %d != total %d",Etot,
	      deltae);

    if(deltas != Stot)
	ajErr("surface total %d != total %d",Stot,
	      deltas);

    if(deltap != Ptot)
	ajErr("protein total %d != total %d",Ptot,
	      deltap);

    if(fabs((double) (1.0-deltaaf)) > 0.005)
	ajErr("afreq total %1.5f should be 1.0",deltaaf);

    if(fabs((double) (1.0-deltasf)) > 0.005)
	ajErr("sfreq total %1.5f should be 1.0",deltasf);


    ajStrDel(&line);
    ajFileClose(&mfptr);

    return;
}




/* @funcstatic antigenic_padit ************************************************
**
** Adds spaces to the output to fill a gap bwteeen begin and end positions
**
** @param [u] outf [AjPFile] Output text file
** @param [r] b [ajint] Begin
** @param [r] e [ajint] End
** @@
******************************************************************************/


static void antigenic_padit(AjPFile outf, ajint b, ajint e)
{
    ajint i;

    for(i=0;i<e-b-1;++i)
	ajFmtPrintF(outf," ");

    return;
}




/* @funcstatic antigenic_dumptoFeat *******************************************
**
** Undocumented.
**
** @param [r] nhits [ajint] Undocumented
** @param [r] hp [const AjPUint] Undocumented
** @param [r] hpos [const AjPUint] Undocumented
** @param [r] hlen [const AjPUint] Undocumented
** @param [r] thisap [const AjPFloat] Undocumented
** @param [r] hwt [const AjPFloat] Undocumented
** @param [u] featout [AjPFeattabOut] Undocumented
** @param [r] seqname [const char*] Undocumented
** @param [r] begin [ajint] Undocumented
** @@
******************************************************************************/

static void antigenic_dumptoFeat(ajint nhits, const AjPUint hp,
				 const AjPUint hpos, const AjPUint hlen,
				 const AjPFloat thisap, const AjPFloat hwt,
				 AjPFeattabOut featout, const char *seqname,
				 ajint begin)
{
    AjPFeattable feattable;
    AjPStr name   = NULL;
    float score   = 0.0;
    AjPStr source = NULL;
    AjPStr type   = NULL;
    AjPStr tag    = NULL;
    AjPStr val    = NULL;
    char strand   ='+';
    ajint frame   = 0;
    ajint i = 0;
    ajint k = 0;
    ajuint m = 0;
    ajuint iend;
    ajuint istart;
    ajint new;
    AjPFeature feature;

    name   = ajStrNew();
    source = ajStrNew();
    type   = ajStrNew();
    tag    = ajStrNew();
    score  = 0.0;


    val    = ajStrNew();
    name   = ajStrNew();
    source = ajStrNew();
    type   = ajStrNew();
    tag    = ajStrNew();



    ajStrAssignC(&name,seqname);

    feattable = ajFeattableNewProt(name);

    ajStrAssignC(&source,"antigenic");
    ajStrAssignC(&type,"misc_feature");

    ajStrAssignC(&tag,"note");

    for(i=nhits-1;i>-1;--i)
    {
	k = ajUintGet(hp,i);

	istart = ajUintGet(hpos,k);

	iend = istart + ajUintGet(hlen,k)-1;

	score = ajFloatGet(hwt,k);
	feature = ajFeatNew(feattable, source, type,
			    istart+begin, iend+begin,
			    score, strand, frame) ;

	new = 0;
	for(m=istart;m<=iend;++m)
	    if(ajFloatGet(thisap,m) == ajFloatGet(hwt,k))
	    {
		if(!new)
		{
		    ajFmtPrintS(&val,"max score at %d",m);
		    ajFeatTagSet(feature,tag,val);
		    new++;
		}
		else
		{
		    ajFmtPrintS(&val,",%d",m);
		    ajFeatTagSet(feature,tag,val);
		}
	    }
    }

    ajFeatSortByStart(feattable);
    ajFeattableWrite(featout, feattable);

    ajFeattableDel(&feattable);

    ajStrDel(&name);
    ajStrDel(&source);
    ajStrDel(&type);
    ajStrDel(&tag);
    ajStrDel(&val);

    return;
}
