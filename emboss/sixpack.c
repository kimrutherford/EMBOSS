/* @source sixpack application
**
** Display a DNA sequence in both direction with its translation
**
** @author Copyright (C) Thomas Laurent (thomas.laurent@uk.lionbioscience.com)
** 30 Sept 2002
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




static int sixpackFindorfs(AjPSeqout outseq, AjPFile outf, ajint s,
			   ajint len, const char *seq,
			   const char *name, ajint orfml, 
			   AjBool addedasterisk, AjBool firstorf,
			   ajint frame, 
			   const char *origname, AjBool mstart);

static void sixpackPrintseq(AjPSeqout outseq, const char *seq, ajint begin,
			    int end, ajint orflength, const char *name,
			    ajint count, ajint frame, const char *origname,
			    ajint min_orflength);




/* @prog sixpack **************************************************************
**
** Display a DNA sequence in both direction with its translation
**
******************************************************************************/

int main(int argc, char **argv)
{
    ajint begin;
    ajint end;
    ajint pepbegin;
    ajint pepend;
    ajint peplen;
    AjPSeq seq;
    AjPSeq pep;
    const AjPStr pepseq = NULL;
    AjPStr substr = NULL;
    EmbPShow ss;
    AjPFile outfile;
    AjPSeqout outseq=NULL;
    AjPStr tablename;
    ajint table;
    AjPRange uppercase;
    AjPRange highlight;
    AjBool numberseq;
    AjBool nameseq;
    ajint width;
    ajint length;
    ajint margin;
    AjBool description;
    ajint offset;
    AjBool html;
    AjPStr descriptionline;
    ajint orfminsize;
    AjPTrn trnTable;
    AjBool reverse;
    AjBool addedasterisk = ajFalse;
    AjBool addlast  = ajTrue;
    AjBool firstorf = ajTrue;
    AjBool mstart   = ajFalse;
    ajint totalorf  = 0;
    ajint totalframes = 3;

    int i;

    embInit("sixpack", argc, argv);

    seq         = ajAcdGetSeq("sequence");
    outfile     = ajAcdGetOutfile("outfile");
    outseq      = ajAcdGetSeqoutall("outseq");
    tablename   = ajAcdGetListSingle("table");
    uppercase   = ajAcdGetRange("uppercase");
    highlight   = ajAcdGetRange("highlight");
    numberseq   = ajAcdGetBoolean("number");
    width       = ajAcdGetInt("width");
    length      = ajAcdGetInt("length");
    margin      = ajAcdGetInt("margin");
    nameseq     = ajAcdGetBoolean("name");
    description = ajAcdGetBoolean("description");
    offset      = ajAcdGetInt("offset");
    html        = ajAcdGetBoolean("html");
    orfminsize  = ajAcdGetInt("orfminsize");
    reverse     = ajAcdGetBoolean("reverse");
    addlast     = ajAcdGetBoolean("lastorf");
    firstorf    = ajAcdGetBoolean("firstorf");
    mstart      = ajAcdGetBoolean("mstart");
    
    
    /* get the number of the genetic code used */
    ajStrToInt(tablename, &table);
    trnTable = ajTrnNewI(table);
    
    /* get begin and end positions */
    begin = ajSeqGetBegin(seq)-1;
    end = ajSeqGetEnd(seq)-1;
    
    /* do the name and description */
    if(nameseq)
    {
	if(html)
	    ajFmtPrintF(outfile, "<H2>%S</H2>\n",
			ajSeqGetNameS(seq));
	else
	    ajFmtPrintF(outfile, "%S\n", ajSeqGetNameS(seq)); 
    }
    
    if(description)
    {
	/*
	 **  wrap the description line at the width of the sequence
	 **  plus margin
	 */
	if(html)
	    ajFmtPrintF(outfile, "<H3>%S</H3>\n",
			ajSeqGetDescS(seq));
	else
	{
	    descriptionline = ajStrNew();
	    ajStrAssignS(&descriptionline, ajSeqGetDescS(seq));
	    ajStrFmtWrap(&descriptionline, width+margin);
	    ajFmtPrintF(outfile, "%S\n", descriptionline);
	    ajStrDel(&descriptionline);
	}
    }
    
    
    /* make the Show Object */
    ss = embShowNew(seq, begin, end, width, length, margin, html, offset);
    
    if(html)
	ajFmtPrintF(outfile, "<PRE>");
    
    /* create the format to display */
    
    if(reverse) 
	embShowAddBlank(ss);
    embShowAddBlank(ss);
    
    
    embShowAddTran(ss, trnTable, 1, AJFALSE, AJFALSE,
		   NULL, orfminsize, AJTRUE, firstorf, addlast, AJTRUE);
    embShowAddTran(ss, trnTable, 2, AJFALSE, AJFALSE,
		   NULL, orfminsize, AJTRUE, firstorf, addlast, AJTRUE);
    embShowAddTran(ss, trnTable, 3, AJFALSE, AJFALSE,
		   NULL, orfminsize, AJTRUE, firstorf, addlast, AJTRUE);
    
    /*	embShowAddBlank(ss);*/
    embShowAddSeq(ss, numberseq, AJFALSE, uppercase, highlight);
    
    if(!numberseq)
	embShowAddTicknum(ss);
    embShowAddTicks(ss);
    
    if(reverse)
	embShowAddComp(ss, numberseq);
    
    if(reverse)
    {
	totalframes = 6;
      
	embShowAddTran(ss, trnTable, -3, AJFALSE,
		       AJFALSE, NULL, orfminsize, AJTRUE,
		       firstorf, addlast, AJTRUE);
	embShowAddTran(ss, trnTable, -2, AJFALSE,
		       AJFALSE, NULL, orfminsize, AJTRUE,
		       firstorf, addlast, AJTRUE);
	embShowAddTran(ss, trnTable, -1, AJFALSE,
		       AJFALSE, NULL, orfminsize, AJTRUE,
		       firstorf, addlast, AJTRUE);
    }
    
    embShowPrint(outfile, ss);
    
    
    /* add a newline at the end of the sequence */
    ajFmtPrintF(outfile, "\n");
    
    embShowDel(&ss);
    
    /* Print the footer */
    ajFmtPrintF(outfile, "##############################\n");
    ajFmtPrintF(outfile, "Minimum size of ORFs : %d\n\n", orfminsize);
    
    /* Write ORFs in a separate file */
    
    for(i=0; i<totalframes; i++)
    {
	ajDebug("try frame: %d\n", i);

	if(i<3)
	    pep = ajTrnSeqOrig(trnTable, seq, i+1);
	else
	    /* frame -1 uses frame 1 codons */
	    pep = ajTrnSeqOrig(trnTable, seq, 2-i);
	  
	pepbegin = ajSeqGetBegin(pep)-1;
	pepend = ajSeqGetEnd(pep)-1;
	pepseq = ajSeqGetSeqS(pep);

	ajStrAssignSubS(&substr,pepseq,pepbegin,pepend);

	/* end with a '*' if we want to and there is not one there already */
	ajDebug("last residue =%c\n", ajSeqGetSeqC(pep)[pepend]);

	if(addlast && ajSeqGetSeqC(pep)[pepend] != '*')
	{
	    ajStrAppendK(&substr,'*');
	    addedasterisk = ajTrue;
	}

	ajDebug("After appending, sequence=%S\n", substr);
	ajStrFmtUpper(&substr);
	  
	peplen = ajStrGetLen(substr);

	totalorf += sixpackFindorfs(outseq, outfile, 0, peplen,
				    ajStrGetPtr(substr),
				    ajSeqGetNameC(pep), orfminsize,
				    addedasterisk, firstorf,
				    i+1, ajSeqGetNameC(seq), mstart);

	ajSeqDel(&pep);
    }

    ajFmtPrintF(outfile,"\nTotal ORFs : %5d\n",totalorf);
    ajFmtPrintF(outfile, "##############################\n\n");
    
    ajTrnDel(&trnTable);
    ajSeqoutClose(outseq);
    ajSeqoutDel(&outseq);
    ajSeqDel(&seq);
    ajSeqDel(&pep);
    ajStrDel(&substr);
    ajStrDel(&tablename);
    ajFileClose(&outfile);
    ajRangeDel(&uppercase);
    ajRangeDel(&highlight);


    embExit();
    return 0;
}




/* @funcstatic sixpackFindorfs ************************************************
**
** Finds ORFs and prints report
**
** @param [u] outseq [AjPSeqout] File where to write fasta sequences
** @param [u] outf [AjPFile] File where to write the report on ORFs 
** @param [r] from [ajint] Zero
** @param [r] to [ajint] Length of the sequence
** @param [r] p [const char*] Sequence
** @param [r] name [const char*] Name of the translated sequence
**                               (with frame number)
** @param [r] min_orflength [ajint] Minimum size of the ORFs to report
** @param [r] addedasterisk [AjBool] True if an asterisk was added at the end
** @param [r] firstorf [AjBool] ajTrue to find first ORF
** @param [r] frame [ajint] Frame number
** @param [r] origname [const char*] Original name of the sequence (DNA)
** @param [r] mstart [AjBool] ajTrue to start with methionine codon only
** @return [int] number of orfs
** @@
******************************************************************************/

static int sixpackFindorfs(AjPSeqout outseq, AjPFile outf, ajint from,
			   ajint to, const char *p, const char *name,
			   ajint min_orflength,
			   AjBool addedasterisk, AjBool firstorf,
			   ajint frame, 
			   const char *origname, AjBool mstart)

{
    ajint i;
    ajint j;
    ajint last_stop = 0;
    ajint orflength = 0;
    ajint orfnb = 0;
    
    for(i=from;i<to;++i)
    {
	if(p[i]=='*')
	{
	    if(!mstart)
		orflength = i-last_stop;
	    else 
	    {
		orflength = 0;
		for(j=last_stop;j<i;j++)
		{
		    if(p[j]=='M')
		    {
			orflength = i-j;
			break;
		    }
		}
	    }

	    if(orflength >= min_orflength)
	    {
		sixpackPrintseq(outseq, p,i-orflength,i-1,orflength,
				name,orfnb+1,frame,origname,
				min_orflength);
		orfnb++;
	    }
	    else if((last_stop == 0) && firstorf && p[0] != '*')
	    {
		if(mstart)
		    orflength=i-last_stop;

		if(orflength > 0)
		{
		    sixpackPrintseq(outseq, p,i-orflength,i-1,orflength,
				    name,orfnb+1,frame,origname,
				    min_orflength);
		    orfnb++;
		}
	    }
	    else if((i == to-1) && addedasterisk)
	    {
		if(orflength > 0)
		{
		    sixpackPrintseq(outseq, p,i-orflength,i-1,orflength,
				    name,orfnb+1,frame,origname,
				    min_orflength);
		    orfnb++;
		}
	    }

	    
	    last_stop = ++i;
	    
	    /* check to see if there are consecutive ****s */
            while(p[i] == '*')
		last_stop = ++i;
	}
    }
    
    ajFmtPrintF(outf,"Total ORFs in frame %d : %5d\n", frame, orfnb);
    
    return orfnb;
}




/* @funcstatic sixpackPrintseq ************************************************
**
** Prints ORFs in the sequence file
**
** @param [u] outseq [AjPSeqout] File where to write fasta sequences
** @param [r] seq [const char*] Sequence to write
** @param [r] begin [ajint] Start position of the ORF to write
** @param [r] end [int] End position of the ORF to write
** @param [r] orflength [ajint] Size of the current ORF
** @param [r] name [const char*] Name of the translated sequence
**                              (with frame number)
** @param [r] count [ajint] Number of the ORF to be written in this frame
** @param [r] frame [ajint] Frame number
** @param [r] origname [const char*] Original name of the sequence (DNA)
** @param [r] min_orflength [ajint] Minimum size for an ORF
** @return [void]
** @@
******************************************************************************/

static void sixpackPrintseq(AjPSeqout outseq,
			    const char *seq, ajint begin, int
			    end, ajint orflength, const char *name,
			    ajint count, ajint frame, 
			    const char *origname, ajint min_orflength)
{
    AjPSeq sq;
    AjPStr str;
    AjPStr nm;

    str  = ajStrNew();
    nm   = ajStrNew();

    sq   = ajSeqNew();
    ajSeqSetProt(sq);

    ajStrAssignSubC(&str,seq,begin,end);
    ajSeqAssignSeqS(sq,str);

    ajFmtPrintS(&nm, "%s_ORF%d  Translation of %s in frame %d, ORF %d, "
		"threshold %d, %daa",
		name,count,origname,frame,count,min_orflength,orflength);
    ajSeqAssignNameS(sq,nm);

    ajSeqoutWriteSeq(outseq, sq);

    ajStrDel(&nm);
    ajStrDel(&str);
    ajSeqDel(&sq);

    return;
}
