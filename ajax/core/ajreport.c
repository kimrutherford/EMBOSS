/******************************************************************************
** @source AJAX REPORT (ajax feature reporting) functions
**
** These functions report AJAX sequence feature data in a variety
** of formats.
**
** @author Copyright (C) 2000 Peter Rice, LION Bioscience Ltd.
** @version 1.0
** @modified Nov 10 First version
** @@
**
** This library is free software; you can redistribute it and/or
** modify it under the terms of the GNU Library General Public
** License as published by the Free Software Foundation; either
** version 2 of the License, or (at your option) any later version.
**
** This library is distributed in the hope that it will be useful,
** but WITHOUT ANY WARRANTY; without even the implied warranty of
** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
** Library General Public License for more details.
**
** You should have received a copy of the GNU Library General Public
** License along with this library; if not, write to the
** Free Software Foundation, Inc., 59 Temple Place - Suite 330,
** Boston, MA  02111-1307, USA.
******************************************************************************/

#include <stddef.h>
#include <stdarg.h>
#include <float.h>
#include <limits.h>
#include <math.h>

#include "ajax.h"




/* @datastatic ReportPFormat **************************************************
**
** Ajax feature report formats 
**
** @attr Name [const char*] Format name
** @attr Desc [const char*] Format description
** @attr Alias [AjBool] Name is an alias for an identical definition
** @attr Mintags [ajuint] Minimum number of special tags needed
** @attr Showseq [AjBool] ajTrue if sequence is to be included
** @attr Nucleotide [AjBool] ajTrue if format can work with nucleotide sequences
** @attr Protein [AjBool] ajTrue if format can work with protein sequences
** @attr Showheader [AjBool] ajTrue if header appears in output
** @attr Write [(void*)] Function to write report
******************************************************************************/

typedef struct ReportSFormat
{
    const char *Name;
    const char *Desc;
    AjBool Alias;
    ajuint Mintags;
    AjBool Showseq;
    AjBool Nucleotide;
    AjBool Protein;
    AjBool Showheader;
    void (*Write) (AjPReport outrpt,
		   const AjPFeattable ftable, const AjPSeq seq);
} ReportOFormat;

#define ReportPFormat ReportOFormat*

static AjPRegexp reportTagExp = NULL;



static void reportWriteEmbl(AjPReport outrpt, const AjPFeattable ftable,
			    const AjPSeq seq);
static void reportWriteGenbank(AjPReport outrpt, const AjPFeattable ftable,
			       const AjPSeq seq);
static void reportWriteGff(AjPReport outrpt, const AjPFeattable ftable,
			   const AjPSeq seq);
static void reportWritePir(AjPReport outrpt, const AjPFeattable ftable,
			   const AjPSeq seq);
static void reportWriteSwiss(AjPReport outrpt,const  AjPFeattable ftable,
			     const AjPSeq seq);
static void reportWriteDasgff(AjPReport outrpt,const  AjPFeattable ftable,
			     const AjPSeq seq);

static void reportWriteTrace(AjPReport outrpt, const AjPFeattable ftable,
			     const AjPSeq seq);
static void reportWriteListFile(AjPReport outrpt, const AjPFeattable ftable,
				const AjPSeq seq);

static void reportWriteDbMotif(AjPReport outrpt, const AjPFeattable ftable,
			       const AjPSeq seq);
static void reportWriteDiffseq(AjPReport outrpt, const AjPFeattable ftable,
			       const AjPSeq seq);
static void reportWriteDraw(AjPReport outrpt, const AjPFeattable ftable,
			   const AjPSeq seq);
static void reportWriteExcel(AjPReport outrpt, const AjPFeattable ftable,
			     const AjPSeq seq);
static void reportWriteFeatTable(AjPReport outrpt, const AjPFeattable ftable,
				 const AjPSeq seq);
static void reportWriteMotif(AjPReport outrpt, const AjPFeattable ftable,
			     const AjPSeq seq);
static void reportWriteNameTable(AjPReport outrpt, const AjPFeattable ftable,
				 const AjPSeq seq);
static void reportWriteRegions(AjPReport outrpt, const AjPFeattable ftable,
			       const AjPSeq seq);
static void reportWriteRestrict(AjPReport outrpt, const AjPFeattable ftable,
                                const AjPSeq seq);
static void reportWriteSeqTable(AjPReport outrpt, const AjPFeattable ftable,
				const AjPSeq seq);
static void reportWriteSimple(AjPReport outrpt, const AjPFeattable ftable,
			      const AjPSeq seq);
static void reportWriteSrs(AjPReport outrpt, const AjPFeattable ftable,
			   const AjPSeq seq);
static void reportWriteSrsFlags(AjPReport outrpt, const AjPFeattable ftable,
				const AjPSeq seq, AjBool withSeq);
static void reportWriteTable(AjPReport outrpt, const AjPFeattable ftable,
			     const AjPSeq seq);

static void reportWriteTagseq(AjPReport outrpt, const AjPFeattable ftable,
			      const AjPSeq seq);

static const char* reportCharname(const AjPReport thys);
static ajint reportGetTags(const AjPReport thys, AjPStr** types, AjPStr** names,
                           AjPStr** prints, ajuint** sizes);




/* @funclist reportFormat *****************************************************
**
** Functions to write feature reports
**
******************************************************************************/

static ReportOFormat reportFormat[] =
{
/*   Name         Description */
/*       Alias MinTags Showseq Nuc     Prot     Header   Function */
   /* standard feature formats */
/*    {"unknown",   "Unknown feature format",
	 AJFALSE, 0, AJFALSE, AJTRUE,  AJFALSE, AJFALSE, reportWriteSimple},*/
    {"simple",    "Simple report",
	 AJFALSE, 0, AJFALSE, AJTRUE,  AJTRUE,  AJTRUE,  reportWriteSimple},
    {"embl",      "EMBL feature format",
	 AJFALSE, 0, AJFALSE, AJTRUE,  AJFALSE, AJFALSE, reportWriteEmbl},
    {"genbank",   "Genbank feature format",
	 AJFALSE, 0, AJFALSE, AJTRUE,  AJFALSE, AJFALSE, reportWriteGenbank},
    {"gff",       "GFF3 feature format",
	 AJFALSE, 0, AJFALSE, AJTRUE,  AJTRUE,  AJFALSE, reportWriteGff},
    {"gff3",       "GFF3 feature format (alias)",
	 AJTRUE,  0, AJFALSE, AJTRUE,  AJTRUE,  AJFALSE, reportWriteGff},
    {"pir",       "PIR feature format",
	 AJFALSE, 0, AJFALSE, AJFALSE, AJTRUE,  AJFALSE, reportWritePir},
    {"swiss",     "Swissprot feature format",
	 AJFALSE, 0, AJFALSE, AJFALSE, AJTRUE,  AJFALSE, reportWriteSwiss},
    {"dasgff",    "DAS GFF feature format",
	 AJFALSE, 0, AJFALSE, AJTRUE,  AJTRUE,  AJFALSE, reportWriteDasgff},
    /* trace  for debug */
    {"debug",     "Debugging trace of full internal data content",
	 AJFALSE, 0, AJTRUE,  AJTRUE,  AJTRUE,  AJTRUE,  reportWriteTrace},
    /* list file for input to other programs */
    {"listfile",  "EMBOSS list file of sequence USAs with ranges",
	 AJFALSE, 0, AJFALSE, AJTRUE,  AJTRUE,  AJTRUE,  reportWriteListFile},
    /* feature reports */
    {"dbmotif",   "Motif database hits",
	 AJFALSE, 0, AJTRUE,  AJTRUE,  AJTRUE,  AJTRUE,  reportWriteDbMotif},
    {"diffseq",   "Differences between a pair of sequences",
	 AJFALSE, 7, AJTRUE,  AJTRUE,  AJTRUE,  AJTRUE,  reportWriteDiffseq},
/*    cirdna/lindna input format - may need work */
    {"draw",      "lindna/cirdna input format",
	 AJFALSE, 0, AJFALSE, AJTRUE,  AJTRUE,  AJTRUE,  reportWriteDraw},
    {"restrict",  "lindna/cirdna input format for restriction sites",
	 AJFALSE, 0, AJFALSE, AJTRUE,  AJTRUE,  AJTRUE,  reportWriteRestrict},
    {"excel",     "Tab-delimited file for import to Microsoft Excel",
	 AJFALSE, 0, AJFALSE, AJTRUE,  AJTRUE,  AJFALSE, reportWriteExcel},
    {"feattable", "EMBL format feature table with internal tags",
	 AJFALSE, 0, AJFALSE, AJTRUE,  AJTRUE,  AJFALSE, reportWriteFeatTable},
    {"motif",     "Motif report",
	 AJFALSE, 0, AJTRUE,  AJTRUE,  AJTRUE,  AJTRUE,  reportWriteMotif},
    {"nametable", "Simple table with sequence name",
	 AJFALSE, 0, AJFALSE, AJTRUE,  AJTRUE,  AJTRUE,  reportWriteNameTable},
    {"regions",   "Annotated sequence regions",
	 AJFALSE, 0, AJFALSE, AJTRUE,  AJTRUE,  AJTRUE,  reportWriteRegions},
    {"seqtable",  "Simple table with sequence on each line",
	 AJFALSE, 0, AJTRUE,  AJTRUE,  AJTRUE,  AJTRUE,  reportWriteSeqTable},
    {"srs",       "Simple report format for SRS",
	 AJFALSE, 0, AJFALSE, AJTRUE,  AJTRUE,  AJTRUE,  reportWriteSrs},
    {"table",     "Simple table",
	 AJFALSE, 0, AJFALSE, AJTRUE,  AJTRUE,  AJTRUE,  reportWriteTable},
    {"tagseq",    "Sequence with features marked below",
	 AJFALSE, 0, AJFALSE, AJTRUE,  AJTRUE,  AJTRUE,  reportWriteTagseq},
    {NULL, NULL, AJFALSE, 0, AJFALSE, AJFALSE, AJFALSE, AJFALSE, NULL}
};




/* @funcstatic reportWriteTrace ***********************************************
**
** Writes a report in Trace format
**
** @param [u] thys [AjPReport] Report object
** @param [r] ftable [const AjPFeattable] Feature table object
** @param [r] seq [const AjPSeq] Sequence object
** @return [void]
** @@
******************************************************************************/

static void reportWriteTrace(AjPReport thys, const AjPFeattable ftable,
			     const AjPSeq seq)
{
    ajReportWriteHeader(thys, ftable, seq);

    ajFmtPrintF(thys->File, "Trace output\n");

    ajFmtPrintF(thys->File, "Name: '%S'\n", thys->Name);
    ajFmtPrintF(thys->File, "Type: '%S'\n", thys->Type);
    ajFmtPrintF(thys->File, "Formatstr: '%S'\n", thys->Formatstr);

    ajFmtPrintF(thys->File, "Feattable (internal): %u features\n",
		ajFeattableGetSize(thys->Fttable));

    if (thys->Fttable)
	ajFeattablePrint(thys->Fttable, thys->File);

    ajFmtPrintF(thys->File, "FtQuery: '%S'\n",
		ajFeattabOutFilename(thys->Ftquery));

    ajFmtPrintF(thys->File, "Extension: '%S'\n", thys->Extension);
    ajFmtPrintF(thys->File, "File: '%F'\n", thys->File);
    ajFmtPrintF(thys->File, "Tagnames: %u\n",
		ajListGetLength(thys->Tagnames));
    ajFmtPrintF(thys->File, "Tagprints: %u\n",
		ajListGetLength(thys->Tagprints));
    ajFmtPrintF(thys->File, "Tagtypes: %u\n",
		ajListGetLength(thys->Tagtypes));
/*   ajFmtPrintF(thys->File, "Header: '%S'\n", thys->Header);*/
/*   ajFmtPrintF(thys->File, "SubHeader: '%S'\n", thys->SubHeader);*/
/*   ajFmtPrintF(thys->File, "Tail: '%S'\n", thys->Tail);*/
/*   ajFmtPrintF(thys->File, "SubTail: '%S'\n", thys->SubTail);*/
    ajFmtPrintF(thys->File, "FileNames: %u\n",
		ajListGetLength(thys->FileNames));
    ajFmtPrintF(thys->File, "FileTypes: %u\n",
		ajListGetLength(thys->FileTypes));
    ajFmtPrintF(thys->File, "Precision: %d\n", thys->Precision);
    ajFmtPrintF(thys->File, "Showacc: %B\n", thys->Showacc);
    ajFmtPrintF(thys->File, "Showdes: %B\n", thys->Showdes);
    ajFmtPrintF(thys->File, "Showusa: %B\n", thys->Showusa);
    ajFmtPrintF(thys->File, "Showscore: %B\n", thys->Showscore);
    ajFmtPrintF(thys->File, "Showstrand: %B\n", thys->Showstrand);
    ajFmtPrintF(thys->File, "Multi: %B\n", thys->Multi);
    ajFmtPrintF(thys->File, "Mintags: %d\n", thys->Mintags);
    ajFmtPrintF(thys->File, "CountSeq: %d\n", thys->CountSeq);
    ajFmtPrintF(thys->File, "CountHit: %d\n", thys->CountHit);
    ajFmtPrintF(thys->File, "TotHits: %d\n", thys->TotHits);
    ajFmtPrintF(thys->File, "MaxHitAll: %d\n", thys->MaxHitAll);
    ajFmtPrintF(thys->File, "MaxHitSeq: %d\n", thys->MaxHitSeq);
    ajFmtPrintF(thys->File, "Format: %d\n", thys->Format);


    ajFmtPrintF(thys->File, "\nFeattable (external): %u features\n",
		ajFeattableGetSize(ftable));
    ajFeattablePrint(ftable, thys->File);

    ajReportWriteTail(thys, ftable, seq);

    return;
}




/* @funcstatic reportWriteEmbl ************************************************
**
** Writes a report in EMBL format
**
** @param [u] thys [AjPReport] Report object
** @param [r] ftable [const AjPFeattable] Feature table object
** @param [r] seq [const AjPSeq] Sequence object
** @return [void]
** @@
******************************************************************************/

static void reportWriteEmbl(AjPReport thys,
			    const AjPFeattable ftable, const AjPSeq seq)
{
    /*  ajFmtPrintF(thys->File, "#EMBL output\n"); */

    ajFeattabOutDel(&thys->Ftquery);
    thys->Ftquery = ajFeattabOutNewCSF("embl", ajSeqGetNameS(seq),
				       ajStrGetPtr(thys->Type),
				       thys->File);
    if(!ajFeattableWrite(thys->Ftquery, ftable))
	ajWarn("ajReportWriteEmbl features output failed format: 'embl'");

    return;
}




/* @funcstatic reportWriteGenbank *********************************************
**
** Writes a report in Genbank format
**
** @param [u] thys [AjPReport] Report object
** @param [r] ftable [const AjPFeattable] Feature table object
** @param [r] seq [const AjPSeq] Sequence object
** @return [void]const 
** @@
******************************************************************************/

static void reportWriteGenbank(AjPReport thys,
			       const AjPFeattable ftable, const AjPSeq seq)
{
    /* ajFmtPrintF(thys->File, "#Genbank output\n"); */

    ajFeattabOutDel(&thys->Ftquery);
    thys->Ftquery = ajFeattabOutNewCSF("genbank", ajSeqGetNameS(seq),
				       ajStrGetPtr(thys->Type),
				       thys->File);

    if(!ajFeattableWrite(thys->Ftquery, ftable))
	ajWarn("ajReportWriteGenbank features output failed format: 'genbank'");

    return;
}




/* @funcstatic reportWriteGff *************************************************
**
** Writes a report in GFF format
**
** @param [u] thys [AjPReport] Report object
** @param [r] ftable [const AjPFeattable] Feature table object
** @param [r] seq [const AjPSeq] Sequence object
** @return [void]
** @@
******************************************************************************/

static void reportWriteGff(AjPReport thys,
			   const AjPFeattable ftable, const AjPSeq seq)
{
    ajFeattabOutDel(&thys->Ftquery);
    thys->Ftquery = ajFeattabOutNewCSF("gff", ajSeqGetNameS(seq),
				       ajStrGetPtr(thys->Type),
				       thys->File);

    if(!ajFeattableWrite(thys->Ftquery, ftable))
	ajWarn("ajReportWriteGff features output failed format: 'gff'");

    return;
}




/* @funcstatic reportWritePir *************************************************
**
** Writes a report in PIR format
**
** @param [u] thys [AjPReport] Report object
** @param [r] ftable [const AjPFeattable] Feature table object
** @param [r] seq [const AjPSeq] Sequence object
** @return [void]
** @@
******************************************************************************/

static void reportWritePir(AjPReport thys,
			   const AjPFeattable ftable, const AjPSeq seq)
{
    ajFeattabOutDel(&thys->Ftquery);
    thys->Ftquery = ajFeattabOutNewCSF("pir", ajSeqGetNameS(seq),
				       ajStrGetPtr(thys->Type),
				       thys->File);
    if(!ajFeattableWrite(thys->Ftquery, ftable))
	ajWarn("ajReportWritePir features output failed format: 'pir'");

    return;
}




/* @funcstatic reportWriteSwiss ***********************************************
**
** Writes a report in SwissProt format
**
** @param [u] thys [AjPReport] Report object
** @param [r] ftable [const AjPFeattable] Feature table object
** @param [r] seq [const AjPSeq] Sequence object
** @return [void]
** @@
******************************************************************************/

static void reportWriteSwiss(AjPReport thys,
			     const AjPFeattable ftable, const AjPSeq seq)
{
    ajFeattabOutDel(&thys->Ftquery);
    thys->Ftquery = ajFeattabOutNewCSF("swissprot", ajSeqGetNameS(seq),
				       ajStrGetPtr(thys->Type),
				       thys->File);
    if(!ajFeattableWrite(thys->Ftquery, ftable))
	ajWarn("ajReportWriteSwiss features output failed format: 'swissprot'");

    return;
}




/* @funcstatic reportWriteDasgff **********************************************
**
** Writes a report in DAS GFF format
**
** @param [u] thys [AjPReport] Report object
** @param [r] ftable [const AjPFeattable] Feature table object
** @param [r] seq [const AjPSeq] Sequence object
** @return [void]
** @@
******************************************************************************/

static void reportWriteDasgff(AjPReport thys,
                              const AjPFeattable ftable, const AjPSeq seq)
{
    if(!thys->CountSeq)
    {
        ajFeattabOutDel(&thys->Ftquery);
        thys->Ftquery = ajFeattabOutNewCSF("dasgff", ajSeqGetNameS(seq),
                                           ajStrGetPtr(thys->Type),
                                           thys->File);
    }
    else
        ajFeattabOutSetSeqname(thys->Ftquery, ajSeqGetNameS(seq));

    if(!ajFeattableWrite(thys->Ftquery, ftable))
	ajWarn("ajReportWriteDasgff features output failed format: 'dasgff'");

    return;
}




/* @funcstatic reportWriteDbMotif *********************************************
**
** Writes a report in DbMotif format
**
** Format:<br>
** Length = [length] <br>
** Start = position [start] of sequence <br>
** End = position [start] of sequence <br>
** ... other tags ... <br>
** [sequence] <br>
** [start and end numbered below sequence] <br>
**
** Data reported: Length, Start, End, Sequence (5 bases around feature)
**
** Tags required: None
**
** Tags used: None
**
** Tags reported: <br>
**   all tags reported as name = value
**
** @param [u] thys [AjPReport] Report object
** @param [r] ftable [const AjPFeattable] Feature table object
** @param [r] seq [const AjPSeq] Sequence object
** @return [void]
** @@
******************************************************************************/

static void reportWriteDbMotif(AjPReport thys,
			       const AjPFeattable ftable, const AjPSeq seq)
{
    AjPFile outf;
    AjIList iterft     = NULL;
    AjPFeature feature = NULL;
    ajuint istart  = 0;
    ajuint iend    = 0;
    ajuint ilen    = 0;
    AjPStr subseq = NULL;
    AjPStr seqstr = NULL;
    AjPStr tmpstr = NULL;
    
    ajuint ntags;
    static AjPStr* tagtypes = NULL;
    static AjPStr* tagnames = NULL;
    static AjPStr* tagprints = NULL;
    static ajuint*  tagsizes = NULL;
    ajuint j = 0;
    AjPStr tagval = NULL;
    ajuint jstart;
    ajuint jend;
    char strand;
    const AjPStr seqalias;

    outf = thys->File;

    
    if(ajSeqIsReversedTrue(seq))
    {
	ajStrAssignS(&seqstr, ajSeqGetSeqS(seq));
	ajSeqstrReverse(&seqstr);
	seqalias = seqstr;
    }
    else
	seqalias = ajSeqGetSeqS(seq);

    ajReportWriteHeader(thys, ftable, seq);
    
    ntags = reportGetTags(thys, &tagtypes, &tagnames, &tagprints, &tagsizes);
    
    iterft = ajListIterNewread(ftable->Features);

    while(!ajListIterDone(iterft))
    {
	feature = (AjPFeature)ajListIterGet(iterft);
	strand = ajFeatGetStrand(feature);
	istart = feature->Start;
	iend   = feature->End;
	ilen    = iend - istart + 1;

	jstart = AJMAX(6, istart);
	jstart -= 6;
	jend = AJMIN(ajSeqGetLen(seq), iend+4);

	ajStrAssignResC(&tmpstr, ilen+10, "");
	j=istart+1;

	while(j++<iend)
	    ajStrAppendK(&tmpstr, ' ');

	ajStrAssignSubS(&subseq, seqalias, jstart, jend);

	/* ajStrFmtUpper(&subseq); */
	ajFmtPrintF(outf, "Length = %d\n", ilen);
	ajFmtPrintF(outf, "Start = position %d of sequence\n", istart);
	ajFmtPrintF(outf, "End = position %d of sequence\n\n", iend);

	if(strand == '-')
	{
	    ajFmtPrintF(outf, "Strand = Reversed\n");
	}

	for(j=0; j < ntags; j++)
	    if(ajFeatGetNoteS(feature, tagnames[j], &tagval))
		ajFmtPrintF(outf, "%S = %S\n", tagprints[j], tagval);

	ajFmtPrintF(outf, "\n");
	ajFmtPrintF(outf, "%S\n", subseq);

	if(istart == iend)
	{
	    ajFmtPrintF(outf, "%*s|\n",(istart-jstart-1), "");
	    ajFmtPrintF(outf, "%*d\n\n", AJMIN(6, istart), istart);
	}
	else
	{
	    ajFmtPrintF(outf, "%*s|%S|\n", (istart-jstart-1), "", tmpstr);
	    ajFmtPrintF(outf, "%*d%S%d\n\n",
			AJMIN(6, istart),
			istart, tmpstr, iend);
	}

	ajStrDel(&tmpstr);
    }
    
    ajStrDel(&subseq);
    ajStrDel(&seqstr);
    ajStrDel(&tagval);
    
    ajListIterDel(&iterft);
    
    ajReportWriteTail(thys, ftable, seq);

    AJFREE(tagtypes);
    AJFREE(tagnames);
    AJFREE(tagprints);
    AJFREE(tagsizes);

    return;
}




/* @funcstatic reportWriteDiffseq *********************************************
**
** Writes a report in Diffseq format, based on the output from the
** diffseq application. The report describes matches, usually short,
** between two sequences and features which overlap them.
**
** A number of tags are used. The report makes little sense without them.
** These tags are used to replicate features in a second sequence.
**
** Format:<br>
**   [Name] [start]-[end] Length: [length] <br>
**   Feature: [special first_feature tag] <br>
**   Sequence: [sequence] <br>
**   Sequence: [special sequence tag] <br>
`**   Feature: [special second_feature tag] <br>
**   [as first line, using special tags name, start, end, length] <br>
**
** Data reported: Name, Start, End, Length, Sequence
**
** Tags required: None
**
** Tags used: <br>
**   start : start position in second sequence <br>
**   end : end in second sequence <br>
**   length : length of match in second sequence <br>
**   name : name of second sequence (set by ajReportGetSeqnameSeq) <br>
**   sequence : sequence of match in second sequence <br>
**   first_feature : feature(s) in first sequence <br>
**   second_feature : feature(s) in second sequence <br>
**
** Tags reported: None
**
** @param [u] thys [AjPReport] Report object
** @param [r] ftable [const AjPFeattable] Feature table object
** @param [r] seq [const AjPSeq] Sequence object
** @return [void]
** @@
******************************************************************************/

static void reportWriteDiffseq(AjPReport thys,
			       const AjPFeattable ftable, const AjPSeq seq)
{
    AjPFile outf;
    AjIList iterft     = NULL;
    AjPFeature feature = NULL;

    ajuint istart = 0;
    ajuint jstart = 0;
    ajuint iend   = 0;
    ajuint jend   = 0;
    ajuint ilen   = 0;
    ajuint jlen   = 0;

    AjPStr subseq = NULL;
    AjPStr seqstr = NULL;
    ajuint i       = 0;
    ajuint ifeat   = 0;
    AjPStr tmpstr = NULL;
    AjPStr jname  = NULL;

    AjPStr firstfeat  = NULL;
    AjPStr secondfeat = NULL;
    AjPStr tagval     = NULL;
    char strand;
    const AjPStr seqalias;

    outf = thys->File;
    
    if(ajSeqIsReversedTrue(seq))
    {
	ajStrAssignS(&seqstr, ajSeqGetSeqS(seq));
	ajSeqstrReverse(&seqstr);
	seqalias = seqstr;
    }
    else
	seqalias = ajSeqGetSeqS(seq);

     ajReportWriteHeader(thys, ftable, seq);
    
    iterft = ajListIterNewread(ftable->Features);

    while(!ajListIterDone(iterft))
    {
	feature = (AjPFeature)ajListIterGet(iterft);
	strand = ajFeatGetStrand(feature);
	istart  = feature->Start;
	iend    = feature->End;
	ilen    = iend - istart + 1;
	ajStrAssignSubS(&subseq, seqalias, istart-1, iend-1);
	/* ajStrFmtUpper(&subseq); */
	i++;
	
	if(!ajFeatGetNoteC(feature, "start", &tagval))
	    jstart = 0;
	else
	    ajStrToUint(tagval, &jstart);

	
	if(!ajFeatGetNoteC(feature, "end", &tagval))
	    jend = 0;
	else
	    ajStrToUint(tagval, &jend);
	
	if(!ajFeatGetNoteC(feature, "length", &tagval))
	    jlen = 0;
	else
	    ajStrToUint(tagval, &jlen);
	
	if(!ajFeatGetNoteC(feature, "name", &jname))
	    ajStrAssignClear(&jname);
	
	if(ilen > 0)
	{
	    if(strand == '-')
		ajFmtPrintF(outf, "\n%S %d-%d Length: %d (Reversed)\n",
			    ajReportGetSeqnameSeq(thys, seq),
                            istart, iend, ilen);
	    else
		ajFmtPrintF(outf, "\n%S %d-%d Length: %d\n",
			    ajReportGetSeqnameSeq(thys, seq),
                            istart, iend, ilen);
	    ifeat = 1;

	    while(ajFeatGetNoteCI(feature, "first_feature",
				  ifeat++, &firstfeat))
		ajFmtPrintF(outf, "Feature: %S\n", firstfeat);

	    ajFmtPrintF(outf, "Sequence: %S\n", subseq);
	}
	else
	{
	    ajFmtPrintF(outf, "\n%S %d Length: %d\n",
			ajReportGetSeqnameSeq(thys, seq),
                        istart, ilen);
	    ifeat = 1;

	    while(ajFeatGetNoteCI(feature, "first_feature",
				  ifeat++, &firstfeat))
		ajFmtPrintF(outf, "Feature: %S\n", firstfeat);

	    ajFmtPrintF(outf, "Sequence: \n");
	}
	
	if(!ajFeatGetNoteC(feature, "sequence", &subseq))
	    ajStrAssignClear(&subseq);
	
	if(jlen > 0)
	{
	    ajFmtPrintF(outf, "Sequence: %S\n", subseq);
	    ifeat = 1;

	    while(ajFeatGetNoteCI(feature, "second_feature",
				  ifeat++, &secondfeat))
		ajFmtPrintF(outf, "Feature: %S\n", secondfeat);

	    ajFmtPrintF(outf, "%S %d-%d Length: %d\n",
			jname, jstart, jend, jlen);
	}
	else
	{
	    ajFmtPrintF(outf, "Sequence: \n");
	    ifeat = 1;

	    while(ajFeatGetNoteCI(feature, "second_feature",
				  ifeat++, &secondfeat))
		ajFmtPrintF(outf, "Feature: %S\n", secondfeat);

	    ajFmtPrintF(outf, "%S %d Length: %d\n",
			jname, jstart, jlen);
	}
	
	ajStrDelStatic(&tmpstr);
    }
    
    ajReportWriteTail(thys, ftable, seq);
    
    ajStrDel(&subseq);
    ajStrDel(&seqstr);
    ajStrDel(&tmpstr);
    ajStrDel(&tagval);
    ajStrDel(&jname);
    
    ajListIterDel(&iterft);

    return;
}




/* @funcstatic reportWriteDraw ************************************************
**
** Writes a report in Draw format, for use as input to cirdna or lindna
**
** Format:<br>
**   group<br>
**<br>
**   label<br>
**   tick  [tagvalue] [start] 8<br>
**   endlabel<br>
**   label<br>
**   tick  [tagvalue] [end] 3<br>
**   endlabel<br>
**<br>
**   endgroup<br>
**
** Data reported:
**
** Tags required:
**
** Tags used:
**
** Tags reported:
**
** @param [u] thys [AjPReport] Report object
** @param [r] ftable [const AjPFeattable] Feature table object
** @param [r] seq [const AjPSeq] Sequence object
** @return [void]
** @@
******************************************************************************/

static void reportWriteDraw(AjPReport thys,
			    const AjPFeattable ftable, const AjPSeq seq)
{
    AjPFile outf;
    AjIList iterft     = NULL;
    AjPFeature feature = NULL;
    AjPStr subseq = NULL;
    AjPStr tagval = NULL;
    ajint istart  = 0;
    ajint iend    = 0;

    outf = thys->File;
    
    ajReportWriteHeader(thys, ftable, seq);
    
    ajFmtPrintF(outf, "Start %d\n", 
 		ajSeqGetBegin(seq) + ajSeqGetOffset(seq));
    ajFmtPrintF(outf, "End   %d\n", 
 		ajSeqGetEnd(seq) + ajSeqGetOffset(seq));

    ajFmtPrintF(outf, "\n"); 
    ajFmtPrintF(outf, "group\n");
    
    iterft = ajListIterNewread(ftable->Features);

    while(!ajListIterDone(iterft))
    {
	feature = (AjPFeature)ajListIterGet(iterft);
	/*strand = ajFeatGetStrand(feature);*/
	istart = feature->Start;
	iend   = feature->End;

	ajFmtPrintF(outf, "label\n");

        ajFmtPrintF(outf, "Range %d %d | | 9 H\n", istart, iend);
        ajFmtPrintF(outf, "%S\n", ajFeatGetType(feature));

        ajFmtPrintF(outf, "endlabel\n");

	ajFmtPrintF(outf, "\n");
    }
    
    ajFmtPrintF(outf, "endgroup\n");

    ajReportWriteTail(thys, ftable, seq);
    
    ajStrDel(&subseq);
    ajStrDel(&tagval);
    
    ajListIterDel(&iterft);

    return;
}




/* @funcstatic reportWriteRestrict ********************************************
**
** Writes a report in Draw format, for use as input to cirdna or lindna
** specialised to select data from restriction sites by selecting feature tags
**
** Format:<br>
**   group<br>
**<br>
**   label<br>
**   tick  [tagvalue] [start] 8<br>
**   endlabel<br>
**   label<br>
**   tick  [tagvalue] [end] 3<br>
**   endlabel<br>
**<br>
**   endgroup<br>
**
** Data reported:
**
** Tags required: Enzyme_name, 5prime, 3primem 5primerev, 3primerev
**
** Tags used:
**
** Tags reported:
**
** @param [u] thys [AjPReport] Report object
** @param [r] ftable [const AjPFeattable] Feature table object
** @param [r] seq [const AjPSeq] Sequence object
** @return [void]
** @@
******************************************************************************/

static void reportWriteRestrict(AjPReport thys,
                                const AjPFeattable ftable, const AjPSeq seq)
{
    AjPFile outf;
    AjIList iterft     = NULL;
    AjPFeature feature = NULL;
    AjPStr subseq = NULL;
    ajuint ntags;
    static AjPStr* tagtypes = NULL;
    static AjPStr* tagnames = NULL;
    static AjPStr* tagprints = NULL;
    static ajuint*  tagsizes = NULL;
    ajuint j = 0;
    AjPStr tagval = NULL;
    ajint jenz = -1;
    ajint j5 = -1;
    ajint j3 = -1;
    ajuint jstart;
    ajuint jend;
    outf = thys->File;
    
    ajReportWriteHeader(thys, ftable, seq);
    
    ntags = reportGetTags(thys, &tagtypes, &tagnames, &tagprints, &tagsizes);
    
    for(j=0; j < ntags; j++)
    {
	if(ajStrMatchCaseC(tagnames[j], "enzyme"))
	    jenz = j;

	if(ajStrMatchCaseC(tagnames[j], "5prime"))
	    j5 = j;

	if(ajStrMatchCaseC(tagnames[j], "3prime"))
	    j3 = j;
    }

    ajFmtPrintF(outf, "Start %d\n", 
 		ajSeqGetBegin(seq) + ajSeqGetOffset(seq));
    ajFmtPrintF(outf, "End   %d\n", 
 		ajSeqGetEnd(seq) + ajSeqGetOffset(seq));

    ajFmtPrintF(outf, "\n"); 

    ajFmtPrintF(outf, "group\n");
    ajFmtPrintF(outf, "Forward\n");
    
    iterft = ajListIterNewread(ftable->Features);

    while(!ajListIterDone(iterft))
    {
	feature = (AjPFeature)ajListIterGet(iterft);

	if (j5 >= 0)
	{
            ajFmtPrintF(outf, "label\n");

	    ajFeatGetNoteS(feature, tagnames[j5], &tagval);
	    ajStrToUint(tagval, &jstart);
	    ajFmtPrintF(outf, "Tick %d 8\n", jstart);

	    if (jenz >= 0)
	    {
		ajFeatGetNoteS(feature, tagnames[jenz], &tagval);
		ajFmtPrintF(outf, "%S\n", tagval);
	    }
	    else
		ajFmtPrintF(outf, "Enz\n");

	    ajFmtPrintF(outf, "endlabel\n");
	}

	ajFmtPrintF(outf, "\n");
    }
    
    ajListIterDel(&iterft);

    ajFmtPrintF(outf, "endgroup\n");
    ajFmtPrintF(outf, "group\n");
    ajFmtPrintF(outf, "Reverse\n");
   
    iterft = ajListIterNewread(ftable->Features);

    while(!ajListIterDone(iterft))
    {
	feature = (AjPFeature)ajListIterGet(iterft);

	if (j3 >= 0)
	{
	    ajFmtPrintF(outf, "label\n");

	    ajFeatGetNoteS(feature, tagnames[j3], &tagval);
	    ajStrToUint(tagval, &jend);
	    ajFmtPrintF(outf, "Tick %d 3\n", jend);

	    if (jenz >= 0)
	    {
		ajFeatGetNoteS(feature, tagnames[jenz], &tagval);
		ajFmtPrintF(outf, "%S\n", tagval);
	    }
	    else
		ajFmtPrintF(outf, "Enz\n");

	    ajFmtPrintF(outf, "endlabel\n");
	}

	ajFmtPrintF(outf, "\n");
    }
    
    ajListIterDel(&iterft);

    ajFmtPrintF(outf, "endgroup\n");
    ajReportWriteTail(thys, ftable, seq);
    
    ajStrDel(&subseq);
    ajStrDel(&tagval);
    

    AJFREE(tagtypes);
    AJFREE(tagnames);
    AJFREE(tagprints);
    AJFREE(tagsizes);

    return;
}




/* @funcstatic reportWriteExcel ***********************************************
**
** Writes a report in Excel (tab delimited) format. Name, start, end
** score and strand are always reported. Other tags in the report definition
** are added as extra columns.
**
** All values are (for now) unquoted. Missing values are reported as '.'
**
** Format:<br>
**   "SeqName Start End Score" (tab delimited) <br>
**   [extra tag names added to first line] <br>
**   Name Start End Score [extra tag values] (tab delimited) <br>
**
** Data reported: Name Start End Score Strand
**
** Tags required: None
**
** Tags used: None
**
** Tags reported: All defined tags
**
** @param [u] thys [AjPReport] Report object
** @param [r] ftable [const AjPFeattable] Feature table object
** @param [r] seq [const AjPSeq] Sequence object
** @return [void]
** @@
******************************************************************************/

static void reportWriteExcel(AjPReport thys,
			     const AjPFeattable ftable, const AjPSeq seq)
{
    AjPFile outf;
    AjIList iterft     = NULL;
    AjPFeature feature = NULL;
    ajuint istart  = 0;
    ajuint iend    = 0;
    float score   = 0.0;
    AjPStr subseq = NULL;
    AjPStr tmpstr = NULL;
    ajuint i = 0;
    ajuint j = 0;
    char strand;
    
    ajuint ntags;
    static AjPStr* tagtypes = NULL;
    static AjPStr* tagnames = NULL;
    static AjPStr* tagprints = NULL;
    static ajuint*  tagsizes = NULL;

    AjPStr tagval = NULL;

    outf = thys->File;
    
    ntags = reportGetTags(thys, &tagtypes, &tagnames, &tagprints, &tagsizes);
    
    if(thys->Showscore)
	ajFmtPrintF(outf, "SeqName\tStart\tEnd\tScore\tStrand");
    else
	ajFmtPrintF(outf, "SeqName\tStart\tEnd\tStrand");
    
    /* then extra tags */
    for(j=0; j < ntags; j++)
	ajFmtPrintF(outf, "\t%S", tagprints[j]);

    ajFmtPrintF(outf, "\n");
    
    iterft = ajListIterNewread(ftable->Features);

    while(!ajListIterDone(iterft))
    {
	feature = (AjPFeature)ajListIterGet(iterft);
	istart  = feature->Start;
	iend    = feature->End;
	score   = feature->Score;
	strand = feature->Strand;

	if(strand != '-')
	    strand = '+';

	ajStrAssignSubS(&subseq, ajSeqGetSeqS(seq), istart-1, iend-1);
	/* ajStrFmtUpper(&subseq); */
	i++;

	if(thys->Showscore)
	    ajFmtPrintF(outf, "%S\t%d\t%d\t%.*f\t%c",
			ajReportGetSeqnameSeq(thys, seq),
			istart, iend, thys->Precision,
			score, strand);
	else
	    ajFmtPrintF(outf, "%S\t%d\t%d\t%c",
			ajReportGetSeqnameSeq(thys, seq),
			istart, iend, strand);
	
	for(j=0; j < ntags; j++)
	{				/* then extra tags */
	    if(ajFeatGetNoteS(feature, tagnames[j], &tagval))
		ajFmtPrintF(outf, "\t%S", tagval);
	    else
		ajFmtPrintF(outf, "\t."); /* missing value '.' for now */
	}

	ajFmtPrintF(outf, "\n");
	ajStrDelStatic(&tmpstr);
    }
    
    ajStrDel(&subseq);
    ajStrDel(&tmpstr);
    ajStrDel(&tagval);
    
    ajListIterDel(&iterft);

    AJFREE(tagtypes);
    AJFREE(tagnames);
    AJFREE(tagprints);
    AJFREE(tagsizes);

    return;
}




/* @funcstatic reportWriteFeatTable *******************************************
**
** Writes a report in FeatTable format. The report is an EMBL feature
** table using only the tags in the report definition. There is no
** requirement for tag names to match standards for the EMBL feature
** table.
**
** The original EMBOSS application for this format was cpgreport.
**
** Format:<br>
**   FT [type] [start]..[end] <br>
**                            /[tagname]=[tagvalue] <br>
**
** Data reported: Type, Start, End
**
** Tags required: None
**
** Tags used: None
**
** Tags reported: All
**
** @param [u] thys [AjPReport] Report object
** @param [r] ftable [const AjPFeattable] Feature table object
** @param [r] seq [const AjPSeq] Sequence object
** @return [void]
** @@
******************************************************************************/

static void reportWriteFeatTable(AjPReport thys,const  AjPFeattable ftable,
				 const AjPSeq seq)
{
    AjPFile outf;
    AjIList iterft     = NULL;
    AjPFeature feature = NULL;
    ajuint istart  = 0;
    ajuint iend    = 0;
    AjPStr subseq = NULL;
    
    ajuint ntags;
    static AjPStr* tagtypes = NULL;
    static AjPStr* tagnames = NULL;
    static AjPStr* tagprints = NULL;
    static ajuint*  tagsizes = NULL;
    ajuint j = 0;
    AjPStr tagval = NULL;

    outf = thys->File;
    
    ntags = reportGetTags(thys, &tagtypes, &tagnames, &tagprints, &tagsizes);
    
    iterft = ajListIterNewread(ftable->Features);

    while(!ajListIterDone(iterft))
    {
	feature = (AjPFeature)ajListIterGet(iterft);
	istart = feature->Start;
	iend = feature->End;
	ajStrAssignSubS(&subseq, ajSeqGetSeqS(seq), istart-1, iend-1);
	/* ajStrFmtUpper(&subseq); */

	if(feature->Strand == '-')
	    ajFmtPrintF(outf, "FT   %-15.15S complement(%d..%d)\n",
			ajFeatGetType(feature),
			istart, iend);
	else
	    ajFmtPrintF(outf, "FT   %-15.15S %d..%d\n",
			ajFeatGetType(feature),
			istart, iend);

	for(j=0; j < ntags; j++)
	{
	    if(ajFeatGetNoteS(feature, tagnames[j], &tagval))
		ajFmtPrintF(outf, "FT                   /%S=\"%S\"\n",
			    tagprints[j], tagval);
	    else
	    {
		/* skip the missing ones for now */
		/*ajFmtPrintF(outf,
		  "FT                   /%S=\"<unknown>\"\n",
		  tagprints[j]);*/
	    }
	}
    }
    
    ajStrDel(&subseq);
    ajStrDel(&tagval);
    
    ajListIterDel(&iterft);

    AJFREE(tagtypes);
    AJFREE(tagnames);
    AJFREE(tagprints);
    AJFREE(tagsizes);

    return;
}




/* @funcstatic reportWriteListFile ********************************************
**
** Writes a report in ListFile format for use as input to another application.
**
** Format:<br>
**   Name[start:end] <br>
**
** Data reported: Name, Start, End, Strand
**
** Tags required: None
**
** Tags used: None
**
** Tags reported: None
**
** @param [u] thys [AjPReport] Report object
** @param [r] ftable [const AjPFeattable] Feature table object
** @param [r] seq [const AjPSeq] Sequence object
** @return [void]
** @@
******************************************************************************/

static void reportWriteListFile(AjPReport thys,
				const AjPFeattable ftable, const AjPSeq seq)
{
    AjPFile outf;
    AjIList iterft     = NULL;
    AjPFeature feature = NULL;
    ajuint istart = 0;
    ajuint iend   = 0;
    ajuint i = 0;
    ajlong ipos;
    ajint jpos;
    AjPStr tmpstr = NULL;
    AjPStr tmpname = NULL;

    outf = thys->File;
    thys->Showusa = ajTrue;		/* so we get a usable USA */

    ajReportWriteHeader(thys, ftable, seq);

    iterft = ajListIterNewread(ftable->Features);

    while(!ajListIterDone(iterft))
    {
	feature = (AjPFeature)ajListIterGet(iterft);
	istart = feature->Start;
	iend = feature->End;
	i++;

	ajFmtPrintS(&tmpstr, "[");

	if(istart)
	    ajFmtPrintAppS(&tmpstr, "%d", istart);

	ajFmtPrintAppS(&tmpstr, ":");

	if(iend)
	    ajFmtPrintAppS(&tmpstr, "%d", iend);

	if(feature->Strand == '-')
	    ajFmtPrintAppS(&tmpstr, ":r");

	ajFmtPrintAppS(&tmpstr, "]");

	ajStrAssignS(&tmpname, ajSeqGetUsaS(seq));

	if(ajStrGetCharLast(tmpname) == ']')
        {
	    jpos = ajStrGetLen(tmpname);
	    ipos = ajStrFindlastC(tmpname, "[");
	    ajStrCutRange(&tmpname, ipos, jpos);
	}

	if(ajStrGetLen(tmpstr) > 3)
	    ajFmtPrintF(outf, "%S%S\n",
			tmpname, tmpstr);
	else
	    ajFmtPrintF(outf, "%S\n",
			tmpname);

	ajStrDelStatic(&tmpstr);
    }

    ajReportWriteTail(thys, ftable, seq);

    ajListIterDel(&iterft);
    ajStrDel(&tmpstr);
    ajStrDel(&tmpname);

    return;
}




/* @funcstatic reportWriteMotif ***********************************************
**
** Writes a report in Motif format.  Based on the original output
** format of antigenic, helixturnhelix and sigcleave.
**
** Format:<br>
**   (1) Score [score] length [length] at [name] [start->[end] <br>
**               *  (marked at position pos) <br>
**             [sequence] <br>
**             |        | <br>
**       [start]        [end] <br>
**   [tagname]: tagvalue
**
** Data reported: Name, Start, End, Length, Score, Sequence
**
** Tags required: None
**
** Tags used: <br>
**   pos (integer, maximum score position)
**
** Tags reported: All
**
** @param [u] thys [AjPReport] Report object
** @param [r] ftable [const AjPFeattable] Feature table object
** @param [r] seq [const AjPSeq] Sequence object
** @return [void]
** @@
******************************************************************************/

static void reportWriteMotif(AjPReport thys,
			     const AjPFeattable ftable, const AjPSeq seq)
{
    AjPFile outf;
    AjIList iterft     = NULL;
    AjPFeature feature = NULL;
    ajuint istart  = 0;
    ajuint iend    = 0;
    float score   = 0.0;
    ajuint ilen    = 0;
    AjPStr subseq = NULL;
    AjPStr seqstr = NULL;
    ajuint i = 0;
    AjPStr tmpstr = NULL;
    
    ajuint ntags;
    static AjPStr* tagtypes = NULL;
    static AjPStr* tagnames = NULL;
    static AjPStr* tagprints = NULL;
    static ajuint*  tagsizes = NULL;
    ajuint j = 0;
    ajuint jstar = UINT_MAX;
    ajuint jplus = UINT_MAX;
    AjPStr tagval = NULL;
    
    ajuint jpos = 0;
    ajuint jpos2 = 0;
    ajuint jposmin = 0;
    ajuint jposmax = 0;
    ajuint jmax = UINT_MAX;
    char strand = '+';
    const AjPStr seqalias;
    
    outf = thys->File;

    if(ajSeqIsReversedTrue(seq))
    {
	ajStrAssignS(&seqstr, ajSeqGetSeqS(seq));
	ajSeqstrReverse(&seqstr);
	seqalias = seqstr;
    }
    else
	seqalias = ajSeqGetSeqS(seq);

    ajReportWriteHeader(thys, ftable, seq);
    
    ntags = reportGetTags(thys, &tagtypes, &tagnames, &tagprints, &tagsizes);

    for(j=0; j < ntags; j++)
    {
	if(ajStrMatchCaseC(tagnames[j], "pos"))
	{
	    jstar = j;
	    ajFmtPrintF(outf, "%S at \"*\"\n", tagprints[jstar]);
	}

	if(ajStrMatchCaseC(tagnames[j], "pos2"))
	{
	    jplus = j;
	    ajFmtPrintF(outf, "%S at \"+\"\n", tagprints[jplus]);
	}
    }

    if(jstar != UINT_MAX || jplus != UINT_MAX)
	ajFmtPrintF(outf, "\n");
	
    
    iterft = ajListIterNewread(ftable->Features);

    while(!ajListIterDone(iterft))
    {
	feature = (AjPFeature)ajListIterGet(iterft);
	istart = feature->Start;
	iend = feature->End;
	score = feature->Score;
	ilen = iend - istart + 1;
	ajStrAssignSubS(&subseq, seqalias, istart-1, iend-1);
	/* ajStrFmtUpper(&subseq); */
	i++;
	ajFmtPrintF(outf, "(%d)", i);

	if(thys->Showscore)
	    ajFmtPrintF(outf, " Score %.*f", thys->Precision, score);

	ajFmtPrintF(outf, " length %d at %s %d->%d",
		    ilen, reportCharname(thys), istart, iend);

	if(strand == '-')
	    ajFmtPrintF(outf, " (Reversed)");

	ajFmtPrintF(outf, "\n");

	jposmin = 0;
	jposmax = 0;
	jpos=0;
	jpos2=0;

	if(jstar != UINT_MAX)
	{
	    if(ajFeatGetNoteS(feature, tagnames[jstar], &tagval))
	    {
		ajStrToUint(tagval, &jpos);
		jposmin = jposmax = jpos;
	    }
	}
	
	if(jplus != UINT_MAX)
	{
	    if(ajFeatGetNoteS(feature, tagnames[jplus], &tagval))
	    {
		ajStrToUint(tagval, &jpos2);

		if(jpos2 < jpos)
		    jposmin = jpos2;
		else
		    jposmax = jpos2;
	    }
	}

	if(jposmin > iend)
	    jposmin=iend;

	if(jposmax > iend)
	    jposmax=jposmin;

	ajStrAssignResC(&tmpstr, ilen,"");

	for(j=istart; j<jposmin; j++)
	    ajStrAppendK(&tmpstr, ' ');

	if(jpos == j)
	    ajStrAppendK(&tmpstr, '*');
	else if(jpos2 == j)
	    ajStrAppendK(&tmpstr, '+');

	if(jposmax > jposmin)
	{
	    for(j++; j<jposmax; j++)
		ajStrAppendK(&tmpstr, ' ');

	    if(jpos == j)
		ajStrAppendK(&tmpstr, '*');
	    else if(jpos2 == j)
		ajStrAppendK(&tmpstr, '+');
	}

	ajFmtPrintF(outf, "           %S\n", tmpstr);

	
	ajFmtPrintF(outf, " Sequence: %S\n", subseq);
	ajStrAssignResC(&tmpstr, ilen,"");

	for(j=istart+1; j<iend; j++)
	    ajStrAppendK(&tmpstr, ' ');

	if(istart == iend)
	{
	    ajFmtPrintF(outf, "           |\n");
	    ajFmtPrintF(outf, "%12d\n", istart);
	}
	else
	{
	    ajFmtPrintF(outf, "           |%S|\n", tmpstr);
	    ajFmtPrintF(outf, "%12d%S%d\n", istart, tmpstr, iend);
	}

	if(ntags)
	    for(j=0; j < ntags; j++)
	    {
		if(j == jmax)
		    continue;

		if(ajFeatGetNoteS(feature, tagnames[j], &tagval))
		    ajFmtPrintF(outf, " %S: %S\n", tagprints[j], tagval);
	    }
	
	ajFmtPrintF(outf, "\n");
	ajStrDelStatic(&tmpstr);
    }
    
    ajReportWriteTail(thys, ftable, seq);
    
    ajStrDel(&subseq);
    ajStrDel(&seqstr);
    ajStrDel(&tmpstr);
    ajStrDel(&tagval);
    
    ajListIterDel(&iterft);

    AJFREE(tagtypes);
    AJFREE(tagnames);
    AJFREE(tagprints);
    AJFREE(tagsizes);

    return;
}




/* @funcstatic reportWriteNameTable *******************************************
**
** Writes a report in NameTable format. See reportWriteSeqTable for a version
** with the sequence. See reportWriteTable for a version without the name,
** as this already appears in the header.
**
** Missing tag values are reported as '.'
** The column width is 6, or longer if the name is longer.
**
** Format:<br>
**   USA    Start   End   Score   [tagnames]
**   [name] [start] [end] [score] [tagvalues]
**
** Data reported:
**
** Tags required: None
**
** Tags used: None
**
** Tags reported:
**
** @param [u] thys [AjPReport] Report object
** @param [r] ftable [const AjPFeattable] Feature table object
** @param [r] seq [const AjPSeq] Sequence object
** @return [void]
** @@
******************************************************************************/

static void reportWriteNameTable(AjPReport thys, const AjPFeattable ftable,
				 const AjPSeq seq)
{
    AjPFile outf;
    AjIList iterft     = NULL;
    AjPFeature feature = NULL;
    ajuint istart  = 0;
    ajuint iend    = 0;
    float score   = 0.0;
    AjPStr subseq = NULL;
    ajuint ntags;
    static AjPStr* tagtypes = NULL;
    static AjPStr* tagnames = NULL;
    static AjPStr* tagprints = NULL;
    static ajuint*  tagsizes = NULL;
    ajuint j = 0;
    AjPStr tagval = NULL;
    static AjPStr tmpstr = NULL;
    char strand;
    AjBool dostrand = ajFalse;

    outf = thys->File;

    if(thys->Showstrand && ajSeqIsNuc(seq))
	dostrand = ajTrue;

    if(!tmpstr)
	ajStrAssignC(&tmpstr, "str");
    
    ajReportWriteHeader(thys, ftable, seq);
    
    ntags = reportGetTags(thys, &tagtypes, &tagnames, &tagprints, &tagsizes);
    
    iterft = ajListIterNewread(ftable->Features);

    while(!ajListIterDone(iterft))
    {
	feature = (AjPFeature)ajListIterGet(iterft);

	for(j=0; j < ntags; j++)
	    if(ajFeatGetNoteS(feature, tagnames[j], &tagval))
		tagsizes[j] = AJMAX(tagsizes[j], ajStrGetLen(tagval));
    }

    ajListIterDel(&iterft);
    ajFmtPrintF(outf, "%-20s %7s %7s", "USA", "Start", "End");

    if(dostrand)
	ajFmtPrintF(outf, " %7s", "Strand");

    if(thys->Showscore)
	ajFmtPrintF(outf,
		    " %7s", "Score");
    
    for(j=0; j < ntags; j++)
    {
	if(ajStrMatchS(tagtypes[j], tmpstr))
	    ajFmtPrintF(outf, " %-*S", tagsizes[j], tagprints[j]);
	else
	    ajFmtPrintF(outf, " %*S", tagsizes[j], tagprints[j]);
    }

    ajFmtPrintF(outf, "\n");
    
    iterft = ajListIterNewread(ftable->Features);

    while(!ajListIterDone(iterft))
    {
	feature =(AjPFeature)ajListIterGet(iterft);
	strand = ajFeatGetStrand(feature);
	istart = feature->Start;
	iend   = feature->End;

	score = feature->Score;
	ajStrAssignSubS(&subseq, ajSeqGetSeqS(seq), istart-1, iend-1);
	/* ajStrFmtUpper(&subseq); */
	
	ajFmtPrintF(outf, "%-20S %7d %7d",
			ajReportGetSeqnameSeq(thys, seq),
			istart, iend);
	if(dostrand)
	    ajFmtPrintF(outf, "       %c", strand);

	if(thys->Showscore)
	    ajFmtPrintF(outf, " %7.*f", thys->Precision, score);

	for(j=0; j < ntags; j++)
	{
	    if(!ajFeatGetNoteS(feature, tagnames[j], &tagval))
		ajStrAssignC(&tagval, ".");

	    if(ajStrMatchS(tagtypes[j], tmpstr))
		ajFmtPrintF(outf, " %-*S", tagsizes[j], tagval);
	    else
		ajFmtPrintF(outf, " %*S", tagsizes[j], tagval);
	}

	ajFmtPrintF(outf, "\n");
    }
    
    ajReportWriteTail(thys, ftable, seq);
    
    ajStrDel(&subseq);
    ajStrDel(&tagval);
    
    ajListIterDel(&iterft);

    AJFREE(tagtypes);
    AJFREE(tagnames);
    AJFREE(tagprints);
    AJFREE(tagsizes);

    return;
}




/* @funcstatic reportWriteRegions *********************************************
**
** Writes a report in Regions format. The report (unusually for the current
** report formats) includes the feature type.
**
** Format: <br>
**   [type] from [start] to [end] ([length] [name]) <br>
**   ([tagname]: [tagvalue], [tagname]: [tagvalue]  ...) <br>
**
** Data reported: Type, Start, End, Length, Name
**
** Tags required: None
**
** Tags used: None
**
** Tags reported: All
**
** @param [u] thys [AjPReport] Report object
** @param [r] ftable [const AjPFeattable] Feature table object
** @param [r] seq [const AjPSeq] Sequence object
** @return [void]
** @@
******************************************************************************/

static void reportWriteRegions(AjPReport thys, const AjPFeattable ftable,
			       const AjPSeq seq)
{
    AjPFile outf;
    AjIList iterft     = NULL;
    AjPFeature feature = NULL;
    ajuint istart  = 0;
    ajuint iend    = 0;
    float score   = 0.0;
    ajuint ilen    = 0;
    AjPStr subseq = NULL;
    AjPStr tagstr = NULL;
    
    ajuint ntags;
    static AjPStr* tagtypes = NULL;
    static AjPStr* tagnames = NULL;
    static AjPStr* tagprints = NULL;
    static ajuint*  tagsizes = NULL;
    ajuint j = 0;
    AjPStr tagval = NULL;

    outf = thys->File;
    
    ajReportWriteHeader(thys, ftable, seq);
    
    ntags = reportGetTags(thys, &tagtypes, &tagnames, &tagprints, &tagsizes);
    
    iterft = ajListIterNewread(ftable->Features);

    while(!ajListIterDone(iterft))
    {
	feature = (AjPFeature)ajListIterGet(iterft);
	istart = feature->Start;
	iend = feature->End;
	score = feature->Score;
	ilen = iend - istart + 1;
	ajStrAssignSubS(&subseq, ajSeqGetSeqS(seq), istart-1, iend-1);
	/* ajStrFmtUpper(&subseq); */
	ajFmtPrintF(outf, "%S from %d to %d (%d %s)\n",
		    ajFeatGetType(feature), istart, iend, ilen,
		    reportCharname(thys));
	if(thys->Showscore)
	    ajFmtPrintF(outf, "   Max score: %.*f", thys->Precision, score);

	if(ntags)
	{
	    ajFmtPrintF(outf, " (");

	    for(j=0; j < ntags; j++)
		if(ajFeatGetNoteS(feature, tagnames[j], &tagval))
		{
		    if(j)
			ajFmtPrintF(outf, ", ");

		    ajFmtPrintF(outf, "%S: %S", tagprints[j], tagval);
		}
	    
	    ajFmtPrintF(outf, ")");	    
	}

	ajFmtPrintF(outf, "\n\n");
    }
    
    ajReportWriteTail(thys, ftable, seq);
    
    ajStrDel(&subseq);
    ajStrDel(&tagstr);
    ajStrDel(&tagval);
    
    ajListIterDel(&iterft);

    AJFREE(tagtypes);
    AJFREE(tagnames);
    AJFREE(tagprints);
    AJFREE(tagsizes);

    return;
}




/* @funcstatic reportWriteSeqTable ********************************************
**
** Writes a report in SeqTable format.
**
** This is a simple table format that
** includes the feature sequence. See reportWriteTable for a version
** without the sequence. Missing tag values are reported as '.'
** The column width is 6, or longer if the name is longer.
**
** Format:<br>
**   Start   End   [tagnames]  Sequence
**   [start] [end] [tagvalues] [sequence]
**
** Data reported:
**
** Tags required: None
**
** Tags used: None
**
** Tags reported: All
**
** @param [u] thys [AjPReport] Report object
** @param [r] ftable [const AjPFeattable] Feature table object
** @param [r] seq [const AjPSeq] Sequence object
** @return [void]
** @@
******************************************************************************/

static void reportWriteSeqTable(AjPReport thys, const AjPFeattable ftable,
				const AjPSeq seq)
{
    AjPFile outf;
    AjIList iterft     = NULL;
    AjPFeature feature = NULL;
    ajuint istart  = 0;
    ajuint iend    = 0;
    AjPStr subseq = NULL;
    AjPStr seqstr = NULL;
    
    ajuint ntags;
    static AjPStr* tagtypes = NULL;
    static AjPStr* tagnames = NULL;
    static AjPStr* tagprints = NULL;
    static ajuint*  tagsizes = NULL;
    ajuint j=0;
    AjPStr tagval = NULL;
    AjPStr tmpstr = NULL;
    AjBool dostrand = ajFalse;
    char strand = '+';
/*    ajint seqlen; */
    const AjPStr seqalias;

    outf = thys->File;
/*    seqlen = ajSeqGetLen(seq); */

    if(ajSeqIsReversedTrue(seq))
    {
	ajStrAssignS(&seqstr, ajSeqGetSeqS(seq));
	ajSeqstrReverse(&seqstr);
	seqalias = seqstr;
    }
    else
	seqalias = ajSeqGetSeqS(seq);

    if(!tmpstr)
	ajStrAssignC(&tmpstr, "str");
    
    if(thys->Showstrand && ajSeqIsNuc(seq))
	dostrand = ajTrue;

    ajReportWriteHeader(thys, ftable, seq);
    
    ntags = reportGetTags(thys, &tagtypes, &tagnames, &tagprints, &tagsizes);

    /*ajDebug("dostrand: %B "
	   "ftable->Offset: %d ftable->Len: %d seqlen: %dan",
	   dostrand, ftable->Offset, ftable->Len,
	   ajSeqGetLen(seq));*/
    /*ajDebug("'%S'an", ajSeqGetSeqS(seq));*/

    iterft = ajListIterNewread(ftable->Features);
    while(!ajListIterDone(iterft))
    {
	feature = (AjPFeature)ajListIterGet(iterft);

	for(j=0; j < ntags; j++)
	    if(ajFeatGetNoteS(feature, tagnames[j], &tagval))
		tagsizes[j] = AJMAX(tagsizes[j], ajStrGetLen(tagval));
    }

    ajListIterDel(&iterft);

    ajFmtPrintF(outf, "%7s %7s", "Start", "End");

    if(dostrand)
	ajFmtPrintF(outf, " %7s", "Strand");

    for(j=0; j < ntags; j++)
    {
	if(ajStrMatchS(tagtypes[j], tmpstr))
	    ajFmtPrintF(outf, " %-*S", tagsizes[j], tagprints[j]);
	else
	    ajFmtPrintF(outf, " %*S", tagsizes[j], tagprints[j]);
    }

    ajFmtPrintF(outf, " Sequence\n");
    iterft = ajListIterNewread(ftable->Features);

    while(!ajListIterDone(iterft))
    {
	feature = (AjPFeature)ajListIterGet(iterft);
	strand = ajFeatGetStrand(feature);
	istart = feature->Start;
	iend   = feature->End;
	ajStrAssignSubS(&subseq, seqalias,
			istart-1, iend-1);
	if(strand == '-')
	    ajSeqstrReverse(&subseq);

	/*ajDebug("%d..%d start: %d end: %d ajFeatGetStrand: '%c'\n",
	       feature->Start, feature->End,
	       istart, iend, ajFeatGetStrand(feature));*/


	/* ajStrFmtUpper(&subseq); */
	
	ajDebug("reportWriteSeqTable subseq %d seq %d %d..%d\n",
		ajStrGetLen(subseq), ajSeqGetLen(seq), istart, iend);
	
	ajFmtPrintF(outf, "%7d %7d", istart, iend);

	if(dostrand)
	    ajFmtPrintF(outf, "       %c", strand);

	for(j=0; j < ntags; j++)
	{
	    if(!ajFeatGetNoteS(feature, tagnames[j], &tagval))
		ajStrAssignC(&tagval, ".");
	    ajDebug("reportWriteSeqTable tagsizes[%d] %d tagval '%S'\n",
		    j, tagsizes[j], tagval);

	    if(ajStrMatchS(tagtypes[j], tmpstr))
		ajFmtPrintF(outf, " %-*S", tagsizes[j], tagval);
	    else
		ajFmtPrintF(outf, " %*S", tagsizes[j], tagval);
	}

	ajFmtPrintF(outf, " %S\n", subseq);
    }
    
    ajReportWriteTail(thys, ftable, seq);
    
    ajStrDel(&tmpstr);
    ajStrDel(&subseq);
    ajStrDel(&seqstr);
    ajStrDel(&tagval);
    
    ajListIterDel(&iterft);

    AJFREE(tagtypes);
    AJFREE(tagnames);
    AJFREE(tagprints);
    AJFREE(tagsizes);

    return;
}




/* @funcstatic reportWriteSimple **********************************************
**
** Writes a report in SRS simple format This is a simple parsable format that
** does not include the feature sequence (see also SRS format)
** for applications where features can be large.
** Missing tag values are reported as '.'
**
** Format:<br>
**   Start   End   [tagnames]  Sequence
**   [start] [end] [tagvalues] [sequence]
**
** Data reported:
**
** Tags required: None
**
** Tags used: None
**
** Tags reported: All
**
** @param [u] thys [AjPReport] Report object
** @param [r] ftable [const AjPFeattable] Feature table object
** @param [r] seq [const AjPSeq] Sequence object
** @return [void]
** @@
******************************************************************************/

static void reportWriteSimple(AjPReport thys,
			      const AjPFeattable ftable, const AjPSeq seq)
{
    static AjBool withSeq = AJFALSE;

    reportWriteSrsFlags(thys, ftable, seq, withSeq);

    return;
}




/* @funcstatic reportWriteSrs *************************************************
**
** Writes a report in SRS format This is a simple parsable format that
** includes the feature sequence.
** Missing tag values are reported as '.'
**
** Format:<br>
**   Start   End   [tagnames]  Sequence
**   [start] [end] [tagvalues] [sequence]
**
** Data reported:
**
** Tags required: None
**
** Tags used: None
**
** Tags reported: All
**
** @param [u] thys [AjPReport] Report object
** @param [r] ftable [const AjPFeattable] Feature table object
** @param [r] seq [const AjPSeq] Sequence object
** @return [void]
** @@
******************************************************************************/

static void reportWriteSrs(AjPReport thys,
			   const AjPFeattable ftable, const AjPSeq seq)
{
    static AjBool withSeq = AJTRUE;

    reportWriteSrsFlags(thys, ftable, seq, withSeq);

    return;
}




/* @funcstatic reportWriteSrsFlags ********************************************
**
** Writes a report in SRS format.
** A flag controls whether to include the sequence.
** Missing tag values are reported as '.'
**
** Format:<br>
**   Start   End   [tagnames]  Sequence
**   [start] [end] [tagvalues] [sequence]
**
** Data reported:
**
** Tags required: None
**
** Tags used: None
**
** Tags reported: All
**
** @param [u] thys [AjPReport] Report object
** @param [r] ftable [const AjPFeattable] Feature table object
** @param [r] seq [const AjPSeq] Sequence object
** @param [r] withSeq [AjBool] If ajTrue, includes the sequence in the output.
** @return [void]
** @@
******************************************************************************/

static void reportWriteSrsFlags(AjPReport thys, const AjPFeattable ftable,
				const AjPSeq seq, AjBool withSeq)
{
    AjPFile outf;
    AjIList iterft     = NULL;
    AjPFeature feature = NULL;
    ajuint istart  = 0;
    ajuint iend    = 0;
    ajuint ilen    = 0;
    float score   = 0.0;
    AjPStr subseq = NULL;
    ajuint ift     = 0;
    
    ajuint ntags;
    static AjPStr* tagtypes = NULL;
    static AjPStr* tagnames = NULL;
    static AjPStr* tagprints = NULL;
    static ajuint*  tagsizes = NULL;
    ajuint j = 0;
    AjPStr tagval = NULL;
    
    outf = thys->File;

    ajReportWriteHeader(thys, ftable, seq);
    
    ntags  = reportGetTags(thys, &tagtypes, &tagnames, &tagprints, &tagsizes);
    iterft = ajListIterNewread(ftable->Features);

    while(!ajListIterDone(iterft))
    {
	feature = (AjPFeature)ajListIterGet(iterft);
	istart  = feature->Start;
	iend    = feature->End;
	ilen    = iend - istart + 1;
	score   = feature->Score;
	ajStrAssignSubS(&subseq, ajSeqGetSeqS(seq), istart-1, iend-1);
	/* ajStrFmtUpper(&subseq); */

	/* blank line before each feature */
	/* don't write at the end, because tail always starts with a
           blank line */

	if(ift)
	    ajFmtPrintF(outf, "\n");

	ift++;

	ajFmtPrintF(outf, "Feature: %d\n", ift);
	ajFmtPrintF(outf, "Name: %S\n", ajReportGetSeqnameSeq(thys, seq));
	ajFmtPrintF(outf, "Start: %d\n", istart);
	ajFmtPrintF(outf, "End: %d\n", iend);
	ajFmtPrintF(outf, "Length: %d\n", ilen);
	if(withSeq)
	    ajFmtPrintF(outf, "Sequence: %S\n", subseq);

	/* We always write the score - ignore thys->Showscore */

	ajFmtPrintF(outf, "Score: %.*f\n",thys->Precision,  score);

	/* We always write the strand - ignore thys->Showstrand */

	ajFmtPrintF(outf, "Strand: %c\n", ajFeatGetStrand(feature));

	if(feature->Frame)
	    ajFmtPrintF(outf, "Frame: %d\n", feature->Frame);

	for(j=0; j < ntags; j++)
	{
	    if(ajFeatGetNoteS(feature, tagnames[j], &tagval))
		ajFmtPrintF(outf, "%S: %S\n", tagprints[j], tagval);
	    else
		ajFmtPrintF(outf, "%S: .\n",
			    tagprints[j]); /* missing value '.' for now */
	}
    }
    
    ajReportWriteTail(thys, ftable, seq);
    
    ajStrDel(&subseq);
    ajStrDel(&tagval);
    
    ajListIterDel(&iterft);

    AJFREE(tagtypes);
    AJFREE(tagnames);
    AJFREE(tagprints);
    AJFREE(tagsizes);

    return;
}




/* @funcstatic reportWriteTable ***********************************************
**
** Writes a report in Table format. See reportWriteSeqTable for a version
** with the sequence. See reportWriteNameTable for a version with
** the name, which was the earlier format for reportWriteTable.
** The name already appears in the sequence header
**
** Missing tag values are reported as '.'
** The column width is 6, or longer if the name is longer.
**
** Format:<br>
**   USA    Start   End   Score   [tagnames]
**   [name] [start] [end] [score] [tagvalues]
**
** Data reported:
**
** Tags required: None
**
** Tags used: None
**
** Tags reported:
**
** @param [u] thys [AjPReport] Report object
** @param [r] ftable [const AjPFeattable] Feature table object
** @param [r] seq [const AjPSeq] Sequence object
** @return [void]
** @@
******************************************************************************/

static void reportWriteTable(AjPReport thys,
			     const AjPFeattable ftable, const AjPSeq seq)
{
    AjPFile outf;
    AjIList iterft     = NULL;
    AjPFeature feature = NULL;
    ajint istart  = 0;
    ajint iend    = 0;
    float score   = 0.0;
    AjPStr subseq = NULL;
    ajuint ntags;
    static AjPStr* tagtypes = NULL;
    static AjPStr* tagnames = NULL;
    static AjPStr* tagprints = NULL;
    static ajuint*  tagsizes = NULL;
    ajuint j = 0;
    AjPStr tagval = NULL;
    AjPStr tmpstr = NULL;
    char strand;
    AjBool dostrand = ajFalse;

    outf = thys->File;
    
    if(thys->Showstrand && ajSeqIsNuc(seq))
	dostrand = ajTrue;

    if(!tmpstr)
	ajStrAssignC(&tmpstr, "str");
    
    ajReportWriteHeader(thys, ftable, seq);
    
    ntags = reportGetTags(thys, &tagtypes, &tagnames, &tagprints, &tagsizes);
    iterft = ajListIterNewread(ftable->Features);

    while(!ajListIterDone(iterft))
    {
	feature = (AjPFeature)ajListIterGet(iterft);

	for(j=0; j < ntags; j++)
	    if(ajFeatGetNoteS(feature, tagnames[j], &tagval))
		tagsizes[j] = AJMAX(tagsizes[j], ajStrGetLen(tagval));
    }

    ajListIterDel(&iterft);

    ajFmtPrintF(outf, "%7s %7s", "Start", "End");

    if(dostrand)
	ajFmtPrintF(outf, " %7s", "Strand");

    if(thys->Showscore)
	ajFmtPrintF(outf, " %7s", "Score");
    
    for(j=0; j < ntags; j++)
    {
	if(ajStrMatchS(tagtypes[j], tmpstr))
	    ajFmtPrintF(outf, " %-*S", tagsizes[j], tagprints[j]);
	else
	    ajFmtPrintF(outf, " %*S", tagsizes[j], tagprints[j]);
    }

    ajFmtPrintF(outf, "\n");
    iterft = ajListIterNewread(ftable->Features);

    while(!ajListIterDone(iterft))
    {
	feature = (AjPFeature)ajListIterGet(iterft);
	strand = ajFeatGetStrand(feature);
	istart = feature->Start;
	iend   = feature->End;
	score = feature->Score;
	ajStrAssignSubS(&subseq, ajSeqGetSeqS(seq), istart-1, iend-1);
	/* ajStrFmtUpper(&subseq); */

	ajFmtPrintF(outf, "%7d %7d", istart, iend);

	if(dostrand)
	    ajFmtPrintF(outf, "       %c", strand);

	if(thys->Showscore)
	    ajFmtPrintF(outf, " %7.*f", thys->Precision, score);

	for(j=0; j < ntags; j++)
	{
	    if(!ajFeatGetNoteS(feature, tagnames[j], &tagval))
		ajStrAssignC(&tagval, ".");

	    if(ajStrMatchS(tagtypes[j], tmpstr))
		ajFmtPrintF(outf, " %-*S", tagsizes[j], tagval);
	    else
		ajFmtPrintF(outf, " %*S", tagsizes[j], tagval);
	}

	ajFmtPrintF(outf, "\n");
    }
    
    ajReportWriteTail(thys, ftable, seq);
    
    ajStrDel(&subseq);
    ajStrDel(&tagval);
    ajStrDel(&tmpstr);

    ajListIterDel(&iterft);

    AJFREE(tagtypes);
    AJFREE(tagnames);
    AJFREE(tagprints);
    AJFREE(tagsizes);

    return;
}




/* @funcstatic reportWriteTagseq **********************************************
**
** Writes a report in Tagseq format. Features are marked up below the sequence.
** Originally developed for the garnier application, but has general uses.
** For garnier, it checks for a '*garnier' feature tag and hard codes the
** standard garnier row names as the tagname.
**
** Format:<br>
**   Sequence (50 residues)<br>
**   tagname        ++++++++++++    +++++++++<br>
**
** If the tag value is a 1 letter code, use this instead of '+'
**
** Data reported:
**
** Tags required: None
**
** Tags used: None
**
** Tags reported:
**
** @param [u] thys [AjPReport] Report object
** @param [r] ftable [const AjPFeattable] Feature table object
** @param [r] seq [const AjPSeq] Sequence object
** @return [void]
** @@
******************************************************************************/

static void reportWriteTagseq(AjPReport thys,
			      const AjPFeattable ftable, const AjPSeq seq)
{
    AjPFile outf;
    AjIList iterft     = NULL;
    AjIList iterkey     = NULL;
    AjPFeature feature = NULL;
    ajint istart  = 0;
    ajint iend    = 0;
    ajuint ilen    = 0;
    AjPStr subseq = NULL;
    AjPStr seqstr = NULL;
    AjPStr substr = NULL;
    ajuint ntags;
    static AjPStr* tagtypes = NULL;
    static AjPStr* tagnames = NULL;
    static AjPStr* tagprints = NULL;
    static ajuint*  tagsizes = NULL;
    ajuint j = 0;
    ajuint i = 0;
    AjPStr tagval = NULL;
    AjPStr seqmarkup;
    AjPStr seqnumber = NULL;
    ajuint seqbeg;
    ajuint seqend;
    ajuint seqlen;
    ajuint ilast;
    ajuint jlast;
    ajuint iinit;
    ajuint istop;
    ajuint margin = 5;
    AjPStr featname;
    AjPTable featkeys = NULL;
    AjPList listkeys = NULL;
    AjBool isgarnier = ajFalse;
    char strand;
    char ctagval;
    const char* garnierkeys[] = {"helix", "sheet", "turns", "coil"};
    AjBool isnuc = ajFalse;
    const AjPStr seqalias;

    outf = thys->File;

    if(ajSeqIsReversedTrue(seq))
    {
	ajStrAssignS(&seqstr, ajSeqGetSeqS(seq));
	ajSeqstrReverse(&seqstr);
	seqalias = seqstr;
    }
    else
	seqalias = ajSeqGetSeqS(seq);

    seqlen = ajSeqGetLen(seq);
    seqbeg = ajSeqGetBegin(seq) + ajSeqGetOffset(seq);
    seqend = ajSeqGetEnd(seq) + ajSeqGetOffset(seq);
    
    ajReportWriteHeader(thys, ftable, seq);
    featkeys = ajTablestrNew(100);
    listkeys = ajListstrNew();

    if(ajFeattableIsNuc(ftable))
	isnuc = ajTrue;

    if(ajSeqIsNuc(seq))
	isnuc = ajTrue;

    ntags = reportGetTags(thys, &tagtypes, &tagnames, &tagprints, &tagsizes);

    for(j=0; j < ntags; j++)
	if(ajStrMatchC(tagnames[j], "garnier"))
	    isgarnier = ajTrue;

    if(isgarnier)
	for(i=0; i < 4; i++)
	{
	    featname = ajStrNewC(garnierkeys[i]);
	    seqmarkup = ajStrNewRes(seqlen+1);
	    ajStrAppendCountK(&seqmarkup, ' ', seqlen);
	    ajTablePut(featkeys, featname, seqmarkup);
	    ajListstrPushAppend(listkeys, featname);
	    margin = AJMAX(margin, ajStrGetLen(featname));
	}
    else
    {
	iterft = ajListIterNewread(ftable->Features);

	while(!ajListIterDone(iterft))
	{
	    feature = (AjPFeature)ajListIterGet(iterft);

	    if(!ajTableMatchS(featkeys, feature->Type))
	    {
		featname = ajStrNewS(feature->Type);
		seqmarkup = ajStrNewRes(seqlen+1);
		ajStrAppendCountK(&seqmarkup, ' ', seqlen);
		ajTablePut(featkeys, featname,seqmarkup);
		ajListstrPushAppend(listkeys, featname);

		if(isnuc)
		    margin = AJMAX(margin,
				   ajStrGetLen(ajFeatTypeNuc(featname)));
		else
		    margin = AJMAX(margin,
				   ajStrGetLen(ajFeatTypeProt(featname)));
	    }
	}

	ajListIterDel(&iterft);
    }

    ajFmtPrintAppS(&seqnumber, "    .%5d", 10);

    if(seqend > 9)
	istop = seqend-9;
    else
	istop = 0;

    for(i=10; i < istop; i+=10)
	ajFmtPrintAppS(&seqnumber, "    .%5d", i+10);

    iterft = ajListIterNewread(ftable->Features);

    while(!ajListIterDone(iterft))
    {
	feature = (AjPFeature)ajListIterGet(iterft);
	strand = ajFeatGetStrand(feature);
	istart = feature->Start;
	iend = feature->End;
	ilen = iend - istart + 1;

	if(isgarnier && ajFeatGetNoteC(feature, "garnier", &tagval))
	{
	    ctagval = ajStrGetCharFirst(tagval);

	    switch(ctagval)
	    {
                case 'H':
                    seqmarkup = ajTableFetchmodC(featkeys, garnierkeys[0]); 
                    break;
                case 'E':
                    seqmarkup = ajTableFetchmodC(featkeys, garnierkeys[1]); 
                    break;
                case 'T':
                    seqmarkup = ajTableFetchmodC(featkeys, garnierkeys[2]); 
                    break;
                default:
                    seqmarkup = ajTableFetchmodC(featkeys, garnierkeys[3]); 
                    break;
	    }

	    ajStrPasteCountK(&seqmarkup, istart-1,
			     ctagval, ilen);
	}
	else
	{
	    seqmarkup = ajTableFetchmodS(featkeys, feature->Type); 
	    ajStrPasteCountK(&seqmarkup, istart-1, strand, ilen);
	}
    }

    iinit = seqbeg-1;

    for(i=iinit; i < seqend; i+=50)
    {
	ilast = AJMIN(i+50-1, seqend-1);
	jlast = AJMIN(i+50-1, ajStrGetLen(seqnumber)-1);

	ajStrAssignSubS(&substr, seqnumber, i, jlast);
	ajFmtPrintF(outf, "%*s %S\n", margin, "", substr);

	ajStrAssignSubS(&subseq, seqalias, i, ilast);
	/* ajStrFmtUpper(&subseq); */

	ajFmtPrintF(outf, "%*s %S\n", margin, "", subseq);
	
	iterkey = ajListIterNewread(listkeys);

	while(!ajListIterDone(iterkey))
	{
	    featname = ajListstrIterGet(iterkey);
	    seqmarkup = ajTableFetchmodS(featkeys, featname);
	    ajStrAssignSubS(&substr, seqmarkup, i, ilast);

	    if(isgarnier)
		ajFmtPrintF(outf, "%*S %S\n",
			    margin, featname, substr);
	    else if (isnuc)
		ajFmtPrintF(outf, "%*S %S\n",
			    margin, ajFeatTypeNuc(featname), substr);
	    else
		ajFmtPrintF(outf, "%*S %S\n",
			    margin, ajFeatTypeProt(featname), substr);
	}

	ajListIterDel(&iterkey);
    }
    
    ajReportWriteTail(thys, ftable, seq);
    
    ajStrDel(&seqstr);
    ajStrDel(&subseq);
    ajStrDel(&tagval);
    
    ajStrDel(&substr);
    ajStrDel(&seqnumber);
    ajListIterDel(&iterft);

    AJFREE(tagtypes);
    AJFREE(tagnames);
    AJFREE(tagprints);
    AJFREE(tagsizes);

    ajTablestrFree(&featkeys);
    ajListFree(&listkeys);

    return;
}




/* @func ajReportDel **********************************************************
**
** Destructor for report objects
**
** @param [d] pthys [AjPReport*] Report object reference
** @return [void]
** @category delete [AjPReport] Default destructor
** @@
******************************************************************************/

void ajReportDel(AjPReport* pthys)
{
    AjPReport thys;
    AjPStr str = NULL;

    thys = *pthys;

    if(!thys)
	return;

    ajStrDel(&thys->Name);
    ajStrDel(&thys->Type);
    ajStrDel(&thys->Formatstr);
    ajStrDel(&thys->Extension);

    while(ajListPop(thys->FileTypes,(void **)&str))
	ajStrDel(&str);

    ajListFree(&thys->FileTypes);

    while(ajListPop(thys->FileNames,(void **)&str))
	ajStrDel(&str);

    ajListFree(&thys->FileNames);

    while(ajListPop(thys->Tagnames,(void **)&str))
	ajStrDel(&str);

    ajListFree(&thys->Tagnames);

    while(ajListPop(thys->Tagprints,(void **)&str))
	ajStrDel(&str);

    ajListFree(&thys->Tagprints);

    while(ajListPop(thys->Tagtypes,(void **)&str))
	ajStrDel(&str);

    ajListFree(&thys->Tagtypes);

    ajStrDel(&thys->Header);
    ajStrDel(&thys->SubHeader);
    ajStrDel(&thys->Tail);
    ajStrDel(&thys->SubTail);

    ajFeattableDel(&thys->Fttable);
    ajFeattabOutDel(&thys->Ftquery);

    ajFileClose(&thys->File);

    AJFREE(*pthys);

    return;
}




/* @func ajReportOpen *********************************************************
**
** Opens a new report file
**
** @param [u] thys [AjPReport] Report object
** @param [r] name [const AjPStr] File name
** @return [AjBool] ajTrue on success
** @@
******************************************************************************/

AjBool ajReportOpen(AjPReport thys, const AjPStr name)
{
    if(!ajReportValid(thys))
	return ajFalse;

    thys->File = ajFileNewOutNameS(name);

    if(thys->File)
	return ajTrue;

    return ajFalse;
}




/* @func ajReportFormatDefault ************************************************
**
** Sets the default format for a feature report
**
** @param [w] pformat [AjPStr*] Default format returned
** @return [AjBool] ajTrue is format was returned
** @@
******************************************************************************/

AjBool ajReportFormatDefault(AjPStr* pformat)
{
    if(ajStrGetLen(*pformat))
	ajDebug("... output format '%S'\n", *pformat);
    else
    {
	/* ajStrAssignEmptyC(pformat, reportFormat[0].Name);*/
	ajStrAssignEmptyC(pformat, "gff");	/* use the real name */
	ajDebug("... output format not set, default to '%S'\n", *pformat);
    }

    return ajTrue;
}




/* @func ajReportFindFormat ***************************************************
**
** Looks for the specified report format in the internal definitions and
** returns the index.
**
** @param [r] format [const AjPStr] Format required.
** @param [w] iformat [ajint*] Index
** @return [AjBool] ajTrue on success.
** @@
******************************************************************************/

AjBool ajReportFindFormat(const AjPStr format, ajint* iformat)
{
    AjPStr tmpformat = NULL;
    ajint i = 0;

    if(!ajStrGetLen(format))
	return ajFalse;

    ajStrAssignS(&tmpformat, format);
    ajStrFmtLower(&tmpformat);

    while(reportFormat[i].Name)
    {
	if(ajStrMatchCaseC(tmpformat, reportFormat[i].Name))
	{
	    *iformat = i;
	    ajStrDel(&tmpformat);

	    return ajTrue;
	}

	i++;
    }

    ajStrDel(&tmpformat);

    return ajFalse;
}




/* @func ajReportSetTagsS ******************************************************
**
** Sets the tag list for a report
**
** @param [u] thys [AjPReport] Report object
** @param [r] taglist [const AjPStr] Tag names list
** @return [AjBool] ajTrue on success
** @@
******************************************************************************/

AjBool ajReportSetTagsS(AjPReport thys, const AjPStr taglist)
{
    AjPStr tmplist   = NULL;
    AjPStr tmpstr    = NULL;
    AjPStr tagtype  = NULL;
    AjPStr tagname  = NULL;
    AjPStr tagprint = NULL;

    /*
     ** assume the tags are a simple list in this format:
     ** type: name[=printname]
     **
     ** spaces are not allowed in names (for ease of parsing the results)
     */

    if(!reportTagExp)
	reportTagExp = ajRegCompC("^ *([^:]+):([^= ]+)(=([^ ]+))?");

    ajStrAssignS(&tmplist, taglist);

    while(ajRegExec(reportTagExp, tmplist))
    {
	tagtype = NULL;
	tagname = NULL;
	tagprint = NULL;
	ajRegSubI(reportTagExp, 1, &tagtype);
	ajRegSubI(reportTagExp, 2, &tagname);
	ajRegSubI(reportTagExp, 4, &tagprint);

	if(!ajStrGetLen(tagprint))
	    ajStrAssignS(&tagprint, tagname);

	ajDebug("Tag '%S' : '%S' print '%S'\n", tagtype, tagname, tagprint);
	ajRegPost(reportTagExp, &tmpstr);
	ajStrAssignS(&tmplist, tmpstr);

	if(!ajListGetLength(thys->Tagtypes))
	{
	    thys->Tagtypes  = ajListNew();
	    thys->Tagnames  = ajListNew();
	    thys->Tagprints = ajListNew();
	}

	ajListPushAppend(thys->Tagtypes,  tagtype);
	ajListPushAppend(thys->Tagnames,  tagname);
	ajListPushAppend(thys->Tagprints, tagprint);
    }

    if(ajStrGetLen(tmplist))
    {				      /* test acdc-reportbadtaglist */
	ajErr("Bad report taglist at '%S'", tmplist);

	return ajFalse;
    }

    ajStrDel(&tmpstr);
    ajStrDel(&tmplist);

    return ajTrue;
}




/* @obsolete ajReportSetTags
** @rename ajReportSetTagsS
*/

__deprecated AjBool ajReportSetTags(AjPReport thys, const AjPStr taglist)
{

    return ajReportSetTagsS(thys, taglist);
}




/* @func ajReportValid ********************************************************
**
** Test for a report object.
**
** Checks the format works with the number of tags.
** Checks the format works with the type (protein or nucleotide).
** Sets the format if not already defined.
**
** @param [u] thys [AjPReport] Report object
** @return [AjBool] ajTrue on success
** @@
******************************************************************************/

AjBool ajReportValid(AjPReport thys)
{
    if(!thys->Format)
	/* test acdc-reportbadformat */
	if(!ajReportFindFormat(thys->Formatstr, &thys->Format))
	{
	    ajErr("Unknown report format '%S'", thys->Formatstr);

	    return ajFalse;
	}

    /* so far, only diffseq format has mintags non-zero */
    /* it needs 7 special tags to compare features */

    if( reportFormat[thys->Format].Mintags > ajListGetLength(thys->Tagnames))
    {
	ajErr("Report format '%s' needs %d tags, has only %d",
	      reportFormat[thys->Format].Name,
	      reportFormat[thys->Format].Mintags,
	      ajListGetLength(thys->Tagnames));

	return ajFalse;
    }

    return ajTrue;
}




/* @func ajReportNew **********************************************************
**
** Constructor for a report object
**
** @return [AjPReport] New report object
** @category new [AjPReport] Default constructor
** @@
******************************************************************************/

AjPReport ajReportNew(void)
{
    AjPReport pthis;

    AJNEW0(pthis);

    pthis->CountSeq  = 0;
    pthis->CountHit  = 0;
    pthis->TotHits   = 0;
    pthis->Name      = ajStrNew();
    pthis->Formatstr = ajStrNew();
    pthis->Format    = 0;
    pthis->Fttable   = NULL;
    pthis->Ftquery   = ajFeattabOutNew();
    pthis->Extension = ajStrNew();
    pthis->Precision = 3;
    pthis->File      = NULL;
    pthis->Showscore = ajTrue;

    return pthis;
}




/* @func ajReportWrite ********************************************************
**
** Writes a feature report
**
** @param [u] thys [AjPReport] Report object
** @param [r] ftable [const AjPFeattable] Feature table object
** @param [r] seq [const AjPSeq] Sequence object
** @return [AjBool] True if data was written
**                  False if maximum output has already been reached.
** @category output [AjPReport] Master sequence output routine
** @@
******************************************************************************/

AjBool ajReportWrite(AjPReport thys,
		     const AjPFeattable ftable, const AjPSeq seq)
{
    ajint maxreport = 0;
    AjPFeattable ftcopy = NULL;
    AjPStr tmpstr = NULL;
    AjBool isnuc = ajFalse;
    AjBool isprot = ajFalse;
    AjBool ok;

    ajDebug("ajReportWrite\n");		/* add ftable name and size */

    if(!thys->Format)
	if(!ajReportFindFormat(thys->Formatstr, &thys->Format))
	    ajDie("unknown report format '%S'", thys->Formatstr);

    if(ajSeqIsNuc(seq) && ajFeattableIsNuc(ftable))
	isnuc = ajTrue;

    if(ajSeqIsProt(seq) && ajFeattableIsProt(ftable))
	isprot = ajTrue;

    ok = ajFalse;

    if(isnuc && reportFormat[thys->Format].Nucleotide)
	ok = ajTrue;
    else if(isprot && reportFormat[thys->Format].Protein)
	ok = ajTrue;

    if(!ok)
    {
	if(isnuc)
	    ajErr("Report format '%S' not supported for nucleotide sequences",
		  thys->Formatstr);
	else if(isprot)
	    ajErr("Report format '%S' not supported for protein sequences",
		  thys->Formatstr);
	else
	    ajErr("Report format '%S' failed: unknown sequence type",
		  thys->Formatstr);

	return ajFalse;
    }

    ajDebug("ajReportWrite %d '%s' %d\n",
	    thys->Format, reportFormat[thys->Format].Name,
	    ajFeattableGetSize(ftable));

    if(thys->MaxHitAll)
    {
	maxreport = thys->MaxHitAll - thys->CountHit;

	if(maxreport <= 0)
	{
            thys->MaxLimit = ajTrue;

	    return ajFalse;
	}
    }

    if(thys->MaxHitSeq)
    {
	if(thys->MaxHitAll)
	    maxreport = AJMIN(maxreport, thys->MaxHitSeq);
	else
	    maxreport = thys->MaxHitSeq;
    }

    ajDebug("ajReportWrite MaxHitSeq: %d MaxHitAll: %d "
	    "CountHit: %d FeattableGetSize: %u maxreport: %d\n",
	    thys->MaxHitSeq, thys->MaxHitAll, thys->CountHit,
	    ajFeattableGetSize(ftable), maxreport);

    ajReportSetType(thys, ftable, seq);

    /* Calling funclist reportFormat() */

    if(maxreport && maxreport < (ajint)ajFeattableGetSize(ftable))
    {
	ajFmtPrintS(&tmpstr, "HitLimit: %d/%u",
		    maxreport, ajFeattableGetSize(ftable));
	ajReportAppendSubtailS(thys, tmpstr);
	ajDebug("new subtail '%S'\n", tmpstr);
	ajStrDel(&tmpstr);
	ftcopy = ajFeattableNewFtableLimit(ftable, maxreport);
	reportFormat[thys->Format].Write(thys, ftcopy, seq);
	ajFeattableDel(&ftcopy);
	thys->CountHit += maxreport;
	thys->TotHits += ajFeattableGetSize(ftable);
    }
    else
    {
	reportFormat[thys->Format].Write(thys, ftable, seq);
	thys->CountHit += ajFeattableGetSize(ftable);
	thys->TotHits += ajFeattableGetSize(ftable);
    }

    ++thys->CountSeq;

    return ajTrue;
}




/* @func ajReportClose ********************************************************
**
** Closes a feature report
**
** @param [u] thys [AjPReport] Report object
** @return [void]
** @@
******************************************************************************/

void ajReportClose(AjPReport thys)
{
    if(!thys)
	return;

    ajDebug("ajReportClose '%F' CountSeq %d CountHit %d\n",
	    thys->File, thys->CountSeq, thys->CountHit);

    if(!thys->File)	     /* already closed, nothing to write to */
	return;

    if(!thys->Format)
	if(!ajReportFindFormat(thys->Formatstr, &thys->Format))
	    ajDie("unknown report format '%S'", thys->Formatstr);

    if(reportFormat[thys->Format].Showheader) {
	if(!thys->CountSeq)
	    ajReportWriteHeader(thys, NULL, NULL);
	ajReportWriteTail(thys, NULL, NULL);
    }

    ajFeattabOutClear(&thys->Ftquery);
    ajFileClose(&thys->File);

    return;
}




/* @funcstatic reportGetTags ***************************************************
**
** Converts a report tagtypes definition (ACD taglist attribute)
** into arrays of tag types, names and printnames.
**
** @param [r] thys [const AjPReport] Report object
** @param [w] types [AjPStr**] Address of array of types generated
** @param [w] names [AjPStr**] Address of array of names generated
** @param [w] prints [AjPStr**] Address of array of print names generated
** @param [w] sizes [ajuint**] Width needed to print heading
** @return [ajint] Number of tagtypes (size of arrays created)
******************************************************************************/

static ajint reportGetTags(const AjPReport thys, AjPStr** types, AjPStr** names,
                           AjPStr** prints, ajuint** sizes)
{
    ajint ntags;
    static ajuint jmin = 6;
    ajint i;

    if(!ajListGetLength(thys->Tagtypes))
	return 0;

    ntags = ajListToarray(thys->Tagnames, (void***) names);
    ntags = ajListToarray(thys->Tagprints, (void***) prints);
    ntags = ajListToarray(thys->Tagtypes,  (void***) types);

    if(ntags)
    {
	AJCRESIZE(*sizes, ntags);

	for(i=0; i < ntags; i++)
	    (*sizes)[i] = AJMAX(jmin, ajStrGetLen((*prints)[i]));
    }
    else
	AJFREE(sizes);

    return ntags;
}




/* @obsolete ajReportLists
** @remove Made static
*/

__deprecated ajint ajReportLists(const AjPReport thys,
                                 AjPStr** types, AjPStr** names,
                                 AjPStr** prints, ajuint** sizes)
{

    return reportGetTags(thys, types, names, prints, sizes);
}




/* @func ajReportWriteHeader **************************************************
**
** Writes a feature report header and updates internal counters.
**
** @param [u] thys [AjPReport] Report object
** @param [r] ftable [const AjPFeattable] Feature table object
** @param [r] seq [const AjPSeq] Sequence object
** @return [void]
** @@
******************************************************************************/

void ajReportWriteHeader(AjPReport thys,
			 const AjPFeattable ftable, const AjPSeq seq)
{
    AjPFile outf;
    AjPStr tmpstr    = NULL;
    AjPStr tmpname   = NULL;
    AjPStr tmptype   = NULL;
    AjIList itername = NULL;
    AjIList itertype = NULL;
    AjBool doSingle  =ajFalse; /* turned off for now - always multi format */
    int i;
    AjPTime today = NULL;
    
    outf = thys->File;

    today =  ajTimeNewTodayFmt("report");
    
    /* Header for the top of the file (first call for report only) */
    
    if(!thys->CountSeq)
    {
	ajFmtPrintF(outf, "########################################\n");
	ajFmtPrintF(outf, "# Program: %S\n", ajUtilGetProgram());
	ajFmtPrintF(outf, "# Rundate: %D\n", today);
	ajFmtPrintF(outf, "# Commandline: %S\n", ajUtilGetProgram());
	ajStrAssignS(&tmpstr, ajUtilGetCmdline());

	if(ajStrGetLen(tmpstr))
	{
	    ajStrExchangeCC(&tmpstr, "\n", "\1#    ");
	    ajStrExchangeCC(&tmpstr, "\1", "\n");
	    ajFmtPrintF(outf, "#    %S\n", tmpstr);
	}

	ajStrAssignS(&tmpstr, ajUtilGetInputs());

	if(ajStrGetLen(tmpstr))
	{
	    ajStrExchangeCC(&tmpstr, "\n", "\1#    ");
	    ajStrExchangeCC(&tmpstr, "\1", "\n");
	    ajFmtPrintF(outf, "#    %S\n", tmpstr);
	}

	if(thys->MaxHitAll)
	    ajFmtPrintF(outf, "# Max_hits_total: %d\n",
			thys->MaxHitAll);
	if(thys->MaxHitSeq)
	    ajFmtPrintF(outf, "# Max_hits_sequence: %d\n",
			thys->MaxHitSeq);
	ajFmtPrintF(outf, "# Report_format: %S\n", thys->Formatstr);
	ajFmtPrintF(outf, "# Report_file: %F\n", outf);

	if(ajListGetLength(thys->FileNames))
	{
	    i = 0;
	    itername = ajListIterNewread(thys->FileNames);
	    itertype = ajListIterNewread(thys->FileTypes);
	    ajFmtPrintF(outf, "# Additional_files: %d\n",
			ajListGetLength(thys->FileNames));
	    while(!ajListIterDone(itername) && !ajListIterDone(itertype))
	    {
		tmpname = (AjPStr)ajListIterGet(itername);
		tmptype = (AjPStr)ajListIterGet(itertype);
		ajFmtPrintF(outf, "# %d: %S (%S)\n", ++i, tmpname, tmptype);
	    }

	    ajListIterDel(&itername);
	    ajListIterDel(&itertype);
	}

	if(!doSingle || thys->Multi)
	    ajFmtPrintF(outf, "########################################\n\n");
	else
	    ajFmtPrintF(outf, "#\n");
    }

    /* Sequence header (can be part of top header) */
    
    if(!doSingle || thys->Multi)
	ajFmtPrintF(outf, "#=======================================\n#\n");
 
    if(seq)
    {
	if(ajSeqIsReversedTrue(seq))
	    ajFmtPrintF(outf, "# Sequence: %S     from: %d   to: %d   "
			"(Reversed)\n",
			ajReportGetSeqnameSeq(thys, seq),
			ajSeqGetBegin(seq) + ajSeqGetOffset(seq),
			ajSeqGetEnd(seq) + ajSeqGetOffset(seq));
	else
	    ajFmtPrintF(outf, "# Sequence: %S     from: %d   to: %d\n",
			ajReportGetSeqnameSeq(thys, seq),
			ajSeqGetBegin(seq) + ajSeqGetOffset(seq),
			ajSeqGetEnd(seq) + ajSeqGetOffset(seq));

	if(thys->Showacc)
	    ajFmtPrintF(outf, "# Accession: %S\n", ajSeqGetAccS(seq));

	if(thys->Showdes)
	    ajFmtPrintF(outf, "# Description: %S\n", ajSeqGetDescS(seq));
    }

    if(ftable)
    {
	ajFmtPrintF(outf, "# HitCount: %d\n",
		    ajFeattableGetSize(ftable));
    }

    if(ajStrGetLen(thys->Header))
    {
	ajStrAssignS(&tmpstr, thys->Header);
	ajStrExchangeCC(&tmpstr, "\n", "\1# ");
	ajStrExchangeCC(&tmpstr, "\1", "\n");
	ajStrTrimEndC(&tmpstr, " ");
	ajFmtPrintF(outf, "#\n");
	ajFmtPrintF(outf, "# %S", tmpstr);

	if(!ajStrSuffixC(tmpstr, "\n#"))
	    ajFmtPrintF(outf, "\n#");

	ajFmtPrintF(outf, "\n");
    }
    
    if(ajStrGetLen(thys->SubHeader))
    {
	ajStrAssignS(&tmpstr, thys->SubHeader);
	ajStrExchangeCC(&tmpstr, "\n", "\1# ");
	ajStrExchangeCC(&tmpstr, "\1", "\n");
	ajStrTrimEndC(&tmpstr, " ");
	ajFmtPrintF(outf, "#\n");
	ajFmtPrintF(outf, "# %S", tmpstr);

	if(!ajStrSuffixC(tmpstr, "\n#"))
	    ajFmtPrintF(outf, "\n#");

	ajFmtPrintF(outf, "\n");
	ajStrDel(&thys->SubHeader);
    }
    
    if(!doSingle || thys->Multi)
	ajFmtPrintF(outf, "#=======================================\n\n");
    else
	ajFmtPrintF(outf, "########################################\n\n");
    
    ajStrDel(&tmpstr);
    AJFREE(today);
    
    return;
}




/* @func ajReportWriteTail ****************************************************
**
** Writes (and clears) a feature report tail
**
** @param [u] thys [AjPReport] Report object
** @param [r] ftable [const AjPFeattable] Feature table object
** @param [r] seq [const AjPSeq] Sequence object
** @return [void]
** @@
******************************************************************************/

void ajReportWriteTail(AjPReport thys,
		       const AjPFeattable ftable, const AjPSeq seq)
{
    AjPFile outf;
    AjPStr tmpstr   = NULL;
    AjBool doSingle = ajFalse; /* turned off for now - always multi format */

    (void) seq;		     /* make it used - we may need it later */

    outf = thys->File;

    if(!doSingle || thys->Multi)
	ajFmtPrintF(outf, "\n#---------------------------------------\n");
    else
	ajFmtPrintF(outf, "\n########################################\n");

    if(ftable)
    {
	if(ajStrGetLen(thys->SubTail))
	{
	    ajStrAssignS(&tmpstr, thys->SubTail);
	    ajStrExchangeCC(&tmpstr, "\n", "\1# ");
	    ajStrExchangeCC(&tmpstr, "\1", "\n");
	    ajStrTrimEndC(&tmpstr, " ");
	    ajFmtPrintF(outf, "#\n");
	    ajFmtPrintF(outf, "# %S", tmpstr);

	    if(!ajStrSuffixC(tmpstr, "\n#"))
		ajFmtPrintF(outf, "\n#");

	    ajFmtPrintF(outf, "\n");
	    ajStrDel(&thys->SubTail);
	}

	if(ajStrGetLen(thys->Tail))
	{
	    ajStrAssignS(&tmpstr, thys->Tail);
	    ajStrExchangeCC(&tmpstr, "\n", "\1# ");
	    ajStrExchangeCC(&tmpstr, "\1", "\n");
	    ajStrTrimEndC(&tmpstr, " ");
	    ajFmtPrintF(outf, "#\n");
	    ajFmtPrintF(outf, "# %S", tmpstr);

	    if(!ajStrSuffixC(tmpstr, "\n#"))
		ajFmtPrintF(outf, "\n#");

	    ajFmtPrintF(outf, "\n");
	}
    }

    if(!ftable)
    {
	if(thys->Totseqs)
            ajFmtPrintF(outf, "# Total_sequences: %Ld\n", thys->Totseqs);

	if(thys->Totlength)
            ajFmtPrintF(outf, "# Total_length: %Ld\n", thys->Totlength);
	ajFmtPrintF(outf, "# Reported_sequences: %d\n", thys->CountSeq);
	ajFmtPrintF(outf, "# Reported_hitcount: %d\n", thys->CountHit);

        if(thys->CountHit < thys->TotHits)
            ajFmtPrintF(outf, "# Unreported_hitcount: %d\n",
                        thys->TotHits - thys->CountHit);
	if(thys->MaxHitAll)
            ajFmtPrintF(outf, "# Max_hitcount: %d\n", thys->MaxHitAll);

	if(thys->MaxLimit)
            ajFmtPrintF(outf, "# Maxhits_stop: %B\n", thys->MaxLimit);
    }

    if(!doSingle || thys->Multi)
	ajFmtPrintF(outf, "#---------------------------------------\n");
    else
	ajFmtPrintF(outf, "########################################\n");

    ajStrDel(&tmpstr);

    return;
}




/* @func ajReportSetHeaderS ****************************************************
**
** Defines a feature report header
**
** @param [u] thys [AjPReport] Report object
** @param [r] header [const AjPStr] Report header with embedded newlines
** @return [void]
** @@
******************************************************************************/

void ajReportSetHeaderS(AjPReport thys, const AjPStr header)
{
    if(!thys)
        return;

    ajStrAssignS(&thys->Header, header);

    return;
}





/* @obsolete ajReportSetHeader
** @rename ajReportSetHeaderS
*/

__deprecated void ajReportSetHeader(AjPReport thys, const AjPStr header)
{
    ajReportSetHeaderS(thys, header);
    return;
}




/* @func ajReportSetHeaderC ***************************************************
**
** Defines a feature report header
**
** @param [u] thys [AjPReport] Report object
** @param [r] header [const char*] Report header with embedded newlines
** @return [void]
** @@
******************************************************************************/

void ajReportSetHeaderC(AjPReport thys, const char* header)
{
    if(!thys)
        return;

    ajStrAssignC(&thys->Header, header);

    return;
}




/* @func ajReportAppendHeaderC ************************************************
**
** Appends to a feature report header
**
** @param [u] thys [AjPReport] Report object
** @param [r] header [const char*] Report header with embedded newlines
** @return [void]
** @@
******************************************************************************/

void ajReportAppendHeaderC(AjPReport thys, const char* header)
{
    if(!thys)
        return;

    if(ajStrGetLen(thys->Header))
	if(ajStrGetCharLast(thys->Header) != '\n')
	    ajStrAppendK(&thys->Header, '\n');

    ajStrAppendC(&thys->Header, header);

    return;
}




/* @func ajReportAppendHeaderS *************************************************
**
** Defines a feature report header
**
** @param [u] thys [AjPReport] Report object
** @param [r] header [const AjPStr] Report header with embedded newlines
** @return [void]
** @@
******************************************************************************/

void ajReportAppendHeaderS(AjPReport thys, const AjPStr header)
{
    if(!thys) return;

    if(ajStrGetLen(thys->Header))
	if(ajStrGetCharLast(thys->Header) != '\n')
	    ajStrAppendK(&thys->Header, '\n');

    ajStrAppendS(&thys->Header, header);

    return;
}




/* @obsolete ajReportAppendHeader
** @rename ajReportAppendHeaderS
*/

__deprecated void ajReportAppendHeader(AjPReport thys, const AjPStr header)
{
    ajReportAppendHeaderS(thys, header);
    return;
}




/* @func ajReportSetSubheaderC ************************************************
**
** Appends to a feature report subheader
**
** @param [u] thys [AjPReport] Report object
** @param [r] header [const char*] Report header with embedded newlines
** @return [void]
** @@
******************************************************************************/

void ajReportSetSubheaderC(AjPReport thys, const char* header)
{
    if(!thys)
        return;

    ajStrAssignC(&thys->SubHeader, header);

    return;
}




/* @func ajReportSetSubheaderS *************************************************
**
** Defines a feature report subheader
**
** @param [u] thys [AjPReport] Report object
** @param [r] header [const AjPStr] Report header with embedded newlines
** @return [void]
** @@
******************************************************************************/

void ajReportSetSubheaderS(AjPReport thys, const AjPStr header)
{
    if(!thys)
        return;

    ajStrAssignS(&thys->SubHeader, header);

    return;
}





/* @obsolete ajReportSetSubHeader
** @rename ajReportSetSubheaderS
*/

__deprecated void ajReportSetSubHeader(AjPReport thys, const AjPStr header)
{
    ajReportSetSubheaderS(thys,header);
}




/* @func ajReportAppendSubheaderC **********************************************
**
** Appends to a feature report subheader
**
** @param [u] thys [AjPReport] Report object
** @param [r] header [const char*] Report header with embedded newlines
** @return [void]
** @@
******************************************************************************/

void ajReportAppendSubheaderC(AjPReport thys, const char* header)
{
    if(!thys)
        return;

    if(ajStrGetLen(thys->SubHeader))
	if(ajStrGetCharLast(thys->SubHeader) != '\n')
	    ajStrAppendK(&thys->SubHeader, '\n');

    ajStrAppendC(&thys->SubHeader, header);

    return;
}





/* @obsolete ajReportAppendSubHeaderC
** @rename ajReportAppendSubheaderC
*/

__deprecated void ajReportAppendSubHeaderC(AjPReport thys, const char* header)
{
    ajReportAppendSubheaderC(thys, header);
    return;
}




/* @func ajReportAppendSubheaderS **********************************************
**
** Defines a feature report subheader
**
** @param [u] thys [AjPReport] Report object
** @param [r] header [const AjPStr] Report header with embedded newlines
** @return [void]
** @@
******************************************************************************/

void ajReportAppendSubheaderS(AjPReport thys, const AjPStr header)
{
    if(!thys)
        return;

    if(ajStrGetLen(thys->SubHeader))
	if(ajStrGetCharLast(thys->SubHeader) != '\n')
	    ajStrAppendK(&thys->SubHeader, '\n');

    ajStrAppendS(&thys->SubHeader, header);

    return;
}




/* @obsolete ajReportAppendSubHeader
** @rename ajReportAppendSubheaderS
*/

__deprecated void ajReportAppendSubHeader(AjPReport thys, const AjPStr header)
{
    ajReportAppendSubheaderS(thys, header);
    return;
}




/* @func ajReportSetSeqstats ***************************************************
**
** Defines a feature report tail
**
** @param [u] thys [AjPReport] Report object
** @param [r] seqall [const AjPSeqall] Sequence stream object
** @return [void]
** @@
******************************************************************************/

void ajReportSetSeqstats(AjPReport thys, const AjPSeqall seqall)
{
    if(!thys)
        return;

    thys->Totseqs = ajSeqallGetCount(seqall);
    thys->Totlength = ajSeqallGetTotlength(seqall);

    return;
}




/* @func ajReportSetSeqsetstats ************************************************
**
** Defines a feature report tail
**
** @param [u] thys [AjPReport] Report object
** @param [r] seqset [const AjPSeqset] Sequence set object
** @return [void]
** @@
******************************************************************************/

void ajReportSetSeqsetstats(AjPReport thys, const AjPSeqset seqset)
{
    if(!thys)
        return;

    thys->Totseqs = ajSeqsetGetSize(seqset);
    thys->Totlength = ajSeqsetGetTotlength(seqset);

    return;
}




/* @func ajReportSetStatistics ************************************************
**
** Defines a feature report tail
**
** @param [u] thys [AjPReport] Report object
** @param [r] totseqs [ajlong] Total number of sequences processed
** @param [r] totlength [ajlong] Total sequence length processed
** @return [void]
** @@
******************************************************************************/

void ajReportSetStatistics(AjPReport thys, ajlong totseqs, ajlong totlength)
{
    if(!thys)
        return;

    thys->Totseqs = totseqs;
    thys->Totlength = totlength;

    return;
}




/* @func ajReportSetTailC *****************************************************
**
** Defines a feature report tail
**
** @param [u] thys [AjPReport] Report object
** @param [r] tail [const char*] Report tail with embedded newlines
** @return [void]
** @@
******************************************************************************/

void ajReportSetTailC(AjPReport thys, const char* tail)
{
    if(!thys)
        return;

    ajStrAssignC(&thys->Tail, tail);

    return;
}




/* @func ajReportSetTailS ******************************************************
**
** Defines a feature report tail
**
** @param [u] thys [AjPReport] Report object
** @param [r] tail [const AjPStr] Report tail with embedded newlines
** @return [void]
** @@
******************************************************************************/

void ajReportSetTailS(AjPReport thys, const AjPStr tail)
{
    if(!thys)
        return;

    ajStrAssignS(&thys->Tail, tail);

    return;
}





/* @obsolete ajReportSetTail
** @rename ajReportSetTailS
*/

__deprecated void ajReportSetTail(AjPReport thys, const AjPStr tail)
{
    ajReportSetTailS(thys, tail);
    return;
}




/* @func ajReportAppendTailC **************************************************
**
** Appends to a feature report tail
**
** @param [u] thys [AjPReport] Report object
** @param [r] tail [const char*] Report tail with embedded newlines
** @return [void]
** @@
******************************************************************************/

void ajReportAppendTailC(AjPReport thys, const char* tail)
{
    if(!thys)
        return;

    if(ajStrGetLen(thys->Tail))
	if(ajStrGetCharLast(thys->Tail) != '\n')
	    ajStrAppendK(&thys->Tail, '\n');

    ajStrAppendC(&thys->Tail, tail);

    return;
}




/* @func ajReportAppendTailS ***************************************************
**
** Appends to a feature report tail
**
** @param [u] thys [AjPReport] Report object
** @param [r] tail [const AjPStr] Report tail with embedded newlines
** @return [void]
** @@
******************************************************************************/

void ajReportAppendTailS(AjPReport thys, const AjPStr tail)
{
    if(!thys)
        return;

    if(ajStrGetLen(thys->Tail))
	if(ajStrGetCharLast(thys->Tail) != '\n')
	    ajStrAppendK(&thys->Tail, '\n');

    ajStrAppendS(&thys->Tail, tail);

    return;
}




/* @obsolete ajReportAppendTail
** @rename ajReportAppendTailS
*/

__deprecated void ajReportAppendTail(AjPReport thys, const AjPStr tail)
{
    ajReportAppendTailS(thys, tail);
    return;
}




/* @func ajReportSetSubtailC **************************************************
**
** Defines a feature report subtail
**
** @param [u] thys [AjPReport] Report object
** @param [r] tail [const char*] Report tail with embedded newlines
** @return [void]
** @@
******************************************************************************/

void ajReportSetSubtailC(AjPReport thys, const char* tail)
{
    if(!thys)
        return;

    ajStrAssignC(&thys->SubTail, tail);

    return;
}





/* @obsolete ajReportSetSubTailC
** @rename ajReportSetSubtailC
*/

__deprecated void ajReportSetSubTailC(AjPReport thys, const char* tail)
{
    ajReportSetSubtailC(thys, tail);
    return;
}




/* @func ajReportSetSubtailS ***************************************************
**
** Defines a feature report subtail
**
** @param [u] thys [AjPReport] Report object
** @param [r] tail [const AjPStr] Report tail with embedded newlines
** @return [void]
** @@
******************************************************************************/

void ajReportSetSubtailS(AjPReport thys, const AjPStr tail)
{
    if(!thys)
        return;

    ajStrAssignS(&thys->SubTail, tail);

    return;
}





/* @obsolete ajReportSetSubTail
** @rename ajReportSetSubtailS
*/

__deprecated void ajReportSetSubTail(AjPReport thys, const AjPStr tail)
{
    ajReportSetSubtailS(thys, tail);
    return;
}




/* @func ajReportAppendSubtailC ***********************************************
**
** Appends to a feature report subtail
**
** @param [u] thys [AjPReport] Report object
** @param [r] tail [const char*] Report tail with embedded newlines
** @return [void]
** @@
******************************************************************************/

void ajReportAppendSubtailC(AjPReport thys, const char* tail)
{
    if(!thys)
        return;

    if(ajStrGetLen(thys->SubTail))
	if(ajStrGetCharLast(thys->SubTail) != '\n')
	    ajStrAppendK(&thys->SubTail, '\n');

    ajStrAppendC(&thys->SubTail, tail);

    return;
}




/* @obsolete ajReportAppendSubTailC
** @rename ajReportAppendSubtailC
*/

__deprecated void ajReportAppendSubTailC(AjPReport thys, const char* tail)
{
    ajReportAppendSubtailC(thys, tail);
    return;
}




/* @func ajReportAppendSubtailS ************************************************
**
** Appends to a feature report subtail
**
** @param [u] thys [AjPReport] Report object
** @param [r] tail [const AjPStr] Report tail with embedded newlines
** @return [void]
** @@
******************************************************************************/

void ajReportAppendSubtailS(AjPReport thys, const AjPStr tail)
{
    if(!thys)
        return;

    if(ajStrGetLen(thys->SubTail))
	if(ajStrGetCharLast(thys->SubTail) != '\n')
	    ajStrAppendK(&thys->SubTail, '\n');

    ajStrAppendS(&thys->SubTail, tail);

    return;
}




/* @obsolete ajReportAppendSubTail
** @rename ajReportAppendSubtailS
*/

__deprecated void ajReportAppendSubTail(AjPReport thys, const AjPStr tail)
{
    ajReportAppendSubtailS(thys, tail);
    return;
}




/* @func ajReportSetType ******************************************************
**
** Sets the report type (if it is not set already)
**
** @param [u] thys [AjPReport] Report object
** @param [r] ftable [const AjPFeattable] Feature table object
** @param [r] seq [const AjPSeq] Sequence object
** @return [void]
** @@
******************************************************************************/

void ajReportSetType(AjPReport thys,
		      const AjPFeattable ftable, const AjPSeq seq)
{
    ajDebug("ajReportSetType '%S' ft: '%S' sq: '%S'\n",
	    thys->Type, ftable->Type, seq->Type);

    if(ajStrGetLen(thys->Type))
	return;

    if(ajStrGetLen(ftable->Type))
    {
	ajStrAssignS(&thys->Type, ftable->Type);

	return;
    }

    if(seq && ajStrGetLen(seq->Type))
    {
	ajStrAssignS(&thys->Type, seq->Type);

	return;
    }

    return;
}




/* @funcstatic reportCharname *************************************************
**
** Returns 'residues' for a protein report, 'bases' for a nucleotide report.
**
** @param [r] thys [const AjPReport] Report object
** @return [const char*] String to print the sequence character type
******************************************************************************/

static const char* reportCharname(const AjPReport thys)
{
    static const char* protstr = "residues";
    static const char* nucstr = "bases";

    if(!ajStrGetLen(thys->Type))
	return protstr;

    switch(ajStrGetCharFirst(thys->Type))
    {
        case 'n':
        case 'N':
            return nucstr;
        default:
            break;
    }

    return protstr;
}




/* @func ajReportGetSeqnameSeq *************************************************
**
** Returns the sequence name or USA depending on the setting in the
** report object (derived from the ACD and command line -rusa option)
**
** @param [r] thys [const AjPReport] Report object
** @param [r] seq [const AjPSeq] Sequence object
** @return [const AjPStr] Sequence name for this report
******************************************************************************/

const AjPStr ajReportGetSeqnameSeq(const AjPReport thys, const AjPSeq seq)
{
    if(thys->Showusa)
	return ajSeqGetUsaS(seq);

    return ajSeqGetNameS(seq);
}




/* @obsolete ajReportSeqName
** @rename ajReportGetSeqnameSeq
*/

__deprecated const AjPStr ajReportSeqName(const AjPReport thys,
                                          const AjPSeq seq)
{
    return ajReportGetSeqnameSeq(thys, seq);
}




/* @func ajReportAddFileF ******************************************************
**
** Adds an additional file name and description to the report.
**
** These will appear in the header as "Additional files"
** to let the user know that there are associated output files available.
**
** @param [u] thys [AjPReport] Report object
** @param [u] file [AjPFile] File
** @param [r] type [const AjPStr] Type (simple text description)
** @return [void]
******************************************************************************/

void ajReportAddFileF(AjPReport thys, AjPFile file, const AjPStr type)
{
    AjPStr tmpname = NULL;
    AjPStr tmptype = NULL;

    if(!thys->FileTypes)
	thys->FileTypes = ajListstrNew();

    if(!thys->FileNames)
	thys->FileNames = ajListstrNew();

    ajStrAssignS(&tmptype, type);
    ajListstrPushAppend(thys->FileTypes, tmptype);

    ajFmtPrintS(&tmpname, "%F", file);
    ajListstrPushAppend(thys->FileNames, tmpname);

    return;
}




/* @obsolete ajReportFileAdd
** @rename ajReportAddFile
*/

__deprecated void ajReportFileAdd(AjPReport thys,
                                  AjPFile file, const AjPStr type)
{
    ajReportAddFileF(thys, file, type);
    return;
}




/* @func ajReportPrintFormat **************************************************
**
** Reports the internal data structures
**
** @param [u] outf [AjPFile] Output file
** @param [r] full [AjBool] Full report (usually ajFalse)
** @return [void]
** @@
******************************************************************************/

void ajReportPrintFormat(AjPFile outf, AjBool full)
{
    ajint i = 0;

    ajFmtPrintF(outf, "\n");
    ajFmtPrintF(outf, "# Report output formats\n");
    ajFmtPrintF(outf, "# Name    Format name (or alias)\n");
    ajFmtPrintF(outf, "# Alias   Alias name\n");
    ajFmtPrintF(outf, "# Nuc     Valid for nucleotide sequences\n");
    ajFmtPrintF(outf, "# Pro     Valid for protein sequences\n");
    ajFmtPrintF(outf, "# Head    Include standard header/footer blocks\n");
    ajFmtPrintF(outf, "# Mintags Minimum number of tags to be specified "
                "(0 for all)\n");
    ajFmtPrintF(outf, "# Showseq Includes sequence\n");
    ajFmtPrintF(outf, "# Desc    Format description\n");
    ajFmtPrintF(outf, "# Name          Alias Nuc Pro Head Mintags Showseq "
		"Description\n");
    ajFmtPrintF(outf, "\n");
    ajFmtPrintF(outf, "RFormat {\n");

    for(i=0; reportFormat[i].Name; i++)
	if(full || !reportFormat[i].Alias)
	    ajFmtPrintF(outf, "  %-12s %5B %3B %3B %3B  %7d %7B \"%s\"\n",
			reportFormat[i].Name,
			reportFormat[i].Alias,
			reportFormat[i].Nucleotide,
			reportFormat[i].Protein,
			reportFormat[i].Showheader,
			reportFormat[i].Mintags,
			reportFormat[i].Showseq,
			reportFormat[i].Desc);

    ajFmtPrintF(outf, "}\n\n");

    return;
}




/* @func ajReportPrinthtmlFormat **********************************************
**
** Reports the internal data structures
**
** @param [u] outf [AjPFile] Output file
** @return [void]
** @@
******************************************************************************/

void ajReportPrinthtmlFormat(AjPFile outf)
{
    ajint i = 0;

    ajFmtPrintF(outf, "<table border=3>");
    ajFmtPrintF(outf, "<tr><th>Report Format</th><th>Alias</th>\n");
    ajFmtPrintF(outf, "<th>Nuc</th><th>Pro</th><th>Showheader</th>\n");
    ajFmtPrintF(outf, "<th>Mintags</th><th>Showseq</th>\n");
    ajFmtPrintF(outf, "<th>Description</th></tr>\n");


    for(i=0; reportFormat[i].Name; i++)
	if(!reportFormat[i].Alias)
	    ajFmtPrintF(outf, "<tr><td>\n%-12s\n</td><td>%5B</td>"
                        "<td>%3B</td><td>%3B</td><td>%3B</td>"
                        "<td>%7d</td><td>%7B</td><td>\"%s\"</td></tr>\n",
			reportFormat[i].Name,
			reportFormat[i].Alias,
			reportFormat[i].Nucleotide,
			reportFormat[i].Protein,
			reportFormat[i].Showheader,
			reportFormat[i].Mintags,
			reportFormat[i].Showseq,
			reportFormat[i].Desc);

    ajFmtPrintF(outf, "</table>\n");

    return;
}




/* @func ajReportPrintbookFormat **********************************************
**
** Reports the report format internals in DocBook format
**
** @param [u] outf [AjPFile] Output file
** @return [void]
** @@
******************************************************************************/

void ajReportPrintbookFormat(AjPFile outf)
{
    ajint i = 0;
    ajint j = 0;
    AjPStr namestr = NULL;
    AjPList fmtlist;
    AjPStr* names;

    fmtlist = ajListstrNew();

    ajFmtPrintF(outf, "<para>The supported report formats are summarised "
                "in the table below. The columns are as follows: "
                "<emphasis>Output format</emphasis> (format name), "
                "<emphasis>Nuc</emphasis> "
                "(\"true\" indicates nucleotide sequence data may be "
                "represented), "
                "<emphasis>Pro</emphasis> (\"true\" indicates protein "
                "sequence data may be represented, "
                "<emphasis>Header</emphasis> (whether the standard EMBOSS "
                "report header is included), "
                "<emphasis>Seq</emphasis> (whether the sequence corresponding "
                "to the features is included), "
                "<emphasis>Tags</emphasis> (number of specific tag-values "
                "reported.  A non-zero value suggests a format is not "
                "suitable for application output that does not generate "
                "these specific tags.) "
                "<emphasis>Description</emphasis> (short description of "
                "the format).</para>\n\n");

    ajFmtPrintF(outf, "<table frame=\"box\" rules=\"cols\">\n");
    ajFmtPrintF(outf, "  <caption>Report formats</caption>\n");
    ajFmtPrintF(outf, "  <thead>\n");
    ajFmtPrintF(outf, "    <tr align=\"center\">\n");
    ajFmtPrintF(outf, "      <th>Output Format</th>\n");
    ajFmtPrintF(outf, "      <th>Nuc</th>\n");
    ajFmtPrintF(outf, "      <th>Pro</th>\n");
    ajFmtPrintF(outf, "      <th>Header</th>\n");
    ajFmtPrintF(outf, "      <th>Seq</th>\n");
    ajFmtPrintF(outf, "      <th>Tags</th>\n");
    ajFmtPrintF(outf, "      <th>Description</th>\n");
    ajFmtPrintF(outf, "    </tr>\n");
    ajFmtPrintF(outf, "  </thead>\n");
    ajFmtPrintF(outf, "  <tbody>\n");

    for(i=1; reportFormat[i].Name; i++)
    {
	if(!reportFormat[i].Alias)
        {
            namestr = ajStrNewC(reportFormat[i].Name);
            ajListPush(fmtlist, namestr);
            namestr = NULL;
        }
    }

    ajListSort(fmtlist, ajStrVcmp);
    ajListstrToarray(fmtlist, &names);

    for(i=0; names[i]; i++)
    {
        for(j=0; reportFormat[j].Name; j++)
        {
            if(ajStrMatchC(names[i],reportFormat[j].Name))
            {
                ajFmtPrintF(outf, "    <tr>\n");
                ajFmtPrintF(outf, "      <td>%s</td>\n",
                            reportFormat[j].Name);
                ajFmtPrintF(outf, "      <td>%B</td>\n",
                            reportFormat[j].Nucleotide);
                ajFmtPrintF(outf, "      <td>%B</td>\n",
                            reportFormat[j].Protein);
                ajFmtPrintF(outf, "      <td>%B</td>\n",
                            reportFormat[j].Showheader);
                ajFmtPrintF(outf, "      <td>%B</td>\n",
                            reportFormat[j].Showseq);
                ajFmtPrintF(outf, "      <td>%d</td>\n",
                            reportFormat[j].Mintags);
                ajFmtPrintF(outf, "      <td>%s</td>\n",
                            reportFormat[j].Desc);
                ajFmtPrintF(outf, "    </tr>\n");
            }
        }
    }

    ajFmtPrintF(outf, "  </tbody>\n");
    ajFmtPrintF(outf, "</table>\n");
    ajStrDel(&namestr);

    names = NULL;
    ajListstrFreeData(&fmtlist);

    return;
}




/* @func ajReportPrintwikiFormat **********************************************
**
** Reports the report format internals as wikitext
**
** @param [u] outf [AjPFile] Output file
** @return [void]
** @@
******************************************************************************/

void ajReportPrintwikiFormat(AjPFile outf)
{
    ajint i = 0;
    ajint j = 0;
    AjPStr namestr = NULL;

    ajFmtPrintF(outf, "{| class=\"wikitable sortable\" border=\"2\"\n");
    ajFmtPrintF(outf, "|-\n");
    ajFmtPrintF(outf, "!Format!!Nuc!!Pro!!Header!!Seq||Tags!!"
                "class=\"unsortable\"|Description\n");

    for(i=1; reportFormat[i].Name; i++)
    {
        if(!reportFormat[i].Alias)
        {
            ajFmtPrintF(outf, "|-\n");
            ajStrAssignC(&namestr, reportFormat[i].Name);


            for(j=i+1; reportFormat[j].Name; j++)
            {
                if(reportFormat[j].Write == reportFormat[i].Write)
                {
                    ajFmtPrintAppS(&namestr, "<br>%s", reportFormat[j].Name);
                    if(!reportFormat[j].Alias) 
                    {
                        ajWarn("Report output format '%s' same as '%s' "
                               "but not alias",
                               reportFormat[j].Name,
                               reportFormat[i].Name);
                    }
                }
            }
            ajFmtPrintF(outf, "|%S||%B||%B||%B||%B||%d||%s\n",
			namestr,
			reportFormat[i].Nucleotide,
			reportFormat[i].Protein,
			reportFormat[i].Showheader,
			reportFormat[i].Showseq,
			reportFormat[i].Mintags,
			reportFormat[i].Desc);
        }
    }

    ajFmtPrintF(outf, "|}\n");

    ajStrDel(&namestr);

    return;
}




/* @func ajReportDummyFunction ************************************************
**
** Dummy function to catch all unused functions defined in the ajreport
** source file.
**
** @return [void]
**
******************************************************************************/

void ajReportDummyFunction(void)
{
    return;
}




/* @func ajReportExit *********************************************************
**
** Cleans up report processing internal memory
**
** @return [void]
** @@
******************************************************************************/

void ajReportExit(void)
{
    ajRegFree(&reportTagExp);

    return;
}
