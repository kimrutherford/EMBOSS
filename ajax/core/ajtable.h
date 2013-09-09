/* @include ajtable ***********************************************************
**
** AJAX table functions
**
** Hash table functions.
**
** @author Copyright (C) 1998 Ian Longden
** @version $Revision: 1.40 $
** @modified 2011 pmr Auto-resizing, destructors, table merges
** @modified $Date: 2013/02/17 13:39:55 $ by $Author: mks $
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

#ifndef AJTABLE_H
#define AJTABLE_H

/* ========================================================================= */
/* ============================= include files ============================= */
/* ========================================================================= */

#include "ajdefine.h"
#include "ajstr.h"

AJ_BEGIN_DECLS




/* ========================================================================= */
/* =============================== constants =============================== */
/* ========================================================================= */




/* @enum AjETableType *********************************************************
**
** AJAX Table Type enumeration
**
** @value ajETableTypeUnknown no type set
** @value ajETableTypeChar    char* type
** @value ajETableTypeStr     AJAX String (AjPStr) type
** @value ajETableTypeInt     AJAX integer (ajint) type
** @value ajETableTypeUint    AJAX unsigned integer (ajuint) type
** @value ajETableTypeLong    AJAX long integer (ajlong) type
** @value ajETableTypeUlong   AJAX unsigned long integer (ajulong) type
** @value ajETableTypeUser    user-defined type
** @value ajETableTypeMax     beyond last defined value
**
******************************************************************************/

typedef enum AjOTableType
{
    ajETableTypeUnknown,
    ajETableTypeChar,
    ajETableTypeStr,
    ajETableTypeInt,
    ajETableTypeUint,
    ajETableTypeLong,
    ajETableTypeUlong,
    ajETableTypeUser,
    ajETableTypeMax
} AjETableType;




/* ========================================================================= */
/* ============================== public data ============================== */
/* ========================================================================= */




/* @data AjPTableNode *********************************************************
**
** AJAX Table Node object.
**
** @attr Link [struct AjSTableNode*] Link top next AJAX Table Node
** @attr Key [void*] Key data
** @attr Value [void*] Value data
** @@
******************************************************************************/

typedef struct AjSTableNode {
    struct AjSTableNode* Link;
    void* Key;
    void* Value;
} AjOTableNode;

#define AjPTableNode AjOTableNode*




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
** @attr Fcmp [ajint function] Key compare function
**                       (0 for match, -1 or +1 if not matched)
** @attr Fhash [ajulong function] Hash function
** @attr Fkeydel [void function] Key destructor, or NULL if not an object
** @attr Fvaldel [void function] Value destructor, or NULL if not an object
** @attr Buckets [AjPTableNode*] Buckets of AJAX Table Node objects
** @attr Size [ajulong] Size - number of hash buckets
** @attr Length [ajulong] Number of entries
** @attr Timestamp [ajuint] Time stamp
** @attr Use [ajuint] Reference count
** @attr Padding [ajuint] Padding to alignment boundary
** @attr Type [AjETableType] AJAX Table Type enumeration
** @@
******************************************************************************/

typedef struct AjSTable
{
    ajint (*Fcmp)(const void* key1, const void* key2);
    ajulong (*Fhash)(const void* key, ajulong hashsize);
    void (*Fkeydel)(void** Pkey);
    void (*Fvaldel)(void** Pvalue);
    AjPTableNode* Buckets;
    ajulong Size;
    ajulong Length;
    ajuint Timestamp;
    ajuint Use;
    ajuint Padding;
    AjETableType Type;
} AjOTable;

#define AjPTable AjOTable*




/* ========================================================================= */
/* =========================== public functions ============================ */
/* ========================================================================= */




/*
** Prototype definitions
*/

void           ajTableExit(void);
void           ajTableClear(AjPTable table);
void           ajTableClearDelete(AjPTable table);
void           ajTableDel(AjPTable* table);
void           ajTableDelKeydelValdel(AjPTable* table,
                                      void (*keydel)(void** Pvalue),
                                      void (*valdel)(void** Pvalue));
void           ajTableDelValdel(AjPTable* table,
                                void (*valdel)(void** Pvalue));
void           ajTableFree(AjPTable* table);

const void*    ajTableFetchV(const AjPTable table, const void* key);
void*          ajTableFetchmodV(const AjPTable table, const void* key);
void*          ajTableFetchmodTraceV(const AjPTable table, const void* key);

const ajint*   ajTableintFetch(const AjPTable table, const ajint* intkey);
ajint*         ajTableintFetchmod(AjPTable table, const ajint* key);

const ajlong*  ajTablelongFetch(const AjPTable table, const ajlong* longkey);
ajlong*        ajTablelongFetchmod(AjPTable table, const ajlong* key);

const ajuint*  ajTableuintFetch(const AjPTable table, const ajuint* uintkey);
ajuint*        ajTableuintFetchmod(AjPTable table, const ajuint* key);

const ajulong* ajTableulongFetch(const AjPTable table,
                                 const ajulong* ulongkey);
ajulong*       ajTableulongFetchmod(AjPTable table, const ajulong* key);

const void*    ajTableFetchC(const AjPTable table, const char* txtkey);
const void*    ajTableFetchS(const AjPTable table, const AjPStr key);
void*          ajTableFetchmodC(const AjPTable table, const char* txtkey);
void*          ajTableFetchmodS(const AjPTable table, const AjPStr key);

ajulong        ajTableGetLength(const AjPTable table);
ajulong        ajTableGetSize(const AjPTable table);
void           ajTableMap(AjPTable table,
                          void (*apply)(const void* key,
                                        void** Pvalue,
                                        void* cl),
                          void* cl);
void           ajTableMapDel(AjPTable table,
                             void (*apply)(void** Pkey,
                                           void** Pvalue,
                                           void* cl),
                             void* cl);
AjBool         ajTableMergeAnd(AjPTable thys, AjPTable table);
AjBool         ajTableMergeEor(AjPTable thys, AjPTable table);
AjBool         ajTableMergeNot(AjPTable thys, AjPTable table);
AjBool         ajTableMergeOr(AjPTable thys, AjPTable table);
void*          ajTablePut(AjPTable table, void* key, void* value);
AjBool         ajTablePutClean(AjPTable table, void* key, void* value,
                               void (*keydel)(void** Pkey),
                               void (*valdel)(void** Pvalue));
void*          ajTablePutTrace(AjPTable table, void* key,
                               void* value);

void*          ajTableRemove(AjPTable table, const void* key);
void*          ajTableRemoveKey(AjPTable table, const void* key,
                                void** truekey);
void           ajTableResizeCount(AjPTable table, ajulong size);

void           ajTableResizeHashsize(AjPTable table, ajulong hashsize);

ajulong        ajTableToarrayKeys(const AjPTable table,
                                  void*** keyarray);
ajulong        ajTableToarrayKeysValues(const AjPTable table,
                                        void*** keyarray, void*** valarray);
ajulong        ajTableToarrayValues(const AjPTable table,
                                    void*** valarray);


AjPTable       ajTablecharNew(ajulong hint);
AjPTable       ajTablecharNewCase(ajulong hint);
AjPTable       ajTablecharNewConst(ajulong hint);
AjPTable       ajTablecharNewCaseConst(ajulong hint);

void           ajTableTrace(const AjPTable table);
void           ajTablecharPrint(const AjPTable table);
void           ajTablestrPrint(const AjPTable table);
void           ajTablestrTrace(const AjPTable table);
void           ajTablestrTracekeys(const AjPTable table);

ajint          ajTablecharCmp(const void* key1, const void* key2);
ajint          ajTablecharCmpCase(const void* key1, const void* key2);
ajint          ajTableintCmp(const void* key1, const void* key2);
ajint          ajTableuintCmp(const void* key1, const void* key2);
ajint          ajTablelongCmp(const void* key1, const void* key2);
ajint          ajTableulongCmp(const void* key1, const void* key2);
ajint          ajTablestrCmp(const void* key1, const void* key2);
ajint          ajTablestrCmpCase(const void* key1, const void* key2);

ajulong        ajTablecharHash(const void* key, ajulong hashsize);
ajulong        ajTablecharHashCase(const void* key, ajulong hashsize);
ajulong        ajTableintHash(const void* key, ajulong hashsize);
ajulong        ajTablelongHash(const void* key, ajulong hashsize);
ajulong        ajTableuintHash(const void* key, ajulong hashsize);
ajulong        ajTableulongHash(const void* key, ajulong hashsize);
ajulong        ajTablestrHash(const void* key, ajulong hashsize);
ajulong        ajTablestrHashCase(const void* key, ajulong hashsize);

AjPTable       ajTableintNew(ajulong size);
AjPTable       ajTableintNewConst(ajulong size);
AjPTable       ajTablelongNew(ajulong size);
AjPTable       ajTablelongNewConst(ajulong size);
AjPTable       ajTableuintNew(ajulong size);
AjPTable       ajTableuintNewConst(ajulong size);
AjPTable       ajTableulongNew(ajulong size);
AjPTable       ajTableulongNewConst(ajulong size);
void           ajTableintFree(AjPTable* ptable);
void           ajTableintFreeKey(AjPTable* ptable);
void           ajTablelongFree(AjPTable* ptable);
void           ajTablelongFreeKey(AjPTable* ptable);
void           ajTableuintFree(AjPTable* ptable);
void           ajTableuintFreeKey(AjPTable* ptable);
void           ajTableulongFree(AjPTable* ptable);
void           ajTableulongFreeKey(AjPTable* ptable);
AjPTable       ajTablestrNew(ajulong size);
AjPTable       ajTablestrNewCase(ajulong size);
AjPTable       ajTablestrNewConst(ajulong size);
AjPTable       ajTablestrNewCaseConst(ajulong size);

void           ajTablestrFree(AjPTable* ptable);
void           ajTablestrFreeKey(AjPTable* ptable);

AjPTable       ajTableNew(ajulong size);
AjPTable       ajTableNewFunctionLen(ajulong size,
                                     ajint (*cmp)(const void* key1,
                                                  const void* key2),
                                     ajulong (*hash)(const void* key,
                                                    ajulong hashsize),
                                     void (*keydel)(void** Pkey),
                                     void (*valdel)(void** Pvalue));
const AjPStr   ajTablestrFetchkeyC(const AjPTable table, const char* key);
const AjPStr   ajTablestrFetchkeyS(const AjPTable table, const AjPStr key);

void           ajTableSetDestroy(AjPTable table,
                                 void (*keydel)(void** Pkey),
                                 void (*valdel)(void** Pvalue));
void           ajTableSetDestroyboth(AjPTable table);
void           ajTableSetDestroykey(AjPTable table,
                                    void (*keydel)(void** Pkey));
void           ajTableSetDestroyvalue(AjPTable table,
                                      void (*valdel)(void** Pvalue));

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
                                  ajint (*cmp)(const void* key1,
                                               const void* key2),
                                  ajulong (*hash)(const void* key,
                                                  ajulong hashsize));

AjBool         ajTableMatchC(const AjPTable table, const char* key);
AjBool         ajTableMatchS(const AjPTable table, const AjPStr key);
AjBool         ajTableMatchV(const AjPTable table, const void* key);

void*          ajTablestrFetchC(const AjPTable table, const char* txtkey);
void*          ajTablestrFetchS(const AjPTable table, const AjPStr key);
AjPStr*        ajTablestrFetchmod(AjPTable table, const AjPStr key);

/*
** End of prototype definitions
*/




#ifdef AJ_COMPILE_DEPRECATED_BOOK
#endif /* AJ_COMPILE_DEPRECATED_BOOK */

#ifdef AJ_COMPILE_DEPRECATED

__deprecated AjPTable   ajTableNewLen(ajuint size);
__deprecated AjPTable   ajTablestrNewCaseLen(ajuint size);
__deprecated AjPTable   ajTablestrNewLen(ajuint size);
__deprecated void*      ajTableFetch(const AjPTable table, const void* key);
__deprecated const void* ajTableFetchKey(const AjPTable table,
                                         const void* key);
__deprecated AjPTable   ajTablecharNewCaseLen(ajuint hint);
__deprecated const AjPStr ajTablestrFetch(const AjPTable table,
                                          const AjPStr key);
__deprecated ajuint     ajTableToarray(const AjPTable table,
                                       void*** keyarray, void*** valarray);
__deprecated ajint      ajTableLength(const AjPTable table);
__deprecated void*      ajTableGet(const AjPTable table, const void* key);
__deprecated ajint      ajStrTableCmp(const void* key1, const void* key2);
__deprecated ajint      ajStrTableCmpC(const void* key1, const void* key2);
__deprecated ajint      ajStrTableCmpCase(const void* key1, const void* key2);
__deprecated ajint      ajStrTableCmpCaseC(const void* key1, const void* key2);
__deprecated ajuint     ajStrTableHash(const void* key, ajuint hashsize);
__deprecated ajuint     ajStrTableHashC(const void* key, ajuint hashsize);
__deprecated ajuint     ajStrTableHashCase(const void* key, ajuint hashsize);
__deprecated ajuint     ajStrTableHashCaseC(const void* key, ajuint hashsize);
__deprecated AjPTable   ajStrTableNewC(ajuint hint);
__deprecated AjPTable   ajStrTableNewCase(ajuint hint);
__deprecated AjPTable   ajStrTableNewCaseC(ajuint hint);
__deprecated void       ajStrTablePrint(const AjPTable table);
__deprecated void       ajStrTablePrintC(const AjPTable table);
__deprecated void       ajStrTableTrace(const AjPTable table);

__deprecated AjPTable   ajTableNewL(ajuint size,
                                    ajint (*cmp)(const void* key1,
                                                 const void* key2),
                                    ajuint (*hash)(const void* key,
                                                   ajuint hashsize));
__deprecated const void* ajTableKey(const AjPTable table, const void* key);
__deprecated void       ajStrTableFree(AjPTable* table);
__deprecated AjPTable   ajStrTableNew(ajuint hint);
__deprecated void       ajStrTableFreeKey(AjPTable* table);

#endif /* AJ_COMPILE_DEPRECATED */




AJ_END_DECLS

#endif /* !AJTABLE_H */
