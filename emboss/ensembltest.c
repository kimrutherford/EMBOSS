/* @source ensembltest application
** Test the Ensembl API to be.
** @author Copyright (C) Michael K. Schuster
** @version 1.0
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
******************************************************************************/

#include "emboss.h"

#ifndef WIN32
#include <sys/resource.h>
#endif




/* @datastatic EnsembltestPProjections ****************************************
**
** Ensembl Test Slice Projections.
**
** @alias EnsembltestSProjections
** @alias EnsembltestOProjections
**
** @attr SliceName [const char*] Ensembl Slice name
** @attr CoordsystemName [const char*] Ensembl Coordinate System name
** @attr CoordsystemVersion [const char*] Ensembl Coordinate System version
** @@
******************************************************************************/

typedef struct EnsembltestSProjections
{
    const char *SliceName;
    const char *CoordsystemName;
    const char *CoordsystemVersion;
} EnsembltestOProjections;

#define EnsembltestPProjections EnsembltestOProjections*




static AjBool ensembltest_analyses(EnsPDatabaseadaptor dba,
                                   AjPFile outfile);

static AjBool ensembltest_assembly_exceptions(EnsPDatabaseadaptor dba,
                                              AjPFile outfile);

static AjBool ensembltest_chromosome(EnsPDatabaseadaptor dba,
                                     AjPSeqout outseq);

static AjBool ensembltest_constants(AjPFile outfile);

static AjBool ensembltest_coordinate_systems(EnsPDatabaseadaptor dba,
                                             AjPFile outfile);

static AjBool ensembltest_ditags(EnsPDatabaseadaptor dba,
                                 AjPFile outfile);

static AjBool ensembltest_density(EnsPDatabaseadaptor dba,
                                  AjPFile outfile);

static AjBool ensembltest_features(EnsPDatabaseadaptor dba,
                                   AjPFile outfile);

static AjBool ensembltest_genes(EnsPDatabaseadaptor dba);

static AjBool ensembltest_markers(EnsPDatabaseadaptor dba,
                                  AjPFile outfile);

static AjBool ensembltest_masking(EnsPDatabaseadaptor dba,
                                  AjPSeqout outseq);

static AjBool ensembltest_meta(EnsPDatabaseadaptor dba,
                               AjPFile outfile);

static AjBool ensembltest_registry(AjPFile outfile);

static AjBool ensembltest_sequence_regions(EnsPDatabaseadaptor dba,
                                           AjPFile outfile);

static AjBool ensembltest_slice_projections(EnsPDatabaseadaptor dba,
                                            AjPFile outfile);

static AjBool ensembltest_sequence(EnsPDatabaseadaptor dba,
                                   AjPSeqout outseq);

static AjBool ensembltest_transformations(EnsPDatabaseadaptor dba,
                                          AjPFile outfile);




/* @prog ensembltest **********************************************************
**
** Demonstration of the Ensembl API to be.
**
******************************************************************************/

int main(int argc, char **argv)
{
#ifndef WIN32
    struct rusage ru;
#endif

    AjBool large = AJFALSE;

    AjESqlconnectionClient client = ajESqlconnectionClientMySQL;

    EnsEDatabaseadaptorGroup group  = ensEDatabaseadaptorGroupCore;

    AjPFile outfile = NULL;

    AjIList iter        = NULL;
    AjPList list        = NULL;
    const AjPList exons = NULL;
    AjPList mrlist      = NULL;

    AjPSeq seq = NULL;

    AjPSeqout outseq = NULL;

    AjPStr user     = NULL;
    AjPStr password = NULL;
    AjPStr host     = NULL;
    AjPStr port     = NULL;
    AjPStr socketf  = NULL;
    AjPStr dbname   = NULL;
    AjPStr species  = NULL;
    AjPStr srname   = NULL;
    AjPStr tmpstr   = NULL;

    EnsPDatabaseadaptor dba = NULL;

    EnsPDatabaseconnection dbc = NULL;

    EnsPExon exon      = NULL;
    EnsPExonadaptor ea = NULL;

    EnsPFeature feature = NULL;

    EnsPMapperresult mr = NULL;

    EnsPTranscript transcript = NULL;
    EnsPTranscriptadaptor tca = NULL;

    EnsPTranslation translation = NULL;
    EnsPTranslationadaptor tla  = NULL;

    embInit("ensembltest", argc, argv);

    list     = ajListNew();
    user     = ajStrNew();
    password = ajStrNew();
    host     = ajStrNew();
    port     = ajStrNew();
    socketf  = ajStrNew();
    dbname   = ajStrNew();
    species  = ajStrNewC("homo sapiens");
    srname   = ajStrNew();

    outfile = ajAcdGetOutfile("outfile");

    outseq = ajAcdGetSeqoutall("outseq");

    large = ajAcdGetBoolean("large");

    ensInit();

    /* Ensembl Registry Test */

    /* Ensembl Vertebrates */

    ajStrAssignC(&user, "anonymous");
    ajStrAssignC(&host, "ensembldb.ensembl.org");
    ajStrAssignC(&port, "5306");

    dbc = ensDatabaseconnectionNewIni(client,
                                      user,
                                      password,
                                      host,
                                      port,
                                      socketf,
                                      dbname,
                                      0);

    if(!dbc)
        ajFatal("main Could not connect as user '%S' to server '%S' "
                "at port '%S' to database '%S'.", user, host, port, dbname);

    ensRegistryLoadDatabaseconnection(dbc);

    ensDatabaseconnectionDel(&dbc);

    /* Ensembl Genomes */

    ajStrAssignC(&user, "anonymous");
    ajStrAssignC(&host, "mysql.ebi.ac.uk");
    ajStrAssignC(&port, "4157");

    dbc = ensDatabaseconnectionNewIni(client,
                                      user,
                                      password,
                                      host,
                                      port,
                                      socketf,
                                      dbname,
                                      0);

    if(!dbc)
        ajFatal("main Could not connect as user '%S' to server '%S' "
                "at port '%S' to database '%S'.", user, host, port, dbname);

    ensRegistryLoadDatabaseconnection(dbc);

    ensDatabaseconnectionDel(&dbc);

    /* */

    dba = ensRegistryGetDatabaseadaptor(group, species);

    /* Fetch Exon ENSE00001434436 */

    ea = ensRegistryGetExonadaptor(dba);

    tmpstr = ajStrNewC("ENSE00001191187");

    ensExonadaptorFetchByStableidentifier(ea, tmpstr, 0, &exon);

    ajFmtPrintF(outfile,
                "Ensembl Exon '%S'\n",
                ensExonGetStableidentifier(exon));

    ensExonFetchSequenceSliceSeq(exon, &seq);

    if(seq)
        ajSeqoutWriteSeq(outseq, seq);
    else
        ajFatal("main could not get an AJAX sequence.\n");

    ajSeqDel(&seq);

    ensExonDel(&exon);

    ajStrDel(&tmpstr);

    /* Fetch Transcript ENST00000262160 SMAD2-001 */

    tca = ensRegistryGetTranscriptadaptor(dba);

    tmpstr = ajStrNewC("ENST00000262160");

    ensTranscriptadaptorFetchByStableidentifier(tca, tmpstr, 0, &transcript);

    ajStrDel(&tmpstr);

    if(transcript)
    {
        feature = ensTranscriptGetFeature(transcript);

        ajFmtPrintF(outfile,
                    "Ensembl Transcript '%S' %S:%d:%d:%d\n",
                    ensTranscriptGetStableidentifier(transcript),
                    ensFeatureGetSeqregionName(feature),
                    ensFeatureGetSeqregionStart(feature),
                    ensFeatureGetSeqregionEnd(feature),
                    ensFeatureGetSeqregionStrand(feature));

        /* Transcript Mapper Test */

        mrlist = ajListNew();

        ensTranscriptMapperTranscriptToslice(transcript, 2, 3, mrlist);

        while(ajListPop(mrlist, (void **) &mr))
        {
            switch(ensMapperresultGetType(mr))
            {
                case ensEMapperresultTypeCoordinate:

                    ajFmtPrintF(outfile,
                                "Ensembl Transcript Mapper map positions 2:3 "
                                "to Slice %d:%d:%d\n",
                                ensMapperresultGetCoordinateStart(mr),
                                ensMapperresultGetCoordinateEnd(mr),
                                ensMapperresultGetCoordinateStrand(mr));

                    break;

                case ensEMapperresultTypeGap:

                    ajFmtPrintF(outfile,
                                "Ensembl Transcript Mapper 2:3 to Slice got "
                                "Mapper Result of type Gap %d:%d. \n",
                                ensMapperresultGetGapStart(mr),
                                ensMapperresultGetGapEnd(mr));

                    break;

                default:

                    ajFmtPrintF(outfile,
                                "Ensembl Transcript Mapper 2:3 to Slice got "
                                "Mapper Result of unexpected type %d.\n",
                                ensMapperresultGetType(mr));
            }

            ensMapperresultDel(&mr);
        }

        ajListFree(&mrlist);

        /* Fetch the Transcript sequence. */

        ensTranscriptFetchSequenceTranscriptSeq(transcript, &seq);

        ajSeqoutWriteSeq(outseq, seq);

        ajSeqDel(&seq);

        /* Fetch the sequence for the canonical Translation. */

        translation = ensTranscriptLoadTranslation(transcript);

        ensTranscriptFetchSequenceTranslationSeq(transcript,
                                                 translation,
                                                 &seq);

        if(seq)
        {
            ajSeqoutWriteSeq(outseq, seq);

            ajSeqDel(&seq);
        }

        /*
        ** Fetch all Exons of this Transcript and write out their
        ** coordinates.
        */

        exons = ensTranscriptLoadExons(transcript);

        iter = ajListIterNewread(exons);

        while(!ajListIterDone(iter))
        {
            exon = (EnsPExon) ajListIterGet(iter);

            feature = ensExonGetFeature(exon);

            ajFmtPrintF(outfile,
                        "  Ensembl Exon '%S'\n"
                        "    Transcript %u:%u\n"
                        "    Slice %d:%d:%d\n"
                        "    Ensembl Translation\n"
                        "      Transcript %u:%u\n"
                        "      Slice %u:%u\n",
                        ensExonGetStableidentifier(exon),
                        ensExonCalculateTranscriptStart(exon,
                                                        transcript),
                        ensExonCalculateTranscriptEnd(exon,
                                                      transcript),
                        ensFeatureGetStart(feature),
                        ensFeatureGetEnd(feature),
                        ensFeatureGetStrand(feature),
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

        ajListIterDel(&iter);
    }

    ensTranscriptDel(&transcript);

    /* Fetch Translation ENSP00000349282 SMAD2-201 */

    tla = ensRegistryGetTranslationadaptor(dba);

    tmpstr = ajStrNewC("ENSP00000349282");

    ensTranslationadaptorFetchByStableidentifier(tla, tmpstr, 0, &translation);

    ajStrDel(&tmpstr);

    if(translation)
    {
        /*
        ** NOTE: Fetching the sequence directly from a Translation causes the
        ** Transcript to be re-fetched from the database. Transcripts
        ** have strong references for their Translations, but
        ** Transaltions do not have any reference to their Transcript.
        ** Use ensTranscriptFetchSequenceTranslationSeq if a Transcript is
        ** already available.
        */

        ensTranslationFetchSequenceSeq(translation, &seq);

        ajSeqoutWriteSeq(outseq, seq);

        ajSeqDel(&seq);
    }

    ensTranslationDel(&translation);

    ensembltest_constants(outfile);

    ensembltest_registry(outfile);

    ensembltest_analyses(dba, outfile);

    ensembltest_meta(dba, outfile);

    ensembltest_coordinate_systems(dba, outfile);

    ensembltest_sequence_regions(dba, outfile);

    ensembltest_assembly_exceptions(dba, outfile);

    ensembltest_slice_projections(dba, outfile);

    ensembltest_features(dba, outfile);

    ensembltest_masking(dba, outseq);

    ensembltest_sequence(dba, outseq);

    ensembltest_markers(dba, outfile);

    ensembltest_ditags(dba, outfile);

    ensembltest_transformations(dba, outfile);

    ensembltest_density(dba, outfile);

    /* Fetch larger data sets... */

    if(large)
    {
        ensembltest_genes(dba);

        /* Fetch sequence for human chromosome 21. */
        if(AJFALSE)
            ensembltest_chromosome(dba, outseq);
    }

    /* Clean up and exit. */

    ajListFree(&list);

    ajFileClose(&outfile);

    ajSeqoutClose(outseq);

    ajSeqoutDel(&outseq);

    ajStrDel(&user);
    ajStrDel(&password);
    ajStrDel(&host);
    ajStrDel(&port);
    ajStrDel(&socketf);
    ajStrDel(&dbname);
    ajStrDel(&species);
    ajStrDel(&srname);

    /* Get resource usage */
#ifndef WIN32
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

    return 0;
}




/* @funcstatic ensembltest_slice_projections **********************************
**
** Project Ensembl Slices.
**
** Project the 'chromosome' X and Y PAR regions (1 - 2709520) into the 'clone'
** and 'contig' Coordinate System, which will use the Generic Assembly Mapper
** for the mapping path 'chromosome' <-> 'contig' and the Chained Assembly
** Mapper for the mapping path 'chromosome' <-> 'contig' <-> 'clone'.
**
** @param [u] dba [EnsPDatabaseadaptor] Ensembl Database Adaptor.
** @param [u] outfile [AjPFile] AJAX File.
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise.
** @@
******************************************************************************/

static AjBool ensembltest_slice_projections(EnsPDatabaseadaptor dba,
                                            AjPFile outfile)
{
    ajint i = 0;

    AjBool debug = AJFALSE;

    AjPList pslist = NULL;

    AjPStr slname    = NULL;
    AjPStr csname    = NULL;
    AjPStr csversion = NULL;
    AjPStr sename    = NULL;
    AjPStr psname    = NULL;

    EnsPProjectionsegment ps = NULL;

    EnsPSlice psslice    = NULL;
    EnsPSlice slice      = NULL;
    EnsPSliceadaptor sla = NULL;

    /* Perform the following Slice projections. */

    EnsembltestOProjections ensembltestProjections[] =
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
            {"chromosome:GRCh37:22:-17000000:+18000000:1", "supercontig", ""},
            {NULL, NULL, NULL}
        };

    debug = ajDebugTest("ensembltest_slice_projections");

    if(!dba)
        return ajFalse;

    if(!outfile)
        return ajFalse;

    ajFmtPrintF(outfile,
                "\n"
                "Ensembl Slice Projections\n");

    ajUser("Ensembl Slice Projections");

    sla = ensRegistryGetSliceadaptor(dba);

    pslist = ajListNew();

    slname    = ajStrNew();
    csname    = ajStrNew();
    csversion = ajStrNew();

    for(i = 0; ensembltestProjections[i].SliceName; i++)
    {
        ajStrAssignC(&slname,    ensembltestProjections[i].SliceName);
        ajStrAssignC(&csname,    ensembltestProjections[i].CoordsystemName);
        ajStrAssignC(&csversion, ensembltestProjections[i].CoordsystemVersion);

        ensSliceadaptorFetchByName(sla, slname, &slice);

        ensSliceFetchName(slice, &sename);

        ajFmtPrintF(outfile,
                    "\n"
                    "  Project Ensembl Slice '%S' into\n"
                    "  Ensembl Coordinate System '%S:%S'.\n",
                    sename, csname, csversion);

        if(debug)
            ajDebug("ensembltest_slice_projections begin ensSliceProject "
                    "for Ensembl Slice '%S' and "
                    "Ensembl Coordinate System '%S:%S'.\n",
                    sename, csname, csversion);

        ensSliceProject(slice, csname, csversion, pslist);

        if(debug)
            ajDebug("ensembltest_slice_projections finished "
                    "ensSliceProject.\n");

        while(ajListPop(pslist, (void **) &ps))
        {
            psslice = ensProjectionsegmentGetTargetSlice(ps);

            ensSliceFetchName(psslice, &psname);

            ajFmtPrintF(outfile,
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

        ensSliceDel(&slice);
    }

    ajListFree(&pslist);

    ajStrDel(&slname);
    ajStrDel(&csname);
    ajStrDel(&csversion);

    return ajTrue;
}




/* @funcstatic ensembltest_analyses *******************************************
**
** Ensembl Analysis tests.
**
** @param [u] dba [EnsPDatabaseadaptor] Ensembl Database Adaptor.
** @param [u] outfile [AjPFile] AJAX File.
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise.
** @@
******************************************************************************/

static AjBool ensembltest_analyses(EnsPDatabaseadaptor dba,
                                   AjPFile outfile)
{
    AjPList as = NULL;

    AjPStr name = NULL;

    EnsPAnalysis analysis  = NULL;
    EnsPAnalysisadaptor aa = NULL;

    if(!dba)
        return ajFalse;

    if(!outfile)
        return ajFalse;

    aa = ensRegistryGetAnalysisadaptor(dba);

    ajFmtPrintF(outfile,
                "\n"
                "Ensembl Analyses\n");

    ajUser("Ensembl Analyses");

    /* Fetch the Ensembl Analysis for identifier 1 */

    ajFmtPrintF(outfile,
                "\n"
                "  Ensembl Analysis for identifier 1\n");

    ensAnalysisadaptorFetchByIdentifier(aa, 1, &analysis);

    ajFmtPrintF(outfile,
                "    Ensembl Analyis %u name '%S'\n",
                ensAnalysisGetIdentifier(analysis),
                ensAnalysisGetName(analysis));

    ensAnalysisDel(&analysis);

    /* Fetch the Ensembl Analysis for name 'cpg' */

    ajFmtPrintF(outfile,
                "\n"
                "  Ensembl Analysis for name 'cpg'\n");

    name = ajStrNewC("cpg");

    ensAnalysisadaptorFetchByName(aa, name, &analysis);

    ajFmtPrintF(outfile,
                "    Ensembl Analyis %u name '%S'\n",
                ensAnalysisGetIdentifier(analysis),
                ensAnalysisGetName(analysis));

    ensAnalysisDel(&analysis);

    ajStrDel(&name);

    /* Fetch all Ensembl Analyses */

    ajFmtPrintF(outfile,
                "\n"
                "  All Ensembl Analyses\n");

    as = ajListNew();

    ensAnalysisadaptorFetchAll(aa, as);

    while(ajListPop(as, (void **) &analysis))
    {
        ajFmtPrintF(outfile,
                    "    Ensembl Analyis %u name '%S'\n",
                    ensAnalysisGetIdentifier(analysis),
                    ensAnalysisGetName(analysis));

        ensAnalysisDel(&analysis);
    }

    ajListFree(&as);

    /* Test SQL escaping of "'" characters. */

    ajFmtPrintF(outfile,
                "\n"
                "  Test SQL escaping of \"'\" characters.\n");

    name = ajStrNewC("'test'");

    ensAnalysisadaptorFetchByName(aa, name, &analysis);

    if(analysis)
        ajFmtPrintF(outfile,
                    "    Ensembl Analyis for name '%S' %u name '%S'\n",
                    name,
                    ensAnalysisGetIdentifier(analysis),
                    ensAnalysisGetName(analysis));
    else
        ajFmtPrintF(outfile,
                    "    No Ensembl Analysis for name '%S', "
                    "which is the expected result.\n",
                    name);

    ensAnalysisDel(&analysis);

    ajStrDel(&name);

    return ajTrue;
}




/* @funcstatic ensembltest_assembly_exceptions ********************************
**
** Project Ensembl Slices crossing Assembly Exceptions.
**
** @param [u] dba [EnsPDatabaseadaptor] Ensembl Database Adaptor.
** @param [u] outfile [AjPFile] AJAX File.
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise.
** @@
******************************************************************************/

static AjBool ensembltest_assembly_exceptions(EnsPDatabaseadaptor dba,
                                              AjPFile outfile)
{
    AjBool debug        = AJFALSE;
    AjBool duplicates   = AJTRUE;
    AjBool nonreference = AJTRUE;
    AjBool lrg          = AJFALSE;

    AjPList pslist = NULL;
    AjPList sllist = NULL;

    AjPStr csname    = NULL;
    AjPStr csversion = NULL;
    AjPStr slname    = NULL;
    AjPStr psname    = NULL;

    EnsPProjectionsegment ps = NULL;

    EnsPSlice psslice    = NULL;
    EnsPSlice slice      = NULL;
    EnsPSliceadaptor sla = NULL;

    debug = ajDebugTest("ensembltest_assembly_exceptions");

    if(debug)
        ajDebug("ensembltest_assembly_exceptions start\n");

    if(!dba)
        return ajFalse;

    if(!outfile)
        return ajFalse;

    ajFmtPrintF(outfile,
                "\n"
                "Ensembl Assembly Exceptions\n");

    ajUser("Ensembl Assembly Exceptions");

    sla = ensRegistryGetSliceadaptor(dba);

    csname    = ajStrNewC("toplevel");
    csversion = ajStrNew();

    pslist = ajListNew();
    sllist = ajListNew();

    ensSliceadaptorFetchAll(sla,
                            csname,
                            csversion,
                            nonreference,
                            duplicates,
                            lrg,
                            sllist);

    while(ajListPop(sllist, (void **) &slice))
    {
        ensSliceFetchName(slice, &slname);

        ajFmtPrintF(outfile,
                    "\n"
                    "  Fetch normalised Slice Projections for Ensembl Slice\n"
                    "  '%S'\n"
                    "\n",
                    slname);

        if(debug)
            ajDebug("ensembltest_assembly_exceptions begin "
                    "ensSliceadaptorRetrieveNormalisedprojection "
                    "for Ensembl Slice '%S'.\n",
                    slname);

        ensSliceadaptorRetrieveNormalisedprojection(sla, slice, pslist);

        if(debug)
            ajDebug("ensembltest_assembly_exceptions finished "
                    "ensSliceadaptorRetrieveNormalisedprojection.\n");

        while(ajListPop(pslist, (void **) &ps))
        {
            psslice = ensProjectionsegmentGetTargetSlice(ps);

            ensSliceFetchName(psslice, &psname);

            ajFmtPrintF(outfile,
                        "    '%S' %u:%u\n"
                        "    '%S'\n"
                        "\n",
                        slname,
                        ensProjectionsegmentGetSourceStart(ps),
                        ensProjectionsegmentGetSourceEnd(ps),
                        psname);

            ajStrDel(&psname);

            ensProjectionsegmentDel(&ps);
        }

        ajStrDel(&slname);

        ensSliceDel(&slice);
    }

    ajListFree(&pslist);
    ajListFree(&sllist);

    ajStrDel(&csname);
    ajStrDel(&csversion);

    if(debug)
        ajDebug("ensembltest_assembly_exceptions end\n");

    return ajTrue;
}




/* @funcstatic ensembltest_constants ******************************************
**
** Ensembl constants tests.
**
** @param [u] outfile [AjPFile] AJAX File.
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise.
** @@
******************************************************************************/

static AjBool ensembltest_constants(AjPFile outfile)
{
    if(!outfile)
        return ajFalse;

    ajFmtPrintF(outfile,
                "\n"
                "Ensembl Constants\n");

    ajUser("Ensembl Constants");

    ajFmtPrintF(outfile,
                "\n"
                "  Ensembl Base Adaptor Maximum Identifiers %u\n",
                ensBaseadaptorMaximumIdentifiers);

    ajFmtPrintF(outfile,
                "\n"
                "  Ensembl Software Version '%s'\n",
                ensSoftwareVersion);

    return ajTrue;
}




/* @funcstatic ensembltest_features *******************************************
**
** Ensembl Feature tests.
**
** @param [u] dba [EnsPDatabaseadaptor] Ensembl Database Adaptor.
** @param [u] outfile [AjPFile] AJAX File.
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise.
** @@
******************************************************************************/

static AjBool ensembltest_features(EnsPDatabaseadaptor dba,
                                   AjPFile outfile)
{
    AjIList iter        = NULL;
    AjPList exons       = NULL;
    AjPList genes       = NULL;
    AjPList transcripts = NULL;
    const AjPList list  = NULL;

    AjPStr slname = NULL;

    EnsPExon exon      = NULL;
    EnsPExonadaptor ea = NULL;

    EnsPFeature feature = NULL;

    EnsPGene gene      = NULL;
    EnsPGeneadaptor ga = NULL;

    EnsPSlice slice      = NULL;
    EnsPSliceadaptor sla = NULL;

    EnsPTranscript transcript = NULL;
    EnsPTranscriptadaptor tca = NULL;

    if(!dba)
        return ajFalse;

    if(!outfile)
        return ajFalse;

    ajFmtPrintF(outfile,
                "\n"
                "Ensembl Features\n");

    ajUser("Ensembl Features");

    sla = ensRegistryGetSliceadaptor(dba);

    /*
    ** Fetch an Ensembl Slice for chromosome:GRCh37:18:45300001:45500000:1,
    ** which is inside SMAD2.
    */

    slname = ajStrNewC("chromosome:GRCh37:18:45300001:45500000:1");

    ensSliceadaptorFetchByName(sla, slname, &slice);

    ensSliceFetchName(slice, &slname);

    /* Fetch all Exons on this Slice. */

    ea = ensRegistryGetExonadaptor(dba);

    exons = ajListNew();

    ensExonadaptorFetchAllbySlice(ea, slice, (AjPStr) NULL, exons);

    ajFmtPrintF(outfile,
                "\n"
                "  Ensembl Slice '%S' %u Exons\n\n",
                slname,
                ajListGetLength(exons));

    while(ajListPop(exons, (void **) &exon))
    {
        feature = ensExonGetFeature(exon);

        ajFmtPrintF(outfile,
                    "    Exon Feature Slice coordinates "
                    "'%S:%d:%d:%d'\n",
                    ensExonGetStableidentifier(exon),
                    ensFeatureGetStart(feature),
                    ensFeatureGetEnd(feature),
                    ensFeatureGetStrand(feature));

        ajFmtPrintF(outfile,
                    "    Exon Sequence Region coordinates "
                    "'%S:%d:%d:%d'\n\n",
                    ensExonGetStableidentifier(exon),
                    ensFeatureGetSeqregionStart(feature),
                    ensFeatureGetSeqregionEnd(feature),
                    ensFeatureGetSeqregionStrand(feature));

        ensExonDel(&exon);
    }

    ajListFree(&exons);

    /* Fetch all Transcripts on this Slice. */

    tca = ensRegistryGetTranscriptadaptor(dba);

    transcripts = ajListNew();

    ensTranscriptadaptorFetchAllbySlice(tca,
                                        slice,
                                        (AjPStr) NULL,
                                        (AjPStr) NULL,
                                        ajTrue,
                                        transcripts);

    ajFmtPrintF(outfile,
                "  Ensembl Slice '%S' %u Transcripts\n\n",
                slname,
                ajListGetLength(transcripts));

    while(ajListPop(transcripts, (void **) &transcript))
    {
        feature = ensTranscriptGetFeature(transcript);

        ajFmtPrintF(outfile,
                    "    Transcript Feature Slice coordinates "
                    "'%S:%d:%d:%d'\n",
                    ensTranscriptGetStableidentifier(transcript),
                    ensFeatureGetStart(feature),
                    ensFeatureGetEnd(feature),
                    ensFeatureGetStrand(feature));

        ajFmtPrintF(outfile,
                    "    Transcript Sequence Region coordinates "
                    "'%S:%d:%d:%d'\n\n",
                    ensTranscriptGetStableidentifier(transcript),
                    ensFeatureGetSeqregionStart(feature),
                    ensFeatureGetSeqregionEnd(feature),
                    ensFeatureGetSeqregionStrand(feature));

        list = ensTranscriptLoadExons(transcript);

        iter = ajListIterNewread(list);

        while(!ajListIterDone(iter))
        {
            exon = (EnsPExon) ajListIterGet(iter);

            feature = ensExonGetFeature(exon);

            ajFmtPrintF(outfile,
                        "      Exon Feature Slice coordinates "
                        "'%S:%d:%d:%d'\n",
                        ensExonGetStableidentifier(exon),
                        ensFeatureGetStart(feature),
                        ensFeatureGetEnd(feature),
                        ensFeatureGetStrand(feature));

            ajFmtPrintF(outfile,
                        "      Exon Sequence Region coordinates "
                        "'%S:%d:%d:%d'\n\n",
                        ensExonGetStableidentifier(exon),
                        ensFeatureGetSeqregionStart(feature),
                        ensFeatureGetSeqregionEnd(feature),
                        ensFeatureGetSeqregionStrand(feature));
        }

        ajListIterDel(&iter);

        /* TODO: List the Translations? */

        ensTranscriptDel(&transcript);
    }

    ajListFree(&transcripts);

    /* Fetch all Genes on this Slice. */

    ga = ensRegistryGetGeneadaptor(dba);

    genes = ajListNew();

    ensGeneadaptorFetchAllbySlice(ga,
                                  slice,
                                  (AjPStr) NULL,
                                  (AjPStr) NULL,
                                  (AjPStr) NULL,
                                  ajTrue,
                                  genes);

    ajFmtPrintF(outfile,
                "  Ensembl Slice '%S' %u Genes\n\n",
                slname,
                ajListGetLength(genes));

    while(ajListPop(genes, (void **) &gene))
    {
        feature = ensGeneGetFeature(gene);

        ajFmtPrintF(outfile,
                    "    Gene Feature Slice coordinates "
                    "'%S:%d:%d:%d'\n",
                    ensGeneGetStableidentifier(gene),
                    ensFeatureGetStart(feature),
                    ensFeatureGetEnd(feature),
                    ensFeatureGetStrand(feature));

        ajFmtPrintF(outfile,
                    "    Gene Sequence Region coordinates "
                    "'%S:%d:%d:%d'\n\n",
                    ensGeneGetStableidentifier(gene),
                    ensFeatureGetSeqregionStart(feature),
                    ensFeatureGetSeqregionEnd(feature),
                    ensFeatureGetSeqregionStrand(feature));

        list = ensGeneLoadTranscripts(gene);

        iter = ajListIterNewread(list);

        while(!ajListIterDone(iter))
        {
            transcript = (EnsPTranscript) ajListIterGet(iter);

            feature = ensTranscriptGetFeature(transcript);

            ajFmtPrintF(outfile,
                        "      Transcript Feature Slice coordinates "
                        "'%S:%d:%d:%d'\n",
                        ensTranscriptGetStableidentifier(transcript),
                        ensFeatureGetStart(feature),
                        ensFeatureGetEnd(feature),
                        ensFeatureGetStrand(feature));

            ajFmtPrintF(outfile,
                        "      Transcript Sequence Region coordinates "
                        "'%S:%d:%d:%d'\n\n",
                        ensTranscriptGetStableidentifier(transcript),
                        ensFeatureGetSeqregionStart(feature),
                        ensFeatureGetSeqregionEnd(feature),
                        ensFeatureGetSeqregionStrand(feature));

        }

        ajListIterDel(&iter);

        ensGeneDel(&gene);
    }

    ajListFree(&genes);

    ajStrDel(&slname);

    ensSliceDel(&slice);

    return ajTrue;
}




/* @funcstatic ensembltest_genes **********************************************
**
** Fetch all Ensembl Genes and write sequences of Transcripts, Translations
** and Exons to disk.
**
** The number of genes processed can be configured by the maxnum ACD parameter
** name. The sequences are written to FASTA files as specified by the
** exons, transcripts and translations ACD parameter names.
**
** @param [u] dba [EnsPDatabaseadaptor] Ensembl Database Adaptor.
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise.
** @@
******************************************************************************/

static AjBool ensembltest_genes(EnsPDatabaseadaptor dba)
{
    register ajint i = 0;

    ajint maxnum = 0;

    AjBool debug = AJFALSE;

    AjIList triter = NULL;
    AjPList exons  = NULL;
    AjPList genes  = NULL;

    const AjPList trlist = NULL;

    AjPSeq seq = NULL;

    AjPSeqout exnoutseq = NULL;
    AjPSeqout trcoutseq = NULL;
    AjPSeqout trloutseq = NULL;

    EnsPExon exon = NULL;

    EnsPGene gene      = NULL;
    EnsPGeneadaptor ga = NULL;

    EnsPTranslation translation = NULL;

    EnsPTranscript transcript = NULL;

    debug = ajDebugTest("ensembltest_genes");

    if(!dba)
        return ajFalse;

    maxnum = ajAcdGetInt("maxnum");

    exnoutseq = ajAcdGetSeqoutall("exonsoutseq");
    trcoutseq = ajAcdGetSeqoutall("transcriptsoutseq");
    trloutseq = ajAcdGetSeqoutall("translationsoutseq");

    ajUser("Ensembl Genes");

    ga = ensRegistryGetGeneadaptor(dba);

    /* Fetch all Genes. */

    exons = ajListNew();
    genes = ajListNew();

    ensGeneadaptorFetchAll(ga, genes);

    /*
    ** Although Genes have not been retrieved from a Slice, the following
    ** function can still sort them by Slices and then Slice start coordinates.
    */

    ensListGeneSortStartAscending(genes);

    while(ajListPop(genes, (void **) &gene))
    {
        /* Check if a limit has been set and if it has not been exceeded. */

        if((maxnum < 0) || (i < maxnum))
        {
            if(debug)
                ajDebug("ensembltest_genes "
                        "Gene stable identifier: '%S'\n",
                        ensGeneGetStableidentifier(gene));

            /* Get all Transcripts of this Gene. */

            trlist = ensGeneLoadTranscripts(gene);

            triter = ajListIterNewread(trlist);

            while(!ajListIterDone(triter))
            {
                transcript = (EnsPTranscript) ajListIterGet(triter);

                if(debug)
                    ajDebug("ensembltest_genes "
                            "Transcript stable identifier: '%S'\n",
                            ensTranscriptGetStableidentifier(transcript));

                /* Fetch and write the sequence of this Transcript. */

                ensTranscriptFetchSequenceTranscriptSeq(transcript, &seq);

                ajSeqoutWriteSeq(trcoutseq, seq);

                ajSeqDel(&seq);

                /* Get the Translation of this Transcript. */

                translation = ensTranscriptLoadTranslation(transcript);

                /* Not every Transcript has a Translation. */

                /*
                ** FIXME: EMBOSS crashes if the AJAX Sequence object is not
                ** defined.
                */

                if(translation)
                {
                    if(debug)
                        ajDebug(
                            "ensembltest_genes "
                            "Translation stable identifier: '%S'\n",
                            ensTranslationGetStableidentifier(translation));

                    /* Fetch and write the sequence of this Translation. */

                    ensTranslationFetchSequenceSeq(translation, &seq);

                    ajSeqoutWriteSeq(trloutseq, seq);

                    ajSeqDel(&seq);
                }
            }

            ajListIterDel(&triter);

            /* Fetch all Exons of this Gene. */

            ensGeneFetchAllExons(gene, exons);

            while(ajListPop(exons, (void **) &exon))
            {
                if(debug)
                    ajDebug("ensembltest_genes "
                            "Exon stable identifier: '%S'\n",
                            ensExonGetStableidentifier(exon));

                /* Fetch and write the sequence of this Exon. */

                ensExonFetchSequenceSliceSeq(exon, &seq);

                ajSeqoutWriteSeq(exnoutseq, seq);

                ajSeqDel(&seq);

                ensExonDel(&exon);
            }
        }

        ensGeneDel(&gene);

        i++;
    }

    ajListFree(&exons);
    ajListFree(&genes);

    ajSeqoutClose(exnoutseq);
    ajSeqoutClose(trcoutseq);
    ajSeqoutClose(trloutseq);

    ajSeqoutDel(&exnoutseq);
    ajSeqoutDel(&trcoutseq);
    ajSeqoutDel(&trloutseq);

    return ajTrue;
}




/* @funcstatic ensembltest_markers ********************************************
**
** Ensembl Marker and Ensembl Marker Feature tests.
**
** @param [u] dba [EnsPDatabaseadaptor] Ensembl Database Adaptor.
** @param [u] outfile [AjPFile] AJAX File.
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise.
** @@
******************************************************************************/

static AjBool ensembltest_markers(EnsPDatabaseadaptor dba,
                                  AjPFile outfile)
{
    AjPList mflist = NULL;

    AjPStr slname = NULL;

    EnsPAnalysis analysis = NULL;

    EnsPFeature feature = NULL;

    EnsPMarkerfeature mf         = NULL;
    EnsPMarkerfeatureadaptor mfa = NULL;

    EnsPMarker marker = NULL;

    EnsPMarkersynonym ms = NULL;

    EnsPSlice slice      = NULL;
    EnsPSliceadaptor sla = NULL;

    if(!dba)
        return ajFalse;

    if(!outfile)
        return ajFalse;

    ajFmtPrintF(outfile,
                "\n"
                "Ensembl Marker Features\n");

    ajUser("Ensembl Marker Features");

    sla = ensRegistryGetSliceadaptor(dba);

    /*
    ** Fetch an Ensembl Slice for chromosome:GRCh37:18:45000001:46000000:1,
    ** which is around SMAD2.
    */

    slname = ajStrNewC("chromosome:GRCh37:18:45000001:46000000:1");

    ensSliceFetchName(slice, &slname);

    ensSliceadaptorFetchByName(sla, slname, &slice);

    /* Fetch all Marker Features on this Slice. */

    mfa = ensRegistryGetMarkerfeatureadaptor(dba);

    mflist = ajListNew();

    ensMarkerfeatureadaptorFetchAllbySlice(mfa,
                                           slice,
                                           0,
                                           0,
                                           (AjPStr) NULL,
                                           mflist);

    ajFmtPrintF(outfile,
                "\n"
                "  Ensembl Slice '%S' %u Marker Features\n\n",
                slname,
                ajListGetLength(mflist));

    while(ajListPop(mflist, (void **) &mf))
    {
        feature = ensMarkerfeatureGetFeature(mf);

        analysis = ensFeatureGetAnalysis(feature);

        marker = ensMarkerfeatureGetMarker(mf);

        ms = ensMarkerGetDisplaysynonym(marker);

        ajFmtPrintF(outfile,
                    "    Ensembl Marker Feature '%u:%S'\n"
                    "    Ensembl Marker         '%u:%s'\n"
                    "    Ensembl Marker Synonym '%S:%S'\n",
                    ensMarkerfeatureGetIdentifier(mf),
                    ensAnalysisGetName(analysis),
                    ensMarkerGetIdentifier(marker),
                    ensMarkerTypeToChar(ensMarkerGetType(marker)),
                    ensMarkersynonymGetName(ms),
                    ensMarkersynonymGetSource(ms));

        ajFmtPrintF(outfile,
                    "    Marker Feature Slice coordinates "
                    "'%d:%d:%d'\n",
                    ensFeatureGetStart(feature),
                    ensFeatureGetEnd(feature),
                    ensFeatureGetStrand(feature));

        ajFmtPrintF(outfile,
                    "    Marker Sequence Region coordinates "
                    "'%d:%d:%d'\n\n",
                    ensFeatureGetSeqregionStart(feature),
                    ensFeatureGetSeqregionEnd(feature),
                    ensFeatureGetSeqregionStrand(feature));

        ensMarkerfeatureDel(&mf);
    }

    ajListFree(&mflist);

    ajStrDel(&slname);

    ensSliceDel(&slice);

    return ajTrue;
}




/* @funcstatic ensembltest_ditags *********************************************
**
** Ensembl Ditag and Ensembl Ditag Feature tests.
**
** @param [u] dba [EnsPDatabaseadaptor] Ensembl Database Adaptor.
** @param [u] outfile [AjPFile] AJAX File.
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise.
** @@
******************************************************************************/

static AjBool ensembltest_ditags(EnsPDatabaseadaptor dba,
                                 AjPFile outfile)
{
    AjPList dtfs = NULL;

    AjPStr slname = NULL;

    EnsPAnalysis analysis = NULL;

    EnsPFeature feature = NULL;

    EnsPDitag dt = NULL;

    EnsPDitagfeature dtf         = NULL;
    EnsPDitagfeatureadaptor dtfa = NULL;

    EnsPSlice slice      = NULL;
    EnsPSliceadaptor sla = NULL;

    if(!dba)
        return ajFalse;

    if(!outfile)
        return ajFalse;

    ajFmtPrintF(outfile,
                "\n"
                "Ensembl Ditag Features\n");

    ajUser("Ensembl Ditag Features");

    sla = ensRegistryGetSliceadaptor(dba);

    /*
    ** Fetch an Ensembl Slice for chromosome:GRCh37:18:45000001:46000000:1,
    ** which is around SMAD2.
    */

    slname = ajStrNewC("chromosome:GRCh37:18:45000001:46000000:1");

    ensSliceadaptorFetchByName(sla, slname, &slice);

    ensSliceFetchName(slice, &slname);

    /* Fetch all Marker Features on this Slice. */

    dtfa = ensRegistryGetDitagfeatureadaptor(dba);

    dtfs = ajListNew();

    ensDitagfeatureadaptorFetchAllbySlice(dtfa,
                                          slice,
                                          (AjPStr) NULL,
                                          (AjPStr) NULL,
                                          dtfs);

    ajFmtPrintF(outfile,
                "\n"
                "  Ensembl Slice '%S' %u Ditag Features\n\n",
                slname,
                ajListGetLength(dtfs));

    while(ajListPop(dtfs, (void **) &dtf))
    {
        feature = ensDitagfeatureGetFeature(dtf);

        analysis = ensFeatureGetAnalysis(feature);

        dt = ensDitagfeatureGetDitag(dtf);

        ajFmtPrintF(outfile,
                    "    Ensembl Ditag Feature '%u:%S:%s'\n"
                    "    Ensembl Ditag         '%u:%S:%S'\n",
                    ensDitagfeatureGetIdentifier(dtf),
                    ensAnalysisGetName(analysis),
                    ensDitagfeatureSideToChar(ensDitagfeatureGetSide(dtf)),
                    ensDitagGetIdentifier(dt),
                    ensDitagGetName(dt),
                    ensDitagGetType(dt));

        ajFmtPrintF(outfile,
                    "    Ditag Feature Slice coordinates "
                    "'%d:%d:%d'\n",
                    ensFeatureGetStart(feature),
                    ensFeatureGetEnd(feature),
                    ensFeatureGetStrand(feature));

        ajFmtPrintF(outfile,
                    "    Ditag Sequence Region coordinates "
                    "'%d:%d:%d'\n\n",
                    ensFeatureGetSeqregionStart(feature),
                    ensFeatureGetSeqregionEnd(feature),
                    ensFeatureGetSeqregionStrand(feature));

        ensDitagfeatureDel(&dtf);
    }

    ajListFree(&dtfs);

    ajStrDel(&slname);

    ensSliceDel(&slice);

    return ajTrue;
}




/* @funcstatic ensembltest_masking ********************************************
**
** Fetch an Ensembl Slice and an Ensembl Repeatmasked Slice and write their
** sequences to disk.
**
** chromosome:GRCh37:22:16040001:16120000:1
**
** @param [u] dba [EnsPDatabaseadaptor] Ensembl Database Adaptor.
** @param [u] outseq [AjPSeqout] AJAX Sequence Output.
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise.
** @@
******************************************************************************/

static AjBool ensembltest_masking(EnsPDatabaseadaptor dba,
                                  AjPSeqout outseq)
{
    EnsERepeatMaskType mtype = ensERepeatMaskTypeSoft;

    AjPList names = NULL;

    AjPSeq seq = NULL;

    AjPStr slname = NULL;

    AjPTable masking = NULL;

    EnsPSlice slice      = NULL;
    EnsPSliceadaptor sla = NULL;

    EnsPRepeatmaskedslice rmslice = NULL;

    if(!dba)
        return ajFalse;

    if(!outseq)
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

    ajSeqoutWriteSeq(outseq, seq);

    ajSeqDel(&seq);

    /*
    ** Fetch the masked genome sequence for Ensembl Slice
    ** chromosome:GRCh37:22:16040001:16120000:1 and all Analyses.
    */

    rmslice = ensRepeatmaskedsliceNewIni(slice,
                                         (AjPList) NULL,
                                         (AjPTable) NULL);

    ensRepeatmaskedsliceFetchSequenceSeq(rmslice, mtype, &seq);

    ajSeqAssignDescC(seq, "Ensembl Repeat Masked Slice all Analyses");

    ajSeqoutWriteSeq(outseq, seq);

    ajSeqDel(&seq);

    ensRepeatmaskedsliceDel(&rmslice);

    /*
    ** Fetch the masked genome sequence for Ensembl Slice
    ** chromosome:GRCh37:22:16040001:16120000:1 and Analysis "RepeatMasked".
    */

    names = ajListstrNew();

    ajListPushAppend(names, (void *) ajStrNewC("RepeatMask"));

    masking = ajTablestrNew(10);

    rmslice = ensRepeatmaskedsliceNewIni(slice, names, masking);

    ensRepeatmaskedsliceFetchSequenceSeq(rmslice, mtype, &seq);

    ajSeqAssignDescC(seq, "Ensembl Repeat Masked Slice RepeatMasker Analysis");

    ajSeqoutWriteSeq(outseq, seq);

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
** @param [u] dba [EnsPDatabaseadaptor] Ensembl Database Adaptor.
** @param [u] outseq [AjPSeqout] AJAX Sequence Output.
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise.
** @@
******************************************************************************/

static AjBool ensembltest_sequence(EnsPDatabaseadaptor dba,
                                   AjPSeqout outseq)
{
    AjPSeq seq = NULL;

    AjPStr slname = NULL;

    EnsPSlice slice      = NULL;
    EnsPSliceadaptor sla = NULL;

    if(!dba)
        return ajFalse;

    if(!outseq)
        return ajFalse;

    ajUser("Ensembl Sequence");

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

    ajSeqoutWriteSeq(outseq, seq);

    ajSeqDel(&seq);

    ensSliceDel(&slice);

    /*
    ** Fetch a Slice covering a gap region on human chromosome 21.
    ** chromosome:GRCh37:21:9400001:9800000:1
    */

    ajStrAssignC(&slname, "chromosome:GRCh37:21:9400001:9800000:1");

    ensSliceadaptorFetchByName(sla, slname, &slice);

    ajStrDel(&slname);

    ensSliceFetchSequenceSubSeq(slice,
                                1 - 60,
                                ensSliceCalculateLength(slice) + 60,
                                +1,
                                &seq);

    ajSeqoutWriteSeq(outseq, seq);

    ajSeqDel(&seq);

    ensSliceDel(&slice);

    return ajTrue;
}




/* @funcstatic ensembltest_chromosome *****************************************
**
** Fetch a Slice for chromosome:GRCh37:21:0:0:0 and write its sequence to disk.
**
** @param [u] dba [EnsPDatabaseadaptor] Ensembl Database Adaptor.
** @param [u] outseq [AjPSeqout] AJAX Sequence Output.
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise.
** @@
******************************************************************************/

static AjBool ensembltest_chromosome(EnsPDatabaseadaptor dba,
                                     AjPSeqout outseq)
{
    AjPSeq seq = NULL;

    AjPStr slname = NULL;

    EnsPSlice slice      = NULL;
    EnsPSliceadaptor sla = NULL;

    if(!dba)
        return ajFalse;

    if(!outseq)
        return ajFalse;

    ajUser("Chromosome");

    sla = ensRegistryGetSliceadaptor(dba);

    /* Fetch a Slice for human chromosome:GRCh37:21:0:0:0 */

    slname = ajStrNewC("chromosome:GRCh37:21:0:0:1");

    ensSliceadaptorFetchByName(sla, slname, &slice);

    ensSliceFetchSequenceAllSeq(slice, &seq);

    ajSeqoutWriteSeq(outseq, seq);

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

      ajSeqoutWriteSeq(outseq, seq);

      ajSeqDel(&seq);

      ensSliceDel(&slice);
    */

    ajStrDel(&slname);

    return ajTrue;
}




/* @funcstatic ensembltest_meta ***********************************************
**
** Ensembl Meta-Information tests.
**
** @param [u] dba [EnsPDatabaseadaptor] Ensembl Database Adaptor.
** @param [u] outfile [AjPFile] AJAX File.
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise.
** @@
******************************************************************************/

static AjBool ensembltest_meta(EnsPDatabaseadaptor dba,
                               AjPFile outfile)
{
    AjPStr value = NULL;

    EnsPMetainformationadaptor mia = NULL;

    if(!dba)
        return ajFalse;

    if(!outfile)
        return ajFalse;

    ajFmtPrintF(outfile,
                "\n"
                "Ensembl Meta-Information\n");

    ajUser("Ensembl Meta-Information");

    mia = ensRegistryGetMetainformationadaptor(dba);

    value = ajStrNew();

    if(ensMetainformationadaptorRetrieveGenebuildversion(mia, &value))
        ajFmtPrintF(outfile,
                    "\n"
                    "  Ensembl Meta-Information Genebuild Version '%S'\n",
                    value);

    if(ensMetainformationadaptorRetrieveSchemaversion(mia, &value))
        ajFmtPrintF(outfile,
                    "  Ensembl Meta-Information Schema Version '%S'\n",
                    value);

    if(ensMetainformationadaptorRetrieveTaxonomyidentifier(mia, &value))
        ajFmtPrintF(outfile,
                    "  Ensembl Meta-Information "
                    "NCBI Taxonomy identifier '%S'\n",
                    value);

    ajStrDel(&value);

    return ajTrue;
}




/* @funcstatic ensembltest_coordinate_systems *********************************
**
** Ensembl Coordinate System tests.
**
** @param [u] dba [EnsPDatabaseadaptor] Ensembl Database Adaptor.
** @param [u] outfile [AjPFile] AJAX File.
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise.
** @@
******************************************************************************/

static AjBool ensembltest_coordinate_systems(EnsPDatabaseadaptor dba,
                                             AjPFile outfile)
{
    AjIList iter   = NULL;
    AjPList cslist = NULL;

    const AjPList mplist = NULL;

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

    if(!dba)
        return ajFalse;

    if(!outfile)
        return ajFalse;

    /* Ensembl Coordinate System tests. */

    ajFmtPrintF(outfile,
                "\n"
                "Ensembl Coordinate Systems\n");

    ajUser("Ensembl Coordinate Systems");

    csa = ensRegistryGetCoordsystemadaptor(dba);

    /* Fetch the top-level Ensembl Coordinate System. */

    ajFmtPrintF(outfile,
                "\n"
                "  Top-Level Coordinate System\n");

    ensCoordsystemadaptorFetchToplevel(csa, &cs);

    ajFmtPrintF(outfile,
                "    Ensembl Coordinate System %u rank %u '%S:%S'\n",
                ensCoordsystemGetIdentifier(cs),
                ensCoordsystemGetRank(cs),
                ensCoordsystemGetName(cs),
                ensCoordsystemGetVersion(cs));

    ensCoordsystemDel(&cs);

    /* Fetch the sequence-level Ensembl Coordinate System. */

    ajFmtPrintF(outfile,
                "\n"
                "  Sequence-Level Coordinate System\n");

    ensCoordsystemadaptorFetchSeqlevel(csa, &cs);

    ajFmtPrintF(outfile,
                "    Ensembl Coordinate System %u rank %u '%S:%S'\n",
                ensCoordsystemGetIdentifier(cs),
                ensCoordsystemGetRank(cs),
                ensCoordsystemGetName(cs),
                ensCoordsystemGetVersion(cs));

    ensCoordsystemDel(&cs);

    /* Fetch all Ensembl Coordinate Systems with name 'chromosome'. */

    ajFmtPrintF(outfile,
                "\n"
                "  Coordinate Systems with name 'chromosome'\n");

    csname1 = ajStrNewC("chromosome");

    cslist = ajListNew();

    ensCoordsystemadaptorFetchAllbyName(csa, csname1, cslist);

    while(ajListPop(cslist, (void **) &cs))
    {
        ajFmtPrintF(outfile,
                    "    Ensembl Coordinate System %u rank %u '%S:%S'\n",
                    ensCoordsystemGetIdentifier(cs),
                    ensCoordsystemGetRank(cs),
                    ensCoordsystemGetName(cs),
                    ensCoordsystemGetVersion(cs));

        ensCoordsystemDel(&cs);
    }

    ajListFree(&cslist);

    ajStrDel(&csname1);

    /*
    ** Fetch the Ensembl Coordinate System with
    ** name 'chromosome' and version 'NCBI35'.
    */

    ajFmtPrintF(outfile,
                "\n"
                "  Coordinate Systems with name 'chromosome' "
                "and version 'NCBI35'\n");

    csname1    = ajStrNewC("chromosome");
    csversion1 = ajStrNewC("NCBI35");

    ensCoordsystemadaptorFetchByName(csa, csname1, csversion1, &cs);

    ajFmtPrintF(outfile,
                "    Ensembl Coordinate System %u rank %u '%S:%S'\n",
                ensCoordsystemGetIdentifier(cs),
                ensCoordsystemGetRank(cs),
                ensCoordsystemGetName(cs),
                ensCoordsystemGetVersion(cs));

    ensCoordsystemDel(&cs);

    ajStrDel(&csname1);
    ajStrDel(&csversion1);

    /* Fetch the (default) Ensembl Coordinate System with name 'chromosome'. */

    ajFmtPrintF(outfile,
                "\n"
                "  Default Coordinate System with name 'chromosome'\n");

    csname1 = ajStrNewC("chromosome");

    ensCoordsystemadaptorFetchByName(csa, csname1, (AjPStr) NULL, &cs);

    ajFmtPrintF(outfile,
                "    Ensembl Coordinate System %u rank %u '%S:%S'\n",
                ensCoordsystemGetIdentifier(cs),
                ensCoordsystemGetRank(cs),
                ensCoordsystemGetName(cs),
                ensCoordsystemGetVersion(cs));

    ensCoordsystemDel(&cs);

    ajStrDel(&csname1);

    /* Fetch all Ensembl Coordinate Systems. */

    ajFmtPrintF(outfile,
                "\n"
                "  All Coordinate Systems\n");

    cslist = ajListNew();

    ensCoordsystemadaptorFetchAll(csa, cslist);

    while(ajListPop(cslist, (void **) &cs))
    {
        ajFmtPrintF(outfile,
                    "    Ensembl Coordinate System %u rank %u '%S:%S'\n",
                    ensCoordsystemGetIdentifier(cs),
                    ensCoordsystemGetRank(cs),
                    ensCoordsystemGetName(cs),
                    ensCoordsystemGetVersion(cs));

        ensCoordsystemDel(&cs);
    }

    ajListFree(&cslist);

    /* Fetch specific Ensembl Coordinate Systems. */

    csname1    = ajStrNewC("chromosome");
    csversion1 = ajStrNewC("NCBI36");

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

    ajFmtPrintF(outfile,
                "\n"
                "  Ensembl Coordinate System Mapping path "
                "'%S:%S' - '%S:%S'\n",
                ensCoordsystemGetName(cs1),
                ensCoordsystemGetVersion(cs1),
                ensCoordsystemGetName(cs2),
                ensCoordsystemGetVersion(cs2));

    mplist = ensCoordsystemadaptorGetMappingpath(csa, cs1, cs2);

    iter = ajListIterNewread(mplist);

    while(!ajListIterDone(iter))
    {
        cs = (EnsPCoordsystem) ajListIterGet(iter);

        ajFmtPrintF(outfile,
                    "    Ensembl Coordinate System "
                    "%u rank %u '%S:%S'\n",
                    ensCoordsystemGetIdentifier(cs),
                    ensCoordsystemGetRank(cs),
                    ensCoordsystemGetName(cs),
                    ensCoordsystemGetVersion(cs));
    }

    ajListIterDel(&iter);

    /* cs1 - cs3 */

    ajFmtPrintF(outfile,
                "\n"
                "  Ensembl Coordinate System Mapping path "
                "'%S:%S' - '%S:%S'\n",
                ensCoordsystemGetName(cs1),
                ensCoordsystemGetVersion(cs1),
                ensCoordsystemGetName(cs3),
                ensCoordsystemGetVersion(cs3));

    mplist = ensCoordsystemadaptorGetMappingpath(csa, cs1, cs3);

    iter = ajListIterNewread(mplist);

    while(!ajListIterDone(iter))
    {
        cs = (EnsPCoordsystem) ajListIterGet(iter);

        ajFmtPrintF(outfile,
                    "    Ensembl Coordinate System "
                    "%u rank %u '%S:%S'\n",
                    ensCoordsystemGetIdentifier(cs),
                    ensCoordsystemGetRank(cs),
                    ensCoordsystemGetName(cs),
                    ensCoordsystemGetVersion(cs));
    }

    ajListIterDel(&iter);

    /* cs2 - cs3 */

    ajFmtPrintF(outfile,
                "\n"
                "  Ensembl Coordinate System Mapping path "
                "'%S:%S' - '%S:%S'\n",
                ensCoordsystemGetName(cs2),
                ensCoordsystemGetVersion(cs2),
                ensCoordsystemGetName(cs3),
                ensCoordsystemGetVersion(cs3));

    mplist = ensCoordsystemadaptorGetMappingpath(csa, cs2, cs3);

    iter = ajListIterNewread(mplist);

    while(!ajListIterDone(iter))
    {
        cs = (EnsPCoordsystem) ajListIterGet(iter);

        ajFmtPrintF(outfile,
                    "    Ensembl Coordinate System "
                    "%u rank %u '%S:%S'\n",
                    ensCoordsystemGetIdentifier(cs),
                    ensCoordsystemGetRank(cs),
                    ensCoordsystemGetName(cs),
                    ensCoordsystemGetVersion(cs));
    }

    ajListIterDel(&iter);

    ensCoordsystemDel(&cs1);
    ensCoordsystemDel(&cs2);
    ensCoordsystemDel(&cs3);

    return ajTrue;
}




/* @funcstatic ensembltest_sequence_regions ***********************************
**
** Ensembl Sequence Region tests.
**
** @param [u] dba [EnsPDatabaseadaptor] Ensembl Database Adaptor.
** @param [u] outfile [AjPFile] AJAX File.
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise.
** @@
******************************************************************************/

static AjBool ensembltest_sequence_regions(EnsPDatabaseadaptor dba,
                                           AjPFile outfile)
{
    AjIList iter       = NULL;
    const AjPList list = NULL;

    EnsPAttribute attribute = NULL;

    EnsPCoordsystem cs = NULL;

    EnsPSeqregion sr         = NULL;
    EnsPSeqregionadaptor sra = NULL;

    if(!dba)
        return ajFalse;

    if(!outfile)
        return ajFalse;

    /* Ensembl Sequence Region tests. */

    ajFmtPrintF(outfile,
                "\n"
                "Ensembl Sequence Regions\n");

    ajUser("Ensembl Sequence Regions");

    sra = ensRegistryGetSeqregionadaptor(dba);

    /*
    ** Fetch the human chromosome 1 via its identifier 226034.
    ** 223781 AC120349.5
    */

    ensSeqregionadaptorFetchByIdentifier(sra, 226034U, &sr);

    cs = ensSeqregionGetCoordsystem(sr);

    ajFmtPrintF(outfile,
                "\n"
                "  Ensembl Sequence Region %u name '%S' length %u '%S:%S'\n",
                ensSeqregionGetIdentifier(sr),
                ensSeqregionGetName(sr),
                ensSeqregionGetLength(sr),
                ensCoordsystemGetName(cs),
                ensCoordsystemGetVersion(cs));

    list = ensSeqregionLoadAttributes(sr);

    iter = ajListIterNewread(list);

    while(!ajListIterDone(iter))
    {
        attribute = (EnsPAttribute) ajListIterGet(iter);

        ajFmtPrintF(outfile,
                    "    Ensembl Attribute name '%S' code '%S' value '%S'\n",
                    ensAttributeGetName(attribute),
                    ensAttributeGetCode(attribute),
                    ensAttributeGetValue(attribute));
    }

    ajListIterDel(&iter);

    ensSeqregionDel(&sr);

    return ajTrue;
}




/* @funcstatic ensembltest_transformations ************************************
**
** Ensembl Feature transformation tests.
**
** @param [u] dba [EnsPDatabaseadaptor] Ensembl Database Adaptor.
** @param [u] outfile [AjPFile] AJAX File.
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise.
** @@
******************************************************************************/

static AjBool ensembltest_transformations(EnsPDatabaseadaptor dba,
                                          AjPFile outfile)
{
    ajuint version = 0;

    AjBool debug = AJFALSE;

    AjPStr slname   = NULL;
    AjPStr stableid = NULL;

    EnsPFeature feature = NULL;

    EnsPGene oldgene   = NULL;
    EnsPGene newgene   = NULL;
    EnsPGeneadaptor ga = NULL;

    EnsPSlice slice      = NULL;
    EnsPSliceadaptor sla = NULL;

    debug = ajDebugTest("ensembltest_transformations");

    if(!dba)
        return ajFalse;

    if(!outfile)
        return ajFalse;

    /* Ensembl Feature transfer and transform tests. */

    ajFmtPrintF(outfile,
                "\n"
                "Ensembl Feature transfer and transform\n");

    ajUser("Ensembl Feature transfer and transform");

    if(debug)
        ajDebug("ensembltest_transformations\n");

    ga = ensRegistryGetGeneadaptor(dba);

    stableid = ajStrNewC("ENSG00000175387");

    ensGeneadaptorFetchByStableidentifier(ga, stableid, version, &oldgene);

    ensGeneLoadTranscripts(oldgene);

    feature = ensGeneGetFeature(oldgene);

    ajFmtPrintF(outfile,
                "\n"
                "  Ensembl Gene %S\n"
                "    Ensembl Feature %S:%d:%d:%d\n",
                ensGeneGetStableidentifier(oldgene),
                ensFeatureGetSeqregionName(feature),
                ensFeatureGetStart(feature),
                ensFeatureGetEnd(feature),
                ensFeatureGetStrand(feature));

    sla = ensRegistryGetSliceadaptor(dba);

    slname = ajStrNewC("chromosome:GRCh37:18:40000000:50000000:1");

    ensSliceadaptorFetchByName(sla, slname, &slice);

    newgene = ensGeneTransfer(oldgene, slice);

    feature = ensGeneGetFeature(newgene);

    ajFmtPrintF(outfile,
                "\n"
                "  Ensembl Gene %S\n"
                "    Ensembl Feature %S:%d:%d:%d\n",
                ensGeneGetStableidentifier(newgene),
                ensFeatureGetSeqregionName(feature),
                ensFeatureGetStart(feature),
                ensFeatureGetEnd(feature),
                ensFeatureGetStrand(feature));

    ensSliceDel(&slice);

    ensGeneDel(&newgene);
    ensGeneDel(&oldgene);

    ajStrDel(&slname);
    ajStrDel(&stableid);

    return ajTrue;
}




/* @funcstatic ensembltest_registry *******************************************
**
** Ensembl Registry tests.
**
** @param [u] outfile [AjPFile] AJAX File.
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise.
** @@
******************************************************************************/

static AjBool ensembltest_registry(AjPFile outfile)
{
    const char *identifiers[] =
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

    register ajuint i = 0;

    EnsEDatabaseadaptorGroup group = ensEDatabaseadaptorGroupNULL;

    AjPStr identifier = NULL;
    AjPStr species    = NULL;

    if(!outfile)
        return ajFalse;

    /* Ensembl Registry tests. */

    ajFmtPrintF(outfile,
                "\n"
                "Ensembl Registry\n");

    ajUser("Ensembl Registry");

    /* Registry test of stable identifier expressions. */

    ensRegistryIdentifierLoadFile((AjPStr) NULL);

    for(i = 0; identifiers[i]; i++) {

        identifier = ajStrNewC(identifiers[i]);

        species = ajStrNew();

        ensRegistryIdentifierResolve(identifier, &species, &group);

        ajFmtPrintF(outfile,
                    "\n"
                    "  identifier '%S' species '%S' group '%s'\n",
                    identifier, species, ensDatabaseadaptorGroupToChar(group));

        ajStrDel(&identifier);
        ajStrDel(&species);
    }

    return ajTrue;
}




/* @funcstatic ensembltest_density ********************************************
**
** Ensembl Density Type and Ensembl Density Feature tests.
**
** @param [u] dba [EnsPDatabaseadaptor] Ensembl Database Adaptor.
** @param [u] outfile [AjPFile] AJAX File.
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise.
** @@
******************************************************************************/

static AjBool ensembltest_density(EnsPDatabaseadaptor dba,
                                  AjPFile outfile)
{
    float maxratio = 0;

    ajuint blocks = 100;

    AjBool interpolate = AJFALSE;

    AjPList dfs = NULL;

    AjPStr anname = NULL;
    AjPStr slname = NULL;

    EnsPAnalysis analysis = NULL;

    EnsPDensityfeature df         = NULL;
    EnsPDensityfeatureadaptor dfa = NULL;

    EnsPDensitytype dt = NULL;

    EnsPFeature feature = NULL;

    EnsPSlice slice      = NULL;
    EnsPSliceadaptor sla = NULL;

    if(!dba)
        return ajFalse;

    if(!outfile)
        return ajFalse;

    /* Ensembl Registry tests. */

    ajFmtPrintF(outfile,
                "\n"
                "Ensembl Density\n");

    ajUser("Ensembl Density");

    /* Registry test of stable identifier expressions. */

    dfa = ensRegistryGetDensityfeatureadaptor(dba);

    sla = ensRegistryGetSliceadaptor(dba);

    slname = ajStrNewC("toplevel::22:0:0:1");

    ensSliceadaptorFetchByName(sla, slname, &slice);

    ensSliceFetchName(slice, &slname);

    /*
    ** PercentageRepeat
    ** geneDensity
    ** snpDensity
    ** PercentageRepeat
    ** knownGeneDensity
    ** PercentGC
    */

    anname = ajStrNewC("PercentGC");

    interpolate = ajTrue;

    maxratio = 5.0;

    dfs = ajListNew();

    ensDensityfeatureadaptorFetchAllbySlice(dfa,
                                            slice,
                                            anname,
                                            blocks,
                                            interpolate,
                                            maxratio,
                                            dfs);

    ajFmtPrintF(outfile,
                "\n"
                "  Ensembl Slice '%S' %u Density Features\n\n",
                slname,
                ajListGetLength(dfs));

    while(ajListPop(dfs, (void **) &df))
    {
        feature = ensDensityfeatureGetFeature(df);

        dt = ensDensityfeatureGetDensitytype(df);

        analysis = ensDensitytypeGetAnalysis(dt);

        ajFmtPrintF(outfile,
                    "    %u '%S' %u %S:%d:%d:%d value %f\n",
                    ensDensityfeatureGetIdentifier(df),
                    ensAnalysisGetName(analysis),
                    ensDensitytypeGetSize(dt),
                    ensFeatureGetSeqregionName(feature),
                    ensFeatureGetSeqregionStart(feature),
                    ensFeatureGetSeqregionEnd(feature),
                    ensFeatureGetSeqregionStrand(feature),
                    ensDensityfeatureGetValue(df));

        ensDensityfeatureDel(&df);
    }

    ajListFree(&dfs);

    ensSliceDel(&slice);

    ajStrDel(&anname);
    ajStrDel(&slname);

    return ajTrue;
}
