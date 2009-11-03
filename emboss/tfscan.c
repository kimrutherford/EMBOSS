/* @source tfscan application
**
** Finds transcription factors
** @author Copyright (C) Alan Bleasby (ableasby@hgmp.mrc.ac.uk)
** @@
**
** 12/03/2000: (AJB) Added accession numbers, end points and matching sequence
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
#include "string.h"




static void tfscan_print_hits(const AjPStr name, AjPList *l, ajint hits,
			      AjPFile outf, ajint begin, ajint end,
			      const AjPTable t, const AjPSeq seq,
			      ajuint minlength,
			      const AjPTable btable);




/* @prog tfscan ***************************************************************
**
** Scans DNA sequences for transcription factors
**
******************************************************************************/

int main(int argc, char **argv)
{
    AjPSeqall seqall;
    AjPSeq seq   = NULL;
    AjPFile outf = NULL;
    AjPFile inf  = NULL;

    ajint begin;
    ajint end;

    AjPList l = NULL;

    AjPStr strand = NULL;
    AjPStr substr = NULL;
    AjPStr line   = NULL;
    AjPStr name   = NULL;
    AjPStr acc    = NULL;
    AjPStr bf     = NULL;
    AjPStr menu;
    AjPStr pattern  = NULL;
    AjPStr opattern = NULL;
    AjPStr pname    = NULL;
    AjPStr key      = NULL;
    AjPStr value    = NULL;
    AjPTable atable = NULL;
    AjPTable btable = NULL;
    
    ajint mismatch;
    ajint minlength;
    
    ajint sum;
    ajint v;

    char cp;
    const char *p;


    embInit("tfscan", argc, argv);

    seqall     = ajAcdGetSeqall("sequence");
    outf       = ajAcdGetOutfile("outfile");
    mismatch   = ajAcdGetInt("mismatch");
    minlength  = ajAcdGetInt("minlength");
    menu       = ajAcdGetListSingle("menu");

    pname = ajStrNew();
    cp=ajStrGetCharFirst(menu);

    if(cp=='F')
	ajStrAssignC(&pname,"tffungi");
    else if(cp=='I')
	ajStrAssignC(&pname,"tfinsect");
    else if(cp=='O')
	ajStrAssignC(&pname,"tfother");
    else if(cp=='P')
	ajStrAssignC(&pname,"tfplant");
    else if(cp=='V')
	ajStrAssignC(&pname,"tfvertebrate");
    else if(cp=='C')
	inf = ajAcdGetDatafile("custom");

    if(cp!='C')
    {
	inf = ajDatafileNewInNameS(pname);
	if(!inf)
	    ajFatal("Either EMBOSS_DATA undefined or TFEXTRACT needs running");
    }

    name     = ajStrNew();
    acc      = ajStrNew();
    bf       = ajStrNewC("");
    substr   = ajStrNew();
    line     = ajStrNew();
    pattern  = ajStrNewC("AA");
    opattern = ajStrNew();

    while(ajSeqallNext(seqall, &seq))
    {
	begin=ajSeqallGetseqBegin(seqall);
	end=ajSeqallGetseqEnd(seqall);
	ajStrAssignC(&name,ajSeqGetNameC(seq));
	strand=ajSeqGetSeqCopyS(seq);

	ajStrAssignSubC(&substr,ajStrGetPtr(strand),begin-1,end-1);
	ajStrFmtUpper(&substr);

	l=ajListNew();
	atable = ajTablestrNewLen(1000);
	btable = ajTablestrNewLen(1000);
	
	sum=0;
	while(ajReadlineTrim(inf,&line))
	{
	    p = ajStrGetPtr(line);

	    if(!*p || *p=='#' || *p=='\n' || *p=='!')
		continue;

	    ajFmtScanS(line,"%S%S%S",&pname,&pattern,&acc);
	    p += ajStrGetLen(pname);
	    while(*p && *p==' ')
		++p;
	    p += ajStrGetLen(pattern);
	    while(*p && *p==' ')
		++p;
	    p += ajStrGetLen(acc);
	    while(*p && *p==' ')
		++p;

	    ajStrAssignS(&opattern,pattern);
	    ajStrAssignC(&bf,p);
	    
	    v = embPatVariablePattern(pattern,substr,pname,l,0,
				      mismatch,begin);
	    if(v)
	    {
		key = ajStrNewC(ajStrGetPtr(pname));
		value = ajStrNewC(ajStrGetPtr(acc));
		ajTablePut(atable,(void *)key,(void *)value);
		key = ajStrNewC(ajStrGetPtr(pname));
		value = ajStrNewC(ajStrGetPtr(bf));
		ajTablePut(btable,(void *)key,(void *)value);
	    }
	    sum += v;
	}

	if(sum)
	    tfscan_print_hits(name,&l,sum,outf,begin,end,atable,seq,minlength,
			      btable);

	ajFileSeek(inf,0L,0);
	ajListFree(&l);
	ajTablestrFree(&atable);
	ajTablestrFree(&btable);
	ajStrDel(&strand);
    }

    ajStrDel(&line);
    ajStrDel(&name);
    ajStrDel(&acc);
    ajStrDel(&pname);
    ajStrDel(&opattern);
    ajStrDel(&bf);
    ajStrDel(&pattern);
    ajStrDel(&substr);
    ajSeqDel(&seq);
    ajFileClose(&inf);
    ajFileClose(&outf);

    ajSeqallDel(&seqall);
    ajSeqDel(&seq);
    ajStrDel(&menu);

    embExit();

    return 0;
}




/* @funcstatic tfscan_print_hits **********************************************
**
** Print matches to transcription factor sites
**
** @param [r] name [const AjPStr] name of test sequence
** @param [w] l [AjPList*] list of hits
** @param [r] hits [ajint] number of hits
** @param [u] outf [AjPFile] output file
** @param [r] begin [ajint] sequence start
** @param [r] end [ajint] sequence end
** @param [r] t [const AjPTable] table of accession numbers
** @param [r] seq [const AjPSeq] test sequence
** @param [r] minlength [ajuint] minimum length of pattern
** @param [r] btable [const AjPTable] BF lines from transfac (if any)
** @@
******************************************************************************/

static void tfscan_print_hits(const AjPStr name, AjPList *l,
			      ajint hits, AjPFile outf,
			      ajint begin, ajint end, const AjPTable t,
			      const AjPSeq seq, ajuint minlength,
			      const  AjPTable btable)
{
    ajint i;
    EmbPMatMatch m;
    AjPStr acc = NULL;
    AjPStr s   = NULL;
    AjPStr bf  = NULL;
    AjPStr lastnam = NULL;
    
    s       = ajStrNew();
    lastnam = ajStrNewC("");

    ajFmtPrintF(outf,"TFSCAN of %s from %d to %d\n\n",ajStrGetPtr(name),
		begin,end);

    for(i=0;i<hits;++i)
    {
	ajListPop(*l,(void **)&m);
	acc = ajTableFetch(t,(const void *)m->seqname);



	if((ajStrCmpS(m->seqname,lastnam)) && ajStrGetLen(lastnam)
	   && ajStrGetLen(s) >= minlength)
	{
	    bf  = ajTableFetch(btable,(const void *)lastnam);
	    if(ajStrGetLen(bf))
		ajFmtPrintF(outf,"                     %S\n",bf);
	}
	
	ajStrAssignS(&lastnam,m->seqname);

	ajStrAssignSubC(&s,ajSeqGetSeqC(seq),m->start-1,m->start+m->len-2);

	if(ajStrGetLen(s) >= minlength)
	    ajFmtPrintF(outf,"%-20s %-8s %-5d %-5d %s\n",ajStrGetPtr(m->seqname),
			ajStrGetPtr(acc),m->start,
			m->start+m->len-1,ajStrGetPtr(s));

	embMatMatchDel(&m);
    }

    ajStrDel(&s);
    ajStrDel(&lastnam);
    
    return;
}
