#ifdef __cplusplus
extern "C"
{
#endif

#ifndef ensrepeat_h
#define ensrepeat_h

#include "ensfeature.h"
#include "enstable.h"




/******************************************************************************
**
** Ensembl Repeat Mask Type enumeration.
**
******************************************************************************/

enum EnsERepeatMaskType
{
    ensERepeatMaskTypeNULL,
    ensERepeatMaskTypeNone,
    ensERepeatMaskTypeSoft,
    ensERepeatMaskTypeHard
};




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
** @attr Adaptor [EnsPDatabaseadaptor] Ensembl Database Adaptor
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
    EnsPDatabaseadaptor Adaptor;
    AjPStr Name;
    AjPStr Class;
    AjPStr Type;
    AjPStr Consensus;
    ajuint Length;
    ajuint Padding;
} EnsORepeatconsensus;

#define EnsPRepeatconsensus EnsORepeatconsensus*




/* @data EnsPRepeatfeatureadaptor *********************************************
**
** Ensembl Repeat Feature Adaptor.
**
** @alias EnsSRepeatfeatureadaptor
** @alias EnsORepeatfeatureadaptor
**
** @cc Bio::EnsEMBL::DBSQL::BaseFeatureadaptor
** @attr Adaptor [EnsPFeatureadaptor] Ensembl Feature Adaptor
** @@
******************************************************************************/

typedef struct EnsSRepeatfeatureadaptor
{
    EnsPFeatureadaptor Adaptor;
} EnsORepeatfeatureadaptor;

#define EnsPRepeatfeatureadaptor EnsORepeatfeatureadaptor*




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




/*
** Prototype definitions
*/

/* Ensembl Repeat Consensus */

EnsPRepeatconsensus ensRepeatconsensusNew(EnsPDatabaseadaptor adaptor,
                                          ajuint identifier,
                                          AjPStr name,
                                          AjPStr class,
                                          AjPStr type,
                                          AjPStr consensus,
                                          ajuint length);

EnsPRepeatconsensus ensRepeatconsensusNewObj(EnsPRepeatconsensus object);

EnsPRepeatconsensus ensRepeatconsensusNewRef(EnsPRepeatconsensus rc);

void ensRepeatconsensusDel(EnsPRepeatconsensus* Prc);

EnsPDatabaseadaptor ensRepeatconsensusGetAdaptor(const EnsPRepeatconsensus rc);

ajuint ensRepeatconsensusGetIdentifier(const EnsPRepeatconsensus rc);

AjPStr ensRepeatconsensusGetName(const EnsPRepeatconsensus rc);

AjPStr ensRepeatconsensusGetClass(const EnsPRepeatconsensus rc);

AjPStr ensRepeatconsensusGetType(const EnsPRepeatconsensus rc);

AjPStr ensRepeatconsensusGetConsensus(const EnsPRepeatconsensus rc);

ajuint ensRepeatconsensusGetLength(const EnsPRepeatconsensus rc);

AjBool ensRepeatconsensusSetAdaptor(EnsPRepeatconsensus rc,
                                    EnsPDatabaseadaptor dba);

AjBool ensRepeatconsensusSetIdentifier(EnsPRepeatconsensus rc,
                                       ajuint identifier);

AjBool ensRepeatconsensusSetName(EnsPRepeatconsensus rc, AjPStr name);

AjBool ensRepeatconsensusSetClass(EnsPRepeatconsensus rc, AjPStr class);

AjBool ensRepeatconsensusSetType(EnsPRepeatconsensus rc, AjPStr type);

AjBool ensRepeatconsensusSetConsensus(EnsPRepeatconsensus rc, AjPStr consensus);

AjBool ensRepeatconsensusSetLength(EnsPRepeatconsensus rc, ajuint length);

ajuint ensRepeatconsensusGetMemSize(const EnsPRepeatconsensus rc);

AjBool ensRepeatconsensusTrace(const EnsPRepeatconsensus rc, ajuint level);

/* Ensembl Repeat Consensus Adaptor */

AjBool ensRepeatconsensusadaptorFetchByIdentifier(EnsPDatabaseadaptor dba,
                                                  ajuint identifier,
                                                  EnsPRepeatconsensus *Prc);

AjBool ensRepeatconsensusadaptorFetchByName(EnsPDatabaseadaptor dba,
                                            const AjPStr name,
                                            EnsPRepeatconsensus *Prc);

AjBool ensRepeatconsensusadaptorFetchByNameClass(EnsPDatabaseadaptor dba,
                                                 const AjPStr name,
                                                 const AjPStr class,
                                                 EnsPRepeatconsensus *Prc);

AjBool ensRepeatconsensusadaptorFetchAllByClassConsensus(
    EnsPDatabaseadaptor dba,
    const AjPStr class,
    const AjPStr consensus,
    AjPList rci);

/* Ensembl Repeat Feature */

EnsPRepeatfeature ensRepeatfeatureNew(EnsPRepeatfeatureadaptor adaptor,
                                      ajuint identifier,
                                      EnsPFeature feature,
                                      EnsPRepeatconsensus rc,
                                      ajint hstart,
                                      ajint hend,
                                      double score);

EnsPRepeatfeature ensRepeatfeatureNewObj(EnsPRepeatfeature object);

EnsPRepeatfeature ensRepeatfeatureNewRef(EnsPRepeatfeature rf);

void ensRepeatfeatureDel(EnsPRepeatfeature* Prf);

const EnsPRepeatfeatureadaptor ensRepeatfeatureGetAdaptor(
    const EnsPRepeatfeature rf);

ajuint ensRepeatfeatureGetIdentifier(const EnsPRepeatfeature rf);

EnsPFeature ensRepeatfeatureGetFeature(const EnsPRepeatfeature rf);

EnsPRepeatconsensus ensRepeatfeatureGetRepeatconsensus(
    const EnsPRepeatfeature rf);

ajint ensRepeatfeatureGetHitStart(const EnsPRepeatfeature rf);

ajint ensRepeatfeatureGetHitEnd(const EnsPRepeatfeature rf);

ajint ensRepeatfeatureGetHitStrand(const EnsPRepeatfeature rf);

double ensRepeatfeatureGetScore(const EnsPRepeatfeature rf);

AjBool ensRepeatfeatureSetAdaptor(EnsPRepeatfeature rf,
                                  EnsPRepeatfeatureadaptor rfa);

AjBool ensRepeatfeatureSetIdentifier(EnsPRepeatfeature rf, ajuint identifier);

AjBool ensRepeatfeatureSetFeature(EnsPRepeatfeature rf, EnsPFeature feature);

AjBool ensRepeatfeatureSetRepeatconsensus(EnsPRepeatfeature rf,
                                          EnsPRepeatconsensus rc);

AjBool ensRepeatfeatureSetHitStart(EnsPRepeatfeature rf, ajuint hstart);

AjBool ensRepeatfeatureSetHitEnd(EnsPRepeatfeature rf, ajuint hend);

AjBool ensRepeatfeatureSetScore(EnsPRepeatfeature rf, double score);

AjBool ensRepeatfeatureTrace(const EnsPRepeatfeature rf, ajuint level);

ajuint ensRepeatfeatureGetMemSize(const EnsPRepeatfeature rf);

/* Ensembl Repeat Feature Adaptor */

EnsPRepeatfeatureadaptor ensRepeatfeatureadaptorNew(EnsPDatabaseadaptor dba);

void ensRepeatfeatureadaptorDel(EnsPRepeatfeatureadaptor *Prfa);

AjBool ensRepeatfeatureadaptorFetchAllBySlice(
    const EnsPRepeatfeatureadaptor rfa,
    EnsPSlice slice,
    const AjPStr anname,
    const AjPStr rctype,
    const AjPStr rcclass,
    const AjPStr rcname,
    AjPList rfs);

/*
** End of prototype definitions
*/




#endif

#ifdef __cplusplus
}
#endif
