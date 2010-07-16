/* @source aligncopy application
**
** Reads and writes (returns) alignments
** @author Copyright (C) Jon Ison (jison@ebi.ac.uk)
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




/* @prog aligncopypair *********************************************************
**
** Reads and writes (returns) alignments
**
******************************************************************************/

int main(int argc, char **argv)
{
    /* Variable Declarations */
    AjPSeqset seqset = NULL;
    AjPAlign  align  = NULL;
    AjPStr    name     = NULL;
    AjPStr    comment  = NULL;
    AjBool    append   = ajFalse;

    AjPSeq   *seqs     = NULL;
    AjPStr    header   = NULL;
    ajuint i;
    ajuint j;
    ajuint nseqs;

    ajuint imax;
    
    /* ACD File Processing */
    embInit("aligncopypair", argc, argv);
    seqset      = ajAcdGetSeqset("sequences");
    align       = ajAcdGetAlign("outfile");
    name       = ajAcdGetString("name");
    comment    = ajAcdGetString("comment");
    append     = ajAcdGetBoolean("append");


    /* Application logic */
    nseqs = ajSeqsetGetSize(seqset);
    seqs = ajSeqsetGetSeqarray(seqset);
    if(ajStrGetLen(name))
        ajFmtPrintS(&header, "Alignment: %S\n\n", name);
    ajStrAppendS(&header, comment);
    if(append)
      ajAlignSetHeaderApp(align, header);
    else
      ajAlignSetHeader(align, header);

    imax = nseqs-1;
    for(i=0;i<imax;i++) 
    {
        for(j=i+1;j<nseqs;j++)
            ajAlignDefineSS(align, seqs[i], seqs[j]);
    }

    ajAlignWrite(align);
    ajAlignClose(align);


    /* Memory management and exit */
    ajSeqsetDel(&seqset);
    ajAlignDel(&align);

    ajStrDel(&name);
    ajStrDel(&comment);
    ajStrDel(&header);
    ajSeqDelarray(&seqs);

    embExit();

    return 0;
}

