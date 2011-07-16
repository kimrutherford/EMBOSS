/* @source Ensembl Miscellaneous functions
**
** @author Copyright (C) 1999 Ensembl Developers
** @author Copyright (C) 2006 Michael K. Schuster
** @modified 2009 by Alan Bleasby for incorporation into EMBOSS core
** @modified $Date: 2011/07/06 21:57:35 $ by $Author: mks $
** @version $Revision: 1.42 $
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

#include "ensmiscellaneous.h"
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

/* @conststatic miscellaneoussetadaptorTables *********************************
**
** Array of Ensembl Miscellaneous Set Adaptor SQL table names
**
******************************************************************************/

static const char* const miscellaneoussetadaptorTables[] =
{
    "misc_set",
    (const char*) NULL
};




/* @conststatic miscellaneoussetadaptorColumns ********************************
**
** Array of Ensembl Miscellaneous Set Adaptor SQL column names
**
******************************************************************************/

static const char* const miscellaneoussetadaptorColumns[] =
{
    "misc_set.misc_set_id",
    "misc_set.code",
    "misc_set.name",
    "misc_set.description",
    "misc_set.max_length",
    (const char*) NULL
};




/* @conststatic miscellaneousfeatureadaptorTables *****************************
**
** Array of Ensembl Miscellaneous Feature Adaptor SQL table names
**
******************************************************************************/

static const char* const miscellaneousfeatureadaptorTables[] =
{
    "misc_feature",
    "misc_feature_misc_set",
    "misc_attrib",
    (const char*) NULL
};




/* @conststatic miscellaneousfeatureadaptorColumns ****************************
**
** Array of Ensembl Miscellaneous Feature Adaptor SQL column names
**
******************************************************************************/

static const char* const miscellaneousfeatureadaptorColumns[] =
{
    "misc_feature.misc_feature_id",
    "misc_feature.seq_region_id",
    "misc_feature.seq_region_start",
    "misc_feature.seq_region_end",
    "misc_feature.seq_region_strand",
    "misc_attrib.attrib_type_id",
    "misc_attrib.value",
    "misc_feature_misc_set.misc_set_id",
    (const char*) NULL
};




/* @conststatic miscellaneousfeatureadaptorLeftjoin ***************************
**
** Array of Ensembl Miscellaneous Feature Adaptor SQL left join conditions
**
******************************************************************************/

static EnsOBaseadaptorLeftjoin miscellaneousfeatureadaptorLeftjoin[] =
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




/* @conststatic miscellaneousfeatureadaptorFinalcondition *********************
**
** Ensembl Miscellaneous Feature Adaptor SQL final conditions
**
******************************************************************************/

static const char* miscellaneousfeatureadaptorFinalcondition =
    " ORDER BY misc_feature.misc_feature_id";




/* ==================================================================== */
/* ======================== private variables ========================= */
/* ==================================================================== */




/* ==================================================================== */
/* ======================== private functions ========================= */
/* ==================================================================== */

static AjBool miscellaneoussetadaptorFetchAllbyStatement(
    EnsPDatabaseadaptor dba,
    const AjPStr statement,
    EnsPAssemblymapper am,
    EnsPSlice slice,
    AjPList mss);

static AjBool miscellaneoussetadaptorCacheInit(
    EnsPMiscellaneoussetadaptor msa);

static AjBool miscellaneoussetadaptorCacheInsert(
    EnsPMiscellaneoussetadaptor msa,
    EnsPMiscellaneousset* Pms);

#if AJFALSE
static AjBool miscellaneoussetadaptorCacheRemove(
    EnsPMiscellaneoussetadaptor msa,
    EnsPMiscellaneousset ms);
#endif

static void miscellaneoussetadaptorCacheClearIdentifier(void** key,
                                                        void** value,
                                                        void* cl);

static void miscellaneoussetadaptorCacheClearCode(void** key,
                                                  void** value,
                                                  void* cl);

static void miscellaneoussetadaptorFetchAll(const void* key,
                                            void** value,
                                            void* cl);

static int listMiscellaneousfeatureCompareStartAscending(const void* P1,
                                                         const void* P2);

static int listMiscellaneousfeatureCompareStartDescending(const void* P1,
                                                          const void* P2);

static AjBool miscellaneousfeatureadaptorHasAttribute(AjPTable attributes,
                                                      ajuint atid,
                                                      const AjPStr value);

static void miscellaneousfeatureadaptorClearAttributes(void** key,
                                                       void** value,
                                                       void* cl);

static AjBool miscellaneousfeatureadaptorHasMiscellaneousset(AjPTable sets,
                                                             ajuint msid);

static void miscellaneousfeatureadaptorClearMiscellaneoussets(void** key,
                                                              void** value,
                                                              void* cl);

static AjBool miscellaneousfeatureadaptorFetchAllbyStatement(
    EnsPDatabaseadaptor dba,
    const AjPStr statement,
    EnsPAssemblymapper am,
    EnsPSlice slice,
    AjPList mfs);

static void* miscellaneousfeatureadaptorCacheReference(void* value);

static void miscellaneousfeatureadaptorCacheDelete(void** value);

static size_t miscellaneousfeatureadaptorCacheSize(const void* value);

static EnsPFeature miscellaneousfeatureadaptorGetFeature(const void* value);




/* ==================================================================== */
/* ===================== All functions by section ===================== */
/* ==================================================================== */




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
** @cc CVS Revision: 1.9
** @cc CVS Tag: branch-ensembl-62
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
** @@
******************************************************************************/

EnsPMiscellaneousset ensMiscellaneoussetNewCpy(
    const EnsPMiscellaneousset ms)
{
    EnsPMiscellaneousset pthis = NULL;

    if(!ms)
        return NULL;

    AJNEW0(pthis);

    pthis->Use = 1;

    pthis->Identifier = ms->Identifier;

    pthis->Adaptor = ms->Adaptor;

    if(ms->Code)
        pthis->Code = ajStrNewRef(ms->Code);

    if(ms->Name)
        pthis->Name = ajStrNewRef(ms->Name);

    if(ms->Description)
        pthis->Description = ajStrNewRef(ms->Description);

    pthis->MaximumLength = ms->MaximumLength;

    return pthis;
}




/* @func ensMiscellaneoussetNewIni ********************************************
**
** Constructor for an Ensembl Miscellaneous Set with initial values.
**
** @cc Bio::EnsEMBL::Storable::new
** @param [u] msa [EnsPMiscellaneoussetadaptor] Ensembl Miscellaneous
**                                              Set Adaptor
** @param [r] identifier [ajuint] SQL database-internal identifier
** @cc Bio::EnsEMBL::MiscSet::new
** @param [u] code [AjPStr] Code
** @param [u] name [AjPStr] Name
** @param [u] description [AjPStr] Description
** @param [r] maxlen [ajuint] Maximum length
**
** @return [EnsPMiscellaneousset] Ensembl Miscellaneous Set or NULL
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

    ms->Use = 1;

    ms->Identifier = identifier;

    ms->Adaptor = msa;

    if(code)
        ms->Code = ajStrNewRef(code);

    if(name)
        ms->Name = ajStrNewRef(name);

    if(description)
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
** @return [EnsPMiscellaneousset] Ensembl Miscellaneous Set
** @@
******************************************************************************/

EnsPMiscellaneousset ensMiscellaneoussetNewRef(EnsPMiscellaneousset ms)
{
    if(!ms)
        return NULL;

    ms->Use++;

    return ms;
}




/* @section destructors *******************************************************
**
** Destruction destroys all internal data structures and frees the
** memory allocated for an Ensembl Miscellaneous Set object.
**
** @fdata [EnsPMiscellaneousset]
**
** @nam3rule Del Destroy (free) an Ensembl Miscellaneous Set object
**
** @argrule * Pms [EnsPMiscellaneousset*] Ensembl Miscellaneous Set
**                                        object address
**
** @valrule * [void]
**
** @fcategory delete
******************************************************************************/




/* @func ensMiscellaneoussetDel ***********************************************
**
** Default destructor for an Ensembl Miscellaneous Set.
**
** @param [d] Pms [EnsPMiscellaneousset*] Ensembl Miscellaneous Set
**                                        object address
**
** @return [void]
** @@
******************************************************************************/

void ensMiscellaneoussetDel(EnsPMiscellaneousset* Pms)
{
    EnsPMiscellaneousset pthis = NULL;

    if(!Pms)
        return;

    if(!*Pms)
        return;

    if(ajDebugTest("ensMiscellaneoussetDel"))
    {
        ajDebug("ensMiscellaneoussetDel\n"
                "  *Pms %p\n",
                *Pms);

        ensMiscellaneoussetTrace(*Pms, 1);
    }

    pthis = *Pms;

    pthis->Use--;

    if(pthis->Use)
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




/* @section element retrieval *************************************************
**
** Functions for returning elements of an Ensembl Miscellaneous Set object.
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
** @valrule Adaptor [EnsPMiscellaneoussetadaptor] Ensembl Miscellaneous
**                                                Set Adaptor or NULL
** @valrule Code [AjPStr] Code or NULL
** @valrule Description [AjPStr] Description or NULL
** @valrule Identifier [ajuint] SQL database-internal identifier or 0
** @valrule Maximumlength [ajuint] Maximum length or 0
** @valrule Name [AjPStr] Name or NULL
**
** @fcategory use
******************************************************************************/




/* @func ensMiscellaneoussetGetAdaptor ****************************************
**
** Get the Ensembl Miscellaneous Set Adaptor element of an
** Ensembl Miscellaneous Set.
**
** @cc Bio::EnsEMBL::Storable::adaptor
** @param [r] ms [const EnsPMiscellaneousset] Ensembl Miscellaneous Set
**
** @return [EnsPMiscellaneoussetadaptor] Ensembl Miscellaneous Set Adaptor
** or NULL
** @@
******************************************************************************/

EnsPMiscellaneoussetadaptor ensMiscellaneoussetGetAdaptor(
    const EnsPMiscellaneousset ms)
{
    if(!ms)
        return NULL;

    return ms->Adaptor;
}




/* @func ensMiscellaneoussetGetCode *******************************************
**
** Get the code element of an Ensembl Miscellaneous Set.
**
** @cc Bio::EnsEMBL::MiscSet::code
** @param [r] ms [const EnsPMiscellaneousset] Ensembl Miscellaneous Set
**
** @return [AjPStr] Code or NULL
** @@
******************************************************************************/

AjPStr ensMiscellaneoussetGetCode(const EnsPMiscellaneousset ms)
{
    if(!ms)
        return NULL;

    return ms->Code;
}




/* @func ensMiscellaneoussetGetDescription ************************************
**
** Get the description element of an Ensembl Miscellaneous Set.
**
** @cc Bio::EnsEMBL::MiscSet::description
** @param [r] ms [const EnsPMiscellaneousset] Ensembl Miscellaneous Set
**
** @return [AjPStr] Description or NULL
** @@
******************************************************************************/

AjPStr ensMiscellaneoussetGetDescription(const EnsPMiscellaneousset ms)
{
    if(!ms)
        return NULL;

    return ms->Description;
}




/* @func ensMiscellaneoussetGetIdentifier *************************************
**
** Get the SQL database-internal identifier element of an
** Ensembl Miscellaneous Set.
**
** @cc Bio::EnsEMBL::Storable::dbID
** @param [r] ms [const EnsPMiscellaneousset] Ensembl Miscellaneous Set
**
** @return [ajuint] SQL database-internal identifier or 0
** @@
******************************************************************************/

ajuint ensMiscellaneoussetGetIdentifier(const EnsPMiscellaneousset ms)
{
    if(!ms)
        return 0;

    return ms->Identifier;
}




/* @func ensMiscellaneoussetGetMaximumlength **********************************
**
** Get the maximum length element of an Ensembl Miscellaneous Set.
**
** @cc Bio::EnsEMBL::MiscSet::longest_feature
** @param [r] ms [const EnsPMiscellaneousset] Ensembl Miscellaneous Set
**
** @return [ajuint] Maximum length
** @@
******************************************************************************/

ajuint ensMiscellaneoussetGetMaximumlength(const EnsPMiscellaneousset ms)
{
    if(!ms)
        return 0;

    return ms->MaximumLength;
}




/* @func ensMiscellaneoussetGetName *******************************************
**
** Get the name element of an Ensembl Miscellaneous Set.
**
** @cc Bio::EnsEMBL::MiscSet::name
** @param [r] ms [const EnsPMiscellaneousset] Ensembl Miscellaneous Set
**
** @return [AjPStr] Name or NULL
** @@
******************************************************************************/

AjPStr ensMiscellaneoussetGetName(const EnsPMiscellaneousset ms)
{
    if(!ms)
        return NULL;

    return ms->Name;
}




/* @section modifiers *********************************************************
**
** Functions for assigning elements of an Ensembl Miscellaneous Set object.
**
** @fdata [EnsPMiscellaneousset]
**
** @nam3rule Set Set one element of an Ensembl Miscellaneous Set
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
** Set the Ensembl Miscellaneous Set Adaptor element of an
** Ensembl Miscellaneous Set.
**
** @cc Bio::EnsEMBL::Storable::adaptor
** @param [u] ms [EnsPMiscellaneousset] Ensembl Miscellaneous Set
** @param [u] msa [EnsPMiscellaneoussetadaptor] Ensembl Miscellaneous
**                                              Set Adaptor
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
** @@
******************************************************************************/

AjBool ensMiscellaneoussetSetAdaptor(EnsPMiscellaneousset ms,
                                     EnsPMiscellaneoussetadaptor msa)
{
    if(!ms)
        return ajFalse;

    ms->Adaptor = msa;

    return ajTrue;
}




/* @func ensMiscellaneoussetSetCode *******************************************
**
** Set the code element of an Ensembl Miscellaneous Set.
**
** @cc Bio::EnsEMBL::MiscSet::code
** @param [u] ms [EnsPMiscellaneousset] Ensembl Miscellaneous Set
** @param [u] code [AjPStr] Code
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise.
** @@
******************************************************************************/

AjBool ensMiscellaneoussetSetCode(EnsPMiscellaneousset ms, AjPStr code)
{
    if(!ms)
        return ajFalse;

    ajStrDel(&ms->Code);

    ms->Code = ajStrNewRef(code);

    return ajTrue;
}




/* @func ensMiscellaneoussetSetDescription ************************************
**
** Set the description element of an Ensembl Miscellaneous Set.
**
** @cc Bio::EnsEMBL::MiscSet::description
** @param [u] ms [EnsPMiscellaneousset] Ensembl Miscellaneous Set
** @param [u] description [AjPStr] Description
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
** @@
******************************************************************************/

AjBool ensMiscellaneoussetSetDescription(EnsPMiscellaneousset ms,
                                         AjPStr description)
{
    if(!ms)
        return ajFalse;

    ajStrDel(&ms->Description);

    ms->Description = ajStrNewRef(description);

    return ajTrue;
}




/* @func ensMiscellaneoussetSetIdentifier *************************************
**
** Set the SQL database-internal identifier element of an
** Ensembl Miscellaneous Set.
**
** @cc Bio::EnsEMBL::Storable::dbID
** @param [u] ms [EnsPMiscellaneousset] Ensembl Miscellaneous Set
** @param [r] identifier [ajuint] SQL database-internal identifier
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
** @@
******************************************************************************/

AjBool ensMiscellaneoussetSetIdentifier(EnsPMiscellaneousset ms,
                                        ajuint identifier)
{
    if(!ms)
        return ajFalse;

    ms->Identifier = identifier;

    return ajTrue;
}




/* @func ensMiscellaneoussetSetMaximumlength **********************************
**
** Set the maximum length element of an Ensembl Miscellaneous Set.
**
** @cc Bio::EnsEMBL::MiscSet::longest_feature
** @param [u] ms [EnsPMiscellaneousset] Ensembl Miscellaneous Set
** @param [r] maxlen [ajuint] Maximum length
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
** @@
******************************************************************************/

AjBool ensMiscellaneoussetSetMaximumlength(EnsPMiscellaneousset ms,
                                           ajuint maxlen)
{
    if(!ms)
        return ajFalse;

    ms->MaximumLength = maxlen;

    return ajTrue;
}




/* @func ensMiscellaneoussetSetName *******************************************
**
** Set the name element of an Ensembl Miscellaneous Set.
**
** @cc Bio::EnsEMBL::MiscSet::name
** @param [u] ms [EnsPMiscellaneousset] Ensembl Miscellaneous Set
** @param [u] name [AjPStr] Name
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
** @@
******************************************************************************/

AjBool ensMiscellaneoussetSetName(EnsPMiscellaneousset ms, AjPStr name)
{
    if(!ms)
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
** @nam3rule Trace Report Ensembl Miscellaneous Set elements to debug file
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
** @@
******************************************************************************/

AjBool ensMiscellaneoussetTrace(const EnsPMiscellaneousset ms, ajuint level)
{
    AjPStr indent = NULL;

    if(!ms)
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
** @@
******************************************************************************/

size_t ensMiscellaneoussetCalculateMemsize(const EnsPMiscellaneousset ms)
{
    size_t size = 0;

    if(!ms)
        return 0;

    size += sizeof (EnsOMiscellaneousset);

    if(ms->Code)
    {
        size += sizeof (AjOStr);

        size += ajStrGetRes(ms->Code);
    }

    if(ms->Name)
    {
        size += sizeof (AjOStr);

        size += ajStrGetRes(ms->Name);
    }

    if(ms->Description)
    {
        size += sizeof (AjOStr);

        size += ajStrGetRes(ms->Description);
    }

    return size;
}




/* @datasection [EnsPMiscellaneoussetadaptor] Ensembl Miscellaneous Set Adaptor
**
** @nam2rule Miscellaneoussetadaptor Functions for manipulating
** Ensembl Miscellaneous Set Adaptor objects
**
** @cc Bio::EnsEMBL::DBSQL::MiscSetAdaptor
** @cc CVS Revision: 1.15
** @cc CVS Tag: branch-ensembl-62
**
******************************************************************************/




/* @funcstatic miscellaneoussetadaptorFetchAllbyStatement *********************
**
** Run a SQL statement against an Ensembl Database Adaptor and consolidate the
** results into an AJAX List of Ensembl Miscellaneous Set objects.
**
** @param [u] dba [EnsPDatabaseadaptor] Ensembl Database Adaptor
** @param [r] statement [const AjPStr] SQL statement
** @param [uN] am [EnsPAssemblymapper] Ensembl Assembly Mapper
** @param [uN] slice [EnsPSlice] Ensembl Slice
** @param [u] mss [AjPList] AJAX List of Ensembl Miscellaneous Sets
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
** @@
******************************************************************************/

static AjBool miscellaneoussetadaptorFetchAllbyStatement(
    EnsPDatabaseadaptor dba,
    const AjPStr statement,
    EnsPAssemblymapper am,
    EnsPSlice slice,
    AjPList mss)
{
    ajuint identifier = 0;
    ajuint maxlen     = 0;

    AjPSqlstatement sqls = NULL;
    AjISqlrow sqli       = NULL;
    AjPSqlrow sqlr       = NULL;

    AjPStr code        = NULL;
    AjPStr name        = NULL;
    AjPStr description = NULL;

    EnsPMiscellaneousset        ms  = NULL;
    EnsPMiscellaneoussetadaptor msa = NULL;

    if(ajDebugTest("miscellaneoussetadaptorFetchAllbyStatement"))
        ajDebug("miscellaneoussetadaptorFetchAllbyStatement\n"
                "  dba %p\n"
                "  statement %p\n"
                "  am %p\n"
                "  slice %p\n"
                "  mss %p\n",
                dba,
                statement,
                am,
                slice,
                mss);

    if(!dba)
        if(!msa)
            return ajFalse;

    if(!statement)
        return ajFalse;

    if(!mss)
        return ajFalse;

    msa = ensRegistryGetMiscellaneoussetadaptor(dba);

    sqls = ensDatabaseadaptorSqlstatementNew(dba, statement);

    sqli = ajSqlrowiterNew(sqls);

    while(!ajSqlrowiterDone(sqli))
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

        ajListPushAppend(mss, (void*) ms);

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
** @valrule * [EnsPMiscellaneoussetadaptor] Ensembl Miscellaneous Set Adaptor
**                                          or NULL
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
** @return [EnsPMiscellaneoussetadaptor] Ensembl Miscellaneous Set Adaptor
**                                       or NULL
** @@
******************************************************************************/

EnsPMiscellaneoussetadaptor ensMiscellaneoussetadaptorNew(
    EnsPDatabaseadaptor dba)
{
    EnsPMiscellaneoussetadaptor msa = NULL;

    if(!dba)
        return NULL;

    AJNEW0(msa);

    msa->Adaptor = ensBaseadaptorNew(
        dba,
        miscellaneoussetadaptorTables,
        miscellaneoussetadaptorColumns,
        (EnsPBaseadaptorLeftjoin) NULL,
        (const char*) NULL,
        (const char*) NULL,
        miscellaneoussetadaptorFetchAllbyStatement);

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
** @argrule * msa [EnsPMiscellaneoussetadaptor] Ensembl Miscellaneous Set
**                                              Adaptor
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
** @param [u] msa [EnsPMiscellaneoussetadaptor] Ensembl Miscellaneous
**                                              Set Adaptor
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
** @@
******************************************************************************/

static AjBool miscellaneoussetadaptorCacheInit(
    EnsPMiscellaneoussetadaptor msa)
{
    AjPList mss      = NULL;

    EnsPMiscellaneousset ms = NULL;

    if(!msa)
        return ajFalse;

    if(msa->CacheByIdentifier)
        return ajFalse;
    else
        msa->CacheByIdentifier = ensTableuintNewLen(0);

    if(msa->CacheByCode)
        return ajFalse;
    else
        msa->CacheByCode = ensTablestrNewLen(0);

    mss = ajListNew();

    ensBaseadaptorFetchAllbyConstraint(msa->Adaptor,
                                       (const AjPStr) NULL,
                                       (EnsPAssemblymapper) NULL,
                                       (EnsPSlice) NULL,
                                       mss);

    while(ajListPop(mss, (void**) &ms))
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
** If a Miscellaneous Set with the same code element is already present in the
** adaptor cache, the Miscellaneous Set is deleted and a pointer to the cached
** Miscellaneous Set is returned.
**
** @param [u] msa [EnsPMiscellaneoussetadaptor] Ensembl Miscellaneous
**                                              Set Adaptor
** @param [u] Pms [EnsPMiscellaneousset*] Ensembl Miscellaneous Set address
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
** @@
******************************************************************************/

static AjBool miscellaneoussetadaptorCacheInsert(
    EnsPMiscellaneoussetadaptor msa,
    EnsPMiscellaneousset* Pms)
{
    ajuint* Pidentifier = NULL;

    EnsPMiscellaneousset ms1 = NULL;
    EnsPMiscellaneousset ms2 = NULL;

    if(!msa)
        return ajFalse;

    if(!msa->CacheByIdentifier)
        return ajFalse;

    if(!msa->CacheByCode)
        return ajFalse;

    if(!Pms)
        return ajFalse;

    if(!*Pms)
        return ajFalse;

    /* Search the identifer cache. */

    ms1 = (EnsPMiscellaneousset) ajTableFetchmodV(
        msa->CacheByIdentifier,
        (const void*) &((*Pms)->Identifier));

    /* Search the code cache. */

    ms2 = (EnsPMiscellaneousset) ajTableFetchmodV(
        msa->CacheByCode,
        (const void*) (*Pms)->Code);

    if((!ms1) && (!ms2))
    {
        /* Insert into the identifier cache. */

        AJNEW0(Pidentifier);

        *Pidentifier = (*Pms)->Identifier;

        ajTablePut(msa->CacheByIdentifier,
                   (void*) Pidentifier,
                   (void*) ensMiscellaneoussetNewRef(*Pms));

        /* Insert into the code cache. */

        ajTablePut(msa->CacheByCode,
                   (void*) ajStrNewS((*Pms)->Code),
                   (void*) ensMiscellaneoussetNewRef(*Pms));
    }

    if(ms1 && ms2 && (ms1 == ms2))
    {
        ajDebug("miscellaneoussetadaptorCacheInsert replaced "
                "Miscellaneous Set %p with one already cached %p.\n",
                *Pms, ms1);

        ensMiscellaneoussetDel(Pms);

        Pms = &ms1;
    }

    if(ms1 && ms2 && (ms1 != ms2))
        ajDebug("miscellaneoussetadaptorCacheInsert detected "
                "Miscellaneous Sets in the identifier and code cache with "
                "identical codes ('%S' and '%S') but different addresses "
                "(%p and %p).\n",
                ms1->Code, ms2->Code, ms1, ms2);

    if(ms1 && (!ms2))
        ajDebug("miscellaneoussetadaptorCacheInsert detected an "
                "Ensembl Miscellaneous Set "
                "in the identifier, but not in the code cache.\n");

    if((!ms1) && ms2)
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
** @param [u] msa [EnsPMiscellaneoussetadaptor] Ensembl Miscellaneous
**                                              Set Adaptor
** @param [u] ms [EnsPMiscellaneousset] Ensembl Miscellaneous Set
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
** @@
******************************************************************************/

static AjBool miscellaneoussetadaptorCacheRemove(
    EnsPMiscellaneoussetadaptor msa,
    EnsPMiscellaneousset ms)
{
    ajuint* Pidentifier = NULL;

    AjPStr key = NULL;

    EnsPMiscellaneousset ms1 = NULL;
    EnsPMiscellaneousset ms2 = NULL;

    if(!msa)
        return ajFalse;

    if(!ms)
        return ajFalse;

    /* Remove the table nodes. */

    ms1 = (EnsPMiscellaneousset) ajTableRemoveKey(
        msa->CacheByIdentifier,
        (const void*) &ms->Identifier,
        (void**) &Pidentifier);

    ms2 = (EnsPMiscellaneousset) ajTableRemoveKey(
        msa->CacheByCode,
        (const void*) ms->Code,
        (void**) &key);

    if(ms1 && (!ms2))
        ajWarn("miscellaneoussetadaptorCacheRemove could remove "
               "Miscellaneous Set '%S' (%u) "
               "only from the identifier cache.\n",
               ms->Code, ms->Identifier);

    if((!ms1) && ms2)
        ajWarn("miscellaneoussetadaptorCacheRemove could remove "
               "Miscellaneous Set '%S' (%u) "
               "only from the code cache.\n",
               ms->Code, ms->Identifier);

    AJFREE(Pidentifier);

    ajStrDel(&key);

    ensMiscellaneoussetDel(&ms1);
    ensMiscellaneoussetDel(&ms2);

    return ajTrue;
}

#endif




/* @funcstatic miscellaneoussetadaptorCacheClearIdentifier ********************
**
** An ajTableMapDel "apply" function to clear the Ensembl Miscellaneous Set
** Adaptor-internal Miscellaneous Set cache. This function deletes the
** unsigned integer identifier key and the Ensembl Miscellaneous Set
** value data.
**
** @param [u] key [void**] AJAX unsigned integer key data address
** @param [u] value [void**] Ensembl Miscellaneous Set value data address
** @param [u] cl [void*] Standard, passed in from ajTableMapDel
** @see ajTableMapDel
**
** @return [void]
** @@
******************************************************************************/

static void miscellaneoussetadaptorCacheClearIdentifier(void** key,
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

    ensMiscellaneoussetDel((EnsPMiscellaneousset*) value);

    *key   = NULL;
    *value = NULL;

    return;
}




/* @funcstatic miscellaneoussetadaptorCacheClearCode **************************
**
** An ajTableMapDel "apply" function to clear the Ensembl Miscellaneous Set
** Adaptor-internal Miscellaneous Set cache. This function deletes the code
** AJAX String key data and the Ensembl Miscellaneous Set value data.
**
** @param [u] key [void**] AJAX String key data address
** @param [u] value [void**] Ensembl Miscellaneous Set value data address
** @param [u] cl [void*] Standard, passed in from ajTableMapDel
** @see ajTableMapDel
**
** @return [void]
** @@
******************************************************************************/

static void miscellaneoussetadaptorCacheClearCode(void** key,
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

    ensMiscellaneoussetDel((EnsPMiscellaneousset*) value);

    *key   = NULL;
    *value = NULL;

    return;
}




/* @func ensMiscellaneoussetadaptorCacheClear *********************************
**
** Clear the Ensembl Miscellaneous Set Adaptor-internal cache of
** Ensembl Miscellaneous Set objects.
**
** @param [u] msa [EnsPMiscellaneoussetadaptor] Ensembl Miscellaneous
**                                              Set Adaptor
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
** @@
******************************************************************************/

AjBool ensMiscellaneoussetadaptorCacheClear(
    EnsPMiscellaneoussetadaptor msa)
{
    if(!msa)
        return ajFalse;

    /* Clear and delete the identifier cache. */

    ajTableMapDel(msa->CacheByIdentifier,
                  miscellaneoussetadaptorCacheClearIdentifier,
                  NULL);

    ajTableFree(&msa->CacheByIdentifier);

    /* Clear and delete the code cache. */

    ajTableMapDel(msa->CacheByCode,
                  miscellaneoussetadaptorCacheClearCode,
                  NULL);

    ajTableFree(&msa->CacheByCode);

    return ajTrue;
}




/* @section destructors *******************************************************
**
** Destruction destroys all internal data structures and frees the
** memory allocated for an Ensembl Miscellaneous Set Adaptor object.
**
** @fdata [EnsPMiscellaneoussetadaptor]
**
** @nam3rule Del Destroy (free) an Ensembl Miscellaneous Set Adaptor object
**
** @argrule * Pmsa [EnsPMiscellaneoussetadaptor*]
** Ensembl Miscellaneous Set Adaptor object address
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
** Ensembl Miscellaneous Set Adaptor object address
**
** @return [void]
** @@
******************************************************************************/

void ensMiscellaneoussetadaptorDel(EnsPMiscellaneoussetadaptor* Pmsa)
{
    EnsPMiscellaneoussetadaptor pthis = NULL;

    if(!Pmsa)
        return;

    if(!*Pmsa)
        return;

    pthis = *Pmsa;

    ensMiscellaneoussetadaptorCacheClear(pthis);

    AJFREE(pthis);

    *Pmsa = NULL;

    return;
}




/* @section element retrieval *************************************************
**
** Functions for returning elements of an
** Ensembl Miscellaneous Set Adaptor object.
**
** @fdata [EnsPMiscellaneoussetadaptor]
**
** @nam3rule Get Return Ensembl Miscellaneous Set Adaptor attribute(s)
** @nam4rule Baseadaptor     Return the Ensembl Base Adaptor
** @nam4rule Databaseadaptor Return the Ensembl Database Adaptor
**
** @argrule * msa [const EnsPMiscellaneoussetadaptor] Ensembl Miscellaneous
**                                                    Set Adaptor
**
** @valrule Baseadaptor [EnsPBaseadaptor] Ensembl Base Adaptor or NULL
** @valrule Databaseadaptor [EnsPDatabaseadaptor] Ensembl Database Adaptor
**                                                or NULL
**
** @fcategory use
******************************************************************************/




/* @func ensMiscellaneoussetadaptorGetBaseadaptor *****************************
**
** Get the Ensembl Base Adaptor element of an
** Ensembl Miscellaneous Set Adaptor.
**
** @param [r] msa [const EnsPMiscellaneoussetadaptor] Ensembl Miscellaneous
**                                                    Set Adaptor
**
** @return [EnsPBaseadaptor] Ensembl Base Adaptor or NULL
** @@
******************************************************************************/

EnsPBaseadaptor ensMiscellaneoussetadaptorGetBaseadaptor(
    const EnsPMiscellaneoussetadaptor msa)
{
    if(!msa)
        return NULL;

    return msa->Adaptor;
}




/* @func ensMiscellaneoussetadaptorGetDatabaseadaptor *************************
**
** Get the Ensembl Database Adaptor element of an
** Ensembl Miscellaneous Set Adaptor.
**
** @param [r] msa [const EnsPMiscellaneoussetadaptor] Ensembl Miscellaneous
**                                                    Set Adaptor
**
** @return [EnsPDatabaseadaptor] Ensembl Database Adaptor or NULL
** @@
******************************************************************************/

EnsPDatabaseadaptor ensMiscellaneoussetadaptorGetDatabaseadaptor(
    const EnsPMiscellaneoussetadaptor msa)
{
    if(!msa)
        return NULL;

    return ensBaseadaptorGetDatabaseadaptor(msa->Adaptor);
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
** @argrule * msa [EnsPMiscellaneoussetadaptor] Ensembl Miscellaneous Set
**                                              Adaptor
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
** @param [u] value [void**] Ensembl Miscellaneous Set value data address
** @param [u] cl [void*] AJAX List of Ensembl Miscellaneous Set objects,
**                       passed in via ajTableMap
** @see ajTableMap
**
** @return [void]
** @@
******************************************************************************/

static void miscellaneoussetadaptorFetchAll(const void* key,
                                            void** value,
                                            void* cl)
{
    EnsPMiscellaneousset* Pms = NULL;

    if(!key)
        return;

    if(!value)
        return;

    if(!*value)
        return;

    if(!cl)
        return;

    Pms = (EnsPMiscellaneousset*) value;

    ajListPushAppend((AjPList) cl, (void*) ensMiscellaneoussetNewRef(*Pms));

    return;
}




/* @func ensMiscellaneoussetadaptorFetchAll ***********************************
**
** Fetch all Ensembl Miscellaneous Sets.
**
** @cc Bio::EnsEMBL::DBSQL::MiscSetAdaptor::fetch_all
** @param [u] msa [EnsPMiscellaneoussetadaptor] Ensembl Miscellaneous Set
**                                              Adaptor
** @param [u] mss [AjPList] AJAX List of Ensembl Miscellaneous Sets
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
** @@
******************************************************************************/

AjBool ensMiscellaneoussetadaptorFetchAll(
    EnsPMiscellaneoussetadaptor msa,
    AjPList mss)
{
    if(!msa)
        return ajFalse;

    if(!mss)
        return ajFalse;

    if(!msa->CacheByIdentifier)
        miscellaneoussetadaptorCacheInit(msa);

    ajTableMap(msa->CacheByIdentifier,
               miscellaneoussetadaptorFetchAll,
               (void*) mss);

    return ajTrue;
}




/* @func ensMiscellaneoussetadaptorFetchByCode ********************************
**
** Fetch an Ensembl Miscellaneous Set by its code.
**
** @cc Bio::EnsEMBL::DBSQL::MiscSetAdaptor::fetch_by_code
** @param [u] msa [EnsPMiscellaneoussetadaptor] Ensembl Miscellaneous
**                                              Set Adaptor
** @param [r] code [const AjPStr] Ensembl Miscellaneous Set code
** @param [wP] Pms [EnsPMiscellaneousset*] Ensembl Miscellaneous Set address
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
** @@
******************************************************************************/

AjBool ensMiscellaneoussetadaptorFetchByCode(
    EnsPMiscellaneoussetadaptor msa,
    const AjPStr code,
    EnsPMiscellaneousset* Pms)
{
    char* txtcode = NULL;

    AjPList mss = NULL;

    AjPStr constraint = NULL;

    EnsPDatabaseadaptor dba = NULL;

    EnsPMiscellaneousset ms = NULL;

    if(!msa)
        return ajFalse;

    if((!code) && (!ajStrGetLen(code)))
        return ajFalse;

    if(!Pms)
        return ajFalse;

    /*
    ** Initially, search the code cache.
    ** For any object returned by the AJAX Table the reference counter needs
    ** to be incremented manually.
    */

    if(!msa->CacheByCode)
        miscellaneoussetadaptorCacheInit(msa);

    *Pms = (EnsPMiscellaneousset) ajTableFetchmodV(msa->CacheByCode,
                                                   (const void*) code);

    if(*Pms)
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

    if(ajListGetLength(mss) > 1)
        ajWarn("ensMiscellaneoussetadaptorFetchByCode got more than one "
               "Ensembl Miscellaneous Sets for (UNIQUE) code '%S'.\n",
               code);

    ajListPop(mss, (void**) Pms);

    miscellaneoussetadaptorCacheInsert(msa, Pms);

    while(ajListPop(mss, (void**) &ms))
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
** @param [u] msa [EnsPMiscellaneoussetadaptor] Ensembl Miscellaneous
**                                              Set Adaptor
** @param [r] identifier [ajuint] SQL database-internal identifier
** @param [wP] Pms [EnsPMiscellaneousset*] Ensembl Miscellaneous Set address
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
** @@
******************************************************************************/

AjBool ensMiscellaneoussetadaptorFetchByIdentifier(
    EnsPMiscellaneoussetadaptor msa,
    ajuint identifier,
    EnsPMiscellaneousset* Pms)
{
    AjPList mss = NULL;

    AjPStr constraint = NULL;

    EnsPMiscellaneousset ms = NULL;

    if(!msa)
        return ajFalse;

    if(!identifier)
        return ajFalse;

    if(!Pms)
        return ajFalse;

    /*
    ** Initially, search the identifier cache.
    ** For any object returned by the AJAX Table the reference counter needs
    ** to be incremented manually.
    */

    if(!msa->CacheByIdentifier)
        miscellaneoussetadaptorCacheInit(msa);

    *Pms = (EnsPMiscellaneousset) ajTableFetchmodV(msa->CacheByIdentifier,
                                                   (const void*) &identifier);

    if(*Pms)
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

    if(ajListGetLength(mss) > 1)
        ajWarn("ensMiscellaneoussetadaptorFetchByIdentifier got more "
               "than one Ensembl Miscellaneous Set for (PRIMARY KEY) "
               "identifier %u.\n", identifier);

    ajListPop(mss, (void**) Pms);

    miscellaneoussetadaptorCacheInsert(msa, Pms);

    while(ajListPop(mss, (void**) &ms))
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
** @cc CVS Revision: 1.17
** @cc CVS Tag: branch-ensembl-62
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
** @argrule Cpy mf [const EnsPMiscellaneousfeature] Ensembl Miscellaneous
**                                                  Feature
** @argrule Ini mfa [EnsPMiscellaneousfeatureadaptor] Ensembl Miscellaneous
**                                                    Feature Adaptor
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
** @param [r] mf [const EnsPMiscellaneousfeature] Ensembl Miscellaneous
**                                                Feature
**
** @return [EnsPMiscellaneousfeature] Ensembl Miscellaneous Feature or NULL
** @@
******************************************************************************/

EnsPMiscellaneousfeature ensMiscellaneousfeatureNewCpy(
    const EnsPMiscellaneousfeature mf)
{
    AjIList iter = NULL;

    EnsPAttribute attribute = NULL;

    EnsPMiscellaneousfeature pthis = NULL;

    EnsPMiscellaneousset ms = NULL;

    if(!mf)
        return NULL;

    AJNEW0(pthis);

    pthis->Use = 1;

    pthis->Identifier = mf->Identifier;

    pthis->Adaptor = mf->Adaptor;

    pthis->Feature = ensFeatureNewRef(mf->Feature);

    /* NOTE: Copy the AJAX List of Ensembl Attribute objects. */

    if(mf->Attributes && ajListGetLength(mf->Attributes))
    {
        pthis->Attributes = ajListNew();

        iter = ajListIterNew(mf->Attributes);

        while(!ajListIterDone(iter))
        {
            attribute = (EnsPAttribute) ajListIterGet(iter);

            ajListPushAppend(pthis->Attributes,
                             (void*) ensAttributeNewRef(attribute));
        }

        ajListIterDel(&iter);
    }
    else
        pthis->Attributes = NULL;

    /* NOTE: Copy the AJAX List of Ensembl Miscellaneous Set objects. */

    if(mf->Miscellaneoussets && ajListGetLength(mf->Miscellaneoussets))
    {
        pthis->Miscellaneoussets = ajListNew();

        iter = ajListIterNew(mf->Miscellaneoussets);

        while(!ajListIterDone(iter))
        {
            ms = (EnsPMiscellaneousset) ajListIterGet(iter);

            ajListPushAppend(pthis->Miscellaneoussets,
                             (void*) ensMiscellaneoussetNewRef(ms));
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
** @param [u] mfa [EnsPMiscellaneousfeatureadaptor] Ensembl Miscellaneous
**                                                  Feature Adaptor
** @param [r] identifier [ajuint] SQL database-internal identifier
** @cc Bio::EnsEMBL::Feature::new
** @param [u] feature [EnsPFeature] Ensembl Feature
** @cc Bio::EnsEMBL::MiscFeature::new
**
** @return [EnsPMiscellaneousfeature] Ensembl Miscellaneous Feature or NULL
** @@
******************************************************************************/

EnsPMiscellaneousfeature ensMiscellaneousfeatureNewIni(
    EnsPMiscellaneousfeatureadaptor mfa,
    ajuint identifier,
    EnsPFeature feature)
{
    EnsPMiscellaneousfeature mf = NULL;

    if(!feature)
        return NULL;

    AJNEW0(mf);

    mf->Use = 1;

    mf->Identifier = identifier;

    mf->Adaptor = mfa;

    mf->Feature = ensFeatureNewRef(feature);

    mf->Attributes = NULL;

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
** @@
******************************************************************************/

EnsPMiscellaneousfeature ensMiscellaneousfeatureNewRef(
    EnsPMiscellaneousfeature mf)
{
    if(!mf)
        return NULL;

    mf->Use++;

    return mf;
}




/* @section destructors *******************************************************
**
** Destruction destroys all internal data structures and frees the
** memory allocated for an Ensembl Miscellaneous Feature object.
**
** @fdata [EnsPMiscellaneousfeature]
**
** @nam3rule Del Destroy (free) an Ensembl Miscellaneous Feature object
**
** @argrule * Pmf [EnsPMiscellaneousfeature*] Ensembl Miscellaneous Feature
**                                            object address
**
** @valrule * [void]
**
** @fcategory delete
******************************************************************************/




/* @func ensMiscellaneousfeatureDel *******************************************
**
** Default destructor for an Ensembl Miscellaneous Feature.
**
** @param [d] Pmf [EnsPMiscellaneousfeature*] Ensembl Miscellaneous Feature
**                                            object address
**
** @return [void]
** @@
******************************************************************************/

void ensMiscellaneousfeatureDel(EnsPMiscellaneousfeature* Pmf)
{
    EnsPAttribute attribute = NULL;

    EnsPMiscellaneousset ms = NULL;

    EnsPMiscellaneousfeature pthis = NULL;


    if(!Pmf)
        return;

    if(!*Pmf)
        return;

    (*Pmf)->Use--;

    pthis = *Pmf;

    if(pthis->Use)
    {
        *Pmf = NULL;

        return;
    }

    ensFeatureDel(&pthis->Feature);

    /* Clear and free the AJAX List of Ensembl Attribute objects. */

    while(ajListPop(pthis->Attributes, (void**) &attribute))
        ensAttributeDel(&attribute);

    ajListFree(&pthis->Attributes);

    /* Clear and free the AJAX List of Ensembl Miscellaneous Set objects. */

    while(ajListPop(pthis->Miscellaneoussets, (void**) &ms))
        ensMiscellaneoussetDel(&ms);

    ajListFree(&pthis->Miscellaneoussets);

    AJFREE(pthis);

    *Pmf = NULL;

    return;
}




/* @section element retrieval *************************************************
**
** Functions for returning elements of an Ensembl Miscellaneous Feature object.
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
** @valrule Attributes [const AjPList] AJAX List of
** Ensembl Attribute objects or NULL
** @valrule Feature [EnsPFeature] Ensembl Feature or NULL
** @valrule Identifier [ajuint] SQL database-internal identifier or 0
** @valrule Miscellaneoussets [const AjPList] AJAX List of
** Ensembl Miscellaneous Set objects or NULL
**
** @fcategory use
******************************************************************************/




/* @func ensMiscellaneousfeatureGetAdaptor ************************************
**
** Get the Ensembl Miscellaneous Feature Adaptor element of an
** Ensembl Miscellaneous Feature.
**
** @cc Bio::EnsEMBL::Storable::adaptor
** @param [r] mf [const EnsPMiscellaneousfeature] Ensembl Miscellaneous
**                                                Feature
**
** @return [EnsPMiscellaneousfeatureadaptor] Ensembl Miscellaneous
**                                           Feature Adaptor or NULL
** @@
******************************************************************************/

EnsPMiscellaneousfeatureadaptor ensMiscellaneousfeatureGetAdaptor(
    const EnsPMiscellaneousfeature mf)
{
    if(!mf)
        return NULL;

    return mf->Adaptor;
}




/* @func ensMiscellaneousfeatureGetAttributes *********************************
**
** Get all Ensembl Attribute objects of an Ensembl Miscellaneous Feature.
**
** @cc Bio::EnsEMBL::MiscFeature::get_all_Attributes
** @param [r] mf [const EnsPMiscellaneousfeature] Ensembl Miscellaneous
**                                                Feature
**
** @return [const AjPList] AJAX List of Ensembl Attribute objects or NULL
** @@
******************************************************************************/

const AjPList ensMiscellaneousfeatureGetAttributes(
    const EnsPMiscellaneousfeature mf)
{
    if(!mf)
        return NULL;

    return mf->Attributes;
}




/* @func ensMiscellaneousfeatureGetFeature ************************************
**
** Get the Ensembl Feature element of an Ensembl Miscellaneous Feature.
**
** @param [r] mf [const EnsPMiscellaneousfeature] Ensembl Miscellaneous
**                                                Feature
**
** @return [EnsPFeature] Ensembl Feature or NULL
** @@
******************************************************************************/

EnsPFeature ensMiscellaneousfeatureGetFeature(
    const EnsPMiscellaneousfeature mf)
{
    if(!mf)
        return NULL;

    return mf->Feature;
}




/* @func ensMiscellaneousfeatureGetIdentifier *********************************
**
** Get the SQL database-internal identifier element of an
** Ensembl Miscellaneous Feature.
**
** @cc Bio::EnsEMBL::Storable::dbID
** @param [r] mf [const EnsPMiscellaneousfeature] Ensembl Miscellaneous
**                                                Feature
**
** @return [ajuint] SQL database-internal identifier or 0
** @@
******************************************************************************/

ajuint ensMiscellaneousfeatureGetIdentifier(
    const EnsPMiscellaneousfeature mf)
{
    if(!mf)
        return 0;

    return mf->Identifier;
}




/* @func ensMiscellaneousfeatureGetMiscellaneoussets **************************
**
** Get all Ensembl Miscellaneous Sets of an Ensembl Miscellaneous Feature.
**
** @cc Bio::EnsEMBL::MiscFeature::get_all_MiscSets
** @param [r] mf [const EnsPMiscellaneousfeature] Ensembl Miscellaneous
**                                                Feature
**
** @return [const AjPList] AJAX List of Ensembl Miscellaneous Sets or NULL
** @@
******************************************************************************/

const AjPList ensMiscellaneousfeatureGetMiscellaneoussets(
    const EnsPMiscellaneousfeature mf)
{
    if(!mf)
        return NULL;

    return mf->Miscellaneoussets;
}




/* @section modifiers *********************************************************
**
** Functions for assigning elements of an Ensembl Miscellaneous Feature object.
**
** @fdata [EnsPMiscellaneousfeature]
**
** @nam3rule Set Set one element of a Miscellaneous Feature
** @nam4rule Adaptor Set the Ensembl Miscellaneous Feature Adaptor
** @nam4rule Identifier Set the SQL database-internal identifier
** @nam4rule Feature Set the Ensembl Feature
**
** @argrule * mf [EnsPMiscellaneousfeature] Ensembl Miscellaneous
**                                          Feature object
** @argrule Adaptor mfa [EnsPMiscellaneousfeatureadaptor] Ensembl Miscellaneous
**                                                        Feature Adaptor
** @argrule Feature feature [EnsPFeature] Ensembl Feature
** @argrule Identifier identifier [ajuint] SQL database-internal identifier
**
** @valrule * [AjBool] ajTrue upon success, ajFalse otherwise
**
** @fcategory modify
******************************************************************************/




/* @func ensMiscellaneousfeatureSetAdaptor ************************************
**
** Set the Ensembl Miscellaneous Feature Adaptor element of an
** Ensembl Miscellaneous Feature.
**
** @cc Bio::EnsEMBL::Storable::adaptor
** @param [u] mf [EnsPMiscellaneousfeature] Ensembl Miscellaneous Feature
** @param [u] mfa [EnsPMiscellaneousfeatureadaptor] Ensembl Miscellaneous
**                                                  Feature Adaptor
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
** @@
******************************************************************************/

AjBool ensMiscellaneousfeatureSetAdaptor(
    EnsPMiscellaneousfeature mf,
    EnsPMiscellaneousfeatureadaptor mfa)
{
    if(!mf)
        return ajFalse;

    mf->Adaptor = mfa;

    return ajTrue;
}




/* @func ensMiscellaneousfeatureSetFeature ************************************
**
** Set the Ensembl Feature element of an Ensembl Miscellaneous Feature.
**
** @param [u] mf [EnsPMiscellaneousfeature] Ensembl Miscellaneous Feature
** @param [u] feature [EnsPFeature] Ensembl Feature
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
** @@
******************************************************************************/

AjBool ensMiscellaneousfeatureSetFeature(
    EnsPMiscellaneousfeature mf,
    EnsPFeature feature)
{
    if(!mf)
        return ajFalse;

    ensFeatureDel(&mf->Feature);

    mf->Feature = ensFeatureNewRef(feature);

    return ajTrue;
}




/* @func ensMiscellaneousfeatureSetIdentifier *********************************
**
** Set the SQL database-internal identifier element of an
** Ensembl Miscellaneous Feature.
**
** @cc Bio::EnsEMBL::Storable::dbID
** @param [u] mf [EnsPMiscellaneousfeature] Ensembl Miscellaneous Feature
** @param [r] identifier [ajuint] SQL database-internal identifier
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
** @@
******************************************************************************/

AjBool ensMiscellaneousfeatureSetIdentifier(
    EnsPMiscellaneousfeature mf,
    ajuint identifier)
{
    if(!mf)
        return ajFalse;

    mf->Identifier = identifier;

    return ajTrue;
}




/* @section element addition **************************************************
**
** Functions for adding elements to an Ensembl Miscellaneous Feature object.
**
** @fdata [EnsPMiscellaneousfeature]
**
** @nam3rule Add Add one object to an Ensembl Miscellaneous Feature
** @nam4rule Attribute Add an Ensembl Attribute
** @nam4rule Miscellaneousset Add an Ensembl Miscellaneous Set
**
** @argrule * mf [EnsPMiscellaneousfeature] Ensembl Miscellaneous Feature
** @argrule Attribute attribute [EnsPAttribute] Ensembl Attribute
** @argrule Miscellaneousset ms [EnsPMiscellaneousset] Ensembl Miscellaneous
**                                                     Set
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
** @@
******************************************************************************/

AjBool ensMiscellaneousfeatureAddAttribute(
    EnsPMiscellaneousfeature mf,
    EnsPAttribute attribute)
{
    if(!mf)
        return ajFalse;

    if(!attribute)
        return ajFalse;

    if(!mf->Attributes)
        mf->Attributes = ajListNew();

    ajListPushAppend(mf->Attributes, (void*) ensAttributeNewRef(attribute));

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
** @@
******************************************************************************/

AjBool ensMiscellaneousfeatureAddMiscellaneousset(
    EnsPMiscellaneousfeature mf,
    EnsPMiscellaneousset ms)
{
    if(!mf)
        return ajFalse;

    if(!ms)
        return ajFalse;

    if(!mf->Miscellaneoussets)
        mf->Miscellaneoussets = ajListNew();

    ajListPushAppend(mf->Miscellaneoussets,
                     (void*) ensMiscellaneoussetNewRef(ms));

    return ajTrue;
}




/* @section debugging *********************************************************
**
** Functions for reporting of an Ensembl Miscellaneous Feature object.
**
** @fdata [EnsPMiscellaneousfeature]
**
** @nam3rule Trace Report Ensembl Miscellaneous Feature elements to debug file
**
** @argrule Trace mf [const EnsPMiscellaneousfeature] Ensembl Miscellaneous
**                                                    Feature
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
** @param [r] mf [const EnsPMiscellaneousfeature] Ensembl Miscellaneous
**                                                Feature
** @param [r] level [ajuint] Indentation level
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
** @@
******************************************************************************/

AjBool ensMiscellaneousfeatureTrace(const EnsPMiscellaneousfeature mf,
                                    ajuint level)
{
    AjPStr indent = NULL;

    if(!mf)
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
** @param [r] mf [const EnsPMiscellaneousfeature] Ensembl Miscellaneous
**                                                Feature
**
** @return [size_t] Memory size in bytes or 0
** @@
******************************************************************************/

size_t ensMiscellaneousfeatureCalculateMemsize(
    const EnsPMiscellaneousfeature mf)
{
    size_t size = 0;

    AjIList iter = NULL;

    EnsPAttribute attribute = NULL;

    EnsPMiscellaneousset ms = NULL;

    if(!mf)
        return 0;

    size += sizeof (EnsOMiscellaneousfeature);

    size += ensFeatureCalculateMemsize(mf->Feature);

    if(mf->Attributes)
    {
        iter = ajListIterNewread(mf->Attributes);

        while(!ajListIterDone(iter))
        {
            attribute = (EnsPAttribute) ajListIterGet(iter);

            size += ensAttributeCalculateMemsize(attribute);
        }

        ajListIterDel(&iter);
    }

    if(mf->Miscellaneoussets)
    {
        iter = ajListIterNewread(mf->Miscellaneoussets);

        while(!ajListIterDone(iter))
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
** @argrule AllAttributes code [const AjPStr] Ensembl Attribute code
** @argrule AllAttributes attributes [AjPList] AJAX List of
** Ensembl Attribute objects
** @argrule AllMiscellaneoussets mf [EnsPMiscellaneousfeature]
** Ensembl Miscellaneous Feature
** @argrule AllMiscellaneoussets code [const AjPStr]
** Ensembl Miscellaneous Set code
** @argrule AllMiscellaneoussets mss [AjPList] AJAX List of
** Ensembl Miscellaneous Set objects
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
** @@
******************************************************************************/

AjBool ensMiscellaneousfeatureFetchAllAttributes(
    EnsPMiscellaneousfeature mf,
    const AjPStr code,
    AjPList attributes)
{
    AjIList iter = NULL;

    EnsPAttribute attribute = NULL;

    if(!mf)
        return ajFalse;

    if(!attributes)
        return ajFalse;

    iter = ajListIterNewread(mf->Attributes);

    while(!ajListIterDone(iter))
    {
        attribute = (EnsPAttribute) ajListIterGet(iter);

        if(code && ajStrGetLen(code))
        {
            if(ajStrMatchCaseS(code, ensAttributeGetCode(attribute)))
                ajListPushAppend(attributes,
                                 (void*) ensAttributeNewRef(attribute));
        }
        else
            ajListPushAppend(attributes,
                             (void*) ensAttributeNewRef(attribute));
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
** @@
******************************************************************************/

AjBool ensMiscellaneousfeatureFetchAllMiscellaneoussets(
    EnsPMiscellaneousfeature mf,
    const AjPStr code,
    AjPList mss)
{
    AjIList iter = NULL;

    EnsPMiscellaneousset ms = NULL;

    if(!mf)
        return ajFalse;

    if(!mss)
        return ajFalse;

    iter = ajListIterNewread(mf->Miscellaneoussets);

    while(!ajListIterDone(iter))
    {
        ms = (EnsPMiscellaneousset) ajListIterGet(iter);

        if(code && ajStrGetLen(code))
        {
            if(ajStrMatchCaseS(code, ensMiscellaneoussetGetCode(ms)))
                ajListPushAppend(mss,
                                 (void*) ensMiscellaneoussetNewRef(ms));
        }
        else
            ajListPushAppend(mss, (void*) ensMiscellaneoussetNewRef(ms));
    }

    ajListIterDel(&iter);

    return ajTrue;
}




/* @datasection [AjPList] AJAX List *******************************************
**
** @nam2rule List Functions for manipulating AJAX List objects
**
******************************************************************************/




/* @section list **************************************************************
**
** Functions for manipulating AJAX List objects.
**
** @fdata [AjPList]
**
** @nam3rule Miscellaneousfeature Functions for manipulating AJAX List objects
** of Ensembl Miscellaneous Feature objects
** @nam4rule Sort Sort functions
** @nam5rule Start Sort by Ensembl Feature start element
** @nam6rule Ascending  Sort in ascending order
** @nam6rule Descending Sort in descending order
**
** @argrule Ascending mfs [AjPList] AJAX List of Ensembl Miscellaneous
**                                  Feature objects
** @argrule Descending mfs [AjPList] AJAX List of Ensembl Miscellaneous
**                                   Feature objects
**
** @valrule * [AjBool] ajTrue upon success, ajFalse otherwise
**
** @fcategory misc
******************************************************************************/




/* @funcstatic listMiscellaneousfeatureCompareStartAscending ******************
**
** AJAX List of Ensembl Miscellaneous Feature objects comparison function to
** sort by Ensembl Feature start coordinate in ascending order.
**
** @param [r] P1 [const void*] Ensembl Miscellaneous Feature address 1
** @param [r] P2 [const void*] Ensembl Miscellaneous Feature address 2
** @see ajListSort
**
** @return [int] The comparison function returns an integer less than,
**               equal to, or greater than zero if the first argument is
**               considered to be respectively less than, equal to, or
**               greater than the second.
** @@
******************************************************************************/

static int listMiscellaneousfeatureCompareStartAscending(const void* P1,
                                                         const void* P2)
{
    const EnsPMiscellaneousfeature mf1 = NULL;
    const EnsPMiscellaneousfeature mf2 = NULL;

    mf1 = *(EnsPMiscellaneousfeature const*) P1;
    mf2 = *(EnsPMiscellaneousfeature const*) P2;

    if(ajDebugTest("listMiscellaneousfeatureCompareStartAscending"))
        ajDebug("listMiscellaneousfeatureCompareStartAscending\n"
                "  mf1 %p\n"
                "  mf2 %p\n",
                mf1,
                mf2);

    /* Sort empty values towards the end of the AJAX List. */

    if(mf1 && (!mf2))
        return -1;

    if((!mf1) && (!mf2))
        return 0;

    if((!mf1) && mf2)
        return +1;

    return ensFeatureCompareStartAscending(mf1->Feature, mf2->Feature);
}




/* @func ensListMiscellaneousfeatureSortStartAscending ************************
**
** Sort an AJAX List of Ensembl Miscellaneous Feature objects by their
** Ensembl Feature start coordinate in ascending order.
**
** @param [u] mfs [AjPList] AJAX List of Ensembl Miscellaneous Feature objects
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
** @@
******************************************************************************/

AjBool ensListMiscellaneousfeatureSortStartAscending(AjPList mfs)
{
    if(!mfs)
        return ajFalse;

    ajListSort(mfs, listMiscellaneousfeatureCompareStartAscending);

    return ajTrue;
}




/* @funcstatic listMiscellaneousfeatureCompareStartDescending *****************
**
** AJAX List of Ensembl Miscellaneous Feature objects comparison function to
** sort by Ensembl Feature start coordinate in descending order.
**
** @param [r] P1 [const void*] Ensembl Miscellaneous Feature address 1
** @param [r] P2 [const void*] Ensembl Miscellaneous Feature address 2
** @see ajListSort
**
** @return [int] The comparison function returns an integer less than,
**               equal to, or greater than zero if the first argument is
**               considered to be respectively less than, equal to, or
**               greater than the second.
** @@
******************************************************************************/

static int listMiscellaneousfeatureCompareStartDescending(const void* P1,
                                                          const void* P2)
{
    const EnsPMiscellaneousfeature mf1 = NULL;
    const EnsPMiscellaneousfeature mf2 = NULL;

    mf1 = *(EnsPMiscellaneousfeature const*) P1;
    mf2 = *(EnsPMiscellaneousfeature const*) P2;

    if(ajDebugTest("listMiscellaneousfeatureCompareStartDescending"))
        ajDebug("listMiscellaneousfeatureCompareStartDescending\n"
                "  mf1 %p\n"
                "  mf2 %p\n",
                mf1,
                mf2);

    /* Sort empty values towards the end of the AJAX List. */

    if(mf1 && (!mf2))
        return -1;

    if((!mf1) && (!mf2))
        return 0;

    if((!mf1) && mf2)
        return +1;

    return ensFeatureCompareStartDescending(mf1->Feature, mf2->Feature);
}




/* @func ensListMiscellaneousfeatureSortStartDescending ***********************
**
** Sort an AJAX List of Ensembl Miscellaneous Feature objects by their
** Ensembl Feature start coordinate in descending order.
**
** @param [u] mfs [AjPList] AJAX List of Ensembl Miscellaneous Feature objects
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
** @@
******************************************************************************/

AjBool ensListMiscellaneousfeatureSortStartDescending(AjPList mfs)
{
    if(!mfs)
        return ajFalse;

    ajListSort(mfs, listMiscellaneousfeatureCompareStartDescending);

    return ajTrue;
}




/* @datasection [EnsPMiscellaneousfeatureadaptor] Ensembl Miscellaneous Feature
** Adaptor
**
** @nam2rule Miscellaneousfeatureadaptor Functions for manipulating
** Ensembl Miscellaneous Feature Adaptor objects
**
** @cc Bio::EnsEMBL::DBSQL::MiscFeatureAdaptor
** @cc CVS Revision: 1.25
** @cc CVS Tag: branch-ensembl-62
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
** @@
** This function keeps an AJAX Table of Attribute code:value String key
** data and AjBool value data.
******************************************************************************/

static AjBool miscellaneousfeatureadaptorHasAttribute(AjPTable attributes,
                                                      ajuint atid,
                                                      const AjPStr value)
{
    AjBool* Pbool = NULL;

    AjPStr key = NULL;

    key = ajFmtStr("%u:$S", atid, value);

    if(ajTableFetchmodV(attributes, (const void*) key))
    {
        ajStrDel(&key);

        return ajTrue;
    }
    else
    {
        AJNEW0(Pbool);

        *Pbool = ajTrue;

        ajTablePut(attributes, (void*) key, (void*) Pbool);

        return ajFalse;
    }
}




/* @funcstatic miscellaneousfeatureadaptorClearAttributes *********************
**
** An ajTableMapDel "apply" function to clear an AJAX Table of
** AJAX String key data and
** AJAX Boolean value data.
**
** This function clears Attribute code:value AJAX String key and
** AJAX Boolean value data from the AJAX Table.
**
** @param [u] key [void**] AJAX String address
** @param [u] value [void**] AJAX Boolean address
** @param [u] cl [void*] Standard, passed in from ajTableMapDel
** @see ajTableMapDel
**
** @return [void]
** @@
******************************************************************************/

static void miscellaneousfeatureadaptorClearAttributes(void** key,
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

    AJFREE(*value);

    *key   = NULL;
    *value = NULL;

    return;
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
** @@
** This function keeps an AJAX Table of Miscellaneous Set identifier unsigned
** integer key data and AjBool value data.
******************************************************************************/

static AjBool miscellaneousfeatureadaptorHasMiscellaneousset(AjPTable sets,
                                                             ajuint msid)
{
    AjBool* Pbool = NULL;

    ajuint* Pidentifier = NULL;

    if(ajTableFetchmodV(sets, (const void*) &msid))
        return ajTrue;

    AJNEW0(Pidentifier);

    *Pidentifier = msid;

    AJNEW0(Pbool);

    *Pbool = ajTrue;

    ajTablePut(sets, (void*) Pidentifier, (void*) Pbool);

    return ajFalse;
}




/* @funcstatic miscellaneousfeatureadaptorClearMiscellaneoussets **************
**
** An ajTableMapDel "apply" function to clear an AJAX Table of
** AJAX unsigned integer key data and
** AJAX Boolean value data.
**
** This function clears Miscellaneous Set identifier unsigned integer key and
** AJAX Boolean value data from the AJAX Table.
**
** @param [u] key [void**] AJAX unsigned integer address
** @param [u] value [void**] AJAX Boolean address
** @param [u] cl [void*] Standard, passed in from ajTableMapDel
** @see ajTableMapDel
**
** @return [void]
** @@
******************************************************************************/

static void miscellaneousfeatureadaptorClearMiscellaneoussets(void** key,
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
    AJFREE(*value);

    *key   = NULL;
    *value = NULL;

    return;
}




/* @funcstatic miscellaneousfeatureadaptorFetchAllbyStatement *****************
**
** Fetch all Ensembl Miscellaneous Feature objects via an SQL statement.
**
** @cc Bio::EnsEMBL::DBSQL::MiscFeatureAdaptor::_objs_from_sth
** @param [u] dba [EnsPDatabaseadaptor] Ensembl Database Adaptor
** @param [r] statement [const AjPStr] SQL statement
** @param [uN] am [EnsPAssemblymapper] Ensembl Assembly Mapper
** @param [uN] slice [EnsPSlice] Ensembl Slice
** @param [u] mfs [AjPList] AJAX List of Ensembl Miscellaneous Feature objects
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
** @@
******************************************************************************/

static AjBool miscellaneousfeatureadaptorFetchAllbyStatement(
    EnsPDatabaseadaptor dba,
    const AjPStr statement,
    EnsPAssemblymapper am,
    EnsPSlice slice,
    AjPList mfs)
{
    ajuint identifier = 0;
    ajuint atid       = 0;
    ajuint msid       = 0;

    ajuint srid    = 0;
    ajuint srstart = 0;
    ajuint srend   = 0;
    ajint srstrand = 0;

    ajint slstart  = 0;
    ajint slend    = 0;
    ajint slstrand = 0;
    ajint sllength = 0;

    ajuint current = 0;
    ajuint throw = 0;

    AjPList mrs = NULL;

    AjPSqlstatement sqls = NULL;
    AjISqlrow sqli       = NULL;
    AjPSqlrow sqlr       = NULL;

    AjPStr avalue = NULL;

    AjPTable attributes = NULL;
    AjPTable sets       = NULL;

    EnsPAssemblymapperadaptor ama = NULL;

    EnsPAttribute attribute = NULL;

    EnsPAttributetype        at  = NULL;
    EnsPAttributetypeadaptor ata = NULL;

    EnsPCoordsystemadaptor csa = NULL;

    EnsPFeature feature = NULL;

    EnsPMiscellaneousfeature mf         = NULL;
    EnsPMiscellaneousfeatureadaptor mfa = NULL;

    EnsPMiscellaneousset ms         = NULL;
    EnsPMiscellaneoussetadaptor msa = NULL;

    EnsPMapperresult mr = NULL;

    EnsPSlice srslice   = NULL;
    EnsPSliceadaptor sa = NULL;

    if(ajDebugTest("miscellaneousfeatureadaptorFetchAllbyStatement"))
        ajDebug("miscellaneousfeatureadaptorFetchAllbyStatement\n"
                "  dba %p\n"
                "  statement %p\n"
                "  am %p\n"
                "  slice %p\n"
                "  mfs %p\n",
                dba,
                statement,
                am,
                slice,
                mfs);

    if(!dba)
        return ajFalse;

    if(!statement)
        return ajFalse;

    if(!mfs)
        return ajFalse;

    ata = ensRegistryGetAttributetypeadaptor(dba);

    csa = ensRegistryGetCoordsystemadaptor(dba);

    mfa = ensRegistryGetMiscellaneousfeatureadaptor(dba);

    msa = ensRegistryGetMiscellaneoussetadaptor(dba);

    sa = ensRegistryGetSliceadaptor(dba);

    if(slice)
        ama = ensRegistryGetAssemblymapperadaptor(dba);

    attributes = ensTablestrNewLen(0);

    sets = ensTableuintNewLen(0);

    mrs = ajListNew();

    sqls = ensDatabaseadaptorSqlstatementNew(dba, statement);

    sqli = ajSqlrowiterNew(sqls);

    while(!ajSqlrowiterDone(sqli))
    {
        identifier = 0;
        srid       = 0;
        srstart    = 0;
        srend      = 0;
        srstrand   = 0;
        atid       = 0;
        avalue     = ajStrNew();
        msid       = 0;

        sqlr = ajSqlrowiterGet(sqli);

        ajSqlcolumnToUint(sqlr, &identifier);
        ajSqlcolumnToUint(sqlr, &srid);
        ajSqlcolumnToUint(sqlr, &srstart);
        ajSqlcolumnToUint(sqlr, &srend);
        ajSqlcolumnToInt(sqlr, &srstrand);
        ajSqlcolumnToUint(sqlr, &atid);
        ajSqlcolumnToStr(sqlr, &avalue);
        ajSqlcolumnToUint(sqlr, &msid);

        if(identifier == throw)
        {
            /*
            ** This Miscellanous Feature is already known to map into a gap,
            ** therefore throw it away immediately.
            */

            ajStrDel(&avalue);

            continue;
        }

        if(identifier == current)
        {
            /* Still working on the same Miscellaneous Feature. */

            if(msid)
                ensMiscellaneoussetadaptorFetchByIdentifier(msa, msid, &ms);

            if(atid > 0 && ajStrGetLen(avalue) &&
               !miscellaneousfeatureadaptorHasAttribute(attributes,
                                                        atid,
                                                        avalue))
            {
                ensAttributetypeadaptorFetchByIdentifier(ata, atid, &at);

                attribute = ensAttributeNewIni(at, avalue);

                ensAttributetypeDel(&at);
            }
        }
        else
        {
            /*
            ** This is a new Miscellaneous Feature, therefore clear all
            ** internal caches.
            */

            ajTableMapDel(attributes,
                          miscellaneousfeatureadaptorClearAttributes,
                          NULL);

            ajTableMapDel(sets,
                          miscellaneousfeatureadaptorClearMiscellaneoussets,
                          NULL);

            current = identifier;

            throw = 0;

            /* Need to get the internal Ensembl Sequence Region identifier. */

            srid = ensCoordsystemadaptorGetSeqregionidentifierInternal(csa,
                                                                       srid);

            /*
            ** Since the Ensembl SQL schema defines Sequence Region start and
            ** end coordinates as unsigned integers for all Feature objects,
            ** the range needs checking.
            */

            if(srstart <= INT_MAX)
                slstart = (ajint) srstart;
            else
                ajFatal("miscellaneousfeatureadaptorFetchAllbyStatement got a "
                        "Sequence Region start coordinate (%u) outside the "
                        "maximum integer limit (%d).",
                        srstart, INT_MAX);

            if(srend <= INT_MAX)
                slend = (ajint) srend;
            else
                ajFatal("miscellaneousfeatureadaptorFetchAllbyStatement got a "
                        "Sequence Region end coordinate (%u) outside the "
                        "maximum integer limit (%d).",
                        srend, INT_MAX);

            slstrand = srstrand;

            /* Fetch a Slice spanning the entire Sequence Region. */

            ensSliceadaptorFetchBySeqregionIdentifier(sa,
                                                      srid,
                                                      0,
                                                      0,
                                                      0,
                                                      &srslice);

            /*
            ** Obtain an Ensembl Assembly Mapper if none was defined, but a
            ** destination Slice was.
            */

            if(am)
                am = ensAssemblymapperNewRef(am);
            else if(slice && (!ensCoordsystemMatch(
                                  ensSliceGetCoordsystemObject(slice),
                                  ensSliceGetCoordsystemObject(srslice))))
                ensAssemblymapperadaptorFetchBySlices(ama,
                                                      slice,
                                                      srslice,
                                                      &am);

            /*
            ** Remap the Feature coordinates to another Ensembl Coordinate
            ** System if an Ensembl Assembly Mapper was provided.
            */

            if(am)
            {
                ensAssemblymapperFastmap(am,
                                         ensSliceGetSeqregion(srslice),
                                         slstart,
                                         slend,
                                         slstrand,
                                         mrs);

                /*
                ** The ensAssemblymapperFastmap function returns at best one
                ** Ensembl Mapper Result.
                */

                ajListPop(mrs, (void**) &mr);

                /*
                ** Skip Feature objects that map to gaps or
                ** Coordinate System boundaries.
                */

                if(ensMapperresultGetType(mr) !=
                   ensEMapperresultTypeCoordinate)
                {
                    /* Load the next Feature but destroy first! */

                    ajStrDel(&avalue);

                    ensSliceDel(&srslice);

                    ensAssemblymapperDel(&am);

                    ensMapperresultDel(&mr);

                    /* Mark the Miscellaneous Feature as one to throw away. */

                    throw = identifier;

                    continue;
                }

                srid     = ensMapperresultGetObjectidentifier(mr);
                slstart  = ensMapperresultGetCoordinateStart(mr);
                slend    = ensMapperresultGetCoordinateEnd(mr);
                slstrand = ensMapperresultGetCoordinateStrand(mr);

                /*
                ** Delete the Sequence Region Slice and fetch a Slice
                ** in the Coordinate System we just mapped to.
                */

                ensSliceDel(&srslice);

                ensSliceadaptorFetchBySeqregionIdentifier(sa,
                                                          srid,
                                                          0,
                                                          0,
                                                          0,
                                                          &srslice);

                ensMapperresultDel(&mr);
            }

            /*
            ** Convert Sequence Region Slice coordinates to destination Slice
            ** coordinates, if a destination Slice has been provided.
            */

            if(slice)
            {
                /* Check that the length of the Slice is within range. */

                if(ensSliceCalculateLength(slice) <= INT_MAX)
                    sllength = (ajint) ensSliceCalculateLength(slice);
                else
                    ajFatal("miscellaneousfeatureadaptorFetchAllbyStatement "
                            "got a Slice, "
                            "which length (%u) exceeds the "
                            "maximum integer limit (%d).",
                            ensSliceCalculateLength(slice), INT_MAX);

                /*
                ** Nothing needs to be done if the destination Slice starts
                ** at 1 and is on the forward strand.
                */

                if((ensSliceGetStart(slice) != 1) ||
                   (ensSliceGetStrand(slice) < 0))
                {
                    if(ensSliceGetStrand(slice) >= 0)
                    {
                        slstart = slstart - ensSliceGetStart(slice) + 1;

                        slend = slend - ensSliceGetStart(slice) + 1;
                    }
                    else
                    {
                        slend = ensSliceGetEnd(slice) - slstart + 1;

                        slstart = ensSliceGetEnd(slice) - slend + 1;

                        slstrand *= -1;
                    }
                }

                /*
                ** Throw away Feature objects off the end of the requested
                ** Slice or on any other than the requested Slice.
                */

                if((slend < 1) ||
                   (slstart > sllength) ||
                   (srid != ensSliceGetSeqregionIdentifier(slice)))
                {
                    /* Load the next Feature but destroy first! */

                    ajStrDel(&avalue);

                    ensSliceDel(&srslice);

                    ensAssemblymapperDel(&am);

                    /* Mark the Miscellaneous Feature as one to throw away. */

                    throw = identifier;

                    continue;
                }

                /*
                ** Delete the Sequence Region Slice and set the
                ** requested Slice.
                */

                ensSliceDel(&srslice);

                srslice = ensSliceNewRef(slice);
            }

            /* Finally, create a new Ensembl Miscellaneous Feature. */

            feature = ensFeatureNewIniS((EnsPAnalysis) NULL,
                                        srslice,
                                        slstart,
                                        slend,
                                        slstrand);

            mf = ensMiscellaneousfeatureNewIni(mfa, identifier, feature);

            ajListPushAppend(mfs, (void*) mf);

            /* Add an Ensembl Attribute if one was defined. */

            if((atid > 0) && ajStrGetLen(avalue))
            {
                if(!miscellaneousfeatureadaptorHasAttribute(attributes,
                                                            atid,
                                                            avalue))
                {
                    ensAttributetypeadaptorFetchByIdentifier(ata, atid, &at);

                    attribute = ensAttributeNewIni(at, avalue);

                    ensAttributetypeDel(&at);

                    ensMiscellaneousfeatureAddAttribute(mf, attribute);

                    ensAttributeDel(&attribute);
                }
            }

            /* Add an Ensembl Miscellaneous Set if one was defined. */

            if(msid)
            {
                if(!miscellaneousfeatureadaptorHasMiscellaneousset(sets, msid))
                {
                    ensMiscellaneoussetadaptorFetchByIdentifier(msa,
                                                                msid,
                                                                &ms);

                    ensMiscellaneousfeatureAddMiscellaneousset(mf, ms);

                    ensMiscellaneoussetDel(&ms);
                }
            }

            ensFeatureDel(&feature);

            ensAssemblymapperDel(&am);

            ensSliceDel(&srslice);

            ajStrDel(&avalue);
        }
    }

    ajSqlrowiterDel(&sqli);

    ensDatabaseadaptorSqlstatementDel(dba, &sqls);

    ajListFree(&mrs);

    ajTableMapDel(attributes,
                  miscellaneousfeatureadaptorClearAttributes,
                  NULL);

    ajTableFree(&attributes);

    ajTableMapDel(sets,
                  miscellaneousfeatureadaptorClearMiscellaneoussets,
                  NULL);

    ajTableFree(&sets);

    return ajTrue;
}




/* @funcstatic miscellaneousfeatureadaptorCacheReference **********************
**
** Wrapper function to reference an Ensembl Miscellaneous Feature
** from an Ensembl Cache.
**
** @param [r] value [void*] Ensembl Miscellaneous Feature
**
** @return [void*] Ensembl Miscellaneous Feature or NULL
** @@
******************************************************************************/

static void* miscellaneousfeatureadaptorCacheReference(void* value)
{
    if(!value)
        return NULL;

    return (void*)
        ensMiscellaneousfeatureNewRef((EnsPMiscellaneousfeature) value);
}




/* @funcstatic miscellaneousfeatureadaptorCacheDelete *************************
**
** Wrapper function to delete an Ensembl Miscellaneous Feature
** from an Ensembl Cache.
**
** @param [r] value [void**] Ensembl Miscellaneous Feature address
**
** @return [void]
** @@
******************************************************************************/

static void miscellaneousfeatureadaptorCacheDelete(void** value)
{
    if(!value)
        return;

    ensMiscellaneousfeatureDel((EnsPMiscellaneousfeature*) value);

    return;
}




/* @funcstatic miscellaneousfeatureadaptorCacheSize ***************************
**
** Wrapper function to determine the memory size of an
** Ensembl Miscellaneous Feature from an Ensembl Cache.
**
** @param [r] value [const void*] Ensembl Miscellaneous Feature
**
** @return [size_t] Memory size in bytes or 0
** @@
******************************************************************************/

static size_t miscellaneousfeatureadaptorCacheSize(const void* value)
{
    if(!value)
        return 0;

    return ensMiscellaneousfeatureCalculateMemsize(
        (const EnsPMiscellaneousfeature) value);
}




/* @funcstatic miscellaneousfeatureadaptorGetFeature **************************
**
** Wrapper function to get the Ensembl Feature of an
** Ensembl Miscellaneous Feature from an Ensembl Feature Adaptor.
**
** @param [r] value [const void*] Ensembl Miscellaneous Feature
**
** @return [EnsPFeature] Ensembl Feature
** @@
******************************************************************************/

static EnsPFeature miscellaneousfeatureadaptorGetFeature(const void* value)
{
    if(!value)
        return NULL;

    return ensMiscellaneousfeatureGetFeature(
        (const EnsPMiscellaneousfeature) value);
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
** @valrule * [EnsPMiscellaneousfeatureadaptor] Ensembl Miscellaneous
**                                              Feature Adaptor or NULL
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
** @return [EnsPMiscellaneousfeatureadaptor] Ensembl Miscellaneous Feature
**                                           Adaptor or NULL
** @@
******************************************************************************/

EnsPMiscellaneousfeatureadaptor ensMiscellaneousfeatureadaptorNew(
    EnsPDatabaseadaptor dba)
{
    if(!dba)
        return NULL;

    return ensFeatureadaptorNew(
        dba,
        miscellaneousfeatureadaptorTables,
        miscellaneousfeatureadaptorColumns,
        miscellaneousfeatureadaptorLeftjoin,
        (const char*) NULL,
        miscellaneousfeatureadaptorFinalcondition,
        miscellaneousfeatureadaptorFetchAllbyStatement,
        (void* (*)(const void* key)) NULL,
        miscellaneousfeatureadaptorCacheReference,
        (AjBool (*)(const void* value)) NULL,
        miscellaneousfeatureadaptorCacheDelete,
        miscellaneousfeatureadaptorCacheSize,
        miscellaneousfeatureadaptorGetFeature,
        "Miscellaneous Feature");
}




/* @section destructors *******************************************************
**
** Destruction destroys all internal data structures and frees the
** memory allocated for an Ensembl Miscellaneous Set Adaptor object.
**
** @fdata [EnsPMiscellaneousfeatureadaptor]
**
** @nam3rule Del Destroy (free) an Ensembl Miscellaneous Feature Adaptor object
**
** @argrule * Pmfa [EnsPMiscellaneousfeatureadaptor*]
** Ensembl Miscellaneous Feature Adaptor object address
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
** Ensembl Miscellaneous Feature Adaptor object address
**
** @return [void]
** @@
******************************************************************************/

void ensMiscellaneousfeatureadaptorDel(
    EnsPMiscellaneousfeatureadaptor* Pmfa)
{
    if(!Pmfa)
        return;

    if(!*Pmfa)
        return;

    ensFeatureadaptorDel(Pmfa);

    *Pmfa = NULL;

    return;
}




/* @section element retrieval *************************************************
**
** Functions for returning elements of an
** Ensembl Miscellaneous Feature Adaptor object.
**
** @fdata [EnsPMiscellaneousfeatureadaptor]
**
** @nam3rule Get Return Ensembl Miscellaneous Feature Adaptor attribute(s)
** @nam4rule Databaseadaptor Return the Ensembl Database Adaptor
**
** @argrule * mfa [EnsPMiscellaneousfeatureadaptor] Ensembl Miscellaneous
**                                                  Feature Adaptor
**
** @valrule Databaseadaptor [EnsPDatabaseadaptor] Ensembl Database Adaptor
**                                                or NULL
**
** @fcategory use
******************************************************************************/




/* @func ensMiscellaneousfeatureadaptorGetDatabaseadaptor *********************
**
** Get the Ensembl Database Adaptor element of an
** Ensembl Miscellaneous Feature Adaptor.
**
** @param [u] mfa [EnsPMiscellaneousfeatureadaptor] Ensembl Miscellaneous
**                                                  Feature Adaptor
**
** @return [EnsPDatabaseadaptor] Ensembl Database Adaptor or NULL
** @@
******************************************************************************/

EnsPDatabaseadaptor ensMiscellaneousfeatureadaptorGetDatabaseadaptor(
    EnsPMiscellaneousfeatureadaptor mfa)
{
    if(!mfa)
        return NULL;

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
** @argrule * mfa [EnsPMiscellaneousfeatureadaptor] Ensembl Miscellaneous
**                                                  Feature Adaptor
** @argrule FetchAll mfs [AjPList] AJAX List of Ensembl Miscellaneous Feature
**                                 objects
** @argrule AllbyAttributecodevalue code [const AjPStr] Ensembl Attribute
** code
** @argrule AllbyAttributecodevalue value [const AjPStr] Ensembl Attribute
** value
** @argrule AllbyAttributecodevalue mfs [AjPList] AJAX List of
** Ensembl Miscellaneous Feature objects
** @argrule AllbySlicecodes slice [EnsPSlice] Ensembl Slice
** @argrule AllbySlicecodes codes [const AjPList] AJAX List of AJAX String
** @argrule AllbySlicecodes mfs [AjPList] AJAX List of Ensembl Miscellaneous
** Feature objects
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
** @param [u] mfa [EnsPMiscellaneousfeatureadaptor] Ensembl Miscellaneous
**                                                  Feature Adaptor
** @param [r] code [const AjPStr] Ensembl Attribute code
** @param [r] value [const AjPStr] Ensembl Attribute value
** @param [u] mfs [AjPList] AJAX List of Ensembl Miscellaneous Feature objects
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
** @@
******************************************************************************/

AjBool ensMiscellaneousfeatureadaptorFetchAllbyAttributecodevalue(
    EnsPMiscellaneousfeatureadaptor mfa,
    const AjPStr code,
    const AjPStr value,
    AjPList mfs)
{
    register ajuint i = 0;

    ajuint mfid = 0;

    char* txtvalue = NULL;

    const char* template = "misc_feature.misc_feature_id in (%S)";

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

    if(!mfa)
        return ajFalse;

    if(!code)
        ajFatal("ensMiscellaneousfeatureadaptorFetchAllbyAttributecodevalue "
                "requires an Ensembl Attribute code.\n");

    ba = ensFeatureadaptorGetBaseadaptor(mfa);

    dba = ensFeatureadaptorGetDatabaseadaptor(mfa);

    ata = ensRegistryGetAttributetypeadaptor(dba);

    ensAttributetypeadaptorFetchByCode(ata, code, &at);

    if(!at)
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

    if(value)
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

    while(!ajSqlrowiterDone(sqli))
    {
        mfid = 0;

        sqlr = ajSqlrowiterGet(sqli);

        ajSqlcolumnToUint(sqlr, &mfid);

        ajFmtPrintAppS(&csv, "%u, ", mfid);

        i++;

        if((i % ensBaseadaptorMaximumIdentifiers) == 0)
        {
            /*
            ** Remove the last comma and space from the
            ** comma-separated values.
            */

            ajStrCutEnd(&csv, 2);

            constraint = ajFmtStr(template, csv);

            ensBaseadaptorFetchAllbyConstraint(ba,
                                               constraint,
                                               (EnsPAssemblymapper) NULL,
                                               (EnsPSlice) NULL,
                                               mfs);

            ajStrDel(&constraint);

            ajStrSetClear(&csv);
        }
    }

    ajSqlrowiterDel(&sqli);

    ensDatabaseadaptorSqlstatementDel(dba, &sqls);

    ajStrDel(&statement);

    /* Run the last statement, but remove the last comma and space first. */

    ajStrCutEnd(&csv, 2);

    constraint = ajFmtStr(template, csv);

    ensBaseadaptorFetchAllbyConstraint(ba,
                                       constraint,
                                       (EnsPAssemblymapper) NULL,
                                       (EnsPSlice) NULL,
                                       mfs);

    ajStrDel(&constraint);
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
** @param [u] mfa [EnsPMiscellaneousfeatureadaptor] Ensembl Miscellaneous
**                                                  Feature Adaptor
** @param [u] slice [EnsPSlice] Ensembl Slice
** @param [r] codes [const AjPList] AJAX List of AJAX String
** (Ensembl Miscellaneous Set code) objects
** @param [u] mfs [AjPList] AJAX List of Ensembl Miscellaneous Feature objects
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
** @@
******************************************************************************/

AjBool ensMiscellaneousfeatureadaptorFetchAllbySlicecodes(
    EnsPMiscellaneousfeatureadaptor mfa,
    EnsPSlice slice,
    const AjPList codes,
    AjPList mfs)
{
    ajuint maxlen = 0;

    AjIList iter = NULL;

    AjPStr code       = NULL;
    AjPStr constraint = NULL;
    AjPStr csv        = NULL;

    EnsPDatabaseadaptor dba = NULL;

    EnsPMiscellaneousset ms         = NULL;
    EnsPMiscellaneoussetadaptor msa = NULL;

    if(!mfa)
        return ajFalse;

    if(!slice)
        return ajFalse;

    if(!codes)
        return ajFalse;

    if(!mfs)
        return ajFalse;

    dba = ensFeatureadaptorGetDatabaseadaptor(mfa);

    msa = ensRegistryGetMiscellaneoussetadaptor(dba);

    csv = ajStrNew();

    iter = ajListIterNewread(codes);

    while(!ajListIterDone(iter))
    {
        code = (AjPStr) ajListIterGet(iter);

        ensMiscellaneoussetadaptorFetchByCode(msa, code, &ms);

        if(ms)
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

    if(ajStrGetLen(csv) > 0)
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
** @argrule * mfa [EnsPMiscellaneousfeatureadaptor] Ensembl Miscellaneous
** Feature Adaptor
** @argrule AllIdentifiers identifiers [AjPList] AJAX List of AJAX unsigned
**                                               integer identifiers
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
** @param [u] mfa [EnsPMiscellaneousfeatureadaptor] Ensembl Miscellaneous
**                                                  Feature Adaptor
** @param [u] identifiers [AjPList] AJAX List of AJAX unsigned integers
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
** @@
******************************************************************************/

AjBool ensMiscellaneousfeatureadaptorRetrieveAllIdentifiers(
    EnsPMiscellaneousfeatureadaptor mfa,
    AjPList identifiers)
{
    AjBool result = AJFALSE;

    AjPStr table = NULL;

    EnsPBaseadaptor ba = NULL;

    if(!mfa)
        return ajFalse;

    if(!identifiers)
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
