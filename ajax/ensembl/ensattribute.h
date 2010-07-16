#ifdef __cplusplus
extern "C"
{
#endif

#ifndef ensattribute_h
#define ensattribute_h

#include "ensdata.h"




/*
** Prototype definitions
*/

/* Ensembl Attribute */

EnsPAttribute ensAttributeNew(AjPStr code,
                              AjPStr name,
                              AjPStr description,
                              AjPStr value);

EnsPAttribute ensAttributeNewObj(const EnsPAttribute object);

EnsPAttribute ensAttributeNewRef(EnsPAttribute attribute);

void ensAttributeDel(EnsPAttribute* Pattribute);

AjPStr ensAttributeGetCode(const EnsPAttribute attribute);

AjPStr ensAttributeGetName(const EnsPAttribute attribute);

AjPStr ensAttributeGetDescription(const EnsPAttribute attribute);

AjPStr ensAttributeGetValue(const EnsPAttribute attribute);

ajulong ensAttributeGetMemsize(const EnsPAttribute attribute);

AjBool ensAttributeTrace(const EnsPAttribute attribute, ajuint level);

/* Ensembl Attribute Adaptor */

EnsPAttributeadaptor ensRegistryGetAttributeadaptor(
    EnsPDatabaseadaptor dba);

EnsPDatabaseadaptor ensAttributeadaptorGetDatabaseadaptor(
    EnsPAttributeadaptor ata);

AjBool ensAttributeadaptorFetchAllByGene(EnsPAttributeadaptor ata,
                                         const EnsPGene gene,
                                         const AjPStr code,
                                         AjPList attributes);

AjBool ensAttributeadaptorFetchAllBySeqregion(EnsPAttributeadaptor ata,
                                              const EnsPSeqregion sr,
                                              const AjPStr code,
                                              AjPList attributes);

AjBool ensAttributeadaptorFetchAllBySlice(EnsPAttributeadaptor ata,
                                          const EnsPSlice slice,
                                          const AjPStr code,
                                          AjPList attributes);

AjBool ensAttributeadaptorFetchAllByTranscript(EnsPAttributeadaptor ata,
                                               const EnsPTranscript transcript,
                                               const AjPStr code,
                                               AjPList attributes);

AjBool ensAttributeadaptorFetchAllByTranslation(
    EnsPAttributeadaptor ata,
    const EnsPTranslation translation,
    const AjPStr code,
    AjPList attributes);

/*
** End of prototype definitions
*/




#endif /* ensattribute_h */

#ifdef __cplusplus
}
#endif
