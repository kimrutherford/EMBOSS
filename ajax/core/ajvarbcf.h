/* @include ajvarbcf **********************************************************
**
** BCF files reading/writing functions
**
** @version $Revision: 1.6 $
** @modified 2011 Mahmut Uludag ported from bcftools project (samtools.sf.net)
** @modified $Date: 2012/07/14 16:51:22 $ by $Author: rice $
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


#ifndef AJVARBCF_H
#define AJVARBCF_H


#define BCF_VERSION "0.1.17-dev (r973:277)"

#define AjPVarBcfFile AjOSeqBamBgzf*

#define bgzf_read(fp, buf, len) ajSeqBamBgzfRead(fp, buf, len)
#define bgzf_write(fp, buf, len) ajSeqBamBgzfWrite(fp, buf, len)

/*
  A member in the structs below is said to "primary" if its content
  cannot be inferred from other members in any of structs below; a
  member is said to be "derived" if its content can be derived from
  other members. For example, AjOVarBcf::str is primary as this comes from
  the input data, while AjOVarBcf::info is derived as it can always be
  correctly set if we know AjOVarBcf::str. Derived members are for quick
  access to the content and must be synchronized with the primary data.
 */


/* @data AjPVarBcfGinfo *******************************************************
**
** Individual BCF variation records
**
** derived info: fmt, len (<-AjOVarBcf::fmt)
**
** @alias AjOVarBcfGinfo
** @alias AjSVarBcfGinfo
**
** @attr fmt [ajuint] format of the block, set by bcf_str2int()
** @attr len [ajint] length of data for each individual
** @attr data [void*] concatenated data
**
******************************************************************************/

typedef struct AjSVarBcfGinfo
{
    ajuint fmt;
    ajint len;
    void *data;
} AjOVarBcfGinfo;

#define AjPVarBcfGinfo AjOVarBcfGinfo*




/* @data AjPVarBcf ********************************************************
**
** Individual BCF variation records
**
** derived info: ref, alt, flt, info, fmt (<-str),
**               n_gi (<-fmt),
**               n_alleles (<-alt),
**               n_smpl (<-bcf_hdr_t::n_smpl)
**
** @alias AjOVarBcf
** @alias AjSVarBcf
**
** @attr tid [ajint] reference sequence id
** @attr pos [ajint] 0-based position
** @attr l_str [ajint] length of ->str
** @attr m_str [ajint] allocated size of ->str
** @attr qual [float] SNP quality
** @attr Padding [ajuint] Padding to alignment boundary
** @attr str [char*] concatenated string of variable length strings in VCF
** 		     (from col.2 to col.7)
** @attr ref [char*] reference sequence name (pointer to ->str)
** @attr alt [char*] alternate allele(s) (pointer to ->str)
** @attr flt [char*] filters applied (pointer to ->str)
** @attr info [char*] variant information (pointer to ->str)
** @attr fmt [char*] format of the genotype fields (pointer to ->str)
** @attr n_gi [ajint] number of genotype fields
** @attr m_gi [ajint] allocated size genotype fields
** @attr gi [AjOVarBcfGinfo*] array of genotype fields
** @attr n_alleles [ajint] number of alleles
** @attr n_smpl [ajint] number of samples
**
******************************************************************************/

typedef struct AjSVarBcf
{
    ajint tid;
    ajint pos;
    ajint l_str;
    ajint m_str;
    float qual;
    ajuint Padding;
    char *str;
    char *ref;
    char *alt;
    char *flt;
    char *info;
    char *fmt;
    ajint n_gi;
    ajint m_gi;
    AjOVarBcfGinfo *gi;
    ajint n_alleles;
    ajint n_smpl;
} AjOVarBcf;

#define AjPVarBcf AjOVarBcf*




/* @data AjPVarBcfHeader ******************************************************
**
** BCF header
**
** derived info: n_ref (<-name),
** 		 n_smpl (<-sname),
** 		 ns (<-name),
** 		 sns (<-sname)
**
** @alias AjOVarBcfHeader
** @alias AjSVarBcfHeader
**
** @attr n_ref  [ajint] number of reference sequences
** @attr n_smpl [ajint] number of samples
** @attr l_nm   [ajint] length of concatenated sequence names; 0 padded
** @attr l_smpl [ajint] length of concatenated sample names; 0 padded
** @attr l_txt  [ajint] length of header text (lines started with ##)
** @attr Padding [ajuint] Padding to alignment boundary
** @attr name   [char*] concatenated sequence names
** @attr sname  [char*] concatenated sample names
** @attr txt    [char*] header text
** @attr ns    [const char**] array of sequence names (pointer to ->name)
** @attr sns   [const char**] array of sample names   (pointer to ->sname)
**
******************************************************************************/

typedef struct AjSVarBcfHeader
{
    ajint n_ref;
    ajint n_smpl;
    ajint l_nm;
    ajint l_smpl;
    ajint l_txt;
    ajuint Padding;
    char *name;
    char *sname;
    char *txt;
    const char **ns;
    const char **sns;
} AjOVarBcfHeader;

#define AjPVarBcfHeader AjOVarBcfHeader*




int      ajVarbcfRead(AjPVarBcfFile fp, ajint nsmpl, AjPVarBcf b);
int      ajVarbcfSync(AjPVarBcf b);
int      ajVarbcfWrite(AjPVarBcfFile fp, ajint nsmpl, const AjPVarBcf b);

AjPVarBcfHeader ajVarbcfHdrRead(AjPVarBcfFile fp);
int      ajVarbcfHdrSync(AjPVarBcfHeader b);
int      ajVarbcfHdrWrite(AjPVarBcfFile fp, const AjPVarBcfHeader h);

void     ajVarbcfHdrDel(AjPVarBcfHeader h);
int      ajVarbcfDel(AjPVarBcf b);

void     ajVarbcfFmtCore(const AjPVarBcfHeader h, AjPVarBcf b, AjPStr* s);




static inline ajuint bcf_str2int(const char *str, int l)
{
    int i;
    ajuint x = 0;

    for (i = 0; i < l && i < 4; ++i)
    {
	if (str[i] == 0)
	    return x;

	x = x<<8 | str[i];
    }

    return x;
}


#endif /* !AJVARBCF_H */
