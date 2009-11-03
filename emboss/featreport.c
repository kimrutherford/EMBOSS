/* @source featcopy application
**
** Return a feature table
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




/* @prog featreport ***********************************************************
**
** Reads and writes (returns) sequences
**
******************************************************************************/

int main(int argc, char **argv)
{
    AjPFeattable feat;
    AjPSeq       seq;
    AjPReport    report;

    embInit("featreport", argc, argv);

    seq = ajAcdGetSeq("sequence");
    feat = ajAcdGetFeatures("features");
    report = ajAcdGetReport("outfile");

    ajReportWrite(report, feat, seq);

    ajSeqDel(&seq);
    ajFeattableDel(&feat);
    ajReportDel(&report);

    embExit();

    return 0;
}
