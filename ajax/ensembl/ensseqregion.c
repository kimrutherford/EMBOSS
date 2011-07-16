/* @source Ensembl Sequence Region functions
**
** @author Copyright (C) 1999 Ensembl Developers
** @author Copyright (C) 2006 Michael K. Schuster
** @modified 2009 by Alan Bleasby for incorporation into EMBOSS core
** @modified $Date: 2011/07/06 21:50:28 $ by $Author: mks $
** @version $Revision: 1.35 $
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

#include "ensattribute.h"
#include "ensbaseadaptor.h"
#include "enscache.h"
#include "ensexternaldatabase.h"
#include "ensseqregion.h"
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

/* @conststatic seqregionadaptorCacheMaxBytes *********************************
**
** Maximum memory size in bytes the Ensembl Sequence Region Adaptor-internal
** Ensembl Cache can use.
**
** 1 << 26 = 64 MiB
**
******************************************************************************/

static const size_t seqregionadaptorCacheMaxBytes = 1 << 26;




/* @conststatic seqregionadaptorCacheMaxCount *********************************
**
** Maximum number of Ensembl Sequence Region objects the
** Ensembl Sequence Region Adaptor-internal Ensembl Cache can hold.
**
** 1 << 16 = 64 Ki
**
******************************************************************************/

static const ajuint seqregionadaptorCacheMaxCount = 1 << 16;




/* @conststatic seqregionadaptorCacheMaxSize **********************************
**
** Maximum memory size in bytes of an Ensembl Sequence Region to be allowed
** into the Ensembl Sequence Region Adaptor-internal Ensembl Cache.
**
******************************************************************************/

static const size_t seqregionadaptorCacheMaxSize = 0;




/* @conststatic seqregionsynonymadaptorTables *********************************
**
** Array of Ensembl Sequence Region Synonym Adaptor SQL table names
**
******************************************************************************/

static const char* const seqregionsynonymadaptorTables[] =
{
    "seq_region_synonym",
    (const char*) NULL
};




/* @conststatic seqregionsynonymadaptorColumns ********************************
**
** Array of Ensembl Sequence Region Synonym Adaptor SQL column names
**
******************************************************************************/

static const char* const seqregionsynonymadaptorColumns[] =
{
    "seq_region_synonym.seq_region_synonym_id",
    "seq_region_synonym.seq_region_id",
    "seq_region_synonym.synonym",
    "seq_region_synonym.external_db_id",
    (const char*) NULL
};




/* ==================================================================== */
/* ======================== private variables ========================= */
/* ==================================================================== */




/* ==================================================================== */
/* ======================== private functions ========================= */
/* ==================================================================== */

static void* seqregionadaptorCacheReference(void* value);

static void seqregionadaptorCacheDelete(void** value);

static size_t seqregionadaptorCacheSize(const void* value);

static void seqregionadaptorCacheInsert(void** x, void* cl);

static AjBool seqregionadaptorCacheLocusReferenceGenomicInit(
    EnsPSeqregionadaptor sra);

static AjBool seqregionadaptorCacheNonReferenceInit(
    EnsPSeqregionadaptor sra);

static AjBool seqregionadaptorFetchAllbyStatement(
    EnsPSeqregionadaptor sra,
    const AjPStr statement,
    AjPList srs);

static AjBool seqregionsynonymadaptorFetchAllbyStatement(
    EnsPDatabaseadaptor dba,
    const AjPStr statement,
    EnsPAssemblymapper am,
    EnsPSlice slice,
    AjPList srss);




/* ==================================================================== */
/* ===================== All functions by section ===================== */
/* ==================================================================== */




/* @filesection ensseqregion **************************************************
**
** @nam1rule ens Function belongs to the Ensembl library
**
******************************************************************************/




/* @datasection [EnsPSeqregion] Ensembl Sequence Region ***********************
**
** @nam2rule Seqregion Functions for manipulating
** Ensembl Sequence Region objects
**
******************************************************************************/




/* @section constructors ******************************************************
**
** All constructors return a new Ensembl Sequence Region by pointer.
** It is the responsibility of the user to first destroy any previous
** Sequence Region. The target pointer does not need to be initialised to
** NULL, but it is good programming practice to do so anyway.
**
** @fdata [EnsPSeqregion]
**
** @nam3rule New Constructor
** @nam4rule Cpy Constructor with existing object
** @nam4rule Ini Constructor with initial values
** @nam4rule Ref Constructor by incrementing the reference counter
**
** @argrule Cpy sr [EnsPSeqregion] Ensembl Sequence Region
** @argrule Ini sra [EnsPSeqregionadaptor] Ensembl Sequence Region Adaptor
** @argrule Ini identifier [ajuint] SQL database-internal identifier
** @argrule Ini cs [EnsPCoordsystem] Ensembl Coordinate System
** @argrule Ini name [AjPStr] Name
** @argrule Ini length [ajint] Length
** @argrule Ref sr [EnsPSeqregion] Ensembl Sequence Region
**
** @valrule * [EnsPSeqregion] Ensembl Sequence Region
**
** @fcategory new
******************************************************************************/




/* @func ensSeqregionNewCpy ***************************************************
**
** Object-based constructor function, which returns an independent object.
**
** @param [u] sr [EnsPSeqregion] Ensembl Sequence Region
**
** @return [EnsPSeqregion] Ensembl Sequence Region or NULL
** @@
******************************************************************************/

EnsPSeqregion ensSeqregionNewCpy(EnsPSeqregion sr)
{
    AjIList iter = NULL;

    EnsPAttribute attribute = NULL;

    EnsPSeqregion pthis = NULL;

    EnsPSeqregionsynonym srs = NULL;

    if(!sr)
        return NULL;

    AJNEW0(pthis);

    pthis->Use = 1;

    pthis->Identifier = sr->Identifier;

    pthis->Adaptor = sr->Adaptor;

    pthis->Coordsystem = ensCoordsystemNewRef(sr->Coordsystem);

    if(sr->Name)
        pthis->Name = ajStrNewRef(sr->Name);

    /* NOTE: Copy the AJAX List of Ensembl Attribute objects */

    if(sr->Attributes && ajListGetLength(sr->Attributes))
    {
        pthis->Attributes = ajListNew();

        iter = ajListIterNew(sr->Attributes);

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

    /* NOTE: Copy the AJAX List of Ensembl Sequence Region Synonym objects */

    if(sr->Seqregionsynonyms && ajListGetLength(sr->Seqregionsynonyms))
    {
        pthis->Seqregionsynonyms = ajListNew();

        iter = ajListIterNew(sr->Seqregionsynonyms);

        while(!ajListIterDone(iter))
        {
            srs = (EnsPSeqregionsynonym) ajListIterGet(iter);

            ajListPushAppend(pthis->Seqregionsynonyms,
                             (void*) ensSeqregionsynonymNewRef(srs));
        }

        ajListIterDel(&iter);
    }
    else
        pthis->Seqregionsynonyms = NULL;

    pthis->Length = sr->Length;

    return pthis;
}




/* @func ensSeqregionNewIni ***************************************************
**
** Constructor for an Ensembl Sequence Region with initial values.
**
** @cc Bio::EnsEMBL::Storable
** @param [uN] sra [EnsPSeqregionadaptor] Ensembl Sequence Region Adaptor
** @param [rE] identifier [ajuint] SQL database-internal identifier
** @cc Bio::EnsEMBL::???
** @param [u] cs [EnsPCoordsystem] Ensembl Coordinate System
** @param [u] name [AjPStr] Name
** @param [r] length [ajint] Length
**
** @return [EnsPSeqregion] Ensembl Sequence Region or NULL
** @@
******************************************************************************/

EnsPSeqregion ensSeqregionNewIni(EnsPSeqregionadaptor sra,
                                 ajuint identifier,
                                 EnsPCoordsystem cs,
                                 AjPStr name,
                                 ajint length)
{
    EnsPSeqregion sr = NULL;

    if(ajDebugTest("ensSeqregionNewIni"))
    {
        ajDebug("ensSeqregionNewIni\n"
                "  sra %p\n"
                "  identifier %u\n"
                "  cs %p\n"
                "  name '%S'\n"
                "  length %d\n",
                sra,
                identifier,
                name,
                cs,
                length);

        ensCoordsystemTrace(cs, 1);
    }

    if(!cs)
        return NULL;

    if(!(name && ajStrGetLen(name)))
        return NULL;

    if(length <= 0)
        return NULL;

    AJNEW0(sr);

    sr->Use = 1;

    sr->Identifier = identifier;

    sr->Adaptor = sra;

    sr->Coordsystem = ensCoordsystemNewRef(cs);

    if(name)
        sr->Name = ajStrNewRef(name);

    sr->Attributes = NULL;

    sr->Seqregionsynonyms = NULL;

    sr->Length = length;

    return sr;
}




/* @func ensSeqregionNewRef ***************************************************
**
** Ensembl Object referencing function, which returns a pointer to the
** Ensembl Object passed in and increases its reference count.
**
** @param [u] sr [EnsPSeqregion] Ensembl Sequence Region
**
** @return [EnsPSeqregion] Ensembl Sequence Region or NULL
** @@
******************************************************************************/

EnsPSeqregion ensSeqregionNewRef(EnsPSeqregion sr)
{
    if(!sr)
        return NULL;

    sr->Use++;

    return sr;
}




/* @section destructors *******************************************************
**
** Destruction destroys all internal data structures and frees the
** memory allocated for an Ensembl Sequence Region object.
**
** @fdata [EnsPSeqregion]
**
** @nam3rule Del Destroy (free) an Ensembl Sequence Region object
**
** @argrule * Psr [EnsPSeqregion*] Ensembl Sequence Region object address
**
** @valrule * [void]
**
** @fcategory delete
******************************************************************************/




/* @func ensSeqregionDel ******************************************************
**
** Default destructor for an Ensembl Sequence Region.
**
** @param [d] Psr [EnsPSeqregion*] Ensembl Sequence Region object address
**
** @return [void]
** @@
******************************************************************************/

void ensSeqregionDel(EnsPSeqregion* Psr)
{
    EnsPAttribute attribute = NULL;

    EnsPSeqregion pthis = NULL;

    EnsPSeqregionsynonym srs = NULL;

    if(!Psr)
        return;

    if(!*Psr)
        return;

    pthis = *Psr;

    pthis->Use--;

    if(pthis->Use)
    {
        *Psr = NULL;

        return;
    }

    ensCoordsystemDel(&pthis->Coordsystem);

    ajStrDel(&pthis->Name);

    /* Clear and delete the AJAX List of Ensembl Attribute objects. */

    while(ajListPop(pthis->Attributes, (void**) &attribute))
        ensAttributeDel(&attribute);

    ajListFree(&pthis->Attributes);

    /*
    ** Clear and delete the AJAX List of
    ** Ensembl Sequence Region Synonym objects.
    */

    while(ajListPop(pthis->Seqregionsynonyms, (void**) &srs))
        ensSeqregionsynonymDel(&srs);

    ajListFree(&pthis->Seqregionsynonyms);

    AJFREE(pthis);

    *Psr = NULL;

    return;
}




/* @section element retrieval *************************************************
**
** Functions for returning elements of an Ensembl Sequence Region object.
**
** @fdata [EnsPSeqregion]
**
** @nam3rule Get Return Ensembl Sequence Region attribute(s)
** @nam4rule Adaptor Return the Ensembl Sequence Region Adaptor
** @nam4rule Coordsystem Return the Ensembl Coordinate System
** @nam4rule Identifier Return the SQL database-internal identifier
** @nam4rule Length Return the length
** @nam4rule Name Return the name
**
** @argrule * sr [const EnsPSeqregion] Ensembl Sequence Region
**
** @valrule Adaptor [EnsPSeqregionadaptor] Ensembl Sequence Region Adaptor
** or NULL
** @valrule Coordsystem [EnsPCoordsystem] Ensembl Coordinate System or NULL
** @valrule Identifier [ajuint] SQL database-internal identifier or 0
** @valrule Length [ajint] Length or 0
** @valrule Name [AjPStr] Name or NULL
**
** @fcategory use
******************************************************************************/




/* @func ensSeqregionGetAdaptor ***********************************************
**
** Get the Ensembl Sequence Region Adaptor element of an
** Ensembl Sequence Region.
**
** @param [r] sr [const EnsPSeqregion] Ensembl Sequence Region
**
** @return [EnsPSeqregionadaptor] Ensembl Sequence Region Adaptor or NULL
** @@
******************************************************************************/

EnsPSeqregionadaptor ensSeqregionGetAdaptor(const EnsPSeqregion sr)
{
    if(!sr)
        return NULL;

    return sr->Adaptor;
}




/* @func ensSeqregionGetCoordsystem *******************************************
**
** Get the Ensembl Coordinate System element of an Ensembl Sequence Region.
**
** @param [r] sr [const EnsPSeqregion] Ensembl Sequence Region
**
** @return [EnsPCoordsystem] Ensembl Coordinate System or NULL
** @@
******************************************************************************/

EnsPCoordsystem ensSeqregionGetCoordsystem(const EnsPSeqregion sr)
{
    if(!sr)
        return NULL;

    return sr->Coordsystem;
}




/* @func ensSeqregionGetIdentifier ********************************************
**
** Get the SQL database-internal identifier element of an
** Ensembl Sequence Region.
**
** @param [r] sr [const EnsPSeqregion] Ensembl Sequence Region
**
** @return [ajuint] SQL database-internal identifier or 0
** @@
******************************************************************************/

ajuint ensSeqregionGetIdentifier(const EnsPSeqregion sr)
{
    if(!sr)
        return 0;

    return sr->Identifier;
}




/* @func ensSeqregionGetLength ************************************************
**
** Get the length element of an Ensembl Sequence Region.
**
** @param [r] sr [const EnsPSeqregion] Ensembl Sequence Region
**
** @return [ajint] Length or 0
** @@
******************************************************************************/

ajint ensSeqregionGetLength(const EnsPSeqregion sr)
{
    if(!sr)
        return 0;

    return sr->Length;
}




/* @func ensSeqregionGetName **************************************************
**
** Get the name element of an Ensembl Sequence Region.
**
** @param [r] sr [const EnsPSeqregion] Ensembl Sequence Region
**
** @return [AjPStr] Name or NULL
** @@
******************************************************************************/

AjPStr ensSeqregionGetName(const EnsPSeqregion sr)
{
    if(!sr)
        return NULL;

    return sr->Name;
}




/* @section load on demand ****************************************************
**
** Functions for returning elements of an Ensembl Sequence Region object,
** which may need loading from an Ensembl SQL database on demand.
**
** @fdata [EnsPSeqregion]
**
** @nam3rule Load Return Ensembl Sequence Region attribute(s) loaded on demand
** @nam4rule Attributes Return all Ensembl Attribute objects
** @nam4rule Seqregionsynonyms Return all Ensembl Sequence Region Synonym
** objects
**
** @argrule * sr [EnsPSeqregion] Ensembl Sequence Region
**
** @valrule Attributes [const AjPList] AJAX List of Ensembl Attribute objects
** or NULL
** @valrule Seqregionsynonyms [const AjPList] AJAX List of
** Ensembl Sequence Region Synonym objects or NULL
**
** @fcategory use
******************************************************************************/




/* @func ensSeqregionLoadAttributes *******************************************
**
** Get all Ensembl Attribute objects of an Ensembl Sequence Region.
**
** This is not a simple accessor function, it will fetch Ensembl Attribute
** objects from the Ensembl database in case the AJAX List is not defined.
**
** @param [u] sr [EnsPSeqregion] Ensembl Sequence Region
**
** @return [const AjPList] AJAX List of Ensembl Attribute objects or NULL
** @@
******************************************************************************/

const AjPList ensSeqregionLoadAttributes(EnsPSeqregion sr)
{
    EnsPAttributeadaptor ata = NULL;

    EnsPDatabaseadaptor dba = NULL;

    if(!sr)
        return NULL;

    if(sr->Attributes)
        return sr->Attributes;

    if(!sr->Adaptor)
    {
        ajDebug("ensSeqregionLoadAttributes cannot fetch Ensembl Attribute "
                "objects for an Ensembl Sequence Region without an "
                "Ensembl Sequence Region Adaptor.\n");

        return NULL;
    }

    dba = ensSeqregionadaptorGetDatabaseadaptor(sr->Adaptor);

    if(!dba)
    {
        ajDebug("ensSeqregionLoadAttributes cannot fetch Ensembl Attribute "
                "objects for an Ensembl Sequence Region without an "
                "Ensembl Database Adaptor set in the "
                "Ensembl Sequence Region Adaptor.\n");

        return NULL;
    }

    ata = ensRegistryGetAttributeadaptor(dba);

    sr->Attributes = ajListNew();

    ensAttributeadaptorFetchAllbySeqregion(ata,
                                           sr,
                                           (const AjPStr) NULL,
                                           sr->Attributes);

    return sr->Attributes;
}




/* @func ensSeqregionLoadSeqregionsynonyms ************************************
**
** Get all Ensembl Sequence Region Synonym objects of an
** Ensembl Sequence Region.
**
** This is not a simple accessor function, it will fetch
** Ensembl Sequence Region Synonym objects from the Ensembl database in case
** the AJAX List is not defined.
**
** @param [u] sr [EnsPSeqregion] Ensembl Sequence Region
**
** @return [const AjPList] AJAX List of Ensembl Sequence Region Synonym objects
** or NULL
** @@
******************************************************************************/

const AjPList ensSeqregionLoadSeqregionsynonyms(EnsPSeqregion sr)
{
    EnsPSeqregionsynonymadaptor srsa = NULL;

    EnsPDatabaseadaptor dba = NULL;

    if(!sr)
        return NULL;

    if(sr->Seqregionsynonyms)
        return sr->Seqregionsynonyms;

    if(!sr->Adaptor)
    {
        ajDebug("ensSeqregionLoadSeqregionsynonyms cannot fetch "
                "Ensembl Sequence Region Synonym objects "
                "for an Ensembl Sequence Region without an "
                "Ensembl Sequence Region Adaptor.\n");

        return NULL;
    }

    dba = ensSeqregionadaptorGetDatabaseadaptor(sr->Adaptor);

    if(!dba)
    {
        ajDebug("ensSeqregionLoadSeqregionsynonyms cannot fetch "
                "Ensembl Sequence Region Synonym objects "
                "for an Ensembl Sequence Region without an "
                "Ensembl Database Adaptor set in the "
                "Ensembl Sequence Region Adaptor.\n");

        return NULL;
    }

    srsa = ensRegistryGetSeqregionsynonymadaptor(dba);

    sr->Seqregionsynonyms = ajListNew();

    ensSeqregionsynonymadaptorFetchAllbySeqregion(srsa,
                                                  sr,
                                                  sr->Seqregionsynonyms);

    return sr->Seqregionsynonyms;
}




/* @section modifiers *********************************************************
**
** Functions for assigning elements of an Ensembl Sequence Region object.
**
** @fdata [EnsPSeqregion]
**
** @nam3rule Set Set one element of an Ensembl Sequence Region
** @nam4rule Adaptor Set the Ensembl Sequence Region Adaptor
** @nam4rule Identifier Set the SQL database-internal identifier
** @nam4rule Coordsystem Set the Ensembl Coordinate System
** @nam4rule Name Set the name
** @nam4rule Length Set the length
**
** @argrule * sr [EnsPSeqregion] Ensembl Sequence Region object
** @argrule Adaptor sra [EnsPSeqregionadaptor] Ensembl Sequence Region Adaptor
** @argrule Coordsystem cs [EnsPCoordsystem] Ensembl Coordinate System
** @argrule Identifier identifier [ajuint] SQL database-internal identifier
** @argrule Length length [ajint] Length
** @argrule Name name [AjPStr] Name
**
** @valrule * [AjBool] ajTrue upon success, ajFalse otherwise
**
** @fcategory modify
******************************************************************************/




/* @func ensSeqregionSetAdaptor ***********************************************
**
** Set the Ensembl Sequence Region Adaptor element of an
** Ensembl Sequence Region.
**
** @param [u] sr [EnsPSeqregion] Ensembl Sequence Region
** @param [u] sra [EnsPSeqregionadaptor] Ensembl Sequence Region Adaptor
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
** @@
******************************************************************************/

AjBool ensSeqregionSetAdaptor(EnsPSeqregion sr, EnsPSeqregionadaptor sra)
{
    if(!sr)
        return ajFalse;

    sr->Adaptor = sra;

    return ajTrue;
}




/* @func ensSeqregionSetCoordsystem *******************************************
**
** Set the Ensembl Coordinate System element of an Ensembl Sequence Region.
**
** @param [u] sr [EnsPSeqregion] Ensembl Sequence Region
** @param [uN] cs [EnsPCoordsystem] Ensembl Coordinate System
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
** @@
******************************************************************************/

AjBool ensSeqregionSetCoordsystem(EnsPSeqregion sr, EnsPCoordsystem cs)
{
    if(!sr)
        return ajFalse;

    ensCoordsystemDel(&sr->Coordsystem);

    sr->Coordsystem = ensCoordsystemNewRef(cs);

    return ajTrue;
}




/* @func ensSeqregionSetIdentifier ********************************************
**
** Set the SQL database-internal identifier element of an
** Ensembl Sequence Region.
**
** @param [u] sr [EnsPSeqregion] Ensembl Sequence Region
** @param [r] identifier [ajuint] SQL database-internal identifier
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
** @@
******************************************************************************/

AjBool ensSeqregionSetIdentifier(EnsPSeqregion sr, ajuint identifier)
{
    if(!sr)
        return ajFalse;

    sr->Identifier = identifier;

    return ajTrue;
}




/* @func ensSeqregionSetLength ************************************************
**
** Set the length element of an Ensembl Sequence Region.
**
** @param [u] sr [EnsPSeqregion] Ensembl Sequence Region
** @param [r] length [ajint] Length
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
** @@
******************************************************************************/

AjBool ensSeqregionSetLength(EnsPSeqregion sr, ajint length)
{
    if(!sr)
        return ajFalse;

    sr->Length = length;

    return ajTrue;
}




/* @func ensSeqregionSetName **************************************************
**
** Set the name element of an Ensembl Sequence Region.
**
** @param [u] sr [EnsPSeqregion] Ensembl Sequence Region
** @param [uN] name [AjPStr] Name
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
** @@
******************************************************************************/

AjBool ensSeqregionSetName(EnsPSeqregion sr, AjPStr name)
{
    if(!sr)
        return ajFalse;

    ajStrDel(&sr->Name);

    if(name)
        sr->Name = ajStrNewRef(name);

    return ajTrue;
}




/* @section element addition **************************************************
**
** Functions for adding elements to an Ensembl Sequence Region object.
**
** @fdata [EnsPSeqregion]
**
** @nam3rule Add Add one object to an Ensembl Sequence Region
** @nam4rule Attribute Add an Ensembl Attribute
** @nam4rule Seqregionsynonym Add an Ensembl Sequence Region Synonym
**
** @argrule * sr [EnsPSeqregion] Ensembl Sequence Region object
** @argrule Attribute attribute [EnsPAttribute] Ensembl Attribute
** @argrule Seqregionsynonym srs [EnsPSeqregionsynonym]
** Ensembl Sequence Region Synonym
**
** @valrule * [AjBool] ajTrue upon success, ajFalse otherwise
**
** @fcategory modify
******************************************************************************/




/* @func ensSeqregionAddAttribute *********************************************
**
** Add an Ensembl Attribute to an Ensembl Sequence Region.
**
** @param [u] sr [EnsPSeqregion] Ensembl Sequence Region
** @param [u] attribute [EnsPAttribute] Ensembl Attribute
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
** @@
******************************************************************************/

AjBool ensSeqregionAddAttribute(EnsPSeqregion sr, EnsPAttribute attribute)
{
    if(!sr)
        return ajFalse;

    if(!attribute)
        return ajFalse;

    if(!sr->Attributes)
        sr->Attributes = ajListNew();

    ajListPushAppend(sr->Attributes,
                     (void*) ensAttributeNewRef(attribute));

    return ajTrue;
}




/* @func ensSeqregionAddSeqregionsynonym **************************************
**
** Add an Ensembl Sequence Region Synonym to an Ensembl Sequence Region.
**
** @param [u] sr [EnsPSeqregion] Ensembl Sequence Region
** @param [u] srs [EnsPSeqregionsynonym] Ensembl Sequence Region Synonym
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
** @@
******************************************************************************/

AjBool ensSeqregionAddSeqregionsynonym(EnsPSeqregion sr,
                                       EnsPSeqregionsynonym srs)
{
    if(!sr)
        return ajFalse;

    if(!srs)
        return ajFalse;

    if(!sr->Seqregionsynonyms)
        sr->Seqregionsynonyms = ajListNew();

    ajListPushAppend(sr->Seqregionsynonyms,
                     (void*) ensSeqregionsynonymNewRef(srs));

    return ajTrue;
}




/* @section debugging *********************************************************
**
** Functions for reporting of an Ensembl Sequence Region object.
**
** @fdata [EnsPSeqregion]
**
** @nam3rule Trace Report Ensembl Sequence Region elements to debug file
**
** @argrule Trace sr [const EnsPSeqregion] Ensembl Sequence Region
** @argrule Trace level [ajuint] Indentation level
**
** @valrule * [AjBool] ajTrue upon success, ajFalse otherwise
**
** @fcategory misc
******************************************************************************/




/* @func ensSeqregionTrace ****************************************************
**
** Trace an Ensembl Sequence Region.
**
** @param [r] sr [const EnsPSeqregion] Ensembl Sequence Region
** @param [r] level [ajuint] Indentation level
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
** @@
******************************************************************************/

AjBool ensSeqregionTrace(const EnsPSeqregion sr, ajuint level)
{
    AjPStr indent = NULL;

    if(!sr)
        return ajFalse;

    indent = ajStrNew();

    ajStrAppendCountK(&indent, ' ', level * 2);

    ajDebug("%SensSeqregionTrace %p\n"
            "%S  Use %u\n"
            "%S  Identifier %u\n"
            "%S  Adaptor %p\n"
            "%S  Coordsystem %p\n"
            "%S  Name '%S'\n"
            "%S  Attributes %p\n"
            "%S  Seqregionsynonyms %p\n"
            "%S  Length %u\n",
            indent, sr,
            indent, sr->Use,
            indent, sr->Identifier,
            indent, sr->Adaptor,
            indent, sr->Coordsystem,
            indent, sr->Name,
            indent, sr->Attributes,
            indent, sr->Seqregionsynonyms,
            indent, sr->Length);

    ensCoordsystemTrace(sr->Coordsystem, level + 1);

    ajStrDel(&indent);

    return ajTrue;
}




/* @section calculate *********************************************************
**
** Functions for calculating values of an Ensembl Sequence Region object.
**
** @fdata [EnsPSeqregion]
**
** @nam3rule Calculate Calculate Ensembl Sequence Region values
** @nam4rule Memsize Calculate the memory size in bytes
**
** @argrule Memsize sr [const EnsPSeqregion] Ensembl Sequence Region
**
** @valrule Memsize [size_t] Memory size in bytes or 0
**
** @fcategory misc
******************************************************************************/




/* @func ensSeqregionCalculateMemsize *****************************************
**
** Calculate the memory size in bytes of an Ensembl Sequence Region.
**
** @param [r] sr [const EnsPSeqregion] Ensembl Sequence Region
**
** @return [size_t] Memory size in bytes or 0
** @@
******************************************************************************/

size_t ensSeqregionCalculateMemsize(const EnsPSeqregion sr)
{
    size_t size = 0;

    AjIList iter = NULL;

    EnsPAttribute attribute = NULL;

    EnsPSeqregionsynonym srs = NULL;

    if(!sr)
        return 0;

    size += sizeof (EnsOSeqregion);

    size += ensCoordsystemCalculateMemsize(sr->Coordsystem);

    if(sr->Name)
    {
        size += sizeof (AjOStr);

        size += ajStrGetRes(sr->Name);
    }

    if(sr->Attributes)
    {
        iter = ajListIterNewread(sr->Attributes);

        while(!ajListIterDone(iter))
        {
            attribute = (EnsPAttribute) ajListIterGet(iter);

            size += ensAttributeCalculateMemsize(attribute);
        }

        ajListIterDel(&iter);
    }

    if(sr->Seqregionsynonyms)
    {
        iter = ajListIterNewread(sr->Seqregionsynonyms);

        while(!ajListIterDone(iter))
        {
            srs = (EnsPSeqregionsynonym) ajListIterGet(iter);

            size += ensSeqregionsynonymCalculateMemsize(srs);
        }

        ajListIterDel(&iter);
    }

    return size;
}




/* @section comparison ********************************************************
**
** Functions for matching Ensembl Sequence Region objects.
**
** @fdata [EnsPSeqregion]
**
** @nam3rule Match Functions for matching Ensembl Sequence Region objects
**
** @argrule * sr1 [const EnsPSeqregion] Ensembl Sequence Region
** @argrule * sr2 [const EnsPSeqregion] Ensembl Sequence Region
**
** @valrule * [AjBool] ajTrue if the Slice objects are equal
**
** @fcategory misc
******************************************************************************/




/* @func ensSeqregionMatch ****************************************************
**
** Test for matching two Ensembl Sequence Regions.
**
** @param [r] sr1 [const EnsPSeqregion] First Ensembl Sequence Region
** @param [r] sr2 [const EnsPSeqregion] Second Ensembl Sequence Region
**
** @return [AjBool] ajTrue if the Ensembl Sequence Regions are equal
** @@
** The comparison is based on an initial pointer equality test and if that
** fails, a case-insensitive string comparison of the name element, as well as
** Coordinate Systems and length element comparisons are performed.
******************************************************************************/

AjBool ensSeqregionMatch(const EnsPSeqregion sr1, const EnsPSeqregion sr2)
{
    if(!sr1)
        return ajFalse;

    if(!sr2)
        return ajFalse;

    if(sr1 == sr2)
        return ajTrue;

    /* Compare identifiers only if they have been set. */

    if(sr1->Identifier && sr2->Identifier &&
       (sr1->Identifier != sr2->Identifier))
        return ajFalse;

    if(!ensCoordsystemMatch(sr1->Coordsystem, sr2->Coordsystem))
        return ajFalse;

    if(!ajStrMatchS(sr1->Name, sr2->Name))
        return ajFalse;

    if(sr1->Length != sr2->Length)
        return ajFalse;

    return ajTrue;
}




/* @section fetch *************************************************************
**
** Functions for fetching objects of an Ensembl Sequence Region object.
**
** @fdata [EnsPSeqregion]
**
** @nam3rule Fetch Fetch Ensembl Sequence Region objects
** @nam4rule All             Fetch all objects
** @nam5rule Attributes      Fetch all Ensembl Attribute objects
**
** @argrule AllAttributes sr [EnsPSeqregion] Ensembl Sequence Region
** @argrule AllAttributes code [const AjPStr] Ensembl Attribute code
** @argrule AllAttributes attributes [AjPList] AJAX List of
** Ensembl Attribute objects
**
** @valrule * [AjBool] ajTrue upon success, ajFalse otherwise
**
** @fcategory misc
******************************************************************************/




/* @func ensSeqregionFetchAllAttributes ***************************************
**
** Fetch all Ensembl Attribute objects of an Ensembl Sequence Region.
**
** The caller is responsible for deleting the Ensembl Attribute objects before
** deleting the AJAX List.
**
** @param [u] sr [EnsPSeqregion] Ensembl Sequence Region
** @param [rN] code [const AjPStr] Ensembl Attribute code
** @param [u] attributes [AjPList] AJAX List of Ensembl Attribute objects
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
** @@
******************************************************************************/

AjBool ensSeqregionFetchAllAttributes(EnsPSeqregion sr,
                                      const AjPStr code,
                                      AjPList attributes)
{
    AjBool match = AJFALSE;

    const AjPList list = NULL;
    AjIList iter = NULL;

    EnsPAttribute attribute = NULL;

    if(!sr)
        return ajFalse;

    if(!attributes)
        return ajFalse;

    list = ensSeqregionLoadAttributes(sr);

    iter = ajListIterNewread(list);

    while(!ajListIterDone(iter))
    {
        attribute = (EnsPAttribute) ajListIterGet(iter);

        if(code)
        {
            if(ajStrMatchCaseS(code, ensAttributeGetCode(attribute)))
                match = ajTrue;
            else
                match = ajFalse;
        }
        else
            match = ajTrue;

        if(match)
            ajListPushAppend(attributes,
                             (void*) ensAttributeNewRef(attribute));
    }

    ajListIterDel(&iter);

    return ajTrue;
}




/* @section query *************************************************************
**
** Functions for querying the properties of an Ensembl Sequence Region.
**
** @fdata [EnsPSeqregion]
**
** @nam3rule Is Check whether an Ensembl Sequence Region represents a
** certain property
** @nam4rule Locusreferencegenomic Check for a Locus Reference Genomic
** Ensembl Sequence Region
** @nam4rule Nonreference Check for a non-reference Ensembl Sequence Region
** @nam4rule Toplevel Check for a top-level Ensembl Sequence Region
**
** @argrule Locusreferencegenomic sr [EnsPSeqregion] Ensembl Sequence Region
** @argrule Locusreferencegenomic Presult [AjBool*] Boolean result
** @argrule Nonreference sr [EnsPSeqregion] Ensembl Sequence Region
** @argrule Nonreference Presult [AjBool*] Boolean result
** @argrule Toplevel sr [EnsPSeqregion] Ensembl Sequence Region
** @argrule Toplevel Presult [AjBool*] Boolean result
**
** @valrule * [AjBool] ajTrue upon success, ajFalse otherwise
**
** @fcategory use
******************************************************************************/




/* @func ensSeqregionIsLocusreferencegenomic **********************************
**
** Check if an Ensembl Sequence Region has an Ensembl Attribute of code
** "LRG" set.
** This function calls ensSeqregionadaptorIsLocusreferencegenomic and
** uses the Ensembl Sequence Region Adaptor-internal cache of LRG
** Ensembl Sequence Region objects.
**
** @param [u] sr [EnsPSeqregion] Ensembl Sequence Region
** @param [u] Presult [AjBool*] ajTrue, if an Ensembl Attribute of code
**                              "non_ref" has been set
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
** @@
******************************************************************************/

AjBool ensSeqregionIsLocusreferencegenomic(EnsPSeqregion sr, AjBool* Presult)
{
    if(!sr)
        return ajFalse;

    if(!Presult)
        return ajFalse;

    return ensSeqregionadaptorIsLocusreferencegenomic(sr->Adaptor,
                                                      sr,
                                                      Presult);
}




/* @func ensSeqregionIsNonreference *******************************************
**
** Check if an Ensembl Sequence Region has an Ensembl Attribute of code
** "non_ref" set.
** This function calls ensSeqregionadaptorIsNonreference and
** uses the Ensembl Sequence Region Adaptor-internal cache of non-reference
** Ensembl Sequence Region objects.
**
** @param [u] sr [EnsPSeqregion] Ensembl Sequence Region
** @param [u] Presult [AjBool*] ajTrue, if an Ensembl Attribute of code
**                              "non_ref" has been set
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
** @@
******************************************************************************/

AjBool ensSeqregionIsNonreference(EnsPSeqregion sr, AjBool* Presult)
{
    if(!sr)
        return ajFalse;

    if(!Presult)
        return ajFalse;

    return ensSeqregionadaptorIsNonreference(sr->Adaptor,
                                             sr,
                                             Presult);
}




/* @func ensSeqregionIsToplevel ***********************************************
**
** Test if an Ensembl Sequence Region has an Ensembl Attribute of code
** "toplevel" set. If Ensembl Attribute objects associated with this
** Ensembl Sequence Region are not already cached, they will be fetched from
** the database.
**
** @param [u] sr [EnsPSeqregion] Ensembl Sequence Region
** @param [u] Presult [AjBool*] ajTrue, if an Ensembl Attribute of code
**                              "toplevel" has been set
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
** @@
******************************************************************************/

AjBool ensSeqregionIsToplevel(EnsPSeqregion sr, AjBool* Presult)
{
    AjIList iter       = NULL;
    const AjPList list = NULL;

    EnsPAttribute attribute = NULL;

    if(!sr)
        return ajFalse;

    if(!Presult)
        return ajFalse;

    list = ensSeqregionLoadAttributes(sr);

    iter = ajListIterNewread(list);

    while(!ajListIterDone(iter))
    {
        attribute = (EnsPAttribute) ajListIterGet(iter);

        if(ajStrMatchC(ensAttributeGetCode(attribute), "toplevel"))
        {
            *Presult = ajTrue;

            break;
        }
    }

    ajListIterDel(&iter);

    return ajTrue;
}




/* @datasection [EnsPSeqregionadaptor] Ensembl Sequence Region Adaptor ********
**
** @nam2rule Seqregionadaptor Functions for manipulating
** Ensembl Sequence Region Adaptor objects
**
******************************************************************************/




/* @funcstatic seqregionadaptorCacheReference *********************************
**
** Wrapper function to reference an Ensembl Sequence Region
** from an Ensembl Cache.
**
** @param [u] value [void*] Ensembl Sequence Region
**
** @return [void*] Ensembl Sequence Region or NULL
** @@
******************************************************************************/

static void* seqregionadaptorCacheReference(void* value)
{
    if(!value)
        return NULL;

    return (void*) ensSeqregionNewRef((EnsPSeqregion) value);
}




/* @funcstatic seqregionadaptorCacheDelete ************************************
**
** Wrapper function to delete an Ensembl Sequence Region from an Ensembl Cache.
**
** @param [u] value [void**] Ensembl Sequence Region address
**
** @return [void]
** @@
** When deleting from the Cache, this function also removes and deletes the
** Ensembl Sequence Region from the name cache, which is based on a
** conventional AJAX Table.
******************************************************************************/

static void seqregionadaptorCacheDelete(void** value)
{
    AjPStr newkey = NULL;
    AjPStr oldkey = NULL;

    EnsPSeqregion  oldsr = NULL;
    EnsPSeqregion* Psr   = NULL;

    if(!value)
        return;

    /*
    ** Synchronise the deletion of this Sequence Region from the
    ** identifier cache, which is based on an Ensembl (LRU) Cache,
    ** with the name cache, based on a conventional AJAX Table,
    ** both in the Sequence Adaptor.
    */

    Psr = (EnsPSeqregion*) value;

    if(!*Psr)
        return;

    if((*Psr)->Adaptor && (*Psr)->Adaptor->CacheByName)
    {
        /* Remove from the name cache. */

        newkey =
            ajFmtStr("%u:%S",
                     ensCoordsystemGetIdentifier((*Psr)->Coordsystem),
                     (*Psr)->Name);

        oldsr = (EnsPSeqregion)
            ajTableRemoveKey((*Psr)->Adaptor->CacheByName,
                             (const void*) newkey,
                             (void**) &oldkey);

        /* Delete the old and new key data. */

        ajStrDel(&oldkey);

        ajStrDel(&newkey);

        /* Delete the value data. */

        ensSeqregionDel(&oldsr);
    }

    ensSeqregionDel(Psr);

    return;
}




/* @funcstatic seqregionadaptorCacheSize **************************************
**
** Wrapper function to determine the memory size of an Ensembl Sequence Region
** from an Ensembl Cache.
**
** @param [r] value [const void*] Ensembl Sequence Region
**
** @return [size_t] Memory size in bytes or 0
** @@
******************************************************************************/

static size_t seqregionadaptorCacheSize(const void* value)
{
    if(!value)
        return 0;

    return ensSeqregionCalculateMemsize((const EnsPSeqregion) value);
}




/* @funcstatic seqregionadaptorCacheLocusReferenceGenomicInit *****************
**
** Initialises an Ensembl Sequence Region Adaptor-internal AJAX List of
** Locus Region Genomic (LRG) Ensembl Sequence Region objects, which are
** associated with Ensembl Attribute objects of code "LRG".
**
** @param [u] sra [EnsPSeqregionadaptor] Ensembl Sequence Region Adaptor
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
** @@
******************************************************************************/

static AjBool seqregionadaptorCacheLocusReferenceGenomicInit(
    EnsPSeqregionadaptor sra)
{
    AjPStr code  = NULL;
    AjPStr value = NULL;

    if(!sra)
        return ajFalse;

    if(sra->CacheLocusReferenceGenomic)
        return ajTrue;

    sra->CacheLocusReferenceGenomic = ajListNew();

    code  = ajStrNewC("LRG");
    value = ajStrNewC("1");

    ensSeqregionadaptorFetchAllbyAttributecodevalue(
        sra,
        code,
        value,
        sra->CacheLocusReferenceGenomic);

    ajStrDel(&code);
    ajStrDel(&value);


    return ajTrue;
}




/* @funcstatic seqregionadaptorCacheNonReferenceInit **************************
**
** Initialises an Ensembl Sequence Region Adaptor-internal AJAX List of
** non-reference (i.e. haplotype) Ensembl Sequence Region objects, which are
** associated with Ensembl Attribute objects of code "non_ref".
**
** @param [u] sra [EnsPSeqregionadaptor] Ensembl Sequence Region Adaptor
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
** @@
******************************************************************************/

static AjBool seqregionadaptorCacheNonReferenceInit(
    EnsPSeqregionadaptor sra)
{
    AjPStr code  = NULL;
    AjPStr value = NULL;

    if(!sra)
        return ajFalse;

    if(sra->CacheNonReference)
        return ajTrue;

    sra->CacheNonReference = ajListNew();

    code  = ajStrNewC("non_ref");
    value = ajStrNewC("1");

    ensSeqregionadaptorFetchAllbyAttributecodevalue(
        sra,
        code,
        value,
        sra->CacheNonReference);

    ajStrDel(&code);
    ajStrDel(&value);

    return ajTrue;
}




/* @section constructors ******************************************************
**
** All constructors return a new Ensembl Sequence Region Adaptor by pointer.
** It is the responsibility of the user to first destroy any previous
** Sequence Region Adaptor. The target pointer does not need to be initialised
** to NULL, but it is good programming practice to do so anyway.
**
** @fdata [EnsPSeqregionadaptor]
**
** @nam3rule New Constructor
**
** @argrule New dba [EnsPDatabaseadaptor] Ensembl Database Adaptor
**
** @valrule * [EnsPSeqregionadaptor] Ensembl Sequence Region Adaptor
**
** @fcategory new
******************************************************************************/




/* @func ensSeqregionadaptorNew ***********************************************
**
** Default constructor for an Ensembl Sequence Region Adaptor.
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
** @see ensRegistryGetSeqregionadaptor
**
** @param [u] dba [EnsPDatabaseadaptor] Ensembl Database Adaptor
**
** @return [EnsPSeqregionadaptor] Ensembl Sequence Region Adaptor or NULL
** @@
******************************************************************************/

EnsPSeqregionadaptor ensSeqregionadaptorNew(
    EnsPDatabaseadaptor dba)
{
    EnsPSeqregionadaptor sra = NULL;

    if(ajDebugTest("ensSeqregionadaptorNew"))
        ajDebug("ensSeqregionadaptorNew\n"
                "  dba %p\n",
                dba);

    if(!dba)
        return NULL;

    AJNEW0(sra);

    sra->Adaptor = dba;

    sra->CacheByIdentifier = ensCacheNew(
        ensECacheTypeNumeric,
        seqregionadaptorCacheMaxBytes,
        seqregionadaptorCacheMaxCount,
        seqregionadaptorCacheMaxSize,
        seqregionadaptorCacheReference,
        seqregionadaptorCacheDelete,
        seqregionadaptorCacheSize,
        (void* (*)(const void* key)) NULL,
        (AjBool (*)(const void* value)) NULL,
        ajFalse,
        "Sequence Region");

    sra->CacheByName = ensTablestrNewLen(0);

    return sra;
}




/* @section destructors *******************************************************
**
** Destruction destroys all internal data structures and frees the
** memory allocated for an Ensembl Sequence Region Adaptor object.
**
** @fdata [EnsPSeqregionadaptor]
**
** @nam3rule Del Destroy (free) an Ensembl Sequence Region Adaptor object
**
** @argrule * Psra [EnsPSeqregionadaptor*] Ensembl Sequence Region Adaptor
**                                         object address
**
** @valrule * [void]
**
** @fcategory delete
******************************************************************************/




/* @func ensSeqregionadaptorDel ***********************************************
**
** Default destructor for an Ensembl Sequence Region Adaptor.
**
** Ensembl Object Adaptors are singleton objects that are registered in the
** Ensembl Registry and weakly referenced by Ensembl Objects that have been
** instantiated by it. Therefore, Ensembl Object Adaptors should never be
** destroyed directly. Upon exit, the Ensembl Registry will call this function
** if required.
**
** @param [d] Psra [EnsPSeqregionadaptor*] Ensembl Sequence Region Adaptor
**                                         object address
**
** @return [void]
** @@
******************************************************************************/

void ensSeqregionadaptorDel(EnsPSeqregionadaptor* Psra)
{
    EnsPSeqregion        sr    = NULL;
    EnsPSeqregionadaptor pthis = NULL;

    if(!Psra)
        return;

    if(!*Psra)
        return;

    if(ajDebugTest("ensSeqregionadaptorDel"))
        ajDebug("ensSeqregionadaptorDel\n"
                "  *Psra %p\n",
                *Psra);

    pthis = *Psra;

    /*
    ** Clear the identifier cache, which is based on an Ensembl LRU Cache.
    ** Clearing the Ensembl LRU Cache automatically clears the name cache via
    ** seqregionCacheDelete so that the AJAX Table can be simply freed.
    */

    ensCacheDel(&pthis->CacheByIdentifier);

    ajTableFree(&pthis->CacheByName);

    /*
    ** Clear the AJAX List of Locus Reference Genomic (LRG)
    ** Ensembl Sequence Region objects.
    */

    if(pthis->CacheLocusReferenceGenomic)
    {
        while(ajListPop(pthis->CacheLocusReferenceGenomic, (void**) &sr))
            ensSeqregionDel(&sr);

        ajListFree(&pthis->CacheLocusReferenceGenomic);
    }

    /* Clear the AJAX List of non-reference Ensembl Sequence Region objects. */

    if(pthis->CacheNonReference)
    {
        while(ajListPop(pthis->CacheNonReference, (void**) &sr))
            ensSeqregionDel(&sr);

        ajListFree(&pthis->CacheNonReference);
    }

    AJFREE(pthis);

    *Psra = NULL;

    return;
}




/* @section element retrieval *************************************************
**
** Functions for returning elements of an Ensembl Sequence Region Adaptor
** object.
**
** @fdata [EnsPSeqregionadaptor]
**
** @nam3rule Get Return Ensembl Sequence Region Adaptor attribute(s)
** @nam4rule GetDatabaseadaptor Return the Ensembl Database Adaptor
**
** @argrule * sra [const EnsPSeqregionadaptor] Ensembl Sequence Region Adaptor
**
** @valrule Databaseadaptor [EnsPDatabaseadaptor] Ensembl Database Adaptor
**
** @fcategory use
******************************************************************************/




/* @func ensSeqregionadaptorGetDatabaseadaptor ********************************
**
** Get the Ensembl Database Adaptor element of an
** Ensembl Sequence Region Adaptor.
**
** @param [r] sra [const EnsPSeqregionadaptor] Ensembl Sequence Region Adaptor
**
** @return [EnsPDatabaseadaptor] Ensembl Database Adaptor or NULL
** @@
******************************************************************************/

EnsPDatabaseadaptor ensSeqregionadaptorGetDatabaseadaptor(
    const EnsPSeqregionadaptor sra)
{
    if(!sra)
        return NULL;

    return sra->Adaptor;
}




/* @section load on demand ****************************************************
**
** Functions for returning elements of an Ensembl Sequence Region Adaptor
** object, which may need loading from an Ensembl SQL database on demand.
**
** @fdata [EnsPSeqregionadaptor]
**
** @nam3rule Load Return Ensembl Sequence Region Adaptor attribute(s)
** loaded on demand
** @nam4rule Locusreferencegenomic Return all Locus Reference Genomic (LRG)
** Ensembl Sequence Region objects
** @nam4rule Nonreferences Return all non-reference Ensembl Sequence Region
** objects
**
** @argrule * sra [EnsPSeqregionadaptor] Ensembl Sequence Region Adaptor
**
** @valrule Locusreferencegenomic [const AjPList] AJAX List of
** Ensembl Sequence Region objects or NULL
** @valrule Nonreferences [const AjPList] AJAX List of
** Ensembl Sequence Region objects or NULL
**
** @fcategory use
******************************************************************************/




/* @func ensSeqregionadaptorLoadLocusreferencegenomic *************************
**
** Load all Ensembl Sequence Region objects associated with an
** Ensembl Attribute of code "LRG", which is set for Locus Reference Genomic
** {LRG} entries.
** This function uses an Ensembl Sequence Region Adaptor-internal cache.
** The caller must not modify the AJAX List or its contents.
**
** @param [u] sra [EnsPSeqregionadaptor] Ensembl Sequence Region Adaptor
**
** @return [const AjPList] AJAX List of Ensembl Sequence Region objects
**                         or NULL
** @@
******************************************************************************/

const AjPList ensSeqregionadaptorLoadLocusreferencegenomic(
    EnsPSeqregionadaptor sra)
{
    if(!sra)
        return NULL;

    if(!sra->CacheLocusReferenceGenomic)
        seqregionadaptorCacheLocusReferenceGenomicInit(sra);

    return sra->CacheLocusReferenceGenomic;
}




/* @func ensSeqregionadaptorLoadNonreferences *********************************
**
** Load all Ensembl Sequence Region objects associated with an
** Ensembl Attribute of code "non_ref", which is set for haplotype assembly
** paths.
** This function uses an Ensembl Sequence Region Adaptor-internal cache.
** The caller must not modify the AJAX List or its contents.
**
** @param [u] sra [EnsPSeqregionadaptor] Ensembl Sequence Region Adaptor
**
** @return [const AjPList] AJAX List of Ensembl Sequence Region objects
**                         or NULL
** @@
******************************************************************************/

const AjPList ensSeqregionadaptorLoadNonreferences(
    EnsPSeqregionadaptor sra)
{
    if(!sra)
        return NULL;

    if(!sra->CacheNonReference)
        seqregionadaptorCacheNonReferenceInit(sra);

    return sra->CacheNonReference;
}




/* @section element retrieval *************************************************
**
** Functions for manipulating an Ensembl Sequence Region Adaptor cache.
**
** @fdata [EnsPSeqregionadaptor]
**
** @nam3rule Cache Manupulate an Ensembl Sequence Region Adaptor cache
** @nam4rule Insert Insert an Ensembl Sequence Region
** @nam4rule Remove Remove an Ensembl Sequence Region
**
** @argrule * sra [EnsPSeqregionadaptor] Ensembl Sequence Region Adaptor
** @argrule Insert Psr [EnsPSeqregion*] Ensembl Sequence Region address
** @argrule Remove sr [const EnsPSeqregion] Ensembl Sequence Region
**
** @valrule * [AjBool] ajTrue upon success, ajFalse otherwise
**
** @fcategory use
******************************************************************************/




/* @func ensSeqregionadaptorCacheInsert ***************************************
**
** Insert an Ensembl Sequence Region into the Sequence Region Adaptor-internal
** cache.
**
** @param [u] sra [EnsPSeqregionadaptor] Ensembl Sequence Region Adaptor
** @param [wP] Psr [EnsPSeqregion*] Ensembl Sequence Region address
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
** @@
******************************************************************************/

AjBool ensSeqregionadaptorCacheInsert(EnsPSeqregionadaptor sra,
                                      EnsPSeqregion* Psr)
{
    ajuint* Pid = NULL;

    AjPStr key = NULL;

    EnsPSeqregion sr1 = NULL;
    EnsPSeqregion sr2 = NULL;

    if(!sra)
        return ajFalse;

    if(!Psr)
        return ajFalse;

    if(!*Psr)
        return ajFalse;

    if(ajDebugTest("ensSeqregionadaptorCacheInsert"))
    {
        ajDebug("seqregionadaptorCacheInsert\n"
                "  sra %p\n"
                "  *Psr %p\n",
                sra,
                *Psr);

        ensSeqregionTrace(*Psr, 1);
    }

    /* Search the identifer cache. */

    AJNEW0(Pid);

    *Pid = (*Psr)->Identifier;

    ensCacheFetch(sra->CacheByIdentifier, (void*) Pid, (void**) &sr1);

    /* Search the name cache. */

    key = ajFmtStr("%u:%S",
                   ensCoordsystemGetIdentifier((*Psr)->Coordsystem),
                   (*Psr)->Name);

    sr2 = (EnsPSeqregion) ajTableFetchmodV(sra->CacheByName,
                                           (const void*) key);

    if((!sr1) && (!sr2))
    {
        /*
        ** None of the caches returned an identical Ensembl Sequence Region
        ** so add this one to both caches. The Ensembl LRU Cache automatically
        ** references the Sequence Region via the
        ** seqregionadaptorCacheReference function, while the AJAX Table-
        ** based cache needs manual referencing.
        */

        ensCacheStore(sra->CacheByIdentifier,
                      (void*) Pid,
                      (void**) Psr);

        ajTablePut(sra->CacheByName,
                   (void*) ajStrNewS(key),
                   (void*) ensSeqregionNewRef(*Psr));
    }

    if(sr1 && sr2 && (sr1 == sr2))
    {
        /*
        ** Both caches returned the same Ensembl Sequence Region so delete
        ** it and return the address of the one already in the cache.
        */

        ensSeqregionDel(Psr);

        *Psr = ensSeqregionNewRef(sr2);
    }

    if(sr1 && sr2 && (sr1 != sr2))
        ajDebug("ensSeqregionadaptorCacheInsert detected Sequence Regions in "
                "the identifier and name cache with identical "
                "Coordinate System identifiers and names "
                "('%u:%S' and '%u:%S') but differnt addresses "
                "(%p and %p).\n",
                ensCoordsystemGetIdentifier(sr1->Coordsystem),
                sr1->Name,
                ensCoordsystemGetIdentifier(sr2->Coordsystem),
                sr2->Name,
                sr1,
                sr2);

    if(sr1 && (!sr2))
        ajDebug("ensSeqregionadaptorCacheInsert detected a Sequence Region in "
                "the identifier, but not in the name cache.\n");

    if((!sr1) && sr2)
        ajDebug("ensSeqregionadaptorCacheInsert detected a Sequence Region in "
                "the name, but not in the identifier cache.\n");

    ensSeqregionDel(&sr1);

    ajStrDel(&key);

    AJFREE(Pid);

    return ajTrue;
}




/* @funcstatic seqregionadaptorCacheInsert ************************************
**
** An ajListMap "apply" function to insert Ensembl Sequence Regions into the
** Ensembl Sequence Region Adaptor-internal cache.
**
** @param [r] x [void**] Ensembl Sequence Region address
** @param [u] cl [void*] Ensembl Sequence Region Adaptor,
**                       passed in from ajListMap
** @see ajListMap
**
** @return [void]
** @@
******************************************************************************/

static void seqregionadaptorCacheInsert(void** x, void* cl)
{
    if(!x)
        return;

    if(!cl)
        return;

    ensSeqregionadaptorCacheInsert((EnsPSeqregionadaptor) cl,
                                   (EnsPSeqregion*) x);

    return;
}




/* @func ensSeqregionadaptorCacheRemove ***************************************
**
** Remove an Ensembl Sequence Region from an Ensembl Sequence Region
** Adaptor-internal cache.
**
** @param [u] sra [EnsPSeqregionadaptor] Ensembl Sequence Region Adaptor
** @param [r] sr [const EnsPSeqregion] Ensembl Sequence Region
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
** @@
** This is essentially a wrapper function to the ensCacheRemove function that
** can also be called automatically by the Ensembl LRU Cache to drop least
** recently used Ensembl Sequence Regions.
******************************************************************************/

AjBool ensSeqregionadaptorCacheRemove(EnsPSeqregionadaptor sra,
                                      const EnsPSeqregion sr)
{
    if(!sra)
        return ajFalse;

    if(!sr)
        return ajFalse;

    ensCacheRemove(sra->CacheByIdentifier, (const void*) &sr->Identifier);

    return ajTrue;
}




/* @section object retrieval **************************************************
**
** Functions for fetching Ensembl Sequence Region objects from an
** Ensembl Core database.
**
** @fdata [EnsPSeqregionadaptor]
**
** @nam3rule Fetch       Fetch Ensembl Sequence Region object(s)
** @nam4rule All         Fetch all Ensembl Sequence Region objects
** @nam4rule Allby       Fetch all Ensembl Sequence Region objects
**                       matching a criterion
** @nam5rule Attributecodevalue Fetch all by an ensembl Attribute code and
** optional value
** @nam5rule Coordsystem Fetch all by an Ensembl Coordinate System
** @nam4rule By          Fetch one Ensembl Sequence Region object
**                       matching a criterion
** @nam5rule Identifier  Fetch by an SQL database-internal identifier
** @nam5rule Name        Fetch by name
** @nam5rule Namefuzzy   Fetch by name via a fuzzy search
** @nam5rule Synonym     Fetch by a synonym
**
** @argrule * sra [EnsPSeqregionadaptor] Ensembl Sequence Region Adaptor
** @argrule AllbyAttributecodevalue code [const AjPStr] Ensembl Attribute
** code
** @argrule AllbyAttributecodevalue value [const AjPStr] Ensembl Attribute
** value
** @argrule AllbyCoordsystem cs [const EnsPCoordsystem] Ensembl Coordinate System
** @argrule ByIdentifier identifier [ajuint] SQL database-internal identifier
** @argrule ByName cs [const EnsPCoordsystem] Ensembl Coordinate System
** @argrule ByName name [const AjPStr] Name
** @argrule ByNamefuzzy cs [const EnsPCoordsystem] Ensembl Coordinate System
** @argrule ByNamefuzzy name [const AjPStr] Name
** @argrule BySynonym synonym [const AjPStr] Synonym
** @argrule All srs [AjPList] AJAX List of Ensembl Sequence Region objects
** @argrule Allby srs [AjPList] AJAX List of Ensembl Sequence Region objects
** @argrule By Psr [EnsPSeqregion*] Ensembl Sequence Region address
**
** @valrule * [AjBool] ajTrue upon success, ajFalse otherwise
**
** @fcategory use
******************************************************************************/




/* @funcstatic seqregionadaptorFetchAllbyStatement ****************************
**
** Run a SQL statement against an Ensembl Database Adaptor and consolidate the
** results into an AJAX List of Ensembl Sequence Regions.
** The caller is responsible for deleting the Ensembl Sequence Regions before
** deleting the AJAX List.
**
** @param [u] sra [EnsPSeqregionadaptor] Ensembl Sequence Region Adaptor
** @param [r] statement [const AjPStr] SQL statement
** @param [u] srs [AjPList] AJAX List of Ensembl Sequence Regions
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
** @@
******************************************************************************/

static AjBool seqregionadaptorFetchAllbyStatement(
    EnsPSeqregionadaptor sra,
    const AjPStr statement,
    AjPList srs)
{
    ajint length = 0;

    ajuint csid = 0;
    ajuint srid = 0;

    AjPSqlstatement sqls = NULL;
    AjISqlrow sqli       = NULL;
    AjPSqlrow sqlr       = NULL;

    AjPStr name = NULL;

    EnsPCoordsystem cs         = NULL;
    EnsPCoordsystemadaptor csa = NULL;

    EnsPSeqregion sr = NULL;

    if(!sra)
        return ajFalse;

    if(!statement)
        return ajFalse;

    if(!srs)
        return ajFalse;

    csa = ensRegistryGetCoordsystemadaptor(sra->Adaptor);

    sqls = ensDatabaseadaptorSqlstatementNew(sra->Adaptor, statement);

    sqli = ajSqlrowiterNew(sqls);

    while(!ajSqlrowiterDone(sqli))
    {
        srid   = 0;
        name   = ajStrNew();
        csid   = 0;
        length = 0;

        sqlr = ajSqlrowiterGet(sqli);

        ajSqlcolumnToUint(sqlr, &srid);
        ajSqlcolumnToStr(sqlr, &name);
        ajSqlcolumnToUint(sqlr, &csid);
        ajSqlcolumnToInt(sqlr, &length);

        ensCoordsystemadaptorFetchByIdentifier(csa, csid, &cs);

        if(!cs)
        {
            ajDebug("seqregionadaptorFetchAllbyStatement got an "
                    "Ensembl Sequence Region with identifier %u that "
                    "references a non-existent Ensembl Coordinate System "
                    "with identifier %u.", srid, csid);

            ajWarn("seqregionadaptorFetchAllbyStatement got an "
                   "Ensembl Sequence Region with identifier %u that "
                   "references a non-existent Ensembl Coordinate System "
                   "with identifier %u.", srid, csid);
        }

        sr = ensSeqregionNewIni(sra, srid, cs, name, length);

        ajListPushAppend(srs, (void*) sr);

        ensCoordsystemDel(&cs);

        ajStrDel(&name);
    }

    ajSqlrowiterDel(&sqli);

    ensDatabaseadaptorSqlstatementDel(sra->Adaptor, &sqls);

    return ajTrue;
}




/* @func ensSeqregionadaptorFetchAllbyAttributecodevalue **********************
**
** Fetch all Ensembl Sequence Regions via an Ensembl Attribute code and
** optional value.
**
** The caller is responsible for deleting the Ensembl Sequence Regions before
** deleting the AJAX List.
**
** @param [u] sra [EnsPSeqregionadaptor] Ensembl Sequence Region Adaptor
** @param [r] code [const AjPStr] Ensembl Attribute code
** @param [rN] value [const AjPStr] Ensembl Attribute value
** @param [u] srs [AjPList] AJAX List of Ensembl Sequence Region objects
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
** @@
******************************************************************************/

AjBool ensSeqregionadaptorFetchAllbyAttributecodevalue(
    EnsPSeqregionadaptor sra,
    const AjPStr code,
    const AjPStr value,
    AjPList srs)
{
    char* txtcode = NULL;
    char* txtvalue = NULL;

    AjPStr statement = NULL;

    if(!sra)
        return ajFalse;

    if(!(code && ajStrGetLen(code)))
        return ajFalse;

    if(!srs)
        return ajFalse;

    ensDatabaseadaptorEscapeC(sra->Adaptor, &txtcode, code);

    statement = ajFmtStr(
        "SELECT "
        "seq_region.seq_region_id, "
        "seq_region.name, "
        "seq_region.coord_system_id, "
        "seq_region.length "
        "FROM "
        "attrib_type, "
        "seq_region_attrib, "
        "seq_region "
        "WHERE "
        "attrib_type.code = '%s' "
        "AND "
        "attrib_type.attrib_type_id = seq_region_attrib.attrib_type_id "
        "AND "
        "seq_region_attrib.seq_region_id = seq_region.seq_region_id",
        txtcode);

    ajCharDel(&txtcode);

    if(value && ajStrGetLen(value))
    {
        ensDatabaseadaptorEscapeC(sra->Adaptor, &txtvalue, value);

        ajFmtPrintAppS(&statement,
                       " AND "
                       "seq_region_attrib.value = '%s'",
                       txtvalue);

        ajCharDel(&txtvalue);
    }

    seqregionadaptorFetchAllbyStatement(sra, statement, srs);

    ajStrDel(&statement);

    /* Insert all Sequence Regions into the adaptor-internal cache. */

    ajListMap(srs, seqregionadaptorCacheInsert, (void*) sra);

    return ajTrue;
}




/* @func ensSeqregionadaptorFetchAllbyCoordsystem *****************************
**
** Fetch all Ensembl Sequence Regions via an Ensembl Coordinate System.
** The caller is responsible for deleting the Ensembl Sequence Regions before
** deleting the AJAX List.
**
** @param [u] sra [EnsPSeqregionadaptor] Ensembl Sequence Region Adaptor
** @param [r] cs [const EnsPCoordsystem] Ensembl Coordinate System
** @param [u] srs [AjPList] AJAX List of Ensembl Sequence Region objects
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
** @@
******************************************************************************/

AjBool ensSeqregionadaptorFetchAllbyCoordsystem(EnsPSeqregionadaptor sra,
                                                const EnsPCoordsystem cs,
                                                AjPList srs)
{
    AjPStr statement = NULL;

    if(!sra)
        return ajFalse;

    if(!cs)
        return ajFalse;

    if(!srs)
        return ajFalse;

    if(ensCoordsystemGetToplevel(cs) == ajTrue)
        statement = ajFmtStr(
            "SELECT "
            "seq_region.seq_region_id, "
            "seq_region.name, "
            "seq_region.coord_system_id, "
            "seq_region.length "
            "FROM "
            "attrib_type, "
            "seq_region_attrib, "
            "seq_region, "
            "coord_system "
            "WHERE "
            "attrib_type.code = 'toplevel' "
            "AND "
            "attrib_type.attrib_type_id = seq_region_attrib.attrib_type_id "
            "AND "
            "seq_region_attrib.seq_region_id = seq_region.seq_region_id "
            "AND "
            "seq_region.coord_system_id = coord_system.coord_system_id "
            "AND "
            "coord_system.species_id = %u",
            ensDatabaseadaptorGetIdentifier(sra->Adaptor));
    else
        statement = ajFmtStr(
            "SELECT "
            "seq_region.seq_region_id, "
            "seq_region.name, "
            "seq_region.coord_system_id, "
            "seq_region.length "
            "FROM "
            "seq_region "
            "WHERE "
            "coord_system_id = %u",
            ensCoordsystemGetIdentifier(cs));

    seqregionadaptorFetchAllbyStatement(sra, statement, srs);

    ajStrDel(&statement);

    /* Insert all Sequence Regions into the adaptor-internal cache. */

    ajListMap(srs, seqregionadaptorCacheInsert, (void*) sra);

    return ajTrue;
}




/* @func ensSeqregionadaptorFetchByIdentifier *********************************
**
** Fetch an Ensembl Sequence Region by its SQL database-internal identifier.
** The caller is responsible for deleting the Ensembl Sequence Region.
**
** @param [u] sra [EnsPSeqregionadaptor] Ensembl Sequence Region Adaptor
** @param [r] identifier [ajuint] SQL database-internal identifier
** @param [wP] Psr [EnsPSeqregion*] Ensembl Sequence Region address
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
** @@
******************************************************************************/

AjBool ensSeqregionadaptorFetchByIdentifier(EnsPSeqregionadaptor sra,
                                            ajuint identifier,
                                            EnsPSeqregion* Psr)
{
    AjPList srs = NULL;

    AjPStr statement = NULL;

    if(!sra)
        return ajFalse;

    if(!identifier)
        return ajFalse;

    if(!Psr)
        return ajFalse;

    /*
    ** Initially, query the identifier cache.
    ** An Ensembl Cache automatically increments the reference counter of any
    ** returned object.
    */

    ensCacheFetch(sra->CacheByIdentifier, (void*) &identifier, (void**) Psr);

    if(*Psr)
        return ajTrue;

    /* Query the database in case no object was returned. */

    statement = ajFmtStr(
        "SELECT "
        "seq_region.seq_region_id, "
        "seq_region.name, "
        "seq_region.coord_system_id, "
        "seq_region.length "
        "FROM "
        "seq_region "
        "WHERE "
        "seq_region.seq_region_id = %u",
        identifier);

    srs = ajListNew();

    seqregionadaptorFetchAllbyStatement(sra, statement, srs);

    if(ajListGetLength(srs) == 0)
        ajDebug("ensSeqregionadaptorFetchByIdentifier got no "
                "Ensembl Sequence Region for identifier %u.\n",
                identifier);
    else if(ajListGetLength(srs) == 1)
    {
        ajListPop(srs, (void**) Psr);

        ensSeqregionadaptorCacheInsert(sra, Psr);
    }
    else if(ajListGetLength(srs) > 1)
    {
        ajDebug("ensSeqregionadaptorFetchByIdentifier got more than one "
                "Ensembl Sequence Region for identifier %u.\n",
                identifier);

        ajWarn("ensSeqregionadaptorFetchByIdentifier got more than one "
               "Ensembl Sequence Region for identifier %u.\n",
               identifier);

        while(ajListPop(srs, (void**) Psr))
            ensSeqregionDel(Psr);

        Psr = (EnsPSeqregion*) NULL;
    }

    ajListFree(&srs);

    ajStrDel(&statement);

    return ajTrue;
}




/* @func ensSeqregionadaptorFetchByName ***************************************
**
** Fetch an Ensembl Sequence Region by name and Ensembl Coordinate System.
** In case the top-level Ensembl Coordinate System or none at all has been
** specified, the Coordinate System of the highest rank will be assumed.
** The caller is responsible for deleting the Ensembl Sequence Region.
**
** @cc  Bio::EnsEMBL::DBSQL::Sliceadaptor::fetch_by_region
** @param [u] sra [EnsPSeqregionadaptor] Ensembl Sequence Region Adaptor
** @param [rN] cs [const EnsPCoordsystem] Ensembl Coordinate System
** @param [r] name [const AjPStr] Name
** @param [wP] Psr [EnsPSeqregion*] Ensembl Sequence Region address
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
** @@
******************************************************************************/

AjBool ensSeqregionadaptorFetchByName(EnsPSeqregionadaptor sra,
                                      const EnsPCoordsystem cs,
                                      const AjPStr name,
                                      EnsPSeqregion* Psr)
{
    char* txtname = NULL;

    AjPList srs = NULL;

    AjPStr key       = NULL;
    AjPStr statement = NULL;

    EnsPSeqregion sr = NULL;

    if(ajDebugTest("ensSeqregionadaptorFetchByName"))
        ajDebug("ensSeqregionadaptorFetchByName\n"
                "  sra %p\n"
                "  cs %p\n"
                "  name '%S'\n"
                "  Psr %p",
                sra,
                cs,
                name,
                Psr);

    if(!sra)
        return ajFalse;

    if(!(name && ajStrGetLen(name)))
        return ajFalse;

    if(!Psr)
        return ajFalse;

    /*
    ** Initially, search the name cache, which can only return a Sequence Region
    ** in case a regular Coordinate System has been specified. For requests
    ** specifying the top-level Coordinate System or no Coordinate System at
    ** all the database needs to be queried for the Sequence Region associated
    ** with the Coordinate System of the highest rank. However, all Sequence
    ** Regions will be inserted into the name cache with their true
    ** Coordinate System, keeping at least the memory requirements minimal.
    **
    ** For any object returned by the AJAX Table the reference counter needs
    ** to be incremented manually.
    */

    key = ajFmtStr("%u:%S", ensCoordsystemGetIdentifier(cs), name);

    *Psr = (EnsPSeqregion) ajTableFetchmodV(sra->CacheByName,
                                            (const void*) key);

    ajStrDel(&key);

    if(*Psr)
    {
        ensSeqregionNewRef(*Psr);

        return ajTrue;
    }

    ensDatabaseadaptorEscapeC(sra->Adaptor, &txtname, name);

    /*
    ** For top-level Coordinate Systems or in case no particular
    ** Coordinate System has been specified, request the Sequence Region
    ** associated with the Coordinate System of the highest rank.
    */

    if((!cs) || ensCoordsystemGetToplevel(cs))
        statement = ajFmtStr(
            "SELECT "
            "seq_region.seq_region_id, "
            "seq_region.name, "
            "seq_region.coord_system_id, "
            "seq_region.length "
            "FROM "
            "coord_system, "
            "seq_region "
            "WHERE "
            "coord_system.species_id = %u "
            "AND "
            "coord_system.coord_system_id = seq_region.coord_system_id "
            "AND "
            "seq_region.name = '%s' "
            "ORDER BY "
            "coord_system.rank "
            "ASC",
            ensDatabaseadaptorGetIdentifier(sra->Adaptor),
            txtname);
    else
        statement = ajFmtStr(
            "SELECT "
            "seq_region.seq_region_id, "
            "seq_region.name, "
            "seq_region.coord_system_id, "
            "seq_region.length "
            "FROM "
            "seq_region "
            "WHERE "
            "seq_region.coord_system_id = %u "
            "AND "
            "seq_region.name = '%s'",
            ensCoordsystemGetIdentifier(cs),
            txtname);

    ajCharDel(&txtname);

    srs = ajListNew();

    seqregionadaptorFetchAllbyStatement(sra, statement, srs);

    ajStrDel(&statement);

    /*
    ** Warn if more than one Ensembl Sequence Region has been returned,
    ** although this will frequently happen if either a top-level
    ** Ensembl Coordinate Systems has been specified or none at all.
    ** An Ensembl Core database may store assembly information for more than
    ** one assembly version to facilitate mapping between different versions.
    */

    if(ajListGetLength(srs) > 1)
        ajDebug("ensSeqregionadaptorFetchByName got more than one "
                "Ensembl Sequence Region for name '%S' and "
                "selected the one with the lowest rank.\n", name);

    /*
    ** Keep only the first Sequence Region, which is associated with the
    ** Coordinate System of the highest rank.
    */

    ajListPop(srs, (void**) Psr);

    ensSeqregionadaptorCacheInsert(sra, Psr);

    while(ajListPop(srs, (void**) &sr))
    {
        ensSeqregionadaptorCacheInsert(sra, &sr);

        ensSeqregionDel(&sr);
    }

    ajListFree(&srs);

    return ajTrue;
}




/* @func ensSeqregionadaptorFetchByNamefuzzy **********************************
**
** Fetch an Ensembl Sequence Region by name and Ensembl Coordinate System using
** a fuzzy match. This function is useful for bacterial arteficial chromosome
** (BAC) clone accession numbers without a sequence version.
** In case the top-level Ensembl Coordinate System or none at all has been
** specified, the Coordinate System of the highest rank will be assumed.
** The caller is responsible for deleting the Ensembl Sequence Region.
**
** @cc  Bio::EnsEMBL::DBSQL::Sliceadaptor::fetch_by_region
** @param [u] sra [EnsPSeqregionadaptor] Ensembl Sequence Region Adaptor
** @param [rN] cs [const EnsPCoordsystem] Ensembl Coordinate System
** @param [r] name [const AjPStr] Name
** @param [wP] Psr [EnsPSeqregion*] Ensembl Sequence Region address
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
** @@
******************************************************************************/

AjBool ensSeqregionadaptorFetchByNamefuzzy(EnsPSeqregionadaptor sra,
                                           const EnsPCoordsystem cs,
                                           const AjPStr name,
                                           EnsPSeqregion* Psr)
{
    char* txtname = NULL;

    register ajint i = 0;

    ajint reslen    = 0;
    ajint maxvernum = 0;
    ajint tmpvernum = 0;

    ajuint maxcsrank = 0;
    ajuint tmpcsrank = 0;

    AjPList srs      = NULL;
    AjPList complete = NULL;
    AjPList partial  = NULL;

    AjPRegexp expression = NULL;

    AjPStr statement = NULL;
    AjPStr tmpstr    = NULL;
    AjPStr tmpverstr = NULL;

    EnsPSeqregion sr    = NULL;
    EnsPSeqregion maxsr = NULL;

    if(ajDebugTest("ensSeqregionadaptorFetchByNamefuzzy"))
        ajDebug("ensSeqregionadaptorFetchByNamefuzzy\n"
                "  sra %p\n"
                "  cs %p\n"
                "  name '%S'\n"
                "  Psr %p",
                sra,
                cs,
                name,
                Psr);

    if(!sra)
        return ajFalse;

    if(!(name && ajStrGetLen(name)))
    {
        ajDebug("ensSeqregionadaptorFetchByNamefuzzy requires a "
                "Sequence Region name.\n");

        return ajFalse;
    }

    if(!Psr)
        return ajFalse;

    ensDatabaseadaptorEscapeC(sra->Adaptor, &txtname, name);

    /*
    ** For top-level Coordinate Systems or in case no particular
    ** Coordinate System has been specified, request the Sequence Region
    ** associated with the Coordinate System of the highest rank.
    */

    if((!cs) || ensCoordsystemGetToplevel(cs))
        statement = ajFmtStr(
            "SELECT "
            "seq_region.seq_region_id, "
            "seq_region.name, "
            "seq_region.coord_system_id, "
            "seq_region.length "
            "FROM "
            "coord_system, "
            "seq_region "
            "WHERE "
            "coord_system.species_id = %u "
            "AND "
            "coord_system.coord_system_id = seq_region.coord_system "
            "AND "
            "seq_region.name LIKE '%s%%' "
            "ORDER BY "
            "coord_system.rank "
            "ASC",
            ensDatabaseadaptorGetIdentifier(sra->Adaptor),
            txtname);
    else
        statement = ajFmtStr(
            "SELECT "
            "seq_region.seq_region_id, "
            "seq_region.name, "
            "seq_region.coord_system_id, "
            "seq_region.length "
            "FROM "
            "seq_region "
            "WHERE "
            "seq_region.coord_system_id = %u "
            "AND "
            "seq_region.name LIKE '%s%%'",
            ensCoordsystemGetIdentifier(cs),
            txtname);

    ajCharDel(&txtname);

    srs = ajListNew();

    seqregionadaptorFetchAllbyStatement(sra, statement, srs);

    ajStrDel(&statement);

    reslen = ajListGetLength(srs);

    complete = ajListNew();

    partial = ajListNew();

    while(ajListPop(srs, (void**) &sr))
    {
        /*
        ** Add all Ensembl Sequence Regions into the
        ** Ensembl Sequence Region Adaptor-internal cache.
        */

        ensSeqregionadaptorCacheInsert(sra, &sr);

        /* Prioritise Sequence Regions which names match completely. */

        if(ajStrMatchS(ensSeqregionGetName(sr), name))
            ajListPushAppend(complete, (void*) sr);
        else if(ajStrPrefixS(ensSeqregionGetName(sr), name))
            ajListPushAppend(partial, (void*) sr);
        else
        {
            ajDebug("ensSeqregionadaptorFetchByNamefuzzy got a "
                    "Sequence Region, which name '%S' does not start "
                    "with the name '%S' that was provided.\n",
                    ensSeqregionGetName(sr),
                    name);

            ensSeqregionDel(&sr);
        }
    }

    ajListFree(&srs);

    /*
    ** If there is a perfect match, keep only the first Sequence Region,
    ** which is associated with the Coordinate System of the highest rank.
    */

    if(ajListGetLength(complete))
        ajListPop(complete, (void**) Psr);
    else
    {
        tmpstr = ajStrNew();

        tmpverstr = ajStrNew();

        expression = ajRegCompC("^\\.([0-9]+)$");

        while(ajListPop(partial, (void**) &sr))
        {
            /* Get the sub-string with the non-matching suffix. */

            ajStrAssignSubS(&tmpstr,
                            ensSeqregionGetName(sr),
                            ajStrGetLen(name),
                            ajStrGetLen(ensSeqregionGetName(sr)));

            if(ajRegExec(expression, tmpstr))
            {
                /*
                ** Find the Sequence Region with the highest sequence
                ** version and the highest Coordinate System rank.
                */

                i = 0;

                while(ajRegSubI(expression, i, &tmpverstr))
                {
                    ajStrToInt(tmpverstr, (ajint*) &tmpvernum);

                    tmpcsrank = ensCoordsystemGetRank(sr->Coordsystem);

                    if((!maxvernum) ||
                       (tmpvernum > maxvernum) ||
                       ((tmpvernum == maxvernum) && (tmpcsrank < maxcsrank)))
                    {
                        maxcsrank = tmpcsrank;
                        maxvernum = tmpvernum;

                        ensSeqregionDel(&maxsr);

                        maxsr = sr;
                    }
                    else
                        ensSeqregionDel(&sr);

                    i++;
                }
            }
            else
                ensSeqregionDel(&sr);
        }

        ajRegFree(&expression);

        ajStrDel(&tmpverstr);
        ajStrDel(&tmpstr);

        *Psr = maxsr;
    }

    /* Delete all remaining Sequence Regions before deleting the AJAX Lists. */

    while(ajListPop(complete, (void**) &sr))
        ensSeqregionDel(&sr);

    ajListFree(&complete);

    while(ajListPop(partial, (void**) &sr))
        ensSeqregionDel(&sr);

    ajListFree(&partial);

    if(reslen > 1)
        ajWarn("ensSeqregionadaptorFetchByNamefuzzy returned more than one "
               "Ensembl Sequence Region. "
               "You might want to check whether the returned "
               "Ensembl Sequence Region '%S' is the one you intended to "
               "fetch '%S'.\n", ensSeqregionGetName(*Psr), name);

    return ajTrue;
}




/* @func ensSeqregionadaptorFetchBySynonym ************************************
**
** Fetch an Ensembl Sequence Region by an Ensembl Sequence Region Synonym.
**
** @cc  Bio::EnsEMBL::DBSQL::Sliceadaptor::fetch_by_region
** @param [u] sra [EnsPSeqregionadaptor] Ensembl Sequence Region Adaptor
** @param [r] synonym [const AjPStr] Synonym
** @param [wP] Psr [EnsPSeqregion*] Ensembl Sequence Region address
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
** @@
******************************************************************************/

AjBool ensSeqregionadaptorFetchBySynonym(EnsPSeqregionadaptor sra,
                                         const AjPStr synonym,
                                         EnsPSeqregion* Psr)
{
    AjBool result = AJFALSE;

    EnsPSeqregionsynonym        srs  = NULL;
    EnsPSeqregionsynonymadaptor srsa = NULL;

    if(!sra)
        return ajFalse;

    if((synonym == NULL) || (ajStrGetLen(synonym) == 0))
        return ajFalse;

    srsa = ensRegistryGetSeqregionsynonymadaptor(sra->Adaptor);

    result = ensSeqregionsynonymadaptorFetchBySynonym(srsa, synonym, &srs);

    if(result == ajFalse)
        return ajFalse;

    if(srs == NULL)
        return ajTrue;

    result = ensSeqregionadaptorFetchByIdentifier(
        sra,
        ensSeqregionsynonymGetSeqregionidentifier(srs),
        Psr);

    ensSeqregionsynonymDel(&srs);

    return result;
}




/* @section query *************************************************************
**
** Functions for querying the properties of an Ensembl Sequence Region object
** via an Ensembl Sequence Region Adaptor.
**
** @fdata [EnsPSeqregionadaptor]
**
** @nam3rule Is Check whether an Ensembl Sequence Region represents a
** certain property
** @nam4rule Locusreferencegenomic Check for a Locus Reference Genomic (LRG)
** Ensembl Sequence Region
** @nam4rule Nonreference Check for a non-reference Ensembl Sequence Region
** @nam4rule Toplevel Check for a top-level Ensembl Sequence Region
**
** @argrule * sra [EnsPSeqregionadaptor] Ensembl Sequence Region Adaptor
** @argrule Locusreferencegenomic sr [const EnsPSeqregion]
** Ensembl Sequence Region
** @argrule Locusreferencegenomic Presult [AjBool*] Boolean result
** @argrule Nonreference sr [const EnsPSeqregion] Ensembl Sequence Region
** @argrule Nonreference Presult [AjBool*] Boolean result
** @argrule Toplevel sr [const EnsPSeqregion] Ensembl Sequence Region
** @argrule Toplevel Presult [AjBool*] Boolean result
**
** @valrule * [AjBool] ajTrue upon success, ajFalse otherwise
**
** @fcategory use
******************************************************************************/




/* @func ensSeqregionadaptorIsLocusreferencegenomic ***************************
**
** Check if a particular Ensembl Sequence Region is associated with an
** Ensembl Attribute of code "LRG", which is set for Locus Reference Genomic
** entries.
** This function uses an Ensembl Sequence Region Adaptor-internal cache.
**
** @param [u] sra [EnsPSeqregionadaptor] Ensembl Sequence Region Adaptor
** @param [r] sr [const EnsPSeqregion] Ensembl Sequence Region
** @param [u] Presult [AjBool*] ajTrue:  This Sequence region has the
**                                       "LRG" attribute set.
**                              ajFalse: This Sequence Region is part of the
**                                       reference sequence.
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
** @@
******************************************************************************/

AjBool ensSeqregionadaptorIsLocusreferencegenomic(EnsPSeqregionadaptor sra,
                                                  const EnsPSeqregion sr,
                                                  AjBool* Presult)
{
    AjIList iterator = NULL;

    EnsPSeqregion pthis = NULL;

    if(ajDebugTest("ensSeqregionadaptorIsLocusreferencegenomic"))
        ajDebug("ensSeqregionadaptorIsLocusreferencegenomic\n"
                "  sra %p\n"
                "  sr %p\n"
                "  Presult %p\n",
                sra,
                sr,
                Presult);

    if(!sra)
        return ajFalse;

    if(!sr)
        return ajFalse;

    if(!Presult)
        return ajFalse;

    *Presult = ajFalse;

    if(!sra->CacheLocusReferenceGenomic)
        seqregionadaptorCacheLocusReferenceGenomicInit(sra);

    iterator = ajListIterNewread(sra->CacheLocusReferenceGenomic);

    while(!ajListIterDone(iterator))
    {
        pthis = (EnsPSeqregion) ajListIterGet(iterator);

        if(ensSeqregionMatch(sr, pthis))
        {
            *Presult = ajTrue;

            break;
        }
    }

    ajListIterDel(&iterator);

    return ajTrue;
}




/* @func ensSeqregionadaptorIsNonreference ************************************
**
** Check if a particular Ensembl Sequence Region is associated with an
** Ensembl Attribute of code "non_ref", which is set for haplotype assembly
** paths.
** This function uses an Ensembl Sequence Region Adaptor-internal cache.
**
** @param [u] sra [EnsPSeqregionadaptor] Ensembl Sequence Region Adaptor
** @param [r] sr [const EnsPSeqregion] Ensembl Sequence Region
** @param [u] Presult [AjBool*] ajTrue:  This Sequence region has the
**                                       "non_ref" attribute set.
**                              ajFalse: This Sequence Region is part of the
**                                       reference sequence.
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
** @@
******************************************************************************/

AjBool ensSeqregionadaptorIsNonreference(EnsPSeqregionadaptor sra,
                                         const EnsPSeqregion sr,
                                         AjBool* Presult)
{
    AjIList iterator = NULL;

    EnsPSeqregion pthis = NULL;

    if(ajDebugTest("ensSeqregionadaptorIsNonreference"))
        ajDebug("ensSeqregionadaptorIsNonreference\n"
                "  sra %p\n"
                "  sr %p\n"
                "  Presult %p\n",
                sra,
                sr,
                Presult);

    if(!sra)
        return ajFalse;

    if(!sr)
        return ajFalse;

    if(!Presult)
        return ajFalse;

    *Presult = ajFalse;

    if(!sra->CacheNonReference)
        seqregionadaptorCacheNonReferenceInit(sra);

    iterator = ajListIterNewread(sra->CacheNonReference);

    while(!ajListIterDone(iterator))
    {
        pthis = (EnsPSeqregion) ajListIterGet(iterator);

        if(ensSeqregionMatch(sr, pthis))
        {
            *Presult = ajTrue;

            break;
        }
    }

    ajListIterDel(&iterator);

    return ajTrue;
}




/* @datasection [EnsPSeqregionsynonym] Ensembl Sequence Region Synonym ********
**
** @nam2rule Seqregionsynonym Functions for manipulating
** Ensembl Sequence Region Synonym objects
**
** @cc Bio::EnsEMBL::SeqRegionSynonym
** @cc CVS Revision: 1.2
** @cc CVS Tag: branch-ensembl-62
**
******************************************************************************/




/* @section constructors ******************************************************
**
** All constructors return a new Ensembl Sequence Region Synonym by pointer.
** It is the responsibility of the user to first destroy any previous
** Sequence Region Synonym. The target pointer does not need to be initialised
** to NULL, but it is good programming practice to do so anyway.
**
** @fdata [EnsPSeqregionsynonym]
**
** @nam3rule New Constructor
** @nam4rule Cpy Constructor with existing object
** @nam4rule Ini Constructor with initial values
** @nam4rule Ref Constructor by incrementing the reference counter
**
** @argrule Cpy srs [EnsPSeqregionsynonym] Ensembl Sequence Region Synonym
** @argrule Ini srsa [EnsPSeqregionsynonymadaptor]
** Ensembl Sequence Region Synonym Adaptor
** @argrule Ini identifier [ajuint] SQL database-internal identifier
** @argrule Ini edb [EnsPExternaldatabase] Ensembl External Database
** @argrule Ini name [AjPStr] Name
** @argrule Ini srid [ajuint] Ensembl Sequence Region identifier
** @argrule Ref srs [EnsPSeqregionsynonym] Ensembl Sequence Region Synonym
**
** @valrule * [EnsPSeqregionsynonym] Ensembl Sequence Region Synonym
**
** @fcategory new
******************************************************************************/




/* @func ensSeqregionsynonymNewCpy ********************************************
**
** Object-based constructor function, which returns an independent object.
**
** @param [u] srs [EnsPSeqregionsynonym] Ensembl Sequence Region Synonym
**
** @return [EnsPSeqregionsynonym] Ensembl Sequence Region Synonym or NULL
** @@
******************************************************************************/

EnsPSeqregionsynonym ensSeqregionsynonymNewCpy(EnsPSeqregionsynonym srs)
{
    EnsPSeqregionsynonym pthis = NULL;

    if(!srs)
        return NULL;

    AJNEW0(pthis);

    pthis->Use = 1;

    pthis->Identifier = srs->Identifier;

    pthis->Adaptor = srs->Adaptor;

    pthis->Externaldatabase = ensExternaldatabaseNewRef(srs->Externaldatabase);

    if(srs->Name)
        pthis->Name = ajStrNewRef(srs->Name);

    pthis->Seqregionidentifier = srs->Seqregionidentifier;

    return pthis;
}




/* @func ensSeqregionsynonymNewIni ********************************************
**
** Constructor for an Ensembl Sequence Region Synonym with initial values.
**
** @cc Bio::EnsEMBL::Storable
** @param [uN] srsa [EnsPSeqregionsynonymadaptor]
** Ensembl Sequence Region Synonym Adaptor
** @param [rE] identifier [ajuint] SQL database-internal identifier
** @cc Bio::EnsEMBL::SeqRegionSynonym
** @param [u] edb [EnsPExternaldatabase] Ensembl External Database
** @param [u] name [AjPStr] Name
** @param [r] srid [ajuint] Ensembl Sequence Region identifier
**
** @return [EnsPSeqregionsynonym] Ensembl Sequence Region Synonym or NULL
** @@
******************************************************************************/

EnsPSeqregionsynonym ensSeqregionsynonymNewIni(
    EnsPSeqregionsynonymadaptor srsa,
    ajuint identifier,
    EnsPExternaldatabase edb,
    AjPStr name,
    ajuint srid)
{
    EnsPSeqregionsynonym srs = NULL;

    if(!edb)
        return NULL;

    if((name == NULL) || (ajStrGetLen(name) == 0))
        return NULL;

    if(srid == 0)
        return NULL;

    AJNEW0(srs);

    srs->Use = 1;

    srs->Identifier = identifier;

    srs->Adaptor = srsa;

    srs->Externaldatabase = ensExternaldatabaseNewRef(edb);

    if(name)
        srs->Name = ajStrNewRef(name);

    srs->Seqregionidentifier = srid;

    return srs;
}




/* @func ensSeqregionsynonymNewRef ********************************************
**
** Ensembl Object referencing function, which returns a pointer to the
** Ensembl Object passed in and increases its reference count.
**
** @param [u] srs [EnsPSeqregionsynonym] Ensembl Sequence Region Synonym
**
** @return [EnsPSeqregionsynonym] Ensembl Sequence Region Synonym or NULL
** @@
******************************************************************************/

EnsPSeqregionsynonym ensSeqregionsynonymNewRef(EnsPSeqregionsynonym srs)
{
    if(!srs)
        return NULL;

    srs->Use++;

    return srs;
}




/* @section destructors *******************************************************
**
** Destruction destroys all internal data structures and frees the
** memory allocated for an Ensembl Sequence Region Synonym object.
**
** @fdata [EnsPSeqregionsynonym]
**
** @nam3rule Del Destroy (free) an Ensembl Sequence Region Synonym object
**
** @argrule * Psrs [EnsPSeqregionsynonym*] Ensembl Sequence Region Synonym
**                                         object address
**
** @valrule * [void]
**
** @fcategory delete
******************************************************************************/




/* @func ensSeqregionsynonymDel ***********************************************
**
** Default destructor for an Ensembl Sequence Region Synonym.
**
** @param [d] Psrs [EnsPSeqregionsynonym*] Ensembl Sequence Region Synonym
**                                         object address
**
** @return [void]
** @@
******************************************************************************/

void ensSeqregionsynonymDel(EnsPSeqregionsynonym* Psrs)
{
    EnsPSeqregionsynonym pthis = NULL;

    if(!Psrs)
        return;

    if(!*Psrs)
        return;

    pthis = *Psrs;

    pthis->Use--;

    if(pthis->Use)
    {
        *Psrs = NULL;

        return;
    }

    ensExternaldatabaseDel(&pthis->Externaldatabase);

    ajStrDel(&pthis->Name);

    AJFREE(pthis);

    *Psrs = NULL;

    return;
}




/* @section element retrieval *************************************************
**
** Functions for returning elements of an
** Ensembl Sequence Region Synonym object.
**
** @fdata [EnsPSeqregionsynonym]
**
** @nam3rule Get Return Sequence Region Synonym attribute(s)
** @nam4rule Adaptor Return the Ensembl Sequence Region Synonym Adaptor
** @nam4rule Externaldatabase Return the Ensembl External Database
** @nam4rule Identifier Return the SQL database-internal identifier
** @nam4rule Name Return the name
** @nam4rule Seqregionidentifier Return the Ensembl Sequence Region identifier
**
** @argrule * srs [const EnsPSeqregionsynonym] Ensembl Sequence Region Synonym
**
** @valrule Adaptor [EnsPSeqregionsynonymadaptor]
** Ensembl Sequence Region Synonym Adaptor or NULL
** @valrule Externaldatabase [EnsPExternaldatabase]
** Ensembl External Database or NULL
** @valrule Identifier [ajuint] SQL database-internal identifier or 0
** @valrule Name [AjPStr] Name or NULL
** @valrule Seqregionidentifier [ajuint]
** Ensembl Sequence Region Identifier or 0
**
** @fcategory use
******************************************************************************/




/* @func ensSeqregionsynonymGetAdaptor ****************************************
**
** Get the Ensembl Sequence Region Synonym Adaptor element of an
** Ensembl Sequence Region Synonym.
**
** @param [r] srs [const EnsPSeqregionsynonym] Ensembl Sequence Region Synonym
**
** @return [EnsPSeqregionsynonymadaptor]
** Ensembl Sequence Region Synonym Adaptor or NULL
** @@
******************************************************************************/

EnsPSeqregionsynonymadaptor ensSeqregionsynonymGetAdaptor(
    const EnsPSeqregionsynonym srs)
{
    if(!srs)
        return NULL;

    return srs->Adaptor;
}




/* @func ensSeqregionsynonymGetExternaldatabase *******************************
**
** Get the Ensembl External Database element of an
** Ensembl Sequence Region Synonym.
**
** @param [r] srs [const EnsPSeqregionsynonym] Ensembl Sequence Region Synonym
**
** @return [EnsPExternaldatabase] Ensembl External Database or NULL
** @@
******************************************************************************/

EnsPExternaldatabase ensSeqregionsynonymGetExternaldatabase(
    const EnsPSeqregionsynonym srs)
{
    if(!srs)
        return NULL;

    return srs->Externaldatabase;
}




/* @func ensSeqregionsynonymGetIdentifier *************************************
**
** Get the SQL database-internal identifier element of an
** Ensembl Sequence Region Synonym.
**
** @param [r] srs [const EnsPSeqregionsynonym] Ensembl Sequence Region Synonym
**
** @return [ajuint] SQL database-internal identifier or 0
** @@
******************************************************************************/

ajuint ensSeqregionsynonymGetIdentifier(
    const EnsPSeqregionsynonym srs)
{
    if(!srs)
        return 0;

    return srs->Identifier;
}




/* @func ensSeqregionsynonymGetName *******************************************
**
** Get the name element of an Ensembl Sequence Region Synonym.
**
** @param [r] srs [const EnsPSeqregionsynonym] Ensembl Sequence Region Synonym
**
** @return [AjPStr] Name or NULL
** @@
******************************************************************************/

AjPStr ensSeqregionsynonymGetName(
    const EnsPSeqregionsynonym srs)
{
    if(!srs)
        return NULL;

    return srs->Name;
}




/* @func ensSeqregionsynonymGetSeqregionidentifier ****************************
**
** Get the Ensembl Sequence Region identifier element of an
** Ensembl Sequence Region Synonym.
**
** @param [r] srs [const EnsPSeqregionsynonym] Ensembl Sequence Region Synonym
**
** @return [ajuint] Ensembl Sequence Region identifier or 0
** @@
******************************************************************************/

ajuint ensSeqregionsynonymGetSeqregionidentifier(
    const EnsPSeqregionsynonym srs)
{
    if(!srs)
        return 0;

    return srs->Seqregionidentifier;
}




/* @section modifiers *********************************************************
**
** Functions for assigning elements of an
** Ensembl Sequence Region Synonym object.
**
** @fdata [EnsPSeqregionsynonym]
**
** @nam3rule Set Set one element of an Ensembl Sequence Region Synonym
** @nam4rule Adaptor Set the Ensembl Sequence Region Synonym Adaptor
** @nam4rule Externaldatabase Set the Ensembl External Database
** @nam4rule Identifier Set the SQL database-internal identifier
** @nam4rule Name Set the name
** @nam4rule Seqregionidentifier Set the Ensembl Sequence Region identifier
**
** @argrule * srs [EnsPSeqregionsynonym] Ensembl Sequence Region Synonym
** @argrule Adaptor srsa [EnsPSeqregionsynonymadaptor]
** Ensembl Sequence Region Synonym Adaptor
** @argrule Externaldatabase edb [EnsPExternaldatabase]
** Ensembl External Database
** @argrule Identifier identifier [ajuint] SQL database-internal identifier
** @argrule Name name [AjPStr] Name
** @argrule Seqregionidentifier srid [ajuint]
** Ensembl Sequence Region identifier
**
** @valrule * [AjBool] ajTrue upon success, ajFalse otherwise
**
** @fcategory modify
******************************************************************************/




/* @func ensSeqregionsynonymSetAdaptor ****************************************
**
** Set the Ensembl Sequence Region Synonym Adaptor element of an
** Ensembl Sequence Region Synonym.
**
** @param [u] srs [EnsPSeqregionsynonym] Ensembl Sequence Region Synonym
** @param [u] srsa [EnsPSeqregionsynonymadaptor]
** Ensembl Sequence Region Synonym Adaptor
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
** @@
******************************************************************************/

AjBool ensSeqregionsynonymSetAdaptor(EnsPSeqregionsynonym srs,
                                     EnsPSeqregionsynonymadaptor srsa)
{
    if(!srs)
        return ajFalse;

    srs->Adaptor = srsa;

    return ajTrue;
}




/* @func ensSeqregionsynonymSetExternaldatabase *******************************
**
** Set the Ensembl External Database element of an
** Ensembl Sequence Region Synonym.
**
** @param [u] srs [EnsPSeqregionsynonym] Ensembl Sequence Region Synonym
** @param [uN] edb [EnsPExternaldatabase] Ensembl External Database
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
** @@
******************************************************************************/

AjBool ensSeqregionsynonymSetExternaldatabase(EnsPSeqregionsynonym srs,
                                              EnsPExternaldatabase edb)
{
    if(!srs)
        return ajFalse;

    ensExternaldatabaseDel(&srs->Externaldatabase);

    srs->Externaldatabase = ensExternaldatabaseNewRef(edb);

    return ajTrue;
}




/* @func ensSeqregionsynonymSetIdentifier *************************************
**
** Set the SQL database-internal identifier element of an
** Ensembl Sequence Region Synonym.
**
** @param [u] srs [EnsPSeqregionsynonym] Ensembl Sequence Region Synonym
** @param [r] identifier [ajuint] SQL database-internal identifier
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
** @@
******************************************************************************/

AjBool ensSeqregionsynonymSetIdentifier(EnsPSeqregionsynonym srs,
                                        ajuint identifier)
{
    if(!srs)
        return ajFalse;

    srs->Identifier = identifier;

    return ajTrue;
}




/* @func ensSeqregionsynonymSetName *******************************************
**
** Set the name element of an Ensembl Sequence Region Synonym.
**
** @param [u] srs [EnsPSeqregionsynonym] Ensembl Sequence Region Synonym
** @param [uN] name [AjPStr] Name
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
** @@
******************************************************************************/

AjBool ensSeqregionsynonymSetName(EnsPSeqregionsynonym srs,
                                  AjPStr name)
{
    if(!srs)
        return ajFalse;

    ajStrDel(&srs->Name);

    if(name)
        srs->Name = ajStrNewRef(name);

    return ajTrue;
}




/* @func ensSeqregionsynonymSetSeqregionidentifier ****************************
**
** Set the Ensembl Sequence Region identifier element of an
** Ensembl Sequence Region.
**
** @param [u] srs [EnsPSeqregionsynonym] Ensembl Sequence Region Synonym
** @param [r] srid [ajuint] Ensembl Sequence Region Synonym
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
** @@
******************************************************************************/

AjBool ensSeqregionsynonymSetSeqregionidentifier(EnsPSeqregionsynonym srs,
                                                 ajuint srid)
{
    if(!srs)
        return ajFalse;

    srs->Seqregionidentifier = srid;

    return ajTrue;
}




/* @section debugging *********************************************************
**
** Functions for reporting of an Ensembl Sequence Region Synonym object.
**
** @fdata [EnsPSeqregionsynonym]
**
** @nam3rule Trace Report Ensembl Sequence Region Synonym elements
** to debug file
**
** @argrule Trace srs [const EnsPSeqregionsynonym]
** Ensembl Sequence Region Synonym
** @argrule Trace level [ajuint] Indentation level
**
** @valrule * [AjBool] ajTrue upon success, ajFalse otherwise
**
** @fcategory misc
******************************************************************************/




/* @func ensSeqregionsynonymTrace *********************************************
**
** Trace an Ensembl Sequence Region Synonym.
**
** @param [r] srs [const EnsPSeqregionsynonym] Ensembl Sequence Region Synonym
** @param [r] level [ajuint] Indentation level
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
** @@
******************************************************************************/

AjBool ensSeqregionsynonymTrace(const EnsPSeqregionsynonym srs, ajuint level)
{
    AjPStr indent = NULL;

    if(!srs)
        return ajFalse;

    indent = ajStrNew();

    ajStrAppendCountK(&indent, ' ', level * 2);

    ajDebug("%SensSeqregionsynonymTrace %p\n"
            "%S  Use %u\n"
            "%S  Identifier %u\n"
            "%S  Adaptor %p\n"
            "%S  Externaldatabase %p\n"
            "%S  Name '%S'\n"
            "%S  Seqregionidentifier %u\n",
            indent, srs,
            indent, srs->Use,
            indent, srs->Identifier,
            indent, srs->Adaptor,
            indent, srs->Externaldatabase,
            indent, srs->Name,
            indent, srs->Seqregionidentifier);

    ensExternaldatabaseTrace(srs->Externaldatabase, level + 1);

    ajStrDel(&indent);

    return ajTrue;
}




/* @section calculate *********************************************************
**
** Functions for calculating values of an
** Ensembl Sequence Region Synonym object.
**
** @fdata [EnsPSeqregionsynonym]
**
** @nam3rule Calculate Calculate Ensembl Sequence Region Synonym values
** @nam4rule Memsize Calculate the memory size in bytes
**
** @argrule Memsize srs [const EnsPSeqregionsynonym]
** Ensembl Sequence Region Synonym
**
** @valrule Memsize [size_t] Memory size in bytes or 0
**
** @fcategory misc
******************************************************************************/




/* @func ensSeqregionsynonymCalculateMemsize **********************************
**
** Calculate the memory size in bytes of an Ensembl Sequence Region Synonym.
**
** @param [r] srs [const EnsPSeqregionsynonym] Ensembl Sequence Region Synonym
**
** @return [size_t] Memory size in bytes or 0
** @@
******************************************************************************/

size_t ensSeqregionsynonymCalculateMemsize(const EnsPSeqregionsynonym srs)
{
    size_t size = 0;

    if(!srs)
        return 0;

    size += sizeof (EnsOSeqregionsynonym);

    size += ensExternaldatabaseCalculateMemsize(srs->Externaldatabase);

    if(srs->Name)
    {
        size += sizeof (AjOStr);

        size += ajStrGetRes(srs->Name);
    }

    return size;
}




/* @datasection [EnsPSeqregionsynonymadaptor]
** Ensembl Sequence Region Synonym Adaptor
**
** @nam2rule Seqregionsynonymadaptor Functions for manipulating
** Ensembl Sequence Region Synonym Adaptor objects
**
** @cc Bio::EnsEMBL::DBSQL::SeqRegionSynonymAdaptor
** @cc CVS Revision: 1.2
** @cc CVS Tag: branch-ensembl-62
**
******************************************************************************/




/* @funcstatic seqregionsynonymadaptorFetchAllbyStatement *********************
**
** Run a SQL statement against an Ensembl Database Adaptor and consolidate the
** results into an AJAX List of Ensembl Sequence Region Synonym objects.
**
** @param [u] dba [EnsPDatabaseadaptor] Ensembl Database Adaptor
** @param [r] statement [const AjPStr] SQL statement
** @param [uN] am [EnsPAssemblymapper] Ensembl Assembly Mapper
** @param [uN] slice [EnsPSlice] Ensembl Slice
** @param [u] srss [AjPList]
** AJAX List of Ensembl Sequence Region Synonym objects
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
** @@
******************************************************************************/

static AjBool seqregionsynonymadaptorFetchAllbyStatement(
    EnsPDatabaseadaptor dba,
    const AjPStr statement,
    EnsPAssemblymapper am,
    EnsPSlice slice,
    AjPList srss)
{
    ajuint identifier = 0;
    ajuint edbid      = 0;
    ajuint srid       = 0;

    AjPSqlstatement sqls = NULL;
    AjISqlrow sqli       = NULL;
    AjPSqlrow sqlr       = NULL;

    AjPStr name            = NULL;

    EnsPExternaldatabase        edb  = NULL;
    EnsPExternaldatabaseadaptor edba = NULL;

    EnsPSeqregionsynonym        srs  = NULL;
    EnsPSeqregionsynonymadaptor srsa = NULL;

    if(ajDebugTest("seqregionsynonymadaptorFetchAllbyStatement"))
        ajDebug("seqregionsynonymadaptorFetchAllbyStatement\n"
                "  dba %p\n"
                "  statement %p\n"
                "  am %p\n"
                "  slice %p\n"
                "  srss %p\n",
                dba,
                statement,
                am,
                slice,
                srss);

    if(!dba)
        return ajFalse;

    if(!statement)
        return ajFalse;

    if(!srss)
        return ajFalse;

    edba = ensRegistryGetExternaldatabaseadaptor(dba);
    srsa = ensRegistryGetSeqregionsynonymadaptor(dba);

    sqls = ensDatabaseadaptorSqlstatementNew(dba, statement);

    sqli = ajSqlrowiterNew(sqls);

    while(!ajSqlrowiterDone(sqli))
    {
        identifier = 0;
        srid       = 0;
        name       = ajStrNew();
        edbid      = 0;

        sqlr = ajSqlrowiterGet(sqli);

        ajSqlcolumnToUint(sqlr, &identifier);
        ajSqlcolumnToUint(sqlr, &srid);
        ajSqlcolumnToStr(sqlr, &name);
        ajSqlcolumnToUint(sqlr, &edbid);

        ensExternaldatabaseadaptorFetchByIdentifier(edba, edbid, &edb);

        srs = ensSeqregionsynonymNewIni(srsa, identifier, edb, name, srid);

        ajListPushAppend(srss, (void*) srs);

        ajStrDel(&name);
    }

    ajSqlrowiterDel(&sqli);

    ensDatabaseadaptorSqlstatementDel(dba, &sqls);

    return ajTrue;
}




/* @section constructors ******************************************************
**
** All constructors return a new Ensembl Sequence Region Synonym Adaptor
** by pointer. It is the responsibility of the user to first destroy any
** previous Sequence Region Synonym Adaptor. The target pointer does not need
** to be initialised to NULL, but it is good programming practice to do so
** anyway.
**
** @fdata [EnsPSeqregionsynonymadaptor]
**
** @nam3rule New Constructor
**
** @argrule New dba [EnsPDatabaseadaptor] Ensembl Database Adaptor
**
** @valrule * [EnsPSeqregionsynonymadaptor]
** Ensembl Sequence Region Synonym Adaptor or NULL
**
** @fcategory new
******************************************************************************/




/* @func ensSeqregionsynonymadaptorNew ****************************************
**
** Default constructor for an Ensembl Sequence Region Synonym Adaptor.
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
** @see ensRegistryGetSeqregionsynonymadaptor
**
** @cc Bio::EnsEMBL::DBSQL::SeqRegionSynonymAdaptor::new
** @param [u] dba [EnsPDatabaseadaptor] Ensembl Database Adaptor
**
** @return [EnsPSeqregionsynonymadaptor]
** Ensembl Sequence Region Synonym Adaptor or NULL
** @@
******************************************************************************/

EnsPSeqregionsynonymadaptor ensSeqregionsynonymadaptorNew(
    EnsPDatabaseadaptor dba)
{
    if(!dba)
        return NULL;

    return ensBaseadaptorNew(
        dba,
        seqregionsynonymadaptorTables,
        seqregionsynonymadaptorColumns,
        (EnsPBaseadaptorLeftjoin) NULL,
        (const char*) NULL,
        (const char*) NULL,
        seqregionsynonymadaptorFetchAllbyStatement);
}




/* @section destructors *******************************************************
**
** Destruction destroys all internal data structures and frees the
** memory allocated for an Ensembl Sequence Region Synonym Adaptor object.
**
** @fdata [EnsPSeqregionsynonymadaptor]
**
** @nam3rule Del Destroy (free) an Ensembl Sequence Region Synonym Adaptor
** object.
**
** @argrule * Psrsa [EnsPSeqregionsynonymadaptor*]
** Ensembl Sequence Region Synonym Adaptor object address
**
** @valrule * [void]
**
** @fcategory delete
******************************************************************************/




/* @func ensSeqregionsynonymadaptorDel ****************************************
**
** Default destructor for an Ensembl Sequence Region Synonym Adaptor.
**
** This function also clears the internal caches.
**
** Ensembl Object Adaptors are singleton objects that are registered in the
** Ensembl Registry and weakly referenced by Ensembl Objects that have been
** instantiated by it. Therefore, Ensembl Object Adaptors should never be
** destroyed directly. Upon exit, the Ensembl Registry will call this function
** if required.
**
** @param [d] Psrsa [EnsPSeqregionsynonymadaptor*]
** Ensembl Sequence Region Synonym Adaptor object address
**
** @return [void]
** @@
******************************************************************************/

void ensSeqregionsynonymadaptorDel(EnsPSeqregionsynonymadaptor* Psrsa)
{
    EnsPSeqregionsynonymadaptor pthis = NULL;

    if(!Psrsa)
        return;

    if(!*Psrsa)
        return;

    pthis = *Psrsa;

    ensBaseadaptorDel(&pthis);

    *Psrsa = NULL;

    return;
}




/* @section element retrieval *************************************************
**
** Functions for returning elements of an
** Ensembl Sequence Region Synonym Adaptor object.
**
** @fdata [EnsPSeqregionsynonymadaptor]
**
** @nam3rule Get Return Ensembl Sequence Region Synonym Adaptor attribute(s)
** @nam4rule Baseadaptor Return the Ensembl Base Adaptor
** @nam4rule Databaseadaptor Return the Ensembl Database Adaptor
**
** @argrule Baseadaptor srsa [EnsPSeqregionsynonymadaptor]
** Ensembl Sequence Region Synonym Adaptor
** @argrule Databaseadaptor srsa [const EnsPSeqregionsynonymadaptor]
** Ensembl Sequence Region Synonym Adaptor
**
** @valrule Baseadaptor [EnsPBaseadaptor] Ensembl Base Adaptor or NULL
** @valrule Databaseadaptor [EnsPDatabaseadaptor] Ensembl Database Adaptor
** or NULL
**
** @fcategory use
******************************************************************************/




/* @func ensSeqregionsynonymadaptorGetBaseadaptor *****************************
**
** Get the Ensembl Base Adaptor element of an
** Ensembl Sequence Region Synonym Adaptor.
**
** @param [u] srsa [EnsPSeqregionsynonymadaptor]
** Ensembl Sequence Region Synonym Adaptor
**
** @return [EnsPBaseadaptor] Ensembl Base Adaptor or NULL
** @@
******************************************************************************/

EnsPBaseadaptor ensSeqregionsynonymadaptorGetBaseadaptor(
    EnsPSeqregionsynonymadaptor srsa)
{
    if(!srsa)
        return NULL;

    return srsa;
}




/* @func ensSeqregionsynonymadaptorGetDatabaseadaptor *************************
**
** Get the Ensembl Database Adaptor element of an
** Ensembl Sequence Region Synonym Adaptor.
**
** @param [r] srsa [const EnsPSeqregionsynonymadaptor]
** Ensembl Sequence Region Synonym Adaptor
**
** @return [EnsPDatabaseadaptor] Ensembl Database Adaptor or NULL
** @@
******************************************************************************/

EnsPDatabaseadaptor ensSeqregionsynonymadaptorGetDatabaseadaptor(
    const EnsPSeqregionsynonymadaptor srsa)
{
    if(!srsa)
        return NULL;

    return ensBaseadaptorGetDatabaseadaptor(srsa);
}




/* @section object retrieval **************************************************
**
** Functions for fetching Ensembl Sequence Region Synonym objects from an
** Ensembl SQL database.
**
** @fdata [EnsPSeqregionsynonymadaptor]
**
** @nam3rule Fetch       Fetch Ensembl Sequence Region Synonym object(s)
** @nam4rule All         Fetch all Ensembl Sequence Region Synonym objects
** @nam4rule Allby       Fetch all Ensembl Sequence Region Synonym objects
**                       matching a criterion
** @nam5rule Seqregion   Fetch all by an Ensembl Sequence Region
** @nam4rule By          Fetch one Ensembl Sequence Region Synonym object
**                       matching a criterion
** @nam5rule Identifier  Fetch by an SQL database-internal identifier
** @nam5rule Name        Fetch by name
** @nam5rule Namefuzzy   Fetch by name via a fuzzy search
** @nam5rule Synonym     Fetch by a synonym
**
** @argrule * srsa [EnsPSeqregionsynonymadaptor]
** Ensembl Sequence Region Synonym Adaptor
** @argrule ByIdentifier identifier [ajuint] SQL database-internal identifier
** @argrule BySynonym synonym [const AjPStr] Synonym
** @argrule All srss [AjPList]
** AJAX List of Ensembl Sequence Region Synonym objects
** @argrule AllbySeqregion sr [const EnsPSeqregion] Ensembl Sequence Region
** @argrule Allby srss [AjPList]
** AJAX List of Ensembl Sequence Region Synonym objects
** @argrule By Psrs [EnsPSeqregionsynonym*]
** Ensembl Sequence Region Synonym address
**
** @valrule * [AjBool] ajTrue upon success, ajFalse otherwise
**
** @fcategory use
******************************************************************************/




/* @func ensSeqregionsynonymadaptorFetchAllbySeqregion ************************
**
** Fetch all Ensembl Sequence Region Synonym objects by an
** Ensembl Sequence Region.
** The caller is responsible for deleting the Ensembl Sequence Region Synonym
** objects before deleting the AJAX List.
**
** @cc Bio::EnsEMBL::DBSQL::SeqRegionSynonymAdaptor::get_synonyms
** @param [u] srsa [EnsPSeqregionsynonymadaptor]
** Ensembl Sequence Region Synonym Adaptor
** @param [r] sr [const EnsPSeqregion] Ensembl Sequence Region
** @param [u] srss [AjPList]
** AJAX List of Ensembl Sequence Region Synonym objects
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
** @@
******************************************************************************/

AjBool ensSeqregionsynonymadaptorFetchAllbySeqregion(
    EnsPSeqregionsynonymadaptor srsa,
    const EnsPSeqregion sr,
    AjPList srss)
{
    AjBool result = AJFALSE;

    AjPStr constraint = NULL;

    if(!srsa)
        return ajFalse;

    if(!sr)
        return ajFalse;

    if(!srss)
        return ajFalse;

    constraint = ajFmtStr("seq_region_synonym.seq_region_id = %u",
                          ensSeqregionGetIdentifier(sr));

    result = ensBaseadaptorFetchAllbyConstraint(
        srsa,
        constraint,
        (EnsPAssemblymapper) NULL,
        (EnsPSlice) NULL,
        srss);

    ajStrDel(&constraint);

    return result;
}




/* @func ensSeqregionsynonymadaptorFetchByIdentifier **************************
**
** Fetch an Ensembl Sequence Region Synonym by its
** SQL database-internal identifier.
** The caller is responsible for deleting the Ensembl Sequence Region Synonym.
**
** @cc Bio::EnsEMBL::DBSQL::SeqRegionSynonymAdaptor::fetch_by_dbID
** @param [u] srsa [EnsPSeqregionsynonymadaptor]
** Ensembl Sequence Region Synonym Adaptor
** @param [r] identifier [ajuint] SQL database-internal identifier
** @param [u] Psrs [EnsPSeqregionsynonym*]
** Ensembl Sequence Region Synonym address
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
** @@
******************************************************************************/

AjBool ensSeqregionsynonymadaptorFetchByIdentifier(
    EnsPSeqregionsynonymadaptor srsa,
    ajuint identifier,
    EnsPSeqregionsynonym* Psrs)
{
    if(!srsa)
        return ajFalse;

    if(!identifier)
        return ajFalse;

    if(!Psrs)
        return ajFalse;

    return ensBaseadaptorFetchByIdentifier(srsa, identifier, (void**) Psrs);
}




/* @func ensSeqregionsynonymadaptorFetchBySynonym *****************************
**
** Fetch an Ensembl Sequence Region Synonym by a synonym.
** The caller is responsible for deleting the Ensembl Sequence Region Synonym.
**
** @param [u] srsa [EnsPSeqregionsynonymadaptor]
** Ensembl Sequence Region Synonym Adaptor
** @param [r] synonym [const AjPStr] Synonym
** @param [u] Psrs [EnsPSeqregionsynonym*]
** Ensembl Sequence Region Synonym address
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
** @@
******************************************************************************/

AjBool ensSeqregionsynonymadaptorFetchBySynonym(
    EnsPSeqregionsynonymadaptor srsa,
    const AjPStr synonym,
    EnsPSeqregionsynonym* Psrs)
{
    char* txtsynonym = NULL;

    AjBool result = AJFALSE;

    AjPList srss = NULL;

    AjPStr constraint = NULL;

    EnsPSeqregionsynonym srs = NULL;

    if(!srsa)
        return ajFalse;

    if((synonym == NULL) || (ajStrGetLen(synonym) == 0))
        return ajFalse;

    if(!Psrs)
        return ajFalse;

    ensBaseadaptorEscapeC(srsa, &txtsynonym, synonym);

    constraint = ajFmtStr("seq_region_synonym.synonym = '%s'", txtsynonym);

    ajCharDel(&txtsynonym);

    srss = ajListNew();

    result = ensBaseadaptorFetchAllbyConstraint(
        srsa,
        constraint,
        (EnsPAssemblymapper) NULL,
        (EnsPSlice) NULL,
        srss);

    ajStrDel(&constraint);

    if(ajListGetLength(srss) > 1)
        ajDebug("ensSeqregionsynonymadaptorFetchBySynonym got more than one "
                "Ensembl Sequence Region Synonym for synonym '%S'.\n",
                synonym);

    /* Keep only the first Ensembl Sequence Region Synonym. */

    ajListPop(srss, (void**) Psrs);

    while(ajListPop(srss, (void**) &srs))
        ensSeqregionsynonymDel(&srs);

    ajListFree(&srss);

    return result;

}
