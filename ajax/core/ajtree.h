/* @include ajtree ************************************************************
**
** AJAX tree functions
**
** These functions create and control trees.
**
** @author Copyright (C) 2004 Peter Rice
** @version $Revision: 1.15 $
** @modified $Date: 2011/10/19 14:52:22 $ by $Author: rice $
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

#ifndef AJTREE_H
#define AJTREE_H

/* ========================================================================= */
/* ============================= include files ============================= */
/* ========================================================================= */

#include "ajdefine.h"
#include "ajstr.h"

AJ_BEGIN_DECLS




/* ========================================================================= */
/* =============================== constants =============================== */
/* ========================================================================= */




/* ========================================================================= */
/* ============================== public data ============================== */
/* ========================================================================= */




/* @enum AjETreeType **********************************************************
**
** Tree type
**
** @value ajETreeAny Any
** @value ajETreeStr String data
** @value ajETreeSpecial Other special data
******************************************************************************/

typedef enum AjOTreeType
{
    ajETreeAny,
    ajETreeStr,
    ajETreeSpecial
} AjETreeType;

#define AjPTreeNode AjOTreeNode*




/* @data AjPTree **************************************************************
**
** Tree data object. Trees are simple linked nodes with back pointers.
**
** Trees can hold any data type. Special functions are available for trees
** of AjPStr values. In general, these functions are the same. Many are
** provided for ease of use to save remembering which calls need special cases.
**
** At the top level, a tree has a list of named nodes and a pointer to the
** top node in the tree.
**
** @new ajTreeNew Creates a new general tree.
** @new ajTreestrNew Creates a new AjPStr tree.
**
** @attr Right [struct AjSTree*] Next tree node
** @attr Left  [struct AjSTree*] Previous tree node
** @attr Up    [struct AjSTree*] Parent tree node
** @attr Down  [struct AjSTree*] First child tree node
** @attr Name [AjPStr] Name string
** @attr Data [void*] Data value
** @attr Freedata [AjBool function] Complex data free function
** @attr Type [AjEnum] Tree type (any, string, etc.)
** @attr Padding [char[4]] Padding to alignment boundary
** @@
******************************************************************************/

typedef struct AjSTree
{
    struct AjSTree* Right;
    struct AjSTree* Left;
    struct AjSTree* Up;
    struct AjSTree* Down;
    AjPStr Name;
    void* Data;
    AjBool (*Freedata)(void** data);
    AjEnum Type;
    char Padding[4];
} AjOTree;

#define AjPTree AjOTree*




/* ========================================================================= */
/* =========================== public functions ============================ */
/* ========================================================================= */




/*
** Prototype definitions
*/

AjBool  ajTreeAddData(AjPTree thys, const AjPStr name, void* data);
AjPTree ajTreeAddNode(AjPTree thys);
AjPTree ajTreeAddSubNode(AjPTree thys);
AjPTree ajTreeCopy(const AjPTree thys);

void    ajTreeDel(AjPTree* pthis);
void    ajTreeExit(void);
void    ajTreeFree(AjPTree* pthis);
AjPTree ajTreeFollow(const AjPTree thys, const AjPTree parent);
ajuint  ajTreeLength(const AjPTree thys);
void    ajTreeMap(AjPTree thys, void apply(void** x, void* cl), void* cl);
AjPTree ajTreeNew(void);
AjPTree ajTreeNewNewick(const AjPStr newick);
AjBool  ajTreestrAddData(AjPTree thys, const AjPStr name, AjPStr data);
AjPTree ajTreestrCopy(const AjPTree thys);
void    ajTreestrDel(AjPTree* pthis);
void    ajTreestrFree(AjPTree* pthis);
ajuint  ajTreestrLength(const AjPTree thys);
void    ajTreestrMap(AjPTree thys, void apply(AjPStr* x, void* cl), void* cl);
AjPTree ajTreestrNew(void);
ajuint  ajTreestrToArray(const AjPTree thys, AjPStr** array);
ajuint  ajTreeToArray(const AjPTree thys, void*** array);
void    ajTreeToNewick(const AjPTree thys, AjPStr* Pnewick);
void    ajTreeTrace(const AjPTree thys);
void    ajTreestrTrace(const AjPTree thys);

AjPTree ajTreeDown(const AjPTree thys);
AjPTree ajTreeNext(const AjPTree thys);
AjPTree ajTreePrev(const AjPTree thys);
AjPTree ajTreeUp(const AjPTree thys);

void    ajTreeDummyFunction(void);

/*
** End of prototype definitions
*/




AJ_END_DECLS

#endif /* !AJTREE_H */
