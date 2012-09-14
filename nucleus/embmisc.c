/* @source embmisc ************************************************************
**
** Miscellaneous routines
**
** @author Copyright (c) 1999 Alan Bleasby
** @version $Revision: 1.14 $
** @modified $Date: 2011/10/18 14:24:24 $ by $Author: rice $
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

#include "embmisc.h"
#include "ajlib.h"




/* @func embMiscMatchPatternDelimC ********************************************
**
** Does a simple OR'd test of matches to (possibly wildcarded) words.
** The words are tested one at a time until a match is found.
** Whitespace and additional delimiter characters can separate the
** words in the pattern.
**
** @param [r] str [const AjPStr] string to test
** @param [r] pattern [const AjPStr] pattern to match with
** @param [r] delim [const char*] additional delimiters
**
** @return [AjBool] ajTrue = found a match
**
** @release 6.2.0
** @@
******************************************************************************/

AjBool embMiscMatchPatternDelimC (const AjPStr str, const AjPStr pattern,
                                  const char* delim)
{

    char whiteSpace[] = " \t\n\r";  /* skip whitespace */
    AjPStrTok tokens;
    AjPStr delimstr = NULL;
    AjPStr key = NULL;
    AjBool val = ajFalse;

    delimstr = ajStrNewC(whiteSpace);
    ajStrAppendC(&delimstr, delim);
    tokens = ajStrTokenNewS(pattern, delimstr);

    while (ajStrTokenNextParse( &tokens, &key))
	if (ajStrMatchWildCaseS(str, key))
	{
	    val = ajTrue;
	    break;
	}

    ajStrTokenDel( &tokens);
    ajStrDel(&key);
    ajStrDel(&delimstr);

    return val;

}
