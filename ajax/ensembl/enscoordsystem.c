/******************************************************************************
**
** @source Ensembl Coordinate System functions
**
** @author Copyright (C) 1999 Ensembl Developers
** @author Copyright (C) 2006 Michael K. Schuster
** @modified 2009 by Alan Bleasby for incorporation into EMBOSS core
** @version $Revision: 1.6 $
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

#include "enscoordsystem.h"
#include "ensmetainformation.h"




/* ==================================================================== */
/* ========================== private data ============================ */
/* ==================================================================== */




/* ==================================================================== */
/* ======================== private functions ========================= */
/* ==================================================================== */

extern EnsPMetainformationadaptor
ensRegistryGetMetainformationadaptor(EnsPDatabaseadaptor dba);

static AjBool coordSystemadaptorFetchAllBySQL(EnsPCoordsystemadaptor adaptor,
                                              const AjPStr statement,
                                              AjPList cslist);

static AjBool coordSystemadaptorCacheInit(EnsPCoordsystemadaptor adaptor);

static AjBool coordSystemadaptorMapPathInit(EnsPCoordsystemadaptor adaptor);

static AjBool coordSystemadaptorSeqregionMapInit(
    EnsPCoordsystemadaptor adaptor);

static void coordSystemadaptorClearIdentifierCache(void **key,
                                                   void **value,
                                                   void *cl);

static void coordSystemadaptorClearNameCacheL2(void **key,
                                               void **value,
                                               void *cl);

static void coordSystemadaptorClearNameCacheL1(void **key,
                                               void **value,
                                               void *cl);

static AjBool coordSystemadaptorCacheExit(EnsPCoordsystemadaptor adaptor);

static void coordSystemadaptorClearMapPath(void **key,
                                           void **value,
                                           void *cl);

static AjBool coordSystemadaptorMapPathExit(EnsPCoordsystemadaptor adaptor);

static void coordSystemadaptorClearSeqregionMap(void **key,
                                                void **value,
                                                void *cl);

static AjBool coordSystemadaptorSeqregionMapExit(
    EnsPCoordsystemadaptor adaptor);

static void coordSystemadaptorFetchAll(const void *key,
                                       void **value,
                                       void *cl);




/* @filesection enscoordsystem ************************************************
**
** @nam1rule ens Function belongs to the Ensembl library.
**
******************************************************************************/




/* @datasection [EnsPCoordsystem] Coordinate System ***************************
**
** Functions for manipulating Ensembl Coordinate System objects
**
** @cc Bio::EnsEMBL::Coordsystem CVS Revision: 1.7
**
** @nam2rule Coordsystem
**
******************************************************************************/




/* @section constructors ******************************************************
**
** All constructors return a new Ensembl Coordinate System by pointer.
** It is the responsibility of the user to first destroy any previous
** Coordinate System. The target pointer does not need to be initialised to
** NULL, but it is good programming practice to do so anyway.
**
** @fdata [EnsPCoordsystem]
** @fcategory new
**
** @nam3rule New Constructor
** @nam4rule NewObj Constructor with existing object
** @nam4rule NewRef Constructor by incrementing the use counter
**
** @argrule Obj object [EnsPCoordsystem] Ensembl Coordinate System
** @argrule Ref object [EnsPCoordsystem] Ensembl Coordinate System
**
** @valrule * [EnsPCoordsystem] Ensembl Coordinate System
**
******************************************************************************/




/* @func ensCoordsystemNew ****************************************************
**
** Default Ensembl Coordinate System constructor.
**
** @cc Bio::EnsEMBL::Storable::new
** @param [r] adaptor [EnsPCoordsystemadaptor] Ensembl Coordinate
**                                             System Adaptor
** @param [r] identifier [ajuint] SQL database-internal identifier
** @cc Bio::EnsEMBL::Coordsystem::new
** @param [u] name [AjPStr] Name
** @param [uN] version [AjPStr] Version
** @param [r] rank [ajuint] Rank
** @param [r] dflt [AjBool] Default attribute
** @param [r] toplevel [AjBool] Top-level attrbute
** @param [r] seqlevel [AjBool] Sequence-level attribute
**
** @return [EnsPCoordsystem] Ensembl Coordinate System or NULL
** @@
** Many Ensembl Coordinate Systems do not have a concept of a version for the
** entire Coordinate System, although they may have a per-sequence version.
** The 'chromosome' Coordinate System usually has a version - i.e. the
** assembly build version - but the clonal Coordinate System does not, despite
** having individual sequence versions. In the case where a Coordinate System
** does not have a version an empty string is used instead.
**
** The highest-level Coordinate System (e.g. chromosome) should have rank 1,
** the second-highest level Coordinate System (e.g. clone) should have rank 2
** and so on.
**
** Top-level should only be set for creating an artificial top-level Coordinate
** System of name 'toplevel'.
**
** FIXME: There is a slight inconsistency in the Perl API.
** The Bio::EnsEMBL::Coordsystem::new method throws immediately if a name
** is missing, but would later set it automatically for top-level
** Coordinate Systems in case it was not provided in the first place.
**
** Bio::EnsEMBL::DBSQL::Coordsystemadaptor has a _fetch_by_attrib and a
** _fetch_by_attrib method, which is not used in the same Perl module.
******************************************************************************/

EnsPCoordsystem ensCoordsystemNew(
    EnsPCoordsystemadaptor adaptor,
    ajuint identifier,
    AjPStr name,
    AjPStr version,
    ajuint rank,
    AjBool dflt,
    AjBool toplevel,
    AjBool seqlevel)
{
    EnsPCoordsystem cs = NULL;

    if(ajDebugTest("ensCoordsystemNew")) 
        ajDebug("ensCoordsystemNew\n"
                "  adaptor %p\n"
                "  identifier %u\n"
                "  name '%S'\n"
                "  version '%S'\n"
                "  rank %u\n"
                "  dflt %B\n"
                "  toplevel %B\n"
                "  seqlevel %B\n",
                adaptor,
                identifier,
                name,
                version,
                rank,
                dflt,
                toplevel,
                seqlevel);

    if(toplevel)
    {
        if(name && ajStrGetLen(name))
        {
            if(!ajStrMatchCaseC(name, "toplevel"))
            {
                ajWarn("ensCoordsystemNew name parameter must be 'toplevel' "
                       "if the top-level parameter is set.\n");

                return NULL;
            }
        }

        if(rank)
        {
            ajWarn("ensCoordsystemNew rank parameter must be 0 "
                   "if the top-level parameter is set.\n");

            return NULL;
        }

        if(seqlevel)
        {
            ajWarn("ensCoordsystemNew sequence-level parameter must not "
                   "be set if the top-level parameter is set.\n");

            return NULL;
        }

        if(dflt)
        {
            ajWarn("ensCoordsystemNew default parameter must not be set "
                   "if the top-level parameter is set.\n");

            return NULL;
        }
    }
    else
    {
        if(name && ajStrGetLen(name))
        {
            if(ajStrMatchCaseC(name, "toplevel"))
            {
                ajWarn("ensCoordsystemNew name parameter cannot be "
                       "'toplevel' for non-top-level Coordinate Systems.\n");

                return NULL;
            }
        }
        else
        {
            ajWarn("ensCoordsystemNew name parameter must be provided for "
                   "non-top-level Coordinate Systems.\n");

            return NULL;
        }

        if(!rank)
        {
            ajWarn("ensCoordsystemNew rank parameter must be non-zero "
                   "for non-top-level Coordinate Systems.\n");

            return NULL;
        }
    }

    AJNEW0(cs);

    cs->Use = 1;

    cs->Identifier = identifier;

    cs->Adaptor = adaptor;

    if(name && (!toplevel))
        cs->Name = ajStrNewRef(name);

    if(toplevel)
        cs->Name = ajStrNewC("toplevel");

    if(version)
        cs->Version = ajStrNewRef(version);

    cs->Rank = rank;

    cs->SequenceLevel = seqlevel;

    cs->TopLevel = toplevel;

    cs->Default = dflt;

    return cs;
}




/* @func ensCoordsystemNewObj *************************************************
**
** Object-based constructor function, which returns an independent object.
**
** @param [u] object [EnsPCoordsystem] Ensembl Coordinate System
**
** @return [EnsPCoordsystem] Ensembl Coordinate System or NULL
** @@
******************************************************************************/

EnsPCoordsystem ensCoordsystemNewObj(EnsPCoordsystem object)
{
    EnsPCoordsystem cs = NULL;

    if(!object)
        return NULL;

    AJNEW0(cs);

    cs->Use = 1;

    cs->Identifier = object->Identifier;

    cs->Adaptor = object->Adaptor;

    if(object->Name)
        cs->Name = ajStrNewRef(object->Name);

    if(object->Version)
        cs->Version = ajStrNewRef(object->Version);

    cs->Default = object->Default;

    cs->SequenceLevel = object->SequenceLevel;

    cs->TopLevel = object->TopLevel;

    cs->Rank = object->Rank;

    return cs;
}




/* @func ensCoordsystemNewRef *************************************************
**
** Ensembl Object referencing function, which returns a pointer to the
** Ensembl Object passed in and increases its reference count.
**
** @param [u] cs [EnsPCoordsystem] Ensembl Coordinate System
**
** @return [EnsPCoordsystem] Ensembl Coordinate System or NULL
** @@
******************************************************************************/

EnsPCoordsystem ensCoordsystemNewRef(EnsPCoordsystem cs)
{
    if(ajDebugTest("ensCoordsystemNewRef"))
    {
        ajDebug("ensCoordsystemNewRef\n"
                "  cs %p\n",
                cs);

        ensCoordsystemTrace(cs, 1);
    }

    if(!cs)
        return NULL;

    cs->Use++;

    return cs;
}




/* @section destructors *******************************************************
**
** Destruction destroys all internal data structures and frees the
** memory allocated for the Ensembl Coordinate System.
**
** @fdata [EnsPCoordsystem]
** @fcategory delete
**
** @nam3rule Del Destroy (free) an Coordinate System object
**
** @argrule * Pcs [EnsPCoordsystem*] Coordinate System object address
**
** @valrule * [void]
**
******************************************************************************/




/* @func ensCoordsystemDel ****************************************************
**
** Default Ensembl Coordinate System destructor.
**
** @param [d] Pcs [EnsPCoordsystem*] Ensembl Coordinate System address
**
** @return [void]
** @@
******************************************************************************/

void ensCoordsystemDel(EnsPCoordsystem *Pcs)
{
    EnsPCoordsystem pthis = NULL;

    if(!Pcs)
        return;

    if(!*Pcs)
        return;

    if(ajDebugTest("ensCoordsystemDel"))
    {
        ajDebug("ensCoordsystemDel\n"
                "  *Pcs %p\n",
                *Pcs);

        ensCoordsystemTrace(*Pcs, 1);
    }

    pthis = *Pcs;

    pthis->Use--;

    if(pthis->Use)
    {
        *Pcs = NULL;

        return;
    }

    ajStrDel(&pthis->Name);

    ajStrDel(&pthis->Version);

    AJFREE(pthis);

    *Pcs = NULL;

    return;
}




/* @section element retrieval *************************************************
**
** Functions for returning elements of an Ensembl Coordinate System object.
**
** @fdata [EnsPCoordsystem]
** @fcategory use
**
** @nam3rule Get Return Coordinate System attribute(s)
** @nam4rule GetAdaptor Return the Ensembl Coordinate System Adaptor
** @nam4rule GetIdentifier Return the SQL database-internal identifier
** @nam4rule GetName Return the name
** @nam4rule GetVersion Return the version
** @nam4rule GetDefault Return the default attribute
** @nam4rule GetSeqLevel Return the sequence-level attribute
** @nam4rule GetTopLevel Return the top-level attribute
** @nam4rule GetRank Return the rank
**
** @argrule * cs [const EnsPCoordsystem] Coordinate System
**
** @valrule Adaptor [EnsPCoordsystemadaptor] Ensembl Coordinate System Adaptor
** @valrule Identifier [ajuint] SQL database-internal identifier
** @valrule Name [AjPStr] Name
** @valrule Version [AjPStr] Version
** @valrule Default [AjBool] Default attribute
** @valrule SeqLevel [AjBool] Sequence-level attribute
** @valrule TopLevel [AjBool] Top-level attribute
** @valrule Rank [ajuint] Rank
******************************************************************************/




/* @func ensCoordsystemGetAdaptor *********************************************
**
** Get the Ensembl Coordinate System Adaptor element of an
** Ensembl Coordinate System.
**
** @param [r] cs [const EnsPCoordsystem] Ensembl Coordinate System
**
** @return [EnsPCoordsystemadaptor] Ensembl Coordinate System Adaptor
** @@
******************************************************************************/

EnsPCoordsystemadaptor ensCoordsystemGetAdaptor(const EnsPCoordsystem cs)
{
    if(!cs)
        return NULL;

    return cs->Adaptor;
}




/* @func ensCoordsystemGetIdentifier ******************************************
**
** Get the SQL database-internal identifier element of an
** Ensembl Coordinate System.
**
** @param [r] cs [const EnsPCoordsystem] Ensembl Coordinate System
**
** @return [ajuint] SQL database-internal identifier
** @@
******************************************************************************/

ajuint ensCoordsystemGetIdentifier(const EnsPCoordsystem cs)
{
    if(!cs)
        return 0;

    return cs->Identifier;
}




/* @func ensCoordsystemGetName ************************************************
**
** Get the name element of an Ensembl Coordinate System.
**
** @param [r] cs [const EnsPCoordsystem] Ensembl Coordinate System
**
** @return [const AjPStr] Name
** @@
******************************************************************************/

const AjPStr ensCoordsystemGetName(const EnsPCoordsystem cs)
{
    if(!cs)
        return NULL;

    return cs->Name;
}




/* @func ensCoordsystemGetVersion *********************************************
**
** Get the version element of an Ensembl Coordinate System.
**
** @param [r] cs [const EnsPCoordsystem] Ensembl Coordinate System
**
** @return [const AjPStr] Version
** @@
******************************************************************************/

const AjPStr ensCoordsystemGetVersion(const EnsPCoordsystem cs)
{
    if(!cs)
        return NULL;

    return cs->Version;
}




/* @func ensCoordsystemGetDefault *********************************************
**
** Get the default element of an Ensembl Coordinate System.
**
** @param [r] cs [const EnsPCoordsystem] Ensembl Coordinate System
**
** @return [AjBool] ajTrue if the Coordinate System version defines the
**                  default of all Coordinate Systems with the same name.
** @@
******************************************************************************/

AjBool ensCoordsystemGetDefault(const EnsPCoordsystem cs)
{
    if(!cs)
        return ajFalse;

    return cs->Default;
}




/* @func ensCoordsystemGetSeqLevel ********************************************
**
** Get the sequence-level element of an Ensembl Coordinate System.
**
** @param [r] cs [const EnsPCoordsystem] Ensembl Coordinate System
**
** @return [AjBool] ajTrue if the Coordinate System defines the sequence-level
** @@
******************************************************************************/

AjBool ensCoordsystemGetSeqLevel(const EnsPCoordsystem cs)
{
    if(!cs)
        return ajFalse;

    return cs->SequenceLevel;
}




/* @func ensCoordsystemGetTopLevel ********************************************
**
** Get the top-level element of an Ensembl Coordinate System.
**
** @param [r] cs [const EnsPCoordsystem] Ensembl Coordinate System
**
** @return [AjBool] ajTrue if the coordinate system defines the top-level
** @@
******************************************************************************/

AjBool ensCoordsystemGetTopLevel(const EnsPCoordsystem cs)
{
    if(!cs)
        return ajFalse;

    return cs->TopLevel;
}




/* @func ensCoordsystemGetRank ************************************************
**
** Get the rank element of an Ensembl Coordinate System.
**
** @param [r] cs [const EnsPCoordsystem] Ensembl Coordinate System
**
** @return [ajuint] Rank
** @@
******************************************************************************/

ajuint ensCoordsystemGetRank(const EnsPCoordsystem cs)
{
    if(!cs)
        return 0;

    return cs->Rank;
}




/* @section element assignment ************************************************
**
** Functions for assigning elements of an Ensembl Coordinate System object.
**
** @fdata [EnsPCoordsystem]
** @fcategory modify
**
** @nam3rule Set Set one element of a Coordinate System
** @nam4rule SetAdaptor Set the Ensembl Coordinate System Adaptor
** @nam4rule SetIdentifier Set the SQL database-internal identifier
**
** @argrule * cs [EnsPCoordsystem] Ensembl Coordinate System object
**
** @valrule * [AjBool] ajTrue upon success, ajFalse otherwise
**
******************************************************************************/




/* @func ensCoordsystemSetAdaptor *********************************************
**
** Set the Object Adaptor element of an Ensembl Coordinate System.
**
** @param [u] cs [EnsPCoordsystem] Ensembl Coordinate System
** @param [r] adaptor [EnsPCoordsystemadaptor] Ensembl Coordinate
**                                             System Adaptor
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
** @@
******************************************************************************/

AjBool ensCoordsystemSetAdaptor(EnsPCoordsystem cs,
                                EnsPCoordsystemadaptor adaptor)
{
    if(!cs)
        return ajFalse;

    cs->Adaptor = adaptor;

    return ajTrue;
}




/* @func ensCoordsystemSetIdentifier ******************************************
**
** Set the SQL database-internal identifier element of an
** Ensembl Coordinate System.
**
** @param [u] cs [EnsPCoordsystem] Ensembl Coordinate System
** @param [r] identifier [ajuint] SQL database-internal identifier
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
** @@
******************************************************************************/

AjBool ensCoordsystemSetIdentifier(EnsPCoordsystem cs, ajuint identifier)
{
    if(!cs)
        return ajFalse;

    cs->Identifier = identifier;

    return ajTrue;
}




/* @section debugging *********************************************************
**
** Functions for reporting of an Ensembl Coordinate System object.
**
** @fdata [EnsPCoordsystem]
** @nam3rule Trace Report Ensembl Coordinate System elements to debug file
**
** @argrule Trace analysis [const EnsPCoordsystem] Ensembl Coordinate System
** @argrule Trace level [ajuint] Indentation level
**
** @valrule * [AjBool] ajTrue upon success, ajFalse otherwise
**
** @fcategory misc
******************************************************************************/




/* @func ensCoordsystemTrace **************************************************
**
** Trace an Ensembl Coordinate System.
**
** @param [r] cs [const EnsPCoordsystem] Ensembl Coordinate System
** @param [r] level [ajuint] Indentation level
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
** @@
******************************************************************************/

AjBool ensCoordsystemTrace(const EnsPCoordsystem cs, ajuint level)
{
    AjPStr indent = NULL;

    if(!cs)
        return ajFalse;

    indent = ajStrNew();

    ajStrAppendCountK(&indent, ' ', level * 2);

    ajDebug("%SensCoordsystemTrace %p\n"
            "%S  Use %u\n"
            "%S  Identifier %u\n"
            "%S  Adaptor %p\n"
            "%S  Name '%S'\n"
            "%S  Version '%S'\n"
            "%S  SequenceLevel '%B'\n"
            "%S  TopLevel '%B'\n"
            "%S  Default '%B'\n"
            "%S  Rank %u\n",
            indent, cs,
            indent, cs->Use,
            indent, cs->Identifier,
            indent, cs->Adaptor,
            indent, cs->Name,
            indent, cs->Version,
            indent, cs->SequenceLevel,
            indent, cs->TopLevel,
            indent, cs->Default,
            indent, cs->Rank);

    ajStrDel(&indent);

    return ajTrue;
}




/* @func ensCoordsystemMappingPathTrace ***************************************
**
** Trace an Ensembl Coordinate System mapping path.
**
** @param [r] css [const AjPList] AJAX List of Ensembl Coordinate Systems
** @param [r] level [ajuint] Indentation level
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
** @@
******************************************************************************/

AjBool ensCoordsystemMappingPathTrace(const AjPList css, ajuint level)
{
    AjIList iter = NULL;

    AjPStr indent = NULL;

    EnsPCoordsystem cs = NULL;

    if(!css)
        return ajFalse;

    indent = ajStrNew();

    ajStrAppendCountK(&indent, ' ', level * 2);

    ajDebug("%SensCoordsystemMappingPathTrace %p\n"
            "%S  length %u\n",
            indent, css,
            indent, ajListGetLength(css));

    iter = ajListIterNewread(css);

    while(!ajListIterDone(iter))
    {
        cs = (EnsPCoordsystem) ajListIterGet(iter);

        if(cs)
            ensCoordsystemTrace(cs, level + 1);
        else
            ajDebug("%S  <nul>\n", indent);
    }

    ajListIterDel(&iter);

    ajStrDel(&indent);

    return ajTrue;
}




/* @section comparison ********************************************************
**
** Functions for comparing Ensembl Coordinate Systems.
**
** @fdata [EnsPCoordsystem]
**
** @nam3rule  Match Compare two Coordinate Systems
**
** @argrule * cs1 [const EnsPCoordsystem] Coordinate System
** @argrule * cs2 [const EnsPCoordsystem] Coordinate System
**
** @valrule * [AjBool] True on success
**
** @fcategory use
******************************************************************************/




/* @func ensCoordsystemMatch **************************************************
**
** Test for matching two Ensembl Coordinate Systems.
**
** @param [r] cs1 [const EnsPCoordsystem] First Ensembl Coordinate System
** @param [r] cs2 [const EnsPCoordsystem] Second Ensembl Coordinate System
**
** @return [AjBool] ajTrue if the Ensembl Coordinate Systems are equal
** @@
** The comparison is based on an initial pointer equality test and if that
** fails, a case-insensitive string comparison of the name and version elements
** is performed.
******************************************************************************/

AjBool ensCoordsystemMatch(const EnsPCoordsystem cs1,
                           const EnsPCoordsystem cs2)
{
    if(ajDebugTest("ensCoordsystemMatch"))
    {
        ajDebug("ensCoordsystemMatch\n"
                "  cs1 %p\n"
                "  cs2 %p\n",
                cs1,
                cs2);

        ensCoordsystemTrace(cs1, 1);
        ensCoordsystemTrace(cs2, 1);
    }

    if(!cs1)
        return ajFalse;

    if(!cs2)
        return ajFalse;

    if(cs1 == cs2)
        return ajTrue;

    if(cs1->Identifier != cs2->Identifier)
        return ajFalse;

    if(!ajStrMatchCaseS(cs1->Name, cs2->Name))
        return ajFalse;

    if(!ajStrMatchCaseS(cs1->Version, cs2->Version))
        return ajFalse;

    if(cs1->Default != cs2->Default)
        return ajFalse;

    if(cs1->SequenceLevel != cs2->SequenceLevel)
        return ajFalse;

    if(cs1->TopLevel != cs2->TopLevel)
        return ajFalse;

    if(cs1->Rank != cs2->Rank)
        return ajFalse;

    return ajTrue;
}




/* @func ensCoordsystemGetMemSize *********************************************
**
** Get the memory size in bytes of an Ensembl Coordinate System.
**
** @param [r] cs [const EnsPCoordsystem] Ensembl Coordinate System
**
** @return [ajuint] Memory size
** @@
******************************************************************************/

ajuint ensCoordsystemGetMemSize(const EnsPCoordsystem cs)
{
    ajuint size = 0;

    if(!cs)
        return 0;

    size += (ajuint) sizeof (EnsOCoordsystem);

    if(cs->Name)
    {
        size += (ajuint) sizeof (AjOStr);

        size += ajStrGetRes(cs->Name);
    }

    if(cs->Version)
    {
        size += (ajuint) sizeof (AjOStr);

        size += ajStrGetRes(cs->Version);
    }

    return size;
}




/* @func ensCoordsystemGetSpecies *********************************************
**
** Get the species element of the Ensembl Database Adaptor the
** Ensembl Coordinate System Adaptor of an Ensembl Coordinate System is
** based on.
**
** @param [r] cs [EnsPCoordsystem] Ensembl Coordinate System
**
** @return [const AjPStr] Ensembl Database Adaptor species
** @@
******************************************************************************/

AjPStr ensCoordsystemGetSpecies(EnsPCoordsystem cs)
{
    if(!cs)
        return NULL;

    if(!cs->Adaptor)
        return NULL;

    return ensDatabaseadaptorGetSpecies(cs->Adaptor->Adaptor);
}




/* @datasection [EnsPCoordsystemadaptor] Coordinate System Adaptor ************
**
** Functions for manipulating Ensembl Coordinate System Adaptor objects
**
** @cc Bio::EnsEMBL::DBSQL::Coordsystemadaptor CVS Revision: 1.25
**
** @nam2rule Coordsystemadaptor
**
******************************************************************************/





/* @section constructors ******************************************************
**
** All constructors return a new Ensembl Coordinate System Adaptor by pointer.
** It is the responsibility of the user to first destroy any previous
** Coordinate System Adaptor. The target pointer does not need to be
** initialised to NULL, but it is good programming practice to do so anyway.
**
** @fdata [EnsPCoordsystemadaptor]
** @fnote None
**
** @nam3rule New Constructor
** @nam4rule NewObj Constructor with existing object
** @nam4rule NewRef Constructor by incrementing the reference counter
**
** @argrule New dba [EnsPDatabaseadaptor] Ensembl Database Adaptor
** @argrule Obj object [EnsPCoordsystemadaptor] Ensembl Coordinate
**                                              System Adaptor
** @argrule Ref object [EnsPCoordsystemadaptor] Ensembl Coordinate
**                                              System Adaptor
**
** @valrule * [EnsPCoordsystemadaptor] Ensembl Coordinate System Adaptor
**
** @fcategory new
******************************************************************************/




/* @funcstatic coordSystemadaptorFetchAllBySQL ********************************
**
** Run a SQL statement against an Ensembl Coordinate System Adaptor and
** consolidate the results into an AJAX List of Ensembl Coordinate System
** objects.
**
** The caller is responsible for deleting the Ensembl Coordinate Systems before
** deleting the AJAX List.
**
** @param [r] adaptor [EnsPCoordsystemadaptor] Ensembl Coordinate
**                                             System Adaptor
** @param [r] statement [const AjPStr] SQL statement
** @param [u] cslist [AjPList] AJAX List of Ensembl Coordinate Systems
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
** @@
******************************************************************************/

static AjBool coordSystemadaptorFetchAllBySQL(EnsPCoordsystemadaptor adaptor,
                                              const AjPStr statement,
                                              AjPList cslist)
{
    ajuint identifier = 0;
    ajuint rank       = 0;

    AjBool dflt     = AJFALSE;
    AjBool seqlevel = AJFALSE;
    AjBool toplevel = AJFALSE;

    AjPSqlstatement sqls = NULL;
    AjISqlrow sqli       = NULL;
    AjPSqlrow sqlr       = NULL;

    AjPStr name      = NULL;
    AjPStr version   = NULL;
    AjPStr attribute = NULL;
    AjPStr value     = NULL;

    AjPStrTok attrtoken = NULL;

    EnsPCoordsystem cs = NULL;

    if(!adaptor)
        return ajFalse;

    if(!statement)
        return ajFalse;

    if(!cslist)
        return ajFalse;

    sqls = ensDatabaseadaptorSqlstatementNew(adaptor->Adaptor, statement);

    sqli = ajSqlrowiterNew(sqls);

    while(!ajSqlrowiterDone(sqli))
    {
        identifier = 0;
        name       = ajStrNew();
        version    = ajStrNew();
        rank       = 0;
        attribute  = ajStrNew();

        value = ajStrNew();

        dflt     = ajFalse;
        toplevel = ajFalse;
        seqlevel = ajFalse;

        sqlr = ajSqlrowiterGet(sqli);

        ajSqlcolumnToUint(sqlr, &identifier);
        ajSqlcolumnToStr(sqlr, &name);
        ajSqlcolumnToStr(sqlr, &version);
        ajSqlcolumnToUint(sqlr, &rank);
        ajSqlcolumnToStr(sqlr, &attribute);

        attrtoken = ajStrTokenNewC(attribute, ",");

        while(ajStrTokenNextParse(&attrtoken, &value))
        {
            if(ajStrMatchCaseC(value, "default_version"))
                dflt = ajTrue;

            if(ajStrMatchCaseC(value, "sequence_level"))
                seqlevel = ajTrue;
        }

        ajStrTokenDel(&attrtoken);

        cs = ensCoordsystemNew(adaptor,
                               identifier,
                               name,
                               version,
                               rank,
                               dflt,
                               toplevel,
                               seqlevel);

        ajListPushAppend(cslist, (void *) cs);

        ajStrDel(&name);
        ajStrDel(&version);
        ajStrDel(&attribute);
        ajStrDel(&value);
    }

    ajSqlrowiterDel(&sqli);

    ajSqlstatementDel(&sqls);

    return ajTrue;
}




/* @funcstatic coordSystemadaptorCacheInit ************************************
**
** Initialise the internal Coordinate System cache of an
** Ensembl Coordinate System Adaptor.
**
** @param [u] adaptor [EnsPCoordsystemadaptor] Ensembl Coordinate
**                                             System Adaptor
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
** @@
******************************************************************************/

static AjBool coordSystemadaptorCacheInit(EnsPCoordsystemadaptor adaptor)
{
    ajuint *Pid   = NULL;
    ajuint *Prank = NULL;

    AjPList cslist = NULL;

    AjPStr statement = NULL;

    AjPTable versions = NULL;

    EnsPCoordsystem cs     = NULL;
    EnsPCoordsystem cstemp = NULL;

    if(!adaptor)
        return ajFalse;

    if(!adaptor->CacheByIdentifier)
        adaptor->CacheByIdentifier =
            ajTableNewFunctionLen(0, ensTableCmpUint, ensTableHashUint);

    if(!adaptor->CacheByName)
        adaptor->CacheByName = ajTablestrNewLen(0);

    if(!adaptor->CacheByRank)
        adaptor->CacheByRank =
            ajTableNewFunctionLen(0, ensTableCmpUint, ensTableHashUint);

    if(!adaptor->CacheByDefault)
        adaptor->CacheByDefault =
            ajTableNewFunctionLen(0, ensTableCmpUint, ensTableHashUint);

    statement = ajFmtStr(
        "SELECT "
        "coord_system.coord_system_id, "
        "coord_system.name, "
        "coord_system.version, "
        "coord_system.rank, "
        "coord_system.attrib "
        "FROM "
        "coord_system "
        "WHERE "
        "coord_system.species_id = %u",
        ensDatabaseadaptorGetIdentifier(adaptor->Adaptor));

    cslist = ajListNew();

    coordSystemadaptorFetchAllBySQL(adaptor, statement, cslist);

    while(ajListPop(cslist, (void **) &cs))
    {
        /* Sequence-level cache */

        if(cs->SequenceLevel)
            adaptor->SeqLevel = (void *) ensCoordsystemNewRef(cs);

        /* Identifier cache */

        AJNEW0(Pid);

        *Pid = cs->Identifier;

        cstemp = (EnsPCoordsystem)
            ajTablePut(adaptor->CacheByIdentifier,
                       (void *) Pid,
                       (void *) ensCoordsystemNewRef(cs));

        if(cstemp)
        {
            ajWarn("coordSystemadaptorCacheInit got more than one "
                   "Ensembl Coordinate System with (PRIMARY KEY) identifier "
                   "%u.\n",
                   cstemp->Identifier);

            ensCoordsystemDel(&cstemp);
        }

        /* Name and Version cache */

        /*
        ** For each Coordinate System of a particular name one or more
        ** versions are supported. Thus, Coordinate Systems are cached in
        ** two levels of AJAX Tables.
        **
        ** First-level Name Table: An AJAX Table storing Coordinate System
        ** name AJAX Strings as key data and second-level AJAX Tables as
        ** value data.
        **
        ** Second-level Version Table: An AJAX Table storing Coordinate System
        ** version AJAX Strings as key data and Coordinate Systems as
        ** value data.
        */

        versions = (AjPTable)
            ajTableFetch(adaptor->CacheByName, (const void *) cs->Name);

        if(!versions)
        {
            /* Create a new versions Table first. */

            versions = ajTablestrNewLen(0);

            ajTablePut(adaptor->CacheByName,
                       (void *) ajStrNewS(cs->Name),
                       (void *) versions);
        }

        cstemp = (EnsPCoordsystem)
            ajTablePut(versions,
                       (void *) ajStrNewS(cs->Version),
                       (void *) ensCoordsystemNewRef(cs));

        if(cstemp)
        {
            ajWarn("coordSystemadaptorCacheInit got more than one "
                   "Ensembl Coordinate System with (UNIQUE) name '%S' and "
                   "version '%S' with identifiers %u and %u.\n",
                   cstemp->Name,
                   cstemp->Version,
                   cstemp->Identifier,
                   cs->Identifier);

            ensCoordsystemDel(&cstemp);
        }

        /* Cache by Rank */

        AJNEW0(Prank);

        *Prank = cs->Rank;

        cstemp = (EnsPCoordsystem)
            ajTablePut(adaptor->CacheByRank,
                       (void *) Prank,
                       (void *) ensCoordsystemNewRef(cs));

        if(cstemp)
        {
            ajWarn("coordSystemadaptorCacheInit got more than one "
                   "Ensembl Coordinate System with (UNIQUE) rank %u and "
                   "identifiers %u and %u.\n",
                   cstemp->Rank,
                   cstemp->Identifier,
                   cs->Identifier);

            ensCoordsystemDel(&cstemp);
        }

        /* Defaults cache */

        /*
        ** Ensembl supports one or more default Coordinate Systems.
        ** Coordinate Systems are stored in a table keyed on the database
        ** identifier.
        */

        if(cs->Default)
        {
            AJNEW0(Pid);

            *Pid = cs->Identifier;

            cstemp = (EnsPCoordsystem)
                ajTablePut(adaptor->CacheByDefault,
                           (void *) Pid,
                           (void *) ensCoordsystemNewRef(cs));

            if(cstemp)
            {
                ajWarn("coordSystemadaptorCacheInit got more than one "
                       "Ensembl Coordinate System with (PRIMARY KEY) "
                       "identifier %u.\n",
                       cstemp->Identifier);

                ensCoordsystemDel(&cstemp);
            }
        }

        /*
        ** All caches keep internal references to the
        ** Ensembl Coordinate System objects.
        */

        ensCoordsystemDel(&cs);
    }

    ajListFree(&cslist);

    ajStrDel(&statement);

    return ajTrue;
}




/* @funcstatic coordSystemadaptorMapPathInit **********************************
**
** Initialise the internal Coordinate System mapping path cache of an
** Ensembl Coordinate System Adaptor.
**
** @param [u] adaptor [EnsPCoordsystemadaptor] Ensembl Coordinate
**                                             System Adaptor
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
** @@
******************************************************************************/

static AjBool coordSystemadaptorMapPathInit(EnsPCoordsystemadaptor adaptor)
{
    AjBool debug = AJFALSE;

    AjIList iter    = NULL;
    AjPList cskeys  = NULL;
    AjPList cslist  = NULL;
    AjPList mis     = NULL;
    AjPList maplist = NULL;

    AjPStr csname    = NULL;
    AjPStr csversion = NULL;
    AjPStr cskey     = NULL;
    AjPStr cs1key    = NULL;
    AjPStr cs2key    = NULL;
    AjPStr mapkey    = NULL;
    AjPStr metakey   = NULL;
    AjPStr metaval   = NULL;

    AjPStrTok cstoken   = NULL;
    AjPStrTok pathtoken = NULL;

    EnsPCoordsystem cs  = NULL;
    EnsPCoordsystem cs1 = NULL;
    EnsPCoordsystem cs2 = NULL;

    EnsPMetainformation mi = NULL;
    EnsPMetainformationadaptor mia = NULL;

    debug = ajDebugTest("coordSystemadaptorMapPathInit");

    if(debug)
        ajDebug("coordSystemadaptorMapPathInit\n"
                "  adaptor %p\n",
                adaptor);

    if(!adaptor)
        return ajFalse;

    if(!adaptor->MappingPaths)
        adaptor->MappingPaths = ajTablestrNewLen(0);

    cskeys = ajListNew();

    /* Read 'assembly.mapping' keys from the Ensembl Core 'meta' table. */

    mia = ensRegistryGetMetainformationadaptor(adaptor->Adaptor);

    metakey = ajStrNewC("assembly.mapping");

    mis = ajListNew();

    ensMetainformationadaptorFetchAllByKey(mia, metakey, mis);

    while(ajListPop(mis, (void **) &mi))
    {
        metaval = ensMetainformationGetValue(mi);

        /*
        ** Split 'assembly.mapping' Meta Information values on '#' or '|'
        ** characters into Coordinate System name:version keys.
        */

        pathtoken = ajStrTokenNewC(metaval, "#|");

        cskey = ajStrNew();

        while(ajStrTokenNextParse(&pathtoken, &cskey))
            ajListPushAppend(cskeys, (void *) ajStrNewS(cskey));

        ajStrDel(&cskey);

        ajStrTokenDel(&pathtoken);

        if(ajListGetLength(cskeys) < 2)
            ajWarn("coordSystemadaptorMapPathInit "
                   "got incorrectly formatted 'assembly.mapping' value from "
                   "Ensembl Core 'meta' table: '%S'",
                   metaval);
        else
        {
            /*
            ** Split Coordinate System keys into names and versions and
            ** fetch the corresponding Coordinate Systems from the database.
            */

            cslist = ajListNew();

            iter = ajListIterNew(cskeys);

            while(!ajListIterDone(iter))
            {
                cskey = (AjPStr) ajListIterGet(iter);

                cstoken = ajStrTokenNewC(cskey, ":");

                csname = ajStrNew();

                csversion = ajStrNew();

                ajStrTokenNextParse(&cstoken, &csname);

                ajStrTokenNextParse(&cstoken, &csversion);

                ensCoordsystemadaptorFetchByName(adaptor,
                                                 csname,
                                                 csversion,
                                                 &cs);

                if(cs)
                    ajListPushAppend(cslist, (void *) cs);
                else
                    ajWarn("coordSystemadaptorMapPathInit could not load "
                           "a Coordinate System for name '%S' and "
                           "version '%S', as specified in the "
                           "Ensembl Core 'meta' table by '%S'.",
                           csname,
                           csversion,
                           metaval);

                ajStrDel(&csname);

                ajStrDel(&csversion);

                ajStrTokenDel(&cstoken);
            }

            ajListIterDel(&iter);

            /*
            ** Test the 'assembly.mapping' Meta-Information value for
            ** '#' characters. A '#' delimiter indicates a special case,
            ** where multiple parts of a 'component' region map to the
            ** same part of an 'assembled' region. As this looks like the
            ** 'long' mapping, we just make the mapping path a bit longer.
            */

            if((ajStrFindC(metaval, "#") >= 0) &&
               (ajListGetLength(cslist) == 2))
            {
                /*
                ** Insert an empty middle node into the mapping path
                ** i.e. the AJAX List of Ensembl Coordinate Systems.
                */

                iter = ajListIterNew(cslist);

                (void) ajListIterGet(iter);

                ajListIterInsert(iter, NULL);

                ajListIterDel(&iter);

                if(debug)
                    ajDebug("coordSystemadaptorMapPathInit "
                            "elongated mapping path '%S'.\n",
                            metaval);
            }

            cs1 = NULL;

            cs2 = NULL;

            /*
            ** Take the first and last Coordinate Systems from the list and
            ** generate name:version Coordinate System keys, before
            ** a name1:version1|name2:version2 map key.
            */

            ajListPeekFirst(cslist, (void **) &cs1);

            if(debug)
            {
                ajDebug("coordSystemadaptorMapPathInit cs1 %p\n", cs1);

                ensCoordsystemTrace(cs1, 1);
            }

            cs1key = ajFmtStr("%S:%S", cs1->Name, cs1->Version);

            ajListPeekLast(cslist, (void **) &cs2);

            if(debug)
            {
                ajDebug("coordSystemadaptorMapPathInit cs2 %p\n", cs2);

                ensCoordsystemTrace(cs2, 1);
            }

            cs2key = ajFmtStr("%S:%S", cs2->Name, cs2->Version);

            mapkey = ajFmtStr("%S|%S", cs1key, cs2key);

            if(debug)
                ajDebug("coordSystemadaptorMapPathInit mapkey '%S'\n", mapkey);

            /* Does a mapping path already exist? */

            maplist = (AjPList)
                ajTableFetch(adaptor->MappingPaths, (const void *) mapkey);

            if(maplist)
            {
                /* A similar map path exists already. */

                ajDebug("coordSystemadaptorMapPathInit "
                        "The Ensembl Core 'meta' table specifies multiple "
                        "mapping paths between Coordinate Systems "
                        "'%S' and '%S'.\n"
                        "Choosing shorter path arbitrarily.\n",
                        cs1key,
                        cs2key);

                if(ajListGetLength(cslist) < ajListGetLength(maplist))
                {
                    /*
                    ** The current map path is shorter than the stored map
                    ** path. Replace the stored List with the current List
                    ** and delete the (longer) stored List. The Table key
                    ** String remains in place.
                    */

                    maplist = (AjPList)
                        ajTablePut(adaptor->MappingPaths,
                                   (void *) mapkey,
                                   (void *) cslist);

                    ajDebug("coordSystemadaptorMapPathInit "
                            "delete longer stored path!\n");

                    while(ajListPop(maplist, (void **) &cs))
                        ensCoordsystemDel(&cs);

                    ajListFree(&maplist);
                }
                else
                {
                    /*
                    ** The current map path is longer than the stored map
                    ** path. Delete this (longer) path list.
                    */

                    ajDebug("coordSystemadaptorMapPathInit "
                            "delete longer current path!\n");

                    while(ajListPop(cslist, (void **) &cs))
                        ensCoordsystemDel(&cs);

                    ajListFree(&cslist);
                }
            }
            else
            {
                /* No similar mappath exists so store the new mappath. */

                ajTablePut(adaptor->MappingPaths,
                           (void *) ajStrNewS(mapkey),
                           (void *) cslist);

                if(debug)
                    ajDebug("coordSystemadaptorMapPathInit "
                            "added new path '%S'.\n",
                            mapkey);
            }

            ajStrDel(&cs1key);
            ajStrDel(&cs2key);
            ajStrDel(&mapkey);
        }

        /*
        ** Clear the List of Coordinate System keys, but do not delete it for
        ** each Meta Information entry.
        */

        while(ajListPop(cskeys, (void **) &cskey))
            ajStrDel(&cskey);

        ensMetainformationDel(&mi);
    }

    ajListFree(&mis);

    ajStrDel(&metakey);

    ajListFree(&cskeys);

    return ajTrue;
}




/* @funcstatic coordSystemadaptorSeqregionMapInit *****************************
**
** Initialise the Ensembl Coordinate System Adaptor- internal
** Ensembl Sequence Region mapping cache for internal to external and
** vice versa mappings.
**
** @param [u] adaptor [EnsPCoordsystemadaptor] Ensembl Coordinate
**                                             System Adaptor
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
** @@
******************************************************************************/

static AjBool coordSystemadaptorSeqregionMapInit(
    EnsPCoordsystemadaptor adaptor)
{
    ajuint internal = 0;
    ajuint external = 0;

    ajuint *Pinternal = NULL;
    ajuint *Pexternal = NULL;
    ajuint *Pprevious = NULL;

    AjBool debug = AJFALSE;

    AjPSqlstatement sqls = NULL;
    AjISqlrow sqli       = NULL;
    AjPSqlrow sqlr       = NULL;

    AjPStr build = NULL;
    AjPStr statement = NULL;

    debug = ajDebugTest("coordSystemadaptorSeqregionMapInit");

    if(debug)
        ajDebug("coordSystemadaptorSeqregionMapInit\n"
                "  adaptor %p\n",
                adaptor);

    if(!adaptor)
        return ajFalse;

    if(adaptor->ExternalToInternal)
        return ajFalse;
    else
        adaptor->ExternalToInternal = MENSTABLEUINTNEW(0);

    if(adaptor->InternalToExternal)
        return ajFalse;
    else
        adaptor->InternalToExternal = MENSTABLEUINTNEW(0);

    build = ajStrNew();

    ensDatabaseadaptorGetSchemaBuild(adaptor->Adaptor, &build);

    if(debug)
        ajDebug("coordSystemadaptorSeqregionMapInit got build '%S'.\n", build);

    /* Get the relations for the current database. */

    statement = ajFmtStr(
        "SELECT "
        "seq_region_mapping.internal_seq_region_id, "
        "seq_region_mapping.external_seq_region_id "
        "FROM "
        "mapping_set, "
        "seq_region_mapping, "
        "seq_region, "
        "coord_system "
        "WHERE "
        "mapping_set.schema_build = '%S' "
        "AND "
        "mapping_set.mapping_set_id = seq_region_mapping.mapping_set_id "
        "AND "
        "seq_region_mapping.internal_seq_region_id = seq_region.seq_region_id "
        "AND "
        "seq_region.coord_system_id = coord_system.coord_system_id "
        "AND "
        "coord_system.species_id = %u",
        build,
        ensDatabaseadaptorGetIdentifier(adaptor->Adaptor));

    ajStrDel(&build);

    sqls = ensDatabaseadaptorSqlstatementNew(adaptor->Adaptor, statement);

    sqli = ajSqlrowiterNew(sqls);

    while(!ajSqlrowiterDone(sqli))
    {
        internal = 0;
        external = 0;

        sqlr = ajSqlrowiterGet(sqli);

        ajSqlcolumnToUint(sqlr, Pinternal);
        ajSqlcolumnToUint(sqlr, Pexternal);

        /* Internal to external mapping. */

        Pprevious = (ajuint *) ajTableFetch(adaptor->InternalToExternal,
                                            (const void *) &internal);

        if(Pprevious)
            ajDebug("coordSystemadaptorSeqregionMapInit got duplicate "
                    "internal Sequence Region identifier:\n"
                    "%u -> %u\n"
                    "%u -> %u\n",
                    internal, *Pprevious,
                    internal, external);
        else
        {
            AJNEW0(Pinternal);
            AJNEW0(Pexternal);

            *Pinternal = internal;
            *Pexternal = external;

            ajTablePut(adaptor->InternalToExternal,
                       (void *) Pinternal,
                       (void *) Pexternal);
        }

        /* External to internal mapping. */

        Pprevious = (ajuint *) ajTableFetch(adaptor->ExternalToInternal,
                                            (const void *) &external);

        if(Pprevious)
            ajDebug("coordSystemadaptorSeqregionMapInit got duplicate "
                    "external Sequence Region identifier:\n"
                    "%u -> %u\n"
                    "%u -> %u\n",
                    external, *Pprevious,
                    external, internal);
        else
        {
            AJNEW0(Pinternal);
            AJNEW0(Pexternal);

            *Pinternal = internal;
            *Pexternal = external;

            ajTablePut(adaptor->ExternalToInternal,
                       (void *) Pexternal,
                       (void *) Pinternal);
        }
    }

    ajSqlrowiterDel(&sqli);

    ajSqlstatementDel(&sqls);

    ajStrDel(&statement);

    return ajTrue;
}




/* @func ensCoordsystemadaptorNew *********************************************
**
** Default constructor for an Ensembl Coordinate System Adaptor.
**
** @param [r] dba [EnsPDatabaseadaptor] Ensembl Database Adaptor
**
** @return [EnsPCoordsystemadaptor] Ensembl Coordinate System Adaptor or NULL
** @@
******************************************************************************/

EnsPCoordsystemadaptor ensCoordsystemadaptorNew(EnsPDatabaseadaptor dba)
{
    EnsPCoordsystemadaptor adaptor = NULL;

    if(!dba)
        return NULL;

    AJNEW0(adaptor);

    adaptor->Adaptor = dba;

    coordSystemadaptorCacheInit(adaptor);

    coordSystemadaptorMapPathInit(adaptor);

    coordSystemadaptorSeqregionMapInit(adaptor);

    /*
    ** Create a Pseudo-Coordinate System 'toplevel' and cache it so that only
    ** one of these is created for each database.
    */

    adaptor->TopLevel = (void *)
        ensCoordsystemNew(adaptor, 0, NULL, NULL, 0, ajFalse, ajTrue, ajFalse);

    return adaptor;
}




/* @section destructors *******************************************************
**
** Destruction destroys all internal data structures and frees the
** memory allocated for the Ensembl Coordinate System Adaptor.
**
** @fdata [EnsPCoordsystemadaptor]
** @fnote None
**
** @nam3rule Del Destroy (free) an Ensembl Coordinate System Adaptor object
**
** @argrule * Padaptor [EnsPCoordsystemadaptor*] Ensembl Coordinate
**                                               System Adaptor object address
**
** @valrule * [void]
**
** @fcategory delete
******************************************************************************/




/* @funcstatic coordSystemadaptorClearIdentifierCache *************************
**
** An ajTableMapDel 'apply' function to clear the Ensembl Coordinate System
** Adaptor-internal Ensembl Coordinate System cache. This function deletes the
** unsigned integer identifier key data and the Ensembl Coordinate System value
** data.
**
** @param [u] key [void**] AJAX unsigned integer key data address
** @param [u] value [void**] Ensembl Coordinate System value data address
** @param [u] cl [void*] Standard, passed in from ajTableMapDel
** @see ajTableMapDel
**
** @return [void]
** @@
******************************************************************************/

static void coordSystemadaptorClearIdentifierCache(void **key,
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

    ensCoordsystemDel((EnsPCoordsystem *) value);

    return;
}




/* @funcstatic coordSystemadaptorClearNameCacheL2 *****************************
**
** An ajTableMapDel 'apply' function to clear the Ensembl Coordinate System
** Adaptor-internal Ensembl Coordinate System cache. This function clears the
** second-level cache of Coordinate System version AJAX String key data and
** Ensembl Coordinate System value data.
**
** @param [u] key [void**] AJAX String key data address
** @param [u] value [void**] Ensembl Coordinate System value data address
** @param [u] cl [void*] Standard, passed in from ajTableMapDel
** @see ajTableMapDel
**
** @return [void]
** @@
******************************************************************************/

static void coordSystemadaptorClearNameCacheL2(void **key,
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

    ensCoordsystemDel((EnsPCoordsystem *) value);

    return;
}




/* @funcstatic coordSystemadaptorClearNameCacheL1 *****************************
**
** An ajTableMapDel 'apply' function to clear the Ensembl Coordinate System
** Adaptor-internal Ensembl Coordinate System cache. This function clears the
** first-level cache of Coordinate System name AJAX String key data and the
** second-level AJAX Table value data.
**
** @param [u] key [void**] AJAX String key data address
** @param [u] value [void**] AJAX Table value data address
** @param [u] cl [void*] Standard, passed in from ajTableMapDel
** @see ajTableMapDel
**
** @return [void]
** @@
******************************************************************************/

static void coordSystemadaptorClearNameCacheL1(void **key,
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

    ajTableMapDel(*((AjPTable *) value),
                  coordSystemadaptorClearNameCacheL2, NULL);

    ajTableFree((AjPTable *) value);

    return;
}




/* @funcstatic coordSystemadaptorCacheExit ************************************
**
** Clears the internal Coordinate System cache of an
** Ensembl Coordinate System Adaptor.
**
** @param [u] adaptor [EnsPCoordsystemadaptor] Ensembl Coordinate
**                                             System Adaptor
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
** @@
******************************************************************************/

static AjBool coordSystemadaptorCacheExit(EnsPCoordsystemadaptor adaptor)
{
    if(ajDebugTest("coordSystemadaptorCacheExit"))
        ajDebug("coordSystemadaptorCacheExit\n"
                "  adaptor %p\n",
                adaptor);

    if(!adaptor)
        return ajFalse;

    /* Clear the Sequence-level cache */

    ensCoordsystemDel((EnsPCoordsystem *) &(adaptor->SeqLevel));

    /* Clear the identifier cache. */

    ajTableMapDel(adaptor->CacheByIdentifier,
                  coordSystemadaptorClearIdentifierCache,
                  NULL);

    ajTableFree(&(adaptor->CacheByIdentifier));

    /* Clear the name cache. */

    ajTableMapDel(adaptor->CacheByName,
                  coordSystemadaptorClearNameCacheL1,
                  NULL);

    ajTableFree(&(adaptor->CacheByName));

    /* Clear the rank cache. */

    ajTableMapDel(adaptor->CacheByRank,
                  coordSystemadaptorClearIdentifierCache,
                  NULL);

    ajTableFree(&(adaptor->CacheByRank));

    /* Clear the defaults cache. */

    ajTableMapDel(adaptor->CacheByDefault,
                  coordSystemadaptorClearIdentifierCache,
                  NULL);

    ajTableFree(&(adaptor->CacheByDefault));

    return ajTrue;
}




/* @funcstatic coordSystemadaptorClearMapPath *********************************
**
** An ajTableMapDel 'apply' function to clear the Ensembl Coordinate System
** Adaptor-internal Coordinate System mapping path. This function clears
** the AJAX String key data and the AJAX List value data, as well as the
** Ensembl Coordinate Systems from the AJAX List.
**
** @param [u] key [void**] AJAX String key data address
** @param [u] value [void**] AJAX List value data address
** @param [u] cl [void*] Standard, passed in from ajTableMapDel
** @see ajTableMapDel
**
** @return [void]
** @@
******************************************************************************/

static void coordSystemadaptorClearMapPath(void **key,
                                           void **value,
                                           void *cl)
{
    EnsPCoordsystem cs = NULL;

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

    while(ajListPop(*((AjPList *) value), (void **) &cs))
        ensCoordsystemDel(&cs);

    ajListFree((AjPList *) value);

    return;
}




/* @funcstatic coordSystemadaptorMapPathExit **********************************
**
** Clears the internal Coordinate System mapping path cache of an
** Ensembl Coordinate System Adaptor.
**
** @param [u] adaptor [EnsPCoordsystemadaptor] Ensembl Coordinate
**                                             System Adaptor
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
** @@
******************************************************************************/

static AjBool coordSystemadaptorMapPathExit(EnsPCoordsystemadaptor adaptor)
{
    if(!adaptor)
        return ajFalse;

    ajTableMapDel(adaptor->MappingPaths, coordSystemadaptorClearMapPath, NULL);

    ajTableFree(&(adaptor->MappingPaths));

    return ajTrue;
}




/* @funcstatic coordSystemadaptorClearSeqregionMap ****************************
**
** An ajTableMapDel 'apply' function to clear the Ensembl Coordinate System
** Adaptor-internal Ensembl Sequence Region identifier mapping cache.
** This function clears the unsigned integer key data and the unsigned integer
** value data.
**
** @param [u] key [void**] AJAX unsigned integer key data address
** @param [u] value [void**] AJAX unsigned integer value data address
** @param [u] cl [void*] Standard, passed in from ajTableMapDel
** @see ajTableMapDel
**
** @return [void]
** @@
******************************************************************************/

static void coordSystemadaptorClearSeqregionMap(void **key,
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




/* @funcstatic coordSystemadaptorSeqregionMapExit *****************************
**
** Clears the internal Sequence Region mapping cache of an
** Ensembl Coordinate System Adaptor.
**
** @param [u] adaptor [EnsPCoordsystemadaptor] Ensembl Coordinate
**                                             System Adaptor
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
** @@
******************************************************************************/

static AjBool coordSystemadaptorSeqregionMapExit(
    EnsPCoordsystemadaptor adaptor)
{
    if(!adaptor)
        return ajFalse;

    ajTableMapDel(adaptor->ExternalToInternal,
                  coordSystemadaptorClearSeqregionMap,
                  NULL);

    ajTableFree(&(adaptor->ExternalToInternal));

    ajTableMapDel(adaptor->InternalToExternal,
                  coordSystemadaptorClearSeqregionMap,
                  NULL);

    ajTableFree(&(adaptor->InternalToExternal));

    return ajTrue;
}




/* @func ensCoordsystemadaptorDel *********************************************
**
** Default destructor for an Ensembl Coordinate System Adaptor.
** This function also clears the internal cordinate system and mapping path
** caches.
**
** @param [d] Padaptor [EnsPCoordsystemadaptor*] Ensembl Coordinate
**                                               System Adaptor address
**
** @return [void]
** @@
******************************************************************************/

void ensCoordsystemadaptorDel(EnsPCoordsystemadaptor* Padaptor)
{
    EnsPCoordsystemadaptor pthis = NULL;

    if(!Padaptor)
        return;

    if(!*Padaptor)
        return;

    if(ajDebugTest("ensCoordsystemadaptorDel"))
        ajDebug("ensCoordsystemadaptorDel\n"
                "  *Padaptor %p\n",
                *Padaptor);

    pthis = *Padaptor;

    coordSystemadaptorCacheExit(pthis);

    coordSystemadaptorMapPathExit(pthis);

    coordSystemadaptorSeqregionMapExit(pthis);

    /* Clear the top-level cache. */

    ensCoordsystemDel((EnsPCoordsystem *) &pthis->TopLevel);

    AJFREE(pthis);

    *Padaptor = NULL;

    return;
}




/* @section element retrieval *************************************************
**
** Functions for returning elements of an Ensembl Coordinate System Adaptor
** object.
**
** @fdata [EnsPCoordsystemadaptor]
** @fcategory use
**
** @nam3rule Get Return Coordinate System Adaptor attribute(s)
** @nam4rule GetDatabaseadaptor Return the Ensembl Database Adaptor
**
** @argrule * adaptor [const EnsPCoordsystemadaptor] Coordinate System Adaptor
**
** @valrule Databaseadaptor [EnsPDatabaseadaptor] Ensembl Database Adaptor
******************************************************************************/




/* @func ensCoordsystemadaptorGetDatabaseadaptor ******************************
**
** Get the Ensembl Database Adaptor element of an
** Ensembl Coordinate System Adaptor.
**
** @param [r] adaptor [const EnsPCoordsystemadaptor] Ensembl Coordinate
**                                                   System Adaptor
**
** @return [EnsPDatabaseadaptor] Ensembl Database Adaptor or NULL
** @@
******************************************************************************/

EnsPDatabaseadaptor ensCoordsystemadaptorGetDatabaseadaptor(
    const EnsPCoordsystemadaptor adaptor)
{
    if(!adaptor)
        return NULL;

    return adaptor->Adaptor;
}




/* @section object retrieval **************************************************
**
** Functions for retrieving Ensembl Coordinate System objects from an
** Ensembl Core database.
**
** @fdata [EnsPCoordsystemadaptor]
** @fnote None
**
** @nam3rule Fetch Retrieve Ensembl Coordinate System object(s)
** @nam4rule FetchAll Retrieve all Ensembl Coordinate System objects
** @nam5rule FetchAllBy Retrieve all Ensembl Coordinate System objects
**                      matching a criterion
** @nam4rule FetchBy Retrieve one Ensembl Coordinate System object
**                   matching a criterion
**
** @argrule * adaptor [const EnsPCoordsystemadaptor] Ensembl Coordinate
**                                                   System Adaptor
** @argrule FetchAll cslist [AjPList] AJAX List of Ensembl Coordinate Systems
** @argrule FetchBy Pcs [EnsPCoordsystem*] Ensembl Coordinate System address
**
** @valrule * [AjBool] ajTrue upon success, ajFalse otherwise
**
** @fcategory use
******************************************************************************/




/* @funcstatic coordSystemadaptorFetchAll *************************************
**
** An ajTableMap 'apply' function to return all Ensembl Coordinate Systems from
** an Ensembl Coordinate System Adaptor-internal cache.
** The caller is responsible for deleting the Ensembl Coordinate Systems before
** deleting the AJAX List.
**
** @param [u] key [const void *] AJAX unsigned integer key data address
** @param [u] value [void**] Ensembl Coordinate System value data address
** @param [u] cl [void*] AJAX List of Ensembl Coordinate System objects,
**                       passed in via ajTableMap
** @see ajTableMap
**
** @return [void]
** @@
******************************************************************************/

static void coordSystemadaptorFetchAll(const void *key,
                                       void **value,
                                       void *cl)
{
    if(!key)
        return;

    if(!value)
        return;

    if(!*value)
        return;

    if(!cl)
        return;

    ajListPushAppend((AjPList) cl, (void *)
                     ensCoordsystemNewRef(*((EnsPCoordsystem *) value)));

    return;
}




/* @func ensCoordsystemadaptorFetchAll ****************************************
**
** Fetch all Ensembl Coordinate Systems.
** The caller is responsible for deleting the Ensembl Coordinate Systems before
** deleting the AJAX List.
**
** @param [r] adaptor [const EnsPCoordsystemadaptor] Ensembl Coordinate
**                                                   System Adaptor
** @param [r] cslist [AjPList] AJAX List of Ensembl Coordinate Systems
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
** @@
******************************************************************************/

AjBool ensCoordsystemadaptorFetchAll(
    const EnsPCoordsystemadaptor adaptor,
    AjPList cslist)
{
    if(!adaptor)
        return ajFalse;

    if(!cslist)
        return ajFalse;

    ajTableMap(adaptor->CacheByIdentifier,
               coordSystemadaptorFetchAll,
               (void *) cslist);

    return ajTrue;
}




/* @func ensCoordsystemadaptorFetchAllByName **********************************
**
** Fetch Ensembl Coordinate Systems of all versions for a name.
** The caller is responsible for deleting the Ensembl Coordinate Systems before
** deleting the AJAX List.
**
** @param [r] adaptor [const EnsPCoordsystemadaptor] Ensembl Coordinate
**                                                   System Adaptor
** @param [r] name [const AjPStr] Name
** @param [u] cslist [AjPList] AJAX List of Ensembl Coordinate Systems
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
** @@
******************************************************************************/

AjBool ensCoordsystemadaptorFetchAllByName(
    const EnsPCoordsystemadaptor adaptor,
    const AjPStr name,
    AjPList cslist)
{
    AjPTable versions = NULL;

    EnsPCoordsystem cs = NULL;

    if(!adaptor)
        return ajFalse;

    if(!name)
        return ajFalse;

    if(!cslist)
        return ajFalse;

    if(ajStrMatchCaseC(name, "seqlevel"))
    {
        ensCoordsystemadaptorFetchSeqLevel(adaptor, &cs);

        ajListPushAppend(cslist, (void *) cs);

        return ajTrue;
    }

    if(ajStrMatchCaseC(name, "toplevel"))
    {
        ensCoordsystemadaptorFetchTopLevel(adaptor, &cs);

        ajListPushAppend(cslist, (void *) cs);

        return ajTrue;
    }

    versions = (AjPTable)
        ajTableFetch(adaptor->CacheByName, (const void *) name);

    if(versions)
        ajTableMap(versions, coordSystemadaptorFetchAll, (void *) cslist);

    return ajTrue;
}




/* @func ensCoordsystemadaptorFetchByIdentifier *******************************
**
** Fetch an Ensembl Coordinate System by its internal database identifier.
** The caller is responsible for deleting the Ensembl Coordinate System.
**
** @param [r] adaptor [const EnsPCoordsystemadaptor] Ensembl Coordinate
**                                                   System Adaptor
** @param [r] identifier [ajuint] SQL database-internal identifier
** @param [wP] Pcs [EnsPCoordsystem*] Ensembl Coordinate System address
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
** @@
******************************************************************************/

AjBool ensCoordsystemadaptorFetchByIdentifier(
    const EnsPCoordsystemadaptor adaptor,
    ajuint identifier,
    EnsPCoordsystem *Pcs)
{
    if(!adaptor)
        return ajFalse;

    if(!identifier)
        return ajFalse;

    if(!Pcs)
        return ajFalse;

    *Pcs = (EnsPCoordsystem)
        ajTableFetch(adaptor->CacheByIdentifier, (const void *) &identifier);

    ensCoordsystemNewRef(*Pcs);

    return ajTrue;
}




/* @func ensCoordsystemadaptorFetchByName *************************************
**
** Fetch an Ensembl Coordinate System by name and version.
** The caller is responsible for deleting the Ensembl Coordinate System.
**
** @param [r] adaptor [const EnsPCoordsystemadaptor] Ensembl Coordinate
**                                                   System Adaptor
** @param [r] name [const AjPStr] Name
** @param [rN] version [const AjPStr] Version
** @param [wP] Pcs [EnsPCoordsystem*] Ensembl Coordinate System address
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
** @@
******************************************************************************/

AjBool ensCoordsystemadaptorFetchByName(
    const EnsPCoordsystemadaptor adaptor,
    const AjPStr name,
    const AjPStr version,
    EnsPCoordsystem *Pcs)
{
    void **valarray = NULL;

    register ajuint i = 0;

    AjPTable versions = NULL;

    EnsPCoordsystem cs = NULL;

    if(ajDebugTest("ensCoordsystemadaptorFetchByName"))
        ajDebug("ensCoordsystemadaptorFetchByName\n"
                "  adaptor %p\n"
                "  name '%S'\n"
                "  version '%S'\n"
                "  Pcs %p\n",
                adaptor,
                name,
                version,
                Pcs);

    if(!adaptor)
        return ajFalse;

    if(!name)
        return ajFalse;

    if(!Pcs)
        return ajFalse;

    *Pcs = (EnsPCoordsystem) NULL;

    if(ajStrMatchCaseC(name, "seqlevel"))
        return ensCoordsystemadaptorFetchSeqLevel(adaptor, Pcs);

    if(ajStrMatchCaseC(name, "toplevel"))
        return ensCoordsystemadaptorFetchTopLevel(adaptor, Pcs);

    versions = (AjPTable)
        ajTableFetch(adaptor->CacheByName, (const void *) name);

    if(versions)
    {
        if(version && ajStrGetLen(version))
            *Pcs = (EnsPCoordsystem)
                ajTableFetch(versions, (const void *) version);
        else
        {
            /*
            ** If no version has been specified search for the default
            ** Ensembl Coordinate System of this name.
            */

            ajTableToarrayValues(versions, &valarray);

            for(i = 0; valarray[i]; i++)
            {
                cs = (EnsPCoordsystem) valarray[i];

                if(cs->Default)
                {
                    *Pcs = cs;

                    break;
                }
            }

            AJFREE(valarray);
        }

        ensCoordsystemNewRef(*Pcs);

        return ajTrue;
    }

    return ajTrue;
}




/* @func ensCoordsystemadaptorFetchByRank *************************************
**
** Fetch an Ensembl Coordinate System by its rank.
** The caller is responsible for deleting the Ensembl Coordinate System.
**
** @param [r] adaptor [const EnsPCoordsystemadaptor] Ensembl Coordinate
**                                                   System Adaptor
** @param [r] rank [ajuint] Rank
** @param [wP] Pcs [EnsPCoordsystem*] Ensembl Coordinate System address
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
** @@
******************************************************************************/

AjBool ensCoordsystemadaptorFetchByRank(
    const EnsPCoordsystemadaptor adaptor,
    ajuint rank,
    EnsPCoordsystem *Pcs)
{
    if(!adaptor)
        return ajFalse;

    if(!rank)
        return ensCoordsystemadaptorFetchTopLevel(adaptor, Pcs);

    if(!Pcs)
        return ajFalse;

    *Pcs = (EnsPCoordsystem)
        ajTableFetch(adaptor->CacheByRank, (const void *) &rank);

    ensCoordsystemNewRef(*Pcs);

    return ajTrue;
}




/* @func ensCoordsystemadaptorFetchSeqLevel ***********************************
**
** Fetch the sequence-level Ensembl Coordinate System.
** The caller is responsible for deleting the Ensembl Coordinate System.
**
** @param [r] adaptor [const EnsPCoordsystemadaptor] Ensembl Coordinate
**                                                   System Adaptor
** @param [wP] Pcs [EnsPCoordsystem*] Ensembl Coordinate System address
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
** @@
******************************************************************************/

AjBool ensCoordsystemadaptorFetchSeqLevel(
    const EnsPCoordsystemadaptor adaptor,
    EnsPCoordsystem *Pcs)
{
    if(!adaptor)
        return ajFalse;

    if(!Pcs)
        return ajFalse;

    *Pcs = ensCoordsystemNewRef((EnsPCoordsystem) adaptor->SeqLevel);

    return ajTrue;
}




/* @func ensCoordsystemadaptorFetchTopLevel ***********************************
**
** Fetch the top-level Ensembl Coordinate System.
** The caller is responsible for deleting the Ensembl Coordinate System.
**
** @param [r] adaptor [const EnsPCoordsystemadaptor] Ensembl Coordinate
**                                                   System Adaptor
** @param [wP] Pcs [EnsPCoordsystem*] Ensembl Coordinate System address
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
** @@
******************************************************************************/

AjBool ensCoordsystemadaptorFetchTopLevel(
    const EnsPCoordsystemadaptor adaptor,
    EnsPCoordsystem *Pcs)
{
    if(!adaptor)
        return ajFalse;

    if(!Pcs)
        return ajFalse;

    *Pcs = ensCoordsystemNewRef((EnsPCoordsystem) adaptor->TopLevel);

    return ajTrue;
}




/* @func ensCoordsystemadaptorGetMappingPath **********************************
**
** Fetch a mapping path between two Ensembl Coordinate Systems.
**
** @param [r] adaptor [const EnsPCoordsystemadaptor] Ensembl Coordinate
**                                                   System Adaptor
** @param [r] cs1 [EnsPCoordsystem] First Ensembl Coordinate System
** @param [r] cs2 [EnsPCoordsystem] Second Ensembl Coordinate System
**
** @return [const AjPList] AJAX List of Ensembl Coordinate Systems or NULL
** @@
** Given two Coordinate Systems this will return a mapping path between them
** if one has been defined. Allowed Mapping paths are explicitly defined in the
** 'meta' table. The following is an example:
**
** mysql> select * from meta where meta_key = 'assembly.mapping';
**
**   +---------+------------------+--------------------------------------+
**   | meta_id | meta_key         | meta_value                           |
**   +---------+------------------+--------------------------------------+
**   |      20 | assembly.mapping | chromosome:NCBI34|contig             |
**   |      21 | assembly.mapping | clone|contig                         |
**   |      22 | assembly.mapping | supercontig|contig                   |
**   |      23 | assembly.mapping | chromosome:NCBI34|contig|clone       |
**   |      24 | assembly.mapping | chromosome:NCBI34|contig|supercontig |
**   |      25 | assembly.mapping | supercontig|contig|clone             |
**   +---------+------------------+--------------------------------------+
**
** For a one-step mapping path to be valid there needs to be a relationship
** between the two Coordinate Systems defined in the assembly table. Two step
** mapping paths work by building on the one-step mapping paths which are
** already defined.
**
** The first coordinate system in a one step mapping path must be the assembled
** coordinate system and the second must be the component.
**
** A '#' delimiter indicates a special case where multiple parts of a
** 'component' region map to the same part of an 'assembled' region.
** In those cases, the mapping path is elongated by an empty middle node.
******************************************************************************/

const AjPList ensCoordsystemadaptorGetMappingPath(
    const EnsPCoordsystemadaptor adaptor,
    EnsPCoordsystem cs1,
    EnsPCoordsystem cs2)
{
    void **keyarray = NULL;
    void **valarray = NULL;

    register ajuint i = 0;

    ajint match = 0;

    AjPList mappath = NULL;
    AjPList midpath = NULL;
    AjPList tmppath = NULL;

    AjPStr cs1key = NULL;
    AjPStr cs2key = NULL;
    AjPStr midkey = NULL;
    AjPStr mapkey = NULL;

    AjPStr cs1str = NULL;
    AjPStr cs2str = NULL;
    AjPStr midstr = NULL;

    AjPTable midcs1 = NULL;
    AjPTable midcs2 = NULL;

    EnsPCoordsystem csp0 = NULL;
    EnsPCoordsystem csp1 = NULL;
    EnsPCoordsystem midcs = NULL;

    if(!adaptor)
        return NULL;

    if(!cs1)
        return NULL;

    if(!cs2)
        return NULL;

    cs1key = ajFmtStr("%S:%S", cs1->Name, cs1->Version);

    cs2key = ajFmtStr("%S:%S", cs2->Name, cs2->Version);

    /* Lookup path for cs1key|cs2key. */

    mapkey = ajFmtStr("%S|%S", cs1key, cs2key);

    mappath = (AjPList)
        ajTableFetch(adaptor->MappingPaths, (const void *) mapkey);

    ajStrDel(&mapkey);

    if(mappath && ajListGetLength(mappath))
    {
        ajStrDel(&cs1key);

        ajStrDel(&cs2key);

        return mappath;
    }

    /* Lookup path for cs2key|cs1key. */

    mapkey = ajFmtStr("%S|%S", cs2key, cs1key);

    mappath = (AjPList)
        ajTableFetch(adaptor->MappingPaths, (const void *) mapkey);

    ajStrDel(&mapkey);

    if(mappath && ajListGetLength(mappath))
    {
        ajStrDel(&cs1key);

        ajStrDel(&cs2key);

        return mappath;
    }

    /*
    ** Still no success. Search for a chained mapping path involving two
    ** coordinate system pairs with a shared middle coordinate system.
    */

    ajDebug("ensCoordsystemadaptorGetMappingPath "
            "no explicit coordinate mapping path between "
            "'%S' and '%S' defined.\n",
            cs1key,
            cs2key);

    /*
    ** Iterate over all mapping paths stored in the
    ** Coordinate System Adaptor.
    */

    midcs1 = ajTablestrNewLen(0);

    midcs2 = ajTablestrNewLen(0);

    ajTableToarrayKeysValues(adaptor->MappingPaths, &keyarray, &valarray);

    for(i = 0; keyarray[i]; i++)
    {
        tmppath = (AjPList) valarray[i];

        /* Consider only paths of two components. */

        if(ajListGetLength(tmppath) != 2)
            continue;

        ajListPeekNumber(tmppath, 0, (void **) &csp0);

        ajListPeekNumber(tmppath, 1, (void **) &csp1);

        match = -1;

        if(ensCoordsystemMatch(csp0, cs1))
            match = 1;

        if(ensCoordsystemMatch(csp1, cs1))
            match = 0;

        if(match >= 0)
        {
            ajListPeekNumber(tmppath, (ajuint) match, (void **) &midcs);

            midkey = ajFmtStr("%S:%S", midcs->Name, midcs->Version);

            if(ajTableFetch(midcs2, (const void *) midkey))
            {
                mapkey = ajFmtStr("%S|%S", cs1key, cs2key);

                midpath = ajListNew();

                ajListPushAppend(midpath,
                                 (void *) ensCoordsystemNewRef(cs1));

                ajListPushAppend(midpath,
                                 (void *) ensCoordsystemNewRef(midcs));

                ajListPushAppend(midpath,
                                 (void *) ensCoordsystemNewRef(cs2));

                ajTablePut(adaptor->MappingPaths,
                           (void *) mapkey,
                           (void *) midpath);

                mappath = midpath;

                /*
                ** Inform the user that an explicit assembly.mapping entry is
                ** missing from the Ensembl Core meta table.
                */

                cs1str = (ajStrGetLen(cs1->Version)) ?
                    ajFmtStr("%S:%S", cs1->Name, cs1->Version) :
                    ajStrNewS(cs1->Name);

                midstr = (ajStrGetLen(midcs->Version)) ?
                    ajFmtStr("%S:%S", midcs->Name, midcs->Version) :
                    ajStrNewS(midcs->Name);

                cs2str = (ajStrGetLen(cs2->Version)) ?
                    ajFmtStr("%S:%S", cs2->Name, cs2->Version) :
                    ajStrNewS(cs2->Name);

                ajDebug("ensCoordsystemadaptorGetMappingPath "
                        "Using implicit mapping path between "
                        "'%S' and '%S' Coordinate Systems.\n"
                        "An explicit 'assembly.mapping' entry should be "
                        "added to the Ensembl Core 'meta' table.\n"
                        "Example: '%S|%S|%S'\n",
                        cs1str,
                        cs2str,
                        cs1str,
                        midstr,
                        cs2str);

                ajStrDel(&cs1str);

                ajStrDel(&cs2str);

                ajStrDel(&midstr);

                ajStrDel(&midkey);

                break;
            }
            else
            {
                midpath = ajListNew();

                ajListPushAppend(midpath,
                                 (void *) ensCoordsystemNewRef(midcs));

                ajTablePut(midcs1,
                           (void *) ajStrNewS(midkey),
                           (void *) midpath);
            }

            ajStrDel(&midkey);
        }

        match = -1;

        if(ensCoordsystemMatch(csp0, cs2))
            match = 1;

        if(ensCoordsystemMatch(csp1, cs2))
            match = 0;

        if(match >= 0)
        {
            ajListPeekNumber(tmppath, (ajuint) match, (void **) &midcs);

            midkey = ajFmtStr("%S:%S", midcs->Name, midcs->Version);

            if(ajTableFetch(midcs1, (const void *) midkey))
            {
                mapkey = ajFmtStr("%S|%S", cs2key, cs1key);

                midpath = ajListNew();

                ajListPushAppend(midpath,
                                 (void *) ensCoordsystemNewRef(cs2));

                ajListPushAppend(midpath,
                                 (void *) ensCoordsystemNewRef(midcs));

                ajListPushAppend(midpath,
                                 (void *) ensCoordsystemNewRef(cs1));

                ajTablePut(adaptor->MappingPaths,
                           (void *) mapkey,
                           (void *) midpath);

                mappath = midpath;

                /*
                ** Inform the user that an explicit assembly.mapping entry is
                ** missing from the Ensembl Core meta table.
                */

                cs1str = (ajStrGetLen(cs1->Version)) ?
                    ajFmtStr("%S:%S", cs1->Name, cs1->Version) :
                    ajStrNewS(cs1->Name);

                midstr = (ajStrGetLen(midcs->Version)) ?
                    ajFmtStr("%S:%S", midcs->Name, midcs->Version) :
                    ajStrNewS(midcs->Name);

                cs2str = (ajStrGetLen(cs2->Version)) ?
                    ajFmtStr("%S:%S", cs2->Name, cs2->Version) :
                    ajStrNewS(cs2->Name);

                ajDebug("ensCoordsystemadaptorGetMappingPath "
                        "Using implicit mapping path between "
                        "'%S' and '%S' Coordinate Systems.\n"
                        "An explicit 'assembly.mapping' entry should be "
                        "added to the Ensembl Core 'meta' table.\n"
                        "Example: '%S|%S|%S'\n",
                        cs1str,
                        cs2str,
                        cs1str,
                        midstr,
                        cs2str);

                ajStrDel(&cs1str);
                ajStrDel(&cs2str);
                ajStrDel(&midstr);
                ajStrDel(&midkey);

                break;
            }
            else
            {
                midpath = ajListNew();

                ajListPushAppend(midpath,
                                 (void *) ensCoordsystemNewRef(midcs));

                ajTablePut(midcs2,
                           (void *) ajStrNewS(midkey),
                           (void *) midpath);
            }

            ajStrDel(&midkey);
        }
    }

    AJFREE(keyarray);

    AJFREE(valarray);

    /*
    ** Clear the temporary AJAX Tables of AJAX String keys and AJAX List
    ** values, before deleting the Tables.
    ** Do not delete Coordinate Systems from the Lists.
    */

    ajTableToarrayKeysValues(midcs1, &keyarray, &valarray);

    for(i = 0; keyarray[i]; i++)
    {
        ajStrDel((AjPStr *) &keyarray[i]);

        while(ajListPop((AjPList) valarray[i], (void **) &midcs))
            ensCoordsystemDel(&midcs);

        ajListFree((AjPList *) &valarray[i]);
    }

    AJFREE(keyarray);
    AJFREE(valarray);

    ajTableFree(&midcs1);

    ajTableToarrayKeysValues(midcs2, &keyarray, &valarray);

    for(i = 0; keyarray[i]; i++)
    {
        ajStrDel((AjPStr *) &keyarray[i]);

        while(ajListPop((AjPList) valarray[i], (void **) &midcs))
            ensCoordsystemDel(&midcs);

        ajListFree((AjPList *) &valarray[i]);
    }

    AJFREE(keyarray);
    AJFREE(valarray);

    ajTableFree(&midcs2);

    ajStrDel(&cs1key);
    ajStrDel(&cs2key);

    return mappath;
}




/* @func ensCoordsystemadaptorGetExternalSeqregionIdentifier ******************
**
** Get the external Ensembl Sequence Region identifier for an
** internal Ensembl Sequence Region identifier.
** If the external Ensembl Sequence Region is not present in the mapping table,
** the identifier for the internal one will be returned.
**
** @param [r] adaptor [const EnsPCoordsystemadaptor] Ensembl Coordinate
**                                                   System Adaptor
** @param [r] srid [ajuint] Internal Ensembl Sequence Region identifier
**
** @return [ajuint] External Ensembl Sequence Region identifier or 0
** @@
******************************************************************************/

ajuint ensCoordsystemadaptorGetExternalSeqregionIdentifier(
    const EnsPCoordsystemadaptor adaptor,
    ajuint srid)
{
    ajuint *Pidentifier = NULL;

    if(!adaptor)
        return 0;

    if(!srid)
        return 0;

    Pidentifier = (ajuint *)
        ajTableFetch(adaptor->InternalToExternal, (const void *) &srid);

    if(Pidentifier)
        return *Pidentifier;

    return srid;
}




/* @func ensCoordsystemadaptorGetInternalSeqregionIdentifier ******************
**
** Get the internal Ensembl Sequence Region identifier for an
** external Ensembl Sequence Region identifier.
** If the internal Ensembl Sequence Region is not present in the mapping table,
** the identifier for the external one will be returned.
**
** @param [r] adaptor [const EnsPCoordsystemadaptor] Ensembl Coordinate
**                                                   System Adaptor
** @param [r] srid [ajuint] External Ensembl Sequence Region identifier
**
** @return [ajuint] Internal Ensembl Sequence Region identifier or 0
** @@
******************************************************************************/

ajuint ensCoordsystemadaptorGetInternalSeqregionIdentifier(
    const EnsPCoordsystemadaptor adaptor,
    ajuint srid)
{
    ajuint *Pidentifier = NULL;

    if(!adaptor)
        return 0;

    if(!srid)
        return 0;

    Pidentifier = (ajuint *)
        ajTableFetch(adaptor->ExternalToInternal, (const void *) &srid);

    if(Pidentifier)
        return *Pidentifier;

    return srid;
}
