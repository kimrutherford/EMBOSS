/* @source msbar application
**
** Mutate sequence beyond all recognition
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
#include <ctype.h>	/* for tolower, toupper */




static void msbar_blockmutn(AjPStr *str, AjBool isnuc,
			    AjPStr const *blocklist,
			    ajint min, ajint max, AjBool inframe);
static void msbar_codonmutn(AjPStr *str, AjBool isnuc,
			    AjPStr const *codonlist,
			    AjBool inframe);
static void msbar_pointmutn(AjPStr *str, AjBool isnuc,
			    AjPStr const *pointlist);
static void msbar_Insert(AjPStr *str, AjBool isnuc, ajint start, ajint end);
static void msbar_Move(AjPStr *str, ajint start, ajint end, ajint destination);
static void msbar_Duplicate(AjPStr *str, ajint start, ajint end);
static AjBool msbar_Unlike(const AjPStr str, AjPSeqall other);




/* @prog msbar ****************************************************************
**
** Mutate sequence beyond all recognition
**
******************************************************************************/

int main(int argc, char **argv)
{

    AjPSeqall seqall;
    AjPSeqall other;
    AjPSeqout seqout;
    AjPSeq seq = NULL;

    AjBool isnuc;
    AjPStr str = NULL;

    AjPStr *pointlist;
    AjPStr *codonlist;
    AjPStr *blocklist;

    ajint i;
    ajint count;
    ajint min;
    ajint max;
    AjBool inframe;

    /*
    ** the number of times to try to produce a sequence that is different to
    **  the 'other' sequences before we give up and output it anyway
    */
    ajint attempts = 10;

    /* count of tries to get a unique sequence */
    ajint try;

    embInit("msbar", argc, argv);

    seqall = ajAcdGetSeqall("sequence");
    seqout = ajAcdGetSeqoutall("outseq");

    pointlist = ajAcdGetList("point");
    codonlist = ajAcdGetList("codon");
    blocklist = ajAcdGetList("block");

    count   = ajAcdGetInt("count");
    inframe = ajAcdGetBool("inframe");
    min     = ajAcdGetInt("minimum");
    max     = ajAcdGetInt("maximum");
    other   = ajAcdGetSeqall("othersequence");


    str = ajStrNew();

    /* seed the random number generator */
    ajRandomSeed();

    while(ajSeqallNext(seqall, &seq))
    {
        ajSeqTrim(seq);
        
	/* is this a protein or nucleic sequence? */
	isnuc = ajSeqIsNuc(seq);

        /*
	** try mutating until there is a result that is not the same as
	** the 'other' sequences
	*/
        for(try=0; try<attempts; try++)
	{
	    /* get a copy of the sequence string */
	    ajStrAssignS(&str, ajSeqGetSeqS(seq));

	    /* do the mutation operations */
	    for(i=0; i<count; i++)
	    {
	        msbar_blockmutn(&str, isnuc, blocklist, min, max, inframe);
	        msbar_codonmutn(&str, isnuc, codonlist, inframe);
	        msbar_pointmutn(&str, isnuc, pointlist);
	    }

            /* Is this mutated sequence different to the 'other' sequences?  */
	    if(msbar_Unlike(str, other))
	        break;
	}

	if(try >= attempts)
	    ajWarn("No unique mutation found after %d attempts", attempts);

	ajSeqAssignSeqS(seq, str);
	ajSeqoutWriteSeq(seqout, seq);
    }

    ajSeqoutClose(seqout);

    ajStrDel(&str);

    ajSeqallDel(&seqall);
    ajSeqallDel(&other);
    ajSeqDel(&seq);
    ajSeqoutDel(&seqout);
    ajStrDelarray(&pointlist);
    ajStrDelarray(&codonlist);
    ajStrDelarray(&blocklist);

    embExit();

    return 0;
}




/* @funcstatic msbar_blockmutn ************************************************
**
** Mutate a random block of sequence
**
** @param [u] str [AjPStr*] sequence to mutate
** @param [r] isnuc [AjBool] TRUE if sequence is nucleic
** @param [r] blocklist [AjPStr const *] Types of block mutations to perform
** @param [r] min [ajint] minimum size of block
** @param [r] max [ajint] maximum size of block
** @param [r] inframe [AjBool] mutate blocks preserving codon frame if TRUE
** @@
******************************************************************************/

static void msbar_blockmutn(AjPStr *str, AjBool isnuc, AjPStr const *blocklist,
			    ajint min, ajint max, AjBool inframe)
{
    ajint i = -1;
    ajint rposstart;
    ajint rposend;
    ajint rpos2;
    ajint opt;
    double xlen;
    double xdiff;
    double td;
    
    while(blocklist[++i])
    {
	ajDebug("Next block mutation operation = %S\n", blocklist[i]);

	/* None */
	if(!ajStrCmpC(blocklist[i], "0"))
	    return;

	/* get the option value */
	ajStrToInt(blocklist[i], &opt);

	/* if 'Any' required, then choose which one */
	if(opt == 1)
	{
	    td = ajRandomNumberD() * 5.;
	    opt = (ajint) td;
	    opt += 2;
	}

	/*
	**  get random block start and end positions in the sequence
	**  (0 to ajStrLen - 1)
	*/
	if(inframe)
	{
	    if(min < 3) min = 3;
	    xlen = ajStrGetLen(*str);
	    td = ajRandomNumberD() * xlen/3.0;
	    rposstart = (ajint) td;
	    xdiff = max - min;
	    td = ajRandomNumberD() * xdiff/3.0;
	    rposend = rposstart + (min/3) + (ajint)td;
	    rposstart *= 3;
	    rposend *= 3;
	    rposend--;
	}
	else
	{
	    xlen = ajStrGetLen(*str);
	    td = ajRandomNumberD() * xlen;
	    rposstart = (ajint) td;
	    xdiff = max - min;
	    td = ajRandomNumberD() * xdiff;
	    rposend = rposstart + min + (ajint) td;
	}


	if(opt == 2)
	{
	    /* Insert */
	    ajDebug("block insert from %d to %d\n", rposstart, rposend);
	    msbar_Insert(str, isnuc, rposstart, rposend);
	}

	if(opt == 3)
	{
	    /* Delete */
	    ajDebug("block deletion from %d to %d\n", rposstart,
			   rposend);
	    ajStrCutRange(str, rposstart, rposend);
	}

	if(opt == 4)
	{
	    /* Change */
	    ajDebug("block change from %d to %d\n", rposstart, rposend);
	    ajStrCutRange(str, rposstart, rposend);
	    msbar_Insert(str, isnuc, rposstart, rposend);
	}

	if(opt == 5)
	{
	    /* Duplication */
	    ajDebug("block duplication from %d to %d\n", rposstart,
			   rposend);
	    msbar_Duplicate(str, rposstart, rposend);
	}

	if(opt == 6)
	{
	    /* Move */
	    if(inframe)
	    {
		xlen = ajStrGetLen(*str);
		td = ajRandomNumberD() * xlen/3.0;
		rpos2 = (ajint) td;
		rpos2 *= 3;
	    }
	    else
	    {
		xlen = ajStrGetLen(*str);
		td = ajRandomNumberD() * xlen;
		rpos2 = (ajint) td;
	    }
	    ajDebug("block move from %d to %d to position %d\n",
			   rposstart, rposend, rpos2);
	    msbar_Move(str, rposstart, rposend, rpos2);
	}
    }

    return;
}




/* @funcstatic msbar_codonmutn ************************************************
**
** Mutate codons
**
** @param [u] str [AjPStr*] Sequence to mutate
** @param [r] isnuc [AjBool] TRUE if sequence is nucleic
** @param [r] codonlist [AjPStr const *] Types of codon mutations to perform
** @param [r] inframe [AjBool] mutate blocks preserving codon frame if TRUE
** @@
******************************************************************************/

static void msbar_codonmutn(AjPStr *str, AjBool isnuc, AjPStr const *codonlist,
			    AjBool inframe)
{
    ajint rpos;
    ajint rpos2;
    ajint i = -1;
    ajint opt;
    double xlen;
    double td;
    
    while(codonlist[++i])
    {
	ajDebug("Next codon mutation operation = %S\n", codonlist[i]);

	/* None */
	if(!ajStrCmpC(codonlist[i], "0"))
	    return;

	/* get the option value */
	ajStrToInt(codonlist[i], &opt);

	/* if 'Any' required, then choose which one */
	if(opt == 1)
	{
	    td = ajRandomNumberD() * 5.;
	    opt = (ajint) td;
	    opt += 2;
	}


	/* get a random position in the sequence (0 to ajStrLen - 1) */
	if(inframe)
	{
	    xlen = ajStrGetLen(*str);
	    td = ajRandomNumberD() * xlen/3.0;
	    rpos = (ajint) td;
	    rpos *= 3;
	}
	else
	{
	    xlen = ajStrGetLen(*str);
	    td = ajRandomNumberD() * xlen;
	    rpos = (ajint) td;
	}

	if(opt == 2)
	{
	    /* Insert */
	    ajDebug("codon insert at %d\n", rpos);
	    msbar_Insert(str, isnuc, rpos, rpos+2);
	}

	if(opt == 3)
	{
	    /* Delete */
	    ajDebug("codon deletion at %d\n", rpos);
	    ajStrCutRange(str, rpos, rpos+2);
	}

	if(opt == 4)
	{
	    /* Change */
	    ajDebug("codon change at %d\n", rpos);
	    ajStrCutRange(str, rpos, rpos+2);
	    msbar_Insert(str, isnuc, rpos, rpos+2);
	}

	if(opt == 5)
	{
	    /* Duplication */
	    ajDebug("codon duplication at %d\n", rpos);
	    msbar_Duplicate(str, rpos, rpos+2);

	}

	if(opt == 6)
	{
	    /* Move */
	    if(inframe)
	    {
		xlen = ajStrGetLen(*str);
		td = ajRandomNumberD() * xlen/3.0;
		rpos2 = (ajint) td;
		rpos2 *= 3;
	    }
	    else
	    {
		xlen = ajStrGetLen(*str);
		td = ajRandomNumberD() * xlen;
		rpos2 = (ajint) td;
	    }
	    ajDebug("codon move from %d to %d\n", rpos, rpos2);
	    msbar_Move(str, rpos, rpos+2, rpos2);
	}


    }

    return;
}




/* @funcstatic msbar_pointmutn ************************************************
**
** Mutate random single points
**
** @param [u] str [AjPStr*] sequence to mutate
** @param [r] isnuc [AjBool] TRUE if sequence is nucleic
** @param [r] pointlist [AjPStr const *] Types of point mutations to perform
** @@
******************************************************************************/

static void msbar_pointmutn(AjPStr *str, AjBool isnuc, AjPStr const *pointlist)
{
    ajint i = -1;
    ajint rpos, rpos2;
    ajint opt;
    double xlen;
    double td;
    
    while(pointlist[++i])
    {
	ajDebug("Next point mutation operation = %S\n", pointlist[i]);
	/* None */
	if(!ajStrCmpC(pointlist[i], "0"))
	    return;

	/* get the option value */
	ajStrToInt(pointlist[i], &opt);

	/* if 'Any' required, then choose which one */
	if(opt == 1)
	{
	    td = ajRandomNumberD() * 5.;
	    opt = (ajint) td;
	    opt += 2;
	}

	/* get a random position in the sequence (0 to ajStrLen - 1) */
	xlen = ajStrGetLen(*str);
	td = ajRandomNumberD() * xlen;
	rpos = (ajint) td;
	
	if(opt == 2)
	{
	    /* Insert */
	    ajDebug("Point insert at %d\n", rpos);
	    msbar_Insert(str, isnuc, rpos, rpos);

	}

	if(opt == 3)
	{
	    /* Delete */
	    ajDebug("Point deletion at %d\n", rpos);
	    ajStrCutRange(str, rpos, rpos);

	}

	if(opt == 4)
	{
	    /* Change */
	    ajDebug("Point change at %d\n", rpos);
	    ajStrCutRange(str, rpos, rpos);
	    msbar_Insert(str, isnuc, rpos, rpos);

	}

	if(opt == 5)
	{
	    /* Duplication */
	    ajDebug("Point duplication at %d\n", rpos);
	    msbar_Duplicate(str, rpos, rpos);

	}

	if(opt == 6)
	{
	    /* Move */
	    xlen = ajStrGetLen(*str);
	    td = ajRandomNumberD() * xlen;
	    rpos2 = (ajint) td;
	    ajDebug("Point move from %d to %d\n", rpos, rpos2);
	    msbar_Move(str, rpos, rpos, rpos2);
	}
    }

    return;
}




/* @funcstatic msbar_Insert ***************************************************
**
** Insert random sequence at a position in the main sequence
**
** @param [u] str [AjPStr*] sequence to insert into
** @param [r] isnuc [AjBool] TRUE if sequence is nucleic
** @param [r] start [ajint] start position of insert
** @param [r] end [ajint] end of insert
** @@
******************************************************************************/

static void msbar_Insert(AjPStr *str, AjBool isnuc, ajint start, ajint end)
{
    char nuc[] = "ACGT";
    char prot[] = "ARNDCQEGHILKMFPSTWYV";
    AjPStr ins = NULL;
    ajint count;
    ajint r;
    double xlen;
    double td;
    
    count = end - start +1;

    while(count--)
    {
	if(isnuc)
	{
	    xlen = strlen(nuc);
	    td = ajRandomNumberD() * xlen;
	    r = (ajint) td;
	    ajStrAppendK(&ins, nuc[r]);
	}
	else
	{
	    xlen = strlen(prot);
	    td = ajRandomNumberD() * xlen;
	    r = (ajint) td;
	    ajStrAppendK(&ins, prot[r]);
	}
    }
    ajDebug("Inserting %S at %d\n", ins, start);
    ajStrInsertS(str, start, ins);
    ajStrDel(&ins);

    return;
}




/* @funcstatic msbar_Move *****************************************************
**
** Move a block of sequence from one position to another
**
** @param [u] str [AjPStr*] sequence to move within
** @param [r] start [ajint] start position of block to move
** @param [r] end [ajint] end position of block to move
** @param [r] destination [ajint] destination of move
** @@
******************************************************************************/

static void msbar_Move(AjPStr *str, ajint start, ajint end, ajint destination)
{
    AjPStr mov = NULL;

    ajStrAssignS(&mov, *str);
    ajStrKeepRange(&mov, start, end);
    ajStrInsertS(str, destination, mov);
    ajStrDel(&mov);

    return;
}




/* @funcstatic msbar_Duplicate ************************************************
**
** Duplicate a block of sequence adjacent to the source block of sequence
**
** @param [u] str [AjPStr*] sequence to duplicate within
** @param [r] start [ajint] start position of block to duplicate
** @param [r] end [ajint] end position of block to duplicate
** @@
******************************************************************************/

static void msbar_Duplicate(AjPStr *str, ajint start, ajint end)
{
    msbar_Move(str, start, end, start);

    return;
}




/* @funcstatic msbar_Unlike ************************************************
**
** Check that the sequence is unlike any other input sequence
**
** @param [r] str [const AjPStr] sequence to check
** @param [u] other [AjPSeqall] set of sequences to check against
** @return [AjBool] True if unlike any other sequence
** @@
******************************************************************************/

static AjBool msbar_Unlike(const AjPStr str, AjPSeqall other)
{
    AjPSeq nextother;

    while(ajSeqallNext(other, &nextother))
        if(ajStrMatchCaseS(str, ajSeqGetSeqS(nextother)))
            return ajFalse;
    
    return ajTrue;
}
