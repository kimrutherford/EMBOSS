
#ifndef ENSKARYOTYPE_H
#define ENSKARYOTYPE_H

/* ==================================================================== */
/* ========================== include files =========================== */
/* ==================================================================== */

#include "ensfeature.h"

AJ_BEGIN_DECLS




/* ==================================================================== */
/* ============================ constants ============================= */
/* ==================================================================== */

/* @const EnsPKaryotypebandadaptor ********************************************
**
** Ensembl Karyotype Band Adaptor
** Defined as an alias in EnsPFeatureadaptor
**
** #alias EnsPFeatureadaptor
**
** #cc Bio::EnsEMBL::DBSQL::KaryotypeBandAdaptor
** ##
******************************************************************************/

#define EnsPKaryotypebandadaptor EnsPFeatureadaptor




/* ==================================================================== */
/* ========================== public data ============================= */
/* ==================================================================== */

/* @data EnsPKaryotypeband ****************************************************
**
** Ensembl Karyotype Band
**
** @alias EnsSKaryotypeband
** @alias EnsOKaryotypeband
**
** @attr Use [ajuint] Use counter
** @cc Bio::EnsEMBL::Storable
** @attr Identifier [ajuint] Internal SQL database identifier (primary key)
** @attr Adaptor [EnsPKaryotypebandadaptor] Ensembl Karyotype Band Adaptor
** @cc Bio::EnsEMBL::Karyotypeband
** @attr Feature [EnsPFeature] Ensembl Feature
** @attr Name [AjPStr] Name
** @attr Stain [AjPStr] Stain
** @@
******************************************************************************/

typedef struct EnsSKaryotypeband
{
    ajuint Use;
    ajuint Identifier;
    EnsPKaryotypebandadaptor Adaptor;
    EnsPFeature Feature;
    AjPStr Name;
    AjPStr Stain;
} EnsOKaryotypeband;

#define EnsPKaryotypeband EnsOKaryotypeband*




/* ==================================================================== */
/* ======================= public functions =========================== */
/* ==================================================================== */

/*
** Prototype definitions
*/

/* Ensembl Karyotype Band */

EnsPKaryotypeband ensKaryotypebandNewCpy(const EnsPKaryotypeband kb);

EnsPKaryotypeband ensKaryotypebandNewIni(EnsPKaryotypebandadaptor kba,
                                         ajuint identifier,
                                         EnsPFeature feature,
                                         AjPStr name,
                                         AjPStr stain);

EnsPKaryotypeband ensKaryotypebandNewRef(EnsPKaryotypeband kb);

void ensKaryotypebandDel(EnsPKaryotypeband* Pkb);

EnsPKaryotypebandadaptor ensKaryotypebandGetAdaptor(
    const EnsPKaryotypeband kb);

EnsPFeature ensKaryotypebandGetFeature(
    const EnsPKaryotypeband kb);

ajuint ensKaryotypebandGetIdentifier(
    const EnsPKaryotypeband kb);

AjPStr ensKaryotypebandGetName(
    const EnsPKaryotypeband kb);

AjPStr ensKaryotypebandGetStain(
    const EnsPKaryotypeband kb);

AjBool ensKaryotypebandSetAdaptor(EnsPKaryotypeband kb,
                                  EnsPKaryotypebandadaptor kba);

AjBool ensKaryotypebandSetFeature(EnsPKaryotypeband kb,
                                  EnsPFeature feature);

AjBool ensKaryotypebandSetIdentifier(EnsPKaryotypeband kb,
                                     ajuint identifier);

AjBool ensKaryotypebandSetName(EnsPKaryotypeband kb,
                               AjPStr name);

AjBool ensKaryotypebandSetStain(EnsPKaryotypeband kb,
                                AjPStr stain);

AjBool ensKaryotypebandTrace(const EnsPKaryotypeband kb,
                             ajuint level);

size_t ensKaryotypebandCalculateMemsize(const EnsPKaryotypeband kb);

AjBool ensListKaryotypebandSortStartAscending(AjPList kbs);

AjBool ensListKaryotypebandSortStartDescending(AjPList kbs);

/* Ensembl Karyoptype Band Adaptor */

EnsPKaryotypebandadaptor ensRegistryGetKaryotypebandadaptor(
    EnsPDatabaseadaptor dba);

EnsPKaryotypebandadaptor ensKaryotypebandadaptorNew(
    EnsPDatabaseadaptor dba);

void ensKaryotypebandadaptorDel(EnsPKaryotypebandadaptor* Padaptor);

EnsPDatabaseadaptor ensKaryotypebandadaptorGetDatabaseadaptor(
    EnsPKaryotypebandadaptor kba);

AjBool ensKaryotypebandadaptorFetchAllbyChromosomeband(
    EnsPKaryotypebandadaptor adaptor,
    const AjPStr name,
    const AjPStr band,
    AjPList kblist);

AjBool ensKaryotypebandadaptorFetchAllbyChromosomename(
    EnsPKaryotypebandadaptor adaptor,
    const AjPStr name,
    AjPList kblist);

/*
** End of prototype definitions
*/




AJ_END_DECLS

#endif /* !ENSKARYOTYPE_H */
