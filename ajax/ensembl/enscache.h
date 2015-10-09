/* @include enscache **********************************************************
**
** Ensembl Cache functions
**
** @author Copyright (C) 1999 Ensembl Developers
** @author Copyright (C) 2006 Michael K. Schuster
** @version $Revision: 1.22 $
** @modified 2009 by Alan Bleasby for incorporation into EMBOSS core
** @modified $Date: 2012/03/04 14:30:36 $ by $Author: mks $
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

#ifndef ENSCACHE_H
#define ENSCACHE_H

/* ========================================================================= */
/* ============================= include files ============================= */
/* ========================================================================= */

#include "ajax.h"

AJ_BEGIN_DECLS




/* ========================================================================= */
/* =============================== constants =============================== */
/* ========================================================================= */

/* @enum EnsECacheType ********************************************************
**
** Ensembl Cache Type enumeration
**
** @value ensECacheTypeNULL Null
** @value ensECacheTypeNumeric Numeric
** @value ensECacheTypeAlphaNumeric Alphanumeric
** @@
******************************************************************************/

typedef enum EnsOCacheType
{
    ensECacheTypeNULL,
    ensECacheTypeNumeric,
    ensECacheTypeAlphaNumeric
} EnsECacheType;




/* ========================================================================= */
/* ============================== public data ============================== */
/* ========================================================================= */

/* @data EnsPCache ************************************************************
**
** Ensembl Cache.
**
** @alias EnsSCache
** @alias EnsOCache
**
** @attr Label [AjPStr] Cache label for statistics output
** @attr List [AjPList] AJAX List implementing LRU functionality
** @attr Table [AjPTable] AJAX Table implementing lookup functionality
** @attr Freference [void* function] Object-specific referencing function
** @attr Fdelete [void function] Object-specific deletion function
** @attr Fsize [size_t function] Object-specific memory sizing function
** @attr Fread [void* function] Object-specific reading function
** @attr Fwrite [AjBool function] Object-specific writing function
** @attr Type [EnsECacheType] Ensembl Cache type
** @attr Synchron [AjBool] ajTrue: Immediately write-back value data
**                         ajFalse: Write-back value data later
** @cc Memory limits
** @attr Bytes [size_t] Current number of cached bytes
** @attr MaxBytes [size_t] Maximum number of allowed bytes
** @attr MaxSize [size_t] Maximum memory size of an object
** @cc Counter limits
** @attr Count [ajuint] Current number of cached entry
** @attr MaxCount [ajuint] Maximum number of allowed entries
** @cc Performance statistics
** @attr Dropped [ajuint] Number of entries dropped by the LRU algorithm
** @attr Removed [ajuint] Number of entries explicitly removed
** @attr Stored [ajuint] Number of entries currently stored
** @attr Hit [ajuint] Number of cache hits
** @attr Miss [ajuint] Number of cache misses
** @attr Padding [ajuint] Padding to alignment boundary
** @@
******************************************************************************/

typedef struct EnsSCache
{
    AjPStr Label;
    AjPList List;
    AjPTable Table;
    void *(*Freference) (void *value);
    void (*Fdelete) (void **Pvalue);
    size_t (*Fsize) (const void *value);
    void *(*Fread) (const void *key);
    AjBool (*Fwrite) (const void *value);
    EnsECacheType Type;
    AjBool Synchron;
    size_t Bytes;
    size_t MaxBytes;
    size_t MaxSize;
    ajuint Count;
    ajuint MaxCount;
    ajuint Dropped;
    ajuint Removed;
    ajuint Stored;
    ajuint Hit;
    ajuint Miss;
    ajuint Padding;
} EnsOCache;

#define EnsPCache EnsOCache*




/* ========================================================================= */
/* =========================== public functions ============================ */
/* ========================================================================= */

/*
** Prototype definitions
*/

/* Ensembl Cache */

EnsPCache ensCacheNew(const EnsECacheType type,
                      size_t maxbytes,
                      ajuint maxcount,
                      size_t maxsize,
                      void *(*Freference) (void *value),
                      void (*Fdelete) (void **Pvalue),
                      size_t (*Fsize) (const void *value),
                      void *(*Fread) (const void *key),
                      AjBool (*Fwrite) (const void *value),
                      AjBool synchron,
                      const char *label);

void ensCacheDel(EnsPCache *Pcache);

AjBool ensCacheTrace(const EnsPCache cache, ajuint level);

AjBool ensCacheFetch(EnsPCache cache, void *key, void **Pvalue);

AjBool ensCacheRemove(EnsPCache cache, const void *key);

AjBool ensCacheStore(EnsPCache cache, void *key, void **Pvalue);

AjBool ensCacheSynchronise(EnsPCache cache);

/*
** End of prototype definitions
*/




AJ_END_DECLS

#endif /* !ENSCACHE_H */
