/******************************************************************************
** @source AJAX tree functions
** These functions create and control trees.
**
** @author Copyright (C) 2004 Peter Rice
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

/*Library* Tree Library *******************************************************
**
** All trees consist of an initial node, with pointers to the next node
** down, right and also (if this is a subtree) up and left
**
** Each node has a type to indicate the data stored.
**
******************************************************************************/

#include <stddef.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>

#include "ajax.h"

static ajint treeNewCnt     = 0;
static ajint treeDelCnt     = 0;
static ajint treeMaxNum     = 0;
static ajint treeNodeCnt    = 0;
static ajint treeIterNewCnt = 0;
static ajint treeIterDelCnt = 0;

static AjPStr treeFollowPath = NULL;

static AjPTree treeNew(AjEnum type);
static void treeTraceNode (const AjPTree thys, ajint num);
static void treestrTraceNode (const AjPTree thys, ajint num);




/* @func ajTreeNew ************************************************************
**
** Creates a new general tree.
**
** @return [AjPTree] new tree;
** @category new [AjPTree] Creates a new general tree.
** @@
******************************************************************************/

AjPTree ajTreeNew(void)
{
    return treeNew(ajETreeAny);
}




/* @func ajTreeNewNewick *******************************************************
**
** Creates a new general tree using a Newick's format phylogenetic tree
** as input
**
** @param [r] newick [const AjPStr] Newick's format tree
** @return [AjPTree] new tree;
** @category new [AjPTree] Creates a new general tree.
** @@
******************************************************************************/

AjPTree ajTreeNewNewick(const AjPStr newick)
{
    AjPTree ret = NULL;
    AjPTree tree = NULL;
    AjPStr result = NULL;
    const char* cp;

    ajint nparam = 0;
    ajint ncomment = 0;
    AjBool iscomment = AJFALSE;
    AjBool isdone = AJFALSE;
    AjBool isquote = AJFALSE;
    AjBool islength = AJFALSE;
    AjPStr tmpstr = NULL;
    AjPStr tmpname = NULL;
    double *flen;

    cp = ajStrGetPtr(newick);

    while(*cp && (*cp != '('))
	cp++;

    if(!*cp)
	return NULL;

    ret = treeNew(ajETreeAny);
    tree = ret;
    isdone = ajFalse;

    while (!isdone && *cp)
    {
	if(iscomment)
	{
	    if(*(cp++) == ']')
		iscomment = ajFalse;

	    continue;
	}

	switch(*cp)
	{
            case '(':
                nparam++;
                tree = ajTreeAddNode(tree);
                break;
            case ')':
                if(!nparam)
                {
                    ajWarn("Bad Newick string '%S' too many closing "
                           "parentheses",
                           newick);
                    break;
                }

                if(islength)
                {
                    AJNEW0(flen);
                    ajStrToDouble(tmpstr, flen);
                    ajTreeAddData(tree, tmpname, flen);
                    ajStrAssignClear(&tmpname);
                    ajStrAssignClear(&tmpstr);
                    islength = ajFalse;
                }
                else
                {
                    ajTreeAddData(tree, tmpstr, NULL);
                    ajStrAssignClear(&tmpstr);
                }
                tree = ajTreeUp(tree);
                nparam--;
                break;
            case ',':
                if(islength)
                {
                    AJNEW0(flen);
                    ajStrToDouble(tmpstr, flen);
                    ajTreeAddData(tree, tmpname, flen);
                    ajStrAssignClear(&tmpname);
                    ajStrAssignClear(&tmpstr);
                    islength = ajFalse;
                }
                else
                {
                    ajTreeAddData(tree, tmpstr, NULL);
                    ajStrAssignClear(&tmpstr);
                }

                tree = ajTreeAddSubNode(tree);
                break;
            case ';':
                if(nparam)
                    ajWarn("Bad Newick string '%S' too few closing parentheses",
                           newick);

                if(islength)
                {
                    AJNEW0(flen);
                    ajStrToDouble(tmpstr, flen);
                    ajTreeAddData(tree, tmpname, flen);
                    ajStrAssignClear(&tmpname);
                    ajStrAssignClear(&tmpstr);
                    islength = ajFalse;
                }
                isdone = ajTrue;
                break;
            case ':':
                if(islength)
                    ajWarn("Bad Newick string '%S' multiple length values",
                           newick);

                islength = ajTrue;
                ajStrAssignS(&tmpname, tmpstr);
                ajStrAssignClear(&tmpstr);
                break;
            case '[':
                ncomment++;
                iscomment = ajTrue;
                break;
            case '\'':
                if(isquote)
                {
                    if(*(cp+1) == '\'')
                    {
                        ajStrAppendK(&tmpstr, *cp);
                        cp++;
                    }
                    else
                        isquote = ajFalse;
                }
                else
                    isquote = ajTrue;

                break;

            default:
                if(isspace(*cp))
                {
                    if(!islength && (*cp == ' ') &&
                       ajStrGetCharLast(tmpstr) != '_')
                        ajStrAppendK(&tmpstr, '_');
                }
                else
                    ajStrAppendK(&tmpstr, *cp);
                break;
	}
	cp++;
    }

    if(iscomment)
	ajWarn("Bad Newick string '%S' unclosed comment",
	       newick);
    if(!isdone)
	ajWarn("Bad Newick string '%S' no final ';'",
	       newick);

    ajTreeToNewick(ret, &result);


    return ret;
}




/* @func ajTreestrNew *********************************************************
**
** Creates a new string tree.
**
** @return [AjPTree] new tree;
** @category new [AjPTree] Creates a new AjPStr tree.
** @@
******************************************************************************/

AjPTree ajTreestrNew(void)
{
    return treeNew(ajETreeStr);
}




/* @funcstatic treeNew ********************************************************
**
** Creates a new tree.
**
** @param [r] type [AjEnum] Defined tree type.
** @return [AjPTree] new tree;
** @@
******************************************************************************/

static AjPTree treeNew(AjEnum type)
{
    AjPTree tree;

    AJNEW0(tree);
    tree->Type = type;

    treeNodeCnt++;			/* dummy */
    treeNewCnt++;

    return tree;
}




/* @func ajTreestrCopy ********************************************************
**
** Copy a string tree.
**
** WARNING: pointers to the data are copied, NOT the data
**          so be careful when cleaning up after copy.
**
** @param [r] thys [const AjPTree] Tree to be copied
** @return [AjPTree] New, copied, tree.
** @category new [AjPTree] Copies an AjPStr tree to a new tree.
** @@
******************************************************************************/

AjPTree ajTreestrCopy(const AjPTree thys)
{
    return ajTreeCopy(thys);
}




/* @func ajTreeCopy ***********************************************************
**
** Copy a tree.
**
** WARNING: pointers to the data are copied, NOT the data
**          so be careful when cleaning up after copy.
**
** @param [r] thys [const AjPTree] tree to be copied
** @return [AjPTree] new copied tree.
** @category new [AjPTree] Copies a tree to a new tree.
** @@
******************************************************************************/

AjPTree ajTreeCopy(const AjPTree thys)
{
    AjPTree newtree;

    if(!thys)
	return NULL;

    newtree = ajTreeNew();
    newtree->Type = thys->Type;

    return newtree;
}




/* @func ajTreeLength *********************************************************
**
** get the number of nodes in the linked tree.
**
** @param [r] thys [const AjPTree] Tree
** @return [ajuint] Number of nodes in tree.
** @category cast [AjPTree] get the number of nodes in a linked tree.
** @@
******************************************************************************/

ajuint ajTreeLength(const AjPTree thys)
{
    AjPTree tree;
    ajint inode=0;

    tree = ajTreeFollow(NULL, thys);

    while(tree)
    {
	if(tree->Data)
            inode++;

	tree = ajTreeFollow(tree, thys);
    }

    return inode;
}




/* @func ajTreestrLength ******************************************************
**
** get the number of nodes in the linked tree.
**
** @param [r] thys [const AjPTree] Tree
** @return [ajuint] Number of nodes in tree.
** @category cast [AjPTree] get the number of nodes in an AjPStr linked tree.
** @@
******************************************************************************/

ajuint ajTreestrLength(const AjPTree thys)
{
    return ajTreeLength(thys);
}




/* @func ajTreeFree ***********************************************************
**
** Free all nodes in the tree.
** NOTE: The data is only freed with a specified tree type.
**       For undefined data types we recommend you to
**       use ajTreeMap with a routine to free the memory.
**
** @param [d] pthis [AjPTree*] Tree
** @return [void]
** @category delete [AjPTree] Free the tree, and free the items with
**                            a simple "free".
** @@
******************************************************************************/

void ajTreeFree(AjPTree* pthis)
{
    AjPTree topnode = NULL;
    AjPTree next    = NULL;
    AjPTree tree    = NULL;
    AjPTree parent  = NULL;

    if(!pthis)
	return;

    if(!*pthis)
	return;

    treeDelCnt++;

    topnode = *pthis;

    tree = ajTreeDown(topnode);

    while(tree && tree != topnode)
    {
	ajDebug("Tree %p Down: %p Right: %p Left: %p Up: %p\n",
	       tree, tree->Down, tree->Right, tree->Left, tree->Up);
	/* if we can go down, simply do so */
	next = ajTreeDown(tree);

	if(next)
	{
	    tree = next;
	    continue;
	}

	/* if we can go right, remove the node we left */
	next = ajTreeNext(tree);

	if(next)
	{
	    AJFREE(tree->Data);
	    AJFREE(tree);
	    tree = next;
	    continue;
	}

	/* if we can go up, remove the node we left */
	/* if not, we are at the top of the whole tree */
	next = ajTreeUp(tree);
	next->Down = NULL;

	AJFREE(tree->Data);
	AJFREE(tree);
	tree = next;
    }

    /* remove the top (sub) node */
    if(tree)
    {
	parent = tree->Up;

	if(parent)
	{
	    if(parent->Down == tree)
		parent->Down = tree->Right;
	    if(tree->Left)
		tree->Left->Right = tree->Right;
	}

	AJFREE(tree->Data);
    }

    AJFREE(*pthis);

    return;
}




/* @func ajTreestrFree ********************************************************
**
** Free all nodes in a string tree.
** Also deletes all the strings. If these are to be preserved,
** use ajTreestrDel instead.
**
** @param [d] pthis [AjPTree*] Tree
** @return [void]
** @category delete [AjPTree] Free the tree, and free the items with ajStrDel
** @@
******************************************************************************/

void ajTreestrFree(AjPTree* pthis)
{
    AjPTree topnode = NULL;
    AjPTree next    = NULL;
    AjPTree tree    = NULL;
    AjPTree parent  = NULL;

    if(!pthis)
	return;

    if(!*pthis)
	return;

    treeDelCnt++;

    topnode = *pthis;

    tree = ajTreeDown(topnode);

    while(tree && tree != topnode)
    {
	/* if we can go down, simply do so */
	next = ajTreeDown(tree);

	if(next)
	{
	    tree = next;
	    continue;
	}

	/* if we can go right, remove the node we left */
	next = ajTreeNext(tree);

	if(next)
	{
	    ajStrDel((AjPStr*)&tree->Data);
	    AJFREE(tree);
	    tree = next;
	    continue;
	}

	/* if we can go up, remove the node we left */
	/* if not, we are at the top of the whole tree */
	next = ajTreeUp(tree);
	ajStrDel((AjPStr*)&tree->Data);
	AJFREE(tree);
	tree = next;
    }

    /* remove the top (sub) node */
    if(tree)
    {
	parent = tree->Up;

	if(parent)
	{
	    if(parent->Down == tree)
		parent->Down = tree->Right;
	    if(tree->Left)
		tree->Left->Right = tree->Right;
	}

	ajStrDel((AjPStr*)&tree->Data);
	AJFREE(tree);
    }

    AJFREE(*pthis);

    return;
}




/* @func ajTreeDel ************************************************************
**
** Free the tree. Do not attempt to free the nodes.
** For use where the node data has been saved elsewhere, for example
** by ajTreeToArray or where the tree is a temporary structure
** referring to permanent data.
**
** @param [d] pthis [AjPTree*] Tree
** @return [void]
** @category delete [AjPTree] Free the tree but do not try to free the nodes.
**                            Nodes should be freed first by ajTreeMap.
** @@
******************************************************************************/

void ajTreeDel(AjPTree* pthis)
{
    AjPTree topnode = NULL;
    AjPTree next    = NULL;
    AjPTree tree    = NULL;
    AjPTree parent  = NULL;

    if(!pthis)
	return;

    if(!*pthis)
	return;

    treeDelCnt++;

    topnode = *pthis;

    tree = ajTreeDown(topnode);

    while(tree && tree != topnode)
    {
	/* if we can go down, simply do so */
	next = ajTreeDown(tree);

	if(next)
	{
	    tree = next;
	    continue;
	}

	/* if we can go right, remove the node we left */
	next = ajTreeNext(tree);

	if(next)
	{
	    AJFREE(tree);
	    tree = next;
	    continue;
	}

	/* if we can go up, remove the node we left */
	/* if not, we are at the top of the whole tree */
	next = ajTreeUp(tree);
	AJFREE(tree);
	tree = next;
    }

    /* remove the top (sub) node */
    if(tree)
    {
	parent = tree->Up;

	if(parent)
	{
	    if(parent->Down == tree)
		parent->Down = tree->Right;
	    if(tree->Left)
		tree->Left->Right = tree->Right;
	}

	AJFREE(tree);
    }
    AJFREE(*pthis);

    return;
}




/* @func ajTreestrDel *********************************************************
**
** Free the tree. Do not attempt to free the nodes.
** For use where the node data has been saved elsewhere, for example
** by ajTreeToArray or where the tree is a temporary structure
** referring to permanent data.
**
** @param [d] pthis [AjPTree*] Tree
** @return [void]
** @category delete [AjPTree] Free the tree but do not try to free the nodes.
**                         use where nodes are still in use,
**                         e.g. in ajTreeToArray.
** @@
******************************************************************************/

void ajTreestrDel(AjPTree* pthis)
{
    ajTreeDel(pthis);

    return;
}




/* @func ajTreeMap ************************************************************
**
** For each data node in the tree call function apply.
**
** @param [u] thys [AjPTree] Tree.
** @param [f] apply [void function] Function to call for each tree item.
** @param [u] cl [void*] Standard, usually NULL.
** @return [void]
** @category modify [AjPTree] Call a function for each node in a tree.
** @@
******************************************************************************/

void ajTreeMap(AjPTree thys, void apply(void** x, void* cl), void* cl)
{
    AjPTree tree;

    assert(apply);

    tree = ajTreeFollow(NULL, thys);

    while(tree)
    {
	if(tree->Data)
	    apply((void**) &tree->Data, cl);

	tree = ajTreeFollow(tree, thys);
    }

    return;
}




/* @func ajTreestrMap *********************************************************
**
** For each node in the tree call function apply,
** with the address of the string and a client pointer.
**
** @param [u] thys [AjPTree] Tree.
** @param [f] apply [void function] Function to call for each tree item.
** @param [u] cl [void*] Standard, usually NULL.
** @return [void]
** @category modify [AjPTree] Call a function for each node in a tree.
** @@
******************************************************************************/

void ajTreestrMap(AjPTree thys, void apply(AjPStr* x, void* cl), void* cl)
{

    AjPTree tree;

    assert(apply);

    tree = ajTreeFollow(NULL, thys);

    while(tree)
    {
	if(tree->Data)
	    apply((AjPStr*) &tree->Data, cl);

	tree = ajTreeFollow(tree, thys);
    }

    return;
}




/* @func ajTreeToArray ********************************************************
**
** Create an array of the pointers to the data.
**
** @param [r] thys [const AjPTree] Tree
** @param [w] array [void***] Array of pointers to tree items.
** @return [ajuint] Size of array of pointers.
** @category cast [AjPTree] Create an array of the pointers to the data.
** @@
******************************************************************************/

ajuint ajTreeToArray(const AjPTree thys, void*** array)
{
    ajuint i = 0;
    ajuint n;
    AjPTree tree;

    n = ajTreeLength(thys);
 
    if(!n)
    {
	*array = NULL;

	return 0;
    }

    if (*array)
	AJFREE(*array);

    *array = AJALLOC((n+1)*sizeof(array));

    tree = ajTreeFollow(NULL, thys);

    while(tree)
    {
	if(tree->Data)
	    (*array)[i++] = tree->Data;

	tree = ajTreeFollow(tree, thys);
    }

    (*array)[n] = NULL;

    return n;
}




/* @func ajTreestrToArray *****************************************************
**
** Create an array of the pointers to the data.
**
** @param [r] thys [const AjPTree] Tree
** @param [w] array [AjPStr**] Array of Strings.
**
** @return [ajuint] Size of array of pointers.
** @category cast [AjPTree] Create an array of the pointers to the data.
** @@
******************************************************************************/

ajuint ajTreestrToArray(const AjPTree thys, AjPStr** array)
{
    ajuint i = 0;
    ajuint n;
    AjPTree tree;

    n = ajTreeLength(thys);
 
    if(!n)
    {
	*array = NULL;

	return 0;
    }

    if (*array)
	AJFREE(*array);

    *array = AJALLOC((n+1)*sizeof(array));

    tree = ajTreeFollow(NULL, thys);

    while(tree)
    {
	if(tree->Data)
	    (*array)[i++] = (AjPStr) tree->Data;

	tree = ajTreeFollow(tree, thys);
    }

    (*array)[n] = NULL;

    return n;
}




/* @func ajTreeDummyFunction **************************************************
**
** Dummy function to catch all unused functions defined in ajtree
**
** @return [void]
******************************************************************************/

void ajTreeDummyFunction(void)
{
    return;
}




/* @func ajTreeAddData ********************************************************
**
** Sets the data value in a terminal tree node.
**
** @param [u] thys [AjPTree] Terminal tree node. Must have no descendants
** @param [r] name [const AjPStr] Leaf name
** @param [o] data [void*] Data value
** @return [AjBool] ajTrue on success
******************************************************************************/

AjBool ajTreeAddData(AjPTree thys, const AjPStr name, void* data)
{
/*
    if (thys->Down)
    {
	ajErr("tried to define data value for non-terminal tree node");
	return ajFalse;
    }
*/

    if (thys->Data)
    {
	ajErr("tried to define data value for node with data");

	return ajFalse;
    }

    thys->Name = ajStrNewS(name);
    thys->Data = data;

    return ajTrue;
}




/* @func ajTreestrAddData *****************************************************
**
** Sets the data value in a terminal string tree node.
**
** @param [u] thys [AjPTree] Terminal tree node. Must have no descendants
** @param [r] name [const AjPStr] Leaf name
** @param [o] data [AjPStr] Data value
** @return [AjBool] ajTrue on success
******************************************************************************/

AjBool ajTreestrAddData(AjPTree thys, const AjPStr name, AjPStr data)
{
    if (thys->Down)
    {
	ajErr("tried to define string data value for non-terminal tree node");

	return ajFalse;
    }

    if (thys->Data)
    {
	ajErr("tried to define string data value for tree node with data");

	return ajFalse;
    }

    thys->Name = ajStrNewS(name);
    thys->Data = (void*) data;

    return ajTrue;
}




/* @func ajTreeAddNode ********************************************************
**
** Creates a new child node of the parent
**
** @param [u] thys [AjPTree] Parent tree node
** @return [AjPTree] New tree node created. It has no data value so far,
**                   so ajTreeAddNode or ajTreeAddData should be called to
**                   define data or further nodes.
******************************************************************************/

AjPTree ajTreeAddNode(AjPTree thys)
{
    AjPTree ret;

    if(thys->Data)
    {
	ajErr("tried to add child to node with data value");

	return NULL;
    }

    AJNEW0(ret);
    ret->Up = thys;
    thys->Down = ret;

    ret->Type = thys->Type;

    return ret;
}




/* @func ajTreeAddSubNode *****************************************************
**
** Creates a new sibling node of the parent
**
** @param [u] thys [AjPTree] Left tree node
** @return [AjPTree] New tree node created. It has no data value so far,
**                   so ajTreeAddNode or ajTreeAddData should be called to
**                  define data or further nodes.
******************************************************************************/

AjPTree ajTreeAddSubNode(AjPTree thys)
{
    AjPTree ret;

    AJNEW0(ret);
    ret->Up = thys->Up;
    ret->Left = thys;
    thys->Right = ret;

    ret->Type = thys->Type;

    return ret;
}




/* @func ajTreeToNewick *******************************************************
**
** Prints a trace of a tree to debug output
**
** @param [r] thys [const AjPTree] Tree object
** @param [w] Pnewick [AjPStr*] Newick format string
** @return [void]
** @@
******************************************************************************/

void ajTreeToNewick(const AjPTree thys, AjPStr* Pnewick)
{
    AjPTree tree;
    const AjPTree root;
    AjPTree nexttree;

    ajStrAssignClear(Pnewick);


    root = thys;
    tree = ajTreeDown(thys);

    if(!tree)
	return;

    ajStrAppendC(Pnewick, "(");

    while(tree && tree != root)
    {
	ajDebug("node %8p '%S'\n", tree, tree->Name);

	if(ajStrGetLen(tree->Name))
	{
	    ajStrAppendS(Pnewick, tree->Name);

	    if(tree->Data)
		ajFmtPrintAppS(Pnewick, ":%.2f", *(double*)tree->Data);
	}

	nexttree = ajTreeDown(tree);

	if(nexttree)
	{
	    tree = nexttree;
	    ajStrAppendC(Pnewick, "(");
	    continue;
	}

	nexttree = ajTreeNext(tree);

	if(nexttree)
	{
	    tree = nexttree;
	    ajStrAppendC(Pnewick, ",");
	    continue;
	}

	/* no way down or right. Go up until we can go right */

	nexttree = ajTreeUp(tree);
	tree = nexttree;
	ajStrAppendC(Pnewick, ")");

	while(tree && tree != root)
	{
	    /* we just went up ... check for a length */

	    ajDebug(" up node %8p '%S'\n", tree, tree->Name);

	    if(tree->Data)
		ajFmtPrintAppS(Pnewick, ":%.2f", *(double*)tree->Data);


	    nexttree = ajTreeNext(tree);

	    if(nexttree)
	    {
		tree = nexttree;
		ajStrAppendC(Pnewick, ",");
		ajDebug(" next node %8p '%S'\n", tree, tree->Name);
		break;
	    }

	    nexttree = ajTreeUp(tree);
	    tree = nexttree;
	    ajStrAppendC(Pnewick, ")");
	}
    }

    if(root->Data)
	ajFmtPrintAppS(Pnewick, ":%.2f", *(double*)root->Data);

    ajStrAppendK(Pnewick, ';');

    ajDebug("ajTreeToNewick '%S'\n", *Pnewick);

    return;
}




/* @func ajTreeTrace *********************************************************
**
** Prints a trace of a tree to debug output
**
** @param [r] thys [const AjPTree] Tree object
** @return [void]
** @@
******************************************************************************/

void ajTreeTrace(const AjPTree thys)
{
    AjPTree tree;
    ajint inode=0;

    ajStrAssignClear(&treeFollowPath);
    treeTraceNode(thys, inode++);

    tree = ajTreeFollow(NULL, thys);

    while(tree)
    {
	treeTraceNode(tree, inode++);
	ajStrAssignClear(&treeFollowPath);
	tree = ajTreeFollow(tree, thys);
    }

    return;
}




/* @func ajTreestrTrace *******************************************************
**
** Prints a trace of a string tree to debug output
**
** @param [r] thys [const AjPTree] Tree object
** @return [void]
** @@
******************************************************************************/

void ajTreestrTrace(const AjPTree thys)
{
    AjPTree tree;
    ajint inode=0;

    ajStrAssignClear(&treeFollowPath);

    treestrTraceNode(thys, inode++);
    tree = ajTreeFollow(NULL, thys);

    while(tree)
    {
	treestrTraceNode(tree, inode++);
	tree = ajTreeFollow(tree, thys);
    }

    return;
}




/* @funcstatic treeTraceNode **************************************************
**
** Write information about a tee to debug output
**
** @param [r] thys [const AjPTree] Tree node
** @param [r] num [ajint] Node number
** @return [void]
******************************************************************************/

static void treeTraceNode (const AjPTree thys, ajint num)
{
    ajint ileft = 0;
    ajint iright = 0;
    ajint iup = 0;
    ajint idown = 0;
    const AjPTree p;

    ajDebug("node %d:%x '%S' '%S'\n",
	    num, thys, thys->Name, treeFollowPath);

    for (p=thys;p->Up; p=p->Up)
	iup++;

    for (p=thys;p->Down; p=p->Down)
	idown++;

    for (p=thys;p->Left; p=p->Left)
	ileft++;

    for (p=thys;p->Right; p=p->Right)
	ileft++;

    ajDebug("  Up: %8x %d\n", thys->Up, iup);
    ajDebug("  Down: %8x %d\n", thys->Down, idown);
    ajDebug("  Right: %8x %d\n", thys->Right, iright);
    ajDebug("  Left: %8x %d\n", thys->Left, ileft);

    if (thys->Data)
    {
	if (thys->Down)
	    ajDebug("    (Down link %p and data %x %f)\n",
		    thys->Down, thys->Data,  *(double *)thys->Data);
	else
	    ajDebug(" End node data: %x %f\n",
		    thys->Data,  *(double *)thys->Data);
    }
    else if (!thys->Down)
    {
	ajDebug(" No data, no down link: ** broken node %x **\n", thys);
    }

    return;
}




/* @funcstatic treestrTraceNode  **********************************************
**
** Write information about a tree node to debug output
**
** @param [r] thys [const AjPTree] Tree node
** @param [r] num [ajint] Node number
** @return [void]
******************************************************************************/

static void treestrTraceNode (const AjPTree thys, ajint num)
{
    ajint ileft = 0;
    ajint iright = 0;
    ajint iup = 0;
    ajint idown = 0;
    const AjPTree p;

    ajDebug("node %d:%x '%S' '%S'",
	    num, thys, thys->Name, treeFollowPath);

    if (thys->Data)
    {
	if (thys->Down)
	    ajDebug(" Data and down link: ** broken node %x **\n", thys);
	else
	    ajDebug(" End node string: '%S'\n", (AjPStr) thys->Data);
    }
    else if (!thys->Down)
    {
	ajDebug(" No data, no down link: ** broken node %x **\n", thys);
    }

    for (p=thys;p->Up; p=p->Up)
	iup++;

    for (p=thys;p->Down; p=p->Down)
	idown++;

    for (p=thys;p->Left; p=p->Left)
	ileft++;

    for (p=thys;p->Right; p=p->Right)
	iright++;

    ajDebug("  Up: %x %d\n", thys->Up, iup);
    ajDebug("  Down: %x %d\n", thys->Down, idown);
    ajDebug("  Right: %x %d\n", thys->Right, iright);
    ajDebug("  Left: %x %d\n", thys->Left, ileft);

    if (thys->Data)
    {
	if (thys->Down)
	    ajDebug(" Data and down link: ** broken node %x **\n", thys);
	else
	    ajDebug(" End node string: '%S'\n", (AjPStr) thys->Data);
    }
    else if (!thys->Down)
	ajDebug(" No data, no down link: ** broken node %x **\n", thys);

    return;
}




/* @func ajTreeFollow *****************************************************
**
** Returns the next node through the tree.
**
** Step down to children, and follow siblings to the right.
**
** When done at any level, go up and right.
**
** Stop when back up at the original parent node.
**
** @param [r] thys [const AjPTree] Currentree node
** @param [r] parent [const AjPTree] Parent tree node
** @return [AjPTree] New tree node created. It has no data value so far,
**                   so ajTreeAddNode or ajTreeAddData should be called to
**                  define data or further nodes.
******************************************************************************/

AjPTree ajTreeFollow(const AjPTree thys, const AjPTree parent)
{
    AjPTree ret;
    AjPTree tree;

    if(!thys)
	thys = parent;

    ret = ajTreeDown(thys);

    if(ret)
    {
	ajStrAppendC(&treeFollowPath, "(");

	return ret;
    }

    ret = ajTreeNext(thys);

    if(ret)
    {
	ajStrAppendC(&treeFollowPath, ",");

	return ret;
    }

    ret = ajTreeUp(thys);
    ajStrAppendC(&treeFollowPath, ")");

    while(ret != parent)
    {
	tree = ret;
	ret = ajTreeNext(tree);

	if(ret)
	{
	    ajStrAppendC(&treeFollowPath, ",");

	    return ret;
	}

	ret = ajTreeUp(tree);
	ajStrAppendC(&treeFollowPath, ")");
    }

    ajStrAppendC(&treeFollowPath, ";");

    return NULL;
}




/* @func ajTreeNext *****************************************************
**
** Returns the next node at the same level
**
** @param [r] thys [const AjPTree] Parent tree node
** @return [AjPTree] New tree node created. It has no data value so far,
**                   so ajTreeAddNode or ajTreeAddData should be called to
**                  define data or further nodes.
******************************************************************************/

AjPTree ajTreeNext(const AjPTree thys)
{
    return thys->Right;
}




/* @func ajTreePrev *****************************************************
**
** Returns the previous node at the same level
**
** @param [r] thys [const AjPTree] Parent tree node
** @return [AjPTree] New tree node created. It has no data value so far,
**                   so ajTreeAddNode or ajTreeAddData should be called to
**                  define data or further nodes.
******************************************************************************/

AjPTree ajTreePrev(const AjPTree thys)
{
    return thys->Left;
}




/* @func ajTreeDown *****************************************************
**
** Returns the next node down 1 level
**
** @param [r] thys [const AjPTree] Parent tree node
** @return [AjPTree] New tree node created. It has no data value so far,
**                   so ajTreeAddNode or ajTreeAddData should be called to
**                  define data or further nodes.
******************************************************************************/

AjPTree ajTreeDown(const AjPTree thys)
{
    return thys->Down;
}




/* @func ajTreeUp *****************************************************
**
** Returns the parent of the present node
**
** @param [r] thys [const AjPTree] Child tree node
** @return [AjPTree] New tree node created. It has no data value so far,
**                   so ajTreeAddNode or ajTreeAddData should be called to
**                  define data or further nodes.
******************************************************************************/

AjPTree ajTreeUp(const AjPTree thys)
{
    return thys->Up;
}




/* @func ajTreeExit ***********************************************************
**
** Prints a summary of tree usage with debug calls
**
** @return [void]
** @@
******************************************************************************/

void ajTreeExit(void)
{
    ajStrDel(&treeFollowPath);

    ajDebug("Tree usage : %d opened, %d closed, %d maxsize %d nodes\n",
	    treeNewCnt, treeDelCnt, treeMaxNum, treeNodeCnt);
    ajDebug("Tree iterator usage : %d opened, %d closed, %d maxsize\n",
	    treeIterNewCnt, treeIterDelCnt);

    return;
}
