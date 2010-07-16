/******************************************************************************
** @source AJAX list functions
** These functions create and control linked lists.
**
** @author Copyright (C) 1998 Ian Longden
** @version 1.0
** @author Copyright (C) 2001 Alan Bleasby
** @version 2.0 Changed lists to be double-linked, completely rewrote
**              iterator handling and added back-iteration functions.
**              Operation of ajListInsert made more intuitive.
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

/*Library* List Library *******************************************************
**
** All lists consist of an initial header followed by the body
** of the list. The Header has three variables:-
** 1) First - a pointer to the linked list (see body)
** 2) Last -  a pointer to the a dummy last node object with next = self
** 3) Count - which holds the number of objects in the linked list
**           (NOT including the header)
** 4) Type - the list type
**
** The body of the linked list contains three variables:-
** 1) next - a pointer to the next linked list object or NULL
** 2) prev - a pointer to the previous linked list object or NULL
** 3) item - a void pointer to the data.
******************************************************************************/

#include <stddef.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>

#include "ajax.h"

#ifdef AJ_SAVESTATS
static ajint listNewCnt     = 0;
static ajint listDelCnt     = 0;
static ajuint listMaxNum     = 0;
static ajint listNodeCnt    = 0;
static ajint listIterNewCnt = 0;
static ajint listIterDelCnt = 0;
#endif

static ajint listFreeNext = 0;
static ajint listFreeMax = 0;
static AjPListNode* listFreeSet = NULL;

static AjPList listNew(AjEnum type);
static void listInsertNode(AjPListNode * pnode, void* x);
static AjPListNode listDummyNode(AjPListNode * pnode);
static void listNodesTrace(const AjPListNode node);
static AjBool listNodeDel(AjPListNode * pnode);
static void* listNodeItem(const AjPListNode node);
static void listArrayTrace(void** array);
static void listFreeSetExpand (void);




/* @filesection ajlist *********************************************************
**
** @nam1rule aj Function belongs to the AJAX library.
**
*/




/* @datasection [AjPList] Lists ***********************************************
**
** Function is for manipulating lists with any value type.
**
** Some functions are specially designed to understand string (AjPStr) values.
**
** @nam2rule List
*/




/* @section Constructors ******************************************************
**
** Constructors for lists
**
** @fdata [AjPList]
**
** @fcategory new
**
** @nam3rule New Constructor
** @nam4rule NewListref Copy constructor with pointers to source data
**
** @argrule NewListref list [const AjPList] Source list
**
** @valrule * [AjPList] New list
**
******************************************************************************/





/* @func ajListNew ************************************************************
**
** Creates a new general list.
**
** @return [AjPList] new list;
** @@
******************************************************************************/

AjPList ajListNew(void)
{
    return listNew(ajEListAny);
}




/* @func ajListNewListref ******************************************************
**
** Copy a list.
**
** WARNING: pointers to the data are copied, NOT the data
**          so be careful when cleaning up after copy.
**
** @param [r] list [const AjPList] list to be copied
** @return [AjPList] new copied list.
** @@
******************************************************************************/

AjPList ajListNewListref(const AjPList list)
{
    AjPList newlist;

    AjPListNode node;

    if(!list)
	return NULL;

    newlist = ajListNew();
    newlist->Type = list->Type;

    for( node=list->First; node->Next; node=node->Next)
	ajListPushAppend(newlist, node->Item);


    return newlist;
}




/* @obsolete ajListCopy
** @rename ajListNewListref
*/

__deprecated AjPList ajListCopy(const AjPList list)
{
    return ajListNewListref(list);
}




/* @obsolete ajListNewArgs
** @remove Use explicit ajListPush calls instead
*/




__deprecated AjPList ajListNewArgs(void* x, ...)
{
    AjPList list;
    va_list ap;
    ajint i = 0;
    void* y;

    list = ajListNew();

    if(!x)
	return list;

    va_start(ap, x);
    y = x;

    for( i=0; y; y = va_arg(ap, void*),i++)
	ajListPushAppend(list, y);

    va_end(ap);

    return list;
}




/* @funcstatic listNew ********************************************************
**
** Creates a new list.
**
** @param [r] type [AjEnum] Defined list type.
** @return [AjPList] new list;
** @@
******************************************************************************/

static AjPList listNew(AjEnum type)
{
    AjPList list;

    AJNEW0(list);
    list->Type = type;

    list->Last = listDummyNode(&list->First);

#ifdef AJ_SAVESTATS
    listNodeCnt--;			/* dummy */
    listNewCnt++;
#endif

    return list;
}




/* @funcstatic listInsertNode *************************************************
**
** Inserts a new node in a list at the current node position.
**
** @param [u] pnode [AjPListNode *] Current node.
** @param [u] x [void*] Data item to insert.
** @return [void]
** @@
******************************************************************************/

static void listInsertNode(AjPListNode * pnode, void* x)
{
    AjPListNode p;

    if(listFreeNext)
        p = listFreeSet[--listFreeNext];
    else
        AJNEW0(p);
    
    p->Item = x;
    p->Next = (*pnode);
    p->Prev = (*pnode)->Prev;

    p->Next->Prev = p;

    *pnode = p;

#ifdef AJ_SAVESTATS
    listNodeCnt++;
#endif

    return;
}




/* @funcstatic listDummyNode **************************************************
**
** Creates a new empty node.
**
** @param [u] pnode [AjPListNode *] New node.
** @return [AjPListNode] Copy of current node.
** @@
******************************************************************************/

static AjPListNode listDummyNode(AjPListNode *pnode)
{
    if(listFreeNext)
    {
        *pnode = listFreeSet[--listFreeNext];
        (*pnode)->Item = NULL;
        (*pnode)->Prev = NULL;
        (*pnode)->Next = NULL;
    }
    else
        AJNEW0(*pnode);

#ifdef AJ_SAVESTATS
    listNodeCnt++;
#endif

    return *pnode;
}




/* @section Adding values ******************************************************
**
** @fdata [AjPList]
**
** @nam3rule Push Add new value
** @nam4rule PushAppend Add new value to end of list
** @nam3rule Pushlist Add new list of values
**
** @argrule * list [AjPList] List
** @argrule Push x [void*] Pointer to data.
** @argrule Pushlist Plist [AjPList*] Source list to be deleted
**
** @valrule * [void]
**
** @fcategory modify
******************************************************************************/




/* @func ajListPush ***********************************************************
**
** Add a new node at the start of the list and add the
** data pointer.
**
** @param [u] list [AjPList] list to be changed.
** @param [u] x [void*] Pointer to data.
** @return [void]
** @@
******************************************************************************/

void ajListPush(AjPList list, void* x)
{
    assert(list);

    listInsertNode(&list->First, x);

    if(!list->Count++)
	list->Last->Prev = list->First;

#ifdef AJ_SAVESTATS
    if(list->Count > listMaxNum)
	listMaxNum = list->Count;
#endif

    return;
}




/* @func ajListPushAppend *****************************************************
**
** Add a new node at the end of the list and add the
** data pointer.
**
** @param [u] list [AjPList] List to be changed.
** @param [u] x [void*] Pointer to data to append.
** @return [void]
** @@
******************************************************************************/

void ajListPushAppend(AjPList list, void* x)
{
    /*
    ** cannot use listInsertNode because that needs a pointer to the
    ** penultimate node, so we use the dummy node and make a new dummy node
    ** instead
    */
    AjPListNode tmp = NULL;

    assert(list);

    if(!list->Count)
    {
	ajListPush(list,x);
	return;
    }

    list->Last->Item = x;

    tmp = list->Last;
    list->Last = listDummyNode(&list->Last->Next);
    list->Last->Prev = tmp;

    list->Count++;

#ifdef AJ_SAVESTATS
    if(list->Count > listMaxNum)
	listMaxNum = list->Count;
#endif

    return;
}




/* @obsolete ajListPushApp
** @rename ajListPushAppend
*/

__deprecated void ajListPushApp(AjPList list, void* x)
{
    ajListPushAppend(list, x);
    return;
}




/* @func ajListPushlist *******************************************************
**
** Adds a list to the start of the current list, then deletes the second list.
**
** @param [u] list [AjPList] List.
** @param [d] Plist [AjPList*] List to be merged.
** @return [void]
** @@
******************************************************************************/

void ajListPushlist(AjPList list, AjPList* Plist)
{
    AjPList more = *Plist;

    if(more->Count)
    {
	/* more list has items */
	if(list->Count)
	{
	    /* master list has items */
	    more->Last->Item = list->First->Item;
	    more->Last->Next = list->First->Next;
	    list->First->Next->Prev = more->Last;
	}
	else
	{
	    /* master list is empty */
	    list->Last = more->Last;
	}

        if(listFreeNext >= listFreeMax)
            listFreeSetExpand();

        if(listFreeNext >= listFreeMax)
            AJFREE(list->First);
        else if(list->First)
            listFreeSet[listFreeNext++] = list->First;
        
	list->First = more->First;
	list->Count += more->Count;
	list->First->Prev = NULL;
	more->First = NULL;
	more->Count=0;

#ifdef AJ_SAVESTATS
	if(list->Count > listMaxNum)
	    listMaxNum = list->Count;
#endif
    }

    ajListFree(Plist);	/* free the list but not the nodes */

    return;
}




/* @obsolete ajListPushList
** @rename ajListPushlist
*/

__deprecated void ajListPushList(AjPList list, AjPList* pmore)
{
    ajListPushlist(list, pmore);
    return;
}




/* @section Modifiers **********************************************************
**
** @fdata [AjPList]
**
** Modifies a list
**
** @nam3rule Map Apply function to each node
** @nam3rule Purge Select and remove unwanted nodes
** @nam3rule Reverse Reverse the order of nodes
** @nam3rule Sort Apply function to each node
** @nam4rule Two Apply second function to each equal node
** @nam5rule Three Apply third function to each equal node
** @suffix Unique Remove non-unique values from the list
**
** @argrule * list [AjPList] List
** @argrule Map apply [void function] Function to apply
** @argrule Map cl [void*] Data (usually NULL) to be passed to apply.
** @argrule Purge test [AjBool* function] Function to find unwanted values
** @argrule Purge nodedelete [void* function] Function to remove a value
** @argrule Sort sort1 [int* function] Function to compare values
** @argrule Two sort2 [int* function] Second function to compare equal values
** @argrule Three sort3 [int* function] Second function to compare equal values
** @argrule Unique nodedelete [void* function] Function to remove a value
**
** @valrule * [void]
**
** @fcategory modify
**
******************************************************************************/




/* @func ajListMap ************************************************************
**
** For each node in the list call function apply.
**
** @param [u] list [AjPList] List.
** @param [f] apply [void function] Function to call for each list item.
** @param [u] cl [void*] Standard, usually NULL.
** @return [void]
** @@
******************************************************************************/

void ajListMap(AjPList list, void apply(void** x, void* cl), void* cl)
{
    AjPListNode rest;

    assert(apply);

    for(rest = list->First; rest->Next; rest = rest->Next)
	apply((void**) &rest->Item, cl);

    return;
}




/* @func ajListPurge ***********************************************************
**
** Garbage collect a list
**
** @param [u] list [AjPList] List.
** @param [f] test [AjBool* function] Function to test whether to delete
** @param [f] nodedelete [void* function] Wrapper function for item destructor
** @return [void]
** @@
******************************************************************************/

void ajListPurge(AjPList list,
		 AjBool (*test)(const void *),
		 void (*nodedelete)(void **))
{
    AjIList iter = NULL;
    void    *ret;


    iter = ajListIterNew(list);

    while((ret=ajListIterGet(iter)))
	if(test(ret))
	{
	    nodedelete(&ret);
	    ajListIterRemove(iter);
	}


    ajListIterDel(&iter);

    return;
}




/* @obsolete ajListGarbageCollect
** @replace ajListPurge (1,2,3/1,3,2)
*/

__deprecated void ajListGarbageCollect(AjPList list, void (*destruct)(void **),
			  AjBool (*compar)(const void *))
{
    ajListPurge(list, compar, destruct);
    return;
}




/* @func ajListReverse ********************************************************
**
** Reverse the order of the nodes in an abstract list.
**
** @param [u] list [AjPList] List
** @return [void]
** @@
******************************************************************************/

void ajListReverse(AjPList list)
{
    AjPListNode head;
    AjPListNode savenext;
    AjPListNode node;

    if(!list)
        return;

    if(list->Count <= 1)
	return;

    head = list->Last;

    list->Last->Prev = list->First;

    for( node = list->First; node->Next; node = savenext)
    {
	savenext = node->Next;
	node->Prev = node->Next;
	node->Next = head;
	head = node;
    }

    list->First = head;

    list->First->Prev = NULL;

    return;
}




/* @func ajListSort ***********************************************************
**
** Sort the items in a list.
**
** @param [u] list [AjPList] List.
** @param [f] sort1 [int* function] Function to compare two list items.
** @return [void]
** @@
******************************************************************************/

void ajListSort(AjPList list, int (*sort1) (const void*, const void*))
{
    void** array = NULL;
    ajint i = 0;
    AjPListNode node;

    /*ajDebug("ajListSort %d items\n", list->Count);*/
    /*ajListTrace(list);*/

    node = list->First;

    if(list->Count <= 1)
	return;

    ajListToarray(list, &array);
    /* listArrayTrace(array);*/

    qsort(array, list->Count, sizeof(void*), sort1);

    while(node->Next)
    {
	node->Item = array[i++];
	node = node->Next;
    }

    AJFREE(array);

    return;
}




/* @func ajListSortTwo *********************************************************
**
** Sort the items in a list using 2 fields in the same object hierarchy.
**
** @param [u] list [AjPList] List.
** @param [f] sort1 [int* function] 1st function to compare two list items.
** @param [f] sort2 [int* function] 2nd function to compare two list items.
** @return [void]
** @@
******************************************************************************/

void ajListSortTwo(AjPList list,
		   int (*sort1) (const void*, const void*),
		   int (*sort2) (const void*, const void*))
{
    AjPListNode node;
    void **ptrs = NULL;
    ajint len;
    ajint limit;
    ajint pos;
    ajint base;
    ajint n;

    node = list->First;

    ajListSort(list,sort1);

    len = ajListToarray(list,&ptrs);

    if(len<2)
	return;

    pos = base = 0;
    limit = len-2;

    while(pos < limit)
    {
	while(!sort1(&ptrs[pos],&ptrs[pos+1]))
	{
	    ++pos;

	    if(pos>limit)
		break;
	}
	++pos;

	n = pos-base;

	if(n>1)
	    qsort((void *)&ptrs[base],n,sizeof(void*),sort2);

	base = pos;
    }

    pos = 0;

    while(node->Next)
    {
	node->Item = ptrs[pos++];
	node = node->Next;
    }

    AJFREE(ptrs);
    return;
}




/* @obsolete ajListSort2
** @rename ajListSortTwo
*/

__deprecated void ajListSort2(AjPList list,
			      int (*sort1) (const void*, const void*),
			      int (*sort2) (const void*, const void*))
{
    ajListSortTwo(list, sort1, sort2);

    return;
}




/* @func ajListSortTwoThree ****************************************************
**
** Sort the items in a list using 3 fields in the same object hierarchy.
**
** @param [u] list [AjPList] List.
** @param [f] sort1 [int* function] 1st function to compare two list items.
** @param [f] sort2 [int* function] 2nd function to compare two list items.
** @param [f] sort3 [int* function] 3rd function to compare two list items.
** @return [void]
** @@
******************************************************************************/

void ajListSortTwoThree(AjPList list,
			int (*sort1) (const void*, const void*),
			int (*sort2) (const void*, const void*),
			int (*sort3) (const void*, const void*))
{
    AjPListNode node;
    void **ptrs = NULL;
    ajint len;
    ajint limit;
    ajint pos;
    ajint base;
    ajint n;

    node = list->First;
    len = ajListGetLength(list);

    if(len<2)
	return;

    ajListSortTwo(list,sort1,sort2);

    len = ajListToarray(list,&ptrs);

    pos = base = 0;
    limit = len-2;

    while(pos < limit)
    {
	while(!sort1(&ptrs[pos],&ptrs[pos+1]) &&
	      !sort2(&ptrs[pos],&ptrs[pos+1]))
	{
	    ++pos;

	    if(pos>limit)
		break;
	}

	++pos;
	n = pos-base;

	if(n>1)
	    qsort((void *)&ptrs[base],n,sizeof(void*),sort3);

	base = pos;
    }

    pos = 0;

    while(node->Next)
    {
	node->Item = ptrs[pos++];
	node = node->Next;
    }

    AJFREE(ptrs);

    return;
}




/* @obsolete ajListSort3
** @rename ajListSortTwoThree
*/

__deprecated void ajListSort3(AjPList list,
			      int (*sort1) (const void*, const void*),
			      int (*sort2) (const void*, const void*),
			      int (*sort3) (const void*, const void*))
{
    ajListSortTwoThree(list, sort1, sort2, sort3);

    return;
}




/* @func ajListSortTwoUnique ***************************************************
**
** Double-sort the items in a list, and remove duplicates
**
** @param [u] list [AjPList] List.
** @param [f] sort1 [int* function] Function to compare two list items.
** @param [f] sort2 [int* function] Function to compare two list items.
** @param [f] nodedelete [void* function] Function to delete an item
** @return [void]
** @@
******************************************************************************/

void ajListSortTwoUnique(AjPList list,
			 int (*sort1) (const void* x, const void* cl),
			 int (*sort2) (const void* x, const void* cl),
			 void (*nodedelete) (void** x, void* cl))
{
    void* item;
    void* previtem = NULL;
    AjIList iter;

    ajDebug("ajListUnique %d items\n", list->Count);

    if(list->Count <= 1)		/* no duplicates */
	return;

    ajListSortTwo(list, sort1, sort2);
    /*ajListTrace(list);*/

    iter = ajListIterNew(list);

    while(!ajListIterDone(iter))
    {
	item = ajListIterGet(iter);

	if(previtem && !sort1(&item, &previtem) &&
	   !sort2(&item, &previtem))
	{
	    nodedelete(&item, NULL);
	    ajListIterRemove(iter);
	}
	else
	    previtem=item;
    }

    ajListIterDel(&iter);

    ajDebug("ajListUnique result %d items\n", list->Count);
    /*ajListTrace(list);*/

    return;
}




/* @obsolete ajListUnique2
** @rename ajListSortTwoUnique
*/

__deprecated void ajListUnique2(AjPList list,
				int (*sort1) (const void* x, const void* cl),
				int (*sort2) (const void* x, const void* cl),
				void (*nodedelete) (void** x, void* cl))
{
    ajListSortTwoUnique(list, sort1, sort2, nodedelete);

    return;
}




/* @func ajListSortUnique ******************************************************
**
** Sort the items in a list, and remove duplicates
**
** @param [u] list [AjPList] List.
** @param [f] sort1 [int* function] Function to compare two list items.
** @param [f] nodedelete [void* function] Function to delete an item
** @return [void]
** @@
******************************************************************************/

void ajListSortUnique(AjPList list,
		      int (*sort1) (const void* x, const void* cl),
		      void (*nodedelete) (void** x, void* cl))
{
    void* item;
    void* previtem = NULL;
    AjIList iter;

    ajDebug("ajListSortUnique %d items\n", list->Count);

    if(list->Count <= 1)		/* no duplicates */
	return;

    ajListSort(list, sort1);
    /*ajListTrace(list);*/

    iter = ajListIterNew(list);

    while(!ajListIterDone(iter))
    {
	item = ajListIterGet(iter);

	if(previtem && !sort1(&item, &previtem))
	{
	    nodedelete(&item, NULL);
	    ajListIterRemove(iter);
	}
	else
	    previtem=item;
    }

    ajListIterDel(&iter);

    ajDebug("ajListSortUnique result %d items\n", list->Count);
    /*ajListTrace(list);*/

    return;
}




/* @obsolete ajListUnique
** @rename ajListSortUnique
*/

__deprecated void ajListUnique(AjPList list,
		  int (*compar) (const void* x, const void* cl),
		  void (*nodedelete) (void** x, void* cl))
{
    ajListSortUnique(list, compar, nodedelete);

    return;
}




/* @section Removing data ******************************************************
**
** @fdata [AjPList]
**
** Retrieves values from a list
**
** @nam3rule Pop Retrieve next value
** @nam4rule PopLast Retrieve last value
**
** @argrule * list [AjPList] List
** @argrule Pop x [void**] Value
**
** @valrule * [AjBool] True on success
** @fcategory cast
**
******************************************************************************/




/* @funcstatic listFreeSetExpand ***********************************************
**
** Expand the list of free nodes
**
** @return [void]
******************************************************************************/

static void listFreeSetExpand (void)
{
    ajint newsize;

    if(!listFreeSet)
    {
        listFreeMax = 1024;
        AJCNEW0(listFreeSet,listFreeMax);

        return;
    }

    if(listFreeMax >= 65536)
        return;

    newsize = listFreeMax + listFreeMax;
    AJCRESIZE0(listFreeSet, listFreeMax, newsize);
    listFreeMax = newsize;

    return;
}




/* @func ajListPop ************************************************************
**
** remove the first node but set pointer to data first.
**
** @param [u] list [AjPList] List
** @param [w] x [void**] pointer to pointer to data
** @return [AjBool] ajTrue on success.
** @@
******************************************************************************/

AjBool ajListPop(AjPList list, void** x)
{
    if(!list) 
    {
        if(x)
            *x = NULL;

	return ajFalse;
    }
    
    if(x)
	*x = listNodeItem(list->First);

    if(!listNodeDel(&list->First))
	return ajFalse;

    list->First->Prev = NULL;
    list->Count--;

    return ajTrue;
}




/* @func ajListPopLast ********************************************************
**
** remove the last node but set pointer to data first.
**
** @param [u] list [AjPList] List
** @param [w] x [void**] pointer to pointer to data
** @return [AjBool] ajTrue on success.
** @@
******************************************************************************/

AjBool ajListPopLast(AjPList list, void** x)
{
    AjPListNode pthis = NULL;

    if(!list)
    {
        if(x)
            *x = NULL;

	return ajFalse;
    }
    

    if(!list->Count)
	return ajFalse;

    pthis = list->Last->Prev;

    if(x)
	*x = listNodeItem(pthis);


    if(list->Count==1)
    {
	list->Last->Prev = NULL;

        if(listFreeNext >= listFreeMax)
            listFreeSetExpand();

        if(listFreeNext >= listFreeMax)
            AJFREE(list->First);
        else if(list->First)
            listFreeSet[listFreeNext++] = list->First;
        
	list->First = list->Last;
    }
    else
    {
	pthis->Prev->Next = list->Last;
	list->Last->Prev = pthis->Prev;

        if(listFreeNext >= listFreeMax)
            listFreeSetExpand();

        if(listFreeNext >= listFreeMax)
            AJFREE(pthis);
        else if(pthis)
        {
            listFreeSet[listFreeNext++] = pthis;
            pthis = NULL;
        }
    }

    --list->Count;

    return ajTrue;
}




/* @obsolete ajListPopEnd
** @rename ajListPopLast
*/

__deprecated AjBool ajListPopEnd(AjPList list, void** x)
{
    return ajListPopLast(list, x);
}




/* @section Element retrieval **************************************************
**
** @fdata [AjPList]
**
** Retrieves attributes or values from a list without changing it
**
** @nam3rule Get Retrieve attribute
** @nam4rule GetLength Retrieve list length
** @nam3rule Mapread Apply function to each node without modifying list or data
** @nam3rule Mapfind Apply function to each node without modifying list
**                   or data
** @nam3rule Peek Pointer to next value
** @nam4rule PeekFirst Pointer to first value
** @nam4rule PeekLast Pointer to last value
** @nam4rule PeekNumber Pointer to numbered value
** @nam3rule Toarray Build array of values
** @nam3rule Toindex Sort index array by list node values
**
** @argrule * list [const AjPList] List
** @argrule Mapfind apply [AjBool function] Function to apply
** @argrule Mapread apply [void function] Function to apply
** @argrule Mapfind cl [void*] Data (usually NULL) to be passed to apply.
** @argrule Mapread cl [void*] Data (usually NULL) to be passed to apply.
** @argrule PeekNumber ipos [ajuint] Position in list
** @argrule Peek x [void**] Value
** @argrule Toarray array [void***] Array of values, ending with NULL
** @argrule Toindex lind [ajuint*] Populated ndex array to be sorted
** @argrule Toindex sort1 [int* function] Function to compare two list items.
**
** @valrule * [AjBool] True on success
** @valrule *Length [ajuint] List length
** @valrule *Mapfind [AjBool] True if function returns true
** @valrule *Mapread [void]
** @valrule *Toarray [ajuint] Array size, excluding final NULL
** @valrule *Toindex [ajuint] Array size, excluding final NULL
**
** @fcategory cast
**
******************************************************************************/




/* @func ajListGetLength ******************************************************
**
** get the number of nodes in the linked list.
**
** @param [r] list [const AjPList] List
** @return [ajuint] Number of nodes in list.
** @@
******************************************************************************/

ajuint ajListGetLength(const AjPList list)
{
    if(!list)
	return 0;

    return list->Count;
}




/* @obsolete ajListLength
** @rename ajListGetLength
*/

__deprecated ajuint ajListLength(const AjPList list)
{
    return ajListGetLength(list);
}




/* @func ajListMapfind *****************************************************
**
** For each node in the list call function 'apply' and return
** ajTrue when any node is matched by the function.
**
** @param [r] list [const AjPList] List
** @param [f] apply [AjBool function] Function to call to test each list item.
** @param [u] cl [void*] Standard, usually NULL.
** @return [AjBool] ajTrue on success.
** @@
******************************************************************************/

AjBool ajListMapfind(const AjPList list,
		     AjBool apply(void** x, void* cl), void* cl)
{
    AjPListNode listnode;

    assert(list);

    assert(apply);

    for( listnode = list->First; listnode->Next; listnode = listnode->Next)
	if(apply(&listnode->Item, cl))
	    return ajTrue;

    return ajFalse;
}




/* @obsolete ajListFind
** @rename ajListMapfind
*/

__deprecated AjBool ajListFind(const AjPList list,
		  AjBool apply(void** x, void* cl), void* cl)
{
    return ajListMapfind(list, apply, cl);
}




/* @func ajListMapread ********************************************************
**
** For each node in the list call function apply.
** The apply function must not modify the list elements.
**
** @param [r] list [const AjPList] List.
** @param [f] apply [void function] Function to call for each list item.
** @param [u] cl [void*] Standard, usually NULL.
** @return [void]
** @@
******************************************************************************/

void ajListMapread(const AjPList list,
		   void apply(void* x, void* cl), void* cl)
{
    AjPListNode rest;

    assert(apply);

    for(rest = list->First; rest->Next; rest = rest->Next)
	apply((void*) rest->Item, cl);

    return;
}




/* @obsolete ajListMapRead
** @rename ajListMapread
*/

__deprecated void ajListMapRead(const AjPList list,
		   void apply(void* x, void* cl), void* cl)
{
    ajListMapread(list, apply, cl);
    return;
}




/* @func ajListPeek ***********************************************************
**
** Return the first node but keep it on the list
**
** @param [r] list [const AjPList] List
** @param [w] x [void**] pointer to pointer to data
** @return [AjBool] ajTrue on success.
** @@
******************************************************************************/

AjBool ajListPeek(const AjPList list, void** x)
{
    if(!list)
	return ajFalse;

    if(!list->Count)
      return ajFalse;

    if(x)
	*x = listNodeItem(list->First);

    return ajTrue;
}




/* @func ajListPeekFirst ******************************************************
**
** Set pointer to first node's data. Does NOT remove the first node.
**
** @param [r] list [const AjPList] List
** @param [w] x [void**] pointer to pointer to data
** @return [AjBool] ajTrue on success.
** @@
******************************************************************************/

AjBool ajListPeekFirst(const AjPList list, void** x)
{
    if(!list)
	return ajFalse;

    if(!list->Count)
        return ajFalse;

  if(x)
	*x = listNodeItem(list->First);

    return ajTrue;
}




/* @obsolete ajListFirst
** @rename ajListPeekFirst
*/

__deprecated AjBool ajListFirst(const AjPList list, void** x)
{
    return ajListPeekFirst(list, x);
}




/* @func ajListPeekLast *******************************************************
**
** Set pointer to last node's data. Does NOT remove the last node.
**
** @param [r] list [const AjPList] List
** @param [w] x [void**] pointer to pointer to data
** @return [AjBool] ajTrue on success.
** @@
******************************************************************************/

AjBool ajListPeekLast(const AjPList list, void** x)
{
    AjPListNode rest;

    if(!list)
	return ajFalse;

    if(!list->Count)
	return ajFalse;

    if(list->Last)
	rest = list->Last->Prev;
    else
	rest = list->First;

    if(x)
	*x = listNodeItem(rest);

    return ajTrue;
}




/* @obsolete ajListLast
** @rename ajListPeekLast
*/

__deprecated AjBool ajListLast(const AjPList list, void** x)
{
    return ajListPeekLast (list, x);
}




/* @func ajListPeekNumber ******************************************************
**
** Set pointer to last node's nth data item. 0 <= n < number of elements.
**
** @param [r] list [const AjPList] List
** @param [r] ipos [ajuint] element of the list
** @param [w] x [void**] pointer to pointer to data
** @return [AjBool] ajTrue on success.
** @@
******************************************************************************/

AjBool ajListPeekNumber(const AjPList list, ajuint ipos, void** x)
{
    AjPListNode rest;
    ajuint len;
    ajuint i;

    if(!list)
	return ajFalse;

    len = ajListGetLength(list);

    if(ipos>=len)
	return ajFalse;

    for(i=0,rest = list->First; i<ipos ; rest = rest->Next)
	i++;

    if(x)
	*x = listNodeItem(rest);

    return ajTrue;
}




AjBool ajListNth(const AjPList list, ajuint ipos, void** x)
{
    return ajListPeekNumber(list, ipos, x);
}




/* @func ajListToarray ********************************************************
**
** Create an array of the pointers to the data.
**
** @param [r] list [const AjPList] List
** @param [w] array [void***] Array of pointers to list items.
** @return [ajuint] Size of array of pointers.
** @@
******************************************************************************/

ajuint ajListToarray(const AjPList list, void*** array)
{
    ajint i;
    ajint n;
    AjPListNode rest;

    n = list->Count;
    rest = list->First;

    if(!n)
    {
	*array = NULL;

	return 0;
    }

    if (*array)
	AJFREE(*array);

    *array = AJALLOC((n+1)*sizeof(array));

    for(i = 0; i < n; i++)
    {
	(*array)[i] = rest->Item;
	rest = rest->Next;
    }

    (*array)[n] = 0;

    return n;
}




/* @obsolete ajListToArray
** @rename ajListToarray
*/

__deprecated ajuint ajListToArray(const AjPList list, void*** array)
{
    return ajListToarray(list, array);
}




/* @funcstatic listArrayTrace *************************************************
**
** Writes debug messages to trace an array generated from a list.
**
** @param [r] array [void**] Array to trace
** @return [void]
** @@
******************************************************************************/

static void listArrayTrace(void** array)
{
    void** v = array;
    ajint i  = 0;

    while(*v)
	ajDebug("array[%d] %x\n", i++, *v++);

    return;
}




/* @func ajListToindex ********************************************************
**
** Create an array of the pointers to the data.
**
** @param [r] list [const AjPList] List
** @param [w] lind [ajuint*] Populated ndex array to be sorted
** @param [f] sort1 [int* function] Function to compare two list items.
** @return [ajuint] Size of index array.
** @@
******************************************************************************/

ajuint ajListToindex(const AjPList list, ajuint* lind,
                     int (*sort1) (const void*, const void*))
{
    ajuint n;
    ajuint s;
    ajuint i;
    ajint j;
    ajuint t;

    AjPListNode *nodes = NULL;
    ajuint* idx = NULL;

    n = list->Count;

    if(!n)
        return 0;

    ajListToarray(list, (void***) &nodes);
    AJCNEW0(idx, n);

    for(i = 0; i < n; i++)
        idx[i] = i;

    for(s=n/2; s>0; s /= 2)
	for(i=s; i<n; ++i)
        {
	    for(j=i-s;
                j>=0 && (sort1(&nodes[idx[j]],&nodes[idx[j+s]]) > 0);
                j-=s)
	    {
		t = lind[j];
		lind[j] = lind[j+s];
		lind[j+s] = t;
		t = idx[j];
		idx[j] = idx[j+s];
		idx[j+s] = t;
	    }
        }
            
    AJFREE(nodes);
    AJFREE(idx);

    return n;
}




/* @section Trace functions ***************************************************
**
** @fdata [AjPList]
**
** @nam3rule Probe Test for memory allocation conflicts
** @nam4rule ProbeData Test data for memory allocation conflicts
** @nam3rule Print Trace contents to standard error
** @nam3rule Trace Trace contents to debug file
**
** @argrule Probe Plist [AjPList const*] Pointer to list
** @argrule Trace list  [const AjPList]  List
**
** @valrule * [void]
**
** @fcategory misc
**
******************************************************************************/




/* @func ajListProbe ***********************************************************
**
** Test list for memory allocation conflicts
**
** @param [r] Plist [AjPList const*] List
** @return [void]
** @@
******************************************************************************/

void ajListProbe(AjPList const * Plist)
{
    AjPListNode next;
    AjPListNode *rest;
    AjPList list;

    if(!Plist)
	return;

    if(!*Plist)
	return;

    list = *Plist;
    rest = &list->First;

    /* don't free the data in the list (we don't know how) */
    /* just free the nodes */
    if(list->Count)
	for( ; (*rest)->Next; *rest = next)
	{
	    next = (*rest)->Next;
	    AJMPROBE(*rest);
	}

    AJMPROBE(*rest);
    AJMPROBE(*Plist);

    return;
}




/* @func ajListProbeData *******************************************************
**
** Test list and data for memory allocation conflicts
**
** @param [r] Plist [AjPList const*] List
** @return [void]
** @@
******************************************************************************/

void ajListProbeData(AjPList const * Plist)
{
    AjPListNode next;
    AjPListNode *rest;
    AjPList list;

    if(!Plist)
	return;

    if(!*Plist)
	return;

    list = *Plist;
    rest = &list->First;


    /* free the data for each node (just a simple free) */
    /* as we free the nodes */

    if(list->Count)
    {
	for( ; (*rest)->Next; *rest = next)
	{
	    AJMPROBE((*rest)->Item);
	    next = (*rest)->Next;
	    AJMPROBE(*rest);
	}
	AJMPROBE((*rest)->Item);
    }

    AJMPROBE(*rest);
    AJMPROBE(*Plist);

    return;
}




/* @func ajListTrace **********************************************************
**
** Traces through a list and validates it
**
** @param [r] list [const AjPList] list to be traced.
** @return [void]
** @category output [AjPList] Traces through a list and validates it
** @@
******************************************************************************/

void ajListTrace(const AjPList list)
{
    ajuint i = 0;
    AjPListNode node;

    if(!list)
	return;

    ajDebug("\nList Trace %x type %d count %d\n",
	    list, list->Type, list->Count);
    ajDebug("first-> %x last-> %x\n", list->First, list->Last);

    for(node=list->First; node->Next; node=node->Next)
    {
	i++;
	ajDebug("Item[%d] item %x (data %x) rest -> %x prev -> %x\n",
		i, node, node->Item, node->Next, node->Prev);
    }

    if(i != list->Count)
    {
	ajDebug("*** list error expect %d items, found %d\n",
		list->Count, i);
	ajErr("*** list error expect %d items, found %d",
	      list->Count, i);
    }

    if(list->Last != node)
    {
	ajDebug("*** list error expect end at %x, found at %x\n",
		list->Last, node);
	ajErr("*** list error expect end at %x, found at %x",
	      list->Last, node);
    }

    return;
}




/* @obsolete ajListNodesNew
** @remove Use calls to ajListPush
*/
__deprecated AjPListNode ajListNodesNew(void* x, ...)
{
    va_list ap;
    AjPListNode topnode;
    AjPListNode node;

    va_start(ap, x);

    topnode = listDummyNode(&node);

    /*ajDebug("ajListNodesNew topnode: %x -> %x\n",
      topnode, topnode->Next);*/
    for( ; x; x = va_arg(ap, void *))
    {
	node->Item = x;
	listDummyNode(&node->Next);
	node->Next->Prev = node;
	/*ajDebug("topnode: %x node: %x, item: %x -> %x\n",
		topnode, node, x, node->Next);*/
    }

    va_end(ap);

    topnode->Prev = NULL;

    /*listNodesTrace(node);*/

    return node;
}




/* @funcstatic listNodesTrace *************************************************
**
** Writes debug messages to trace from the current list node.
**
** @param [r] node [const AjPListNode] Current node.
** @return [void]
** @@
******************************************************************************/

static void listNodesTrace(const AjPListNode node)
{
    const AjPListNode p = node;

    ajDebug("listNodesTrace %x\n", p);

    while(p->Next)
    {
	ajDebug("node %x item %x -> %x\n", p, p->Item, p->Next);
	p = p->Next;
    }

    return;
}




/* @obsolete ajListAppend
** @remove append data with ajListPushAppend
*/

__deprecated void ajListAppend(AjPList list, AjPListNode* morenodes)
{
    AjPListNode more = *morenodes;

    assert(list);

    /*listNodesTrace(*morenodes);*/

    more->Next->Prev = list->Last;
    list->Last->Next = more->Next;
    list->Last->Item = more->Item;

    while(more->Next)
    {				/* need to get to the end of the list */
	more = more->Next;
	list->Count++;
#ifdef AJ_SAVESTATS
	if(list->Count > listMaxNum)
	    listMaxNum = list->Count;
#endif
    }

    list->Last = more;		/* now we can set the end of the list */
    AJFREE(*morenodes);		/* first extra node (only) was duplicated */

    return;
}




/* @funcstatic listNodeItem ***************************************************
**
** Return the data item for a list node.
**
** @param [r] node  [const AjPListNode] Current node.
** @return [void*] Data item.
** @@
******************************************************************************/

static void* listNodeItem(const AjPListNode node)
{
    if(!node || !node->Next)
	return NULL;

    return node->Item;
}




/* @section Destructors *******************************************************
**
** @fdata [AjPList]
**
** Destructors can only deal with the list and cannot free complex data
**
** @nam3rule Del Delete list
** @nam3rule Free Delete list
** @nam4rule FreeData Delete list and free values
**
** @argrule * Plist [AjPList*] List
**
** @valrule * [void]
**
** @fcategory delete
**
******************************************************************************/




/* @func ajListFree ***********************************************************
**
** Free all nodes in the list.
** NOTE: The data is only freed with a specified list type.
**       For undefined data types we recommend you to
**       use ajListMap with a routine to free the memory.
**
** @param [d] Plist [AjPList*] List
** @return [void]
** @@
******************************************************************************/

void ajListFree(AjPList* Plist)
{
    AjPListNode next;
    AjPListNode *rest;
    AjPList list;

    if(!Plist)
	return;

    if(!*Plist)
	return;

#ifdef AJ_SAVESTATS
    listDelCnt++;
#endif

    list = *Plist;
    rest = &list->First;

    /* don't free the data in the list (we don't know how) */
    /* just free the nodes */
    if(list->Count)
	for( ; (*rest)->Next; *rest = next)
	{
	    next = (*rest)->Next;

            if(listFreeNext >= listFreeMax)
                listFreeSetExpand();

            if(listFreeNext >= listFreeMax)
                AJFREE(*rest);
            else if(*rest)
            {
                listFreeSet[listFreeNext++] = *rest;
                *rest = NULL;
            }
	}

    if(listFreeNext >= listFreeMax)
        listFreeSetExpand();

    if(listFreeNext >= listFreeMax)
        AJFREE(*rest);
    else if(*rest)
    {
        listFreeSet[listFreeNext++] = *rest;
        *rest = NULL;
    }
    
    AJFREE(*Plist);

    return;
}




/* @func ajListFreeData *******************************************************
**
** Free all nodes in the list. Free all the data values.
** For more complex data objects use ajListMap with a routine to
** free the object memory.
**
** @param [d] Plist [AjPList*] List
** @return [void]
** @@
******************************************************************************/

void ajListFreeData(AjPList* Plist)
{
    AjPListNode next;
    AjPListNode *rest;
    AjPList list;

    if(!Plist)
	return;

    if(!*Plist)
	return;

#ifdef AJ_SAVESTATS
    listDelCnt++;
#endif

    list = *Plist;
    rest = &list->First;


    /* free the data for each node (just a simple free) */
    /* as we free the nodes */

    if(list->Count)
    {
	for( ; (*rest)->Next; *rest = next)
	{
	    AJFREE((*rest)->Item);
	    next = (*rest)->Next;

            if(listFreeNext >= listFreeMax)
                listFreeSetExpand();

            if(listFreeNext >= listFreeMax)
                AJFREE(*rest);
            else if(*rest)
            {
                listFreeSet[listFreeNext++] = *rest;
                *rest = NULL;
            }
        }
	AJFREE((*rest)->Item);
    }

    if(listFreeNext >= listFreeMax)
        listFreeSetExpand();

    if(listFreeNext >= listFreeMax)
        AJFREE(*rest);
    else if(*rest)
    {
        listFreeSet[listFreeNext++] = *rest;
        *rest = NULL;
    }

    AJFREE(*Plist);

    return;
}




/* @obsolete ajListDel
** @rename ajListFree
*/

__deprecated void ajListDel(AjPList* Plist)
{
    AjPList list;
    AjPListNode *rest = NULL;
    AjPListNode next  = NULL;

    if(!Plist)
	return;

    if(!*Plist)
	return;

#ifdef AJ_SAVESTATS
    listDelCnt++;
#endif

    list = *Plist;
    rest = &list->First;

    if(list->Count)
	for( ; (*rest)->Next; *rest = next)
	{
	    next = (*rest)->Next;

            if(listFreeNext >= listFreeMax)
                listFreeSetExpand();

            if(listFreeNext >= listFreeMax)
                AJFREE(*rest);
            else if(*rest)
            {
                listFreeSet[listFreeNext++] = *rest;
                *rest = NULL;
            }
        }

    if(listFreeNext >= listFreeMax)
        listFreeSetExpand();

    if(listFreeNext >= listFreeMax)
        AJFREE(*rest);
    else if(*rest)
    {
        listFreeSet[listFreeNext++] = *rest;
        *rest = NULL;
    }

    AJFREE(*Plist);

    return;
}




/* @funcstatic listNodeDel ****************************************************
**
** Remove a node from the list.
**
** @param [d] pnode  [AjPListNode*] Current node.
** @return [AjBool] ajTrue on success.
** @@
******************************************************************************/

static AjBool listNodeDel(AjPListNode * pnode)
{
    AjPListNode node;
    AjPListNode tmp;

    node = *pnode;

    if(!node || !node->Next)
	return ajFalse;

    tmp = node->Prev;
    node = node->Next;
    node->Prev = tmp;

    if(listFreeNext >= listFreeMax)
        listFreeSetExpand();

    if(listFreeNext >= listFreeMax)
        AJFREE(*pnode);
    else if(*pnode)
        listFreeSet[listFreeNext++] = *pnode;
 
    *pnode = node;

    return ajTrue;
}




/* @section unused ************************************************************
**
** @fdata [AjPList]
**
** @nam3rule Unused Contains dummy calls to unused functions to keep
**                 compilers happy
**
** @argrule Unused array [void**] Dummy array
**
** @valrule * [void]
** @fcategory misc
**
******************************************************************************/




/* @func ajListUnused *********************************************************
**
** Dummy function to catch all unused functions defined in ajlist
**
** @param [r] array [void**] Array needed by ajListArrayTrace
** @return [void]
******************************************************************************/

void ajListUnused(void** array)
{
    const AjPListNode p = NULL;
    listNodesTrace(p);
    listArrayTrace(array);

    return;
}




/* @section exit **************************************************************
**
** Functions called on exit from the program by ajExit to do
** any necessary cleanup and to report internal statistics to the debug file
**
** @fdata      [AjPList]
** @fnote     general exit functions, no arguments
**
** @nam3rule Exit Cleanup and report on exit
**
** @valrule * [void]
**
** @fcategory misc
*/




/* @func ajListExit ***********************************************************
**
** Prints a summary of list usage with debug calls
**
** @return [void]
** @@
******************************************************************************/

void ajListExit(void)
{
    ajint i;

#ifdef AJ_SAVESTATS
    ajDebug("List usage : %d opened, %d closed, %d maxsize %d nodes\n",
	     listNewCnt, listDelCnt, listMaxNum, listNodeCnt);
    ajDebug("List iterator usage : %d opened, %d closed\n",
	     listIterNewCnt, listIterDelCnt);
#endif
    if(listFreeNext)
        for(i=0;i<listFreeNext;i++)
            AJFREE(listFreeSet[i]);

            
    if(listFreeSet) 
        AJFREE(listFreeSet);

    listFreeNext = 0;
    listFreeMax = 0;
    
    return;
}




/* @datasection [AjIList] List iterators **************************************
**
** Function is for manipulating lists with any value type.
**
** Some functions are specially designed to understand string (AjPStr) values.
**
** @nam2rule List List data
** @nam3rule Iter Iterator
**
*******************************************************************************/




/* @section constructors ******************************************************
**
** @fdata [AjIList]
**
** @nam4rule New      List iterator constructor.
** @nam5rule NewBack  List iterator reverse direction constructor.
** @nam4rule Newread  List iterator read-only constructor.
** @nam5rule NewreadBack  List iterator read-only reverse direction constructor.
**
** @argrule New list [AjPList] Original list
** @argrule Newread list [const AjPList] Original list
**
** @valrule * [AjIList] List iterator
**
** @fcategory new
*/




/* @func ajListIterNew ********************************************************
**
** Creates an iterator to operate from start to end of list.
**
** @param [u] list [AjPList] List
**                 Not const in practice - the iterator can insert
**                 and delete entries
** @return [AjIList] New list iterator
** @@
******************************************************************************/

AjIList ajListIterNew(AjPList list)
{
    AjIList iter;

    if(!list)
	return NULL;

    AJNEW0(iter);
    iter->Head = list;
    iter->Back = ajFalse;
    iter->Here = list->First;
    iter->Modify = ajTrue;

#ifdef AJ_SAVESTATS
    listIterNewCnt++;
#endif

    return iter;
}




/* @obsolete ajListIter
** @rename ajListIterNew
*/

__deprecated AjIList ajListIter(AjPList list)
{
    return ajListIterNew(list);
}




/* @func ajListIterNewBack *****************************************************
**
** Creates an iterator to operate from end to start of the list.
**
** @param [u] list [AjPList] List
**                 Not const - the iterator can insert and delete entries
** @return [AjIList] New list iterator
** @@
******************************************************************************/

AjIList ajListIterNewBack(AjPList list)
{
    AjIList iter;
    AjPListNode node = NULL;
    AjPListNode tmp  = NULL;

    if(!list)
	return NULL;

    if(!list->Count)
	return NULL;

    for(node=list->First; node->Next; node = node->Next)
	tmp = node;

    list->Last->Prev = tmp;

    AJNEW0(iter);
    iter->Head = list;
    iter->Back = ajTrue;
    iter->Here = tmp->Next;
    iter->Modify = ajTrue;

#ifdef AJ_SAVESTATS
    listIterNewCnt++;
#endif

    return iter;
}




/* @obsolete ajListIterBack
** @rename ajListIterNewBack
*/

__deprecated AjIList ajListIterBack(AjPList list)
{
    return ajListIterNewBack(list);
}




/* @func ajListIterNewread *****************************************************
**
** Creates an iterator to operate from start to end of list.
**
** @param [r] list [const AjPList] List
**                 Not const in practice - the iterator can insert
**                 and delete entries
** @return [AjIList] New list iterator
** @@
******************************************************************************/

AjIList ajListIterNewread(const AjPList list)
{
    AjIList iter;

    if(!list)
	return NULL;

    AJNEW0(iter);
    iter->ReadHead = list;
    iter->Back = ajFalse;
    iter->Here = list->First;
    iter->Modify = ajFalse;

#ifdef AJ_SAVESTATS
    listIterNewCnt++;
#endif

    return iter;
}




/* @obsolete ajListIterRead
** @rename ajListIterNewread
*/

__deprecated AjIList ajListIterRead(const AjPList list)
{
    return ajListIterNewread(list);
}




/* @func ajListIterNewreadBack *************************************************
**
** Creates an iterator to operate from end to start of the list.
**
** @param [r] list [const AjPList] List
** @return [AjIList] New list iterator
** @@
******************************************************************************/

AjIList ajListIterNewreadBack(const AjPList list)
{
    AjIList iter;
    AjPListNode node = NULL;
    AjPListNode tmp  = NULL;

    if(!list)
	return NULL;

    if(!list->Count)
	return NULL;

    for(node=list->First; node->Next; node = node->Next)
	tmp = node;

    list->Last->Prev = tmp;

    AJNEW0(iter);
    iter->ReadHead = list;
    iter->Back = ajTrue;
    iter->Here = tmp->Next;
    iter->Modify = ajFalse;

#ifdef AJ_SAVESTATS
    listIterNewCnt++;
#endif

    return iter;
}




/* @obsolete ajListIterBackRead
** @rename ajListIterNewreadBack
*/

__deprecated AjIList ajListIterBackRead(const AjPList list)
{
    return ajListIterNewreadBack(list);
}




/* @section tests *************************************************************
** @fdata [AjIList]
** @nam4rule   Done      Check whether iteration has ended (no more
**                             characters).
** @nam5rule   DoneBack  Reverse iterator
**
** @argrule Done iter [const AjIList] Iterator
**
** @valrule * [AjBool] Result of test
**
** @fcategory use
*/




/* @func ajListIterDone *******************************************************
**
** Tests whether an iterator has completed yet.
**
** @param [r] iter [const AjIList] List iterator.
** @return [AjBool] ajTrue if the iterator is exhausted.
** @@
******************************************************************************/

AjBool ajListIterDone(const AjIList iter)
{
    AjPListNode p;

    if(!iter)
	return ajTrue;

    p = iter->Here;

    if(!iter->Back)
    {
	if(!p->Next)
	    return ajTrue;
    }
    else
	if(!p->Next->Next || !p->Next->Next->Next)
	    return ajTrue;

    return ajFalse;
}




/* @obsolete ajListIterMore
** @remove use ajListIterDone
*/

__deprecated AjBool ajListIterMore(const AjIList iter)
{
    return (!ajListIterDone(iter));
}




/* @obsolete ajListIterMoreBack
** @remove use ajListIterDoneBack
*/

__deprecated AjBool ajListIterMoreBack(const AjIList iter)
{
    return (!ajListIterDoneBack(iter));
}




/* @func ajListIterDoneBack ***************************************************
**
** Tests whether a backwards iterator has completed yet.
**
** @param [r] iter [const AjIList] List iterator.
** @return [AjBool] ajTrue if the iterator is exhausted.
** @@
******************************************************************************/

AjBool ajListIterDoneBack(const AjIList iter)
{
    AjPListNode p;

    if(!iter)
	return ajTrue;

    p = iter->Here;

    if(!p->Prev)
	return ajTrue;

    return ajFalse;
}




/* @obsolete ajListIterBackDone
** @rename ajListIterDoneBack
*/

__deprecated AjBool ajListIterBackDone(const AjIList iter)
{
    return ajListIterDoneBack(iter);
}




/* @section destructors *******************************************************
** @fdata [AjIList]
**
** @nam4rule Del Destructor
**
** @argrule Del iter [AjIList*] List iterator
**
** @valrule * [void]
**
** @fcategory delete
*/




/* @func ajListIterDel *******************************************************
**
** Destructor for a list iterator.
**
** @param [d] iter [AjIList*] List iterator.
** @return [void]
** @@
******************************************************************************/

void ajListIterDel(AjIList* iter)
{
    AJFREE(*iter);

#ifdef AJ_SAVESTATS
    listIterDelCnt++;
#endif

    return;
}




/* @obsolete ajListIterFree
** @rename ajListIterDel
*/

__deprecated void ajListIterFree(AjIList* iter)
{
    ajListIterDel(iter);
    return;
}




/* @obsolete ajListIterBackMore
** @remove use ajListIterDoneBack
*/

__deprecated AjBool ajListIterBackMore(const AjIList iter)
{
    return (!ajListIterDoneBack(iter));
}




/* @section stepping **********************************************************
**
** @fdata [AjIList]
**
** @nam4rule   Get      Next value.
** @nam5rule   GetBack  Next value in reverse direction
**
** @argrule * iter [AjIList] String iterator
**
** @valrule * [void*] Value
**
** @fcategory modify
*/




/* @func ajListIterGet ********************************************************
**
** Returns next item using iterator, or steps off the end.
**
** @param [u] iter [AjIList] List iterator.
** @return [void*] Data item returned.
** @@
******************************************************************************/

void* ajListIterGet(AjIList iter)
{
    AjPListNode p;
    void *ret;

    if(!iter)
	return NULL;

    p = iter->Here;

    if(!iter->Back)
    {
	if(!p->Next)
	    return NULL;

	ret = p->Item;
	iter->Here = p->Next;
    }
    else
    {
	if(!p->Next->Next || !p->Next->Next->Next)
	  return NULL;

	iter->Back = ajFalse;
	ret = p->Next->Item;
	iter->Here = p->Next->Next;
    }

    return ret;
}




/* @obsolete ajListIterNext
** @rename ajListIterGet
*/

__deprecated void* ajListIterNext(AjIList iter)
{
    return ajListIterGet(iter);
}




/* @func ajListIterGetBack *************************************************
**
** Returns next item using back iterator.
**
** @param [u] iter [AjIList] List iterator.
** @return [void*] Data item returned.
** @@
******************************************************************************/

void* ajListIterGetBack(AjIList iter)
{
    AjPListNode p;
    void* ret;

    if(!iter)
	return NULL;

    p = iter->Here;

    if(!p->Prev)
      return NULL;

    if(!iter->Back && p->Prev->Prev)
    {
        ret = p->Prev->Prev->Item;
        iter->Here = p->Prev->Prev;
        iter->Back = ajTrue;
    }
    else
    {
	ret = p->Prev->Item;
	iter->Here = p->Prev;
    }

    return ret;
}




/* @obsolete ajListIterBackNext
** @rename ajListIterGetBack
*/

__deprecated void* ajListIterBackNext(AjIList iter)
{
    return ajListIterGetBack(iter);
}




/* @section modifiers **********************************************************
**
** @fdata [AjIList]
**
** @nam4rule   Insert  Insert a new value at iterator position
** @nam4rule   Remove  Remove value at iterator position
** @nam4rule   Rewind  Rewind list to start
**
** @argrule * iter [AjIList] String iterator
** @argrule Insert x [void*] Value
**
** @valrule * [void]
**
** @fcategory modify
*/




/* @func ajListIterInsert ******************************************************
**
** Insert an item in a list, using an iterator (if not null)
** to show which position to insert. Otherwise, simply push.
**
** @param [u] iter [AjIList] List iterator.
** @param [u] x [void*] Data item to insert.
** @return [void]
** @@
******************************************************************************/

void ajListIterInsert(AjIList iter, void* x)
{
    AjPList list = iter->Head;
    AjPListNode p;

    /* ajDebug("ajListInsert\n");*/

    if (!iter->Modify)
    {
	ajErr("ajListIterInsert called on a read-only iterator");
	return;
    }

    p = iter->Here;


    if(!iter->Back)
    {
	if(!p->Prev)
	    listInsertNode(&list->First,x);
	else
	    listInsertNode(&p->Prev->Next,x);

	iter->Here = p->Prev;
    }
    else
    {
	if(!p->Next)
	    ajFatal("Cannot add a new node for unused back iterator\n");

	if(!p->Prev)
	    listInsertNode(&list->First,x);
	else
	    listInsertNode(&p->Prev->Next,x);
    }

    list->Count++;
#ifdef AJ_SAVESTATS
    if(list->Count > listMaxNum)
	listMaxNum = list->Count;
#endif

    /*ajListTrace(list);*/
    /*ajListIterTrace(iter);*/

    return;
}




/* @obsolete ajListInsert
** @rename ajListIterInsert
*/

__deprecated void ajListInsert(AjIList iter, void* x)
{
    ajListIterInsert(iter, x);
    return;
}




/* @func ajListIterRemove *****************************************************
**
** Remove an item from a list, using an iterator (if not null)
** to show which item. Otherwise remove the first item.
**
** We want to remove the item just fetched by the iterator.
**
** @param [u] iter [AjIList] List iterator.
** @return [void]
** @@
******************************************************************************/

void ajListIterRemove(AjIList iter)
{
    AjPListNode p;

    /* ajDebug("ajListIterRemove\n");*/

    if(!iter->Modify)
    {
	ajDie("Attempt to modify read-only iterator with ajListIterRemove\n");
	return;
    }

    p = iter->Here;

    if(!iter->Back)
    {
	if(!p->Prev)
	    ajDie("Attempt to delete from unused iterator\n");

	if(!p->Prev->Prev)
	    listNodeDel(&(iter->Head->First));
	else
	    listNodeDel(&p->Prev->Prev->Next);
    }
    else
	listNodeDel(&p->Prev->Prev->Next);


    iter->Head->Count--;

    return;
}




/* @obsolete ajListRemove
** @rename ajListIterRemove
*/

__deprecated void ajListRemove(AjIList iter)
{

    ajListIterRemove(iter);
    return;
}




/* @func ajListIterRewind ******************************************************
**
** Resets iterator to start position
**
** @param [u] iter [AjIList] List iterator.
** @return [void]
** @@
******************************************************************************/

void ajListIterRewind(AjIList iter)
{
    AjPListNode node = NULL;
    AjPListNode tmp  = NULL;
    const AjPList list;

    if(!iter)
        return;

    if(iter->Modify)
        list = iter->Head;
    else
        list = iter->ReadHead;

    if(iter->Back)
    {
        for(node=list->First; node->Next; node = node->Next)
            tmp = node;

        iter->Here = tmp->Next;
    }
    else
    {
        iter->Here = list->First;
    }
    
    return;
}




/* @section Trace functions ***************************************************
**
** @fdata [AjIList]
**
** @nam4rule Trace Trace contents to debug file
**
** @argrule * iter [const AjIList] List
**
** @valrule * [void]
**
** @fcategory misc
**
******************************************************************************/




/* @func ajListIterTrace ******************************************************
**
** Traces a list iterator and validates it.
**
** @param [r] iter [const AjIList] list iterator to be traced.
** @return [void]
** @@
******************************************************************************/

void ajListIterTrace(const AjIList iter)
{
    ajuint icount = 0;

    if(!iter)
    {
	ajDebug("\nIterator NULL\n");
	return;
    }

    if(iter->Head)
	icount = iter->Head->Count;
    else if(iter->ReadHead)
	icount = iter->ReadHead->Count;

    ajDebug("\nIter Head %x ReadHead %x Here %x Back %B Modify %B Len: %d\n",
	    iter->Head,iter->ReadHead,iter->Here,iter->Back,
	    iter->Modify, icount);

    return;
}




/* @datasection [AjPList] String lists *****************************************
**
** Functions working on lists of string values
**
** @nam2rule Liststr
**
******************************************************************************/




/* @section Constructors ******************************************************
**
** Constructors for lists
**
** @fdata [AjPList]
**
** @fcategory new
**
** @nam3rule New Constructor
** @nam4rule NewList Copy constructor with its own copy of the data
** @nam4rule NewListref Copy constructor with pointers to source data
**
** @argrule NewList list [const AjPList] Source list
** @argrule NewListref list [const AjPList] Source list
**
** @valrule * [AjPList] New list
**
******************************************************************************/




/* @func ajListstrNew *********************************************************
**
** Creates a new string list.
**
** @return [AjPList] new list;
** @@
******************************************************************************/

AjPList ajListstrNew(void)
{
    return listNew(ajEListStr);
}




/* @func ajListstrNewList ******************************************************
**
** Copy a list, with copies of all the string values.
**
** General data cannot be copied so this is a string-only function
**
** @param [r] list [const AjPList] list to be copied
** @return [AjPList] new copied list.
** @@
******************************************************************************/

AjPList ajListstrNewList(const AjPList list)
{
    AjPList newlist;

    AjPListNode node;
    AjPStr tmpstr = NULL;

    if(!list)
	return NULL;

    newlist = ajListNew();
    newlist->Type = list->Type;

    for( node=list->First; node->Next; node=node->Next)
    {
	tmpstr = ajStrNewS(node->Item);
	ajListstrPushAppend(newlist, tmpstr);
    }


    return newlist;
}




/* @obsolete ajListstrClone
** @remove use ajListstrNewList
*/

__deprecated ajuint ajListstrClone(const AjPList list, AjPList newlist)
{
    AjPListNode node;
    ajint ret = 0;
    AjPStr newstr;

    if(!list)
	return 0;

    if(!newlist)
	return 0;

    for( node=list->First; node->Next; node=node->Next)
    {
        newstr = NULL;
	ajStrAssignS(&newstr, node->Item);
	ajListPushAppend(newlist, newstr);
	ret++;
    }

    return ret;
}




/* @func ajListstrNewListref **************************************************
**
** Copy a string list.
**
** WARNING: pointers to the data are copied, NOT the data
**          so be careful when cleaning up after copy.
**
** @param [r] list [const AjPList] List to be copied
** @return [AjPList] New, copied, list.
** @@
******************************************************************************/

AjPList ajListstrNewListref(const AjPList list)
{
    return ajListNewListref(list);
}




/* @obsolete ajListstrCopy
** @rename ajListstrNewList
*/

__deprecated AjPList ajListstrCopy(const AjPList list)
{
    return ajListstrNewListref(list);
}




/* @section Adding values ******************************************************
**
** @fdata [AjPList]
**
** @nam3rule Push Add new key and value
** @nam4rule PushAppend Add to end of list
** @nam3rule Pushlist Add new list of values
**
** @argrule * list [AjPList] List
** @argrule Push x [AjPStr] Pointer to data.
** @argrule Pushlist Plist [AjPList*] Source list to be deleted
**
** @valrule * [void]
**
** @fcategory modify
******************************************************************************/




/* @func ajListstrPush ********************************************************
**
** Add a new node at the start of a string list.
**
** @param [u] list [AjPList] list to be changed.
** @param [u] x [AjPStr] String data.
** @return [void]
** @@
******************************************************************************/

void ajListstrPush(AjPList list, AjPStr x)
{
    ajListPush(list, (void*) x);

    return;
}




/* @func ajListstrPushAppend ***************************************************
**
** Add a new node at the end of the list and add the
** data pointer.
**
** @param [u] list [AjPList] List to be changed.
** @param [u] x [AjPStr] String to append.
** @return [void]
** @@
******************************************************************************/

void ajListstrPushAppend(AjPList list, AjPStr x)
{
    ajListPushAppend(list, (void*) x);

    return;
}




/* @obsolete ajListstrPushApp
** @rename ajListstrPushAppend
*/

__deprecated void ajListstrPushApp(AjPList list, AjPStr x)
{

    ajListstrPushAppend(list, x);

    return;
}




/* @func ajListstrPushlist ****************************************************
**
** Adds a list to the start of the current list, then deletes the second list.
**
** @param [u] list [AjPList] List.
** @param [d] Plist [AjPList*] List to be merged.
** @return [void]
** @@
******************************************************************************/

void ajListstrPushlist(AjPList list, AjPList* Plist)
{
    ajListPushlist(list, Plist);

    return;
}




/* @obsolete ajListstrPushList
** @rename ajListstrPushlist
*/

__deprecated void ajListstrPushList(AjPList list, AjPList* Plist)
{
    ajListstrPushlist(list, Plist);

    return;
}




/* @section Modifiers **********************************************************
**
** @fdata [AjPList]
**
** Modifies a list
**
** @nam3rule Map Apply function to each node
** @nam3rule Reverse Reverse the order of nodes
**
** @argrule * list [AjPList] List
** @argrule Map apply [void function] Function to apply
** @argrule Map cl [void*] Data (usually NULL) to be passed to apply.
**
** @valrule * [void]
**
** @fcategory modify
**
******************************************************************************/




/* @func ajListstrMap *********************************************************
**
** For each node in the list call function apply,
** with the address of the string and a client pointer.
**
** @param [u] list [AjPList] List.
** @param [f] apply [void function] Function to call for each list item.
** @param [u] cl [void*] Standard, usually NULL.
** @return [void]
** @@
******************************************************************************/

void ajListstrMap(AjPList list, void apply(AjPStr* x, void* cl), void* cl)
{
    AjPListNode rest;

    assert(apply);

    for(rest=list->First; rest->Next; rest = rest->Next)
	apply((AjPStr*) &rest->Item, cl);

    return;
}




/* @func ajListstrReverse *****************************************************
**
** Reverse the order of the nodes in a string list.
**
** @param [u] list [AjPList] List
** @return [void]
** @@
******************************************************************************/

void ajListstrReverse(AjPList list)
{
    ajListReverse(list);

    return;
}




/* @section Removing data ******************************************************
**
** @fdata [AjPList]
**
** Retrieves values from a list
**
** @nam3rule Pop Retrieve next value
** @nam4rule PopLast Retrieve last value
**
** @argrule * list [AjPList] List
** @argrule Pop Pstr [AjPStr*] Value
**
** @valrule * [AjBool] True on success
** @fcategory cast
**
******************************************************************************/




/* @func ajListstrPop *********************************************************
**
** Remove the first node but set pointer to data first.
**
** @param [u] list [AjPList] List
** @param [w] Pstr [AjPStr*] String
** @return [AjBool] ajTrue on success.
** @@
******************************************************************************/

AjBool ajListstrPop(AjPList list, AjPStr* Pstr)
{
    if(!list)
	return ajFalse;

    if(Pstr)
	*Pstr = (AjPStr) listNodeItem(list->First);

    if(!listNodeDel(&list->First))
	return ajFalse;

    list->First->Prev = NULL;
    list->Count--;

    return ajTrue;
}




/* @func ajListstrPopLast *****************************************************
**
** Remove the last node but set pointer to data first.
**
** @param [u] list [AjPList] List
** @param [w] Pstr [AjPStr*] String
** @return [AjBool] ajTrue on success.
** @@
******************************************************************************/

AjBool ajListstrPopLast(AjPList list, AjPStr *Pstr)
{
    AjPListNode pthis = NULL;

    if(!list)
	return ajFalse;

    if(!list->Count)
	return ajFalse;

    pthis = list->Last->Prev;

    if(Pstr)
	*Pstr = (AjPStr) listNodeItem(pthis);


    if(list->Count==1)
    {
	list->Last->Prev = NULL;

        if(listFreeNext >= listFreeMax)
            listFreeSetExpand();

        if(listFreeNext >= listFreeMax)
            AJFREE(list->First);
        else if(list->First)
            listFreeSet[listFreeNext++] = list->First;
        
	list->First = list->Last;
    }
    else
    {
	pthis->Prev->Next = list->Last;
	list->Last->Prev = pthis->Prev;

        if(listFreeNext >= listFreeMax)
            listFreeSetExpand();

        if(listFreeNext >= listFreeMax)
            AJFREE(pthis);
        else if(pthis)
        {
            listFreeSet[listFreeNext++] = pthis;
            pthis = NULL;
        }
    }

    --list->Count;
    return ajTrue;
}




/* @obsolete ajListstrPopEnd
** @rename ajListstrPopLast
*/

__deprecated AjBool ajListstrPopEnd(AjPList list, AjPStr *x)
{
    return ajListstrPopLast(list, x);
}




/* @section Element retrieval **************************************************
**
** @fdata [AjPList]
**
** Retrieves attributes or values from a list without changing it
**
** @nam3rule Get Retrieve attribute
** @nam4rule GetLength Retrieve list length
** @nam3rule Mapfind Apply function to each node without modifying list
**                       or data
** @nam3rule Mapread Apply function to each node without modifying list or data
** @nam3rule Peek Pointer to next value
** @nam4rule PeekFirst Pointer to first value
** @nam4rule PeekLast Pointer to last value
** @nam4rule PeekNumber Pointer to numbered value
** @nam3rule Toarray Build array of values
** @nam4rule ToarrayAppend Append to array of values
**
** @argrule * list [const AjPList] List
** @argrule Mapfind apply [AjBool function] Function to apply
** @argrule Mapfind cl [void*] Data (usually NULL) to be passed to apply.
** @argrule Mapread apply [void* function] Function to apply
** @argrule Mapread cl [void*] Data (usually NULL) to be passed to apply.
** @argrule PeekNumber ipos [ajuint] Position in list
** @argrule Peek Pstr [AjPStr*] Value
** @argrule Toarray array [AjPStr**] Array of values, ending with NULL
**
** @valrule * [AjBool] True on success
** @valrule *Length [ajuint] List length
** @valrule *Mapfind [AjBool]
** @valrule *Mapread [void]
** @valrule *Toarray [ajuint] Array size, excluding final NULL
**
** @fcategory cast
**
******************************************************************************/




/* @func ajListstrGetLength ***************************************************
**
** get the number of nodes in the linked list.
**
** @param [r] list [const AjPList] List
** @return [ajuint] Number of nodes in list.
** @@
******************************************************************************/

ajuint ajListstrGetLength(const AjPList list)
{
    return ajListGetLength(list);
}




/* @obsolete ajListstrLength
** @rename ajListstrGetLength
*/

__deprecated ajuint ajListstrLength(const AjPList list)
{
    return ajListstrGetLength(list);
}




/* @func ajListstrMapfind **************************************************
**
** For each node in the list call function apply and return
** ajTrue when any node is matched by the function.
**
** @param [r] list [const AjPList] List
** @param [f] apply [AjBool function] Function to call to test each list item.
** @param [u] cl [void*] Standard, usually NULL.
** @return [AjBool] ajTrue on success.
** @@
******************************************************************************/

AjBool ajListstrMapfind(const AjPList list,
			AjBool apply(AjPStr* x, void* cl),
			void* cl)
{
    AjPListNode listnode;

    assert(list);

    assert(apply);

    for(listnode = list->First; listnode->Next; listnode = listnode->Next)
	if(apply((AjPStr*) &listnode->Item, cl))
	    return ajTrue;

    return ajFalse;
}




/* @obsolete ajListstrFind
** @rename ajListstrMapfind
*/

__deprecated AjBool ajListstrFind(const AjPList list,
				  AjBool apply(AjPStr* x, void* cl),
				  void* cl)
{
    return ajListstrMapfind(list, apply, cl);
}




/* @func ajListstrMapread *****************************************************
**
** For each node in the list call function apply,
** with the address of the string and a client pointer.
** The apply function must not modify the list elements.
**
** @param [r] list [const AjPList] List.
** @param [f] apply [void* function] Function to call for each list item.
** @param [u] cl [void*] Standard, usually NULL.
** @return [void]
** @@
******************************************************************************/

void ajListstrMapread(const AjPList list,
		      void (*apply)(AjPStr x, void* cl), void* cl)
{
    AjPListNode rest;

    assert(apply);

    for(rest=list->First; rest->Next; rest = rest->Next)
	apply((AjPStr) rest->Item, cl);


    return;
}




/* @obsolete ajListstrMapRead
** @rename ajListstrMapread
*/

__deprecated void ajListstrMapRead(const AjPList list,
		  void apply(AjPStr x, void* cl), void* cl)
{

    ajListstrMapread(list, apply, cl);

    return;
}




/* @func ajListstrPeek ********************************************************
**
** Return the first node but keep it on the list.
**
** @param [r] list [const AjPList] List
** @param [w] Pstr [AjPStr*] String
** @return [AjBool] ajTrue on success.
** @@
******************************************************************************/

AjBool ajListstrPeek(const AjPList list, AjPStr* Pstr)
{
    if(!list)
	return ajFalse;

    if(!list->Count)
	return ajFalse;

    if(Pstr)
	*Pstr = (AjPStr) listNodeItem(list->First);

    return ajTrue;
}




/* @func ajListstrToarray *****************************************************
**
** create an array of the pointers to the data.
**
** @param [r] list [const AjPList] List
** @param [w] array [AjPStr**] Array of Strings.
**
** @return [ajuint] Size of array of pointers.
** @@
******************************************************************************/

ajuint ajListstrToarray(const AjPList list, AjPStr** array)
{
    ajint i;
    ajint n;
    AjPListNode rest;

    n = list->Count;
    rest = list->First;

    if(!n)
    {
	*array = NULL;

	return 0;
    }

    *array = AJALLOC((n+1)*sizeof(array));

    for(i = 0; i < n; i++)
    {
	(*array)[i] = (AjPStr) rest->Item;
	rest = rest->Next;
    }

    (*array)[n] = 0;

    return n;
}




/* @obsolete ajListstrToArray
** @rename ajListstrToarray
*/

__deprecated ajuint ajListstrToArray(const AjPList list, AjPStr** array)
{
    return ajListstrToarray(list, array);
}




/* @func ajListstrToarrayAppend ************************************************
**
** append to an array of the pointers to the data.
**
** @param [r] list [const AjPList] List
** @param [w] array [AjPStr**] Array of Strings.
**
** @return [ajuint] Size of array of pointers.
**
** @@
******************************************************************************/

ajuint ajListstrToarrayAppend(const AjPList list, AjPStr** array)
{
    ajint i;
    ajint n;
    ajint j;
    AjPListNode rest;

    rest = list->First;

    if (*array)
    {
	for (j=0; array[j]; j++)
	    continue;
    }
    else
	j = 0;

    n = list->Count + j;

    if(!n)
    {
	*array = NULL;

	return 0;
    }

    AJCRESIZE(*array, (n+1));

    for(i = j; i < n; i++)
    {
	(*array)[i] = (AjPStr) rest->Item;
	rest = rest->Next;
    }

    (*array)[n] = 0;

    return n;
}




/* @obsolete ajListstrToArrayApp
** @rename ajListstrToarrayAppend
*/

__deprecated ajuint ajListstrToArrayApp(const AjPList list, AjPStr** array)
{
    return ajListstrToarrayAppend(list, array);
}




/* @section Trace functions ***************************************************
**
** @fdata [AjPList]
**
** @nam3rule Print Trace contents to standard error
** @nam3rule Trace Trace contents to debug file
**
** @argrule * list [const AjPList] List
**
** @valrule * [void]
**
** @fcategory misc
**
******************************************************************************/




/* @func ajListstrTrace *******************************************************
**
** Traces through a string list and validates it
**
** @param [r] list [const AjPList] list to be traced.
** @return [void]
** @@
******************************************************************************/

void ajListstrTrace(const AjPList list)
{
    ajuint i = 0;
    AjPListNode node;

    if(!list)
	return;

    ajDebug("\nList Trace %x type %d count %d\n",
	     list, list->Type, list->Count);
    ajDebug("rest-> %x last-> %x\n",
	    list->First, list->Last);

    for(node=list->First; node->Next; node=node->Next)
    {
	i++;
	ajDebug("Item[%d] item %x '%S' rest -> %x prev -> %x\n",
		 i, node, (AjPStr) node->Item, node->Next, node->Prev);
    }

    if(i != list->Count)
    {
	ajDebug("*** list error expect %d items, found %d\n",
		list->Count, i);
	ajErr("*** list error expect %d items, found %d",
	      list->Count, i);
    }

    if(list->Last != node)
    {
	ajDebug("*** list error expect end at %x, found at %x\n",
		list->Last, node);
	ajErr("*** list error expect end at %x, found at %x",
	      list->Last, node);
    }

    return;
}




/* @obsolete ajListstrNewArgs
** @remove Use ajListstrNew and ajListPush
*/
__deprecated AjPList ajListstrNewArgs(AjPStr x, ...)
{
    AjPList list;
    va_list ap;
    ajint i = 0;
    AjPStr y;

    list = ajListstrNew();

    if(!x)
	return list;

    va_start(ap, x);
    y = x;

    for( i=0; y; y = va_arg(ap, AjPStr),i++)
	ajListstrPushAppend(list, y);

    va_end(ap);

    return list;
}




/* @section Destructors *******************************************************
**
** @fdata [AjPList]
**
** Destructors can only deal with the list and cannot free complex data
**
** @nam3rule Del Delete list
** @nam3rule Free Delete list
** @nam4rule FreeData Delete list and free values
**
** @argrule * Plist [AjPList*] List
**
** @valrule * [void]
**
** @fcategory delete
**
******************************************************************************/




/* @func ajListstrFree *********************************************************
**
** Free the list. Do not attempt to free the nodes.
** For use where the node data has been saved elsewhere, for example
** by ajListToarray or where the list is a temporary structure
** referring to permanent data.
**
** @param [d] Plist [AjPList*] List
** @return [void]
** @@
******************************************************************************/

void ajListstrFree(AjPList* Plist)
{
    ajListFree(Plist);

    return;
}




/* @obsolete ajListstrDel
** @rename ajListstrFree
*/

__deprecated void ajListstrDel(AjPList* pthis)
{
    ajListstrFree(pthis);

    return;
}




/* @func ajListstrFreeData *****************************************************
**
** Free all nodes in a string list.
** Also deletes all the strings. If these are to be preserved,
** use ajListstrDel instead.
**
** @param [d] Plist [AjPList*] List
** @return [void]
** @@
******************************************************************************/

void ajListstrFreeData(AjPList* Plist)
{
    AjPListNode next;
    AjPListNode *rest;
    AjPList list;

    if(!Plist)
	return;

    if(!*Plist)
	return;

#ifdef AJ_SAVESTATS
    listDelCnt++;
#endif

    list = *Plist;
    rest = &list->First;

    if(list->Count)
    {
	/* free the data in the list (if we know how) */
	for( ; (*rest)->Next; *rest = next)
	{
	    next = (*rest)->Next;
	    ajStrDel((AjPStr*) &(*rest)->Item);

            if(listFreeNext >= listFreeMax)
                listFreeSetExpand();

            if(listFreeNext >= listFreeMax)
                AJFREE(*rest);
            else if(*rest)
            {
                listFreeSet[listFreeNext++] = *rest;
                *rest=NULL;
            }
	}

	ajStrDel((AjPStr*) &(*rest)->Item);
    }


    if(listFreeNext >= listFreeMax)
        listFreeSetExpand();

    if(listFreeNext >= listFreeMax)
        AJFREE(*rest);
    else if(*rest)
    {
        listFreeSet[listFreeNext++] = *rest;
        *rest = NULL;
    }
    
    AJFREE(*Plist);

    return;
}




/* @datasection [AjIList] String list iterators ********************************
**
** Function is for manipulating lists with string values.
**
** Some functions are specially designed to understand string (AjPStr) values.
**
** @nam2rule Liststr String list data
** @nam3rule Iter Iterator
**
*******************************************************************************/




/* @section stepping **********************************************************
**
** @fdata [AjIList]
**
** @nam4rule   Get      Next value.
** @nam5rule   GetBack  Next value in reverse direction
**
** @argrule * iter [AjIList] String iterator
**
** @valrule * [AjPStr] Value
**
** @fcategory modify
*/




/* @func ajListstrIterGet *****************************************************
**
** Returns next item using iterator, or steps off the end.
**
** @param [u] iter [AjIList] List iterator.
** @return [AjPStr] Data item returned.
** @@
******************************************************************************/

AjPStr ajListstrIterGet(AjIList iter)
{
    AjPListNode p;
    void *ret;

    if(!iter)
	return NULL;

    p = iter->Here;

    if(!iter->Back)
    {
	if(!p->Next)
	    return NULL;

	ret = p->Item;
	iter->Here = p->Next;
    }
    else
    {
	if(!p->Next->Next || !p->Next->Next->Next)
	  return NULL;

	iter->Back = ajFalse;
	ret = p->Next->Item;
	iter->Here = p->Next->Next;
    }

    return (AjPStr) ret;
}




/* @func ajListstrIterGetBack *************************************************
**
** Returns next item using back iterator.
**
** @param [u] iter [AjIList] List iterator.
** @return [AjPStr] Data item returned.
** @@
******************************************************************************/

AjPStr ajListstrIterGetBack(AjIList iter)
{
    AjPListNode p;
    void* ret;

    if(!iter)
	return NULL;

    p = iter->Here;

    if(!p->Prev)
      return NULL;

    if(!iter->Back)
    {
	ret = p->Prev->Prev->Item;
	iter->Here = p->Prev->Prev;
	iter->Back = ajTrue;
    }
    else
    {
	ret = p->Prev->Item;
	iter->Here = p->Prev;
    }

    return (AjPStr) ret;
}




/* @section modifiers **********************************************************
**
** @fdata [AjIList]
**
** @nam4rule   Insert  Insert a new value at iterator position
** @nam4rule   Remove  Remove value at iterator position
**
** @argrule * iter [AjIList] String iterator
** @argrule Insert str [AjPStr] Value
**
** @valrule * [void]
**
** @fcategory modify
*/




/* @func ajListstrIterInsert ***************************************************
**
** Insert an item in a list, using an iterator (if not null)
** to show which position to insert. Otherwise, simply push.
**
** @param [u] iter [AjIList] List iterator.
** @param [u] str [AjPStr] String to insert.
** @return [void]
** @@
******************************************************************************/

void ajListstrIterInsert(AjIList iter, AjPStr str)
{
    AjPList list = iter->Head;
    AjPListNode p;

    /*ajDebug("ajListstrInsert\n");*/
    ajListstrTrace(list);
    ajListstrIterTrace(iter);

    if (!iter->Modify)
    {
	ajErr("ajListInsert called on a read-only iterator");
	return;
    }

    p = iter->Here;


    if(!iter->Back)
    {
	if(!p->Prev)
	    listInsertNode(&list->First,str);
	else
	    listInsertNode(&p->Prev->Next,str);
	iter->Here = p->Prev;
    }
    else
    {
	if(!p->Next)
	    ajFatal("Cannot add a new node for unused back iterator\n");

	if(!p->Prev)
	    listInsertNode(&list->First,str);
	else
	    listInsertNode(&p->Prev->Next,str);
    }

    list->Count++;
#ifdef AJ_SAVESTATS
    if(list->Count > listMaxNum)
	listMaxNum = list->Count;
#endif

    ajListstrTrace(list);
    ajListstrIterTrace(iter);

    return;
}




/* @obsolete ajListstrInsert
** @rename ajListstrIterInsert
*/

__deprecated void ajListstrInsert(AjIList iter, AjPStr str)
{
    ajListstrIterInsert(iter, str);

    return;
}




/* @func ajListstrIterRemove **************************************************
**
** Remove an item from a list, using an iterator (if not null)
** to show which item. Otherwise remove the first item.
**
** We want to remove the item just fetched by the iterator.
**
** @param [u] iter [AjIList] List iterator.
** @return [void]
** @@
******************************************************************************/

void ajListstrIterRemove(AjIList iter)
{
    AjPListNode p;

    /* ajDebug("ajListstrIterRemove\n");*/

    if (!iter->Modify)
    {
	ajErr("ajListstrIterRemove called on a read-only iterator");
	return;
    }

    p = iter->Here;

    if(!iter->Back)
    {
	if(!p->Prev)
	    ajFatal("Attempt to delete from unused iterator\n");

	if(!p->Prev->Prev)
	{
	    ajStrDel((AjPStr *)&(iter->Head->First->Item));
	    listNodeDel(&(iter->Head->First));
	}
	else
	{
	    ajStrDel((AjPStr *)&p->Prev->Prev->Next->Item);
	    listNodeDel(&p->Prev->Prev->Next);
	}
    }
    else
    {
	ajStrDel((AjPStr *)&p->Prev->Prev->Next->Item);
	listNodeDel(&p->Prev->Prev->Next);
    }

    iter->Head->Count--;

    return;
}




/* @obsolete ajListstrRemove
** @rename ajListstrIterRemove
*/

__deprecated void ajListstrRemove(AjIList iter)
{
    ajListstrIterRemove(iter);

    return;
}




/* @section Trace functions ***************************************************
**
** @fdata [AjIList]
**
** @nam4rule Trace Trace contents to debug file
**
** @argrule * iter [const AjIList] List
**
** @valrule * [void]
**
** @fcategory misc
**
******************************************************************************/




/* @func ajListstrIterTrace ***************************************************
**
** Traces a list iterator and validates it
**
** @param [r] iter [const AjIList] List iterator to be traced.
** @return [void]
** @@
******************************************************************************/

void ajListstrIterTrace(const AjIList iter)
{
    if(!iter)
	return;

    ajDebug("\nIterator Head %x ReadHead %x Here %x Back %B Item %S\n",
	    iter->Head, iter->ReadHead, iter->Here,iter->Back,
	    (AjPStr)iter->Here->Item);

    return;
}
