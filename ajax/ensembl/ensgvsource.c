/* @source Ensembl Genetic Variation Source functions
**
** @author Copyright (C) 1999 Ensembl Developers
** @author Copyright (C) 2006 Michael K. Schuster
** @modified 2009 by Alan Bleasby for incorporation into EMBOSS core
** @modified $Date: 2011/07/06 21:56:10 $ by $Author: mks $
** @version $Revision: 1.7 $
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

#include "ensgvsource.h"
#include "ensmetainformation.h"
#include "enstable.h"




/* ==================================================================== */
/* ============================ constants ============================= */
/* ==================================================================== */




/* ==================================================================== */
/* ======================== global variables ========================== */
/* ==================================================================== */




/* ==================================================================== */
/* ========================== private data ============================ */
/* ==================================================================== */




/* ==================================================================== */
/* ======================== private constants ========================= */
/* ==================================================================== */

/* @conststatic gvsourceSomatic ***********************************************
**
** The Ensembl Genetic Variation Source somatic element is
** enumerated in both, the SQL table definition and the data structure.
** The following strings are used for conversion in database operations and
** correspond to EnsEGvsourceSomatic and the
** "source.somatic_status" field.
**
** #attr [static const char* const*] gvsourceSomatic
** ##
******************************************************************************/

static const char* const gvsourceSomatic[] =
{
    "",
    "germline",
    "somatic",
    "mixed",
    (const char*) NULL
};




/* @conststatic gvsourceType **************************************************
**
** The Ensembl Genetic Variation Source type element is
** enumerated in both, the SQL table definition and the data structure.
** The following strings are used for conversion in database operations and
** correspond to EnsEGvsourceType and the
** "source.type" field.
**
** #attr [static const char* const*] gvsourceType
** ##
******************************************************************************/

static const char* const gvsourceType[] =
{
    "",
    "chip",
    (const char*) NULL
};




/* @conststatic gvsourceadaptorTables *****************************************
**
** Array of Ensembl Genetic Variation Source Adaptor SQL table names
**
******************************************************************************/

static const char* const gvsourceadaptorTables[] =
{
    "source",
    (const char*) NULL
};




/* @conststatic gvsourceadaptorColumns ****************************************
**
** Array of Ensembl Genetic Variation Source Adaptor SQL column names
**
******************************************************************************/

static const char* const gvsourceadaptorColumns[] =
{
    "source.source_id",
    "source.name",
    "source.version",
    "source.description",
    "source.url",
    "source.type",
    "source.somatic_status",
    (const char*) NULL
};




/* ==================================================================== */
/* ======================== private variables ========================= */
/* ==================================================================== */




/* ==================================================================== */
/* ======================== private functions ========================= */
/* ==================================================================== */

static AjBool gvsourceadaptorFetchAllbyStatement(
    EnsPDatabaseadaptor dba,
    const AjPStr statement,
    EnsPAssemblymapper am,
    EnsPSlice slice,
    AjPList gvss);

static AjBool gvsourceadaptorCacheInsert(EnsPGvsourceadaptor gvsa,
                                         EnsPGvsource* Pgvs);

static AjBool gvsourceadaptorCacheInit(EnsPGvsourceadaptor gvsa);

static void gvsourceadaptorCacheClearIdentifier(void** key,
                                                void** value,
                                                void* cl);

static void gvsourceadaptorCacheClearName(void** key,
                                          void** value,
                                          void* cl);

static AjBool gvsourceadaptorCacheExit(EnsPGvsourceadaptor gvsa);

static void gvsourceadaptorFetchAll(const void* key,
                                    void** value,
                                    void* cl);




/* ==================================================================== */
/* ===================== All functions by section ===================== */
/* ==================================================================== */




/* @filesection ensgvsource ***************************************************
**
** @nam1rule ens Function belongs to the Ensembl library
**
******************************************************************************/




/* @datasection [EnsPGvsource] Ensembl Genetic Variation Source ***************
**
** @nam2rule Gvsource Functions for manipulating
** Ensembl Genetic Variation Source objects
**
** NOTE: The Ensembl Genetic Variation Source object has no counterpart in the
** Ensembl Genetic Variation Perl API. It has been split out of the
** Bio::EnsEMBL::Variation::Variation class, because a small number of entries
** in the 'variation_source' table are linked from a large number of entries in
** other tables. By caching all Ensembl Genetic Variation Source objects in an
** Ensembl Genetic Variation Source Adaptor, all other Ensembl Genetic
** Variation objects can reference the same data objects.
**
** @cc Bio::EnsEMBL::Variation::Variation
** @cc CVS Revision: 1.55
** @cc CVS Tag: branch-ensembl-62
**
******************************************************************************/




/* @section constructors ******************************************************
**
** All constructors return a new Ensembl Genetic Variation Source by pointer.
** It is the responsibility of the user to first destroy any previous
** Genetic Variation Source. The target pointer does not need to be
** initialised to NULL, but it is good programming practice to do so anyway.
**
** @fdata [EnsPGvsource]
**
** @nam3rule New Constructor
** @nam4rule Cpy Constructor with existing object
** @nam4rule Ini Constructor with initial values
** @nam4rule Ref Constructor by incrementing the reference counter
**
** @argrule Cpy gvs [const EnsPGvsource] Ensembl Genetic Variation Source
** @argrule Ini gvsa [EnsPGvsourceadaptor] Ensembl Genetic Variation
** @argrule Ini identifier [ajuint] SQL database-internal identifier
** @argrule Ini name [AjPStr] Name
** @argrule Ini version [AjPStr] Version
** @argrule Ini description [AjPStr] Description
** @argrule Ini url [AjPStr] Uniform Resource Locator
** @argrule Ini gvss [EnsEGvsourceSomatic]
** Ensembl Genetic Variation Source Somatic enumeration
** @argrule Ini gvst [EnsEGvsourceType]
** Ensembl Genetic Variation Source Type enumeration
** @argrule Ref gvs [EnsPGvsource] Ensembl Genetic Variation Source
**
** @valrule * [EnsPGvsource] Ensembl Genetic Variation Source
**
** @fcategory new
******************************************************************************/




/* @func ensGvsourceNewCpy ****************************************************
**
** Object-based constructor function, which returns an independent object.
**
** @param [r] gvs [const EnsPGvsource] Ensembl Genetic Variation Source
**
** @return [EnsPGvsource] Ensembl Genetic Variation Source or NULL
** @@
******************************************************************************/

EnsPGvsource ensGvsourceNewCpy(const EnsPGvsource gvs)
{
    EnsPGvsource pthis = NULL;

    if(!gvs)
        return NULL;

    AJNEW0(pthis);

    pthis->Use = 1;

    pthis->Identifier = gvs->Identifier;

    pthis->Adaptor = gvs->Adaptor;

    if(gvs->Name)
        pthis->Name = ajStrNewRef(gvs->Name);

    if(gvs->Version)
        pthis->Version = ajStrNewRef(gvs->Version);

    if(gvs->Description)
        pthis->Description = ajStrNewRef(gvs->Description);

    if(gvs->URL)
        pthis->URL = ajStrNewRef(gvs->URL);

    pthis->Somatic = gvs->Somatic;

    pthis->Type = gvs->Type;

    return pthis;
}




/* @func ensGvsourceNewIni ****************************************************
**
** Constructor for an Ensembl Genetic Variation Source with initial values.
**
** @cc Bio::EnsEMBL::Storable::new
** @param [u] gvsa [EnsPGvsourceadaptor] Ensembl Genetic Variation
**                                       Source Adaptor
** @param [r] identifier [ajuint] SQL database-internal identifier
** @cc Bio::EnsEMBL::Variation::*::new
** @param [u] name [AjPStr] Name
** @param [u] version [AjPStr] Version
** @param [u] description [AjPStr] Description
** @param [u] url [AjPStr] Uniform Resource Locator
** @param [u] gvss [EnsEGvsourceSomatic]
** Ensembl Genetic Variation Source Somatic enumeration
** @param [u] gvst [EnsEGvsourceType]
** Ensembl Genetic Variation Source Type enumeration
**
** @return [EnsPGvsource] Ensembl Genetic Variation Source
** @@
******************************************************************************/

EnsPGvsource ensGvsourceNewIni(EnsPGvsourceadaptor gvsa,
                               ajuint identifier,
                               AjPStr name,
                               AjPStr version,
                               AjPStr description,
                               AjPStr url,
                               EnsEGvsourceSomatic gvss,
                               EnsEGvsourceType gvst)
{
    EnsPGvsource gvs = NULL;

    AJNEW0(gvs);

    gvs->Use = 1;

    gvs->Identifier = identifier;

    gvs->Adaptor = gvsa;

    if(name)
        gvs->Name = ajStrNewRef(name);

    if(version)
        gvs->Version = ajStrNewRef(version);

    if(description)
        gvs->Description = ajStrNewRef(description);

    if(url)
        gvs->URL = ajStrNewRef(url);

    gvs->Somatic = gvss;

    gvs->Type = gvst;

    return gvs;
}




/* @func ensGvsourceNewRef ****************************************************
**
** Ensembl Object referencing function, which returns a pointer to the
** Ensembl Object passed in and increases its reference count.
**
** @param [u] gvs [EnsPGvsource] Ensembl Genetic Variation Source
**
** @return [EnsPGvsource] Ensembl Genetic Variation Source
** @@
******************************************************************************/

EnsPGvsource ensGvsourceNewRef(EnsPGvsource gvs)
{
    if(!gvs)
        return NULL;

    gvs->Use++;

    return gvs;
}




/* @section destructors *******************************************************
**
** Destruction destroys all internal data structures and frees the
** memory allocated for an Ensembl Genetic Variation Source object.
**
** @fdata [EnsPGvsource]
**
** @nam3rule Del Destroy (free) an Ensembl Genetic Variation Source object
**
** @argrule * Pgvs [EnsPGvsource*] Ensembl Genetic Variation Source
**                                 object address
**
** @valrule * [void]
**
** @fcategory delete
******************************************************************************/




/* @func ensGvsourceDel *******************************************************
**
** Default destructor for an Ensembl Genetic Variation Source.
**
** @param [d] Pgvs [EnsPGvsource*] Ensembl Genetic Variation Source
**                                 object address
**
** @return [void]
** @@
******************************************************************************/

void ensGvsourceDel(EnsPGvsource* Pgvs)
{
    EnsPGvsource pthis = NULL;

    if(!Pgvs)
        return;

    if(!*Pgvs)
        return;

    if(ajDebugTest("ensGvsourceDel"))
    {
        ajDebug("ensGvsourceDel\n"
                "  *Pgvs %p\n",
                *Pgvs);

        ensGvsourceTrace(*Pgvs, 1);
    }

    pthis = *Pgvs;

    pthis->Use--;

    if(pthis->Use)
    {
        *Pgvs = NULL;

        return;
    }

    ajStrDel(&pthis->Name);
    ajStrDel(&pthis->Version);
    ajStrDel(&pthis->Description);
    ajStrDel(&pthis->URL);

    AJFREE(pthis);

    *Pgvs = NULL;

    return;
}




/* @section element retrieval *************************************************
**
** Functions for returning elements of an
** Ensembl Genetic Variation Source object.
**
** @fdata [EnsPGvsource]
**
** @nam3rule Get Return Genetic Variation Source attribute(s)
** @nam4rule Adaptor Return the Ensembl Genetic Variation Source Adaptor
** @nam4rule Description Return the description
** @nam4rule Identifier Return the SQL database-internal identifier
** @nam4rule Name Return the name
** @nam4rule Somatic
** Return the Ensembl Genetic Variation Source Somatic enumeration
** @nam4rule Type
** Return the Ensembl Genetic Variation Source Type enumeration
** @nam4rule Version Return the version
** @nam4rule Url Return the Uniform Resource Locator
**
** @argrule * gvs [const EnsPGvsource] Genetic Variation Source
**
** @valrule Adaptor [EnsPGvsourceadaptor] Ensembl Genetic Variation
**                                        Source Adaptor or NULL
** @valrule Identifier [ajuint] SQL database-internal identifier or 0
** @valrule Description [AjPStr] Description or NULL
** @valrule Name [AjPStr] Name or NULL
** @valrule Somatic [EnsEGvsourceSomatic]
** Ensembl Genetic Variation Source Somatic enumeration
** @valrule Type [EnsEGvsourceType]
** Ensembl Genetic Variation Source Type enumeration
** @valrule Version [AjPStr] Version or NULL
** @valrule Url [AjPStr] Uniform Resource Locator or NULL
**
** @fcategory use
******************************************************************************/




/* @func ensGvsourceGetAdaptor ************************************************
**
** Get the Ensembl Genetic Variation Source Adaptor element of an
** Ensembl Genetic Variation Source.
**
** @param [r] gvs [const EnsPGvsource] Ensembl Genetic Variation Source
**
** @return [EnsPGvsourceadaptor] Ensembl Genetic Variation Source Adaptor
** or NULL
** @@
******************************************************************************/

EnsPGvsourceadaptor ensGvsourceGetAdaptor(const EnsPGvsource gvs)
{
    if(!gvs)
        return NULL;

    return gvs->Adaptor;
}




/* @func ensGvsourceGetDescription ********************************************
**
** Get the description element of an Ensembl Genetic Variation Source.
**
** @cc Bio::EnsEMBL::Variation::Variation::source_description
** @param [r] gvs [const EnsPGvsource] Ensembl Genetic Variation Source
**
** @return [AjPStr] Description or NULL
** @@
******************************************************************************/

AjPStr ensGvsourceGetDescription(const EnsPGvsource gvs)
{
    if(!gvs)
        return NULL;

    return gvs->Description;
}





/* @func ensGvsourceGetIdentifier *********************************************
**
** Get the SQL database-internal identifier element of an
** Ensembl Genetic Variation Source.
**
** @param [r] gvs [const EnsPGvsource] Ensembl Genetic Variation Source
**
** @return [ajuint] SQL database-internal identifier or 0
** @@
******************************************************************************/

ajuint ensGvsourceGetIdentifier(const EnsPGvsource gvs)
{
    if(!gvs)
        return 0;

    return gvs->Identifier;
}





/* @func ensGvsourceGetName ***************************************************
**
** Get the name element of an Ensembl Genetic Variation Source.
**
** @param [r] gvs [const EnsPGvsource] Ensembl Genetic Variation Source
**
** @return [AjPStr] Name or NULL
** @@
******************************************************************************/

AjPStr ensGvsourceGetName(const EnsPGvsource gvs)
{
    if(!gvs)
        return NULL;

    return gvs->Name;
}





/* @func ensGvsourceGetSomatic ************************************************
**
** Get the Ensembl Genetic Variation Source Somatic enumeration element of an
** Ensembl Genetic Variation Source.
**
** @cc Bio::EnsEMBL::Variation::Variation::is_somatic
** @param [r] gvs [const EnsPGvsource] Ensembl Genetic Variation Source
**
** @return [EnsEGvsourceSomatic]
** Ensembl Genetic Variation Source Somatic enumeration or
** ensEGvsourceSomaticNULL
** @@
******************************************************************************/

EnsEGvsourceSomatic ensGvsourceGetSomatic(const EnsPGvsource gvs)
{
    if(!gvs)
        return ensEGvsourceSomaticNULL;

    return gvs->Somatic;
}




/* @func ensGvsourceGetType ***************************************************
**
** Get the Ensembl Genetic Variation Source Type enumeration element of an
** Ensembl Genetic Variation Source.
**
** @cc Bio::EnsEMBL::Variation::Variation::source_type
** @param [r] gvs [const EnsPGvsource] Ensembl Genetic Variation Source
**
** @return [EnsEGvsourceType]
** Ensembl Genetic Variation Source Type enumeration or
** ensEGvsourceTypeNULL
** @@
******************************************************************************/

EnsEGvsourceType ensGvsourceGetType(const EnsPGvsource gvs)
{
    if(!gvs)
        return ensEGvsourceTypeNULL;

    return gvs->Type;
}




/* @func ensGvsourceGetUrl ****************************************************
**
** Get the Uniform Resource Locator element of an
** Ensembl Genetic Variation Source.
**
** @cc Bio::EnsEMBL::Variation::Variation::source_url
** @param [r] gvs [const EnsPGvsource] Ensembl Genetic Variation Source
**
** @return [AjPStr] Uniform Resource Locator or NULL
** @@
******************************************************************************/

AjPStr ensGvsourceGetUrl(const EnsPGvsource gvs)
{
    if(!gvs)
        return NULL;

    return gvs->URL;
}





/* @func ensGvsourceGetVersion ************************************************
**
** Get the version element of an Ensembl Genetic Variation Source.
**
** @param [r] gvs [const EnsPGvsource] Ensembl Genetic Variation Source
**
** @return [AjPStr] Version or NULL
** @@
******************************************************************************/

AjPStr ensGvsourceGetVersion(const EnsPGvsource gvs)
{
    if(!gvs)
        return NULL;

    return gvs->Version;
}





/* @section element assignment ************************************************
**
** Functions for assigning elements of an
** Ensembl Genetic Variation Source object.
**
** @fdata [EnsPGvsource]
**
** @nam3rule Set Set one element of a Genetic Variation Source
** @nam4rule Adaptor Set the Ensembl Genetic Variation Source Adaptor
** @nam4rule Description Set the description
** @nam4rule Identifier Set the SQL database-internal identifier
** @nam4rule Name Set the name
** @nam4rule Somatic
** Set the Ensembl Genetic Variation Source Somatic enumeration
** @nam4rule Type
** Set the Ensembl Genetic Variation Source Type enumeration
** @nam4rule Url Set the Uniform Resource Locator
** @nam4rule Version Set the version
**
** @argrule * gvs [EnsPGvsource] Ensembl Genetic Variation Source object
** @argrule Adaptor gvsa [EnsPGvsourceadaptor] Ensembl Genetic Variation
** @argrule Description description [AjPStr] Description
** @argrule Identifier identifier [ajuint] SQL database-internal identifier
** @argrule Name name [AjPStr] Name
** @argrule Somatic gvss [EnsEGvsourceSomatic]
** Ensembl Genetic Variation Source Somatic enumeration
** @argrule Type gvst [EnsEGvsourceType]
** Ensembl Genetic Variation Source Type enumeration
** @argrule Url url [AjPStr] Uniform Resopurce Locator
** @argrule Version version [AjPStr] Version
**
** @valrule * [AjBool] ajTrue upon success, ajFalse otherwise
**
** @fcategory modify
******************************************************************************/




/* @func ensGvsourceSetAdaptor ************************************************
**
** Set the Ensembl Genetic Variation Source Adaptor element of an
** Ensembl Genetic Variation Source.
**
** @param [u] gvs [EnsPGvsource] Ensembl Genetic Variation Source
** @param [u] gvsa [EnsPGvsourceadaptor] Ensembl Genetic Variation
**                                       Source Adaptor
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
** @@
******************************************************************************/

AjBool ensGvsourceSetAdaptor(EnsPGvsource gvs, EnsPGvsourceadaptor gvsa)
{
    if(!gvs)
        return ajFalse;

    gvs->Adaptor = gvsa;

    return ajTrue;
}




/* @func ensGvsourceSetDescription ********************************************
**
** Set the description element of an Ensembl Genetic Variation Source.
**
** @param [u] gvs [EnsPGvsource] Ensembl Genetic Variation Source
** @param [u] description [AjPStr] Description
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
** @@
******************************************************************************/

AjBool ensGvsourceSetDescription(EnsPGvsource gvs, AjPStr description)
{
    if(!gvs)
        return ajFalse;

    ajStrDel(&gvs->Description);

    if(description)
        gvs->Description = ajStrNewRef(description);

    return ajTrue;
}




/* @func ensGvsourceSetIdentifier *********************************************
**
** Set the SQL database-internal identifier element of an
** Ensembl Genetic Variation Source.
**
** @param [u] gvs [EnsPGvsource] Ensembl Genetic Variation Source
** @param [r] identifier [ajuint] SQL database-internal identifier
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
** @@
******************************************************************************/

AjBool ensGvsourceSetIdentifier(EnsPGvsource gvs, ajuint identifier)
{
    if(!gvs)
        return ajFalse;

    gvs->Identifier = identifier;

    return ajTrue;
}




/* @func ensGvsourceSetName ***************************************************
**
** Set the name element of an Ensembl Genetic Variation Source.
**
** @param [u] gvs [EnsPGvsource] Ensembl Genetic Variation Source
** @param [u] name [AjPStr] Name
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
** @@
******************************************************************************/

AjBool ensGvsourceSetName(EnsPGvsource gvs, AjPStr name)
{
    if(!gvs)
        return ajFalse;

    ajStrDel(&gvs->Name);

    if(name)
        gvs->Name = ajStrNewRef(name);

    return ajTrue;
}




/* @func ensGvsourceSetSomatic ************************************************
**
** Set the Ensembl Genetic Variation Source Somatic enumeration element of an
** Ensembl Genetic Variation Source.
**
** @cc Bio::EnsEMBL::Variation::Variation::is_somatic
** @param [u] gvs [EnsPGvsource] Ensembl Genetic Variation Source
** @param [u] gvss [EnsEGvsourceSomatic]
** Ensembl Genetic Variation Source Somatic enumeration
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
** @@
******************************************************************************/

AjBool ensGvsourceSetSomatic(EnsPGvsource gvs, EnsEGvsourceSomatic gvss)
{
    if(!gvs)
        return ajFalse;

    gvs->Somatic = gvss;

    return ajTrue;
}




/* @func ensGvsourceSetType ***************************************************
**
** Set the Ensembl Genetic Variation Source Type enumeration element of an
** Ensembl Genetic Variation Source.
**
** @cc Bio::EnsEMBL::Variation::Variation::source_type
** @param [u] gvs [EnsPGvsource] Ensembl Genetic Variation Source
** @param [u] gvst [EnsEGvsourceType]
** Ensembl Genetic Variation Source Type enumeration
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
** @@
******************************************************************************/

AjBool ensGvsourceSetType(EnsPGvsource gvs, EnsEGvsourceType gvst)
{
    if(!gvs)
        return ajFalse;

    gvs->Type = gvst;

    return ajTrue;
}




/* @func ensGvsourceSetUrl ****************************************************
**
** Set the Uniform Resopurce Locator element of an
** Ensembl Genetic Variation Source.
**
** @param [u] gvs [EnsPGvsource] Ensembl Genetic Variation Source
** @param [u] url [AjPStr] Uniform Resopurce Locator
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
** @@
******************************************************************************/

AjBool ensGvsourceSetUrl(EnsPGvsource gvs, AjPStr url)
{
    if(!gvs)
        return ajFalse;

    ajStrDel(&gvs->URL);

    if(url)
        gvs->URL = ajStrNewRef(url);

    return ajTrue;
}




/* @func ensGvsourceSetVersion ************************************************
**
** Set the version element of an Ensembl Genetic Variation Source.
**
** @param [u] gvs [EnsPGvsource] Ensembl Genetic Variation Source
** @param [u] version [AjPStr] Version
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
** @@
******************************************************************************/

AjBool ensGvsourceSetVersion(EnsPGvsource gvs, AjPStr version)
{
    if(!gvs)
        return ajFalse;

    ajStrDel(&gvs->Version);

    if(version)
        gvs->Version = ajStrNewRef(version);

    return ajTrue;
}




/* @section debugging *********************************************************
**
** Functions for reporting of an Ensembl Genetic Variation Source object.
**
** @fdata [EnsPGvsource]
**
** @nam3rule Trace Report Ensembl Genetic Variation Source elements to
**                 debug file
**
** @argrule Trace gvs [const EnsPGvsource] Ensembl Genetic Variation Source
** @argrule Trace level [ajuint] Indentation level
**
** @valrule * [AjBool] ajTrue upon success, ajFalse otherwise
**
** @fcategory misc
******************************************************************************/




/* @func ensGvsourceTrace *****************************************************
**
** Trace an Ensembl Genetic Variation Source.
**
** @param [r] gvs [const EnsPGvsource] Ensembl Genetic Variation Source
** @param [r] level [ajuint] Indentation level
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
** @@
******************************************************************************/

AjBool ensGvsourceTrace(const EnsPGvsource gvs, ajuint level)
{
    AjPStr indent = NULL;

    if(!gvs)
        return ajFalse;

    indent = ajStrNew();

    ajStrAppendCountK(&indent, ' ', level * 2);

    ajDebug("%SensGvsourceTrace %p\n"
            "%S  Use %u\n"
            "%S  Identifier %u\n"
            "%S  Adaptor %p\n"
            "%S  Name '%S'\n"
            "%S  Version '%S'\n"
            "%S  Description '%S'\n"
            "%S  URL '%S'\n"
            "%S  Somatic '%s'\n"
            "%S  Type '%s'\n",
            indent, gvs,
            indent, gvs->Use,
            indent, gvs->Identifier,
            indent, gvs->Adaptor,
            indent, gvs->Name,
            indent, gvs->Version,
            indent, gvs->Description,
            indent, gvs->URL,
            indent, ensGvsourceSomaticToChar(gvs->Somatic),
            indent, ensGvsourceTypeToChar(gvs->Type));

    ajStrDel(&indent);

    return ajTrue;
}




/* @section calculate *********************************************************
**
** Functions for calculating values of an
** Ensembl Genetic Variation Source object.
**
** @fdata [EnsPGvsource]
**
** @nam3rule Calculate Calculate Ensembl Genetic Variation Source values
** @nam4rule Memsize Calculate the memory size in bytes
**
** @argrule * gvs [const EnsPGvsource] Ensembl Genetic Variation Source
**
** @valrule Memsize [size_t] Memory size in bytes or 0
**
** @fcategory misc
******************************************************************************/




/* @func ensGvsourceCalculateMemsize ******************************************
**
** Calculate the memory size in bytes of an Ensembl Genetic Variation Source.
**
** @param [r] gvs [const EnsPGvsource] Ensembl Genetic Variation Source
**
** @return [size_t] Memory size in bytes or 0
** @@
******************************************************************************/

size_t ensGvsourceCalculateMemsize(const EnsPGvsource gvs)
{
    size_t size = 0;

    if(!gvs)
        return 0;

    size += sizeof (EnsOGvsource);

    if(gvs->Name)
    {
        size += sizeof (AjOStr);

        size += ajStrGetRes(gvs->Name);
    }

    if(gvs->Version)
    {
        size += sizeof (AjOStr);

        size += ajStrGetRes(gvs->Version);
    }

    if(gvs->Description)
    {
        size += sizeof (AjOStr);

        size += ajStrGetRes(gvs->Description);
    }

    if(gvs->URL)
    {
        size += sizeof (AjOStr);

        size += ajStrGetRes(gvs->URL);
    }

    return size;
}




/* @section comparison ********************************************************
**
** Functions for comparing Ensembl Genetic Variation Source objects
**
** @fdata [EnsPGvsource]
**
** @nam3rule  Match Compare two Ensembl Genetic Variation Source objects
**
** @argrule * gvs1 [const EnsPGvsource] Genetic Variation Source
** @argrule * gvs2 [const EnsPGvsource] Genetic Variation Source
**
** @valrule * [AjBool] ajTrue if the Ensembl Genetic Variation Source objects
** are equal
**
** @fcategory use
******************************************************************************/




/* @func ensGvsourceMatch *****************************************************
**
** Test for matching two Ensembl Genetic Variation Source objects.
**
** @param [r] gvs1 [const EnsPGvsource] First Ensembl Genetic Variation Source
** @param [r] gvs2 [const EnsPGvsource] Second Ensembl Genetic Variation Source
**
** @return [AjBool] ajTrue if the Ensembl Genetic Variation Source objects
** are equal
** @@
** The comparison is based on an initial pointer equality test and if that
** fails, a case-insensitive string comparison of the name and version elements
** is performed.
******************************************************************************/

AjBool ensGvsourceMatch(const EnsPGvsource gvs1, const EnsPGvsource gvs2)
{
    if(ajDebugTest("ensGvsourceMatch"))
    {
        ajDebug("ensGvsourceMatch\n"
                "  gvs1 %p\n"
                "  gvs2 %p\n",
                gvs1,
                gvs2);

        ensGvsourceTrace(gvs1, 1);
        ensGvsourceTrace(gvs2, 1);
    }

    if(!gvs1)
        return ajFalse;

    if(!gvs2)
        return ajFalse;

    if(gvs1 == gvs2)
        return ajTrue;

    if(gvs1->Identifier != gvs2->Identifier)
        return ajFalse;

    if(gvs1->Adaptor != gvs2->Adaptor)
        return ajFalse;

    if(!ajStrMatchCaseS(gvs1->Name, gvs2->Name))
        return ajFalse;

    if(!ajStrMatchCaseS(gvs1->Version, gvs2->Version))
        return ajFalse;

    if(!ajStrMatchCaseS(gvs1->Description, gvs2->Description))
        return ajFalse;

    if(!ajStrMatchCaseS(gvs1->URL, gvs2->URL))
        return ajFalse;

    if(gvs1->Somatic != gvs2->Somatic)
        return ajFalse;

    if(gvs1->Type != gvs2->Type)
        return ajFalse;

    return ajTrue;
}




/* @datasection [EnsEGvsourceSomatic] Ensembl Genetic Variation
** Source Somatic enumeration
**
** @nam2rule Gvsource Functions for manipulating
** Ensembl Genetic Variation Source objects
** @nam3rule GvsourceSomatic Functions for manipulating
** Ensembl Genetic Variation Source Somatic enumerations
**
******************************************************************************/




/* @section Misc **************************************************************
**
** Functions for returning an Ensembl Genetic Variation Source Somatic
** enumeration.
**
** @fdata [EnsEGvsourceSomatic]
**
** @nam4rule From Ensembl Genetic Variation Source Somatic query
** @nam5rule Str  AJAX String object query
**
** @argrule  Str  somatic  [const AjPStr] Somatic string
**
** @valrule * [EnsEGvsourceSomatic] Ensembl Genetic Variation
** Source Somatic enumeration or ensEGvsourceSomaticNULL
**
** @fcategory misc
******************************************************************************/




/* @func ensGvsourceSomaticFromStr ********************************************
**
** Convert an AJAX String into an Ensembl Genetic Variation Source
** Somatic enumeration.
**
** @param [r] somatic [const AjPStr] Somatic string
**
** @return [EnsEGvsourceSomatic] Ensembl Genetic Variation
** Source Somatic enumeration or ensEGvsourceSomaticNULL
** @@
******************************************************************************/

EnsEGvsourceSomatic ensGvsourceSomaticFromStr(
    const AjPStr somatic)
{
    register EnsEGvsourceSomatic i =
        ensEGvsourceSomaticNULL;

    EnsEGvsourceSomatic gvss =
        ensEGvsourceSomaticNULL;

    for(i = ensEGvsourceSomaticNULL;
        gvsourceSomatic[i];
        i++)
        if(ajStrMatchC(somatic, gvsourceSomatic[i]))
            gvss = i;

    if(!gvss)
        ajDebug("ensGvsourceSomaticFromStr encountered "
                "unexpected string '%S'.\n", somatic);

    return gvss;
}




/* @section Cast **************************************************************
**
** Functions for returning attributes of an
** Ensembl Genetic Variation Source Somatic enumeration.
**
** @fdata [EnsEGvsourceSomatic]
**
** @nam4rule To   Return Ensembl Genetic Variation Source Somatic
**                       enumeration
** @nam5rule Char Return C character string value
**
** @argrule To gvss [EnsEGvsourceSomatic] Ensembl Genetic Variation
** Source Somatic enumeration
**
** @valrule Char [const char*] Ensembl Genetic Variation Source Somatic
** C-type (char*) string
**
** @fcategory cast
******************************************************************************/




/* @func ensGvsourceSomaticToChar *********************************************
**
** Convert an Ensembl Genetic Variation Source Somatic enumeration
** into a C-type (char*) string.
**
** @param [u] gvss [EnsEGvsourceSomatic] Ensembl Genetic Variation
** Source Somatic enumeration
**
** @return [const char*] Ensembl Genetic Variation Source Somatic
**                       C-type (char*) string
** @@
******************************************************************************/

const char* ensGvsourceSomaticToChar(
    EnsEGvsourceSomatic gvss)
{
    register EnsEGvsourceSomatic i =
        ensEGvsourceSomaticNULL;

    for(i = ensEGvsourceSomaticNULL;
        gvsourceSomatic[i] && (i < gvss);
        i++);

    if(!gvsourceSomatic[i])
        ajDebug("ensGvsourceSomaticToChar encountered an "
                "out of boundary error on Ensembl "
                "Genetic Variation Source Somatic enumeration %d.\n",
                gvss);

    return gvsourceSomatic[i];
}




/* @datasection [EnsEGvsourceType] Ensembl Genetic Variation Source Type
** enumeration
**
** @nam2rule Gvsource Functions for manipulating
** Ensembl Genetic Variation Source objects
** @nam3rule GvsourceType Functions for manipulating
** Ensembl Genetic Variation Source Type enumerations
**
******************************************************************************/




/* @section Misc **************************************************************
**
** Functions for returning an Ensembl Genetic Variation Source Type
** enumeration.
**
** @fdata [EnsEGvsourceType]
**
** @nam4rule From Ensembl Genetic Variation Source Type query
** @nam5rule Str  AJAX String object query
**
** @argrule  Str  type  [const AjPStr] Type string
**
** @valrule * [EnsEGvsourceType] Ensembl Genetic Variation
** Source Type enumeration or ensEGvsourceTypeNULL
**
** @fcategory misc
******************************************************************************/




/* @func ensGvsourceTypeFromStr ***********************************************
**
** Convert an AJAX String into an Ensembl Genetic Variation Source
** Type enumeration.
**
** @param [r] type [const AjPStr] Type string
**
** @return [EnsEGvsourceType] Ensembl Genetic Variation
** Source Type enumeration or ensEGvsourceTypeNULL
** @@
******************************************************************************/

EnsEGvsourceType ensGvsourceTypeFromStr(
    const AjPStr type)
{
    register EnsEGvsourceType i =
        ensEGvsourceTypeNULL;

    EnsEGvsourceType gvst =
        ensEGvsourceTypeNULL;

    for(i = ensEGvsourceTypeNULL;
        gvsourceType[i];
        i++)
        if(ajStrMatchC(type, gvsourceType[i]))
            gvst = i;

    if(!gvst)
        ajDebug("ensGvsourceTypeFromStr encountered "
                "unexpected string '%S'.\n", type);

    return gvst;
}




/* @section Cast **************************************************************
**
** Functions for returning attributes of an
** Ensembl Genetic Variation Source Type enumeration.
**
** @fdata [EnsEGvsourceType]
**
** @nam4rule To   Return Ensembl Genetic Variation Source Type
**                       enumeration
** @nam5rule Char Return C character string value
**
** @argrule To gvst [EnsEGvsourceType] Ensembl Genetic Variation
** Source Type enumeration
**
** @valrule Char [const char*] Ensembl Genetic Variation Source Type
** C-type (char*) string
**
** @fcategory cast
******************************************************************************/




/* @func ensGvsourceTypeToChar ************************************************
**
** Convert an Ensembl Genetic Variation Source Type enumeration
** into a C-type (char*) string.
**
** @param [u] gvst [EnsEGvsourceType] Ensembl Genetic Variation
** Source Type enumeration
**
** @return [const char*] Ensembl Genetic Variation Source Type
**                       C-type (char*) string
** @@
******************************************************************************/

const char* ensGvsourceTypeToChar(
    EnsEGvsourceType gvst)
{
    register EnsEGvsourceType i =
        ensEGvsourceTypeNULL;

    for(i = ensEGvsourceTypeNULL;
        gvsourceType[i] && (i < gvst);
        i++);

    if(!gvsourceType[i])
        ajDebug("ensGvsourceTypeToChar encountered an "
                "out of boundary error on Ensembl "
                "Genetic Variation Source Type enumeration %d.\n",
                gvst);

    return gvsourceType[i];
}




/* @datasection [EnsPGvsourceadaptor] Ensembl Genetic Variation Source Adaptor
**
** @nam2rule Gvsourceadaptor Functions for manipulating
** Ensembl Genetic Variation Source Adaptor objects
**
** NOTE: The Ensembl Genetic Variation Source Adaptor has no counterpart in the
** Ensembl Genetic Variation Perl API. It has been split out of the
** Bio::EnsEMBL::Variation::DBSQL::VariationAdaptor class, because a  small
** number of entries in the 'variation_source' table are linked to a large
** number of entries in other tables. By caching all Ensembl Genetic Variation
** Source objects in an Ensembl Genetic Variation Source Adaptor, all other
** Ensembl Genetic Variation objects can reference the same data objects.
**
** @cc Bio::EnsEMBL::Variation::DBSQL::VariationAdaptor
** @cc CVS Revision: 1.78
** @cc CVS Tag: branch-ensembl-62
**
******************************************************************************/




/* @funcstatic gvsourceadaptorFetchAllbyStatement *****************************
**
** Run a SQL statement against an Ensembl Database Adaptor and consolidate the
** results into an AJAX List of Ensembl Genetic Variation Source objects.
**
** @cc Bio::EnsEMBL::Variation::DBSQL::VariationAdaptor::_objFromHashref
** @param [u] dba [EnsPDatabaseadaptor] Ensembl Database Adaptor
** @param [r] statement [const AjPStr] SQL statement
** @param [uN] am [EnsPAssemblymapper] Ensembl Assembly Mapper
** @param [uN] slice [EnsPSlice] Ensembl Slice
** @param [u] gvss [AjPList] AJAX List of Ensembl Genetic Variation Source
** objects
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
** @@
******************************************************************************/

static AjBool gvsourceadaptorFetchAllbyStatement(
    EnsPDatabaseadaptor dba,
    const AjPStr statement,
    EnsPAssemblymapper am,
    EnsPSlice slice,
    AjPList gvss)
{
    ajuint identifier = 0;

    AjPSqlstatement sqls = NULL;
    AjISqlrow sqli       = NULL;
    AjPSqlrow sqlr       = NULL;

    AjPStr name        = NULL;
    AjPStr version     = NULL;
    AjPStr description = NULL;
    AjPStr url         = NULL;
    AjPStr gvssomatic  = NULL;
    AjPStr gvstype     = NULL;

    EnsEGvsourceSomatic gvsse = ensEGvsourceSomaticNULL;
    EnsEGvsourceType    gvste = ensEGvsourceTypeNULL;

    EnsPGvsource        gvs  = NULL;
    EnsPGvsourceadaptor gvsa = NULL;

    if(ajDebugTest("gvsourceadaptorFetchAllbyStatement"))
        ajDebug("gvsourceadaptorFetchAllbyStatement\n"
                "  dba %p\n"
                "  statement %p\n"
                "  am %p\n"
                "  slice %p\n"
                "  gvss %p\n",
                dba,
                statement,
                am,
                slice,
                gvss);

    if(!dba)
        return ajFalse;

    if(!statement)
        return ajFalse;

    if(!gvss)
        return ajFalse;

    gvsa = ensRegistryGetGvsourceadaptor(dba);

    sqls = ensDatabaseadaptorSqlstatementNew(dba, statement);

    sqli = ajSqlrowiterNew(sqls);

    while(!ajSqlrowiterDone(sqli))
    {
        identifier  = 0;
        name        = ajStrNew();
        version     = ajStrNew();
        description = ajStrNew();
        url         = ajStrNew();
        gvstype     = ajStrNew();
        gvssomatic  = ajStrNew();

        sqlr = ajSqlrowiterGet(sqli);

        ajSqlcolumnToUint(sqlr, &identifier);
        ajSqlcolumnToStr(sqlr, &name);
        ajSqlcolumnToStr(sqlr, &version);
        ajSqlcolumnToStr(sqlr, &description);
        ajSqlcolumnToStr(sqlr, &url);
        ajSqlcolumnToStr(sqlr, &gvstype);
        ajSqlcolumnToStr(sqlr, &gvssomatic);

        gvsse = ensGvsourceSomaticFromStr(gvssomatic);

        if(!gvsse)
            ajDebug("gvsourceadaptorFetchAllbyStatement could not "
                    "get Ensembl Genetic Variation Source Somatic "
                    "for string '%S'.\n", gvssomatic);

        gvste = ensGvsourceTypeFromStr(gvstype);

        if(!gvste)
            ajDebug("gvsourceadaptorFetchAllbyStatement could not "
                    "get Ensembl Genetic Variation Source Type "
                    "for string '%S'.\n", gvstype);

        gvs = ensGvsourceNewIni(gvsa,
                                identifier,
                                name,
                                version,
                                description,
                                url,
                                gvsse,
                                gvste);

        ajListPushAppend(gvss, (void*) gvs);

        ajStrDel(&name);
        ajStrDel(&version);
        ajStrDel(&description);
        ajStrDel(&url);
        ajStrDel(&gvstype);
        ajStrDel(&gvssomatic);
    }

    ajSqlrowiterDel(&sqli);

    ensDatabaseadaptorSqlstatementDel(dba, &sqls);

    return ajTrue;
}




/* @funcstatic gvsourceadaptorCacheInsert *************************************
**
** Insert an Ensembl Genetic Variation Source into the Ensembl Genetic
** Variation Source Adaptor-internal cache. If a Source with the same name
** element is already present in the adaptor cache, the Source is deleted and
** a pointer to the cached Source is returned.
**
** @param [u] gvsa [EnsPGvsourceadaptor] Ensembl Genetic Variation
**                                       Source Adaptor
** @param [u] Pgvs [EnsPGvsource*] Ensembl Genetic Variation Source address
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
** @@
******************************************************************************/

static AjBool gvsourceadaptorCacheInsert(EnsPGvsourceadaptor gvsa,
                                         EnsPGvsource* Pgvs)
{
    ajuint* Pidentifier = NULL;

    EnsPGvsource gvs1 = NULL;
    EnsPGvsource gvs2 = NULL;

    if(!gvsa)
        return ajFalse;

    if(!gvsa->CacheByIdentifier)
        return ajFalse;

    if(!gvsa->CacheByName)
        return ajFalse;

    if(!Pgvs)
        return ajFalse;

    if(!*Pgvs)
        return ajFalse;

    /* Search the identifier cache. */

    gvs1 = (EnsPGvsource) ajTableFetchmodV(
        gvsa->CacheByIdentifier,
        (const void*) &((*Pgvs)->Identifier));

    /* Search the name cache. */

    gvs2 = (EnsPGvsource) ajTableFetchmodV(
        gvsa->CacheByName,
        (const void*) (*Pgvs)->Name);

    if((!gvs1) && (!gvs2))
    {
        /* Insert into the identifier cache. */

        AJNEW0(Pidentifier);

        *Pidentifier = (*Pgvs)->Identifier;

        ajTablePut(gvsa->CacheByIdentifier,
                   (void*) Pidentifier,
                   (void*) ensGvsourceNewRef(*Pgvs));

        /* Insert into the name cache. */

        ajTablePut(gvsa->CacheByName,
                   (void*) ajStrNewS((*Pgvs)->Name),
                   (void*) ensGvsourceNewRef(*Pgvs));
    }

    if(gvs1 && gvs2 && (gvs1 == gvs2))
    {
        ajDebug("gvsourceadaptorCacheInsert replaced Ensembl Genetic "
                "Variation Source %p with one already cached %p.\n",
                *Pgvs, gvs1);

        ensGvsourceDel(Pgvs);

        ensGvsourceNewRef(gvs1);

        Pgvs = &gvs1;
    }

    if(gvs1 && gvs2 && (gvs1 != gvs2))
        ajDebug("gvsourceadaptorCacheInsert detected an Ensembl Genetic "
                "Variation Source in the identifier and name cache with "
                "identical names ('%S' and '%S') but different addresses "
                "(%p and %p).\n",
                gvs1->Name, gvs2->Name, gvs1, gvs2);

    if(gvs1 && (!gvs2))
        ajDebug("gvsourceadaptorCacheInsert detected an Ensembl Analysis "
                "in the identifier, but not in the name cache.\n");

    if((!gvs1) && gvs2)
        ajDebug("gvsourceadaptorCacheInsert detected an Ensembl Analysis "
                "in the name, but not in the identifier cache.\n");

    return ajTrue;
}




#if AJFALSE
/* @funcstatic gvsourceadaptorCacheRemove *************************************
**
** Remove an Ensembl Genetic Variation Source from the Ensembl Genetic
** Variation Source Adaptor-internal cache.
**
** @param [u] gvsa [EnsPGvsourceadaptor] Ensembl Genetic Variation
**                                       Source Adaptor
** @param [u] gvs [EnsPGvsource] Ensembl Genetic Variation Source
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
** @@
******************************************************************************/

static AjBool gvsourceadaptorCacheRemove(EnsPGvsourceadaptor gvsa,
                                         EnsPGvsource gvs)
{
    ajuint* Pidentifier = NULL;

    AjPStr key = NULL;

    EnsPGvsource gvs1 = NULL;
    EnsPGvsource gvs2 = NULL;

    if(!gvsa)
        return ajFalse;

    if(!gvs)
        return ajFalse;

    /* Remove the table nodes. */

    gvs1 = (EnsPGvsource)
        ajTableRemoveKey(gvsa->CacheByIdentifier,
                         (const void*) &gvs->Identifier,
                         (void**) &Pidentifier);

    gvs2 = (EnsPGvsource)
        ajTableRemoveKey(gvsa->CacheByName,
                         (const void*) gvs->Name,
                         (void**) &key);

    if(gvs1 && (!gvs2))
        ajWarn("gvsourceadaptorCacheRemove could remove Ensembl Genetic "
               "Variation Source with identifier %u and name '%S' only from "
               "the identifier cache.\n",
               gvs->Identifier,
               gvs->Name);

    if((!gvs1) && gvs2)
        ajWarn("gvsourceadaptorCacheRemove could remove Ensembl Genetic "
               "Variation Source with identifier %u and name '%S' only from "
               "the name cache.\n",
               gvs->Identifier,
               gvs->Name);

    AJFREE(Pidentifier);

    ajStrDel(&key);

    ensGvsourceDel(&gvs1);
    ensGvsourceDel(&gvs2);

    return ajTrue;
}

#endif




/* @funcstatic gvsourceadaptorCacheInit ***************************************
**
** Initialise the internal Ensembl Genetic Variation Source cache of an
** Ensembl Genetic Variation Source Adaptor.
**
** @param [u] gvsa [EnsPGvsourceadaptor] Ensembl Genetic Variation
**                                       Source Adaptor
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
** @@
******************************************************************************/

static AjBool gvsourceadaptorCacheInit(EnsPGvsourceadaptor gvsa)
{
    AjPList gvss = NULL;
    AjPList mis  = NULL;

    AjPStr key = NULL;

    EnsPDatabaseadaptor dba = NULL;

    EnsPGvsource gvs = NULL;

    EnsPMetainformation        mi  = NULL;
    EnsPMetainformationadaptor mia = NULL;

    if(ajDebugTest("gvsourceadaptorCacheInit"))
        ajDebug("gvsourceadaptorCacheInit\n"
                "  gvsa %p\n",
                gvsa);

    if(!gvsa)
        return ajFalse;

    if(gvsa->CacheByIdentifier)
        return ajFalse;
    else
        gvsa->CacheByIdentifier = ensTableuintNewLen(0);

    if(gvsa->CacheByName)
        return ajFalse;
    else
        gvsa->CacheByName = ensTablestrNewLen(0);

    /* Get the default Ensembl Genetic Variation Source. */

    dba = ensBaseadaptorGetDatabaseadaptor(gvsa->Adaptor);

    mia = ensRegistryGetMetainformationadaptor(dba);

    key = ajStrNewC("source.default_source");

    mis = ajListNew();

    ensMetainformationadaptorFetchAllbyKey(mia, key, mis);

    ajStrDel(&key);

    ajListPeekFirst(mis, (void**) &mi);

    /* Fetch all Ensembl Genetic Variation Source objects. */

    gvss = ajListNew();

    ensBaseadaptorFetchAllbyConstraint(gvsa->Adaptor,
                                       (const AjPStr) NULL,
                                       (EnsPAssemblymapper) NULL,
                                       (EnsPSlice) NULL,
                                       gvss);

    while(ajListPop(gvss, (void**) &gvs))
    {
        if(ajStrMatchS(gvs->Name, ensMetainformationGetValue(mi)))
            gvsa->DefaultGvsource = (void*) ensGvsourceNewRef(gvs);

        gvsourceadaptorCacheInsert(gvsa, &gvs);

        /*
        ** Both caches hold internal references to the
        ** Ensembl Genetic Variation Source objects.
        */

        ensGvsourceDel(&gvs);
    }

    ajListFree(&gvss);

    while(ajListPop(mis, (void**) &mi))
        ensMetainformationDel(&mi);

    ajListFree(&mis);

    return ajTrue;
}




/* @section constructors ******************************************************
**
** All constructors return a new Ensembl Genetic Variation Source Adaptor by
** pointer. It is the responsibility of the user to first destroy any previous
** Ensembl Genetic Variation Source Adaptor. The target pointer does not need
** to be initialised to NULL, but it is good programming practice to do so
** anyway.
**
** @fdata [EnsPGvsourceadaptor]
**
** @nam3rule New Constructor
**
** @argrule New dba [EnsPDatabaseadaptor] Ensembl Database Adaptor
**
** @valrule * [EnsPGvsourceadaptor] Ensembl Genetic Variation Source Adaptor
**
** @fcategory new
******************************************************************************/




/* @func ensGvsourceadaptorNew ************************************************
**
** Default constructor for an Ensembl Genetic Variation Source Adaptor.
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
** @see ensRegistryGetGvsourceadaptor
**
** @param [u] dba [EnsPDatabaseadaptor] Ensembl Database Adaptor
**
** @return [EnsPGvsourceadaptor] Ensembl Genetic Variation Source Adaptor
**                               or NULL
** @@
******************************************************************************/

EnsPGvsourceadaptor ensGvsourceadaptorNew(EnsPDatabaseadaptor dba)
{
    EnsPGvsourceadaptor gvsa = NULL;

    if(!dba)
        return NULL;

    if(ajDebugTest("ensGvsourceadaptorNew"))
        ajDebug("ensGvsourceadaptorNew\n"
                "  dba %p\n",
                dba);

    AJNEW0(gvsa);

    gvsa->Adaptor = ensBaseadaptorNew(
        dba,
        gvsourceadaptorTables,
        gvsourceadaptorColumns,
        (EnsPBaseadaptorLeftjoin) NULL,
        (const char*) NULL,
        (const char*) NULL,
        gvsourceadaptorFetchAllbyStatement);

    /*
    ** NOTE: The cache cannot be initialised here because the
    ** gvsourceadaptorCacheInit function calls
    ** ensBaseadaptorFetchAllbyConstraint, which calls
    ** gvsourceadaptorFetchAllbyStatement, which calls
    ** ensRegistryGetGvsourceadaptor. At that point, however, the
    ** Ensembl Genetic Variation Source Adaptor has not been stored in the
    ** Ensembl Registry. Therefore, each ensGvsourceadaptorFetch function
    ** has to test the presence of the adaptor-internal cache and eventually
    ** initialise before accessing it.
    **
    **  gvsourceadaptorCacheInit(aa);
    */

    return gvsa;
}




/* @funcstatic gvsourceadaptorCacheClearIdentifier ****************************
**
** An ajTableMapDel "apply" function to clear the Ensembl Genetic Variation
** Source Adaptor-internal Ensembl Genetic Variation Source cache.
** This function deletes the unsigned integer identifier key and the
** Ensembl Genetic Variation value data.
**
** @param [u] key [void**] AJAX unsigned integer key data address
** @param [u] value [void**] Ensembl Genetic Variation Source
**                           value data address
** @param [u] cl [void*] Standard, passed in from ajTableMapDel
** @see ajTableMapDel
**
** @return [void]
** @@
******************************************************************************/

static void gvsourceadaptorCacheClearIdentifier(void** key,
                                                void** value,
                                                void* cl)
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

    ensGvsourceDel((EnsPGvsource*) value);

    *key   = NULL;
    *value = NULL;

    return;
}




/* @funcstatic gvsourceadaptorCacheClearName **********************************
**
** An ajTableMapDel "apply" function to clear the Ensembl Genetic Variation
** Source Adaptor-internal Ensembl Genetic Variation Source cache.
** This function deletes the AJAX String key data and the
** Ensembl Genetic Variation Source value data.
**
** @param [u] key [void**] AJAX String key data address
** @param [u] value [void**] Ensembl Genetic Variation Source
**                           value data address
** @param [u] cl [void*] Standard, passed in from ajTableMapDel
** @see ajTableMapDel
**
** @return [void]
** @@
******************************************************************************/

static void gvsourceadaptorCacheClearName(void** key,
                                          void** value,
                                          void* cl)
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

    ajStrDel((AjPStr*) key);

    ensGvsourceDel((EnsPGvsource*) value);

    *key   = NULL;
    *value = NULL;

    return;
}




/* @funcstatic gvsourceadaptorCacheExit ***************************************
**
** Clears the internal Ensembl Genetic Variation Source cache of an
** Ensembl Genetic Variation Source Adaptor.
**
** @param [u] gvsa [EnsPGvsourceadaptor] Ensembl Genetic Variation
**                                       Source Adaptor
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
** @@
******************************************************************************/

static AjBool gvsourceadaptorCacheExit(EnsPGvsourceadaptor gvsa)
{
    if(!gvsa)
        return ajFalse;

    /* Clear and delete the identifier cache. */

    ajTableMapDel(gvsa->CacheByIdentifier,
                  gvsourceadaptorCacheClearIdentifier,
                  NULL);

    ajTableFree(&gvsa->CacheByIdentifier);

    /* Clear and delete the name cache. */

    ajTableMapDel(gvsa->CacheByName,
                  gvsourceadaptorCacheClearName,
                  NULL);

    ajTableFree(&gvsa->CacheByName);

    ensGvsourceDel((EnsPGvsource*) &gvsa->DefaultGvsource);

    return ajTrue;
}




/* @section destructors *******************************************************
**
** Destruction destroys all internal data structures and frees the
** memory allocated for an Ensembl Genetic Variation Source Adaptor object.
**
** @fdata [EnsPGvsourceadaptor]
**
** @nam3rule Del Destroy (free) an
** Ensembl Genetic Variation Source Adaptor object
**
** @argrule * Pgvsa [EnsPGvsourceadaptor*]
** Ensembl Genetic Variation Source Adaptor object address
**
** @valrule * [void]
**
** @fcategory delete
******************************************************************************/




/* @func ensGvsourceadaptorDel ************************************************
**
** Default destructor for an Ensembl Genetic Variation Source Adaptor.
**
** This function also clears the internal caches.
**
** Ensembl Object Adaptors are singleton objects that are registered in the
** Ensembl Registry and weakly referenced by Ensembl Objects that have been
** instantiated by it. Therefore, Ensembl Object Adaptors should never be
** destroyed directly. Upon exit, the Ensembl Registry will call this function
** if required.
**
** @param [d] Pgvsa [EnsPGvsourceadaptor*]
** Ensembl Genetic Variation Source Adaptor object address
**
** @return [void]
** @@
******************************************************************************/

void ensGvsourceadaptorDel(EnsPGvsourceadaptor* Pgvsa)
{
    EnsPGvsourceadaptor pthis = NULL;

    if(!Pgvsa)
        return;

    if(!*Pgvsa)
        return;

    pthis = *Pgvsa;

    gvsourceadaptorCacheExit(pthis);

    ensBaseadaptorDel(&pthis->Adaptor);

    AJFREE(pthis);

    *Pgvsa = NULL;

    return;
}




/* @section element retrieval *************************************************
**
** Functions for returning elements of an
** Ensembl Genetic Variation Source Adaptor object.
**
** @fdata [EnsPGvsourceadaptor]
**
** @nam3rule Get Return Ensembl Genetic Variation Source Adaptor attribute(s)
** @nam4rule Baseadaptor Return the Ensembl Base Adaptor
** @nam4rule Databaseadaptor Return the Ensembl Database Adaptor
**
** @argrule * gvsa [const EnsPGvsourceadaptor] Ensembl Genetic Variation
**                                             Source Adaptor
**
** @valrule Baseadaptor [EnsPBaseadaptor] Ensembl Base Adaptor or NULL
** @valrule Databaseadaptor [EnsPDatabaseadaptor] Ensembl Database Adaptor or
** NULL
**
** @fcategory use
******************************************************************************/




/* @func ensGvsourceadaptorGetBaseadaptor *************************************
**
** Get the Ensembl Base Adaptor element of an
** Ensembl Genetic Variation Source Adaptor.
**
** @param [r] gvsa [const EnsPGvsourceadaptor] Ensembl Genetic Variation
**                                             Source Adaptor
**
** @return [EnsPBaseadaptor] Ensembl Base Adaptor
** @@
******************************************************************************/

EnsPBaseadaptor ensGvsourceadaptorGetBaseadaptor(
    const EnsPGvsourceadaptor gvsa)
{
    if(!gvsa)
        return NULL;

    return gvsa->Adaptor;
}




/* @func ensGvsourceadaptorGetDatabaseadaptor *********************************
**
** Get the Ensembl Database Adaptor element of an
** Ensembl Genetic Variation Source Adaptor.
**
** @param [r] gvsa [const EnsPGvsourceadaptor] Ensembl Genetic Variation
**                                             Source Adaptor
**
** @return [EnsPDatabaseadaptor] Ensembl Database Adaptor
** @@
******************************************************************************/

EnsPDatabaseadaptor ensGvsourceadaptorGetDatabaseadaptor(
    const EnsPGvsourceadaptor gvsa)
{
    if(!gvsa)
        return NULL;

    return ensBaseadaptorGetDatabaseadaptor(gvsa->Adaptor);
}




/* @section object retrieval **************************************************
**
** Functions for fetching Ensembl Genetic Variation Source objects from an
** Ensembl Genetic Variation database.
**
** @fdata [EnsPGvsourceadaptor]
**
** @nam3rule Fetch Fetch Ensembl Genetic Variation Source object(s)
** @nam4rule FetchAll Fetch all Ensembl Genetic Variation Source objects
** @nam4rule FetchAllby Fetch all Ensembl Genetic Variation Source objects
**                      matching a criterion
** @nam5rule Gvsource Fetch by an Ensembl Genetic Variation Source
** @nam6rule GvsourceType
** Fetch all by an Ensembl Genetic Variation Source Type enumeration
** @nam4rule FetchBy Fetch one Ensembl Genetic Variation Source object
**                   matching a criterion
** @nam5rule Identifier Fetch by an SQL database-internal identifier
** @nam5rule Name       Fetch by a name
** @nam4rule Default    Fetch the default
**
** @argrule * gvsa [EnsPGvsourceadaptor]
** Ensembl Genetic Variation Source Adaptor
** @argrule All gvss [AjPList]
** AJAX List of Ensembl Genetic Variation Source objects
** @argrule GvsourceType gvst [EnsEGvsourceType]
** Ensembl Genetic Variation Source Type enumeration
** @argrule Allby gvss [AjPList]
** AJAX List of Ensembl Genetic Variation Source objects
** @argrule ByIdentifier identifier [ajuint] SQL database-internal identifier
** @argrule ByIdentifier Pgvs [EnsPGvsource*]
** Ensembl Genetic Variation Source address
** @argrule ByName name [const AjPStr] Ensembl Genetic Variation Source name
** @argrule ByName Pgvs [EnsPGvsource*]
** Ensembl Genetic Variation Source address
** @argrule Default Pgvs [EnsPGvsource*]
** Ensembl Genetic Variation Source address
**
** @valrule * [AjBool] ajTrue upon success, ajFalse otherwise
**
** @fcategory use
******************************************************************************/




/* @funcstatic gvsourceadaptorFetchAll ****************************************
**
** An ajTableMap "apply" function to return all
** Ensembl Genetic Variation Source objects from the
** Ensembl Genetic Variation Source Adaptor-internal cache.
**
** @param [u] key [const void*] AJAX unsigned integer key data address
** @param [u] value [void**] Ensembl Genetic Variation Source
**                           value data address
** @param [u] cl [void*] AJAX List of Ensembl Genetic Variation Source objects,
**                       passed in via ajTableMap
** @see ajTableMap
**
** @return [void]
** @@
******************************************************************************/

static void gvsourceadaptorFetchAll(const void* key,
                                    void** value,
                                    void* cl)
{
    if(!key)
        return;

    if(!value)
        return;

    if(!*value)
        return;

    if(!cl)
        return;

    ajListPushAppend((AjPList) cl,
                     (void*) ensGvsourceNewRef(*((EnsPGvsource*) value)));

    return;
}




/* @func ensGvsourceadaptorFetchAll *******************************************
**
** Fetch all Ensembl Genetic Variation Source objects.
**
** The caller is responsible for deleting the Ensembl Genetic Variation Source
** objects before deleting the AJAX List.
**
** @cc Bio::EnsEMBL::Variation::DBSQL::VariationAdaptor::get_all_sources
** @param [u] gvsa [EnsPGvsourceadaptor]
** Ensembl Genetic Variation Source Adaptor
** @param [u] gvss [AjPList]
** AJAX List of Ensembl Genetic Variation Source objects
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
** @@
******************************************************************************/

AjBool ensGvsourceadaptorFetchAll(
    EnsPGvsourceadaptor gvsa,
    AjPList gvss)
{
    if(!gvsa)
        return ajFalse;

    if(!gvss)
        return ajFalse;

    if(!gvsa->CacheByIdentifier)
        gvsourceadaptorCacheInit(gvsa);

    ajTableMap(gvsa->CacheByIdentifier,
               gvsourceadaptorFetchAll,
               (void*) gvss);

    return ajTrue;
}




/* @func ensGvsourceadaptorFetchAllbyGvsourceType *****************************
**
** Fetch all Ensembl Genetic Variation Source objects by an
** Ensembl Genetic Variation Source Type enumeration.
**
** The caller is responsible for deleting the Ensembl Genetic Variation Source
** objects before deleting the AJAX List.
**
** @cc Bio::EnsEMBL::Variation::DBSQL::VariationAdaptor::
** fetch_all_by_source_type
** @param [u] gvsa [EnsPGvsourceadaptor]
** Ensembl Genetic Variation Source Adaptor
** @param [u] gvst [EnsEGvsourceType]
** Ensembl Genetic Variation Source Type enumeration
** @param [u] gvss [AjPList]
** AJAX List of Ensembl Genetic Variation Source objects
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
** @@
******************************************************************************/

AjBool ensGvsourceadaptorFetchAllbyGvsourceType(
    EnsPGvsourceadaptor gvsa,
    EnsEGvsourceType gvst,
    AjPList gvss)
{
    register ajuint i = 0;

    void** valarray = NULL;

    if(!gvsa)
        return ajFalse;

    if(!gvst)
        return ajFalse;

    if(!gvsa->CacheByIdentifier)
        gvsourceadaptorCacheInit(gvsa);

    ajTableToarrayValues(gvsa->CacheByIdentifier, &valarray);

    for(i = 0; valarray[i]; i++)
        if(gvst == ensGvsourceGetType((EnsPGvsource) valarray[i]))
            ajListPushAppend(
                gvss,
                (void*) ensGvsourceNewRef((EnsPGvsource) valarray[i]));

    AJFREE(valarray);

    return ajTrue;
}




/* @func ensGvsourceadaptorFetchByIdentifier **********************************
**
** Fetch an Ensembl Genetic Variation Source by its SQL database-internal
** identifier.
**
** The caller is responsible for deleting the Ensembl Genetic Variation Source.
**
** @param [u] gvsa [EnsPGvsourceadaptor] Ensembl Genetic Variation
**                                       Source Adaptor
** @param [r] identifier [ajuint] SQL database-internal identifier
** @param [wP] Pgvs [EnsPGvsource*] Ensembl Genetic Variation Source address
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
** @@
******************************************************************************/

AjBool ensGvsourceadaptorFetchByIdentifier(
    EnsPGvsourceadaptor gvsa,
    ajuint identifier,
    EnsPGvsource* Pgvs)
{
    AjPList gvss = NULL;

    AjPStr constraint = NULL;

    EnsPGvsource gvs = NULL;

    if(!gvsa)
        return ajFalse;

    if(!identifier)
        return ajFalse;

    if(!Pgvs)
        return ajFalse;

    /*
    ** Initially, search the identifier cache.
    ** For any object returned by the AJAX Table the reference counter needs
    ** to be incremented manually.
    */

    if(!gvsa->CacheByIdentifier)
        gvsourceadaptorCacheInit(gvsa);

    *Pgvs = (EnsPGvsource) ajTableFetchmodV(gvsa->CacheByIdentifier,
                                            (const void*) &identifier);

    if(*Pgvs)
    {
        ensGvsourceNewRef(*Pgvs);

        return ajTrue;
    }

    /* For a cache miss re-query the database. */

    constraint = ajFmtStr("source.source_id = %u", identifier);

    gvss = ajListNew();

    ensBaseadaptorFetchAllbyConstraint(gvsa->Adaptor,
                                       constraint,
                                       (EnsPAssemblymapper) NULL,
                                       (EnsPSlice) NULL,
                                       gvss);

    if(ajListGetLength(gvss) > 1)
        ajWarn("ensGvsourceadaptorFetchByIdentifier got more than one "
               "Ensembl Genetic Variation Source objects for (PRIMARY KEY) "
               "identifier %u.\n",
               identifier);

    ajListPop(gvss, (void**) Pgvs);

    gvsourceadaptorCacheInsert(gvsa, Pgvs);

    while(ajListPop(gvss, (void**) &gvs))
    {
        gvsourceadaptorCacheInsert(gvsa, &gvs);

        ensGvsourceDel(&gvs);
    }

    ajListFree(&gvss);

    ajStrDel(&constraint);

    return ajTrue;
}




/* @func ensGvsourceadaptorFetchByName ****************************************
**
** Fetch an Ensembl Genetic Variation Source by its name.
**
** The caller is responsible for deleting the Ensembl Genetic Variation Source.
**
** @cc Bio::EnsEMBL::Variation::DBSQL::VariationAdaptor::get_source_version
** @param [u] gvsa [EnsPGvsourceadaptor] Ensembl Genetic Variation
**                                       Source Adaptor
** @param [r] name [const AjPStr] Ensembl Genetic Variation Source name
** @param [wP] Pgvs [EnsPGvsource*] Ensembl Genetic Variation Source address
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
** @@
******************************************************************************/

AjBool ensGvsourceadaptorFetchByName(
    EnsPGvsourceadaptor gvsa,
    const AjPStr name,
    EnsPGvsource* Pgvs)
{
    char* txtname = NULL;

    AjPList gvss = NULL;

    AjPStr constraint = NULL;

    EnsPGvsource gvs = NULL;

    if(!gvsa)
        return ajFalse;

    if((!name) && (!ajStrGetLen(name)))
        return ajFalse;

    if(!Pgvs)
        return ajFalse;

    /*
    ** Initially, search the name cache.
    ** For any object returned by the AJAX Table the reference counter needs
    ** to be incremented manually.
    */

    if(!gvsa->CacheByName)
        gvsourceadaptorCacheInit(gvsa);

    *Pgvs = (EnsPGvsource) ajTableFetchmodV(gvsa->CacheByName,
                                            (const void*) name);

    if(*Pgvs)
    {
        ensGvsourceNewRef(*Pgvs);

        return ajTrue;
    }

    /* In case of a cache miss, re-query the database. */

    ensBaseadaptorEscapeC(gvsa->Adaptor, &txtname, name);

    constraint = ajFmtStr("source.name = '%s'", txtname);

    ajCharDel(&txtname);

    gvss = ajListNew();

    ensBaseadaptorFetchAllbyConstraint(gvsa->Adaptor,
                                       constraint,
                                       (EnsPAssemblymapper) NULL,
                                       (EnsPSlice) NULL,
                                       gvss);

    if(ajListGetLength(gvss) > 1)
        ajWarn("ensGvsourceadaptorFetchByName got more than one "
               "Ensembl Genetic Variation Source for (UNIQUE) name '%S'.\n",
               name);

    ajListPop(gvss, (void**) Pgvs);

    gvsourceadaptorCacheInsert(gvsa, Pgvs);

    while(ajListPop(gvss, (void**) &gvs))
    {
        gvsourceadaptorCacheInsert(gvsa, &gvs);

        ensGvsourceDel(&gvs);
    }

    ajListFree(&gvss);

    ajStrDel(&constraint);

    return ajTrue;
}




/* @func ensGvsourceadaptorFetchDefault ***************************************
**
** Fetch the default Ensembl Genetic Variation Source.
**
** The caller is responsible for deleting the Ensembl Genetic Variation Source.
**
** @cc Bio::EnsEMBL::Variation::DBSQL::VariationAdaptor::get_default_source
** @param [u] gvsa [EnsPGvsourceadaptor] Ensembl Genetic Variation
**                                       Source Adaptor
** @param [u] Pgvs [EnsPGvsource*] Ensembl Genetic Variation Source address
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
** @@
******************************************************************************/

AjBool ensGvsourceadaptorFetchDefault(
    EnsPGvsourceadaptor gvsa,
    EnsPGvsource* Pgvs)
{
    if(!gvsa)
        return ajFalse;

    if(!Pgvs)
        return ajFalse;

    *Pgvs = ensGvsourceNewRef((EnsPGvsource) gvsa->DefaultGvsource);

    return ajTrue;
}
