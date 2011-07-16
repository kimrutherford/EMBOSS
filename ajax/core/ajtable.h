#ifdef __cplusplus
extern "C"
{
#endif

#ifndef ajtable_h
#define ajtable_h

struct binding {
	struct binding *link;
	void *key;
        void *value;
};




/* @data AjPTable *************************************************************
**
** Hash table object. Tables are key/value pairs with a simple hash function
** to provide rapid searching for keys.
**
** Tables can hold any data type. Special functions are available for
** tables of AjPStr values, but these are in the ajstr library,
** and start with ajStrTable...
**
** In general, these functions are the same
** but with different hash and comparison functions used. Alternative
** function names are provided in all cases to save remembering which
** calls need special cases.
**
** @new ajTableNew Creates a table.
** @delete ajTableFree Deallocates and clears a table.
** @modify ajTablePut Adds or updates a value for a given key.
** @modify ajTableMap Calls a function for each key/value in a table.
** @modify ajTableRemove Removes a key/value pair from a table, and returns
**                    the value.
** @cast ajTableToarray Creates an array to hold each key value pair
**                     in pairs of array elements. The last element is null.
** @cast ajTableGet Returns the value for a given key.
** @cast ajTableLength Returns the number of keys in a table.
** @output ajTableTrace Writes debug messages to trace the contents of a table.
**
** @attr cmp [(ajint*)] Compare function(0 for match, -1 or +1 if not matched)
** @attr hash [(unsigned*)] Hash function
** @attr defdel [(void*)] Default key destructor, or NULL if none
** @attr keydel [(void*)] Key destructor, or NULL if not an object
** @attr valdel [(void*)] Value destructor, or NULL if not an object
** @attr buckets [struct binding**] Buckets
** @attr length [ajuint] Number of entries
** @attr timestamp [ajuint] Time stamp
** @attr size [ajuint] Size - number of hash buckets
** @attr Use [ajuint] Reference count
** @attr Type [AjEnum] Enumerated type
** @attr Padding [char[4]] Padding to alignment boundary
** @@
******************************************************************************/

typedef struct AjSTable {
    ajint(*cmp)(const void *x, const void *y);
    unsigned(*hash)(const void *key, unsigned hashsize);
    void(*defdel)(void **key);
    void(*keydel)(void **key);
    void(*valdel)(void **value);
    struct binding **buckets;
    ajuint length;
    ajuint timestamp;
    ajuint size;
    ajuint Use;
    AjEnum Type;
    char Padding[4];
} AjOTable;

#define AjPTable AjOTable*



/*
** Prototype definitions
*/

void           ajTableExit(void);
void           ajTableClear(AjPTable table);
void           ajTableClearDelete(AjPTable table);
void           ajTableDel(AjPTable* table);
void           ajTableDelKeydelValdel(AjPTable* table,
                                      void keydel(void** Pvalue),
                                      void valdel(void** Pvalue));
void           ajTableDelValdel(AjPTable* table,
                                  void valdel(void** Pvalue));
void           ajTableFree(AjPTable* table);

const void*    ajTableFetchV(const AjPTable table, const void *key);
void*          ajTableFetchmodV(const AjPTable table, const void *key);
void*          ajTableFetchmodTraceV(const AjPTable table, const void *key);

const ajint*   ajTableintFetch(const AjPTable table, const ajint* intkey);
ajint*         ajTableintFetchmod(AjPTable table, const ajint *key);

const ajlong*  ajTablelongFetch(const AjPTable table, const ajlong* longkey);
ajlong*        ajTablelongFetchmod(AjPTable table, const ajlong *key);

const ajuint*  ajTableuintFetch(const AjPTable table, const ajuint* uintkey);
ajuint*        ajTableuintFetchmod(AjPTable table, const ajuint *key);

const ajulong* ajTableulongFetch(const AjPTable table,
                                  const ajulong* ulongkey);
ajulong*       ajTableulongFetchmod(AjPTable table, const ajulong *key);

const void*    ajTableFetchC(const AjPTable table, const char* txtkey);
const void*    ajTableFetchS(const AjPTable table, const AjPStr key);
void*          ajTableFetchmodC(const AjPTable table, const char* txtkey);
void*          ajTableFetchmodS(const AjPTable table, const AjPStr key);

ajuint         ajTableGetLength(const AjPTable table);
ajuint         ajTableGetSize(const AjPTable table);
void           ajTableMap(AjPTable table,
                          void apply(const void *key, void **value, void *cl),
                          void *cl);
void           ajTableMapDel(AjPTable table,
                             void apply(void **key, void **value, void *cl),
                             void *cl);
AjBool         ajTableMergeAnd(AjPTable thys, AjPTable table);
AjBool         ajTableMergeEor(AjPTable thys, AjPTable table);
AjBool         ajTableMergeNot(AjPTable thys, AjPTable table);
AjBool         ajTableMergeOr(AjPTable thys, AjPTable table);
void*          ajTablePut(AjPTable table, void *key,
                          void *value);
AjBool         ajTablePutClean(AjPTable table, void *key, void *value,
                               void keydel(void** Pkey),
                               void valdel(void** Pvalue));
void*          ajTablePutTrace(AjPTable table, void *key,
                               void *value);

void*          ajTableRemove(AjPTable table, const void *key);
void*          ajTableRemoveKey(AjPTable table, const void *key,
                                void** truekey);
void           ajTableResizeCount(AjPTable table, ajuint size);

void           ajTableResizeHashsize(AjPTable table, ajuint hashsize);

ajuint         ajTableToarrayKeys(const AjPTable table,
                                  void*** keyarray);
ajuint         ajTableToarrayKeysValues(const AjPTable table,
                                        void*** keyarray, void*** valarray);
ajuint         ajTableToarrayValues(const AjPTable table,
                                    void*** valarray);


AjPTable       ajTablecharNew(ajuint hint);
AjPTable       ajTablecharNewCase(ajuint hint);
AjPTable       ajTablecharNewConst(ajuint hint);
AjPTable       ajTablecharNewCaseConst(ajuint hint);

void           ajTableTrace(const AjPTable table);
void           ajTablecharPrint(const AjPTable table);
void           ajTablestrPrint(const AjPTable table);
void           ajTablestrTrace(const AjPTable table);
void           ajTablestrTracekeys(const AjPTable table);

ajint          ajTablecharCmp(const void *x, const void *y);
ajint          ajTablecharCmpCase(const void *x, const void *y);
ajint          ajTableintCmp(const void *x, const void *y);
ajint          ajTableuintCmp(const void *x, const void *y);
ajint          ajTablelongCmp(const void *x, const void *y);
ajint          ajTableulongCmp(const void *x, const void *y);
ajint          ajTablestrCmp(const void *x, const void *y);
ajint          ajTablestrCmpCase(const void *x, const void *y);

unsigned       ajTablecharHash(const void *key, unsigned hashsize);
unsigned       ajTablecharHashCase(const void *key, unsigned hashsize);
unsigned       ajTableintHash(const void *key, unsigned hashsize);
unsigned       ajTablelongHash(const void *key, unsigned hashsize);
unsigned       ajTableuintHash(const void *key, unsigned hashsize);
unsigned       ajTableulongHash(const void *key, unsigned hashsize);
unsigned       ajTablestrHash(const void *key, unsigned hashsize);
unsigned       ajTablestrHashCase(const void *key, unsigned hashsize);

AjPTable       ajTableintNew(ajuint size);
AjPTable       ajTableintNewConst(ajuint size);
AjPTable       ajTablelongNew(ajuint size);
AjPTable       ajTablelongNewConst(ajuint size);
AjPTable       ajTableuintNew(ajuint size);
AjPTable       ajTableuintNewConst(ajuint size);
AjPTable       ajTableulongNew(ajuint size);
AjPTable       ajTableulongNewConst(ajuint size);
void           ajTableintFree(AjPTable* ptable);
void           ajTableintFreeKey(AjPTable* ptable);
void           ajTablelongFree(AjPTable* ptable);
void           ajTablelongFreeKey(AjPTable* ptable);
void           ajTableuintFree(AjPTable* ptable);
void           ajTableuintFreeKey(AjPTable* ptable);
void           ajTableulongFree(AjPTable* ptable);
void           ajTableulongFreeKey(AjPTable* ptable);
AjPTable       ajTablestrNew(ajuint size);
AjPTable       ajTablestrNewCase(ajuint size);
AjPTable       ajTablestrNewConst(ajuint size);
AjPTable       ajTablestrNewCaseConst(ajuint size);

void           ajTablestrFree(AjPTable* ptable);
void           ajTablestrFreeKey(AjPTable* ptable);

AjPTable       ajTableNew(ajuint size);
AjPTable       ajTableNewFunctionLen(ajuint size,
                                     ajint cmp(const void *x, const void *y),
                                     ajuint hash(const void *key,
                                                 ajuint hashsize),
                                     void keydel(void** key),
                                     void valdel(void** value));
const AjPStr   ajTablestrFetchkeyC(const AjPTable table, const char *key);
const AjPStr   ajTablestrFetchkeyS(const AjPTable table, const AjPStr key);

void           ajTableSetDestroy(AjPTable table,
                                 void keydel(void** key),
                                 void valdel(void** value));
void           ajTableSetDestroyboth(AjPTable table);
void           ajTableSetDestroykey(AjPTable table,
                                    void keydel(void** key));
void           ajTableSetDestroyvalue(AjPTable table,
                                      void valdel(void** value));

void           ajTableSettypeDefault(AjPTable table);
void           ajTableSettypeChar(AjPTable table);
void           ajTableSettypeCharCase(AjPTable table);
void           ajTableSettypeInt(AjPTable table);
void           ajTableSettypeLong(AjPTable table);
void           ajTableSettypeUint(AjPTable table);
void           ajTableSettypeUlong(AjPTable table);
void           ajTableSettypeString(AjPTable table);
void           ajTableSettypeStringCase(AjPTable table);
void           ajTableSettypeUser(AjPTable table,
                                  ajint cmp(const void *x, const void *y),
                                  ajuint hash(const void *key,
                                              ajuint hashsize));

AjBool         ajTableMatchC(const AjPTable table, const char *key);
AjBool         ajTableMatchS(const AjPTable table, const AjPStr key);
AjBool         ajTableMatchV(const AjPTable table, const void *key);

void*          ajTablestrFetchC(const AjPTable table, const char* txtkey);
void*          ajTablestrFetchS(const AjPTable table, const AjPStr key);
AjPStr*        ajTablestrFetchmod(AjPTable table, const AjPStr key);

/*
** End of prototype definitions
*/

__deprecated AjPTable   ajTableNewLen(ajuint size);
__deprecated AjPTable   ajTablestrNewCaseLen(ajuint size);
__deprecated AjPTable   ajTablestrNewLen(ajuint size);
__deprecated void*      ajTableFetch(const AjPTable table, const void *key);
__deprecated const void * ajTableFetchKey(const AjPTable table,
                                          const void *key);
__deprecated AjPTable   ajTablecharNewCaseLen(ajuint hint);
__deprecated const AjPStr ajTablestrFetch(const AjPTable table,
                                           const AjPStr key);
__deprecated ajuint     ajTableToarray(const AjPTable table,
                                       void*** keyarray, void*** valarray);
__deprecated ajint      ajTableLength(const AjPTable table);
__deprecated void*      ajTableGet(const AjPTable table, const void *key);
__deprecated ajint      ajStrTableCmp(const void *x, const void *y);
__deprecated ajint      ajStrTableCmpC(const void *x, const void *y);
__deprecated ajint      ajStrTableCmpCase(const void *x, const void *y);
__deprecated ajint      ajStrTableCmpCaseC(const void *x, const void *y);
__deprecated unsigned   ajStrTableHash(const void *key, unsigned hashsize);
__deprecated unsigned   ajStrTableHashC(const void *key, unsigned hashsize);
__deprecated unsigned   ajStrTableHashCase(const void *key, unsigned hashsize);
__deprecated unsigned   ajStrTableHashCaseC(const void *key, unsigned hashsize);
__deprecated AjPTable   ajStrTableNewC(ajuint hint);
__deprecated AjPTable   ajStrTableNewCase(ajuint hint);
__deprecated AjPTable   ajStrTableNewCaseC(ajuint hint);
__deprecated void       ajStrTablePrint(const AjPTable table);
__deprecated void       ajStrTablePrintC(const AjPTable table);
__deprecated void       ajStrTableTrace(const AjPTable table);

__deprecated AjPTable   ajTableNewL(ajuint size,
			ajint cmp(const void *x, const void *y),
			unsigned hash(const void *key, unsigned hashsize));
__deprecated const void* ajTableKey(const AjPTable table, const void *key);
__deprecated void       ajStrTableFree(AjPTable *table);
__deprecated AjPTable   ajStrTableNew(ajuint hint);
__deprecated void       ajStrTableFreeKey(AjPTable *table);
#endif

#ifdef __cplusplus
}
#endif
