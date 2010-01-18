/* @source checktrans application
**
** Check translations made with transeq (document these translations)
**
** @author Copyright (C) Rodrigo Lopez & Alan Bleasby
** @@
** Adapted from work done by Alan Bleasy
** Modified by Gary Williams 19 April 2000 to remove output to STDOUT and to
**	write ORF sequences to a single file instead of many individual ones.
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
#include <math.h>
#include <stdlib.h>




static void checktrans_findorfs(AjPSeqout outseq, AjPFile outf, ajint s,
				ajint len, const char *seq, const char *name,
				ajint orfml, AjBool addedasterisk);

static void checktrans_ajbseq(AjPSeqout outseq, const char *seq,
			      ajint begin, int end, 
			      const char *name, ajint count);

static void checktrans_dumptofeat(AjPFeattabOut featout, ajint from, ajint to,
				  const char *p, const char *seqname,
				  ajint min_orflength);




/* @prog checktrans ***********************************************************
**
** Reports STOP codons and ORF statistics of a protein sequence
**
******************************************************************************/

int main(int argc, char **argv)
{
    AjPSeqall seqall;
    AjPSeq    seq    = NULL;
    AjPFile   outf   = NULL;
    const AjPStr    strand = NULL;
    AjPStr    substr = NULL;
    AjPSeqout outseq = NULL;

    AjPFeattabOut featout = NULL;
    AjBool addlast;

    ajint begin;
    ajint end;
    ajint len;
    ajint orfml;
    AjBool addedasterisk = ajFalse;

    embInit("checktrans",argc,argv);

    seqall  = ajAcdGetSeqall("sequence");
    outf    = ajAcdGetOutfile("outfile");
    orfml   = ajAcdGetInt("orfml");
    outseq  = ajAcdGetSeqoutall("outseq");
    featout = ajAcdGetFeatout("outfeat");
    addlast = ajAcdGetBoolean("addlast");

    substr    = ajStrNew();


    while(ajSeqallNext(seqall, &seq))
    {
        begin = ajSeqGetBegin(seq);
        end   = ajSeqGetEnd(seq);

        strand = ajSeqGetSeqS(seq);

	ajStrAssignSubS(&substr,strand,begin-1,end-1);

        /* end with a '*' if needed and there is not one there already */
        if(addlast && ajSeqGetSeqC(seq)[end-1] != '*')
	{
            ajStrAppendK(&substr,'*');
            addedasterisk = ajTrue;
        }
	ajDebug("After appending, sequence=%S\n", substr);
        ajStrFmtUpper(&substr);

        len=ajStrGetLen(substr);

	ajFmtPrintF(outf,"\n\nCHECKTRANS of %s from %d to %d\n\n",
		    ajSeqGetNameC(seq),begin,begin+len-1);

        checktrans_findorfs(outseq, outf, 0, len, ajStrGetPtr(substr),
			    ajSeqGetNameC(seq), orfml, addedasterisk);

	checktrans_dumptofeat(featout,0,len,ajStrGetPtr(substr),
			      ajSeqGetNameC(seq),
			      orfml);
    }

    ajSeqallDel(&seqall);
    ajFileClose(&outf);
    ajSeqoutClose(outseq);
    ajSeqoutDel(&outseq);
    ajFeattabOutDel(&featout);

    ajSeqDel(&seq);
    ajStrDel(&substr);

    embExit();

    return 0;
}




/* @funcstatic checktrans_findorfs ********************************************
**
** Undocumented.
**
** @param [u] outseq [AjPSeqout] Undocumented
** @param [u] outf [AjPFile] Undocumented
** @param [r] from [ajint] Undocumented
** @param [r] to [ajint] Undocumented
** @param [r] p [const char*] Undocumented
** @param [r] name [const char*] Undocumented
** @param [r] min_orflength [ajint] Undocumented
** @param [r] addedasterisk [AjBool] True if an asterisk was added at the end
** @@
******************************************************************************/

static void checktrans_findorfs (AjPSeqout outseq, AjPFile outf, ajint from,
				 ajint to, const char *p, const char *name,
				 ajint min_orflength, AjBool addedasterisk)

{
    ajint i;
    ajint count = 1;
    ajint last_stop = 0;
    ajint orflength = 0;

    ajFmtPrintF(outf,"\tORF#\tPos\tLen\tORF Range\tSequence name\n\n");

    for(i=from;i<to;++i)
    {
	if(p[i]=='*')
	{
	    orflength=i-last_stop;
	    if(orflength >= min_orflength)
	    {
		ajFmtPrintF(outf,"\t%d\t%d\t%d\t%d-%d\t%s_%d\n", count,
			    i+1, orflength, i-orflength+1, i, name,count);
		checktrans_ajbseq(outseq, p,i-orflength,i-1,name,count);

	    }
	    last_stop = ++i;
	    ++count;
            while(p[i] == '*')
	    {
		/* check to see if we have consecutive ****s */
		last_stop = ++i;
		++count;
            }
	}
    }

    /* don't count the last asterisk if it was added by the program */
    if(addedasterisk)
    	--count;

    ajFmtPrintF(outf,"\n\tTotal STOPS: %5d\n\n ",count-1);

    return;
}




/* @funcstatic checktrans_ajbseq **********************************************
**
** Undocumented.
**
** @param [u] outseq [AjPSeqout] Undocumented
** @param [r] seq [const char*] Undocumented
** @param [r] begin [ajint] Undocumented
** @param [r] end [int] Undocumented
** @param [r] name [const char*] Undocumented
** @param [r] count [ajint] Undocumented
** @@
******************************************************************************/

static void checktrans_ajbseq(AjPSeqout outseq, const char *seq,
			      ajint begin, int end,
			      const char *name, ajint count)
{
    AjPSeq sq;
    AjPStr str;
    AjPStr nm;

    sq  = ajSeqNew();
    str = ajStrNew();
    nm  = ajStrNew();

    ajStrAssignSubC(&str,seq,begin,end);
    ajSeqAssignSeqS(sq,str);

    ajFmtPrintS(&nm,"%s_%d",name,count);
    ajSeqAssignNameS(sq,nm);

    ajSeqoutWriteSeq(outseq, sq);

    ajStrDel(&nm);
    ajStrDel(&str);
    ajSeqDel(&sq);

    return;
}




/* @funcstatic checktrans_dumptofeat ******************************************
**
** Undocumented.
**
** @param [u] featout [AjPFeattabOut] Undocumented
** @param [r] from [ajint] Undocumented
** @param [r] to [ajint] Undocumented
** @param [r] p [const char*] Undocumented
** @param [r] seqname [const char*] Undocumented
** @param [r] min_orflength [ajint] Undocumented
** @@
******************************************************************************/

static void checktrans_dumptofeat(AjPFeattabOut featout, ajint from, ajint to,
				  const char *p, const char *seqname,
				  ajint min_orflength)
{
    ajint i;
    ajint count = 1;
    ajint last_stop = 0;
    ajint orflength = 0;
    AjPFeattable feattable;
    AjPStr name   = NULL;
    AjPStr source = NULL;
    AjPStr type   = NULL;
    char strand = '+';
    ajint frame = 0;
    AjPFeature feature;
    float score = 0.0;

    name   = ajStrNew();
    source = ajStrNew();
    type   = ajStrNew();


    ajStrAssignC(&name,seqname);

    feattable = ajFeattableNewProt(name);

    ajStrAssignC(&source,"checktrans");
    ajStrAssignC(&type,"misc_feature");


    for(i=from;i<to;++i)
    {
	if(p[i]=='*')
	{
	    orflength=i-last_stop;
	    if(orflength >= min_orflength)
	    {
		feature = ajFeatNew(feattable, source, type,
				    i-orflength+1,i, score, strand, frame) ;
		if(!feature)
		    ajDebug("Error adding feature to table");
	    }
	    last_stop = ++i;
	    ++count;
	    while(p[i] == '*')
	    {
		/* check to see if there are consecutive ****s */
		last_stop = ++i;
		++count;
	    }
	}
    }

    ajFeatSortByStart(feattable);
    ajFeattableWrite(featout, feattable);
    ajFeattableDel(&feattable);

    ajStrDel(&name);
    ajStrDel(&source);
    ajStrDel(&type);

    return;
}
