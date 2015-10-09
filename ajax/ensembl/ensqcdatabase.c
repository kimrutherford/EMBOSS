/* @source ensqcdatabase ******************************************************
**
** Ensembl Quality Check Database functions
**
** @author Copyright (C) 1999 Ensembl Developers
** @author Copyright (C) 2006 Michael K. Schuster
** @version $Revision: 1.47 $
** @modified 2009 by Alan Bleasby for incorporation into EMBOSS core
** @modified $Date: 2012/04/12 20:34:16 $ by $Author: mks $
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

/* ========================================================================= */
/* ============================= include files ============================= */
/* ========================================================================= */

#include "ensqcdatabase.h"
#include "enstable.h"




/* ========================================================================= */
/* =============================== constants =============================== */
/* ========================================================================= */




/* ========================================================================= */
/* =========================== global variables ============================ */
/* ========================================================================= */




/* ========================================================================= */
/* ============================= private data ============================== */
/* ========================================================================= */




/* ========================================================================= */
/* =========================== private constants =========================== */
/* ========================================================================= */

/* @conststatic qcdatabaseKClass **********************************************
**
** The Ensembl Quality Check Database class member is enumerated in
** both, the SQL table definition and the data structure. The following strings
** are used for conversion in database operations and correspond to
** EnsEQcdatabaseClass.
**
******************************************************************************/

static const char *qcdatabaseKClass[] =
{
    "",
    "unknown",
    "reference",
    "test",
    "both",
    "genome",
    (const char *) NULL
};




/* @conststatic qcdatabaseKType ***********************************************
**
** The Ensembl Quality Check Database type member is enumerated in
** both, the SQL table definition and the data structure. The following strings
** are used for conversion in database operations and correspond to
** EnsEQcdatabaseType.
**
******************************************************************************/

static const char *qcdatabaseKType[] =
{
    "",
    "unknown",
    "dna",
    "protein",
    (const char *) NULL
};




/* @conststatic qcdatabaseadaptorKTables **************************************
**
** Array of Ensembl Quality Check Database Adaptor SQL table names
**
******************************************************************************/

static const char *qcdatabaseadaptorKTables[] =
{
    "sequence_db",
    (const char *) NULL
};




/* @conststatic qcdatabaseadaptorKColumns *************************************
**
** Array of Ensembl Quality Check Database Adaptor SQL column names
**
******************************************************************************/

static const char *qcdatabaseadaptorKColumns[] =
{
    "sequence_db.sequence_db_id",
    "sequence_db.analysis_id",
    "sequence_db.name",
    "sequence_db.dbrelease",
    "sequence_db.date",
    "sequence_db.format",
    "sequence_db.type",
    "sequence_db.class",
    "sequence_db.species",
    "sequence_db.dbgroup",
    "sequence_db.host",
    "sequence_db.directory",
    "sequence_db.file",
    "sequence_db.external_url",
    (const char *) NULL
};




/* ========================================================================= */
/* =========================== private variables =========================== */
/* ========================================================================= */




/* ========================================================================= */
/* =========================== private functions =========================== */
/* ========================================================================= */

static AjBool qcdatabaseadaptorFetchAllbyStatement(
    EnsPBaseadaptor ba,
    const AjPStr statement,
    EnsPAssemblymapper am,
    EnsPSlice slice,
    AjPList qcdbs);

static AjBool qcdatabaseadaptorCacheInsert(EnsPQcdatabaseadaptor qcdba,
                                           EnsPQcdatabase *Pqcdb);

static AjBool qcdatabaseadaptorCacheRemove(EnsPQcdatabaseadaptor qcdba,
                                           EnsPQcdatabase qcdb);

static AjBool qcdatabaseadaptorCacheInit(EnsPQcdatabaseadaptor qcdba);




/* ========================================================================= */
/* ======================= All functions by section ======================== */
/* ========================================================================= */




/* @filesection ensqcdatabase *************************************************
**
** @nam1rule ens Function belongs to the Ensembl library
**
******************************************************************************/




/* @datasection [EnsPQcdatabase] Ensembl Quality Check Database ***************
**
** @nam2rule Qcdatabase Functions for manipulating
** Ensembl Quality Check Database objects
**
** @cc Bio::EnsEMBL::QC::SequenceDB
** @cc CVS Revision:
** @cc CVS Tag:
**
******************************************************************************/




/* @section constructors ******************************************************
**
** All constructors return a new Ensembl Quality Check Database by pointer.
** It is the responsibility of the user to first destroy any previous
** Quality Check Database. The target pointer does not need to be initialised
** to NULL, but it is good programming practice to do so anyway.
**
** @fdata [EnsPQcdatabase]
**
** @nam3rule New Constructor
** @nam4rule Cpy Constructor with existing object
** @nam4rule Ini Constructor with initial values
** @nam4rule Ref Constructor by incrementing the reference counter
**
** @argrule Cpy qcdb [const EnsPQcdatabase] Ensembl Quality Check Database
** @argrule Ini qcdba [EnsPQcdatabaseadaptor]
** Ensembl Quality Check Database Adaptor
** @argrule Ini identifier [ajuint] SQL database-internal identifier
** @argrule Ini analysis [EnsPAnalysis] Ensembl Analysis
** @argrule Ini name [AjPStr] Name
** @argrule Ini release [AjPStr] Release
** @argrule Ini date [AjPStr] Date
** @argrule Ini format [AjPStr] Format
** @argrule Ini qcdbc [EnsEQcdatabaseClass]
** Ensembl Quality Check Database Class enumeration
** @argrule Ini qcdbt [EnsEQcdatabaseType]
** Ensembl Quality Check Database Type enumeration
** @argrule Ini dbaspecies [AjPStr] Ensembl Database Adaptor species
** @argrule Ini dbag [EnsEDatabaseadaptorGroup]
** Ensembl Database Adaptor Group enumeration
** @argrule Ini host [AjPStr] Host
** @argrule Ini directory [AjPStr] Directory
** @argrule Ini file [AjPStr] File
** @argrule Ini externalurl [AjPStr] External URL
** @argrule Ref qcdb [EnsPQcdatabase] Ensembl Quality Check Database
**
** @valrule * [EnsPQcdatabase] Ensembl Quality Check Database or NULL
**
** @fcategory new
******************************************************************************/




/* @func ensQcdatabaseNewCpy **************************************************
**
** Object-based constructor function, which returns an independent object.
**
** @param [r] qcdb [const EnsPQcdatabase] Ensembl Quality Check Database
**
** @return [EnsPQcdatabase] Ensembl Quality Check Database or NULL
**
** @release 6.4.0
** @@
******************************************************************************/

EnsPQcdatabase ensQcdatabaseNewCpy(const EnsPQcdatabase qcdb)
{
    EnsPQcdatabase pthis = NULL;

    if (!qcdb)
        return NULL;

    AJNEW0(pthis);

    pthis->Use        = 1U;
    pthis->Identifier = qcdb->Identifier;
    pthis->Adaptor    = qcdb->Adaptor;

    if (qcdb->Analysis)
        pthis->Analysis = ensAnalysisNewRef(qcdb->Analysis);

    if (qcdb->Name)
        pthis->Name = ajStrNewRef(qcdb->Name);

    if (qcdb->Release)
        pthis->Release = ajStrNewRef(qcdb->Release);

    if (qcdb->Date)
        pthis->Date = ajStrNewRef(qcdb->Date);

    if (qcdb->Format)
        pthis->Format = ajStrNewRef(qcdb->Format);

    pthis->Class = qcdb->Class;
    pthis->Type  = qcdb->Type;

    if (qcdb->DatabaseadaptorSpecies)
        pthis->DatabaseadaptorSpecies = ajStrNewRef(
            qcdb->DatabaseadaptorSpecies);

    pthis->DatabaseadaptorGroup = qcdb->DatabaseadaptorGroup;

    if (qcdb->Host)
        pthis->Host = ajStrNewRef(qcdb->Host);

    if (qcdb->Directory)
        pthis->Directory = ajStrNewRef(qcdb->Directory);

    if (qcdb->File)
        pthis->File = ajStrNewRef(qcdb->File);

    if (qcdb->UrlExternal)
        pthis->UrlExternal = ajStrNewRef(qcdb->UrlExternal);

    if (qcdb->UrlInternal)
        pthis->UrlInternal = ajStrNewRef(qcdb->UrlInternal);

    return pthis;
}




/* @func ensQcdatabaseNewIni **************************************************
**
** Constructor for an Ensembl Quality Check Database with initial values.
**
** @cc Bio::EnsEMBL::Storable::new
** @param [u] qcdba [EnsPQcdatabaseadaptor]
** Ensembl Quality Check Database Adaptor
** @param [r] identifier [ajuint] SQL database-internal identifier
** @cc Bio::EnsEMBL::QC::SequenceDB::new
** @param [u] analysis [EnsPAnalysis] Ensembl Analysis
** @param [u] name [AjPStr] Name
** @param [u] release [AjPStr] Release
** @param [u] date [AjPStr] Date
** @param [u] format [AjPStr] Format
** @param [u] qcdbc [EnsEQcdatabaseClass]
** Ensembl Quality Check Database Class enumeration
** @param [u] qcdbt [EnsEQcdatabaseType]
** Ensembl Quality Check Database Type enumeration
** @param [u] dbaspecies [AjPStr] Ensembl Database Adaptor species
** @param [u] dbag [EnsEDatabaseadaptorGroup]
** Ensembl Database Adaptor Group enumeration
** @param [u] host [AjPStr] Host
** @param [u] directory [AjPStr] Directory
** @param [u] file [AjPStr] File
** @param [u] externalurl [AjPStr] External URL
**
** @return [EnsPQcdatabase] Ensembl Quality Check Database or NULL
**
** @release 6.4.0
** @@
******************************************************************************/

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
                                   AjPStr externalurl)
{
    EnsPQcdatabase qcdb = NULL;

    if (!analysis)
        return NULL;

    if (!name)
        return NULL;

    if (!release)
        return NULL;

    AJNEW0(qcdb);

    qcdb->Use        = 1U;
    qcdb->Identifier = identifier;
    qcdb->Adaptor    = qcdba;
    qcdb->Analysis   = ensAnalysisNewRef(analysis);

    if (name)
        qcdb->Name = ajStrNewRef(name);

    if (release)
        qcdb->Release = ajStrNewRef(release);

    if (date)
        qcdb->Date = ajStrNewRef(date);

    if (format)
        qcdb->Format = ajStrNewRef(format);

    qcdb->Class = qcdbc;

    qcdb->Type = qcdbt;

    if (dbaspecies)
        qcdb->DatabaseadaptorSpecies = ajStrNewRef(dbaspecies);

    qcdb->DatabaseadaptorGroup = dbag;

    if (host)
        qcdb->Host = ajStrNewRef(host);

    if (directory)
        qcdb->Directory = ajStrNewRef(directory);

    if (file)
        qcdb->File = ajStrNewRef(file);

    if (externalurl)
        qcdb->UrlExternal = ajStrNewRef(externalurl);

    qcdb->UrlInternal = NULL;

    return qcdb;
}




/* @func ensQcdatabaseNewRef **************************************************
**
** Ensembl Object referencing function, which returns a pointer to the
** Ensembl Object passed in and increases its reference count.
**
** @param [u] qcdb [EnsPQcdatabase] Ensembl Quality Check Database
**
** @return [EnsPQcdatabase] Ensembl Quality Check Database or NULL
**
** @release 6.2.0
** @@
******************************************************************************/

EnsPQcdatabase ensQcdatabaseNewRef(EnsPQcdatabase qcdb)
{
    if (!qcdb)
        return NULL;

    qcdb->Use++;

    return qcdb;
}




/* @section destructors *******************************************************
**
** Destruction destroys all internal data structures and frees the memory
** allocated for an Ensembl Quality Check Database object.
**
** @fdata [EnsPQcdatabase]
**
** @nam3rule Del Destroy (free) an Ensembl Quality Check Database
**
** @argrule * Pqcdb [EnsPQcdatabase*] Ensembl Quality Check Database address
**
** @valrule * [void]
**
** @fcategory delete
******************************************************************************/




/* @func ensQcdatabaseDel *****************************************************
**
** Default destructor for an Ensembl Quality Check Database.
**
** @param [d] Pqcdb [EnsPQcdatabase*] Ensembl Quality Check Database address
**
** @return [void]
**
** @release 6.2.0
** @@
******************************************************************************/

void ensQcdatabaseDel(EnsPQcdatabase *Pqcdb)
{
    EnsPQcdatabase pthis = NULL;

    if (!Pqcdb)
        return;

#if defined(AJ_DEBUG) && AJ_DEBUG >= 1
    if (ajDebugTest("ensQcdatabaseDel"))
    {
        ajDebug("ensQcdatabaseDel\n"
                "  *Pqcdb %p\n",
                *Pqcdb);

        ensQcdatabaseTrace(*Pqcdb, 1);
    }
#endif /* defined(AJ_DEBUG) && AJ_DEBUG >= 1 */

    if (!*Pqcdb)
        return;

    pthis = *Pqcdb;

    pthis->Use--;

    if (pthis->Use)
    {
        *Pqcdb = NULL;

        return;
    }

    ensAnalysisDel(&pthis->Analysis);

    ajStrDel(&pthis->Name);
    ajStrDel(&pthis->Release);
    ajStrDel(&pthis->Date);
    ajStrDel(&pthis->Format);
    ajStrDel(&pthis->DatabaseadaptorSpecies);
    ajStrDel(&pthis->Host);
    ajStrDel(&pthis->Directory);
    ajStrDel(&pthis->File);
    ajStrDel(&pthis->UrlExternal);
    ajStrDel(&pthis->UrlInternal);

    AJFREE(pthis);

    *Pqcdb = NULL;

    return;
}




/* @section member retrieval **************************************************
**
** Functions for returning members of an
** Ensembl Quality Check Database object.
**
** @fdata [EnsPQcdatabase]
**
** @nam3rule Get Return Ensembl Quality Check Database attribute(s)
** @nam4rule Adaptor Return the Ensembl Quality Check Database Adaptor
** @nam4rule Analysis Return the Ensembl Analysis
** @nam4rule Class Return the class
** @nam4rule Databaseadaptor Return Ensembl Database Adaptor attribute(s)
** @nam5rule Group Return the Ensembl Database Adaptor Group
** @nam5rule Species Return the Ensembl Database Adaptor species
** @nam4rule Date Return the date
** @nam4rule Directory Return the directory
** @nam4rule File Return the file
** @nam4rule Format return the format
** @nam4rule Host Return the host
** @nam4rule Identifier Return the SQL database-internal identifier
** @nam4rule Name Return the name
** @nam4rule Release Return the release
** @nam4rule Type Return the type
** @nam4rule Url Return a Uniform Resource Locator
** @nam5rule External Return the external URL
** @nam5rule Internal Return the internal URL
**
** @argrule * qcdb [const EnsPQcdatabase] Ensembl Quality Check Database
**
** @valrule Adaptor [EnsPQcdatabaseadaptor]
** Ensembl Quality Check Database Adaptor or NULL
** @valrule Analysis [EnsPAnalysis] Ensembl Analysis or NULL
** @valrule Class [EnsEQcdatabaseClass] Class or ensEQcdatabaseClassNULL
** @valrule DatabaseadaptorGroup [EnsEDatabaseadaptorGroup]
** Ensembl Database Adaptor Group enumeration or ensEDatabaseadaptorGroupNULL
** @valrule DatabaseadaptorSpecies [AjPStr]
** Ensembl Database Adaptor species or NULL
** @valrule Date [AjPStr] Date or NULL
** @valrule Directory [AjPStr] Directory or NULL
** @valrule File [AjPStr] File or NULL
** @valrule Format [AjPStr] Format or NULL
** @valrule Host [AjPStr] Host or NULL
** @valrule Identifier [ajuint] SQL database-internal identifier or 0U
** @valrule Name [AjPStr] Name or NULL
** @valrule Release [AjPStr] Release or NULL
** @valrule Type [EnsEQcdatabaseType] Type or ensEQcdatabaseTypeNULL
** @valrule UrlExternal [AjPStr] External URL or NULL
** @valrule UrlInternal [AjPStr] Internal URL or NULL
**
** @fcategory use
******************************************************************************/




/* @func ensQcdatabaseGetAdaptor **********************************************
**
** Get the Ensembl Quality Check Database Adaptor member of an
** Ensembl Quality Check Database.
**
** @param [r] qcdb [const EnsPQcdatabase] Ensembl Quality Check Database
**
** @return [EnsPQcdatabaseadaptor] Ensembl Quality Check Database Adaptor
** or NULL
**
** @release 6.2.0
** @@
******************************************************************************/

EnsPQcdatabaseadaptor ensQcdatabaseGetAdaptor(
    const EnsPQcdatabase qcdb)
{
    return (qcdb) ? qcdb->Adaptor : NULL;
}




/* @func ensQcdatabaseGetAnalysis *********************************************
**
** Get the Ensembl Analysis member of an Ensembl Quality Check Database.
**
** @param [r] qcdb [const EnsPQcdatabase] Ensembl Quality Check Database
**
** @return [EnsPAnalysis] Ensembl Analysis or NULL
**
** @release 6.2.0
** @@
******************************************************************************/

EnsPAnalysis ensQcdatabaseGetAnalysis(
    const EnsPQcdatabase qcdb)
{
    return (qcdb) ? qcdb->Analysis : NULL;
}




/* @func ensQcdatabaseGetClass ************************************************
**
** Get the class member of an Ensembl Quality Check Database.
**
** @param [r] qcdb [const EnsPQcdatabase] Ensembl Quality Check Database
**
** @return [EnsEQcdatabaseClass] Class or ensEQcdatabaseClassNULL
**
** @release 6.2.0
** @@
******************************************************************************/

EnsEQcdatabaseClass ensQcdatabaseGetClass(
    const EnsPQcdatabase qcdb)
{
    return (qcdb) ? qcdb->Class : ensEQcdatabaseClassNULL;
}




/* @func ensQcdatabaseGetDatabaseadaptorGroup *********************************
**
** Get the Ensembl Database Adaptor Group enumeration member of an
** Ensembl Quality Check Database.
**
** @param [r] qcdb [const EnsPQcdatabase] Ensembl Quality Check Database
**
** @return [EnsEDatabaseadaptorGroup]
** Ensembl Database Adaptor Group enumeration or ensEDatabaseadaptorGroupNULL
**
** @release 6.4.0
** @@
******************************************************************************/

EnsEDatabaseadaptorGroup ensQcdatabaseGetDatabaseadaptorGroup(
    const EnsPQcdatabase qcdb)
{
    return (qcdb) ? qcdb->DatabaseadaptorGroup : ensEDatabaseadaptorGroupNULL;
}




/* @func ensQcdatabaseGetDatabaseadaptorSpecies *******************************
**
** Get the Ensembl Database Adaptor species member of an
** Ensembl Quality Check Database.
**
** @param [r] qcdb [const EnsPQcdatabase] Ensembl Quality Check Database
**
** @return [AjPStr] Ensembl Database Adaptor species or NULL
**
** @release 6.4.0
** @@
******************************************************************************/

AjPStr ensQcdatabaseGetDatabaseadaptorSpecies(
    const EnsPQcdatabase qcdb)
{
    return (qcdb) ? qcdb->DatabaseadaptorSpecies : NULL;
}




/* @func ensQcdatabaseGetDate *************************************************
**
** Get the date member of an Ensembl Quality Check Database.
**
** @param [r] qcdb [const EnsPQcdatabase] Ensembl Quality Check Database
**
** @return [AjPStr] Date or NULL
**
** @release 6.2.0
** @@
******************************************************************************/

AjPStr ensQcdatabaseGetDate(
    const EnsPQcdatabase qcdb)
{
    return (qcdb) ? qcdb->Date : NULL;
}




/* @func ensQcdatabaseGetDirectory ********************************************
**
** Get the directory member of an Ensembl Quality Check Database.
**
** @param [r] qcdb [const EnsPQcdatabase] Ensembl Quality Check Database
**
** @return [AjPStr] Directory or NULL
**
** @release 6.2.0
** @@
******************************************************************************/

AjPStr ensQcdatabaseGetDirectory(
    const EnsPQcdatabase qcdb)
{
    return (qcdb) ? qcdb->Directory : NULL;
}




/* @func ensQcdatabaseGetFile *************************************************
**
** Get the file member of an Ensembl Quality Check Database.
**
** @param [r] qcdb [const EnsPQcdatabase] Ensembl Quality Check Database
**
** @return [AjPStr] File or NULL
**
** @release 6.2.0
** @@
******************************************************************************/

AjPStr ensQcdatabaseGetFile(
    const EnsPQcdatabase qcdb)
{
    return (qcdb) ? qcdb->File : NULL;
}




/* @func ensQcdatabaseGetFormat ***********************************************
**
** Get the format member of an Ensembl Quality Check Database.
**
** @param [r] qcdb [const EnsPQcdatabase] Ensembl Quality Check Database
**
** @return [AjPStr] Format
**
** @release 6.2.0
** @@
******************************************************************************/

AjPStr ensQcdatabaseGetFormat(
    const EnsPQcdatabase qcdb)
{
    return (qcdb) ? qcdb->Format : NULL;
}




/* @func ensQcdatabaseGetHost *************************************************
**
** Get the host member of an Ensembl Quality Check Database.
**
** @param [r] qcdb [const EnsPQcdatabase] Ensembl Quality Check Database
**
** @return [AjPStr] Host or NULL
**
** @release 6.2.0
** @@
******************************************************************************/

AjPStr ensQcdatabaseGetHost(
    const EnsPQcdatabase qcdb)
{
    return (qcdb) ? qcdb->Host : NULL;
}




/* @func ensQcdatabaseGetIdentifier *******************************************
**
** Get the SQL database-internal identifier member of an
** Ensembl Quality Check Database.
**
** @param [r] qcdb [const EnsPQcdatabase] Ensembl Quality Check Database
**
** @return [ajuint] SQL database-internal identifier or 0U
**
** @release 6.2.0
** @@
******************************************************************************/

ajuint ensQcdatabaseGetIdentifier(
    const EnsPQcdatabase qcdb)
{
    return (qcdb) ? qcdb->Identifier : 0U;
}




/* @func ensQcdatabaseGetName *************************************************
**
** Get the name member of an Ensembl Quality Check Database.
**
** @param [r] qcdb [const EnsPQcdatabase] Ensembl Quality Check Database
**
** @return [AjPStr] Name or NULL
**
** @release 6.2.0
** @@
******************************************************************************/

AjPStr ensQcdatabaseGetName(
    const EnsPQcdatabase qcdb)
{
    return (qcdb) ? qcdb->Name : NULL;
}




/* @func ensQcdatabaseGetRelease **********************************************
**
** Get the release member of an Ensembl Quality Check Database.
**
** @param [r] qcdb [const EnsPQcdatabase] Ensembl Quality Check Database
**
** @return [AjPStr] Release or NULL
**
** @release 6.2.0
** @@
******************************************************************************/

AjPStr ensQcdatabaseGetRelease(
    const EnsPQcdatabase qcdb)
{
    return (qcdb) ? qcdb->Release : NULL;
}




/* @func ensQcdatabaseGetType *************************************************
**
** Get the type member of an Ensembl Quality Check Database.
**
** @param [r] qcdb [const EnsPQcdatabase] Ensembl Quality Check Database
**
** @return [EnsEQcdatabaseType] Type or ensEQcdasfeatureTypeNULL
**
** @release 6.2.0
** @@
******************************************************************************/

EnsEQcdatabaseType ensQcdatabaseGetType(
    const EnsPQcdatabase qcdb)
{
    return (qcdb) ? qcdb->Type : ensEQcdatabaseTypeNULL;
}




/* @func ensQcdatabaseGetUrlExternal ******************************************
**
** Get the external Uniform Resource Locator (URL) member of an
** Ensembl Quality Check Database.
**
** @param [r] qcdb [const EnsPQcdatabase] Ensembl Quality Check Database
**
** @return [AjPStr] External URL or NULL
**
** @release 6.4.0
** @@
******************************************************************************/

AjPStr ensQcdatabaseGetUrlExternal(
    const EnsPQcdatabase qcdb)
{
    return (qcdb) ? qcdb->UrlExternal : NULL;
}




/* @func ensQcdatabaseGetUrlInternal ******************************************
**
** Get the internal Uniform Resource Locator (URL) member of an
** Ensembl Quality Check Database.
**
** @param [r] qcdb [const EnsPQcdatabase] Ensembl Quality Check Database
**
** @return [AjPStr] Internal URL or NULL
**
** @release 6.4.0
** @@
******************************************************************************/

AjPStr ensQcdatabaseGetUrlInternal(
    const EnsPQcdatabase qcdb)
{
    return (qcdb) ? qcdb->UrlInternal : NULL;
}




/* @section modifiers *********************************************************
**
** Functions for assigning members of an
** Ensembl Quality Check Database object.
**
** @fdata [EnsPQcdatabase]
**
** @nam3rule Set Set one member of an Ensembl Quality Check Database
** @nam4rule Adaptor Set the Ensembl Quality Check Database Adaptor
** @nam4rule Analysis Set the Ensembl Analysis
** @nam4rule Class Set the class
** @nam4rule Databaseadaptor Set Ensembl Database Adaptor members
** @nam5rule Group Set the Ensembl Database Adaptor group
** @nam5rule Species Set the Ensembl Database Adaptor species
** @nam4rule Date Set the date
** @nam4rule Directory Set the directory
** @nam4rule File Set the file
** @nam4rule Format Set the format
** @nam4rule Host Set the host
** @nam4rule Identifier Set the SQL database-internal identifier
** @nam4rule Name Set the name
** @nam4rule Release Set the release
** @nam4rule Type Set the type
** @nam4rule Url Set a Uniform Resource Locator
** @nam5rule External Set the external URL
** @nam5rule Internal Set the internal URL
**
** @argrule * qcdb [EnsPQcdatabase] Ensembl Quality Check Database
** @argrule Adaptor qcdba [EnsPQcdatabaseadaptor]
** Ensembl Quality Check Database Adaptor
** @argrule Analysis analysis [EnsPAnalysis] Ensembl Analysis
** @argrule Class qcdbc [EnsEQcdatabaseClass] Class
** @argrule DatabaseadaptorGroup dbag [EnsEDatabaseadaptorGroup]
** Ensembl Database Adaptor Group enumeration
** @argrule DatabaseadaptorSpecies dbaspecies [AjPStr] Ensembl Database Adaptor
** species
** @argrule Date date [AjPStr] Date
** @argrule Directory directory [AjPStr] Directory
** @argrule File file [AjPStr] File
** @argrule Format format [AjPStr] Format
** @argrule Host host [AjPStr] Host
** @argrule Identifier identifier [ajuint] SQL database-internal identifier
** @argrule Name name [AjPStr] Name
** @argrule Release release [AjPStr] Release
** @argrule Type qcdbt [EnsEQcdatabaseType] Type
** @argrule UrlExternal url [AjPStr] External URL
** @argrule UrlInternal url [AjPStr] Internal URL
**
** @valrule * [AjBool] ajTrue upon success, ajFalse otherwise
**
** @fcategory modify
******************************************************************************/




/* @func ensQcdatabaseSetAdaptor **********************************************
**
** Set the Ensembl Quality Check Database Adaptor member of an
** Ensembl Quality Check Database.
**
** @param [u] qcdb [EnsPQcdatabase] Ensembl Quality Check Database
** @param [u] qcdba [EnsPQcdatabaseadaptor]
** Ensembl Quality Check Database Adaptor
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
**
** @release 6.2.0
** @@
******************************************************************************/

AjBool ensQcdatabaseSetAdaptor(EnsPQcdatabase qcdb,
                               EnsPQcdatabaseadaptor qcdba)
{
    if (!qcdb)
        return ajFalse;

    qcdb->Adaptor = qcdba;

    return ajTrue;
}




/* @func ensQcdatabaseSetAnalysis *********************************************
**
** Set the Ensembl Analysis member of an Ensembl Quality Check Database.
**
** @param [u] qcdb [EnsPQcdatabase] Ensembl Quality Check Database
** @param [u] analysis [EnsPAnalysis] Ensembl Analysis
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
**
** @release 6.2.0
** @@
******************************************************************************/

AjBool ensQcdatabaseSetAnalysis(EnsPQcdatabase qcdb,
                                EnsPAnalysis analysis)
{
    if (!qcdb)
        return ajFalse;

    ensAnalysisDel(&qcdb->Analysis);

    qcdb->Analysis = ensAnalysisNewRef(analysis);

    return ajTrue;
}




/* @func ensQcdatabaseSetClass ************************************************
**
** Set the class member of an Ensembl Quality Check Database.
**
** @param [u] qcdb [EnsPQcdatabase] Ensembl Quality Check Database
** @param [u] qcdbc [EnsEQcdatabaseClass] Class
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
**
** @release 6.2.0
** @@
******************************************************************************/

AjBool ensQcdatabaseSetClass(EnsPQcdatabase qcdb,
                             EnsEQcdatabaseClass qcdbc)
{
    if (!qcdb)
        return ajFalse;

    qcdb->Class = qcdbc;

    return ajTrue;
}




/* @func ensQcdatabaseSetDatabaseadaptorGroup *********************************
**
** Set the Ensembl Database Adaptor Group enumeration member of an
** Ensembl Quality Check Database.
**
** @param [u] qcdb [EnsPQcdatabase] Ensembl Quality Check Database
** @param [u] dbag [EnsEDatabaseadaptorGroup]
** Ensembl Database Adaptor Group enumeration
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
**
** @release 6.4.0
** @@
******************************************************************************/

AjBool ensQcdatabaseSetDatabaseadaptorGroup(EnsPQcdatabase qcdb,
                                            EnsEDatabaseadaptorGroup dbag)
{
    if (!qcdb)
        return ajFalse;

    qcdb->DatabaseadaptorGroup = dbag;

    return ajTrue;
}




/* @func ensQcdatabaseSetDatabaseadaptorSpecies *******************************
**
** Set the Ensembl Database Adaptor species member of an
** Ensembl Quality Check Database.
**
** @param [u] qcdb [EnsPQcdatabase] Ensembl Quality Check Database
** @param [u] dbaspecies [AjPStr] Ensembl Database Adaptor species
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
**
** @release 6.4.0
** @@
******************************************************************************/

AjBool ensQcdatabaseSetDatabaseadaptorSpecies(EnsPQcdatabase qcdb,
                                              AjPStr dbaspecies)
{
    if (!qcdb)
        return ajFalse;

    ajStrDel(&qcdb->DatabaseadaptorSpecies);

    qcdb->DatabaseadaptorSpecies = ajStrNewRef(dbaspecies);

    return ajTrue;
}




/* @func ensQcdatabaseSetDate *************************************************
**
** Set the date member of an Ensembl Quality Check Database.
**
** @param [u] qcdb [EnsPQcdatabase] Ensembl Quality Check Database
** @param [u] date [AjPStr] Date
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
**
** @release 6.2.0
** @@
******************************************************************************/

AjBool ensQcdatabaseSetDate(EnsPQcdatabase qcdb,
                            AjPStr date)
{
    if (!qcdb)
        return ajFalse;

    ajStrDel(&qcdb->Date);

    qcdb->Date = ajStrNewRef(date);

    return ajTrue;
}




/* @func ensQcdatabaseSetDirectory ********************************************
**
** Set the directory member of an Ensembl Quality Check Database.
**
** @param [u] qcdb [EnsPQcdatabase] Ensembl Quality Check Database
** @param [u] directory [AjPStr] Directory
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
**
** @release 6.2.0
** @@
******************************************************************************/

AjBool ensQcdatabaseSetDirectory(EnsPQcdatabase qcdb,
                                 AjPStr directory)
{
    if (!qcdb)
        return ajFalse;

    ajStrDel(&qcdb->Directory);

    qcdb->Directory = ajStrNewRef(directory);

    return ajTrue;
}




/* @func ensQcdatabaseSetFile *************************************************
**
** Set the file member of an Ensembl Quality Check Database.
**
** @param [u] qcdb [EnsPQcdatabase] Ensembl Quality Check Database
** @param [u] file [AjPStr] File
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
**
** @release 6.2.0
** @@
******************************************************************************/

AjBool ensQcdatabaseSetFile(EnsPQcdatabase qcdb,
                            AjPStr file)
{
    if (!qcdb)
        return ajFalse;

    ajStrDel(&qcdb->File);

    qcdb->File = ajStrNewRef(file);

    return ajTrue;
}




/* @func ensQcdatabaseSetFormat ***********************************************
**
** Set the format member of an Ensembl Quality Check Database.
**
** @param [u] qcdb [EnsPQcdatabase] Ensembl Quality Check Database
** @param [u] format [AjPStr] Format
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
**
** @release 6.2.0
** @@
******************************************************************************/

AjBool ensQcdatabaseSetFormat(EnsPQcdatabase qcdb,
                              AjPStr format)
{
    if (!qcdb)
        return ajFalse;

    ajStrDel(&qcdb->Format);

    qcdb->Format = ajStrNewRef(format);

    return ajTrue;
}




/* @func ensQcdatabaseSetHost *************************************************
**
** Set the host member of an Ensembl Quality Check Database.
**
** @param [u] qcdb [EnsPQcdatabase] Ensembl Quality Check Database
** @param [u] host [AjPStr] Host
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
**
** @release 6.2.0
** @@
******************************************************************************/

AjBool ensQcdatabaseSetHost(EnsPQcdatabase qcdb,
                            AjPStr host)
{
    if (!qcdb)
        return ajFalse;

    ajStrDel(&qcdb->Host);

    qcdb->Host = ajStrNewRef(host);

    return ajTrue;
}




/* @func ensQcdatabaseSetIdentifier *******************************************
**
** Set the SQL database-internal identifier member of an
** Ensembl Quality Check Database.
**
** @param [u] qcdb [EnsPQcdatabase] Ensembl Quality Check Database
** @param [r] identifier [ajuint] SQL database-internal identifier
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
**
** @release 6.2.0
** @@
******************************************************************************/

AjBool ensQcdatabaseSetIdentifier(EnsPQcdatabase qcdb,
                                  ajuint identifier)
{
    if (!qcdb)
        return ajFalse;

    qcdb->Identifier = identifier;

    return ajTrue;
}




/* @func ensQcdatabaseSetName *************************************************
**
** Set the name member of an Ensembl Quality Check Database.
**
** @param [u] qcdb [EnsPQcdatabase] Ensembl Quality Check Database
** @param [u] name [AjPStr] Name
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
**
** @release 6.2.0
** @@
******************************************************************************/

AjBool ensQcdatabaseSetName(EnsPQcdatabase qcdb,
                            AjPStr name)
{
    if (!qcdb)
        return ajFalse;

    ajStrDel(&qcdb->Name);

    qcdb->Name = ajStrNewRef(name);

    return ajTrue;
}




/* @func ensQcdatabaseSetRelease **********************************************
**
** Set the release member of an Ensembl Quality Check Database.
**
** @param [u] qcdb [EnsPQcdatabase] Ensembl Quality Check Database
** @param [u] release [AjPStr] Release
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
**
** @release 6.2.0
** @@
******************************************************************************/

AjBool ensQcdatabaseSetRelease(EnsPQcdatabase qcdb,
                               AjPStr release)
{
    if (!qcdb)
        return ajFalse;

    ajStrDel(&qcdb->Release);

    qcdb->Release = ajStrNewRef(release);

    return ajTrue;
}




/* @func ensQcdatabaseSetType *************************************************
**
** Set the type member of an Ensembl Quality Check Database.
**
** @param [u] qcdb [EnsPQcdatabase] Ensembl Quality Check Database
** @param [u] qcdbt [EnsEQcdatabaseType] Type
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
**
** @release 6.2.0
** @@
******************************************************************************/

AjBool ensQcdatabaseSetType(EnsPQcdatabase qcdb,
                            EnsEQcdatabaseType qcdbt)
{
    if (!qcdb)
        return ajFalse;

    qcdb->Type = qcdbt;

    return ajTrue;
}




/* @func ensQcdatabaseSetUrlExternal ******************************************
**
** Set the external URL member of an Ensembl Quality Check Database.
**
** @param [u] qcdb [EnsPQcdatabase] Ensembl Quality Check Database
** @param [u] url [AjPStr] External URL
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
**
** @release 6.4.0
** @@
******************************************************************************/

AjBool ensQcdatabaseSetUrlExternal(EnsPQcdatabase qcdb,
                                   AjPStr url)
{
    if (!qcdb)
        return ajFalse;

    ajStrDel(&qcdb->UrlExternal);

    qcdb->UrlExternal = ajStrNewRef(url);

    return ajTrue;
}




/* @func ensQcdatabaseSetUrlInternal ******************************************
**
** Set the internal URL member of an Ensembl Quality Check Database.
**
** @param [u] qcdb [EnsPQcdatabase] Ensembl Quality Check Database
** @param [u] url [AjPStr] Internal URL
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
**
** @release 6.4.0
** @@
******************************************************************************/

AjBool ensQcdatabaseSetUrlInternal(EnsPQcdatabase qcdb,
                                   AjPStr url)
{
    if (!qcdb)
        return ajFalse;

    ajStrDel(&qcdb->UrlInternal);

    qcdb->UrlInternal = ajStrNewRef(url);

    return ajTrue;
}




/* @section debugging *********************************************************
**
** Functions for reporting of an Ensembl Quality Check Database object.
**
** @fdata [EnsPQcdatabase]
**
** @nam3rule Trace Report Ensembl Quality Check Database members to debug file
**
** @argrule Trace qcdb [const EnsPQcdatabase] Ensembl Quality Check Database
** @argrule Trace level [ajuint] Indentation level
**
** @valrule * [AjBool] ajTrue upon success, ajFalse otherwise
**
** @fcategory misc
******************************************************************************/




/* @func ensQcdatabaseTrace ***************************************************
**
** Trace an Ensembl Quality Check Database.
**
** @param [r] qcdb [const EnsPQcdatabase] Ensembl Quality Check Database
** @param [r] level [ajuint] Indentation level
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
**
** @release 6.2.0
** @@
******************************************************************************/

AjBool ensQcdatabaseTrace(const EnsPQcdatabase qcdb, ajuint level)
{
    AjPStr indent = NULL;

    if (!qcdb)
        return ajFalse;

    indent = ajStrNew();

    ajStrAppendCountK(&indent, ' ', level * 2);

    ajDebug("%SensQcdatabaseTrace %p\n"
            "%S  Use %u\n"
            "%S  Identifier %u\n"
            "%S  Adaptor %p\n"
            "%S  Analysis %p\n"
            "%S  Name '%S'\n"
            "%S  Release '%S'\n"
            "%S  Date '%S'\n"
            "%S  Format '%S'\n"
            "%S  Class %d\n"
            "%S  Type %d\n"
            "%S  DatabaseadaptorSpecies '%S'\n"
            "%S  DatabaseadaptorGroup '%s'\n"
            "%S  Host '%S'\n"
            "%S  Directory '%S'\n"
            "%S  File '%S'\n"
            "%S  UrlExternal %p\n"
            "%S  UrlInternal '%S'\n",
            indent, qcdb,
            indent, qcdb->Use,
            indent, qcdb->Identifier,
            indent, qcdb->Adaptor,
            indent, qcdb->Analysis,
            indent, qcdb->Name,
            indent, qcdb->Release,
            indent, qcdb->Date,
            indent, qcdb->Format,
            indent, qcdb->Class,
            indent, qcdb->Type,
            indent, qcdb->DatabaseadaptorSpecies,
            indent, ensDatabaseadaptorGroupToChar(qcdb->DatabaseadaptorGroup),
            indent, qcdb->Host,
            indent, qcdb->Directory,
            indent, qcdb->File,
            indent, qcdb->UrlExternal,
            indent, qcdb->UrlInternal);

    ensAnalysisTrace(qcdb->Analysis, 1);

    ajStrDel(&indent);

    return ajTrue;
}




/* @section calculate *********************************************************
**
** Functions for calculating values of an
** Ensembl Quality Check Database object.
**
** @fdata [EnsPQcdatabase]
**
** @nam3rule Calculate Calculate Ensembl Quality Check Database values
** @nam4rule Memsize Calculate the memory size in bytes
**
** @argrule * qcdb [const EnsPQcdatabase] Ensembl Quality Check Database
**
** @valrule Memsize [size_t] Memory size in bytes or 0
**
** @fcategory misc
******************************************************************************/




/* @func ensQcdatabaseCalculateMemsize ****************************************
**
** Calculate the memory size in bytes of an Ensembl Quality Check Database.
**
** @param [r] qcdb [const EnsPQcdatabase] Ensembl Quality Check Database
**
** @return [size_t] Memory size in bytes or 0
**
** @release 6.4.0
** @@
******************************************************************************/

size_t ensQcdatabaseCalculateMemsize(const EnsPQcdatabase qcdb)
{
    size_t size = 0;

    if (!qcdb)
        return 0;

    size += sizeof (EnsOQcdatabase);

    size += ensAnalysisCalculateMemsize(qcdb->Analysis);

    if (qcdb->Name)
    {
        size += sizeof (AjOStr);

        size += ajStrGetRes(qcdb->Name);
    }

    if (qcdb->Release)
    {
        size += sizeof (AjOStr);

        size += ajStrGetRes(qcdb->Release);
    }

    if (qcdb->Date)
    {
        size += sizeof (AjOStr);

        size += ajStrGetRes(qcdb->Date);
    }

    if (qcdb->Format)
    {
        size += sizeof (AjOStr);

        size += ajStrGetRes(qcdb->Format);
    }

    if (qcdb->DatabaseadaptorSpecies)
    {
        size += sizeof (AjOStr);

        size += ajStrGetRes(qcdb->DatabaseadaptorSpecies);
    }

    if (qcdb->Host)
    {
        size += sizeof (AjOStr);

        size += ajStrGetRes(qcdb->Host);
    }

    if (qcdb->Directory)
    {
        size += sizeof (AjOStr);

        size += ajStrGetRes(qcdb->Directory);
    }

    if (qcdb->File)
    {
        size += sizeof (AjOStr);

        size += ajStrGetRes(qcdb->File);
    }

    if (qcdb->UrlExternal)
    {
        size += sizeof (AjOStr);

        size += ajStrGetRes(qcdb->UrlExternal);
    }

    if (qcdb->UrlInternal)
    {
        size += sizeof (AjOStr);

        size += ajStrGetRes(qcdb->UrlInternal);
    }

    return size;
}




/* @section matching **********************************************************
**
** Functions for matching Ensembl Quality Check Database objects
**
** @fdata [EnsPQcdatabase]
**
** @nam3rule Match Compare two Ensembl Quality Check Database objects
**
** @argrule * qcdb1 [const EnsPQcdatabase] Ensembl Quality Check Database
** @argrule * qcdb2 [const EnsPQcdatabase] Ensembl Quality Check Database
**
** @valrule * [AjBool] True on success
**
** @fcategory use
******************************************************************************/




/* @func ensQcdatabaseMatch ***************************************************
**
** Tests for matching two Ensembl Quality Check Database objects.
**
** @param [r] qcdb1 [const EnsPQcdatabase]
** First Ensembl Quality Check Database
** @param [r] qcdb2 [const EnsPQcdatabase]
** Second Ensembl Quality Check Database
**
** @return [AjBool] ajTrue if the Ensembl Quality Check Database objects
**                  are equal
**
** @release 6.2.0
** @@
** The comparison is based on initial pointer equality and if that fails
** each member is compared.
******************************************************************************/

AjBool ensQcdatabaseMatch(const EnsPQcdatabase qcdb1,
                          const EnsPQcdatabase qcdb2)
{
    if (!qcdb1)
        return ajFalse;

    if (!qcdb2)
        return ajFalse;

    if (qcdb1 == qcdb2)
        return ajTrue;

    if (qcdb1->Identifier != qcdb2->Identifier)
        return ajFalse;

    if ((qcdb1->Adaptor && qcdb2->Adaptor) &&
        (qcdb1->Adaptor != qcdb2->Adaptor))
        return ajFalse;

    if (!ajStrMatchCaseS(qcdb1->Name, qcdb2->Name))
        return ajFalse;

    if (!ajStrMatchCaseS(qcdb1->Release, qcdb2->Release))
        return ajFalse;

    if (!ajStrMatchCaseS(qcdb1->Date, qcdb2->Date))
        return ajFalse;

    if (!ajStrMatchCaseS(qcdb1->Format, qcdb2->Format))
        return ajFalse;

    if (qcdb1->Class != qcdb2->Class)
        return ajFalse;

    if (qcdb1->Type != qcdb2->Type)
        return ajFalse;

    if (!ajStrMatchCaseS(qcdb1->DatabaseadaptorSpecies,
                         qcdb2->DatabaseadaptorSpecies))
        return ajFalse;

    if (qcdb1->DatabaseadaptorGroup != qcdb2->DatabaseadaptorGroup)
        return ajFalse;

    if (!ajStrMatchCaseS(qcdb1->Host, qcdb2->Host))
        return ajFalse;

    if (!ajStrMatchCaseS(qcdb1->Directory, qcdb2->Directory))
        return ajFalse;

    if (!ajStrMatchCaseS(qcdb1->File, qcdb2->File))
        return ajFalse;

    if (!ajStrMatchCaseS(qcdb1->UrlExternal, qcdb2->UrlExternal))
        return ajFalse;

    if (!ajStrMatchCaseS(qcdb1->UrlInternal, qcdb2->UrlInternal))
        return ajFalse;

    return ajTrue;
}




/* @datasection [EnsEQcdatabaseClass] Ensembl Quality Check Database Class ****
**
** @nam2rule Qcdatabase Functions for manipulating
** Ensembl Quality Check Database objects
** @nam3rule QcdatabaseClass Functions for manipulating
** Ensembl Quality Check Database Class enumerations
**
******************************************************************************/




/* @section Misc **************************************************************
**
** Functions for returning an Ensembl Quality Check Database Class enumeration.
**
** @fdata [EnsEQcdatabaseClass]
**
** @nam4rule From Ensembl Quality Check Database Class query
** @nam5rule Str  AJAX String object query
**
** @argrule  Str  classstr  [const AjPStr] Class string
**
** @valrule * [EnsEQcdatabaseClass]
** Ensembl Quality Check Database Class enumeration or ensEQcdatabaseClassNULL
**
** @fcategory misc
******************************************************************************/




/* @func ensQcdatabaseClassFromStr ********************************************
**
** Convert an AJAX String into an
** Ensembl Quality Check Database Class enumeration.
**
** @param [r] classstr [const AjPStr] Class string
**
** @return [EnsEQcdatabaseClass]
** Ensembl Quality Check Database Class or ensEQcdatabaseClassNULL
**
** @release 6.2.0
** @@
******************************************************************************/

EnsEQcdatabaseClass ensQcdatabaseClassFromStr(const AjPStr classstr)
{
    register EnsEQcdatabaseClass i = ensEQcdatabaseClassNULL;

    EnsEQcdatabaseClass qcdbc = ensEQcdatabaseClassNULL;

    for (i = ensEQcdatabaseClassNULL;
         qcdatabaseKClass[i];
         i++)
        if (ajStrMatchCaseC(classstr, qcdatabaseKClass[i]))
            qcdbc = i;

    if (!qcdbc)
        ajDebug("ensQcdatabaseClassFromStr encountered "
                "unexpected string '%S'.\n", classstr);

    return qcdbc;
}




/* @section Cast **************************************************************
**
** Functions for returning attributes of an
** Ensembl Quality Check Database Class enumeration.
**
** @fdata [EnsEQcdatabaseClass]
**
** @nam4rule To   Return Ensembl Quality Check Database Class enumeration
** @nam5rule Char Return C character string value
**
** @argrule To qcdbc [EnsEQcdatabaseClass]
** Ensembl Quality Check Database Class enumeration
**
** @valrule Char [const char*] Class or NULL
**
** @fcategory cast
******************************************************************************/




/* @func ensQcdatabaseClassToChar *********************************************
**
** Convert an Ensembl Quality Check Database Class enumeration into a
** C-type (char *) string.
**
** @param [u] qcdbc [EnsEQcdatabaseClass]
** Ensembl Quality Check Database Class enumeration
**
** @return [const char*]
** Ensembl Quality Check Database Class C-type (char *) string
** or NULL
**
** @release 6.2.0
** @@
******************************************************************************/

const char* ensQcdatabaseClassToChar(EnsEQcdatabaseClass qcdbc)
{
    register EnsEQcdatabaseClass i = ensEQcdatabaseClassNULL;

    for (i = ensEQcdatabaseClassNULL;
         qcdatabaseKClass[i] && (i < qcdbc);
         i++);

    if (!qcdatabaseKClass[i])
        ajDebug("ensQcdatabaseClassToChar encountered an "
                "out of boundary error on "
                "Ensembl Quality Check Database Class enumeration %d.\n",
                qcdbc);

    return qcdatabaseKClass[i];
}




/* @datasection [EnsEQcdatabaseType] Ensembl Quality Check Database Type ******
**
** @nam2rule Qcdatabase Functions for manipulating
** Ensembl Quality Check Database objects
** @nam3rule QcdatabaseType Functions for manipulating
** Ensembl Quality Check Database Type enumerations
**
******************************************************************************/




/* @section Misc **************************************************************
**
** Functions for returning an Ensembl Quality Check Database Type enumeration.
**
** @fdata [EnsEQcdatabaseType]
**
** @nam4rule From Ensembl Quality Check Database Type query
** @nam5rule Str  AJAX String object query
**
** @argrule  Str  typestr  [const AjPStr] Type string
**
** @valrule * [EnsEQcdatabaseType]
** Ensembl Quality Check Database Type enumeration or ensEQcdatabaseTypeNULL
**
** @fcategory misc
******************************************************************************/




/* @func ensQcdatabaseTypeFromStr *********************************************
**
** Convert an AJAX String into an
** Ensembl Quality Check Database Type enumeration.
**
** @param [r] typestr [const AjPStr] Type string
**
** @return [EnsEQcdatabaseType]
** Ensembl Quality Check Database Type enumeration or ensEQcdatabaseTypeNULL
**
** @release 6.2.0
** @@
******************************************************************************/

EnsEQcdatabaseType ensQcdatabaseTypeFromStr(const AjPStr typestr)
{
    register EnsEQcdatabaseType i = ensEQcdatabaseTypeNULL;

    EnsEQcdatabaseType qcdbt = ensEQcdatabaseTypeNULL;

    for (i = ensEQcdatabaseTypeNULL;
         qcdatabaseKType[i];
         i++)
        if (ajStrMatchCaseC(typestr, qcdatabaseKType[i]))
            qcdbt = i;

    if (!qcdbt)
        ajDebug("ensQcdatabaseTypeFromStr encountered "
                "unexpected string '%S'.\n", typestr);

    return qcdbt;
}




/* @section Cast **************************************************************
**
** Functions for returning attributes of an
** Ensembl Quality Check Database Type enumeration.
**
** @fdata [EnsEQcdatabaseType]
**
** @nam4rule To   Return Ensembl Quality Check Database Type enumeration
** @nam5rule Char Return C character string value
**
** @argrule To qcdbt [EnsEQcdatabaseType]
** Ensembl Quality Check Database Type enumeration
**
** @valrule Char [const char*] Class or NULL
**
** @fcategory cast
******************************************************************************/




/* @func ensQcdatabaseTypeToChar **********************************************
**
** Convert an Ensembl Quality Check Database Type enumeration into a
** C-type (char *) string.
**
** @param [u] qcdbt [EnsEQcdatabaseType]
** Ensembl Quality Check Database Type enumeration
**
** @return [const char*]
** Ensembl Quality Check Database Type C-type (char *) string or NULL
**
** @release 6.2.0
** @@
******************************************************************************/

const char* ensQcdatabaseTypeToChar(EnsEQcdatabaseType qcdbt)
{
    register EnsEQcdatabaseType i = ensEQcdatabaseTypeNULL;

    for (i = ensEQcdatabaseTypeNULL;
         qcdatabaseKType[i] && (i < qcdbt);
         i++);

    if (!qcdatabaseKType[i])
        ajDebug("ensQcdatabaseTypeToChar encountered an "
                "out of boundary error on "
                "Ensembl Quality Check Database Type enumeration %d.\n",
                qcdbt);

    return qcdatabaseKType[i];
}




/* @datasection [EnsPQcdatabaseadaptor] Ensembl Quality Check Database Adaptor
**
** @nam2rule Qcdatabaseadaptor Functions for manipulating
** Ensembl Quality Check Database Adaptor objects
**
** @cc Bio::EnsEMBL::QC::DBSQL::SequenceDBAdaptor
** @cc CVS Revision:
** @cc CVS Tag:
**
******************************************************************************/




/* @funcstatic qcdatabaseadaptorFetchAllbyStatement ***************************
**
** Run a SQL statement against an Ensembl Database Adaptor and consolidate the
** results into an AJAX List of Ensembl Quality Check Database objects.
**
** @param [u] ba [EnsPBaseadaptor] Ensembl Base Adaptor
** @param [r] statement [const AjPStr] SQL statement
** @param [uN] am [EnsPAssemblymapper] Ensembl Assembly Mapper
** @param [uN] slice [EnsPSlice] Ensembl Slice
** @param [u] qcdbs [AjPList] AJAX List of
** Ensembl Quality Check Database objects
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
**
** @release 6.4.0
** @@
******************************************************************************/

static AjBool qcdatabaseadaptorFetchAllbyStatement(
    EnsPBaseadaptor ba,
    const AjPStr statement,
    EnsPAssemblymapper am,
    EnsPSlice slice,
    AjPList qcdbs)
{
    ajuint identifier = 0U;
    ajuint analysisid = 0U;

    AjPSqlstatement sqls = NULL;
    AjISqlrow sqli       = NULL;
    AjPSqlrow sqlr       = NULL;

    AjPStr name        = NULL;
    AjPStr release     = NULL;
    AjPStr date        = NULL;
    AjPStr format      = NULL;
    AjPStr type        = NULL;
    AjPStr class       = NULL;
    AjPStr species     = NULL;
    AjPStr group       = NULL;
    AjPStr host        = NULL;
    AjPStr directory   = NULL;
    AjPStr file        = NULL;
    AjPStr externalurl = NULL;

    EnsEDatabaseadaptorGroup dbag = ensEDatabaseadaptorGroupNULL;
    EnsEQcdatabaseClass     qcdbc = ensEQcdatabaseClassNULL;
    EnsEQcdatabaseType      qcdbt = ensEQcdatabaseTypeNULL;

    EnsPAnalysis analysis  = NULL;
    EnsPAnalysisadaptor aa = NULL;

    EnsPDatabaseadaptor dba = NULL;

    EnsPQcdatabase qcdb         = NULL;
    EnsPQcdatabaseadaptor qcdba = NULL;

    if (ajDebugTest("qcdatabaseadaptorFetchAllbyStatement"))
        ajDebug("qcdatabaseadaptorFetchAllbyStatement\n"
                "  ba %p\n"
                "  statement %p\n"
                "  am %p\n"
                "  slice %p\n"
                "  qcdbs %p\n",
                ba,
                statement,
                am,
                slice,
                qcdbs);

    if (!ba)
        return ajFalse;

    if (!statement)
        return ajFalse;

    if (!qcdbs)
        return ajFalse;

    dba = ensBaseadaptorGetDatabaseadaptor(ba);

    aa    = ensRegistryGetAnalysisadaptor(dba);
    qcdba = ensRegistryGetQcdatabaseadaptor(dba);

    sqls = ensDatabaseadaptorSqlstatementNew(dba, statement);

    sqli = ajSqlrowiterNew(sqls);

    while (!ajSqlrowiterDone(sqli))
    {
        identifier  = 0;
        analysisid  = 0;
        name        = ajStrNew();
        release     = ajStrNew();
        date        = ajStrNew();
        format      = ajStrNew();
        type        = ajStrNew();
        class       = ajStrNew();
        species     = ajStrNew();
        group       = ajStrNew();
        host        = ajStrNew();
        directory   = ajStrNew();
        file        = ajStrNew();
        externalurl = ajStrNew();

        sqlr = ajSqlrowiterGet(sqli);

        ajSqlcolumnToUint(sqlr, &identifier);
        ajSqlcolumnToUint(sqlr, &analysisid);
        ajSqlcolumnToStr(sqlr, &name);
        ajSqlcolumnToStr(sqlr, &release);
        ajSqlcolumnToStr(sqlr, &date);
        ajSqlcolumnToStr(sqlr, &format);
        ajSqlcolumnToStr(sqlr, &type);
        ajSqlcolumnToStr(sqlr, &class);
        ajSqlcolumnToStr(sqlr, &species);
        ajSqlcolumnToStr(sqlr, &group);
        ajSqlcolumnToStr(sqlr, &host);
        ajSqlcolumnToStr(sqlr, &directory);
        ajSqlcolumnToStr(sqlr, &file);
        ajSqlcolumnToStr(sqlr, &externalurl);

        ensAnalysisadaptorFetchByIdentifier(aa, analysisid, &analysis);

        qcdbt = ensQcdatabaseTypeFromStr(type);

        qcdbc = ensQcdatabaseClassFromStr(class);

        dbag = ensDatabaseadaptorGroupFromStr(group);

        qcdb = ensQcdatabaseNewIni(qcdba,
                                   identifier,
                                   analysis,
                                   name,
                                   release,
                                   date,
                                   format,
                                   qcdbc,
                                   qcdbt,
                                   species,
                                   dbag,
                                   host,
                                   directory,
                                   file,
                                   externalurl);

        ajListPushAppend(qcdbs, (void *) qcdb);

        ensAnalysisDel(&analysis);

        ajStrDel(&name);
        ajStrDel(&release);
        ajStrDel(&date);
        ajStrDel(&format);
        ajStrDel(&type);
        ajStrDel(&class);
        ajStrDel(&species);
        ajStrDel(&group);
        ajStrDel(&host);
        ajStrDel(&directory);
        ajStrDel(&file);
        ajStrDel(&externalurl);
    }

    ajSqlrowiterDel(&sqli);

    ensDatabaseadaptorSqlstatementDel(dba, &sqls);

    return ajTrue;
}




/* @funcstatic qcdatabaseadaptorCacheInsert ***********************************
**
** Insert an Ensembl Quality Check Database into the
** Ensembl Quality Check Database Adaptor-internal cache.
** If an Ensembl Quality Check Database with the same name member is already
** present in the adaptor cache, the Ensembl Quality Check Database is deleted
** and a pointer to the cached Ensembl Quality Check Database is returned.
**
** @param [u] qcdba [EnsPQcdatabaseadaptor]
** Ensembl Quality Check Database Adaptor
** @param [u] Pqcdb [EnsPQcdatabase*] Ensembl Quality Check Database address
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
**
** @release 6.3.0
** @@
******************************************************************************/

static AjBool qcdatabaseadaptorCacheInsert(EnsPQcdatabaseadaptor qcdba,
                                           EnsPQcdatabase *Pqcdb)
{
    ajuint *Pidentifier = NULL;

    AjPStr key = NULL;

    EnsPQcdatabase qcdb1 = NULL;
    EnsPQcdatabase qcdb2 = NULL;

    if (!qcdba)
        return ajFalse;

    if (!qcdba->CacheByIdentifier)
        return ajFalse;

    if (!qcdba->CacheByName)
        return ajFalse;

    if (!Pqcdb)
        return ajFalse;

    if (!*Pqcdb)
        return ajFalse;

    /* Search the identifer cache. */

    qcdb1 = (EnsPQcdatabase) ajTableFetchmodV(
        qcdba->CacheByIdentifier,
        (const void *) &((*Pqcdb)->Identifier));

    /* Search the name cache. */

    key = ajFmtStr("%S:%S", (*Pqcdb)->Name, (*Pqcdb)->Release);

    qcdb2 = (EnsPQcdatabase) ajTableFetchmodS(qcdba->CacheByName, key);

    if ((!qcdb1) && (!qcdb2))
    {
        /* Insert into the identifier cache. */

        AJNEW0(Pidentifier);

        *Pidentifier = (*Pqcdb)->Identifier;

        ajTablePut(qcdba->CacheByIdentifier,
                   (void *) Pidentifier,
                   (void *) ensQcdatabaseNewRef(*Pqcdb));

        /* Insert into the name cache. */

        ajTablePut(qcdba->CacheByName,
                   (void *) ajStrNewS(key),
                   (void *) ensQcdatabaseNewRef(*Pqcdb));
    }

    if (qcdb1 && qcdb2 && (qcdb1 == qcdb2))
    {
        ajDebug("qcdatabaseadaptorCacheInsert replaced "
                "Ensembl Quality Check Database %p with "
                "one already cached %p.\n",
                *Pqcdb, qcdb1);

        ensQcdatabaseDel(Pqcdb);

        Pqcdb = &qcdb1;
    }

    if (qcdb1 && qcdb2 && (qcdb1 != qcdb2))
        ajDebug("qcdatabaseadaptorCacheInsert detected "
                "Ensembl Quality Check Database objects in the "
                "identifier and name cache with identical names "
                "('%S' and '%S') but different addresses (%p and %p).\n",
                qcdb1->Name, qcdb2->Name, qcdb1, qcdb2);

    if (qcdb1 && (!qcdb2))
        ajDebug("qcdatabaseadaptorCacheInsert detected an "
                "Ensembl Quality Check Database "
                "in the identifier, but not in the name cache.\n");

    if ((!qcdb1) && qcdb2)
        ajDebug("qcdatabaseadaptorCacheInsert detected an "
                "Ensembl Quality Check Database "
                "in the name, but not in the identifier cache.\n");

    ajStrDel(&key);

    return ajTrue;
}




/* @funcstatic qcdatabaseadaptorCacheRemove ***********************************
**
** Remove an Ensembl Quality Check Database from the
** Ensembl Quality Check Database Adaptor-internal cache.
**
** @param [u] qcdba [EnsPQcdatabaseadaptor]
** Ensembl Quality Check Database Adaptor
** @param [u] qcdb [EnsPQcdatabase] Ensembl Quality Check Database
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
**
** @release 6.4.0
** @@
******************************************************************************/

static AjBool qcdatabaseadaptorCacheRemove(EnsPQcdatabaseadaptor qcdba,
                                           EnsPQcdatabase qcdb)
{
    EnsPQcdatabase qcdb1 = NULL;
    EnsPQcdatabase qcdb2 = NULL;

    if (!qcdba)
        return ajFalse;

    if (!qcdb)
        return ajFalse;

    /* Remove the table nodes. */

    qcdb1 = (EnsPQcdatabase) ajTableRemove(
        qcdba->CacheByIdentifier,
        (const void *) &qcdb->Identifier);

    qcdb2 = (EnsPQcdatabase) ajTableRemove(
        qcdba->CacheByName,
        (const void *) qcdb->Name);

    if (qcdb1 && (!qcdb2))
        ajWarn("qcdatabaseadaptorCacheRemove could remove "
               "Ensembl Quality Check Database "
               "with identifier %u and name '%S' only from the identifier "
               "cache.\n",
               qcdb->Identifier,
               qcdb->Name);

    if ((!qcdb1) && qcdb2)
        ajWarn("qcdatabaseadaptorCacheRemove could remove "
               "Ensembl Quality Check Database "
               "with identifier %u and name '%S' only from the name cache.\n",
               qcdb->Identifier,
               qcdb->Name);

    ensQcdatabaseDel(&qcdb1);
    ensQcdatabaseDel(&qcdb2);

    return ajTrue;
}




/* @funcstatic qcdatabaseadaptorCacheInit *************************************
**
** Initialise the internal Quality Check Database cache of an
** Ensembl Quality Check Database Adaptor.
**
** @param [u] qcdba [EnsPQcdatabaseadaptor]
** Ensembl Quality Check Database Adaptor
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
**
** @release 6.3.0
** @@
******************************************************************************/

static AjBool qcdatabaseadaptorCacheInit(EnsPQcdatabaseadaptor qcdba)
{
    AjPList qcdbs = NULL;

    EnsPQcdatabase qcdb = NULL;

    if (!qcdba)
        return ajFalse;

    if (qcdba->CacheByIdentifier)
        return ajFalse;
    else
    {
        qcdba->CacheByIdentifier = ajTableuintNew(0);

        ajTableSetDestroyvalue(
            qcdba->CacheByIdentifier,
            (void (*)(void **)) &ensQcdatabaseDel);
    }

    if (qcdba->CacheByName)
        return ajFalse;
    else
    {
        qcdba->CacheByName = ajTablestrNew(0);

        ajTableSetDestroyvalue(
            qcdba->CacheByName,
            (void (*)(void **)) &ensQcdatabaseDel);
    }

    qcdbs = ajListNew();

    ensBaseadaptorFetchAllbyConstraint(qcdba->Adaptor,
                                       (const AjPStr) NULL,
                                       (EnsPAssemblymapper) NULL,
                                       (EnsPSlice) NULL,
                                       qcdbs);

    while (ajListPop(qcdbs, (void **) &qcdb))
    {
        qcdatabaseadaptorCacheInsert(qcdba, &qcdb);

        ensQcdatabaseDel(&qcdb);
    }

    ajListFree(&qcdbs);

    return ajTrue;
}




/* @section constructors ******************************************************
**
** All constructors return a new Ensembl Quality Check Database Adaptor by
** pointer. It is the responsibility of the user to first destroy any previous
** Quality Check Database Adaptor. The target pointer does not need to be
** initialised to NULL, but it is good programming practice to do so anyway.
**
** @fdata [EnsPQcdatabaseadaptor]
**
** @nam3rule New Constructor
**
** @argrule New dba [EnsPDatabaseadaptor] Ensembl Database Adaptor
**
** @valrule * [EnsPQcdatabaseadaptor]
** Ensembl Quality Check Database Adaptor or NULL
**
** @fcategory new
******************************************************************************/




/* @func ensQcdatabaseadaptorNew **********************************************
**
** Default constructor for an Ensembl Quality Check Database Adaptor.
**
** Ensembl Object Adaptors are singleton objects in the sense that a single
** instance of an Ensembl Object Adaptor connected to a particular database is
** sufficient to instantiate any number of Ensembl Objects from the database.
** Each Ensembl Object will have a weak reference to the Object Adaptor that
** instantiated it. Therefore, Ensembl Object Adaptors should not be
** instantiated directly, but rather obtained from the Ensembl Registry,
** which will in turn call this function if neccessary.
**
** @see ensRegistryGetDatabaseadaptor
** @see ensRegistryGetQcdatabaseadaptor
**
** @param [u] dba [EnsPDatabaseadaptor] Ensembl Database Adaptor
**
** @return [EnsPQcdatabaseadaptor]
** Ensembl Quality Check Database Adaptor or NULL
**
** @release 6.2.0
** @@
******************************************************************************/

EnsPQcdatabaseadaptor ensQcdatabaseadaptorNew(
    EnsPDatabaseadaptor dba)
{
    EnsPQcdatabaseadaptor qcdba = NULL;

    if (!dba)
        return NULL;

    AJNEW0(qcdba);

    qcdba->Adaptor = ensBaseadaptorNew(
        dba,
        qcdatabaseadaptorKTables,
        qcdatabaseadaptorKColumns,
        (const EnsPBaseadaptorLeftjoin) NULL,
        (const char *) NULL,
        (const char *) NULL,
        &qcdatabaseadaptorFetchAllbyStatement);

    /*
    ** NOTE: The cache cannot be initialised here because the
    ** qcdatabaseadaptorCacheInit function calls
    ** ensBaseadaptorFetchAllbyConstraint, which calls
    ** qcdatabaseadaptorFetchAllbyStatement, which calls
    ** ensRegistryGetQcdatabaseadaptor. At that point, however, the
    ** Ensembl Quality Check Database Adaptor has not been stored in the
    ** Ensembl Registry. Therefore, each ensQcdatabaseadaptorFetch function has
    ** to test the presence of the adaptor-internal cache and eventually
    ** initialise before accessing it.
    **
    **  qcdatabaseadaptorCacheInit(qcdba);
    */

    return qcdba;
}




/* @section destructors *******************************************************
**
** Destruction destroys all internal data structures and frees the memory
** allocated for an Ensembl Quality Check Database Adaptor object.
**
** @fdata [EnsPQcdatabaseadaptor]
**
** @nam3rule Del Destroy (free) an Ensembl Quality Check Database Adaptor
**
** @argrule * Pqcdba [EnsPQcdatabaseadaptor*]
** Ensembl Quality Check Database Adaptor address
**
** @valrule * [void]
**
** @fcategory delete
******************************************************************************/




/* @func ensQcdatabaseadaptorDel **********************************************
**
** Default destructor for an Ensembl Quality Check Database Adaptor.
**
** Ensembl Object Adaptors are singleton objects that are registered in the
** Ensembl Registry and weakly referenced by Ensembl Objects that have been
** instantiated by it. Therefore, Ensembl Object Adaptors should never be
** destroyed directly. Upon exit, the Ensembl Registry will call this function
** if required.
**
** @param [d] Pqcdba [EnsPQcdatabaseadaptor*]
** Ensembl Quality Check Database Adaptor address
**
** @return [void]
**
** @release 6.2.0
** @@
******************************************************************************/

void ensQcdatabaseadaptorDel(EnsPQcdatabaseadaptor *Pqcdba)
{
    EnsPQcdatabaseadaptor pthis = NULL;

    if (!Pqcdba)
        return;

#if defined(AJ_DEBUG) && AJ_DEBUG >= 1
    if (ajDebugTest("ensQcdatabaseadaptorDel"))
        ajDebug("ensQcdatabaseadaptorDel\n"
                "  *Pqcdba %p\n",
                *Pqcdba);
#endif /* defined(AJ_DEBUG) && AJ_DEBUG >= 1 */

    if (!*Pqcdba)
        return;

    pthis = *Pqcdba;

    ajTableDel(&pthis->CacheByIdentifier);
    ajTableDel(&pthis->CacheByName);

    ensBaseadaptorDel(&pthis->Adaptor);

    AJFREE(pthis);

    *Pqcdba = NULL;

    return;
}




/* @section member retrieval **************************************************
**
** Functions for returning members of an
** Ensembl Quality Check Database Adaptor object.
**
** @fdata [EnsPQcdatabaseadaptor]
**
** @nam3rule Get Return Ensembl Quality Check Database Adaptor attribute(s)
** @nam4rule Baseadaptor Return the Ensembl Base Adaptor
** @nam4rule Databaseadaptor Return the Ensembl Database Adaptor
**
** @argrule * qcdba [EnsPQcdatabaseadaptor]
** Ensembl Quality Check Database Adaptor
**
** @valrule Baseadaptor [EnsPBaseadaptor] Ensembl Base Adaptor or NULL
** @valrule Databaseadaptor [EnsPDatabaseadaptor] Ensembl Database Adaptor
** or NULL
**
** @fcategory use
******************************************************************************/




/* @func ensQcdatabaseadaptorGetBaseadaptor ***********************************
**
** Get the Ensembl Base Adaptor member of an
** Ensembl Quality Check Database Adaptor.
**
** @param [u] qcdba [EnsPQcdatabaseadaptor]
** Ensembl Quality Check Database Adaptor
**
** @return [EnsPBaseadaptor] Ensembl Base Adaptor or NULL
**
** @release 6.4.0
** @@
******************************************************************************/

EnsPBaseadaptor ensQcdatabaseadaptorGetBaseadaptor(
    EnsPQcdatabaseadaptor qcdba)
{
    return (qcdba) ? qcdba->Adaptor : NULL;
}




/* @func ensQcdatabaseadaptorGetDatabaseadaptor *******************************
**
** Get the Ensembl Database Adaptor member of an
** Ensembl Quality Check Database Adaptor.
**
** @param [u] qcdba [EnsPQcdatabaseadaptor]
** Ensembl Quality Check Database Adaptor
**
** @return [EnsPDatabaseadaptor] Ensembl Database Adaptor or NULL
**
** @release 6.4.0
** @@
******************************************************************************/

EnsPDatabaseadaptor ensQcdatabaseadaptorGetDatabaseadaptor(
    EnsPQcdatabaseadaptor qcdba)
{
    return (qcdba) ? ensBaseadaptorGetDatabaseadaptor(qcdba->Adaptor) : NULL;
}




/* @section object retrieval **************************************************
**
** Functions for fetching Ensembl Quality Check Database objects from an
** Ensembl SQL database.
**
** @fdata [EnsPQcdatabaseadaptor]
**
** @nam3rule Fetch Fetch Ensembl Quality Check Database object(s)
** @nam4rule All   Fetch all Ensembl Quality Check Database objects
** @nam4rule Allby Fetch all Ensembl Quality Check Database objects matching
**                 a criterion
** @nam5rule Classtype Fetch all by a class and type
** @nam4rule By    Fetch one Ensembl Quality Check Database object matching
**                 a criterion
** @nam5rule Identifier Fetch by an SQL database internal identifier
** @nam5rule Name Fetch by a name
**
** @argrule * qcdba [EnsPQcdatabaseadaptor]
** Ensembl Quality Check Database Adaptor
** @argrule All qcdbs [AjPList] AJAX List of
** Ensembl Quality Check Database objects
** @argrule AllbyClasstype qcdbc [EnsEQcdatabaseClass]
** Ensembl Quality Check Database Class enumeration
** @argrule AllbyClasstype qcdbt [EnsEQcdatabaseType]
** Ensembl Quality Check Database Type enumeration
** @argrule AllbyClasstype qcdbs [AjPList] AJAX List of
** Ensembl Quality Check Database objects
** @argrule ByIdentifier identifier [ajuint] SQL database-internal identifier
** @argrule ByName name [const AjPStr]
** Ensembl Quality Check Database name
** @argrule ByName release [const AjPStr]
** Ensembl Quality Check Database release
** @argrule By Pqcdb [EnsPQcdatabase*] Ensembl Quality Check Database address
**
** @valrule * [AjBool] ajTrue upon success, ajFalse otherwise
**
** @fcategory use
******************************************************************************/




/* @func ensQcdatabaseadaptorFetchAllbyClasstype ******************************
**
** Fetch an Ensembl Quality Check Database by its class and type.
**
** @param [u] qcdba [EnsPQcdatabaseadaptor]
** Ensembl Quality Check Database Adaptor
** @param [u] qcdbc [EnsEQcdatabaseClass]
** Ensembl Quality Check Database Class enumeration
** @param [u] qcdbt [EnsEQcdatabaseType]
** Ensembl Quality Check Database Type enumeration
** @param [u] qcdbs [AjPList] AJAX List of
** Ensembl Quality Check Database objects
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
**
** @release 6.4.0
** @@
******************************************************************************/

AjBool ensQcdatabaseadaptorFetchAllbyClasstype(EnsPQcdatabaseadaptor qcdba,
                                               EnsEQcdatabaseClass qcdbc,
                                               EnsEQcdatabaseType qcdbt,
                                               AjPList qcdbs)
{
    void **keyarray = NULL;
    void **valarray = NULL;

    register ajuint i = 0U;

    AjBool cmatch = AJFALSE;
    AjBool tmatch = AJFALSE;

    if (!qcdba)
        return ajFalse;

    if (!qcdbs)
        return ajFalse;

    if (!qcdba->CacheByIdentifier)
        qcdatabaseadaptorCacheInit(qcdba);

    ajTableToarrayKeysValues(qcdba->CacheByIdentifier, &keyarray, &valarray);

    for (i = 0U; keyarray[i]; i++)
    {
        /* Match the class member if one has been provided. */

        if (qcdbc)
        {
            if (((EnsPQcdatabase) valarray[i])->Class == qcdbc)
                cmatch = ajTrue;
            else
                cmatch = ajFalse;
        }
        else
            cmatch = ajTrue;

        /* Match the type member if one has been provided. */

        if (qcdbt)
        {
            if (((EnsPQcdatabase) valarray[i])->Type == qcdbt)
                tmatch = ajTrue;
            else
                tmatch = ajFalse;
        }
        else
            tmatch = ajTrue;

        if (cmatch && tmatch)
            ajListPushAppend(qcdbs, (void *)
                             ensQcdatabaseNewRef(valarray[i]));
    }

    AJFREE(keyarray);
    AJFREE(valarray);

    return ajTrue;
}




/* @func ensQcdatabaseadaptorFetchByIdentifier ********************************
**
** Fetch an Ensembl Quality Check Database by its
** SQL database-internal identifier.
**
** @param [u] qcdba [EnsPQcdatabaseadaptor]
** Ensembl Quality Check Database Adaptor
** @param [r] identifier [ajuint] SQL database-internal identifier
** @param [wP] Pqcdb [EnsPQcdatabase*] Ensembl Quality Check Database address
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
**
** @release 6.2.0
** @@
******************************************************************************/

AjBool ensQcdatabaseadaptorFetchByIdentifier(EnsPQcdatabaseadaptor qcdba,
                                             ajuint identifier,
                                             EnsPQcdatabase *Pqcdb)
{
    if (!qcdba)
        return ajFalse;

    if (!Pqcdb)
        return ajFalse;

    if (!qcdba->CacheByIdentifier)
        qcdatabaseadaptorCacheInit(qcdba);

    /*
    ** Initially, search the identifier cache.
    ** For any object returned by the AJAX Table the reference counter needs
    ** to be incremented manually.
    */

    *Pqcdb = (EnsPQcdatabase) ajTableFetchmodV(qcdba->CacheByIdentifier,
                                               (const void *) &identifier);

    if (*Pqcdb)
    {
        ensQcdatabaseNewRef(*Pqcdb);

        return ajTrue;
    }

    ensBaseadaptorFetchByIdentifier(qcdba->Adaptor,
                                    identifier,
                                    (void **) Pqcdb);

    qcdatabaseadaptorCacheInsert(qcdba, Pqcdb);

    return ajTrue;
}




/* @func ensQcdatabaseadaptorFetchByName **************************************
**
** Fetch an Ensembl Quality Check Database by its name and release.
**
** @param [u] qcdba [EnsPQcdatabaseadaptor]
** Ensembl Quality Check Database Adaptor
** @param [r] name [const AjPStr] Ensembl Quality Check Database name
** @param [r] release [const AjPStr] Ensembl Quality Check Database release
** @param [wP] Pqcdb [EnsPQcdatabase*] Ensembl Quality Check Database address
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
**
** @release 6.2.0
** @@
******************************************************************************/

AjBool ensQcdatabaseadaptorFetchByName(EnsPQcdatabaseadaptor qcdba,
                                       const AjPStr name,
                                       const AjPStr release,
                                       EnsPQcdatabase *Pqcdb)
{
    char *txtname    = NULL;
    char *txtrelease = NULL;

    AjPList qcdbs = NULL;

    AjPStr key        = NULL;
    AjPStr constraint = NULL;

    EnsPQcdatabase qcdb = NULL;

    if (!qcdba)
        return ajFalse;

    if ((!name) && (!ajStrGetLen(name)))
        return ajFalse;

    if ((!release) && (!ajStrGetLen(release)))
        return ajFalse;

    if (!Pqcdb)
        return ajFalse;

    if (!qcdba->CacheByName)
        qcdatabaseadaptorCacheInit(qcdba);

    /*
    ** Initially, search the name cache.
    ** For any object returned by the AJAX Table the reference counter needs
    ** to be incremented manually.
    */

    key = ajFmtStr("%S:%S", name, release);

    *Pqcdb = (EnsPQcdatabase) ajTableFetchmodS(qcdba->CacheByName, key);

    ajStrDel(&key);

    if (*Pqcdb)
    {
        ensQcdatabaseNewRef(*Pqcdb);

        return ajTrue;
    }

    /* In case of a cache miss, query the database. */

    ensBaseadaptorEscapeC(qcdba->Adaptor, &txtname, name);
    ensBaseadaptorEscapeC(qcdba->Adaptor, &txtrelease, release);

    constraint = ajFmtStr("sequence_db.name = '%s' "
                          "AND "
                          "sequence_db.release = '%s'",
                          txtname,
                          txtrelease);

    ajCharDel(&txtname);
    ajCharDel(&txtrelease);

    qcdbs = ajListNew();

    ensBaseadaptorFetchAllbyConstraint(qcdba->Adaptor,
                                       constraint,
                                       (EnsPAssemblymapper) NULL,
                                       (EnsPSlice) NULL,
                                       qcdbs);

    if (ajListGetLength(qcdbs) > 1)
        ajWarn("ensQcdatabaseadaptorFetchByName got more than "
               "one Ensembl Quality Check Database for (UNIQUE) name '%S' "
               "and release '%S'.\n", name, release);

    ajListPop(qcdbs, (void **) Pqcdb);

    qcdatabaseadaptorCacheInsert(qcdba, Pqcdb);

    while (ajListPop(qcdbs, (void **) &qcdb))
    {
        qcdatabaseadaptorCacheInsert(qcdba, &qcdb);

        ensQcdatabaseDel(&qcdb);
    }

    ajListFree(&qcdbs);

    ajStrDel(&constraint);

    return ajTrue;
}




/* @section object access *****************************************************
**
** Functions for accessing Ensembl Quality Check Database objects in an
** Ensembl SQL database.
**
** @fdata [EnsPQcdatabaseadaptor]
**
** @nam3rule Delete Delete Ensembl Quality Check Database object(s)
** @nam3rule Store  Store Ensembl Quality Check Database object(s)
** @nam3rule Update Update Ensembl Quality Check Database object(s)
**
** @argrule * qcdba [EnsPQcdatabaseadaptor]
** Ensembl Quality Check Database Adaptor
** @argrule Delete qcdb [EnsPQcdatabase]
** Ensembl Quality Check Database
** @argrule Store qcdb [EnsPQcdatabase]
** Ensembl Quality Check Database
** @argrule Update qcdb [const EnsPQcdatabase]
** Ensembl Quality Check Database
**
** @valrule * [AjBool] ajTrue upon success, ajFalse otherwise
**
** @fcategory use
******************************************************************************/




/* @func ensQcdatabaseadaptorDelete *******************************************
**
** Delete an Ensembl Quality Check Database.
**
** @param [u] qcdba [EnsPQcdatabaseadaptor]
** Ensembl Quality Check Database Adaptor
** @param [u] qcdb [EnsPQcdatabase] Ensembl Quality Check Database
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
**
** @release 6.2.0
** @@
******************************************************************************/

AjBool ensQcdatabaseadaptorDelete(EnsPQcdatabaseadaptor qcdba,
                                  EnsPQcdatabase qcdb)
{
    AjBool result = AJFALSE;

    AjPSqlstatement sqls = NULL;

    AjPStr statement = NULL;

    EnsPDatabaseadaptor dba = NULL;

    if (!qcdba)
        return ajFalse;

    if (!qcdb)
        return ajFalse;

    if (!ensQcdatabaseGetIdentifier(qcdb))
        return ajFalse;

    /* Remove this object from the cache. */

    qcdatabaseadaptorCacheRemove(qcdba, qcdb);

    dba = ensBaseadaptorGetDatabaseadaptor(qcdba->Adaptor);

    statement = ajFmtStr(
        "DELETE FROM "
        "sequence_db "
        "WHERE "
        "sequence_db.sequence_db_id = %u",
        qcdb->Identifier);

    sqls = ensDatabaseadaptorSqlstatementNew(dba, statement);

    if (ajSqlstatementGetAffectedrows(sqls))
    {
        qcdb->Adaptor    = (EnsPQcdatabaseadaptor) NULL;
        qcdb->Identifier = 0;

        result = ajTrue;
    }

    ensDatabaseadaptorSqlstatementDel(dba, &sqls);

    ajStrDel(&statement);

    return result;
}




/* @func ensQcdatabaseadaptorStore ********************************************
**
** Store an Ensembl Quality Check Database.
**
** @param [u] qcdba [EnsPQcdatabaseadaptor]
** Ensembl Quality Check Database Adaptor
** @param [u] qcdb [EnsPQcdatabase] Ensembl Quality Check Database
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
**
** @release 6.2.0
** @@
******************************************************************************/

AjBool ensQcdatabaseadaptorStore(EnsPQcdatabaseadaptor qcdba,
                                 EnsPQcdatabase qcdb)
{
    char *txtname        = NULL;
    char *txtrelease     = NULL;
    char *txtdate        = NULL;
    char *txtformat      = NULL;
    char *txtspecies     = NULL;
    char *txthost        = NULL;
    char *txtdirectory   = NULL;
    char *txtfile        = NULL;
    char *txtexternalurl = NULL;

    AjBool result = AJFALSE;

    AjPSqlstatement sqls = NULL;

    AjPStr statement = NULL;

    EnsPDatabaseadaptor dba = NULL;

    if (!qcdba)
        return ajFalse;

    if (!qcdb)
        return ajFalse;

    if (ensQcdatabaseGetAdaptor(qcdb) &&
        ensQcdatabaseGetIdentifier(qcdb))
        return ajFalse;

    dba = ensBaseadaptorGetDatabaseadaptor(qcdba->Adaptor);

    ensDatabaseadaptorEscapeC(dba, &txtname, qcdb->Name);
    ensDatabaseadaptorEscapeC(dba, &txtrelease, qcdb->Release);
    ensDatabaseadaptorEscapeC(dba, &txtdate, qcdb->Date);
    ensDatabaseadaptorEscapeC(dba, &txtformat, qcdb->Format);
    ensDatabaseadaptorEscapeC(dba, &txtspecies, qcdb->DatabaseadaptorSpecies);
    ensDatabaseadaptorEscapeC(dba, &txthost, qcdb->Host);
    ensDatabaseadaptorEscapeC(dba, &txtdirectory, qcdb->Directory);
    ensDatabaseadaptorEscapeC(dba, &txtfile, qcdb->File);
    ensDatabaseadaptorEscapeC(dba, &txtexternalurl, qcdb->UrlExternal);

    statement = ajFmtStr(
        "INSERT IGNORE INTO "
        "sequence_db "
        "SET "
        "sequence_db.analysis_id = %u, "
        "sequence_db.name = '%s', "
        "sequence_db.dbrelease = '%s', "
        "sequence_db.date = '%s', "
        "sequence_db.format = '%s' "
        "sequence_db.type = '%s', "
        "sequence_db.class = '%s', "
        "sequence_db.species = '%s', "
        "sequence_db.dbgroup = '%s', "
        "sequence_db.host = '%s', "
        "sequence_db.directory = '%s', "
        "sequence_db.file = '%s', "
        "sequence_db.external_url = '%s'",
        ensAnalysisGetIdentifier(qcdb->Analysis),
        txtname,
        txtrelease,
        txtdate,
        txtformat,
        ensQcdatabaseTypeToChar(qcdb->Type),
        ensQcdatabaseClassToChar(qcdb->Class),
        txtspecies,
        ensDatabaseadaptorGroupToChar(qcdb->DatabaseadaptorGroup),
        txthost,
        txtdirectory,
        txtfile,
        txtexternalurl);

    ajCharDel(&txtname);
    ajCharDel(&txtrelease);
    ajCharDel(&txtdate);
    ajCharDel(&txtformat);
    ajCharDel(&txtspecies);
    ajCharDel(&txthost);
    ajCharDel(&txtdirectory);
    ajCharDel(&txtfile);
    ajCharDel(&txtexternalurl);

    sqls = ensDatabaseadaptorSqlstatementNew(dba, statement);

    if (ajSqlstatementGetAffectedrows(sqls))
    {
        ensQcdatabaseSetIdentifier(qcdb, ajSqlstatementGetIdentifier(sqls));

        ensQcdatabaseSetAdaptor(qcdb, qcdba);

        /* Insert into the cache. */

        qcdatabaseadaptorCacheInsert(qcdba, &qcdb);

        result = ajTrue;
    }

    ensDatabaseadaptorSqlstatementDel(dba, &sqls);

    ajStrDel(&statement);

    return result;
}




/* @func ensQcdatabaseadaptorUpdate *******************************************
**
** Update an Ensembl Quality Check Database.
**
** @param [u] qcdba [EnsPQcdatabaseadaptor]
** Ensembl Quality Check Database Adaptor
** @param [r] qcdb [const EnsPQcdatabase] Ensembl Quality Check Database
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
**
** @release 6.2.0
** @@
******************************************************************************/

AjBool ensQcdatabaseadaptorUpdate(EnsPQcdatabaseadaptor qcdba,
                                  const EnsPQcdatabase qcdb)
{
    char *txtname        = NULL;
    char *txtrelease     = NULL;
    char *txtdate        = NULL;
    char *txtformat      = NULL;
    char *txtspecies     = NULL;
    char *txthost        = NULL;
    char *txtdirectory   = NULL;
    char *txtfile        = NULL;
    char *txtexternalurl = NULL;

    AjBool result = AJFALSE;

    AjPSqlstatement sqls = NULL;

    AjPStr statement = NULL;

    EnsPDatabaseadaptor dba = NULL;

    if (!qcdba)
        return ajFalse;

    if (!qcdb)
        return ajFalse;

    if (!ensQcdatabaseGetIdentifier(qcdb))
        return ajFalse;

    dba = ensBaseadaptorGetDatabaseadaptor(qcdba->Adaptor);

    ensDatabaseadaptorEscapeC(dba, &txtname, qcdb->Name);
    ensDatabaseadaptorEscapeC(dba, &txtrelease, qcdb->Release);
    ensDatabaseadaptorEscapeC(dba, &txtdate, qcdb->Date);
    ensDatabaseadaptorEscapeC(dba, &txtformat, qcdb->Format);
    ensDatabaseadaptorEscapeC(dba, &txtspecies, qcdb->DatabaseadaptorSpecies);
    ensDatabaseadaptorEscapeC(dba, &txthost, qcdb->Host);
    ensDatabaseadaptorEscapeC(dba, &txtdirectory, qcdb->Directory);
    ensDatabaseadaptorEscapeC(dba, &txtfile, qcdb->File);
    ensDatabaseadaptorEscapeC(dba, &txtexternalurl, qcdb->UrlExternal);

    statement = ajFmtStr(
        "UPDATE IGNORE "
        "sequence_db "
        "SET "
        "sequence_db.analysis_id = %u, "
        "sequence_db.name = '%s', "
        "sequence_db.dbrelease = '%s', "
        "sequence_db.date = '%s', "
        "sequence_db.format = '%s' "
        "sequence_db.type = '%s', "
        "sequence_db.class = '%s', "
        "sequence_db.species = '%s', "
        "sequence_db.dbgroup = '%s', "
        "sequence_db.host = '%s', "
        "sequence_db.directory = '%s', "
        "sequence_db.file = '%s', "
        "sequence_db.external_url = '%s' "
        "WHERE "
        "sequence_db.sequence_db_id = %u",
        ensAnalysisGetIdentifier(qcdb->Analysis),
        txtname,
        txtrelease,
        txtdate,
        txtformat,
        ensQcdatabaseTypeToChar(qcdb->Type),
        ensQcdatabaseClassToChar(qcdb->Class),
        txtspecies,
        ensDatabaseadaptorGroupToChar(qcdb->DatabaseadaptorGroup),
        txthost,
        txtdirectory,
        txtfile,
        txtexternalurl,
        qcdb->Identifier);

    ajCharDel(&txtname);
    ajCharDel(&txtrelease);
    ajCharDel(&txtdate);
    ajCharDel(&txtformat);
    ajCharDel(&txtspecies);
    ajCharDel(&txthost);
    ajCharDel(&txtdirectory);
    ajCharDel(&txtfile);
    ajCharDel(&txtexternalurl);

    sqls = ensDatabaseadaptorSqlstatementNew(dba, statement);

    if (ajSqlstatementGetAffectedrows(sqls))
        result = ajTrue;

    ensDatabaseadaptorSqlstatementDel(dba, &sqls);

    ajStrDel(&statement);

    return result;
}
