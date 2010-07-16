#ifdef __cplusplus
extern "C"
{
#endif

#ifndef ensqcsequence_h
#define ensqcsequence_h

#include "ensqcdatabase.h"




/* @data EnsPQcsequenceadaptor ************************************************
**
** Ensembl QC Sequence Adaptor
**
** @alias EnsSQcsequenceadaptor
** @alias EnsOQcsequenceadaptor
**
** @attr Adaptor [EnsPBaseadaptor] Ensembl Base Adaptor
** @attr CacheByIdentifier [AjPTable] Identifier cache
** @attr CacheByName [AjPTable] Name cache
** @@
** FIXME: Should this also have a CacheByAccessionVersion???
******************************************************************************/

typedef struct EnsSQcsequenceadaptor
{
    EnsPBaseadaptor Adaptor;
    AjPTable CacheByIdentifier;
    AjPTable CacheByName;
} EnsOQcsequenceadaptor;

#define EnsPQcsequenceadaptor EnsOQcsequenceadaptor*




/* @data EnsPQcsequence *******************************************************
**
** Ensembl QC Sequence
**
** @alias EnsSQcsequence
** @alias EnsOQcsequence
**
** @attr Use [ajuint] Use counter
** @cc Bio::EnsEMBL::Storable
** @attr Identifier [ajuint] SQL database-internal identifier
** @attr Adaptor [EnsPQcsequenceadaptor] Ensembl Sequence Adaptor
** @cc Bio::EnsEMBL::QC::Sequence
** @cc 'sequence' SQL table
** @attr Qcdatabase [EnsPQcdatabase] Ensembl QC Database
** @attr Name [AjPStr] Name
** @attr Accession [AjPStr] Accession
** @attr Description [AjPStr] Description
** @attr Type [AjPStr] Type
** @attr Version [ajuint] Version
** @attr Length [ajuint] Length
** @attr CDSStart [ajuint] Coding sequence start
** @attr CDSEnd [ajuint] Coding sequence end
** @attr CDSStrand [ajint] Coding sequence strand
** @attr PolyA [ajuint] Poly A+ tail length
** @@
******************************************************************************/

typedef struct EnsSQcsequence
{
    ajuint Use;
    ajuint Identifier;
    EnsPQcsequenceadaptor Adaptor;
    EnsPQcdatabase Qcdatabase;
    AjPStr Name;
    AjPStr Accession;
    AjPStr Description;
    AjPStr Type;
    ajuint Version;
    ajuint Length;
    ajuint CDSStart;
    ajuint CDSEnd;
    ajint CDSStrand;
    ajuint PolyA;
} EnsOQcsequence;

#define EnsPQcsequence EnsOQcsequence*




/*
** Prototype definitions
*/

/* Ensembl Quality Check Sequence */

EnsPQcsequence ensQcsequenceNew(EnsPQcsequenceadaptor adaptor,
                                ajuint identifier,
                                EnsPQcdatabase qcdb,
                                AjPStr name,
                                AjPStr accession,
                                ajuint version,
                                AjPStr type,
                                ajuint length,
                                ajuint cdsstart,
                                ajuint cdsend,
                                ajint cdsstrand,
                                ajuint polya,
                                AjPStr description);

EnsPQcsequence ensQcsequenceNewObj(const EnsPQcsequence object);

EnsPQcsequence ensQcsequenceNewRef(EnsPQcsequence qcs);

void ensQcsequenceDel(EnsPQcsequence* Pqcs);

EnsPQcsequenceadaptor ensQcsequenceGetAdaptor(const EnsPQcsequence qcs);

ajuint ensQcsequenceGetIdentifier(const EnsPQcsequence qcs);

EnsPQcdatabase ensQcsequenceGetQcdatabase(const EnsPQcsequence qcs);

AjPStr ensQcsequenceGetName(const EnsPQcsequence qcs);

AjPStr ensQcsequenceGetAccession(const EnsPQcsequence qcs);

ajuint ensQcsequenceGetVersion(const EnsPQcsequence qcs);

AjPStr ensQcsequenceGetType(const EnsPQcsequence qcs);

ajuint ensQcsequenceGetLength(const EnsPQcsequence qcs);

ajuint ensQcsequenceGetCDSStart(const EnsPQcsequence qcs);

ajuint ensQcsequenceGetCDSEnd(const EnsPQcsequence qcs);

ajint ensQcsequenceGetCDSStrand(const EnsPQcsequence qcs);

ajuint ensQcsequenceGetPolyA(const EnsPQcsequence qcs);

AjPStr ensQcsequenceGetDescription(const EnsPQcsequence qcs);

AjBool ensQcsequenceSetAdaptor(EnsPQcsequence qcs, EnsPQcsequenceadaptor qcsa);

AjBool ensQcsequenceSetIdentifier(EnsPQcsequence qcs, ajuint identifier);

AjBool ensQcsequenceSetQcdatabase(EnsPQcsequence qcs, EnsPQcdatabase qcdb);

AjBool ensQcsequenceSetName(EnsPQcsequence qcs, AjPStr name);

AjBool ensQcsequenceSetAccession(EnsPQcsequence qcs, AjPStr accession);

AjBool ensQcsequenceSetVersion(EnsPQcsequence qcs, ajuint version);

AjBool ensQcsequenceSetType(EnsPQcsequence qcs, AjPStr type);

AjBool ensQcsequenceSetLength(EnsPQcsequence qcs, ajuint length);

AjBool ensQcsequenceSetCDSStart(EnsPQcsequence qcs, ajuint cdsstart);

AjBool ensQcsequenceSetCDSEnd(EnsPQcsequence qcs, ajuint cdsend);

AjBool ensQcsequenceSetCDSStrand(EnsPQcsequence qcs, ajint cdsstrand);

AjBool ensQcsequenceSetPolyA(EnsPQcsequence qcs, ajuint polya);

AjBool ensQcsequenceSetDescription(EnsPQcsequence qcs, AjPStr description);

ajulong ensQcsequenceGetMemsize(const EnsPQcsequence qcs);

AjBool ensQcsequenceTrace(const EnsPQcsequence qcs, ajuint level);

AjBool ensQcsequenceMatch(const EnsPQcsequence qcs1,
                          const EnsPQcsequence qcs2);

ajuint ensQcsequenceGetQcdatabaseIdentifier(const EnsPQcsequence qcs);

AjBool ensHTMLEncodeSGMLID(AjPStr *Pstr);

AjBool ensHTMLEncodeEntities(AjPStr *Pstr);

AjBool ensQcsequenceFetchExternalURL(const EnsPQcsequence qcs, AjPStr *Pstr);

AjBool ensQcsequenceFetchExternalAnchor(const EnsPQcsequence qcs,
                                        AjPStr *Pstr,
                                        AjBool htmlid);

AjBool ensQcsequenceFetchInternalAnchor(const EnsPQcsequence qcs,
                                        AjPStr *Pstr);

/* Ensembl Quality Check Sequence Adaptor */

EnsPQcsequenceadaptor ensRegistryGetQcsequenceadaptor(
    EnsPDatabaseadaptor dba);

EnsPQcsequenceadaptor ensQcsequenceadaptorNew(
    EnsPDatabaseadaptor dba);

void ensQcsequenceadaptorDel(EnsPQcsequenceadaptor* Pqcsa);

AjBool ensQcsequenceadaptorFetchByIdentifier(EnsPQcsequenceadaptor adaptor,
                                             ajuint identifier,
                                             EnsPQcsequence *Pqcs);

AjBool ensQcsequenceadaptorFetchByAccession(EnsPQcsequenceadaptor qcsa,
                                            ajuint qcdbid,
                                            const AjPStr accession,
                                            EnsPQcsequence *Pqcs);

AjBool ensQcsequenceadaptorFetchByAccessionVersion(EnsPQcsequenceadaptor qcsa,
                                                   ajuint qcdbid,
                                                   const AjPStr accession,
                                                   ajuint version,
                                                   EnsPQcsequence *Pqcs);

AjBool ensQcsequenceadaptorFetchByName(EnsPQcsequenceadaptor qcsa,
                                       ajuint qcdbid,
                                       const AjPStr name,
                                       EnsPQcsequence *Pqcs);

AjBool ensQcsequenceadaptorFetchAllByQcdatabase(EnsPQcsequenceadaptor qcsa,
                                                const EnsPQcdatabase qcdb,
                                                AjPList qcss);

AjBool ensQcsequenceadaptorStore(EnsPQcsequenceadaptor qcsa,
                                 EnsPQcsequence qcs);

AjBool ensQcsequenceadaptorUpdate(EnsPQcsequenceadaptor qcsa,
                                  const EnsPQcsequence qcs);

AjBool ensQcsequenceadaptorDelete(EnsPQcsequenceadaptor qcsa,
                                  EnsPQcsequence qcs);


/*
** End of prototype definitions
*/




#endif /* ensqcsequence_h */

#ifdef __cplusplus
}
#endif
