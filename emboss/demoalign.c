/* @source seqretset application
**
** Read and write sequences as a set
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




/* @prog demoalign ************************************************************
**
** Reads a sequence set and writes it as an alignment
**
******************************************************************************/

int main(int argc, char **argv)
{
    AjPSeqset seqset;
    AjPAlign align;
    AjPMatrix imat;
    AjPMatrixf fmat;
    AjBool dofloat;

    embInit("demoalign", argc, argv);

    seqset  = ajAcdGetSeqset("sequence");
    align   = ajAcdGetAlign("outfile");
    fmat    = ajAcdGetMatrixf("floatmatrix");
    imat    = ajAcdGetMatrix("intmatrix");
    dofloat = ajAcdGetBool("dofloat");

    ajSeqsetFill(seqset);

    ajAlignDefine(align, seqset);

    if(dofloat)
    {
	ajAlignSetMatrixFloat(align, fmat);
	ajAlignSetGapR(align, 5.0, -0.3);
    }
    else
    {
	ajAlignSetMatrixInt(align, imat);
	ajAlignSetGapI(align, 9, -1);
    }

    ajAlignWrite(align);
    ajAlignClose(align);

    ajAlignDel(&align);
    ajSeqsetDel(&seqset);

    if(dofloat)
	ajMatrixDel(&imat);
    else
	ajMatrixfDel(&fmat);

    embExit();

    return 0;
}


