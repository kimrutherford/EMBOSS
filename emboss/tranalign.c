/* @source tranalign application
**
** Align nucleic sequences guided by the alignment of the translation
**
** @author Copyright (C) Gary Williams (gwilliam@hgmp.mrc.ac.uk)
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




static void tranalign_AddGaps (AjPSeq newseq,
			       const AjPSeq nseq, const AjPSeq pseq,
			       ajint npos);




/* @prog tranalign ************************************************************
**
** Align nucleic sequences guided by the alignment of the translation
**
******************************************************************************/

int main(int argc, char **argv)
{
    AjPSeqall nucseq;		/* input nucleic sequences */
    AjPSeqset protseq;		/* input aligned protein sequences */
    AjPSeqout seqout;
    AjPSeq nseq;		/* next nucleic sequence to align */
    const AjPSeq pseq;		/* next protein sequence use in alignment */
    AjPTrn trnTable;
    AjPSeq pep;			/* translation of nseq */
    AjPStr tablelist;
    ajint table;
    AjPSeqset outseqset;	/* set of aligned nucleic sequences */
    ajint proteinseqcount = 0;
    AjPStr degapstr = NULL;
    AjPStr degapstr2 = NULL;	/* used to check if it matches with START removed */
    AjPStr codon = NULL;	/* holds temporary codon to check if is START */
    char aa;			/* translated putative START codon */
    ajint type;			/* returned type of the putative START codon */
    ajint pos = 0;		/* start position of guide protein in translation */
    AjPSeq newseq = NULL;	/* output aligned nucleic sequence */
    ajint frame;

    embInit("tranalign", argc, argv);

    nucseq    = ajAcdGetSeqall("asequence");
    protseq   = ajAcdGetSeqset("bsequence");
    tablelist = ajAcdGetListSingle("table");
    seqout    = ajAcdGetSeqoutset("outseq");

    outseqset = ajSeqsetNew();
    degapstr  = ajStrNew();

    /* initialise the translation table */
    ajStrToInt(tablelist, &table);
    trnTable = ajTrnNewI(table);

    ajSeqsetFill(protseq);

    while(ajSeqallNext(nucseq, &nseq))
    {
    	if((pseq = ajSeqsetGetseqSeq(protseq, proteinseqcount++)) == NULL)
    	    ajErr("No guide protein sequence available for "
		  "nucleic sequence %S",
		  ajSeqGetNameS(nseq));

	ajDebug("Aligning %S and %S\n",
		ajSeqGetNameS(nseq), ajSeqGetNameS(pseq));

        /* get copy of pseq string with no gaps */
        ajStrAssignS(&degapstr, ajSeqGetSeqS(pseq));
        ajStrRemoveGap(&degapstr);

        /*
	** for each translation frame look for subset of pep that
	** matches pseq
	*/
        for(frame = 1; frame <4; frame++)
	{
	    ajDebug("trying frame %d\n", frame);
            pep = ajTrnSeqOrig(trnTable, nseq, frame);
            degapstr2 = ajStrNew();
            ajStrAssignRef(&degapstr2, degapstr);
            pos = ajStrFindCaseS(ajSeqGetSeqS(pep), degapstr);

            /* 
            ** we might have a START codon that should be translated as 'M'
            ** we need to check if there is a match after a possible START codon 
            */
            if(pos == -1 && ajStrGetLen(degapstr) > 1 && 
               (ajStrGetPtr(degapstr)[0] == 'M' || ajStrGetPtr(degapstr)[0] == 'm')) {
                /* see if pep minus the first character is a match */
                ajStrCutStart(&degapstr2, 1);
                pos = ajStrFindCaseS(ajSeqGetSeqS(pep), degapstr2); 
                /* pos is >= 1 if we have a match that is after the first residue */
                if (pos >= 1) {
                    /* point back at the putative START Methionine */
                    pos--;
                    /* test if first codon is a START */
                    codon = ajStrNew();
                    ajStrAssignSubS(&codon, ajSeqGetSeqS(nseq), 
                                (pos*3)+frame-1, (pos*3)+frame+2);
                    type = ajTrnStartStop(trnTable, codon, &aa);
                    if (type != 1) {
                        /* first codon is not a valid START, force a mismatch */
                        pos = -1;
                    }
                    ajStrDel(&codon);
                
            	} else {
                    /* force 'pos == 0' to be treated as a mismatch */
            	    pos = -1;
            	}
            }

            ajStrDel(&degapstr2);
            ajSeqDel(&pep);

            if(pos != -1)
            	break;
        }

        if(pos == -1)
	    ajErr("Guide protein sequence %S not found in nucleic sequence %S",
		  ajSeqGetNameS(pseq), ajSeqGetNameS(nseq));
	else
	{
	    ajDebug("got a match with frame=%d\n", frame);
            /* extract the coding region of nseq with gaps */
            newseq = ajSeqNew();
            ajSeqSetNuc(newseq);
            ajSeqAssignNameS(newseq, ajSeqGetNameS(nseq));
            ajSeqAssignDescS(newseq, ajSeqGetDescS(nseq));
            tranalign_AddGaps(newseq, nseq, pseq, (pos*3)+frame-1);

            /* output the gapped nucleic sequence */
            ajSeqsetApp(outseqset, newseq);

            ajSeqDel(&newseq);
        }

        ajStrRemoveWhiteExcess(&degapstr);
    }

    ajSeqoutWriteSet(seqout, outseqset);
    ajSeqoutClose(seqout);

    ajTrnDel(&trnTable);
    ajSeqsetDel(&outseqset);
    ajStrDel(&degapstr);
    ajStrDel(&degapstr2);

    ajSeqallDel(&nucseq);
    ajSeqDel(&nseq);
    ajSeqoutDel(&seqout);
    ajSeqsetDel(&protseq);
    ajStrDel(&tablelist);

    embExit();

    return 0;
}




/* @funcstatic tranalign_AddGaps **********************************************
**
** Adds bases or gaps to newseq from nseq guided by pseq
** starting at npos
**
** @param [u] newseq [AjPSeq] newseq
** @param [r] nseq [const AjPSeq] nseq
** @param [r] pseq [const AjPSeq] pseq
** @param [r] npos [ajint] nseq start pos
** @return [void]
** @@
******************************************************************************/

static void tranalign_AddGaps(AjPSeq newseq,
			      const AjPSeq nseq, const AjPSeq pseq,
			      ajint npos)
{

    AjPStr newstr = NULL;
    ajuint ppos = 0;

    newstr = ajStrNew();

    for(; ppos<ajSeqGetLen(pseq); ppos++)
    	if(ajSeqGetSeqC(pseq)[ppos] == '-')
    	    ajStrAppendC(&newstr, "---");
	else
	{
    	    ajStrAppendSubS(&newstr, ajSeqGetSeqS(nseq), npos, npos+2);
    	    npos+=3;
    	}

    ajDebug("aligned seq=%S\n", newstr);
    ajSeqAssignSeqS(newseq, newstr);

    ajStrDel(&newstr);

    return;
}
