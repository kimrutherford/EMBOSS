/* @source ajseqbam ***********************************************************
**
** AJAX BAM format sequence processing functions
**
** These functions control all aspects of AJAX BAM file processing
**
** @author Copyright (C) 2010 Peter Rice ported from samtools
** @version $Revision: 1.36 $
** @modified 2010-2011 Peter Rice
** @modified $Date: 2012/06/27 08:07:40 $ by $Author: rice $
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

#include "ajseqbam.h"
#include "ajassert.h"
#include "ajlist.h"
#include "ajtable.h"
#include "ajutil.h"
#include "ajsys.h"


#include <stdio.h>
#include <ctype.h>
#include <errno.h>
#include <fcntl.h>


#ifndef WIN32
#include <sys/file.h>
#else /* WIN32 */
#include <io.h>
#define open _open
#define fileno _fileno
#endif /* !WIN32 */



/* Don't define this as a const static int if used in array def */
#define AJSEQBAM_BLOCK_HEADER_LENGTH 18


const char *cigarcode = "MIDNSHP";
const char *bam_nt16_rev_table = "=ACMGRSVTWYHKDBN";

static AjBool bamBigendian   = ajFalse;
static AjBool bamInitialized = ajFalse;

const char *o_hd_tags[] = {"SO","GO",NULL};
const char *r_hd_tags[] = {"VN",NULL};

const char *o_sq_tags[] = {"AS","M5","UR","SP",NULL};
const char *r_sq_tags[] = {"SN","LN",NULL};
const char *u_sq_tags[] = {"SN",NULL};

const char *o_rg_tags[] = {"LB","DS","PU","PI","CN","DT","PL",NULL};
const char *r_rg_tags[] = {"ID",NULL};
const char *u_rg_tags[] = {"ID",NULL};

const char *o_pg_tags[] = {"VN","CL",NULL};
const char *r_pg_tags[] = {"ID",NULL};

const char *types[]          = {"HD","SQ","RG","PG","CO",NULL};
const char **optional_tags[] = {o_hd_tags,o_sq_tags,o_rg_tags,o_pg_tags,NULL,
                                NULL};
const char **required_tags[] = {r_hd_tags,r_sq_tags,r_rg_tags,r_pg_tags,NULL,
                                NULL};
const char **unique_tags[]   = {NULL,     u_sq_tags,u_rg_tags,NULL,NULL,NULL};


unsigned char bam_nt16_table[256] =
{
	15,15,15,15, 15,15,15,15, 15,15,15,15, 15,15,15,15,
	15,15,15,15, 15,15,15,15, 15,15,15,15, 15,15,15,15,
	15,15,15,15, 15,15,15,15, 15,15,15,15, 15,15,15,15,
	 1, 2, 4, 8, 15,15,15,15, 15,15,15,15, 15, 0 /*=*/,15,15,
	15, 1,14, 2, 13,15,15, 4, 11,15,15,12, 15, 3,15,15,
	15,15, 5, 6,  8,15, 7, 9, 15,10,15,15, 15,15,15,15,
	15, 1,14, 2, 13,15,15, 4, 11,15,15,12, 15, 3,15,15,
	15,15, 5, 6,  8,15, 7, 9, 15,10,15,15, 15,15,15,15,
	15,15,15,15, 15,15,15,15, 15,15,15,15, 15,15,15,15,
	15,15,15,15, 15,15,15,15, 15,15,15,15, 15,15,15,15,
	15,15,15,15, 15,15,15,15, 15,15,15,15, 15,15,15,15,
	15,15,15,15, 15,15,15,15, 15,15,15,15, 15,15,15,15,
	15,15,15,15, 15,15,15,15, 15,15,15,15, 15,15,15,15,
	15,15,15,15, 15,15,15,15, 15,15,15,15, 15,15,15,15,
	15,15,15,15, 15,15,15,15, 15,15,15,15, 15,15,15,15,
	15,15,15,15, 15,15,15,15, 15,15,15,15, 15,15,15,15
};




/* @datastatic BamPHeaderTag **************************************************
**
** BAM header tag
**
** @attr value [char*] Value
** @attr key [char[2]] Key
** @attr Padding [char[6]] Padding for alignment
******************************************************************************/

typedef struct BamSHeaderTag
{
    char *value;
    char key[2];
    char Padding[6];
} BamOHeaderTag;

#define BamPHeaderTag BamOHeaderTag*




/* @datastatic BamPHeaderLine *************************************************
**
** BAM header line
**
** @attr tags [AjPList] List of tags
** @attr type [char[2]] Type
** @attr Padding [char[6]] Padding for alignment
******************************************************************************/

typedef struct BamSHeaderLine
{
    AjPList tags;
    char type[2];
    char Padding[6];
} BamOHeaderLine;

#define BamPHeaderLine BamOHeaderLine*

const char *bam_flag2char_table = "pPuUrR12sfd\0\0\0\0\0";

static AjPSeqBamBgzf  bamBgzfNew(void);
static void           bamCacheFree(AjPSeqBamBgzf fp);
static int            bamCacheLoadBlock(AjPSeqBamBgzf fp, ajlong block_address);
static int            bamDeflateBlock(AjPSeqBamBgzf fp, int block_length);
static void           bamDestroyHeaderHash(AjPSeqBamHeader header);
static unsigned char *bamGetAux(const AjPSeqBam b, const char tag[2]);
static int            bamHeaderCheck(const char *header);
static void           bamHeaderFree(AjPList *header);
static BamPHeaderTag  bamHeaderLineHasTag(const BamPHeaderLine hline,
                                         const char *key);
static BamPHeaderLine bamHeaderLineParse(const char *headerLine);
static AjPTable	      bamHeaderLineToTablemap(const BamPHeaderLine hline);
static void           bamHeaderLineFree(BamPHeaderLine *hline);
static int            bamHeaderLineValidate(BamPHeaderLine hline);
static AjPList        bamHeaderParse2(const char *headerText);
static AjPTable       bamHeaderTotable(const AjPList _dict, const char type[2],
                                      const char key_tag[2],
                                      const char value_tag[2]);
static int            bamInflateBlock(AjPSeqBamBgzf fp, int block_length);
static int            bamReadBlock(AjPSeqBamBgzf fp);
static void           bamSwapEndianData(const AjPSeqBamCore c, int data_len,
                                        unsigned char *data);
static BamPHeaderTag  bamTagNew(const char *name, const char *value_from,
                                const char *value_to);

static const char*    bamNextline(char **lineptr, size_t *n, const char *text);
static int            bamTagExists(const char *tag, const char **tags);




/* @func ajSeqBamCalend *****************************************************
**
** Calculate the rightmost coordinate of an alignment on the reference genome
**
** @param [r] c [const AjPSeqBamCore] pointer to the AjPSeqBamCore structure
** @param [r] cigar [const ajuint*] the corresponding CIGAR array
** @return [ajuint] the rightmost coordinate, 0-based
**
** @release 6.5.0
******************************************************************************/

ajuint ajSeqBamCalend(const AjPSeqBamCore c, const ajuint *cigar)
{
	ajuint k;
	ajuint end;
	ajint op;
	
	end = c->pos;
	
	for (k = 0; k < c->n_cigar; ++k)
	{
		op = cigar[k] & BAM_CIGAR_MASK;
		if (op == BAM_CMATCH || op == BAM_CDIFF || op == BAM_CEQUAL
			|| op == BAM_CDEL || op == BAM_CREF_SKIP)
			end += cigar[k] >> BAM_CIGAR_SHIFT;
	}
	
	return end;
}




/* @func ajSeqBamReg2bin ******************************************************
**
** Calculate the minimum bin that contains a region [beg,end).
**
** @param [r] beg [ajuint] start of the region, 0-based
** @param [r] end [ajuint] end of the region, 0-based
** @return [int] bin
**
******************************************************************************/

int ajSeqBamReg2bin(ajuint beg, ajuint end)
{
    --end;

    if(beg>>14 == end>>14)
        return 4681 + (beg>>14);

    if(beg>>17 == end>>17)
        return 585 + (beg>>17);

    if(beg>>20 == end>>20)
        return 73 + (beg>>20);

    if(beg>>23 == end>>23)
        return 9 + (beg>>23);

    if(beg>>26 == end>>26)
        return 1 + (beg>>26);

    return 0;
}




/* #funcstatic bam_copy1 *****************************************************
**
** Copy an alignment
**
** #param [u] bdst [AjPSeqBam] destination alignment struct
** #param [r] bsrc [const AjPSeqBam] source alignment struct
** #return [AjPSeqBam] pointer to the destination alignment struct
**
******************************************************************************/
/*
//static inline AjPSeqBam bam_copy1(AjPSeqBam bdst, const AjPSeqBam bsrc)
//{
//    unsigned char *data = NULL;
//    int m_data;
//
//    /# backup data and m_data #/
//    data = bdst->data;
//    m_data = bdst->m_data;
//    
//    
//    /# double the capacity #/
//    if (m_data < bsrc->m_data)
//    {
//        m_data = bsrc->m_data; kroundup32(m_data);
//        data = (unsigned char*)realloc(data, m_data);
//    }
//
//    /# copy var-len data #/
//    memcpy(data, bsrc->data, bsrc->data_len);
//
//    /# copy the rest #/
//    *bdst = *bsrc;
//
//    /# restore the backup #/
//    bdst->m_data = m_data;
//    bdst->data = data;
//
//    return bdst;
//}
*/




/* @funcstatic bam_aux_type2size **********************************************
**
** Returns size of the array elements in optional fields of alignment records
**
** @param [r] x [int] type of the array elements
** @return [int] size of the array elements
**
******************************************************************************/

static inline int bam_aux_type2size(int x)
{
	if (x == 'C' || x == 'c' || x == 'A')
	    return 1;
	else if (x == 'S' || x == 's')
	    return 2;
	else if (x == 'I' || x == 'i' || x == 'f')
	    return 4;
	else
	    return 0;
}




/* @func ajSeqBamHeaderNew ****************************************************
**
** Create an empty BAM header object
**
** @return [AjPSeqBamHeader] BAM header object
**
**
** @release 6.3.0
******************************************************************************/

AjPSeqBamHeader ajSeqBamHeaderNew(void)
{
    AjPSeqBamHeader header=NULL;

    AJNEW0(header);

    return header;
}




/* @func ajSeqBamHeaderNewTextC ***********************************************
**
** Create an empty BAM header object with given header text
**
** @param [r]  txt [const char*] BAM/SAM header text string
** @return [AjPSeqBamHeader] BAM header object
**
**
** @release 6.3.0
******************************************************************************/

AjPSeqBamHeader ajSeqBamHeaderNewTextC(const char* txt)
{
    AjPSeqBamHeader ret;

    AJNEW0(ret);

    ret->l_text = strlen(txt);
    ret->text = ajCharNewResC(txt, ret->l_text + 1);

    return ret;
}




/* @func ajSeqBamHeaderSetTextC ***********************************************
**
** Set BAM header text
**
** @param [u]  header [AjPSeqBamHeader] BAM header
** @param [r]  txt [const char*] header text
** @return [void]
**
**
** @release 6.5.0
******************************************************************************/

void ajSeqBamHeaderSetTextC(AjPSeqBamHeader header, const char* txt)
{

    if(header->text)
	AJFREE(header->text);

    header->l_text = strlen(txt);
    header->text = ajCharNewResC(txt, header->l_text + 1);

    return;
}




/* @func ajSeqBamHeaderNewN ***************************************************
**
** Create an empty BAM header object with n reference sequences
**
** @param [r] n [ajint] Number of target/reference sequences
** @return [AjPSeqBamHeader] BAM header object
**
**
** @release 6.5.0
******************************************************************************/

AjPSeqBamHeader ajSeqBamHeaderNewN(ajint n)
{
    AjPSeqBamHeader ret;

    AJNEW0(ret);

    ret->n_targets = n;
    AJCNEW0(ret->target_name, n);
    AJCNEW0(ret->target_len, n);

    return ret;
}




/* @func ajSeqBamDel **********************************************************
**
** Deletes a BAM alignment object
**
** @param [d] Pbam [AjPSeqBam*] BAM alignment object
** @return [void]
**
**
** @release 6.5.0
******************************************************************************/

void ajSeqBamDel(AjPSeqBam *Pbam)
{
    AjPSeqBam bam = NULL;

    bam = *Pbam;

    if(bam == 0)
        return;

    AJFREE(bam->data);
    AJFREE(*Pbam);

    return;
}




/* @func ajSeqBamHeaderDel ****************************************************
**
** Destructor for a BAM header object
**
** @param [d] Pheader [AjPSeqBamHeader*] BAM header object
** @return [void]
**
**
** @release 6.3.0
******************************************************************************/

void ajSeqBamHeaderDel(AjPSeqBamHeader *Pheader)
{
    ajint i;
    AjPSeqBamHeader header = NULL;

    header = *Pheader;
    
    if(header == 0)
        return;

    if(header->target_name)
    {
        for(i = 0; i < header->n_targets; ++i)
            AJFREE(header->target_name[i]);

        AJFREE(header->target_name);
        AJFREE(header->target_len);
    }

    AJFREE(header->text);

    if(header->dict)
        bamHeaderFree(&header->dict);

    if(header->rg2lib)
        ajTableFree(&header->rg2lib);

    bamDestroyHeaderHash(header);

    AJFREE(*Pheader);

    return;
}




/* @func ajSeqBamHeaderWrite **************************************************
**
** Write a header structure to BAM
**
** @param [u] fp [AjPSeqBamBgzf] BAM file handler
** @param [r] header [const AjPSeqBamHeader] Header structure
** @return [int] Always 0 currently
**
**
** @release 6.3.0
******************************************************************************/

int ajSeqBamHeaderWrite(AjPSeqBamBgzf fp, const AjPSeqBamHeader header)
{
    char buf[4];
    ajint i;
    ajint name_len;
    ajint x;
    char *p = NULL;

    if(!bamInitialized)
    {
        bamInitialized = ajTrue;
	bamBigendian = ajUtilGetBigendian();
    }

    /* write "BAM1" */
    strncpy(buf, "BAM\001", 4);
    ajSeqBamBgzfWrite(fp, buf, 4);

    /* write plain text and the number of reference sequences */

    if(bamBigendian)
    {
        x = header->l_text;
        ajByteRevInt(&x);
        ajSeqBamBgzfWrite(fp, &x, 4);

        if (header->l_text)
            ajSeqBamBgzfWrite(fp, header->text, header->l_text);

        x = header->n_targets;
        ajByteRevInt(&x);
        ajSeqBamBgzfWrite(fp, &x, 4);
    }
    else
    {
        ajSeqBamBgzfWrite(fp, &header->l_text, 4);

        if(header->l_text)
            ajSeqBamBgzfWrite(fp, header->text, header->l_text);

        ajSeqBamBgzfWrite(fp, &header->n_targets, 4);
    }

    /* write sequence names and lengths */

    for(i = 0; i != header->n_targets; ++i)
    {
        p = header->target_name[i];
        name_len = strlen(p) + 1;

        if(bamBigendian)
        {
            x = name_len;
            ajByteRevInt(&x);
            ajSeqBamBgzfWrite(fp, &x, 4);
        }
        else
            ajSeqBamBgzfWrite(fp, &name_len, 4);

        ajSeqBamBgzfWrite(fp, p, name_len);

        if(bamBigendian)
        {
            x = header->target_len[i];
            ajByteRevInt(&x);
            ajSeqBamBgzfWrite(fp, &x, 4);
        }
        else
            ajSeqBamBgzfWrite(fp, &header->target_len[i], 4);
    }

    return 0;
}




/* @funcstatic bamSwapEndianData **********************************************
**
** Swap bigendian data in a BAM core data buffer
**
** @param [r] c [const AjPSeqBamCore] BAM core data
** @param [r] data_len [int] Data length
** @param [u] data [unsigned char*] Data buffer
** @return [void]
**
**
** @release 6.3.0
******************************************************************************/

static void bamSwapEndianData(const AjPSeqBamCore c, int data_len,
                              unsigned char *data)
{
    unsigned char *s = NULL;
    ajuint i;
    ajuint *cigar = NULL;
    unsigned char type;

    cigar = (ajuint*)(data + c->l_qname);

    s = data + c->n_cigar*4 + c->l_qname + c->l_qseq + (c->l_qseq + 1)/2;

    for(i = 0; i < c->n_cigar; ++i)
        ajByteRevUint(&cigar[i]);

    while(s < data + data_len)
    {
        s += 2; /* skip key */
        type = toupper(*s); ++s; /* skip type */

        if(type == 'C' || type == 'A')
            ++s;
        else if(type == 'S')
        {
            ajByteRevShort((ajshort*)s);
            s += 2;
        }
        else if(type == 'I' || type == 'F')
        {
            ajByteRevUint((ajuint*)s);
            s += 4;
        }
        else if(type == 'D')
        {
            ajByteRevLong((ajlong*)s);
            s += 8;
        }
        else if(type == 'Z' || type == 'H')
        {
            while(*s)
                ++s;

            ++s;
        }
        else if (type == 'B')
        {
            ajuint n;
            ajint Bsize = bam_aux_type2size(*s);
            memcpy(&n, s + 1, 4);

            if (1 == Bsize) {
            } else if (2 == Bsize) {
        	for (i = 0; i < n; i += 2)
        	    ajByteRevShort((ajshort*)s + 5 + i);
            } else if (4 == Bsize) {
        	for (i = 0; i < n; i += 4)
        	    ajByteRevUint((ajuint*)s + 5 + i);
            }
            ajByteRevUint((ajuint*)s+1);
        }

    }

    return;
}




/* @func ajSeqBamRead *********************************************************
**
** Read a BAM alignment record of a read(query sequence)
**
** The file position indicator must be placed right before an
** alignment. Upon success, this function will set the position
** indicator to the start of the next alignment. This function is not
** affected by the machine endianness.
**
** @param [u] fp [AjPSeqBamBgzf] BAM file handler
** @param [u] b [AjPSeqBam] read alignment; all members are updated.
** @return [int] Number of bytes read from the file
**
** @release 6.3.0
** @@
******************************************************************************/

int ajSeqBamRead(AjPSeqBamBgzf fp, AjPSeqBam b)
{
    AjPSeqBamCore c = &b->core;
    ajint block_len;
    ajint ret;
    ajint i;
    ajuint x[8];
    ajint oldsize;

    if(!bamInitialized)
    {
        bamInitialized = ajTrue;
	bamBigendian = ajUtilGetBigendian();
    }

    assert(BAM_CORE_SIZE == 32);

    if((ret = ajSeqBamBgzfRead(fp, &block_len, 4)) != 4)
    {
        if(ret == 0)
            return -1; /* normal end-of-file */

        return -2; /* truncated */
    }

    if(block_len < (ajint) BAM_CORE_SIZE)
        ajErr("block_len: %d core_size: %d", block_len, BAM_CORE_SIZE);

    if(ajSeqBamBgzfRead(fp, x, BAM_CORE_SIZE) != BAM_CORE_SIZE)
        return -3;

    if(bamBigendian)
    {
        ajByteRevInt(&block_len);

        for(i = 0; i < 8; ++i)
            ajByteRevUint(x + i);
    }

    c->tid = x[0];
    c->pos = x[1];
    c->bin = x[2]>>16;
    c->qual = x[2]>>8&0xff;
    c->l_qname = x[2]&0xff;
    c->flag = x[3]>>16;
    c->n_cigar = x[3]&0xffff;
    c->l_qseq = x[4];
    c->mtid = x[5];
    c->mpos = x[6];
    c->isize = x[7];
    b->data_len = block_len - BAM_CORE_SIZE;

    if(b->m_data < b->data_len)
    {
	oldsize = b->m_data;
        b->m_data = b->data_len;
        kroundup32(b->m_data);
        AJRESIZE0(b->data, oldsize, b->m_data);
    }

    if(ajSeqBamBgzfRead(fp, b->data, b->data_len) != b->data_len)
    {
        ajErr("ajSeqBamBgzfRead len: %d failed", b->data_len);
        return -4;
    }

    b->l_aux = b->data_len - c->n_cigar * 4 - c->l_qname -
        c->l_qseq - (c->l_qseq+1)/2;

    if(bamBigendian)
        bamSwapEndianData(c, b->data_len, b->data);

    return 4 + block_len;
}




/* @func ajSeqBamWrite ********************************************************
**
** Writes BAM alignment data to a BGZ file
**
** @param [u] fp [AjPSeqBamBgzf]BAM file handler
** @param [r] b [const AjPSeqBam] alignment to write
** @return [int] number of bytes written to the file
**
**
** @release 6.3.0
******************************************************************************/

int ajSeqBamWrite(AjPSeqBamBgzf fp, const AjPSeqBam b)
{
    const AjPSeqBamCore c;
    int data_len;
    unsigned char *data = NULL;

    ajuint x[8];
    ajuint block_len;
    ajuint y;
    int i;

    c         = &b->core;
    data_len  = b->data_len;
    data      = b->data;
    block_len = data_len + BAM_CORE_SIZE;

    if(!bamInitialized)
    {
        bamInitialized = ajTrue;
	bamBigendian = ajUtilGetBigendian();
    }

    assert(BAM_CORE_SIZE == 32);

    x[0] = c->tid;
    x[1] = c->pos;
    x[2] = (ajuint)c->bin<<16 | c->qual<<8 | c->l_qname;
    x[3] = (ajuint)c->flag<<16 | c->n_cigar;
    x[4] = c->l_qseq;
    x[5] = c->mtid;
    x[6] = c->mpos;
    x[7] = c->isize;

    if(bamBigendian)
    {
        for(i = 0; i < 8; ++i)
            ajByteRevUint(x + i);

        y = block_len;
        ajByteRevUint(&y);
        ajSeqBamBgzfWrite(fp, &y, 4);
        bamSwapEndianData(c, data_len, data);
    }
    else
        ajSeqBamBgzfWrite(fp, &block_len, 4);

    ajSeqBamBgzfWrite(fp, x, BAM_CORE_SIZE);
    ajSeqBamBgzfWrite(fp, data, data_len);

    if(bamBigendian)
        bamSwapEndianData(c, data_len, data);

    return 4 + block_len;
}




/* @func ajSeqBamGetLibrary ***************************************************
**
** Return the name of the library from the BAM file header RG tag
**
** @param [u] h [AjPSeqBamHeader] BAM header
** @param [r] b [const AjPSeqBam] BAM record
** @return [const char*] Library name
**
**
** @release 6.3.0
******************************************************************************/

const char* ajSeqBamGetLibrary(AjPSeqBamHeader h, const AjPSeqBam b)
{
    const unsigned char *rg;
/*
** FIXME: this should also check the LB tag associated with each alignment
*/
    if(h->dict == 0)
        h->dict = bamHeaderParse2(h->text);

    if(h->rg2lib == 0)
        h->rg2lib = bamHeaderTotable(h->dict, "RG", "ID", "LB");

    rg = bamGetAux(b, "RG");

    return (rg == 0) ? 0 : ajTableFetchC(h->rg2lib, (const char*)(rg + 1));
}




/* @datastatic BamPCache ******************************************************
**
** BAM block cache
**
** @alias BamSCache
** @alias BamOCache
**
** @attr block [unsigned char*] Block data
** @attr end_offset [ajlong] End offset
** @attr size [int]Block size
** @attr padding [int] To alignment boundary
**
******************************************************************************/

typedef struct BamSCache
{
    unsigned char *block;
    ajlong end_offset;
    int size;
    int padding;
} BamOCache;

#define BamPCache BamOCache*


/* @conststatic BAM variables *************************************************
**
** Variables defined for BAM processing
**
** @value AJSEQBAM_DEFAULT_BLOCK_SIZE [int] Default block size of 64k
** @value AJSEQBAM_MAX_BLOCK_SIZE [int] Maximum block size also 64k
** @value AJSEQBAM_BLOCK_FOOTER_LENGTH [int] Block footer length
** @value AJSEQBAM_GZIP_ID1 [int] Gzip magic id 1
** @value AJSEQBAM_GZIP_ID2 [int] Gzip magic id 2
** @value AJSEQBAM_CM_DEFLATE [int] CM deflate
** @value AJSEQBAM_FLG_FEXTRA [int] Fextra flag
** @value AJSEQBAM_OS_UNKNOWN [int] Unkown OS value
** @value AJSEQBAM_BGZF_ID1 [int] Bgfz magic id 1 'B'
** @value AJSEQBAM_BGZF_ID2 [int] Bgfz magic id 2 'C'
** @value AJSEQBAM_BGZF_LEN [int] Bgzf length
** @value AJSEQBAM_BGZF_XLEN [int] Bgzf xlength (length plus 4)
** @value AJSEQBAM_GZIP_WINDOW_BITS [int] Gzip window bits (-15 - no zlib header
** @value AJSEQBAM_Z_DEFAULT_MEM_LEVEL [int] Z default memory level
******************************************************************************/

static const int AJSEQBAM_DEFAULT_BLOCK_SIZE = 64 * 1024;
static const int AJSEQBAM_MAX_BLOCK_SIZE = 64 * 1024;

static const int AJSEQBAM_BLOCK_FOOTER_LENGTH = 8;

static const int AJSEQBAM_GZIP_ID1 = 31;
static const int AJSEQBAM_GZIP_ID2 = 139;
static const int AJSEQBAM_CM_DEFLATE = 8;
static const int AJSEQBAM_FLG_FEXTRA = 4;
static const int AJSEQBAM_OS_UNKNOWN = 255;
static const int AJSEQBAM_BGZF_ID1 = 66;
static const int AJSEQBAM_BGZF_ID2 = 67;
static const int AJSEQBAM_BGZF_LEN = 2;
static const int AJSEQBAM_BGZF_XLEN = 6;

static const int AJSEQBAM_GZIP_WINDOW_BITS = -15;
static const int AJSEQBAM_Z_DEFAULT_MEM_LEVEL = 8;





static inline void packInt16(unsigned char* buffer, ajushort value);

static inline void packInt32(unsigned char* buffer, ajuint value);
static inline int  unpackInt16(const unsigned char* buffer);
static inline int  BAMBGZFMIN(int x, int y);




/* @funcstatic packInt16 *****************************************************
**
** Pack a 2-byte integer
**
** @param [u] buffer [unsigned char*] Buffer of at least 2 bytes
** @param [r] value [ajushort] Unsigned integer value
** @return [void]
******************************************************************************/

static inline void packInt16(unsigned char* buffer, ajushort value)
{
    buffer[0] = (unsigned char) value;
    buffer[1] = value >> 8;
}




/* @funcstatic unpackInt16 ****************************************************
**
** Unpack a 2-byte integer
**
** @param [r] buffer [const unsigned char*] Buffer of at least 2 bytes
** @return [int] Integer value from first 2 bytes of buffer
**
******************************************************************************/

static inline int unpackInt16(const unsigned char* buffer)
{
    return (buffer[0] | (buffer[1] << 8));
}





/* @funcstatic packInt32 *****************************************************
**
** Pack a 4-byte integer
**
** @param [u] buffer [unsigned char*] Buffer of at least 4 bytes
** @param [r] value [ajuint] Unsigned integer value
** @return [void]
******************************************************************************/

static inline void packInt32(unsigned char* buffer, ajuint value)
{
    buffer[0] = value;
    buffer[1] = value >> 8;
    buffer[2] = value >> 16;
    buffer[3] = value >> 24;
}





/* @funcstatic BAMBGZFMIN ****************************************************
**
** Return minimum of two integers
**
** @param [r] x [int] First integer
** @param [r] y [int] Second integer
** @return [int] Lowest integer value
******************************************************************************/

static inline int BAMBGZFMIN(int x, int y)
{
    return (x < y) ? x : y;
}





/* @funcstatic bamReportError *************************************************
**
** Save error message in the BGZ file error attribute
**
** @param [u] fp [AjPSeqBamBgzf] BGZ file
** @param [r] message [const char*] message text
** @return [void]
**
**
** @release 6.3.0
******************************************************************************/

static void bamReportError(AjPSeqBamBgzf fp, const char* message)
{
    ajWarn("++bamReportError '%s'", message);
    fp->error = message;

    return;
}




/* @funcstatic bamBgzfNew *****************************************************
**
** Initialize a BGZ file object
**
** @return [AjPSeqBamBgzf] BGZ file object
**
**
** @release 6.3.0
******************************************************************************/

static AjPSeqBamBgzf bamBgzfNew(void)
{
    AjPSeqBamBgzf fp;

    fp = calloc(1, sizeof(AjOSeqBamBgzf));
    fp->uncompressed_block_size = AJSEQBAM_MAX_BLOCK_SIZE;
    fp->uncompressed_block = malloc(AJSEQBAM_MAX_BLOCK_SIZE);
    fp->compressed_block_size = AJSEQBAM_MAX_BLOCK_SIZE;
    fp->compressed_block = malloc(AJSEQBAM_MAX_BLOCK_SIZE);
    fp->cache_size = 0;
    fp->cache = NULL;

    return fp;
}




/* @funcstatic bamBgzfOpenfdRead **********************************************
**
** Open a BGZ file for reading
**
** @param [r] fd [int] File descriptor of opened file
** @return [AjPSeqBamBgzf] BGZ file object
**
**
** @release 6.3.0
******************************************************************************/

static AjPSeqBamBgzf bamBgzfOpenfdRead(int fd)
{
    FILE* file;
    AjPSeqBamBgzf fp;

    file = ajSysFuncFdopen(fd, "r");
    
    if(file == 0)
        return 0;

    fp = bamBgzfNew();
    fp->file_descriptor = fd;
    fp->open_mode = 'r';
    fp->file = file;
    fp->cache = ajTableNew(512);

    fseek(fp->file, 0L, SEEK_SET);

    return fp;
}




/* @funcstatic bamBgzfOpenfdWrite *********************************************
**
** Open a BGZ file for writing
**
** @param [r] fd [int] File descriptor of opened file
** @param [r] is_uncompressed [char] If non-zero, file is uncompressed
** @return [AjPSeqBamBgzf] BGZ file object
**
**
** @release 6.3.0
******************************************************************************/

static AjPSeqBamBgzf bamBgzfOpenfdWrite(int fd, char is_uncompressed)
{
    FILE* file;
    AjPSeqBamBgzf fp;

    file = ajSysFuncFdopen(fd, "w");
    
    if(file == 0)
        return 0;

    fp = malloc(sizeof(AjOSeqBamBgzf));
    fp->file_descriptor = fd;
    fp->open_mode = 'w';
    fp->owned_file = 0;
    fp->is_uncompressed = is_uncompressed;
    fp->file = file;
    fp->uncompressed_block_size = AJSEQBAM_DEFAULT_BLOCK_SIZE;
    fp->uncompressed_block = NULL;
    fp->compressed_block_size = AJSEQBAM_MAX_BLOCK_SIZE;
    fp->compressed_block = malloc(AJSEQBAM_MAX_BLOCK_SIZE);
    fp->block_address = 0;
    fp->block_offset = 0;
    fp->block_length = 0;
    fp->error = NULL;

    return fp;
}




/* @func ajSeqBamBgzfOpenC ****************************************************
**
** Open a named BGZ file for reading or writing
**
** @param [r] path [const char*] File path
** @param [r] mode [const char*] File open mode 'r' 'R' 'w' or 'W'
** @return [AjPSeqBamBgzf] BGZ file object
**
**
** @release 6.3.0
******************************************************************************/

AjPSeqBamBgzf ajSeqBamBgzfOpenC(const char* path, const char*  mode)
{
    AjPSeqBamBgzf fp = NULL;
    int fd;
    int oflag;
    
    /* The reading mode is preferred. */
    if(mode[0] == 'r' || mode[0] == 'R')
    {
        oflag = O_RDONLY;
#ifdef WIN32
        oflag |= O_BINARY;
#endif /* WIN32 */
        fd = open(path, oflag);

        if(fd == -1)
            return 0;

        fp = bamBgzfOpenfdRead(fd);
    }
    else if(mode[0] == 'w' || mode[0] == 'W')
    {
        oflag = O_WRONLY | O_CREAT | O_TRUNC;
#ifdef WIN32
        oflag |= O_BINARY;
#endif /* WIN32 */
        fd = open(path, oflag, 0666);

        if(fd == -1)
            return 0;

        fp = bamBgzfOpenfdWrite(fd, strstr(mode, "u")? 1 : 0);
    }

    if(fp != NULL)
        fp->owned_file = 1;

    return fp;
}




/* @func ajSeqBamBgzfOpenfd ***************************************************
**
** Open a BGZip BAM file by file descriptor
**
** @param [r] fd [int] open file descriptor
** @param [r] mode [const char*] File open mode 'r' 'R' 'w' or 'W'
** @return [AjPSeqBamBgzf] BGZip BAM file
**
**
** @release 6.3.0
******************************************************************************/

AjPSeqBamBgzf ajSeqBamBgzfOpenfd(int fd, const char * mode)
{
    if(fd == -1)
        return 0;

    if (mode[0] == 'r' || mode[0] == 'R')
        return bamBgzfOpenfdRead(fd);
    else if (mode[0] == 'w' || mode[0] == 'W')
        return bamBgzfOpenfdWrite(fd, strstr(mode, "u")? 1 : 0);

    return NULL;
}




/* @func ajSeqBamBgzfSetInfile ************************************************
**
** Sets data structure to read from an open BGZip BAM file 
**
** @param [u] gzfile [AjPSeqBamBgzf] BGZip BAM file
** @param [u] inf [AjPFile] Open input file object 
** @return [AjBool] True on success
** @release 6.5.0
******************************************************************************/

AjBool ajSeqBamBgzfSetInfile(AjPSeqBamBgzf gzfile, AjPFile inf)
{
    if(!inf)
        return ajFalse;

    gzfile->file_descriptor = fileno(ajFileGetFileptr(inf));
    gzfile->open_mode = 'r';
    gzfile->owned_file = 0;
    gzfile->is_uncompressed = '\0';
    gzfile->file = ajFileGetFileptr(inf);
    gzfile->uncompressed_block_size = AJSEQBAM_DEFAULT_BLOCK_SIZE;
    gzfile->uncompressed_block = NULL;
    gzfile->compressed_block_size = AJSEQBAM_MAX_BLOCK_SIZE;
    gzfile->compressed_block = malloc(AJSEQBAM_MAX_BLOCK_SIZE);
    gzfile->block_address = 0;
    gzfile->block_offset = 0;
    gzfile->block_length = 0;
    gzfile->error = NULL;

    return ajTrue;
}




/* @func ajSeqBamBgzfSetOutfile ***********************************************
**
** Sets data structure to write to an open BGZip BAM file 
**
** @param [u] gzfile [AjPSeqBamBgzf] BGZip BAM file
** @param [u] outf [AjPFile] Open output file object 
** @return [AjBool] True on success
** @release 6.5.0
******************************************************************************/

AjBool ajSeqBamBgzfSetOutfile(AjPSeqBamBgzf gzfile, AjPFile outf)
{
    if(!outf)
        return ajFalse;

    gzfile->file_descriptor = fileno(ajFileGetFileptr(outf));
    gzfile->open_mode = 'w';
    gzfile->owned_file = 0;
    gzfile->is_uncompressed = '\0';
    gzfile->file = ajFileGetFileptr(outf);
    gzfile->uncompressed_block_size = AJSEQBAM_DEFAULT_BLOCK_SIZE;
    gzfile->uncompressed_block = NULL;
    gzfile->compressed_block_size = AJSEQBAM_MAX_BLOCK_SIZE;
    gzfile->compressed_block = malloc(AJSEQBAM_MAX_BLOCK_SIZE);
    gzfile->block_address = 0;
    gzfile->block_offset = 0;
    gzfile->block_length = 0;
    gzfile->error = NULL;

    return ajTrue;
}




/* @func ajSeqBamBgzfNew *****************************************************
**
** Constructs a BGZip BAM file by file
**
** @param [u] file [FILE*] open file
** @param [r] mode [const char*] File open mode 'r' 'R' 'w' or 'W'
** @return [AjPSeqBamBgzf] BGZip BAM file
**
**
** @release 6.5.0
******************************************************************************/

AjPSeqBamBgzf ajSeqBamBgzfNew(FILE* file, const char*  mode)
{
    AjPSeqBamBgzf fp;

    if(file == 0)
        return 0;

    fp = bamBgzfNew();
    fp->file_descriptor = fileno(file);

    if(mode[0] == 'r' || mode[0] == 'R')
    {
	fp->open_mode = 'r';
	fp->cache = ajTableNew(512);
    }
    else
	fp->open_mode = 'w';

    fp->file = file;
    fseek(fp->file, 0L, SEEK_SET);

    return fp;
}




/* @funcstatic bamDeflateBlock ************************************************
**
** Deflate an uncompressed block
**
** @param [u] fp  [AjPSeqBamBgzf] BGZ file
** @param [r] block_length [int] Uncompressed block length
** @return [int] Length of compressed block
**
**
** @release 6.3.0
******************************************************************************/

static int bamDeflateBlock(AjPSeqBamBgzf fp, int block_length)
{
    /*
    ** Deflate the block in fp->uncompressed_block into fp->compressed_block.
    ** Also adds an extra field that stores the compressed block length.
    */

    char *buffer = NULL;
    int buffer_size;
    int input_length;
    int compressed_length;
    int compress_level;
    int status;
    z_stream zs;
    ajuint crc;
    int remaining;

    buffer      = fp->compressed_block;
    buffer_size = fp->compressed_block_size;
    
    /* Init gzip header */
    buffer[0] = AJSEQBAM_GZIP_ID1;
    buffer[1] = AJSEQBAM_GZIP_ID2;
    buffer[2] = AJSEQBAM_CM_DEFLATE;
    buffer[3] = AJSEQBAM_FLG_FEXTRA;
    buffer[4] = 0; /* mtime */
    buffer[5] = 0;
    buffer[6] = 0;
    buffer[7] = 0;
    buffer[8] = 0;
    buffer[9] = AJSEQBAM_OS_UNKNOWN;
    buffer[10] = AJSEQBAM_BGZF_XLEN;
    buffer[11] = 0;
    buffer[12] = AJSEQBAM_BGZF_ID1;
    buffer[13] = AJSEQBAM_BGZF_ID2;
    buffer[14] = AJSEQBAM_BGZF_LEN;
    buffer[15] = 0;
    buffer[16] = 0; /* placeholder for block length */
    buffer[17] = 0;

    /* loop to retry for blocks that do not compress enough */
    input_length = block_length;
    compressed_length = 0;

    while(1)
    {
	compress_level = fp->is_uncompressed ? Z_NO_COMPRESSION :
		Z_DEFAULT_COMPRESSION;
        zs.zalloc = NULL;
        zs.zfree  = NULL;
        zs.next_in   = fp->uncompressed_block;
        zs.avail_in  = input_length;
        zs.next_out  = (void*)&buffer[AJSEQBAM_BLOCK_HEADER_LENGTH];
        zs.avail_out = buffer_size - AJSEQBAM_BLOCK_HEADER_LENGTH -
            AJSEQBAM_BLOCK_FOOTER_LENGTH;

        status = deflateInit2(&zs, compress_level, Z_DEFLATED,
                              AJSEQBAM_GZIP_WINDOW_BITS,
                              AJSEQBAM_Z_DEFAULT_MEM_LEVEL,
                              Z_DEFAULT_STRATEGY);

        if(status != Z_OK)
        {
            bamReportError(fp, "deflate init failed");
            return -1;
        }

        status = deflate(&zs, Z_FINISH);

        if(status != Z_STREAM_END)
        {
            deflateEnd(&zs);

            if(status == Z_OK)
            {
                /*
                ** Not enough space in buffer.
                ** Can happen in the rare case the input doesn't
                ** compress enough.
                ** Reduce the amount of input until it fits.
                */
                input_length -= 1024;

                if(input_length <= 0)
                {
                    /* should never happen */
                    bamReportError(fp, "input reduction failed");
                    return -1;
                }

                continue;
            }

            bamReportError(fp, "deflate failed");
            return -1;
        }

        status = deflateEnd(&zs);

        if(status != Z_OK)
        {
            bamReportError(fp, "deflate end failed");
            return -1;
        }

        compressed_length = zs.total_out;
        compressed_length += AJSEQBAM_BLOCK_HEADER_LENGTH +
            AJSEQBAM_BLOCK_FOOTER_LENGTH;

        if(compressed_length > AJSEQBAM_MAX_BLOCK_SIZE)
        {
            /* should never happen */
            bamReportError(fp, "deflate overflow");
            return -1;
        }

        break;
    }

    packInt16((unsigned char*)&buffer[16], compressed_length-1);
    crc = crc32(0L, NULL, 0L);
    crc = crc32(crc, fp->uncompressed_block, input_length);
    packInt32((unsigned char*)&buffer[compressed_length-8], crc);
    packInt32((unsigned char*)&buffer[compressed_length-4], input_length);

    remaining = block_length - input_length;

    if(remaining > 0)
    {
        if(remaining > input_length)
        {
            /* should never happen (check so we can use memcpy) */
            bamReportError(fp, "remainder too large");
            return -1;
        }

        memcpy((unsigned char*)fp->uncompressed_block,
               (unsigned char*)fp->uncompressed_block + input_length,
               remaining);
    }

    fp->block_offset = remaining;

    return compressed_length;
}




/* @funcstatic bamInflateBlock ************************************************
**
** Uncompress a compressed block
**
** @param [u] fp [AjPSeqBamBgzf] BGZ file object
** @param [r] block_length  [int] Compressed block length
** @return [int] Uncompressed block length
**
**
** @release 6.3.0
******************************************************************************/

static int bamInflateBlock(AjPSeqBamBgzf fp, int block_length)
{
    /* Inflate the block in fp->compressed_block into fp->uncompressed_block */
    int status;
    z_stream zs;

    zs.zalloc = NULL;
    zs.zfree = NULL;
    zs.next_in = (unsigned char*)fp->compressed_block + 18;
    zs.avail_in = block_length - 16;
    zs.next_out = fp->uncompressed_block;
    zs.avail_out = fp->uncompressed_block_size;

    status = inflateInit2(&zs, AJSEQBAM_GZIP_WINDOW_BITS);

    if(status != Z_OK)
    {
        bamReportError(fp, "inflate init failed");
        return -1;
    }

    status = inflate(&zs, Z_FINISH);

    if(status != Z_STREAM_END)
    {
        inflateEnd(&zs);
        bamReportError(fp, "inflate failed");
        return -1;
    }

    status = inflateEnd(&zs);

    if(status != Z_OK)
    {
        bamReportError(fp, "inflate failed");
        return -1;
    }

    return zs.total_out;
}




/* @funcstatic bamHeaderCheck *************************************************
**
** Check a BAM file header
**
** @param [r] header [const char*] Header buffer
** @return [int] 0 on success
**
**
** @release 6.3.0
******************************************************************************/

static int bamHeaderCheck(const char *header)
{
    return (header[0] == AJSEQBAM_GZIP_ID1 &&
            header[1] == (char) AJSEQBAM_GZIP_ID2 &&
            header[2] == Z_DEFLATED &&
            (header[3] & AJSEQBAM_FLG_FEXTRA) != 0 &&
            unpackInt16((const unsigned char*)&header[10]) ==
            AJSEQBAM_BGZF_XLEN &&
            header[12] == AJSEQBAM_BGZF_ID1 &&
            header[13] == AJSEQBAM_BGZF_ID2 &&
            unpackInt16((const unsigned char*)&header[14]) ==
            AJSEQBAM_BGZF_LEN);
}




/* @funcstatic bamCacheFree ***************************************************
**
** Free BAM cache data
**
** @param [u] fp [AjPSeqBamBgzf] BGZ file object
** @return [void]
**
**
** @release 6.3.0
******************************************************************************/

static void bamCacheFree(AjPSeqBamBgzf fp)
{

    /* if fp->open_mode is not 'r' then fp->cache is expected to be null */

    if(!fp->cache)
        return;

    ajTableDel(&fp->cache);

    return;
}




/* @funcstatic bamCacheLoadBlock **********************************************
**
** Load a block from cache
**
** @param [u] fp [AjPSeqBamBgzf] BGZ file object
** @param [r] block_address [ajlong] Block offset in file
** @return [int] Size of block
**
**
** @release 6.3.0
******************************************************************************/

static int bamCacheLoadBlock(AjPSeqBamBgzf fp, ajlong block_address)
{
	const BamPCache p;

	p = ajTableFetchV(fp->cache, &block_address);

	if(!p)
            return 0;

	if(fp->block_length != 0)
            fp->block_offset = 0;

	fp->block_address = block_address;
	fp->block_length = p->size;
	memcpy(fp->uncompressed_block, p->block, AJSEQBAM_MAX_BLOCK_SIZE);
	fseek(fp->file, p->end_offset, SEEK_SET);

	return p->size;
}




/* @funcstatic bamCacheBlock **************************************************
**
** add a block to the cache
** remove an old block if it exceeds maximum
**
** @param [u] fp [AjPSeqBamBgzf] BGZ file
** @param [r] size [int] Cache size of data from current position
** @return [void]
**
**
** @release 6.3.0
******************************************************************************/

static void bamCacheBlock(AjPSeqBamBgzf fp, int size)
{
    BamPCache p;
    BamPCache oldp;

    if(AJSEQBAM_MAX_BLOCK_SIZE >= fp->cache_size)
        return;

    if((ajint) ajTableGetLength(fp->cache) * AJSEQBAM_MAX_BLOCK_SIZE >
       fp->cache_size)
    {
        /*
        ** A better way would be to remove the oldest block in the
        ** cache, but here a random one is removed for simplicity. This
        ** should not have a big impact on performance.
        */

        /* some way to remove a value - or half the values - from the table */
    }

    AJNEW0(p);
    p->size = fp->block_length;
    p->end_offset = fp->block_address + size;
    p->block = malloc(AJSEQBAM_MAX_BLOCK_SIZE);
    memcpy(p->block, fp->uncompressed_block, AJSEQBAM_MAX_BLOCK_SIZE);
    oldp = ajTablePut(fp->cache, &fp->block_address, p);

    if(oldp)
    {
        AJFREE(oldp->block);
        AJFREE(oldp);
    }

    return;
}




/* @funcstatic bamReadBlock ***************************************************
**
** Read the next block of data from a BGZ file
**
** @param [u] fp [AjPSeqBamBgzf] BGZ file handler
** @return [int] Zeo on success
**
**
** @release 6.3.0
******************************************************************************/

static int bamReadBlock(AjPSeqBamBgzf fp)
{
    char header[AJSEQBAM_BLOCK_HEADER_LENGTH];
    int size = 0;
    ajlong block_address;
    int count;
    int block_length;
    char *compressed_block;
    int remaining;

    block_address = ftell(fp->file);

    if(bamCacheLoadBlock(fp, block_address)) 
        return 0;

    count = fread(header, 1, sizeof(header), fp->file);

    if(count == 0)
    {
        fp->block_length = 0;
        return 0;
    }

    size = count;

    if ((unsigned int) count != sizeof(header))
    {
        bamReportError(fp, "read failed");
        return -1;
    }

    if(!bamHeaderCheck(header))
    {
      /*bamReportError(fp, "invalid block header");*/
        return -1;
    }

    block_length = unpackInt16((unsigned char*)&header[16]) + 1;
    compressed_block = (char*) fp->compressed_block;
    memcpy(compressed_block, header, AJSEQBAM_BLOCK_HEADER_LENGTH);
    remaining = block_length - AJSEQBAM_BLOCK_HEADER_LENGTH;
    count = fread(&compressed_block[AJSEQBAM_BLOCK_HEADER_LENGTH], 1,
                  remaining, fp->file);

    if(count != remaining)
    {
        bamReportError(fp, "read failed");
        return -1;
    }

    size += count;
    count = bamInflateBlock(fp, block_length);

    if(count < 0)
        return -1;

    if(fp->block_length != 0)
    {
        /* Do not reset offset if this read follows a seek. */
        fp->block_offset = 0;
    }

    fp->block_address = block_address;
    fp->block_length = count;
    bamCacheBlock(fp, size);

    return 0;
}




/* @func ajSeqBamBgzfRead *****************************************************
**
** Read a BAM file data record of a given length
**
** @param [u] fp [AjPSeqBamBgzf] BGZ file object
** @param [u] data [void*] Buffer
** @param [r] length [int] length of buffer
** @return [int] Number of bytes read
**               -1 on error
**
** @release 6.3.0
******************************************************************************/

int ajSeqBamBgzfRead(AjPSeqBamBgzf fp, void *data, int length)
{
    int bytes_read;
    char *output;
    int available;
    int copy_length;
    char *buffer;

    if(length <= 0)
        return 0;

    if(fp->open_mode != 'r')
    {
        bamReportError(fp, "file not open for reading");
        return -1;
    }

    bytes_read = 0;
    output = data;

    while(bytes_read < length)
    {
        available = fp->block_length - fp->block_offset;

        if(available <= 0)
        {
            if(bamReadBlock(fp) != 0)
            {
                ajDebug("bamReadBlock failed\n");
                return -1;
            }

            available = fp->block_length - fp->block_offset;

            if(available <= 0)
                break;
        }

        copy_length = BAMBGZFMIN(length-bytes_read, available);
        buffer = fp->uncompressed_block;
        memcpy(output, buffer + fp->block_offset, copy_length);
        fp->block_offset += copy_length;
        output += copy_length;
        bytes_read += copy_length;
    }

    if(fp->block_offset == fp->block_length)
    {
        fp->block_address = ftell(fp->file);
        fp->block_offset = 0;
        fp->block_length = 0;
    }

    return bytes_read;
}




/* @func ajSeqBamBgzfFlush ****************************************************
**
** Flush block to output file
**
** @param [u] fp [AjPSeqBamBgzf] Output file
** @return [int] 0 on success, -1 on failure
**
** @release 6.3.0
******************************************************************************/

int ajSeqBamBgzfFlush(AjPSeqBamBgzf fp)
{
    int count;
    int block_length;

    while (fp->block_offset > 0)
    {
        block_length = bamDeflateBlock(fp, fp->block_offset);

        if (block_length < 0)
        {
            return -1;
        }

        count = fwrite(fp->compressed_block, 1, block_length, fp->file);

        if (count != block_length)
        {
            bamReportError(fp, "write failed");
            return -1;
        }

        fp->block_address += block_length;
    }

    return 0;
}




/* @func ajSeqBamBgzfWrite ****************************************************
**
** Write length bytes from data to the file.
**
** @param [u] fp [AjPSeqBamBgzf] BAM file
** @param [r] data [const void*] data buffer
** @param [r] length [int] Length of data buffer to be written
** @return [int] Returns the number of bytes written.
**               Returns -1 on error.
**
** @release 6.3.0
******************************************************************************/

int ajSeqBamBgzfWrite(AjPSeqBamBgzf fp, const void* data, int length)
{
    const char *input;
    int block_length;
    int bytes_written;
    int copy_length;
    char *buffer;

    if(fp->open_mode != 'w')
    {
        bamReportError(fp, "file not open for writing");
        return -1;
    }

    if(fp->uncompressed_block == NULL)
        fp->uncompressed_block = malloc(fp->uncompressed_block_size);

    input = data;
    block_length = fp->uncompressed_block_size;
    bytes_written = 0;

    while(bytes_written < length)
    {
        copy_length = BAMBGZFMIN(block_length - fp->block_offset,
                                 length - bytes_written);

        buffer = fp->uncompressed_block;
        memcpy(buffer + fp->block_offset, input, copy_length);
        fp->block_offset += copy_length;
        input += copy_length;
        bytes_written += copy_length;

        if(fp->block_offset == block_length)
        {
            if(ajSeqBamBgzfFlush(fp) != 0)
                break;
        }
    }

    return bytes_written;
}




/* @func ajSeqBamBgzfClose ****************************************************
**
** Close a BGZ file and free all associated resources.
**
** Does not close the underlying file descriptor if created with bgzf_fdopen.
**
** @param [u] fp [AjPSeqBamBgzf] BGZ file
** @return [int] Returns zero on success, -1 on error.
**
**
** @release 6.3.0
******************************************************************************/

int ajSeqBamBgzfClose(AjPSeqBamBgzf fp)
{
    int block_length;

    if(!fp) return 0;

    if(fp->open_mode == 'w')
    {
        if(ajSeqBamBgzfFlush(fp) != 0)
            return -1;
        /* add an empty last block */
        block_length = bamDeflateBlock(fp, 0);
        fwrite(fp->compressed_block, 1, block_length, fp->file);

        if(fflush(fp->file) != 0)
        {
            bamReportError(fp, "flush failed");
            return -1;
        }
    }

    if(fp->owned_file)
    {
        if(fclose(fp->file) != 0)
            return -1;
    }

    free(fp->uncompressed_block);
    free(fp->compressed_block);
    bamCacheFree(fp);
    free(fp);

    return 0;
}




/* @func ajSeqBamBgzfEof ******************************************************
**
** Check for end of file marker for a BGzip BAM file
**
** @param [u] fp [AjPSeqBamBgzf] Undocumented
** @return [int] 1 if we are at the end of file mark
**               0 if we are higher in the file
**               -1 for an error
**
**
** @release 6.3.0
******************************************************************************/

int ajSeqBamBgzfEof(AjPSeqBamBgzf fp)
{
    static unsigned char magic[28] = "\037\213\010\4\0\0\0\0\0\377\6\0"
        "\102\103\2\0\033\0\3\0\0\0\0\0\0\0\0\0";
    unsigned char buf[28];
    ajint offset;
    int status;

    offset = ftell(fp->file);

    status = fseek(fp->file, -28, SEEK_END);
    if(status != 0)
    {
        ajDebug("seqBamBgzfEof seek end-28 failed errno %d: %s\n",
                errno, strerror(errno));
        return -1;
    }

    /* Ubuntu warns if retval of fread is not tested */
    if(!fread(buf, 1, 28, fp->file))
    {
        ajDebug("seqBamBgzfEof fread failed errno %d: %s\n",
                errno, strerror(errno));
    }
    
    status = fseek(fp->file, offset, SEEK_SET); /* back where we started */
    if(status != 0)
    {
        ajDebug("seqBamBgzfEof seek %d failed errno %d: %s\n",
                offset,  errno, strerror(errno));
        return -1;
    }

    return (memcmp(magic, buf, 28) == 0) ? 1 : 0;
}




/* @func ajSeqBamHeaderRead ***************************************************
**
** Read header section of BAM files
**
** @param [u] gzfile [AjPSeqBamBgzf] BAM file handler
** @return [AjPSeqBamHeader] header object
**
** @release 6.5.0
** @@
******************************************************************************/

AjPSeqBamHeader ajSeqBamHeaderRead(AjPSeqBamBgzf gzfile)
{
    AjPSeqBamHeader header=NULL;
    char bambuf [4] = "   ";
    char* targetname;
    ajint status;
    ajuint i;
    ajuint ilen;
    ajuint ntargets;
    ajuint targetlen;
    ajuint namelen = 0;

    static AjBool bigendian = ajFalse;

    bigendian = ajUtilGetBigendian();

    status = ajSeqBamBgzfEof(gzfile);
    ajDebug("called ajSeqBamBgzfEof status %d errno %d ESPIPE %d\n",
            status, errno, ESPIPE);

    if (status < 0 && (errno == ESPIPE))
    {
	/*
	 ** If the file is a pipe, checking the EOF marker will *always* fail
	 ** with ESPIPE.  Suppress the error message in this case.
	 */
    }
    else if (status < 0) /* allow 0 - older BAM files have no EOF block */
    {
	/* failed and not a PIPE */
	return NULL;
    }

    status = ajSeqBamBgzfRead(gzfile, bambuf, 4);

    ajDebug("called ajSeqBamBgzfRead status %d\n", status);

    if(status < 0)
    {
	return NULL;
    }

    if (strncmp(bambuf, "BAM\001", 4))
    {
	return NULL;
    }

    /* BAM header */

    /* read plain text and the number of reference sequences */
    status = ajSeqBamBgzfRead(gzfile, &ilen, 4);

    if(status != 4)
	ajErr("seqReadBam failed to read reference length");

    if(bigendian)
	ajByteRevUint(&ilen);

    header = ajSeqBamHeaderNew();

    header->text = ajCharNewRes(ilen+1);
    ajSeqBamBgzfRead(gzfile, header->text, ilen);
    ajDebug("header plain text %u '%s'\n", ilen, header->text);

    header->dict = bamHeaderParse2(header->text);
    header->rg2lib = bamHeaderTotable(header->dict, "RG", "ID", "LB");

    ajSeqBamBgzfRead(gzfile, &ntargets, 4);

    if(bigendian)
	ajByteRevUint(&ntargets);

    ajDebug("bam_header_read: # bam targets = %u\n", ntargets);
    header->n_targets = ntargets;

    AJCNEW0(header->target_name, header->n_targets);
    AJCNEW0(header->target_len, header->n_targets);

    /* read reference sequence names and lengths */

    for(i=0; i < ntargets; i++)
    {
	ajSeqBamBgzfRead(gzfile, &namelen, 4);

	if(bigendian)
	    ajByteRevUint(&namelen);

	targetname = ajCharNewRes(namelen+1);
	ajSeqBamBgzfRead(gzfile, targetname, namelen);
	ajSeqBamBgzfRead(gzfile, &targetlen, 4);

	if(bigendian)
	    ajByteRevUint(&targetlen);

	header->target_name[i] = targetname;
	header->target_len[i] = targetlen;

	ajDebug("bam target[%u] %u '%s'\n", i, targetlen, targetname);
    }

    return header;

}




/* @funcstatic bamDestroyHeaderHash *******************************************
**
** Free the hash table in a BAM header object
**
** @param [u] header [AjPSeqBamHeader] Header object
** @return [void]
**
**
** @release 6.3.0
******************************************************************************/

static void bamDestroyHeaderHash(AjPSeqBamHeader header)
{
    if(header->hash)
        ajTableFree(&header->hash);

    return;
}




/* @funcstatic bamHeaderFree **************************************************
**
** Destructor for a BAM header list
**
** @param [d] header [AjPList*] Header list object
** @return [void]
**
**
** @release 6.3.0
******************************************************************************/

static void bamHeaderFree(AjPList *header)
{
    BamPHeaderLine hline = NULL;

    while(ajListGetLength(*header))
    {
        ajListPop(*header, (void**)&hline);

        if(hline)
            bamHeaderLineFree(&hline);
    }

    ajListFree(header);

    return;
}




/* @funcstatic bamHeaderLineFree **********************************************
**
** Free a BAM header line
**
** @param [d] Phline [BamPHeaderLine*] Header line object
** @return [void]
**
**
** @release 6.3.0
******************************************************************************/

static void bamHeaderLineFree(BamPHeaderLine *Phline)
{
    BamPHeaderLine hline = *Phline;
    AjPList tags = hline->tags;
    BamPHeaderTag htag = NULL;

    while(ajListGetLength(tags))
    {
        ajListPop(tags, (void**)&htag);
        AJFREE(htag->value);
        AJFREE(htag);
    }

    ajListFree(&tags);
    AJFREE(*Phline);

    return;
}




/* @funcstatic bamHeaderParse2 ************************************************
**
** Parse BAM file header text
**
** @param [r] headerText [const char*] Header text
** @return [AjPList] list of header records
**
**
** @release 6.3.0
******************************************************************************/

static AjPList bamHeaderParse2(const char *headerText)
{
    AjPList hlines = NULL;
    BamPHeaderLine hline;
    const char *text;
    char *buf=NULL;
    size_t nbuf = 0;

    if(!headerText)
        return 0;

    text = headerText;
    hlines = ajListNew();

    while((text=bamNextline(&buf, &nbuf, text)))
    {
        hline = bamHeaderLineParse(buf);

        if(hline && bamHeaderLineValidate(hline))
            ajListPushAppend(hlines, hline);
        else
        {
            if(hline)
                bamHeaderLineFree(&hline);

            bamHeaderFree(&hlines);
            AJFREE(buf);

            return NULL;
        }
    }

    AJFREE(buf);

    return hlines;
}




/* @funcstatic bamHeaderTotable ***********************************************
**
** Extract table data from a list of BAM header records
**
** @param [r] dict [const AjPList] List of header records
** @param [r] type [const char[2]] Type
** @param [r] key_tag [const char[2]] Key tag
** @param [r] value_tag [const char[2]] Value tag
** @return [AjPTable] Table of BAM header tags and values
**
**
** @release 6.3.0
******************************************************************************/

static AjPTable bamHeaderTotable(const AjPList dict, const char type[2],
                                 const char key_tag[2],
                                 const char value_tag[2])
{
    AjPTable tbl = NULL;
    BamPHeaderLine hline = NULL;
    AjIList k;
    BamPHeaderTag key;
    BamPHeaderTag value;

    tbl = ajTablecharNew(512);

    if(dict == 0)       /* return an empty (not null) hash table */
        return tbl;

    k = ajListIterNewread(dict);

    while(!ajListIterDone(k))
    {
        hline = ajListIterGet(k);

        if(hline->type[0]!=type[0] || hline->type[1]!=type[1])
            continue;
        
        key   = bamHeaderLineHasTag(hline,key_tag);
        value = bamHeaderLineHasTag(hline,value_tag); 

        if(!key || !value)
            continue;
        
        if(ajTablePut(tbl, key->value, value->value))
            ajWarn("[bamHeaderTotable] The key %s not unique.\n",
                   key->value);
    }

    ajListIterDel(&k);

    return tbl;
}




/* @funcstatic bamNextline ****************************************************
**
** Mimics the behaviour of getline, except it returns pointer to the
** next chunk of the text or NULL if everything has been read.
**
** The lineptr should be freed by the caller. The newline character is
** stripped.
**
** @param [u] lineptr [char**] Buffer
** @param [u] n [size_t*] Buffer current size, can be extended
** @param [r] text [const char*] Text starting at current position
** @return [const char*] Next chunk of text, NULL when all is done.
**
**
** @release 6.3.0
******************************************************************************/

static const char* bamNextline(char **lineptr, size_t *n, const char *text)
{
    ajuint len;
    const char *to = NULL;

    to = text;
    
    if(!*to)
        return NULL;

    while(*to && *to!='\n' && *to!='\r')
        to++;

    len = to - text + 1;

    if(*to)
    {
        /* Advance the pointer for the next call */
        if(*to=='\n')
            to++;
        else if(*to=='\r' && *(to+1)=='\n')
            to+=2;
    }

    if(!len)
        return to;

    if(!*lineptr) 
    {
        *lineptr = malloc(len);
        *n = len;
    }
    else if(*n<len) 
    {
        *lineptr = realloc(*lineptr, len);
        *n = len;
    }

    if(!*lineptr)
        ajDie("[bamNextline] Insufficient memory!\n");

    memcpy(*lineptr,text,len);
    (*lineptr)[len-1] = 0;

    return to;
}




/* @func ajSeqBamHeaderLineParse **********************************************
**
** Parse a header line and creates a table with tag-value pairs
** in the header line
**
** @param [r] headerLine [const char*] Header line
** @return [AjPTable] Table with tag-value pairs
**
**
** @release 6.3.0
******************************************************************************/

AjPTable ajSeqBamHeaderLineParse(const char *headerLine)
{
    BamPHeaderLine hline=NULL;
    AjPTable ret=NULL;

    hline = bamHeaderLineParse(headerLine);

    if(!hline)
	return NULL;

    bamHeaderLineValidate(hline);

    ret = bamHeaderLineToTablemap(hline);

    if(!ret)
	return NULL;

    ajTablePut(ret,
	    ajCharNewC("type"),
	    ajCharNewResLenC(hline->type,3,2));

    bamHeaderLineFree(&hline);

    return ret;
}




/* @funcstatic bamHeaderLineToTablemap ****************************************
**
** Looks for a tag key and returns the tags and values in a
** table
**
** @param [r] hline [const BamPHeaderLine] Header line
** @return [AjPTable] Table of tags and values
**
**
** @release 6.5.0
******************************************************************************/

static AjPTable bamHeaderLineToTablemap(const BamPHeaderLine hline)
{
    AjPList tags = hline->tags;
    BamPHeaderTag tag = NULL;
    AjPTable hlinemap = NULL;
    AjIList iter = NULL;

    iter = ajListIterNewread(tags);
    hlinemap = ajTablecharNew(16);
    ajTableSetDestroyvalue(hlinemap, &ajMemFree);

    while(!ajListIterDone(iter))
    {
        tag = ajListIterGet(iter);

        ajTablePut(hlinemap, ajCharNewResLenC(tag->key,3,2),
        	   ajCharNewC(tag->value));
    }

    ajListIterDel(&iter);

    return hlinemap;
}




/* @funcstatic bamHeaderLineParse *********************************************
**
** Parse a header line and creates a header line object
**
** @param [r] headerLine [const char*] Header line
** @return [BamPHeaderLine] Header line object
**
**
** @release 6.3.0
******************************************************************************/

static BamPHeaderLine bamHeaderLineParse(const char *headerLine)
{
    BamPHeaderLine hline;
    BamPHeaderTag tag;
    const char *from, *to;
    int itype;

    from = headerLine;

    if(*from != '@')
    {
        ajWarn("[bamHeaderLineParse] expected '@', got [%s]\n", headerLine);
        return 0;
    }

    to = ++from;

    while(*to && *to!='\t')
        to++;

    if(to-from != 2)
    {
        ajWarn("[bamHeaderLineParse] expected '@XY', got [%s]\n", headerLine);
        return 0;
    }
    
    AJNEW(hline);
    hline->type[0] = from[0];
    hline->type[1] = from[1];
    hline->tags = NULL;

    itype = bamTagExists(hline->type, types);
    
    from = to;

    while(*to && *to=='\t')
        to++;

    if(to-from != 1)
    {
        ajWarn("[bamHeaderLineParse] multiple tabs on line [%s] (%d)\n",
               headerLine,(int)(to-from));
        return 0;
    }

    from = to;

    while(*from)
    {
        while(*to && *to!='\t')
            to++;

        if(!required_tags[itype] && !optional_tags[itype])
            tag = bamTagNew("  ",from,to-1);
        else
            tag = bamTagNew(from,from+3,to-1);

        if(bamHeaderLineHasTag(hline,tag->key)) 
            ajWarn("The tag '%c%c' is present (at least) twice on line [%s]\n",
                   tag->key[0],tag->key[1], headerLine);

        if(!hline->tags)
            hline->tags = ajListNew();

        ajListPushAppend(hline->tags, tag);

        from = to;

        while(*to && *to=='\t')
            to++;

        if(*to && to-from != 1)
        {
            ajWarn("[bamHeaderLineParse] multiple tabs on line [%s] (%d)\n",
                   headerLine,(int)(to-from));
            return 0;
        }

        from = to;
    }

    return hline;
}




/* @funcstatic bamHeaderLineValidate ******************************************
**
** Validate a BAM header record
**
** Must be of an existing type, all tags must be recognised and all
** required tags must be present
**
** @param [u] hline [BamPHeaderLine] Header line object
** @return [int] 1 on success, 0 and  a message on failure
**
**
** @release 6.3.0
******************************************************************************/

static int bamHeaderLineValidate(BamPHeaderLine hline)
{
    AjPList tags = NULL;
    BamPHeaderTag tag = NULL;
    int itype, itag;
    AjIList iter = NULL;
    
    /* Is the type correct? */
    itype = bamTagExists(hline->type, types);

    if(itype==-1) 
    {
        ajWarn("The type [%c%c] not recognised.\n",
               hline->type[0],hline->type[1]);
        return 0;
    }

    /* Has all required tags? */
    itag = 0;

    while(required_tags[itype] && required_tags[itype][itag])
    {
        if(!bamHeaderLineHasTag(hline,required_tags[itype][itag]))
        {
            ajWarn("The tag [%c%c] required for [%c%c] not present.\n",
                   required_tags[itype][itag][0],
                   required_tags[itype][itag][1],
                   hline->type[0],hline->type[1]);

            return 0;
        }

        itag++;
    }

    /* Are all tags recognised? */
    tags = hline->tags;
    iter = ajListIterNewread(tags);

    while(!ajListIterDone(iter))
    {
        tag = ajListIterGet(iter);

        if (!bamTagExists(tag->key,required_tags[itype]) &&
            !bamTagExists(tag->key,optional_tags[itype]))
        {
            ajWarn("Unknown tag [%c%c] for [%c%c].\n",
                   tag->key[0],tag->key[1], hline->type[0],hline->type[1]);

            ajListIterDel(&iter);

            return 0;
        }
    }

    ajListIterDel(&iter);

    return 1;
}




/* @funcstatic bamHeaderLineHasTag ********************************************
**
** Looks for a tag key and returns the tag and value as a
** BamPheaderTag object if found
**
** @param [r] hline [const BamPHeaderLine] Header line
** @param [r] key [const char*] Key to look for
** @return [BamPHeaderTag] Header tag if found, NULL if not
**
**
** @release 6.3.0
******************************************************************************/

static BamPHeaderTag bamHeaderLineHasTag(const BamPHeaderLine hline,
                                         const char *key)
{
    AjPList tags = hline->tags;
    BamPHeaderTag tag = NULL;
    BamPHeaderTag ret = NULL;
    AjIList iter = NULL;

    iter = ajListIterNewread(tags);

    while(!ajListIterDone(iter))
    {
        tag = ajListIterGet(iter);

        if(tag->key[0]==key[0] && tag->key[1]==key[1])
        {
            ret = tag;
            break;
        }
    }

    ajListIterDel(&iter);

    return ret;
}




/* @funcstatic bamTagExists ***************************************************
**
** Look for a tag "XY" in a predefined const char *[] array.
**
** @param [r] tag [const char*] query tag name
** @param [r] tags [const char**] array of tage names
** @return [int] tag number on success, -1 on failure
**
**
** @release 6.3.0
******************************************************************************/

static int bamTagExists(const char *tag, const char **tags)
{
    int itag=0;

    if(!tags)
        return -1;

    while(tags[itag])
    {
        if(tags[itag][0]==tag[0] && tags[itag][1]==tag[1])
            return itag; 

        itag++;
    }

    return -1;
}




/* @funcstatic bamTagNew ******************************************************
**
**  name points to "XY", value_from points to the first character of
**  the value string and value_to points to the last character of the
**  value string.
**
** @param [r] name [const char*] tag name
** @param [r] value_from [const char*] first character of value
** @param [r] value_to [const char*] last character of value
** @return [BamPHeaderTag] Header tag object
**
**
** @release 6.3.0
******************************************************************************/

static BamPHeaderTag bamTagNew(const char *name, const char *value_from,
                               const char *value_to)
{
    BamPHeaderTag tag = NULL;
    int len;

    AJNEW0(tag);
    len = value_to-value_from+1;
    
    tag->key[0] = name[0];
    tag->key[1] = name[1];
    tag->value = malloc(len+1);
    memcpy(tag->value,value_from,len+1);
    tag->value[len] = 0;

    return tag;
}




#define MBAMSKIPTAG(s) do {                                             \
        int type = toupper(*(s));                                       \
        ++(s);                                                          \
        if (type == 'C' || type == 'A')                                 \
            ++(s);                                                      \
        else if (type == 'S')                                           \
            (s) += 2;                                                   \
        else if (type == 'I' || type == 'F')                            \
            (s) += 4;                                                   \
        else if (type == 'D')                                           \
            (s) += 8;                                                   \
        else if (type == 'Z' || type == 'H')                            \
        {                                                               \
            while(*(s))                                                 \
                ++(s);                                                  \
            ++(s);                                                      \
        }                                                               \
    } while(0)





/* @func ajSeqBamAuxAppend ****************************************************
**
** Append auxiliary data tag to a BAM record
**
** @param [u] b [AjPSeqBam] BAM record
** @param [r] tag [const char[2]] Tag name
** @param [r] type [char] Tag type
** @param [r] len [int] data length
** @param [r] data [const unsigned char*] Tag type
** @return [void]
**
**
** @release 6.5.0
******************************************************************************/

void ajSeqBamAuxAppend(AjPSeqBam b, const char tag[2],
		       char type, int len, const unsigned char* data)
{
    int ori_len = b->data_len;

    b->data_len += 3 + len;
    b->l_aux += 3 + len;

    if (b->m_data < b->data_len) {
	b->m_data = b->data_len;
	kroundup32(b->m_data);
	b->data = (unsigned char*)realloc(b->data, b->m_data);
    }
    b->data[ori_len] = tag[0]; b->data[ori_len + 1] = tag[1];
    b->data[ori_len + 2] = type;
    memcpy(b->data + ori_len + 3, data, len);
}




/* @funcstatic bamGetAux ******************************************************
**
** Get auxiliary data tag from a BAM record
**
** @param [r] b [const AjPSeqBam] BAM record
** @param [r] tag [const char[2]] Tag name
** @return [unsigned char*] Tag value
**
**
** @release 6.3.0
******************************************************************************/

static unsigned char* bamGetAux(const AjPSeqBam b, const char tag[2])
{
    unsigned char *s;
    int y;
    int x;

    y = tag[0]<<8 | tag[1];
    
    s = MAJSEQBAMAUX(b);

    while(s < b->data + b->data_len)
    {
        x = (int)s[0]<<8 | s[1];
        s += 2;

        if(x == y)
            return s;

        MBAMSKIPTAG(s);
    }

    return 0;
}




/* @func ajSeqBamBgzfSeek *****************************************************
**
** Seek function for a BAM file
**
** @param [u] fp [AjPSeqBamBgzf] BGZ file object
** @param [r] pos [ajlong] Buffer
** @param [r] where [int] length of buffer
** @return [ajlong] Number of bytes read,
**                  -1 on error
**
** @release 6.5.0
******************************************************************************/

ajlong ajSeqBamBgzfSeek(AjPSeqBamBgzf fp, ajlong pos, int where)
{
    int block_offset;
    ajlong block_address;

    if (fp->open_mode != 'r')
    {
	ajErr("file not open for read");
	return -1;
    }

    if (where != SEEK_SET)
    {
	ajErr("unimplemented seek option");
	return -1;
    }

    block_offset = pos & 0xFFFF;
    block_address = (pos >> 16) & 0xFFFFFFFFFFFFLL;

    if (fseek(fp->file, block_address, SEEK_SET) != 0)
    {
	ajErr("seek failed");
	return -1;
    }

    fp->block_length = 0;  /* indicates current block is not loaded */
    fp->block_address = block_address;
    fp->block_offset = block_offset;

    return 0;
}




/* @func ajSeqBamValidate *****************************************************
**
** Simple validation of a BAM record
**
** Other fields could also be checked, especially the auxiliary data
**
** @param [r] header [const AjPSeqBamHeader] header object
** @param [r] b [const AjPSeqBam] BAM alignment record
** @return [int] 1 on success, 0 otherwise
**
** @release 6.5.0
******************************************************************************/

int ajSeqBamValidate(const AjPSeqBamHeader header, const AjPSeqBam b)
{
    char *s;

    if (b->core.tid < -1 || b->core.mtid < -1)
	return 0;

    if (header && (b->core.tid >= header->n_targets
	    || b->core.mtid >= header->n_targets))
	return 0;

    if (b->data_len < b->core.l_qname)
	return 0;

    s = memchr(MAJSEQBAMQNAME(b), '\0', b->core.l_qname);

    if (s != &MAJSEQBAMQNAME(b)[b->core.l_qname-1])
	return 0;

    return 1;
}




/* @func ajSeqBamHeaderGetRefseqTags ******************************************
**
** Return reference sequence tags, in a table which includes sub-tables for
** each individual reference sequence.
** Each sub-table including tag value pairs for a reference sequence.
**
** @param [r] header [const AjPSeqBamHeader] BAM header
** @return [AjPTable] Table of tables for reference sequence tags
**
**
** @release 6.5.0
******************************************************************************/

AjPTable ajSeqBamHeaderGetRefseqTags(const AjPSeqBamHeader header)
{
    AjPTable ret = NULL;
    BamPHeaderLine hline = NULL;
    AjIList k = NULL;
    BamPHeaderTag uri = NULL;
    BamPHeaderTag md5 = NULL;
    BamPHeaderTag species = NULL;
    BamPHeaderTag seqname = NULL;
    BamPHeaderTag assemid = NULL;
    AjPTable tags = NULL;
    const AjPList dict = NULL;

    dict = header->dict;

    if(dict == NULL)
	return ret;

    ret = ajTablecharNewConst(16);

    k = ajListIterNewread(dict);

    while(!ajListIterDone(k))
    {
	hline = ajListIterGet(k);

	if(hline->type[0] != 'S' || hline->type[1] != 'Q')
	    continue;

	seqname = bamHeaderLineHasTag(hline,"SN");

	if(!seqname)
	{
	    ajErr("ajSeqBamHeaderGetRefseqTags: missing SN tag"
		  " in SQ header line");
	    return NULL;
	}

	tags = ajTablecharNew(8);

	assemid = bamHeaderLineHasTag(hline,"AS");
	if(assemid)
	    ajTablePut(tags, ajCharNewC(assemid->key), assemid->value);

	uri   = bamHeaderLineHasTag(hline,"UR");
	if(uri)
	    ajTablePut(tags, ajCharNewC(uri->key), uri->value);

	md5 = bamHeaderLineHasTag(hline,"M5");
	if(md5)
	    ajTablePut(tags, ajCharNewC(md5->key), md5->value);

	species = bamHeaderLineHasTag(hline,"SP");
	if(species)
	    ajTablePut(tags, ajCharNewC(species->key), species->value);

	ajTablePut(ret, seqname->value, tags);

    }

    ajListIterDel(&k);

    return ret;
}




/* @func ajSeqBamHeaderGetReadgroupTags****************************************
**
** Return read group tags, in a table which includes sub-tables for
** each individual read group.
** Each sub-table including tag value pairs for a read group.
**
** @param [r] header [const AjPSeqBamHeader] BAM header
** @return [AjPTable] Table of tables for read-group tags
**
**
** @release 6.5.0
******************************************************************************/

AjPTable ajSeqBamHeaderGetReadgroupTags(const AjPSeqBamHeader header)
{
    AjPTable ret = NULL;
    BamPHeaderLine hline = NULL;
    AjIList k = NULL;
    BamPHeaderTag tag = NULL;
    AjPTable tags = NULL;
    AjIList iter = NULL;
    const AjPList dict = NULL;
    char* id = NULL;

    dict = header->dict;

    if(dict == NULL)
	return ret;

    ret = ajTablecharNewConst(16);

    k = ajListIterNewread(dict);

    while(!ajListIterDone(k))
    {
	hline = ajListIterGet(k);

	if(hline->type[0] != 'R' || hline->type[1] != 'G') /* RG header line */
	    continue;

	tags = ajTablecharNew(8);

	iter = ajListIterNewread(hline->tags);

	while(!ajListIterDone(iter))
	{
	    tag = ajListIterGet(iter);

	    if(tag->key[0]=='I' && tag->key[1]=='D') /* ID tag for RG line */
	    {
		id = tag->value;
		continue;
	    }
	    ajTablePut(tags, ajCharNewResLenC(tag->key, 3, 2), tag->value);
	}

	ajListIterDel(&iter);

	if(!id)
	{
	    ajErr("ajSeqBamHeaderGetReadgroupTags: missing ID tag"
		    " in RG header line");
	    return NULL;
	}

	ajTablePut(ret, id, tags);

    }

    ajListIterDel(&k);

    return ret;
}




/* @func ajSeqBamHeaderGetSortorder *******************************************
**
** Return sort-order defined in header
**
** @param [r] header [const AjPSeqBamHeader] BAM header
** @return [const char*] sort order for the BAM file
**
**
** @release 6.5.0
******************************************************************************/

const char* ajSeqBamHeaderGetSortorder(const AjPSeqBamHeader header)
{
    const char* ret = NULL;
    BamPHeaderLine hline = NULL;
    AjIList k = NULL;
    BamPHeaderTag so = NULL;

    if(!header || !header->dict)
	return NULL;

    k = ajListIterNewread(header->dict);

    while(!ajListIterDone(k))
    {
	hline = ajListIterGet(k);

	if(hline->type[0] != 'H' || hline->type[1] != 'D')
	    continue;

	so = bamHeaderLineHasTag(hline,"SO");

	if(so)
	    ret = so->value;

	break;

    }

    ajListIterDel(&k);

    return ret;
}
