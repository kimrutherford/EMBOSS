/******************************************************************************
**
** @source Ensembl Sequence Region functions
**
** @author Copyright (C) 1999 Ensembl Developers
** @author Copyright (C) 2006 Michael K. Schuster
** @modified 2009 by Alan Bleasby for incorporation into EMBOSS core
** @version $Revision: 1.14 $
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

#include "ensseqregion.h"
#include "ensattribute.h"




/* ==================================================================== */
/* ========================== private data ============================ */
/* ==================================================================== */




/* ==================================================================== */
/* ======================== private functions ========================= */
/* ==================================================================== */

static void* seqregionadaptorCacheReference(void *value);

static void seqregionadaptorCacheDelete(void **value);

static ajulong seqregionadaptorCacheSize(const void* value);

static void seqregionadaptorCacheInsert(void** x, void* cl);

static AjBool seqregionadaptorCacheNonReferenceInit(EnsPSeqregionadaptor sra);

static AjBool seqregionadaptorCacheNonReferenceClear(EnsPSeqregionadaptor sra);

static void seqregionadaptorCacheNonReferenceDelete(void **key,
                                                    void **value,
                                                    void *cl);

static AjBool seqregionadaptorFetchAllBySQL(EnsPSeqregionadaptor sra,
                                            const AjPStr statement,
                                            AjPList srs);




/* @filesection ensseqregion **************************************************
**
** @nam1rule ens Function belongs to the Ensembl library
**
******************************************************************************/




/* @datasection [EnsPSeqregion] Sequence Region *******************************
**
** Functions for manipulating Ensembl Sequence Region objects
**
** @cc Bio::EnsEMBL::Slice CVS Revision: 1.255
**
** @nam2rule Seqregion
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
** @fnote None
**
** @nam3rule New Constructor
** @nam4rule NewObj Constructor with existing object
** @nam4rule NewRef Constructor by incrementing the reference counter
**
** @argrule Obj object [EnsPSeqregion] Ensembl Sequence Region
** @argrule Ref object [EnsPSeqregion] Ensembl Sequence Region
**
** @valrule * [EnsPSeqregion] Ensembl Sequence Region
**
** @fcategory new
******************************************************************************/




/* @func ensSeqregionNew ******************************************************
**
** Default constructor for an Ensembl Sequence Region.
**
** @cc Bio::EnsEMBL::Storable
** @param [rN] sra [EnsPSeqregionadaptor] Ensembl Sequence Region Adaptor
** @param [rE] identifier [ajuint] SQL database-internal identifier
** @cc Bio::EnsEMBL::???
** @param [r] cs [EnsPCoordsystem] Ensembl Coordinate System
** @param [r] name [AjPStr] Name
** @param [r] length [ajint] Length
**
** @return [EnsPSeqregion] Ensembl Sequence Region or NULL
** @@
******************************************************************************/

EnsPSeqregion ensSeqregionNew(EnsPSeqregionadaptor sra,
                              ajuint identifier,
                              EnsPCoordsystem cs,
                              AjPStr name,
                              ajint length)
{
    EnsPSeqregion sr = NULL;

    if(ajDebugTest("ensSeqregionNew"))
    {
        ajDebug("ensSeqregionNew\n"
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

    sr->Length = length;

    return sr;
}




/* @func ensSeqregionNewObj ***************************************************
**
** Object-based constructor function, which returns an independent object.
**
** @param [u] object [EnsPSeqregion] Ensembl Sequence Region
**
** @return [EnsPSeqregion] Ensembl Sequence Region or NULL
** @@
******************************************************************************/

EnsPSeqregion ensSeqregionNewObj(EnsPSeqregion object)
{
    AjIList iter = NULL;

    EnsPAttribute attribute = NULL;

    EnsPSeqregion sr = NULL;

    if(!object)
        return NULL;

    AJNEW0(sr);

    sr->Use = 1;

    sr->Identifier = object->Identifier;

    sr->Adaptor = object->Adaptor;

    sr->Coordsystem = ensCoordsystemNewRef(object->Coordsystem);

    if(object->Name)
        sr->Name = ajStrNewRef(object->Name);

    /* Copy the AJAX List of Ensembl Attributes */

    if(object->Attributes && ajListGetLength(object->Attributes))
    {
        sr->Attributes = ajListNew();

        iter = ajListIterNew(object->Attributes);

        while(!ajListIterDone(iter))
        {
            attribute = (EnsPAttribute) ajListIterGet(iter);

            ajListPushAppend(sr->Attributes, (void *) attribute);
        }

        ajListIterDel(&iter);
    }
    else
        sr->Attributes = NULL;

    sr->Length = object->Length;

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
** memory allocated for the Ensembl Sequence Region.
**
** @fdata [EnsPSeqregion]
** @fnote None
**
** @nam3rule Del Destroy (free) an Sequence Region object
**
** @argrule * Psr [EnsPSeqregion*] Sequence Region object address
**
** @valrule * [void]
**
** @fcategory delete
******************************************************************************/




/* @func ensSeqregionDel ******************************************************
**
** Default Ensembl Sequence Region destructor.
**
** @param [d] Psr [EnsPSeqregion*] Ensembl Sequence Region address
**
** @return [void]
** @@
******************************************************************************/

void ensSeqregionDel(EnsPSeqregion *Psr)
{
    EnsPAttribute attribute = NULL;
    EnsPSeqregion pthis = NULL;

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

    /* Clear and delete the AJAX List of Ensembl Attributes. */

    while(ajListPop(pthis->Attributes, (void **) &attribute))
        ensAttributeDel(&attribute);

    ajListFree(&pthis->Attributes);

    AJFREE(pthis);

    *Psr = NULL;

    return;
}




/* @section element retrieval *************************************************
**
** Functions for returning elements of an Ensembl Sequence Region object.
**
** @fdata [EnsPSeqregion]
** @fnote None
**
** @nam3rule Get Return Sequence Region attribute(s)
** @nam4rule GetAdaptor Return the Ensembl Sequence Region Adaptor
** @nam4rule GetIdentifier Return the SQL database-internal identifier
** @nam4rule GetName Return the name
** @nam4rule GetCoordsystem Return the Ensembl Coordinate System
** @nam4rule GetLength Return the length
** @nam4rule GetAttributes Get all Ensembl Attributes
**
** @argrule * sr [const EnsPSeqregion] Sequence Region
**
** @valrule Adaptor [EnsPSeqregionadaptor] Ensembl Sequence Region Adaptor
** @valrule Identifier [ajuint] SQL database-internal identifier
** @valrule Coordsystem [EnsPCoordsystem] Ensembl Coordinate System
** @valrule Name [AjPStr] Name
** @valrule Length [ajuint] Length
** @valrule Attributes [const AjPList] AJAX List of Ensembl Attributes
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
** @return [const EnsPSeqregionadaptor] Ensembl Sequence Region Adaptor
** @@
******************************************************************************/

const EnsPSeqregionadaptor ensSeqregionGetAdaptor(const EnsPSeqregion sr)
{
    if(!sr)
        return NULL;

    return sr->Adaptor;
}




/* @func ensSeqregionGetIdentifier ********************************************
**
** Get the SQL database-internal identifier element of an
** Ensembl Sequence Region.
**
** @param [r] sr [const EnsPSeqregion] Ensembl Sequence Region
**
** @return [ajuint] SQL database-internal identifier
** @@
******************************************************************************/

ajuint ensSeqregionGetIdentifier(const EnsPSeqregion sr)
{
    if(!sr)
        return 0;

    return sr->Identifier;
}




/* @func ensSeqregionGetCoordsystem *******************************************
**
** Get the Ensembl Coordinate System element of an Ensembl Sequence Region.
**
** @param [r] sr [const EnsPSeqregion] Ensembl Sequence Region
**
** @return [EnsPCoordsystem] Ensembl Coordinate System
** @@
******************************************************************************/

EnsPCoordsystem ensSeqregionGetCoordsystem(const EnsPSeqregion sr)
{
    if(!sr)
        return 0;

    return sr->Coordsystem;
}




/* @func ensSeqregionGetName **************************************************
**
** Get the name element of an Ensembl Sequence Region.
**
** @param [r] sr [const EnsPSeqregion] Ensembl Sequence Region
**
** @return [AjPStr] Name
** @@
******************************************************************************/

AjPStr ensSeqregionGetName(const EnsPSeqregion sr)
{
    if(!sr)
        return NULL;

    return sr->Name;
}




/* @func ensSeqregionGetLength ************************************************
**
** Get the length element of an Ensembl Sequence Region.
**
** @param [r] sr [const EnsPSeqregion] Ensembl Sequence Region
**
** @return [ajint] Length
** @@
******************************************************************************/

ajint ensSeqregionGetLength(const EnsPSeqregion sr)
{
    if(!sr)
        return 0;

    return sr->Length;
}




/* @func ensSeqregionGetAttributes ********************************************
**
** Get all Ensembl Attributes of an Ensembl Sequence Region.
**
** This is not a simple accessor function, it will fetch Ensembl Attributes
** from the Ensembl Core database in case the AJAX List is empty.
**
** @param [u] sr [EnsPSeqregion] Ensembl Sequence Region
**
** @return [const AjPList] AJAX List of Ensembl Attributes
** @@
******************************************************************************/

const AjPList ensSeqregionGetAttributes(EnsPSeqregion sr)
{
    EnsPAttributeadaptor ata = NULL;

    EnsPDatabaseadaptor dba = NULL;

    if(!sr)
        return NULL;

    if(sr->Attributes)
        return sr->Attributes;

    if(!sr->Adaptor)
    {
        ajDebug("ensSeqregionGetAttributes cannot fetch "
                "Ensembl Attributes for a Sequence Region without a "
                "Sequence Region Adaptor.\n");

        return NULL;
    }

    dba = ensSeqregionadaptorGetDatabaseadaptor(sr->Adaptor);

    if(!dba)
    {
        ajDebug("ensSeqregionGetAttributes cannot fetch "
                "Ensembl Attributes for a Sequence Region without a "
                "Database Adaptor set in the Sequence Region Adaptor.\n");

        return NULL;
    }

    ata = ensRegistryGetAttributeadaptor(dba);

    sr->Attributes = ajListNew();

    ensAttributeadaptorFetchAllBySeqregion(ata,
                                           sr,
                                           (const AjPStr) NULL,
                                           sr->Attributes);

    return sr->Attributes;
}




/* @section modifiers *********************************************************
**
** Functions for assigning elements of an Ensembl Sequence Region object.
**
** @fdata [EnsPSeqregion]
** @fnote None
**
** @nam3rule Set Set one element of a Sequence Region
** @nam4rule SetAdaptor Set the Ensembl Sequence Region Adaptor
** @nam4rule SetIdentifier Set the SQL database-internal identifier
** @nam4rule SetCoordsystem Set the Ensembl Coordinate System
** @nam4rule SetName Set the name
** @nam4rule SetLength Set the length
**
** @argrule * sr [EnsPSeqregion] Ensembl Sequence Region object
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
    if(!sra)
        return ajFalse;

    sr->Adaptor = sra;

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




/* @func ensSeqregionSetCoordsystem *****************************************
**
** Set the Ensembl Coordinate System element of an Ensembl Sequence Region.
**
** @param [u] sr [EnsPSeqregion] Ensembl Sequence Region
** @param [rN] cs [EnsPCoordsystem] Ensembl Coordinate System
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
                     (void *) ensAttributeNewRef(attribute));

    return ajTrue;
}




/* @section debugging *********************************************************
**
** Functions for reporting of an Ensembl Sequence Region object.
**
** @fdata [EnsPSeqregion]
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
            "%S  Length %u\n",
            indent, sr,
            indent, sr->Use,
            indent, sr->Identifier,
            indent, sr->Adaptor,
            indent, sr->Coordsystem,
            indent, sr->Name,
            indent, sr->Attributes,
            indent, sr->Length);

    ensCoordsystemTrace(sr->Coordsystem, level + 1);

    ajStrDel(&indent);

    return ajTrue;
}




/* @func ensSeqregionGetMemsize ***********************************************
**
** Get the memory size in bytes of an Ensembl Sequence Region.
**
** @param [r] sr [const EnsPSeqregion] Ensembl Sequence Region
**
** @return [ajulong] Memory size
** @@
******************************************************************************/

ajulong ensSeqregionGetMemsize(const EnsPSeqregion sr)
{
    ajulong size = 0;

    AjIList iter = NULL;

    EnsPAttribute attribute = NULL;

    if(!sr)
        return 0;

    size += sizeof (EnsOSeqregion);

    size += ensCoordsystemGetMemsize(sr->Coordsystem);

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

            size += ensAttributeGetMemsize(attribute);
        }

        ajListIterDel(&iter);
    }

    return size;
}




/* @func ensSeqregionMatch **************************************************
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




/* @func ensSeqregionFetchAllAttributes ***************************************
**
** Fetch all Ensembl Attributes of an Ensembl Sequence Region.
** The caller is responsible for deleting the Ensembl Attributes before
** deleting the AJAX List.
**
** @param [u] sr [EnsPSeqregion] Ensembl Sequence Region
** @param [rN] code [const AjPStr] Ensembl Attribute code
** @param [u] attributes [AjPList] AJAX List of Ensembl Attributes
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

    list = ensSeqregionGetAttributes(sr);

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
                             (void *) ensAttributeNewRef(attribute));
    }

    ajListIterDel(&iter);

    return ajTrue;
}




/* @func ensSeqregionIsNonReference *******************************************
**
** Test if an Ensembl Sequence Region has an Ensembl Attributes of code
** 'non_ref' set. This function calls ensSeqregionadaptorIsNonReference and
** uses the Ensembl Sequence Region Adaptor-internal cache of non-reference
** Sequence Region identifiers.
**
** @param [u] sr [EnsPSeqregion] Ensembl Sequence Region
** @param [u] Presult [AjBool*] ajTrue, if an Ensembl Attribute of code
**                              'non_ref' has been set
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
** @@
******************************************************************************/

AjBool ensSeqregionIsNonReference(EnsPSeqregion sr, AjBool *Presult)
{
    if(!sr)
        return ajFalse;

    if(!Presult)
        return ajFalse;

    return ensSeqregionadaptorIsNonReference(sr->Adaptor, sr, Presult);
}




/* @func ensSeqregionIsTopLevel ***********************************************
**
** Test if an Ensembl Sequence Region has an Ensembl Attributes of code
** 'toplevel' set. If Ensembl Attributes associated with this Ensembl Sequence
** Region are not already cached, they will be fetched from the database.
**
** @param [u] sr [EnsPSeqregion] Ensembl Sequence Region
** @param [u] Presult [AjBool*] ajTrue, if an Ensembl Attribute of code
**                              'toplevel' has been set
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
** @@
******************************************************************************/

AjBool ensSeqregionIsTopLevel(EnsPSeqregion sr, AjBool *Presult)
{
    AjIList iter       = NULL;
    const AjPList list = NULL;

    EnsPAttribute attribute = NULL;

    if(!sr)
        return ajFalse;

    if(!Presult)
        return ajFalse;

    list = ensSeqregionGetAttributes(sr);

    iter = ajListIterNewread(list);

    while(!ajListIterDone(iter))
    {
        attribute = ajListIterGet(iter);

        if(ajStrMatchC(ensAttributeGetCode(attribute), "toplevel"))
        {
            *Presult = ajTrue;

            break;
        }
    }

    ajListIterDel(&iter);

    return ajTrue;
}




/* @datasection [EnsPSeqregionadaptor] Sequence Region Adaptor ****************
**
** Functions for manipulating Ensembl Sequence Region Adaptor objects
**
** @cc Bio::EnsEMBL::DBSQL::Sliceadaptor CVS Revision: 1.107
**
** @nam2rule Seqregionadaptor
**
******************************************************************************/

/* 1 << 26 = 64 MiB */

static ajuint seqregionadaptorCacheMaxBytes = 1 << 26;

/* 1 << 16 = 64 Ki  */

static ajuint seqregionadaptorCacheMaxCount = 1 << 16;

static ajuint seqregionadaptorCacheMaxSize = 0;




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

static void* seqregionadaptorCacheReference(void *value)
{
    if(!value)
        return NULL;

    return (void *) ensSeqregionNewRef((EnsPSeqregion) value);
}




/* @funcstatic seqregionadaptorCacheDelete ************************************
**
** Wrapper function to delete (or de-reference) an Ensembl Sequence Region
** from an Ensembl Cache.
**
** @param [u] value [void**] Ensembl Sequence Region address
**
** @return [void]
** @@
** When deleting from the Cache, this function also removes and deletes the
** Ensembl Sequence Region from the name cache, which is based on a
** conventional AJAX Table.
******************************************************************************/

static void seqregionadaptorCacheDelete(void **value)
{
    AjPStr newkey = NULL;
    AjPStr oldkey = NULL;

    EnsPSeqregion oldsr = NULL;
    EnsPSeqregion *Psr  = NULL;

    if(!value)
        return;

    /*
    ** Synchronise the deletion of this Sequence Region from the
    ** identifier cache, which is based on an Ensembl (LRU) Cache,
    ** with the name cache, based on a conventional AJAX Table,
    ** both in the Sequence Adaptor.
    */

    Psr = (EnsPSeqregion *) value;

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
                             (const void *) newkey,
                             (void **) &oldkey);

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
** @return [ajulong] Memory size
** @@
******************************************************************************/

static ajulong seqregionadaptorCacheSize(const void* value)
{
    if(!value)
        return 0;

    return ensSeqregionGetMemsize((const EnsPSeqregion) value);
}




/* @funcstatic seqregionadaptorCacheNonReferenceInit **************************
**
** Initialises an Ensembl Sequence Region Adaptor-internal AJAX Table of
** non-reference (i.e. haplotype) Ensembl Sequence Region identifier key and
** AJAX Boolean value data.
**
** @param [u] sra [EnsPSeqregionadaptor] Ensembl Sequence Region Adaptor
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
** @@
******************************************************************************/

static AjBool seqregionadaptorCacheNonReferenceInit(EnsPSeqregionadaptor sra)
{
    ajuint *Pidentifier = NULL;

    AjBool *Pflag = NULL;

    AjPList srs = NULL;

    AjPStr code = NULL;

    EnsPSeqregion sr = NULL;

    if(!sra)
        return ajFalse;

    if(!sra->CacheNonReference)
        sra->CacheNonReference = MENSTABLEUINTNEW(0);

    code = ajStrNewC("non_ref");

    srs = ajListNew();

    ensSeqregionadaptorFetchAllByAttributeCodeValue(sra,
                                                    code,
                                                    (AjPStr) NULL,
                                                    srs);

    while(ajListPop(srs, (void **) &sr))
    {
        AJNEW0(Pidentifier);

        *Pidentifier = ensSeqregionGetIdentifier(sr);

        AJNEW0(Pflag);

        *Pflag = ajTrue;

        ajTablePut(sra->CacheNonReference,
                   (void *) Pidentifier,
                   (void *) Pflag);

        ensSeqregionDel(&sr);
    }

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
** @fnote None
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
** @param [r] dba [EnsPDatabaseadaptor] Ensembl Database Adaptor
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

    sra->CacheByName = ajTablestrNewLen(0);

    return sra;
}




/* @funcstatic seqregionadaptorCacheNonReferenceClear *************************
**
** Clears the Sequence Region Adaptor-internal AJAX Table of non-reference
** (i.e. haplotype) Ensembl Sequence Regions.
**
** @param [u] sra [EnsPSeqregionadaptor] Ensembl Sequence Region Adaptor
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
** @@
******************************************************************************/

static AjBool seqregionadaptorCacheNonReferenceClear(EnsPSeqregionadaptor sra)
{
    if(!sra)
        return ajFalse;

    ajTableMapDel(sra->CacheNonReference,
                  seqregionadaptorCacheNonReferenceDelete,
                  NULL);

    return ajTrue;
}




/* @funcstatic seqregionadaptorCacheNonReferenceDelete ************************
**
** An ajTableMapDel 'apply' function to clear the Ensembl Sequence Region
** Adaptor-internal AJAX Table of non-reference Sequence Region identifiers.
** This function deletes the unsigned integer Ensembl Sequence Region
** identifier key and the AJAX Boolean value data.
**
** @param [u] key [void**] AJAX unsigned integer key data address
** @param [u] value [void**] AJAX Boolean value data address
** @param [u] cl [void*] Standard, passed in from ajTableMapDel
** @see ajTableMapDel
**
** @return [void]
** @@
******************************************************************************/

static void seqregionadaptorCacheNonReferenceDelete(void **key,
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
    AJFREE(*value);

    return;
}




/* @section destructors *******************************************************
**
** Destruction destroys all internal data structures and frees the
** memory allocated for the Ensembl Sequence Region Adaptor.
**
** @fdata [EnsPSeqregionadaptor]
** @fnote None
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
**                                         address
**
** @return [void]
** @@
******************************************************************************/

void ensSeqregionadaptorDel(EnsPSeqregionadaptor *Psra)
{
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
    ** seqregionCacheDelete.
    */

    ensCacheDel(&pthis->CacheByIdentifier);

    ajTableFree(&pthis->CacheByName);

    /* Clear the cache of non-refrence Ensembl Sequence Region identifiers. */

    seqregionadaptorCacheNonReferenceClear(pthis);

    ajTableFree(&pthis->CacheNonReference);

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
** @fnote None
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




/* @func ensSeqregionadaptorCacheInsert ***************************************
**
** Insert an Ensembl Sequence Region into the Sequence Region Adaptor-internal
** cache.
**
** @param [u] sra [EnsPSeqregionadaptor] Ensembl Sequence Region Adaptor
** @param [wP] Psr [EnsPSeqregion *] Ensembl Sequence Region address
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
** @@
******************************************************************************/

AjBool ensSeqregionadaptorCacheInsert(EnsPSeqregionadaptor sra,
                                      EnsPSeqregion *Psr)
{
    ajuint *Pid = NULL;

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

    sr1 = (EnsPSeqregion) ensCacheFetch(sra->CacheByIdentifier, (void *) Pid);

    /* Search the name cache. */

    key = ajFmtStr("%u:%S",
                   ensCoordsystemGetIdentifier((*Psr)->Coordsystem),
                   (*Psr)->Name);

    sr2 = (EnsPSeqregion) ajTableFetch(sra->CacheByName, (const void *) key);

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
                      (void *) Pid,
                      (void **) Psr);

        ajTablePut(sra->CacheByName,
                   (void *) ajStrNewS(key),
                   (void *) ensSeqregionNewRef(*Psr));
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
** An ajListMap 'apply' function to insert Ensembl Sequence Regions into the
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
                                   (EnsPSeqregion *) x);

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

    ensCacheRemove(sra->CacheByIdentifier, (const void *) &sr->Identifier);

    return ajTrue;
}




/* @section object retrieval **************************************************
**
** Functions for retrieving Ensembl Sequence Region objects from an
** Ensembl Core database.
**
** @fdata [EnsPSeqregionadaptor]
** @fnote None
**
** @nam3rule Fetch Retrieve Ensembl Sequence Region object(s)
** @nam4rule FetchAll Retrieve all Ensembl Sequence Region objects
** @nam5rule FetchAllBy Retrieve all Ensembl Sequence Region objects
**                      matching a criterion
** @nam4rule FetchBy Retrieve one Ensembl Sequence Region object
**                   matching a criterion
**
** @argrule * sra [EnsPSeqregionadaptor] Ensembl Sequence Region Adaptor
** @argrule FetchAll [AjPList] AJAX List of Ensembl Sequence Region objects
**
** @valrule * [AjBool] ajTrue upon success, ajFalse otherwise
**
** @fcategory use
******************************************************************************/




/* @funcstatic seqregionadaptorFetchAllBySQL **********************************
**
** Run a SQL statement against an Ensembl Database Adaptor and consolidate the
** results into an AJAX List of Ensembl Sequence Regions.
** The caller is responsible for deleting the Ensembl Sequence Regions before
** deleting the AJAX List.
**
** @param [r] sra [EnsPSeqregionadaptor] Ensembl Sequence Region Adaptor
** @param [r] statement [const AjPStr] SQL statement
** @param [u] srs [AjPList] AJAX List of Ensembl Sequence Regions
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
** @@
******************************************************************************/

static AjBool seqregionadaptorFetchAllBySQL(EnsPSeqregionadaptor sra,
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
            ajDebug("seqregionadaptorFetchAllBySQL got an "
                    "Ensembl Sequence Region with identifier %u that "
                    "references a non-existent Ensembl Coordinate System "
                    "with identifier %u.", srid, csid);

            ajWarn("seqregionadaptorFetchAllBySQL got an "
                   "Ensembl Sequence Region with identifier %u that "
                   "references a non-existent Ensembl Coordinate System "
                   "with identifier %u.", srid, csid);
        }

        sr = ensSeqregionNew(sra, srid, cs, name, length);

        ajListPushAppend(srs, (void *) sr);

        ensCoordsystemDel(&cs);

        ajStrDel(&name);
    }

    ajSqlrowiterDel(&sqli);

    ensDatabaseadaptorSqlstatementDel(sra->Adaptor, &sqls);

    return ajTrue;
}




/* @func ensSeqregionadaptorFetchByIdentifier *********************************
**
** Fetch an Ensembl Sequence Region by its SQL database-internal identifier.
** The caller is responsible for deleting the Ensembl Sequence Region.
**
** @param [u] sra [EnsPSeqregionadaptor] Ensembl Sequence Region Adaptor
** @param [r] identifier [ajuint] SQL database-internal identifier
** @param [wP] Psr [EnsPSeqregion *] Ensembl Sequence Region address
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
** @@
******************************************************************************/

AjBool ensSeqregionadaptorFetchByIdentifier(EnsPSeqregionadaptor sra,
                                            ajuint identifier,
                                            EnsPSeqregion *Psr)
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

    *Psr = (EnsPSeqregion) ensCacheFetch(sra->CacheByIdentifier,
                                         (void *) &identifier);

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

    seqregionadaptorFetchAllBySQL(sra, statement, srs);

    if(ajListGetLength(srs) == 0)
        ajDebug("ensSeqregionadaptorFetchByIdentifier got no "
                "Ensembl Sequence Region for identifier %u.\n",
                identifier);
    else if(ajListGetLength(srs) == 1)
    {
        ajListPop(srs, (void **) Psr);

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

        while(ajListPop(srs, (void **) Psr))
            ensSeqregionDel(Psr);

        Psr = (EnsPSeqregion *) NULL;
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
** @param [wP] Psr [EnsPSeqregion *] Ensembl Sequence Region address
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
** @@
******************************************************************************/

AjBool ensSeqregionadaptorFetchByName(EnsPSeqregionadaptor sra,
                                      const EnsPCoordsystem cs,
                                      const AjPStr name,
                                      EnsPSeqregion *Psr)
{
    char *txtname = NULL;

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
    ** Initally, search the name cache, which can only return a Sequence Region
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

    *Psr = (EnsPSeqregion) ajTableFetch(sra->CacheByName, (const void *) key);

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

    if((!cs) || ensCoordsystemIsTopLevel(cs))
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

    seqregionadaptorFetchAllBySQL(sra, statement, srs);

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

    ajListPop(srs, (void **) Psr);

    ensSeqregionadaptorCacheInsert(sra, Psr);

    while(ajListPop(srs, (void **) &sr))
    {
        ensSeqregionadaptorCacheInsert(sra, &sr);

        ensSeqregionDel(&sr);
    }

    ajListFree(&srs);

    return ajTrue;
}




/* @func ensSeqregionadaptorFetchByNameFuzzy **********************************
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
** @param [wP] Psr [EnsPSeqregion *] Ensembl Sequence Region address
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
** @@
******************************************************************************/

AjBool ensSeqregionadaptorFetchByNameFuzzy(EnsPSeqregionadaptor sra,
                                           const EnsPCoordsystem cs,
                                           const AjPStr name,
                                           EnsPSeqregion *Psr)
{
    char *txtname = NULL;

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

    if(ajDebugTest("ensSeqregionadaptorFetchByNameFuzzy"))
        ajDebug("ensSeqregionadaptorFetchByNameFuzzy\n"
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
        ajDebug("ensSeqregionadaptorFetchByNameFuzzy requires a "
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

    if((!cs) || ensCoordsystemIsTopLevel(cs))
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

    seqregionadaptorFetchAllBySQL(sra, statement, srs);

    ajStrDel(&statement);

    reslen = ajListGetLength(srs);

    complete = ajListNew();

    partial = ajListNew();

    while(ajListPop(srs, (void **) &sr))
    {
        /*
        ** Add all Ensembl Sequence Regions into the
        ** Ensembl Sequence Region Adaptor-internal cache.
        */

        ensSeqregionadaptorCacheInsert(sra, &sr);

        /* Prioritise Sequence Regions which names match completely. */

        if(ajStrMatchS(ensSeqregionGetName(sr), name))
            ajListPushAppend(complete, (void *) sr);
        else if(ajStrPrefixS(ensSeqregionGetName(sr), name))
            ajListPushAppend(partial, (void *) sr);
        else
        {
            ajDebug("ensSeqregionadaptorFetchByNameFuzzy got a "
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
        ajListPop(complete, (void **) Psr);
    else
    {
        tmpstr = ajStrNew();

        tmpverstr = ajStrNew();

        expression = ajRegCompC("^\\.([0-9]+)$");

        while(ajListPop(partial, (void **) &sr))
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
                    ajStrToInt(tmpverstr, (ajint *) &tmpvernum);

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

    while(ajListPop(complete, (void **) &sr))
        ensSeqregionDel(&sr);

    ajListFree(&complete);

    while(ajListPop(partial, (void **) &sr))
        ensSeqregionDel(&sr);

    ajListFree(&partial);

    if(reslen > 1)
        ajWarn("ensSeqregionadaptorFetchByNameFuzzy returned more than one "
               "Ensembl Sequence Region. "
               "You might want to check whether the returned "
               "Ensembl Sequence Region '%S' is the one you intended to "
               "fetch '%S'.\n", ensSeqregionGetName(*Psr), name);

    return ajTrue;
}




/* @func ensSeqregionadaptorFetchAllByCoordsystem *****************************
**
** Fetch all Ensembl Sequence Regions via an Ensembl Coordinate System.
** The caller is responsible for deleting the Ensembl Sequence Regions before
** deleting the AJAX List.
**
** @param [u] sra [EnsPSeqregionadaptor] Ensembl Sequence Region Adaptor
** @param [r] cs [const EnsPCoordsystem] Ensembl Coordinate System
** @param [u] srs [AjPList] AJAX List of Ensembl Sequence Regions
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
** @@
******************************************************************************/

AjBool ensSeqregionadaptorFetchAllByCoordsystem(EnsPSeqregionadaptor sra,
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

    if(ensCoordsystemIsTopLevel(cs))
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

    seqregionadaptorFetchAllBySQL(sra, statement, srs);

    ajStrDel(&statement);

    /* Insert all Sequence Regions into the adaptor-internal cache. */

    ajListMap(srs, seqregionadaptorCacheInsert, (void *) sra);

    return ajTrue;
}




/* @func ensSeqregionadaptorFetchAllByAttributeCodeValue **********************
**
** Fetch all Ensembl Sequence Regions via an Ensembl Attribute code and
** optional value.
**
** The caller is responsible for deleting the Ensembl Sequence Regions before
** deleting the AJAX List.
**
** @param [u] sra [EnsPSeqregionadaptor] Ensembl Sequence Region Adaptor
** @param [r] code [const AjPStr] Ensembl Attribute code
** @param [rN] value [const AjPStr] Ensembl Sequence Region Attribute value
** @param [u] srs [AjPList] AJAX List of Ensembl Sequence Regions
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
** @@
******************************************************************************/

AjBool ensSeqregionadaptorFetchAllByAttributeCodeValue(
    EnsPSeqregionadaptor sra,
    const AjPStr code,
    const AjPStr value,
    AjPList srs)
{
    char *txtcode = NULL;
    char *txtvalue = NULL;

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
        "attribute_type.code = '%s' "
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

    seqregionadaptorFetchAllBySQL(sra, statement, srs);

    ajStrDel(&statement);

    return ajTrue;
}




/* @func ensSeqregionadaptorIsNonReference ************************************
**
** Check if a particular Ensembl Sequence Region is associated with an
** Ensembl Attribute of code 'non_ref', which is set for haplotype assembly
** paths.
** This function uses an Ensembl Sequence Region Adaptor-internal cache.
**
** @param [u] sra [EnsPSeqregionadaptor] Ensembl Sequence Region Adaptor
** @param [r] sr [const EnsPSeqregion] Ensembl Sequence Region
** @param [u] Presult [AjBool*] ajTrue:  This Sequence region has the
**                                       'non_ref' attribute set.
**                              ajFalse: This Sequence Region is part of the
**                                       reference sequence.
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
** @@
******************************************************************************/

AjBool ensSeqregionadaptorIsNonReference(EnsPSeqregionadaptor sra,
                                         const EnsPSeqregion sr,
                                         AjBool *Presult)
{
    ajuint identifier = 0;

    if(!sra)
        return ajFalse;

    if(!sr)
        return ajFalse;

    if(!Presult)
        return ajFalse;

    if(!sra->CacheNonReference)
        seqregionadaptorCacheNonReferenceInit(sra);

    identifier = ensSeqregionGetIdentifier(sr);

    if(ajTableFetch(sra->CacheNonReference, (const void *) &identifier))
        *Presult = ajFalse;
    else
        *Presult = ajTrue;

    return ajTrue;
}
