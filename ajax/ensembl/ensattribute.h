
#ifndef ENSATTRIBUTE_H
#define ENSATTRIBUTE_H

/* ==================================================================== */
/* ========================== include files =========================== */
/* ==================================================================== */

#include "ensdata.h"

AJ_BEGIN_DECLS




/* ==================================================================== */
/* ============================ constants ============================= */
/* ==================================================================== */




/* ==================================================================== */
/* ========================== public data ============================= */
/* ==================================================================== */




/* ==================================================================== */
/* ======================= public functions =========================== */
/* ==================================================================== */

/*
** Prototype definitions
*/

/* Ensembl Attribute */

EnsPAttribute ensAttributeNewCpy(const EnsPAttribute attribute);

EnsPAttribute ensAttributeNewIni(EnsPAttributetype at,
                                 AjPStr value);

EnsPAttribute ensAttributeNewRef(EnsPAttribute attribute);

void ensAttributeDel(EnsPAttribute* Pattribute);

EnsPAttributetype ensAttributeGetAttributetype(const EnsPAttribute attribute);

AjPStr ensAttributeGetValue(const EnsPAttribute attribute);

AjBool ensAttributeTrace(const EnsPAttribute attribute, ajuint level);

size_t ensAttributeCalculateMemsize(const EnsPAttribute attribute);

AjPStr ensAttributeGetCode(const EnsPAttribute attribute);

AjPStr ensAttributeGetDescription(const EnsPAttribute attribute);

AjPStr ensAttributeGetName(const EnsPAttribute attribute);

/* Ensembl Attribute Adaptor */

EnsPAttributeadaptor ensRegistryGetAttributeadaptor(
    EnsPDatabaseadaptor dba);

EnsPDatabaseadaptor ensAttributeadaptorGetDatabaseadaptor(
    EnsPAttributeadaptor ata);

AjBool ensAttributeadaptorFetchAllbyGene(
    EnsPAttributeadaptor ata,
    const EnsPGene gene,
    const AjPStr code,
    AjPList attributes);

AjBool ensAttributeadaptorFetchAllbySeqregion(
    EnsPAttributeadaptor ata,
    const EnsPSeqregion sr,
    const AjPStr code,
    AjPList attributes);

AjBool ensAttributeadaptorFetchAllbySlice(
    EnsPAttributeadaptor ata,
    const EnsPSlice slice,
    const AjPStr code,
    AjPList attributes);

AjBool ensAttributeadaptorFetchAllbyTranscript(
    EnsPAttributeadaptor ata,
    const EnsPTranscript transcript,
    const AjPStr code,
    AjPList attributes);

AjBool ensAttributeadaptorFetchAllbyTranslation(
    EnsPAttributeadaptor ata,
    const EnsPTranslation translation,
    const AjPStr code,
    AjPList attributes);

/* Ensembl Attribute Type */

EnsPAttributetype ensAttributetypeNewCpy(const EnsPAttributetype at);

EnsPAttributetype ensAttributetypeNewIni(EnsPAttributetypeadaptor ata,
                                         ajuint identifier,
                                         AjPStr code,
                                         AjPStr name,
                                         AjPStr description);

EnsPAttributetype ensAttributetypeNewRef(EnsPAttributetype at);

void ensAttributetypeDel(EnsPAttributetype* Pat);

EnsPAttributetypeadaptor ensAttributetypeGetAdaptor(
    const EnsPAttributetype at);

AjPStr ensAttributetypeGetCode(
    const EnsPAttributetype at);

AjPStr ensAttributetypeGetDescription(
    const EnsPAttributetype at);

ajuint ensAttributetypeGetIdentifier(
    const EnsPAttributetype at);

AjPStr ensAttributetypeGetName(
    const EnsPAttributetype at);

AjBool ensAttributetypeSetAdaptor(EnsPAttributetype at,
                                  EnsPAttributetypeadaptor ata);

AjBool ensAttributetypeSetCode(EnsPAttributetype at,
                               AjPStr code);

AjBool ensAttributetypeSetDescription(EnsPAttributetype at,
                                      AjPStr description);

AjBool ensAttributetypeSetIdentifier(EnsPAttributetype at,
                                     ajuint identifier);

AjBool ensAttributetypeSetName(EnsPAttributetype at,
                               AjPStr name);

AjBool ensAttributetypeTrace(const EnsPAttributetype at, ajuint level);

size_t ensAttributetypeCalculateMemsize(const EnsPAttributetype at);

/* Ensembl Attribute Type Adaptor */

EnsPAttributetypeadaptor ensRegistryGetAttributetypeadaptor(
    EnsPDatabaseadaptor dba);

EnsPAttributetypeadaptor ensAttributetypeadaptorNew(
    EnsPDatabaseadaptor dba);

AjBool ensAttributetypeadaptorCacheClear(EnsPAttributetypeadaptor ata);

void ensAttributetypeadaptorDel(EnsPAttributetypeadaptor* Pata);

EnsPBaseadaptor ensAttributetypeadaptorGetBaseadaptor(
    EnsPAttributetypeadaptor ata);

EnsPDatabaseadaptor ensAttributetypeadaptorGetDatabaseadaptor(
    EnsPAttributetypeadaptor ata);

AjBool ensAttributetypeadaptorFetchAll(EnsPAttributetypeadaptor ata,
                                       AjPList ats);

AjBool ensAttributetypeadaptorFetchByCode(EnsPAttributetypeadaptor ata,
                                          const AjPStr code,
                                          EnsPAttributetype* Pat);

AjBool ensAttributetypeadaptorFetchByIdentifier(EnsPAttributetypeadaptor ata,
                                                ajuint identifier,
                                                EnsPAttributetype* Pat);

/*
** End of prototype definitions
*/




AJ_END_DECLS

#endif /* !ENSATTRIBUTE_H */
