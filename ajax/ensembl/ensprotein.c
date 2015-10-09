/* @source ensprotein *********************************************************
**
** Ensembl Protein Feature functions
**
** @author Copyright (C) 1999 Ensembl Developers
** @author Copyright (C) 2006 Michael K. Schuster
** @version $Revision: 1.20 $
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

#include "ensprotein.h"




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

/* @conststatic proteinfeatureadaptorKTables **********************************
**
** Array of Ensembl Protein Feature Adaptor SQL table names
**
******************************************************************************/

static const char *const proteinfeatureadaptorKTables[] =
{
    "protein_feature",
    (const char *) NULL
};




/* @conststatic proteinfeatureadaptorKColumns *********************************
**
** Array of Ensembl Protein Feature Adaptor SQL column names
**
******************************************************************************/

static const char *const proteinfeatureadaptorKColumns[] =
{
    "protein_feature.protein_feature_id",
    "protein_feature.translation_id",
    "protein_feature.seq_start",
    "protein_feature.seq_end",
    "protein_feature.hit_name",
    "protein_feature.hit_start",
    "protein_feature.hit_end",
    "protein_feature.analysis_id",
    "protein_feature.score",
    "protein_feature.evalue",
    "protein_feature.perc_ident",
    "interpro.interpro_ac",
    "xref.display_label",
    (const char *) NULL
};




/* @conststatic proteinfeatureadaptorKLeftjoin ********************************
**
** Array of Ensembl Protein Feature Adaptor SQL left join conditions
**
******************************************************************************/

static const EnsOBaseadaptorLeftjoin proteinfeatureadaptorKLeftjoin[] =
{
    {"interpro", "protein_feature.hit_name = interpro.id"},
    {"xref", "interpro.interpro_ac = xref.dbprimary_acc"},
    {(const char *) NULL, (const char *) NULL}
};




/* ========================================================================= */
/* =========================== private variables =========================== */
/* ========================================================================= */




/* ========================================================================= */
/* =========================== private functions =========================== */
/* ========================================================================= */

static AjBool proteinfeatureadaptorFetchAllbyStatement(
    EnsPBaseadaptor ba,
    const AjPStr statement,
    EnsPAssemblymapper am,
    EnsPSlice slice,
    AjPList pfs);




/* ========================================================================= */
/* ======================= All functions by section ======================== */
/* ========================================================================= */




/* @filesection ensprotein ****************************************************
**
** @nam1rule ens Function belongs to the Ensembl library
**
******************************************************************************/




/* @datasection [EnsPProteinfeature] Ensembl Protein Feature ******************
**
** @nam2rule Proteinfeature Functions for manipulating
** Ensembl Protein Feature objects
**
** @cc Bio::EnsEMBL::ProteinFeature
** @cc CVS Revision: 1.18
** @cc CVS Tag: branch-ensembl-66
**
******************************************************************************/





/* @section constructors ******************************************************
**
** All constructors return a new Ensembl Protein Feature by pointer.
** It is the responsibility of the user to first destroy any previous
** Protein Feature. The target pointer does not need to be initialised to
** NULL, but it is good programming practice to do so anyway.
**
** @fdata [EnsPProteinfeature]
**
** @nam3rule New Constructor
** @nam4rule Cpy Constructor with existing object
** @nam4rule Ini Constructor with initial values
** @nam4rule Ref Constructor by incrementing the reference counter
**
** @argrule Cpy pf [const EnsPProteinfeature] Ensembl Protein Feature
** @argrule Ini pfa [EnsPProteinfeatureadaptor] Ensembl Protein Feature Adaptor
** @argrule Ini identifier [ajuint] SQL database-internal identifier
** @argrule Ini fp [EnsPFeaturepair] Ensembl Feature Pair
** @argrule Ini accession [AjPStr] (InterPro) Accession
** @argrule Ini description [AjPStr] (InterPro) Description
** @argrule Ref pf [EnsPProteinfeature] Ensembl Protein Feature
**
** @valrule * [EnsPProteinfeature] Ensembl Protein Feature or NULL
**
** @fcategory new
******************************************************************************/




/* @func ensProteinfeatureNewCpy **********************************************
**
** Object-based constructor function, which returns an independent object.
**
** @param [r] pf [const EnsPProteinfeature] Ensembl Protein Feature
**
** @return [EnsPProteinfeature] Ensembl Protein Feature or NULL
**
** @release 6.4.0
** @@
******************************************************************************/

EnsPProteinfeature ensProteinfeatureNewCpy(const EnsPProteinfeature pf)
{
    EnsPProteinfeature pthis = NULL;

    if (!pf)
        return NULL;

    AJNEW0(pthis);

    pthis->Use         = 1U;
    pthis->Adaptor     = pf->Adaptor;
    pthis->Featurepair = ensFeaturepairNewRef(pf->Featurepair);

    if (pf->Accession)
        pthis->Accession = ajStrNewRef(pf->Accession);

    if (pf->Description)
        pthis->Description = ajStrNewRef(pf->Description);

    return pthis;
}




/* @func ensProteinfeatureNewIni **********************************************
**
** Constructor of an Ensembl Protein Feature with initial values.
**
** @cc Bio::EnsEMBL::Storable
** @param [u] pfa [EnsPProteinfeatureadaptor] Ensembl Protein Feature Adaptor
** @param [r] identifier [ajuint] SQL database-internal identifier
** @cc Bio::EnsEMBL::FeaturePair
** @param [u] fp [EnsPFeaturepair] Ensembl Feature Pair
** @cc Bio::EnsEMBL::ProteinFeature
** @param [u] accession [AjPStr] (InterPro) Accession
** @param [u] description [AjPStr] (InterPro) Description
**
** @return [EnsPProteinfeature] Ensembl Protein Feature or NULL
**
** @release 6.4.0
** @@
******************************************************************************/

EnsPProteinfeature ensProteinfeatureNewIni(EnsPProteinfeatureadaptor pfa,
                                           ajuint identifier,
                                           EnsPFeaturepair fp,
                                           AjPStr accession,
                                           AjPStr description)
{
    EnsPProteinfeature pf = NULL;

    AJNEW0(pf);

    pf->Use         = 1U;
    pf->Identifier  = identifier;
    pf->Adaptor     = pfa;
    pf->Featurepair = ensFeaturepairNewRef(fp);

    if (accession)
        pf->Accession = ajStrNewRef(accession);

    if (description)
        pf->Description = ajStrNewRef(description);

    return pf;
}




/* @func ensProteinfeatureNewRef **********************************************
**
** Ensembl Object referencing function, which returns a pointer to the
** Ensembl Object passed in and increases its reference count.
**
** @param [u] pf [EnsPProteinfeature] Ensembl Protein Feature
**
** @return [EnsPProteinfeature] Ensembl Protein Feature or NULL
**
** @release 6.2.0
** @@
******************************************************************************/

EnsPProteinfeature ensProteinfeatureNewRef(EnsPProteinfeature pf)
{
    if (!pf)
        return NULL;

    pf->Use++;

    return pf;
}




/* @section destructors *******************************************************
**
** Destruction destroys all internal data structures and frees the memory
** allocated for an Ensembl Protein Feature object.
**
** @fdata [EnsPProteinfeature]
**
** @nam3rule Del Destroy (free) an Ensembl Protein Feature
**
** @argrule * Ppf [EnsPProteinfeature*] Ensembl Protein Feature address
**
** @valrule * [void]
**
** @fcategory delete
******************************************************************************/




/* @func ensProteinfeatureDel *************************************************
**
** Default destructor for an Ensembl Protein Feature.
**
** @param [d] Ppf [EnsPProteinfeature*] Ensembl Protein Feature address
**
** @return [void]
**
** @release 6.2.0
** @@
******************************************************************************/

void ensProteinfeatureDel(EnsPProteinfeature *Ppf)
{
    EnsPProteinfeature pthis = NULL;

    if (!Ppf)
        return;

#if defined(AJ_DEBUG) && AJ_DEBUG >= 1
    if (ajDebugTest("ensProteinfeatureDel"))
    {
        ajDebug("ensProteinfeatureDel\n"
                "  *Ppf %p\n",
                *Ppf);

        ensProteinfeatureTrace(*Ppf, 1);
    }
#endif /* defined(AJ_DEBUG) && AJ_DEBUG >= 1 */

    if (!*Ppf)
        return;

    pthis = *Ppf;

    pthis->Use--;

    if (pthis->Use)
    {
        *Ppf = NULL;

        return;
    }

    ensFeaturepairDel(&pthis->Featurepair);

    ajStrDel(&pthis->Accession);
    ajStrDel(&pthis->Description);

    AJFREE(pthis);

    *Ppf = NULL;

    return;
}




/* @section member retrieval **************************************************
**
** Functions for returning members of an Ensembl Protein Feature object.
**
** @fdata [EnsPProteinfeature]
**
** @nam3rule Get Return Protein Feature attribute(s)
** @nam4rule Accession Return the accession
** @nam4rule Adaptor Return the Ensembl Protein Feature Adaptor
** @nam4rule Description Return the description
** @nam4rule Featurepair Return the Ensembl Feature Pair
** @nam4rule Identifier Return the SQL database-internal identifier
**
** @argrule * pf [const EnsPProteinfeature] Protein Feature
**
** @valrule Accession [AjPStr] Accession or NULL
** @valrule Adaptor [EnsPProteinfeatureadaptor] Ensembl Protein Feature Adaptor
** or NULL
** @valrule Description [AjPStr] Description or NULL
** @valrule Featurepair [EnsPFeaturepair] Ensembl Feature Pair or NULL
** @valrule Identifier [ajuint] SQL database-internal identifier or 0U
**
** @fcategory use
******************************************************************************/




/* @func ensProteinfeatureGetAccession ****************************************
**
** Get the (InterPro) accession member of an Ensembl Protein Feature.
**
** @param [r] pf [const EnsPProteinfeature] Ensembl Protein Feature
**
** @return [AjPStr] (InterPro) accession or NULL
**
** @release 6.2.0
** @@
******************************************************************************/

AjPStr ensProteinfeatureGetAccession(
    const EnsPProteinfeature pf)
{
    return (pf) ? pf->Accession : NULL;
}




/* @func ensProteinfeatureGetAdaptor ******************************************
**
** Get the Ensembl Protein Feature Adaptor member of an
** Ensembl Protein Feature.
**
** @param [r] pf [const EnsPProteinfeature] Ensembl Protein Feature
**
** @return [EnsPProteinfeatureadaptor] Ensembl Protein Feature Adaptor or NULL
**
** @release 6.2.0
** @@
******************************************************************************/

EnsPProteinfeatureadaptor ensProteinfeatureGetAdaptor(
    const EnsPProteinfeature pf)
{
    return (pf) ? pf->Adaptor : NULL;
}




/* @func ensProteinfeatureGetDescription **************************************
**
** Get the (InterPro) description member of an Ensembl Protein Feature.
**
** @param [r] pf [const EnsPProteinfeature] Ensembl Protein Feature
**
** @return [AjPStr] (InterPro) description or NULL
**
** @release 6.2.0
** @@
******************************************************************************/

AjPStr ensProteinfeatureGetDescription(
    const EnsPProteinfeature pf)
{
    return (pf) ? pf->Description : NULL;
}




/* @func ensProteinfeatureGetFeaturepair **************************************
**
** Get the Ensembl Feature Pair member of an Ensembl Protein Feature.
**
** @param [r] pf [const EnsPProteinfeature] Ensembl Protein Feature
**
** @return [EnsPFeaturepair] Ensembl Feature Pair or NULL
**
** @release 6.2.0
** @@
******************************************************************************/

EnsPFeaturepair ensProteinfeatureGetFeaturepair(
    const EnsPProteinfeature pf)
{
    return (pf) ? pf->Featurepair : NULL;
}




/* @func ensProteinfeatureGetIdentifier ***************************************
**
** Get the SQL database-internal identifier member of an
** Ensembl Protein Feature.
**
** @param [r] pf [const EnsPProteinfeature] Ensembl Protein Feature
**
** @return [ajuint] SQL database-internal identifier or 0U
**
** @release 6.2.0
** @@
******************************************************************************/

ajuint ensProteinfeatureGetIdentifier(
    const EnsPProteinfeature pf)
{
    return (pf) ? pf->Identifier : 0U;
}




/* @section member assignment *************************************************
**
** Functions for assigning members of an Ensembl Protein Feature object.
**
** @fdata [EnsPProteinfeature]
**
** @nam3rule Set Set one member of a Protein Feature
** @nam4rule Accession Set the accession
** @nam4rule Adaptor Set the Ensembl Protein Feature Adaptor
** @nam4rule Description Set the description
** @nam4rule Featurepair Set the Ensembl Feature Pair
** @nam4rule Identifier Set the SQL database-internal identifier
**
** @argrule * pf [EnsPProteinfeature] Ensembl Protein Feature object
** @argrule Accession accession [AjPStr] Accession
** @argrule Adaptor pfa [EnsPProteinfeatureadaptor] Ensembl Protein
**                                                  Feature Adaptor
** @argrule Description description [AjPStr] Description
** @argrule Featurepair fp [EnsPFeaturepair] Ensembl Feature Pair
** @argrule Identifier identifier [ajuint] SQL database-internal identifier
**
** @valrule * [AjBool] ajTrue upon success, ajFalse otherwise
**
** @fcategory modify
******************************************************************************/




/* @func ensProteinfeatureSetAccession ****************************************
**
** Set the accession member of an Ensembl Protein Feature.
**
** @param [u] pf [EnsPProteinfeature] Ensembl Protein Feature
** @param [u] accession [AjPStr] Accession
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
**
** @release 6.2.0
** @@
******************************************************************************/

AjBool ensProteinfeatureSetAccession(EnsPProteinfeature pf,
                                     AjPStr accession)
{
    if (!pf)
        return ajFalse;

    ajStrDel(&pf->Accession);

    pf->Accession = ajStrNewRef(accession);

    return ajTrue;
}




/* @func ensProteinfeatureSetAdaptor ******************************************
**
** Set the Ensembl Protein Feature Adaptor member of an
** Ensembl Protein Feature.
**
** @param [u] pf [EnsPProteinfeature] Ensembl Protein Feature
** @param [u] pfa [EnsPProteinfeatureadaptor] Ensembl Protein Feature Adaptor
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
**
** @release 6.2.0
** @@
******************************************************************************/

AjBool ensProteinfeatureSetAdaptor(EnsPProteinfeature pf,
                                   EnsPProteinfeatureadaptor pfa)
{
    if (!pf)
        return ajFalse;

    pf->Adaptor = pfa;

    return ajTrue;
}




/* @func ensProteinfeatureSetDescription **************************************
**
** Set the description member of an Ensembl Protein Feature.
**
** @param [u] pf [EnsPProteinfeature] Ensembl Protein Feature
** @param [u] description [AjPStr] Description
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
**
** @release 6.2.0
** @@
******************************************************************************/

AjBool ensProteinfeatureSetDescription(EnsPProteinfeature pf,
                                       AjPStr description)
{
    if (!pf)
        return ajFalse;

    ajStrDel(&pf->Description);

    pf->Description = ajStrNewRef(description);

    return ajTrue;
}




/* @func ensProteinfeatureSetFeaturepair **************************************
**
** Set the Ensembl Feature Pair member of an Ensembl Protein Feature.
**
** @param [u] pf [EnsPProteinfeature] Ensembl Protein Feature
** @param [u] fp [EnsPFeaturepair] Ensembl Feature Pair
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
**
** @release 6.2.0
** @@
******************************************************************************/

AjBool ensProteinfeatureSetFeaturepair(EnsPProteinfeature pf,
                                       EnsPFeaturepair fp)
{
    if (!pf)
        return ajFalse;

    ensFeaturepairDel(&pf->Featurepair);

    pf->Featurepair = ensFeaturepairNewRef(fp);

    return ajTrue;
}




/* @func ensProteinfeatureSetIdentifier ***************************************
**
** Set the SQL database-internal identifier member of an
** Ensembl Protein Feature.
**
** @param [u] pf [EnsPProteinfeature] Ensembl Protein Feature
** @param [r] identifier [ajuint] SQL database-internal identifier
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
**
** @release 6.2.0
** @@
******************************************************************************/

AjBool ensProteinfeatureSetIdentifier(EnsPProteinfeature pf,
                                      ajuint identifier)
{
    if (!pf)
        return ajFalse;

    pf->Identifier = identifier;

    return ajTrue;
}




/* @section debugging *********************************************************
**
** Functions for reporting of an Ensembl Protein Feature object.
**
** @fdata [EnsPProteinfeature]
**
** @nam3rule Trace Report Ensembl Protein Feature members to debug file
**
** @argrule Trace pf [const EnsPProteinfeature] Ensembl Protein Feature
** @argrule Trace level [ajuint] Indentation level
**
** @valrule * [AjBool] ajTrue upon success, ajFalse otherwise
**
** @fcategory misc
******************************************************************************/




/* @func ensProteinfeatureTrace ***********************************************
**
** Trace an Ensembl Protein Feature.
**
** @param [r] pf [const EnsPProteinfeature] Ensembl Protein Feature
** @param [r] level [ajuint] Indentation level
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
**
** @release 6.2.0
** @@
******************************************************************************/

AjBool ensProteinfeatureTrace(const EnsPProteinfeature pf, ajuint level)
{
    AjPStr indent = NULL;

    if (!pf)
        return ajFalse;

    indent = ajStrNew();

    ajStrAppendCountK(&indent, ' ', level * 2);

    ajDebug("%SensProteinfeatureTrace %p\n"
            "%S  Use %u\n"
            "%S  Identifier %u\n"
            "%S  Adaptor %p\n"
            "%S  Featurepair %p\n"
            "%S  Accession '%S'\n"
            "%S  Description '%S'\n",
            indent, pf,
            indent, pf->Use,
            indent, pf->Identifier,
            indent, pf->Adaptor,
            indent, pf->Featurepair,
            indent, pf->Accession,
            indent, pf->Description);

    ensFeaturepairTrace(pf->Featurepair, level + 1);

    ajStrDel(&indent);

    return ajTrue;
}




/* @section calculate *********************************************************
**
** Functions for calculating values of an Ensembl Protein Feature object.
**
** @fdata [EnsPProteinfeature]
**
** @nam3rule Calculate Calculate Ensembl Protein Feature values
** @nam4rule Memsize Calculate the memory size in bytes
**
** @argrule * pf [const EnsPProteinfeature] Ensembl Protein Feature
**
** @valrule Memsize [size_t] Memory size in bytes or 0
**
** @fcategory misc
******************************************************************************/




/* @func ensProteinfeatureCalculateMemsize ************************************
**
** Get the memory size in bytes of an Ensembl Protein Feature.
**
** @param [r] pf [const EnsPProteinfeature] Ensembl Protein Feature
**
** @return [size_t] Memory size in bytes or 0
**
** @release 6.4.0
** @@
******************************************************************************/

size_t ensProteinfeatureCalculateMemsize(const EnsPProteinfeature pf)
{
    size_t size = 0;

    if (!pf)
        return 0;

    size += sizeof (EnsOProteinfeature);

    size += ensFeaturepairCalculateMemsize(pf->Featurepair);

    if (pf->Accession)
    {
        size += sizeof (AjOStr);

        size += ajStrGetRes(pf->Accession);
    }

    if (pf->Description)
    {
        size += sizeof (AjOStr);

        size += ajStrGetRes(pf->Description);
    }

    return size;
}




/* @datasection [EnsPProteinfeatureadaptor] Ensembl Protein Feature Adaptor ***
**
** @nam2rule Proteinfeatureadaptor Functions for manipulating
** Ensembl Protein Feature Adaptor objects
**
** @cc Bio::EnsEMBL::DBSQL::ProteinFeatureAdaptor
** @cc CVS Revision: 1.35
** @cc CVS Tag: branch-ensembl-66
**
******************************************************************************/




/* @funcstatic proteinfeatureadaptorFetchAllbyStatement ***********************
**
** Run a SQL statement against an Ensembl Database Adaptor and consolidate the
** results into an AJAX List of Ensembl Protein Feature objects.
**
** @param [u] ba [EnsPBaseadaptor] Ensembl Base Adaptor
** @param [r] statement [const AjPStr] SQL statement
** @param [uN] am [EnsPAssemblymapper] Ensembl Assembly Mapper
** @param [uN] slice [EnsPSlice] Ensembl Slice
** @param [u] pfs [AjPList] AJAX List of Ensembl Protein Feature objects
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
**
** @release 6.4.0
** @@
******************************************************************************/

static AjBool proteinfeatureadaptorFetchAllbyStatement(
    EnsPBaseadaptor ba,
    const AjPStr statement,
    EnsPAssemblymapper am,
    EnsPSlice slice,
    AjPList pfs)
{
    double evalue  = 0.0;
    double score   = 0.0;

    float identity = 0.0F;

    ajuint identifier = 0U;
    ajuint tlstart    = 0U;
    ajuint tlend      = 0U;
    ajuint analysisid = 0U;
    ajuint hitstart   = 0U;
    ajuint hitend     = 0U;

    AjPSqlstatement sqls = NULL;
    AjISqlrow sqli       = NULL;
    AjPSqlrow sqlr       = NULL;

    AjPStr tlid        = NULL;
    AjPStr hitname     = NULL;
    AjPStr accession   = NULL;
    AjPStr description = NULL;

    EnsPAnalysis analysis  = NULL;
    EnsPAnalysisadaptor aa = NULL;

    EnsPDatabaseadaptor dba = NULL;

    EnsPFeature srcfeature = NULL;
    EnsPFeature trgfeature = NULL;

    EnsPFeaturepair fp = NULL;

    EnsPProteinfeature pf         = NULL;
    EnsPProteinfeatureadaptor pfa = NULL;

    if (ajDebugTest("proteinfeatureadaptorFetchAllbyStatement"))
        ajDebug("proteinfeatureadaptorFetchAllbyStatement\n"
                "  ba %p\n"
                "  statement %p\n"
                "  am %p\n"
                "  slice %p\n"
                "  pfs %p\n",
                ba,
                statement,
                am,
                slice,
                pfs);

    if (!ba)
        return ajFalse;

    if (!statement)
        return ajFalse;

    if (!pfs)
        return ajFalse;

    dba = ensBaseadaptorGetDatabaseadaptor(ba);

    aa  = ensRegistryGetAnalysisadaptor(dba);
    pfa = ensRegistryGetProteinfeatureadaptor(dba);

    sqls = ensDatabaseadaptorSqlstatementNew(dba, statement);

    sqli = ajSqlrowiterNew(sqls);

    while (!ajSqlrowiterDone(sqli))
    {
        identifier  = 0;
        tlid        = ajStrNew();
        tlstart     = 0;
        tlend       = 0;
        hitname     = ajStrNew();
        hitstart    = 0;
        hitend      = 0;
        analysisid  = 0;
        score       = 0.0;
        evalue      = 0.0;
        identity    = 0.0F;
        accession   = ajStrNew();
        description = ajStrNew();

        sqlr = ajSqlrowiterGet(sqli);

        ajSqlcolumnToUint(sqlr, &identifier);
        ajSqlcolumnToStr(sqlr, &tlid);
        ajSqlcolumnToUint(sqlr, &tlstart);
        ajSqlcolumnToUint(sqlr, &tlend);
        ajSqlcolumnToStr(sqlr, &hitname);
        ajSqlcolumnToUint(sqlr, &hitstart);
        ajSqlcolumnToUint(sqlr, &hitend);
        ajSqlcolumnToUint(sqlr, &analysisid);
        ajSqlcolumnToDouble(sqlr, &score);
        ajSqlcolumnToDouble(sqlr, &evalue);
        ajSqlcolumnToFloat(sqlr, &identity);
        ajSqlcolumnToStr(sqlr, &accession);
        ajSqlcolumnToStr(sqlr, &description);

        ensAnalysisadaptorFetchByIdentifier(aa, analysisid, &analysis);

        /*
        ** NOTE: The translation_id is currently handled as an AJAX String
        ** to fit into the seqname member of an Ensembl Feature.
        */

        srcfeature = ensFeatureNewIniN(analysis, tlid, tlstart, tlend, 1);

        trgfeature = ensFeatureNewIniN((EnsPAnalysis) NULL,
                                       hitname,
                                       hitstart,
                                       hitend,
                                       1);

        fp = ensFeaturepairNewIni(srcfeature,
                                  trgfeature,
                                  (EnsPExternaldatabase) NULL, /* edb */
                                  (AjPStr) NULL, /* extra data */
                                  (AjPStr) NULL, /* srcspecies */
                                  (AjPStr) NULL, /* trgspecies */
                                  0, /* groupid */
                                  0, /* levelid */
                                  evalue,
                                  score,
                                  0, /* srccoverage */
                                  0, /* trgcoverage */
                                  identity);

        /* Create a Protein Feature object. */

        pf = ensProteinfeatureNewIni(pfa,
                                     identifier,
                                     fp,
                                     accession,
                                     description);

        ajListPushAppend(pfs, (void *) pf);

        ensFeaturepairDel(&fp);

        ensFeatureDel(&srcfeature);
        ensFeatureDel(&trgfeature);

        ajStrDel(&tlid);
        ajStrDel(&hitname);
        ajStrDel(&accession);
        ajStrDel(&description);

        ensAnalysisDel(&analysis);
    }

    ajSqlrowiterDel(&sqli);

    ensDatabaseadaptorSqlstatementDel(dba, &sqls);

    return ajTrue;
}




/* @section constructors ******************************************************
**
** All constructors return a new Ensembl Protein Feature Adaptor by pointer.
** It is the responsibility of the user to first destroy any previous
** Protein Feature Adaptor. The target pointer does not need to be
** initialised to NULL, but it is good programming practice to do so anyway.
**
** @fdata [EnsPProteinfeatureadaptor]
**
** @nam3rule New Constructor
**
** @argrule New dba [EnsPDatabaseadaptor] Ensembl Database Adaptor
**
** @valrule * [EnsPProteinfeatureadaptor]
** Ensembl Protein Feature Adaptor or NULL
**
** @fcategory new
******************************************************************************/




/* @func ensProteinfeatureadaptorNew ******************************************
**
** Default constructor for an Ensembl Protein Feature Adaptor.
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
** @see ensRegistryGetProteinfeatureadaptor
**
** @param [u] dba [EnsPDatabaseadaptor] Ensembl Database Adaptor
**
** @return [EnsPProteinfeatureadaptor] Ensembl Protein Feature Adaptor or NULL
**
** @release 6.2.0
** @@
******************************************************************************/

EnsPProteinfeatureadaptor ensProteinfeatureadaptorNew(
    EnsPDatabaseadaptor dba)
{
    if (!dba)
        return NULL;

    return ensBaseadaptorNew(
        dba,
        proteinfeatureadaptorKTables,
        proteinfeatureadaptorKColumns,
        proteinfeatureadaptorKLeftjoin,
        (const char *) NULL,
        (const char *) NULL,
        &proteinfeatureadaptorFetchAllbyStatement);
}




/* @section destructors *******************************************************
**
** Destruction destroys all internal data structures and frees the memory
** allocated for an Ensembl Protein Feature Adaptor object.
**
** @fdata [EnsPProteinfeatureadaptor]
**
** @nam3rule Del Destroy (free) an Ensembl Protein Feature Adaptor
**
** @argrule * Ppfa [EnsPProteinfeatureadaptor*]
** Ensembl Protein Feature Adaptor address
**
** @valrule * [void]
**
** @fcategory delete
******************************************************************************/




/* @func ensProteinfeatureadaptorDel ******************************************
**
** Default destructor for an Ensembl Protein Feature Adaptor.
**
** Ensembl Object Adaptors are singleton objects that are registered in the
** Ensembl Registry and weakly referenced by Ensembl Objects that have been
** instantiated by it. Therefore, Ensembl Object Adaptors should never be
** destroyed directly. Upon exit, the Ensembl Registry will call this function
** if required.
**
** @param [d] Ppfa [EnsPProteinfeatureadaptor*]
** Ensembl Protein Feature Adaptor address
**
** @return [void]
**
** @release 6.2.0
** @@
******************************************************************************/

void ensProteinfeatureadaptorDel(EnsPProteinfeatureadaptor *Ppfa)
{
    ensBaseadaptorDel(Ppfa);

	return;
}




/* @section member retrieval **************************************************
**
** Functions for returning members of an
** Ensembl Protein Feature Adaptor object.
**
** @fdata [EnsPProteinfeatureadaptor]
**
** @nam3rule Get Return Ensembl Protein Feature Adaptor attribute(s)
** @nam4rule Databaseadaptor Return the Ensembl Database Adaptor
**
** @argrule * pfa [EnsPProteinfeatureadaptor] Ensembl Protein Feature Adaptor
**
** @valrule Databaseadaptor [EnsPDatabaseadaptor] Ensembl Database Adaptor
** or NULL
**
** @fcategory use
******************************************************************************/




/* @func ensProteinfeatureadaptorGetDatabaseadaptor ***************************
**
** Get the Ensembl Database Adaptor member of an
** Ensembl Protein Feature Adaptor.
**
** @param [u] pfa [EnsPProteinfeatureadaptor] Ensembl Protein Feature Adaptor
**
** @return [EnsPDatabaseadaptor] Ensembl Database Adaptor or NULL
**
** @release 6.4.0
** @@
******************************************************************************/

EnsPDatabaseadaptor ensProteinfeatureadaptorGetDatabaseadaptor(
    EnsPProteinfeatureadaptor pfa)
{
    return ensBaseadaptorGetDatabaseadaptor(pfa);
}




/* @section object retrieval **************************************************
**
** Functions for fetching Ensembl Protein Feature objects from an
** Ensembl SQL database.
**
** @fdata [EnsPProteinfeatureadaptor]
**
** @nam3rule Fetch Fetch Ensembl Protein Feature object(s)
** @nam4rule All   Fetch all Ensembl Protein Feature objects
** @nam4rule Allby Fetch all Ensembl Protein Feature objects
**                      matching a criterion
** @nam5rule Translationidentifier Fetch all by an Ensembl Translation
**                                 identifier
** @nam4rule By    Fetch one Ensembl Protein Feature object
**                 matching a criterion
** @nam5rule Identifier Fetch by a SQL database-internal identifier
**
** @argrule * pfa [EnsPProteinfeatureadaptor]
** Ensembl Protein Feature Adaptor
** @argrule AllbyTranslationidentifier tlid [ajuint]
** Ensembl Translation identifier
** @argrule AllbyTranslationidentifier pfs [AjPList]
** AJAX List of Ensembl Protein Feature objects
** @argrule ByIdentifier identifier [ajuint]
** SQL database-internal identifier
** @argrule ByIdentifier Ppf [EnsPProteinfeature*]
** Ensembl Protein Feature address
**
** @valrule * [AjBool] ajTrue upon success, ajFalse otherwise
**
** @fcategory use
******************************************************************************/




/* @func ensProteinfeatureadaptorFetchAllbyTranslationidentifier **************
**
** Fetch an Ensembl Protein Feature via an Ensembl Translation identifier.
** The caller is responsible for deleting the Ensembl Protein Feature.
**
** @param [u] pfa [EnsPProteinfeatureadaptor] Ensembl Protein Feature Adaptor
** @param [r] tlid [ajuint] Ensembl Translation identifier
** @param [u] pfs [AjPList] AJAX List of Ensembl Protein Feature objects
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
**
** @release 6.4.0
** @@
******************************************************************************/

AjBool ensProteinfeatureadaptorFetchAllbyTranslationidentifier(
    EnsPProteinfeatureadaptor pfa,
    ajuint tlid,
    AjPList pfs)
{
    AjPStr constraint = NULL;

    if (!pfa)
        return ajFalse;

    if (!tlid)
        return ajFalse;

    if (!pfs)
        return ajFalse;

    constraint = ajFmtStr("protein_feature.translation_id = %u", tlid);

    ensBaseadaptorFetchAllbyConstraint(pfa,
                                       constraint,
                                       (EnsPAssemblymapper) NULL,
                                       (EnsPSlice) NULL,
                                       pfs);

    ajStrDel(&constraint);

    return ajTrue;
}




/* @func ensProteinfeatureadaptorFetchByIdentifier ****************************
**
** Fetch an Ensembl Protein Feature by its SQL database-internal identifier.
**
** @param [u] pfa [EnsPProteinfeatureadaptor] Ensembl Protein Feature Adaptor
** @param [r] identifier [ajuint] SQL database-internal identifier
** @param [wP] Ppf [EnsPProteinfeature*] Ensembl Protein Feature address
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
**
** @release 6.2.0
** @@
******************************************************************************/

AjBool ensProteinfeatureadaptorFetchByIdentifier(
    EnsPProteinfeatureadaptor pfa,
    ajuint identifier,
    EnsPProteinfeature *Ppf)
{
    if (!pfa)
        return ajFalse;

    if (!identifier)
        return ajFalse;

    if (!Ppf)
        return ajFalse;

    return ensBaseadaptorFetchByIdentifier(pfa, identifier, (void **) Ppf);
}
