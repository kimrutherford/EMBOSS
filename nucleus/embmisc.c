/* @source embmisc.c
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




/* @func embMiscMatchPatternDelimC *********************************************
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
