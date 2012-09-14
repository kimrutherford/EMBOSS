/* @source ajvarwrite *********************************************************
**
** AJAX variation data writing functions
**
** @author Copyright (C) 2010 Peter Rice
** @version $Revision: 1.18 $
** @modified Oct 25 2010 pmr First AJAX version
** @modified $Date: 2012/07/10 09:27:41 $ by $Author: rice $
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

#include "ajlib.h"

#include "ajvarwrite.h"
#include "ajvar.h"
#include "ajfile.h"
#include "ajtagval.h"
#include "ajseqbam.h"
#include "ajsys.h"
#include "ajvarbcf.h"

static AjBool varoutWriteBcf(AjPFile outf, const AjPVar var);
static AjBool varoutWriteVcf40(AjPFile outf, const AjPVar var);
static AjBool varoutWriteVcf41(AjPFile outf, const AjPVar var);
static AjBool varoutWriteNextBcf(AjPFile outf, const AjPVar var);
static AjBool varoutWriteNextVcf40(AjPFile outf, const AjPVar var);
static AjBool varoutWriteNextVcf41(AjPFile outf, const AjPVar var);

static void varoutVcfread(AjOVarBcf *b, const char* p, int k);

static AjPVarBcfFile varBcfFile = NULL;




/* @datastatic VarPOutFormat ************************************************
**
** Variation output formats data structure
**
** @alias VarSoutFormat
** @alias VarOOutFormat
**
** @attr Name [const char*] Format name
** @attr Desc [const char*] Format description
** @attr Write [AjBool function] Output function, returns ajTrue on success
** @attr WriteNext [AjBool function] Partial output function
** @@
******************************************************************************/

typedef struct VarSOutFormat
{
    const char *Name;
    const char *Desc;
    AjBool (*Write) (AjPFile outf, const AjPVar var);
    AjBool (*WriteNext) (AjPFile outf, const AjPVar var);
} VarOOutFormat;

#define VarPOutFormat VarOOutFormat*


static VarOOutFormat varoutFormatDef[] =
{
/* "Name",        "Description" */
/*     WriteFunction      WriteNextFunction */

  {"bcf",         "BCF format",
       &varoutWriteBcf,   &varoutWriteNextBcf},

  {"vcf40",         "VCFv4.0 format",
       &varoutWriteVcf40, &varoutWriteNextVcf40},

  {"vcf",           "VCFv4.0 format",
       &varoutWriteVcf40, &varoutWriteNextVcf40},

  {"vcf41",         "VCFv4.1 format",
       &varoutWriteVcf41, &varoutWriteNextVcf41},

  {NULL, NULL, NULL, NULL}
};




/* @filesection ajvar *******************************************************
**
** @nam1rule aj Function belongs to the AJAX library.
**
******************************************************************************/





/* @datasection [AjPoutfile] Variation data output ****************************
**
** Function is for manipulating variation data objects
**
** @nam2rule Varout Variation data output
**
******************************************************************************/




/* @section variation data outputs *********************************************
**
** These functions write the variation data provided by the first argument
**
** @fdata [AjPoutfile]
**
** @nam3rule Write Write variation data
** @nam4rule Format Use a named format
** @nam4rule Next Write next block of data
**
** @argrule Write outf [AjPOutfile] Output file
** @argrule Write var [const AjPVar] Variation data
** @argrule Format fmt [const AjPStr] Format name
**
** @valrule * [AjBool] true on success
**
** @fcategory output
**
******************************************************************************/




/* @func ajVaroutWrite ********************************************************
**
** Write variation data in a named format
**
** @param [u] outf [AjPOutfile] Output file
** @param [r] var [const AjPVar] Variation object
**
** @return [AjBool] True on success
**
**
** @release 6.4.0
******************************************************************************/

AjBool ajVaroutWrite(AjPOutfile outf, const AjPVar var)
{
    ajuint i = ajOutfileGetFormatindex(outf);
    AjPFile outfile = ajOutfileGetFile(outf);

    return (*varoutFormatDef[i].Write)(outfile, var);
}





/* @func ajVaroutWriteNext ****************************************************
**
** Write latest chunk of variation data in a named format
**
** @param [u] outf [AjPOutfile] Output file
** @param [r] var [const AjPVar] Variation object
**
** @return [AjBool] True on success
**
** @release 6.5.0
******************************************************************************/

AjBool ajVaroutWriteNext(AjPOutfile outf, const AjPVar var)
{
    ajuint i = ajOutfileGetFormatindex(outf);
    AjPFile outfile = ajOutfileGetFile(outf);

    return (*varoutFormatDef[i].WriteNext)(outfile, var);
}





/* @funcstatic varoutWriteVcf40 ***********************************************
**
** Write variation data in VCFv4.0 format
**
** @param [u] outf [AjPFile] Output file
** @param [r] var [const AjPVar] Variation data object
**
** @return [AjBool] True on success
**
** @release 6.5.0
**
******************************************************************************/

static AjBool varoutWriteVcf40(AjPFile outf, const AjPVar var)
{
    AjPVarHeader varheader = NULL;
    AjPVarData   vardata   = NULL;
    AjPVarField  varfield  = NULL;
    AjPVarSample varsample = NULL;
    AjPTagval tagval = NULL;
    AjIList iter = NULL;
    AjIList itersam = NULL;
    AjPStr tmpstr = NULL;
    ajuint i;

    if(!outf) return ajFalse;
    if(!var) return ajFalse;

    ajFmtPrintF(outf, "##fileformat=VCFv4.0\n");

    varheader = var->Header;

    /* header */

    iter = ajListIterNewread(varheader->Header);
    while(!ajListIterDone(iter))
    {
        tagval = ajListIterGet(iter);
        if(!ajStrMatchC( MAJTAGVALGETTAG(tagval), "fileformat"))
            ajFmtPrintF(outf, "##%S=%S\n",
                        MAJTAGVALGETTAG(tagval),
                        MAJTAGVALGETVALUE(tagval));
    }
    ajListIterDel(&iter);

    iter = ajListIterNewread(varheader->Fields);
    while(!ajListIterDone(iter))
    {
        varfield = ajListIterGet(iter);
        ajFmtPrintF(outf,
                    "##%S=<ID=%S,Number=%s,Type=%s,Description=\"%S\">\n",
                    varfield->Field,
                    varfield->Id,
                    ajVarfieldGetNumber(varfield),
                    ajVarfieldGetType(varfield),
                    varfield->Desc);
    }
    ajListIterDel(&iter);

    iter = ajListIterNewread(varheader->Filters);
    while(!ajListIterDone(iter))
    {
        tagval = ajListIterGet(iter);
        ajFmtPrintF(outf,
                    "##FILTER=<ID=%S,Description=\"%S\">\n",
                    MAJTAGVALGETTAG(tagval),
                    MAJTAGVALGETVALUE(tagval));
    }
    ajListIterDel(&iter);

    iter = ajListIterNewread(varheader->Alts);
    while(!ajListIterDone(iter))
    {
        tagval = ajListIterGet(iter);
        ajFmtPrintF(outf,
                    "##ALT=<ID=%S,Description=\"%S\">\n",
                    MAJTAGVALGETTAG(tagval),
                    MAJTAGVALGETVALUE(tagval));
    }
    ajListIterDel(&iter);

    iter = ajListIterNewread(varheader->Samples);
    while(!ajListIterDone(iter))
    {
        varsample = ajListIterGet(iter);
        ajFmtPrintF(outf,
                    "##SAMPLE=<ID=%S,Genomes=",
                    varsample->Id);

        for(i=0; i < varsample->Number; i++)
        {
            if(i)
                ajFmtPrintF(outf, ";");
            ajFmtPrintF(outf, "%S", varsample->Genomes[i]);
        }

        ajFmtPrintF(outf, ",Mixture=");

        for(i=0; i < varsample->Number; i++)
        {
            if(i)
                ajFmtPrintF(outf, ";");
            ajStrFromFloat(&tmpstr, varsample->Mixture[i], 4);
            ajStrTrimC(&tmpstr, "0");
            ajFmtPrintF(outf, "%S", tmpstr);
        }
        ajStrDel(&tmpstr);

        ajFmtPrintF(outf, ",Description=\"");

        for(i=0; i < varsample->Number; i++)
        {
            if(i)
                ajFmtPrintF(outf, ";");
            ajFmtPrintF(outf, "%S", varsample->Desc[i]);
        }

        ajFmtPrintF(outf, "\">\n");

    }

    ajListIterDel(&iter);

    iter = ajListIterNewread(varheader->Pedigrees);
    while(!ajListIterDone(iter))
    {
        tagval = ajListIterGet(iter);
        ajFmtPrintF(outf,
                    "##PEDIGREE=%S\n",
                    MAJTAGVALGETVALUE(tagval));
    }
    ajListIterDel(&iter);

    ajFmtPrintF(outf, "#CHROM\tPOS\tID\tREF\tALT\tQUAL\tFILTER\tINFO\tFORMAT");
    iter = ajListIterNewread(varheader->SampleIds);
    while(!ajListIterDone(iter))
    {
        tmpstr = ajListIterGet(iter);
        ajFmtPrintF(outf, "\t%S", tmpstr);
    }
    ajListIterDel(&iter);
    ajFmtPrintF(outf, "\n");

    /* data */

    iter = ajListIterNewread(var->Data);
    while(!ajListIterDone(iter))
    {
        vardata = ajListIterGet(iter);
        ajFmtPrintF(outf,
                    "%S\t%u\t%S\t%S\t%S\t%S\t%S\t%S\t%S",
                    vardata->Chrom, vardata->Pos, vardata->Id,
                    vardata->Ref, vardata->Alt,
                    vardata->Qual, vardata->Filter,
                    vardata->Info, vardata->Format);
        itersam = ajListIterNewread(vardata->Samples);
        while(!ajListIterDone(itersam))
        {
            tmpstr = ajListIterGet(itersam);
            ajFmtPrintF(outf, "\t%S", tmpstr);
        }
        ajListIterDel(&itersam);
        ajFmtPrintF(outf, "\n");
    }
    ajListIterDel(&iter);

    return ajTrue;
}




/* @funcstatic varoutWriteNextVcf40 *******************************************
**
** Write variation data in VCFv4.0 format
**
** @param [u] outf [AjPFile] Output file
** @param [r] var [const AjPVar] Variation data object
**
** @return [AjBool] True on success
**
** @release 6.5.0
**
******************************************************************************/

static AjBool varoutWriteNextVcf40(AjPFile outf, const AjPVar var)
{
    AjPVarHeader varheader = NULL;
    AjPVarData   vardata   = NULL;
    AjPVarField  varfield  = NULL;
    AjPVarSample varsample = NULL;
    AjPTagval tagval = NULL;
    AjIList iter = NULL;
    AjIList itersam = NULL;
    AjPStr tmpstr = NULL;
    ajuint i;

    if(!outf) return ajFalse;
    if(!var) return ajFalse;

    ajDebug("varoutWriteNextVcf40 Hasdata: %B data: %Lu\n",
            var->Hasdata, ajListGetLength(var->Data));

    if(!var->Hasdata)
    {
        ajFmtPrintF(outf, "##fileformat=VCFv4.0\n");

        varheader = var->Header;

        /* header */

        iter = ajListIterNewread(varheader->Header);
        while(!ajListIterDone(iter))
        {
            tagval = ajListIterGet(iter);
            if(!ajStrMatchC( MAJTAGVALGETTAG(tagval), "fileformat"))
                ajFmtPrintF(outf, "##%S=%S\n",
                            MAJTAGVALGETTAG(tagval),
                            MAJTAGVALGETVALUE(tagval));
        }
        ajListIterDel(&iter);

        iter = ajListIterNewread(varheader->Fields);
        while(!ajListIterDone(iter))
        {
            varfield = ajListIterGet(iter);
            ajFmtPrintF(outf,
                        "##%S=<ID=%S,Number=%s,Type=%s,Description=\"%S\">\n",
                        varfield->Field,
                        varfield->Id,
                        ajVarfieldGetNumber(varfield),
                        ajVarfieldGetType(varfield),
                        varfield->Desc);
        }
        ajListIterDel(&iter);

        iter = ajListIterNewread(varheader->Filters);
        while(!ajListIterDone(iter))
        {
            tagval = ajListIterGet(iter);
            ajFmtPrintF(outf,
                        "##FILTER=<ID=%S,Description=\"%S\">\n",
                        MAJTAGVALGETTAG(tagval),
                        MAJTAGVALGETVALUE(tagval));
        }
        ajListIterDel(&iter);

        iter = ajListIterNewread(varheader->Alts);
        while(!ajListIterDone(iter))
        {
            tagval = ajListIterGet(iter);
            ajFmtPrintF(outf,
                        "##ALT=<ID=%S,Description=\"%S\">\n",
                        MAJTAGVALGETTAG(tagval),
                        MAJTAGVALGETVALUE(tagval));
        }
        ajListIterDel(&iter);

        iter = ajListIterNewread(varheader->Samples);
        while(!ajListIterDone(iter))
        {
            varsample = ajListIterGet(iter);
            ajFmtPrintF(outf,
                        "##SAMPLE=<ID=%S,Genomes=",
                        varsample->Id);

            for(i=0; i < varsample->Number; i++)
            {
                if(i)
                    ajFmtPrintF(outf, ";");
                ajFmtPrintF(outf, "%S", varsample->Genomes[i]);
            }

            ajFmtPrintF(outf, ",Mixture=");

            for(i=0; i < varsample->Number; i++)
            {
                if(i)
                    ajFmtPrintF(outf, ";");
                ajStrFromFloat(&tmpstr, varsample->Mixture[i], 4);
                ajStrTrimC(&tmpstr, "0");
                ajFmtPrintF(outf, "%S", tmpstr);
            }
            ajStrDel(&tmpstr);

            ajFmtPrintF(outf, ",Description=\"");

            for(i=0; i < varsample->Number; i++)
            {
                if(i)
                    ajFmtPrintF(outf, ";");
                ajFmtPrintF(outf, "%S", varsample->Desc[i]);
            }

            ajFmtPrintF(outf, "\">\n");

        }

        ajListIterDel(&iter);

        iter = ajListIterNewread(varheader->Pedigrees);
        while(!ajListIterDone(iter))
        {
            tagval = ajListIterGet(iter);
            ajFmtPrintF(outf,
                        "##PEDIGREE=%S\n",
                        MAJTAGVALGETVALUE(tagval));
        }
        ajListIterDel(&iter);

        ajFmtPrintF(outf,
                    "#CHROM\tPOS\tID\tREF\tALT\tQUAL\tFILTER\tINFO\tFORMAT");
        iter = ajListIterNewread(varheader->SampleIds);
        while(!ajListIterDone(iter))
        {
            tmpstr = ajListIterGet(iter);
            ajFmtPrintF(outf, "\t%S", tmpstr);
        }
        ajListIterDel(&iter);
        ajFmtPrintF(outf, "\n");

        return ajTrue;
    }

    /* data */

    iter = ajListIterNewread(var->Data);
    while(!ajListIterDone(iter))
    {
        vardata = ajListIterGet(iter);
        ajFmtPrintF(outf,
                    "%S\t%u\t%S\t%S\t%S\t%S\t%S\t%S\t%S",
                    vardata->Chrom, vardata->Pos, vardata->Id,
                    vardata->Ref, vardata->Alt,
                    vardata->Qual, vardata->Filter,
                    vardata->Info, vardata->Format);
        itersam = ajListIterNewread(vardata->Samples);
        while(!ajListIterDone(itersam))
        {
            tmpstr = ajListIterGet(itersam);
            ajFmtPrintF(outf, "\t%S", tmpstr);
        }
        ajListIterDel(&itersam);
        ajFmtPrintF(outf, "\n");
    }
    ajListIterDel(&iter);

    return ajTrue;
}




/* @funcstatic varoutWriteBcf *************************************************
**
** Write variation data in BCF format
**
** @param [u] outf [AjPFile] Output file
** @param [r] var [const AjPVar] Variation data object
**
** @return [AjBool] True on success
**
** @release 6.5.0
**
******************************************************************************/

static AjBool varoutWriteBcf(AjPFile outf, const AjPVar var)
{
    AjPVarHeader varheader = NULL;
    AjPVarData   vardata   = NULL;
    AjPVarField  varfield  = NULL;
    AjPVarSample varsample = NULL;
    AjPTagval tagval = NULL;
    AjIList iter = NULL;
    AjIList itersam = NULL;
    AjPStr tmpstr = NULL;
    ajuint i;
    AjPStr meta = NULL;
    AjPStr smpl = NULL; /* sample names */

    AjPVarBcfFile fp = NULL;
    AjPVarBcfHeader h = NULL;
    AjOVarBcf *b = NULL;
    int k=0;

    if(!outf) return ajFalse;
    if(!var) return ajFalse;

    if(!var->Hasdata)
    {
        fp = ajSeqBamBgzfNew(ajFileGetFileptr(outf), "w");
        varBcfFile = fp;

        varheader = var->Header;


        /* header */

        AJNEW0(h);
        meta = ajStrNewRes(4096);
        ajFmtPrintAppS(&meta,"##fileformat=VCFv4.1\n");

        iter = ajListIterNewread(varheader->Header);
        while(!ajListIterDone(iter))
        {
            tagval = ajListIterGet(iter);
            if(!ajStrMatchC( MAJTAGVALGETTAG(tagval), "fileformat"))
                ajFmtPrintAppS(&meta,"##%S=%S\n",
                               MAJTAGVALGETTAG(tagval),
                               MAJTAGVALGETVALUE(tagval));
        }
        ajListIterDel(&iter);

        iter = ajListIterNewread(varheader->Fields);
        while(!ajListIterDone(iter))
        {
            varfield = ajListIterGet(iter);
            ajFmtPrintAppS(&meta,
                           "##%S=<ID=%S,Number=%s,Type=%s,Description=\"%S\">\n",
                           varfield->Field,
                           varfield->Id,
                           ajVarfieldGetNumber(varfield),
                           ajVarfieldGetType(varfield),
                           varfield->Desc);
        }
        ajListIterDel(&iter);

        iter = ajListIterNewread(varheader->Filters);
        while(!ajListIterDone(iter))
        {
            tagval = ajListIterGet(iter);
            ajFmtPrintAppS(&meta, "##FILTER=<ID=%S,Description=\"%S\">\n",
                           MAJTAGVALGETTAG(tagval),
                           MAJTAGVALGETVALUE(tagval));
        }
        ajListIterDel(&iter);

        iter = ajListIterNewread(varheader->Alts);
        while(!ajListIterDone(iter))
        {
            tagval = ajListIterGet(iter);
            ajFmtPrintAppS(&meta, "##ALT=<ID=%S,Description=\"%S\">\n",
                           MAJTAGVALGETTAG(tagval),
                           MAJTAGVALGETVALUE(tagval));
        }
        ajListIterDel(&iter);


        h->n_smpl = (ajint) ajListGetLength(varheader->Samples);

        iter = ajListIterNewread(varheader->Samples);
        while(!ajListIterDone(iter))
        {
            varsample = ajListIterGet(iter);
            ajFmtPrintAppS(&meta, "##SAMPLE=<ID=%S,Genomes=",
                           varsample->Id);
            
            for(i=0; i < varsample->Number; i++)
            {
                if(i)
                    ajFmtPrintAppS(&meta, ";");
                ajFmtPrintAppS(&meta, "%S", varsample->Genomes[i]);
            }

            ajFmtPrintAppS(&meta, ",Mixture=");

            for(i=0; i < varsample->Number; i++)
            {
                ajStrFromFloat(&tmpstr, varsample->Mixture[i], 4);
                ajStrTrimC(&tmpstr, "0");
                ajFmtPrintAppS(&meta, "%S", tmpstr);
            }
            ajStrDel(&tmpstr);

            ajFmtPrintAppS(&meta, ",Description=\"");
            
            for(i=0; i < varsample->Number; i++)
            {
                if(i)
                    ajFmtPrintAppS(&meta, ";");
                ajFmtPrintAppS(&meta, "%S", varsample->Desc[i]);
            }

            ajFmtPrintAppS(&meta, "\">\n");

        }

        ajListIterDel(&iter);

        iter = ajListIterNewread(varheader->Pedigrees);
        while(!ajListIterDone(iter))
        {
            tagval = ajListIterGet(iter);
            ajFmtPrintAppS(&meta, "##PEDIGREE=%S\n",
                           MAJTAGVALGETVALUE(tagval));
        }
        ajListIterDel(&iter);

        iter = ajListIterNewread(varheader->SampleIds);
        while(!ajListIterDone(iter))
        {
            tmpstr = ajListIterGet(iter);
            ajStrAppendS(&smpl, tmpstr);
            ajStrAppendK(&smpl, '\0');
        }
        ajListIterDel(&iter);

        h->n_smpl = (ajint) ajListGetLength(varheader->SampleIds);
        h->l_smpl = ajStrGetLen(smpl);
        AJCNEW0(h->sname, h->l_smpl);
        memcpy(h->sname, ajStrGetPtr(smpl), h->l_smpl);

        h->txt = ajCharNewS(meta);
        h->l_txt = ajStrGetLen(meta)+1;

        ajVarbcfHdrWrite(fp, h);

        ajVarbcfHdrDel(h);
        return ajTrue;
    }

    fp = varBcfFile;
        
    /* data */

    AJNEW0(b);
    b->n_smpl = (ajint) ajListGetLength(varheader->SampleIds);

    iter = ajListIterNewread(var->Data);
    while(!ajListIterDone(iter))
    {
	int* tid = NULL;
	const int* ctid = NULL;
	AjPStr str = NULL;


	vardata = ajListIterGet(iter);
	ajDebug("%S\t%u\t%S\t%S\t%S\t%S\t%S\t%S\t%S",
		vardata->Chrom, vardata->Pos, vardata->Id,
		vardata->Ref, vardata->Alt,
		vardata->Qual, vardata->Filter,
		vardata->Info, vardata->Format);

	ctid = (const int*)ajTableFetchS(varheader->RefseqIds, vardata->Chrom);
	if(!ctid)
	{/* TODO: check this */
	    AJNEW0(tid);
	    *tid = (int) ajTableGetLength(varheader->RefseqIds);
	    ajTablePut(varheader->RefseqIds, vardata->Chrom, tid);
	    ctid=tid;
	}
	b->tid = *ctid;
	b->pos = vardata->Pos-1;
	ajStrToFloat(vardata->Qual, &b->qual);

	b->l_str =
		ajStrGetLen(vardata->Id)+
		ajStrGetLen(vardata->Ref)+
		ajStrGetLen(vardata->Alt)+
		ajStrGetLen(vardata->Filter)+
		ajStrGetLen(vardata->Info)+
		ajStrGetLen(vardata->Format)+6;

	str = ajStrNewRes(b->l_str);

	ajStrAppendS(&str, vardata->Id);
	ajStrAppendK(&str, '\0');
	ajStrAppendS(&str, vardata->Ref);
	ajStrAppendK(&str, '\0');
	ajStrAppendS(&str,vardata->Alt);
	ajStrAppendK(&str, '\0');
	ajStrAppendS(&str,vardata->Filter);
	ajStrAppendK(&str, '\0');
	ajStrAppendS(&str,vardata->Info);
	ajStrAppendK(&str, '\0');
	ajStrAppendS(&str,vardata->Format);
	ajStrAppendK(&str, '\0');

	/* TODO: instead of writing to the string first
	 * better??? to write to the b->str directly ??? */

	AJCRESIZE0(b->str,b->m_str, b->l_str);
	b->m_str = b->l_str;
	memcpy((void*)b->str, ajStrGetPtr(str), b->l_str);

	ajStrDel(&str);

	/*
	 * after setting the format field, vcf.c:vcf_read function calls
	 * bcf.c:ajVarbcfSync function, sam/bcftools project
	 *
	 */

	ajVarbcfSync(b);


	/*b->n_smpl = ajListGetLength(vardata->Samples);*/

	itersam = ajListIterNewread(vardata->Samples);
	k=9;
	while(!ajListIterDone(itersam))
	{
	    tmpstr = ajListIterGet(itersam);
	    ajDebug("\t%S", tmpstr);

	    varoutVcfread(b, ajStrGetPtr(tmpstr), k++);
	}
	ajListIterDel(&itersam);
	ajDebug("\n");

	ajVarbcfWrite(fp, b->n_smpl, b);
	AJFREE(b->str);
    }
    ajListIterDel(&iter);

    /*ajSeqBamBgzfClose(fp);*/
    fp=NULL;

    ajVarbcfDel(b);

    ajStrDel(&meta);
    ajStrDel(&smpl);

    return ajTrue;
}




/* @funcstatic varoutWriteNextBcf *********************************************
**
** Write variation data in BCF format
**
** @param [u] outf [AjPFile] Output file
** @param [r] var [const AjPVar] Variation data object
**
** @return [AjBool] True on success
**
** @release 6.5.0
**
******************************************************************************/

static AjBool varoutWriteNextBcf(AjPFile outf, const AjPVar var)
{
    AjPVarHeader varheader = NULL;
    AjPVarData   vardata   = NULL;
    AjPVarField  varfield  = NULL;
    AjPVarSample varsample = NULL;
    AjPTagval tagval = NULL;
    AjIList iter = NULL;
    AjIList itersam = NULL;
    AjPStr tmpstr = NULL;
    ajuint i;
    AjPStr meta = NULL;
    AjPStr smpl = NULL; /* sample names */

    AjPVarBcfFile fp = NULL;
    AjPVarBcfHeader h = NULL;
    AjOVarBcf *b = NULL;
    int k=0;

    if(!outf) return ajFalse;
    if(!var) return ajFalse;


    ajDebug("varoutWriteNextBcf Hasdata: %B data: %Lu\n",
            var->Hasdata, ajListGetLength(var->Data));

    if(!var->Hasdata)
    {
        varheader = var->Header;

        fp = ajSeqBamBgzfNew(ajFileGetFileptr(outf), "w");
        varBcfFile = fp;

        /* header */

        AJNEW0(h);
        meta = ajStrNewRes(4096);
        ajFmtPrintAppS(&meta,"##fileformat=VCFv4.1\n");

        iter = ajListIterNewread(varheader->Header);
        while(!ajListIterDone(iter))
        {
            tagval = ajListIterGet(iter);
            if(!ajStrMatchC( MAJTAGVALGETTAG(tagval), "fileformat"))
                ajFmtPrintAppS(&meta,"##%S=%S\n",
                               MAJTAGVALGETTAG(tagval),
                               MAJTAGVALGETVALUE(tagval));
        }
        ajListIterDel(&iter);

        iter = ajListIterNewread(varheader->Fields);
        while(!ajListIterDone(iter))
        {
            varfield = ajListIterGet(iter);
            ajFmtPrintAppS(&meta,
                           "##%S=<ID=%S,Number=%s,Type=%s,"
                           "Description=\"%S\">\n",
                           varfield->Field,
                           varfield->Id,
                           ajVarfieldGetNumber(varfield),
                           ajVarfieldGetType(varfield),
                           varfield->Desc);
        }
        ajListIterDel(&iter);

        iter = ajListIterNewread(varheader->Filters);
        while(!ajListIterDone(iter))
        {
            tagval = ajListIterGet(iter);
            ajFmtPrintAppS(&meta, "##FILTER=<ID=%S,Description=\"%S\">\n",
                           MAJTAGVALGETTAG(tagval),
                           MAJTAGVALGETVALUE(tagval));
        }
        ajListIterDel(&iter);

        iter = ajListIterNewread(varheader->Alts);
        while(!ajListIterDone(iter))
        {
            tagval = ajListIterGet(iter);
            ajFmtPrintAppS(&meta, "##ALT=<ID=%S,Description=\"%S\">\n",
                           MAJTAGVALGETTAG(tagval),
                           MAJTAGVALGETVALUE(tagval));
        }
        ajListIterDel(&iter);


        h->n_smpl = (ajint) ajListGetLength(varheader->Samples);

        iter = ajListIterNewread(varheader->Samples);
        while(!ajListIterDone(iter))
        {
            varsample = ajListIterGet(iter);
            ajFmtPrintAppS(&meta, "##SAMPLE=<ID=%S,Genomes=",
                           varsample->Id);

            for(i=0; i < varsample->Number; i++)
            {
                if(i)
                    ajFmtPrintAppS(&meta, ";");
                ajFmtPrintAppS(&meta, "%S", varsample->Genomes[i]);
            }

            ajFmtPrintAppS(&meta, ",Mixture=");

            for(i=0; i < varsample->Number; i++)
            {
                ajStrFromFloat(&tmpstr, varsample->Mixture[i], 4);
                ajStrTrimC(&tmpstr, "0");
                ajFmtPrintAppS(&meta, "%S", tmpstr);
            }
            ajStrDel(&tmpstr);

            ajFmtPrintAppS(&meta, ",Description=\"");

            for(i=0; i < varsample->Number; i++)
            {
                if(i)
                    ajFmtPrintAppS(&meta, ";");
                ajFmtPrintAppS(&meta, "%S", varsample->Desc[i]);
            }

            ajFmtPrintAppS(&meta, "\">\n");

        }

        ajListIterDel(&iter);

        iter = ajListIterNewread(varheader->Pedigrees);
        while(!ajListIterDone(iter))
        {
            tagval = ajListIterGet(iter);
            ajFmtPrintAppS(&meta, "##PEDIGREE=%S\n",
                           MAJTAGVALGETVALUE(tagval));
        }
        ajListIterDel(&iter);

        iter = ajListIterNewread(varheader->SampleIds);
        while(!ajListIterDone(iter))
        {
            tmpstr = ajListIterGet(iter);
            ajStrAppendS(&smpl, tmpstr);
            ajStrAppendK(&smpl, '\0');
        }
        ajListIterDel(&iter);

        h->n_smpl = (ajint) ajListGetLength(varheader->SampleIds);
        h->l_smpl = ajStrGetLen(smpl);
        AJCNEW0(h->sname, h->l_smpl);
        memcpy(h->sname, ajStrGetPtr(smpl), h->l_smpl);

        h->txt = ajCharNewS(meta);
        h->l_txt = ajStrGetLen(meta)+1;

        ajVarbcfHdrWrite(fp, h);

        ajStrDel(&meta);
        ajStrDel(&smpl);
        ajVarbcfHdrDel(h);

        return ajTrue;
    }

    /* data */

    fp = varBcfFile;

    varheader = var->Header;
    AJNEW0(b);
    b->n_smpl = (ajint) ajListGetLength(varheader->SampleIds);

    iter = ajListIterNewread(var->Data);
    while(!ajListIterDone(iter))
    {
	int* tid = NULL;
	const int* ctid = NULL;
	AjPStr str = NULL;

	vardata = ajListIterGet(iter);
	ajDebug("%S\t%u\t%S\t%S\t%S\t%S\t%S\t%S\t%S",
		vardata->Chrom, vardata->Pos, vardata->Id,
		vardata->Ref, vardata->Alt,
		vardata->Qual, vardata->Filter,
		vardata->Info, vardata->Format);

	ctid = (const int*)ajTableFetchS(varheader->RefseqIds, vardata->Chrom);
	if(!ctid)
	{/* TODO: check this */
	    AJNEW0(tid);
	    *tid = (int) ajTableGetLength(varheader->RefseqIds);
	    ajTablePut(varheader->RefseqIds, vardata->Chrom, tid);
	    ctid=tid;
	}
	b->tid = *ctid;
	b->pos = vardata->Pos-1;
	ajStrToFloat(vardata->Qual, &b->qual);

	b->l_str =
		ajStrGetLen(vardata->Id)+
		ajStrGetLen(vardata->Ref)+
		ajStrGetLen(vardata->Alt)+
		ajStrGetLen(vardata->Filter)+
		ajStrGetLen(vardata->Info)+
		ajStrGetLen(vardata->Format)+6;

	str = ajStrNewRes(b->l_str);

	ajStrAppendS(&str, vardata->Id);
	ajStrAppendK(&str, '\0');
	ajStrAppendS(&str, vardata->Ref);
	ajStrAppendK(&str, '\0');
	ajStrAppendS(&str,vardata->Alt);
	ajStrAppendK(&str, '\0');
	ajStrAppendS(&str,vardata->Filter);
	ajStrAppendK(&str, '\0');
	ajStrAppendS(&str,vardata->Info);
	ajStrAppendK(&str, '\0');
	ajStrAppendS(&str,vardata->Format);
	ajStrAppendK(&str, '\0');

	/* TODO: instead of writing to the string first
	 * better??? to write to the b->str directly ??? */

	AJCRESIZE0(b->str,b->m_str, b->l_str);
	b->m_str = b->l_str;
	memcpy((void*)b->str, ajStrGetPtr(str), b->l_str);

	ajStrDel(&str);

	/*
	 * after setting the format field, vcf.c:vcf_read function calls
	 * bcf.c:ajVarbcfSync function, sam/bcftools project
	 *
	 */

	ajVarbcfSync(b);


	/*b->n_smpl = ajListGetLength(vardata->Samples);*/

	itersam = ajListIterNewread(vardata->Samples);
	k=9;
	while(!ajListIterDone(itersam))
	{
	    tmpstr = ajListIterGet(itersam);
	    ajDebug("\t%S", tmpstr);

	    varoutVcfread(b, ajStrGetPtr(tmpstr), k++);
	}
	ajListIterDel(&itersam);
	ajDebug("\n");

	ajVarbcfWrite(fp, b->n_smpl, b);
	AJFREE(b->str);
    }
    ajListIterDel(&iter);

    ajSeqBamBgzfClose(fp);
    fp=NULL;

    ajVarbcfHdrDel(h);
    ajVarbcfDel(b);

    ajStrDel(&meta);
    ajStrDel(&smpl);

    return ajTrue;
}




/* @funcstatic varoutVcfread *************************************************
**
** part of the vcf_read function in vcf.c in sam/bcftools
**
** @param [u] b [AjOVarBcf*] BCF variation record where data is read into
** @param [r] p [const char*] VCF optional data string
** @param [r] k [int] VCF optional data column number
**
** @return [void]
**
** @release 6.5.0
**
******************************************************************************/

static void varoutVcfread(AjOVarBcf *b, const char* p, int k)
{
    char*q=NULL;
    char* qq = NULL;
    AjPStr tok=NULL;
    const char* save=NULL;
    int i=0;
    int j=0;
    int x=0;
    int y=0;
    double gl=0;
    float* data=NULL;

    /* k >= 9 */
    if (strncmp(p, "./.", 3) == 0)
    {
	for (i = 0; i < b->n_gi; ++i)
	{
	    if (b->gi[i].fmt == bcf_str2int("GT", 2))
	    {
		((unsigned char*)b->gi[i].data)[k-9] = 1<<7;
	    }
	    else if (b->gi[i].fmt == bcf_str2int("GQ", 2))
	    {
		((unsigned char*)b->gi[i].data)[k-9] = 0;
	    }
	    else if (b->gi[i].fmt == bcf_str2int("SP", 2))
	    {
		((ajint*)b->gi[i].data)[k-9] = 0;
	    }
	    else if (b->gi[i].fmt == bcf_str2int("DP", 2))
	    {
		((ajushort*)b->gi[i].data)[k-9] = 0;
	    }
	    else if (b->gi[i].fmt == bcf_str2int("PL", 2))
	    {
		y = b->n_alleles * (b->n_alleles + 1) / 2;
		memset((unsigned char*)b->gi[i].data + (k - 9) * y, 0, y);
	    }
	    else if (b->gi[i].fmt == bcf_str2int("GL", 2))
	    {
		y = b->n_alleles * (b->n_alleles + 1) / 2;
		memset((float*)b->gi[i].data + (k - 9) * y, 0, y * 4);
	    }
	}
	return;
    }

    save = p;

    for (i = 0; ajSysFuncStrtokR(NULL, ":", &save, &tok) && i < b->n_gi; ++i)
    {
 	q = ajCharNewS(tok);
	qq = q;

	if (b->gi[i].fmt == bcf_str2int("GT", 2))
	{
	    ((unsigned char*)b->gi[i].data)[k-9] =
		    (q[0] - '0')<<3 | (q[2] - '0') | (q[1] == '/'? 0 : 1) << 6;
	}
	else if (b->gi[i].fmt == bcf_str2int("GQ", 2))
	{
	    double _x = strtod(q, &q);
	    x = (int)(_x + .499);
	    if (x > 255) x = 255;
	    ((unsigned char*)b->gi[i].data)[k-9] = x;
	}
	else if (b->gi[i].fmt == bcf_str2int("SP", 2))
	{
	    x = strtol(q, &q, 10);
	    if (x > 0xffff) x = 0xffff;
	    ((ajuint*)b->gi[i].data)[k-9] = x;
	}
	else if (b->gi[i].fmt == bcf_str2int("DP", 2))
	{
	    x = strtol(q, &q, 10);
	    if (x > 0xffff) x = 0xffff;
	    ((ajushort*)b->gi[i].data)[k-9] = x;
	}
	else if (b->gi[i].fmt == bcf_str2int("PL", 2))
	{
	    y = b->n_alleles * (b->n_alleles + 1) / 2;

	    for (j = 0; j < y; ++j)
	    {
		x = strtol(q, &q, 10);
		if (x > 255) x = 255;
		((unsigned char*)b->gi[i].data)[(k-9) * y + j] = x;
		++q;
	    }
	}
	else if (b->gi[i].fmt == bcf_str2int("GL", 2))
	{
	    data = (float*)b->gi[i].data;
	    y = b->n_alleles * (b->n_alleles + 1) / 2;

	    for (j = 0; j < y; ++j)
	    {
		gl = strtod(q, &q);
		data[(k-9) * y + j] = (float) (gl > 0? -gl/10. : gl);
		++q;
	    }
	}

	AJFREE(qq);
    }

    ajStrDel(&tok);

    return;
}




/* @funcstatic varoutWriteVcf41 ***********************************************
**
** Write variation data in VCFv4.1 format
**
** @param [u] outf [AjPFile] Output file
** @param [r] var [const AjPVar] Variation data object
**
** @return [AjBool] True on success
**
** @release 6.5.0
**
******************************************************************************/

static AjBool varoutWriteVcf41(AjPFile outf, const AjPVar var)
{
    AjPVarHeader varheader = NULL;
    AjPVarData   vardata   = NULL;
    AjPVarField  varfield  = NULL;
    AjPVarSample varsample = NULL;
    AjPTagval tagval = NULL;
    AjIList iter = NULL;
    AjIList itersam = NULL;
    AjPStr tmpstr = NULL;
    ajuint i;

    if(!outf) return ajFalse;
    if(!var) return ajFalse;

    ajFmtPrintF(outf, "##fileformat=VCFv4.1\n");

    varheader = var->Header;

    /* header */

    iter = ajListIterNewread(varheader->Header);
    while(!ajListIterDone(iter))
    {
        tagval = ajListIterGet(iter);
        if(!ajStrMatchC( MAJTAGVALGETTAG(tagval), "fileformat"))
            ajFmtPrintF(outf, "##%S=%S\n",
                        MAJTAGVALGETTAG(tagval),
                        MAJTAGVALGETVALUE(tagval));
    }
    ajListIterDel(&iter);

    iter = ajListIterNewread(varheader->Fields);
    while(!ajListIterDone(iter))
    {
        varfield = ajListIterGet(iter);
        ajFmtPrintF(outf,
                    "##%S=<ID=%S,Number=%s,Type=%s,Description=\"%S\">\n",
                    varfield->Field,
                    varfield->Id,
                    ajVarfieldGetNumber(varfield),
                    ajVarfieldGetType(varfield),
                    varfield->Desc);
    }
    ajListIterDel(&iter);

    iter = ajListIterNewread(varheader->Filters);
    while(!ajListIterDone(iter))
    {
        tagval = ajListIterGet(iter);
        ajFmtPrintF(outf,
                    "##FILTER=<ID=%S,Description=\"%S\">\n",
                    MAJTAGVALGETTAG(tagval),
                    MAJTAGVALGETVALUE(tagval));
    }
    ajListIterDel(&iter);

    iter = ajListIterNewread(varheader->Alts);
    while(!ajListIterDone(iter))
    {
        tagval = ajListIterGet(iter);
        ajFmtPrintF(outf,
                    "##ALT=<ID=%S,Description=\"%S\">\n",
                    MAJTAGVALGETTAG(tagval),
                    MAJTAGVALGETVALUE(tagval));
    }
    ajListIterDel(&iter);

    iter = ajListIterNewread(varheader->Samples);
    while(!ajListIterDone(iter))
    {
        varsample = ajListIterGet(iter);
        ajFmtPrintF(outf,
                    "##SAMPLE=<ID=%S,Genomes=",
                    varsample->Id);

        for(i=0; i < varsample->Number; i++)
        {
            if(i)
                ajFmtPrintF(outf, ";");
            ajFmtPrintF(outf, "%S", varsample->Genomes[i]);
        }

        ajFmtPrintF(outf, ",Mixture=");

        for(i=0; i < varsample->Number; i++)
        {
            if(i)
                ajFmtPrintF(outf, ";");
            ajStrFromFloat(&tmpstr, varsample->Mixture[i], 4);
            ajStrTrimC(&tmpstr, "0");
            ajFmtPrintF(outf, "%S", tmpstr);
        }
        ajStrDel(&tmpstr);

        ajFmtPrintF(outf, ",Description=\"");

        for(i=0; i < varsample->Number; i++)
        {
            if(i)
                ajFmtPrintF(outf, ";");
            ajFmtPrintF(outf, "%S", varsample->Desc[i]);
        }

        ajFmtPrintF(outf, "\">\n");

    }

    ajListIterDel(&iter);

    iter = ajListIterNewread(varheader->Pedigrees);
    while(!ajListIterDone(iter))
    {
        tagval = ajListIterGet(iter);
        ajFmtPrintF(outf,
                    "##PEDIGREE=%S\n",
                    MAJTAGVALGETVALUE(tagval));
    }
    ajListIterDel(&iter);

    ajFmtPrintF(outf, "#CHROM\tPOS\tID\tREF\tALT\tQUAL\tFILTER\tINFO\tFORMAT");
    iter = ajListIterNewread(varheader->SampleIds);
    while(!ajListIterDone(iter))
    {
        tmpstr = ajListIterGet(iter);
        ajFmtPrintF(outf, "\t%S", tmpstr);
    }
    ajListIterDel(&iter);
    ajFmtPrintF(outf, "\n");

    /* data */

    iter = ajListIterNewread(var->Data);
    while(!ajListIterDone(iter))
    {
        vardata = ajListIterGet(iter);
        ajFmtPrintF(outf,
                    "%S\t%u\t%S\t%S\t%S\t%S\t%S\t%S\t%S",
                    vardata->Chrom, vardata->Pos, vardata->Id,
                    vardata->Ref, vardata->Alt,
                    vardata->Qual, vardata->Filter,
                    vardata->Info, vardata->Format);
        itersam = ajListIterNewread(vardata->Samples);
        while(!ajListIterDone(itersam))
        {
            tmpstr = ajListIterGet(itersam);
            ajFmtPrintF(outf,
                    "\t%S",
                        tmpstr);
        }
        ajListIterDel(&itersam);
        ajFmtPrintF(outf, "\n");
    }
    ajListIterDel(&iter);

    return ajTrue;
}




/* @funcstatic varoutWriteNextVcf41 *******************************************
**
** Write partial variation data in VCFv4.1 format
**
** @param [u] outf [AjPFile] Output file
** @param [r] var [const AjPVar] Variation data object
**
** @return [AjBool] True on success
**
** @release 6.5.0
**
******************************************************************************/

static AjBool varoutWriteNextVcf41(AjPFile outf, const AjPVar var)
{
    AjPVarHeader varheader = NULL;
    AjPVarData   vardata   = NULL;
    AjPVarField  varfield  = NULL;
    AjPVarSample varsample = NULL;
    AjPTagval tagval = NULL;
    AjIList iter = NULL;
    AjIList itersam = NULL;
    AjPStr tmpstr = NULL;
    ajuint i;

    if(!outf) return ajFalse;
    if(!var) return ajFalse;

    ajDebug("varoutWriteNextVcf41 Hasdata: %B data: %Lu\n",
            var->Hasdata, ajListGetLength(var->Data));
    if(!var->Hasdata)
    {
        ajFmtPrintF(outf, "##fileformat=VCFv4.1\n");

        varheader = var->Header;

        /* header */

        iter = ajListIterNewread(varheader->Header);
        while(!ajListIterDone(iter))
        {
            tagval = ajListIterGet(iter);
            if(!ajStrMatchC( MAJTAGVALGETTAG(tagval), "fileformat"))
                ajFmtPrintF(outf, "##%S=%S\n",
                            MAJTAGVALGETTAG(tagval),
                            MAJTAGVALGETVALUE(tagval));
        }
        ajListIterDel(&iter);

        iter = ajListIterNewread(varheader->Fields);
        while(!ajListIterDone(iter))
        {
            varfield = ajListIterGet(iter);
            ajFmtPrintF(outf,
                        "##%S=<ID=%S,Number=%s,Type=%s,Description=\"%S\">\n",
                        varfield->Field,
                        varfield->Id,
                        ajVarfieldGetNumber(varfield),
                        ajVarfieldGetType(varfield),
                        varfield->Desc);
        }
        ajListIterDel(&iter);

        iter = ajListIterNewread(varheader->Filters);
        while(!ajListIterDone(iter))
        {
            tagval = ajListIterGet(iter);
            ajFmtPrintF(outf,
                        "##FILTER=<ID=%S,Description=\"%S\">\n",
                        MAJTAGVALGETTAG(tagval),
                        MAJTAGVALGETVALUE(tagval));
        }
        ajListIterDel(&iter);

        iter = ajListIterNewread(varheader->Alts);
        while(!ajListIterDone(iter))
        {
            tagval = ajListIterGet(iter);
            ajFmtPrintF(outf,
                        "##ALT=<ID=%S,Description=\"%S\">\n",
                        MAJTAGVALGETTAG(tagval),
                        MAJTAGVALGETVALUE(tagval));
        }
        ajListIterDel(&iter);

        iter = ajListIterNewread(varheader->Samples);
        while(!ajListIterDone(iter))
        {
            varsample = ajListIterGet(iter);
            ajFmtPrintF(outf,
                        "##SAMPLE=<ID=%S,Genomes=",
                        varsample->Id);

            for(i=0; i < varsample->Number; i++)
            {
                if(i)
                    ajFmtPrintF(outf, ";");
                ajFmtPrintF(outf, "%S", varsample->Genomes[i]);
            }

            ajFmtPrintF(outf, ",Mixture=");

            for(i=0; i < varsample->Number; i++)
            {
                if(i)
                    ajFmtPrintF(outf, ";");
                ajStrFromFloat(&tmpstr, varsample->Mixture[i], 4);
                ajStrTrimC(&tmpstr, "0");
                ajFmtPrintF(outf, "%S", tmpstr);
            }
            ajStrDel(&tmpstr);

            ajFmtPrintF(outf, ",Description=\"");
            
            for(i=0; i < varsample->Number; i++)
            {
                if(i)
                    ajFmtPrintF(outf, ";");
                ajFmtPrintF(outf, "%S", varsample->Desc[i]);
            }

            ajFmtPrintF(outf, "\">\n");

        }

        ajListIterDel(&iter);

        iter = ajListIterNewread(varheader->Pedigrees);
        while(!ajListIterDone(iter))
        {
            tagval = ajListIterGet(iter);
            ajFmtPrintF(outf,
                        "##PEDIGREE=%S\n",
                        MAJTAGVALGETVALUE(tagval));
        }
        ajListIterDel(&iter);

        ajFmtPrintF(outf, "#CHROM\tPOS\tID\tREF\tALT\tQUAL\tFILTER"
                    "\tINFO\tFORMAT");
        iter = ajListIterNewread(varheader->SampleIds);
        while(!ajListIterDone(iter))
        {
            tmpstr = ajListIterGet(iter);
            ajFmtPrintF(outf, "\t%S", tmpstr);
        }
        ajListIterDel(&iter);
        ajFmtPrintF(outf, "\n");

        return ajTrue;
    }

    /* data */

    iter = ajListIterNewread(var->Data);
    while(!ajListIterDone(iter))
    {
        vardata = ajListIterGet(iter);
        ajFmtPrintF(outf,
                    "%S\t%u\t%S\t%S\t%S\t%S\t%S\t%S\t%S",
                    vardata->Chrom, vardata->Pos, vardata->Id,
                    vardata->Ref, vardata->Alt,
                    vardata->Qual, vardata->Filter,
                    vardata->Info, vardata->Format);
        itersam = ajListIterNewread(vardata->Samples);
        while(!ajListIterDone(itersam))
        {
            tmpstr = ajListIterGet(itersam);
            ajFmtPrintF(outf,
                    "\t%S",
                        tmpstr);
        }
        ajListIterDel(&itersam);
        ajFmtPrintF(outf, "\n");
    }
    ajListIterDel(&iter);

    return ajTrue;
}




/* @datasection [none] Miscellaneous functions ********************************
**
** Functions to initialise and clean up internals
**
** @nam2rule Varout Variation output internals
**
******************************************************************************/




/* @section Printing **********************************************************
**
** Printing details of the internals to a file
**
** @fdata [none]
**
** @nam2rule Varoutprint
**
** @fcategory output
**
******************************************************************************/




/* @section Print *************************************************************
**
** Printing to a file
**
** @fdata [none]
**
** @nam3rule Book Print as docbook table
** @nam3rule Html Print as html table
** @nam3rule Wiki Print as wiki table
** @nam3rule Text Print as text
**
** @argrule * outf [AjPFile] output file
** @argrule Text full [AjBool] Print all details
**
** @valrule * [void]
**
** @fcategory cast
**
******************************************************************************/




/* @func ajVaroutprintBook ****************************************************
**
** Reports the variation format internals as Docbook text
**
** @param [u] outf [AjPFile] Output file
** @return [void]
**
** @release 6.4.0
** @@
******************************************************************************/

void ajVaroutprintBook(AjPFile outf)
{
    ajint i = 0;

    ajFmtPrintF(outf, "<para>The supported variation output "
                "formats are summarised "
                "in the table below. The columns are as follows: "
                "<emphasis>Output format</emphasis> (format name), "
                 "<emphasis>Description</emphasis> (short description of "
                "the format).</para> \n\n");

    ajFmtPrintF(outf, "<table frame=\"box\" rules=\"cols\">\n");
    ajFmtPrintF(outf, "  <caption>Variation output formats</caption>\n");
    ajFmtPrintF(outf, "  <thead>\n");
    ajFmtPrintF(outf, "    <tr align=\"center\">\n");
    ajFmtPrintF(outf, "      <th>Output Format</th>\n");
    ajFmtPrintF(outf, "      <th>Description</th>\n");
    ajFmtPrintF(outf, "    </tr>\n");
    ajFmtPrintF(outf, "  </thead>\n");
    ajFmtPrintF(outf, "  <tbody>\n");

    for(i=0; varoutFormatDef[i].Name; i++)
    {
        ajFmtPrintF(outf, "    <tr>\n");
        ajFmtPrintF(outf, "      <td>%s</td>\n",
                    varoutFormatDef[i].Name);
        ajFmtPrintF(outf, "      <td>%s</td>\n",
                    varoutFormatDef[i].Desc);
    }

    ajFmtPrintF(outf, "  </tbody>\n");
    ajFmtPrintF(outf, "</table>\n");

    return;
}




/* @func ajVaroutprintHtml ****************************************************
**
** Reports the internal data structures
**
** @param [u] outf [AjPFile] Output file
** @return [void]
**
** @release 6.4.0
** @@
******************************************************************************/

void ajVaroutprintHtml(AjPFile outf)
{
    ajint i = 0;

    ajFmtPrintF(outf, "<table border=3>");
    ajFmtPrintF(outf, "<tr><th>Variation Format</th>\n");
    ajFmtPrintF(outf, "<th>Description</th></tr>\n");
    ajFmtPrintF(outf, "\n");
    ajFmtPrintF(outf, "# variation output formats\n");
    ajFmtPrintF(outf, "# Name    Format name (or alias)\n");
    ajFmtPrintF(outf, "# Desc    Format description\n");
    ajFmtPrintF(outf, "# Name         Description\n");
    ajFmtPrintF(outf, "\n");
    ajFmtPrintF(outf, "VarFormat {\n");

    for(i=0; varoutFormatDef[i].Name; i++)
    {
        ajFmtPrintF(outf, "<tr><td>\n%-12s\n"
                        "<td>\"%s\"</td></tr>\n",
			varoutFormatDef[i].Name,
			varoutFormatDef[i].Desc);
    }

    ajFmtPrintF(outf, "}\n\n");

    return;
}




/* @func ajVaroutprintText ****************************************************
**
** Reports the internal data structures
**
** @param [u] outf [AjPFile] Output file
** @param [r] full [AjBool] Full report (usually ajFalse)
** @return [void]
**
** @release 6.4.0
** @@
******************************************************************************/

void ajVaroutprintText(AjPFile outf, AjBool full)
{
    ajint i = 0;

    ajFmtPrintF(outf, "\n");
    ajFmtPrintF(outf, "# Variation output formats\n");
    ajFmtPrintF(outf, "# Name    Format name (or alias)\n");
    ajFmtPrintF(outf, "# Desc    Format description\n");
    ajFmtPrintF(outf, "# Name         Description\n");
    ajFmtPrintF(outf, "\n");
    ajFmtPrintF(outf, "VarFormat {\n");

    for(i=0; varoutFormatDef[i].Name; i++)
    {
	if(full)
	    ajFmtPrintF(outf, "  %-12s \"%s\"\n",
			varoutFormatDef[i].Name,
			varoutFormatDef[i].Desc);
    }

    ajFmtPrintF(outf, "}\n\n");

    return;
}




/* @func ajVaroutprintWiki ****************************************************
**
** Reports the variation output format internals as wikitext
**
** @param [u] outf [AjPFile] Output file
** @return [void]
**
** @release 6.4.0
** @@
******************************************************************************/

void ajVaroutprintWiki(AjPFile outf)
{
    ajint i = 0;
    ajint j = 0;
    AjPStr namestr = NULL;

    ajFmtPrintF(outf, "{| class=\"wikitable sortable\" border=\"2\"\n");
    ajFmtPrintF(outf, "|-\n");
    ajFmtPrintF(outf, "!Format!!"
                "class=\"unsortable\"|Description\n");

    for(i=0; varoutFormatDef[i].Name; i++)
    {
        ajFmtPrintF(outf, "|-\n");
        ajStrAssignC(&namestr, varoutFormatDef[i].Name);

        for(j=i+1; varoutFormatDef[j].Name; j++)
        {
            if(varoutFormatDef[j].Write == varoutFormatDef[i].Write)
            {
                ajFmtPrintAppS(&namestr, "<br>%s",
                               varoutFormatDef[j].Name);
                ajWarn("Variation output format '%s' same as '%s' "
                       "but not alias",
                       varoutFormatDef[j].Name,
                       varoutFormatDef[i].Name);
            }
        }
        ajFmtPrintF(outf, "|%S||%s\n",
		    namestr,
		    varoutFormatDef[i].Desc);
    }

    ajFmtPrintF(outf, "|}\n");

    ajStrDel(&namestr);

    return;
}




/* @datasection [none] Output formats *****************************************
**
** Output formats internals
**
** @nam2rule Varoutformat Variation output format specific
**
******************************************************************************/




/* @section cast **************************************************************
**
** Values for output formats
**
** @fdata [none]
**
** @nam3rule Find Return index to named format
** @nam3rule Test Test format value
**
** @argrule * format [const AjPStr] Format name
** @argrule Find iformat [ajint*] Index matching format name
**
** @valrule * [AjBool] True if found
**
** @fcategory cast
**
******************************************************************************/




/* @func ajVaroutformatFind ***************************************************
**
** Looks for the specified format(s) in the internal definitions and
** returns the index.
**
** Sets iformat as the recognised format, and returns ajTrue.
**
** @param [r] format [const AjPStr] Format required.
** @param [w] iformat [ajint*] Index
** @return [AjBool] ajTrue on success.
**
** @release 6.4.0
** @@
******************************************************************************/

AjBool ajVaroutformatFind(const AjPStr format, ajint* iformat)
{
    AjPStr tmpformat = NULL;
    ajuint i = 0;

    /* ajDebug("ajVaroutformatFind '%S'\n", format); */
    if(!ajStrGetLen(format))
	return ajFalse;

    ajStrAssignS(&tmpformat, format);
    ajStrFmtLower(&tmpformat);

    for(i=0; varoutFormatDef[i].Name; i++)
    {
	/* ajDebug("test %d '%s' \n", i, varoutFormatDef[i].Name); */
	if(ajStrMatchCaseC(tmpformat, varoutFormatDef[i].Name))
	{
	    *iformat = i;
	    ajStrDel(&tmpformat);
	    /* ajDebug("found '%s' at %d\n", varoutFormatDef[i].Name, i); */
	    return ajTrue;
	}
    }

    ajErr("Unknown output format '%S'", format);

    ajStrDel(&tmpformat);

    *iformat = 0;

    return ajFalse;
}




/* @func ajVaroutformatTest ***************************************************
**
** Tests whether a named variation output format is known
**
** @param [r] format [const AjPStr] Format
** @return [AjBool] ajTrue if formats was accepted
**
** @release 6.4.0
** @@
******************************************************************************/

AjBool ajVaroutformatTest(const AjPStr format)
{
    ajint i;

    for (i=0; varoutFormatDef[i].Name; i++)
	if(ajStrMatchCaseC(format, varoutFormatDef[i].Name))
	    return ajTrue;

    return ajFalse;
}




/* @datasection [none] Miscellaneous functions ********************************
**
** Functions to initialise and clean up internals
**
** @nam2rule Varout Variation output internals
**
******************************************************************************/




/* @section Miscellaneous *****************************************************
**
** Functions to initialise and clean up internals
**
** @fdata [none]
**
** @nam3rule Exit Clean up and exit
**
** @valrule * [void]
**
** @fcategory misc
**
******************************************************************************/




/* @func ajVaroutExit *********************************************************
**
** Cleans up variation output internal memory
**
** @return [void]
**
** @release 6.4.0
** @@
******************************************************************************/

void ajVaroutExit(void)
{
    return;
}
