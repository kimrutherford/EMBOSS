/* @source entrails application
**
** Show EMBOSS internals
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




/* @prog entrails *************************************************************
**
** Reports internal data structures as a guide for user interface developers
**
******************************************************************************/

int main(int argc, char **argv)
{

    AjPFile outf;
    AjBool full;

    embInit("entrails", argc, argv);

    outf = ajAcdGetOutfile("outfile");
    full = ajAcdGetBoolean("fullreport");

    ajAcdPrintType(outf, full);
    ajAcdPrintQual(outf, full);
    ajAcdPrintAppl(outf, full);
    ajSeqPrintInFormat(outf, full);
    ajSeqoutPrintFormat(outf, full);
    ajSeqPrintType(outf, full);
    ajTextdbPrintAccess(outf, full);
    ajSeqdbPrintAccess(outf, full);
    ajFeatdbPrintAccess(outf, full);
    ajAssemdbPrintAccess(outf, full);
    ajObodbPrintAccess(outf, full);
    ajResourcedbPrintAccess(outf, full);
    ajTaxdbPrintAccess(outf, full);
    ajUrldbPrintAccess(outf, full);
    ajVardbPrintAccess(outf, full);
    ajNamPrintDbAttr(outf, full);
    ajNamPrintRsAttr(outf, full);
    ajFeatinPrintFormat(outf, full);
    ajFeatoutPrintFormat(outf, full);
    ajAsseminprintText(outf, full);
    ajAssemoutprintText(outf, full);
    ajOboinprintText(outf, full);
    ajObooutprintText(outf, full);
    ajResourceinprintText(outf, full);
    ajResourceoutprintText(outf, full);
    ajTaxinprintText(outf, full);
    ajTaxoutprintText(outf, full);
    ajTextinprintText(outf, full);
    ajTextoutprintText(outf, full);
    ajUrlinprintText(outf, full);
    ajUrloutprintText(outf, full);
    ajVarinprintText(outf, full);
    ajVaroutprintText(outf, full);
    ajAlignPrintFormat(outf, full);
    ajReportPrintFormat(outf, full);
    ajCodPrintFormat(outf, full);

    ajGraphicsPrintType(outf, full);

    ajFileClose(&outf);

    embExit();

    return 0;
}
