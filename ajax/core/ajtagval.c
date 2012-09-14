/* @include ajtagval **********************************************************
**
** Handling of tag-value pairs of strings
**
** @author Copyright (C) 2011 Peter Rice
** @version $Revision: 1.1 $
** @modified $Date: 2011/11/08 15:01:23 $ by $Author: rice $
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
******************************************************************************/

#include "ajlib.h"
#include "ajtagval.h"




/* @filesection ajtagval ******************************************************
**
** @nam1rule aj Function belongs to the AJAX library.
**
******************************************************************************/




/* @datasection [AjPTagval] Tag value data ************************************
**
** Function is for manipulating tag value pairs
**
** @nam2rule Tagval Tag value pairs
**
******************************************************************************/




/* @section constructors ******************************************************
**
** Constructors
**
** @fdata [AjPTagval]
**
** @nam3rule New Constructor
** @suffix   C Character values
** @suffix   S String    values
**
** @argrule C tagtxt [const char*]  Tag name
** @argrule C valtxt [const char*]  Tag value
** @argrule S tag    [const AjPStr] Tag name
** @argrule S val    [const AjPStr] Tag value
**
** @valrule * [AjPTagval] Tag value pair object
**
** @fcategory new
**
******************************************************************************/




/* @func ajTagvalNewC *********************************************************
**
** Tag value pair constructor
**
** @param [r] tagtxt [const char*] Tag name
** @param [r] valtxt [const char*] Tag value
** @return [AjPTagval] New object
**
** @release 6.5.0
** @@
******************************************************************************/

AjPTagval ajTagvalNewC(const char* tagtxt, const char* valtxt)
{
    AjPTagval ret;

    AJNEW0(ret);

    ret->Tag = ajStrNewC(tagtxt);
    ret->Value = ajStrNewC(valtxt);

    return ret;
}




/* @func ajTagvalNewS *********************************************************
**
** Tag value pair constructor
**
** @param [r] tag   [const AjPStr] Tag name
** @param [r] val [const AjPStr] Tag value
** @return [AjPTagval] New object
**
** @release 6.5.0
** @@
******************************************************************************/

AjPTagval ajTagvalNewS(const AjPStr tag, const AjPStr val)
{
    AjPTagval ret;

    AJNEW0(ret);

    ret->Tag = ajStrNewS(tag);
    ret->Value = ajStrNewS(val);

    return ret;
}




/* @section Tag value pair destructors ****************************************
**
** Destruction destroys all internal data structures and frees the
** memory allocated for the tag value pair object.
**
** @fdata [AjPTagval]
**
** @nam3rule Del Destructor
**
** @argrule Del Ptagval [AjPTagval*] Tag value pair object
**
** @valrule * [void]
**
** @fcategory delete
**
******************************************************************************/




/* @func ajTagvalDel **********************************************************
**
** Tag value pair destructor
**
** @param [d] Ptagval       [AjPTagval*] Tag value pair object to delete
** @return [void] 
**
** @release 6.5.0
** @@
******************************************************************************/

void ajTagvalDel(AjPTagval *Ptagval)
{
    AjPTagval tagval;

    if(!Ptagval) return;
    if(!(*Ptagval)) return;

    tagval = *Ptagval;

    ajStrDel(&tagval->Tag);
    ajStrDel(&tagval->Value);

    AJFREE(*Ptagval);
    *Ptagval = NULL;

    return;
}




/* @section element retrieval *************************************************
**
** Functions for returning elements of a tag value pair object.
**
** @fdata       [AjPTagval]
**
** @nam3rule   Get         Retrieve an unmodified element of a string object. 
** @nam4rule   GetTag      Retrieve tag name
** @nam4rule   GetValue    Retrieve tag value
**
** @argrule * tagval [const AjPTagval] Tag value pair
**
** @valrule GetTag   [const AjPStr] Tag name
** @valrule GetValue [const AjPStr] Tag value
**
** @fcategory use
******************************************************************************/




/* @func ajTagvalGetTag *******************************************************
**
** Returns the tag name
**
** @param [r] tagval [const AjPTagval] Source tag value pair
** @return [const AjPStr] Current tag name, or a null string if undefined.
**
** @release 6.5.0
** @@
******************************************************************************/

const AjPStr ajTagvalGetTag(const AjPTagval tagval)
{
    if(!tagval)
	return NULL;


    return tagval->Tag;
}




/* @macro MAJTAGVALGETTAG *****************************************************
**
** Returns the tag name
**
** A macro version of {ajTagvalGetTag} available in case it is needed for speed.
**
** @param [r] tagval [const AjPTagval] Source tag value pair
** @return [const AjPStr] Current tag name, or a null string if undefined.
**
** @release 6.5.0
** @@
******************************************************************************/




/* @func ajTagvalGetValue *****************************************************
**
** Returns the tag value
**
** @param [r] tagval [const AjPTagval] Source tag value pair
** @return [const AjPStr] Current tag value, or a null string if undefined.
**
** @release 6.5.0
** @@
******************************************************************************/

const AjPStr ajTagvalGetValue(const AjPTagval tagval)
{
    if(!tagval)
	return NULL;


    return tagval->Value;
}




/* @macro MAJTAGVALGETVALUE ***************************************************
**
** Returns the tag value
**
** A macro version of {ajTagvalGetTagValue} available in case it is
** needed for speed.
**
** @param [r] str [const AjPStr] Source string
** @return [const AjPStr] Current tag value, or a null string if undefined.
** @@
******************************************************************************/




/* @section modifiers *********************************************************
**
** @fdata [AjPTagval]
**
** @nam3rule  Append      Add text to the tag value
** @nam3rule  Fmt         Change the format of a tag name
** @nam3rule  Replace     Replace the tag value
**
** @nam4rule  FmtLower    Convert to lower case.
** @nam4rule  FmtUpper    Convert to upper case.
**
** @suffix C  Character data
** @suffix S  String data
**
** @argrule * tagval [AjPTagval] Tag value pair
** @argrule C txt [const char*] Text
** @argrule S str [const AjPStr] Text
**
** @valrule * [void]
**
** @fcategory modify
******************************************************************************/




/* @func ajTagvalAppendC ******************************************************
**
** Adds text to the tag value
**
** @param [u] tagval [AjPTagval] Tag value pair
** @param [r] txt [const char*] Text
** @return [void]
**
** @release 6.5.0
** @@
******************************************************************************/

void ajTagvalAppendC(AjPTagval tagval, const char* txt)
{
    ajStrAppendC(&tagval->Value, txt);
    return;
}




/* @func ajTagvalAppendS ******************************************************
**
** Adds text to the tag value
**
** @param [u] tagval [AjPTagval] Tag value pair
** @param [r] str [const AjPStr] Text
** @return [void]
**
** @release 6.5.0
** @@
******************************************************************************/

void ajTagvalAppendS(AjPTagval tagval, const AjPStr str)
{
    ajStrAppendS(&tagval->Value, str);
    return;
}




/* @func ajTagvalFmtLower *****************************************************
**
** Converts a tag value pair name to lower case
**
** @param [u] tagval [AjPTagval] Tag value pair
** @return [void]
**
** @release 6.5.0
** @@
******************************************************************************/

void ajTagvalFmtLower(AjPTagval tagval)
{
    ajStrFmtLower(&tagval->Tag);
    return;
}




/* @func ajTagvalFmtUpper *****************************************************
**
** Converts a tag value pair name to upper case
**
** @param [u] tagval [AjPTagval] Tag value pair
** @return [void]
**
** @release 6.5.0
** @@
******************************************************************************/

void ajTagvalFmtUpper(AjPTagval tagval)
{
    ajStrFmtUpper(&tagval->Tag);
    return;
}




/* @func ajTagvalReplaceC ******************************************************
**
** Replace the tag value
**
** @param [u] tagval [AjPTagval] Tag value pair
** @param [r] txt [const char*] Text
** @return [void]
**
** @release 6.5.0
** @@
******************************************************************************/

void ajTagvalReplaceC(AjPTagval tagval, const char* txt)
{
    ajStrAssignC(&tagval->Value, txt);
    return;
}




/* @func ajTagvalReplaceS ******************************************************
**
** Replace the tag value
**
** @param [u] tagval [AjPTagval] Tag value pair
** @param [r] str [const AjPStr] Text
** @return [void]
**
** @release 6.5.0
** @@
******************************************************************************/

void ajTagvalReplaceS(AjPTagval tagval, const AjPStr str)
{
    ajStrAssignS(&tagval->Value, str);
    return;
}
