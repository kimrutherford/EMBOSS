/******************************************************************************
** @source Ensembl Genetic Variation functions
**
** @author Copyright (C) 1999 Ensembl Developers
** @author Copyright (C) 2006 Michael K. Schuster
** @modified 2009 by Alan Bleasby for incorporation into EMBOSS core
** @version $Revision: 1.5 $
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

#include "ensvariation.h"




/* ==================================================================== */
/* ========================== private data ============================ */
/* ==================================================================== */




/* ==================================================================== */
/* ======================== private functions ========================= */
/* ==================================================================== */

extern EnsPMetainformationadaptor
ensRegistryGetMetainformationadaptor(EnsPDatabaseadaptor dba);

extern EnsPGvindividualadaptor
ensRegistryGetGvindividualadaptor(EnsPDatabaseadaptor dba);

extern EnsPGvpopulationadaptor
ensRegistryGetGvpopulationadaptor(EnsPDatabaseadaptor dba);

extern EnsPGvsampleadaptor
ensRegistryGetGvsampleadaptor(EnsPDatabaseadaptor dba);

extern EnsPGvvariationadaptor
ensRegistryGetGvvariationadaptor(EnsPDatabaseadaptor dba);

extern EnsPSliceadaptor
ensRegistryGetSliceadaptor(EnsPDatabaseadaptor dba);

static void gvVariationadaptorClearAlleles(void **key, void **value, void *cl);

static void gvVariationadaptorClearPopulations(void **key,
                                               void **value,
                                               void *cl);

static void gvVariationadaptorClearSynonyms(void **key,
                                            void **value,
                                            void *cl);

static AjBool gvVariationadaptorFetchAllBySQL(EnsPGvvariationadaptor gvva,
                                              const AjPStr statement,
                                              AjPList gvvs);

static AjBool gvVariationadaptorFetchFlankFromCore(EnsPGvvariationadaptor gvva,
                                                   ajuint srid,
                                                   ajint srstart,
                                                   ajint srend,
                                                   ajint srstrand,
                                                   AjPStr *Psequence);




/* @filesection ensvariation **************************************************
**
** @nam1rule ens Function belongs to the Ensembl library
**
******************************************************************************/




/* @datasection [EnsPGvallele] Genetic Variation Allele ***********************
**
** Functions for manipulating Ensembl Genetic Variation Allele objects
**
** @cc Bio::EnsEMBL::Variation::Allele CVS Revision: 1.3
**
** @nam2rule Gvallele
**
******************************************************************************/




/* @section constructors ******************************************************
**
** All constructors return a new Ensembl Genetic Variation Allele by pointer.
** It is the responsibility of the user to first destroy any previous
** Genetic Variation Allele. The target pointer does not need to be
** initialised to NULL, but it is good programming practice to do so anyway.
**
** @fdata [EnsPGvallele]
** @fnote None
**
** @nam3rule New Constructor
** @nam4rule NewObj Constructor with existing object
** @nam4rule NewRef Constructor by incrementing the reference counter
**
** @argrule Obj object [EnsPGvallele] Ensembl Genetic Variation Allele
** @argrule Ref object [EnsPGvallele] Ensembl Genetic Variation Allele
**
** @valrule * [EnsPGvallele] Ensembl Genetic Variation Allele
**
** @fcategory new
******************************************************************************/




/* @func ensGvalleleNew *******************************************************
**
** Default constructor for an Ensembl Genetic Variation Allele.
**
** @cc Bio::EnsEMBL::Storable::new
** @param [r] gvaa [EnsPGvalleleadaptor] Ensembl Genetic Variation
**                                       Allele Adaptor
** @param [r] identifier [ajuint] SQL database-internal identifier
** @cc Bio::EnsEMBL::Variation::Allele::new
** @param [u] gvp [EnsPGvpopulation] Ensembl Genetic Variation Population
** @param [u] allelestr [AjPStr] Allele
** @param [r] frequency [float] Frequency
** @param [r] subsnpid [ajuint] Sub-SNP identifier
**
** @return [EnsPGvallele] Ensembl Genetic Variation Allele
** @@
******************************************************************************/

EnsPGvallele ensGvalleleNew(EnsPGvalleleadaptor gvaa,
                            ajuint identifier,
                            EnsPGvpopulation gvp,
                            AjPStr allelestr,
                            float frequency,
                            ajuint subsnpid)
{
    EnsPGvallele gva = NULL;

    if(!gvp)
        return NULL;

    if(!allelestr)
        return NULL;

    AJNEW0(gva);

    gva->Use = 1;

    gva->Identifier = identifier;

    gva->Adaptor = gvaa;

    gva->Gvpopulation = ensGvpopulationNewRef(gvp);

    if(allelestr)
        gva->Allele = ajStrNewS(allelestr);

    gva->Frequency = frequency;

    gva->SubSNPIdentifier = subsnpid;

    return gva;
}




/* @func ensGvalleleNewObj ****************************************************
**
** Object-based constructor function, which returns an independent object.
**
** @param [r] object [const EnsPGvallele] Ensembl Genetic Variation Allele
**
** @return [EnsPGvallele] Ensembl Genetic Variation Allele or NULL
** @@
******************************************************************************/

EnsPGvallele ensGvalleleNewObj(const EnsPGvallele object)
{
    EnsPGvallele gva = NULL;

    if(!object)
        return NULL;

    AJNEW0(gva);

    gva->Use = 1;

    gva->Identifier = object->Identifier;

    gva->Adaptor = object->Adaptor;

    gva->Gvpopulation = ensGvpopulationNewRef(object->Gvpopulation);

    if(object->Allele)
        gva->Allele = ajStrNewRef(object->Allele);

    gva->Frequency = object->Frequency;

    gva->SubSNPIdentifier = object->SubSNPIdentifier;

    return gva;
}




/* @func ensGvalleleNewRef ****************************************************
**
** Ensembl Object referencing function, which returns a pointer to the
** Ensembl Object passed in and increases its reference count.
**
** @param [u] gva [EnsPGvallele] Ensembl Genetic Variation Allele
**
** @return [EnsPGvallele] Ensembl Genetic Variation Allele
** @@
******************************************************************************/

EnsPGvallele ensGvalleleNewRef(EnsPGvallele gva)
{
    if(!gva)
        return NULL;

    gva->Use++;

    return gva;
}




/* @section destructors *******************************************************
**
** Destruction destroys all internal data structures and frees the
** memory allocated for the Ensembl Genetic Variation Allele.
**
** @fdata [EnsPGvallele]
** @fnote None
**
** @nam3rule Del Destroy (free) a Genetic Variation Allele object
**
** @argrule * Pgva [EnsPGvallele*] Genetic Variation Allele object address
**
** @valrule * [void]
**
** @fcategory delete
******************************************************************************/




/* @func ensGvalleleDel *******************************************************
**
** Default destructor for an Ensembl Genetic Variation Allele.
**
** @param [d] Pgva [EnsPGvallele*] Ensembl Genetic Variation Allele address
**
** @return [void]
** @@
******************************************************************************/

void ensGvalleleDel(EnsPGvallele *Pgva)
{
    EnsPGvallele pthis = NULL;

    if(!Pgva)
        return;

    if(!*Pgva)
        return;

    if(ajDebugTest("ensGvalleleDel"))
    {
        ajDebug("ensGvalleleDel\n"
                "  *Pgva %p\n",
                *Pgva);

        ensGvalleleTrace(*Pgva, 1);
    }

    pthis = *Pgva;

    pthis->Use--;

    if(pthis->Use)
    {
        *Pgva = NULL;

        return;
    }

    ensGvpopulationDel(&pthis->Gvpopulation);

    ajStrDel(&pthis->Allele);

    AJFREE(pthis);

    *Pgva = NULL;

    return;
}




/* @section element retrieval *************************************************
**
** Functions for returning elements of an
** Ensembl Genetic Variation Allele object.
**
** @fdata [EnsPGvallele]
** @fnote None
**
** @nam3rule Get Return Genetic Variation Allele attribute(s)
** @nam4rule GetAdaptor Return the Ensembl Genetic Variation Allele Adaptor
** @nam4rule GetIdentifier Return the SQL database-internal identifier
** @nam4rule GetGvpopulation Return the Ensembl Genetic Variation Population
** @nam4rule GetAllele Return the allele
** @nam4rule GetFrequency Return the frequency
** @nam4rule GetSubSNPIdentifier Return the sub-SNP identifier
**
** @argrule * gva [const EnsPGvallele] Genetic Variation Allele
**
** @valrule Adaptor [EnsPGvalleleadaptor] Ensembl Genetic Variation
**                                        Allele Adaptor
** @valrule Identifier [ajuint] SQL database-internal identifier
** @valrule Gvpopulation [EnsPGvpopulation] Ensembl Genetic Variation
**                                          Population
** @valrule Allele [AjPStr] Allele
** @valrule Frequency [float] Frequency
** @valrule SubSNPIdentifier [ajuint] Sub-SNP identifier
**
** @fcategory use
******************************************************************************/




/* @func ensGvalleleGetAdaptor ************************************************
**
** Get the Ensembl Genetic Variation Allele Adaptor element of an
** Ensembl Genetic Variation Allele.
**
** @param [r] gva [const EnsPGvallele] Ensembl Genetic Variation Allele
**
** @return [EnsPGvalleleadaptor] Ensembl Genetic Variation Allele Adaptor
** @@
******************************************************************************/

EnsPGvalleleadaptor ensGvalleleGetAdaptor(const EnsPGvallele gva)
{
    if(!gva)
        return NULL;

    return gva->Adaptor;
}




/* @func ensGvalleleGetIdentifier *********************************************
**
** Get the SQL database-internal identifier element of an
** Ensembl Genetic Variation Allele.
**
** @param [r] gva [const EnsPGvallele] Ensembl Genetic Variation Allele
**
** @return [ajuint] Internal database identifier
** @@
******************************************************************************/

ajuint ensGvalleleGetIdentifier(const EnsPGvallele gva)
{
    if(!gva)
        return 0;

    return gva->Identifier;
}




/* @func ensGvalleleGetPopulation *********************************************
**
** Get the Ensembl Genetic Variation Population element of an
** Ensembl Genetic Variation Allele.
**
** @param [r] gva [const EnsPGvallele] Ensembl Genetic Variation Allele
**
** @return [EnsPGvpopulation] Ensembl Genetic Variation Population
** @@
******************************************************************************/

EnsPGvpopulation ensGvalleleGetPopulation(const EnsPGvallele gva)
{
    if(!gva)
        return NULL;

    return gva->Gvpopulation;
}




/* @func ensGvalleleGetAllele *************************************************
**
** Get the allele element of an Ensembl Genetic Variation Allele.
**
** @param [r] gva [const EnsPGvallele] Ensembl Genetic Variation Allele
**
** @return [AjPStr] Allele
** @@
******************************************************************************/

AjPStr ensGvalleleGetAllele(const EnsPGvallele gva)
{
    if(!gva)
        return NULL;

    return gva->Allele;
}




/* @func ensGvalleleGetFrequency **********************************************
**
** Get the frequency element of an Ensembl Genetic Variation Allele.
**
** @param [r] gva [const EnsPGvallele] Ensembl Genetic Variation Allele
**
** @return [float] Frequency
** @@
******************************************************************************/

float ensGvalleleGetFrequency(const EnsPGvallele gva)
{
    if(!gva)
        return 0;

    return gva->Frequency;
}




/* @func ensGvalleleGetSubSNPIdentifier ***************************************
**
** Get the sub-SNP identifier element of an Ensembl Genetic Variation Allele.
**
** @param [r] gva [const EnsPGvallele] Ensembl Genetic Variation Allele
**
** @return [ajuint] Sub-SNP identifier
** @@
******************************************************************************/

ajuint
ensGvalleleGetSubSNPIdentifier(const EnsPGvallele gva)
{
    if(!gva)
        return 0;

    return gva->SubSNPIdentifier;
}




/* @section element assignment ************************************************
**
** Functions for assigning elements of an
** Ensembl Genetic Variation Allele object.
**
** @fdata [EnsPGvallele]
** @fnote None
**
** @nam3rule Set Set one element of a Genetic Variation Allele
** @nam4rule SetAdaptor Set the Ensembl Genetic Variation Allele Adaptor
** @nam4rule SetIdentifier Set the SQL database-internal identifier
** @nam4rule SetGvpopulation Set the Ensembl Genetic Variation Population
** @nam4rule SetAllele Set the allele
** @nam4rule SetFrequency Set the frequency
** @nam4rule SetSubSNPIdentifier Set the sub-SNP identifier
**
** @argrule * gva [EnsPGvallele] Ensembl Genetic Variation Allele object
**
** @valrule * [AjBool] ajTrue upon success, ajFalse otherwise
**
** @fcategory modify
******************************************************************************/




/* @func ensGvalleleSetAdaptor ************************************************
**
** Set the Ensembl Genetic Variation Allele Adaptor element of an
** Ensembl Genetic Variation Allele.
**
** @param [u] gva [EnsPGvallele] Ensembl Genetic Variation Allele
** @param [r] gvaa [EnsPGvalleleadaptor] Ensembl Genetic Variation
**                                       Allele Adaptor
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
** @@
******************************************************************************/

AjBool ensGvalleleSetAdaptor(EnsPGvallele gva, EnsPGvalleleadaptor gvaa)
{
    if(!gva)
        return ajFalse;

    gva->Adaptor = gvaa;

    return ajTrue;
}




/* @func ensGvalleleSetIdentifier *********************************************
**
** Set the SQL database-internal identifier element of an
** Ensembl Genetic Variation Allele.
**
** @param [u] gva [EnsPGvallele] Ensembl Genetic Variation Allele
** @param [r] identifier [ajuint] SQL database-internal identifier
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
** @@
******************************************************************************/

AjBool ensGvalleleSetIdentifier(EnsPGvallele gva, ajuint identifier)
{
    if(!gva)
        return ajFalse;

    gva->Identifier = identifier;

    return ajTrue;
}




/* @func ensGvalleleSetPopulation *********************************************
**
** Set the Ensembl Genetic Variation Population element of an
** Ensembl Genetic Variation Allele.
**
** @param [u] gva [EnsPGvallele] Ensembl Genetic Variation Allele
** @param [u] gvp [EnsPGvpopulation] Ensembl Genetic Variation Population
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
** @@
******************************************************************************/

AjBool ensGvalleleSetPopulation(EnsPGvallele gva, EnsPGvpopulation gvp)
{
    if(!gva)
        return ajFalse;

    ensGvpopulationDel(&gva->Gvpopulation);

    gva->Gvpopulation = ensGvpopulationNewRef(gvp);

    return ajTrue;
}




/* @func ensGvalleleSetAllele *************************************************
**
** Set the allele element of an Ensembl Genetic Variation Allele.
**
** @param [u] gva [EnsPGvallele] Ensembl Genetic Variation Allele
** @param [u] allelestr [AjPStr] Allele
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
** @@
******************************************************************************/

AjBool ensGvalleleSetAllele(EnsPGvallele gva, AjPStr allelestr)
{
    if(!gva)
        return ajFalse;

    ajStrDel(&gva->Allele);

    if(allelestr)
        gva->Allele = ajStrNewRef(allelestr);

    return ajTrue;
}




/* @func ensGvalleleSetFrequency **********************************************
**
** Set the frequency element of an Ensembl Genetic Variation Allele.
**
** @param [u] gva [EnsPGvallele] Ensembl Genetic Variation Allele
** @param [r] frequency [float] Frequency
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
** @@
******************************************************************************/

AjBool ensGvalleleSetFrequency(EnsPGvallele gva, float frequency)
{
    if(!gva)
        return ajFalse;

    gva->Frequency = frequency;

    return ajTrue;
}




/* @func ensGvalleleSetSubSNPIdentifier ***************************************
**
** Set the sub-SNP identifier element of an Ensembl Genetic Variation Allele.
**
** @param [u] gva [EnsPGvallele] Ensembl Genetic Variation Allele
** @param [r] subsnpid [ajuint] Sub-SNP identifier
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
** @@
******************************************************************************/

AjBool
ensGvalleleSetSubSNPIdentifier(EnsPGvallele gva, ajuint subsnpid)
{
    if(!gva)
        return ajFalse;

    gva->SubSNPIdentifier = subsnpid;

    return ajTrue;
}




/* @func ensGvalleleGetMemSize ************************************************
**
** Get the memory size in bytes of an Ensembl Genetic Variation Allele.
**
** @param [r] gva [const EnsPGvallele] Ensembl Genetic Variation Allele
**
** @return [ajuint] Memory size
** @@
******************************************************************************/

ajuint ensGvalleleGetMemSize(const EnsPGvallele gva)
{
    ajuint size = 0;

    if(!gva)
        return 0;

    size += (ajuint) sizeof (EnsOGvallele);

    size += ensGvpopulationGetMemSize(gva->Gvpopulation);

    if(gva->Allele)
    {
        size += (ajuint) sizeof (AjOStr);

        size += ajStrGetRes(gva->Allele);
    }

    return size;
}




/* @section debugging *********************************************************
**
** Functions for reporting of an Ensembl Genetic Variation Allele object.
**
** @fdata [EnsPGvallele]
** @nam3rule Trace Report Ensembl Genetic Variation Allele elements to
**                 debug file
**
** @argrule Trace gva [const EnsPGvallele] Ensembl Genetic Variation Allele
** @argrule Trace level [ajuint] Indentation level
**
** @valrule * [AjBool] ajTrue upon success, ajFalse otherwise
**
** @fcategory misc
******************************************************************************/




/* @func ensGvalleleTrace *****************************************************
**
** Trace an Ensembl Genetic Variation Allele.
**
** @param [r] gva [const EnsPGvallele] Ensembl Genetic Variation Allele
** @param [r] level [ajuint] Indentation level
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
** @@
******************************************************************************/

AjBool ensGvalleleTrace(const EnsPGvallele gva, ajuint level)
{
    AjPStr indent = NULL;

    if(!gva)
        return ajFalse;

    indent = ajStrNew();

    ajStrAppendCountK(&indent, ' ', level * 2);

    ajDebug("%SensGvalleleTrace %p\n"
            "%S  Use %u\n"
            "%S  Identifier %u\n"
            "%S  Adaptor %p\n"
            "%S  Population %p\n"
            "%S  Allele '%S'\n"
            "%S  Frequency %f\n"
            "%S  SubSNPIdentifier %u\n",
            indent, gva,
            indent, gva->Use,
            indent, gva->Identifier,
            indent, gva->Adaptor,
            indent, gva->Gvpopulation,
            indent, gva->Allele,
            indent, gva->Frequency,
            indent, gva->SubSNPIdentifier);

    ensGvpopulationTrace(gva->Gvpopulation, level + 1);

    ajStrDel(&indent);

    return ajTrue;
}




/* @datasection [EnsPGvgenotype] Genetic Variation Genotype *******************
**
** Functions for manipulating Ensembl Genetic Variation Genotype objects
**
** @nam2rule Gvgenotype
**
******************************************************************************/




/* @section constructors ******************************************************
**
** All constructors return a new Ensembl Genetic Variation Genotype by pointer.
** It is the responsibility of the user to first destroy any previous
** Genetic Variation Genotype. The target pointer does not need to be
** initialised to NULL, but it is good programming practice to do so anyway.
**
** @fdata [EnsPGvgenotype]
** @fnote None
**
** @nam3rule New Constructor
** @nam4rule NewObj Constructor with existing object
** @nam4rule NewRef Constructor by incrementing the reference counter
**
** @argrule Obj object [EnsPGvgenotype] Ensembl Genetic Variation Genotype
** @argrule Ref object [EnsPGvgenotype] Ensembl Genetic Variation Genotype
**
** @valrule * [EnsPGvgenotype] Ensembl Genetic Variation Genotype
**
** @fcategory new
******************************************************************************/




/* @func ensGvgenotypeNew *****************************************************
**
** Default constructor for an Ensembl Genetic Variation Genotype.
**
** @cc Bio::EnsEMBL::Storable::new
** @param [r] gvga [EnsPGvgenotypeadaptor] Ensembl Genetic Variation
**                                         Genotype Adaptor
** @param [r] identifier [ajuint] SQL database-internal identifier
** @cc Bio::EnsEMBL::Variation::Genotype::new
** @param [u] allele1 [AjPStr] Allele 1
** @param [u] allele2 [AjPStr] Allele 2
**
** @return [EnsPGvgenotype] Ensembl Genetic Variation Genotype
** @@
******************************************************************************/

EnsPGvgenotype ensGvgenotypeNew(EnsPGvgenotypeadaptor gvga,
                                ajuint identifier,
                                AjPStr allele1,
                                AjPStr allele2)
{
    EnsPGvgenotype gvg = NULL;

    AJNEW0(gvg);

    gvg->Use = 1;

    gvg->Identifier = identifier;

    gvg->Adaptor = gvga;

    if(allele1)
        gvg->Allele1 = ajStrNewRef(allele1);

    if(allele2)
        gvg->Allele2 = ajStrNewRef(allele2);

    return gvg;
}




/* @func ensGvgenotypeNewObj **************************************************
**
** Object-based constructor function, which returns an independent object.
**
** @param [r] object [const EnsPGvgenotype] Ensembl Genetic Variation Genotype
**
** @return [EnsPGvgenotype] Ensembl Genetic Variation Genotype or NULL
** @@
******************************************************************************/

EnsPGvgenotype ensGvgenotypeNewObj(const EnsPGvgenotype object)
{
    EnsPGvgenotype gvg = NULL;

    if(!object)
        return NULL;

    AJNEW0(gvg);

    gvg->Use = 1;

    gvg->Identifier = object->Identifier;

    gvg->Adaptor = object->Adaptor;

    if(object->Allele1)
        gvg->Allele1 = ajStrNewRef(object->Allele1);

    if(object->Allele2)
        gvg->Allele2 = ajStrNewRef(object->Allele2);

    return gvg;
}




/* @func ensGvgenotypeNewRef **************************************************
**
** Ensembl Object referencing function, which returns a pointer to the
** Ensembl Object passed in and increases its reference count.
**
** @param [u] gvg [EnsPGvgenotype] Ensembl Genetic Variation Genotype
**
** @return [EnsPGvgenotype] Ensembl Genetic Variation Genotype
** @@
******************************************************************************/

EnsPGvgenotype ensGvgenotypeNewRef(EnsPGvgenotype gvg)
{
    if(!gvg)
        return NULL;

    gvg->Use++;

    return gvg;
}




/* @section destructors *******************************************************
**
** Destruction destroys all internal data structures and frees the
** memory allocated for the Ensembl Genetic Variation Genotype.
**
** @fdata [EnsPGvgenotype]
** @fnote None
**
** @nam3rule Del Destroy (free) a Genetic Variation Genotype object
**
** @argrule * Pgvg [EnsPGvgenotype*] Genetic Variation Genotype
**                                   object address
**
** @valrule * [void]
**
** @fcategory delete
******************************************************************************/




/* @func ensGvgenotypeDel *****************************************************
**
** Default destructor for an Ensembl Genetic Variation Genotype.
**
** @param [d] Pgvg [EnsPGvgenotype*] Ensembl Genetic Variation Genotype
**                                   address
**
** @return [void]
** @@
******************************************************************************/

void ensGvgenotypeDel(EnsPGvgenotype *Pgvg)
{
    EnsPGvgenotype pthis = NULL;

    if(!Pgvg)
        return;

    if(!*Pgvg)
        return;

    if(ajDebugTest("ensGvgenotypeDel"))
    {
        ajDebug("ensGvgenotypeDel\n"
                "  *Pgvg %p\n",
                *Pgvg);

        ensGvgenotypeTrace(*Pgvg, 1);
    }

    pthis = *Pgvg;

    pthis->Use--;

    if(pthis->Use)
    {
        *Pgvg = NULL;

        return;
    }

    ajStrDel(&pthis->Allele1);
    ajStrDel(&pthis->Allele2);

    AJFREE(pthis);

    *Pgvg = NULL;

    return;
}




/* @section element retrieval *************************************************
**
** Functions for returning elements of an
** Ensembl Genetic Variation Genotype object.
**
** @fdata [EnsPGvgenotype]
** @fnote None
**
** @nam3rule Get Return Genetic Variation Genotype attribute(s)
** @nam4rule GetAdaptor Return the Ensembl Genetic Variation Genotype Adaptor
** @nam4rule GetIdentifier Return the SQL database-internal identifier
** @nam4rule GetAllele1 Return the allele 1
** @nam4rule GetAllele2 Return the allele 2
**
** @argrule * gvg [const EnsPGvgenotype] Genetic Variation Genotype
**
** @valrule Adaptor [EnsPGvgenotypeadaptor] Ensembl Genetic Variation
**                                          Genotype Adaptor
** @valrule Identifier [ajuint] SQL database-internal identifier
** @valrule Allele1 [AjPStr] Allele 1
** @valrule Allele2 [AjPStr] Allele 2
**
** @fcategory use
******************************************************************************/




/* @func ensGvgenotypeGetAdaptor **********************************************
**
** Get the Ensembl Genetic Variation Genotype Adaptor element of an
** Ensembl Genetic Variation Genotype.
**
** @param [r] gvg [const EnsPGvgenotype] Ensembl Genetic Variation Genotype
**
** @return [EnsPGvgenotypeadaptor] Ensembl Genetic Variation Genotype Adaptor
** @@
******************************************************************************/

EnsPGvgenotypeadaptor ensGvgenotypeGetAdaptor(const EnsPGvgenotype gvg)
{
    if(!gvg)
        return NULL;

    return gvg->Adaptor;
}




/* @func ensGvgenotypeGetIdentifier *******************************************
**
** Get the SQL database-internal identifier element of an
** Ensembl Genetic Variation Genotype.
**
** @param [r] gvg [const EnsPGvgenotype] Ensembl Genetic Variation Genotype
**
** @return [ajuint] Internal database identifier
** @@
******************************************************************************/

ajuint ensGvgenotypeGetIdentifier(const EnsPGvgenotype gvg)
{
    if(!gvg)
        return 0;

    return gvg->Identifier;
}




/* @func ensGvgenotypeGetAllele1 **********************************************
**
** Get the allele 1 element of an Ensembl Genetic Variation Genotype.
**
** @param [r] gvg [const EnsPGvgenotype] Ensembl Genetic Variation Genotype
**
** @return [AjPStr] Allele 1
** @@
******************************************************************************/

AjPStr ensGvgenotypeGetAllele1(const EnsPGvgenotype gvg)
{
    if(!gvg)
        return NULL;

    return gvg->Allele1;
}




/* @func ensGvgenotypeGetAllele2 **********************************************
**
** Get the allele 2 element of an Ensembl Genetic Variation Genotype.
**
** @param [r] gvg [const EnsPGvgenotype] Ensembl Genetic Variation Genotype
**
** @return [AjPStr] Allele 2
** @@
******************************************************************************/

AjPStr ensGvgenotypeGetAllele2(const EnsPGvgenotype gvg)
{
    if(!gvg)
        return NULL;

    return gvg->Allele2;
}




/* @section element assignment ************************************************
**
** Functions for assigning elements of an
** Ensembl Genetic Variation Genotype object.
**
** @fdata [EnsPGvgenotype]
** @fnote None
**
** @nam3rule Set Set one element of a Genetic Variation Genotype
** @nam4rule SetAdaptor Set the Ensembl Genetic Variation Genotype Adaptor
** @nam4rule SetIdentifier Set the SQL database-internal identifier
** @nam4rule SetAllele1 Set the allele 1
** @nam4rule SetAllele2 Set the allele 2
**
** @argrule * gvg [EnsPGvgenotype] Ensembl Genetic Variation Genotype object
**
** @valrule * [AjBool] ajTrue upon success, ajFalse otherwise
**
** @fcategory modify
******************************************************************************/




/* @func ensGvgenotypeSetAdaptor **********************************************
**
** Set the Ensembl Genetic Variation Genotype Adaptor element of an
** Ensembl Genetic Variation Genotype.
**
** @param [u] gvg [EnsPGvvsample] Ensembl Genetic Variation Genotype
** @param [u] gvga [EnsPGvgenotypeadaptor] Ensembl Genetic Variation
**                                         Genotype Adaptor
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
** @@
******************************************************************************/

AjBool ensGvgenotypeSetAdaptor(EnsPGvgenotype gvg, EnsPGvgenotypeadaptor gvga)
{
    if(!gvg)
        return ajFalse;

    gvg->Adaptor = gvga;

    return ajTrue;
}




/* @func ensGvgenotypeSetIdentifier *******************************************
**
** Set the SQL database-internal identifier element of an
** Ensembl Genetic Variation Genotype.
**
** @param [u] gvg [EnsPGvgenotype] Ensembl Genetic Variation Genotype
** @param [r] identifier [ajuint] SQL database-internal identifier
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
** @@
******************************************************************************/

AjBool ensGvgenotypeSetIdentifier(EnsPGvgenotype gvg, ajuint identifier)
{
    if(!gvg)
        return ajFalse;

    gvg->Identifier = identifier;

    return ajTrue;
}




/* @func ensGvgenotypeSetAllele1 **********************************************
**
** Set the allele 1 element of an Ensembl Genetic Variation Genotype.
**
** @param [u] gvg [EnsPGvgenotype] Ensembl Genetic Variation Genotype
** @param [u] allele1 [AjPStr] Allele 1
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
** @@
******************************************************************************/

AjBool ensGvgenotypeSetAllele1(EnsPGvgenotype gvg, AjPStr allele1)
{
    if(!gvg)
        return ajFalse;

    ajStrDel(&gvg->Allele1);

    if(allele1)
        gvg->Allele1 = ajStrNewRef(allele1);

    return ajTrue;
}




/* @func ensGvgenotypeSetAllele2 **********************************************
**
** Set the allele 2 element of an Ensembl Genetic Variation Genotype.
**
** @param [u] gvg [EnsPGvgenotype] Ensembl Genetic Variation Genotype
** @param [u] allele2 [AjPStr] Allele 2
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
** @@
******************************************************************************/

AjBool ensGvgenotypeSetAllele2(EnsPGvgenotype gvg, AjPStr allele2)
{
    if(!gvg)
        return ajFalse;

    ajStrDel(&gvg->Allele2);

    if(allele2)
        gvg->Allele2 = ajStrNewRef(allele2);

    return ajTrue;
}




/* @func ensGvgenotypeGetMemSize **********************************************
**
** Get the memory size in bytes of an Ensembl Genetic Variation Genotype.
**
** @param [r] gvg [const EnsPGvgenotype] Ensembl Genetic Variation Genotype
**
** @return [ajuint] Memory size
** @@
******************************************************************************/

ajuint ensGvgenotypeGetMemSize(const EnsPGvgenotype gvg)
{
    ajuint size = 0;

    if(!gvg)
        return 0;

    size += (ajuint) sizeof (EnsOGvgenotype);

    if(gvg->Allele1)
    {
        size += (ajuint) sizeof (AjOStr);

        size += ajStrGetRes(gvg->Allele1);
    }

    if(gvg->Allele2)
    {
        size += (ajuint) sizeof (AjOStr);

        size += ajStrGetRes(gvg->Allele2);
    }

    return size;
}




/* @section debugging *********************************************************
**
** Functions for reporting of an Ensembl Genetic Variation Genotype object.
**
** @fdata [EnsPGvgenotype]
** @nam3rule Trace Report Ensembl Genetic Variation Genotype elements to
**                 debug file
**
** @argrule Trace gvg [const EnsPGvgenotype] Ensembl Genetic Variation Genotype
** @argrule Trace level [ajuint] Indentation level
**
** @valrule * [AjBool] ajTrue upon success, ajFalse otherwise
**
** @fcategory misc
******************************************************************************/




/* @func ensGvgenotypeTrace ***************************************************
**
** Trace an Ensembl Genetic Variation Genotype.
**
** @param [r] gvg [const EnsPGvgenotype] Ensembl Genetic Variation Genotype
** @param [r] level [ajuint] Indentation level
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
** @@
******************************************************************************/

AjBool ensGvgenotypeTrace(const EnsPGvgenotype gvg, ajuint level)
{
    AjPStr indent = NULL;

    if(!gvg)
        return ajFalse;

    indent = ajStrNew();

    ajStrAppendCountK(&indent, ' ', level * 2);

    ajDebug("%SensGvgenotypeTrace %p\n"
            "%S  Use %u\n"
            "%S  Identifier %u\n"
            "%S  Adaptor %p\n"
            "%S  Allele1 '%S'\n"
            "%S  Allele2 '%S'\n",
            indent, gvg,
            indent, gvg->Use,
            indent, gvg->Identifier,
            indent, gvg->Adaptor,
            indent, gvg->Allele1,
            indent, gvg->Allele2);

    ajStrDel(&indent);

    return ajTrue;
}




/* @datasection [EnsPGvvariation] Genetic Variation Variation *****************
**
** Functions for manipulating Ensembl Genetic Variation Variation objects
**
** @cc Bio::EnsEMBL::Variation::Variation CVS Revision: 1.22
**
** @nam2rule Gvvariation
**
******************************************************************************/

static const char *gvVariationValidationState[] =
{
    "cluster",
    "freq",
    "submitter",
    "doublehit",
    "hapmap",
    "failed",
    "non-polymorphic",
    "observed",
    NULL
};




/* @section constructors ******************************************************
**
** All constructors return a new Ensembl Genetic Variation Variation by
** pointer. It is the responsibility of the user to first destroy any previous
** Genetic Variation Variation. The target pointer does not need to be
** initialised to NULL, but it is good programming practice to do so anyway.
**
** @fdata [EnsPGvvariation]
** @fnote None
**
** @nam3rule New Constructor
** @nam4rule NewObj Constructor with existing object
** @nam4rule NewRef Constructor by incrementing the reference counter
**
** @argrule Obj object [EnsPGvvariation] Ensembl Genetic Variation Variation
** @argrule Ref object [EnsPGvvariation] Ensembl Genetic Variation Variation
**
** @valrule * [EnsPGvvariation] Ensembl Genetic Variation Variation
**
** @fcategory new
******************************************************************************/




/* @func ensGvvariationNew ****************************************************
**
** Default constructor for an Ensembl Genetic Variation Variation.
**
** @cc Bio::EnsEMBL::Storable::new
** @param [u] gvva [EnsPGvvariationadaptor] Ensembl Genetic Variation
**                                          Variation Adaptor
** @param [r] identifier [ajuint] SQL database-internal identifier
** @cc Bio::EnsEMBL::Variation::Variation::new
** @param [u] name [AjPStr] Name
** @param [u] source [AjPStr] Source
** @param [u] ancestralallele [AjPStr] Ancestral allele
** @param [r] synonyms [AjPTable] Synonyms, keys are source databases and
**                                values are AJAX Lists of AJAX Strings of
**                                synonyms
** @param [r] alleles [AjPList] AJAX List of Ensembl Alleles
** @param [r] validationstates [AjPList] AJAX List of validation state
**                                       AJAX Strings
** @param [u] moltype [AjPStr] Molecule type
** @param [u] fiveflank [AjPStr] Five prime flanking sequence
** @param [u] threeflank [AjPStr] Three prime flanking sequence
** @param [u] faileddescription [AjPStr] Failed description
**
** @return [EnsPGvvariation] Ensembl Genetic Variation Variation
** @@
******************************************************************************/

EnsPGvvariation ensGvvariationNew(EnsPGvvariationadaptor gvva,
                                  ajuint identifier,
                                  AjPStr name,
                                  AjPStr source,
                                  AjPStr ancestralallele,
                                  AjPTable synonyms,
                                  AjPList alleles,
                                  AjPList validationstates,
                                  AjPStr moltype,
                                  AjPStr fiveflank,
                                  AjPStr threeflank,
                                  AjPStr faileddescription)
{
    void **keyarray = NULL;
    void **valarray = NULL;

    register ajuint i = 0;

    AjIList iter = NULL;
    AjPList list = NULL;

    AjPStr synonym = NULL;
    AjPStr vstate  = NULL;

    EnsPGvallele gva = NULL;

    EnsPGvvariation gvv = NULL;

    /* FIXME: Input restrictions are missing. */

    AJNEW0(gvv);

    gvv->Use = 1;

    gvv->Identifier = identifier;

    gvv->Adaptor = gvva;

    if(name)
        gvv->Name = ajStrNewRef(name);

    if(source)
        gvv->Source = ajStrNewRef(source);

    if(ancestralallele)
        gvv->AncestralAllele = ajStrNewRef(ancestralallele);

    /*
    ** Copy the AJAX Table of AJAX String key data (source database) and
    ** AJAX List value data. The AJAX List contains AJAX Strings (synonyms).
    */

    if(synonyms)
    {
        gvv->Synonyms = ajTablestrNewLen(0);

        ajTableToarrayKeysValues(synonyms, &keyarray, &valarray);

        for(i = 0; keyarray[i]; i++)
        {
            list = ajListstrNew();

            ajTablePut(gvv->Synonyms,
                       (void *) ajStrNewRef((AjPStr) keyarray[i]),
                       (void *) list);

            iter = ajListIterNew((AjPList) valarray[i]);

            while(!ajListIterDone(iter))
            {
                synonym = (AjPStr) ajListIterGet(iter);

                if(synonym)
                    ajListPushAppend(list, (void *) ajStrNewRef(synonym));
            }

            ajListIterDel(&iter);
        }

        AJFREE(keyarray);
        AJFREE(valarray);
    }

    /* Copy the AJAX List of Ensembl Alleles. */

    gvv->Gvalleles = ajListNew();

    iter = ajListIterNew(alleles);

    while(ajListIterDone(iter))
    {
        gva = (EnsPGvallele) ajListIterGet(iter);

        ajListPushAppend(gvv->Gvalleles, (void *) ensGvalleleNewRef(gva));
    }

    ajListIterDel(&iter);

    /* Copy the AJAX List of validation state AJAX Strings. */

    gvv->ValidationStates = ajListstrNew();

    iter = ajListIterNew(validationstates);

    while(!ajListIterDone(iter))
    {
        vstate = (AjPStr) ajListIterGet(iter);

        if(vstate)
            ajListPushAppend(gvv->ValidationStates,
                             (void *) ajStrNewRef(vstate));
    }

    ajListIterDel(&iter);

    if(moltype)
        gvv->MoleculeType = ajStrNewRef(moltype);

    if(fiveflank)
        gvv->FivePrimeFlank = ajStrNewRef(fiveflank);

    if(threeflank)
        gvv->ThreePrimeFlank = ajStrNewRef(threeflank);

    if(faileddescription)
        gvv->FailedDescription = ajStrNewRef(faileddescription);

    return gvv;
}




/* @func ensGvvariationNewObj *************************************************
**
** Object-based constructor function, which returns an independent object.
**
** @param [r] object [const EnsPGvvariation] Ensembl Genetic Variation
**                                           Variation
**
** @return [EnsPGvvariation] Ensembl Genetic Variation Variation or NULL
** @@
******************************************************************************/

EnsPGvvariation ensGvvariationNewObj(const EnsPGvvariation object)
{
    void **keyarray = NULL;
    void **valarray = NULL;

    register ajuint i = 0;

    AjIList iter = NULL;
    AjPList list = NULL;

    AjPStr synonym = NULL;
    AjPStr vstate = NULL;

    EnsPGvallele gva = NULL;

    EnsPGvvariation gvv = NULL;

    if(!object)
        return NULL;

    AJNEW0(gvv);

    gvv->Use = 1;

    gvv->Identifier = object->Identifier;

    gvv->Adaptor = object->Adaptor;

    if(object->Name)
        gvv->Name = ajStrNewRef(object->Name);

    if(object->Source)
        gvv->Source = ajStrNewRef(object->Source);

    /*
    ** Copy the AJAX Table of AJAX String key data (source database) and
    ** AJAX List value data. The AJAX List contains AJAX Strings (synonyms).
    */

    if(object->Synonyms)
    {
        gvv->Synonyms = ajTablestrNewLen(0);

        ajTableToarrayKeysValues(object->Synonyms, &keyarray, &valarray);

        for(i = 0; keyarray[i]; i++)
        {
            list = ajListstrNew();

            ajTablePut(gvv->Synonyms,
                       (void *) ajStrNewRef((AjPStr) keyarray[i]),
                       (void *) list);

            iter = ajListIterNew((AjPList) valarray[i]);

            while(!ajListIterDone(iter))
            {
                synonym = (AjPStr) ajListIterGet(iter);

                if(synonym)
                    ajListPushAppend(list,
                                     (void *) ajStrNewRef(synonym));
            }

            ajListIterDel(&iter);
        }

        AJFREE(keyarray);
        AJFREE(valarray);
    }

    /*
    ** Copy the AJAX Table of AJAX String key data (synonyms) and
    ** AJAX String value data (handles).
    */

    if(object->Handles)
    {
        gvv->Handles = ajTablestrNewLen(0);

        ajTableToarrayKeysValues(object->Handles, &keyarray, &valarray);

        for(i = 0; keyarray[i]; i++)
            ajTablePut(gvv->Handles,
                       (void *) ajStrNewRef((AjPStr) keyarray[i]),
                       (void *) ajStrNewRef((AjPStr) valarray[i]));

        AJFREE(keyarray);
        AJFREE(valarray);
    }

    if(object->AncestralAllele)
        gvv->AncestralAllele = ajStrNewRef(object->AncestralAllele);

    /* Copy the AJAX List of Ensembl Genetic Variation Alleles. */

    gvv->Gvalleles = ajListNew();

    iter = ajListIterNew(object->Gvalleles);

    while(!ajListIterDone(iter))
    {
        gva = (EnsPGvallele) ajListIterGet(iter);

        ajListPushAppend(gvv->Gvalleles, (void *) ensGvalleleNewRef(gva));
    }

    ajListIterDel(&iter);

    /* Copy the AJAX List of variation state AJAX Strings. */

    /* FIXME: The variation API stores those as a bit score! */

    gvv->ValidationStates = ajListNew();

    iter = ajListIterNew(object->ValidationStates);

    while(!ajListIterDone(iter))
    {
        vstate = (AjPStr) ajListIterGet(iter);

        if(vstate)
            ajListPushAppend(gvv->ValidationStates,
                             (void *) ajStrNewRef(vstate));
    }

    ajListIterDel(&iter);

    if(object->MoleculeType)
        gvv->MoleculeType = ajStrNewRef(object->MoleculeType);

    if(object->FivePrimeFlank)
        gvv->FivePrimeFlank = ajStrNewRef(object->FivePrimeFlank);

    if(object->ThreePrimeFlank)
        gvv->ThreePrimeFlank = ajStrNewRef(object->ThreePrimeFlank);

    if(object->FailedDescription)
        gvv->FailedDescription = ajStrNewRef(object->FailedDescription);

    return gvv;
}




/* @func ensGvvariationNewRef *************************************************
**
** Ensembl Object referencing function, which returns a pointer to the
** Ensembl Object passed in and increases its reference count.
**
** @param [u] gvv [EnsPGvvariation] Ensembl Genetic Variation Variation
**
** @return [EnsPGvvariation] Ensembl Genetic Variation Variation
** @@
******************************************************************************/

EnsPGvvariation ensGvvariationNewRef(EnsPGvvariation gvv)
{
    if(!gvv)
        return NULL;

    gvv->Use++;

    return gvv;
}




/* @section destructors *******************************************************
**
** Destruction destroys all internal data structures and frees the
** memory allocated for the Ensembl Genetic Variation Variation.
**
** @fdata [EnsPGvvariation]
** @fnote None
**
** @nam3rule Del Destroy (free) a Genetic Variation Variation object
**
** @argrule * Pgvv [EnsPGvvariation*] Genetic Variation Variation
**                                    object address
**
** @valrule * [void]
**
** @fcategory delete
******************************************************************************/




/* @func ensGvvariationDel ****************************************************
**
** Default destructor for an Ensembl Genetic Variation Variation.
**
** @param [d] Pgvv [EnsPGvvariation*] Ensembl Genetic Variation
**                                    Variation address
**
** @return [void]
** @@
******************************************************************************/

void ensGvvariationDel(EnsPGvvariation *Pgvv)
{
    void **keyarray = NULL;
    void **valarray = NULL;

    register ajuint i = 0;

    AjPStr synonym = NULL;

    EnsPGvallele gva = NULL;

    EnsPGvvariation pthis = NULL;

    if(!Pgvv)
        return;

    if(!*Pgvv)
        return;

    if(ajDebugTest("ensGvvariationDel"))
    {
        ajDebug("ensGvvariationDel\n"
                "  *Pgvv %p\n",
                *Pgvv);

        ensGvvariationTrace(*Pgvv, 1);
    }

    pthis = *Pgvv;

    pthis->Use--;

    if(pthis->Use)
    {
        *Pgvv = NULL;

        return;
    }

    ajStrDel(&pthis->Name);
    ajStrDel(&pthis->Source);

    /*
    ** Clear the AJAX Table of AJAX String key data (source database) and
    ** AJAX List value data. The AJAX List contains AJAX Strings (synonyms).
    */

    if(pthis->Synonyms)
    {
        ajTableToarrayKeysValues(pthis->Synonyms, &keyarray, &valarray);

        for(i = 0; keyarray[i]; i++)
        {
            ajStrDel((AjPStr *) &keyarray[i]);

            while(ajListPop((AjPList) valarray[i], (void **) &synonym))
                ajStrDel(&synonym);

            ajListFree((AjPList *) &valarray[i]);
        }

        AJFREE(keyarray);
        AJFREE(valarray);
    }

    if(pthis->Handles)
        ajTablestrFree(&pthis->Handles);

    ajStrDel(&pthis->AncestralAllele);

    /* Clear the AJAX List of Ensembl Genetic Variation Alleles. */

    while(ajListPop(pthis->Gvalleles, (void **) &gva))
        ensGvalleleDel(&gva);

    ajListFree(&pthis->Gvalleles);

    /* Clear the AJAX List of validation state AJAX Strings. */

    while(ajListPop(pthis->ValidationStates, (void **) &synonym))
        ajStrDel(&synonym);

    ajListFree(&pthis->ValidationStates);

    ajStrDel(&pthis->MoleculeType);
    ajStrDel(&pthis->FivePrimeFlank);
    ajStrDel(&pthis->ThreePrimeFlank);
    ajStrDel(&pthis->FailedDescription);

    AJFREE(pthis);

    *Pgvv = NULL;

    return;
}




/* @section element retrieval *************************************************
**
** Functions for returning elements of an
** Ensembl Genetic Variation Variation object.
**
** @fdata [EnsPGvvariation]
** @fnote None
**
** @nam3rule Get Return Genetic Variation Variation attribute(s)
** @nam4rule GetAdaptor Return the Ensembl Genetic Variation Variation Adaptor
** @nam4rule GetIdentifier Return the SQL database-internal identifier
** @nam4rule GetName Return the name
** @nam4rule GetSource Return the source
** @nam4rule GetSynoyms Return synonms
** @nam4rule GetAncestralAllele Return the ancestral allele
** @nam4rule GetGvalleles Return Ensembl Genetic Variation Alleles
** @nam4rule GetValidationStates Return the validation states
** @nam4rule GetMoleculeType Return the molecule type
** @nam4rule GetFivePrimeFlank Return the five prime flank
** @nam4rule GetThreePrimeFlank Return the three prime flank
** @nam4rule GetFailedDescription Return the failed description
**
** @argrule * gvv [const EnsPGvvariation] Genetic Variation Variation
**
** @valrule Adaptor [EnsPGvvariationadaptor] Ensembl Genetic Variation
**                                           Variation Adaptor
** @valrule Identifier [ajuint] SQL database-internal identifier
** @valrule Name [AjPStr] Name
** @valrule Source [AjPStr] Source
** @valrule Synonyms [const AjPTable] Synoyms
** @valrule AncestralAllele [AjPStr] Ancestral allele
** @valrule Gvalleles [const AjPList] Ensembl Genetic Variation Alleles
** @valrule ValidationStates [const AjPList] Validation states
** @valrule MoleculeType [AjPStr] Molecule type
** @valrule fiveprimeflank [AjPStr] Five prime flank
** @valrule threeprimeflank [AjPStr] Three prime flank
** @valrule faileddescription [AjPStr] Failed description
**
** @fcategory use
******************************************************************************/




/* @func ensGvvariationGetAdaptor *********************************************
**
** Get the Ensembl Genetic Variation Variation Adaptor element of an
** Ensembl Genetic Variation Variation.
**
** @param [r] gvv [const EnsPGvvariation] Ensembl Genetic Variation Variation
**
** @return [EnsPGvvariationadaptor] Ensembl Genetic Variation
**                                  Variation Adaptor
** @@
******************************************************************************/

EnsPGvvariationadaptor ensGvvariationGetAdaptor(const EnsPGvvariation gvv)
{
    if(!gvv)
        return NULL;

    return gvv->Adaptor;
}




/* @func ensGvvariationGetIdentifier ******************************************
**
** Get the SQL database-internal identifier element of an
** Ensembl Genetic Variation Variation.
**
** @param [r] gvv [const EnsPGvvariation] Ensembl Genetic Variation Variation
**
** @return [ajuint] Internal database identifier
** @@
******************************************************************************/

ajuint ensGvvariationGetIdentifier(const EnsPGvvariation gvv)
{
    if(!gvv)
        return 0;

    return gvv->Identifier;
}




/* @func ensGvvariationGetName ************************************************
**
** Get the name element of an Ensembl Genetic Variation Variation.
**
** @cc Bio::EnsEMBL::Variation::Variation::name
** @param [r] gvv [const EnsPGvvariation] Ensembl Genetic Variation Variation
**
** @return [AjPStr] Name
** @@
******************************************************************************/

AjPStr ensGvvariationGetName(const EnsPGvvariation gvv)
{
    if(!gvv)
        return NULL;

    return gvv->Name;
}




/* @func ensGvvariationGetSource **********************************************
**
** Get the source element of an Ensembl Genetic Variation Variation.
**
** @cc Bio::EnsEMBL::Variation::Variation::source
** @param [r] gvv [const EnsPGvvariation] Ensembl Genetic Variation Variation
**
** @return [AjPStr] Source
** @@
******************************************************************************/

AjPStr ensGvvariationGetSource(const EnsPGvvariation gvv)
{
    if(!gvv)
        return NULL;

    return gvv->Source;
}




/* @func ensGvvariationGetSynonyms ********************************************
**
** Get the synonyms element of an Ensembl Genetic Variation Variation.
** Synonyms are stored in an AJAX Table of source AJAX String key data and
** AJAX List value data. The AJAX Lists contain synonym AJAX Strings.
**
** @param [r] gvv [const EnsPGvvariation] Ensembl Genetic Variation Variation
** @see ensGvvariationFetchAllSynonyms
** @see ensGvvariationFetchAllSynonymSources
**
** @return [const AjPTable] Synonyms
** @@
******************************************************************************/

const AjPTable ensGvvariationGetSynonyms(const EnsPGvvariation gvv)
{
    if(!gvv)
        return NULL;

    return gvv->Synonyms;
}




/* @func ensGvvariationGetAncestralAllele *************************************
**
** Get the ancestral allele element of an Ensembl Genetic Variation Variation.
**
** @cc Bio::EnsEMBL::Variation::Variation::ancestral_allele
** @param [r] gvv [const EnsPGvvariation] Ensembl Genetic Variation Variation
**
** @return [AjPStr] Ancestral allele
** @@
******************************************************************************/

AjPStr ensGvvariationGetAncestralAllele(const EnsPGvvariation gvv)
{
    if(!gvv)
        return NULL;

    return gvv->AncestralAllele;
}




/* @func ensGvvariationGetGvalleles *******************************************
**
** Get all Ensembl Genetic Variation Alleles of an
** Ensembl Genetic Variation Variation.
**
** @cc Bio::EnsEMBL::Variation::Variation::get_all_Alleles
** @param [r] gvv [const EnsPGvvariation] Ensembl Genetic Variation Variation
**
** @return [const AjPList] AJAX List of Ensembl Genetic Variation Alleles
** @@
******************************************************************************/

const AjPList ensGvvariationGetGvalleles(const EnsPGvvariation gvv)
{
    if(!gvv)
        return NULL;

    return gvv->Gvalleles;
}




/* @func ensGvvariationGetValidationStates ************************************
**
** Get all validation states of an
** Ensembl Genetic Variation Variation.
**
** @cc Bio::EnsEMBL::Variation::Variation::get_all_validation_states
** @param [r] gvv [const EnsPGvvariation] Ensembl Genetic Variation Variation
**
** @return [const AjPList] AJAX List of AJAX String validation states
** @@
******************************************************************************/

const AjPList ensGvvariationGetValidationStates(const EnsPGvvariation gvv)
{
    if(!gvv)
        return NULL;

    return gvv->ValidationStates;
}




/* @func ensGvvariationGetMoleculeType ****************************************
**
** Get the molecule type element of an Ensembl Genetic Variation Variation.
**
** @cc Bio::EnsEMBL::Variation::Variation::moltype
** @param [r] gvv [const EnsPGvvariation] Ensembl Genetic Variation Variation
**
** @return [AjPStr] Molecule type
** @@
******************************************************************************/

AjPStr ensGvvariationGetMoleculeType(const EnsPGvvariation gvv)
{
    if(!gvv)
        return NULL;

    return gvv->MoleculeType;
}




/* @func ensGvvariationGetFivePrimeFlank **************************************
**
** Get the five prime flank element of an Ensembl Genetic Variation Variation.
**
** @cc Bio::EnsEMBL::Variation::Variation::five_prime_flanking_seq
** @param [r] gvv [const EnsPGvvariation] Ensembl Genetic Variation Variation
**
** @return [AjPStr] Five prime flank
** @@
******************************************************************************/

AjPStr ensGvvariationGetFivePrimeFlank(const EnsPGvvariation gvv)
{
    if(!gvv)
        return NULL;

    return gvv->FivePrimeFlank;
}




/* @func ensGvvariationGetThreePrimeFlank *************************************
**
** Get the three prime flank element of an Ensembl Genetic Variation Variation.
**
** @cc Bio::EnsEMBL::Variation::Variation::three_prime_flanking_seq
** @param [r] gvv [const EnsPGvvariation] Ensembl Genetic Variation Variation
**
** @return [AjPStr] Three prime flank
** @@
******************************************************************************/

AjPStr ensGvvariationGetThreePrimeFlank(const EnsPGvvariation gvv)
{
    if(!gvv)
        return NULL;

    return gvv->ThreePrimeFlank;
}




/* @func ensGvvariationGetFailedDescription ***********************************
**
** Get the failed description element of an
** Ensembl Genetic Variation Variation.
**
** @cc Bio::EnsEMBL::Variation::Variation::failed_description
** @param [r] gvv [const EnsPGvvariation] Ensembl Genetic Variation Variation
**
** @return [AjPStr] Three prime flank
** @@
******************************************************************************/

AjPStr ensGvvariationGetFailedDescription(const EnsPGvvariation gvv)
{
    if(!gvv)
        return NULL;

    return gvv->FailedDescription;
}




/* @section element assignment ************************************************
**
** Functions for assigning elements of an
** Ensembl Genetic Variation Variation object.
**
** @fdata [EnsPGvvariation]
** @fnote None
**
** @nam3rule Set Set one element of a Genetic Variation Variation
** @nam4rule SetAdaptor Set the Ensembl Genetic Variation Variation Adaptor
** @nam4rule SetIdentifier Set the SQL database-internal identifier
** @nam4rule SetName Set the name
** @nam4rule SetSource Set the source
** @nam4rule SetAncestralAllele Set the ancestral allele
** @nam4rule SetMoleculeType Set the molecule type
** @nam4rule SetFivePrimeFlank Set the five prime flank
** @nam4rule SetThreePrimeFlank Set the three prime flank
** @nam4rule SetFailedDescription Set the failed description
**
** @argrule * gvv [EnsPGvvariation] Ensembl Genetic Variation Variation object
**
** @valrule * [AjBool] ajTrue upon success, ajFalse otherwise
**
** @fcategory modify
******************************************************************************/




/* @func ensGvvariationSetAdaptor *********************************************
**
** Set the Ensembl Genetic Variation Variation Adaptor element of an
** Ensembl Genetic Variation Variation.
**
** @param [u] gvv [EnsPGvvariation] Ensembl Genetic Variation Variation
** @param [r] gvva [EnsPGvvariationadaptor] Ensembl Genetic Variation
**                                          Variation Adaptor
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
** @@
******************************************************************************/

AjBool ensGvvariationSetAdaptor(EnsPGvvariation gvv,
                                EnsPGvvariationadaptor gvva)
{
    if(!gvv)
        return ajFalse;

    gvv->Adaptor = gvva;

    return ajTrue;
}




/* @func ensGvvariationSetIdentifier ******************************************
**
** Set the SQL database-internal identifier element of an
** Ensembl Genetic Variation Variation.
**
** @param [u] gvv [EnsPGvvariation] Ensembl Genetic Variation Variation
** @param [r] identifier [ajuint] SQL database-internal identifier
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
** @@
******************************************************************************/

AjBool ensGvvariationSetIdentifier(EnsPGvvariation gvv, ajuint identifier)
{
    if(!gvv)
        return ajFalse;

    gvv->Identifier = identifier;

    return ajTrue;
}




/* @func ensGvvariationSetName ************************************************
**
** Set the name element of an Ensembl Genetic Variation Variation.
**
** @param [u] gvv [EnsPGvvariation] Ensembl Genetic Variation Variation
** @param [u] name [AjPStr] Name
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
** @@
******************************************************************************/

AjBool ensGvvariationSetName(EnsPGvvariation gvv, AjPStr name)
{
    if(!gvv)
        return ajFalse;

    ajStrDel(&gvv->Name);

    if(name)
        gvv->Name = ajStrNewRef(name);

    return ajTrue;
}




/* @func ensGvvariationSetSource **********************************************
**
** Set the source element of an Ensembl Genetic Variation Variation.
**
** @param [u] gvv [EnsPGvvariation] Ensembl Genetic Variation Variation
** @param [u] source [AjPStr] Source
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
** @@
******************************************************************************/

AjBool ensGvvariationSetSource(EnsPGvvariation gvv, AjPStr source)
{
    if(!gvv)
        return ajFalse;

    ajStrDel(&gvv->Source);

    if(source)
        gvv->Source = ajStrNewRef(source);

    return ajTrue;
}




/* @func ensGvvariationSetAncestralAllele *************************************
**
** Set the ancestral allele element of an Ensembl Genetic Variation Variation.
**
** @param [u] gvv [EnsPGvvariation] Ensembl Genetic Variation Variation
** @param [u] ancestralallele [AjPStr] Ancestral allele
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
** @@
******************************************************************************/

AjBool ensGvvariationSetAncestralAllele(EnsPGvvariation gvv,
                                        AjPStr ancestralallele)
{
    if(!gvv)
        return ajFalse;

    ajStrDel(&gvv->AncestralAllele);

    if(ancestralallele)
        gvv->AncestralAllele = ajStrNewRef(ancestralallele);

    return ajTrue;
}




/* @func ensGvvariationSetMoleculeType ****************************************
**
** Set the molecule type element of an Ensembl Genetic Variation Variation.
**
** @param [u] gvv [EnsPGvvariation] Ensembl Genetic Variation Variation
** @param [u] moltype [AjPStr] Molecule type
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
** @@
******************************************************************************/

AjBool ensGvvariationSetMoleculeType(EnsPGvvariation gvv, AjPStr moltype)
{
    if(!gvv)
        return ajFalse;

    ajStrDel(&gvv->AncestralAllele);

    if(moltype)
        gvv->AncestralAllele = ajStrNewRef(moltype);

    return ajTrue;
}




/* @func ensGvvariationSetFivePrimeFlank **************************************
**
** Set the five prime flank element of an Ensembl Genetic Variation Variation.
**
** @param [u] gvv [EnsPGvvariation] Ensembl Genetic Variation Variation
** @param [u] fiveflank [AjPStr] Five prime flank
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
** @@
******************************************************************************/

AjBool ensGvvariationSetFivePrimeFlank(EnsPGvvariation gvv,
                                       AjPStr fiveflank)
{
    if(!gvv)
        return ajFalse;

    ajStrDel(&gvv->FivePrimeFlank);

    if(fiveflank)
        gvv->FivePrimeFlank = ajStrNewRef(fiveflank);

    return ajTrue;
}




/* @func ensGvvariationSetThreePrimeFlank *************************************
**
** Set the three prime flank element of an Ensembl Genetic Variation Variation.
**
** @param [u] gvv [EnsPGvvariation] Ensembl Genetic Variation Variation
** @param [u] threeflank [AjPStr] Three prime flank
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
** @@
******************************************************************************/

AjBool ensGvvariationSetThreePrimeFlank(EnsPGvvariation gvv,
                                        AjPStr threeflank)
{
    if(!gvv)
        return ajFalse;

    ajStrDel(&gvv->ThreePrimeFlank);

    if(threeflank)
        gvv->ThreePrimeFlank = ajStrNewRef(threeflank);

    return ajTrue;
}




/* @func ensGvvariationSetFailedDescription ***********************************
**
** Set the failed description element of an
** Ensembl Genetic Variation Variation.
**
** @param [u] gvv [EnsPGvvariation] Ensembl Genetic Variation Variation
** @param [u] faileddescription [AjPStr] Failed description
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
** @@
******************************************************************************/

AjBool ensGvvariationSetFailedDescription(EnsPGvvariation gvv,
                                          AjPStr faileddescription)
{
    if(!gvv)
        return ajFalse;

    ajStrDel(&gvv->FailedDescription);

    if(faileddescription)
        gvv->FailedDescription = ajStrNewRef(faileddescription);

    return ajTrue;
}




/* @func ensGvvariationGetMemSize *********************************************
**
** Get the memory size in bytes of an
** Ensembl Genetic Variation Variation.
**
** @param [r] gvv [const EnsPGvvariation] Ensembl Genetic Variation Variation
**
** @return [ajuint] Memory size
** @@
******************************************************************************/

ajuint ensGvvariationGetMemSize(const EnsPGvvariation gvv)
{
    ajuint size = 0;

    if(!gvv)
        return 0;

    size += (ajuint) sizeof (EnsOGvvariation);

    if(gvv->Name)
    {
        size += (ajuint) sizeof (AjOStr);

        size += ajStrGetRes(gvv->Name);
    }

    if(gvv->Source)
    {
        size += (ajuint) sizeof (AjOStr);

        size += ajStrGetRes(gvv->Source);
    }

    /* FIXME: AJAX Table of Synonms is missing! */

    if(gvv->AncestralAllele)
    {
        size += (ajuint) sizeof (AjOStr);

        size += ajStrGetRes(gvv->AncestralAllele);
    }

    /* FIXME: AJAX List of Ensembl Genetic Variation Alleles is missing! */

    /* FIXME: AJAX List of AJAX String validation states is missing! */

    if(gvv->MoleculeType)
    {
        size += (ajuint) sizeof (AjOStr);

        size += ajStrGetRes(gvv->MoleculeType);
    }

    if(gvv->FivePrimeFlank)
    {
        size += (ajuint) sizeof (AjOStr);

        size += ajStrGetRes(gvv->FivePrimeFlank);
    }

    if(gvv->ThreePrimeFlank)
    {
        size += (ajuint) sizeof (AjOStr);

        size += ajStrGetRes(gvv->ThreePrimeFlank);
    }

    if(gvv->FailedDescription)
    {
        size += (ajuint) sizeof (AjOStr);

        size += ajStrGetRes(gvv->FailedDescription);
    }

    return size;
}




/* @section debugging *********************************************************
**
** Functions for reporting of an Ensembl Genetic Variation Variation object.
**
** @fdata [EnsPGvvariation]
** @nam3rule Trace Report Ensembl Genetic Variation Variation elements
**                 to debug file
**
** @argrule Trace gvv [const EnsPGvvariation] Ensembl Genetic Variation
**                                            Variation
** @argrule Trace level [ajuint] Indentation level
**
** @valrule * [AjBool] ajTrue upon success, ajFalse otherwise
**
** @fcategory misc
******************************************************************************/




/* @func ensGvvariationTrace **************************************************
**
** Trace an Ensembl Genetic Variation Variation.
**
** @param [r] gvv [const EnsPGvvariation] Ensembl Genetic Variation Variation
** @param [r] level [ajuint] Indentation level
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
** @@
******************************************************************************/

AjBool ensGvvariationTrace(const EnsPGvvariation gvv, ajuint level)
{
    AjPStr indent = NULL;

    if(!gvv)
        return ajFalse;

    indent = ajStrNew();

    ajStrAppendCountK(&indent, ' ', level * 2);

    ajDebug("ensGvvariationTrace %p\n"
            "%S  Use %u\n"
            "%S  Identifier %u\n"
            "%S  Adaptor %p\n"
            "%S  Name '%S'\n"
            "%S  Source '%S'\n"
            "%S  Synonyms %p\n"
            "%S  AncestralAllele '%S'\n"
            "%S  Gvalleles %p\n"
            "%S  ValidationStates %p\n"
            "%S  MoleculeType '%S'\n"
            "%S  FivePrimeFlank '%S'\n"
            "%S  ThreePrimeFlank '%S'\n"
            "%S  FailedDescription '%S'\n",
            indent, gvv,
            indent, gvv->Use,
            indent, gvv->Identifier,
            indent, gvv->Adaptor,
            indent, gvv->Name,
            indent, gvv->Source,
            indent, gvv->Synonyms,
            indent, gvv->AncestralAllele,
            indent, gvv->Gvalleles,
            indent, gvv->ValidationStates,
            indent, gvv->MoleculeType,
            indent, gvv->FivePrimeFlank,
            indent, gvv->ThreePrimeFlank,
            indent, gvv->FailedDescription);

    ajStrDel(&indent);

    /* FIXME: AJAX Table of Synonms is missing! */

    /* FIXME: AJAX List of Ensembl Genetic Variation Alleles is missing! */

    /* FIXME: AJAX List of AJAX String validation states is missing! */

    return ajTrue;
}




/* @func ensGvvariationAddSynonym *********************************************
**
** Add a synonym to an Ensembl Genetic Variation Variation.
**
** @cc Bio::EnsEMBL::Variation::Variation::add_synonym
** @param [u] gvv [EnsPGvvariation] Ensembl Genetic Variation Variation
** @param [u] source [AjPStr] Source database
** @param [u] synonym [AjPStr] Synonym
** @param [uN] handle [AjPStr] Handle or submitter identifier
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
** @@
******************************************************************************/

AjBool ensGvvariationAddSynonym(EnsPGvvariation gvv,
                                AjPStr source,
                                AjPStr synonym,
                                AjPStr handle)
{
    AjPList list = NULL;

    if(!gvv)
        return ajFalse;

    if(!source)
        return ajFalse;

    if(!synonym)
        return ajFalse;

    if(!gvv->Synonyms)
        gvv->Synonyms = ajTablestrNewLen(0);

    list = (AjPList) ajTableFetch(gvv->Synonyms, (const void *) source);

    if(!list)
    {
        list = ajListstrNew();

        ajTablePut(gvv->Synonyms,
                   (void *) ajStrNewRef(source),
                   (void *) list);
    }

    ajListPushAppend(list, (void *) ajStrNewRef(synonym));

    if(handle && ajStrGetLen(handle))
    {
        if(!gvv->Handles)
            gvv->Handles = ajTablestrNewLen(0);

        if(!ajTablestrFetch(gvv->Handles, synonym))
            ajTablePut(gvv->Handles,
                       (void *) ajStrNewRef(synonym),
                       (void *) ajStrNewRef(handle));
    }

    return ajTrue;
}




/* @func ensGvvariationAddGvallele ********************************************
**
** Add an Ensembl Genetic Variation Allele to an
** Ensembl Genetic Variation Variation.
**
** @cc Bio::EnsEMBL::Variation::Variation::add_Allele
** @param [u] gvv [EnsPGvvariation] Ensembl Genetic Variation Variation
** @param [u] gva [EnsPGvallele] Ensembl Genetic Variation Allele
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
** @@
******************************************************************************/

AjBool ensGvvariationAddGvallele(EnsPGvvariation gvv, EnsPGvallele gva)
{
    if(!gvv)
        return ajFalse;

    if(!gva)
        return ajFalse;

    ajListPushAppend(gvv->Gvalleles, (void *) ensGvalleleNewRef(gva));

    return ajTrue;
}




/* @func ensGvvariationValidationStateFromStr *********************************
**
** Convert an AJAX String into an Ensembl Genetic Variaton Variation
** validation state element.
**
** @param [r] state [const AjPStr] Validation state string
**
** @return [AjEnum] Ensembl Genetic Variation validation state element or
**                  ensEGvvariationValidationStateNULL
** @@
******************************************************************************/

AjEnum ensGvvariationValidationStateFromStr(const AjPStr state)
{
    register ajint i = 0;

    AjEnum estate = ensEGvvariationValidationStateNULL;

    for(i = 1; gvVariationValidationState[i]; i++)
        if(ajStrMatchC(state, gvVariationValidationState[i]))
            estate = i;

    if(!estate)
        ajDebug("ensGvvariationValidationStateFromStr encountered "
                "unexpected string '%S'.\n", state);

    return estate;
}




/* @func ensGvvariationValidationStateToChar **********************************
**
** Convert an Ensembl Genetic Variation Variation validation state element
** into a C-type (char*) string.
**
** @param [r] state [const AjEnum] Ensembl Genetic Variation Variation
**                                 validation state enumerator
**
** @return [const char*] Ensembl Genetic Variation Variation validation state
**                       C-type (char*) string
** @@
******************************************************************************/

const char* ensGvvariationValidationStateToChar(const AjEnum state)
{
    register ajint i = 0;

    if(!state)
        return NULL;

    for(i = 1; gvVariationValidationState[i] && (i < state); i++);

    if(!gvVariationValidationState[i])
        ajDebug("ensGvvariationValidationStateToChar encountered an "
                "out of boundary error on gender %d.\n", state);

    return gvVariationValidationState[i];
}




/* @func ensGvvariationFetchAllSynonyms ***************************************
**
** Fetch all synonyms of an Ensembl Genetic Variation Variation.
**
** @param [r] gvv [const EnsPGvvariation] Ensembl Genetic Variation Variation
** @param [r] source [const AjPStr] Source
** @param [u] synonyms [AjPList] AJAX List of synonym AJAX Strings
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
** @@
******************************************************************************/

AjBool ensGvvariationFetchAllSynonyms(const EnsPGvvariation gvv,
                                      const AjPStr source,
                                      AjPList synonyms)
{
    void **keyarray = NULL;
    void **valarray = NULL;

    register ajuint i = 0;

    AjIList iter = NULL;
    AjPList list = NULL;

    AjPStr synonym = NULL;

    if(!gvv)
        return ajFalse;

    if(!synonyms)
        return ajFalse;

    if(gvv->Synonyms)
    {
        if(source)
        {
            list = (AjPList)
                ajTableFetch(gvv->Synonyms, (const void *) source);

            iter = ajListIterNew(list);

            while(!ajListIterDone(iter))
            {
                synonym = (AjPStr) ajListIterGet(iter);

                if(synonym)
                    ajListPushAppend(synonyms,
                                     (void *) ajStrNewRef(synonym));
            }
        }
        else
        {
            ajTableToarrayKeysValues(gvv->Synonyms, &keyarray, &valarray);

            for(i = 0; keyarray[i]; i++)
            {
                iter = ajListIterNew((AjPList) valarray[i]);

                while(!ajListIterDone(iter))
                {
                    synonym = (AjPStr) ajListIterGet(iter);

                    if(synonym)
                        ajListPushAppend(synonyms,
                                         (void *) ajStrNewRef(synonym));
                }

                ajListIterDel(&iter);
            }

            AJFREE(keyarray);

            AJFREE(valarray);
        }
    }

    return ajTrue;
}




/* @func ensGvvariationFetchAllSynonymSources *********************************
**
** Fetch all synonym sources of an Ensembl Genetic Variation Variation.
**
** @param [r] gvv [const EnsPGvvariation] Ensembl Genetic Variation Variation
** @param [u] sources [AjPList] AJAX List of source AJAX Strings
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
** @@
******************************************************************************/

AjBool ensGvvariationFetchAllSynonymSources(const EnsPGvvariation gvv,
                                            AjPList sources)
{
    void **keyarray = NULL;
    void **valarray = NULL;

    register ajuint i = 0;

    if(!gvv)
        return ajFalse;

    if(!sources)
        return ajFalse;

    if(gvv->Synonyms)
    {
        ajTableToarrayKeysValues(gvv->Synonyms, &keyarray, &valarray);

        for(i = 0; keyarray[i]; i++)
            ajListPushAppend(sources,
                             (void *) ajStrNewRef((AjPStr) keyarray[i]));

        AJFREE(keyarray);
        AJFREE(valarray);
    }

    return ajTrue;
}




/* @func ensGvvariationFetchHandleBySynonym ***********************************
**
** Fetch a handle by a synonym of an Ensembl Genetic Variation Variation.
**
** @param [r] gvv [EnsPGvvariation] Ensembl Genetic Variation Variation
** @param [r] synonym [const AjPStr] Synonym
** @param [u] Phandle [AjPStr *] Handle AJAX String address
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
** @@
******************************************************************************/

AjBool
ensGvvariationFetchHandleBySynonym(EnsPGvvariation gvv,
                                   const AjPStr synonym,
                                   AjPStr *Phandle)
{
    AjPStr *Pstring = NULL;

    if(!gvv)
        return ajFalse;

    if(!synonym)
        return ajFalse;

    if(!Phandle)
        return ajFalse;

    if(gvv->Handles)
    {
        Pstring = ajTablestrFetchmod(gvv->Handles, synonym);

        if(Pstring)
        {
            ajStrAssignS(Phandle, *Pstring);

            return ajTrue;
        }
        else
            return ajFalse;
    }
    else
        return ajFalse;
}




/* @datasection [EnsPGvvariationadaptor] Genetic Variation Variation Adaptor
**
** Functions for manipulating Ensembl Genetic Variation Variation Adaptor
** objects
**
** @cc Bio::EnsEMBL::Variation::DBSQL::VariationAdaptor CVS Revision: 1.35
**
** @nam2rule Gvvariationadaptor
**
******************************************************************************/




/* @funcstatic gvVariationadaptorClearAlleles *********************************
**
** An ajTableMapDel 'apply' function to clear the Ensembl Genetic Variation
** Variation Adaptor-internal Genetic Variation Alleles cache. This function
** deletes the unsigned identifier key data and the Ensembl Genetic Variation
** Alleles value data.
**
** @param [u] key [void**] AJAX unsigned integer key data address
** @param [u] value [void**] Ensembl Genetic Variation Allele
**                            value data address
** @param [u] cl [void*] Standard, passed in from ajTableMapDel
** @see ajTableMapDel
**
** @return [void]
** @@
******************************************************************************/

static void gvVariationadaptorClearAlleles(void **key, void **value, void *cl)
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

    ensGvalleleDel((EnsPGvallele *) value);

    return;
}




/* @funcstatic gvVariationadaptorClearPopulations *****************************
**
** An ajTableMapDel 'apply' function to clear the Ensembl Genetic Variation
** Variation Adaptor-internal Genetic Variation Population cache. This function
** deletes the unsigned identifier key data and the Ensembl Genetic Variation
** Population value data.
**
** @param [u] key [void**] AJAX unsigned integer key data address
** @param [u] value [void**] Ensembl Genetic Variation Population
**                            value data address
** @param [u] cl [void*] Standard, passed in from ajTableMapDel
** @see ajTableMapDel
**
** @return [void]
** @@
******************************************************************************/

static void gvVariationadaptorClearPopulations(void **key, void **value,
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

    ensGvpopulationDel((EnsPGvpopulation *) value);

    return;
}




/* @funcstatic gvVariationadaptorClearSynonyms ********************************
**
** An ajTableMapDel 'apply' function to clear the Ensembl Genetic Variation
** Variation Adaptor-internal synonym cache. This function deletes the
** AJAX String key data and the unsigned integer value data.
**
** @param [u] key [void**] AJAX String key data address
** @param [u] value [void**] AJAX unsigned integer value data address
** @param [u] cl [void*] Standard, passed in from ajTableMapDel
** @see ajTableMapDel
**
** @return [void]
** @@
******************************************************************************/

static void gvVariationadaptorClearSynonyms(void **key, void **value, void *cl)
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

    ajStrDel((AjPStr *) value);

    AJFREE(*value);

    return;
}




/* @funcstatic gvVariationadaptorFetchAllBySQL ********************************
**
** Fetch all Ensembl Genetic Variation Variation objects via an SQL statement.
**
** @param [r] gvva [EnsPGvvariationadaptor] Ensembl Genetic Variation
**                                          Variation Adaptor
** @param [r] statement [const AjPStr] SQL statement
** @param [u] gvvs [AjPList] AJAX List of Ensembl Genetic Variation
**                           Variation objects
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
** @@
******************************************************************************/

static AjBool gvVariationadaptorFetchAllBySQL(EnsPGvvariationadaptor gvva,
                                              const AjPStr statement,
                                              AjPList gvvs)
{
    float allelefreq = 0;

    ajuint identifier     = 0;
    ajuint alleleid       = 0;
    ajuint allelesampleid = 0;
    ajuint allelessid     = 0;
    ajuint cidentifier    = 0;
    ajuint calleleid      = 0;

    ajuint *Pidentifier = NULL;

    AjPList vlist = NULL;

    AjPSqlstatement sqls = NULL;
    AjISqlrow sqli       = NULL;
    AjPSqlrow sqlr       = NULL;

    AjPStr name            = NULL;
    AjPStr vstatus         = NULL;
    AjPStr source          = NULL;
    AjPStr ancestralallele = NULL;
    AjPStr allelestr       = NULL;
    AjPStr moltype         = NULL;
    AjPStr synname         = NULL;
    AjPStr synhandle       = NULL;
    AjPStr synsource       = NULL;
    AjPStr failed          = NULL;
    AjPStr key             = NULL;
    AjPStr vtoken          = NULL;

    AjPStrTok token = NULL;

    AjPTable alleles     = NULL;
    AjPTable populations = NULL;
    AjPTable synonyms    = NULL;

    EnsPGvallele gva         = NULL;
    EnsPGvalleleadaptor gvaa = NULL;

    EnsPGvpopulation gvp         = NULL;
    EnsPGvpopulationadaptor gvpa = NULL;

    EnsPGvvariation gvv = NULL;

    if(!gvva)
        return ajFalse;

    if(!statement)
        return ajFalse;

    if(!gvvs)
        return ajFalse;

    alleles = MENSTABLEUINTNEW(0);

    populations = MENSTABLEUINTNEW(0);

    synonyms = ajTablestrNewLen(0);

    gvpa = ensRegistryGetGvpopulationadaptor(gvva);

    sqls = ensDatabaseadaptorSqlstatementNew(gvva, statement);

    sqli = ajSqlrowiterNew(sqls);

    while(!ajSqlrowiterDone(sqli))
    {
        identifier     = 0;
        name           = ajStrNew();
        vstatus        = ajStrNew();
        vtoken         = ajStrNew();
        source         = ajStrNew();
        alleleid       = 0;
        allelessid     = 0;
        allelestr      = ajStrNew();
        allelefreq     = 0;
        allelesampleid = 0;
        moltype        = ajStrNew();
        synname        = ajStrNew();
        synhandle      = ajStrNew();
        synsource      = ajStrNew();
        failed         = ajStrNew();

        sqlr = ajSqlrowiterGet(sqli);

        ajSqlcolumnToUint(sqlr, &identifier);
        ajSqlcolumnToStr(sqlr, &name);
        ajSqlcolumnToStr(sqlr, &vstatus);
        ajSqlcolumnToStr(sqlr, &source);
        ajSqlcolumnToStr(sqlr, &ancestralallele);
        ajSqlcolumnToUint(sqlr, &alleleid);
        ajSqlcolumnToUint(sqlr, &allelessid);
        ajSqlcolumnToStr(sqlr, &allelestr);
        ajSqlcolumnToFloat(sqlr, &allelefreq);
        ajSqlcolumnToUint(sqlr, &allelesampleid);
        ajSqlcolumnToStr(sqlr, &moltype);
        ajSqlcolumnToStr(sqlr, &synname);
        ajSqlcolumnToStr(sqlr, &synhandle);
        ajSqlcolumnToStr(sqlr, &synsource);
        ajSqlcolumnToStr(sqlr, &failed);

        if(cidentifier != identifier)
        {
            /* Split the comma-separated list of validation states. */

            token = ajStrTokenNewC(vstatus, ",");

            vlist = ajListNew();

            while(ajStrTokenNextParse(&token, &vtoken))
                ajListPushAppend(vlist, (void *) ajStrNewS(vtoken));

            if(ajListGetLength(vlist) == 0)
                ajListPushAppend(vlist, (void *) ajStrNew());

            gvv = ensGvvariationNew(gvva,
                                    identifier,
                                    name,
                                    source,
                                    ancestralallele,
                                    (AjPTable) NULL, /* synonyms */
                                    (AjPList) NULL, /* alleles */
                                    vlist,
                                    moltype,
                                    (AjPStr) NULL, /* fiveflank */
                                    (AjPStr) NULL, /* threeflank */
                                    failed);

            ajListPushAppend(gvvs, (void *) gvv);

            /* Clear the AJAX List of validation states. */

            while(ajListPop(vlist, (void **) &vtoken))
                ajStrDel(&vtoken);

            ajListFree(&vlist);

            cidentifier = identifier;
        }

        if(calleleid != alleleid)
        {
            if(allelesampleid)
            {
                gvp = (EnsPGvpopulation)
                    ajTableFetch(populations, (void *) &allelesampleid);

                if(!gvp)
                {
                    ensGvpopulationadaptorFetchByIdentifier(gvpa,
                                                            allelesampleid,
                                                            &gvp);

                    if(gvp)
                    {
                        AJNEW0(Pidentifier);

                        ajTablePut(populations,
                                   (void *) Pidentifier,
                                   (void *) gvp);
                    }
                }
            }
            else
                gvp = (EnsPGvpopulation) NULL;

            if(alleleid)
            {
                gva = ensGvalleleNew(gvaa,
                                     alleleid,
                                     gvp,
                                     allelestr,
                                     allelefreq,
                                     allelessid);

                ensGvvariationAddGvallele(gvv, gva);

                ensGvalleleDel(&gva);

                calleleid = alleleid;
            }

            if(ajStrGetLen(synsource))
            {
                key = ajFmtStr("%S:%S", synsource, synname);

                Pidentifier = (ajuint *)
                    ajTableFetch(synonyms, (const void *) key);

                if(Pidentifier)
                    ajStrDel(&key);
                else
                {
                    AJNEW0(Pidentifier);

                    ajTablePut(synonyms, (void *) key, (void *) Pidentifier);

                    ensGvvariationAddSynonym(gvv,
                                             synsource,
                                             synname,
                                             synhandle);
                }
            }
        }

        ajStrDel(&name);
        ajStrDel(&vstatus);
        ajStrDel(&source);
        ajStrDel(&ancestralallele);
        ajStrDel(&allelestr);
        ajStrDel(&moltype);
        ajStrDel(&synname);
        ajStrDel(&synhandle);
        ajStrDel(&synsource);
        ajStrDel(&failed);
    }

    ajSqlrowiterDel(&sqli);

    ajSqlstatementDel(&sqls);

    /* Clear the AJAX Table of Ensembl Genetic Variation Populations. */

    ajTableMapDel(populations, gvVariationadaptorClearPopulations, NULL);

    ajTableFree(&populations);

    /* Clear the AJAX Table of Ensembl Genetic Variation Alleles. */

    ajTableMapDel(alleles, gvVariationadaptorClearAlleles, NULL);

    ajTableFree(&alleles);

    /* Clear the AJAX Table of Synonyms. */

    ajTableMapDel(synonyms, gvVariationadaptorClearSynonyms, NULL);

    ajTableFree(&synonyms);

    return ajTrue;
}




/* @func ensGvvariationadaptorFetchByIdentifier *******************************
**
** Fetch an Ensembl Genetic Variation Variation via its
** SQL database-internal identifier.
** The caller is responsible for deleting the
** Ensembl Genetic Variation Variation.
**
** @cc Bio::EnsEMBL::Variation::DBSQL::Variationadaptor::fetch_by_dbID
** @param [u] gvva [EnsPGvvariationadaptor] Ensembl Genetic Variation
**                                          Variation Adaptor
** @param [r] identifier [ajuint] SQL database-internal identifier
** @param [wP] Pgvv [EnsPGvvariation*] Ensembl Genetic Variation
**                                     Variation address
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
** @@
******************************************************************************/

AjBool ensGvvariationadaptorFetchByIdentifier(EnsPGvvariationadaptor gvva,
                                              ajuint identifier,
                                              EnsPGvvariation *Pgvv)
{
    AjPList variations = NULL;

    AjPStr statement = NULL;

    EnsPGvvariation gvv = NULL;

    if(!gvva)
        return ajFalse;

    if(!identifier)
        return ajFalse;

    if(!Pgvv)
        return ajFalse;

    statement = ajFmtStr(
        "SELECT "
        "variation.variation_id, "
        "variation.name, "
        "variation.validation_status, "
        "source1.name, "
        "variation.ancestral_allele, "
        "allele.allele_id, "
        "allele.subsnp_id, "
        "allele.allele, "
        "allele.frequency, "
        "allele.sample_id, "
        "variation_synonym.moltype, "
        "variation_synonym.name, "
        "subsnp_handle.handle, "
        "source2.name, "
        "failed_description.description "
        "FROM "
        "(variation, source source1) "
        "LEFT JOIN "
        "allele "
        "ON "
        "variation.variation_id = allele.variation_id "
        "LEFT JOIN "
        "variation_synonym "
        "ON "
        "variation.variation_id = "
        "variation_synonym.variation_id "
        "LEFT JOIN "
        "source source2 "
        "ON "
        "variation_synonym.source_id = source2.source_id "
        "LEFT JOIN "
        "failed_variation "
        "ON "
        "variation.variation_id = "
        "failed_variation.variation_id "
        "LEFT JOIN "
        "failed_description "
        "ON "
        "failed_variation.failed_description_id = "
        "failed_description.failed_description_id "
        "LEFT JOIN "
        "subsnp_handle "
        "ON "
        "variation_synonym.subsnp_id = "
        "subsnp_handle.subsnp_id "
        "WHERE "
        "variation.source_id = source1.source_id "
        "AND "
        "variation.variation_id = %u",
        identifier);

    variations = ajListNew();

    gvVariationadaptorFetchAllBySQL(gvva, statement, variations);

    if(ajListGetLength(variations) > 1)
        ajWarn("ensGvvariationadaptorFetchByIdentifier go more than one "
               "Ensembl Genetic Variation Variation for identifier %u.",
               identifier);

    ajListPop(variations, (void **) Pgvv);

    while(ajListPop(variations, (void *) &gvv))
        ensGvvariationDel(&gvv);

    ajListFree(&variations);

    ajStrDel(&statement);

    return ajTrue;
}




/* @func ensGvvariationadaptorFetchByName *************************************
**
** Fetch an Ensembl Genetic Variation Variation via its name.
** The caller is responsible for deleting the
** Ensembl Genetic Variation Variation.
**
** @cc Bio::EnsEMBL::Variation::DBSQL::VariationAdaptor::fetch_by_name
** @param [u] gvva [EnsPGvvariationadaptor] Ensembl Genetic Variation
**                                          Variation Adaptor
** @param [r] name [const AjPStr] Name
** @param [rN] source [const AjPStr] Source
** @param [wP] Pgvv [EnsPGvvariation*] Ensembl Genetic Variation
**                                     Variation address
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
** @@
******************************************************************************/

AjBool ensGvvariationadaptorFetchByName(EnsPGvvariationadaptor gvva,
                                        const AjPStr name,
                                        const AjPStr source,
                                        EnsPGvvariation *Pgvv)
{
    char *Ptxtname   = NULL;
    char *Ptxtsource = NULL;

    AjPList gvvs = NULL;

    AjPStr extra     = NULL;
    AjPStr statement = NULL;

    EnsPGvvariation gvv = NULL;

    if(!gvva)
        return ajFalse;

    if(!name)
        return ajFalse;

    if(!Pgvv)
        return ajFalse;

    ensDatabaseadaptorEscapeC(gvva, &Ptxtname, name);

    if(source && ajStrGetLen(source))
    {
        ensDatabaseadaptorEscapeC(gvva, &Ptxtsource, source);

        extra = ajFmtStr("AND source1.name = '%s' ", Ptxtsource);

        ajCharDel(&Ptxtsource);
    }
    else
        extra = ajStrNew();

    statement = ajFmtStr(
        "SELECT "
        "variation.variation_id, "
        "variation.name, "
        "variation.validation_status, "
        "source1.name, "
        "variation.ancestral_allele, "
        "allele.allele_id, "
        "allele.subsnp_id, "
        "allele.allele, "
        "allele.frequency, "
        "allele.sample_id, "
        "variation_synonym.moltype, "
        "variation_synonym.name, "
        "subsnp_handle.handle, "
        "source2.name, "
        "failed_description.description "
        /*
          "FROM "
          "variation, "
          "source source1, "
          "source source2, "
          "allele, "
          "variation_synonym "
        */
        "FROM "
        "(variation, source source1) "
        "LEFT JOIN "
        "allele "
        "ON "
        "variation.variation_id = allele.variation_id "
        "LEFT JOIN "
        "variation_synonym "
        "ON "
        "variation.variation_id = "
        "variation_synonym.variation_id "
        "LEFT JOIN "
        "source source2 "
        "ON "
        "variation_synonym.source_id = source2.source_id "
        "LEFT JOIN "
        "failed_variation "
        "ON "
        "variation.variation_id = "
        "failed_variation.variation_id "
        "LEFT JOIN "
        "failed_description "
        "ON "
        "failed_variation.failed_description_id = "
        "failed_description.failed_description_id "
        "LEFT JOIN "
        "subsnp_handle "
        "ON "
        "variation_synonym.subsnp_id = "
        "subsnp_handle.subsnp_id "
        /*
          "WHERE "
          "variation.variation_id = "
          "allele.variation_id "
        */
        /*
          "AND "
          "variation.variation_id = "
          "variation_synonym.variation_id "
        */
        "WHERE "
        "variation.source_id = source1.source_id "
        /*
          "AND "
          "variation_synonym.source_id = "
          "source2.source_id "
        */
        "AND "
        "variation.name = '%s' "
        "%S "
        "ORDER BY "
        "allele.allele_id",
        Ptxtname,
        extra);

    gvvs = ajListNew();

    gvVariationadaptorFetchAllBySQL(gvva, statement, gvvs);

    ajStrDel(&statement);

    if(ajListGetLength(gvvs) > 1)
        ajWarn("ensGvvariationadaptorFetchByIdentifier go more than one "
               "Ensembl Genetic Variation Variation for name '%S' and "
               "source '%S'.", name, source);

    if(ajListGetLength(gvvs) == 0)
    {
        statement = ajFmtStr(
            "SELECT "
            "variation.variation_id, "
            "variation.name, "
            "variation.validation_status, "
            "source1.name, "
            "variation.ancestral_allele, "
            "allele.allele_id, "
            "allele.subsnp_id, "
            "allele.allele, "
            "allele.frequency, "
            "allele.sample_id, "
            "variation_synonym1.moltype, "
            "variation_synonym2.name, "
            "subsnp_handle.handle, "
            "source2.name, "
            "NULL "
            "FROM "
            "("
            "variation, "
            "source source1, "
            "source source2, "
            "allele, "
            "variation_synonym variation_synonym1, "
            "variation_synonym variation_synonym2"
            ") "
            "LEFT JOIN "
            "subsnp_handle "
            "ON "
            "variation_synonym2.subsnp_id = "
            "subsnp_handle.subsnp_id "
            "WHERE "
            "variation.variation_id = allele.variation_id "
            "AND "
            "variation.variation_id = "
            "variation_synonym1.variation_id "
            "AND "
            "variation.variation_id = "
            "variation_synonym2.variation_id "
            "AND "
            "variation.source_id = "
            "source1.source_id "
            "AND "
            "variation_synonym2.source_id = "
            "source2.source_id "
            "AND "
            "variation_synonym1.name = '%s' "
            "%S "
            "ORDER BY "
            "allele.allele_id",
            Ptxtname,
            extra);

        gvVariationadaptorFetchAllBySQL(gvva, statement, gvvs);

        ajStrDel(&statement);
    }

    if(ajListGetLength(gvvs) > 1)
        ajWarn("ensGvvariationadaptorFetchByIdentifier go more than one "
               "Ensembl Genetic Variation Variation for name '%S' and "
               "source '%S'.", name, source);

    ajListPop(gvvs, (void **) Pgvv);

    while(ajListPop(gvvs, (void *) &gvv))
        ensGvvariationDel(&gvv);

    ajListFree(&gvvs);

    ajCharDel(&Ptxtname);

    ajStrDel(&extra);

    return ajTrue;
}




/* @func ensGvvariationadaptorFetchAllBySource ********************************
**
** Fetch all Ensembl Genetic Variation Variations via a source.
** The caller is responsible for deleting the
** Ensembl Genetic Variation Variations before deleting the AJAX List.
**
** @cc Bio::EnsEMBL::Variation::DBSQL::VariationAdaptor::fetch_all_by_source
** @param [u] gvva [const EnsPGvvariationadaptor] Ensembl Genetic Variation
**                                                Variation Adaptor
** @param [r] source [const AjPStr] Source
** @param [r] primary [AjBool] Primary
** @param [u] gvvs [AjPList] AJAX List of Ensembl Genetic Variation Variations
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
** @@
******************************************************************************/

AjBool ensGvvariationadaptorFetchAllBySource(EnsPGvvariationadaptor gvva,
                                             const AjPStr source,
                                             AjBool primary,
                                             AjPList gvvs)
{
    char *Ptxtsource = NULL;

    AjPStr statement = NULL;

    if(!gvva)
        return ajFalse;

    if(!source)
        return ajFalse;

    if(!gvvs)
        return ajFalse;

    ensDatabaseadaptorEscapeC(gvva, &Ptxtsource, source);

    statement = ajFmtStr(
        "SELECT "
        "variation.variation_id, "
        "variation.name, "
        "variation.validation_status, "
        "source1.name, "
        "variation.ancestral_allele, "
        "allele.allele_id, "
        "allele.subsnp_id, "
        "allele.allele, "
        "allele.frequency, "
        "allele.sample_id, "
        "variation_synonym.moltype, "
        "variation_synonym.name, "
        "subsnp_handle.handle, "
        "source2.name, "
        "failed_description.description "
        "FROM "
        "(variation, source source1) "
        "LEFT JOIN "
        "allele "
        "ON "
        "variation.variation_id = "
        "allele.variation_id "
        "LEFT JOIN "
        "variation_synonym "
        "ON "
        "variation.variation_id = "
        "variation_synonym.variation_id "
        "LEFT JOIN "
        "source source2 "
        "ON "
        "variation_synonym.source_id = "
        "source2.source_id "
        "LEFT JOIN "
        "failed_variation "
        "ON "
        "variation.variation_id = "
        "failed_variation.variation_id "
        "LEFT JOIN "
        "failed_description "
        "ON "
        "failed_variation.failed_description_id = "
        "failed_description.failed_description_id "
        "LEFT JOIN "
        "subsnp_handle "
        "ON "
        "variation_synonym.subsnp_id = "
        "subsnp_handle.subsnp_id "
        "WHERE "
        "variation.source_id = "
        "source1.source_id "
        "AND "
        "source1.name = '%s'",
        Ptxtsource);

    gvVariationadaptorFetchAllBySQL(gvva, statement, gvvs);

    ajStrDel(&statement);

    if(!primary)
    {
        /*
        ** We need to include variation_synonym as well,
        ** where the variation was merged with NCBI dbSNP.
        */

        statement = ajFmtStr(
            "SELECT "
            "variation.variation_id, "
            "variation.name, "
            "variation.validation_status, "
            "source1.name, "
            "variation.ancestral_allele, "
            "allele.allele_id, "
            "allele.subsnp_id, "
            "allele.allele, "
            "allele.frequency, "
            "allele.sample_id, "
            "variation_synonym1.moltype, "
            "variation_synonym1.name, "
            "subsnp_handle.handle, "
            "source2.name, "
            "NULL "
            "FROM "
            "("
            "variation, "
            "source source1, "
            "source source2, "
            "variation_synonym variation_synonym1"
            ") "
            "LEFT JOIN "
            "allele "
            "ON "
            "variation.variation_id = "
            "allele.variation_id "
            "LEFT JOIN "
            "subsnp_handle "
            "ON variation_synonym1.subsnp_id = "
            "subsnp_handle.subsnp_id "
            "WHERE "
            "variation.variation_id = "
            "variation_synonym1.variation_id "
            "AND "
            "variation.source_id = "
            "source1.source_id "
            "AND "
            "variation_synonym1.source_id = "
            "source2.source_id "
            "AND "
            "source2.name = '%s' "
            "ORDER BY "
            "variation.variation_id",
            Ptxtsource);

        /* need to merge both lists, trying to avoid duplicates. */

        gvVariationadaptorFetchAllBySQL(gvva, statement, gvvs);

        ajStrDel(&statement);
    }

    ajCharDel(&Ptxtsource);

    return ajTrue;
}




/* @func ensGvvariationAdaptorFetchAllByGgvopulation **************************
**
** Fetch all Ensembl Genetic Variation Variations via an
** Ensembl Genetic Variation Population.
** The caller is responsible for deleting the
** Ensembl Genetic Variation Variations before deleting the AJAX List.
**
** @cc Bio::EnsEMBL::Variation::DBSQL::VariationAdaptor::
**     fetch_all_by_Population
** @param [u] gvva [EnsPGvvariationadaptor] Ensembl Genetic Variation
**                                          Variation Adaptor
** @param [r] gvp [const EnsPGvpopulation] Ensembl Genetic Variation Population
** @param [u] gvvs [AjPList] AJAX List of Ensembl Genetic Variation Variations
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
** @@
******************************************************************************/

AjBool ensGvvariationAdaptorFetchAllByGvpopulation(EnsPGvvariationadaptor gvva,
                                                   const EnsPGvpopulation gvp,
                                                   AjPList gvvs)
{
    AjPStr statement = NULL;

    if(!gvva)
        return ajFalse;

    if(!gvp)
        return ajFalse;

    if(!gvvs)
        return ajFalse;

    statement = ajFmtStr(
        "SELECT "
        "variation.variation_id, "
        "variation.name, "
        "variation.validation_status, "
        "source1.name, "
        "variation.ancestral_allele, "
        "allele.allele_id, "
        "allele.subsnp_id, "
        "allele.allele, "
        "allele.frequency, "
        "allele.sample_id, "
        "variation_synonym.moltype, "
        "variation_synonym.name, "
        "subsnp_handle.handle, "
        "source2.name, "
        "failed_description.description "
        "FROM "
        "(variation, source source1, allele) "
        "LEFT JOIN "
        "variation_synonym "
        "ON "
        "variation.variation_id = "
        "variation_synonym.variation_id "
        "LEFT JOIN "
        "source source2 "
        "ON "
        "variation_synonym.source_id = "
        "source2.source_id "
        "LEFT JOIN "
        "failed_variation "
        "ON "
        "variation.variation_id = "
        "failed_variation.variation_id "
        "LEFT JOIN "
        "failed_description "
        "ON "
        "failed_variation.failed_description_id = "
        "failed_description.failed_description_id "
        "LEFT JOIN "
        "subsnp_handle "
        "ON "
        "variation_synonym.subsnp_id = "
        "subsnp_handle.subsnp_id "
        "WHERE "
        "variation.variation_id = allele.variation_id "
        "AND "
        "variation.source_id = "
        "source1.source_id "
        "AND "
        "allele.smaple_id = %u",
        ensGvpopulationGetIdentifier(gvp));

    gvVariationadaptorFetchAllBySQL(gvva, statement, gvvs);

    ajStrDel(&statement);

    return ajTrue;
}




/* @func ensGvvariationadaptorFetchAllSources *********************************
**
** Fetch all sources.
** The caller is responsible for deleting the
** AJAX Strings before deleting the AJAX List.
**
** @cc Bio::EnsEMBL::Variation::DBSQL::VariationAdaptor::get_all_sources
** @param [u] gvva [EnsPGvvariationadaptor] Ensembl Genetic Variation
**                                          Variation Adaptor
** @param [u] sources [AjPList] AJAX List of AJAX String sources
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
** @@
******************************************************************************/

AjBool ensGvvariationadaptorFetchAllSources(EnsPGvvariationadaptor gvva,
                                            AjPList sources)
{
    AjPSqlstatement sqls = NULL;
    AjISqlrow sqli       = NULL;
    AjPSqlrow sqlr       = NULL;

    AjPStr source    = NULL;
    AjPStr statement = NULL;

    if(!gvva)
        return ajFalse;

    if(!sources)
        return ajFalse;

    statement = ajStrNewC("SELECT source.name from source");

    sqls = ensDatabaseadaptorSqlstatementNew(gvva, statement);

    sqli = ajSqlrowiterNew(sqls);

    while(!ajSqlrowiterDone(sqli))
    {
        source = ajStrNew();

        sqlr = ajSqlrowiterGet(sqli);

        ajSqlcolumnToStr(sqlr, &source);

        ajListPushAppend(sources, (void *) ajStrNewRef(source));

        ajStrDel(&source);
    }

    ajSqlrowiterDel(&sqli);

    ajSqlstatementDel(&sqls);

    ajStrDel(&statement);

    return ajTrue;
}




/* @func ensGvvariationAdaptorFetchSourceVersion ******************************
**
** Fetch the version of a source.
** The caller is responsible for deleting the AJAX String.
**
** @cc Bio::EnsEMBL::Variation::DBSQL::VariationAdaptor::get_source_version
** @param [r] gvva [const EnsPGvvariationadaptor] Ensembl Genetic Variation
**                                                Variation Adaptor
** @param [r] source [const AjPStr] Source
** @param [u] version [AjPStr] Version
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
** @@
******************************************************************************/

AjBool ensGvvariationadaptorFetchSourceVersion(EnsPGvvariationadaptor gvva,
                                               const AjPStr source,
                                               AjPStr *Pversion)
{
    char *Ptxtsource = NULL;

    AjPSqlstatement sqls = NULL;
    AjISqlrow sqli       = NULL;
    AjPSqlrow sqlr       = NULL;

    AjPStr statement = NULL;
    AjPStr version   = NULL;

    if(!gvva)
        return ajFalse;

    if(!source)
        return ajFalse;

    if(!Pversion)
        return ajFalse;

    ensDatabaseadaptorEscapeC(gvva, &Ptxtsource, source);

    statement = ajFmtStr(
        "SELECT version from source where name ='%s'",
        Ptxtsource);

    ajCharDel(&Ptxtsource);

    sqls = ensDatabaseadaptorSqlstatementNew(gvva, statement);

    sqli = ajSqlrowiterNew(sqls);

    while(!ajSqlrowiterDone(sqli))
    {
        version = ajStrNew();

        sqlr = ajSqlrowiterGet(sqli);

        ajSqlcolumnToStr(sqlr, &version);

        ajStrAssignS(Pversion, version);

        ajStrDel(&version);
    }

    ajSqlrowiterDel(&sqli);

    ajSqlstatementDel(&sqls);

    ajStrDel(&statement);

    return ajTrue;
}




/* @func ensGvvariationadaptorFetchDefaultSource ******************************
**
** Fetch the default sources.
** The caller is responsible for deleting the AJAX String.
**
** @cc Bio::EnsEMBL::Variation::DBSQL::VariationAdaptor::get_default_source
** @param [r] gvva [const EnsPGVVariationAdaptor] Ensembl Genetic Variation
**                                                Variation Adaptor
** @param [u] source [AjStr] Source
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
** @@
******************************************************************************/

AjBool ensGvvariationadaptorFetchDefaultSource(EnsPGvvariationadaptor gvva,
                                               AjPStr *Psource)
{
    AjPList mis = NULL;

    AjPStr key = NULL;

    EnsPMetainformation mi         = NULL;
    EnsPMetainformationadaptor mia = NULL;

    if(!gvva)
        return ajFalse;

    if(!Psource)
        return ajFalse;

    mia = ensRegistryGetMetainformationadaptor(gvva);

    key = ajStrNewC("source.default_source");

    mis = ajListNew();

    ensMetainformationadaptorFetchAllByKey(mia, key, mis);

    ajListPop(mis, (void **) &mi);

    if(mi)
    {
        *Psource = ajStrNewS(ensMetainformationGetValue(mi));

        ensMetainformationDel(&mi);
    }

    while(ajListPop(mis, (void **) &mi))
        ensMetainformationDel(&mi);

    ajListFree(&mis);

    ajStrDel(&key);

    return ajTrue;
}




static AjBool gvVariationadaptorFetchFlankFromCore(EnsPGvvariationadaptor gvva,
                                                   ajuint srid,
                                                   ajint srstart,
                                                   ajint srend,
                                                   ajint srstrand,
                                                   AjPStr *Psequence)
{
    EnsPSlice slice      = NULL;
    EnsPSliceadaptor sla = NULL;

    if(!gvva)
        return ajFalse;

    if(!srid)
        return ajFalse;

    if(!Psequence)
        return ajFalse;

    sla = ensRegistryGetSliceadaptor(gvva);

    /*
    ** FIXME: The Registry does not provide a DNA adaptor function as of
    ** yet. The function above will not work!
    ** The Perl API uses the following functions:

    if(defined $self->db()->dnadb()){
    my $slice_adaptor = $self->db()->dnadb()->get_SliceAdaptor();
    ...
    }

    ** TODO: We need a function to return the Ensembl Database Addptor that
    ** is connected to the DNA database for any particular
    ** Ensembl Database Adaptor.
    ** Maybe we should have an Ensembl Database Adaptor of type 'reference',
    ** which has to be explicitly registered. It it is present it will be
    ** returned, if not, the Database Adaptor with type 'core' will be!
    ** This could work, but requires changes to the registry!
    */

    if(sla)
    {
        ensSliceadaptorFetchBySeqregionIdentifier(sla, srid, 0, 0, 0, &slice);

        if(!slice)
            ajWarn("gvVariationadaptorFetchFlankFromCore could not get an "
                   "Ensembl Slice for Ensembl Sequence region identifier %u.",
                   srid);

        ensSliceFetchSubSequenceStr(slice,
                                    srstart,
                                    srend,
                                    srstrand,
                                    Psequence);
    }

    /*
    ** FIXME: The Perl API has the following two lines twice, but $flank isn't
    ** used. This is inefficient.
    my $flank = $slice->subseq($seq_region_start,$seq_region_end,$seq_region_strand);
    return $slice->subseq($seq_region_start,$seq_region_end,$seq_region_strand);
    */

    return ajTrue;
}




AjBool ensGvvariationadaptorFetchFlankingSequence(EnsPGvvariationadaptor gvva,
                                                  ajuint variationid,
                                                  AjPStr *Pfiveseq,
                                                  AjPStr *Pthreeseq)
{
    ajint srstrand = 0;

    ajuint srid     = 0;
    ajuint usrstart = 0;
    ajuint usrend   = 0;
    ajuint dsrstart = 0;
    ajuint dsrend   = 0;

    AjPSqlstatement sqls = NULL;
    AjISqlrow sqli       = NULL;
    AjPSqlrow sqlr       = NULL;

    AjPStr statement = NULL;
    AjPStr useq      = NULL;
    AjPStr dseq      = NULL;

    if(!gvva)
        return ajFalse;

    if(!variationid)
        return ajFalse;

    if(!Pfiveseq)
        return ajFalse;

    if(!Pthreeseq)
        return ajFalse;

    statement = ajFmtStr(
        "SELECT "
        "flanking_sequence.seq_region_id, "
        "flanking_sequence.seq_region_strand, "
        "flanking_sequence.up_seq, "
        "flanking_sequence.down_seq, "
        "flanking_sequence.up_seq_region_start, "
        "flanking_sequence.up_seq_region_end, "
        "flanking_sequence.down_seq_region_start, "
        "flanking_sequence.down_seq_region_end "
        "FROM "
        "flanking_sequence "
        "WHERE "
        "flanking_sequence.variation_id = %u",
        variationid);

    sqls = ensDatabaseadaptorSqlstatementNew(gvva, statement);

    sqli = ajSqlrowiterNew(sqls);

    while(!ajSqlrowiterDone(sqli))
    {
        srid     = 0;
        srstrand = 0;
        useq     = ajStrNew();
        dseq     = ajStrNew();
        usrstart = 0;
        usrend   = 0;
        dsrstart = 0;
        dsrend   = 0;

        sqlr = ajSqlrowiterGet(sqli);

        ajSqlcolumnToUint(sqlr, &srid);
        ajSqlcolumnToInt(sqlr, &srstrand);
        ajSqlcolumnToStr(sqlr, &useq);
        ajSqlcolumnToStr(sqlr, &dseq);
        ajSqlcolumnToUint(sqlr, &usrstart);
        ajSqlcolumnToUint(sqlr, &usrend);
        ajSqlcolumnToUint(sqlr, &dsrstart);
        ajSqlcolumnToUint(sqlr, &dsrend);

        if(!ajStrGetLen(useq))
        {
            if(srid)
                gvVariationadaptorFetchFlankFromCore(gvva,
                                                     srid,
                                                     usrstart,
                                                     usrend,
                                                     srstrand,
                                                     &useq);
            else
                ajWarn("ensGvvariationadaptorFetchFlankingSequence "
                       "could not get "
                       "Ensembl Sequence Region identifier for "
                       "Ensembl Genetic Variation Variation %u.",
                       variationid);
        }

        ajStrAssignS(Pfiveseq, useq);

        if(!ajStrGetLen(dseq))
        {
            if(srid)
                gvVariationadaptorFetchFlankFromCore(gvva,
                                                     srid,
                                                     dsrstart,
                                                     dsrend,
                                                     srstrand,
                                                     &dseq);
            else
                ajWarn("ensGVVariationAdaptorFetchFlankingSequence "
                       "could not get "
                       "Ensembl Sequence Region identifier for "
                       "Ensembl Genetic Variation Variation %u.",
                       variationid);    
        }

        ajStrAssignS(Pthreeseq, dseq);

        ajStrDel(&useq);
        ajStrDel(&dseq);
    }

    ajSqlrowiterDel(&sqli);

    ajSqlstatementDel(&sqls);

    ajStrDel(&statement);

    return ajTrue;
}




/* @datasection [EnsPGvvariationfeature] Genetic Variation Variation Feature **
**
** Functions for manipulating Ensembl Genetic Variation Variation Feature
** objects
**
** @nam2rule Gvvariationfeature
**
******************************************************************************/




/* @section constructors ******************************************************
**
** All constructors return a new Ensembl Genetic Variation Variation Feature by
** pointer. It is the responsibility of the user to first destroy any previous
** Genetic Variation Variation Feature. The target pointer does not need to be
** initialised to NULL, but it is good programming practice to do so anyway.
**
** @fdata [EnsPGvvariationfeature]
** @fnote None
**
** @nam3rule New Constructor
** @nam4rule NewObj Constructor with existing object
** @nam4rule NewRef Constructor by incrementing the reference counter
**
** @argrule Obj object [EnsPGvvariationfeature] Ensembl Genetic Variation
**                                              Variation Feature
** @argrule Ref object [EnsPGvvariationfeature] Ensembl Genetic Variation
**                                              Variation Feature
**
** @valrule * [EnsPGvvariationfeature] Ensembl Genetic Variation
**                                     Variation Feature
**
** @fcategory new
******************************************************************************/




/* @func ensGvvariationfeatureNew *********************************************
**
** Default constructor for an Ensembl Genetic Variation Variation Feature.
**
** @cc Bio::EnsEMBL::Storable::new
** @param [r] gvvfa [EnsPGvvariationfeatureadaptor] Ensembl Genetic Variation
**                                                  Variation Feature Adaptor
** @param [r] identifier [ajuint] SQL database-internal identifier
** @cc Bio::EnsEMBL::Variation::VariationFeature::new
** @param [u] feature [EnsPFeature] Ensembl Feature
** @param [u] gvv [EnsPGvvariation] Ensembl Genetic Variation Variation
** @param [u] name [AjPStr] Name
** @param [u] source [AjPStr] Source
** @param [u] validation [AjPStr] Validation code
** @param [u] consequence [AjPStr] Consequence type
** @param [r] mapweight [ajuint] Map weight
**
** @return [EnsPGvvariationfeature] Ensembl Genetic Variation Variation Feature
** @@
******************************************************************************/

EnsPGvvariationfeature ensGvvariationfeatureNew(
    EnsPGvvariationfeatureadaptor gvvfa,
    ajuint identifier,
    EnsPFeature feature,
    EnsPGvvariation gvv,
    AjPStr name,
    AjPStr source,
    AjPStr validation,
    AjPStr consequence,
    ajuint mapweight)
{
    EnsPGvvariationfeature gvvf = NULL;

    /* FIXME: Input restrictions are missing. */

    AJNEW0(gvvf);

    gvvf->Use = 1;

    gvvf->Identifier = identifier;

    gvvf->Adaptor = gvvfa;

    gvvf->Feature = ensFeatureNewRef(feature);

    gvvf->Gvvariation = ensGvvariationNewRef(gvv);

    if(name)
        gvvf->Name = ajStrNewRef(name);

    if(source)
        gvvf->Source = ajStrNewRef(source);

    if(validation)
        gvvf->ValidationCode = ajStrNewRef(validation);

    if(consequence)
        gvvf->ConsequenceType = ajStrNewRef(consequence);

    gvvf->MapWeight = mapweight;

    return gvvf;
}




/* @func ensGvvariationfeatureNewObj ******************************************
**
** Object-based constructor function, which returns an independent object.
**
** @param [r] object [const EnsPGvvariationfeature] Ensembl Genetic Variation
**                                                  Variation Feature
**
** @return [EnsPGvvariationfeature] Ensembl Genetic Variation
**                                  Variation Feature or NULL
** @@
******************************************************************************/

EnsPGvvariationfeature ensGvvariationfeatureNewObj(
    const EnsPGvvariationfeature object)
{
    EnsPGvvariationfeature gvvf = NULL;

    if(!object)
        return NULL;

    AJNEW0(gvvf);

    gvvf->Use = 1;

    gvvf->Identifier = object->Identifier;

    gvvf->Adaptor = object->Adaptor;

    gvvf->Feature = ensFeatureNewRef(object->Feature);

    gvvf->Gvvariation = ensGvvariationNewRef(object->Gvvariation);

    if(object->Name)
        gvvf->Name = ajStrNewRef(object->Name);

    if(object->Source)
        gvvf->Source = ajStrNewRef(object->Source);

    if(object->ValidationCode)
        gvvf->Source = ajStrNewRef(object->ValidationCode);

    if(object->ConsequenceType)
        gvvf->Source = ajStrNewRef(object->ConsequenceType);

    gvvf->MapWeight = object->MapWeight;

    return gvvf;
}




/* @func ensGvvariationfeatureNewRef ******************************************
**
** Ensembl Object referencing function, which returns a pointer to the
** Ensembl Object passed in and increases its reference count.
**
** @param [u] gvvf [EnsPGvvariationfeature] Ensembl Genetic Variation
**                                          Variation Feature
**
** @return [EnsPGvvariationfeature] Ensembl Genetic Variation Variation Feature
** @@
******************************************************************************/

EnsPGvvariationfeature ensGvvariationfeatureNewRef(EnsPGvvariationfeature gvvf)
{
    if(!gvvf)
        return NULL;

    gvvf->Use++;

    return gvvf;
}




/* @section destructors *******************************************************
**
** Destruction destroys all internal data structures and frees the
** memory allocated for the Ensembl Genetic Variation Variation Feature.
**
** @fdata [EnsPGvvariationfeature]
** @fnote None
**
** @nam3rule Del Destroy (free) a Genetic Variation Variation Feature object
**
** @argrule * Pgvvf [EnsPGvvariationfeature*] Genetic Variation
**                                            Variation Feature object address
**
** @valrule * [void]
**
** @fcategory delete
******************************************************************************/




/* @func ensGvvariationfeatureDel *********************************************
**
** Default destructor for an Ensembl Genetic Variation Variation Feature.
**
** @param [d] Pgvvf [EnsPGvvariationfeature*] Ensembl Genetic Variation
**                                            Variation Feature address
**
** @return [void]
** @@
******************************************************************************/

void ensGvvariationfeatureDel(EnsPGvvariationfeature *Pgvvf)
{
    EnsPGvvariationfeature pthis = NULL;

    if(!Pgvvf)
        return;

    if(!*Pgvvf)
        return;

    if(ajDebugTest("ensGvvariationfeatureDel"))
    {
        ajDebug("ensGvvariationfeatureDel\n"
                "  *Pgvvf %p\n",
                *Pgvvf);

        ensGvvariationfeatureTrace(*Pgvvf, 1);
    }

    pthis = *Pgvvf;

    pthis->Use--;

    if(pthis->Use)
    {
        *Pgvvf = NULL;

        return;
    }

    ensFeatureDel(&pthis->Feature);

    ensGvvariationDel(&pthis->Gvvariation);

    ajStrDel(&pthis->Name);
    ajStrDel(&pthis->Source);
    ajStrDel(&pthis->ValidationCode);
    ajStrDel(&pthis->ConsequenceType);

    AJFREE(pthis);

    *Pgvvf = NULL;

    return;
}




/* @section element retrieval *************************************************
**
** Functions for returning elements of an
** Ensembl Genetic Variation Variation Feature object.
**
** @fdata [EnsPGvvariationfeature]
** @fnote None
**
** @nam3rule Get Return Genetic Variation Variation Feature attribute(s)
** @nam4rule GetAdaptor Return the Ensembl Genetic Variation
**                      Variation Feature Adaptor
** @nam4rule GetIdentifier Return the SQL database-internal identifier
** @nam4rule GetFeature Return the Ensembl Feature
** @nam4rule GetGvvariation Return the Ensembl Genetic Variation Variation
** @nam4rule GetName Return the name
** @nam4rule GetSource Return the source
** @nam4rule GetValidationCode Return the validation code
** @nam4rule GetConsequenceType Return the consequence type
** @nam4rule GetMapWeight Return the map weight
**
** @argrule * gvvf [const EnsPGvvariationfeature] Genetic Variation
**                                                Variation Feature
**
** @valrule Adaptor [EnsPGvvariationfeatureadaptor] Ensembl Genetic Variation
**                                                  Variation Feature Adaptor
** @valrule Identifier [ajuint] SQL database-internal identifier
** @valrule Feature [EnsPFeature] Ensembl Feature
** @valrule Gvvariation [EnsPGvvariation] Ensembl Genetic Variation Variation
** @valrule Name [AjPStr] Name
** @valrule Source [AjPStr] Source
** @valrule ValidationCode [AjPStr] Validation code
** @valrule ConsequenceType [AjPStr] Consequence type
** @valrule MapWeight [ajuint] Map weight
**
** @fcategory use
******************************************************************************/




/* @func ensGvvariationfeatureGetAdaptor **************************************
**
** Get the Ensembl Genetic Variation Variation Feature Adaptor element of an
** Ensembl Genetic Variation Variation Feature.
**
** @param [r] gvvf [const EnsPGvvariationfeature] Ensembl Genetic Variation
**                                                Variation Feature
**
** @return [EnsPGvvariationfeatureadaptor] Ensembl Genetic Variation
**                                         Variation Feature Adaptor
** @@
******************************************************************************/

EnsPGvvariationfeatureadaptor ensGvvariationfeatureGetAdaptor(
    const EnsPGvvariationfeature gvvf)
{
    if(!gvvf)
        return NULL;

    return gvvf->Adaptor;
}




/* @func ensGvvariationfeatureGetIdentifier ***********************************
**
** Get the SQL database-internal identifier element of an
** Ensembl Genetic Variation Variation Feature.
**
** @param [r] gvvf [const EnsPGvvariationfeature] Ensembl Genetic Variation
**                                                Variation Feature
**
** @return [ajuint] Internal database identifier
** @@
******************************************************************************/

ajuint ensGvvariationfeatureGetIdentifier(const EnsPGvvariationfeature gvvf)
{
    if(!gvvf)
        return 0;

    return gvvf->Identifier;
}




/* @func ensGvvariationfeatureGetFeature **************************************
**
** Get the Ensembl Feature element of an
** Ensembl Genetic Variation Variation Feature.
**
** @param [r] gvvf [const EnsPGvvariationfeature] Ensembl Genetic Variation
**                                                Variation Feature
**
** @return [EnsPFeature] Ensembl Feature
** @@
******************************************************************************/

EnsPFeature ensGvvariationfeatureGetFeature(const EnsPGvvariationfeature gvvf)
{
    if(!gvvf)
        return NULL;

    return gvvf->Feature;
}




/* @func ensGvvariationfeatureGetGvvariation **********************************
**
** Get the Ensembl Genetic Variation Variation element of an
** Ensembl Genetic Variation Variation Feature.
**
** @param [r] gvvf [const EnsPGvvariationfeature] Ensembl Genetic Variation
**                                                Variation Feature
**
** @return [EnsPGvvariation] Ensembl Genetic Variation Variation
** @@
******************************************************************************/

EnsPGvvariation ensGvvariationfeatureGetGvvariation(
    const EnsPGvvariationfeature gvvf)
{
    if(!gvvf)
        return NULL;

    return gvvf->Gvvariation;
}




/* @func ensGvvariationfeatureGetName *****************************************
**
** Get the name element of an Ensembl Genetic Variation Variation Feature.
**
** @param [r] gvvf [const EnsPGvvariationfeature] Ensembl Genetic Variation
**                                                Variation Feature
**
** @return [AjPStr] Name
** @@
******************************************************************************/

AjPStr ensGvvariationfeatureGetName(const EnsPGvvariationfeature gvvf)
{
    if(!gvvf)
        return NULL;

    return gvvf->Name;
}




/* @func ensGvvariationfeatureGetSource ***************************************
**
** Get the source element of an Ensembl Genetic Variation Variation Feature.
**
** @param [r] gvvf [const EnsPGvvariationfeature] Ensembl Genetic Variation
**                                                Variation Feature
**
** @return [AjPStr] Source
** @@
******************************************************************************/

AjPStr ensGvvariationfeatureGetSource(const EnsPGvvariationfeature gvvf)
{
    if(!gvvf)
        return NULL;

    return gvvf->Source;
}




/* @func ensGvvariationfeatureGetValidationCode *******************************
**
** Get the validation code element of an
** Ensembl Genetic Variation Variation Feature.
**
** @param [r] gvvf [const EnsPGvvariationfeature] Ensembl Genetic Variation
**                                                Variation Feature
**
** @return [AjPStr] Validation code
** @@
******************************************************************************/

AjPStr ensGvvariationfeatureGetValidationCode(const EnsPGvvariationfeature gvvf)
{
    if(!gvvf)
        return NULL;

    return gvvf->ValidationCode;
}




/* @func ensGvvariationfeatureGetConsequenceType ******************************
**
** Get the consequence type element of an
** Ensembl Genetic Variation Variation Feature.
**
** @param [r] gvvf [const EnsPGvvariationfeature] Ensembl Genetic Variation
**                                                Variation Feature
**
** @return [AjPStr] Consequence type
** @@
******************************************************************************/

AjPStr ensGvvariationfeatureGetConsequenceType(
    const EnsPGvvariationfeature gvvf)
{
    if(!gvvf)
        return NULL;

    return gvvf->ConsequenceType;
}




/* @func ensGvvariationfeatureGetMapWeight ************************************
**
** Get the map weight element of an
** Ensembl Genetic Variation Variation Feature.
**
** @param [r] gvvf [const EnsPGvvariationfeature] Ensembl Genetic Variation
**                                                Variation Feature
**
** @return [ajuint] Map weight
** @@
******************************************************************************/

ajuint ensGvvariationfeatureGetMapWeight(const EnsPGvvariationfeature gvvf)
{
    if(!gvvf)
        return 0;

    return gvvf->MapWeight;
}




/* @section element assignment ************************************************
**
** Functions for assigning elements of an
** Ensembl Genetic Variation Variation Feature object.
**
** @fdata [EnsPGvvariationfeature]
** @fnote None
**
** @nam3rule Set Set one element of a Genetic Variation Variation Feature
** @nam4rule SetAdaptor Set the Ensembl Genetic Variation
**                      Variation Feature Adaptor
** @nam4rule SetIdentifier Set the SQL database-internal identifier
** @nam4rule SetFeature Set the Ensembl Feature
** @nam4rule SetGvvariation Set the Ensembl Genetic Variation Variation
** @nam4rule SetName Set the name
** @nam4rule SetSource Set the source
** @nam4rule SetValidationCode Set the validation code
** @nam4rule SetConsequenceType Set the consequence type
** @nam4rule SetMapWeight Set the map weight
**
** @argrule * gvvf [EnsPGvvariationfeature] Ensembl Genetic Variation
**                                          Variation Feature object
**
** @valrule * [AjBool] ajTrue upon success, ajFalse otherwise
**
** @fcategory modify
******************************************************************************/




/* @func ensGvvariationfeatureSetAdaptor **************************************
**
** Set the Ensembl Genetic Variation Variation Feature Adaptor element of an
** Ensembl Genetic Variation Variation Feature.
**
** @param [u] gvvf [EnsPGvvariationfeature] Ensembl Genetic Variation
**                                          Variation Feature
** @param [r] gvvfa [EnsPGvvariationfeatureadaptor] Ensembl Genetic Variation
**                                                  Variation Feature Adaptor
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
** @@
******************************************************************************/

AjBool ensGvvariationfeatureSetAdaptor(EnsPGvvariationfeature gvvf,
                                       EnsPGvvariationfeatureadaptor gvvfa)
{
    if(!gvvf)
        return ajFalse;

    gvvf->Adaptor = gvvfa;

    return ajTrue;
}




/* @func ensGvvariationSetIdentifier ******************************************
**
** Set the SQL database-internal identifier element of an
** Ensembl Genetic Variation Variation Feature.
**
** @param [u] gvvf [EnsPGvvariationfeature] Ensembl Genetic Variation
**                                          Variation Feature
** @param [r] identifier [ajuint] SQL database-internal identifier
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
** @@
******************************************************************************/

AjBool ensGvvariationfeatureSetIdentifier(EnsPGvvariationfeature gvvf,
                                          ajuint identifier)
{
    if(!gvvf)
        return ajFalse;

    gvvf->Identifier = identifier;

    return ajTrue;
}




/* @func ensGvvariationfeatureSetFeature **************************************
**
** Set the Ensembl Feature element of an
** Ensembl Genetic Variation Variation Feature.
**
** @param [u] gvvf [EnsPGvvariationfeature] Ensembl Genetic Variation
**                                          Variation Feature
** @param [u] feature [EnsPFeature] Ensembl Feature
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
** @@
******************************************************************************/

AjBool ensGvvariationfeatureSetFeature(EnsPGvvariationfeature gvvf,
                                       EnsPFeature feature)
{
    if(!gvvf)
        return ajFalse;

    ensFeatureDel(&gvvf->Feature);

    gvvf->Feature = ensFeatureNewRef(feature);

    return ajTrue;
}




/* @func ensGvvariationfeatureSetGvvariation **********************************
**
** Set the Ensembl Gentic Variation Variation element of an
** Ensembl Genetic Variation Variation Feature.
**
** @param [u] gvvf [EnsPGvvariationfeature] Ensembl Genetic Variation
**                                          Variation Feature
** @param [u] gvv [EnsPGvvariation] Ensembl Genetic Variation Variation
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
** @@
******************************************************************************/

AjBool ensGvvariationfeatureSetGvvariation(EnsPGvvariationfeature gvvf,
                                           EnsPGvvariation gvv)
{
    if(!gvvf)
        return ajFalse;

    ensGvvariationDel(&gvvf->Gvvariation);

    gvvf->Gvvariation = ensGvvariationNewRef(gvv);

    return ajTrue;
}




/* @func ensGvvariationfeatureSetName *****************************************
**
** Set the name element of an Ensembl Genetic Variation Variation Feature.
**
** @param [u] gvvf [EnsPGvvariationfeature] Ensembl Genetic Variation
**                                          Variation Feature
** @param [u] name [AjPStr] Name
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
** @@
******************************************************************************/

AjBool ensGvvariationfeatureSetName(EnsPGvvariationfeature gvvf, AjPStr name)
{
    if(!gvvf)
        return ajFalse;

    ajStrDel(&gvvf->Name);

    if(name)
        gvvf->Name = ajStrNewRef(name);

    return ajTrue;
}




/* @func ensGvvariationfeatureSetSource ***************************************
**
** Set the source element of an Ensembl Genetic Variation Variation Feature.
**
** @param [u] gvvf [EnsPGvvariationfeature] Ensembl Genetic Variation
**                                          Variation Feature
** @param [u] source [AjPStr] Source
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
** @@
******************************************************************************/

AjBool ensGvvariationfeatureSetSource(EnsPGvvariationfeature gvvf,
                                      AjPStr source)
{
    if(!gvvf)
        return ajFalse;

    ajStrDel(&gvvf->Source);

    if(source)
        gvvf->Name = ajStrNewRef(source);

    return ajTrue;
}




/* @func ensGvvariationfeatureSetValidationCode *******************************
**
** Set the validation code element of an
** Ensembl Genetic Variation Variation Feature.
**
** @param [u] gvvf [EnsPGvvariationfeature] Ensembl Genetic Variation
**                                          Variation Feature
** @param [u] validationcode [AjPStr] Validation code
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
** @@
******************************************************************************/

AjBool ensGvvariationfeatureSetValidationCode(EnsPGvvariationfeature gvvf,
                                              AjPStr validationcode)
{
    if(!gvvf)
        return ajFalse;

    ajStrDel(&gvvf->ValidationCode);

    if(validationcode)
        gvvf->ValidationCode = ajStrNewRef(validationcode);

    return ajTrue;
}




/* @func ensGvvariationfeatureSetConsequenceType ******************************
**
** Set the consequence type element of an
** Ensembl Genetic Variation Variation Feature.
**
** @param [u] gvvf [EnsPGvvariationfeature] Ensembl Genetic Variation
**                                          Variation Feature
** @param [u] consequencetype [AjPStr] Consequence type
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
** @@
******************************************************************************/

AjBool ensGvvariationfeatureSetConsequenceType(EnsPGvvariationfeature gvvf,
                                               AjPStr consequencetype)
{
    if(!gvvf)
        return ajFalse;

    ajStrDel(&gvvf->ConsequenceType);

    if(consequencetype)
        gvvf->ConsequenceType = ajStrNewRef(consequencetype);

    return ajTrue;
}




/* @func ensGvvariationfeatureSetMapWeight ************************************
**
** Set the map weight element of an
** Ensembl Genetic Variation Variation Feature.
**
** @param [u] gvvf [EnsPGvvariationfeature] Ensembl Genetic Variation
**                                          Variation Feature
** @param [r] mapweight [ajuint] Map weight
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
** @@
******************************************************************************/

AjBool ensGvvariationfeatureSetMapWeight(EnsPGvvariationfeature gvvf,
                                         ajuint mapweight)
{
    if(!gvvf)
        return ajFalse;

    gvvf->MapWeight = mapweight;

    return ajTrue;
}




/* @func ensGvvariationfeatureGetMemSize **************************************
**
** Get the memory size in bytes of an
** Ensembl Genetic Variation Variation Feature.
**
** @param [r] gvvf [const EnsPGvvariationfeature] Ensembl Genetic Variation
**                                                Variation Feature
**
** @return [ajuint] Memory size
** @@
******************************************************************************/

ajuint ensGvvariationfeatureGetMemSize(const EnsPGvvariationfeature gvvf)
{
    ajuint size = 0;

    if(!gvvf)
        return 0;

    size += (ajuint) sizeof (EnsOGvvariationfeature);

    size += ensFeatureGetMemSize(gvvf->Feature);

    size += ensGvvariationGetMemSize(gvvf->Gvvariation);

    if(gvvf->Name)
    {
        size += (ajuint) sizeof (AjOStr);

        size += ajStrGetRes(gvvf->Name);
    }

    if(gvvf->Source)
    {
        size += (ajuint) sizeof (AjOStr);

        size += ajStrGetRes(gvvf->Source);
    }

    if(gvvf->ValidationCode)
    {
        size += (ajuint) sizeof (AjOStr);

        size += ajStrGetRes(gvvf->ValidationCode);
    }

    if(gvvf->ConsequenceType)
    {
        size += (ajuint) sizeof (AjOStr);

        size += ajStrGetRes(gvvf->ConsequenceType);
    }

    return size;
}




/* @section debugging *********************************************************
**
** Functions for reporting of an
** Ensembl Genetic Variation Variation Feature object.
**
** @fdata [EnsPGvvariationfeature]
** @nam3rule Trace Report Ensembl Genetic Variation Variation Feature elements
**                 to debug file
**
** @argrule Trace gvvf [const EnsPGvvariationfeature] Ensembl Genetic Variation
**                                                    Variation Feature
** @argrule Trace level [ajuint] Indentation level
**
** @valrule * [AjBool] ajTrue upon success, ajFalse otherwise
**
** @fcategory misc
******************************************************************************/




/* @func ensGvvariationfeatureTrace *******************************************
**
** Trace an Ensembl Genetic Variation Variation Feature.
**
** @param [r] gvvf [const EnsPGvvariationfeature] Ensembl Genetic Variation
**                                                Variation Feature
** @param [r] level [ajuint] Indentation level
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
** @@
******************************************************************************/

AjBool ensGvvariationfeatureTrace(const EnsPGvvariationfeature gvvf,
                                  ajuint level)
{
    AjPStr indent = NULL;

    if(!gvvf)
        return ajFalse;

    indent = ajStrNew();

    ajStrAppendCountK(&indent, ' ', level * 2);

    ajDebug("ensGvvariationfeatureTrace %p\n"
            "%S  Use %u\n"
            "%S  Identifier %u\n"
            "%S  Adaptor %p\n"
            "%S  Feature %p\n"
            "%S  Gvvariation %p\n"
            "%S  Name '%S'\n"
            "%S  Source '%S'\n"
            "%S  ValidationCode '%S'\n"
            "%S  ConsequenceType '%S'\n"
            "%S  MapWeight %u\n",
            indent, gvvf,
            indent, gvvf->Use,
            indent, gvvf->Identifier,
            indent, gvvf->Adaptor,
            indent, gvvf->Feature,
            indent, gvvf->Gvvariation,
            indent, gvvf->Name,
            indent, gvvf->Source,
            indent, gvvf->ValidationCode,
            indent, gvvf->ConsequenceType,
            indent, gvvf->MapWeight);

    ajStrDel(&indent);

    ensFeatureTrace(gvvf->Feature, level + 1);

    ensGvvariationTrace(gvvf->Gvvariation, level + 1);

    return ajTrue;
}
