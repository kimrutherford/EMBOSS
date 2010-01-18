/******************************************************************************
** @source ensembltest application
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
** @attr SliceName [const char*] Ensembl Slice name.
** @attr CoordsystemName [const char*] Ensembl Coordinate System name.
** @attr CoordsystemVersion [const char*] Ensembl Coordinate System version.
** @@
******************************************************************************/

typedef struct EnsembltestSProjections
{
    const char *SliceName;
    const char *CoordsystemName;
    const char *CoordsystemVersion;
} EnsembltestOProjections;

#define EnsembltestPProjections EnsembltestOProjections*



static AjBool ensembltest_registry(AjPFile outfile);
static AjBool ensembltest_analyses(EnsPDatabaseadaptor dba, AjPFile outfile);
static AjBool ensembltest_meta(EnsPDatabaseadaptor dba, AjPFile outfile);
static AjBool ensembltest_coordinate_systems(EnsPDatabaseadaptor dba,
                                             AjPFile outfile);
static AjBool ensembltest_sequence_regions(EnsPDatabaseadaptor dba,
                                           AjPFile outfile);
static AjBool ensembltest_assembly_exceptions(EnsPDatabaseadaptor dba,
                                              AjPFile outfile);
static AjBool ensembltest_slice_projections(EnsPDatabaseadaptor dba,
                                            AjPFile outfile);
static AjBool ensembltest_features(EnsPDatabaseadaptor dba, AjPFile outfile);
static AjBool ensembltest_masking(EnsPDatabaseadaptor dba, AjPSeqout outseq);
static AjBool ensembltest_markers(EnsPDatabaseadaptor dba,  AjPFile outfile);
static AjBool ensembltest_ditags(EnsPDatabaseadaptor dba,  AjPFile outfile);
static AjBool ensembltest_transformations(EnsPDatabaseadaptor dba,
                                          AjPFile outfile);
static AjBool ensembltest_density(EnsPDatabaseadaptor dba, AjPFile outfile);
static AjBool ensembltest_genes(EnsPDatabaseadaptor dba);




/* @prog testensembl **********************************************************
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
    
    AjEnum client = ajESqlClientMySQL;
    AjEnum group  = ensEDatabaseadaptorGroupCore;
    
    AjPFile outfile = NULL;
    
    AjIList iter         = NULL;
    AjPList list         = NULL;
    const AjPList exlist = NULL;
    AjPList mrlist       = NULL;
    
    AjPSeq seq = NULL;
    
    AjPSeqout outseq = NULL;
    
    AjPStr user     = NULL;
    AjPStr password = NULL;
    AjPStr host     = NULL;
    AjPStr port     = NULL;
    AjPStr socket   = NULL;
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
    EnsPTranscriptadaptor ta  = NULL;
    
    EnsPTranslation translation = NULL;
    
    embInit("ensembltest", argc, argv);
    
    list     = ajListNew();
    user     = ajStrNewC("anonymous");
    password = ajStrNew();
    host     = ajStrNewC("ensembldb.ensembl.org");
    port     = ajStrNewC("5306");
    socket   = ajStrNew();
    dbname   = ajStrNew();
    species  = ajStrNewC("homo sapiens");
    srname   = ajStrNew();
    
    outfile = ajAcdGetOutfile("outfile");
    
    outseq = ajAcdGetSeqoutall("outseq");
    
    large = ajAcdGetBoolean("large");
    
    /* AJAX SQL test. */
    
    /* This needs moving into Ensembl Init and AJAX init */
    
    if(!ajSqlInit())
	ajFatal("main Library initialisation failed.");
    
    ensInit();
    
    dbc = ensDatabaseconnectionNew(client,
				   user,
				   password,
				   host,
				   port,
				   socket,
				   dbname);
    
    if(!dbc)
	ajFatal("main Could not connect as user '%S' to server '%S' "
		"at port '%S' to database '%S'.", user, host, port, dbname);
    
    /* Ensembl Registry test. */
    
    ensRegistryLoadFromServer(dbc);
    
    ensDatabaseconnectionDel(&dbc);
    
    ensRegistryTraceAliases(0);
    
    ensRegistryTraceEntries(0);
    
    ajDebug("main ensRegistryLoadFromServer finished.\n");
    
    dba = ensRegistryGetDatabaseadaptor(group, species);
    
    /* Fetch Exon ENSE00001434436 */
    
    ea = ensRegistryGetExonadaptor(dba);
    
    tmpstr = ajStrNewC("ENSE00001191187");
    
    ensExonadaptorFetchByStableIdentifier(ea, tmpstr, 0, &exon);
    
    ajFmtPrintF(outfile,
		"Ensembl Exon '%S'\n",
		ensExonGetStableIdentifier(exon));
    
    ensExonFetchSequenceSeq(exon, &seq);
    
    if(seq)
	ajSeqoutWriteSeq(outseq, seq);
    else
	ajFatal("main could not get an AJAX sequence.\n");
    
    ajSeqDel(&seq);
    
    ensExonDel(&exon);
    
    ajStrDel(&tmpstr);
    
    /* Fetch Transcript ENST00000262160 */
    
    ta = ensRegistryGetTranscriptadaptor(dba);
    
    tmpstr = ajStrNewC("ENST00000262160");
    
    ensTranscriptadaptorFetchByStableIdentifier(ta, tmpstr, 0, &transcript);
    
    ajStrDel(&tmpstr);
    
    if(transcript)
    {
	feature = ensTranscriptGetFeature(transcript);
	
	ajFmtPrintF(outfile,
		    "Ensembl Transcript '%S' %S:%d:%d:%d\n",
		    ensTranscriptGetStableIdentifier(transcript),
		    ensFeatureGetSeqregionName(feature),
		    ensFeatureGetSeqregionStart(feature),
		    ensFeatureGetSeqregionEnd(feature),
		    ensFeatureGetSeqregionStrand(feature));
	
	/* Transcript Mapper Test */
	
	mrlist = ajListNew();
	
	ensTranscriptMapperTranscript2Slice(transcript, 2, 3, mrlist);
	
	while(ajListPop(mrlist, (void **) &mr))
	{
	    if(ensMapperresultGetType(mr) == ensEMapperresultCoordinate)
	    {
		
		ajFmtPrintF(outfile,
			    "Ensembl Transcript Mapper map positions 2:3 to "
			    "Slice %d:%d:%d\n",
			    ensMapperresultGetStart(mr),
			    ensMapperresultGetEnd(mr),
			    ensMapperresultGetStrand(mr));
	    }
	    else if(ensMapperresultGetType(mr) == ensEMapperresultGap)
	    {
		ajFmtPrintF(outfile,
			    "Ensembl Transcript Mapper 2:3 to Slice got "
			    "Mapper Result of type Gap %d:%d. \n",
			    ensMapperresultGetGapStart(mr),
			    ensMapperresultGetGapEnd(mr));
	    }
	    else
	    {
		ajFmtPrintF(outfile,
			    "Ensembl Transcript Mapper 2:3 to Slice got "
			    "Mapper Result of other type?\n");	    
		
		ensMapperresultTrace(mr, 1);
	    }
	    
	    ensMapperresultDel(&mr);
	}
	
	ajListFree(&mrlist);
	
	/* Trace the Transcript Mapper */
	
	/*
	** FIXME: For debugging only!
	 ensMapperTrace(transcript->ExonCoordMapper, 1);
	*/
	
	/* Fetch the Transcript sequence. */
	
	ensTranscriptFetchSequenceSeq(transcript, &seq);
	
	ajSeqoutWriteSeq(outseq, seq);
	
	ajSeqDel(&seq);
	
	translation = ensTranscriptFetchTranslation(transcript);
	
	if(translation)
	{
	    ensTranslationFetchSequenceSeq(translation, &seq);
	    
	    ajSeqoutWriteSeq(outseq, seq);
	    
	    ajSeqDel(&seq);
	}
	
	/*
	** Fetch all Exons of this Transcript and write out their
	** coordinates.
	*/
	
	exlist = ensTranscriptGetExons(transcript);
	
	iter = ajListIterNewread(exlist);
	
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
			ensExonGetStableIdentifier(exon),
			ensExonGetTranscriptStart(exon, transcript),
			ensExonGetTranscriptEnd(exon, transcript),
			ensFeatureGetStart(feature),
			ensFeatureGetEnd(feature),
			ensFeatureGetStrand(feature),
			ensExonGetTranscriptCodingStart(exon, transcript),
			ensExonGetTranscriptCodingEnd(exon, transcript),
			ensExonGetSliceCodingStart(exon, transcript),
			ensExonGetSliceCodingEnd(exon, transcript));
	}
	
	ajListIterDel(&iter);
	
	ensTranslationDel(&translation);
    }
    
    ensTranscriptDel(&transcript);
    
    ensembltest_registry(outfile);
    
    ensembltest_analyses(dba, outfile);
    
    ensembltest_meta(dba, outfile);
    
    ensembltest_coordinate_systems(dba, outfile);
    
    ensembltest_sequence_regions(dba, outfile);
    
    ensembltest_assembly_exceptions(dba, outfile);
    
    ensembltest_slice_projections(dba, outfile);
    
    ensembltest_features(dba, outfile);
    
    ensembltest_masking(dba, outseq);
    
    ensembltest_markers(dba, outfile);
    
    ensembltest_ditags(dba, outfile);
    
    ensembltest_transformations(dba, outfile);
    
    ensembltest_density(dba, outfile);
    
    /* Fetch larger data sets... */
    
    if(large)
    {
	ensembltest_genes(dba);
	
	/* FIXME: Fetch sequence for human chromosome 21.
	ensembltest_chromosome(dba, outseq);
	*/
    }
    
    /* Clean up and exit. */
    
    ensExit();
    
    ajListFree(&list);
    
    ajFileClose(&outfile);
    
    ajSeqoutClose(outseq);
    
    ajSeqoutDel(&outseq);
    
    ajStrDel(&user);
    ajStrDel(&password);
    ajStrDel(&host);
    ajStrDel(&port);
    ajStrDel(&socket);
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
	    /*
	     ru.ru_utime.tv_sec * 1.0E6 + ru.ru_utime.tv_usec,
	     ru.ru_stime.tv_sec * 1.0E6 + ru.ru_stime.tv_usec,
	     */
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
** @param [r] dba [EnsPDatabaseadaptor] Ensembl Database Adaptor.
** @param [r] slname [AjPStr] Ensembl Slice name.
** @param [r] csname [AjPStr] Ensembl Coordinate System name.
** @param [r] csversion [AjPStr] Ensembl Coordinate System version.
** @param [r] outfile [AjPFile] AJAX File.
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise.
** @@
******************************************************************************/

static AjBool ensembltest_slice_projections(EnsPDatabaseadaptor dba,
                                            AjPFile outfile)
{
    ajint i = 0;
    
    AjPList pslist = NULL;
    
    AjPStr slname    = NULL;
    AjPStr csname    = NULL;
    AjPStr csversion = NULL;
    AjPStr sename    = NULL;
    AjPStr psname    = NULL;
    
    EnsPProjectionsegment ps = NULL;
    
    EnsPSlice psslice   = NULL;
    EnsPSlice slice     = NULL;
    EnsPSliceadaptor sa = NULL;
    
    /*
    ** Map the clone:NCBI36:AC120349.5:1:148596:1 Slice back to the
    ** 'chromosome' Coordinate System, which will use the Chained Assembly
    ** Mapper for the mapping path 'clone' <-> 'contig' <-> 'chromosome'.
    */
    
    EnsembltestOProjections ensembltestProjections[] =
    {
	/*
	** FIXME: These do not work, because AJAX does not cope well with ::
	 {"clone::AC120349.5:1:148596:1", "chromosome", "NCBI36"},
	 {"clone::AC120349.5:1:148596:1", "toplevel", ""},
	*/
    {"clone::AC120349.5:0:0:0", "chromosome", "NCBI36"},
    {"clone::AC120349.5:0:0:0", "toplevel", ""},
    {"chromosome:NCBI36:X:1:3000000:1", "contig", ""},
    {"chromosome:NCBI36:X:1:3000000:1", "clone", ""},
    {"chromosome:NCBI36:Y:1:3000000:1", "contig", ""},
    {"chromosome:NCBI36:Y:1:3000000:1", "clone", ""},
    {NULL, NULL, NULL}
    };
    
    if(!dba)
	return ajFalse;
    
    if(!outfile)
	return ajFalse;
    
    ajFmtPrintF(outfile, "\n");
    
    ajFmtPrintF(outfile, "Ensembl Slice Projections\n");
    
    sa = ensRegistryGetSliceadaptor(dba);
    
    pslist = ajListNew();
    
    slname = ajStrNew();
    
    csname = ajStrNew();
    
    csversion = ajStrNew();
    
    for(i = 0; ensembltestProjections[i].SliceName; i++)
    {
	ajStrAssignC(&slname, ensembltestProjections[i].SliceName);
	ajStrAssignC(&csname, ensembltestProjections[i].CoordsystemName);
	ajStrAssignC(&csversion, ensembltestProjections[i].CoordsystemVersion);
	
	ensSliceadaptorFetchByName(sa, slname, &slice);
	
	ensSliceFetchName(slice, &sename);
	
	ajFmtPrintF(outfile, "\n");
	
	ajFmtPrintF(outfile,
		    "  Project Ensembl Slice '%S' into\n"
		    "  Ensembl Coordinate System '%S:%S'.\n"
		    "\n", sename, csname, csversion);
	
	/*
	 ajDebug("ensembltest_assembly_exceptions begin ensSliceProject for "
		 "Ensembl Slice '%S' and "
		 "Ensembl Coordinate System '%S:%S'.\n"
		 sename, csname, csversion);
	 */
	
	ensSliceProject(slice, csname, csversion, pslist);
	
	/*
	 ajDebug(
		 "ensembltest_assembly_exceptions finished "
		 "ensSliceProject.\n"
		 );
	 */
	
	while(ajListPop(pslist, (void **) &ps))
	{
	    psslice = ensProjectionsegmentGetTrgSlice(ps);
	    
	    ensSliceFetchName(psslice, &psname);
	    
	    ajFmtPrintF(outfile,
			"    '%S' %u:%u\n"
			"    '%S'\n"
			"\n",
			sename,
			ensProjectionsegmentGetSrcStart(ps),
			ensProjectionsegmentGetSrcEnd(ps),
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
** @param [r] dba [EnsPDatabaseadaptor] Ensembl Database Adaptor.
** @param [r] outfile [AjPFile] AJAX File.
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise.
** @@
******************************************************************************/

static AjBool ensembltest_analyses(EnsPDatabaseadaptor dba, AjPFile outfile)
{
    AjPList list = NULL;
    
    AjPStr name = NULL;
    
    EnsPAnalysis analysis  = NULL;
    EnsPAnalysisadaptor aa = NULL;
    
    if(!dba)
	return ajFalse;
    
    if(!outfile)
	return ajFalse;
    
    aa = ensRegistryGetAnalysisadaptor(dba);
    
    ajFmtPrintF(outfile, "\n");
    ajFmtPrintF(outfile, "Ensembl Analyses\n");
    
    /* Fetch the Ensembl Analysis for identifier 1 */
    
    ajFmtPrintF(outfile, "\n");
    ajFmtPrintF(outfile, "  Ensembl Analysis for identifier 1\n");
    
    ensAnalysisadaptorFetchByIdentifier(aa, 1, &analysis);
    
    ajFmtPrintF(outfile,
		"    Ensembl Analyis %u name '%S'\n",
		ensAnalysisGetIdentifier(analysis),
		ensAnalysisGetName(analysis));
    
    ensAnalysisDel(&analysis);
    
    /* Fetch the Ensembl Analysis for name 'cpg' */
    
    ajFmtPrintF(outfile, "\n");
    ajFmtPrintF(outfile, "  Ensembl Analysis for name 'cpg'\n");
    
    name = ajStrNewC("cpg");
    
    ensAnalysisadaptorFetchByName(aa, name, &analysis);
    
    ajFmtPrintF(outfile,
		"    Ensembl Analyis %u name '%S'\n",
		ensAnalysisGetIdentifier(analysis),
		ensAnalysisGetName(analysis));
    
    ensAnalysisDel(&analysis);
    
    ajStrDel(&name);
    
    /* Fetch all Ensembl Analyses */
    
    ajFmtPrintF(outfile, "\n");
    
    ajFmtPrintF(outfile, "  All Ensembl Analyses\n");
    
    list = ajListNew();
    
    ensAnalysisadaptorFetchAll(aa, list);
    
    while(ajListPop(list, (void **) &analysis))
    {
	ajFmtPrintF(outfile,
		    "    Ensembl Analyis %u name '%S'\n",
		    ensAnalysisGetIdentifier(analysis),
		    ensAnalysisGetName(analysis));
	
	ensAnalysisDel(&analysis);
    }
    
    ajListFree(&list);
    
    /* Test SQL escaping of "'" characters. */
    
    ajFmtPrintF(outfile, "\n");
    ajFmtPrintF(outfile, "  Test SQL escaping of \"'\" characters.\n");
    
    name = ajStrNewC("'test'");
    
    ensAnalysisadaptorFetchByName(aa, name, &analysis);
    
    if(analysis)
	ajFmtPrintF(outfile,
		    "    Ensembl Analyis for name '%S' %u name '%S'\n",
		    name,
		    ensAnalysisGetIdentifier(analysis),
		    ensAnalysisGetName(analysis));
    else
	ajFmtPrintF(outfile, "    No Ensembl Analysis for name '%S'.\n", name);
    
    ensAnalysisDel(&analysis);
    
    ajStrDel(&name);
    
    return ajTrue;
}




/* @funcstatic ensembltest_assembly_exceptions ********************************
**
** Project Ensembl Slices crossing Assembly Exceptions.
**
** @param [r] dba [EnsPDatabaseadaptor] Ensembl Database Adaptor.
** @param [r] outfile [AjPFile] AJAX File.
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise.
** @@
******************************************************************************/

static AjBool ensembltest_assembly_exceptions(EnsPDatabaseadaptor dba,
                                              AjPFile outfile)
{
    AjBool duplicates = AJTRUE;
    AjBool nonref = AJTRUE;
    
    AjPList pslist = NULL;
    AjPList sllist = NULL;
    
    AjPStr csname    = NULL;
    AjPStr csversion = NULL;
    AjPStr slname    = NULL;
    AjPStr psname    = NULL;
    
    EnsPProjectionsegment ps = NULL;
    
    EnsPSlice psslice   = NULL;
    EnsPSlice slice     = NULL;
    EnsPSliceadaptor sa = NULL;
    
    if(!dba)
	return ajFalse;
    
    if(!outfile)
	return ajFalse;
    
    ajFmtPrintF(outfile, "\n");
    ajFmtPrintF(outfile, "Ensembl Assembly Exceptions\n");
    
    sa = ensRegistryGetSliceadaptor(dba);
    
    csname = ajStrNewC("toplevel");
    
    csversion = ajStrNew();
    
    pslist = ajListNew();
    
    sllist = ajListNew();
    
    ensSliceadaptorFetchAll(sa, csname, csversion, nonref, duplicates, sllist);
    
    while(ajListPop(sllist, (void **) &slice))
    {
	ensSliceFetchName(slice, &slname);
	
	ajFmtPrintF(outfile,
		    "\n"
		    "Fetch normalised Slice Projections for Ensembl Slice\n"
		    "'%S'\n"
		    "\n", slname);
	
	/*
	 ajDebug("ensembltest_assembly_exceptions begin "
		 "ensSliceadaptorFetchNormalisedSliceProjection "
		 "for Ensembl Slice '%S'.\n", slname);
	 */
	
	ensSliceadaptorFetchNormalisedSliceProjection(sa, slice, pslist);
	
	/*
	 ajDebug("ensembltest_assembly_exceptions finished "
		 "ensSliceadaptorFetchNormalisedSliceProjection.\n");
	 */
	
	while(ajListPop(pslist, (void **) &ps))
	{
	    psslice = ensProjectionsegmentGetTrgSlice(ps);
	    
	    ensSliceFetchName(psslice, &psname);
	    
	    ajFmtPrintF(outfile,
			"'%S' %u:%u\n"
			"'%S'\n"
			"\n",
			slname,
			ensProjectionsegmentGetSrcStart(ps),
			ensProjectionsegmentGetSrcEnd(ps),
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
    
    return ajTrue;
}




/* @funcstatic ensembltest_features *******************************************
**
** Ensembl Feature tests.
**
** @param [r] dba [EnsPDatabaseadaptor] Ensembl Database Adaptor.
** @param [r] outfile [AjPFile] AJAX File.
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise.
** @@
******************************************************************************/

static AjBool ensembltest_features(EnsPDatabaseadaptor dba, AjPFile outfile)
{
    AjPList exlist = NULL;
    
    AjPStr csname    = NULL;
    AjPStr csversion = NULL;
    AjPStr srname    = NULL;
    AjPStr name      = NULL;
    
    EnsPExon exon      = NULL;
    EnsPExonadaptor ea = NULL;
    
    EnsPFeature feature = NULL;
    
    EnsPGene gene      = NULL;
    EnsPGeneadaptor ga = NULL;
    
    EnsPSlice slice     = NULL;
    EnsPSliceadaptor sa = NULL;
    
    EnsPTranscript transcript = NULL;
    EnsPTranscriptadaptor ta  = NULL;

    if(!dba)
	return ajFalse;
    
    if(!outfile)
	return ajFalse;
    
    ajFmtPrintF(outfile, "\n");
    ajFmtPrintF(outfile, "Ensembl Features\n");
    ajFmtPrintF(outfile, "\n");
    
    sa = ensRegistryGetSliceadaptor(dba);
    
    /*
    ** Fetch an Ensembl Slice for chromosome:NCBI36:18:43676900:43677100:1,
    ** which is inside SMAD2.
    */
    
    csname = ajStrNewC("chromosome");
    
    csversion = ajStrNewC("NCBI36");
    
    srname = ajStrNewC("18");
    
    ensSliceadaptorFetchByRegion(sa,
				 csname,
				 csversion,
				 srname,
				 43676900,
				 43677100,
				 1,
				 &slice);
    
    ajStrDel(&csname);
    ajStrDel(&csversion);
    ajStrDel(&srname);
    
    /* Fetch all Genes on this Slice. */
    
    ga = ensRegistryGetGeneadaptor(dba);
    
    /* Fetch all Transcripts on this Slice. */
    
    ta = ensRegistryGetTranscriptadaptor(dba);
    
    /* Fetch all Exons on this Slice. */
    
    ea = ensRegistryGetExonadaptor(dba);
    
    exlist = ajListNew();
    
    ensExonadaptorFetchAllBySlice(ea, slice, exlist);
    
    while(ajListPop(exlist, (void **) &exon))
    {
	ensSliceFetchName(slice, &name);
	
	ajFmtPrintF(outfile, "  Ensembl Slice '%S'\n", name);
	
	ajStrDel(&name);
	
	feature = ensExonGetFeature(exon);
	
	ajFmtPrintF(outfile,
		    "  Ensembl Exon Feature Slice coordinates "
		    "'%S:%d:%d:%d'\n",
		    ensExonGetStableIdentifier(exon),
		    ensFeatureGetStart(feature),
		    ensFeatureGetEnd(feature),
		    ensFeatureGetStrand(feature));
	
	ajFmtPrintF(outfile,
		    "  Ensembl Exon Sequence Region coordinates "
		    "'%S:%d:%d:%d'\n",
		    ensExonGetStableIdentifier(exon),
		    ensFeatureGetSeqregionStart(feature),
		    ensFeatureGetSeqregionEnd(feature),
		    ensFeatureGetSeqregionStrand(feature));
	
	ensExonDel(&exon);
    }
    
    ajListFree(&exlist);
    
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
** @param [r] dba [EnsPDatabaseadaptor] Ensembl Database Adaptor.
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise.
** @@
******************************************************************************/

static AjBool ensembltest_genes(EnsPDatabaseadaptor dba)
{
    ajint i      = 0;
    ajint maxnum = 0;
    
    AjIList triter = NULL;
    AjPList exlist = NULL;
    AjPList gnlist = NULL;

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
    
    if(!dba)
	return ajFalse;
    
    maxnum = ajAcdGetInt("maxnum");
    
    exnoutseq = ajAcdGetSeqoutall("exonsoutseq");
    trcoutseq = ajAcdGetSeqoutall("transcriptsoutseq");
    trloutseq = ajAcdGetSeqoutall("translationsoutseq");
    
    ga = ensRegistryGetGeneadaptor(dba);
    
    /* Fetch all Genes. */
    
    gnlist = ajListNew();
    
    ensGeneadaptorFetchAll(ga, gnlist);
    
    while(ajListPop(gnlist, (void **) &gene))
    {
	/* Check if a limit has been set and if it has not been exceeded. */
	
	if((maxnum < 0) || (i < maxnum))
	{
	    /*
	     ajDebug("ensembltest_genes "
		     "Gene stable identifier: '%S'\n",
		     ensGeneGetStableIdentifier(gene));
	     */
	    
	    /* Get all Transcripts of this Gene. */
	    
	    trlist = ensGeneGetTranscripts(gene);
	    
	    triter = ajListIterNewread(trlist);
	    
	    while(!ajListIterDone(triter))
	    {
		transcript = (EnsPTranscript) ajListIterGet(triter);
		
		/*
		 ajDebug("ensembltest_genes "
			 "Transcript stable identifier: '%S'\n",
			 ensTranscriptGetStableIdentifier(transcript));
		 */
		
		/* Fetch and write the sequence of this Transcript. */
		
		ensTranscriptFetchSequenceSeq(transcript, &seq);
		
		ajSeqoutWriteSeq(trcoutseq, seq);
		
		ajSeqDel(&seq);
		
		/* Fetch the Translation of this Transcript. */
		
		translation = ensTranscriptFetchTranslation(transcript);
		
		/* Not every Transcript has a Translation. */
		
		/*
                ** FIXME: EMBOSS crashes if the AJAX Sequence object is not
                ** defined.
                */
		
		if(translation)
		{
		    /*
		     ajDebug("ensembltest_genes "
			     "Translation stable identifier: '%S'\n",
			     ensTranslationGetStableIdentifier(translation));
		     */
		    
		    /* Fetch and write the sequence of this Translation. */
		    
		    ensTranslationFetchSequenceSeq(translation, &seq);
		    
		    ajSeqoutWriteSeq(trloutseq, seq);
		    
		    ajSeqDel(&seq);
		    
		    ensTranslationDel(&translation);
		}
	    }
	    
	    ajListIterDel(&triter);
	    
	    /* Fetch all Exons of this Gene. */
	    
	    exlist = ajListNew();
	    
	    ensGeneFetchAllExons(gene, exlist);
	    
	    while(ajListPop(exlist, (void **) &exon))
	    {
		/*
		 ajDebug("ensembltest_genes "
			 "Exon stable identifier: '%S'\n",
			 ensExonGetStableIdentifier(exon));
		 */
		
		/* Fetch and write the sequence of this Exon. */
		
		ensExonFetchSequenceSeq(exon, &seq);
		
		ajSeqoutWriteSeq(exnoutseq, seq);
		
		ajSeqDel(&seq);
		
		ensExonDel(&exon);
	    }
	    
	    ajListFree(&exlist);
	}
	
	ensGeneDel(&gene);
	
	i++;
    }
    
    ajListFree(&gnlist);
    
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
** @param [r] dba [EnsPDatabaseadaptor] Ensembl Database Adaptor.
** @param [r] outfile [AjPFile] AJAX File.
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise.
** @@
******************************************************************************/

static AjBool ensembltest_markers(EnsPDatabaseadaptor dba,  AjPFile outfile)
{
    AjPList mflist = NULL;
    
    AjPStr csname    = NULL;
    AjPStr csversion = NULL;
    AjPStr srname    = NULL;
    AjPStr slname    = NULL;
    
    EnsPAnalysis analysis = NULL;
    
    EnsPFeature feature = NULL;
    
    EnsPMarkerfeature mf         = NULL;
    EnsPMarkerfeatureadaptor mfa = NULL;
    
    EnsPMarker marker = NULL;
    
    EnsPMarkersynonym ms = NULL;
    
    EnsPSlice slice     = NULL;
    EnsPSliceadaptor sa = NULL;
    
    if(!dba)
	return ajFalse;
    
    if(!outfile)
	return ajFalse;
    
    ajFmtPrintF(outfile, "\n");
    ajFmtPrintF(outfile, "Ensembl Marker Features\n");
    ajFmtPrintF(outfile, "\n");
    
    sa = ensRegistryGetSliceadaptor(dba);
    
    /*
    ** Fetch an Ensembl Slice for chromosome:NCBI36:18:43000000:44000000:1,
    ** which is around SMAD2.
    */
    
    csname = ajStrNewC("chromosome");
    
    csversion = ajStrNewC("NCBI36");
    
    srname = ajStrNewC("18");
    
    ensSliceadaptorFetchByRegion(sa,
				 csname,
				 csversion,
				 srname,
				 43000000,
				 44000000,
				 1,
				 &slice);
    
    ajStrDel(&csname);
    ajStrDel(&csversion);
    ajStrDel(&srname);
    
    /* Fetch all Marker Features on this Slice. */
    
    mfa = ensRegistryGetMarkerfeatureadaptor(dba);
    
    mflist = ajListNew();
    
    ensMarkerfeatureadaptorFetchAllBySlice(mfa,
					   slice,
					   0,
					   0,
					   (AjPStr) NULL,
					   mflist);
    
    ensSliceFetchName(slice, &slname);
    
    ajFmtPrintF(outfile,
		"  Ensembl Slice '%S' %u Marker Features\n\n",
		slname,
		ajListGetLength(mflist));
    
    while(ajListPop(mflist, (void **) &mf))
    {
	feature = ensMarkerfeatureGetFeature(mf);
	
	analysis = ensFeatureGetAnalysis(feature);
	
	marker = ensMarkerfeatureGetMarker(mf);
	
	ms = ensMarkerGetDisplaySynonym(marker);
	
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
		    "    Slice coordinates '%d:%d:%d'\n",
		    ensFeatureGetStart(feature),
		    ensFeatureGetEnd(feature),
		    ensFeatureGetStrand(feature));
	
	ajFmtPrintF(outfile,
		    "    Sequence Region coordinates '%d:%d:%d'\n\n",
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
** @param [r] dba [EnsPDatabaseadaptor] Ensembl Database Adaptor.
** @param [r] outfile [AjPFile] AJAX File.
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise.
** @@
******************************************************************************/

static AjBool ensembltest_ditags(EnsPDatabaseadaptor dba,  AjPFile outfile)
{
    AjPList dtfs = NULL;
    
    AjPStr csname    = NULL;
    AjPStr csversion = NULL;
    AjPStr srname    = NULL;
    AjPStr slname    = NULL;
    
    EnsPAnalysis analysis = NULL;
    
    EnsPFeature feature = NULL;
    
    EnsPDitag dt = NULL;
    
    EnsPDitagfeature dtf         = NULL;
    EnsPDitagfeatureadaptor dtfa = NULL;
    
    EnsPSlice slice     = NULL;
    EnsPSliceadaptor sa = NULL;
    
    if(!dba)
	return ajFalse;
    
    if(!outfile)
	return ajFalse;
    
    ajFmtPrintF(outfile, "\n");
    ajFmtPrintF(outfile, "Ensembl Di-Tag Features\n");
    ajFmtPrintF(outfile, "\n");
    
    sa = ensRegistryGetSliceadaptor(dba);
    
    /*
    ** Fetch an Ensembl Slice for chromosome:NCBI36:18:43000000:44000000:1,
    ** which is around SMAD2.
    */
    
    csname = ajStrNewC("chromosome");
    
    csversion = ajStrNewC("NCBI36");
    
    srname = ajStrNewC("18");
    
    ensSliceadaptorFetchByRegion(sa,
				 csname,
				 csversion,
				 srname,
				 43000000,
				 44000000,
				 1,
				 &slice);
    
    ajStrDel(&csname);
    ajStrDel(&csversion);
    ajStrDel(&srname);
    
    /* Fetch all Marker Features on this Slice. */
    
    dtfa = ensRegistryGetDitagfeatureadaptor(dba);
    
    dtfs = ajListNew();
    
    ensDitagfeatureadaptorFetchAllBySlice(dtfa,
					  slice,
					  (AjPStr) NULL,
					  (AjPStr) NULL,
					  dtfs);
    
    ensSliceFetchName(slice, &slname);
    
    ajFmtPrintF(outfile,
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
		    "    Slice coordinates '%d:%d:%d'\n",
		    ensFeatureGetStart(feature),
		    ensFeatureGetEnd(feature),
		    ensFeatureGetStrand(feature));
	
	ajFmtPrintF(outfile,
		    "    Sequence Region coordinates '%d:%d:%d'\n\n",
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
** @param [r] dba [EnsPDatabaseadaptor] Ensembl Database Adaptor.
** @param [r] outseq [AjPSeqout] AJAX Sequence Output.
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise.
** @@
******************************************************************************/

static AjBool ensembltest_masking(EnsPDatabaseadaptor dba, AjPSeqout outseq)
{
    AjEnum mtype = ensERepeatMaskTypeSoft;
    
    AjPList names = NULL;
    
    AjPSeq seq = NULL;
    
    AjPStr anname    = NULL;
    AjPStr csname    = NULL;
    AjPStr csversion = NULL;
    AjPStr srname    = NULL;
    
    AjPTable masking = NULL;
    
    EnsPSlice slice     = NULL;
    EnsPSliceadaptor sa = NULL;
    
    EnsPRepeatmaskedslice rmslice = NULL;
    
    if(!dba)
	return ajFalse;
    
    sa = ensRegistryGetSliceadaptor(dba);
    
    /* Fetch an Ensembl Slice for chromosome:NCBI36:21:13200000:13600000:1. */
    
    csname = ajStrNewC("chromosome");
    
    csversion = ajStrNewC("NCBI36");
    
    srname = ajStrNewC("21");
    
    ensSliceadaptorFetchByRegion(sa,
				 csname,
				 csversion,
				 srname,
				 13200001,
				 13600000,
				 1,
				 &slice);
    
    ajStrDel(&csname);
    ajStrDel(&csversion);
    ajStrDel(&srname);
    
    /*
    ** Fetch the (unmasked) genome sequence for Ensembl Slice
    ** chromosome:NCBI36:21:13200001:13600000:1.
    */
    
    ensSliceFetchSequenceSeq(slice, &seq);
    
    ajSeqAssignDescC(seq, "Ensembl Slice");
    
    ajSeqoutWriteSeq(outseq, seq);
    
    ajSeqDel(&seq);
    
    /*
    ** Fetch the masked genome sequence for Ensembl Slice
    ** chromosome:NCBI36:21:13200001:13600000:1.
    */
    
    names = ajListNew(); /* FIXME: This should be an ajListstrNew() */
    
    anname = ajStrNewC("RepeatMask");
    
    ajListPushAppend(names, (void *) anname);
    
    masking = ajTablestrNewLen(0);
    
    rmslice = ensRepeatmaskedsliceNew(slice, names, masking);
    
    ensRepeatmaskedsliceFetchSequenceSeq(rmslice, mtype, &seq);
    
    ajSeqAssignDescC(seq, "Ensembl Repeat Masked Slice");
    
    ajSeqoutWriteSeq(outseq, seq);
    
    ajSeqDel(&seq);
    
    ensRepeatmaskedsliceDel(&rmslice);
    
    while(ajListPop(names, (void **) &anname))
	ajStrDel(&anname);
    
    ajListFree(&names);
    
    ajTableFree(&masking);
    
    ensSliceDel(&slice);
    
    return ajTrue;
}




/* @funcstatic ensembltest_chromosome *****************************************
**
** Fetch a Slice for chromosome:NCBI36:21:0:0:0 and write its sequence to disk.
**
** @param [r] dba [EnsPDatabaseadaptor] Ensembl Database Adaptor.
** @param [r] outseq [AjPSeqout] AJAX Sequence Output.
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise.
** @@
******************************************************************************/

static AjBool ensembltest_chromosome(EnsPDatabaseadaptor dba, AjPSeqout outseq)
{
    AjPSeq seq = NULL;
    
    AjPStr csname    = NULL;
    AjPStr csversion = NULL;
    AjPStr srname    = NULL;
    AjPStr seqstr    = NULL;
    AjPStr tmpstr    = NULL;
    
    EnsPSlice slice     = NULL;
    EnsPSliceadaptor sa = NULL;
    
    if(!dba)
	return ajFalse;
    
    sa = ensRegistryGetSliceadaptor(dba);
    
    /* Fetch a Slice for human chromosome:NCBI36:21:0:0:0 */
    
    csname = ajStrNewC("chromosome");
    
    csversion = ajStrNewC("NCBI36");
    
    srname = ajStrNewC("21");
    
    ensSliceadaptorFetchByRegion(sa,
				 csname,
				 csversion,
				 srname,
				 0,
				 0,
				 0,
				 &slice);
    
    ajStrDel(&csname);
    ajStrDel(&csversion);
    ajStrDel(&srname);
    
    tmpstr = ajStrNew();
    
    ensSliceFetchName(slice, &tmpstr);
    
    seqstr = ajStrNew();
    
    ensSliceFetchSequenceStr(slice, &seqstr);
    
    seq = ajSeqNewNameS(seqstr, tmpstr);
    
    ajStrDel(&seqstr);
    ajStrDel(&tmpstr);
    
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
     
     ajStrAssignC(&csname1, "chromosome");
     
     ajStrAssignC(&csversion1, "BROADO5");
     
     ajStrAssignC(&srname, "1");
     */
    
    return ajTrue;
}




/* @funcstatic ensembltest_meta ***********************************************
**
** Ensembl Meta-Information tests.
**
** @param [r] dba [EnsPDatabaseadaptor] Ensembl Database Adaptor.
** @param [r] outfile [AjPFile] AJAX File.
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise.
** @@
******************************************************************************/

static AjBool ensembltest_meta(EnsPDatabaseadaptor dba, AjPFile outfile)
{
    AjPStr value = NULL;
    
    EnsPMetainformationadaptor mia = NULL;
    
    if(!dba)
	return ajFalse;
    
    if(!outfile)
	return ajFalse;
    
    ajFmtPrintF(outfile, "\n");
    ajFmtPrintF(outfile, "Ensembl Meta-Information\n");
    ajFmtPrintF(outfile, "\n");
    
    mia = ensRegistryGetMetainformationadaptor(dba);
    
    value = ajStrNew();
    
    if(ensMetainformationadaptorGetGenebuildVersion(mia, &value))
	ajFmtPrintF(outfile,
		    "  Ensembl Meta-Information Genebuild Version '%S'\n",
		    value);
    
    if(ensMetainformationadaptorGetSchemaVersion(mia, &value))
	ajFmtPrintF(outfile,
		    "  Ensembl Meta-Information Schema Version '%S'\n",
		    value);
    
    if(ensMetainformationadaptorGetTaxonomyIdentifier(mia, &value))
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
** @param [r] dba [EnsPDatabaseadaptor] Ensembl Database Adaptor.
** @param [r] outfile [AjPFile] AJAX File.
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
    
    ajFmtPrintF(outfile, "\n");
    ajFmtPrintF(outfile, "Ensembl Coordinate Systems\n");
    
    csa = ensRegistryGetCoordsystemadaptor(dba);
    
    /* Fetch the top-level Ensembl Coordinate System. */
    
    ajFmtPrintF(outfile, "\n");
    ajFmtPrintF(outfile, "  Top-Level Coordinate System\n");
    
    ensCoordsystemadaptorFetchTopLevel(csa, &cs);
    
    ajFmtPrintF(outfile,
		"    Ensembl Coordinate System %u rank %u '%S:%S'\n",
		ensCoordsystemGetIdentifier(cs),
		ensCoordsystemGetRank(cs),
		ensCoordsystemGetName(cs),
		ensCoordsystemGetVersion(cs));
    
    ensCoordsystemDel(&cs);
    
    /* Fetch the sequence-level Ensembl Coordinate System. */
    
    ajFmtPrintF(outfile, "\n");
    ajFmtPrintF(outfile, "  Sequence-Level Coordinate System\n");
    
    ensCoordsystemadaptorFetchSeqLevel(csa, &cs);
    
    ajFmtPrintF(outfile,
		"    Ensembl Coordinate System %u rank %u '%S:%S'\n",
		ensCoordsystemGetIdentifier(cs),
		ensCoordsystemGetRank(cs),
		ensCoordsystemGetName(cs),
		ensCoordsystemGetVersion(cs));
    
    ensCoordsystemDel(&cs);
    
    /* Fetch all Ensembl Coordinate Systems with name 'chromosome'. */
    
    ajFmtPrintF(outfile, "\n");
    ajFmtPrintF(outfile, "  Coordinate Systems with name 'chromosome'\n");
    
    csname1 = ajStrNewC("chromosome");
    
    cslist = ajListNew();
    
    ensCoordsystemadaptorFetchAllByName(csa, csname1, cslist);
    
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
    
    ajFmtPrintF(outfile, "\n");
    ajFmtPrintF(outfile,
		"  Coordinate Systems with name 'chromosome' "
		"and version 'NCBI35'\n");
    
    csname1 = ajStrNewC("chromosome");
    
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
    
    ajFmtPrintF(outfile, "\n");
    ajFmtPrintF(outfile,
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
    
    ajFmtPrintF(outfile, "\n");
    ajFmtPrintF(outfile,
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
    
    ajFmtPrintF(outfile, "\n");
    
    ajFmtPrintF(outfile,
		"  Ensembl Coordinate System Mapping path "
		"'%S:%S' - '%S:%S'\n",
		ensCoordsystemGetName(cs1),
		ensCoordsystemGetVersion(cs1),
		ensCoordsystemGetName(cs2),
		ensCoordsystemGetVersion(cs2));
    
    mplist = ensCoordsystemadaptorGetMappingPath(csa, cs1, cs2);
    
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
    
    ajFmtPrintF(outfile, "\n");
    ajFmtPrintF(outfile,
		"  Ensembl Coordinate System Mapping path "
		"'%S:%S' - '%S:%S'\n",
		ensCoordsystemGetName(cs1),
		ensCoordsystemGetVersion(cs1),
		ensCoordsystemGetName(cs3),
		ensCoordsystemGetVersion(cs3));
    
    mplist = ensCoordsystemadaptorGetMappingPath(csa, cs1, cs3);
    
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
    
    ajFmtPrintF(outfile, "\n");
    
    ajFmtPrintF(outfile,
		"  Ensembl Coordinate System Mapping path "
		"'%S:%S' - '%S:%S'\n",
		ensCoordsystemGetName(cs2),
		ensCoordsystemGetVersion(cs2),
		ensCoordsystemGetName(cs3),
		ensCoordsystemGetVersion(cs3));
    
    mplist = ensCoordsystemadaptorGetMappingPath(csa, cs2, cs3);
    
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
** @param [r] dba [EnsPDatabaseadaptor] Ensembl Database Adaptor.
** @param [r] outfile [AjPFile] AJAX File.
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
    
    ajFmtPrintF(outfile, "\n");
    
    ajFmtPrintF(outfile, "Ensembl Sequence Regions\n");
    
    sra = ensRegistryGetSeqregionadaptor(dba);
    
    /* Fetch the human chromosome 1 via its identifier 226034. */
    /* 223781 AC120349.5 */
    
    ensSeqregionadaptorFetchByIdentifier(sra, 226034, &sr);
    
    ajFmtPrintF(outfile, "\n");
    
    cs = ensSeqregionGetCoordsystem(sr);
    
    ajFmtPrintF(outfile,
		"  Ensembl Sequence Region %u name '%S' length %u '%S:%S'\n",
		ensSeqregionGetIdentifier(sr),
		ensSeqregionGetName(sr),
		ensSeqregionGetLength(sr),
		ensCoordsystemGetName(cs),
		ensCoordsystemGetVersion(cs));
    
    list = ensSeqregionGetAttributes(sr);
    
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
** @param [r] dba [EnsPDatabaseadaptor] Ensembl Database Adaptor.
** @param [r] outfile [AjPFile] AJAX File.
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise.
** @@
******************************************************************************/

static AjBool ensembltest_transformations(EnsPDatabaseadaptor dba,
                                          AjPFile outfile)
{
    ajuint version = 0;
    
    AjPStr name     = NULL;
    AjPStr stableid = NULL;
    
    EnsPFeature feature = NULL;
    
    EnsPGene oldgene   = NULL;
    EnsPGene newgene   = NULL;
    EnsPGeneadaptor ga = NULL;
    
    EnsPSlice slice     = NULL;
    EnsPSliceadaptor sa = NULL;
    
    if(!dba)
	return ajFalse;
    
    if(!outfile)
	return ajFalse;
    
    /* Ensembl Feature transfer and transform tests. */
    
    ajFmtPrintF(outfile, "\n");
    ajFmtPrintF(outfile, "Ensembl Feature transfer and transform\n");
    
    ajDebug("ensembltest_transformations\n");
    
    ga = ensRegistryGetGeneadaptor(dba);
    
    stableid = ajStrNewC("ENSG00000175387");
    
    ensGeneadaptorFetchByStableIdentifier(ga, stableid, version, &oldgene);
    
    ensGeneGetTranscripts(oldgene);
    
    ensGeneTrace(oldgene, 1);
    
    feature = ensGeneGetFeature(oldgene);
    
    ajFmtPrintF(outfile, "\n");
    ajFmtPrintF(outfile,
		"  Ensembl Gene %S\n"
		"    Ensembl Feature %S:%d:%d:%d\n",
		ensGeneGetStableIdentifier(oldgene),
		ensFeatureGetSeqregionName(feature),
		ensFeatureGetStart(feature),
		ensFeatureGetEnd(feature),
		ensFeatureGetStrand(feature));
    
    sa = ensRegistryGetSliceadaptor(dba);
    
    name = ajStrNewC("chromosome:NCBI36:18:40000000:50000000:1");
    
    ensSliceadaptorFetchByName(sa, name, &slice);
    
    ensSliceTrace(slice, 1);
    
    newgene = ensGeneTransfer(oldgene, slice);
    
    ensGeneTrace(newgene, 1);
    
    feature = ensGeneGetFeature(newgene);
    
    ajFmtPrintF(outfile, "\n");
    ajFmtPrintF(outfile,
		"  Ensembl Gene %S\n"
		"    Ensembl Feature %S:%d:%d:%d\n",
		ensGeneGetStableIdentifier(newgene),
		ensFeatureGetSeqregionName(feature),
		ensFeatureGetStart(feature),
		ensFeatureGetEnd(feature),
		ensFeatureGetStrand(feature));
    
    ensSliceDel(&slice);
    
    ensGeneDel(&newgene);
    ensGeneDel(&oldgene);
    
    ajStrDel(&name);
    ajStrDel(&stableid);
    
    return ajTrue;
}




/* @funcstatic ensembltest_registry *******************************************
**
** Ensembl Registry tests.
**
** @param [r] outfile [AjPFile] AJAX File.
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
    
    AjEnum group = ensEDatabaseadaptorGroupNULL;
    
    AjPStr identifier = NULL;
    AjPStr species    = NULL;
    
    if(!outfile)
	return ajFalse;
    
    /* Ensembl Registry tests. */
    
    ajFmtPrintF(outfile, "\n");
    ajFmtPrintF(outfile, "Ensembl Registry\n");
    ajFmtPrintF(outfile, "\n");
    
    /* Registry test of stable identifier expressions. */
    
    ensRegistryLoadIdentifiers();
    
    for(i = 0; identifiers[i]; i++) {
	
	identifier = ajStrNewC(identifiers[i]);
	
	species = ajStrNew();
	
	ensRegistryGetSpeciesGroup(identifier, &species, &group);
	
	ajFmtPrintF(outfile,
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
** @param [r] dba [EnsPDatabaseadaptor] Ensembl Database Adaptor.
** @param [r] outfile [AjPFile] AJAX File.
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise.
** @@
******************************************************************************/

static AjBool ensembltest_density(EnsPDatabaseadaptor dba, AjPFile outfile)
{
    float maxratio = 0;
    
    ajint srstart  = 0;
    ajint srend    = 0;
    ajint srstrand = 0;
    
    ajuint blocks = 100;
    
    AjBool interpolate = AJFALSE;
    
    AjPList dfs = NULL;
    
    AjPStr anname    = NULL;
    AjPStr csname    = NULL;
    AjPStr csversion = NULL;
    AjPStr srname    = NULL;
    AjPStr slname    = NULL;
    
    EnsPAnalysis analysis = NULL;
    
    EnsPDensityfeature df         = NULL;
    EnsPDensityfeatureadaptor dfa = NULL;
    
    EnsPDensitytype dt = NULL;
    
    EnsPFeature feature = NULL;
    
    EnsPSlice slice     = NULL;
    EnsPSliceadaptor sa = NULL;
    
    if(!outfile)
	return ajFalse;
    
    /* Ensembl Registry tests. */
    
    ajFmtPrintF(outfile, "\n");
    ajFmtPrintF(outfile, "Ensembl Density\n");
    ajFmtPrintF(outfile, "\n");
    
    /* Registry test of stable identifier expressions. */
    
    dfa = ensRegistryGetDensityfeatureadaptor(dba);
    
    sa = ensRegistryGetSliceadaptor(dba);
    
    csname = ajStrNewC("toplevel");
    srname = ajStrNewC("22");
    
    ensSliceadaptorFetchByRegion(sa,
				 csname,
				 csversion,
				 srname,
				 srstart,
				 srend,
				 srstrand,
				 &slice);
    
    slname = ajStrNew();
    
    ensSliceFetchName(slice, &slname);
    
    ajFmtPrintF(outfile, "Slice %S\n", slname);
    ajFmtPrintF(outfile, "\n");
    
    ajStrDel(&slname);
    
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
    
    ensDensityfeatureadaptorFetchAllBySlice(dfa,
					    slice,
					    anname,
					    blocks,
					    interpolate,
					    maxratio,
					    dfs);
    
    while(ajListPop(dfs, (void **) &df))
    {
	feature = ensDensityfeatureGetFeature(df);
	
	dt = ensDensityfeatureGetDensitytype(df);
	
	analysis = ensDensitytypeGetAnalysis(dt);
	
	ajFmtPrintF(outfile,
		    "  %u '%S' %u %S:%d:%d:%d value %f\n",
		    ensDensityfeatureGetIdentifier(df),
		    ensAnalysisGetName(analysis),
		    ensDensitytypeGetBlockSize(dt),
		    ensFeatureGetSeqregionName(feature),
		    ensFeatureGetSeqregionStart(feature),
		    ensFeatureGetSeqregionEnd(feature),
		    ensFeatureGetSeqregionStrand(feature),
		    ensDensityfeatureGetDensityValue(df));
	
	ensDensityfeatureDel(&df);
    }
    
    ajListFree(&dfs);
    
    ensSliceDel(&slice);

    ajStrDel(&anname);
    ajStrDel(&csname);
    ajStrDel(&srname);
    
    return ajTrue;
}
