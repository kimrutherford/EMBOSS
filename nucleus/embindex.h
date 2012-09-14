/* @include embindex **********************************************************
**
** B+ Tree Indexing plus Disc Cache.
**
** @author Copyright (c) 2003 Alan Bleasby
** @version $Revision: 1.34 $
** @modified $Date: 2012/05/24 16:57:10 $ by $Author: rice $
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

#ifndef EMBINDEX_H
#define EMBINDEX_H



/* ========================================================================= */
/* ============================= include files ============================= */
/* ========================================================================= */

#include "ajdefine.h"
#include "ajstr.h"
#include "ajlist.h"
#include "ajindex.h"
#include "ajreg.h"

AJ_BEGIN_DECLS




/* ========================================================================= */
/* =============================== constants =============================== */
/* ========================================================================= */




/* ========================================================================= */
/* ============================== public data ============================== */
/* ========================================================================= */




/* @data EmbPBtreeEntry *******************************************************
**
** Index tree entries
**
** @alias EmbOBtreeEntry
** @alias EmbSBtreeEntry
**
** @attr dbname [AjPStr] Database name
** @attr dbrs [AjPStr] Index resource definition
** @attr release [AjPStr] Release number
** @attr date [AjPStr] Release date
** @attr dbtype [AjPStr] Database type
** @attr directory [AjPStr] Database directory
** @attr idirectory [AjPStr] Index directory
** @attr idextension [AjPStr] Id index extension
** @attr maxid [AjPStr] Longest id in data
** @attr files [AjPList] List of data filenames
** @attr reffiles [AjPList*] Lists of data reference filenames
** @attr fields [AjPList] EMBOSS index field structures
** @attr id [AjPStr] Entry identifier
** @attr idcache [AjPBtcache] Id cache structure
** @attr pripagecount [ajlong] Cache primary page count
** @attr secpagecount [ajlong] Cache secondary page count
** @attr do_id [AjBool] If true, build id index
** @attr compressed [AjBool] If true, compress id index
** @attr nfiles [ajuint] Data file count
** @attr refcount [ajuint] Reference file(s) for each entry
** @attr idlen [ajuint] Maximum id length in index
** @attr idmaxlen [ajuint] Maximum id length in data
** @attr idtruncate [ajuint] Number of ids truncated
** @attr pripagesize [ajuint] Default page size
** @attr pricachesize [ajuint] Defalt cache size
** @attr idorder [ajuint] Id index primary order
** @attr idfill [ajuint] Id index primary fill count
** @attr secpagesize [ajuint] Default page size
** @attr seccachesize [ajuint] Defalt cache size
** @attr idsecorder [ajuint] Id index secondary order
** @attr idsecfill [ajuint] Id index secondary fill count
** @attr fpos [ajlong] Input file position
** @attr reffpos [ajlong*] Input extra (reference) file positions
******************************************************************************/

typedef struct EmbSBtreeEntry
{
    AjPStr dbname;
    AjPStr dbrs;
    AjPStr release;
    AjPStr date;
    AjPStr dbtype;

    AjPStr directory;
    AjPStr idirectory;
    AjPStr idextension;
    AjPStr maxid;

    AjPList files;
    AjPList *reffiles;
    AjPList fields;

    AjPStr id;
    AjPBtcache idcache;
    ajlong pripagecount;
    ajlong secpagecount;

    AjBool do_id;
    AjBool compressed;

    ajuint nfiles;

    ajuint refcount;

    ajuint idlen;
    ajuint idmaxlen;
    ajuint idtruncate;

    ajuint pripagesize;
    ajuint pricachesize;

    ajuint idorder;
    ajuint idfill;

    ajuint secpagesize;
    ajuint seccachesize;

    ajuint idsecorder;
    ajuint idsecfill;
    

    ajlong fpos;
    ajlong *reffpos;
    
} EmbOBtreeEntry;
#define EmbPBtreeEntry EmbOBtreeEntry*
 

/* @data EmbPBtreeField *******************************************************
**
** Index tree entries
**
** @alias EmbSBtreeField
** @alias EmbOBtreeField
**
** @attr cache     [AjPBtcache] Cache structure
** @attr data      [AjPList] Keywords to index
** @attr name      [AjPStr] File basename
** @attr extension [AjPStr] File extension
** @attr maxkey    [AjPStr] Longest keyword found
** @attr freelist  [AjPStr*] Free data elements for reuse
** @attr pripagecount [ajulong] Index primary page count
** @attr secpagecount [ajulong] Index secondary page count
** @attr pripagesize  [ajuint] Index primary page size
** @attr secpagesize  [ajuint] Index secondary page size
** @attr pricachesize [ajuint] Index primary cache size
** @attr seccachesize [ajuint] Index secondary cache size
** @attr order     [ajuint] Primary page order
** @attr fill      [ajuint] Primary page fill count
** @attr secorder  [ajuint] Secondary page order
** @attr secfill   [ajuint] Secondary page fill count
** @attr refcount  [ajuint] Number of reference file(s) per entry
** @attr len       [ajuint] Maximum keyword length in index
** @attr idlen     [ajuint] Maximum id length in index
** @attr maxlen    [ajuint] Maximum keyword length in data
** @attr truncate  [ajuint] Number of keywords truncated
** @attr freecount [ajuint] Free list used
** @attr freesize  [ajuint] Free list size
** @attr secondary [AjBool] Secondary index if true
** @attr compressed [AjBool] Compress index if true
** @attr Padding [char[4]] Padding to alignment boundary
******************************************************************************/
    
typedef struct EmbSBtreeField
{
    AjPBtcache cache;
    AjPList data;
    AjPStr name;
    AjPStr extension;
    AjPStr maxkey;
    AjPStr *freelist;
    ajulong pripagecount;
    ajulong secpagecount;
    ajuint pripagesize;
    ajuint secpagesize;
    ajuint pricachesize;
    ajuint seccachesize;
    ajuint order;
    ajuint fill;
    ajuint secorder;
    ajuint secfill;
    ajuint refcount;
    ajuint len;
    ajuint idlen;
    ajuint maxlen;
    ajuint truncate;
    ajuint freecount;
    ajuint freesize;
    AjBool secondary;
    AjBool compressed;
    char   Padding[4];
} EmbOBtreeField;
#define EmbPBtreeField EmbOBtreeField*

/* ========================================================================= */
/* =========================== public functions ============================ */
/* ========================================================================= */



/*
** Prototype definitions
*/

void   embBtreeIndexEntry(EmbPBtreeEntry entry,
                          ajuint dbno);
void   embBtreeIndexField(EmbPBtreeField field,
                          const EmbPBtreeEntry entry,
                          ajuint dbno);
ajuint embBtreeIndexPrimary(EmbPBtreeField field,
                            const EmbPBtreeEntry entry,
                            ajuint dbno);
ajuint embBtreeIndexSecondary(EmbPBtreeField field,
                              const EmbPBtreeEntry entry);

void   embBtreeFindEmblAc(const AjPStr readline, EmbPBtreeField field,
                          AjPStr *Pstr);
void   embBtreeParseEmblAc(const AjPStr readline, EmbPBtreeField field);
void   embBtreeParseEmblDe(const AjPStr readline, EmbPBtreeField field);
void   embBtreeParseEmblKw(const AjPStr readline, EmbPBtreeField field);
void   embBtreeParseEmblSv(const AjPStr readline, EmbPBtreeField field);
void   embBtreeParseEmblTx(const AjPStr readline, EmbPBtreeField field);
void   embBtreeParseFastaAc(const AjPStr readline, EmbPBtreeField field);
void   embBtreeParseFastaDe(const AjPStr readline, EmbPBtreeField field);
void   embBtreeParseFastaSv(const AjPStr readline, EmbPBtreeField field);
void   embBtreeParseGenbankAc(const AjPStr readline, EmbPBtreeField field);
void   embBtreeParseGenbankDe(const AjPStr readline, EmbPBtreeField field);
void   embBtreeParseGenbankKw(const AjPStr readline, EmbPBtreeField field);
void   embBtreeParseGenbankTx(const AjPStr readline, EmbPBtreeField field);

void   embBtreeParseEntry(const AjPStr readline, AjPRegexp regexp,
                          EmbPBtreeEntry entry);
void   embBtreeParseField(const AjPStr readline, AjPRegexp regexp,
                          EmbPBtreeField field);
void   embBtreeParseFieldSecond(const AjPStr readline, AjPRegexp regexp,
                               EmbPBtreeField field);
void   embBtreeParseFieldThird(const AjPStr readline, AjPRegexp regexp,
                               EmbPBtreeField field);
void   embBtreeParseFieldTrim(const AjPStr readline, AjPRegexp regexp,
                              EmbPBtreeField field);
void   embBtreeReportEntry(AjPFile outf, const EmbPBtreeEntry entry);
void   embBtreeReportField(AjPFile outf, const EmbPBtreeField field);
void   embBtreeEmblAC(const AjPStr acline, AjPList aclist);
void   embBtreeEmblKW(const AjPStr kwline, AjPList kwlist, ajuint maxlen);
void   embBtreeEmblDE(const AjPStr deline, AjPList delist, ajuint maxlen);
void   embBtreeEmblSV(const AjPStr idline, AjPList svlist);
void   embBtreeEmblTX(const AjPStr kwline, AjPList kwlist, ajuint maxlen);
void   embBtreeGenBankAC(const AjPStr acline, AjPList aclist);
void   embBtreeGenBankKW(const AjPStr kwline, AjPList kwlist, ajuint maxlen);
void   embBtreeGenBankDE(const AjPStr kwline, AjPList kwlist, ajuint maxlen);
void   embBtreeGenBankTX(const AjPStr kwline, AjPList kwlist, ajuint maxlen);

void   embBtreeFastaDE(const AjPStr kwline, AjPList kwlist, ajuint maxlen);
void   embBtreeFastaSV(const AjPStr kwline, AjPList kwlist, ajuint maxlen);


ajuint  embBtreeReadDir(AjPStr **filelist, const AjPStr fdirectory,
		       const AjPStr files, const AjPStr exclude);
EmbPBtreeEntry embBtreeEntryNew(ajuint refcount);
void           embBtreeEntrySetCompressed(EmbPBtreeEntry entry);
ajuint         embBtreeSetFields(EmbPBtreeEntry entry, AjPStr const * fields);
void           embBtreeEntryDel(EmbPBtreeEntry *thys);
void           embBtreeSetDbInfo(EmbPBtreeEntry entry, const AjPStr name,
				 const AjPStr dbrs,
		                 const AjPStr date, const AjPStr release,
		                 const AjPStr type, const AjPStr directory,
		                 const AjPStr idirectory);
ajuint          embBtreeGetFiles(EmbPBtreeEntry entry, const AjPStr fdirectory,
				const AjPStr files, const AjPStr exclude);
AjBool         embBtreeWriteEntryFile(const EmbPBtreeEntry entry);
void           embBtreeGetRsInfo(EmbPBtreeEntry entry);
AjBool         embBtreeOpenCaches(EmbPBtreeEntry entry);
AjBool         embBtreeCloseCaches(EmbPBtreeEntry entry);
AjBool         embBtreeDumpParameters(EmbPBtreeEntry entry);

EmbPBtreeField embBtreeFieldNewC(const char* nametxt);
EmbPBtreeField embBtreeFieldNewS(const AjPStr name, ajuint refcount);
void           embBtreeFieldDel(EmbPBtreeField *Pthis);
AjBool         embBtreeFieldGetdataS(EmbPBtreeField field, AjPStr *Pstr);
void           embBtreeFieldSetCompressed(EmbPBtreeField field);
void           embBtreeFieldSetSecondary(EmbPBtreeField field);
void           embBtreeFieldSetIdtype(EmbPBtreeField field);
EmbPBtreeField embBtreeGetFieldC(EmbPBtreeEntry entry, const char * nametxt);
EmbPBtreeField embBtreeGetFieldS(EmbPBtreeEntry entry, const AjPStr name);

void           embIndexExit(void);

/*
** End of prototype definitions
*/

#if 0
AjBool         embBtreeProbeCaches(EmbPBtreeEntry entry);
#endif


AJ_END_DECLS

#endif  /* !EMBINDEX_H */

