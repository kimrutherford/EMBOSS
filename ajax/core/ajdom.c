/******************************************************************************
** @source AJAX DOM functions
**
** @author Copyright (C) 2006 Alan Bleasby
** @version 1.0
** @modified Jul 03 2006 ajb First version
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

#include "ajax.h"

#define AJDOMDESTROY 1
#define AJDOMKEEP 0

#define AJDOM_TABLE_HINT 1000




static AjBool          domIsAncestor(const AjPDomNode node,
				     const AjPDomNode parent);

static AjPDomNodeEntry domDoLookupNode(const AjPDomNodeList list,
				       const AjPDomNode node);
static void            domUpdateNode(AjPDomNode node);
static AjPDomNode      domDoRemoveChild(AjPDomNode node, AjPDomNode child);
static void            domRemoveFromMap(AjPDomNodeList list,
					const AjPDomNode key);
static void            domAddToMap(AjPDomNodeList list, AjPDomNode key,
				   AjPDomNodeEntry val);


static AjPDomNode  domNodeListItemFiltered(const AjPDomNodeList list,
					   ajint index, ajuint nodetype);

static void        domTraverse(AjPDomNodeList list, AjPDomNode node,
			       const AjPStr tagname);
static void        domTraverseC(AjPDomNodeList list, AjPDomNode node,
				const char *tagname);

static AjPDomNode  domNodeCloneNode(AjPDomDocument ownerdocument,
				    const AjPDomNode node, AjBool deep);


static void   domWriteEncoded(const AjPStr s, AjPFile outf);




ajuint cmtx[] =
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



static const char *nodeinfo[] =
{
	"No such node type",
	"AJDOM_ELEMENT_NODE",
	"AJDOM_ATTRIBUTE_NODE",
	"AJDOM_TEXT_NODE",
	"AJDOM_CDATA_SECTION_NODE",
	"AJDOM_ENTITY_REFERENCE_NODE",
	"AJDOM_ENTITY_NODE",
	"AJDOM_PROCESSING_INSTRUCTION_NODE",
	"AJDOM_COMMENT_NODE",
	"AJDOM_DOCUMENT_NODE",
	"AJDOM_DOCUMENT_TYPE_NODE",
	"AJDOM_DOCUMENT_FRAGMENT_NODE",
	"AJDOM_NOTATION_NODE"
};




#define AJDOM_NOCHILD(p,q) !(cmtx[(p)->type - 1] & (1 << ((q)->type - 1)))
#define AJDOM_DOCMOD(p,q) ((p)->type == AJDOM_DOCUMENT_NODE && \
					(q)->type == AJDOM_ELEMENT_NODE)
#define AJDOM_DOCTYPEMOD(p,q) ((p)->type == AJDOM_DOCUMENT_NODE && \
					(q)->type == AJDOM_DOCUMENT_TYPE_NODE)
#define AJDOM_CANTDO(p,q) (AJDOM_NOCHILD(p,q) || \
					(AJDOM_DOCMOD(p,q) && \
					 (p)->sub.Document.documentelement))





/* @funcstatic domRemoveFromMap **********************************************
**
** Remove a key/value pair from a DOM nodelist 
**
** @param [w] list [AjPDomNodeList] node list
** @param [r] key [const AjPDomNode] key
** @return [void]
** @@
******************************************************************************/

static void domRemoveFromMap(AjPDomNodeList list, const AjPDomNode key)
{
    AjPDomNode trukey;
    AjPDomNodeEntry val;
    if(!list->table)
	return;

    val = ajTableRemoveKey(list->table,key, (void**) &trukey);

    if(val)
    {
	AJFREE(val);
	AJFREE(trukey);
    }

    return;
}




/* @funcstatic domAddToMap ****************************************************
**
** Add a key/value pair to a DOM nodelist 
**
** @param [w] list [AjPDomNodeList] node list
** @param [o] key [AjPDomNode] key
** @param [u] val [AjPDomNodeEntry] value
** @return [void]
** @@
******************************************************************************/

static void domAddToMap(AjPDomNodeList list, AjPDomNode key,
			AjPDomNodeEntry val)
{
    if(!list->table)
	list->table = ajTableNewLen(AJDOM_TABLE_HINT);
    

    domRemoveFromMap(list,key);

    ajTablePut(list->table,key,val);

    return;
}




/* @func ajDomNodeListAppend *************************************************
**
** Append a child node to a DOM nodelist 
**
** @param [w] list [AjPDomNodeList] node list
** @param [u] child [AjPDomNode] child
** @return [AjPDomNodeEntry] Node list child entry created
** @@
******************************************************************************/

AjPDomNodeEntry ajDomNodeListAppend(AjPDomNodeList list,
				    AjPDomNode child)
{
    AjPDomNodeEntry p;
    AjPDomDocumentType doctype;

    if(!list)
    {
	ajWarn("ajDomNodeListAppend: Null pointer error\n");
	return NULL;
    }

    if(list->filter)
    {
	ajWarn("ajDomNodeListAppend: Filtered list error\n");
	return NULL;
    }

    AJNEW0(p);

    domAddToMap(list,child,p);

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
       (doctype=child->ownerdocument->sub.Document.doctype) &&
       list == doctype->childnodes)
    {
	if(child->type == AJDOM_NOTATION_NODE)
	    ++doctype->sub.DocumentType.notations->length;
	else if(child->type == AJDOM_ENTITY_NODE)
	    ++doctype->sub.DocumentType.entities->length;
    }

    if(child->type == AJDOM_ATTRIBUTE_NODE)
	child->sub.Attr.ownerelement = list->ownerelement;
    
    return p;
}




/* @func ajDomNodeAppendChild ************************************************
**
** Appends an extra child at the end of the childnodes list of a node.
** If extrachild is already in the list, it is first removed.  
**
** @param [w] node [AjPDomNode] node
** @param [u] extrachild [AjPDomNode] extra child
** @return [AjPDomNode] Pointer to extrachild or NULL if error
** @@
******************************************************************************/

AjPDomNode ajDomNodeAppendChild(AjPDomNode node, AjPDomNode extrachild)
{
    AjPDomNode n    = NULL;
    AjPDomNode next = NULL;
    
    if(!node || !extrachild)
	return NULL;

    if(extrachild->ownerdocument != node->ownerdocument &&
       node->type != AJDOM_DOCUMENT_NODE &&
       extrachild->type != AJDOM_DOCUMENT_TYPE_NODE)
    {
	ajWarn("ajDomNodeAppendChild: Wrong document");
	return NULL;
    }

    if(extrachild->type == AJDOM_DOCUMENT_FRAGMENT_NODE)
    {
	for(n=extrachild->firstchild; n; n=n->nextsibling)
	    if(AJDOM_CANTDO(node,n) || domIsAncestor(n,node))
	    {
		ajWarn("ajDomNodeAppendChild: Hierarchy Request Error\n");
		return NULL;
	    }

	for(n=extrachild->firstchild; n; n=next)
	{
	    next = n->nextsibling;

	    if(!ajDomRemoveChild(extrachild,n))
		return NULL;

	    if(!ajDomNodeAppendChild(node,n))
	    {
		ajDomDocumentDestroyNode(n->ownerdocument,n);
		return NULL;
	    }
	}

	return extrachild;
    }


    if(AJDOM_CANTDO(node,extrachild) || domIsAncestor(extrachild,node))
    {
	ajWarn("ajDomNodeAppendChild: Hierarchy Request Error 2\n");
	return NULL;
    }

    ajDomRemoveChild(node,extrachild);

    if(!ajDomNodeListAppend(node->childnodes,extrachild))
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

    if(AJDOM_DOCMOD(node,extrachild))
	node->sub.Document.documentelement = extrachild;
    else if(AJDOM_DOCTYPEMOD(node,extrachild))
    {
	node->sub.Document.doctype = extrachild;
	extrachild->ownerdocument = node;
    }

    domUpdateNode(node);

    return extrachild;
}




/* @funcstatic domUpdateNode *************************************************
**
** Update the commonparent entry of a node 
**
** @param [w] node [AjPDomNode] node
** @return [void]
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
	if(n==node->ownerdocument->sub.Document.commonparent)
	    return;
	else if(!p && n->subtreeModified == 1)
	    p = n;
	else
	    n->subtreeModified = 1;

    node->ownerdocument->sub.Document.commonparent = p;

    return;
}

    


/* @funcstatic domIsAncestor *************************************************
**
** Tests whether a putative parent is the parent of a child
**
** @param [r] node [const AjPDomNode] node
** @param [r] parent [const AjPDomNode] putative parent
** @return [AjBool] true if ancestor found
** @@
******************************************************************************/

static AjBool domIsAncestor(const AjPDomNode node, const AjPDomNode parent)
{
    const AjPDomNode p = NULL;

    for(p=parent; p; p=p->parentnode)
	if(p==node)
	    return ajTrue;

    return ajFalse;
}




/* @func ajDomRemoveChild ***************************************************
**
** Removes a child node from a list of children 
**
** @param [w] node [AjPDomNode] node
** @param [u] child [AjPDomNode] child to remove
** @return [AjPDomNode] child removed
** @@
******************************************************************************/

AjPDomNode ajDomRemoveChild(AjPDomNode node, AjPDomNode child)
{
    if(!node || !child)
	return NULL;

    if(child->ownerdocument != node->ownerdocument &&
       child->ownerdocument != node)
    {
	ajWarn("ajDomRemoveChild: Wrong document");

	return NULL;
    }

    child = domDoRemoveChild(node,child);

    return child;
}




/* @func ajDomNodeListExists *************************************************
**
** Check whether a child exists in a nodelist 
**
** @param [u] list [AjPDomNodeList] list
** @param [r] child [const AjPDomNode] child
** @return [AjBool] true if child is in the list
** @@
******************************************************************************/

AjBool ajDomNodeListExists(AjPDomNodeList list, const AjPDomNode child)
{
    AjPDomNodeEntry e = NULL;

    if(!list || list->filter)
	return ajFalse;

    e = domDoLookupNode(list,child);

    if(e)
	return ajTrue;

    return ajFalse;
}




/* @funcstatic domDoLookupNode ***********************************************
**
** Return a pointer to a given node of a nodelist
**
** @param [r] list [const AjPDomNodeList] list
** @param [r] node [const AjPDomNode] node
** @return [AjPDomNodeEntry] node entry or NULL if not found
** @@
******************************************************************************/

static AjPDomNodeEntry domDoLookupNode(const AjPDomNodeList list,
				       const AjPDomNode node)
{
    AjPDomNodeEntry p;

    p = (AjPDomNodeEntry) ajTableFetch(list->table,node);

    return p;
}




/* @func ajDomNodeListRemove *************************************************
**
** Remove a child from a nodelist 
**
** @param [w] list [AjPDomNodeList] list
** @param [u] child [AjPDomNode] child
** @return [AjPDomNodeEntry] child removed or NULL if not found
** @@
******************************************************************************/

AjPDomNodeEntry ajDomNodeListRemove(AjPDomNodeList list, AjPDomNode child)
{
    AjPDomNodeEntry e;
    AjPDomNode trukey;
    AjPDomNodeEntry val;

    if(!list)
    {
	ajWarn("ajDomNodeListRemove: Empty list");
	return NULL;
    }

    if(list->filter)
    {
	ajWarn("ajDomNodeListRemove: Filtered list error");
	return NULL;
    }
    
    e = domDoLookupNode(list,child);

    if(!e)
	return NULL;

    val = ajTableRemoveKey(list->table,child, (void**) &trukey);
    AJFREE(val);
    AJFREE(trukey);

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

    if(child->type == AJDOM_ATTRIBUTE_NODE)
	child->sub.Attr.ownerelement = NULL;

    return e;
}




/* @funcstatic domDoRemoveChild ********************************************** 
**
** Low level removal of a child node from a list of children 
**
** @param [w] node [AjPDomNode] node
** @param [u] child [AjPDomNode] child to remove
** @return [AjPDomNode] child removed
** @@
******************************************************************************/

static AjPDomNode domDoRemoveChild(AjPDomNode node, AjPDomNode child)
{
    AjPDomNodeEntry e = NULL;

    if(!ajDomNodeListExists(node->childnodes,child))
	return NULL;

    e = ajDomNodeListRemove(node->childnodes,child);
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

    if(AJDOM_DOCMOD(node,child))
	node->sub.Document.documentelement = NULL;
    else if(AJDOM_DOCTYPEMOD(node,child))
    {
	node->sub.Document.doctype = NULL;
	child->ownerdocument = NULL;
    }
    else
	domUpdateNode(node);

    return child;
}




/* @func ajDomDocumentDestroyNode ********************************************
**
** Frees a node and its children
**
** @param [w] doc [AjPDomDocument] document
** @param [w] node [AjPDomNode] node
** @return [void]
** @@
******************************************************************************/

void ajDomDocumentDestroyNode(AjPDomDocument doc, AjPDomNode node)
{

    if(!node)
	return;

    if(node->childnodes)
	ajDomDocumentDestroyNodeList(doc,node->childnodes,AJDOMDESTROY);

    switch(node->type)
    {
        case AJDOM_ELEMENT_NODE:
            ajDomDocumentDestroyNodeList(doc,node->attributes,AJDOMDESTROY);
            ajStrDel(&node->sub.Element.tagname);
            ajStrDel(&node->name);
            break;
        case AJDOM_TEXT_NODE:
        case AJDOM_COMMENT_NODE:
        case AJDOM_CDATA_SECTION_NODE:
            ajStrDel(&node->value);
            break;
        case AJDOM_ATTRIBUTE_NODE:
            ajStrDel(&node->name);
            ajStrDel(&node->value);
            break;
        case AJDOM_ENTITY_REFERENCE_NODE:
        case AJDOM_ENTITY_NODE:
            ajStrDel(&node->name);
            ajStrDel(&node->value);
            ajStrDel(&node->sub.Entity.publicid);
            ajStrDel(&node->sub.Entity.systemid);
            ajStrDel(&node->sub.Entity.notationname);
            break;
        case AJDOM_PROCESSING_INSTRUCTION_NODE:
            ajStrDel(&node->name);
            ajStrDel(&node->value);
            break;
        case AJDOM_DOCUMENT_NODE:
            ajStrDel(&node->sub.Document.version);
            ajStrDel(&node->sub.Document.encoding);
            break;
        case AJDOM_DOCUMENT_TYPE_NODE:
            ajDomDocumentDestroyNodeList(doc,node->sub.DocumentType.entities,
                                         AJDOMKEEP);
            ajDomDocumentDestroyNodeList(doc,node->sub.DocumentType.notations,
                                         AJDOMKEEP);
            ajStrDel(&node->sub.DocumentType.name);
            ajStrDel(&node->sub.DocumentType.publicid);
            ajStrDel(&node->sub.DocumentType.systemid);
            ajStrDel(&node->name);
            break;
        case AJDOM_NOTATION_NODE:
            ajStrDel(&node->sub.Notation.publicid);
            ajStrDel(&node->sub.Notation.systemid);
            ajStrDel(&node->name);
            break;
    }

    AJFREE(node);
    
    return;
}




/* @func ajDomDocumentDestroyNodeList ****************************************
**
** Frees a nodelist
**
** @param [w] doc [AjPDomDocument] document
** @param [w] list [AjPDomNodeList] list
** @param [r] donodes [AjBool] free nodes as well if true
** @return [void]
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
		    ajDomDocumentDestroyNode(doc,entry->node);

		tmp = entry;
		entry = entry->next;
		AJFREE(tmp);
	    }
	}
	

	/* AJB: return to this */
	if(list->table)
	    ajTableFree(&list->table);

	AJFREE(list);
    }

    return;
}




/* @func ajDomCreateNodeList **************************************************
**
** Create a nodelist
**
** @param [u] doc [AjPDomDocument] document
** @return [AjPDomNodeList] new list
** @@
******************************************************************************/

AjPDomNodeList ajDomCreateNodeList(AjPDomDocument doc)
{
    AjPDomNodeList list = NULL;
    
    AJNEW0(list);
    list->ownerdocument = doc;

    return list;
}




/* @func ajDomDocumentCreateNode *********************************************
**
** General document node creation
**
** @param [u] doc [AjPDomDocument] document
** @param [r] nodetype [ajuint] type of node to create
** @return [AjPDomNode] new node
** @@
******************************************************************************/

AjPDomNode ajDomDocumentCreateNode(AjPDomDocument doc, ajuint nodetype)
{
    AjPDomNode node;

    if (!doc && nodetype != AJDOM_DOCUMENT_NODE &&
	nodetype != AJDOM_DOCUMENT_TYPE_NODE)
    {
	ajWarn("ajDocumentCreateNode: allocation failure\n");
	return NULL;
    }

    AJNEW0(node);

    node->ownerdocument = doc;
    node->type      = nodetype;

    switch (nodetype)
    {
        case AJDOM_DOCUMENT_NODE:
        case AJDOM_DOCUMENT_TYPE_NODE:
        case AJDOM_ELEMENT_NODE:
        case AJDOM_ATTRIBUTE_NODE:
        case AJDOM_ENTITY_REFERENCE_NODE:
        case AJDOM_ENTITY_NODE:
        case AJDOM_DOCUMENT_FRAGMENT_NODE:
            node->childnodes = ajDomCreateNodeList(doc);

            if(node->childnodes == NULL)
	    {
		ajWarn("ajDocumentCreateNode: ajDocumentCreateNodeList"
		       " failed\n");
                ajDomDocumentDestroyNode(doc,node);
                return NULL;
            }
    }


    return node;
}




/* @func ajDomImplementationCreateDocumentType *******************************
**
** Creates an empty DocumentType node into which entities/notations (etc)
** can be placed 
**
** @param [r] qualname [const AjPStr] qualified name
** @param [r] publicid [const AjPStr] public id
** @param [r] systemid [const AjPStr] systemid
** @return [AjPDomDocumentType] new DocumentType node
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

    return ajDomImplementationCreateDocumentTypeC(p,r,s);
}




/* @func ajDomImplementationCreateDocumentTypeC ******************************
**
** Creates an empty DocumentType node into which entities/notations (etc)
** can be placed 
**
** @param [r] qualname [const char *] qualified name
** @param [r] publicid [const char *] public id
** @param [r] systemid [const char *] systemid
** @return [AjPDomDocumentType] new DocumentType node
** @@
******************************************************************************/

AjPDomDocumentType ajDomImplementationCreateDocumentTypeC(const char *qualname,
							  const char *publicid,
							  const char *systemid)
{
    AjPDomDocumentType doctype;
    AjPDomNodeMap entities;
    AjPDomNodeMap notations;


    if(!(doctype=ajDomDocumentCreateNode(NULL,AJDOM_DOCUMENT_TYPE_NODE)))
    {
	ajWarn("ajDomImplementationCreateDocumentType: Cannot create node");
	return NULL;
    }

    ajStrAssignC(&doctype->sub.DocumentType.name,qualname);
    doctype->name = doctype->sub.DocumentType.name;
    
    if(publicid)
	ajStrAssignC(&doctype->sub.DocumentType.publicid,publicid);

    if(systemid)
	ajStrAssignC(&doctype->sub.DocumentType.systemid,systemid);
	

    entities  = ajDomCreateNodeList(NULL);
    notations = ajDomCreateNodeList(NULL);

    entities->filter = AJDOM_ENTITY_NODE;
    entities->list   = doctype->childnodes;
    
    notations->filter = AJDOM_NOTATION_NODE;
    notations->list   = doctype->childnodes;    

    doctype->sub.DocumentType.entities = entities;
    doctype->sub.DocumentType.notations = notations;

    return doctype;
}




/* @func ajDomImplementationCreateDocument ***********************************
**
** Creates an XML Document object of the specified type with its document
** element.
**
** @param [r] uri [const AjPStr] uri (not implemented. Pass NULL)
** @param [r] qualname [const AjPStr] qualified name
** @param [u] doctype [AjPDomDocumentType] doctype
** @return [AjPDomDocument] new document
** @@
******************************************************************************/

AjPDomDocument ajDomImplementationCreateDocument(const AjPStr uri,
						 const AjPStr qualname,
						 AjPDomDocumentType doctype)
{
    const char *p = NULL;

    (void) uri;				/* temporary use */

    if(qualname)
        p = qualname->Ptr;

    return ajDomImplementationCreateDocumentC(NULL,p,doctype);
}




/* @func ajDomImplementationCreateDocumentC **********************************
**
** Creates an XML Document object of the specified type with its document
** element.
**
** @param [r] uri [const char *] uri (not implemented. Pass NULL)
** @param [r] qualname [const char *] qualified name
** @param [u] doctype [AjPDomDocumentType] doctype
** @return [AjPDomDocument] new document
** @@
******************************************************************************/

AjPDomDocument ajDomImplementationCreateDocumentC(const char *uri,
						  const char *qualname,
					          AjPDomDocumentType doctype)
{
    AjPDomDocument doc     = NULL;
    AjPDomElement  element = NULL;

    uri = NULL;

    if(!(doc = ajDomDocumentCreateNode(NULL,AJDOM_DOCUMENT_NODE)))
    {
	ajWarn("ajDomImplementationCreateDocumentC: document memory\n");
	return NULL;
    }

    doc->name = ajStrNew();
    ajStrAssignC(&doc->name,"#document");

    if(doctype)
	ajDomNodeAppendChild(doc,doctype);

    if(qualname)
	if(strlen(qualname))
	{
	    element = ajDomDocumentCreateElementC(doc,qualname);
	    if(!element)
	    {
		ajWarn("ajDomImplementationCreateDocumentC: element memory\n");
		ajDomDocumentDestroyNode(doc,doc);

		return NULL;
	    }

	    ajDomNodeAppendChild(doc,element);
	}

    return doc;
}




/* @func ajDomNodeMapGetItem ********************************************
**
** Returns the named node from a nodemap
**
** @param [r] map [const AjPDomNodeMap] map
** @param [r] name [const AjPStr] name
** @return [AjPDomNode] node
** @@
******************************************************************************/

AjPDomNode ajDomNodeMapGetItem(const AjPDomNodeMap map, const AjPStr name)
{
    char *p = NULL;

    if(name)
	p = name->Ptr;
    
    return ajDomNodeMapGetItemC(map,p);
}




/* @func ajDomNodeMapGetItemC *******************************************
**
** Returns the named node from a nodemap
**
** @param [r] map [const AjPDomNodeMap] map
** @param [r] name [const char *] name
** @return [AjPDomNode] node
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
	    for(e=map->first; e; e=e->next)
		if(e->node->type == nodetype &&
		   !strcmp(name,e->node->name->Ptr))
			    return e->node;
	}
	else
	    for(e=map->first; e; e=e->next)
		if(!strcmp(name,e->node->name->Ptr))
		    return e->node;
    }
    
    return NULL;
}




/* @func ajDomElementGetAttribute ********************************************
**
** Returns the value of a named attribute 
**
** @param [r] element [const AjPDomElement] element
** @param [r] name [const AjPStr] name
** @return [AjPStr] value or empty string
** @@
******************************************************************************/

AjPStr ajDomElementGetAttribute(const AjPDomElement element, const AjPStr name)
{
    char *p = NULL;

    if(name)
	p = name->Ptr;

    return ajDomElementGetAttributeC(element,p);
}




/* @func ajDomElementGetAttributeC *******************************************
**
** Returns the value of a named attribute 
**
** @param [r] element [const AjPDomElement] element
** @param [r] name [const char *] name
** @return [AjPStr] value or empty string
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
	if((node=ajDomNodeMapGetItemC(element->attributes,name)))
	    ajStrAssignS(&ret,node->value);
    }

    return ret;
}




/* @func ajDomNodeMapSetItem *******************************************
**
** Enter arg into the nodemap using name as the key.
** If a node with the same name already exists in the map it will be replaced
** with the new node and returned.
** The replaced node should usually be freed with DestroyNode.
**
** @param [u] map [AjPDomNodeMap] map
** @param [u] arg [AjPDomNode] arg
** @return [AjPDomNode] replaced node or NULL
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
	    ajWarn("ajDomNodeMapSetItem: No mod allowed\n");

	    return NULL;
	}

	if(map->ownerdocument != arg->ownerdocument)
	{
	    ajWarn("ajDomNodeMapSetItem: Wrong document\n");

	    return NULL;
	}

	if(arg->type == AJDOM_ATTRIBUTE_NODE &&
	   arg->sub.Attr.ownerelement &&
	   arg->sub.Attr.ownerelement != map->ownerelement)
	{
	    ajWarn("ajDomNodeMapSetItem: In use attribute error\n");

	    return NULL;
	}

	for(e=map->first; e && strcmp(arg->name->Ptr,e->node->name->Ptr);
	    e=e->next)
	{
	    ;
	}

	if(e)
	{
	    tmp = e->node;
	    e->node = arg;
	    if(arg->type == AJDOM_ATTRIBUTE_NODE)
	    {
		arg->sub.Attr.ownerelement = map->ownerelement;
		tmp->sub.Attr.ownerelement = NULL;
	    }

	    return tmp;
	}

	ajDomNodeListAppend(map,arg);
    }

    return NULL;
}




/* @func ajDomNodeMapRemoveItem *****************************************
**
** Removes and returns item from a  map
**
** @param [u] map [AjPDomNodeMap] map
** @param [r] name [const AjPStr] name
** @return [AjPDomNode] removed node or NULL
** @@
******************************************************************************/

AjPDomNode ajDomNodeMapRemoveItem(AjPDomNodeMap map, const AjPStr name)
{
    char *p = NULL;

    if(name)
	p = name->Ptr;

    return ajDomNodeMapRemoveItemC(map,p);
}




/* @func ajDomNodeMapRemoveItemC ****************************************
**
** Removes and returns item from a  map
**
** @param [w] map [AjPDomNodeMap] map
** @param [r] name [const char *] name
** @return [AjPDomNode] removed node or NULL
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
	    ajWarn("ajDomNodeMapRemoveItemC: No mod allowed\n");

	    return NULL;
	}

	for(e=map->first; e; e=e->next)
	{
	    if(!strcmp(name,e->node->name->Ptr) &&
	       ajDomNodeListRemove(map,e->node))
	    {
		r = e->node;
		free(e);

		if(r->type == AJDOM_ATTRIBUTE_NODE)
		    r->sub.Attr.ownerelement = NULL;

		return r;
	    }
	}
    }

    ajWarn("ajDomNodeMapRemoveItemC: Not found error\n");

    return NULL;
}




/* @func ajDomNodeMapItem ****************************************************
**
** Return the node in the map at a given index
**
** @param [r] map [const AjPDomNodeMap] map
** @param [r] indexnum [ajint] index
** @return [AjPDomNode] node or NULL
** @@
******************************************************************************/

AjPDomNode ajDomNodeMapItem(const AjPDomNodeMap map, ajint indexnum)
{
    return ajDomNodeListItem(map,indexnum);
}




/* @funcstatic domNodeListItemFiltered ***************************************
**
** Filter nodelist item
**
** @param [r] list [const AjPDomNodeList] list
** @param [r] indexnum [ajint] index
** @param [r] nodetype [ajuint] nodetype
** @return [AjPDomNode] node or NULL
** @@
******************************************************************************/

static AjPDomNode domNodeListItemFiltered(const AjPDomNodeList list,
					  ajint indexnum, ajuint nodetype)
{
    AjPDomNodeEntry e = NULL;
    
    if(list && indexnum >= 0 && indexnum < list->length)
	for(e=list->first; e; e=e->next)
	    if(e->node->type == nodetype)
	    {
		if(!indexnum)
		    return e->node;

		--indexnum;
	    }

    return NULL;
}




/* @func ajDomNodeListItem ***************************************************
**
** Return the node in the list at a given index
**
** @param [r] list [const AjPDomNodeList] list
** @param [r] indexnum [ajint] index
** @return [AjPDomNode] node or NULL
** @@
******************************************************************************/

AjPDomNode ajDomNodeListItem(const AjPDomNodeList list, ajint indexnum)
{
    AjPDomNodeEntry e = NULL;

    if(list)
    {
	if(list->filter)
	    return domNodeListItemFiltered(list->list,indexnum,list->filter);

	if(indexnum >= 0 && indexnum < list->length)
	    for(e=list->first; e; e=e->next, --indexnum)
		if(!indexnum)
		    return e->node;
    }

    return NULL;
}




/* @func ajDomElementSetAttribute ********************************************
**
** Adds a new attribute or sets the value of an existing attribute.
**
** @param [r] element [const AjPDomElement] element
** @param [r] name [const AjPStr] name
** @param [r] value [const AjPStr] value
** @return [void]
** @@
******************************************************************************/

void ajDomElementSetAttribute(const AjPDomElement element, const AjPStr name,
			      const AjPStr value)
{
    if(!element || !name || !value || !element->attributes)
	return;

    if(!ajStrGetLen(name) || !ajStrGetLen(value))
        return;

    ajDomElementSetAttributeC(element,name->Ptr,value->Ptr);

    return;
}




/* @func ajDomElementSetAttributeC *******************************************
**
** Adds a new attribute or sets the value of an existing attribute.
**
** @param [r] element [const AjPDomElement] element
** @param [r] name [const char *] name
** @param [r] value [const char *] value
** @return [void]
** @@
******************************************************************************/

void ajDomElementSetAttributeC(const AjPDomElement element, const char *name,
			       const char *value)
{
    AjPDomNode attr = NULL;
    AjPStr prevvalue = NULL;


    if(!element || !name || !value || !element->attributes)
	return;

    attr = ajDomNodeMapGetItemC(element->attributes,name);

    if(attr)
    {
	prevvalue = attr->value;

	attr->sub.Attr.value = ajStrNew();
	ajStrAssignC(&attr->sub.Attr.value,value);
	/* AJB: dirty */
	attr->value = attr->sub.Attr.value;
    }
    else
    {
	prevvalue = NULL;
	if(!(attr=ajDomDocumentCreateAttributeC(element->ownerdocument,name)))
	{
	    ajWarn("ajDomElementSetAttribute: DOM create failed\n");
	    return;
	}

	ajStrDel(&attr->value);

	attr->sub.Attr.value = ajStrNew();
	ajStrAssignC(&attr->sub.Attr.value,value);
	/* AJB: dirty */
	attr->value = attr->sub.Attr.value;

	ajDomNodeMapSetItem(element->attributes, attr);
    }

    domUpdateNode(element->parentnode);

    if(prevvalue)
	ajStrDel(&prevvalue);

    return;
}




/* @func ajDomElementRemoveAttribute *****************************************
**
** Remove and free a named attribute
**
** @param [u] element [AjPDomElement] element
** @param [r] name [const AjPStr] name
** @return [void]
** @@
******************************************************************************/

void ajDomElementRemoveAttribute(AjPDomElement element, const AjPStr name)
{

    if(!name)
        return;

    ajDomElementRemoveAttributeC(element,name->Ptr);

    return;
}




/* @func ajDomElementRemoveAttributeC ****************************************
**
** Remove and free a named attribute
**
** @param [u] element [AjPDomElement] element
** @param [r] name [const char *] name
** @return [void]
** @@
******************************************************************************/

void ajDomElementRemoveAttributeC(AjPDomElement element, const char *name)
{
    AjPDomNode attr = NULL;

    if(!element || !name)
	return;

    attr = ajDomNodeMapRemoveItemC(element->attributes,name);

    if(attr)
    {
	domUpdateNode(element->parentnode);
	ajDomDocumentDestroyNode(attr->ownerdocument,attr);
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
** @@
******************************************************************************/

AjPDomNode ajDomElementGetAttributeNode(const AjPDomElement element,
					const AjPStr name)
{
    if(element && name)
	return ajDomNodeMapGetItem(element->attributes,name);

    return NULL;
}




/* @func ajDomElementGetAttributeNodeC ***************************************
**
** Returns the named attribute node.
**
** @param [r] element [const AjPDomElement] element
** @param [r] name [const char *] name
** @return [AjPDomNode] node or NULL
** @@
******************************************************************************/

AjPDomNode ajDomElementGetAttributeNodeC(const AjPDomElement element,
					 const char *name)
{
    if(element && name)
	return ajDomNodeMapGetItemC(element->attributes,name);

    return NULL;
}




/* @func ajDomElementSetAttributeNode ****************************************
**
** Add an attribute to the attributes of an element.
** If this element already has an attribute with the same name it will be
** replaced with the new attribute and returned.
**
** @param [u] element [AjPDomElement] element
** @param [u] newattr [AjPDomNode] name
** @return [AjPDomNode] new or replaced attribute
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
	ajWarn("ajDomElementSetAttributeNode: Wrong document\n");
	return NULL;
    }

    attr = ajDomNodeMapSetItem(element->attributes,newattr);

    domUpdateNode(element->parentnode);

    return attr;
}




/* @func ajDomElementRemoveAttributeNode *************************************
**
** Removes and returns a pointer to an attribute
**
** @param [w] element [AjPDomElement] element
** @param [u] oldattr [AjPDomNode] attribute
** @return [AjPDomNode] removed attribute or NULL
** @@
******************************************************************************/

AjPDomNode ajDomElementRemoveAttributeNode(AjPDomElement element,
					   AjPDomNode oldattr)
{
    if(!element || !oldattr || !ajDomNodeListRemove(element->attributes,
						    oldattr))
    {
	ajWarn("ajDomElementRemoveAttributeNode: DOM not found error\n");
	return NULL;
    }

    domUpdateNode(element->parentnode);

    return oldattr;
}




/* @funcstatic domTraverse ***************************************************
**
** Preorder elements
**
** @param [w] list [AjPDomNodeList] list
** @param [u] node [AjPDomNode] node
** @param [r] tagname [const AjPStr] tagname
** @return [void]
** @@
******************************************************************************/

static void domTraverse(AjPDomNodeList list, AjPDomNode node,
			const AjPStr tagname)
{
    AjPDomNode n = NULL;

    if(list && node && node->type == AJDOM_ELEMENT_NODE && tagname)
    {
	if(ajStrMatchC(tagname,"*") || !strcmp(tagname->Ptr,node->name->Ptr))
	    ajDomNodeListAppend(list,node);

	for(n=node->firstchild; n; n=n->nextsibling)
	    domTraverse(list,n,tagname);
    }

    return;
}




/* @funcstatic domTraverseC ***************************************************
**
** Preorder elements
**
** @param [w] list [AjPDomNodeList] list
** @param [u] node [AjPDomNode] node
** @param [r] tagname [const char *] tagname
** @return [void]
** @@
******************************************************************************/

static void domTraverseC(AjPDomNodeList list, AjPDomNode node,
			 const char *tagname)
{
    AjPDomNode n = NULL;

    if(list && node && node->type == AJDOM_ELEMENT_NODE && tagname)
    {
	if(ajCharMatchC(tagname,"*") || !strcmp(tagname,node->name->Ptr))
	    ajDomNodeListAppend(list,node);

	for(n=node->firstchild; n; n=n->nextsibling)
	    domTraverseC(list,n,tagname);
    }

    return;
}




/* @func ajDomElementGetElementsByTagName ************************************
**
** Perform a preorder traversal of the entire document.
** Return a nodelist of the elements with the name tagname in the order
** in which they are found.
**
** @param [u] element [AjPDomElement] element
** @param [r] name [const AjPStr] name
** @return [AjPDomNodeList] nodelist
** @@
******************************************************************************/

AjPDomNodeList ajDomElementGetElementsByTagName(AjPDomElement element,
						const AjPStr name)
{

    if(!element || !name)
        return NULL;

    return ajDomElementGetElementsByTagNameC(element,name->Ptr);
}




/* @func ajDomElementGetElementsByTagNameC ***********************************
**
** Perform a preorder traversal of the entire document.
** Return a nodelist of the elements with the name tagname in the order
** in which they are found.
**
** @param [u] element [AjPDomElement] element
** @param [r] name [const char *] name
** @return [AjPDomNodeList] nodelist
** @@
******************************************************************************/

AjPDomNodeList ajDomElementGetElementsByTagNameC(AjPDomElement element,
						 const char *name)
{
    AjPDomNodeList list = NULL;
    AjPDomNode n = NULL;

    if(element && element->type == AJDOM_ELEMENT_NODE && name &&
       (list=ajDomCreateNodeList(element->ownerdocument)))
    {
	for(n=element->firstchild; n; n=n->nextsibling)
	    domTraverseC(list,n,name);

	return list;
    }

    return NULL;
}




/* @func ajDomElementNormalise ***********************************************
**
** Merge adjacent text node content into "normal" form in the subtree of
** a node. Remove empty text node.
**
** @param [w] element [AjPDomElement] element
** @return [void]
** @@
******************************************************************************/

void ajDomElementNormalise(AjPDomElement element)
{
    AjPDomNode node = NULL;
    AjPDomText last = NULL;

    if(element)
    {
	for(node=element->firstchild; node; node=node->nextsibling)
	{
	    if(node->type == AJDOM_TEXT_NODE)
	    {
		if(last)
		{
		    ajDomCharacterDataInsertData(node,0,last->value);
		    ajDomRemoveChild(element,last);
		    ajDomDocumentDestroyNode(last->ownerdocument,last);
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




/* @func ajDomCharacterDataSubstringData *************************************
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

    if(offset < 0 || offset > (dlen=data->sub.CharacterData.length) ||
       count < 0)
	return NULL;

    if(count > (dlen - offset))
	count = dlen - offset;

    sub = ajStrNew();

    ajStrAssignSubS(&sub,data->value,offset,offset+count-1);

    return sub;
}




/* @func ajDomCharacterDataAppendData ****************************************
**
** Append a string to a character data node
**
** @param [w] data [AjPDomCharacterData] character data
** @param [r] arg [const AjPStr] string
** @return [void]
** @@
******************************************************************************/

void ajDomCharacterDataAppendData(AjPDomCharacterData data, const AjPStr arg)
{

    if(!data || !arg)
	return;

    ajDomCharacterDataAppendDataC(data,arg->Ptr);

    return;
}




/* @func ajDomCharacterDataAppendDataC ***************************************
**
** Append a string to a character data node
**
** @param [w] data [AjPDomCharacterData] character data
** @param [r] arg [const char *] string
** @return [void]
** @@
******************************************************************************/

void ajDomCharacterDataAppendDataC(AjPDomCharacterData data, const char *arg)
{

    if(!data || !arg)
	return;

    ajStrAppendC(&data->value,arg);
    data->sub.CharacterData.length = ajStrGetLen(data->value);
    data->sub.CharacterData.data = data->value;

    domUpdateNode(data->parentnode);

    return;
}




/* @func ajDomCharacterDataInsertData ****************************************
**
** Insert a string at position offset
**
** @param [w] data [AjPDomCharacterData] character data
** @param [r] offset [ajint] offset
** @param [r] arg [const AjPStr] string
** @return [void]
** @@
******************************************************************************/

void ajDomCharacterDataInsertData(AjPDomCharacterData data, ajint offset,
				  const AjPStr arg)
{

    if(!data || !arg)
	return;

    ajDomCharacterDataInsertDataC(data,offset,arg->Ptr);

    return;
}




/* @func ajDomCharacterDataInsertDataC ***************************************
**
** Insert a string at position offset
**
** @param [w] data [AjPDomCharacterData] character data
** @param [r] offset [ajint] offset
** @param [r] arg [const char *] string
** @return [void]
** @@
******************************************************************************/

void ajDomCharacterDataInsertDataC(AjPDomCharacterData data, ajint offset,
				   const char *arg)
{

    if(!data || !arg)
	return;

    if(offset < 0 || offset > data->sub.CharacterData.length)
	return;

    ajStrInsertC(&data->value,offset,arg);
    data->sub.CharacterData.length = ajStrGetLen(data->value);
    data->sub.CharacterData.data = data->value;
    
    domUpdateNode(data->parentnode);
    
    return;
}




/* @func ajDomCharacterDataDeleteData ***************************************
**
** Remove at most count characters of position offset
**
** @param [w] data [AjPDomCharacterData] character data
** @param [r] offset [ajint] offset
** @param [r] count [ajint] count
** @return [void]
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

    ajStrCutRange(&data->value, offset, offset+count-1);
    data->sub.CharacterData.length = ajStrGetLen(data->value);
    data->sub.CharacterData.data = data->value;
    
    domUpdateNode(data->parentnode);

    return;
}




/* @func ajDomCharacterDataReplaceData ***************************************
**
** Replace offset/count characters with a string 
**
** @param [w] data [AjPDomCharacterData] character data
** @param [r] offset [ajint] offset
** @param [r] count [ajint] count
** @param [r] arg [const AjPStr] replacement string
** @return [void]
** @@
******************************************************************************/

void ajDomCharacterDataReplaceData(AjPDomCharacterData data, ajint offset,
				   ajint count, const AjPStr arg)
{
    ajDomCharacterDataDeleteData(data,offset,count);
    ajDomCharacterDataInsertData(data,offset,arg);

    return;
}




/* @func ajDomCharacterDataReplaceDataC **************************************
**
** Replace offset/count characters with a string 
**
** @param [w] data [AjPDomCharacterData] character data
** @param [r] offset [ajint] offset
** @param [r] count [ajint] count
** @param [r] arg [const char *] replacement string
** @return [void]
** @@
******************************************************************************/

void ajDomCharacterDataReplaceDataC(AjPDomCharacterData data, ajint offset,
				    ajint count, const char *arg)
{
    ajDomCharacterDataDeleteData(data,offset,count);
    ajDomCharacterDataInsertDataC(data,offset,arg);

    return;
}




/* @func ajDomCharacterDataGetLength ******************************************
**
** Return length of character data 
**
** @param [r] data [const AjPDomCharacterData] character data
** @return [ajint] length
** @@
******************************************************************************/

ajint ajDomCharacterDataGetLength(const AjPDomCharacterData data)
{
    return data ? data->sub.CharacterData.length : 0;
}




/* @func ajDomTextSplitText **************************************************
**
** Split a text node at the specified offset into two adjacent text nodes.
** The first is a string offset size in length whereas the second is the
** remainder. If offset is equal to the length of the string the new sibling
** has zero length 
**
** @param [w] text [AjPDomText] text
** @param [r] offset [ajint] offset
** @return [AjPDomText] remainder node or NULL
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
    
    ajStrAssignSubS(&sub,text->value,offset,len-1);
    
    node = ajDomDocumentCreateTextNode(text->ownerdocument,sub);

    ajStrDel(&sub);

    if(!node)
	return NULL;

    ajDomCharacterDataDeleteData(text,offset,len-1);
    ajDomNodeInsertBefore(text->parentnode,node,text->nextsibling);

    return node;
}




/* @func ajDomDocumentCreateElement ******************************************
**
** Create a document element
**
** @param [w] doc [AjPDomDocument] document
** @param [r] tagname [const AjPStr] tagname
** @return [AjPDomElement] element
** @@
******************************************************************************/

AjPDomElement ajDomDocumentCreateElement(AjPDomDocument doc,
					 const AjPStr tagname)
{
    if(!tagname)
        return NULL;

    return ajDomDocumentCreateElementC(doc,tagname->Ptr);
}




/* @func ajDomDocumentCreateElementC ******************************************
**
** Create a document element
**
** @param [w] doc [AjPDomDocument] document
** @param [r] tagname [const char *] tagname
** @return [AjPDomElement] element
** @@
******************************************************************************/

AjPDomElement ajDomDocumentCreateElementC(AjPDomDocument doc,
					  const char *tagname)
{
    AjPDomElement element = NULL;

    if(!tagname)
        return NULL;

    element = ajDomDocumentCreateNode(doc, AJDOM_ELEMENT_NODE);

    if(!element)
	return NULL;

    element->sub.Element.tagname = ajStrNewC(tagname);
    element->name = element->sub.Element.tagname;

    element->attributes = ajDomCreateNodeList(doc);

    element->attributes->ownerelement = element;

    return element;
}




/* @func ajDomDocumentCreateDocumentFragment *********************************
**
** Create an empty document fragment
**
** @param [w] doc [AjPDomDocument] document
** @return [AjPDomDocumentFragment] fragment
** @@
******************************************************************************/

AjPDomDocumentFragment ajDomDocumentCreateDocumentFragment(AjPDomDocument doc)
{
    AjPDomDocumentFragment frag = NULL;

    frag = ajDomDocumentCreateNode(doc, AJDOM_DOCUMENT_FRAGMENT_NODE);

    if(frag)
	frag->name = ajStrNewC("#document-fragment");

    return frag;
}




/* @func ajDomDocumentCreateTextNode *****************************************
**
** Create a  text node
**
** @param [w] doc [AjPDomDocument] document
** @param [r] data [const AjPStr] text
** @return [AjPDomText] text node
** @@
******************************************************************************/

AjPDomText ajDomDocumentCreateTextNode(AjPDomDocument doc, const AjPStr data)
{
    if(!data)
        return NULL;

    return ajDomDocumentCreateTextNodeC(doc,data->Ptr);
}




/* @func ajDomDocumentCreateTextNodeC ****************************************
**
** Create a text node
**
** @param [w] doc [AjPDomDocument] document
** @param [r] data [const char *] text
** @return [AjPDomText] text node
** @@
******************************************************************************/

AjPDomText ajDomDocumentCreateTextNodeC(AjPDomDocument doc, const char *data)
{
    AjPDomText text = NULL;

    text = ajDomDocumentCreateNode(doc, AJDOM_TEXT_NODE);
    if(!text)
	return NULL;

    text->name = ajStrNewC("#text");

    text->sub.CharacterData.data = ajStrNewC(data);
    text->value = text->sub.CharacterData.data;

    text->sub.CharacterData.length = ajStrGetLen(text->value);

    return text;
}




/* @func ajDomDocumentCreateComment *****************************************
**
** Create a comment node
**
** @param [w] doc [AjPDomDocument] document
** @param [r] data [const AjPStr] text
** @return [AjPDomComment] comment node
** @@
******************************************************************************/

AjPDomComment ajDomDocumentCreateComment(AjPDomDocument doc, const AjPStr data)
{
    if(!data)
        return NULL;

    return ajDomDocumentCreateCommentC(doc,data->Ptr);
}




/* @func ajDomDocumentCreateCommentC *****************************************
**
** Create a comment node
**
** @param [w] doc [AjPDomDocument] document
** @param [r] data [const char *] text
** @return [AjPDomComment] comment node
** @@
******************************************************************************/

AjPDomComment ajDomDocumentCreateCommentC(AjPDomDocument doc, const char *data)
{
    AjPDomComment comment = NULL;

    comment = ajDomDocumentCreateNode(doc, AJDOM_COMMENT_NODE);

    if(!comment)
	return NULL;

    comment->name = ajStrNewC("#comment");

    comment->sub.CharacterData.data = ajStrNewC(data);
    comment->value = comment->sub.CharacterData.data;

    comment->sub.CharacterData.length = ajStrGetLen(comment->value);

    return comment;
}




/* @func ajDomDocumentCreateCDATASection *************************************
**
** Create a CDATA section
**
** @param [w] doc [AjPDomDocument] document
** @param [r] data [const AjPStr] text
** @return [AjPDomCDATASection] comment node
** @@
******************************************************************************/

AjPDomCDATASection ajDomDocumentCreateCDATASection(AjPDomDocument doc,
						   const AjPStr data)
{
    if(!data)
        return NULL;

    return ajDomDocumentCreateCDATASectionC(doc,data->Ptr);
}




/* @func ajDomDocumentCreateCDATASectionC ************************************
**
** Create a CDATA section
**
** @param [w] doc [AjPDomDocument] document
** @param [r] data [const char *] text
** @return [AjPDomCDATASection] comment node
** @@
******************************************************************************/

AjPDomCDATASection ajDomDocumentCreateCDATASectionC(AjPDomDocument doc,
						    const char *data)
{
    AjPDomCDATASection cdata = NULL;

    cdata = ajDomDocumentCreateNode(doc, AJDOM_CDATA_SECTION_NODE);

    if(!cdata)
	return NULL;

    cdata->name = ajStrNewC("#cdata-section");

    cdata->sub.CharacterData.data = ajStrNewC(data);
    cdata->value = cdata->sub.CharacterData.data;

    cdata->sub.CharacterData.length = ajStrGetLen(cdata->value);

    return cdata;
}




/* @func ajDomDocumentCreateAttribute ****************************************
**
** Create an attribute node
**
** @param [w] doc [AjPDomDocument] document
** @param [r] name [const AjPStr] text
** @return [AjPDomAttr] attribute node
** @@
******************************************************************************/

AjPDomAttr ajDomDocumentCreateAttribute(AjPDomDocument doc,
					const AjPStr name)
{
    AjPDomAttr attr = NULL;

    attr = ajDomDocumentCreateNode(doc, AJDOM_ATTRIBUTE_NODE);

    if(!attr)
	return NULL;

    attr->sub.Attr.name = ajStrNewS(name);
    attr->name = attr->sub.Attr.name;

    attr->sub.Attr.value = ajStrNewC("");
    attr->value = attr->sub.Attr.value;
    attr->sub.Attr.specified = 1;

    return attr;
}




/* @func ajDomDocumentCreateAttributeC ***************************************
**
** Create an attribute node
**
** @param [w] doc [AjPDomDocument] document
** @param [r] name [const char *] text
** @return [AjPDomAttr] attribute node
** @@
******************************************************************************/

AjPDomAttr ajDomDocumentCreateAttributeC(AjPDomDocument doc,
					 const char *name)
{
    AjPDomAttr attr = NULL;

    attr = ajDomDocumentCreateNode(doc, AJDOM_ATTRIBUTE_NODE);

    if(!attr)
	return NULL;

    attr->sub.Attr.name = ajStrNewC(name);
    attr->name = attr->sub.Attr.name;

    attr->sub.Attr.value = ajStrNewC("");
    attr->value = attr->sub.Attr.value;
    attr->sub.Attr.specified = 1;

    return attr;
}




/* @func ajDomDocumentCreateEntityReference **********************************
**
** Create an entity reference node
**
** @param [w] doc [AjPDomDocument] document
** @param [r] name [const AjPStr] text
** @return [AjPDomEntityReference] entity reference node
** @@
******************************************************************************/

AjPDomEntityReference ajDomDocumentCreateEntityReference(AjPDomDocument doc,
							 const AjPStr name)
{
    if(!name)
        return NULL;

    return ajDomDocumentCreateEntityReferenceC(doc,name->Ptr);
}




/* @func ajDomDocumentCreateEntityReferenceC *********************************
**
** Create an entity reference node
**
** @param [w] doc [AjPDomDocument] document
** @param [r] name [const char *] text
** @return [AjPDomEntityReference] entity reference node
** @@
******************************************************************************/

AjPDomEntityReference ajDomDocumentCreateEntityReferenceC(AjPDomDocument doc,
							  const char *name)
{
    AjPDomEntityReference eref = NULL;

    eref = ajDomDocumentCreateNode(doc, AJDOM_ENTITY_REFERENCE_NODE);

    if(!eref)
	return NULL;

    eref->name = ajStrNewC(name);

    return eref;
}




/* @func ajDomDocumentCreateProcessingInstruction ****************************
**
** Create a processing instruction node
**
** @param [w] doc [AjPDomDocument] document
** @param [r] target [const AjPStr] target
** @param [r] data [const AjPStr] data
** @return [AjPDomPi] entity reference node
** @@
******************************************************************************/

AjPDomPi ajDomDocumentCreateProcessingInstruction(AjPDomDocument doc,
						  const AjPStr target,
						  const AjPStr data)
{
    if(!target || !data)
        return NULL;

    return ajDomDocumentCreateProcessingInstructionC(doc,target->Ptr,
						     data->Ptr);
}




/* @func ajDomDocumentCreateProcessingInstructionC ***************************
**
** Create a processing instruction node
**
** @param [w] doc [AjPDomDocument] document
** @param [r] target [const char *] target
** @param [r] data [const char *] data
** @return [AjPDomPi] entity reference node
** @@
******************************************************************************/

AjPDomPi ajDomDocumentCreateProcessingInstructionC(AjPDomDocument doc,
						   const char *target,
						   const char *data)
{
    AjPDomPi pin = NULL;

    pin = ajDomDocumentCreateNode(doc,AJDOM_PROCESSING_INSTRUCTION_NODE);

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




/* @func ajDomDocumentGetElementsByTagName ***********************************
**
** Perform a preorder traversal of the entire document. Return a nodelist
** of the elements matching tagname in the order in which they are found.
**
** @param [u] doc [AjPDomDocument] document
** @param [r] name [const AjPStr] name
** @return [AjPDomNodeList] nodelist
** @@
******************************************************************************/

AjPDomNodeList ajDomDocumentGetElementsByTagName(AjPDomDocument doc,
						 const AjPStr name)
{
    AjPDomNodeList list = NULL;

    if(doc && doc->type == AJDOM_DOCUMENT_NODE && name &&
       (list=ajDomCreateNodeList(doc)))
    {
	domTraverse(list,doc->sub.Document.documentelement,name);
	return list;
    }

    return NULL;
}




/* @func ajDomDocumentGetElementsByTagNameC **********************************
**
** Perform a preorder traversal of the entire document. Return a nodelist
** of the elements matching tagname in the order in which they are found.
**
** @param [u] doc [AjPDomDocument] document
** @param [r] name [const char *] name
** @return [AjPDomNodeList] nodelist
** @@
******************************************************************************/

AjPDomNodeList ajDomDocumentGetElementsByTagNameC(AjPDomDocument doc,
						  const char *name)
{
    AjPDomNodeList list = NULL;

    if(doc && doc->type == AJDOM_DOCUMENT_NODE && name &&
       (list=ajDomCreateNodeList(doc)))
    {
	domTraverseC(list,doc->sub.Document.documentelement,name);

	return list;
    }

    return NULL;
}




/* @func ajDomDocumentGetDoctype *********************************************
**
** Get document type
**
** @param [r] doc [const AjPDomDocument] document
** @return [AjPDomDocumentType] doctype
** @@
******************************************************************************/

AjPDomDocumentType ajDomDocumentGetDoctype(const AjPDomDocument doc)
{
    return doc ? doc->sub.Document.doctype : NULL;
}




/* @func ajDomDocumentGetDocumentElement *************************************
**
** Get the root element of the document tree. The root element is also
** accessible through the childnodes nodelist member, however the children
** of a document may also be processing instructions, document type nodes,
** and comments which may precede the document element in the list. 
**
** @param [r] doc [const AjPDomDocument] document
** @return [AjPDomElement] root element
** @@
******************************************************************************/

AjPDomElement ajDomDocumentGetDocumentElement(const AjPDomDocument doc)
{
    return doc ? doc->sub.Document.documentelement : NULL;
}




/* @func ajDomPrintNode ******************************************************
**
** Print a node with indentation
**
** @param [r] node [const AjPDomNode] node
** @param [r] indent [ajint] indentation
** @return [void]
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

    for(i=0;i < indent; ++i)
	ajFmtPrint("    ");

    ajFmtPrint("%s: %S=%S\n",nodeinfo[node->type],node->name,node->value);

    if(node->type == AJDOM_ELEMENT_NODE && node->attributes->length)
    {
	ajFmtPrint("    ");
	n = ajDomNodeMapItem(node->attributes,0);
	ajFmtPrint("%S=%S",n->name,n->value);

	for(i=1;i<node->attributes->length;++i)
	{
	    n = ajDomNodeMapItem(node->attributes,i);
	    ajFmtPrint(",%S=%S",n->name,n->value);
	}
	ajFmtPrint("\n");

	for(i=0;i<indent;++i)
	    ajFmtPrint("    ");
    }

    for(n=node->firstchild; n; n=n->nextsibling)
	ajDomPrintNode(n,indent+1);

    return;
}




/* @func ajDomPrintNode2 *****************************************************
**
** Print a node with zero indentation
**
** @param [r] node [const AjPDomNode] node
** @return [void]
** @@
******************************************************************************/

void ajDomPrintNode2(const AjPDomNode node)
{
    ajFmtPrint("\n");
    ajDomPrintNode(node,0);

    return;
}




/* @func ajDomNodePrintNode **************************************************
**
** Print a node internals
**
** @param [r] node [const AjPDomNode] node
** @return [void]
** @@
******************************************************************************/

void ajDomNodePrintNode(const AjPDomNode node)
{
    if(!node)
    {
	ajFmtPrint("Node was null\n");

	return;
    }

    ajFmtPrint("\nName=%S,Value=%S,",node->name,node->value);
    ajFmtPrint("\n\tType=%d",node->type);

    ajFmtPrint(",Parentnode->name=%S,Firstchild->name=%S",
	       node->parentnode->name, node->firstchild->name);

    ajFmtPrint(",Lastchild->name=%S,\n\tChildnodes->length=%d",
	       node->lastchild->name,
	       (!node->childnodes ? 0 : node->childnodes->length));

    ajFmtPrint(",Previoussibling->name=%S,Nextsibling->name=%S,"
	       "Attributes->length=%d\n",
	       node->previoussibling->name,node->nextsibling->name,
	       (!node->attributes ? 0 : node->attributes->length));

    return;
}




/* @func ajDomNodeInsertBefore ***********************************************
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
	ajWarn("ajDomNodeInsertBefore: Wrong document\n");

	return NULL;
    }

    if(refchild && refchild->parentnode != node)
    {
	ajWarn("ajDomNodeInsertBefore: Hierarchy error\n");

	return NULL;
    }

    if(newchild->type == AJDOM_DOCUMENT_FRAGMENT_NODE)
    {
	for(n=newchild->firstchild; n; n=n->nextsibling)
	    if(AJDOM_CANTDO(node,n) || domIsAncestor(n,node))
	    {
		ajWarn("ajDomNodeInsertBefore: Hierarchy Request Error\n");

		return NULL;
	    }

	for(n=newchild->firstchild; n; n=nxt)
	{
	    nxt = n->nextsibling;
	    if(!domDoRemoveChild(newchild,n))
		return NULL;

	    if(!ajDomNodeInsertBefore(node,n,refchild))
	    {
		ajDomDocumentDestroyNode(n->ownerdocument,n);
		return NULL;
	    }
	}

	return newchild;
    }


    domDoRemoveChild(node,newchild);

    if(!(ajDomNodeListInsert(node->childnodes,newchild,refchild)))
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

    if(AJDOM_DOCMOD(node,newchild))
	node->sub.Document.documentelement = newchild;
    else if(AJDOM_DOCTYPEMOD(node,newchild))
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
	ajWarn("ajDomNodeListInsert: Filtered list error\n");

	return NULL;
    }

    if(refchild)
    {
	s = domDoLookupNode(list,refchild);
	if(!s || s->node != refchild)
	{
	    ajWarn("ajDomNodeListInsert: not found error\n");

	    return NULL;
	}
    }

    AJNEW0(e);
    
    domAddToMap(list,newchild,e);

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
	if(s==list->first)
	    list->first = e;
	else
	    s->prev->next = e;
	s->prev = e;
    }

    ++list->length;

    if(newchild->type == AJDOM_ATTRIBUTE_NODE)
	newchild->sub.Attr.ownerelement = list->ownerelement;

    return e;
}




/* @func ajDomNodeReplaceChild ***********************************************
**
** Replace oldchild with newchild in the list of children.
**
** @param [u] node [AjPDomNode] node
** @param [u] newchild [AjPDomNode] node to insert
** @param [u] oldchild [AjPDomNode] node to replace
** @return [AjPDomNode] oldchild node
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
	ajWarn("ajDomNodeReplaceChild: Wrong document\n");

	return NULL;
    }
    
    if(!ajDomNodeListExists(node->childnodes,oldchild))
    {
	ajWarn("ajDomNodeReplaceChild: Oldchild not found\n");

	return NULL;
    }

    if(newchild->type == AJDOM_DOCUMENT_FRAGMENT_NODE)
    {
	for(n=newchild->firstchild; n; n=n->nextsibling)
	    if(AJDOM_CANTDO(node,n) || domIsAncestor(n,node))
	    {
		ajWarn("ajDomNodeReplaceChild: Hierarchy Request Error\n");

		return NULL;
	    }

	for(n=newchild->firstchild; n; n=nxt)
	{
	    nxt = n->nextsibling;
	    if(!domDoRemoveChild(newchild,n))
		return NULL;

	    if(!ajDomNodeInsertBefore(node,n,oldchild))
	    {
		ajDomDocumentDestroyNode(n->ownerdocument,n);

		return NULL;
	    }
	}

	if(!domDoRemoveChild(node,oldchild))
	    return NULL;

	return oldchild;
    }


    if(AJDOM_CANTDO(node,newchild) || domIsAncestor(newchild,node))
    {
	ajWarn("ajDomNodeReplaceChild: Hierarchy Request Error\n");
	return NULL;
    }
    
    domDoRemoveChild(node,newchild);

    if(!ajDomNodeListExists(node->childnodes,oldchild))
	return NULL;

    ajDomNodeListReplace(node->childnodes,newchild,oldchild);

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

    if(AJDOM_DOCMOD(node,newchild))
	node->sub.Document.documentelement = newchild;
    else if(AJDOM_DOCTYPEMOD(node,newchild))
    {
	node->sub.Document.doctype = newchild;
	newchild->ownerdocument = node;
    }

    domUpdateNode(node);

    return oldchild;
}




/* @func ajDomNodeListReplace *******************************************
**
** Replace oldchild with newchild in a nodelist.
**
** @param [u] list [AjPDomNodeList] list
** @param [u] newchild [AjPDomNode] node to insert
** @param [u] oldchild [AjPDomNode] node to replace
** @return [AjPDomNodeEntry] inserted node
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
	ajWarn("ajDomNodeListReplace: Filtered list error\n");

	return NULL;
    }
    
    e = domDoLookupNode(list,oldchild);
    if(!e)
    {
	ajWarn("ajDomNodeListReplace: Not found error\n");

	return NULL;
    }

    domRemoveFromMap(list,oldchild);
    domAddToMap(list,newchild,e);

    e->node = newchild;

    if(oldchild->type == AJDOM_ATTRIBUTE_NODE)
	oldchild->sub.Attr.ownerelement = NULL;

    return e;
}




/* @funcstatic domNodeCloneNode **********************************************
**
** Low level clone node
**
** @param [u] ownerdocument [AjPDomDocument] owner document for clone
** @param [r] node [const AjPDomNode] node to clone
** @param [r] deep [AjBool] do a deep clone
** @return [AjPDomNode] clone
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
    case AJDOM_ELEMENT_NODE:
	clone = ajDomDocumentCreateElement(ownerdocument,node->name);

	if(clone)
	    for(e=node->attributes->first; e; e=e->next)
		if(!(ctmp = domNodeCloneNode(ownerdocument,e->node,deep)) ||
		   !ajDomNodeListAppend(clone->attributes,ctmp))
		{
		    ajDomDocumentDestroyNode(clone->ownerdocument,ctmp);
		    ajDomDocumentDestroyNode(clone->ownerdocument,clone);
		    return NULL;
		}
	break;

    case AJDOM_ATTRIBUTE_NODE:
	if((clone = ajDomDocumentCreateAttribute(ownerdocument,node->name)))
	{
	    clone->sub.Attr.specified = node->sub.Attr.specified;

	    if(!node->value)
		return NULL;

	    ajStrAssignS(&clone->value,node->value);
	    /* AJB: dirty */
	    clone->sub.Attr.value = clone->value;
	}

	break;

    case AJDOM_COMMENT_NODE:
	clone = ajDomDocumentCreateComment(ownerdocument,node->value);
	break;

    case AJDOM_TEXT_NODE:
	clone = ajDomDocumentCreateTextNode(ownerdocument,node->value);
	break;

    case AJDOM_CDATA_SECTION_NODE:
	clone = ajDomDocumentCreateCDATASection(ownerdocument,node->value);
	break;

    case AJDOM_DOCUMENT_FRAGMENT_NODE:
	clone = ajDomDocumentCreateDocumentFragment(ownerdocument);
	break;

    case AJDOM_DOCUMENT_NODE:
	clone = ownerdocument;
	break;

    case AJDOM_PROCESSING_INSTRUCTION_NODE:
	clone = ajDomDocumentCreateProcessingInstruction(ownerdocument,
			      node->sub.ProcessingInstruction.target,
			      node->sub.ProcessingInstruction.data);
	break;

    case AJDOM_ENTITY_NODE:
	if((clone = ajDomDocumentCreateNode(ownerdocument,AJDOM_ENTITY_NODE)))
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
	    
    case AJDOM_NOTATION_NODE:
	if((clone = ajDomDocumentCreateNode(ownerdocument,
					    AJDOM_NOTATION_NODE)))
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

    case AJDOM_DOCUMENT_TYPE_NODE:
	if((clone = ajDomImplementationCreateDocumentType(node->name,
							  NULL,NULL)))
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

    case AJDOM_ENTITY_REFERENCE_NODE:
	ajWarn("Entity reference clone not implemented\n");
	return NULL;
    }


    if(deep && clone && node->childnodes)
	for(ntmp=node->firstchild; ntmp; ntmp=ntmp->nextsibling)
	{
	    ctmp = domNodeCloneNode(ownerdocument,ntmp,ajTrue);

	    if(!ctmp || !ajDomNodeAppendChild(clone,ctmp))
	    {
		ajDomDocumentDestroyNode(clone->ownerdocument,ctmp);
		ajDomDocumentDestroyNode(clone->ownerdocument,clone);
		return NULL;
	    }
	}

    return clone;
}




/* @func ajDomNodeCloneNode **********************************************
**
** Clone node
**
** @param [u] node [AjPDomNode] node to clone
** @param [r] deep [AjBool] do a deep clone
** @return [AjPDomNode] clone
** @@
******************************************************************************/

AjPDomNode ajDomNodeCloneNode(AjPDomNode node, AjBool deep)
{
    AjPDomDocument doc = NULL;
    
    if(!node)
	return NULL;

    if(node->type == AJDOM_DOCUMENT_NODE)
    {
	if(!(doc = ajDomImplementationCreateDocument(NULL,NULL,NULL)))
	    return NULL;

	return domNodeCloneNode(doc,node,deep);
    }

    return domNodeCloneNode(node->ownerdocument,node,deep);
}




/* @funcstatic domWriteEncoded **********************************************
**
** Write a string using entity substitutions
**
** @param [r] s [const AjPStr] string
** @param [w] outf [AjPFile] output file
** @return [void]
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
	len = strcspn(p,"<>&\"");
	if(len)
	{
	    ajStrAssignSubC(&tmp,p,0,len-1);
	    ajFmtPrintF(outf,"%S",tmp);
	    p += len;
	}

	switch (*p)
	{
            case '\0':
                break;
            case '<':
                ajFmtPrintF(outf,"&lt;");
                break;
            case '>':
                ajFmtPrintF(outf,"&gt;");
                break;
            case '&':
                ajFmtPrintF(outf,"&apos;");
                break;
            case '"':
                ajFmtPrintF(outf,"&quot;");
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




/* @func ajDomNodeHasChildNodes **********************************************
**
** Check whether a node has any children
**
** @param [r] node [const AjPDomNode] node to clone
** @return [AjBool] true if children
** @@
******************************************************************************/

AjBool ajDomNodeHasChildNodes(const AjPDomNode node)
{
    return node != NULL && node->firstchild;
}




/* @func ajDomWrite **********************************************************
**
** Write XML from memory
**
** @param [r] node [const AjPDomDocument] document to write
** @param [u] outf [AjPFile] output file
** @return [ajint] zero OK, negative if error
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
        case AJDOM_ELEMENT_NODE:
            ajFmtPrintF(outf,"<");
            ajFmtPrintF(outf,"%S",node->name);

            for(e=node->attributes->first; e; e=e->next)
            {
                ajFmtPrintF(outf," %S=\"",e->node->name);
                domWriteEncoded(e->node->value,outf);
                ajFmtPrintF(outf,"\"");
            }
	
            if(ajDomNodeHasChildNodes(node))
            {
                ajFmtPrintF(outf,">");

                for(c=node->firstchild; c; c=c->nextsibling)
                    if(ajDomWrite(c,outf) == -1)
                        return -1;

                ajFmtPrintF(outf,"</");
                ajFmtPrintF(outf,"%S",node->name);
                ajFmtPrintF(outf,">");
            }
            else
                ajFmtPrintF(outf,"/>");

            break;

        case AJDOM_ATTRIBUTE_NODE:
            break;

        case AJDOM_TEXT_NODE:
            domWriteEncoded(node->value,outf);
            break;

        case AJDOM_CDATA_SECTION_NODE:
            break;

        case AJDOM_ENTITY_REFERENCE_NODE:
            break;

        case AJDOM_NOTATION_NODE:
            ajFmtPrintF(outf,"    <!NOTATION ");
            ajFmtPrintF(outf,"%S",node->name);

            if(node->sub.Notation.publicid)
            {
                ajFmtPrintF(outf," PUBLIC \"");
                ajFmtPrintF(outf,"%S",node->sub.Notation.publicid);
                ajFmtPrintF(outf,"\" \"");
                ajFmtPrintF(outf,"%S",node->sub.Notation.systemid);
                ajFmtPrintF(outf,"\"");
            }
            else if(node->sub.Notation.systemid)
            {
                ajFmtPrintF(outf," SYSTEM \"");
                ajFmtPrintF(outf,"%S",node->sub.Notation.systemid);
                ajFmtPrintF(outf,"\"");
            }

            ajFmtPrintF(outf,">");
            break;

        case AJDOM_ENTITY_NODE:
            ajFmtPrintF(outf,"    <!ENTITY ");
            ajFmtPrintF(outf,"%S",node->name);

            if(node->value)
            {
                ajFmtPrintF(outf,"\"");
                ajFmtPrintF(outf,"%S",node->value);
                ajFmtPrintF(outf,"\"");
            }
            else
            {
                if(node->sub.Entity.publicid)
                {
                    ajFmtPrintF(outf," PUBLIC \"");
                    ajFmtPrintF(outf,"%S",node->sub.Entity.publicid);
                    ajFmtPrintF(outf,"\" \"");
                    ajFmtPrintF(outf,"%S",node->sub.Entity.systemid);
                    ajFmtPrintF(outf,"\"");
                }
                else if(node->sub.Entity.systemid)
                {
                    ajFmtPrintF(outf," SYSTEM \"");
                    ajFmtPrintF(outf,"%S",node->sub.Entity.systemid);
                    ajFmtPrintF(outf,"\"");
                }

                if(node->sub.Entity.notationname)
                {
                    ajFmtPrintF(outf," NDATA ");
                    ajFmtPrintF(outf,"%S",node->sub.Entity.notationname);
                }
            }

            ajFmtPrintF(outf,">");
            break;

        case AJDOM_PROCESSING_INSTRUCTION_NODE:
            ajFmtPrintF(outf,"<?");
            ajFmtPrintF(outf,"%S ",node->sub.ProcessingInstruction.target);
            domWriteEncoded(node->sub.ProcessingInstruction.data,outf);
            ajFmtPrintF(outf,"?>");
            break;

        case AJDOM_COMMENT_NODE:
            ajFmtPrintF(outf,"<!--");
            domWriteEncoded(node->value,outf);
            ajFmtPrintF(outf,"-->");
            break;

        case AJDOM_DOCUMENT_NODE:
            ajFmtPrintF(outf,"<?xml");
            ajFmtPrintF(outf," version=\"");

            if(node->sub.Document.version)
                ajFmtPrintF(outf,"%S",node->sub.Document.version);
            else
                ajFmtPrintF(outf,"1.0");

            ajFmtPrintF(outf,"\"");

            if(node->sub.Document.standalone)
                ajFmtPrintF(outf," standalone=\"yes\"");

            ajFmtPrintF(outf,"?>\n");

            for(c=node->firstchild; c; c=c->nextsibling)
                if(ajDomWrite(c,outf) == -1)
                    return -1;

            ajFmtPrintF(outf,"\n");
            break;

        case AJDOM_DOCUMENT_TYPE_NODE:
            ajFmtPrintF(outf,"\n<!DOCTYPE ");
            ajFmtPrintF(outf,"%S",node->sub.DocumentType.name);

            if(node->sub.DocumentType.systemid)
            {
                ajFmtPrintF(outf," SYSTEM \"");
                ajFmtPrintF(outf,"%S",node->sub.DocumentType.systemid);
                ajFmtPrintF(outf,"\"");
            }
            else if(node->sub.DocumentType.publicid)
            {
                ajFmtPrintF(outf," PUBLIC \"");
                ajFmtPrintF(outf,"%S",node->sub.DocumentType.publicid);
                ajFmtPrintF(outf,"\"");
            }

            if(node->sub.DocumentType.internalsubset)
            {
                ajFmtPrintF(outf," [\n");
                ajFmtPrintF(outf,"%S",node->sub.DocumentType.internalsubset);
                ajFmtPrintF(outf,"]>\n");
            }
            else
                ajFmtPrintF(outf,">\n");

            break;

        case AJDOM_DOCUMENT_FRAGMENT_NODE:
            break;
    }

    return 0;
}




/* @func ajDomWriteIndent ****************************************************
**
** Write XML from memory
**
** @param [r] node [const AjPDomDocument] document to write
** @param [u] outf [AjPFile] output file
** @param [r] indent [ajint] indent level
** @return [ajint] zero OK, negative if error
** @@
******************************************************************************/

ajint ajDomWriteIndent(const AjPDomDocument node, AjPFile outf, ajint indent)
{
    AjPDomNodeEntry e = NULL;
    AjPDomNode c = NULL;
    ajint i;

    if(!node || !outf)
	return -1;

    for(i=0;i<indent;++i)
        ajFmtPrintF(outf," ");

    switch(node->type)
    {
        case AJDOM_ELEMENT_NODE:
            ajFmtPrintF(outf,"<");
            ajFmtPrintF(outf,"%S",node->name);

            for(e=node->attributes->first; e; e=e->next)
            {
                ajFmtPrintF(outf," %S=\"",e->node->name);
                domWriteEncoded(e->node->value,outf);
                ajFmtPrintF(outf,"\"");
            }
	
            if(ajDomNodeHasChildNodes(node))
            {
                ajFmtPrintF(outf,">\n");

                for(c=node->firstchild; c; c=c->nextsibling)
                    if(ajDomWriteIndent(c,outf,indent+2) == -1)
                        return -1;

                for(i=0;i<indent;++i)
                    ajFmtPrintF(outf," ");

                ajFmtPrintF(outf,"</");
                ajFmtPrintF(outf,"%S",node->name);
                ajFmtPrintF(outf,">\n");
            }
            else
                ajFmtPrintF(outf,"/>\n");

            break;

        case AJDOM_ATTRIBUTE_NODE:
            break;

        case AJDOM_TEXT_NODE:
            domWriteEncoded(node->value,outf);
            break;

        case AJDOM_CDATA_SECTION_NODE:
            break;

        case AJDOM_ENTITY_REFERENCE_NODE:
            break;

        case AJDOM_NOTATION_NODE:
            ajFmtPrintF(outf,"    <!NOTATION ");
            ajFmtPrintF(outf,"%S",node->name);

            if(node->sub.Notation.publicid)
            {
                ajFmtPrintF(outf," PUBLIC \"");
                ajFmtPrintF(outf,"%S",node->sub.Notation.publicid);
                ajFmtPrintF(outf,"\" \"");
                ajFmtPrintF(outf,"%S",node->sub.Notation.systemid);
                ajFmtPrintF(outf,"\"");
            }
            else if(node->sub.Notation.systemid)
            {
                ajFmtPrintF(outf," SYSTEM \"");
                ajFmtPrintF(outf,"%S",node->sub.Notation.systemid);
                ajFmtPrintF(outf,"\"");
            }

            ajFmtPrintF(outf,">\n");
            break;

        case AJDOM_ENTITY_NODE:
            ajFmtPrintF(outf,"    <!ENTITY ");
            ajFmtPrintF(outf,"%S",node->name);

            if(node->value)
            {
                ajFmtPrintF(outf,"\"");
                ajFmtPrintF(outf,"%S",node->value);
                ajFmtPrintF(outf,"\"");
            }
            else
            {
                if(node->sub.Entity.publicid)
                {
                    ajFmtPrintF(outf," PUBLIC \"");
                    ajFmtPrintF(outf,"%S",node->sub.Entity.publicid);
                    ajFmtPrintF(outf,"\" \"");
                    ajFmtPrintF(outf,"%S",node->sub.Entity.systemid);
                    ajFmtPrintF(outf,"\"");
                }
                else if(node->sub.Entity.systemid)
                {
                    ajFmtPrintF(outf," SYSTEM \"");
                    ajFmtPrintF(outf,"%S",node->sub.Entity.systemid);
                    ajFmtPrintF(outf,"\"");
                }

                if(node->sub.Entity.notationname)
                {
                    ajFmtPrintF(outf," NDATA ");
                    ajFmtPrintF(outf,"%S",node->sub.Entity.notationname);
                }
            }

            ajFmtPrintF(outf,">\n");
            break;

        case AJDOM_PROCESSING_INSTRUCTION_NODE:
            ajFmtPrintF(outf,"<?");
            ajFmtPrintF(outf,"%S ",node->sub.ProcessingInstruction.target);
            domWriteEncoded(node->sub.ProcessingInstruction.data,outf);
            ajFmtPrintF(outf,"?>\n");
            break;

        case AJDOM_COMMENT_NODE:
            ajFmtPrintF(outf,"<!--");
            domWriteEncoded(node->value,outf);
            ajFmtPrintF(outf,"-->\n");
            break;

        case AJDOM_DOCUMENT_NODE:
            ajFmtPrintF(outf,"<?xml");
            ajFmtPrintF(outf," version=\"");

            if(node->sub.Document.version)
                ajFmtPrintF(outf,"%S",node->sub.Document.version);
            else
                ajFmtPrintF(outf,"1.0");

            ajFmtPrintF(outf,"\"");

            if(node->sub.Document.standalone)
                ajFmtPrintF(outf," standalone=\"yes\"");

            ajFmtPrintF(outf,"?>\n");

            for(c=node->firstchild; c; c=c->nextsibling)
                if(ajDomWriteIndent(c,outf,indent+2) == -1)
                    return -1;

            ajFmtPrintF(outf,"\n");
            break;

        case AJDOM_DOCUMENT_TYPE_NODE:
            ajFmtPrintF(outf,"\n<!DOCTYPE ");
            ajFmtPrintF(outf,"%S",node->sub.DocumentType.name);

            if(node->sub.DocumentType.systemid)
            {
                ajFmtPrintF(outf," SYSTEM \"");
                ajFmtPrintF(outf,"%S",node->sub.DocumentType.systemid);
                ajFmtPrintF(outf,"\"");
            }
            else if(node->sub.DocumentType.publicid)
            {
                ajFmtPrintF(outf," PUBLIC \"");
                ajFmtPrintF(outf,"%S",node->sub.DocumentType.publicid);
                ajFmtPrintF(outf,"\"");
            }

            if(node->sub.DocumentType.internalsubset)
            {
                ajFmtPrintF(outf," [\n");
                ajFmtPrintF(outf,"%S",node->sub.DocumentType.internalsubset);
                ajFmtPrintF(outf,"]>\n");
            }
            else
                ajFmtPrintF(outf,">\n");

            break;

        case AJDOM_DOCUMENT_FRAGMENT_NODE:
            break;
    }

    return 0;
}
