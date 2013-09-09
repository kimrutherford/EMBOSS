/* @source ensembltest ********************************************************
**
** Ensembl API test routines
**
** @author Copyright (C) 2006 Michael K. Schuster
** @version $Revision: 1.44 $
** @modified $Date: 2013/02/17 13:11:02 $ by $Author: mks $
** @@
**
** This program is free software; you can redistribute it and/or
** modify it under the terms of the GNU General Public License
** as published by the Free Software Foundation; either version 2
** of the License, or (at your option) any later version.
**
** This program is distributed in the hope that it will be useful,
** but WITHOUT ANY WARRANTY; without even the implied warranty of
** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
** GNU General Public License for more details.
**
** You should have received a copy of the GNU General Public License
** along with this program; if not, write to the Free Software
** Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
**
******************************************************************************/




/* ========================================================================= */
/* ============================= include files ============================= */
/* ========================================================================= */

#include "emboss.h"

#ifndef WIN32
#include <limits.h>
#include <stddef.h>
#include <sys/time.h>
#include <sys/resource.h>
#include <sys/types.h>
#endif




/* ========================================================================= */
/* ============================= private data ============================== */
/* ========================================================================= */

/* @datastatic EnsembltestPProjections ****************************************
**
** Ensembl Test Slice Projection data structure.
**
** @alias EnsembltestSProjection
** @alias EnsembltestOProjection
**
** @attr SliceName [const char*] Ensembl Slice name
** @attr CoordsystemName [const char*] Ensembl Coordinate System name
** @attr CoordsystemVersion [const char*] Ensembl Coordinate System version
** @@
******************************************************************************/

typedef struct EnsembltestSProjection
{
    const char* SliceName;
    const char* CoordsystemName;
    const char* CoordsystemVersion;
} EnsembltestOProjection;

#define EnsembltestPProjection EnsembltestOProjection*




/* ========================================================================= */
/* =========================== private constants =========================== */
/* ========================================================================= */




/* ========================================================================= */
/* =========================== private variables =========================== */
/* ========================================================================= */




/* ========================================================================= */
/* =========================== private functions =========================== */
/* ========================================================================= */

static AjBool ensembltest_document_feature(EnsPFeature feature,
                                           ajuint level,
                                           AjPStr *Pstring);

static AjBool ensembltest_ambiguity(AjPFile aoutfile);

static AjBool ensembltest_analysis(EnsPDatabaseadaptor dba,
                                   AjPFile aoutfile);

static AjBool ensembltest_assembly_exception(EnsPDatabaseadaptor dba,
                                             AjPFile aoutfile);

static AjBool ensembltest_basealignfeature(EnsPDatabaseadaptor dba,
                                           AjPFile aoutfile);

static AjBool ensembltest_chromosome(EnsPDatabaseadaptor dba,
                                     AjPSeqout aoutseqall);

static AjBool ensembltest_constants(AjPFile aoutfile);

static AjBool ensembltest_coordinate_system(EnsPDatabaseadaptor dba,
                                            AjPFile aoutfile);

static AjBool ensembltest_databaseentry(EnsPDatabaseadaptor dba,
                                        AjPFile aoutfile);

static AjBool ensembltest_datatypes(AjPFile aoutfile);

static AjBool ensembltest_ditag(EnsPDatabaseadaptor dba,
                                AjPFile aoutfile);

static AjBool ensembltest_density(EnsPDatabaseadaptor dba,
                                  AjPFile aoutfile);

static AjBool ensembltest_exon(EnsPDatabaseadaptor dba,
                               AjPFile aoutfile);

static AjBool ensembltest_feature(EnsPDatabaseadaptor dba,
                                  AjPFile aoutfile);

static AjBool ensembltest_genes(EnsPDatabaseadaptor dba);

static AjBool ensembltest_marker(EnsPDatabaseadaptor dba,
                                 AjPFile aoutfile);

static AjBool ensembltest_masking(EnsPDatabaseadaptor dba,
                                  AjPSeqout aoutseqall);

static AjBool ensembltest_meta_information(EnsPDatabaseadaptor dba,
                                           AjPFile aoutfile);

static AjBool ensembltest_operon(EnsPDatabaseadaptor dba,
                                 AjPFile aoutfile);

static AjBool ensembltest_ontology(EnsPDatabaseadaptor dba,
                                   AjPFile aoutfile);

static AjBool ensembltest_miscellaneous(EnsPDatabaseadaptor dba,
                                        AjPFile aoutfile);

static AjBool ensembltest_registry(AjPFile aoutfile);

static AjBool ensembltest_sequence_region(EnsPDatabaseadaptor dba,
                                          AjPFile aoutfile);

static AjBool ensembltest_slice_projection(EnsPDatabaseadaptor dba,
                                           AjPFile aoutfile);

static AjBool ensembltest_sequence(EnsPDatabaseadaptor dba,
                                   AjPSeqout aoutseqall);

static AjBool ensembltest_transcript(EnsPDatabaseadaptor dba,
                                     AjPFile aoutfile);

static AjBool ensembltest_transformations(EnsPDatabaseadaptor dba,
                                          AjPFile aoutfile);

static AjBool ensembltest_translations(EnsPDatabaseadaptor dba,
                                       AjPFile aoutfile);




/* @prog ensembltest **********************************************************
**
** Ensembl API test routines
**
******************************************************************************/

int main(int argc, char** argv)
{
#ifndef WIN32
    struct rusage ru;
#endif

    AjBool large = AJFALSE;

    AjPFile aoutfile = NULL;

    AjIList iterator = NULL;
    AjPList svrnames = NULL;

    AjPSeqout aoutseqall = NULL; /* Ensembl Slice sequences */

    AjPStr species  = NULL;
    AjPStr svrname  = NULL;
    AjPStr svrvalue = NULL;
    AjPStr svrurl   = NULL;

    EnsEDatabaseadaptorGroup group  = ensEDatabaseadaptorGroupNULL;

    EnsPDatabaseadaptor dba = NULL;

    EnsPDatabaseconnection dbc = NULL;

    embInit("ensembltest", argc, argv);

    aoutfile = ajAcdGetOutfile("aoutfile");

    aoutseqall = ajAcdGetSeqoutall("aoutseqall");

    large = ajAcdGetBoolean("large");

    ensInit();

    /*
    ** Ensembl Registry Test
    **
    ** Get all AJAX Server definitions and use the ones with
    ** "method" "ensembl" to load the Ensembl Registry.
    */

    svrurl   = ajStrNew();
    svrvalue = ajStrNew();

    svrnames = ajListstrNew();

    ajNamListListServers(svrnames);

    iterator = ajListIterNewread(svrnames);

    while (!ajListIterDone(iterator))
    {
        svrname = ajListstrIterGet(iterator);

#if AJFALSE
        /* FIXME: Restrict to AJAX Server name ensembl only! */
        if (!ajStrMatchC(svrname, "ensembl"))
            continue;
#endif

        ajStrAssignClear(&svrvalue);

        ajNamSvrGetAttrC(svrname, "method", &svrvalue);

        if (ajStrMatchC(svrvalue, "ensembl"))
        {
            ajUser("Ensembl Registry load AJAX Server '%S'", svrname);

            ajStrAssignClear(&svrurl);

            ajNamSvrGetUrl(svrname, &svrurl);

            if (ajStrGetLen(svrurl) == 0)
                ajFatal("Could not get a URL for AJAX server name '%S'.",
                        svrname);

            dbc = ensDatabaseconnectionNewUrl(svrurl);

            if (!dbc)
                ajFatal("Could not get an Ensembl Database Connection for "
                        "URL '%S'.",
                        svrurl);

            ensRegistryLoadDatabaseconnection(dbc);

            ensDatabaseconnectionDel(&dbc);
        }
    }

    ajListIterDel(&iterator);

    ajListFree(&svrnames);

    ajStrDel(&svrurl);
    ajStrDel(&svrvalue);

    /* Test routines perfomed on a Homo sapiens core database. */

    group    = ensEDatabaseadaptorGroupCore;
    species  = ajStrNewC("homo_sapiens");

    dba = ensRegistryGetDatabaseadaptor(group, species);

    ensembltest_constants(aoutfile);
    ensembltest_datatypes(aoutfile);
    ensembltest_ambiguity(aoutfile);
    ensembltest_registry(aoutfile);
    ensembltest_analysis(dba, aoutfile);
    ensembltest_meta_information(dba, aoutfile);
    ensembltest_coordinate_system(dba, aoutfile);
    ensembltest_sequence_region(dba, aoutfile);
    ensembltest_assembly_exception(dba, aoutfile);
    ensembltest_slice_projection(dba, aoutfile);
    ensembltest_masking(dba, aoutseqall);
    ensembltest_sequence(dba, aoutseqall);
    ensembltest_feature(dba, aoutfile);
    ensembltest_exon(dba, aoutfile);
    ensembltest_transcript(dba, aoutfile);
    ensembltest_translations(dba, aoutfile);
    ensembltest_ditag(dba, aoutfile);
    ensembltest_density(dba, aoutfile);
    ensembltest_marker(dba, aoutfile);
    ensembltest_miscellaneous(dba, aoutfile);
    ensembltest_transformations(dba, aoutfile);
    ensembltest_ontology(dba, aoutfile);
    ensembltest_databaseentry(dba, aoutfile);
    ensembltest_basealignfeature(dba, aoutfile);

    /* Fetch larger data sets... */

    if (large)
    {
        ensembltest_genes(dba);

        /* Fetch sequence for human chromosome 21. */

        if (AJFALSE)
            ensembltest_chromosome(dba, aoutseqall);
    }

    /* Test routines performed on an Escherichia coli K12 core database. */

    group = ensEDatabaseadaptorGroupCore;
    ajStrAssignC(&species, "e_coli_k12");

    dba = ensRegistryGetDatabaseadaptor(group, species);

    ensembltest_operon(dba, aoutfile);

    /* Clean up and exit. */

    ajFileClose(&aoutfile);

    ajSeqoutClose(aoutseqall);

    ajSeqoutDel(&aoutseqall);

    ajStrDel(&species);

#ifndef WIN32

    /* Get resource usage */

    getrusage(RUSAGE_SELF, &ru);

    ajDebug("main resource usage\n"
            "  user time %ld s\n"
            "  user time %ld us\n"
            "  system time %ld s\n"
            "  system time %ld us\n"
            "  max resident set size %ld kB\n"
            "  integral shared text memory size %ld kB*ticks\n"
            "  integral unshared data size %ld kB*ticks\n"
            "  integral unshared stack size %ld kB*ticks\n"
            "  page reclaims %ld\n"
            "  page faults %ld\n"
            "  swaps %ld\n"
            "  block input operations %ld\n"
            "  block output operations %ld\n"
            "  messages sent %ld\n"
            "  messages received %ld\n"
            "  signals received %ld\n"
            "  voluntary context switches %ld\n"
            "  involuntary context switches %ld\n",
            ru.ru_utime.tv_sec,
            ru.ru_utime.tv_usec,
            ru.ru_stime.tv_sec,
            ru.ru_stime.tv_usec,
            ru.ru_maxrss,
            ru.ru_ixrss,
            ru.ru_idrss,
            ru.ru_isrss,
            ru.ru_minflt,
            ru.ru_majflt,
            ru.ru_nswap,
            ru.ru_inblock,
            ru.ru_oublock,
            ru.ru_msgsnd,
            ru.ru_msgrcv,
            ru.ru_nsignals,
            ru.ru_nvcsw,
            ru.ru_nivcsw);

#endif


#if defined(__APPLE__)

    /* Inserted to allow the leaks application to analyse this process. */

    ajUser("ensembltest sleeping for 30 seconds.");

    sleep(30);

#endif

    embExit();

    return EXIT_SUCCESS;
}




/* @funcstatic ensembltest_document_feature ***********************************
**
** Helper function to document Ensembl Feature members.
**
** The caller is responsible for deleting the AJAX String.
**
** @param [u] feature [EnsPFeature] Ensembl Feature
** @param [r] level [ajuint] Indentation level
** @param [wP] Pstring [AjPStr*] AJAX String
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
** @@
******************************************************************************/

static AjBool ensembltest_document_feature(EnsPFeature feature,
                                           ajuint level,
                                           AjPStr *Pstring)
{
    AjPStr indent = NULL;
    AjPStr slname = NULL;

    EnsPAnalysis analysis = NULL;

    EnsPSlice slice = NULL;

    if (!feature)
        return ajFalse;

    if (!Pstring)
        return ajFalse;

    if (*Pstring)
        ajStrAssignClear(Pstring);
    else
        *Pstring = ajStrNew();

    indent = ajStrNew();

    ajStrAppendCountK(&indent, ' ', level * 2);

    ajFmtPrintAppS(Pstring,
                   "%S  Ensembl Feature\n",
                   indent);

    slice = ensFeatureGetSlice(feature);

    if (slice)
    {
        slname = ajStrNew();

        ensSliceFetchName(slice, &slname);

        ajFmtPrintAppS(Pstring,
                       "%S    Ensembl Slice '%S'\n"
                       "%S      coordinates '%d:%d:%d'\n"
                       "%S    Sequence Region\n"
                       "%S      coordinates '%S:%d:%d:%d'\n",
                       indent,
                       slname,
                       indent,
                       ensFeatureGetStart(feature),
                       ensFeatureGetEnd(feature),
                       ensFeatureGetStrand(feature),
                       indent,
                       ensFeatureGetSeqregionName(feature),
                       ensFeatureGetSeqregionStart(feature),
                       ensFeatureGetSeqregionEnd(feature),
                       ensFeatureGetSeqregionStrand(feature));

        ajStrDel(&slname);
    }

    analysis = ensFeatureGetAnalysis(feature);

    if (analysis)
        ajFmtPrintAppS(Pstring,
                       "%S    Ensembl Analysis\n"
                       "%S      identifier %u\n"
                       "%S      name '%S'\n",
                       indent,
                       indent,
                       ensAnalysisGetIdentifier(analysis),
                       indent,
                       ensAnalysisGetName(analysis));

    ajStrDel(&indent);

    return ajTrue;
}




/* @funcstatic ensembltest_ambiguity ******************************************
**
** Ensembl Ambiguity Codes.
**
** Convert base strings in ambiguity codes and vice versa.
**
** @param [u] aoutfile [AjPFile] AJAX File
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
** @@
******************************************************************************/

static AjBool ensembltest_ambiguity(AjPFile aoutfile)
{
    const char *basestr = "ATG";

    ajint ambiguity = 0;

    if (!aoutfile)
        return ajFalse;

    ajFmtPrintF(aoutfile,
                "\n"
                "Ensembl Ambiguity Codes\n");

    ajUser("Ensembl Ambiguity Codes");

    ambiguity = (ajint) ensUtilityBaseAmbiguityFromString(basestr);

    ajFmtPrintF(aoutfile,
                "\n"
                "  Ambiguity code %d for base string '%s'\n",
                ambiguity,
                basestr);

    ajFmtPrintF(aoutfile,
                "\n"
                "  Base string '%s' for ambiguity code 'Y'\n",
                ensUtilityBaseAmbiguityToString('Y'));

    return ajTrue;
}




/* @funcstatic ensembltest_slice_projection ***********************************
**
** Ensembl Slice Projection.
**
** Project the 'chromosome' X and Y PAR regions (1 - 2709520) into the 'clone'
** and 'contig' Coordinate System, which will use the Generic Assembly Mapper
** for the mapping path 'chromosome' <-> 'contig' and the Chained Assembly
** Mapper for the mapping path 'chromosome' <-> 'contig' <-> 'clone'.
**
** @param [u] dba [EnsPDatabaseadaptor] Ensembl Database Adaptor
** @param [u] aoutfile [AjPFile] AJAX File
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
** @@
******************************************************************************/

static AjBool ensembltest_slice_projection(EnsPDatabaseadaptor dba,
                                           AjPFile aoutfile)
{
    register ajuint i = 0U;

    AjBool debug = AJFALSE;

    AjPList pss = NULL;

    AjPStr slname    = NULL;
    AjPStr csname    = NULL;
    AjPStr csversion = NULL;
    AjPStr sename    = NULL;
    AjPStr psname    = NULL;

    EnsPProjectionsegment ps = NULL;

    EnsPSlice psslice    = NULL;
    EnsPSlice seslice    = NULL;
    EnsPSliceadaptor sla = NULL;

    /* Project the following Ensembl Slice objects. */

    const EnsembltestOProjection ensembltestProjection[] =
        {
            /*
            ** Test the Ensembl Chained Assembly Mapper for the
            ** mapping path 'clone' <-> 'contig' <-> 'chromosome'.
            */
            {"clone::AC120349.5:0:0:0", "chromosome", "GRCh37"},
            /* Test the Ensembl Top-Level Assembly Mapper. */
            {"clone::AC120349.5:0:0:0", "toplevel", ""},
            /* Test the PAR regions at X and Y p-telomeres. */
            {"chromosome:GRCh37:X:1:3000000:1", "contig", ""},
            {"chromosome:GRCh37:X:1:3000000:1", "clone", ""},
            {"chromosome:GRCh37:Y:1:3000000:1", "contig", ""},
            {"chromosome:GRCh37:Y:1:3000000:1", "clone", ""},
            {"chromosome:GRCh37:Y:0:0:0", "supercontig", ""},
            /* Test a Slice projection inside a HAP region. */
            {
                "chromosome:GRCh37:HSCHR6_MHC_COX:29000000:32000000:1",
                "clone",
                ""
            },
            /* Test a Slice projection outside a HAP region. */
            {
                "chromosome:GRCh37:HSCHR6_MHC_COX:24000000:26000000:1",
                "clone",
                ""
            },
            /* Test for negative Slice coordinates. */
            {"chromosome:GRCh37:22:-17000000:18000000:1", "supercontig", ""},
            /* Test centromer regions. */
            {"chromosome:GRCh37:21:13200001:14600000:1", "contig", ""},
            {(const char *) NULL, (const char *) NULL, (const char *) NULL}
        };

    debug = ajDebugTest("ensembltest_slice_projection");

    if (!dba)
        return ajFalse;

    if (!aoutfile)
        return ajFalse;

    ajFmtPrintF(aoutfile,
                "\n"
                "Ensembl Slice Projection\n");

    ajUser("Ensembl Slice Projection");

    sla = ensRegistryGetSliceadaptor(dba);

    pss = ajListNew();

    slname    = ajStrNew();
    csname    = ajStrNew();
    csversion = ajStrNew();

    for (i = 0U; ensembltestProjection[i].SliceName; i++)
    {
        ajStrAssignC(&slname,    ensembltestProjection[i].SliceName);
        ajStrAssignC(&csname,    ensembltestProjection[i].CoordsystemName);
        ajStrAssignC(&csversion, ensembltestProjection[i].CoordsystemVersion);

        ajFmtPrintF(aoutfile,
                    "\n"
                    "  Project Ensembl Slice '%S' into\n"
                    "  Ensembl Coordinate System '%S:%S'.\n",
                    slname, csname, csversion);

        ensSliceadaptorFetchByName(sla, slname, &seslice);

        ensSliceFetchName(seslice, &sename);

        if (debug)
            ajDebug("ensembltest_slice_projection begin ensSliceProject "
                    "for Ensembl Slice '%S' and "
                    "Ensembl Coordinate System '%S:%S'.\n",
                    sename, csname, csversion);

        ensSliceProject(seslice, csname, csversion, pss);

        if (debug)
            ajDebug("ensembltest_slice_projection finished "
                    "ensSliceProject.\n");

        while (ajListPop(pss, (void**) &ps))
        {
            psslice = ensProjectionsegmentGetTargetSlice(ps);

            ensSliceFetchName(psslice, &psname);

            ajFmtPrintF(aoutfile,
                        "\n"
                        "    '%S' %u:%u\n"
                        "    '%S'\n",
                        sename,
                        ensProjectionsegmentGetSourceStart(ps),
                        ensProjectionsegmentGetSourceEnd(ps),
                        psname);

            ajStrDel(&psname);

            ensProjectionsegmentDel(&ps);
        }

        ajStrDel(&sename);

        ensSliceDel(&seslice);
    }

    ajListFree(&pss);

    ajStrDel(&slname);
    ajStrDel(&csname);
    ajStrDel(&csversion);

    return ajTrue;
}




/* @funcstatic ensembltest_basealignfeature ***********************************
**
** Ensembl Base Align Feature tests.
**
** @param [u] dba [EnsPDatabaseadaptor] Ensembl Database Adaptor
** @param [u] aoutfile [AjPFile] AJAX File
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
** @@
******************************************************************************/

static AjBool ensembltest_basealignfeature(EnsPDatabaseadaptor dba,
                                           AjPFile aoutfile)
{
    AjPList bafs = NULL;

    AjPStr hitname = NULL;
    AjPStr anname  = NULL;

    EnsPBasealignfeature baf = NULL;

/*    EnsPDnaalignfeatureadaptor dafa = NULL;*/

    EnsPFeature srcfeature = NULL;
    EnsPFeature trgfeature = NULL;

    EnsPFeaturepair fp = NULL;

    EnsPProteinalignfeatureadaptor pafa = NULL;

    if (!dba)
        return ajFalse;

    if (!aoutfile)
        return ajFalse;

    ajFmtPrintF(aoutfile,
                "\n"
                "Ensembl Base Align Feature\n");

    ajUser("Ensembl Base Align Feature");

/*    dafa = ensRegistryGetDnaalignfeatureadaptor(dba);*/
    pafa = ensRegistryGetProteinalignfeatureadaptor(dba);

    bafs = ajListNew();

    hitname = ajStrNewC("Q15796");
    anname  = ajStrNewC("uniprot_mammal");

    ajFmtPrintF(aoutfile,
                "\n"
                "  Fetch all Ensembl Protein Align Feature objects by\n"
                "  hitname '%S' and Ensembl Analysis name '%S'\n",
                hitname, anname);

    ensProteinalignfeatureadaptorFetchAllbyHitunversioned(pafa,
                                                          hitname,
                                                          anname,
                                                          bafs);

    ensListBasealignfeatureSortSourceStartAscending(bafs);

    ajFmtPrintF(aoutfile,
                "\n"
                "    Found %Lu Ensembl Protein Align Feature objects for\n"
                "    hitname '%S' and Ensembl Analysis name '%S'\n",
                ajListGetLength(bafs),
                hitname, anname);

    while (ajListPop(bafs, (void **) &baf))
    {
        fp = ensBasealignfeatureGetFeaturepair(baf);

        srcfeature = ensFeaturepairGetSourceFeature(fp);
        trgfeature = ensFeaturepairGetTargetFeature(fp);

        ajFmtPrintF(aoutfile,
                    "\n"
                    "    Ensembl Protein Align Feature\n"
                    "      Identifier %u\n"
                    /*  "      Type %d\n" */
                    "      Cigar '%S'\n"
                    "      Source Ensembl Feature %S:%d:%d:%d\n"
                    "      Target Ensembl Feature %S:%d:%d:%d\n",
                    ensBasealignfeatureGetIdentifier(baf),
                    /* ensBasealignfeatureGetType(baf), */
                    ensBasealignfeatureGetCigar(baf),
                    ensFeatureGetSeqregionName(srcfeature),
                    ensFeatureGetSeqregionStart(srcfeature),
                    ensFeatureGetSeqregionEnd(srcfeature),
                    ensFeatureGetSeqregionStrand(srcfeature),
                    ensFeatureGetSequencename(trgfeature),
                    ensFeatureGetStart(trgfeature),
                    ensFeatureGetEnd(trgfeature),
                    ensFeatureGetStrand(trgfeature));

        ensBasealignfeatureDel(&baf);
    }

    ajListFree(&bafs);

    ajStrDel(&hitname);
    ajStrDel(&anname);

    return ajTrue;
}




/* @funcstatic ensembltest_analysis *******************************************
**
** Ensembl Analysis tests.
**
** @param [u] dba [EnsPDatabaseadaptor] Ensembl Database Adaptor
** @param [u] aoutfile [AjPFile] AJAX File
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
** @@
******************************************************************************/

static AjBool ensembltest_analysis(EnsPDatabaseadaptor dba,
                                   AjPFile aoutfile)
{
    AjPList analyses = NULL;

    AjPStr name = NULL;

    EnsPAnalysis        analysis        = NULL;
    EnsPAnalysisadaptor analysisadaptor = NULL;

    if (!dba)
        return ajFalse;

    if (!aoutfile)
        return ajFalse;

    analysisadaptor = ensRegistryGetAnalysisadaptor(dba);

    name = ajStrNew();

    ajFmtPrintF(aoutfile,
                "\n"
                "Ensembl Analysis\n");

    ajUser("Ensembl Analysis");

    /* Fetch the Ensembl Analysis for identifier 1 */

    ajFmtPrintF(aoutfile,
                "\n"
                "  Ensembl Analysis for identifier 1\n");

    ensAnalysisadaptorFetchByIdentifier(analysisadaptor, 1, &analysis);

    ajFmtPrintF(aoutfile,
                "    Ensembl Analysis\n"
                "      identifier %u\n"
                "      name '%S'\n",
                ensAnalysisGetIdentifier(analysis),
                ensAnalysisGetName(analysis));

    ensAnalysisDel(&analysis);

    /* Fetch the Ensembl Analysis for name 'cpg' */

    ajFmtPrintF(aoutfile,
                "\n"
                "  Ensembl Analysis for name 'cpg'\n");

    ajStrAssignC(&name, "cpg");

    ensAnalysisadaptorFetchByName(analysisadaptor, name, &analysis);

    ajFmtPrintF(aoutfile,
                "    Ensembl Analysis\n"
                "      identifier %u\n"
                "      name '%S'\n",
                ensAnalysisGetIdentifier(analysis),
                ensAnalysisGetName(analysis));

    ensAnalysisDel(&analysis);

    /* Fetch all Ensembl Analysis objects */

    ajFmtPrintF(aoutfile,
                "\n"
                "  All Ensembl Analysis objects\n");

    analyses = ajListNew();

    ensAnalysisadaptorFetchAll(analysisadaptor, analyses);

    ensListAnalysisSortIdentifierAscending(analyses);

    while (ajListPop(analyses, (void**) &analysis))
    {
        ajFmtPrintF(aoutfile,
                    "    Ensembl Analysis\n"
                    "      identifier %u\n"
                    "      name '%S'\n",
                    ensAnalysisGetIdentifier(analysis),
                    ensAnalysisGetName(analysis));

        ensAnalysisDel(&analysis);
    }

    ajListFree(&analyses);

    /* Test SQL escaping of "'" characters. */

    ajFmtPrintF(aoutfile,
                "\n"
                "  Test SQL escaping of \"'\" characters.\n");

    ajStrAssignC(&name, "'test'");

    ensAnalysisadaptorFetchByName(analysisadaptor, name, &analysis);

    if (analysis)
        ajFmtPrintF(aoutfile,
                    "    Ensembl Analysis for name '%S'\n"
                    "      identifier %u\n"
                    "      name '%S'\n",
                    name,
                    ensAnalysisGetIdentifier(analysis),
                    ensAnalysisGetName(analysis));
    else
        ajFmtPrintF(aoutfile,
                    "    No Ensembl Analysis for name '%S', "
                    "which is the expected result.\n",
                    name);

    ensAnalysisDel(&analysis);

    ajStrDel(&name);

    return ajTrue;
}




/* @funcstatic ensembltest_assembly_exception *********************************
**
** Project Ensembl Slices crossing Assembly Exception.
**
** @param [u] dba [EnsPDatabaseadaptor] Ensembl Database Adaptor
** @param [u] aoutfile [AjPFile] AJAX File
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
** @@
******************************************************************************/

static AjBool ensembltest_assembly_exception(EnsPDatabaseadaptor dba,
                                             AjPFile aoutfile)
{
    AjBool debug        = AJFALSE;
    AjBool duplicates   = AJTRUE;
    AjBool nonreference = AJTRUE;
    AjBool lrg          = AJFALSE;

    AjPList pss    = NULL;
    AjPList slices = NULL;

    AjPStr csname    = NULL;
    AjPStr csversion = NULL;
    AjPStr sename    = NULL;
    AjPStr psname    = NULL;

    EnsPProjectionsegment ps = NULL;

    EnsPSlice psslice    = NULL;
    EnsPSlice seslice    = NULL;
    EnsPSliceadaptor sla = NULL;

    debug = ajDebugTest("ensembltest_assembly_exception");

    if (debug)
        ajDebug("ensembltest_assembly_exception start\n");

    if (!dba)
        return ajFalse;

    if (!aoutfile)
        return ajFalse;

    ajFmtPrintF(aoutfile,
                "\n"
                "Ensembl Assembly Exception\n");

    ajUser("Ensembl Assembly Exception");

    sla = ensRegistryGetSliceadaptor(dba);

    csname    = ajStrNewC("toplevel");
    csversion = ajStrNew();

    pss    = ajListNew();
    slices = ajListNew();

    ensSliceadaptorFetchAll(sla,
                            csname,
                            csversion,
                            nonreference,
                            duplicates,
                            lrg,
                            slices);

    ensListSliceSortIdentifierAscending(slices);

    while (ajListPop(slices, (void**) &seslice))
    {
        ensSliceFetchName(seslice, &sename);

        ajFmtPrintF(aoutfile,
                    "\n"
                    "  Fetch normalised Slice Projections for Ensembl Slice\n"
                    "  '%S'\n"
                    "\n",
                    sename);

        if (debug)
            ajDebug("ensembltest_assembly_exceptions begin "
                    "ensSliceadaptorRetrieveNormalisedprojection "
                    "for Ensembl Slice '%S'.\n",
                    sename);

        ensSliceadaptorRetrieveNormalisedprojection(sla,
                                                    seslice,
                                                    pss);

        if (debug)
            ajDebug("ensembltest_assembly_exceptions finished "
                    "ensSliceadaptorRetrieveNormalisedprojection.\n");

        while (ajListPop(pss, (void**) &ps))
        {
            psslice = ensProjectionsegmentGetTargetSlice(ps);

            ensSliceFetchName(psslice, &psname);

            ajFmtPrintF(aoutfile,
                        "    '%S' %u:%u\n"
                        "    '%S'\n"
                        "\n",
                        sename,
                        ensProjectionsegmentGetSourceStart(ps),
                        ensProjectionsegmentGetSourceEnd(ps),
                        psname);

            ajStrDel(&psname);

            ensProjectionsegmentDel(&ps);
        }

        ajStrDel(&sename);

        ensSliceDel(&seslice);
    }

    ajListFree(&pss);
    ajListFree(&slices);

    ajStrDel(&csname);
    ajStrDel(&csversion);

    if (debug)
        ajDebug("ensembltest_assembly_exception end\n");

    return ajTrue;
}




/* @funcstatic ensembltest_constants ******************************************
**
** Ensembl constants tests.
**
** @param [u] aoutfile [AjPFile] AJAX File
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
** @@
******************************************************************************/

static AjBool ensembltest_constants(AjPFile aoutfile)
{
    if (!aoutfile)
        return ajFalse;

    ajFmtPrintF(aoutfile,
                "\n"
                "Ensembl Constants\n");

    ajUser("Ensembl Constants");

    ajFmtPrintF(aoutfile,
                "\n"
                "  Ensembl Base Adaptor Maximum Identifiers %u\n",
                ensKBaseadaptorMaximumIdentifiers);

    ajFmtPrintF(aoutfile,
                "\n"
                "  Ensembl Software Version '%s'\n",
                ensKSoftwareVersion);

    return ajTrue;
}




/* @funcstatic ensembltest_datatypes ******************************************
**
** Ensembl Data Type tests.
**
** @param [u] aoutfile [AjPFile] AJAX File
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
** @@
******************************************************************************/

static AjBool ensembltest_datatypes(AjPFile aoutfile)
{
    ajulong maximum = 0U;
    ajulong size = 0U;

    if (!aoutfile)
        return ajFalse;

    ajFmtPrintF(aoutfile,
                "\n"
                "Ensembl Data Types\n"
                "\n");

    ajUser("Ensembl Data Types");

#ifndef WIN32
    maximum = USHRT_MAX;
    size = sizeof (unsigned short int);
    ajFmtPrintF(aoutfile,
                "  sizeof (unsigned short int) %Lu USHRT_MAX %Lx\n",
                size, maximum);

    maximum = UINT_MAX;
    size = sizeof (unsigned int);
    ajFmtPrintF(aoutfile,
                "  sizeof (unsigned int) %Lu UINT_MAX %Lx\n",
                size, maximum);

    maximum = ULONG_MAX;
    size = sizeof (unsigned long int);
    ajFmtPrintF(aoutfile,
                "  sizeof (unsigned long int) %Lu ULONG_MAX %Lx\n",
                size, maximum);

    /*
    ** ULLONG_MAX is a C99 feature not available by default on the
    ** Intel Compiler implementation.
    */
#if defined(_DARWIN_C_SOURCE)
    maximum = ULLONG_MAX;
    size = sizeof (unsigned long long int);
    ajFmtPrintF(aoutfile,
                "  sizeof (unsigned long long int) %Lu ULLONG_MAX %Lx\n",
                size, maximum);
#endif /* AJFALSE */

    maximum = 0U;
    size = sizeof (off_t);
    ajFmtPrintF(aoutfile,
                "  sizeof (off_t) %Lu\n",
                size);

    /* SIZE_T_MAX is a _DARWIN_C_SOURCE feature */
#if defined(_DARWIN_C_SOURCE)
    maximum = SIZE_T_MAX;
    size = sizeof (size_t);
    ajFmtPrintF(aoutfile,
                "  sizeof (size_t) %Lu SIZE_T_MAX %Lx\n",
                size, maximum);
#else
    size = sizeof (size_t);
    ajFmtPrintF(aoutfile,
                "  sizeof (size_t) %Lu\n",
                size);
#endif /* _DARWIN_C_SOURCE */
#endif /* !WIN32 */

    size = sizeof (ajushort);
    ajFmtPrintF(aoutfile,
                "  sizeof (ajushort) %Lu\n",
                size);

    size = sizeof (ajuint);
    ajFmtPrintF(aoutfile,
                "  sizeof (ajuint) %Lu\n",
                size);

    size = sizeof (ajulong);
    ajFmtPrintF(aoutfile,
                "  sizeof (ajulong) %Lu\n",
                size);

    return ajTrue;
}




/* @funcstatic ensembltest_databaseentry **************************************
**
** Ensembl Database Entry tests.
**
** @param [u] dba [EnsPDatabaseadaptor] Ensembl Database Adaptor
** @param [u] aoutfile [AjPFile] AJAX File
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
** @@
******************************************************************************/

static AjBool ensembltest_databaseentry(EnsPDatabaseadaptor dba,
                                        AjPFile aoutfile)
{
    AjPList dbes = NULL;

    AjPStr name     = NULL;
    AjPStr dbname   = NULL;
    AjPStr stableid = NULL;

    EnsPDatabaseentry        dbe  = NULL;
    EnsPDatabaseentryadaptor dbea = NULL;

    EnsPGene        gene  = NULL;
    EnsPGeneadaptor ga    = NULL;

    if (!dba)
        return ajFalse;

    if (!aoutfile)
        return ajFalse;

    ajFmtPrintF(aoutfile,
                "\n"
                "Ensembl Database Entry\n");

    ajUser("Ensembl Database Entry");

    dbea = ensRegistryGetDatabaseentryadaptor(dba);
    ga   = ensRegistryGetGeneadaptor(dba);

    name = ajStrNewC("SMAD2");

    dbname = ajStrNewC("HGNC");

    dbes = ajListNew();

    ajFmtPrintF(aoutfile,
                "\n"
                "  Fetch all Ensembl Database Entry objects by\n"
                "  name '%S' and dbname '%S'\n",
                name, dbname);

    ensDatabaseentryadaptorFetchAllbyName(dbea, name, dbname, dbes);

    ajFmtPrintF(aoutfile,
                "\n"
                "    Found %Lu Ensembl Database Entry objects for\n"
                "    name '%S' and dbname '%S'\n",
                ajListGetLength(dbes), name, dbname);

    while (ajListPop(dbes, (void **) &dbe))
    {
        ajFmtPrintF(aoutfile,
                    "\n"
                    "    Ensembl Database Entry %u\n"
                    "      Database Display Name '%S'\n"
                    "      Database Name '%S'\n"
                    "      Database Release '%S'\n"
                    "      Primary Identifier '%S'\n"
                    "      Display Identifier '%S'\n"
                    "      Version '%S'\n",
                    ensDatabaseentryGetIdentifier(dbe),
                    ensDatabaseentryGetDbDisplayname(dbe),
                    ensDatabaseentryGetDbName(dbe),
                    ensDatabaseentryGetDbRelease(dbe),
                    ensDatabaseentryGetPrimaryidentifier(dbe),
                    ensDatabaseentryGetDisplayidentifier(dbe),
                    ensDatabaseentryGetVersion(dbe));

        ensDatabaseentryDel(&dbe);
    }

    stableid = ajStrNewC("ENSG00000175387");

    ajFmtPrintF(aoutfile,
                "\n"
                "  Fetch all Ensembl Database Entry objects by\n"
                "  Ensembl Gene '%S'\n",
                stableid);

    ensGeneadaptorFetchByStableidentifier(ga, stableid, 0U, &gene);

    ensDatabaseentryadaptorFetchAllbyGene(dbea,
                                          gene,
                                          (const AjPStr) NULL,
                                          ensEExternaldatabaseTypeNULL,
                                          dbes);

    ajFmtPrintF(aoutfile,
                "\n"
                "    Found %Lu Ensembl Database Entry objects for\n"
                "    Ensembl Gene '%S'\n",
                ajListGetLength(dbes),
                stableid);

    while (ajListPop(dbes, (void **) &dbe))
    {
        ajFmtPrintF(aoutfile,
                    "\n"
                    "    Ensembl Database Entry\n"
                    "      Database Display Name '%S'\n"
                    "      Database Name '%S'\n"
                    "      Database Release '%S'\n"
                    "      Primary Identifier '%S'\n"
                    "      Display Identifier '%S'\n"
                    "      Version '%S'\n",
                    ensDatabaseentryGetDbDisplayname(dbe),
                    ensDatabaseentryGetDbName(dbe),
                    ensDatabaseentryGetDbRelease(dbe),
                    ensDatabaseentryGetPrimaryidentifier(dbe),
                    ensDatabaseentryGetDisplayidentifier(dbe),
                    ensDatabaseentryGetVersion(dbe));

        ensDatabaseentryDel(&dbe);
    }

    ensGeneDel(&gene);

    ajStrDel(&stableid);

    ajListFree(&dbes);

    ajStrDel(&name);
    ajStrDel(&dbname);

    return ajTrue;
}




/* @funcstatic ensembltest_feature ********************************************
**
** Ensembl Feature tests.
**
** @param [u] dba [EnsPDatabaseadaptor] Ensembl Database Adaptor
** @param [u] aoutfile [AjPFile] AJAX File
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
** @@
******************************************************************************/

static AjBool ensembltest_feature(EnsPDatabaseadaptor dba,
                                  AjPFile aoutfile)
{
    AjIList iterator    = NULL;
    AjPList exons       = NULL;
    AjPList genes       = NULL;
    AjPList transcripts = NULL;

    const AjPList features = NULL;

    AjPStr featurelines = NULL;
    AjPStr fslname      = NULL;
    AjPStr oslname      = NULL;

    EnsPExon        exon        = NULL;
    EnsPExonadaptor exonadaptor = NULL;

    EnsPFeature feature = NULL;

    EnsPGene gene      = NULL;
    EnsPGeneadaptor ga = NULL;

    EnsPSlice slice      = NULL;
    EnsPSliceadaptor sla = NULL;

    EnsPTranscript transcript = NULL;
    EnsPTranscriptadaptor tca = NULL;

    if (!dba)
        return ajFalse;

    if (!aoutfile)
        return ajFalse;

    ajFmtPrintF(aoutfile,
                "\n"
                "Ensembl Feature\n");

    ajUser("Ensembl Feature");

    sla = ensRegistryGetSliceadaptor(dba);

    /*
    ** Fetch an Ensembl Slice for chromosome:GRCh37:18:45300001:45500000:1,
    ** which is inside SMAD2.
    */

    oslname = ajStrNewC("chromosome:GRCh37:18:45300001:45500000:1");

    ensSliceadaptorFetchByName(sla, oslname, &slice);

    /* Fetch all Exon objects on this Slice. */

    exonadaptor = ensRegistryGetExonadaptor(dba);

    exons = ajListNew();

    ensExonadaptorFetchAllbySlice(exonadaptor, slice, (AjPStr) NULL, exons);

    ensListExonSortStartAscending(exons);

    ensSliceFetchName(slice, &fslname);

    ajFmtPrintF(aoutfile,
                "\n"
                "  Ensembl Slice '%S' has %Lu "
                "Ensembl Exon object(s)\n"
                "\n",
                fslname,
                ajListGetLength(exons));

    while (ajListPop(exons, (void**) &exon))
    {
        feature = ensExonGetFeature(exon);

        ensembltest_document_feature(feature, 3U, &featurelines);

        ajFmtPrintF(aoutfile,
                    "    Ensembl Exon %u '%S'\n"
                    "%S"
                    "\n",
                    ensExonGetIdentifier(exon),
                    ensExonGetStableidentifier(exon),
                    featurelines);

        ensExonDel(&exon);
    }

    ajListFree(&exons);

    /* Fetch all Ensembl Transcript objects on this Ensembl Slice. */

    tca = ensRegistryGetTranscriptadaptor(dba);

    transcripts = ajListNew();

    ensTranscriptadaptorFetchAllbySlice(tca,
                                        slice,
                                        (AjPStr) NULL,
                                        (AjPStr) NULL,
                                        ajTrue,
                                        transcripts);

    ensListTranscriptSortStartAscending(transcripts);

    ensSliceFetchName(slice, &fslname);

    ajFmtPrintF(aoutfile,
                "\n"
                "  Ensembl Slice '%S' has %Lu "
                "Ensembl Transcript object(s)\n"
                "\n",
                fslname,
                ajListGetLength(transcripts));

    while (ajListPop(transcripts, (void**) &transcript))
    {
        feature = ensTranscriptGetFeature(transcript);

        ensembltest_document_feature(feature, 2U, &featurelines);

        ajFmtPrintF(aoutfile,
                    "    Ensembl Transcript %u '%S'\n"
                    "%S"
                    "\n",
                    ensTranscriptGetIdentifier(transcript),
                    ensTranscriptGetStableidentifier(transcript),
                    featurelines);

        features = ensTranscriptLoadExons(transcript);

        iterator = ajListIterNewread(features);

        while (!ajListIterDone(iterator))
        {
            exon = (EnsPExon) ajListIterGet(iterator);

            feature = ensExonGetFeature(exon);

            ensembltest_document_feature(feature, 3U, &featurelines);

            ajFmtPrintF(aoutfile,
                        "      Ensembl Exon %u '%S'\n"
                        "%S"
                        "\n",
                        ensExonGetIdentifier(exon),
                        ensExonGetStableidentifier(exon),
                        featurelines);
        }

        ajListIterDel(&iterator);

        /* TODO: List the Ensembl Translation objects */

        ensTranscriptDel(&transcript);
    }

    ajListFree(&transcripts);

    /* Fetch all Ensembl Gene objects on this Ensembl Slice. */

    ga = ensRegistryGetGeneadaptor(dba);

    genes = ajListNew();

    ensGeneadaptorFetchAllbySlice(ga,
                                  slice,
                                  (AjPStr) NULL,
                                  (AjPStr) NULL,
                                  (AjPStr) NULL,
                                  ajTrue,
                                  genes);

    ensListGeneSortStartAscending(genes);

    ensSliceFetchName(slice, &fslname);

    ajFmtPrintF(aoutfile,
                "\n"
                "  Ensembl Slice '%S' has %Lu "
                "Ensembl Gene object(s)\n"
                "\n",
                fslname,
                ajListGetLength(genes));

    while (ajListPop(genes, (void**) &gene))
    {
        feature = ensGeneGetFeature(gene);

        ensembltest_document_feature(feature, 2U, &featurelines);

        ajFmtPrintF(aoutfile,
                    "    Ensembl Gene %u '%S'\n"
                    "%S"
                    "\n",
                    ensGeneGetIdentifier(gene),
                    ensGeneGetStableidentifier(gene),
                    featurelines);

        features = ensGeneLoadTranscripts(gene);

        iterator = ajListIterNewread(features);

        while (!ajListIterDone(iterator))
        {
            transcript = (EnsPTranscript) ajListIterGet(iterator);

            feature = ensTranscriptGetFeature(transcript);

            ensembltest_document_feature(feature, 3U, &featurelines);

            ajFmtPrintF(aoutfile,
                        "      Ensembl Transcript %u '%S'\n"
                        "%S"
                        "\n",
                        ensTranscriptGetIdentifier(transcript),
                        ensTranscriptGetStableidentifier(transcript),
                        featurelines);
        }

        ajListIterDel(&iterator);

        ensGeneDel(&gene);
    }

    ajListFree(&genes);

    ensSliceDel(&slice);

    ajStrDel(&featurelines);
    ajStrDel(&fslname);
    ajStrDel(&oslname);

    return ajTrue;
}




/* @funcstatic ensembltest_genes **********************************************
**
** Fetch all Ensembl Gene objects and write sequences of Transcript,
** Translation and Exon objects to disk.
**
** The number of Ensembl Gene objects processed can be configured by the
** maxnum ACD input parameter. The sequences of Exon, Transcript and
** Translation objects are written to FASTA files specified by the
** boutseqall, coutseqall and doutseqall ACD output parameters, respectively.
**
** @param [u] dba [EnsPDatabaseadaptor] Ensembl Database Adaptor
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
** @@
******************************************************************************/

static AjBool ensembltest_genes(EnsPDatabaseadaptor dba)
{
    register ajint i = 0;

    ajint maxnum = 0;

    AjBool debug = AJFALSE;

    AjIList iterator = NULL;
    AjPList exons    = NULL;
    AjPList genes    = NULL;

    const AjPList transcripts = NULL;

    AjPSeq seq = NULL;

    AjPSeqout boutseqall = NULL; /* Ensembl Exon sequences */
    AjPSeqout coutseqall = NULL; /* Ensembl Transcript sequences */
    AjPSeqout doutseqall = NULL; /* Ensembl Translation sequences */

    EnsPExon exon = NULL;

    EnsPGene gene      = NULL;
    EnsPGeneadaptor ga = NULL;

    EnsPTranscript transcript = NULL;

    EnsPTranslation translation = NULL;

    debug = ajDebugTest("ensembltest_genes");

    if (!dba)
        return ajFalse;

    maxnum = ajAcdGetInt("maxnum");

    boutseqall = ajAcdGetSeqoutall("boutseqall");
    coutseqall = ajAcdGetSeqoutall("coutseqall");
    doutseqall = ajAcdGetSeqoutall("doutseqall");

    ajUser("Ensembl Genes");

    ga = ensRegistryGetGeneadaptor(dba);

    /* Fetch all Genes. */

    exons = ajListNew();
    genes = ajListNew();

    ensGeneadaptorFetchAll(ga, genes);

    /*
    ** Although Ensembl Gene objects have not been retrieved from an
    ** Ensembl Slice, the following function can still sort them by
    ** Slice objects and then Slice start coordinates.
    */

    ensListGeneSortStartAscending(genes);

    while (ajListPop(genes, (void**) &gene))
    {
        /* Check if a limit has been set and if it has not been exceeded. */

        if ((maxnum < 0) || (i < maxnum))
        {
            if (debug)
                ajDebug("ensembltest_genes Ensembl Gene "
                        "%u stable identifier '%S'\n",
                        ensGeneGetIdentifier(gene),
                        ensGeneGetStableidentifier(gene));

            /* Get all Transcript objects of this Gene. */

            transcripts = ensGeneLoadTranscripts(gene);

            iterator = ajListIterNewread(transcripts);

            while (!ajListIterDone(iterator))
            {
                transcript = (EnsPTranscript) ajListIterGet(iterator);

                if (debug)
                    ajDebug("ensembltest_genes Ensembl Transcript "
                            "%u stable identifier '%S'\n",
                            ensTranscriptGetIdentifier(transcript),
                            ensTranscriptGetStableidentifier(transcript));

                /* Fetch and write the sequence of this Transcript. */

                ensTranscriptFetchSequenceTranscriptSeq(transcript, &seq);

                ajSeqoutWriteSeq(coutseqall, seq);

                ajSeqDel(&seq);

                /* Get the Translation of this Transcript. */

                translation = ensTranscriptLoadTranslation(transcript);

                /* Not every Transcript has a Translation. */

                /*
                ** FIXME: EMBOSS crashes if the AJAX Sequence object is not
                ** defined.
                */

                if (translation)
                {
                    if (debug)
                        ajDebug(
                            "ensembltest_genes Ensembl Translation "
                            "%u stable identifier '%S'\n",
                            ensTranslationGetIdentifier(translation),
                            ensTranslationGetStableidentifier(translation));

                    /*
                    ** Fetch and write the sequence of this
                    ** Ensembl Translation.
                    */

                    ensTranslationFetchSequenceSeq(translation, &seq);

                    ajSeqoutWriteSeq(doutseqall, seq);

                    ajSeqDel(&seq);
                }
            }

            ajListIterDel(&iterator);

            /* Fetch all Ensembl Exon objects of this Ensembl Gene. */

            ensGeneFetchAllExons(gene, exons);

            while (ajListPop(exons, (void**) &exon))
            {
                if (debug)
                    ajDebug("ensembltest_genes Ensembl Exon "
                            "%u stable identifier '%S'\n",
                            ensExonGetIdentifier(exon),
                            ensExonGetStableidentifier(exon));

                /* Fetch and write the sequence of this Exon. */

                ensExonFetchSequenceSliceSeq(exon, &seq);

                ajSeqoutWriteSeq(boutseqall, seq);

                ajSeqDel(&seq);

                ensExonDel(&exon);
            }
        }

        ensGeneDel(&gene);

        i++;
    }

    ajListFree(&exons);
    ajListFree(&genes);

    ajSeqoutClose(boutseqall);
    ajSeqoutClose(coutseqall);
    ajSeqoutClose(doutseqall);

    ajSeqoutDel(&boutseqall);
    ajSeqoutDel(&coutseqall);
    ajSeqoutDel(&doutseqall);

    return ajTrue;
}




/* @funcstatic ensembltest_marker *********************************************
**
** Ensembl Marker and Ensembl Marker Feature tests.
**
** @param [u] dba [EnsPDatabaseadaptor] Ensembl Database Adaptor
** @param [u] aoutfile [AjPFile] AJAX File
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
** @@
******************************************************************************/

static AjBool ensembltest_marker(EnsPDatabaseadaptor dba,
                                 AjPFile aoutfile)
{
    AjPList mfs = NULL;

    AjPStr fslname = NULL;
    AjPStr oslname = NULL;

    EnsPAnalysis analysis = NULL;

    EnsPFeature feature = NULL;

    EnsPMarkerfeature mf         = NULL;
    EnsPMarkerfeatureadaptor mfa = NULL;

    EnsPMarker marker = NULL;

    EnsPMarkersynonym ms = NULL;

    EnsPSlice slice      = NULL;
    EnsPSliceadaptor sla = NULL;

    if (!dba)
        return ajFalse;

    if (!aoutfile)
        return ajFalse;

    ajFmtPrintF(aoutfile,
                "\n"
                "Ensembl Marker\n");

    ajUser("Ensembl Marker");

    sla = ensRegistryGetSliceadaptor(dba);

    /*
    ** Fetch an Ensembl Slice for chromosome:GRCh37:18:45000001:46000000:1,
    ** which is around SMAD2.
    */

    oslname = ajStrNewC("chromosome:GRCh37:18:45000001:46000000:1");

    ensSliceFetchName(slice, &oslname);

    ensSliceadaptorFetchByName(sla, oslname, &slice);

    /* Fetch all Marker Feature objects on this Slice. */

    mfa = ensRegistryGetMarkerfeatureadaptor(dba);

    mfs = ajListNew();

    ensMarkerfeatureadaptorFetchAllbySlice(mfa,
                                           slice,
                                           0,
                                           0,
                                           (AjPStr) NULL,
                                           mfs);

    ensListMarkerfeatureSortStartAscending(mfs);

    ensSliceFetchName(slice, &fslname);

    ajFmtPrintF(aoutfile,
                "\n"
                "  Ensembl Slice '%S' has %Lu "
                "Ensembl Marker Feature object(s)\n"
                "\n",
                fslname,
                ajListGetLength(mfs));

    while (ajListPop(mfs, (void**) &mf))
    {
        feature = ensMarkerfeatureGetFeature(mf);

        analysis = ensFeatureGetAnalysis(feature);

        marker = ensMarkerfeatureGetMarker(mf);

        ms = ensMarkerGetDisplaysynonym(marker);

        ensSliceFetchName(ensFeatureGetSlice(feature), &fslname);

        ajFmtPrintF(aoutfile,
                    "    Ensembl Marker Feature %u mapweight %d\n"
                    "      Ensembl Slice '%S' coordinates '%d:%d:%d'\n"
                    "      Sequence Region coordinates '%S:%d:%d:%d'\n"
                    "      Ensembl Analysis %u name '%S'\n"
                    "      Ensembl Marker %u type '%s'\n"
                    "      Ensembl Marker Synonym %u name '%S' source '%S'\n"
                    "\n",
                    ensMarkerfeatureGetIdentifier(mf),
                    ensMarkerfeatureGetMapweight(mf),
                    fslname,
                    ensFeatureGetStart(feature),
                    ensFeatureGetEnd(feature),
                    /* ensFeatureGetStrand(feature), */
                    0,
                    ensFeatureGetSeqregionName(feature),
                    ensFeatureGetSeqregionStart(feature),
                    ensFeatureGetSeqregionEnd(feature),
                    /* ensFeatureGetSeqregionStrand(feature), */
                    0,
                    ensAnalysisGetIdentifier(analysis),
                    ensAnalysisGetName(analysis),
                    ensMarkerGetIdentifier(marker),
                    ensMarkerTypeToChar(ensMarkerGetType(marker)),
                    ensMarkersynonymGetIdentifier(ms),
                    ensMarkersynonymGetName(ms),
                    ensMarkersynonymGetSource(ms));

        ensMarkerfeatureDel(&mf);
    }

    ajListFree(&mfs);

    ajStrDel(&fslname);
    ajStrDel(&oslname);

    ensSliceDel(&slice);

    return ajTrue;
}




/* @funcstatic ensembltest_ontology *******************************************
**
** Ensembl Ontology and Ensembl Ontology Term tests.
**
** @param [u] dba [EnsPDatabaseadaptor] Ensembl Database Adaptor
** @param [u] aoutfile [AjPFile] AJAX File
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
** @@
******************************************************************************/

static AjBool ensembltest_ontology(EnsPDatabaseadaptor dba,
                                   AjPFile aoutfile)
{
    AjPList cots = NULL; /* Child Ontology Term objects */
    AjPList pots = NULL; /* Parent Ontology Term objects */

    AjPStr accession    = NULL;
    AjPStr termname     = NULL;
    AjPStr ontologyname = NULL;
    AjPStr species      = NULL;

    EnsPDatabaseadaptor eodba = NULL;

    EnsPOntologyterm       cot  = NULL;
    EnsPOntologyterm       pot  = NULL;
    EnsPOntologytermadaptor ota = NULL;

    if (!dba)
        return ajFalse;

    if (!aoutfile)
        return ajFalse;

    ajFmtPrintF(aoutfile,
                "\n"
                "Ensembl Ontology\n");

    ajUser("Ensembl Ontology");

    species = ajStrNewC("default");

    eodba = ensRegistryGetDatabaseadaptor(ensEDatabaseadaptorGroupOntology,
                                          species);

    ajStrDel(&species);

    ota = ensRegistryGetOntologytermadaptor(eodba);

    cots = ajListNew();
    pots = ajListNew();

    /* Fetch by an accession. */

    accession = ajStrNewC("GO:0046332");

    ensOntologytermadaptorFetchByAccession(ota, accession, &cot);

    ajStrDel(&accession);

    ajFmtPrintF(aoutfile,
                "\n"
                "  Fetch all by child Ensembl Ontology Term\n");

    ensOntologytermadaptorFetchAllbyChild(ota, cot, pots);

    ajFmtPrintF(aoutfile,
                "\n"
                "  Child Ensembl Ontology Term\n"
                "    Accession '%S'\n"
                "    Name '%S'\n"
                "    Definition '%S'\n"
                "    Ontology '%S'\n",
                ensOntologytermGetAccession(cot),
                ensOntologytermGetName(cot),
                ensOntologytermGetDefinition(cot),
                ensOntologyGetName(ensOntologytermGetOntology(cot)));

    while (ajListPop(pots, (void**) &pot))
    {
        ajFmtPrintF(aoutfile,
                    "\n"
                    "    Parent Ensembl Ontology Term\n"
                    "      Accession '%S'\n"
                    "      Name '%S'\n"
                    "      Definition '%S'\n"
                    "      Ontology '%S'\n",
                    ensOntologytermGetAccession(pot),
                    ensOntologytermGetName(pot),
                    ensOntologytermGetDefinition(pot),
                    ensOntologyGetName(ensOntologytermGetOntology(pot)));

        ensOntologytermDel(&pot);
    }

    ajFmtPrintF(aoutfile,
                "\n"
                "  Fetch all by descendant Ensembl Ontology Term\n");

    /* TODO: Also run with closest and zerodistance set to ajTrue. */

    ensOntologytermadaptorFetchAllbyDescendant(ota,
                                               cot,
                                               (const AjPStr) NULL,
                                               ajFalse,
                                               ajFalse,
                                               pots);

    ajFmtPrintF(aoutfile,
                "\n"
                "  Descendant Ensembl Ontology Term\n"
                "    Accession '%S'\n"
                "    Name '%S'\n"
                "    Definition '%S'\n"
                "    Ontology '%S'\n",
                ensOntologytermGetAccession(cot),
                ensOntologytermGetName(cot),
                ensOntologytermGetDefinition(cot),
                ensOntologyGetName(ensOntologytermGetOntology(cot)));

    while (ajListPop(pots, (void**) &pot))
    {
        ajFmtPrintF(aoutfile,
                    "\n"
                    "    Ancestor Ensembl Ontology Term\n"
                    "      Accession '%S'\n"
                    "      Name '%S'\n"
                    "      Definition '%S'\n"
                    "      Ontology '%S'\n",
                    ensOntologytermGetAccession(pot),
                    ensOntologytermGetName(pot),
                    ensOntologytermGetDefinition(pot),
                    ensOntologyGetName(ensOntologytermGetOntology(pot)));

        ensOntologytermDel(&pot);
    }

    ajFmtPrintF(aoutfile,
                "\n"
                "  Fetch all by parent Ensembl Ontology Term\n");

    ensOntologytermadaptorFetchAllbyParent(ota, cot, pots);

    ajFmtPrintF(aoutfile,
                "\n"
                "  Parent Ensembl Ontology Term\n"
                "    Accession '%S'\n"
                "    Name '%S'\n"
                "    Definition '%S'\n"
                "    Ontology '%S'\n",
                ensOntologytermGetAccession(cot),
                ensOntologytermGetName(cot),
                ensOntologytermGetDefinition(cot),
                ensOntologyGetName(ensOntologytermGetOntology(cot)));

    while (ajListPop(pots, (void**) &pot))
    {
        ajFmtPrintF(aoutfile,
                    "\n"
                    "    Child Ensembl Ontology Term\n"
                    "      Accession '%S'\n"
                    "      Name '%S'\n"
                    "      Definition '%S'\n"
                    "      Ontology '%S'\n",
                    ensOntologytermGetAccession(pot),
                    ensOntologytermGetName(pot),
                    ensOntologytermGetDefinition(pot),
                    ensOntologyGetName(ensOntologytermGetOntology(pot)));

        ensOntologytermDel(&pot);
    }

    ajFmtPrintF(aoutfile,
                "\n"
                "  Fetch all by ancestor Ensembl Ontology Term\n");

    ensOntologytermadaptorFetchAllbyAncestor(ota, cot, pots);

    ajFmtPrintF(aoutfile,
                "\n"
                "  Ancestor Ensembl Ontology Term\n"
                "    Accession '%S'\n"
                "    Name '%S'\n"
                "    Definition '%S'\n"
                "    Ontology '%S'\n",
                ensOntologytermGetAccession(cot),
                ensOntologytermGetName(cot),
                ensOntologytermGetDefinition(cot),
                ensOntologyGetName(ensOntologytermGetOntology(cot)));

    while (ajListPop(pots, (void**) &pot))
    {
        ajFmtPrintF(aoutfile,
                    "\n"
                    "    Descendant Ensembl Ontology Term\n"
                    "      Accession '%S'\n"
                    "      Name '%S'\n"
                    "      Definition '%S'\n"
                    "      Ontology '%S'\n",
                    ensOntologytermGetAccession(pot),
                    ensOntologytermGetName(pot),
                    ensOntologytermGetDefinition(pot),
                    ensOntologyGetName(ensOntologytermGetOntology(pot)));

        ensOntologytermDel(&pot);
    }

    ensOntologytermDel(&cot);

    /* Fetch by a name. */

    ajFmtPrintF(aoutfile,
                "\n"
                "  Fetch all by Ensembl Ontology Term name\n");

    termname     = ajStrNewC("%SMAD binding%");
    ontologyname = ajStrNewC("GO");

    ensOntologytermadaptorFetchAllbyName(ota,
                                         termname,
                                         ontologyname,
                                         cots);

    ajStrDel(&termname);
    ajStrDel(&ontologyname);

    while (ajListPop(cots, (void**) &cot))
    {
        ajFmtPrintF(aoutfile,
                    "\n"
                    "  Child Ensembl Ontology Term\n"
                    "    Accession '%S'\n"
                    "    Name '%S'\n"
                    "    Definition '%S'\n"
                    "    Ontology '%S'\n",
                    ensOntologytermGetAccession(cot),
                    ensOntologytermGetName(cot),
                    ensOntologytermGetDefinition(cot),
                    ensOntologyGetName(ensOntologytermGetOntology(cot)));

        ensOntologytermadaptorFetchAllbyChild(ota, cot, pots);

        while (ajListPop(pots, (void**) &pot))
        {
            ajFmtPrintF(aoutfile,
                        "\n"
                        "    Parent Ensembl Ontology Term\n"
                        "      Accession '%S'\n"
                        "      Name '%S'\n"
                        "      Definition '%S'\n"
                        "      Ontology '%S'\n",
                        ensOntologytermGetAccession(pot),
                        ensOntologytermGetName(pot),
                        ensOntologytermGetDefinition(pot),
                        ensOntologyGetName(ensOntologytermGetOntology(pot)));

            ensOntologytermDel(&pot);
        }

        ensOntologytermDel(&cot);
    }

    ajListFree(&cots);
    ajListFree(&pots);

    /*
    ** Explicitly disconnect the Ensembl Database Adaptor to the
    ** Ensembl Ontology Database.
    */

    ensDatabaseadaptorDisconnect(eodba);

    return ajTrue;
}




/* @funcstatic ensembltest_operon *********************************************
**
** Ensembl Operon and Ensembl Operon Transcript tests.
**
** @param [u] dba [EnsPDatabaseadaptor] Ensembl Database Adaptor
** @param [u] aoutfile [AjPFile] AJAX File
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
** @@
******************************************************************************/

static AjBool ensembltest_operon(EnsPDatabaseadaptor dba,
                                 AjPFile aoutfile)
{
    AjIList iter1 = NULL;
    AjIList iter2 = NULL;

    const AjPList genes = NULL;
    const AjPList ots   = NULL;

    AjPStr displaylabel = NULL;
    AjPStr featurelines = NULL;

    EnsPFeature feature = NULL;

    EnsPGene gene = NULL;

    EnsPOperon operon = NULL;
    EnsPOperonadaptor opa = NULL;

    EnsPOperontranscript ot = NULL;

    if (!dba)
        return ajFalse;

    if (!aoutfile)
        return ajFalse;

    ajFmtPrintF(aoutfile,
                "\n"
                "Ensembl Operon\n");

    ajUser("Ensembl Operon");

    opa = ensRegistryGetOperonadaptor(dba);

    /* Fetch the lacZYA operon */

    ajFmtPrintF(aoutfile,
                "\n"
                "  Fetch the Ensembl Operon with display label 'lacZYA'\n");

    displaylabel = ajStrNewC("lacZYA");

    ensOperonadaptorFetchByDisplaylabel(opa, displaylabel, &operon);

    feature = ensOperonGetFeature(operon);

    ensembltest_document_feature(feature, 1U, &featurelines);

    ajFmtPrintF(aoutfile,
                "\n"
                "  Ensembl Operon %u\n"
                "%S"
                "    Stable identifier '%S'\n"
                "    Version %u\n"
                "    Display label '%S'\n",
                ensOperonGetIdentifier(operon),
                featurelines,
                ensOperonGetStableidentifier(operon),
                ensOperonGetVersion(operon),
                ensOperonGetDisplaylabel(operon));

    ots = ensOperonLoadOperontranscripts(operon);

    iter1 = ajListIterNewread(ots);

    while (!ajListIterDone(iter1))
    {
        ot = (EnsPOperontranscript) ajListIterGet(iter1);

        feature = ensOperontranscriptGetFeature(ot);

        ensembltest_document_feature(feature, 2U, &featurelines);

        ajFmtPrintF(aoutfile,
                    "\n"
                    "    Ensembl Operon Transcript %u\n"
                    "%S"
                    "      Stable identifier '%S'\n"
                    "      Version %u\n"
                    "      Display label '%S'\n",
                    ensOperontranscriptGetIdentifier(ot),
                    featurelines,
                    ensOperontranscriptGetStableidentifier(ot),
                    ensOperontranscriptGetVersion(ot),
                    ensOperontranscriptGetDisplaylabel(ot));

        genes = ensOperontranscriptLoadGenes(ot);

        iter2 = ajListIterNewread(genes);

        while (!ajListIterDone(iter2))
        {
            gene = (EnsPGene) ajListIterGet(iter2);

            feature = ensGeneGetFeature(gene);

            ensembltest_document_feature(feature, 3U, &featurelines);

            ajFmtPrintF(aoutfile,
                        "\n"
                        "      Ensembl Gene %u\n"
                        "%S"
                        "        Stable identifier '%S'\n"
                        "        Version %u\n",
                        ensGeneGetIdentifier(gene),
                        featurelines,
                        ensGeneGetStableidentifier(gene),
                        ensGeneGetVersion(gene));
        }

        ajListIterDel(&iter2);
    }

    ajListIterDel(&iter1);

    ensOperonDel(&operon);

    ajStrDel(&displaylabel);
    ajStrDel(&featurelines);

    return ajTrue;
}




/* @funcstatic ensembltest_ditag **********************************************
**
** Ensembl Ditag and Ensembl Ditag Feature tests.
**
** @param [u] dba [EnsPDatabaseadaptor] Ensembl Database Adaptor
** @param [u] aoutfile [AjPFile] AJAX File
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
** @@
******************************************************************************/

static AjBool ensembltest_ditag(EnsPDatabaseadaptor dba,
                                AjPFile aoutfile)
{
    AjPList dtfs = NULL;

    AjPStr fslname      = NULL;
    AjPStr oslname      = NULL;
    AjPStr featurelines = NULL;

    EnsPFeature feature = NULL;

    EnsPDitag dt = NULL;

    EnsPDitagfeature        dtf  = NULL;
    EnsPDitagfeatureadaptor dtfa = NULL;

    EnsPSlice slice      = NULL;
    EnsPSliceadaptor sla = NULL;

    if (!dba)
        return ajFalse;

    if (!aoutfile)
        return ajFalse;

    ajFmtPrintF(aoutfile,
                "\n"
                "Ensembl Ditag\n");

    ajUser("Ensembl Ditag");

    sla = ensRegistryGetSliceadaptor(dba);

    /*
    ** Fetch an Ensembl Slice for chromosome:GRCh37:18:45000001:46000000:1,
    ** which is around SMAD2.
    */

    oslname = ajStrNewC("chromosome:GRCh37:18:45000001:46000000:1");

    ensSliceadaptorFetchByName(sla, oslname, &slice);

    /* Fetch all Ensembl Marker Feature objects on this Ensembl Slice. */

    dtfa = ensRegistryGetDitagfeatureadaptor(dba);

    dtfs = ajListNew();

    ensDitagfeatureadaptorFetchAllbySlice(dtfa,
                                          slice,
                                          (AjPStr) NULL,
                                          (AjPStr) NULL,
                                          dtfs);

    ensListDitagfeatureSortStartAscending(dtfs);

    ensSliceFetchName(slice, &fslname);

    ajFmtPrintF(aoutfile,
                "\n"
                "  Ensembl Slice '%S' has %Lu "
                "Ensembl Ditag Feature object(s)\n"
                "\n",
                fslname,
                ajListGetLength(dtfs));

    while (ajListPop(dtfs, (void**) &dtf))
    {
        feature = ensDitagfeatureGetFeature(dtf);

        ensembltest_document_feature(feature, 3U, &featurelines);

        dt = ensDitagfeatureGetDitag(dtf);

        ajFmtPrintF(aoutfile,
                    "    Ensembl Ditag Feature %u side '%s'\n"
                    "%S"
                    "      Ensembl Ditag %u name '%S' type '%S'\n"
                    "\n",
                    ensDitagfeatureGetIdentifier(dtf),
                    ensDitagfeatureSideToChar(ensDitagfeatureGetSide(dtf)),
                    featurelines,
                    ensDitagGetIdentifier(dt),
                    ensDitagGetName(dt),
                    ensDitagGetType(dt));

        ensDitagfeatureDel(&dtf);
    }

    ajListFree(&dtfs);

    ensSliceDel(&slice);

    ajStrDel(&featurelines);
    ajStrDel(&fslname);
    ajStrDel(&oslname);

    return ajTrue;
}




/* @funcstatic ensembltest_masking ********************************************
**
** Fetch an Ensembl Slice and an Ensembl Repeatmasked Slice and write their
** sequences to disk.
**
** chromosome:GRCh37:22:16040001:16120000:1
**
** @param [u] dba [EnsPDatabaseadaptor] Ensembl Database Adaptor
** @param [u] aoutseqall [AjPSeqout] AJAX Sequence Output
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
** @@
******************************************************************************/

static AjBool ensembltest_masking(EnsPDatabaseadaptor dba,
                                  AjPSeqout aoutseqall)
{
    EnsERepeatMaskType mtype = ensERepeatMaskTypeSoft;

    AjPList names = NULL;

    AjPSeq seq = NULL;

    AjPStr slname = NULL;

    AjPTable masking = NULL;

    EnsPSlice slice      = NULL;
    EnsPSliceadaptor sla = NULL;

    EnsPRepeatmaskedslice rmslice = NULL;

    if (!dba)
        return ajFalse;

    if (!aoutseqall)
        return ajFalse;

    ajUser("Ensembl Repeat Masked Slice");

    sla = ensRegistryGetSliceadaptor(dba);

    /* Fetch an Ensembl Slice for chromosome:GRCh37:22:16040001:16120000:1. */

    slname = ajStrNewC("chromosome:GRCh37:22:16040001:16120000:1");

    ensSliceadaptorFetchByName(sla, slname, &slice);

    ajStrDel(&slname);

    /*
    ** Fetch the unmasked genome sequence for Ensembl Slice
    ** chromosome:GRCh37:22:16040001:16120000:1.
    */

    ensSliceFetchSequenceAllSeq(slice, &seq);

    ajSeqAssignDescC(seq, "Ensembl Slice");

    ajSeqoutWriteSeq(aoutseqall, seq);

    ajSeqDel(&seq);

    /*
    ** Fetch the masked genome sequence for Ensembl Slice
    ** chromosome:GRCh37:22:16040001:16120000:1 and all
    ** Ensembl Analysis objects.
    */

    rmslice = ensRepeatmaskedsliceNewIni(slice,
                                         (AjPList) NULL,
                                         (AjPTable) NULL);

    ensRepeatmaskedsliceFetchSequenceSeq(rmslice, mtype, &seq);

    ajSeqAssignDescC(seq, "Ensembl Repeat Masked Slice all Analysis objects");

    ajSeqoutWriteSeq(aoutseqall, seq);

    ajSeqDel(&seq);

    ensRepeatmaskedsliceDel(&rmslice);

    /*
    ** Fetch the masked genome sequence for Ensembl Slice
    ** chromosome:GRCh37:22:16040001:16120000:1 and Analysis "RepeatMasked".
    */

    names = ajListstrNew();

    ajListPushAppend(names, (void*) ajStrNewC("repeatmask"));

    masking = ajTablestrNew(10);

    rmslice = ensRepeatmaskedsliceNewIni(slice, names, masking);

    ensRepeatmaskedsliceFetchSequenceSeq(rmslice, mtype, &seq);

    ajSeqAssignDescC(seq, "Ensembl Repeat Masked Slice RepeatMasker Analysis");

    ajSeqoutWriteSeq(aoutseqall, seq);

    ajSeqDel(&seq);

    ensRepeatmaskedsliceDel(&rmslice);

    ajListstrFreeData(&names);

    ajTableFree(&masking);

    ensSliceDel(&slice);

    return ajTrue;
}




/* @funcstatic ensembltest_sequence *******************************************
**
** Fetch genomic sequences and write them to disk.
**
** An Ensembl Slice contig:GRCh37:AP000522.1:0:0:1, covering the first contig
** of human chromosome 21 is fetched from the database and a sub-sequence,
** which is actually larger than the Slice (i.e. -60 to +60) is written to
** disk.
**
** Fetch a Slice covering a gap region on human chromosome 21.
** chromosome:GRCh37:21:9400001:9800000:1 and again, write a sub-sequence,
** which is longer than the Slice (i.e. -60 to +60).
**
** @param [u] dba [EnsPDatabaseadaptor] Ensembl Database Adaptor
** @param [u] aoutseqall [AjPSeqout] AJAX Sequence Output
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
** @@
******************************************************************************/

static AjBool ensembltest_sequence(EnsPDatabaseadaptor dba,
                                   AjPSeqout aoutseqall)
{
    AjPFeature feature = NULL;

    AjPList genes = NULL;

    AjPSeq seq = NULL;

    AjPStr slname = NULL;
    AjPStr mvalue = NULL;

    EnsPGene gene      = NULL;
    EnsPGeneadaptor ga = NULL;

    EnsPMetainformationadaptor mia = NULL;

    EnsPSlice slice      = NULL;
    EnsPSliceadaptor sla = NULL;

    if (!dba)
        return ajFalse;

    if (!aoutseqall)
        return ajFalse;

    ajUser("Ensembl Sequence");

    ga = ensRegistryGetGeneadaptor(dba);

    mia = ensRegistryGetMetainformationadaptor(dba);

    sla = ensRegistryGetSliceadaptor(dba);

    /* Fetch an Ensembl Slice covering the first contig on chromosome 21. */

    slname = ajStrNewC("contig::AP000522.1:0:0:1");

    ensSliceadaptorFetchByName(sla, slname, &slice);

    /*
    ** Fetch a sub-sequence of this Slice, which is actually larger than the
    ** Slice itself (i.e. -60 to + 60).
    */

    ensSliceFetchSequenceSubSeq(slice,
                                1 - 60,
                                ensSliceCalculateLength(slice) + 60,
                                +1,
                                &seq);

    ajSeqoutWriteSeq(aoutseqall, seq);

    ajSeqDel(&seq);

    ensSliceDel(&slice);

    /*
    ** Fetch a Slice covering a gap region on human chromosome 21.
    ** chromosome:GRCh37:21:9400001:10000000:1
    */

    ajStrAssignC(&slname, "chromosome:GRCh37:21:9400001:10000000:1");

    ensSliceadaptorFetchByName(sla, slname, &slice);

    ajStrDel(&slname);

    ensSliceFetchSequenceSubSeq(slice,
                                1 - 60,
                                ensSliceCalculateLength(slice) + 60,
                                +1,
                                &seq);

    /*
    ** FIXME: Set seq->Molecule, seq->Class and seq->Division.
    ** It does not seem that there are any public accessor functions.
    ** At least the molecule type has to be EMBL/GenBank/DDBJ-specific,
    ** but how could the code know at this point, in which format the
    ** sequence object is going to be written?
    ** The AjPSeqin object would specify whether sequence features are
    ** requested an AjPSeqout object would specify teh output format?
    ** Therefore, bot objects would need to be passed into Ensembl Sequence
    ** functions.
    */

    if (!seq->Fttable)
        seq->Fttable = ajFeattableNewSeq(seq);

    mvalue = ajStrNewC("source");

    feature = ajFeatNewNucFlags(
        seq->Fttable,
        (AjPStr) NULL,
        mvalue,
        1, /* Start */
        ajSeqGetLen(seq), /* End */
        0.0F, /* Score */
        '\0', /* Strand */
        0, /* Frame */
        0, /* Exon number */
        0, /* Start 2 */
        0, /* End 2 */
        (AjPStr) NULL, /* Remote Identifier */
        (AjPStr) NULL, /* Label */
        0); /* Flags */

    ensMetainformationadaptorRetrieveSpeciesScientificname(mia, &mvalue);

    ajFeatTagAddCS(feature, "organism", mvalue);

    ensMetainformationadaptorRetrieveTaxonomyidentifier(mia, &mvalue);

    ajStrInsertC(&mvalue, 0, "taxonomy:");

    ajFeatTagAddCS(feature, "db_xref", mvalue);

    ajFeatTagAddCC(feature, "mol_type", "genomic DNA");

    ajStrDel(&mvalue);

    /*
    ** Fetch all Ensembl Gene objects on this Ensembl Slice and add them as
    ** AJAX Feature objects into an AJAX Feature Table.
    */

    /*
    ** TODO: Ensembl Feature objects are now converted into AJAX Feature
    ** objects on the basis of an AJAX Sequence object. This ascertains that
    ** Ensembl Feature objects are correctly transfered onto an Ensembl Slice
    ** matching the AJAX Sequence object before the actual conversion.
    ** Specififying an AJAX Sequence object also allows for setting the
    ** Sequence Class, Molecule and Type members and initialising the
    ** AJAX Feature Table if not defined. For all these, public AJAX functions
    ** seem to be missing.
    **
    ** TODO: Maybe even better could be passing an AJAX Sequence Output object,
    ** which would know about the requested formats, into the functions that
    ** currently return AJAX Sequence objects. This would allow us to directly
    ** assemble AJAX Feature Tables if required.
    ** ensSliceFetchSequenceAllSeq(EnsPSlice slice,
    **                             AjPSeqout seqout,
    **                             AjPSeq* Psequence)
    ** TODO: Maybe the ensSliceFetchSequenceAllSeqin should just accept an
    ** Ensembl Slice, an AJAX Sequence and an AJAX Sequence Input object.
    ** Rather than returning an new AJAX Sequence object, this could reassign
    ** the relevant elements. This would move the sequence retrieval functions
    ** closer to the EMBOSS standards.
    */

    genes = ajListNew();

    ensGeneadaptorFetchAllbySlice(ga,
                                  slice,
                                  (AjPStr) NULL, /* Ensembl Analysis name */
                                  (AjPStr) NULL, /* Ensembl Gene source */
                                  (AjPStr) NULL, /* Ensembl Gene biotype */
                                  ajTrue, /* load Transcript objects */
                                  genes);

    while (ajListPop(genes, (void**) &gene))
    {
        ensSequenceAddFeatureGene(seq, gene, &feature);
        ensGeneDel(&gene);
    }

    ajListFree(&genes);

    ajSeqoutWriteSeq(aoutseqall, seq);

    ajSeqDel(&seq);

    ensSliceDel(&slice);

    return ajTrue;
}




/* @funcstatic ensembltest_chromosome *****************************************
**
** Fetch a Slice for chromosome:GRCh37:21:0:0:0 and write its sequence to disk.
**
** @param [u] dba [EnsPDatabaseadaptor] Ensembl Database Adaptor
** @param [u] aoutseqall [AjPSeqout] AJAX Sequence Output
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
** @@
******************************************************************************/

static AjBool ensembltest_chromosome(EnsPDatabaseadaptor dba,
                                     AjPSeqout aoutseqall)
{
    AjPSeq seq = NULL;

    AjPStr slname = NULL;

    EnsPSlice slice      = NULL;
    EnsPSliceadaptor sla = NULL;

    if (!dba)
        return ajFalse;

    if (!aoutseqall)
        return ajFalse;

    ajUser("Chromosome");

    sla = ensRegistryGetSliceadaptor(dba);

    /* Fetch a Slice for human chromosome:GRCh37:21:0:0:0 */

    slname = ajStrNewC("chromosome:GRCh37:21:0:0:1");

    ensSliceadaptorFetchByName(sla, slname, &slice);

    ensSliceFetchSequenceAllSeq(slice, &seq);

    ajSeqoutWriteSeq(aoutseqall, seq);

    ajSeqDel(&seq);

    ensSliceDel(&slice);

    /*
    ** Fetch the genome sequence for Monodelphis domestica
    ** chromosome 1, which is 748 Mb (!!!) long.
    */

    /*
      ajStrAssignC(&species, "monodelphis domestica");

      dba = ensRegistryGetDatabaseadaptor(group, species);

      sla = ensRegistryGetSliceadaptor(dba);

      slname = ajStrAssignC(&slname, "chromosome:BROADO5:0:0:1");

      ensSliceadaptorFetchByName(sla, name, &slice);

      ensSliceFetchSequenceAllSeq(slice, &seq);

      ajSeqoutWriteSeq(aoutseqall, seq);

      ajSeqDel(&seq);

      ensSliceDel(&slice);
    */

    ajStrDel(&slname);

    return ajTrue;
}




/* @funcstatic ensembltest_meta_information ***********************************
**
** Ensembl Meta-Information tests.
**
** @param [u] dba [EnsPDatabaseadaptor] Ensembl Database Adaptor
** @param [u] aoutfile [AjPFile] AJAX File
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
** @@
******************************************************************************/

static AjBool ensembltest_meta_information(EnsPDatabaseadaptor dba,
                                           AjPFile aoutfile)
{
    AjPStr value = NULL;

    EnsPMetainformationadaptor mia = NULL;

    if (!dba)
        return ajFalse;

    if (!aoutfile)
        return ajFalse;

    ajFmtPrintF(aoutfile,
                "\n"
                "Ensembl Meta-Information\n");

    ajUser("Ensembl Meta-Information");

    mia = ensRegistryGetMetainformationadaptor(dba);

    value = ajStrNew();

    if (ensMetainformationadaptorRetrieveGenebuildversion(mia, &value))
        ajFmtPrintF(aoutfile,
                    "\n"
                    "  Ensembl Meta-Information Genebuild Version '%S'\n",
                    value);

    if (ensMetainformationadaptorRetrieveSchemaversion(mia, &value))
        ajFmtPrintF(aoutfile,
                    "  Ensembl Meta-Information Schema Version '%S'\n",
                    value);

    if (ensMetainformationadaptorRetrieveTaxonomyidentifier(mia, &value))
        ajFmtPrintF(aoutfile,
                    "  Ensembl Meta-Information "
                    "NCBI Taxonomy identifier '%S'\n",
                    value);

    ajStrDel(&value);

    return ajTrue;
}




/* @funcstatic ensembltest_coordinate_system **********************************
**
** Ensembl Coordinate System tests.
**
** @param [u] dba [EnsPDatabaseadaptor] Ensembl Database Adaptor
** @param [u] aoutfile [AjPFile] AJAX File
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
** @@
******************************************************************************/

static AjBool ensembltest_coordinate_system(EnsPDatabaseadaptor dba,
                                            AjPFile aoutfile)
{
    AjIList iterator = NULL;
    AjPList css      = NULL;

    const AjPList mappath = NULL;

    AjPStr csname1 = NULL;
    AjPStr csname2 = NULL;
    AjPStr csname3 = NULL;

    AjPStr csversion1 = NULL;
    AjPStr csversion2 = NULL;
    AjPStr csversion3 = NULL;

    EnsPCoordsystem cs  = NULL;
    EnsPCoordsystem cs1 = NULL;
    EnsPCoordsystem cs2 = NULL;
    EnsPCoordsystem cs3 = NULL;

    EnsPCoordsystemadaptor csa = NULL;

    if (!dba)
        return ajFalse;

    if (!aoutfile)
        return ajFalse;

    /* Ensembl Coordinate System tests. */

    ajFmtPrintF(aoutfile,
                "\n"
                "Ensembl Coordinate System\n");

    ajUser("Ensembl Coordinate System");

    csa = ensRegistryGetCoordsystemadaptor(dba);

    /* Fetch the top-level Ensembl Coordinate System. */

    ajFmtPrintF(aoutfile,
                "\n"
                "  Top-Level Ensembl Coordinate System\n");

    ensCoordsystemadaptorFetchToplevel(csa, &cs);

    ajFmtPrintF(aoutfile,
                "    Ensembl Coordinate System\n"
                "      identifier %u\n"
                "      rank %u\n"
                "      name:version '%S:%S'\n",
                ensCoordsystemGetIdentifier(cs),
                ensCoordsystemGetRank(cs),
                ensCoordsystemGetName(cs),
                ensCoordsystemGetVersion(cs));

    ensCoordsystemDel(&cs);

    /* Fetch the sequence-level Ensembl Coordinate System. */

    ajFmtPrintF(aoutfile,
                "\n"
                "  Sequence-Level Ensembl Coordinate System\n");

    ensCoordsystemadaptorFetchSeqlevel(csa, &cs);

    ajFmtPrintF(aoutfile,
                "    Ensembl Coordinate System\n"
                "      identifier %u\n"
                "      rank %u\n"
                "      name:version '%S:%S'\n",
                ensCoordsystemGetIdentifier(cs),
                ensCoordsystemGetRank(cs),
                ensCoordsystemGetName(cs),
                ensCoordsystemGetVersion(cs));

    ensCoordsystemDel(&cs);

    /* Fetch all Ensembl Coordinate Systems with name 'chromosome'. */

    ajFmtPrintF(aoutfile,
                "\n"
                "  All Ensembl Coordinate System objects "
                "with name 'chromosome'\n");

    csname1 = ajStrNewC("chromosome");

    css = ajListNew();

    ensCoordsystemadaptorFetchAllbyName(csa, csname1, css);

    ensListCoordsystemSortIdentifierAscending(css);

    while (ajListPop(css, (void**) &cs))
    {
        ajFmtPrintF(aoutfile,
                    "    Ensembl Coordinate System\n"
                    "      identifier %u\n"
                    "      rank %u\n"
                    "      name:version '%S:%S'\n",
                    ensCoordsystemGetIdentifier(cs),
                    ensCoordsystemGetRank(cs),
                    ensCoordsystemGetName(cs),
                    ensCoordsystemGetVersion(cs));

        ensCoordsystemDel(&cs);
    }

    ajListFree(&css);

    ajStrDel(&csname1);

    /*
    ** Fetch the Ensembl Coordinate System with
    ** name 'chromosome' and version 'NCBI35'.
    */

    ajFmtPrintF(aoutfile,
                "\n"
                "  Ensembl Coordinate System with "
                "name 'chromosome' and version 'NCBI35'\n");

    csname1    = ajStrNewC("chromosome");
    csversion1 = ajStrNewC("NCBI35");

    ensCoordsystemadaptorFetchByName(csa, csname1, csversion1, &cs);

    ajFmtPrintF(aoutfile,
                "    Ensembl Coordinate System\n"
                "      identifier %u\n"
                "      rank %u\n"
                "      name:version '%S:%S'\n",
                ensCoordsystemGetIdentifier(cs),
                ensCoordsystemGetRank(cs),
                ensCoordsystemGetName(cs),
                ensCoordsystemGetVersion(cs));

    ensCoordsystemDel(&cs);

    ajStrDel(&csname1);
    ajStrDel(&csversion1);

    /* Fetch the (default) Ensembl Coordinate System with name 'chromosome'. */

    ajFmtPrintF(aoutfile,
                "\n"
                "  Default Ensembl Coordinate System with name 'chromosome'\n");

    csname1 = ajStrNewC("chromosome");

    ensCoordsystemadaptorFetchByName(csa, csname1, (AjPStr) NULL, &cs);

    ajFmtPrintF(aoutfile,
                "    Ensembl Coordinate System\n"
                "      identifier %u\n"
                "      rank %u\n"
                "      name:version '%S:%S'\n",
                ensCoordsystemGetIdentifier(cs),
                ensCoordsystemGetRank(cs),
                ensCoordsystemGetName(cs),
                ensCoordsystemGetVersion(cs));

    ensCoordsystemDel(&cs);

    ajStrDel(&csname1);

    /* Fetch all Ensembl Coordinate Systems. */

    ajFmtPrintF(aoutfile,
                "\n"
                "  All Ensembl Coordinate System objects\n");

    css = ajListNew();

    ensCoordsystemadaptorFetchAll(csa, css);

    ensListCoordsystemSortIdentifierAscending(css);

    while (ajListPop(css, (void**) &cs))
    {
        ajFmtPrintF(aoutfile,
                    "    Ensembl Coordinate System\n"
                    "      identifier %u\n"
                    "      rank %u\n"
                    "      name:version '%S:%S'\n",
                    ensCoordsystemGetIdentifier(cs),
                    ensCoordsystemGetRank(cs),
                    ensCoordsystemGetName(cs),
                    ensCoordsystemGetVersion(cs));

        ensCoordsystemDel(&cs);
    }

    ajListFree(&css);

    /* Fetch specific Ensembl Coordinate Systems. */

    csname1    = ajStrNewC("chromosome");
    csversion1 = ajStrNewC("GRCh37");

    csname2    = ajStrNewC("contig");
    csversion2 = ajStrNewC("");

    csname3    = ajStrNewC("clone");
    csversion3 = ajStrNewC("");

    ensCoordsystemadaptorFetchByName(csa, csname1, csversion1, &cs1);
    ensCoordsystemadaptorFetchByName(csa, csname2, csversion2, &cs2);
    ensCoordsystemadaptorFetchByName(csa, csname3, csversion3, &cs3);

    ajStrDel(&csname1);
    ajStrDel(&csversion1);

    ajStrDel(&csname2);
    ajStrDel(&csversion2);

    ajStrDel(&csname3);
    ajStrDel(&csversion3);

    /* Fetch Ensembl Coordinate System mapping paths cs1 - cs2. */

    ajFmtPrintF(aoutfile,
                "\n"
                "  Ensembl Coordinate System Mapping path "
                "'%S:%S' - '%S:%S'\n",
                ensCoordsystemGetName(cs1),
                ensCoordsystemGetVersion(cs1),
                ensCoordsystemGetName(cs2),
                ensCoordsystemGetVersion(cs2));

    mappath = ensCoordsystemadaptorGetMappingpath(csa, cs1, cs2);

    iterator = ajListIterNewread(mappath);

    while (!ajListIterDone(iterator))
    {
        cs = (EnsPCoordsystem) ajListIterGet(iterator);

        ajFmtPrintF(aoutfile,
                    "    Ensembl Coordinate System\n"
                    "      identifier %u\n"
                    "      rank %u\n"
                    "      name:version '%S:%S'\n",
                    ensCoordsystemGetIdentifier(cs),
                    ensCoordsystemGetRank(cs),
                    ensCoordsystemGetName(cs),
                    ensCoordsystemGetVersion(cs));
    }

    ajListIterDel(&iterator);

    /* Fetch Ensembl Coordinate System mapping paths cs1 - cs3 */

    ajFmtPrintF(aoutfile,
                "\n"
                "  Ensembl Coordinate System Mapping path "
                "'%S:%S' - '%S:%S'\n",
                ensCoordsystemGetName(cs1),
                ensCoordsystemGetVersion(cs1),
                ensCoordsystemGetName(cs3),
                ensCoordsystemGetVersion(cs3));

    mappath = ensCoordsystemadaptorGetMappingpath(csa, cs1, cs3);

    iterator = ajListIterNewread(mappath);

    while (!ajListIterDone(iterator))
    {
        cs = (EnsPCoordsystem) ajListIterGet(iterator);

        ajFmtPrintF(aoutfile,
                    "    Ensembl Coordinate System\n"
                    "      identifier %u\n"
                    "      rank %u\n"
                    "      name:version '%S:%S'\n",
                    ensCoordsystemGetIdentifier(cs),
                    ensCoordsystemGetRank(cs),
                    ensCoordsystemGetName(cs),
                    ensCoordsystemGetVersion(cs));
    }

    ajListIterDel(&iterator);

    /* Fetch Ensembl Coordinate System mapping paths cs2 - cs3 */

    ajFmtPrintF(aoutfile,
                "\n"
                "  Ensembl Coordinate System Mapping path "
                "'%S:%S' - '%S:%S'\n",
                ensCoordsystemGetName(cs2),
                ensCoordsystemGetVersion(cs2),
                ensCoordsystemGetName(cs3),
                ensCoordsystemGetVersion(cs3));

    mappath = ensCoordsystemadaptorGetMappingpath(csa, cs2, cs3);

    iterator = ajListIterNewread(mappath);

    while (!ajListIterDone(iterator))
    {
        cs = (EnsPCoordsystem) ajListIterGet(iterator);

        ajFmtPrintF(aoutfile,
                    "    Ensembl Coordinate System\n"
                    "      identifier %u\n"
                    "      rank %u\n"
                    "      name:version '%S:%S'\n",
                    ensCoordsystemGetIdentifier(cs),
                    ensCoordsystemGetRank(cs),
                    ensCoordsystemGetName(cs),
                    ensCoordsystemGetVersion(cs));
    }

    ajListIterDel(&iterator);

    ensCoordsystemDel(&cs1);
    ensCoordsystemDel(&cs2);
    ensCoordsystemDel(&cs3);

    return ajTrue;
}




/* @funcstatic ensembltest_sequence_region ************************************
**
** Ensembl Sequence Region tests.
**
** @param [u] dba [EnsPDatabaseadaptor] Ensembl Database Adaptor
** @param [u] aoutfile [AjPFile] AJAX File
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
** @@
******************************************************************************/

static AjBool ensembltest_sequence_region(EnsPDatabaseadaptor dba,
                                          AjPFile aoutfile)
{
    AjIList iterator = NULL;

    const AjPList attributes = NULL;

    AjPStr synonym = NULL;

    EnsPAttribute attribute = NULL;

    EnsPCoordsystem cs = NULL;

    EnsPSeqregion        sr  = NULL;
    EnsPSeqregionadaptor sra = NULL;

    if (!dba)
        return ajFalse;

    if (!aoutfile)
        return ajFalse;

    /* Ensembl Sequence Region tests. */

    ajFmtPrintF(aoutfile,
                "\n"
                "Ensembl Sequence Region\n");

    ajUser("Ensembl Sequence Region");

    sra = ensRegistryGetSeqregionadaptor(dba);

    /*
    ** Fetch the human chromosome 1 via its SQL table-internal identifier
    ** seq_region.seq_region_id = 27511.
    ** 42123 BAC clone AC120349.5
    ** 14320 contig AC120349.5
    */

    ensSeqregionadaptorFetchByIdentifier(sra, 27511U, &sr);

    cs = ensSeqregionGetCoordsystem(sr);

    ajFmtPrintF(aoutfile,
                "\n"
                "  Ensembl Sequence Region %u name '%S' length %u '%S:%S'\n",
                ensSeqregionGetIdentifier(sr),
                ensSeqregionGetName(sr),
                ensSeqregionGetLength(sr),
                ensCoordsystemGetName(cs),
                ensCoordsystemGetVersion(cs));

    attributes = ensSeqregionLoadAttributes(sr);

    iterator = ajListIterNewread(attributes);

    while (!ajListIterDone(iterator))
    {
        attribute = (EnsPAttribute) ajListIterGet(iterator);

        ajFmtPrintF(aoutfile,
                    "    Ensembl Attribute\n"
                    "      name  '%S'\n"
                    "      code  '%S'\n"
                    "      value '%S'\n",
                    ensAttributeGetName(attribute),
                    ensAttributeGetCode(attribute),
                    ensAttributeGetValue(attribute));
    }

    ajListIterDel(&iterator);

    ensSeqregionDel(&sr);

    /* CRCh37 chromosome 1 */

    synonym = ajStrNewC("CM000663.1");

    ensSeqregionadaptorFetchBySynonym(sra, synonym, &sr);

    cs = ensSeqregionGetCoordsystem(sr);

    ajFmtPrintF(aoutfile,
                "\n"
                "  Ensembl Sequence Region %u name '%S' length %u '%S:%S'\n",
                ensSeqregionGetIdentifier(sr),
                ensSeqregionGetName(sr),
                ensSeqregionGetLength(sr),
                ensCoordsystemGetName(cs),
                ensCoordsystemGetVersion(cs));

    ensSeqregionDel(&sr);

    ajStrDel(&synonym);

    return ajTrue;
}




/* @funcstatic ensembltest_exon ***********************************************
**
** Ensembl Exon tests.
**
** @param [u] dba [EnsPDatabaseadaptor] Ensembl Database Adaptor
** @param [u] aoutfile [AjPFile] AJAX File
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
** @@
******************************************************************************/

static AjBool ensembltest_exon(EnsPDatabaseadaptor dba,
                               AjPFile aoutfile)
{
    AjPStr featurelines = NULL;
    AjPStr stableid     = NULL;

    EnsPExon exon      = NULL;
    EnsPExonadaptor ea = NULL;

    EnsPFeature feature = NULL;

    if (!dba)
        return ajFalse;

    if (!aoutfile)
        return ajFalse;

    ajFmtPrintF(aoutfile,
                "\n"
                "Ensembl Exon\n");

    ajUser("Ensembl Exon");

    ea = ensRegistryGetExonadaptor(dba);

    /* Fetch Exon ENSE00001434436 */

    stableid = ajStrNewC("ENSE00001191187");

    ensExonadaptorFetchByStableidentifier(ea, stableid, 0, &exon);

    ajStrDel(&stableid);

    feature = ensExonGetFeature(exon);

    ensembltest_document_feature(feature, 2U, &featurelines);

    ajFmtPrintF(aoutfile,
                "\n"
                "  Ensembl Exon %u '%S'\n"
                "%S",
                ensExonGetIdentifier(exon),
                ensExonGetStableidentifier(exon),
                featurelines);

    ensExonDel(&exon);

    ajStrDel(&featurelines);

    return ajTrue;
}




/* @funcstatic ensembltest_transcript *****************************************
**
** Ensembl Transcript tests.
**
** @param [u] dba [EnsPDatabaseadaptor] Ensembl Database Adaptor
** @param [u] aoutfile [AjPFile] AJAX File
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
** @@
******************************************************************************/

static AjBool ensembltest_transcript(EnsPDatabaseadaptor dba,
                                     AjPFile aoutfile)
{
    ajuint trcstart = 2U;
    ajuint trcend   = 3U;

    AjIList iterator = NULL;
    const AjPList exons = NULL;
    AjPList mrs = NULL;

    AjPStr featurelines = NULL;
    AjPStr slname       = NULL;
    AjPStr stableid     = NULL;

    EnsPExon exon = NULL;

    EnsPFeature feature = NULL;

    EnsPMapperresult mr = NULL;

    EnsPTranscript transcript = NULL;
    EnsPTranscriptadaptor tca = NULL;

    EnsPTranslation translation = NULL;

    if (!dba)
        return ajFalse;

    if (!aoutfile)
        return ajFalse;

    ajFmtPrintF(aoutfile,
                "\n"
                "Ensembl Transcript\n");

    ajUser("Ensembl Transcript");

    /* Fetch Transcript ENST00000262160 SMAD2-001 */

    tca = ensRegistryGetTranscriptadaptor(dba);

    stableid = ajStrNewC("ENST00000262160");

    ensTranscriptadaptorFetchByStableidentifier(tca, stableid, 0, &transcript);

    ajStrDel(&stableid);

    if (transcript)
    {
        feature = ensTranscriptGetFeature(transcript);

        ensSliceFetchName(ensFeatureGetSlice(feature), &slname);

        ensembltest_document_feature(feature, 1U, &featurelines);

        ajFmtPrintF(aoutfile,
                    "\n"
                    "  Ensembl Transcript %u '%S'\n"
                    "%S",
                    ensTranscriptGetIdentifier(transcript),
                    ensTranscriptGetStableidentifier(transcript),
                    featurelines);

        /* Transcript Mapper Test */

        mrs = ajListNew();

        ensTranscriptMapperTranscriptToslice(transcript,
                                             trcstart,
                                             trcend,
                                             mrs);

        while (ajListPop(mrs, (void**) &mr))
        {
            switch(ensMapperresultGetType(mr))
            {
                case ensEMapperresultTypeCoordinate:

                    ajFmtPrintF(aoutfile,
                                "  Ensembl Transcript Mapper mapped positions "
                                "%u:%u to Ensembl Slice '%S' coordinates "
                                "'%d:%d:%d'.\n",
                                trcstart, trcend, slname,
                                ensMapperresultGetCoordinateStart(mr),
                                ensMapperresultGetCoordinateEnd(mr),
                                ensMapperresultGetCoordinateStrand(mr));

                    break;

                case ensEMapperresultTypeGap:

                    ajFmtPrintF(aoutfile,
                                "  Ensembl Transcript Mapper mapped positions "
                                "%u:%u to Ensembl Slice '%S' gap "
                                "'%d:%d'.\n",
                                trcstart, trcend, slname,
                                ensMapperresultGetGapStart(mr),
                                ensMapperresultGetGapEnd(mr));

                    break;

                default:

                    ajFmtPrintF(aoutfile,
                                "  Ensembl Transcript Mapper %u:%u to Slice "
                                "got Mapper Result of unexpected type %d.\n",
                                trcstart, trcend,
                                ensMapperresultGetType(mr));
            }

            ensMapperresultDel(&mr);
        }

        ajListFree(&mrs);

        /* Fetch the sequence for the canonical Translation. */

        translation = ensTranscriptLoadTranslation(transcript);

        /*
        ** Fetch all Ensembl Exon objects of this Ensembl Transcript and
        ** write out their coordinates.
        */

        exons = ensTranscriptLoadExons(transcript);

        iterator = ajListIterNewread(exons);

        while (!ajListIterDone(iterator))
        {
            exon = (EnsPExon) ajListIterGet(iterator);

            feature = ensExonGetFeature(exon);

            ensSliceFetchName(ensFeatureGetSlice(feature), &slname);

            ajFmtPrintF(aoutfile,
                        "\n"
                        "  Ensembl Exon %u '%S'\n"
                        "    Transcript %u '%S' coordinates '%u:%u'\n"
                        "    Slice '%S' coordinates '%d:%d:%d'\n"
                        "    Ensembl Translation %u '%S'\n"
                        "      Transcript coordinates '%u:%u'\n"
                        "      Slice coordinates '%u:%u'\n",
                        ensExonGetIdentifier(exon),
                        ensExonGetStableidentifier(exon),
                        ensTranscriptGetIdentifier(transcript),
                        ensTranscriptGetStableidentifier(transcript),
                        ensExonCalculateTranscriptStart(exon, transcript),
                        ensExonCalculateTranscriptEnd(exon, transcript),
                        slname,
                        ensFeatureGetStart(feature),
                        ensFeatureGetEnd(feature),
                        ensFeatureGetStrand(feature),
                        ensTranslationGetIdentifier(translation),
                        ensTranslationGetStableidentifier(translation),
                        ensExonCalculateTranscriptCodingStart(exon,
                                                              transcript,
                                                              translation),
                        ensExonCalculateTranscriptCodingEnd(exon,
                                                            transcript,
                                                            translation),
                        ensExonCalculateSliceCodingStart(exon,
                                                         transcript,
                                                         translation),
                        ensExonCalculateSliceCodingEnd(exon,
                                                       transcript,
                                                       translation));
        }

        ajListIterDel(&iterator);
    }

    ensTranscriptDel(&transcript);

    ajStrDel(&featurelines);
    ajStrDel(&slname);

    return ajTrue;
}




/* @funcstatic ensembltest_translations ***************************************
**
** Ensembl Translation tests.
**
** @param [u] dba [EnsPDatabaseadaptor] Ensembl Database Adaptor
** @param [u] aoutfile [AjPFile] AJAX File
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
** @@
******************************************************************************/

static AjBool ensembltest_translations(EnsPDatabaseadaptor dba,
                                       AjPFile aoutfile)
{
    AjPStr stableid = NULL;

    EnsPTranslation translation = NULL;
    EnsPTranslationadaptor tla  = NULL;

    if (!dba)
        return ajFalse;

    if (!aoutfile)
        return ajFalse;

    /* Fetch Translation ENSP00000349282 SMAD2-201 */

    tla = ensRegistryGetTranslationadaptor(dba);

    stableid = ajStrNewC("ENSP00000349282");

    ensTranslationadaptorFetchByStableidentifier(tla,
                                                 stableid,
                                                 0,
                                                 &translation);

    ajStrDel(&stableid);

    if (translation)
    {
        /*
        ** NOTE: Fetching the sequence directly from a Translation causes the
        ** Transcript to be re-fetched from the database. Transcript objects
        ** have strong references for their Translation objects, but
        ** Transaltion objects do not have any reference to their Transcript.
        ** Use ensTranscriptFetchSequenceTranslationSeq if a Transcript is
        ** already available.
        */

#if AJFALSE
        ensTranslationFetchSequenceSeq(translation, &seq);

        if (seq)
        {
            ajSeqoutWriteSeq(aoutseqall, seq);
            ajSeqDel(&seq);
        }
#endif /* AJFALSE */
    }

    ensTranslationDel(&translation);

    return ajTrue;
}




/* @funcstatic ensembltest_transformations ************************************
**
** Ensembl Feature transformation tests.
**
** @param [u] dba [EnsPDatabaseadaptor] Ensembl Database Adaptor
** @param [u] aoutfile [AjPFile] AJAX File
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
** @@
******************************************************************************/

static AjBool ensembltest_transformations(EnsPDatabaseadaptor dba,
                                          AjPFile aoutfile)
{
    ajuint version = 0U;

    AjBool debug = AJFALSE;

    AjPStr fslname  = NULL;
    AjPStr oslname  = NULL;
    AjPStr stableid = NULL;

    EnsPFeature feature = NULL;

    EnsPGene oldgene   = NULL;
    EnsPGene newgene   = NULL;
    EnsPGeneadaptor ga = NULL;

    EnsPSlice slice      = NULL;
    EnsPSliceadaptor sla = NULL;

    debug = ajDebugTest("ensembltest_transformations");

    if (!dba)
        return ajFalse;

    if (!aoutfile)
        return ajFalse;

    /* Ensembl Feature transfer and transform tests. */

    ajFmtPrintF(aoutfile,
                "\n"
                "Ensembl Feature transfer and transform\n");

    ajUser("Ensembl Feature transfer and transform");

    if (debug)
        ajDebug("ensembltest_transformations\n");

    ga = ensRegistryGetGeneadaptor(dba);

    stableid = ajStrNewC("ENSG00000175387");

    ensGeneadaptorFetchByStableidentifier(ga, stableid, version, &oldgene);

    ensGeneLoadTranscripts(oldgene);

    feature = ensGeneGetFeature(oldgene);

    ensSliceFetchName(ensFeatureGetSlice(feature), &fslname);

    ajFmtPrintF(aoutfile,
                "\n"
                "  Ensembl Gene %u '%S'\n"
                "    Ensembl Slice '%S' coordinates '%d:%d:%d'\n"
                "    Sequence Region coordinates '%S:%d:%d:%d'\n",
                ensGeneGetIdentifier(oldgene),
                ensGeneGetStableidentifier(oldgene),
                fslname,
                ensFeatureGetStart(feature),
                ensFeatureGetEnd(feature),
                ensFeatureGetStrand(feature),
                ensFeatureGetSeqregionName(feature),
                ensFeatureGetSeqregionStart(feature),
                ensFeatureGetSeqregionEnd(feature),
                ensFeatureGetSeqregionStrand(feature));

    sla = ensRegistryGetSliceadaptor(dba);

    oslname = ajStrNewC("chromosome:GRCh37:18:40000000:50000000:1");

    ensSliceadaptorFetchByName(sla, oslname, &slice);

    ensSliceFetchName(slice, &fslname);

    ajFmtPrintF(aoutfile,
                "\n"
                "  Transform to Ensembl Slice '%S'\n",
                fslname);

    newgene = ensGeneTransfer(oldgene, slice);

    feature = ensGeneGetFeature(newgene);

    ensSliceFetchName(ensFeatureGetSlice(feature), &fslname);

    ajFmtPrintF(aoutfile,
                "\n"
                "  Ensembl Gene %u '%S'\n"
                "    Ensembl Slice '%S' coordinates '%d:%d:%d'\n"
                "    Sequence Region coordinates '%S:%d:%d:%d'\n",
                ensGeneGetIdentifier(newgene),
                ensGeneGetStableidentifier(newgene),
                fslname,
                ensFeatureGetStart(feature),
                ensFeatureGetEnd(feature),
                ensFeatureGetStrand(feature),
                ensFeatureGetSeqregionName(feature),
                ensFeatureGetSeqregionStart(feature),
                ensFeatureGetSeqregionEnd(feature),
                ensFeatureGetSeqregionStrand(feature));

    ensSliceDel(&slice);

    ensGeneDel(&newgene);
    ensGeneDel(&oldgene);

    ajStrDel(&fslname);
    ajStrDel(&oslname);
    ajStrDel(&stableid);

    return ajTrue;
}




/* @funcstatic ensembltest_registry *******************************************
**
** Ensembl Registry tests.
**
** @param [u] aoutfile [AjPFile] AJAX File
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
** @@
******************************************************************************/

static AjBool ensembltest_registry(AjPFile aoutfile)
{
    const char* identifiers[] =
        {
            "ENSG00000175387",
            "ENST00000262160",
            "ENSP00000262160",
            "ENSE00001191187",
            "OTTHUMG00000132652",
            "OTTHUMT00000255903",
            "OTTHUMP00000163489",
            "OTTHUME00001467551",
            "ENSMUSG00000024563",
            "ENSMUST00000025453",
            "ENSMUSP00000025453",
            "ENSMUSE00000481173",
            "ENSF00000000000",
            (const char*) NULL
        };

    register ajuint i = 0U;

    EnsEDatabaseadaptorGroup group = ensEDatabaseadaptorGroupNULL;

    AjPStr identifier = NULL;
    AjPStr species    = NULL;

    if (!aoutfile)
        return ajFalse;

    /* Ensembl Registry tests. */

    ajFmtPrintF(aoutfile,
                "\n"
                "Ensembl Registry\n");

    ajUser("Ensembl Registry");

    /* Registry test of stable identifier expressions. */

    ensRegistryIdentifierLoadFile((AjPStr) NULL);

    for (i = 0U; identifiers[i]; i++) {

        identifier = ajStrNewC(identifiers[i]);

        species = ajStrNew();

        ensRegistryIdentifierResolve(identifier, &species, &group);

        ajFmtPrintF(aoutfile,
                    "\n"
                    "  Ensembl stable identifier '%S'\n"
                    "  Ensembl Database Adaptor species '%S'\n"
                    "  Ensembl Database Adaptor group   '%s'\n",
                    identifier,
                    species,
                    ensDatabaseadaptorGroupToChar(group));

        ajStrDel(&identifier);
        ajStrDel(&species);
    }

    return ajTrue;
}




/* @funcstatic ensembltest_density ********************************************
**
** Ensembl Density Type and Ensembl Density Feature tests.
**
** @param [u] dba [EnsPDatabaseadaptor] Ensembl Database Adaptor
** @param [u] aoutfile [AjPFile] AJAX File
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
** @@
******************************************************************************/

static AjBool ensembltest_density(EnsPDatabaseadaptor dba,
                                  AjPFile aoutfile)
{
    float maxratio = 0.0F;

    ajuint blocks = 100U;

    AjBool interpolate = AJFALSE;

    AjPList dfs = NULL;

    AjPStr anname  = NULL;
    AjPStr fslname = NULL;
    AjPStr oslname = NULL;

    EnsPAnalysis analysis = NULL;

    EnsPDensityfeature        df  = NULL;
    EnsPDensityfeatureadaptor dfa = NULL;

    EnsPDensitytype dt = NULL;

    EnsPFeature feature = NULL;

    EnsPSlice slice      = NULL;
    EnsPSliceadaptor sla = NULL;

    if (!dba)
        return ajFalse;

    if (!aoutfile)
        return ajFalse;

    /* Ensembl Registry tests. */

    ajFmtPrintF(aoutfile,
                "\n"
                "Ensembl Density\n");

    ajUser("Ensembl Density");

    /* Registry test of stable identifier expressions. */

    dfa = ensRegistryGetDensityfeatureadaptor(dba);

    sla = ensRegistryGetSliceadaptor(dba);

    oslname = ajStrNewC("toplevel::22:0:0:1");

    ensSliceadaptorFetchByName(sla, oslname, &slice);

    /*
    ** percentagerepeat
    ** genedensity
    ** snpdensity
    ** percentagerepeat
    ** knowngenedensity
    ** percentgc
    */

    anname = ajStrNewC("percentgc");

    interpolate = ajTrue;

    maxratio = 5.0F;

    dfs = ajListNew();

    ensDensityfeatureadaptorFetchAllbySlice(dfa,
                                            slice,
                                            anname,
                                            blocks,
                                            interpolate,
                                            maxratio,
                                            dfs);

    ensSliceFetchName(slice, &fslname);

    ajFmtPrintF(aoutfile,
                "\n"
                "  Ensembl Slice '%S' has %Lu "
                "Ensembl Density Feature object(s)\n"
                "\n",
                fslname,
                ajListGetLength(dfs));

    while (ajListPop(dfs, (void**) &df))
    {
        feature = ensDensityfeatureGetFeature(df);

        dt = ensDensityfeatureGetDensitytype(df);

        analysis = ensDensitytypeGetAnalysis(dt);

        ensSliceFetchName(ensFeatureGetSlice(feature), &fslname);

        ajFmtPrintF(aoutfile,
                    "    Ensembl Density Feature %u value %.6f\n"
                    "      Ensembl Slice '%S' coordinates '%d:%d:%d'\n"
                    "      Sequence Region coordinates '%S:%d:%d:%d'\n"
                    "      Ensembl Analysis %u name '%S'\n"
                    "      Ensembl Density Type %u size %u type '%s'\n"
                    "\n",
                    ensDensityfeatureGetIdentifier(df),
                    ensDensityfeatureGetValue(df),
                    fslname,
                    ensFeatureGetStart(feature),
                    ensFeatureGetEnd(feature),
                    ensFeatureGetStrand(feature),
                    ensFeatureGetSeqregionName(feature),
                    ensFeatureGetSeqregionStart(feature),
                    ensFeatureGetSeqregionEnd(feature),
                    ensFeatureGetSeqregionStrand(feature),
                    ensAnalysisGetIdentifier(analysis),
                    ensAnalysisGetName(analysis),
                    ensDensitytypeGetIdentifier(dt),
                    ensDensitytypeGetSize(dt),
                    ensDensitytypeTypeToChar(ensDensitytypeGetType(dt)));

        ensDensityfeatureDel(&df);
    }

    ajListFree(&dfs);

    ensSliceDel(&slice);

    ajStrDel(&anname);
    ajStrDel(&fslname);
    ajStrDel(&oslname);

    return ajTrue;
}




/* @funcstatic ensembltest_miscellaneous **************************************
**
** Ensembl Miscellaneous Set and Ensembl Miscellaneous Feature tests.
**
** @param [u] dba [EnsPDatabaseadaptor] Ensembl Database Adaptor
** @param [u] aoutfile [AjPFile] AJAX File
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
** @@
******************************************************************************/

static AjBool ensembltest_miscellaneous(EnsPDatabaseadaptor dba,
                                        AjPFile aoutfile)
{
    AjIList iter = NULL;

    const AjPList list = NULL;

    AjPList codes = NULL;
    AjPList mfs   = NULL;
    AjPList mss   = NULL;

    AjPStr fslname = NULL;
    AjPStr oslname = NULL;

    EnsPAnalysis analysis = NULL;

    EnsPAttribute attribute = NULL;

    EnsPFeature feature = NULL;

    EnsPMiscellaneousfeature        mf  = NULL;
    EnsPMiscellaneousfeatureadaptor mfa = NULL;

    EnsPMiscellaneousset        ms  = NULL;
    EnsPMiscellaneoussetadaptor msa = NULL;

    EnsPSlice slice      = NULL;
    EnsPSliceadaptor sla = NULL;

    if (!dba)
        return ajFalse;

    if (!aoutfile)
        return ajFalse;

    /* Ensembl Miscellaneous tests. */

    ajFmtPrintF(aoutfile,
                "\n"
                "Ensembl Miscellaneous\n");

    ajUser("Ensembl Miscellaneous");

    mfa = ensRegistryGetMiscellaneousfeatureadaptor(dba);
    msa = ensRegistryGetMiscellaneoussetadaptor(dba);
    sla = ensRegistryGetSliceadaptor(dba);

    /* Fetch all Ensembl Miscellaneous Set objects */

    ajFmtPrintF(aoutfile,
                "\n"
                "  All Ensembl Miscellaneous Set objects\n");

    codes = ajListstrNew();

    mss = ajListNew();

    ensMiscellaneoussetadaptorFetchAll(msa, mss);

    ensListMiscellaneoussetSortIdentifierAscending(mss);

    ajFmtPrintF(aoutfile,
                "\n"
                "  %Lu Ensembl Miscellaneous Set object(s)\n"
                "\n",
                ajListGetLength(mss));

    while (ajListPop(mss, (void **) &ms))
    {
        ajFmtPrintF(aoutfile,
                    "    Ensembl Miscellaneous Set %u code '%S' name '%S'\n",
                    ensMiscellaneoussetGetIdentifier(ms),
                    ensMiscellaneoussetGetCode(ms),
                    ensMiscellaneoussetGetName(ms));

        ajListstrPushAppend(codes, ajStrNewS(ensMiscellaneoussetGetCode(ms)));

        ensMiscellaneoussetDel(&ms);
    }

    ajListFree(&mss);

    /*
    ** Fetch an Ensembl Slice for chromosome:GRCh37:18:45000001:46000000:1,
    ** which is around SMAD2.
    */

    oslname = ajStrNewC("chromosome:GRCh37:18:45000001:46000000:1");

    ensSliceadaptorFetchByName(sla, oslname, &slice);

    /*
    ** Fetch all Ensembl Miscellaneous Feature objects on this
    ** Ensembl Slice for all Ensembl Miscellaneous Set codes.
    */

    mfs = ajListNew();

    ensMiscellaneousfeatureadaptorFetchAllbySlicecodes(mfa,
                                                       slice,
                                                       codes,
                                                       mfs);

    ensListMiscellaneousfeatureSortStartAscending(mfs);

    ensSliceFetchName(slice, &fslname);

    ajFmtPrintF(aoutfile,
                "\n"
                "  Ensembl Slice '%S' has %Lu "
                "Ensembl Miscellaneous Feature object(s)\n"
                "\n",
                fslname,
                ajListGetLength(mfs));

    while (ajListPop(mfs, (void **) &mf))
    {
        feature = ensMiscellaneousfeatureGetFeature(mf);

        analysis = ensFeatureGetAnalysis(feature);

        ensSliceFetchName(ensFeatureGetSlice(feature), &fslname);

        ajFmtPrintF(aoutfile,
                    "    Ensembl Miscellaneous Feature %u\n"
                    "      Ensembl Slice '%S' coordinates '%d:%d:%d'\n"
                    "      Sequence Region coordinates '%S:%d:%d:%d'\n"
                    "      Ensembl Analysis %u name '%S'\n",
                    ensMiscellaneousfeatureGetIdentifier(mf),
                    fslname,
                    ensFeatureGetStart(feature),
                    ensFeatureGetEnd(feature),
                    ensFeatureGetStrand(feature),
                    ensFeatureGetSeqregionName(feature),
                    ensFeatureGetSeqregionStart(feature),
                    ensFeatureGetSeqregionEnd(feature),
                    ensFeatureGetSeqregionStrand(feature),
                    ensAnalysisGetIdentifier(analysis),
                    ensAnalysisGetName(analysis));

        /* List all Ensembl Attribute objects. */

        list = ensMiscellaneousfeatureGetAttributes(mf);

        ajFmtPrintF(aoutfile,
                    "    %Lu Ensembl Attribute object(s)\n",
                    ajListGetLength(list));

        iter = ajListIterNewread(list);

        while (!ajListIterDone(iter))
        {
            attribute = (EnsPAttribute) ajListIterGet(iter);

            ajFmtPrintF(aoutfile,
                        "      Ensembl Attribute code '%S' value '%S'\n",
                        ensAttributeGetCode(attribute),
                        ensAttributeGetValue(attribute));
        }

        ajListIterDel(&iter);

        /* List all Ensembl Miscellaneous Set objects. */

        list = ensMiscellaneousfeatureGetMiscellaneoussets(mf);

        ajFmtPrintF(aoutfile,
                    "    %Lu Ensembl Miscellaneous Set object(s)\n",
                    ajListGetLength(list));

        iter = ajListIterNewread(list);

        while (!ajListIterDone(iter))
        {
            ms = (EnsPMiscellaneousset) ajListIterGet(iter);

            ajFmtPrintF(aoutfile,
                        "      Ensembl Miscellaneous Set %u "
                        "code '%S' name '%S'\n",
                        ensMiscellaneoussetGetIdentifier(ms),
                        ensMiscellaneoussetGetCode(ms),
                        ensMiscellaneoussetGetName(ms));
        }

        ajListIterDel(&iter);

        ajFmtPrintF(aoutfile, "\n");

        ensMiscellaneousfeatureDel(&mf);
    }

    ajListFree(&mfs);

    ajListstrFreeData(&codes);

    ensSliceDel(&slice);

    ajStrDel(&fslname);
    ajStrDel(&oslname);

    return ajTrue;
}
