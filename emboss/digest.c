/* @source digest application
**
** Calculate protein proteolytic (and CNBr) digest fragments
**
** @author Copyright (C) Alan Bleasby (ajb@ebi.ac.uk)
** @modified: ajb. Added ragging. Contributed by Gregoire R Thomas
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
#ifndef WIN32
#include <strings.h>
#endif



static void digest_report_hits(AjPFeattable TabRpt,
			       AjPList l, ajint be, const char *s);
static void digest_print_hits(AjPList l, AjPFile outf, ajint be,
			      const char *s);




/* @prog digest ***************************************************************
**
** Protein proteolytic enzyme or reagent cleavage digest
**
******************************************************************************/

int main(int argc, char **argv)
{
    AjPSeqall  seqall;
    AjPSeq  a;
    AjPStr  substr;
    AjPStr  rname;
    ajint be;
    ajint en;
    ajint len;

    AjBool unfavoured;
    AjBool overlap;
    AjBool allpartials;
    AjPStr menu;
    AjPStr rag;
    ajint  n = 0;
    ajint  r = 0;
    
    AjPFile  outf = NULL;
    AjPReport report    = NULL;
    AjPFeattable TabRpt = NULL;
    AjPStr tmpStr = NULL;
    AjPList  l;
    AjPList  pa;
    AjPFile mfptr   = NULL;

    AjBool nterm = ajFalse;
    AjBool cterm = ajFalse;
    AjBool dorag = ajFalse;

    ajint     ncomp;
    ajint     npart;

    EmbPPropMolwt *mwdata = NULL;
    AjBool mono;
    

    embInit("digest", argc, argv);

    seqall      = ajAcdGetSeqall("seqall");
    menu        = ajAcdGetListSingle("menu");
    dorag       = ajAcdGetBoolean("ragging");
    rag         = ajAcdGetListSingle("termini");
    unfavoured  = ajAcdGetBoolean("unfavoured");
    overlap     = ajAcdGetBoolean("overlap");
    allpartials = ajAcdGetBoolean("allpartials");
    report      = ajAcdGetReport("outfile");
    mfptr       = ajAcdGetDatafile("mwdata");
    mono        = ajAcdGetBoolean("mono");
    
    /* obsolete. Can be uncommented in acd file and here to reuse */

    /* outf      = ajAcdGetOutfile("originalfile"); */

    ajStrToInt(menu, &n);
    --n;

    ajStrToInt(rag, &r);

    if(r==2 || r==4)
	nterm = ajTrue;

    if(r==3 || r==4)
	cterm = ajTrue;


    mwdata = embPropEmolwtRead(mfptr);

    while(ajSeqallNext(seqall, &a))
    {
	substr = ajStrNew();
	be     = ajSeqGetBegin(a);
	en     = ajSeqGetEnd(a);
	ajStrAssignSubC(&substr,ajSeqGetSeqC(a),be-1,en-1);
        ajStrFmtUpper(&substr);

	len = en-be+1;

	l     = ajListNew();
	pa    = ajListNew();
	rname = ajStrNew();

	TabRpt = ajFeattableNewSeq(a);

	embPropCalcFragments(ajStrGetPtr(substr),n,&l,&pa,
			     unfavoured,overlap,
			     allpartials,&ncomp,&npart,&rname,
			     nterm, cterm, dorag, mwdata, mono);

	if(outf)
	    ajFmtPrintF(outf,"DIGEST of %s from %d to %d Molwt=%10.3f\n\n",
			ajSeqGetNameC(a),be,en,
			embPropCalcMolwt(ajSeqGetSeqC(a),0,len-1,mwdata,mono));
	if(!ncomp)
	{
	    if(outf)
		ajFmtPrintF(outf,
			    "Is not proteolytically digested using %s\n",
			    ajStrGetPtr(rname));
	}
	else
	{
	    if(outf)
	    {
		ajFmtPrintF(outf,"Complete digestion with %s "
			    "yields %d fragments:\n",
			    ajStrGetPtr(rname),ncomp);
		digest_print_hits(l,outf,be,ajStrGetPtr(substr));
	    }
	    ajFmtPrintS(&tmpStr,
			"Complete digestion with %S yields %d fragments",
			rname,ncomp);
	    ajReportSetHeader(report, tmpStr);
	    digest_report_hits(TabRpt,l,be, ajStrGetPtr(substr));
	    ajReportWrite(report, TabRpt, a);
	    ajFeattableClear(TabRpt);
	}

	if(overlap && !allpartials && npart)
	{
	    if(outf)
	    {
		ajFmtPrintF(outf,
			    "\n\nPartial digest with %s yields %d extras.\n",
			    ajStrGetPtr(rname),npart);
		ajFmtPrintF(outf,"Only overlapping partials shown:\n");
		digest_print_hits(pa,outf,be,ajStrGetPtr(substr));
	    }
	    ajFmtPrintS(&tmpStr,
			"\n\nPartial digest with %S yields %d extras.\n",
			rname,npart);
	    ajFmtPrintAppS(&tmpStr,"Only overlapping partials shown:\n");
	    ajReportSetHeader(report, tmpStr);
	    digest_report_hits(TabRpt, pa,be,ajStrGetPtr(substr));
	    ajReportWrite(report, TabRpt, a);
	    ajFeattableClear(TabRpt);
	}

	if(allpartials && npart)
	{
	    if(outf)
	    {
		ajFmtPrintF(outf,
			    "\n\nPartial digest with %s yields %d extras.\n",
			    ajStrGetPtr(rname),npart);
		ajFmtPrintF(outf,"All partials shown:\n");
		digest_print_hits(pa,outf,be,ajStrGetPtr(substr));
	    }
	    ajFmtPrintS(&tmpStr,
			"\n\nPartial digest with %S yields %d extras.\n",
			rname,npart);
	    ajFmtPrintAppS(&tmpStr,"All partials shown:\n");
	    ajReportSetHeader(report, tmpStr);
	    digest_report_hits(TabRpt, pa,be, ajStrGetPtr(substr));
	    ajReportWrite(report, TabRpt, a);
	    ajFeattableClear(TabRpt);
	}
    }


    embPropMolwtDel(&mwdata);

    ajReportDel(&report);

    ajFeattableDel(&TabRpt);
    
    ajSeqDel(&a);
    ajSeqallDel(&seqall);

    ajStrDel(&rname);
    ajStrDel(&substr);
    ajListFree(&pa);
    ajListFree(&l);
    ajStrDel(&menu);
    ajStrDel(&rag);

    if(outf)
	ajFileClose(&outf);
    ajFileClose(&mfptr);

    ajStrDel(&tmpStr);

    embExit();

    return 0;
}




/* @funcstatic digest_print_hits **********************************************
**
** Undocumented.
**
** @param [u] l [AjPList] Undocumented
** @param [u] outf [AjPFile] Undocumented
** @param [r] be [ajint] Undocumented
** @param [r] s [const char*] Undocumented
** @@
******************************************************************************/



static void digest_print_hits(AjPList l, AjPFile outf, ajint be, const char *s)
{
    EmbPPropFrag fr;
    AjPStr t;
    ajint len;
    size_t stlen;
    
    t   = ajStrNew();
    stlen = strlen(s);
    len = (ajint) stlen;

    ajFmtPrintF(outf,
		"Start   End     Molwt      Sequence (up to 38 residues)\n");
    while(ajListPop(l,(void **)&fr))
    {
	ajStrAssignSubC(&t,s,fr->start,fr->end);
	ajFmtPrintF(outf,"%-8d%-8d%-10.3f ",fr->start+be,fr->end+be,
		    fr->molwt);
	if(fr->start>0)
	    ajFmtPrintF(outf,"(%c) ",*(s+(fr->start)-1));
	else
	    ajFmtPrintF(outf," () ");

	ajFmtPrintF(outf,"%-.38s ",ajStrGetPtr(t));
	if(fr->end<len-1)
	    ajFmtPrintF(outf,"(%c) ",*(s+(fr->end+1)));
	else
	    ajFmtPrintF(outf," () ");

	if(fr->end-fr->start+1>38)
	    ajFmtPrintF(outf,"...");
	ajFmtPrintF(outf,"\n");
	AJFREE(fr);
    }

    ajStrDel(&t);

    return;
}




/* @funcstatic digest_report_hits *********************************************
**
** Undocumented.
**
** @param [u] TabRpt [AjPFeattable] feature table object to store results
** @param [u] l [AjPList] Undocumented
** @param [r] be [ajint] Undocumented
** @param [r] s [const char*] Undocumented
** @@
******************************************************************************/

static void digest_report_hits(AjPFeattable TabRpt, AjPList l, ajint be,
			       const char* s)
{
    AjPFeature gf = NULL;
    EmbPPropFrag fr;
    AjPStr t;
    ajint len;
    AjPStr tmpStr = NULL;
    size_t stlen;
    
    t   = ajStrNew();
    stlen = strlen(s);
    len = (ajint) stlen;

    while(ajListPop(l,(void **)&fr))
    {
	ajStrAssignSubC(&t,s,fr->start,fr->end);
	gf = ajFeatNewII(TabRpt,fr->start+be,fr->end+be);
	ajFmtPrintS(&tmpStr, "*molwt %.3f", fr->molwt);
	ajFeatTagAdd(gf,  NULL, tmpStr);
	if(fr->start>0)
	{
	    ajFmtPrintS(&tmpStr, "*cterm %c", *(s+(fr->start)-1));
	    ajFeatTagAdd(gf,  NULL, tmpStr);
	}

	if(fr->end<len-1)
	{
	    ajFmtPrintS(&tmpStr, "*nterm %c", *(s+(fr->end+1)));
	    ajFeatTagAdd(gf,  NULL, tmpStr);
	}

	AJFREE(fr);
    }

    ajStrDel(&t);
    ajStrDel(&tmpStr);

    return;
}
