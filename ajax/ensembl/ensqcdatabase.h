#ifdef __cplusplus
extern "C"
{
#endif

#ifndef ensqcdatabase_h
#define ensqcdatabase_h

#include "ensanalysis.h"




/* @data EnsPQcdatabaseadaptor ************************************************
**
** Ensembl QC Database Adaptor
**
** @alias EnsSQcdatabaseadaptor
** @alias EnsOQcdatabaseadaptor
**
** @attr Adaptor [EnsPBaseadaptor] Ensembl Base Adaptor
** @attr CacheByIdentifier [AjPTable] Identifier cache
** @attr CacheByName [AjPTable] Name cache
** @@
******************************************************************************/

typedef struct EnsSQcdatabaseadaptor
{
    EnsPBaseadaptor Adaptor;
    AjPTable CacheByIdentifier;
    AjPTable CacheByName;
} EnsOQcdatabaseadaptor;

#define EnsPQcdatabaseadaptor EnsOQcdatabaseadaptor*




/* EnsEQcdatabaseClass ********************************************************
**
** Ensembl QC Database Class enumeration
**
******************************************************************************/

typedef enum EnsOQcdatabaseClass
{
    ensEQcdatabaseClassNULL,
    ensEQcdatabaseClassUnknown,
    ensEQcdatabaseClassReference,
    ensEQcdatabaseClassTest,
    ensEQcdatabaseClassBoth,
    ensEQcdatabaseClassGenome
} EnsEQcdatabaseClass;




/* EnsEQcdatabaseType *********************************************************
**
** Ensembl QC Database Type enumeration
**
******************************************************************************/

typedef enum EnsOQcdatabaseType
{
    ensEQcdatabaseTypeNULL,
    ensEQcdatabaseTypeUnknown,
    ensEQcdatabaseTypeDNA,
    ensEQcdatabaseTypeProtein
} EnsEQcdatabaseType;




/* @data EnsPQcdatabase *******************************************************
**
** Ensembl QC Database
**
** @alias EnsSQcdatabase
** @alias EnsOQcdatabase
**
** @attr Use [ajuint] Use counter
** @cc Bio::EnsEMBL::Storable
** @attr Identifier [ajuint] SQL database-internal identifier
** @attr Adaptor [EnsPQcdatabaseadaptor] Ensembl Database Adaptor
** @cc Bio::EnsEMBL::QC::SequenceDB
** @cc 'sequence_db' SQL table
** @attr Analysis [EnsPAnalysis] Ensembl Analysis
** @attr Name [AjPStr] Name
** @attr Release [AjPStr] Release
** @attr Date [AjPStr] Date
** @attr Format [AjPStr] Format
** @attr Class [EnsEQcdatabaseClass] Class
** @attr Type [EnsEQcdatabaseType] Type
** @attr Species [AjPStr] Registry species
** @attr Group [EnsEDatabaseadaptorGroup] Ensembl Database Adaptor group
** @attr Padding [ajuint] Padding to alignment boundary
** @attr Host [AjPStr] Host
** @attr Directory [AjPStr] Directory
** @attr File [AjPStr] File
** @attr ExternalURL [AjPStr] External URL
** @attr InternalURL [AjPStr] Internal URL
** @@
******************************************************************************/

typedef struct EnsSQcdatabase
{
    ajuint Use;
    ajuint Identifier;
    EnsPQcdatabaseadaptor Adaptor;
    EnsPAnalysis Analysis;
    AjPStr Name;
    AjPStr Release;
    AjPStr Date;
    AjPStr Format;
    EnsEQcdatabaseClass Class;
    EnsEQcdatabaseType Type;
    AjPStr Species;
    EnsEDatabaseadaptorGroup Group;
    ajuint Padding;
    AjPStr Host;
    AjPStr Directory;
    AjPStr File;
    AjPStr ExternalURL;
    AjPStr InternalURL;
} EnsOQcdatabase;

#define EnsPQcdatabase EnsOQcdatabase*




/*
** Prototype definitions
*/

/* Ensembl Quality Check Database */

EnsPQcdatabase ensQcdatabaseNew(EnsPQcdatabaseadaptor qcdba,
                                ajuint identifier,
                                EnsPAnalysis analysis,
                                AjPStr name,
                                AjPStr release,
                                AjPStr date,
                                AjPStr format,
                                EnsEQcdatabaseClass class,
                                EnsEQcdatabaseType type,
                                AjPStr species,
                                EnsEDatabaseadaptorGroup group,
                                AjPStr host,
                                AjPStr directory,
                                AjPStr file,
                                AjPStr externalurl);

EnsPQcdatabase ensQcdatabaseNewObj(const EnsPQcdatabase object);

EnsPQcdatabase ensQcdatabaseNewRef(EnsPQcdatabase qcdb);

void ensQcdatabaseDel(EnsPQcdatabase* Pqcdb);

EnsPQcdatabaseadaptor ensQcdatabaseGetAdaptor(const EnsPQcdatabase qcdb);

ajuint ensQcdatabaseGetIdentifier(const EnsPQcdatabase qcdb);

const EnsPAnalysis ensQcdatabaseGetAnalysis(const EnsPQcdatabase qcdb);

AjPStr ensQcdatabaseGetName(const EnsPQcdatabase qcdb);

AjPStr ensQcdatabaseGetRelease(const EnsPQcdatabase qcdb);

AjPStr ensQcdatabaseGetDate(const EnsPQcdatabase qcdb);

AjPStr ensQcdatabaseGetFormat(const EnsPQcdatabase qcdb);

EnsEQcdatabaseClass ensQcdatabaseGetClass(const EnsPQcdatabase qcdb);

EnsEQcdatabaseType ensQcdatabaseGetType(const EnsPQcdatabase qcdb);

AjPStr ensQcdatabaseGetSpecies(const EnsPQcdatabase qcdb);

EnsEDatabaseadaptorGroup ensQcdatabaseGetGroup(const EnsPQcdatabase qcdb);

AjPStr ensQcdatabaseGetHost(const EnsPQcdatabase qcdb);

AjPStr ensQcdatabaseGetDirectory(const EnsPQcdatabase qcdb);

AjPStr ensQcdatabaseGetFile(const EnsPQcdatabase qcdb);

AjPStr ensQcdatabaseGetExternalURL(const EnsPQcdatabase qcdb);

AjPStr ensQcdatabaseGetInternalURL(const EnsPQcdatabase qcdb);

AjBool ensQcdatabaseSetAdaptor(EnsPQcdatabase qcdb,
                               EnsPQcdatabaseadaptor qcdba);

AjBool ensQcdatabaseSetIdentifier(EnsPQcdatabase qcdb, ajuint identifier);

AjBool ensQcdatabaseSetAnalysis(EnsPQcdatabase qcdb, EnsPAnalysis analysis);

AjBool ensQcdatabaseSetName(EnsPQcdatabase qcdb, AjPStr name);

AjBool ensQcdatabaseSetRelease(EnsPQcdatabase qcdb, AjPStr release);

AjBool ensQcdatabaseSetDate(EnsPQcdatabase qcdb, AjPStr date);

AjBool ensQcdatabaseSetFormat(EnsPQcdatabase qcdb, AjPStr format);

AjBool ensQcdatabaseSetClass(EnsPQcdatabase qcdb, EnsEQcdatabaseClass class);

AjBool ensQcdatabaseSetType(EnsPQcdatabase qcdb, EnsEQcdatabaseType type);

AjBool ensQcdatabaseSetSpecies(EnsPQcdatabase qcdb, AjPStr species);

AjBool ensQcdatabaseSetGroup(EnsPQcdatabase qcdb,
                             EnsEDatabaseadaptorGroup group);

AjBool ensQcdatabaseSetHost(EnsPQcdatabase qcdb, AjPStr host);

AjBool ensQcdatabaseSetDirectory(EnsPQcdatabase qcdb, AjPStr directory);

AjBool ensQcdatabaseSetFile(EnsPQcdatabase qcdb, AjPStr file);

AjBool ensQcdatabaseSetExternalURL(EnsPQcdatabase qcdb, AjPStr url);

AjBool ensQcdatabaseSetInternalURL(EnsPQcdatabase qcdb, AjPStr url);

ajulong ensQcdatabaseGetMemsize(const EnsPQcdatabase qcdb);

AjBool ensQcdatabaseTrace(const EnsPQcdatabase qcdb, ajuint level);

EnsEQcdatabaseClass ensQcdatabaseClassFromStr(const AjPStr class);

EnsEQcdatabaseType ensQcdatabaseTypeFromStr(const AjPStr type);

const char *ensQcdatabaseClassToChar(EnsEQcdatabaseClass class);

const char *ensQcdatabaseTypeToChar(EnsEQcdatabaseType type);

AjBool ensQcdatabaseMatch(const EnsPQcdatabase qcdb1,
                          const EnsPQcdatabase qcdb2);

/* Ensembl Quality Check Database Adaptor */

EnsPQcdatabaseadaptor ensRegistryGetQcdatabaseadaptor(
    EnsPDatabaseadaptor dba);

EnsPQcdatabaseadaptor ensQcdatabaseadaptorNew(
    EnsPDatabaseadaptor dba);

void ensQcdatabaseadaptorDel(EnsPQcdatabaseadaptor* Pqcdba);

AjBool ensQcdatabaseadaptorFetchByIdentifier(EnsPQcdatabaseadaptor qcdba,
                                             ajuint identifier,
                                             EnsPQcdatabase *Pqcdb);

AjBool ensQcdatabaseadaptorFetchByName(EnsPQcdatabaseadaptor qcdba,
                                       const AjPStr name,
                                       const AjPStr release,
                                       EnsPQcdatabase *Pqcdb);

AjBool ensQcdatabaseadaptorFetchAllByClassType(EnsPQcdatabaseadaptor qcdba,
                                               EnsEQcdatabaseClass class,
                                               EnsEQcdatabaseType type,
                                               AjPList qcdbs);

AjBool ensQcdatabaseadaptorStore(EnsPQcdatabaseadaptor qcdba,
                                 EnsPQcdatabase qcdb);

AjBool ensQcdatabaseadaptorUpdate(EnsPQcdatabaseadaptor qcdba,
                                  const EnsPQcdatabase qcdb);

AjBool ensQcdatabaseadaptorDelete(EnsPQcdatabaseadaptor qcdba,
                                  EnsPQcdatabase qcdb);


/*
** End of prototype definitions
*/




#endif /* ensqcdatabase_h */

#ifdef __cplusplus
}
#endif
