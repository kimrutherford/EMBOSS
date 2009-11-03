#ifdef __cplusplus
extern "C"
{
#endif

#ifndef embindex_h
#define embindex_h

#define BTREE_DEF_IDLEN     15
#define BTREE_DEF_ACLEN     15
#define BTREE_DEF_SVLEN     15
#define BTREE_DEF_KWLEN     15
#define BTREE_DEF_DELEN     15
#define BTREE_DEF_TXLEN     15
#define BTREE_DEF_KWLIMIT   15
#define BTREE_DEF_CACHESIZE 100
#define BTREE_DEF_PAGESIZE  2048




/* @data EmbPBtreeEntry *******************************************************
**
** Index tree entries
**
** @alias EmbSBtreeEntry
**
** @attr do_id [AjBool] Undocumented
** @attr do_accession [AjBool] Undocumented
** @attr do_sv [AjBool] Undocumented
** @attr do_description [AjBool] Undocumented
** @attr do_keyword [AjBool] Undocumented
** @attr do_taxonomy [AjBool] Undocumented
** @attr dbname [AjPStr] Undocumented
** @attr dbrs [AjPStr] Undocumented
** @attr release [AjPStr] Undocumented
** @attr date [AjPStr] Undocumented
** @attr dbtype [AjPStr] Undocumented
** @attr directory [AjPStr] Undocumented
** @attr idirectory [AjPStr] Undocumented
** @attr files [AjPList] Undocumented
** @attr reffiles [AjPList] Undocumented
** @attr nfiles [ajuint] Undocumented
** @attr cachesize [ajuint] Undocumented
** @attr pagesize [ajuint] Undocumented
** @attr idlen [ajuint] Undocumented
** @attr aclen [ajuint] Undocumented
** @attr svlen [ajuint] Undocumented
** @attr kwlen [ajuint] Undocumented
** @attr delen [ajuint] Undocumented
** @attr txlen [ajuint] Undocumented
** @attr idpagesize [ajuint] Undocumented
** @attr idcachesize [ajuint] Undocumented
** @attr idorder [ajuint] Undocumented
** @attr idfill [ajuint] Undocumented
** @attr idsecorder [ajuint] Undocumented
** @attr idsecfill [ajuint] Undocumented
** @attr acpagesize [ajuint] Undocumented
** @attr accachesize [ajuint] Undocumented
** @attr acorder [ajuint] Undocumented
** @attr acfill [ajuint] Undocumented
** @attr acsecorder [ajuint] Undocumented
** @attr acsecfill [ajuint] Undocumented
** @attr svpagesize [ajuint] Undocumented
** @attr svcachesize [ajuint] Undocumented
** @attr svorder [ajuint] Undocumented
** @attr svfill [ajuint] Undocumented
** @attr svsecorder [ajuint] Undocumented
** @attr svsecfill [ajuint] Undocumented
** @attr kwpagesize [ajuint] Undocumented
** @attr kwcachesize [ajuint] Undocumented
** @attr kworder [ajuint] Undocumented
** @attr kwfill [ajuint] Undocumented
** @attr kwsecorder [ajuint] Undocumented
** @attr kwsecfill [ajuint] Undocumented
** @attr depagesize [ajuint] Undocumented
** @attr decachesize [ajuint] Undocumented
** @attr deorder [ajuint] Undocumented
** @attr defill [ajuint] Undocumented
** @attr desecorder [ajuint] Undocumented
** @attr desecfill [ajuint] Undocumented
** @attr txpagesize [ajuint] Undocumented
** @attr txcachesize [ajuint] Undocumented
** @attr txorder [ajuint] Undocumented
** @attr txfill [ajuint] Undocumented
** @attr txsecorder [ajuint] Undocumented
** @attr txsecfill [ajuint] Undocumented
** @attr Padding [ajuint] Padding to alignment boundary
** @attr idcache [AjPBtcache] Undocumented
** @attr accache [AjPBtcache] Undocumented
** @attr svcache [AjPBtcache] Undocumented
** @attr kwcache [AjPBtcache] Undocumented
** @attr decache [AjPBtcache] Undocumented
** @attr txcache [AjPBtcache] Undocumented
** @attr fpos [ajlong] Undocumented
** @attr reffpos [ajlong] Undocumented
** @attr id [AjPStr] Undocumented
** @attr ac [AjPList] Undocumented
** @attr sv [AjPList] Undocumented
** @attr tx [AjPList] Undocumented
** @attr kw [AjPList] Undocumented
** @attr de [AjPList] Undocumented
******************************************************************************/

typedef struct EmbSBtreeEntry
{
    AjBool do_id;
    AjBool do_accession;
    AjBool do_sv;
    AjBool do_description;
    AjBool do_keyword;
    AjBool do_taxonomy;
    AjPStr dbname;
    AjPStr dbrs;
    AjPStr release;
    AjPStr date;
    AjPStr dbtype;

    AjPStr directory;
    AjPStr idirectory;

    AjPList files;
    AjPList reffiles;
    ajuint   nfiles;
    ajuint cachesize;
    ajuint pagesize;

    ajuint idlen;
    ajuint aclen;
    ajuint svlen;
    ajuint kwlen;
    ajuint delen;
    ajuint txlen;

    ajuint idpagesize;
    ajuint idcachesize;
    ajuint idorder;
    ajuint idfill;
    ajuint idsecorder;
    ajuint idsecfill;
    ajuint acpagesize;
    ajuint accachesize;
    ajuint acorder;
    ajuint acfill;
    ajuint acsecorder;
    ajuint acsecfill;
    ajuint svpagesize;
    ajuint svcachesize;
    ajuint svorder;
    ajuint svfill;
    ajuint svsecorder;
    ajuint svsecfill;

    ajuint kwpagesize;
    ajuint kwcachesize;
    ajuint kworder;
    ajuint kwfill;
    ajuint kwsecorder;
    ajuint kwsecfill;
    ajuint depagesize;
    ajuint decachesize;
    ajuint deorder;
    ajuint defill;
    ajuint desecorder;
    ajuint desecfill;
    ajuint txpagesize;
    ajuint txcachesize;
    ajuint txorder;
    ajuint txfill;
    ajuint txsecorder;
    ajuint txsecfill;
    ajuint Padding;
    
    AjPBtcache idcache;
    AjPBtcache accache;
    AjPBtcache svcache;
    AjPBtcache kwcache;
    AjPBtcache decache;
    AjPBtcache txcache;

    ajlong fpos;
    ajlong reffpos;
    
    AjPStr id;
    AjPList ac;
    AjPList sv;
    AjPList tx;
    AjPList kw;
    AjPList de;
} EmbOBtreeEntry;
#define EmbPBtreeEntry EmbOBtreeEntry*
 



/*
** Prototype definitions
*/

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
ajuint          embBtreeSetFields(EmbPBtreeEntry entry, AjPStr const * fields);
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
