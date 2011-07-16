/* @source Ensembl Karyotype Band functions
**
** @author Copyright (C) 1999 Ensembl Developers
** @author Copyright (C) 2006 Michael K. Schuster
** @modified 2009 by Alan Bleasby for incorporation into EMBOSS core
** @modified $Date: 2011/07/06 21:50:28 $ by $Author: mks $
** @version $Revision: 1.33 $
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

#include "enskaryotype.h"




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

/* @conststatic karyotypebandadaptorTables ************************************
**
** Array of Ensembl Karyotype Band Adaptor SQL table names
**
******************************************************************************/

static const char* const karyotypebandadaptorTables[] =
{
    "karyotype",
    (const char*) NULL
};




/* @conststatic karyotypebandadaptorColumns ***********************************
**
** Array of Ensembl Karyotype Band Adaptor SQL column names
**
******************************************************************************/

static const char* const karyotypebandadaptorColumns[] =
{
    "karyotype.karyotype_id",
    "karyotype.seq_region_id",
    "karyotype.seq_region_start",
    "karyotype.seq_region_end",
    "karyotype.band",
    "karyotype.stain",
    (const char*) NULL
};




/* ==================================================================== */
/* ======================== private variables ========================= */
/* ==================================================================== */




/* ==================================================================== */
/* ======================== private functions ========================= */
/* ==================================================================== */

static int listKaryotypebandCompareStartAscending(const void* P1,
                                                  const void* P2);

static int listKaryotypebandCompareStartDescending(const void* P1,
                                                   const void* P2);

static AjBool karyotypebandadaptorFetchAllbyStatement(
    EnsPDatabaseadaptor dba,
    const AjPStr statement,
    EnsPAssemblymapper am,
    EnsPSlice slice,
    AjPList kblist);

static void* karyotypebandadaptorCacheReference(void* value);

static void karyotypebandadaptorCacheDelete(void** value);

static size_t karyotypebandadaptorCacheSize(const void* value);

static EnsPFeature karyotypebandadaptorGetFeature(const void* value);




/* ==================================================================== */
/* ===================== All functions by section ===================== */
/* ==================================================================== */




/* @filesection enskaryotype **************************************************
**
** @nam1rule ens Function belongs to the Ensembl library
**
******************************************************************************/




/* @datasection [EnsPKaryotypeband] Ensembl Karyotype Band ********************
**
** @nam2rule Karyotypeband Functions for manipulating
** Ensembl Karyotype Band objects
**
** @cc Bio::EnsEMBL::Karyotypeband
** @cc CVS Revision: 1.10
** @cc CVS Tag: branch-ensembl-62
**
******************************************************************************/




/* @section constructors ******************************************************
**
** All constructors return a new Ensembl Karyotype Band by pointer.
** It is the responsibility of the user to first destroy any previous
** Karyotype Band. The target pointer does not need to be initialised to
** NULL, but it is good programming practice to do so anyway.
**
** @fdata [EnsPKaryotypeband]
**
** @nam3rule New Constructor
** @nam4rule Cpy Constructor with existing object
** @nam4rule Ini Constructor with initial values
** @nam4rule Ref Constructor by incrementing the reference counter
**
** @argrule Cpy kb [const EnsPKaryotypeband] Ensembl Karyotype Band
** @argrule Ini kba [EnsPKaryotypebandadaptor] Ensembl Karyotype Band Adaptor
** @argrule Ini identifier [ajuint] Identifier
** @argrule Ini feature [EnsPFeature] Ensembl Feature
** @argrule Ini name [AjPStr] Name
** @argrule Ini stain [AjPStr] Stain
** @argrule Ref kb [EnsPKaryotypeband] Ensembl Karyotype Band
**
** @valrule * [EnsPKaryotypeband] Ensembl Karyotype Band or NULL
**
** @fcategory new
******************************************************************************/




/* @func ensKaryotypebandNewCpy ***********************************************
**
** Object-based constructor function, which returns an independent object.
**
** @param [r] kb [const EnsPKaryotypeband] Ensembl Karyotype Band
**
** @return [EnsPKaryotypeband] Ensembl Karyotype Band or NULL
** @@
******************************************************************************/

EnsPKaryotypeband ensKaryotypebandNewCpy(const EnsPKaryotypeband kb)
{
    EnsPKaryotypeband pthis = NULL;

    if(!kb)
        return NULL;

    AJNEW0(pthis);

    pthis->Use = 1;

    pthis->Identifier = kb->Identifier;

    pthis->Adaptor = kb->Adaptor;

    pthis->Feature = ensFeatureNewRef(kb->Feature);

    if(kb->Name)
        pthis->Name = ajStrNewRef(kb->Name);

    if(kb->Stain)
        pthis->Stain = ajStrNewRef(kb->Stain);

    return pthis;
}




/* @func ensKaryotypebandNewIni ***********************************************
**
** Constructor for an Ensembl Karyotype Band with initial values.
**
** @cc Bio::EnsEMBL::Karyotypeband::new
** @param [u] kba [EnsPKaryotypebandadaptor] Ensembl Karyotype Band Adaptor
** @param [r] identifier [ajuint] Identifier
** @cc Bio::EnsEMBL::Feature::new
** @param [u] feature [EnsPFeature] Ensembl Feature
** @param [u] name [AjPStr] Name
** @param [u] stain [AjPStr] Stain
**
** @return [EnsPKaryotypeband] Ensembl Karyotype Band or NULL
** @@
******************************************************************************/

EnsPKaryotypeband ensKaryotypebandNewIni(EnsPKaryotypebandadaptor kba,
                                         ajuint identifier,
                                         EnsPFeature feature,
                                         AjPStr name,
                                         AjPStr stain)
{
    EnsPKaryotypeband kb = NULL;

    if(!feature)
        return NULL;

    AJNEW0(kb);

    kb->Use = 1;

    kb->Identifier = identifier;

    kb->Adaptor = kba;

    kb->Feature = ensFeatureNewRef(feature);

    if(name)
        kb->Name = ajStrNewRef(name);

    if(stain)
        kb->Stain = ajStrNewRef(stain);

    return kb;
}




/* @func ensKaryotypebandNewRef ***********************************************
**
** Ensembl Object referencing function, which returns a pointer to the
** Ensembl Object passed in and increases its reference count.
**
** @param [u] kb [EnsPKaryotypeband] Ensembl Karyotype Band
**
** @return [EnsPKaryotypeband] Ensembl Karyotype Band or NULL
** @@
******************************************************************************/

EnsPKaryotypeband ensKaryotypebandNewRef(EnsPKaryotypeband kb)
{
    if(!kb)
        return NULL;

    kb->Use++;

    return kb;
}




/* @section destructors *******************************************************
**
** Destruction destroys all internal data structures and frees the
** memory allocated for an Ensembl Karyotype Band object.
**
** @fdata [EnsPKaryotypeband]
**
** @nam3rule Del Destroy (free) an Ensembl Karyotype Band object
**
** @argrule * Pkb [EnsPKaryotypeband*] Ensembl Karyotype Band object address
**
** @valrule * [void]
**
** @fcategory delete
******************************************************************************/




/* @func ensKaryotypebandDel **************************************************
**
** Default destructor for an Ensembl Karyotype Band.
**
** @param [d] Pkb [EnsPKaryotypeband*] Ensembl Karyotype Band object address
**
** @return [void]
** @@
******************************************************************************/

void ensKaryotypebandDel(EnsPKaryotypeband* Pkb)
{
    EnsPKaryotypeband pthis = NULL;

    if(!Pkb)
        return;

    if(!*Pkb)
        return;

    pthis = *Pkb;

    pthis->Use--;

    if(pthis->Use)
    {
        *Pkb = NULL;

        return;
    }

    ensFeatureDel(&pthis->Feature);

    ajStrDel(&pthis->Name);
    ajStrDel(&pthis->Stain);

    AJFREE(pthis);

    *Pkb = NULL;

    return;
}




/* @section element retrieval *************************************************
**
** Functions for returning elements of an Ensembl Karyotype Band object.
**
** @fdata [EnsPKaryotypeband]
**
** @nam3rule Get Return Karyotype Band attribute(s)
** @nam4rule Adaptor Return the Ensembl Karyotype Band Adaptor
** @nam4rule Feature Return the Ensembl Feature
** @nam4rule Identifier Return the SQL database-internal identifier
** @nam4rule Name Return the name
** @nam4rule Stain Return the stain
**
** @argrule * kb [const EnsPKaryotypeband] Karyotype Band
**
** @valrule Adaptor [EnsPKaryotypebandadaptor] Ensembl Karyotype Band Adaptor
** or NULL
** @valrule Feature [EnsPFeature] Ensembl Feature or NULL
** @valrule Identifier [ajuint] SQL database-internal identifier or 0
** @valrule Name [AjPStr] Name or NULL
** @valrule Stain [AjPStr] Stain or NULL
**
** @fcategory use
******************************************************************************/




/* @func ensKaryotypebandGetAdaptor *******************************************
**
** Get the Ensembl Karyotype Band Adaptor element of an Ensembl Karyotype Band.
**
** @param [r] kb [const EnsPKaryotypeband] Ensembl Karyotype Band
**
** @return [EnsPKaryotypebandadaptor] Ensembl Karyotype Band Adaptor or NULL
** @@
******************************************************************************/

EnsPKaryotypebandadaptor ensKaryotypebandGetAdaptor(
    const EnsPKaryotypeband kb)
{
    if(!kb)
        return NULL;

    return kb->Adaptor;
}




/* @func ensKaryotypebandGetFeature *******************************************
**
** Get the Ensembl Feature element of an Ensembl Karyotype Band.
**
** @param [r] kb [const EnsPKaryotypeband] Ensembl Karyotype Band
**
** @return [EnsPFeature] Ensembl Feature or NULL
** @@
******************************************************************************/

EnsPFeature ensKaryotypebandGetFeature(
    const EnsPKaryotypeband kb)
{
    if(!kb)
        return NULL;

    return kb->Feature;
}




/* @func ensKaryotypebandGetIdentifier ****************************************
**
** Get the SQL database-internal identifier element of an
** Ensembl Karyotype Band.
**
** @param [r] kb [const EnsPKaryotypeband] Ensembl Karyotype Band
**
** @return [ajuint] SQL database-internal identifier or 0
** @@
******************************************************************************/

ajuint ensKaryotypebandGetIdentifier(
    const EnsPKaryotypeband kb)
{
    if(!kb)
        return 0;

    return kb->Identifier;
}




/* @func ensKaryotypebandGetName **********************************************
**
** Get the name element of an Ensembl Karyotype Band.
**
** @param [r] kb [const EnsPKaryotypeband] Ensembl Karyotype Band
**
** @return [AjPStr] Name or NULL
** @@
******************************************************************************/

AjPStr ensKaryotypebandGetName(
    const EnsPKaryotypeband kb)
{
    if(!kb)
        return NULL;

    return kb->Name;
}




/* @func ensKaryotypebandGetStain *********************************************
**
** Get the stain element of an Ensembl Karyotype Band.
**
** @param [r] kb [const EnsPKaryotypeband] Ensembl Karyotype Band
**
** @return [AjPStr] Stain or NULL
** @@
******************************************************************************/

AjPStr ensKaryotypebandGetStain(
    const EnsPKaryotypeband kb)
{
    if(!kb)
        return NULL;

    return kb->Stain;
}




/* @section modifiers *********************************************************
**
** Functions for assigning elements of an Ensembl Karyotype Band object.
**
** @fdata [EnsPKaryotypeband]
**
** @nam3rule Set Set one element of an Karyotype Band
** @nam4rule Adaptor Set the Ensembl Karyotype Band Adaptor
** @nam4rule Feature Set the Ensembl Feature
** @nam4rule Identifier Set the SQL database-internal identifier
** @nam4rule Name Set the name
** @nam4rule Stain Set the stain
**
** @argrule * kb [EnsPKaryotypeband] Ensembl Karyotype Band object
** @argrule Adaptor kba [EnsPKaryotypebandadaptor] Ensembl Karyotype Band
** Adaptor
** @argrule Feature feature [EnsPFeature] Ensembl Feature
** @argrule Identifier identifier [ajuint] Database identifier
** @argrule Name name [AjPStr] Name
** @argrule Stain stain [AjPStr] Logic name
**
** @valrule * [AjBool] ajTrue upon success, ajFalse otherwise
**
** @fcategory modify
******************************************************************************/




/* @func ensKaryotypebandSetAdaptor *******************************************
**
** Set the Ensembl Karyotype Band Adaptor element of an Ensembl Karyotype Band.
**
** @param [u] kb [EnsPKaryotypeband] Ensembl Karyotype Band
** @param [u] kba [EnsPKaryotypebandadaptor] Ensembl Karyotype Band Adaptor
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
** @@
******************************************************************************/

AjBool ensKaryotypebandSetAdaptor(EnsPKaryotypeband kb,
                                  EnsPKaryotypebandadaptor kba)
{
    if(!kb)
        return ajFalse;

    kb->Adaptor = kba;

    return ajTrue;
}




/* @func ensKaryotypebandSetFeature *******************************************
**
** Set the Ensembl Feature element of an Ensembl Karyotype Band.
**
** @param [u] kb [EnsPKaryotypeband] Ensembl Karyotype Band
** @param [u] feature [EnsPFeature] Ensembl Feature
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
** @@
******************************************************************************/

AjBool ensKaryotypebandSetFeature(EnsPKaryotypeband kb,
                                  EnsPFeature feature)
{
    if(!kb)
        return ajFalse;

    ensFeatureDel(&kb->Feature);

    kb->Feature = ensFeatureNewRef(feature);

    return ajTrue;
}




/* @func ensKaryotypebandSetIdentifier ****************************************
**
** Set the SQL database-internal identifier element of an
** Ensembl Karyotype Band.
**
** @param [u] kb [EnsPKaryotypeband] Ensembl Karyotype Band
** @param [r] identifier [ajuint] Database identifier
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
** @@
******************************************************************************/

AjBool ensKaryotypebandSetIdentifier(EnsPKaryotypeband kb,
                                     ajuint identifier)
{
    if(!kb)
        return ajFalse;

    kb->Identifier = identifier;

    return ajTrue;
}




/* @func ensKaryotypebandSetName **********************************************
**
** Set the name element of an Ensembl Karyotype Band.
**
** @param [u] kb [EnsPKaryotypeband] Ensembl Karyotype Band
** @param [u] name [AjPStr] Name
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
** @@
******************************************************************************/

AjBool ensKaryotypebandSetName(EnsPKaryotypeband kb,
                               AjPStr name)
{
    if(!kb)
        return ajFalse;

    ajStrDel(&kb->Name);

    kb->Name = ajStrNewRef(name);

    return ajTrue;
}




/* @func ensKaryotypebandSetStain *********************************************
**
** Set the stain element of an Ensembl Karyotype Band.
**
** @param [u] kb [EnsPKaryotypeband] Ensembl Karyotype Band
** @param [u] stain [AjPStr] Logic name
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
** @@
******************************************************************************/

AjBool ensKaryotypebandSetStain(EnsPKaryotypeband kb,
                                AjPStr stain)
{
    if(!kb)
        return ajFalse;

    ajStrDel(&kb->Stain);

    kb->Stain = ajStrNewRef(stain);

    return ajTrue;
}




/* @section debugging *********************************************************
**
** Functions for reporting of an Ensembl Karyotype Band object.
**
** @fdata [EnsPKaryotypeband]
**
** @nam3rule Trace Report Ensembl Karyotype Band elements to debug file
**
** @argrule Trace kb [const EnsPKaryotypeband] Ensembl Karyotype Band
** @argrule Trace level [ajuint] Indentation level
**
** @valrule * [AjBool] ajTrue upon success, ajFalse otherwise
**
** @fcategory misc
******************************************************************************/




/* @func ensKaryotypebandTrace ************************************************
**
** Trace an Ensembl Karyotype Band.
**
** @param [r] kb [const EnsPKaryotypeband] Ensembl Karyotype Band
** @param [r] level [ajuint] Indentation level
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
** @@
******************************************************************************/

AjBool ensKaryotypebandTrace(const EnsPKaryotypeband kb, ajuint level)
{
    AjPStr indent = NULL;

    if(!kb)
        return ajFalse;

    indent = ajStrNew();

    ajStrAppendCountK(&indent, ' ', level * 2);

    ajDebug("%SensKaryotypebandTrace %p\n"
            "%S  Adaptor %p\n"
            "%S  Identifier %u\n"
            "%S  Feature %p\n"
            "%S  Name '%S'\n"
            "%S  Stain '%S'\n",
            indent, kb,
            indent, kb->Adaptor,
            indent, kb->Identifier,
            indent, kb->Feature,
            indent, kb->Name,
            indent, kb->Stain);

    ensFeatureTrace(kb->Feature, 1);

    ajStrDel(&indent);

    return ajTrue;
}




/* @section calculate *********************************************************
**
** Functions for calculating values of an Ensembl Karyotype Band object.
**
** @fdata [EnsPKaryotypeband]
**
** @nam3rule Calculate Calculate Ensembl Karyotype Band values
** @nam4rule Memsize Calculate the memory size in bytes
**
** @argrule * kb [const EnsPKaryotypeband] Ensembl Karyotype Band
**
** @valrule Memsize [size_t] Memory size in bytes or 0
**
** @fcategory misc
******************************************************************************/




/* @func ensKaryotypebandCalculateMemsize *************************************
**
** Get the memory size in bytes of an Ensembl Karyotype Band.
**
** @param [r] kb [const EnsPKaryotypeband] Ensembl Karyotype Band
**
** @return [size_t] Memory size in bytes or 0
** @@
******************************************************************************/

size_t ensKaryotypebandCalculateMemsize(const EnsPKaryotypeband kb)
{
    size_t size = 0;

    if(!kb)
        return 0;

    size += sizeof (EnsOKaryotypeband);

    size += ensFeatureCalculateMemsize(kb->Feature);

    if(kb->Name)
    {
        size += sizeof (AjOStr);

        size += ajStrGetRes(kb->Name);
    }

    if(kb->Stain)
    {
        size += sizeof (AjOStr);

        size += ajStrGetRes(kb->Stain);
    }

    return size;
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
** @nam3rule Karyotypeband Functions for manipulating AJAX List objects of
** Ensembl Karyotype Band objects
** @nam4rule Sort Sort functions
** @nam5rule Start Sort by Ensembl Feature start element
** @nam6rule Ascending  Sort in ascending order
** @nam6rule Descending Sort in descending order
**
** @argrule Ascending kbs [AjPList] AJAX List of Ensembl Karyotype Band
** objects
** @argrule Descending kbs [AjPList] AJAX List of Ensembl Karyotype Band
** objects
**
** @valrule * [AjBool] ajTrue upon success, ajFalse otherwise
**
** @fcategory misc
******************************************************************************/




/* @funcstatic listKaryotypebandCompareStartAscending *************************
**
** AJAX List of Ensembl Karyotype Band objects comparison function to sort by
** Ensembl Feature start coordinate in ascending order.
**
** @param [r] P1 [const void*] Ensembl Karyotype Band address 1
** @param [r] P2 [const void*] Ensembl Karyotype Band address 2
** @see ajListSort
**
** @return [int] The comparison function returns an integer less than,
**               equal to, or greater than zero if the first argument is
**               considered to be respectively less than, equal to, or
**               greater than the second.
** @@
******************************************************************************/

static int listKaryotypebandCompareStartAscending(const void* P1,
                                                  const void* P2)
{
    const EnsPKaryotypeband kb1 = NULL;
    const EnsPKaryotypeband kb2 = NULL;

    kb1 = *(EnsPKaryotypeband const*) P1;
    kb2 = *(EnsPKaryotypeband const*) P2;

    if(ajDebugTest("listKaryotypebandCompareStartAscending"))
        ajDebug("listKaryotypebandCompareStartAscending\n"
                "  kb1 %p\n"
                "  kb2 %p\n",
                kb1,
                kb2);

    /* Sort empty values towards the end of the AJAX List. */

    if(kb1 && (!kb2))
        return -1;

    if((!kb1) && (!kb2))
        return 0;

    if((!kb1) && kb2)
        return +1;

    return ensFeatureCompareStartAscending(kb1->Feature, kb2->Feature);
}




/* @func ensListKaryotypebandSortStartAscending *******************************
**
** Sort an AJAX List of Ensembl Karyotype Band objects by their
** Ensembl Feature start coordinate in ascending order.
**
** @param [u] kbs [AjPList] AJAX List of Ensembl Karyotype Band objects
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
** @@
******************************************************************************/

AjBool ensListKaryotypebandSortStartAscending(AjPList kbs)
{
    if(!kbs)
        return ajFalse;

    ajListSort(kbs, listKaryotypebandCompareStartAscending);

    return ajTrue;
}




/* @funcstatic listKaryotypebandCompareStartDescending ************************
**
** AJAX List of Ensembl Karyotype Band objects comparison function to sort by
** Ensembl Feature start coordinate in descending order.
**
** @param [r] P1 [const void*] Ensembl Karyotype Band address 1
** @param [r] P2 [const void*] Ensembl Karyotype Band address 2
** @see ajListSort
**
** @return [int] The comparison function returns an integer less than,
**               equal to, or greater than zero if the first argument is
**               considered to be respectively less than, equal to, or
**               greater than the second.
** @@
******************************************************************************/

static int listKaryotypebandCompareStartDescending(const void* P1,
                                                   const void* P2)
{
    const EnsPKaryotypeband kb1 = NULL;
    const EnsPKaryotypeband kb2 = NULL;

    kb1 = *(EnsPKaryotypeband const*) P1;
    kb2 = *(EnsPKaryotypeband const*) P2;

    if(ajDebugTest("listKaryotypebandCompareStartDescending"))
        ajDebug("listKaryotypebandCompareStartDescending\n"
                "  kb1 %p\n"
                "  kb2 %p\n",
                kb1,
                kb2);

    /* Sort empty values towards the end of the AJAX List. */

    if(kb1 && (!kb2))
        return -1;

    if((!kb1) && (!kb2))
        return 0;

    if((!kb1) && kb2)
        return +1;

    return ensFeatureCompareStartDescending(kb1->Feature, kb2->Feature);
}




/* @func ensListKaryotypebandSortStartDescending ******************************
**
** Sort an AJAX List of Ensembl Karyotype Band objects by their
** Ensembl Feature start coordinate in descending order.
**
** @param [u] kbs [AjPList] AJAX List of Ensembl Karyotype band objects
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
** @@
******************************************************************************/

AjBool ensListKaryotypebandSortStartDescending(AjPList kbs)
{
    if(!kbs)
        return ajFalse;

    ajListSort(kbs, listKaryotypebandCompareStartDescending);

    return ajTrue;
}




/* @datasection [EnsPKaryotypebandadaptor] Ensembl Karyotype Band Adaptor *****
**
** @nam2rule Karyotypebandadaptor Functions for manipulating
** Ensembl Karyotype Band Adaptor objects
**
** @cc Bio::EnsEMBL::DBSQL::Karyotypebandadaptor
** @cc CVS Revision: 1.33
** @cc CVS Tag: branch-ensembl-62
**
******************************************************************************/




/* @funcstatic karyotypebandadaptorFetchAllbyStatement ************************
**
** Fetch all Ensembl Karyotype Band objects via an SQL statement.
**
** @param [u] dba [EnsPDatabaseadaptor] Ensembl Database Adaptor
** @param [r] statement [const AjPStr] SQL statement
** @param [uN] am [EnsPAssemblymapper] Ensembl Assembly Mapper
** @param [uN] slice [EnsPSlice] Ensembl Slice
** @param [u] kblist [AjPList] AJAX List of Ensembl Karyotype Band objects
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
** @@
******************************************************************************/

static AjBool karyotypebandadaptorFetchAllbyStatement(
    EnsPDatabaseadaptor dba,
    const AjPStr statement,
    EnsPAssemblymapper am,
    EnsPSlice slice,
    AjPList kblist)
{
    ajuint identifier = 0;

    ajuint srid    = 0;
    ajuint srstart = 0;
    ajuint srend   = 0;

    AjPStr name  = NULL;
    AjPStr stain = NULL;

    AjPSqlstatement sqls = NULL;
    AjISqlrow sqli       = NULL;
    AjPSqlrow sqlr       = NULL;

    EnsPCoordsystemadaptor csa = NULL;

    EnsPFeature feature = NULL;

    EnsPKaryotypeband kb         = NULL;
    EnsPKaryotypebandadaptor kba = NULL;

    EnsPSlice srslice   = NULL;
    EnsPSliceadaptor sa = NULL;

    if(ajDebugTest("karyotypebandadaptorFetchAllbyStatement"))
        ajDebug("karyotypebandadaptorFetchAllbyStatement\n"
                "  dba %p\n"
                "  statement %p\n"
                "  am %p\n"
                "  slice %p\n"
                "  kblist %p\n",
                dba,
                statement,
                am,
                slice,
                kblist);

    if(!dba)
        return ajFalse;

    if(!statement)
        return ajFalse;

    (void) am;

    (void) slice;

    if(!kblist)
        return ajFalse;

    csa = ensRegistryGetCoordsystemadaptor(dba);

    kba = ensRegistryGetKaryotypebandadaptor(dba);

    sa = ensRegistryGetSliceadaptor(dba);

    sqls = ensDatabaseadaptorSqlstatementNew(dba, statement);

    sqli = ajSqlrowiterNew(sqls);

    while(!ajSqlrowiterDone(sqli))
    {
        identifier = 0;
        srid       = 0;
        srstart    = 0;
        srend      = 0;
        name       = ajStrNew();
        stain      = ajStrNew();

        sqlr = ajSqlrowiterGet(sqli);

        ajSqlcolumnToUint(sqlr, &identifier);
        ajSqlcolumnToUint(sqlr, &srid);
        ajSqlcolumnToUint(sqlr, &srstart);
        ajSqlcolumnToUint(sqlr, &srend);
        ajSqlcolumnToStr(sqlr, &name);
        ajSqlcolumnToStr(sqlr, &stain);

        /* Need to get the internal Ensembl Sequence Region identifier. */

        srid = ensCoordsystemadaptorGetSeqregionidentifierInternal(csa, srid);

        ensSliceadaptorFetchBySeqregionIdentifier(sa, srid, 0, 0, 0, &srslice);

        feature = ensFeatureNewIniS((EnsPAnalysis) NULL,
                                    srslice,
                                    srstart,
                                    srend,
                                    1);

        kb = ensKaryotypebandNewIni(kba, identifier, feature, name, stain);

        ajListPushAppend(kblist, (void*) kb);

        ensFeatureDel(&feature);

        ensSliceDel(&srslice);

        ajStrDel(&name);
        ajStrDel(&stain);
    }

    ajSqlrowiterDel(&sqli);

    ensDatabaseadaptorSqlstatementDel(dba, &sqls);

    return ajTrue;
}




/* @funcstatic karyotypebandadaptorCacheReference *****************************
**
** Wrapper function to reference an Ensembl Karyotype Band
** from an Ensembl Cache.
**
** @param [r] value [void*] Ensembl Karyotype Band
**
** @return [void*] Ensembl Karyotype Band or NULL
** @@
******************************************************************************/

static void* karyotypebandadaptorCacheReference(void* value)
{
    if(!value)
        return NULL;

    return (void*) ensKaryotypebandNewRef((EnsPKaryotypeband) value);
}




/* @funcstatic karyotypebandadaptorCacheDelete ********************************
**
** Wrapper function to delete an Ensembl Karyotype Band from an Ensembl Cache.
**
** @param [r] value [void**] Ensembl Karyotype Band
**
** @return [void]
** @@
******************************************************************************/

static void karyotypebandadaptorCacheDelete(void** value)
{
    if(!value)
        return;

    ensKaryotypebandDel((EnsPKaryotypeband*) value);

    return;
}




/* @funcstatic karyotypebandadaptorCacheSize **********************************
**
** Wrapper function to determine the memory size of an Ensembl Karyotype Band
** from an Ensembl Cache.
**
** @param [r] value [const void*] Ensembl Karyotype Band
**
** @return [size_t] Memory size in bytes or 0
** @@
******************************************************************************/

static size_t karyotypebandadaptorCacheSize(const void* value)
{
    if(!value)
        return 0;

    return ensKaryotypebandCalculateMemsize((const EnsPKaryotypeband) value);
}




/* @funcstatic karyotypebandadaptorGetFeature *********************************
**
** Wrapper function to get the Ensembl Feature element from an
** Ensembl Karyotype Band.
**
** @param [r] value [const void*] Ensembl Karyotype Band
**
** @return [EnsPFeature] Ensembl Feature or NULL
** @@
******************************************************************************/

static EnsPFeature karyotypebandadaptorGetFeature(const void* value)
{
    if(!value)
        return NULL;

    return ensKaryotypebandGetFeature((const EnsPKaryotypeband) value);
}




/* @section constructors ******************************************************
**
** All constructors return a new Ensembl Karyotype Band Adaptor by pointer.
** It is the responsibility of the user to first destroy any previous
** Karyotype Band Adaptor. The target pointer does not need to be
** initialised to NULL, but it is good programming practice to do so anyway.
**
** @fdata [EnsPKaryotypebandadaptor]
**
** @nam3rule New Constructor
**
** @argrule New dba [EnsPDatabaseadaptor] Ensembl Database Adaptor
**
** @valrule * [EnsPKaryotypebandadaptor] Ensembl Karyotype Band Adaptor or NULL
**
** @fcategory new
******************************************************************************/




/* @func ensKaryotypebandadaptorNew *******************************************
**
** Default constructor for an Ensembl Karyotype Band Adaptor.
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
** @see ensRegistryGetKaryotypebandadaptor
**
** @param [u] dba [EnsPDatabaseadaptor] Ensembl Database Adaptor
**
** @return [EnsPKaryotypebandadaptor] Ensembl Karyotype Band Adaptor or NULL
** @@
******************************************************************************/

EnsPKaryotypebandadaptor ensKaryotypebandadaptorNew(
    EnsPDatabaseadaptor dba)
{
    if(!dba)
        return NULL;

    return ensFeatureadaptorNew(
        dba,
        karyotypebandadaptorTables,
        karyotypebandadaptorColumns,
        (EnsPBaseadaptorLeftjoin) NULL,
        (const char*) NULL,
        (const char*) NULL,
        karyotypebandadaptorFetchAllbyStatement,
        (void* (*)(const void* key)) NULL,
        karyotypebandadaptorCacheReference,
        (AjBool (*)(const void* value)) NULL,
        karyotypebandadaptorCacheDelete,
        karyotypebandadaptorCacheSize,
        karyotypebandadaptorGetFeature,
        "Karyotype Band");
}




/* @section destructors *******************************************************
**
** Destruction destroys all internal data structures and frees the
** memory allocated for an Ensembl Karyotype Band Adaptor object.
**
** @fdata [EnsPKaryotypebandadaptor]
**
** @nam3rule Del Destroy (free) an Ensembl Karyotype Band Adaptor object
**
** @argrule * Pkba [EnsPKaryotypebandadaptor*] Ensembl Karyotype Band Adaptor
**                                             object address
**
** @valrule * [void]
**
** @fcategory delete
******************************************************************************/




/* @func ensKaryotypebandadaptorDel *******************************************
**
** Default destructor for an Ensembl Karyotype Band Adaptor.
**
** Ensembl Object Adaptors are singleton objects that are registered in the
** Ensembl Registry and weakly referenced by Ensembl Objects that have been
** instantiated by it. Therefore, Ensembl Object Adaptors should never be
** destroyed directly. Upon exit, the Ensembl Registry will call this function
** if required.
**
** @param [d] Pkba [EnsPKaryotypebandadaptor*] Ensembl Karyotype Band Adaptor
**                                             object address
**
** @return [void]
** @@
******************************************************************************/

void ensKaryotypebandadaptorDel(EnsPKaryotypebandadaptor* Pkba)
{
    if(!Pkba)
        return;

    if(!*Pkba)
        return;

    ensFeatureadaptorDel(Pkba);

    *Pkba = NULL;

    return;
}




/* @section element retrieval *************************************************
**
** Functions for returning elements of an
** Ensembl Karyotype Band Adaptor object.
**
** @fdata [EnsPKaryotypebandadaptor]
**
** @nam3rule Get Return Ensembl Karyotype Band Adaptor attribute(s)
** @nam4rule GetDatabaseadaptor Return the Ensembl Database Adaptor
**
** @argrule * kba [EnsPKaryotypebandadaptor] Ensembl Karyotype Band Adaptor
**
** @valrule Databaseadaptor [EnsPDatabaseadaptor] Ensembl Database Adaptor
** or NULL
**
** @fcategory use
******************************************************************************/




/* @func ensKaryotypebandadaptorGetDatabaseadaptor ****************************
**
** Get the Ensembl Database Adaptor element of an
** Ensembl Karyotype Band Adaptor.
**
** @param [u] kba [EnsPKaryotypebandadaptor] Ensembl Karyotype Band Adaptor
**
** @return [EnsPDatabaseadaptor] Ensembl Database Adaptor or NULL
** @@
******************************************************************************/

EnsPDatabaseadaptor ensKaryotypebandadaptorGetDatabaseadaptor(
    EnsPKaryotypebandadaptor kba)
{
    if(!kba)
        return NULL;

    return ensFeatureadaptorGetDatabaseadaptor(kba);
}




/* @section canonical object retrieval ****************************************
**
** Functions for fetching Ensembl Karyotype Band objects from an
** Ensembl SQL database.
**
** @fdata [EnsPKaryotypebandadaptor]
**
** @nam3rule Fetch Retrieve Ensembl Karyotype Band object(s)
** @nam4rule All Retrieve all Ensembl Karyotype Band objects
** @nam4rule Allby Retrieve all Ensembl Karyotype Band objects matching a
** criterion
** @nam5rule Chromosomeband Fetch all by a chromosome band
** @nam5rule Chromosomename Fetch all by a chromosome name
** @nam5rule Slice Fetch all by an Ensembl Slice
** @nam4rule By Fetch one Ensembl Karyotype Band object matching a criterion
** @nam5rule Identifier Fetch by SQL database-internal identifier
**
** @argrule * kba [EnsPKaryotypebandadaptor] Ensembl Karyotype Band Adaptor
** @argrule All kbs [AjPList] AJAX List of Ensembl Karyotype Band objects
** @argrule AllbySlice slice [EnsPSlice] Ensembl Slice
** @argrule AllbySlice constraint [const AjPStr] SQL constraint
** @argrule AllbySlice kbs [AjPList] AJAX List of Ensembl Karyotype Band
** objects
** @argrule AllbyChromosomeband name [const AjPStr] Chromosome name
** @argrule AllbyChromosomeband band [const AjPStr] Karyotype Band name
** @argrule AllbyChromosomeband kblist [AjPList] AJAX List of Ensembl
** Karyotype Band objects
** @argrule AllbyChromosomename name [const AjPStr] Chromosome name
** @argrule AllbyChromosomename kblist [AjPList] AJAX List of Ensembl
** Karyotype Band objects
** @argrule ByIdentifier identifier [ajuint] SQL database-internal identifier
** @argrule ByIdentifier Pkb [EnsPKaryotypeband*] Ensembl Karyotype Band
** address
**
** @valrule * [AjBool] ajTrue upon success, ajFalse otherwise
**
** @fcategory use
******************************************************************************/




/* @func ensKaryotypebandadaptorFetchAllbyChromosomeband **********************
**
** Fetch all Ensembl Karyotype Band objects via a chromosome and band name.
**
** This function uses fuzzy matching of the band name.
** For example the bands 'q23.1' and 'q23.4' could be matched by 'q23'.
**
** @cc Bio::EnsEMBL::DBSQL::Karyotypebandadaptor::fetch_all_by_chr_band
** @param [u] kba [EnsPKaryotypebandadaptor] Ensembl Karyotype Band Adaptor
** @param [r] name [const AjPStr] Chromosome name
** @param [r] band [const AjPStr] Karyotype Band name
** @param [u] kblist [AjPList] AJAX List of Ensembl Karyotype Band objects
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
** @@
******************************************************************************/

AjBool ensKaryotypebandadaptorFetchAllbyChromosomeband(
    EnsPKaryotypebandadaptor kba,
    const AjPStr name,
    const AjPStr band,
    AjPList kblist)
{
    char* txtband = NULL;

    AjBool result = AJFALSE;

    AjPStr constraint = NULL;

    EnsPDatabaseadaptor dba = NULL;

    EnsPSlice slice     = NULL;
    EnsPSliceadaptor sa = NULL;

    if(!kba)
        return ajFalse;

    if(!name)
        return ajFalse;

    if(!band)
        return ajFalse;

    if(!kblist)
        return ajFalse;

    dba = ensFeatureadaptorGetDatabaseadaptor(kba);

    sa = ensRegistryGetSliceadaptor(dba);

    ensSliceadaptorFetchBySeqregionName(sa,
                                        (const AjPStr) NULL,
                                        (const AjPStr) NULL,
                                        name,
                                        &slice);

    if(!slice)
    {
        ajDebug("ensKaryotypebandadaptorFetchAllbyChromosomeband could not "
                "fetch an Ensembl Slice for chromosome name '%S'.\n", name);

        return ajFalse;
    }

    ensDatabaseadaptorEscapeC(dba, &txtband, band);

    constraint = ajFmtStr("karyotype.band LIKE '%s%%'", txtband);

    ajCharDel(&txtband);

    result = ensFeatureadaptorFetchAllbySlice(kba,
                                              slice,
                                              constraint,
                                              (const AjPStr) NULL,
                                              kblist);

    ajStrDel(&constraint);

    ensSliceDel(&slice);

    return result;
}




/* @func ensKaryotypebandadaptorFetchAllbyChromosomename **********************
**
** Fetch all Ensembl Karyotype Band objects via a chromosome name.
**
** @cc Bio::EnsEMBL::DBSQL::Karyotypebandadaptor::fetch_all_by_chr_name
** @param [u] kba [EnsPKaryotypebandadaptor] Ensembl Karyotype Band Adaptor
** @param [r] name [const AjPStr] Chromosome name
** @param [u] kblist [AjPList] AJAX List of Ensembl Karyotype Band objects
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
** @@
******************************************************************************/

AjBool ensKaryotypebandadaptorFetchAllbyChromosomename(
    EnsPKaryotypebandadaptor kba,
    const AjPStr name,
    AjPList kblist)
{
    AjBool result = AJFALSE;

    EnsPDatabaseadaptor dba = NULL;

    EnsPSlice slice     = NULL;
    EnsPSliceadaptor sa = NULL;

    if(!kba)
        return ajFalse;

    if(!name)
        return ajFalse;

    if(!kblist)
        return ajFalse;

    dba = ensFeatureadaptorGetDatabaseadaptor(kba);

    sa = ensRegistryGetSliceadaptor(dba);

    ensSliceadaptorFetchBySeqregionName(sa,
                                        (const AjPStr) NULL,
                                        (const AjPStr) NULL,
                                        name,
                                        &slice);

    if(!slice)
    {
        ajDebug("ensKaryotypebandadaptorFetchAllbyChromosomename could not "
                "fetch an Ensembl Slice for chromosome name '%S'.\n", name);

        return ajFalse;
    }

    result = ensFeatureadaptorFetchAllbySlice(kba,
                                              slice,
                                              (const AjPStr) NULL,
                                              (const AjPStr) NULL,
                                              kblist);

    ensSliceDel(&slice);

    return result;
}
