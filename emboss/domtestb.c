/* @source domtestb application
**
** Create, manipulate and write out XML
**
** @author Copyright (C) Alan Bleasby
** @@
**
** This program is free software; you can redistribute it and/or
** modify it under the terms of the GNU General Public License
** as published by the Free Software Foundation; either version 2
** of the License, or (at your option) any later version.
**
** This program is distributed in the hope that it will be useful,
** but WITHOUT ANY WARRANTY; without even the implied warranty of
** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
** GNU General Public License for more details.
**
** You should have received a copy of the GNU General Public License
** along with this program; if not, write to the Free Software
** Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
******************************************************************************/

#include "emboss.h"




/* @prog domtestb *************************************************************
**
** Create, manipulate and write out XML
**
******************************************************************************/

int main(int argc, char **argv)
{
    AjPDomDocument doc = NULL;

    AjPDomElement root = NULL;
    AjPDomElement fu   = NULL;
    AjPDomElement bar  = NULL;
    AjPDomElement emboss = NULL;
    AjPDomElement clone  = NULL;
    AjPDomElement wibble = NULL;

    AjPDomText txt = NULL;

    AjPDomAttr attr = NULL;
    
    AjPFile outf = NULL;

    ajint i;
    ajint  n;
    
    
    embInit("domtestb", argc, argv);

    outf = ajAcdGetOutfile("outfile");

    /* Create the document */
    
    doc = ajDomImplementationCreateDocument(NULL,NULL,NULL);

    /* Create some elements */
    
    root = ajDomDocumentCreateElementC(doc, "root");
    fu   = ajDomDocumentCreateElementC(doc, "fu");
    bar  = ajDomDocumentCreateElementC(doc, "bar");

    /* Create a text node */
    txt = ajDomDocumentCreateTextNodeC(doc, "This is text.");

    /* Play around with the text node by appending text to it */
    ajDomCharacterDataAppendDataC(txt, "More text.");

    /* ... and inserting some moretext at position 5 */
    ajDomCharacterDataInsertDataC(txt, 5, "INSERTED ");
    
    /*
    ** Create a simple XML structure
    ** There is a root node having children 'fu' and 'bar'
    ** Element 'bar' has a text node child 'txt'
    */
    
    ajDomNodeAppendChild(doc, root);
    ajDomNodeAppendChild(root, fu);
    ajDomNodeAppendChild(root, bar);
    ajDomNodeAppendChild(bar, txt);

    
    /* Show the XML so far */
    ajDomWrite(doc,outf);
    ajFmtPrintF(outf,"\n\n");
    
    /* Make a copy of the 'bar' node and all its children ('1') */
    clone = ajDomNodeCloneNode(bar, 1);


    /* Replace 'INSERTED' with 'Hello World' */
    ajDomCharacterDataReplaceDataC(txt, 5, 8, "HELLO  WORLD");

    /* Delete the 'is' in 'This' */
    ajDomCharacterDataDeleteData(txt, 2, 2);

    /*
    ** Split the text at position 14 creating two adjacent text  nodes
    */
    ajDomTextSplitText(txt, 14);


    /*
    ** Have a peek so far. The clone and text split have no visible effect
    ** at this point.
    ** Here the 'root' node is written, not the whole document.
    */
    ajDomWrite(root,outf);
    ajFmtPrintF(outf,"\n\n\n");

    
    /*
    ** Append the cloned node to the 'fu' node i.e. it will appear
    ** before the end of the closing </fu> . The NULL specifies that
    ** it is an append.
    */
    ajDomNodeInsertBefore(fu, clone, NULL);

    ajDomWrite(root,outf);
    ajFmtPrintF(outf,"\n\n\n");


    /*
    ** Replace the 'txt' node with the 'wibble' node as the child of 'bar'.
    ** This shows the effect of having previously split the text node
    ** into two adjacent nodes.
    */
    wibble  = ajDomDocumentCreateElementC(doc, "wibble");
    ajDomNodeReplaceChild(bar, wibble, txt);

    ajDomWrite(root,outf);
    ajFmtPrintF(outf,"\n\n\n");

    /* Remove the bar node from the root node */
    ajDomRemoveChild(root, bar);

    ajDomWrite(root,outf);
    ajFmtPrintF(outf,"\n\n\n");

    /* Append the 'txt' node to the 'clone' node list */
    ajDomNodeInsertBefore(clone, txt, NULL);

    ajDomWrite(root,outf);
    ajFmtPrintF(outf,"\n\n\n");

    emboss  = ajDomDocumentCreateElementC(doc, "emboss");
    
     /* Insert another node element 'emboss' before the 'txt'  node */
    ajDomNodeInsertBefore(clone, emboss, txt);

    ajDomWrite(root,outf);
    ajFmtPrintF(outf,"\n\n\n");

    /* Insert element 'bar' before the 'txt'  node */
    ajDomNodeInsertBefore(clone, bar, txt);

    ajDomWrite(root,outf);
    ajFmtPrintF(outf,"\n\n\n");

    /* Print the node names in the clone element */
    n = clone->childnodes->length;

    for(i=0; i < n; ++i)
        ajFmtPrintF(outf,"%S\n",ajDomNodeListItem(clone->childnodes,i)->name);


    /* Create and destroy an attribute just because it's possible */
    attr = ajDomDocumentCreateAttributeC(doc, "myatt");
    ajDomDocumentDestroyNode(doc, &attr);

    ajDomWrite(doc,outf);

    /* Delete the entire document */
    ajDomDocumentDestroyNode(doc, &doc);

    ajFileClose(&outf);

    embExit();

    
    return 0;
}
