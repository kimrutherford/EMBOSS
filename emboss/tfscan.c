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




static void tfscan_print_hits(AjPList *l, ajint hits,
			      AjPReport outf,
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
    AjPReport outf = NULL;
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
    outf       = ajAcdGetReport("outfile");
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
	atable = ajTablestrNew(1000);
	btable = ajTablestrNew(1000);
	
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
	    ajStrAssignC(&bf,p); /* rest of line */
	    
	    v = embPatVariablePattern(pattern,substr,pname,l,0,
				      mismatch,begin);
	    if(v)
	    {
		key = ajStrNewS(pname);
		value = ajStrNewS(acc);
		ajTablePut(atable,(void *)key,(void *)value);
		key = ajStrNewS(pname);
		value = ajStrNewS(bf);
		ajTablePut(btable,(void *)key,(void *)value);
	    }
	    sum += v;
	}

	if(sum)
	    tfscan_print_hits(&l,sum,outf,atable,seq,minlength,
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
    ajReportClose(outf);
    ajReportDel(&outf);

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
** @param [w] l [AjPList*] list of hits
** @param [r] hits [ajint] number of hits
** @param [u] outf [AjPReport] output report
** @param [r] t [const AjPTable] table of accession numbers
** @param [r] seq [const AjPSeq] test sequence
** @param [r] minlength [ajuint] minimum length of pattern
** @param [r] btable [const AjPTable] BF lines from transfac (if any)
** @@
******************************************************************************/

static void tfscan_print_hits(AjPList *l,
			      ajint hits, AjPReport outf,
			      const AjPTable t,
			      const AjPSeq seq, ajuint minlength,
			      const  AjPTable btable)
{
    ajint i;
    EmbPMatMatch m;
    const AjPStr acc = NULL;
    const AjPStr bf  = NULL;
    AjPFeattable ftable = NULL;
    AjPFeature gf  = NULL;
    AjPStr type = ajStrNewC("SO:0000235");
    AjPStr tmpstr = NULL;

    ftable = ajFeattableNewSeq(seq);

    /*ajFmtPrintF(outf,"TFSCAN of %s from %d to %d\n\n",ajStrGetPtr(name),
      begin,end);*/

    for(i=0;i<hits;++i)
    {
	ajListPop(*l,(void **)&m);
	acc = ajTableFetchS(t, m->seqname);

	if(m->len >= minlength)
        {
	    /*ajFmtPrintF(outf,"%-20s %-8s %-5d %-5d %s\n",
                        ajStrGetPtr(m->seqname),
			ajStrGetPtr(acc),m->start,
			m->start+m->len-1,ajStrGetPtr(s));*/
            if(m->forward)
                gf = ajFeatNew(ftable, ajUtilGetProgram(), type,
                               m->start,
                               m->start+m->len-1,
                               (float) m->score,
                               '+',0);
            else
                gf = ajFeatNew(ftable, ajUtilGetProgram(), type,
                               m->start,
                               m->start+m->len-1,
                               (float) m->score,
                               '-',0);
            ajFmtPrintS(&tmpstr, "*acc %S", acc);
            ajFeatTagAdd(gf, NULL, tmpstr);
            bf  = ajTableFetchS(btable, m->seqname);
            ajFmtPrintS(&tmpstr, "*factor %S", bf);
            ajFeatTagAdd(gf, NULL, tmpstr);
        }

	embMatMatchDel(&m);
    }

    ajReportWrite(outf, ftable, seq);
    ajFeattableDel(&ftable);
    ajStrDel(&tmpstr);
    ajStrDel(&type);

    return;
}
