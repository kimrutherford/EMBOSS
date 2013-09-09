/* @source ensgvsource ********************************************************
**
** Ensembl Genetic Variation Source functions
**
** @author Copyright (C) 1999 Ensembl Developers
** @author Copyright (C) 2006 Michael K. Schuster
** @version $Revision: 1.26 $
** @modified 2009 by Alan Bleasby for incorporation into EMBOSS core
** @modified $Date: 2013/02/17 13:02:10 $ by $Author: mks $
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

#include "ensgvsource.h"
#include "ensmetainformation.h"
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

/* @conststatic gvsourceKSomatic **********************************************
**
** The Ensembl Genetic Variation Source somatic member is
** enumerated in both, the SQL table definition and the data structure.
** The following strings are used for conversion in database operations and
** correspond to EnsEGvsourceSomatic and the
** "source.somatic_status" field.
**
** #attr [static const char *const*] gvsourceKSomatic
** ##
******************************************************************************/

static const char *const gvsourceKSomatic[] =
{
    "",
    "germline",
    "somatic",
    "mixed",
    (const char *) NULL
};




/* @conststatic gvsourceKType *************************************************
**
** The Ensembl Genetic Variation Source type member is
** enumerated in both, the SQL table definition and the data structure.
** The following strings are used for conversion in database operations and
** correspond to EnsEGvsourceType and the
** "source.type" field.
**
** #attr [static const char *const*] gvsourceKType
** ##
******************************************************************************/

static const char *const gvsourceKType[] =
{
    "",
    "chip",
    (const char *) NULL
};




/* @conststatic gvsourceadaptorKTablenames ************************************
**
** Array of Ensembl Genetic Variation Source Adaptor SQL table names
**
******************************************************************************/

static const char *const gvsourceadaptorKTablenames[] =
{
    "source",
    (const char *) NULL
};




/* @conststatic gvsourceadaptorKColumnnames ***********************************
**
** Array of Ensembl Genetic Variation Source Adaptor SQL column names
**
******************************************************************************/

static const char *const gvsourceadaptorKColumnnames[] =
{
    "source.source_id",
    "source.name",
    "source.version",
    "source.description",
    "source.url",
    "source.type",
    "source.somatic_status",
    (const char *) NULL
};




/* ========================================================================= */
/* =========================== private variables =========================== */
/* ========================================================================= */




/* ========================================================================= */
/* =========================== private functions =========================== */
/* ========================================================================= */

static AjBool gvsourceadaptorFetchAllbyStatement(
    EnsPBaseadaptor ba,
    const AjPStr statement,
    EnsPAssemblymapper am,
    EnsPSlice slice,
    AjPList gvss);

static AjBool gvsourceadaptorCacheInsert(EnsPGvsourceadaptor gvsa,
                                         EnsPGvsource *Pgvs);

static AjBool gvsourceadaptorCacheInit(EnsPGvsourceadaptor gvsa);

static void gvsourceadaptorFetchAll(const void *key,
                                    void **Pvalue,
                                    void *cl);




/* ========================================================================= */
/* ======================= All functions by section ======================== */
/* ========================================================================= */




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
** @cc CVS Revision: 1.68
** @cc CVS Tag: branch-ensembl-68
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
** @valrule * [EnsPGvsource] Ensembl Genetic Variation Source or NULL
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
**
** @release 6.4.0
** @@
******************************************************************************/

EnsPGvsource ensGvsourceNewCpy(const EnsPGvsource gvs)
{
    EnsPGvsource pthis = NULL;

    if (!gvs)
        return NULL;

    AJNEW0(pthis);

    pthis->Use        = 1U;
    pthis->Identifier = gvs->Identifier;
    pthis->Adaptor    = gvs->Adaptor;

    if (gvs->Name)
        pthis->Name = ajStrNewRef(gvs->Name);

    if (gvs->Version)
        pthis->Version = ajStrNewRef(gvs->Version);

    if (gvs->Description)
        pthis->Description = ajStrNewRef(gvs->Description);

    if (gvs->URL)
        pthis->URL = ajStrNewRef(gvs->URL);

    pthis->Somatic = gvs->Somatic;
    pthis->Type    = gvs->Type;

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
** @return [EnsPGvsource] Ensembl Genetic Variation Source or NULL
**
** @release 6.4.0
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

    gvs->Use        = 1U;
    gvs->Identifier = identifier;
    gvs->Adaptor    = gvsa;

    if (name)
        gvs->Name = ajStrNewRef(name);

    if (version)
        gvs->Version = ajStrNewRef(version);

    if (description)
        gvs->Description = ajStrNewRef(description);

    if (url)
        gvs->URL = ajStrNewRef(url);

    gvs->Somatic = gvss;
    gvs->Type    = gvst;

    return gvs;
}




/* @func ensGvsourceNewRef ****************************************************
**
** Ensembl Object referencing function, which returns a pointer to the
** Ensembl Object passed in and increases its reference count.
**
** @param [u] gvs [EnsPGvsource] Ensembl Genetic Variation Source
**
** @return [EnsPGvsource] Ensembl Genetic Variation Source or NULL
**
** @release 6.3.0
** @@
******************************************************************************/

EnsPGvsource ensGvsourceNewRef(EnsPGvsource gvs)
{
    if (!gvs)
        return NULL;

    gvs->Use++;

    return gvs;
}




/* @section destructors *******************************************************
**
** Destruction destroys all internal data structures and frees the memory
** allocated for an Ensembl Genetic Variation Source object.
**
** @fdata [EnsPGvsource]
**
** @nam3rule Del Destroy (free) an Ensembl Genetic Variation Source
**
** @argrule * Pgvs [EnsPGvsource*] Ensembl Genetic Variation Source address
**
** @valrule * [void]
**
** @fcategory delete
******************************************************************************/




/* @func ensGvsourceDel *******************************************************
**
** Default destructor for an Ensembl Genetic Variation Source.
**
** @param [d] Pgvs [EnsPGvsource*] Ensembl Genetic Variation Source address
**
** @return [void]
**
** @release 6.3.0
** @@
******************************************************************************/

void ensGvsourceDel(EnsPGvsource *Pgvs)
{
    EnsPGvsource pthis = NULL;

    if (!Pgvs)
        return;

#if defined(AJ_DEBUG) && AJ_DEBUG >= 1
    if (ajDebugTest("ensGvsourceDel"))
    {
        ajDebug("ensGvsourceDel\n"
                "  *Pgvs %p\n",
                *Pgvs);

        ensGvsourceTrace(*Pgvs, 1);
    }
#endif /* defined(AJ_DEBUG) && AJ_DEBUG >= 1 */

    if (!(pthis = *Pgvs) || --pthis->Use)
    {
        *Pgvs = NULL;

        return;
    }

    ajStrDel(&pthis->Name);
    ajStrDel(&pthis->Version);
    ajStrDel(&pthis->Description);
    ajStrDel(&pthis->URL);

    ajMemFree((void **) Pgvs);

    return;
}




/* @section member retrieval **************************************************
**
** Functions for returning members of an
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
** @valrule Identifier [ajuint] SQL database-internal identifier or 0U
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
** Get the Ensembl Genetic Variation Source Adaptor member of an
** Ensembl Genetic Variation Source.
**
** @param [r] gvs [const EnsPGvsource] Ensembl Genetic Variation Source
**
** @return [EnsPGvsourceadaptor] Ensembl Genetic Variation Source Adaptor
** or NULL
**
** @release 6.3.0
** @@
******************************************************************************/

EnsPGvsourceadaptor ensGvsourceGetAdaptor(const EnsPGvsource gvs)
{
    return (gvs) ? gvs->Adaptor : NULL;
}




/* @func ensGvsourceGetDescription ********************************************
**
** Get the description member of an Ensembl Genetic Variation Source.
**
** @cc Bio::EnsEMBL::Variation::Variation::source_description
** @param [r] gvs [const EnsPGvsource] Ensembl Genetic Variation Source
**
** @return [AjPStr] Description or NULL
**
** @release 6.3.0
** @@
******************************************************************************/

AjPStr ensGvsourceGetDescription(const EnsPGvsource gvs)
{
    return (gvs) ? gvs->Description : NULL;
}





/* @func ensGvsourceGetIdentifier *********************************************
**
** Get the SQL database-internal identifier member of an
** Ensembl Genetic Variation Source.
**
** @param [r] gvs [const EnsPGvsource] Ensembl Genetic Variation Source
**
** @return [ajuint] SQL database-internal identifier or 0U
**
** @release 6.3.0
** @@
******************************************************************************/

ajuint ensGvsourceGetIdentifier(const EnsPGvsource gvs)
{
    return (gvs) ? gvs->Identifier : 0U;
}





/* @func ensGvsourceGetName ***************************************************
**
** Get the name member of an Ensembl Genetic Variation Source.
**
** @param [r] gvs [const EnsPGvsource] Ensembl Genetic Variation Source
**
** @return [AjPStr] Name or NULL
**
** @release 6.3.0
** @@
******************************************************************************/

AjPStr ensGvsourceGetName(const EnsPGvsource gvs)
{
    return (gvs) ? gvs->Name : NULL;
}





/* @func ensGvsourceGetSomatic ************************************************
**
** Get the Ensembl Genetic Variation Source Somatic enumeration member of an
** Ensembl Genetic Variation Source.
**
** @cc Bio::EnsEMBL::Variation::Variation::is_somatic
** @param [r] gvs [const EnsPGvsource] Ensembl Genetic Variation Source
**
** @return [EnsEGvsourceSomatic]
** Ensembl Genetic Variation Source Somatic enumeration or
** ensEGvsourceSomaticNULL
**
** @release 6.4.0
** @@
******************************************************************************/

EnsEGvsourceSomatic ensGvsourceGetSomatic(const EnsPGvsource gvs)
{
    return (gvs) ? gvs->Somatic : ensEGvsourceSomaticNULL;
}




/* @func ensGvsourceGetType ***************************************************
**
** Get the Ensembl Genetic Variation Source Type enumeration member of an
** Ensembl Genetic Variation Source.
**
** @cc Bio::EnsEMBL::Variation::Variation::source_type
** @param [r] gvs [const EnsPGvsource] Ensembl Genetic Variation Source
**
** @return [EnsEGvsourceType]
** Ensembl Genetic Variation Source Type enumeration or
** ensEGvsourceTypeNULL
**
** @release 6.4.0
** @@
******************************************************************************/

EnsEGvsourceType ensGvsourceGetType(const EnsPGvsource gvs)
{
    return (gvs) ? gvs->Type : ensEGvsourceTypeNULL;
}




/* @func ensGvsourceGetUrl ****************************************************
**
** Get the Uniform Resource Locator member of an
** Ensembl Genetic Variation Source.
**
** @cc Bio::EnsEMBL::Variation::Variation::source_url
** @param [r] gvs [const EnsPGvsource] Ensembl Genetic Variation Source
**
** @return [AjPStr] Uniform Resource Locator or NULL
**
** @release 6.4.0
** @@
******************************************************************************/

AjPStr ensGvsourceGetUrl(const EnsPGvsource gvs)
{
    return (gvs) ? gvs->URL : NULL;
}





/* @func ensGvsourceGetVersion ************************************************
**
** Get the version member of an Ensembl Genetic Variation Source.
**
** @param [r] gvs [const EnsPGvsource] Ensembl Genetic Variation Source
**
** @return [AjPStr] Version or NULL
**
** @release 6.3.0
** @@
******************************************************************************/

AjPStr ensGvsourceGetVersion(const EnsPGvsource gvs)
{
    return (gvs) ? gvs->Version : NULL;
}





/* @section member assignment *************************************************
**
** Functions for assigning members of an
** Ensembl Genetic Variation Source object.
**
** @fdata [EnsPGvsource]
**
** @nam3rule Set Set one member of a Genetic Variation Source
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
** Set the Ensembl Genetic Variation Source Adaptor member of an
** Ensembl Genetic Variation Source.
**
** @param [u] gvs [EnsPGvsource] Ensembl Genetic Variation Source
** @param [u] gvsa [EnsPGvsourceadaptor] Ensembl Genetic Variation
**                                       Source Adaptor
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
**
** @release 6.3.0
** @@
******************************************************************************/

AjBool ensGvsourceSetAdaptor(EnsPGvsource gvs, EnsPGvsourceadaptor gvsa)
{
    if (!gvs)
        return ajFalse;

    gvs->Adaptor = gvsa;

    return ajTrue;
}




/* @func ensGvsourceSetDescription ********************************************
**
** Set the description member of an Ensembl Genetic Variation Source.
**
** @param [u] gvs [EnsPGvsource] Ensembl Genetic Variation Source
** @param [u] description [AjPStr] Description
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
**
** @release 6.3.0
** @@
******************************************************************************/

AjBool ensGvsourceSetDescription(EnsPGvsource gvs, AjPStr description)
{
    if (!gvs)
        return ajFalse;

    ajStrDel(&gvs->Description);

    if (description)
        gvs->Description = ajStrNewRef(description);

    return ajTrue;
}




/* @func ensGvsourceSetIdentifier *********************************************
**
** Set the SQL database-internal identifier member of an
** Ensembl Genetic Variation Source.
**
** @param [u] gvs [EnsPGvsource] Ensembl Genetic Variation Source
** @param [r] identifier [ajuint] SQL database-internal identifier
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
**
** @release 6.3.0
** @@
******************************************************************************/

AjBool ensGvsourceSetIdentifier(EnsPGvsource gvs, ajuint identifier)
{
    if (!gvs)
        return ajFalse;

    gvs->Identifier = identifier;

    return ajTrue;
}




/* @func ensGvsourceSetName ***************************************************
**
** Set the name member of an Ensembl Genetic Variation Source.
**
** @param [u] gvs [EnsPGvsource] Ensembl Genetic Variation Source
** @param [u] name [AjPStr] Name
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
**
** @release 6.3.0
** @@
******************************************************************************/

AjBool ensGvsourceSetName(EnsPGvsource gvs, AjPStr name)
{
    if (!gvs)
        return ajFalse;

    ajStrDel(&gvs->Name);

    if (name)
        gvs->Name = ajStrNewRef(name);

    return ajTrue;
}




/* @func ensGvsourceSetSomatic ************************************************
**
** Set the Ensembl Genetic Variation Source Somatic enumeration member of an
** Ensembl Genetic Variation Source.
**
** @cc Bio::EnsEMBL::Variation::Variation::is_somatic
** @param [u] gvs [EnsPGvsource] Ensembl Genetic Variation Source
** @param [u] gvss [EnsEGvsourceSomatic]
** Ensembl Genetic Variation Source Somatic enumeration
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
**
** @release 6.4.0
** @@
******************************************************************************/

AjBool ensGvsourceSetSomatic(EnsPGvsource gvs, EnsEGvsourceSomatic gvss)
{
    if (!gvs)
        return ajFalse;

    gvs->Somatic = gvss;

    return ajTrue;
}




/* @func ensGvsourceSetType ***************************************************
**
** Set the Ensembl Genetic Variation Source Type enumeration member of an
** Ensembl Genetic Variation Source.
**
** @cc Bio::EnsEMBL::Variation::Variation::source_type
** @param [u] gvs [EnsPGvsource] Ensembl Genetic Variation Source
** @param [u] gvst [EnsEGvsourceType]
** Ensembl Genetic Variation Source Type enumeration
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
**
** @release 6.4.0
** @@
******************************************************************************/

AjBool ensGvsourceSetType(EnsPGvsource gvs, EnsEGvsourceType gvst)
{
    if (!gvs)
        return ajFalse;

    gvs->Type = gvst;

    return ajTrue;
}




/* @func ensGvsourceSetUrl ****************************************************
**
** Set the Uniform Resopurce Locator member of an
** Ensembl Genetic Variation Source.
**
** @param [u] gvs [EnsPGvsource] Ensembl Genetic Variation Source
** @param [u] url [AjPStr] Uniform Resopurce Locator
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
**
** @release 6.4.0
** @@
******************************************************************************/

AjBool ensGvsourceSetUrl(EnsPGvsource gvs, AjPStr url)
{
    if (!gvs)
        return ajFalse;

    ajStrDel(&gvs->URL);

    if (url)
        gvs->URL = ajStrNewRef(url);

    return ajTrue;
}




/* @func ensGvsourceSetVersion ************************************************
**
** Set the version member of an Ensembl Genetic Variation Source.
**
** @param [u] gvs [EnsPGvsource] Ensembl Genetic Variation Source
** @param [u] version [AjPStr] Version
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
**
** @release 6.3.0
** @@
******************************************************************************/

AjBool ensGvsourceSetVersion(EnsPGvsource gvs, AjPStr version)
{
    if (!gvs)
        return ajFalse;

    ajStrDel(&gvs->Version);

    if (version)
        gvs->Version = ajStrNewRef(version);

    return ajTrue;
}




/* @section debugging *********************************************************
**
** Functions for reporting of an Ensembl Genetic Variation Source object.
**
** @fdata [EnsPGvsource]
**
** @nam3rule Trace Report Ensembl Genetic Variation Source members to
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
**
** @release 6.3.0
** @@
******************************************************************************/

AjBool ensGvsourceTrace(const EnsPGvsource gvs, ajuint level)
{
    AjPStr indent = NULL;

    if (!gvs)
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
** Functions for calculating information from an
** Ensembl Genetic Variation Source object.
**
** @fdata [EnsPGvsource]
**
** @nam3rule Calculate Calculate Ensembl Genetic Variation Source information
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
**
** @release 6.4.0
** @@
******************************************************************************/

size_t ensGvsourceCalculateMemsize(const EnsPGvsource gvs)
{
    size_t size = 0;

    if (!gvs)
        return 0;

    size += sizeof (EnsOGvsource);

    if (gvs->Name)
    {
        size += sizeof (AjOStr);

        size += ajStrGetRes(gvs->Name);
    }

    if (gvs->Version)
    {
        size += sizeof (AjOStr);

        size += ajStrGetRes(gvs->Version);
    }

    if (gvs->Description)
    {
        size += sizeof (AjOStr);

        size += ajStrGetRes(gvs->Description);
    }

    if (gvs->URL)
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
**
** @release 6.4.0
** @@
** The comparison is based on an initial pointer equality test and if that
** fails, a case-insensitive string comparison of the name and version members
** is performed.
******************************************************************************/

AjBool ensGvsourceMatch(const EnsPGvsource gvs1, const EnsPGvsource gvs2)
{
    if (ajDebugTest("ensGvsourceMatch"))
    {
        ajDebug("ensGvsourceMatch\n"
                "  gvs1 %p\n"
                "  gvs2 %p\n",
                gvs1,
                gvs2);

        ensGvsourceTrace(gvs1, 1);
        ensGvsourceTrace(gvs2, 1);
    }

    if (!gvs1)
        return ajFalse;

    if (!gvs2)
        return ajFalse;

    if (gvs1 == gvs2)
        return ajTrue;

    if (gvs1->Identifier != gvs2->Identifier)
        return ajFalse;

    if (gvs1->Adaptor != gvs2->Adaptor)
        return ajFalse;

    if (!ajStrMatchCaseS(gvs1->Name, gvs2->Name))
        return ajFalse;

    if (!ajStrMatchCaseS(gvs1->Version, gvs2->Version))
        return ajFalse;

    if (!ajStrMatchCaseS(gvs1->Description, gvs2->Description))
        return ajFalse;

    if (!ajStrMatchCaseS(gvs1->URL, gvs2->URL))
        return ajFalse;

    if (gvs1->Somatic != gvs2->Somatic)
        return ajFalse;

    if (gvs1->Type != gvs2->Type)
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
**
** @release 6.4.0
** @@
******************************************************************************/

EnsEGvsourceSomatic ensGvsourceSomaticFromStr(
    const AjPStr somatic)
{
    register EnsEGvsourceSomatic i =
        ensEGvsourceSomaticNULL;

    EnsEGvsourceSomatic gvss =
        ensEGvsourceSomaticNULL;

    for (i = ensEGvsourceSomaticNULL;
         gvsourceKSomatic[i];
         i++)
        if (ajStrMatchC(somatic, gvsourceKSomatic[i]))
            gvss = i;

    if (!gvss)
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
** @argrule To gvss [EnsEGvsourceSomatic]
** Ensembl Genetic Variation Source Somatic enumeration
**
** @valrule Char [const char*]
** Ensembl Genetic Variation Source Somatic C-type (char *) string
**
** @fcategory cast
******************************************************************************/




/* @func ensGvsourceSomaticToChar *********************************************
**
** Convert an Ensembl Genetic Variation Source Somatic enumeration
** into a C-type (char *) string.
**
** @param [u] gvss [EnsEGvsourceSomatic]
** Ensembl Genetic Variation Source Somatic enumeration
**
** @return [const char*]
** Ensembl Genetic Variation Source Somatic C-type (char *) string
**
** @release 6.4.0
** @@
******************************************************************************/

const char* ensGvsourceSomaticToChar(
    EnsEGvsourceSomatic gvss)
{
    register EnsEGvsourceSomatic i =
        ensEGvsourceSomaticNULL;

    for (i = ensEGvsourceSomaticNULL;
         gvsourceKSomatic[i] && (i < gvss);
         i++);

    if (!gvsourceKSomatic[i])
        ajDebug("ensGvsourceSomaticToChar "
                "encountered an out of boundary error on "
                "Ensembl Genetic Variation Source Somatic "
                "enumeration %d.\n",
                gvss);

    return gvsourceKSomatic[i];
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
** @valrule * [EnsEGvsourceType]
** Ensembl Genetic Variation Source Type enumeration or ensEGvsourceTypeNULL
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
** @return [EnsEGvsourceType]
** Ensembl Genetic Variation Source Type enumeration or ensEGvsourceTypeNULL
**
** @release 6.4.0
** @@
******************************************************************************/

EnsEGvsourceType ensGvsourceTypeFromStr(
    const AjPStr type)
{
    register EnsEGvsourceType i =
        ensEGvsourceTypeNULL;

    EnsEGvsourceType gvst =
        ensEGvsourceTypeNULL;

    for (i = ensEGvsourceTypeNULL;
         gvsourceKType[i];
         i++)
        if (ajStrMatchC(type, gvsourceKType[i]))
            gvst = i;

    if (!gvst)
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
** @argrule To gvst [EnsEGvsourceType]
** Ensembl Genetic Variation Source Type enumeration
**
** @valrule Char [const char*]
** Ensembl Genetic Variation Source Type C-type (char *) string
**
** @fcategory cast
******************************************************************************/




/* @func ensGvsourceTypeToChar ************************************************
**
** Convert an Ensembl Genetic Variation Source Type enumeration
** into a C-type (char *) string.
**
** @param [u] gvst [EnsEGvsourceType]
** Ensembl Genetic Variation Source Type enumeration
**
** @return [const char*]
** Ensembl Genetic Variation Source Type C-type (char *) string
**
** @release 6.4.0
** @@
******************************************************************************/

const char* ensGvsourceTypeToChar(
    EnsEGvsourceType gvst)
{
    register EnsEGvsourceType i =
        ensEGvsourceTypeNULL;

    for (i = ensEGvsourceTypeNULL;
         gvsourceKType[i] && (i < gvst);
         i++);

    if (!gvsourceKType[i])
        ajDebug("ensGvsourceTypeToChar "
                "encountered an out of boundary error on "
                "Ensembl Genetic Variation Source Type "
                "enumeration %d.\n",
                gvst);

    return gvsourceKType[i];
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
** @cc CVS Revision: 1.90
** @cc CVS Tag: branch-ensembl-68
**
******************************************************************************/




/* @funcstatic gvsourceadaptorFetchAllbyStatement *****************************
**
** Run a SQL statement against an Ensembl Database Adaptor and consolidate the
** results into an AJAX List of Ensembl Genetic Variation Source objects.
**
** @cc Bio::EnsEMBL::Variation::DBSQL::VariationAdaptor::_objFromHashref
** @param [u] ba [EnsPBaseadaptor] Ensembl Base Adaptor
** @param [r] statement [const AjPStr] SQL statement
** @param [uN] am [EnsPAssemblymapper] Ensembl Assembly Mapper
** @param [uN] slice [EnsPSlice] Ensembl Slice
** @param [u] gvss [AjPList] AJAX List of Ensembl Genetic Variation Source
** objects
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
**
** @release 6.4.0
** @@
******************************************************************************/

static AjBool gvsourceadaptorFetchAllbyStatement(
    EnsPBaseadaptor ba,
    const AjPStr statement,
    EnsPAssemblymapper am,
    EnsPSlice slice,
    AjPList gvss)
{
    ajuint identifier = 0U;

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

    EnsPDatabaseadaptor dba = NULL;

    EnsPGvsource        gvs  = NULL;
    EnsPGvsourceadaptor gvsa = NULL;

    if (ajDebugTest("gvsourceadaptorFetchAllbyStatement"))
        ajDebug("gvsourceadaptorFetchAllbyStatement\n"
                "  ba %p\n"
                "  statement %p\n"
                "  am %p\n"
                "  slice %p\n"
                "  gvss %p\n",
                ba,
                statement,
                am,
                slice,
                gvss);

    if (!ba)
        return ajFalse;

    if (!statement)
        return ajFalse;

    if (!gvss)
        return ajFalse;

    dba = ensBaseadaptorGetDatabaseadaptor(ba);

    gvsa = ensRegistryGetGvsourceadaptor(dba);

    sqls = ensDatabaseadaptorSqlstatementNew(dba, statement);

    sqli = ajSqlrowiterNew(sqls);

    while (!ajSqlrowiterDone(sqli))
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

        if (!gvsse)
            ajDebug("gvsourceadaptorFetchAllbyStatement could not "
                    "get Ensembl Genetic Variation Source Somatic "
                    "for string '%S'.\n", gvssomatic);

        gvste = ensGvsourceTypeFromStr(gvstype);

        if (!gvste)
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

        ajListPushAppend(gvss, (void *) gvs);

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
** member is already present in the adaptor cache, the Source is deleted and
** a pointer to the cached Source is returned.
**
** @param [u] gvsa [EnsPGvsourceadaptor] Ensembl Genetic Variation
**                                       Source Adaptor
** @param [u] Pgvs [EnsPGvsource*] Ensembl Genetic Variation Source address
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
**
** @release 6.3.0
** @@
******************************************************************************/

static AjBool gvsourceadaptorCacheInsert(EnsPGvsourceadaptor gvsa,
                                         EnsPGvsource *Pgvs)
{
    ajuint *Pidentifier = NULL;

    EnsPGvsource gvs1 = NULL;
    EnsPGvsource gvs2 = NULL;

    if (!gvsa)
        return ajFalse;

    if (!gvsa->CacheByIdentifier)
        return ajFalse;

    if (!gvsa->CacheByName)
        return ajFalse;

    if (!Pgvs)
        return ajFalse;

    if (!*Pgvs)
        return ajFalse;

    /* Search the identifier cache. */

    gvs1 = (EnsPGvsource) ajTableFetchmodV(
        gvsa->CacheByIdentifier,
        (const void *) &((*Pgvs)->Identifier));

    /* Search the name cache. */

    gvs2 = (EnsPGvsource) ajTableFetchmodS(gvsa->CacheByName, (*Pgvs)->Name);

    if ((!gvs1) && (!gvs2))
    {
        /* Insert into the identifier cache. */

        AJNEW0(Pidentifier);

        *Pidentifier = (*Pgvs)->Identifier;

        ajTablePut(gvsa->CacheByIdentifier,
                   (void *) Pidentifier,
                   (void *) ensGvsourceNewRef(*Pgvs));

        /* Insert into the name cache. */

        ajTablePut(gvsa->CacheByName,
                   (void *) ajStrNewS((*Pgvs)->Name),
                   (void *) ensGvsourceNewRef(*Pgvs));
    }

    if (gvs1 && gvs2 && (gvs1 == gvs2))
    {
        ajDebug("gvsourceadaptorCacheInsert replaced Ensembl Genetic "
                "Variation Source %p with one already cached %p.\n",
                *Pgvs, gvs1);

        ensGvsourceDel(Pgvs);

        ensGvsourceNewRef(gvs1);

        Pgvs = &gvs1;
    }

    if (gvs1 && gvs2 && (gvs1 != gvs2))
        ajDebug("gvsourceadaptorCacheInsert detected an Ensembl Genetic "
                "Variation Source in the identifier and name cache with "
                "identical names ('%S' and '%S') but different addresses "
                "(%p and %p).\n",
                gvs1->Name, gvs2->Name, gvs1, gvs2);

    if (gvs1 && (!gvs2))
        ajDebug("gvsourceadaptorCacheInsert detected an Ensembl Analysis "
                "in the identifier, but not in the name cache.\n");

    if ((!gvs1) && gvs2)
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
**
** @release 6.3.0
** @@
******************************************************************************/

static AjBool gvsourceadaptorCacheRemove(EnsPGvsourceadaptor gvsa,
                                         EnsPGvsource gvs)
{
    EnsPGvsource gvs1 = NULL;
    EnsPGvsource gvs2 = NULL;

    if (!gvsa)
        return ajFalse;

    if (!gvs)
        return ajFalse;

    gvs1 = (EnsPGvsource) ajTableRemove(
        gvsa->CacheByIdentifier,
        (const void *) &gvs->Identifier);

    gvs2 = (EnsPGvsource) ajTableRemove(
        gvsa->CacheByName,
        (const void *) gvs->Name);

    if (gvs1 && (!gvs2))
        ajWarn("gvsourceadaptorCacheRemove could remove Ensembl Genetic "
               "Variation Source with identifier %u and name '%S' only from "
               "the identifier cache.\n",
               gvs->Identifier,
               gvs->Name);

    if ((!gvs1) && gvs2)
        ajWarn("gvsourceadaptorCacheRemove could remove Ensembl Genetic "
               "Variation Source with identifier %u and name '%S' only from "
               "the name cache.\n",
               gvs->Identifier,
               gvs->Name);

    ensGvsourceDel(&gvs1);
    ensGvsourceDel(&gvs2);

    return ajTrue;
}

#endif /* AJFALSE */




/* @funcstatic gvsourceadaptorCacheInit ***************************************
**
** Initialise the internal Ensembl Genetic Variation Source cache of an
** Ensembl Genetic Variation Source Adaptor.
**
** @param [u] gvsa [EnsPGvsourceadaptor] Ensembl Genetic Variation
**                                       Source Adaptor
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
**
** @release 6.3.0
** @@
******************************************************************************/

static AjBool gvsourceadaptorCacheInit(EnsPGvsourceadaptor gvsa)
{
    AjPList gvss = NULL;
    AjPList mis  = NULL;

    AjPStr key = NULL;

    EnsPGvsource gvs = NULL;

    EnsPMetainformation mi = NULL;

    if (ajDebugTest("gvsourceadaptorCacheInit"))
        ajDebug("gvsourceadaptorCacheInit\n"
                "  gvsa %p\n",
                gvsa);

    if (!gvsa)
        return ajFalse;

    if (gvsa->CacheByIdentifier)
        return ajFalse;
    else
    {
        gvsa->CacheByIdentifier = ajTableuintNew(0U);

        ajTableSetDestroyvalue(
            gvsa->CacheByIdentifier,
            (void (*)(void **)) &ensGvsourceDel);
    }

    if (gvsa->CacheByName)
        return ajFalse;
    else
    {
        gvsa->CacheByName = ajTablestrNew(0U);

        ajTableSetDestroyvalue(
            gvsa->CacheByName,
            (void (*)(void **)) &ensGvsourceDel);
    }

    /* Get the default Ensembl Genetic Variation Source. */

    key = ajStrNewC("source.default_source");

    mis = ajListNew();

    ensMetainformationadaptorFetchAllbyKey(
        ensRegistryGetMetainformationadaptor(
            ensGvsourceadaptorGetDatabaseadaptor(gvsa)),
        key,
        mis);

    ajStrDel(&key);

    ajListPeekFirst(mis, (void **) &mi);

    /* Fetch all Ensembl Genetic Variation Source objects. */

    gvss = ajListNew();

    ensBaseadaptorFetchAllbyConstraint(
        ensGvsourceadaptorGetBaseadaptor(gvsa),
        (const AjPStr) NULL,
        (EnsPAssemblymapper) NULL,
        (EnsPSlice) NULL,
        gvss);

    while (ajListPop(gvss, (void **) &gvs))
    {
        if (ajStrMatchS(gvs->Name, ensMetainformationGetValue(mi)))
            gvsa->DefaultGvsource = ensGvsourceNewRef(gvs);

        gvsourceadaptorCacheInsert(gvsa, &gvs);

        /*
        ** Both caches hold internal references to the
        ** Ensembl Genetic Variation Source objects.
        */

        ensGvsourceDel(&gvs);
    }

    ajListFree(&gvss);

    while (ajListPop(mis, (void **) &mi))
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
** @valrule * [EnsPGvsourceadaptor]
** Ensembl Genetic Variation Source Adaptor or NULL
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
** @return [EnsPGvsourceadaptor]
** Ensembl Genetic Variation Source Adaptor or NULL
**
** @release 6.3.0
** @@
******************************************************************************/

EnsPGvsourceadaptor ensGvsourceadaptorNew(EnsPDatabaseadaptor dba)
{
    EnsPGvsourceadaptor gvsa = NULL;

    if (!dba)
        return NULL;

    if (ajDebugTest("ensGvsourceadaptorNew"))
        ajDebug("ensGvsourceadaptorNew\n"
                "  dba %p\n",
                dba);

    AJNEW0(gvsa);

    gvsa->Adaptor = ensBaseadaptorNew(
        dba,
        gvsourceadaptorKTablenames,
        gvsourceadaptorKColumnnames,
        (const EnsPBaseadaptorLeftjoin) NULL,
        (const char *) NULL,
        (const char *) NULL,
        &gvsourceadaptorFetchAllbyStatement);

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




/* @section destructors *******************************************************
**
** Destruction destroys all internal data structures and frees the memory
** allocated for an Ensembl Genetic Variation Source Adaptor object.
**
** @fdata [EnsPGvsourceadaptor]
**
** @nam3rule Del Destroy (free) an
** Ensembl Genetic Variation Source Adaptor
**
** @argrule * Pgvsa [EnsPGvsourceadaptor*]
** Ensembl Genetic Variation Source Adaptor address
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
** Ensembl Genetic Variation Source Adaptor address
**
** @return [void]
**
** @release 6.3.0
** @@
******************************************************************************/

void ensGvsourceadaptorDel(EnsPGvsourceadaptor *Pgvsa)
{
    EnsPGvsourceadaptor pthis = NULL;

    if (!Pgvsa)
        return;

#if defined(AJ_DEBUG) && AJ_DEBUG >= 1
    if (ajDebugTest("ensGvsourceadaptorDel"))
        ajDebug("ensGvsourceadaptorDel\n"
                "  *Pgvsa %p\n",
                *Pgvsa);
#endif /* defined(AJ_DEBUG) && AJ_DEBUG >= 1 */

    if (!(pthis = *Pgvsa))
        return;

    ensBaseadaptorDel(&pthis->Adaptor);

    ajTableDel(&pthis->CacheByIdentifier);
    ajTableDel(&pthis->CacheByName);

    ensGvsourceDel(&pthis->DefaultGvsource);

    ajMemFree((void **) Pgvsa);

    return;
}




/* @section member retrieval **************************************************
**
** Functions for returning members of an
** Ensembl Genetic Variation Source Adaptor object.
**
** @fdata [EnsPGvsourceadaptor]
**
** @nam3rule Get Return Ensembl Genetic Variation Source Adaptor attribute(s)
** @nam4rule Baseadaptor Return the Ensembl Base Adaptor
** @nam4rule Databaseadaptor Return the Ensembl Database Adaptor
**
** @argrule * gvsa [EnsPGvsourceadaptor]
** Ensembl Genetic Variation Source Adaptor
**
** @valrule Baseadaptor [EnsPBaseadaptor]
** Ensembl Base Adaptor or NULL
** @valrule Databaseadaptor [EnsPDatabaseadaptor]
** Ensembl Database Adaptor or NULL
**
** @fcategory use
******************************************************************************/




/* @func ensGvsourceadaptorGetBaseadaptor *************************************
**
** Get the Ensembl Base Adaptor member of an
** Ensembl Genetic Variation Source Adaptor.
**
** @param [u] gvsa [EnsPGvsourceadaptor]
** Ensembl Genetic Variation Source Adaptor
**
** @return [EnsPBaseadaptor] Ensembl Base Adaptor or NULL
**
** @release 6.3.0
** @@
******************************************************************************/

EnsPBaseadaptor ensGvsourceadaptorGetBaseadaptor(
    EnsPGvsourceadaptor gvsa)
{
    return (gvsa) ? gvsa->Adaptor : NULL;
}




/* @func ensGvsourceadaptorGetDatabaseadaptor *********************************
**
** Get the Ensembl Database Adaptor member of an
** Ensembl Genetic Variation Source Adaptor.
**
** @param [u] gvsa [EnsPGvsourceadaptor]
** Ensembl Genetic Variation Source Adaptor
**
** @return [EnsPDatabaseadaptor] Ensembl Database Adaptor or NULL
**
** @release 6.3.0
** @@
******************************************************************************/

EnsPDatabaseadaptor ensGvsourceadaptorGetDatabaseadaptor(
    EnsPGvsourceadaptor gvsa)
{
    return ensBaseadaptorGetDatabaseadaptor(
        ensGvsourceadaptorGetBaseadaptor(gvsa));
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
** @argrule ByIdentifier identifier [ajuint]
** SQL database-internal identifier
** @argrule ByIdentifier Pgvs [EnsPGvsource*]
** Ensembl Genetic Variation Source address
** @argrule ByName name [const AjPStr]
** Ensembl Genetic Variation Source name
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
** @param [u] Pvalue [void**] Ensembl Genetic Variation Source
**                           value data address
** @param [u] cl [void*] AJAX List of Ensembl Genetic Variation Source objects,
**                       passed in via ajTableMap
** @see ajTableMap
**
** @return [void]
**
** @release 6.3.0
** @@
******************************************************************************/

static void gvsourceadaptorFetchAll(const void *key,
                                    void **Pvalue,
                                    void *cl)
{
    if (!key)
        return;

    if (!Pvalue)
        return;

    if (!*Pvalue)
        return;

    if (!cl)
        return;

    ajListPushAppend((AjPList) cl,
                     (void *) ensGvsourceNewRef(*((EnsPGvsource *) Pvalue)));

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
**
** @release 6.3.0
** @@
******************************************************************************/

AjBool ensGvsourceadaptorFetchAll(
    EnsPGvsourceadaptor gvsa,
    AjPList gvss)
{
    if (!gvsa)
        return ajFalse;

    if (!gvss)
        return ajFalse;

    if (!gvsa->CacheByIdentifier)
        gvsourceadaptorCacheInit(gvsa);

    ajTableMap(gvsa->CacheByIdentifier,
               &gvsourceadaptorFetchAll,
               (void *) gvss);

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
**
** @release 6.4.0
** @@
******************************************************************************/

AjBool ensGvsourceadaptorFetchAllbyGvsourceType(
    EnsPGvsourceadaptor gvsa,
    EnsEGvsourceType gvst,
    AjPList gvss)
{
    register ajuint i = 0U;

    void **valarray = NULL;

    if (!gvsa)
        return ajFalse;

    if (!gvst)
        return ajFalse;

    if (!gvsa->CacheByIdentifier)
        gvsourceadaptorCacheInit(gvsa);

    ajTableToarrayValues(gvsa->CacheByIdentifier, &valarray);

    for (i = 0U; valarray[i]; i++)
        if (gvst == ensGvsourceGetType((EnsPGvsource) valarray[i]))
            ajListPushAppend(
                gvss,
                (void *) ensGvsourceNewRef((EnsPGvsource) valarray[i]));

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
**
** @release 6.3.0
** @@
******************************************************************************/

AjBool ensGvsourceadaptorFetchByIdentifier(
    EnsPGvsourceadaptor gvsa,
    ajuint identifier,
    EnsPGvsource *Pgvs)
{
    AjBool result = AJFALSE;

    if (!gvsa)
        return ajFalse;

    if (!identifier)
        return ajFalse;

    if (!Pgvs)
        return ajFalse;

    *Pgvs = NULL;

    /*
    ** Initially, search the identifier cache.
    ** For any object returned by the AJAX Table the reference counter needs
    ** to be incremented manually.
    */

    if (!gvsa->CacheByIdentifier)
        gvsourceadaptorCacheInit(gvsa);

    *Pgvs = (EnsPGvsource) ajTableFetchmodV(gvsa->CacheByIdentifier,
                                            (const void *) &identifier);

    if (*Pgvs)
    {
        ensGvsourceNewRef(*Pgvs);

        return ajTrue;
    }

    /* For a cache miss re-query the database. */

    result = ensBaseadaptorFetchByIdentifier(
        ensGvsourceadaptorGetBaseadaptor(gvsa),
        identifier,
        (void **) Pgvs);

    gvsourceadaptorCacheInsert(gvsa, Pgvs);

    return result;
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
**
** @release 6.3.0
** @@
******************************************************************************/

AjBool ensGvsourceadaptorFetchByName(
    EnsPGvsourceadaptor gvsa,
    const AjPStr name,
    EnsPGvsource *Pgvs)
{
    char *txtname = NULL;

    AjBool result = AJFALSE;

    AjPList gvss = NULL;

    AjPStr constraint = NULL;

    EnsPBaseadaptor ba = NULL;

    EnsPGvsource gvs = NULL;

    if (!gvsa)
        return ajFalse;

    if ((!name) && (!ajStrGetLen(name)))
        return ajFalse;

    if (!Pgvs)
        return ajFalse;

    *Pgvs = NULL;

    /*
    ** Initially, search the name cache.
    ** For any object returned by the AJAX Table the reference counter needs
    ** to be incremented manually.
    */

    if (!gvsa->CacheByName)
        gvsourceadaptorCacheInit(gvsa);

    *Pgvs = (EnsPGvsource) ajTableFetchmodS(gvsa->CacheByName, name);

    if (*Pgvs)
    {
        ensGvsourceNewRef(*Pgvs);

        return ajTrue;
    }

    /* In case of a cache miss, re-query the database. */

    ba = ensGvsourceadaptorGetBaseadaptor(gvsa);

    ensBaseadaptorEscapeC(ba, &txtname, name);

    constraint = ajFmtStr("source.name = '%s'", txtname);

    ajCharDel(&txtname);

    gvss = ajListNew();

    result = ensBaseadaptorFetchAllbyConstraint(
        ba,
        constraint,
        (EnsPAssemblymapper) NULL,
        (EnsPSlice) NULL,
        gvss);

    if (ajListGetLength(gvss) > 1)
        ajWarn("ensGvsourceadaptorFetchByName got more than one "
               "Ensembl Genetic Variation Source for (UNIQUE) name '%S'.\n",
               name);

    ajListPop(gvss, (void **) Pgvs);

    gvsourceadaptorCacheInsert(gvsa, Pgvs);

    while (ajListPop(gvss, (void **) &gvs))
    {
        gvsourceadaptorCacheInsert(gvsa, &gvs);

        ensGvsourceDel(&gvs);
    }

    ajListFree(&gvss);

    ajStrDel(&constraint);

    return result;
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
**
** @release 6.4.0
** @@
******************************************************************************/

AjBool ensGvsourceadaptorFetchDefault(
    EnsPGvsourceadaptor gvsa,
    EnsPGvsource *Pgvs)
{
    if (!gvsa)
        return ajFalse;

    if (!Pgvs)
        return ajFalse;

    *Pgvs = ensGvsourceNewRef(gvsa->DefaultGvsource);

    return ajTrue;
}
