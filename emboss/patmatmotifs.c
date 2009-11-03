/* @source patmatmotifs.c
** @author Copyright (C) Sinead O'Leary (soleary@hgmp.mrc.ac.uk)
** @@
** Application for pattern matching, a sequence against a database of motifs.
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




/* @prog patmatmotifs *********************************************************
**
** Search a PROSITE motif database with a protein sequence
**
******************************************************************************/

int main(int argc, char **argv)
{

    AjPFile inf	 = NULL;
    AjPFile inf2 = NULL;
    AjPFeattable tab = NULL;
    AjPReport report = NULL;

    AjPSeq sequence = NULL;

    AjPStr redatanew = NULL;
    AjPStr str	     = NULL;
    AjPStr regexp    = NULL;
    AjPStr temp	     = NULL;
    AjPStr text      = NULL;
    AjPStr docdata   = NULL;
    AjPStr data      = NULL;
    AjPStr accession = NULL;
    AjPStr name      = NULL;
    EmbPPatMatch match = NULL;
    AjPStr savereg = NULL;
    AjPStr fthit   = NULL;

    AjBool full;
    AjBool prune;

    ajint i;
    ajint number;
    ajint start;
    ajint end;
    ajint length;
    ajint zstart;
    ajint zend;
    const char *p;
    ajint seqlength;
    AjPStr tmpstr  = NULL;
    AjPStr tailstr = NULL;
    AjPFeature gf;

    embInit("patmatmotifs", argc, argv);

    ajStrAssignC(&fthit, "SO:0001067");

    savereg   = ajStrNew();
    str       = ajStrNew();
    regexp    = ajStrNew();
    temp      = ajStrNew();
    data      = ajStrNew();
    accession = ajStrNew();
    text      = ajStrNew();
    name      = ajStrNew();

    sequence = ajAcdGetSeq("sequence");
    report   = ajAcdGetReport("outfile");
    full     = ajAcdGetBoolean("full");
    prune    = ajAcdGetBoolean("prune");

    ajSeqFmtUpper(sequence);		/* prosite regexs are all upper case */
    tab = ajFeattableNewSeq(sequence);
    ajStrAssignC(&tailstr, "");

    seqlength = ajStrGetLen(str);
    str       = ajSeqGetSeqCopyS(sequence);

    redatanew = ajStrNewC("PROSITE/prosite.lines");
    docdata   = ajStrNewC("PROSITE/");

    inf = ajDatafileNewInNameS(redatanew);
    if(!inf)
	ajFatal("Either EMBOSS_DATA undefined or PROSEXTRACT needs running");

    ajFmtPrintAppS(&tmpstr, "Full: %B\n", full);
    ajFmtPrintAppS(&tmpstr, "Prune: %B\n", prune);
    ajFmtPrintAppS(&tmpstr, "Data_file: %F\n", inf);
    ajReportSetHeader(report, tmpstr);

    while(ajReadlineTrim(inf, &regexp))
    {
	p=ajStrGetPtr(regexp);
	if(*p && *p!=' ' && *p!='^')
	{
	    p=ajSysFuncStrtok(p," ");
	    ajStrAssignC(&name,p);
	    if(prune)
		if(ajStrMatchCaseC(name,"myristyl") ||
		   ajStrMatchCaseC(name,"asn_glycosylation") ||
		   ajStrMatchCaseC(name,"camp_phospho_site") ||
		   ajStrMatchCaseC(name,"pkc_phospho_site") ||
		   ajStrMatchCaseC(name,"ck2_phospho_site") ||
		   ajStrMatchCaseC(name,"tyr_phospho_site"))
		{
		    for(i=0;i<4;++i)
			ajReadlineTrim(inf, &regexp);
		    continue;
		}
	    p=ajSysFuncStrtok(NULL," ");
	    ajStrAssignC(&accession,p);
	}

	if(ajStrPrefixC(regexp, "^"))
	{
	    p = ajStrGetPtr(regexp);

	    ajStrAssignC(&temp,p+1);
	    ajStrAssignC(&savereg,p+1);

	    match = embPatMatchFind(temp, str, ajFalse, ajFalse);
	    number = embPatMatchGetNumber(match);

	    for(i=0; i<number; i++)
	    {
		seqlength = ajStrGetLen(str);

		start = 1+embPatMatchGetStart(match, i);

		end = 1+embPatMatchGetEnd(match, i);

		length = embPatMatchGetLen(match, i);

		gf = ajFeatNew(tab, NULL, fthit, start, end,
			       (float) length, ' ', 0);

		ajFmtPrintS(&tmpstr, "*motif %S", name);
		ajFeatTagAdd(gf, NULL, tmpstr);

		if(start-5<0)
		    zstart = 0;
		else
		    zstart = start-5;

		if(end+5> seqlength)
		    zend = end;
		else
		    zend = end+5;


		ajStrAssignSubS(&temp, str, zstart, zend);
	    }


	    if(full && number)
	    {
		ajStrAssignC(&redatanew,ajStrGetPtr(docdata));
		ajStrAppendC(&redatanew,ajStrGetPtr(accession));
		inf2 = ajDatafileNewInNameS(redatanew);
		if(!inf2)
		    continue;

		/*
		** Insert Prosite documentation from files made by
		** prosextract.c
		*/
		ajFmtPrintAppS(&tailstr, "Motif: %S\n", name);
		ajFmtPrintAppS(&tailstr, "Count: %d\n\n", number);
		while(ajReadlineTrim(inf2, &text))
		    ajFmtPrintAppS(&tailstr, "%S\n", text);

		ajFmtPrintAppS(&tailstr, "\n***************\n\n");
		ajFileClose(&inf2);

	    }
	    embPatMatchDel(&match);
	}
    }

    ajReportSetTail(report,tailstr);
    ajReportWrite(report, tab, sequence);

    ajReportDel(&report);
    ajFeattableDel(&tab);

    ajStrDel(&temp);
    ajStrDel(&regexp);
    ajStrDel(&savereg);
    ajStrDel(&str);
    ajStrDel(&data);
    ajStrDel(&docdata);
    ajStrDel(&text);
    ajStrDel(&redatanew);
    ajStrDel(&accession);
    ajSeqDel(&sequence);
    ajStrDel(&tailstr);
    ajStrDel(&fthit);
    ajStrDel(&name);
    ajStrDel(&tmpstr);

    ajFeattableDel(&tab);
    ajFileClose(&inf);

    embExit();

    return 0;
}
