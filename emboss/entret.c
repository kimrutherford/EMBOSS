/* @source entret application
**
** Retrieves original flatfile entries
**
** @author Copyright (C) Alan Bleasby (ableasby@hgmp.mrc.ac.uk)
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




/* @prog entret ***************************************************************
**
** Reads and writes (returns) flatfile entries
**
******************************************************************************/

int main(int argc, char **argv)
{
    AjPSeqall seqall = NULL;
    AjPFile outf = NULL;
    AjPSeq seq   = NULL;
    AjBool firstonly;

    embInit("entret", argc, argv);

    outf   = ajAcdGetOutfile("outfile");
    seqall = ajAcdGetSeqall("sequence");

    firstonly = ajAcdGetBoolean("firstonly");
    while(ajSeqallNext(seqall,&seq))
    {
	ajFmtPrintF(outf,"%S",ajSeqGetEntryS(seq));
	if(firstonly)
	    break;
    }

    ajFileClose(&outf);
    ajSeqallDel(&seqall);
    ajSeqDel(&seq);

    embExit();
    return 0;
}
