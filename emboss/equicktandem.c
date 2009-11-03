/* @source equicktandem application
**
** Quick tandem repeat finder
**
** @author Copyright (C) Richard Durbin, J Thierry-Mieg
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

/*  File: quicktandem.c
**  Author: Richard Durbin (rd@mrc-lmba.cam.ac.uk)
**  Copyright (C) J Thierry-Mieg and R Durbin, 1993
**-------------------------------------------------------------------
** This file is part of the ACEDB genome database package, written by
** 	Richard Durbin (MRC LMB, UK) rd@mrc-lmba.cam.ac.uk, and
**	Jean Thierry-Mieg (CRBM du CNRS, France) mieg@crbm1.cnusc.fr
**
** Description: to prescreen sequences for tandem repeats.  Use
** 		tandem on anything that looks significant.
** Exported functions:
** HISTORY:
** Created: Tue Jan 19 21:25:59 1993 (rd)
**-------------------------------------------------------------------
*/

#include "emboss.h"




static AjPFile outfile = NULL;
static AjPSeqCvt cvt   = NULL;




static void equicktandem_print(AjPFile outf, ajint begin);
static void equicktandem_report(AjPFeattable tab, ajint begin);




static char *back     = NULL;
static char *front    = NULL;
static char *maxback  = NULL;
static char *maxfront = NULL;
static char* sq = NULL;
static ajint gap;
static ajint max;
static ajint score;




/* @prog equicktandem *********************************************************
**
** Finds tandem repeats
**
******************************************************************************/

int main(int argc, char **argv)
{
    ajint thresh;
    ajint maxrepeat;
    AjPSeq sequence = NULL;
    AjPSeq saveseq  = NULL;
    AjPStr tseq = NULL;
    AjPStr str  = NULL;
    AjPStr substr    = NULL;
    AjPFeattable tab = NULL;
    AjPReport report = NULL;
    AjPStr tmpstr    = NULL;

    ajint  begin;
    ajint  end;
    ajint  len;

    embInit("equicktandem", argc, argv);

    report    = ajAcdGetReport("outfile");
    outfile   = ajAcdGetOutfile("origfile");
    sequence  = ajAcdGetSeq("sequence");
    thresh    = ajAcdGetInt("threshold");
    maxrepeat = ajAcdGetInt("maxrepeat");

    saveseq = ajSeqNewSeq(sequence);
    tab = ajFeattableNewSeq(saveseq);

    ajFmtPrintAppS(&tmpstr, "Threshold: %d\n", thresh);
    ajFmtPrintAppS(&tmpstr, "Maxrepeat: %d\n", maxrepeat);
    ajReportSetHeader(report, tmpstr);

    begin = ajSeqGetBegin(sequence) - 1;
    end   = ajSeqGetEnd(sequence) - 1;

    substr = ajStrNew();
    str = ajSeqGetSeqCopyS(sequence);
    ajStrAssignSubS(&substr,str,begin,end);
    ajSeqAssignSeqS(sequence,substr);

    cvt = ajSeqcvtNewNumberC("ACGTN");
    ajSeqConvertNum(sequence, cvt, &tseq);
    sq = ajStrGetuniquePtr(&tseq);

    /* careful - sequence can be shorter than the maximum repeat length */

    if((len=ajStrGetLen(substr)) < maxrepeat)
      maxrepeat = ajStrGetLen(substr);

    for(gap = 1; gap <= maxrepeat; ++gap)
    {
	back = sq;
	front = back + gap;
	score = max = 0;
	while(front-sq<=len)
	{
	    if(*front == 'Z')
	    {
		if(max >= thresh)
		{
		    equicktandem_print(outfile, begin);
		    equicktandem_report(tab, begin);
		    back = maxfront;
		    front = back + gap;
		    score = max = 0;
		}
		else
		{
		    back = front;
		    front = back + gap;
		    score = max = 0;
		}
	    }
	    else if(*front != *back)
		--score;
	    else if(score <= 0)
	    {
		if(max >= thresh)
		{
		    equicktandem_print(outfile, begin);
		    equicktandem_report(tab, begin);
		    back = maxfront;
		    front = back + gap;
		    score = max = 0;
		}
		else
		{
		    maxback = back;
		    score = 1;
		}
	    }

	    else if(++score > max)
	    {
		max = score;
		maxfront = front;
	    }
	    ++back;
	    ++front;
	}

	if(max >= thresh)
	{
	    equicktandem_print(outfile, begin);
	    equicktandem_report(tab, begin);
	}
    }

    ajReportWrite(report, tab, saveseq);
    ajReportDel(&report);
    ajFileClose(&outfile);

    ajFeattableDel(&tab);
    ajSeqDel(&sequence);
    ajSeqDel(&saveseq);
    ajSeqcvtDel(&cvt);

    ajStrDel(&str);
    ajStrDel(&substr);
    ajStrDel(&tseq);
    ajStrDel(&tmpstr);

    embExit();

    return 0;
}




/* @funcstatic equicktandem_print *********************************************
**
** Prints the original output format, but simply returns if the
** output file is NULL.
**
** Sets the printed region to 'Z' to exclude it from further analysis.
**
** @param [u] outf [AjPFile] Undocumented
** @param [r] begin [ajint] Undocumented
** @@
******************************************************************************/

static void equicktandem_print(AjPFile outf, ajint begin)
{
    char* cp;

    if(!outf)
	return;

    ajFmtPrintF(outf, "%6d %10d %10d %2d %3d\n",
		max, 1+maxback-sq+begin, 1+maxfront-sq+begin,
		gap, (maxfront-maxback+1)/gap);
    for(cp = maxback; cp <= maxfront; ++cp)
	*cp = 'Z';

    return;
}




/* @funcstatic equicktandem_report ********************************************
**
** Saves a result as a feature.
**
** @param [u] tab [AjPFeattable] Undocumented
** @param [r] begin [ajint] Undocumented
** @@
******************************************************************************/

static void equicktandem_report(AjPFeattable tab, ajint begin)
{
    static char* cp;

    AjPFeature gf;
    AjPStr rpthit = NULL;
    AjPStr s = NULL;

    if(!rpthit)
	ajStrAssignC(&rpthit, "repeat_region");

    /*
       ajFmtPrintF(outf, "%6d %10d %10d %2d %3d\n",
       max, 1+maxback-sq+begin, 1+maxfront-sq+begin,
       gap, (maxfront-maxback+1)/gap);
       */


    gf = ajFeatNew(tab, NULL, rpthit,
		   1+(ajint)(maxback-sq)+begin,
		   1+(ajint)(maxfront-sq)+begin,
		   (float) max, '+', 0);

    ajFeatTagAddCC(gf, "rpt_type", "TANDEM");
    ajFmtPrintS(&s, "*rpt_size %d", gap);
    ajFeatTagAdd(gf, NULL, s);
    ajFmtPrintS(&s, "*rpt_count %d", (maxfront-maxback+1) / gap);
    ajFeatTagAdd(gf, NULL, s);

    for(cp = maxback; cp <= maxfront; ++cp)
	*cp = 'Z';

    ajStrDel(&rpthit);
    ajStrDel(&s);

    return;
}

