/* @source transeq application
**
** Translate nucleic acid sequences
**
** @author Copyright (C) Gary Williams (gwilliam@hgmp.mrc.ac.uk)
** Mar  4 17:18 1999 (ajb)
** Jul 19 19:24 2000 (ajb)
** Jun 29 16:50 2001 (gww) use new version of ajTrnSeqOrig
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




static void transeq_Trim(AjPSeq seq);
static void transeq_GetFrames(AjPStr const *framelist, AjBool *frames);
static void transeq_Clean(AjPSeq seq);




/* @prog transeq **************************************************************
**
** Translate nucleic acid sequences
**
******************************************************************************/

int main(int argc, char **argv)
{
    AjPSeqall seqall;
    AjPSeqout seqout;
    AjPSeq seq;
    AjPTrn trnTable;
    AjPSeq pep;
    AjPStr *framelist;
    AjBool frames[6];	/* frames to be translated 1 to 3, -1 to -3 */
    AjPStr tablename;
    ajint table;
    AjPRange regions;
    AjPRange seqregions;
    AjBool trim;
    AjBool clean;
    AjBool defr  = ajFalse; /* true if the range covers the whole sequence */
    AjBool alternate;

    int i;

    embInit("transeq", argc, argv);

    seqout    = ajAcdGetSeqoutall("outseq");
    seqall    = ajAcdGetSeqall("sequence");
    framelist = ajAcdGetList("frame");
    tablename = ajAcdGetListSingle("table");
    regions   = ajAcdGetRange("regions");
    trim      = ajAcdGetBoolean("trim");
    clean     = ajAcdGetBoolean("clean");
    alternate = ajAcdGetBoolean("alternative");

    /* get the frames to be translated */
    transeq_GetFrames(framelist, frames);

    /* initialise the translation table */
    ajStrToInt(tablename, &table);
    trnTable = ajTrnNewI(table);

    /* shift values of translate region to match -sbegin=n parameter */
    /*  ajRangeBegin(regions, ajSeqallGetseqBegin(seqall));*/


    while(ajSeqallNext(seqall, &seq))
    {
	ajSeqTrim(seq);

	seqregions = ajRangeCopy(regions);
        defr = ajRangeDefault(seqregions, seq);

	/* get regions to translate */
	if(!defr)
	    ajRangeSeqExtract(seqregions, seq);

        for(i=0; i<6; i++)
	{
            ajDebug("try frame: %d\n", i);
            if(frames[i])
	    {
                if(i<3)
	            pep = ajTrnSeqOrig(trnTable, seq, i+1);
	        else
		    if(alternate) /* frame -1 uses codons starting at end */
			pep = ajTrnSeqOrig(trnTable, seq, -i-1);
		    else	/* frame -1 uses frame 1 codons */
	              pep = ajTrnSeqOrig(trnTable, seq, 2-i);

	        if(trim)
	            transeq_Trim(pep);

                if(clean)
                    transeq_Clean(pep); /* clean after the trim */

	        ajSeqoutWriteSeq(seqout, pep);
	        ajSeqDel(&pep);
	    }
	}
	ajRangeDel(&seqregions);
    }

    ajSeqoutClose(seqout);

    ajTrnDel(&trnTable);
    ajSeqallDel(&seqall);
    ajSeqDel(&seq);
    ajSeqDel(&pep);
    ajStrDelarray(&framelist);
    ajStrDel(&tablename);
    ajSeqoutDel(&seqout);
    ajRangeDel(&regions);
    ajRangeDel(&seqregions);

    embExit();
    return 0;
}





/* @funcstatic transeq_Trim ***************************************************
**
** Removes X, and/or * characters from the end of the translation
**
** @param [u] seq [AjPSeq] sequence to trim
** @return [void]
** @@
******************************************************************************/

static void transeq_Trim(AjPSeq seq)
{
    AjPStr s;
    char *p;
    char c;
    ajint i;
    ajint len;

    s = ajSeqGetSeqCopyS(seq);
    p = ajStrGetuniquePtr(&s);
    
    len = ajStrGetLen(s)-1;

    for(i=len; i>=0; i--)
    {
	c = *(p+i);

	if(c != 'X' && c != '*' )
	    break;
    }

    if(i < len)
	ajStrTruncateLen(&s, i+1);
    ajSeqAssignSeqS(seq, s);

    return;
}




/* @funcstatic transeq_Clean *************************************************
**
** Converts * characters to X's in the translation
**
**
** @param [u] seq [AjPSeq] sequence to clean
** @return [void]
** @@
******************************************************************************/

static void transeq_Clean(AjPSeq seq)
{
    AjPStr str;

    str = ajSeqGetSeqCopyS(seq);

    ajStrExchangeSetCC(&str, "*", "X");
    ajSeqAssignSeqS(seq, str);

    return;
}




/* @funcstatic transeq_GetFrames **********************************************
**
** Converts the list of frame numbers into a boolean vector.
** Frame numbers are ordered in the vector as:
** 1, 2, 3 -1, -2, -3
**
** @param [r] framelist [AjPStr const *] list of frame numbers
** @param [w] frames [AjBool*] Boolean vector
** @return [void]
** @@
******************************************************************************/

static void transeq_GetFrames(AjPStr const *framelist, AjBool *frames)
{
    int i;

    /* reset the vector */
    for(i=0; i<6; i++)
        frames[i] = ajFalse;


    for(i=0; framelist[i]; i++)
    {
        if(ajStrMatchC(framelist[i], "1"))
            frames[0] = ajTrue;
	else if(ajStrMatchC(framelist[i], "2"))
            frames[1] = ajTrue;
	else if(ajStrMatchC(framelist[i], "3"))
            frames[2] = ajTrue;
	else if(ajStrMatchC(framelist[i], "-1"))
            frames[3] = ajTrue;
	else if(ajStrMatchC(framelist[i], "-2"))
            frames[4] = ajTrue;
	else if(ajStrMatchC(framelist[i], "-3"))
            frames[5] = ajTrue;
	else if(ajStrMatchC(framelist[i], "F"))
	{
            frames[0] = ajTrue;
            frames[1] = ajTrue;
            frames[2] = ajTrue;
        }
	else if(ajStrMatchC(framelist[i], "R"))
	{
            frames[3] = ajTrue;
            frames[4] = ajTrue;
            frames[5] = ajTrue;
        }
	else if(ajStrMatchC(framelist[i], "6"))
	{
            frames[0] = ajTrue;
            frames[1] = ajTrue;
            frames[2] = ajTrue;
            frames[3] = ajTrue;
            frames[4] = ajTrue;
            frames[5] = ajTrue;
	}
	else
	    ajErr("Unknown frame: '%S'", framelist[i]);
    }

    return;
}
