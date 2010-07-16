/******************************************************************************
**
** @source Ensembl Quality Check Database functions
**
** @author Copyright (C) 1999 Ensembl Developers
** @author Copyright (C) 2006 Michael K. Schuster
** @modified 2009 by Alan Bleasby for incorporation into EMBOSS core
** @version $Revision: 1.15 $
** @@
**
** This library is free software; you can redistribute it and/or
** modify it under the terms of the GNU Library General Public
** License as published by the Free Software Foundation; either
** version 2 of the License, or (at your option) any later version.
**
** This library is distributed in the hope that it will be useful,
** but WITHOUT ANY WARRANTY; without even the implied warranty of
** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
** Library General Public License for more details.
**
** You should have received a copy of the GNU Library General Public
** License along with this library; if not, write to the
** Free Software Foundation, Inc., 59 Temple Place - Suite 330,
** Boston, MA  02111-1307, USA.
******************************************************************************/

/* ==================================================================== */
/* ========================== include files =========================== */
/* ==================================================================== */

#include "ensqcdatabase.h"




/* ==================================================================== */
/* ========================== private data ============================ */
/* ==================================================================== */

/* qcdatabaseClass ************************************************************
**
** The Ensembl Quality Check Database class element is enumerated in
** both, the SQL table definition and the data structure. The following strings
** are used for conversion in database operations and correspond to
** EnsEQcdatabaseClass.
**
******************************************************************************/

static const char *qcdatabaseClass[] =
{
    (const char *) NULL,
    "unknown",
    "reference",
    "test",
    "both",
    "genome",
    (const char *) NULL
};




/* qcdatabaseType *************************************************************
**
** The Ensembl Quality Check Database type element is enumerated in
** both, the SQL table definition and the data structure. The following strings
** are used for conversion in database operations and correspond to
** EnsEQcdatabaseType.
**
******************************************************************************/

static const char *qcdatabaseType[] =
{
    (const char *) NULL,
    "unknown",
    "dna",
    "protein",
    (const char *) NULL
};




/* ==================================================================== */
/* ======================== private functions ========================= */
/* ==================================================================== */

static AjBool qcdatabaseadaptorFetchAllBySQL(EnsPDatabaseadaptor dba,
                                             const AjPStr statement,
                                             EnsPAssemblymapper am,
                                             EnsPSlice slice,
                                             AjPList qcdbs);

static AjBool qcdatabaseadaptorCacheInsert(EnsPQcdatabaseadaptor qcdba,
                                           EnsPQcdatabase *Pqcdb);

static AjBool qcdatabaseadaptorCacheInit(EnsPQcdatabaseadaptor qcdba);

static void qcdatabaseadaptorCacheClearIdentifier(void **key,
                                                  void **value,
                                                  void *cl);

static void qcdatabaseadaptorCacheClearName(void **key,
                                            void **value,
                                            void *cl);

static AjBool qcdatabaseadaptorCacheExit(EnsPQcdatabaseadaptor qcdba);




/* @filesection ensqcdatabase *************************************************
**
** @nam1rule ens Function belongs to the Ensembl library
**
******************************************************************************/




/* @datasection [EnsPQcdatabase] QC Database **********************************
**
** Functions for manipulating Ensembl QC Database objects
**
** Bio::EnsEMBL::QC::SequenceDB CVS Revision:
**
** @nam2rule Qcdatabase
**
******************************************************************************/




/* @section constructors ******************************************************
**
** All constructors return a new Ensembl QC Database by pointer.
** It is the responsibility of the user to first destroy any previous
** QC Database. The target pointer does not need to be initialised to
** NULL, but it is good programming practice to do so anyway.
**
** @fdata [EnsPQcdatabase]
** @fnote None
**
** @nam3rule New Constructor
** @nam4rule NewObj Constructor with existing object
** @nam4rule NewRef Constructor by incrementing the reference counter
**
** @argrule Obj object [EnsPQcdatabase] Ensembl QC Database
** @argrule Ref object [EnsPQcdatabase] Ensembl QC Database
**
** @valrule * [EnsPQcdatabase] Ensembl QC Database
**
** @fcategory new
******************************************************************************/




/* @func ensQcdatabaseNew *****************************************************
**
** Default constructor for an Ensembl QC Database.
**
** @cc Bio::EnsEMBL::Storable::new
** @param [u] qcdba [EnsPQcdatabaseadaptor] Ensembl QC Database Adaptor
** @param [r] identifier [ajuint] SQL database-internal identifier
** @cc Bio::EnsEMBL::QC::SequenceDB::new
** @param [u] analysis [EnsPAnalysis] Ensembl Analysis
** @param [u] name [AjPStr] Name
** @param [u] release [AjPStr] Release
** @param [u] date [AjPStr] Date
** @param [u] format [AjPStr] Format
** @param [r] class [EnsEQcdatabaseClass] Class
** @param [r] type [EnsEQcdatabaseType] Type
** @param [u] species [AjPStr] Ensembl Database Adaptor species
** @param [r] group [EnsEDatabaseadaptorGroup] Ensembl Database Adaptor group
** @param [u] host [AjPStr] Host
** @param [u] directory [AjPStr] Directory
** @param [u] file [AjPStr] File
** @param [u] externalurl [AjPStr] External URL
**
** @return [EnsPQcdatabase] Ensembl QC Database or NULL
** @@
******************************************************************************/

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
                                AjPStr externalurl)
{
    EnsPQcdatabase qcdb = NULL;

    if(!analysis)
        return NULL;

    if(!name)
        return NULL;

    if(!release)
        return NULL;

    AJNEW0(qcdb);

    qcdb->Use = 1;

    qcdb->Identifier = identifier;

    qcdb->Adaptor = qcdba;

    qcdb->Analysis = ensAnalysisNewRef(analysis);

    if(name)
        qcdb->Name = ajStrNewRef(name);

    if(release)
        qcdb->Release = ajStrNewRef(release);

    if(date)
        qcdb->Date = ajStrNewRef(date);

    if(format)
        qcdb->Format = ajStrNewRef(format);

    qcdb->Class = class;

    qcdb->Type = type;

    if(species)
        qcdb->Species = ajStrNewRef(species);

    qcdb->Group = group;

    if(host)
        qcdb->Host = ajStrNewRef(host);

    if(directory)
        qcdb->Directory = ajStrNewRef(directory);

    if(file)
        qcdb->File = ajStrNewRef(file);

    if(externalurl)
        qcdb->ExternalURL = ajStrNewRef(externalurl);

    qcdb->InternalURL = NULL;

    return qcdb;
}




/* @func ensQcdatabaseNewObj **************************************************
**
** Object-based constructor function, which returns an independent object.
**
** @param [r] object [const EnsPQcdatabase] Ensembl QC Database
**
** @return [EnsPQcdatabase] Ensembl QC Database or NULL
** @@
******************************************************************************/

EnsPQcdatabase ensQcdatabaseNewObj(const EnsPQcdatabase object)
{
    EnsPQcdatabase qcdb = NULL;

    if(!object)
        return NULL;

    AJNEW0(qcdb);

    qcdb->Use = 1;

    qcdb->Identifier = object->Identifier;

    qcdb->Adaptor = object->Adaptor;

    if(object->Analysis)
        qcdb->Analysis = ensAnalysisNewRef(object->Analysis);

    if(object->Name)
        qcdb->Name = ajStrNewRef(object->Name);

    if(object->Release)
        qcdb->Release = ajStrNewRef(object->Release);

    if(object->Date)
        qcdb->Date = ajStrNewRef(object->Date);

    if(object->Format)
        qcdb->Format = ajStrNewRef(object->Format);

    qcdb->Class = object->Class;

    qcdb->Type = object->Type;

    if(object->Species)
        qcdb->Species = ajStrNewRef(object->Species);

    qcdb->Group = object->Group;

    if(object->Host)
        qcdb->Host = ajStrNewRef(object->Host);

    if(object->Directory)
        qcdb->Directory = ajStrNewRef(object->Directory);

    if(object->File)
        qcdb->File = ajStrNewRef(object->File);

    if(object->ExternalURL)
        qcdb->ExternalURL = ajStrNewRef(object->ExternalURL);

    if(object->InternalURL)
        qcdb->InternalURL = ajStrNewRef(object->InternalURL);

    return qcdb;
}




/* @func ensQcdatabaseNewRef **************************************************
**
** Ensembl Object referencing function, which returns a pointer to the
** Ensembl Object passed in and increases its reference count.
**
** @param [u] qcdb [EnsPQcdatabase] Ensembl QC Database
**
** @return [EnsPQcdatabase] Ensembl QC Database or NULL
** @@
******************************************************************************/

EnsPQcdatabase ensQcdatabaseNewRef(EnsPQcdatabase qcdb)
{
    if(!qcdb)
        return NULL;

    qcdb->Use++;

    return qcdb;
}




/* @section destructors *******************************************************
**
** Destruction destroys all internal data structures and frees the
** memory allocated for the Ensembl QC Database.
**
** @fdata [EnsPQcdatabase]
** @fnote None
**
** @nam3rule Del Destroy (free) a QC Database object
**
** @argrule * Pqcdb [EnsPQcdatabase*] Ensembl QC Database object address
**
** @valrule * [void]
**
** @fcategory delete
******************************************************************************/




/* @func ensQcdatabaseDel *****************************************************
**
** Default destructor for an Ensembl QC Database.
**
** @param [d] Pqcdb [EnsPQcdatabase*] Ensembl QC Database address
**
** @return [void]
** @@
******************************************************************************/

void ensQcdatabaseDel(EnsPQcdatabase *Pqcdb)
{
    EnsPQcdatabase pthis = NULL;

    if(ajDebugTest("ensQcdatabaseDel"))
    {
        ajDebug("ensQcdatabaseDel\n"
                "  *Pqcdb %p\n",
                *Pqcdb);

        ensQcdatabaseTrace(*Pqcdb, 1);
    }

    if(!Pqcdb)
        return;

    if(!*Pqcdb)
        return;

    pthis = *Pqcdb;

    pthis->Use--;

    if(pthis->Use)
    {
        *Pqcdb = NULL;

        return;
    }

    ensAnalysisDel(&pthis->Analysis);

    ajStrDel(&pthis->Name);
    ajStrDel(&pthis->Release);
    ajStrDel(&pthis->Date);
    ajStrDel(&pthis->Format);
    ajStrDel(&pthis->Species);
    ajStrDel(&pthis->Host);
    ajStrDel(&pthis->Directory);
    ajStrDel(&pthis->File);
    ajStrDel(&pthis->ExternalURL);
    ajStrDel(&pthis->InternalURL);

    AJFREE(pthis);

    *Pqcdb = NULL;

    return;
}




/* @section element retrieval *************************************************
**
** Functions for returning elements of an Ensembl QC Database object.
**
** @fdata [EnsPQcdatabase]
** @fnote None
**
** @nam3rule Get Return QC Database attribute(s)
** @nam4rule GetAdaptor Return the Ensembl QC Database Adaptor
** @nam4rule GetIdentifier Return the SQL database-internal identifier
**
** @argrule * qcdb [const EnsPQcdatabase] QC Database
**
** @valrule Adaptor [EnsPQcdatabaseadaptor] Ensembl QC Database Adaptor
** @valrule Identifier [ajuint] SQL database-internal identifier
** @valrule Analysis [EnsPAnalysis] Ensembl Analysis
** @valrule Name [AjPStr] Name
** @valrule Release [AjPStr] Release
** @valrule Date [AjPStr] Date
** @valrule Format [AjPStr] Format
** @valrule Class [EnsEQcdatabaseClass] Class
** @valrule Type [EnsEQcdatabaseType] Type
** @valrule Species [AjPStr] Ensembl Database Adaptor species
** @valrule Group [EnsEDatabaseadaptorGroup] Ensembl Database Adaptor group
** @valrule Host [AjPStr] Host
** @valrule Directory [AjPStr] Directory
** @valrule File [AjPStr] File
** @valrule ExternalURL [AjPStr] External URL
** @valrule InternalURL [AjPStr] Internal URL
**
** @fcategory use
******************************************************************************/




/* @func ensQcdatabaseGetAdaptor **********************************************
**
** Get the Ensembl QC Database Adaptor element of an Ensembl QC Database.
**
** @param [r] qcdb [const EnsPQcdatabase] Ensembl QC Database
**
** @return [EnsPQcdatabaseadaptor] Ensembl QC Database Adaptor
** @@
******************************************************************************/

EnsPQcdatabaseadaptor ensQcdatabaseGetAdaptor(const EnsPQcdatabase qcdb)
{
    if(!qcdb)
        return NULL;

    return qcdb->Adaptor;
}




/* @func ensQcdatabaseGetIdentifier *******************************************
**
** Get the SQL database-internal identifier element of an Ensembl QC Database.
**
** @param [r] qcdb [const EnsPQcdatabase] Ensembl QC Database
**
** @return [ajuint] SQL database-internal identifier
** @@
******************************************************************************/

ajuint ensQcdatabaseGetIdentifier(const EnsPQcdatabase qcdb)
{
    if(!qcdb)
        return 0;

    return qcdb->Identifier;
}




/* @func ensQcdatabaseGetAnalysis *********************************************
**
** Get the Ensembl Analysis element of an Ensembl QC Database.
**
** @param [r] qcdb [const EnsPQcdatabase] Ensembl QC Database
**
** @return [const EnsPAnalysis] Ensembl Analysis
** @@
******************************************************************************/

const EnsPAnalysis ensQcdatabaseGetAnalysis(const EnsPQcdatabase qcdb)
{
    if(!qcdb)
        return NULL;

    return qcdb->Analysis;
}




/* @func ensQcdatabaseGetName *************************************************
**
** Get the name element of an Ensembl QC Database.
**
** @param [r] qcdb [const EnsPQcdatabase] Ensembl QC Database
**
** @return [AjPStr] Name
** @@
******************************************************************************/

AjPStr ensQcdatabaseGetName(const EnsPQcdatabase qcdb)
{
    if(!qcdb)
        return NULL;

    return qcdb->Name;
}




/* @func ensQcdatabaseGetRelease **********************************************
**
** Get the release element of an Ensembl QC Database.
**
** @param [r] qcdb [const EnsPQcdatabase] Ensembl QC Database
**
** @return [AjPStr] Release
** @@
******************************************************************************/

AjPStr ensQcdatabaseGetRelease(const EnsPQcdatabase qcdb)
{
    if(!qcdb)
        return NULL;

    return qcdb->Release;
}




/* @func ensQcdatabaseGetDate *************************************************
**
** Get the date element of an Ensembl QC Database.
**
** @param [r] qcdb [const EnsPQcdatabase] Ensembl QC Database
**
** @return [AjPStr] Date
** @@
******************************************************************************/

AjPStr ensQcdatabaseGetDate(const EnsPQcdatabase qcdb)
{
    if(!qcdb)
        return NULL;

    return qcdb->Date;
}




/* @func ensQcdatabaseGetFormat ***********************************************
**
** Get the format element of an Ensembl QC Database.
**
** @param [r] qcdb [const EnsPQcdatabase] Ensembl QC Database
**
** @return [AjPStr] Format
** @@
******************************************************************************/

AjPStr ensQcdatabaseGetFormat(const EnsPQcdatabase qcdb)
{
    if(!qcdb)
        return NULL;

    return qcdb->Format;
}




/* @func ensQcdatabaseGetClass ************************************************
**
** Get the class element of an Ensembl QC Database.
**
** @param [r] qcdb [const EnsPQcdatabase] Ensembl QC Database
**
** @return [EnsEQcdatabaseClass] Class or ensEQcdatabaseClassNULL
** @@
******************************************************************************/

EnsEQcdatabaseClass ensQcdatabaseGetClass(const EnsPQcdatabase qcdb)
{
    if(!qcdb)
        return ensEQcdatabaseClassNULL;

    return qcdb->Class;
}




/* @func ensQcdatabaseGetType *************************************************
**
** Get the type element of an Ensembl QC Database.
**
** @param [r] qcdb [const EnsPQcdatabase] Ensembl QC Database
**
** @return [EnsEQcdatabaseType] Type or ensEQcdasfeatureTypeNULL
** @@
******************************************************************************/

EnsEQcdatabaseType ensQcdatabaseGetType(const EnsPQcdatabase qcdb)
{
    if(!qcdb)
        return ensEQcdatabaseTypeNULL;

    return qcdb->Type;
}




/* @func ensQcdatabaseGetSpecies **********************************************
**
** Get the Ensembl Database Adaptor species element of an Ensembl QC Database.
**
** @param [r] qcdb [const EnsPQcdatabase] Ensembl QC Database
**
** @return [AjPStr] Ensembl Database Adaptor species
** @@
******************************************************************************/

AjPStr ensQcdatabaseGetSpecies(const EnsPQcdatabase qcdb)
{
    if(!qcdb)
        return NULL;

    return qcdb->Species;
}




/* @func ensQcdatabaseGetGroup ************************************************
**
** Get the Ensembl Database Adaptor group element of an Ensembl QC Database.
**
** @param [r] qcdb [const EnsPQcdatabase] Ensembl QC Database
**
** @return [EnsEDatabaseadaptorGroup] Ensembl Database Adaptor group or
**                                    ensEDatabaseadaptorGroupNULL
** @@
******************************************************************************/

EnsEDatabaseadaptorGroup ensQcdatabaseGetGroup(const EnsPQcdatabase qcdb)
{
    if(!qcdb)
        return ensEDatabaseadaptorGroupNULL;

    return qcdb->Group;
}




/* @func ensQcdatabaseGetHost *************************************************
**
** Get the host element of an Ensembl QC Database.
**
** @param [r] qcdb [const EnsPQcdatabase] Ensembl QC Database
**
** @return [AjPStr] Host
** @@
******************************************************************************/

AjPStr ensQcdatabaseGetHost(const EnsPQcdatabase qcdb)
{
    if(!qcdb)
        return NULL;

    return qcdb->Host;
}




/* @func ensQcdatabaseGetDirectory ********************************************
**
** Get the directory element of an Ensembl QC Database.
**
** @param [r] qcdb [const EnsPQcdatabase] Ensembl QC Database
**
** @return [AjPStr] Directory
** @@
******************************************************************************/

AjPStr ensQcdatabaseGetDirectory(const EnsPQcdatabase qcdb)
{
    if(!qcdb)
        return NULL;

    return qcdb->Directory;
}




/* @func ensQcdatabaseGetFile *************************************************
**
** Get the file element of an Ensembl QC Database.
**
** @param [r] qcdb [const EnsPQcdatabase] Ensembl QC Database
**
** @return [AjPStr] File
** @@
******************************************************************************/

AjPStr ensQcdatabaseGetFile(const EnsPQcdatabase qcdb)
{
    if(!qcdb)
        return NULL;

    return qcdb->File;
}




/* @func ensQcdatabaseGetExternalURL ******************************************
**
** Get the external URL element of an Ensembl QC Database.
**
** @param [r] qcdb [const EnsPQcdatabase] Ensembl QC Database
**
** @return [AjPStr] External URL
** @@
******************************************************************************/

AjPStr ensQcdatabaseGetExternalURL(const EnsPQcdatabase qcdb)
{
    if(!qcdb)
        return NULL;

    return qcdb->ExternalURL;
}




/* @func ensQcdatabaseGetInternalURL ******************************************
**
** Get the internal URL element of an Ensembl QC Database.
**
** @param [r] qcdb [const EnsPQcdatabase] Ensembl QC Database
**
** @return [AjPStr] Internal URL
** @@
******************************************************************************/

AjPStr ensQcdatabaseGetInternalURL(const EnsPQcdatabase qcdb)
{
    if(!qcdb)
        return NULL;

    return qcdb->InternalURL;
}




/* @section modifiers *********************************************************
**
** Functions for assigning elements of an Ensembl QC Database object.
**
** @fdata [EnsPQcdatabase]
** @fnote None
**
** @nam3rule Set Set one element of a QC Database
** @nam4rule SetAdaptor Set the Ensembl QC Database Adaptor
** @nam4rule SetIdentifier Set the SQL database-internal identifier
** @nam4rule SetName Set the name
** @nam4rule SetRelease Set the release
** @nam4rule SetDate Set the date
** @nam4rule SetFormat Set the format
** @nam4rule SetClass Set the class
** @nam4rule SetType Set the type
** @nam4rule SetSpecies Set the Ensembl Database Adaptor species
** @nam4rule SetGroup Set the Ensembl Database Adaptor group
** @nam4rule SetHost Set the host
** @nam4rule SetDirectory Set the directory
** @nam4rule SetFile Set the file
** @nam4rule SetExternalURL Set the external URL
** @nam4rule SetInternalURL Set the internal URL
**
** @argrule * qcdb [EnsPQcdatabase] Ensembl QC Database
**
** @valrule * [AjBool] ajTrue upon success, ajFalse otherwise
**
** @fcategory modify
******************************************************************************/




/* @func ensQcdatabaseSetAdaptor **********************************************
**
** Set the Ensembl QC Database Adaptor element of an Ensembl QC Database.
**
** @param [u] qcdb [EnsPQcdatabase] Ensembl QC Database
** @param [r] qcdba [EnsPQcdatabaseadaptor] Ensembl QC Database Adaptor
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
** @@
******************************************************************************/

AjBool ensQcdatabaseSetAdaptor(EnsPQcdatabase qcdb, EnsPQcdatabaseadaptor qcdba)
{
    if(!qcdb)
        return ajFalse;

    qcdb->Adaptor = qcdba;

    return ajTrue;
}




/* @func ensQcdatabaseSetIdentifier *******************************************
**
** Set the SQL database-internal identifier element of an Ensembl QC Database.
**
** @param [u] qcdb [EnsPQcdatabase] Ensembl QC Database
** @param [r] identifier [ajuint] SQL database-internal identifier
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
** @@
******************************************************************************/

AjBool ensQcdatabaseSetIdentifier(EnsPQcdatabase qcdb, ajuint identifier)
{
    if(!qcdb)
        return ajFalse;

    qcdb->Identifier = identifier;

    return ajTrue;
}




/* @func ensQcdatabaseSetAnalysis *********************************************
**
** Set the Ensembl Analysis element of an Ensembl QC Database.
**
** @param [u] qcdb [EnsPQcdatabase] Ensembl QC Database
** @param [u] analysis [EnsPAnalysis] Ensembl Analysis
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
** @@
******************************************************************************/

AjBool ensQcdatabaseSetAnalysis(EnsPQcdatabase qcdb, EnsPAnalysis analysis)
{
    if(!qcdb)
        return ajFalse;

    ensAnalysisDel(&qcdb->Analysis);

    qcdb->Analysis = ensAnalysisNewRef(analysis);

    return ajTrue;
}




/* @func ensQcdatabaseSetName *************************************************
**
** Set the name element of an Ensembl QC Database.
**
** @param [u] qcdb [EnsPQcdatabase] Ensembl QC Database
** @param [u] name [AjPStr] Name
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
** @@
******************************************************************************/

AjBool ensQcdatabaseSetName(EnsPQcdatabase qcdb, AjPStr name)
{
    if(!qcdb)
        return ajFalse;

    ajStrDel(&qcdb->Name);

    qcdb->Name = ajStrNewRef(name);

    return ajTrue;
}




/* @func ensQcdatabaseSetRelease **********************************************
**
** Set the release element of an Ensembl QC Database.
**
** @param [u] qcdb [EnsPQcdatabase] Ensembl QC Database
** @param [u] release [AjPStr] Release
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
** @@
******************************************************************************/

AjBool ensQcdatabaseSetRelease(EnsPQcdatabase qcdb, AjPStr release)
{
    if(!qcdb)
        return ajFalse;

    ajStrDel(&qcdb->Release);

    qcdb->Release = ajStrNewRef(release);

    return ajTrue;
}




/* @func ensQcdatabaseSetDate *************************************************
**
** Set the date element of an Ensembl QC Database.
**
** @param [u] qcdb [EnsPQcdatabase] Ensembl QC Database
** @param [u] date [AjPStr] Date
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
** @@
******************************************************************************/

AjBool ensQcdatabaseSetDate(EnsPQcdatabase qcdb, AjPStr date)
{
    if(!qcdb)
        return ajFalse;

    ajStrDel(&qcdb->Date);

    qcdb->Date = ajStrNewRef(date);

    return ajTrue;
}




/* @func ensQcdatabaseSetFormat ***********************************************
**
** Set the format element of an Ensembl QC Database.
**
** @param [u] qcdb [EnsPQcdatabase] Ensembl QC Database
** @param [u] format [AjPStr] Format
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
** @@
******************************************************************************/

AjBool ensQcdatabaseSetFormat(EnsPQcdatabase qcdb, AjPStr format)
{
    if(!qcdb)
        return ajFalse;

    ajStrDel(&qcdb->Format);

    qcdb->Format = ajStrNewRef(format);

    return ajTrue;
}




/* @func ensQcdatabaseSetClass ************************************************
**
** Set the class element of an Ensembl QC Database.
**
** @param [u] qcdb [EnsPQcdatabase] Ensembl QC Database
** @param [r] class [EnsEQcdatabaseClass] Class
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
** @@
******************************************************************************/

AjBool ensQcdatabaseSetClass(EnsPQcdatabase qcdb, EnsEQcdatabaseClass class)
{
    if(!qcdb)
        return ajFalse;

    qcdb->Class = class;

    return ajTrue;
}




/* @func ensQcdatabaseSetType *************************************************
**
** Set the type element of an Ensembl QC Database.
**
** @param [u] qcdb [EnsPQcdatabase] Ensembl QC Database
** @param [r] type [EnsEQcdatabaseType] Type
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
** @@
******************************************************************************/

AjBool ensQcdatabaseSetType(EnsPQcdatabase qcdb, EnsEQcdatabaseType type)
{
    if(!qcdb)
        return ajFalse;

    qcdb->Type = type;

    return ajTrue;
}




/* @func ensQcdatabaseSetSpecies **********************************************
**
** Set the Ensembl Database Adaptor species element of an Ensembl QC Database.
**
** @param [u] qcdb [EnsPQcdatabase] Ensembl QC Database
** @param [u] species [AjPStr] Ensembl Database Adaptor species element
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
** @@
******************************************************************************/

AjBool ensQcdatabaseSetSpecies(EnsPQcdatabase qcdb, AjPStr species)
{
    if(!qcdb)
        return ajFalse;

    ajStrDel(&qcdb->Species);

    qcdb->Species = ajStrNewRef(species);

    return ajTrue;
}




/* @func ensQcdatabaseSetGroup ************************************************
**
** Set the Ensembl Database Adaptor group element of an Ensembl QC Database.
**
** @param [u] qcdb [EnsPQcdatabase] Ensembl QC Database
** @param [u] group [EnsEDatabaseadaptorGroup] Ensembl Database Adaptor group
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
** @@
******************************************************************************/

AjBool ensQcdatabaseSetGroup(EnsPQcdatabase qcdb,
                             EnsEDatabaseadaptorGroup group)
{
    if(!qcdb)
        return ajFalse;

    qcdb->Group = group;

    return ajTrue;
}




/* @func ensQcdatabaseSetHost *************************************************
**
** Set the host element of an Ensembl QC Database.
**
** @param [u] qcdb [EnsPQcdatabase] Ensembl QC Database
** @param [u] host [AjPStr] Host
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
** @@
******************************************************************************/

AjBool ensQcdatabaseSetHost(EnsPQcdatabase qcdb, AjPStr host)
{
    if(!qcdb)
        return ajFalse;

    ajStrDel(&qcdb->Host);

    qcdb->Host = ajStrNewRef(host);

    return ajTrue;
}




/* @func ensQcdatabaseSetDirectory ********************************************
**
** Set the directory element of an Ensembl QC Database.
**
** @param [u] qcdb [EnsPQcdatabase] Ensembl QC Database
** @param [u] directory [AjPStr] Directory
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
** @@
******************************************************************************/

AjBool ensQcdatabaseSetDirectory(EnsPQcdatabase qcdb, AjPStr directory)
{
    if(!qcdb)
        return ajFalse;

    ajStrDel(&qcdb->Directory);

    qcdb->Directory = ajStrNewRef(directory);

    return ajTrue;
}




/* @func ensQcdatabaseSetFile *************************************************
**
** Set the file element of an Ensembl QC Database.
**
** @param [u] qcdb [EnsPQcdatabase] Ensembl QC Database
** @param [u] file [AjPStr] File
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
** @@
******************************************************************************/

AjBool ensQcdatabaseSetFile(EnsPQcdatabase qcdb, AjPStr file)
{
    if(!qcdb)
        return ajFalse;

    ajStrDel(&qcdb->File);

    qcdb->File = ajStrNewRef(file);

    return ajTrue;
}




/* @func ensQcdatabaseSetExternalURL ******************************************
**
** Set the external URL element of an Ensembl QC Database.
**
** @param [u] qcdb [EnsPQcdatabase] Ensembl QC Database
** @param [u] url [AjPStr] External URL
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
** @@
******************************************************************************/

AjBool ensQcdatabaseSetExternalURL(EnsPQcdatabase qcdb, AjPStr url)
{
    if(!qcdb)
        return ajFalse;

    ajStrDel(&qcdb->ExternalURL);

    qcdb->ExternalURL = ajStrNewRef(url);

    return ajTrue;
}




/* @func ensQcdatabaseSetInternalURL ******************************************
**
** Set the internal URL element of an Ensembl QC Database.
**
** @param [u] qcdb [EnsPQcdatabase] Ensembl QC Database
** @param [u] url [AjPStr] Internal URL
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
** @@
******************************************************************************/

AjBool ensQcdatabaseSetInternalURL(EnsPQcdatabase qcdb, AjPStr url)
{
    if(!qcdb)
        return ajFalse;

    ajStrDel(&qcdb->InternalURL);

    qcdb->InternalURL = ajStrNewRef(url);

    return ajTrue;
}




/* @func ensQcdatabaseGetMemsize **********************************************
**
** Get the memory size in bytes of an Ensembl QC Database.
**
** @param [r] qcdb [const EnsPQcdatabase] Ensembl QC Database
**
** @return [ajulong] Memory size
** @@
******************************************************************************/

ajulong ensQcdatabaseGetMemsize(const EnsPQcdatabase qcdb)
{
    ajulong size = 0;

    if(!qcdb)
        return 0;

    size += sizeof (EnsOQcdatabase);

    size += ensAnalysisGetMemsize(qcdb->Analysis);

    if(qcdb->Name)
    {
        size += sizeof (AjOStr);

        size += ajStrGetRes(qcdb->Name);
    }

    if(qcdb->Release)
    {
        size += sizeof (AjOStr);

        size += ajStrGetRes(qcdb->Release);
    }

    if(qcdb->Date)
    {
        size += sizeof (AjOStr);

        size += ajStrGetRes(qcdb->Date);
    }

    if(qcdb->Format)
    {
        size += sizeof (AjOStr);

        size += ajStrGetRes(qcdb->Format);
    }

    if(qcdb->Species)
    {
        size += sizeof (AjOStr);

        size += ajStrGetRes(qcdb->Species);
    }

    if(qcdb->Host)
    {
        size += sizeof (AjOStr);

        size += ajStrGetRes(qcdb->Host);
    }

    if(qcdb->Directory)
    {
        size += sizeof (AjOStr);

        size += ajStrGetRes(qcdb->Directory);
    }

    if(qcdb->File)
    {
        size += sizeof (AjOStr);

        size += ajStrGetRes(qcdb->File);
    }

    if(qcdb->ExternalURL)
    {
        size += sizeof (AjOStr);

        size += ajStrGetRes(qcdb->ExternalURL);
    }

    if(qcdb->InternalURL)
    {
        size += sizeof (AjOStr);

        size += ajStrGetRes(qcdb->InternalURL);
    }

    return size;
}




/* @section debugging *********************************************************
**
** Functions for reporting of an Ensembl QC Database object.
**
** @fdata [EnsPQcdatabase]
** @nam3rule Trace Report Ensembl QC Database elements to debug file
**
** @argrule Trace qcdb [const EnsPQcdatabase] Ensembl QC Database
** @argrule Trace level [ajuint] Indentation level
**
** @valrule * [AjBool] ajTrue upon success, ajFalse otherwise
**
** @fcategory misc
******************************************************************************/




/* @func ensQcdatabaseTrace ***************************************************
**
** Trace an Ensembl QC Database.
**
** @param [r] qcdb [const EnsPQcdatabase] Ensembl QC Database
** @param [r] level [ajuint] Indentation level
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
** @@
******************************************************************************/

AjBool ensQcdatabaseTrace(const EnsPQcdatabase qcdb, ajuint level)
{
    AjPStr indent = NULL;

    if(!qcdb)
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
            "%S  Species '%S'\n"
            "%S  Group '%s'\n"
            "%S  Host '%S'\n"
            "%S  Directory '%S'\n"
            "%S  File '%S'\n"
            "%S  ExternalURL %p\n"
            "%S  InternalURL '%S'\n",
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
            indent, qcdb->Species,
            indent, ensDatabaseadaptorGroupToChar(qcdb->Group),
            indent, qcdb->Host,
            indent, qcdb->Directory,
            indent, qcdb->File,
            indent, qcdb->ExternalURL,
            indent, qcdb->InternalURL);

    ensAnalysisTrace(qcdb->Analysis, 1);

    ajStrDel(&indent);

    return ajTrue;
}




/* @func ensQcdatabaseClassFromStr ********************************************
**
** Convert an AJAX String into an Ensembl QC Database class element.
**
** @param [r] class [const AjPStr] Class string
**
** @return [EnsEQcdatabaseClass] Ensembl QC Database class or
**                               ensEQcdatabaseClassNULL
** @@
******************************************************************************/

EnsEQcdatabaseClass ensQcdatabaseClassFromStr(const AjPStr class)
{
    register EnsEQcdatabaseClass i = ensEQcdatabaseClassNULL;

    EnsEQcdatabaseClass eclass = ensEQcdatabaseClassNULL;

    for(i = ensEQcdatabaseClassUnknown; qcdatabaseClass[i]; i++)
        if(ajStrMatchCaseC(class, qcdatabaseClass[i]))
            eclass = i;

    if(!eclass)
        ajDebug("ensQcdatabaseClassFromStr encountered "
                "unexpected string '%S'.\n", class);

    return eclass;
}




/* @func ensQcdatabaseTypeFromStr *********************************************
**
** Convert an AJAX String into an Ensembl QC Database type element.
**
** @param [r] type [const AjPStr] Type string
**
** @return [EnsEQcdatabaseType] Ensembl QC Database type or
**                              ensEQcdatabaseTypeNULL
** @@
******************************************************************************/

EnsEQcdatabaseType ensQcdatabaseTypeFromStr(const AjPStr type)
{
    register EnsEQcdatabaseType i = ensEQcdatabaseTypeNULL;

    EnsEQcdatabaseType etype = ensEQcdatabaseTypeNULL;

    for(i = ensEQcdatabaseTypeUnknown; qcdatabaseType[i]; i++)
        if(ajStrMatchCaseC(type, qcdatabaseType[i]))
            etype = i;

    if(!etype)
        ajDebug("ensQcdatabaseTypeFromStr encountered "
                "unexpected string '%S'.\n", type);

    return etype;
}




/* @func ensQcdatabaseClassToChar *********************************************
**
** Convert an Ensembl QC Database class element into a C-type (char*) string.
**
** @param [r] class [EnsEQcdatabaseClass] Ensembl QC Database class
**
** @return [const char*] Ensembl QC Database class C-type (char*) string
** @@
******************************************************************************/

const char* ensQcdatabaseClassToChar(EnsEQcdatabaseClass class)
{
    register EnsEQcdatabaseClass i = ensEQcdatabaseClassNULL;

    if(!class)
        return NULL;

    for(i = ensEQcdatabaseClassUnknown;
        qcdatabaseClass[i] && (i < class);
        i++);

    if(!qcdatabaseClass[i])
        ajDebug("ensQcdatabaseClassToChar encountered an "
                "out of boundary error on group %d.\n", class);

    return qcdatabaseClass[i];
}




/* @func ensQcdatabaseTypeToChar **********************************************
**
** Convert an Ensembl QC Database type element into a C-type (char*) string.
**
** @param [r] type [EnsEQcdatabaseType] Ensembl QC Database type
**
** @return [const char*] Ensembl QC Database type C-type (char*) string
** @@
******************************************************************************/

const char* ensQcdatabaseTypeToChar(EnsEQcdatabaseType type)
{
    register EnsEQcdatabaseType i = ensEQcdatabaseTypeNULL;

    if(!type)
        return NULL;

    for(i = ensEQcdatabaseTypeUnknown;
        qcdatabaseType[i] && (i < type);
        i++);

    if(!qcdatabaseType[i])
        ajDebug("ensQcdatabaseTypeToChar encountered an "
                "out of boundary error on group %d.\n", type);

    return qcdatabaseType[i];
}




/* @func ensQcdatabaseMatch ***************************************************
**
** Tests for matching two Ensembl QC Databases.
**
** @param [r] qcdb1 [const EnsPQcdatabase] First Ensembl QC Database
** @param [r] qcdb2 [const EnsPQcdatabase] Second Ensembl QC Database
**
** @return [AjBool] ajTrue if the Ensembl QC Databases are equal
** @@
** The comparison is based on initial pointer equality and if that fails
** each element is compared.
******************************************************************************/

AjBool ensQcdatabaseMatch(const EnsPQcdatabase qcdb1,
                          const EnsPQcdatabase qcdb2)
{
    if(!qcdb1)
        return ajFalse;

    if(!qcdb2)
        return ajFalse;

    if(qcdb1 == qcdb2)
        return ajTrue;

    if(qcdb1->Identifier != qcdb2->Identifier)
        return ajFalse;

    if((qcdb1->Adaptor && qcdb2->Adaptor) &&
       (qcdb1->Adaptor != qcdb2->Adaptor))
        return ajFalse;

    if(!ajStrMatchCaseS(qcdb1->Name, qcdb2->Name))
        return ajFalse;

    if(!ajStrMatchCaseS(qcdb1->Release, qcdb2->Release))
        return ajFalse;

    if(!ajStrMatchCaseS(qcdb1->Date, qcdb2->Date))
        return ajFalse;

    if(!ajStrMatchCaseS(qcdb1->Format, qcdb2->Format))
        return ajFalse;

    if(qcdb1->Class != qcdb2->Class)
        return ajFalse;

    if(qcdb1->Type != qcdb2->Type)
        return ajFalse;

    if(!ajStrMatchCaseS(qcdb1->Species, qcdb2->Species))
        return ajFalse;

    if(qcdb1->Group != qcdb2->Group)
        return ajFalse;

    if(!ajStrMatchCaseS(qcdb1->Host, qcdb2->Host))
        return ajFalse;

    if(!ajStrMatchCaseS(qcdb1->Directory, qcdb2->Directory))
        return ajFalse;

    if(!ajStrMatchCaseS(qcdb1->File, qcdb2->File))
        return ajFalse;

    if(!ajStrMatchCaseS(qcdb1->ExternalURL, qcdb2->ExternalURL))
        return ajFalse;

    if(!ajStrMatchCaseS(qcdb1->InternalURL, qcdb2->InternalURL))
        return ajFalse;

    return ajTrue;
}




/* @datasection [EnsPQcdatabaseadaptor] QC Database Adaptor *******************
**
** Functions for manipulating Ensembl QC Database Adaptor objects
**
** Bio::EnsEMBL::QC::DBSQL::SequenceDBAdaptor CVS Revision:
**
** @nam2rule Qcdatabaseadaptor
**
******************************************************************************/

static const char *qcdatabaseadaptorTables[] =
{
    "sequence_db",
    (const char *) NULL
};




static const char *qcdatabaseadaptorColumns[] =
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




static EnsOBaseadaptorLeftJoin qcdatabaseadaptorLeftJoin[] =
{
    {(const char*) NULL, (const char*) NULL}
};




static const char *qcdatabaseadaptorDefaultCondition =
    (const char*) NULL;




static const char *qcdatabaseadaptorFinalCondition =
    (const char *) NULL;




/* @funcstatic qcdatabaseadaptorFetchAllBySQL *********************************
**
** Run a SQL statement against an Ensembl Database Adaptor and consolidate the
** results into an AJAX List of Ensembl QC Database objects.
**
** @param [u] dba [EnsPDatabaseadaptor] Ensembl Database Adaptor
** @param [r] statement [const AjPStr] SQL statement
** @param [uN] am [EnsPAssemblymapper] Ensembl Assembly Mapper
** @param [uN] slice [EnsPSlice] Ensembl Slice
** @param [u] qcdbs [AjPList] AJAX List of Ensembl QC Databases
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
** @@
******************************************************************************/

static AjBool qcdatabaseadaptorFetchAllBySQL(EnsPDatabaseadaptor dba,
                                             const AjPStr statement,
                                             EnsPAssemblymapper am,
                                             EnsPSlice slice,
                                             AjPList qcdbs)
{
    ajuint identifier = 0;
    ajuint analysisid = 0;

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

    EnsEDatabaseadaptorGroup egroup = ensEDatabaseadaptorGroupNULL;
    EnsEQcdatabaseClass eclass      = ensEQcdatabaseClassNULL;
    EnsEQcdatabaseType etype        = ensEQcdatabaseTypeNULL;

    EnsPAnalysis analysis  = NULL;
    EnsPAnalysisadaptor aa = NULL;

    EnsPQcdatabase qcdb         = NULL;
    EnsPQcdatabaseadaptor qcdba = NULL;

    if(ajDebugTest("qcdatabaseadaptorFetchAllBySQL"))
        ajDebug("qcdatabaseadaptorFetchAllBySQL\n"
                "  dba %p\n"
                "  statement %p\n"
                "  am %p\n"
                "  slice %p\n"
                "  qcdbs %p\n",
                dba,
                statement,
                am,
                slice,
                qcdbs);

    if(!dba)
        return ajFalse;

    if(!statement)
        return ajFalse;

    if(!qcdbs)
        return ajFalse;

    aa = ensRegistryGetAnalysisadaptor(dba);

    qcdba = ensRegistryGetQcdatabaseadaptor(dba);

    sqls = ensDatabaseadaptorSqlstatementNew(dba, statement);

    sqli = ajSqlrowiterNew(sqls);

    while(!ajSqlrowiterDone(sqli))
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

        etype = ensQcdatabaseTypeFromStr(type);

        eclass = ensQcdatabaseClassFromStr(class);

        egroup = ensDatabaseadaptorGroupFromStr(group);

        qcdb = ensQcdatabaseNew(qcdba,
                                identifier,
                                analysis,
                                name,
                                release,
                                date,
                                format,
                                eclass,
                                etype,
                                species,
                                egroup,
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
** Insert an Ensembl QC Database into the QC Database Adaptor-internal cache.
** If a QC Database with the same name element is already present in the
** adaptor cache, the QC Database is deleted and a pointer to the cached
** QC Database is returned.
**
** @param [u] qcdba [EnsPQcdatabaseadaptor] Ensembl QC Database Adaptor
** @param [u] Pqcdb [EnsPQcdatabase*] Ensembl QC Database address
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
** @@
******************************************************************************/

static AjBool qcdatabaseadaptorCacheInsert(EnsPQcdatabaseadaptor qcdba,
                                           EnsPQcdatabase *Pqcdb)
{
    ajuint *Pidentifier = NULL;

    AjPStr key = NULL;

    EnsPQcdatabase qcdb1 = NULL;
    EnsPQcdatabase qcdb2 = NULL;

    if(!qcdba)
        return ajFalse;

    if(!qcdba->CacheByIdentifier)
        return ajFalse;

    if(!qcdba->CacheByName)
        return ajFalse;

    if(!Pqcdb)
        return ajFalse;

    if(!*Pqcdb)
        return ajFalse;

    /* Search the identifer cache. */

    qcdb1 = (EnsPQcdatabase) ajTableFetch(
        qcdba->CacheByIdentifier,
        (const void *) &((*Pqcdb)->Identifier));

    /* Search the name cache. */

    key = ajFmtStr("%S:%S", (*Pqcdb)->Name, (*Pqcdb)->Release);

    qcdb2 = (EnsPQcdatabase) ajTableFetch(
        qcdba->CacheByName,
        (const void *) key);

    if((!qcdb1) && (!qcdb2))
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

    if(qcdb1 && qcdb2 && (qcdb1 == qcdb2))
    {
        ajDebug("qcdatabaseadaptorCacheInsert replaced "
                "Ensembl QC Database %p with "
                "one already cached %p.\n",
                *Pqcdb, qcdb1);

        ensQcdatabaseDel(Pqcdb);

        Pqcdb = &qcdb1;
    }

    if(qcdb1 && qcdb2 && (qcdb1 != qcdb2))
        ajDebug("qcdatabaseadaptorCacheInsert detected "
                "Ensembl QC Databases in the "
                "identifier and name cache with identical names "
                "('%S' and '%S') but different addresses (%p and %p).\n",
                qcdb1->Name, qcdb2->Name, qcdb1, qcdb2);

    if(qcdb1 && (!qcdb2))
        ajDebug("qcdatabaseadaptorCacheInsert detected an "
                "Ensembl QC Database "
                "in the identifier, but not in the name cache.\n");

    if((!qcdb1) && qcdb2)
        ajDebug("qcdatabaseadaptorCacheInsert detected an "
                "Ensembl QC Database "
                "in the name, but not in the identifier cache.\n");

    ajStrDel(&key);

    return ajTrue;
}




/* @funcstatic qcdatabaseadaptorCacheInit *************************************
**
** Initialise the internal QC Database cache of an Ensembl QC Database Adaptor.
**
** @param [u] qcdba [EnsPQcdatabaseadaptor] Ensembl QC Database Adaptor
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
** @@
******************************************************************************/

static AjBool qcdatabaseadaptorCacheInit(EnsPQcdatabaseadaptor qcdba)
{
    AjPList qcdbs = NULL;

    EnsPQcdatabase qcdb = NULL;

    if(!qcdba)
        return ajFalse;

    if(qcdba->CacheByIdentifier)
        return ajFalse;
    else
        qcdba->CacheByIdentifier =
            ajTableNewFunctionLen(0, ensTableCmpUint, ensTableHashUint);

    if(qcdba->CacheByName)
        return ajFalse;
    else
        qcdba->CacheByName = ajTablestrNewLen(0);

    qcdbs = ajListNew();

    ensBaseadaptorGenericFetch(qcdba->Adaptor,
                               (const AjPStr) NULL,
                               (EnsPAssemblymapper) NULL,
                               (EnsPSlice) NULL,
                               qcdbs);

    while(ajListPop(qcdbs, (void **) &qcdb))
    {
        qcdatabaseadaptorCacheInsert(qcdba, &qcdb);

        ensQcdatabaseDel(&qcdb);
    }

    ajListFree(&qcdbs);

    return ajTrue;
}




/* @section constructors ******************************************************
**
** All constructors return a new Ensembl QC Database Adaptor by
** pointer. It is the responsibility of the user to first destroy any previous
** QC Database Adaptor. The target pointer does not need to be
** initialised to NULL, but it is good programming practice to do so anyway.
**
** @fdata [EnsPQcdatabaseadaptor]
** @fnote None
**
** @nam3rule New Constructor
**
** @argrule New dba [EnsPDatabaseadaptor] Ensembl Database Adaptor
**
** @valrule * [EnsPQcdatabaseadaptor] Ensembl QC Database Adaptor
**
** @fcategory new
******************************************************************************/




/* @func ensQcdatabaseadaptorNew **********************************************
**
** Default constructor for an Ensembl QC Database Adaptor.
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
** @return [EnsPQcdatabaseadaptor] Ensembl QC Database Adaptor or NULL
** @@
******************************************************************************/

EnsPQcdatabaseadaptor ensQcdatabaseadaptorNew(
    EnsPDatabaseadaptor dba)
{
    EnsPQcdatabaseadaptor qcdba = NULL;

    if(!dba)
        return NULL;

    AJNEW0(qcdba);

    qcdba->Adaptor = ensBaseadaptorNew(
        dba,
        qcdatabaseadaptorTables,
        qcdatabaseadaptorColumns,
        qcdatabaseadaptorLeftJoin,
        qcdatabaseadaptorDefaultCondition,
        qcdatabaseadaptorFinalCondition,
        qcdatabaseadaptorFetchAllBySQL);

    qcdatabaseadaptorCacheInit(qcdba);

    return qcdba;
}




/* @funcstatic qcdatabaseadaptorCacheClearIdentifier **************************
**
** An ajTableMapDel 'apply' function to clear the Ensembl QC Database
** Adaptor-internal QC Database cache. This function deletes the
** unsigned integer identifier key and the Ensembl QC Database value
** data.
**
** @param [u] key [void**] AJAX unsigned integer key data address
** @param [u] value [void**] Ensembl QC Database value data address
** @param [u] cl [void*] Standard, passed in from ajTableMapDel
** @see ajTableMapDel
**
** @return [void]
** @@
******************************************************************************/

static void qcdatabaseadaptorCacheClearIdentifier(void **key,
                                                  void **value,
                                                  void *cl)
{
    if(!key)
        return;

    if(!*key)
        return;

    if(!value)
        return;

    if(!*value)
        return;

    (void) cl;

    AJFREE(*key);

    ensQcdatabaseDel((EnsPQcdatabase *) value);

    return;
}




/* @funcstatic qcdatabaseadaptorCacheClearName ********************************
**
** An ajTableMapDel 'apply' function to clear the Ensembl QC Database
** Adaptor-internal QC Database cache. This function deletes the name
** AJAX String key data and the Ensembl QC Database value data.
**
** @param [u] key [void**] AJAX String key data address
** @param [u] value [void**] Ensembl QC Database value data address
** @param [u] cl [void*] Standard, passed in from ajTableMapDel
** @see ajTableMapDel
**
** @return [void]
** @@
******************************************************************************/

static void qcdatabaseadaptorCacheClearName(void **key,
                                            void **value,
                                            void *cl)
{
    if(!key)
        return;

    if(!*key)
        return;

    if(!value)
        return;

    if(!*value)
        return;

    (void) cl;

    ajStrDel((AjPStr *) key);

    ensQcdatabaseDel((EnsPQcdatabase *) value);

    return;
}




/* @funcstatic qcdatabaseadaptorCacheExit *************************************
**
** Clears the internal QC Database cache of an Ensembl QC Database Adaptor.
**
** @param [u] qcdba [EnsPQcdatabaseadaptor] Ensembl QC Database Adaptor
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
** @@
******************************************************************************/

static AjBool qcdatabaseadaptorCacheExit(EnsPQcdatabaseadaptor qcdba)
{
    if(!qcdba)
        return ajFalse;

    /* Clear and delete the identifier cache. */

    ajTableMapDel(qcdba->CacheByIdentifier,
                  qcdatabaseadaptorCacheClearIdentifier,
                  NULL);

    ajTableFree(&qcdba->CacheByIdentifier);

    /* Clear and delete the name cache. */

    ajTableMapDel(qcdba->CacheByName,
                  qcdatabaseadaptorCacheClearName,
                  NULL);

    ajTableFree(&qcdba->CacheByName);

    return ajTrue;
}




/* @section destructors *******************************************************
**
** Destruction destroys all internal data structures and frees the
** memory allocated for the Ensembl QC Database Adaptor.
**
** @fdata [EnsPQcdatabaseadaptor]
** @fnote None
**
** @nam3rule Del Destroy (free) a QC Database Adaptor object
**
** @argrule * Pqcdba [EnsPQcdatabaseadaptor*] QC Database Adaptor
**                                            object address
**
** @valrule * [void]
**
** @fcategory delete
******************************************************************************/




/* @func ensQcdatabaseadaptorDel **********************************************
**
** Default destructor for an Ensembl QC Database Adaptor.
**
** Ensembl Object Adaptors are singleton objects that are registered in the
** Ensembl Registry and weakly referenced by Ensembl Objects that have been
** instantiated by it. Therefore, Ensembl Object Adaptors should never be
** destroyed directly. Upon exit, the Ensembl Registry will call this function
** if required.
**
** @param [d] Pqcdba [EnsPQcdatabaseadaptor*] Ensembl QC Database Adaptor
**                                            address
**
** @return [void]
** @@
******************************************************************************/

void ensQcdatabaseadaptorDel(EnsPQcdatabaseadaptor *Pqcdba)
{
    EnsPQcdatabaseadaptor pthis = NULL;

    if(!Pqcdba)
        return;

    if(!*Pqcdba)
        return;

    if(ajDebugTest("ensQcdatabaseadaptorDel"))
        ajDebug("ensQcdatabaseadaptorDel\n"
                "  *Pqcdba %p\n",
                *Pqcdba);

    pthis = *Pqcdba;

    qcdatabaseadaptorCacheExit(pthis);

    ensBaseadaptorDel(&pthis->Adaptor);

    AJFREE(pthis);

    *Pqcdba = NULL;

    return;
}




/* @func ensQcdatabaseadaptorFetchByIdentifier ********************************
**
** Fetch an Ensembl QC Database by its SQL database-internal identifier.
**
** @param [u] qcdba [EnsPQcdatabaseadaptor] Ensembl QC Database Adaptor
** @param [r] identifier [ajuint] SQL database-internal identifier
** @param [wP] Pqcdb [EnsPQcdatabase*] Ensembl QC Database address
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
** @@
******************************************************************************/

AjBool ensQcdatabaseadaptorFetchByIdentifier(EnsPQcdatabaseadaptor qcdba,
                                             ajuint identifier,
                                             EnsPQcdatabase *Pqcdb)
{
    if(!qcdba)
        return ajFalse;

    if(!Pqcdb)
        return ajFalse;

    /*
    ** Initally, search the identifier cache.
    ** For any object returned by the AJAX Table the reference counter needs
    ** to be incremented manually.
    */

    *Pqcdb = (EnsPQcdatabase)
        ajTableFetch(qcdba->CacheByIdentifier, (const void *) &identifier);

    if(*Pqcdb)
    {
        ensQcdatabaseNewRef(*Pqcdb);

        return ajTrue;
    }

    /*
    ** TODO: Try to retrieve directly from the database?
    **
    ** *Pqcdb = (EnsPQcdatabase)
    ** ensBaseadaptorFetchByIdentifier(adaptor->Adaptor, identifier);
    **
    ** qcdatabaseadaptorCacheInsert(adaptor, Pqcdb);
    */

    return ajTrue;
}




/* @func ensQcdatabaseadaptorFetchByName **************************************
**
** Fetch an Ensembl QC Database by its name and release.
**
** @param [u] qcdba [EnsPQcdatabaseadaptor] Ensembl QC Database Adaptor
** @param [r] name [const AjPStr] Ensembl QC Database name
** @param [r] release [const AjPStr] Ensembl QC Database release
** @param [wP] Pqcdb [EnsPQcdatabase*] Ensembl QC Database address
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
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

    if(!qcdba)
        return ajFalse;

    if((!name) && (!ajStrGetLen(name)))
        return ajFalse;

    if((!release) && (!ajStrGetLen(release)))
        return ajFalse;

    if(!Pqcdb)
        return ajFalse;

    /*
    ** Initally, search the name cache.
    ** For any object returned by the AJAX Table the reference counter needs
    ** to be incremented manually.
    */

    key = ajFmtStr("%S:%S", name, release);

    *Pqcdb = (EnsPQcdatabase)
        ajTableFetch(qcdba->CacheByName, (const void *) key);

    ajStrDel(&key);

    if(*Pqcdb)
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

    ensBaseadaptorGenericFetch(qcdba->Adaptor,
                               constraint,
                               (EnsPAssemblymapper) NULL,
                               (EnsPSlice) NULL,
                               qcdbs);

    if(ajListGetLength(qcdbs) > 1)
        ajWarn("ensQcdatabaseadaptorFetchByName got more than "
               "one Ensembl QC Database for (UNIQUE) name '%S' "
               "and release '%S'.\n", name, release);

    ajListPop(qcdbs, (void **) Pqcdb);

    qcdatabaseadaptorCacheInsert(qcdba, Pqcdb);

    while(ajListPop(qcdbs, (void **) &qcdb))
    {
        qcdatabaseadaptorCacheInsert(qcdba, &qcdb);

        ensQcdatabaseDel(&qcdb);
    }

    ajListFree(&qcdbs);

    ajStrDel(&constraint);

    return ajTrue;
}




/* @func ensQcdatabaseadaptorFetchAllByClassType ******************************
**
** Fetch an Ensembl QC Database by its class and type.
**
** @param [u] qcdba [EnsPQcdatabaseadaptor] Ensembl QC Database Adaptor
** @param [r] class [EnsEQcdatabaseClass] Ensembl QC Database class
** @param [r] type [EnsEQcdatabaseType] Ensembl QC Database type
** @param [u] qcdbs [AjPList] AJAX List of Ensembl QC Databases
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
** @@
******************************************************************************/

AjBool ensQcdatabaseadaptorFetchAllByClassType(EnsPQcdatabaseadaptor qcdba,
                                               EnsEQcdatabaseClass class,
                                               EnsEQcdatabaseType type,
                                               AjPList qcdbs)
{
    void **keyarray = NULL;
    void **valarray = NULL;

    register ajuint i = 0;

    AjBool cmatch = AJFALSE;
    AjBool tmatch = AJFALSE;

    if(!qcdba)
        return ajFalse;

    if(!qcdbs)
        return ajFalse;

    ajTableToarrayKeysValues(qcdba->CacheByIdentifier, &keyarray, &valarray);

    for(i = 0; keyarray[i]; i++)
    {
        /* Match the class element if one has been provided. */

        if(class)
        {
            if(((EnsPQcdatabase) valarray[i])->Class == class)
                cmatch = ajTrue;
            else
                cmatch = ajFalse;
        }
        else
            cmatch = ajTrue;

        /* Match the type element if one has been provided. */

        if(type)
        {
            if(((EnsPQcdatabase) valarray[i])->Type == type)
                tmatch = ajTrue;
            else
                tmatch = ajFalse;
        }
        else
            tmatch = ajTrue;

        if(cmatch && tmatch)
            ajListPushAppend(qcdbs, (void *)
                             ensQcdatabaseNewRef(valarray[i]));
    }

    AJFREE(keyarray);
    AJFREE(valarray);

    return ajTrue;
}




/* @func ensQcdatabaseadaptorStore ********************************************
**
** Store an Ensembl QC Database.
**
** @param [u] qcdba [EnsPQcdatabaseadaptor] Ensembl QC Database Adaptor
** @param [u] qcdb [EnsPQcdatabase] Ensembl QC Database
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
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

    AjBool value = AJFALSE;

    AjPSqlstatement sqls = NULL;

    AjPStr statement = NULL;

    EnsPDatabaseadaptor dba = NULL;

    if(!qcdba)
        return ajFalse;

    if(!qcdb)
        return ajFalse;

    if(ensQcdatabaseGetAdaptor(qcdb) &&
       ensQcdatabaseGetIdentifier(qcdb))
        return ajFalse;

    dba = ensBaseadaptorGetDatabaseadaptor(qcdba->Adaptor);

    ensDatabaseadaptorEscapeC(dba, &txtname, qcdb->Name);
    ensDatabaseadaptorEscapeC(dba, &txtrelease, qcdb->Release);
    ensDatabaseadaptorEscapeC(dba, &txtdate, qcdb->Date);
    ensDatabaseadaptorEscapeC(dba, &txtformat, qcdb->Format);
    ensDatabaseadaptorEscapeC(dba, &txtspecies, qcdb->Species);
    ensDatabaseadaptorEscapeC(dba, &txthost, qcdb->Host);
    ensDatabaseadaptorEscapeC(dba, &txtdirectory, qcdb->Directory);
    ensDatabaseadaptorEscapeC(dba, &txtfile, qcdb->File);
    ensDatabaseadaptorEscapeC(dba, &txtexternalurl, qcdb->ExternalURL);

    statement = ajFmtStr("INSERT IGNORE INTO "
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
                         ensDatabaseadaptorGroupToChar(qcdb->Group),
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

    if(ajSqlstatementGetAffectedrows(sqls))
    {
        ensQcdatabaseSetIdentifier(qcdb, ajSqlstatementGetIdentifier(sqls));

        ensQcdatabaseSetAdaptor(qcdb, qcdba);

        value = ajTrue;
    }

    ensDatabaseadaptorSqlstatementDel(dba, &sqls);

    ajStrDel(&statement);

    return value;
}




/* @func ensQcdatabaseadaptorUpdate *******************************************
**
** Update an Ensembl QC Database.
**
** @param [u] qcdba [EnsPQcdatabaseadaptor] Ensembl QC Database Adaptor
** @param [r] qcdb [const EnsPQcdatabase] Ensembl QC Database
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
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

    AjBool value = AJFALSE;

    AjPSqlstatement sqls = NULL;

    AjPStr statement = NULL;

    EnsPDatabaseadaptor dba = NULL;

    if(!qcdba)
        return ajFalse;

    if(!qcdb)
        return ajFalse;

    if(!ensQcdatabaseGetIdentifier(qcdb))
        return ajFalse;

    dba = ensBaseadaptorGetDatabaseadaptor(qcdba->Adaptor);

    ensDatabaseadaptorEscapeC(dba, &txtname, qcdb->Name);
    ensDatabaseadaptorEscapeC(dba, &txtrelease, qcdb->Release);
    ensDatabaseadaptorEscapeC(dba, &txtdate, qcdb->Date);
    ensDatabaseadaptorEscapeC(dba, &txtformat, qcdb->Format);
    ensDatabaseadaptorEscapeC(dba, &txtspecies, qcdb->Species);
    ensDatabaseadaptorEscapeC(dba, &txthost, qcdb->Host);
    ensDatabaseadaptorEscapeC(dba, &txtdirectory, qcdb->Directory);
    ensDatabaseadaptorEscapeC(dba, &txtfile, qcdb->File);
    ensDatabaseadaptorEscapeC(dba, &txtexternalurl, qcdb->ExternalURL);

    statement = ajFmtStr("UPDATE IGNORE "
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
                         ensDatabaseadaptorGroupToChar(qcdb->Group),
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

    if(ajSqlstatementGetAffectedrows(sqls))
        value = ajTrue;

    ensDatabaseadaptorSqlstatementDel(dba, &sqls);

    ajStrDel(&statement);

    return value;
}




/* @func ensQcdatabaseadaptorDelete *******************************************
**
** Delete an Ensembl QC Database.
**
** @param [u] qcdba [EnsPQcdatabaseadaptor] Ensembl QC Database Adaptor
** @param [u] qcdb [EnsPQcdatabase] Ensembl QC Database
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
** @@
******************************************************************************/

AjBool ensQcdatabaseadaptorDelete(EnsPQcdatabaseadaptor qcdba,
                                  EnsPQcdatabase qcdb)
{
    AjBool value = AJFALSE;

    AjPSqlstatement sqls = NULL;

    AjPStr statement = NULL;

    EnsPDatabaseadaptor dba = NULL;

    if(!qcdba)
        return ajFalse;

    if(!qcdb)
        return ajFalse;

    if(!ensQcdatabaseGetIdentifier(qcdb))
        return ajFalse;

    dba = ensBaseadaptorGetDatabaseadaptor(qcdba->Adaptor);

    statement = ajFmtStr("DELETE FROM "
                         "sequence_db "
                         "WHERE "
                         "sequence_db.sequence_db_id = %u",
                         qcdb->Identifier);

    sqls = ensDatabaseadaptorSqlstatementNew(dba, statement);

    if(ajSqlstatementGetAffectedrows(sqls))
    {
        qcdb->Adaptor    = (EnsPQcdatabaseadaptor) NULL;
        qcdb->Identifier = 0;

        value = ajTrue;
    }

    ensDatabaseadaptorSqlstatementDel(dba, &sqls);

    ajStrDel(&statement);

    return value;
}
