#ifdef __cplusplus
extern "C"
{
#endif

#ifndef ajdom_h
#define ajdom_h


#define AJDOM_ELEMENT_NODE                1
#define AJDOM_ATTRIBUTE_NODE              2
#define AJDOM_TEXT_NODE                   3
#define AJDOM_CDATA_SECTION_NODE          4
#define AJDOM_ENTITY_REFERENCE_NODE       5
#define AJDOM_ENTITY_NODE                 6
#define AJDOM_PROCESSING_INSTRUCTION_NODE 7
#define AJDOM_COMMENT_NODE                8
#define AJDOM_DOCUMENT_NODE               9
#define AJDOM_DOCUMENT_TYPE_NODE          10
#define AJDOM_DOCUMENT_FRAGMENT_NODE      11
#define AJDOM_NOTATION_NODE               12




/* @data AjSDomNode **********************************************************
**
** DOM parsing node
**
** @alias AjODomNode
** @alias AjPDomNode
**
** @attr name [AjPStr] name of node
** @attr value [AjPStr] value associated with name
** @attr parentnode [struct AjSDomNode*] ancestor
** @attr childnodes [struct AjSDomNodeList*] children
** @attr firstchild [struct AjSDomNode*] first child in list
** @attr lastchild [struct AjSDomNode*] last child in list
** @attr previoussibling [struct AjSDomNode*] last node at this level
** @attr nextsibling [struct AjSDomNode*] next node at this level
** @attr attributes [struct AjSDomNodeList*] attributes
** @attr ownerdocument [struct AjSDomNode*] document owner
** @attr sub [union] substructure depending on node type
** @attr type [ajuint] type of node
** @attr subtreeModified [ajuint] internal flag
**
******************************************************************************/

typedef struct AjSDomNode
{
    AjPStr name;
    AjPStr value;

    struct AjSDomNode     *parentnode;
    struct AjSDomNodeList *childnodes;
    struct AjSDomNode     *firstchild;
    struct AjSDomNode     *lastchild;
    struct AjSDomNode     *previoussibling;
    struct AjSDomNode     *nextsibling;

    struct AjSDomNodeList *attributes;

    struct AjSDomNode     *ownerdocument;



    union 
    {
	struct
	{
	    struct AjSDomNode   *doctype;
	    struct AjSDomNode   *documentelement;
	    struct AjSDomNode   *document;
	    struct AjSDomNode   *defaultview;
	    struct AjSDomNode   *commonparent;
	    AjPStr version;
	    AjPStr encoding;
	    ajint standalone;
	    char  Padding[4];
	} Document;

	struct
	{
	    AjPStr name;
	    struct AjSDomNodeList *entities;
	    struct AjSDomNodeList *notations;
	    AjPStr publicid;
	    AjPStr systemid;
	    AjPStr internalsubset;
	} DocumentType;

	struct 
	{
	    AjPStr tagname;
	} Element;

	struct
	{
	    AjPStr name;
	    AjPStr value;
	    struct AjSDomNode *ownerelement;
	    ajint specified;
	    char Padding[4];
	} Attr;

	struct
	{
	    AjPStr data;
	    ajint length;
            char Padding[4];
	} CharacterData;

	struct
	{
	    AjPStr publicid;
	    AjPStr systemid;
	} Notation;

	struct
	{
	    AjPStr publicid;
	    AjPStr systemid;
	    AjPStr notationname;
	} Entity;

	struct
	{
	    AjPStr target;
	    AjPStr data;
	} ProcessingInstruction;
    } sub;

    ajuint type;
    ajuint subtreeModified;

} AjODomNode;

#define AjPDomNode AjODomNode*




/* @data AjSDomNodeEntry ******************************************************
**
** DOM parsing node entry
**
** @alias AjODomNodeEntry
** @alias AjPDomNodeEntry
**
** @attr prev [struct AjSDomNodeEntry*] previous node
** @attr next [struct AjSDomNodeEntry*] next node
** @attr node [AjPDomNode] linked list node
**
******************************************************************************/

typedef struct AjSDomNodeEntry
{
    struct AjSDomNodeEntry *prev;
    struct AjSDomNodeEntry *next;
    AjPDomNode node;
} AjODomNodeEntry;

#define AjPDomNodeEntry AjODomNodeEntry*




/* @data AjSDomNodeList ******************************************************
**
** DOM parsing node list
**
** @alias AjODomNodeList
** @alias AjPDomNodeList
**
** @attr ownerdocument [AjPDomNode] document owner
** @attr ownerelement [AjPDomNode] element owner
** @attr first [AjPDomNodeEntry] first in list
** @attr last [AjPDomNodeEntry] last in list
** @attr length [ajint] length
** @attr filter [ajuint] filter
** @attr list [struct AjSDomNodeList*] list
** @attr table [AjPTable] fast lookup table
**
******************************************************************************/

typedef struct AjSDomNodeList 
{
    AjPDomNode ownerdocument;
    AjPDomNode ownerelement;
    AjPDomNodeEntry first;
    AjPDomNodeEntry last;
    ajint length;
    ajuint filter;
    struct AjSDomNodeList *list;
    AjPTable table;
} AjODomNodeList;

#define AjPDomNodeList AjODomNodeList*




/* @data AjSDomUserdata ******************************************************
**
** DOM Userdata used when reading XML
**
** @alias AjODomUserdata
** @alias AjPDomUserdata
**
** @attr Buffer [AjPStr] Utility string
** @attr Stack [AjPList] Stack
** @attr Cdata [AjBool] Cdata
** @attr Padding [AjBool] Pad to alignment boundary
**
******************************************************************************/

typedef struct AjSDomUserdata 
{
    AjPStr  Buffer;
    AjPList Stack;
    AjBool  Cdata;
    AjBool  Padding;
} AjODomUserdata;

#define AjPDomUserdata AjODomUserdata*




#define AjPDomDocument AjPDomNode
#define AjPDomNodeMap AjPDomNodeList
#define AjPDomDocumentType AjPDomNode
#define AjPDomElement AjPDomNode
#define AjPDomText AjPDomNode
#define AjPDomCharacterData AjPDomNode
#define AjPDomDocumentFragment AjPDomNode
#define AjPDomCDATASection AjPDomNode
#define AjPDomComment AjPDomNode
#define AjPDomAttr AjPDomNode
#define AjPDomEntityReference AjPDomNode
#define AjPDomEntity AjPDomNode
#define AjPDomPi AjPDomNode
#define AjPDomNotation AjPDomNode




/*
** Prototype definitions
*/


AjPDomNode ajDomDocumentCreateNode(AjPDomDocument doc, ajuint nodetype);

AjPDomDocumentType ajDomImplementationCreateDocumentType(const AjPStr qualname,
							 const AjPStr publicid,
							const AjPStr systemid);
AjPDomDocumentType ajDomImplementationCreateDocumentTypeC(const char *qualname,
							  const char *publicid,
							 const char *systemid);

AjPDomDocument  ajDomImplementationCreateDocument(const AjPStr uri,
						  const AjPStr qualname,
						  AjPDomDocumentType doctype);
AjPDomDocument ajDomImplementationCreateDocumentC(const char *uri,
						  const char *qualname,
						  AjPDomDocumentType doctype);

ajint ajDomNodeListGetLen(const AjPDomNodeList list);

AjPDomNodeEntry ajDomNodeListAppend(AjPDomNodeList list,
				    AjPDomNode child);

AjPStr          ajDomElementGetAttribute(const AjPDomElement element,
					 const AjPStr name);
AjPStr          ajDomElementGetAttributeC(const AjPDomElement element,
				          const char *name);

void            ajDomElementSetAttribute(const AjPDomElement element,
					 const AjPStr name,
					 const AjPStr value);
void            ajDomElementSetAttributeC(const AjPDomElement element,
					  const char *name,
			                  const char *value);
void            ajDomElementRemoveAttribute(AjPDomElement element,
					    const AjPStr name);
void            ajDomElementRemoveAttributeC(AjPDomElement element,
					     const char *name);

AjPDomNode ajDomElementRemoveAttributeNode(AjPDomElement element,
					   AjPDomNode oldattr);
AjPDomNode     ajDomElementGetAttributeNode(const AjPDomElement element,
					    const AjPStr name);
AjPDomNode     ajDomElementGetAttributeNodeC(const AjPDomElement element,
					     const char *name);

AjPDomNode     ajDomElementSetAttributeNode(AjPDomElement element,
					    AjPDomNode newattr);
AjPDomNodeList ajDomElementGetElementsByTagName(AjPDomElement element,
						const AjPStr name);
AjPDomNodeList ajDomElementGetElementsByTagNameC(AjPDomElement element,
						 const char *name);

void           ajDomElementNormalise(AjPDomElement element);
AjPStr         ajDomCharacterDataSubstringData(const AjPDomCharacterData data,
					       ajint offset, ajint count);
void           ajDomCharacterDataAppendData(AjPDomCharacterData data,
					    const AjPStr arg);
void           ajDomCharacterDataAppendDataC(AjPDomCharacterData data,
					     const char *arg);
void           ajDomCharacterDataInsertData(AjPDomCharacterData data,
					    ajint offset, const AjPStr arg);
void           ajDomCharacterDataInsertDataC(AjPDomCharacterData data,
					     ajint offset, const char *arg);

void           ajDomCharacterDataDeleteData(AjPDomCharacterData data,
					    ajint offset, ajint count);
void           ajDomCharacterDataReplaceData(AjPDomCharacterData data,
					     ajint offset, ajint count,
					     const AjPStr arg);
void           ajDomCharacterDataReplaceDataC(AjPDomCharacterData data,
					      ajint offset, ajint count,
					      const char *arg);

ajint          ajDomCharacterDataGetLength(const AjPDomCharacterData data);
AjPDomText     ajDomTextSplitText(AjPDomText text, ajint offset);
AjPDomElement  ajDomDocumentCreateElement(AjPDomDocument doc,
					  const AjPStr tagname);
AjPDomElement ajDomDocumentCreateElementC(AjPDomDocument doc,
					  const char *tagname);

AjPDomDocumentFragment ajDomDocumentCreateDocumentFragment(AjPDomDocument doc);

AjPDomText ajDomDocumentCreateTextNode(AjPDomDocument doc, const AjPStr data);
AjPDomText ajDomDocumentCreateTextNodeC(AjPDomDocument doc, const char *data);

AjPDomComment ajDomDocumentCreateComment(AjPDomDocument doc,
					 const AjPStr data);
AjPDomComment ajDomDocumentCreateCommentC(AjPDomDocument doc,
					  const char *data);

AjPDomCDATASection ajDomDocumentCreateCDATASection(AjPDomDocument doc,
						   const AjPStr data);
AjPDomCDATASection ajDomDocumentCreateCDATASectionC(AjPDomDocument doc,
						    const char *data);

AjPDomAttr ajDomDocumentCreateAttribute(AjPDomDocument doc,
					const AjPStr name);
AjPDomAttr ajDomDocumentCreateAttributeC(AjPDomDocument doc,
					 const char *name);

AjPDomEntityReference ajDomDocumentCreateEntityReference(AjPDomDocument doc,
							 const AjPStr name);
AjPDomEntityReference ajDomDocumentCreateEntityReferenceC(AjPDomDocument doc,
							  const char *name);

AjPDomPi ajDomDocumentCreateProcessingInstruction(AjPDomDocument doc,
						  const AjPStr target,
						  const AjPStr data);
AjPDomPi ajDomDocumentCreateProcessingInstructionC(AjPDomDocument doc,
						   const char *target,
						   const char *data);



AjPDomNodeList ajDomDocumentGetElementsByTagName(AjPDomDocument doc,
						 const AjPStr name);
AjPDomNodeList ajDomDocumentGetElementsByTagNameC(AjPDomDocument doc,
						  const char *name);

AjPDomDocumentType ajDomDocumentGetDoctype(const AjPDomDocument doc);
AjPDomElement      ajDomDocumentGetDocumentElement(const AjPDomDocument doc);

AjPDomNode         ajDomNodeAppendChild(AjPDomNode node,
					AjPDomNode extrachild);
AjPDomNode         ajDomNodeInsertBefore(AjPDomNode node, AjPDomNode newchild,
			                 AjPDomNode refchild);
AjPDomNode         ajDomRemoveChild(AjPDomNode node, AjPDomNode child);
void               ajDomDocumentDestroyNode(AjPDomDocument doc,
					    AjPDomNode node);
void               ajDomDocumentDestroyNodeList(AjPDomDocument doc,
						AjPDomNodeList list,
						AjBool donodes);
AjPDomNode         ajDomNodeReplaceChild(AjPDomNode node, AjPDomNode newchild,
					 AjPDomNode oldchild);
AjPDomNode         ajDomNodeCloneNode(AjPDomNode node, AjBool deep);
AjPDomNode         ajDomNodeListItem(const AjPDomNodeList list, ajint indexnum);
AjBool             ajDomNodeListExists(AjPDomNodeList list,
				       const AjPDomNode child);
AjPDomNodeEntry    ajDomNodeListRemove(AjPDomNodeList list, AjPDomNode child);
AjPDomNodeList     ajDomCreateNodeList(AjPDomDocument doc);

AjPDomNode         ajDomNodeMapGetItem(const AjPDomNodeMap map,
				       const AjPStr name);
AjPDomNode         ajDomNodeMapGetItemC(const AjPDomNodeMap map,
					const char *name);
AjPDomNode         ajDomNodeMapSetItem(AjPDomNodeMap map, AjPDomNode arg);

AjPDomNode         ajDomNodeMapRemoveItem(AjPDomNodeMap map,
			 		  const AjPStr name);
AjPDomNode         ajDomNodeMapRemoveItemC(AjPDomNodeMap map,
			 		   const char *name);
AjPDomNode         ajDomNodeMapItem(const AjPDomNodeMap map, ajint indexnum);

AjPDomNodeEntry  ajDomNodeListInsert(AjPDomNodeList list, AjPDomNode newchild,
				     AjPDomNode refchild);
AjPDomNodeEntry  ajDomNodeListReplace(AjPDomNodeList list, AjPDomNode newchild,
				      AjPDomNode oldchild);

AjBool ajDomNodeHasChildNodes(const AjPDomNode node);


ajint ajDomWrite(const AjPDomDocument node, AjPFile outf);
ajint ajDomWriteIndent(const AjPDomDocument node, AjPFile outf, ajint indent);



void ajDomPrintNode(const AjPDomNode node, ajint indent);
void ajDomPrintNode2(const AjPDomNode node);
void ajDomNodePrintNode(const AjPDomNode node);


ajint ajDomReadFp(AjPDomDocument node, FILE *stream);
ajint ajDomReadFilebuff(AjPDomDocument node, AjPFilebuff buff);
ajint ajDomReadString(AjPDomDocument node, AjPStr str);
    



/*
** End of prototype definitions
*/



#endif


#ifdef __cplusplus
}
#endif
