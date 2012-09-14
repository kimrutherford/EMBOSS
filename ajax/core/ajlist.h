/* @include ajlist ************************************************************
**
** AJAX List functions
** These functions create and control linked lists.
**
** @author Copyright (C) 1998 Ian Longden
** @version $Revision: 1.48 $
** @modified 2001 Alan Bleasby
**              Changed lists to be double-linked, completely rewrote
**              iterator handling and added back-iteration functions.
**              Operation of ajListInsert made more intuitive.
** @modified $Date: 2012/07/02 15:47:30 $ by $Author: rice $
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

#ifndef AJLIST_H
#define AJLIST_H

/* ========================================================================= */
/* ============================= include files ============================= */
/* ========================================================================= */

#include "ajdefine.h"
#include "ajstr.h"

AJ_BEGIN_DECLS




/* ========================================================================= */
/* =============================== constants =============================== */
/* ========================================================================= */




/* @enum AjEListType **********************************************************
**
** AJAX List Type enumeration
**
** @value ajEListTypeAny Any data
** @value ajEListTypeStr String data
** @@
******************************************************************************/

typedef enum AjOListType
{
    ajEListTypeAny,
    ajEListTypeStr
} AjEListType;




/* ========================================================================= */
/* ============================== public data ============================== */
/* ========================================================================= */




/* @data AjPListNode **********************************************************
**
** AJAX List Node
**
** @alias AjSListNode
** @alias AjOListNode
**
** @attr Next [struct AjSListNode*] Next AJAX List Node object
** @attr Prev [struct AjSListNode*] Previous AJAX List Node object
** @attr Item [void*] Data value
** @@
******************************************************************************/

typedef struct AjSListNode
{
    struct AjSListNode* Next;
    struct AjSListNode* Prev;
    void* Item;
} AjOListNode;

#define AjPListNode AjOListNode*




/* @data AjPList **************************************************************
**
** List data object. Lists are simple linked lists with performance optimised
** to allow rapid extension of the beginning or end of the list.
**
** Lists can hold any data type. Special functions are available for lists
** of AjPStr values. In general, these functions are the same. Many are
** provided for ease of use to save remembering which calls need special cases.
**
** @alias AjSList
** @alias AjOList
**
** @attr First [AjPListNode] first node
** @attr Last [AjPListNode] dummy last node
** @attr Fitemdel [void function] Item destructor, or NULL if not an object
** @attr Count [ajulong] Number of nodes
** @attr Use [ajuint] Reference count
** @attr Padding [AjBool] Padding to alignment boundary
** @@
******************************************************************************/

typedef struct AjSList
{
    AjPListNode First;
    AjPListNode Last;
    void (*Fitemdel)(void** Pitem);
    ajulong Count;
    ajuint Use;
    AjBool Padding;
} AjOList;

#define AjPList AjOList*




/* @data AjIList **************************************************************
**
** AJAX list iterator data structure
**
** @alias AjSIList
** @alias AjOIList
**
** @attr Head [AjPList] Head of modifiable list
** @attr ReadHead [const AjPList] Head of read-only list
** @attr Here [AjPListNode] Current list node
** @attr Back [AjBool] Direction of last iterative move, true if reading back
** @attr Modify [AjBool] Allows iterator to modify the sequence
**
** @@
******************************************************************************/

typedef struct AjSIList
{
    AjPList Head;
    const AjPList ReadHead;
    AjPListNode Here;
    AjBool Back;
    AjBool Modify;
} AjOIList;

#define AjIList AjOIList*




/*
** Prototype definitions
*/

AjPList     ajListNewRef(AjPList list);
AjPList     ajListNewListref(const AjPList list);
void        ajListExit(void);
AjBool      ajListPeekFirst(const AjPList thys, void** Pitem);
void        ajListFree(AjPList* Plist);
void        ajListFreeData(AjPList* Plist);
AjIList     ajListIterNew(AjPList list);
AjIList     ajListIterNewBack(AjPList list);
AjIList     ajListIterNewreadBack(const AjPList list);
AjIList     ajListIterNewread(const AjPList list);
AjBool      ajListIterDoneBack(const AjIList iter);
void        ajListIterDel(AjIList* iter);
void*       ajListIterGet(AjIList iter);
void*       ajListIterGetBack(AjIList iter);
void        ajListIterRewind(AjIList iter);
AjBool      ajListIterDone(const AjIList iter);
void        ajListIterTrace(const AjIList iter);
void*       ajListDrop(AjPList thys, void* item);
AjBool      ajListPeekLast(const AjPList thys, void** Pitem);
AjBool      ajListPeekNumber(const AjPList thys, ajulong n, void** Pitem);
ajulong     ajListGetLength(const AjPList list);
ajulong     ajListstrGetMemsize(const AjPList list);
void        ajListMap(AjPList list,
                      void (*apply)(void** Pitem, void* cl), void* cl);
void        ajListMapread(const AjPList list,
                          void (*apply)(void* item, void* cl), void* cl);
AjPList     ajListNew(void);
AjBool      ajListPeek(const AjPList list, void** Pitem);
AjBool      ajListPop(AjPList list, void** Pitem);
AjBool      ajListPopLast(AjPList thys, void** Pitem);
void        ajListProbe(AjPList const* list);
void        ajListProbeData(AjPList const* list);
void        ajListPurge(AjPList list,
                        AjBool (*test)(const void* item),
                        void (*itemdel)(void** Pitem));
void        ajListPush(AjPList list, void* item);
void        ajListPushAppend(AjPList list, void* item);
void        ajListReverse(AjPList list);

AjPList     ajListstrNewList(const AjPList list);
AjPList     ajListstrNewListref(const AjPList list);
void        ajListstrFree(AjPList* Plist);
void        ajListstrFreeData(AjPList* Plist);
AjPStr      ajListstrIterGet(AjIList iter);
AjPStr      ajListstrIterGetBack(AjIList iter);
void        ajListstrIterTrace(const AjIList iter);
ajulong     ajListstrGetLength(const AjPList list);
void        ajListstrMap(AjPList thys,
                         void (*apply)(AjPStr* str, void* cl), void* cl);
void        ajListstrMapread(const AjPList thys,
                             void (*apply)(AjPStr str, void* cl), void* cl);
AjPList     ajListstrNew(void);
AjBool      ajListstrPeek(const AjPList list, AjPStr* Pstr);
AjBool      ajListstrPop(AjPList list, AjPStr* Pstr);
AjBool      ajListstrPopLast(AjPList thys, AjPStr* Pstr);
void        ajListstrPush(AjPList list, AjPStr str);
void        ajListstrPushAppend(AjPList list, AjPStr str);
void        ajListstrReverse(AjPList list);
ajulong     ajListstrToarray(const AjPList list, AjPStr** array);
ajulong     ajListstrToarrayAppend(const AjPList list, AjPStr** array);
void        ajListstrTrace(const AjPList list);

ajulong     ajListToarray(const AjPList list, void*** array);
ajulong     ajListToindex(const AjPList list, ajuint* listindex,
                          int (*compar1)(const void* item1,
                                         const void* item2));
void        ajListTrace(const AjPList list);

AjBool      ajListMapfind(const AjPList listhead,
                          AjBool (*apply)(void** Pitem, void* cl),
                          void* cl);
AjBool      ajListstrMapfind(const AjPList listhead,
                             AjBool (*apply)(AjPStr* Pstr, void* cl),
                             void* cl);

void        ajListPushlist(AjPList list, AjPList* Plist);
void        ajListstrPushlist(AjPList list, AjPList* Plist);
void        ajListIterInsert(AjIList iter, void* item);
void        ajListIterRemove(AjIList iter);
void        ajListstrIterInsert(AjIList iter, AjPStr str);
void        ajListstrIterRemove(AjIList iter);
void        ajListSort(AjPList thys,
                       int (*compar1)(const void* item1,
                                      const void* item2));
void        ajListSortTwo(AjPList thys,
                          int (*compar1)(const void* item1,
                                         const void* item2),
                          int (*compar2)(const void* item1,
                                         const void* item2));
void        ajListSortTwoThree(AjPList thys,
                               int (*compar1)(const void* item1,
                                              const void* item2),
                               int (*compar2)(const void* item1,
                                              const void* item2),
                               int (*compar3)(const void* item1,
                                              const void* item2));

void        ajListSortUnique(AjPList thys,
                             int (*compar1)(const void* item1,
                                            const void* item2),
                             void (*itemdel)(void** Pitem,
                                             void* cl));
void        ajListSortTwoUnique(AjPList thys,
                                int (*compar1)(const void* item1,
                                               const void* item2),
                                int (*compar2)(const void* item1,
                                               const void* item2),
                                void (*itemdel)(void** Pitem,
                                                void* cl));
void        ajListUnused(void** array);

/*
** End of prototype definitions
*/




#ifdef AJ_COMPILE_DEPRECATED_BOOK
#endif /* AJ_COMPILE_DEPRECATED_BOOK */

#ifdef AJ_COMPILE_DEPRECATED

__deprecated void        ajListPushList(AjPList list, AjPList* Plist);
__deprecated void        ajListstrPushList(AjPList list, AjPList* Plist);
__deprecated ajuint      ajListstrClone(const AjPList thys, AjPList newlist);
__deprecated AjPList     ajListNewArgs(void* x, ...);

__deprecated AjPListNode ajListNodesNew(void *x, ...);
__deprecated AjPList  ajListstrNewArgs(AjPStr x, ...);

__deprecated AjPList  ajListCopy(const AjPList list);
__deprecated void     ajListPushApp(AjPList list, void* item);

__deprecated void     ajListAppend(AjPList list, AjPListNode* tail);
__deprecated ajuint   ajListLength(const AjPList list);

__deprecated AjBool   ajListFirst(const AjPList thys, void** Pitem);
__deprecated AjBool   ajListLast(const AjPList thys, void** Pitem);
__deprecated AjBool   ajListNth(const AjPList thys, ajuint n, void** Pitem);
__deprecated AjBool   ajListPopEnd(AjPList thys, void** Pitem);
__deprecated void     ajListstrPushApp(AjPList list, AjPStr str);
__deprecated void     ajListDel(AjPList* Plist);
__deprecated AjBool   ajListstrPopEnd(AjPList thys, AjPStr* Pstr);
__deprecated ajuint   ajListstrToArray(const AjPList list, AjPStr** array);
__deprecated ajuint   ajListstrToArrayApp(const AjPList list, AjPStr** array);
__deprecated ajuint   ajListToArray(const AjPList list, void*** array);
__deprecated ajuint   ajListstrLength(const AjPList list);
__deprecated AjBool   ajListFind(const AjPList listhead,
                                 AjBool (*apply)(void** Pitem, void* cl),
                                 void* cl);
__deprecated AjBool   ajListstrFind(const AjPList listhead,
                                    AjBool (*apply)(AjPStr* Pstr, void* cl),
                                    void* cl);

__deprecated void     ajListMapRead(const AjPList list,
                                    void apply(void* item, void* cl),
                                    void* cl);
__deprecated void     ajListstrMapRead(const AjPList thys,
                                       void (*apply)(AjPStr str, void* cl),
                                       void* cl);

__deprecated void     ajListstrDel(AjPList* Plist);
__deprecated AjPList  ajListstrCopy(const AjPList list);

__deprecated AjIList  ajListIter(AjPList list);
__deprecated AjIList  ajListIterBack(AjPList list);
__deprecated AjIList  ajListIterBackRead(const AjPList list);
__deprecated AjIList  ajListIterRead(const AjPList list);
__deprecated AjBool   ajListIterBackDone(const AjIList iter);
__deprecated AjBool   ajListIterBackMore(const AjIList iter);
__deprecated void*    ajListIterBackNext(AjIList iter);
__deprecated void     ajListIterFree(AjIList* iter);
__deprecated AjBool   ajListIterMoreBack(const AjIList iter);
__deprecated AjBool   ajListIterMore(const AjIList iter);
__deprecated void*    ajListIterNext(AjIList iter);

__deprecated void     ajListInsert(AjIList iter, void* item);
__deprecated void     ajListRemove(AjIList iter);

__deprecated void     ajListstrInsert(AjIList iter, AjPStr str);
__deprecated void     ajListstrRemove(AjIList iter);
__deprecated void     ajListSort2(AjPList thys,
                                  int (*compar1)(const void* item1,
                                                 const void* item2),
                                  int (*compar2)(const void* item1,
                                                 const void* item2));
__deprecated void     ajListSort3(AjPList thys,
                                  int (*compar1)(const void* item1,
                                                 const void* item2),
                                  int (*compar2)(const void* item1,
                                                 const void* item2),
                                  int (*compar3)(const void* item1,
                                                 const void* item2));

__deprecated void     ajListUnique(AjPList thys,
                                   int (*compar)(const void* item1,
                                                 const void* item2),
                                   void (*itemdel)(void** Pitem,
                                                   void* cl));
__deprecated void     ajListUnique2(AjPList thys,
                                    int (*compar1)(const void* item1,
                                                   const void* item2),
                                    int (*compar2)(const void* item1,
                                                   const void* item2),
                                    void (*itemdel)(void** Pitem,
                                                    void* cl));

__deprecated void     ajListGarbageCollect(AjPList list,
                                           void (*itemdel)(void** Pitem),
                                           AjBool (*test)(const void* item));

#endif /* AJ_COMPILE_DEPRECATED */




AJ_END_DECLS

#endif /* !AJLIST_H */
