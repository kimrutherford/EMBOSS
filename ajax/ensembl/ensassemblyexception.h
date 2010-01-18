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




/******************************************************************************
**
** Ensembl Assembly Exception Type enumeration.
**
******************************************************************************/

enum EnsEAssemblyexceptionType
{
    ensEAssemblyexceptionTypeNULL,
    ensEAssemblyexceptionTypeHAP,
    ensEAssemblyexceptionTypePAR,
    ensEAssemblyexceptionTypeHAPREF
};




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
** @attr Type [AjEnum] Type
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
    AjEnum Type;
} EnsOAssemblyexception;

#define EnsPAssemblyexception EnsOAssemblyexception*




/*
** Prototype definitions
*/

/* Ensembl Assembly Exception */

EnsPAssemblyexception ensAssemblyexceptionNew(
    EnsPAssemblyexceptionadaptor adaptor,
    ajuint identifier,
    ajuint srid,
    ajuint srstart,
    ajuint srend,
    ajuint exid,
    ajuint exstart,
    ajuint exend,
    ajint ori,
    AjEnum type);

EnsPAssemblyexception ensAssemblyexceptionNewObj(
    const EnsPAssemblyexception object);

EnsPAssemblyexception ensAssemblyexceptionNewRef(EnsPAssemblyexception ae);

void ensAssemblyexceptionDel(EnsPAssemblyexception* Pae);

EnsPAssemblyexceptionadaptor ensAssemblyexceptionGetAdaptor(
    const EnsPAssemblyexception ae);

ajuint ensAssemblyexceptionGetIdentifier(const EnsPAssemblyexception ae);

ajuint ensAssemblyexceptionGetSeqregionIdentifier(
    const EnsPAssemblyexception ae);

ajuint ensAssemblyexceptionGetSeqregionStart(const EnsPAssemblyexception ae);

ajuint ensAssemblyexceptionGetSeqregionEnd(const EnsPAssemblyexception ae);

ajuint ensAssemblyexceptionGetExcRegionIdentifier(
    const EnsPAssemblyexception ae);

ajuint ensAssemblyexceptionGetExcRegionStart(const EnsPAssemblyexception ae);

ajuint ensAssemblyexceptionGetExcRegionEnd(const EnsPAssemblyexception ae);

ajint ensAssemblyexceptionGetOrientation(const EnsPAssemblyexception ae);

AjEnum ensAssemblyexceptionGetType(const EnsPAssemblyexception ae);

AjBool ensAssemblyexceptionSetAdaptor(EnsPAssemblyexception ae,
                                      EnsPAssemblyexceptionadaptor adaptor);

AjBool ensAssemblyexceptionSetIdentifier(EnsPAssemblyexception ae,
                                         ajuint identifier);

AjBool ensAssemblyexceptionSetSeqregionIdentifier(EnsPAssemblyexception ae,
                                                  ajuint srid);

AjBool ensAssemblyexceptionSetSeqregionStart(EnsPAssemblyexception ae,
                                             ajuint srstart);

AjBool ensAssemblyexceptionSetSeqregionEnd(EnsPAssemblyexception ae,
                                           ajuint srend);

AjBool ensAssemblyexceptionSetExcRegionIdentifier(EnsPAssemblyexception ae,
                                                  ajuint exid);

AjBool ensAssemblyexceptionSetExcRegionStart(EnsPAssemblyexception ae,
                                             ajuint exstart);

AjBool ensAssemblyexceptionSetExcRegionEnd(EnsPAssemblyexception ae,
                                           ajuint exend);

AjBool ensAssemblyexceptionSetOrientation(EnsPAssemblyexception ae,
                                          ajint orientation);

AjBool ensAssemblyexceptionSetType(EnsPAssemblyexception ae, AjEnum type);

AjBool ensAssemblyexceptionTrace(const EnsPAssemblyexception ae, ajuint level);

ajuint ensAssemblyexceptionGetMemSize(const EnsPAssemblyexception ae);

AjEnum ensAssemblyexceptionTypeFromStr(const AjPStr type);

const char* ensAssemblyexceptionTypeToChar(const AjEnum type);

/* Ensembl Assembly Exception Adaptor */

EnsPAssemblyexceptionadaptor ensAssemblyexceptionadaptorNew(
    EnsPDatabaseadaptor dba);

void ensAssemblyexceptionadaptorDel(EnsPAssemblyexceptionadaptor* Padaptor);

AjBool ensAssemblyexceptionadaptorFetchAll(
    const EnsPAssemblyexceptionadaptor adaptor,
    AjPList aes);

AjBool ensAssemblyexceptionadaptorFetchAllBySeqregionIdentifier(
    const EnsPAssemblyexceptionadaptor adaptor,
    ajuint srid,
    AjPList aes);

/*
** End of prototype definitions
*/




#endif

#ifdef __cplusplus
}
#endif

/*
** FIXME: The Ensembl Core SQL schema has some inconsistencies between the
** 'seq_region' and 'assembly_exception' tables.
** The 'assembly_exception.seq_region_start' and
** the 'assembly_exception.seq_region_end' fields are unsigned, while
** the 'seq_region.length' field is signed.
** The 'seq_region.length' field should really be unsigned.
**
** The 'assembly_exception.ori' field could be SMALLINT, but the table is
** small, so the gain of storage space would be minimal.
**
** FIXME: Ensembl core should move from TINYINT to SMALLINT as TINYINT is
** not part of the SQL:2003 standard.
*/
