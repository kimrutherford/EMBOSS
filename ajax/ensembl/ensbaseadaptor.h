
#ifndef ENSBASEADAPTOR_H
#define ENSBASEADAPTOR_H

/* ==================================================================== */
/* ========================== include files =========================== */
/* ==================================================================== */

#include "ensassemblymapper.h"
#include "ensdata.h"

AJ_BEGIN_DECLS




/* ==================================================================== */
/* ============================ constants ============================= */
/* ==================================================================== */

extern const ajuint ensBaseadaptorMaximumIdentifiers;




/* ==================================================================== */
/* ========================== public data ============================= */
/* ==================================================================== */




/* ==================================================================== */
/* ======================= public functions =========================== */
/* ==================================================================== */

/*
** Prototype definitions
*/

/* Ensembl Base Adaptor */

EnsPBaseadaptor ensBaseadaptorNew(
    EnsPDatabaseadaptor dba,
    const char* const* Ptables,
    const char* const* Pcolumns,
    const EnsPBaseadaptorLeftjoin leftjoin,
    const char* condition,
    const char* final,
    AjBool Fstatement(EnsPDatabaseadaptor dba,
                      const AjPStr statement,
                      EnsPAssemblymapper am,
                      EnsPSlice slice,
                      AjPList objects));

void ensBaseadaptorDel(EnsPBaseadaptor* Pba);

const char* const* ensBaseadaptorGetColumns(const EnsPBaseadaptor ba);

EnsPDatabaseadaptor ensBaseadaptorGetDatabaseadaptor(const EnsPBaseadaptor ba);

const char* const* ensBaseadaptorGetTables(const EnsPBaseadaptor ba);

AjBool ensBaseadaptorSetColumns(EnsPBaseadaptor ba,
                                const char* const* Pcolumns);

AjBool ensBaseadaptorSetDefaultcondition(EnsPBaseadaptor ba,
                                         const char* condition);

AjBool ensBaseadaptorSetFinalcondition(EnsPBaseadaptor ba, const char* final);

AjBool ensBaseadaptorSetTables(EnsPBaseadaptor ba,
                               const char* const* Ptables);

AjBool ensBaseadaptorEscapeC(EnsPBaseadaptor ba, char** Ptxt,
                             const AjPStr str);

AjBool ensBaseadaptorEscapeS(EnsPBaseadaptor ba, AjPStr* Pstr,
                             const AjPStr str);

AjBool ensBaseadaptorGetMultispecies(const EnsPBaseadaptor ba);

const char* ensBaseadaptorGetPrimarytable(const EnsPBaseadaptor ba);

ajuint ensBaseadaptorGetSpeciesidentifier(const EnsPBaseadaptor ba);

AjBool ensBaseadaptorFetchAll(EnsPBaseadaptor ba,
                              AjPList objects);

AjBool ensBaseadaptorFetchAllbyConstraint(EnsPBaseadaptor ba,
                                          const AjPStr constraint,
                                          EnsPAssemblymapper am,
                                          EnsPSlice slice,
                                          AjPList objects);

AjBool ensBaseadaptorFetchAllbyIdentifiers(EnsPBaseadaptor ba,
                                           const AjPList identifiers,
                                           EnsPSlice slice,
                                           AjPList objects);

AjBool ensBaseadaptorFetchByIdentifier(EnsPBaseadaptor ba,
                                       ajuint identifier,
                                       void** Pobject);

AjBool ensBaseadaptorRetrieveAllIdentifiers(EnsPBaseadaptor ba,
                                            const AjPStr table,
                                            const AjPStr primary,
                                            AjPList identifiers);

AjBool ensBaseadaptorRetrieveAllStrings(EnsPBaseadaptor ba,
                                        const AjPStr table,
                                        const AjPStr primary,
                                        AjPList strings);

/*
** End of prototype definitions
*/




AJ_END_DECLS

#endif /* !ENSBASEADAPTOR_H */
