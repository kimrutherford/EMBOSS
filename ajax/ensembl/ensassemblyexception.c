/* @source ensassemblyexception ***********************************************
**
** Ensembl Assembly Exception functions
**
** @author Copyright (C) 1999 Ensembl Developers
** @author Copyright (C) 2006 Michael K. Schuster
** @version $Revision: 1.50 $
** @modified 2009 by Alan Bleasby for incorporation into EMBOSS core
** @modified $Date: 2012/04/12 20:34:16 $ by $Author: mks $
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

#include "ensassemblyexception.h"
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

/* @conststatic assemblyexceptionKType ****************************************
**
** The Ensembl Assembly Exception type is enumerated in both, the SQL table
** definition and the data structure. The following strings are used for
** conversion in database operations and correspond to
** EnsEAssemblyexceptionType.
**
** HAP:         Haplotype
** PAR:         Pseudo-autosomal Region
** PATCH_FIX:   Genome sequence patch with updated sequence
** PATCH_NOVEL: Genome sequence patch with new sequence
**
******************************************************************************/

static const char *assemblyexceptionKType[] =
{
    "",
    "HAP",
    "PAR",
    "PATCH_FIX",
    "PATCH_NOVEL",
    "HAP REF",
    "PATCH_FIX REF",
    "PATCH_NOVEL REF",
    (const char *) NULL
};




/* ========================================================================= */
/* =========================== private variables =========================== */
/* ========================================================================= */




/* ========================================================================= */
/* =========================== private functions =========================== */
/* ========================================================================= */

static int listAssemblyexceptionCompareReferenceEndAscending(
    const void *item1,
    const void *item2);

static int listAssemblyexceptionCompareReferenceEndDescending(
    const void *item1,
    const void *item2);

static AjBool assemblyexceptionadaptorFetchAllbyStatement(
    EnsPAssemblyexceptionadaptor aea,
    const AjPStr statement,
    AjPList aes);

static AjBool assemblyexceptionadaptorCacheInit(
    EnsPAssemblyexceptionadaptor aea);

static void assemblyexceptionadaptorListAssemblyexceptionValdel(void **Pvalue);

static void assemblyexceptionadaptorFetchAll(const void *key,
                                             void **Pvalue,
                                             void *cl);




/* ========================================================================= */
/* ======================= All functions by section ======================== */
/* ========================================================================= */




/* @filesection ensassemblyexception ******************************************
**
** @nam1rule ens Function belongs to the Ensembl library
**
******************************************************************************/




/* @datasection [EnsPAssemblyexception] Ensembl Assembly Exception ************
**
** @nam2rule Assemblyexception Functions for manipulating
** Ensembl Assembly Exception objects
**
** The Ensembl Assembly Exception class has no counterpart in the Perl API,
** it has been split out of the Bio::EnsEMBL::DBSQL::SliceAdaptor class.
**
** @cc Bio::EnsEMBL::Slice
** @cc CVS Revision: 1.292
** @cc CVS Tag: branch-ensembl-66
**
** @cc Bio::EnsEMBL::DBSQL::SliceAdaptor
** @cc CVS Revision: 1.126
** @cc CVS Tag: branch-ensembl-66
**
******************************************************************************/




/* @section constructors ******************************************************
**
** All constructors return a new Ensembl Assembly Exception by pointer.
** It is the responsibility of the user to first destroy any previous
** Assembly Exception. The target pointer does not need to be initialised to
** NULL, but it is good programming practice to do so anyway.
**
** @fdata [EnsPAssemblyexception]
**
** @nam3rule New Constructor
** @nam4rule Cpy Constructor with existing object
** @nam4rule Ini Constructor with initial values
** @nam4rule NewRef Constructor by incrementing the reference counter
**
** @argrule Cpy ae [const EnsPAssemblyexception] Ensembl Assembly Exception
** @argrule Ini aea [EnsPAssemblyexceptionadaptor] Ensembl Assembly
** @argrule Ini identifier [ajuint] SQL database-internal identifier
** @argrule Ini srid [ajuint] Reference Ensembl Sequence Region identifier
** @argrule Ini srstart [ajuint] Reference Ensembl Sequence Region start
** @argrule Ini srend [ajuint] Reference Ensembl Sequence Region end
** @argrule Ini erid [ajuint] Exception Ensembl Sequence Region identifier
** @argrule Ini erstart [ajuint] Exception Ensembl Sequence Region start
** @argrule Ini erend [ajuint] Exception Ensembl Sequence Region end
** @argrule Ini ori [ajint] Orientation
** @argrule Ini aet [EnsEAssemblyexceptionType]
** Ensembl Assembly Exception Type enumeration
** @argrule Ref ae [EnsPAssemblyexception] Ensembl Assembly Exception
**
** @valrule * [EnsPAssemblyexception] Ensembl Assembly Exception or NULL
**
** @fcategory new
******************************************************************************/




/* @func ensAssemblyexceptionNewCpy *******************************************
**
** Object-based constructor function, which returns an independent object.
**
** @param [r] ae [const EnsPAssemblyexception] Ensembl Assembly Exception
**
** @return [EnsPAssemblyexception] Ensembl Assembly Exception or NULL
**
** @release 6.4.0
** @@
******************************************************************************/

EnsPAssemblyexception ensAssemblyexceptionNewCpy(
    const EnsPAssemblyexception ae)
{
    EnsPAssemblyexception pthis = NULL;

    if (!ae)
        return NULL;

    AJNEW0(pthis);

    pthis->Use = 1U;

    pthis->Identifier         = ae->Identifier;
    pthis->Adaptor            = ae->Adaptor;
    pthis->ReferenceSeqregion = ae->ReferenceSeqregion;
    pthis->ReferenceStart     = ae->ReferenceStart;
    pthis->ReferenceEnd       = ae->ReferenceEnd;
    pthis->ExceptionSeqregion = ae->ExceptionSeqregion;
    pthis->ExceptionStart     = ae->ExceptionStart;
    pthis->ExceptionEnd       = ae->ExceptionEnd;
    pthis->Orientation        = ae->Orientation;
    pthis->Type               = ae->Type;

    return pthis;
}




/* @func ensAssemblyexceptionNewIni *******************************************
**
** Constructor for an Ensembl Assembly Exception with initial values.
**
** @param [u] aea [EnsPAssemblyexceptionadaptor]
** Ensembl Assembly Exception Adaptor
** @param [r] identifier [ajuint] SQL database-internal identifier
** @param [r] srid [ajuint] Reference Ensembl Sequence Region identifier
** @param [r] srstart [ajuint] Reference Ensembl Sequence Region start
** @param [r] srend [ajuint] Reference Ensembl Sequence Region end
** @param [r] erid [ajuint] Exception Ensembl Sequence Region identifier
** @param [r] erstart [ajuint] Exception Ensembl Sequence Region start
** @param [r] erend [ajuint] Exception Ensembl Sequence Region end
** @param [r] ori [ajint] Orientation
** @param [u] aet [EnsEAssemblyexceptionType]
** Ensembl Assembly Exception Type enumeration
**
** @return [EnsPAssemblyexception] Ensembl Assembl Exception or NULL
**
** @release 6.4.0
** @@
******************************************************************************/

EnsPAssemblyexception ensAssemblyexceptionNewIni(
    EnsPAssemblyexceptionadaptor aea,
    ajuint identifier,
    ajuint srid,
    ajuint srstart,
    ajuint srend,
    ajuint erid,
    ajuint erstart,
    ajuint erend,
    ajint ori,
    EnsEAssemblyexceptionType aet)
{
    EnsPAssemblyexception ae = NULL;

    if (!srid)
        return NULL;

    if (!erid)
        return NULL;

    AJNEW0(ae);

    ae->Use = 1U;

    ae->Identifier         = identifier;
    ae->Adaptor            = aea;
    ae->ReferenceSeqregion = srid;
    ae->ReferenceStart     = srstart;
    ae->ReferenceEnd       = srend;
    ae->ExceptionSeqregion = erid;
    ae->ExceptionStart     = erstart;
    ae->ExceptionEnd       = erend;
    ae->Orientation        = ori;
    ae->Type               = aet;

    return ae;
}




/* @func ensAssemblyexceptionNewRef *******************************************
**
** Ensembl Object referencing function, which returns a pointer to the
** Ensembl Object passed in and increases its reference count.
**
** @param [u] ae [EnsPAssemblyexception] Ensembl Assembly Exception
**
** @return [EnsPAssemblyexception] Ensembl Assembly Exception or NULL
**
** @release 6.2.0
** @@
******************************************************************************/

EnsPAssemblyexception ensAssemblyexceptionNewRef(EnsPAssemblyexception ae)
{
    if (!ae)
        return NULL;

    ae->Use++;

    return ae;
}




/* @section destructors *******************************************************
**
** Destruction destroys all internal data structures and frees the memory
** allocated for an Ensembl Assembly Exception object.
**
** @fdata [EnsPAssemblyexception]
**
** @nam3rule Del Destroy (free) an Ensembl Assembly Exception object
**
** @argrule * Pae [EnsPAssemblyexception*]
** Ensembl Assembly Exception object address
**
** @valrule * [void]
**
** @fcategory delete
******************************************************************************/




/* @func ensAssemblyexceptionDel **********************************************
**
** Default destructor for an Ensembl Assembly Exception.
**
** @param [d] Pae [EnsPAssemblyexception*] Ensembl Assembly Exception
** object address
**
** @return [void]
**
** @release 6.2.0
** @@
******************************************************************************/

void ensAssemblyexceptionDel(EnsPAssemblyexception *Pae)
{
    EnsPAssemblyexception pthis = NULL;

    if (!Pae)
        return;

#if defined(AJ_DEBUG) && AJ_DEBUG >= 1
    if (ajDebugTest("ensAssemblyexceptionDel"))
    {
        ajDebug("ensAssemblyexceptionDel\n"
                "  *Pae %p\n",
                *Pae);

        ensAssemblyexceptionTrace(*Pae, 1);
    }
#endif /* defined(AJ_DEBUG) && AJ_DEBUG >= 1 */

    if (!*Pae)
        return;

    pthis = *Pae;

    pthis->Use--;

    if (pthis->Use)
    {
        *Pae = NULL;

        return;
    }

    AJFREE(pthis);

    *Pae = NULL;

    return;
}




/* @section member retrieval **************************************************
**
** Functions for returning members of an Ensembl Assembly Exception object.
**
** @fdata [EnsPAssemblyexception]
**
** @nam3rule Get Return Assembly Exception attribute(s)
** @nam4rule Adaptor Return the Ensembl Assembly Excepton Adaptor
** @nam4rule Exception Return exception Ensembl Sequence Region attribute(s)
** @nam5rule End Return the exception Ensembl Sequence Region end
** @nam5rule Seqregion Return the exception Ensembl Sequence Region identifier
** @nam5rule Start Return the exception Ensembl Sequence Region start
** @nam4rule Identifier Return the SQL database-internal identifier
** @nam4rule Reference Return reference Ensembl Sequence Region attribute(s)
** @nam5rule End Return the reference Ensembl Sequence Region end
** @nam5rule Seqregion Return the reference Ensembl Sequence Region identifier
** @nam5rule Start Return the reference Ensembl Sequence Region start
** @nam4rule Orientation Return the orientation
** @nam4rule Type Return the type
**
** @argrule * ae [const EnsPAssemblyexception] Ensembl Assembly Exception
**
** @valrule Adaptor [EnsPAssemblyexceptionadaptor]
** Ensembl Assembly Exception Adaptor or NULL
** @valrule End [ajuint] Ensembl Sequence Region end or 0U
** @valrule Seqregion [ajuint] Ensembl Sequence Region identifier or 0U
** @valrule Start [ajuint] Ensembl Sequence Region start or 0U
** @valrule Identifier [ajuint] SQL database-internal identifier or 0U
** @valrule Orientation [ajint] Orientation or 0
** @valrule Type [EnsEAssemblyexceptionType] Type or
** ensEAssemblyexceptionTypeNULL
**
** @fcategory use
******************************************************************************/




/* @func ensAssemblyexceptionGetAdaptor ***************************************
**
** Get the Ensembl Assembly Exception Adaptor member of an
** Ensembl Assembly Exception.
**
** @param [r] ae [const EnsPAssemblyexception] Ensembl Assembly Exception
**
** @return [EnsPAssemblyexceptionadaptor]
** Ensembl Assembly Exception Adaptor or NULL
**
** @release 6.2.0
** @@
******************************************************************************/

EnsPAssemblyexceptionadaptor ensAssemblyexceptionGetAdaptor(
    const EnsPAssemblyexception ae)
{
    return (ae) ? ae->Adaptor : NULL;
}




/* @func ensAssemblyexceptionGetExceptionEnd **********************************
**
** Get the exception Ensembl Sequence Region end member of an
** Ensembl Assembly Exception.
**
** @param [r] ae [const EnsPAssemblyexception] Ensembl Assembly Exception
**
** @return [ajuint] Exception Ensembl Sequence Region end or 0U
**
** @release 6.4.0
** @@
******************************************************************************/

ajuint ensAssemblyexceptionGetExceptionEnd(
    const EnsPAssemblyexception ae)
{
    return (ae) ? ae->ExceptionEnd : 0U;
}




/* @func ensAssemblyexceptionGetExceptionSeqregion ****************************
**
** Get the exception Ensembl Sequence Region identifier member of an
** Ensembl Assembly Exception.
**
** @param [r] ae [const EnsPAssemblyexception] Ensembl Assembly Exception
**
** @return [ajuint] Exception Ensembl Sequence Region identifier or 0U
**
** @release 6.4.0
** @@
******************************************************************************/

ajuint ensAssemblyexceptionGetExceptionSeqregion(
    const EnsPAssemblyexception ae)
{
    return (ae) ? ae->ExceptionSeqregion : 0U;
}




/* @func ensAssemblyexceptionGetExceptionStart ********************************
**
** Get the exception Ensembl Sequence Region start member of an
** Ensembl Assembly Exception.
**
** @param [r] ae [const EnsPAssemblyexception] Ensembl Assembly Exception
**
** @return [ajuint] Exception Ensembl Sequence Region start or 0U
**
** @release 6.4.0
** @@
******************************************************************************/

ajuint ensAssemblyexceptionGetExceptionStart(
    const EnsPAssemblyexception ae)
{
    return (ae) ? ae->ExceptionStart : 0U;
}




/* @func ensAssemblyexceptionGetIdentifier ************************************
**
** Get the SQL database-internal identifier member of an
** Ensembl Assembly Exception.
**
** @param [r] ae [const EnsPAssemblyexception] Ensembl Assembly Exception
**
** @return [ajuint] SQL database-internal identifier or 0U
**
** @release 6.2.0
** @@
******************************************************************************/

ajuint ensAssemblyexceptionGetIdentifier(
    const EnsPAssemblyexception ae)
{
    return (ae) ? ae->Identifier : 0U;
}




/* @func ensAssemblyexceptionGetOrientation ***********************************
**
** Get the orientation member of an Ensembl Assembly Exception.
**
** @param [r] ae [const EnsPAssemblyexception] Ensembl Assembly Exception
**
** @return [ajint] Orientation or 0
**
** @release 6.2.0
** @@
******************************************************************************/

ajint ensAssemblyexceptionGetOrientation(
    const EnsPAssemblyexception ae)
{
    return (ae) ? ae->Orientation : 0;
}




/* @func ensAssemblyexceptionGetReferenceEnd **********************************
**
** Get the reference Ensembl Sequence Region end member of an
** Ensembl Assembly Exception.
**
** @param [r] ae [const EnsPAssemblyexception] Ensembl Assembly Exception
**
** @return [ajuint] Reference Ensembl Sequence Region end or 0U
**
** @release 6.4.0
** @@
******************************************************************************/

ajuint ensAssemblyexceptionGetReferenceEnd(
    const EnsPAssemblyexception ae)
{
    return (ae) ? ae->ReferenceEnd : 0U;
}




/* @func ensAssemblyexceptionGetReferenceSeqregion ****************************
**
** Get the reference Ensembl Sequence Region identifier member of an
** Ensembl Assembly Exception.
**
** @param [r] ae [const EnsPAssemblyexception] Ensembl Assembly Exception
**
** @return [ajuint] Reference Ensembl Sequence Region identifier or 0U
**
** @release 6.4.0
** @@
******************************************************************************/

ajuint ensAssemblyexceptionGetReferenceSeqregion(
    const EnsPAssemblyexception ae)
{
    return (ae) ? ae->ReferenceSeqregion : 0U;
}




/* @func ensAssemblyexceptionGetReferenceStart ********************************
**
** Get the reference Ensembl Sequence Region start member of an
** Ensembl Assembly Exception.
**
** @param [r] ae [const EnsPAssemblyexception] Ensembl Assembly Exception
**
** @return [ajuint] Reference Ensembl Sequence Region start or 0U
**
** @release 6.4.0
** @@
******************************************************************************/

ajuint ensAssemblyexceptionGetReferenceStart(
    const EnsPAssemblyexception ae)
{
    return (ae) ? ae->ReferenceStart : 0U;
}




/* @func ensAssemblyexceptionGetType ******************************************
**
** Get the type member of an Ensembl Assembly Exception.
**
** @param [r] ae [const EnsPAssemblyexception] Ensembl Assembly Exception
**
** @return [EnsEAssemblyexceptionType] Type or ensEAssemblyexceptionTypeNULL
**
** @release 6.2.0
** @@
******************************************************************************/

EnsEAssemblyexceptionType ensAssemblyexceptionGetType(
    const EnsPAssemblyexception ae)
{
    return (ae) ? ae->Type : ensEAssemblyexceptionTypeNULL;
}




/* @section member assignment *************************************************
**
** Functions for assigning members of an Ensembl Assembly Exception object.
**
** @fdata [EnsPAssemblyexception]
**
** @nam3rule Set Set one member of an Assembly Exception
** @nam4rule Adaptor Set the Ensembl Assembly Excepton Adaptor
** @nam4rule Identifier Set the SQL database-internal identifier
** @nam4rule Reference Set reference Ensembl Sequence Region attribute(s)
** @nam5rule End Set the reference Ensembl Sequence Region end
** @nam5rule Seqregion Set thereference Ensembl Sequence Region identifier
** @nam5rule Start Set the reference Ensembl Sequence Region start
** @nam4rule Exception Set exception Ensembl Sequence Region attribute(s)
** @nam5rule End Set the exception Ensembl Sequence Region end
** @nam5rule Seqregion Set the exception Ensembl Sequence Region identifier
** @nam5rule Start Set the exception Ensembl Sequence Region start
** @nam4rule Orientation Set the orientation
** @nam4rule Type Set the type
**
** @argrule * ae [EnsPAssemblyexception] Assembly Exception
** @argrule Adaptor aea [EnsPAssemblyexceptionadaptor]
** Ensembl Assembly Exception Adaptor
** @argrule ExceptionEnd erend [ajuint] Ensembl Sequence Region end
** @argrule ExceptionSeqregion erid [ajuint] Ensembl Sequence Region identifier
** @argrule ExceptionStart erstart [ajuint] Ensembl Sequence Region start
** @argrule Identifier identifier [ajuint] SQL database-internal identifier
** @argrule Orientation orientation [ajint] Orientation
** @argrule ReferenceEnd srend [ajuint] Ensembl Sequence Region end
** @argrule ReferenceSeqregion srid [ajuint] Ensembl Sequence Region identifier
** @argrule ReferenceStart srstart [ajuint] Ensembl Sequence Region start
** @argrule Type aet [EnsEAssemblyexceptionType]
** Ensembl Assembly Exception Type enumeration
**
** @valrule * [AjBool] ajTrue upon success, ajFalse otherwise
**
** @fcategory modify
******************************************************************************/




/* @func ensAssemblyexceptionSetAdaptor ***************************************
**
** Set the Ensembl Assembly Exception Adaptor member of an
** Ensembl Assembly Exception.
**
** @param [u] ae [EnsPAssemblyexception] Ensembl Assembly Exception
** @param [u] aea [EnsPAssemblyexceptionadaptor]
** Ensembl Assembly Exception Adaptor
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
**
** @release 6.2.0
** @@
******************************************************************************/

AjBool ensAssemblyexceptionSetAdaptor(EnsPAssemblyexception ae,
                                      EnsPAssemblyexceptionadaptor aea)
{
    if (!ae)
        return ajFalse;

    ae->Adaptor = aea;

    return ajTrue;
}




/* @func ensAssemblyexceptionSetExceptionEnd **********************************
**
** Set the exception Ensembl Sequence Region end member of an
** Ensembl Assembly Exception.
**
** @param [u] ae [EnsPAssemblyexception] Ensembl Assembly Exception
** @param [r] erend [ajuint] Exception Ensembl Sequence Region end
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
**
** @release 6.4.0
** @@
******************************************************************************/

AjBool ensAssemblyexceptionSetExceptionEnd(EnsPAssemblyexception ae,
                                           ajuint erend)
{
    if (!ae)
        return ajFalse;

    ae->ExceptionEnd = erend;

    return ajTrue;
}




/* @func ensAssemblyexceptionSetExceptionSeqregion ****************************
**
** Set the exception Ensembl Sequence Region identifier member of an
** Ensembl Assembly Exception.
**
** @param [u] ae [EnsPAssemblyexception] Ensembl Assembly Exception
** @param [r] erid [ajuint] Exception Ensembl Sequence Region identifier
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
**
** @release 6.4.0
** @@
******************************************************************************/

AjBool ensAssemblyexceptionSetExceptionSeqregion(EnsPAssemblyexception ae,
                                                 ajuint erid)
{
    if (!ae)
        return ajFalse;

    ae->ExceptionSeqregion = erid;

    return ajTrue;
}




/* @func ensAssemblyexceptionSetExceptionStart ********************************
**
** Set the exception Ensembl Sequence Region start member of an
** Ensembl Assembly Exception.
**
** @param [u] ae [EnsPAssemblyexception] Ensembl Assembly Exception
** @param [r] erstart [ajuint] Exception Ensembl Sequence Region start
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
**
** @release 6.4.0
** @@
******************************************************************************/

AjBool ensAssemblyexceptionSetExceptionStart(EnsPAssemblyexception ae,
                                             ajuint erstart)
{
    if (!ae)
        return ajFalse;

    ae->ExceptionStart = erstart;

    return ajTrue;
}




/* @func ensAssemblyexceptionSetIdentifier ************************************
**
** Set the SQL database identifier member of an Ensembl Assembly Exception.
**
** @param [u] ae [EnsPAssemblyexception] Ensembl Assembly Exception
** @param [r] identifier [ajuint] SQL database-internal identifier
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
**
** @release 6.3.0
** @@
******************************************************************************/

AjBool ensAssemblyexceptionSetIdentifier(EnsPAssemblyexception ae,
                                         ajuint identifier)
{
    if (!ae)
        return ajFalse;

    ae->Identifier = identifier;

    return ajTrue;
}




/* @func ensAssemblyexceptionSetOrientation ***********************************
**
** Set the orientation member of an Ensembl Assembly Exception.
**
** @param [u] ae [EnsPAssemblyexception] Ensembl Assembly Exception
** @param [r] orientation [ajint] Orientation
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
**
** @release 6.2.0
** @@
******************************************************************************/

AjBool ensAssemblyexceptionSetOrientation(EnsPAssemblyexception ae,
                                          ajint orientation)
{
    if (!ae)
        return ajFalse;

    ae->Orientation = orientation;

    return ajTrue;
}




/* @func ensAssemblyexceptionSetReferenceEnd **********************************
**
** Set the reference Ensembl Sequence Region end member of an
** Ensembl Assembly Exception.
**
** @param [u] ae [EnsPAssemblyexception] Ensembl Assembly Exception
** @param [r] srend [ajuint] Reference Ensembl Sequence Region end
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
**
** @release 6.4.0
** @@
******************************************************************************/

AjBool ensAssemblyexceptionSetReferenceEnd(EnsPAssemblyexception ae,
                                           ajuint srend)
{
    if (!ae)
        return ajFalse;

    ae->ReferenceEnd = srend;

    return ajTrue;
}




/* @func ensAssemblyexceptionSetReferenceSeqregion ****************************
**
** Set the reference Ensembl Sequence Region identifier member of an
** Ensembl Assembly Exception.
**
** @param [u] ae [EnsPAssemblyexception] Ensembl Assembly Exception
** @param [r] srid [ajuint] Reference Ensembl Sequence Region identifier
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
**
** @release 6.4.0
** @@
******************************************************************************/

AjBool ensAssemblyexceptionSetReferenceSeqregion(EnsPAssemblyexception ae,
                                                 ajuint srid)
{
    if (!ae)
        return ajFalse;

    ae->ReferenceSeqregion = srid;

    return ajTrue;
}




/* @func ensAssemblyexceptionSetReferenceStart ********************************
**
** Set the reference Ensembl Sequence Region start member of an
** Ensembl Assembly Exception.
**
** @param [u] ae [EnsPAssemblyexception] Ensembl Assembly Exception
** @param [r] srstart [ajuint] Reference Ensembl Sequence Region start
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
**
** @release 6.4.0
** @@
******************************************************************************/

AjBool ensAssemblyexceptionSetReferenceStart(EnsPAssemblyexception ae,
                                             ajuint srstart)
{
    if (!ae)
        return ajFalse;

    ae->ReferenceStart = srstart;

    return ajTrue;
}




/* @func ensAssemblyexceptionSetType ******************************************
**
** Set the type member of an Ensembl Assembly Exception.
**
** @param [u] ae [EnsPAssemblyexception] Ensembl Assembly Exception
** @param [u] aet [EnsEAssemblyexceptionType]
** Ensembl Assembly Exception Type enumeration
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
**
** @release 6.2.0
** @@
******************************************************************************/

AjBool ensAssemblyexceptionSetType(EnsPAssemblyexception ae,
                                   EnsEAssemblyexceptionType aet)
{
    if (!ae)
        return ajFalse;

    ae->Type = aet;

    return ajTrue;
}




/* @section debugging *********************************************************
**
** Functions for reporting of an Ensembl Assembly Exception object.
**
** @fdata [EnsPAssemblyexception]
**
** @nam3rule Trace Report Ensembl Assembly Exception members to debug file
**
** @argrule Trace ae [const EnsPAssemblyexception] Ensembl Assembly Exception
** @argrule Trace level [ajuint] Indentation level
**
** @valrule * [AjBool] ajTrue upon success, ajFalse otherwise
**
** @fcategory misc
******************************************************************************/




/* @func ensAssemblyexceptionTrace ********************************************
**
** Trace an Ensembl Assembly Eception.
**
** @param [r] ae [const EnsPAssemblyexception] Ensembl Assembly Exception
** @param [r] level [ajuint] Indentation level
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
**
** @release 6.2.0
** @@
******************************************************************************/

AjBool ensAssemblyexceptionTrace(const EnsPAssemblyexception ae, ajuint level)
{
    AjPStr indent = NULL;

    if (!ae)
        return ajFalse;

    indent = ajStrNew();

    ajStrAppendCountK(&indent, ' ', level * 2);

    ajDebug("%SensAssemblyexceptionTrace %p\n"
            "%S  Identifier %u\n"
            "%S  ReferenceSeqregion %u\n"
            "%S  ReferenceStart %u\n"
            "%S  ReferenceEnd %u\n"
            "%S  ExceptionSeqregion %u\n"
            "%S  ExceptionStart %d\n"
            "%S  ExceptionEnd %u\n"
            "%S  Orientation %d\n"
            "%S  Type '%s'\n"
            "%S  Use %u\n",
            indent, ae,
            indent, ae->Identifier,
            indent, ae->ReferenceSeqregion,
            indent, ae->ReferenceStart,
            indent, ae->ReferenceEnd,
            indent, ae->ExceptionSeqregion,
            indent, ae->ExceptionStart,
            indent, ae->ExceptionEnd,
            indent, ensAssemblyexceptionTypeToChar(ae->Type),
            indent, ae->Orientation,
            indent, ae->Use);

    ajStrDel(&indent);

    return ajTrue;
}




/* @section calculate *********************************************************
**
** Functions for calculating values of an Ensembl Assembly Exception object.
**
** @fdata [EnsPAssemblyexception]
**
** @nam3rule Calculate Calculate Ensembl Assembly Exception values
** @nam4rule Exception Calculate attributes for the exception
** @nam4rule Memsize Calculate the memory size in bytes
** @nam4rule Reference Calculate attriutes for the reference
** @nam5rule Length Calculate the length
**
** @argrule * ae [const EnsPAssemblyexception] Ensembl Assembly Exception
**
** @valrule Length [ajuint] Length or 0U
** @valrule Memsize [size_t] Memory size in bytes or 0
**
** @fcategory misc
******************************************************************************/




/* @func ensAssemblyexceptionCalculateExceptionLength *************************
**
** Calculate the exception length of an Ensembl Assembly Exception.
**
** @param [r] ae [const EnsPAssemblyexception] Ensembl Assembly Exception
**
** @return [ajuint] Exception length or 0U
**
** @release 6.4.0
** @@
******************************************************************************/

ajuint ensAssemblyexceptionCalculateExceptionLength(
    const EnsPAssemblyexception ae)
{
    if (!ae)
        return 0U;

    return ae->ExceptionEnd - ae->ExceptionStart + 1U;
}




/* @func ensAssemblyexceptionCalculateMemsize *********************************
**
** Calculate the memory size in bytes of an Ensembl Assembly Exception.
**
** @param [r] ae [const EnsPAssemblyexception] Ensembl Assembly Exception
**
** @return [size_t] Memory size in bytes or 0
**
** @release 6.4.0
** @@
******************************************************************************/

size_t ensAssemblyexceptionCalculateMemsize(const EnsPAssemblyexception ae)
{
    size_t size = 0;

    if (!ae)
        return 0;

    size += sizeof (EnsOAssemblyexception);

    return size;
}




/* @func ensAssemblyexceptionCalculateReferenceLength *************************
**
** Calculate the reference length of an Ensembl Assembly Exception.
**
** @param [r] ae [const EnsPAssemblyexception] Ensembl Assembly Exception
**
** @return [ajuint] Reference length or 0U
**
** @release 6.4.0
** @@
******************************************************************************/

ajuint ensAssemblyexceptionCalculateReferenceLength(
    const EnsPAssemblyexception ae)
{
    if (!ae)
        return 0U;

    return ae->ReferenceEnd - ae->ReferenceStart + 1U;
}




/* @datasection [EnsEAssemblyexceptionType] Ensembl Assembly Exception Type ***
**
** @nam2rule Assemblyexception Functions for manipulating
** Ensembl Assembly Exception objects
** @nam3rule Type Functions for manipulating
** Ensembl Assembly Exception Type enumerations
**
******************************************************************************/




/* @section cast **************************************************************
**
** Functions for type casting values of an
** Ensembl Assembly Exception Type enumeration.
**
** @fdata [EnsEAssemblyexceptionType]
**
** @nam4rule From Ensembl Assembly Exception Type query
** @nam5rule Str AJAX String object query
**
** @argrule FromStr type [const AjPStr] Type string
**
** @valrule FromStr [EnsEAssemblyexceptionType] Ensembl Assembly Exception Type
** enumeration or ensEAssemblyexceptionTypeNULL
**
** @fcategory misc
******************************************************************************/




/* @func ensAssemblyexceptionTypeFromStr **************************************
**
** Return an Ensembl Assembly Exception Type enumeration from
** an AJAX String.
**
** @param [r] type [const AjPStr] Type string
**
** @return [EnsEAssemblyexceptionType]
** Ensembl Assembly Exception Type enumeration or ensEAssemblyexceptionTypeNULL
**
** @release 6.2.0
** @@
******************************************************************************/

EnsEAssemblyexceptionType ensAssemblyexceptionTypeFromStr(const AjPStr type)
{
    register EnsEAssemblyexceptionType i = ensEAssemblyexceptionTypeNULL;

    EnsEAssemblyexceptionType aet = ensEAssemblyexceptionTypeNULL;

    for (i = ensEAssemblyexceptionTypeNULL;
         assemblyexceptionKType[i];
         i++)
        if (ajStrMatchC(type, assemblyexceptionKType[i]))
            aet = i;

    if (!aet)
        ajDebug("ensAssemblyexceptionTypeFromStr encountered "
                "unexpected string '%S'.\n", type);

    return aet;
}




/* @section Cast **************************************************************
**
** Functions for returning attributes of an
** Ensembl Assembly Exception Type enumeration.
**
** @fdata [EnsEAssemblyexceptionType]
**
** @nam4rule To Return Ensembl Assembly Exception Type enumeration
** @nam5rule Char Return C character string value
**
** @argrule To aet [EnsEAssemblyexceptionType]
** Ensembl Assembly Exception Type enumeration
**
** @valrule * [const char*] Ensembl Assembly Exception Type
** @valrule *Char [const char*] Ensembl Assembly Exception Type or NULL
**
** @fcategory cast
******************************************************************************/




/* @func ensAssemblyexceptionTypeToChar ***************************************
**
** Cast an Ensembl Assembly Exception Type enumeration into a
** C-type (char *) string.
**
** @param [u] aet [EnsEAssemblyexceptionType]
** Ensembl Assembly Exception Type enumeration
**
** @return [const char*] Status C-type (char *) string or NULL
**
** @release 6.2.0
** @@
******************************************************************************/

const char* ensAssemblyexceptionTypeToChar(EnsEAssemblyexceptionType aet)
{
    register EnsEAssemblyexceptionType i = ensEAssemblyexceptionTypeNULL;

    for (i = ensEAssemblyexceptionTypeNULL;
         assemblyexceptionKType[i] && (i < aet);
         i++);

    if (!assemblyexceptionKType[i])
        ajDebug("ensAssemblyexceptionTypeToChar encountered an "
                "out of boundary error on "
                "Ensembl Assembly Exception Type enumeration %d.\n",
                aet);

    return assemblyexceptionKType[i];
}




/* @funcstatic listAssemblyexceptionCompareReferenceEndAscending **************
**
** AJAX List of Ensembl Assembly Exception objects comparison function to sort
** by reference end coordinate in ascending order.
**
** @param [r] item1 [const void*] Ensembl Assembly Exception address 1
** @param [r] item2 [const void*] Ensembl Assembly Exception address 2
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

static int listAssemblyexceptionCompareReferenceEndAscending(
    const void *item1,
    const void *item2)
{
    int result = 0;

    EnsPAssemblyexception ae1 = *(EnsOAssemblyexception *const *) item1;
    EnsPAssemblyexception ae2 = *(EnsOAssemblyexception *const *) item2;

#if defined(AJ_DEBUG) && AJ_DEBUG >= 2
    if (ajDebugTest("listAssemblyexceptionCompareReferenceEndAscending"))
    {
        ajDebug("listAssemblyexceptionCompareReferenceEndAscending\n"
                "  ae1 %p\n"
                "  ae2 %p\n",
                ae1,
                ae2);

        ensAssemblyexceptionTrace(ae1, 1);
        ensAssemblyexceptionTrace(ae2, 1);
    }
#endif /* defined(AJ_DEBUG) && AJ_DEBUG >= 2 */

    /* Sort empty values towards the end of the AJAX List. */

    if (ae1 && (!ae2))
        return -1;

    if ((!ae1) && (!ae2))
        return 0;

    if ((!ae1) && ae2)
        return +1;

    if (ae1->ReferenceEnd < ae2->ReferenceEnd)
        result = -1;

    if (ae1->ReferenceEnd > ae2->ReferenceEnd)
        result = +1;

    return result;
}




/* @funcstatic listAssemblyexceptionCompareReferenceEndDescending *************
**
** AJAX List of Ensembl Assembly Exception objects comparison function to sort
** by reference end coordinate in descending order.
**
** @param [r] item1 [const void*] Ensembl Assembly Exception address 1
** @param [r] item2 [const void*] Ensembl Assembly Exception address 2
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

static int listAssemblyexceptionCompareReferenceEndDescending(
    const void *item1,
    const void *item2)
{
    int result = 0;

    EnsPAssemblyexception ae1 = *(EnsOAssemblyexception *const *) item1;
    EnsPAssemblyexception ae2 = *(EnsOAssemblyexception *const *) item2;

#if defined(AJ_DEBUG) && AJ_DEBUG >= 2
    if (ajDebugTest("listAssemblyexceptionCompareReferenceEndDescending"))
    {
        ajDebug("listAssemblyexceptionCompareReferenceEndDescending\n"
                "  ae1 %p\n"
                "  ae2 %p\n",
                ae1,
                ae2);

        ensAssemblyexceptionTrace(ae1, 1);
        ensAssemblyexceptionTrace(ae2, 1);
    }
#endif /* defined(AJ_DEBUG) && AJ_DEBUG >= 2 */

    /* Sort empty values towards the end of the AJAX List. */

    if (ae1 && (!ae2))
        return -1;

    if ((!ae1) && (!ae2))
        return 0;

    if ((!ae1) && ae2)
        return +1;

    if (ae1->ReferenceEnd < ae2->ReferenceEnd)
        result = +1;

    if (ae1->ReferenceEnd > ae2->ReferenceEnd)
        result = -1;

    return result;
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
** @nam3rule Assemblyexception Functions for manipulating AJAX List objects of
** Ensembl Assembly Exception objects
** @nam4rule Sort Sort functions
** @nam5rule Exception Sort by exception
** @nam5rule Reference Sort by reference
** @nam6rule End Sort by end coordinate
** @nam6rule Start Sort by start coordinate
** @nam7rule Ascending Sort in ascending order
** @nam7rule Descending Sort in descending order
**
** @argrule Ascending aes [AjPList] AJAX List of
** Ensembl Assembly Exception objects
** @argrule Descending aes [AjPList] AJAX List of
** Ensembl Assembly Exception objects
**
** @valrule * [AjBool] ajTrue upon success, ajFalse otherwise
**
** @fcategory misc
******************************************************************************/




/* @func ensListAssemblyexceptionSortReferenceEndAscending ********************
**
** Sort an AJAX List of Ensembl Assembly Exception objects by their
** reference end member in ascending order.
**
** @param [u] aes [AjPList] AJAX List of Ensembl Assembly Exception objects
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
**
** @release 6.4.0
** @@
******************************************************************************/

AjBool ensListAssemblyexceptionSortReferenceEndAscending(AjPList aes)
{
    if (!aes)
        return ajFalse;

    ajListSort(aes, &listAssemblyexceptionCompareReferenceEndAscending);

    return ajTrue;
}




/* @func ensListAssemblyexceptionSortReferenceEndDescending *******************
**
** Sort an AJAX List of Ensembl Assembly Exception objects by their
** reference end member in descending order.
**
** @param [u] aes [AjPList] AJAX List of Ensembl Assembly Exception objects
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
**
** @release 6.4.0
** @@
******************************************************************************/

AjBool ensListAssemblyexceptionSortReferenceEndDescending(AjPList aes)
{
    if (!aes)
        return ajFalse;

    ajListSort(aes, &listAssemblyexceptionCompareReferenceEndDescending);

    return ajTrue;
}




/* @datasection [EnsPAssemblyexceptionadaptor] Ensembl Assembly Exception
** Adaptor
**
** @nam2rule Assemblyexceptionadaptor Functions for manipulating
** Ensembl Assembly Exception Adaptor objects
**
** @cc Bio::EnsEMBL::DBSQL::SliceAdaptor
** @cc CVS Revision: 1.104
**
******************************************************************************/




/* @section constructors ******************************************************
**
** All constructors return a new Ensembl Assembly Exception Adaptor by pointer.
** It is the responsibility of the user to first destroy any previous
** Assembly Exception Adaptor. The target pointer does not need to be
** initialised to NULL, but it is good programming practice to do so anyway.
**
** @fdata [EnsPAssemblyexceptionadaptor]
**
** @nam3rule New Constructor
**
** @argrule New dba [EnsPDatabaseadaptor] Ensembl Database Adaptor
**
** @valrule * [EnsPAssemblyexceptionadaptor]
** Ensembl Assembly Exception Adaptor or NULL
**
** @fcategory new
******************************************************************************/




/* @funcstatic assemblyexceptionadaptorFetchAllbyStatement ********************
**
** Run a SQL statement against an Ensembl Assembly Exception Adaptor and
** consolidate the results into an AJAX List of Ensembl Assembly Exception
** objects.
**
** @param [u] aea [EnsPAssemblyexceptionadaptor]
** Ensembl Assembly Exception Adaptor
** @param [r] statement [const AjPStr] SQL statement
** @param [u] aes [AjPList] AJAX List of Ensembl Assembly Exception objects
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
**
** @release 6.4.0
** @@
******************************************************************************/

static AjBool assemblyexceptionadaptorFetchAllbyStatement(
    EnsPAssemblyexceptionadaptor aea,
    const AjPStr statement,
    AjPList aes)
{
    ajint ori = 0;

    ajuint identifier = 0U;
    ajuint erid       = 0U;
    ajuint srid       = 0U;
    ajuint erstart    = 0U;
    ajuint srstart    = 0U;
    ajuint erend      = 0U;
    ajuint srend      = 0U;

    AjPSqlstatement sqls = NULL;
    AjISqlrow sqli       = NULL;
    AjPSqlrow sqlr       = NULL;

    AjPStr typestr = NULL;

    EnsEAssemblyexceptionType aet = ensEAssemblyexceptionTypeNULL;

    EnsPAssemblyexception ae = NULL;

    if (!aea)
        return ajFalse;

    if (!statement)
        return ajFalse;

    if (!aes)
        return ajFalse;

    sqls = ensDatabaseadaptorSqlstatementNew(aea->Adaptor, statement);

    sqli = ajSqlrowiterNew(sqls);

    while (!ajSqlrowiterDone(sqli))
    {
        identifier = 0;
        srid       = 0;
        srstart    = 0;
        srend      = 0;
        typestr    = ajStrNew();
        erid       = 0;
        erstart    = 0;
        erend      = 0;
        ori        = 0;

        aet = ensEAssemblyexceptionTypeNULL;

        sqlr = ajSqlrowiterGet(sqli);

        ajSqlcolumnToUint(sqlr, &identifier);
        ajSqlcolumnToUint(sqlr, &srid);
        ajSqlcolumnToUint(sqlr, &srstart);
        ajSqlcolumnToUint(sqlr, &srend);
        ajSqlcolumnToStr(sqlr, &typestr);
        ajSqlcolumnToUint(sqlr, &erid);
        ajSqlcolumnToUint(sqlr, &erstart);
        ajSqlcolumnToUint(sqlr, &erend);
        ajSqlcolumnToInt(sqlr, &ori);

        /* Set the Assembly Exception type. */

        aet = ensAssemblyexceptionTypeFromStr(typestr);

        if (!aet)
            ajFatal("assemblyexceptionadaptorFetchAllbyStatement "
                    "got unexpected Assembly Exception type '%S' "
                    "from database.\n",
                    typestr);

        ae = ensAssemblyexceptionNewIni(aea,
                                        identifier,
                                        srid,
                                        srstart,
                                        srend,
                                        erid,
                                        erstart,
                                        erend,
                                        ori,
                                        aet);

        ajListPushAppend(aes, (void *) ae);

        ajStrDel(&typestr);
    }

    ajSqlrowiterDel(&sqli);

    ensDatabaseadaptorSqlstatementDel(aea->Adaptor, &sqls);

    return ajTrue;
}




/* @funcstatic assemblyexceptionadaptorCacheInit ******************************
**
** Initialise the Ensembl Assembly Exception cache in the
** Assembly Exception Adaptor.
**
** @param [u] aea [EnsPAssemblyexceptionadaptor]
** Ensembl Assembly Exception Adaptor
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
**
** @release 6.3.0
** @@
******************************************************************************/

static AjBool assemblyexceptionadaptorCacheInit(
    EnsPAssemblyexceptionadaptor aea)
{
    ajuint *Pidentifier = NULL;

    AjPList list = NULL;
    AjPList aes  = NULL;

    AjPStr statement = NULL;

    EnsPAssemblyexception ae = NULL;

    if (!aea)
        return ajFalse;

    if (aea->CacheByReferenceSeqregion)
        return ajTrue;
    else
    {
        aea->CacheByReferenceSeqregion = ajTableuintNew(0);

        ajTableSetDestroyvalue(
            aea->CacheByReferenceSeqregion,
            (void (*)(void **))
            &assemblyexceptionadaptorListAssemblyexceptionValdel);
    }

    statement = ajFmtStr(
        "SELECT "
        "assembly_exception.assembly_exception_id, "
        "assembly_exception.seq_region_id, "
        "assembly_exception.seq_region_start, "
        "assembly_exception.seq_region_end, "
        "assembly_exception.exc_type, "
        "assembly_exception.exc_seq_region_id, "
        "assembly_exception.exc_seq_region_start, "
        "assembly_exception.exc_seq_region_end, "
        "assembly_exception.ori "
        "FROM "
        "assembly_exception, "
        "seq_region, "
        "coord_system "
        "WHERE "
        "seq_region.seq_region_id = "
        "assembly_exception.seq_region_id "
        "AND "
        "seq_region.coord_system_id = "
        "coord_system.coord_system_id "
        "AND "
        "coord_system.species_id = %u",
        ensDatabaseadaptorGetIdentifier(aea->Adaptor));

    aes = ajListNew();

    assemblyexceptionadaptorFetchAllbyStatement(aea, statement, aes);

    ajStrDel(&statement);

    while (ajListPop(aes, (void **) &ae))
    {
        list = (AjPList) ajTableFetchmodV(
            aea->CacheByReferenceSeqregion,
            (const void *) &ae->ReferenceSeqregion);

        if (!list)
        {
            AJNEW0(Pidentifier);

            *Pidentifier = ae->ReferenceSeqregion;

            list = ajListNew();

            ajTablePut(aea->CacheByReferenceSeqregion,
                       (void *) Pidentifier,
                       (void *) list);
        }

        ajListPushAppend(list, (void *) ae);
    }

    ajListFree(&aes);

    return ajTrue;
}




/* @func ensAssemblyexceptionadaptorNew ***************************************
**
** Default constructor for an Ensembl Assembly Exception Adaptor.
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
** @see ensRegistryGetAssemblyexceptionadaptor
**
** @param [u] dba [EnsPDatabaseadaptor] Ensembl Database Adaptor
**
** @return [EnsPAssemblyexceptionadaptor]
** Ensembl Assembly Exception Adaptor or NULL
**
** @release 6.2.0
** @@
******************************************************************************/

EnsPAssemblyexceptionadaptor ensAssemblyexceptionadaptorNew(
    EnsPDatabaseadaptor dba)
{
    EnsPAssemblyexceptionadaptor aea = NULL;

    if (!dba)
        return NULL;

    AJNEW0(aea);

    aea->Adaptor = dba;

    assemblyexceptionadaptorCacheInit(aea);

    return aea;
}




/* @section destructors *******************************************************
**
** Destruction destroys all internal data structures and frees the memory
** allocated for an Ensembl Assembly Exception Adaptor object.
**
** @fdata [EnsPAssemblyexceptionadaptor]
**
** @nam3rule Del Destroy (free) an Ensembl Assembly Exception Adaptor object
**
** @argrule * Paea [EnsPAssemblyexceptionadaptor*]
** Ensembl Assembly Exception Adaptor object address
**
** @valrule * [void]
**
** @fcategory delete
******************************************************************************/




/* @funcstatic assemblyexceptionadaptorListAssemblyexceptionValdel ************
**
** An ajTableSetDestroyvalue "valdel" function to clear AJAX Table value data.
** This function removes and deletes Ensembl Assembly Exception objects
** from an AJAX List object, before deleting the AJAX List object.
**
** @param [d] Pvalue [void**] AJAX List address
** @see ajTableSetDestroyvalue
**
** @return [void]
**
** @release 6.3.0
** @@
******************************************************************************/

static void assemblyexceptionadaptorListAssemblyexceptionValdel(void **Pvalue)
{
    EnsPAssemblyexception ae = NULL;

    if (!Pvalue)
        return;

    if (!*Pvalue)
        return;

    while (ajListPop(*((AjPList *) Pvalue), (void **) &ae))
        ensAssemblyexceptionDel(&ae);

    ajListFree((AjPList *) Pvalue);

    return;
}




/* @func ensAssemblyexceptionadaptorDel ***************************************
**
** Default destructor for an Ensembl Assembly Exception Adaptor.
**
** This function also clears the internal Assembly Exception cache.
**
** Ensembl Object Adaptors are singleton objects that are registered in the
** Ensembl Registry and weakly referenced by Ensembl Objects that have been
** instantiated by it. Therefore, Ensembl Object Adaptors should never be
** destroyed directly. Upon exit, the Ensembl Registry will call this function
** if required.
**
** @param [d] Paea [EnsPAssemblyexceptionadaptor*]
** Ensembl Assembly Exception Adaptor object address
**
** @return [void]
**
** @release 6.2.0
** @@
******************************************************************************/

void ensAssemblyexceptionadaptorDel(EnsPAssemblyexceptionadaptor *Paea)
{
    EnsPAssemblyexceptionadaptor pthis = NULL;

    if (!Paea)
        return;

#if defined(AJ_DEBUG) && AJ_DEBUG >= 1
    if (ajDebugTest("ensAssemblyexceptionadaptorDel"))
        ajDebug("ensAssemblyexceptionadaptorDel\n"
                "  *Paea %p\n",
                *Paea);
#endif /* defined(AJ_DEBUG) && AJ_DEBUG >= 1 */

    if (!*Paea)
        return;

    pthis = *Paea;

    ajTableDel(&pthis->CacheByReferenceSeqregion);

    AJFREE(pthis);

    *Paea = NULL;

    return;
}




/* @section member retrieval **************************************************
**
** Functions for returning members of an
** Ensembl Assembly Exception Adaptor object.
**
** @fdata [EnsPAssemblyexceptionadaptor]
**
** @nam3rule Get Return Ensembl Assembly Exception Adaptor attribute(s)
** @nam4rule Databaseadaptor Return the Ensembl Database Adaptor
**
** @argrule * aea [EnsPAssemblyexceptionadaptor]
** Ensembl Assembly Exception Adaptor
**
** @valrule Databaseadaptor [EnsPDatabaseadaptor] Ensembl Database Adaptor
** or NULL
**
** @fcategory use
******************************************************************************/




/* @func ensAssemblyexceptionadaptorGetDatabaseadaptor ************************
**
** Get the Ensembl Database Adaptor member of an
** Ensembl Assembly Exception Adaptor.
**
** @param [u] aea [EnsPAssemblyexceptionadaptor]
** Ensembl Assembly Exception Adaptor
**
** @return [EnsPDatabaseadaptor] Ensembl Database Adaptor or NULL
**
** @release 6.4.0
** @@
******************************************************************************/

EnsPDatabaseadaptor ensAssemblyexceptionadaptorGetDatabaseadaptor(
    EnsPAssemblyexceptionadaptor aea)
{
    return (aea) ? aea->Adaptor : NULL;
}




/* @section object retrieval **************************************************
**
** Functions for fetching Ensembl Assembly Exception objects from an
** Ensembl SQL database.
**
** @fdata [EnsPAssemblyexceptionadaptor]
**
** @nam3rule Fetch     Fetch Ensembl Assembly Exception object(s)
** @nam4rule All       Fetch all Ensembl Assembly Exception objects
** @nam4rule Allby     Fetch all Ensembl Assembly Exception objects
**                     matching a criterion
** @nam5rule Reference Fetch by reference
** @nam6rule Seqregion Fetch by a reference Ensembl Sequence Region identifier
** @nam4rule By        Fetch one Ensembl Assembly Exception object
**                     matching a criterion
**
** @argrule * aea [const EnsPAssemblyexceptionadaptor]
** Ensembl Assembly Exception Adaptor
** @argrule FetchAll aes [AjPList]
** AJAX List of Ensembl Assembly Exception objects
** @argrule FetchAllbyReferenceSeqregion srid [ajuint]
** Reference Ensembl Sequence Region
** @argrule FetchAllby aes [AjPList]
** AJAX List of Ensembl Assembly Exception objects
**
** @valrule * [AjBool] ajTrue upon success, ajFalse otherwise
**
** @fcategory use
******************************************************************************/




/* @funcstatic assemblyexceptionadaptorFetchAll *******************************
**
** An ajTableMap "apply" function to return all Ensembl Assembly Exception
** objects from the Ensembl Assembly Exception Adaptor-internal cache.
**
** @param [u] key [const void*] AJAX unsigned integer key data address
** @param [u] Pvalue [void**] Ensembl Assembly Exception value data address
** @param [u] cl [void*]
** AJAX List of Ensembl Assembly Exception objects, passed in via ajTableMap
** @see ajTableMap
**
** @return [void]
**
** @release 6.3.0
** @@
******************************************************************************/

static void assemblyexceptionadaptorFetchAll(const void *key,
                                             void **Pvalue,
                                             void *cl)
{
    AjIList iter = NULL;

    EnsPAssemblyexception ae = NULL;

    if (!key)
        return;

    if (!Pvalue)
        return;

    if (!*Pvalue)
        return;

    if (!cl)
        return;

    iter = ajListIterNew(*((AjPList *) Pvalue));

    while (!ajListIterDone(iter))
    {
        ae = (EnsPAssemblyexception) ajListIterGet(iter);

        ajListPushAppend((AjPList) cl,
                         (void *) ensAssemblyexceptionNewRef(ae));
    }

    ajListIterDel(&iter);

    return;
}




/* @func ensAssemblyexceptionadaptorFetchAll **********************************
**
** Fetch all Ensembl Assembly Exception objects.
**
** The caller is responsible for deleting the Ensembl Assembly Exception
** objects before deleting the AJAX List.
**
** @param [r] aea [const EnsPAssemblyexceptionadaptor]
** Ensembl Assembly Exception Adaptor
** @param [u] aes [AjPList] AJAX List of Ensembl Assembly Exception objects
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
**
** @release 6.2.0
** @@
******************************************************************************/

AjBool ensAssemblyexceptionadaptorFetchAll(
    const EnsPAssemblyexceptionadaptor aea,
    AjPList aes)
{
    if (!aea)
        return ajFalse;

    if (!aes)
        return ajFalse;

    ajTableMap(aea->CacheByReferenceSeqregion,
               &assemblyexceptionadaptorFetchAll,
               (void *) aes);

    return ajTrue;
}




/* @func ensAssemblyexceptionadaptorFetchAllbyReferenceSeqregion **************
**
** Fetch all Ensembl Assembly Exception objects by a reference
** Ensembl Sequence Region identifier.
**
** The caller is responsible for deleting the Ensembl Assembly Exception
** objects before deleting the AJAX List.
**
** @param [r] aea [const EnsPAssemblyexceptionadaptor]
** Ensembl Assembly Exception Adaptor
** @param [r] srid [ajuint] Reference Ensembl Sequence Region identifier
** @param [u] aes [AjPList] AJAX List of Ensembl Assembly Exception objects
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
**
** @release 6.4.0
** @@
******************************************************************************/

AjBool ensAssemblyexceptionadaptorFetchAllbyReferenceSeqregion(
    const EnsPAssemblyexceptionadaptor aea,
    ajuint srid,
    AjPList aes)
{
    AjIList iter = NULL;
    AjPList list = NULL;

    EnsPAssemblyexception ae = NULL;

    if (ajDebugTest("ensAssemblyexceptionadaptorFetchAllbyReferenceSeqregion"))
        ajDebug("ensAssemblyexceptionadaptorFetchAllbyReferenceSeqregion\n"
                "  aea %p\n"
                "  srid %u\n"
                "  aes %p\n",
                aea,
                srid,
                aes);

    if (!aea)
        return ajFalse;

    if (!aes)
        return ajFalse;

    list = (AjPList) ajTableFetchmodV(aea->CacheByReferenceSeqregion,
                                      (const void *) &srid);

    iter = ajListIterNew(list);

    while (!ajListIterDone(iter))
    {
        ae = (EnsPAssemblyexception) ajListIterGet(iter);

        ajListPushAppend(aes, (void *) ensAssemblyexceptionNewRef(ae));
    }

    ajListIterDel(&iter);

    return ajTrue;
}
