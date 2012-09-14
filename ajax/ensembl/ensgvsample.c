/* @source ensgvsample ********************************************************
**
** Ensembl Genetic Variation Sample functions
**
** @author Copyright (C) 1999 Ensembl Developers
** @author Copyright (C) 2006 Michael K. Schuster
** @version $Revision: 1.50 $
** @modified 2009 by Alan Bleasby for incorporation into EMBOSS core
** @modified $Date: 2012/07/14 14:52:40 $ by $Author: rice $
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

#include "ensgvsample.h"




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

/* @conststatic gvsampleKDisplay **********************************************
**
** The Ensembl Genetic Variation Sample display member is enumerated in
** both, the SQL table definition and the data structure. The following
** strings are used for conversion in database operations and correspond to
** EnsEGvsampleDisplay.
**
******************************************************************************/

static const char *gvsampleKDisplay[] =
{
    "",
    "REFERENCE",
    "DEFAULT",
    "DISPLAYABLE",
    "UNDISPLAYABLE"
    "LD",
    "MARTDISPLAYABLE",
    (const char *) NULL
};




/* @conststatic gvsampleadaptorKTables ****************************************
**
** Array of Ensembl Genetic Variation Sample Adaptor SQL table names
**
******************************************************************************/

static const char *gvsampleadaptorKTables[] =
{
    "sample",
    (const char *) NULL
};




/* @conststatic gvsampleadaptorKColumns ***************************************
**
** Array of Ensembl Genetic Variation Sample Adaptor SQL column names
**
******************************************************************************/

static const char *gvsampleadaptorKColumns[] =
{
    "sample.sample_id",
    "sample.name",
    "sample.size",
    "sample.description",
    "sample.display",
    (const char *) NULL
};




/* ========================================================================= */
/* =========================== private variables =========================== */
/* ========================================================================= */




/* ========================================================================= */
/* =========================== private functions =========================== */
/* ========================================================================= */

static AjBool gvsampleadaptorFetchAllbyStatement(
    EnsPBaseadaptor ba,
    const AjPStr statement,
    EnsPAssemblymapper am,
    EnsPSlice slice,
    AjPList gvss);




/* ========================================================================= */
/* ======================= All functions by section ======================== */
/* ========================================================================= */




/* @filesection ensgvsample ***************************************************
**
** @nam1rule ens Function belongs to the Ensembl library
**
******************************************************************************/




/* @datasection [EnsPGvsample] Ensembl Genetic Variation Sample ***************
**
** @nam2rule Gvsample Functions for manipulating
** Ensembl Genetic Variation Sample objects
**
** @cc Bio::EnsEMBL::Variation::Sample
** @cc CVS Revision: 1.4
** @cc CVS Tag: branch-ensembl-66
**
******************************************************************************/




/* @section constructors ******************************************************
**
** All constructors return a new Ensembl Genetic Variation Sample by pointer.
** It is the responsibility of the user to first destroy any previous
** Genetic Variation Sample. The target pointer does not need to be initialised
** to NULL, but it is good programming practice to do so anyway.
**
** @fdata [EnsPGvsample]
**
** @nam3rule New Constructor
** @nam4rule Cpy Constructor with existing object
** @nam4rule Ini Constructor with initial values
** @nam4rule Ref Constructor by incrementing the reference counter
**
** @argrule Cpy gvs [const EnsPGvsample] Ensembl Genetic Variation Sample
** @argrule Ini gvsa [EnsPGvsampleadaptor] Ensembl Genetic Variation
** @argrule Ini identifier [ajuint] SQL database-internal identifier
** @argrule Ini name [AjPStr] Name
** @argrule Ini description [AjPStr] Description
** @argrule Ini display [EnsEGvsampleDisplay] Display
** @argrule Ini size [ajuint] Size
** @argrule Ref gvs [EnsPGvsample] Ensembl Genetic Variation Sample
**
** @valrule * [EnsPGvsample] Ensembl Genetic Variation Sample or NULL
**
** @fcategory new
******************************************************************************/




/* @func ensGvsampleNewCpy ****************************************************
**
** Object-based constructor function, which returns an independent object.
**
** @param [r] gvs [const EnsPGvsample] Ensembl Genetic Variation Sample
**
** @return [EnsPGvsample] Ensembl Genetic Variation Sample or NULL
**
** @release 6.4.0
** @@
******************************************************************************/

EnsPGvsample ensGvsampleNewCpy(const EnsPGvsample gvs)
{
    EnsPGvsample pthis = NULL;

    if (!gvs)
        return NULL;

    AJNEW0(pthis);

    pthis->Use        = 1U;
    pthis->Identifier = gvs->Identifier;
    pthis->Adaptor    = gvs->Adaptor;

    if (gvs->Name)
        pthis->Name = ajStrNewRef(gvs->Name);

    if (gvs->Description)
        pthis->Description = ajStrNewRef(gvs->Description);

    pthis->Display = gvs->Display;
    pthis->Size    = gvs->Size;

    return pthis;
}




/* @func ensGvsampleNewIni ****************************************************
**
** Constructor for an Ensembl Genetic Variation Sample with initial values.
**
** @cc Bio::EnsEMBL::Storable::new
** @param [u] gvsa [EnsPGvsampleadaptor]
** Ensembl Genetic Variation Sample Adaptor
** @param [r] identifier [ajuint] SQL database-internal identifier
** @cc Bio::EnsEMBL::Variation::Sample::new
** @param [u] name [AjPStr] Name
** @param [u] description [AjPStr] Description
** @param [u] display [EnsEGvsampleDisplay] Display
** @param [r] size [ajuint] Size
**
** @return [EnsPGvsample] Ensembl Genetic Variation Sample or NULL
**
** @release 6.4.0
** @@
******************************************************************************/

EnsPGvsample ensGvsampleNewIni(EnsPGvsampleadaptor gvsa,
                               ajuint identifier,
                               AjPStr name,
                               AjPStr description,
                               EnsEGvsampleDisplay display,
                               ajuint size)
{
    EnsPGvsample gvs = NULL;

    AJNEW0(gvs);

    gvs->Use        = 1U;
    gvs->Identifier = identifier;
    gvs->Adaptor    = gvsa;

    if (name)
        gvs->Name = ajStrNewRef(name);

    if (description)
        gvs->Description = ajStrNewRef(description);

    gvs->Display = display;
    gvs->Size    = size;

    return gvs;
}




/* @func ensGvsampleNewRef ****************************************************
**
** Ensembl Object referencing function, which returns a pointer to the
** Ensembl Object passed in and increases its reference count.
**
** @param [u] gvs [EnsPGvsample] Ensembl Genetic Variation Sample
**
** @return [EnsPGvsample] Ensembl Genetic Variation Sample or NULL
**
** @release 6.2.0
** @@
******************************************************************************/

EnsPGvsample ensGvsampleNewRef(EnsPGvsample gvs)
{
    if (!gvs)
        return NULL;

    gvs->Use++;

    return gvs;
}




/* @section destructors *******************************************************
**
** Destruction destroys all internal data structures and frees the memory
** allocated for an Ensembl Genetic Variation Sample object.
**
** @fdata [EnsPGvsample]
**
** @nam3rule Del Destroy (free) an Ensembl Genetic Variation Sample
**
** @argrule * Pgvs [EnsPGvsample*] Ensembl Genetic Variation Sample address
**
** @valrule * [void]
**
** @fcategory delete
******************************************************************************/




/* @func ensGvsampleDel *******************************************************
**
** Default destructor for an Ensembl Genetic Variation Sample.
**
** @param [d] Pgvs [EnsPGvsample*] Ensembl Genetic Variation Sample address
**
** @return [void]
**
** @release 6.2.0
** @@
******************************************************************************/

void ensGvsampleDel(EnsPGvsample *Pgvs)
{
    EnsPGvsample pthis = NULL;

    if (!Pgvs)
        return;

#if defined(AJ_DEBUG) && AJ_DEBUG >= 1
    if (ajDebugTest("ensGvsampleDel"))
    {
        ajDebug("ensGvsampleDel\n"
                "  *Psample %p\n",
                *Pgvs);

        ensGvsampleTrace(*Pgvs, 1);
    }
#endif /* defined(AJ_DEBUG) && AJ_DEBUG >= 1 */

    if (!*Pgvs)
        return;

    pthis = *Pgvs;

    pthis->Use--;

    if (pthis->Use)
    {
        *Pgvs = NULL;

        return;
    }

    ajStrDel(&pthis->Name);
    ajStrDel(&pthis->Description);

    AJFREE(pthis);

    *Pgvs = NULL;

    return;
}




/* @section member retrieval **************************************************
**
** Functions for returning members of an
** Ensembl Genetic Variation Sample object.
**
** @fdata [EnsPGvsample]
**
** @nam3rule Get Return Genetic Variation Sample attribute(s)
** @nam4rule Adaptor Return the Ensembl Genetic Variation Sample Adaptor
** @nam4rule Description Return the description
** @nam4rule Display Return the display
** @nam4rule Identifier Return the SQL database-internal identifier
** @nam4rule Name Return the name
** @nam4rule Size Return the size
**
** @argrule * gvs [const EnsPGvsample] Genetic Variation Sample
**
** @valrule Adaptor [EnsPGvsampleadaptor]
** Ensembl Genetic Variation Sample Adaptor or NULL
** @valrule Description [AjPStr] Description or NULL
** @valrule Display [EnsEGvsampleDisplay] Display or ensEGvsampleDisplayNULL
** @valrule Identifier [ajuint] SQL database-internal identifier or 0U
** @valrule Name [AjPStr] Name or NULL
** @valrule Size [ajuint] Size or 0U
**
** @fcategory use
******************************************************************************/




/* @func ensGvsampleGetAdaptor ************************************************
**
** Get the Ensembl Genetic Variation Sample Adaptor member of an
** Ensembl Genetic Variation Sample.
**
** @param [r] gvs [const EnsPGvsample] Ensembl Genetic Variation Sample
**
** @return [EnsPGvsampleadaptor] Ensembl Genetic Variation Sample Adaptor
** or NULL
**
** @release 6.2.0
** @@
******************************************************************************/

EnsPGvsampleadaptor ensGvsampleGetAdaptor(const EnsPGvsample gvs)
{
    return (gvs) ? gvs->Adaptor : NULL;
}




/* @func ensGvsampleGetDescription ********************************************
**
** Get the description member of an Ensembl Genetic Variation Sample.
**
** @param  [r] gvs [const EnsPGvsample] Ensembl Genetic Variation Sample
**
** @return [AjPStr] Description or NULL
**
** @release 6.2.0
** @@
******************************************************************************/

AjPStr ensGvsampleGetDescription(const EnsPGvsample gvs)
{
    return (gvs) ? gvs->Description : NULL;
}




/* @func ensGvsampleGetDisplay ************************************************
**
** Get the display member of an Ensembl Genetic Variation Sample.
**
** @param  [r] gvs [const EnsPGvsample] Ensembl Genetic Variation Sample
**
** @return [EnsEGvsampleDisplay] Display or ensEGvsampleDisplayNULL
**
** @release 6.2.0
** @@
******************************************************************************/

EnsEGvsampleDisplay ensGvsampleGetDisplay(const EnsPGvsample gvs)
{
    return (gvs) ? gvs->Display : ensEGvsampleDisplayNULL;
}




/* @func ensGvsampleGetIdentifier *********************************************
**
** Get the SQL database-internal identifier member of an
** Ensembl Genetic Variation Sample.
**
** @param [r] gvs [const EnsPGvsample] Ensembl Genetic Variation Sample
**
** @return [ajuint] SQL database-internal identifier or 0U
**
** @release 6.2.0
** @@
******************************************************************************/

ajuint ensGvsampleGetIdentifier(const EnsPGvsample gvs)
{
    return (gvs) ? gvs->Identifier : 0U;
}




/* @func ensGvsampleGetName ***************************************************
**
** Get the name member of an Ensembl Genetic Variation Sample.
**
** @param  [r] gvs [const EnsPGvsample] Ensembl Genetic Variation Sample
**
** @return [AjPStr] Name or NULL
**
** @release 6.2.0
** @@
******************************************************************************/

AjPStr ensGvsampleGetName(const EnsPGvsample gvs)
{
    return (gvs) ? gvs->Name : NULL;
}




/* @func ensGvsampleGetSize ***************************************************
**
** Get the size member of an Ensembl Genetic Variation Sample.
**
** @param  [r] gvs [const EnsPGvsample] Ensembl Genetic Variation Sample
**
** @return [ajuint] Size or 0U
**
** @release 6.2.0
** @@
******************************************************************************/

ajuint ensGvsampleGetSize(const EnsPGvsample gvs)
{
    return (gvs) ? gvs->Size : 0U;
}




/* @section member assignment *************************************************
**
** Functions for assigning members of an
** Ensembl Genetic Variation Sample object.
**
** @fdata [EnsPGvsample]
**
** @nam3rule Set Set one member of a Genetic Variation Sample
** @nam4rule Adaptor Set the Ensembl Genetic Variation Sample Adaptor
** @nam4rule Identifier Set the SQL database-internal identifier
** @nam4rule Name Set the name
** @nam4rule Description Set the description
** @nam4rule Display Set the display
** @nam4rule Size Set the size
**
** @argrule * gvs [EnsPGvsample] Ensembl Genetic Variation Sample object
** @argrule Adaptor gvsa [EnsPGvsampleadaptor] Ensembl Genetic Variation
** Sample Adaptor
** @argrule Description description [AjPStr] Description
** @argrule Display display [EnsEGvsampleDisplay] Display
** @argrule Identifier identifier [ajuint] SQL database-internal identifier
** @argrule Name name [AjPStr] Name
** @argrule Size size [ajuint] Size
**
** @valrule * [AjBool] ajTrue upon success, ajFalse otherwise
**
** @fcategory modify
******************************************************************************/




/* @func ensGvsampleSetAdaptor ************************************************
**
** Set the Ensembl Genetic Variation Sample Adaptor member of an
** Ensembl Genetic Variation Sample.
**
** @param [u] gvs [EnsPGvsample] Ensembl Genetic Variation Sample
** @param [u] gvsa [EnsPGvsampleadaptor] Ensembl Genetic Variation
**                                       Sample Adaptor
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
**
** @release 6.2.0
** @@
******************************************************************************/

AjBool ensGvsampleSetAdaptor(EnsPGvsample gvs, EnsPGvsampleadaptor gvsa)
{
    if (!gvs)
        return ajFalse;

    gvs->Adaptor = gvsa;

    return ajTrue;
}




/* @func ensGvsampleSetDescription ********************************************
**
** Set the description member of an Ensembl Genetic Variation Sample.
**
** @param [u] gvs [EnsPGvsample] Ensembl Genetic Variation Sample
** @param [u] description [AjPStr] Description
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
**
** @release 6.2.0
** @@
******************************************************************************/

AjBool ensGvsampleSetDescription(EnsPGvsample gvs, AjPStr description)
{
    if (!gvs)
        return ajFalse;

    ajStrDel(&gvs->Description);

    if (description)
        gvs->Description = ajStrNewRef(description);

    return ajTrue;
}




/* @func ensGvsampleSetDisplay ************************************************
**
** Set the display member of an Ensembl Genetic Variation Sample.
**
** @param [u] gvs [EnsPGvsample] Ensembl Genetic Variation Sample
** @param [u] display [EnsEGvsampleDisplay] Display
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
**
** @release 6.2.0
** @@
******************************************************************************/

AjBool ensGvsampleSetDisplay(EnsPGvsample gvs, EnsEGvsampleDisplay display)
{
    if (!gvs)
        return ajFalse;

    gvs->Display = display;

    return ajTrue;
}




/* @func ensGvsampleSetIdentifier *********************************************
**
** Set the SQL database-internal identifier member of an
** Ensembl Genetic Variation Sample.
**
** @param [u] gvs [EnsPGvsample] Ensembl Genetic Variation Sample
** @param [r] identifier [ajuint] SQL database-internal identifier
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
**
** @release 6.2.0
** @@
******************************************************************************/

AjBool ensGvsampleSetIdentifier(EnsPGvsample gvs, ajuint identifier)
{
    if (!gvs)
        return ajFalse;

    gvs->Identifier = identifier;

    return ajTrue;
}




/* @func ensGvsampleSetName ***************************************************
**
** Set the name member of an Ensembl Genetic Variation Sample.
**
** @param [u] gvs [EnsPGvsample] Ensembl Genetic Variation Sample
** @param [u] name [AjPStr] Name
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
**
** @release 6.2.0
** @@
******************************************************************************/

AjBool ensGvsampleSetName(EnsPGvsample gvs, AjPStr name)
{
    if (!gvs)
        return ajFalse;

    ajStrDel(&gvs->Name);

    if (name)
        gvs->Name = ajStrNewRef(name);

    return ajTrue;
}




/* @func ensGvsampleSetSize ***************************************************
**
** Set the size member of an Ensembl Genetic Variation Sample.
**
** @param [u] gvs [EnsPGvsample] Ensembl Genetic Variation Sample
** @param [r] size [ajuint] Size
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
**
** @release 6.2.0
** @@
******************************************************************************/

AjBool ensGvsampleSetSize(EnsPGvsample gvs, ajuint size)
{
    if (!gvs)
        return ajFalse;

    gvs->Size = size;

    return ajTrue;
}




/* @section debugging *********************************************************
**
** Functions for reporting of an Ensembl Genetic Variation Sample object.
**
** @fdata [EnsPGvsample]
**
** @nam3rule Trace Report Ensembl Genetic Variation Sample members to
**                 debug file
**
** @argrule Trace gvs [const EnsPGvsample] Ensembl Genetic Variation Sample
** @argrule Trace level [ajuint] Indentation level
**
** @valrule * [AjBool] ajTrue upon success, ajFalse otherwise
**
** @fcategory misc
******************************************************************************/




/* @func ensGvsampleTrace *****************************************************
**
** Trace an Ensembl Genetic Variation Sample.
**
** @param [r] gvs [const EnsPGvsample] Ensembl Genetic Variation Sample
** @param [r] level [ajuint] Indentation level
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
**
** @release 6.2.0
** @@
******************************************************************************/

AjBool ensGvsampleTrace(const EnsPGvsample gvs, ajuint level)
{
    AjPStr indent = NULL;

    if (!gvs)
        return ajFalse;

    indent = ajStrNew();

    ajStrAppendCountK(&indent, ' ', level * 2);

    ajDebug("%SensGvsampleTrace %p\n"
            "%S  Use %u\n"
            "%S  Identifier %u\n"
            "%S  Adaptor %p\n"
            "%S  Name '%S'\n"
            "%S  Description '%S'\n"
            "%S  Display '%s'\n"
            "%S  Size %u\n",
            indent, gvs,
            indent, gvs->Use,
            indent, gvs->Identifier,
            indent, gvs->Adaptor,
            indent, gvs->Name,
            indent, gvs->Description,
            indent, ensGvsampleDisplayToChar(gvs->Display),
            indent, gvs->Size);

    ajStrDel(&indent);

    return ajTrue;
}




/* @section calculate *********************************************************
**
** Functions for calculating values of an
** Ensembl Genetic Variation Sample object.
**
** @fdata [EnsPGvsample]
**
** @nam3rule Calculate Calculate Ensembl Genetic Variation Sample values
** @nam4rule Memsize Calculate the memory size in bytes
**
** @argrule * gvs [const EnsPGvsample] Ensembl Genetic Variation Sample
**
** @valrule Memsize [size_t] Memory size in bytes or 0
**
** @fcategory misc
******************************************************************************/




/* @func ensGvsampleCalculateMemsize ******************************************
**
** Get the memory size in bytes of an Ensembl Genetic Variation Sample.
**
** @param [r] gvs [const EnsPGvsample] Ensembl Genetic Variation Sample
**
** @return [size_t] Memory size in bytes or 0
**
** @release 6.4.0
** @@
******************************************************************************/

size_t ensGvsampleCalculateMemsize(const EnsPGvsample gvs)
{
    size_t size = 0;

    if (!gvs)
        return 0;

    size += sizeof (EnsOGvsample);

    if (gvs->Name)
    {
        size += sizeof (AjOStr);

        size += ajStrGetRes(gvs->Name);
    }

    if (gvs->Description)
    {
        size += sizeof (AjOStr);

        size += ajStrGetRes(gvs->Description);
    }

    return size;
}




/* @datasection [EnsEGvsampleDisplay] Ensembl Genetic Variation Sample Display
** enumeration
**
** @nam2rule Gvsample Functions for manipulating
** Ensembl Genetic Variation Sample objects
** @nam3rule GvsampleDisplay Functions for manipulating
** Ensembl Genetic Variation Sample Display enumerations
**
******************************************************************************/




/* @section Misc **************************************************************
**
** Functions for returning an Ensembl Genetic Variation Sample Display
** enumeration.
**
** @fdata [EnsEGvsampleDisplay]
**
** @nam4rule From Ensembl Genetic Variation Sample Display query
** @nam5rule Str  AJAX String object query
**
** @argrule  Str  display  [const AjPStr] Display string
**
** @valrule * [EnsEGvsampleDisplay] Ensembl Genetic Variation Sample Display
** enumeration or ensEGvsampleDisplayNULL
**
** @fcategory misc
******************************************************************************/




/* @func ensGvsampleDisplayFromStr ********************************************
**
** Convert an AJAX String into an Ensembl Genetic Variation Sample Display
** enumeration.
**
** @param [r] display [const AjPStr] Display string
**
** @return [EnsEGvsampleDisplay] Ensembl Genetic Variation Sample Display
** enumeration or ensEGvsampleDisplayNULL
**
** @release 6.2.0
** @@
******************************************************************************/

EnsEGvsampleDisplay ensGvsampleDisplayFromStr(const AjPStr display)
{
    register EnsEGvsampleDisplay i = ensEGvsampleDisplayNULL;

    EnsEGvsampleDisplay edisplay = ensEGvsampleDisplayNULL;

    for (i = ensEGvsampleDisplayNULL;
         gvsampleKDisplay[i];
         i++)
        if (ajStrMatchC(display, gvsampleKDisplay[i]))
            edisplay = i;

    if (!edisplay)
        ajDebug("ensGvsampleDisplayFromStr encountered "
                "unexpected string '%S'.\n", display);

    return edisplay;
}




/* @section Cast **************************************************************
**
** Functions for returning attributes of an
** Ensembl Genetic Variation Sample Display enumeration.
**
** @fdata [EnsEGvsampleDisplay]
**
** @nam4rule To   Return Ensembl Genetic Variation Sample Display enumeration
** @nam5rule Char Return C character string value
**
** @argrule To gvsd [EnsEGvsampleDisplay]
** Ensembl Genetic Variation Sample Display enumeration
**
** @valrule Char [const char*]
** Ensembl Genetic Variation Sample Display C-type (char *) string
**
** @fcategory cast
******************************************************************************/




/* @func ensGvsampleDisplayToChar *********************************************
**
** Convert an Ensembl Genetic Variation Sample Display enumeration into a
** C-type (char *) string.
**
** @param [u] gvsd [EnsEGvsampleDisplay]
** Ensembl Genetic Variation Sample Display enumeration
**
** @return [const char*]
** Ensembl Genetic Variation Sample Display C-type (char *) string
**
** @release 6.2.0
** @@
******************************************************************************/

const char* ensGvsampleDisplayToChar(EnsEGvsampleDisplay gvsd)
{
    register EnsEGvsampleDisplay i = ensEGvsampleDisplayNULL;

    for (i = ensEGvsampleDisplayNULL;
         gvsampleKDisplay[i] && (i < gvsd);
         i++);

    if (!gvsampleKDisplay[i])
        ajDebug("ensGvsampleDisplayToChar encountered an "
                "out of boundary error on "
                "Ensembl Genetic Variation Sample Display enumeration %d.\n",
                gvsd);

    return gvsampleKDisplay[i];
}




/* @datasection [EnsPGvsampleadaptor] Ensembl Genetic Variation Sample Adaptor
**
** @nam2rule Gvsampleadaptor Functions for manipulating
** Ensembl Genetic Variation Sample Adaptor objects
**
** @cc Bio::EnsEMBL::Variation::DBSQL::SampleAdaptor
** @cc CVS Revision: 1.10
** @cc CVS Tag: branch-ensembl-66
**
******************************************************************************/




/* @funcstatic gvsampleadaptorFetchAllbyStatement *****************************
**
** Fetch all Ensembl Genetic Variation Sample objects via an SQL statement.
**
** @param [u] ba [EnsPBaseadaptor] Ensembl Base Adaptor
** @param [r] statement [const AjPStr] SQL statement
** @param [uN] am [EnsPAssemblymapper] Ensembl Assembly Mapper
** @param [uN] slice [EnsPSlice] Ensembl Slice
** @param [u] gvss [AjPList] AJAX List of Ensembl Genetic Variation Sample
**                           objects
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
**
** @release 6.4.0
** @@
******************************************************************************/

static AjBool gvsampleadaptorFetchAllbyStatement(
    EnsPBaseadaptor ba,
    const AjPStr statement,
    EnsPAssemblymapper am,
    EnsPSlice slice,
    AjPList gvss)
{
    ajuint identifier = 0U;
    ajuint size       = 0U;

    EnsEGvsampleDisplay edisplay = ensEGvsampleDisplayNULL;

    AjPSqlstatement sqls = NULL;
    AjISqlrow sqli       = NULL;
    AjPSqlrow sqlr       = NULL;

    AjPStr name        = NULL;
    AjPStr description = NULL;
    AjPStr display     = NULL;

    EnsPDatabaseadaptor dba = NULL;

    EnsPGvsample gvs         = NULL;
    EnsPGvsampleadaptor gvsa = NULL;

    if (ajDebugTest("gvsampleadaptorFetchAllbyStatement"))
        ajDebug("gvsampleadaptorFetchAllbyStatement\n"
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

    gvsa = ensRegistryGetGvsampleadaptor(dba);

    sqls = ensDatabaseadaptorSqlstatementNew(dba, statement);

    sqli = ajSqlrowiterNew(sqls);

    while (!ajSqlrowiterDone(sqli))
    {
        identifier  = 0;
        name        = ajStrNew();
        size        = 0;
        description = ajStrNew();
        display     = ajStrNew();

        sqlr = ajSqlrowiterGet(sqli);

        ajSqlcolumnToUint(sqlr, &identifier);
        ajSqlcolumnToStr(sqlr, &name);
        ajSqlcolumnToUint(sqlr, &size);
        ajSqlcolumnToStr(sqlr, &description);
        ajSqlcolumnToStr(sqlr, &display);

        edisplay = ensGvsampleDisplayFromStr(display);

        gvs = ensGvsampleNewIni(gvsa,
                                identifier,
                                name,
                                description,
                                edisplay,
                                size);

        ajListPushAppend(gvss, (void *) gvs);

        ajStrDel(&name);
        ajStrDel(&description);
        ajStrDel(&display);
    }

    ajSqlrowiterDel(&sqli);

    ensDatabaseadaptorSqlstatementDel(dba, &sqls);

    return ajTrue;
}




/* @section constructors ******************************************************
**
** All constructors return a new Ensembl Genetic Variation Sample Adaptor
** by pointer.
** It is the responsibility of the user to first destroy any previous
** Ensembl Genetic Variation Sample Adaptor. The target pointer does not
** need to be initialised to NULL, but it is good programming practice to do
** so anyway.
**
** @fdata [EnsPGvsampleadaptor]
**
** @nam3rule New Constructor
**
** @argrule New dba [EnsPDatabaseadaptor] Ensembl Database Adaptor
**
** @valrule * [EnsPGvsampleadaptor]
** Ensembl Genetic Variation Sample Adaptor or NULL
**
** @fcategory new
******************************************************************************/




/* @func ensGvsampleadaptorNew ************************************************
**
** Default constructor for an Ensembl Genetic Variation Sample Adaptor.
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
** @see ensRegistryGetGvsampleadaptor
**
** @cc Bio::EnsEMBL::DBSQL::Baseadaptor::new
** @param [u] dba [EnsPDatabaseadaptor] Ensembl Database Adaptor
**
** @return [EnsPGvsampleadaptor]
** Ensembl Genetic Variation Sample Adaptor or NULL
**
** @release 6.2.0
** @@
******************************************************************************/

EnsPGvsampleadaptor ensGvsampleadaptorNew(
    EnsPDatabaseadaptor dba)
{
    return ensBaseadaptorNew(
        dba,
        gvsampleadaptorKTables,
        gvsampleadaptorKColumns,
        (const EnsPBaseadaptorLeftjoin) NULL,
        (const char *) NULL,
        (const char *) NULL,
        &gvsampleadaptorFetchAllbyStatement);
}




/* @section destructors *******************************************************
**
** Destruction destroys all internal data structures and frees the memory
** allocated for an Ensembl Genetic Variation Sample Adaptor object.
**
** @fdata [EnsPGvsampleadaptor]
**
** @nam3rule Del Destroy (free) an
** Ensembl Genetic Variation Sample Adaptor
**
** @argrule * Pgvsa [EnsPGvsampleadaptor*]
** Ensembl Genetic Variation Sample Adaptor address
**
** @valrule * [void]
**
** @fcategory delete
******************************************************************************/




/* @func ensGvsampleadaptorDel ************************************************
**
** Default destructor for an Ensembl Genetic Variation Sample Adaptor.
**
** Ensembl Object Adaptors are singleton objects that are registered in the
** Ensembl Registry and weakly referenced by Ensembl Objects that have been
** instantiated by it. Therefore, Ensembl Object Adaptors should never be
** destroyed directly. Upon exit, the Ensembl Registry will call this function
** if required.
**
** @param [d] Pgvsa [EnsPGvsampleadaptor*]
** Ensembl Genetic Variation Sample Adaptor address
**
** @return [void]
**
** @release 6.2.0
** @@
******************************************************************************/

void ensGvsampleadaptorDel(EnsPGvsampleadaptor *Pgvsa)
{
    ensBaseadaptorDel(Pgvsa);

	return;
}




/* @section member retrieval **************************************************
**
** Functions for returning members of an
** Ensembl Genetic Variation Sample Adaptor object.
**
** @fdata [EnsPGvsampleadaptor]
**
** @nam3rule Get Return Genetic Variation Sample Adaptor attribute(s)
** @nam4rule Baseadaptor Return the Ensembl Base Adaptor
** @nam4rule Databaseadaptor Return the Ensembl Database Adaptor
**
** @argrule * gvsa [EnsPGvsampleadaptor] Genetic Variation Sample Adaptor
**
** @valrule Baseadaptor [EnsPBaseadaptor] Ensembl Base Adaptor or NULL
** @valrule Databaseadaptor [EnsPDatabaseadaptor] Ensembl Database Adaptor
**                                                or NULL
**
** @fcategory use
******************************************************************************/




/* @func ensGvsampleadaptorGetBaseadaptor *************************************
**
** Get the Ensembl Base Adaptor member of an
** Ensembl Genetic Variation Sample Adaptor.
**
** @param [u] gvsa [EnsPGvsampleadaptor] Ensembl Genetic Variation
**                                       Sample Adaptor
**
** @return [EnsPBaseadaptor] Ensembl Base Adaptor or NULL
**
** @release 6.4.0
** @@
******************************************************************************/

EnsPBaseadaptor ensGvsampleadaptorGetBaseadaptor(
    EnsPGvsampleadaptor gvsa)
{
    return gvsa;
}




/* @func ensGvsampleadaptorGetDatabaseadaptor *********************************
**
** Get the Ensembl Database Adaptor member of an
** Ensembl Genetic Variation Sample Adaptor.
**
** @param [u] gvsa [EnsPGvsampleadaptor] Ensembl Genetic Variation
**                                       Sample Adaptor
**
** @return [EnsPDatabaseadaptor] Ensembl Database Adaptor or NULL
**
** @release 6.4.0
** @@
******************************************************************************/

EnsPDatabaseadaptor ensGvsampleadaptorGetDatabaseadaptor(
    EnsPGvsampleadaptor gvsa)
{
    return ensBaseadaptorGetDatabaseadaptor(gvsa);
}




/* @section object retrieval **************************************************
**
** Functions for fetching Ensembl Genetic Variation Sample objects from an
** Ensembl SQL database.
**
** @fdata [EnsPGvsampleadaptor]
**
** @nam3rule Fetch Fetch Ensembl Genetic Variation Sample object(s)
** @nam4rule All Fetch all Ensembl Genetic Variation Sample objects
** @nam4rule Allby Fetch all Ensembl Genetic Variation Sample objects
**                 matching a criterion
** @nam5rule Display Fetch all by an Ensembl Genetic Variation Display
**                   enumeration
** @nam5rule Identifiers Fetch all by an AJAX Table
** @nam4rule By Fetch one Ensembl Genetic Variation Sample object
**              matching a criterion
** @nam5rule Identifier Fetch ba a SQL database-internal identifier
**
** @argrule * gvsa [EnsPGvsampleadaptor]
** Ensembl Genetic Variation Sample Adaptor
** @argrule All gvss [AjPList]
** AJAX List of Ensembl Genetic Variation Sample objects
** @argrule AllbyDisplay gvsd [EnsEGvsampleDisplay]
** Ensembl Genetic Variation Sample Display enumeration
** @argrule AllbyDisplay gvss [AjPList] AJAX List of
** Ensembl Genetic Variation Sample objects
** @argrule AllbyIdentifiers gvss [AjPTable] AJAX Table of
** Ensembl Genetic Variation Sample objects
** @argrule ByIdentifier identifier [ajuint]
** SQL database-internal identifier
** @argrule ByIdentifier Pgvs [EnsPGvsample*]
** Ensembl Genetic Variation Sample object address
**
** @valrule * [AjBool] ajTrue upon success, ajFalse otherwise
**
** @fcategory use
******************************************************************************/




/* @func ensGvsampleadaptorFetchAllbyDisplay **********************************
**
** Fetch all Ensembl Genetic Variation Sample objects by an
** Ensembl Genetic Variation Sample Display enumeration.
**
** @param [u] gvsa [EnsPGvsampleadaptor]
** Ensembl Genetic Variation Sample Adaptor
** @param [u] gvsd [EnsEGvsampleDisplay]
** Ensembl Genetic Variation Sample Display enumeration
** @param [u] gvss [AjPList] AJAX List of
** Ensembl Genetic Variation Sample objects
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
**
** @release 6.4.0
** @@
******************************************************************************/

AjBool ensGvsampleadaptorFetchAllbyDisplay(
    EnsPGvsampleadaptor gvsa,
    EnsEGvsampleDisplay gvsd,
    AjPList gvss)
{
    AjBool result = AJFALSE;

    AjPStr constraint = NULL;

    if (!gvsa)
        return ajFalse;

    if (!gvss)
        return ajFalse;

    constraint = ajFmtStr(
        "sample.display = '%s'",
        ensGvsampleDisplayToChar(gvsd));

    result = ensBaseadaptorFetchAllbyConstraint(
        gvsa,
        constraint,
        (EnsPAssemblymapper) NULL,
        (EnsPSlice) NULL,
        gvss);

    ajStrDel(&constraint);

    return result;
}




/* @func ensGvsampleadaptorFetchAllbyIdentifiers ******************************
**
** Fetch all Ensembl Genetic Variation Sample objects by an
** AJAX unsigned integer key data and assign them as value data.
**
** The caller is responsible for deleting the Ensembl Genetic Variation
** Sample value data before deleting the AJAX Table.
**
** @cc Bio::EnsEMBL::Variation::DBSQL::Sampleadaptor::fetch_all_by_dbID_list
** @param [u] gvsa [EnsPGvsampleadaptor] Ensembl Genetic Variation
**                                       Sample Adaptor
** @param [u] gvss [AjPTable] AJAX Table of AJAX unsigned integer identifier
**                            key data and Ensembl Genetic Variation
**                            Sample value data
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
**
** @release 6.4.0
** @@
******************************************************************************/

AjBool ensGvsampleadaptorFetchAllbyIdentifiers(
    EnsPGvsampleadaptor gvsa,
    AjPTable gvss)
{
    return ensBaseadaptorFetchAllbyIdentifiers(
        gvsa,
        (EnsPSlice) NULL,
        (ajuint (*)(const void *)) &ensGvsampleGetIdentifier,
        gvss);
}




/* @func ensGvsampleadaptorFetchByIdentifier **********************************
**
** Fetch an Ensembl Genetic Variation Sample by its SQL database-internal
** identifier.
**
** @cc Bio::EnsEMBL::Variation::DBSQL::SampleAdaptor::fetch_by_dbID
** @param [u] gvsa [EnsPGvsampleadaptor] Ensembl Genetic Variation
**                                       Sample Adaptor
** @param [r] identifier [ajuint] SQL database-internal identifier
** @param [wP] Pgvs [EnsPGvsample*] Ensembl Genetic Variation Sample address
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
**
** @release 6.2.0
** @@
******************************************************************************/

AjBool ensGvsampleadaptorFetchByIdentifier(
    EnsPGvsampleadaptor gvsa,
    ajuint identifier,
    EnsPGvsample *Pgvs)
{
    if (!gvsa)
        return ajFalse;

    if (!Pgvs)
        return ajFalse;

    return ensBaseadaptorFetchByIdentifier(gvsa, identifier, (void **) Pgvs);
}




/* @section object retrieval **************************************************
**
** Functions for retrieving Ensembl Genetic Variation Sample-releated objects
** from an Ensembl SQL database.
**
** @fdata [EnsPGvsampleadaptor]
**
** @nam3rule Retrieve Retrieve Ensembl Genetic Variation Sample-related
** object(s)
** @nam4rule All Retrieve all releated objects
** @nam5rule Identifiers Retrieve all AJAX unsigned integer identifiers
** @nam5rule Synonyms Retrieve all synonyms
** @nam6rule By Retrieve by
** @nam7rule Identifier
** @nam7rule Synonym
**
** @argrule * gvsa [EnsPGvsampleadaptor] Ensembl Genetic Variation
** Sample Adaptor
** @argrule AllIdentifiersBySynonym synonym [const AjPStr] Ensembl Genetic
** Variation Sample synonym
** @argrule AllIdentifiersBySynonym source [const AjPStr] Source
** @argrule AllIdentifiersBySynonym idlist [AjPList] AJAX List of Ensembl
** Genetic Variation Sample synonyms
** @argrule AllSynonymsByIdentifier identifier [ajuint] Ensembl Genetic
** Variation Sample identifier
** @argrule AllSynonymsByIdentifier source [const AjPStr] Source
** @argrule AllSynonymsByIdentifier synonyms [AjPList]
** AJAX List of (synonym) AJAX String objects
**
** @valrule * [AjBool] ajTrue upon success, ajFalse otherwise
**
** @fcategory use
******************************************************************************/




/* @func ensGvsampleadaptorRetrieveAllIdentifiersBySynonym ********************
**
** Fetch all Ensembl Genetic Variation Sample identifiers for an
** Ensembl Genetic Variation Sample synonym.
**
** @cc Bio::EnsEMBL::Variation::DBSQL::SampleAdaptor::fetch_sample_by_synonym
** @param [u] gvsa [EnsPGvsampleadaptor] Ensembl Genetic Variation
**                                       Sample Adaptor
** @param [r] synonym [const AjPStr] Ensembl Genetic Variation Sample synonym
** @param [rN] source [const AjPStr] Source
** @param [u] idlist [AjPList] AJAX List of Ensembl Genetic Variation
**                             Sample identifiers
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
**
** @release 6.4.0
** @@
******************************************************************************/

AjBool ensGvsampleadaptorRetrieveAllIdentifiersBySynonym(
    EnsPGvsampleadaptor gvsa,
    const AjPStr synonym,
    const AjPStr source,
    AjPList idlist)
{
    char *txtsynonym = NULL;
    char *txtsource  = NULL;

    ajuint *Pidentifier = NULL;

    AjPSqlstatement sqls = NULL;
    AjISqlrow sqli       = NULL;
    AjPSqlrow sqlr       = NULL;

    AjPStr statement = NULL;

    EnsPDatabaseadaptor dba = NULL;

    if (!gvsa)
        return ajFalse;

    if (!synonym)
        return ajFalse;

    if (!idlist)
        return ajFalse;

    dba = ensBaseadaptorGetDatabaseadaptor(gvsa);

    if (!dba)
        return ajFalse;

    ensDatabaseadaptorEscapeC(dba, &txtsynonym, synonym);

    if (source)
    {
        ensDatabaseadaptorEscapeC(dba, &txtsource, source);

        statement = ajFmtStr(
            "SELECT "
            "sample_synonym.sample_id "
            "FROM "
            "sample_synonym, "
            "source "
            "WHERE "
            "sample_synonym.name = '%s' "
            "AND "
            "sample_synonym.source_id = source.source_id "
            "AND "
            "source.name = '%s'",
            txtsynonym,
            txtsource);

        ajCharDel(&txtsource);
    }
    else
        statement = ajFmtStr(
            "SELECT "
            "sample_synonym.sample_id "
            "FROM "
            "sample_synonym "
            "WHERE "
            "sample_synonym.name = '%s'",
            txtsynonym);

    ajCharDel(&txtsynonym);

    sqls = ensDatabaseadaptorSqlstatementNew(dba, statement);

    sqli = ajSqlrowiterNew(sqls);

    while (!ajSqlrowiterDone(sqli))
    {
        AJNEW0(Pidentifier);

        sqlr = ajSqlrowiterGet(sqli);

        ajSqlcolumnToUint(sqlr, Pidentifier);

        ajListPushAppend(idlist, (void *) Pidentifier);
    }

    ajSqlrowiterDel(&sqli);

    ensDatabaseadaptorSqlstatementDel(dba, &sqls);

    ajStrDel(&statement);

    return ajTrue;
}




/* @func ensGvsampleadaptorRetrieveAllSynonymsByIdentifier ********************
**
** Fetch all Ensembl Genetic Variation Sample synonyms for an
** Ensembl Genetic Variation Sample identifier.
**
** @cc Bio::EnsEMBL::Variation::DBSQL::SampleAdaptor::fetch_synonyms
** @param [u] gvsa [EnsPGvsampleadaptor] Ensembl Genetic Variation
**                                       Sample Adaptor
** @param [r] identifier [ajuint] Ensembl Genetic Variation Sample identifier
** @param [rN] source [const AjPStr] Source
** @param [u] synonyms [AjPList] AJAX List of (synonym) AJAX String objects
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
**
** @release 6.4.0
** @@
******************************************************************************/

AjBool ensGvsampleadaptorRetrieveAllSynonymsByIdentifier(
    EnsPGvsampleadaptor gvsa,
    ajuint identifier,
    const AjPStr source,
    AjPList synonyms)
{
    char *txtsource = NULL;

    AjPSqlstatement sqls = NULL;
    AjISqlrow sqli       = NULL;
    AjPSqlrow sqlr       = NULL;

    AjPStr name      = NULL;
    AjPStr statement = NULL;

    EnsPDatabaseadaptor dba = NULL;

    if (!gvsa)
        return ajFalse;

    if (!identifier)
        return ajFalse;

    if (!synonyms)
        return ajFalse;

    dba = ensBaseadaptorGetDatabaseadaptor(gvsa);

    if (!dba)
        return ajFalse;

    if (source)
    {
        ensDatabaseadaptorEscapeC(dba, &txtsource, source);

        statement = ajFmtStr(
            "SELECT "
            "sample_synonym.name "
            "FROM "
            "sample_synonym, "
            "source "
            "WHERE "
            "sample_synonym.sample_id = %u "
            "AND "
            "sample_synonym.source_id = source.source_id "
            "AND "
            "source.name = '%s'",
            identifier,
            txtsource);

        ajCharDel(&txtsource);
    }
    else
        statement = ajFmtStr(
            "SELECT "
            "sample_synonym.name "
            "FROM "
            "sample_synonym "
            "WHERE "
            "sample_synonym.sample_id = %u",
            identifier);

    sqls = ensDatabaseadaptorSqlstatementNew(dba, statement);

    sqli = ajSqlrowiterNew(sqls);

    while (!ajSqlrowiterDone(sqli))
    {
        name = ajStrNew();

        sqlr = ajSqlrowiterGet(sqli);

        ajSqlcolumnToStr(sqlr, &name);

        ajListPushAppend(synonyms, (void *) ajStrNewRef(name));

        ajStrDel(&name);
    }

    ajSqlrowiterDel(&sqli);

    ensDatabaseadaptorSqlstatementDel(dba, &sqls);

    ajStrDel(&statement);

    return ajTrue;
}
