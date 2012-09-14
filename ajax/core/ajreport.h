/* @include ajreport **********************************************************
**
** AJAX REPORT (ajax feature reporting) functions
**
** These functions report AJAX sequence feature data in a variety
** of formats.
**
** @author Copyright (C) 2000 Peter Rice, LION Bioscience Ltd.
** @version  $Revision: 1.40 $
** @modified Nov 10 First version
** @modified $Date: 2011/10/18 14:23:40 $ by $Author: rice $
** @@
**
** This library is free software; you can redistribute it and/or
** modify it under the terms of the GNU Lesser General Public
** License as published by the Free Software Foundation; either
** version 2.1 of the License, or (at your option) any later version.
**
** This library is distributed in the hope that it will be useful,
** but WITHOUT ANY WARRANTY; without even the implied warranty of
** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
** Lesser General Public License for more details.
**
** You should have received a copy of the GNU Lesser General Public
** License along with this library; if not, write to the Free Software
** Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,
** MA  02110-1301,  USA.
**
******************************************************************************/

#ifndef AJREPORT_H
#define AJREPORT_H

/* ========================================================================= */
/* ============================= include files ============================= */
/* ========================================================================= */

#include "ajdefine.h"
#include "ajmath.h"
#include "ajstr.h"
#include "ajfile.h"
#include "ajlist.h"
#include "ajfeatdata.h"
#include "ajseqdata.h"

AJ_BEGIN_DECLS




/* ========================================================================= */
/* =============================== constants =============================== */
/* ========================================================================= */




/* ========================================================================= */
/* ============================== public data ============================== */
/* ========================================================================= */




/* @data AjPReport ************************************************************
**
** Ajax Report Output object.
**
** Holds definition of feature report output.
**
** @alias AjSReport
** @alias AjOReport
**
** @other AjPSeqout Sequence output
** @other AjPFile Input and output files
**
** @attr Name [AjPStr] As "Source" for features, usually empty
** @attr Type [AjPStr] "P" Protein or "N" Nucleotide
** @attr Formatstr [AjPStr] Report format (-rformat)
** @attr Fttable [AjPFeattable] Feature table to use (obsolete?)
** @attr Ftquery [AjPFeattabOut] Output definition for features
** @attr Extension [AjPStr] Output file extension
** @attr File [AjPFile] Output file object
** @attr Tagnames [AjPList] List of extra tag names (from ACD)
** @attr Tagprints [AjPList] List of extra tag printnames (from ACD)
** @attr Tagtypes [AjPList] List of extra tag datatypes (from ACD)
** @attr Header [AjPStr] Text to add to header with newlines
** @attr SubHeader [AjPStr] Text to add to subheader with newlines
** @attr Tail [AjPStr] Text to add to tail with newlines
** @attr SubTail [AjPStr] Text to add to subtail with newlines
** @attr FileNames [AjPList] Names of extra files (see FileTypes)
** @attr FileTypes [AjPList] Types of extra files (see FileNames)
** @attr Totseqs [ajlong] Total number of sequences processed
** @attr Totlength [ajlong] Total length of sequences processed
** @attr Precision [ajint] Floating precision for score
** @attr Showacc [AjBool] Report accession number
** @attr Showdes [AjBool] Report sequence description
** @attr Showusa [AjBool] Report USA (-rusa) or only seqname
** @attr Showscore [AjBool] Report score (if optional for format)
** @attr Showstrand [AjBool] Report nucleotide strand (if optional for format)
** @attr Multi [AjBool] if true, assume >1 sequence
** @attr Mintags [ajint] Minimum number of tags to report
** @attr CountSeq [ajint] Number of sequences reported so far
** @attr CountHit [ajint] Number of features reported so far
** @attr TotHits [ajint] Number of features found so far
** @attr MaxHitAll [ajint] Maximum number of hits to report overall
** @attr MaxHitSeq [ajint] Maximum number of hits to report for each sequence
** @attr MaxLimit [AjBool] if true, maximum hits reached
** @attr Format [AjEnum] Report format (index number)
** @attr Padding [char[4]] Padding to alignment boundary
**
** @new ajReportNew Default constructor
** @delete ajReportDel Default destructor
** @output ajReportWrite Master sequence output routine
** @@
******************************************************************************/

typedef struct AjSReport {
    AjPStr Name;
    AjPStr Type;
    AjPStr Formatstr;
    AjPFeattable Fttable;
    AjPFeattabOut Ftquery;
    AjPStr Extension;
    AjPFile File;
    AjPList Tagnames;
    AjPList Tagprints;
    AjPList Tagtypes;
    AjPStr Header;
    AjPStr SubHeader;
    AjPStr Tail;
    AjPStr SubTail;
    AjPList FileNames;
    AjPList FileTypes;
    ajlong Totseqs;
    ajlong Totlength;
    ajint Precision;
    AjBool Showacc;
    AjBool Showdes;
    AjBool Showusa;
    AjBool Showscore;
    AjBool Showstrand;
    AjBool Multi;
    ajint Mintags;
    ajint CountSeq;
    ajint CountHit;
    ajint TotHits;
    ajint MaxHitAll;
    ajint MaxHitSeq;
    AjBool MaxLimit;
    AjEnum Format;
    char   Padding[4];
} AjOReport;

#define AjPReport AjOReport*




/* ========================================================================= */
/* =========================== public functions ============================ */
/* ========================================================================= */




/*
** Prototype definitions
*/

void         ajReportClose(AjPReport pthys);
void         ajReportDel(AjPReport* pthys);
void         ajReportDummyFunction(void);
void         ajReportExit(void);
void         ajReportAddFileF(AjPReport thys,
                              AjPFile file, const AjPStr type);
AjBool       ajReportFindFormat(const AjPStr format, ajint* iformat);
AjBool       ajReportFormatDefault(AjPStr* pformat);
AjPReport    ajReportNew(void);
AjBool       ajReportOpen(AjPReport thys, const AjPStr name);
void         ajReportPrintFormat(AjPFile outf, AjBool full);
void         ajReportPrintbookFormat(AjPFile outf);
void         ajReportPrinthtmlFormat(AjPFile outf);
void         ajReportPrintwikiFormat(AjPFile outf);
const AjPStr ajReportGetSeqnameSeq(const AjPReport thys, const AjPSeq seq);
void         ajReportAppendHeaderS(AjPReport thys, const AjPStr header);
void         ajReportAppendHeaderC(AjPReport thys, const char* header);
void         ajReportSetHeaderS(AjPReport thys, const AjPStr header);
void         ajReportSetHeaderC(AjPReport thys, const char* header);
void         ajReportAppendSubheaderS(AjPReport thys, const AjPStr header);
void         ajReportAppendSubheaderC(AjPReport thys, const char* header);
void         ajReportSetSubheaderC(AjPReport thys, const char* header);
void         ajReportSetSubheaderS(AjPReport thys, const AjPStr header);
void         ajReportSetSeqstats(AjPReport thys, const AjPSeqall seqall);
void         ajReportSetSeqsetstats(AjPReport thys, const AjPSeqset seqset);
void         ajReportSetStatistics(AjPReport thys, ajlong totseqs,
                                   ajlong totlength);
AjBool       ajReportSetTagsS(AjPReport thys, const AjPStr taglist);
void         ajReportAppendTailS(AjPReport thys, const AjPStr tail);
void         ajReportAppendTailC(AjPReport thys, const char* tail);
void         ajReportSetTailS(AjPReport thys, const AjPStr tail);
void         ajReportSetTailC(AjPReport thys, const char* tail);
void         ajReportAppendSubtailS(AjPReport thys, const AjPStr tail);
void         ajReportAppendSubtailC(AjPReport thys, const char* tail);
void         ajReportSetSubtailS(AjPReport thys, const AjPStr tail);
void         ajReportSetSubtailC(AjPReport thys, const char* tail);
void         ajReportSetType(AjPReport thys,
                             const AjPFeattable ftable, const AjPSeq seq);
AjBool       ajReportValid(AjPReport thys);
AjBool       ajReportWrite(AjPReport thys,
                           const AjPFeattable ftable,  const AjPSeq seq);
void         ajReportWriteHeader(AjPReport thys,
                                 const AjPFeattable ftable, const AjPSeq seq);
void         ajReportWriteTail(AjPReport thys,
                               const AjPFeattable ftable, const AjPSeq seq);

/*
** End of prototype definitions
*/




#ifdef AJ_COMPILE_DEPRECATED_BOOK
#endif /* AJ_COMPILE_DEPRECATED_BOOK */

#ifdef AJ_COMPILE_DEPRECATED

__deprecated void     ajReportFileAdd(AjPReport thys,
                                      AjPFile file, const AjPStr type);
__deprecated void     ajReportAppendSubHeader(AjPReport thys,
                                              const AjPStr header);
__deprecated void     ajReportAppendSubHeaderC(AjPReport thys,
                                               const char* header);
__deprecated void     ajReportSetSubHeader(AjPReport thys,
                                           const AjPStr header);
__deprecated AjBool   ajReportSetTags(AjPReport thys,
                                      const AjPStr taglist);
__deprecated void     ajReportAppendTail(AjPReport thys, const AjPStr tail);
__deprecated void     ajReportSetTail(AjPReport thys, const AjPStr tail);
__deprecated void     ajReportAppendHeader(AjPReport thys,
                                           const AjPStr header);
__deprecated void     ajReportSetHeader(AjPReport thys, const AjPStr header);
__deprecated void     ajReportAppendSubTail(AjPReport thys, const AjPStr tail);
__deprecated void     ajReportAppendSubTailC(AjPReport thys, const char* tail);
__deprecated void     ajReportSetSubTail(AjPReport thys, const AjPStr tail);
__deprecated void     ajReportSetSubTailC(AjPReport thys, const char* tail);
__deprecated const    AjPStr ajReportSeqName(const AjPReport thys,
                                             const AjPSeq seq);
__deprecated ajint    ajReportLists(const AjPReport thys,
                                    AjPStr** types, AjPStr** names,
                                    AjPStr** prints, ajuint** tagsizes);

#endif /* AJ_COMPILE_DEPRECATED */




AJ_END_DECLS

#endif /* !AJREPORT_H */
