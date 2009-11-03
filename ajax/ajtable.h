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
** @attr cmp [(ajint*)] Compare function (0 for match, -1 or +1 if not matched)
** @attr hash [(unsigned*)] Hash function
** @attr length [ajint] Number of entries
** @attr timestamp [unsigned] Time stamp
** @attr buckets [struct binding**] Buckets
** @attr size [ajuint] Size - number of hash buckets
** @attr Padding [char[4]] Padding to alignment boundary
** @@
******************************************************************************/

typedef struct AjSTable {
  ajint (*cmp)(const void *x, const void *y);
  unsigned (*hash)(const void *key, unsigned hashsize);
  ajint length;
  unsigned timestamp;
  struct binding **buckets;
  ajuint size;
  char Padding[4];
} AjOTable;

#define AjPTable AjOTable*




/*
** Prototype definitions
*/

void       ajTableExit(void);
void       ajTableFree (AjPTable* table);
void*      ajTableFetch  (const AjPTable table, const void *key);
ajuint     ajTableGetLength (const AjPTable table);
void       ajTableMap    (AjPTable table,
			  void apply(const void *key, void **value, void *cl),
			  void *cl);
void       ajTableMapDel (AjPTable table,
			  void apply(void **key, void **value, void *cl),
			  void *cl);
void*      ajTablePut   (AjPTable table, void *key,
			 void *value);
void*      ajTableRemove (AjPTable table, const void *key);
void*      ajTableRemoveKey(AjPTable table, const void *key, void** truekey);
ajuint     ajTableToarray (const AjPTable table,
			   void*** keyarray, void*** valarray);
void       ajTableTrace   (const AjPTable table);

ajint      ajTablecharCmp     (const void *x, const void *y);
void       ajTablecharPrint (const AjPTable table);
ajint      ajTablecharCmpCase (const void *x, const void *y);
unsigned   ajTablecharHash     (const void *key, unsigned hashsize);
unsigned   ajTablecharHashCase (const void *key, unsigned hashsize);
AjPTable   ajTablecharNew (void);
AjPTable   ajTablecharNewCase(void);
AjPTable   ajTablecharNewCaseLen (ajuint hint);
AjPTable   ajTablecharNewLen (ajuint hint);

void       ajTablestrPrint  (const AjPTable table);
void       ajTablestrTrace (const AjPTable table);
ajint      ajTablestrCmp      (const void *x, const void *y);
ajint      ajTablestrCmpCase  (const void *x, const void *y);
unsigned   ajTablestrHash      (const void *key, unsigned hashsize);
unsigned   ajTablestrHashCase  (const void *key, unsigned hashsize);
AjPTable   ajTablestrNew(void);
AjPTable   ajTablestrNewCase(void);
AjPTable   ajTablestrNewCaseLen(ajuint size);
AjPTable   ajTablestrNewLen(ajuint size);
void       ajTablestrFree(AjPTable* ptable);
void       ajTablestrFreeKey(AjPTable* ptable);

AjPTable   ajTableNewLen(ajuint size);
AjPTable   ajTableNewFunctionLen(ajuint size,
				 ajint cmp(const void *x, const void *y),
				 ajuint hash(const void *key, ajuint hashsize));
const void * ajTableFetchKey(const AjPTable table, const void *key);

/*
** End of prototype definitions
*/

__deprecated ajint      ajTableLength (const AjPTable table);
__deprecated void*      ajTableGet  (const AjPTable table, const void *key);
__deprecated ajint      ajStrTableCmp      (const void *x, const void *y);
__deprecated ajint      ajStrTableCmpC     (const void *x, const void *y);
__deprecated ajint      ajStrTableCmpCase  (const void *x, const void *y);
__deprecated ajint      ajStrTableCmpCaseC (const void *x, const void *y);
__deprecated unsigned   ajStrTableHash      (const void *key, unsigned hashsize);
__deprecated unsigned   ajStrTableHashC     (const void *key, unsigned hashsize);
__deprecated unsigned   ajStrTableHashCase  (const void *key, unsigned hashsize);
__deprecated unsigned   ajStrTableHashCaseC (const void *key, unsigned hashsize);
__deprecated AjPTable   ajStrTableNewC (ajuint hint);
__deprecated AjPTable   ajStrTableNewCase  (ajuint hint);
__deprecated AjPTable   ajStrTableNewCaseC (ajuint hint);
__deprecated void       ajStrTablePrint  (const AjPTable table);
__deprecated void       ajStrTablePrintC (const AjPTable table);
__deprecated void       ajStrTableTrace (const AjPTable table);

__deprecated AjPTable   ajTableNew (ajuint hint,
		       ajint cmp(const void *x, const void *y),
		       unsigned hash(const void *key, unsigned hashsize));
__deprecated AjPTable   ajTableNewL (ajuint size,
			ajint cmp(const void *x, const void *y),
			unsigned hash(const void *key, unsigned hashsize));
__deprecated const void* ajTableKey  (const AjPTable table, const void *key);
__deprecated void       ajStrTableFree (AjPTable *table);
__deprecated AjPTable   ajStrTableNew  (ajuint hint);
__deprecated void       ajStrTableFreeKey (AjPTable *table);
#endif

#ifdef __cplusplus
}
#endif
