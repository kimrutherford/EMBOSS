/* @source ensmiscellaneous ***************************************************
**
** Ensembl Miscellaneous functions
**
** @author Copyright (C) 1999 Ensembl Developers
** @author Copyright (C) 2006 Michael K. Schuster
** @version $Revision: 1.61 $
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

#include "ensmiscellaneous.h"
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

/* @conststatic miscellaneoussetadaptorKTables ********************************
**
** Array of Ensembl Miscellaneous Set Adaptor SQL table names
**
******************************************************************************/

static const char *const miscellaneoussetadaptorKTables[] =
{
    "misc_set",
    (const char *) NULL
};




/* @conststatic miscellaneoussetadaptorKColumns *******************************
**
** Array of Ensembl Miscellaneous Set Adaptor SQL column names
**
******************************************************************************/

static const char *const miscellaneoussetadaptorKColumns[] =
{
    "misc_set.misc_set_id",
    "misc_set.code",
    "misc_set.name",
    "misc_set.description",
    "misc_set.max_length",
    (const char *) NULL
};




/* @conststatic miscellaneousfeatureadaptorKTables ****************************
**
** Array of Ensembl Miscellaneous Feature Adaptor SQL table names
**
******************************************************************************/

static const char *const miscellaneousfeatureadaptorKTables[] =
{
    "misc_feature",
    "misc_feature_misc_set",
    "misc_attrib",
    (const char *) NULL
};




/* @conststatic miscellaneousfeatureadaptorKColumns ***************************
**
** Array of Ensembl Miscellaneous Feature Adaptor SQL column names
**
******************************************************************************/

static const char *const miscellaneousfeatureadaptorKColumns[] =
{
    "misc_feature.misc_feature_id",
    "misc_feature.seq_region_id",
    "misc_feature.seq_region_start",
    "misc_feature.seq_region_end",
    "misc_feature.seq_region_strand",
    "misc_attrib.attrib_type_id",
    "misc_attrib.value",
    "misc_feature_misc_set.misc_set_id",
    (const char *) NULL
};




/* @conststatic miscellaneousfeatureadaptorKLeftjoin **************************
**
** Array of Ensembl Miscellaneous Feature Adaptor SQL left join conditions
**
******************************************************************************/

static const EnsOBaseadaptorLeftjoin miscellaneousfeatureadaptorKLeftjoin[] =
{
    {
        "misc_feature_misc_set",
        "misc_feature.misc_feature_id = misc_feature_misc_set.misc_feature_id"
    },
    {
        "misc_attrib",
        "misc_feature.misc_feature_id = misc_attrib.misc_feature_id"
    },
    {NULL, NULL}
};




/* @conststatic miscellaneousfeatureadaptorKFinalcondition ********************
**
** Ensembl Miscellaneous Feature Adaptor SQL final conditions
**
******************************************************************************/

static const char *miscellaneousfeatureadaptorKFinalcondition =
    " ORDER BY misc_feature.misc_feature_id";




/* ========================================================================= */
/* =========================== private variables =========================== */
/* ========================================================================= */




/* ========================================================================= */
/* =========================== private functions =========================== */
/* ========================================================================= */

static int listMiscellaneoussetCompareIdentifierAscending(
    const void *item1,
    const void *item2);

static int listMiscellaneoussetCompareIdentifierDescending(
    const void *item1,
    const void *item2);

static AjBool miscellaneoussetadaptorFetchAllbyStatement(
    EnsPBaseadaptor ba,
    const AjPStr statement,
    EnsPAssemblymapper am,
    EnsPSlice slice,
    AjPList mss);

static AjBool miscellaneoussetadaptorCacheInit(
    EnsPMiscellaneoussetadaptor msa);

static AjBool miscellaneoussetadaptorCacheInsert(
    EnsPMiscellaneoussetadaptor msa,
    EnsPMiscellaneousset *Pms);

#if AJFALSE
static AjBool miscellaneoussetadaptorCacheRemove(
    EnsPMiscellaneoussetadaptor msa,
    EnsPMiscellaneousset ms);
#endif /* AJFALSE */

static void miscellaneoussetadaptorFetchAll(const void *key,
                                            void **Pvalue,
                                            void *cl);

static int listMiscellaneousfeatureCompareEndAscending(
    const void *item1,
    const void *item2);

static int listMiscellaneousfeatureCompareEndDescending(
    const void *item1,
    const void *item2);

static int listMiscellaneousfeatureCompareIdentifierAscending(
    const void *item1,
    const void *item2);

static int listMiscellaneousfeatureCompareStartAscending(
    const void *item1,
    const void *item2);

static int listMiscellaneousfeatureCompareStartDescending(
    const void *item1,
    const void *item2);

static AjBool miscellaneousfeatureadaptorHasAttribute(AjPTable attributes,
                                                      ajuint atid,
                                                      const AjPStr value);

static AjBool miscellaneousfeatureadaptorHasMiscellaneousset(AjPTable sets,
                                                             ajuint msid);

static AjBool miscellaneousfeatureadaptorFetchAllbyStatement(
    EnsPBaseadaptor ba,
    const AjPStr statement,
    EnsPAssemblymapper am,
    EnsPSlice slice,
    AjPList mfs);




/* ========================================================================= */
/* ======================= All functions by section ======================== */
/* ========================================================================= */




/* @filesection ensmiscellaneous **********************************************
**
** @nam1rule ens Function belongs to the Ensembl library
**
******************************************************************************/




/* @datasection [EnsPMiscellaneousset] Ensembl Miscellaneous Set **************
**
** @nam2rule Miscellaneousset Functions for manipulating
** Ensembl Miscellaneous Set objects
**
** @cc Bio::EnsEMBL::MiscSet
** @cc CVS Revision: 1.10
** @cc CVS Tag: branch-ensembl-66
**
******************************************************************************/




/* @section constructors ******************************************************
**
** All constructors return a new Ensembl Miscellaneous Set by pointer.
** It is the responsibility of the user to first destroy any previous
** Miscellaneous Set. The target pointer does not need to be initialised to
** NULL, but it is good programming practice to do so anyway.
**
** @fdata [EnsPMiscellaneousset]
**
** @nam3rule New Constructor
** @nam4rule Cpy Constructor with existing object
** @nam4rule Ini Constructor with initial values
** @nam4rule Ref Constructor by incrementing the reference counter
**
** @argrule Cpy ms [const EnsPMiscellaneousset] Ensembl Miscellaneous Set
** @argrule Ini msa [EnsPMiscellaneoussetadaptor] Ensembl Miscellaneous
** @argrule Ini identifier [ajuint] SQL database-internal identifier
** @argrule Ini code [AjPStr] Code
** @argrule Ini name [AjPStr] Name
** @argrule Ini description [AjPStr] Description
** @argrule Ini maxlen [ajuint] Maximum length
** @argrule Ref ms [EnsPMiscellaneousset] Ensembl Miscellaneous Set
**
** @valrule * [EnsPMiscellaneousset] Ensembl Miscellaneous Set or NULL
**
** @fcategory new
******************************************************************************/




/* @func ensMiscellaneoussetNewCpy ********************************************
**
** Object-based constructor function, which returns an independent object.
**
** @param [r] ms [const EnsPMiscellaneousset] Ensembl Miscellaneous Set
**
** @return [EnsPMiscellaneousset] Ensembl Miscellaneous Set or NULL
**
** @release 6.4.0
** @@
******************************************************************************/

EnsPMiscellaneousset ensMiscellaneoussetNewCpy(
    const EnsPMiscellaneousset ms)
{
    EnsPMiscellaneousset pthis = NULL;

    if (!ms)
        return NULL;

    AJNEW0(pthis);

    pthis->Use        = 1U;
    pthis->Identifier = ms->Identifier;
    pthis->Adaptor    = ms->Adaptor;

    if (ms->Code)
        pthis->Code = ajStrNewRef(ms->Code);

    if (ms->Name)
        pthis->Name = ajStrNewRef(ms->Name);

    if (ms->Description)
        pthis->Description = ajStrNewRef(ms->Description);

    pthis->MaximumLength = ms->MaximumLength;

    return pthis;
}




/* @func ensMiscellaneoussetNewIni ********************************************
**
** Constructor for an Ensembl Miscellaneous Set with initial values.
**
** @cc Bio::EnsEMBL::Storable::new
** @param [u] msa [EnsPMiscellaneoussetadaptor]
** Ensembl Miscellaneous Set Adaptor
** @param [r] identifier [ajuint] SQL database-internal identifier
** @cc Bio::EnsEMBL::MiscSet::new
** @param [u] code [AjPStr] Code
** @param [u] name [AjPStr] Name
** @param [u] description [AjPStr] Description
** @param [r] maxlen [ajuint] Maximum length
**
** @return [EnsPMiscellaneousset] Ensembl Miscellaneous Set or NULL
**
** @release 6.4.0
** @@
******************************************************************************/

EnsPMiscellaneousset ensMiscellaneoussetNewIni(
    EnsPMiscellaneoussetadaptor msa,
    ajuint identifier,
    AjPStr code,
    AjPStr name,
    AjPStr description,
    ajuint maxlen)
{
    EnsPMiscellaneousset ms = NULL;

    AJNEW0(ms);

    ms->Use        = 1U;
    ms->Identifier = identifier;
    ms->Adaptor    = msa;

    if (code)
        ms->Code = ajStrNewRef(code);

    if (name)
        ms->Name = ajStrNewRef(name);

    if (description)
        ms->Description = ajStrNewRef(description);

    ms->MaximumLength = maxlen;

    return ms;
}




/* @func ensMiscellaneoussetNewRef ********************************************
**
** Ensembl Object referencing function, which returns a pointer to the
** Ensembl Object passed in and increases its reference count.
**
** @param [u] ms [EnsPMiscellaneousset] Ensembl Miscellaneous Set
**
** @return [EnsPMiscellaneousset] Ensembl Miscellaneous Set or NULL
**
** @release 6.2.0
** @@
******************************************************************************/

EnsPMiscellaneousset ensMiscellaneoussetNewRef(EnsPMiscellaneousset ms)
{
    if (!ms)
        return NULL;

    ms->Use++;

    return ms;
}




/* @section destructors *******************************************************
**
** Destruction destroys all internal data structures and frees the memory
** allocated for an Ensembl Miscellaneous Set object.
**
** @fdata [EnsPMiscellaneousset]
**
** @nam3rule Del Destroy (free) an Ensembl Miscellaneous Set
**
** @argrule * Pms [EnsPMiscellaneousset*] Ensembl Miscellaneous Set address
**
** @valrule * [void]
**
** @fcategory delete
******************************************************************************/




/* @func ensMiscellaneoussetDel ***********************************************
**
** Default destructor for an Ensembl Miscellaneous Set.
**
** @param [d] Pms [EnsPMiscellaneousset*] Ensembl Miscellaneous Set address
**
** @return [void]
**
** @release 6.2.0
** @@
******************************************************************************/

void ensMiscellaneoussetDel(EnsPMiscellaneousset *Pms)
{
    EnsPMiscellaneousset pthis = NULL;

    if (!Pms)
        return;

#if defined(AJ_DEBUG) && AJ_DEBUG >= 1
    if (ajDebugTest("ensMiscellaneoussetDel"))
    {
        ajDebug("ensMiscellaneoussetDel\n"
                "  *Pms %p\n",
                *Pms);

        ensMiscellaneoussetTrace(*Pms, 1);
    }
#endif /* defined(AJ_DEBUG) && AJ_DEBUG >= 1 */

    if (!*Pms)
        return;

    pthis = *Pms;

    pthis->Use--;

    if (pthis->Use)
    {
        *Pms = NULL;

        return;
    }

    ajStrDel(&pthis->Code);
    ajStrDel(&pthis->Name);
    ajStrDel(&pthis->Description);

    AJFREE(pthis);

    *Pms = NULL;

    return;
}




/* @section member retrieval **************************************************
**
** Functions for returning members of an Ensembl Miscellaneous Set object.
**
** @fdata [EnsPMiscellaneousset]
**
** @nam3rule Get Return Ensembl Miscellaneous Set attribute(s)
** @nam4rule Adaptor Return the Ensembl Miscellaneous Set Adaptor
** @nam4rule Code Return the code
** @nam4rule Description Return the description
** @nam4rule Identifier Return the SQL database-internal identifier
** @nam4rule Maximumlength Return the maximum length
** @nam4rule Name Return the name
**
** @argrule * ms [const EnsPMiscellaneousset] Ensembl Miscellaneous Set
**
** @valrule Adaptor [EnsPMiscellaneoussetadaptor]
** Ensembl Miscellaneous Set Adaptor or NULL
** @valrule Code [AjPStr] Code or NULL
** @valrule Description [AjPStr] Description or NULL
** @valrule Identifier [ajuint] SQL database-internal identifier or 0U
** @valrule Maximumlength [ajuint] Maximum length or 0U
** @valrule Name [AjPStr] Name or NULL
**
** @fcategory use
******************************************************************************/




/* @func ensMiscellaneoussetGetAdaptor ****************************************
**
** Get the Ensembl Miscellaneous Set Adaptor member of an
** Ensembl Miscellaneous Set.
**
** @cc Bio::EnsEMBL::Storable::adaptor
** @param [r] ms [const EnsPMiscellaneousset] Ensembl Miscellaneous Set
**
** @return [EnsPMiscellaneoussetadaptor] Ensembl Miscellaneous Set Adaptor
** or NULL
**
** @release 6.2.0
** @@
******************************************************************************/

EnsPMiscellaneoussetadaptor ensMiscellaneoussetGetAdaptor(
    const EnsPMiscellaneousset ms)
{
    return (ms) ? ms->Adaptor : NULL;
}




/* @func ensMiscellaneoussetGetCode *******************************************
**
** Get the code member of an Ensembl Miscellaneous Set.
**
** @cc Bio::EnsEMBL::MiscSet::code
** @param [r] ms [const EnsPMiscellaneousset] Ensembl Miscellaneous Set
**
** @return [AjPStr] Code or NULL
**
** @release 6.2.0
** @@
******************************************************************************/

AjPStr ensMiscellaneoussetGetCode(const EnsPMiscellaneousset ms)
{
    return (ms) ? ms->Code : NULL;
}




/* @func ensMiscellaneoussetGetDescription ************************************
**
** Get the description member of an Ensembl Miscellaneous Set.
**
** @cc Bio::EnsEMBL::MiscSet::description
** @param [r] ms [const EnsPMiscellaneousset] Ensembl Miscellaneous Set
**
** @return [AjPStr] Description or NULL
**
** @release 6.2.0
** @@
******************************************************************************/

AjPStr ensMiscellaneoussetGetDescription(const EnsPMiscellaneousset ms)
{
    return (ms) ? ms->Description : NULL;
}




/* @func ensMiscellaneoussetGetIdentifier *************************************
**
** Get the SQL database-internal identifier member of an
** Ensembl Miscellaneous Set.
**
** @cc Bio::EnsEMBL::Storable::dbID
** @param [r] ms [const EnsPMiscellaneousset] Ensembl Miscellaneous Set
**
** @return [ajuint] SQL database-internal identifier or 0U
**
** @release 6.2.0
** @@
******************************************************************************/

ajuint ensMiscellaneoussetGetIdentifier(const EnsPMiscellaneousset ms)
{
    return (ms) ? ms->Identifier : 0U;
}




/* @func ensMiscellaneoussetGetMaximumlength **********************************
**
** Get the maximum length member of an Ensembl Miscellaneous Set.
**
** @cc Bio::EnsEMBL::MiscSet::longest_feature
** @param [r] ms [const EnsPMiscellaneousset] Ensembl Miscellaneous Set
**
** @return [ajuint] Maximum length or 0U
**
** @release 6.3.0
** @@
******************************************************************************/

ajuint ensMiscellaneoussetGetMaximumlength(const EnsPMiscellaneousset ms)
{
    return (ms) ? ms->MaximumLength : 0U;
}




/* @func ensMiscellaneoussetGetName *******************************************
**
** Get the name member of an Ensembl Miscellaneous Set.
**
** @cc Bio::EnsEMBL::MiscSet::name
** @param [r] ms [const EnsPMiscellaneousset] Ensembl Miscellaneous Set
**
** @return [AjPStr] Name or NULL
**
** @release 6.2.0
** @@
******************************************************************************/

AjPStr ensMiscellaneoussetGetName(const EnsPMiscellaneousset ms)
{
    return (ms) ? ms->Name : NULL;
}




/* @section modifiers *********************************************************
**
** Functions for assigning members of an Ensembl Miscellaneous Set object.
**
** @fdata [EnsPMiscellaneousset]
**
** @nam3rule Set Set one member of an Ensembl Miscellaneous Set
** @nam4rule Adaptor Set the Ensembl Miscellaneous Set Adaptor
** @nam4rule Code Set the code
** @nam4rule Description Set the description
** @nam4rule Identifier Set the SQL database-internal identifier
** @nam4rule Maximumlength Set the maximum length
** @nam4rule Name Set the name
**
** @argrule * ms [EnsPMiscellaneousset] Ensembl Miscellaneous Set object
** @argrule Adaptor msa [EnsPMiscellaneoussetadaptor] Ensembl Miscellaneous
**                                                    Set Adaptor
** @argrule Code code [AjPStr] Code
** @argrule Description description [AjPStr] Description
** @argrule Identifier identifier [ajuint] SQL database-internal identifier
** @argrule Maximumlength maxlen [ajuint] Maximum length
** @argrule Name name [AjPStr] Name
**
** @valrule * [AjBool] ajTrue upon success, ajFalse otherwise
**
** @fcategory modify
******************************************************************************/




/* @func ensMiscellaneoussetSetAdaptor ****************************************
**
** Set the Ensembl Miscellaneous Set Adaptor member of an
** Ensembl Miscellaneous Set.
**
** @cc Bio::EnsEMBL::Storable::adaptor
** @param [u] ms [EnsPMiscellaneousset] Ensembl Miscellaneous Set
** @param [u] msa [EnsPMiscellaneoussetadaptor] Ensembl Miscellaneous
**                                              Set Adaptor
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
**
** @release 6.2.0
** @@
******************************************************************************/

AjBool ensMiscellaneoussetSetAdaptor(EnsPMiscellaneousset ms,
                                     EnsPMiscellaneoussetadaptor msa)
{
    if (!ms)
        return ajFalse;

    ms->Adaptor = msa;

    return ajTrue;
}




/* @func ensMiscellaneoussetSetCode *******************************************
**
** Set the code member of an Ensembl Miscellaneous Set.
**
** @cc Bio::EnsEMBL::MiscSet::code
** @param [u] ms [EnsPMiscellaneousset] Ensembl Miscellaneous Set
** @param [u] code [AjPStr] Code
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise.
**
** @release 6.2.0
** @@
******************************************************************************/

AjBool ensMiscellaneoussetSetCode(EnsPMiscellaneousset ms, AjPStr code)
{
    if (!ms)
        return ajFalse;

    ajStrDel(&ms->Code);

    ms->Code = ajStrNewRef(code);

    return ajTrue;
}




/* @func ensMiscellaneoussetSetDescription ************************************
**
** Set the description member of an Ensembl Miscellaneous Set.
**
** @cc Bio::EnsEMBL::MiscSet::description
** @param [u] ms [EnsPMiscellaneousset] Ensembl Miscellaneous Set
** @param [u] description [AjPStr] Description
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
**
** @release 6.2.0
** @@
******************************************************************************/

AjBool ensMiscellaneoussetSetDescription(EnsPMiscellaneousset ms,
                                         AjPStr description)
{
    if (!ms)
        return ajFalse;

    ajStrDel(&ms->Description);

    ms->Description = ajStrNewRef(description);

    return ajTrue;
}




/* @func ensMiscellaneoussetSetIdentifier *************************************
**
** Set the SQL database-internal identifier member of an
** Ensembl Miscellaneous Set.
**
** @cc Bio::EnsEMBL::Storable::dbID
** @param [u] ms [EnsPMiscellaneousset] Ensembl Miscellaneous Set
** @param [r] identifier [ajuint] SQL database-internal identifier
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
**
** @release 6.2.0
** @@
******************************************************************************/

AjBool ensMiscellaneoussetSetIdentifier(EnsPMiscellaneousset ms,
                                        ajuint identifier)
{
    if (!ms)
        return ajFalse;

    ms->Identifier = identifier;

    return ajTrue;
}




/* @func ensMiscellaneoussetSetMaximumlength **********************************
**
** Set the maximum length member of an Ensembl Miscellaneous Set.
**
** @cc Bio::EnsEMBL::MiscSet::longest_feature
** @param [u] ms [EnsPMiscellaneousset] Ensembl Miscellaneous Set
** @param [r] maxlen [ajuint] Maximum length
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
**
** @release 6.3.0
** @@
******************************************************************************/

AjBool ensMiscellaneoussetSetMaximumlength(EnsPMiscellaneousset ms,
                                           ajuint maxlen)
{
    if (!ms)
        return ajFalse;

    ms->MaximumLength = maxlen;

    return ajTrue;
}




/* @func ensMiscellaneoussetSetName *******************************************
**
** Set the name member of an Ensembl Miscellaneous Set.
**
** @cc Bio::EnsEMBL::MiscSet::name
** @param [u] ms [EnsPMiscellaneousset] Ensembl Miscellaneous Set
** @param [u] name [AjPStr] Name
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
**
** @release 6.2.0
** @@
******************************************************************************/

AjBool ensMiscellaneoussetSetName(EnsPMiscellaneousset ms, AjPStr name)
{
    if (!ms)
        return ajFalse;

    ajStrDel(&ms->Name);

    ms->Name = ajStrNewRef(name);

    return ajTrue;
}




/* @section debugging *********************************************************
**
** Functions for reporting of an Ensembl Miscellaneous Set object.
**
** @fdata [EnsPMiscellaneousset]
**
** @nam3rule Trace Report Ensembl Miscellaneous Set members to debug file
**
** @argrule Trace ms [const EnsPMiscellaneousset] Ensembl Miscellaneous Set
** @argrule Trace level [ajuint] Indentation level
**
** @valrule * [AjBool] ajTrue upon success, ajFalse otherwise
**
** @fcategory misc
******************************************************************************/




/* @func ensMiscellaneoussetTrace *********************************************
**
** Trace an Ensembl Miscellaneous Set.
**
** @param [r] ms [const EnsPMiscellaneousset] Ensembl Miscellaneous Set
** @param [r] level [ajuint] Indentation level
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
**
** @release 6.2.0
** @@
******************************************************************************/

AjBool ensMiscellaneoussetTrace(const EnsPMiscellaneousset ms, ajuint level)
{
    AjPStr indent = NULL;

    if (!ms)
        return ajFalse;

    indent = ajStrNew();

    ajStrAppendCountK(&indent, ' ', level * 2);

    ajDebug("%SensMiscellaneoussetTrace %p\n"
            "%S  Use %u\n"
            "%S  Identifier %u\n"
            "%S  Adaptor %p\n"
            "%S  Code '%S'\n"
            "%S  Name '%S'\n"
            "%S  Description '%S'\n"
            "%S  MaximumLength %u\n",
            indent, ms,
            indent, ms->Use,
            indent, ms->Identifier,
            indent, ms->Adaptor,
            indent, ms->Code,
            indent, ms->Name,
            indent, ms->Description,
            indent, ms->MaximumLength);

    ajStrDel(&indent);

    return ajTrue;
}




/* @section calculate *********************************************************
**
** Functions for calculating values of an Ensembl Miscellaneous Set object.
**
** @fdata [EnsPMiscellaneousset]
**
** @nam3rule Calculate Calculate Ensembl Miscellaneous Set values
** @nam4rule Memsize Calculate the memory size in bytes
**
** @argrule * ms [const EnsPMiscellaneousset] Ensembl Miscellaneous Set
**
** @valrule Memsize [size_t] Memory size in bytes or 0
**
** @fcategory misc
******************************************************************************/




/* @func ensMiscellaneoussetCalculateMemsize **********************************
**
** Calculate the memory size in bytes of an Ensembl Miscellaneous Set.
**
** @param [r] ms [const EnsPMiscellaneousset] Ensembl Miscellaneous Set
**
** @return [size_t] Memory size in bytes or 0
**
** @release 6.4.0
** @@
******************************************************************************/

size_t ensMiscellaneoussetCalculateMemsize(const EnsPMiscellaneousset ms)
{
    size_t size = 0;

    if (!ms)
        return 0;

    size += sizeof (EnsOMiscellaneousset);

    if (ms->Code)
    {
        size += sizeof (AjOStr);

        size += ajStrGetRes(ms->Code);
    }

    if (ms->Name)
    {
        size += sizeof (AjOStr);

        size += ajStrGetRes(ms->Name);
    }

    if (ms->Description)
    {
        size += sizeof (AjOStr);

        size += ajStrGetRes(ms->Description);
    }

    return size;
}




/* @datasection [AjPList] AJAX List *******************************************
**
** @nam2rule List Functions for manipulating AJAX List objects
**
******************************************************************************/




/* @funcstatic listMiscellaneoussetCompareIdentifierAscending *****************
**
** AJAX List of Ensembl Miscellaneous Set objects comparison function to sort
** by Ensembl Miscellaneous Set identifier in ascending order.
**
** @param [r] item1 [const void*] Ensembl Miscellaneous Set address 1
** @param [r] item2 [const void*] Ensembl Miscellaneous Set address 2
** @see ajListSort
**
** @return [int] The comparison function returns an integer less than,
**               equal to, or greater than zero if the first argument is
**               considered to be respectively less than, equal to, or
**               greater than the second.
**
** @release 6.4.0
** @@
******************************************************************************/

static int listMiscellaneoussetCompareIdentifierAscending(
    const void *item1,
    const void *item2)
{
    EnsPMiscellaneousset ms1 = *(EnsOMiscellaneousset *const *) item1;
    EnsPMiscellaneousset ms2 = *(EnsOMiscellaneousset *const *) item2;

#if defined(AJ_DEBUG) && AJ_DEBUG >= 2
    if (ajDebugTest("listMiscellaneoussetCompareIdentifierAscending"))
        ajDebug("listMiscellaneoussetCompareIdentifierAscending\n"
                "  ms1 %p\n"
                "  ms2 %p\n",
                ms1,
                ms2);
#endif /* defined(AJ_DEBUG) && AJ_DEBUG >= 2 */

    /* Sort empty values towards the end of the AJAX List. */

    if (ms1 && (!ms2))
        return -1;

    if ((!ms1) && (!ms2))
        return 0;

    if ((!ms1) && ms2)
        return +1;

    if (ms1->Identifier < ms2->Identifier)
        return -1;

    if (ms1->Identifier > ms2->Identifier)
        return +1;

    return 0;
}




/* @funcstatic listMiscellaneoussetCompareIdentifierDescending ****************
**
** AJAX List of Ensembl Miscellaneous Set objects comparison function to sort
** by Ensembl Miscellaneous Set identifier in descending order.
**
** @param [r] item1 [const void*] Ensembl Miscellaneous Set address 1
** @param [r] item2 [const void*] Ensembl Miscellaneous Set address 2
** @see ajListSort
**
** @return [int] The comparison function returns an integer less than,
**               equal to, or greater than zero if the first argument is
**               considered to be respectively less than, equal to, or
**               greater than the second.
**
** @release 6.4.0
** @@
******************************************************************************/

static int listMiscellaneoussetCompareIdentifierDescending(
    const void *item1,
    const void *item2)
{
    EnsPMiscellaneousset ms1 = *(EnsOMiscellaneousset *const *) item1;
    EnsPMiscellaneousset ms2 = *(EnsOMiscellaneousset *const *) item2;

#if defined(AJ_DEBUG) && AJ_DEBUG >= 2
    if (ajDebugTest("listMiscellaneoussetCompareIdentifierDescending"))
        ajDebug("listMiscellaneoussetCompareIdentifierDescending\n"
                "  ms1 %p\n"
                "  ms2 %p\n",
                ms1,
                ms2);
#endif /* defined(AJ_DEBUG) && AJ_DEBUG >= 2 */

    /* Sort empty values towards the end of the AJAX List. */

    if (ms1 && (!ms2))
        return -1;

    if ((!ms1) && (!ms2))
        return 0;

    if ((!ms1) && ms2)
        return +1;

    if (ms1->Identifier > ms2->Identifier)
        return -1;

    if (ms1->Identifier < ms2->Identifier)
        return +1;

    return 0;
}




/* @section list **************************************************************
**
** Functions for manipulating AJAX List objects.
**
** @fdata [AjPList]
**
** @nam3rule Miscellaneousset Functions for manipulating AJAX List objects of
** Ensembl Miscellaneous Set objects
** @nam4rule Sort Sort functions
** @nam5rule Identifier Sort by Ensembl Miscellaneous Set identifier member
** @nam5rule Name Sort by Ensembl Miscellaneous Set name member
** @nam6rule Ascending  Sort in ascending order
** @nam6rule Descending Sort in descending order
**
** @argrule Sort mss [AjPList] AJAX List of Ensembl Miscellaneous Set objects
**
** @valrule * [AjBool] ajTrue upon success, ajFalse otherwise
**
** @fcategory misc
******************************************************************************/




/* @func ensListMiscellaneoussetSortIdentifierAscending ***********************
**
** Sort an AJAX List of Ensembl Miscellaneous Set objects by their
** Ensembl Miscellaneous Set identifier in ascending order.
**
** @param [u] mss [AjPList] AJAX List of Ensembl Miscellaneous Set objects
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
**
** @release 6.4.0
** @@
******************************************************************************/

AjBool ensListMiscellaneoussetSortIdentifierAscending(AjPList mss)
{
    if (!mss)
        return ajFalse;

    ajListSort(mss, &listMiscellaneoussetCompareIdentifierAscending);

    return ajTrue;
}




/* @func ensListMiscellaneoussetSortIdentifierDescending **********************
**
** Sort an AJAX List of Ensembl Miscellaneous Set objects by their
** Ensembl Miscellaneous Set identifier in descending order.
**
** @param [u] mss [AjPList] AJAX List of Ensembl Miscellaneous Set objects
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
**
** @release 6.4.0
** @@
******************************************************************************/

AjBool ensListMiscellaneoussetSortIdentifierDescending(AjPList mss)
{
    if (!mss)
        return ajFalse;

    ajListSort(mss, &listMiscellaneoussetCompareIdentifierDescending);

    return ajTrue;
}




/* @datasection [EnsPMiscellaneoussetadaptor] Ensembl Miscellaneous Set Adaptor
**
** @nam2rule Miscellaneoussetadaptor Functions for manipulating
** Ensembl Miscellaneous Set Adaptor objects
**
** @cc Bio::EnsEMBL::DBSQL::MiscSetAdaptor
** @cc CVS Revision: 1.16
** @cc CVS Tag: branch-ensembl-66
**
******************************************************************************/




/* @funcstatic miscellaneoussetadaptorFetchAllbyStatement *********************
**
** Run a SQL statement against an Ensembl Database Adaptor and consolidate the
** results into an AJAX List of Ensembl Miscellaneous Set objects.
**
** @param [u] ba [EnsPBaseadaptor] Ensembl Base Adaptor
** @param [r] statement [const AjPStr] SQL statement
** @param [uN] am [EnsPAssemblymapper] Ensembl Assembly Mapper
** @param [uN] slice [EnsPSlice] Ensembl Slice
** @param [u] mss [AjPList] AJAX List of Ensembl Miscellaneous Sets
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
**
** @release 6.4.0
** @@
******************************************************************************/

static AjBool miscellaneoussetadaptorFetchAllbyStatement(
    EnsPBaseadaptor ba,
    const AjPStr statement,
    EnsPAssemblymapper am,
    EnsPSlice slice,
    AjPList mss)
{
    ajuint identifier = 0U;
    ajuint maxlen     = 0U;

    AjPSqlstatement sqls = NULL;
    AjISqlrow sqli       = NULL;
    AjPSqlrow sqlr       = NULL;

    AjPStr code        = NULL;
    AjPStr name        = NULL;
    AjPStr description = NULL;

    EnsPDatabaseadaptor dba = NULL;

    EnsPMiscellaneousset        ms  = NULL;
    EnsPMiscellaneoussetadaptor msa = NULL;

    if (ajDebugTest("miscellaneoussetadaptorFetchAllbyStatement"))
        ajDebug("miscellaneoussetadaptorFetchAllbyStatement\n"
                "  ba %p\n"
                "  statement %p\n"
                "  am %p\n"
                "  slice %p\n"
                "  mss %p\n",
                ba,
                statement,
                am,
                slice,
                mss);

    if (!ba)
        return ajFalse;

    if (!statement)
        return ajFalse;

    if (!mss)
        return ajFalse;

    dba = ensBaseadaptorGetDatabaseadaptor(ba);

    msa = ensRegistryGetMiscellaneoussetadaptor(dba);

    sqls = ensDatabaseadaptorSqlstatementNew(dba, statement);

    sqli = ajSqlrowiterNew(sqls);

    while (!ajSqlrowiterDone(sqli))
    {
        identifier  = 0;
        code        = ajStrNew();
        name        = ajStrNew();
        description = ajStrNew();
        maxlen      = 0;

        sqlr = ajSqlrowiterGet(sqli);

        ajSqlcolumnToUint(sqlr, &identifier);
        ajSqlcolumnToStr(sqlr, &code);
        ajSqlcolumnToStr(sqlr, &name);
        ajSqlcolumnToStr(sqlr, &description);
        ajSqlcolumnToUint(sqlr, &maxlen);

        ms = ensMiscellaneoussetNewIni(msa,
                                       identifier,
                                       code,
                                       name,
                                       description,
                                       maxlen);

        ajListPushAppend(mss, (void *) ms);

        ajStrDel(&code);
        ajStrDel(&name);
        ajStrDel(&description);
    }

    ajSqlrowiterDel(&sqli);

    ensDatabaseadaptorSqlstatementDel(dba, &sqls);

    return ajTrue;
}




/* @section constructors ******************************************************
**
** All constructors return a new Ensembl Miscellaneous Set Adaptor by pointer.
** It is the responsibility of the user to first destroy any previous
** Miscellaneous Set Adaptor. The target pointer does not need to be
** initialised to NULL, but it is good programming practice to do so anyway.
**
** @fdata [EnsPMiscellaneoussetadaptor]
**
** @nam3rule New Constructor
**
** @argrule New dba [EnsPDatabaseadaptor] Ensembl Database Adaptor
**
** @valrule * [EnsPMiscellaneoussetadaptor]
** Ensembl Miscellaneous Set Adaptor or NULL
**
** @fcategory new
******************************************************************************/




/* @func ensMiscellaneoussetadaptorNew ****************************************
**
** Default constructor for an Ensembl Miscellaneous Set Adaptor.
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
** @see ensRegistryGetMiscellaneoussetadaptor
**
** @cc Bio::EnsEMBL::DBSQL::MiscSetAdaptor::new
** @param [u] dba [EnsPDatabaseadaptor] Ensembl Database Adaptor
**
** @return [EnsPMiscellaneoussetadaptor]
** Ensembl Miscellaneous Set Adaptor or NULL
**
** @release 6.2.0
** @@
******************************************************************************/

EnsPMiscellaneoussetadaptor ensMiscellaneoussetadaptorNew(
    EnsPDatabaseadaptor dba)
{
    EnsPMiscellaneoussetadaptor msa = NULL;

    if (!dba)
        return NULL;

    AJNEW0(msa);

    msa->Adaptor = ensBaseadaptorNew(
        dba,
        miscellaneoussetadaptorKTables,
        miscellaneoussetadaptorKColumns,
        (const EnsPBaseadaptorLeftjoin) NULL,
        (const char *) NULL,
        (const char *) NULL,
        &miscellaneoussetadaptorFetchAllbyStatement);

    /*
    ** NOTE: The cache cannot be initialised here because the
    ** miscellaneoussetadaptorCacheInit function calls
    ** ensBaseadaptorFetchAllbyConstraint, which calls
    ** miscellaneoussetadaptorFetchAllbyStatement, which calls
    ** ensRegistryGetMiscellaneoussetadaptor. At that point, however, the
    ** Miscellaneous Set Adaptor has not been stored in the Registry.
    ** Therefore, each ensMiscellaneoussetadaptorFetch function has to test
    ** the presence of the adaptor-internal cache and eventually initialise
    ** before accessing it.
    **
    miscellaneoussetadaptorCacheInit(msa);
    */

    return msa;
}




/* @section cache *************************************************************
**
** Functions for maintaining the Ensembl Miscellaneous Set Adaptor-internal
** cache of Ensembl Miscellaneous Set objects.
**
** @fdata [EnsPMiscellaneoussetadaptor]
**
** @nam3rule Cache Process an Ensembl Miscellaneous Set Adaptor-internal cache
** @nam4rule Clear Clear the Ensembl Miscellaneous Set Adaptor-internal cache
**
** @argrule * msa [EnsPMiscellaneoussetadaptor]
** Ensembl Miscellaneous Set Adaptor
**
** @valrule * [AjBool] True on success, ajFalse otherwise
**
** @fcategory use
******************************************************************************/




/* @funcstatic miscellaneoussetadaptorCacheInit *******************************
**
** Initialise the internal Miscellaneous Set cache of an
** Ensembl Miscellaneous Set Adaptor.
**
** @param [u] msa [EnsPMiscellaneoussetadaptor]
** Ensembl Miscellaneous Set Adaptor
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
**
** @release 6.3.0
** @@
******************************************************************************/

static AjBool miscellaneoussetadaptorCacheInit(
    EnsPMiscellaneoussetadaptor msa)
{
    AjPList mss      = NULL;

    EnsPMiscellaneousset ms = NULL;

    if (!msa)
        return ajFalse;

    if (msa->CacheByIdentifier)
        return ajFalse;
    else
    {
        msa->CacheByIdentifier = ajTableuintNew(0);

        ajTableSetDestroyvalue(
            msa->CacheByIdentifier,
            (void (*)(void **)) &ensMiscellaneoussetDel);
    }

    if (msa->CacheByCode)
        return ajFalse;
    else
    {
        msa->CacheByCode = ajTablestrNew(0);

        ajTableSetDestroyvalue(
            msa->CacheByCode,
            (void (*)(void **)) &ensMiscellaneoussetDel);
    }

    mss = ajListNew();

    ensBaseadaptorFetchAllbyConstraint(msa->Adaptor,
                                       (const AjPStr) NULL,
                                       (EnsPAssemblymapper) NULL,
                                       (EnsPSlice) NULL,
                                       mss);

    while (ajListPop(mss, (void **) &ms))
    {
        miscellaneoussetadaptorCacheInsert(msa, &ms);

        /*
        ** Both caches hold internal references to the
        ** Miscellaneous Set objects.
        */

        ensMiscellaneoussetDel(&ms);
    }

    ajListFree(&mss);

    return ajTrue;
}




/* @funcstatic miscellaneoussetadaptorCacheInsert *****************************
**
** Insert an Ensembl Miscellaneous Set into the
** Miscellaneous Set Adaptor-internal cache.
** If a Miscellaneous Set with the same code member is already present in the
** adaptor cache, the Miscellaneous Set is deleted and a pointer to the cached
** Miscellaneous Set is returned.
**
** @param [u] msa [EnsPMiscellaneoussetadaptor]
** Ensembl Miscellaneous Set Adaptor
** @param [u] Pms [EnsPMiscellaneousset*] Ensembl Miscellaneous Set address
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
**
** @release 6.3.0
** @@
******************************************************************************/

static AjBool miscellaneoussetadaptorCacheInsert(
    EnsPMiscellaneoussetadaptor msa,
    EnsPMiscellaneousset *Pms)
{
    ajuint *Pidentifier = NULL;

    EnsPMiscellaneousset ms1 = NULL;
    EnsPMiscellaneousset ms2 = NULL;

    if (!msa)
        return ajFalse;

    if (!msa->CacheByIdentifier)
        return ajFalse;

    if (!msa->CacheByCode)
        return ajFalse;

    if (!Pms)
        return ajFalse;

    if (!*Pms)
        return ajFalse;

    /* Search the identifer cache. */

    ms1 = (EnsPMiscellaneousset) ajTableFetchmodV(
        msa->CacheByIdentifier,
        (const void *) &((*Pms)->Identifier));

    /* Search the code cache. */

    ms2 = (EnsPMiscellaneousset) ajTableFetchmodS(
        msa->CacheByCode,
        (*Pms)->Code);

    if ((!ms1) && (!ms2))
    {
        /* Insert into the identifier cache. */

        AJNEW0(Pidentifier);

        *Pidentifier = (*Pms)->Identifier;

        ajTablePut(msa->CacheByIdentifier,
                   (void *) Pidentifier,
                   (void *) ensMiscellaneoussetNewRef(*Pms));

        /* Insert into the code cache. */

        ajTablePut(msa->CacheByCode,
                   (void *) ajStrNewS((*Pms)->Code),
                   (void *) ensMiscellaneoussetNewRef(*Pms));
    }

    if (ms1 && ms2 && (ms1 == ms2))
    {
        ajDebug("miscellaneoussetadaptorCacheInsert replaced "
                "Miscellaneous Set %p with one already cached %p.\n",
                *Pms, ms1);

        ensMiscellaneoussetDel(Pms);

        Pms = &ms1;
    }

    if (ms1 && ms2 && (ms1 != ms2))
        ajDebug("miscellaneoussetadaptorCacheInsert detected "
                "Miscellaneous Sets in the identifier and code cache with "
                "identical codes ('%S' and '%S') but different addresses "
                "(%p and %p).\n",
                ms1->Code, ms2->Code, ms1, ms2);

    if (ms1 && (!ms2))
        ajDebug("miscellaneoussetadaptorCacheInsert detected an "
                "Ensembl Miscellaneous Set "
                "in the identifier, but not in the code cache.\n");

    if ((!ms1) && ms2)
        ajDebug("miscellaneoussetadaptorCacheInsert detected an "
                "Ensembl Miscellaneous Set "
                "in the code, but not in the identifier cache.\n");

    return ajTrue;
}




#if AJFALSE
/* @funcstatic miscellaneoussetadaptorCacheRemove *****************************
**
** Remove an Ensembl Miscellaneous Set from the
** Miscellaneous Set Adaptor-internal cache.
**
** @param [u] msa [EnsPMiscellaneoussetadaptor]
** Ensembl Miscellaneous Set Adaptor
** @param [u] ms [EnsPMiscellaneousset] Ensembl Miscellaneous Set
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
**
** @release 6.3.0
** @@
******************************************************************************/

static AjBool miscellaneoussetadaptorCacheRemove(
    EnsPMiscellaneoussetadaptor msa,
    EnsPMiscellaneousset ms)
{
    EnsPMiscellaneousset ms1 = NULL;
    EnsPMiscellaneousset ms2 = NULL;

    if (!msa)
        return ajFalse;

    if (!ms)
        return ajFalse;

    ms1 = (EnsPMiscellaneousset) ajTableRemove(
        msa->CacheByIdentifier,
        (const void *) &ms->Identifier);

    ms2 = (EnsPMiscellaneousset) ajTableRemove(
        msa->CacheByCode,
        (const void *) ms->Code);

    if (ms1 && (!ms2))
        ajWarn("miscellaneoussetadaptorCacheRemove could remove "
               "Miscellaneous Set '%S' (%u) "
               "only from the identifier cache.\n",
               ms->Code, ms->Identifier);

    if ((!ms1) && ms2)
        ajWarn("miscellaneoussetadaptorCacheRemove could remove "
               "Miscellaneous Set '%S' (%u) "
               "only from the code cache.\n",
               ms->Code, ms->Identifier);

    ensMiscellaneoussetDel(&ms1);
    ensMiscellaneoussetDel(&ms2);

    return ajTrue;
}

#endif /* AJFALSE */




/* @func ensMiscellaneoussetadaptorCacheClear *********************************
**
** Clear the Ensembl Miscellaneous Set Adaptor-internal cache of
** Ensembl Miscellaneous Set objects.
**
** @param [u] msa [EnsPMiscellaneoussetadaptor]
** Ensembl Miscellaneous Set Adaptor
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
**
** @release 6.4.0
** @@
******************************************************************************/

AjBool ensMiscellaneoussetadaptorCacheClear(
    EnsPMiscellaneoussetadaptor msa)
{
    if (!msa)
        return ajFalse;

    ajTableDel(&msa->CacheByIdentifier);
    ajTableDel(&msa->CacheByCode);

    return ajTrue;
}




/* @section destructors *******************************************************
**
** Destruction destroys all internal data structures and frees the memory
** allocated for an Ensembl Miscellaneous Set Adaptor object.
**
** @fdata [EnsPMiscellaneoussetadaptor]
**
** @nam3rule Del Destroy (free) an Ensembl Miscellaneous Set Adaptor
**
** @argrule * Pmsa [EnsPMiscellaneoussetadaptor*]
** Ensembl Miscellaneous Set Adaptor address
**
** @valrule * [void]
**
** @fcategory delete
******************************************************************************/




/* @func ensMiscellaneoussetadaptorDel ****************************************
**
** Default destructor for an Ensembl Miscellaneous Set Adaptor.
**
** This function also clears the internal caches.
**
** Ensembl Object Adaptors are singleton objects that are registered in the
** Ensembl Registry and weakly referenced by Ensembl Objects that have been
** instantiated by it. Therefore, Ensembl Object Adaptors should never be
** destroyed directly. Upon exit, the Ensembl Registry will call this function
** if required.
**
** @param [d] Pmsa [EnsPMiscellaneoussetadaptor*]
** Ensembl Miscellaneous Set Adaptor address
**
** @return [void]
**
** @release 6.2.0
** @@
******************************************************************************/

void ensMiscellaneoussetadaptorDel(EnsPMiscellaneoussetadaptor *Pmsa)
{
    EnsPMiscellaneoussetadaptor pthis = NULL;

    if (!Pmsa)
        return;

#if defined(AJ_DEBUG) && AJ_DEBUG >= 1
    if (ajDebugTest("ensMiscellaneoussetadaptorDel"))
        ajDebug("ensMiscellaneoussetadaptorDel\n"
                "  *Pmsa %p\n",
                *Pmsa);
#endif /* defined(AJ_DEBUG) && AJ_DEBUG >= 1 */

    if (!*Pmsa)
        return;

    pthis = *Pmsa;

    ensMiscellaneoussetadaptorCacheClear(pthis);

    ensBaseadaptorDel(&pthis->Adaptor);

    AJFREE(pthis);

    *Pmsa = NULL;

    return;
}




/* @section member retrieval **************************************************
**
** Functions for returning members of an
** Ensembl Miscellaneous Set Adaptor object.
**
** @fdata [EnsPMiscellaneoussetadaptor]
**
** @nam3rule Get Return Ensembl Miscellaneous Set Adaptor attribute(s)
** @nam4rule Baseadaptor     Return the Ensembl Base Adaptor
** @nam4rule Databaseadaptor Return the Ensembl Database Adaptor
**
** @argrule * msa [const EnsPMiscellaneoussetadaptor]
** Ensembl Miscellaneous Set Adaptor
**
** @valrule Baseadaptor [EnsPBaseadaptor]
** Ensembl Base Adaptor or NULL
** @valrule Databaseadaptor [EnsPDatabaseadaptor]
** Ensembl Database Adaptor or NULL
**
** @fcategory use
******************************************************************************/




/* @func ensMiscellaneoussetadaptorGetBaseadaptor *****************************
**
** Get the Ensembl Base Adaptor member of an
** Ensembl Miscellaneous Set Adaptor.
**
** @param [r] msa [const EnsPMiscellaneoussetadaptor]
** Ensembl Miscellaneous Set Adaptor
**
** @return [EnsPBaseadaptor] Ensembl Base Adaptor or NULL
**
** @release 6.4.0
** @@
******************************************************************************/

EnsPBaseadaptor ensMiscellaneoussetadaptorGetBaseadaptor(
    const EnsPMiscellaneoussetadaptor msa)
{
    return (msa) ? msa->Adaptor : NULL;
}




/* @func ensMiscellaneoussetadaptorGetDatabaseadaptor *************************
**
** Get the Ensembl Database Adaptor member of an
** Ensembl Miscellaneous Set Adaptor.
**
** @param [r] msa [const EnsPMiscellaneoussetadaptor]
** Ensembl Miscellaneous Set Adaptor
**
** @return [EnsPDatabaseadaptor] Ensembl Database Adaptor or NULL
**
** @release 6.2.0
** @@
******************************************************************************/

EnsPDatabaseadaptor ensMiscellaneoussetadaptorGetDatabaseadaptor(
    const EnsPMiscellaneoussetadaptor msa)
{
    return (msa) ? ensBaseadaptorGetDatabaseadaptor(msa->Adaptor) : NULL;
}




/* @section object fetching ***************************************************
**
** Functions for fetching Ensembl Miscellaneous Set objects from an
** Ensembl SQL database.
**
** @fdata [EnsPMiscellaneoussetadaptor]
**
** @nam3rule Fetch   Fetch Ensembl Miscellaneous Set object(s)
** @nam4rule All     Fetch all Ensembl Miscellaneous Set objects
** @nam5rule Allby   Fetch all Ensembl Miscellaneous Set objects
**                   matching a criterion
** @nam4rule FetchBy Fetch one Ensembl Miscellaneous Set object
**                   matching a criterion
** @nam5rule Code    Fetch by code
** @nam5rule Identifier Fetch by an SQL database-internal identifier
**
** @argrule * msa [EnsPMiscellaneoussetadaptor]
** Ensembl Miscellaneous Set Adaptor
** @argrule All mss [AjPList] AJAX List of Ensembl Miscellaneous Set objects
** @argrule ByCode code [const AjPStr] Ensembl Miscellaneous Set code
** @argrule ByIdentifier identifier [ajuint] SQL database-internal identifier
** @argrule By Pms [EnsPMiscellaneousset*] Ensembl Miscellaneous Set address
**
** @valrule * [AjBool] ajTrue upon success, ajFalse otherwise
**
** @fcategory use
******************************************************************************/




/* @funcstatic miscellaneoussetadaptorFetchAll ********************************
**
** An ajTableMap "apply" function to return all Miscellaneous Set objects
** from the Ensembl Miscellaneous Set Adaptor-internal cache.
**
** @param [u] key [const void*] AJAX unsigned integer key data address
** @param [u] Pvalue [void**] Ensembl Miscellaneous Set value data address
** @param [u] cl [void*] AJAX List of Ensembl Miscellaneous Set objects,
**                       passed in via ajTableMap
** @see ajTableMap
**
** @return [void]
**
** @release 6.3.0
** @@
******************************************************************************/

static void miscellaneoussetadaptorFetchAll(const void *key,
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

    ajListPushAppend(
        (AjPList) cl,
        (void *) ensMiscellaneoussetNewRef(
            *((EnsPMiscellaneousset *) Pvalue)));

    return;
}




/* @func ensMiscellaneoussetadaptorFetchAll ***********************************
**
** Fetch all Ensembl Miscellaneous Sets.
**
** @cc Bio::EnsEMBL::DBSQL::MiscSetAdaptor::fetch_all
** @param [u] msa [EnsPMiscellaneoussetadaptor]
** Ensembl Miscellaneous Set Adaptor
** @param [u] mss [AjPList] AJAX List of Ensembl Miscellaneous Set objects
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
**
** @release 6.2.0
** @@
******************************************************************************/

AjBool ensMiscellaneoussetadaptorFetchAll(
    EnsPMiscellaneoussetadaptor msa,
    AjPList mss)
{
    if (!msa)
        return ajFalse;

    if (!mss)
        return ajFalse;

    if (!msa->CacheByIdentifier)
        miscellaneoussetadaptorCacheInit(msa);

    ajTableMap(msa->CacheByIdentifier,
               &miscellaneoussetadaptorFetchAll,
               (void *) mss);

    return ajTrue;
}




/* @func ensMiscellaneoussetadaptorFetchByCode ********************************
**
** Fetch an Ensembl Miscellaneous Set by its code.
**
** @cc Bio::EnsEMBL::DBSQL::MiscSetAdaptor::fetch_by_code
** @param [u] msa [EnsPMiscellaneoussetadaptor]
** Ensembl Miscellaneous Set Adaptor
** @param [r] code [const AjPStr] Ensembl Miscellaneous Set code
** @param [wP] Pms [EnsPMiscellaneousset*] Ensembl Miscellaneous Set address
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
**
** @release 6.2.0
** @@
******************************************************************************/

AjBool ensMiscellaneoussetadaptorFetchByCode(
    EnsPMiscellaneoussetadaptor msa,
    const AjPStr code,
    EnsPMiscellaneousset *Pms)
{
    char *txtcode = NULL;

    AjPList mss = NULL;

    AjPStr constraint = NULL;

    EnsPDatabaseadaptor dba = NULL;

    EnsPMiscellaneousset ms = NULL;

    if (!msa)
        return ajFalse;

    if ((!code) && (!ajStrGetLen(code)))
        return ajFalse;

    if (!Pms)
        return ajFalse;

    /*
    ** Initially, search the code cache.
    ** For any object returned by the AJAX Table the reference counter needs
    ** to be incremented manually.
    */

    if (!msa->CacheByCode)
        miscellaneoussetadaptorCacheInit(msa);

    *Pms = (EnsPMiscellaneousset) ajTableFetchmodS(msa->CacheByCode, code);

    if (*Pms)
    {
        ensMiscellaneoussetNewRef(*Pms);

        return ajTrue;
    }

    /* In case of a cache miss, query the database. */

    dba = ensBaseadaptorGetDatabaseadaptor(msa->Adaptor);

    ensDatabaseadaptorEscapeC(dba, &txtcode, code);

    constraint = ajFmtStr("misc_set.code = '%s'", txtcode);

    ajCharDel(&txtcode);

    mss = ajListNew();

    ensBaseadaptorFetchAllbyConstraint(msa->Adaptor,
                                       constraint,
                                       (EnsPAssemblymapper) NULL,
                                       (EnsPSlice) NULL,
                                       mss);

    if (ajListGetLength(mss) > 1)
        ajWarn("ensMiscellaneoussetadaptorFetchByCode got more than one "
               "Ensembl Miscellaneous Sets for (UNIQUE) code '%S'.\n",
               code);

    ajListPop(mss, (void **) Pms);

    miscellaneoussetadaptorCacheInsert(msa, Pms);

    while (ajListPop(mss, (void **) &ms))
    {
        miscellaneoussetadaptorCacheInsert(msa, &ms);

        ensMiscellaneoussetDel(&ms);
    }

    ajListFree(&mss);

    ajStrDel(&constraint);

    return ajTrue;
}




/* @func ensMiscellaneoussetadaptorFetchByIdentifier **************************
**
** Fetch an Ensembl Miscellaneous Set by its SQL database-internal identifier.
**
** @cc Bio::EnsEMBL::DBSQL::MiscSetAdaptor::fetch_by_dbID
** @param [u] msa [EnsPMiscellaneoussetadaptor]
** Ensembl Miscellaneous Set Adaptor
** @param [r] identifier [ajuint] SQL database-internal identifier
** @param [wP] Pms [EnsPMiscellaneousset*] Ensembl Miscellaneous Set address
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
**
** @release 6.2.0
** @@
******************************************************************************/

AjBool ensMiscellaneoussetadaptorFetchByIdentifier(
    EnsPMiscellaneoussetadaptor msa,
    ajuint identifier,
    EnsPMiscellaneousset *Pms)
{
    AjPList mss = NULL;

    AjPStr constraint = NULL;

    EnsPMiscellaneousset ms = NULL;

    if (!msa)
        return ajFalse;

    if (!identifier)
        return ajFalse;

    if (!Pms)
        return ajFalse;

    /*
    ** Initially, search the identifier cache.
    ** For any object returned by the AJAX Table the reference counter needs
    ** to be incremented manually.
    */

    if (!msa->CacheByIdentifier)
        miscellaneoussetadaptorCacheInit(msa);

    *Pms = (EnsPMiscellaneousset) ajTableFetchmodV(msa->CacheByIdentifier,
                                                   (const void *) &identifier);

    if (*Pms)
    {
        ensMiscellaneoussetNewRef(*Pms);

        return ajTrue;
    }

    /* For a cache miss query the database. */

    constraint = ajFmtStr("misc_set.misc_set_id = %u", identifier);

    mss = ajListNew();

    ensBaseadaptorFetchAllbyConstraint(msa->Adaptor,
                                       constraint,
                                       (EnsPAssemblymapper) NULL,
                                       (EnsPSlice) NULL,
                                       mss);

    if (ajListGetLength(mss) > 1)
        ajWarn("ensMiscellaneoussetadaptorFetchByIdentifier got more "
               "than one Ensembl Miscellaneous Set for (PRIMARY KEY) "
               "identifier %u.\n", identifier);

    ajListPop(mss, (void **) Pms);

    miscellaneoussetadaptorCacheInsert(msa, Pms);

    while (ajListPop(mss, (void **) &ms))
    {
        miscellaneoussetadaptorCacheInsert(msa, &ms);

        ensMiscellaneoussetDel(&ms);
    }

    ajListFree(&mss);

    ajStrDel(&constraint);

    return ajTrue;
}




/* @datasection [EnsPMiscellaneousfeature] Ensembl Miscellaneous Feature ******
**
** @nam2rule Miscellaneousfeature Functions for manipulating
** Ensembl Miscellaneous Feature objects
**
** @cc Bio::EnsEMBL::MiscFeature
** @cc CVS Revision: 1.19
** @cc CVS Tag: branch-ensembl-66
**
******************************************************************************/




/* @section constructors ******************************************************
**
** All constructors return a new Ensembl Miscellaneous Feature by pointer.
** It is the responsibility of the user to first destroy any previous
** Miscellaneous Feature. The target pointer does not need to be initialised
** to NULL, but it is good programming practice to do so anyway.
**
** @fdata [EnsPMiscellaneousfeature]
**
** @nam3rule New Constructor
** @nam4rule Cpy Constructor with existing object
** @nam4rule Ini Constructor with initial value
** @nam4rule Ref Constructor by incrementing the reference counter
**
** @argrule Cpy mf [const EnsPMiscellaneousfeature]
** Ensembl Miscellaneous Feature
** @argrule Ini mfa [EnsPMiscellaneousfeatureadaptor]
** Ensembl Miscellaneous Feature Adaptor
** @argrule Ini identifier [ajuint] SQL database-internal identifier
** @argrule Ini feature [EnsPFeature] Ensembl Feature
** @argrule Ref mf [EnsPMiscellaneousfeature] Ensembl Miscellaneous Feature
**
** @valrule * [EnsPMiscellaneousfeature] Ensembl Miscellaneous Feature or NULL
**
** @fcategory new
******************************************************************************/




/* @func ensMiscellaneousfeatureNewCpy ****************************************
**
** Object-based constructor function, which returns an independent object.
**
** @param [r] mf [const EnsPMiscellaneousfeature]
** Ensembl Miscellaneous Feature
**
** @return [EnsPMiscellaneousfeature] Ensembl Miscellaneous Feature or NULL
**
** @release 6.4.0
** @@
******************************************************************************/

EnsPMiscellaneousfeature ensMiscellaneousfeatureNewCpy(
    const EnsPMiscellaneousfeature mf)
{
    AjIList iter = NULL;

    EnsPAttribute attribute = NULL;

    EnsPMiscellaneousfeature pthis = NULL;

    EnsPMiscellaneousset ms = NULL;

    if (!mf)
        return NULL;

    AJNEW0(pthis);

    pthis->Use        = 1U;
    pthis->Identifier = mf->Identifier;
    pthis->Adaptor    = mf->Adaptor;
    pthis->Feature    = ensFeatureNewRef(mf->Feature);

    /* NOTE: Copy the AJAX List of Ensembl Attribute objects. */

    if (mf->Attributes && ajListGetLength(mf->Attributes))
    {
        pthis->Attributes = ajListNew();

        iter = ajListIterNew(mf->Attributes);

        while (!ajListIterDone(iter))
        {
            attribute = (EnsPAttribute) ajListIterGet(iter);

            ajListPushAppend(pthis->Attributes,
                             (void *) ensAttributeNewRef(attribute));
        }

        ajListIterDel(&iter);
    }
    else
        pthis->Attributes = NULL;

    /* NOTE: Copy the AJAX List of Ensembl Miscellaneous Set objects. */

    if (mf->Miscellaneoussets && ajListGetLength(mf->Miscellaneoussets))
    {
        pthis->Miscellaneoussets = ajListNew();

        iter = ajListIterNew(mf->Miscellaneoussets);

        while (!ajListIterDone(iter))
        {
            ms = (EnsPMiscellaneousset) ajListIterGet(iter);

            ajListPushAppend(pthis->Miscellaneoussets,
                             (void *) ensMiscellaneoussetNewRef(ms));
        }

        ajListIterDel(&iter);
    }
    else
        pthis->Miscellaneoussets = NULL;

    return pthis;
}




/* @func ensMiscellaneousfeatureNewIni ****************************************
**
** Constructor for an Ensembl Miscellaneous Feature with initial values.
**
** @cc Bio::EnsEMBL::Storable::new
** @param [u] mfa [EnsPMiscellaneousfeatureadaptor]
** Ensembl Miscellaneous Feature Adaptor
** @param [r] identifier [ajuint] SQL database-internal identifier
** @cc Bio::EnsEMBL::Feature::new
** @param [u] feature [EnsPFeature] Ensembl Feature
** @cc Bio::EnsEMBL::MiscFeature::new
**
** @return [EnsPMiscellaneousfeature] Ensembl Miscellaneous Feature or NULL
**
** @release 6.4.0
** @@
******************************************************************************/

EnsPMiscellaneousfeature ensMiscellaneousfeatureNewIni(
    EnsPMiscellaneousfeatureadaptor mfa,
    ajuint identifier,
    EnsPFeature feature)
{
    EnsPMiscellaneousfeature mf = NULL;

    if (!feature)
        return NULL;

    AJNEW0(mf);

    mf->Use               = 1U;
    mf->Identifier        = identifier;
    mf->Adaptor           = mfa;
    mf->Feature           = ensFeatureNewRef(feature);
    mf->Attributes        = NULL;
    mf->Miscellaneoussets = NULL;

    return mf;
}




/* @func ensMiscellaneousfeatureNewRef ****************************************
**
** Ensembl Object referencing function, which returns a pointer to the
** Ensembl Object passed in and increases its reference count.
**
** @param [u] mf [EnsPMiscellaneousfeature] Ensembl Miscellaneous Feature
**
** @return [EnsPMiscellaneousfeature] Ensembl Miscellaneous Feature or NULL
**
** @release 6.2.0
** @@
******************************************************************************/

EnsPMiscellaneousfeature ensMiscellaneousfeatureNewRef(
    EnsPMiscellaneousfeature mf)
{
    if (!mf)
        return NULL;

    mf->Use++;

    return mf;
}




/* @section destructors *******************************************************
**
** Destruction destroys all internal data structures and frees the memory
** allocated for an Ensembl Miscellaneous Feature object.
**
** @fdata [EnsPMiscellaneousfeature]
**
** @nam3rule Del Destroy (free) an Ensembl Miscellaneous Feature
**
** @argrule * Pmf [EnsPMiscellaneousfeature*]
** Ensembl Miscellaneous Feature address
**
** @valrule * [void]
**
** @fcategory delete
******************************************************************************/




/* @func ensMiscellaneousfeatureDel *******************************************
**
** Default destructor for an Ensembl Miscellaneous Feature.
**
** @param [d] Pmf [EnsPMiscellaneousfeature*]
** Ensembl Miscellaneous Feature address
**
** @return [void]
**
** @release 6.2.0
** @@
******************************************************************************/

void ensMiscellaneousfeatureDel(EnsPMiscellaneousfeature *Pmf)
{
    EnsPAttribute attribute = NULL;

    EnsPMiscellaneousset ms = NULL;

    EnsPMiscellaneousfeature pthis = NULL;


    if (!Pmf)
        return;

#if defined(AJ_DEBUG) && AJ_DEBUG >= 1
    if (ajDebugTest("ensMiscellaneousfeatureDel"))
    {
        ajDebug("ensMiscellaneousfeatureDel\n"
                "  *Pmf %p\n",
                *Pmf);

        ensMiscellaneousfeatureTrace(*Pmf, 1);
    }
#endif /* defined(AJ_DEBUG) && AJ_DEBUG >= 1 */

    if (!*Pmf)
        return;

    pthis = *Pmf;

    pthis->Use--;

    if (pthis->Use)
    {
        *Pmf = NULL;

        return;
    }

    ensFeatureDel(&pthis->Feature);

    /* Clear and free the AJAX List of Ensembl Attribute objects. */

    while (ajListPop(pthis->Attributes, (void **) &attribute))
        ensAttributeDel(&attribute);

    ajListFree(&pthis->Attributes);

    /* Clear and free the AJAX List of Ensembl Miscellaneous Set objects. */

    while (ajListPop(pthis->Miscellaneoussets, (void **) &ms))
        ensMiscellaneoussetDel(&ms);

    ajListFree(&pthis->Miscellaneoussets);

    AJFREE(pthis);

    *Pmf = NULL;

    return;
}




/* @section member retrieval **************************************************
**
** Functions for returning members of an Ensembl Miscellaneous Feature object.
**
** @fdata [EnsPMiscellaneousfeature]
**
** @nam3rule Get Return Miscellaneous Feature attribute(s)
** @nam4rule Adaptor Return the Ensembl Miscellaneous Feature Adaptor
** @nam4rule Attributes Return all Ensembl Attribute objects
** @nam4rule Feature Return the Ensembl Feature
** @nam4rule Identifier Return the SQL database-internal identifier
** @nam4rule Miscellaneoussets Return all Ensembl Miscellaneous Set objects
**
** @argrule * mf [const EnsPMiscellaneousfeature] Ensembl Miscellaneous Feature
**
** @valrule Adaptor [EnsPMiscellaneousfeatureadaptor]
** Ensembl Miscellaneous Feature Adaptor or NULL
** @valrule Attributes [const AjPList]
** AJAX List of Ensembl Attribute objects or NULL
** @valrule Feature [EnsPFeature] Ensembl Feature or NULL
** @valrule Identifier [ajuint] SQL database-internal identifier or 0U
** @valrule Miscellaneoussets [const AjPList]
** AJAX List of Ensembl Miscellaneous Set objects or NULL
**
** @fcategory use
******************************************************************************/




/* @func ensMiscellaneousfeatureGetAdaptor ************************************
**
** Get the Ensembl Miscellaneous Feature Adaptor member of an
** Ensembl Miscellaneous Feature.
**
** @cc Bio::EnsEMBL::Storable::adaptor
** @param [r] mf [const EnsPMiscellaneousfeature]
** Ensembl Miscellaneous Feature
**
** @return [EnsPMiscellaneousfeatureadaptor]
** Ensembl Miscellaneous Feature Adaptor or NULL
**
** @release 6.2.0
** @@
******************************************************************************/

EnsPMiscellaneousfeatureadaptor ensMiscellaneousfeatureGetAdaptor(
    const EnsPMiscellaneousfeature mf)
{
    return (mf) ? mf->Adaptor : NULL;
}




/* @func ensMiscellaneousfeatureGetAttributes *********************************
**
** Get all Ensembl Attribute objects of an Ensembl Miscellaneous Feature.
**
** @cc Bio::EnsEMBL::MiscFeature::get_all_Attributes
** @param [r] mf [const EnsPMiscellaneousfeature]
** Ensembl Miscellaneous Feature
**
** @return [const AjPList] AJAX List of Ensembl Attribute objects or NULL
**
** @release 6.2.0
** @@
******************************************************************************/

const AjPList ensMiscellaneousfeatureGetAttributes(
    const EnsPMiscellaneousfeature mf)
{
    return (mf) ? mf->Attributes : NULL;
}




/* @func ensMiscellaneousfeatureGetFeature ************************************
**
** Get the Ensembl Feature member of an Ensembl Miscellaneous Feature.
**
** @param [r] mf [const EnsPMiscellaneousfeature]
** Ensembl Miscellaneous Feature
**
** @return [EnsPFeature] Ensembl Feature or NULL
**
** @release 6.2.0
** @@
******************************************************************************/

EnsPFeature ensMiscellaneousfeatureGetFeature(
    const EnsPMiscellaneousfeature mf)
{
    return (mf) ? mf->Feature : NULL;
}




/* @func ensMiscellaneousfeatureGetIdentifier *********************************
**
** Get the SQL database-internal identifier member of an
** Ensembl Miscellaneous Feature.
**
** @cc Bio::EnsEMBL::Storable::dbID
** @param [r] mf [const EnsPMiscellaneousfeature]
** Ensembl Miscellaneous Feature
**
** @return [ajuint] SQL database-internal identifier or 0U
**
** @release 6.2.0
** @@
******************************************************************************/

ajuint ensMiscellaneousfeatureGetIdentifier(
    const EnsPMiscellaneousfeature mf)
{
    return (mf) ? mf->Identifier : 0U;
}




/* @func ensMiscellaneousfeatureGetMiscellaneoussets **************************
**
** Get all Ensembl Miscellaneous Sets of an Ensembl Miscellaneous Feature.
**
** @cc Bio::EnsEMBL::MiscFeature::get_all_MiscSets
** @param [r] mf [const EnsPMiscellaneousfeature]
** Ensembl Miscellaneous Feature
**
** @return [const AjPList] AJAX List of Ensembl Miscellaneous Sets or NULL
**
** @release 6.2.0
** @@
******************************************************************************/

const AjPList ensMiscellaneousfeatureGetMiscellaneoussets(
    const EnsPMiscellaneousfeature mf)
{
    return (mf) ? mf->Miscellaneoussets : NULL;
}




/* @section modifiers *********************************************************
**
** Functions for assigning members of an Ensembl Miscellaneous Feature object.
**
** @fdata [EnsPMiscellaneousfeature]
**
** @nam3rule Set Set one member of a Miscellaneous Feature
** @nam4rule Adaptor Set the Ensembl Miscellaneous Feature Adaptor
** @nam4rule Identifier Set the SQL database-internal identifier
** @nam4rule Feature Set the Ensembl Feature
**
** @argrule * mf [EnsPMiscellaneousfeature]
** Ensembl Miscellaneous Feature object
** @argrule Adaptor mfa [EnsPMiscellaneousfeatureadaptor]
** Ensembl Miscellaneous Feature Adaptor
** @argrule Feature feature [EnsPFeature] Ensembl Feature
** @argrule Identifier identifier [ajuint] SQL database-internal identifier
**
** @valrule * [AjBool] ajTrue upon success, ajFalse otherwise
**
** @fcategory modify
******************************************************************************/




/* @func ensMiscellaneousfeatureSetAdaptor ************************************
**
** Set the Ensembl Miscellaneous Feature Adaptor member of an
** Ensembl Miscellaneous Feature.
**
** @cc Bio::EnsEMBL::Storable::adaptor
** @param [u] mf [EnsPMiscellaneousfeature] Ensembl Miscellaneous Feature
** @param [u] mfa [EnsPMiscellaneousfeatureadaptor]
** Ensembl Miscellaneous Feature Adaptor
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
**
** @release 6.2.0
** @@
******************************************************************************/

AjBool ensMiscellaneousfeatureSetAdaptor(
    EnsPMiscellaneousfeature mf,
    EnsPMiscellaneousfeatureadaptor mfa)
{
    if (!mf)
        return ajFalse;

    mf->Adaptor = mfa;

    return ajTrue;
}




/* @func ensMiscellaneousfeatureSetFeature ************************************
**
** Set the Ensembl Feature member of an Ensembl Miscellaneous Feature.
**
** @param [u] mf [EnsPMiscellaneousfeature] Ensembl Miscellaneous Feature
** @param [u] feature [EnsPFeature] Ensembl Feature
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
**
** @release 6.2.0
** @@
******************************************************************************/

AjBool ensMiscellaneousfeatureSetFeature(
    EnsPMiscellaneousfeature mf,
    EnsPFeature feature)
{
    if (!mf)
        return ajFalse;

    ensFeatureDel(&mf->Feature);

    mf->Feature = ensFeatureNewRef(feature);

    return ajTrue;
}




/* @func ensMiscellaneousfeatureSetIdentifier *********************************
**
** Set the SQL database-internal identifier member of an
** Ensembl Miscellaneous Feature.
**
** @cc Bio::EnsEMBL::Storable::dbID
** @param [u] mf [EnsPMiscellaneousfeature] Ensembl Miscellaneous Feature
** @param [r] identifier [ajuint] SQL database-internal identifier
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
**
** @release 6.2.0
** @@
******************************************************************************/

AjBool ensMiscellaneousfeatureSetIdentifier(
    EnsPMiscellaneousfeature mf,
    ajuint identifier)
{
    if (!mf)
        return ajFalse;

    mf->Identifier = identifier;

    return ajTrue;
}




/* @section member addition ***************************************************
**
** Functions for adding members to an Ensembl Miscellaneous Feature object.
**
** @fdata [EnsPMiscellaneousfeature]
**
** @nam3rule Add Add one object to an Ensembl Miscellaneous Feature
** @nam4rule Attribute Add an Ensembl Attribute
** @nam4rule Miscellaneousset Add an Ensembl Miscellaneous Set
**
** @argrule * mf [EnsPMiscellaneousfeature] Ensembl Miscellaneous Feature
** @argrule Attribute attribute [EnsPAttribute] Ensembl Attribute
** @argrule Miscellaneousset ms [EnsPMiscellaneousset]
** Ensembl Miscellaneous Set
**
** @valrule * [AjBool] ajTrue upon success, ajFalse otherwise
**
** @fcategory modify
******************************************************************************/




/* @func ensMiscellaneousfeatureAddAttribute **********************************
**
** Add an Ensembl Attribute to an Ensembl Miscellaneous Feature.
**
** @cc Bio::EnsEMBL::MiscFeature::add_Attribute
** @param [u] mf [EnsPMiscellaneousfeature] Ensembl Miscellaneous Feature
** @param [u] attribute [EnsPAttribute] Ensembl Attribute
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
**
** @release 6.2.0
** @@
******************************************************************************/

AjBool ensMiscellaneousfeatureAddAttribute(
    EnsPMiscellaneousfeature mf,
    EnsPAttribute attribute)
{
    if (!mf)
        return ajFalse;

    if (!attribute)
        return ajFalse;

    if (!mf->Attributes)
        mf->Attributes = ajListNew();

    ajListPushAppend(mf->Attributes, (void *) ensAttributeNewRef(attribute));

    return ajTrue;
}




/* @func ensMiscellaneousfeatureAddMiscellaneousset ***************************
**
** Add an Ensembl Miscellaneous Set to an Ensembl Miscellaneous Feature.
**
** @cc Bio::EnsEMBL::MiscFeature::add_MiscSet
** @param [u] mf [EnsPMiscellaneousfeature] Ensembl Miscellaneous Feature
** @param [u] ms [EnsPMiscellaneousset] Ensembl Miscellaneous Set
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
**
** @release 6.2.0
** @@
******************************************************************************/

AjBool ensMiscellaneousfeatureAddMiscellaneousset(
    EnsPMiscellaneousfeature mf,
    EnsPMiscellaneousset ms)
{
    if (!mf)
        return ajFalse;

    if (!ms)
        return ajFalse;

    if (!mf->Miscellaneoussets)
        mf->Miscellaneoussets = ajListNew();

    ajListPushAppend(mf->Miscellaneoussets,
                     (void *) ensMiscellaneoussetNewRef(ms));

    return ajTrue;
}




/* @section debugging *********************************************************
**
** Functions for reporting of an Ensembl Miscellaneous Feature object.
**
** @fdata [EnsPMiscellaneousfeature]
**
** @nam3rule Trace Report Ensembl Miscellaneous Feature members to debug file
**
** @argrule Trace mf [const EnsPMiscellaneousfeature]
** Ensembl Miscellaneous Feature
** @argrule Trace level [ajuint] Indentation level
**
** @valrule * [AjBool] ajTrue upon success, ajFalse otherwise
**
** @fcategory misc
******************************************************************************/




/* @func ensMiscellaneousfeatureTrace *****************************************
**
** Trace an Ensembl Miscellaneous Feature.
**
** @param [r] mf [const EnsPMiscellaneousfeature]
** Ensembl Miscellaneous Feature
** @param [r] level [ajuint] Indentation level
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
**
** @release 6.2.0
** @@
******************************************************************************/

AjBool ensMiscellaneousfeatureTrace(const EnsPMiscellaneousfeature mf,
                                    ajuint level)
{
    AjPStr indent = NULL;

    if (!mf)
        return ajFalse;

    indent = ajStrNew();

    ajStrAppendCountK(&indent, ' ', level * 2);

    ajDebug("%SensMiscellaneousfeatureTrace %p\n"
            "%S  Use %u\n"
            "%S  Identifier %u\n"
            "%S  Adaptor %p\n"
            "%S  Feature %p\n"
            "%S  Attributes %p\n"
            "%S  Miscellaneoussets %p\n",
            indent, mf,
            indent, mf->Use,
            indent, mf->Identifier,
            indent, mf->Adaptor,
            indent, mf->Feature,
            indent, mf->Attributes,
            indent, mf->Miscellaneoussets);

    ensFeatureTrace(mf->Feature, level + 1);

    ajStrDel(&indent);

    return ajTrue;
}




/* @section calculate *********************************************************
**
** Functions for calculating values of an Ensembl Miscellaneous Feature object.
**
** @fdata [EnsPMiscellaneousfeature]
**
** @nam3rule Calculate Calculate Ensembl Miscellaneous Feature values
** @nam4rule Memsize Calculate the memory size in bytes
**
** @argrule * mf [const EnsPMiscellaneousfeature] Ensembl Miscellaneous Feature
**
** @valrule Memsize [size_t] Memory size in bytes or 0
**
** @fcategory misc
******************************************************************************/




/* @func ensMiscellaneousfeatureCalculateMemsize ******************************
**
** Calculate the memory size in bytes of an Ensembl Miscellaneous Feature.
**
** @param [r] mf [const EnsPMiscellaneousfeature]
** Ensembl Miscellaneous Feature
**
** @return [size_t] Memory size in bytes or 0
**
** @release 6.4.0
** @@
******************************************************************************/

size_t ensMiscellaneousfeatureCalculateMemsize(
    const EnsPMiscellaneousfeature mf)
{
    size_t size = 0;

    AjIList iter = NULL;

    EnsPAttribute attribute = NULL;

    EnsPMiscellaneousset ms = NULL;

    if (!mf)
        return 0;

    size += sizeof (EnsOMiscellaneousfeature);

    size += ensFeatureCalculateMemsize(mf->Feature);

    if (mf->Attributes)
    {
        iter = ajListIterNewread(mf->Attributes);

        while (!ajListIterDone(iter))
        {
            attribute = (EnsPAttribute) ajListIterGet(iter);

            size += ensAttributeCalculateMemsize(attribute);
        }

        ajListIterDel(&iter);
    }

    if (mf->Miscellaneoussets)
    {
        iter = ajListIterNewread(mf->Miscellaneoussets);

        while (!ajListIterDone(iter))
        {
            ms = (EnsPMiscellaneousset) ajListIterGet(iter);

            size += ensMiscellaneoussetCalculateMemsize(ms);
        }

        ajListIterDel(&iter);
    }

    return size;
}




/* @section fetch *************************************************************
**
** Functions for fetching values of an Ensembl Miscellaneous Feature object.
**
** @fdata [EnsPMiscellaneousfeature]
**
** @nam3rule Fetch Fetch Ensembl Miscellaneous Feature values
** @nam4rule All Fetch all objects
** @nam5rule Attributes Fetch all Ensembl Attribute objects
** @nam5rule Miscellaneoussets Fetch all Ensembl Miscellaneous Set objects
**
** @argrule AllAttributes mf [EnsPMiscellaneousfeature]
** Ensembl Miscellaneous Feature
** @argrule AllAttributes code [const AjPStr]
** Ensembl Attribute code
** @argrule AllAttributes attributes [AjPList]
** AJAX List of Ensembl Attribute objects
** @argrule AllMiscellaneoussets mf [EnsPMiscellaneousfeature]
** Ensembl Miscellaneous Feature
** @argrule AllMiscellaneoussets code [const AjPStr]
** Ensembl Miscellaneous Set code
** @argrule AllMiscellaneoussets mss [AjPList]
** AJAX List of Ensembl Miscellaneous Set objects
**
** @valrule * [AjBool] ajTrue upon success, ajFalse otherwise
**
** @fcategory misc
******************************************************************************/




/* @func ensMiscellaneousfeatureFetchAllAttributes ****************************
**
** Fetch all Ensembl Attribute objects of an Ensembl Miscellaneous Feature.
**
** The caller is responsible for deleting the Ensembl Attribute objects
** before deleting the AJAX List.
**
** @cc Bio::EnsEMBL::MiscFeature::get_all_Attributes
** @param [u] mf [EnsPMiscellaneousfeature] Ensembl Miscellaneous Feature
** @param [r] code [const AjPStr] Ensembl Attribute code
** @param [u] attributes [AjPList] AJAX List of Ensembl Attribute objects
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
**
** @release 6.2.0
** @@
******************************************************************************/

AjBool ensMiscellaneousfeatureFetchAllAttributes(
    EnsPMiscellaneousfeature mf,
    const AjPStr code,
    AjPList attributes)
{
    AjIList iter = NULL;

    EnsPAttribute attribute = NULL;

    if (!mf)
        return ajFalse;

    if (!attributes)
        return ajFalse;

    iter = ajListIterNewread(mf->Attributes);

    while (!ajListIterDone(iter))
    {
        attribute = (EnsPAttribute) ajListIterGet(iter);

        if (code && ajStrGetLen(code))
        {
            if (ajStrMatchCaseS(code, ensAttributeGetCode(attribute)))
                ajListPushAppend(attributes,
                                 (void *) ensAttributeNewRef(attribute));
        }
        else
            ajListPushAppend(attributes,
                             (void *) ensAttributeNewRef(attribute));
    }

    ajListIterDel(&iter);

    return ajTrue;
}




/* @func ensMiscellaneousfeatureFetchAllMiscellaneoussets *********************
**
** Fetch all Ensembl Miscellaneous Set objects of an
** Ensembl Miscellaneous Feature.
**
** The caller is responsible for deleting the Ensembl Miscellaneous Set objects
** before deleting the AJAX List.
**
** @cc Bio::EnsEMBL::MiscFeature::get_all_MiscSets
** @param [u] mf [EnsPMiscellaneousfeature] Ensembl Miscellaneous Feature
** @param [r] code [const AjPStr] Ensembl Miscellaneous Set code
** @param [u] mss [AjPList] AJAX List of Ensembl Miscellanaeous Set objects
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
**
** @release 6.2.0
** @@
******************************************************************************/

AjBool ensMiscellaneousfeatureFetchAllMiscellaneoussets(
    EnsPMiscellaneousfeature mf,
    const AjPStr code,
    AjPList mss)
{
    AjIList iter = NULL;

    EnsPMiscellaneousset ms = NULL;

    if (!mf)
        return ajFalse;

    if (!mss)
        return ajFalse;

    iter = ajListIterNewread(mf->Miscellaneoussets);

    while (!ajListIterDone(iter))
    {
        ms = (EnsPMiscellaneousset) ajListIterGet(iter);

        if (code && ajStrGetLen(code))
        {
            if (ajStrMatchCaseS(code, ensMiscellaneoussetGetCode(ms)))
                ajListPushAppend(mss,
                                 (void *) ensMiscellaneoussetNewRef(ms));
        }
        else
            ajListPushAppend(mss, (void *) ensMiscellaneoussetNewRef(ms));
    }

    ajListIterDel(&iter);

    return ajTrue;
}




/* @datasection [AjPList] AJAX List *******************************************
**
** @nam2rule List Functions for manipulating AJAX List objects
**
******************************************************************************/




/* @funcstatic listMiscellaneousfeatureCompareEndAscending ********************
**
** AJAX List of Ensembl Miscellaneous Feature objects comparison function to
** sort by Ensembl Feature end coordinate in ascending order.
**
** @param [r] item1 [const void*] Ensembl Miscellaneous Feature address 1
** @param [r] item2 [const void*] Ensembl Miscellaneous Feature address 2
** @see ajListSort
**
** @return [int] The comparison function returns an integer less than,
**               equal to, or greater than zero if the first argument is
**               considered to be respectively less than, equal to, or
**               greater than the second.
**
** @release 6.4.0
** @@
******************************************************************************/

static int listMiscellaneousfeatureCompareEndAscending(
    const void *item1,
    const void *item2)
{
    EnsPMiscellaneousfeature mf1 = *(EnsOMiscellaneousfeature *const *) item1;
    EnsPMiscellaneousfeature mf2 = *(EnsOMiscellaneousfeature *const *) item2;

#if defined(AJ_DEBUG) && AJ_DEBUG >= 2
    if (ajDebugTest("listMiscellaneousfeatureCompareEndAscending"))
        ajDebug("listMiscellaneousfeatureCompareEndAscending\n"
                "  mf1 %p\n"
                "  mf2 %p\n",
                mf1,
                mf2);
#endif /* defined(AJ_DEBUG) && AJ_DEBUG >= 2 */

    /* Sort empty values towards the end of the AJAX List. */

    if (mf1 && (!mf2))
        return -1;

    if ((!mf1) && (!mf2))
        return 0;

    if ((!mf1) && mf2)
        return +1;

    return ensFeatureCompareEndAscending(mf1->Feature, mf2->Feature);
}




/* @funcstatic listMiscellaneousfeatureCompareEndDescending *******************
**
** AJAX List of Ensembl Miscellaneous Feature objects comparison function to
** sort by Ensembl Feature end coordinate in descending order.
**
** @param [r] item1 [const void*] Ensembl Miscellaneous Feature address 1
** @param [r] item2 [const void*] Ensembl Miscellaneous Feature address 2
** @see ajListSort
**
** @return [int] The comparison function returns an integer less than,
**               equal to, or greater than zero if the first argument is
**               considered to be respectively less than, equal to, or
**               greater than the second.
**
** @release 6.4.0
** @@
******************************************************************************/

static int listMiscellaneousfeatureCompareEndDescending(
    const void *item1,
    const void *item2)
{
    EnsPMiscellaneousfeature mf1 = *(EnsOMiscellaneousfeature *const *) item1;
    EnsPMiscellaneousfeature mf2 = *(EnsOMiscellaneousfeature *const *) item2;

#if defined(AJ_DEBUG) && AJ_DEBUG >= 2
    if (ajDebugTest("listMiscellaneousfeatureCompareEndDescending"))
        ajDebug("listMiscellaneousfeatureCompareEndDescending\n"
                "  mf1 %p\n"
                "  mf2 %p\n",
                mf1,
                mf2);
#endif /* defined(AJ_DEBUG) && AJ_DEBUG >= 2 */

    /* Sort empty values towards the end of the AJAX List. */

    if (mf1 && (!mf2))
        return -1;

    if ((!mf1) && (!mf2))
        return 0;

    if ((!mf1) && mf2)
        return +1;

    return ensFeatureCompareEndDescending(mf1->Feature, mf2->Feature);
}




/* @funcstatic listMiscellaneousfeatureCompareIdentifierAscending *************
**
** AJAX List of Ensembl Miscellaneous Feature objects comparison function to
** sort by identifier member in ascending order.
**
** @param [r] item1 [const void*] Ensembl Miscellaneous Feature address 1
** @param [r] item2 [const void*] Ensembl Miscellaneous Feature address 2
** @see ajListSort
**
** @return [int] The comparison function returns an integer less than,
**               equal to, or greater than zero if the first argument is
**               considered to be respectively less than, equal to, or
**               greater than the second.
**
** @release 6.4.0
** @@
******************************************************************************/

static int listMiscellaneousfeatureCompareIdentifierAscending(
    const void *item1,
    const void *item2)
{
    EnsPMiscellaneousfeature mf1 = *(EnsOMiscellaneousfeature *const *) item1;
    EnsPMiscellaneousfeature mf2 = *(EnsOMiscellaneousfeature *const *) item2;

#if defined(AJ_DEBUG) && AJ_DEBUG >= 2
    if (ajDebugTest("listMiscellaneousfeatureCompareIdentifierAscending"))
        ajDebug("listMiscellaneousfeatureCompareIdentifierAscending\n"
                "  mf1 %p\n"
                "  mf2 %p\n",
                mf1,
                mf2);
#endif /* defined(AJ_DEBUG) && AJ_DEBUG >= 2 */

    /* Sort empty values towards the end of the AJAX List. */

    if (mf1 && (!mf2))
        return -1;

    if ((!mf1) && (!mf2))
        return 0;

    if ((!mf1) && mf2)
        return +1;

    if (mf1->Identifier < mf2->Identifier)
        return -1;

    if (mf1->Identifier > mf2->Identifier)
        return +1;

    return 0;
}




/* @funcstatic listMiscellaneousfeatureCompareStartAscending ******************
**
** AJAX List of Ensembl Miscellaneous Feature objects comparison function to
** sort by Ensembl Feature start coordinate in ascending order.
**
** @param [r] item1 [const void*] Ensembl Miscellaneous Feature address 1
** @param [r] item2 [const void*] Ensembl Miscellaneous Feature address 2
** @see ajListSort
**
** @return [int] The comparison function returns an integer less than,
**               equal to, or greater than zero if the first argument is
**               considered to be respectively less than, equal to, or
**               greater than the second.
**
** @release 6.4.0
** @@
******************************************************************************/

static int listMiscellaneousfeatureCompareStartAscending(
    const void *item1,
    const void *item2)
{
    EnsPMiscellaneousfeature mf1 = *(EnsOMiscellaneousfeature *const *) item1;
    EnsPMiscellaneousfeature mf2 = *(EnsOMiscellaneousfeature *const *) item2;

#if defined(AJ_DEBUG) && AJ_DEBUG >= 2
    if (ajDebugTest("listMiscellaneousfeatureCompareStartAscending"))
        ajDebug("listMiscellaneousfeatureCompareStartAscending\n"
                "  mf1 %p\n"
                "  mf2 %p\n",
                mf1,
                mf2);
#endif /* defined(AJ_DEBUG) && AJ_DEBUG >= 2 */

    /* Sort empty values towards the end of the AJAX List. */

    if (mf1 && (!mf2))
        return -1;

    if ((!mf1) && (!mf2))
        return 0;

    if ((!mf1) && mf2)
        return +1;

    return ensFeatureCompareStartAscending(mf1->Feature, mf2->Feature);
}




/* @funcstatic listMiscellaneousfeatureCompareStartDescending *****************
**
** AJAX List of Ensembl Miscellaneous Feature objects comparison function to
** sort by Ensembl Feature start coordinate in descending order.
**
** @param [r] item1 [const void*] Ensembl Miscellaneous Feature address 1
** @param [r] item2 [const void*] Ensembl Miscellaneous Feature address 2
** @see ajListSort
**
** @return [int] The comparison function returns an integer less than,
**               equal to, or greater than zero if the first argument is
**               considered to be respectively less than, equal to, or
**               greater than the second.
**
** @release 6.4.0
** @@
******************************************************************************/

static int listMiscellaneousfeatureCompareStartDescending(
    const void *item1,
    const void *item2)
{
    EnsPMiscellaneousfeature mf1 = *(EnsOMiscellaneousfeature *const *) item1;
    EnsPMiscellaneousfeature mf2 = *(EnsOMiscellaneousfeature *const *) item2;

#if defined(AJ_DEBUG) && AJ_DEBUG >= 2
    if (ajDebugTest("listMiscellaneousfeatureCompareStartDescending"))
        ajDebug("listMiscellaneousfeatureCompareStartDescending\n"
                "  mf1 %p\n"
                "  mf2 %p\n",
                mf1,
                mf2);
#endif /* defined(AJ_DEBUG) && AJ_DEBUG >= 2 */

    /* Sort empty values towards the end of the AJAX List. */

    if (mf1 && (!mf2))
        return -1;

    if ((!mf1) && (!mf2))
        return 0;

    if ((!mf1) && mf2)
        return +1;

    return ensFeatureCompareStartDescending(mf1->Feature, mf2->Feature);
}




/* @section list **************************************************************
**
** Functions for manipulating AJAX List objects.
**
** @fdata [AjPList]
**
** @nam3rule Miscellaneousfeature Functions for manipulating AJAX List objects
** of Ensembl Miscellaneous Feature objects
** @nam4rule Sort       Sort functions
** @nam5rule End        Sort by Ensembl Feature end member
** @nam5rule Identifier Sort by identifier member
** @nam5rule Start      Sort by Ensembl Feature start member
** @nam6rule Ascending  Sort in ascending order
** @nam6rule Descending Sort in descending order
**
** @argrule * mfs [AjPList]
** AJAX List of Ensembl Miscellaneous Feature objects
**
** @valrule * [AjBool] ajTrue upon success, ajFalse otherwise
**
** @fcategory misc
******************************************************************************/




/* @func ensListMiscellaneousfeatureSortEndAscending **************************
**
** Sort an AJAX List of Ensembl Miscellaneous Feature objects by their
** Ensembl Feature end coordinate in ascending order.
**
** @param [u] mfs [AjPList] AJAX List of Ensembl Miscellaneous Feature objects
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
**
** @release 6.4.0
** @@
******************************************************************************/

AjBool ensListMiscellaneousfeatureSortEndAscending(AjPList mfs)
{
    if (!mfs)
        return ajFalse;

    ajListSortTwoThree(mfs,
                       &listMiscellaneousfeatureCompareEndAscending,
                       &listMiscellaneousfeatureCompareStartAscending,
                       &listMiscellaneousfeatureCompareIdentifierAscending);

    return ajTrue;
}




/* @func ensListMiscellaneousfeatureSortEndDescending *************************
**
** Sort an AJAX List of Ensembl Miscellaneous Feature objects by their
** Ensembl Feature end coordinate in descending order.
**
** @param [u] mfs [AjPList] AJAX List of Ensembl Miscellaneous Feature objects
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
**
** @release 6.4.0
** @@
******************************************************************************/

AjBool ensListMiscellaneousfeatureSortEndDescending(AjPList mfs)
{
    if (!mfs)
        return ajFalse;

    ajListSortTwoThree(mfs,
                       &listMiscellaneousfeatureCompareEndDescending,
                       &listMiscellaneousfeatureCompareStartDescending,
                       &listMiscellaneousfeatureCompareIdentifierAscending);

    return ajTrue;
}




/* @func ensListMiscellaneousfeatureSortIdentifierAscending *******************
**
** Sort an AJAX List of Ensembl Miscellaneous Feature objects by their
** identifier member in ascending order.
**
** @param [u] mfs [AjPList] AJAX List of Ensembl Miscellaneous Feature objects
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
**
** @release 6.4.0
** @@
******************************************************************************/

AjBool ensListMiscellaneousfeatureSortIdentifierAscending(AjPList mfs)
{
    if (!mfs)
        return ajFalse;

    ajListSort(mfs, &listMiscellaneousfeatureCompareIdentifierAscending);

    return ajTrue;
}




/* @func ensListMiscellaneousfeatureSortStartAscending ************************
**
** Sort an AJAX List of Ensembl Miscellaneous Feature objects by their
** Ensembl Feature start coordinate in ascending order.
**
** @param [u] mfs [AjPList] AJAX List of Ensembl Miscellaneous Feature objects
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
**
** @release 6.4.0
** @@
******************************************************************************/

AjBool ensListMiscellaneousfeatureSortStartAscending(AjPList mfs)
{
    if (!mfs)
        return ajFalse;

    ajListSortTwoThree(mfs,
                       &listMiscellaneousfeatureCompareStartAscending,
                       &listMiscellaneousfeatureCompareEndAscending,
                       &listMiscellaneousfeatureCompareIdentifierAscending);

    return ajTrue;
}




/* @func ensListMiscellaneousfeatureSortStartDescending ***********************
**
** Sort an AJAX List of Ensembl Miscellaneous Feature objects by their
** Ensembl Feature start coordinate in descending order.
**
** @param [u] mfs [AjPList] AJAX List of Ensembl Miscellaneous Feature objects
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
**
** @release 6.4.0
** @@
******************************************************************************/

AjBool ensListMiscellaneousfeatureSortStartDescending(AjPList mfs)
{
    if (!mfs)
        return ajFalse;

    ajListSortTwoThree(mfs,
                       &listMiscellaneousfeatureCompareStartDescending,
                       &listMiscellaneousfeatureCompareEndDescending,
                       &listMiscellaneousfeatureCompareIdentifierAscending);

    return ajTrue;
}




/* @datasection [EnsPMiscellaneousfeatureadaptor] Ensembl Miscellaneous Feature
** Adaptor
**
** @nam2rule Miscellaneousfeatureadaptor Functions for manipulating
** Ensembl Miscellaneous Feature Adaptor objects
**
** @cc Bio::EnsEMBL::DBSQL::MiscFeatureAdaptor
** @cc CVS Revision: 1.26
** @cc CVS Tag: branch-ensembl-66
**
******************************************************************************/




/* @funcstatic miscellaneousfeatureadaptorHasAttribute ************************
**
** Check whether an Ensembl Attribute has already been indexed for a particular
** Ensembl Miscellaneous Feature.
**
** @param [u] attributes [AjPTable] AJAX Table of AJAX String key data
** (code:value pairs) and AJAX Boolean value data
** @param [r] atid [ajuint] Ensembl Attribute Type identifier
** @param [r] value [const AjPStr] Ensembl Attribute value
**
** @return [AjBool] ajTrue: A particular code:value String has already
**                          been indexed.
**                  ajFalse: A particular code:value String has not been
**                           indexed before, but has been added now.
**
** @release 6.3.0
** @@
** This function keeps an AJAX Table of Attribute code:value String key
** data and AjBool value data.
******************************************************************************/

static AjBool miscellaneousfeatureadaptorHasAttribute(AjPTable attributes,
                                                      ajuint atid,
                                                      const AjPStr value)
{
    AjBool *Pbool = NULL;

    AjPStr key = NULL;

    key = ajFmtStr("%u:%S", atid, value);

    if (ajTableMatchS(attributes, key))
    {
        ajStrDel(&key);

        return ajTrue;
    }
    else
    {
        AJNEW0(Pbool);

        *Pbool = ajTrue;

        ajTablePut(attributes, (void *) key, (void *) Pbool);

        return ajFalse;
    }
}




/* @funcstatic miscellaneousfeatureadaptorHasMiscellaneousset *****************
**
** Check whether an Ensembl Miscellaneous Set has already been indexed for a
** particular Ensembl Miscellaneous Feature.
**
** @param [u] sets [AjPTable] AJAX Table of AJAX unsigned integers
** (Ensembl Miscellaneous Set identifiers)
** @param [r] msid [ajuint] Ensembl Miscellaneous Set identifier
**
** @return [AjBool] ajTrue: A particular Ensembl Miscellaneous Set identifier
**                          has already been indexed.
**                  ajFalse: A particular Ensembl Miscellaneous Set identifier
**                           has not been indexed before, but has been added
**                           now.
**
** @release 6.3.0
** @@
** This function keeps an AJAX Table of Miscellaneous Set identifier unsigned
** integer key data and AjBool value data.
******************************************************************************/

static AjBool miscellaneousfeatureadaptorHasMiscellaneousset(AjPTable sets,
                                                             ajuint msid)
{
    AjBool *Pbool = NULL;

    ajuint *Pidentifier = NULL;

    if (ajTableMatchV(sets, (const void *) &msid))
        return ajTrue;

    AJNEW0(Pidentifier);

    *Pidentifier = msid;

    AJNEW0(Pbool);

    *Pbool = ajTrue;

    ajTablePut(sets, (void *) Pidentifier, (void *) Pbool);

    return ajFalse;
}




/* @funcstatic miscellaneousfeatureadaptorFetchAllbyStatement *****************
**
** Fetch all Ensembl Miscellaneous Feature objects via an SQL statement.
**
** @cc Bio::EnsEMBL::DBSQL::MiscFeatureAdaptor::_objs_from_sth
** @param [u] ba [EnsPBaseadaptor] Ensembl Base Adaptor
** @param [r] statement [const AjPStr] SQL statement
** @param [uN] am [EnsPAssemblymapper] Ensembl Assembly Mapper
** @param [uN] slice [EnsPSlice] Ensembl Slice
** @param [u] mfs [AjPList] AJAX List of Ensembl Miscellaneous Feature objects
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
**
** @release 6.4.0
** @@
******************************************************************************/

static AjBool miscellaneousfeatureadaptorFetchAllbyStatement(
    EnsPBaseadaptor ba,
    const AjPStr statement,
    EnsPAssemblymapper am,
    EnsPSlice slice,
    AjPList mfs)
{
    ajuint identifier = 0U;
    ajuint atid       = 0U;
    ajuint msid       = 0U;

    ajuint srid     = 0U;
    ajuint srstart  = 0U;
    ajuint srend    = 0U;
    ajint  srstrand = 0;

    ajuint current = 0U;
    ajuint throw = 0U;

    AjPSqlstatement sqls = NULL;
    AjISqlrow sqli       = NULL;
    AjPSqlrow sqlr       = NULL;

    AjPStr avalue = NULL;

    AjPTable attributes = NULL;
    AjPTable sets       = NULL;

    EnsPAttribute attribute = NULL;

    EnsPAttributetype        at  = NULL;
    EnsPAttributetypeadaptor ata = NULL;

    EnsPDatabaseadaptor dba = NULL;

    EnsPFeature feature = NULL;

    EnsPMiscellaneousfeature mf         = NULL;
    EnsPMiscellaneousfeatureadaptor mfa = NULL;

    EnsPMiscellaneousset ms         = NULL;
    EnsPMiscellaneoussetadaptor msa = NULL;

    if (ajDebugTest("miscellaneousfeatureadaptorFetchAllbyStatement"))
        ajDebug("miscellaneousfeatureadaptorFetchAllbyStatement\n"
                "  ba %p\n"
                "  statement %p\n"
                "  am %p\n"
                "  slice %p\n"
                "  mfs %p\n",
                ba,
                statement,
                am,
                slice,
                mfs);

    if (!ba)
        return ajFalse;

    if (!statement)
        return ajFalse;

    if (!mfs)
        return ajFalse;

    dba = ensBaseadaptorGetDatabaseadaptor(ba);

    ata = ensRegistryGetAttributetypeadaptor(dba);
    mfa = ensRegistryGetMiscellaneousfeatureadaptor(dba);
    msa = ensRegistryGetMiscellaneoussetadaptor(dba);

    attributes = ajTablestrNew(0);

    ajTableSetDestroyvalue(attributes, &ajMemFree);

    sets = ajTableuintNew(0);

    ajTableSetDestroyvalue(sets, &ajMemFree);

    sqls = ensDatabaseadaptorSqlstatementNew(dba, statement);

    sqli = ajSqlrowiterNew(sqls);

    while (!ajSqlrowiterDone(sqli))
    {
        identifier = 0U;
        srid       = 0U;
        srstart    = 0U;
        srend      = 0U;
        srstrand   = 0;
        atid       = 0U;
        avalue     = ajStrNew();
        msid       = 0U;

        sqlr = ajSqlrowiterGet(sqli);

        ajSqlcolumnToUint(sqlr, &identifier);
        ajSqlcolumnToUint(sqlr, &srid);
        ajSqlcolumnToUint(sqlr, &srstart);
        ajSqlcolumnToUint(sqlr, &srend);
        ajSqlcolumnToInt(sqlr, &srstrand);
        ajSqlcolumnToUint(sqlr, &atid);
        ajSqlcolumnToStr(sqlr, &avalue);
        ajSqlcolumnToUint(sqlr, &msid);

        if (identifier == throw)
        {
            /*
            ** This Miscellanous Feature is already known to map into a gap,
            ** therefore throw it away immediately.
            */

            ajStrDel(&avalue);

            continue;
        }

        if (identifier != current)
        {
            /*
            ** This is a new Miscellaneous Feature, therefore clear all
            ** internal caches.
            */

            ajTableClearDelete(attributes);
            ajTableClearDelete(sets);

            mf      = NULL;
            current = identifier;
            throw   = 0U;

            ensBaseadaptorRetrieveFeature(ba,
                                          0U,
                                          srid,
                                          srstart,
                                          srend,
                                          srstrand,
                                          am,
                                          slice,
                                          &feature);

            if (!feature)
            {
                throw = identifier;

                ajStrDel(&avalue);

                continue;
            }

            /* Finally, create a new Ensembl Miscellaneous Feature. */

            mf = ensMiscellaneousfeatureNewIni(mfa, identifier, feature);

            ensFeatureDel(&feature);

            ajListPushAppend(mfs, (void *) mf);
        }

        /* Add an Ensembl Attribute if one was defined. */

        if ((atid > 0) &&
            (ajStrGetLen(avalue) > 0) &&
            !miscellaneousfeatureadaptorHasAttribute(attributes, atid, avalue))
        {
            ensAttributetypeadaptorFetchByIdentifier(ata, atid, &at);

            attribute = ensAttributeNewIni(at, avalue);

            ensAttributetypeDel(&at);

            ensMiscellaneousfeatureAddAttribute(mf, attribute);

            ensAttributeDel(&attribute);
        }

        /* Add an Ensembl Miscellaneous Set if one was defined. */

        if ((msid > 0) &&
            !miscellaneousfeatureadaptorHasMiscellaneousset(sets, msid))
        {
            ensMiscellaneoussetadaptorFetchByIdentifier(msa, msid, &ms);

            ensMiscellaneousfeatureAddMiscellaneousset(mf, ms);

            ensMiscellaneoussetDel(&ms);
        }

        ajStrDel(&avalue);
    }

    ajSqlrowiterDel(&sqli);

    ensDatabaseadaptorSqlstatementDel(dba, &sqls);

    ajTableDel(&attributes);
    ajTableDel(&sets);

    return ajTrue;
}




/* @section constructors ******************************************************
**
** All constructors return a new Ensembl Miscellaneous Feature Adaptor by
** pointer.
** It is the responsibility of the user to first destroy any previous
** Miscellaneous Feature Adaptor. The target pointer does not need to be
** initialised to NULL, but it is good programming practice to do so anyway.
**
** @fdata [EnsPMiscellaneousfeatureadaptor]
**
** @nam3rule New Constructor
**
** @argrule New dba [EnsPDatabaseadaptor] Ensembl Database Adaptor
**
** @valrule * [EnsPMiscellaneousfeatureadaptor]
** Ensembl Miscellaneous Feature Adaptor or NULL
**
** @fcategory new
******************************************************************************/




/* @func ensMiscellaneousfeatureadaptorNew ************************************
**
** Default constructor for an Ensembl Miscellaneous Feature Adaptor.
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
** @see ensRegistryGetMiscellaneousfeatureadaptor
**
** @cc Bio::EnsEMBL::DBSQL::MiscSetAdaptor::new
** @cc Bio::EnsEMBL::DBSQL::MiscFeatureAdaptor::new
** @param [u] dba [EnsPDatabaseadaptor] Ensembl Database Adaptor
**
** @return [EnsPMiscellaneousfeatureadaptor]
** Ensembl Miscellaneous Feature Adaptor or NULL
**
** @release 6.2.0
** @@
******************************************************************************/

EnsPMiscellaneousfeatureadaptor ensMiscellaneousfeatureadaptorNew(
    EnsPDatabaseadaptor dba)
{
    if (!dba)
        return NULL;

    return ensFeatureadaptorNew(
        dba,
        miscellaneousfeatureadaptorKTables,
        miscellaneousfeatureadaptorKColumns,
        miscellaneousfeatureadaptorKLeftjoin,
        (const char *) NULL,
        miscellaneousfeatureadaptorKFinalcondition,
        &miscellaneousfeatureadaptorFetchAllbyStatement,
        (void *(*)(const void *)) NULL,
        (void *(*)(void *)) &ensMiscellaneousfeatureNewRef,
        (AjBool (*)(const void *)) NULL,
        (void (*)(void **)) &ensMiscellaneousfeatureDel,
        (size_t (*)(const void *)) &ensMiscellaneousfeatureCalculateMemsize,
        (EnsPFeature (*)(const void *)) &ensMiscellaneousfeatureGetFeature,
        "Miscellaneous Feature");
}




/* @section destructors *******************************************************
**
** Destruction destroys all internal data structures and frees the memory
** allocated for an Ensembl Miscellaneous Set Adaptor object.
**
** @fdata [EnsPMiscellaneousfeatureadaptor]
**
** @nam3rule Del Destroy (free) an Ensembl Miscellaneous Feature Adaptor
**
** @argrule * Pmfa [EnsPMiscellaneousfeatureadaptor*]
** Ensembl Miscellaneous Feature Adaptor address
**
** @valrule * [void]
**
** @fcategory delete
******************************************************************************/




/* @func ensMiscellaneousfeatureadaptorDel ************************************
**
** Default destructor for an Ensembl Miscellaneous Feature Adaptor.
**
** Ensembl Object Adaptors are singleton objects that are registered in the
** Ensembl Registry and weakly referenced by Ensembl Objects that have been
** instantiated by it. Therefore, Ensembl Object Adaptors should never be
** destroyed directly. Upon exit, the Ensembl Registry will call this function
** if required.
**
** @param [d] Pmfa [EnsPMiscellaneousfeatureadaptor*]
** Ensembl Miscellaneous Feature Adaptor address
**
** @return [void]
**
** @release 6.2.0
** @@
******************************************************************************/

void ensMiscellaneousfeatureadaptorDel(
    EnsPMiscellaneousfeatureadaptor *Pmfa)
{
    ensFeatureadaptorDel(Pmfa);

	return;
}




/* @section member retrieval **************************************************
**
** Functions for returning members of an
** Ensembl Miscellaneous Feature Adaptor object.
**
** @fdata [EnsPMiscellaneousfeatureadaptor]
**
** @nam3rule Get Return Ensembl Miscellaneous Feature Adaptor attribute(s)
** @nam4rule Databaseadaptor Return the Ensembl Database Adaptor
**
** @argrule * mfa [EnsPMiscellaneousfeatureadaptor]
** Ensembl Miscellaneous Feature Adaptor
**
** @valrule Databaseadaptor [EnsPDatabaseadaptor]
** Ensembl Database Adaptor or NULL
**
** @fcategory use
******************************************************************************/




/* @func ensMiscellaneousfeatureadaptorGetDatabaseadaptor *********************
**
** Get the Ensembl Database Adaptor member of an
** Ensembl Miscellaneous Feature Adaptor.
**
** @param [u] mfa [EnsPMiscellaneousfeatureadaptor]
** Ensembl Miscellaneous Feature Adaptor
**
** @return [EnsPDatabaseadaptor] Ensembl Database Adaptor or NULL
**
** @release 6.4.0
** @@
******************************************************************************/

EnsPDatabaseadaptor ensMiscellaneousfeatureadaptorGetDatabaseadaptor(
    EnsPMiscellaneousfeatureadaptor mfa)
{
    return ensFeatureadaptorGetDatabaseadaptor(mfa);
}




/* @section object fetching ***************************************************
**
** Functions for fetching Ensembl Miscellaneous Feature objects from an
** Ensembl Core database.
**
** @fdata [EnsPMiscellaneousfeatureadaptor]
**
** @nam3rule Fetch Fetch Ensembl Miscellaneous Feature object(s)
** @nam4rule All   Fetch all Ensembl Miscellaneous Feature objects
** @nam4rule Allby Fetch all Ensembl Miscellaneous Feature objects
**                 matching a criterion
** @nam5rule Attributecodevalue Fetch all by Ensembl Attribute code and value
** @nam5rule Slicecodes Fetch all by an Ensembl Slice and an AJAX List of AJAX
** String (Ensembl Miscelleaneous Set codes)
** @nam4rule By    Fetch one Ensembl Miscellaneous Feature object
**                 matching a criterion
**
** @argrule * mfa [EnsPMiscellaneousfeatureadaptor]
** Ensembl Miscellaneous Feature Adaptor
** @argrule FetchAll mfs [AjPList]
** AJAX List of Ensembl Miscellaneous Feature objects
** @argrule AllbyAttributecodevalue code [const AjPStr]
** Ensembl Attribute code
** @argrule AllbyAttributecodevalue value [const AjPStr]
** Ensembl Attribute value
** @argrule AllbyAttributecodevalue mfs [AjPList]
** AJAX List of Ensembl Miscellaneous Feature objects
** @argrule AllbySlicecodes slice [EnsPSlice] Ensembl Slice
** @argrule AllbySlicecodes codes [const AjPList]
** AJAX List of AJAX String objects
** @argrule AllbySlicecodes mfs [AjPList]
** AJAX List of Ensembl Miscellaneous Feature objects
**
** @valrule * [AjBool] ajTrue upon success, ajFalse otherwise
**
** @fcategory use
******************************************************************************/




/* @func ensMiscellaneousfeatureadaptorFetchAllbyAttributecodevalue ***********
**
** Fetch all Ensembl Miscellaneous Feature objects by an Ensembl Slice and
** an Ensembl Attribute code and value.
**
** @cc Bio::EnsEMBL::DBSQL::MiscFeatureAdaptor::
**     fetch_all_by_attribute_type_value
** @param [u] mfa [EnsPMiscellaneousfeatureadaptor]
** Ensembl Miscellaneous Feature Adaptor
** @param [r] code [const AjPStr] Ensembl Attribute code
** @param [r] value [const AjPStr] Ensembl Attribute value
** @param [u] mfs [AjPList] AJAX List of Ensembl Miscellaneous Feature objects
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
**
** @release 6.4.0
** @@
******************************************************************************/

AjBool ensMiscellaneousfeatureadaptorFetchAllbyAttributecodevalue(
    EnsPMiscellaneousfeatureadaptor mfa,
    const AjPStr code,
    const AjPStr value,
    AjPList mfs)
{
    register ajuint i = 0U;

    ajuint mfid = 0U;

    char *txtvalue = NULL;

    const char *template = "misc_feature.misc_feature_id in (%S)";

    AjPSqlstatement sqls = NULL;
    AjISqlrow sqli       = NULL;
    AjPSqlrow sqlr       = NULL;

    AjPStr csv        = NULL;
    AjPStr constraint = NULL;
    AjPStr statement  = NULL;

    EnsPAttributetype        at  = NULL;
    EnsPAttributetypeadaptor ata = NULL;

    EnsPBaseadaptor ba = NULL;

    EnsPDatabaseadaptor dba = NULL;

    if (!mfa)
        return ajFalse;

    if (!code)
        ajFatal("ensMiscellaneousfeatureadaptorFetchAllbyAttributecodevalue "
                "requires an Ensembl Attribute code.\n");

    ba = ensFeatureadaptorGetBaseadaptor(mfa);

    dba = ensFeatureadaptorGetDatabaseadaptor(mfa);

    ata = ensRegistryGetAttributetypeadaptor(dba);

    ensAttributetypeadaptorFetchByCode(ata, code, &at);

    if (!at)
        return ajTrue;

    /*
    ** Need to do two queries so that all of the identifiers come back with
    ** the Feature objects. The problem with adding attrib constraints to
    ** filter the misc_features, which come back is that not all of the
    ** Attribute objects will come back
    */

    statement = ajFmtStr(
        "SELECT DISTINCT "
        "misc_attrib.misc_feature_id "
        "FROM "
        "misc_attrib, "
        "misc_feature, "
        "seq_region, "
        "coord_system "
        "WHERE "
        "misc_attrib.attrib_type_id = %u "
        "AND "
        "misc_attrib.misc_feature_id = misc_feature.misc_feature_id "
        "AND "
        "misc_feature.seq_region_id = seq_region.seq_region_id "
        "AND "
        "seq_region.coord_system_id = coord_system.coord_system_id "
        "AND "
        "coord_system.species_id = %u",
        ensAttributetypeGetIdentifier(at),
        ensDatabaseadaptorGetIdentifier(dba));

    if (value)
    {
        ensDatabaseadaptorEscapeC(dba, &txtvalue, value);

        ajFmtPrintAppS(&statement, " AND misc_attrib.value = '%s'", txtvalue);

        ajCharDel(&txtvalue);
    }

    sqls = ensDatabaseadaptorSqlstatementNew(dba, statement);

    sqli = ajSqlrowiterNew(sqls);

    csv = ajStrNew();

    /*
    ** Large queries are split into smaller ones on the basis of the maximum
    ** number of identifiers configured in the Ensembl Base Adaptor module.
    ** This ensures that MySQL is faster and the maximum query size is not
    ** exceeded.
    */

    while (!ajSqlrowiterDone(sqli))
    {
        mfid = 0;

        sqlr = ajSqlrowiterGet(sqli);

        ajSqlcolumnToUint(sqlr, &mfid);

        ajFmtPrintAppS(&csv, "%u, ", mfid);

        /*
        ** Run the statement if the maximum chunk size is exceed or
        ** if there are no more AJAX List elements to process.
        */

        if ((((i + 1U) % ensKBaseadaptorMaximumIdentifiers) == 0) ||
            ajSqlrowiterDone(sqli))
        {
            /*
            ** Remove the last comma and space from the
            ** comma-separated values.
            */

            ajStrCutEnd(&csv, 2);

            if (ajStrGetLen(csv))
            {
                constraint = ajFmtStr(template, csv);

                ensBaseadaptorFetchAllbyConstraint(
                    ba,
                    constraint,
                    (EnsPAssemblymapper) NULL,
                    (EnsPSlice) NULL,
                    mfs);

                ajStrDel(&constraint);
            }

            ajStrSetClear(&csv);
        }

        i++;
    }

    ajSqlrowiterDel(&sqli);

    ensDatabaseadaptorSqlstatementDel(dba, &sqls);

    ajStrDel(&statement);

    ajStrDel(&csv);

    return ajTrue;
}




/* @func ensMiscellaneousfeatureadaptorFetchAllbySlicecodes *******************
**
** Fetch all Ensembl Miscellaneous Feature objects by an Ensembl Slice and
** Ensembl Miscellaneous Set codes.
**
** @cc Bio::EnsEMBL::DBSQL::MiscFeatureAdaptor::
**     fetch_all_by_Slice_and_set_code
** @param [u] mfa [EnsPMiscellaneousfeatureadaptor]
** Ensembl Miscellaneous Feature Adaptor
** @param [u] slice [EnsPSlice] Ensembl Slice
** @param [r] codes [const AjPList] AJAX List of AJAX String
** (Ensembl Miscellaneous Set code) objects
** @param [u] mfs [AjPList] AJAX List of Ensembl Miscellaneous Feature objects
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
**
** @release 6.4.0
** @@
******************************************************************************/

AjBool ensMiscellaneousfeatureadaptorFetchAllbySlicecodes(
    EnsPMiscellaneousfeatureadaptor mfa,
    EnsPSlice slice,
    const AjPList codes,
    AjPList mfs)
{
    ajuint maxlen = 0U;

    AjIList iter = NULL;

    AjPStr code       = NULL;
    AjPStr constraint = NULL;
    AjPStr csv        = NULL;

    EnsPDatabaseadaptor dba = NULL;

    EnsPMiscellaneousset ms         = NULL;
    EnsPMiscellaneoussetadaptor msa = NULL;

    if (!mfa)
        return ajFalse;

    if (!slice)
        return ajFalse;

    if (!codes)
        return ajFalse;

    if (!mfs)
        return ajFalse;

    dba = ensFeatureadaptorGetDatabaseadaptor(mfa);

    msa = ensRegistryGetMiscellaneoussetadaptor(dba);

    csv = ajStrNew();

    iter = ajListIterNewread(codes);

    while (!ajListIterDone(iter))
    {
        code = (AjPStr) ajListIterGet(iter);

        ensMiscellaneoussetadaptorFetchByCode(msa, code, &ms);

        if (ms)
        {
            maxlen = (ensMiscellaneoussetGetMaximumlength(ms) > maxlen) ?
                ensMiscellaneoussetGetMaximumlength(ms) : maxlen;

            ajFmtPrintAppS(&csv, "%u, ", ensMiscellaneoussetGetIdentifier(ms));

            ensMiscellaneoussetDel(&ms);
        }
        else
            ajWarn("ensMiscellaneousfeatureadaptorFetchAllbySlicecodes "
                   "no Miscellaneous Set with code '%S'.\n", code);
    }

    ajListIterDel(&iter);

    /* Remove the last comma and space from the comma-separated values. */

    ajStrCutEnd(&csv, 2);

    if (ajStrGetLen(csv) > 0)
    {
        constraint = ajFmtStr("misc_feature_misc_set.misc_set_id IN (%S)",
                              csv);

        ensFeatureadaptorSetMaximumlength(mfa, maxlen);

        ensFeatureadaptorFetchAllbySlice(mfa,
                                         slice,
                                         constraint,
                                         (const AjPStr) NULL,
                                         mfs);

        ensFeatureadaptorSetMaximumlength(mfa, 0);

        ajStrDel(&constraint);
    }

    ajStrDel(&csv);

    return ajTrue;
}




/* @section accessory object retrieval ****************************************
**
** Functions for fetching objects releated to Ensembl Miscellaneous Feature
** objects from an Ensembl SQL database.
**
** @fdata [EnsPMiscellaneousfeatureadaptor]
**
** @nam3rule Retrieve Retrieve Ensembl Miscellaneous Feature-releated object(s)
** @nam4rule All Retrieve all Ensembl Miscellaneous Feature-releated objects
** @nam5rule Identifiers Fetch all SQL database-internal identifiers
**
** @argrule * mfa [EnsPMiscellaneousfeatureadaptor]
** Ensembl Miscellaneous Feature Adaptor
** @argrule AllIdentifiers identifiers [AjPList]
** AJAX List of AJAX unsigned integer identifiers
**
** @valrule * [AjBool] ajTrue upon success, ajFalse otherwise
**
** @fcategory use
******************************************************************************/




/* @func ensMiscellaneousfeatureadaptorRetrieveAllIdentifiers *****************
**
** Fetch all SQL database-internal identifiers of
** Ensembl Miscellaneous Feature objects.
**
** The caller is responsible for deleting the AJAX unsigned integers before
** deleting the AJAX List.
**
** @cc Bio::EnsEMBL::DBSQL::MiscFeatureAdaptor::list_dbIDs
** @param [u] mfa [EnsPMiscellaneousfeatureadaptor]
** Ensembl Miscellaneous Feature Adaptor
** @param [u] identifiers [AjPList] AJAX List of AJAX unsigned integers
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
**
** @release 6.4.0
** @@
******************************************************************************/

AjBool ensMiscellaneousfeatureadaptorRetrieveAllIdentifiers(
    EnsPMiscellaneousfeatureadaptor mfa,
    AjPList identifiers)
{
    AjBool result = AJFALSE;

    AjPStr table = NULL;

    EnsPBaseadaptor ba = NULL;

    if (!mfa)
        return ajFalse;

    if (!identifiers)
        return ajFalse;

    ba = ensFeatureadaptorGetBaseadaptor(mfa);

    table = ajStrNewC("misc_feature");

    result = ensBaseadaptorRetrieveAllIdentifiers(ba,
                                                  table,
                                                  (AjPStr) NULL,
                                                  identifiers);

    ajStrDel(&table);

    return result;
}
