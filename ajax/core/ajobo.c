/******************************************************************************
** @source AJAX OBO handling functions
**
** @author Copyright (C) 2010 Peter Rice
** @version 1.0
** @modified May 5 pmr First AJAX version
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




/******************************************************************************
**
** To Do:
**
** alt_id needs to become a stored ID somehow and resolve to the same list
**
** validate instance, annotation and formula stanzas
**
** do something (store) header tags
**
** more syntax validation where useful
**
** escape quotes somewhere - after processing the quoted string,
** return it escaped.
**
** validate number of times a term is found -
**   count all possible terms and compare to the
**     minima 0:may-be-absent 1:required
**     maxima 0:unlimited 1:unique 2:pair-at-least
**
** Report all tags that have the obsolete value set
**
******************************************************************************/

#if 0
static const char *oboBuiltin[] =
{
    "OBO:TYPE", 
    "OBO:TERM", 
    "OBO:TERM_OR_TYPE", 
    "OBO:INSTANCE", 
    "xsd:simpleType",
    "xsd:string",
    "xsd:integer",
    "xsd:decimal",
    "xsd:negativeInteger",
    "xsd:positiveInteger",
    "xsd:nonNegativeInteger",
    "xsd:nonPositiveInteger",
    "xsd:boolean",
    "xsd:date",
    NULL
};
#endif



static const char *oboStanza[] =
{
    "Typedef", 
    "Term",
    "Instance", 
    "Annotation",               /* new in 1.3 */
    "Formula",                  /* new in 1.3 */
    NULL
};




enum tagtype {
    TAG_ANY, TAG_BOOL, TAG_QTEXT, TAG_DBX, TAG_QXREF, TAG_ID, TAG_NAME
};




/*
** dbxrefs are comma-separated ids and possible "comment"
** need to parse by ',' with quotes honoured.
** watch out for \" within quotes - perhaps these should be escaped later?
** or just accept as [dbxref-list]
*/

/*
** should parse out trailing modifiers from any tag
** {name=value ...}
*/

/* should parse out any dbxref list from any tag? */

typedef struct OboSTagdef 
{
    const char* Tag;
    ajint       Min;
    ajint       Count;
    ajint       Type;
    ajint       Padding;
    const char* Obsolete;
} OboOTagdef;




static OboOTagdef oboTermTags[] =
{
    {"id",              TAG_ID,   1, 1, 0, NULL}, 
    {"is_anonymous",    TAG_BOOL, 0, 1, 0, NULL}, 
    {"name",            TAG_NAME, 1, 1, 0, NULL}, 
    {"namespace",       TAG_ANY, 0, 1, 0, NULL}, 
    {"alt_id",          TAG_ANY, 0, 0, 0, NULL}, 
    {"def",             TAG_DBX, 0, 1, 0, NULL}, 
    {"comment",         TAG_ANY, 0, 1, 0, NULL}, 
    {"subset",          TAG_QXREF, 0, 0, 0, NULL}, 
    {"synonym",         TAG_NAME, 0, 0, 0, NULL},
    {"exact_synonym",   TAG_NAME, 0, 0, 0, "synonym EXACT"},
    {"narrow_synonym",  TAG_NAME, 0, 0, 0, "synonym NARROW"},
    {"broad_synonym",   TAG_NAME, 0, 0, 0, "synonym BROAD"},
    {"xref",            TAG_ID, 0, 0, 0, NULL}, 
    {"xref_analog",     TAG_ID, 0, 0, 0, "xref"}, 
    {"xref_unk",        TAG_ID, 0, 0, 0, "xref"}, 
    {"is_a",            TAG_ID, 0, 0, 0, NULL},  
    {"intersection_of", TAG_ANY, 0, 2, 0, NULL},
    {"union_of",        TAG_ANY, 0, 2, 0, NULL}, 
    {"disjoint_from",   TAG_ID, 0, 0, 0, NULL}, 
    {"relationship",    TAG_ANY, 0, 0, 0, NULL}, 
    {"is_obsolete",     TAG_BOOL, 0, 1, 0, NULL}, 
    {"replaced_by",     TAG_ID, 0, 0, 0, NULL},          /* for is_obsolete */
    {"consider",        TAG_ANY, 0, 0, 0, NULL},         /* for is_obsolete */
    {"use_term",        TAG_ANY, 0, 0, 0, "consider"}, 
    {"builtin",         TAG_BOOL, 0, 1, 0, NULL},
    {"created_by",      TAG_ANY, 0, 1, 0, NULL},               /* new in 1.3 */
    {"creation_date",   TAG_ANY, 0, 1, 0, NULL},            /* new in 1.3 */
    {NULL, 0, 0, 0, 0, NULL}
};




static const char *oboTypedefTags[] =
{
    "id", 
    "is_anonymous", 
    "name", 
    "namespace", 
    "alt_id", 
    "def", 
    "comment", 
    "subset", 
    "synonym",
    "exact_synonym",
    "narrow_synonym",
    "broad_synonym",
    "xref", 
    "xref_analog", 
    "xref_unk", 
    "domain", 
    "range", 
    "is_anti_symmetric", 
    "is_cyclic", 
    "is_reflexive", 
    "is_symmetric", 
    "is_transitive", 
    "is_a", 
    "intersection_of",              /* new in 1.3 */
    "union_of",                     /* new in 1.3 */
    "disjoint_from",                /* new in 1.3 */
    "inverse_of", 
    "inverse_of_at_instance_level", /* new in 1.3 */
    "transitive_over",
    "relationship",
    "is_obsolete", 
    "replaced_by",                  /* for is_obsolete */
    "consider",                     /* for is_obsolete */
    "is_metadata_tag", 
    "created_by",               /* new in 1.3 */
    "creation_date",            /* new in 1.3 */
    NULL
};



#if 0
static const char *oboInstanceTags[] =
{
    "id", 
    "is_anonymous", 
    "name", 
    "namespace", 
    "alt_id", 
    "comment",
    "synonym",
    "xref",
    "instance_of",
    "property_value",
    "is_obsolete",              /* for is_obsolete */
    "replaced_by",              /* for is_obsolete */
    "consider", 
    NULL
};




static const char *oboAnnotationTags[] =             /* new in 1.3 */
{
    "id", 
    "is_anonymous", 
    "name", 
    "namespace", 
    "alt_id", 
    "comment",
    "subset",
    "synonym",
    "xref",
    "is_a",
    "created_by",
    "creation_date",
    "is_obsolete", 
    "replaced_by",              /* for is_obsolete */
    "consider",                 /* for is_obsolete */
    "subject",
    "relation",
    "is_negated",
    "object",
    "source",
    "assigned_by",
    "evidence",
    "secondary_taxon",          /* order undefined in 1.3 draft */
     NULL
};




static const char *oboFormulaTags[] =      /* new and undocumented in 1.3 */
{
    "id", 
    "is_anonymous", 
    "name", 
    "namespace", 
    "alt_id", 
    "comment",
    "formula",
    NULL
};
#endif /* #if 0 */



static const char *oboHeaderTags[] =
{
    "format-version",
    "data-version",
    "version",
    "date",
    "saved-by",
    "auto-generated-by",
    "import",
    "subsetdef",
    "synonymtypedef",
    "default-namespace",
    "idspace",
    "default-relationship-id-prefix",
    "id-mapping",
    "remark",
    "treat-xrefs-as-equivalent",                        /* new in 1.3 */
    "treat-xrefs-as-genus-differentia",                 /* new in 1.3 */
    "treat-xrefs-as-relationship",                      /* new in 1.3 */
    "treat-xrefs-as-is_a",                              /* new in 1.3 */
    "relax-unique-identifier-assumption-for-namespace", /* new in 1.3 */
    "relax-unique-label-assumption-for-namespace",      /* new in 1.3 */
    "",
    NULL
};




static AjBool     oboInitDone = AJFALSE;
static ajuint     oboErrorCount = 0;

static AjBool oboInit(void);
static AjBool oboCutComment(AjPStr *Pline, AjPStr *Pcomment);
static AjBool oboCutModifier(AjPStr *Pline, AjPStr *Pmodifier);
#if 0
static AjBool oboCutDbxref(AjPStr *Pline, AjPStr *Pdbxref);
#endif
static AjBool oboEscape(AjPStr *Pline);
static void   oboWarn(const AjPFile obofile, ajuint linecnt,
                      const char* fmt, ...);




/* @func ajOboNew *************************************************************
**
** Term constructor
**
** @return [AjPObo] New object
** @@
******************************************************************************/

AjPObo ajOboNew(void)
{
    AjPObo ret;

    AJNEW0(ret);

    return ret;
}




/* @func ajOboTagNew **********************************************************
**
** Tag constructor
**
** @param [r] name [const AjPStr] Name
** @param [r] value [const AjPStr] Value
** @param [r] modifier [const AjPStr] Modifier
** @param [r] comment [const AjPStr] Comment
** @param [r] linenum [ajuint] OBO file line number
** @return [AjPOboTag] New object
** @@
******************************************************************************/

AjPOboTag ajOboTagNew(const AjPStr name, const AjPStr value,
                      const AjPStr modifier, const AjPStr comment,
                      ajuint linenum)
{
    AjPOboTag ret;

    AJNEW0(ret);
    ret->Name  = ajStrNewS(name);
    ret->Value    = ajStrNewS(value);
    ret->Modifier  = ajStrNewS(modifier);
    ret->Comment  = ajStrNewS(comment);
    ret->Linenumber = linenum;

    return ret;
}




/* @func ajOboTermNew *********************************************************
**
** Term constructor
**
** @return [AjPOboTerm] New object
** @@
******************************************************************************/

AjPOboTerm ajOboTermNew(void)
{
    AjPOboTerm ret;

    AJNEW0(ret);

    ret->Taglist = ajListNew();

    return ret;
}




/* @func ajOboTagDel *********************************************************
**
** Tag destructor
**
** @param [d] Ptag       [AjPOboTag*]  Tag object to delete
** @return [void] 
** @@
******************************************************************************/

void ajOboTagDel(AjPOboTag *Ptag)
{
    if(!Ptag)
        ajFatal("Null arg error 1 in ajOboTagDel");
    else if(!(*Ptag))
        ajFatal("Null arg error 2 in ajOboTagDel");

    ajStrDel(&(*Ptag)->Name);
    ajStrDel(&(*Ptag)->Value);
    ajStrDel(&(*Ptag)->Comment);

    AJFREE(*Ptag);
    *Ptag = NULL;

    return;
}




/* @func ajOboTermDel *********************************************************
**
** Term destructor
**
** @param [d] Pterm       [AjPOboTerm*]  Term object to delete
** @return [void] 
** @@
******************************************************************************/

void ajOboTermDel(AjPOboTerm *Pterm)
{
    AjPOboTerm term = NULL;
    AjPOboTag tag = NULL;

    if(!Pterm)
        ajFatal("Null arg error 1 in ajOboTermDel");
    else if(!(*Pterm))
        ajFatal("Null arg error 2 in ajOboTermDel");

    term = *Pterm;

    ajStrDel(&term->Id);
    ajStrDel(&term->Trueid);
    ajStrDel(&term->Name);
    ajStrDel(&term->Namespace);
    ajStrDel(&term->Def);
    ajStrDel(&term->Comment);

    while(ajListPop(term->Taglist, (void**) &tag))
        ajOboTagDel(&tag);

    ajListFree(&term->Taglist);

    AJFREE(*Pterm);
    *Pterm = NULL;

    return;
}




/* @funcstatic oboInit ********************************************************
**
** Initialised the internals for parsing standard OBO format files
**
** @return [AjBool] True on success
**
******************************************************************************/

static AjBool oboInit(void)
{

    if(oboInitDone)
        return ajTrue;

    oboInitDone = ajTrue;

    return ajTrue;
}




/* @func ajOboParseObo ********************************************************
**
** Parse an OBO format file
**
** @param [u] obofile [AjPFile] OBO format input file
** @param [r] validations [const char*] Validations to turn on or off
** @return [AjPObo] Ontology object
******************************************************************************/

AjPObo ajOboParseObo(AjPFile obofile, const char* validations)
{
    AjPObo ret;
    AjPList idprefixlist = NULL;
    ajuint linecnt = 0;
    AjPStr line     = NULL;
    AjPStr saveline = NULL;
    AjBool isstanza   = ajFalse;
    AjBool incomplete = ajFalse;
    AjPStr token = NULL;
    AjPStr rest  = NULL;
    AjPStr stanzatype     = NULL;
    AjPStr laststanzatype = NULL;
    AjPStr id     = NULL;
    AjPStr altid  = NULL;
    AjPStr lastid = NULL;
    AjPStr name   = NULL;
    AjPStr modifier = NULL;
    AjPStr comment  = NULL;
    AjBool ok = ajFalse;
    ajuint i;
    ajuint j;
    ajint icmp;
    ajint icolon;
    AjBool found;
    AjBool isterm = AJFALSE;
    AjPOboTerm term    = NULL;
    AjPOboTerm altterm = NULL;
    AjPOboTerm oldterm = NULL;
    AjPList stanzalist = NULL;
    AjPList oldlist    = NULL;
    AjPOboTag obotag   = NULL;
    ajuint ntypes = 0;
    AjPStr *typeids    = NULL;
    AjPList *typetags  = NULL;
    ajuint nterms = 0;
    AjPStr *termids    = NULL;
    AjPOboTerm *terms  = NULL;
    AjIList iter       = NULL;
    AjIList iterpref   = NULL;
    ajuint s;
    ajint k;
    AjPStr t;
    AjPList u;
    AjPOboTerm uterm;
    ajuint jsave;
    ajuint jj;
    AjPStr idprefix       = NULL;
    AjPStr tmpstr         = NULL;
    AjPStr tmppref        = NULL;
    AjPStrTok idsplit     = NULL;
    AjPStrTok validsplit  = NULL;
    AjPStr validstr       = NULL;
    AjBool obovalid        = ajTrue;
    AjBool obovalididorder = ajTrue;
    AjBool obovalididunk   = ajTrue;
    AjPStr namekey = NULL;
    AjPStr namestr = NULL;
    AjPList namelist = NULL;

    ret = ajOboNew();
    namelist = ajListNew();

    if(validations)
    {
        validstr = ajStrNewC(validations);
        ajStrTokenAssignC(&validsplit, validstr, ",");

        while(ajStrTokenNextParse(&validsplit, &tmpstr))
        {
            if(ajStrMatchC(tmpstr, "none"))
            {
                obovalid = ajFalse;
                obovalididorder = ajFalse;
                obovalididunk = ajFalse;
            }
            else if(ajStrMatchC(tmpstr, "noidorder"))
                obovalididorder = ajFalse;
            else if(ajStrMatchC(tmpstr, "nounkid"))
                obovalididunk = ajFalse;
            else
                ajWarn("ajOboParseObo called with unknown validation '%S'",
                       tmpstr);
        }
        
    }
    
    ret->Termtable = ajTablestrNewLen(35000);
    ret->Termnametable = ajTablestrNewLen(35000);
    ret->Typedeftable = ajTablestrNew();
    ret->Instancetable = ajTablestrNew();

    idprefixlist = ajListstrNew();

    oboInit();

    while(ajReadlineTrim(obofile, &line))
    {
        linecnt++;

        oboCutComment(&line, &comment);
        oboEscape(&line);

        if(ajStrGetLen(line) && isspace((int) ajStrGetCharLast(line)))
           oboWarn(obofile, linecnt,
                   "trailing whitespace on line");

        if((ajStrGetCharLast(line) == '\\') &&
           (ajStrGetCharPos(line, -2) != '\\'))
        {
            incomplete = ajTrue;
            ajStrAppendS(&saveline, line);
            continue;
        }
        
        if(incomplete)          /* continued line */
        {
            ajStrAppendS(&saveline, line);
            ajStrAssignS(&line, saveline);
            ajStrAssignC(&saveline, "");
            incomplete = ajFalse;
        }

        if (!ajStrGetLen(line)) /* empty line */
        {
            continue;
        }
        else if(ajStrGetCharFirst(line) == '[')
        {
            if(!isstanza) 
            {
                /* validate header */
                isstanza = ajTrue;
            }
            else
            {
                if(stanzalist)
                {
                    if(isterm)
                    {
                        ajListPushlist(term->Taglist, &stanzalist);
                        oldterm = ajTablePut(ret->Termtable, id, term);
                        if(oldterm)
                        {
                            oboWarn(obofile, linecnt,
                                    "Duplicate %S id '%S'",
                                    stanzatype, id);
                            oldterm = NULL;
                        }

                        while(ajListGetLength(namelist))
                        {
                            namekey = NULL;
                            ajListstrPop(namelist, &namestr);
                            if(ajStrGetLen(term->Namespace))
                                ajFmtPrintS(&namekey, "%S:%S",
                                            term->Namespace,
                                            namestr);
                            else
                                namekey = ajStrNewS(namestr);
                            if(term->Obsolete)
                                ajStrAppendC(&namekey, " (obsolete)");
                            oldterm = ajTablePut(ret->Termnametable,
                                                 namekey, term);
                            if(oldterm)
                            {
                                oboWarn(obofile, linecnt,
                                        "%S Name/Synonym %S duplicates '%S'",
                                        term->Id, namekey, oldterm->Id);
                                oldterm = NULL;
                            }
                        }
                     }
                    else
                    {
                        if(ajStrMatchC(stanzatype, "Typedef"))
                            oldlist = ajTablePut(ret->Typedeftable,
                                                 id, stanzalist);
                        else if(ajStrMatchC(stanzatype, "Instance"))
                            oldlist = ajTablePut(ret->Instancetable,
                                                 id, stanzalist);
                        else if(ajStrMatchC(stanzatype, "Annotation"))
                            oldlist = ajTablePut(ret->Annotable,
                                                 id, stanzalist);
                        else if(ajStrMatchC(stanzatype, "Formula"))
                            oldlist = ajTablePut(ret->Formulatable,
                                                 id, stanzalist);
                        else 
                            oldlist = ajTablePut(ret->Misctable,
                                                 id, stanzalist);
                        if(oldlist)
                        {
                            oboWarn(obofile, linecnt,
                                    "Duplicate %S id '%S'",
                                    stanzatype, id);
                            oldlist = NULL;
                        }
                    }
                }
                if(linecnt > 50000) break;
                
            }

            id = NULL;
            stanzalist = ajListNew();
            
            ajStrExtractWord(line, &rest, &token);
            if(ajStrGetLen(rest))
                oboWarn(obofile, linecnt,
                        "unexpected characters in stanza line: '%S'",
                        rest);
            if(ajStrGetCharLast(token) != ']')
                oboWarn(obofile, linecnt,
                        "unexpected characters in stanza type: '%S'",
                        token);
            ajStrAssignS(&laststanzatype, stanzatype);
            ajStrAssignS(&stanzatype, token);
            ajStrCutEnd(&stanzatype,1);
            ajStrCutStart(&stanzatype,1);

            if(ajStrMatchC(stanzatype, "Term"))
            {
                isterm = ajTrue;
                term = ajOboTermNew();
            }
            else
                isterm = ajFalse;

            if(ajStrGetLen(laststanzatype) &&
               !ajStrMatchS(laststanzatype, stanzatype))
            {
                ok = ajFalse;
                for(i=0; oboStanza[i]; i++)
                {
                    if(ajStrMatchC(laststanzatype, oboStanza[i]))
                    {
                        for(j=i+1; oboStanza[j]; j++)
                        {
                            if(ajStrMatchC(stanzatype, oboStanza[j]))
                            {
                                ok = ajTrue;
                                break;
                            }
                        }
                        break;
                    }
                }
                if(!ok && obovalid)
                    oboWarn(obofile, linecnt,
                            "stanza '%S' after '%S'",
                            stanzatype, laststanzatype);
            }
            
            
            ok = ajFalse;
            for(i=0;oboStanza[i];i++)
            {
                if(ajStrMatchC(stanzatype, oboStanza[i]))
                    ok = ajTrue;
            }
            if(!ok)
                oboWarn(obofile, linecnt,
                        "unknown stanza type '%S'",
                        token);
        }
        else 
        {
            if(oboCutModifier(&line, &modifier))
                oboWarn(obofile, linecnt,
                        "modifier found '%S'",
                        modifier);

            /* parse name: value ! comment */
            ajStrExtractWord(line, &rest, &token);

            if(!ajStrGetLen(rest))
                oboWarn(obofile, linecnt,
                        "missing value for tag '%S' in %S '%S'",
                        token, stanzatype, id);
            ajStrAssignS(&name, token);

            if(ajStrGetCharLast(name) != ':')
                oboWarn(obofile, linecnt,
                        "bad name '%S'",
                        token);
            ajStrCutEnd(&name, 1);

            if(ajStrMatchC(name, "id"))
            {
                ajStrAssignS(&id, rest);
                icolon = ajStrFindAnyK(id, ':');
                if(icolon > 0)
                {
                    ajStrAssignSubS(&tmpstr, id, 0, icolon);

                    if(!ajStrMatchS(idprefix, tmpstr))
                    {
                        iterpref = ajListIterNew(idprefixlist);
                        found = ajFalse;

                        while(!ajListIterDone(iterpref))
                        {
                            tmppref = ajListstrIterGet(iterpref);

                            if(ajStrMatchS(tmppref, tmpstr))
                            {
                                found = ajTrue;
                                break;
                            }
                        }

                        ajListIterDel(&iterpref);

                        if(!found)
                        {
                            ajStrAssignS(&idprefix, tmpstr);
                            ajListstrPush(idprefixlist, tmpstr);
                            tmpstr = NULL;
                        }
                    }
                }
                
                if(ajStrGetLen(lastid))
                {
                    if(ajStrMatchS(stanzatype, laststanzatype))
                    {
                        icmp = MAJSTRCMPS(id, lastid);

                        if(!icmp)
                            oboWarn(obofile, linecnt,
                                    "duplicate of previous ID '%S'",
                                    id);
                        else if(obovalididorder && (icmp < 0))
                            oboWarn(obofile, linecnt,
                                    "ID out of order '%S' after '%S",
                                    id, lastid);
                    }
                }
                ajStrAssignS(&lastid, id);
            }

            if(isstanza)
            {
                obotag = ajOboTagNew(name, rest, modifier, comment, linecnt);
                ajListPushAppend(stanzalist, obotag);
                obotag = NULL;

                if(isterm)
                {
                    if(ajStrMatchC(name, "id"))
                    {
                        if(term->Id)
                            oboWarn(obofile, linecnt,
                                    "Multiple '%S' tags for ID '%S'",
                                    name, id);
                        ajStrAssignS(&term->Id, rest);
                    }
                    else if(ajStrMatchC(name, "name"))
                    {
                        if(term->Name)
                            oboWarn(obofile, linecnt,
                                    "Multiple '%S' tags for ID '%S'",
                                    name, id);
                        ajStrAssignS(&term->Name, rest);
                        namekey = ajStrNewS(rest);
                        ajListstrPushAppend(namelist, namekey);
                    }
                    else if(ajStrMatchC(name, "synonym"))
                    {
                        if(ajStrGetCharFirst(rest) == '"')
                        {
                            i = ajStrFindlastK(rest, '"');
                            if(i > 1)
                            {
                                namekey = ajStrNewRes(i);
                                ajStrAssignSubS(&namekey, rest, 1, i-1);
                                ajListstrPushAppend(namelist, namekey);
                            }
                        }
                    }
                    else if(ajStrMatchC(name, "namespace"))
                    {
                        if(term->Namespace)
                            oboWarn(obofile, linecnt,
                                    "Multiple '%S' tags for ID '%S'",
                                    name, id);
                        ajStrAssignS(&term->Namespace, rest);
                    }
                    else if(ajStrMatchC(name, "def"))
                    {
                        if(term->Def)
                            oboWarn(obofile, linecnt,
                                    "Multiple '%S' tags for ID '%S'",
                                    name, id);
                        ajStrAssignS(&term->Def, rest);
                    }
                    else if(ajStrMatchC(name, "comment"))
                    {
                        if(term->Comment)
                            oboWarn(obofile, linecnt,
                                    "Multiple '%S' tags for ID '%S'",
                                    name, id);
                        ajStrAssignS(&term->Comment, rest);
                    }
                    else if(ajStrMatchC(name, "is_obsolete"))
                    {
                        if(term->Obsolete)
                            oboWarn(obofile, linecnt,
                                    "Multiple '%S' tags for ID '%S'",
                                    name, id);
                        if(ajStrPrefixC(rest, "true"))
                            term->Obsolete = ajTrue;
                        else if(ajStrPrefixC(rest, "false"))
                            term->Obsolete = ajFalse;
                        else
                            oboWarn(obofile, linecnt,
                                    "Bad value '%S' boolean tag for ID '%S'",
                                    name, id);
                    }
                    else if(ajStrMatchC(name, "builtin"))
                    {
                        if(term->Obsolete)
                            oboWarn(obofile, linecnt,
                                    "Multiple '%S' tags for ID '%S'",
                                    name, id);
                        if(ajStrPrefixC(rest, "true"))
                            term->Builtin = ajTrue;
                        else if(ajStrPrefixC(rest, "false"))
                            term->Builtin = ajFalse;
                        else
                            oboWarn(obofile, linecnt,
                                    "Bad value '%S' boolean tag for ID '%S'",
                                    name, id);
                    }
                    else if(ajStrMatchC(name, "alt_id"))
                    {
                        altterm = ajOboTermNew();
                        ajStrAssignS(&altterm->Id, rest);
                        ajStrAssignS(&altterm->Trueid, id);
                        altid = ajStrNewS(rest);
                        oldterm = ajTablePut(ret->Termtable, altid, altterm);
                        altid = NULL;

                        if(oldterm)
                        {
                            oboWarn(obofile, linecnt,
                                    "Duplicate %S alt_id '%S'",
                                    stanzatype, id);
                            oldterm = NULL;
                        }
                    }
                }
            }
            else                /* processing header tags */
            {
                ok =  ajFalse;

                for(i=0;oboHeaderTags[i];i++)
                {
                    if(ajStrMatchC(name, oboHeaderTags[i]))
                    {
                        ok = ajTrue;
                        break;
                    }
                }

                if(!ok)
                    oboWarn(obofile, linecnt,
                            "unknown header name '%S'",
                            token);
            }

            ajStrTrimWhite(&rest);
        }
    }

    if(stanzalist)
    {
        if(isterm)
        {
            if(!term->Obsolete)
            {
                ajListPushlist(term->Taglist, &stanzalist);
                oldterm = ajTablePut(ret->Termtable, id, term);            

                if(oldterm)
                {
                    oboWarn(obofile, linecnt,
                            "Duplicate %S id '%S'",
                            stanzatype, id);
                    ajOboTermDel(&oldterm);
                }

                while(ajListGetLength(namelist))
                {
                    namekey = NULL;
                    ajListstrPop(namelist, &namestr);
                    if(ajStrGetLen(term->Namespace))
                        ajFmtPrintS(&namekey, "%S:%S",
                                    term->Namespace,
                                    namestr);
                    else
                        namekey = ajStrNewS(namestr);
                    if(term->Obsolete)
                        ajStrAppendC(&namekey, " (obsolete)");
                            
                    oldterm = ajTablePut(ret->Termnametable,
                                         namekey, term);
                    if(oldterm)
                    {
                        oboWarn(obofile, linecnt,
                                "%S Name/Synonym %S duplicates '%S'",
                                term->Id, namekey, oldterm->Id);
                        oldterm = NULL;
                    }
                }

                term = NULL;
            }
        }
        else 
        {
            if(ajStrMatchC(stanzatype, "Typedef"))
                oldlist = ajTablePut(ret->Typedeftable, id, stanzalist);
            else if(ajStrMatchC(stanzatype, "Instance"))
                oldlist = ajTablePut(ret->Instancetable, id, stanzalist);
            else if(ajStrMatchC(stanzatype, "Annotation"))
                oldlist = ajTablePut(ret->Annotable, id, stanzalist);
            else if(ajStrMatchC(stanzatype, "Formula"))
                oldlist = ajTablePut(ret->Formulatable, id, stanzalist);
            else 
                oldlist = ajTablePut(ret->Misctable, id, stanzalist);

            if(oldlist)
            {
                oboWarn(obofile, linecnt,
                        "Duplicate %S id '%S'",
                        stanzatype, id);
                oldlist = NULL;
            }
        }
    }

    id = NULL;
    stanzalist = ajListNew();

    ajDebug("typedefs:%u terms:%u instances:%u annotations:%u formulas:%u "
           "other:%u\n",
            ajTableGetLength(ret->Typedeftable),
            ajTableGetLength(ret->Termtable),
            ajTableGetLength(ret->Instancetable),
            ajTableGetLength(ret->Annotable),
            ajTableGetLength(ret->Formulatable),
            ajTableGetLength(ret->Misctable));

    /*
    ** Typedefs first
    ** fetch and sort them
    */

    ntypes = ajTableToarrayKeysValues(ret->Typedeftable,
                                      (void***) &typeids,
                                      (void***) &typetags);
    for(s=ntypes/2; s>0; s /= 2)
	for(i=s; i<ntypes; ++i)
	    for(k=i-s;k>=0 && MAJSTRCMPS(typeids[k],typeids[k+s])>0; k-=s)
	    {
		t = typeids[k];
		typeids[k] = typeids[k+s];
		typeids[k+s] = t;

		u = typetags[k];
		typetags[k] = typetags[k+s];
		typetags[k+s] = u;
	    }

    for(i=0; i<ntypes; i++)
    {
        id = typeids[i];
        iter = ajListIterNewread(typetags[i]);
        j = 0;
        jsave = 0;

        while(!ajListIterDone(iter))
        {
            obotag = ajListIterGet(iter);

            while(oboTypedefTags[j] &&
                  !ajStrMatchC(obotag->Name, oboTypedefTags[j]))
                j++;

            if(oboTypedefTags[j])
                jsave = j;
            else
            {
                for(jj=0; jj < jsave; jj++)
                {
                    if(obovalididorder &&
                       ajStrMatchC(obotag->Name, oboTypedefTags[jj]))
                    {
                        ajUser("type tag out of order '%S' after '%s' "
                               "for id '%S'",
                               obotag->Name, oboTypedefTags[jsave], id);
                        j = jj;
                        break;
                    }
                }
                
            }
            /* check for IDs */
            ajStrTokenAssignC(&idsplit, obotag->Value, " \t,;[]{}()'\"");

            while (ajStrTokenNextParse(&idsplit, &tmpstr))
            {
                if(ajStrPrefixS(tmpstr, idprefix))
                    if(obovalididunk && !ajTableFetch(ret->Termtable, tmpstr))
                        oboWarn(obofile, obotag->Linenumber,
                                "Unknown id '%S' for tag '%S' in typedef '%S'",
                                tmpstr, obotag->Name, id);
            }
            
        }
    }

    /*
    ** Validate terms
    */

    nterms = ajTableToarrayKeysValues(ret->Termtable,
                                      (void***) &termids,
                                      (void***) &terms);
    for(s=nterms/2; s>0; s /= 2)
	for(i=s; i<nterms; ++i)
	    for(k=i-s;k>=0 && MAJSTRCMPS(termids[k],termids[k+s])>0; k-=s)
	    {
		t = termids[k];
		termids[k] = termids[k+s];
		termids[k+s] = t;

		uterm = terms[k];
		terms[k] = terms[k+s];
		terms[k+s] = uterm;
	    }

    for(i=0; i<nterms; i++)
    {
        id = termids[i];

        if(ajStrGetLen(terms[i]->Trueid))
        {
            ajStrTokenAssignC(&idsplit, terms[i]->Trueid, " \t,;[]{}()'\"");

            while (ajStrTokenNextParse(&idsplit, &tmpstr))
            {
                ajStrTrimEndC(&tmpstr, ".");

                if(ajStrPrefixS(tmpstr, idprefix))
                    if(obovalididunk && !ajTableFetch(ret->Termtable, tmpstr))
                        oboWarn(obofile, 0,
                                "Unknown id '%S' for tag alt_id in term '%S'",
                                tmpstr, terms[i]->Trueid);
            }

            continue;
        }

        /* valid values for major tags*/

        if(!terms[i]->Name)
        {
            oboWarn(obofile, 0,
                    "no 'name' for term '%S' ... OWL data only  in OBO 1.3",
                    id);
        }
        
        if(terms[i]->Def)
        {
            if(ajStrGetCharFirst(terms[i]->Def) != '"')
                oboWarn(obofile, obotag->Linenumber,
                        "bad 'def' tag value for term '%S'",
                        id);
        }
        
        iter = ajListIterNewread(terms[i]->Taglist);
        j = 0;
        jj = 0;
        jsave = 0;

        while(!ajListIterDone(iter))
        {
            obotag = ajListIterGet(iter);

            j = 0;

            while(oboTermTags[j].Tag &&
                  !ajStrMatchC(obotag->Name, oboTermTags[j].Tag))
                j++;

            if(oboTermTags[j].Tag)
                jsave = j;
            else
            {
                jj = 0;

                while(typeids[jj] &&
                      !ajStrMatchS(obotag->Name, typeids[jj]))
                    jj++;

                if(!typeids[jj])
                    oboWarn(obofile, obotag->Linenumber,
                            "term tag not found '%S' for id '%S'",
                            obotag->Name, id);
            }
            /* check for IDs */
            ajStrTokenAssignC(&idsplit, obotag->Value, " \t,;[]{}()'\"");

            while (ajStrTokenNextParse(&idsplit, &tmpstr))
            {
                ajStrTrimEndC(&tmpstr, ".");

                if(ajStrPrefixS(tmpstr, idprefix))
                    if(obovalididunk && !ajTableFetch(ret->Termtable, tmpstr))
                        oboWarn(obofile, obotag->Linenumber,
                                "Unknown id '%S' for tag '%S' in term '%S'",
                                tmpstr, obotag->Name, id);
            }
            
        }
    }

    ajDebug("%u id prefixes current '%S'\n",
            ajListGetLength(idprefixlist), idprefix);

    iterpref = ajListIterNew(idprefixlist);
    found = ajFalse;

    while(!ajListIterDone(iterpref))
    {
        tmppref = ajListstrIterGet(iterpref);
        ajDebug("   '%S'\n", tmppref);
    }

    ajListIterDel(&iterpref);

    return ret; 
}




/* @funcstatic oboEscape ******************************************************
**
** Clean up escaped characters
**
** @param [u] Pline [AjPStr*] Line with possible comment text
** @return [AjBool] True if an escaped character was found
******************************************************************************/

static AjBool oboEscape(AjPStr *Pline)
{
    const char* cp;
    ajuint ilen = 0;

    AjPStr tmpstr = NULL;
    char lastchar = '\0';

    AjBool ret = ajFalse;

    ilen = ajStrGetLen(*Pline);
    cp = ajStrGetPtr(*Pline);
    tmpstr = ajStrNewRes(ilen+1);

    while (*cp)
    {
        if(lastchar == '\\')
        {
            ret = ajTrue;

            switch (*cp)
            {
                case '\"':      /* keep it for now */
                    ajStrAppendK(&tmpstr, '\\');
                    ajStrAppendK(&tmpstr, *cp);
                    break;
                case 'n':
                    ajStrAppendK(&tmpstr, '\n');
                    break;
                case 't':
                    ajStrAppendK(&tmpstr, '\t');
                    break;
                case 'W':
                    ajStrAppendK(&tmpstr, ' ');
                    break;
                default:
                    ajStrAppendK(&tmpstr, *cp);
                    break;
            }
            
        }
        else if(*cp != '\\')
            ajStrAppendK(&tmpstr, *cp);

        lastchar = *cp++;
    }

    if(lastchar == '\\')        /* \ at the end means ignore the newline */
        ret = ajTrue;

    if(ret)
        ajStrAssignS(Pline, tmpstr);

    return ret;
}




/* @funcstatic oboCutComment **************************************************
**
** Remove comments starting with an unescaped '!'
**
** @param [u] Pline [AjPStr*] Line with possible comment text
** @param [u] Pcomment [AjPStr*] Comment text
** @return [AjBool] True if a comment was found
******************************************************************************/

static AjBool oboCutComment(AjPStr *Pline, AjPStr *Pcomment)
{
    const char* cp;
    ajuint i      = 0;
    ajuint spaces = 0;
    ajuint ilen   = 0;
    AjBool doescape = ajFalse;

    ilen = ajStrGetLen(*Pline);
    cp = ajStrGetPtr(*Pline);

    while (*cp)
    {
        if(*cp == '!')
        {
            if(!doescape)
            {
                if((i+1) < ilen)
                {
                    ajStrAssignSubS(Pcomment, *Pline, i+1, -1);
                    ajStrTrimWhite(Pcomment);
                }
                else
                    ajStrAssignC(Pcomment, "");

                ajStrCutEnd(Pline, ilen-i+spaces);

                return ajTrue;
            }
        }
        
        if(*cp == '\\')
            doescape = ajTrue;
        else
        {
            doescape = ajFalse;

            if(isspace(*cp))
                spaces++;
            else
                spaces = 0;
        }
        i++;
        cp++;
    }
    
    ajStrAssignC(Pcomment, "");

    return ajFalse;
}




#if 0
/* @funcstatic oboCutDbxref *************************************************
**
** Remove trailing dbxrefs in square braces
**
** @param [u] Pline [AjPStr*] Line with possible trailing dbxref text
** @param [u] Pdbxref [AjPStr*] Dbxref text
** @return [AjBool] True if a trailing dbxref was found
******************************************************************************/

static AjBool oboCutDbxref(AjPStr *Pline, AjPStr *Pdbxref)
{
    ajint i = 0;
    ajuint ilen;

    ilen = ajStrGetLen(*Pline);

    if(ajStrGetCharLast(*Pline) != ']')
        return ajFalse;

    i = ajStrFindlastK(*Pline, '[');
    if(i < 0)
    {
        ajUser("Failed to complete trailing dbxref: '%S'",
               *Pline);
        return ajFalse;
    }

    ajStrAssignSubS(Pdbxref, *Pline, i, -1);
    ajStrTrimWhite(Pdbxref);

    if(i)
    {
        ajStrCutEnd(Pline, ilen-i);
        ajStrTrimWhite(Pline);
    }
    else
        ajStrAssignC(Pline, "");
 
    return ajTrue;
}
#endif




/* @funcstatic oboCutModifier *************************************************
**
** Remove trailing modifiers in curly braces
**
** @param [u] Pline [AjPStr*] Line with possible trailing modifier text
** @param [u] Pmodifier [AjPStr*] Modifier text
** @return [AjBool] True if a trailing modifier was found
******************************************************************************/

static AjBool oboCutModifier(AjPStr *Pline, AjPStr *Pmodifier)
{
    ajint i = 0;
    ajuint ilen = 0;

    ilen = ajStrGetLen(*Pline);

    if(ajStrGetCharLast(*Pline) != '}')
        return ajFalse;

    i = ajStrFindlastK(*Pline, '{');

    if(i < 0)
    {
        ajUser("Failed to complete trailing modifier: '%S'",
               *Pline);
        return ajFalse;
    }

    ajStrAssignSubS(Pmodifier, *Pline, i, -1);
    ajStrTrimWhite(Pmodifier);

    if(i)
    {
        ajStrCutEnd(Pline, ilen-i);
        ajStrTrimWhite(Pline);
    }
    else
        ajStrAssignC(Pline, "");
 
    return ajTrue;
}




/* @funcstatic oboWarn *******************************************************
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

static void oboWarn(const AjPFile obofile, ajuint linecnt, const char* fmt, ...)
{
    va_list args;
    AjPStr errstr = NULL;

    oboErrorCount++;

    va_start(args, fmt) ;
    ajFmtVPrintS(&errstr, fmt, args);
    va_end(args) ;

    ajWarn("File %F line %u: %S", obofile, linecnt, errstr);
    ajStrDel(&errstr);

    return;
}




/* @func ajOboFetchTerm *******************************************************
**
** Retrieves an OBO term
**
** @param [r] thys [const AjPObo] Parsed ontology
** @param [r] query [const AjPStr] OBO identifier or name
** @return [AjPOboTerm] OBO term
******************************************************************************/

AjPOboTerm ajOboFetchTerm(const AjPObo thys, const AjPStr query)
{
    AjPOboTerm ret;
    ajuint irecurs = 0;

    if(!thys)
        return NULL;

    ret = ajTableFetch(thys->Termtable, query);

    while(ret && ajStrGetLen(ret->Trueid))
    {
        if(++irecurs > 256)
            return NULL;

        ret = ajTableFetch(thys->Termtable, ret->Trueid);
    }

    return ret;
}




/* @func ajOboFetchName *******************************************************
**
** Retrieves an OBO term by name
**
** The name must be an exact match to a name or synonym for the term
**
** @param [r] thys [const AjPObo] Parsed ontology
** @param [r] query [const AjPStr] OBO identifier
** @return [AjPOboTerm] OBO term
******************************************************************************/

AjPOboTerm ajOboFetchName(const AjPObo thys, const AjPStr query)
{
    AjPOboTerm ret;
    ajuint irecurs = 0;

    if(!thys)
        return NULL;

    ret = ajTableFetch(thys->Termnametable, query);

    while(ret && ajStrGetLen(ret->Trueid))
    {
        if(++irecurs > 256)
            return NULL;

        ret = ajTableFetch(thys->Termtable, ret->Trueid);
    }

    return ret;
}
