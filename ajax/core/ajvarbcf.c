/* @source ajvarbcf ***********************************************************
**
** BCF files reading/writing functions
**
** @version $Revision: 1.5 $
** @modified 2011 Mahmut Uludag ported from bcftools project (samtools.sf.net)
** @modified $Date: 2012/07/14 14:52:39 $ by $Author: rice $
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

/* The MIT License

   Copyright (c) 2010 Broad Institute

   Permission is hereby granted, free of charge, to any person obtaining
   a copy of this software and associated documentation files (the
   "Software"), to deal in the Software without restriction, including
   without limitation the rights to use, copy, modify, merge, publish,
   distribute, sublicense, and/or sell copies of the Software, and to
   permit persons to whom the Software is furnished to do so, subject to
   the following conditions:

   The above copyright notice and this permission notice shall be
   included in all copies or substantial portions of the Software.

   THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
   EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
   MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
   NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS
   BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN
   ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
   CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
   SOFTWARE.
*/

#include <string.h>
#include <ctype.h>
#include <stdio.h>

#include "ajstr.h"
#include "ajfmt.h"
#include "ajseqbam.h"
#include "ajsys.h"
#include "ajmess.h"
#include "ajmem.h"
#include "ajvarbcf.h"




/* @func ajVarbcfHdrWrite *****************************************************
**
** Write BCF header data
**
** @param [u] fp [AjPVarBcfFile] bcf file pointer
** @param [r] h [const AjPVarBcfHeader] bcf header
**
** @return [int] length of data written
**
**
** @release 6.5.0
******************************************************************************/

int ajVarbcfHdrWrite(AjPVarBcfFile fp, const AjPVarBcfHeader h)
{
	if (fp == 0 || h == 0) return -1;

	bgzf_write(fp, "BCF\4", 4);
	bgzf_write(fp, &h->l_nm, 4);
	bgzf_write(fp, h->name, h->l_nm);
	bgzf_write(fp, &h->l_smpl, 4);
	bgzf_write(fp, h->sname, h->l_smpl);
	bgzf_write(fp, &h->l_txt, 4);
	bgzf_write(fp, h->txt, h->l_txt);

	ajSeqBamBgzfFlush(fp);

	return 16 + h->l_nm + h->l_smpl + h->l_txt;
}




/* @func ajVarbcfHdrRead ******************************************************
**
** Read BCF header data
**
** @param [u] fp [AjPVarBcfFile] bcf object including file pointer
** @return [AjPVarBcfHeader] bcf header
**
**
** @release 6.5.0
******************************************************************************/

AjPVarBcfHeader ajVarbcfHdrRead(AjPVarBcfFile fp)
{
    unsigned char magic[4];
    AjPVarBcfHeader h;

    if (fp == 0) return 0;

    AJNEW0(h);

    bgzf_read(fp, magic, 4);
    bgzf_read(fp, &h->l_nm, 4);
    h->name = malloc(h->l_nm);
    bgzf_read(fp, h->name, h->l_nm);
    bgzf_read(fp, &h->l_smpl, 4);
    h->sname = malloc(h->l_smpl);
    bgzf_read(fp, h->sname, h->l_smpl);
    bgzf_read(fp, &h->l_txt, 4);
    h->txt = malloc(h->l_txt);
    bgzf_read(fp, h->txt, h->l_txt);
    ajVarbcfHdrSync(h);
    return h;
}




/* @func ajVarbcfHdrDel *******************************************************
**
** Delete BCF header data
**
** @param [d] h [AjPVarBcfHeader] bcf header
** @return [void]
**
**
** @release 6.5.0
******************************************************************************/

void ajVarbcfHdrDel(AjPVarBcfHeader h)
{
    if (h == 0) return;

    AJFREE(h->name);
    AJFREE(h->sname);
    AJFREE(h->txt);
    AJFREE(h->ns);
    AJFREE(h->sns);
    AJFREE(h);
}




/* @funcstatic varbcfConcatenatedlines ****************************************
**
** returns array of lines in concatenated reference-sequence/sample-name lines
**
** @param [r] l [int] length of the text
** @param [r] str [const char*] text, concatenated lines
** @param [u] nlines [int*] number of lines
** @return [const char**] array of lines in the text
**
**
** @release 6.5.0
******************************************************************************/

static inline const char ** varbcfConcatenatedlines(int l, const char *str,
					                                 int *nlines)
{
    int n = 0;
    const char *p;
    const char **list;

    *nlines = 0;

    if (l == 0 || str == 0) return 0;

    for (p = str; p != str + l; ++p)
	if (*p == 0) ++n;

    *nlines = n;
    list = calloc(n, sizeof(void*));
    list[0] = str;

    for (p = str, n = 1; p < str + l - 1; ++p)
	if (*p == 0) list[n++] = p + 1;

    return list;
}




/* @func ajVarbcfHdrSync ******************************************************
**
** Sync BCF header data (set bcf_hdr_t::ns and bcf_hdr_t::sns)
**
** @param [u] b [AjPVarBcfHeader] bcf header
** @return [int] 0 on success, -1 if the header object was null
**
**
** @release 6.5.0
******************************************************************************/

int ajVarbcfHdrSync(AjPVarBcfHeader b)
{
    if (b == 0) return -1;

    if (b->ns)
	free(b->ns);

    if (b->sns)
	free(b->sns);

    if (b->l_nm)
	b->ns = varbcfConcatenatedlines(b->l_nm, b->name, &b->n_ref);
    else
	b->ns = 0, b->n_ref = 0;

    b->sns = varbcfConcatenatedlines(b->l_smpl, b->sname, &b->n_smpl);

    return 0;
}




/* @func ajVarbcfSync *********************************************************
**
** Sync individual BCF variation records
**
** call this function if b->str is changed
**
** @param [u] b [AjPVarBcf] bcf variation record
** @return [int] 0 on success, -1 on errors
**
**
** @release 6.5.0
******************************************************************************/

int ajVarbcfSync(AjPVarBcf b)
{
    char *p= NULL;
    char *tmp[5];
    const char* save=NULL;
    AjPStr auxS=NULL;
    ajint i=0;
    ajint n=0;
    ajint n_smpl = b->n_smpl;
    ajint oldlen;

    /* set ref, alt, flt, info, fmt */
    b->ref = b->alt = b->flt = b->info = b->fmt = 0;
    for (p = b->str, n = 0; p < b->str + b->l_str; ++p)
    {
	if (*p == 0 && p+1 != b->str + b->l_str)
	{
	    if (n == 5)
	    {
		++n;
		break;
	    }
	    else tmp[n++] = p + 1;
	}
    }

    if (n != 5)
    {
	ajErr("[%s] incorrect number of fields (%d != 5) at %d:%d\n",
	      "ajVarbcfSync", n, b->tid, b->pos);
	return -1;
    }
    b->ref = tmp[0];
    b->alt = tmp[1];
    b->flt = tmp[2];
    b->info = tmp[3];
    b->fmt = tmp[4];

    /* set n_alleles */
    if (*b->alt == 0)
	b->n_alleles = 1;
    else
    {
	for (p = b->alt, n = 1; *p; ++p)
	    if (*p == ',') ++n;
	b->n_alleles = n + 1;
    }

    /* set n_gi and gi[i].fmt */
    for (p = b->fmt, n = 1; *p; ++p)
	if (*p == ':') ++n;
    if (n > b->m_gi)
    {
	int old_m = b->m_gi;
	b->m_gi = n;
	kroundup32(b->m_gi);
	b->gi = realloc(b->gi, b->m_gi * sizeof(AjOVarBcfGinfo));
	memset(b->gi + old_m, 0, (b->m_gi - old_m) * sizeof(AjOVarBcfGinfo));
    }
    b->n_gi = n;

    save=b->fmt;

    for (n = 0; ajSysFuncStrtokR(NULL, ":", &save, &auxS);)
    {
	b->gi[n++].fmt = bcf_str2int(ajStrGetPtr(auxS), ajStrGetLen(auxS));
    }

    /* set gi[i].len */
    for (i = 0; i < b->n_gi; ++i)
    {
        oldlen = b->gi[i].len;
	if (b->gi[i].fmt == bcf_str2int("PL", 2))
	{
	    b->gi[i].len = b->n_alleles * (b->n_alleles + 1) / 2;
	}
	else if (b->gi[i].fmt == bcf_str2int("DP", 2) ||
		b->gi[i].fmt == bcf_str2int("HQ", 2))
	{
	    b->gi[i].len = 2;
	}
	else if (b->gi[i].fmt == bcf_str2int("GQ", 2) ||
		b->gi[i].fmt == bcf_str2int("GT", 2))
	{
	    b->gi[i].len = 1;
	}
	else if (b->gi[i].fmt == bcf_str2int("SP", 2))
	{
	    b->gi[i].len = 4;
	}
	else if (b->gi[i].fmt == bcf_str2int("GL", 2))
	{
	    b->gi[i].len = b->n_alleles * (b->n_alleles + 1) / 2 * 4;
	}
        if(b->gi[i].len > oldlen)
            AJRESIZE0(b->gi[i].data, n_smpl * oldlen, n_smpl * b->gi[i].len);
    }

    ajStrDel(&auxS);

    return 0;
}




/* @func ajVarbcfWrite ********************************************************
**
** Write individual BCF variation records
**
** @param [u] fp [AjPVarBcfFile] bcf object including file pointer
** @param [r] nsmpl [ajint] Number of samples
** @param [r] b [const AjPVarBcf] individual bcf variation record
** @return [int] length of data written?
**
**
** @release 6.5.0
******************************************************************************/

int ajVarbcfWrite(AjPVarBcfFile fp, ajint nsmpl, const AjPVarBcf b)
{
    int i=0;
    int l=0;

    if (b == 0) return -1;

    bgzf_write(fp, &b->tid, 4);
    bgzf_write(fp, &b->pos, 4);
    bgzf_write(fp, &b->qual, 4);
    bgzf_write(fp, &b->l_str, 4);
    bgzf_write(fp, b->str, b->l_str);
    l = 12 + b->l_str;

    for (i = 0; i < b->n_gi; ++i)
    {
	bgzf_write(fp, b->gi[i].data, b->gi[i].len * nsmpl);
	l += b->gi[i].len * nsmpl;
    }

    return l;
}




/* @func ajVarbcfRead *********************************************************
**
** Read individual BCF variation records
**
*
** @param [u] fp [AjPVarBcfFile] bcf file
** @param [r] nsmpl [ajint] Number of samples
** @param [u] b [AjPVarBcf] individual bcf variation record to be read
** @return [int] length of data written, -1 on end-of-file, and < -1 for errors
**
**
** @release 6.5.0
******************************************************************************/

int ajVarbcfRead(AjPVarBcfFile fp, ajint nsmpl, AjPVarBcf b)
{
    int i=0;
    int l=0;
    int oldl=0;

    if (b == 0) return -1;

    if (bgzf_read(fp, &b->tid, 4) == 0) return -1;

    b->n_smpl = nsmpl;
    bgzf_read(fp, &b->pos, 4);
    bgzf_read(fp, &b->qual, 4);
    bgzf_read(fp, &b->l_str, 4);

    if (b->l_str > b->m_str)
    {
        oldl = b->l_str;
	b->m_str = b->l_str;
	kroundup32(b->m_str);
        AJRESIZE0(b->str, oldl, b->m_str);
    }

    bgzf_read(fp, b->str, b->l_str);
    l = 12 + b->l_str;

    if (ajVarbcfSync(b) < 0)
	return -2;

    for (i = 0; i < b->n_gi; ++i)
    {
	bgzf_read(fp, b->gi[i].data, b->gi[i].len * nsmpl);
	l += b->gi[i].len * nsmpl;
    }

    return l;
}




/* @func ajVarbcfDel **********************************************************
**
** Delete individual BCF variation records
**
** @param [d] b [AjPVarBcf] individual bcf variation record to be deleted
** @return [int] 0 if the object was deleted, -1 if the object pointer was null
**
**
** @release 6.5.0
******************************************************************************/

int ajVarbcfDel(AjPVarBcf b)
{
    int i;

    if (b == 0) return -1;

    free(b->str);

    for (i = 0; i < b->m_gi; ++i)
	free(b->gi[i].data);

    free(b->gi);
    free(b);

    return 0;
}




/* @funcstatic varbcfFmtStr **********************************************
**
** For null values in BCF a '.' character is printed in VCF
**
** @param [r] p [const char*] field/option value in bcf
** @param [u] s [AjPStr *] vcf string to be updated
** @return [void]
**
******************************************************************************/

static inline void varbcfFmtStr(const char *p, AjPStr* s)
{
    if (*p == 0)
	ajStrAppendK(s, '.');
    else
	ajStrAppendC(s, p);
}




/* @func ajVarbcfFmtCore ******************************************************
**
** Write individual BCF variation records into a string in VCF format
**
** @param [r] h [const AjPVarBcfHeader] bcf header object
** @param [u] b [AjPVarBcf] individual bcf variation record to be read
** @param [u] s [AjPStr*] string for vcf representation of the record
** @return [void]
**
**
** @release 6.5.0
******************************************************************************/

void ajVarbcfFmtCore(const AjPVarBcfHeader h, AjPVarBcf b, AjPStr* s)
{
    int i, j, x;
    int k;
    unsigned char *d=NULL;
    float *f=NULL;

    if (h->n_ref)
	ajStrAppendC(s, h->ns[b->tid]);
    else
	ajFmtPrintAppS(s, "%d", b->tid);

    ajStrAppendK(s, '\t');

    ajFmtPrintAppS(s,"%d\t", b->pos+1);

    varbcfFmtStr(b->str, s);
    ajStrAppendK(s, '\t');

    varbcfFmtStr(b->ref, s);
    ajStrAppendK(s, '\t');

    varbcfFmtStr(b->alt, s);
    ajStrAppendK(s, '\t');

    ajFmtPrintAppS(s, "%.3g", b->qual);
    ajStrAppendK(s, '\t');

    varbcfFmtStr(b->flt, s);
    ajStrAppendK(s, '\t');

    varbcfFmtStr(b->info, s);
    if (b->fmt[0])
    {
	ajStrAppendK(s, '\t');
	varbcfFmtStr(b->fmt, s);
    }

    x = b->n_alleles * (b->n_alleles + 1) / 2;

    if (b->n_gi == 0) return;

    for (j = 0; j < h->n_smpl; ++j)
    {
	ajStrAppendK(s, '\t');

	for (i = 0; i < b->n_gi; ++i)
	{
	    if (i)
		ajStrAppendK(s, ':');

	    if (b->gi[i].fmt == bcf_str2int("PL", 2))
	    {
		d = (unsigned char*)b->gi[i].data + j * x;

		for (k = 0; k < x; ++k)
		{
		    if (k > 0)
			ajStrAppendK(s, ',');

		    ajFmtPrintAppS(s, "%u", d[k]);
		}
	    }
	    else if (b->gi[i].fmt == bcf_str2int("DP", 2))
	    {
		ajFmtPrintAppS(s, "%u",((ajushort*)b->gi[i].data)[j]);
	    }
	    else if (b->gi[i].fmt == bcf_str2int("GQ", 2))
	    {
		ajFmtPrintAppS(s, "%u",((unsigned char*)b->gi[i].data)[j]);
	    }
	    else if (b->gi[i].fmt == bcf_str2int("SP", 2))
	    {
		ajFmtPrintAppS(s, "%u",((ajint*)b->gi[i].data)[j]);
	    }
	    else if (b->gi[i].fmt == bcf_str2int("GT", 2))
	    {
		int y = ((unsigned char*)b->gi[i].data)[j];

		if (y>>7&1)
		{
		    ajStrAppendC(s, "./.");
		} else {
		    ajStrAppendK(s, '0' + (y>>3&7));
		    ajStrAppendK(s, (y>>6&1  ? '|' : '/'));
		    ajStrAppendK(s, '0' + (y&7));
		}
	    }
	    else if (b->gi[i].fmt == bcf_str2int("GL", 2))
	    {
		f = (float*)b->gi[i].data + j * x;

		for (k = 0; k < x; ++k)
		{
		    if (k > 0)
			ajStrAppendK(s, ',');

		    ajFmtPrintAppS(s, "%.2f", f[k]);
		}
	    }
	    else ajStrAppendK(s, '.'); /* custom fields */
	}
    }
}
