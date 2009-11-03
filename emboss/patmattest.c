/* @source patmattest.c
**
** General test routine for pattern matching.
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




static const char *testset[] =
{
    "GAN","GAATTC","CCSGG","GANTC","GABNNNNNVTC","GA", "GANN","TC"
};




/* @prog patmattest ***********************************************************
**
** Testing
**
******************************************************************************/

int main(int argc, char **argv)
{
    AjPStr cut    = NULL;
    AjPStr cutseq = NULL;
    AjPStr new    = NULL;
    AjPStr test   = NULL;
    AjPStr regexp = NULL;
    AjPFile outf  = NULL;
    EmbPPatMatch results = NULL;
    AjPSeq seq;
    ajuint i;
    ajuint j;

    embInit("patmattest", argc, argv);

    seq = ajAcdGetSeq("sequence1");

    cutseq = ajAcdGetString("expression");

    outf = ajAcdGetOutfile("outfile");

    ajStrAssignC(&test,"GAATTCCCGGAGATTCCGACTC");


    for(i=0;i<8;i++)
    {
	ajStrAssignC(&cut,testset[i]);

	/* Create the regular expression from the plain text */
	regexp = embPatSeqCreateRegExp(cut,0);

	/* find the matches */
	results = embPatMatchFind(regexp,test, ajFalse, ajFalse);


	ajFmtPrintF(outf,"01234567890123456789012345\n");
	ajFmtPrintF(outf,"%S\n",test);
	ajFmtPrintF(outf,"%S %S\n",cut,regexp);
	ajFmtPrintF(outf,"%d matches found\n",results->number);
	for(j=0;j<results->number;j++)
	    ajFmtPrintF(outf,"start = %d len = %d\n",
			results->start[j],results->len[j]);
	ajFmtPrintF(outf," \n");
	embPatMatchDel(&results);
	ajStrDel(&regexp);
	ajStrDel(&cut);
    }
    ajStrDel(&test);

    results = embPatSeqMatchFind(seq, cutseq);
    ajFmtPrintF(outf,"%S\n",cutseq);
    ajFmtPrintF(outf,"%d matches found\n",results->number);
    for(j=0;j < embPatMatchGetNumber(results) ;j++)
    {
	ajFmtPrintF(outf,"start = %d len = %d\n",
		    embPatMatchGetStart(results,j),
		    embPatMatchGetLen(results,j));
	/* get a copy of the string */
	new = ajStrNewRes(results->len[j]);
	ajStrAssignSubS(&new,ajSeqGetSeqS(seq),embPatMatchGetStart(results,j),
		    embPatMatchGetEnd(results,j));
	ajFmtPrintF(outf,"%S\n",new);
	ajStrDel(&new);
    }

    ajFmtPrintF(outf," \n");
    embPatMatchDel(&results);
    ajStrDel(&cutseq);
    ajSeqDel(&seq);

    ajFileClose(&outf);

    embExit();

    return 0;
}
