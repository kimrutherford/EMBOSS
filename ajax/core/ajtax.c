/******************************************************************************
** @Source AJAX NCBI Taxonomy handling functions
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




static AjPTable taxidtable = NULL;
static AjPTable taxnametable = NULL;
static AjPTable taxNameclassTable = NULL;
static AjPTable taxRankTable = NULL;
static AjPTable taxEmblcodeTable = NULL;

#define TAXFLAG_INHERITDIV    0x01
#define TAXFLAG_INHERITCODE   0x02
#define TAXFLAG_INHERITMITO   0x04
#define TAXFLAG_HIDDENGENBANK 0x10
#define TAXFLAG_HIDDENSUBTREE 0x20


static ajint taxTableCmp(const void *x, const void *y);
static ajuint taxTableHash(const void *key, ajuint hashsize);




/* @func ajTaxNew **********************************************************
**
** Taxonomy node constructor
**
** @return [AjPTax] New object
** @@
******************************************************************************/

AjPTax ajTaxNew(void)
{
    AjPTax ret;

    AJNEW0(ret);

    ret->Namelist = ajListNew();
    ret->Citations = ajListNew();

    return ret;
}




/* @func ajTaxCitNew **********************************************************
**
** Taxonomy citation constructor
**
** @return [AjPTaxCit] New object
** @@
******************************************************************************/

AjPTaxCit ajTaxCitNew(void)
{
    AjPTaxCit ret;

    AJNEW0(ret);

    return ret;
}




/* @func ajTaxCodeNew *********************************************************
**
** Taxonomy genetic code constructor
**
** @return [AjPTaxCode] New object
** @@
******************************************************************************/

AjPTaxCode ajTaxCodeNew(void)
{
    AjPTaxCode ret;

    AJNEW0(ret);

    return ret;
}




/* @func ajTaxDelNew **********************************************************
**
** Taxonomy deleted id constructor
**
** @return [AjPTaxDel] New object
** @@
******************************************************************************/

AjPTaxDel ajTaxDelNew(void)
{
    AjPTaxDel ret;

    AJNEW0(ret);

    return ret;
}




/* @func ajTaxDivNew **********************************************************
**
** Taxonomy division constructor
**
** @return [AjPTaxDiv] New object
** @@
******************************************************************************/

AjPTaxDiv ajTaxDivNew(void)
{
    AjPTaxDiv ret;

    AJNEW0(ret);

    return ret;
}




/* @func ajTaxMergeNew ********************************************************
**
** Taxonomy merged id  constructor
**
** @return [AjPTaxMerge] New object
** @@
******************************************************************************/

AjPTaxMerge ajTaxMergeNew(void)
{
    AjPTaxMerge ret;

    AJNEW0(ret);

    return ret;
}




/* @func ajTaxNameNew **********************************************************
**
** Taxonomy names constructor
**
** @return [AjPTaxName] New object
** @@
******************************************************************************/

AjPTaxName ajTaxNameNew(void)
{
    AjPTaxName ret;

    AJNEW0(ret);

    return ret;
}




/* @func ajTaxDel *********************************************************
**
** Taxonomy node destructor
**
** @param [d] Ptax [AjPTax*]  Taxonomy node object to delete
** @return [void] 
** @@
******************************************************************************/

void ajTaxDel(AjPTax *Ptax)
{
    if(!Ptax)
        ajFatal("Null arg error 1 in ajTaxDel");
    else if(!(*Ptax))
        ajFatal("Null arg error 2 in ajTaxDel");

/*
    ajStrDel(&(*Ptax)->Name);
*/

    AJFREE(*Ptax);
    *Ptax = NULL;

    return;
}




/* @func ajTaxParse ***********************************************************
**
** Parse an NCBI Taxonomy file
**
** @param [u] taxfile [AjPFile] NCBI Taxonomy format input file
** @return [AjBool] True on success
******************************************************************************/

AjBool ajTaxParse(AjPFile taxfile)
{
    AjPStr line = NULL;
    ajuint linecnt = 0;

    while(ajReadlineTrim(taxfile, &line))
        linecnt++;

    return ajTrue;
}




/* @func ajTaxLoad ************************************************************
**
** Parse an NCBI Taxonomy database
**
** @param [r] taxdir [const AjPDir] NCBI Taxonomy database directory
** @return [AjBool] True on success
**
******************************************************************************/

AjBool ajTaxLoad(const AjPDir taxdir)
{
    AjPFile infile;
    AjPStr line    = NULL;
    AjPStr tmpstr  = NULL;
    ajint i        = 0;
    ajint lasti    = 0;
    ajuint linecnt = 0;
    AjPTax oldtax = NULL;
    AjPTax tax    = NULL;
    AjPTaxDiv taxdiv     = NULL;
    AjPTaxName taxname   = NULL;
    AjPTaxCode taxcode   = NULL;
    AjPTaxDel taxdel     = NULL;
    AjPTaxMerge taxmerge = NULL;
    AjPTaxCit taxcit = NULL;
    ajuint nfield = 0;
    ajlong taxid = 0;
    AjPStr idstr = NULL;
    ajuint tmpid = 0;
    AjPStr tablestr = NULL;
    AjPStr tmpkey = NULL;
    AjPStr tmpval = NULL;
    ajlong *ptaxid;

    taxidtable = ajTableNewFunctionLen(600000, taxTableCmp, taxTableHash);
    taxnametable = ajTablestrNewLen(900000);
    taxRankTable = ajTablestrNewLen(20);
    taxNameclassTable = ajTablestrNewLen(20);
    taxEmblcodeTable = ajTablestrNewLen(20);

    infile = ajFileNewInNamePathC("nodes.dmp", ajDirGetPath(taxdir));
    linecnt = 0;

    while(ajReadlineTrim(infile, &line))
    {
        linecnt++;
        nfield = 0;
        lasti = 0;

        /* files have tab | tab as delimiters */

        tax = ajTaxNew();

        i = ajStrFindNextC(line, lasti, "\t|\t");

        if(i<0)
        {
            ajWarn("%F line %u: only %u fields",
                      infile, linecnt, nfield);
            continue;
        }

        ajStrAssignSubS(&tmpstr, line, lasti, i-1);

        if(!ajStrToUint(tmpstr, &tax->Taxid))
            ajWarn("%F line %u: invalid taxid '%S'",
                   infile, linecnt, tmpstr);

        lasti = i+3;
        nfield++;

        i = ajStrFindNextC(line, lasti, "\t|\t");

        if(i<0)
        {
            ajWarn("%F line %u: only %u fields",
                      infile, linecnt, nfield);
            continue;
        }

        ajStrAssignSubS(&tmpstr, line, lasti, i-1);

        if(!ajStrToUint(tmpstr, &tax->Parent))
            ajWarn("%F line %u: invalid parent '%S'",
                   infile, linecnt, tmpstr);

        lasti = i+3;
        nfield++;

        i = ajStrFindNextC(line, lasti, "\t|\t");

        if(i<0)
        {
            ajWarn("%F line %u: only %u fields",
                      infile, linecnt, nfield);
            continue;
        }

        ajStrAssignSubS(&tmpstr, line, lasti, i-1);
        tablestr = ajTableFetch(taxRankTable, tmpstr);

        if(!tablestr)
        {
            tmpkey = ajStrNewS(tmpstr);
            tmpval = ajStrNewS(tmpstr);
            ajTablePut(taxRankTable, tmpkey, tmpstr);
            tax->Rank = ajStrNewRef(tmpval);
        }
        else
            tax->Rank = ajStrNewRef(tablestr);
        
        lasti = i+3;
        nfield++;

        i = ajStrFindNextC(line, lasti, "\t|\t");

        if(i<0)
        {
            ajWarn("%F line %u: only %u fields",
                      infile, linecnt, nfield);
            continue;
        }

        if(i > lasti)
        {
            ajStrAssignSubS(&tmpstr, line, lasti, i-1);
            tablestr = ajTableFetch(taxEmblcodeTable, tmpstr);

            if(!tablestr)
            {
                tmpkey = ajStrNewS(tmpstr);
                tmpval = ajStrNewS(tmpstr);
                ajTablePut(taxEmblcodeTable, tmpkey, tmpstr);
                tax->Emblcode = ajStrNewRef(tmpval);
            }
            else
                tax->Emblcode = ajStrNewRef(tablestr);
        }

        lasti = i+3;
        nfield++;

        i = ajStrFindNextC(line, lasti, "\t|\t");

        if(i<0)
        {
            ajWarn("%F line %u: only %u fields",
                      infile, linecnt, nfield);
            continue;
        }

        ajStrAssignSubS(&tmpstr, line, lasti, i-1);

        if(!ajStrToUint(tmpstr, &tmpid))
            ajWarn("%F line %u: invalid division id '%S'",
                   infile, linecnt, tmpstr);
        else
            tax->Divid = tmpid;

        lasti = i+3;
        nfield++;

        i = ajStrFindNextC(line, lasti, "\t|\t");

        if(i<0)
        {
            ajWarn("%F line %u: only %u fields",
                      infile, linecnt, nfield);
            continue;
        }

        ajStrAssignSubS(&tmpstr, line, lasti, i-1);

        if(ajStrMatchC(tmpstr, "1"))
            tax->Flags |= TAXFLAG_INHERITDIV;

        lasti = i+3;
        nfield++;

        i = ajStrFindNextC(line, lasti, "\t|\t");

        if(i<0)
        {
            ajWarn("%F line %u: only %u fields",
                      infile, linecnt, nfield);
            continue;
        }

        ajStrAssignSubS(&tmpstr, line, lasti, i-1);

        if(!ajStrToUint(tmpstr, &tmpid))
            ajWarn("%F line %u: invalid gencode '%S'",
                   infile, linecnt, tmpstr);
        else
            tax->Gencode = tmpid;

        lasti = i+3;
        nfield++;

        i = ajStrFindNextC(line, lasti, "\t|\t");

        if(i<0)
        {
            ajWarn("%F line %u: only %u fields",
                      infile, linecnt, nfield);
            continue;
        }

        ajStrAssignSubS(&tmpstr, line, lasti, i-1);

        if(ajStrMatchC(tmpstr, "1"))
            tax->Flags |= TAXFLAG_INHERITCODE;

        lasti = i+3;
        nfield++;

        i = ajStrFindNextC(line, lasti, "\t|\t");

        if(i<0)
        {
            ajWarn("%F line %u: only %u fields",
                      infile, linecnt, nfield);
            continue;
        }

        ajStrAssignSubS(&tmpstr, line, lasti, i-1);

        if(!ajStrToUint(tmpstr, &tmpid))
            ajWarn("%F line %u: invalid mitocode '%S'",
                   infile, linecnt, tmpstr);
        else
            tax->Mitocode = tmpid;

        lasti = i+3;
        nfield++;

        i = ajStrFindNextC(line, lasti, "\t|\t");

        if(i<0)
        {
            ajWarn("%F line %u: only %u fields",
                      infile, linecnt, nfield);
            continue;
        }

        ajStrAssignSubS(&tmpstr, line, lasti, i-1);

        if(ajStrMatchC(tmpstr, "1"))
            tax->Flags |= TAXFLAG_INHERITMITO;

        lasti = i+3;
        nfield++;

        i = ajStrFindNextC(line, lasti, "\t|\t");

        if(i<0)
        {
            ajWarn("%F line %u: only %u fields",
                      infile, linecnt, nfield);
            continue;
        }

        ajStrAssignSubS(&tmpstr, line, lasti, i-1);

        if(ajStrMatchC(tmpstr, "1"))
            tax->Flags |= TAXFLAG_HIDDENGENBANK;

        lasti = i+3;
        nfield++;

        i = ajStrFindNextC(line, lasti, "\t|\t");

        if(i<0)
        {
            ajWarn("%F line %u: only %u fields",
                      infile, linecnt, nfield);
            continue;
        }

        ajStrAssignSubS(&tmpstr, line, lasti, i-1);

        if(ajStrMatchC(tmpstr, "1"))
            tax->Flags |= TAXFLAG_HIDDENSUBTREE;

        lasti = i+3;
        nfield++;

        if((ajint) ajStrGetLen(line) > (lasti+2))
            ajStrAssignSubS(&tax->Comment, line, lasti, -3);

        lasti = i+3;
        nfield++;

        AJNEW0(ptaxid);
        *ptaxid = tax->Taxid;
        oldtax = ajTablePut(taxidtable, (void*) ptaxid, tax);
        if(oldtax)
            ajWarn("%F line %u: Duplicate taxon id '%u' (%Lu) found %u",
                   infile, linecnt, tax->Taxid, *ptaxid, oldtax->Taxid);
        ptaxid = NULL;
        tax = NULL;
    }

    ajUser("nodes.dmp %u records", linecnt);

    infile = ajFileNewInNamePathC("names.dmp", ajDirGetPath(taxdir));
    linecnt = 0;

    while(ajReadlineTrim(infile, &line))
    {
        linecnt++;
        nfield = 0;
        lasti = 0;

        /* files have tab | tab as delimiters */

        taxname = ajTaxNameNew();

        i = ajStrFindNextC(line, lasti, "\t|\t");

        if(i<0)
        {
            ajWarn("%F line %u: only %u fields",
                      infile, linecnt, nfield);
            continue;
        }

        ajStrAssignSubS(&tmpstr, line, lasti, i-1);

        if(!ajStrToLong(tmpstr, &taxid))
            ajWarn("%F line %u: invalid taxid '%S'",
                   infile, linecnt, tmpstr);

        lasti = i+3;
        nfield++;

        i = ajStrFindNextC(line, lasti, "\t|\t");

        if(i<0)
        {
            ajWarn("%F line %u: only %u fields",
                      infile, linecnt, nfield);
            continue;
        }

        if(i > lasti)
            ajStrAssignSubS(&taxname->Name, line, lasti, i-1);

        lasti = i+3;
        nfield++;

        i = ajStrFindNextC(line, lasti, "\t|\t");

        if(i<0)
        {
            ajWarn("%F line %u: only %u fields",
                      infile, linecnt, nfield);
            continue;
        }

        if(i > lasti)
            ajStrAssignSubS(&taxname->UniqueName, line, lasti, i-1);

        lasti = i+3;
        nfield++;

        if((ajint)ajStrGetLen(line) > (lasti+2))
        {
            ajStrAssignSubS(&tmpstr, line, lasti, -3);
            tablestr = ajTableFetch(taxNameclassTable, tmpstr);

            if(!tablestr)
            {
                tmpkey = ajStrNewS(tmpstr);
                tmpval = ajStrNewS(tmpstr);
                ajTablePut(taxNameclassTable, tmpkey, tmpstr);
                taxname->NameClass = ajStrNewRef(tmpval);
            }
            else
                taxname->NameClass = ajStrNewRef(tablestr);
        }

        lasti = i+3;
        nfield++;

        if(!ajStrMatchC(taxname->NameClass, "authority") &&
           ajStrGetLen(taxname->Name))
        {
            tax = ajTableFetch(taxnametable, taxname->Name);

            if(!tax)
                ajTablePut(taxnametable, taxname->Name, (void*) &taxid);
        }

        if(!ajStrMatchC(taxname->NameClass, "authority") &&
           ajStrGetLen(taxname->UniqueName))
        {
            tax = ajTableFetch(taxnametable, taxname->Name);

            if(!tax)
                ajTablePut(taxnametable, taxname->Name, (void*) &taxid);
        }

        tax = ajTableFetch(taxidtable, (const void*) &taxid);

        if(!tax)
            ajWarn("%F line %u: unknown taxon id '%u' '%S''",
                   infile, linecnt, taxid, tmpstr);
        else
        {
            ajListPushAppend(tax->Namelist, taxname);
            taxname = NULL;
        }
    }

    ajUser("names.dmp %u records", linecnt);

    infile = ajFileNewInNamePathC("division.dmp", ajDirGetPath(taxdir));
    linecnt = 0;

    while(ajReadlineTrim(infile, &line))
    {
        linecnt++;
        nfield = 0;
        lasti = 0;

        /* files have tab | tab as delimiters */

        taxdiv = ajTaxDivNew();

        i = ajStrFindNextC(line, lasti, "\t|\t");

        if(i<0)
        {
            ajWarn("%F line %u: only %u fields",
                      infile, linecnt, nfield);
            continue;
        }

        ajStrAssignSubS(&tmpstr, line, lasti, i-1);

        if(!ajStrToUint(tmpstr, &taxdiv->Divid))
            ajWarn("%F line %u: invalid division id '%S'",
                   infile, linecnt, tmpstr);

        lasti = i+3;
        nfield++;

        i = ajStrFindNextC(line, lasti, "\t|\t");

        if(i<0)
        {
            ajWarn("%F line %u: only %u fields",
                      infile, linecnt, nfield);
            continue;
        }

        if(i > lasti)
            ajStrAssignSubS(&taxdiv->GbCode, line, lasti, i-1);

        lasti = i+3;
        nfield++;

        i = ajStrFindNextC(line, lasti, "\t|\t");

        if(i<0)
        {
            ajWarn("%F line %u: only %u fields",
                      infile, linecnt, nfield);
            continue;
        }

        if(i > lasti)
            ajStrAssignSubS(&taxdiv->GbName, line, lasti, i-1);

        lasti = i+3;
        nfield++;

        if((ajint) ajStrGetLen(line) > (lasti+2))
            ajStrAssignSubS(&taxdiv->Comments, line, lasti, -3);

        lasti = i+3;
        nfield++;
    }

    ajUser("division.dmp %u records", linecnt);

    infile = ajFileNewInNamePathC("gencode.dmp", ajDirGetPath(taxdir));
    linecnt = 0;

    while(ajReadlineTrim(infile, &line))
    {
        linecnt++;
        nfield = 0;
        lasti = 0;

        /* files have tab | tab as delimiters */

        taxcode = ajTaxCodeNew();

        i = ajStrFindNextC(line, lasti, "\t|\t");

        if(i<0)
        {
            ajWarn("%F line %u: only %u fields",
                      infile, linecnt, nfield);
            continue;
        }

        ajStrAssignSubS(&tmpstr, line, lasti, i-1);

        if(!ajStrToUint(tmpstr, &taxcode->Gencode))
            ajWarn("%F line %u: invalid gencode '%S'",
                   infile, linecnt, tmpstr);

        lasti = i+3;
        nfield++;

        i = ajStrFindNextC(line, lasti, "\t|\t");

        if(i<0)
        {
            ajWarn("%F line %u: only %u fields",
                      infile, linecnt, nfield);
            continue;
        }

        if(i > lasti)
            ajStrAssignSubS(&taxcode->Abbrev, line, lasti, i-1);

        lasti = i+3;
        nfield++;

        i = ajStrFindNextC(line, lasti, "\t|\t");

        if(i<0)
        {
            ajWarn("%F line %u: only %u fields",
                      infile, linecnt, nfield);
            continue;
        }

        if(i > lasti)
            ajStrAssignSubS(&taxcode->Name, line, lasti, i-1);

        lasti = i+3;
        nfield++;

        i = ajStrFindNextC(line, lasti, "\t|\t");

        if(i<0)
        {
            ajWarn("%F line %u: only %u fields",
                      infile, linecnt, nfield);
            continue;
        }

        if(i > lasti)
            ajStrAssignSubS(&taxcode->Trans, line, lasti, i-1);

        lasti = i+3;
        nfield++;

        i = ajStrFindNextC(line, lasti, "\t|");

        if(i<0)
        {
            ajWarn("%F line %u: only %u fields",
                      infile, linecnt, nfield);
            continue;
        }
        
        if(i > lasti)
            ajStrAssignSubS(&taxcode->Starts, line, lasti, -3);

        lasti = i+3;
        nfield++;
    }

    ajUser("gencode.dmp %u records", linecnt);
    
    infile = ajFileNewInNamePathC("delnodes.dmp", ajDirGetPath(taxdir));
    linecnt = 0;

    while(ajReadlineTrim(infile, &line))
    {
        linecnt++;
        nfield = 0;
        lasti = 0;

        /* files have tab | tab as delimiters */

        taxdel = ajTaxDelNew();

        ajStrAssignSubS(&tmpstr, line, 0, -3);

        if(!ajStrToUint(tmpstr, &taxdel->Taxid))
            ajWarn("%F line %u: invalid taxid '%S'",
                   infile, linecnt, tmpstr);

        nfield++;
    }

    ajUser("delnodes.dmp %u records", linecnt);

    infile = ajFileNewInNamePathC("merged.dmp", ajDirGetPath(taxdir));
    linecnt = 0;

    while(ajReadlineTrim(infile, &line))
    {
        linecnt++;
        nfield = 0;
        lasti = 0;

        /* files have tab | tab as delimiters */

        taxmerge = ajTaxMergeNew();

        i = ajStrFindNextC(line, lasti, "\t|\t");

        if(i<0)
        {
            ajWarn("%F line %u: only %u fields",
                      infile, linecnt, nfield);
            continue;
        }

        ajStrAssignSubS(&tmpstr, line, lasti, i-1);

        if(!ajStrToUint(tmpstr, &taxmerge->Taxid))
            ajWarn("%F line %u: invalid taxid '%S'",
                   infile, linecnt, tmpstr);
        lasti = i+3;
        nfield++;

        ajStrAssignSubS(&tmpstr, line, lasti, -3);

        if(!ajStrToUint(tmpstr, &taxmerge->Mergeid))
            ajWarn("%F line %u: invalid merged taxid '%S'",
                   infile, linecnt, tmpstr);
        nfield++;
    }

    ajUser("merged.dmp %u records", linecnt);

    infile = ajFileNewInNamePathC("citations.dmp", ajDirGetPath(taxdir));
    linecnt = 0;

    while(ajReadlineTrim(infile, &line))
    {
        linecnt++;
        nfield = 0;
        lasti = 0;

        /* files have tab | tab as delimiters */

        taxcit = ajTaxCitNew();

        i = ajStrFindNextC(line, lasti, "\t|\t");

        if(i<0)
        {
            ajWarn("%F line %u: only %u fields",
                      infile, linecnt, nfield);
            continue;
        }

        ajStrAssignSubS(&tmpstr, line, lasti, i-1);

        if(!ajStrToUint(tmpstr, &taxcit->Citid))
            ajWarn("%F line %u: invalid citation id '%S'",
                   infile, linecnt, tmpstr);

        lasti = i+3;
        nfield++;

        i = ajStrFindNextC(line, lasti, "\t|\t");

        if(i<0)
        {
            ajWarn("%F line %u: only %u fields",
                      infile, linecnt, nfield);
            continue;
        }

        if(i > lasti)
            ajStrAssignSubS(&taxcit->Key, line, lasti, i-1);

        lasti = i+3;
        nfield++;

        i = ajStrFindNextC(line, lasti, "\t|\t");

        if(i<0)
        {
            ajWarn("%F line %u: only %u fields",
                      infile, linecnt, nfield);
            continue;
        }

        ajStrAssignSubS(&tmpstr, line, lasti, i-1);

        if(ajStrGetLen(tmpstr))
        {
            if(!ajStrToUint(tmpstr, &taxcit->Pubmed))
                ajWarn("%F line %u: invalid pubmed id '%S'",
                       infile, linecnt, tmpstr);
        }

        lasti = i+3;
        nfield++;

        i = ajStrFindNextC(line, lasti, "\t|\t");

        if(i<0)
        {
            ajWarn("%F line %u: only %u fields",
                      infile, linecnt, nfield);
            continue;
        }

        ajStrAssignSubS(&tmpstr, line, lasti, i-1);

        if(ajStrGetLen(tmpstr))
        {
            if(!ajStrToUint(tmpstr, &taxcit->Medline))
                ajWarn("%F line %u: invalid medline id '%S'",
                       infile, linecnt, tmpstr);
        }

        lasti = i+3;
        nfield++;

        i = ajStrFindNextC(line, lasti, "\t|\t");

        if(i<0)
        {
            ajWarn("%F line %u: only %u fields",
                      infile, linecnt, nfield);
            continue;
        }

        if(i > lasti)
            ajStrAssignSubS(&taxcit->Url, line, lasti, i-1);

        lasti = i+3;
        nfield++;

        i = ajStrFindNextC(line, lasti, "\t|\t");

        if(i<0)
        {
            ajWarn("%F line %u: only %u fields",
                      infile, linecnt, nfield);
            continue;
        }

        if(i > lasti)
            ajStrAssignSubS(&taxcit->Text, line, lasti, i-1);

        lasti = i+3;
        nfield++;

        ajStrAssignSubS(&tmpstr, line, lasti, -3); /* taxid list */
        nfield++;

        lasti = 0;
        ajStrTrimWhite(&tmpstr);
        i = ajStrFindNextC(tmpstr, lasti, " ");

        while(i > 0)
        {
            ajStrAssignSubS(&idstr, tmpstr, lasti, i-1);

            if(!ajStrToLong(idstr, &taxid))
            ajWarn("%F line %u: invalid taxid '%S'",
                   infile, linecnt, idstr);
            tax =  ajTableFetch(taxidtable, (const void*) &taxid);

            if(!tax)
                ajWarn("%F line %u: unknown taxon id '%u' '%S' '%S''",
                       infile, linecnt, taxid, idstr, tmpstr);
            else
            {
                taxcit->Refcount++;
                ajListPushAppend(tax->Citations, (void*) taxcit);
            }

            lasti = i+1;
            i = ajStrFindNextC(tmpstr, lasti, " ");
        }
    }

    ajUser("citations.dmp %u records", linecnt);

    ajUser("id table: %u", ajTableGetLength(taxidtable));
    ajUser("name table: %u", ajTableGetLength(taxnametable));

    ajUser("reftables rank: %u embl: %u nameclass: %u",
           ajTableGetLength(taxRankTable),
           ajTableGetLength(taxEmblcodeTable),
           ajTableGetLength(taxNameclassTable));


    ajUser("sizes node: %u", (ajuint) sizeof(*tax));

    return ajTrue;
}




/* @funcstatic taxTableCmp ****************************************************
**
** Default comparison function for key comparison
**
** @param [r] x [const void*] First key
** @param [r] y [const void*] Second key
** @return [ajint] 0 for success, 1 for different keys
** @@
******************************************************************************/

static ajint taxTableCmp(const void *x, const void *y)
{
    ajlong ix = *(const ajlong*)x;
    ajlong iy = *(const ajlong*)y;

    if(ix == iy)
        return 0;

    return 1;
}




/* @funcstatic taxTableHash ***************************************************
**
** Hash function for the cache table
**
** @param [r] key [const void*] Key
** @param [r] hashsize [ajuint] Hash size (maximum hash value)
** @return [ajuint] Hash value in range 0 to hashsize-1
** @@
******************************************************************************/

static ajuint taxTableHash(const void *key, ajuint hashsize)
{
    ajlong pageno = *(const ajlong*)key;

    return(pageno % hashsize);
}
