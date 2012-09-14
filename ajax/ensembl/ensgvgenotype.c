/* @source ensgvgenotype ******************************************************
**
** Ensembl Genetic Variation Genotype functions
**
** @author Copyright (C) 1999 Ensembl Developers
** @author Copyright (C) 2006 Michael K. Schuster
** @version $Revision: 1.18 $
** @modified 2009 by Alan Bleasby for incorporation into EMBOSS core
** @modified $Date: 2012/07/14 14:52:40 $ by $Author: rice $
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

#include "ensgvbaseadaptor.h"
#include "ensgvdatabaseadaptor.h"
#include "ensgvgenotype.h"
#include "ensgvvariation.h"




/* ========================================================================= */
/* =============================== constants =============================== */
/* ========================================================================= */

/* @conststatic gvgenotypecodeadaptorKTables ***********************************
**
** Array of Ensembl Genetic Variation Genotype Code Adaptor SQL table names
**
******************************************************************************/

static const char *gvgenotypecodeadaptorKTables[] =
{
    "genotype_code",
    "allele_code",
    (const char *) NULL
};




/* @conststatic gvgenotypecodeadaptorKColumns *********************************
**
** Array of Ensembl Genetic Variation Genotype Code Adaptor SQL column names
**
******************************************************************************/

static const char *gvgenotypecodeadaptorKColumns[] =
{
    "genotype_code.genotype_code_id",
    "genotype_code.haplotype_id",
    "allele_code.allele",
    (const char *) NULL
};




/* @conststatic gvgenotypecodeadaptorKDefaultcondition ************************
**
** Ensembl Genetic Variation Genotype Code Adaptor SQL default condition
**
******************************************************************************/

static const char *gvgenotypecodeadaptorKDefaultcondition =
    "genotype_code.allele_code_id = allele_code.allele_code_id";




/* ========================================================================= */
/* =========================== global variables ============================ */
/* ========================================================================= */




/* ========================================================================= */
/* ============================= private data ============================== */
/* ========================================================================= */

/* @datastatic GvPHaplotype ***************************************************
**
** Genetic Variation Haplotype.
**
** Associates haplotype identifiers with alleles.
**
** @alias GvSHaplotype
** @alias GvOHaplotype
**
** @attr Haplotypeidentifier [ajuint] Haplotype identifier
** @attr Padding [ajuint] Padding to alignment boundary
** @attr Allele [AjPStr] Allele
** @@
******************************************************************************/

typedef struct GvSHaplotype
{
    ajuint Haplotypeidentifier;
    ajuint Padding;
    AjPStr Allele;
} GvOHaplotype;

#define GvPHaplotype GvOHaplotype*




/* ========================================================================= */
/* =========================== private constants =========================== */
/* ========================================================================= */




/* ========================================================================= */
/* =========================== private variables =========================== */
/* ========================================================================= */




/* ========================================================================= */
/* =========================== private functions =========================== */
/* ========================================================================= */

static GvPHaplotype gvHaplotypeNewIni(ajuint haplotypeid, AjPStr allele);

static void gvHaplotypeDel(GvPHaplotype *Pgvh);

static int listGvhaplotypeCompareHaplotypeidentifierAscending(
    const void *item1,
    const void *item2);

static AjBool listGvhaplotypeSortHaplotypeidentifierAscending(AjPList gvhs);

static AjBool gvgenotypecodeadaptorFetchAllbyStatement(
    EnsPBaseadaptor ba,
    const AjPStr statement,
    EnsPAssemblymapper am,
    EnsPSlice slice,
    AjPList gvgcs);




/* ========================================================================= */
/* ======================= All functions by section ======================== */
/* ========================================================================= */




/* @filesection ensgvgenotype *************************************************
**
** @nam1rule ens Function belongs to the Ensembl library
**
******************************************************************************/




/* @funcstatic gvHaplotypeNewIni **********************************************
**
** Genetic Variation Haplotype constructor with initial values.
**
** @param [r] haplotypeid [ajuint] Haplotype identifier
** @param [u] allele [AjPStr] Allele
**
** @return [GvPHaplotype] Genetic Variation Haplotype or NULL
**
** @release 6.5.0
** @@
******************************************************************************/

static GvPHaplotype gvHaplotypeNewIni(ajuint haplotypeid, AjPStr allele)
{
    GvPHaplotype gvh = NULL;

    if (!haplotypeid)
        return NULL;

    if (!allele)
        return NULL;

    AJNEW0(gvh);

    gvh->Haplotypeidentifier = haplotypeid;

    gvh->Allele = ajStrNewRef(allele);

    return gvh;
}




/* @funcstatic gvHaplotypeDel *************************************************
**
** Default destructor for a Genetic Variation Haplotype object.
**
** @param [d] Pgvh [GvPHaplotype*] Genetic Variation Haplotype address
**
** @return [void]
**
** @release 6.5.0
** @@
******************************************************************************/

static void gvHaplotypeDel(GvPHaplotype *Pgvh)
{
    GvPHaplotype pthis = NULL;

    if (!Pgvh)
        return;

    pthis = *Pgvh;

    ajStrDel(&pthis->Allele);

    AJFREE(pthis);

    *Pgvh = NULL;

    return;
}




/* @funcstatic listGvhaplotypeCompareHaplotypeidentifierAscending *************
**
** AJAX List of Genetic Variation Haplotype objects comparison function
** to sort by haplotype identifier in ascending order.
**
** @param [r] item1 [const void*] Genetic Variation Haplotype address 1
** @param [r] item2 [const void*] Genetic Variation Haplotype address 2
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

static int listGvhaplotypeCompareHaplotypeidentifierAscending(
    const void *item1,
    const void *item2)
{
    GvPHaplotype gvh1 = *(GvOHaplotype *const *) item1;
    GvPHaplotype gvh2 = *(GvOHaplotype *const *) item2;

#if defined(AJ_DEBUG) && AJ_DEBUG >= 2
    if (ajDebugTest("listGvhaplotypeCompareHaplotypeidentifierAscending"))
        ajDebug("listGvhaplotypeCompareHaplotypeidentifierAscending\n"
                "  gvh1 %p\n"
                "  gvh2 %p\n",
                gvh1,
                gvh2);
#endif /* defined(AJ_DEBUG) && AJ_DEBUG >= 2 */

    /* Sort empty values towards the end of the AJAX List. */

    if (gvh1 && (!gvh2))
        return -1;

    if ((!gvh1) && (!gvh2))
        return 0;

    if ((!gvh1) && gvh2)
        return +1;

    if (gvh1->Haplotypeidentifier < gvh2->Haplotypeidentifier)
        return -1;

    if (gvh1->Haplotypeidentifier > gvh2->Haplotypeidentifier)
        return +1;

    return 0;
}




/* @funcstatic listGvhaplotypeSortHaplotypeidentifierAscending ****************
**
** Sort an AJAX List of Genetic Variation Haplotype objects by their
** haplotype identifier in ascending order.
**
** @param [u] gvhs [AjPList] AJAX List of Genetic Variation Haplotype objects
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
**
** @release 6.4.0
** @@
******************************************************************************/

static AjBool listGvhaplotypeSortHaplotypeidentifierAscending(AjPList gvhs)
{
    if (!gvhs)
        return ajFalse;

    ajListSort(gvhs, &listGvhaplotypeCompareHaplotypeidentifierAscending);

    return ajTrue;
}




/* @datasection [EnsPGvgenotype] Ensembl Genetic Variation Genotype ***********
**
** @nam2rule Gvgenotype Functions for manipulating
** Ensembl Genetic Variation Genotype objects
**
** @cc Bio::EnsEMBL::Variation::Genotype
** @cc CVS Revision: 1.8
** @cc CVS Tag: branch-ensembl-66
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
** @argrule Ini gvga [EnsPGvgenotypeadaptor]
** Ensembl Genetic Variation Genotype Adaptor
** @argrule Ini identifier [ajuint] SQL database-internal identifier
** @argrule Ini gvv [EnsPGvvariation] Ensembl Genetic Variation Variation
** @argrule Ini alleles [AjPList] AJAX List of AJAX String objects
** @argrule Ini subhandle [AjPStr] Subhandle
** @argrule Ini subidentifier [ajuint] Subidentifier
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
**
** @release 6.4.0
** @@
******************************************************************************/

EnsPGvgenotype ensGvgenotypeNewCpy(const EnsPGvgenotype gvg)
{
    AjIList iter = NULL;

    AjPStr allele = NULL;

    EnsPGvgenotype pthis = NULL;

    if (!gvg)
        return NULL;

    AJNEW0(pthis);

    pthis->Use        = 1U;
    pthis->Identifier = gvg->Identifier;
    pthis->Adaptor    = gvg->Adaptor;

    pthis->Gvvariation = ensGvvariationNewRef(gvg->Gvvariation);

    /* NOTE: Copy the AJAX List of AJAX String objects */

    if (gvg->Alleles)
    {
        pthis->Alleles = ajListstrNew();

        iter = ajListIterNew(gvg->Alleles);

        while (!ajListIterDone(iter))
        {
            allele = ajListstrIterGet(iter);

            ajListstrPushAppend(pthis->Alleles, allele);
        }

        ajListIterDel(&iter);
    }

    pthis->Subhandle = ajStrNewRef(gvg->Subhandle);

    pthis->Subidentifier = gvg->Subidentifier;

    return pthis;
}




/* @func ensGvgenotypeNewIni **************************************************
**
** Constructor for an Ensembl Genetic Variation Genotype with initial values.
**
** @cc Bio::EnsEMBL::Storable::new
** @param [u] gvga [EnsPGvgenotypeadaptor]
** Ensembl Genetic Variation Genotype Adaptor
** @param [r] identifier [ajuint] SQL database-internal identifier
** @cc Bio::EnsEMBL::Variation::Genotype::new
** @param [u] gvv [EnsPGvvariation] Ensembl Genetic Variation Variation
** @param [u] alleles [AjPList] AJAX List of AJAX String objects
** @param [u] subhandle [AjPStr] Subhandle
** @param [r] subidentifier [ajuint] Subidentifier
**
** @return [EnsPGvgenotype] Ensembl Genetic Variation Genotype or NULL
**
** @release 6.4.0
** @@
******************************************************************************/

EnsPGvgenotype ensGvgenotypeNewIni(EnsPGvgenotypeadaptor gvga,
                                   ajuint identifier,
                                   EnsPGvvariation gvv,
                                   AjPList alleles,
                                   AjPStr subhandle,
                                   ajuint subidentifier)
{
    AjIList iter = NULL;

    AjPStr allele = NULL;

    EnsPGvgenotype gvg = NULL;

    AJNEW0(gvg);

    gvg->Use        = 1U;
    gvg->Identifier = identifier;
    gvg->Adaptor    = gvga;

    gvg->Gvvariation = ensGvvariationNewRef(gvv);

    /* NOTE: Copy the AJAX List of AJAX String objects. */

    if (alleles)
    {
        gvg->Alleles = ajListstrNew();

        iter = ajListIterNew(alleles);

        while (!ajListIterDone(iter))
        {
            allele = ajListstrIterGet(iter);

            ajListstrPushAppend(gvg->Alleles, allele);
        }

        ajListIterDel(&iter);
    }

    if (subhandle)
        gvg->Subhandle = ajStrNewS(subhandle);

    gvg->Subidentifier = subidentifier;

    return gvg;
}




/* @func ensGvgenotypeNewRef **************************************************
**
** Ensembl Object referencing function, which returns a pointer to the
** Ensembl Object passed in and increases its reference count.
**
** @param [u] gvg [EnsPGvgenotype] Ensembl Genetic Variation Genotype
**
** @return [EnsPGvgenotype] Ensembl Genetic Variation Genotype or NULL
**
** @release 6.2.0
** @@
******************************************************************************/

EnsPGvgenotype ensGvgenotypeNewRef(EnsPGvgenotype gvg)
{
    if (!gvg)
        return NULL;

    gvg->Use++;

    return gvg;
}




/* @section destructors *******************************************************
**
** Destruction destroys all internal data structures and frees the memory
** allocated for an Ensembl Genetic Variation Genotype object.
**
** @fdata [EnsPGvgenotype]
**
** @nam3rule Del Destroy (free) a Genetic Variation Genotype object
**
** @argrule * Pgvg [EnsPGvgenotype*] Ensembl Genetic Variation Genotype address
**
** @valrule * [void]
**
** @fcategory delete
******************************************************************************/




/* @func ensGvgenotypeDel *****************************************************
**
** Default destructor for an Ensembl Genetic Variation Genotype.
**
** @param [d] Pgvg [EnsPGvgenotype*] Ensembl Genetic Variation Genotype address
**
** @return [void]
**
** @release 6.2.0
** @@
******************************************************************************/

void ensGvgenotypeDel(EnsPGvgenotype *Pgvg)
{
    EnsPGvgenotype pthis = NULL;

    if (!Pgvg)
        return;

#if defined(AJ_DEBUG) && AJ_DEBUG >= 1
    if (ajDebugTest("ensGvgenotypeDel"))
    {
        ajDebug("ensGvgenotypeDel\n"
                "  *Pgvg %p\n",
                *Pgvg);

        ensGvgenotypeTrace(*Pgvg, 1);
    }
#endif /* defined(AJ_DEBUG) && AJ_DEBUG >= 1 */

    if (!*Pgvg)
        return;

    pthis = *Pgvg;

    pthis->Use--;

    if (pthis->Use)
    {
        *Pgvg = NULL;

        return;
    }

    ensGvvariationDel(&pthis->Gvvariation);

    ajListstrFree(&pthis->Alleles);

    ajStrDel(&pthis->Subhandle);

    AJFREE(pthis);

    *Pgvg = NULL;

    return;
}




/* @section member retrieval **************************************************
**
** Functions for returning members of an
** Ensembl Genetic Variation Genotype object.
**
** @fdata [EnsPGvgenotype]
**
** @nam3rule Get Return Genetic Variation Genotype attribute(s)
** @nam4rule Adaptor Return the Ensembl Genetic Variation Genotype Adaptor
** @nam4rule Alleles Return the ALAX List of AJAX String (allele) objects
** @nam4rule Gvvariation Return the Ensembl Genetic Variation Variation
** @nam4rule Identifier Return the SQL database-internal identifier
** @nam4rule Subhandle Return the subhandle
** @nam4rule Subidentifier Return the subidentifier
**
** @argrule * gvg [const EnsPGvgenotype] Ensembl Genetic Variation Genotype
**
** @valrule Adaptor [EnsPGvgenotypeadaptor]
** Ensembl Genetic Variation Genotype Adaptor or NULL
** @valrule Alleles [const AjPList] AJAX List of AJAX String objects or NULL
** @valrule Gvvariation [EnsPGvvariation]
** Ensembl Genetic Variation Variation or NULL
** @valrule Identifier [ajuint] SQL database-internal identifier or 0U
** @valrule Subhandle [AjPStr] Subhandle or NULL
** @valrule Subidentifier [ajuint] Subidentifier
**
** @fcategory use
******************************************************************************/




/* @func ensGvgenotypeGetAdaptor **********************************************
**
** Get the Ensembl Genetic Variation Genotype Adaptor member of an
** Ensembl Genetic Variation Genotype.
**
** @param [r] gvg [const EnsPGvgenotype] Ensembl Genetic Variation Genotype
**
** @return [EnsPGvgenotypeadaptor] Ensembl Genetic Variation Genotype Adaptor
** or NULL
**
** @release 6.2.0
** @@
******************************************************************************/

EnsPGvgenotypeadaptor ensGvgenotypeGetAdaptor(const EnsPGvgenotype gvg)
{
    return (gvg) ? gvg->Adaptor : NULL;
}




/* @func ensGvgenotypeGetAlleles **********************************************
**
** Get the AJAX List of AJAX String (allele) objects of an
** Ensembl Genetic Variation Genotype.
**
** @param [r] gvg [const EnsPGvgenotype] Ensembl Genetic Variation Genotype
**
** @return [const AjPList] AJAX List of AJAX String objects or NULL
**
** @release 6.2.0
** @@
******************************************************************************/

const AjPList ensGvgenotypeGetAlleles(const EnsPGvgenotype gvg)
{
    return (gvg) ? gvg->Alleles : NULL;
}




/* @func ensGvgenotypeGetGvvariation ******************************************
**
** Get the Ensembl Genetic Variation Variation member of an
** Ensembl Genetic Variation Genotype.
**
** @param [r] gvg [const EnsPGvgenotype] Ensembl Genetic Variation Genotype
**
** @return [EnsPGvvariation] Ensembl Genetic Variation Variation or NULL
**
** @release 6.2.0
** @@
******************************************************************************/

EnsPGvvariation ensGvgenotypeGetGvvariation(const EnsPGvgenotype gvg)
{
    return (gvg) ? gvg->Gvvariation : NULL;
}




/* @func ensGvgenotypeGetIdentifier *******************************************
**
** Get the SQL database-internal identifier member of an
** Ensembl Genetic Variation Genotype.
**
** @param [r] gvg [const EnsPGvgenotype] Ensembl Genetic Variation Genotype
**
** @return [ajuint] SQL database-internal identifier or 0U
**
** @release 6.2.0
** @@
******************************************************************************/

ajuint ensGvgenotypeGetIdentifier(const EnsPGvgenotype gvg)
{
    return (gvg) ? gvg->Identifier : 0U;
}




/* @func ensGvgenotypeGetSubhandle ********************************************
**
** Get the subhandle of an Ensembl Genetic Variation Genotype.
**
** @param [r] gvg [const EnsPGvgenotype] Ensembl Genetic Variation Genotype
**
** @return [AjPStr] Subhandle or NULL
**
** @release 6.2.0
** @@
******************************************************************************/

AjPStr ensGvgenotypeGetSubhandle(const EnsPGvgenotype gvg)
{
    return (gvg) ? gvg->Subhandle : NULL;
}




/* @func ensGvgenotypeGetSubidentifier ****************************************
**
** Get the subidentifier member of an
** Ensembl Genetic Variation Genotype.
**
** @param [r] gvg [const EnsPGvgenotype] Ensembl Genetic Variation Genotype
**
** @return [ajuint] subidentifier or 0U
**
** @release 6.2.0
** @@
******************************************************************************/

ajuint ensGvgenotypeGetSubidentifier(const EnsPGvgenotype gvg)
{
    return (gvg) ? gvg->Subidentifier : 0U;
}




/* @section member assignment *************************************************
**
** Functions for assigning members of an
** Ensembl Genetic Variation Genotype object.
**
** @fdata [EnsPGvgenotype]
**
** @nam3rule Set Set one member of a Genetic Variation Genotype
** @nam4rule Adaptor Set the Ensembl Genetic Variation Genotype Adaptor
** @nam4rule Gvvariation Set the Ensembl Genetic Variation Variation
** @nam4rule Identifier Set the SQL database-internal identifier
** @nam4rule Subhandle Set the subhandle
** @nam4rule Subidentifier Set the subidentifier
**
** @argrule * gvg [EnsPGvgenotype] Ensembl Genetic Variation Genotype object
** @argrule Adaptor gvga [EnsPGvgenotypeadaptor]
** Ensembl Genetic Variation Genotype Adaptor
** @argrule Gvvariation gvv [EnsPGvvariation]
** Ensembl Genetic Variation Variation
** @argrule Identifier identifier [ajuint] SQL database-internal identifier
** @argrule Subhandle subhandle [AjPStr] Subhandle
** @argrule Subidentifier subidentifier [ajuint] Subidentifier
**
** @valrule * [AjBool] ajTrue upon success, ajFalse otherwise
**
** @fcategory modify
******************************************************************************/




/* @func ensGvgenotypeSetAdaptor **********************************************
**
** Set the Ensembl Genetic Variation Genotype Adaptor member of an
** Ensembl Genetic Variation Genotype.
**
** @param [u] gvg [EnsPGvgenotype] Ensembl Genetic Variation Genotype
** @param [u] gvga [EnsPGvgenotypeadaptor]
** Ensembl Genetic Variation Genotype Adaptor
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
**
** @release 6.2.0
** @@
******************************************************************************/

AjBool ensGvgenotypeSetAdaptor(EnsPGvgenotype gvg,
                               EnsPGvgenotypeadaptor gvga)
{
    if (!gvg)
        return ajFalse;

    gvg->Adaptor = gvga;

    return ajTrue;
}




/* @func ensGvgenotypeSetGvvariation ******************************************
**
** Set the Ensembl Genetic Variation Variation member of an
** Ensembl Genetic Variation Genotype.
**
** @param [u] gvg [EnsPGvgenotype] Ensembl Genetic Variation Genotype
** @param [u] gvv [EnsPGvvariation] Ensembl Genetic Variation Variation
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
**
** @release 6.2.0
** @@
******************************************************************************/

AjBool ensGvgenotypeSetGvvariation(EnsPGvgenotype gvg,
                                   EnsPGvvariation gvv)
{
    if (!gvg)
        return ajFalse;

    ensGvvariationDel(&gvg->Gvvariation);

    gvg->Gvvariation = ensGvvariationNewRef(gvv);

    return ajTrue;
}




/* @func ensGvgenotypeSetIdentifier *******************************************
**
** Set the SQL database-internal identifier member of an
** Ensembl Genetic Variation Genotype.
**
** @param [u] gvg [EnsPGvgenotype] Ensembl Genetic Variation Genotype
** @param [r] identifier [ajuint] SQL database-internal identifier
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
**
** @release 6.2.0
** @@
******************************************************************************/

AjBool ensGvgenotypeSetIdentifier(EnsPGvgenotype gvg,
                                  ajuint identifier)
{
    if (!gvg)
        return ajFalse;

    gvg->Identifier = identifier;

    return ajTrue;
}




/* @func ensGvgenotypeSetSubhandle ********************************************
**
** Set the subhandle member of an
** Ensembl Genetic Variation Genotype.
**
** @param [u] gvg [EnsPGvgenotype] Ensembl Genetic Variation Genotype
** @param [u] subhandle [AjPStr] Subhandle
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
**
** @release 6.2.0
** @@
******************************************************************************/

AjBool ensGvgenotypeSetSubhandle(EnsPGvgenotype gvg,
                                 AjPStr subhandle)
{
    if (!gvg)
        return ajFalse;

    ajStrDel(&gvg->Subhandle);

    gvg->Subhandle = ajStrNewRef(subhandle);

    return ajTrue;
}




/* @func ensGvgenotypeSetSubidentifier ****************************************
**
** Set the subidentifier member of an
** Ensembl Genetic Variation Genotype.
**
** @param [u] gvg [EnsPGvgenotype] Ensembl Genetic Variation Genotype
** @param [r] subidentifier [ajuint] Subidentifier
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
**
** @release 6.2.0
** @@
******************************************************************************/

AjBool ensGvgenotypeSetSubidentifier(EnsPGvgenotype gvg,
                                     ajuint subidentifier)
{
    if (!gvg)
        return ajFalse;

    gvg->Subidentifier = subidentifier;

    return ajTrue;
}




/* @section member addition ***************************************************
**
** Functions for adding members to an
** Ensembl Genetic Variation Genotype object.
**
** @fdata [EnsPGvgenotype]
**
** @nam3rule Add Add one object to an Ensembl Genetic Variation Genotype
** @nam4rule Allele Add an allele
** @nam4rule Gvgenotypecode Add all alleles of an
** Ensembl Genetic Variation Genotype Code
**
** @argrule * gvg [EnsPGvgenotype] Ensembl Genetic Variation Genotype
** @argrule Allele allele [AjPStr] Allele
** @argrule Gvgenotypecode gvgc [EnsPGvgenotypecode]
** Ensembl Genetic Variation Genotype Code
**
** @valrule * [AjBool] ajTrue upon success, ajFalse otherwise
**
** @fcategory modify
******************************************************************************/




/* @func ensGvgenotypeAddAllele ***********************************************
**
** Add an allele to an Ensembl Genetic Variation Genotype.
**
** @param [u] gvg [EnsPGvgenotype] Ensembl Genetic Variation Genotype
** @param [u] allele [AjPStr] Allele
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
**
** @release 6.5.0
** @@
******************************************************************************/

AjBool ensGvgenotypeAddAllele(EnsPGvgenotype gvg,
                              AjPStr allele)
{
    if (!gvg)
        return ajFalse;

    if (!allele)
        return ajFalse;

    if (!gvg->Alleles)
        gvg->Alleles = ajListstrNew();

    ajListstrPushAppend(gvg->Alleles, ajStrNewRef(allele));

    return ajTrue;
}




/* @func ensGvgenotypeAddGvgenotypecode ***************************************
**
** Add alleles from an Ensembl Genetic Variation Genotype Code to an
** Ensembl Genetic Variation Genotype.
**
** @param [u] gvg [EnsPGvgenotype]
** Ensembl Genetic Variation Genotype
** @param [u] gvgc [EnsPGvgenotypecode]
** Ensembl Genetic Variation Genotype Code
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
**
** @release 6.5.0
** @@
******************************************************************************/

AjBool ensGvgenotypeAddGvgenotypecode(EnsPGvgenotype gvg,
                                      EnsPGvgenotypecode gvgc)
{
    AjIList iter = NULL;

    AjPStr allele = NULL;

    if (!gvg)
        return ajFalse;

    if (!gvgc)
        return ajFalse;

    if (!gvg->Alleles)
        gvg->Alleles = ajListstrNew();

    iter = ajListIterNewread(ensGvgenotypecodeGetAlleles(gvgc));

    while (!ajListIterDone(iter))
    {
        allele = ajListstrIterGet(iter);

        if (allele)
            ajListstrPushAppend(gvg->Alleles, ajStrNewRef(allele));
    }

    ajListIterDel(&iter);

    return ajTrue;
}




/* @section debugging *********************************************************
**
** Functions for reporting of an Ensembl Genetic Variation Genotype object.
**
** @fdata [EnsPGvgenotype]
**
** @nam3rule Trace Report Ensembl Genetic Variation Genotype members to
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
**
** @release 6.2.0
** @@
******************************************************************************/

AjBool ensGvgenotypeTrace(const EnsPGvgenotype gvg, ajuint level)
{
    AjIList iter = NULL;

    AjPStr allele = NULL;
    AjPStr indent = NULL;

    if (!gvg)
        return ajFalse;

    indent = ajStrNew();

    ajStrAppendCountK(&indent, ' ', level * 2);

    ajDebug("%SensGvgenotypeTrace %p\n"
            "%S  Use %u\n"
            "%S  Identifier %u\n"
            "%S  Adaptor %p\n"
            "%S  Gvvariation %p\n"
            "%S  Alleles %p\n"
            "%S  Subhandle '%S'\n"
            "%S  Subidentifier %u\n",
            indent, gvg,
            indent, gvg->Use,
            indent, gvg->Identifier,
            indent, gvg->Adaptor,
            indent, gvg->Gvvariation,
            indent, gvg->Alleles,
            indent, gvg->Subhandle,
            indent, gvg->Subidentifier);

    ensGvvariationTrace(gvg->Gvvariation, level + 1);

    iter = ajListIterNew(gvg->Alleles);

    while (!ajListIterDone(iter))
    {
        allele = ajListstrIterGet(iter);

        ajDebug("%S    Allele '%S'\n", indent, allele);
    }

    ajListIterDel(&iter);

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
**
** @release 6.4.0
** @@
******************************************************************************/

size_t ensGvgenotypeCalculateMemsize(const EnsPGvgenotype gvg)
{
    size_t size = 0;

    AjIList iter = NULL;

    AjPStr allele = NULL;

    if (!gvg)
        return 0;

    size += sizeof (EnsOGvgenotype);

    size += sizeof (AjOList);

    iter = ajListIterNew(gvg->Alleles);

    while (!ajListIterDone(iter))
    {
        allele = ajListstrIterGet(iter);

        if (allele)
        {
            size += sizeof (AjOStr);

            size += ajStrGetRes(allele);
        }
    }

    ajListIterDel(&iter);

    if (gvg->Subhandle)
    {
        size += sizeof (AjOStr);

        size += ajStrGetRes(gvg->Subhandle);
    }

    return size;
}




/* @section fetch *********************************************************
**
** Functions for fetching values of an
** Ensembl Genetic Variation Genotype object.
**
** @fdata [EnsPGvgenotype]
**
** @nam3rule Fetch Fetch Ensembl Genetic Variation Genotype values
** @nam4rule Allele Fetch an allele
** @nam5rule Number Fetch the allele with number
** @nam5rule String Fetch the allel string
**
** @argrule * gvg [const EnsPGvgenotype] Ensembl Genetic Variation Genotype
** @argrule AlleleNumber number [ajuint] Number
** @argrule AlleleNumber Pallele [AjPStr*] Allele
** @argrule AlleleString Pstring [AjPStr*] Allele string
**
** @valrule Allele [AjBool] ajTrue upon success, ajFalse otherwise
**
** @fcategory misc
******************************************************************************/




/* @func ensGvgenotypeFetchAlleleNumber ***************************************
**
** Fetch the allele with number of an Ensembl Genetic Variation Genotype.
**
** @param [r] gvg [const EnsPGvgenotype] Ensembl Genetic Variation Genotype
** @param [r] number [ajuint] Number
** @param [u] Pallele [AjPStr*] Allele
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
**
** @release 6.2.0
** @@
******************************************************************************/

AjBool ensGvgenotypeFetchAlleleNumber(const EnsPGvgenotype gvg,
                                      ajuint number,
                                      AjPStr *Pallele)
{
    AjPStr allele = NULL;

    if (!gvg)
        return ajFalse;

    if (!Pallele)
        return ajFalse;

    if (*Pallele)
        ajStrAssignClear(Pallele);
    else
        *Pallele = ajStrNew();

    ajListPeekNumber(gvg->Alleles, number, (void **) &allele);

    ajStrAssignS(Pallele, allele);

    return ajTrue;
}




/* @func ensGvgenotypeFetchAlleleString ***************************************
**
** Fetch a string of alleles separated by '|' characters of an
** Ensembl Genetic Variation Genotype.
**
** @param [r] gvg [const EnsPGvgenotype] Ensembl Genetic Variation Genotype
** @param [u] Pstring [AjPStr*] Allele string
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
**
** @release 6.2.0
** @@
******************************************************************************/

AjBool ensGvgenotypeFetchAlleleString(const EnsPGvgenotype gvg,
                                      AjPStr *Pstring)
{
    AjIList iter = NULL;

    AjPStr allele = NULL;

    if (!gvg)
        return ajFalse;

    if (!Pstring)
        return ajFalse;

    if (*Pstring)
        ajStrAssignClear(Pstring);
    else
        *Pstring = ajStrNew();

    iter = ajListIterNew(gvg->Alleles);

    while (!ajListIterDone(iter))
    {
        allele = ajListstrIterGet(iter);

        ajStrAppendS(Pstring, allele);

        if (!ajListIterDone(iter))
            ajStrAppendK(Pstring, '|');
    }

    ajListIterDel(&iter);

    return ajTrue;
}




/* @datasection [EnsPGvgenotypecode] Ensembl Genetic Variation Genotype Code **
**
** @nam2rule Gvgenotypecode Functions for manipulating
** Ensembl Genetic Variation Genotype Code objects
**
** @cc Bio::EnsEMBL::Variation::GenotypeCode
** @cc CVS Revision: 1.2
** @cc CVS Tag: branch-ensembl-66
**
******************************************************************************/




/* @section constructors ******************************************************
**
** All constructors return a new Ensembl Genetic Variation Genotype Code
** by pointer.
** It is the responsibility of the user to first destroy any previous
** Genetic Variation Genotype Code. The target pointer does not need to be
** initialised to NULL, but it is good programming practice to do so anyway.
**
** @fdata [EnsPGvgenotypecode]
**
** @nam3rule New Constructor
** @nam4rule Cpy Constructor with existing object
** @nam4rule Ini Constructor with initial values
** @nam4rule Ref Constructor by incrementing the reference counter
**
** @argrule Cpy gvgc [const EnsPGvgenotypecode]
** Ensembl Genetic Variation Genotype Code
** @argrule Ini gvgca [EnsPGvgenotypecodeadaptor]
** Ensembl Genetic Variation Genotype Code Adaptor
** @argrule Ini identifier [ajuint] SQL database-internal identifier
** @argrule Ini alleles [AjPList] AJAX List of AJAX String (allele) objects
** @argrule Ref gvgc [EnsPGvgenotypecode]
** Ensembl Genetic Variation Genotype Code
**
** @valrule * [EnsPGvgenotypecode]
** Ensembl Genetic Variation Genotype Code or NULL
**
** @fcategory new
******************************************************************************/




/* @func ensGvgenotypecodeNewCpy **********************************************
**
** Object-based constructor function, which returns an independent object.
**
** @param [r] gvgc [const EnsPGvgenotypecode]
** Ensembl Genetic Variation Genotype Code
**
** @return [EnsPGvgenotypecode]
** Ensembl Genetic Variation Genotype Code or NULL
**
** @release 6.5.0
** @@
******************************************************************************/

EnsPGvgenotypecode ensGvgenotypecodeNewCpy(const EnsPGvgenotypecode gvgc)
{
    AjIList iter = NULL;

    AjPStr allele = NULL;

    EnsPGvgenotypecode pthis = NULL;

    if (!gvgc)
        return NULL;

    AJNEW0(pthis);

    pthis->Use        = 1U;
    pthis->Identifier = gvgc->Identifier;
    pthis->Adaptor    = gvgc->Adaptor;

    /* NOTE: Copy the AJAX List of AJAX String (allele) objects */

    if (gvgc->Alleles && ajListGetLength(gvgc->Alleles))
    {
        pthis->Alleles = ajListstrNew();

        iter = ajListIterNew(gvgc->Alleles);

        while (!ajListIterDone(iter))
        {
            allele = ajListstrIterGet(iter);

            if (allele)
                ajListstrPushAppend(pthis->Alleles, ajStrNewS(allele));
        }

        ajListIterDel(&iter);
    }

    return pthis;
}




/* @func ensGvgenotypecodeNewIni **********************************************
**
** Constructor for an Ensembl Genetic Variation Genotype Code
** with initial values.
**
** @cc Bio::EnsEMBL::Storable::new
** @param [u] gvgca [EnsPGvgenotypecodeadaptor]
** Ensembl Genetic Variation Genotype Code Adaptor
** @param [r] identifier [ajuint] SQL database-internal identifier
** @cc Bio::EnsEMBL::Variation::GenotypeCode::new
** @param [u] alleles [AjPList] AJAX List of AJAX String (allele) objects
**
** @return [EnsPGvgenotypecode]
** Ensembl Genetic Variation Genotype Code or NULL
**
** @release 6.5.0
** @@
******************************************************************************/

EnsPGvgenotypecode ensGvgenotypecodeNewIni(EnsPGvgenotypecodeadaptor gvgca,
                                           ajuint identifier,
                                           AjPList alleles)
{
    AjIList iter = NULL;

    AjPStr allele = NULL;

    EnsPGvgenotypecode gvgc = NULL;

    AJNEW0(gvgc);

    gvgc->Use        = 1U;
    gvgc->Identifier = identifier;
    gvgc->Adaptor    = gvgca;

    /* NOTE: Copy the AJAX List of AJAX String (allele) objects */

    if (alleles && ajListGetLength(alleles))
    {
        gvgc->Alleles = ajListstrNew();

        iter = ajListIterNew(alleles);

        while (!ajListIterDone(iter))
        {
            allele = ajListstrIterGet(iter);

            if (allele)
                ajListstrPushAppend(gvgc->Alleles, ajStrNewS(allele));
        }

        ajListIterDel(&iter);
    }

    return gvgc;
}




/* @func ensGvgenotypecodeNewRef **********************************************
**
** Ensembl Object referencing function, which returns a pointer to the
** Ensembl Object passed in and increases its reference count.
**
** @param [u] gvgc [EnsPGvgenotypecode]
** Ensembl Genetic Variation Genotype Code
**
** @return [EnsPGvgenotypecode]
** Ensembl Genetic Variation Genotype Code or NULL
**
** @release 6.5.0
** @@
******************************************************************************/

EnsPGvgenotypecode ensGvgenotypecodeNewRef(EnsPGvgenotypecode gvgc)
{
    if (!gvgc)
        return NULL;

    gvgc->Use++;

    return gvgc;
}




/* @section destructors *******************************************************
**
** Destruction destroys all internal data structures and frees the memory
** allocated for an Ensembl Genetic Variation Genotype Code object.
**
** @fdata [EnsPGvgenotypecode]
**
** @nam3rule Del Destroy (free) a Genetic Variation Genotype Code object
**
** @argrule * Pgvgc [EnsPGvgenotypecode*]
** Ensembl Genetic Variation Genotype Code address
**
** @valrule * [void]
**
** @fcategory delete
******************************************************************************/




/* @func ensGvgenotypecodeDel *************************************************
**
** Default destructor for an Ensembl Genetic Variation Genotype Code.
**
** @param [d] Pgvgc [EnsPGvgenotypecode*]
** Ensembl Genetic Variation Genotype Code address
**
** @return [void]
**
** @release 6.5.0
** @@
******************************************************************************/

void ensGvgenotypecodeDel(EnsPGvgenotypecode *Pgvgc)
{
    EnsPGvgenotypecode pthis = NULL;

    if (!Pgvgc)
        return;

#if defined(AJ_DEBUG) && AJ_DEBUG >= 1
    if (ajDebugTest("ensGvgenotypecodeDel"))
    {
        ajDebug("ensGvgenotypecodeDel\n"
                "  *Pgvgc %p\n",
                *Pgvgc);

        ensGvgenotypecodeTrace(*Pgvgc, 1);
    }
#endif /* defined(AJ_DEBUG) && AJ_DEBUG >= 1 */

    if (!*Pgvgc)
        return;

    pthis = *Pgvgc;

    pthis->Use--;

    if (pthis->Use)
    {
        *Pgvgc = NULL;

        return;
    }

    ajListstrFreeData(&pthis->Alleles);

    AJFREE(pthis);

    *Pgvgc = NULL;

    return;
}




/* @section member retrieval **************************************************
**
** Functions for returning members of an
** Ensembl Genetic Variation Genotype Code object.
**
** @fdata [EnsPGvgenotypecode]
**
** @nam3rule Get Return Genetic Variation Genotype Code attribute(s)
** @nam4rule Adaptor Return the Ensembl Genetic Variation Genotype Code Adaptor
** @nam4rule Alleles Return the alleles
** @nam4rule Identifier Return the SQL database-internal identifier
**
** @argrule * gvgc [const EnsPGvgenotypecode]
** Ensembl Genetic Variation Genotype Code
**
** @valrule Adaptor [EnsPGvgenotypecodeadaptor]
** Ensembl Genetic Variation Genotype Code Adaptor or NULL
** @valrule Alleles [const AjPList]
** AJAX List of AJAX String (allele) objects or NULL
** @valrule Identifier [ajuint] SQL database-internal identifier or 0U
**
** @fcategory use
******************************************************************************/




/* @func ensGvgenotypecodeGetAdaptor ******************************************
**
** Get the Ensembl Genetic Variation Genotype Code Adaptor member of an
** Ensembl Genetic Variation Genotype Code.
**
** @param [r] gvgc [const EnsPGvgenotypecode]
** Ensembl Genetic Variation Genotype Code
**
** @return [EnsPGvgenotypecodeadaptor]
** Ensembl Genetic Variation Genotype Code Adaptor
** or NULL
**
** @release 6.5.0
** @@
******************************************************************************/

EnsPGvgenotypecodeadaptor ensGvgenotypecodeGetAdaptor(
    const EnsPGvgenotypecode gvgc)
{
    return (gvgc) ? gvgc->Adaptor : NULL;
}




/* @func ensGvgenotypecodeGetAlleles ******************************************
**
** Get the alleles member of an
** Ensembl Genetic Variation Genotype Code.
**
** @param [r] gvgc [const EnsPGvgenotypecode]
** Ensembl Genetic Variation Genotype Code
**
** @return [const AjPList] AJAX List of AJAX String (allele) objects or NULL
**
** @release 6.5.0
** @@
******************************************************************************/

const AjPList ensGvgenotypecodeGetAlleles(
    const EnsPGvgenotypecode gvgc)
{
    return (gvgc) ? gvgc->Alleles : NULL;
}




/* @func ensGvgenotypecodeGetIdentifier ***************************************
**
** Get the SQL database-internal identifier member of an
** Ensembl Genetic Variation Genotype Code.
**
** @param [r] gvgc [const EnsPGvgenotypecode]
** Ensembl Genetic Variation Genotype Code
**
** @return [ajuint] SQL database-internal identifier or 0U
**
** @release 6.5.0
** @@
******************************************************************************/

ajuint ensGvgenotypecodeGetIdentifier(
    const EnsPGvgenotypecode gvgc)
{
    return (gvgc) ? gvgc->Identifier : 0U;
}




/* @section member assignment *************************************************
**
** Functions for assigning members of an
** Ensembl Genetic Variation Genotype Code object.
**
** @fdata [EnsPGvgenotypecode]
**
** @nam3rule Set Set one member of a Genetic Variation Genotype Code
** @nam4rule Adaptor Set the Ensembl Genetic Variation Genotype Code Adaptor
** @nam4rule Genotype Set the genotype
** @nam4rule Identifier Set the SQL database-internal identifier
**
** @argrule * gvgc [EnsPGvgenotypecode]
** Ensembl Genetic Variation Genotype Code object
** @argrule Adaptor gvgca [EnsPGvgenotypecodeadaptor]
** Ensembl Genetic Variation Genotype Code Adaptor
** @argrule Alleles genotype [AjPList]
** AJAX List of AJAX String (allele) objects
** @argrule Identifier identifier [ajuint] SQL database-internal identifier
**
** @valrule * [AjBool] ajTrue upon success, ajFalse otherwise
**
** @fcategory modify
******************************************************************************/




/* @func ensGvgenotypecodeSetAdaptor ******************************************
**
** Set the Ensembl Genetic Variation Genotype Code Adaptor member of an
** Ensembl Genetic Variation Genotype Code.
**
** @param [u] gvgc [EnsPGvgenotypecode]
** Ensembl Genetic Variation Genotype Code
** @param [u] gvgca [EnsPGvgenotypecodeadaptor]
** Ensembl Genetic Variation Genotype Code Adaptor
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
**
** @release 6.5.0
** @@
******************************************************************************/

AjBool ensGvgenotypecodeSetAdaptor(EnsPGvgenotypecode gvgc,
                                   EnsPGvgenotypecodeadaptor gvgca)
{
    if (!gvgc)
        return ajFalse;

    gvgc->Adaptor = gvgca;

    return ajTrue;
}




/* @func ensGvgenotypecodeSetIdentifier ***************************************
**
** Set the SQL database-internal identifier member of an
** Ensembl Genetic Variation Genotype Code.
**
** @param [u] gvgc [EnsPGvgenotypecode]
** Ensembl Genetic Variation Genotype Code
** @param [r] identifier [ajuint] SQL database-internal identifier
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
**
** @release 6.5.0
** @@
******************************************************************************/

AjBool ensGvgenotypecodeSetIdentifier(EnsPGvgenotypecode gvgc,
                                      ajuint identifier)
{
    if (!gvgc)
        return ajFalse;

    gvgc->Identifier = identifier;

    return ajTrue;
}




/* @section member addition ***************************************************
**
** Functions for adding members to an
** Ensembl Genetic Variation Genotype Code object.
**
** @fdata [EnsPGvgenotypecode]
**
** @nam3rule Add Add one object to an Ensembl Genetic Variation Genotype Code
** @nam4rule Allele Add an allele
**
** @argrule * gvgc [EnsPGvgenotypecode] Ensembl Genetic Variation Genotype Code
** @argrule Allele allele [AjPStr] Allele
**
** @valrule * [AjBool] ajTrue upon success, ajFalse otherwise
**
** @fcategory modify
******************************************************************************/




/* @func ensGvgenotypecodeAddAllele *******************************************
**
** Add an allele to an Ensembl Genetic Variation Genotype Code.
**
** @param [u] gvgc [EnsPGvgenotypecode]
** Ensembl Genetic Variation Genotype Code
** @param [u] allele [AjPStr] Allele
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
**
** @release 6.5.0
** @@
******************************************************************************/

AjBool ensGvgenotypecodeAddAllele(EnsPGvgenotypecode gvgc, AjPStr allele)
{
    if (!gvgc)
        return ajFalse;

    if (!allele)
        return ajFalse;

    if (!gvgc->Alleles)
        gvgc->Alleles = ajListstrNew();

    ajListstrPushAppend(gvgc->Alleles, allele);

    return ajTrue;
}




/* @section debugging *********************************************************
**
** Functions for reporting of an
** Ensembl Genetic Variation Genotype Code object.
**
** @fdata [EnsPGvgenotypecode]
**
** @nam3rule Trace Report Ensembl Genetic Variation Genotype Code members to
**                 debug file
**
** @argrule Trace gvgc [const EnsPGvgenotypecode]
** Ensembl Genetic Variation Genotype Code
** @argrule Trace level [ajuint] Indentation level
**
** @valrule * [AjBool] ajTrue upon success, ajFalse otherwise
**
** @fcategory misc
******************************************************************************/




/* @func ensGvgenotypecodeTrace ***********************************************
**
** Trace an Ensembl Genetic Variation Genotype Code.
**
** @param [r] gvgc [const EnsPGvgenotypecode]
** Ensembl Genetic Variation Genotype Code
** @param [r] level [ajuint] Indentation level
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
**
** @release 6.5.0
** @@
******************************************************************************/

AjBool ensGvgenotypecodeTrace(const EnsPGvgenotypecode gvgc, ajuint level)
{
    AjIList iter = NULL;

    AjPStr allele = NULL;
    AjPStr indent = NULL;

    if (!gvgc)
        return ajFalse;

    indent = ajStrNew();

    ajStrAppendCountK(&indent, ' ', level * 2);

    ajDebug("%SensGvgenotypecodeTrace %p\n"
            "%S  Use %u\n"
            "%S  Identifier %u\n"
            "%S  Adaptor %p\n"
            "%S  Alleles %p\n",
            indent, gvgc,
            indent, gvgc->Use,
            indent, gvgc->Identifier,
            indent, gvgc->Adaptor,
            indent, gvgc->Alleles);

    iter = ajListIterNew(gvgc->Alleles);

    while (!ajListIterDone(iter))
    {
        allele = ajListstrIterGet(iter);

        ajDebug("%S    Allele '%S'\n", indent, allele);
    }

    ajListIterDel(&iter);

    ajStrDel(&indent);

    return ajTrue;
}




/* @section calculate *********************************************************
**
** Functions for calculating values of an
** Ensembl Genetic Variation Genotype Code object.
**
** @fdata [EnsPGvgenotypecode]
**
** @nam3rule Calculate Calculate Ensembl Genetic Variation Genotype Code values
** @nam4rule Memsize Calculate the memory size in bytes
**
** @argrule * gvgc [const EnsPGvgenotypecode]
** Ensembl Genetic Variation Genotype Code
**
** @valrule Memsize [size_t] Memory size in bytes or 0
**
** @fcategory misc
******************************************************************************/




/* @func ensGvgenotypecodeCalculateMemsize ************************************
**
** Calculate the memory size in bytes of an
** Ensembl Genetic Variation Genotype Code.
**
** @param [r] gvgc [const EnsPGvgenotypecode]
** Ensembl Genetic Variation Genotype Code
**
** @return [size_t] Memory size in bytes or 0
**
** @release 6.5.0
** @@
******************************************************************************/

size_t ensGvgenotypecodeCalculateMemsize(const EnsPGvgenotypecode gvgc)
{
    size_t size = 0;

    AjIList iter = NULL;

    AjPStr allele = NULL;

    if (!gvgc)
        return 0;

    size += sizeof (EnsOGvgenotypecode);

    size += sizeof (AjOList);

    iter = ajListIterNew(gvgc->Alleles);

    while (!ajListIterDone(iter))
    {
        allele = ajListstrIterGet(iter);

        if (allele)
        {
            size += sizeof (AjOStr);

            size += ajStrGetRes(allele);
        }
    }

    ajListIterDel(&iter);

    return size;
}




/* @datasection [EnsPGvgenotypecodeadaptor] Ensembl Genetic Variation Genotype
** Code Adaptor
**
** @nam2rule Gvgenotypecodeadaptor Functions for manipulating
** Ensembl Genetic Variation Genotype Code Adaptor objects
**
** @cc Bio::EnsEMBL::Variation::DBSQL::GenotypeCodeAdaptor
** @cc CVS Revision: 1.3
** @cc CVS Tag: branch-ensembl-66
**
******************************************************************************/




/* @funcstatic gvgenotypecodeadaptorFetchAllbyStatement ***********************
**
** Fetch all Ensembl Genetic Variation Genotype Code objects via an
** SQL statement.
**
** @cc Bio::EnsEMBL::Variation::DBSQL::GenotypeCodeAdaptor::
**     _objs_from_sth
** @param [u] ba [EnsPBaseadaptor] Ensembl Base Adaptor
** @param [r] statement [const AjPStr] SQL statement
** @param [uN] am [EnsPAssemblymapper] Ensembl Assembly Mapper
** @param [uN] slice [EnsPSlice] Ensembl Slice
** @param [u] gvgcs [AjPList] AJAX List of Ensembl Genetic Variation
**                            Genotype Code objects
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
**
** @release 6.5.0
** @@
******************************************************************************/

static AjBool gvgenotypecodeadaptorFetchAllbyStatement(
    EnsPBaseadaptor ba,
    const AjPStr statement,
    EnsPAssemblymapper am,
    EnsPSlice slice,
    AjPList gvgcs)
{
    register ajuint i = 0U;

    ajuint identifier    = 0U;
    ajuint haplotypeid   = 0U;
    ajuint ploidy        = 0U;

    AjPList gvhs = NULL;

    AjPSqlstatement sqls = NULL;
    AjISqlrow sqli       = NULL;
    AjPSqlrow sqlr       = NULL;

    AjPStr allele = NULL;

    EnsPDatabaseadaptor dba = NULL;

    EnsPGvgenotypecode        gvgc  = NULL;
    EnsPGvgenotypecodeadaptor gvgca = NULL;

    GvPHaplotype gvh = NULL;

    if (ajDebugTest("gvgenotypecodeadaptorFetchAllbyStatement"))
        ajDebug("gvgenotypecodeadaptorFetchAllbyStatement\n"
                "  ba %p\n"
                "  statement %p\n"
                "  am %p\n"
                "  slice %p\n"
                "  gvgcs %p\n",
                ba,
                statement,
                am,
                slice,
                gvgcs);

    if (!ba)
        return ajFalse;

    if (!statement)
        return ajFalse;

    if (!gvgcs)
        return ajFalse;

    gvhs = ajListNew();

    dba = ensBaseadaptorGetDatabaseadaptor(ba);

    gvgca = ensRegistryGetGvgenotypecodeadaptor(dba);

    ploidy = ensGvbaseadaptorLoadPloidy(gvgca);

    sqls = ensDatabaseadaptorSqlstatementNew(dba, statement);

    sqli = ajSqlrowiterNew(sqls);

    while (!ajSqlrowiterDone(sqli))
    {
        identifier  = 0U;
        haplotypeid = 0U;
        allele      = ajStrNew();

        sqlr = ajSqlrowiterGet(sqli);

        ajSqlcolumnToUint(sqlr, &identifier);
        ajSqlcolumnToUint(sqlr, &haplotypeid);
        ajSqlcolumnToStr(sqlr, &allele);

        if (ensGvgenotypecodeGetIdentifier(gvgc) != identifier)
        {
            /* Deal with haplotypes of the previous object. */

            listGvhaplotypeSortHaplotypeidentifierAscending(gvhs);

            i = 0U;

            while (ajListPop(gvhs, (void **) &gvh))
            {
                if (i < ploidy)
                    ensGvgenotypecodeAddAllele(gvgc, gvh->Allele);

                gvHaplotypeDel(&gvh);
            }

            /* New object. */

            gvgc = ensGvgenotypecodeNewIni(gvgca, identifier, (AjPList) NULL);

            ajListPushAppend(gvgcs, (void *) gvgc);
        }

        /*
        ** Add the allele under its haplotype identifier to the
        ** temporary AJAX List.
        */

        gvh = gvHaplotypeNewIni(haplotypeid, allele);

        ajListPushAppend(gvhs, (void *) gvh);

        ajStrDel(&allele);
    }

    ajSqlrowiterDel(&sqli);

    ensDatabaseadaptorSqlstatementDel(dba, &sqls);

    /* Add allele strings to the last object. */

    listGvhaplotypeSortHaplotypeidentifierAscending(gvhs);

    i = 0U;

    while (ajListPop(gvhs, (void **) &gvh))
    {
        if (i < ploidy)
            ensGvgenotypecodeAddAllele(gvgc, gvh->Allele);

        gvHaplotypeDel(&gvh);
    }

    ajListFree(&gvhs);

    return ajTrue;
}




/* @section constructors ******************************************************
**
** All constructors return a new Ensembl Genetic Variation Genotype Code
** Adaptor by pointer.
** It is the responsibility of the user to first destroy any previous
** Genetic Variation Genotype Code Adaptor.
** The target pointer does not need to be initialised to NULL,
** but it is good programming practice to do so anyway.
**
** @fdata [EnsPGvgenotypecodeadaptor]
**
** @nam3rule New Constructor
**
** @argrule New dba [EnsPDatabaseadaptor] Ensembl Database Adaptor
**
** @valrule * [EnsPGvgenotypecodeadaptor]
** Ensembl Genetic Variation Genotype Code Adaptor or NULL
**
** @fcategory new
******************************************************************************/




/* @func ensGvgenotypecodeadaptorNew ******************************************
**
** Default constructor for an
** Ensembl Genetic Variation Genotype Code Adaptor.
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
** @see ensRegistryGetGvgenotypecodeadaptor
**
** @cc Bio::EnsEMBL::Variation::DBSQL::GenotypeCodeAdaptor::new
** @param [u] dba [EnsPDatabaseadaptor] Ensembl Database Adaptor
**
** @return [EnsPGvgenotypecodeadaptor]
** Ensembl Genetic Variation Genotype Code Adaptor or NULL
**
** @release 6.5.0
** @@
******************************************************************************/

EnsPGvgenotypecodeadaptor ensGvgenotypecodeadaptorNew(
    EnsPDatabaseadaptor dba)
{
    EnsPGvdatabaseadaptor gvdba = NULL;

    if (ajDebugTest("ensGvgenotypecodeadaptorNew"))
        ajDebug("ensGvgenotypecodeadaptorNew\n"
                "  dba %p\n",
                dba);

    if (!dba)
        return NULL;

    gvdba = ensRegistryGetGvdatabaseadaptor(dba);

    if (!gvdba)
        return NULL;

    return ensGvbaseadaptorNew(gvdba,
                               gvgenotypecodeadaptorKTables,
                               gvgenotypecodeadaptorKColumns,
                               (const EnsPBaseadaptorLeftjoin) NULL,
                               gvgenotypecodeadaptorKDefaultcondition,
                               (const char *) NULL,
                               &gvgenotypecodeadaptorFetchAllbyStatement);
}




/* @section destructors *******************************************************
**
** Destruction destroys all internal data structures and frees the memory
** allocated for an Ensembl Genetic Variation Genotype Code Adaptor
** object.
**
** @fdata [EnsPGvgenotypecodeadaptor]
**
** @nam3rule Del Destroy (free) an
** Ensembl Genetic Variation Genotype Code Adaptor
**
** @argrule * Pgvgca [EnsPGvgenotypecodeadaptor*]
** Ensembl Genetic Variation Genotype Code Adaptor address
**
** @valrule * [void]
**
** @fcategory delete
******************************************************************************/




/* @func ensGvgenotypecodeadaptorDel ******************************************
**
** Default destructor for an
** Ensembl Genetic Variation Genotype Code Adaptor.
**
** Ensembl Object Adaptors are singleton objects that are registered in the
** Ensembl Registry and weakly referenced by Ensembl Objects that have been
** instantiated by it. Therefore, Ensembl Object Adaptors should never be
** destroyed directly. Upon exit, the Ensembl Registry will call this function
** if required.
**
** @param [d] Pgvgca [EnsPGvgenotypecodeadaptor*]
** Ensembl Genetic Variation Genotype Code Adaptor address
**
** @return [void]
**
** @release 6.5.0
** @@
******************************************************************************/

void ensGvgenotypecodeadaptorDel(
    EnsPGvgenotypecodeadaptor *Pgvgca)
{
    ensGvbaseadaptorDel(Pgvgca);

	return;
}




/* @section object retrieval **************************************************
**
** Functions for fetching Ensembl Genetic Variation Genotype Code objects
** from an Ensembl SQL database.
**
** @fdata [EnsPGvgenotypecodeadaptor]
**
** @nam3rule Fetch Fetch Ensembl Genetic Variation Genotype Code
** object(s)
** @nam4rule All   Fetch all Ensembl Genetic Variation Genotype Code
** objects
** @nam4rule Allby Fetch all Ensembl Genetic Variation Genotype Code
**                 objects matching a criterion
** @nam5rule Identifiers Fetch all by an AJAX Table of AJAX unsigned integer
** (Ensembl Genetic Variation Genotype Code identifier) key data and
** Ensembl Genetic Variation Genotype Code object value data
** @nam5rule Identifier Fetch by an SQL database-internal identifier
**
** @argrule * gvgca [EnsPGvgenotypecodeadaptor]
** Ensembl Genetic Variation Genotype Code Adaptor
** @argrule AllbyIdentifiers gvgcs [AjPTable] AJAX Table of
** AJAX unsigned integer
** (Ensembl Genetic Variation Genotype Code identifier) key data and
** Ensembl Genetic Variation Genotype Code object value data
** @argrule Identifier identifier [ajuint] SQL database-internal identifier
** @argrule By Pggcs [EnsPGvgenotypecode*]
** Ensembl Genetic Variation Genotype Code address
**
** @valrule * [AjBool] ajTrue upon success, ajFalse otherwise
**
** @fcategory use
******************************************************************************/




/* @func ensGvgenotypecodeadaptorFetchAllbyIdentifiers ************************
**
** Fetch all Ensembl Genetic Variation Genotype Code objects by an
** AJAX Table of AJAX unsigned integer
** (Ensembl Genetic Variation Genotype Code identifier) key data and
** Ensembl Genetic Variation Genotype Code object value data
**
** The caller is responsible for deleting the
** Ensembl Genetic Variation Population Genotype objects before deleting the
** AJAX Table.
**
** @cc Bio::EnsEMBL::Variation::DBSQL::GenotypeCodeAdaptor::
**     fetch_all_by_dbID_list
** @param [u] gvgca [EnsPGvgenotypecodeadaptor]
** Ensembl Genetic Variation Genotype Code Adaptor
** @param [u] gvgcs [AjPTable] AJAX Table of AJAX unsigned integer
** (Ensembl Genetic Variation Genotype Code identifier) and
** Ensembl Genetic Variation Genotype Code objects
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
**
** @release 6.5.0
** @@
******************************************************************************/

AjBool ensGvgenotypecodeadaptorFetchAllbyIdentifiers(
    EnsPGvgenotypecodeadaptor gvgca,
    AjPTable gvgcs)
{
    EnsPBaseadaptor ba = NULL;

    if (!gvgca)
        return ajFalse;

    if (!gvgcs)
        return ajFalse;

    ba = ensGvbaseadaptorGetBaseadaptor(gvgca);

    return ensBaseadaptorFetchAllbyIdentifiers(
        ba,
        (EnsPSlice) NULL,
        (ajuint (*)(const void *)) &ensGvgenotypecodeGetIdentifier,
        gvgcs);
}
