/******************************************************************************
** @source Ensembl Quality Check Sequence functions
**
** @author Copyright (C) 1999 Ensembl Developers
** @author Copyright (C) 2006 Michael K. Schuster
** @modified 2009 by Alan Bleasby for incorporation into EMBOSS core
** @version $Revision: 1.10 $
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

#include "ensqcsequence.h"




/* ==================================================================== */
/* ========================== private data ============================ */
/* ==================================================================== */




/* ==================================================================== */
/* ======================== private functions ========================= */
/* ==================================================================== */

static AjBool qcsequenceadaptorFetchAllBySQL(EnsPDatabaseadaptor dba,
                                             const AjPStr statement,
                                             EnsPAssemblymapper am,
                                             EnsPSlice slice,
                                             AjPList qcss);




/* @filesection ensqcsequence *************************************************
**
** @nam1rule ens Function belongs to the Ensembl library
**
******************************************************************************/




/* @datasection [EnsPQcsequence] QC Sequence **********************************
**
** Functions for manipulating Ensembl QC Sequence objects
**
** Bio::EnsEMBL::QC::Sequence CVS Revision:
**
** @nam2rule Qcsequence
**
******************************************************************************/




/* @section constructors ******************************************************
**
** All constructors return a new Ensembl QC Sequence by pointer.
** It is the responsibility of the user to first destroy any previous
** QC Sequence. The target pointer does not need to be initialised to
** NULL, but it is good programming practice to do so anyway.
**
** @fdata [EnsPQcsequence]
** @fnote None
**
** @nam3rule New Constructor
** @nam4rule NewObj Constructor with existing object
** @nam4rule NewRef Constructor by incrementing the reference counter
**
** @argrule Obj object [EnsPQcsequence] Ensembl QC Sequence
** @argrule Ref object [EnsPQcsequence] Ensembl QC Sequence
**
** @valrule * [EnsPQcsequence] Ensembl QC Sequence
**
** @fcategory new
******************************************************************************/




/* @func ensQcsequenceNew *****************************************************
**
** Default constructor for an Ensembl QC Sequence.
**
** @cc Bio::EnsEMBL::Storable::new
** @param [u] qcsa [EnsPQcsequenceadaptor] Ensembl QC Sequence Adaptor
** @param [r] identifier [ajuint] SQL database-internal identifier
** @cc Bio::EnsEMBL::QC::Sequence::new
** @param [u] qcdb [EnsPQcdatabase] Ensembl QC Database
** @param [u] name [AjPStr] Name
** @param [u] accession [AjPStr] Accession
** @param [r] version [ajuint] Version
** @param [u] type [AjPStr] Type
** @param [r] length [ajuint] Length
** @param [r] cdsstart [ajuint] CDS start
** @param [r] cdsend [ajuint] CDS end
** @param [r] cdsstrand [ajint] CDS strand
** @param [r] polya [ajuint] PolyA+ tail length
** @param [u] description [AjPStr] Description
**
** @return [EnsPQcsequence] Ensembl QC Sequence or NULL
** @@
******************************************************************************/

EnsPQcsequence ensQcsequenceNew(EnsPQcsequenceadaptor qcsa,
                                ajuint identifier,
                                EnsPQcdatabase qcdb,
                                AjPStr name,
                                AjPStr accession,
                                ajuint version,
                                AjPStr type,
                                ajuint length,
                                ajuint cdsstart,
                                ajuint cdsend,
                                ajint cdsstrand,
                                ajuint polya,
                                AjPStr description)
{
    EnsPQcsequence qcs = NULL;

    if(!qcdb)
        return NULL;

    if(!name)
        return NULL;

    if(!accession)
        return NULL;

    AJNEW0(qcs);

    qcs->Use = 1;

    qcs->Identifier = identifier;

    qcs->Adaptor = qcsa;

    qcs->Qcdatabase = ensQcdatabaseNewRef(qcdb);

    if(name)
        qcs->Name = ajStrNewRef(name);

    if(accession)
        qcs->Accession = ajStrNewRef(accession);

    qcs->Version = version;

    if(type)
        qcs->Type = ajStrNewRef(type);

    qcs->Length = length;

    qcs->CDSStart = cdsstart;

    qcs->CDSEnd = cdsend;

    qcs->CDSStrand = cdsstrand;

    qcs->PolyA = polya;

    if(description)
        qcs->Description = ajStrNewRef(description);

    return qcs;
}




/* @func ensQcsequenceNewObj **************************************************
**
** Object-based constructor function, which returns an independent object.
**
** @param [r] object [const EnsPQcsequence] Ensembl QC Sequence
**
** @return [EnsPQcsequence] Ensembl QC Sequence or NULL
** @@
******************************************************************************/

EnsPQcsequence ensQcsequenceNewObj(const EnsPQcsequence object)
{
    EnsPQcsequence qcs = NULL;

    if(!object)
        return NULL;

    AJNEW0(qcs);

    qcs->Use = 1;

    qcs->Identifier = object->Identifier;

    qcs->Adaptor = object->Adaptor;

    qcs->Qcdatabase = ensQcdatabaseNewRef(object->Qcdatabase);

    if(object->Name)
        qcs->Name = ajStrNewRef(object->Name);

    if(object->Accession)
        qcs->Accession = ajStrNewRef(object->Accession);

    qcs->Version = object->Version;

    if(object->Type)
        qcs->Type = ajStrNewRef(object->Type);

    qcs->Length = object->Length;

    qcs->CDSStart = object->CDSStart;

    qcs->CDSEnd = object->CDSEnd;

    qcs->CDSStrand = object->CDSStrand;

    qcs->PolyA = object->PolyA;

    if(object->Description)
        qcs->Description = ajStrNewRef(object->Description);

    return qcs;
}




/* @func ensQcsequenceNewRef **************************************************
**
** Ensembl Object referencing function, which returns a pointer to the
** Ensembl Object passed in and increases its reference count.
**
** @param [u] qcs [EnsPQcsequence] Ensembl QC Sequence
**
** @return [EnsPQcsequence] Ensembl QC Sequence
** @@
******************************************************************************/

EnsPQcsequence ensQcsequenceNewRef(EnsPQcsequence qcs)
{
    if(!qcs)
        return NULL;

    qcs->Use++;

    return qcs;
}




/* @section destructors *******************************************************
**
** Destruction destroys all internal data structures and frees the
** memory allocated for the Ensembl QC Sequence.
**
** @fdata [EnsPQcsequence]
** @fnote None
**
** @nam3rule Del Destroy (free) a QC Sequence object
**
** @argrule * Pqcs [EnsPQcsequence*] QC Sequence object address
**
** @valrule * [void]
**
** @fcategory delete
******************************************************************************/




/* @func ensQcsequenceDel *****************************************************
**
** Default destructor for an Ensembl QC Sequence.
**
** @param [d] Pqcs [EnsPQcsequence*] Ensembl QC Sequence address
**
** @return [void]
** @@
******************************************************************************/

void ensQcsequenceDel(EnsPQcsequence *Pqcs)
{
    EnsPQcsequence pthis = NULL;

    if(!Pqcs)
        return;

    if(!*Pqcs)
        return;

    if(ajDebugTest("ensQcsequenceDel"))
    {
        ajDebug("ensQcsequenceDel\n"
                "  *Pqcs %p\n",
                *Pqcs);

        ensQcsequenceTrace(*Pqcs, 1);
    }

    pthis = *Pqcs;

    pthis->Use--;

    if(pthis->Use)
    {
        *Pqcs = NULL;

        return;
    }

    ensQcdatabaseDel(&pthis->Qcdatabase);

    ajStrDel(&pthis->Name);
    ajStrDel(&pthis->Accession);
    ajStrDel(&pthis->Type);
    ajStrDel(&pthis->Description);

    AJFREE(pthis);

    *Pqcs = NULL;

    return;
}




/* @section element retrieval *************************************************
**
** Functions for returning elements of an Ensembl QC Sequence object.
**
** @fdata [EnsPQcsequence]
** @fnote None
**
** @nam3rule Get Return QC Sequence attribute(s)
** @nam4rule GetAdaptor Return the Ensembl QC Sequence Adaptor
** @nam4rule GetIdentifier Return the SQL database-internal identifier
**
** @argrule * qcs [const EnsPQcsequence] QC Sequence
**
** @valrule Adaptor [EnsPQcsequenceadaptor] Ensembl QC Sequence Adaptor
** @valrule Identifier [ajuint] SQL database-internal identifier
** @valrule Qcdatabase [EnsPQcdatabase] Ensembl QC Database
** @valrule Name [AjPStr] Name
** @valrule Accession [AjPStr] Accession
** @valrule Version [ajuint] Version
** @valrule Type [AjPStr] Type
** @valrule Length [ajuint] Length
** @valrule CDSStart [ajuint] CDS Start
** @valrule CDSEnd [ajuint] CDS End
** @valrule CDSStrand [ajuint] CDS Strand
** @valrule PolyA [ajuint] PolyA tail length
** @valrule Description [AjPStr] Description
**
** @fcategory use
******************************************************************************/




/* @func ensQcsequenceGetAdaptor **********************************************
**
** Get the Ensembl QC Sequence Adaptor element of an Ensembl QC Sequence.
**
** @param [r] qcs [const EnsPQcsequence] Ensembl QC Sequence
**
** @return [EnsPQcsequenceadaptor] Ensembl QC Sequence Adaptor
** @@
******************************************************************************/

EnsPQcsequenceadaptor ensQcsequenceGetAdaptor(const EnsPQcsequence qcs)
{
    if(!qcs)
        return NULL;

    return qcs->Adaptor;
}




/* @func ensQcsequenceGetIdentifier *******************************************
**
** Get the SQL database-internal identifier element of an Ensembl QC Sequence.
**
** @param [r] qcs [const EnsPQcsequence] Ensembl QC Sequence
**
** @return [ajuint] Internal database identifier
** @@
******************************************************************************/

ajuint ensQcsequenceGetIdentifier(const EnsPQcsequence qcs)
{
    if(!qcs)
        return 0;

    return qcs->Identifier;
}




/* @func ensQcsequenceGetQcdatabase *******************************************
**
** Get the Ensembl QC Database element of an Ensembl QC Sequence.
**
** @param [r] qcs [const EnsPQcsequence] Ensembl QC Sequence
**
** @return [EnsPQcdatabase] Ensembl QC Database
** @@
******************************************************************************/

EnsPQcdatabase ensQcsequenceGetQcdatabase(const EnsPQcsequence qcs)
{
    if(!qcs)
        return NULL;

    return qcs->Qcdatabase;
}




/* @func ensQcsequenceGetName *************************************************
**
** Get the name element of an Ensembl QC Sequence.
**
** @param [r] qcs [const EnsPQcsequence] Ensembl QC Sequence
**
** @return [AjPStr] Name
** @@
******************************************************************************/

AjPStr ensQcsequenceGetName(const EnsPQcsequence qcs)
{
    if(!qcs)
        return NULL;

    return qcs->Name;
}




/* @func ensQcsequenceGetAccession ********************************************
**
** Get the accession element of an Ensembl QC Sequence.
**
** @param [r] qcs [const EnsPQcsequence] Ensembl QC Sequence
**
** @return [AjPStr] Accession
** @@
******************************************************************************/

AjPStr ensQcsequenceGetAccession(const EnsPQcsequence qcs)
{
    if(!qcs)
        return NULL;

    return qcs->Accession;
}




/* @func ensQcsequenceGetVersion **********************************************
**
** Get the version element of an Ensembl QC Sequence.
**
** @param [r] qcs [const EnsPQcsequence] Ensembl QC Sequence
**
** @return [ajuint] Version
** @@
******************************************************************************/

ajuint ensQcsequenceGetVersion(const EnsPQcsequence qcs)
{
    if(!qcs)
        return 0;

    return qcs->Version;
}




/* @func ensQcsequenceGetType *************************************************
**
** Get the type element of an Ensembl QC Sequence.
**
** @param [r] qcs [const EnsPQcsequence] Ensembl QC Sequence
**
** @return [AjPStr] Type
** @@
******************************************************************************/

AjPStr ensQcsequenceGetType(const EnsPQcsequence qcs)
{
    if(!qcs)
        return NULL;

    return qcs->Type;
}




/* @func ensQcsequenceGetLength ***********************************************
**
** Get the length element of an Ensembl QC Sequence.
**
** @param [r] qcs [const EnsPQcsequence] Ensembl QC Sequence
**
** @return [ajuint] Length
** @@
******************************************************************************/

ajuint ensQcsequenceGetLength(const EnsPQcsequence qcs)
{
    if(!qcs)
        return 0;

    return qcs->Length;
}




/* @func ensQcsequenceGetCDSStart *********************************************
**
** Get the CDS start element of an Ensembl QC Sequence.
**
** @param [r] qcs [const EnsPQcsequence] Ensembl QC Sequence
**
** @return [ajuint] CDS start
** @@
******************************************************************************/

ajuint ensQcsequenceGetCDSStart(const EnsPQcsequence qcs)
{
    if(!qcs)
        return 0;

    return qcs->CDSStart;
}




/* @func ensQcsequenceGetCDSEnd ***********************************************
**
** Get the CDS end element of an Ensembl QC Sequence.
**
** @param [r] qcs [const EnsPQcsequence] Ensembl QC Sequence
**
** @return [ajuint] CDS start
** @@
******************************************************************************/

ajuint ensQcsequenceGetCDSEnd(const EnsPQcsequence qcs)
{
    if(!qcs)
        return 0;

    return qcs->CDSEnd;
}




/* @func ensQcsequenceGetCDSStrand ********************************************
**
** Get the CDS strand element of an Ensembl QC Sequence.
**
** @param [r] qcs [const EnsPQcsequence] Ensembl QC Sequence
**
** @return [ajint] CDS strand
** @@
******************************************************************************/

ajint ensQcsequenceGetCDSStrand(const EnsPQcsequence qcs)
{
    if(!qcs)
        return 0;

    return qcs->CDSStrand;
}




/* @func ensQcsequenceGetPolyA ************************************************
**
** Get the PolyA+ tail length element of an Ensembl QC Sequence.
**
** @param [r] qcs [const EnsPQcsequence] Ensembl QC Sequence
**
** @return [ajuint] PolyA+ tail length
** @@
******************************************************************************/

ajuint ensQcsequenceGetPolyA(const EnsPQcsequence qcs)
{
    if(!qcs)
        return 0;

    return qcs->PolyA;
}




/* @func ensQcsequenceGetDescription ******************************************
**
** Get the description element of an Ensembl QC Sequence.
**
** @param [r] qcs [const EnsPQcsequence] Ensembl QC Sequence
**
** @return [AjPStr] Description
** @@
******************************************************************************/

AjPStr ensQcsequenceGetDescription(const EnsPQcsequence qcs)
{
    if(!qcs)
        return NULL;

    return qcs->Description;
}




/* @section modifiers *********************************************************
**
** Functions for assigning elements of an Ensembl QC Sequence object.
**
** @fdata [EnsPQcsequence]
** @fnote None
**
** @nam3rule Set Set one element of a QC Sequence
** @nam4rule SetAdaptor Set the Ensembl QC Sequence Adaptor
** @nam4rule SetIdentifier Set the SQL database-internal identifier
** @nam4rule SetQcdatabase Set the Ensembl QC Database
** @nam4rule SetName Set the name
** @nam4rule SetAccession Set the accession
** @nam4rule SetVersion Set the version
** @nam4rule SetType Set the type
** @nam4rule SetLength Set the length
** @nam4rule SetCDSStart Set the CDS start
** @nam4rule SetCDSEnd Set the CDS end
** @nam4rule SetCDSStrand Set the CDS strand
** @nam4rule SetPolyA Set the PolyA+ tail length
** @nam4rule SetDescription Set the description
**
** @argrule * qcs [EnsPQcsequence] Ensembl QC Sequence
**
** @valrule * [AjBool] ajTrue upon success, ajFalse otherwise
**
** @fcategory modify
******************************************************************************/




/* @func ensQcsequenceSetAdaptor **********************************************
**
** Set the Ensembl QC Sequence Adaptor element of an Ensembl QC Sequence.
**
** @param [u] qcs [EnsPQcsequence] Ensembl QC Sequence
** @param [uN] qcsa [EnsPQcsequenceadaptor] Ensembl QC Sequence Adaptor
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
** @@
******************************************************************************/

AjBool ensQcsequenceSetAdaptor(EnsPQcsequence qcs, EnsPQcsequenceadaptor qcsa)
{
    if(!qcs)
        return ajFalse;

    qcs->Adaptor = qcsa;

    return ajTrue;
}




/* @func ensQcsequenceSetIdentifier *******************************************
**
** Set the SQL database-internal identifier element of an Ensembl QC Sequence.
**
** @param [u] qcs [EnsPQcsequence] Ensembl QC Sequence
** @param [r] identifier [ajuint] SQL database-internal identifier
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
** @@
******************************************************************************/

AjBool ensQcsequenceSetIdentifier(EnsPQcsequence qcs, ajuint identifier)
{
    if(!qcs)
        return ajFalse;

    qcs->Identifier = identifier;

    return ajTrue;
}




/* @func ensQcsequenceSetQcdatabase *******************************************
**
** Set the Ensembl QC Database element of an Ensembl QC Sequence.
**
** @param [u] qcs [EnsPQcsequence] Ensembl QC Sequence
** @param [uN] qcdb [EnsPQcdatabase] Ensembl QC Database
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
** @@
******************************************************************************/

AjBool ensQcsequenceSetQcdatabase(EnsPQcsequence qcs, EnsPQcdatabase qcdb)
{
    if(!qcs)
        return ajFalse;

    ensQcdatabaseDel(&qcs->Qcdatabase);

    qcs->Qcdatabase = ensQcdatabaseNewRef(qcdb);

    return ajTrue;
}




/* @func ensQcsequenceSetName *************************************************
**
** Set the name element of an Ensembl QC Sequence.
**
** @param [u] qcs [EnsPQcsequence] Ensembl QC Sequence
** @param [uN] name [AjPStr] Name
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
** @@
******************************************************************************/

AjBool ensQcsequenceSetName(EnsPQcsequence qcs, AjPStr name)
{
    if(!qcs)
        return ajFalse;

    ajStrDel(&qcs->Name);

    qcs->Name = ajStrNewRef(name);

    return ajTrue;
}




/* @func ensQcsequenceSetAccession ********************************************
**
** Set the accession element of an Ensembl QC Sequence.
**
** @param [u] qcs [EnsPQcsequence] Ensembl QC Sequence
** @param [uN] accession [AjPStr] Accession
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
** @@
******************************************************************************/

AjBool ensQcsequenceSetAccession(EnsPQcsequence qcs, AjPStr accession)
{
    if(!qcs)
        return ajFalse;

    ajStrDel(&qcs->Accession);

    qcs->Accession = ajStrNewRef(accession);

    return ajTrue;
}




/* @func ensQcsequenceSetVersion **********************************************
**
** Set the version element of an Ensembl QC Sequence.
**
** @param [u] qcs [EnsPQcsequence] Ensembl QC Sequence
** @param [r] version [ajuint] Version
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
** @@
******************************************************************************/

AjBool ensQcsequenceSetVersion(EnsPQcsequence qcs, ajuint version)
{
    if(!qcs)
        return ajFalse;

    qcs->Version = version;

    return ajTrue;
}




/* @func ensQcsequenceSetType *************************************************
**
** Set the type element of an Ensembl QC Sequence.
**
** @param [u] qcs [EnsPQcsequence] Ensembl QC Sequence
** @param [uN] type [AjPStr] Type
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
** @@
******************************************************************************/

AjBool ensQcsequenceSetType(EnsPQcsequence qcs, AjPStr type)
{
    if(!qcs)
        return ajFalse;

    ajStrDel(&qcs->Type);

    qcs->Type = ajStrNewRef(type);

    return ajTrue;
}




/* @func ensQcsequenceSetLength ***********************************************
**
** Set the length element of an Ensembl QC Sequence.
**
** @param [u] qcs [EnsPQcsequence] Ensembl QC Sequence
** @param [r] length [ajuint] Length
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
** @@
******************************************************************************/

AjBool ensQcsequenceSetLength(EnsPQcsequence qcs, ajuint length)
{
    if(!qcs)
        return ajFalse;

    qcs->Length = length;

    return ajTrue;
}




/* @func ensQcsequenceSetCDSStart *********************************************
**
** Set the CDS start element of an Ensembl QC Sequence.
**
** @param [u] qcs [EnsPQcsequence] Ensembl QC Sequence
** @param [r] cdsstart [ajuint] CDS start
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
** @@
******************************************************************************/

AjBool ensQcsequenceSetCDSStart(EnsPQcsequence qcs, ajuint cdsstart)
{
    if(!qcs)
        return ajFalse;

    qcs->CDSStart = cdsstart;

    return ajTrue;
}




/* @func ensQcsequenceSetCDSEnd ***********************************************
**
** Set the CDS end element of an Ensembl QC Sequence.
**
** @param [u] qcs [EnsPQcsequence] Ensembl QC Sequence
** @param [r] cdsend [ajuint] CDS end
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
** @@
******************************************************************************/

AjBool ensQcsequenceSetCDSEnd(EnsPQcsequence qcs, ajuint cdsend)
{
    if(!qcs)
        return ajFalse;

    qcs->CDSEnd = cdsend;

    return ajTrue;
}




/* @func ensQcsequenceSetCDSStrand ********************************************
**
** Set the CDS strand element of an Ensembl QC Sequence.
**
** @param [u] qcs [EnsPQcsequence] Ensembl QC Sequence
** @param [r] cdsstrand [ajint] CDS strand
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
** @@
******************************************************************************/

AjBool ensQcsequenceSetCDSStrand(EnsPQcsequence qcs, ajint cdsstrand)
{
    if(!qcs)
        return ajFalse;

    qcs->CDSStrand = cdsstrand;

    return ajTrue;
}




/* @func ensQcsequenceSetPolyA ************************************************
**
** Set the PolyA+ tail length element of an Ensembl QC Sequence.
**
** @param [u] qcs [EnsPQcsequence] Ensembl QC Sequence
** @param [r] polya [ajuint] PolyA+ tail length
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
** @@
******************************************************************************/

AjBool ensQcsequenceSetPolyA(EnsPQcsequence qcs, ajuint polya)
{
    if(!qcs)
        return ajFalse;

    qcs->PolyA = polya;

    return ajTrue;
}




/* @func ensQcsequenceSetDescription ******************************************
**
** Set the description element of an Ensembl QC Sequence.
**
** @param [u] qcs [EnsPQcsequence] Ensembl QC Sequence
** @param [uN] description [AjPStr] Description
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
** @@
******************************************************************************/

AjBool ensQcsequenceSetDescription(EnsPQcsequence qcs, AjPStr description)
{
    if(!qcs)
        return ajFalse;

    ajStrDel(&qcs->Description);

    qcs->Description = ajStrNewRef(description);

    return ajTrue;
}




/* @func ensQcsequenceGetMemsize **********************************************
**
** Get the memory size in bytes of an Ensembl QC Sequence.
**
** @param [r] qcs [const EnsPQcsequence] Ensembl QC Sequence
**
** @return [ajulong] Memory size
** @@
******************************************************************************/

ajulong ensQcsequenceGetMemsize(const EnsPQcsequence qcs)
{
    ajulong size = 0;

    if(!qcs)
        return 0;

    size += sizeof (EnsOQcsequence);

    size += ensQcdatabaseGetMemsize(qcs->Qcdatabase);

    if(qcs->Name)
    {
        size += sizeof (AjOStr);

        size += ajStrGetRes(qcs->Name);
    }

    if(qcs->Accession)
    {
        size += sizeof (AjOStr);

        size += ajStrGetRes(qcs->Accession);
    }

    if(qcs->Type)
    {
        size += sizeof (AjOStr);

        size += ajStrGetRes(qcs->Type);
    }

    if(qcs->Description)
    {
        size += sizeof (AjOStr);

        size += ajStrGetRes(qcs->Description);
    }

    return size;
}




/* @section debugging *********************************************************
**
** Functions for reporting of an Ensembl QC Sequence object.
**
** @fdata [EnsPQcsequence]
** @nam3rule Trace Report Ensembl QC Sequence elements to debug file
**
** @argrule Trace qcs [const EnsPQcsequence] Ensembl QC Sequence
** @argrule Trace level [ajuint] Indentation level
**
** @valrule * [AjBool] ajTrue upon success, ajFalse otherwise
**
** @fcategory misc
******************************************************************************/




/* @func ensQcsequenceTrace ***************************************************
**
** Trace an Ensembl QC Sequence.
**
** @param [r] qcs [const EnsPQcsequence] Ensembl QC Sequence
** @param [r] level [ajuint] Indentation level
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
** @@
******************************************************************************/

AjBool ensQcsequenceTrace(const EnsPQcsequence qcs, ajuint level)
{
    AjPStr indent = NULL;

    if(!qcs)
        return ajFalse;

    indent = ajStrNew();

    ajStrAppendCountK(&indent, ' ', level * 2);

    ajDebug("%SensQcsequenceTrace %p\n"
            "%S  Use %u\n"
            "%S  Identifier %u\n"
            "%S  Adaptor %p\n"
            "%S  Qcdatabase %p\n"
            "%S  Name '%S'\n"
            "%S  Accession '%S'\n"
            "%S  Version %u\n"
            "%S  Type '%S'\n"
            "%S  Length %u\n"
            "%S  CDSStart %u\n"
            "%S  CDSEnd %u\n"
            "%S  CDSStrand %d\n"
            "%S  PolyA %u\n"
            "%S  Description '%S'\n",
            indent, qcs,
            indent, qcs->Use,
            indent, qcs->Identifier,
            indent, qcs->Adaptor,
            indent, qcs->Qcdatabase,
            indent, qcs->Name,
            indent, qcs->Accession,
            indent, qcs->Version,
            indent, qcs->Type,
            indent, qcs->Length,
            indent, qcs->CDSStart,
            indent, qcs->CDSEnd,
            indent, qcs->CDSStrand,
            indent, qcs->PolyA,
            indent, qcs->Description);

    ensQcdatabaseTrace(qcs->Qcdatabase, 1);

    ajStrDel(&indent);

    return ajTrue;
}




/* @func ensQcsequenceGetQcdatabaseIdentifier *********************************
**
** Get the Ensembl QC Database identifier of an Ensembl QC Sequence.
**
** @param [r] qcs [const EnsPQcsequence] Ensembl QC Sequence
**
** @return [ajuint] Ensembl QC Database identifier
** @@
******************************************************************************/

ajuint ensQcsequenceGetQcdatabaseIdentifier(const EnsPQcsequence qcs)
{
    if(!qcs)
        return 0;

    return ensQcdatabaseGetIdentifier(qcs->Qcdatabase);
}




/* @func ensQcsequenceMatch ***************************************************
**
** Tests for matching two Ensembl QC Sequences.
**
** @param [r] qcs1 [const EnsPQcsequence] First Ensembl QC Sequence
** @param [r] qcs2 [const EnsPQcsequence] Second Ensembl QC Sequence
**
** @return [AjBool] ajTrue if the Ensembl QC Sequences are equal
** @@
** The comparison is based on initial pointer equality and if that fails
** each element is compared.
******************************************************************************/

AjBool ensQcsequenceMatch(const EnsPQcsequence qcs1,
                          const EnsPQcsequence qcs2)
{
    if(!qcs1)
        return ajFalse;

    if(!qcs2)
        return ajFalse;

    if(qcs1 == qcs2)
        return ajTrue;

    if(qcs1->Identifier != qcs2->Identifier)
        return ajFalse;

    if((qcs1->Adaptor && qcs2->Adaptor) && (qcs1->Adaptor != qcs2->Adaptor))
        return ajFalse;

    if(!ensQcdatabaseMatch(qcs1->Qcdatabase, qcs2->Qcdatabase))
        return ajFalse;

    if(!ajStrMatchCaseS(qcs1->Name, qcs2->Name))
        return ajFalse;

    if(!ajStrMatchCaseS(qcs1->Accession, qcs2->Accession))
        return ajFalse;

    if(!ajStrMatchCaseS(qcs1->Description, qcs2->Description))
        return ajFalse;

    if(!ajStrMatchCaseS(qcs1->Type, qcs2->Type))
        return ajFalse;

    if(qcs1->Version != qcs2->Version)
        return ajFalse;

    if(qcs1->Length != qcs2->Length)
        return ajFalse;

    if(qcs1->CDSStart != qcs2->CDSStart)
        return ajFalse;

    if(qcs1->CDSEnd != qcs2->CDSEnd)
        return ajFalse;

    if(qcs1->CDSStrand != qcs2->CDSStrand)
        return ajFalse;

    if(qcs1->PolyA != qcs2->PolyA)
        return ajFalse;

    return ajTrue;
}




/* @func ensHTMLEncodeSGMLID **************************************************
**
** Fetch a valid HTML 4.01 identifier or name token from an AJAX String.
** The caller is responsible for deleting the AJAX String.
**
** The HTML 4.01 standard defines the content of id attributes as the SGML
** ID and NAME tokens, as defined in
** "Information Processing -- Text and Office Systems --
** Standard Generalized Markup Language (SGML)", ISO 8879:1986.
** http://www.iso.ch/cate/d16387.html
** http://www.w3.org/TR/html401/struct/global.html#h-7.5.2
**
** ID and NAME tokens must begin with a letter ([A-Za-z]) and may be followed
** by any number of letters, digits ([0-9]), hyphens ("-"), underscores ("_"),
** colons (":"), and periods (".").
** http://www.w3.org/TR/html401/types.html#type-name
**
** @param [w] Pstr [AjPStr*] AjAX String
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
** @@
******************************************************************************/

AjBool ensHTMLEncodeSGMLID(AjPStr *Pstr)
{
/* FIXME: This function should move! */

    const char *txtstr = NULL;

    if(!Pstr)
        return ajFalse;

    ajStrExchangeSetRestCK(Pstr,
                           "-."
                           "0123456789"
                           ":"
                           "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
                           "_"
                           "abcdefghijklmnopqrstuvwxyz",
                           '_');

    txtstr = ajStrGetPtr(*Pstr);

    if(!
       ((*txtstr >= 'A' && *txtstr <= 'Z') ||
        (*txtstr >= 'a' && *txtstr <= 'z')))
        ajStrInsertC(Pstr, 0, "ID_");

    return ajTrue;
}




/* @func ensHTMLEncodeEntities ************************************************
**
** Encode HTML entities in an AJAX String.
**
**
** http://www.w3.org/TR/html401/charset.html#h-5.3.2
**
** @param [r] Pstr [AjPStr*] URL string
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
** @@
** NOTE: Since EMBOSS is limited to ASCII only " & < and > need replacing.
******************************************************************************/

AjBool ensHTMLEncodeEntities(AjPStr *Pstr)
{
/* FIXME: This function should move! */

    if(!Pstr)
        return ajFalse;

    ajStrExchangeCC(Pstr, "\"", "&quot;");
    ajStrExchangeCC(Pstr, "&", "&amp;");
    ajStrExchangeCC(Pstr, "<", "&lt;");
    ajStrExchangeCC(Pstr, ">", "&gt;");

    return ajTrue;
}




/* @func ensQcsequenceFetchExternalURL ****************************************
**
** Fetch an external URL for an Ensembl QC Sequence.
**
** @param [r] qcs [const EnsPQcsequence] Ensembl QC Sequence
** @param [r] Pstr [AjPStr*] URL string
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
** @@
******************************************************************************/

AjBool ensQcsequenceFetchExternalURL(const EnsPQcsequence qcs, AjPStr *Pstr)
{
    AjPStr exturl = NULL;
    AjPStr version = NULL;

    EnsPQcdatabase qcdb = NULL;

    if(!qcs)
        return ajFalse;

    if(!Pstr)
        return ajFalse;

    qcdb = ensQcsequenceGetQcdatabase(qcs);

    exturl = ensQcdatabaseGetExternalURL(qcdb);

    if(exturl && ajStrGetLen(exturl))
    {
        ajStrAssignS(Pstr, exturl);

        ajStrExchangeCS(Pstr, "###NAME###", qcs->Name);

        ajStrExchangeCS(Pstr, "###ACCESSION###", qcs->Accession);

        version = ajFmtStr("%d", qcs->Version);

        ajStrExchangeCS(Pstr, "###VERSION###", version);

        ajStrDel(&version);
    }

    /*
    ** TODO: Replace HTML entities via a suitable library function?
    **
    ** FIXME: Is this actually neccessary here?
    ** Shouldn't that be done when inserting into a HTML attribute like href?
    */

    return ajTrue;
}




/* @func ensQcsequenceFetchExternalAnchor *************************************
**
** Fetch an external HTML anchor for an Ensembl QC Sequence.
**
** Returns a complete HTML anchor element, which displays the sequence name
** and links it to the corresponding sequence entry in the external source
** database. Linking is dependent on template information in the
** 'sequence_db.external_url' field of the Ensembl QC database. Although the
** sequence name is displayed, the link is based on ###NAME###, ###ACCESSION###
** or ###VERSION### placeholders in the template URL. If no template is
** available, just an anchor element the name set as 'id' attribute is
** returned.
**
** @param [r] qcs [const EnsPQcsequence] Ensembl QC Sequence
** @param [w] Pstr [AjPStr*] Anchor string
** @param [r] htmlid [AjBool] Set the HTML id attribute in the anchor element
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
** @@
******************************************************************************/

AjBool ensQcsequenceFetchExternalAnchor(const EnsPQcsequence qcs,
                                        AjPStr *Pstr,
                                        AjBool htmlid)
{
    AjPStr sgmlid  = NULL;
    AjPStr exturl  = NULL;
    AjPStr qcsname = NULL;
    AjPStr qcdname = NULL;

    if(!qcs)
        return ajFalse;

    if(!Pstr)
        return ajFalse;

    ensQcsequenceFetchExternalURL(qcs, &exturl);

    if(exturl && ajStrGetLen(exturl))
    {
        ajStrAppendC(Pstr, "<a ");

        if(htmlid)
        {
            sgmlid = ajStrNewS(qcs->Name);

            ensHTMLEncodeSGMLID(&sgmlid);

            ajFmtPrintAppS(Pstr, "id=\"%S\" ", sgmlid);

            ajStrDel(&sgmlid);
        }

        ensHTMLEncodeEntities(&exturl);

        qcdname = ajStrNewS(ensQcdatabaseGetName(qcs->Qcdatabase));

        ensHTMLEncodeEntities(&qcdname);

        qcsname = ajStrNewS(qcs->Name);

        ensHTMLEncodeEntities(&qcsname);

        ajFmtPrintAppS(Pstr,
                       "href=\"%S\" target=\"%S\">%S</a>",
                       exturl,
                       qcdname,
                       qcsname);

        ajStrDel(&qcdname);

        ajStrDel(&qcsname);
    }
    else
    {
        if(htmlid)
        {
            sgmlid = ajStrNewS(qcs->Name);

            ensHTMLEncodeSGMLID(&sgmlid);

            qcsname = ajStrNewS(qcs->Name);

            ensHTMLEncodeEntities(&qcsname);

            ajFmtPrintAppS(Pstr,
                           "<a id=\"%S\">%S</a>",
                           sgmlid,
                           qcsname);

            ajStrDel(&sgmlid);

            ajStrDel(&qcsname);
        }
        else
            ajStrAppendS(Pstr, qcs->Name);
    }

    ajStrDel(&exturl);

    return ajTrue;
}




/* @func ensQcsequenceFetchInternalAnchor *************************************
**
** Fetch an internal HTML anchor for an Ensembl QC Sequence.
**
** Returns a complete HTML anchor element, which displays the sequence name
** and links it to the corresponding document-internal anchor element.
**
** @param [r] qcs [const EnsPQcsequence] Ensembl QC Sequence
** @param [w] Pstr [AjPStr*] Anchor string
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
** @@
******************************************************************************/

AjBool ensQcsequenceFetchInternalAnchor(const EnsPQcsequence qcs,
                                        AjPStr *Pstr)
{
    AjPStr sgmlid = NULL;
    AjPStr name   = NULL;

    if(!qcs)
        return ajFalse;

    if(!Pstr)
        return ajFalse;

    sgmlid = ajStrNewS(qcs->Name);

    ensHTMLEncodeSGMLID(&sgmlid);

    name = ajStrNewS(qcs->Name);

    ensHTMLEncodeEntities(&name);

    ajFmtPrintAppS(Pstr, "<a href=\"#%S\">%S</a>", sgmlid, name);

    ajStrDel(&sgmlid);
    ajStrDel(&name);

    return ajTrue;
}




/* @datasection [EnsPQcsequenceadaptor] QC Sequence Adaptor *******************
**
** Functions for manipulating Ensembl QC Sequence Adaptor objects
**
** Bio::EnsEMBL::QC::DBSQL::Sequenceadaptor CVS Revision:
**
** @nam2rule Qcsequenceadaptor
**
******************************************************************************/

static const char *qcsequenceadaptorTables[] =
{
    "sequence",
    (const char *) NULL
};




static const char *qcsequenceadaptorColumns[] =
{
    "sequence.sequence_id",
    "sequence.sequence_db_id",
    "sequence.name",
    "sequence.accession",
    "sequence.version",
    "sequence.type",
    "sequence.length",
    "sequence.cdsstart",
    "sequence.cdend",
    "sequence.cdsstrand",
    "sequence.polya",
    "sequence.description",
    (const char *) NULL
};




static EnsOBaseadaptorLeftJoin qcsequenceadaptorLeftJoin[] =
{
    {(const char*) NULL, (const char*) NULL}
};




static const char *qcsequenceadaptorDefaultCondition =
    (const char*) NULL;




static const char *qcsequenceadaptorFinalCondition =
    (const char *) NULL;




/* @funcstatic qcsequenceadaptorFetchAllBySQL *********************************
**
** Run a SQL statement against an Ensembl Database Adaptor and consolidate the
** results into an AJAX List of Ensembl QC Sequence objects.
**
** @param [u] dba [EnsPDatabaseadaptor] Ensembl Database Adaptor
** @param [r] statement [const AjPStr] SQL statement
** @param [uN] am [EnsPAssemblymapper] Ensembl Assembly Mapper
** @param [uN] slice [EnsPSlice] Ensembl Slice
** @param [u] qcss [AjPList] AJAX List of Ensembl QC Sequences
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
** @@
******************************************************************************/

static AjBool qcsequenceadaptorFetchAllBySQL(EnsPDatabaseadaptor dba,
                                             const AjPStr statement,
                                             EnsPAssemblymapper am,
                                             EnsPSlice slice,
                                             AjPList qcss)
{
    ajuint identifier = 0;
    ajuint databaseid = 0;
    ajuint version    = 0;
    ajuint length     = 0;
    ajuint cdsstart   = 0;
    ajuint cdsend     = 0;
    ajint cdsstrand   = 0;
    ajuint polya      = 0;

    AjPSqlstatement sqls = NULL;
    AjISqlrow sqli       = NULL;
    AjPSqlrow sqlr       = NULL;

    AjPStr name        = NULL;
    AjPStr accession   = NULL;
    AjPStr type        = NULL;
    AjPStr description = NULL;

    EnsPQcsequence qcs         = NULL;
    EnsPQcsequenceadaptor qcsa = NULL;

    EnsPQcdatabase qcdb         = NULL;
    EnsPQcdatabaseadaptor qcdba = NULL;

    if(ajDebugTest("qcsequenceadaptorFetchAllBySQL"))
        ajDebug("qcsequenceadaptorFetchAllBySQL\n"
                "  dba %p\n"
                "  statement %p\n"
                "  am %p\n"
                "  slice %p\n"
                "  qcss %p\n",
                dba,
                statement,
                am,
                slice,
                qcss);

    if(!dba)
        return ajFalse;

    if(!statement)
        return ajFalse;

    if(!qcss)
        return ajFalse;

    qcdba = ensRegistryGetQcdatabaseadaptor(dba);

    qcsa  = ensRegistryGetQcsequenceadaptor(dba);

    sqls  = ensDatabaseadaptorSqlstatementNew(dba, statement);

    sqli  = ajSqlrowiterNew(sqls);

    while(!ajSqlrowiterDone(sqli))
    {
        identifier  = 0;
        databaseid  = 0;
        name        = ajStrNew();
        accession   = ajStrNew();
        version     = 0;
        type        = ajStrNew();
        length      = 0;
        cdsstart    = 0;
        cdsend      = 0;
        cdsstrand   = 0;
        polya       = 0;
        description = ajStrNew();

        sqlr = ajSqlrowiterGet(sqli);

        ajSqlcolumnToUint(sqlr, &identifier);
        ajSqlcolumnToUint(sqlr, &databaseid);
        ajSqlcolumnToStr(sqlr, &name);
        ajSqlcolumnToStr(sqlr, &accession);
        ajSqlcolumnToUint(sqlr, &version);
        ajSqlcolumnToStr(sqlr, &type);
        ajSqlcolumnToUint(sqlr, &length);
        ajSqlcolumnToUint(sqlr, &cdsstart);
        ajSqlcolumnToUint(sqlr, &cdsend);
        ajSqlcolumnToInt(sqlr, &cdsstrand);
        ajSqlcolumnToUint(sqlr, &polya);
        ajSqlcolumnToStr(sqlr, &description);

        ensQcdatabaseadaptorFetchByIdentifier(qcdba,
                                              databaseid,
                                              &qcdb);

        qcs = ensQcsequenceNew(qcsa,
                               identifier,
                               qcdb,
                               name,
                               accession,
                               version,
                               type,
                               length,
                               cdsstart,
                               cdsend,
                               cdsstrand,
                               polya,
                               description);

        ajListPushAppend(qcss, (void *) qcs);

        ensQcdatabaseDel(&qcdb);

        ajStrDel(&name);
        ajStrDel(&accession);
        ajStrDel(&type);
        ajStrDel(&description);
    }

    ajSqlrowiterDel(&sqli);

    ensDatabaseadaptorSqlstatementDel(dba, &sqls);

    return ajTrue;
}




/* @section constructors ******************************************************
**
** All constructors return a new Ensembl QC Sequence Adaptor by pointer.
** It is the responsibility of the user to first destroy any previous
** QC Sequence Adaptor. The target pointer does not need to be
** initialised to NULL, but it is good programming practice to do so anyway.
**
** @fdata [EnsPQcsequenceadaptor]
** @fnote None
**
** @nam3rule New Constructor
**
** @argrule New dba [EnsPDatabaseadaptor] Ensembl Database Adaptor
**
** @valrule * [EnsPQcsequenceadaptor] Ensembl QC Sequence Adaptor
**
** @fcategory new
******************************************************************************/




/* @func ensQcsequenceadaptorNew **********************************************
**
** Default constructor for an Ensembl QC Sequence Adaptor.
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
** @see ensRegistryGetQcsequenceadaptor
**
** @param [r] dba [EnsPDatabaseadaptor] Ensembl Database Adaptor
**
** @return [EnsPQcsequenceadaptor] Ensembl QC Sequence Adaptor or NULL
** @@
******************************************************************************/

EnsPQcsequenceadaptor ensQcsequenceadaptorNew(
    EnsPDatabaseadaptor dba)
{
    EnsPQcsequenceadaptor qcsa = NULL;

    if(!dba)
        return NULL;

    AJNEW0(qcsa);

    qcsa->Adaptor = ensBaseadaptorNew(
        dba,
        qcsequenceadaptorTables,
        qcsequenceadaptorColumns,
        qcsequenceadaptorLeftJoin,
        qcsequenceadaptorDefaultCondition,
        qcsequenceadaptorFinalCondition,
        qcsequenceadaptorFetchAllBySQL);

    return qcsa;
}




/* @section destructors *******************************************************
**
** Destruction destroys all internal data structures and frees the
** memory allocated for the Ensembl QC Sequence Adaptor.
**
** @fdata [EnsPQcsequenceadaptor]
** @fnote None
**
** @nam3rule Del Destroy (free) a QC Sequence Adaptor object
**
** @argrule * Pqcsa [EnsPQcsequenceadaptor*] QC Sequence Adaptor object address
**
** @valrule * [void]
**
** @fcategory delete
******************************************************************************/




/* @func ensQcsequenceadaptorDel **********************************************
**
** Default destructor for an Ensembl QC Sequence Adaptor.
**
** Ensembl Object Adaptors are singleton objects that are registered in the
** Ensembl Registry and weakly referenced by Ensembl Objects that have been
** instantiated by it. Therefore, Ensembl Object Adaptors should never be
** destroyed directly. Upon exit, the Ensembl Registry will call this function
** if required.
**
** @param [d] Pqcsa [EnsPQcsequenceadaptor*] Ensembl QC Sequence Adaptor
**                                           address
**
** @return [void]
** @@
******************************************************************************/

void ensQcsequenceadaptorDel(EnsPQcsequenceadaptor *Pqcsa)
{
    EnsPQcsequenceadaptor pthis = NULL;

    if(!Pqcsa)
        return;

    if(!*Pqcsa)
        return;

    if(ajDebugTest("ensQcsequenceadaptorDel"))
        ajDebug("ensQcsequenceadaptorDel\n"
                "  *Pqcsa %p\n",
                *Pqcsa);

    pthis = *Pqcsa;

    ensBaseadaptorDel(&pthis->Adaptor);

    AJFREE(pthis);

    *Pqcsa = NULL;

    return;
}




/* @func ensQcsequenceadaptorFetchByIdentifier ********************************
**
** Fetch an Ensembl QC Sequence via its SQL database-internal identifier.
** The caller is responsible for deleting the Ensembl QC Sequence.
**
** @param [u] qcsa [EnsPQcsequenceadaptor] Ensembl QC Sequence Adaptor
** @param [r] identifier [ajuint] SQL database-internal QC Sequence identifier
** @param [wP] Pqcs [EnsPQcsequence*] Ensembl QC Sequence address
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
** @@
******************************************************************************/

AjBool ensQcsequenceadaptorFetchByIdentifier(EnsPQcsequenceadaptor qcsa,
                                             ajuint identifier,
                                             EnsPQcsequence *Pqcs)
{
    if(!qcsa)
        return ajFalse;

    if(!identifier)
        return ajFalse;

    if(!Pqcs)
        return ajFalse;

    *Pqcs = (EnsPQcsequence)
        ensBaseadaptorFetchByIdentifier(qcsa->Adaptor, identifier);

    return ajTrue;
}




/* @func ensQcsequenceadaptorFetchByAccession *********************************
**
** Fetch an Ensembl QC Sequence by its accession number.
** The caller is responsible for deleting the Ensembl QC Sequence.
**
** @param [u] qcsa [EnsPQcsequenceadaptor] Ensembl QC Sequence Adaptor
** @param [r] qcdbid [ajuint] Ensembl QC Database identifier
** @param [r] accession [const AjPStr] Ensembl QC Sequence accession number
** @param [wP] Pqcs [EnsPQcsequence*] Ensembl QC Sequence address
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
** @@
******************************************************************************/

AjBool ensQcsequenceadaptorFetchByAccession(EnsPQcsequenceadaptor qcsa,
                                            ajuint qcdbid,
                                            const AjPStr accession,
                                            EnsPQcsequence *Pqcs)
{
    char *txtaccession = NULL;

    AjPList qcss = NULL;

    AjPStr constraint = NULL;

    EnsPQcsequence qcs = NULL;

    if(!qcsa)
        return ajFalse;

    if(!accession)
        return ajFalse;

    if(!Pqcs)
        return ajFalse;

    ensBaseadaptorEscapeC(qcsa->Adaptor, &txtaccession, accession);

    constraint = ajFmtStr("sequence.sequence_db_id = %u "
                          "AND "
                          "sequence.accession = '%s'",
                          qcdbid,
                          txtaccession);

    ajCharDel(&txtaccession);

    qcss = ajListNew();

    ensBaseadaptorGenericFetch(qcsa->Adaptor,
                               constraint,
                               (EnsPAssemblymapper) NULL,
                               (EnsPSlice) NULL,
                               qcss);

    if(ajListGetLength(qcss) > 1)
        ajWarn("ensQcsequenceadaptorFetchByAccession got more than "
               "one Ensembl QC Sequence for accession '%S' "
               "and QC Database identifier %u.\n",
               accession, qcdbid);

    ajListPop(qcss, (void **) Pqcs);

    /*
    ** TODO: To implement?
    ** qcsequenceadaptorCacheInsert(qcsa, Pqcs);
    */

    while(ajListPop(qcss, (void **) &qcs))
    {
        /*
        ** TODO: To implement?
        ** qcsequenceadaptorCacheInsert(qcsa, &qcs);
        */

        ensQcsequenceDel(&qcs);
    }

    ajListFree(&qcss);

    ajStrDel(&constraint);

    return ajTrue;
}




/* @func ensQcsequenceadaptorFetchByAccessionVersion **************************
**
** Fetch an Ensembl QC Sequence by its accession number and sequence version.
** The caller is responsible for deleting the Ensembl QC Sequence.
**
** @param [u] qcsa [EnsPQcsequenceadaptor] Ensembl QC Sequence Adaptor
** @param [r] qcdbid [ajuint] Ensembl QC Database identifier
** @param [r] accession [const AjPStr] Ensembl QC Sequence accession number
** @param [r] version [ajuint] Ensembl QC Sequence version
** @param [wP] Pqcs [EnsPQcsequence*] Ensembl QC Sequence address
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
** @@
******************************************************************************/

AjBool ensQcsequenceadaptorFetchByAccessionVersion(EnsPQcsequenceadaptor qcsa,
                                                   ajuint qcdbid,
                                                   const AjPStr accession,
                                                   ajuint version,
                                                   EnsPQcsequence *Pqcs)
{
    char *txtaccession = NULL;

    AjPList qcss = NULL;

    AjPStr constraint = NULL;

    EnsPQcsequence qcs = NULL;

    if(!qcsa)
        return ajFalse;

    if(!accession)
        return ajFalse;

    if(!Pqcs)
        return ajFalse;

    ensBaseadaptorEscapeC(qcsa->Adaptor, &txtaccession, accession);

    constraint = ajFmtStr("sequence.sequence_db_id = %u "
                          "AND "
                          "sequence.accession = '%s'",
                          qcdbid,
                          txtaccession);

    ajCharDel(&txtaccession);

    if(version)
        ajFmtPrintAppS(&constraint, " AND sequence.version = %u", version);

    qcss = ajListNew();

    ensBaseadaptorGenericFetch(qcsa->Adaptor,
                               constraint,
                               (EnsPAssemblymapper) NULL,
                               (EnsPSlice) NULL,
                               qcss);

    if(ajListGetLength(qcss) > 1)
        ajWarn("ensQcsequenceadaptorFetchByAccession got more than "
               "one Ensembl QC Sequence for accession '%S' "
               "and QC Database identifier %u.\n",
               accession, qcdbid);

    ajListPop(qcss, (void **) Pqcs);

    /*
    ** TODO: To implement?
    ** qcsequenceadaptorCacheInsert(qcsa, Pqcs);
    */

    while(ajListPop(qcss, (void **) &qcs))
    {

        /*
        ** TODO: To implement?
        ** qcsequenceadaptorCacheInsert(qcsa, &qcs);
        */

        ensQcsequenceDel(&qcs);
    }

    ajListFree(&qcss);

    ajStrDel(&constraint);

    return ajTrue;
}




/* @func ensQcsequenceadaptorFetchByName **************************************
**
** Fetch an Ensembl QC Sequence by its name.
** The caller is responsible for deleting the Ensembl QC Sequence.
**
** @param [u] qcsa [EnsPQcsequenceadaptor] Ensembl QC Sequence Adaptor
** @param [r] qcdbid [ajuint] Ensembl QC Database identifier
** @param [r] name [const AjPStr] Ensembl QC Sequence name
** @param [wP] Pqcs [EnsPQcsequence*] Ensembl QC Sequence address
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
** @@
******************************************************************************/

AjBool ensQcsequenceadaptorFetchByName(EnsPQcsequenceadaptor qcsa,
                                       ajuint qcdbid,
                                       const AjPStr name,
                                       EnsPQcsequence *Pqcs)
{
    char *txtname = NULL;

    AjPList qcss = NULL;

    AjPStr constraint = NULL;

    EnsPQcsequence qcs = NULL;

    if(!qcsa)
        return ajFalse;

    if(!name)
        return ajFalse;

    if(!Pqcs)
        return ajFalse;

    ensBaseadaptorEscapeC(qcsa->Adaptor, &txtname, name);

    constraint = ajFmtStr("sequence.sequence_db_id = %u "
                          "AND "
                          "sequence.name = '%s'",
                          qcdbid,
                          txtname);

    ajCharDel(&txtname);

    qcss = ajListNew();

    ensBaseadaptorGenericFetch(qcsa->Adaptor,
                               constraint,
                               (EnsPAssemblymapper) NULL,
                               (EnsPSlice) NULL,
                               qcss);

    if(ajListGetLength(qcss) > 1)
        ajWarn("ensQcsequenceadaptorFetchByName got more than "
               "one Ensembl QC Sequence for (UNIQUE) name '%S' "
               "and QC Database identifier %u.\n",
               name, qcdbid);

    ajListPop(qcss, (void **) Pqcs);

    /*
    ** TODO: To implement?
    ** qcsequenceadaptorCacheInsert(qcsa, Pqcs);
    */

    while(ajListPop(qcss, (void **) &qcs))
    {
        /*
        ** TODO: To implement?
        ** qcsequenceadaptorCacheInsert(qcsa, &qcs);
        */

        ensQcsequenceDel(&qcs);
    }

    ajListFree(&qcss);

    ajStrDel(&constraint);

    return ajTrue;
}




/* @func ensQcsequenceadaptorFetchAllByQcdatabase *****************************
**
** Fetch all Ensembl QC Sequences via an Ensembl QC Database.
** The caller is responsible for deleting the Ensembl QC Sequences
** before deleting the AJAX List.
**
** @param [u] qcsa [EnsPQcsequenceadaptor] Ensembl QC Sequence Adaptor
** @param [r] qcdb [const EnsPQcdatabase] Ensembl QC Database
** @param [u] qcss [AjPList] AJAX List of Ensembl QC Sequences
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
** @@
******************************************************************************/

AjBool ensQcsequenceadaptorFetchAllByQcdatabase(EnsPQcsequenceadaptor qcsa,
                                                const EnsPQcdatabase qcdb,
                                                AjPList qcss)
{
    AjPStr constraint = NULL;

    if(!qcsa)
        return ajFalse;

    if(!qcdb)
        return ajFalse;

    if(!qcss)
        return ajFalse;

    constraint = ajFmtStr("sequence.sequence_db_id = %u",
                          ensQcdatabaseGetIdentifier(qcdb));

    ensBaseadaptorGenericFetch(qcsa->Adaptor,
                               constraint,
                               (EnsPAssemblymapper) NULL,
                               (EnsPSlice) NULL,
                               qcss);

    ajStrDel(&constraint);

    return ajTrue;
}




/* @func ensQcsequenceadaptorStore ********************************************
**
** Store an Ensembl QC Sequence.
**
** @param [u] qcsa [EnsPQcsequenceadaptor] Ensembl QC Sequence Adaptor
** @param [u] qcs [EnsPQcsequence] Ensembl QC Seqeunce
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
** @@
******************************************************************************/

AjBool ensQcsequenceadaptorStore(EnsPQcsequenceadaptor qcsa,
                                 EnsPQcsequence qcs)
{
    char *txtname        = NULL;
    char *txtaccession   = NULL;
    char *txtdescription = NULL;
    char *txttype        = NULL;

    AjBool value = AJFALSE;

    AjPSqlstatement sqls = NULL;

    AjPStr statement = NULL;

    EnsPDatabaseadaptor dba = NULL;

    if(!qcsa)
        return ajFalse;

    if(!qcs)
        return ajFalse;

    if(ensQcsequenceGetAdaptor(qcs) &&
       ensQcsequenceGetIdentifier(qcs))
        return ajFalse;

    dba = ensBaseadaptorGetDatabaseadaptor(qcsa->Adaptor);

    ensDatabaseadaptorEscapeC(dba, &txtname, qcs->Name);
    ensDatabaseadaptorEscapeC(dba, &txtaccession, qcs->Accession);
    ensDatabaseadaptorEscapeC(dba, &txtdescription, qcs->Description);
    ensDatabaseadaptorEscapeC(dba, &txttype, qcs->Type);

    statement = ajFmtStr("INSERT IGNORE INTO "
                         "sequence "
                         "SET "
                         "sequence.sequence_db_id = %u, "
                         "sequence.name = '%s', "
                         "sequence.accession = '%s', "
                         "sequence.version = %u, "
                         "sequence.description = '%s' "
                         "sequence.molecule_type = '%s', "
                         "sequence.length = %u, "
                         "sequence.cds_start = %u, "
                         "sequence.cds_end = %u, "
                         "sequence.cds_strand = %d, "
                         "sequence.poly_a = %u",
                         ensQcsequenceGetQcdatabaseIdentifier(qcs),
                         txtname,
                         txtaccession,
                         qcs->Version,
                         txtdescription,
                         txttype,
                         qcs->Length,
                         qcs->CDSStart,
                         qcs->CDSEnd,
                         qcs->CDSStrand,
                         qcs->PolyA);

    ajCharDel(&txtname);
    ajCharDel(&txtaccession);
    ajCharDel(&txtdescription);
    ajCharDel(&txttype);

    sqls = ensDatabaseadaptorSqlstatementNew(dba, statement);

    if(ajSqlstatementGetAffectedrows(sqls))
    {
        ensQcsequenceSetIdentifier(qcs, ajSqlstatementGetIdentifier(sqls));

        ensQcsequenceSetAdaptor(qcs, qcsa);

        value = ajTrue;
    }

    ensDatabaseadaptorSqlstatementDel(dba, &sqls);

    ajStrDel(&statement);

    return value;
}




/* @func ensQcsequenceadaptorUpdate *******************************************
**
** Update an Ensembl QC Sequence.
**
** @param [u] qcsa [EnsPQcsequenceadaptor] Ensembl QC Sequence Adaptor
** @param [r] qcs [const EnsPQcsequence] Ensembl QC Sequence
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
** @@
******************************************************************************/

AjBool ensQcsequenceadaptorUpdate(EnsPQcsequenceadaptor qcsa,
                                  const EnsPQcsequence qcs)
{
    char *txtname        = NULL;
    char *txtaccession   = NULL;
    char *txtdescription = NULL;
    char *txttype        = NULL;

    AjBool value = AJFALSE;

    AjPSqlstatement sqls = NULL;

    AjPStr statement = NULL;

    EnsPDatabaseadaptor dba = NULL;

    if(!qcsa)
        return ajFalse;

    if(!qcs)
        return ajFalse;

    if(!ensQcsequenceGetIdentifier(qcs))
        return ajFalse;

    dba = ensBaseadaptorGetDatabaseadaptor(qcsa->Adaptor);

    ensDatabaseadaptorEscapeC(dba, &txtname, qcs->Name);
    ensDatabaseadaptorEscapeC(dba, &txtaccession, qcs->Accession);
    ensDatabaseadaptorEscapeC(dba, &txtdescription, qcs->Description);
    ensDatabaseadaptorEscapeC(dba, &txttype, qcs->Type);

    statement = ajFmtStr("UPDATE IGNORE "
                         "sequence "
                         "SET "
                         "sequence.sequence_db_id = %u, "
                         "sequence.name = '%s', "
                         "sequence.accession = '%s', "
                         "sequence.version = %u, "
                         "sequence.description = '%s' "
                         "sequence.molecule_type = '%s', "
                         "sequence.length = %u, "
                         "sequence.cds_start = %u, "
                         "sequence.cds_end = %u, "
                         "sequence.cds_strand = %d, "
                         "sequence.poly_a = %u "
                         "WHERE "
                         "sequence.sequence_id = %u",
                         ensQcsequenceGetQcdatabaseIdentifier(qcs),
                         txtname,
                         txtaccession,
                         qcs->Version,
                         txtdescription,
                         txttype,
                         qcs->Length,
                         qcs->CDSStart,
                         qcs->CDSEnd,
                         qcs->CDSStrand,
                         qcs->PolyA,
                         qcs->Identifier);

    ajCharDel(&txtname);
    ajCharDel(&txtaccession);
    ajCharDel(&txtdescription);
    ajCharDel(&txttype);

    sqls = ensDatabaseadaptorSqlstatementNew(dba, statement);

    if(ajSqlstatementGetAffectedrows(sqls))
        value = ajTrue;

    ensDatabaseadaptorSqlstatementDel(dba, &sqls);

    ajStrDel(&statement);

    return value;
}




/* @func ensQcsequenceadaptorDelete *******************************************
**
** Delete an Ensembl QC Sequence.
**
** @param [u] qcsa [EnsPQcsequenceadaptor] Ensembl QC Sequence Adaptor
** @param [u] qcs [EnsPQcsequence] Ensembl QC Sequence
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
** @@
******************************************************************************/

AjBool ensQcsequenceadaptorDelete(EnsPQcsequenceadaptor qcsa,
                                  EnsPQcsequence qcs)
{
    AjBool value = AJFALSE;

    AjPSqlstatement sqls = NULL;

    AjPStr statement = NULL;

    EnsPDatabaseadaptor dba = NULL;

    if(!qcsa)
        return ajFalse;

    if(!qcs)
        return ajFalse;

    if(!ensQcsequenceGetIdentifier(qcs))
        return ajFalse;

    dba = ensBaseadaptorGetDatabaseadaptor(qcsa->Adaptor);

    statement = ajFmtStr("DELETE FROM "
                         "sequence "
                         "WHERE "
                         "sequence.sequence_id = %u",
                         qcs->Identifier);

    sqls = ensDatabaseadaptorSqlstatementNew(dba, statement);

    if(ajSqlstatementGetAffectedrows(sqls))
    {
        qcs->Adaptor    = (EnsPQcsequenceadaptor) NULL;
        qcs->Identifier = 0;

        value = ajTrue;
    }

    ensDatabaseadaptorSqlstatementDel(dba, &sqls);

    ajStrDel(&statement);

    return value;
}
