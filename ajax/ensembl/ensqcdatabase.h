/* @include ensqcdatabase *****************************************************
**
** Ensembl Quality Check Database functions
**
** @author Copyright (C) 1999 Ensembl Developers
** @author Copyright (C) 2006 Michael K. Schuster
** @version $Revision: 1.19 $
** @modified 2009 by Alan Bleasby for incorporation into EMBOSS core
** @modified $Date: 2012/02/04 10:30:24 $ by $Author: mks $
** @@
**
** This library is free software; you can redistribute it and/or
** modify it under the terms of the GNU Lesser General Public
** License as published by the Free Software Foundation; either
** version 2.1 of the License, or (at your option) any later version.
**
** This library is distributed in the hope that it will be useful,
** but WITHOUT ANY WARRANTY; without even the implied warranty of
** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
** Lesser General Public License for more details.
**
** You should have received a copy of the GNU Lesser General Public
** License along with this library; if not, write to the Free Software
** Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,
** MA  02110-1301,  USA.
**
******************************************************************************/

#ifndef ENSQCDATABASE_H
#define ENSQCDATABASE_H

/* ========================================================================= */
/* ============================= include files ============================= */
/* ========================================================================= */

#include "ensanalysis.h"

AJ_BEGIN_DECLS




/* ========================================================================= */
/* =============================== constants =============================== */
/* ========================================================================= */

/* @enum EnsEQcdatabaseClass **************************************************
**
** Ensembl Quality Check Database Class enumeration
**
** @value ensEQcdatabaseClassNULL Null
** @value ensEQcdatabaseClassUnknown Unknown
** @value ensEQcdatabaseClassReference Reference database
** @value ensEQcdatabaseClassTest Test database
** @value ensEQcdatabaseClassBoth Reference and test database
** @value ensEQcdatabaseClassGenome Genome database
** @@
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




/* @enum EnsEQcdatabaseType ***************************************************
**
** Ensembl Quality Check Database Type enumeration
**
** @value ensEQcdatabaseTypeNULL Null
** @value ensEQcdatabaseTypeUnknown Unknown sequence database
** @value ensEQcdatabaseTypeDNA DNA sequence database
** @value ensEQcdatabaseTypeProtein Protein sequence database
** @@
******************************************************************************/

typedef enum EnsOQcdatabaseType
{
    ensEQcdatabaseTypeNULL,
    ensEQcdatabaseTypeUnknown,
    ensEQcdatabaseTypeDNA,
    ensEQcdatabaseTypeProtein
} EnsEQcdatabaseType;




/* ========================================================================= */
/* ============================== public data ============================== */
/* ========================================================================= */

/* @data EnsPQcdatabaseadaptor ************************************************
**
** Ensembl Quality Check Database Adaptor
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




/* @data EnsPQcdatabase *******************************************************
**
** Ensembl Quality Check Database
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
** @attr DatabaseadaptorSpecies [AjPStr] Ensembl Database Adaptor species
** @attr DatabaseadaptorGroup [EnsEDatabaseadaptorGroup] Ensembl Database
** Adaptor Group enumeration
** @attr Padding [ajuint] Padding to alignment boundary
** @attr Host [AjPStr] Host
** @attr Directory [AjPStr] Directory
** @attr File [AjPStr] File
** @attr UrlExternal [AjPStr] External URL
** @attr UrlInternal [AjPStr] Internal URL
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
    AjPStr DatabaseadaptorSpecies;
    EnsEDatabaseadaptorGroup DatabaseadaptorGroup;
    ajuint Padding;
    AjPStr Host;
    AjPStr Directory;
    AjPStr File;
    AjPStr UrlExternal;
    AjPStr UrlInternal;
} EnsOQcdatabase;

#define EnsPQcdatabase EnsOQcdatabase*




/* ========================================================================= */
/* =========================== public functions ============================ */
/* ========================================================================= */

/*
** Prototype definitions
*/

/* Ensembl Quality Check Database */

EnsPQcdatabase ensQcdatabaseNewCpy(const EnsPQcdatabase qcdb);

EnsPQcdatabase ensQcdatabaseNewIni(EnsPQcdatabaseadaptor qcdba,
                                   ajuint identifier,
                                   EnsPAnalysis analysis,
                                   AjPStr name,
                                   AjPStr release,
                                   AjPStr date,
                                   AjPStr format,
                                   EnsEQcdatabaseClass qcdbc,
                                   EnsEQcdatabaseType qcdbt,
                                   AjPStr dbaspecies,
                                   EnsEDatabaseadaptorGroup dbag,
                                   AjPStr host,
                                   AjPStr directory,
                                   AjPStr file,
                                   AjPStr externalurl);

EnsPQcdatabase ensQcdatabaseNewRef(EnsPQcdatabase qcdb);

void ensQcdatabaseDel(EnsPQcdatabase *Pqcdb);

EnsPQcdatabaseadaptor ensQcdatabaseGetAdaptor(
    const EnsPQcdatabase qcdb);

EnsPAnalysis ensQcdatabaseGetAnalysis(
    const EnsPQcdatabase qcdb);

EnsEQcdatabaseClass ensQcdatabaseGetClass(
    const EnsPQcdatabase qcdb);

EnsEDatabaseadaptorGroup ensQcdatabaseGetDatabaseadaptorGroup(
    const EnsPQcdatabase qcdb);

AjPStr ensQcdatabaseGetDatabaseadaptorSpecies(
    const EnsPQcdatabase qcdb);

AjPStr ensQcdatabaseGetDate(
    const EnsPQcdatabase qcdb);

AjPStr ensQcdatabaseGetDirectory(
    const EnsPQcdatabase qcdb);

AjPStr ensQcdatabaseGetFile(
    const EnsPQcdatabase qcdb);

AjPStr ensQcdatabaseGetFormat(
    const EnsPQcdatabase qcdb);

AjPStr ensQcdatabaseGetHost(
    const EnsPQcdatabase qcdb);

ajuint ensQcdatabaseGetIdentifier(
    const EnsPQcdatabase qcdb);

AjPStr ensQcdatabaseGetName(
    const EnsPQcdatabase qcdb);

AjPStr ensQcdatabaseGetRelease(
    const EnsPQcdatabase qcdb);

EnsEQcdatabaseType ensQcdatabaseGetType(
    const EnsPQcdatabase qcdb);

AjPStr ensQcdatabaseGetUrlExternal(
    const EnsPQcdatabase qcdb);

AjPStr ensQcdatabaseGetUrlInternal(
    const EnsPQcdatabase qcdb);

AjBool ensQcdatabaseSetAdaptor(EnsPQcdatabase qcdb,
                               EnsPQcdatabaseadaptor qcdba);

AjBool ensQcdatabaseSetAnalysis(EnsPQcdatabase qcdb,
                                EnsPAnalysis analysis);

AjBool ensQcdatabaseSetClass(EnsPQcdatabase qcdb,
                             EnsEQcdatabaseClass qcdbc);

AjBool ensQcdatabaseSetDatabaseadaptorGroup(EnsPQcdatabase qcdb,
                                            EnsEDatabaseadaptorGroup dbag);

AjBool ensQcdatabaseSetDatabaseadaptorSpecies(EnsPQcdatabase qcdb,
                                              AjPStr species);

AjBool ensQcdatabaseSetDate(EnsPQcdatabase qcdb,
                            AjPStr date);

AjBool ensQcdatabaseSetDirectory(EnsPQcdatabase qcdb,
                                 AjPStr directory);

AjBool ensQcdatabaseSetFile(EnsPQcdatabase qcdb,
                            AjPStr file);

AjBool ensQcdatabaseSetFormat(EnsPQcdatabase qcdb,
                              AjPStr format);

AjBool ensQcdatabaseSetHost(EnsPQcdatabase qcdb,
                            AjPStr host);

AjBool ensQcdatabaseSetIdentifier(EnsPQcdatabase qcdb,
                                  ajuint identifier);

AjBool ensQcdatabaseSetName(EnsPQcdatabase qcdb,
                            AjPStr name);

AjBool ensQcdatabaseSetRelease(EnsPQcdatabase qcdb,
                               AjPStr release);

AjBool ensQcdatabaseSetType(EnsPQcdatabase qcdb,
                            EnsEQcdatabaseType qcdbt);

AjBool ensQcdatabaseSetUrlExternal(EnsPQcdatabase qcdb,
                                   AjPStr url);

AjBool ensQcdatabaseSetUrlInternal(EnsPQcdatabase qcdb,
                                   AjPStr url);

AjBool ensQcdatabaseTrace(const EnsPQcdatabase qcdb, ajuint level);

size_t ensQcdatabaseCalculateMemsize(const EnsPQcdatabase qcdb);

AjBool ensQcdatabaseMatch(const EnsPQcdatabase qcdb1,
                          const EnsPQcdatabase qcdb2);

EnsEQcdatabaseClass ensQcdatabaseClassFromStr(const AjPStr classstr);

const char *ensQcdatabaseClassToChar(EnsEQcdatabaseClass qcdbc);

EnsEQcdatabaseType ensQcdatabaseTypeFromStr(const AjPStr typestr);

const char *ensQcdatabaseTypeToChar(EnsEQcdatabaseType qcdbt);

/* Ensembl Quality Check Database Adaptor */

EnsPQcdatabaseadaptor ensRegistryGetQcdatabaseadaptor(
    EnsPDatabaseadaptor dba);

EnsPQcdatabaseadaptor ensQcdatabaseadaptorNew(
    EnsPDatabaseadaptor dba);

void ensQcdatabaseadaptorDel(EnsPQcdatabaseadaptor *Pqcdba);

EnsPBaseadaptor ensQcdatabaseadaptorGetBaseadaptor(
    EnsPQcdatabaseadaptor qcdba);

EnsPDatabaseadaptor ensQcdatabaseadaptorGetDatabaseadaptor(
    EnsPQcdatabaseadaptor qcdba);

AjBool ensQcdatabaseadaptorFetchAllbyClasstype(EnsPQcdatabaseadaptor qcdba,
                                               EnsEQcdatabaseClass qcdbc,
                                               EnsEQcdatabaseType qcdbt,
                                               AjPList qcdbs);

AjBool ensQcdatabaseadaptorFetchByIdentifier(EnsPQcdatabaseadaptor qcdba,
                                             ajuint identifier,
                                             EnsPQcdatabase *Pqcdb);

AjBool ensQcdatabaseadaptorFetchByName(EnsPQcdatabaseadaptor qcdba,
                                       const AjPStr name,
                                       const AjPStr release,
                                       EnsPQcdatabase *Pqcdb);

AjBool ensQcdatabaseadaptorDelete(EnsPQcdatabaseadaptor qcdba,
                                  EnsPQcdatabase qcdb);

AjBool ensQcdatabaseadaptorStore(EnsPQcdatabaseadaptor qcdba,
                                 EnsPQcdatabase qcdb);

AjBool ensQcdatabaseadaptorUpdate(EnsPQcdatabaseadaptor qcdba,
                                  const EnsPQcdatabase qcdb);


/*
** End of prototype definitions
*/




AJ_END_DECLS

#endif /* !ENSQCDATABASE_H */
