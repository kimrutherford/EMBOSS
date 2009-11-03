/* @source wordcount application
**
** Counts words of a specified size in a DNA sequence
**
** @author
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
#include "ajtable.h"




/* @prog wordcount ************************************************************
**
** Counts words of a specified size in a DNA sequence
**
******************************************************************************/

int main(int argc, char **argv)
{
    AjPSeqall seqall;
    AjPSeq seq;
    AjPTable table = 0;
    AjPFile outf;
    ajint wordsize;
    ajint mincount;

    embInit("wordcount", argc, argv);

    seqall = ajAcdGetSeqall("sequence1");

    wordsize = ajAcdGetInt("wordsize");
    outf     = ajAcdGetOutfile("outfile");
    mincount = ajAcdGetInt("mincount");

    embWordLength(wordsize);

    while (ajSeqallNext(seqall, &seq))
    {
	embWordGetTable(&table, seq);		/* get table of words   */
    }

    embWordPrintTableFI(table, mincount, outf); /* print table of words */
    /*
     **  test if table can be added to
     **  if(getWordTable(&table, seq, wordcount)) ?? get table of words ??
     **  {
     **       printWordTable(table);              ?? print table of words ??
     **  }
     */
    embWordFreeTable(&table);	/* free table of words */

    ajSeqallDel(&seqall);
    ajSeqDel(&seq);
    ajFileClose(&outf);

    embExit();

    return 0;
}
