#ifdef __cplusplus
extern "C"
{
#endif

#ifndef ensassemblyexception_h
#define ensassemblyexception_h

#include "ensdatabaseadaptor.h"
#include "enstable.h"




/* @data EnsPAssemblyexceptionadaptor *****************************************
**
** Ensembl Assembly Exception Adaptor.
**
** @alias EnsSAssemblyexceptionadaptor
** @alias EnsOAssemblyexceptionadaptor
**
** @attr Adaptor [EnsPDatabaseadaptor] Ensembl Database Adaptor
** @attr CacheBySeqregionIdentifier [AjPTable] Ensembl Assembly Exception cache
**                                             indexed on Ensembl Sequence
**                                             Region identifiers
** @@
******************************************************************************/

typedef struct EnsSAssemblyexceptionadaptor
{
    EnsPDatabaseadaptor Adaptor;
    AjPTable CacheBySeqregionIdentifier;
} EnsOAssemblyexceptionadaptor;

#define EnsPAssemblyexceptionadaptor EnsOAssemblyexceptionadaptor*




/* EnsEAssemblyexceptionType **************************************************
**
** Ensembl Assembly Exception Type enumeration.
**
******************************************************************************/

typedef enum EnsOAssemblyexceptionType
{
    ensEAssemblyexceptionTypeNULL,
    ensEAssemblyexceptionTypeHAP,
    ensEAssemblyexceptionTypePAR,
    ensEAssemblyexceptionTypeHAPREF
} EnsEAssemblyexceptionType;




/* @data EnsPAssemblyexception ************************************************
**
** Ensembl Assembly Exception.
**
** An Ensembl Assembly Exception object encapsulates information about
** exceptions in the reference assembly path. Currently, the Ensembl system
** models two types of assembly exceptions.
**
** Haplotype (HAPs) assemblies represent alternative assembly paths of regions
** of increased genetic variablility.
** Pseudo-autosomal regions (PARs) are paired stretches at either tip of
** (mammalian) sex chromosomes where frequent chrossing over events occur.
** Due to the frequent exchange of genetic material those sequences represent
** exact copies of each other.
**
** @alias EnsSAssemblyexception
** @alias EnsOAssemblyexception
**
** @attr Use [ajuint] Use counter
** @cc Bio::EnsEMBL::Storable
** @attr Identifier [ajuint] SQL database-internal identifier
** @attr Adaptor [EnsPAssemblyexceptionadaptor] Ensembl Assembly
**                                              Exception Adaptor
** @cc Bio::EnsEMBL::AssemblyException
** @attr SeqregionIdentifier [ajuint] Ensembl Sequence Region identifier
** @attr SeqregionStart [ajuint] Ensembl Sequence Region start
** @attr SeqregionEnd [ajuint] Ensembl Sequence Region end
** @attr ExcRegionIdentifier [ajuint] Ensembl Sequence Region identifier
** @attr ExcRegionStart [ajuint] Ensembl Sequence Region start
** @attr ExcRegionEnd [ajuint] Ensembl Sequence Region end
** @attr Orientation [ajint] Orientation
** @attr Type [EnsEAssemblyexceptionType] Type
** @@
******************************************************************************/

typedef struct EnsSAssemblyexception
{
    ajuint Use;
    ajuint Identifier;
    EnsPAssemblyexceptionadaptor Adaptor;
    ajuint SeqregionIdentifier;
    ajuint SeqregionStart;
    ajuint SeqregionEnd;
    ajuint ExcRegionIdentifier;
    ajuint ExcRegionStart;
    ajuint ExcRegionEnd;
    ajint Orientation;
    EnsEAssemblyexceptionType Type;
} EnsOAssemblyexception;

#define EnsPAssemblyexception EnsOAssemblyexception*




/*
** Prototype definitions
*/

/* Ensembl Assembly Exception */

EnsPAssemblyexception ensAssemblyexceptionNew(
    EnsPAssemblyexceptionadaptor aea,
    ajuint identifier,
    ajuint srid,
    ajuint srstart,
    ajuint srend,
    ajuint erid,
    ajuint erstart,
    ajuint erend,
    ajint ori,
    EnsEAssemblyexceptionType type);

EnsPAssemblyexception ensAssemblyexceptionNewObj(
    const EnsPAssemblyexception object);

EnsPAssemblyexception ensAssemblyexceptionNewRef(EnsPAssemblyexception ae);

void ensAssemblyexceptionDel(EnsPAssemblyexception* Pae);

EnsPAssemblyexceptionadaptor ensAssemblyexceptionGetAdaptor(
    const EnsPAssemblyexception ae);

ajuint ensAssemblyexceptionGetIdentifier(
    const EnsPAssemblyexception ae);

ajuint ensAssemblyexceptionGetSeqregionIdentifier(
    const EnsPAssemblyexception ae);

ajuint ensAssemblyexceptionGetSeqregionStart(
    const EnsPAssemblyexception ae);

ajuint ensAssemblyexceptionGetSeqregionEnd(
    const EnsPAssemblyexception ae);

ajuint ensAssemblyexceptionGetExcRegionIdentifier(
    const EnsPAssemblyexception ae);

ajuint ensAssemblyexceptionGetExcRegionStart(
    const EnsPAssemblyexception ae);

ajuint ensAssemblyexceptionGetExcRegionEnd(const EnsPAssemblyexception ae);

ajint ensAssemblyexceptionGetOrientation(
    const EnsPAssemblyexception ae);

EnsEAssemblyexceptionType ensAssemblyexceptionGetType(
    const EnsPAssemblyexception ae);

AjBool ensAssemblyexceptionSetAdaptor(EnsPAssemblyexception ae,
                                      EnsPAssemblyexceptionadaptor aea);

AjBool ensAssemblyexceptionSetIdentifier(EnsPAssemblyexception ae,
                                         ajuint identifier);

AjBool ensAssemblyexceptionSetSeqregionIdentifier(EnsPAssemblyexception ae,
                                                  ajuint srid);

AjBool ensAssemblyexceptionSetSeqregionStart(EnsPAssemblyexception ae,
                                             ajuint srstart);

AjBool ensAssemblyexceptionSetSeqregionEnd(EnsPAssemblyexception ae,
                                           ajuint srend);

AjBool ensAssemblyexceptionSetExcRegionIdentifier(EnsPAssemblyexception ae,
                                                  ajuint erid);

AjBool ensAssemblyexceptionSetExcRegionStart(EnsPAssemblyexception ae,
                                             ajuint erstart);

AjBool ensAssemblyexceptionSetExcRegionEnd(EnsPAssemblyexception ae,
                                           ajuint erend);

AjBool ensAssemblyexceptionSetOrientation(EnsPAssemblyexception ae,
                                          ajint orientation);

AjBool ensAssemblyexceptionSetType(EnsPAssemblyexception ae,
                                   EnsEAssemblyexceptionType type);

AjBool ensAssemblyexceptionTrace(const EnsPAssemblyexception ae, ajuint level);

ajulong ensAssemblyexceptionGetMemsize(const EnsPAssemblyexception ae);

EnsEAssemblyexceptionType ensAssemblyexceptionTypeFromStr(const AjPStr type);

const char* ensAssemblyexceptionTypeToChar(
    const EnsEAssemblyexceptionType type);

/* Ensembl Assembly Exception Adaptor */

EnsPAssemblyexceptionadaptor ensRegistryGetAssemblyexceptionadaptor(
    EnsPDatabaseadaptor dba);

EnsPAssemblyexceptionadaptor ensAssemblyexceptionadaptorNew(
    EnsPDatabaseadaptor dba);

void ensAssemblyexceptionadaptorDel(EnsPAssemblyexceptionadaptor* Paea);

AjBool ensAssemblyexceptionadaptorFetchAll(
    const EnsPAssemblyexceptionadaptor aea,
    AjPList aes);

AjBool ensAssemblyexceptionadaptorFetchAllBySeqregionIdentifier(
    const EnsPAssemblyexceptionadaptor aea,
    ajuint srid,
    AjPList aes);

/*
** End of prototype definitions
*/




#endif /* ensassemblyexception_h */

#ifdef __cplusplus
}
#endif
