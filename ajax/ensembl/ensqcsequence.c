/* @source Ensembl Quality Check Sequence functions
**
** @author Copyright (C) 1999 Ensembl Developers
** @author Copyright (C) 2006 Michael K. Schuster
** @modified 2009 by Alan Bleasby for incorporation into EMBOSS core
** @modified $Date: 2011/05/25 19:55:04 $ by $Author: mks $
** @version $Revision: 1.23 $
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

/* @conststatic qcsequenceadaptorTables ***************************************
**
** Array of Ensembl Quality Check Sequence Adaptor SQL table names
**
******************************************************************************/

static const char* qcsequenceadaptorTables[] =
{
    "sequence",
    (const char*) NULL
};




/* @conststatic qcsequenceadaptorColumns **************************************
**
** Array of Ensembl Quality Check Sequence Adaptor SQL column names
**
******************************************************************************/

static const char* qcsequenceadaptorColumns[] =
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
    (const char*) NULL
};




/* ==================================================================== */
/* ======================== private variables ========================= */
/* ==================================================================== */




/* ==================================================================== */
/* ======================== private functions ========================= */
/* ==================================================================== */

static AjBool qcsequenceadaptorFetchAllbyStatement(
    EnsPDatabaseadaptor dba,
    const AjPStr statement,
    EnsPAssemblymapper am,
    EnsPSlice slice,
    AjPList qcss);




/* ==================================================================== */
/* ===================== All functions by section ===================== */
/* ==================================================================== */




/* @filesection ensqcsequence *************************************************
**
** @nam1rule ens Function belongs to the Ensembl library
**
******************************************************************************/




/* @datasection [EnsPQcsequence] Ensembl Quality Check Sequence ***************
**
** @nam2rule Qcsequence Functions for manipulating
** Ensembl Quality Check Sequence objects
**
** @cc Bio::EnsEMBL::QC::Sequence
** @cc CVS Revision:
** @cc CVS Tag:
**
******************************************************************************/




/* @section constructors ******************************************************
**
** All constructors return a new Ensembl Quality Check Sequence by pointer.
** It is the responsibility of the user to first destroy any previous
** Quality Check Sequence. The target pointer does not need to be initialised
** to NULL, but it is good programming practice to do so anyway.
**
** @fdata [EnsPQcsequence]
**
** @nam3rule New Constructor
** @nam4rule Cpy Constructor with existing object
** @nam4rule Ini Constructor with initial values
** @nam4rule Ref Constructor by incrementing the reference counter
**
** @argrule Cpy qcs [const EnsPQcsequence] Ensembl Quality Check Sequence
** @argrule Ini qcsa [EnsPQcsequenceadaptor]
** Ensembl Quality Check Sequence Adaptor
** @argrule Ini identifier [ajuint] SQL database-internal identifier
** @argrule Ini qcdb [EnsPQcdatabase] Ensembl Quality Check Database
** @argrule Ini name [AjPStr] Name
** @argrule Ini accession [AjPStr] Accession
** @argrule Ini version [ajuint] Version
** @argrule Ini type [AjPStr] Type
** @argrule Ini length [ajuint] Length
** @argrule Ini cdsstart [ajuint] CDS start
** @argrule Ini cdsend [ajuint] CDS end
** @argrule Ini cdsstrand [ajint] CDS strand
** @argrule Ini polya [ajuint] PolyA+ tail length
** @argrule Ini description [AjPStr] Description
** @argrule Ref qcs [EnsPQcsequence] Ensembl Quality Check Sequence
**
** @valrule * [EnsPQcsequence] Ensembl Quality Check Sequence
**
** @fcategory new
******************************************************************************/




/* @func ensQcsequenceNewCpy **************************************************
**
** Object-based constructor function, which returns an independent object.
**
** @param [r] qcs [const EnsPQcsequence] Ensembl Quality Check Sequence
**
** @return [EnsPQcsequence] Ensembl Quality Check Sequence or NULL
** @@
******************************************************************************/

EnsPQcsequence ensQcsequenceNewCpy(const EnsPQcsequence qcs)
{
    EnsPQcsequence pthis = NULL;

    if(!qcs)
        return NULL;

    AJNEW0(pthis);

    pthis->Use = 1;

    pthis->Identifier = qcs->Identifier;

    pthis->Adaptor = qcs->Adaptor;

    pthis->Qcdatabase = ensQcdatabaseNewRef(qcs->Qcdatabase);

    if(qcs->Name)
        pthis->Name = ajStrNewRef(qcs->Name);

    if(qcs->Accession)
        pthis->Accession = ajStrNewRef(qcs->Accession);

    pthis->Version = qcs->Version;

    if(qcs->Type)
        pthis->Type = ajStrNewRef(qcs->Type);

    pthis->Length    = qcs->Length;
    pthis->CdsStart  = qcs->CdsStart;
    pthis->CdsEnd    = qcs->CdsEnd;
    pthis->CdsStrand = qcs->CdsStrand;
    pthis->Polya     = qcs->Polya;

    if(qcs->Description)
        pthis->Description = ajStrNewRef(qcs->Description);

    return pthis;
}




/* @func ensQcsequenceNewIni **************************************************
**
** Constructor for an Ensembl Quality Check Sequence with initial values.
**
** @cc Bio::EnsEMBL::Storable::new
** @param [u] qcsa [EnsPQcsequenceadaptor]
** Ensembl Quality Check Sequence Adaptor
** @param [r] identifier [ajuint] SQL database-internal identifier
** @cc Bio::EnsEMBL::QC::Sequence::new
** @param [u] qcdb [EnsPQcdatabase] Ensembl Quality Check Database
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
** @return [EnsPQcsequence] Ensembl Quality Check Sequence or NULL
** @@
******************************************************************************/

EnsPQcsequence ensQcsequenceNewIni(EnsPQcsequenceadaptor qcsa,
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

    qcs->Length    = length;
    qcs->CdsStart  = cdsstart;
    qcs->CdsEnd    = cdsend;
    qcs->CdsStrand = cdsstrand;
    qcs->Polya     = polya;

    if(description)
        qcs->Description = ajStrNewRef(description);

    return qcs;
}




/* @func ensQcsequenceNewRef **************************************************
**
** Ensembl Object referencing function, which returns a pointer to the
** Ensembl Object passed in and increases its reference count.
**
** @param [u] qcs [EnsPQcsequence] Ensembl Quality Check Sequence
**
** @return [EnsPQcsequence] Ensembl Quality Check Sequence or NULL
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
** memory allocated for an Ensembl Quality Check Sequence object.
**
** @fdata [EnsPQcsequence]
**
** @nam3rule Del Destroy (free) an Ensembl Quality Check Sequence object
**
** @argrule * Pqcs [EnsPQcsequence*] Ensembl Quality Check Sequence
**                                   object address
**
** @valrule * [void]
**
** @fcategory delete
******************************************************************************/




/* @func ensQcsequenceDel *****************************************************
**
** Default destructor for an Ensembl Quality Check Sequence.
**
** @param [d] Pqcs [EnsPQcsequence*] Ensembl Quality Check Sequence
**                                   object address
**
** @return [void]
** @@
******************************************************************************/

void ensQcsequenceDel(EnsPQcsequence* Pqcs)
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
** Functions for returning elements of an
** Ensembl Quality Check Sequence object.
**
** @fdata [EnsPQcsequence]
**
** @nam3rule Get Return Ensembl Quality Check Sequence attribute(s)
** @nam4rule Accession Return the accession number
** @nam4rule Adaptor Return the Ensembl Quality Check Sequence Adaptor
** @nam4rule Cds Return coding sequence members
** @nam5rule End Return the coding sequence end
** @nam5rule Start Return the coding sequence start
** @nam5rule Strand Return the coding sequence strand
** @nam4rule Description Return the description
** @nam4rule Identifier Return the SQL database-internal identifier
** @nam4rule Length Return the length
** @nam4rule Name Return the name
** @nam4rule Polya Return the PolyA+ tail length
** @nam4rule Qcdatabase Return the Ensembl Quality Check Database
** @nam4rule Type Retrun the type
** @nam4rule Version Return the version
**
** @argrule * qcs [const EnsPQcsequence] Ensembl Quality Check Sequence
**
** @valrule Accession [AjPStr] Accession number or NULL
** @valrule Adaptor [EnsPQcsequenceadaptor]
** Ensembl Quality Check Sequence Adaptor or NULL
** @valrule CdsEnd [ajuint] Coding sequence end or 0
** @valrule CdsStart [ajuint] Coding sequence start or 0
** @valrule CdsStrand [ajint] Coding sequence strand or 0
** @valrule Description [AjPStr] Description or NULL
** @valrule Identifier [ajuint] SQL database-internal identifier or 0
** @valrule Length [ajuint] Length or 0
** @valrule Name [AjPStr] Name or NULL
** @valrule Polya [ajuint] Polya tail length or 0
** @valrule Qcdatabase [EnsPQcdatabase] Ensembl Quality Check Database or NULL
** @valrule Type [AjPStr] Type or NULL
** @valrule Version [ajuint] Version or 0
**
** @fcategory use
******************************************************************************/




/* @func ensQcsequenceGetAccession ********************************************
**
** Get the accession number element of an Ensembl Quality Check Sequence.
**
** @param [r] qcs [const EnsPQcsequence] Ensembl Quality Check Sequence
**
** @return [AjPStr] Accession number or NULL
** @@
******************************************************************************/

AjPStr ensQcsequenceGetAccession(const EnsPQcsequence qcs)
{
    if(!qcs)
        return NULL;

    return qcs->Accession;
}




/* @func ensQcsequenceGetAdaptor **********************************************
**
** Get the Ensembl Quality Check Sequence Adaptor element of an
** Ensembl Quality Check Sequence.
**
** @param [r] qcs [const EnsPQcsequence] Ensembl Quality Check Sequence
**
** @return [EnsPQcsequenceadaptor] Ensembl Quality Check Sequence Adaptor
** or NULL
** @@
******************************************************************************/

EnsPQcsequenceadaptor ensQcsequenceGetAdaptor(const EnsPQcsequence qcs)
{
    if(!qcs)
        return NULL;

    return qcs->Adaptor;
}




/* @func ensQcsequenceGetCdsEnd ***********************************************
**
** Get the CDS end element of an Ensembl Quality Check Sequence.
**
** @param [r] qcs [const EnsPQcsequence] Ensembl Quality Check Sequence
**
** @return [ajuint] CDS start or 0
** @@
******************************************************************************/

ajuint ensQcsequenceGetCdsEnd(const EnsPQcsequence qcs)
{
    if(!qcs)
        return 0;

    return qcs->CdsEnd;
}




/* @func ensQcsequenceGetCdsStart *********************************************
**
** Get the CDS start element of an Ensembl Quality Check Sequence.
**
** @param [r] qcs [const EnsPQcsequence] Ensembl Quality Check Sequence
**
** @return [ajuint] CDS start or 0
** @@
******************************************************************************/

ajuint ensQcsequenceGetCdsStart(const EnsPQcsequence qcs)
{
    if(!qcs)
        return 0;

    return qcs->CdsStart;
}




/* @func ensQcsequenceGetCdsStrand ********************************************
**
** Get the CDS strand element of an Ensembl Quality Check Sequence.
**
** @param [r] qcs [const EnsPQcsequence] Ensembl Quality Check Sequence
**
** @return [ajint] CDS strand or 0
** @@
******************************************************************************/

ajint ensQcsequenceGetCdsStrand(const EnsPQcsequence qcs)
{
    if(!qcs)
        return 0;

    return qcs->CdsStrand;
}




/* @func ensQcsequenceGetDescription ******************************************
**
** Get the description element of an Ensembl Quality Check Sequence.
**
** @param [r] qcs [const EnsPQcsequence] Ensembl Quality Check Sequence
**
** @return [AjPStr] Description or NULL
** @@
******************************************************************************/

AjPStr ensQcsequenceGetDescription(const EnsPQcsequence qcs)
{
    if(!qcs)
        return NULL;

    return qcs->Description;
}




/* @func ensQcsequenceGetIdentifier *******************************************
**
** Get the SQL database-internal identifier element of an
** Ensembl Quality Check Sequence.
**
** @param [r] qcs [const EnsPQcsequence] Ensembl Quality Check Sequence
**
** @return [ajuint] SQL database-internal identifier or 0
** @@
******************************************************************************/

ajuint ensQcsequenceGetIdentifier(const EnsPQcsequence qcs)
{
    if(!qcs)
        return 0;

    return qcs->Identifier;
}




/* @func ensQcsequenceGetLength ***********************************************
**
** Get the length element of an Ensembl Quality Check Sequence.
**
** @param [r] qcs [const EnsPQcsequence] Ensembl Quality Check Sequence
**
** @return [ajuint] Length or 0
** @@
******************************************************************************/

ajuint ensQcsequenceGetLength(const EnsPQcsequence qcs)
{
    if(!qcs)
        return 0;

    return qcs->Length;
}




/* @func ensQcsequenceGetName *************************************************
**
** Get the name element of an Ensembl Quality Check Sequence.
**
** @param [r] qcs [const EnsPQcsequence] Ensembl Quality Check Sequence
**
** @return [AjPStr] Name or NULL
** @@
******************************************************************************/

AjPStr ensQcsequenceGetName(const EnsPQcsequence qcs)
{
    if(!qcs)
        return NULL;

    return qcs->Name;
}




/* @func ensQcsequenceGetPolya ************************************************
**
** Get the PolyA+ tail length element of an Ensembl Quality Check Sequence.
**
** @param [r] qcs [const EnsPQcsequence] Ensembl Quality Check Sequence
**
** @return [ajuint] PolyA+ tail length or 0
** @@
******************************************************************************/

ajuint ensQcsequenceGetPolya(const EnsPQcsequence qcs)
{
    if(!qcs)
        return 0;

    return qcs->Polya;
}




/* @func ensQcsequenceGetQcdatabase *******************************************
**
** Get the Ensembl Quality Check Database element of an
** Ensembl Quality Check Sequence.
**
** @param [r] qcs [const EnsPQcsequence] Ensembl Quality Check Sequence
**
** @return [EnsPQcdatabase] Ensembl Quality Check Database or NULL
** @@
******************************************************************************/

EnsPQcdatabase ensQcsequenceGetQcdatabase(const EnsPQcsequence qcs)
{
    if(!qcs)
        return NULL;

    return qcs->Qcdatabase;
}




/* @func ensQcsequenceGetType *************************************************
**
** Get the type element of an Ensembl Quality Check Sequence.
**
** @param [r] qcs [const EnsPQcsequence] Ensembl Quality Check Sequence
**
** @return [AjPStr] Type or NULL
** @@
******************************************************************************/

AjPStr ensQcsequenceGetType(const EnsPQcsequence qcs)
{
    if(!qcs)
        return NULL;

    return qcs->Type;
}




/* @func ensQcsequenceGetVersion **********************************************
**
** Get the version element of an Ensembl Quality Check Sequence.
**
** @param [r] qcs [const EnsPQcsequence] Ensembl Quality Check Sequence
**
** @return [ajuint] Version or 0
** @@
******************************************************************************/

ajuint ensQcsequenceGetVersion(const EnsPQcsequence qcs)
{
    if(!qcs)
        return 0;

    return qcs->Version;
}




/* @section modifiers *********************************************************
**
** Functions for assigning elements of an
** Ensembl Quality Check Sequence object.
**
** @fdata [EnsPQcsequence]
**
** @nam3rule Set Set one element of an Ensembl Quality Check Sequence
** @nam4rule Accession Set the accession number
** @nam4rule Adaptor Set the Ensembl Quality Check Sequence Adaptor
** @nam4rule Cds Set coding sequence members
** @nam5rule End Set the CDS end
** @nam5rule Start Set the CDS start
** @nam5rule Strand Set the CDS strand
** @nam4rule Description Set the description
** @nam4rule Identifier Set the SQL database-internal identifier
** @nam4rule Length Set the length
** @nam4rule Name Set the name
** @nam4rule Polya Set the Polya+ tail length
** @nam4rule Qcdatabase Set the Ensembl Quality Check Database
** @nam4rule Type Set the type
** @nam4rule Version Set the version
**
** @argrule * qcs [EnsPQcsequence] Ensembl Quality Check Sequence
** @argrule Accession accession [AjPStr] Accession number
** @argrule Adaptor qcsa [EnsPQcsequenceadaptor]
** Ensembl Quality Check Sequence Adaptor
** @argrule CdsEnd cdsend [ajuint] CDS end
** @argrule CdsStart cdsstart [ajuint] CDS start
** @argrule CdsStrand cdsstrand [ajint] CDS strand
** @argrule Description description [AjPStr] Description
** @argrule Identifier identifier [ajuint] SQL database-internal identifier
** @argrule Length length [ajuint] Length
** @argrule Name name [AjPStr] Name
** @argrule Polya polya [ajuint] PolyA+ tail length
** @argrule Qcdatabase qcdb [EnsPQcdatabase] Ensembl Quality Check Database
** @argrule Type type [AjPStr] Type
** @argrule Version version [ajuint] Version
**
** @valrule * [AjBool] ajTrue upon success, ajFalse otherwise
**
** @fcategory modify
******************************************************************************/




/* @func ensQcsequenceSetAccession ********************************************
**
** Set the accession number element of an Ensembl Quality Check Sequence.
**
** @param [u] qcs [EnsPQcsequence] Ensembl Quality Check Sequence
** @param [uN] accession [AjPStr] Accession number
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




/* @func ensQcsequenceSetAdaptor **********************************************
**
** Set the Ensembl Quality Check Sequence Adaptor element of an
** Ensembl Quality Check Sequence.
**
** @param [u] qcs [EnsPQcsequence] Ensembl Quality Check Sequence
** @param [uN] qcsa [EnsPQcsequenceadaptor]
** Ensembl Quality Check Sequence Adaptor
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




/* @func ensQcsequenceSetCdsEnd ***********************************************
**
** Set the CDS end element of an Ensembl Quality Check Sequence.
**
** @param [u] qcs [EnsPQcsequence] Ensembl Quality Check Sequence
** @param [r] cdsend [ajuint] CDS end
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
** @@
******************************************************************************/

AjBool ensQcsequenceSetCdsEnd(EnsPQcsequence qcs, ajuint cdsend)
{
    if(!qcs)
        return ajFalse;

    qcs->CdsEnd = cdsend;

    return ajTrue;
}




/* @func ensQcsequenceSetCdsStart *********************************************
**
** Set the CDS start element of an Ensembl Quality Check Sequence.
**
** @param [u] qcs [EnsPQcsequence] Ensembl Quality Check Sequence
** @param [r] cdsstart [ajuint] CDS start
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
** @@
******************************************************************************/

AjBool ensQcsequenceSetCdsStart(EnsPQcsequence qcs, ajuint cdsstart)
{
    if(!qcs)
        return ajFalse;

    qcs->CdsStart = cdsstart;

    return ajTrue;
}




/* @func ensQcsequenceSetCdsStrand ********************************************
**
** Set the CDS strand element of an Ensembl Quality Check Sequence.
**
** @param [u] qcs [EnsPQcsequence] Ensembl Quality Check Sequence
** @param [r] cdsstrand [ajint] CDS strand
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
** @@
******************************************************************************/

AjBool ensQcsequenceSetCdsStrand(EnsPQcsequence qcs, ajint cdsstrand)
{
    if(!qcs)
        return ajFalse;

    qcs->CdsStrand = cdsstrand;

    return ajTrue;
}




/* @func ensQcsequenceSetDescription ******************************************
**
** Set the description element of an Ensembl Quality Check Sequence.
**
** @param [u] qcs [EnsPQcsequence] Ensembl Quality Check Sequence
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




/* @func ensQcsequenceSetIdentifier *******************************************
**
** Set the SQL database-internal identifier element of an
** Ensembl Quality Check Sequence.
**
** @param [u] qcs [EnsPQcsequence] Ensembl Quality Check Sequence
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




/* @func ensQcsequenceSetLength ***********************************************
**
** Set the length element of an Ensembl Quality Check Sequence.
**
** @param [u] qcs [EnsPQcsequence] Ensembl Quality Check Sequence
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




/* @func ensQcsequenceSetName *************************************************
**
** Set the name element of an Ensembl Quality Check Sequence.
**
** @param [u] qcs [EnsPQcsequence] Ensembl Quality Check Sequence
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




/* @func ensQcsequenceSetPolya ************************************************
**
** Set the PolyA+ tail length element of an Ensembl Quality Check Sequence.
**
** @param [u] qcs [EnsPQcsequence] Ensembl Quality Check Sequence
** @param [r] polya [ajuint] PolyA+ tail length
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
** @@
******************************************************************************/

AjBool ensQcsequenceSetPolya(EnsPQcsequence qcs, ajuint polya)
{
    if(!qcs)
        return ajFalse;

    qcs->Polya = polya;

    return ajTrue;
}




/* @func ensQcsequenceSetQcdatabase *******************************************
**
** Set the Ensembl Quality Check Database element of an
** Ensembl Quality Check Sequence.
**
** @param [u] qcs [EnsPQcsequence] Ensembl Quality Check Sequence
** @param [uN] qcdb [EnsPQcdatabase] Ensembl Quality Check Database
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




/* @func ensQcsequenceSetType *************************************************
**
** Set the type element of an Ensembl Quality Check Sequence.
**
** @param [u] qcs [EnsPQcsequence] Ensembl Quality Check Sequence
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




/* @func ensQcsequenceSetVersion **********************************************
**
** Set the version element of an Ensembl Quality Check Sequence.
**
** @param [u] qcs [EnsPQcsequence] Ensembl Quality Check Sequence
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




/* @section debugging *********************************************************
**
** Functions for reporting of an Ensembl Quality Check Sequence object.
**
** @fdata [EnsPQcsequence]
**
** @nam3rule Trace Report Ensembl Quality Check Sequence elements to debug file
**
** @argrule Trace qcs [const EnsPQcsequence] Ensembl Quality Check Sequence
** @argrule Trace level [ajuint] Indentation level
**
** @valrule * [AjBool] ajTrue upon success, ajFalse otherwise
**
** @fcategory misc
******************************************************************************/




/* @func ensQcsequenceTrace ***************************************************
**
** Trace an Ensembl Quality Check Sequence.
**
** @param [r] qcs [const EnsPQcsequence] Ensembl Quality Check Sequence
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
            "%S  CdsStart %u\n"
            "%S  CdsEnd %u\n"
            "%S  CdsStrand %d\n"
            "%S  Polya %u\n"
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
            indent, qcs->CdsStart,
            indent, qcs->CdsEnd,
            indent, qcs->CdsStrand,
            indent, qcs->Polya,
            indent, qcs->Description);

    ensQcdatabaseTrace(qcs->Qcdatabase, 1);

    ajStrDel(&indent);

    return ajTrue;
}




/* @section calculate *********************************************************
**
** Functions for calculating values of an
** Ensembl Quality Check Sequence object.
**
** @fdata [EnsPQcsequence]
**
** @nam3rule Calculate   Calculate Ensembl Quality Check Sequence values
** @nam4rule Memsize     Calculate the memory size in bytes
**
** @argrule Memsize qcs [const EnsPQcsequence] Ensembl Quality Check Sequence
**
** @valrule Memsize [size_t] Memory size in bytes or 0
**
** @fcategory misc
******************************************************************************/




/* @func ensQcsequenceCalculateMemsize ****************************************
**
** Get the memory size in bytes of an Ensembl Quality Check Sequence.
**
** @param [r] qcs [const EnsPQcsequence] Ensembl Quality Check Sequence
**
** @return [size_t] Memory size in bytes or 0
** @@
******************************************************************************/

size_t ensQcsequenceCalculateMemsize(const EnsPQcsequence qcs)
{
    size_t size = 0;

    if(!qcs)
        return 0;

    size += sizeof (EnsOQcsequence);

    size += ensQcdatabaseCalculateMemsize(qcs->Qcdatabase);

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




/* @section convenience functions *********************************************
**
** Ensembl Quality Check Sequence convenience functions
**
** @fdata [EnsPQcsequence]
**
** @nam3rule Get Get member(s) of associated objects
** @nam4rule Qcdatabase Get Ensembl Quality Check Database members
** @nam5rule Identifier Get the SQL database-internal identifier
**
** @argrule * qcs [const EnsPQcsequence] Ensembl Quality Check Sequence
**
** @valrule QcdatabaseIdentifier [ajuint] Ensembl Quality Check Database
** SQL database-internal identifier or 0
**
** @fcategory use
******************************************************************************/




/* @func ensQcsequenceGetQcdatabaseIdentifier *********************************
**
** Get the Ensembl Quality Check Database SQL database-internal identifier
** of an Ensembl Quality Check Sequence.
**
** @param [r] qcs [const EnsPQcsequence] Ensembl Quality Check Sequence
**
** @return [ajuint] Ensembl Quality Check Database SQL database-internal
** identifier or 0
** @@
******************************************************************************/

ajuint ensQcsequenceGetQcdatabaseIdentifier(const EnsPQcsequence qcs)
{
    if(!qcs)
        return 0;

    return ensQcdatabaseGetIdentifier(qcs->Qcdatabase);
}




/* @section matching **********************************************************
**
** Functions for matching Ensembl Quality Check Sequence objects
**
** @fdata [EnsPQcsequence]
**
** @nam3rule Match Compare two Ensembl Quality Check Sequence objects
**
** @argrule * qcs1 [const EnsPQcsequence] Ensembl Quality Check Sequence
** @argrule * qcs2 [const EnsPQcsequence] Ensembl Quality Check Sequence
**
** @valrule * [AjBool] True on success
**
** @fcategory use
******************************************************************************/




/* @func ensQcsequenceMatch ***************************************************
**
** Tests for matching two Ensembl Quality Check Sequences.
**
** @param [r] qcs1 [const EnsPQcsequence] First Ensembl Quality Check Sequence
** @param [r] qcs2 [const EnsPQcsequence] Second Ensembl Quality Check Sequence
**
** @return [AjBool] ajTrue if the Ensembl Quality Check Sequences are equal
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

    if(qcs1->CdsStart != qcs2->CdsStart)
        return ajFalse;

    if(qcs1->CdsEnd != qcs2->CdsEnd)
        return ajFalse;

    if(qcs1->CdsStrand != qcs2->CdsStrand)
        return ajFalse;

    if(qcs1->Polya != qcs2->Polya)
        return ajFalse;

    return ajTrue;
}




/* @section fetch *************************************************************
**
** Functions for fetching values of an Ensembl Quality Check Sequence object.
**
** @fdata [EnsPQcsequence]
**
** @nam3rule Fetch Fetch Ensembl Quality Check Sequence values
** @nam4rule All Fetch all objects
** @nam4rule Anchor Fetch an HTML anchor
** @nam5rule External Fetch an HTML document-external anchor
** @nam5rule Internal Fetch an HTML document-internal anchor
** @nam4rule Url Fetch a Uniform Resource Locator (URL)
** @nam5rule External Fetch an external Uniform Resource Locator (URL)
** @nam5rule Internal Fetch an internal Uniform Resource Locator (URL)
**
** @argrule AnchorExternal qcs [const EnsPQcsequence]
** Ensembl Quality Check Sequence
** @argrule AnchorExternal htmlid [AjBool] Set the HTML id attribute in the
** anchor element
** @argrule AnchorInternal qcs [const EnsPQcsequence]
** Ensembl Quality Check Sequence
** @argrule Anchor Pstr [AjPStr*] Anchor string
** @argrule UrlExternal qcs [const EnsPQcsequence]
** Ensembl Quality Check Sequence
** @argrule UrlExternal Pstr [AjPStr*] URL string
**
** @valrule * [AjBool] ajTrue upon success, ajFalse otherwise
**
** @fcategory misc
******************************************************************************/




/* @func ensQcsequenceFetchAnchorExternal *************************************
**
** Fetch an external HTML anchor for an Ensembl Quality Check Sequence.
**
** Returns a complete HTML anchor element, which displays the sequence name
** and links it to the corresponding sequence entry in the external source
** database. Linking is dependent on template information in the
** 'sequence_db.external_url' field of the Ensembl Quality Check database.
** Although the sequence name is displayed, the link is based on ###NAME###,
** ###ACCESSION### or ###VERSION### placeholders in the template URL. If no
** template isavailable, just an anchor element the name set as 'id' attribute
** is returned.
**
** @param [r] qcs [const EnsPQcsequence] Ensembl Quality Check Sequence
** @param [r] htmlid [AjBool] Set the HTML id attribute in the anchor element
** @param [w] Pstr [AjPStr*] Anchor string
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
** @@
******************************************************************************/

AjBool ensQcsequenceFetchAnchorExternal(const EnsPQcsequence qcs,
                                        AjBool htmlid,
                                        AjPStr* Pstr)
{
    AjPStr sgmlid  = NULL;
    AjPStr exturl  = NULL;
    AjPStr qcsname = NULL;
    AjPStr qcdname = NULL;

    if(!qcs)
        return ajFalse;

    if(!Pstr)
        return ajFalse;

    ensQcsequenceFetchUrlExternal(qcs, &exturl);

    if(exturl && ajStrGetLen(exturl))
    {
        ajStrAppendC(Pstr, "<a ");

        if(htmlid)
        {
            sgmlid = ajStrNewS(qcs->Name);

            ensHtmlEncodeSgmlid(&sgmlid);

            ajFmtPrintAppS(Pstr, "id=\"%S\" ", sgmlid);

            ajStrDel(&sgmlid);
        }

        ensHtmlEncodeEntities(&exturl);

        qcdname = ajStrNewS(ensQcdatabaseGetName(qcs->Qcdatabase));

        ensHtmlEncodeEntities(&qcdname);

        qcsname = ajStrNewS(qcs->Name);

        ensHtmlEncodeEntities(&qcsname);

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

            ensHtmlEncodeSgmlid(&sgmlid);

            qcsname = ajStrNewS(qcs->Name);

            ensHtmlEncodeEntities(&qcsname);

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




/* @func ensQcsequenceFetchAnchorInternal *************************************
**
** Fetch an internal HTML anchor for an Ensembl Quality Check Sequence.
**
** Returns a complete HTML anchor element, which displays the sequence name
** and links it to the corresponding document-internal anchor element.
**
** @param [r] qcs [const EnsPQcsequence] Ensembl Quality Check Sequence
** @param [w] Pstr [AjPStr*] Anchor string
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
** @@
******************************************************************************/

AjBool ensQcsequenceFetchAnchorInternal(const EnsPQcsequence qcs,
                                        AjPStr* Pstr)
{
    AjPStr sgmlid = NULL;
    AjPStr name   = NULL;

    if(!qcs)
        return ajFalse;

    if(!Pstr)
        return ajFalse;

    sgmlid = ajStrNewS(qcs->Name);

    ensHtmlEncodeSgmlid(&sgmlid);

    name = ajStrNewS(qcs->Name);

    ensHtmlEncodeEntities(&name);

    ajFmtPrintAppS(Pstr, "<a href=\"#%S\">%S</a>", sgmlid, name);

    ajStrDel(&sgmlid);
    ajStrDel(&name);

    return ajTrue;
}




/* @func ensQcsequenceFetchUrlExternal ****************************************
**
** Fetch an external URL for an Ensembl Quality Check Sequence.
**
** The caller is responsible for deleting the AJAX String.
**
** @param [r] qcs [const EnsPQcsequence] Ensembl Quality Check Sequence
** @param [u] Pstr [AjPStr*] URL string
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
** @@
******************************************************************************/

AjBool ensQcsequenceFetchUrlExternal(const EnsPQcsequence qcs, AjPStr* Pstr)
{
    AjPStr exturl = NULL;
    AjPStr version = NULL;

    EnsPQcdatabase qcdb = NULL;

    if(!qcs)
        return ajFalse;

    if(!Pstr)
        return ajFalse;

    if(*Pstr)
        ajStrAssignClear(Pstr);
    else
        *Pstr = ajStrNew();

    qcdb = ensQcsequenceGetQcdatabase(qcs);

    exturl = ensQcdatabaseGetUrlExternal(qcdb);

    if(exturl && ajStrGetLen(exturl))
    {
        ajStrAssignS(Pstr, exturl);

        ajStrExchangeCS(Pstr, "###NAME###", qcs->Name);

        ajStrExchangeCS(Pstr, "###ACCESSION###", qcs->Accession);

        version = ajFmtStr("%d", qcs->Version);

        ajStrExchangeCS(Pstr, "###VERSION###", version);

        ajStrDel(&version);
    }

    return ajTrue;
}




/* @datasection [AjPStr] AJAX String ******************************************
**
** @nam2rule Html Functions for manipulating AJAX String objects
**
******************************************************************************/




/* @section encode ************************************************************
**
** Functions for manipulating AJAX String objects cconforming to HTML rules.
**
** @fdata [AjPStr]
**
** @nam3rule Encode Encode AJAX String objects conforming to HTML rules
** @nam4rule Entities Encode HTML character entities
** @nam4rule Sgmlid   Encode HTML SGML identifiers
**
** @argrule ensHtmlEncodeEntities Pstr [AjPStr*] HTML string
** @argrule ensHtmlEncodeSgmlid Pstr [AjPStr*] HTML String
**
** @valrule * [AjBool] ajTrue upon success, ajFalse otherwise
**
** @fcategory misc
******************************************************************************/




/* @func ensHtmlEncodeEntities ************************************************
**
** Encode HTML character entities in an AJAX String.
**
** http://www.w3.org/TR/html401/charset.html#h-5.3.2
**
** @param [u] Pstr [AjPStr*] HTML string
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
** @@
** NOTE: Since EMBOSS is limited to ASCII only '"', '&', '<', and '>' need
** replacing.
** FIXME: This function should move!
******************************************************************************/

AjBool ensHtmlEncodeEntities(AjPStr* Pstr)
{
    if(!Pstr)
        return ajFalse;

    if(!*Pstr)
        return ajFalse;

    /*
    ** FIXME: The & character has to be the first character to replace.
    ** It would be better to check for the occurence of &amp; or &...; more
    ** generally, before replacing.
    */

    ajStrExchangeCC(Pstr, "&", "&amp;");
    ajStrExchangeCC(Pstr, "\"", "&quot;");
    ajStrExchangeCC(Pstr, "<", "&lt;");
    ajStrExchangeCC(Pstr, ">", "&gt;");

    return ajTrue;
}




/* @func ensHtmlEncodeSgmlid **************************************************
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
** @param [w] Pstr [AjPStr*] SGML identifier string
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
** @@
** FIXME: This function should move!
******************************************************************************/

AjBool ensHtmlEncodeSgmlid(AjPStr* Pstr)
{
    const char* txtstr = NULL;

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




/* @datasection [EnsPQcsequenceadaptor] Ensembl Quality Check Sequence Adaptor
**
** @nam2rule Qcsequenceadaptor Functions for manipulating
** Ensembl Quality Check Sequence Adaptor objects
**
** @cc Bio::EnsEMBL::QC::DBSQL::Sequenceadaptor
** @cc CVS Revision:
** @cc CVS Tag:
**
******************************************************************************/




/* @funcstatic qcsequenceadaptorFetchAllbyStatement ***************************
**
** Run a SQL statement against an Ensembl Database Adaptor and consolidate the
** results into an AJAX List of Ensembl Quality Check Sequence objects.
**
** @param [u] dba [EnsPDatabaseadaptor] Ensembl Database Adaptor
** @param [r] statement [const AjPStr] SQL statement
** @param [uN] am [EnsPAssemblymapper] Ensembl Assembly Mapper
** @param [uN] slice [EnsPSlice] Ensembl Slice
** @param [u] qcss [AjPList] AJAX List of Ensembl Quality Check Sequences
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
** @@
******************************************************************************/

static AjBool qcsequenceadaptorFetchAllbyStatement(
    EnsPDatabaseadaptor dba,
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

    if(ajDebugTest("qcsequenceadaptorFetchAllbyStatement"))
        ajDebug("qcsequenceadaptorFetchAllbyStatement\n"
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

        qcs = ensQcsequenceNewIni(qcsa,
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

        ajListPushAppend(qcss, (void*) qcs);

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
** All constructors return a new Ensembl Quality Check Sequence Adaptor by
** pointer. It is the responsibility of the user to first destroy any previous
** Quality Check Sequence Adaptor. The target pointer does not need to be
** initialised to NULL, but it is good programming practice to do so anyway.
**
** @fdata [EnsPQcsequenceadaptor]
**
** @nam3rule New Constructor
**
** @argrule New dba [EnsPDatabaseadaptor] Ensembl Database Adaptor
**
** @valrule * [EnsPQcsequenceadaptor] Ensembl Quality Check Sequence Adaptor
**
** @fcategory new
******************************************************************************/




/* @func ensQcsequenceadaptorNew **********************************************
**
** Default constructor for an Ensembl Quality Check Sequence Adaptor.
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
** @param [u] dba [EnsPDatabaseadaptor] Ensembl Database Adaptor
**
** @return [EnsPQcsequenceadaptor] Ensembl Quality Check Sequence Adaptor
** or NULL
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
        (EnsPBaseadaptorLeftjoin) NULL,
        (const char*) NULL,
        (const char*) NULL,
        qcsequenceadaptorFetchAllbyStatement);

    return qcsa;
}




/* @section destructors *******************************************************
**
** Destruction destroys all internal data structures and frees the
** memory allocated for an Ensembl Quality Check Sequence Adaptor object.
**
** @fdata [EnsPQcsequenceadaptor]
**
** @nam3rule Del Destroy (free) an Ensembl Quality Check Sequence Adaptor
**               object
**
** @argrule * Pqcsa [EnsPQcsequenceadaptor*]
** Ensembl Quality Check Sequence Adaptor object address
**
** @valrule * [void]
**
** @fcategory delete
******************************************************************************/




/* @func ensQcsequenceadaptorDel **********************************************
**
** Default destructor for an Ensembl Quality Check Sequence Adaptor.
**
** Ensembl Object Adaptors are singleton objects that are registered in the
** Ensembl Registry and weakly referenced by Ensembl Objects that have been
** instantiated by it. Therefore, Ensembl Object Adaptors should never be
** destroyed directly. Upon exit, the Ensembl Registry will call this function
** if required.
**
** @param [d] Pqcsa [EnsPQcsequenceadaptor*]
** Ensembl Quality Check Sequence Adaptor object address
**
** @return [void]
** @@
******************************************************************************/

void ensQcsequenceadaptorDel(EnsPQcsequenceadaptor* Pqcsa)
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




/* @section element retrieval *************************************************
**
** Functions for returning elements of an
** Ensembl Quality Check Sequence Adaptor object.
**
** @fdata [EnsPQcsequenceadaptor]
**
** @nam3rule Get Return Ensembl Quality Check Sequence Adaptor attribute(s)
** @nam4rule Baseadaptor Return the Ensembl Base Adaptor
** @nam4rule Databaseadaptor Return the Ensembl Database Adaptor
**
** @argrule * qcsa [EnsPQcsequenceadaptor]
** Ensembl Quality Check Sequence Adaptor
**
** @valrule Baseadaptor [EnsPBaseadaptor] Ensembl Base Adaptor or NULL
** @valrule Databaseadaptor [EnsPDatabaseadaptor] Ensembl Database Adaptor
** or NULL
**
** @fcategory use
******************************************************************************/




/* @func ensQcsequenceadaptorGetBaseadaptor ***********************************
**
** Get the Ensembl Base Adaptor element of an
** Ensembl Quality Check Sequence Adaptor.
**
** @param [u] qcsa [EnsPQcsequenceadaptor]
** Ensembl Quality Check Sequence Adaptor
**
** @return [EnsPBaseadaptor] Ensembl Base Adaptor or NULL
** @@
******************************************************************************/

EnsPBaseadaptor ensQcsequenceadaptorGetBaseadaptor(
    EnsPQcsequenceadaptor qcsa)
{
    if(!qcsa)
        return NULL;

    return qcsa->Adaptor;
}




/* @func ensQcsequenceadaptorGetDatabaseadaptor *******************************
**
** Get the Ensembl Database Adaptor element of an
** Ensembl Quality Check Sequence Adaptor.
**
** @param [u] qcsa [EnsPQcsequenceadaptor]
** Ensembl Quality Check Sequence Adaptor
**
** @return [EnsPDatabaseadaptor] Ensembl Database Adaptor or NULL
** @@
******************************************************************************/

EnsPDatabaseadaptor ensQcsequenceadaptorGetDatabaseadaptor(
    EnsPQcsequenceadaptor qcsa)
{
    if(!qcsa)
        return NULL;

    return ensBaseadaptorGetDatabaseadaptor(qcsa->Adaptor);
}




/* @section object retrieval **************************************************
**
** Functions for fetching Ensembl Quality Check Sequence objects from an
** Ensembl SQL database.
**
** @fdata [EnsPQcsequenceadaptor]
**
** @nam3rule Fetch      Fetch Ensembl Quality Check Sequence object(s)
** @nam4rule All        Fetch all Ensembl Quality Check Sequence objects
** @nam4rule Allby      Fetch all Ensembl Quality Check Sequence objects
**                      matching a criterion
** @nam5rule Qcdatabase Fetch all by an Ensembl Quality Check Database
** @nam4rule By         Fetch one Ensembl Quality Check Sequence object
**                      matching a criterion
** @nam5rule Accession  Fetch by an accession number
** @nam5rule Identifier Fetch by an SQL database internal identifier
** @nam5rule Name       Fetch by a name
**
** @argrule * qcsa [EnsPQcsequenceadaptor]
** Ensembl Quality Check Sequence Adaptor
** @argrule AllbyQcdatabase qcdb [const EnsPQcdatabase]
** Ensembl Quality Check Database
** @argrule Allby qcss [AjPList] AJAX List of Ensembl Quality Check Sequence
** objects
** @argrule ByAccession qcdbid [ajuint]
** Ensembl Quality Check Database identifier
** @argrule ByAccession accession [const AjPStr]
** Ensembl Quality Check Sequence accession number
** @argrule ByName qcdbid [ajuint] Ensembl Quality Check Database identifier
** @argrule ByName name [const AjPStr] Ensembl Quality Check Sequence name
** @argrule ByAccession version [ajuint] Ensembl Quality Check Sequence version
** @argrule ByIdentifier identifier [ajuint] SQL database-internal identifier
** @argrule By Pqcs [EnsPQcsequence*] Ensembl Quality Check Sequence address
**
** @valrule * [AjBool] ajTrue upon success, ajFalse otherwise
**
** @fcategory use
******************************************************************************/




/* @func ensQcsequenceadaptorFetchAllbyQcdatabase *****************************
**
** Fetch all Ensembl Quality Check Sequences via an
** Ensembl Quality Check Database.
** The caller is responsible for deleting the Ensembl Quality Check Sequences
** before deleting the AJAX List.
**
** @param [u] qcsa [EnsPQcsequenceadaptor]
** Ensembl Quality Check Sequence Adaptor
** @param [r] qcdb [const EnsPQcdatabase] Ensembl Quality Check Database
** @param [u] qcss [AjPList] AJAX List of Ensembl Quality Check Sequences
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
** @@
******************************************************************************/

AjBool ensQcsequenceadaptorFetchAllbyQcdatabase(EnsPQcsequenceadaptor qcsa,
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

    ensBaseadaptorFetchAllbyConstraint(qcsa->Adaptor,
                                       constraint,
                                       (EnsPAssemblymapper) NULL,
                                       (EnsPSlice) NULL,
                                       qcss);

    ajStrDel(&constraint);

    return ajTrue;
}




/* @func ensQcsequenceadaptorFetchByAccession *********************************
**
** Fetch an Ensembl Quality Check Sequence by its accession number and
** sequence version.
**
** The caller is responsible for deleting the Ensembl Quality Check Sequence.
**
** @param [u] qcsa [EnsPQcsequenceadaptor]
** Ensembl Quality Check Sequence Adaptor
** @param [r] qcdbid [ajuint] Ensembl Quality Check Database identifier
** @param [r] accession [const AjPStr]
** Ensembl Quality Check Sequence accession number
** @param [rN] version [ajuint]
** Ensembl Quality Check Sequence version
** @param [wP] Pqcs [EnsPQcsequence*] Ensembl Quality Check Sequence address
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
** @@
******************************************************************************/

AjBool ensQcsequenceadaptorFetchByAccession(EnsPQcsequenceadaptor qcsa,
                                            ajuint qcdbid,
                                            const AjPStr accession,
                                            ajuint version,
                                            EnsPQcsequence* Pqcs)
{
    char* txtaccession = NULL;

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

    ensBaseadaptorFetchAllbyConstraint(qcsa->Adaptor,
                                       constraint,
                                       (EnsPAssemblymapper) NULL,
                                       (EnsPSlice) NULL,
                                       qcss);

    if(ajListGetLength(qcss) > 1)
        ajWarn("ensQcsequenceadaptorFetchByAccession got more than "
               "one Ensembl Quality Check Sequence for accession '%S' "
               "and Quality Check Database identifier %u.\n",
               accession, qcdbid);

    ajListPop(qcss, (void**) Pqcs);

    /*
    ** TODO: To implement?
    ** qcsequenceadaptorCacheInsert(qcsa, Pqcs);
    */

    while(ajListPop(qcss, (void**) &qcs))
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




/* @func ensQcsequenceadaptorFetchByIdentifier ********************************
**
** Fetch an Ensembl Quality Check Sequence via its
** SQL database-internal identifier.
**
** The caller is responsible for deleting the Ensembl Quality Check Sequence.
**
** @param [u] qcsa [EnsPQcsequenceadaptor]
** Ensembl Quality Check Sequence Adaptor
** @param [r] identifier [ajuint]
** SQL database-internal Ensembl Quality Check Sequence identifier
** @param [wP] Pqcs [EnsPQcsequence*] Ensembl Quality Check Sequence address
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
** @@
******************************************************************************/

AjBool ensQcsequenceadaptorFetchByIdentifier(EnsPQcsequenceadaptor qcsa,
                                             ajuint identifier,
                                             EnsPQcsequence* Pqcs)
{
    if(!qcsa)
        return ajFalse;

    if(!identifier)
        return ajFalse;

    if(!Pqcs)
        return ajFalse;

    return ensBaseadaptorFetchByIdentifier(qcsa->Adaptor,
                                           identifier,
                                           (void**) Pqcs);
}




/* @func ensQcsequenceadaptorFetchByName **************************************
**
** Fetch an Ensembl Quality Check Sequence by its name.
**
** The caller is responsible for deleting the Ensembl Quality Check Sequence.
**
** @param [u] qcsa [EnsPQcsequenceadaptor]
** Ensembl Quality Check Sequence Adaptor
** @param [r] qcdbid [ajuint] Ensembl Quality Check Database identifier
** @param [r] name [const AjPStr] Ensembl Quality Check Sequence name
** @param [wP] Pqcs [EnsPQcsequence*] Ensembl Quality Check Sequence address
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
** @@
******************************************************************************/

AjBool ensQcsequenceadaptorFetchByName(EnsPQcsequenceadaptor qcsa,
                                       ajuint qcdbid,
                                       const AjPStr name,
                                       EnsPQcsequence* Pqcs)
{
    char* txtname = NULL;

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

    ensBaseadaptorFetchAllbyConstraint(qcsa->Adaptor,
                                       constraint,
                                       (EnsPAssemblymapper) NULL,
                                       (EnsPSlice) NULL,
                                       qcss);

    if(ajListGetLength(qcss) > 1)
        ajWarn("ensQcsequenceadaptorFetchByName got more than "
               "one Ensembl Quality Check Sequence for (UNIQUE) name '%S' "
               "and Quality Check Database identifier %u.\n",
               name, qcdbid);

    ajListPop(qcss, (void**) Pqcs);

    /*
    ** TODO: To implement?
    ** qcsequenceadaptorCacheInsert(qcsa, Pqcs);
    */

    while(ajListPop(qcss, (void**) &qcs))
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




/* @section object access *****************************************************
**
** Functions for accessing Ensembl Quality Check Sequence objects in an
** Ensembl SQL database.
**
** @fdata [EnsPQcsequenceadaptor]
**
** @nam3rule Delete Delete Ensembl Quality Check Sequence object(s)
** @nam3rule Store  Store Ensembl Quality Check Sequence object(s)
** @nam3rule Update Update Ensembl Quality Check Sequence object(s)
**
** @argrule * qcsa [EnsPQcsequenceadaptor]
** Ensembl Quality Check Sequence Adaptor
** @argrule Delete qcs [EnsPQcsequence] Ensembl Quality Check Sequence
** @argrule Store qcs [EnsPQcsequence] Ensembl Quality Check Sequence
** @argrule Update qcs [const EnsPQcsequence] Ensembl Quality Check Sequence
**
** @valrule * [AjBool] ajTrue upon success, ajFalse otherwise
**
** @fcategory use
******************************************************************************/




/* @func ensQcsequenceadaptorDelete *******************************************
**
** Delete an Ensembl Quality Check Sequence.
**
** @param [u] qcsa [EnsPQcsequenceadaptor]
** Ensembl Quality Check Sequence Adaptor
** @param [u] qcs [EnsPQcsequence] Ensembl Quality Check Sequence
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
** @@
******************************************************************************/

AjBool ensQcsequenceadaptorDelete(EnsPQcsequenceadaptor qcsa,
                                  EnsPQcsequence qcs)
{
    AjBool result = AJFALSE;

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

        result = ajTrue;
    }

    ensDatabaseadaptorSqlstatementDel(dba, &sqls);

    ajStrDel(&statement);

    return result;
}




/* @func ensQcsequenceadaptorStore ********************************************
**
** Store an Ensembl Quality Check Sequence.
**
** @param [u] qcsa [EnsPQcsequenceadaptor]
** Ensembl Quality Check Sequence Adaptor
** @param [u] qcs [EnsPQcsequence] Ensembl Quality Check Seqeunce
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
** @@
******************************************************************************/

AjBool ensQcsequenceadaptorStore(EnsPQcsequenceadaptor qcsa,
                                 EnsPQcsequence qcs)
{
    char* txtname        = NULL;
    char* txtaccession   = NULL;
    char* txtdescription = NULL;
    char* txttype        = NULL;

    AjBool result = AJFALSE;

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
                         qcs->CdsStart,
                         qcs->CdsEnd,
                         qcs->CdsStrand,
                         qcs->Polya);

    ajCharDel(&txtname);
    ajCharDel(&txtaccession);
    ajCharDel(&txtdescription);
    ajCharDel(&txttype);

    sqls = ensDatabaseadaptorSqlstatementNew(dba, statement);

    if(ajSqlstatementGetAffectedrows(sqls))
    {
        ensQcsequenceSetIdentifier(qcs, ajSqlstatementGetIdentifier(sqls));

        ensQcsequenceSetAdaptor(qcs, qcsa);

        result = ajTrue;
    }

    ensDatabaseadaptorSqlstatementDel(dba, &sqls);

    ajStrDel(&statement);

    return result;
}




/* @func ensQcsequenceadaptorUpdate *******************************************
**
** Update an Ensembl Quality Check Sequence.
**
** @param [u] qcsa [EnsPQcsequenceadaptor]
** Ensembl Quality Check Sequence Adaptor
** @param [r] qcs [const EnsPQcsequence] Ensembl Quality Check Sequence
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
** @@
******************************************************************************/

AjBool ensQcsequenceadaptorUpdate(EnsPQcsequenceadaptor qcsa,
                                  const EnsPQcsequence qcs)
{
    char* txtname        = NULL;
    char* txtaccession   = NULL;
    char* txtdescription = NULL;
    char* txttype        = NULL;

    AjBool result = AJFALSE;

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
                         qcs->CdsStart,
                         qcs->CdsEnd,
                         qcs->CdsStrand,
                         qcs->Polya,
                         qcs->Identifier);

    ajCharDel(&txtname);
    ajCharDel(&txtaccession);
    ajCharDel(&txtdescription);
    ajCharDel(&txttype);

    sqls = ensDatabaseadaptorSqlstatementNew(dba, statement);

    if(ajSqlstatementGetAffectedrows(sqls))
        result = ajTrue;

    ensDatabaseadaptorSqlstatementDel(dba, &sqls);

    ajStrDel(&statement);

    return result;
}
