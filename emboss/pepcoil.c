/* @source pepcoil application
**
** Displays coiled coil sites in proteins
** @author Copyright (C) Alan Bleasby (ableasby@hgmp.mrc.ac.uk)
** @@
** Original program "PEPCOIL" by Peter Rice (EGCG 1991)
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

#define COILFILE "Epepcoil.dat"




static void pepcoil_readcoildat(AjPFloat2d *rdat);
static float pepcoil_probcoil(float score);
static ajint pepcoil_inframe(ajint start, ajint pos, ajint frame, ajint len);




/* @prog pepcoil **************************************************************
**
** Predicts coiled coil regions
**
******************************************************************************/

int main(int argc, char **argv)
{
    AjPSeqall seqall;
    AjPSeq seq    = NULL;
    AjPReport report  = NULL;
    AjPFeattable TabRpt = NULL;
    AjPFeature gf = NULL;
    AjPStr strand = NULL;
    AjPStr sstr   = NULL;
    AjPStr stmp   = NULL;
    AjPStr substr = NULL;
    AjPStr tmpstr = NULL;
    AjPStr tmpframe = NULL;
    AjPStr fthit  = NULL;
    AjPStr ftmiss = NULL;

    ajint begin;
    ajint end;
    ajint len;
    ajint coilpos=1;

    const char *p;
    char *q;

    ajint i;
    ajint j;
    ajint k;
    ajint window;

    AjBool coil;
    AjBool frame;
    AjBool other;


    AjPFloat scores  = NULL;
    AjPFloat pscores = NULL;
    AjPFloat probs   = NULL;
    AjPFloat rframes = NULL;
    AjPInt   frames  = NULL;
    AjPInt   parray  = NULL;

    float score;
    float maxscore;
    float maxmaxscore;
    float maxcoil;

    ajint maxframe;
    ajint rescode;
    ajint wstart;
    ajint wend;
    ajint lwin;
    ajint win;
    ajint isub;
    ajint startcoil;
    ajint endcoil;
/*    ajint lencoil; */
    ajint startframe;
    ajint coilframe;
    ajint fframe;

    AjBool iscoil;
    AjPFloat2d rdat = NULL;
    AjPList framelist = NULL;

    embInit("pepcoil", argc, argv);


    rdat = ajFloat2dNew();
    pepcoil_readcoildat(&rdat);

    seqall = ajAcdGetSeqall("sequence");
    window = ajAcdGetInt("window");
    report = ajAcdGetReport("outfile");

    coil   = ajAcdGetBoolean("coil");
    frame  = ajAcdGetBoolean("frame");
    other  = ajAcdGetBoolean("other");


    substr = ajStrNew();
    sstr   = ajStrNew();
    stmp   = ajStrNew();

    scores  = ajFloatNew();
    pscores = ajFloatNew();
    probs   = ajFloatNew();
    rframes = ajFloatNew();
    frames  = ajIntNew();
    parray  = ajIntNew();

    ajStrAssignC(&fthit, "coiled");
    ajStrAssignC(&ftmiss, "region");

    ajFmtPrintS(&tmpstr,"Window size: %d residues\n",window);
    ajReportSetHeaderS(report, tmpstr);
    framelist = ajListstrNew();
    
    while(ajSeqallNext(seqall, &seq))
    {
	begin = ajSeqallGetseqBegin(seqall);
	end   = ajSeqallGetseqEnd(seqall);

	strand = ajSeqGetSeqCopyS(seq);
	ajStrFmtUpper(&strand);

	ajStrAssignSubC(&substr,ajStrGetPtr(strand),begin-1,end-1);
	ajStrAssignSubC(&sstr,ajStrGetPtr(strand),begin-1,end-1);

	len = ajStrGetLen(substr);

	TabRpt = ajFeattableNewSeq(seq);

	q = ajStrGetuniquePtr(&substr);
	for(i=0;i<len;++i,++q)
	    *q = (char) ajBasecodeToInt(*q);

	p = ajStrGetPtr(substr);

	maxmaxscore = -1.0;
	isub = window -1;
	for(i=0;i<len;++i)
	{
	    wstart = (0 > i-isub) ? 0 : i-isub;
	    lwin = (len < i+isub) ? len : i+isub;
	    wend = (i < lwin-isub) ? i : lwin-isub;

	    maxscore = -1.0;
	    maxframe = 0;

	    for(win=wstart;win<=wend;++win)
		for(fframe=1;fframe<=7;++fframe)
		{
		    k = fframe - (((i-win)+1)%7);

		    if(k<1)
			k+=7;

		    for(j=0;j<window;++j)
		    {
			rescode = *(p+win+j);
			ajFloatPut(&scores,j,ajFloat2dGet(rdat,rescode,k++));
			if(k>7) k=1;
		    }
		    score = ajMathGmean(ajFloatFloat(scores),window);

		    if(score>maxscore)
		    {
			maxscore = score;
			maxframe = fframe;
		    }
		}
	    ajFloatPut(&pscores,i,maxscore);
	    ajIntPut(&frames,i,maxframe);
	    ajFloatPut(&rframes,i,(float) maxframe);
	    ajFloatPut(&probs,i,pepcoil_probcoil(maxscore));
	    if(ajFloatGet(pscores,i)>maxmaxscore)
		maxmaxscore=ajFloatGet(pscores,i);
	}

	for(i=0;i<len;++i)
	    ajIntPut(&parray,i,i);


	iscoil = (ajFloatGet(probs,0) >= 0.5);
	startcoil = 0;
	coilframe = ajIntGet(frames,0);
	startframe=0;
	maxcoil=ajFloatGet(probs,0);
	maxscore=ajFloatGet(pscores,0);
	if(iscoil && coil)
	{
	    /*
	    ajFmtPrintS(&tmpstr,"Prediction starts at %d\n",
			startcoil + begin);
	    ajReportAppendSubheaderS(report, tmpstr);
	    */
	}

	for(i=0;i<len;++i)
	{
	    if(ajFloatGet(probs,i) >= 0.5) /* we are in a coil */
	    {
		if(iscoil)      /* continuing coil */
		{
		    if(ajFloatGet(probs,i) > maxcoil)
		    {
			maxcoil = ajFloatGet(probs,i);
			coilpos = i+1;
		    }
		    if(ajFloatGet(pscores,i) > maxscore)
		    {
			maxscore = ajFloatGet(pscores,i);
		    }
		    if(pepcoil_inframe(startframe,i,ajIntGet(frames,i),7)
		       != coilframe)
		    {
			if(i && frame)
			{
			    ajFmtPrintS(&tmpframe, "*frames %d..%d",
					coilframe,
					ajIntGet(frames,i-1));
                            ajListstrPushAppend(framelist, tmpframe);
                            tmpframe = NULL;
			/*
			    ajFmtPrintF(outf,"%10d..%d   frame %d..%d\n",
					startframe+begin,i-1+begin,
					coilframe+begin,ajIntGet(frames,i-1)
					+begin);
			*/
			}
			coilframe  = ajIntGet(frames,i);
			startframe = i;
		    }
		}
		else            /* start of a new coil */
		{
		    endcoil = i-1;
		    /* lencoil = endcoil-startcoil+1; */
		    if(other)
		    {
			gf = ajFeatNewProt(TabRpt, NULL, ftmiss,
					   startcoil+begin,
					   endcoil+begin,
					   maxscore);
			ajFmtPrintS(&tmpstr, "*probability %.3f",
				    maxcoil);
			ajFeatTagAdd(gf,  NULL, tmpstr);
			ajFmtPrintS(&tmpstr, "*pos %d",
				    coilpos);
			ajFeatTagAdd(gf,  NULL, tmpstr);
			/*
			ajFmtPrintF(outf,
			  "\nOther structures from %d to %d (%d residues)\n",
			  startcoil+begin,endcoil+begin,lencoil);
			ajFmtPrintF(outf,
				    "   Max score: %.3f (probability %.2f)\n",
				    maxscore,maxcoil);
			*/
		    }
		    iscoil = ajTrue;
		    startcoil  = i;
		    coilpos = i+1;
		    maxcoil    = ajFloatGet(probs,i);
		    maxscore   = ajFloatGet(pscores,i);
		    coilframe  = ajIntGet(frames,i);
		    startframe = i;
		    if(coil)
		    {
			/*
			ajFmtPrintS(&tmpstr,"Prediction starts at %d\n",
				    startcoil + begin);
			ajReportAppendSubheaderS(report, tmpstr);
			ajFmtPrintF(outf,"\nPrediction starts at %d\n",
			startcoil+begin);
			*/
		    }
		}
	    }
	    else
	    {
		if(iscoil)
		{
			    
		    endcoil = i-1;
		    /* lencoil = endcoil - startcoil; */
		    if(frame)
		    {
			/*
			ajFmtPrintF(outf,"%10d..%d   frame %d..%d\n",
				    startframe+begin,i-1+begin,
				    coilframe+begin,ajIntGet(frames,i-1)
				    +begin);
			*/
		    }
		    if(coil)
		    {
			gf = ajFeatNewProt(TabRpt, NULL, fthit,
					   startframe+begin,
					   i-1+begin,
					   maxscore);
			if(frame)
			{
			    ajFmtPrintS(&tmpstr, "*frames %d..%d",
					coilframe,
					ajIntGet(frames,i-1));
			    ajFeatTagAdd(gf,  NULL, tmpstr);
			}
			ajFmtPrintS(&tmpstr, "*predict coiled");
			ajFeatTagAdd(gf,  NULL, tmpstr);
			ajFmtPrintS(&tmpstr, "*probability %.3f",
				    maxcoil);
			ajFeatTagAdd(gf,  NULL, tmpstr);
			ajFmtPrintS(&tmpstr, "*pos %d",
				    coilpos);
			ajFeatTagAdd(gf,  NULL, tmpstr);
                        while(ajListstrPop(framelist, &tmpframe))
                        {
			    ajFeatTagAdd(gf,  NULL, tmpframe);
                        }
			/*
			ajFmtPrintF(outf,
			  "probable coiled-coil from %d to %d (%d residues)\n",
			  startcoil+begin,endcoil+begin,lencoil);
			ajFmtPrintF(outf,
				    "   Max score: %.3f (probability %.2f)\n",
				    maxscore,maxcoil);
			*/
		    }
		    iscoil    = ajFalse;
		    maxcoil   = ajFloatGet(probs,i);
		    maxscore  = ajFloatGet(pscores,i);
		    startcoil = i;
		    coilpos = i+1;
		}
		else
		{
		    if(ajFloatGet(probs,i) > maxcoil)
		    {
			maxcoil = ajFloatGet(probs,i);
			coilpos = i+1;
		    }
		    if(ajFloatGet(pscores,i) > maxscore)
		    {
			maxscore = ajFloatGet(pscores,i);
		    }
		}
	    }
	}

	/* lencoil = len-startcoil; */
	if(iscoil)
	{
	    if(coil)
	    {
		gf = ajFeatNewProt(TabRpt, NULL, fthit,
				   startcoil+begin,
				   len-1+begin,
				   maxscore);
		ajFmtPrintS(&tmpstr, "*predict coiled");
		ajFeatTagAdd(gf,  NULL, tmpstr);
		ajFmtPrintS(&tmpstr, "*probability %.3f",
			    maxcoil);
		ajFeatTagAdd(gf,  NULL, tmpstr);
		ajFmtPrintS(&tmpstr, "*pos %d",
			    coilpos);
		ajFeatTagAdd(gf,  NULL, tmpstr);
                while(ajListstrPop(framelist, &tmpframe))
                {
                    ajFeatTagAdd(gf,  NULL, tmpframe);
                }
	    /*
		ajFmtPrintF(outf,
			 "Probable coiled-coil from %d to %d (%d residues)\n",
			 startcoil+begin,len+begin-1,lencoil);
		ajFmtPrintF(outf,"   Max score: %.3f (probability %.2f)\n",
			    maxscore,maxcoil);
	    */
	    }
	}
	else
	    if(other)
	    {
		gf = ajFeatNewProt(TabRpt, NULL, ftmiss,
				   startcoil+begin,
				   len-1+begin,
				   maxscore);
		ajFmtPrintS(&tmpstr, "*probability %.3f",
			    maxcoil);
		ajFeatTagAdd(gf,  NULL, tmpstr);
		ajFmtPrintS(&tmpstr, "*pos %d",
			    coilpos);
		ajFeatTagAdd(gf,  NULL, tmpstr);
		/*
		ajFmtPrintF(outf,
			    "\nOther structures from %d to %d (%d residues)\n",
			    startcoil+begin,len+begin-1,lencoil);
		ajFmtPrintF(outf,"   Max score: %.3f (probability %.2f)\n",
			    maxscore,maxcoil);
		*/
	    }

	ajReportWrite(report, TabRpt, seq);
	ajFeattableDel(&TabRpt);
	ajStrDel(&strand);
    }

    ajListstrFreeData(&framelist);
    ajIntDel(&frames);
    ajIntDel(&parray);

    ajFloatDel(&scores);
    ajFloatDel(&pscores);
    ajFloatDel(&probs);
    ajFloatDel(&rframes);

    ajFloat2dDel(&rdat);

    ajStrDel(&stmp);
    ajStrDel(&sstr);
    ajStrDel(&substr);
    ajSeqDel(&seq);
    ajReportDel(&report);

    ajSeqallDel(&seqall);
    ajSeqDel(&seq);

    ajStrDel(&fthit);
    ajStrDel(&ftmiss);
    ajStrDel(&tmpstr);

    embExit();

    return 0;
}




/* @funcstatic pepcoil_readcoildat ********************************************
**
** Read coil data from emboss data area
**
** @param [w] rdat [AjPFloat2d*] coil data
** @@
******************************************************************************/

static void pepcoil_readcoildat(AjPFloat2d *rdat)
{
    AjPFile mfptr = NULL;
    AjPStr  line  = NULL;
    static  char delim[]=" \t\n";

    char *p;
    const char *q;
    ajint n;
    ajint c;

    float v;

    mfptr = ajDatafileNewInNameC(COILFILE);
    if(!mfptr)
	ajFatal("Pepcoil data file '%s' not found\n",COILFILE);

    line = ajStrNew();

    while(ajReadline(mfptr, &line))
    {
	p=ajStrGetuniquePtr(&line);
	if(*p=='#' || *p=='!' || !*p) continue;
	ajCharFmtUpper(p);
	q=p;
	q=ajSysFuncStrtok(q," \t");
	n=ajBasecodeToInt(*q);
	c = 0;
	while((q=ajSysFuncStrtok(NULL,delim)))
	{
	    sscanf(q,"%f",&v);
	    ajFloat2dPut(rdat,n,c++,v);
	}
    }

    ajStrDel(&line);
    ajFileClose(&mfptr);

    return;
}




/* @funcstatic pepcoil_probcoil ***********************************************
**
** Calculate coil probability
**
** @param [r] score [float] score
** @return [float] probability
** @@
******************************************************************************/

static float pepcoil_probcoil(float score)
{
    float gcc;
    float gg;
    double td;
    
    td = ajCvtGaussToProb((float)1.63,(float)0.24,score);
    gcc = (float) td;
    td  = ajCvtGaussToProb((float)0.77,(float)0.20,score);
    gg  = (float) td;

    return gcc/((float)30.0*gg + gcc);
}




/* @funcstatic pepcoil_inframe ************************************************
**
** Return frame
**
** @param [r] start [ajint] start
** @param [r] pos [ajint] position
** @param [r] frame [ajint] frame
** @param [r] len [ajint] length
** @return [ajint] frame value
** @@
******************************************************************************/

static ajint pepcoil_inframe(ajint start, ajint pos, ajint frame, ajint len)
{
    return 1+ajMathModulo(frame-pos+start-1,len);
}
