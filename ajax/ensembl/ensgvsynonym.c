/* @source ensgvsynonym *******************************************************
**
** Ensembl Genetic Variation Synonym functions
**
** @author Copyright (C) 1999 Ensembl Developers
** @author Copyright (C) 2006 Michael K. Schuster
** @version $Revision: 1.21 $
** @modified 2009 by Alan Bleasby for incorporation into EMBOSS core
** @modified $Date: 2013/02/17 13:02:40 $ by $Author: mks $
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
#include "ensgvsynonym.h"




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




/* ========================================================================= */
/* =========================== private variables =========================== */
/* ========================================================================= */




/* ========================================================================= */
/* =========================== private functions =========================== */
/* ========================================================================= */




/* ========================================================================= */
/* ======================= All functions by section ======================== */
/* ========================================================================= */




/* @filesection ensgvsynonym **************************************************
**
** @nam1rule ens Function belongs to the Ensembl library
**
******************************************************************************/




/* @datasection [EnsPGvsynonym] Ensembl Genetic Variation Synonym *************
**
** @nam2rule Gvsynonym Functions for manipulating
** Ensembl Genetic Variation Synonym objects
**
** NOTE: The Ensembl Genetic Variation Synonym object has no counterpart in the
** Ensembl Genetic Variation Perl API. It has been split out of the
** Bio::EnsEMBL::Variation::Variation object to formalise entries from the
** 'variation_synonym' table.
**
** @cc Bio::EnsEMBL::Variation::Variation
** @cc CVS Revision: 1.68
** @cc CVS Tag: branch-ensembl-68
**
******************************************************************************/




/* @section constructors ******************************************************
**
** All constructors return a new Ensembl Genetic Variation Synonym by pointer.
** It is the responsibility of the user to first destroy any previous
** Genetic Variation Synonym. The target pointer does not need to be
** initialised to NULL, but it is good programming practice to do so anyway.
**
** @fdata [EnsPGvsynonym]
**
** @nam3rule New Constructor
** @nam4rule Cpy Constructor with existing object
** @nam4rule Ini Constructor with initial values
** @nam4rule Ref Constructor by incrementing the reference counter
**
** @argrule Cpy gvs [const EnsPGvsynonym] Ensembl Genetic Variation Synonym
** @argrule Ini gvsa [EnsPGvsynonymadaptor] Ensembl Genetic Variation
** @argrule Ini identifier [ajuint] SQL database-internal identifier
** @argrule Ini gvsource [EnsPGvsource] Ensembl Genetic Variation Source
** @argrule Ini name [AjPStr] Name
** @argrule Ini moleculetype [AjPStr] Molecule type
** @argrule Ini gvvidentifier [ajuint] Ensembl Genetic Variation identifier
** @argrule Ini subidentifier [ajuint] Sub identifier
** @argrule Ref gvs [EnsPGvsynonym] Ensembl Genetic Variation Synonym
**
** @valrule * [EnsPGvsynonym] Ensembl Genetic Variation Synonym or NULL
**
** @fcategory new
******************************************************************************/




/* @func ensGvsynonymNewCpy ***************************************************
**
** Object-based constructor function, which returns an independent object.
**
** @param [r] gvs [const EnsPGvsynonym] Ensembl Genetic Variation Synonym
**
** @return [EnsPGvsynonym] Ensembl Genetic Variation Synonym or NULL
**
** @release 6.4.0
** @@
******************************************************************************/

EnsPGvsynonym ensGvsynonymNewCpy(const EnsPGvsynonym gvs)
{
    EnsPGvsynonym pthis = NULL;

    if (!gvs)
        return NULL;

    AJNEW0(pthis);

    pthis->Use        = 1U;
    pthis->Identifier = gvs->Identifier;
    pthis->Adaptor    = gvs->Adaptor;
    pthis->Gvsource   = ensGvsourceNewRef(gvs->Gvsource);

    if (gvs->Name)
        pthis->Name = ajStrNewRef(gvs->Name);

    if (gvs->Moleculetype)
        pthis->Moleculetype = ajStrNewRef(gvs->Moleculetype);

    pthis->Gvvariationidentifier = gvs->Gvvariationidentifier;
    pthis->Subidentifier         = gvs->Subidentifier;

    return pthis;
}




/* @func ensGvsynonymNewIni ***************************************************
**
** Constructor for an Ensembl Genetic Variation Synonym with initial values.
**
** @cc Bio::EnsEMBL::Storable::new
** @param [u] gvsa [EnsPGvsynonymadaptor]
** Ensembl Genetic Variation Synonym Adaptor
** @param [r] identifier [ajuint] SQL database-internal identifier
** @cc Bio::EnsEMBL::Variation::*::new
** @param [u] gvsource [EnsPGvsource] Ensembl Genetic Variation Source
** @param [u] name [AjPStr] Name
** @param [u] moleculetype [AjPStr] Molecule type
** @param [r] gvvidentifier [ajuint] Ensembl Genetic Variation identifier
** @param [r] subidentifier [ajuint] Sub identifier
**
** @return [EnsPGvsynonym] Ensembl Genetic Variation Synonym or NULL
**
** @release 6.4.0
** @@
******************************************************************************/

EnsPGvsynonym ensGvsynonymNewIni(EnsPGvsynonymadaptor gvsa,
                                 ajuint identifier,
                                 EnsPGvsource gvsource,
                                 AjPStr name,
                                 AjPStr moleculetype,
                                 ajuint gvvidentifier,
                                 ajuint subidentifier)
{
    EnsPGvsynonym gvs = NULL;

    AJNEW0(gvs);

    gvs->Use        = 1U;
    gvs->Identifier = identifier;
    gvs->Adaptor    = gvsa;
    gvs->Gvsource   = ensGvsourceNewRef(gvsource);

    if (name)
        gvs->Name = ajStrNewRef(name);

    if (moleculetype)
        gvs->Moleculetype = ajStrNewRef(moleculetype);

    gvs->Gvvariationidentifier = gvvidentifier;
    gvs->Subidentifier         = subidentifier;

    return gvs;
}




/* @func ensGvsynonymNewRef ***************************************************
**
** Ensembl Object referencing function, which returns a pointer to the
** Ensembl Object passed in and increases its reference count.
**
** @param [u] gvs [EnsPGvsynonym] Ensembl Genetic Variation Synonym
**
** @return [EnsPGvsynonym] Ensembl Genetic Variation Synonym or NULL
**
** @release 6.4.0
** @@
******************************************************************************/

EnsPGvsynonym ensGvsynonymNewRef(EnsPGvsynonym gvs)
{
    if (!gvs)
        return NULL;

    gvs->Use++;

    return gvs;
}




/* @section destructors *******************************************************
**
** Destruction destroys all internal data structures and frees the memory
** allocated for an Ensembl Genetic Variation Synonym object.
**
** @fdata [EnsPGvsynonym]
**
** @nam3rule Del Destroy (free) an Ensembl Genetic Variation Synonym
**
** @argrule * Pgvs [EnsPGvsynonym*] Ensembl Genetic Variation Synonym address
**
** @valrule * [void]
**
** @fcategory delete
******************************************************************************/




/* @func ensGvsynonymDel ******************************************************
**
** Default destructor for an Ensembl Genetic Variation Synonym.
**
** @param [d] Pgvs [EnsPGvsynonym*] Ensembl Genetic Variation Synonym address
**
** @return [void]
**
** @release 6.4.0
** @@
******************************************************************************/

void ensGvsynonymDel(EnsPGvsynonym *Pgvs)
{
    EnsPGvsynonym pthis = NULL;

    if (!Pgvs)
        return;

#if defined(AJ_DEBUG) && AJ_DEBUG >= 1
    if (ajDebugTest("ensGvsynonymDel"))
    {
        ajDebug("ensGvsynonymDel\n"
                "  *Pgvs %p\n",
                *Pgvs);

        ensGvsynonymTrace(*Pgvs, 1);
    }
#endif /* defined(AJ_DEBUG) && AJ_DEBUG >= 1 */

    if (!(pthis = *Pgvs) || --pthis->Use)
    {
        *Pgvs = NULL;

        return;
    }

    ensGvsourceDel(&pthis->Gvsource);

    ajStrDel(&pthis->Name);
    ajStrDel(&pthis->Moleculetype);

    ajMemFree((void **) Pgvs);

    return;
}




/* @section member retrieval **************************************************
**
** Functions for returning members of an
** Ensembl Genetic Variation Synonym object.
**
** @fdata [EnsPGvsynonym]
**
** @nam3rule Get Return Genetic Variation Synonym attribute(s)
** @nam4rule Adaptor Return the Ensembl Genetic Variation Synonym Adaptor
** @nam4rule Gvsource Return the Ensembl Genetic Variation Source
** @nam4rule Gvvariationidentifier
** Return the Ensembl Genetic Variation identifier
** @nam4rule Identifier Return the SQL database-internal identifier
** @nam4rule Moleculetype Return the molecule type
** @nam4rule Name Return the name
** @nam4rule Subidentifier Return the sub-identifier
**
** @argrule * gvs [const EnsPGvsynonym] Genetic Variation Synonym
**
** @valrule Adaptor [EnsPGvsynonymadaptor]
** Ensembl Genetic Variation Synonym Adaptor or NULL
** @valrule Gvsource [EnsPGvsource] Ensembl Genetic Variation Source or NULL
** @valrule Gvvariationidentifier [ajuint]
** Ensembl Genetic Variation identifier or 0U
** @valrule Identifier [ajuint] SQL database-internal identifier or 0U
** @valrule Moleculetype [AjPStr] Molecule type or NULL
** @valrule Name [AjPStr] Name or NULL
** @valrule Subidentifier [ajuint] Sub-identifier or 0U
**
** @fcategory use
******************************************************************************/




/* @func ensGvsynonymGetAdaptor ***********************************************
**
** Get the Ensembl Genetic Variation Synonym Adaptor member of an
** Ensembl Genetic Variation Synonym.
**
** @param [r] gvs [const EnsPGvsynonym] Ensembl Genetic Variation Synonym
**
** @return [EnsPGvsynonymadaptor] Ensembl Genetic Variation Synonym Adaptor
** or NULL
**
** @release 6.4.0
** @@
******************************************************************************/

EnsPGvsynonymadaptor ensGvsynonymGetAdaptor(const EnsPGvsynonym gvs)
{
    return (gvs) ? gvs->Adaptor : NULL;
}




/* @func ensGvsynonymGetGvsource **********************************************
**
** Get the Ensembl Genetic Variation Source member of an
** Ensembl Genetic Variation Synonym.
**
** @param [r] gvs [const EnsPGvsynonym] Ensembl Genetic Variation Synonym
**
** @return [EnsPGvsource] Ensembl Genetic Variation Source or NULL
**
** @release 6.4.0
** @@
******************************************************************************/

EnsPGvsource ensGvsynonymGetGvsource(const EnsPGvsynonym gvs)
{
    return (gvs) ? gvs->Gvsource : NULL;
}





/* @func ensGvsynonymGetGvvariationidentifier *********************************
**
** Get the Ensembl Genetic Variation identifier member of an
** Ensembl Genetic Variation Synonym.
**
** @param [r] gvs [const EnsPGvsynonym] Ensembl Genetic Variation Synonym
**
** @return [ajuint] Ensembl Genetic Variation identifier or 0U
**
** @release 6.4.0
** @@
******************************************************************************/

ajuint ensGvsynonymGetGvvariationidentifier(const EnsPGvsynonym gvs)
{
    return (gvs) ? gvs->Gvvariationidentifier : 0U;
}





/* @func ensGvsynonymGetIdentifier ********************************************
**
** Get the SQL database-internal identifier member of an
** Ensembl Genetic Variation Synonym.
**
** @param [r] gvs [const EnsPGvsynonym] Ensembl Genetic Variation Synonym
**
** @return [ajuint] SQL database-internal identifier or 0U
**
** @release 6.4.0
** @@
******************************************************************************/

ajuint ensGvsynonymGetIdentifier(const EnsPGvsynonym gvs)
{
    return (gvs) ? gvs->Identifier : 0U;
}





/* @func ensGvsynonymGetMoleculetype ******************************************
**
** Get the molecule type member of an Ensembl Genetic Variation Synonym.
**
** @param [r] gvs [const EnsPGvsynonym] Ensembl Genetic Variation Synonym
**
** @return [AjPStr] Molecule type or NULL
**
** @release 6.4.0
** @@
******************************************************************************/

AjPStr ensGvsynonymGetMoleculetype(const EnsPGvsynonym gvs)
{
    return (gvs) ? gvs->Moleculetype : NULL;
}





/* @func ensGvsynonymGetName **************************************************
**
** Get the name member of an Ensembl Genetic Variation Synonym.
**
** @param [r] gvs [const EnsPGvsynonym] Ensembl Genetic Variation Synonym
**
** @return [AjPStr] Name or NULL
**
** @release 6.4.0
** @@
******************************************************************************/

AjPStr ensGvsynonymGetName(const EnsPGvsynonym gvs)
{
    return (gvs) ? gvs->Name : NULL;
}





/* @func ensGvsynonymGetSubidentifier *****************************************
**
** Get the Sub-identifier member of an Ensembl Genetic Variation Synonym.
**
** @param [r] gvs [const EnsPGvsynonym] Ensembl Genetic Variation Synonym
**
** @return [ajuint] Sub-identifier or 0U
**
** @release 6.4.0
** @@
******************************************************************************/

ajuint ensGvsynonymGetSubidentifier(const EnsPGvsynonym gvs)
{
    return (gvs) ? gvs->Subidentifier : 0U;
}





/* @section member assignment *************************************************
**
** Functions for assigning members of an
** Ensembl Genetic Variation Synonym object.
**
** @fdata [EnsPGvsynonym]
**
** @nam3rule Set Set one member of a Genetic Variation Synonym
** @nam4rule Adaptor Set the Ensembl Genetic Variation Synonym Adaptor
** @nam4rule Gvsource Set the Ensembl Genetic Variation Source
** @nam4rule Gvvariationidentifier Set the Ensembl Genetic Variation
**                                    identifier
** @nam4rule Identifier Set the SQL database-internal identifier
** @nam4rule Moleculetype Set the molecule type
** @nam4rule Name Set the name
** @nam4rule SetSubidentifier Set the sub-identifier
**
** @argrule * gvs [EnsPGvsynonym] Ensembl Genetic Variation Synonym object
** @argrule Adaptor gvsa [EnsPGvsynonymadaptor] Ensembl Genetic Variation
** @argrule Gvsource gvsource [EnsPGvsource] Ensembl Genetic Variation Source
** @argrule Gvvariationidentifier gvvidentifier [ajuint] Ensembl Genetic
** Variation identifier
** @argrule Identifier identifier [ajuint] SQL database-internal identifier
** @argrule Moleculetype moleculetype [AjPStr] Molecule type
** @argrule Name name [AjPStr] Name
** @argrule Subidentifier subidentifier [ajuint] Sub-identifier
**
** @valrule * [AjBool] ajTrue upon success, ajFalse otherwise
**
** @fcategory modify
******************************************************************************/




/* @func ensGvsynonymSetAdaptor ***********************************************
**
** Set the Ensembl Genetic Variation Synonym Adaptor member of an
** Ensembl Genetic Variation Synonym.
**
** @param [u] gvs [EnsPGvsynonym] Ensembl Genetic Variation Synonym
** @param [u] gvsa [EnsPGvsynonymadaptor] Ensembl Genetic Variation
**                                        Synonym Adaptor
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
**
** @release 6.4.0
** @@
******************************************************************************/

AjBool ensGvsynonymSetAdaptor(EnsPGvsynonym gvs,
                              EnsPGvsynonymadaptor gvsa)
{
    if (!gvs)
        return ajFalse;

    gvs->Adaptor = gvsa;

    return ajTrue;
}




/* @func ensGvsynonymSetGvsource **********************************************
**
** Set the Ensembl Genetic Variation Source member of an
** Ensembl Genetic Variation Synonym.
**
** @param [u] gvs [EnsPGvsynonym] Ensembl Genetic Variation Synonym
** @param [u] gvsource [EnsPGvsource] Ensembl Genetic Variation Source
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
**
** @release 6.4.0
** @@
******************************************************************************/

AjBool ensGvsynonymSetGvsource(EnsPGvsynonym gvs,
                               EnsPGvsource gvsource)
{
    if (!gvs)
        return ajFalse;

    ensGvsourceDel(&gvs->Gvsource);

    gvs->Gvsource = ensGvsourceNewRef(gvsource);

    return ajTrue;
}




/* @func ensGvsynonymSetGvvariationidentifier *********************************
**
** Set the Ensembl Genetic Variation identifier member of an
** Ensembl Genetic Variation Synonym.
**
** @param [u] gvs [EnsPGvsynonym] Ensembl Genetic Variation Synonym
** @param [r] gvvidentifier [ajuint] Ensembl Genetic Variation identifier
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
**
** @release 6.4.0
** @@
******************************************************************************/

AjBool ensGvsynonymSetGvvariationidentifier(EnsPGvsynonym gvs,
                                            ajuint gvvidentifier)
{
    if (!gvs)
        return ajFalse;

    gvs->Gvvariationidentifier = gvvidentifier;

    return ajTrue;
}




/* @func ensGvsynonymSetIdentifier ********************************************
**
** Set the SQL database-internal identifier member of an
** Ensembl Genetic Variation Synonym.
**
** @param [u] gvs [EnsPGvsynonym] Ensembl Genetic Variation Synonym
** @param [r] identifier [ajuint] SQL database-internal identifier
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
**
** @release 6.4.0
** @@
******************************************************************************/

AjBool ensGvsynonymSetIdentifier(EnsPGvsynonym gvs,
                                 ajuint identifier)
{
    if (!gvs)
        return ajFalse;

    gvs->Identifier = identifier;

    return ajTrue;
}




/* @func ensGvsynonymSetMoleculetype ******************************************
**
** Set the molecule type member of an Ensembl Genetic Variation Synonym.
**
** @param [u] gvs [EnsPGvsynonym] Ensembl Genetic Variation Synonym
** @param [u] moleculetype [AjPStr] Molecule type
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
**
** @release 6.4.0
** @@
******************************************************************************/

AjBool ensGvsynonymSetMoleculetype(EnsPGvsynonym gvs,
                                   AjPStr moleculetype)
{
    if (!gvs)
        return ajFalse;

    ajStrDel(&gvs->Moleculetype);

    if (moleculetype)
        gvs->Moleculetype = ajStrNewRef(moleculetype);

    return ajTrue;
}




/* @func ensGvsynonymSetName **************************************************
**
** Set the name member of an Ensembl Genetic Variation Synonym.
**
** @param [u] gvs [EnsPGvsynonym] Ensembl Genetic Variation Synonym
** @param [u] name [AjPStr] Name
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
**
** @release 6.4.0
** @@
******************************************************************************/

AjBool ensGvsynonymSetName(EnsPGvsynonym gvs,
                           AjPStr name)
{
    if (!gvs)
        return ajFalse;

    ajStrDel(&gvs->Name);

    if (name)
        gvs->Name = ajStrNewRef(name);

    return ajTrue;
}




/* @func ensGvsynonymSetSubidentifier *****************************************
**
** Set the sub-identifier member of an Ensembl Genetic Variation Synonym.
**
** @param [u] gvs [EnsPGvsynonym] Ensembl Genetic Variation Synonym
** @param [r] subidentifier [ajuint] Sub-identifier
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
**
** @release 6.4.0
** @@
******************************************************************************/

AjBool ensGvsynonymSetSubidentifier(EnsPGvsynonym gvs,
                                    ajuint subidentifier)
{
    if (!gvs)
        return ajFalse;

    gvs->Subidentifier = subidentifier;

    return ajTrue;
}




/* @section debugging *********************************************************
**
** Functions for reporting of an Ensembl Genetic Variation Synonym object.
**
** @fdata [EnsPGvsynonym]
**
** @nam3rule Trace Report Ensembl Genetic Variation Synonym members to
**                 debug file
**
** @argrule Trace gvs [const EnsPGvsynonym] Ensembl Genetic Variation Synonym
** @argrule Trace level [ajuint] Indentation level
**
** @valrule * [AjBool] ajTrue upon success, ajFalse otherwise
**
** @fcategory misc
******************************************************************************/




/* @func ensGvsynonymTrace ****************************************************
**
** Trace an Ensembl Genetic Variation Synonym.
**
** @param [r] gvs [const EnsPGvsynonym] Ensembl Genetic Variation Synonym
** @param [r] level [ajuint] Indentation level
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
**
** @release 6.4.0
** @@
******************************************************************************/

AjBool ensGvsynonymTrace(const EnsPGvsynonym gvs, ajuint level)
{
    AjPStr indent = NULL;

    if (!gvs)
        return ajFalse;

    indent = ajStrNew();

    ajStrAppendCountK(&indent, ' ', level * 2);

    ajDebug("%SensGvsynonymTrace %p\n"
            "%S  Use %u\n"
            "%S  Identifier %u\n"
            "%S  Adaptor %p\n"
            "%S  Gvsource %p\n"
            "%S  Name '%S'\n"
            "%S  Moleculetype '%S'\n"
            "%S  Gvvariationidentifier %u\n"
            "%S  Subidentifier %u\n",
            indent, gvs,
            indent, gvs->Use,
            indent, gvs->Identifier,
            indent, gvs->Adaptor,
            indent, gvs->Gvsource,
            indent, gvs->Name,
            indent, gvs->Moleculetype,
            indent, gvs->Gvvariationidentifier,
            indent, gvs->Subidentifier);

    ensGvsourceTrace(gvs->Gvsource, level  + 1);

    ajStrDel(&indent);

    return ajTrue;
}




/* @section calculate *********************************************************
**
** Functions for calculating information from an
** Ensembl Genetic Variation Synonym object.
**
** @fdata [EnsPGvsynonym]
**
** @nam3rule Calculate Calculate Ensembl Genetic Variation Synonym information
** @nam4rule Memsize Calculate the memory size in bytes
**
** @argrule * gvs [const EnsPGvsynonym] Ensembl Genetic Variation Synonym
**
** @valrule Memsize [size_t] Memory size in bytes or 0
**
** @fcategory misc
******************************************************************************/




/* @func ensGvsynonymCalculateMemsize *****************************************
**
** Get the memory size in bytes of an Ensembl Genetic Variation Synonym.
**
** @param [r] gvs [const EnsPGvsynonym] Ensembl Genetic Variation Synonym
**
** @return [size_t] Memory size in bytes or 0
**
** @release 6.4.0
** @@
******************************************************************************/

size_t ensGvsynonymCalculateMemsize(const EnsPGvsynonym gvs)
{
    size_t size = 0;

    if (!gvs)
        return 0;

    size += sizeof (EnsOGvsynonym);

    size += ensGvsourceCalculateMemsize(gvs->Gvsource);

    if (gvs->Name)
    {
        size += sizeof (AjOStr);

        size += ajStrGetRes(gvs->Name);
    }

    if (gvs->Moleculetype)
    {
        size += sizeof (AjOStr);

        size += ajStrGetRes(gvs->Moleculetype);
    }

    return size;
}
