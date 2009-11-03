/* @source seqinfo application
**
** Information about sequences
**
** @author Copyright (C) Peter Rice
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




/* @prog seqinfo **************************************************************
**
** Returns sequence information
**
******************************************************************************/

int main(int argc, char **argv)
{
    AjPSeqall seqall;
    AjPFile outf;
    AjPSeq seq = NULL;
    const AjPStr seqstr;
    const AjPStr seqdesc;
    ajint len;
    float pgc;

    embInit("seqinfo", argc, argv);

    seqall = ajAcdGetSeqall("sequence");
    outf   = ajAcdGetOutfile("outfile");

    while(ajSeqallNext(seqall, &seq))
    {
	seqdesc = ajSeqGetDescS(seq);
	len     = ajSeqGetLen(seq);

	ajFmtPrintF(outf, "Sequence \'%s\'\n",     ajSeqGetNameC(seq));

	if(!ajStrIsWhite(seqdesc))
	    ajFmtPrintF(outf, "Description:\t%S\n", seqdesc );

	if(ajSeqIsNuc(seq))
	{
	    ajFmtPrintF(outf, "Type:\t\tDNA\n");
	    ajFmtPrintF(outf, "Length:\t\t%d basepairs\n", len);
	    seqstr = ajSeqGetSeqS(seq);
	    pgc = ajMeltGC(seqstr,len);
	    ajFmtPrintF(outf, "GC Content:\t%-8.4f%%\n", pgc*100.0);
	}
	else if(ajSeqIsProt(seq))
	{
	    ajFmtPrintF(outf, "Type:\t\tProtein\n");
	    ajFmtPrintF(outf, "Length:\t\t%d residues\n", len);
	}
	else
	{
	    ajFmtPrintF(outf, "Type:\t\tUnknown\n");
	    ajFmtPrintF(outf, "Length:\t\t%d residues\n", len);
	}
    }

    ajSeqallDel(&seqall);
    ajFileClose(&outf);
    ajSeqDel(&seq);

    embExit();

    return 0;
}
