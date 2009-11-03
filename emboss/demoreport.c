/* @source demoreport application
**
** Demomnstration of how the report functions should be used.
** @author Copyright (C) Peter Rice (pmr@sanger.ac.uk)
** @@
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




/* @prog demoreport ***********************************************************
**
** Reads a feature table (and sequence) and writes as a report
**
******************************************************************************/

int main(int argc, char **argv)
{

    AjPReport report;
    AjPSeq seq = NULL;
    AjPFeattable ftab;

    embInit("demoreport", argc, argv);

    report = ajAcdGetReport("outfile");
    seq    = ajAcdGetSeq("sequence");

    ftab = ajSeqGetFeatCopy(seq);

    ajReportWrite(report, ftab, seq);

    ajReportDel(&report);
    ajSeqDel(&seq);
    ajFeattableDel(&ftab);

    embExit();

    return 0;
}
