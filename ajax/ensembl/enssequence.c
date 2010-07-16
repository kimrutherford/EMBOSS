/******************************************************************************
** @source Ensembl Sequence functions
**
** @author Copyright (C) 1999 Ensembl Developers
** @author Copyright (C) 2006 Michael K. Schuster
** @modified 2009 by Alan Bleasby for incorporation into EMBOSS core
** @version $Revision: 1.12 $
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

#include "enssequence.h"
#include "enssequenceedit.h"




/* ==================================================================== */
/* ========================== private data ============================ */
/* ==================================================================== */




/* ==================================================================== */
/* ======================== private functions ========================= */
/* ==================================================================== */

static void* sequenceadaptorCacheReference(void *value);

static void sequenceadaptorCacheDelete(void **value);

static ajulong sequenceadaptorCacheSize(const void *value);




/* @filesection enssequence ***************************************************
**
** @nam1rule ens Function belongs to the Ensembl library
**
******************************************************************************/




/* @datasection [EnsPSequenceadaptor] Sequence Adaptor ************************
**
** Functions for manipulating Ensembl Sequence Adaptor objects
**
** @cc Bio::EnsEMBL::DBSQL::Sequenceadaptor CVS Revision: 1.36
**
** @nam2rule Sequenceadaptor
**
******************************************************************************/

/* 1 << 18 = 256 KiB, about the size of a BAC clone */

static ajuint sequenceChunkPower = 18;

/* 1 << 26 =  64 MiB */

static ajuint sequenceadaptorCacheMaxBytes = 1 << 26;

/* 1 << 16 =  64 Ki  */

static ajuint sequenceadaptorCacheMaxCount = 1 << 16;

static ajuint sequenceadaptorCacheMaxSize = 0;

/* 1 << 18 = 256 Ki * 5, or about 5 BAC clones */

static ajuint sequenceadaptorCacheMaximum = (1 << 18) * 5;




/* @funcstatic sequenceadaptorCacheReference **********************************
**
** Wrapper function to reference an Ensembl Sequence (i.e. an AJAX String)
** from an Ensembl Cache.
**
** @param [u] value [void*] AJAX String
**
** @return [void*] AJAX String or NULL
** @@
******************************************************************************/

static void* sequenceadaptorCacheReference(void *value)
{
    if(!value)
        return NULL;

    return (void *) ajStrNewRef((AjPStr) value);
}




/* @funcstatic sequenceadaptorCacheDelete *************************************
**
** Wrapper function to delete (or de-reference) an Ensembl Sequence
** (i.e. an AJAX String) from an Ensembl Cache.
**
** @param [u] value [void**] AJAX String address
**
** @return [void]
** @@
******************************************************************************/

static void sequenceadaptorCacheDelete(void **value)
{
    if(!value)
        return;

    ajStrDel((AjPStr *) value);

    *value = NULL;

    return;
}




/* @funcstatic sequenceadaptorCacheSize ***************************************
**
** Wrapper function to calculate the memory size of an Ensembl Sequence
** (i.e. an AJAX String) from an Ensembl Cache.
**
** @param [r] value [const void*] AJAX String
**
** @return [ajulong] Memory size
** @@
******************************************************************************/

static ajulong sequenceadaptorCacheSize(const void *value)
{
    ajulong size = 0;

    if(!value)
        return 0;

    size += sizeof (AjOStr);

    size += ajStrGetRes((const AjPStr) value);

    return size;
}




/* @section constructors ******************************************************
**
** All constructors return a new Ensembl Sequence Adaptor by pointer.
** It is the responsibility of the user to first destroy any previous
** Sequence Adaptor. The target pointer does not need to be initialised to
** NULL, but it is good programming practice to do so anyway.
**
** @fdata [EnsPSequenceadaptor]
** @fnote None
**
** @nam3rule New Constructor
**
** @argrule New dba [EnsPDatabaseadaptor] Ensembl Database Adaptor
** @argrule Obj object [EnsPSequenceadaptor] Ensembl Sequence Adaptor
** @argrule Ref object [EnsPSequenceadaptor] Ensembl Sequence Adaptor
**
** @valrule * [EnsPSequenceadaptor] Ensembl Sequence Adaptor
**
** @fcategory new
******************************************************************************/




/* @func ensSequenceadaptorNew ************************************************
**
** Default constructor for an Ensembl Sequence Adaptor.
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
** @see ensRegistryGetSequenceadaptor
**
** @cc Bio::EnsEMBL::DBSQL::Sequenceadaptor::new
** @param [r] dba [EnsPDatabaseadaptor] Ensembl Database Adaptor
**
** @return [EnsPSequenceadaptor] Ensembl Sequence Adaptor or NULL
** @@
******************************************************************************/

EnsPSequenceadaptor ensSequenceadaptorNew(
    EnsPDatabaseadaptor dba)
{
    EnsPSequenceadaptor sa = NULL;

    if(!dba)
        return NULL;

    if(ajDebugTest("ensSequenceadaptorNew"))
        ajDebug("ensSequenceadaptorNew\n"
                "  dba %p\n",
                dba);

    AJNEW0(sa);

    sa->Adaptor = dba;

    sa->Cache = ensCacheNew(ensECacheTypeAlphaNumeric,
                            sequenceadaptorCacheMaxBytes,
                            sequenceadaptorCacheMaxCount,
                            sequenceadaptorCacheMaxSize,
                            sequenceadaptorCacheReference,
                            sequenceadaptorCacheDelete,
                            sequenceadaptorCacheSize,
                            (void* (*)(const void* key)) NULL,
                            (AjBool (*)(const void* value)) NULL,
                            ajFalse,
                            "Sequence");

    return sa;
}




/* @section destructors *******************************************************
**
** Destruction destroys all internal data structures and frees the
** memory allocated for the Ensembl Sequence Adaptor.
**
** @fdata [EnsPSequenceadaptor]
** @fnote None
**
** @nam3rule Del Destroy (free) a Sequence Adaptor object
**
** @argrule * Psa [EnsPSequenceadaptor*] Sequence Adaptor object address
**
** @valrule * [void]
**
** @fcategory delete
******************************************************************************/




/* @func ensSequenceadaptorDel ************************************************
**
** Default destructor for an Ensembl Sequence Adaptor.
**
** Ensembl Object Adaptors are singleton objects that are registered in the
** Ensembl Registry and weakly referenced by Ensembl Objects that have been
** instantiated by it. Therefore, Ensembl Object Adaptors should never be
** destroyed directly. Upon exit, the Ensembl Registry will call this function
** if required.
**
** @param [d] Psa [EnsPSequenceadaptor*] Ensembl Sequence Adaptor address
**
** @return [void]
** @@
******************************************************************************/

void ensSequenceadaptorDel(EnsPSequenceadaptor *Psa)
{
    EnsPSequenceadaptor pthis = NULL;

    if(!Psa)
        return;

    if(!*Psa)
        return;

    if(ajDebugTest("ensSequenceadaptorDel"))
        ajDebug("ensSequenceadaptorDel\n"
                "  *Psa %p\n",
                *Psa);

    pthis = *Psa;

    if(pthis->Cache)
        ensCacheDel(&pthis->Cache);

    AJFREE(pthis);

    *Psa = NULL;

    return;
}




/* @section object fetching ***************************************************
**
** Functions for retrieving Sequence objects from an Ensembl Core database.
**
** @fdata [EnsPSequenceadaptor]
** @fnote None
**
** @nam3rule Fetch Retrieve Sequence object(s)
** @nam4rule FetchStr Fetch a Sequence as AJAX String
** @nam4rule FetchSeq Fetch a Sequence as AJAX Sequence
** @nam5rule BySeqregion Fetch Sequence via an Ensembl Sequence Region
** @nam5rule BySlice Fetch Sequence via an Ensembl Slice
**
** @argrule * sa [const EnsPSequenceadaptor] Ensembl Sequence Adaptor
** @argrule BySeqregion sr [EnsPSeqregion] Ensembl Sequence Region
** @argrule BySlice slice [EnsPSlice] Ensembl Slice
**
** @valrule FetchStr Psequence [AjPStr*] AJAX String address
** @valrule FetchSeq Psequence [AjPSeq*] AJAX Sequence address
**
** @fcategory use
******************************************************************************/




/* @func ensSequenceadaptorFetchSubStrBySeqregion *****************************
**
** Fetch a sub-sequence of an Ensembl Sequence Region as an AJAX String.
** The start coodinate is one-based, as in the SQL SUBSTRING function.
** A start of 1 and a length equal the Sequence Region length covers the whole
** Sequence Region.
**
** The caller is responsible for deleting the AJAX String.
**
** @cc Bio::EnsEMBL::DBSQL::Sequenceadaptor::_fetch_seq
** @see ensSequenceadaptorFetchSubStrBySlice
** @param [r] sa [const EnsPSequenceadaptor] Ensembl Sequence Adaptor
** @param [r] sr [EnsPSeqregion] Ensembl Sequence Region
** @param [r] start [ajuint] Start coordinate
** @param [r] length [ajuint] Sequence length
** @param [u] Psequence [AjPStr*] Sequence address
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
** @@
** This function will only return biological sequence information for Ensembl
** Sequence Regions, which are in the sequence-level Coordinate System. All
** other Sequence Regions do not have sequence attached so that their sequence
** can only be fetched in the context of an Ensembl Slice, which is
** subsequently mapped to the sequence-level Coordinate System. See the
** description of the ensSequenceadaptorFetchSubStrBySlice function for
** further details.
******************************************************************************/

AjBool ensSequenceadaptorFetchSubStrBySeqregion(const EnsPSequenceadaptor sa,
                                                EnsPSeqregion sr,
                                                ajuint start,
                                                ajuint length,
                                                AjPStr *Psequence)
{
    register ajuint i = 0;

    ajuint chkmin = 0;
    ajuint chkmax = 0;
    ajuint posmin = 0;

    AjPSqlstatement sqls = NULL;
    AjISqlrow sqli       = NULL;
    AjPSqlrow sqlr       = NULL;

    AjPStr chkstr    = NULL;
    AjPStr tmpstr    = NULL;
    AjPStr key       = NULL;
    AjPStr statement = NULL;

    if(ajDebugTest("ensSequenceadaptorFetchSubStrBySeqregion"))
    {
        ajDebug("ensSequenceadaptorFetchSubStrBySeqregion\n"
                "  sa %p\n"
                "  sr %p\n"
                "  start %d\n"
                "  length %d\n",
                sa,
                sr,
                start,
                length);

        ensSeqregionTrace(sr, 1);
    }

    if(!sa)
        return ajFalse;

    if(!sr)
        return ajFalse;

    if(!Psequence)
        return ajFalse;

    /*
    ** Allocate an AJAX String and reserve space in blocks of 256 KiB
    ** (1 << 18) based on the requested length plus one position for the
    ** 'nul' string terminator.
    */

    if(*Psequence)
        ajStrAssignClear(Psequence);
    else
        *Psequence = ajStrNewRes((((length + 1) >> sequenceChunkPower) + 1)
                                 << sequenceChunkPower);

    if(length < sequenceadaptorCacheMaximum)
    {
        chkmin = (start - 1) >> sequenceChunkPower;

        chkmax = (start + length - 1) >> sequenceChunkPower;

        /*
        ** Allocate an AJAX String and reserve space for the number of
        ** sequence chunks plus one for the 'nul' string terminator.
        */

        tmpstr = ajStrNewRes(((1 << sequenceChunkPower) + 1) *
                             (chkmax - chkmin + 1));

        /* Piece together sequence from cached sequence chunks. */

        for(i = chkmin; i <= chkmax; i++)
        {
            key = ajFmtStr("%u:%u", ensSeqregionGetIdentifier(sr), i);

            chkstr = (AjPStr) ensCacheFetch(sa->Cache, (void *) key);

            if(chkstr)
            {
                ajStrAppendS(&tmpstr, chkstr);

                ajStrDel(&chkstr);
            }
            else
            {
                /* Retrieve uncached chunks of the sequence. */

                posmin = (i << sequenceChunkPower) + 1;

                statement = ajFmtStr(
                    "SELECT "
                    "SUBSTRING(dna.sequence FROM %u FOR %u) "
                    "FROM "
                    "dna "
                    "WHERE "
                    "dna.seq_region_id = %u",
                    posmin,
                    1 << sequenceChunkPower,
                    ensSeqregionGetIdentifier(sr));

                sqls = ensDatabaseadaptorSqlstatementNew(sa->Adaptor,
                                                         statement);

                sqli = ajSqlrowiterNew(sqls);

                while(!ajSqlrowiterDone(sqli))
                {
                    sqlr = ajSqlrowiterGet(sqli);

                    /*
                    ** Allocate an AJAX String and reserve space for the
                    ** maximum sequence chunk length plus the 'nul' string
                    ** terminator.
                    */

                    chkstr = ajStrNewRes((1 << sequenceChunkPower) + 1);

                    ajSqlcolumnToStr(sqlr, &chkstr);

                    /*
                    ** Always store upper case sequence
                    ** so that it can be properly soft-masked.
                    */

                    ajStrFmtUpper(&chkstr);

                    ensCacheStore(sa->Cache, (void *) key, (void **) &chkstr);

                    ajStrAppendS(&tmpstr, chkstr);

                    ajStrDel(&chkstr);
                }

                ajSqlrowiterDel(&sqli);

                ensDatabaseadaptorSqlstatementDel(sa->Adaptor, &sqls);

                ajStrDel(&statement);
            }

            ajStrDel(&key);
        }

        /* Return only the requested portion of the entire sequence. */

        posmin = (chkmin << sequenceChunkPower) + 1;

        ajStrAssignSubS(Psequence,
                        tmpstr,
                        start - posmin,
                        start - posmin + length - 1);

        ajStrDel(&tmpstr);
    }
    else
    {
        /* Do not attempt any caching for requests of very large sequences. */

        statement = ajFmtStr(
            "SELECT "
            "SUBSTRING(dna.sequence FROM %u FOR %u) "
            "FROM dna "
            "WHERE "
            "dna.seq_region_id = %u",
            start,
            length,
            ensSeqregionGetIdentifier(sr));

        sqls = ensDatabaseadaptorSqlstatementNew(sa->Adaptor, statement);

        sqli = ajSqlrowiterNew(sqls);

        while(!ajSqlrowiterDone(sqli))
        {
            sqlr = ajSqlrowiterGet(sqli);

            /*
            ** Allocate an AJAX String and reserve space of the length plus
            ** the 'nul' string terminator.
            */

            chkstr = ajStrNewRes((length + 1));

            ajSqlcolumnToStr(sqlr, &chkstr);

            /*
            ** Always return upper case sequence
            ** so that it can be properly soft-masked.
            */

            ajStrFmtUpper(&chkstr);

            ajStrAssignS(Psequence, chkstr);

            ajStrDel(&chkstr);
        }

        ajSqlrowiterDel(&sqli);

        ensDatabaseadaptorSqlstatementDel(sa->Adaptor, &sqls);

        ajStrDel(&statement);
    }

    return ajTrue;
}




/* @func ensSequenceadaptorFetchStrBySeqregion ********************************
**
** Fetch the sequence of an Ensembl Sequence Region as an AJAX String.
**
** The caller is responsible for deleting the AJAX String.
**
** @cc Bio::EnsEMBL::DBSQL::Sequenceadaptor::_fetch_seq
** @see ensSequenceadaptorFetchStrBySlice
** @param [r] sa [const EnsPSequenceadaptor] Ensembl Sequence Adaptor
** @param [r] sr [EnsPSeqregion] Ensembl Sequence Region
** @param [u] Psequence [AjPStr*] Sequence address
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
** @@
** This function will only return biological sequence information for Ensembl
** Sequence Regions, which are in the sequence-level Coordinate System. All
** other Sequence Regions do not have sequence attached so that their sequence
** can only be fetched in the context of an Ensembl Slice, which is
** subsequently mapped to the sequence-level Coordinate System. See the
** description of the ensSequenceadaptorFetchStrBySlice function for
** further details.
******************************************************************************/

AjBool ensSequenceadaptorFetchStrBySeqregion(const EnsPSequenceadaptor sa,
                                             EnsPSeqregion sr,
                                             AjPStr *Psequence)
{
    if(!sa)
        return ajFalse;

    if(!sr)
        return ajFalse;

    if(!Psequence)
        return ajFalse;

    return ensSequenceadaptorFetchSubStrBySeqregion(
        sa,
        sr,
        1,
        (ajuint) ensSeqregionGetLength(sr),
        Psequence);
}




/* @func ensSequenceadaptorFetchSubSeqBySeqregion *****************************
**
** Fetch a sub-sequence of an Ensembl Sequence Region as an AJAX Sequence.
** The start coodinate is one-based, as in the SQL SUBSTRING function.
** A start of 1 and a length equal the Sequence Region length covers the whole
** Sequence Region.
**
** The caller is responsible for deleting the AJAX Sequence.
**
** @param [r] sa [const EnsPSequenceadaptor] Ensembl Sequence Adaptor
** @param [r] sr [EnsPSeqregion] Ensembl Sequence Region
** @param [r] start [ajuint] Start coordinate
** @param [r] length [ajuint] Sequence length
** @param [wP] Psequence [AjPSeq*] AJAX Sequence address
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
** @@
** This function will only return biological sequence information for Ensembl
** Sequence Regions, which are in the sequence-level Coordinate System. All
** other Sequence Regions do not have sequence attached so that their sequence
** can only be fetched in the context of an Ensembl Slices, which is
** subsequently mapped to the sequence-level Coordinate System. See the
** description of the ensSequenceadaptorFetchSeqBySlice function for
** further details.
******************************************************************************/

AjBool ensSequenceadaptorFetchSubSeqBySeqregion(const EnsPSequenceadaptor sa,
                                                EnsPSeqregion sr,
                                                ajuint start,
                                                ajuint length,
                                                AjPSeq *Psequence)
{
    AjPStr name     = NULL;
    AjPStr sequence = NULL;

    if(ajDebugTest("ensSequenceadaptorFetchSubSeqBySeqregion"))
    {
        ajDebug("ensSequenceadaptorFetchSubSeqBySeqregion\n"
                "  sa %p\n"
                "  sr %p\n"
                "  start %u\n"
                "  length %u\n"
                "  Psequence %p\n",
                sa,
                sr,
                start,
                length,
                Psequence);

        ensSeqregionTrace(sr, 1);
    }

    if(!sa)
        return ajFalse;

    if(!sr)
        return ajFalse;

    if(!Psequence)
        return ajFalse;

    name = ajFmtStr("%S:%S:%S:%u:%u:1",
                    ensCoordsystemGetName(ensSeqregionGetCoordsystem(sr)),
                    ensCoordsystemGetVersion(ensSeqregionGetCoordsystem(sr)),
                    ensSeqregionGetName(sr),
                    start,
                    start + length - 1);

    ensSequenceadaptorFetchSubStrBySeqregion(sa, sr, start, length, &sequence);

    *Psequence = ajSeqNewNameS(sequence, name);

    ajSeqSetNuc(*Psequence);

    ajStrDel(&name);
    ajStrDel(&sequence);

    return ajTrue;
}




/* @func ensSequenceadaptorFetchSeqBySeqregion ********************************
**
** Fetch the sequence of an Ensembl Sequence Region as an AJAX Sequence.
**
** The caller is responsible for deleting the AJAX String.
**
** @param [r] sa [const EnsPSequenceadaptor] Ensembl Sequence Adaptor
** @param [r] sr [EnsPSeqregion] Ensembl Sequence Region
** @param [wP] Psequence [AjPSeq*] AJAX Sequence address
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
** @@
** This function will only return biological sequence information for Ensembl
** Sequence Regions, which are in the sequence-level Coordinate System. All
** other Sequence Regions do not have sequence attached so that their sequence
** can only be fetched in the context of an Ensembl Slices, which is
** subsequently mapped to the sequence-level Coordinate System. See the
** description of the ensSequenceadaptorFetchSeqBySlice function for
** further details.
******************************************************************************/

AjBool ensSequenceadaptorFetchSeqBySeqregion(const EnsPSequenceadaptor sa,
                                             EnsPSeqregion sr,
                                             AjPSeq *Psequence)
{
    if(!sa)
        return ajFalse;

    if(!sr)
        return ajFalse;

    if(!Psequence)
        return ajFalse;

    return ensSequenceadaptorFetchSubSeqBySeqregion(
        sa,
        sr,
        1,
        (ajuint) ensSeqregionGetLength(sr),
        Psequence);
}




/* @func ensSequenceadaptorFetchSubStrBySlice *********************************
**
** Fetch a sub-sequence of an Ensembl Slice as an AJAX String.
** Coordinates are relative to the Slice and one-based.
** A start of 1 and an end equal the Slice length covers the whole Slice.
**
** The caller is responsible for deleting the AJAX String.
**
** @cc Bio::EnsEMBL::DBSQL::Sequenceadaptor::fetch_by_Slice_start_end_strand
** @see ensSequenceadaptorFetchStrBySlice
** @param [r] sa [const EnsPSequenceadaptor] Ensembl Sequence Adaptor
** @param [r] slice [EnsPSlice] Ensembl Slice
** @param [r] start [ajint] Start coordinate
** @param [r] end [ajint] End coordinate
** @param [r] strand [ajint] Strand information
** @param [u] Psequence [AjPStr*] Sequence address
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
** @@
******************************************************************************/

AjBool ensSequenceadaptorFetchSubStrBySlice(const EnsPSequenceadaptor sa,
                                            EnsPSlice slice,
                                            ajint start,
                                            ajint end,
                                            ajint strand,
                                            AjPStr *Psequence)
{
    ajint five   = 0;
    ajint three  = 0;
    ajint fshift = 0;
    ajint tshift = 0;

    ajuint srstart  = 0;
    ajuint srlength = 0;

    ajuint padding = 0;
    ajuint total   = 0;

    AjBool debug = AJFALSE;

    AjPList pslist = NULL;
    AjPList ses    = NULL;

    AjPStr tmpstr = NULL;

    EnsPCoordsystem seqlvlcs   = NULL;
    EnsPCoordsystemadaptor csa = NULL;

    EnsPProjectionsegment ps = NULL;

    EnsPSeqregion sr = NULL;

    EnsPSequenceEdit se = NULL;

    EnsPSlice eslice       = NULL;
    EnsPSlice nslice       = NULL;
    const EnsPSlice sslice = NULL;
    EnsPSliceadaptor sla   = NULL;

    debug = ajDebugTest("ensSequenceadaptorFetchSubStrBySlice");

    if(debug)
    {
        ajDebug("ensSequenceadaptorFetchSubStrBySlice\n"
                "  sa %p\n"
                "  slice %p\n"
                "  start %d\n"
                "  end %d\n"
                "  strand %d\n"
                "  Psequence %p\n",
                sa,
                slice,
                start,
                end,
                strand,
                Psequence);

        ensSliceTrace(slice, 1);
    }

    if(!sa)
        return ajFalse;

    if(!slice)
        return ajFalse;

    if(!strand)
        strand = 1;

    if(start > end)
    {
        ajDebug("ensSequenceadaptorFetchSubStrBySlice requires the start %d "
                "to be less than or equal to the end %d coordinate.\n",
                start, end);

        return ajFalse;
    }

    if(!Psequence)
        return ajFalse;

    /*
    ** Allocate an AJAX string and reserve space in blocks of 256 KiB
    ** (1 << 18) based on the requested length plus one position for the
    ** 'nul' string terminator.
    */

    if(*Psequence)
        ajStrAssignClear(Psequence);
    else
        *Psequence = ajStrNewRes((((end - start + 1 + 1)
                                   >> sequenceChunkPower) + 1)
                                 << sequenceChunkPower);

    /*
    ** Get a new Slice that spans the exact region to retrieve DNA from.
    ** Only this short region of the Slice needs mapping into the
    ** squence-level coordinate system.
    */

    /* Relative Slice coordinates range from 1 to length. */

    five = 1 - start;

    three = end - ensSliceGetLength(slice);

    if(five || three)
        ensSliceFetchExpandedSlice(slice,
                                   five,
                                   three,
                                   ajFalse,
                                   &fshift,
                                   &tshift,
                                   &eslice);
    else
        eslice = ensSliceNewRef(slice);

    /*
    ** Retrieve normalised, non-symlinked Slices, which allows fetching of
    ** haplotypes (HAPs) and pseudo-autosomal regions (PARs).
    */

    sla = ensRegistryGetSliceadaptor(sa->Adaptor);

    pslist = ajListNew();

    ensSliceadaptorFetchNormalisedSliceProjection(sla, eslice, pslist);

    if(!ajListGetLength(pslist))
        ajFatal("ensSequenceadaptorFetchSubStrBySlice could not "
                "retrieve normalised Slices. Database contains incorrect "
                "information in the 'assembly_exception' table.\n");

    /*
    ** Call this method again with any Slice that was sym-linked to by this
    ** Slice.
    */

    ajListPeekFirst(pslist, (void **) &ps);

    if((ajListGetLength(pslist) != 1) ||
       (!ensSliceMatch(ensProjectionsegmentGetTrgSlice(ps), slice)))
    {
        tmpstr = ajStrNew();

        while(ajListPop(pslist, (void **) &ps))
        {
            nslice = ensProjectionsegmentGetTrgSlice(ps);

            ensSequenceadaptorFetchStrBySlice(sa, nslice, &tmpstr);

            ajStrAppendS(Psequence, tmpstr);

            ensProjectionsegmentDel(&ps);
        }

        ajStrDel(&tmpstr);

        ajListFree(&pslist);

        if(strand < 0)
            ajSeqstrReverse(Psequence);

        ensSliceDel(&eslice);

        return ajTrue;
    }

    /*
    ** Clear the AJAX List of Ensembl Projection Segments resulting from the
    ** projection of the expanded Slice to normalised Slices.
    */

    while(ajListPop(pslist, (void **) &ps))
        ensProjectionsegmentDel(&ps);

    /*
    ** Now, we need to project this Slice onto the sequence-level
    ** Coordinate System even if the Slice is in the same Coordinate System,
    ** we want to trim out flanking gaps, if the Slice is past the edges of
    ** the Sequence Region.
    */

    csa = ensRegistryGetCoordsystemadaptor(sa->Adaptor);

    ensCoordsystemadaptorFetchSeqLevel(csa, &seqlvlcs);

    ensSliceProject(slice,
                    ensCoordsystemGetName(seqlvlcs),
                    ensCoordsystemGetVersion(seqlvlcs),
                    pslist);

    ensCoordsystemDel(&seqlvlcs);

    /*
    ** Fetch the sequence for each of the Sequence Regions projected onto.
    ** Allocate space for 512 KiB (1 << 19) that should fit a BAC clone.
    */

    tmpstr = ajStrNewRes((1 << 19) + 1);

    while(ajListPop(pslist, (void **) &ps))
    {
        /* Check for gaps between Projection Segments and pad them with Ns. */

        padding = ensProjectionsegmentGetSrcStart(ps) - total - 1;

        if(padding)
        {
            ajStrAppendCountK(Psequence, 'N', padding);

            if(debug)
                ajDebug("ensSequenceadaptorFetchSubStrBySlice got total %u "
                        "and Projection Segment source start %u, "
                        "therefore added %u N padding between.\n",
                        total,
                        ensProjectionsegmentGetSrcStart(ps),
                        padding);
        }

        sslice = ensProjectionsegmentGetTrgSlice(ps);

        sr = ensSliceGetSeqregion(sslice);

        srstart = ensSliceGetStart(sslice);

        srlength = ensSliceGetLength(sslice);

        ensSequenceadaptorFetchSubStrBySeqregion(sa,
                                                 sr,
                                                 srstart,
                                                 srlength,
                                                 &tmpstr);

        if(ensSliceGetStrand(sslice) < 0)
            ajSeqstrReverse(&tmpstr);

        ajStrAppendS(Psequence, tmpstr);

        total = ensProjectionsegmentGetSrcEnd(ps);

        ensProjectionsegmentDel(&ps);
    }

    ajStrDel(&tmpstr);

    ajListFree(&pslist);

    /* Check for any remaining gaps at the end. */

    padding = ensSliceGetLength(slice) - total;

    if(padding)
    {
        ajStrAppendCountK(Psequence, 'N', padding);

        if(debug)
            ajDebug("ensSequenceadaptorFetchSubStrBySlice got total %u "
                    "and Ensembl Slice length %u, "
                    "therefore added %u N padding.\n",
                    total,
                    ensSliceGetLength(slice),
                    padding);
    }

    /*
    ** If the sequence is too short, because we came in with a sequence-level
    ** Slice that was partially off the Sequence Region, pad the end with Ns
    ** to make it long enough.
    */

    padding = ensSliceGetLength(slice) - ajStrGetLen(*Psequence);

    if(padding)
    {
        ajStrAppendCountK(Psequence, 'N', padding);

        if(debug)
            ajDebug("ensSequenceadaptorFetchSubStrBySlice got "
                    "sequence length %u, but Slice length %u, "
                    "therefore added %u N final padding.\n",
                    ajStrGetLen(*Psequence),
                    ensSliceGetLength(slice),
                    padding);
    }

    /* Apply Sequence Edits. */

    ses = ajListNew();

    ensSliceFetchAllSequenceEdits(slice, ses);

    /*
    ** Sort Sequence Edits in reverse order to avoid the complication of
    ** adjusting down-stream Sequence Edit coordinates.
    */

    ensSequenceEditSortByStartDescending(ses);

    while(ajListPop(ses, (void **) &se))
    {
        /* Adjust Sequence Edit coodinates to the Sub-Slice. */

        ensSequenceEditApplyEdit(se, ensSliceGetStart(eslice) - 1, Psequence);

        ensSequenceEditDel(&se);
    }

    ajListFree(&ses);

    /* Reverse sequence if requested. */

    if(strand < 0)
        ajSeqstrReverse(Psequence);

    ensSliceDel(&eslice);

    return ajTrue;
}




/* @func ensSequenceadaptorFetchStrBySlice ************************************
**
** Fetch the sequence of an Ensembl Slice as an AJAX String.
**
** The caller is responsible for deleting the AJAX String.
**
** @param [r] sa [const EnsPSequenceadaptor] Ensembl Sequence Adaptor
** @param [r] slice [EnsPSlice] Ensembl Slice
** @param [u] Psequence [AjPStr*] Sequence address
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
** @@
******************************************************************************/

AjBool ensSequenceadaptorFetchStrBySlice(const EnsPSequenceadaptor sa,
                                         EnsPSlice slice,
                                         AjPStr *Psequence)
{
    if(!sa)
        return ajFalse;

    if(!slice)
        return ajFalse;

    if(!Psequence)
        return ajFalse;

    return ensSequenceadaptorFetchSubStrBySlice(
        sa,
        slice,
        1,
        ensSliceGetLength(slice),
        1,
        Psequence);
}




/* @func ensSequenceadaptorFetchSubSeqBySlice *********************************
**
** Fetch a sub-sequence of an Ensembl Slice as an AJAX Sequence.
** Coordinates are relative to the Slice and one-based.
** A start of 1 and an end equal the Slice length covers the whole Slice.
**
** The caller is responsible for deleting the AJAX Sequence.
**
** @param [r] sa [const EnsPSequenceadaptor] Ensembl Sequence Adaptor
** @param [r] slice [EnsPSlice] Ensembl Slice
** @param [r] start [ajint] Start coordinate
** @param [r] end [ajint] End coordinate
** @param [r] strand [ajint] Strand information
** @param [wP] Psequence [AjPSeq*] AJAX Sequence address
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
** @@
******************************************************************************/

AjBool ensSequenceadaptorFetchSubSeqBySlice(const EnsPSequenceadaptor sa,
                                            EnsPSlice slice,
                                            ajint start,
                                            ajint end,
                                            ajint strand,
                                            AjPSeq *Psequence)
{
    ajint srstart  = 0;
    ajint srend    = 0;
    ajint srstrand = 0;

    AjPStr name = NULL;
    AjPStr sequence = NULL;

    if(!sa)
        return ajFalse;

    if(!slice)
        return ajFalse;

    if(!strand)
        strand = 1;

    if(!Psequence)
        return ajFalse;

    /* Transform relative into absolute coordinates for the Slice name. */

    if(ensSliceGetStrand(slice) > 0)
    {
        srstart = ensSliceGetStart(slice) + start - 1;

        srend = ensSliceGetStart(slice) + end - 1;

        srstrand = +strand;
    }
    else
    {
        srstart = ensSliceGetEnd(slice) - end + 1;

        srend = ensSliceGetEnd(slice) - start + 1;

        srstrand = -strand;
    }

    name = ajFmtStr("%S:%S:%S:%d:%d:%d",
                    ensSliceGetCoordsystemName(slice),
                    ensSliceGetCoordsystemVersion(slice),
                    ensSliceGetSeqregionName(slice),
                    srstart,
                    srend,
                    srstrand);

    ensSequenceadaptorFetchSubStrBySlice(sa,
                                         slice,
                                         start,
                                         end,
                                         strand,
                                         &sequence);

    *Psequence = ajSeqNewNameS(sequence, name);

    ajSeqSetNuc(*Psequence);

    ajStrDel(&name);
    ajStrDel(&sequence);

    return ajTrue;
}




/* @func ensSequenceadaptorFetchSeqBySlice ************************************
**
** Fetch the sequence of an Ensembl Slice as an AJAX Sequence.
**
** The caller is responsible for deleting the AJAX Sequence.
**
** @param [r] sa [const EnsPSequenceadaptor] Ensembl Sequence Adaptor
** @param [r] slice [EnsPSlice] Ensembl Slice
** @param [wP] Psequence [AjPSeq*] AJAX Sequence address
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
** @@
******************************************************************************/

AjBool ensSequenceadaptorFetchSeqBySlice(const EnsPSequenceadaptor sa,
                                         EnsPSlice slice,
                                         AjPSeq *Psequence)
{
    if(!sa)
        return ajFalse;

    if(!slice)
        return ajFalse;

    if(!Psequence)
        return ajFalse;

    return ensSequenceadaptorFetchSubSeqBySlice(
        sa,
        slice,
        1,
        ensSliceGetLength(slice),
        1,
        Psequence);
}
