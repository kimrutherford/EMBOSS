/* @source Ensembl Assembly Exception functions
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

#include "ensassemblyexception.h"
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

/* @conststatic assemblyexceptionType *****************************************
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

static const char* assemblyexceptionType[] =
{
    "",
    "HAP",
    "PAR",
    "PATCH_FIX",
    "PATCH_NOVEL",
    "HAP REF",
    "PATCH_FIX REF",
    "PATCH_NOVEL REF",
    (const char*) NULL
};




/* ==================================================================== */
/* ======================== private variables ========================= */
/* ==================================================================== */




/* ==================================================================== */
/* ======================== private functions ========================= */
/* ==================================================================== */

static int listAssemblyexceptionCompareReferenceEndAscending(const void* P1,
                                                             const void* P2);

static int listAssemblyexceptionCompareReferenceEndDescending(const void* P1,
                                                              const void* P2);

static AjBool assemblyexceptionadaptorFetchAllbyStatement(
    EnsPAssemblyexceptionadaptor aea,
    const AjPStr statement,
    AjPList aes);

static AjBool assemblyexceptionadaptorCacheInit(
    EnsPAssemblyexceptionadaptor aea);

static void assemblyexceptionadaptorClearIdentifierCache(void** key,
                                                         void** value,
                                                         void* cl);

static AjBool assemblyexceptionadaptorCacheExit(
    EnsPAssemblyexceptionadaptor aea);

static void assemblyexceptionadaptorFetchAll(const void* key,
                                             void** value,
                                             void* cl);




/* ==================================================================== */
/* ===================== All functions by section ===================== */
/* ==================================================================== */




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
** @cc CVS Revision: 1.273
** @cc CVS Tag: branch-ensembl-62
**
** @cc Bio::EnsEMBL::DBSQL::SliceAdaptor
** @cc CVS Revision: 1.121
** @cc CVS Tag: branch-ensembl-62
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
** @argrule Ini aet [EnsEAssemblyexceptionType] Type
** @argrule Ref ae [EnsPAssemblyexception] Ensembl Assembly Exception
**
** @valrule * [EnsPAssemblyexception] Ensembl Assembly Exception
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
** @@
******************************************************************************/

EnsPAssemblyexception ensAssemblyexceptionNewCpy(
    const EnsPAssemblyexception ae)
{
    EnsPAssemblyexception pthis = NULL;

    if(!ae)
        return NULL;

    AJNEW0(pthis);

    pthis->Use = 1;

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
** @param [u] aea [EnsPAssemblyexceptionadaptor] Ensembl Assembly
**                                               Exception Adaptor
** @param [r] identifier [ajuint] SQL database-internal identifier
** @param [r] srid [ajuint] Reference Ensembl Sequence Region identifier
** @param [r] srstart [ajuint] Reference Ensembl Sequence Region start
** @param [r] srend [ajuint] Reference Ensembl Sequence Region end
** @param [r] erid [ajuint] Exception Ensembl Sequence Region identifier
** @param [r] erstart [ajuint] Exception Ensembl Sequence Region start
** @param [r] erend [ajuint] Exception Ensembl Sequence Region end
** @param [r] ori [ajint] Orientation
** @param [u] aet [EnsEAssemblyexceptionType] Type
**
** @return [EnsPAssemblyexception] Ensembl Assembl Exception or NULL
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

    if(!srid)
        return NULL;

    if(!erid)
        return NULL;

    AJNEW0(ae);

    ae->Use = 1;

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
** @@
******************************************************************************/

EnsPAssemblyexception ensAssemblyexceptionNewRef(EnsPAssemblyexception ae)
{
    if(!ae)
        return NULL;

    ae->Use++;

    return ae;
}




/* @section destructors *******************************************************
**
** Destruction destroys all internal data structures and frees the
** memory allocated for an Ensembl Assembly Exception object.
**
** @fdata [EnsPAssemblyexception]
**
** @nam3rule Del Destroy (free) an Ensembl Assembly Exception object
**
** @argrule * Pae [EnsPAssemblyexception*] Ensembl Assembly Exception object
**                                         address
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
** @@
******************************************************************************/

void ensAssemblyexceptionDel(EnsPAssemblyexception* Pae)
{
    EnsPAssemblyexception pthis = NULL;

    if(!Pae)
        return;

    if(!*Pae)
        return;

    pthis = *Pae;

    pthis->Use--;

    if(pthis->Use)
    {
        *Pae = NULL;

        return;
    }

    AJFREE(pthis);

    *Pae = NULL;

    return;
}




/* @section element retrieval *************************************************
**
** Functions for returning elements of an Ensembl Assembly Exception object.
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
** @valrule Adaptor [EnsPAssemblyexceptionadaptor] Ensembl Assembly
**                                                 Exception Adaptor or NULL
** @valrule End [ajuint] Ensembl Sequence Region end or 0
** @valrule Seqregion [ajuint] Ensembl Sequence Region identifier or 0
** @valrule Start [ajuint] Ensembl Sequence Region start or 0
** @valrule Identifier [ajuint] SQL database-internal identifier or 0
** @valrule Orientation [ajint] Orientation or 0
** @valrule Type [EnsEAssemblyexceptionType] Type or
** ensEAssemblyexceptionTypeNULL
**
** @fcategory use
******************************************************************************/




/* @func ensAssemblyexceptionGetAdaptor ***************************************
**
** Get the Ensembl Assembly Exception Adaptor element of an
** Ensembl Assembly Exception.
**
** @param [r] ae [const EnsPAssemblyexception] Ensembl Assembly Exception
**
** @return [EnsPAssemblyexceptionadaptor] Ensembl Assembly Exception Adaptor
**                                        or NULL
** @@
******************************************************************************/

EnsPAssemblyexceptionadaptor ensAssemblyexceptionGetAdaptor(
    const EnsPAssemblyexception ae)
{
    if(!ae)
        return 0;

    return ae->Adaptor;
}




/* @func ensAssemblyexceptionGetExceptionEnd **********************************
**
** Get the exception Ensembl Sequence Region end element of an
** Ensembl Assembly Exception.
**
** @param [r] ae [const EnsPAssemblyexception] Ensembl Assembly Exception
**
** @return [ajuint] Exception Ensembl Sequence Region end or 0
** @@
******************************************************************************/

ajuint ensAssemblyexceptionGetExceptionEnd(
    const EnsPAssemblyexception ae)
{
    if(!ae)
        return 0;

    return ae->ExceptionEnd;
}




/* @func ensAssemblyexceptionGetExceptionSeqregion ****************************
**
** Get the exception Ensembl Sequence Region identifier element of an
** Ensembl Assembly Exception.
**
** @param [r] ae [const EnsPAssemblyexception] Ensembl Assembly Exception
**
** @return [ajuint] Exception Ensembl Sequence Region identifier or 0
** @@
******************************************************************************/

ajuint ensAssemblyexceptionGetExceptionSeqregion(
    const EnsPAssemblyexception ae)
{
    if(!ae)
        return 0;

    return ae->ExceptionSeqregion;
}




/* @func ensAssemblyexceptionGetExceptionStart ********************************
**
** Get the exception Ensembl Sequence Region start element of an
** Ensembl Assembly Exception.
**
** @param [r] ae [const EnsPAssemblyexception] Ensembl Assembly Exception
**
** @return [ajuint] Exception Ensembl Sequence Region start or 0
** @@
******************************************************************************/

ajuint ensAssemblyexceptionGetExceptionStart(
    const EnsPAssemblyexception ae)
{
    if(!ae)
        return 0;

    return ae->ExceptionStart;
}




/* @func ensAssemblyexceptionGetIdentifier ************************************
**
** Get the SQL database-internal identifier element of an
** Ensembl Assembly Exception.
**
** @param [r] ae [const EnsPAssemblyexception] Ensembl Assembly Exception
**
** @return [ajuint] SQL database-internal identifier or 0
** @@
******************************************************************************/

ajuint ensAssemblyexceptionGetIdentifier(
    const EnsPAssemblyexception ae)
{
    if(!ae)
        return 0;

    return ae->Identifier;
}




/* @func ensAssemblyexceptionGetOrientation ***********************************
**
** Get the orientation element of an Ensembl Assembly Exception.
**
** @param [r] ae [const EnsPAssemblyexception] Ensembl Assembly Exception
**
** @return [ajint] Orientation or 0
** @@
******************************************************************************/

ajint ensAssemblyexceptionGetOrientation(
    const EnsPAssemblyexception ae)
{
    if(!ae)
        return 0;

    return ae->Orientation;
}




/* @func ensAssemblyexceptionGetReferenceEnd **********************************
**
** Get the reference Ensembl Sequence Region end element of an
** Ensembl Assembly Exception.
**
** @param [r] ae [const EnsPAssemblyexception] Ensembl Assembly Exception
**
** @return [ajuint] Reference Ensembl Sequence Region end or 0
** @@
******************************************************************************/

ajuint ensAssemblyexceptionGetReferenceEnd(
    const EnsPAssemblyexception ae)
{
    if(!ae)
        return 0;

    return ae->ReferenceEnd;
}




/* @func ensAssemblyexceptionGetReferenceSeqregion ****************************
**
** Get the reference Ensembl Sequence Region identifier element of an
** Ensembl Assembly Exception.
**
** @param [r] ae [const EnsPAssemblyexception] Ensembl Assembly Exception
**
** @return [ajuint] Reference Ensembl Sequence Region identifier or 0
** @@
******************************************************************************/

ajuint ensAssemblyexceptionGetReferenceSeqregion(
    const EnsPAssemblyexception ae)
{
    if(!ae)
        return 0;

    return ae->ReferenceSeqregion;
}




/* @func ensAssemblyexceptionGetReferenceStart ********************************
**
** Get the reference Ensembl Sequence Region start element of an
** Ensembl Assembly Exception.
**
** @param [r] ae [const EnsPAssemblyexception] Ensembl Assembly Exception
**
** @return [ajuint] Reference Ensembl Sequence Region start or 0
** @@
******************************************************************************/

ajuint ensAssemblyexceptionGetReferenceStart(
    const EnsPAssemblyexception ae)
{
    if(!ae)
        return 0;

    return ae->ReferenceStart;
}




/* @func ensAssemblyexceptionGetType ******************************************
**
** Get the type element of an Ensembl Assembly Exception.
**
** @param [r] ae [const EnsPAssemblyexception] Ensembl Assembly Exception
**
** @return [EnsEAssemblyexceptionType] Type or ensEAssemblyexceptionTypeNULL
** @@
******************************************************************************/

EnsEAssemblyexceptionType ensAssemblyexceptionGetType(
    const EnsPAssemblyexception ae)
{
    if(!ae)
        return ensEAssemblyexceptionTypeNULL;

    return ae->Type;
}




/* @section element assignment ************************************************
**
** Functions for assigning elements of an Ensembl Assembly Exception object.
**
** @fdata [EnsPAssemblyexception]
**
** @nam3rule Set Set one element of an Assembly Exception
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
** @argrule Adaptor aea [EnsPAssemblyexceptionadaptor] Ensembl Assembly
**                                                     Exception Adaptor
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
** Set the Ensembl Assembly Exception Adaptor element of an
** Ensembl Assembly Exception.
**
** @param [u] ae [EnsPAssemblyexception] Ensembl Assembly Exception
** @param [u] aea [EnsPAssemblyexceptionadaptor] Ensembl Assembly
**                                               Exception Adaptor
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
** @@
******************************************************************************/

AjBool ensAssemblyexceptionSetAdaptor(EnsPAssemblyexception ae,
                                      EnsPAssemblyexceptionadaptor aea)
{
    if(!ae)
        return ajFalse;

    ae->Adaptor = aea;

    return ajTrue;
}




/* @func ensAssemblyexceptionSetExceptionEnd **********************************
**
** Set the exception Ensembl Sequence Region end element of an
** Ensembl Assembly Exception.
**
** @param [u] ae [EnsPAssemblyexception] Ensembl Assembly Exception
** @param [r] erend [ajuint] Exception Ensembl Sequence Region end
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
** @@
******************************************************************************/

AjBool ensAssemblyexceptionSetExceptionEnd(EnsPAssemblyexception ae,
                                           ajuint erend)
{
    if(!ae)
        return ajFalse;

    ae->ExceptionEnd = erend;

    return ajTrue;
}




/* @func ensAssemblyexceptionSetExceptionSeqregion ****************************
**
** Set the exception Ensembl Sequence Region identifier element of an
** Ensembl Assembly Exception.
**
** @param [u] ae [EnsPAssemblyexception] Ensembl Assembly Exception
** @param [r] erid [ajuint] Exception Ensembl Sequence Region identifier
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
** @@
******************************************************************************/

AjBool ensAssemblyexceptionSetExceptionSeqregion(EnsPAssemblyexception ae,
                                                 ajuint erid)
{
    if(!ae)
        return ajFalse;

    ae->ExceptionSeqregion = erid;

    return ajTrue;
}




/* @func ensAssemblyexceptionSetExceptionStart ********************************
**
** Set the exception Ensembl Sequence Region start element of an
** Ensembl Assembly Exception.
**
** @param [u] ae [EnsPAssemblyexception] Ensembl Assembly Exception
** @param [r] erstart [ajuint] Exception Ensembl Sequence Region start
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
** @@
******************************************************************************/

AjBool ensAssemblyexceptionSetExceptionStart(EnsPAssemblyexception ae,
                                             ajuint erstart)
{
    if(!ae)
        return ajFalse;

    ae->ExceptionStart = erstart;

    return ajTrue;
}




/* @func ensAssemblyexceptionSetIdentifier ************************************
**
** Set the SQL database identifier element of an Ensembl Assembly Exception.
**
** @param [u] ae [EnsPAssemblyexception] Ensembl Assembly Exception
** @param [r] identifier [ajuint] SQL database-internal identifier
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
** @@
******************************************************************************/

AjBool ensAssemblyexceptionSetIdentifier(EnsPAssemblyexception ae,
                                         ajuint identifier)
{
    if(!ae)
        return ajFalse;

    ae->Identifier = identifier;

    return ajTrue;
}




/* @func ensAssemblyexceptionSetOrientation ***********************************
**
** Set the orientation element of an Ensembl Assembly Exception.
**
** @param [u] ae [EnsPAssemblyexception] Ensembl Assembly Exception
** @param [r] orientation [ajint] Orientation
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
** @@
******************************************************************************/

AjBool ensAssemblyexceptionSetOrientation(EnsPAssemblyexception ae,
                                          ajint orientation)
{
    if(!ae)
        return ajFalse;

    ae->Orientation = orientation;

    return ajTrue;
}




/* @func ensAssemblyexceptionSetReferenceEnd **********************************
**
** Set the reference Ensembl Sequence Region end element of an
** Ensembl Assembly Exception.
**
** @param [u] ae [EnsPAssemblyexception] Ensembl Assembly Exception
** @param [r] srend [ajuint] Reference Ensembl Sequence Region end
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
** @@
******************************************************************************/

AjBool ensAssemblyexceptionSetReferenceEnd(EnsPAssemblyexception ae,
                                           ajuint srend)
{
    if(!ae)
        return ajFalse;

    ae->ReferenceEnd = srend;

    return ajTrue;
}




/* @func ensAssemblyexceptionSetReferenceSeqregion ****************************
**
** Set the reference Ensembl Sequence Region identifier element of an
** Ensembl Assembly Exception.
**
** @param [u] ae [EnsPAssemblyexception] Ensembl Assembly Exception
** @param [r] srid [ajuint] Reference Ensembl Sequence Region identifier
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
** @@
******************************************************************************/

AjBool ensAssemblyexceptionSetReferenceSeqregion(EnsPAssemblyexception ae,
                                                 ajuint srid)
{
    if(!ae)
        return ajFalse;

    ae->ReferenceSeqregion = srid;

    return ajTrue;
}




/* @func ensAssemblyexceptionSetReferenceStart ********************************
**
** Set the reference Ensembl Sequence Region start element of an
** Ensembl Assembly Exception.
**
** @param [u] ae [EnsPAssemblyexception] Ensembl Assembly Exception
** @param [r] srstart [ajuint] Reference Ensembl Sequence Region start
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
** @@
******************************************************************************/

AjBool ensAssemblyexceptionSetReferenceStart(EnsPAssemblyexception ae,
                                             ajuint srstart)
{
    if(!ae)
        return ajFalse;

    ae->ReferenceStart = srstart;

    return ajTrue;
}




/* @func ensAssemblyexceptionSetType ******************************************
**
** Set the type element of an Ensembl Assembly Exception.
**
** @param [u] ae [EnsPAssemblyexception] Ensembl Assembly Exception
** @param [u] aet [EnsEAssemblyexceptionType]
** Ensembl Assembly Exception Type enumeration
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
** @@
******************************************************************************/

AjBool ensAssemblyexceptionSetType(EnsPAssemblyexception ae,
                                   EnsEAssemblyexceptionType aet)
{
    if(!ae)
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
** @nam3rule Trace Report Ensembl Assembly Exception elements to debug file
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
** @@
******************************************************************************/

AjBool ensAssemblyexceptionTrace(const EnsPAssemblyexception ae, ajuint level)
{
    AjPStr indent = NULL;

    if(!ae)
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
** @valrule Length [ajuint] Length or 0
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
** @return [ajuint] Exception length or 0
** @@
******************************************************************************/

ajuint ensAssemblyexceptionCalculateExceptionLength(
    const EnsPAssemblyexception ae)
{
    if(!ae)
        return 0;

    return ae->ExceptionEnd - ae->ExceptionStart + 1;
}




/* @func ensAssemblyexceptionCalculateMemsize *********************************
**
** Calculate the memory size in bytes of an Ensembl Assembly Exception.
**
** @param [r] ae [const EnsPAssemblyexception] Ensembl Assembly Exception
**
** @return [size_t] Memory size in bytes or 0
** @@
******************************************************************************/

size_t ensAssemblyexceptionCalculateMemsize(const EnsPAssemblyexception ae)
{
    size_t size = 0;

    if(!ae)
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
** @return [ajuint] Reference length or 0
** @@
******************************************************************************/

ajuint ensAssemblyexceptionCalculateReferenceLength(
    const EnsPAssemblyexception ae)
{
    if(!ae)
        return 0;

    return ae->ReferenceEnd - ae->ReferenceStart + 1;
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
** @nam5rule Str  AJAX String object query
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
** @@
******************************************************************************/

EnsEAssemblyexceptionType ensAssemblyexceptionTypeFromStr(const AjPStr type)
{
    register EnsEAssemblyexceptionType i = ensEAssemblyexceptionTypeNULL;

    EnsEAssemblyexceptionType aet = ensEAssemblyexceptionTypeNULL;

    for(i = ensEAssemblyexceptionTypeNULL;
        assemblyexceptionType[i];
        i++)
        if(ajStrMatchC(type, assemblyexceptionType[i]))
            aet = i;

    if(!aet)
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
** C-type (char*) string.
**
** @param [u] aet [EnsEAssemblyexceptionType]
** Ensembl Assembly Exception Type enumeration
**
** @return [const char*] Status C-type (char*) string or NULL
** @@
******************************************************************************/

const char* ensAssemblyexceptionTypeToChar(EnsEAssemblyexceptionType aet)
{
    register EnsEAssemblyexceptionType i = ensEAssemblyexceptionTypeNULL;

    for(i = ensEAssemblyexceptionTypeNULL;
        assemblyexceptionType[i] && (i < aet);
        i++);

    if(!assemblyexceptionType[i])
        ajDebug("ensAssemblyexceptionTypeToChar encountered an "
                "out of boundary error on "
                "Ensembl Assembly Exception Type enumeration %d.\n",
                aet);

    return assemblyexceptionType[i];
}




/* @funcstatic listAssemblyexceptionCompareReferenceEndAscending **************
**
** AJAX List of Ensembl Assembly Exception objects comparison function to sort
** by reference end coordinate in ascending order.
**
** @param [r] P1 [const void*] Ensembl Assembly Exception address 1
** @param [r] P2 [const void*] Ensembl Assembly Exception address 2
** @see ajListSort
**
** @return [int] The comparison function returns an integer less than,
**               equal to, or greater than zero if the first argument is
**               considered to be respectively less than, equal to, or
**               greater than the second.
** @@
******************************************************************************/

static int listAssemblyexceptionCompareReferenceEndAscending(const void* P1,
                                                             const void* P2)
{
    int result = 0;

    EnsPAssemblyexception ae1 = NULL;
    EnsPAssemblyexception ae2 = NULL;

    ae1 = *(EnsPAssemblyexception const*) P1;
    ae2 = *(EnsPAssemblyexception const*) P2;

    if(ajDebugTest("listAssemblyexceptionCompareReferenceEndAscending"))
    {
        ajDebug("listAssemblyexceptionCompareReferenceEndAscending\n"
                "  ae1 %p\n"
                "  ae2 %p\n",
                ae1,
                ae2);

        ensAssemblyexceptionTrace(ae1, 1);
        ensAssemblyexceptionTrace(ae2, 1);
    }

    /* Sort empty values towards the end of the AJAX List. */

    if(ae1 && (!ae2))
        return -1;

    if((!ae1) && (!ae2))
        return 0;

    if((!ae1) && ae2)
        return +1;

    if(ae1->ReferenceEnd < ae2->ReferenceEnd)
        result = -1;

    if(ae1->ReferenceEnd > ae2->ReferenceEnd)
        result = +1;

    return result;
}




/* @funcstatic listAssemblyexceptionCompareReferenceEndDescending *************
**
** AJAX List of Ensembl Assembly Exception objects comparison function to sort
** by reference end coordinate in descending order.
**
** @param [r] P1 [const void*] Ensembl Assembly Exception address 1
** @param [r] P2 [const void*] Ensembl Assembly Exception address 2
** @see ajListSort
**
** @return [int] The comparison function returns an integer less than,
**               equal to, or greater than zero if the first argument is
**               considered to be respectively less than, equal to, or
**               greater than the second.
** @@
******************************************************************************/

static int listAssemblyexceptionCompareReferenceEndDescending(const void* P1,
                                                              const void* P2)
{
    int result = 0;

    EnsPAssemblyexception ae1 = NULL;
    EnsPAssemblyexception ae2 = NULL;

    ae1 = *(EnsPAssemblyexception const*) P1;
    ae2 = *(EnsPAssemblyexception const*) P2;

    if(ajDebugTest("listAssemblyexceptionCompareReferenceEndDescending"))
    {
        ajDebug("listAssemblyexceptionCompareReferenceEndDescending\n"
                "  ae1 %p\n"
                "  ae2 %p\n",
                ae1,
                ae2);

        ensAssemblyexceptionTrace(ae1, 1);
        ensAssemblyexceptionTrace(ae2, 1);
    }

    /* Sort empty values towards the end of the AJAX List. */

    if(ae1 && (!ae2))
        return -1;

    if((!ae1) && (!ae2))
        return 0;

    if((!ae1) && ae2)
        return +1;

    if(ae1->ReferenceEnd < ae2->ReferenceEnd)
        result = +1;

    if(ae1->ReferenceEnd > ae2->ReferenceEnd)
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
** @nam7rule Ascending  Sort in ascending order
** @nam7rule Descending Sort in descending order
**
** @argrule Ascending aes [AjPList]  AJAX List of
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
** reference end element in ascending order.
**
** @param [u] aes [AjPList] AJAX List of Ensembl Assembly Exception objects
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
** @@
******************************************************************************/

AjBool ensListAssemblyexceptionSortReferenceEndAscending(AjPList aes)
{
    if(!aes)
        return ajFalse;

    ajListSort(aes, listAssemblyexceptionCompareReferenceEndAscending);

    return ajTrue;
}




/* @func ensListAssemblyexceptionSortReferenceEndDescending *******************
**
** Sort an AJAX List of Ensembl Assembly Exception objects by their
** reference end element in descending order.
**
** @param [u] aes [AjPList] AJAX List of Ensembl Assembly Exception objects
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
** @@
******************************************************************************/

AjBool ensListAssemblyexceptionSortReferenceEndDescending(AjPList aes)
{
    if(!aes)
        return ajFalse;

    ajListSort(aes, listAssemblyexceptionCompareReferenceEndDescending);

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
** @valrule * [EnsPAssemblyexceptionadaptor] Ensembl Assembly Exception Adaptor
**
** @fcategory new
******************************************************************************/




/* @funcstatic assemblyexceptionadaptorFetchAllbyStatement ********************
**
** Run a SQL statement against an Ensembl Assembly Exception Adaptor and
** consolidate the results into an AJAX List of Ensembl Assembly Exception
** objects.
**
** @param [u] aea [EnsPAssemblyexceptionadaptor] Ensembl Assembly
**                                               Exception Adaptor
** @param [r] statement [const AjPStr] SQL statement
** @param [u] aes [AjPList] AJAX List of Ensembl Assembly Exceptions
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
** @@
******************************************************************************/

static AjBool assemblyexceptionadaptorFetchAllbyStatement(
    EnsPAssemblyexceptionadaptor aea,
    const AjPStr statement,
    AjPList aes)
{
    ajint ori = 0;

    ajuint identifier = 0;
    ajuint erid       = 0;
    ajuint srid       = 0;
    ajuint erstart    = 0;
    ajuint srstart    = 0;
    ajuint erend      = 0;
    ajuint srend      = 0;

    AjPSqlstatement sqls = NULL;
    AjISqlrow sqli       = NULL;
    AjPSqlrow sqlr       = NULL;

    AjPStr typestr = NULL;

    EnsEAssemblyexceptionType aet = ensEAssemblyexceptionTypeNULL;

    EnsPAssemblyexception ae = NULL;

    if(!aea)
        return ajFalse;

    if(!statement)
        return ajFalse;

    if(!aes)
        return ajFalse;

    sqls = ensDatabaseadaptorSqlstatementNew(aea->Adaptor, statement);

    sqli = ajSqlrowiterNew(sqls);

    while(!ajSqlrowiterDone(sqli))
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

        if(!aet)
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

        ajListPushAppend(aes, (void*) ae);

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
** @param [u] aea [EnsPAssemblyexceptionadaptor] Ensembl Assembly
**                                               Exception Adaptor
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
** @@
******************************************************************************/

static AjBool assemblyexceptionadaptorCacheInit(
    EnsPAssemblyexceptionadaptor aea)
{
    ajuint* Pidentifier = NULL;

    AjPList list = NULL;
    AjPList aes  = NULL;

    AjPStr statement = NULL;

    EnsPAssemblyexception ae = NULL;

    if(!aea)
        return ajFalse;

    if(aea->CacheByReferenceSeqregion)
        return ajTrue;
    else
        aea->CacheByReferenceSeqregion = ensTableuintNewLen(0);

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

    while(ajListPop(aes, (void**) &ae))
    {
        list = (AjPList) ajTableFetchmodV(aea->CacheByReferenceSeqregion,
                                          (const void*) &ae->ReferenceSeqregion);

        if(!list)
        {
            AJNEW0(Pidentifier);

            *Pidentifier = ae->ReferenceSeqregion;

            list = ajListNew();

            ajTablePut(aea->CacheByReferenceSeqregion,
                       (void*) Pidentifier,
                       (void*) list);
        }

        ajListPushAppend(list, (void*) ae);
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
** @return [EnsPAssemblyexceptionadaptor] Ensembl Assembly Exception Adaptor
**                                        or NULL
** @@
******************************************************************************/

EnsPAssemblyexceptionadaptor ensAssemblyexceptionadaptorNew(
    EnsPDatabaseadaptor dba)
{
    EnsPAssemblyexceptionadaptor aea = NULL;

    if(!dba)
        return NULL;

    AJNEW0(aea);

    aea->Adaptor = dba;

    assemblyexceptionadaptorCacheInit(aea);

    return aea;
}




/* @section destructors *******************************************************
**
** Destruction destroys all internal data structures and frees the
** memory allocated for an Ensembl Assembly Exception Adaptor object.
**
** @fdata [EnsPAssemblyexceptionadaptor]
**
** @nam3rule Del Destroy (free) an Ensembl Assembly Exception Adaptor object
**
** @argrule * Paea [EnsPAssemblyexceptionadaptor*] Assembly Exception Adaptor
**                                                 object address
**
** @valrule * [void]
**
** @fcategory delete
******************************************************************************/




/* @funcstatic assemblyexceptionadaptorClearIdentifierCache *******************
**
** An ajTableMapDel "apply" function to clear the Ensembl Assembly Exception
** Adaptor-internal Ensembl Assembly Exception cache. This function clears the
** unsigned integer key data and the AJAX List value data, as well as the
** Ensembl Assembly Exceptions from the AJAX List.
**
** @param [u] key [void**] AJAX unsigned integer key data address
** @param [u] value [void**] AJAX List value data address
** @param [u] cl [void*] Standard, passed in from ajTableMapDel
** @see ajTableMapDel
**
** @return [void]
** @@
******************************************************************************/

static void assemblyexceptionadaptorClearIdentifierCache(void** key,
                                                         void** value,
                                                         void* cl)
{
    EnsPAssemblyexception ae = NULL;

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

    while(ajListPop(*((AjPList*) value), (void**) &ae))
        ensAssemblyexceptionDel(&ae);

    ajListFree((AjPList*) value);

    *key   = NULL;
    *value = NULL;

    return;
}




/* @funcstatic assemblyexceptionadaptorCacheExit ******************************
**
** Clears the Ensembl Assembly Exception cache in the
** Ensembl Assembly Exception Adaptor.
**
** @param [u] aea [EnsPAssemblyexceptionadaptor] Ensembl Assembly
**                                               Exception Adaptor
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
** @@
******************************************************************************/

static AjBool assemblyexceptionadaptorCacheExit(
    EnsPAssemblyexceptionadaptor aea)
{
    if(!aea)
        return ajFalse;

    if(!aea->CacheByReferenceSeqregion)
        return ajTrue;

    ajTableMapDel(aea->CacheByReferenceSeqregion,
                  assemblyexceptionadaptorClearIdentifierCache,
                  NULL);

    ajTableFree(&aea->CacheByReferenceSeqregion);

    return ajTrue;
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
** @@
******************************************************************************/

void ensAssemblyexceptionadaptorDel(EnsPAssemblyexceptionadaptor* Paea)
{
    EnsPAssemblyexceptionadaptor pthis = NULL;

    if(!Paea)
        return;

    if(!*Paea)
        return;

    pthis = *Paea;

    assemblyexceptionadaptorCacheExit(pthis);

    AJFREE(pthis);

    *Paea = NULL;

    return;
}




/* @section element retrieval *************************************************
**
** Functions for returning elements of an
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
** Get the Ensembl Database Adaptor element of an
** Ensembl Assembly Exception Adaptor.
**
** @param [u] aea [EnsPAssemblyexceptionadaptor]
** Ensembl Assembly Exception Adaptor
**
** @return [EnsPDatabaseadaptor] Ensembl Database Adaptor or NULL
** @@
******************************************************************************/

EnsPDatabaseadaptor ensAssemblyexceptionadaptorGetDatabaseadaptor(
    EnsPAssemblyexceptionadaptor aea)
{
    if(!aea)
        return NULL;

    return aea->Adaptor;
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
** @argrule * aea [const EnsPAssemblyexceptionadaptor] Ensembl Assembly
**                                                     Exception Adaptor
** @argrule FetchAll aes [AjPList] AJAX List of Ensembl Assembly Exception
**                                 objects
** @argrule FetchAllbyReferenceSeqregion srid [ajuint] Reference Ensembl
**                                                     Sequence Region
** @argrule FetchAllby aes [AjPList] AJAX List of Ensembl Assembly Exception
**                                   objects
**
** @valrule * [AjBool] ajTrue upon success, ajFalse otherwise
**
** @fcategory use
******************************************************************************/




/* @funcstatic assemblyexceptionadaptorFetchAll *******************************
**
** An ajTableMap "apply" function to return all Ensembl Assembly Exceptions
** from the Ensembl Assembly Exception Adaptor-internal cache.
**
** @param [u] key [const void*] AJAX unsigned integer key data address
** @param [u] value [void**] Ensembl Assembly Exception value data address
** @param [u] cl [void*] AJAX List of Ensembl Assembly Exception objects,
**                        passed in via ajTableMap
** @see ajTableMap
**
** @return [void]
** @@
******************************************************************************/

static void assemblyexceptionadaptorFetchAll(const void* key,
                                             void** value,
                                             void* cl)
{
    AjIList iter = NULL;

    EnsPAssemblyexception ae = NULL;

    if(!key)
        return;

    if(!value)
        return;

    if(!*value)
        return;

    if(!cl)
        return;

    iter = ajListIterNew(*((AjPList*) value));

    while(!ajListIterDone(iter))
    {
        ae = (EnsPAssemblyexception) ajListIterGet(iter);

        ajListPushAppend((AjPList) cl,
                         (void*) ensAssemblyexceptionNewRef(ae));
    }

    ajListIterDel(&iter);

    return;
}




/* @func ensAssemblyexceptionadaptorFetchAll **********************************
**
** Fetch all Ensembl Assembly Exceptions.
**
** The caller is responsible for deleting the Ensembl Assembly Exception
** objects before deleting the AJAX List.
**
** @param [r] aea [const EnsPAssemblyexceptionadaptor] Ensembl Assembly
**                                                     Exception Adaptor
** @param [u] aes [AjPList] AJAX List of Ensembl Assembly Exceptions
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
** @@
******************************************************************************/

AjBool ensAssemblyexceptionadaptorFetchAll(
    const EnsPAssemblyexceptionadaptor aea,
    AjPList aes)
{
    if(!aea)
        return ajFalse;

    if(!aes)
        return ajFalse;

    ajTableMap(aea->CacheByReferenceSeqregion,
               assemblyexceptionadaptorFetchAll,
               (void*) aes);

    return ajTrue;
}




/* @func ensAssemblyexceptionadaptorFetchAllbyReferenceSeqregion **************
**
** Fetch all Ensembl Assembly Exceptions by a reference
** Ensembl Sequence Region identifier.
**
** The caller is responsible for deleting the Ensembl Assembly Exception
** objects before deleting the AJAX List.
**
** @param [r] aea [const EnsPAssemblyexceptionadaptor] Ensembl Assembly
**                                                     Exception Adaptor
** @param [r] srid [ajuint] Reference Ensembl Sequence Region identifier
** @param [u] aes [AjPList] AJAX List of Ensembl Assembly Exceptions
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
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

    if(ajDebugTest("ensAssemblyexceptionadaptorFetchAllbyReferenceSeqregion"))
        ajDebug("ensAssemblyexceptionadaptorFetchAllbyReferenceSeqregion\n"
                "  aea %p\n"
                "  srid %u\n"
                "  aes %p\n",
                aea,
                srid,
                aes);

    if(!aea)
        return ajFalse;

    if(!aes)
        return ajFalse;

    list = (AjPList) ajTableFetchmodV(aea->CacheByReferenceSeqregion,
                                      (const void*) &srid);

    iter = ajListIterNew(list);

    while(!ajListIterDone(iter))
    {
        ae = (EnsPAssemblyexception) ajListIterGet(iter);

        ajListPushAppend(aes, (void*) ensAssemblyexceptionNewRef(ae));
    }

    ajListIterDel(&iter);

    return ajTrue;
}
