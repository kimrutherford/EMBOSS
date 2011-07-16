#ifdef __cplusplus
extern "C"
{
#endif

#ifndef embindex_h
#define embindex_h



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
** @attr reffiles [AjPList] List of data reference filenames
** @attr fields [AjPList] EMBOSS index field structures
** @attr id [AjPStr] Entry identifier
** @attr idcache [AjPBtcache] Id cache structure
** @attr idpagecount [ajlong] Cache page count
** @attr do_id [AjBool] If true, build id index
** @attr compressed [AjBool] If true, compress id index
** @attr nfiles [ajuint] Data file count
** @attr cachesize [ajuint] Defalt cache size
** @attr pagesize [ajuint] Default page size
** @attr idlen [ajuint] Maximum id length in index
** @attr idmaxlen [ajuint] Maximum id length in data
** @attr idtruncate [ajuint] Number of ids truncated
** @attr idpagesize [ajuint] Id index page size
** @attr idcachesize [ajuint] Id index cache size
** @attr idorder [ajuint] Id index primary order
** @attr idfill [ajuint] Id index primary fill count
** @attr idsecorder [ajuint] Id index secondary order
** @attr idsecfill [ajuint] Id index secondary fill count
** @attr fpos [ajlong] Input file position
** @attr reffpos [ajlong] Input second (reference) file position
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
    AjPList reffiles;
    AjPList fields;

    AjPStr id;
    AjPBtcache idcache;
    ajlong idpagecount;

    AjBool do_id;
    AjBool compressed;

    ajuint nfiles;
    ajuint cachesize;
    ajuint pagesize;

    ajuint idlen;
    ajuint idmaxlen;
    ajuint idtruncate;

    ajuint idpagesize;
    ajuint idcachesize;
    ajuint idorder;
    ajuint idfill;
    ajuint idsecorder;
    ajuint idsecfill;
    

    ajlong fpos;
    ajlong reffpos;
    
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
** @attr pagecount [ajulong] Index page count
** @attr pagesize  [ajuint] Index page size
** @attr cachesize [ajuint] Index cache size
** @attr order     [ajuint] Primary page order
** @attr fill      [ajuint] Primary page fill count
** @attr secorder  [ajuint] Secondary page order
** @attr secfill   [ajuint] Secondary page fill count
** @attr len       [ajuint] Maximum keyword length in index
** @attr maxlen    [ajuint] Maximum keyword length in data
** @attr truncate  [ajuint] Number of keywords truncated
** @attr secondary [AjBool] Secondary index if true
** @attr compressed [AjBool] Compress index if true
** @attr Padding   [char[4]] Padding to alignment boundary
******************************************************************************/
    
typedef struct EmbSBtreeField
{
    AjPBtcache cache;
    AjPList data;
    AjPStr name;
    AjPStr extension;
    AjPStr maxkey;
    ajulong pagecount;
    ajuint pagesize;
    ajuint cachesize;
    ajuint order;
    ajuint fill;
    ajuint secorder;
    ajuint secfill;
    ajuint len;
    ajuint maxlen;
    ajuint truncate;
    AjBool secondary;
    AjBool compressed;
    char   Padding[4];
} EmbOBtreeField;
#define EmbPBtreeField EmbOBtreeField*

/*
** Prototype definitions
*/

void   embBtreeIndexEntry(EmbPBtreeEntry entry,
                          ajuint dbno);
void   embBtreeIndexField(EmbPBtreeField field,
                          const EmbPBtreeEntry entry,
                          ajuint dbno);
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
EmbPBtreeEntry embBtreeEntryNew(void);
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
EmbPBtreeField embBtreeFieldNewS(const AjPStr name);
void           embBtreeFieldDel(EmbPBtreeField *Pthis);
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

#endif

#ifdef __cplusplus
}
#endif
