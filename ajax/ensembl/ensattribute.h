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

EnsPAttribute ensAttributeNew(AjPStr code, AjPStr name, AjPStr description,
                              AjPStr value);

EnsPAttribute ensAttributeNewObj(const EnsPAttribute object);

EnsPAttribute ensAttributeNewRef(EnsPAttribute attribute);

void ensAttributeDel(EnsPAttribute* Pattribute);

AjPStr ensAttributeGetCode(const EnsPAttribute attribute);

AjPStr ensAttributeGetName(const EnsPAttribute attribute);

AjPStr ensAttributeGetDescription(const EnsPAttribute attribute);

AjPStr ensAttributeGetValue(const EnsPAttribute attribute);

ajuint ensAttributeGetMemSize(const EnsPAttribute attribute);

AjBool ensAttributeTrace(const EnsPAttribute attribute, ajuint level);

/* Ensembl Attribute Adaptor */

AjBool ensAttributeadaptorFetchAllByGene(EnsPDatabaseadaptor adaptor,
                                         const EnsPGene gene,
                                         const AjPStr code,
                                         AjPList attributes);

AjBool ensAttributeadaptorFetchAllBySeqregion(EnsPDatabaseadaptor adaptor,
                                              const EnsPSeqregion sr,
                                              const AjPStr code,
                                              AjPList attributes);

AjBool ensAttributeadaptorFetchAllBySlice(EnsPDatabaseadaptor adaptor,
                                          const EnsPSlice slice,
                                          const AjPStr code,
                                          AjPList attributes);

AjBool ensAttributeadaptorFetchAllByTranscript(EnsPDatabaseadaptor adaptor,
                                               const EnsPTranscript transcript,
                                               const AjPStr code,
                                               AjPList attributes);

AjBool ensAttributeadaptorFetchAllByTranslation(
    EnsPDatabaseadaptor adaptor,
    const EnsPTranslation translation,
    const AjPStr code,
    AjPList attributes);

/*
** End of prototype definitions
*/




#endif

#ifdef __cplusplus
}
#endif
