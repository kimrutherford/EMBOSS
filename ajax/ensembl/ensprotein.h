
#ifndef ENSPROTEIN_H
#define ENSPROTEIN_H

/* ==================================================================== */
/* ========================== include files =========================== */
/* ==================================================================== */

#include "ensfeature.h"

AJ_BEGIN_DECLS




/* ==================================================================== */
/* ============================ constants ============================= */
/* ==================================================================== */

/* @const EnsPProteinfeatureadaptor *******************************************
**
** Ensembl Protein Feature Adaptor.
** Defined as an alias in EnsPBaseadaptor.
**
** #alias EnsPBaseadaptor
**
** #cc Bio::EnsEMBL::DBSQL::ProteinFeatureAdaptor
** ##
******************************************************************************/

#define EnsPProteinfeatureadaptor EnsPBaseadaptor




/* ==================================================================== */
/* ========================== public data ============================= */
/* ==================================================================== */

/* @data EnsPProteinfeature ***************************************************
**
** Ensembl Protein Feature.
**
** @alias EnsSProteinfeature
** @alias EnsOProteinfeature
**
** @attr Use [ajuint] Use counter
** @cc Bio::EnsEMBL::Storable
** @attr Identifier [ajuint] SQL database-internal identifier
** @attr Adaptor [EnsPProteinfeatureadaptor] Ensembl Protein Feature Adaptor
** @cc Bio::EnsEMBL::Featurepair
** @attr Featurepair [EnsPFeaturepair] Ensembl Feature Pair
** @cc Bio::EnsEMBL::Proteinfeature
** @attr Accession [AjPStr] (InterPro) Accession
** @attr Description [AjPStr] (InterPro) Description
** @@
******************************************************************************/

typedef struct EnsSProteinfeature
{
    ajuint Use;
    ajuint Identifier;
    EnsPProteinfeatureadaptor Adaptor;
    EnsPFeaturepair Featurepair;
    AjPStr Accession;
    AjPStr Description;
} EnsOProteinfeature;

#define EnsPProteinfeature EnsOProteinfeature*




/* ==================================================================== */
/* ======================= public functions =========================== */
/* ==================================================================== */

/*
** Prototype definitions
*/

/* Ensembl Protein Feature */

EnsPProteinfeature ensProteinfeatureNewCpy(const EnsPProteinfeature object);

EnsPProteinfeature ensProteinfeatureNewIni(
    EnsPProteinfeatureadaptor pfa,
    ajuint identifier,
    EnsPFeaturepair fp,
    AjPStr accession,
    AjPStr description);

EnsPProteinfeature ensProteinfeatureNewRef(EnsPProteinfeature pf);

void ensProteinfeatureDel(EnsPProteinfeature* Ppf);

AjPStr ensProteinfeatureGetAccession(
    const EnsPProteinfeature pf);

EnsPProteinfeatureadaptor ensProteinfeatureGetAdaptor(
    const EnsPProteinfeature pf);

AjPStr ensProteinfeatureGetDescription(
    const EnsPProteinfeature pf);

EnsPFeaturepair ensProteinfeatureGetFeaturepair(
    const EnsPProteinfeature pf);

ajuint ensProteinfeatureGetIdentifier(
    const EnsPProteinfeature pf);

AjBool ensProteinfeatureSetAccession(EnsPProteinfeature pf,
                                     AjPStr accession);

AjBool ensProteinfeatureSetAdaptor(EnsPProteinfeature pf,
                                   EnsPProteinfeatureadaptor pfa);

AjBool ensProteinfeatureSetDescription(EnsPProteinfeature pf,
                                       AjPStr description);

AjBool ensProteinfeatureSetFeaturepair(EnsPProteinfeature pf,
                                       EnsPFeaturepair fp);

AjBool ensProteinfeatureSetIdentifier(EnsPProteinfeature pf,
                                      ajuint identifier);

AjBool ensProteinfeatureTrace(const EnsPProteinfeature pf, ajuint level);

size_t ensProteinfeatureCalculateMemsize(const EnsPProteinfeature pf);

/* Ensembl Protein Feature Adaptor */

EnsPProteinfeatureadaptor ensRegistryGetProteinfeatureadaptor(
    EnsPDatabaseadaptor dba);

EnsPProteinfeatureadaptor ensProteinfeatureadaptorNew(
    EnsPDatabaseadaptor dba);

void ensProteinfeatureadaptorDel(EnsPProteinfeatureadaptor* Ppfa);

EnsPDatabaseadaptor ensProteinfeatureadaptorGetDatabaseadaptor(
    EnsPProteinfeatureadaptor pfa);

AjBool ensProteinfeatureadaptorFetchAllbyTranslationidentifier(
    EnsPProteinfeatureadaptor pfa,
    ajuint tlid,
    AjPList pfs);

AjBool ensProteinfeatureadaptorFetchByIdentifier(
    EnsPProteinfeatureadaptor pfa,
    ajuint identifier,
    EnsPProteinfeature* Ppf);

/*
** End of prototype definitions
*/




AJ_END_DECLS

#endif /* !ENSPROTEIN_H */
