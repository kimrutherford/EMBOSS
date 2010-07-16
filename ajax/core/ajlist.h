#ifdef __cplusplus
extern "C"
{
#endif

#ifndef ajlist_h
#define ajlist_h

#include "ajdefine.h"

enum AjEListType {ajEListAny, ajEListStr};




/* @data AjPListNode **********************************************************
**
** Substructure of AjPList
**
** @alias AjSListNode
** @alias AjOListNode
**
** @attr Next [struct AjSListNode*] next item
** @attr Prev [struct AjSListNode*] previous item
** @attr Item [void*] data value
** @@
******************************************************************************/

typedef struct AjSListNode {
	struct AjSListNode* Next;
	struct AjSListNode* Prev;
	void *Item;
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
** @attr Count [ajuint] Number of nodes
** @attr Type [AjEnum] List type (any, string, etc.)
** @@
******************************************************************************/

typedef struct AjSList {
  AjPListNode First;
  AjPListNode Last;
  ajuint Count;
  AjEnum Type;
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

typedef struct AjSIList {
  AjPList Head ;
  const AjPList ReadHead ;
  AjPListNode Here;
  AjBool Back;
  AjBool Modify;
} AjOIList;

#define AjIList AjOIList*


/*
** Prototype definitions
*/

AjPList     ajListNewListref(const AjPList list);
void        ajListExit (void);
AjBool      ajListPeekFirst (const AjPList thys, void** x);
void        ajListFree   (AjPList *list);
void        ajListFreeData   (AjPList *list);
AjIList     ajListIterNew (AjPList list);
AjIList     ajListIterNewBack (AjPList list);
AjIList     ajListIterNewreadBack (const AjPList list);
AjIList     ajListIterNewread (const AjPList list);
AjBool      ajListIterDoneBack (const AjIList iter);
void        ajListIterDel (AjIList* iter);
void*       ajListIterGet (AjIList iter);
void*       ajListIterGetBack (AjIList iter);
void        ajListIterRewind (AjIList iter);
AjBool      ajListIterDone (const AjIList iter);
void        ajListIterTrace (const AjIList iter);
AjBool      ajListPeekLast (const AjPList thys, void** x);
AjBool      ajListPeekNumber (const AjPList thys, ajuint n, void** x);
ajuint      ajListGetLength (const AjPList list);
void        ajListMap    (AjPList list,
			  void apply(void **x, void *cl), void *cl);
void        ajListMapread(const AjPList list,
			  void apply(void *x, void *cl), void *cl);
AjPList     ajListNew (void);          /* return header */
AjBool      ajListPeek    (const AjPList list, void** x);
AjBool      ajListPop    (AjPList list, void** x);
AjBool      ajListPopLast (AjPList thys, void** x);
void        ajListProbe   (AjPList const *list);
void        ajListProbeData   (AjPList const *list);
void        ajListPurge(AjPList list,
			AjBool (*test)(const void *),
			void (*nodedelete)(void **));
void        ajListPush   (AjPList list, void* x);      /* " " */
void        ajListPushAppend(AjPList list, void* x);
void        ajListReverse(AjPList list);

AjPList     ajListstrNewList   (const AjPList list);
AjPList     ajListstrNewListref   (const AjPList list);
void        ajListstrFree   (AjPList *list);
void        ajListstrFreeData   (AjPList *list);
AjPStr      ajListstrIterGet (AjIList iter);
AjPStr      ajListstrIterGetBack (AjIList iter);
void        ajListstrIterTrace (const AjIList iter);
ajuint      ajListstrGetLength (const AjPList list);
void        ajListstrMap (AjPList thys,
			  void apply(AjPStr* x, void* cl), void* cl);
void        ajListstrMapread (const AjPList thys,
			      void (*apply) (AjPStr x, void* cl), void* cl);
AjPList     ajListstrNew (void);          /* return header */
AjBool      ajListstrPeek    (const AjPList list, AjPStr* x);
AjBool      ajListstrPop    (AjPList list, AjPStr* x);
AjBool      ajListstrPopLast (AjPList thys, AjPStr *x);
void        ajListstrPush (AjPList list, AjPStr x);
void        ajListstrPushAppend (AjPList list, AjPStr x);
void        ajListstrReverse (AjPList list);
ajuint      ajListstrToarray (const AjPList list, AjPStr** array);
ajuint      ajListstrToarrayAppend (const AjPList list, AjPStr** array);
void        ajListstrTrace   (const AjPList list);

ajuint      ajListToarray (const AjPList list, void*** array);
ajuint      ajListToindex(const AjPList list, ajuint* listindex,
                          int (*sort1) (const void*, const void*));
void        ajListTrace   (const AjPList list);

AjBool      ajListMapfind (const AjPList listhead,
			   AjBool apply(void **x, void *cl), void *cl);
AjBool      ajListstrMapfind (const AjPList listhead,
			      AjBool apply(AjPStr* x, void *cl), void *cl);

void        ajListPushlist (AjPList list, AjPList* pmore);
void        ajListstrPushlist (AjPList list, AjPList* pmore);
void        ajListIterInsert (AjIList iter, void* x);
void        ajListIterRemove (AjIList iter);
void        ajListstrIterInsert (AjIList iter, AjPStr x);
void        ajListstrIterRemove (AjIList iter);
void        ajListSort (AjPList thys,
			int (*sort1) (const void*, const void*));
void        ajListSortTwo (AjPList thys,
			 int (*sort1) (const void*, const void*),
			 int (*sort2) (const void*, const void*));
void        ajListSortTwoThree (AjPList thys,
			 int (*sort1) (const void*, const void*),
			 int (*sort2) (const void*, const void*),
			 int (*sort3) (const void*, const void*));

void        ajListSortUnique (AjPList thys,
			  int (*sort1) (const void* x, const void* cl),
			  void (*nodedelete) (void** x, void* cl));
void        ajListSortTwoUnique(AjPList thys,
			  int (*sort1) (const void* x, const void* cl),
			  int (*sort2) (const void* x, const void* cl),
			  void (*nodedelete) (void** x, void* cl));
void        ajListUnused(void** array);

/*
** End of prototype definitions
*/

__deprecated void        ajListPushList (AjPList list, AjPList* pmore);
__deprecated void        ajListstrPushList (AjPList list, AjPList* pmore);
__deprecated ajuint      ajListstrClone (const AjPList thys, AjPList newlist);
__deprecated AjPList   ajListNewArgs(void* x, ...);  /* new header returned */

__deprecated AjPListNode ajListNodesNew (void *x, ...); /* same as NewArgs but
							no header */
__deprecated AjPList ajListstrNewArgs(AjPStr x, ...);  /* new header returned */

__deprecated AjPList  ajListCopy(const AjPList list);   /* new list returned */
__deprecated void     ajListPushApp (AjPList list, void* x);

__deprecated void     ajListAppend (AjPList list, AjPListNode* tail);
__deprecated ajuint   ajListLength (const AjPList list);

__deprecated AjBool   ajListFirst (const AjPList thys, void** x);
__deprecated AjBool   ajListLast (const AjPList thys, void** x);
__deprecated AjBool   ajListNth (const AjPList thys, ajuint n, void** x);
__deprecated AjBool   ajListPopEnd (AjPList thys, void** x);
__deprecated void     ajListstrPushApp (AjPList list, AjPStr x);
__deprecated void     ajListDel   (AjPList *list);
__deprecated AjBool   ajListstrPopEnd (AjPList thys, AjPStr *x);
__deprecated ajuint   ajListstrToArray (const AjPList list, AjPStr** array);
__deprecated ajuint   ajListstrToArrayApp(const AjPList list, AjPStr** array);
__deprecated ajuint   ajListToArray (const AjPList list, void*** array);
__deprecated ajuint      ajListstrLength (const AjPList list);
__deprecated AjBool      ajListFind (const AjPList listhead,
			AjBool apply(void **x, void *cl), void *cl);
__deprecated AjBool      ajListstrFind (const AjPList listhead,
			   AjBool apply(AjPStr* x, void *cl), void *cl);

__deprecated void        ajListMapRead(const AjPList list,
			  void apply(void *x, void *cl), void *cl);
__deprecated void        ajListstrMapRead (const AjPList thys,
			      void apply(AjPStr x, void* cl), void* cl);

__deprecated void        ajListstrDel   (AjPList *list);
__deprecated AjPList     ajListstrCopy   (const AjPList list);

__deprecated AjIList     ajListIter (AjPList list);
__deprecated AjIList     ajListIterBack (AjPList list);
__deprecated AjIList     ajListIterBackRead (const AjPList list);
__deprecated AjIList     ajListIterRead (const AjPList list);
__deprecated AjBool      ajListIterBackDone (const AjIList iter);
__deprecated AjBool      ajListIterBackMore (const AjIList iter);
__deprecated void*       ajListIterBackNext (AjIList iter);
__deprecated void        ajListIterFree (AjIList* iter);
__deprecated AjBool      ajListIterMoreBack (const AjIList iter);
__deprecated AjBool      ajListIterMore (const AjIList iter);
__deprecated void*       ajListIterNext (AjIList iter);

__deprecated void        ajListInsert (AjIList iter, void* x);
__deprecated void        ajListRemove (AjIList iter);

__deprecated void        ajListstrInsert (AjIList iter, AjPStr x);
__deprecated void        ajListstrRemove (AjIList iter);
__deprecated void        ajListSort2 (AjPList thys,
			 int (*sort1) (const void*, const void*),
			 int (*sort2) (const void*, const void*));
__deprecated void        ajListSort3 (AjPList thys,
			 int (*sort1) (const void*, const void*),
			 int (*sort2) (const void*, const void*),
			 int (*sort3) (const void*, const void*));

__deprecated void        ajListUnique (AjPList thys,
			  int (*compar) (const void* x, const void* cl),
			  void (*nodedelete) (void** x, void* cl));
__deprecated void        ajListUnique2(AjPList thys,
			  int (*compar1) (const void* x, const void* cl),
			  int (*compar2) (const void* x, const void* cl),
			  void (*nodedelete) (void** x, void* cl));

__deprecated void        ajListGarbageCollect (AjPList list,
					       void (*destruct)(void **),
					       AjBool (*compar)(const void *));

#endif

#ifdef __cplusplus
}
#endif
