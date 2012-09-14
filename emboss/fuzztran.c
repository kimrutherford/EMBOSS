/* @source fuzztran application
**
** Finds fuzzy protein patterns in nucleic acid sequences
** @author Copyright (C) Alan Bleasby (ableasby@hgmp.mrc.ac.uk)
**
** Last modified by Jon Ison on Wed Jun 28 14:04:18 BST 2006
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


static void fuzztran_SourceFeature(const AjPFeattable thys, const AjPSeq pseq,
				   ajint start, ajint frame,
				   AjPFeattable sourcetab);




/* @prog fuzztran *************************************************************
**
** Protein pattern search after translation
**
******************************************************************************/

int main(int argc, char **argv)
{
    AjPSeqall seqall = NULL;
    AjPSeq seq = NULL;
    AjPFeattable tab = NULL;
    AjPFeattable seqtab = NULL;
    AjPReport report = NULL;
    AjPPatlistSeq plist = NULL;
    AjBool writeok = ajTrue;

    AjPSeq pseq = NULL;

    AjPStr gcode;
    AjPTrn trantable;
    AjPStr frame;
    AjPStr pro = NULL;
    ajint  table;
    ajint frameno;

    AjPStr text = NULL;

    ajint begin;
    ajint end;

    AjPStr tmpstr = NULL;

    embInit("fuzztran", argc, argv);

    seqall   = ajAcdGetSeqall("sequence");
    plist    = ajAcdGetPattern("pattern");
    report   = ajAcdGetReport("outfile");
    gcode    = ajAcdGetListSingle("table");
    frame    = ajAcdGetListSingle("frame");

    ajPatlistSeqDoc(plist, &tmpstr);
    ajFmtPrintAppS(&tmpstr, "TransTable: %S\n", ajAcdGetValue("table"));
    ajFmtPrintAppS(&tmpstr, "Frames: %S\n", ajAcdGetValue("frame"));
    ajReportSetHeaderS(report, tmpstr);

    ajStrToInt(gcode,&table);

    trantable = ajTrnNewI(table);

    writeok=ajTrue;
    while(writeok && ajSeqallNext(seqall,&seq))
    {
	begin = ajSeqallGetseqBegin(seqall);
	end   = ajSeqallGetseqEnd(seqall);
	ajStrAssignSubC(&text,ajSeqGetSeqC(seq),begin-1,end-1);
	ajStrFmtUpper(&text);
	seqtab = ajFeattableNewDna(ajSeqGetNameS(seq));

	if(ajStrMatchCaseC(frame,"F") || ajStrMatchCaseC(frame,"6"))
	{
	    tab = ajFeattableNewProt(ajSeqGetNameS(seq));
	    ajStrAssignC(&pro,"");
	    ajTrnSeqFrameS(trantable,text,1,&pro);
	    pseq = ajSeqNewNameS(pro, ajSeqGetNameS(seq));
	    embPatlistSeqSearchAll(tab,pseq,plist,ajFalse);
	    fuzztran_SourceFeature(tab, pseq, begin, 1, seqtab);
	    ajFeattableDel(&tab);
	    ajSeqDel(&pseq);

	    tab = ajFeattableNewProt(ajSeqGetNameS(seq));
	    ajStrAssignC(&pro,"");
	    ajTrnSeqFrameS(trantable,text,2,&pro);
	    pseq = ajSeqNewNameS(pro, ajSeqGetNameS(seq));
	    embPatlistSeqSearchAll(tab,pseq,plist,ajFalse);
	    fuzztran_SourceFeature(tab, pseq, begin, 2, seqtab);
	    ajFeattableDel(&tab);
	    ajSeqDel(&pseq);

	    tab = ajFeattableNewProt(ajSeqGetNameS(seq));
	    ajStrAssignC(&pro,"");
	    ajTrnSeqFrameS(trantable,text,3,&pro);
	    pseq = ajSeqNewNameS(pro, ajSeqGetNameS(seq));
	    embPatlistSeqSearchAll(tab,pseq,plist,ajFalse);
	    fuzztran_SourceFeature(tab, pseq, begin, 3, seqtab);
	    ajFeattableDel(&tab);
	    ajSeqDel(&pseq);
	}
	if(ajStrMatchCaseC(frame,"R") || ajStrMatchCaseC(frame,"6"))
	{
	    tab = ajFeattableNewProt(ajSeqGetNameS(seq));
	    ajStrAssignC(&pro,"");
	    ajTrnSeqFrameS(trantable,text,-1,&pro);
	    pseq = ajSeqNewNameS(pro, ajSeqGetNameS(seq));
	    embPatlistSeqSearchAll(tab,pseq,plist,ajFalse);
	    fuzztran_SourceFeature(tab, pseq, begin, -1, seqtab);
	    ajFeattableDel(&tab);
	    ajSeqDel(&pseq);

	    tab = ajFeattableNewProt(ajSeqGetNameS(seq));
	    ajStrAssignC(&pro,"");
	    ajTrnSeqFrameS(trantable,text,-2,&pro);
	    pseq = ajSeqNewNameS(pro, ajSeqGetNameS(seq));
	    embPatlistSeqSearchAll(tab,pseq,plist,ajFalse);
	    fuzztran_SourceFeature(tab, pseq, begin, -2, seqtab);
	    ajFeattableDel(&tab);
	    ajSeqDel(&pseq);

	    tab = ajFeattableNewProt(ajSeqGetNameS(seq));
	    ajStrAssignC(&pro,"");
	    ajTrnSeqFrameS(trantable,text,-3,&pro);
	    pseq = ajSeqNewNameS(pro, ajSeqGetNameS(seq));
	    embPatlistSeqSearchAll(tab,pseq,plist,ajFalse);
	    fuzztran_SourceFeature(tab, pseq, end, -3, seqtab);
	    ajFeattableDel(&tab);
	    ajSeqDel(&pseq);
	}
	ajStrToInt(frame,&frameno);
	if(frameno != 0 && frameno != 6)
	{
	    tab = ajFeattableNewProt(ajSeqGetNameS(seq));
	    ajStrAssignC(&pro,"");
	    ajTrnSeqFrameS(trantable,text,frameno,&pro);
	    pseq = ajSeqNewNameS(pro, ajSeqGetNameS(seq));
	    embPatlistSeqSearchAll(tab,pseq,plist,ajFalse);
	    fuzztran_SourceFeature(tab, pseq, begin, frameno, seqtab);
	    ajFeattableDel(&tab);
	    ajSeqDel(&pseq);
	}

	if(ajFeattableGetSize(seqtab))
	    writeok = ajReportWrite(report, seqtab, seq);
	ajStrDel(&text);
    }
    ajReportSetSeqstats(report, seqall);

    ajStrDel(&pro);
    ajStrDel(&text);
    ajStrDel(&tmpstr);

    ajStrDel(&gcode);
    ajStrDel(&frame);

    ajSeqallDel(&seqall);
    ajSeqDel(&seq);
    ajSeqDel(&pseq);

    ajReportClose(report);
    ajReportDel(&report);
    ajPatlistSeqDel(&plist);
    ajFeattableDel(&seqtab);
    ajFeattableDel(&tab);

    ajTrnDel(&trantable);
    embExit();

    return 0;
}




/* @funcstatic fuzztran_SourceFeature *****************************************
**
** Creates a dna source feature matching the all features in a
** protein fetaure table.
**
** @param [r] thys [const AjPFeattable] Protein feature table object
** @param [r] pseq [const AjPSeq] Protein sequence object
** @param [r] start [ajint] Start position in source sequence
**                          (base position of start of first codon,
**                          counted from the start of the sequence)
** @param [r] frame [ajint] Reading frame 1, 2 or 3 for forward and
**                          -1 -2 or -3 for reverse
** @param [u] sourcetab [AjPFeattable] Protein feature table object
** @return [void]
** @@
******************************************************************************/

static void fuzztran_SourceFeature(const AjPFeattable thys, const AjPSeq pseq,
				   ajint start, ajint frame,
				   AjPFeattable sourcetab)
{
    AjIList iter=NULL;
    AjPFeature protfeature = NULL;
    AjPFeature sourcefeature = NULL;
    char strand;
    ajint framenum;
    AjBool rev;
    ajint begin = 0;
    ajint end = 0;
    AjPStr fthit = NULL;
    AjPStr s = NULL;
    AjPStr t = NULL;
    AjPStr v = NULL;

    ajStrAssignC(&fthit, "hit");

    if(frame > 0)
    {
	strand = '+';
	framenum = frame;
	rev = ajFalse;
    }
    else
    {
	strand = '-';
	framenum = -frame;
	rev = ajTrue;
    }

    if(thys->Features)
    {
	iter = ajListIterNew(thys->Features) ;
	while(!ajListIterDone(iter))
	{
	    protfeature = (AjPFeature)ajListIterGet(iter);
	    if(rev)
	    {
		begin = start - framenum
		    - 3*protfeature->Start + 4;
		end = start - framenum
		    - 3*protfeature->End + 2;
	    }
	    else
	    {
		begin = start + framenum
		    + 3*protfeature->Start - 4;
		end = start + framenum
		    + 3*protfeature->End - 2;
	    }
	    sourcefeature = ajFeatNew(sourcetab, NULL, fthit, begin, end,
				      ajFeatGetScore(protfeature),
				      strand, framenum);
	    if(ajFeatGetNoteC(protfeature, "pat", &v))
	    {
		ajFmtPrintS(&s, "*pat %S", v);
		ajFeatTagAddSS(sourcefeature, NULL, s);
	    }
	    if(ajFeatGetNoteC(protfeature, "mismatch", &v))
	    {
		ajFmtPrintS(&s, "*mismatch %S", v);
		ajFeatTagAddSS(sourcefeature, NULL, s);
	    }
	    ajFmtPrintS(&s, "*frame %d", framenum);
	    ajFeatTagAddSS(sourcefeature, NULL, s);

	    ajFmtPrintS(&s, "*start %d", ajFeatGetStart(protfeature));
	    ajFeatTagAddSS(sourcefeature, NULL, s);

	    ajFmtPrintS(&s, "*end %d", ajFeatGetEnd(protfeature));
	    ajFeatTagAddSS(sourcefeature, NULL, s);

	    ajStrAssignSubS(&t,ajSeqGetSeqS(pseq),
			    ajFeatGetStart(protfeature)-1,
			    ajFeatGetEnd(protfeature)-1);
	    ajFmtPrintS(&s, "*translation %S", t);
	    ajFeatTagAddSS(sourcefeature, NULL, s);

	}
	ajListIterDel(&iter) ;
    }

    ajStrDel(&fthit);
    ajStrDel(&s);
    ajStrDel(&t);
    ajStrDel(&v);

    return;
}
