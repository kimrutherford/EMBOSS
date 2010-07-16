/******************************************************************************
** @source AJAX public data resource functions
**
** @author Copyright (C) 2010 Peter Rice
** @version 1.0
** @modified Jun 10 pmr First version
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

#include <math.h>
#include "ajax.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include <limits.h>

static ajuint     resourceErrorCount = 0;


static AjPTable resourceIdTable = NULL;
static AjPTable resourceNameTable = NULL;

const char* resourceStatus[] = {
    "Supported",
    "Referenced",
    "Unavailable",
    NULL
};

const char* resourceDataFormat[] = {
    "Unknown",
    "HTML",
    "Text",
    "Text (EMBL)",
    "Text (fasta)",
    "Text (gff)",
    NULL
};
    
const char* resourceTags[] = {
    "ID",
    "IDOther",
    "Name",
    "Desc",
    "Server",
    "Cat",
    "EDAM",
    "Link",
    "LinkNB",
    "Query",
    "Example",
    "Contact",
    "Email",
    "Note",
    "Status",
    NULL
};

static void resourceWarn(const AjPFile obofile, ajuint linecnt,
                         const char* fmt, ...);




/* @func ajResourceParse ******************************************************
**
** Parse the standard list of public data resources
**
** @param [u] dbfile [AjPFile] Open database list file
** @param [r] validations [const char*] Validations to be turned on or off
**
******************************************************************************/

void ajResourceParse(AjPFile dbfile, const char* validations)
{
    AjPStr line = NULL;
    ajuint linecnt = 0;
    AjPResource res = NULL;
    AjPStr token = NULL;
    AjPStr rest = NULL;
    AjPStr tmpstr = NULL;
    AjPStr idkey = NULL;
    AjPStr namekey = NULL;
    ajuint j;

    (void) validations;

    while (ajReadlineTrim(dbfile, &line))
    {
        linecnt++;

        if(!ajStrGetLen(line))
            continue;

        if(ajStrGetCharFirst(line) == '#')
            continue;

        ajStrExtractWord(line, &rest, &token);

        j=0;
        while(resourceTags[j] &&
                  !ajStrMatchC(token, resourceTags[j]))
                j++;
        if(!resourceTags[j])
        {
            resourceWarn(dbfile, linecnt,
                         "unknown tag name '%S'",
                         token);
            continue;
        }

        if(ajStrMatchC(token, "ID"))
        {
            AJNEW0(res);
            res->Id = ajStrNewS(rest);
            idkey = ajStrNewS(rest);
            ajTablePut(resourceIdTable, idkey, res);
        }
        else  if(ajStrMatchC(token, "IDOther"))
        {
            if(ajStrMatchC(rest, "None"))
                continue;
            if(!res->Otherids)
                res->Otherids = ajListNew();
            tmpstr = ajStrNewS(rest);
            ajListPushAppend(res->Otherids, tmpstr);
            tmpstr = NULL;
            idkey = ajStrNewS(rest);
            ajTablePut(resourceIdTable, idkey, res);
        }   
        else  if(ajStrMatchC(token, "Name"))
        {
            if(ajStrGetLen(res->Name))
                resourceWarn(dbfile, linecnt,
                             "duplicate tag Name");
            else
                res->Name = ajStrNewS(rest);
            namekey = ajStrNewS(rest);
            ajTablePut(resourceNameTable, namekey, res);                
        }   
        else  if(ajStrMatchC(token, "Desc"))
        {
            if(ajStrGetLen(res->Desc))
                resourceWarn(dbfile, linecnt,
                             "duplicate tag Desc");
            else
                res->Desc = ajStrNewS(rest);
        }   
        else  if(ajStrMatchC(token, "Server"))
        {
            if(ajStrGetLen(res->Server))
                resourceWarn(dbfile, linecnt,
                             "duplicate tag Server");
            else
                res->Server = ajStrNewS(rest);
        }   
        else  if(ajStrMatchC(token, "Category"))
        {
            if(ajStrGetLen(res->Category))
                resourceWarn(dbfile, linecnt,
                             "duplicate tag Cat");
            else
                res->Category = ajStrNewS(rest);
        }   
        else  if(ajStrMatchC(token, "EDAM"))
        {
            if(!res->Edamrefs)
                res->Edamrefs = ajListNew();
            tmpstr = ajStrNewS(rest);
            ajListPushAppend(res->Edamrefs, tmpstr);
            tmpstr = NULL;
        }   
        else  if(ajStrMatchC(token, "Link"))
        {
            if(!res->Links)
                res->Links = ajListNew();
            tmpstr = ajStrNewS(rest);
            ajListPushAppend(res->Links, tmpstr);
            tmpstr = NULL;
        }   
        else  if(ajStrMatchC(token, "LinkNB"))
        {
            if(ajStrGetLen(res->Linknote))
                resourceWarn(dbfile, linecnt,
                             "duplicate tag LinkNB");
            else
                res->Linknote = ajStrNewS(rest);
        }   
        else  if(ajStrMatchC(token, "Query"))
        {
            if(!res->Queries)
                res->Queries = ajListNew();
            tmpstr = ajStrNewS(rest);
            ajListPushAppend(res->Queries, tmpstr);
            tmpstr = NULL;
        }   
        else  if(ajStrMatchC(token, "Example"))
        {
            if(!res->Examples)
                res->Examples = ajListNew();
            tmpstr = ajStrNewS(rest);
            ajListPushAppend(res->Examples, tmpstr);
            tmpstr = NULL;
        }   
        else  if(ajStrMatchC(token, "Contact"))
        {
            if(ajStrGetLen(res->Contact))
                resourceWarn(dbfile, linecnt,
                             "duplicate tag Contact");
            else
                res->Contact = ajStrNewS(rest);
        }   
        else  if(ajStrMatchC(token, "Email"))
        {
            if(ajStrGetLen(res->Email))
                resourceWarn(dbfile, linecnt,
                             "duplicate tag Email");
            else
                res->Email = ajStrNewS(rest);
        }   
        else  if(ajStrMatchC(token, "Note"))
        {
            if(ajStrGetLen(res->Notes))
                resourceWarn(dbfile, linecnt,
                             "duplicate tag Note");
            else
                res->Notes = ajStrNewS(rest);
        }   
        else  if(ajStrMatchC(token, "Status"))
        {
            j=0;
            while(resourceStatus[j] &&
                  !ajStrMatchC(rest, resourceStatus[j]))
                j++;
            if(!resourceStatus[j])
                resourceWarn(dbfile, linecnt,
                             "unknown Status '%S'", rest);

            if(ajStrGetLen(res->Status))
                resourceWarn(dbfile, linecnt,
                             "duplicate tag Desc");
            else
                res->Status = ajStrNewS(rest);
        }   
    }

    return;
}




/* @funcstatic resourceWarn **************************************************
**
** Formatted write as an error message.
**
** @param [r] obofile [const AjPFile] Input OBO file
** @param [r] linecnt [ajuint] Current line number
** @param [r] fmt [const char*] Format string
** @param [v] [...] Format arguments.
** @return [void]
** @@
******************************************************************************/

static void resourceWarn(const AjPFile obofile, ajuint linecnt,
                         const char* fmt, ...)
{
    va_list args;
    AjPStr errstr = NULL;

    resourceErrorCount++;

    va_start(args, fmt) ;
    ajFmtVPrintS(&errstr, fmt, args);
    va_end(args) ;

    ajWarn("File %F line %u: %S", obofile, linecnt, errstr);
    ajStrDel(&errstr);

    return;
}




