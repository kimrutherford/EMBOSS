#ifdef __cplusplus
extern "C"
{
#endif

#ifndef ensprediction_h
#define ensprediction_h

#include "ensfeature.h"




/* EnsPPredictionexonadaptor **************************************************
**
** Ensembl Prediction Exon Adaptor
**
** Bio::EnsEMBL::DBSQL::PredictionExonAdaptor
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
** @cc Bio::EnsEMBL::PredictionExon
** @cc 'prediction_exon' SQL table
** @attr Score [double] Score
** @attr Pvalue [double] P-value
** @attr StartPhase [ajint] Start phase
** @attr Padding [char[4]] Padding to alignment boundary
** @attr SequenceCache [AjPStr] Sequence cache
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
    AjPStr SequenceCache;
} EnsOPredictionexon;

#define EnsPPredictionexon EnsOPredictionexon*




/* EnsPPredictiontranscriptadaptor ********************************************
**
** Ensembl Prediction Transcript Adaptor
**
** Bio::EnsEMBL::DBSQL::PredictionTranscriptAdaptor
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
** @cc Bio::EnsEMBL::PredictionTranscript
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

EnsPPredictionexon ensPredictionexonNew(EnsPPredictionexonadaptor pea,
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
                                   EnsPPredictionexonadaptor pea);

AjBool ensPredictionexonSetIdentifier(EnsPPredictionexon pe,
                                      ajuint identifier);

AjBool ensPredictionexonSetFeature(EnsPPredictionexon pe,
                                   EnsPFeature feature);

AjBool ensPredictionexonSetStartPhase(EnsPPredictionexon pe,
                                      ajint sphase);

AjBool ensPredictionexonSetScore(EnsPPredictionexon pe,
                                 double score);

AjBool ensPredictionexonSetPvalue(EnsPPredictionexon pe,
                                  double pvalue);

AjBool ensPredictionexonTrace(const EnsPPredictionexon pe, ajuint level);

ajint ensPredictionexonGetEndPhase(const EnsPPredictionexon pe);

ajulong ensPredictionexonGetMemsize(const EnsPPredictionexon pe);

EnsPPredictionexon ensPredictionexonTransform(EnsPPredictionexon pe,
                                              const AjPStr csname,
                                              const AjPStr csversion);

EnsPPredictionexon ensPredictionexonTransfer(EnsPPredictionexon pe,
                                             EnsPSlice slice);

AjBool ensPredictionexonFetchSequenceStr(EnsPPredictionexon pe,
                                         AjPStr *Psequence);

AjBool ensPredictionexonFetchSequenceSeq(EnsPPredictionexon pe,
                                         AjPSeq* Psequence);

AjBool ensPredictionexonSortByStartAscending(AjPList pes);

AjBool ensPredictionexonSortByStartDescending(AjPList pes);

/* Ensembl Prediction Exon Adaptor */

EnsPPredictionexonadaptor ensRegistryGetPredictionexonadaptor(
    EnsPDatabaseadaptor dba);

EnsPPredictionexonadaptor ensPredictionexonadaptorNew(
    EnsPDatabaseadaptor dba);

void ensPredictionexonadaptorDel(EnsPPredictionexonadaptor *Ppea);

AjBool ensPredictionexonadaptorFetchAllByPredictiontranscript(
    EnsPPredictionexonadaptor pea,
    const EnsPPredictiontranscript pt,
    AjPList pes);

/* Ensembl Prediction Transcript */

EnsPPredictiontranscript ensPredictiontranscriptNew(
    EnsPPredictiontranscriptadaptor pta,
    ajuint identifier,
    EnsPFeature feature,
    AjPStr label);

EnsPPredictiontranscript ensPredictiontranscriptNewObj(
    const EnsPPredictiontranscript object);

EnsPPredictiontranscript ensPredictiontranscriptNewRef(
    EnsPPredictiontranscript pt);

void ensPredictiontranscriptDel(
    EnsPPredictiontranscript *Ppt);

EnsPPredictiontranscriptadaptor ensPredictiontranscriptGetAdaptor(
    const EnsPPredictiontranscript pt);

ajuint ensPredictiontranscriptGetIdentifier(
    const EnsPPredictiontranscript pt);

EnsPFeature ensPredictiontranscriptGetFeature(
    const EnsPPredictiontranscript pt);

AjPStr ensPredictiontranscriptGetDisplayLabel(
    const EnsPPredictiontranscript pt);

AjBool ensPredictiontranscriptSetAdaptor(
    EnsPPredictiontranscript pt,
    EnsPPredictiontranscriptadaptor pta);

AjBool ensPredictiontranscriptSetIdentifier(
    EnsPPredictiontranscript pt,
    ajuint identifier);

AjBool ensPredictiontranscriptSetFeature(
    EnsPPredictiontranscript pt,
    EnsPFeature feature);

AjBool ensPredictiontranscriptSetDisplayLabel(
    EnsPPredictiontranscript pt,
    AjPStr label);

AjBool ensPredictiontranscriptTrace(
    const EnsPPredictiontranscript pt,
    ajuint level);

ajulong ensPredictiontranscriptGetMemsize(
    const EnsPPredictiontranscript pt);

const AjPList ensPredictiontranscriptGetExons(
    EnsPPredictiontranscript pt);

ajuint ensPredictiontranscriptGetTranscriptCodingStart(
    const EnsPPredictiontranscript pt);

ajuint ensPredictiontranscriptGetTranscriptCodingEnd(
    EnsPPredictiontranscript pt);

ajuint ensPredictiontranscriptGetSliceCodingStart(
    const EnsPPredictiontranscript pt);

ajuint ensPredictiontranscriptGetSliceCodingEnd(
    const EnsPPredictiontranscript pt);

AjBool ensPredictiontranscriptFetchSequenceStr(
    EnsPPredictiontranscript pt,
    AjPStr *Psequence);

AjBool ensPredictiontranscriptFetchSequenceSeq(
    EnsPPredictiontranscript pt,
    AjPSeq *Psequence);

AjBool ensPredictiontranscriptFetchTranslationSequenceStr(
    EnsPPredictiontranscript pt,
    AjPStr *Psequence);

AjBool ensPredictiontranscriptFetchTranslationSequenceSeq(
    EnsPPredictiontranscript pt,
    AjPSeq *Psequence);

AjBool ensPredictiontranscriptSortByStartAscending(AjPList pts);

AjBool ensPredictiontranscriptSortByStartDescending(AjPList pts);

/* Ensembl Prediction Transcript Adaptor */

EnsPPredictiontranscriptadaptor ensRegistryGetPredictiontranscriptadaptor(
    EnsPDatabaseadaptor dba);

EnsPPredictiontranscriptadaptor ensPredictiontranscriptadaptorNew(
    EnsPDatabaseadaptor dba);

void ensPredictiontranscriptadaptorDel(
    EnsPPredictiontranscriptadaptor *Ppta);

AjBool ensPredictiontranscriptadaptorFetchByIdentifier(
    EnsPPredictiontranscriptadaptor pta,
    ajuint identifier,
    EnsPPredictiontranscript *Ppt);

AjBool ensPredictiontranscriptadaptorFetchByStableIdentifier(
    EnsPPredictiontranscriptadaptor pta,
    const AjPStr stableid,
    EnsPPredictiontranscript *Ppt);

/*
** End of prototype definitions
*/




#endif /* ensprediction_h */

#ifdef __cplusplus
}
#endif
