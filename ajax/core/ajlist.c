/* @source ajlist *************************************************************
**
** AJAX List functions
** These functions create and control linked lists.
**
** @author Copyright (C) 1998 Ian Longden
** @version $Revision: 1.92 $
** @modified 2001 Alan Bleasby
**              Changed lists to be double-linked, completely rewrote
**              iterator handling and added back-iteration functions.
**              Operation of ajListInsert made more intuitive.
** @modified $Date: 2012/07/03 16:21:04 $ by $Author: rice $
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




/*Library* List Library *******************************************************
**
** All lists consist of an initial header followed by the body
** of the list. The Header has three variables:-
** 1) First - a pointer to the linked list (see body)
** 2) Last -  a pointer to the a dummy last node object with next = self
** 3) Count - which holds the number of objects in the linked list
**            (NOT including the header)
** 4) Type - the list type
**
** The body of the linked list contains three variables:-
** 1) Next - a pointer to the next linked list object or NULL
** 2) Prev - a pointer to the previous linked list object or NULL
** 3) item - a void pointer to the data.
******************************************************************************/

/* ========================================================================= */
/* ============================= include files ============================= */
/* ========================================================================= */


#include "ajlib.h"

#include "ajlist.h"
#include "ajassert.h"

#include <stddef.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>




/* ========================================================================= */
/* =============================== constants =============================== */
/* ========================================================================= */




/* ========================================================================= */
/* =========================== global variables ============================ */
/* ========================================================================= */




/* ========================================================================= */
/* ============================= private data ============================== */
/* ========================================================================= */




/* ========================================================================= */
/* =========================== private constants =========================== */
/* ========================================================================= */




/* ========================================================================= */
/* =========================== private variables =========================== */
/* ========================================================================= */

#ifdef AJ_SAVESTATS
static ajint listNewCnt     = 0;
static ajint listDelCnt     = 0;
static ajulong listMaxNum    = 0UL;
static ajulong listNodeCnt   = 0UL;
static ajint listIterNewCnt = 0;
static ajint listIterDelCnt = 0;
#endif

static ajulong listFreeNext = 0UL;
static ajulong listFreeMax  = 0UL;
static AjPListNode* listFreeSet = NULL;




/* ========================================================================= */
/* =========================== private functions =========================== */
/* ========================================================================= */

static AjPList listNew(void (*itemdel)(void** Pitem));
static void listInsertNode(AjPListNode* Pnode, void* item);
static AjPListNode listDummyNode(AjPListNode* Pnode);
static void listNodesTrace(const AjPListNode node);
static AjBool listNodeDel(AjPListNode* Pnode);
static void* listNodeItem(const AjPListNode node);
static void listArrayTrace(void** array);
static void listFreeSetExpand(void);
static void listDelStr(void** Pstr);




/* ========================================================================= */
/* ======================= All functions by section ======================== */
/* ========================================================================= */




/* @filesection ajlist ********************************************************
**
** @nam1rule aj Function belongs to the AJAX library.
**
******************************************************************************/




/* @datasection [AjPList] Lists ***********************************************
**
** @nam2rule List Function is for manipulating lists with any value type.
**
** Some functions are specially designed to understand string (AjPStr) values.
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
** @nam4rule NewListref Copy constructor with pointers to source data
** @nam4rule NewRef New reference to an existing list
**
** @argrule NewListref list [const AjPList] Source list
** @argrule NewRef list [AjPList] Source list
**
** @valrule * [AjPList] New list
**
******************************************************************************/





/* @func ajListNew ************************************************************
**
** Creates a new general list.
**
** @return [AjPList] new list;
**
** @release 1.0.0
** @@
******************************************************************************/

AjPList ajListNew(void)
{
    return listNew(NULL);
}




/* @func ajListNewListref *****************************************************
**
** Copy a list.
**
** WARNING: pointers to the data are copied, NOT the data
**          so be careful when cleaning up after copy.
**
** @param [r] list [const AjPList] list to be copied
** @return [AjPList] new copied list.
**
** @release 5.0.0
** @@
******************************************************************************/

AjPList ajListNewListref(const AjPList list)
{
    AjPList newlist = NULL;

    AjPListNode node = NULL;

    if(!list)
        return NULL;

    newlist = ajListNew();

    for(node = list->First; node->Next; node = node->Next)
        ajListPushAppend(newlist, node->Item);


    return newlist;
}




/* @func ajListNewRef *********************************************************
**
** Makes a reference-counted copy of any list
**
** @param [u] list [AjPList] list to be given a new reference
** @return [AjPList] new list;
**
** @release 6.4.0
** @@
******************************************************************************/

AjPList ajListNewRef(AjPList list)
{
    list->Use++;

    return list;
}




/* @funcstatic listNew ********************************************************
**
** Creates a new list.
**
** @param [f] itemdel [void function] Data destructor
** @return [AjPList] new list;
**
** @release 1.0.0
** @@
******************************************************************************/

static AjPList listNew (void (*itemdel)(void** Pitem))
{
    AjPList list = NULL;

    AJNEW0(list);
    list->Fitemdel = itemdel;
    list->Last     = listDummyNode(&list->First);

#ifdef AJ_SAVESTATS
    listNodeCnt--;                      /* dummy */
    listNewCnt++;
#endif

    list->Use = 1;

    return list;
}




/* @funcstatic listInsertNode *************************************************
**
** Inserts a new node in a list at the current node position.
**
** @param [u] Pnode [AjPListNode*] Current AJAX List Node address
** @param [u] item [void*] Data item to insert.
** @return [void]
**
** @release 1.0.0
** @@
******************************************************************************/

static void listInsertNode(AjPListNode* Pnode, void* item)
{
    AjPListNode node = NULL;

    if(listFreeNext)
        node = listFreeSet[--listFreeNext];
    else
        AJNEW0(node);

    node->Item = item;
    node->Next = (*Pnode);
    node->Prev = (*Pnode)->Prev;

    node->Next->Prev = node;

    *Pnode = node;

#ifdef AJ_SAVESTATS
    listNodeCnt++;
#endif

    return;
}




/* @funcstatic listDummyNode **************************************************
**
** Creates a new empty node.
**
** @param [u] Pnode [AjPListNode*] New AJAX List Node address
** @return [AjPListNode] Copy of current node
**
** @release 1.0.0
** @@
******************************************************************************/

static AjPListNode listDummyNode(AjPListNode* Pnode)
{
    if(listFreeNext)
    {
        *Pnode = listFreeSet[--listFreeNext];
        (*Pnode)->Item = NULL;
        (*Pnode)->Prev = NULL;
        (*Pnode)->Next = NULL;
    }
    else
        AJNEW0(*Pnode);

#ifdef AJ_SAVESTATS
    listNodeCnt++;
#endif

    return *Pnode;
}




/* @section Adding values *****************************************************
**
** @fdata [AjPList]
**
** @nam3rule Push Add new value
** @nam4rule PushAppend Add new value to end of list
** @nam3rule Pushlist Add new list of values
**
** @argrule * list [AjPList] List
** @argrule Push item [void*] Pointer to data.
** @argrule Pushlist Plist [AjPList*] Source AJAX List address to be deleted
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
** @param [u] item [void*] Pointer to data.
** @return [void]
**
** @release 1.0.0
** @@
******************************************************************************/

void ajListPush(AjPList list, void* item)
{
    assert(list);

    listInsertNode(&list->First, item);

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
** @param [u] item [void*] Pointer to data to append.
** @return [void]
**
** @release 5.0.0
** @@
******************************************************************************/

void ajListPushAppend(AjPList list, void* item)
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
        ajListPush(list, item);
        return;
    }

    list->Last->Item = item;

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




/* @func ajListPushlist *******************************************************
**
** Adds a list to the start of the current list, then deletes the second list.
**
** @param [u] list [AjPList] List.
** @param [d] Plist [AjPList*] AJAX List address to be merged.
** @return [void]
**
** @release 5.0.0
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

    ajListFree(Plist);  /* free the list but not the nodes */

    return;
}




/* @section Modifiers *********************************************************
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
** @argrule Purge test [AjBool function] Function to find unwanted values
** @argrule Purge itemdel [void function] Function to remove a value
** @argrule Sort compar1 [int function]
** First function to compare values
** @argrule Two compar2 [int function]
** Second function to compare equal values
** @argrule Three compar3 [int function]
** Third function to compare equal values
** @argrule Unique itemdel [void function] Function to remove a value
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
**
** @release 1.0.0
** @@
******************************************************************************/

void ajListMap(AjPList list, void (*apply)(void** Pitem, void* cl), void* cl)
{
    AjPListNode rest;

    assert(apply);

    for(rest = list->First; rest->Next; rest = rest->Next)
        (*apply)(&rest->Item, cl);

    return;
}




/* @func ajListPurge **********************************************************
**
** Garbage collect a list
**
** @param [u] list [AjPList] List.
** @param [f] test [AjBool function] Function to test whether to delete
** @param [f] itemdel [void function] Wrapper function for item destructor
** @return [void]
**
** @release 5.0.0
** @@
******************************************************************************/

void ajListPurge(AjPList list,
                 AjBool (*test)(const void* item),
                 void (*itemdel)(void** Pitem))
{
    void* item = NULL;

    AjIList iter = NULL;


    iter = ajListIterNew(list);

    while((item = ajListIterGet(iter)))
        if((*test)(item))
        {
            (*itemdel)(&item);
            ajListIterRemove(iter);
        }


    ajListIterDel(&iter);

    return;
}




/* @func ajListReverse ********************************************************
**
** Reverse the order of the nodes in an abstract list.
**
** @param [u] list [AjPList] List
** @return [void]
**
** @release 1.0.0
** @@
******************************************************************************/

void ajListReverse(AjPList list)
{
    AjPListNode head     = NULL;
    AjPListNode savenext = NULL;
    AjPListNode node     = NULL;

    if(!list)
        return;

    if(list->Count <= 1)
        return;

    head = list->Last;

    list->Last->Prev = list->First;

    for(node = list->First; node->Next; node = savenext)
    {
        savenext   = node->Next;
        node->Prev = node->Next;
        node->Next = head;
        head       = node;
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
** @param [f] compar1 [int function] Function to compare two list items.
** @return [void]
**
** @release 1.0.0
** @@
******************************************************************************/

void ajListSort(AjPList list,
                int (*compar1)(const void* item1,
                               const void* item2))
{
    void** array = NULL;

    ajulong i = 0UL;

    AjPListNode node = NULL;

    /*ajDebug("ajListSort %Lu items\n", list->Count);*/
    /*ajListTrace(list);*/

    node = list->First;

    if(list->Count <= 1)
        return;

    ajListToarray(list, &array);
    /* listArrayTrace(array);*/

    qsort(array, (size_t) list->Count, sizeof(void*), compar1);

    while(node->Next)
    {
        node->Item = array[i++];
        node = node->Next;
    }

    AJFREE(array);

    return;
}




/* @func ajListSortTwo ********************************************************
**
** Sort the items in a list using 2 fields in the same object hierarchy.
**
** @param [u] list [AjPList] List.
** @param [f] compar1 [int function] 1st function to compare two list items.
** @param [f] compar2 [int function] 2nd function to compare two list items.
** @return [void]
**
** @release 5.0.0
** @@
******************************************************************************/

void ajListSortTwo(AjPList list,
                   int (*compar1)(const void* item1,
                                  const void* item2),
                   int (*compar2)(const void* item1,
                                  const void* item2))
{
    AjPListNode node = NULL;

    void** items = NULL;

    ajulong len   = 0UL;
    ajulong limit = 0UL;
    ajulong pos   = 0UL;
    ajulong base  = 0UL;
    ajulong n     = 0UL;

    node = list->First;

    ajListSort(list, compar1);

    len = ajListToarray(list, &items);

    if(len < 2)
        return;

    pos = base = 0UL;
    limit = len - 2;

    while(pos < limit)
    {
        while((*compar1)(&items[pos], &items[pos + 1]) == 0)
        {
            ++pos;

            if(pos > limit)
                break;
        }
        ++pos;

        n = pos - base;

        if(n > 1)
	  qsort((void*) &items[base], (size_t) n, sizeof(void*), compar2);

        base = pos;
    }

    pos = 0UL;

    while(node->Next)
    {
        node->Item = items[pos++];
        node = node->Next;
    }

    AJFREE(items);

    return;
}




/* @func ajListSortTwoThree ***************************************************
**
** Sort the items in a list using 3 fields in the same object hierarchy.
**
** @param [u] list [AjPList] List.
** @param [f] compar1 [int function] 1st function to compare two list items.
** @param [f] compar2 [int function] 2nd function to compare two list items.
** @param [f] compar3 [int function] 3rd function to compare two list items.
** @return [void]
**
** @release 5.0.0
** @@
******************************************************************************/

void ajListSortTwoThree(AjPList list,
                        int (*compar1)(const void* item1,
                                       const void* item2),
                        int (*compar2)(const void* item1,
                                       const void* item2),
                        int (*compar3)(const void* item1,
                                       const void* item2))
{
    AjPListNode node = NULL;

    void** items = NULL;

    ajulong len   = 0UL;
    ajulong limit = 0UL;
    ajulong pos   = 0UL;
    ajulong base  = 0UL;
    ajulong n     = 0UL;

    node = list->First;
    len = ajListGetLength(list);

    if(len < 2)
        return;

    ajListSortTwo(list, compar1, compar2);

    len = ajListToarray(list, &items);

    pos = base = 0UL;
    limit = len - 2;

    while(pos < limit)
    {
        while(((*compar1)(&items[pos],&items[pos+1]) == 0) &&
              ((*compar2)(&items[pos],&items[pos+1]) == 0))
        {
            ++pos;

            if(pos > limit)
                break;
        }

        ++pos;
        n = pos - base;

        if(n > 1)
	  qsort((void*) &items[base], (size_t) n, sizeof(void*), compar3);

        base = pos;
    }

    pos = 0UL;

    while(node->Next)
    {
        node->Item = items[pos++];
        node = node->Next;
    }

    AJFREE(items);

    return;
}




/* @func ajListSortTwoUnique **************************************************
**
** Double-sort the items in a list, and remove duplicates
**
** @param [u] list [AjPList] List.
** @param [f] compar1 [int function] Function to compare two list items.
** @param [f] compar2 [int function] Function to compare two list items.
** @param [f] itemdel [void function] Function to delete an item
** @return [void]
**
** @release 5.0.0
** @@
******************************************************************************/

void ajListSortTwoUnique(AjPList list,
                         int (*compar1)(const void* item1,
                                        const void* item2),
                         int (*compar2)(const void* item1,
                                        const void* item2),
                         void (*itemdel)(void** Pitem,
                                         void* cl))
{
    void* item     = NULL;
    void* previtem = NULL;

    AjIList iter = NULL;

    ajDebug("ajListSortTwoUnique %Lu items\n", list->Count);

    if(list->Count <= 1)                /* no duplicates */
        return;

    ajListSortTwo(list, compar1, compar2);
    /*ajListTrace(list);*/

    iter = ajListIterNew(list);

    while(!ajListIterDone(iter))
    {
        item = ajListIterGet(iter);

        if(previtem &&
           ((*compar1)(&item, &previtem) == 0) &&
           ((*compar2)(&item, &previtem) == 0))
        {
            (*itemdel)(&item, NULL);
            ajListIterRemove(iter);
        }
        else
            previtem = item;
    }

    ajListIterDel(&iter);

    ajDebug("ajListUnique result %Lu items\n", list->Count);
    /*ajListTrace(list);*/

    return;
}




/* @func ajListSortUnique *****************************************************
**
** Sort the items in a list, and remove duplicates
**
** @param [u] list [AjPList] List.
** @param [f] compar1 [int function] Function to compare two list items.
** @param [f] itemdel [void function] Function to delete an item
** @return [void]
**
** @release 5.0.0
** @@
******************************************************************************/

void ajListSortUnique(AjPList list,
                      int (*compar1)(const void* item1,
                                     const void* item2),
                      void (*itemdel)(void** Pitem,
                                      void* cl))
{
    void* item     = NULL;
    void* previtem = NULL;

    AjIList iter = NULL;

    ajDebug("ajListSortUnique %Lu items\n", list->Count);

    if(list->Count <= 1)                /* no duplicates */
        return;

    ajListSort(list, compar1);
    /*ajListTrace(list);*/

    iter = ajListIterNew(list);

    while(!ajListIterDone(iter))
    {
        item = ajListIterGet(iter);

        if(previtem && ((*compar1)(&item, &previtem) == 0))
        {
            (*itemdel)(&item, NULL);
            ajListIterRemove(iter);
        }
        else
            previtem = item;
    }

    ajListIterDel(&iter);

    ajDebug("ajListSortUnique result %d items\n", list->Count);
    /*ajListTrace(list);*/

    return;
}




/* @section Removing data *****************************************************
**
** @fdata [AjPList]
**
** Retrieves values from a list
**
** @nam3rule Pop Retrieve next value
** @nam4rule PopLast Retrieve last value
** @nam3rule Drop Remove value by reference
**
** @argrule * list [AjPList] List
** @argrule Drop item [void*] Value
** @argrule Pop Pitem [void**] Value
**
** @valrule *Drop [void*] Removed value or NULL if not found
** @valrule *Pop [AjBool] True on success
** @fcategory cast
**
******************************************************************************/




/* @funcstatic listFreeSetExpand **********************************************
**
** Expand the list of free nodes
**
** @return [void]
**
** @release 6.0.0
******************************************************************************/

static void listFreeSetExpand(void)
{
    ajulong newsize;

    if(!listFreeSet)
    {
        listFreeMax = 1024;
        AJCNEW0(listFreeSet,listFreeMax);

        return;
    }

    if(listFreeMax >= 65536)
        return;

    newsize = listFreeMax + listFreeMax;
    AJCRESIZE0(listFreeSet, (size_t) listFreeMax, (size_t) newsize);
    listFreeMax = newsize;

    return;
}




/* @func ajListDrop ***********************************************************
**
** Removes a data value from a list if the address matches data for a node
**
** @param [u] list [AjPList] List
** @param [r] item [void*] Data item
** @return [void*] Dropped data item.
**
** @release 6.5.0
** @@
******************************************************************************/

void* ajListDrop(AjPList list, void* item)
{
    void* ret = NULL;

    AjIList iter = NULL;

    ajDebug("ajListDrop %p\n", item);

    iter = ajListIterNewBack(list);

    while(!ajListIterDoneBack(iter))
    {
        ret = ajListIterGetBack(iter);

        ajDebug("test %p\n", ret);

        if(ret == item)
        {
            ajListIterRemove(iter);
            ajListIterDel(&iter);
            return ret;
        }
    }
    ajListIterDel(&iter);

    return NULL;
}




/* @func ajListPop ************************************************************
**
** remove the first node but set pointer to data first.
**
** @param [u] list [AjPList] List
** @param [w] Pitem [void**] pointer to pointer to data
** @return [AjBool] ajTrue on success.
**
** @release 1.0.0
** @@
******************************************************************************/

AjBool ajListPop(AjPList list, void** Pitem)
{
    if(!list)
    {
        if(Pitem)
            *Pitem = NULL;

        return ajFalse;
    }

    if(Pitem)
        *Pitem = listNodeItem(list->First);

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
** @param [w] Pitem [void**] pointer to pointer to data
** @return [AjBool] ajTrue on success.
**
** @release 5.0.0
** @@
******************************************************************************/

AjBool ajListPopLast(AjPList list, void** Pitem)
{
    AjPListNode node = NULL;

    if(!list)
    {
        if(Pitem)
            *Pitem = NULL;

        return ajFalse;
    }


    if(!list->Count)
        return ajFalse;

    node = list->Last->Prev;

    if(Pitem)
        *Pitem = listNodeItem(node);


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
        node->Prev->Next = list->Last;
        list->Last->Prev = node->Prev;

        if(listFreeNext >= listFreeMax)
            listFreeSetExpand();

        if(listFreeNext >= listFreeMax)
            AJFREE(node);
        else if(node)
        {
            listFreeSet[listFreeNext++] = node;
            node = NULL;
        }
    }

    --list->Count;

    return ajTrue;
}




/* @section Element retrieval *************************************************
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
** @argrule PeekNumber ipos [ajulong] Position in list
** @argrule Peek Pitem [void**] Value
** @argrule Toarray array [void***] Array of values, ending with NULL
** @argrule Toindex lind [ajuint*] Populated index array to be sorted
** @argrule Toindex compar1 [int function] Function to compare two list items.
**
** @valrule * [AjBool] True on success
** @valrule *Length [ajulong] List length
** @valrule *Mapfind [AjBool] True if function returns true
** @valrule *Mapread [void]
** @valrule *Toarray [ajulong] Array size, excluding final NULL
** @valrule *Toindex [ajulong] Array size, excluding final NULL
**
** @fcategory cast
**
******************************************************************************/




/* @func ajListGetLength ******************************************************
**
** get the number of nodes in the linked list.
**
** @param [r] list [const AjPList] List
** @return [ajulong] Number of nodes in list.
**
** @release 5.0.0
** @@
******************************************************************************/

ajulong ajListGetLength(const AjPList list)
{
    if(!list)
        return 0UL;

    return list->Count;
}




/* @func ajListMapfind ********************************************************
**
** For each node in the list call function 'apply' and return
** ajTrue when any node is matched by the function.
**
** @param [r] list [const AjPList] List
** @param [f] apply [AjBool function] Function to call to test each list item.
** @param [u] cl [void*] Standard, usually NULL.
** @return [AjBool] ajTrue on success.
**
** @release 5.0.0
** @@
******************************************************************************/

AjBool ajListMapfind(const AjPList list,
                     AjBool (*apply)(void** Pitem, void* cl), void* cl)
{
    AjPListNode listnode = NULL;

    assert(list);

    assert(apply);

    for( listnode = list->First; listnode->Next; listnode = listnode->Next)
        if((*apply)(&listnode->Item, cl))
            return ajTrue;

    return ajFalse;
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
**
** @release 5.0.0
** @@
******************************************************************************/

void ajListMapread(const AjPList list,
                   void (*apply)(void* item, void* cl), void* cl)
{
    AjPListNode rest = NULL;

    assert(apply);

    for(rest = list->First; rest->Next; rest = rest->Next)
        (*apply)(rest->Item, cl);

    return;
}




/* @func ajListPeek ***********************************************************
**
** Return the first node but keep it on the list
**
** @param [r] list [const AjPList] List
** @param [w] Pitem [void**] pointer to pointer to data
** @return [AjBool] ajTrue on success.
**
** @release 2.7.0
** @@
******************************************************************************/

AjBool ajListPeek(const AjPList list, void** Pitem)
{
    if(!list)
        return ajFalse;

    if(!list->Count)
        return ajFalse;

    if(Pitem)
        *Pitem = listNodeItem(list->First);

    return ajTrue;
}




/* @func ajListPeekFirst ******************************************************
**
** Set pointer to first node's data. Does NOT remove the first node.
**
** @param [r] list [const AjPList] List
** @param [w] Pitem [void**] pointer to pointer to data
** @return [AjBool] ajTrue on success.
**
** @release 5.0.0
** @@
******************************************************************************/

AjBool ajListPeekFirst(const AjPList list, void** Pitem)
{
    if(!list)
        return ajFalse;

    if(!list->Count)
        return ajFalse;

    if(Pitem)
        *Pitem = listNodeItem(list->First);

    return ajTrue;
}




/* @func ajListPeekLast *******************************************************
**
** Set pointer to last node's data. Does NOT remove the last node.
**
** @param [r] list [const AjPList] List
** @param [w] Pitem [void**] pointer to pointer to data
** @return [AjBool] ajTrue on success.
**
** @release 5.0.0
** @@
******************************************************************************/

AjBool ajListPeekLast(const AjPList list, void** Pitem)
{
    AjPListNode rest = NULL;

    if(!list)
        return ajFalse;

    if(!list->Count)
        return ajFalse;

    if(list->Last)
        rest = list->Last->Prev;
    else
        rest = list->First;

    if(Pitem)
        *Pitem = listNodeItem(rest);

    return ajTrue;
}




/* @func ajListPeekNumber *****************************************************
**
** Set pointer to last node's nth data item. 0 <= n < number of elements.
**
** @param [r] list [const AjPList] List
** @param [r] ipos [ajulong] element of the list
** @param [w] Pitem [void**] pointer to pointer to data
** @return [AjBool] ajTrue on success.
**
** @release 5.0.0
** @@
******************************************************************************/

AjBool ajListPeekNumber(const AjPList list, ajulong ipos, void** Pitem)
{
    ajulong len = 0UL;
    ajulong i   = 0UL;

    AjPListNode rest = NULL;

    if(!list)
        return ajFalse;

    len = ajListGetLength(list);

    if(ipos>=len)
        return ajFalse;

    for(i = 0Ul, rest = list->First; i < ipos ; rest = rest->Next)
        i++;

    if(Pitem)
        *Pitem = listNodeItem(rest);

    return ajTrue;
}




/* @func ajListToarray ********************************************************
**
** Create an array of the pointers to the data.
**
** @param [r] list [const AjPList] List
** @param [w] array [void***] Array of pointers to list items.
** @return [ajulong] Size of array of pointers.
**
** @release 5.0.0
** @@
******************************************************************************/

ajulong ajListToarray(const AjPList list, void*** array)
{
    ajulong i = 0UL;
    ajulong n = 0UL;

    AjPListNode rest = NULL;

    n    = list->Count;
    rest = list->First;

    if(!n)
    {
        *array = NULL;

        return 0UL;
    }

    if(*array)
        AJFREE(*array);

    *array = AJALLOC((size_t)(n + 1) * sizeof(array));

    for(i = 0UL; i < n; i++)
    {
        (*array)[i] = rest->Item;
        rest = rest->Next;
    }

    (*array)[n] = NULL;

    return n;
}




/* @funcstatic listArrayTrace *************************************************
**
** Writes debug messages to trace an array generated from a list.
**
** @param [r] array [void**] Array to trace
** @return [void]
**
** @release 1.0.0
** @@
******************************************************************************/

static void listArrayTrace(void** array)
{
    void** v = array;

    ajuint i = 0UL;

    while(*v)
        ajDebug("array[%Lu] %p\n", i++, *v++);

    return;
}




/* @func ajListToindex ********************************************************
**
** Create an array of the pointers to the data.
**
** @param [r] list [const AjPList] List
** @param [w] lind [ajuint*] Populated index array to be sorted
** @param [f] compar1 [int function] Function to compare two list items.
** @return [ajulong] Size of index array.
**
** @release 6.3.0
** @@
******************************************************************************/

ajulong ajListToindex(const AjPList list, ajuint* lind,
                     int (*compar1)(const void* item1, const void* item2))
{
    ajulong n = 0UL;
    ajulong s = 0UL;
    ajulong i = 0UL;
    ajlong  j = 0L;
    ajuint  it = 0U;
    ajulong lt = 0UL;

    AjPListNode* nodes = NULL;
    ajulong* idx = NULL;

    n = list->Count;

    if(!n)
        return 0UL;

    ajListToarray(list, (void***) &nodes);
    AJCNEW0(idx, n);

    for(i = 0UL; i < n; i++)
        idx[i] = i;

    for(s = n / 2; s > 0; s /= 2)
        for(i = s; i < n; ++i)
        {
            for(j = i - s;
                j >= 0 && ((*compar1)(&nodes[idx[j]], &nodes[idx[j+s]]) > 0);
                j -= s)
            {
                it = lind[j];
                lind[j] = lind[j + s];
                lind[j + s] = it;
                lt = idx[j];
                idx[j] = idx[j + s];
                idx[j + s] = lt;
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




/* @func ajListProbe **********************************************************
**
** Test list for memory allocation conflicts
**
** @param [r] Plist [AjPList const*] List
** @return [void]
**
** @release 6.0.0
** @@
******************************************************************************/

void ajListProbe(AjPList const* Plist)
{
    AjPList list = NULL;

    AjPListNode  next = NULL;
    AjPListNode* rest = NULL;

    if(!Plist)
        return;

    if(!*Plist)
        return;

    list = *Plist;
    rest = &list->First;

    /* don't free the data in the list (we don't know how) */
    /* just free the nodes */
    if(list->Count)
        for(; (*rest)->Next; *rest = next)
        {
            next = (*rest)->Next;
            AJMPROBE(*rest);
        }

    AJMPROBE(*rest);
    AJMPROBE(*Plist);

    return;
}




/* @func ajListProbeData ******************************************************
**
** Test list and data for memory allocation conflicts
**
** @param [r] Plist [AjPList const*] List
** @return [void]
**
** @release 6.0.0
** @@
******************************************************************************/

void ajListProbeData(AjPList const* Plist)
{
    AjPListNode  next = NULL;
    AjPListNode* rest = NULL;

    AjPList list = NULL;

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
        for(; (*rest)->Next; *rest = next)
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
**
** @release 1.0.0
** @@
******************************************************************************/

void ajListTrace(const AjPList list)
{
    ajulong i = 0UL;

    AjPListNode node = NULL;

    if(!list)
        return;

    ajDebug("\nList Trace %p count %d\n",
            list, list->Count);
    ajDebug("first-> %p last-> %p\n", list->First, list->Last);

    for(node = list->First; node->Next; node = node->Next)
    {
        i++;
        ajDebug("Item[%Lu] item %p (data %p) rest -> %p prev -> %p\n",
                i, node, node->Item, node->Next, node->Prev);
    }

    if(i != list->Count)
    {
        ajErr("*** list error expect %Lu items, found %Lu",
              list->Count, i);
    }

    if(list->Last != node)
    {
        ajDebug("*** list error expect end at %p, found at %p\n",
                list->Last, node);
        ajErr("*** list error expect end at %p, found at %p",
              list->Last, node);
    }

    return;
}




/* @funcstatic listNodesTrace *************************************************
**
** Writes debug messages to trace from the current list node.
**
** @param [r] node [const AjPListNode] Current node.
** @return [void]
**
** @release 1.0.0
** @@
******************************************************************************/

static void listNodesTrace(const AjPListNode node)
{
    ajDebug("listNodesTrace %p\n", node);

    while(node->Next)
    {
        ajDebug("node %p item %p -> %p\n", node, node->Item, node->Next);
        node = node->Next;
    }

    return;
}





/* @funcstatic listNodeItem ***************************************************
**
** Return the data item for a list node.
**
** @param [r] node  [const AjPListNode] Current node.
** @return [void*] Data item.
**
** @release 1.0.0
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
** @argrule * Plist [AjPList*] AJAX List address
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
** @param [d] Plist [AjPList*] AJAX List address
** @return [void]
**
** @release 1.0.0
** @@
******************************************************************************/

void ajListFree(AjPList* Plist)
{
    AjPList list = NULL;

    AjPListNode  next = NULL;
    AjPListNode* rest = NULL;

    if(!Plist)
        return;

    if(!*Plist)
        return;

#ifdef AJ_SAVESTATS
    listDelCnt++;
#endif

    list = *Plist;

    if(!list->Use)
        ajErr("trying to delete unused list");

    --list->Use;

    if(!list->Use)
    {                                   /* any other references? */
        rest = &list->First;

        /* don't free the data in the list (we don't know how) */
        /* just free the nodes */
        if(list->Count)
            for(; (*rest)->Next; *rest = next)
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
    }

    *Plist = NULL;

    return;
}




/* @func ajListFreeData *******************************************************
**
** Free all nodes in the list. Free all the data values.
** For more complex data objects use ajListMap with a routine to
** free the object memory.
**
** @param [d] Plist [AjPList*] AJAX List address
** @return [void]
**
** @release 4.1.0
** @@
******************************************************************************/

void ajListFreeData(AjPList* Plist)
{
    AjPList list = NULL;

    AjPListNode  next = NULL;
    AjPListNode* rest = NULL;

    if(!Plist)
        return;

    if(!*Plist)
        return;

#ifdef AJ_SAVESTATS
    listDelCnt++;
#endif

    list = *Plist;

    if(!list->Use)
        ajErr("trying to delete unused list");

    --list->Use;

    if(!list->Use)
    {                                   /* any other references? */
        rest = &list->First;

        /* free the data for each node (just a simple free) */
        /* as we free the nodes */

        if(list->Count)
        {
            for(; (*rest)->Next; *rest = next)
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
    }

    *Plist = NULL;

    return;
}




/* @funcstatic listDelStr *****************************************************
**
** Delete a string object value
**
** @param [d] Pstr [void**] AJAX String object to be deleted
** @return [void]
**
** @release 6.4.0
******************************************************************************/

static void listDelStr(void** Pstr)
{
    if(!Pstr)
        return;

    if(!*Pstr)
        return;

    ajStrDel((AjPStr*) Pstr);

    return;
}




/* @funcstatic listNodeDel ****************************************************
**
** Remove a node from the list.
**
** @param [d] Pnode [AjPListNode*] Current AJAX List Node address
** @return [AjBool] ajTrue on success
**
** @release 1.0.0
** @@
******************************************************************************/

static AjBool listNodeDel(AjPListNode* Pnode)
{
    AjPListNode node = NULL;
    AjPListNode tmp  = NULL;

    node = *Pnode;

    if(!node || !node->Next)
        return ajFalse;

    tmp = node->Prev;
    node = node->Next;
    node->Prev = tmp;

    if(listFreeNext >= listFreeMax)
        listFreeSetExpand();

    if(listFreeNext >= listFreeMax)
        AJFREE(*Pnode);
    else if(*Pnode)
        listFreeSet[listFreeNext++] = *Pnode;

    *Pnode = node;

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
**
** @release 5.0.0
******************************************************************************/

void ajListUnused(void** array)
{
    const AjPListNode node = NULL;
    listNodesTrace(node);
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
******************************************************************************/




/* @func ajListExit ***********************************************************
**
** Prints a summary of list usage with debug calls
**
** @return [void]
**
** @release 2.8.0
** @@
******************************************************************************/

void ajListExit(void)
{
    ajulong i = 0UL;

#ifdef AJ_SAVESTATS
    ajDebug("List usage : %d opened, %d closed, %Ld maxsize %Ld nodes\n",
            listNewCnt, listDelCnt, listMaxNum, listNodeCnt);
    ajDebug("List iterator usage : %d opened, %d closed\n",
            listIterNewCnt, listIterDelCnt);
#endif
    if(listFreeNext)
        for(i = 0UL; i < listFreeNext; i++)
            AJFREE(listFreeSet[i]);


    if(listFreeSet)
        AJFREE(listFreeSet);

    listFreeNext = 0UL;
    listFreeMax  = 0UL;

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
******************************************************************************/




/* @section constructors ******************************************************
**
** @fdata [AjIList]
**
** @nam4rule New      List iterator constructor.
** @nam5rule NewBack  List iterator reverse direction constructor.
** @nam4rule Newread  List iterator read-only constructor.
** @nam5rule NewreadBack List iterator read-only reverse direction constructor.
**
** @argrule New list [AjPList] Original list
** @argrule Newread list [const AjPList] Original list
**
** @valrule * [AjIList] List iterator
**
** @fcategory new
******************************************************************************/




/* @func ajListIterNew ********************************************************
**
** Creates an iterator to operate from start to end of list.
**
** @param [u] list [AjPList] List
**                 Not const in practice - the iterator can insert
**                 and delete entries
** @return [AjIList] New list iterator
**
** @release 5.0.0
** @@
******************************************************************************/

AjIList ajListIterNew(AjPList list)
{
    AjIList iter = NULL;

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




/* @func ajListIterNewBack ****************************************************
**
** Creates an iterator to operate from end to start of the list.
**
** @param [u] list [AjPList] List
**                 Not const - the iterator can insert and delete entries
** @return [AjIList] New list iterator
**
** @release 5.0.0
** @@
******************************************************************************/

AjIList ajListIterNewBack(AjPList list)
{
    AjIList iter = NULL;

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




/* @func ajListIterNewread ****************************************************
**
** Creates an iterator to operate from start to end of read-only list.
**
** @param [r] list [const AjPList] List
**                 Not const in practice - the iterator can insert
**                 and delete entries
** @return [AjIList] New list iterator
**
** @release 5.0.0
** @@
******************************************************************************/

AjIList ajListIterNewread(const AjPList list)
{
    AjIList iter = NULL;

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




/* @func ajListIterNewreadBack ************************************************
**
** Creates an iterator to operate from end to start of the list.
**
** @param [r] list [const AjPList] List
** @return [AjIList] New list iterator
**
** @release 5.0.0
** @@
******************************************************************************/

AjIList ajListIterNewreadBack(const AjPList list)
{
    AjIList iter = NULL;

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
******************************************************************************/




/* @func ajListIterDone *******************************************************
**
** Tests whether an iterator has completed yet.
**
** @param [r] iter [const AjIList] List iterator.
** @return [AjBool] ajTrue if the iterator is exhausted.
**
** @release 1.0.0
** @@
******************************************************************************/

AjBool ajListIterDone(const AjIList iter)
{
    AjPListNode node = NULL;

    if(!iter)
        return ajTrue;

    node = iter->Here;

    if(!iter->Back)
    {
        if(!node->Next)
            return ajTrue;
    }
    else
        if(!node->Next->Next || !node->Next->Next->Next)
            return ajTrue;

    return ajFalse;
}




/* @func ajListIterDoneBack ***************************************************
**
** Tests whether a backwards iterator has completed yet.
**
** @param [r] iter [const AjIList] List iterator.
** @return [AjBool] ajTrue if the iterator is exhausted.
**
** @release 5.0.0
** @@
******************************************************************************/

AjBool ajListIterDoneBack(const AjIList iter)
{
    AjPListNode node = NULL;

    if(!iter)
        return ajTrue;

    node = iter->Here;

    if(!node->Prev)
        return ajTrue;

    return ajFalse;
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
******************************************************************************/




/* @func ajListIterDel ********************************************************
**
** Destructor for a list iterator.
**
** @param [d] iter [AjIList*] List iterator.
** @return [void]
**
** @release 5.0.0
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
******************************************************************************/




/* @func ajListIterGet ********************************************************
**
** Returns next item using iterator, or steps off the end.
**
** @param [u] iter [AjIList] List iterator.
** @return [void*] Data item returned.
**
** @release 5.0.0
** @@
******************************************************************************/

void* ajListIterGet(AjIList iter)
{
    void* item = NULL;

    AjPListNode node = NULL;

    if(!iter)
        return NULL;

    node = iter->Here;

    if(!iter->Back)
    {
        if(!node->Next)
            return NULL;

        item = node->Item;
        iter->Here = node->Next;
    }
    else
    {
        if(!node->Next->Next || !node->Next->Next->Next)
            return NULL;

        iter->Back = ajFalse;
        item = node->Next->Item;
        iter->Here = node->Next->Next;
    }

    return item;
}




/* @func ajListIterGetBack ****************************************************
**
** Returns next item using back iterator.
**
** @param [u] iter [AjIList] List iterator.
** @return [void*] Data item returned.
**
** @release 5.0.0
** @@
******************************************************************************/

void* ajListIterGetBack(AjIList iter)
{
    void* item = NULL;

    AjPListNode node = NULL;

    if(!iter)
        return NULL;

    node = iter->Here;

    if(!node->Prev)
        return NULL;

    if(!iter->Back && node->Prev->Prev)
    {
        item = node->Prev->Prev->Item;
        iter->Here = node->Prev->Prev;
        iter->Back = ajTrue;
    }
    else
    {
        item = node->Prev->Item;
        iter->Here = node->Prev;
    }

    return item;
}




/* @section modifiers *********************************************************
**
** @fdata [AjIList]
**
** @nam4rule   Insert  Insert a new value at iterator position
** @nam4rule   Remove  Remove value at iterator position
** @nam4rule   Rewind  Rewind list to start
**
** @argrule * iter [AjIList] String iterator
** @argrule Insert item [void*] Value
**
** @valrule * [void]
**
** @fcategory modify
******************************************************************************/




/* @func ajListIterInsert *****************************************************
**
** Insert an item in a list, using an iterator (if not null)
** to show which position to insert. Otherwise, simply push.
**
** @param [u] iter [AjIList] List iterator.
** @param [u] item [void*] Data item to insert.
** @return [void]
**
** @release 5.0.0
** @@
******************************************************************************/

void ajListIterInsert(AjIList iter, void* item)
{
    AjPList list = iter->Head;

    AjPListNode node = NULL;

    /* ajDebug("ajListInsert\n");*/

    if(!iter->Modify)
    {
        ajErr("ajListIterInsert called on a read-only iterator");
        return;
    }

    node = iter->Here;

    if(!iter->Back)
    {
        if(!node->Prev)
            listInsertNode(&list->First, item);
        else
            listInsertNode(&node->Prev->Next, item);

        iter->Here = node->Prev;
    }
    else
    {
        if(!node->Next)
            ajFatal("Cannot add a new node for unused back iterator\n");

        if(!node->Prev)
            listInsertNode(&list->First, item);
        else
            listInsertNode(&node->Prev->Next, item);
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




/* @func ajListIterRemove *****************************************************
**
** Remove an item from a list, using an iterator (if not null)
** to show which item. Otherwise remove the first item.
**
** We want to remove the item just fetched by the iterator.
**
** @param [u] iter [AjIList] List iterator.
** @return [void]
**
** @release 5.0.0
** @@
******************************************************************************/

void ajListIterRemove(AjIList iter)
{
    AjPListNode node = NULL;

    /* ajDebug("ajListIterRemove\n");*/

    if(!iter->Modify)
    {
        ajDie("Attempt to modify read-only iterator with ajListIterRemove\n");
        return;
    }

    node = iter->Here;

    if(!iter->Back)
    {
        if(!node->Prev)
            ajDie("Attempt to delete from unused iterator\n");

        if(!node->Prev->Prev)
            listNodeDel(&(iter->Head->First));
        else
            listNodeDel(&node->Prev->Prev->Next);
    }
    else
        listNodeDel(&node->Prev->Prev->Next);


    iter->Head->Count--;

    return;
}




/* @func ajListIterRewind *****************************************************
**
** Resets iterator to start position
**
** @param [u] iter [AjIList] List iterator.
** @return [void]
**
** @release 6.0.0
** @@
******************************************************************************/

void ajListIterRewind(AjIList iter)
{
    const AjPList list = NULL;

    AjPListNode node = NULL;
    AjPListNode tmp  = NULL;

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
**
** @release 1.0.0
** @@
******************************************************************************/

void ajListIterTrace(const AjIList iter)
{
    ajulong icount = 0;

    if(!iter)
    {
        ajDebug("\nIterator NULL\n");
        return;
    }

    if(iter->Head)
        icount = iter->Head->Count;
    else if(iter->ReadHead)
        icount = iter->ReadHead->Count;

    ajDebug("\nIter Head %p ReadHead %p Here %p Back %B Modify %B Len: %Ld\n",
            iter->Head,iter->ReadHead,iter->Here,iter->Back,
            iter->Modify, icount);

    return;
}




/* @datasection [AjPList] String lists ****************************************
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
**
** @release 1.0.0
** @@
******************************************************************************/

AjPList ajListstrNew(void)
{
    return listNew(&listDelStr);
}




/* @func ajListstrNewList *****************************************************
**
** Copy a list, with copies of all the string values.
**
** General data cannot be copied so this is a string-only function
**
** @param [r] list [const AjPList] list to be copied
** @return [AjPList] new copied list.
**
** @release 5.0.0
** @@
******************************************************************************/

AjPList ajListstrNewList(const AjPList list)
{
    AjPList newlist = NULL;

    AjPListNode node = NULL;

    AjPStr tmpstr = NULL;

    if(!list)
        return NULL;

    newlist = listNew(&listDelStr);
    newlist->Fitemdel = list->Fitemdel;

    for( node=list->First; node->Next; node=node->Next)
    {
        tmpstr = ajStrNewS(node->Item);
        ajListstrPushAppend(newlist, tmpstr);
    }


    return newlist;
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
**
** @release 5.0.0
** @@
******************************************************************************/

AjPList ajListstrNewListref(const AjPList list)
{
    return ajListNewListref(list);
}




/* @section Adding values *****************************************************
**
** @fdata [AjPList]
**
** @nam3rule Push Add new key and value
** @nam4rule PushAppend Add to end of list
** @nam3rule Pushlist Add new list of values
**
** @argrule * list [AjPList] List
** @argrule Push str [AjPStr] Pointer to data.
** @argrule Pushlist Plist [AjPList*] Source AJAX List address to be deleted
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
** @param [u] str [AjPStr] String data.
** @return [void]
**
** @release 1.0.0
** @@
******************************************************************************/

void ajListstrPush(AjPList list, AjPStr str)
{
    ajListPush(list, (void*) str);

    return;
}




/* @func ajListstrPushAppend **************************************************
**
** Add a new node at the end of the list and add the
** data pointer.
**
** @param [u] list [AjPList] List to be changed.
** @param [u] str [AjPStr] String to append.
** @return [void]
**
** @release 5.0.0
** @@
******************************************************************************/

void ajListstrPushAppend(AjPList list, AjPStr str)
{
    ajListPushAppend(list, (void*) str);

    return;
}




/* @func ajListstrPushlist ****************************************************
**
** Adds a list to the start of the current list, then deletes the second list.
**
** @param [u] list [AjPList] List.
** @param [d] Plist [AjPList*] AJAX List address to be merged.
** @return [void]
**
** @release 5.0.0
** @@
******************************************************************************/

void ajListstrPushlist(AjPList list, AjPList* Plist)
{
    ajListPushlist(list, Plist);

    return;
}




/* @section Modifiers *********************************************************
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
**
** @release 1.0.0
** @@
******************************************************************************/

void ajListstrMap(AjPList list,
                  void (*apply)(AjPStr* str, void* cl), void* cl)
{
    AjPListNode rest = NULL;

    assert(apply);

    for(rest = list->First; rest->Next; rest = rest->Next)
        (*apply)((AjPStr*) &rest->Item, cl);

    return;
}




/* @func ajListstrReverse *****************************************************
**
** Reverse the order of the nodes in a string list.
**
** @param [u] list [AjPList] List
** @return [void]
**
** @release 1.0.0
** @@
******************************************************************************/

void ajListstrReverse(AjPList list)
{
    ajListReverse(list);

    return;
}




/* @section Removing data *****************************************************
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
**
** @release 1.0.0
** @@
******************************************************************************/

AjBool ajListstrPop(AjPList list, AjPStr* Pstr)
{
    if(!list)
        return ajFalse;

    if(Pstr)
    {
        ajStrDel(Pstr);
        *Pstr = (AjPStr) listNodeItem(list->First);
    }

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
**
** @release 5.0.0
** @@
******************************************************************************/

AjBool ajListstrPopLast(AjPList list, AjPStr* Pstr)
{
    AjPListNode node = NULL;

    if(!list)
        return ajFalse;

    if(!list->Count)
        return ajFalse;

    node = list->Last->Prev;

    if(Pstr)
    {
        ajStrDel(Pstr);
        *Pstr = (AjPStr) listNodeItem(node);
    }

    if(list->Count == 1)
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
        node->Prev->Next = list->Last;
        list->Last->Prev = node->Prev;

        if(listFreeNext >= listFreeMax)
            listFreeSetExpand();

        if(listFreeNext >= listFreeMax)
            AJFREE(node);
        else if(node)
        {
            listFreeSet[listFreeNext++] = node;
            node = NULL;
        }
    }

    --list->Count;

    return ajTrue;
}




/* @section Element retrieval *************************************************
**
** @fdata [AjPList]
**
** Retrieves attributes or values from a list without changing it
**
** @nam3rule Get Retrieve attribute
** @nam4rule GetLength Retrieve list length
** @nam4rule GetMemsize Retrieve memory used by list
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
** @argrule Mapread apply [void function] Function to apply
** @argrule Mapread cl [void*] Data (usually NULL) to be passed to apply.
** @argrule PeekNumber ipos [ajuint] Position in list
** @argrule Peek Pstr [AjPStr*] Value
** @argrule Toarray array [AjPStr**] Array of values, ending with NULL
**
** @valrule * [AjBool] True on success
** @valrule *Length [ajulong] List length
** @valrule *Memsize [ajulong] Memory used
** @valrule *Mapfind [AjBool]
** @valrule *Mapread [void]
** @valrule *Toarray [ajulong] Array size, excluding final NULL
**
** @fcategory cast
**
******************************************************************************/




/* @func ajListstrGetLength ***************************************************
**
** get the number of nodes in the linked list.
**
** @param [r] list [const AjPList] List
** @return [ajulong] Number of nodes in list.
**
** @release 5.0.0
** @@
******************************************************************************/

ajulong ajListstrGetLength(const AjPList list)
{
    return ajListGetLength(list);
}




/* @func ajListstrGetMemsize **************************************************
**
** get the memory used nodes in the linked string list.
**
** @param [r] list [const AjPList] List
** @return [ajulong] Number of bytes allocated in list.
**
** @release 6.5.0
** @@
******************************************************************************/

ajulong ajListstrGetMemsize(const AjPList list)
{
    ajulong lbytes = 0L;
    AjPListNode node = NULL;
    AjPStr tmpstr;

    if(!list)
        return lbytes;

    for(node = list->First; node->Next; node=node->Next)
    {
        tmpstr = (AjPStr) listNodeItem(node);
        lbytes += (sizeof(*node) + sizeof(*tmpstr) + tmpstr->Res);
    }

    lbytes += sizeof(*node);
    lbytes += sizeof(*list);

    return lbytes;
}




/* @func ajListstrMapfind *****************************************************
**
** For each node in the list call function apply and return
** ajTrue when any node is matched by the function.
**
** @param [r] list [const AjPList] List
** @param [f] apply [AjBool function] Function to call to test each list item.
** @param [u] cl [void*] Standard, usually NULL.
** @return [AjBool] ajTrue on success.
**
** @release 5.0.0
** @@
******************************************************************************/

AjBool ajListstrMapfind(const AjPList list,
                        AjBool (*apply)(AjPStr* Pstr, void* cl),
                        void* cl)
{
    AjPListNode listnode = NULL;

    assert(list);

    assert(apply);

    for(listnode = list->First; listnode->Next; listnode = listnode->Next)
        if((*apply)((AjPStr*) &listnode->Item, cl))
            return ajTrue;

    return ajFalse;
}




/* @func ajListstrMapread *****************************************************
**
** For each node in the list call function apply,
** with the address of the string and a client pointer.
** The apply function must not modify the list elements.
**
** @param [r] list [const AjPList] List.
** @param [f] apply [void function] Function to call for each list item.
** @param [u] cl [void*] Standard, usually NULL.
** @return [void]
**
** @release 5.0.0
** @@
******************************************************************************/

void ajListstrMapread(const AjPList list,
                      void (*apply)(AjPStr str, void* cl), void* cl)
{
    AjPListNode rest = NULL;

    assert(apply);

    for(rest = list->First; rest->Next; rest = rest->Next)
        (*apply)((AjPStr) rest->Item, cl);


    return;
}




/* @func ajListstrPeek ********************************************************
**
** Return the first node but keep it on the list.
**
** @param [r] list [const AjPList] List
** @param [w] Pstr [AjPStr*] String
** @return [AjBool] ajTrue on success.
**
** @release 2.7.0
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
** @return [ajulong] Size of array of pointers.
**
** @release 5.0.0
** @@
******************************************************************************/

ajulong ajListstrToarray(const AjPList list, AjPStr** array)
{
    ajulong i = 0UL;
    ajulong n = 0UL;

    AjPListNode rest = NULL;

    n = list->Count;
    rest = list->First;

    if(!n)
    {
        *array = NULL;

        return 0UL;
    }

    *array = AJALLOC((size_t) (n + 1) * sizeof(array));

    for(i = 0UL; i < n; i++)
    {
        (*array)[i] = (AjPStr) rest->Item;
        rest = rest->Next;
    }

    (*array)[n] = (AjPStr) NULL;

    return n;
}




/* @func ajListstrToarrayAppend ***********************************************
**
** append to an array of the pointers to the data.
**
** @param [r] list [const AjPList] List
** @param [w] array [AjPStr**] Array of Strings.
**
** @return [ajulong] Size of array of pointers.
**
**
** @release 5.0.0
** @@
******************************************************************************/

ajulong ajListstrToarrayAppend(const AjPList list, AjPStr** array)
{
    ajulong i = 0UL;
    ajulong n = 0UL;
    ajulong j = 0UL;

    AjPListNode rest = NULL;

    rest = list->First;

    if(*array)
    {
        for(j = 0UL; array[j]; j++)
            continue;
    }
    else
        j = 0UL;

    n = list->Count + j;

    if(!n)
    {
        *array = NULL;

        return 0UL;
    }

    AJCRESIZE(*array, (size_t) (n + 1));

    for(i = j; i < n; i++)
    {
        (*array)[i] = (AjPStr) rest->Item;
        rest = rest->Next;
    }

    (*array)[n] = (AjPStr) NULL;

    return n;
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
**
** @release 1.0.0
** @@
******************************************************************************/

void ajListstrTrace(const AjPList list)
{
    ajulong i = 0UL;

    AjPListNode node = NULL;

    if(!list)
        return;

    ajDebug("\nList Trace %p count %Lu\n",
            list, list->Count);
    ajDebug("rest-> %p last-> %p\n",
            list->First, list->Last);

    for(node = list->First; node->Next; node = node->Next)
    {
        i++;
        ajDebug("Item[%Lu] item %p '%S' rest -> %p prev -> %p\n",
                i, node, (AjPStr) node->Item, node->Next, node->Prev);
    }

    if(i != list->Count)
    {
        ajErr("*** list error expect %Lu items, found %Lu",
              list->Count, i);
    }

    if(list->Last != node)
    {
        ajErr("*** list error expect end at %p, found at %p",
              list->Last, node);
    }

    return;
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
** @argrule * Plist [AjPList*] AJAX List address
**
** @valrule * [void]
**
** @fcategory delete
**
******************************************************************************/




/* @func ajListstrFree ********************************************************
**
** Free the list. Do not attempt to free the nodes.
** For use where the node data has been saved elsewhere, for example
** by ajListToarray or where the list is a temporary structure
** referring to permanent data.
**
** @param [d] Plist [AjPList*] AJAX List address
** @return [void]
**
** @release 1.0.0
** @@
******************************************************************************/

void ajListstrFree(AjPList* Plist)
{
    ajListFree(Plist);

    return;
}




/* @func ajListstrFreeData ****************************************************
**
** Free all nodes in a string list.
** Also deletes all the strings. If these are to be preserved,
** use ajListstrDel instead.
**
** @param [d] Plist [AjPList*] AJAX List address
** @return [void]
**
** @release 5.0.0
** @@
******************************************************************************/

void ajListstrFreeData(AjPList* Plist)
{
    AjPList list = NULL;

    AjPListNode  next = NULL;
    AjPListNode* rest = NULL;

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
        for(; (*rest)->Next; *rest = next)
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




/* @datasection [AjIList] String list iterators *******************************
**
** Function is for manipulating lists with string values.
**
** Some functions are specially designed to understand string (AjPStr) values.
**
** @nam2rule Liststr String list data
** @nam3rule Iter Iterator
**
******************************************************************************/




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
******************************************************************************/




/* @func ajListstrIterGet *****************************************************
**
** Returns next item using iterator, or steps off the end.
**
** @param [u] iter [AjIList] List iterator.
** @return [AjPStr] Data item returned.
**
** @release 6.0.0
** @@
******************************************************************************/

AjPStr ajListstrIterGet(AjIList iter)
{
    void* item = NULL;

    AjPListNode node = NULL;

    if(!iter)
        return NULL;

    node = iter->Here;

    if(!iter->Back)
    {
        if(!node->Next)
            return NULL;

        item = node->Item;
        iter->Here = node->Next;
    }
    else
    {
        if(!node->Next->Next || !node->Next->Next->Next)
            return NULL;

        iter->Back = ajFalse;
        item = node->Next->Item;
        iter->Here = node->Next->Next;
    }

    return (AjPStr) item;
}




/* @func ajListstrIterGetBack *************************************************
**
** Returns next item using back iterator.
**
** @param [u] iter [AjIList] List iterator.
** @return [AjPStr] Data item returned.
**
** @release 6.0.0
** @@
******************************************************************************/

AjPStr ajListstrIterGetBack(AjIList iter)
{
    void* item = NULL;

    AjPListNode node = NULL;

    if(!iter)
        return NULL;

    node = iter->Here;

    if(!node->Prev)
        return NULL;

    if(!iter->Back)
    {
        item = node->Prev->Prev->Item;
        iter->Here = node->Prev->Prev;
        iter->Back = ajTrue;
    }
    else
    {
        item = node->Prev->Item;
        iter->Here = node->Prev;
    }

    return (AjPStr) item;
}




/* @section modifiers *********************************************************
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
******************************************************************************/




/* @func ajListstrIterInsert **************************************************
**
** Insert an item in a list, using an iterator (if not null)
** to show which position to insert. Otherwise, simply push.
**
** @param [u] iter [AjIList] List iterator.
** @param [u] str [AjPStr] String to insert.
** @return [void]
**
** @release 5.0.0
** @@
******************************************************************************/

void ajListstrIterInsert(AjIList iter, AjPStr str)
{
    AjPList list = iter->Head;

    AjPListNode node = NULL;

    /*ajDebug("ajListstrInsert\n");*/
    ajListstrTrace(list);
    ajListstrIterTrace(iter);

    if(!iter->Modify)
    {
        ajErr("ajListInsert called on a read-only iterator");
        return;
    }

    node = iter->Here;


    if(!iter->Back)
    {
        if(!node->Prev)
            listInsertNode(&list->First, str);
        else
            listInsertNode(&node->Prev->Next, str);
        iter->Here = node->Prev;
    }
    else
    {
        if(!node->Next)
            ajFatal("Cannot add a new node for unused back iterator\n");

        if(!node->Prev)
            listInsertNode(&list->First, str);
        else
            listInsertNode(&node->Prev->Next, str);
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




/* @func ajListstrIterRemove **************************************************
**
** Remove an item from a list, using an iterator (if not null)
** to show which item. Otherwise remove the first item.
**
** We want to remove the item just fetched by the iterator.
**
** @param [u] iter [AjIList] List iterator.
** @return [void]
**
** @release 5.0.0
** @@
******************************************************************************/

void ajListstrIterRemove(AjIList iter)
{
    AjPListNode node = NULL;

    /* ajDebug("ajListstrIterRemove\n");*/

    if(!iter->Modify)
    {
        ajErr("ajListstrIterRemove called on a read-only iterator");
        return;
    }

    node = iter->Here;

    if(!iter->Back)
    {
        if(!node->Prev)
            ajFatal("Attempt to delete from unused iterator\n");

        if(!node->Prev->Prev)
        {
            ajStrDel((AjPStr*) &(iter->Head->First->Item));
            listNodeDel(&(iter->Head->First));
        }
        else
        {
            ajStrDel((AjPStr*) &node->Prev->Prev->Next->Item);
            listNodeDel(&node->Prev->Prev->Next);
        }
    }
    else
    {
        ajStrDel((AjPStr*) &node->Prev->Prev->Next->Item);
        listNodeDel(&node->Prev->Prev->Next);
    }

    iter->Head->Count--;

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
**
** @release 1.0.0
** @@
******************************************************************************/

void ajListstrIterTrace(const AjIList iter)
{
    if(!iter)
        return;

    ajDebug("\nIterator Head %p ReadHead %p Here %p Back %B Item %S\n",
            iter->Head, iter->ReadHead, iter->Here,iter->Back,
            (AjPStr)iter->Here->Item);

    return;
}




#ifdef AJ_COMPILE_DEPRECATED_BOOK
#endif




#ifdef AJ_COMPILE_DEPRECATED
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




/* @obsolete ajListPushApp
** @rename ajListPushAppend
*/

__deprecated void ajListPushApp(AjPList list, void* item)
{
    ajListPushAppend(list, item);
    return;
}




/* @obsolete ajListPushList
** @rename ajListPushlist
*/

__deprecated void ajListPushList(AjPList list, AjPList* Plist)
{
    ajListPushlist(list, Plist);
    return;
}




/* @obsolete ajListGarbageCollect
** @replace ajListPurge (1,2,3/1,3,2)
*/

__deprecated void ajListGarbageCollect(AjPList list,
                                       void (*itemdel)(void** Pitem),
                                       AjBool (*test)(const void* item))
{
    ajListPurge(list, test, itemdel);
    return;
}




/* @obsolete ajListSort2
** @rename ajListSortTwo
*/

__deprecated void ajListSort2(AjPList list,
                              int (*compar1)(const void* item1,
                                             const void* item2),
                              int (*compar2)(const void* item1,
                                             const void* item2))
{
    ajListSortTwo(list, compar1, compar2);

    return;
}




/* @obsolete ajListSort3
** @rename ajListSortTwoThree
*/

__deprecated void ajListSort3(AjPList list,
                              int (*compar1)(const void* item1,
                                             const void* item2),
                              int (*compar2)(const void* item1,
                                             const void* item2),
                              int (*compar3)(const void* item1,
                                             const void* item2))
{
    ajListSortTwoThree(list, compar1, compar2, compar3);

    return;
}




/* @obsolete ajListUnique2
** @rename ajListSortTwoUnique
*/

__deprecated void ajListUnique2(AjPList list,
                                int (*compar1)(const void* item1,
                                               const void* item2),
                                int (*compar2)(const void* item1,
                                               const void* item2),
                                void (*itemdel)(void** Pitem,
                                                void* cl))
{
    ajListSortTwoUnique(list, compar1, compar2, itemdel);

    return;
}




/* @obsolete ajListUnique
** @rename ajListSortUnique
*/

__deprecated void ajListUnique(AjPList list,
                               int (*compar)(const void* item1,
                                             const void* item2),
                               void (*itemdel)(void** Pitem,
                                               void* cl))
{
    ajListSortUnique(list, compar, itemdel);

    return;
}




/* @obsolete ajListPopEnd
** @rename ajListPopLast
*/

__deprecated AjBool ajListPopEnd(AjPList list, void** Pitem)
{
    return ajListPopLast(list, Pitem);
}




/* @obsolete ajListLength
** @rename ajListGetLength
*/

__deprecated ajuint ajListLength(const AjPList list)
{
    return ajListGetLength(list);
}




/* @obsolete ajListFind
** @rename ajListMapfind
*/

__deprecated AjBool ajListFind(const AjPList list,
                               AjBool (*apply)(void** Pitem, void* cl),
                               void* cl)
{
    return ajListMapfind(list, apply, cl);
}




/* @obsolete ajListMapRead
** @rename ajListMapread
*/

__deprecated void ajListMapRead(const AjPList list,
                                void (*apply)(void* item, void* cl), void* cl)
{
    ajListMapread(list, apply, cl);
    return;
}




/* @obsolete ajListFirst
** @rename ajListPeekFirst
*/

__deprecated AjBool ajListFirst(const AjPList list, void** Pitem)
{
    return ajListPeekFirst(list, Pitem);
}




/* @obsolete ajListLast
** @rename ajListPeekLast
*/

__deprecated AjBool ajListLast(const AjPList list, void** Pitem)
{
    return ajListPeekLast(list, Pitem);
}




/* @obsolete ajListNth
** @rename ajListPeekNumber
*/

__deprecated AjBool ajListNth(const AjPList list, ajuint ipos, void** Pitem)
{
    return ajListPeekNumber(list, ipos, Pitem);
}




/* @obsolete ajListToArray
** @rename ajListToarray
*/

__deprecated ajuint ajListToArray(const AjPList list, void*** array)
{
    return ajListToarray(list, array);
}




/* @obsolete ajListDel
** @rename ajListFree
*/

__deprecated void ajListDel(AjPList* Plist)
{
    AjPList list = NULL;

    AjPListNode* rest = NULL;
    AjPListNode  next = NULL;

    if(!Plist)
        return;

    if(!*Plist)
        return;

#ifdef AJ_SAVESTATS
    listDelCnt++;
#endif

    list = *Plist;

    if(!list->Use)
        ajErr("trying to delete unused list");

    --list->Use;

    if(!list->Use)
    {                                   /* any other references? */
        rest = &list->First;

        if(list->Count)
            for(; (*rest)->Next; *rest = next)
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
    }

    *Plist = NULL;

    return;
}




/* @obsolete ajListIter
** @rename ajListIterNew
*/

__deprecated AjIList ajListIter(AjPList list)
{
    return ajListIterNew(list);
}




/* @obsolete ajListIterBack
** @rename ajListIterNewBack
*/

__deprecated AjIList ajListIterBack(AjPList list)
{
    return ajListIterNewBack(list);
}




/* @obsolete ajListIterRead
** @rename ajListIterNewread
*/

__deprecated AjIList ajListIterRead(const AjPList list)
{
    return ajListIterNewread(list);
}




/* @obsolete ajListIterBackRead
** @rename ajListIterNewreadBack
*/

__deprecated AjIList ajListIterBackRead(const AjPList list)
{
    return ajListIterNewreadBack(list);
}




/* @obsolete ajListNodesNew
** @remove Use calls to ajListPush
*/
__deprecated AjPListNode ajListNodesNew(void* x, ...)
{
    va_list ap;

    AjPListNode topnode = NULL;
    AjPListNode node    = NULL;

    va_start(ap, x);

    topnode = listDummyNode(&node);

    /*ajDebug("ajListNodesNew topnode: %p -> %p\n",
      topnode, topnode->Next);*/
    for(; x; x = va_arg(ap, void*))
    {
        node->Item = x;
        listDummyNode(&node->Next);
        node->Next->Prev = node;
        /*ajDebug("topnode: %p node: %p, item: %p -> %p\n",
          topnode, node, x, node->Next);*/
    }

    va_end(ap);

    topnode->Prev = NULL;

    /*listNodesTrace(node);*/

    return node;
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
    {                           /* need to get to the end of the list */
        more = more->Next;
        list->Count++;
#ifdef AJ_SAVESTATS
        if(list->Count > listMaxNum)
            listMaxNum = list->Count;
#endif
    }

    list->Last = more;          /* now we can set the end of the list */
    AJFREE(*morenodes);         /* first extra node (only) was duplicated */

    return;
}




/* @obsolete ajListIterMoreBack
** @remove use ajListIterDoneBack
*/

__deprecated AjBool ajListIterMoreBack(const AjIList iter)
{
    return (!ajListIterDoneBack(iter));
}




/* @obsolete ajListIterBackDone
** @rename ajListIterDoneBack
*/

__deprecated AjBool ajListIterBackDone(const AjIList iter)
{
    return ajListIterDoneBack(iter);
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




/* @obsolete ajListIterNext
** @rename ajListIterGet
*/

__deprecated void* ajListIterNext(AjIList iter)
{
    return ajListIterGet(iter);
}




/* @obsolete ajListIterBackNext
** @rename ajListIterGetBack
*/

__deprecated void* ajListIterBackNext(AjIList iter)
{
    return ajListIterGetBack(iter);
}




/* @obsolete ajListInsert
** @rename ajListIterInsert
*/

__deprecated void ajListInsert(AjIList iter, void* item)
{
    ajListIterInsert(iter, item);
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




/* @obsolete ajListstrClone
** @remove use ajListstrNewList
*/

__deprecated ajuint ajListstrClone(const AjPList list, AjPList newlist)
{
    ajint ret = 0;

    AjPListNode node = NULL;

    AjPStr newstr = NULL;

    if(!list)
        return 0U;

    if(!newlist)
        return 0U;

    for(node = list->First; node->Next; node = node->Next)
    {
        newstr = NULL;
        ajStrAssignS(&newstr, node->Item);
        ajListPushAppend(newlist, newstr);
        ret++;
    }

    return ret;
}




/* @obsolete ajListstrCopy
** @rename ajListstrNewList
*/

__deprecated AjPList ajListstrCopy(const AjPList list)
{
    return ajListstrNewListref(list);
}




/* @obsolete ajListstrPushApp
** @rename ajListstrPushAppend
*/

__deprecated void ajListstrPushApp(AjPList list, AjPStr str)
{

    ajListstrPushAppend(list, str);

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




/* @obsolete ajListstrPopEnd
** @rename ajListstrPopLast
*/

__deprecated AjBool ajListstrPopEnd(AjPList list, AjPStr* Pstr)
{
    return ajListstrPopLast(list, Pstr);
}




/* @obsolete ajListstrLength
** @rename ajListstrGetLength
*/

__deprecated ajuint ajListstrLength(const AjPList list)
{
    return ajListstrGetLength(list);
}




/* @obsolete ajListstrFind
** @rename ajListstrMapfind
*/

__deprecated AjBool ajListstrFind(const AjPList list,
                                  AjBool (*apply)(AjPStr* Pstr, void* cl),
                                  void* cl)
{
    return ajListstrMapfind(list, apply, cl);
}




/* @obsolete ajListstrMapRead
** @rename ajListstrMapread
*/

__deprecated void ajListstrMapRead(const AjPList list,
                                   void (*apply)(AjPStr str, void* cl),
                                   void* cl)
{

    ajListstrMapread(list, apply, cl);

    return;
}




/* @obsolete ajListstrToArray
** @rename ajListstrToarray
*/

__deprecated ajuint ajListstrToArray(const AjPList list, AjPStr** array)
{
    return ajListstrToarray(list, array);
}




/* @obsolete ajListstrToArrayApp
** @rename ajListstrToarrayAppend
*/

__deprecated ajuint ajListstrToArrayApp(const AjPList list, AjPStr** array)
{
    return ajListstrToarrayAppend(list, array);
}




/* @obsolete ajListstrNewArgs
** @remove Use ajListstrNew and ajListPush
*/
__deprecated AjPList ajListstrNewArgs(AjPStr x, ...)
{
    va_list ap;
    ajint i = 0;

    AjPList list = NULL;

    AjPStr y = NULL;

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




/* @obsolete ajListstrDel
** @rename ajListstrFree
*/

__deprecated void ajListstrDel(AjPList* Plist)
{
    ajListstrFree(Plist);

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




/* @obsolete ajListstrRemove
** @rename ajListstrIterRemove
*/

__deprecated void ajListstrRemove(AjIList iter)
{
    ajListstrIterRemove(iter);

    return;
}
#endif
