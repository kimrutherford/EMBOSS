/* @source ajdom **************************************************************
**
** AJAX Document Object Model (DOM) functions
**
** @author Copyright (C) 2006 Alan Bleasby
** @version $Revision: 1.35 $
** @modified Jul 03 2006 ajb First version
** @modified $Date: 2012/03/22 21:19:36 $ by $Author: mks $
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

/* ========================================================================= */
/* ============================= include files ============================= */
/* ========================================================================= */

#include "ajlib.h"
#include "ajdom.h"
#include "ajfileio.h"
#include "expat.h"

#include <string.h>




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

/* @conststatic domKCmtx ******************************************************
**
** AJAX DOM cmtx
**
******************************************************************************/

static const ajuint domKCmtx[] =
{
    0x00dd, /* element    */
    0x0014, /* attribute  */
    0x0000, /* text       */
    0x0000, /* cdata      */
    0x00dd, /* entity ref */
    0x00dd, /* entity     */
    0x0000, /* proc inst  */
    0x0000, /* comment    */
    0x02c1, /* doc        */
    0x0820, /* doc type   */
    0x00dd, /* doc frag   */
    0x0000  /* notation   */
};




/* @conststatic domKNodeinfo **************************************************
**
** AJAX DOM Node Type names
**
******************************************************************************/

static const char *domKNodeinfo[] =
{
    "ajEDomNodeTypeNULL",
    "ajEDomNodeTypeElement",
    "ajEDomNodeTypeAttribute",
    "ajEDomNodeTypeText",
    "ajEDomNodeTypeCdataSection",
    "ajEDomNodeTypeEntityReference",
    "ajEDomNodeTypeEntityNode",
    "ajEDomNodeTypeProcessingInstruction",
    "ajEDomNodeTypeComment",
    "ajEDomNodeTypeDocumentNode",
    "ajEDomNodeTypeDocumentType",
    "ajEDomNodeTypeDocumentFragment",
    "ajEDomNodeTypeNotation",
    (const char *) NULL
};




#define AJDOM_TABLE_HINT 1000

#define AJXML_BUFSIZ 8192

#define AJDOM_NOCHILD(p,q) !(domKCmtx[(p)->type - 1] & (1 << ((q)->type - 1)))
#define AJDOM_DOCMOD(p,q) ((p)->type == ajEDomNodeTypeDocumentNode &&   \
                           (q)->type == ajEDomNodeTypeElement)
#define AJDOM_DOCTYPEMOD(p,q) ((p)->type == ajEDomNodeTypeDocumentNode && \
                               (q)->type == ajEDomNodeTypeDocumentType)
#define AJDOM_CANTDO(p,q) (AJDOM_NOCHILD(p,q) ||                \
                           (AJDOM_DOCMOD(p,q) &&                \
                            (p)->sub.Document.documentelement))




/* ========================================================================= */
/* =========================== private variables =========================== */
/* ========================================================================= */




/* ========================================================================= */
/* =========================== private functions =========================== */
/* ========================================================================= */

static AjBool           domIsAncestor(const AjPDomNode node,
                                      const AjPDomNode parent);

static AjPDomNodeEntry  domDoLookupNode(const AjPDomNodeList list,
                                        const AjPDomNode node);
static void             domUpdateNode(AjPDomNode node);
static AjPDomNode       domDoRemoveChild(AjPDomNode node, AjPDomNode child);
static void             domRemoveFromMap(AjPDomNodeList list,
                                         const AjPDomNode key);
static void             domAddToMap(AjPDomNodeList list, AjPDomNode key,
                                    AjPDomNodeEntry val);


static AjPDomNode       domNodeListItemFiltered(const AjPDomNodeList list,
                                                ajint indx, ajuint nodetype);

static void             domTraverse(AjPDomNodeList list, AjPDomNode node,
                                    const AjPStr tagname);
static void             domTraverseC(AjPDomNodeList list, AjPDomNode node,
                                     const char *tagname);

static AjPDomNode       domNodeCloneNode(AjPDomDocument ownerdocument,
                                         const AjPDomNode node, AjBool deep);


static void             domWriteEncoded(const AjPStr s, AjPFile outf);

static AjPDomUserdata   domUserdataNew(void);
static void domUserdataDel(AjPDomUserdata *thys);

static void domExpatStart(void *udata, const XML_Char *name,
                          const XML_Char **atts);
static void domExpatEnd(void *udata, const XML_Char *name);
static void domExpatChardata(void *udata, const XML_Char *str,
                             int len);
static void domExpatCdataStart(void *udata);
static void domExpatCdataEnd(void *udata);
static void domExpatComment(void *udata, const XML_Char *str);
static void domExpatXmlDecl(void *udata, const XML_Char *version,
                            const XML_Char *encoding, int standalone);
static void domExpatDoctypeStart(void *udata, const XML_Char *doctypename,
                                 const XML_Char *sysid, const XML_Char *pubid,
                                 int hasinternalsubset);
static void domExpatDoctypeEnd(void *udata);
static void domExpatElement(void *udata, const XML_Char *name,
                            XML_Content *model);
static void domExpatAttlist(void *udata, const XML_Char *name,
                            const XML_Char *attname, const XML_Char *atttype,
                            const XML_Char *deflt, int isrequired);
static void domExpatEntity(void *udata,
                           const XML_Char *entityname, int isparam,
                           const XML_Char *value, int lenval,
                           const XML_Char *base, const XML_Char *systemid,
                           const XML_Char *publicid, const XML_Char *notname);
static void domExpatNotation(void *udata, const XML_Char *notname,
                             const XML_Char *base, const XML_Char *systemid,
                             const XML_Char *publicid);
static void domClearMapValues(void **key, void **value, void *cl);
static void domClearMapAll(void **key, void **value, void *cl);




/* ========================================================================= */
/* ======================= All functions by section ======================== */
/* ========================================================================= */




/* @funcstatic domRemoveFromMap ***********************************************
**
** Remove a key/value pair from a DOM node list
**
** @param [w] list [AjPDomNodeList] node list
** @param [r] key [const AjPDomNode] key
** @return [void]
**
** @release 4.0.0
** @@
******************************************************************************/

static void domRemoveFromMap(AjPDomNodeList list, const AjPDomNode key)
{
    AjPDomNode trukey;

    if(!list->table)
        return;

    /*
    ** don't free the values, just remove them from the map
    ** if(val)
    */

    ajTableRemoveKey(list->table, key, (void **) &trukey);

    return;
}




/* @funcstatic domAddToMap ****************************************************
**
** Add a key/value pair to a DOM node list
**
** @param [w] list [AjPDomNodeList] node list
** @param [o] key [AjPDomNode] key
** @param [u] val [AjPDomNodeEntry] value
** @return [void]
**
** @release 4.0.0
** @@
******************************************************************************/

static void domAddToMap(AjPDomNodeList list, AjPDomNode key,
                        AjPDomNodeEntry val)
{
    if(!list->table)
        list->table = ajTableNew(AJDOM_TABLE_HINT);


    domRemoveFromMap(list, key);

    ajTablePut(list->table, key, val);

    return;
}




/* @func ajDomNodeListGetLen **************************************************
**
** Return the length of a DOM node list
**
** @param [r] list [const AjPDomNodeList] node list
** @return [ajint] Length (-1 if error)
**
** @release 6.3.0
** @@
******************************************************************************/

ajint ajDomNodeListGetLen(const AjPDomNodeList list)
{

    if(!list)
        return -1;

    return list->length;
}




/* @func ajDomNodeListAppend **************************************************
**
** Append a child node to a DOM node list
**
** @param [w] list [AjPDomNodeList] node list
** @param [u] child [AjPDomNode] child
** @return [AjPDomNodeEntry] Node list child entry created
**
** @release 4.0.0
** @@
******************************************************************************/

AjPDomNodeEntry ajDomNodeListAppend(AjPDomNodeList list,
                                    AjPDomNode child)
{
    AjPDomNodeEntry p;
    AjPDomDocumentType doctype;

    if(!list)
    {
        ajDebug("ajDomNodeListAppend: Null pointer error\n");
        return NULL;
    }

    if(list->filter)
    {
        ajDebug("ajDomNodeListAppend: Filtered list error\n");
        return NULL;
    }

    AJNEW0(p);

    domAddToMap(list, child, p);

    p->node = child;

    if(!list->first)
    {
        list->first = p;
        list->last  = p;
    }
    else
    {
        list->last->next = p;
        p->prev = list->last;
        list->last = p;
    }

    ++list->length;

    if(child->ownerdocument &&
       (doctype = child->ownerdocument->sub.Document.doctype) &&
       list == doctype->childnodes)
    {
        if(child->type == ajEDomNodeTypeNotation)
            ++doctype->sub.DocumentType.notations->length;
        else if(child->type == ajEDomNodeTypeEntityNode)
            ++doctype->sub.DocumentType.entities->length;
    }

    if(child->type == ajEDomNodeTypeAttribute)
        child->sub.Attr.ownerelement = list->ownerelement;

    return p;
}




/* @func ajDomNodeAppendChild *************************************************
**
** Appends an extra child at the end of the child nodes list of a node.
** If extrachild is already in the list, it is first removed.
**
** @param [w] node [AjPDomNode] node
** @param [u] extrachild [AjPDomNode] extra child
** @return [AjPDomNode] Pointer to extrachild or NULL if error
**
** @release 4.0.0
** @@
******************************************************************************/

AjPDomNode ajDomNodeAppendChild(AjPDomNode node, AjPDomNode extrachild)
{
    AjPDomNode n    = NULL;
    AjPDomNode next = NULL;

    if(!node || !extrachild)
        return NULL;

    if(extrachild->ownerdocument != node->ownerdocument &&
       node->type != ajEDomNodeTypeDocumentNode &&
       extrachild->type != ajEDomNodeTypeDocumentType)
    {
        ajDebug("ajDomNodeAppendChild: Wrong document");
        return NULL;
    }

    if(extrachild->type == ajEDomNodeTypeDocumentFragment)
    {
        for(n = extrachild->firstchild; n; n = n->nextsibling)
            if(AJDOM_CANTDO(node, n) || domIsAncestor(n, node))
            {
                ajDebug("ajDomNodeAppendChild: Hierarchy Request Error\n");
                return NULL;
            }

        for(n = extrachild->firstchild; n; n = next)
        {
            next = n->nextsibling;

            if(!ajDomRemoveChild(extrachild, n))
                return NULL;

            if(!ajDomNodeAppendChild(node, n))
            {
                ajDomDocumentDestroyNode(n->ownerdocument, n);
                return NULL;
            }
        }

        return extrachild;
    }


    if(AJDOM_CANTDO(node, extrachild) || domIsAncestor(extrachild, node))
    {
        ajDebug("ajDomNodeAppendChild: Hierarchy Request Error 2\n");
        return NULL;
    }

    domDoRemoveChild(node, extrachild);

    if(!ajDomNodeListAppend(node->childnodes, extrachild))
        return NULL;

    if(!node->firstchild)
    {
        node->firstchild = extrachild;
        node->lastchild  = extrachild;
        extrachild->previoussibling = NULL;
        extrachild->nextsibling = NULL;
    }
    else
    {
        node->lastchild->nextsibling = extrachild;
        extrachild->previoussibling = node->lastchild;
        node->lastchild = extrachild;
    }

    extrachild->nextsibling = NULL;
    extrachild->parentnode = node;

    if(AJDOM_DOCMOD(node, extrachild))
        node->sub.Document.documentelement = extrachild;
    else if(AJDOM_DOCTYPEMOD(node, extrachild))
    {
        node->sub.Document.doctype = extrachild;
        extrachild->ownerdocument = node;
    }

    domUpdateNode(node);

    return extrachild;
}




/* @funcstatic domUpdateNode **************************************************
**
** Update the common parent entry of a node
**
** @param [w] node [AjPDomNode] node
** @return [void]
**
** @release 4.0.0
** @@
******************************************************************************/

static void domUpdateNode(AjPDomNode node)
{
    AjPDomNode p = NULL;
    AjPDomNode n = NULL;

    if(!node || !node->ownerdocument)
        return;

    if(!node->ownerdocument->sub.Document.commonparent)
    {
        node->ownerdocument->sub.Document.commonparent = node;
        return;
    }

    p = NULL;

    for(n = node; n; n = n->parentnode)
        if(n == node->ownerdocument->sub.Document.commonparent)
            return;
        else if(!p && n->subtreeModified == 1)
            p = n;
        else
            n->subtreeModified = 1;

    node->ownerdocument->sub.Document.commonparent = p;

    return;
}




/* @funcstatic domIsAncestor **************************************************
**
** Tests whether a putative parent is the parent of a child
**
** @param [r] node [const AjPDomNode] node
** @param [r] parent [const AjPDomNode] putative parent
** @return [AjBool] true if ancestor found
**
** @release 4.0.0
** @@
******************************************************************************/

static AjBool domIsAncestor(const AjPDomNode node, const AjPDomNode parent)
{
    const AjPDomNode p = NULL;

    for(p = parent; p; p = p->parentnode)
        if(p == node)
            return ajTrue;

    return ajFalse;
}




/* @func ajDomRemoveChild *****************************************************
**
** Removes a child node from a list of children
**
** @param [w] node [AjPDomNode] node
** @param [u] child [AjPDomNode] child to remove
** @return [AjPDomNode] child removed
**
** @release 4.0.0
** @@
******************************************************************************/

AjPDomNode ajDomRemoveChild(AjPDomNode node, AjPDomNode child)
{
    if(!node || !child)
        return NULL;

    if(child->ownerdocument != node->ownerdocument &&
       child->ownerdocument != node)
    {
        ajDebug("ajDomRemoveChild: Wrong document");

        return NULL;
    }

    child = domDoRemoveChild(node, child);

    return child;
}




/* @func ajDomNodeListExists **************************************************
**
** Check whether a child exists in a node list
**
** @param [u] list [AjPDomNodeList] list
** @param [r] child [const AjPDomNode] child
** @return [AjBool] true if child is in the list
**
** @release 4.0.0
** @@
******************************************************************************/

AjBool ajDomNodeListExists(AjPDomNodeList list, const AjPDomNode child)
{
    AjPDomNodeEntry e = NULL;

    if(!list || list->filter)
        return ajFalse;

    e = domDoLookupNode(list, child);

    if(e)
        return ajTrue;

    return ajFalse;
}




/* @funcstatic domDoLookupNode ************************************************
**
** Return a pointer to a given node of a node list
**
** @param [r] list [const AjPDomNodeList] list
** @param [r] node [const AjPDomNode] node
** @return [AjPDomNodeEntry] node entry or NULL if not found
**
** @release 4.0.0
** @@
******************************************************************************/

static AjPDomNodeEntry domDoLookupNode(const AjPDomNodeList list,
                                       const AjPDomNode node)
{
    AjPDomNodeEntry p;

    p = (AjPDomNodeEntry) ajTableFetchmodV(list->table, node);

    return p;
}




/* @func ajDomNodeListRemove **************************************************
**
** Remove a child from a node list
**
** @param [w] list [AjPDomNodeList] list
** @param [u] child [AjPDomNode] child
** @return [AjPDomNodeEntry] child removed or NULL if not found
**
** @release 4.0.0
** @@
******************************************************************************/

AjPDomNodeEntry ajDomNodeListRemove(AjPDomNodeList list, AjPDomNode child)
{
    AjPDomNodeEntry e;
    AjPDomNode trukey;

    if(!list)
    {
        ajDebug("ajDomNodeListRemove: Empty list");
        return NULL;
    }

    if(list->filter)
    {
        ajDebug("ajDomNodeListRemove: Filtered list error");
        return NULL;
    }

    e = domDoLookupNode(list, child);

    if(!e)
        return NULL;

    /*
    ** Don't delete the key/value - just remove them from the lookup
    ** table. The value is still required
    */

    ajTableRemoveKey(list->table, child, (void **) &trukey);

    if(list->first == list->last)
    {
        list->first = NULL;
        list->last  = NULL;
    }
    else if(e == list->first)
    {
        list->first = e->next;
        list->first->prev = NULL;
    }
    else if(e == list->last)
    {
        list->last = e->prev;
        list->last->next = NULL;
    }
    else
    {
        e->prev->next = e->next;
        e->next->prev = e->prev;
    }

    --list->length;

    if(child->type == ajEDomNodeTypeAttribute)
        child->sub.Attr.ownerelement = NULL;

    return e;
}




/* @funcstatic domDoRemoveChild ***********************************************
**
** Low level removal of a child node from a list of children
**
** @param [w] node [AjPDomNode] node
** @param [u] child [AjPDomNode] child to remove
** @return [AjPDomNode] child removed
**
** @release 4.0.0
** @@
******************************************************************************/

static AjPDomNode domDoRemoveChild(AjPDomNode node, AjPDomNode child)
{
    AjPDomNodeEntry e = NULL;

    if(!ajDomNodeListExists(node->childnodes, child))
        return NULL;

    e = ajDomNodeListRemove(node->childnodes, child);
    free(e);

    if(node->firstchild == node->lastchild)
    {
        node->firstchild = NULL;
        node->lastchild = NULL;
    }
    else if(child == node->firstchild)
    {
        node->firstchild = child->nextsibling;
        node->firstchild->previoussibling = NULL;
    }
    else if(child == node->lastchild)
    {
        node->lastchild = child->previoussibling;
        node->lastchild->nextsibling = NULL;
    }
    else
    {
        child->previoussibling->nextsibling = child->nextsibling;
        child->nextsibling->previoussibling = child->previoussibling;
    }

    child->previoussibling = NULL;
    child->nextsibling = NULL;
    child->parentnode = NULL;

    if(AJDOM_DOCMOD(node, child))
        node->sub.Document.documentelement = NULL;
    else if(AJDOM_DOCTYPEMOD(node, child))
    {
        node->sub.Document.doctype = NULL;
        child->ownerdocument = NULL;
    }
    else
        domUpdateNode(node);

    return child;
}




/* @func ajDomDocumentDestroyNode *********************************************
**
** Frees a node and its children
**
** @param [w] doc [AjPDomDocument] document
** @param [w] node [AjPDomNode] node
** @return [void]
**
** @release 4.0.0
** @@
******************************************************************************/

void ajDomDocumentDestroyNode(AjPDomDocument doc, AjPDomNode node)
{

    if(!node)
        return;

    if(node->childnodes)
        ajDomDocumentDestroyNodeList(doc, node->childnodes, AJDOMDESTROY);

    switch(node->type)
    {
        case ajEDomNodeTypeElement:
            ajDomDocumentDestroyNodeList(doc, node->attributes, AJDOMDESTROY);
            ajStrDel(&node->name);
            break;
        case ajEDomNodeTypeText:
            ajStrDel(&node->name);
            ajStrDel(&node->value);
            break;
        case ajEDomNodeTypeComment:
            ajStrDel(&node->name);
            ajStrDel(&node->value);
            break;
        case ajEDomNodeTypeCdataSection:
            ajStrDel(&node->name);
            ajStrDel(&node->value);
            break;
        case ajEDomNodeTypeAttribute:
            ajStrDel(&node->name);
            ajStrDel(&node->value);
            break;
        case ajEDomNodeTypeEntityReference:
        case ajEDomNodeTypeEntityNode:
            ajStrDel(&node->name);
            ajStrDel(&node->value);
            ajStrDel(&node->sub.Entity.publicid);
            ajStrDel(&node->sub.Entity.systemid);
            ajStrDel(&node->sub.Entity.notationname);
            break;
        case ajEDomNodeTypeProcessingInstruction:
            ajStrDel(&node->name);
            ajStrDel(&node->value);
            break;
        case ajEDomNodeTypeDocumentNode:
            ajStrDel(&node->name);
            ajStrDel(&node->sub.Document.version);
            ajStrDel(&node->sub.Document.encoding);
            break;
        case ajEDomNodeTypeDocumentType:
            ajDomDocumentDestroyNodeList(doc, node->sub.DocumentType.entities,
                                         AJDOMDESTROY);
            ajDomDocumentDestroyNodeList(doc, node->sub.DocumentType.notations,
                                         AJDOMDESTROY);
            ajStrDel(&node->sub.DocumentType.publicid);
            ajStrDel(&node->sub.DocumentType.systemid);
            ajStrDel(&node->name);
            break;
        case ajEDomNodeTypeNotation:
            ajStrDel(&node->sub.Notation.publicid);
            ajStrDel(&node->sub.Notation.systemid);
            ajStrDel(&node->name);
            break;
        default:
            ajDebug("ajDomDocumentDestroyNode got unexpected "
                    "AJAX DOM Node type %d.\n", node->type);
    }

    AJFREE(node);

    return;
}




/* @func ajDomDocumentDestroyNodeList *****************************************
**
** Frees a node list
**
** @param [w] doc [AjPDomDocument] document
** @param [w] list [AjPDomNodeList] list
** @param [r] donodes [AjBool] free nodes as well if true
** @return [void]
**
** @release 4.0.0
** @@
******************************************************************************/

void ajDomDocumentDestroyNodeList(AjPDomDocument doc, AjPDomNodeList list,
                                  AjBool donodes)
{
    AjPDomNodeEntry entry = NULL;
    AjPDomNodeEntry tmp   = NULL;

    if(list)
    {
        if(list->filter)
        {
            entry = list->first;

            while(entry)
            {
                if(donodes)
                    ajDomDocumentDestroyNode(doc, entry->node);

                tmp = entry;
                entry = entry->next;
                AJFREE(tmp);
            }
        }

        if(list->table)
        {
            if(donodes)
                ajTableMapDel(list->table, &domClearMapAll, (void *) doc);
            else
                ajTableMapDel(list->table, &domClearMapValues, NULL);

            ajTableFree(&list->table);
        }

        AJFREE(list);
    }

    return;
}




/* @funcstatic domClearMapValues **********************************************
**
** Clear only the values from the map table
**
** @param [r] key [void**] Standard argument, table key.
** @param [r] value [void**] Standard argument, table data item.
** @param [r] cl [void*] Standard argument, usually NULL
**
** @return [void]
**
** @release 6.3.0
** @@
******************************************************************************/

static void domClearMapValues(void **key, void **value, void *cl)
{
    AjPDomNodeEntry entry = NULL;

    entry = (AjPDomNodeEntry) *value;
    AJFREE(entry);

    (void) cl;                          /* make it used */
    (void) key;

    *value = NULL;

    return;
}




/* @funcstatic domClearMapAll *************************************************
**
** Clear keys and values from the map table
**
** @param [r] key [void**] Standard argument, table key.
** @param [r] value [void**] Standard argument, table data item.
** @param [r] cl [void*] Standard argument, usually NULL
**
** @return [void]
**
** @release 6.4.0
** @@
******************************************************************************/

static void domClearMapAll(void **key, void **value, void *cl)
{
    AjPDomNodeEntry entry = NULL;
    AjPDomNode node = NULL;
    AjPDomDocument doc = NULL;

    entry = (AjPDomNodeEntry) *value;
    AJFREE(entry);

    node = (AjPDomNode) *key;

    doc = (AjPDomDocument) cl;

    ajDomDocumentDestroyNode(doc, node);

    *value = NULL;

    return;
}




/* @func ajDomCreateNodeList **************************************************
**
** Create a node list
**
** @param [u] doc [AjPDomDocument] document
** @return [AjPDomNodeList] new list
**
** @release 4.0.0
** @@
******************************************************************************/

AjPDomNodeList ajDomCreateNodeList(AjPDomDocument doc)
{
    AjPDomNodeList list = NULL;

    AJNEW0(list);
    list->ownerdocument = doc;

    return list;
}




/* @func ajDomDocumentCreateNode **********************************************
**
** General document node creation
**
** @param [u] doc [AjPDomDocument] document
** @param [u] nodetype [AjEDomNodeType] AJAX DOM Node Type enumeration
** @return [AjPDomNode] new node
**
** @release 4.0.0
** @@
******************************************************************************/

AjPDomNode ajDomDocumentCreateNode(AjPDomDocument doc,
                                   AjEDomNodeType nodetype)
{
    AjPDomNode node;

    if (!doc && nodetype != ajEDomNodeTypeDocumentNode &&
        nodetype != ajEDomNodeTypeDocumentType)
    {
        ajDebug("ajDocumentCreateNode: allocation failure\n");
        return NULL;
    }

    AJNEW0(node);

    node->ownerdocument = doc;
    node->type          = nodetype;

    switch (nodetype)
    {
        case ajEDomNodeTypeDocumentNode:
        case ajEDomNodeTypeDocumentType:
        case ajEDomNodeTypeElement:
        case ajEDomNodeTypeAttribute:
        case ajEDomNodeTypeEntityReference:
        case ajEDomNodeTypeEntityNode:
        case ajEDomNodeTypeDocumentFragment:
            node->childnodes = ajDomCreateNodeList(doc);

            if(node->childnodes == NULL)
            {
                ajDebug("ajDocumentCreateNode: ajDocumentCreateNodeList"
                        " failed\n");
                ajDomDocumentDestroyNode(doc, node);
                return NULL;
            }
        default:
            ajDebug("ajDomDocumentCreateNode got unexpected "
                    "AJAX DOM Node type %d.\n", nodetype);
    }

    return node;
}




/* @func ajDomImplementationCreateDocumentType ********************************
**
** Creates an empty DocumentType node into which entities/notations (etc)
** can be placed
**
** @param [r] qualname [const AjPStr] qualified name
** @param [r] publicid [const AjPStr] public id
** @param [r] systemid [const AjPStr] systemid
** @return [AjPDomDocumentType] new DocumentType node
**
** @release 4.0.0
** @@
******************************************************************************/

AjPDomDocumentType ajDomImplementationCreateDocumentType(const AjPStr qualname,
                                                         const AjPStr publicid,
                                                         const AjPStr systemid)
{
    char *p = NULL;
    char *s = NULL;
    char *r = NULL;

    if(qualname)
        p = qualname->Ptr;

    if(publicid)
        r = publicid->Ptr;

    if(systemid)
        s = systemid->Ptr;

    return ajDomImplementationCreateDocumentTypeC(p, r, s);
}




/* @func ajDomImplementationCreateDocumentTypeC *******************************
**
** Creates an empty DocumentType node into which entities/notations (etc)
** can be placed
**
** @param [r] qualname [const char *] qualified name
** @param [r] publicid [const char *] public id
** @param [r] systemid [const char *] systemid
** @return [AjPDomDocumentType] new DocumentType node
**
** @release 4.0.0
** @@
******************************************************************************/

AjPDomDocumentType ajDomImplementationCreateDocumentTypeC(const char *qualname,
                                                          const char *publicid,
                                                          const char *systemid)
{
    AjPDomDocumentType doctype;
    AjPDomNodeMap entities;
    AjPDomNodeMap notations;


    if(!(doctype = ajDomDocumentCreateNode(NULL, ajEDomNodeTypeDocumentType)))
    {
        ajDebug("ajDomImplementationCreateDocumentType: Cannot create node");
        return NULL;
    }

    ajStrAssignC(&doctype->sub.DocumentType.name, qualname);
    doctype->name = doctype->sub.DocumentType.name;

    if(publicid)
        ajStrAssignC(&doctype->sub.DocumentType.publicid, publicid);

    if(systemid)
        ajStrAssignC(&doctype->sub.DocumentType.systemid, systemid);


    entities  = ajDomCreateNodeList(NULL);
    notations = ajDomCreateNodeList(NULL);

    entities->filter = ajEDomNodeTypeEntityNode;
    entities->list   = doctype->childnodes;

    notations->filter = ajEDomNodeTypeNotation;
    notations->list   = doctype->childnodes;

    doctype->sub.DocumentType.entities = entities;
    doctype->sub.DocumentType.notations = notations;

    return doctype;
}




/* @func ajDomImplementationCreateDocument ************************************
**
** Creates an XML Document object of the specified type with its document
** element.
**
** @param [r] uri [const AjPStr] uri (not implemented. Pass NULL)
** @param [r] qualname [const AjPStr] qualified name
** @param [u] doctype [AjPDomDocumentType] doctype
** @return [AjPDomDocument] new document
**
** @release 4.0.0
** @@
******************************************************************************/

AjPDomDocument ajDomImplementationCreateDocument(const AjPStr uri,
                                                 const AjPStr qualname,
                                                 AjPDomDocumentType doctype)
{
    const char *p = NULL;

    (void) uri;                         /* temporary use */

    if(qualname)
        p = qualname->Ptr;

    return ajDomImplementationCreateDocumentC(NULL, p, doctype);
}




/* @func ajDomImplementationCreateDocumentC ***********************************
**
** Creates an XML Document object of the specified type with its document
** element.
**
** @param [r] uri [const char *] uri (not implemented. Pass NULL)
** @param [r] qualname [const char *] qualified name
** @param [u] doctype [AjPDomDocumentType] doctype
** @return [AjPDomDocument] new document
**
** @release 4.0.0
** @@
******************************************************************************/

AjPDomDocument ajDomImplementationCreateDocumentC(const char *uri,
                                                  const char *qualname,
                                                  AjPDomDocumentType doctype)
{
    AjPDomDocument doc     = NULL;
    AjPDomElement  element = NULL;

    (void) uri;

    if(!(doc = ajDomDocumentCreateNode(NULL, ajEDomNodeTypeDocumentNode)))
    {
        ajDebug("ajDomImplementationCreateDocumentC: document memory\n");
        return NULL;
    }

    doc->name = ajStrNew();
    ajStrAssignC(&doc->name, "#document");

    if(doctype)
        ajDomNodeAppendChild(doc, doctype);

    if(qualname)
        if(strlen(qualname))
        {
            element = ajDomDocumentCreateElementC(doc, qualname);
            if(!element)
            {
                ajDebug("ajDomImplementationCreateDocumentC: element memory\n");
                ajDomDocumentDestroyNode(doc, doc);

                return NULL;
            }

            ajDomNodeAppendChild(doc, element);
        }

    return doc;
}




/* @func ajDomNodeMapGetItem **************************************************
**
** Returns the named node from a node map
**
** @param [r] map [const AjPDomNodeMap] map
** @param [r] name [const AjPStr] name
** @return [AjPDomNode] node
**
** @release 4.0.0
** @@
******************************************************************************/

AjPDomNode ajDomNodeMapGetItem(const AjPDomNodeMap map, const AjPStr name)
{
    char *p = NULL;

    if(name)
        p = name->Ptr;

    return ajDomNodeMapGetItemC(map, p);
}




/* @func ajDomNodeMapGetItemC *************************************************
**
** Returns the named node from a node map
**
** @param [r] map [const AjPDomNodeMap] map
** @param [r] name [const char *] name
** @return [AjPDomNode] node
**
** @release 4.0.0
** @@
******************************************************************************/

AjPDomNode ajDomNodeMapGetItemC(const AjPDomNodeMap map, const char *name)
{
    AjPDomNodeEntry e = NULL;
    ajuint nodetype;

    if(map && name)
    {
        if(map->filter)
        {
            nodetype = map->filter;
            map = map->list;
            for(e = map->first; e; e = e->next)
                if(e->node->type == nodetype &&
                   !strcmp(name, e->node->name->Ptr))
                    return e->node;
        }
        else
            for(e = map->first; e; e = e->next)
                if(!strcmp(name, e->node->name->Ptr))
                    return e->node;
    }

    return NULL;
}




/* @func ajDomElementGetAttribute *********************************************
**
** Returns the value of a named attribute
**
** @param [r] element [const AjPDomElement] element
** @param [r] name [const AjPStr] name
** @return [AjPStr] value or empty string
**
** @release 4.0.0
** @@
******************************************************************************/

AjPStr ajDomElementGetAttribute(const AjPDomElement element, const AjPStr name)
{
    char *p = NULL;

    if(name)
        p = name->Ptr;

    return ajDomElementGetAttributeC(element, p);
}




/* @func ajDomElementGetAttributeC ********************************************
**
** Returns the value of a named attribute
**
** @param [r] element [const AjPDomElement] element
** @param [r] name [const char *] name
** @return [AjPStr] value or empty string
**
** @release 4.0.0
** @@
******************************************************************************/

AjPStr ajDomElementGetAttributeC(const AjPDomElement element,
                                 const char *name)
{
    AjPDomNode node = NULL;
    AjPStr ret = NULL;

    ret = ajStrNewC("");

    if(element && name && element->attributes)
    {
        if((node = ajDomNodeMapGetItemC(element->attributes, name)))
            ajStrAssignS(&ret, node->value);
    }

    return ret;
}




/* @func ajDomNodeMapSetItem **************************************************
**
** Enter arg into the node map using name as the key.
** If a node with the same name already exists in the map it will be replaced
** with the new node and returned.
** The replaced node should usually be freed with DestroyNode.
**
** @param [u] map [AjPDomNodeMap] map
** @param [u] arg [AjPDomNode] arg
** @return [AjPDomNode] replaced node or NULL
**
** @release 4.0.0
** @@
******************************************************************************/

AjPDomNode ajDomNodeMapSetItem(AjPDomNodeMap map, AjPDomNode arg)
{
    AjPDomNodeEntry e = NULL;
    AjPDomNode tmp = NULL;

    if(map && arg)
    {
        if(map->filter)
        {
            ajDebug("ajDomNodeMapSetItem: No mod allowed\n");

            return NULL;
        }

        if(map->ownerdocument != arg->ownerdocument)
        {
            ajDebug("ajDomNodeMapSetItem: Wrong document\n");

            return NULL;
        }

        if(arg->type == ajEDomNodeTypeAttribute &&
           arg->sub.Attr.ownerelement &&
           arg->sub.Attr.ownerelement != map->ownerelement)
        {
            ajDebug("ajDomNodeMapSetItem: In use attribute error\n");

            return NULL;
        }

        for(e = map->first; e && strcmp(arg->name->Ptr, e->node->name->Ptr);
            e = e->next)
        {
            ;
        }

        if(e)
        {
            tmp = e->node;
            e->node = arg;
            if(arg->type == ajEDomNodeTypeAttribute)
            {
                arg->sub.Attr.ownerelement = map->ownerelement;
                tmp->sub.Attr.ownerelement = NULL;
            }

            return tmp;
        }

        ajDomNodeListAppend(map, arg);
    }

    return NULL;
}




/* @func ajDomNodeMapRemoveItem ***********************************************
**
** Removes and returns item from a  map
**
** @param [u] map [AjPDomNodeMap] map
** @param [r] name [const AjPStr] name
** @return [AjPDomNode] removed node or NULL
**
** @release 4.0.0
** @@
******************************************************************************/

AjPDomNode ajDomNodeMapRemoveItem(AjPDomNodeMap map, const AjPStr name)
{
    char *p = NULL;

    if(name)
        p = name->Ptr;

    return ajDomNodeMapRemoveItemC(map, p);
}




/* @func ajDomNodeMapRemoveItemC **********************************************
**
** Removes and returns item from a  map
**
** @param [w] map [AjPDomNodeMap] map
** @param [r] name [const char *] name
** @return [AjPDomNode] removed node or NULL
**
** @release 4.0.0
** @@
******************************************************************************/

AjPDomNode ajDomNodeMapRemoveItemC(AjPDomNodeMap map, const char *name)
{
    AjPDomNodeEntry e = NULL;
    AjPDomNode r = NULL;

    if(map && strlen(name))
    {
        if(map->filter)
        {
            ajDebug("ajDomNodeMapRemoveItemC: No mod allowed\n");

            return NULL;
        }

        for(e = map->first; e; e = e->next)
        {
            if(!strcmp(name, e->node->name->Ptr) &&
               ajDomNodeListRemove(map, e->node))
            {
                r = e->node;
                free(e);

                if(r->type == ajEDomNodeTypeAttribute)
                    r->sub.Attr.ownerelement = NULL;

                return r;
            }
        }
    }

    ajDebug("ajDomNodeMapRemoveItemC: Not found error\n");

    return NULL;
}




/* @func ajDomNodeMapItem *****************************************************
**
** Return the node in the map at a given index
**
** @param [r] map [const AjPDomNodeMap] map
** @param [r] indexnum [ajint] index
** @return [AjPDomNode] node or NULL
**
** @release 4.0.0
** @@
******************************************************************************/

AjPDomNode ajDomNodeMapItem(const AjPDomNodeMap map, ajint indexnum)
{
    return ajDomNodeListItem(map, indexnum);
}




/* @funcstatic domNodeListItemFiltered ****************************************
**
** Filter node list item
**
** @param [r] list [const AjPDomNodeList] list
** @param [r] indexnum [ajint] index
** @param [r] nodetype [ajuint] nodetype
** @return [AjPDomNode] node or NULL
**
** @release 4.0.0
** @@
******************************************************************************/

static AjPDomNode domNodeListItemFiltered(const AjPDomNodeList list,
                                          ajint indexnum, ajuint nodetype)
{
    AjPDomNodeEntry e = NULL;

    if(list && indexnum >= 0 && indexnum < list->length)
        for(e = list->first; e; e = e->next)
            if(e->node->type == nodetype)
            {
                if(!indexnum)
                    return e->node;

                --indexnum;
            }

    return NULL;
}




/* @func ajDomNodeListItem ****************************************************
**
** Return the node in the list at a given index
**
** @param [r] list [const AjPDomNodeList] list
** @param [r] indexnum [ajint] index
** @return [AjPDomNode] node or NULL
**
** @release 4.0.0
** @@
******************************************************************************/

AjPDomNode ajDomNodeListItem(const AjPDomNodeList list, ajint indexnum)
{
    AjPDomNodeEntry e = NULL;

    if(list)
    {
        if(list->filter)
            return domNodeListItemFiltered(list->list, indexnum, list->filter);

        if(indexnum >= 0 && indexnum < list->length)
            for(e = list->first; e; e = e->next, --indexnum)
                if(!indexnum)
                    return e->node;
    }

    return NULL;
}




/* @func ajDomElementSetAttribute *********************************************
**
** Adds a new attribute or sets the value of an existing attribute.
**
** @param [r] element [const AjPDomElement] element
** @param [r] name [const AjPStr] name
** @param [r] value [const AjPStr] value
** @return [void]
**
** @release 4.0.0
** @@
******************************************************************************/

void ajDomElementSetAttribute(const AjPDomElement element, const AjPStr name,
                              const AjPStr value)
{
    if(!element || !name || !value || !element->attributes)
        return;

    if(!ajStrGetLen(name) || !ajStrGetLen(value))
        return;

    ajDomElementSetAttributeC(element, name->Ptr, value->Ptr);

    return;
}




/* @func ajDomElementSetAttributeC ********************************************
**
** Adds a new attribute or sets the value of an existing attribute.
**
** @param [r] element [const AjPDomElement] element
** @param [r] name [const char *] name
** @param [r] value [const char *] value
** @return [void]
**
** @release 4.0.0
** @@
******************************************************************************/

void ajDomElementSetAttributeC(const AjPDomElement element, const char *name,
                               const char *value)
{
    AjPDomNode attr = NULL;
    AjPStr prevvalue = NULL;


    if(!element || !name || !value || !element->attributes)
        return;

    attr = ajDomNodeMapGetItemC(element->attributes, name);

    if(attr)
    {
        prevvalue = attr->value;

        attr->sub.Attr.value = ajStrNew();
        ajStrAssignC(&attr->sub.Attr.value, value);
        /* AJB: dirty */
        attr->value = attr->sub.Attr.value;
    }
    else
    {
        prevvalue = NULL;
        if(!(attr = ajDomDocumentCreateAttributeC(element->ownerdocument,
                                                  name)))
        {
            ajDebug("ajDomElementSetAttribute: DOM create failed\n");
            return;
        }

        ajStrDel(&attr->value);

        attr->sub.Attr.value = ajStrNew();
        ajStrAssignC(&attr->sub.Attr.value, value);
        /* AJB: dirty */
        attr->value = attr->sub.Attr.value;

        ajDomNodeMapSetItem(element->attributes, attr);
    }

    domUpdateNode(element->parentnode);

    if(prevvalue)
        ajStrDel(&prevvalue);

    return;
}




/* @func ajDomElementRemoveAttribute ******************************************
**
** Remove and free a named attribute
**
** @param [u] element [AjPDomElement] element
** @param [r] name [const AjPStr] name
** @return [void]
**
** @release 4.0.0
** @@
******************************************************************************/

void ajDomElementRemoveAttribute(AjPDomElement element, const AjPStr name)
{

    if(!name)
        return;

    ajDomElementRemoveAttributeC(element, name->Ptr);

    return;
}




/* @func ajDomElementRemoveAttributeC *****************************************
**
** Remove and free a named attribute
**
** @param [u] element [AjPDomElement] element
** @param [r] name [const char *] name
** @return [void]
**
** @release 4.0.0
** @@
******************************************************************************/

void ajDomElementRemoveAttributeC(AjPDomElement element, const char *name)
{
    AjPDomNode attr = NULL;

    if(!element || !name)
        return;

    attr = ajDomNodeMapRemoveItemC(element->attributes, name);

    if(attr)
    {
        domUpdateNode(element->parentnode);
        ajDomDocumentDestroyNode(attr->ownerdocument, attr);
    }

    return;
}




/* @func ajDomElementGetAttributeNode *****************************************
**
** Returns the named attribute node.
**
** @param [r] element [const AjPDomElement] element
** @param [r] name [const AjPStr] name
** @return [AjPDomNode] node or NULL
**
** @release 4.0.0
** @@
******************************************************************************/

AjPDomNode ajDomElementGetAttributeNode(const AjPDomElement element,
                                        const AjPStr name)
{
    if(element && name)
        return ajDomNodeMapGetItem(element->attributes, name);

    return NULL;
}




/* @func ajDomElementGetAttributeNodeC ****************************************
**
** Returns the named attribute node.
**
** @param [r] element [const AjPDomElement] element
** @param [r] name [const char *] name
** @return [AjPDomNode] node or NULL
**
** @release 4.0.0
** @@
******************************************************************************/

AjPDomNode ajDomElementGetAttributeNodeC(const AjPDomElement element,
                                         const char *name)
{
    if(element && name)
        return ajDomNodeMapGetItemC(element->attributes, name);

    return NULL;
}




/* @func ajDomElementSetAttributeNode *****************************************
**
** Add an attribute to the attributes of an element.
** If this element already has an attribute with the same name it will be
** replaced with the new attribute and returned.
**
** @param [u] element [AjPDomElement] element
** @param [u] newattr [AjPDomNode] name
** @return [AjPDomNode] new or replaced attribute
**
** @release 4.0.0
** @@
******************************************************************************/

AjPDomNode ajDomElementSetAttributeNode(AjPDomElement element,
                                        AjPDomNode newattr)
{
    AjPDomNode attr = NULL;

    if(!element || !newattr)
        return NULL;

    if(element->ownerdocument != newattr->ownerdocument)
    {
        ajDebug("ajDomElementSetAttributeNode: Wrong document\n");
        return NULL;
    }

    attr = ajDomNodeMapSetItem(element->attributes, newattr);

    domUpdateNode(element->parentnode);

    return attr;
}




/* @func ajDomElementRemoveAttributeNode **************************************
**
** Removes and returns a pointer to an attribute
**
** @param [w] element [AjPDomElement] element
** @param [u] oldattr [AjPDomNode] attribute
** @return [AjPDomNode] removed attribute or NULL
**
** @release 4.0.0
** @@
******************************************************************************/

AjPDomNode ajDomElementRemoveAttributeNode(AjPDomElement element,
                                           AjPDomNode oldattr)
{
    if(!element || !oldattr || !ajDomNodeListRemove(element->attributes,
                                                    oldattr))
    {
        ajDebug("ajDomElementRemoveAttributeNode: DOM not found error\n");
        return NULL;
    }

    domUpdateNode(element->parentnode);

    return oldattr;
}




/* @funcstatic domTraverse ****************************************************
**
** Pre-order elements
**
** @param [w] list [AjPDomNodeList] list
** @param [u] node [AjPDomNode] node
** @param [r] tagname [const AjPStr] tagname
** @return [void]
**
** @release 4.0.0
** @@
******************************************************************************/

static void domTraverse(AjPDomNodeList list, AjPDomNode node,
                        const AjPStr tagname)
{
    AjPDomNode n = NULL;

    if(list && node && node->type == ajEDomNodeTypeElement && tagname)
    {
        if(ajStrMatchC(tagname, "*") || !strcmp(tagname->Ptr, node->name->Ptr))
            ajDomNodeListAppend(list, node);

        for(n = node->firstchild; n; n = n->nextsibling)
            domTraverse(list, n, tagname);
    }

    return;
}




/* @funcstatic domTraverseC ***************************************************
**
** Pre-order elements
**
** @param [w] list [AjPDomNodeList] list
** @param [u] node [AjPDomNode] node
** @param [r] tagname [const char *] tagname
** @return [void]
**
** @release 4.0.0
** @@
******************************************************************************/

static void domTraverseC(AjPDomNodeList list, AjPDomNode node,
                         const char *tagname)
{
    AjPDomNode n = NULL;

    if(list && node && node->type == ajEDomNodeTypeElement && tagname)
    {
        if(ajCharMatchC(tagname, "*") || !strcmp(tagname, node->name->Ptr))
            ajDomNodeListAppend(list, node);

        for(n = node->firstchild; n; n = n->nextsibling)
            domTraverseC(list, n, tagname);
    }

    return;
}




/* @func ajDomElementGetElementsByTagName *************************************
**
** Perform a pre-order traversal of the entire document.
** Return a node list of the elements with the name tagname in the order
** in which they are found.
**
** @param [u] element [AjPDomElement] element
** @param [r] name [const AjPStr] name
** @return [AjPDomNodeList] node list
**
** @release 4.0.0
** @@
******************************************************************************/

AjPDomNodeList ajDomElementGetElementsByTagName(AjPDomElement element,
                                                const AjPStr name)
{

    if(!element || !name)
        return NULL;

    return ajDomElementGetElementsByTagNameC(element, name->Ptr);
}




/* @func ajDomElementGetElementsByTagNameC ************************************
**
** Perform a pre-order traversal of the entire document.
** Return a node list of the elements with the name tagname in the order
** in which they are found.
**
** @param [u] element [AjPDomElement] element
** @param [r] name [const char *] name
** @return [AjPDomNodeList] node list
**
** @release 4.0.0
** @@
******************************************************************************/

AjPDomNodeList ajDomElementGetElementsByTagNameC(AjPDomElement element,
                                                 const char *name)
{
    AjPDomNodeList list = NULL;
    AjPDomNode n = NULL;

    if(element && element->type == ajEDomNodeTypeElement && name &&
       (list=ajDomCreateNodeList(element->ownerdocument)))
    {
        for(n = element->firstchild; n; n = n->nextsibling)
            domTraverseC(list, n, name);

        return list;
    }

    return NULL;
}




/* @func ajDomElementNormalise ************************************************
**
** Merge adjacent text node content into "normal" form in the subtree of
** a node. Remove empty text node.
**
** @param [w] element [AjPDomElement] element
** @return [void]
**
** @release 4.0.0
** @@
******************************************************************************/

void ajDomElementNormalise(AjPDomElement element)
{
    AjPDomNode node = NULL;
    AjPDomText last = NULL;


    if(element)
    {
        for(node = element->firstchild; node; node = node->nextsibling)
        {
            if(node->type == ajEDomNodeTypeText)
            {
                if(last)
                {
                    ajDomCharacterDataInsertData(node, 0, last->value);
                    ajDomRemoveChild(element, last);
                    ajDomDocumentDestroyNode(last->ownerdocument, last);
                }

                last = node;
            }
            else
            {
                last = NULL;

                ajDomElementNormalise(node);
            }

            /* AJB: Would do a return here if exception */
        }
    }

    return;
}




/* @func ajDomCharacterDataSubstringData **************************************
**
** Return a string representing the substring beginning at the character at
** offset with a length of count.
** If the sum of offset and count exceeds the length of the character data,
** a string representing the remainder of the string is returned.
**
** @param [r] data [const AjPDomCharacterData] character data
** @param [r] offset [ajint] offset
** @param [r] count [ajint] count

** @return [AjPStr] substring
**
** @release 4.0.0
** @@
******************************************************************************/

AjPStr ajDomCharacterDataSubstringData(const AjPDomCharacterData data,
                                       ajint offset,
                                       ajint count)
{
    AjPStr sub = NULL;
    ajint dlen = 0;

    if(!data)
        return NULL;

    if(offset < 0 || offset > (dlen = data->sub.CharacterData.length) ||
       count < 0)
        return NULL;

    if(count > (dlen - offset))
        count = dlen - offset;

    sub = ajStrNew();

    ajStrAssignSubS(&sub, data->value, offset, offset + count - 1);

    return sub;
}




/* @func ajDomCharacterDataAppendData *****************************************
**
** Append a string to a character data node
**
** @param [w] data [AjPDomCharacterData] character data
** @param [r] arg [const AjPStr] string
** @return [void]
**
** @release 4.0.0
** @@
******************************************************************************/

void ajDomCharacterDataAppendData(AjPDomCharacterData data, const AjPStr arg)
{

    if(!data || !arg)
        return;

    ajDomCharacterDataAppendDataC(data, arg->Ptr);

    return;
}




/* @func ajDomCharacterDataAppendDataC ****************************************
**
** Append a string to a character data node
**
** @param [w] data [AjPDomCharacterData] character data
** @param [r] arg [const char *] string
** @return [void]
**
** @release 4.0.0
** @@
******************************************************************************/

void ajDomCharacterDataAppendDataC(AjPDomCharacterData data, const char *arg)
{

    if(!data || !arg)
        return;

    ajStrAppendC(&data->value, arg);
    data->sub.CharacterData.length = ajStrGetLen(data->value);
    data->sub.CharacterData.data = data->value;

    domUpdateNode(data->parentnode);

    return;
}




/* @func ajDomCharacterDataInsertData *****************************************
**
** Insert a string at position offset
**
** @param [w] data [AjPDomCharacterData] character data
** @param [r] offset [ajint] offset
** @param [r] arg [const AjPStr] string
** @return [void]
**
** @release 4.0.0
** @@
******************************************************************************/

void ajDomCharacterDataInsertData(AjPDomCharacterData data, ajint offset,
                                  const AjPStr arg)
{

    if(!data || !arg)
        return;

    ajDomCharacterDataInsertDataC(data, offset, arg->Ptr);

    return;
}




/* @func ajDomCharacterDataInsertDataC ****************************************
**
** Insert a string at position offset
**
** @param [w] data [AjPDomCharacterData] character data
** @param [r] offset [ajint] offset
** @param [r] arg [const char *] string
** @return [void]
**
** @release 4.0.0
** @@
******************************************************************************/

void ajDomCharacterDataInsertDataC(AjPDomCharacterData data, ajint offset,
                                   const char *arg)
{

    if(!data || !arg)
        return;

    if(offset < 0 || offset > data->sub.CharacterData.length)
        return;

    ajStrInsertC(&data->value, offset, arg);
    data->sub.CharacterData.length = ajStrGetLen(data->value);
    data->sub.CharacterData.data = data->value;

    domUpdateNode(data->parentnode);

    return;
}




/* @func ajDomCharacterDataDeleteData *****************************************
**
** Remove at most count characters of position offset
**
** @param [w] data [AjPDomCharacterData] character data
** @param [r] offset [ajint] offset
** @param [r] count [ajint] count
** @return [void]
**
** @release 4.0.0
** @@
******************************************************************************/

void ajDomCharacterDataDeleteData(AjPDomCharacterData data, ajint offset,
                                  ajint count)
{
    ajint dlen = 0;

    if(!data)
        return;

    dlen = ajStrGetLen(data->value);

    if(offset <0 || offset > dlen)
        return;

    if(count < 0 || (offset + count) > dlen)
        count = dlen - offset;

    ajStrCutRange(&data->value, offset, offset + count - 1);
    data->sub.CharacterData.length = ajStrGetLen(data->value);
    data->sub.CharacterData.data = data->value;

    domUpdateNode(data->parentnode);

    return;
}




/* @func ajDomCharacterDataReplaceData ****************************************
**
** Replace offset/count characters with a string
**
** @param [w] data [AjPDomCharacterData] character data
** @param [r] offset [ajint] offset
** @param [r] count [ajint] count
** @param [r] arg [const AjPStr] replacement string
** @return [void]
**
** @release 4.0.0
** @@
******************************************************************************/

void ajDomCharacterDataReplaceData(AjPDomCharacterData data, ajint offset,
                                   ajint count, const AjPStr arg)
{
    ajDomCharacterDataDeleteData(data, offset, count);
    ajDomCharacterDataInsertData(data, offset, arg);

    return;
}




/* @func ajDomCharacterDataReplaceDataC ***************************************
**
** Replace offset/count characters with a string
**
** @param [w] data [AjPDomCharacterData] character data
** @param [r] offset [ajint] offset
** @param [r] count [ajint] count
** @param [r] arg [const char *] replacement string
** @return [void]
**
** @release 4.0.0
** @@
******************************************************************************/

void ajDomCharacterDataReplaceDataC(AjPDomCharacterData data, ajint offset,
                                    ajint count, const char *arg)
{
    ajDomCharacterDataDeleteData(data, offset, count);
    ajDomCharacterDataInsertDataC(data, offset, arg);

    return;
}




/* @func ajDomCharacterDataGetLength ******************************************
**
** Return length of character data
**
** @param [r] data [const AjPDomCharacterData] character data
** @return [ajint] length
**
** @release 4.0.0
** @@
******************************************************************************/

ajint ajDomCharacterDataGetLength(const AjPDomCharacterData data)
{
    return data ? data->sub.CharacterData.length : 0;
}




/* @func ajDomTextSplitText ***************************************************
**
** Split a text node at the specified offset into two adjacent text nodes.
** The first is a string offset size in length whereas the second is the
** remainder. If offset is equal to the length of the string the new sibling
** has zero length
**
** @param [w] text [AjPDomText] text
** @param [r] offset [ajint] offset
** @return [AjPDomText] remainder node or NULL
**
** @release 4.0.0
** @@
******************************************************************************/

AjPDomText ajDomTextSplitText(AjPDomText text, ajint offset)
{
    AjPDomText node;
    AjPStr sub = NULL;
    ajint len = 0;

    if(!text || !text->parentnode)
        return NULL;

    if(offset < 0 || offset >text->sub.CharacterData.length)
        return NULL;

    sub = ajStrNew();
    len = ajStrGetLen(text->value);

    ajStrAssignSubS(&sub, text->value, offset, len - 1);

    node = ajDomDocumentCreateTextNode(text->ownerdocument, sub);

    ajStrDel(&sub);

    if(!node)
        return NULL;

    ajDomCharacterDataDeleteData(text, offset, len - 1);
    ajDomNodeInsertBefore(text->parentnode, node, text->nextsibling);

    return node;
}




/* @func ajDomDocumentCreateElement *******************************************
**
** Create a document element
**
** @param [w] doc [AjPDomDocument] document
** @param [r] tagname [const AjPStr] tagname
** @return [AjPDomElement] element
**
** @release 4.0.0
** @@
******************************************************************************/

AjPDomElement ajDomDocumentCreateElement(AjPDomDocument doc,
                                         const AjPStr tagname)
{
    if(!tagname)
        return NULL;

    return ajDomDocumentCreateElementC(doc, tagname->Ptr);
}




/* @func ajDomDocumentCreateElementC ******************************************
**
** Create a document element
**
** @param [w] doc [AjPDomDocument] document
** @param [r] tagname [const char *] tagname
** @return [AjPDomElement] element
**
** @release 4.0.0
** @@
******************************************************************************/

AjPDomElement ajDomDocumentCreateElementC(AjPDomDocument doc,
                                          const char *tagname)
{
    AjPDomElement element = NULL;

    if(!tagname)
        return NULL;

    element = ajDomDocumentCreateNode(doc, ajEDomNodeTypeElement);

    if(!element)
        return NULL;

    element->sub.Element.tagname = ajStrNewC(tagname);
    element->name = element->sub.Element.tagname;

    element->attributes = ajDomCreateNodeList(doc);

    element->attributes->ownerelement = element;

    return element;
}




/* @func ajDomDocumentCreateDocumentFragment **********************************
**
** Create an empty document fragment
**
** @param [w] doc [AjPDomDocument] document
** @return [AjPDomDocumentFragment] fragment
**
** @release 4.0.0
** @@
******************************************************************************/

AjPDomDocumentFragment ajDomDocumentCreateDocumentFragment(AjPDomDocument doc)
{
    AjPDomDocumentFragment frag = NULL;

    frag = ajDomDocumentCreateNode(doc, ajEDomNodeTypeDocumentFragment);

    if(frag)
        frag->name = ajStrNewC("#document-fragment");

    return frag;
}




/* @func ajDomDocumentCreateTextNode ******************************************
**
** Create a  text node
**
** @param [w] doc [AjPDomDocument] document
** @param [r] data [const AjPStr] text
** @return [AjPDomText] text node
**
** @release 4.0.0
** @@
******************************************************************************/

AjPDomText ajDomDocumentCreateTextNode(AjPDomDocument doc, const AjPStr data)
{
    if(!data)
        return NULL;

    return ajDomDocumentCreateTextNodeC(doc, data->Ptr);
}




/* @func ajDomDocumentCreateTextNodeC *****************************************
**
** Create a text node
**
** @param [w] doc [AjPDomDocument] document
** @param [r] data [const char *] text
** @return [AjPDomText] text node
**
** @release 4.0.0
** @@
******************************************************************************/

AjPDomText ajDomDocumentCreateTextNodeC(AjPDomDocument doc, const char *data)
{
    AjPDomText text = NULL;

    text = ajDomDocumentCreateNode(doc, ajEDomNodeTypeText);
    if(!text)
        return NULL;

    text->name = ajStrNewC("#text");

    text->sub.CharacterData.data = ajStrNewC(data);
    text->value = text->sub.CharacterData.data;

    text->sub.CharacterData.length = ajStrGetLen(text->value);

    return text;
}




/* @func ajDomDocumentCreateComment *******************************************
**
** Create a comment node
**
** @param [w] doc [AjPDomDocument] document
** @param [r] data [const AjPStr] text
** @return [AjPDomComment] comment node
**
** @release 4.0.0
** @@
******************************************************************************/

AjPDomComment ajDomDocumentCreateComment(AjPDomDocument doc, const AjPStr data)
{
    if(!data)
        return NULL;

    return ajDomDocumentCreateCommentC(doc, data->Ptr);
}




/* @func ajDomDocumentCreateCommentC ******************************************
**
** Create a comment node
**
** @param [w] doc [AjPDomDocument] document
** @param [r] data [const char *] text
** @return [AjPDomComment] comment node
**
** @release 4.0.0
** @@
******************************************************************************/

AjPDomComment ajDomDocumentCreateCommentC(AjPDomDocument doc, const char *data)
{
    AjPDomComment comment = NULL;

    comment = ajDomDocumentCreateNode(doc, ajEDomNodeTypeComment);

    if(!comment)
        return NULL;

    comment->name = ajStrNewC("#comment");

    comment->sub.CharacterData.data = ajStrNewC(data);
    comment->value = comment->sub.CharacterData.data;

    comment->sub.CharacterData.length = ajStrGetLen(comment->value);

    return comment;
}




/* @func ajDomDocumentCreateCDATASection **************************************
**
** Create a CDATA section
**
** @param [w] doc [AjPDomDocument] document
** @param [r] data [const AjPStr] text
** @return [AjPDomCDATASection] comment node
**
** @release 4.0.0
** @@
******************************************************************************/

AjPDomCDATASection ajDomDocumentCreateCDATASection(AjPDomDocument doc,
                                                   const AjPStr data)
{
    if(!data)
        return NULL;

    return ajDomDocumentCreateCDATASectionC(doc, data->Ptr);
}




/* @func ajDomDocumentCreateCDATASectionC *************************************
**
** Create a CDATA section
**
** @param [w] doc [AjPDomDocument] document
** @param [r] data [const char *] text
** @return [AjPDomCDATASection] comment node
**
** @release 4.0.0
** @@
******************************************************************************/

AjPDomCDATASection ajDomDocumentCreateCDATASectionC(AjPDomDocument doc,
                                                    const char *data)
{
    AjPDomCDATASection cdata = NULL;

    cdata = ajDomDocumentCreateNode(doc, ajEDomNodeTypeCdataSection);

    if(!cdata)
        return NULL;

    cdata->name = ajStrNewC("#cdata-section");

    cdata->sub.CharacterData.data = ajStrNewC(data);
    cdata->value = cdata->sub.CharacterData.data;

    cdata->sub.CharacterData.length = ajStrGetLen(cdata->value);

    return cdata;
}




/* @func ajDomDocumentCreateAttribute *****************************************
**
** Create an attribute node
**
** @param [w] doc [AjPDomDocument] document
** @param [r] name [const AjPStr] text
** @return [AjPDomAttr] attribute node
**
** @release 4.0.0
** @@
******************************************************************************/

AjPDomAttr ajDomDocumentCreateAttribute(AjPDomDocument doc,
                                        const AjPStr name)
{
    AjPDomAttr attr = NULL;

    attr = ajDomDocumentCreateNode(doc, ajEDomNodeTypeAttribute);

    if(!attr)
        return NULL;

    attr->sub.Attr.name = ajStrNewS(name);
    attr->name = attr->sub.Attr.name;

    attr->sub.Attr.value = ajStrNewC("");
    attr->value = attr->sub.Attr.value;
    attr->sub.Attr.specified = 1;

    return attr;
}




/* @func ajDomDocumentCreateAttributeC ****************************************
**
** Create an attribute node
**
** @param [w] doc [AjPDomDocument] document
** @param [r] name [const char *] text
** @return [AjPDomAttr] attribute node
**
** @release 4.0.0
** @@
******************************************************************************/

AjPDomAttr ajDomDocumentCreateAttributeC(AjPDomDocument doc,
                                         const char *name)
{
    AjPDomAttr attr = NULL;

    attr = ajDomDocumentCreateNode(doc, ajEDomNodeTypeAttribute);

    if(!attr)
        return NULL;

    attr->sub.Attr.name = ajStrNewC(name);
    attr->name = attr->sub.Attr.name;

    attr->sub.Attr.value = ajStrNewC("");
    attr->value = attr->sub.Attr.value;
    attr->sub.Attr.specified = 1;

    return attr;
}




/* @func ajDomDocumentCreateEntityReference ***********************************
**
** Create an entity reference node
**
** @param [w] doc [AjPDomDocument] document
** @param [r] name [const AjPStr] text
** @return [AjPDomEntityReference] entity reference node
**
** @release 4.0.0
** @@
******************************************************************************/

AjPDomEntityReference ajDomDocumentCreateEntityReference(AjPDomDocument doc,
                                                         const AjPStr name)
{
    if(!name)
        return NULL;

    return ajDomDocumentCreateEntityReferenceC(doc, name->Ptr);
}




/* @func ajDomDocumentCreateEntityReferenceC **********************************
**
** Create an entity reference node
**
** @param [w] doc [AjPDomDocument] document
** @param [r] name [const char *] text
** @return [AjPDomEntityReference] entity reference node
**
** @release 4.0.0
** @@
******************************************************************************/

AjPDomEntityReference ajDomDocumentCreateEntityReferenceC(AjPDomDocument doc,
                                                          const char *name)
{
    AjPDomEntityReference eref = NULL;

    eref = ajDomDocumentCreateNode(doc, ajEDomNodeTypeEntityReference);

    if(!eref)
        return NULL;

    eref->name = ajStrNewC(name);

    return eref;
}




/* @func ajDomDocumentCreateProcessingInstruction *****************************
**
** Create a processing instruction node
**
** @param [w] doc [AjPDomDocument] document
** @param [r] target [const AjPStr] target
** @param [r] data [const AjPStr] data
** @return [AjPDomPi] entity reference node
**
** @release 4.0.0
** @@
******************************************************************************/

AjPDomPi ajDomDocumentCreateProcessingInstruction(AjPDomDocument doc,
                                                  const AjPStr target,
                                                  const AjPStr data)
{
    if(!target || !data)
        return NULL;

    return ajDomDocumentCreateProcessingInstructionC(doc, target->Ptr,
                                                     data->Ptr);
}




/* @func ajDomDocumentCreateProcessingInstructionC ****************************
**
** Create a processing instruction node
**
** @param [w] doc [AjPDomDocument] document
** @param [r] target [const char *] target
** @param [r] data [const char *] data
** @return [AjPDomPi] entity reference node
**
** @release 4.0.0
** @@
******************************************************************************/

AjPDomPi ajDomDocumentCreateProcessingInstructionC(AjPDomDocument doc,
                                                   const char *target,
                                                   const char *data)
{
    AjPDomPi pin = NULL;

    pin = ajDomDocumentCreateNode(doc, ajEDomNodeTypeProcessingInstruction);

    if(pin)
    {
        pin->sub.ProcessingInstruction.target = ajStrNewC(target);
        /* AJB: dirty */
        pin->name = pin->sub.ProcessingInstruction.target;
        pin->sub.ProcessingInstruction.data = ajStrNewC(data);
        /* AJB: dirty */
        pin->value = pin->sub.ProcessingInstruction.data;
    }

    return pin;
}




/* @func ajDomDocumentGetElementsByTagName ************************************
**
** Perform a pre-order traversal of the entire document. Return a node list
** of the elements matching tagname in the order in which they are found.
**
** @param [u] doc [AjPDomDocument] document
** @param [r] name [const AjPStr] name
** @return [AjPDomNodeList] node list
**
** @release 4.0.0
** @@
******************************************************************************/

AjPDomNodeList ajDomDocumentGetElementsByTagName(AjPDomDocument doc,
                                                 const AjPStr name)
{
    AjPDomNodeList list = NULL;

    if(doc && doc->type == ajEDomNodeTypeDocumentNode && name &&
       (list = ajDomCreateNodeList(doc)))
    {
        domTraverse(list, doc->sub.Document.documentelement, name);
        return list;
    }

    return NULL;
}




/* @func ajDomDocumentGetElementsByTagNameC ***********************************
**
** Perform a pre-order traversal of the entire document. Return a node list
** of the elements matching tagname in the order in which they are found.
**
** @param [u] doc [AjPDomDocument] document
** @param [r] name [const char *] name
** @return [AjPDomNodeList] node list
**
** @release 4.0.0
** @@
******************************************************************************/

AjPDomNodeList ajDomDocumentGetElementsByTagNameC(AjPDomDocument doc,
                                                  const char *name)
{
    AjPDomNodeList list = NULL;

    if(doc && doc->type == ajEDomNodeTypeDocumentNode && name &&
       (list = ajDomCreateNodeList(doc)))
    {
        domTraverseC(list, doc->sub.Document.documentelement, name);

        return list;
    }

    return NULL;
}




/* @func ajDomDocumentGetDoctype **********************************************
**
** Get document type
**
** @param [r] doc [const AjPDomDocument] document
** @return [AjPDomDocumentType] doctype
**
** @release 4.0.0
** @@
******************************************************************************/

AjPDomDocumentType ajDomDocumentGetDoctype(const AjPDomDocument doc)
{
    return doc ? doc->sub.Document.doctype : NULL;
}




/* @func ajDomDocumentGetDocumentElement **************************************
**
** Get the root element of the document tree. The root element is also
** accessible through the child nodes node list member, however the children
** of a document may also be processing instructions, document type nodes,
** and comments which may precede the document element in the list.
**
** @param [r] doc [const AjPDomDocument] document
** @return [AjPDomElement] root element
**
** @release 4.0.0
** @@
******************************************************************************/

AjPDomElement ajDomDocumentGetDocumentElement(const AjPDomDocument doc)
{
    return doc ? doc->sub.Document.documentelement : NULL;
}




/* @func ajDomPrintNode *******************************************************
**
** Print a node with indentation
**
** @param [r] node [const AjPDomNode] node
** @param [r] indent [ajint] indentation
** @return [void]
**
** @release 4.0.0
** @@
******************************************************************************/

void ajDomPrintNode(const AjPDomNode node, ajint indent)
{
    AjPDomNode n;
    ajint i;

    if(!node)
    {
        ajFmtPrint("Node was null\n");
        return;
    }

    for(i = 0; i < indent; ++i)
        ajFmtPrint("    ");

    ajFmtPrint("%s: %S=%S\n",
               domKNodeinfo[node->type], node->name, node->value);

    if(node->type == ajEDomNodeTypeElement && node->attributes->length)
    {
        ajFmtPrint("    ");
        n = ajDomNodeMapItem(node->attributes, 0);
        ajFmtPrint("%S=%S", n->name, n->value);

        for(i = 1; i < node->attributes->length; ++i)
        {
            n = ajDomNodeMapItem(node->attributes, i);
            ajFmtPrint(",%S=%S", n->name, n->value);
        }
        ajFmtPrint("\n");

        for(i = 0; i < indent; ++i)
            ajFmtPrint("    ");
    }

    for(n = node->firstchild; n; n = n->nextsibling)
        ajDomPrintNode(n, indent + 1);

    return;
}




/* @func ajDomPrintNode2 ******************************************************
**
** Print a node with zero indentation
**
** @param [r] node [const AjPDomNode] node
** @return [void]
**
** @release 4.0.0
** @@
******************************************************************************/

void ajDomPrintNode2(const AjPDomNode node)
{
    ajFmtPrint("\n");
    ajDomPrintNode(node, 0);

    return;
}




/* @func ajDomNodePrintNode ***************************************************
**
** Print a node internals
**
** @param [r] node [const AjPDomNode] node
** @return [void]
**
** @release 4.0.0
** @@
******************************************************************************/

void ajDomNodePrintNode(const AjPDomNode node)
{
    if(!node)
    {
        ajFmtPrint("Node was null\n");

        return;
    }

    ajFmtPrint("\nName=%S,Value=%S,", node->name, node->value);
    ajFmtPrint("\n\tType=%d", node->type);

    ajFmtPrint(",Parentnode->name=%S,Firstchild->name=%S",
               node->parentnode->name, node->firstchild->name);

    ajFmtPrint(",Lastchild->name=%S,\n\tChildnodes->length=%d",
               node->lastchild->name,
               (!node->childnodes ? 0 : node->childnodes->length));

    ajFmtPrint(",Previoussibling->name=%S,Nextsibling->name=%S,"
               "Attributes->length=%d\n",
               node->previoussibling->name, node->nextsibling->name,
               (!node->attributes ? 0 : node->attributes->length));

    return;
}




/* @func ajDomNodeInsertBefore ************************************************
**
** Insert newchild into node directly before the existing child refchild.
** If refchild is a null pointer, newchild will be appended to the list.
** If newchild is a fragment node, all children are moved into this node
** in the same order before refchild.
** If newchild is already in the list it will first be removed.
**
** @param [w] node [AjPDomNode] node
** @param [u] newchild [AjPDomNode] node to insert
** @param [u] refchild [AjPDomNode] node to insert before
** @return [AjPDomNode] inserted node
**
** @release 4.0.0
** @@
******************************************************************************/

AjPDomNode ajDomNodeInsertBefore(AjPDomNode node, AjPDomNode newchild,
                                 AjPDomNode refchild)
{
    AjPDomNode n = NULL;
    AjPDomNode nxt = NULL;

    if(!node || !newchild)
        return NULL;

    if(newchild->ownerdocument != node->ownerdocument &&
       newchild->ownerdocument != node)
    {
        ajDebug("ajDomNodeInsertBefore: Wrong document\n");

        return NULL;
    }

    if(refchild && refchild->parentnode != node)
    {
        ajDebug("ajDomNodeInsertBefore: Hierarchy error\n");

        return NULL;
    }

    if(newchild->type == ajEDomNodeTypeDocumentFragment)
    {
        for(n = newchild->firstchild; n; n = n->nextsibling)
            if(AJDOM_CANTDO(node, n) || domIsAncestor(n, node))
            {
                ajDebug("ajDomNodeInsertBefore: Hierarchy Request Error\n");

                return NULL;
            }

        for(n = newchild->firstchild; n; n = nxt)
        {
            nxt = n->nextsibling;
            if(!domDoRemoveChild(newchild, n))
                return NULL;

            if(!ajDomNodeInsertBefore(node, n, refchild))
            {
                ajDomDocumentDestroyNode(n->ownerdocument, n);
                return NULL;
            }
        }

        return newchild;
    }


    domDoRemoveChild(node, newchild);

    if(!(ajDomNodeListInsert(node->childnodes, newchild, refchild)))
        return NULL;

    if(!node->firstchild)
    {
        node->firstchild = newchild;
        node->lastchild  = newchild;
        newchild->previoussibling = NULL;
        newchild->nextsibling = NULL;
    }
    else if(!refchild)
    {
        newchild->previoussibling = node->lastchild;
        node->lastchild->nextsibling = newchild;
        node->lastchild = newchild;
        newchild->nextsibling = NULL;
    }
    else
    {
        newchild->previoussibling = refchild->previoussibling;
        newchild->nextsibling = refchild;
        if(refchild == node->firstchild)
        {
            node->firstchild = newchild;
            newchild->previoussibling = NULL;
        }
        else
            refchild->previoussibling->nextsibling = newchild;

        refchild->previoussibling = newchild;
    }

    newchild->parentnode = node;

    if(AJDOM_DOCMOD(node, newchild))
        node->sub.Document.documentelement = newchild;
    else if(AJDOM_DOCTYPEMOD(node, newchild))
    {
        node->sub.Document.doctype = newchild;
        newchild->ownerdocument = node;
    }

    domUpdateNode(node);

    return newchild;
}




/* @func ajDomNodeListInsert **************************************************
**
** Insert newchild into list directly before the existing child refchild.
**
** @param [u] list [AjPDomNodeList] list
** @param [u] newchild [AjPDomNode] node to insert
** @param [u] refchild [AjPDomNode] node to insert before
** @return [AjPDomNodeEntry] inserted node
**
** @release 4.0.0
** @@
******************************************************************************/

AjPDomNodeEntry ajDomNodeListInsert(AjPDomNodeList list, AjPDomNode newchild,
                                    AjPDomNode refchild)
{
    AjPDomNodeEntry e = NULL;
    AjPDomNodeEntry s = NULL;

    if(!list)
        return NULL;

    if(list->filter)
    {
        ajDebug("ajDomNodeListInsert: Filtered list error\n");

        return NULL;
    }

    if(refchild)
    {
        s = domDoLookupNode(list, refchild);
        if(!s || s->node != refchild)
        {
            ajDebug("ajDomNodeListInsert: not found error\n");

            return NULL;
        }
    }

    AJNEW0(e);

    domAddToMap(list, newchild, e);

    e->node = newchild;
    if(!list->length)
    {
        list->first = e;
        list->last = e;
    }
    else if(!refchild)
    {
        e->prev = list->last;
        list->last->next = e;
        list->last = e;
    }
    else
    {
        e->prev = s->prev;
        e->next = s;
        if(s == list->first)
            list->first = e;
        else
            s->prev->next = e;
        s->prev = e;
    }

    ++list->length;

    if(newchild->type == ajEDomNodeTypeAttribute)
        newchild->sub.Attr.ownerelement = list->ownerelement;

    return e;
}




/* @func ajDomNodeReplaceChild ************************************************
**
** Replace oldchild with newchild in the list of children.
**
** @param [u] node [AjPDomNode] node
** @param [u] newchild [AjPDomNode] node to insert
** @param [u] oldchild [AjPDomNode] node to replace
** @return [AjPDomNode] oldchild node
**
** @release 4.0.0
** @@
******************************************************************************/

AjPDomNode ajDomNodeReplaceChild(AjPDomNode node, AjPDomNode newchild,
                                 AjPDomNode oldchild)
{
    AjPDomNode nxt = NULL;
    AjPDomNode n   = NULL;

    if(!node || !newchild || !oldchild)
        return NULL;

    if(newchild->ownerdocument != node->ownerdocument &&
       newchild->ownerdocument != node)
    {
        ajDebug("ajDomNodeReplaceChild: Wrong document\n");

        return NULL;
    }

    if(!ajDomNodeListExists(node->childnodes, oldchild))
    {
        ajDebug("ajDomNodeReplaceChild: Oldchild not found\n");

        return NULL;
    }

    if(newchild->type == ajEDomNodeTypeDocumentFragment)
    {
        for(n = newchild->firstchild; n; n = n->nextsibling)
            if(AJDOM_CANTDO(node, n) || domIsAncestor(n, node))
            {
                ajDebug("ajDomNodeReplaceChild: Hierarchy Request Error\n");

                return NULL;
            }

        for(n = newchild->firstchild; n; n = nxt)
        {
            nxt = n->nextsibling;
            if(!domDoRemoveChild(newchild, n))
                return NULL;

            if(!ajDomNodeInsertBefore(node, n, oldchild))
            {
                ajDomDocumentDestroyNode(n->ownerdocument, n);

                return NULL;
            }
        }

        if(!domDoRemoveChild(node, oldchild))
            return NULL;

        return oldchild;
    }


    if(AJDOM_CANTDO(node, newchild) || domIsAncestor(newchild, node))
    {
        ajDebug("ajDomNodeReplaceChild: Hierarchy Request Error\n");
        return NULL;
    }

    domDoRemoveChild(node, newchild);

    if(!ajDomNodeListExists(node->childnodes, oldchild))
        return NULL;

    ajDomNodeListReplace(node->childnodes, newchild, oldchild);

    node->firstchild = node->childnodes->first->node;
    node->lastchild  = node->childnodes->last->node;

    if((newchild->previoussibling = oldchild->previoussibling))
        newchild->previoussibling->nextsibling = newchild;

    if((newchild->nextsibling = oldchild->nextsibling))
        newchild->nextsibling->previoussibling = newchild;

    newchild->parentnode = node;
    oldchild->parentnode = NULL;
    oldchild->previoussibling = NULL;
    oldchild->nextsibling = NULL;

    if(AJDOM_DOCMOD(node, newchild))
        node->sub.Document.documentelement = newchild;
    else if(AJDOM_DOCTYPEMOD(node, newchild))
    {
        node->sub.Document.doctype = newchild;
        newchild->ownerdocument = node;
    }

    domUpdateNode(node);

    return oldchild;
}




/* @func ajDomNodeListReplace *************************************************
**
** Replace oldchild with newchild in a node list.
**
** @param [u] list [AjPDomNodeList] list
** @param [u] newchild [AjPDomNode] node to insert
** @param [u] oldchild [AjPDomNode] node to replace
** @return [AjPDomNodeEntry] inserted node
**
** @release 4.0.0
** @@
******************************************************************************/

AjPDomNodeEntry ajDomNodeListReplace(AjPDomNodeList list, AjPDomNode newchild,
                                     AjPDomNode oldchild)
{
    AjPDomNodeEntry e = NULL;

    if(!list)
        return NULL;

    if(list->filter)
    {
        ajDebug("ajDomNodeListReplace: Filtered list error\n");

        return NULL;
    }

    e = domDoLookupNode(list, oldchild);
    if(!e)
    {
        ajDebug("ajDomNodeListReplace: Not found error\n");

        return NULL;
    }

    domRemoveFromMap(list, oldchild);
    domAddToMap(list, newchild, e);

    e->node = newchild;

    if(oldchild->type == ajEDomNodeTypeAttribute)
        oldchild->sub.Attr.ownerelement = NULL;

    return e;
}




/* @funcstatic domNodeCloneNode ***********************************************
**
** Low level clone node
**
** @param [u] ownerdocument [AjPDomDocument] owner document for clone
** @param [r] node [const AjPDomNode] node to clone
** @param [r] deep [AjBool] do a deep clone
** @return [AjPDomNode] clone
**
** @release 4.0.0
** @@
******************************************************************************/

static AjPDomNode domNodeCloneNode(AjPDomDocument ownerdocument,
                                   const AjPDomNode node, AjBool deep)
{
    AjPDomNode clone  = NULL;
    AjPDomNode ntmp   = NULL;
    AjPDomNode ctmp   = NULL;
    AjPDomNodeEntry e = NULL;


    switch(node->type)
    {
        case ajEDomNodeTypeElement:
            clone = ajDomDocumentCreateElement(ownerdocument, node->name);

            if(clone)
                for(e = node->attributes->first; e; e = e->next)
                    if(!(ctmp = domNodeCloneNode(ownerdocument, e->node, deep))
                       ||
                       !ajDomNodeListAppend(clone->attributes, ctmp))
                    {
                        ajDomDocumentDestroyNode(clone->ownerdocument, ctmp);
                        ajDomDocumentDestroyNode(clone->ownerdocument, clone);
                        return NULL;
                    }
            break;

        case ajEDomNodeTypeAttribute:
            if((clone = ajDomDocumentCreateAttribute(ownerdocument,
                                                     node->name)))
            {
                clone->sub.Attr.specified = node->sub.Attr.specified;

                if(!node->value)
                    return NULL;

                ajStrAssignS(&clone->value, node->value);
                /* AJB: dirty */
                clone->sub.Attr.value = clone->value;
            }

            break;

        case ajEDomNodeTypeComment:
            clone = ajDomDocumentCreateComment(ownerdocument, node->value);
            break;

        case ajEDomNodeTypeText:
            clone = ajDomDocumentCreateTextNode(ownerdocument, node->value);
            break;

        case ajEDomNodeTypeCdataSection:
            clone = ajDomDocumentCreateCDATASection(ownerdocument,
                                                    node->value);
            break;

        case ajEDomNodeTypeDocumentFragment:
            clone = ajDomDocumentCreateDocumentFragment(ownerdocument);
            break;

        case ajEDomNodeTypeDocumentNode:
            clone = ownerdocument;
            break;

        case ajEDomNodeTypeProcessingInstruction:
            clone = ajDomDocumentCreateProcessingInstruction(
                ownerdocument,
                node->sub.ProcessingInstruction.target,
                node->sub.ProcessingInstruction.data);
            break;

        case ajEDomNodeTypeEntityNode:
            if((clone = ajDomDocumentCreateNode(ownerdocument,
                                                ajEDomNodeTypeEntityNode)))
            {
                clone->name  = ajStrNewS(node->name);
                clone->value = ajStrNewS(node->value);

                if(node->sub.Entity.publicid)
                    clone->sub.Entity.publicid = ajStrNewS(node->sub.Entity.
                                                           publicid);
                if(node->sub.Entity.systemid)
                    clone->sub.Entity.systemid = ajStrNewS(node->sub.Entity.
                                                           systemid);
                if(node->sub.Entity.notationname)
                    clone->sub.Entity.notationname = ajStrNewS(node->sub.
                                                               Entity.
                                                               notationname);
            }

            break;

        case ajEDomNodeTypeNotation:
            if((clone = ajDomDocumentCreateNode(ownerdocument,
                                                ajEDomNodeTypeNotation)))
            {
                clone->name = ajStrNewS(node->name);

                if(node->sub.Notation.publicid)
                    clone->sub.Notation.publicid = ajStrNewS(node->sub.
                                                             Notation.
                                                             publicid);
                if(node->sub.Notation.systemid)
                    clone->sub.Notation.systemid = ajStrNewS(node->sub.
                                                             Notation.
                                                             systemid);
            }

            break;

        case ajEDomNodeTypeDocumentType:
            if((clone = ajDomImplementationCreateDocumentType(node->name,
                                                              NULL, NULL)))
            {
                if(node->sub.DocumentType.publicid)
                    clone->sub.DocumentType.publicid = ajStrNewS(node->sub.
                                                                 DocumentType.
                                                                 publicid);
                if(node->sub.DocumentType.systemid)
                    clone->sub.DocumentType.systemid = ajStrNewS(node->sub.
                                                                 DocumentType.
                                                                 systemid);
            }

            ownerdocument->sub.Document.doctype = clone;
            clone->ownerdocument = ownerdocument;
            break;

        case ajEDomNodeTypeEntityReference:
            ajDebug("Entity reference clone not implemented\n");
            return NULL;

        default:
            ajDebug("domNodeCloneNode got unexpected "
                    "AJAX DOM Node type %d.\n", node->type);
    }


    if(deep && clone && node->childnodes)
        for(ntmp = node->firstchild; ntmp; ntmp = ntmp->nextsibling)
        {
            ctmp = domNodeCloneNode(ownerdocument, ntmp, ajTrue);

            if(!ctmp || !ajDomNodeAppendChild(clone, ctmp))
            {
                ajDomDocumentDestroyNode(clone->ownerdocument, ctmp);
                ajDomDocumentDestroyNode(clone->ownerdocument, clone);
                return NULL;
            }
        }

    return clone;
}




/* @func ajDomNodeCloneNode ***************************************************
**
** Clone node
**
** @param [u] node [AjPDomNode] node to clone
** @param [r] deep [AjBool] do a deep clone
** @return [AjPDomNode] clone
**
** @release 4.0.0
** @@
******************************************************************************/

AjPDomNode ajDomNodeCloneNode(AjPDomNode node, AjBool deep)
{
    AjPDomDocument doc = NULL;

    if(!node)
        return NULL;

    if(node->type == ajEDomNodeTypeDocumentNode)
    {
        if(!(doc = ajDomImplementationCreateDocument(NULL, NULL, NULL)))
            return NULL;

        return domNodeCloneNode(doc, node, deep);
    }

    return domNodeCloneNode(node->ownerdocument, node, deep);
}




/* @funcstatic domWriteEncoded ************************************************
**
** Write a string using entity substitutions
**
** @param [r] s [const AjPStr] string
** @param [w] outf [AjPFile] output file
** @return [void]
**
** @release 4.0.0
** @@
******************************************************************************/

static void domWriteEncoded(const AjPStr s, AjPFile outf)
{
    size_t len;
    const char *p;
    AjPStr tmp = NULL;

    tmp = ajStrNew();

    p = ajStrGetPtr(s);

    while(*p)
    {
        len = strcspn(p, "<>&\"");
        if(len)
        {
            ajStrAssignSubC(&tmp, p, 0, len - 1);
            ajFmtPrintF(outf, "%S", tmp);
            p += len;
        }

        switch (*p)
        {
            case '\0':
                break;
            case '<':
                ajFmtPrintF(outf, "&lt;");
                break;
            case '>':
                ajFmtPrintF(outf, "&gt;");
                break;
            case '&':
                ajFmtPrintF(outf, "&apos;");
                break;
            case '"':
                ajFmtPrintF(outf, "&quot;");
                break;
            default:
                break;
        };

        if(*p)
            ++p;
    }

    ajStrDel(&tmp);

    return;
}




/* @func ajDomNodeHasChildNodes ***********************************************
**
** Check whether a node has any children
**
** @param [r] node [const AjPDomNode] node to clone
** @return [AjBool] true if children
**
** @release 4.0.0
** @@
******************************************************************************/

AjBool ajDomNodeHasChildNodes(const AjPDomNode node)
{
    return node != NULL && node->firstchild;
}




/* @func ajDomWrite ***********************************************************
**
** Write XML from memory
**
** @param [r] node [const AjPDomDocument] document to write
** @param [u] outf [AjPFile] output file
** @return [ajint] zero OK, negative if error
**
** @release 4.0.0
** @@
******************************************************************************/

ajint ajDomWrite(const AjPDomDocument node, AjPFile outf)
{
    AjPDomNodeEntry e = NULL;
    AjPDomNode c = NULL;

    if(!node || !outf)
        return -1;

    switch(node->type)
    {
        case ajEDomNodeTypeElement:
            ajFmtPrintF(outf, "<");
            ajFmtPrintF(outf, "%S", node->name);

            for(e = node->attributes->first; e; e = e->next)
            {
                ajFmtPrintF(outf, " %S=\"", e->node->name);
                domWriteEncoded(e->node->value, outf);
                ajFmtPrintF(outf, "\"");
            }

            if(ajDomNodeHasChildNodes(node))
            {
                ajFmtPrintF(outf, ">");

                for(c = node->firstchild; c; c = c->nextsibling)
                    if(ajDomWrite(c, outf) == -1)
                        return -1;

                ajFmtPrintF(outf, "</");
                ajFmtPrintF(outf, "%S", node->name);
                ajFmtPrintF(outf, ">");
            }
            else
                ajFmtPrintF(outf, "/>");

            break;

        case ajEDomNodeTypeAttribute:
            break;

        case ajEDomNodeTypeText:
            domWriteEncoded(node->value, outf);
            break;

        case ajEDomNodeTypeCdataSection:
            break;

        case ajEDomNodeTypeEntityReference:
            break;

        case ajEDomNodeTypeNotation:
            ajFmtPrintF(outf, "    <!NOTATION ");
            ajFmtPrintF(outf, "%S", node->name);

            if(node->sub.Notation.publicid)
            {
                ajFmtPrintF(outf, " PUBLIC \"");
                ajFmtPrintF(outf, "%S", node->sub.Notation.publicid);
                ajFmtPrintF(outf, "\" \"");
                ajFmtPrintF(outf, "%S", node->sub.Notation.systemid);
                ajFmtPrintF(outf, "\"");
            }
            else if(node->sub.Notation.systemid)
            {
                ajFmtPrintF(outf, " SYSTEM \"");
                ajFmtPrintF(outf, "%S", node->sub.Notation.systemid);
                ajFmtPrintF(outf, "\"");
            }

            ajFmtPrintF(outf, ">");
            break;

        case ajEDomNodeTypeEntityNode:
            ajFmtPrintF(outf, "    <!ENTITY ");
            ajFmtPrintF(outf, "%S", node->name);

            if(node->value)
            {
                ajFmtPrintF(outf, "\"");
                ajFmtPrintF(outf, "%S", node->value);
                ajFmtPrintF(outf, "\"");
            }
            else
            {
                if(node->sub.Entity.publicid)
                {
                    ajFmtPrintF(outf, " PUBLIC \"");
                    ajFmtPrintF(outf, "%S", node->sub.Entity.publicid);
                    ajFmtPrintF(outf, "\" \"");
                    ajFmtPrintF(outf, "%S", node->sub.Entity.systemid);
                    ajFmtPrintF(outf, "\"");
                }
                else if(node->sub.Entity.systemid)
                {
                    ajFmtPrintF(outf, " SYSTEM \"");
                    ajFmtPrintF(outf, "%S", node->sub.Entity.systemid);
                    ajFmtPrintF(outf, "\"");
                }

                if(node->sub.Entity.notationname)
                {
                    ajFmtPrintF(outf, " NDATA ");
                    ajFmtPrintF(outf, "%S", node->sub.Entity.notationname);
                }
            }

            ajFmtPrintF(outf, ">");
            break;

        case ajEDomNodeTypeProcessingInstruction:
            ajFmtPrintF(outf, "<?");
            ajFmtPrintF(outf, "%S ", node->sub.ProcessingInstruction.target);
            domWriteEncoded(node->sub.ProcessingInstruction.data, outf);
            ajFmtPrintF(outf, "?>");
            break;

        case ajEDomNodeTypeComment:
            ajFmtPrintF(outf, "<!--");
            domWriteEncoded(node->value, outf);
            ajFmtPrintF(outf, "-->");
            break;

        case ajEDomNodeTypeDocumentNode:
            ajFmtPrintF(outf, "<?xml");
            ajFmtPrintF(outf, " version=\"");

            if(node->sub.Document.version)
                ajFmtPrintF(outf, "%S", node->sub.Document.version);
            else
                ajFmtPrintF(outf, "1.0");

            ajFmtPrintF(outf, "\"");

            if(node->sub.Document.standalone)
                ajFmtPrintF(outf, " standalone=\"yes\"");

            ajFmtPrintF(outf, "?>\n");

            for(c=node->firstchild; c; c=c->nextsibling)
                if(ajDomWrite(c, outf) == -1)
                    return -1;

            ajFmtPrintF(outf, "\n");
            break;

        case ajEDomNodeTypeDocumentType:
            ajFmtPrintF(outf, "\n<!DOCTYPE ");
            ajFmtPrintF(outf, "%S", node->sub.DocumentType.name);

            if(node->sub.DocumentType.systemid)
            {
                ajFmtPrintF(outf, " SYSTEM \"");
                ajFmtPrintF(outf, "%S", node->sub.DocumentType.systemid);
                ajFmtPrintF(outf, "\"");
            }
            else if(node->sub.DocumentType.publicid)
            {
                ajFmtPrintF(outf, " PUBLIC \"");
                ajFmtPrintF(outf, "%S", node->sub.DocumentType.publicid);
                ajFmtPrintF(outf, "\"");
            }

            if(node->sub.DocumentType.internalsubset)
            {
                ajFmtPrintF(outf, " [\n");
                ajFmtPrintF(outf, "%S", node->sub.DocumentType.internalsubset);
                ajFmtPrintF(outf, "]>\n");
            }
            else
                ajFmtPrintF(outf, ">\n");

            break;

        case ajEDomNodeTypeDocumentFragment:
            break;

        default:
            ajDebug("ajDomWrite got unexpected "
                    "AJAX DOM Node type %d.\n", node->type);
    }

    return 0;
}




/* @func ajDomWriteIndent *****************************************************
**
** Write XML from memory
**
** @param [r] node [const AjPDomDocument] document to write
** @param [u] outf [AjPFile] output file
** @param [r] indent [ajint] indent level
** @return [ajint] zero OK, negative if error
**
** @release 4.1.0
** @@
******************************************************************************/

ajint ajDomWriteIndent(const AjPDomDocument node, AjPFile outf, ajint indent)
{
    AjPDomNodeEntry e = NULL;
    AjPDomNode c = NULL;
    ajint i;

    if(!node || !outf)
        return -1;

    for(i = 0; i < indent; ++i)
        ajFmtPrintF(outf, " ");

    switch(node->type)
    {
        case ajEDomNodeTypeElement:
            ajFmtPrintF(outf, "<");
            ajFmtPrintF(outf, "%S", node->name);

            for(e = node->attributes->first; e; e = e->next)
            {
                ajFmtPrintF(outf, " %S=\"", e->node->name);
                domWriteEncoded(e->node->value, outf);
                ajFmtPrintF(outf, "\"");
            }

            if(ajDomNodeHasChildNodes(node))
            {
                ajFmtPrintF(outf, ">\n");

                for(c = node->firstchild; c; c = c->nextsibling)
                    if(ajDomWriteIndent(c, outf, indent + 2) == -1)
                        return -1;

                for(i = 0; i < indent; ++i)
                    ajFmtPrintF(outf, " ");

                ajFmtPrintF(outf, "</");
                ajFmtPrintF(outf, "%S", node->name);
                ajFmtPrintF(outf, ">\n");
            }
            else
                ajFmtPrintF(outf, "/>\n");

            break;

        case ajEDomNodeTypeAttribute:
            break;

        case ajEDomNodeTypeText:
            domWriteEncoded(node->value, outf);
            break;

        case ajEDomNodeTypeCdataSection:
            break;

        case ajEDomNodeTypeEntityReference:
            break;

        case ajEDomNodeTypeNotation:
            ajFmtPrintF(outf, "    <!NOTATION ");
            ajFmtPrintF(outf, "%S", node->name);

            if(node->sub.Notation.publicid)
            {
                ajFmtPrintF(outf, " PUBLIC \"");
                ajFmtPrintF(outf, "%S", node->sub.Notation.publicid);
                ajFmtPrintF(outf, "\" \"");
                ajFmtPrintF(outf, "%S", node->sub.Notation.systemid);
                ajFmtPrintF(outf, "\"");
            }
            else if(node->sub.Notation.systemid)
            {
                ajFmtPrintF(outf, " SYSTEM \"");
                ajFmtPrintF(outf, "%S", node->sub.Notation.systemid);
                ajFmtPrintF(outf, "\"");
            }

            ajFmtPrintF(outf, ">\n");
            break;

        case ajEDomNodeTypeEntityNode:
            ajFmtPrintF(outf, "    <!ENTITY ");
            ajFmtPrintF(outf, "%S", node->name);

            if(node->value)
            {
                ajFmtPrintF(outf, "\"");
                ajFmtPrintF(outf, "%S", node->value);
                ajFmtPrintF(outf, "\"");
            }
            else
            {
                if(node->sub.Entity.publicid)
                {
                    ajFmtPrintF(outf, " PUBLIC \"");
                    ajFmtPrintF(outf, "%S", node->sub.Entity.publicid);
                    ajFmtPrintF(outf, "\" \"");
                    ajFmtPrintF(outf, "%S", node->sub.Entity.systemid);
                    ajFmtPrintF(outf, "\"");
                }
                else if(node->sub.Entity.systemid)
                {
                    ajFmtPrintF(outf, " SYSTEM \"");
                    ajFmtPrintF(outf, "%S", node->sub.Entity.systemid);
                    ajFmtPrintF(outf, "\"");
                }

                if(node->sub.Entity.notationname)
                {
                    ajFmtPrintF(outf, " NDATA ");
                    ajFmtPrintF(outf, "%S", node->sub.Entity.notationname);
                }
            }

            ajFmtPrintF(outf, ">\n");
            break;

        case ajEDomNodeTypeProcessingInstruction:
            ajFmtPrintF(outf, "<?");
            ajFmtPrintF(outf, "%S ", node->sub.ProcessingInstruction.target);
            domWriteEncoded(node->sub.ProcessingInstruction.data, outf);
            ajFmtPrintF(outf, "?>\n");
            break;

        case ajEDomNodeTypeComment:
            ajFmtPrintF(outf, "<!--");
            domWriteEncoded(node->value, outf);
            ajFmtPrintF(outf, "-->\n");
            break;

        case ajEDomNodeTypeDocumentNode:
            ajFmtPrintF(outf, "<?xml");
            ajFmtPrintF(outf, " version=\"");

            if(node->sub.Document.version)
                ajFmtPrintF(outf, "%S", node->sub.Document.version);
            else
                ajFmtPrintF(outf, "1.0");

            ajFmtPrintF(outf, "\"");

            if(node->sub.Document.standalone)
                ajFmtPrintF(outf, " standalone=\"yes\"");

            ajFmtPrintF(outf, "?>\n");

            for(c = node->firstchild; c; c = c->nextsibling)
                if(ajDomWriteIndent(c, outf, indent + 2) == -1)
                    return -1;

            ajFmtPrintF(outf, "\n");
            break;

        case ajEDomNodeTypeDocumentType:
            ajFmtPrintF(outf, "\n<!DOCTYPE ");
            ajFmtPrintF(outf, "%S", node->sub.DocumentType.name);

            if(node->sub.DocumentType.systemid)
            {
                ajFmtPrintF(outf, " SYSTEM \"");
                ajFmtPrintF(outf, "%S", node->sub.DocumentType.systemid);
                ajFmtPrintF(outf, "\"");
            }
            else if(node->sub.DocumentType.publicid)
            {
                ajFmtPrintF(outf, " PUBLIC \"");
                ajFmtPrintF(outf, "%S", node->sub.DocumentType.publicid);
                ajFmtPrintF(outf, "\"");
            }

            if(node->sub.DocumentType.internalsubset)
            {
                ajFmtPrintF(outf, " [\n");
                ajFmtPrintF(outf, "%S", node->sub.DocumentType.internalsubset);
                ajFmtPrintF(outf, "]>\n");
            }
            else
                ajFmtPrintF(outf, ">\n");

            break;

        case ajEDomNodeTypeDocumentFragment:
            break;

        default:
            ajDebug("ajDomWriteIndent got unexpected "
                    "AJAX DOM Node type %d.\n", node->type);
    }

    return 0;
}




/* @funcstatic domUserdataNew *************************************************
**
** Create userdata object
**
** @return [AjPDomUserdata] DOM Userdata for expat XML reading
**
** @release 6.3.0
** @@
******************************************************************************/

static AjPDomUserdata domUserdataNew(void)
{
    AjPDomUserdata ret = NULL;

    AJNEW0(ret);

    ret->Buffer = ajStrNew();
    ret->Stack  = ajListNew();
    ret->Cdata  = ajFalse;

    return ret;
}




/* @funcstatic domUserdataDel *************************************************
**
** Destroy userdata object
**
** @param [d] thys [AjPDomUserdata*] Userdata object pointer
**
** @return [void]
**
** @release 6.3.0
** @@
******************************************************************************/

static void domUserdataDel(AjPDomUserdata *thys)
{
    AjPDomUserdata pthis = NULL;

    if(!thys)
        return;

    pthis = *thys;

    if(!pthis)
        return;

    ajStrDel(&pthis->Buffer);
    ajListFree(&pthis->Stack);

    AJFREE(pthis);

    *thys = NULL;

    return;
}




/* @funcstatic domExpatStart **************************************************
**
** XML reading Expat start function
**
** @param [u] udata [void*] Userdata pointer
** @param [r] name [const XML_Char*] Name
** @param [r] atts [const XML_Char**] Attributes
**
** @return [void]
**
** @release 6.3.0
** @@
******************************************************************************/

static void domExpatStart(void *udata, const XML_Char *name,
                          const XML_Char **atts)
{
    AjPDomUserdata userdata = NULL;
    AjPDomDocument parent   = NULL;
    AjPDomDocument child    = NULL;
    ajint i = 0;
    AjPStr sname = NULL;


    if(!udata || !name || !atts)
        return;

    userdata = (AjPDomUserdata) udata;

    ajListPeek(userdata->Stack, (void **) &parent);

    if(!parent)
        return;

    ajStrAssignC(&userdata->Buffer, name);

    if(!(child = ajDomDocumentCreateElement(parent->ownerdocument,
                                            userdata->Buffer)))
        return;

    if(!ajDomNodeAppendChild(parent, child))
        return;

    sname = ajStrNew();

    for(i = 0; atts[i]; i += 2)
    {
        ajStrAssignC(&sname, atts[i]);
        ajStrAssignC(&userdata->Buffer, atts[i + 1]);

        ajDomElementSetAttribute(child, sname, userdata->Buffer);
    }

    ajListPush(userdata->Stack, (void *) child);

    ajStrDel(&sname);

    return;
}




/* @funcstatic domExpatEnd ****************************************************
**
** XML reading Expat end function
**
** @param [u] udata [void*] Userdata pointer
** @param [r] name [const XML_Char*] Name
**
** @return [void]
**
** @release 6.3.0
** @@
******************************************************************************/

static void domExpatEnd(void *udata, const XML_Char *name)
{
    AjPDomUserdata userdata = NULL;
    AjPDomDocument node = NULL;

    userdata = (AjPDomUserdata) udata;

    ajListPop(userdata->Stack, (void **)&node);

    (void) name;

    return;
}




/* @funcstatic domExpatChardata ***********************************************
**
** XML reading Expat chardata function
**
** @param [u] udata [void*] Userdata pointer
** @param [r] str [const XML_Char*] Char data
** @param [r] len [int] Length
**
** @return [void]
**
** @release 6.3.0
** @@
******************************************************************************/

static void domExpatChardata(void *udata, const XML_Char *str,
                             int len)
{
    AjPDomUserdata userdata = NULL;
    AjPDomDocument parent   = NULL;
    AjPDomText txt = NULL;

    if(!udata || !str || !len)
        return;

    /*
    ** Care needs to be exercised with this function.
    ** First, it may need several callbacks to recover long strings.
    ** The operation might need to be converted to an append
    ** and only create nodes if the strings are complete
    **
    ** Its main use is to maintain the whitepace formatting  in
    ** the original XML file and could, in many cases, be
    ** ignored.
    */

    userdata = (AjPDomUserdata) udata;

    ajListPeek(userdata->Stack, (void **) &parent);

    if(!parent)
        return;

    ajStrAssignLenC(&userdata->Buffer, str, len);

    if(userdata->Cdata)
    {
        if(!(txt = ajDomDocumentCreateCDATASection(parent->ownerdocument,
                                                   userdata->Buffer)))
            return;
    }
    else
    {
        if(!(txt = ajDomDocumentCreateTextNode(parent->ownerdocument,
                                               userdata->Buffer)))
            return;
    }


    ajDomNodeAppendChild(parent, txt);

    return;
}




/* @funcstatic domExpatCdataStart *********************************************
**
** XML reading Expat CDATA start function
**
** @param [u] udata [void*] Userdata pointer
**
** @return [void]
**
** @release 6.3.0
** @@
******************************************************************************/

static void domExpatCdataStart(void *udata)
{
    AjPDomUserdata userdata = NULL;

    if(!udata)
        return;

    userdata = (AjPDomUserdata) udata;

    userdata->Cdata = ajTrue;

    return;
}




/* @funcstatic domExpatCdataEnd ***********************************************
**
** XML reading Expat CDATA end function
**
** @param [u] udata [void*] Userdata pointer
**
** @return [void]
**
** @release 6.3.0
** @@
******************************************************************************/

static void domExpatCdataEnd(void *udata)
{
    AjPDomUserdata userdata = NULL;

    if(!udata)
        return;

    userdata = (AjPDomUserdata) udata;

    userdata->Cdata = ajFalse;

    return;
}




/* @funcstatic domExpatComment ************************************************
**
** XML reading Expat comment function
**
** @param [u] udata [void*] Userdata pointer
** @param [r] str [const XML_Char*] Char data
**
** @return [void]
**
** @release 6.3.0
** @@
******************************************************************************/

static void domExpatComment(void *udata, const XML_Char *str)
{
    AjPDomUserdata userdata = NULL;
    AjPDomDocument parent   = NULL;
    AjPDomComment comment = NULL;

    if(!udata || !str)
        return;

    userdata = (AjPDomUserdata) udata;

    ajListPeek(userdata->Stack, (void **) &parent);

    if(!parent)
        return;

    ajStrAssignC(&userdata->Buffer, str);

    if((comment = ajDomDocumentCreateComment(parent->ownerdocument,
                                             userdata->Buffer)))
        ajDomNodeAppendChild(parent, comment);

    return;
}




/* @funcstatic domExpatProcessing *********************************************
**
** XML reading Expat processing instruction function
**
** @param [u] udata [void*] Userdata pointer
** @param [r] target [const XML_Char*] Char data
** @param [r] str [const XML_Char*] Char data
**
** @return [void]
**
** @release 6.3.0
** @@
******************************************************************************/

static void domExpatProcessing(void *udata, const XML_Char *target,
                               const XML_Char *str)
{
    AjPDomUserdata userdata = NULL;
    AjPDomDocument parent   = NULL;
    AjPDomPi pi = NULL;
    AjPStr t = NULL;

    if(!udata || !target || !!str)
        return;

    userdata = (AjPDomUserdata) udata;

    ajListPeek(userdata->Stack, (void **) &parent);

    if(!parent)
        return;

    t = ajStrNew();

    ajStrAssignC(&t, target);
    ajStrAssignC(&userdata->Buffer, str);

    if((pi = ajDomDocumentCreateProcessingInstruction(parent->ownerdocument,
                                                      t, userdata->Buffer)))
        ajDomNodeAppendChild(parent, pi);

    ajStrDel(&t);

    return;
}




/* @funcstatic domExpatXmlDecl ************************************************
**
** XML reading Expat XML declaration function
**
** @param [u] udata [void*] Userdata pointer
** @param [r] version [const XML_Char*] Version
** @param [r] encoding [const XML_Char*] Encoding
** @param [r] standalone [int] Standalone
**
** @return [void]
**
** @release 6.3.0
** @@
******************************************************************************/

static void domExpatXmlDecl(void *udata, const XML_Char *version,
                            const XML_Char *encoding, int standalone)
{
    AjPDomUserdata userdata = NULL;
    AjPDomDocument document   = NULL;

    if(!udata)
        return;

    userdata = (AjPDomUserdata) udata;

    ajListPeek(userdata->Stack, (void **) &document);

    if(!document)
        return;

    if(!document->sub.Document.version)
        document->sub.Document.version = ajStrNew();

    if(!document->sub.Document.encoding)
        document->sub.Document.encoding = ajStrNew();

    if(version)
        ajStrAssignC(&document->sub.Document.version, version);

    if(encoding)
        ajStrAssignC(&document->sub.Document.encoding, encoding);

    document->sub.Document.standalone = standalone;

    return;
}




/* @funcstatic domExpatDoctypeStart *******************************************
**
** XML reading Expat Doctype start function
**
** @param [u] udata [void*] Userdata pointer
** @param [r] doctypename [const XML_Char*] Doctype name
** @param [r] sysid [const XML_Char*] Sysid
** @param [r] pubid [const XML_Char*] Sysid
** @param [r] hasinternalsubset [int] Internal subset flag
**
** @return [void]
**
** @release 6.3.0
** @@
******************************************************************************/

static void domExpatDoctypeStart(void *udata, const XML_Char *doctypename,
                                 const XML_Char *sysid, const XML_Char *pubid,
                                 int hasinternalsubset)
{
    AjPDomUserdata userdata = NULL;
    AjPDomDocument doc      = NULL;
    AjPDomDocumentType type = NULL;

    (void) hasinternalsubset;


    if(!udata)
        return;

    userdata = (AjPDomUserdata) udata;

    if(!ajListPeek(userdata->Stack, (void **) &doc))
        return;

    if(!doc)
        return;

    if(doc->sub.Document.doctype)
    {
        ajDebug("domExpatDoctypeStart: doctype already exists");
        return;
    }

    ajStrAssignC(&userdata->Buffer, doctypename);

    if(!(type = ajDomImplementationCreateDocumentType(userdata->Buffer, NULL,
                                                      NULL)))
        return;

    if(sysid)
        ajStrAssignC(&type->sub.DocumentType.systemid, sysid);

    if(pubid)
        ajStrAssignC(&type->sub.DocumentType.publicid, pubid);

    if(!ajDomNodeAppendChild(doc, type))
        return;

    doc->sub.Document.doctype = type;

    ajListPush(userdata->Stack, (void *) type);

    return;
}




/* @funcstatic domExpatDoctypeEnd *********************************************
**
** XML reading Expat Doctype end function
**
** @param [u] udata [void*] Userdata pointer
**
** @return [void]
**
** @release 6.3.0
** @@
******************************************************************************/

static void domExpatDoctypeEnd(void *udata)
{
    AjPDomUserdata userdata = NULL;
    AjPDomDocumentType type = NULL;

    if(!udata)
        return;

    userdata = (AjPDomUserdata) udata;

    ajListPop(userdata->Stack, (void **)&type);

    return;
}




/* @funcstatic domExpatElement ************************************************
**
** XML reading Expat Element function
**
** @param [u] udata [void*] Userdata pointer
** @param [r] name [const XML_Char*] Name
** @param [d] model [XML_Content*] Model
**
** @return [void]
**
** @release 6.3.0
** @@
******************************************************************************/

static void domExpatElement(void *udata, const XML_Char *name,
                            XML_Content *model)
{
    udata = NULL;
    name  = NULL;

    (void) udata;
    (void) name;

    free(model);

    return;
}




/* @funcstatic domExpatAttlist ************************************************
**
** XML reading Expat attribute list function
**
** @param [u] udata [void*] Userdata pointer
** @param [r] name [const XML_Char*] Element name
** @param [r] attname [const XML_Char*] Attribute Name
** @param [r] atttype [const XML_Char*] Attribute Type
** @param [r] deflt [const XML_Char*] Default
** @param [r] isrequired [int] Required flag
**
** @return [void]
**
** @release 6.3.0
** @@
******************************************************************************/

static void domExpatAttlist(void *udata, const XML_Char *name,
                            const XML_Char *attname, const XML_Char *atttype,
                            const XML_Char *deflt, int isrequired)
{
    (void) udata;
    (void) name;
    (void) attname;
    (void) atttype;
    (void) deflt;
    (void) isrequired;

    return;
}




/* @funcstatic domExpatEntity *************************************************
**
** XML reading Expat entity function
**
** @param [u] udata [void*] Userdata pointer
** @param [r] entityname [const XML_Char*] Entity name
** @param [r] isparam [int] Flag for parameter entity
** @param [r] value [const XML_Char*] Value
** @param [r] lenval [int] Value length
** @param [r] base [const XML_Char*] Base
** @param [r] systemid [const XML_Char*] System ID
** @param [r] publicid [const XML_Char*] Public ID
** @param [r] notname [const XML_Char*] Notation name
**
** @return [void]
**
** @release 6.3.0
** @@
******************************************************************************/

static void domExpatEntity(void *udata,
                           const XML_Char *entityname, int isparam,
                           const XML_Char *value, int lenval,
                           const XML_Char *base, const XML_Char *systemid,
                           const XML_Char *publicid, const XML_Char *notname)
{
    AjPDomUserdata userdata = NULL;
    AjPDomDocumentType doctype = NULL;
    AjPDomEntity entity = NULL;

    (void) base;

    if(isparam)
        return;

    if(!udata)
        return;

    userdata = (AjPDomUserdata) udata;

    ajListPop(userdata->Stack, (void **)&doctype);

    if(!(entity = ajDomDocumentCreateNode(doctype->ownerdocument,
                                          ajEDomNodeTypeEntityNode)))
        return;

    ajStrAssignC(&entity->name, entityname);

    if(value)
        ajStrAssignLenC(&entity->value, value, lenval);

    if(publicid)
        ajStrAssignC(&entity->sub.Entity.publicid, publicid);

    if(systemid)
        ajStrAssignC(&entity->sub.Entity.systemid, systemid);

    if(notname)
        ajStrAssignC(&entity->sub.Entity.notationname, notname);

    ajDomNodeAppendChild(doctype, entity);

    return;
}




/* @funcstatic domExpatNotation ***********************************************
**
** XML reading Expat notation function
**
** @param [u] udata [void*] Userdata pointer
** @param [r] notname [const XML_Char*] Entity name
** @param [r] base [const XML_Char*] Base
** @param [r] systemid [const XML_Char*] System ID
** @param [r] publicid [const XML_Char*] Public ID
**
** @return [void]
**
** @release 6.3.0
** @@
******************************************************************************/

static void domExpatNotation(void *udata, const XML_Char *notname,
                             const XML_Char *base, const XML_Char *systemid,
                             const XML_Char *publicid)
{
    AjPDomUserdata userdata = NULL;
    AjPDomDocumentType doctype = NULL;
    AjPDomNotation notation = NULL;

    (void) base;

    if(!udata)
        return;

    userdata = (AjPDomUserdata) udata;

    ajListPop(userdata->Stack, (void **)&doctype);

    if(!(notation = ajDomDocumentCreateNode(doctype->ownerdocument,
                                            ajEDomNodeTypeNotation)))
        return;

    ajStrAssignC(&notation->name, notname);

    if(publicid)
        ajStrAssignC(&notation->sub.Notation.publicid, publicid);

    if(systemid)
        ajStrAssignC(&notation->sub.Notation.systemid, systemid);

    ajDomNodeAppendChild(doctype, notation);

    return;
}




/* @func ajDomReadFp **********************************************************
**
** Read XML into memory from a file pointer
**
** @param [u] node [AjPDomDocument] document to write
** @param [u] stream [FILE*] stream
** @return [ajint] zero OK, negative if error
**
** @release 6.3.0
** @@
******************************************************************************/

ajint ajDomReadFp(AjPDomDocument node, FILE *stream)
{
    AjPDomUserdata userdata = NULL;
    XML_Parser parser = NULL;
    void *buf = NULL;
    ajlong n = 0L;
    ajint ret;
    ajint done;

    if(!node || !stream)
        return -1;

    parser = XML_ParserCreate(NULL);
    if(!parser)
        return -1;



    userdata = domUserdataNew();

    node->ownerdocument = node;

    ajListPush(userdata->Stack, (void *) node);

    XML_SetElementHandler(parser, domExpatStart, domExpatEnd);
    XML_SetCharacterDataHandler(parser, domExpatChardata);
    XML_SetCdataSectionHandler(parser, domExpatCdataStart, domExpatCdataEnd);
    XML_SetCommentHandler(parser, domExpatComment);
    XML_SetProcessingInstructionHandler(parser, domExpatProcessing);
    XML_SetXmlDeclHandler(parser, domExpatXmlDecl);
    XML_SetDoctypeDeclHandler(parser, domExpatDoctypeStart,
                              domExpatDoctypeEnd);
    XML_SetElementDeclHandler(parser, domExpatElement);
    XML_SetAttlistDeclHandler(parser, domExpatAttlist);
    XML_SetEntityDeclHandler(parser, domExpatEntity);
    XML_SetNotationDeclHandler(parser, domExpatNotation);
    XML_SetUserData(parser, userdata);

    ret = -1;
    for(;;)
    {
        if(!(buf = XML_GetBuffer(parser, AJXML_BUFSIZ)))
            break;

        if(!(n = fread(buf, 1, AJXML_BUFSIZ, stream)) && ferror(stream))
            break;

        if(!XML_ParseBuffer(parser, (int) n , (done = feof(stream))))
        {
            ajDebug("ajDomReadFp: Expat error [%s] at XML line %d",
                    XML_ErrorString(XML_GetErrorCode(parser)),
                    XML_GetCurrentLineNumber(parser));
            break;
        }

        if(done)
        {
            ret = 0;
            break;
        }
    }

    ajDomElementNormalise(node->sub.Document.documentelement);

    domUserdataDel(&userdata);

    XML_ParserFree(parser);

    return ret;
}




/* @func ajDomReadFilebuff ****************************************************
**
** Read XML into memory from a file pointer
**
** @param [u] node [AjPDomDocument] document to write
** @param [u] buff [AjPFilebuff] File buffer
** @return [ajint] zero OK, negative if error
**
** @release 6.3.0
** @@
******************************************************************************/

ajint ajDomReadFilebuff(AjPDomDocument node, AjPFilebuff buff)
{
    AjPDomUserdata userdata = NULL;
    XML_Parser parser = NULL;
    int done = 0;
    ajint len = 0;
    AjPStr line = NULL;

    parser = XML_ParserCreate(NULL);
    if(!parser)
        return -1;

    userdata = domUserdataNew();

    node->ownerdocument = node;

    ajListPush(userdata->Stack, (void *) node);

    XML_SetElementHandler(parser, domExpatStart, domExpatEnd);
    XML_SetCharacterDataHandler(parser, domExpatChardata);
    XML_SetCdataSectionHandler(parser, domExpatCdataStart, domExpatCdataEnd);
    XML_SetCommentHandler(parser, domExpatComment);
    XML_SetProcessingInstructionHandler(parser, domExpatProcessing);
    XML_SetXmlDeclHandler(parser, domExpatXmlDecl);
    XML_SetDoctypeDeclHandler(parser, domExpatDoctypeStart,
                              domExpatDoctypeEnd);
    XML_SetElementDeclHandler(parser, domExpatElement);
    XML_SetAttlistDeclHandler(parser, domExpatAttlist);
    XML_SetEntityDeclHandler(parser, domExpatEntity);
    XML_SetNotationDeclHandler(parser, domExpatNotation);
    XML_SetUserData(parser, userdata);

    line = ajStrNew();

    do
    {
        ajBuffreadLine(buff, &line);
        done = ajFilebuffIsEmpty(buff);
        len = ajStrGetLen(line);

        if(!XML_Parse(parser, line->Ptr, len, done))
        {
            ajDebug("ajDomReadFilebuff: %s at XML line %d\n",
                    XML_ErrorString(XML_GetErrorCode(parser)),
                    XML_GetCurrentLineNumber(parser));

            domUserdataDel(&userdata);
            XML_ParserFree(parser);
            ajStrDel(&line);

            return -1;
        }

    } while (!done);

    ajDomElementNormalise(node->sub.Document.documentelement);

    domUserdataDel(&userdata);

    XML_ParserFree(parser);

    ajStrDel(&line);

    return 0;
}




/* @func ajDomReadString ******************************************************
**
** Read XML into memory from a string
**
** @param [u] node [AjPDomDocument] document to write
** @param [u] str [AjPStr] XML string
** @return [ajint] zero OK, negative if error
**
** @release 6.3.0
** @@
******************************************************************************/

ajint ajDomReadString(AjPDomDocument node, AjPStr str)
{
    AjPDomUserdata userdata = NULL;
    XML_Parser parser = NULL;
    int done = 0;
    ajint len = 0;

    parser = XML_ParserCreate(NULL);
    if(!parser)
        return -1;

    userdata = domUserdataNew();

    node->ownerdocument = node;

    ajListPush(userdata->Stack, (void *) node);

    XML_SetElementHandler(parser, domExpatStart, domExpatEnd);
    XML_SetCharacterDataHandler(parser, domExpatChardata);
    XML_SetCdataSectionHandler(parser, domExpatCdataStart, domExpatCdataEnd);
    XML_SetCommentHandler(parser, domExpatComment);
    XML_SetProcessingInstructionHandler(parser, domExpatProcessing);
    XML_SetXmlDeclHandler(parser, domExpatXmlDecl);
    XML_SetDoctypeDeclHandler(parser, domExpatDoctypeStart,
                              domExpatDoctypeEnd);
    XML_SetElementDeclHandler(parser, domExpatElement);
    XML_SetAttlistDeclHandler(parser, domExpatAttlist);
    XML_SetEntityDeclHandler(parser, domExpatEntity);
    XML_SetNotationDeclHandler(parser, domExpatNotation);
    XML_SetUserData(parser, userdata);

    done = 1;

    len = ajStrGetLen(str);

    if(!XML_Parse(parser, str->Ptr, len, done))
    {
        ajDebug("ajDomReadString: %s at XML line %d\n",
                XML_ErrorString(XML_GetErrorCode(parser)),
                XML_GetCurrentLineNumber(parser));

        return -1;
    }

    ajDomElementNormalise(node->sub.Document.documentelement);

    domUserdataDel(&userdata);

    XML_ParserFree(parser);

    return 0;
}




/* @func ajDomTextGetText *****************************************************
**
** Return the text from a text node
**
** @param [w] text [AjPDomText] text
** @return [AjPStr] text or NULL
**
** @release 6.4.0
** @@
******************************************************************************/

AjPStr ajDomTextGetText(AjPDomText text)
{
    if(!text)
        return NULL;

    if(text->type != ajEDomNodeTypeText)
        return NULL;

    return text->value;
}




/* @func ajDomElementGetText **************************************************
**
** Return the text from an element node
**
** @param [w] element [AjPDomElement] element
** @return [AjPStr] text or NULL
**
** @release 6.4.0
** @@
******************************************************************************/

AjPStr ajDomElementGetText(AjPDomElement element)
{
    if(!element)
        return NULL;

    if(element->type != ajEDomNodeTypeElement)
        return NULL;

    if(!element->firstchild)
        return NULL;

    return ajDomTextGetText(element->firstchild);
}




/* @func ajDomElementGetNthChildByTagNameC ************************************
**
** Returns the nth child of a named element
**
** @param [u] doc [AjPDomDocument] doc
** @param [u] element [AjPDomElement] element
** @param [r] name [const char *] element name
** @param [r] n [ajint] number of child to get
** @return [AjPDomElement] child or NULL
**
** @release 6.4.0
** @@
******************************************************************************/

AjPDomElement ajDomElementGetNthChildByTagNameC(AjPDomDocument doc,
                                                AjPDomElement element,
                                                const char *name,
                                                ajint n)
{
    AjPDomNodeList list;
    ajint len;
    AjPDomElement ret = NULL;

    list = ajDomElementGetElementsByTagNameC(element, name);

    if(!list)
        return NULL;

    len = ajDomNodeListGetLen(list);

    if(!len)
    {
        ajDomDocumentDestroyNodeList(doc, list, AJDOMKEEP);
        return NULL;
    }

    if(n < 0 || n > len - 1)
    {
        ajDebug("ajDomElementgetNthChildByTagnameC: index out of range");
        ajDomDocumentDestroyNodeList(doc, list, AJDOMKEEP);
        return NULL;
    }

    ret = ajDomNodeListItem(list, n);

    ajDomDocumentDestroyNodeList(doc, list, AJDOMKEEP);

    return ret;
}




/* @func ajDomElementGetFirstChildByTagNameC **********************************
**
** Returns the 1st child of a named element
**
** @param [u] doc [AjPDomDocument] doc
** @param [u] element [AjPDomElement] element
** @param [r] name [const char*] element name
** @return [AjPDomElement] child or NULL
**
** @release 6.4.0
** @@
******************************************************************************/

AjPDomElement ajDomElementGetFirstChildByTagNameC(AjPDomDocument doc,
                                                  AjPDomElement element,
                                                  const char *name)
{

    return ajDomElementGetNthChildByTagNameC(doc, element, name, 0);
}
