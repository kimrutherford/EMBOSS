/* @source Ensembl Sequence functions
**
** @author Copyright (C) 1999 Ensembl Developers
** @author Copyright (C) 2006 Michael K. Schuster
** @modified 2009 by Alan Bleasby for incorporation into EMBOSS core
** @modified $Date: 2011/07/06 21:50:28 $ by $Author: mks $
** @version $Revision: 1.29 $
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

#include "enscache.h"
#include "ensprojectionsegment.h"
#include "enssequence.h"
#include "enssequenceedit.h"




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

/* @conststatic sequenceChunkPower ********************************************
**
** The Ensembl Sequence Adaptor handles sequences in larger chunks internally.
** Both, memory allocation and SQL database operations become more efficient,
** since requests for neighbouring regions are likely to be returned from the
** same block.
**
** 1 << 18 = 256 KiB, about the size of a BAC clone
**
******************************************************************************/

static const ajuint sequenceChunkPower = 18;




/* @conststatic sequenceadaptorCacheMaxBytes **********************************
**
** The maximum memory size in bytes the Ensembl Sequence Adaptor-internal
** Ensembl Cache can use.
**
** 1 << 26 = 64 MiB
**
******************************************************************************/

static const size_t sequenceadaptorCacheMaxBytes = 1 << 26;




/* @conststatic sequenceadaptorCacheMaxCount **********************************
**
** The mamximum number of sequence chunks the Ensembl Sequence Adaptor-internal
** Ensembl Cache can hold.
**
** 1 << 16 = 64 Ki
**
******************************************************************************/

static const ajuint sequenceadaptorCacheMaxCount = 1 << 16;




/* @conststatic sequenceadaptorCacheMaxSize ***********************************
**
** Maximum size of a sequence chunk to be allowed into the
** Ensembl Sequence Adaptor-internal Ensembl Cache.
**
******************************************************************************/

static const size_t sequenceadaptorCacheMaxSize = 0;




/* @conststatic sequenceadaptorCacheMaxLength *********************************
**
** Maximum length of a sequence request up to which it gets chunked and cached.
** Larger request are returned directly.
**
** 1 << 21 = 2 Mi, or about 8 BAC clones
**
******************************************************************************/

static const ajuint sequenceadaptorCacheMaxLength = 1 << 21;




/* ==================================================================== */
/* ======================== private variables ========================= */
/* ==================================================================== */




/* ==================================================================== */
/* ======================== private functions ========================= */
/* ==================================================================== */

static void* sequenceadaptorCacheReference(void* value);

static void sequenceadaptorCacheDelete(void** value);

static size_t sequenceadaptorCacheSize(const void* value);

static AjBool sequenceadaptorFetchCircularsliceSubStr(EnsPSequenceadaptor sa,
                                                      EnsPSlice slice,
                                                      ajint start,
                                                      ajint end,
                                                      ajint strand,
                                                      AjPStr* Psequence);




/* ==================================================================== */
/* ===================== All functions by section ===================== */
/* ==================================================================== */




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
** @cc CVS Revision: 1.48
** @cc CVS Tag: branch-ensembl-62
**
******************************************************************************/




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

static void* sequenceadaptorCacheReference(void* value)
{
    if(!value)
        return NULL;

    return (void*) ajStrNewRef((AjPStr) value);
}




/* @funcstatic sequenceadaptorCacheDelete *************************************
**
** Wrapper function to delete an Ensembl Sequence
** (i.e. an AJAX String) from an Ensembl Cache.
**
** @param [u] value [void**] AJAX String address
**
** @return [void]
** @@
******************************************************************************/

static void sequenceadaptorCacheDelete(void** value)
{
    if(!value)
        return;

    ajStrDel((AjPStr*) value);

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
** @return [size_t] Memory size in bytes or 0
** @@
******************************************************************************/

static size_t sequenceadaptorCacheSize(const void* value)
{
    size_t size = 0;

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
** @param [u] dba [EnsPDatabaseadaptor] Ensembl Database Adaptor
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
** memory allocated for an Ensembl Sequence Adaptor object.
**
** @fdata [EnsPSequenceadaptor]
**
** @nam3rule Del Destroy (free) an Ensembl Sequence Adaptor object
**
** @argrule * Psa [EnsPSequenceadaptor*] Ensembl Sequence Adaptor
**                                       object address
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
** @param [d] Psa [EnsPSequenceadaptor*] Ensembl Sequence Adaptor
**                                       object address
**
** @return [void]
** @@
******************************************************************************/

void ensSequenceadaptorDel(EnsPSequenceadaptor* Psa)
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
**
** @nam3rule Fetch Retrieve Sequence object(s)
** @nam4rule Seqregion Fetch by an Ensembl Sequence Region
** @nam4rule Slice Fetch by an Ensembl Slice
** @nam5rule All Fetch the complete sequence
** @nam5rule Sub Fetch a sub-sequence
** @nam6rule Str Fetch as an AJAX String
** @nam6rule Seq Fetch as an AJAX Sequence
**
** @argrule * sa [EnsPSequenceadaptor] Ensembl Sequence Adaptor
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
** @param [u] sa [EnsPSequenceadaptor] Ensembl Sequence Adaptor
** @param [r] sr [const EnsPSeqregion] Ensembl Sequence Region
** @param [wP] Psequence [AjPSeq*] AJAX Sequence address
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
** @@
** This function will only return biological sequence information for Ensembl
** Sequence Regions, which are in the sequence-level Coordinate System. All
** other Sequence Regions do not have sequence attached so that their sequence
** can only be fetched in the context of an Ensembl Slices, which is
** subsequently mapped to the sequence-level Coordinate System. See the
** description of the ensSequenceadaptorFetchSliceAllSeq function for
** further details.
******************************************************************************/

AjBool ensSequenceadaptorFetchSeqregionAllSeq(EnsPSequenceadaptor sa,
                                              const EnsPSeqregion sr,
                                              AjPSeq* Psequence)
{
    if(!sa)
        return ajFalse;

    if(!sr)
        return ajFalse;

    if(!Psequence)
        return ajFalse;

    return ensSequenceadaptorFetchSeqregionSubSeq(
        sa,
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
** @param [u] sa [EnsPSequenceadaptor] Ensembl Sequence Adaptor
** @param [r] sr [const EnsPSeqregion] Ensembl Sequence Region
** @param [u] Psequence [AjPStr*] Sequence address
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
** @@
** This function will only return biological sequence information for Ensembl
** Sequence Regions, which are in the sequence-level Coordinate System. All
** other Sequence Regions do not have sequence attached so that their sequence
** can only be fetched in the context of an Ensembl Slice, which is
** subsequently mapped to the sequence-level Coordinate System. See the
** description of the ensSequenceadaptorFetchSliceAllStr function for
** further details.
******************************************************************************/

AjBool ensSequenceadaptorFetchSeqregionAllStr(EnsPSequenceadaptor sa,
                                              const EnsPSeqregion sr,
                                              AjPStr* Psequence)
{
    if(!sa)
        return ajFalse;

    if(!sr)
        return ajFalse;

    if(!Psequence)
        return ajFalse;

    return ensSequenceadaptorFetchSeqregionSubStr(
        sa,
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
** @param [u] sa [EnsPSequenceadaptor] Ensembl Sequence Adaptor
** @param [r] sr [const EnsPSeqregion] Ensembl Sequence Region
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
** description of the ensSequenceadaptorFetchSliceAllSeq function for
** further details.
******************************************************************************/

AjBool ensSequenceadaptorFetchSeqregionSubSeq(EnsPSequenceadaptor sa,
                                              const EnsPSeqregion sr,
                                              ajuint start,
                                              ajuint length,
                                              AjPSeq* Psequence)
{
    AjPStr name     = NULL;
    AjPStr sequence = NULL;

    if(ajDebugTest("ensSequenceadaptorFetchSeqregionSubSeq"))
    {
        ajDebug("ensSequenceadaptorFetchSeqregionSubSeq\n"
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

    ensSequenceadaptorFetchSeqregionSubStr(sa, sr, start, length, &sequence);

    *Psequence = ajSeqNewNameS(sequence, name);

    ajSeqSetNuc(*Psequence);

    ajStrDel(&name);
    ajStrDel(&sequence);

    return ajTrue;
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
** @param [u] sa [EnsPSequenceadaptor] Ensembl Sequence Adaptor
** @param [r] sr [const EnsPSeqregion] Ensembl Sequence Region
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
** description of the ensSequenceadaptorFetchSliceSubStr function for
** further details.
******************************************************************************/

AjBool ensSequenceadaptorFetchSeqregionSubStr(EnsPSequenceadaptor sa,
                                              const EnsPSeqregion sr,
                                              ajuint start,
                                              ajuint length,
                                              AjPStr* Psequence)
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

    if(ajDebugTest("ensSequenceadaptorFetchSeqregionSubStr"))
    {
        ajDebug("ensSequenceadaptorFetchSeqregionSubStr\n"
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
    ** Reserve sequence space in larger blocks based on the requested length
    ** plus one position for the 'nul' string terminator.
    */

    if(*Psequence)
        ajStrAssignClear(Psequence);
    else
        *Psequence = ajStrNewRes((((length + 1) >> sequenceChunkPower) + 1)
                                 << sequenceChunkPower);

    if(length < sequenceadaptorCacheMaxLength)
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

            ensCacheFetch(sa->Cache, (void*) key, (void**) &chkstr);

            if(chkstr)
            {
                ajStrAppendS(&tmpstr, chkstr);

                ajStrDel(&chkstr);
            }
            else
            {
                /* Retrieve uncached sequence chunks. */

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

                    ensCacheStore(sa->Cache, (void*) key, (void**) &chkstr);

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




/* @func ensSequenceadaptorFetchSliceAllSeq ***********************************
**
** Fetch the sequence of an Ensembl Slice as an AJAX Sequence.
**
** The caller is responsible for deleting the AJAX Sequence.
**
** @param [u] sa [EnsPSequenceadaptor] Ensembl Sequence Adaptor
** @param [u] slice [EnsPSlice] Ensembl Slice
** @param [wP] Psequence [AjPSeq*] AJAX Sequence address
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
** @@
******************************************************************************/

AjBool ensSequenceadaptorFetchSliceAllSeq(EnsPSequenceadaptor sa,
                                          EnsPSlice slice,
                                          AjPSeq* Psequence)
{
    if(!sa)
        return ajFalse;

    if(!slice)
        return ajFalse;

    if(!Psequence)
        return ajFalse;

    return ensSequenceadaptorFetchSliceSubSeq(
        sa,
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
** @param [u] sa [EnsPSequenceadaptor] Ensembl Sequence Adaptor
** @param [u] slice [EnsPSlice] Ensembl Slice
** @param [u] Psequence [AjPStr*] Sequence address
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
** @@
******************************************************************************/

AjBool ensSequenceadaptorFetchSliceAllStr(EnsPSequenceadaptor sa,
                                          EnsPSlice slice,
                                          AjPStr* Psequence)
{
    if(!sa)
        return ajFalse;

    if(!slice)
        return ajFalse;

    if(!Psequence)
        return ajFalse;

    return ensSequenceadaptorFetchSliceSubStr(
        sa,
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
** @param [u] sa [EnsPSequenceadaptor] Ensembl Sequence Adaptor
** @param [u] slice [EnsPSlice] Ensembl Slice
** @param [r] start [ajint] Start coordinate
** @param [r] end [ajint] End coordinate
** @param [r] strand [ajint] Strand information
** @param [wP] Psequence [AjPSeq*] AJAX Sequence address
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
** @@
******************************************************************************/

AjBool ensSequenceadaptorFetchSliceSubSeq(EnsPSequenceadaptor sa,
                                          EnsPSlice slice,
                                          ajint start,
                                          ajint end,
                                          ajint strand,
                                          AjPSeq* Psequence)
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

    ensSequenceadaptorFetchSliceSubStr(sa,
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
** @param [u] sa [EnsPSequenceadaptor] Ensembl Sequence Adaptor
** @param [u] slice [EnsPSlice] Ensembl Slice
** @param [r] start [ajint] Start coordinate
** @param [r] end [ajint] End coordinate
** @param [r] strand [ajint] Strand information
** @param [u] Psequence [AjPStr*] Sequence address
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
** @@
******************************************************************************/

AjBool ensSequenceadaptorFetchSliceSubStr(EnsPSequenceadaptor sa,
                                          EnsPSlice slice,
                                          ajint start,
                                          ajint end,
                                          ajint strand,
                                          AjPStr* Psequence)
{
    const char* Ptr = NULL;

    register ajuint i = 0;

    ajint five   = 0;
    ajint three  = 0;
    ajint fshift = 0;
    ajint tshift = 0;

    ajuint padding = 0;
    ajuint total   = 0;

    AjBool circular = AJFALSE;
    AjBool debug    = AJFALSE;

    AjPList pslist = NULL;
    AjPList ses    = NULL;

    AjPStr tmpstr = NULL;

    EnsPCoordsystem seqlvlcs   = NULL;
    EnsPCoordsystemadaptor csa = NULL;

    EnsPProjectionsegment ps = NULL;

    EnsPSequenceedit se = NULL;

    EnsPSlice eslice     = NULL;
    EnsPSlice nslice     = NULL;
    EnsPSlice sslice     = NULL;
    EnsPSliceadaptor sla = NULL;

    debug = ajDebugTest("ensSequenceadaptorFetchSliceSubStr");

    if(debug)
    {
        ajDebug("ensSequenceadaptorFetchSliceSubStr\n"
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

    if(!Psequence)
        return ajFalse;

    if(!ensSliceIsCircular(slice, &circular))
        return ajFalse;

    if(circular == ajTrue)
    {
        if(start > end)
            return sequenceadaptorFetchCircularsliceSubStr(
                sa,
                slice,
                start,
                end,
                strand,
                Psequence);

        if(start < 0)
            start += ensSliceGetSeqregionLength(slice);

        if(end < 0)
            end += ensSliceGetSeqregionLength(slice);

        if(ensSliceGetStart(slice) > ensSliceGetEnd(slice))
            return sequenceadaptorFetchCircularsliceSubStr(
                sa,
                slice,
                ensSliceGetStart(slice),
                ensSliceGetEnd(slice),
                strand,
                Psequence);
    }
    else
    {
        if(start > end)
        {
            ajDebug("ensSequenceadaptorFetchSliceSubStr requires the start %d "
                    "to be less than or equal to the end %d coordinate for "
                    "linear Ensembl Slice objects.\n",
                    start, end);

            return ajFalse;
        }
    }

    if(!strand)
        strand = 1;

    /*
    ** Reserve sequence space in larger blocks based on the requested length
    ** plus one position for the 'nul' string terminator.
    */

    if(*Psequence)
        ajStrAssignClear(Psequence);
    else
        *Psequence = ajStrNewRes(
            (((ensSliceCalculateRegion(slice, start, end) + 1)
              >> sequenceChunkPower) + 1) << sequenceChunkPower);

    /*
    ** Get a new Slice that spans the exact region to retrieve DNA from.
    ** Only this short region of the Slice needs mapping into the
    ** sequence-level coordinate system.
    */

    /* Relative Slice coordinates range from 1 to length. */

    five = 1 - start;

    three = end - ensSliceCalculateLength(slice);

    if(five || three)
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

    sla = ensRegistryGetSliceadaptor(sa->Adaptor);

    pslist = ajListNew();

    ensSliceadaptorRetrieveNormalisedprojection(sla, eslice, pslist);

    if(!ajListGetLength(pslist))
        ajFatal("ensSequenceadaptorFetchSliceSubStr could not "
                "retrieve normalised Slices. Database contains incorrect "
                "information in the 'assembly_exception' table.\n");

    /*
    ** Call this method again with any Slice that was sym-linked to by this
    ** Slice.
    */

    ajListPeekFirst(pslist, (void**) &ps);

    if((ajListGetLength(pslist) != 1) ||
       (!ensSliceMatch(ensProjectionsegmentGetTargetSlice(ps), slice)))
    {
        tmpstr = ajStrNew();

        while(ajListPop(pslist, (void**) &ps))
        {
            nslice = ensProjectionsegmentGetTargetSlice(ps);

            ensSequenceadaptorFetchSliceAllStr(sa, nslice, &tmpstr);

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
    ** Clear the AJAX List of Ensembl Projection Segment objects resulting
    ** from the projection of the expanded Slice object to normalised
    ** Slice objects.
    */

    while(ajListPop(pslist, (void**) &ps))
        ensProjectionsegmentDel(&ps);

    /*
    ** Now, this Slice needs projecting onto the sequence-level Coordinate
    ** System even if it is already in this Coordinate System, because
    ** flanking gaps need trimming out the Slice is past the edges of
    ** the Sequence Region.
    */

    csa = ensRegistryGetCoordsystemadaptor(sa->Adaptor);

    ensCoordsystemadaptorFetchSeqlevel(csa, &seqlvlcs);

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

    while(ajListPop(pslist, (void**) &ps))
    {
        /*
        ** Check for gaps between Projection Segment objects and
        ** pad them with Ns.
        */

        padding = ensProjectionsegmentGetSourceStart(ps) - total - 1;

        if(padding)
        {
            ajStrAppendCountK(Psequence, 'N', padding);

            if(debug)
                ajDebug("ensSequenceadaptorFetchSliceSubStr got total %u "
                        "and Projection Segment source start %u, "
                        "therefore added %u N padding between.\n",
                        total,
                        ensProjectionsegmentGetSourceStart(ps),
                        padding);
        }

        sslice = ensProjectionsegmentGetTargetSlice(ps);

        ensSequenceadaptorFetchSeqregionSubStr(
            sa,
            ensSliceGetSeqregion(sslice),
            ensSliceGetStart(sslice),
            ensSliceCalculateLength(sslice),
            &tmpstr);

        if(ensSliceGetStrand(sslice) < 0)
            ajSeqstrReverse(&tmpstr);

        ajStrAppendS(Psequence, tmpstr);

        total = ensProjectionsegmentGetSourceEnd(ps);

        ensProjectionsegmentDel(&ps);
    }

    ajStrDel(&tmpstr);

    ajListFree(&pslist);

    /* Check for any remaining gaps at the end. */

    padding = ensSliceCalculateLength(slice) - total;

    if(padding)
    {
        ajStrAppendCountK(Psequence, 'N', padding);

        if(debug)
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

    for(i = 0, Ptr = ajStrGetPtr(*Psequence); (Ptr && *Ptr); i++, Ptr++)
        if(i == UINT_MAX)
            ajFatal("ensSequenceadaptorFetchSliceSubStr exeeded UINT_MAX.");

    padding = ensSliceCalculateLength(slice) - i;

    if(padding)
    {
        ajStrAppendCountK(Psequence, 'N', padding);

        if(debug)
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

    while(ajListPop(ses, (void**) &se))
    {
        /* Adjust Sequence Edit coordinates to the Sub-Slice. */

        ensSequenceeditApplyString(se,
                                   ensSliceGetStart(eslice) - 1,
                                   Psequence);

        ensSequenceeditDel(&se);
    }

    ajListFree(&ses);

    /* Reverse sequence if requested. */

    if(strand < 0)
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
** @param [u] sa [EnsPSequenceadaptor] Ensembl Sequence Adaptor
** @param [u] slice [EnsPSlice] Ensembl Slice
** @param [r] start [ajint] Start coordinate
** @param [r] end [ajint] End coordinate
** @param [r] strand [ajint] Strand information
** @param [u] Psequence [AjPStr*] Sequence address
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
** @@
******************************************************************************/

static AjBool sequenceadaptorFetchCircularsliceSubStr(EnsPSequenceadaptor sa,
                                                      EnsPSlice slice,
                                                      ajint start,
                                                      ajint end,
                                                      ajint strand,
                                                      AjPStr* Psequence)
{
    const char* Ptr = NULL;

    register ajuint i = 0;

    ajint mpoint = 0;

    ajint five   = 0;
    ajint three  = 0;
    ajint fshift = 0;
    ajint tshift = 0;

    ajuint padding = 0;
    ajuint total   = 0;

    AjBool circular = AJFALSE;
    AjBool debug    = AJFALSE;

    AjPList pslist = NULL;
    AjPList ses    = NULL;

    AjPStr sequence1 = NULL;
    AjPStr sequence2 = NULL;
    AjPStr tmpstr    = NULL;

    EnsPCoordsystem seqlvlcs   = NULL;
    EnsPCoordsystemadaptor csa = NULL;

    EnsPProjectionsegment ps = NULL;

    EnsPSequenceedit se = NULL;

    EnsPSlice eslice     = NULL;
    EnsPSlice nslice     = NULL;
    EnsPSlice sslice     = NULL;
    EnsPSliceadaptor sla = NULL;

    debug = ajDebugTest("sequenceadaptorFetchCircularsliceSubStr");

    if(debug)
    {
        ajDebug("sequenceadaptorFetchCircularsliceSubStr\n"
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

    if(!Psequence)
        return ajFalse;

    if(*Psequence)
        ajStrAssignClear(Psequence);
    else
        *Psequence = ajStrNewRes(
            (((ensSliceCalculateRegion(slice, start, end) + 1)
              >> sequenceChunkPower) + 1) << sequenceChunkPower);

    if(!ensSliceIsCircular(slice, &circular))
        return ajFalse;

    if((start > end) && (circular == ajTrue))
    {
        mpoint
            = ensSliceGetSeqregionLength(slice)
            - ensSliceGetStart(slice)
            + 1;

        sequence1 = ajStrNew();
        sequence2 = ajStrNew();

        sequenceadaptorFetchCircularsliceSubStr(
            sa,
            slice,
            1,
            mpoint,
            1,
            &sequence1);

        sequenceadaptorFetchCircularsliceSubStr(
            sa,
            slice,
            mpoint + 1,
            ensSliceCalculateLength(slice),
            1,
            &sequence1);

        if(ensSliceGetStrand(slice) >= 0)
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

    if(five || three)
    {
        if(ensSliceGetStrand(slice) >= 0)
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

    sla = ensRegistryGetSliceadaptor(sa->Adaptor);

    pslist = ajListNew();

    ensSliceadaptorRetrieveNormalisedprojection(sla, eslice, pslist);

    if(!ajListGetLength(pslist))
        ajFatal("sequenceadaptorFetchCircularsliceSubStr could not "
                "retrieve normalised Slices. Database contains incorrect "
                "information in the 'assembly_exception' table.\n");


    /*
    ** Call this method again with any Slice that was sym-linked to by this
    ** Slice.
    */

    ajListPeekFirst(pslist, (void**) &ps);

    if((ajListGetLength(pslist) != 1) ||
       (!ensSliceMatch(ensProjectionsegmentGetTargetSlice(ps), slice)))
    {
        tmpstr = ajStrNew();

        while(ajListPop(pslist, (void**) &ps))
        {
            nslice = ensProjectionsegmentGetTargetSlice(ps);

            ensSequenceadaptorFetchSliceAllStr(sa, nslice, &tmpstr);

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
    ** Clear the AJAX List of Ensembl Projection Segment objects resulting
    ** from the projection of the expanded Slice object to normalised
    ** Slice objects.
    */

    while(ajListPop(pslist, (void**) &ps))
        ensProjectionsegmentDel(&ps);

    /*
    ** Now, this Slice needs projecting onto the sequence-level Coordinate
    ** System even if it is already in this Coordinate System, because
    ** flanking gaps need trimming out the Slice is past the edges of
    ** the Sequence Region.
    */

    csa = ensRegistryGetCoordsystemadaptor(sa->Adaptor);

    ensCoordsystemadaptorFetchSeqlevel(csa, &seqlvlcs);

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

    while(ajListPop(pslist, (void**) &ps))
    {
        /*
        ** Check for gaps between Projection Segment objects and
        ** pad them with Ns.
        */

        padding = ensProjectionsegmentGetSourceStart(ps) - total - 1;

        if(padding)
        {
            ajStrAppendCountK(Psequence, 'N', padding);

            if(debug)
                ajDebug("sequenceadaptorFetchCircularsliceSubStr got total %u "
                        "and Projection Segment source start %u, "
                        "therefore added %u N padding between.\n",
                        total,
                        ensProjectionsegmentGetSourceStart(ps),
                        padding);
        }

        sslice = ensProjectionsegmentGetTargetSlice(ps);

        ensSequenceadaptorFetchSeqregionSubStr(
            sa,
            ensSliceGetSeqregion(sslice),
            ensSliceGetStart(sslice),
            ensSliceCalculateLength(sslice),
            &tmpstr);

        if(ensSliceGetStrand(sslice) < 0)
            ajSeqstrReverse(&tmpstr);

        ajStrAppendS(Psequence, tmpstr);

        total = ensProjectionsegmentGetSourceEnd(ps);

        ensProjectionsegmentDel(&ps);
    }

    /* Check for any remaining gaps at the end. */

    padding = ensSliceCalculateLength(slice) - total;

    if(padding)
    {
        ajStrAppendCountK(Psequence, 'N', padding);

        if(debug)
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

    for(i = 0, Ptr = ajStrGetPtr(*Psequence); (Ptr && *Ptr); i++, Ptr++)
        if(i == UINT_MAX)
            ajFatal("sequenceadaptorFetchCircularsliceSubStr exeeded "
                    "UINT_MAX.");

    padding = ensSliceCalculateLength(slice) - i;

    if(padding)
    {
        ajStrAppendCountK(Psequence, 'N', padding);

        if(debug)
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

    while(ajListPop(ses, (void**) &se))
    {
        /* Adjust Sequence Edit coordinates to the Sub-Slice. */

        ensSequenceeditApplyString(se,
                                   ensSliceGetStart(eslice) - 1,
                                   Psequence);

        ensSequenceeditDel(&se);
    }

    ajListFree(&ses);

    /* Reverse sequence if requested. */

    if(strand < 0)
        ajSeqstrReverse(Psequence);

    ensSliceDel(&eslice);

    return ajTrue;
}
