#ifdef __cplusplus
extern "C"
{
#endif

#ifndef ensprediction_h
#define ensprediction_h

#include "ensfeature.h"




/******************************************************************************
**
** Ensembl Prediction Exon Adaptor
**
******************************************************************************/

#define EnsPPredictionexonadaptor EnsPFeatureadaptor




/* @data EnsPPredictionexon ***************************************************
**
** Ensembl Prediction Exon.
**
** @alias EnsSPredictionexon
** @alias EnsOPredictionexon
**
** @attr Use [ajuint] Use counter
** @cc Bio::EnsEMBL::Storable
** @attr Identifier [ajuint] Internal SQL database identifier (primary key)
** @attr Adaptor [EnsPPredictionexonadaptor] Ensembl Prediction Exon Adaptor
** @cc Bio::EnsEMBL::Feature
** @attr Feature [EnsPFeature] Ensembl Feature
** @cc Bio::EnsEMBL::Predictionexon
** @cc 'prediction_exon' SQL table
** @attr Score [double] Score
** @attr Pvalue [double] P-value
** @attr StartPhase [ajint] Start phase
** @attr Padding [char[4]] Padding to alignment boundary
** @@
******************************************************************************/

typedef struct EnsSPredictionexon
{
    ajuint Use;
    ajuint Identifier;
    EnsPPredictionexonadaptor Adaptor;
    EnsPFeature Feature;
    double Score;
    double Pvalue;
    ajint StartPhase;
    char Padding[4];
} EnsOPredictionexon;

#define EnsPPredictionexon EnsOPredictionexon*




/******************************************************************************
**
** Ensembl Prediction Transcript Adaptor
**
******************************************************************************/

#define EnsPPredictiontranscriptadaptor EnsPFeatureadaptor




/* @data EnsPPredictiontranscript *********************************************
**
** Ensembl Prediction Transcript.
**
** @alias EnsSPredictiontranscript
** @alias EnsOPredictiontranscript
**
** @attr Use [ajuint] Use counter
** @cc Bio::EnsEMBL::Storable
** @attr Identifier [ajuint] SQL database-internal identifier
** @attr Adaptor [EnsPPredictiontranscriptadaptor] Ensembl Prediction
**                                                 Transcript Adaptor
** @cc Bio::EnsEMBL::Feature
** @attr Feature [EnsPFeature] Ensembl Feature
** @cc Bio::EnsEMBL::Predictiontranscript
** @attr DisplayLabel [AjPStr] Display label
** @attr Predictionexons [AjPList] AJAX List of Ensembl Prediction Exons
** @@
******************************************************************************/

typedef struct EnsSPredictiontranscript
{
    ajuint Use;
    ajuint Identifier;
    EnsPPredictiontranscriptadaptor Adaptor;
    EnsPFeature Feature;
    AjPStr DisplayLabel;
    AjPList Predictionexons;
} EnsOPredictiontranscript;

#define EnsPPredictiontranscript EnsOPredictiontranscript*




/*
** Prototype definitions
*/

/* Ensembl Prediction Exon */

EnsPPredictionexon ensPredictionexonNew(EnsPPredictionexonadaptor adaptor,
                                        ajuint identifier,
                                        EnsPFeature feature,
                                        ajint sphase,
                                        double score,
                                        double pvalue);

EnsPPredictionexon ensPredictionexonNewObj(const EnsPPredictionexon object);

EnsPPredictionexon ensPredictionexonNewRef(EnsPPredictionexon pe);

void ensPredictionexonDel(EnsPPredictionexon* Ppe);

EnsPPredictionexonadaptor ensPredictionexonGetAdaptor(
    const EnsPPredictionexon pe);

ajuint ensPredictionexonGetIdentifier(const EnsPPredictionexon pe);

EnsPFeature ensPredictionexonGetFeature(const EnsPPredictionexon pe);

ajint ensPredictionexonGetStartPhase(const EnsPPredictionexon pe);

double ensPredictionexonGetScore(const EnsPPredictionexon pe);

double ensPredictionexonGetPvalue(const EnsPPredictionexon pe);

AjBool ensPredictionexonSetAdaptor(EnsPPredictionexon pe,
                                   EnsPPredictionexonadaptor adaptor);

AjBool ensPredictionexonSetIdentifier(EnsPPredictionexon pe, ajuint identifier);

AjBool ensPredictionexonSetFeature(EnsPPredictionexon pe, EnsPFeature feature);

AjBool ensPredictionexonSetStartPhase(EnsPPredictionexon pe, ajint sphase);

AjBool ensPredictionexonSetScore(EnsPPredictionexon pe, double score);

AjBool ensPredictionexonSetPvalue(EnsPPredictionexon pe, double pvalue);

AjBool ensPredictionexonTrace(const EnsPPredictionexon pe, ajuint level);

ajint ensPredictionexonGetEndPhase(const EnsPPredictionexon pe);

ajuint ensPredictionexonGetMemSize(const EnsPPredictionexon pe);

EnsPPredictionexon ensPredictionexonTransform(EnsPPredictionexon pe,
                                              const AjPStr csname,
                                              const AjPStr csversion);

EnsPPredictionexon ensPredictionexonTransfer(EnsPPredictionexon pe,
                                             EnsPSlice slice);

/* Ensembl Prediction Exon Adaptor */

EnsPPredictionexonadaptor ensPredictionexonadaptorNew(EnsPDatabaseadaptor dba);

void ensPredictionexonadaptorDel(EnsPPredictionexonadaptor *Padaptor);

AjBool ensPredictionexonadaptorFetchAllByPredictiontranscript(
    EnsPPredictionexonadaptor pea,
    const EnsPPredictiontranscript pt,
    AjPList pes);

/* Ensembl Prediction Transcript */

EnsPPredictiontranscript ensPredictiontranscriptNew(
    EnsPPredictiontranscriptadaptor adaptor,
    ajuint identifier,
    EnsPFeature feature,
    AjPStr label);

EnsPPredictiontranscript ensPredictiontranscriptNewObj(
    const EnsPPredictiontranscript object);

EnsPPredictiontranscript ensPredictiontranscriptNewRef(
    EnsPPredictiontranscript pt);

void ensPredictiontranscriptDel(EnsPPredictiontranscript *Ppt);

EnsPPredictiontranscriptadaptor ensPredictiontranscriptGetAdaptor(
    const EnsPPredictiontranscript pt);

ajuint ensPredictiontranscriptGetIdentifier(const EnsPPredictiontranscript pt);

EnsPFeature ensPredictiontranscriptGetFeature(
    const EnsPPredictiontranscript pt);

AjPStr ensPredictiontranscriptGetDisplayLabel(
    const EnsPPredictiontranscript pt);

AjBool ensPredictiontranscriptSetAdaptor(
    EnsPPredictiontranscript pt,
    EnsPPredictiontranscriptadaptor adaptor);

AjBool ensPredictiontranscriptSetIdentifier(EnsPPredictiontranscript pt,
                                            ajuint identifier);

AjBool ensPredictiontranscriptSetFeature(EnsPPredictiontranscript pt,
                                         EnsPFeature feature);

AjBool ensPredictiontranscriptSetDisplayLabel(EnsPPredictiontranscript pt,
                                              AjPStr label);

AjBool ensPredictiontranscriptTrace(const EnsPPredictiontranscript pt,
                                    ajuint level);

ajuint ensPredictiontranscriptGetMemSize(const EnsPPredictiontranscript pt);

const AjPList ensPredictiontranscriptGetExons(EnsPPredictiontranscript pt);

/* Ensembl Prediction Transcript Adaptor */

EnsPPredictiontranscriptadaptor ensPredictiontranscriptadaptorNew(
    EnsPDatabaseadaptor dba);

void ensPredictiontranscriptadaptorDel(
    EnsPPredictiontranscriptadaptor *Padaptor);

AjBool ensPredictiontranscriptadaptorFetchByIdentifier(
    EnsPPredictiontranscriptadaptor adaptor,
    ajuint identifier,
    EnsPPredictiontranscript *Ppt);

AjBool ensPredictiontranscriptadaptorFetchByStableIdentifier(
    EnsPPredictiontranscriptadaptor adaptor,
    const AjPStr stableid,
    EnsPPredictiontranscript *Ppt);

/*
** End of prototype definitions
*/




#endif

#ifdef __cplusplus
}
#endif
