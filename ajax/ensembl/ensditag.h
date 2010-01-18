#ifdef __cplusplus
extern "C"
{
#endif

#ifndef ensditag_h
#define ensditag_h

#include "ensfeature.h"




/* @data EnsPDitag ************************************************************
**
** Ensembl Ditag
**
** @alias EnsSDitag
** @alias EnsODitag
**
** @attr Use [ajuint] Use counter
** @cc Bio::EnsEMBL::Storable
** @attr Identifier [ajuint] Internal SQL database identifier (primary key)
** @attr Adaptor [EnsPDatabaseadaptor] Ensembl Database Adaptor
** @cc Bio::EnsEMBL::Map::Ditag
** @attr Name [AjPStr] Name
** @attr Type [AjPStr] Source
** @attr Sequence [AjPStr] Sequence
** @attr Count [ajuint] Count
** @attr Padding [ajuint] Padding to alignment boundary
** @@
******************************************************************************/

typedef struct EnsSDitag
{
    ajuint Use;
    ajuint Identifier;
    EnsPDatabaseadaptor Adaptor;
    AjPStr Name;
    AjPStr Type;
    AjPStr Sequence;
    ajuint Count;
    ajuint Padding;
} EnsODitag;

#define EnsPDitag EnsODitag*




/* @data EnsPDitagfeatureadaptor **********************************************
**
** Ensembl Ditag Feature Adaptor.
**
** @alias EnsSDitagfeatureadaptor
** @alias EnsODitagfeatureadaptor
**
** @cc Bio::EnsEMBL::Map::DBSQL::Ditagfeatureadaptor
** @attr Adaptor [EnsPFeatureadaptor] Ensembl Feature Adaptor
** @@
******************************************************************************/

typedef struct EnsSDitagfeatureadaptor
{
    EnsPFeatureadaptor Adaptor;
} EnsODitagfeatureadaptor;

#define EnsPDitagfeatureadaptor EnsODitagfeatureadaptor*




/******************************************************************************
**
** Ensembl Ditag Feature Side enumeration.
**
******************************************************************************/

enum EnsEDitagfeatureSide
{
    ensEDitagfeatureSideNULL,
    ensEDitagfeatureSideLeft,
    ensEDitagfeatureSideRight,
    ensEDitagfeatureSideFull
};




/* @data EnsPDitagfeature *****************************************************
**
** Ensembl Ditag Feature
**
** @alias EnsSDitagfeature
** @alias EnsODitagfeature
**
** @attr Use [ajuint] Use counter
** @cc Bio::EnsEMBL::Storable
** @attr Identifier [ajuint] Internal SQL database identifier (primary key)
** @attr Adaptor [EnsPDitagfeatureadaptor] Ensembl Ditag Feature Adaptor
** @cc Bio::EnsEMBL::Feature
** @attr Feature [EnsPFeature] Ensembl Feature
** @cc Bio::EnsEMBL::Map::DitagFeature
** @attr Ditag [EnsPDitag] Ditag
** @attr Cigar [AjPStr] CIGAR line
** @attr Side [AjEnum] Side
** @attr TargetStart [ajint] Target start
** @attr TargetEnd [ajint] Target end
** @attr TargetStrand [ajint] Target strand
** @attr PairIdentifier [ajuint] Pair identifier
** @attr Padding [ajuint] Padding to alignment boundary
** @@
******************************************************************************/

typedef struct EnsSDitagfeature
{
    ajuint Use;
    ajuint Identifier;
    EnsPDitagfeatureadaptor Adaptor;
    EnsPFeature Feature;
    EnsPDitag Ditag;
    AjPStr Cigar;
    AjEnum Side;
    ajint TargetStart;
    ajint TargetEnd;
    ajint TargetStrand;
    ajuint PairIdentifier;
    ajuint Padding;
} EnsODitagfeature;

#define EnsPDitagfeature EnsODitagfeature*




/*
** Prototype definitions
*/

/* Ensembl Ditag */

EnsPDitag ensDitagNew(EnsPDatabaseadaptor adaptor,
                      ajuint identifier,
                      AjPStr name,
                      AjPStr type,
                      AjPStr sequence,
                      ajuint count);

EnsPDitag ensDitagNewObj(const EnsPDitag object);

EnsPDitag ensDitagNewRef(EnsPDitag dt);

void ensDitagDel(EnsPDitag* Pdt);

EnsPDatabaseadaptor ensDitagGetAdaptor(const EnsPDitag dt);

ajuint ensDitagGetIdentifier(const EnsPDitag dt);

AjPStr ensDitagGetName(const EnsPDitag dt);

AjPStr ensDitagGetType(const EnsPDitag dt);

AjPStr ensDitagGetSequence(const EnsPDitag dt);

ajuint ensDitagGetCount(const EnsPDitag dt);

AjBool ensDitagSetAdaptor(EnsPDitag dt, EnsPDatabaseadaptor adaptor);

AjBool ensDitagSetIdentifier(EnsPDitag dt, ajuint identifier);

AjBool ensDitagSetName(EnsPDitag dt, AjPStr name);

AjBool ensDitagSetType(EnsPDitag dt, AjPStr type);

AjBool ensDitagSetSequence(EnsPDitag dt, AjPStr sequence);

AjBool ensDitagSetCount(EnsPDitag dt, ajuint count);

AjBool ensDitagTrace(const EnsPDitag dt, ajuint level);

ajuint ensDitagGetMemSize(const EnsPDitag dt);

/* Ensembl Ditag Adaptor */

AjBool ensDitagadaptorFetchByIdentifier(EnsPDatabaseadaptor dba,
                                        ajuint identifier,
                                        EnsPDitag *Pdt);

AjBool ensDitagadaptorFetchAll(EnsPDatabaseadaptor dba,
                               const AjPStr name,
                               const AjPStr type,
                               AjPList dts);

AjBool ensDitagadaptorFetchAllByName(EnsPDatabaseadaptor dba,
                                     const AjPStr name,
                                     AjPList dts);

AjBool ensDitagadaptorFetchAllByType(EnsPDatabaseadaptor dba,
                                     const AjPStr type,
                                     AjPList dts);

/* Ensembl Ditag Feature */

EnsPDitagfeature ensDitagfeatureNew(EnsPDitagfeatureadaptor adaptor,
                                    ajuint identifier,
                                    EnsPFeature feature,
                                    EnsPDitag dt,
                                    AjPStr cigar,
                                    AjEnum side,
                                    ajint tstart,
                                    ajint tend,
                                    ajint tstrand,
                                    ajuint pairid);

EnsPDitagfeature ensDitagfeatureNewObj(const EnsPDitagfeature object);

EnsPDitagfeature ensDitagfeatureNewRef(EnsPDitagfeature dtf);

void ensDitagfeatureDel(EnsPDitagfeature* Pdtf);

EnsPDitagfeatureadaptor ensDitagfeatureGetAdaptor(const EnsPDitagfeature dtf);

ajuint ensDitagfeatureGetIdentifier(const EnsPDitagfeature dtf);

EnsPFeature ensDitagfeatureGetFeature(const EnsPDitagfeature dtf);

EnsPDitag ensDitagfeatureGetDitag(const EnsPDitagfeature dtf);

AjPStr ensDitagfeatureGetCigar(const EnsPDitagfeature dtf);

AjEnum ensDitagfeatureGetSide(const EnsPDitagfeature dtf);

ajint ensDitagfeatureGetTargetStart(const EnsPDitagfeature dtf);

ajint ensDitagfeatureGetTargetEnd(const EnsPDitagfeature dtf);

ajint ensDitagfeatureGetTargetStrand(const EnsPDitagfeature dtf);

ajuint ensDitagfeatureGetPairIdentifier(const EnsPDitagfeature dtf);

AjBool ensDitagfeatureSetAdaptor(EnsPDitagfeature dtf,
                                 EnsPDitagfeatureadaptor adaptor);

AjBool ensDitagfeatureSetIdentifier(EnsPDitagfeature dtf, ajuint identifier);

AjBool ensDitagfeatureSetFeature(EnsPDitagfeature dtf, EnsPFeature feature);

AjBool ensDitagfeatureSetDitag(EnsPDitagfeature dtf, EnsPDitag dt);

AjBool ensDitagfeatureSetCigar(EnsPDitagfeature dtf, AjPStr cigar);

AjBool ensDitagfeatureSetSide(EnsPDitagfeature dtf, AjEnum side);

AjBool ensDitagfeatureSetTargetStart(EnsPDitagfeature dtf, ajint tstart);

AjBool ensDitagfeatureSetTargetEnd(EnsPDitagfeature dtf, ajint tend);

AjBool ensDitagfeatureSetTargetStrand(EnsPDitagfeature dtf, ajint tstrand);

AjBool ensDitagfeatureSetPairIdentifier(EnsPDitagfeature dtf, ajuint pairid);

AjBool ensDitagfeatureTrace(const EnsPDitagfeature dtf, ajuint level);

ajuint ensDitagfeatureGetMemSize(const EnsPDitagfeature dtf);

AjEnum ensDitagfeatureSideFromStr(const AjPStr side);

const char *ensDitagfeatureSideToChar(const AjEnum side);

/* Ensembl Ditag Feature Adaptor */

EnsPDitagfeatureadaptor ensDitagfeatureadaptorNew(EnsPDatabaseadaptor dba);

void ensDitagfeatureadaptorDel(EnsPDitagfeatureadaptor *Padaptor);

AjBool ensDitagadaptorFetchAllByIdentifier(const EnsPDitagfeatureadaptor adaptor,
                                           ajuint identifier,
                                           EnsPDitagfeature *Pdtf);

AjBool ensDitagfeatureadaptorFetchAllByDitag(const EnsPDitagfeatureadaptor adaptor,
                                             const EnsPDitag dt,
                                             AjPList dtfs);

AjBool ensDitagfeatureadaptorFetchAllByType(const EnsPDitagfeatureadaptor adaptor,
                                            const AjPStr type,
                                            AjPList dtfs);

AjBool ensDitagfeatureadaptorFetchAllBySlice(const EnsPDitagfeatureadaptor adaptor,
                                             EnsPSlice slice,
                                             const AjPStr type,
                                             const AjPStr anname,
                                             AjPList dtfs);

/*
** End of prototype definitions
*/




#endif

#ifdef __cplusplus
}
#endif
