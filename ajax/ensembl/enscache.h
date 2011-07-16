
#ifndef ENSCACHE_H
#define ENSCACHE_H

/* ==================================================================== */
/* ========================== include files =========================== */
/* ==================================================================== */

#include "ajax.h"

AJ_BEGIN_DECLS




/* ==================================================================== */
/* ============================ constants ============================= */
/* ==================================================================== */

/* @const EnsECacheType *******************************************************
**
** Ensembl Cache Type enumeration.
**
******************************************************************************/

typedef enum EnsOCacheType
{
    ensECacheTypeNULL,
    ensECacheTypeNumeric,
    ensECacheTypeAlphaNumeric
} EnsECacheType;




/* ==================================================================== */
/* ========================== public data ============================= */
/* ==================================================================== */

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
** @attr Reference [(void**)] Object-specific referencing function
** @attr Delete [(void*)] Object-specific deletion function
** @attr Size [(size_t*)] Object-specific memory sizing function
** @attr Read [(void**)] Object-specific reading function
** @attr Write [(AjBool*)] Object-specific writing function
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
** @@
******************************************************************************/

typedef struct EnsSCache
{
    AjPStr Label;
    AjPList List;
    AjPTable Table;
    void* (*Reference)(void* value);
    void (*Delete)(void** Pvalue);
    size_t (*Size)(const void* value);
    void* (*Read)(const void* key);
    AjBool (*Write)(const void* value);
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
} EnsOCache;

#define EnsPCache EnsOCache*




/* ==================================================================== */
/* ======================= public functions =========================== */
/* ==================================================================== */

/*
** Prototype definitions
*/

/* Ensembl Cache */

EnsPCache ensCacheNew(const EnsECacheType type,
                      size_t maxbytes,
                      ajuint maxcount,
                      size_t maxsize,
                      void* Freference(void* value),
                      void Fdelete(void** Pvalue),
                      size_t Fsize(const void* value),
                      void* Fread(const void* key),
                      AjBool Fwrite(const void* value),
                      AjBool synchron,
                      const char* label);

void ensCacheDel(EnsPCache* Pcache);

AjBool ensCacheTrace(const EnsPCache cache, ajuint level);

AjBool ensCacheFetch(EnsPCache cache, void* key, void** Pvalue);

AjBool ensCacheRemove(EnsPCache cache, const void* key);

AjBool ensCacheStore(EnsPCache cache, void* key, void** Pvalue);

AjBool ensCacheSynchronise(EnsPCache cache);

/*
** End of prototype definitions
*/




AJ_END_DECLS

#endif /* !ENSCACHE_H */
