/******************************************************************************
** @source Ensembl Cache functions
**
** @author Copyright (C) 1999 Ensembl Developers
** @author Copyright (C) 2006 Michael K. Schuster
** @modified 2009 by Alan Bleasby for incorporation into EMBOSS core
** @version $Revision: 1.8 $
** @@
**
** Bio::EnsEMBL::Utils::Cache CVS Revision: 1.2
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




/* ==================================================================== */
/* ========================== private data ============================ */
/* ==================================================================== */




/* @datastatic CachePNode *****************************************************
**
** Ensembl Cache Node.
**
** @alias CacheSNode
** @alias CacheONode
**
** @attr Key [void*] Key data address
** @attr Value [void*] Value data address
** @attr Bytes [ajulong] Byte size of this node including key and value data
** @attr Dirty [AjBool] Flag to mark that value data has not been written back
** @@
******************************************************************************/

typedef struct CacheSNode
{
    void *Key;
    void *Value;
    ajulong Bytes;
    AjBool Dirty;
} CacheONode;

#define CachePNode CacheONode*




/* ==================================================================== */
/* ======================== private functions ========================= */
/* ==================================================================== */

static CachePNode cacheNodeNew(const EnsPCache cache, void* key, void* value);

static void cacheNodeDel(const EnsPCache cache, CachePNode* Pnode);

static AjBool cacheNodeInsert(EnsPCache cache, CachePNode node);

static AjBool cacheNodeRemove(EnsPCache cache, const CachePNode node);




/* @funcstatic cacheNodeNew ***************************************************
**
** Default constructor for an Ensembl Cache Node.
**
** The size of the Cache Node will be estimated according to the Ensembl Cache
** type with the function already provided at the Cache initialisation stage.
** This fuction will also reference value data to increment an internal usage
** counter and prevent deletion of value data while in the cache.
**
** @param [r] cache [const EnsPCache] Ensembl Cache
** @param [r] key [void*] Key data address
** @param [r] value [void*] Value data address
**
** @return [CachePNode] Ensembl Cache Node or NULL
** @@
******************************************************************************/

static CachePNode cacheNodeNew(const EnsPCache cache, void* key, void* value)
{
    ajuint *Puintkey = NULL;

    CachePNode node = NULL;

    if(!cache)
        return NULL;

    if(!key)
        return NULL;

    if(!value)
        return NULL;

    AJNEW0(node);

    /* Add the size of the Ensembl Cache Node itself. */

    node->Bytes += sizeof (CacheONode);

    switch(cache->Type)
    {
        case ensECacheTypeNumeric:

            /* Reference AJAX unsigned integer key data. */

            AJNEW0(Puintkey);

            *Puintkey = *((ajuint *) key);

            node->Key = (void *) Puintkey;

            /* Add the size of unsigned integer key data. */

            node->Bytes += sizeof (ajuint);

            break;

        case ensECacheTypeAlphaNumeric:

            /* Reference AJAX String key data. */

            node->Key = (void *) ajStrNewS((AjPStr) key);

            /* Add the size of AJAX String key data. */

            node->Bytes += sizeof (AjOStr);

            node->Bytes += ajStrGetRes((AjPStr) node->Key);

            break;

        default:

            ajWarn("cacheNodeNew got unexpected Cache type %d.\n",
                   cache->Type);
    }

    /* Reference the value data. */

    if(cache->Reference && value)
        node->Value = (*cache->Reference)(value);

    /* Calculate the size of the value data. */

    if(cache->Size && node->Value)
        node->Bytes += (*cache->Size)(node->Value);

    node->Dirty = ajFalse;

    return node;
}




/* @funcstatic cacheNodeDel ***************************************************
**
** Default destructor for an Ensembl Cache Node.
**
** @param [r] cache [const EnsPCache] Ensembl Cache
** @param [d] Pnode [CachePNode*] Ensembl Cache Node address
**
** @return [void]
** @@
******************************************************************************/

static void cacheNodeDel(const EnsPCache cache, CachePNode* Pnode)
{
    if(!cache)
        return;

    if(!Pnode)
        return;

    if(!*Pnode)
        return;

    switch(cache->Type)
    {
        case ensECacheTypeNumeric:

            /* Delete AJAX unsigned integer key data. */

            AJFREE((*Pnode)->Key);

            break;

        case ensECacheTypeAlphaNumeric:

            /* Delete AJAX String key data. */

            ajStrDel((AjPStr *) &((*Pnode)->Key));

            break;

        default:

            ajWarn("cacheNodeDel got unexpected Cache type %d.\n",
                   cache->Type);
    }

    /* Delete the value data. */

    if(cache->Delete && (*Pnode)->Value)
        (*cache->Delete)(&((*Pnode)->Value));

    AJFREE(*Pnode);

    *Pnode = NULL;

    return;
}




/* @funcstatic cacheNodeInsert ************************************************
**
** Insert an Ensembl Cache Node into an Ensembl Cache.
**
** @param [u] cache [EnsPCache] Ensembl Cache
** @param [u] node [CachePNode] Ensembl Cache Node
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
** @@
******************************************************************************/

static AjBool cacheNodeInsert(EnsPCache cache, CachePNode node)
{
    CachePNode old = NULL;

    if(!cache)
        return ajFalse;

    if(!node)
        return ajFalse;

    if(cache->MaxSize && (node->Bytes > cache->MaxSize))
        return ajFalse;

    /* Insert the node into the AJAX List. */

    ajListPushAppend(cache->List, (void *) node);

    /* Insert the node into the AJAX Table. */

    ajTablePut(cache->Table, node->Key, (void *) node);

    /* Update the cache statistics. */

    cache->Bytes += node->Bytes;

    cache->Count++;

    cache->Stored++;

    /* If the cache is too big, remove the top node(s). */

    while((cache->MaxBytes && (cache->Bytes > cache->MaxBytes)) ||
          (cache->MaxCount && (cache->Count > cache->MaxCount)))
    {
        /* Remove the top node from the AJAX List. */

        ajListPop(cache->List, (void **) &old);

        /* Remove the node also from the AJAX Table. */

        ajTableRemove(cache->Table, old->Key);

        /* Update the cache statistics. */

        cache->Bytes -= old->Bytes;

        cache->Count--;

        cache->Dropped++;

        /* Write changes of value data to disk if any. */

        if(cache->Write && old->Value && old->Dirty)
            (*cache->Write)(old->Value);

        /* Both, key and value data are deleted via cacheNodeDel. */

        cacheNodeDel(cache, &old);
    }

    return ajTrue;
}




/* @funcstatic cacheNodeRemove ************************************************
**
** Remove an Ensembl Cache Node from an Ensembl Cache.
**
** @param [u] cache [EnsPCache] Ensembl Cache
** @param [r] node [const CachePNode] Ensembl Cache Node
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
** @@
******************************************************************************/

static AjBool cacheNodeRemove(EnsPCache cache, const CachePNode node)
{
    AjIList iter = NULL;

    CachePNode lnode = NULL;

    if(!cache)
        return ajFalse;

    if(!node)
        return ajFalse;

    /* Remove the node from the AJAX List. */

    iter = ajListIterNew(cache->List);

    while(!ajListIterDone(iter))
    {
        lnode = (CachePNode) ajListIterGet(iter);

        if(lnode == node)
        {
            ajListIterRemove(iter);

            break;
        }
    }

    ajListIterDel(&iter);

    /* Remove the node from the AJAX Table. */

    ajTableRemove(cache->Table, node->Key);

    /* Update the cache statistics. */

    cache->Bytes -= node->Bytes;

    cache->Count--;

    cache->Removed++;

    return ajTrue;
}




/* @filesection enscache ******************************************************
**
** @nam1rule ens Function belongs to the AJAX Ensembl library
** @nam2rule Cache Ensembl Cache objects
**
******************************************************************************/




/* @datasection [EnsPCache] Ensembl Cache *************************************
**
** Functions for Ensembl Caches
**
**
******************************************************************************/




/* @section functions *********************************************************
**
** @fdata [EnsPCache]
** @fcategory misc
**
******************************************************************************/




/* @func ensCacheNew **********************************************************
**
** Default constructor for an Ensembl Cache.
**
** @param [r] type [EnsECacheType] Ensembl Cache type
**                 (ensECacheTypeNumeric or ensECacheTypeAlphaNumeric)
** @param [r] maxbytes [ajulong] Maximum number of bytes held in the cache
** @param [r] maxcount [ajuint] Maximum number of objects to be cached
** @param [r] maxsize [ajulong] Maximum size of an object to be cached
** @param [f] Freference [void* function] Object-specific referencing function
** @param [f] Fdelete [void function] Object-specific deletion function
** @param [f] Fsize [ajulong function] Object-specific memory sizing function
** @param [f] Fread [void* function] Object-specific reading function
** @param [f] Fwrite [AjBool function] Object-specific writing function
** @param [r] synchron [AjBool] ajTrue: Immediately write-back value data
**                              ajFalse: Write-back value data later
** @param [r] label [const char*] Cache label for statistics output
**
** @return [EnsPCache] Ensembl Cache or NULL
** @@
** The maximum size parameter should prevent the cache from purging too many
** objects when very large objects are inserted. If not set it defaults to
** a tenth of the maximum cache size.
**
** Object-specific functions are required to reference objects held in the
** cache or delete objects once purged from the cache, as well as memory sizing
** functions and object-specific read and write back functions.
******************************************************************************/

EnsPCache ensCacheNew(EnsECacheType type,
                      ajulong maxbytes,
                      ajuint maxcount,
                      ajulong maxsize,
                      void* Freference(void* value),
                      void Fdelete(void** value),
                      ajulong Fsize(const void* value),
                      void* Fread(const void* key),
                      AjBool Fwrite(const void* value),
                      AjBool synchron,
                      const char *label)
{
    AjBool debug = AJFALSE;

    EnsPCache cache = NULL;

    debug = ajDebugTest("ensCacheNew");

    if(debug)
        ajDebug("ensCacheNew\n"
                "  type %d\n"
                "  maxbytes %Lu\n"
                "  maxcount %u\n"
                "  maxsize %Lu\n"
                "  Freference %p\n"
                "  Fdelete %p\n"
                "  Fsize %p\n"
                "  Fread %p\n"
                "  Fwrite %p\n"
                "  synchron '%B'\n"
                "  label '%s'\n",
                type,
                maxbytes,
                maxcount,
                maxsize,
                Freference,
                Fdelete,
                Fsize,
                Fread,
                Fwrite,
                synchron,
                label);

    if((type < ensECacheTypeNumeric) || (type > ensECacheTypeAlphaNumeric))
        ajFatal("ensCacheNew requires a valid type.\n");

    if((!maxbytes) && (!maxcount))
        ajFatal("ensCacheNew requires either a "
                "maximum bytes or maximum count limit.\n");

    if(!maxsize)
        maxsize = maxbytes ? maxbytes / 10 + 1 : 0;

    if(maxbytes && (!maxsize))
        ajFatal("ensCacheNew requires a maximum size limit, "
                "when a maximum bytes limit is set.");

    /* TODO: Find and set a sensible value here! */

    if(debug)
        ajDebug("ensCacheNew maxbytes %Lu, maxcount %u, maxsize %Lu.\n",
                maxbytes, maxcount, maxsize);

    if(maxbytes && (maxbytes < 1000))
        ajFatal("ensCacheNew cannot set a maximum bytes limit (%Lu) under "
                "1000, as each Cache Node requires %Lu bytes alone.",
                maxbytes, sizeof (CachePNode));

    /* TODO: Find and set a sensible value here! */

    if(maxsize && (maxsize < 3))
        ajFatal("ensCacheNew cannot set a maximum size limit (%Lu) under "
                "3 bytes. maximum bytes %Lu maximum count %u.",
                maxsize, maxbytes, maxcount);

    /*
    ** Pointers to functions for automatic reading of data not yet in the
    ** cache and writing of data modified in cache are not mandatory.
    ** If not specified the cache will simply lack this functionality.
    ** However, the specification of a function deleting stale cache entries
    ** and a function calculating the size of value data are required.
    */

    if(!(void*)Freference)
        ajFatal("ensCacheNew requires a referencing function.");

    if(!(void*)Fdelete)
        ajFatal("ensCacheNew requires a deletion function.");

    if(maxsize && (!(void*)Fsize))
        ajFatal("ensCacheNew requires a memory sizing function "
                "when a maximum size limit has been defined.");

    if(!label)
        ajFatal("ensCacheNew requires a label.");

    AJNEW0(cache);

    cache->Label = ajStrNewC(label);
    cache->List  = ajListNew();

    switch(type)
    {
        case ensECacheTypeNumeric:

            cache->Table = ajTableNewFunctionLen(0,
                                                 ensTableCmpUint,
                                                 ensTableHashUint);

            break;

        case ensECacheTypeAlphaNumeric:

            cache->Table = ajTablestrNewLen(0);

            break;

        default:
            ajWarn("ensCacheNew got unexpected Cache type %d.\n",
                   cache->Type);
    }

    cache->Reference = Freference;
    cache->Delete    = Fdelete;
    cache->Size      = Fsize;
    cache->Read      = Fread;
    cache->Write     = Fwrite;
    cache->Type      = type;
    cache->Synchron  = synchron;
    cache->MaxBytes  = maxbytes;
    cache->MaxCount  = maxcount;
    cache->MaxSize   = maxsize;
    cache->Bytes     = 0;
    cache->Count     = 0;
    cache->Dropped   = 0;
    cache->Removed   = 0;
    cache->Stored    = 0;
    cache->Hit       = 0;
    cache->Miss      = 0;

    return cache;
}




/* @func ensCacheDel **********************************************************
**
** Default destructor for an Ensembl Cache.
**
** @param [u] Pcache [EnsPCache*] Ensembl Cache address
**
** @return [void]
** @@
** Value data in Cache Nodes that have not been synchronised are written-back.
** Cache flags are reset for value data before the value data is deleted.
** After deletion of all Cache Nodes a summary statistics is printed and the
** Ensembl Cache is destroyed.
******************************************************************************/

void ensCacheDel(EnsPCache* Pcache)
{
    AjBool debug = AJFALSE;

    EnsPCache pthis = NULL;

    CachePNode node = NULL;

    if(!Pcache)
        return;

    if(!*Pcache)
        return;

    debug = ajDebugTest("ensCacheDel");

    if(debug)
        ajDebug("ensCacheDel\n"
                "  *Pcache %p\n",
                *Pcache);

    pthis = *Pcache;

    /* Remove nodes from the AJAX List. */

    while(ajListPop(pthis->List, (void **) &node))
    {
        /* Remove the node from the AJAX Table. */

        ajTableRemove(pthis->Table, node->Key);

        /* Update the cache statistics. */

        pthis->Count--;

        pthis->Bytes -= node->Bytes;

        /* Write changes of value data to disk if any. */

        if(pthis->Write && node->Value && node->Dirty)
            (*pthis->Write)(node->Value);

        /* Both, key and value data are deleted via cacheNodeDel. */

        cacheNodeDel(pthis, &node);
    }

    if(debug)
        ensCacheTrace(pthis, 1);

    ajStrDel(&pthis->Label);

    ajListFree(&pthis->List);

    ajTableFree(&pthis->Table);

    AJFREE(pthis);

    *Pcache = NULL;

    return;
}




/* @func ensCacheFetch ********************************************************
**
** Fetch a value from an Ensembl Cache via a key. If the value is not already
** in the cache it will be read by the function provided at the Cache
** initialisation stage.
**
** The caller is resposible for deleting the returned object.
**
** @param [u] cache [EnsPCache] Ensembl Cache
** @param [r] key [void*] Key data address
**
** @return [void*] Value data address or NULL
** @@
******************************************************************************/

void* ensCacheFetch(EnsPCache cache, void *key)
{
    void *value = NULL;

    AjIList iter = NULL;

    CachePNode lnode = NULL;
    CachePNode tnode = NULL;

    if(!cache)
        return NULL;

    if(!key)
        return NULL;

    tnode = (CachePNode) ajTableFetch(cache->Table, key);

    if(tnode)
    {
        cache->Hit++;

        /* Move the node to the end of the list. */

        iter = ajListIterNew(cache->List);

        while(!ajListIterDone(iter))
        {
            lnode = (CachePNode) ajListIterGet(iter);

            if(lnode == tnode)
            {
                ajListIterRemove(iter);

                ajListPushAppend(cache->List, (void *) lnode);

                break;
            }
        }

        ajListIterDel(&iter);

        /*
        ** Reference the object when returned by the cache so that external
        ** code has to delete it irrespectively whether it was read from the
        ** cache or instantiated by the cache->Read function.
        */

        if(cache->Reference && tnode->Value)
            value = (*cache->Reference)(tnode->Value);
    }
    else
    {
        cache->Miss++;

        if(cache->Read)
        {
            value = (*cache->Read)(key);

            if(value)
            {
                tnode = cacheNodeNew(cache, key, value);

                if(!cacheNodeInsert(cache, tnode))
                    cacheNodeDel(cache, &tnode);
            }
        }
    }

    return value;
}




/* @func ensCacheStore ********************************************************
**
** Insert value data into an Ensembl Cache under key data.
**
** @param [u] cache [EnsPCache] Ensembl Cache
** @param [r] key [void*] Key data address
** @param [r] value [void**] Value data address
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
** @@
******************************************************************************/

AjBool ensCacheStore(EnsPCache cache, void* key, void** value)
{
    CachePNode node = NULL;

    if(!cache)
        return ajFalse;

    if(!key)
        return ajFalse;

    if(!value)
        return ajFalse;

    /* Is a node already cached under this key? */

    node = (CachePNode) ajTableFetch(cache->Table, key);

    if(node)
    {
        /*
        ** Delete the Object passed in and increase the reference counter
        ** of the cached Object before assigning it.
        */

        (*cache->Delete)(value);

        *value = (*cache->Reference)(node->Value);
    }
    else
    {
        node = cacheNodeNew(cache, key, *value);

        if(cacheNodeInsert(cache, node))
        {
            if(cache->Synchron)
            {
                if(cache->Write && node->Value)
                    (*cache->Write)(node->Value);

                node->Dirty = ajFalse;
            }
            else
                node->Dirty = ajTrue;
        }
        else
        {
            if(cache->Write && node->Value)
                (*cache->Write)(node->Value);

            cacheNodeDel(cache, &node);
        }
    }

    return ajTrue;
}




/* @func ensCacheRemove *******************************************************
**
** Remove value data from an Ensembl Cache via key data.
**
** @param [u] cache [EnsPCache] Ensembl Cache
** @param [r] key [const void*] Key data
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
** @@
******************************************************************************/

AjBool ensCacheRemove(EnsPCache cache, const void* key)
{
    CachePNode node = NULL;

    if(!cache)
        return ajFalse;

    if(!key)
        return ajFalse;

    node = (CachePNode) ajTableFetch(cache->Table, key);

    if(node)
    {
        cacheNodeRemove(cache, node);

        /* Both, key and value data are deleted via cacheNodeDel. */

        cacheNodeDel(cache, &node);
    }

    return ajTrue;
}




/* @func ensCacheSynchronise **************************************************
**
** Synchronise an Ensembl Cache by writing-back all value data that have not
** been written before.
**
** @param [u] cache [EnsPCache] Ensembl Cache
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
** @@
******************************************************************************/

AjBool ensCacheSynchronise(EnsPCache cache)
{
    AjIList iter = NULL;

    CachePNode node = NULL;

    if(!cache)
        return ajFalse;

    iter = ajListIterNew(cache->List);

    while(!ajListIterDone(iter))
    {
        node = (CachePNode) ajListIterGet(iter);

        if(cache->Write && node->Value && node->Dirty)
        {
            (*cache->Write)(node->Value);

            node->Dirty = ajFalse;
        }
    }

    ajListIterDel(&iter);

    return ajTrue;
}




/* @func ensCacheTrace ********************************************************
**
** Writes debug messages to trace the contents of a cache.
**
** @param [r] cache [const EnsPCache] Ensembl Cache
** @param [r] level [ajuint] Indentation level
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
** @@
******************************************************************************/

AjBool ensCacheTrace(const EnsPCache cache, ajuint level)
{
    double ratio = 0.0;

    AjPStr indent = NULL;

    if(!cache)
        return ajFalse;

    indent = ajStrNew();

    ajStrAppendCountK(&indent, ' ', level * 2);

    if(cache->Hit || cache->Miss)
        ratio = (double) cache->Hit /
            ((double) cache->Hit + (double) cache->Miss);

    ajDebug("%SensCache trace %p\n"
            "%S  Label '%S'\n"
            "%S  List %p length: %u\n"
            "%S  Table %p length: %u\n"
            "%S  Type %d\n"
            "%S  Synchron %B\n"
            "%S  MaxBytes %Lu\n"
            "%S  MaxCount %u\n"
            "%S  MaxSize %Lu\n"
            "%S  Bytes %Lu\n"
            "%S  Count %u\n"
            "%S  Dropped %u\n"
            "%S  Removed %u\n"
            "%S  Stored %u\n"
            "%S  Hit %u\n"
            "%S  Miss %u\n"
            "%S  Hit/(Hit + Miss) %f\n",
            indent, cache,
            indent, cache->Label,
            indent, cache->List, ajListGetLength(cache->List),
            indent, cache->Table, ajTableGetLength(cache->Table),
            indent, cache->Type,
            indent, cache->Synchron,
            indent, cache->MaxBytes,
            indent, cache->MaxCount,
            indent, cache->MaxSize,
            indent, cache->Bytes,
            indent, cache->Count,
            indent, cache->Dropped,
            indent, cache->Removed,
            indent, cache->Stored,
            indent, cache->Hit,
            indent, cache->Miss,
            indent, ratio);

    ajStrDel(&indent);

    return ajTrue;
}
