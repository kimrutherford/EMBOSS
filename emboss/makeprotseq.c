/******************************************************************************
** @source makeprotseq.c application
**
** Create random sequence.
**
** @author Copyright (C) 2004 Henrikki Almusa, Medicel Oy
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


static AjPStr makeprotseq_random_sequence(AjPStr const * seqchar,
					   ajint scmax, ajint length);
static void makeprotseq_default_chars(AjPList* list);
static void makeprotseq_parse_pepstats(AjPList* list,AjPFile data);




/* @prog makeprotseq **********************************************************
**
** Creates a random protein sequence. Can use first protein distribution from
** pepstats output to specify sequence composition.
**
******************************************************************************/

int main(int argc, char **argv)
{
    AjPSeqout outseq = NULL;
    AjPList list     = NULL;
    AjPSeq seq       = NULL;
    AjPStr insert    = NULL;
    AjPStr seqstr    = NULL;
    AjPStr* seqr     = NULL;
    AjPFile data     = NULL;
    ajint start   = 0;
    ajint length  = 0;
    ajint amount  = 0;
    ajint scmax   = 0;
    ajint extra   = 0;

    embInit("makeprotseq", argc, argv);

    data     = ajAcdGetInfile("pepstatsfile");
    insert   = ajAcdGetString("insert");
    start    = ajAcdGetInt("start");
    length   = ajAcdGetInt("length");
    amount   = ajAcdGetInt("amount");
    outseq   = ajAcdGetSeqoutall("outseq");

    list = ajListstrNew();

    /* this is checked by acd
    if(amount <=0 || length <= 0)
	ajFatal("Amount or length is 0 or less. "
                 "Unable to create any sequences"); */

    /* if insert, make sure sequence is large enough */
    if(ajStrGetLen(insert))
    {
	length -= ajStrGetLen(insert);
	/* start= start <= 1 ? 0 : --start; */ /* checked in acd */
	start--;

	if(length <= 0)
	    ajFatal("Sequence smaller than inserted part. "
		     "Unable to create sequences.");
    }

    /* make the list of AjPStr to be used in sequence creation */
    if(data)
    {
	ajDebug("Distribution datafile '%s' given checking type\n",
		 ajFileGetPrintnameC(data));
	seqstr = ajStrNew();
	ajReadlineTrim(data,&seqstr);

	if(ajStrFindC(seqstr,"PEPSTATS") == 0)
	{
	    makeprotseq_parse_pepstats(&list,data);
	}
	else
	{
	    ajWarn("Not pepstats file. Making completely random sequences.");
	    makeprotseq_default_chars(&list);
	}

	ajStrDel(&seqstr);
	ajFileClose(&data);
    }
    else
	makeprotseq_default_chars(&list);

    /* if insert, make sure type is correct */
    /* typecheking code is not working, uncomment and test after it is
    if(ajStrGetLen(insert))
    {
	seqstr = ajStrNew();
	if(prot)
	    ajStrAssignC(&seqstr,"pureprotein");
	if(!ajSeqTypeCheckS(&insert,seqstr))
	    ajFatal("Insert not the same sequence type as sequence itself.");
	ajStrDel(&seqstr);
    } */

    /* array allows fast creation of a sequences */
    scmax = ajListstrToarray(list,&seqr);
    if(!scmax)
	ajFatal("No strings in list. No characters to make the sequence.");

    ajDebug("Distribution array done.\nscmax '%d', extra '%d', first '%S'\n",
	     scmax,extra,seqr[0]);

    ajRandomSeed();

    while(amount-- > 0)
    {
	seqstr = makeprotseq_random_sequence(seqr,scmax,length);

	if(ajStrGetLen(insert))
	    ajStrInsertS(&seqstr,start,insert);

	ajStrFmtLower(&seqstr);
	seq = ajSeqNew();

	ajSeqAssignSeqS(seq, seqstr);
	ajSeqSetProt(seq);

	ajSeqoutWriteSeq(outseq, seq);
	ajSeqDel(&seq);
	ajStrDel(&seqstr);
    }

    ajSeqoutClose(outseq);
    ajSeqoutDel(&outseq);
    ajListstrFreeData(&list);
    ajStrDel(&insert);
    AJFREE(seqr);

    embExit();

    return 0;
}




/* @funcstatic makeprotseq_random_sequence ************************************
**
** Creates string containing random sequence from given character distribution.
**
** @param [r] seqchar [AjPStr const *] Characters use to make sequence from
** @param [r] scmax [ajint] lenght of the seqchar string
** @param [r] length [ajint] Length of the wanted sequence
** @return [AjPStr] Sequence string
** @@
******************************************************************************/

static AjPStr makeprotseq_random_sequence(AjPStr const * seqchar,
                                          ajint scmax, ajint length)
{
    AjPStr seq = ajStrNew();
    ajint idx  = 0;
    ajint len  = length;

    while(len-- > 0)
    {
	idx = (ajint) (ajRandomDouble()*scmax);
	ajStrAppendS(&seq,seqchar[idx]);
    }

    return seq;
}




/* @funcstatic makeprotseq_default_chars **************************************
**
** Crates equal ditribution of characters for completely random sequences.
**
** @param [w] list [AjPList*] List with character distributions
** @return [void]
** @@
******************************************************************************/

static void makeprotseq_default_chars(AjPList* list)
{
    int i;
    int max;
    char *chars;
    char seqCharProtPure[]  = "ACDEFGHIKLMNPQRSTVWYacdefghiklmnpqrstvwy";
    int  seqCharProtPureLength = 40;
    AjPStr tmp;

    chars = seqCharProtPure;
    max = seqCharProtPureLength;

    for(i = 0; i < max; i++)
    {
	tmp = ajStrNew();
	tmp = ajFmtStr("%c",chars[i]);
	ajListstrPushAppend(*list,tmp);
    }

    return;
}




/* @funcstatic makeprotseq_parse_pepstats *************************************
**
** Parses the pepstats files first protein sequences amino acid distribution.
**
** @param [w] list [AjPList*] List with character distributions
** @param [u] data [AjPFile] Pepstats file
** @return [void]
** @@
******************************************************************************/

static void makeprotseq_parse_pepstats(AjPList* list,AjPFile data)
{
    AjPStr line = ajStrNew();
    AjPStr ch;
    AjPStr chcopy = NULL;
    const AjPStr tok;
    double value = 0.0;
    ajint count = 0;
    ajint i = 0;

    ajDebug("Parsing pepstats file.\n");

    /* skip the lines before residues */
    while(ajReadline(data,&line))
    {
	if(ajStrFindC(line,"Residue") == 0)
	    break;
    }

    /* parse residue part */
    while(ajReadlineTrim(data,&line))
    {
	if(ajStrParseCountC(line," \t") == 0)
	   break;

	ch = ajStrNew();
	tok = ajStrParseWhite(line);
	ajStrAppendS(&ch,tok);
	ajStrFmtLower(&ch);

	for(count = 1;count < 5;count++)
	    tok = ajStrParseWhite(NULL);

	ajStrToDouble(tok,&value);
	count = (ajint) (value * 100) +
	    ((value - (int) value ) >= 0.5 ? 1 : 0);

	for(i=0;i<count;i++)
	{
	    chcopy = ajStrNewS(ch);
	    ajListstrPush(*list,chcopy);
	}

	ajStrDel(&ch);
    }

    ajStrDel(&line);

    return;
}
