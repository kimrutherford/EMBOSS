/********************************************************************
** @source AJAX structure functions
**
** @author Copyright (C) 2002 Hugh Morgan (hmorgan2@hgmp.mrc.ac.uk)
** @author Copyright (C) 2002 Alan Bleasby
** @version 1.0 
** @modified Feb 27 HLM First version
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
********************************************************************/

#include "ajax.h"
#include <math.h>

#define ONE_METER_FONT 1.784
#define RESIDUE_SQUARE_SIZE 1.7
#define SQUARESIZE 0.03

#ifdef GROUT


static AjPXmlNode	xml_MakeNewNode(AjPGraphXml file,const AjPStr name,
					AjPXmlNode parent);
static AjPXmlNode	xml_MakeNewNodeC(AjPGraphXml file, const char *name, 
					 AjPXmlNode parent);

static AjPXmlNode 	xml_MakeNewShapeNode(AjPGraphXml file,
					     AjPXmlNode parentNode,
					     const AjPStr nameReqd);
static AjPXmlNode 	xml_MakeNewShapeNodeC(AjPGraphXml file,
					      AjPXmlNode parentNode,
					      const char *nameReqd);
static AjPXmlNode 	xml_GetNodeTypeMakeIfNot(AjPGraphXml file,
						 const AjPStr nameReqd);
static AjPXmlNode 	xml_GetNodeTypeMakeIfNotC(AjPGraphXml file,
						  const char *nameReqd);
static AjPGraphXml 	xml_CreateNewOutputFile();
static AjPXmlNode 	xml_GetCurrentGraphic(const AjPGraphXml file);
static AjPXmlNode 	xml_GetCurrentScene(const AjPGraphXml file);
static AjPXmlNode 	xml_SetNode(GdomeNode *node);
static GdomeNode* 	xml_GetNode(AjPXmlNode node);
static GdomeElement* 	xml_GetNodeElement(AjPXmlNode node);
static AjPXmlNode 	xml_GetParent(AjPXmlNode node);

static void 	xml_AddCylinder(AjPGraphXml file, double xCentre,
				double yCentre, double angle, double height,
				double width);
static AjPStr 	xml_StrFromBool(AjBool boole);
static AjBool 	xml_StrFromDouble(AjPStr *result, double val);
static AjBool 	xml_AngleIsInSecondHalfOfCircle(double angle);
static AjBool 	xml_PresentGraphicTypeIs(AjPGraphXml file, const AjPStr name);
static AjBool 	xml_PresentGraphicTypeIsC(AjPGraphXml file, const char *name);
static AjPStr 	xml_GetAttribute(AjPXmlNode node, const AjPStr atName);
static AjPStr 	xml_GetAttributeC(AjPXmlNode node, const char *atName);
static void 	xml_SetAttribute(AjPXmlNode node, const AjPStr atName,
				 const AjPStr atValue);
static void 	xml_SetAttributeC(AjPXmlNode node, const char *atName,
				  const char *atValue);
static AjPStr 	xml_GetIndex(AjPXmlNode node);
static void 	xml_SetIndex(AjPXmlNode node, const AjPStr index);
static AjPStr 	xml_GetPoints(AjPXmlNode node);
static AjBool 	xml_SetPoints(AjPXmlNode node, const AjPStr points);

static AjPStr 	xml_PresentColourAsString(const AjPGraphXml file);
static AjBool 	xml_FileNeedsProtoDeclare(const AjPGraphXml file,
					  const AjPStr protoName);
static AjBool 	xml_FileNeedsProtoDeclareC(const AjPGraphXml file,
					   const char *protoName);
static AjBool 	xml_IsShapeThisColour(AjPGraphXml file,
				      AjPXmlNode shape);

static void 	xml_AddArc(AjPGraphXml file, double xCentre, double yCentre,
			   double startAngle, double endAngle, double radius);

static void 	xml_AddACoord(double x, double y, AjBool joined, AjPStr* coord,
			      AjPStr* index);
static int 	xml_GetLastInt(const AjPStr str);
static double 	xml_GetLastDouble(const AjPStr str);
static double 	xml_GetDoubleNo(const AjPStr str, int index);

static void 	xml_AddGraphProto(AjPGraphXml file);
static void 	xml_AddDNAPlotProto(AjPGraphXml file);
static void 	xml_AddCommonBit(AjPGraphXml file);
static AjBool 	xml_WriteFile(const AjPGraphXml file, const AjPStr filename);
static AjBool 	xml_WriteStdout(const AjPGraphXml file);

static void 	xml_SetCurrentGraphic(AjPGraphXml file, AjPXmlNode node);
static void 	xml_SetCurrentScene(AjPGraphXml file, AjPXmlNode node);
static void 	xml_ClearFile(AjPGraphXml *file);
static void 	xml_UnrefNode(AjPXmlNode *pnode);

static void     xml_clear_nodeTypes(void **key, void **value, void *cl);
static void     xml_deltablenode(void **key, void **value, void *cl);




/* @func ajXmlNodeNew ***********************************************
**
** Create a new node and allocate the memory
**
** @return [AjPXmlNode] the new node
** @@
*********************************************************************/

AjPXmlNode ajXmlNodeNew()
{
    AjPXmlNode thys;
    AJNEW0(thys);
    
    return thys;
}




/* @func ajXmlFileNew ***********************************************
**
** Create a new file and allocate the memory
**
** @return [AjPGraphXml] the new file
** @@
*********************************************************************/

AjPGraphXml ajXmlFileNew()
{
    AjPGraphXml thys;

    AJNEW0(thys);

    return thys;
}




/* @func ajXmlNodeDel ***********************************************
**
** Delete the node
**
** @param [d] thys [AjPXmlNode*] the node to delete
**
** @return [void] 
** @@
*********************************************************************/

void ajXmlNodeDel(AjPXmlNode *thys)
{
    if(!thys || !*thys)
	return;

    AJFREE(*thys);
    *thys = NULL;
    
    return;
}




/* @func ajXmlFileDel ***********************************************
**
** Delete the file
**
** @param [d] thys [AjPGraphXml*] the file to delete
**
** @return [void] 
** @@
*********************************************************************/

void ajXmlFileDel(AjPGraphXml *thys)
{
    if(!thys || !*thys)
	return;


    if((*thys)->nodeTypes)
    {
	ajTableMapDel((*thys)->nodeTypes,xml_clear_nodeTypes,NULL);
	ajTableFree(&(*thys)->nodeTypes);
    }


    AJFREE(*thys);
    *thys = NULL;
    
    return;
}




/* @func ajXmlSetMaxMin *********************************************
**
** sets the max min values of a graph
**
** @param [w] file [AjPGraphXml] the file to add the max min values to
** @param [r] xMin [double] the x min value
** @param [r] yMin [double] the y min value
** @param [r] xMax [double] the x max value
** @param [r] yMax [double] the y max value
**
** @return [AjBool] ajTrue if the current Graphics object is valid to 
** add MaxMin, eg. a graph
** @@
*********************************************************************/

AjBool ajXmlSetMaxMin (AjPGraphXml file, double xMin, double yMin, 
		       double xMax, double yMax)
{
    AjPXmlNode minMaxNode;
    AjPStr doub  = NULL;
    AjPStr value = NULL;
    
    value = ajStrNewC("[");
    doub  = ajStrNew();
    

    if(!xml_PresentGraphicTypeIsC(file, "Graph"))
    {
	ajStrDel(&doub);
	ajStrDel(&value);
	return ajFalse;
    }
    

    minMaxNode = xml_MakeNewNodeC(file, "fieldValue", 
				 xml_GetCurrentGraphic(file));

    xml_StrFromDouble(&doub, xMin);
    ajStrAppendS(&value, doub);  
    ajStrAppendC(&value, ", ");  


    xml_StrFromDouble(&doub, yMin);
    ajStrAppendS(&value, doub);  
    ajStrAppendC(&value, ", ");  


    xml_StrFromDouble(&doub, xMax);
    ajStrAppendS(&value, doub);  
    ajStrAppendC(&value, ", ");  


    xml_StrFromDouble(&doub, yMax);
    ajStrAppendS(&value, doub);  
    ajStrAppendC(&value, "]");  


    xml_SetAttributeC(minMaxNode, "fieldName", "MaxMin");
    xml_SetAttributeC(minMaxNode, "value", ajStrGetPtr(value));
    
    xml_UnrefNode(&minMaxNode);
    

    ajStrDel(&doub);
    ajStrDel(&value);

    return ajTrue;
}




/* @func ajXmlWriteFile **********************************************
**
** writes the file 
**
** @param [w] file [AjPGraphXml] the file to write
** @param [r] filename [const AjPStr] the filename to write
**
** @return [AjBool] ajTrue if file correctly written
** @@
*********************************************************************/

AjBool ajXmlWriteFile(AjPGraphXml file, const AjPStr filename)
{
    return xml_WriteFile(file, filename);
}




/* @func ajXmlWriteStdout ********************************************
**
** writes the file 
**
** @param [w] file [AjPGraphXml] the file to write
**
** @return [AjBool] ajTrue if file correctly written
** @@
*********************************************************************/

AjBool ajXmlWriteStdout(AjPGraphXml file)
{
    return xml_WriteStdout(file);
}




/* @func ajXmlClearFile **********************************************
**
** Clears the memory allocated to this file
**
** Needs a reference so that the pointer in the caller will be zeroed.
**
** @param [f] pfile [AjPGraphXml*] the file to clear
**
** @return [void]
** @@
*********************************************************************/

void ajXmlClearFile(AjPGraphXml *pfile)
{
    xml_ClearFile(pfile);

    return;
}




/* @func ajXmlSetSource *********************************************
**
** Adds the source to a graph object
**
** @param [w] file [AjPGraphXml] the file to add the title to
** @param [r] title [const AjPStr] the title
**
** @return [AjBool] ajTrue if the current Graphics object is valid to 
** add Source, eg. a graph
** @@
*********************************************************************/

AjBool ajXmlSetSource(AjPGraphXml file, const AjPStr title)
{
    AjPXmlNode titleNode;

    if(!xml_PresentGraphicTypeIsC(file,"Graph"))
	return ajFalse;

    titleNode = xml_MakeNewNodeC(file, "fieldValue", 
				xml_GetCurrentGraphic(file));
    xml_SetAttributeC(titleNode, "fieldName", "Source");
    xml_SetAttributeC(titleNode, "value", ajStrGetPtr(title));

    xml_UnrefNode(&titleNode);

    return ajTrue;
}




/* @func ajXmlAddMainTitleC ******************************************
**
** Adds a title to a graph object
**
** @param [w] file [AjPGraphXml] the file to add the title to
** @param [r] title [const char *] the title
**
** @return [AjBool] ajTrue if the current Graphics object is valid to 
** add Source, eg. a graph
** @@
*********************************************************************/

AjBool ajXmlAddMainTitleC(AjPGraphXml file, const char *title)
{
    AjPXmlNode graphNode;
    AjPStr titleAltered = NULL;

    /* Not sure about this hugh */
    titleAltered = ajStrNew();
    ajFmtPrintS(&titleAltered,"\"%s\"",title);

    graphNode = xml_MakeNewNodeC(file, "fieldValue", 
				xml_GetCurrentGraphic(file));

    xml_SetAttributeC(graphNode, "name", "Graph.mainTitle");
    xml_SetAttributeC(graphNode, "value", ajStrGetPtr(titleAltered));

    xml_UnrefNode(&graphNode);

    ajStrDel(&titleAltered);

    return ajTrue;
}




/* @func ajXmlAddXTitleC *********************************************
**
** Adds a title to a graph object
**
** @param [w] file [AjPGraphXml] the file to add the title to
** @param [r] title [const char *] the title
**
** @return [AjBool] ajTrue if the current Graphics object is valid to 
** add XTitle, eg. a graph
** @@
*********************************************************************/

AjBool ajXmlAddXTitleC (AjPGraphXml file, const char *title)
{
    AjPXmlNode titleNode;
    AjPStr titleAltered = NULL;


    /* Not sure about this hugh */
    titleAltered = ajStrNew();
    ajFmtPrintS(&titleAltered,"\"%s\"",title);

    if(!xml_PresentGraphicTypeIsC(file,"Graph"))
	return ajFalse;

    titleNode = xml_MakeNewNodeC(file, "fieldValue", 
				xml_GetCurrentGraphic(file));
    xml_SetAttributeC(titleNode, "name", "Graph.xTitle");
    xml_SetAttributeC(titleNode, "value", ajStrGetPtr(titleAltered));

    xml_UnrefNode(&titleNode);
    ajStrDel(&titleAltered);

    return ajTrue;
}




/* @func ajXmlAddYTitleC *********************************************
**
** Adds a title to a graph object
**
** @param [w] file [AjPGraphXml] the file to add the title to
** @param [r] title [const char *] the title
**
** @return [AjBool] ajTrue if the current Graphics object is valid to
** add YTitle, eg. a graph
** @@
*********************************************************************/

AjBool ajXmlAddYTitleC(AjPGraphXml file, const char *title)
{
    AjPXmlNode titleNode;
    AjPStr titleAltered = NULL;

    /* Not sure about this hugh */
    titleAltered = ajStrNew();
    ajFmtPrintS(&titleAltered,"\"%s\"",title);

    if(!xml_PresentGraphicTypeIsC(file,"Graph"))
	return ajFalse;

    titleNode = xml_MakeNewNodeC(file, "fieldValue", 
				xml_GetCurrentGraphic(file));
    xml_SetAttributeC(titleNode, "name", "Graph.yTitle");
    xml_SetAttributeC(titleNode, "value", ajStrGetPtr(titleAltered));

    xml_UnrefNode(&titleNode);
    ajStrDel(&titleAltered);

    return ajTrue;
}




/* @func ajXmlAddMainTitle *****************************************
**
** Adds a title to a graph object
**
** @param [w] file [AjPGraphXml] the file to add the title to
** @param [r] title [const AjPStr] the title
**
** @return [AjBool] ajTrue if the current Graphics object is valid to 
** add Source, eg. a graph
** @@
*********************************************************************/

AjBool ajXmlAddMainTitle(AjPGraphXml file, const AjPStr title)
{
    return ajXmlAddMainTitleC(file, ajStrGetPtr(title));
}




/* @func ajXmlAddXTitle ********************************************
**
** Adds a title to a graph object
**
** @param [w] file [AjPGraphXml] the file to add the title to
** @param [r] title [const AjPStr] the title
**
** @return [AjBool] ajTrue if the current Graphics object is valid to 
** add XTitle, eg. a graph
** @@
*********************************************************************/

AjBool ajXmlAddXTitle (AjPGraphXml file, const AjPStr title)
{
    return ajXmlAddXTitleC(file, ajStrGetPtr(title));
}




/* @func ajXmlAddYTitle ********************************************
**
** Adds a title to a graph object
**
** @param [w] file [AjPGraphXml] the file to add the title to
** @param [r] title [const AjPStr] the title
**
** @return [AjBool] ajTrue if the current Graphics object is valid to
** add YTitle, eg. a graph
** @@
*********************************************************************/

AjBool ajXmlAddYTitle(AjPGraphXml file, const AjPStr title)
{
    return ajXmlAddYTitleC(file, ajStrGetPtr(title));
}




/* @func ajXmlAddTextCentred *****************************************
**
** Adds some text, centered at point given
**
** @param [w] file [AjPGraphXml] the file to add the line to
** @param [r] x [double] the x coordinate
** @param [r] y [double] the y coordinate
** @param [r] size [double] font size
** @param [r] angle [double] font angle
** @param [r] fontFamily [const AjPStr] the name of the font family
** @param [r] fontStyle [const AjPStr] the style or the font
** @param [r] text [const AjPStr] the text to be written
**
** @return [void]
** @@
*********************************************************************/

void ajXmlAddTextCentred(AjPGraphXml file, double x, double y, 
			 double size, double angle, const AjPStr fontFamily,
			 const AjPStr fontStyle, const AjPStr text)
{
    float stringLength;

    stringLength = ajStrGetLen(text) * size / ONE_METER_FONT;
    ajXmlAddText(file, (x - ((stringLength / 2) * cos(angle))), y, 
		 size, angle, fontFamily, fontStyle, text);
    
    return;
}




/* @func ajXmlAddTextC ***********************************************
**
** Adds some text
**
** @param [w] file [AjPGraphXml] the file to add the line to
** @param [r] x [double] the x coordinate
** @param [r] y [double] the y coordinate
** @param [r] size [double] font size
** @param [r] angle [double] font angle
** @param [r] fontFamily [const char *] the name of the font family
** @param [r] fontStyle [const char *] the style or the font
** @param [r] text [const char *] the text to be written
**
** @return [void]
** @@
*********************************************************************/

void ajXmlAddTextC(AjPGraphXml file, double x, double y, double size, 
		  double angle, const char *fontFamily, const char *fontStyle, 
		  const char *text)
{
    AjPXmlNode transformNode  = NULL;
    AjPXmlNode elText         = NULL;
    AjPXmlNode elFont         = NULL;
    AjPXmlNode tranformParent = NULL;
    AjPXmlNode tempNode       = NULL;
    AjPStr attributeValue     =NULL;
    AjPStr temp = NULL;
    AjPStr name = NULL;
    AjPStr attributeVal = NULL;
    ajint i;
    ajint limit = 0;
    
    GdomeNodeList *listAppearance = NULL;
    GdomeException exc;
    GdomeDOMString *nodeName = NULL;

    temp = ajStrNew();
    attributeVal = ajStrNew();

    if(xml_PresentGraphicTypeIsC(file,"Graph"))
    {

	nodeName = gdome_str_mkref("fieldValue");
	listAppearance = gdome_el_getElementsByTagName
	    (xml_GetNodeElement(xml_GetCurrentGraphic(file)), nodeName, 
	     &exc);

	limit = gdome_nl_length(listAppearance, &exc);

	for(i=0; (i<limit && tranformParent == NULL) ; ++i)
	{
	    tempNode = xml_SetNode(gdome_nl_item(listAppearance, i, 
						 &exc));
	    attributeValue = xml_GetAttributeC(tempNode, "name");

	    if(ajStrMatchC(attributeValue, "children"))
		tranformParent = xml_SetNode(gdome_nl_item(listAppearance, 
							   i, &exc));

	    ajXmlNodeDel(&tempNode);
	    ajStrDel(&attributeValue);
	}	    

	if(!tranformParent)
	{
	    tranformParent = xml_MakeNewNodeC(file, "fieldValue", 
					     xml_GetCurrentGraphic(file));
		
	    xml_SetAttributeC(tranformParent, "name", "children");
	}
    }
    else
	tranformParent = xml_GetCurrentGraphic(file);

    transformNode = xml_MakeNewNodeC(file, "Transform", tranformParent);

    xml_AddACoord(x, y, ajFalse, &attributeVal, &temp);
    xml_SetAttributeC(transformNode, "translation", ajStrGetPtr(attributeVal));
    ajStrAssignC(&attributeVal,"0 0 1 ");
    ajStrAssignClear(&temp);

    xml_StrFromDouble(&temp, angle);
    ajStrAppendS(&attributeVal, temp);
  
    xml_SetAttributeC(transformNode, "rotation", ajStrGetPtr(attributeVal));

    elText = xml_MakeNewShapeNodeC(file, transformNode, "Text");

    xml_SetAttributeC(elText, "string", text);

    /* I have had to remove this for now, as the parser cannot handle 
       this type of node */
    /*
       if(fontFamily != NULL || fontStyle != NULL || size >= 0)
       {
       elFont = xml_MakeNewNodeC(file, "FontStyle", elText);
       if(fontFamily != NULL)
       {
       xml_SetAttributeC(elFont, "family", fontFamily);
       }
       if(fontStyle != NULL)
       {
       xml_SetAttributeC(elFont, "style", fontStyle);
       }
       if(size >= 0)
       {
       ajStrFromDouble(&temp, size, 2);
       xml_SetAttributeC(elFont, "size", ajStrGetPtr(temp));
       }
       }
       */

    ajStrDel(&attributeVal);
    ajStrDel(&temp);
    ajStrDel(&name);

    xml_UnrefNode(&transformNode);
    xml_UnrefNode(&elText);

    if(elFont != NULL)
	xml_UnrefNode(&elFont);

    return;
}




/* @func ajXmlAddText ***********************************************
**
** Adds some text
**
** @param [w] file [AjPGraphXml] the file to add the line to
** @param [r] x [double] the x coordinate
** @param [r] y [double] the y coordinate
** @param [r] size [double] font size
** @param [r] angle [double] font angle
** @param [r] fontFamily [const AjPStr] the name of the font family
** @param [r] fontStyle [const AjPStr] the style or the font
** @param [r] text [const AjPStr] the text to be written
**
** @return [void]
** @@
*********************************************************************/

void ajXmlAddText(AjPGraphXml file, double x, double y, double size, 
		  double angle, const AjPStr fontFamily, 
		  const AjPStr fontStyle, 
		  const AjPStr text)
{
    ajXmlAddTextC(file,x,y,size,angle,
                  (fontFamily) ? ajStrGetPtr(fontFamily) : NULL,
		  (fontStyle)  ? ajStrGetPtr(fontStyle)  : NULL,
		  (text) ? ajStrGetPtr(text) : NULL);
    return;
}




/* @func ajXmlAddTextWithCJustify *************************************
**
** Adds some text with the justification set
**
** @param [w] file [AjPGraphXml] the file to add the line to
** @param [r] x [double] the x coordinate
** @param [r] y [double] the y coordinate
** @param [r] size [double] font size
** @param [r] angle [double] font angle
** @param [r] fontFamily [const AjPStr] the name of the font family
** @param [r] fontStyle [const AjPStr] the style or the font
** @param [r] text [const AjPStr] the text to be written
** @param [r] horizontal [AjBool] whether the text advances 
** horizontally in its major direction (horizontal = ajTrue) or 
** vertically in its major direction (horizontal = ajFalse).
** @param [r] leftToRight [AjBool] direction of text advance in the 
** major or minor direction
** @param [r] topToBottom [AjBool] direction of text advance in the 
** major or minor direction
** @param [r] justifyMajor [const char *] alignment of the text in the 
** Major direction
** @param [r] justifyMinor [const char *] alignment of the text in the 
** Minor direction
**
** @return [void]
** @@
*********************************************************************/

void ajXmlAddTextWithCJustify(AjPGraphXml file, double x, double y, 
			      double size, double angle, 
			      const AjPStr fontFamily, const AjPStr fontStyle, 
			      const AjPStr text, AjBool horizontal, 
			      AjBool leftToRight, AjBool topToBottom, 
			      const char *justifyMajor,
			      const char *justifyMinor)
{
    AjPXmlNode transformNode;
    AjPXmlNode elText;
    AjPXmlNode elFont;
    AjPStr attributeVal = NULL;
    AjPStr temp = NULL;

    temp         = ajStrNew();
    attributeVal = ajStrNew();

    transformNode = xml_MakeNewNodeC(file, "Transform", 
				    xml_GetCurrentGraphic(file));

    xml_AddACoord(x, y, ajFalse, &attributeVal, &temp);
    xml_SetAttributeC(transformNode, "translation", ajStrGetPtr(attributeVal));
  
    ajStrSetClear(&attributeVal);
    xml_AddACoord(angle, 0, ajFalse, &attributeVal, &temp);
    xml_SetAttributeC(transformNode, "rotation", ajStrGetPtr(attributeVal));

    elFont = elText = xml_MakeNewShapeNodeC(file, transformNode, 
					    "Text");

    xml_SetAttributeC(elText, "string", ajStrGetPtr(text));

    elFont = xml_MakeNewNodeC(file, "Font", elText);
    if(fontFamily != NULL)
	xml_SetAttributeC(elFont, "family", ajStrGetPtr(fontFamily));

    if(fontStyle != NULL)
	xml_SetAttributeC(elFont, "style", ajStrGetPtr(fontStyle));

    if(size >= 0)
    {
	ajStrFromInt(&temp, size);
	xml_SetAttributeC(elFont, "size", ajStrGetPtr(temp));
    }

    xml_SetAttributeC(elFont, "horizontal", 
		      ajStrGetPtr(xml_StrFromBool(horizontal)));
    xml_SetAttributeC(elFont, "leftToRight", 
		      ajStrGetPtr(xml_StrFromBool(leftToRight)));
    xml_SetAttributeC(elFont, "topToBottom", 
		      ajStrGetPtr(xml_StrFromBool(topToBottom)));

    ajStrAssignC(&attributeVal,"\"");

    ajStrAppendC(&attributeVal, justifyMajor);
    ajStrAppendC(&attributeVal, "\",\"");
    ajStrAppendC(&attributeVal, justifyMinor);
    ajStrAppendC(&attributeVal, "\"");

    xml_SetAttributeC(elFont, "justify", ajStrGetPtr(attributeVal));

    ajStrDel(&attributeVal);
    ajStrDel(&temp);

    xml_UnrefNode(&transformNode);
    xml_UnrefNode(&elText);
    xml_UnrefNode(&elFont);

    return;
}




/* @func ajXmlAddTextWithJustify *************************************
**
** Adds some text with the justification set
**
** @param [w] file [AjPGraphXml] the file to add the line to
** @param [r] x [double] the x coordinate
** @param [r] y [double] the y coordinate
** @param [r] size [double] font size
** @param [r] angle [double] font angle
** @param [r] fontFamily [const AjPStr] the name of the font family
** @param [r] fontStyle [const AjPStr] the style or the font
** @param [r] text [const AjPStr] the text to be written
** @param [r] horizontal [AjBool] whether the text advances 
** horizontally in its major direction (horizontal = ajTrue) or 
** vertically in its major direction (horizontal = ajFalse).
** @param [r] leftToRight [AjBool] direction of text advance in the 
** major or minor direction
** @param [r] topToBottom [AjBool] direction of text advance in the 
** major or minor direction
** @param [r] justifyMajor [const AjPStr] alignment of the text in the 
** Major direction
** @param [r] justifyMinor [const AjPStr] alignment of the text in the 
** Minor direction
**
** @return [void]
** @@
*********************************************************************/

void ajXmlAddTextWithJustify(AjPGraphXml file, double x, double y, 
			     double size, double angle, 
			     const AjPStr fontFamily, const AjPStr fontStyle, 
			     const AjPStr text, AjBool horizontal, 
			     AjBool leftToRight, AjBool topToBottom, 
			     const AjPStr justifyMajor,
			     const AjPStr justifyMinor)
{
    ajXmlAddTextWithCJustify(file,x,y,size,angle,fontFamily,fontStyle, 
			     text,horizontal,leftToRight,topToBottom, 
			     ajStrGetPtr(justifyMajor),
                             ajStrGetPtr(justifyMinor));
    return;
}




/* @func ajXmlAddTextOnArc *****************************************
**
** adds text on an arc.  If the text is too long it is shrunk, if it 
** does 
** not fill the arc it is centred. A character of length 1m is size 
** 1.784
**
** @param [w] file [AjPGraphXml] the file to add the text to
** @param [r] xCentre [double] the x value of the centre of the 
** circle
** @param [r] yCentre [double] the y value of the centre of the 
** circle
** @param [r] startAngle [double] the angle of the start of the arc
** @param [r] endAngle [double] the angle of the end of the arc
** @param [r] radius [double] the radius of the circle
** @param [r] size [double] the font size
** @param [r] fontFamily [const AjPStr] the font Family 
** @param [r] fontStyle [const AjPStr] the  font Style
** @param [r] text [const AjPStr] the text
**
** @return [void]
** @@
*********************************************************************/

void ajXmlAddTextOnArc(AjPGraphXml file, double xCentre, double yCentre,
		       double startAngle, double endAngle, 
		       double radius, double size, const AjPStr fontFamily, 
		       const AjPStr fontStyle, const AjPStr text)
{
    double anglularExtentPerLetter;
    double letterAngle;
    double letterXPos;
    double letterYPos;
    double textLength;
    double arcLength;
    double lettersInFront;
    ajint  limit;
    
    int i;

    AjPStr letter = NULL;

    letter = ajStrNew();

    textLength = ajStrGetLen(text) / ONE_METER_FONT;
    arcLength = (endAngle - startAngle) * radius;
    
    if(textLength > arcLength)
    {
	size = (arcLength * ONE_METER_FONT) / ajStrGetLen(text);
	lettersInFront = 0;
    }
    else if(textLength < arcLength)
	lettersInFront = ((arcLength - textLength) / 2) / 
	    (size / ONE_METER_FONT);
    else
	lettersInFront = 0;
    
    anglularExtentPerLetter = size / (radius * ONE_METER_FONT);
    
    /* why does this run one more time than it should? hugh */
    /* 
       for(i=lettersInFront; i<(((double) ajStrGetLen(text)) + 
       lettersInFront); ++i) 
       */

    limit = ajStrGetLen(text);

    for(i=0; i<limit; ++i)
    {
	ajStrAssignSubC(&letter, ajStrGetPtr(text), i, i);

	if(!ajStrMatchC(letter, " "))
	{
	    letterAngle = ((((double)i) + lettersInFront) * 
			   anglularExtentPerLetter) + startAngle;
	    letterXPos = (cos(letterAngle + startAngle) * radius) +
		xCentre;
	    letterYPos = (sin(letterAngle + startAngle) * radius) +
		yCentre;
	
	    ajXmlAddText(file, letterXPos, letterYPos, size, 
			 letterAngle, fontFamily, fontStyle, 
			 letter);
	}
    }

    ajStrDel(&letter);

    return;
}




/* @func ajXmlAddJoinedLineSetEqualGapsF ****************************
**
** Adds a set of joined lines with the x values equaly spaced
**
** @param [w] file [AjPGraphXml] the file to add the line to
** @param [r] y [const float*] pointer to 1st y coordinate
** @param [r] numberOfPoints [int] number Of Points
** @param [r] startX [float] x start value
** @param [r] increment [float] increment
**
** @return [void]
** @@
*********************************************************************/

void ajXmlAddJoinedLineSetEqualGapsF(AjPGraphXml file, const float *y, 
				     int numberOfPoints, float startX,
				     float increment)
{
    double *xn;
    double *yn;
    int i;

    AJCNEW0(xn,numberOfPoints);
    AJCNEW0(yn,numberOfPoints);
    
    for(i=0.0; i<numberOfPoints; ++i)
    {
	xn[i] = (double) startX + (((double) i) * increment);
	yn[i] = (double) y[i];
    }

    ajXmlAddJoinedLineSet(file, xn, yn, numberOfPoints);
    
    AJFREE(xn);
    AJFREE(yn);

    return;
}




/* @func ajXmlAddJoinedLineSetF *************************************
**
** Adds a set of joined lines
**
** @param [w] file [AjPGraphXml] the file to add the line to
** @param [r] x [const float*] pointer to 1st x coordinate
** @param [r] y [const float*] pointer to 1st y coordinate
** @param [r] numberOfPoints [int] number Of Points
**
** @return [void]
** @@
*********************************************************************/

void ajXmlAddJoinedLineSetF(AjPGraphXml file, const float *x, const float *y, 
			    int numberOfPoints)
{
    double *xn;
    double *yn;
    int i;
    
    AJCNEW(xn, numberOfPoints);
    AJCNEW(yn, numberOfPoints);

    for(i=0; i<numberOfPoints; ++i)
    {
	xn[i] = (double) x[i];
	yn[i] = (double) y[i];
    }

    ajXmlAddJoinedLineSet(file, xn, yn, numberOfPoints);

    AJFREE(xn);
    AJFREE(yn);
    
    return;
}




/* @func ajXmlAddJoinedLineSet ***************************************
**
** Adds a set of joined lines
**
** @param [w] file [AjPGraphXml] the file to add the line to
** @param [r] x [const double*] pointer to 1st x coordinate
** @param [r] y [const double*] pointer to 1st y coordinate
** @param [r] numberOfPoints [int] number Of Points
**
** @return [void]
** @@
*********************************************************************/

void ajXmlAddJoinedLineSet(AjPGraphXml file, const double *x, const double *y, 
			   int numberOfPoints)
{
    AjPXmlNode el;
    int count;
    AjPStr index = NULL;
    AjPStr coord = NULL;

    /* I try and find preexisting IndexedLineSet's  */
    el = xml_GetNodeTypeMakeIfNotC(file, "IndexedLineSet"); 

    coord = xml_GetPoints(el);
    index = xml_GetIndex(el);

    for(count = 0; count < numberOfPoints; ++count)
	xml_AddACoord(x[count], y[count], (count!=0), &coord, &index);

    xml_SetPoints(el, coord);
    xml_SetIndex(el, index);

    /* tidy up */
    ajStrDel(&coord);
    ajStrDel(&index);

    if(el != xml_GetCurrentGraphic(file))
	xml_UnrefNode(&el);

    return;
}




/* @func ajXmlAddLine ***********************************************
**
** Adds a line
**
** @param [w] file [AjPGraphXml] the file to add the line to
** @param [r] x1 [double] 1st x coordinate
** @param [r] y1 [double] 1st y coordinate
** @param [r] x2 [double] 2nd x coordinate
** @param [r] y2 [double] 2nd y coordinate
**
** @return [void]
** @@
*********************************************************************/

void ajXmlAddLine(AjPGraphXml file, double x1, double y1, double x2, 
		  double y2)
{
    AjPXmlNode el;
    AjPStr coord = NULL;
    AjPStr index = NULL;

    el = xml_GetNodeTypeMakeIfNotC(file, "IndexedLineSet");

    /* setting up the coords to add */
    coord = xml_GetPoints(el);
    index = xml_GetIndex(el);

    xml_AddACoord(x1, y1, ajFalse, &coord, &index);
    xml_AddACoord(x2, y2, ajTrue, &coord, &index);

    xml_SetPoints(el, coord);
    xml_SetIndex(el, index);

    /* tidy up */
    ajStrDel(&coord);
    ajStrDel(&index);

    if(el != xml_GetCurrentGraphic(file))
	xml_UnrefNode(&el);

    return;
}




/* @func ajXmlAddLineF **********************************************
**
** Adds a line with floats
**
** @param [w] file [AjPGraphXml] the file to add the line to
** @param [r] x1 [float] 1st x coordinate
** @param [r] y1 [float] 1st y coordinate
** @param [r] x2 [float] 2nd x coordinate
** @param [r] y2 [float] 2nd y coordinate
**
** @return [void]
** @@
*********************************************************************/

void ajXmlAddLineF(AjPGraphXml file, float x1, float y1, float x2, 
		   float y2)
{
    ajXmlAddLine(file, (double) x1, (double) y1, (double) x2, 
		 (double) y2);

    return;
}




/* @func ajXmlAddPoint **********************************************
**
** Adds a Point
**
** @param [w] file [AjPGraphXml] the file to add the point to
** @param [r] x1 [double] x coordinate
** @param [r] y1 [double] y coordinate
**
** @return [void]
** @@
*********************************************************************/

void ajXmlAddPoint(AjPGraphXml file, double x1, double y1)
{
    AjPXmlNode el;
    AjPStr coord = NULL;
    AjPStr index = NULL;

    el = xml_GetNodeTypeMakeIfNotC(file, "IndexedLineSet");
  
    /* setting up the coords to add */
    coord = xml_GetPoints(el);
    index = xml_GetIndex(el);

    xml_AddACoord(x1, y1, ajFalse, &coord, &index);
    xml_AddACoord(x1, y1, ajTrue, &coord, &index);

    xml_SetPoints(el, coord);
    xml_SetIndex(el, index);

    /* tidy up */
    ajStrDel(&coord);
    ajStrDel(&index);

    if(el != xml_GetCurrentGraphic(file))
	xml_UnrefNode(&el);

    return;
}




/* @func ajXmlAddHistogramEqualGapsF ********************************
**
** Adds a Histogram with each bar the same width, with y axis at 0.
**
** @param [w] file [AjPGraphXml] the file to add the line to
** @param [r] y [const float*] the y coordinates, heights of bars
** @param [r] numPoints [int] number of bars
** @param [r] startX [float] start position of first bar
** @param [r] xGap [float] x size of each bin / bar
**
** @return [void]
** @@
*********************************************************************/

void ajXmlAddHistogramEqualGapsF(AjPGraphXml file, const float *y, 
				 int numPoints, float startX, 
				 float xGap)
{
    int point;

    AjPXmlNode el;
    AjPStr coord = NULL;
    AjPStr index = NULL;

		
    el = xml_GetNodeTypeMakeIfNotC(file, "IndexedFaceSet");

    /* setting up the coords to add */
    coord = xml_GetPoints(el);
    index = xml_GetIndex(el);

    for(point = 0; point < numPoints; ++point)
	if(y[point] != 0.0)
	{
	    if(!y[point] > 0)
	    {
		xml_AddACoord(((double) (startX + (point * xGap))),
			      ((double) 0), ajFalse, &coord, &index);
		xml_AddACoord(((double) (startX + (point * xGap))),
			      ((double) y[point]), ajTrue, &coord, &index);
		xml_AddACoord(((double) (startX + ((point+ 1) * xGap))),
			      ((double) y[point]), ajTrue, &coord, &index);
		xml_AddACoord(((double) (startX + ((point+ 1) * xGap))),
			      ((double)  0), ajTrue, &coord, &index);
	    }
	    else
	    {
		xml_AddACoord(((double) (startX + ((point+ 1) * xGap))),
			      ((double)  0), ajFalse, &coord, &index);
		xml_AddACoord(((double) (startX + ((point+ 1) * xGap))),
			      ((double) y[point]), ajTrue, &coord, &index);
		xml_AddACoord(((double) (startX + (point * xGap))),
			      ((double) y[point]), ajTrue, &coord, &index);
		xml_AddACoord(((double) (startX + (point * xGap))),
			      ((double)  0), ajTrue, &coord, &index);
	    }
	}	    

    xml_SetPoints(el, coord);
    xml_SetIndex(el, index);

    /* tidy up */
    xml_UnrefNode(&el);
    ajStrDel(&coord);
    ajStrDel(&index);

    return;
}




/* @func ajXmlAddRectangleSet ***************************************
**
** Adds a rectangle
**
** @param [w] file [AjPGraphXml] the file to add the line to
** @param [r] x1 [const double*] 1st x coordinate
** @param [r] y1 [const double*] 1st y coordinate
** @param [r] x2 [const double*] 2nd x coordinate
** @param [r] y2 [const double*] 2nd y coordinate
** @param [r] numPoints [int] number of points
** @param [r] fill [AjBool] Filled rectangle?
**
** @return [void]
** @@
*********************************************************************/

void ajXmlAddRectangleSet(AjPGraphXml file,
			  const double *x1, const double *y1, 
			  const double *x2, const double *y2, int numPoints, 
			  AjBool fill)
{
    int point;
    AjPXmlNode el;
    AjPStr coord = NULL;
    AjPStr index = NULL;
    
    if(fill)
    {
	el = xml_GetNodeTypeMakeIfNotC(file, "IndexedFaceSet");

	/* setting up the coords to add */
	coord = xml_GetPoints(el);
	index = xml_GetIndex(el);

	for(point = 0; point < numPoints; ++point)
	{
	    if(!(((x1[point] > x2[point]) && (y1[point] > y2[point]))
		 || ((x1[point] < x2[point]) && (y1[point] < y2[point]))) )
	    {
		xml_AddACoord(x1[point], y1[point], ajFalse, &coord, &index);
		xml_AddACoord(x1[point], y2[point], ajTrue, &coord, &index);
		xml_AddACoord(x2[point], y2[point], ajTrue, &coord, &index);
		xml_AddACoord(x2[point], y1[point], ajTrue, &coord, &index);
	    }
	    else
	    {
		xml_AddACoord(x2[point], y1[point], ajFalse, &coord, &index);
		xml_AddACoord(x2[point], y2[point], ajTrue, &coord, &index);
		xml_AddACoord(x1[point], y2[point], ajTrue, &coord, &index);
		xml_AddACoord(x1[point], y1[point], ajTrue, &coord, &index);
	    }
	}	    

	xml_SetPoints(el, coord);
	xml_SetIndex(el, index);

	/* tidy up */
	xml_UnrefNode(&el);
	ajStrDel(&coord);
	ajStrDel(&index);
    }
    else
	for(point = 0; point < numPoints; ++point)
	    ajXmlAddRectangle(file, x1[point], y1[point], x2[point], 
			      y2[point], fill);

    return;
}




/* @func ajXmlAddRectangle ******************************************
**
** Adds a rectangle
**
** @param [w] file [AjPGraphXml] the file to add the line to
** @param [r] x1 [double] 1st x coordinate
** @param [r] y1 [double] 1st y coordinate
** @param [r] x2 [double] 2nd x coordinate
** @param [r] y2 [double] 2nd y coordinate
** @param [r] fill [AjBool] Filled rectangle?
**
** @return [void]
** @@
*********************************************************************/

void ajXmlAddRectangle(AjPGraphXml file, double x1, double y1, 
		       double x2, double y2, AjBool fill)
{
    AjPXmlNode el;
    AjPStr coord = NULL;
    AjPStr index = NULL;

    if(fill)
    {  
	el = xml_GetNodeTypeMakeIfNotC(file, "IndexedFaceSet");

	/* setting up the coords to add */
	coord = xml_GetPoints(el);
	index = xml_GetIndex(el);

	if(!(((x1 > x2) && (y1 > y2)) || ((x1 < x2) && (y1 < y2))) )
	{
	    xml_AddACoord(x1, y1, ajFalse, &coord, &index);
	    xml_AddACoord(x1, y2, ajTrue, &coord, &index);
	    xml_AddACoord(x2, y2, ajTrue, &coord, &index);
	    xml_AddACoord(x2, y1, ajTrue, &coord, &index);
	}
	else
	{
	    xml_AddACoord(x2, y1, ajFalse, &coord, &index);
	    xml_AddACoord(x2, y2, ajTrue, &coord, &index);
	    xml_AddACoord(x1, y2, ajTrue, &coord, &index);
	    xml_AddACoord(x1, y1, ajTrue, &coord, &index);
	}
	    
	xml_SetPoints(el, coord);
	xml_SetIndex(el, index);

	/* tidy up */
	xml_UnrefNode(&el);
	ajStrDel(&coord);
	ajStrDel(&index);
    }
    else
    {
	double x[5];
	double y[5];
      
	if((((x1 > x2) && (y1 > y2)) || ((x1 < x2) && (y1 < y2))) )
	{
	    x[0] = x1;
	    x[1] = x2;
	    x[2] = x2;
	    x[3] = x1;
	    x[4] = x1;
      
	    y[0] = y1;
	    y[1] = y1;
	    y[2] = y2;
	    y[3] = y2;
	    y[4] = y1;
	}
	else
	{
	    x[0] = x2;
	    x[1] = x1;
	    x[2] = x1;
	    x[3] = x2;
	    x[4] = x2;
      
	    y[0] = y1;
	    y[1] = y1;
	    y[2] = y2;
	    y[3] = y2;
	    y[4] = y1;
	}	  
      
	ajXmlAddJoinedLineSet(file, x, y, 5);      
    }
      
    return;
}




/* @func ajXmlAddCylinder *******************************************
**
** Adds a Cylinder
**
** @param [w] file [AjPGraphXml] the file to add the Cylinder to
** @param [r] x1 [double] 1st x coordinate
** @param [r] y1 [double] 1st y coordinate
** @param [r] x2 [double] 2nd x coordinate
** @param [r] y2 [double] 2nd y coordinate
** @param [r] width [double] width 
**
** @return [void]
** @@
*********************************************************************/

void ajXmlAddCylinder(AjPGraphXml file, double x1, double y1, 
		      double x2, double y2, double width)
{
    double height;

    double xCentre;
    double yCentre;

    double angle;

    height  = pow((pow((x2-x1), 2) + (pow((y2-y1), 2))), 0.5);
    xCentre =  x1 + ((x2 - x1) / 2);
    yCentre = y1 + ((y2 - y1) / 2);
    angle   = atan2((y2 - y1), (x2 - x1));

    xml_AddCylinder(file, xCentre, yCentre, angle, height, width);

    return;
}




/* @func ajXmlAddPointLabelCircle ************************************
**
** adds a Point Label
**
** @param [w] file [AjPGraphXml] the file to add the object to
** @param [r] angle [double] the angle of label to add
** @param [r] xCentre [double] the x value of the centre of the 
** circle
** @param [r] yCentre [double] the y value of the centre of the 
** circle
** @param [r] radius [double] the radius of the circle
** @param [r] length [double] the length of the line of the label
** @param [r] size [double] the font size
** @param [r] fontFamily [const AjPStr] the font Family 
** @param [r] fontStyle [const AjPStr] the  font Style
** @param [r] text [const AjPStr] the text of label to add
**
** @return [AjBool] ajTrue if correctly added
** @@
*********************************************************************/

AjBool ajXmlAddPointLabelCircle(AjPGraphXml file, double angle, 
				double xCentre, double yCentre, 
				double radius, double length, 
				double size, const AjPStr fontFamily, 
				const AjPStr fontStyle,const  AjPStr text)
{
    double xStart;
    double yStart;
    double xEnd;
    double yEnd;
    double textXStart;
    double textYStart;

    AjPStr textPrinted = NULL;
    
    if(!xml_PresentGraphicTypeIsC(file, "DNAPlot"))
	return ajFalse;

    textPrinted = ajStrNewS(text);

    xStart = (sin(angle) * radius) + xCentre;
    yStart = (cos(angle) * radius) + yCentre;
    xEnd = (sin(angle) * (radius + length)) + xCentre;
    yEnd = (cos(angle) * (radius + length)) + yCentre;
    
    if(xml_AngleIsInSecondHalfOfCircle(angle))
	ajStrReverse(&textPrinted);

    textXStart = (cos(angle) * (radius + (length * 2))) + xCentre;
    textYStart = (sin(angle) * (radius + (length * 2))) + yCentre;

    ajXmlAddLine(file, xStart, yStart, xEnd, yEnd);

    ajXmlAddTextWithCJustify(file, textXStart, textYStart, size, 
			     (angle - acos(0)), fontFamily, fontStyle, 
			     textPrinted, ajTrue, ajTrue, ajTrue, "BEGIN",
			     "MIDDLE");

    ajStrDel(&textPrinted);

    return ajTrue;
}




/* @func ajXmlAddSectionLabelCircle **********************************
**
** adds a Section Label
**
** @param [w] file [AjPGraphXml] the file to add the object to
** @param [r] startAngle [double] the start angle of label to add
** @param [r] endAngle [double] the end angle of label to add
** @param [r] xCentre [double] the x value of the centre of the 
** circle
** @param [r] yCentre [double] the y value of the centre of the 
** circle
** @param [r] radius [double] the radius of the circle
** @param [r] width [double] the width of the label
** @param [r] labelArmAngle [double] the angle subtended by the label
** arms (ignored if labelStyle = Block
** @param [r] labelStyle [const AjPStr] the label style
** @param [r] textPosition [double] the text Position relative to the
** circle
** @param [r] size [double] the font size
** @param [r] fontFamily [const AjPStr] the font Family 
** @param [r] fontStyle [const AjPStr] the  font Style
** @param [r] text [const AjPStr] the text of label to add
**
** @return [AjBool] ajTrue if correctly added
** @@
*********************************************************************/

AjBool ajXmlAddSectionLabelCircle(AjPGraphXml file, double startAngle, 
				  double endAngle, double xCentre, 
				  double yCentre, double radius, 
				  double width, double labelArmAngle, 
				  const AjPStr labelStyle, 
				  double textPosition, double size, 
				  const AjPStr fontFamily,
				  const AjPStr fontStyle,
				  const AjPStr text)
{
    double xStartInner;
    double yStartInner;
    double xEndInner;
    double yEndInner;
    double textXStart;
    double textYStart;

    double xStartOuter;
    double yStartOuter;
    double xEndOuter;
    double yEndOuter;

    double xStartInnerLabel;
    double yStartInnerLabel;
    double xEndInnerLabel;
    double yEndInnerLabel;

    double xStartOuterLabel;
    double yStartOuterLabel;
    double xEndOuterLabel;
    double yEndOuterLabel;

    if(!xml_PresentGraphicTypeIsC(file, "DNAPlot"))
	return ajFalse;


    if(ajStrMatchC(labelStyle, "Block"))
    {
	xStartInner = (sin(startAngle) * (radius - (width / 2)) + 
		       xCentre);
	yStartInner = (cos(startAngle) * (radius - (width / 2)) + 
		       yCentre);
	xEndInner = (sin(endAngle) * (radius - (width / 2)) + 
		     xCentre);
	yEndInner = (cos(endAngle) * (radius - (width / 2)) + 
		     yCentre);
	xStartOuter = (sin(startAngle) * (radius + (width / 2)) + 
		       xCentre);
	yStartOuter = (cos(startAngle) * (radius + (width / 2)) + 
		       yCentre);
	xEndOuter = (sin(endAngle) * (radius + (width / 2)) + 
		     xCentre);
	yEndOuter = (cos(endAngle) * (radius + (width / 2)) + 
		     yCentre);

	textXStart = (sin(startAngle) * (radius + textPosition) + 
		      xCentre);
	textYStart = (cos(startAngle) * (radius + textPosition) + 
		      yCentre);

	/* should realy make this a long nurbs, perhaps later hugh */

	ajXmlAddArc(file, xCentre, yCentre, startAngle, endAngle, 
		    (radius - (width / 2)));
	ajXmlAddArc(file, xCentre, yCentre, startAngle, endAngle, 
		    (radius + (width / 2)));
	ajXmlAddLine(file, xStartInner, yStartInner, xStartOuter, 
		     yStartOuter);
	ajXmlAddLine(file, xEndInner, yEndInner, xEndOuter, 
		     yEndOuter);
        ajXmlAddTextOnArc(file, xCentre, yCentre, startAngle, 
			  endAngle, (radius + textPosition), size, 
			  fontFamily, fontStyle, text);
    }
    else if(ajStrMatchC(labelStyle, "SquareBracketed") || 
	    ajStrMatchC(labelStyle, "Arrowed") )
    {
	ajXmlAddArc(file, xCentre, yCentre, startAngle, endAngle, 
		    radius);
        ajXmlAddTextOnArc(file, xCentre, yCentre, startAngle, 
			  endAngle, (radius + textPosition), size, 
			  fontFamily, fontStyle, text);

	if(ajStrMatchC(labelStyle, "SquareBracketed"))
	{
	    xStartInner = (sin(startAngle) * (radius - (width / 2)) + 
			   xCentre);
	    yStartInner = (cos(startAngle) * (radius - (width / 2)) + 
			   yCentre);
	    xEndInner = (sin(endAngle) * (radius - (width / 2)) + 
			 xCentre);
	    yEndInner = (cos(endAngle) * (radius - (width / 2)) + 
			 yCentre);
	    xStartOuter = (sin(startAngle) * (radius + (width / 2)) + 
			   xCentre);
	    yStartOuter = (cos(startAngle) * (radius + (width / 2)) + 
			   yCentre);
	    xEndOuter = (sin(endAngle) * (radius + (width / 2)) + 
			 xCentre);
	    yEndOuter = (cos(endAngle) * (radius + (width / 2)) + 
			 yCentre);
	    
	    xStartInnerLabel = (sin(startAngle - labelArmAngle) * 
				(radius - (width / 2)) + xCentre);
	    yStartInnerLabel = (cos(startAngle - labelArmAngle) * 
				(radius - (width / 2)) + yCentre);
	    xEndInnerLabel = (sin(endAngle + labelArmAngle) * 
			      (radius - (width / 2)) + xCentre);
	    yEndInnerLabel = (cos(endAngle + labelArmAngle) * 
			      (radius - (width / 2)) + yCentre);
	    xStartOuterLabel = (sin(startAngle - labelArmAngle) * 
				(radius + (width / 2)) + xCentre);
	    yStartOuterLabel = (cos(startAngle - labelArmAngle) * 
				(radius + (width / 2)) + yCentre);
	    xEndOuterLabel = (sin(endAngle + labelArmAngle) * 
			      (radius + (width / 2)) + xCentre);
	    yEndOuterLabel = (cos(endAngle + labelArmAngle) * 
			      (radius + (width / 2)) + yCentre);
	    
	    ajXmlAddLine(file, xStartInner, yStartInner, xStartOuter, 
			 yStartOuter);
	    ajXmlAddLine(file, xStartInner, yStartInner, 
			 xStartInnerLabel, yStartInnerLabel);
	    ajXmlAddLine(file, xStartOuter, xStartOuter, 
			 xStartOuterLabel,yStartOuterLabel );
	    ajXmlAddLine(file, xEndInner, yEndInner, xEndOuter, 
			 yEndOuter);
	    ajXmlAddLine(file, xEndInner, yEndInner, xEndInnerLabel, 
			 yEndInnerLabel);
	    ajXmlAddLine(file, xEndOuter, xEndOuter, xEndOuterLabel,
			 yEndOuterLabel );
	}

	if(ajStrMatchC(labelStyle, "Arrowed"))
	{
	    xStartInner = (sin(startAngle) * radius + xCentre);
	    yStartInner = (cos(startAngle) * radius + yCentre);
	    xEndInner = (sin(endAngle) * radius + xCentre);
	    yEndInner = (cos(endAngle) * radius + yCentre);
	    
	    xStartInnerLabel = (sin(startAngle + labelArmAngle) * 
				(radius - (width / 2)) + xCentre);
	    yStartInnerLabel = (cos(startAngle + labelArmAngle) * 
				(radius - (width / 2)) + yCentre);
	    xEndInnerLabel = (sin(endAngle - labelArmAngle) * 
			      (radius - (width / 2)) + xCentre);
	    yEndInnerLabel = (cos(endAngle - labelArmAngle) * 
			      (radius - (width / 2)) + yCentre);
	    xStartOuterLabel = (sin(startAngle + labelArmAngle) * 
				(radius + (width / 2)) + xCentre);
	    yStartOuterLabel = (cos(startAngle + labelArmAngle) * 
				(radius + (width / 2)) + yCentre);
	    xEndOuterLabel = (sin(endAngle - labelArmAngle) * 
			      (radius + (width / 2)) + xCentre);
	    yEndOuterLabel = (cos(endAngle - labelArmAngle) * 
			      (radius + (width / 2)) + yCentre);
	
	    ajXmlAddLine(file, xStartInner, yStartInner, 
			 xStartOuterLabel, yStartOuterLabel);
	    ajXmlAddLine(file, xStartInner, yStartInner, 
			 xStartInnerLabel, yStartInnerLabel);
	    ajXmlAddLine(file, xEndInner, yEndInner, xEndOuterLabel, 
			 yEndOuterLabel);
	    ajXmlAddLine(file, xEndInner, yEndInner, xEndInnerLabel, 
			 yEndInnerLabel);
	}	
    }

    return ajTrue;
}




/* @func ajXmlAddPointLabelLinear ************************************
**
** adds a Point Label on a linear DNA plot
**
** @param [w] file [AjPGraphXml] the file to add the object to
** @param [r] angle [double] the angle of label to add
** @param [r] xPoint [double] the x value of the label
** @param [r] yPoint [double] the y value of the label
** @param [r] length [double] the length of the line of the label
** @param [r] textParallelToLine [AjBool] ajTrue if the text if 
** Parrallel to the line, ajFalse if the line is at right angles to the 
** line
** @param [r] size [double] the font size
** @param [r] fontFamily [const AjPStr] the font Family 
** @param [r] fontStyle [const AjPStr] the  font Style
** @param [r] text [const AjPStr] the text of label to add
**
** @return [AjBool] ajTrue if correctly added
** @@
*********************************************************************/

AjBool ajXmlAddPointLabelLinear(AjPGraphXml file, double angle, 
				double xPoint, double yPoint, 
				double length, 
				AjBool textParallelToLine, 
				double size, const AjPStr fontFamily, 
				const AjPStr fontStyle,const  AjPStr text)
{
    double xEnd;
    double yEnd;
    double textXStart;
    double textYStart;
    double textAngle;

    AjPStr textPrinted = NULL;
    
    if(!xml_PresentGraphicTypeIsC(file, "DNAPlot"))
	return ajFalse;

    textPrinted = ajStrNewS(text);

    xEnd = (sin(angle) * length) + xPoint;
    yEnd = (cos(angle) * length) + yPoint;
    textXStart = (sin(angle) * length * 2) + xPoint;
    textYStart = (cos(angle) * length * 2) + yPoint;
    
    if(textParallelToLine)
	textAngle = angle;
    else
	textAngle = angle + (acos(0) / 2);

    if(xml_AngleIsInSecondHalfOfCircle(textAngle))
	ajStrReverse(&textPrinted);

    ajXmlAddLine(file, xPoint, yPoint, xEnd, yEnd);

    if(textParallelToLine)
	ajXmlAddTextWithCJustify(file, textXStart, textYStart, size, 
				 (textAngle - acos(0)), fontFamily, 
				 fontStyle, textPrinted, ajTrue, ajTrue, 
				 ajTrue, "BEGIN", "MIDDLE");
    else
	ajXmlAddTextWithCJustify(file, textXStart, textYStart, size, 
				(textAngle - acos(0)), fontFamily, 
				fontStyle, textPrinted, ajTrue, ajTrue, 
				ajTrue, "MIDDLE", "MIDDLE");
    
    ajStrDel(&textPrinted);

    return ajTrue;
}




/* @func ajXmlAddSectionLabelLinear **********************************
**
** adds a Section Label to Linear DNA Plot
**
** @param [w] file [AjPGraphXml] the file to add the object to
** @param [r] xStart [double] the start x value of label to add
** @param [r] yStart [double] the start y value of label to add
** @param [r] xEnd [double] the end x value of label to add
** @param [r] yEnd [double] the end y value of label to add
** @param [r] width [double] the width of the label
** @param [r] labelArmLength [double] the distance the arms go back 
** (ignored if labelStyle = Block
** @param [r] labelStyle [const AjPStr] the label style
** @param [r] textPosition [double] the text Position relative to the
** circle
** @param [r] size [double] the font size
** @param [r] fontFamily [const AjPStr] the font Family 
** @param [r] fontStyle [const AjPStr] the  font Style
** @param [r] text [const AjPStr] the text of label to add
**
** @return [AjBool] ajTrue if correctly added
** @@
*********************************************************************/

AjBool ajXmlAddSectionLabelLinear(AjPGraphXml file, double xStart, 
				  double yStart, double xEnd, 
				  double yEnd, double width, 
				  double labelArmLength, 
				  const AjPStr labelStyle, 
				  double textPosition, double size, 
				  const AjPStr fontFamily,
				  const AjPStr fontStyle,
				  const AjPStr text)
{
    double xStartInner;
    double yStartInner;
    double xEndInner;
    double yEndInner;
    double textXStart;
    double textYStart;

    double xStartOuter;
    double yStartOuter;
    double xEndOuter;
    double yEndOuter;

    double xChange;
    double yChange;
    double angle;
    
    double xStartLabel;
    double xEndLabel;
    double yOuter;
    double yInner;

    /* I think I should have some error checking here, 
       but not sure what hugh */
    /*    
       if(!xml_PresentGraphicTypeIsC(file, "DNAPlot"))
           return ajFalse;
    */

    xChange = xEnd - xStart;
    yChange = yEnd - yStart;
    angle = atan(width / labelArmLength);

    if(yChange < 0)
	angle += (acos(0) * 2);

    textXStart = xStart + (sin(angle) * textPosition);
    textYStart = yStart + (sin(angle) * textPosition);

    xStartInner = xStart - (sin(angle) * (width / 2));
    yStartInner = yStart - (cos(angle) *(width / 2));
    xEndInner = xEnd - (sin(angle) *(width / 2));
    yEndInner = yEnd - (cos(angle) *(width / 2));
    xStartOuter = xStart + (sin(angle) *(width / 2));
    yStartOuter = yStart + (cos(angle) *(width / 2));
    xEndOuter = xEnd + (sin(angle) *(width / 2));
    yEndOuter = yEnd + (cos(angle) *(width / 2));

    if(text != NULL)
	ajXmlAddTextCentred(file, textXStart, textYStart, size, angle,
			    fontFamily, fontStyle, text);
    

    if(ajStrMatchC(labelStyle, "Cylinder"))
	ajXmlAddCylinder(file, xStart, yStart, xEnd, yEnd, width);
    else if(ajStrMatchC(labelStyle, "Block"))
    {
	ajXmlAddLine(file, xStartInner, yStartInner, xStartOuter, 
		     yStartOuter);
	ajXmlAddLine(file, xStartOuter, yStartOuter, xEndOuter, 
		     yEndOuter);
	ajXmlAddLine(file, xEndInner, yEndInner, xEndOuter, 
		     yEndOuter);
	ajXmlAddLine(file, xStartInner, yStartInner, xEndInner, 
		     yEndInner);
    }
    else if(ajStrMatchC(labelStyle, "SquareBracketed") || 
	    ajStrMatchC(labelStyle, "Arrowed") || 
	    ajStrMatchC(labelStyle, "Lined") )
    {
	ajXmlAddLine(file, xStart, yStart, xEnd, yEnd);

	xStartLabel = xStart + 2 * (sin(angle) * 
				    (labelArmLength / 2));
	xEndLabel = xEnd - 2 * (sin(angle) * (labelArmLength / 2));
	yOuter = yStart + 2 * (cos(angle) * (labelArmLength / 2));
	yInner = yStart - 2 * (cos(angle) * (labelArmLength / 2));

	if(ajStrMatchC(labelStyle, "SquareBracketed"))
	{
	    ajXmlAddLine(file, xStart, yInner, xStart, yOuter);
	    ajXmlAddLine(file, xStart, yInner, xStartLabel, yInner);
	    ajXmlAddLine(file, xStart, yOuter, xStartLabel,yOuter);
	    ajXmlAddLine(file, xEnd, yInner, xEnd, yOuter);
	    ajXmlAddLine(file, xEnd, yInner, xEndLabel, yInner);
	    ajXmlAddLine(file, xEnd, yOuter, xEndLabel, yOuter);
	}

	if(ajStrMatchC(labelStyle, "Arrowed"))
	{
	    ajXmlAddLine(file, xStart, yStart, xStartLabel, yOuter);
	    ajXmlAddLine(file, xStart, yStart, xStartLabel, yInner);
	    ajXmlAddLine(file, xEnd, yEnd, xEndLabel, yOuter);
	    ajXmlAddLine(file, xEnd, yEnd, xEndLabel, yInner);
	}

	if(ajStrMatchC(labelStyle, "Lined"))
	{
	    ajXmlAddLine(file, xStart, yOuter, xStart, yInner);
	}
    }
    else if(ajStrMatchC(labelStyle, "ReverseSquareBracketed") ||
	    ajStrMatchC(labelStyle, "ReverseArrowed") )
    {
	ajXmlAddLine(file, xStart, yStart, xEnd, yEnd);

	xStartLabel = xStart - 2 * (sin(angle) * 
				    (labelArmLength / 2));
	xEndLabel = xEnd + 2 * (sin(angle) * (labelArmLength / 2));
	yOuter = yStart + 2 * (cos(angle) * (labelArmLength / 2));
	yInner = yStart - 2 * (cos(angle) * (labelArmLength / 2));

	if(ajStrMatchC(labelStyle, "ReverseSquareBracketed"))
	{
	    ajXmlAddLine(file, xStart, yInner, xStart, yOuter);
	    ajXmlAddLine(file, xStart, yInner, xStartLabel, yInner);
	    ajXmlAddLine(file, xStart, yOuter, xStartLabel,yOuter);
	    ajXmlAddLine(file, xEnd, yInner, xEnd, yOuter);
	    ajXmlAddLine(file, xEnd, yInner, xEndLabel, yInner);
	    ajXmlAddLine(file, xEnd, yOuter, xEndLabel,yOuter);
	}

	if(ajStrMatchC(labelStyle, "ReverseArrowed"))
	{
	    ajXmlAddLine(file, xStart, yStart, xStartLabel, yOuter);
	    ajXmlAddLine(file, xStart, yStart, xStartLabel, yInner);
	    ajXmlAddLine(file, xEnd, yEnd, xEndLabel, yOuter);
	    ajXmlAddLine(file, xEnd, yEnd, xEndLabel, yInner);
	}
    }


    return ajTrue;
}




/* @func ajXmlAddSquareResidueLinear ***************************************
**
** adds a residue letter in a square
**
** @param [w] file [AjPGraphXml] the file to add the object to
** @param [r] residue [char] residue letter
** @param [r] x [float] the x coordinate
** @param [r] y [float] the y coordinate
**
** @return [AjBool] ajTrue if correctly added
** @@
*********************************************************************/

AjBool ajXmlAddSquareResidueLinear(AjPGraphXml file, char residue, 
				   float x, float y)
{ 
    float *xCoords;
    float *yCoords;
     
    AJCNEW(xCoords, 5);
    AJCNEW(yCoords, 5);
    
    xCoords[0] = x - RESIDUE_SQUARE_SIZE;
    xCoords[1] = x + RESIDUE_SQUARE_SIZE;
    xCoords[2] = x + RESIDUE_SQUARE_SIZE;
    xCoords[3] = x - RESIDUE_SQUARE_SIZE;
    xCoords[4] = x - RESIDUE_SQUARE_SIZE;
    
    yCoords[0] = y - RESIDUE_SQUARE_SIZE;
    yCoords[1] = y - RESIDUE_SQUARE_SIZE;
    yCoords[2] = y + RESIDUE_SQUARE_SIZE;
    yCoords[3] = y + RESIDUE_SQUARE_SIZE;
    yCoords[4] = y - RESIDUE_SQUARE_SIZE;
    
    ajXmlAddJoinedLineSetF(file, xCoords, yCoords, 5);

    ajXmlAddNakedResidueLinear(file, residue, x, y);
    
    AJFREE(xCoords);
    AJFREE(yCoords);

    return ajTrue;
}




/* @func ajXmlAddOctagonalResidueLinear ************************************
**
** adds a residue letter in a square
**
** @param [w] file [AjPGraphXml] the file to add the object to
** @param [r] residue [char] residue letter
** @param [r] x [float] the x coordinate
** @param [r] y [float] the y coordinate
**
** @return [AjBool] ajTrue if correctly added
** @@
*********************************************************************/

AjBool ajXmlAddOctagonalResidueLinear(AjPGraphXml file, char residue, 
				      float x, float y)
{
    float *xCoords;
    float *yCoords;

    AJCNEW(xCoords, 9);
    AJCNEW(yCoords, 9);
    
    xCoords[0] = x - (0.5 * RESIDUE_SQUARE_SIZE);
    xCoords[1] = x + (0.5 * RESIDUE_SQUARE_SIZE);
    xCoords[2] = x + RESIDUE_SQUARE_SIZE;
    xCoords[3] = x + RESIDUE_SQUARE_SIZE;
    xCoords[4] = x + (0.5 * RESIDUE_SQUARE_SIZE);
    xCoords[5] = x - (0.5 * RESIDUE_SQUARE_SIZE);
    xCoords[6] = x - RESIDUE_SQUARE_SIZE;
    xCoords[7] = x - RESIDUE_SQUARE_SIZE;
    xCoords[8] = x - (0.5 * RESIDUE_SQUARE_SIZE);
    
    yCoords[0] = y + RESIDUE_SQUARE_SIZE;
    yCoords[1] = y + RESIDUE_SQUARE_SIZE;
    yCoords[2] = y + (0.5 * RESIDUE_SQUARE_SIZE);
    yCoords[3] = y - (0.5 * RESIDUE_SQUARE_SIZE);
    yCoords[4] = y - RESIDUE_SQUARE_SIZE;
    yCoords[5] = y - RESIDUE_SQUARE_SIZE;
    yCoords[6] = y - (0.5 * RESIDUE_SQUARE_SIZE);
    yCoords[7] = y + (0.5 * RESIDUE_SQUARE_SIZE);
    yCoords[8] = y + RESIDUE_SQUARE_SIZE;
    
    ajXmlAddJoinedLineSetF(file, xCoords, yCoords, 9);

    ajXmlAddNakedResidueLinear(file, residue, x, y);
    
    AJFREE(xCoords);
    AJFREE(yCoords);

    return ajTrue;
}




/* @func ajXmlAddDiamondResidueLinear **************************************
**
** adds a residue letter in a square
**
** @param [w] file [AjPGraphXml] the file to add the object to
** @param [r] residue [char] residue letter
** @param [r] x [float] the x coordinate
** @param [r] y [float] the y coordinate
**
** @return [AjBool] ajTrue if correctly added
** @@
*********************************************************************/

AjBool ajXmlAddDiamondResidueLinear(AjPGraphXml file, char residue, 
				    float x, float y)
{    
    float *xCoords;
    float *yCoords;

    AJCNEW(xCoords, 5);
    AJCNEW(yCoords, 5);
    
    xCoords[0] = x;
    xCoords[1] = x + (1.41 * RESIDUE_SQUARE_SIZE);
    xCoords[2] = x;
    xCoords[3] = x - (1.41 * RESIDUE_SQUARE_SIZE);
    xCoords[4] = x;
    
    yCoords[0] = y + (1.41 * RESIDUE_SQUARE_SIZE);
    yCoords[1] = y;
    yCoords[2] = y - (1.41 * RESIDUE_SQUARE_SIZE);
    yCoords[3] = y;
    yCoords[4] = y + (1.41 * RESIDUE_SQUARE_SIZE);
    
    ajXmlAddJoinedLineSetF(file, xCoords, yCoords, 5);

    ajXmlAddNakedResidueLinear(file, residue, x, y);
    
    AJFREE(xCoords);
    AJFREE(yCoords);

    return ajTrue;
}




/* @func ajXmlAddNakedResidueLinear **********************************
**
** adds a residue letter in a square
**
** @param [w] file [AjPGraphXml] the file to add the object to
** @param [r] residue [char] residue letter
** @param [r] x [float] the x coordinate
** @param [r] y [float] the y coordinate
**
** @return [AjBool] ajTrue if correctly added
** @@
*********************************************************************/

AjBool ajXmlAddNakedResidueLinear(AjPGraphXml file, char residue, 
				  float x, float y)
{    
    static char residueString[2];

    residueString[1]='\0';
    *residueString=residue;

    ajXmlAddTextC(file, (x - 1.2), (y - 1.2), 4.0, 0, "TYPEWRITER",
		 NULL, residueString);
    
    return ajTrue;
}




/* @func ajXmlAddSquareResidue ***************************************
**
** adds a residue letter in a square
**
** @param [w] file [AjPGraphXml] the file to add the object to
** @param [r] residue [char] residue letter
** @param [r] radius [double] the polar radius
** @param [r] angle [double] the angle in degrees
**
** @return [AjBool] ajTrue if correctly added
** @@
*********************************************************************/

AjBool ajXmlAddSquareResidue(AjPGraphXml file, char residue, 
			     double radius, double angle)
{ 
    float squareSize;
    
    float  x;
    float  y;
 
    float *xCoords;
    float *yCoords;

    squareSize = SQUARESIZE;
     
    ajPolToRec(radius, angle, &x, &y);

    AJCNEW(xCoords, 5);
    AJCNEW(yCoords, 5);
    
    xCoords[0] = x - squareSize;
    xCoords[1] = x + squareSize;
    xCoords[2] = x + squareSize;
    xCoords[3] = x - squareSize;
    xCoords[4] = x - squareSize;
    
    yCoords[0] = y - squareSize;
    yCoords[1] = y - squareSize;
    yCoords[2] = y + squareSize;
    yCoords[3] = y + squareSize;
    yCoords[4] = y - squareSize;
    
    ajXmlAddJoinedLineSetF(file, xCoords, yCoords, 5);

    ajXmlAddNakedResidue(file, residue, radius, angle);
    
    AJFREE(xCoords);
    AJFREE(yCoords);

    return ajTrue;
}




/* @func ajXmlAddOctagonalResidue ************************************
**
** adds a residue letter in a square
**
** @param [w] file [AjPGraphXml] the file to add the object to
** @param [r] residue [char] residue letter
** @param [r] radius [double] the polar radius
** @param [r] angle [double] the angle in degrees
**
** @return [AjBool] ajTrue if correctly added
** @@
*********************************************************************/

AjBool ajXmlAddOctagonalResidue(AjPGraphXml file, char residue, 
				double radius, double angle)
{
    float squareSize;
    
    float  x;
    float  y;
 
    float *xCoords;
    float *yCoords;

    squareSize = SQUARESIZE;
    
    ajPolToRec(radius, angle, &x, &y);

    AJCNEW(xCoords, 9);
    AJCNEW(yCoords, 9);
    
    xCoords[0] = x - (0.5 * squareSize);
    xCoords[1] = x + (0.5 * squareSize);
    xCoords[2] = x + squareSize;
    xCoords[3] = x + squareSize;
    xCoords[4] = x + (0.5 * squareSize);
    xCoords[5] = x - (0.5 * squareSize);
    xCoords[6] = x - squareSize;
    xCoords[7] = x - squareSize;
    xCoords[8] = x - (0.5 * squareSize);
    
    yCoords[0] = y + squareSize;
    yCoords[1] = y + squareSize;
    yCoords[2] = y + (0.5 * squareSize);
    yCoords[3] = y - (0.5 * squareSize);
    yCoords[4] = y - squareSize;
    yCoords[5] = y - squareSize;
    yCoords[6] = y - (0.5 * squareSize);
    yCoords[7] = y + (0.5 * squareSize);
    yCoords[8] = y + squareSize;
    
    ajXmlAddJoinedLineSetF(file, xCoords, yCoords, 9);

    ajXmlAddNakedResidue(file, residue, radius, angle);
    
    AJFREE(xCoords);
    AJFREE(yCoords);

    return ajTrue;
}




/* @func ajXmlAddDiamondResidue **************************************
**
** adds a residue letter in a square
**
** @param [w] file [AjPGraphXml] the file to add the object to
** @param [r] residue [char] residue letter
** @param [r] radius [double] the polar radius
** @param [r] angle [double] the angle in degrees
**
** @return [AjBool] ajTrue if correctly added
** @@
*********************************************************************/

AjBool ajXmlAddDiamondResidue(AjPGraphXml file, char residue, 
			      double radius, double angle)
{    
    float squareSize;
    
    float  x;
    float  y;
 
    float *xCoords;
    float *yCoords;

    squareSize = SQUARESIZE;

    ajPolToRec(radius, angle, &x, &y);

    AJCNEW(xCoords, 5);
    AJCNEW(yCoords, 5);
    
    xCoords[0] = x;
    xCoords[1] = x + (1.41 * squareSize);
    xCoords[2] = x;
    xCoords[3] = x - (1.41 * squareSize);
    xCoords[4] = x;
    
    yCoords[0] = y + (1.41 * squareSize);
    yCoords[1] = y;
    yCoords[2] = y - (1.41 * squareSize);
    yCoords[3] = y;
    yCoords[4] = y + (1.41 * squareSize);
    
    ajXmlAddJoinedLineSetF(file, xCoords, yCoords, 5);

    ajXmlAddNakedResidue(file, residue, radius, angle);
    
    AJFREE(xCoords);
    AJFREE(yCoords);

    return ajTrue;
}




/* @func ajXmlAddNakedResidue ****************************************
**
** adds a residue letter in a square
**
** @param [w] file [AjPGraphXml] the file to add the object to
** @param [r] residue [char] residue letter
** @param [r] radius [double] the polar radius
** @param [r] angle [double] the angle in degrees
**
** @return [AjBool] ajTrue if correctly added
** @@
*********************************************************************/

AjBool ajXmlAddNakedResidue(AjPGraphXml file, char residue, 
			    double radius, double angle)
{    
    float  x;
    float  y;
    static char residueString[2];

    residueString[1]='\0';
    *residueString=residue;

    ajPolToRec(radius, angle, &x, &y);

    ajXmlAddTextC(file, (x - 0.018), (y - 0.018), 0.07, 0, "TYPEWRITER",
		  NULL, residueString);
    
    return ajTrue;
}




/* @func ajXmlFitTextOnLine ******************************************
**
** Computes the character size (in vrml terms) needed to write a text 
** string with specified length (in world meters).  The length 
** of the string is the distance between (x1,y1) and (x2,y2)
**
** @param [r] x1 [float] x1 coor.
** @param [r] y1 [float] y1 coor.
** @param [r] x2 [float] x2 coor.
** @param [r] y2 [float] y2 coor.
** @param [r] text [const AjPStr] The text to be displayed.
** @return [float] The character size (in mm) that fits the specified
**                 height and length.
** @@
*********************************************************************/

float ajXmlFitTextOnLine(float x1, float y1, float x2, float y2, 
			 const AjPStr text)
{
    float lineLength;
    ajint textLength;
    
    textLength = ajStrGetLen(text);

    lineLength = pow((pow((x2 - x1), 2) + pow((y2 - y1), 2)), 0.5);
    
    return (lineLength / textLength) * ONE_METER_FONT;
}




/* AJB  Hugh, this function doesn't return anything anywhere! */

/* @func ajXmlGetColour **********************************************
**
** gets Colour
**
** @param [r] file [const AjPGraphXml] the file get colour of
** @param [w] r [double*] red component
** @param [w] g [double*] green component
** @param [w] b [double*] blue component
**
** @return [void]
** @@
*********************************************************************/

void ajXmlGetColour(const AjPGraphXml file, double *r, double *g, double *b)
{
    *r = file->colour[0];
    *g = file->colour[1];
    *b = file->colour[2];

    return;
}




/* @func ajXmlSetColour **********************************************
**
** sets Colour
**
** @param [w] file [AjPGraphXml] the file to alter
** @param [r] r [double] red component
** @param [r] g [double] green component
** @param [r] b [double] blue component
**
** @return [void]
** @@
*********************************************************************/

void ajXmlSetColour(AjPGraphXml file, double r, double g, double b)
{
    /* should I make a private function do this? Hardly worth it,*/
    file->colour[0] = r;
    file->colour[1] = g;
    file->colour[2] = b;

    return;
}




/* @func ajXmlSetColourFromCode *************************************
**
** sets Colour from the code.  Codes are -
**
** 0 black (default background)
** 1 red
** 2 yellow
** 3 green
** 4 aquamarine
** 5 pink
** 6 wheat
** 7 grey
** 8 brown
** 9 blue
** 10 BlueViolet
** 11 cyan
** 12 turquoise
** 13 magenta
** 14 salmon
** 15 white (default foreground)
**
** @param [w] file [AjPGraphXml] the file to alter
** @param [r] colour [ajint] colour
**
** @return [void]
** @@
*********************************************************************/

void ajXmlSetColourFromCode(AjPGraphXml file, ajint colour)
{
    switch (colour)
    {
        case 0:
            ajXmlSetColour(file, 0.0, 0.0, 0.0);
            break;
        case 1:
            ajXmlSetColour(file, 1.0, 0.0, 0.0);
            break;
        case 2:
            ajXmlSetColour(file, 1.0, 1.0, 0.0);
            break;
        case 3:
            ajXmlSetColour(file, 0.0, 1.0, 0.0);
            break;
        case 4:
            ajXmlSetColour(file, 0.0, 1.0, 1.0);
            break;
        case 5:
            ajXmlSetColour(file, 0.8, 0.2, 0.2);
            break;
        case 6:
            ajXmlSetColour(file, 0.5, 0.5, 0.1);
            break;
        case 7:
            ajXmlSetColour(file, 0.5, 0.5, 0.5);
            break;
        case 8:
            ajXmlSetColour(file, 0.4, 0.4, 0.4);
            break;
        case 9:
            ajXmlSetColour(file, 0.0, 0.0, 1.0);
            break;
        case 10:
            ajXmlSetColour(file, 1, 0, 1);
            break;
        case 11:
            ajXmlSetColour(file, 0.2, 0.2, 0.8);
            break;
        case 12:
            ajXmlSetColour(file, 0.2, 0.8, 0.2);
            break;
        case 13:
            ajXmlSetColour(file, 0.6, 0.4, 0.2);
            break;
        case 14:
            ajXmlSetColour(file, 0.5, 0.2, 0.2);
            break;
        case 15:
            ajXmlSetColour(file, 1, 1, 1);
            break;
        default:
            break;
    }    

    return;
}




/* @func ajXmlCreateNewOutputFile ************************************
**
** Sets up the output file 
**
** @return [AjPGraphXml] a colection of pointers to the file created
** @@
*********************************************************************/

AjPGraphXml ajXmlCreateNewOutputFile()
{
    return xml_CreateNewOutputFile();
}




/* @func ajXmlAddGraphic ********************************************
**
** adds a general graphics group
**
** @param [w] file [AjPGraphXml] the file to add the object to
** @param [r] type [const AjPStr] the type of graphic to add
**
** @return [void]
** @@
*********************************************************************/

void ajXmlAddGraphic(AjPGraphXml file, const AjPStr type)
{
    ajXmlAddGraphicC(file, ajStrGetPtr(type));

    return;
}




/* @func ajXmlAddGraphicC *********************************************
**
** adds a general graphics group
**
** @param [w] file [AjPGraphXml] the file to add the object to
** @param [r] type [const char *] the type of graphic to add
**
** @return [void]
** @@
*********************************************************************/

void ajXmlAddGraphicC(AjPGraphXml file, const char *type)
{
    AjPXmlNode el = NULL;

    if(type == NULL)
    {
	/* I create a new element, the group */
	el = xml_MakeNewNodeC(file, "Group", xml_GetCurrentScene(file));

	/* I move file->currentGraphic to the children */
	xml_SetCurrentGraphic(file, el);
    } 
    else 
    {
	if(!strcmp(type, "Graph")) 
	{
	    if(xml_FileNeedsProtoDeclareC(file, "Graph"))
		xml_AddGraphProto(file);

	    el = xml_MakeNewNodeC(file, "ProtoInstance", file->currentScene);
	    xml_SetAttributeC(el, "name", type);
	    xml_SetCurrentGraphic(file, el);   
	}
	else 
	    if(!strcmp(type, "Histogram")) 
	    {
		if(xml_FileNeedsProtoDeclareC(file, "Graph"))
		    xml_AddGraphProto(file);

		el = xml_MakeNewNodeC(file, type, file->currentScene);
		xml_SetCurrentGraphic(file, el);   
	    }
	    else 
	    {
		if(!strcmp(type, "DNAPlot")) 
		{
		    if(xml_FileNeedsProtoDeclareC(file, "DNAPlot"))
			xml_AddDNAPlotProto(file);

		    el = xml_MakeNewNodeC(file, type, file->currentScene);
		    xml_SetCurrentGraphic(file, el);   
		}
	    }
    }


    if(file->nodeTypes)
	file->nodeTypes = ajTablestrNewLen(1);
    
    /*  el unrefed above if type == NULL */
    /* look to see if I need some checking here hugh */

    return;
}




/* @func ajXmlAddArc ***********************************************
**
** adds an arc
**
** @param [w] file [AjPGraphXml] the file to add the max min values to
** @param [r] xCentre [double] the x value of the centre of the 
** circle
** @param [r] yCentre [double] the y value of the centre of the 
** circle
** @param [r] startAngle [double] the startAngle of the arc
** @param [r] endAngle [double] the endAngle of the arc
** @param [r] radius [double] the radius of the circle
**
** @return [void]
** @@
*********************************************************************/

void ajXmlAddArc(AjPGraphXml file, double xCentre, double yCentre, 
		 double startAngle, double endAngle, double radius)
{
    double angularExtent;
    double modAngularExtent;
    double arcCentreAngle;
    
    angularExtent = endAngle - startAngle;
    modAngularExtent = pow(pow(angularExtent, 2), 0.5);
    
    if(xml_AngleIsInSecondHalfOfCircle(modAngularExtent))
    {
	arcCentreAngle = startAngle + (angularExtent / 2);
	xml_AddArc(file, xCentre, yCentre, startAngle, arcCentreAngle,
		   radius);
	xml_AddArc(file, xCentre, yCentre, arcCentreAngle, endAngle,
		   radius);
    }
    else
	xml_AddArc(file, xCentre, yCentre, startAngle, endAngle,
		   radius);

    return;
}




/* @func ajXmlAddCircleF *********************************************
**
** adds a circle
**
** @param [w] file [AjPGraphXml] the file to add the max min values to
** @param [r] xCentre [float] the x value of the centre of the 
** circle
** @param [r] yCentre [float] the y value of the centre of the 
** circle
** @param [r] radius [float] the radius of the circle
**
** @return [void]
** @@
******************************************************************************/

void ajXmlAddCircleF(AjPGraphXml file, float xCentre, float yCentre, 
		     float radius)
{
    ajXmlAddCircle(file, (double) xCentre, (double) yCentre, 
		   (double) radius);

    return;
}




/* @func ajXmlAddCircle *********************************************
**
** adds a circle
**
** @param [w] file [AjPGraphXml] the file to add the max min values to
** @param [r] xCentre [double] the x value of the centre of the 
** circle
** @param [r] yCentre [double] the y value of the centre of the 
** circle
** @param [r] radius [double] the radius of the circle
**
** @return [void]
** @@
*********************************************************************/

void ajXmlAddCircle(AjPGraphXml file, double xCentre, double yCentre, 
		    double radius)
{
    AjPStr temp          = NULL;
    AjPStr controlPoints = NULL;
    AjPXmlNode circleNode;

    double controlPointsDbs[14] =
    {
        0, 1,
        1.732050808, 1,
        0.866025403, -0.5,
        0, 2,
        -0.866025403, -0.5,
        -1.732050808, 1,
        0, 1,
    };
    int i;

    temp = ajStrNew();
    controlPoints = ajStrNew();

    for(i=0; i<14; ++i)
	controlPointsDbs[i] *= radius;

    for(i=0; i<14; i+=2)
    {
	controlPointsDbs[i] += xCentre;
	controlPointsDbs[i+1] += yCentre;
    }

    for(i=0; i<14; i+=2)
	xml_AddACoord(controlPointsDbs[i], controlPointsDbs[i+1], 
		      ajFalse, &controlPoints, &temp);
    
    circleNode = xml_MakeNewShapeNodeC(file, 
				       xml_GetCurrentGraphic(file), 
				       "NurbsCurve2D");
    xml_SetAttributeC(circleNode, "knots", "0 0 0 1 1 2 2 3 3 3");
    xml_SetAttributeC(circleNode, "order", "3");
    xml_SetAttributeC(circleNode, "controlPoint", ajStrGetPtr(controlPoints));
    xml_SetAttributeC(circleNode, "weight", "1 0.5 1 0.5 1 0.5 1");

    ajStrDel(&controlPoints);
    ajStrDel(&temp);
    
    return;
}




/* @func ajXmlAddGroutOption ****************************************
**
** adds an option value pair for the Grout display
**
** @param [w] file [AjPGraphXml] the file to add the max min values to
** @param [r] name [const AjPStr] name of option
** @param [r] value [const AjPStr] value of option
**
** @return [void]
** @@
*********************************************************************/

void ajXmlAddGroutOption(AjPGraphXml file,
			 const AjPStr name, const AjPStr value)
{
    ajXmlAddGroutOptionC(file, ajStrGetPtr(name), ajStrGetPtr(value));

    return;
}




/* @func ajXmlAddGroutOptionC ****************************************
**
** adds an option value pair for the Grout display
**
** @param [w] file [AjPGraphXml] the file to add the max min values to
** @param [r] name [const char *] name of option
** @param [r] value [const char *] value of option
**
** @return [void]
** @@
*********************************************************************/

void ajXmlAddGroutOptionC(AjPGraphXml file,
			  const char *name, const char *value)
{
    AjPXmlNode headNode  = NULL;
    AjPXmlNode otherNode = NULL;
    GdomeException exc;
    int i;
    GdomeNodeList *listShapes = NULL;
    GdomeDOMString *nodeName;
    ajint limit;
    
    otherNode = xml_SetNode((GdomeNode *) gdome_doc_documentElement 
			    (file->doc, &exc));
    nodeName = gdome_str_mkref("head");
    listShapes = gdome_el_getElementsByTagName
	((xml_GetNodeElement(otherNode)), 
	 nodeName, &exc);

    limit = gdome_nl_length(listShapes, &exc);

    for(i=0; i<limit && headNode == NULL; ++i)
	headNode = xml_SetNode(gdome_nl_item(listShapes, i, &exc));


    if(headNode == NULL)
	headNode = xml_MakeNewNodeC(file, "head", otherNode);
    
    xml_UnrefNode(&otherNode);
    otherNode = xml_MakeNewNodeC(file, "meta", headNode);
    xml_SetAttributeC(otherNode, "name", name);
    xml_SetAttributeC(otherNode, "content", value);

    xml_UnrefNode(&otherNode);
    xml_UnrefNode(&headNode);
    gdome_str_unref(nodeName);
    gdome_nl_unref(listShapes, &exc);

    return;
}




/* @funcstatic xml_AddCylinder ********************************************
**
** Adds a Cylinder
**
** @param [w] file [AjPGraphXml] the file to add the Cylinder to
** @param [r] xCentre [double] x coordinate of centre
** @param [r] yCentre [double] y coordinate of centre
** @param [r] angle [double] angle relative to vertical
** @param [r] height [double] height
** @param [r] width [double] width 
**
** @return [void]
** @@
*********************************************************************/

static void xml_AddCylinder(AjPGraphXml file, double xCentre, double yCentre, 
			    double angle, double height, double width)
{
    AjPStr translation  = NULL;
    AjPStr rotation     = NULL;
    AjPStr temp         = NULL;
    AjPXmlNode transformNode;
    AjPXmlNode elCylinder;

    temp        = ajStrNew();
    translation = ajStrNew();
    rotation    = ajStrNewC("0 0 1 ");    

    xml_StrFromDouble(&translation, xCentre);
    ajStrAppendC(&translation, " ");
    xml_StrFromDouble(&temp, yCentre);
    ajStrAppendS(&translation, temp);
    ajStrAppendC(&translation, " 0");

    ajStrAssignClear(&temp);
    xml_StrFromDouble(&temp, angle);
    ajStrAppendS(&rotation, temp);
    
    transformNode = xml_MakeNewNodeC(file, "Transform", 
				    xml_GetCurrentGraphic(file));

    xml_SetAttributeC(transformNode, "translation", ajStrGetPtr(translation));
    xml_SetAttributeC(transformNode, "rotation", ajStrGetPtr(rotation));
  

    elCylinder = xml_MakeNewShapeNodeC(file, transformNode, 
				      "Cylinder");

    ajStrAssignClear(&temp);
    xml_StrFromDouble(&temp, height);
    xml_SetAttributeC(elCylinder, "height", ajStrGetPtr(temp));
    ajStrAssignClear(&temp);
    xml_StrFromDouble(&temp, width);
    xml_SetAttributeC(elCylinder, "width", ajStrGetPtr(temp));

    ajStrDel(&temp);
    ajStrDel(&rotation);
    ajStrDel(&translation);

    xml_UnrefNode(&elCylinder);
    xml_UnrefNode(&transformNode);

    return;
}




/* @funcstatic xml_StrFromBool *********************************************
**
** converts an AjBool into a string
**
** @param [r] boole [AjBool] the AjBool
**
** @return [AjPStr] "TRUE" or "FALSE" depending on the AjBool
** @@
*********************************************************************/

static AjPStr xml_StrFromBool(AjBool boole)
{
    AjPStr text = NULL;
    
    if(boole)
	text = ajStrNewC("TRUE");
    else
	text = ajStrNewC("FALSE");

    return text;
}




/* @funcstatic xml_StrFromDouble ************************************
**
** Local Method
** returns a string with the correct number of decimal places 
**
** @param [w] result [AjPStr*] the string to change to double
** @param [r] val [double] the double to change to string
**
** @return [AjBool] ajTrue if the string was correctly assigned
** @@
*********************************************************************/

static AjBool xml_StrFromDouble(AjPStr *result, double val)
{
    AjPStr whole    = NULL;
    AjPStr signStr  = NULL;
    AjPStr mantisa  = NULL;
    AjPStr exponent = NULL;
    int sign;
    int i;
    int exponentInt;
    int decimalPlaces;
    AjBool iret;
    
    whole = ajStrNew();
    signStr = ajStrNew();
    mantisa = ajStrNew();
    exponent = ajStrNew();

    ajStrFromDoubleExp(&whole, val, 14);

    ajStrAssignSubC(&signStr, ajStrGetPtr(whole), 0, 0);

    if(ajStrMatchC(signStr, "-"))
	sign = 1;
    else
	sign = 0;

    ajStrAssignSubC(&mantisa, ajStrGetPtr(whole), (0+sign), (15+sign));
    ajStrAssignSubC(&exponent, ajStrGetPtr(whole), (17+sign), (20+sign));
    ajStrToInt(exponent, &exponentInt);

    for(i=14; ajStrSuffixC(mantisa, "0"); --i)
	iret = ajStrCutEnd(&mantisa,1);

    decimalPlaces = i-exponentInt;

    if(decimalPlaces<0)
	decimalPlaces = 0;

    ajStrDel(&whole);
    ajStrDel(&signStr);
    ajStrDel(&mantisa);
    ajStrDel(&exponent);

    return ajStrFromDouble(result, val, decimalPlaces); 
}




/* @funcstatic xml_AngleIsInSecondHalfOfCircle *****************************
**
** Calculates if this angle is in the second half of the circle
**
** @param [r] angle [double] the angle
**
** @return [AjBool] ajTrue if the angle is in the second half of the 
** circle
** @@
*********************************************************************/

static AjBool xml_AngleIsInSecondHalfOfCircle(double angle)
{
    int halfCount;
    int intHalfHalfCount;
    double halfHalfCount;
    double doubleIntHalfHalfCount;
    
    /* there must be a better way to do it hugh */
    halfCount = (int) angle / (acos(0) * 2);
    halfHalfCount = ((double) halfCount) / 2.0;
    intHalfHalfCount = ((int) halfHalfCount);  
    doubleIntHalfHalfCount = ((double) intHalfHalfCount);
    
    if(halfHalfCount == doubleIntHalfHalfCount)
	return ajFalse;

    return ajTrue;
}




/* @funcstatic xml_PresentGraphicTypeIs ****************************
**
** Local Method
** 
**
** @param [w] file [AjPGraphXml] the file to add the node to
** @param [r] name [const AjPStr] name graphic should be
**
** @return [AjBool] True if graphic type is name
** @@
*********************************************************************/

static AjBool xml_PresentGraphicTypeIs(AjPGraphXml file, const AjPStr name)
{
    AjBool returnValue    = ajFalse;
    AjPStr attributeValue = NULL;

    attributeValue = xml_GetAttributeC(xml_GetCurrentGraphic(file), 
				       "name");

    returnValue = ajStrMatchS(attributeValue, name);
  
    if(ajStrMatchC(attributeValue, "") && (name == NULL))
	returnValue = ajTrue;

    ajStrDel(&attributeValue);

    return returnValue;
}




/* @funcstatic xml_PresentGraphicTypeIsC ****************************
**
** Local Method
** 
**
** @param [w] file [AjPGraphXml] the file to add the node to
** @param [r] name [const char *] name graphic should be
**
** @return [AjBool] True if graphic type is name
** @@
*********************************************************************/

static AjBool xml_PresentGraphicTypeIsC(AjPGraphXml file, const char *name)
{
    AjBool returnValue    = ajFalse;
    AjPStr attributeValue = NULL;

    attributeValue = xml_GetAttributeC(xml_GetCurrentGraphic(file), 
				       "name");

    returnValue = ajStrMatchC(attributeValue, name);
  
    if(ajStrMatchC(attributeValue, "") && (name == NULL))
	returnValue = ajTrue;

    ajStrDel(&attributeValue);

    return returnValue;
}




/* @funcstatic xml_GetAttributeC *************************************
**
** Local Method
** Gets an attribute
**
** @param [w] node [AjPXmlNode] node to set attribute of
** @param [r] atName [const char *] name of attribute
**
** @return [AjPStr] value of attribute
** @@
*********************************************************************/

static AjPStr xml_GetAttributeC(AjPXmlNode node, const char *atName)
{
    GdomeDOMString *attributeValue;
    GdomeException exc;

    AjPStr temp = NULL;

    GdomeDOMString *nodeName = NULL;
    
    nodeName = gdome_str_mkref_dup(atName);
    attributeValue = gdome_el_getAttribute (xml_GetNodeElement(node),
					    nodeName, &exc);
    
    temp = ajStrNewC(attributeValue->str);
    gdome_str_unref(attributeValue);
    gdome_str_unref (nodeName);
    
    return temp;
}




/* @funcstatic xml_GetAttribute *************************************
**
** Local Method
** Gets an attribute
**
** @param [w] node [AjPXmlNode] node to set attribute of
** @param [r] atName [const AjPStr] name of attribute
**
** @return [AjPStr] value of attribute
** @@
*********************************************************************/

static AjPStr xml_GetAttribute(AjPXmlNode node, const AjPStr atName)
{

    return xml_GetAttributeC(node,ajStrGetPtr(atName));
}




/* @funcstatic xml_SetAttributeC ************************************
**
** Local Method
** Sets an attribute from a char*
**
** @param [w] node [AjPXmlNode] node to set attribute of
** @param [r] atName [const char*] name of attribute
** @param [r] atValue [const char*] value of attribute
**
** @@
*********************************************************************/

static void xml_SetAttributeC(AjPXmlNode node,
			      const char *atName, const char *atValue)
{
    GdomeException exc;
    GdomeDOMString *nodeName  = NULL;
    GdomeDOMString *nodeName2 = NULL;
    
    nodeName = gdome_str_mkref_dup(atName);
    nodeName2 = gdome_str_mkref_dup(atValue);
    gdome_el_setAttribute (xml_GetNodeElement(node), nodeName, 
			   nodeName2, &exc);
    gdome_str_unref(nodeName);
    gdome_str_unref(nodeName2);

    return;
}




/* @funcstatic xml_SetAttribute ************************************
**
** Local Method
** Sets an attribute
**
** @param [w] node [AjPXmlNode] node to set attribute of
** @param [r] atName [const AjPStr] name of attribute
** @param [r] atValue [const AjPStr] value of attribute
**
** @@
*********************************************************************/

static void xml_SetAttribute(AjPXmlNode node,
			     const AjPStr atName, const AjPStr atValue)
{
    xml_SetAttributeC(node, ajStrGetPtr(atName), ajStrGetPtr(atValue));
    
    return;
}




/* @funcstatic xml_GetIndex ****************************************
**
** Local Method
** Gets the list of coord index's for an Indexed Line Set
**
** @param [u] passedNode [AjPXmlNode] node to get the index's of
**
** @return [AjPStr] value of index's
** @@
*********************************************************************/

static AjPStr xml_GetIndex(AjPXmlNode passedNode)
{
    GdomeException exc;
    GdomeDOMString *nodeName = NULL;
    AjBool proto;
    AjPStr ajAttributeValue  = NULL;
    AjPXmlNode tempNode      = NULL;
    AjPXmlNode returnNode    = NULL;
    AjPXmlNode node          = NULL;
    AjPStr temp              = NULL;

    GdomeNodeList *listShapes = NULL;
    int i;
    ajint limit;

    proto = ajFalse;
    
    nodeName = gdome_el_tagName(xml_GetNodeElement(passedNode), &exc);
    
    if(ajCharMatchC("Shape", nodeName->str))
    {
	gdome_str_unref(nodeName);
	nodeName = gdome_str_mkref("IndexedLineSet");
	listShapes = gdome_el_getElementsByTagName
	    (xml_GetNodeElement(passedNode), nodeName, &exc);
	limit = gdome_nl_length(listShapes, &exc);

	for(i=0; i<limit && node == NULL; ++i)
	    node = xml_SetNode(gdome_nl_item(listShapes, i, &exc));

	gdome_nl_unref(listShapes, &exc);

	if(node == NULL)
	{
	    gdome_str_unref(nodeName);
	    nodeName = gdome_str_mkref("IndexedFaceSet");
	    listShapes = gdome_el_getElementsByTagName
		(xml_GetNodeElement(passedNode), nodeName, &exc);
	    limit = gdome_nl_length(listShapes, &exc);

	    for(i=0; i<limit && node == NULL; ++i)
		node = xml_SetNode(gdome_nl_item(listShapes, i, &exc));

	    gdome_nl_unref(listShapes, &exc);
	}
    }
    else
	node = passedNode;

    gdome_str_unref(nodeName);
    nodeName = gdome_el_tagName(xml_GetNodeElement(passedNode), &exc);

    if(ajCharMatchC("ProtoInstance", nodeName->str))
    {
	ajAttributeValue = xml_GetAttributeC(node, "name");

	if(ajStrMatchC(ajAttributeValue, "Graph"))
	    proto = ajTrue;

	ajStrDel(&ajAttributeValue);
    }

    gdome_str_unref(nodeName);

    if(proto)
    {
	nodeName = gdome_el_tagName(xml_GetNodeElement(node), &exc);

	gdome_str_unref(nodeName);
	nodeName = gdome_str_mkref("fieldValue");
	listShapes = gdome_el_getElementsByTagName
	    (xml_GetNodeElement(node), nodeName, &exc);
	limit = gdome_nl_length(listShapes, &exc);

	for(i=0; i<limit && returnNode == NULL; ++i)
	{
	    tempNode = xml_SetNode(gdome_nl_item(listShapes, i, &exc));
	    ajAttributeValue = xml_GetAttributeC(tempNode, 
						 "name");

	    if(ajStrMatchC(ajAttributeValue, "Graph.index"))
		temp = xml_GetAttributeC(tempNode, "value");
	    
	    ajStrDel(&ajAttributeValue);
	    xml_UnrefNode(&tempNode);
	}
	gdome_str_unref(nodeName);
	gdome_nl_unref(listShapes, &exc);
    }
    else
    {
	nodeName = gdome_el_tagName(xml_GetNodeElement(node), &exc);

	/*
	   if(ajCharMatchC("Shape", nodeName->str))
	   {
	   gdome_str_unref(nodeName);
	   nodeName = gdome_str_mkref("IndexedFaceSet");
	   listShapes = gdome_el_getElementsByTagName
	   (xml_GetNodeElement(passedNode), nodeName, &exc);
	   for(i=0; (i<gdome_nl_length(listShapes, &exc) 
	   && node == NULL) ; ++i)
	   {
	   tempNode = xml_SetNode(gdome_nl_item(listShapes, i, &exc));
	   }
	   gdome_nl_unref(listShapes, &exc);
	   temp = xml_GetAttribute(tempNode, "coordIndex");
	   xml_UnrefNode(&tempNode);	    
	   }
	   else
	   */
	{
	    temp = xml_GetAttributeC(node, "coordIndex");
	}
	
    }
    
    return temp;
}




/* @funcstatic xml_SetIndex ****************************************
**
** Local Method
** Sets the list of coord index's for an Indexed Line Set
**
** @param [u] passedNode [AjPXmlNode] node to get the index's of
** @param [r] index [const AjPStr] value of index's
**
** @return [void]
** @@
*********************************************************************/

static void xml_SetIndex(AjPXmlNode passedNode, const AjPStr index)
{
    GdomeException exc;
    GdomeDOMString *nodeName  = NULL;
    AjBool proto;
    AjPStr ajAttributeValue   = NULL;
    AjPXmlNode tempNode       = NULL;
    AjPXmlNode returnNode     = NULL;
    AjPXmlNode node           = NULL;
    GdomeNodeList *listShapes = NULL;
    int i;
    ajint limit;
    
    proto = ajFalse;
    
    nodeName = gdome_el_tagName(xml_GetNodeElement(passedNode), &exc);

    if(ajCharMatchC("Shape", nodeName->str))
    {
	nodeName = gdome_str_mkref("IndexedLineSet");
	gdome_str_unref(nodeName);
	listShapes = gdome_el_getElementsByTagName
	    (xml_GetNodeElement(passedNode), nodeName, &exc);
	limit = gdome_nl_length(listShapes, &exc);

	for(i=0; i<limit && node == NULL; ++i)
	    node = xml_SetNode(gdome_nl_item(listShapes, i, &exc));

	gdome_nl_unref(listShapes, &exc);

	if(node == NULL)
	{
	    gdome_str_unref(nodeName);
	    nodeName = gdome_str_mkref("IndexedFaceSet");
	    listShapes = gdome_el_getElementsByTagName
		(xml_GetNodeElement(passedNode), nodeName, &exc);
	    limit = gdome_nl_length(listShapes, &exc);

	    for(i=0; i<limit && node == NULL; ++i)
		node = xml_SetNode(gdome_nl_item(listShapes, i, &exc));

	    gdome_nl_unref(listShapes, &exc);
	}
    }
    else
	node = passedNode;

    gdome_str_unref(nodeName);
    nodeName = gdome_el_tagName(xml_GetNodeElement(passedNode), &exc);

    if(ajCharMatchC("ProtoInstance", nodeName->str))
    {
	ajAttributeValue = xml_GetAttributeC(node, "name");

	if(ajStrMatchC(ajAttributeValue, "Graph"))
	    proto = ajTrue;

	ajStrDel(&ajAttributeValue);
    }

    gdome_str_unref(nodeName);
    
    if(proto)
    {
	nodeName = gdome_str_mkref("fieldValue");
	listShapes = gdome_el_getElementsByTagName
	    (xml_GetNodeElement(node), nodeName, &exc);
	limit = gdome_nl_length(listShapes, &exc);

	for(i=0; i<limit && returnNode == NULL; ++i)
	{
	    tempNode = xml_SetNode(gdome_nl_item(listShapes, i, &exc));

	    ajAttributeValue = xml_GetAttributeC(tempNode, 
						 "name");

	    if(ajStrMatchC(ajAttributeValue, "Graph.index"))
		xml_SetAttributeC(tempNode, "value", ajStrGetPtr(index));

	    ajStrDel(&ajAttributeValue);
	    xml_UnrefNode(&tempNode);
	}

	gdome_str_unref(nodeName);
	gdome_nl_unref(listShapes, &exc);
    }
    else
    {
	if(ajCharMatchC("Shape", nodeName->str))
	{
	    gdome_str_unref(nodeName);
	    nodeName = gdome_str_mkref("IndexedFaceSet");
	    listShapes = gdome_el_getElementsByTagName
		(xml_GetNodeElement(passedNode), nodeName, &exc);
	    limit = gdome_nl_length(listShapes, &exc);

	    for(i=0; i<limit && tempNode == NULL; ++i)
		tempNode = xml_SetNode(gdome_nl_item(listShapes, i, &exc));

	    gdome_nl_unref(listShapes, &exc);
	    xml_SetAttributeC(tempNode, "coordIndex", ajStrGetPtr(index));
	    xml_UnrefNode(&tempNode);	    
	}
	else
	    xml_SetAttributeC(node, "coordIndex", ajStrGetPtr(index));
    }
    
    return;
}




/* @funcstatic xml_GetPoints ****************************************
**
** Local Method
** Gets the list of points for an Indexed Line Set
**
** @param [u] passedNode [AjPXmlNode] node to get the points of
**
** @return [AjPStr] value of points
** @@
*********************************************************************/

static AjPStr xml_GetPoints(AjPXmlNode passedNode)
{
    GdomeDOMString *attributeValue;
    GdomeException exc;
    AjPStr temp = NULL;
    GdomeNodeList *listCoordinate = NULL;
    GdomeNodeList *listShapes     = NULL;
    AjPXmlNode coordinateNode     = NULL;
    int i;
    GdomeDOMString *nodeName = NULL;
    AjBool proto;
    AjPStr ajAttributeValue  = NULL;
    AjPXmlNode tempNode      = NULL;
    AjPXmlNode node          = NULL;
    ajint limit;
    
    proto = ajFalse;

    nodeName = gdome_el_tagName(xml_GetNodeElement(passedNode), &exc);

    if(ajCharMatchC("Shape", nodeName->str))
    {
	gdome_str_unref(nodeName);
	nodeName = gdome_str_mkref("IndexedLineSet");
	listShapes = gdome_el_getElementsByTagName
	    (xml_GetNodeElement(passedNode), nodeName, &exc);
	limit = gdome_nl_length(listShapes, &exc);

	for(i=0; i<limit && node == NULL; ++i)
	    node = xml_SetNode(gdome_nl_item(listShapes, i, &exc));

	gdome_nl_unref(listShapes, &exc);

	if(node == NULL)
	{
	    gdome_str_unref(nodeName);
	    nodeName = gdome_str_mkref("IndexedFaceSet");
	    listShapes = gdome_el_getElementsByTagName
		(xml_GetNodeElement(passedNode), nodeName, &exc);
	    limit = gdome_nl_length(listShapes, &exc);

	    for(i=0; i<limit && node == NULL; ++i)
		node = xml_SetNode(gdome_nl_item(listShapes, i, &exc));

	    gdome_nl_unref(listShapes, &exc);
	}
    }
    else
	node = passedNode;
    
    gdome_str_unref(nodeName);
    nodeName = gdome_el_tagName(xml_GetNodeElement(passedNode), &exc);
    
    if(ajCharMatchC("ProtoInstance", nodeName->str))
    {
	ajAttributeValue = xml_GetAttributeC(node, "name");

	if(ajStrMatchC(ajAttributeValue, "Graph"))
	    proto = ajTrue;

	ajStrDel(&ajAttributeValue);
    }

    gdome_str_unref(nodeName);

    if(proto)
    {
	nodeName = gdome_str_mkref("fieldValue");
	listShapes = gdome_el_getElementsByTagName
	    (xml_GetNodeElement(node), nodeName, &exc);
	limit = gdome_nl_length(listShapes, &exc);

	for(i=0; i<limit && temp == NULL; ++i)
	{
	    tempNode = xml_SetNode(gdome_nl_item(listShapes, i, &exc));
	    ajAttributeValue = xml_GetAttributeC(tempNode, 
						 "name");

	    if(ajStrMatchC(ajAttributeValue, "Graph.points"))
		temp = xml_GetAttributeC(tempNode, "value");

	    ajStrDel(&ajAttributeValue);
	    xml_UnrefNode(&tempNode);
	}
	gdome_str_unref(nodeName);
	gdome_nl_unref(listShapes, &exc);
    }
    else
    {
	nodeName = gdome_n_nodeName(xml_GetNode(node), &exc);
    
	if(!(ajCharMatchC(nodeName->str, "IndexedLineSet")
	     || ajCharMatchC(nodeName->str, "IndexedFaceSet")
	     || ajCharMatchC(nodeName->str, "IndexedPointSet")))
	{
	    ajDebug("Exception: cannot get points from a node "
		    "that is not an IndexedSet, this is a %s", nodeName->str);

	    return ajStrNewC("");
	}

 	gdome_str_unref(nodeName);
	nodeName = gdome_str_mkref("Coordinate");
	listCoordinate = gdome_el_getElementsByTagName
	    (xml_GetNodeElement(node), nodeName, &exc);

	if(gdome_nl_length(listCoordinate, &exc)!=0)
	    coordinateNode = xml_SetNode( gdome_nl_item 
					 (listCoordinate, 0, &exc));

	if(xml_GetNode(coordinateNode) == NULL)
	{
	    ajDebug("Exception: IndexedLineSet does not have points");
	    return ajStrNewC("");
	}	
    
	gdome_str_unref(nodeName);
	nodeName = gdome_str_mkref("point");
	attributeValue = gdome_el_getAttribute 
	    (xml_GetNodeElement(coordinateNode), nodeName, &exc);

	xml_UnrefNode(&coordinateNode);

	if(listCoordinate != NULL)
	    gdome_nl_unref(listCoordinate, &exc);
    
	/* Do I need to unref the attributeValue? I guess yes so I do not 
	   use the line below */
	/*
	   return ajStrNewC(attributeValue->str); 
	   */
	temp = ajStrNewC(attributeValue->str);
	gdome_str_unref(nodeName);
	gdome_str_unref(attributeValue);
    }
    
    return temp;
}




/* @funcstatic xml_SetPoints ****************************************
**
** Local Method
** Sets the list of points for an Indexed Line Set
**
** @param [u] passedNode [AjPXmlNode] node to set the points of
** @param [r] points [const AjPStr] value of points
**
** @return [AjBool] ajTrue if points set correctly
** @@
*********************************************************************/

static AjBool xml_SetPoints(AjPXmlNode passedNode, const AjPStr points)
{
    GdomeException exc;
    GdomeNodeList *listCoordinate = NULL;
    AjPXmlNode coordinateNode     = NULL;
    int i;
    GdomeDOMString *nodeName2     = NULL;
    GdomeDOMString *nodeName      = NULL;
    
    GdomeNodeList *listShapes     = NULL;
    AjBool proto;
    AjPStr ajAttributeValue = NULL;
    AjPXmlNode tempNode     = NULL;
    AjPXmlNode returnNode   = NULL;
    AjPXmlNode node         = NULL;
    ajint limit;
    
    proto = ajFalse;
    
    nodeName = gdome_el_tagName(xml_GetNodeElement(passedNode), &exc);

    if(ajCharMatchC("Shape", nodeName->str))
    {
	nodeName = gdome_str_mkref("IndexedLineSet");
	listShapes = gdome_el_getElementsByTagName
	    (xml_GetNodeElement(passedNode), nodeName, &exc);
	limit = gdome_nl_length(listShapes, &exc);

	for(i=0; i<limit && node == NULL; ++i)
	    node = xml_SetNode(gdome_nl_item(listShapes, i, &exc));

	gdome_nl_unref(listShapes, &exc);

	if(node == NULL)
	{
	    gdome_str_unref(nodeName);
	    nodeName = gdome_str_mkref("IndexedFaceSet");
	    listShapes = gdome_el_getElementsByTagName
		(xml_GetNodeElement(passedNode), nodeName, &exc);
	    limit = gdome_nl_length(listShapes, &exc);

	    for(i=0; i<limit && node == NULL; ++i)
		node = xml_SetNode(gdome_nl_item(listShapes, i, &exc));

	    gdome_nl_unref(listShapes, &exc);
	}
    }
    else
	node = passedNode;
    
    gdome_str_unref(nodeName);
    nodeName = gdome_el_tagName(xml_GetNodeElement(passedNode), &exc);

    if(ajCharMatchC("ProtoInstance", nodeName->str))
    {
	ajAttributeValue = xml_GetAttributeC(node, "name");

	if(ajStrMatchC(ajAttributeValue, "Graph"))
	    proto = ajTrue;

	ajStrDel(&ajAttributeValue);
    }
        
    gdome_str_unref(nodeName);

    if(proto)
    {
	nodeName = gdome_str_mkref("fieldValue");
	listShapes = gdome_el_getElementsByTagName
	    (xml_GetNodeElement(node), nodeName, &exc);
	limit = gdome_nl_length(listShapes, &exc);

	for(i=0; i<limit && returnNode == NULL; ++i)
	{
	    tempNode = xml_SetNode(gdome_nl_item(listShapes, i, &exc));
	    ajAttributeValue = xml_GetAttributeC(tempNode, 
						 "name");

	    if(ajStrMatchC(ajAttributeValue, "Graph.points"))
		xml_SetAttributeC(tempNode, "value", ajStrGetPtr(points));

	    ajStrDel(&ajAttributeValue);
	    xml_UnrefNode(&tempNode);
	}

	gdome_nl_unref(listShapes, &exc);
	gdome_str_unref(nodeName);
    }
    else
    {
	nodeName = gdome_n_nodeName(xml_GetNode(node), &exc);
    
	if(!(ajCharMatchC(nodeName->str, "IndexedLineSet")
	     || ajCharMatchC(nodeName->str, "IndexedFaceSet")
	     || ajCharMatchC(nodeName->str, "IndexedPointSet")))
	{
	    ajDebug("Exception: cannot set points from a node "
		   "that is not an IndexedSet, this is a %s", nodeName->str);

	    return ajFalse;
	}

	gdome_str_unref(nodeName);
	nodeName = gdome_str_mkref("Coordinate");
	listCoordinate = gdome_el_getElementsByTagName
	    ((xml_GetNodeElement(node)), nodeName, &exc);

	if(gdome_nl_length(listCoordinate, &exc)!=0)
	    coordinateNode = xml_SetNode( gdome_nl_item 
					 (listCoordinate, 0, &exc));

	gdome_nl_unref(listCoordinate, &exc);

	if(xml_GetNode(coordinateNode) == NULL)
	{
	    ajDebug("Exception: "
		   "IndexedLineSet does not have points");

	    return ajFalse;
	}	
    
	gdome_str_unref(nodeName);
	nodeName = gdome_str_mkref_dup("point");
	nodeName2 = gdome_str_mkref_dup(ajStrGetPtr(points));
	gdome_el_setAttribute(xml_GetNodeElement(coordinateNode), 
			      nodeName, nodeName2, &exc);

	xml_UnrefNode(&coordinateNode);
	gdome_str_unref(nodeName);
	gdome_str_unref(nodeName2);

    }
    
    return ajTrue;
}




/* @funcstatic xml_MakeNewNode **************************************
**
** Local Method
** Makes a new DOM node and appends it to the parent node
**
** @param [w] file [AjPGraphXml] the file to add the node to
** @param [r] name [const AjPStr] name of node
** @param [u] parent [AjPXmlNode] parent node to add to
**
** @return [AjPXmlNode] new node
** @@
*********************************************************************/

static AjPXmlNode xml_MakeNewNode(AjPGraphXml file, const AjPStr name, 
				  AjPXmlNode parent)
{
    return xml_MakeNewNodeC(file, ajStrGetPtr(name), parent);
}




/* @funcstatic xml_MakeNewNodeC **************************************
**
** Local Method
** Makes a new DOM node and appends it to the parent node, from char*
**
** @param [w] file [AjPGraphXml] the file to add the node to
** @param [r] name [const char*] name of node
** @param [u] parent [AjPXmlNode] parent node to add to
**
** @return [AjPXmlNode] new node
** @@
*********************************************************************/

static AjPXmlNode xml_MakeNewNodeC(AjPGraphXml file, const char *name, 
				  AjPXmlNode parent)
{
    GdomeException exc;
    GdomeNode *parentNode; 
    GdomeNode *elNode; 
    GdomeNode *anotherElNode; 
    AjPXmlNode el;

    GdomeDOMString *nodeName = NULL;

    nodeName = gdome_str_mkref_dup(name);
    elNode = (GdomeNode *) gdome_doc_createElement (file->doc, nodeName,
						    &exc);
    el = xml_SetNode(elNode);

    parentNode = xml_GetNode(parent);
    anotherElNode = gdome_n_appendChild (parentNode, elNode, &exc);
    /*  gdome_n_appendChild (parentNode, elNode, &exc); hugh */

    gdome_str_unref (nodeName);
    gdome_n_unref(anotherElNode, &exc);

    return el;
}




/* @funcstatic xml_PresentColourAsString **********************************
**
** Local Method
** returns present Colour As String
**
** @param [r] file [const AjPGraphXml] the file with the present colour
**
** @return [AjPStr] the colour
** @@
*********************************************************************/
static  AjPStr xml_PresentColourAsString(const AjPGraphXml file)
{
    AjPStr colour = NULL;
    AjPStr temp   = NULL;
    int i;

    colour = ajStrNew();
    temp   = ajStrNew();
    
    for(i=0; i<3; ++i)
    {
	ajStrAssignClear(&temp);
	xml_StrFromDouble(&temp, file->colour[i]);
	ajStrAppendS(&colour, temp);

	if(i<2)
	    ajStrAppendC(&colour, " ");
    }

    ajStrDel(&temp);
    
    return colour;
}




/* @funcstatic xml_FileNeedsProtoDeclareC **********************************
**
** Local Method
** returns whether this file needs this type of proto Declaration
**
** @param [r] file [const AjPGraphXml] the file to check
** @param [r] protoName [const char *] the proto name
**
** @return [AjBool] true if this file needs this type of proto Declaration
** @@
*********************************************************************/

static AjBool xml_FileNeedsProtoDeclareC(const AjPGraphXml file,
					 const char *protoName)
{
    GdomeException exc;
    AjPXmlNode presentNode;
    GdomeNodeList* listProtos;
    int i;
    ajint limit;
    
    AjPStr presentProtoName = NULL; 
    GdomeDOMString *nodeName = NULL;

    presentNode = xml_GetCurrentGraphic(file);

    nodeName = gdome_str_mkref("ProtoDeclare");
    listProtos = gdome_el_getElementsByTagName
	((xml_GetNodeElement(xml_GetCurrentScene(file))), nodeName, 
	 &exc);
    limit = gdome_nl_length(listProtos, &exc);

    for(i=0; i<limit; ++i)
    {
	presentNode = xml_SetNode( gdome_nl_item (listProtos, i, 
						   &exc));
	presentProtoName = xml_GetAttributeC(presentNode, 
					     "name");
	if(ajStrMatchC(presentProtoName, protoName))
	{
	    gdome_nl_unref(listProtos, &exc);
	    gdome_str_unref(nodeName);
	    ajStrDel(&presentProtoName);
	    xml_UnrefNode(&presentNode);

	    return ajFalse;
	}

	xml_UnrefNode(&presentNode);
	ajStrDel(&presentProtoName);
    }

    /*  Both of these cause a crash. hugh */
    /*
       xml_UnrefNode(&presentNode);
       gdome_n_unref(presentNode, &exc); 
       */
    gdome_nl_unref(listProtos, &exc);
    gdome_str_unref(nodeName);
    /* presentNode and presentProtoName unrefed above  */

    return ajTrue;
}




/* @funcstatic xml_FileNeedsProtoDeclare **********************************
**
** Local Method
** returns whether this file needs this type of proto Declaration
**
** @param [r] file [const AjPGraphXml] the file to check
** @param [r] protoName [const AjPStr] the proto name
**
** @return [AjBool] true if this file needs this type of proto Declaration
** @@
*********************************************************************/

static AjBool xml_FileNeedsProtoDeclare(const AjPGraphXml file,
					const AjPStr protoName)
{
    return xml_FileNeedsProtoDeclareC(file,ajStrGetPtr(protoName));
}




/* @funcstatic xml_IsShapeThisColour *************************************
**
** Local Method
** returns whether this shape is the present colour
**
** @param [u] file [AjPGraphXml] the file to check
** @param [u] shape [AjPXmlNode] the shape to check
**
** @return [AjBool] true if this shape is current colour
** @@
*********************************************************************/

static AjBool xml_IsShapeThisColour(AjPGraphXml file, AjPXmlNode shape)
{
    GdomeException exc;
    GdomeNodeList *listAppearance = NULL;
    GdomeNodeList *listMaterial   = NULL;
    GdomeDOMString *nodeName      = NULL;
    AjPXmlNode elMaterial = NULL;
    AjPXmlNode tempNode   = NULL;
    AjPStr colour         = NULL;
    AjPStr presentColour  = NULL;
    AjPStr attributeValue = NULL;
    AjBool returnValue = ajFalse;

    ajint i;
    ajint limit;
 
    if(xml_PresentGraphicTypeIsC(file, "Graph") 
       && xml_GetNode(shape) == 
       xml_GetNode(xml_GetCurrentGraphic(file)))
    {
	nodeName = gdome_str_mkref("fieldValue");
	listAppearance = gdome_el_getElementsByTagName
	    ((xml_GetNodeElement(shape)), 
	     nodeName, &exc);
	limit = gdome_nl_length(listAppearance, &exc);

	for(i=0; i<limit && presentColour == NULL; ++i)
	{
	    tempNode = xml_SetNode(gdome_nl_item(listAppearance, i, 
						 &exc));
	    attributeValue = xml_GetAttributeC(tempNode, "name");

	    if(ajStrMatchC(attributeValue, "Graph.colour"))
		colour = xml_GetAttributeC(tempNode, 
					   "value");

	    ajXmlNodeDel(&tempNode);
	    ajStrDel(&attributeValue);
	}

	gdome_nl_unref(listAppearance, &exc);
	gdome_str_unref(nodeName);

	if(colour == NULL)
	{
	    tempNode = xml_MakeNewNodeC(file, "fieldValue", shape);
		
	    xml_SetAttributeC(tempNode, "name", "Graph.colour");

	    ajStrDel(&attributeValue);
	    attributeValue = xml_PresentColourAsString(file);
	    xml_SetAttributeC(tempNode, "value", 
			     ajStrGetPtr(attributeValue));

	    ajStrDel(&attributeValue);
	    ajXmlNodeDel(&tempNode);

	    return ajTrue;
	}

	presentColour = xml_PresentColourAsString(file);
    }
    else
    {
	/* needs error checking */
	nodeName = gdome_str_mkref("Appearance");
	/*	listAppearance = gdome_el_getElementsByTagName
		(((GdomeElement *) gdome_nl_item(listAppearance, 0, &exc)), 
		nodeName, &exc);*/
	listAppearance = gdome_el_getElementsByTagName
	    (xml_GetNodeElement(shape), nodeName, &exc);
	gdome_str_unref(nodeName);

	nodeName = gdome_str_mkref("Material");
	listMaterial = gdome_el_getElementsByTagName
	    (((GdomeElement *) gdome_nl_item(listAppearance, 0, &exc)), 
	     nodeName, &exc);

	elMaterial = xml_SetNode( gdome_nl_item(listMaterial, 0, &exc));

	colour = xml_GetAttributeC(elMaterial, "diffuseColor");

	presentColour = xml_PresentColourAsString(file);

	gdome_nl_unref(listAppearance, &exc);
	gdome_nl_unref(listMaterial, &exc);

	xml_UnrefNode(&elMaterial);

	ajStrDel(&colour);
	ajStrDel(&presentColour);

	gdome_str_unref(nodeName);
    }
    
    returnValue = ajStrMatchS(colour, presentColour);


    return returnValue;
}




/* @funcstatic xml_MakeNewShapeNodeC *********************************
**
** Local Method
** returns makes a New Shape Node
**
** @param [u] file [AjPGraphXml] the file to find the node in
** @param [u] parentNode [AjPXmlNode] parent node to add to
** @param [r] nameReqd [const char *] name of node
**
** @return [AjPXmlNode] new node
** @@
*********************************************************************/

static AjPXmlNode xml_MakeNewShapeNodeC(AjPGraphXml file, 
					AjPXmlNode parentNode, 
					const char *nameReqd)
{
    AjPXmlNode returnNode;
    AjPXmlNode shape;
    AjPXmlNode Appearance;
    AjPXmlNode Material;
    AjPStr     colour = NULL;

    shape = xml_MakeNewNodeC(file, "Shape", parentNode); 

    returnNode = xml_MakeNewNodeC(file, nameReqd, shape);

    Appearance = xml_MakeNewNodeC(file, "Appearance", shape);
    Material = xml_MakeNewNodeC(file, "Material", Appearance);

    colour = xml_PresentColourAsString(file);
    xml_SetAttributeC(Material, "diffuseColor", ajStrGetPtr(colour));

    xml_UnrefNode(&shape);
    xml_UnrefNode(&Appearance);
    xml_UnrefNode(&Material);
    ajStrDel(&colour);
        
    return returnNode;
}




/* @funcstatic xml_MakeNewShapeNode *********************************
**
** Local Method
** returns makes a New Shape Node
**
** @param [u] file [AjPGraphXml] the file to find the node in
** @param [u] parentNode [AjPXmlNode] parent node to add to
** @param [r] nameReqd [const AjPStr] name of node
**
** @return [AjPXmlNode] new node
** @@
*********************************************************************/

static AjPXmlNode xml_MakeNewShapeNode(AjPGraphXml file, 
				       AjPXmlNode parentNode, 
				       const AjPStr nameReqd)
{
    return xml_MakeNewShapeNodeC(file,parentNode,ajStrGetPtr(nameReqd));
}




/* @funcstatic xml_AddArc *************************************************
**
** adds an arc.  Assumes the arc is 0 <= extent < pi
**
** @param [u] file [AjPGraphXml] the file to add the max min values to
** @param [r] xCentre [double] the x value of the centre of the circle
** @param [r] yCentre [double] the y value of the centre of the circle
** @param [r] startAngle [double] the startAngle of the arc
** @param [r] endAngle [double] the endAngle of the arc
** @param [r] radius [double] the radius of the circle
**
** @return [void]
** @@
*********************************************************************/

static void xml_AddArc(AjPGraphXml file, double xCentre, double yCentre, 
		       double startAngle, double endAngle, double radius)
{
    double xStart;
    double yStart;
    double xMiddle;
    double yMiddle;
    double xEnd;
    double yEnd;
    double middleWeight;

    double anglularExtent;
    double xChordCentre;
    double yChordCentre;
    double chordLength;

    double vectorAngle;
    double vectorLength;
    double centreToChordCentre;
    double chordCentreToMiddle;

    AjPStr temp          = NULL;
    AjPStr controlPoints = NULL;
    AjPStr weights       = NULL;
    AjPXmlNode nurbsNode;

    temp = ajStrNew();
    controlPoints = ajStrNew();
    weights = ajStrNew();

    xStart = sin(startAngle) * radius + xCentre;
    yStart = cos(startAngle) * radius + yCentre;
    xEnd = sin(endAngle) * radius + xCentre;
    yEnd = cos(endAngle) * radius + yCentre;
    
    anglularExtent = endAngle - startAngle;
    chordLength = pow(pow((xEnd - xStart), 2) + pow((yEnd - yStart), 
						    2), 0.5);
    xChordCentre = (xEnd - xStart) / 2 + xStart;
    yChordCentre = (yEnd - yStart) / 2 + yStart;
    
    vectorAngle = startAngle + (anglularExtent / 2);
    centreToChordCentre = pow((pow(radius, 2) - pow((chordLength / 2),
						    2)), 0.5);
    chordCentreToMiddle = (chordLength / 2) * tan(anglularExtent / 2);
    vectorLength = centreToChordCentre + chordCentreToMiddle;

    xMiddle = xCentre + (vectorLength * sin(vectorAngle));
    yMiddle = yCentre + (vectorLength * cos(vectorAngle));
    middleWeight = cos(anglularExtent / 2);

    xml_AddACoord(xStart, yStart, ajFalse, &controlPoints, &temp);
    xml_AddACoord(xMiddle, yMiddle, ajFalse, &controlPoints, &temp);
    xml_AddACoord(xEnd, yEnd, ajFalse, &controlPoints, &temp);

    ajStrAppendC(&weights, "1 ");
    ajStrAssignClear(&temp);
    xml_StrFromDouble(&temp, middleWeight);
    ajStrAppendC(&weights, ajStrGetPtr(temp));
    ajStrAppendC(&weights, " 1");
    
    nurbsNode = xml_MakeNewShapeNodeC(file, 
				      xml_GetCurrentGraphic(file), 
				      "NurbsCurve");
    xml_SetAttributeC(nurbsNode, "knot", "0,0,0,1,1,1");
    xml_SetAttributeC(nurbsNode, "order", "3");
    xml_SetAttributeC(nurbsNode, "controlPoint", ajStrGetPtr(controlPoints));
    xml_SetAttributeC(nurbsNode, "weight", ajStrGetPtr(weights));
    
    ajStrDel(&temp);
    ajStrDel(&controlPoints);
    ajStrDel(&weights);

    return;
}




/* @funcstatic xml_GetNodeTypeMakeIfNot *****************************
**
** Local Method
** returns first? node of this type, Makes a new one If Not
**
** @param [u] file [AjPGraphXml] the file to find the node in
** @param [r] nameReqd [const AjPStr] name of node
**
** @return [AjPXmlNode] found or new node
** @@
*********************************************************************/
static AjPXmlNode xml_GetNodeTypeMakeIfNot(AjPGraphXml file,
					   const AjPStr nameReqd)
{
    GdomeException exc;
    AjPXmlNode returnNode  = NULL;
    AjPXmlNode returnNode2 = NULL;
    AjPXmlNode coordinateNode;
    AjPXmlNode tempNode;
    AjPXmlNode shapeNodeParent       = NULL;
    GdomeNodeList *listShapes        = NULL;
    GdomeNodeList *listGeometrys     = NULL;
    GdomeNodeList *listIndexLineSets = NULL;
    int i;
    int j;
    AjPStr attributeValue    = NULL;
    GdomeDOMString *nodeName = NULL;
    AjBool hasCoord;
    AjBool hasCoordIndex;
    AjPTable colourTable;
    ajint limit;
    ajint limit2;
    
    /*    returnNode = (AjPXmlNode) ajTableFetch(file->nodeTypes, nameReqd); */


    colourTable = (AjPTable) ajTableFetch(file->nodeTypes, nameReqd);

    if(colourTable != NULL)
	returnNode = (AjPXmlNode) ajTableFetch(colourTable, 
					     xml_PresentColourAsString(file));
    


    /*   if(returnNode != NULL && xml_IsShapeThisColour(file, returnNode)) */
    if(returnNode != NULL)
    {
	GdomeException exc;

	/*	if(xml_IsShapeThisColour(file, returnNode)) */
	{
	    returnNode2 = xml_SetNode(xml_GetNode(returnNode));
	    gdome_n_ref(xml_GetNode(returnNode), &exc);

	    return returnNode2;
	}
    }
  
    hasCoord = ajFalse;
    hasCoordIndex = ajFalse;


    if(xml_PresentGraphicTypeIsC(file, "Graph") 
       && ajStrMatchC(nameReqd, "IndexedLineSet")
       && xml_IsShapeThisColour(file, xml_GetCurrentGraphic(file)))
    {
	nodeName = gdome_str_mkref("fieldValue");
	listShapes = gdome_el_getElementsByTagName
	    ((xml_GetNodeElement(xml_GetCurrentGraphic(file))), 
	     nodeName, &exc);
	limit = gdome_nl_length(listShapes, &exc);

	for(i=0; i<limit && returnNode == NULL; ++i)
	{
	    tempNode = xml_SetNode(gdome_nl_item(listShapes, i, &exc));
	  
	    attributeValue = xml_GetAttributeC(tempNode, "fieldName");

	    if(ajStrMatchC(attributeValue, "Graph.points"))
		hasCoord = ajTrue;

	    if(ajStrMatchC(attributeValue, "Graph.index"))
		hasCoordIndex = ajTrue;

	    ajStrDel(&attributeValue);
	    xml_UnrefNode(&tempNode);
	}

	if(!hasCoord)
	{
	    tempNode = xml_MakeNewNodeC(file, "fieldValue", 
				       xml_GetCurrentGraphic(file));
	    xml_SetAttributeC(tempNode, "name", "Graph.points");
	    xml_SetAttributeC(tempNode, "value", "");

	    xml_UnrefNode(&tempNode);
	}

	if(!hasCoordIndex)
	{
	    tempNode = xml_MakeNewNodeC(file, "fieldValue", 
				       xml_GetCurrentGraphic(file));
	    xml_SetAttributeC(tempNode, "name", "Graph.index");
	    xml_SetAttributeC(tempNode, "value", "");

	    xml_UnrefNode(&tempNode);
	}

	gdome_str_unref(nodeName);
	gdome_nl_unref(listShapes, &exc);

	returnNode2 = xml_SetNode
	    (xml_GetNode(xml_GetCurrentGraphic(file)));
	gdome_n_ref(xml_GetNode(xml_GetCurrentGraphic(file)), &exc);


	colourTable = (AjPTable) ajTableFetch(file->nodeTypes, nameReqd);

	if(colourTable == NULL)
	{
	    colourTable = ajTablestrNewLen(1);
    
	    ajTablePut(file->nodeTypes, (void *) ajStrNewS(nameReqd),
		       (void *)colourTable);
	}

	ajTablePut(colourTable, 
		   (void *) xml_PresentColourAsString(file),
		   (void *)returnNode2);

	return(xml_GetCurrentGraphic(file));
      
    }
    else 
    {
	returnNode = NULL;

	if(xml_PresentGraphicTypeIsC(file, "Graph"))
	{
	    shapeNodeParent = NULL;
	    nodeName = gdome_str_mkref("fieldValue");
	    listShapes = gdome_el_getElementsByTagName
		((xml_GetNodeElement(xml_GetCurrentGraphic(file))), 
		 nodeName, &exc);
	    limit = gdome_nl_length(listShapes, &exc);

	    for(i=0; i<limit && returnNode == NULL; ++i)      
	    {
		nodeName = gdome_str_mkref("name");
		attributeValue = ajStrNewC(gdome_el_getAttribute
		   (((GdomeElement*)gdome_nl_item(listShapes, i, &exc)),
					    nodeName, 
					    &exc)->str);

		if(ajStrMatchC(attributeValue, "children"))
		    shapeNodeParent = xml_SetNode(gdome_nl_item
						  (listShapes, i, &exc));
	    }

	    if(shapeNodeParent == NULL)
	    {
		shapeNodeParent = xml_MakeNewNodeC(file, "fieldValue", 
						  xml_GetCurrentGraphic(file));
		xml_SetAttributeC(shapeNodeParent, "name", "children");
		 
	    }
	}
	else
	    shapeNodeParent = xml_GetCurrentGraphic(file);
     
	nodeName = gdome_str_mkref("Shape");
	listShapes = gdome_el_getElementsByTagName
	    ((xml_GetNodeElement(shapeNodeParent)), nodeName, &exc);
	limit = gdome_nl_length(listShapes, &exc);

	for(i=0; i<limit && returnNode == NULL; ++i)
	{
	    tempNode = xml_SetNode(gdome_nl_item(listShapes, i, &exc));

	    if(xml_IsShapeThisColour(file, tempNode))
	    {
		gdome_str_unref(nodeName);
		nodeName = gdome_str_mkref("geometry");
		listGeometrys = gdome_el_getElementsByTagName
		    ((GdomeElement *)gdome_nl_item(listShapes, i, &exc), 
		     nodeName, &exc);
		limit2 = gdome_nl_length(listGeometrys, &exc);

		for(j=0; j<limit2 && returnNode == NULL; ++j)
		{
		    gdome_str_unref(nodeName);
		    nodeName = gdome_str_mkref(ajStrGetPtr(nameReqd));
		    listIndexLineSets  = gdome_el_getElementsByTagName
			((GdomeElement *)gdome_nl_item(listGeometrys, j, 
						       &exc), nodeName, &exc);
		    if(gdome_nl_length(listIndexLineSets, &exc)!=0)
			returnNode = xml_SetNode( gdome_nl_item 
						 (listIndexLineSets, 0, 
						  &exc));

		    gdome_nl_unref(listIndexLineSets, &exc);
		}

		gdome_nl_unref(listGeometrys, &exc);
	    }

	    xml_UnrefNode(&tempNode);
	}

	if(returnNode == NULL)
	{
	    returnNode = xml_MakeNewShapeNode(file, shapeNodeParent, 
					      nameReqd);
	    if(ajStrMatchC(nameReqd, "IndexedLineSet")
	       || ajStrMatchC(nameReqd, "IndexedFaceSet")
	       || ajStrMatchC(nameReqd, "IndexedPointSet"))
	    {
		coordinateNode = xml_MakeNewNodeC(file, "Coordinate", 
						 returnNode);

		xml_UnrefNode(&coordinateNode);
	    }
	}
	 
	gdome_nl_unref(listShapes, &exc);
	gdome_str_unref(nodeName);
	/*	if(shapeNodeParent != xml_GetCurrentGraphic(file))
		{
		xml_UnrefNode(&shapeNodeParent);
		} */

	/* really not sure about this hugh should be returning a shape not
           an indexed line set */
	/*	ajXmlNodeDel(&returnNode);
		returnNode = shapeNodeParent; */
	
	
	returnNode2 = xml_GetParent(returnNode);
	gdome_n_ref(xml_GetNode(returnNode2), &exc);


	colourTable = (AjPTable) ajTableFetch(file->nodeTypes, nameReqd);

	if(colourTable == NULL)
	{
	    colourTable = ajTablestrNewLen(1);
	    ajTablePut(file->nodeTypes, (void *) ajStrNewS(nameReqd),
		       (void *)colourTable);
	}

	ajTablePut(colourTable, 
		   (void *) xml_PresentColourAsString(file),
		   (void *)returnNode2);

	/* listShapes and nodeName unrefed just above */
	/* tempNode, unrefed as required */

	return returnNode;

    }
      
      
 /*   returnNode2 = xml_SetNode(xml_GetNode(returnNode));
    gdome_n_ref(xml_GetNode(returnNode), &exc);


    colourTable = (AjPTable) ajTableFetch(file->nodeTypes, nameReqd);
    if(colourTable == NULL)
    {
	colourTable = ajTablestrNewLen(1);
	ajTablePut(file->nodeTypes, 
		   (const void *) ajStrNewS(nameReqd),
		   (void *)colourTable);
    }
    ajTablePut(colourTable, 
	       (const void *) xml_PresentColourAsString(file),
	       (void *)returnNode2); */

    /* listShapes and nodeName unrefed just above */
    /* tempNode, unrefed as required */

    /*	ajXmlWriteStdout(file); */

 /*   return returnNode; */
}




/* @funcstatic xml_GetNodeTypeMakeIfNotC *****************************
**
** Local Method
** returns first? node of this type, Makes a new one If Not
**
** @param [u] file [AjPGraphXml] the file to find the node in
** @param [r] nameReqd [const char *] name of node
**
** @return [AjPXmlNode] found or new node
** @@
*********************************************************************/

static AjPXmlNode xml_GetNodeTypeMakeIfNotC(AjPGraphXml file,
					    const char *nameReqd)
{
    AjPStr str     = NULL;
    AjPXmlNode ret = NULL;

    str = ajStrNewC(nameReqd);
    ret =  xml_GetNodeTypeMakeIfNot(file, str);
    ajStrDel(&str);

    return ret;
}




/* @funcstatic xml_AddACoord ***************************************
**
** Local Method
** adds a coord to string, incremets the index in another
**
** @param [r] x [double]  x coordinate
** @param [r] y [double]  y coordinate
** @param [r] joined [AjBool] conect this to last line?
** @param [w] coord [AjPStr*] string to add coord to
** @param [w] index [AjPStr*] string to add index to
**
*********************************************************************/

static void xml_AddACoord(double x, double y, AjBool joined, AjPStr* coord, 
			  AjPStr* index)
{
    AjPStr temp = NULL;
    int lastIndex;

    temp = ajStrNew();

    if(ajStrCmpC((*coord), "") != 0)
	ajStrAppendC(coord,  ", ");

    xml_StrFromDouble(&temp, x);
    ajStrAppendC(coord, ajStrGetPtr(temp));
    ajStrAppendC(coord, " ");

    ajStrAssignClear(&temp);
    xml_StrFromDouble(&temp, y);
    ajStrAppendC(coord, ajStrGetPtr(temp));
    ajStrAppendC(coord, " ");

    ajStrAssignClear(&temp);
    ajStrFromInt(&temp, 0);
    ajStrAppendC(coord, ajStrGetPtr(temp));
    
    if(ajStrCmpC((*index), "") != 0)
    {
	lastIndex = xml_GetLastInt(*index);
	ajStrAppendC(index,  " ");

	if(!joined)
	    ajStrAppendC(index, "-1 ");
    }
    else
	lastIndex = -1;

    ajStrAssignClear(&temp);
    ajStrFromInt(&temp, (lastIndex+1));
    ajStrAppendC(index, ajStrGetPtr(temp));
      
    ajStrDel(&temp);

    return;
}




/* @funcstatic xml_GetLastInt ***************************************
**
** gets Last Int of a string
**
** @param [r] str [const AjPStr] string to find int in
**
** @return [int] the last interger of a string, 0 and an ecxeption if 
** not an int
** @@
*********************************************************************/

static int xml_GetLastInt(const AjPStr str)
{
    int count;
    int i;
    int value;
    const AjPStr token = NULL;

    count = ajStrParseCountC(str, " ");
    token = ajStrParseWhite(str);

    for(i = count - 1; i >= 1; --i)
	token = ajStrParseWhite(NULL);


    if(token != NULL)
    {    
	if(ajStrIsInt (token))
	    ajStrToInt(token, &value);
	else
	    ajDebug("Exception: Last token not Int\n");
    }
    else
	ajDebug("Exception: string passed empty\n");


    return value;
}




/* @funcstatic xml_GetLastDouble ************************************
**
** gets Last Double of a string
**
** @param [r] str [const AjPStr] string to find double in
**
** @return [double] the last double of a string, 0 and an exception if 
** not a double
** @@
*********************************************************************/
static double xml_GetLastDouble(const AjPStr str)
{
    int count;
    int i;
    double value;
    const AjPStr token = NULL;

    count = ajStrParseCountC(str, " ");
    token = ajStrParseWhite(str);

    for(i = count - 1; i >= 1; --i)
	token = ajStrParseWhite(NULL);

    if(token != NULL)
    {    
	if(ajStrIsDouble (token))
	    ajStrToDouble(token, &value);
	else
	    ajDebug("Exception: Last token not Double\n");
    }
    else
	ajDebug("Exception: string passed empty\n");

    return value;
}



/* @funcstatic xml_GetDoubleNo **************************************
**
** returns the double at position index
**
** @param [r] str [const AjPStr] string to find double in
** @param [r] index [int] position of required double
**
** @return [double] the index number double of a string, 
** @@
*********************************************************************/

static double xml_GetDoubleNo(const AjPStr str, int index)
{
    int count;
    int i;
    double value;
    const AjPStr token = NULL;

    count = ajStrParseCountC(str, " ");
    token = ajStrParseWhite(str);

    if(index>count)
	ajDebug("Exception: index higher than no. of tokens\n");


    for(i = 1; i <= index; ++i)
	token = ajStrParseWhite(NULL);

    if(token != NULL)
    {    
	if(ajStrIsDouble (token))
	    ajStrToDouble(token, &value);
	else
	    ajDebug("Exception: %d token not Double\n", 
		   index);
    }
    else
	ajDebug("Exception: string passed empty\n");

    return value;
}




/* @funcstatic xml_CreateNewOutputFile ************************************
**
** Sets up the output file 
**
** @return [AjPGraphXml] a colection of pointers to the file created
** @@
*********************************************************************/

static AjPGraphXml xml_CreateNewOutputFile()
{
    GdomeException exc;
    GdomeDOMString *name;
    GdomeDOMString *publicId;
    GdomeDOMString *systemId;
    AjPGraphXml file = NULL;
    AjPXmlNode rootElement;
    GdomeDocumentType *docType;
  
    file = ajXmlFileNew();

    file->domimpl = gdome_di_mkref ();
    name = gdome_str_mkref_dup ("X3D");
    publicId = gdome_str_mkref_dup 
	("ISO//Web3D//DTD X3D 3.0 Interactive//EN");
    systemId = gdome_str_mkref_dup
	("http://www.web3d.org/specifications/x3d-3.0.dtd");
    docType = gdome_di_createDocumentType(file->domimpl, name, 
					  publicId, systemId, &exc);
  
    /* do I not need to allocate the space? I am sure not */
    ajXmlSetColour(file, 1.0, 1.0, 1.0);

    file->doc = gdome_di_createDocument(file->domimpl, NULL, name, 
					docType, &exc);

    rootElement = xml_SetNode((GdomeNode *) gdome_doc_documentElement 
			      (file->doc, &exc));

    xml_SetAttributeC(rootElement, "profile", "Immersive");

    xml_SetCurrentScene(file, rootElement);   
    xml_SetCurrentGraphic(file, rootElement);
  
    if (xml_GetNode(file->currentGraphic) == NULL) 
	ajDebug("Document.documentElement: NULL\n\tException #%d\n", exc);


    xml_AddCommonBit(file);

    file->nodeTypes = ajTablestrNewLen(1);
    
    gdome_str_unref (name);
    gdome_str_unref (publicId);
    gdome_str_unref (systemId);
    gdome_dt_unref (docType, &exc);

    return file;
}




/* @funcstatic xml_AddGraphProto *************************************
**
** Adds the nodes that are set up the Graph Proto
**
** Could realy be all part of addGraphic, but it is easyer to read
** Private Function
**
** @param [w] file [AjPGraphXml] file to add bit to
**
** @return [void]
** @@
*********************************************************************/

static void xml_AddGraphProto(AjPGraphXml file)
{
    AjPXmlNode protoNode;
    AjPXmlNode fieldNode;
    AjPXmlNode groupNode;
    AjPXmlNode parentNode;
    AjPXmlNode grandParentNode;

    protoNode = xml_MakeNewNodeC(file, "ProtoDeclare",
				 xml_GetCurrentScene(file));


    xml_SetAttributeC(protoNode, "name", "Graph");
    /*  xml_SetAttributeC(protoNode, "EXPORT", "Graphic_Children"); */

    parentNode = xml_MakeNewNodeC(file, "ProtoInterface", protoNode);
    

    fieldNode = xml_MakeNewNodeC(file, "field", parentNode);
    xml_SetAttributeC(fieldNode, "name", "Graph.mainTitle");
    xml_SetAttributeC(fieldNode, "type", "SFString");
    xml_SetAttributeC(fieldNode, "accessType", "initializeOnly");

    xml_UnrefNode(&fieldNode);
    fieldNode = xml_MakeNewNodeC(file, "field", parentNode);
    xml_SetAttributeC(fieldNode, "name", "Graph.xTitle");
    xml_SetAttributeC(fieldNode, "type", "SFString");
    xml_SetAttributeC(fieldNode, "accessType", "initializeOnly");

    xml_UnrefNode(&fieldNode);
    fieldNode = xml_MakeNewNodeC(file, "field", parentNode);
    xml_SetAttributeC(fieldNode, "name", "Graph.yTitle");
    xml_SetAttributeC(fieldNode, "type", "SFString");
    xml_SetAttributeC(fieldNode, "accessType", "initializeOnly");

    xml_UnrefNode(&fieldNode);
    fieldNode = xml_MakeNewNodeC(file, "field", parentNode);
    xml_SetAttributeC(fieldNode, "name", "Graph.colour");
    xml_SetAttributeC(fieldNode, "type", "SFColor");
    xml_SetAttributeC(fieldNode, "accessType", "initializeOnly");

    xml_UnrefNode(&fieldNode);
    fieldNode = xml_MakeNewNodeC(file, "field", parentNode);
    xml_SetAttributeC(fieldNode, "name", "Graph.index");
    xml_SetAttributeC(fieldNode, "type", "MFInt32");
    xml_SetAttributeC(fieldNode, "accessType", "initializeOnly");
    xml_SetAttributeC(fieldNode, "value", "0 1");

    xml_UnrefNode(&fieldNode);
    fieldNode = xml_MakeNewNodeC(file, "field", parentNode);
    xml_SetAttributeC(fieldNode, "name", "Graph.points");
    xml_SetAttributeC(fieldNode, "type", "MFVec3f");
    xml_SetAttributeC(fieldNode, "accessType", "initializeOnly");
    xml_SetAttributeC(fieldNode, "value", "0 0 0, 0 0 0");

    xml_UnrefNode(&fieldNode);
    fieldNode = xml_MakeNewNodeC(file, "field", parentNode);
    xml_SetAttributeC(fieldNode, "name", "children");
    xml_SetAttributeC(fieldNode, "type", "MFNode");
    xml_SetAttributeC(fieldNode, "accessType", "initializeOnly");


    xml_UnrefNode(&parentNode);
    parentNode = xml_MakeNewNodeC(file, "ProtoBody", protoNode);
    groupNode = xml_MakeNewNodeC(file, "Group", parentNode);
    xml_UnrefNode(&protoNode);
    protoNode = xml_MakeNewNodeC(file, "Shape", groupNode);
    xml_UnrefNode(&parentNode);
  
    /* This is the bit 'cos the appearance node is FKD */
    /*
       parentNode = xml_MakeNewNodeC(file, "Appearance",  protoNode);
       grandParentNode = xml_MakeNewNodeC(file, "Material", parentNode);
       xml_UnrefNode(&parentNode);
       parentNode = xml_MakeNewNodeC(file, "IS", grandParentNode);
       xml_UnrefNode(&grandParentNode);
       grandParentNode = xml_MakeNewNodeC(file, "connect", parentNode);
       
       
       xml_SetAttributeC(grandParentNode, "protoField", "Graph.colour");
       xml_SetAttributeC(grandParentNode, "nodeField", "emissiveColor");
       */

    xml_UnrefNode(&fieldNode);
    fieldNode = xml_MakeNewNodeC(file, "IndexedLineSet", protoNode);
    /* put this line back when you sort out the above hugh */
    /*
       xml_UnrefNode(&parentNode);
       */
    parentNode = xml_MakeNewNodeC(file, "IS", fieldNode);
    /* put this line back when you sort out the above hugh */
    /*
       xml_UnrefNode(&grandParentNode);
       */
    grandParentNode = xml_MakeNewNodeC(file, "connect", parentNode);

    xml_SetAttributeC(grandParentNode, "protoField", "Graph.index");
    xml_SetAttributeC(grandParentNode, "nodeField", "coordIndex");

    xml_UnrefNode(&parentNode);
    parentNode = xml_MakeNewNodeC(file, "Coordinate", fieldNode);
    xml_UnrefNode(&grandParentNode);
    grandParentNode = xml_MakeNewNodeC(file, "IS", parentNode);
    xml_UnrefNode(&parentNode);
    parentNode = xml_MakeNewNodeC(file, "connect", grandParentNode);

    xml_SetAttributeC(parentNode, "protoField", "Graph.points");
    xml_SetAttributeC(parentNode, "nodeField", "point");

    /* This is the replacement temp hugh see above as well */
    xml_UnrefNode(&parentNode);
    parentNode = xml_MakeNewNodeC(file, "Color", fieldNode);
    xml_SetAttributeC(parentNode, "color", "0 0 0");
  
    xml_UnrefNode(&protoNode);
    protoNode = xml_MakeNewNodeC(file, "Group", groupNode);
    xml_UnrefNode(&grandParentNode);
    grandParentNode = xml_MakeNewNodeC(file, "IS", protoNode);
    xml_UnrefNode(&parentNode);
    parentNode = xml_MakeNewNodeC(file, "connect", grandParentNode);

    xml_SetAttributeC(parentNode, "protoField", "children");
    xml_SetAttributeC(parentNode, "nodeField", "children");

    xml_UnrefNode(&protoNode);
    xml_UnrefNode(&fieldNode);
    xml_UnrefNode(&groupNode);
    xml_UnrefNode(&grandParentNode);
    xml_UnrefNode(&parentNode);

    return;
}




/* @funcstatic xml_AddDNAPlotProto ***********************************
**
** Adds the nodes that set up the DNA Plot Proto
**
** @param [w] file [AjPGraphXml] file to add bit to
**
** @return [void]
** @@
*********************************************************************/

static void xml_AddDNAPlotProto(AjPGraphXml file)
{
    AjPXmlNode protoNode;
    AjPXmlNode fieldNode;
    AjPXmlNode parentNode;
    AjPXmlNode grandParentNode;

    parentNode = xml_GetParent(xml_GetCurrentScene(file));
    grandParentNode = xml_GetParent(parentNode);
    protoNode = xml_MakeNewNodeC(file, "ProtoDeclare", grandParentNode);
    xml_SetAttributeC(protoNode, "name", "DNAPlot");
    xml_SetAttributeC(protoNode, "EXPORT", "Graphic_Children");


    fieldNode = xml_MakeNewNodeC(file, "field", protoNode);
    xml_SetAttributeC(fieldNode, "name", "Source");
    xml_SetAttributeC(fieldNode, "type", "SFString");
    xml_SetAttributeC(fieldNode, "value", "");

    xml_UnrefNode(&fieldNode);
    fieldNode = xml_MakeNewNodeC(file, "field", protoNode);
    xml_SetAttributeC(fieldNode, "IS", "Graphic_Children.Shape");
    xml_SetAttributeC(fieldNode, "name", "Shape");
    xml_SetAttributeC(fieldNode, "type", "Node");
    xml_SetAttributeC(fieldNode, "value", "NULL");

    xml_UnrefNode(&fieldNode);
    fieldNode = xml_MakeNewNodeC(file, "children", protoNode);
    xml_SetAttributeC(fieldNode, "DEF", "Graphic_Children");

    xml_UnrefNode(&protoNode);
    xml_UnrefNode(&fieldNode);
    xml_UnrefNode(&grandParentNode);
    xml_UnrefNode(&parentNode);

    return;
}




/* @funcstatic xml_AddCommonBit *************************************
**
** Adds the nodes that are common for all files
**
** Could realy be all part of setUpOutputFile, but it is easier to 
** read
** Private Function
**
** @param [w] file [AjPGraphXml] file to add common bit to
**
** @return [void]
** @@
*********************************************************************/

static void xml_AddCommonBit(AjPGraphXml file)
{
    /* remove this */
    /*
       AjPXmlNode el2;
       AjPXmlNode el3;
       AjPXmlNode elChildren;
       */
    AjPXmlNode el;


    /* Xj3D does not like this at the moment, sort it out */
    /*
       el = xml_MakeNewNodeC(file, "head", xml_GetCurrentGraphic(file));
       el2 = xml_MakeNewNodeC(file, "meta", el);
       */
    /*
       xml_UnrefNode(&el);
       */
    el = xml_MakeNewNodeC(file, "Scene", xml_GetCurrentGraphic(file));
    /*
       el3 = xml_MakeNewNodeC(file, "Group", el);
       elChildren = xml_MakeNewNodeC(file, "children", el3); 
       */

    /* I move file->currentScene and file->currentGraphic to the Scene*/
    /*
       xml_SetCurrentScene(file, elChildren);
       xml_SetCurrentGraphic(file, elChildren);
       */
    xml_SetCurrentScene(file, el);
    xml_SetCurrentGraphic(file, el);

    /*
       xml_UnrefNode(&el); 
       xml_UnrefNode(&el2);
       */
    /*
       xml_UnrefNode(&el);
       */
    /* can you have this line? */
    /*
       xml_UnrefNode(&el3);
       */

    return;
}




/* @funcstatic xml_WriteFile ***********************************************
**
** writes the file 
**
** @param [r] file [const AjPGraphXml] the file to write
** @param [r] filename [const AjPStr] the file name
**
** @return [AjBool] ajTrue if file written correctly
** @@
*********************************************************************/

static AjBool xml_WriteFile(const AjPGraphXml file, const AjPStr filename)
{
    GdomeException exc;

    if (!gdome_di_saveDocToFileEnc (file->domimpl, file->doc, 
				 ajStrGetPtr(filename), "UTF-8",
				 GDOME_SAVE_LIBXML_INDENT, &exc)) 
    {
	ajDebug("DOMImplementation.saveDocToFile: failed\n\tException #%d\n",
		exc);
	return ajFalse;
    }

    return ajTrue;
}




/* @funcstatic xml_WriteStdout *********************************************
**
** writes the file 
**
** @param [r] file [const AjPGraphXml] the file to write
**
** @return [AjBool] ajTrue if file writen correctly
** @@
*********************************************************************/

static AjBool xml_WriteStdout(const AjPGraphXml file)
{
    GdomeException exc;
    char *output;
    ajint outputLength, i;
  
    if (!gdome_di_saveDocToMemoryEnc (file->domimpl, file->doc, &output,
				      "UTF-8", GDOME_SAVE_LIBXML_INDENT,
				      &exc)) 
    {
	ajDebug("DOMImplementation.saveDocToMemory: failed\n\tException #%d\n",
		exc);

	return ajFalse;
    }
    outputLength = strlen(output);

    for(i = 0; i < outputLength; ++i)
	fprintf(stdout,"%c", output[i]);

    free(output);
  
    return ajTrue;
}




/* @funcstatic xml_GetCurrentGraphic ********************************
**
** Local Method
** gets the current Graphic
**
** @param [r] file [const AjPGraphXml] the file
**
** @return [AjPXmlNode] current Graphic
** @@
*********************************************************************/

static AjPXmlNode xml_GetCurrentGraphic(const AjPGraphXml file)
{
    return file->currentGraphic;
}




/* @funcstatic xml_GetCurrentScene **********************************
**
** Local Method
** gets the current scene
**
** @param [r] file [const AjPGraphXml] the file
**
** @return [AjPXmlNode] current scene
** @@
*********************************************************************/

static AjPXmlNode xml_GetCurrentScene(const AjPGraphXml file)
{
    return file->currentScene;
}




/* @funcstatic xml_SetCurrentGraphic ********************************
**
** Local Method
** Sets the current graphic to be node
**
** @param [w] file [AjPGraphXml] file to write to
** @param [u] node [AjPXmlNode] the node
**
** @return [void]
** @@
*********************************************************************/

static void xml_SetCurrentGraphic(AjPGraphXml file, AjPXmlNode node)
{

    if(file->currentGraphic && 
       !(file->currentScene == file->currentGraphic))
	if(xml_GetNode(file->currentGraphic) != 0)
	    xml_UnrefNode(&file->currentGraphic);
    
    file->currentGraphic = node;

    return;
}




/* @funcstatic xml_SetCurrentScene **********************************
**
** Local Method
** Sets the current Scene to be node
**
** @param [w] file [AjPGraphXml] file to write to
** @param [u] node [AjPXmlNode] the node
**
** @return [void]
** @@
*********************************************************************/

static void xml_SetCurrentScene(AjPGraphXml file, AjPXmlNode node)
{
    /* why does this cause proplems, but above does not? */
    /* but above does cause problems, I am not sure why */
    /* Sorted now I think, but a bit messy */
    /* Is it messy?  I kind of like the way it does it */
    
    if(file->currentScene &&
       !(file->currentScene == file->currentGraphic))
	if(xml_GetNode(file->currentScene) != 0)
	    xml_UnrefNode(&file->currentScene);

    file->currentScene = node;

    return;
}




/* @funcstatic xml_SetNode *****************************************
**
** Local Method
** Makes an AjPXmlNode
**
** @param [u] node [GdomeNode*] node to set
**
** @return [AjPXmlNode] new node
** @@
*********************************************************************/

static AjPXmlNode xml_SetNode(GdomeNode *node)
{
    AjPXmlNode retNode = ajXmlNodeNew();
    
    retNode->theNode = node;

    return retNode;
}




/* @funcstatic xml_GetNode *****************************************
**
** Local Method
** gets the node of a AjPXmlNode
**
** @param [u] node [AjPXmlNode] node to set
**
** @return [GdomeNode*] new node
** @@
********************************************************************/

static GdomeNode* xml_GetNode(AjPXmlNode node)
{
    return node->theNode;
}




/* @funcstatic xml_GetNodeElement **********************************
**
** Local Method
** gets the node of a AjPXmlNode as a GdomeElement
**
** @param [u] node [AjPXmlNode] node to get
**
** @return [GdomeElement*] the node
** @@
*********************************************************************/

static GdomeElement* xml_GetNodeElement(AjPXmlNode node)
{
    return (GdomeElement *) node->theNode;
}




/* @funcstatic xml_GetParent **************************************
**
** Local Method
** gets the node of a AjPXmlNode as a GdomeElement
**
** @param [u] node [AjPXmlNode] node to get
**
** @return [AjPXmlNode] the node
** @@
*********************************************************************/

static AjPXmlNode xml_GetParent(AjPXmlNode node)
{
    GdomeException exc; 

    /* this really should not be running xml_SetNode, but I think it 
    ** has to.
    ** OK only as long as you unref the node each time */

    return xml_SetNode(gdome_n_parentNode(xml_GetNode(node), &exc));
}




/* @funcstatic xml_ClearFile ***********************************************
**
** Clears the memory allocated to this file
**
** Needs a reference so the pointer in the caller can be correctly zeroed.
**
** @param [d] pfile [AjPGraphXml*] the file to clear
**
** @return [void]
** @@
*********************************************************************/

static void xml_ClearFile(AjPGraphXml *pfile)
{
    GdomeException exc;

    AjPGraphXml file = *pfile;

    if(file->currentGraphic == file->currentScene)
	ajXmlNodeDel(&file->currentGraphic);
    else
    {
	ajXmlNodeDel(&file->currentGraphic);
	ajXmlNodeDel(&file->currentScene);
    }


/*    gdome_di_freeDoc (file->domimpl, file->doc, &exc);*/
    gdome_doc_unref (file->doc, &exc);
    gdome_di_unref (file->domimpl, &exc);
    
    ajXmlFileDel(pfile);

    return;
}




/* @funcstatic  xml_UnrefNode ***********************************************
**
** Clears the memory allocated to this node
**
** @param [d] pnode [AjPXmlNode*] the node to clear
**
** @return [void]
** @@
*********************************************************************/

static void xml_UnrefNode(AjPXmlNode *pnode)
{
    AjPXmlNode node = *pnode;
    GdomeException exc;

    gdome_n_unref (xml_GetNode(node), &exc);
    ajXmlNodeDel(pnode);

    return;
}




/* @func  xml_Unused ***********************************************
**
** Catch functions that are defined but never used to stop compile warnings
**
** @return [void]
** @@
*********************************************************************/

void xml_Unused()
{
    AjPStr str      = NULL;
    double ret;
    AjPGraphXml file = NULL;
    AjPXmlNode node = NULL;
    
    ret = xml_GetLastDouble(str);
    ret = xml_GetDoubleNo(str,0);

    xml_MakeNewNode(file,str,node);
    xml_PresentGraphicTypeIs(file,str);
    xml_GetAttribute(node,str);    
    xml_SetAttribute(node,str,str);
    xml_FileNeedsProtoDeclare(file,str);

    return;
}




/* @funcstatic  xml_clear_nodeTypes ************************************
**
** Clear primary table allocation for colourtable
**
** @param [r] key [void**] Standard argument, table key.
** @param [r] value [void**] Standard argument, table data item.
** @param [r] cl [void*] Standard argument, usually NULL
**
** @return [void]
** @@
*********************************************************************/

static void xml_clear_nodeTypes(void **key, void **value, void *cl)
{
    AjPTable table = (AjPTable) *value;
    AjPStr skey = (AjPStr) *key;
    
    ajStrDel(&skey);
    ajTableMapDel(table,xml_deltablenode,NULL);

    ajTableFree(&table);

    *key = NULL;
    *value = NULL;

    return;
}




/* @funcstatic  xml_deltablenode ************************************
**
** Clear secondary (XmlNode) table allocation for colourtable subtables
**
** @param [r] key [void**] Standard argument, table key.
** @param [r] value [void**] Standard argument, table data item.
** @param [r] cl [void*] Standard argument, usually NULL
**
** @return [void]
** @@
*********************************************************************/

static void xml_deltablenode(void **key, void **value, void *cl)
{
    AjPXmlNode node;
    AjPStr skey;

    node = (AjPXmlNode) *value;
    skey = (AjPStr) *key;

    ajStrDel(&skey);
    ajXmlNodeDel(&node);

    *key = NULL;
    *value = NULL;

    return;
}

#endif
