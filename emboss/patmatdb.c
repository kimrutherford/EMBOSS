/* @source patmatDB.c
** @author Copyright (C) Sinead O'Leary (soleary@hgmp.mrc.ac.uk)
** @@
**
** Application for pattern matching, one Prosite motif against a
** sequence database.
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




/* @prog patmatdb *************************************************************
**
** Search a protein sequence with a motif
**
******************************************************************************/

int main(int argc, char **argv)
{
    AjPFeattable tab = NULL;
    AjPReport report = NULL;

    AjPSeqall seqall = NULL;
    AjPSeq seq	     = NULL;
    AjPStr str 	     = NULL;
    AjPStr regexp    = NULL;
    AjPStr motif     = NULL;
    AjPStr temp      = NULL;

    AjPStr regexpdata  = NULL;
    EmbPPatMatch match = NULL;


    ajint i;
    ajint number;
    ajint start;
    ajint end;
    ajint length;
    ajint zstart;
    ajint zend;
    ajint seqlength;
    AjPStr tmpstr = NULL;
    AjPStr fthit = NULL;

    embInit("patmatdb", argc, argv);

    seqall = ajAcdGetSeqall("sequence");
    motif  = ajAcdGetString("motif");
    report = ajAcdGetReport("outfile");

    temp=ajStrNew();
    ajStrFmtUpper(&motif);
    ajStrAssignC(&fthit, "SO:0001067");

    /*converting the Prosite motif to a reg exps */
    regexp =embPatPrositeToRegExp(motif);

    ajFmtPrintAppS(&tmpstr, "Motif: %S\n", motif);
    ajReportSetHeaderS(report, tmpstr);

    while(ajSeqallNext(seqall, &seq))
    {
	str = ajSeqGetSeqCopyS(seq);
	ajStrFmtUpper(&str);

	/* comparing the reg exps to sequence for matches. */
	match 	= embPatMatchFind(regexp, str, ajFalse, ajFalse);

	/*returns the number of posix matches in the structure. */
	number 	= embPatMatchGetNumber(match);

	if(number)
	  tab = ajFeattableNewSeq(seq);

	for(i=0; i<number; i++)
	{
	    seqlength = ajStrGetLen(str);

	    /*returns length from pattern match for index'th item. */
	    length = embPatMatchGetLen(match, i);

	    /*
	    ** returns the start position from the pattern match for the
            ** index'th item.
	    */
	    start = 1+embPatMatchGetStart(match, i);

	    /*
	    ** returns the end point for the pattern match for the
	    ** index'th item.
	    */
	    end	= 1+embPatMatchGetEnd(match, i);

	    ajFeatNewProt(tab, NULL, fthit, start, end,
			  (float) length);

	    if(start-5<0)
		zstart = 0;
	    else
		zstart = start-6;

	    if(end+4> seqlength)
		zend = end;
	    else
		zend = end+4;

	    ajStrAssignSubS(&temp, str, zstart, zend);
	}

	if(number)
        {
	    ajReportWrite(report, tab, seq);
	    ajFeattableDel(&tab);
	}
	embPatMatchDel(&match);
	ajStrDel(&str);
    }
    ajReportSetSeqstats(report, seqall);

    ajStrDel(&regexp);
    ajStrDel(&temp);
    ajStrDel(&motif);
    ajStrDel(&str);
    ajStrDel(&regexpdata);

    ajReportClose(report);
    ajReportDel(&report);

    ajSeqallDel(&seqall);
    ajSeqDel(&seq);
    ajStrDel(&fthit);
    ajStrDel(&tmpstr);

    embExit();

    return 0;
}
