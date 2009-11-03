#ifdef __cplusplus
extern "C"
{
#endif

#ifndef ajtree_h
#define ajtree_h

#include "ajdefine.h"
#include "ajlist.h"

enum AjETreeType {ajETreeAny, ajETreeStr, ajETreeSpecial};

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
** @attr Freedata [(AjBool*)] Complex data free function
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

#endif

#ifdef __cplusplus
}
#endif
