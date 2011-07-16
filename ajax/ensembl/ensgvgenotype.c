/* @source Ensembl Genetic Variation Genotype functions
**
** @author Copyright (C) 1999 Ensembl Developers
** @author Copyright (C) 2006 Michael K. Schuster
** @modified 2009 by Alan Bleasby for incorporation into EMBOSS core
** @modified $Date: 2011/07/06 21:50:28 $ by $Author: mks $
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

#include "ensgvgenotype.h"




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




/* ==================================================================== */
/* ======================== private variables ========================= */
/* ==================================================================== */




/* ==================================================================== */
/* ======================== private functions ========================= */
/* ==================================================================== */




/* ==================================================================== */
/* ===================== All functions by section ===================== */
/* ==================================================================== */




/* @filesection ensgvgenotype *************************************************
**
** @nam1rule ens Function belongs to the Ensembl library
**
******************************************************************************/




/* @datasection [EnsPGvgenotype] Ensembl Genetic Variation Genotype ***********
**
** @nam2rule Gvgenotype Functions for manipulating
** Ensembl Genetic Variation Genotype objects
**
** @cc Bio::EnsEMBL::Variation::Genotype
** @cc CVS Revision: 1.4
** @cc CVS Tag: branch-ensembl-62
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
**
** @nam3rule New Constructor
** @nam4rule Cpy Constructor with existing object
** @nam4rule Ini Constructor with initial values
** @nam4rule Ref Constructor by incrementing the reference counter
**
** @argrule Cpy gvg [const EnsPGvgenotype] Ensembl Genetic Variation Genotype
** @argrule Ini gvga [EnsPGvgenotypeadaptor] Ensembl Genetic Variation
** @argrule Ini identifier [ajuint] SQL database-internal identifier
** @argrule Ini allele1 [AjPStr] Allele 1
** @argrule Ini allele2 [AjPStr] Allele 2
** @argrule Ref gvg [EnsPGvgenotype] Ensembl Genetic Variation Genotype
**
** @valrule * [EnsPGvgenotype] Ensembl Genetic Variation Genotype or NULL
**
** @fcategory new
******************************************************************************/




/* @func ensGvgenotypeNewCpy **************************************************
**
** Object-based constructor function, which returns an independent object.
**
** @param [r] gvg [const EnsPGvgenotype] Ensembl Genetic Variation Genotype
**
** @return [EnsPGvgenotype] Ensembl Genetic Variation Genotype or NULL
** @@
******************************************************************************/

EnsPGvgenotype ensGvgenotypeNewCpy(const EnsPGvgenotype gvg)
{
    EnsPGvgenotype pthis = NULL;

    if(!gvg)
        return NULL;

    AJNEW0(pthis);

    pthis->Use = 1;

    pthis->Identifier = gvg->Identifier;

    pthis->Adaptor = gvg->Adaptor;

    if(gvg->Allele1)
        pthis->Allele1 = ajStrNewRef(gvg->Allele1);

    if(gvg->Allele2)
        pthis->Allele2 = ajStrNewRef(gvg->Allele2);

    return pthis;
}




/* @func ensGvgenotypeNewIni **************************************************
**
** Constructor for an Ensembl Genetic Variation Genotype with initial values.
**
** @cc Bio::EnsEMBL::Storable::new
** @param [u] gvga [EnsPGvgenotypeadaptor] Ensembl Genetic Variation
**                                         Genotype Adaptor
** @param [r] identifier [ajuint] SQL database-internal identifier
** @cc Bio::EnsEMBL::Variation::Genotype::new
** @param [u] allele1 [AjPStr] Allele 1
** @param [u] allele2 [AjPStr] Allele 2
**
** @return [EnsPGvgenotype] Ensembl Genetic Variation Genotype or NULL
** @@
******************************************************************************/

EnsPGvgenotype ensGvgenotypeNewIni(EnsPGvgenotypeadaptor gvga,
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
** memory allocated for an Ensembl Genetic Variation Genotype object.
**
** @fdata [EnsPGvgenotype]
**
** @nam3rule Del Destroy (free) a Genetic Variation Genotype object
**
** @argrule * Pgvg [EnsPGvgenotype*] Ensembl Genetic Variation Genotype
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
**                                   object address
**
** @return [void]
** @@
******************************************************************************/

void ensGvgenotypeDel(EnsPGvgenotype* Pgvg)
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
**
** @nam3rule Get Return Genetic Variation Genotype attribute(s)
** @nam4rule Adaptor Return the Ensembl Genetic Variation Genotype Adaptor
** @nam4rule Allele1 Return the allele 1
** @nam4rule Allele2 Return the allele 2
** @nam4rule Identifier Return the SQL database-internal identifier
**
** @argrule * gvg [const EnsPGvgenotype] Genetic Variation Genotype
**
** @valrule Adaptor [EnsPGvgenotypeadaptor] Ensembl Genetic Variation
**                                          Genotype Adaptor or NULL
** @valrule Allele1 [AjPStr] Allele 1 or NULL
** @valrule Allele2 [AjPStr] Allele 2 or NULL
** @valrule Identifier [ajuint] SQL database-internal identifier or 0
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
** or NULL
** @@
******************************************************************************/

EnsPGvgenotypeadaptor ensGvgenotypeGetAdaptor(const EnsPGvgenotype gvg)
{
    if(!gvg)
        return NULL;

    return gvg->Adaptor;
}




/* @func ensGvgenotypeGetAllele1 **********************************************
**
** Get the allele 1 element of an Ensembl Genetic Variation Genotype.
**
** @param [r] gvg [const EnsPGvgenotype] Ensembl Genetic Variation Genotype
**
** @return [AjPStr] Allele 1 or NULL
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
** @return [AjPStr] Allele 2 or NULL
** @@
******************************************************************************/

AjPStr ensGvgenotypeGetAllele2(const EnsPGvgenotype gvg)
{
    if(!gvg)
        return NULL;

    return gvg->Allele2;
}




/* @func ensGvgenotypeGetIdentifier *******************************************
**
** Get the SQL database-internal identifier element of an
** Ensembl Genetic Variation Genotype.
**
** @param [r] gvg [const EnsPGvgenotype] Ensembl Genetic Variation Genotype
**
** @return [ajuint] SQL database-internal identifier or 0
** @@
******************************************************************************/

ajuint ensGvgenotypeGetIdentifier(const EnsPGvgenotype gvg)
{
    if(!gvg)
        return 0;

    return gvg->Identifier;
}




/* @section element assignment ************************************************
**
** Functions for assigning elements of an
** Ensembl Genetic Variation Genotype object.
**
** @fdata [EnsPGvgenotype]
**
** @nam3rule Set Set one element of a Genetic Variation Genotype
** @nam4rule Adaptor Set the Ensembl Genetic Variation Genotype Adaptor
** @nam4rule Allele1 Set the allele 1
** @nam4rule Allele2 Set the allele 2
** @nam4rule Identifier Set the SQL database-internal identifier
**
** @argrule * gvg [EnsPGvgenotype] Ensembl Genetic Variation Genotype object
** @argrule Adaptor gvga [EnsPGvgenotypeadaptor] Ensembl Genetic Variation
** @argrule Allele1 allele1 [AjPStr] Allele 1
** @argrule Allele2 allele2 [AjPStr] Allele 2
** @argrule Identifier identifier [ajuint] SQL database-internal identifier
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
** @param [u] gvg [EnsPGvgenotype] Ensembl Genetic Variation Genotype
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




/* @section debugging *********************************************************
**
** Functions for reporting of an Ensembl Genetic Variation Genotype object.
**
** @fdata [EnsPGvgenotype]
**
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




/* @section calculate *********************************************************
**
** Functions for calculating values of an
** Ensembl Genetic Variation Genotype object.
**
** @fdata [EnsPGvgenotype]
**
** @nam3rule Calculate Calculate Ensembl Genetic Variation Genotype values
** @nam4rule Memsize Calculate the memory size in bytes
**
** @argrule * gvg [const EnsPGvgenotype] Ensembl Genetic Variation Genotype
**
** @valrule Memsize [size_t] Memory size in bytes or 0
**
** @fcategory misc
******************************************************************************/




/* @func ensGvgenotypeCalculateMemsize ****************************************
**
** Calculate the memory size in bytes of an
** Ensembl Genetic Variation Genotype.
**
** @param [r] gvg [const EnsPGvgenotype] Ensembl Genetic Variation Genotype
**
** @return [size_t] Memory size in bytes or 0
** @@
******************************************************************************/

size_t ensGvgenotypeCalculateMemsize(const EnsPGvgenotype gvg)
{
    size_t size = 0;

    if(!gvg)
        return 0;

    size += sizeof (EnsOGvgenotype);

    if(gvg->Allele1)
    {
        size += sizeof (AjOStr);

        size += ajStrGetRes(gvg->Allele1);
    }

    if(gvg->Allele2)
    {
        size += sizeof (AjOStr);

        size += ajStrGetRes(gvg->Allele2);
    }

    return size;
}
