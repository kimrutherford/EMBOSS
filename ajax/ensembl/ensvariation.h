
#ifndef ENSVARIATION_H
#define ENSVARIATION_H

/* ==================================================================== */
/* ========================== include files =========================== */
/* ==================================================================== */

#include "ensgvdata.h"

AJ_BEGIN_DECLS




/* ==================================================================== */
/* ============================ constants ============================= */
/* ==================================================================== */

/* @const EnsPGvtranscriptvariationadaptor ************************************
**
** Ensembl Genetic Variation Transcript Variation Adaptor
** Defined as an alias in EnsPGvbaseadaptor
**
** #alias EnsPGvbaseadaptor
** ##
******************************************************************************/

#define EnsPGvtranscriptvariationadaptor EnsPGvbaseadaptor




/* @const EnsEGvconsequenceType ***********************************************
**
** Ensembl Genetic Variation Consequence Type enumeration.
**
******************************************************************************/

typedef enum EnsOGvconsequenceType
{
    ensEGvconsequenceTypeNULL,
    ensEGvconsequenceTypeEssentialsplicesite,
    ensEGvconsequenceTypeStopgained,
    ensEGvconsequenceTypeStoplost,
    ensEGvconsequenceTypeComplexindel,
    ensEGvconsequenceTypeFrameshiftcoding,
    ensEGvconsequenceTypeSplicesite,
    ensEGvconsequenceTypePartialcodon,
    ensEGvconsequenceTypeSynonymouscoding,
    ensEGvconsequenceTypeRegulatoryregion,
    ensEGvconseqeunceTypeWithinmaturemirna,
    ensEGvconsequenceType5primeutr,
    ensEGvconsequenceType3primeutr,
    ensEGvconsequenceTypeUtr,
    ensEGvconsequenceTypeIntronic,
    ensEGvconsequenceTypeNmdtranscript,
    ensEGvconsequenceTypeWithinnoncodinggene,
    ensEGvconsequenceTypeUpstream,
    ensEGvconsequenceTypeDownstream,
    ensEGvconsequenceTypeHgmdmutation,
    ensEGvconsequenceTypeNoconsequence,
    ensEGvconsequenceTypeIntergenic,
    ensEGvconsequenceType_
} EnsEGvconsequenceType;




/* ==================================================================== */
/* ========================== public data ============================= */
/* ==================================================================== */

/* @data EnsPGvconsequence ****************************************************
**
** Ensembl Genetic Variation Consequence.
**
** @alias EnsSGvconsequence
** @alias EnsOGvconsequence
**
** @attr Use [ajuint] Use counter
** @cc Bio::EnsEMBL::Variation::ConsequenceType
** @attr Transcriptidentifier [ajuint] Ensembl Transcript identifier
** @attr Gvvariationfeatureidentifier [ajuint] Ensembl Genetic Variation
**                                             Variation Feature identifier
** @attr Start [ajuint] Start
** @attr End [ajuint] End
** @attr Strand [ajuint] Strand
** @attr Alleles [AjPList] AJAX List of AJAX String objects
** @attr Types [AjPList] AJAX List of AJAX String objects
** @@
******************************************************************************/

typedef struct EnsSGvconsequence
{
    ajuint Use;
    ajuint Transcriptidentifier;
    ajuint Gvvariationfeatureidentifier;
    ajuint Start;
    ajuint End;
    ajuint Strand;
    AjPList Alleles;
    AjPList Types;
} EnsOGvconsequence;

#define EnsPGvconsequence EnsOGvconsequence*




/* @data EnsPGvtranscriptvariation ********************************************
**
** Ensembl Genetic Variation Transcript Variation.
**
** @alias EnsSGvtranscriptvariation
** @alias EnsOGvtranscriptvariation
**
** @attr Use [ajuint] Use counter
** @cc Bio::EnsEMBL::Storable
** @attr Identifier [ajuint] SQL database-internal identifier
** @attr Adaptor [EnsPGvtranscriptvariationadaptor]
** Ensembl Genetic Variation Transcript Variation Adaptor
** @cc Bio::EnsEMBL::Variation::TranscriptVariation
** @attr Gvvariationfeature [EnsPGvvariationfeature]
** Ensembl Genetic Variation Variation Feature
** @attr TranscriptObject [EnsPTranscript] Ensembl Transcript
** @attr TranslationAllele [AjPStr] Ensembl Translation allele
** @attr TranscriptStart [ajuint] Transcript start
** @attr TranscriptEnd [ajuint] Transcript end
** @attr CodingStart [ajuint] Coding start
** @attr CodingEnd [ajuint] Coding end
** @attr TranslationStart [ajuint] Translation start
** @attr TranslationEnd [ajuint] Translation end
** @attr GvconsequenceTypes [ajuint] Ensembl Genetic Variation Consequence Type
** bit field
** @attr Padding [ajuint] Padding to alignment boundary
** @@
******************************************************************************/

typedef struct EnsSGvtranscriptvariation
{
    ajuint Use;
    ajuint Identifier;
    EnsPGvtranscriptvariationadaptor Adaptor;
    EnsPGvvariationfeature Gvvariationfeature;
    EnsPTranscript TranscriptObject;
    AjPStr TranslationAllele;
    ajuint TranscriptStart;
    ajuint TranscriptEnd;
    ajuint CodingStart;
    ajuint CodingEnd;
    ajuint TranslationStart;
    ajuint TranslationEnd;
    ajuint GvconsequenceTypes;
    ajuint Padding;
} EnsOGvtranscriptvariation;

#define EnsPGvtranscriptvariation EnsOGvtranscriptvariation*




/* ==================================================================== */
/* ======================= public functions =========================== */
/* ==================================================================== */

/*
** Prototype definitions
*/

/* Ensembl Genetic Variation Consequence */

EnsPGvconsequence ensGvconsequenceNewCpy(const EnsPGvconsequence gvc);

EnsPGvconsequence ensGvconsequenceNewIni(ajuint transcriptid,
                                         ajuint gvvfid,
                                         ajint start,
                                         ajint end,
                                         ajint strand);

EnsPGvconsequence ensGvconsequenceNewRef(EnsPGvconsequence gvc);

void ensGvconsequenceDel(EnsPGvconsequence* Pgvc);

ajint ensGvconsequenceGetEnd(
    const EnsPGvconsequence gvc);

ajuint ensGvconsequenceGetGvvariationfeatureidentifier(
    const EnsPGvconsequence gvc);

ajint ensGvconsequenceGetStart(
    const EnsPGvconsequence gvc);

ajint ensGvconsequenceGetStrand(
    const EnsPGvconsequence gvc);

ajuint ensGvconsequenceGetTranscriptidentifier(
    const EnsPGvconsequence gvc);

AjBool ensGvconsequenceSetEnd(EnsPGvconsequence gvc,
                              ajint end);

AjBool ensGvconsequenceSetGvvariationfeatureidentifier(EnsPGvconsequence gvc,
                                                       ajuint gvvfid);

AjBool ensGvconsequenceSetStart(EnsPGvconsequence gvc,
                                ajint start);

AjBool ensGvconsequenceSetStrand(EnsPGvconsequence gvc,
                                 ajint strand);

AjBool ensGvconsequenceSetTranscriptidentifier(EnsPGvconsequence gvc,
                                               ajuint transcriptid);

AjBool ensGvconsequenceTrace(const EnsPGvconsequence gvc,
                             ajuint level);

size_t ensGvconsequenceCalculateMemsize(const EnsPGvconsequence gvc);

/* Ensembl Genetic Variation Consequence Type */

EnsEGvconsequenceType ensGvconsequenceTypeFromStr(
    const AjPStr consequencetype);

const char* ensGvconsequenceTypeToChar(
    EnsEGvconsequenceType gvct);

ajuint ensGvconsequenceTypesFromSet(const AjPStr gvctset);

AjBool ensGvconsequenceTypesToSet(ajuint gvctbf, AjPStr* Pgvctset);

/* Ensembl Genetic Variation Transcript Variation */

EnsPGvtranscriptvariation ensGvtranscriptvariationNewCpy(
    const EnsPGvtranscriptvariation gvtv);

EnsPGvtranscriptvariation ensGvtranscriptvariationNewIni(
    EnsPGvtranscriptvariationadaptor gvtva,
    ajuint identifier,
    EnsPTranscript transcript,
    EnsPGvvariationfeature gvvf,
    AjPStr translationallele,
    AjPStr consequencetype,
    ajuint codingstart,
    ajuint codingend,
    ajuint transcriptstart,
    ajuint transcriptend,
    ajuint translationstart,
    ajuint translationend);

EnsPGvtranscriptvariation ensGvtranscriptvariationNewRef(
    EnsPGvtranscriptvariation gvtv);

void ensGvtranscriptvariationDel(EnsPGvtranscriptvariation* Pgvtv);

EnsPGvtranscriptvariationadaptor ensGvtranscriptvariationGetAdaptor(
    const EnsPGvtranscriptvariation gvtv);

ajuint ensGvtranscriptvariationGetCodingEnd(
    const EnsPGvtranscriptvariation gvtv);

ajuint ensGvtranscriptvariationGetCodingStart(
    const EnsPGvtranscriptvariation gvtv);

EnsPGvvariationfeature ensGvtranscriptvariationGetGvvariationfeature(
    const EnsPGvtranscriptvariation gvtv);

ajuint ensGvtranscriptvariationGetIdentifier(
    const EnsPGvtranscriptvariation gvtv);

ajuint ensGvtranscriptvariationGetTranscriptEnd(
    const EnsPGvtranscriptvariation gvtv);

EnsPTranscript ensGvtranscriptvariationGetTranscriptObject(
    const EnsPGvtranscriptvariation gvtv);

ajuint ensGvtranscriptvariationGetTranscriptStart(
    const EnsPGvtranscriptvariation gvtv);

AjPStr ensGvtranscriptvariationGetTranslationAllele(
    const EnsPGvtranscriptvariation gvtv);

ajuint ensGvtranscriptvariationGetTranslationEnd(
    const EnsPGvtranscriptvariation gvtv);

ajuint ensGvtranscriptvariationGetTranslationStart(
    const EnsPGvtranscriptvariation gvtv);

AjBool ensGvtranscriptvariationSetAdaptor(
    EnsPGvtranscriptvariation gvtv,
    EnsPGvtranscriptvariationadaptor gvtva);

AjBool ensGvtranscriptvariationSetCodingEnd(
    EnsPGvtranscriptvariation gvtv,
    ajuint codingend);

AjBool ensGvtranscriptvariationSetCodingStart(
    EnsPGvtranscriptvariation gvtv,
    ajuint codingstart);

AjBool ensGvtranscriptvariationSetGvvariationfeature(
    EnsPGvtranscriptvariation gvtv,
    EnsPGvvariationfeature gvvf);

AjBool ensGvtranscriptvariationSetIdentifier(
    EnsPGvtranscriptvariation gvtv,
    ajuint identifier);

AjBool ensGvtranscriptvariationSetTranscriptEnd(
    EnsPGvtranscriptvariation gvtv,
    ajuint transcriptend);

AjBool ensGvtranscriptvariationSetTranscriptObject(
    EnsPGvtranscriptvariation gvtv,
    EnsPTranscript transcript);

AjBool ensGvtranscriptvariationSetTranscriptStart(
    EnsPGvtranscriptvariation gvtv,
    ajuint transcriptstart);

AjBool ensGvtranscriptvariationSetTranslationAllele(
    EnsPGvtranscriptvariation gvtv,
    AjPStr translationallele);

AjBool ensGvtranscriptvariationSetTranslationEnd(
    EnsPGvtranscriptvariation gvtv,
    ajuint translationend);

AjBool ensGvtranscriptvariationSetTranslationStart(
    EnsPGvtranscriptvariation gvtv,
    ajuint translationstart);

AjBool ensGvtranscriptvariationTrace(const EnsPGvtranscriptvariation gvtv,
                                     ajuint level);

size_t ensGvtranscriptvariationCalculateMemsize(
    const EnsPGvtranscriptvariation gvtv);

/* Ensembl Genetic Variation Transcript Variation Adaptor */

EnsPGvtranscriptvariationadaptor ensRegistryGetGvtranscriptvariationadaptor(
    EnsPDatabaseadaptor dba);

EnsPGvtranscriptvariationadaptor ensGvtranscriptvariationadaptorNew(
    EnsPDatabaseadaptor dba);

void ensGvtranscriptvariationadaptorDel(
    EnsPGvtranscriptvariationadaptor* Pgvtva);

/*
** End of prototype definitions
*/




AJ_END_DECLS

#endif /* !ENSVARIATION_H */
