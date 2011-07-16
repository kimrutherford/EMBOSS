
#ifndef ENSREPEAT_H
#define ENSREPEAT_H

/* ==================================================================== */
/* ========================== include files =========================== */
/* ==================================================================== */

#include "ensfeature.h"

AJ_BEGIN_DECLS




/* ==================================================================== */
/* ============================ constants ============================= */
/* ==================================================================== */

/* @const EnsPRepeatconsensusadaptor ******************************************
**
** Ensembl Repeat Consensus Adaptor.
** Defined as an alias in EnsPBaseadaptor
** #alias EnsPBaseadaptor
** ##
******************************************************************************/

#define EnsPRepeatconsensusadaptor EnsPBaseadaptor




/* @const EnsPRepeatfeatureadaptor ********************************************
**
** Ensembl Repeat Feature Adaptor.
** Defined as an alias in EnsPFeatureadaptor
** #alias EnsPFeatureadaptor
** ##
******************************************************************************/

#define EnsPRepeatfeatureadaptor EnsPFeatureadaptor




/* ==================================================================== */
/* ========================== public data ============================= */
/* ==================================================================== */

/* @data EnsPRepeatconsensus **************************************************
**
** Ensembl Repeat Consensus.
**
** @alias EnsSRepeatconsensus
** @alias EnsORepeatconsensus
**
** @attr Use [ajuint] Use counter
** @cc Bio::EnsEMBL::Storable
** @attr Identifier [ajuint] The SQL database-internal identifier
** @attr Adaptor [EnsPRepeatconsensusadaptor] Ensembl Repeat Consensus Adaptor
** @cc Bio::EnsEMBL::Repeatconsensus
** @attr Name [AjPStr] Repeat Consensus name
** @attr Class [AjPStr] Repeat Consensus class
** @attr Type [AjPStr] Repeat Consensus type
** @attr Consensus [AjPStr] Repeat Consensus sequence
** @attr Length [ajuint] Repeat Consensus sequence length
** @attr Padding [ajuint] Padding to alignment boundary
** @@
******************************************************************************/

typedef struct EnsSRepeatconsensus
{
    ajuint Use;
    ajuint Identifier;
    EnsPRepeatconsensusadaptor Adaptor;
    AjPStr Name;
    AjPStr Class;
    AjPStr Type;
    AjPStr Consensus;
    ajuint Length;
    ajuint Padding;
} EnsORepeatconsensus;

#define EnsPRepeatconsensus EnsORepeatconsensus*




/* @data EnsPRepeatfeature ****************************************************
**
** Ensembl Repeat Feature.
**
** @alias EnsSRepeatfeature
** @alias EnsORepeatfeature
**
** @attr Use [ajuint] Use counter
** @cc Bio::EnsEMBL::Storable
** @attr Identifier [ajuint] The SQL database-internal identifier
** @attr Adaptor [EnsPRepeatfeatureadaptor] Ensembl Repeat Feature Adaptor
** @cc Bio::EnsEMBL::Repeatfeature
** @attr Feature [EnsPFeature] Ensembl Feature
** @attr Repeatconsensus [EnsPRepeatconsensus] Ensembl Repeat Consensus
** @attr HitStart [ajint] The hit start on he consensus sequence
** @attr HitEnd [ajint] The hit end on the consensus sequence
** @attr Score [double] Score
** @@
******************************************************************************/

typedef struct EnsSRepeatfeature
{
    ajuint Use;
    ajuint Identifier;
    EnsPRepeatfeatureadaptor Adaptor;
    EnsPFeature Feature;
    EnsPRepeatconsensus Repeatconsensus;
    ajint HitStart;
    ajint HitEnd;
    double Score;
} EnsORepeatfeature;

#define EnsPRepeatfeature EnsORepeatfeature*




/* ==================================================================== */
/* ======================= public functions =========================== */
/* ==================================================================== */

/*
** Prototype definitions
*/

/* Ensembl Repeat Consensus */

EnsPRepeatconsensus ensRepeatconsensusNewCpy(const EnsPRepeatconsensus rc);

EnsPRepeatconsensus ensRepeatconsensusNewIni(EnsPRepeatconsensusadaptor rca,
                                             ajuint identifier,
                                             AjPStr name,
                                             AjPStr class,
                                             AjPStr type,
                                             AjPStr consensus,
                                             ajuint length);

EnsPRepeatconsensus ensRepeatconsensusNewRef(EnsPRepeatconsensus rc);

void ensRepeatconsensusDel(EnsPRepeatconsensus* Prc);

EnsPRepeatconsensusadaptor ensRepeatconsensusGetAdaptor(
    const EnsPRepeatconsensus rc);

AjPStr ensRepeatconsensusGetClass(const EnsPRepeatconsensus rc);

AjPStr ensRepeatconsensusGetConsensus(const EnsPRepeatconsensus rc);

ajuint ensRepeatconsensusGetIdentifier(const EnsPRepeatconsensus rc);

ajuint ensRepeatconsensusGetLength(const EnsPRepeatconsensus rc);

AjPStr ensRepeatconsensusGetName(const EnsPRepeatconsensus rc);

AjPStr ensRepeatconsensusGetType(const EnsPRepeatconsensus rc);

AjBool ensRepeatconsensusSetAdaptor(EnsPRepeatconsensus rc,
                                    EnsPRepeatconsensusadaptor rca);

AjBool ensRepeatconsensusSetClass(EnsPRepeatconsensus rc,
                                  AjPStr class);

AjBool ensRepeatconsensusSetConsensus(EnsPRepeatconsensus rc,
                                      AjPStr consensus);

AjBool ensRepeatconsensusSetIdentifier(EnsPRepeatconsensus rc,
                                       ajuint identifier);

AjBool ensRepeatconsensusSetLength(EnsPRepeatconsensus rc,
                                   ajuint length);

AjBool ensRepeatconsensusSetName(EnsPRepeatconsensus rc,
                                 AjPStr name);

AjBool ensRepeatconsensusSetType(EnsPRepeatconsensus rc,
                                 AjPStr type);

size_t ensRepeatconsensusCalculateMemsize(const EnsPRepeatconsensus rc);

AjBool ensRepeatconsensusTrace(const EnsPRepeatconsensus rc, ajuint level);

AjBool ensTableRepeatconsensusClear(AjPTable table);

AjBool ensTableRepeatconsensusDelete(AjPTable* Ptable);

/* Ensembl Repeat Consensus Adaptor */

EnsPRepeatconsensusadaptor ensRegistryGetRepeatconsensusadaptor(
    EnsPDatabaseadaptor dba);

EnsPRepeatconsensusadaptor ensRepeatconsensusadaptorNew(
    EnsPDatabaseadaptor dba);

void ensRepeatconsensusadaptorDel(EnsPRepeatconsensusadaptor* Prca);

EnsPDatabaseadaptor ensRepeatconsensusadaptorGetDatabaseadaptor(
    EnsPRepeatconsensusadaptor rca);

AjBool ensRepeatconsensusadaptorFetchByIdentifier(
    EnsPRepeatconsensusadaptor rca,
    ajuint identifier,
    EnsPRepeatconsensus* Prc);

AjBool ensRepeatconsensusadaptorFetchByName(
    EnsPRepeatconsensusadaptor rca,
    const AjPStr name,
    const AjPStr class,
    EnsPRepeatconsensus* Prc);

AjBool ensRepeatconsensusadaptorFetchAllbyClassconsensus(
    EnsPRepeatconsensusadaptor rca,
    const AjPStr class,
    const AjPStr consensus,
    AjPList rci);

AjBool ensRepeatconsensusadaptorFetchAllbyIdentifiers(
    EnsPRepeatconsensusadaptor rca,
    AjPTable rcit);

/* Ensembl Repeat Feature */

EnsPRepeatfeature ensRepeatfeatureNewCpy(const EnsPRepeatfeature rf);

EnsPRepeatfeature ensRepeatfeatureNewIni(EnsPRepeatfeatureadaptor rfa,
                                         ajuint identifier,
                                         EnsPFeature feature,
                                         EnsPRepeatconsensus rc,
                                         ajint hstart,
                                         ajint hend,
                                         double score);

EnsPRepeatfeature ensRepeatfeatureNewRef(EnsPRepeatfeature rf);

void ensRepeatfeatureDel(EnsPRepeatfeature* Prf);

EnsPRepeatfeatureadaptor ensRepeatfeatureGetAdaptor(
    const EnsPRepeatfeature rf);

EnsPFeature ensRepeatfeatureGetFeature(
    const EnsPRepeatfeature rf);

ajint ensRepeatfeatureGetHitEnd(
    const EnsPRepeatfeature rf);

ajint ensRepeatfeatureGetHitStart(
    const EnsPRepeatfeature rf);

ajint ensRepeatfeatureGetHitStrand(
    const EnsPRepeatfeature rf);

ajuint ensRepeatfeatureGetIdentifier(
    const EnsPRepeatfeature rf);

EnsPRepeatconsensus ensRepeatfeatureGetRepeatconsensus(
    const EnsPRepeatfeature rf);

double ensRepeatfeatureGetScore(
    const EnsPRepeatfeature rf);

AjBool ensRepeatfeatureSetAdaptor(EnsPRepeatfeature rf,
                                  EnsPRepeatfeatureadaptor rfa);

AjBool ensRepeatfeatureSetFeature(EnsPRepeatfeature rf,
                                  EnsPFeature feature);

AjBool ensRepeatfeatureSetHitEnd(EnsPRepeatfeature rf,
                                 ajuint hend);

AjBool ensRepeatfeatureSetHitStart(EnsPRepeatfeature rf,
                                   ajuint hstart);

AjBool ensRepeatfeatureSetIdentifier(EnsPRepeatfeature rf,
                                     ajuint identifier);

AjBool ensRepeatfeatureSetRepeatconsensus(EnsPRepeatfeature rf,
                                          EnsPRepeatconsensus rc);

AjBool ensRepeatfeatureSetScore(EnsPRepeatfeature rf,
                                double score);

AjBool ensRepeatfeatureTrace(const EnsPRepeatfeature rf, ajuint level);

size_t ensRepeatfeatureCalculateMemsize(const EnsPRepeatfeature rf);

AjBool ensListRepeatfeatureSortStartAscending(AjPList rfs);

AjBool ensListRepeatfeatureSortStartDescending(AjPList rfs);

/* Ensembl Repeat Feature Adaptor */

EnsPRepeatfeatureadaptor ensRegistryGetRepeatfeatureadaptor(
    EnsPDatabaseadaptor dba);

EnsPRepeatfeatureadaptor ensRepeatfeatureadaptorNew(
    EnsPDatabaseadaptor dba);

void ensRepeatfeatureadaptorDel(EnsPRepeatfeatureadaptor* Prfa);

EnsPDatabaseadaptor ensRepeatfeatureadaptorGetDatabaseadaptor(
    EnsPRepeatfeatureadaptor rfa);

AjBool ensRepeatfeatureadaptorFetchAllbySlice(
    EnsPRepeatfeatureadaptor rfa,
    EnsPSlice slice,
    const AjPStr anname,
    const AjPStr rctype,
    const AjPStr rcclass,
    const AjPStr rcname,
    AjPList rfs);

/*
** End of prototype definitions
*/




AJ_END_DECLS

#endif /* !ENSREPEAT_H */
