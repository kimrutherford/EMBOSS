/* @source enssequence ********************************************************
**
** Ensembl Sequence functions
**
** @author Copyright (C) 1999 Ensembl Developers
** @author Copyright (C) 2006 Michael K. Schuster
** @version $Revision: 1.46 $
** @modified 2009 by Alan Bleasby for incorporation into EMBOSS core
** @modified $Date: 2013/02/17 13:02:40 $ by $Author: mks $
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

#include "enscache.h"
#include "ensprojectionsegment.h"
#include "enssequence.h"
#include "enssequenceedit.h"




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

/* @conststatic sequenceKChunkPower *******************************************
**
** The Ensembl Sequence Adaptor handles sequences in larger chunks internally.
** Both, memory allocation and SQL database operations become more efficient,
** since requests for neighbouring regions are likely to be returned from the
** same block.
**
** 1 << 18 = 256 KiB, about the size of a BAC clone
**
******************************************************************************/

static const ajuint sequenceKChunkPower = 18U;




/* @conststatic sequenceadaptorKCacheMaxBytes *********************************
**
** The maximum memory size in bytes the Ensembl Sequence Adaptor-internal
** Ensembl Cache can use.
**
** 1 << 26 = 64 MiB
**
******************************************************************************/

static const size_t sequenceadaptorKCacheMaxBytes = 1U << 26U;




/* @conststatic sequenceadaptorKCacheMaxCount *********************************
**
** The mamximum number of sequence chunks the Ensembl Sequence Adaptor-internal
** Ensembl Cache can hold.
**
** 1 << 16 = 64 Ki
**
******************************************************************************/

static const ajuint sequenceadaptorKCacheMaxCount = 1U << 16U;




/* @conststatic sequenceadaptorKCacheMaxSize **********************************
**
** Maximum size of a sequence chunk to be allowed into the
** Ensembl Sequence Adaptor-internal Ensembl Cache.
**
******************************************************************************/

static const size_t sequenceadaptorKCacheMaxSize = 0U;




/* @conststatic sequenceadaptorKCacheMaxLength ********************************
**
** Maximum length of a sequence request up to which it gets chunked and cached.
** Larger request are returned directly.
**
** 1 << 21 = 2 Mi, or about 8 BAC clones
**
******************************************************************************/

static const ajuint sequenceadaptorKCacheMaxLength = 1U << 21U;




/* ========================================================================= */
/* =========================== private variables =========================== */
/* ========================================================================= */




/* ========================================================================= */
/* =========================== private functions =========================== */
/* ========================================================================= */

static size_t sequenceadaptorCacheSize(const void *value);

static AjBool sequenceadaptorFetchCircularsliceSubStr(
    EnsPSequenceadaptor sqa,
    EnsPSlice slice,
    ajint start,
    ajint end,
    ajint strand,
    AjPStr *Psequence);




/* ========================================================================= */
/* ======================= All functions by section ======================== */
/* ========================================================================= */




/* @filesection enssequence ***************************************************
**
** @nam1rule ens Function belongs to the Ensembl library
**
******************************************************************************/




/* @datasection [EnsPSequenceadaptor] Ensembl Sequence Adaptor ****************
**
** @nam2rule Sequenceadaptor Functions for manipulating
** Ensembl Sequence Adaptor objects
**
** @cc Bio::EnsEMBL::DBSQL::Sequenceadaptor
** @cc CVS Revision: 1.54
** @cc CVS Tag: branch-ensembl-68
**
******************************************************************************/




/* @funcstatic sequenceadaptorCacheSize ***************************************
**
** Wrapper function to calculate the memory size of an Ensembl Sequence
** (i.e. an AJAX String) from an Ensembl Cache.
**
** @param [r] value [const void*] AJAX String
**
** @return [size_t] Memory size in bytes or 0
**
** @release 6.3.0
** @@
******************************************************************************/

static size_t sequenceadaptorCacheSize(const void *value)
{
    size_t size = 0;

    if (!value)
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
**
** @nam3rule New Constructor
**
** @argrule New dba [EnsPDatabaseadaptor] Ensembl Database Adaptor
** @argrule Obj object [EnsPSequenceadaptor] Ensembl Sequence Adaptor
** @argrule Ref object [EnsPSequenceadaptor] Ensembl Sequence Adaptor
**
** @valrule * [EnsPSequenceadaptor] Ensembl Sequence Adaptor or NULL
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
** @param [u] dba [EnsPDatabaseadaptor] Ensembl Database Adaptor
**
** @return [EnsPSequenceadaptor] Ensembl Sequence Adaptor or NULL
**
** @release 6.2.0
** @@
******************************************************************************/

EnsPSequenceadaptor ensSequenceadaptorNew(
    EnsPDatabaseadaptor dba)
{
    EnsPSequenceadaptor sqa = NULL;

    if (!dba)
        return NULL;

    if (ajDebugTest("ensSequenceadaptorNew"))
        ajDebug("ensSequenceadaptorNew\n"
                "  dba %p\n",
                dba);

    AJNEW0(sqa);

    sqa->Adaptor = dba;

    sqa->Cache = ensCacheNew(
        ensECacheTypeAlphaNumeric,
        sequenceadaptorKCacheMaxBytes,
        sequenceadaptorKCacheMaxCount,
        sequenceadaptorKCacheMaxSize,
        (void *(*)(void *)) &ajStrNewRef,
        (void (*)(void **)) &ajStrDel,
        &sequenceadaptorCacheSize,
        (void *(*)(const void *)) NULL,
        (AjBool (*)(const void *)) NULL,
        ajFalse,
        "Sequence");

    return sqa;
}




/* @section destructors *******************************************************
**
** Destruction destroys all internal data structures and frees the memory
** allocated for an Ensembl Sequence Adaptor object.
**
** @fdata [EnsPSequenceadaptor]
**
** @nam3rule Del Destroy (free) an Ensembl Sequence Adaptor
**
** @argrule * Psqa [EnsPSequenceadaptor*] Ensembl Sequence Adaptor address
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
** @param [d] Psqa [EnsPSequenceadaptor*] Ensembl Sequence Adaptor address
**
** @return [void]
**
** @release 6.2.0
** @@
******************************************************************************/

void ensSequenceadaptorDel(EnsPSequenceadaptor *Psqa)
{
    EnsPSequenceadaptor pthis = NULL;

    if (!Psqa)
        return;

#if defined(AJ_DEBUG) && AJ_DEBUG >= 1
    if (ajDebugTest("ensSequenceadaptorDel"))
        ajDebug("ensSequenceadaptorDel\n"
                "  *Psqa %p\n",
                *Psqa);
#endif /* defined(AJ_DEBUG) && AJ_DEBUG >= 1 */

    if (!(pthis = *Psqa))
        return;

    ensCacheDel(&pthis->Cache);

    ajMemFree((void **) Psqa);

    return;
}




/* @section cache *************************************************************
**
** Functions for maintaining the Ensembl Sequence Adaptor-internal cache.
**
** @fdata [EnsPSequenceadaptor]
**
** @nam3rule Cache
** @nam4rule Clear Clear the Ensembl Sequence Adaptor-internal cache
**
** @argrule * sqa [EnsPSequenceadaptor] Ensembl Sequence Adaptor
**
** @valrule * [AjBool] ajTrue upon success, ajFalse otherwise
**
** @fcategory use
******************************************************************************/




/* @func ensSequenceadaptorCacheClear *****************************************
**
** Clear the internal cache of an Ensembl Sequence Adaptor.
**
** @cc Bio::EnsEMBL::DBSQL::SequenceAdaptor::clear_cache
** @param [u] sqa [EnsPSequenceadaptor] Ensembl Sequence Adaptor
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
**
** @release 6.5.0
** @@
******************************************************************************/

AjBool ensSequenceadaptorCacheClear(EnsPSequenceadaptor sqa)
{
    if (!sqa)
        return ajFalse;

    return ensCacheClear(sqa->Cache);
}




/* @section member retrieval **************************************************
**
** Functions for returning members of an
** Ensembl Sequence Adaptor object.
**
** @fdata [EnsPSequenceadaptor]
**
** @nam3rule Get Return Ensembl Sequence Adaptor attribute(s)
** @nam4rule Databaseadaptor Return the Ensembl Database Adaptor
**
** @argrule * sqa [EnsPSequenceadaptor] Ensembl Sequence Adaptor
**
** @valrule Databaseadaptor [EnsPDatabaseadaptor]
** Ensembl Database Adaptor or NULL
**
** @fcategory use
******************************************************************************/




/* @func ensSequenceadaptorGetDatabaseadaptor ***************************
**
** Get the Ensembl Database Adaptor of an Ensembl Sequence Adaptor.
**
** @param [u] sqa [EnsPSequenceadaptor]
** Ensembl Sequence Adaptor
**
** @return [EnsPDatabaseadaptor] Ensembl Database Adaptor or NULL
**
** @release 6.5.0
** @@
******************************************************************************/

EnsPDatabaseadaptor ensSequenceadaptorGetDatabaseadaptor(
    EnsPSequenceadaptor sqa)
{
    return (sqa) ? sqa->Adaptor : NULL;
}




/* @section object fetching ***************************************************
**
** Functions for retrieving Sequence objects from an Ensembl Core database.
**
** @fdata [EnsPSequenceadaptor]
**
** @nam3rule Fetch Fetch Ensembl Sequence object(s)
** @nam4rule Seqregion Fetch by an Ensembl Sequence Region
** @nam4rule Slice Fetch by an Ensembl Slice
** @nam5rule All Fetch the complete sequence
** @nam5rule Sub Fetch a sub-sequence
** @nam6rule Str Fetch as an AJAX String
** @nam6rule Seq Fetch as an AJAX Sequence
**
** @argrule * sqa [EnsPSequenceadaptor] Ensembl Sequence Adaptor
** @argrule Seqregion sr [const EnsPSeqregion] Ensembl Sequence Region
** @argrule SeqregionSub start [ajuint] Start coordinate
** @argrule SeqregionSub length [ajuint] Sequence length
** @argrule Slice slice [EnsPSlice] Ensembl Slice
** @argrule SliceSub start [ajint] Start
** @argrule SliceSub end [ajint] End
** @argrule SliceSub strand [ajint] Strand
** @argrule Str Psequence [AjPStr*] AJAX String address
** @argrule Seq Psequence [AjPSeq*] AJAX Sequence address
**
** @valrule * [AjBool] ajTrue upon success, ajFalse otherwise
**
** @fcategory use
******************************************************************************/




/* @func ensSequenceadaptorFetchSeqregionAllSeq *******************************
**
** Fetch the sequence of an Ensembl Sequence Region as an AJAX Sequence.
**
** The caller is responsible for deleting the AJAX String.
**
** @param [u] sqa [EnsPSequenceadaptor] Ensembl Sequence Adaptor
** @param [r] sr [const EnsPSeqregion] Ensembl Sequence Region
** @param [wP] Psequence [AjPSeq*] AJAX Sequence address
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
**
** @release 6.4.0
** @@
** This function will only return biological sequence information for
** Ensembl Sequence Region objects, which are in the sequence-level
** Ensembl Coordinate System. All other Ensembl Sequence Region objects do not
** have sequence attached so that their sequence can only be fetched in the
** context of an Ensembl Slice, which is subsequently mapped to the
** sequence-level Ensembl Coordinate System. See the description of the
** ensSequenceadaptorFetchSliceAllSeq function for further details.
******************************************************************************/

AjBool ensSequenceadaptorFetchSeqregionAllSeq(EnsPSequenceadaptor sqa,
                                              const EnsPSeqregion sr,
                                              AjPSeq *Psequence)
{
    return ensSequenceadaptorFetchSeqregionSubSeq(
        sqa,
        sr,
        1,
        (ajuint) ensSeqregionGetLength(sr),
        Psequence);
}




/* @func ensSequenceadaptorFetchSeqregionAllStr *******************************
**
** Fetch the sequence of an Ensembl Sequence Region as an AJAX String.
**
** The caller is responsible for deleting the AJAX String.
**
** @cc Bio::EnsEMBL::DBSQL::Sequenceadaptor::_fetch_seq
** @see ensSequenceadaptorFetchSliceAllStr
** @param [u] sqa [EnsPSequenceadaptor] Ensembl Sequence Adaptor
** @param [r] sr [const EnsPSeqregion] Ensembl Sequence Region
** @param [u] Psequence [AjPStr*] Sequence address
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
**
** @release 6.4.0
** @@
** This function will only return biological sequence information for
** Ensembl Sequence Region objects, which are in the sequence-level
** Ensembl Coordinate System. All other Ensembl Sequence Region objects do not
** have sequence attached so that their sequence can only be fetched in the
** context of an Ensembl Slice, which is subsequently mapped to the
** sequence-level Ensembl Coordinate System. See the description of the
** ensSequenceadaptorFetchSliceAllStr function for further details.
******************************************************************************/

AjBool ensSequenceadaptorFetchSeqregionAllStr(EnsPSequenceadaptor sqa,
                                              const EnsPSeqregion sr,
                                              AjPStr *Psequence)
{
    return ensSequenceadaptorFetchSeqregionSubStr(
        sqa,
        sr,
        1,
        (ajuint) ensSeqregionGetLength(sr),
        Psequence);
}




/* @func ensSequenceadaptorFetchSeqregionSubSeq *******************************
**
** Fetch a sub-sequence of an Ensembl Sequence Region as an AJAX Sequence.
** The start coordinate is one-based, as in the SQL SUBSTRING function.
** A start of 1 and a length equal the Sequence Region length covers the whole
** Sequence Region.
**
** The caller is responsible for deleting the AJAX Sequence.
**
** @param [u] sqa [EnsPSequenceadaptor] Ensembl Sequence Adaptor
** @param [r] sr [const EnsPSeqregion] Ensembl Sequence Region
** @param [r] start [ajuint] Start coordinate
** @param [r] length [ajuint] Sequence length
** @param [wP] Psequence [AjPSeq*] AJAX Sequence address
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
**
** @release 6.4.0
** @@
** This function will only return biological sequence information for
** Ensembl Sequence Region objects, which are in the sequence-level
** Ensembl Coordinate System. All other Ensembl Sequence Region objects do not
** have sequence attached so that their sequence can only be fetched in the
** context of an Ensembl Slice, which is subsequently mapped to the
** sequence-level Ensembl Coordinate System. See the description of the
** ensSequenceadaptorFetchSliceAllSeq function for further details.
******************************************************************************/

AjBool ensSequenceadaptorFetchSeqregionSubSeq(EnsPSequenceadaptor sqa,
                                              const EnsPSeqregion sr,
                                              ajuint start,
                                              ajuint length,
                                              AjPSeq *Psequence)
{
    AjBool result = AJFALSE;

    AjPStr name     = NULL;
    AjPStr sequence = NULL;

    if (ajDebugTest("ensSequenceadaptorFetchSeqregionSubSeq"))
    {
        ajDebug("ensSequenceadaptorFetchSeqregionSubSeq\n"
                "  sqa %p\n"
                "  sr %p\n"
                "  start %u\n"
                "  length %u\n"
                "  Psequence %p\n",
                sqa,
                sr,
                start,
                length,
                Psequence);

        ensSeqregionTrace(sr, 1);
    }

    if (!sqa)
        return ajFalse;

    if (!sr)
        return ajFalse;

    if (!Psequence)
        return ajFalse;

    /*
    ** It is sligtly more efficient, if undefined AJAX String objects are
    ** directly allocated by the following functions to their final size.
    */

    name = ajFmtStr(
        "%S:%S:%S:%u:%u:1",
        ensCoordsystemGetName(ensSeqregionGetCoordsystem(sr)),
        ensCoordsystemGetVersion(ensSeqregionGetCoordsystem(sr)),
        ensSeqregionGetName(sr),
        start,
        start + length - 1);

    result = ensSequenceadaptorFetchSeqregionSubStr(
        sqa,
        sr,
        start,
        length,
        &sequence);

    if (*Psequence)
    {
        ajSeqClear(*Psequence);

        ajSeqAssignNameS(*Psequence, name);
        ajSeqAssignSeqS(*Psequence, sequence);
    }
    else
        *Psequence = ajSeqNewNameS(sequence, name);

    ajSeqSetNuc(*Psequence);

    ajStrDel(&name);
    ajStrDel(&sequence);

    return result;
}




/* @func ensSequenceadaptorFetchSeqregionSubStr *******************************
**
** Fetch a sub-sequence of an Ensembl Sequence Region as an AJAX String.
** The start coordinate is one-based, as in the SQL SUBSTRING function.
** A start of 1 and a length equal the Sequence Region length covers the whole
** Sequence Region.
**
** The caller is responsible for deleting the AJAX String.
**
** @cc Bio::EnsEMBL::DBSQL::Sequenceadaptor::_fetch_seq
** @see ensSequenceadaptorFetchSliceSubStr
** @param [u] sqa [EnsPSequenceadaptor] Ensembl Sequence Adaptor
** @param [r] sr [const EnsPSeqregion] Ensembl Sequence Region
** @param [r] start [ajuint] Start coordinate
** @param [r] length [ajuint] Sequence length
** @param [u] Psequence [AjPStr*] Sequence address
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
**
** @release 6.4.0
** @@
** This function will only return biological sequence information for
** Ensembl Sequence Region objects, which are in the sequence-level
** Ensembl Coordinate System. All other Ensembl Sequence Region objects do not
** have sequence attached so that their sequence can only be fetched in the
** context of an Ensembl Slice, which is subsequently mapped to the
** sequence-level Ensembl Coordinate System. See the description of the
** ensSequenceadaptorFetchSliceSubStr function for further details.
******************************************************************************/

AjBool ensSequenceadaptorFetchSeqregionSubStr(EnsPSequenceadaptor sqa,
                                              const EnsPSeqregion sr,
                                              ajuint start,
                                              ajuint length,
                                              AjPStr *Psequence)
{
    register ajuint i = 0U;

    ajuint chkmin = 0U;
    ajuint chkmax = 0U;
    ajuint posmin = 0U;

    AjPSqlstatement sqls = NULL;
    AjISqlrow sqli       = NULL;
    AjPSqlrow sqlr       = NULL;

    AjPStr chkstr    = NULL;
    AjPStr tmpstr    = NULL;
    AjPStr key       = NULL;
    AjPStr statement = NULL;

    EnsPDatabaseadaptor dba = NULL;

    if (ajDebugTest("ensSequenceadaptorFetchSeqregionSubStr"))
    {
        ajDebug("ensSequenceadaptorFetchSeqregionSubStr\n"
                "  sqa %p\n"
                "  sr %p\n"
                "  start %d\n"
                "  length %d\n",
                sqa,
                sr,
                start,
                length);

        ensSeqregionTrace(sr, 1);
    }

    if (!sqa)
        return ajFalse;

    if (!sr)
        return ajFalse;

    if (!Psequence)
        return ajFalse;

    /*
    ** Reserve sequence space in larger blocks based on the requested length
    ** plus one position for the 'nul' string terminator.
    */

    if (*Psequence)
        ajStrAssignClear(Psequence);
    else
        *Psequence = ajStrNewRes((((length + 1) >> sequenceKChunkPower) + 1)
                                 << sequenceKChunkPower);

    dba = ensSequenceadaptorGetDatabaseadaptor(sqa);

    if (length < sequenceadaptorKCacheMaxLength)
    {
        chkmin = (start - 1) >> sequenceKChunkPower;

        chkmax = (start + length - 1) >> sequenceKChunkPower;

        /*
        ** Allocate an AJAX String and reserve space for the number of
        ** sequence chunks plus one for the 'nul' string terminator.
        */

        tmpstr = ajStrNewRes(((1 << sequenceKChunkPower) + 1) *
                             (chkmax - chkmin + 1));

        /* Piece together sequence from cached sequence chunks. */

        for (i = chkmin; i <= chkmax; i++)
        {
            key = ajFmtStr("%u:%u", ensSeqregionGetIdentifier(sr), i);

            chkstr = NULL;

            ensCacheFetch(sqa->Cache, (void *) key, (void **) &chkstr);

            if (chkstr)
            {
                ajStrAppendS(&tmpstr, chkstr);

                ajStrDel(&chkstr);
            }
            else
            {
                /* Fetch uncached sequence chunks. */

                posmin = (i << sequenceKChunkPower) + 1;

                statement = ajFmtStr(
                    "SELECT "
                    "SUBSTRING(dna.sequence FROM %u FOR %u) "
                    "FROM "
                    "dna "
                    "WHERE "
                    "dna.seq_region_id = %u",
                    posmin,
                    1 << sequenceKChunkPower,
                    ensSeqregionGetIdentifier(sr));

                sqls = ensDatabaseadaptorSqlstatementNew(dba, statement);

                sqli = ajSqlrowiterNew(sqls);

                while (!ajSqlrowiterDone(sqli))
                {
                    sqlr = ajSqlrowiterGet(sqli);

                    /*
                    ** Allocate an AJAX String and reserve space for the
                    ** maximum sequence chunk length plus the 'nul' string
                    ** terminator.
                    */

                    chkstr = ajStrNewRes((1 << sequenceKChunkPower) + 1);

                    ajSqlcolumnToStr(sqlr, &chkstr);

                    /*
                    ** Always store upper case sequence
                    ** so that it can be properly soft-masked.
                    */

                    ajStrFmtUpper(&chkstr);

                    ensCacheStore(sqa->Cache, (void *) key, (void **) &chkstr);

                    ajStrAppendS(&tmpstr, chkstr);

                    ajStrDel(&chkstr);
                }

                ajSqlrowiterDel(&sqli);

                ensDatabaseadaptorSqlstatementDel(dba, &sqls);

                ajStrDel(&statement);
            }

            ajStrDel(&key);
        }

        /* Return only the requested portion of the entire sequence. */

        posmin = (chkmin << sequenceKChunkPower) + 1;

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

        sqls = ensDatabaseadaptorSqlstatementNew(dba, statement);

        sqli = ajSqlrowiterNew(sqls);

        while (!ajSqlrowiterDone(sqli))
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

        ensDatabaseadaptorSqlstatementDel(dba, &sqls);

        ajStrDel(&statement);
    }

    return ajTrue;
}




/* @func ensSequenceadaptorFetchSliceAllSeq ***********************************
**
** Fetch the sequence of an Ensembl Slice as an AJAX Sequence.
**
** The caller is responsible for deleting the AJAX Sequence.
**
** @param [u] sqa [EnsPSequenceadaptor] Ensembl Sequence Adaptor
** @param [u] slice [EnsPSlice] Ensembl Slice
** @param [wP] Psequence [AjPSeq*] AJAX Sequence address
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
**
** @release 6.4.0
** @@
******************************************************************************/

AjBool ensSequenceadaptorFetchSliceAllSeq(EnsPSequenceadaptor sqa,
                                          EnsPSlice slice,
                                          AjPSeq *Psequence)
{
    return ensSequenceadaptorFetchSliceSubSeq(
        sqa,
        slice,
        1,
        ensSliceCalculateLength(slice),
        1,
        Psequence);
}




/* @func ensSequenceadaptorFetchSliceAllStr ***********************************
**
** Fetch the sequence of an Ensembl Slice as an AJAX String.
**
** The caller is responsible for deleting the AJAX String.
**
** @param [u] sqa [EnsPSequenceadaptor] Ensembl Sequence Adaptor
** @param [u] slice [EnsPSlice] Ensembl Slice
** @param [u] Psequence [AjPStr*] Sequence address
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
**
** @release 6.4.0
** @@
******************************************************************************/

AjBool ensSequenceadaptorFetchSliceAllStr(EnsPSequenceadaptor sqa,
                                          EnsPSlice slice,
                                          AjPStr *Psequence)
{
    return ensSequenceadaptorFetchSliceSubStr(
        sqa,
        slice,
        1,
        ensSliceCalculateLength(slice),
        1,
        Psequence);
}




/* @func ensSequenceadaptorFetchSliceSubSeq ***********************************
**
** Fetch a sub-sequence of an Ensembl Slice as an AJAX Sequence.
** Coordinates are relative to the Slice and one-based.
** A start of 1 and an end equal the Slice length covers the whole Slice.
**
** The caller is responsible for deleting the AJAX Sequence.
**
** @param [u] sqa [EnsPSequenceadaptor] Ensembl Sequence Adaptor
** @param [u] slice [EnsPSlice] Ensembl Slice
** @param [r] start [ajint] Start coordinate
** @param [r] end [ajint] End coordinate
** @param [r] strand [ajint] Strand information
** @param [wP] Psequence [AjPSeq*] AJAX Sequence address
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
**
** @release 6.4.0
** @@
******************************************************************************/

AjBool ensSequenceadaptorFetchSliceSubSeq(EnsPSequenceadaptor sqa,
                                          EnsPSlice slice,
                                          ajint start,
                                          ajint end,
                                          ajint strand,
                                          AjPSeq *Psequence)
{
    ajint srstart  = 0;
    ajint srend    = 0;
    ajint srstrand = 0;

    AjBool result = AJFALSE;

    AjPStr name     = NULL;
    AjPStr sequence = NULL;

    if (!sqa)
        return ajFalse;

    if (!slice)
        return ajFalse;

    if (!strand)
        strand = 1;

    if (!Psequence)
        return ajFalse;

    /* Transform relative into absolute coordinates for the Slice name. */

    if (ensSliceGetStrand(slice) > 0)
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

    /*
    ** It is sligtly more efficient, if undefined AJAX String objects are
    ** directly allocated by the following functions to their final size.
    */

    name = ajFmtStr(
        "%S:%S:%S:%d:%d:%d",
        ensSliceGetCoordsystemName(slice),
        ensSliceGetCoordsystemVersion(slice),
        ensSliceGetSeqregionName(slice),
        srstart,
        srend,
        srstrand);

    result = ensSequenceadaptorFetchSliceSubStr(
        sqa,
        slice,
        start,
        end,
        strand,
        &sequence);

    if (*Psequence)
    {
        ajSeqClear(*Psequence);

        ajSeqAssignNameS(*Psequence, name);
        ajSeqAssignSeqS(*Psequence, sequence);
    }
    else
        *Psequence = ajSeqNewNameS(sequence, name);

    ajSeqSetNuc(*Psequence);

    ajStrDel(&name);
    ajStrDel(&sequence);

    return result;
}




/* @func ensSequenceadaptorFetchSliceSubStr ***********************************
**
** Fetch a sub-sequence of an Ensembl Slice as an AJAX String.
** Coordinates are relative to the Slice and one-based.
** A start of 1 and an end equal the Slice length covers the whole Slice.
**
** The caller is responsible for deleting the AJAX String.
**
** @cc Bio::EnsEMBL::DBSQL::Sequenceadaptor::fetch_by_Slice_start_end_strand
** @see ensSequenceadaptorFetchSliceAllStr
** @param [u] sqa [EnsPSequenceadaptor] Ensembl Sequence Adaptor
** @param [u] slice [EnsPSlice] Ensembl Slice
** @param [r] start [ajint] Start coordinate
** @param [r] end [ajint] End coordinate
** @param [r] strand [ajint] Strand information
** @param [u] Psequence [AjPStr*] Sequence address
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
**
** @release 6.4.0
** @@
******************************************************************************/

AjBool ensSequenceadaptorFetchSliceSubStr(EnsPSequenceadaptor sqa,
                                          EnsPSlice slice,
                                          ajint start,
                                          ajint end,
                                          ajint strand,
                                          AjPStr *Psequence)
{
    const char *Ptr = NULL;

    register ajuint i = 0U;

    ajint five   = 0;
    ajint three  = 0;
    ajint fshift = 0;
    ajint tshift = 0;

    ajuint padding = 0U;
    ajuint total   = 0U;

    AjBool circular = AJFALSE;
    AjBool debug    = AJFALSE;

    AjPList pslist = NULL;
    AjPList ses    = NULL;

    AjPStr tmpstr = NULL;

    EnsPCoordsystem seqlvlcs   = NULL;
    EnsPCoordsystemadaptor csa = NULL;

    EnsPDatabaseadaptor dba = NULL;

    EnsPProjectionsegment ps = NULL;

    EnsPSequenceedit se = NULL;

    EnsPSlice eslice     = NULL;
    EnsPSlice nslice     = NULL;
    EnsPSlice sslice     = NULL;
    EnsPSliceadaptor sla = NULL;

    debug = ajDebugTest("ensSequenceadaptorFetchSliceSubStr");

    if (debug)
    {
        ajDebug("ensSequenceadaptorFetchSliceSubStr\n"
                "  sqa %p\n"
                "  slice %p\n"
                "  start %d\n"
                "  end %d\n"
                "  strand %d\n"
                "  Psequence %p\n",
                sqa,
                slice,
                start,
                end,
                strand,
                Psequence);

        ensSliceTrace(slice, 1);
    }

    if (!sqa)
        return ajFalse;

    if (!slice)
        return ajFalse;

    if (!Psequence)
        return ajFalse;

    if (!ensSliceIsCircular(slice, &circular))
        return ajFalse;

    if (circular == ajTrue)
    {
        if (start > end)
            return sequenceadaptorFetchCircularsliceSubStr(
                sqa,
                slice,
                start,
                end,
                strand,
                Psequence);

        if (start < 0)
            start += ensSliceGetSeqregionLength(slice);

        if (end < 0)
            end += ensSliceGetSeqregionLength(slice);

        if (ensSliceGetStart(slice) > ensSliceGetEnd(slice))
            return sequenceadaptorFetchCircularsliceSubStr(
                sqa,
                slice,
                ensSliceGetStart(slice),
                ensSliceGetEnd(slice),
                strand,
                Psequence);
    }

    if (start > end)
    {
        ajDebug("ensSequenceadaptorFetchSliceSubStr requires the start %d "
                "to be less than or equal to the end %d coordinate.\n",
                start, end);

        return ajFalse;
    }

    if (!strand)
        strand = 1;

    /*
    ** Reserve sequence space in larger blocks based on the requested length
    ** plus one position for the 'nul' string terminator.
    */

    if (*Psequence)
        ajStrAssignClear(Psequence);
    else
        *Psequence = ajStrNewRes(
            (((ensSliceCalculateRegion(slice, start, end) + 1)
              >> sequenceKChunkPower) + 1) << sequenceKChunkPower);

    dba = ensSequenceadaptorGetDatabaseadaptor(sqa);

    csa = ensRegistryGetCoordsystemadaptor(dba);
    sla = ensRegistryGetSliceadaptor(dba);

    /*
    ** Get a new Slice that spans the exact region to retrieve DNA from.
    ** Only this short region of the Slice needs mapping into the
    ** sequence-level coordinate system.
    */

    /* Relative Slice coordinates range from 1 to length. */

    five = 1 - start;

    three = end - ensSliceCalculateLength(slice);

    if (five || three)
        ensSliceFetchSliceexpanded(slice,
                                   five,
                                   three,
                                   ajFalse,
                                   &fshift,
                                   &tshift,
                                   &eslice);
    else
        eslice = ensSliceNewRef(slice);

    /*
    ** Retrieve normalised, non-symlinked Slice objects, which allows fetching
    ** of haplotypes (HAPs) and pseudo-autosomal regions (PARs).
    */

    pslist = ajListNew();

    ensSliceadaptorRetrieveNormalisedprojection(sla, eslice, pslist);

    if (!ajListGetLength(pslist))
        ajFatal("ensSequenceadaptorFetchSliceSubStr could not "
                "retrieve normalised Slices. Database contains incorrect "
                "information in the 'assembly_exception' table.\n");

    /*
    ** Call this method again with any Slice that was sym-linked to by this
    ** Slice.
    */

    ajListPeekFirst(pslist, (void **) &ps);

    if ((ajListGetLength(pslist) != 1) ||
        (!ensSliceMatch(ensProjectionsegmentGetTargetSlice(ps), slice)))
    {
        tmpstr = ajStrNew();

        while (ajListPop(pslist, (void **) &ps))
        {
            nslice = ensProjectionsegmentGetTargetSlice(ps);

            ensSequenceadaptorFetchSliceAllStr(sqa, nslice, &tmpstr);

            ajStrAppendS(Psequence, tmpstr);

            ensProjectionsegmentDel(&ps);
        }

        ajStrDel(&tmpstr);

        ajListFree(&pslist);

        if (strand < 0)
            ajSeqstrReverse(Psequence);

        ensSliceDel(&eslice);

        return ajTrue;
    }

    /*
    ** Clear the AJAX List of Ensembl Projection Segment objects resulting
    ** from the projection of the expanded Slice object to normalised
    ** Slice objects.
    */

    while (ajListPop(pslist, (void **) &ps))
        ensProjectionsegmentDel(&ps);

    /*
    ** Now, this Slice needs projecting onto the sequence-level Coordinate
    ** System even if it is already in this Coordinate System, because
    ** flanking gaps need trimming out the Slice is past the edges of
    ** the Sequence Region.
    */

    ensCoordsystemadaptorFetchSeqlevel(csa, &seqlvlcs);

    ensSliceProject(slice,
                    ensCoordsystemGetName(seqlvlcs),
                    ensCoordsystemGetVersion(seqlvlcs),
                    pslist);

    ensCoordsystemDel(&seqlvlcs);

    /*
    ** Fetch the sequence for each of the Ensembl Sequence Region objects
    ** projected onto. Allocate space for 512 KiB (1 << 19) that should fit
    ** one BAC clone.
    */

    tmpstr = ajStrNewRes((1 << 19) + 1);

    while (ajListPop(pslist, (void **) &ps))
    {
        /*
        ** Check for gaps between Ensembl Projection Segment objects and
        ** pad them with Ns.
        */

        padding = ensProjectionsegmentGetSourceStart(ps) - total - 1;

        if (padding)
        {
            ajStrAppendCountK(Psequence, 'N', padding);

            if (debug)
                ajDebug("ensSequenceadaptorFetchSliceSubStr got total %u "
                        "and Projection Segment source start %u, "
                        "therefore added %u N padding between.\n",
                        total,
                        ensProjectionsegmentGetSourceStart(ps),
                        padding);
        }

        sslice = ensProjectionsegmentGetTargetSlice(ps);

        ensSequenceadaptorFetchSeqregionSubStr(
            sqa,
            ensSliceGetSeqregion(sslice),
            ensSliceGetStart(sslice),
            ensSliceCalculateLength(sslice),
            &tmpstr);

        if (ensSliceGetStrand(sslice) < 0)
            ajSeqstrReverse(&tmpstr);

        ajStrAppendS(Psequence, tmpstr);

        total = ensProjectionsegmentGetSourceEnd(ps);

        ensProjectionsegmentDel(&ps);
    }

    ajStrDel(&tmpstr);

    ajListFree(&pslist);

    /* Check for any remaining gaps at the end. */

    padding = ensSliceCalculateLength(slice) - total;

    if (padding)
    {
        ajStrAppendCountK(Psequence, 'N', padding);

        if (debug)
            ajDebug("ensSequenceadaptorFetchSliceSubStr got total %u "
                    "and Ensembl Slice length %u, "
                    "therefore added %u N padding.\n",
                    total,
                    ensSliceCalculateLength(slice),
                    padding);
    }

    /*
    ** If the sequence is too short, because we came in with a sequence-level
    ** Slice that was partially off the Sequence Region, pad the end with Ns
    ** to make it long enough.
    ** NOTE: Since ajStrGetLen returns size_t, which exceeds ajint,
    ** the sequence length needs to be determined here.
    ** padding = ensSliceCalculateLength(slice) - ajStrGetLen(*Psequence);
    */

    for (i = 0U, Ptr = ajStrGetPtr(*Psequence); (Ptr && *Ptr); i++, Ptr++)
        if (i == UINT_MAX)
            ajFatal("ensSequenceadaptorFetchSliceSubStr exeeded UINT_MAX.");

    padding = ensSliceCalculateLength(slice) - i;

    if (padding)
    {
        ajStrAppendCountK(Psequence, 'N', padding);

        if (debug)
            ajDebug("ensSequenceadaptorFetchSliceSubStr got "
                    "sequence length %u, but Slice length %u, "
                    "therefore added %u N final padding.\n",
                    ajStrGetLen(*Psequence),
                    ensSliceCalculateLength(slice),
                    padding);
    }

    /* Apply Sequence Edits. */

    ses = ajListNew();

    ensSliceFetchAllSequenceedits(slice, ses);

    /*
    ** Sort Sequence Edits in reverse order to avoid the complication of
    ** adjusting down-stream Sequence Edit coordinates.
    */

    ensListSequenceeditSortStartDescending(ses);

    while (ajListPop(ses, (void **) &se))
    {
        /* Adjust Sequence Edit coordinates to the Sub-Slice. */

        ensSequenceeditApplyString(se,
                                   ensSliceGetStart(eslice) - 1,
                                   Psequence);

        ensSequenceeditDel(&se);
    }

    ajListFree(&ses);

    /* Reverse sequence if requested. */

    if (strand < 0)
        ajSeqstrReverse(Psequence);

    ensSliceDel(&eslice);

    return ajTrue;
}




/* @funcstatic sequenceadaptorFetchCircularsliceSubStr ************************
**
** Fetch a sub-sequence of a (circular) Ensembl Slice as an AJAX String.
** Coordinates are relative to the Slice and one-based.
** A start of 1 and an end equal the Slice length covers the whole Slice.
**
** The caller is responsible for deleting the AJAX String.
**
** @cc Bio::EnsEMBL::DBSQL::Sequenceadaptor::
** _fetch_by_Slice_start_end_strand_circular
** @param [u] sqa [EnsPSequenceadaptor] Ensembl Sequence Adaptor
** @param [u] slice [EnsPSlice] Ensembl Slice
** @param [r] start [ajint] Start coordinate
** @param [r] end [ajint] End coordinate
** @param [r] strand [ajint] Strand information
** @param [u] Psequence [AjPStr*] Sequence address
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
**
** @release 6.4.0
** @@
******************************************************************************/

static AjBool sequenceadaptorFetchCircularsliceSubStr(EnsPSequenceadaptor sqa,
                                                      EnsPSlice slice,
                                                      ajint start,
                                                      ajint end,
                                                      ajint strand,
                                                      AjPStr *Psequence)
{
    const char *Ptr = NULL;

    register ajuint i = 0U;

    ajint mpoint = 0;

    ajint five   = 0;
    ajint three  = 0;
    ajint fshift = 0;
    ajint tshift = 0;

    ajuint padding = 0U;
    ajuint total   = 0U;

    AjBool circular = AJFALSE;
    AjBool debug    = AJFALSE;

    AjPList pslist = NULL;
    AjPList ses    = NULL;

    AjPStr sequence1 = NULL;
    AjPStr sequence2 = NULL;
    AjPStr tmpstr    = NULL;

    EnsPCoordsystem seqlvlcs   = NULL;
    EnsPCoordsystemadaptor csa = NULL;

    EnsPDatabaseadaptor dba = NULL;

    EnsPProjectionsegment ps = NULL;

    EnsPSequenceedit se = NULL;

    EnsPSlice eslice     = NULL;
    EnsPSlice nslice     = NULL;
    EnsPSlice sslice     = NULL;
    EnsPSliceadaptor sla = NULL;

    debug = ajDebugTest("sequenceadaptorFetchCircularsliceSubStr");

    if (debug)
    {
        ajDebug("sequenceadaptorFetchCircularsliceSubStr\n"
                "  sqa %p\n"
                "  slice %p\n"
                "  start %d\n"
                "  end %d\n"
                "  strand %d\n"
                "  Psequence %p\n",
                sqa,
                slice,
                start,
                end,
                strand,
                Psequence);

        ensSliceTrace(slice, 1);
    }

    if (!sqa)
        return ajFalse;

    if (!slice)
        return ajFalse;

    if (!Psequence)
        return ajFalse;

    if (*Psequence)
        ajStrAssignClear(Psequence);
    else
        *Psequence = ajStrNewRes(
            (((ensSliceCalculateRegion(slice, start, end) + 1)
              >> sequenceKChunkPower) + 1) << sequenceKChunkPower);

    if (!ensSliceIsCircular(slice, &circular))
        return ajFalse;

    dba = ensSequenceadaptorGetDatabaseadaptor(sqa);

    csa = ensRegistryGetCoordsystemadaptor(dba);
    sla = ensRegistryGetSliceadaptor(dba);

    if ((start > end) && (circular == ajTrue))
    {
        mpoint
            = ensSliceGetSeqregionLength(slice)
            - ensSliceGetStart(slice)
            + 1;

        sequence1 = ajStrNew();
        sequence2 = ajStrNew();

        sequenceadaptorFetchCircularsliceSubStr(
            sqa,
            slice,
            1,
            mpoint,
            1,
            &sequence1);

        sequenceadaptorFetchCircularsliceSubStr(
            sqa,
            slice,
            mpoint + 1,
            ensSliceCalculateLength(slice),
            1,
            &sequence1);

        if (ensSliceGetStrand(slice) >= 0)
        {
            ajStrAppendS(Psequence, sequence1);
            ajStrAppendS(Psequence, sequence2);
        }
        else
        {
            ajStrAppendS(Psequence, sequence2);
            ajStrAppendS(Psequence, sequence1);

            ajSeqstrReverse(Psequence);
        }

        ajStrDel(&sequence1);
        ajStrDel(&sequence2);

        return ajTrue;
    }

    /* Get a new Slice that spans the exact region to retrieve DNA from. */

    /* Relative Slice coordinates range from 1 to length. */

    five = 1 - start;

    three = end - ensSliceCalculateLength(slice);

    if (five || three)
    {
        if (ensSliceGetStrand(slice) >= 0)
            ensSliceFetchSliceexpanded(slice,
                                       five,
                                       three,
                                       ajFalse,
                                       &fshift,
                                       &tshift,
                                       &eslice);
        else
            ensSliceFetchSliceexpanded(slice,
                                       three,
                                       five,
                                       ajFalse,
                                       &tshift,
                                       &fshift,
                                       &eslice);
    }

    /*
    ** Retrieve normalized non-symlinked Slice objects, which allows fetching
    ** of haplotypes (HAPs) and pseudo-autosomal regions (PARs).
    */

    pslist = ajListNew();

    ensSliceadaptorRetrieveNormalisedprojection(sla, eslice, pslist);

    if (!ajListGetLength(pslist))
        ajFatal("sequenceadaptorFetchCircularsliceSubStr could not "
                "retrieve normalised Slices. Database contains incorrect "
                "information in the 'assembly_exception' table.\n");


    /*
    ** Call this method again with any Slice that was sym-linked to by this
    ** Slice.
    */

    ajListPeekFirst(pslist, (void **) &ps);

    if ((ajListGetLength(pslist) != 1) ||
        (!ensSliceMatch(ensProjectionsegmentGetTargetSlice(ps), slice)))
    {
        tmpstr = ajStrNew();

        while (ajListPop(pslist, (void **) &ps))
        {
            nslice = ensProjectionsegmentGetTargetSlice(ps);

            ensSequenceadaptorFetchSliceAllStr(sqa, nslice, &tmpstr);

            ajStrAppendS(Psequence, tmpstr);

            ensProjectionsegmentDel(&ps);
        }

        ajStrDel(&tmpstr);

        ajListFree(&pslist);

        if (strand < 0)
            ajSeqstrReverse(Psequence);

        ensSliceDel(&eslice);

        return ajTrue;
    }

    /*
    ** Clear the AJAX List of Ensembl Projection Segment objects resulting
    ** from the projection of the expanded Slice object to normalised
    ** Slice objects.
    */

    while (ajListPop(pslist, (void **) &ps))
        ensProjectionsegmentDel(&ps);

    /*
    ** Now, this Slice needs projecting onto the sequence-level Coordinate
    ** System even if it is already in this Coordinate System, because
    ** flanking gaps need trimming out the Slice is past the edges of
    ** the Sequence Region.
    */

    ensCoordsystemadaptorFetchSeqlevel(csa, &seqlvlcs);

    ensSliceProject(slice,
                    ensCoordsystemGetName(seqlvlcs),
                    ensCoordsystemGetVersion(seqlvlcs),
                    pslist);

    ensCoordsystemDel(&seqlvlcs);

    /*
    ** Fetch the sequence for each of the Ensembl Sequence Region objects
    ** projected onto. Allocate space for 512 KiB (1 << 19) that should fit
    ** one BAC clone.
    */

    tmpstr = ajStrNewRes((1 << 19) + 1);

    while (ajListPop(pslist, (void **) &ps))
    {
        /*
        ** Check for gaps between Ensembl Projection Segment objects and
        ** pad them with Ns.
        */

        padding = ensProjectionsegmentGetSourceStart(ps) - total - 1;

        if (padding)
        {
            ajStrAppendCountK(Psequence, 'N', padding);

            if (debug)
                ajDebug("sequenceadaptorFetchCircularsliceSubStr got total %u "
                        "and Projection Segment source start %u, "
                        "therefore added %u N padding between.\n",
                        total,
                        ensProjectionsegmentGetSourceStart(ps),
                        padding);
        }

        sslice = ensProjectionsegmentGetTargetSlice(ps);

        ensSequenceadaptorFetchSeqregionSubStr(
            sqa,
            ensSliceGetSeqregion(sslice),
            ensSliceGetStart(sslice),
            ensSliceCalculateLength(sslice),
            &tmpstr);

        if (ensSliceGetStrand(sslice) < 0)
            ajSeqstrReverse(&tmpstr);

        ajStrAppendS(Psequence, tmpstr);

        total = ensProjectionsegmentGetSourceEnd(ps);

        ensProjectionsegmentDel(&ps);
    }

    /* Check for any remaining gaps at the end. */

    padding = ensSliceCalculateLength(slice) - total;

    if (padding)
    {
        ajStrAppendCountK(Psequence, 'N', padding);

        if (debug)
            ajDebug("sequenceadaptorFetchCircularsliceSubStr got total %u "
                    "and Ensembl Slice length %u, "
                    "therefore added %u N padding.\n",
                    total,
                    ensSliceCalculateLength(slice),
                    padding);
    }

    /*
    ** If the sequence is too short, because we came in with a sequence-level
    ** Slice that was partially off the Sequence Region, pad the end with Ns
    ** to make it long enough.
    ** NOTE: Since ajStrGetLen returns size_t, which exceeds ajint,
    ** the sequence length needs to be determined here.
    ** padding = ensSliceCalculateLength(slice) - ajStrGetLen(*Psequence);
    */

    for (i = 0U, Ptr = ajStrGetPtr(*Psequence); (Ptr && *Ptr); i++, Ptr++)
        if (i == UINT_MAX)
            ajFatal("sequenceadaptorFetchCircularsliceSubStr exeeded "
                    "UINT_MAX.");

    padding = ensSliceCalculateLength(slice) - i;

    if (padding)
    {
        ajStrAppendCountK(Psequence, 'N', padding);

        if (debug)
            ajDebug("sequenceadaptorFetchCircularsliceSubStr got "
                    "sequence length %u, but Slice length %u, "
                    "therefore added %u N final padding.\n",
                    ajStrGetLen(*Psequence),
                    ensSliceCalculateLength(slice),
                    padding);
    }

    /* Apply Sequence Edits. */

    ses = ajListNew();

    ensSliceFetchAllSequenceedits(slice, ses);

    /*
    ** Sort Sequence Edits in reverse order to avoid the complication of
    ** adjusting down-stream Sequence Edit coordinates.
    */

    ensListSequenceeditSortStartDescending(ses);

    while (ajListPop(ses, (void **) &se))
    {
        /* Adjust Sequence Edit coordinates to the Sub-Slice. */

        ensSequenceeditApplyString(se,
                                   ensSliceGetStart(eslice) - 1,
                                   Psequence);

        ensSequenceeditDel(&se);
    }

    ajListFree(&ses);

    /* Reverse sequence if requested. */

    if (strand < 0)
        ajSeqstrReverse(Psequence);

    ensSliceDel(&eslice);

    return ajTrue;
}
