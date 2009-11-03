/* @source pscan application
**
** Reports fingerprints in a protein sequence
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




static void pscan_print_hits(AjPFile outf, AjPList l, ajuint nmotifs,
			     ajuint begin);




/* @prog pscan ****************************************************************
**
** Scans proteins using PRINTS
**
******************************************************************************/

int main(int argc, char **argv)
{
    AjPSeqall seqall;
    AjPSeq seq   = NULL;
    AjPFile outf = NULL;
    ajint begin;
    ajint end;
    ajuint emin;
    ajuint emax;

    AjPFile mfile = NULL;
    EmbPMatPrints s = NULL;
    AjPList l = NULL;
    AjPStr strand = NULL;
    AjPStr substr = NULL;
    AjPStr name = NULL;

    AjBool all;
    AjBool ordered;

    ajint hits;
    ajint nmotifs;

    embInit("pscan", argc, argv);

    seqall = ajAcdGetSeqall("sequence");
    outf   = ajAcdGetOutfile("outfile");
    emin   = ajAcdGetInt("emin");
    emax   = ajAcdGetInt("emax");

    substr = ajStrNew();
    name   = ajStrNew();

    all = ordered = ajTrue;



    while(ajSeqallNext(seqall, &seq))
    {
	begin = ajSeqallGetseqBegin(seqall);
	end   = ajSeqallGetseqEnd(seqall);

	ajStrAssignC(&name,ajSeqGetNameC(seq));
	strand = ajSeqGetSeqCopyS(seq);

	ajStrAssignSubC(&substr,ajStrGetPtr(strand),begin-1,end-1);

	nmotifs = 0;
	ajDebug("pscan sequence '%S'\n", ajSeqGetNameS(seq));
	embMatPrintsInit(&mfile);
	ajDebug("pscan prints file '%F'\n", mfile);
	l = ajListNew();
	while((s = embMatProtReadInt(mfile)))
	{
	    ajDebug("pscan file '%F' read\n", mfile);
	    if(s->n >= emin && s->n <= emax)
	    {
		hits=embMatProtScanInt(substr,name,s,&l,&all,&ordered,
					    1);
		if(hits)
		    ++nmotifs;
	    }
	    embMatProtDelInt(&s);
	}

	pscan_print_hits(outf, l, nmotifs, begin);
	ajListFree(&l);


	ajStrDel(&strand);
	ajFileClose(&mfile);
    }

    ajStrDel(&substr);
    ajStrDel(&name);

    ajSeqallDel(&seqall);
    ajSeqDel(&seq);
    ajFileClose(&outf);

    embExit();

    return 0;
}




/* @funcstatic pscan_print_hits ***********************************************
**
** Undocumented.
**
** @param [u] outf [AjPFile] outfile
** @param [u] l [AjPList] hits
** @param [r] nmotifs [ajuint] number of hits
** @param [r] begin [ajuint] start position
** @@
******************************************************************************/

static void pscan_print_hits(AjPFile outf, AjPList l, ajuint nmotifs,
			     ajuint begin)
{
    EmbPMatMatch mm;
    ajuint i;
    ajuint j;
    AjBool found;
    ajuint nleft;
    ajuint maxelem = 0;
    ajuint maxhpm;
    ajuint hpm = 0;

    nleft = nmotifs;


    ajFmtPrintF(outf,"\n\nCLASS 1\n");
    ajFmtPrintF(outf,"Fingerprints with all elements in order\n\n");


    found = ajTrue;

    while(found && nleft)
    {
	found = ajFalse;
	maxelem = 0;
	for(i=0;i<nleft;++i)
	{
	    ajListPop(l,(void **)&mm);
	    if(mm->all && mm->ordered)
	    {
		maxelem = AJMAX(maxelem,mm->n);
		found = ajTrue;
	    }
	    hpm = mm->hpm;

	    ajListPushAppend(l,(void *)mm);
	    for(j=1;j<hpm;++j)
	    {
		ajListPop(l,(void **)&mm);
		ajListPushAppend(l,(void *)mm);
	    }
	}


	if(found)
	{
	    found = ajFalse;
	    for(i=0;i<nleft;++i)
	    {
		ajListPop(l,(void **)&mm);
		hpm = mm->hpm;

		if(mm->all && mm->ordered && maxelem==mm->n)
		    break;
		ajListPushAppend(l,(void *)mm);

		for(j=1;j<hpm;++j)
		{
		    ajListPop(l,(void **)&mm);
		    ajListPushAppend(l,(void *)mm);
		}
	    }

	    if(mm->all && mm->ordered && maxelem==mm->n)
	    {
		ajFmtPrintF(outf,"Fingerprint %s Elements %d\n",
			    ajStrGetPtr(mm->cod),mm->n);
		ajFmtPrintF(outf,"    Accession number %s\n",
			    ajStrGetPtr(mm->acc));
		ajFmtPrintF(outf,"    %s\n",ajStrGetPtr(mm->tit));
		ajFmtPrintF(outf,"  Element %d Threshold %d%% Score %d%%\n",
			    mm->element+1, mm->thresh, mm->score);
		ajFmtPrintF(outf,"             Start position %d Length %d\n",
			    mm->start+begin,mm->len);

		embMatMatchDel(&mm);
	    }

	    for(i=1;i<hpm;++i)
	    {
		ajListPop(l,(void **)&mm);
		ajFmtPrintF(outf,"  Element %d Threshold %d%% Score %d%%\n",
			    mm->element+1, mm->thresh, mm->score);
		ajFmtPrintF(outf,"             Start position %d Length %d\n",
			    mm->start+begin,mm->len);

		embMatMatchDel(&mm);
	    }

	    --nleft;
	}

    }

    ajFmtPrintF(outf,"\n\nCLASS 2\n");
    ajFmtPrintF(outf,"All elements match but not all in the "
		"correct order\n\n");

    found = ajTrue;

    while(found && nleft)
    {
	found = ajFalse;
	maxelem = 0;

	for(i=0;i<nleft;++i)
	{
	    ajListPop(l,(void **)&mm);
	    if(mm->all)
	    {
		maxelem = AJMAX(maxelem,mm->n);
		found = ajTrue;
	    }
	    hpm = mm->hpm;
	    ajListPushAppend(l,(void *)mm);

	    for(j=1;j<hpm;++j)
	    {
		ajListPop(l,(void **)&mm);
		ajListPushAppend(l,(void *)mm);
	    }
	}

	if(found)
	{

	    found=ajFalse;
	    for(i=0;i<nleft;++i)
	    {
		ajListPop(l,(void **)&mm);
		hpm = mm->hpm;
		if(mm->all && maxelem==mm->n)
		    break;
		ajListPushAppend(l,(void *)mm);

		for(j=1;j<hpm;++j)
		{
		    ajListPop(l,(void **)&mm);
		    ajListPushAppend(l,(void *)mm);
		}
	    }

	    if(mm->all && maxelem==mm->n)
	    {
		ajFmtPrintF(outf,"Fingerprint %s Elements %d\n",
			    ajStrGetPtr(mm->cod),mm->n);
		ajFmtPrintF(outf,"    Accession number %s\n",
			    ajStrGetPtr(mm->acc));
		ajFmtPrintF(outf,"    %s\n",ajStrGetPtr(mm->tit));
		ajFmtPrintF(outf,"  Element %d Threshold %d%% Score %d%%\n",
			    mm->element+1, mm->thresh, mm->score);
		ajFmtPrintF(outf,"             Start position %d Length %d\n",
			    mm->start+begin,mm->len);

		embMatMatchDel(&mm);
	    }

	    for(i=1;i<hpm;++i)
	    {
		ajListPop(l,(void **)&mm);
		ajFmtPrintF(outf,"  Element %d Threshold %d%% Score %d%%\n",
			    mm->element+1, mm->thresh, mm->score);
		ajFmtPrintF(outf,"             Start position %d Length %d\n",
			    mm->start+begin,mm->len);

		embMatMatchDel(&mm);
	    }

	    --nleft;
	}

    }




    ajFmtPrintF(outf,"\n\nCLASS 3\n");
    ajFmtPrintF(outf,
		"Not all elements match but those that do are in order\n\n");

    found = ajTrue;

    while(found && nleft)
    {
	found  = ajFalse;
	maxhpm = 0;
	for(i=0;i<nleft;++i)
	{
	    ajListPop(l,(void **)&mm);
	    if(mm->ordered)
	    {
		maxhpm = AJMAX(maxelem,mm->hpm);
		found  = ajTrue;
	    }
	    hpm = mm->hpm;
	    ajListPushAppend(l,(void *)mm);

	    for(j=1;j<hpm;++j)
	    {
		ajListPop(l,(void **)&mm);
		ajListPushAppend(l,(void *)mm);
	    }
	}


	if(found)
	{
	    found = ajFalse;
	    for(i=0;i<nleft;++i)
	    {
		ajListPop(l,(void **)&mm);
		hpm = mm->hpm;

		if(mm->ordered && maxhpm==mm->hpm)
		    break;
		ajListPushAppend(l,(void *)mm);

		for(j=1;j<hpm;++j)
		{
		    ajListPop(l,(void **)&mm);
		    ajListPushAppend(l,(void *)mm);
		}
	    }

	    if(mm->ordered && maxhpm==mm->hpm)
	    {
		ajFmtPrintF(outf,"Fingerprint %s Elements %d\n",
			    ajStrGetPtr(mm->cod),mm->n);
		ajFmtPrintF(outf,"    Accession number %s\n",
			    ajStrGetPtr(mm->acc));
		ajFmtPrintF(outf,"    %s\n",ajStrGetPtr(mm->tit));
		ajFmtPrintF(outf,"  Element %d Threshold %d%% Score %d%%\n",
			    mm->element+1, mm->thresh, mm->score);
		ajFmtPrintF(outf,"             Start position %d Length %d\n",
			    mm->start+begin,mm->len);


		embMatMatchDel(&mm);
	    }

	    for(i=1;i<hpm;++i)
	    {
		ajListPop(l,(void **)&mm);
		ajFmtPrintF(outf,"  Element %d Threshold %d%% Score %d%%\n",
			    mm->element+1, mm->thresh, mm->score);
		ajFmtPrintF(outf,"             Start position %d Length %d\n",
			    mm->start+begin,mm->len);

		embMatMatchDel(&mm);
	    }

	    --nleft;
	}

    }




    ajFmtPrintF(outf,"\n\nCLASS 4\n");
    ajFmtPrintF(outf,
		"Remaining partial matches\n\n");

    found = ajTrue;

    while(found && nleft)
    {
	found = ajFalse;
	maxhpm = 0;

	for(i=0;i<nleft;++i)
	{
	    ajListPop(l,(void **)&mm);
	    maxhpm = AJMAX(maxelem,mm->hpm);
	    found = ajTrue;

	    hpm = mm->hpm;
	    ajListPushAppend(l,(void *)mm);
	    for(j=1;j<hpm;++j)
	    {
		ajListPop(l,(void **)&mm);
		ajListPushAppend(l,(void *)mm);
	    }
	}


	if(found)
	{
	    found = ajFalse;
	    for(i=0;i<nleft;++i)
	    {
		ajListPop(l,(void **)&mm);
		hpm = mm->hpm;

		if(maxhpm==mm->hpm)
		    break;
		ajListPushAppend(l,(void *)mm);

		for(j=1;j<hpm;++j)
		{
		    ajListPop(l,(void **)&mm);
		    ajListPushAppend(l,(void *)mm);
		}
	    }

	    if(maxhpm==mm->hpm)
	    {
		ajFmtPrintF(outf,"Fingerprint %s Elements %d\n",
			    ajStrGetPtr(mm->cod),mm->n);
		ajFmtPrintF(outf,"    Accession number %s\n",
			    ajStrGetPtr(mm->acc));
		ajFmtPrintF(outf,"    %s\n",ajStrGetPtr(mm->tit));
		ajFmtPrintF(outf,"  Element %d Threshold %d%% Score %d%%\n",
			    mm->element+1, mm->thresh, mm->score);
		ajFmtPrintF(outf,"             Start position %d Length %d\n",
			    mm->start+begin,mm->len);


		embMatMatchDel(&mm);
	    }

	    for(i=1;i<hpm;++i)
	    {
		ajListPop(l,(void **)&mm);
		ajFmtPrintF(outf,"  Element %d Threshold %d%% Score %d%%\n",
			    mm->element+1, mm->thresh, mm->score);
		ajFmtPrintF(outf,"             Start position %d Length %d\n",
			    mm->start+begin,mm->len);

		embMatMatchDel(&mm);
	    }

	    --nleft;
	}

    }


    while(ajListPop(l,(void **)&mm))
	if(mm)
	    embMatMatchDel(&mm);

    return;
}
