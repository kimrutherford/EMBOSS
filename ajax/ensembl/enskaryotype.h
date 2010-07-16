#ifdef __cplusplus
extern "C"
{
#endif

#ifndef enskaryotype_h
#define enskaryotype_h

#include "ensfeature.h"




/* @data EnsPKaryotypebandadaptor *********************************************
**
** Ensembl Karyotype Band Adaptor
**
** @alias EnsSKaryotypebandadaptor
** @alias EnsOKaryotypebandadaptor
**
** @cc Bio::EnsEMBL::BaseFeatureadaptor
** @attr Adaptor [EnsPFeatureadaptor] Ensembl Feature Adaptor
** @@
******************************************************************************/

typedef struct EnsSKaryotypebandadaptor
{
    EnsPFeatureadaptor Adaptor;
} EnsOKaryotypebandadaptor;

#define EnsPKaryotypebandadaptor EnsOKaryotypebandadaptor*




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




/*
** Prototype definitions
*/

/* Ensembl Karyotype Band */

EnsPKaryotypeband ensKaryotypebandNew(EnsPKaryotypebandadaptor kba,
                                      ajuint identifier,
                                      EnsPFeature feature,
                                      AjPStr name,
                                      AjPStr stain);

EnsPKaryotypeband ensKaryotypebandNewObj(const EnsPKaryotypeband object);

EnsPKaryotypeband ensKaryotypebandNewRef(EnsPKaryotypeband kb);

void ensKaryotypebandDel(EnsPKaryotypeband* Pkb);

EnsPKaryotypebandadaptor ensKaryotypebandGetAdaptor(const EnsPKaryotypeband kb);

ajuint ensKaryotypebandGetIdentifier(const EnsPKaryotypeband kb);

EnsPFeature ensKaryotypebandGetFeature(const EnsPKaryotypeband kb);

AjPStr ensKaryotypebandGetName(const EnsPKaryotypeband kb);

AjPStr ensKaryotypebandGetStain(const EnsPKaryotypeband kb);

AjBool ensKaryotypebandSetAdaptor(EnsPKaryotypeband kb,
                                  EnsPKaryotypebandadaptor kba);

AjBool ensKaryotypebandSetIdentifier(EnsPKaryotypeband kb, ajuint identifier);

AjBool ensKaryotypebandSetFeature(EnsPKaryotypeband kb, EnsPFeature feature);

AjBool ensKaryotypebandSetName(EnsPKaryotypeband kb, AjPStr name);

AjBool ensKaryotypebandSetStain(EnsPKaryotypeband kb, AjPStr stain);

AjBool ensKaryotypebandTrace(const EnsPKaryotypeband kb, ajuint level);

ajulong ensKaryotypebandGetMemsize(const EnsPKaryotypeband kb);

AjBool ensKaryotypebandSortByStartAscending(AjPList kbs);

AjBool ensKaryotypebandSortByStartDescending(AjPList kbs);

/* Ensembl Karyoptype Band Adaptor */

EnsPKaryotypebandadaptor ensRegistryGetKaryotypebandadaptor(
    EnsPDatabaseadaptor dba);

EnsPKaryotypebandadaptor ensKaryotypebandadaptorNew(
    EnsPDatabaseadaptor dba);

void ensKaryotypebandadaptorDel(EnsPKaryotypebandadaptor* Padaptor);

AjBool ensKaryotypebandadaptorFetchAllByChromosomeName(
    EnsPKaryotypebandadaptor adaptor,
    const AjPStr name,
    AjPList kblist);

AjBool ensKaryotypebandadaptorFetchAllByChromosomeBand(
    EnsPKaryotypebandadaptor adaptor,
    const AjPStr name,
    const AjPStr band,
    AjPList kblist);

/*
** End of prototype definitions
*/




#endif /* enskaryotype_h */

#ifdef __cplusplus
}
#endif
