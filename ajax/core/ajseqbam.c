/*
** This is free software; you can redistribute it and/or
** modify it under the terms of the GNU Library General Public License
** as published by the Free Software Foundation; either version 2
** of the License, or (at your option) any later version.
**
** This program is distributed in the hope that it will be useful,
** but WITHOUT ANY WARRANTY; without even the implied warranty of
** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
** GNU General Public License for more details.
**
** You should have received a copy of the GNU Library General Public License
** along with this program; if not, write to the Free Software
** Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
******************************************************************************/

#include "ajax.h"
#include <stdio.h>
#include <ctype.h>
#include <errno.h>
#include <fcntl.h>

#ifndef WIN32
#include <sys/file.h>
#else
#define open _open
#endif

#include "ajseqbam.h"

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


typedef struct BamSHeaderTag
{
    char *value;
    char key[2];
    char Padding[6];
} BamOHeaderTag;

#define BamPHeaderTag BamOHeaderTag*

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





/* #funcstatic bam_reg2bin ***************************************************
**
** Calculate the minimum bin that contains a region [beg,end).
**
** #param [r] beg [ajuint] start of the region, 0-based
** #param [r] end [ajuint] end of the region, 0-based
** #return [int] bin
**
******************************************************************************/

static inline int bam_reg2bin(ajuint beg, ajuint end)
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

static inline AjPSeqBam bam_copy1(AjPSeqBam bdst, const AjPSeqBam bsrc)
{
    unsigned char *data = NULL;
    int m_data;

    /* backup data and m_data */
    data = bdst->data;
    m_data = bdst->m_data;
    
    
    /* double the capacity */
    if (m_data < bsrc->m_data)
    {
        m_data = bsrc->m_data; kroundup32(m_data);
        data = (unsigned char*)realloc(data, m_data);
    }

    /* copy var-len data */
    memcpy(data, bsrc->data, bsrc->data_len);

    /* copy the rest */
    *bdst = *bsrc;

    /* restore the backup */
    bdst->m_data = m_data;
    bdst->data = data;

    return bdst;
}




/* @func ajSeqBamHeaderNew *****************************************************
**
** Create an empty BAM header object
**
** @return [AjPSeqBamHeader] BAM header object
**
******************************************************************************/

AjPSeqBamHeader ajSeqBamHeaderNew(void)
{
    return (AjPSeqBamHeader)calloc(1, sizeof(AjOSeqBamHeader));
}




/* @func ajSeqBamHeaderNewTextC ***********************************************
**
** Create an empty BAM header object
**
** @param [r]  txt [const char*] BAM/SAM header text string
** @return [AjPSeqBamHeader] BAM header object
**
******************************************************************************/

AjPSeqBamHeader ajSeqBamHeaderNewTextC(const char* txt)
{
    AjPSeqBamHeader ret;

    AJNEW0(ret);

    ret->l_text = strlen(txt);
    ret->text = ajCharNewResC(txt, ret->l_text + 1);

    return ret;
}




/* @func ajSeqBamHeaderDel ****************************************************
**
** Destructor for a BAM header object
**
** @param [d] Pheader [AjPSeqBamHeader*] BAM header object
** @return [void]
**
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
        ajListstrFree(&header->dict);

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




/* @funcstatic bamSwapEndianData ***********************************************
**
** Swap bigendian data in a BAM core data buffer
**
** @param [r] c [const AjPSeqBamCore] BAM core data
** @param [r] data_len [int] Data length
** @param [u] data [unsigned char*] Data buffer
** @return [void]
**
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
    }

    return;
}




/* @func ajSeqBamRead *********************************************************
**
** Read a BAM file record
**
** The file position indicator must be placed right before an
** alignment. Upon success, this function will set the position
** indicator to the start of the next alignment. This function is not
** affected by the machine endianness.
**
** @param [u] fp [AjPSeqBamBgzf] BAM file handler
** @param [u] b [AjPSeqBam] read alignment; all members are updated.
** @return [int] Number of bytes read from the file
** @@
******************************************************************************/

int ajSeqBamRead(AjPSeqBamBgzf fp, AjPSeqBam b)
{
    AjPSeqBamCore c = &b->core;
    ajint block_len;
    ajint ret;
    ajint i;
    ajuint x[8];

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
        b->m_data = b->data_len;
        kroundup32(b->m_data);
        b->data = (unsigned char*)realloc(b->data, b->m_data);
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




/* @func ajSeqBamWrite *******************************************************
**
** Writes BAM alignment data to a BGZ file
**
** @param [u] fp [AjPSeqBamBgzf]BAM file handler
** @param [r] b [const AjPSeqBam] alignment to write
** @return [int] number of bytes written to the file
**
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


static const int DEFAULT_BLOCK_SIZE = 64 * 1024;
static const int MAX_BLOCK_SIZE = 64 * 1024;

#define BLOCK_HEADER_LENGTH 18
static const int BLOCK_FOOTER_LENGTH = 8;

static const int GZIP_ID1 = 31;
static const int GZIP_ID2 = 139;
static const int CM_DEFLATE = 8;
static const int FLG_FEXTRA = 4;
static const int OS_UNKNOWN = 255;
static const int BGZF_ID1 = 66; /* 'B' */
static const int BGZF_ID2 = 67; /* 'C' */
static const int BGZF_LEN = 2;
static const int BGZF_XLEN = 6; /* BGZF_LEN+4 */

static const int GZIP_WINDOW_BITS = -15; /* no zlib header */
static const int Z_DEFAULT_MEM_LEVEL = 8;

static inline void packInt16(unsigned char* buffer, ajushort value);
static inline int unpackInt16(const unsigned char* buffer);
static inline void packInt32(unsigned char* buffer, ajuint value);
static inline int BAMBGZFMIN(int x, int y);




/* #funcstatic packInt16 *****************************************************
**
** Undocumented
******************************************************************************/

static inline void packInt16(unsigned char* buffer, ajushort value)
{
    buffer[0] = (unsigned char) value;
    buffer[1] = value >> 8;
}



/* #funcstatic upackInt16 ****************************************************
**
** Undocumented
******************************************************************************/

static inline int unpackInt16(const unsigned char* buffer)
{
    return (buffer[0] | (buffer[1] << 8));
}




/* #funcstatic packInt32 *****************************************************
**
** Undocumented
******************************************************************************/

static inline void packInt32(unsigned char* buffer, ajuint value)
{
    buffer[0] = value;
    buffer[1] = value >> 8;
    buffer[2] = value >> 16;
    buffer[3] = value >> 24;
}




/* #funcstatic BAMBGZFMIN ****************************************************
**
** Undocumented
******************************************************************************/

static inline int BAMBGZFMIN(int x, int y)
{
    return (x < y) ? x : y;
}




/* @funcstatic bamReportError **************************************************
**
** Save error message in the BGZ file error attribute
**
** @param [u] fp [AjPSeqBamBgzf] BGZ file
** @param [r] message [const char*] message text
** @return [void]
**
******************************************************************************/

static void bamReportError(AjPSeqBamBgzf fp, const char* message)
{
    ajUser("++bamReportError '%s'", message);
    fp->error = message;

    return;
}




/* @funcstatic bamBgzfNew *****************************************************
**
** Initialize a BGZ file object
**
** @return [AjPSeqBamBgzf] BGZ file object
**
******************************************************************************/

static AjPSeqBamBgzf bamBgzfNew(void)
{
    AjPSeqBamBgzf fp;

    fp = calloc(1, sizeof(AjOSeqBamBgzf));
    fp->uncompressed_block_size = MAX_BLOCK_SIZE;
    fp->uncompressed_block = malloc(MAX_BLOCK_SIZE);
    fp->compressed_block_size = MAX_BLOCK_SIZE;
    fp->compressed_block = malloc(MAX_BLOCK_SIZE);
    fp->cache_size = 0;
    fp->cache = ajTableNew(512);

    return fp;
}




/* @funcstatic bamBgzfOpenfdRead **********************************************
**
** Open a BGZ file for reading
**
** @param [r] fd [int] File descriptor of opened file
** @return [AjPSeqBamBgzf] BGZ file object
**
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
    fp->uncompressed_block_size = DEFAULT_BLOCK_SIZE;
    fp->uncompressed_block = NULL;
    fp->compressed_block_size = MAX_BLOCK_SIZE;
    fp->compressed_block = malloc(MAX_BLOCK_SIZE);
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
** @param [r] mode [const char*] Fil eopen mode 'r' 'R' 'w' or 'W'
** @return [AjPSeqBamBgzf] BGZ file object
**
******************************************************************************/

AjPSeqBamBgzf ajSeqBamBgzfOpenC(const char* path,
                                const char*  mode)
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
#endif
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
#endif
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




/* @funcstatic bamDeflateBlock ************************************************
**
** Deflate an uncompressed block
**
** @param [u] fp  [AjPSeqBamBgzf] BGZ file
** @param [r] block_length [int] Uncompressed block length
** @return [int] Length of compressed block
**
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
    buffer[0] = GZIP_ID1;
    buffer[1] = GZIP_ID2;
    buffer[2] = CM_DEFLATE;
    buffer[3] = FLG_FEXTRA;
    buffer[4] = 0; /* mtime */
    buffer[5] = 0;
    buffer[6] = 0;
    buffer[7] = 0;
    buffer[8] = 0;
    buffer[9] = OS_UNKNOWN;
    buffer[10] = BGZF_XLEN;
    buffer[11] = 0;
    buffer[12] = BGZF_ID1;
    buffer[13] = BGZF_ID2;
    buffer[14] = BGZF_LEN;
    buffer[15] = 0;
    buffer[16] = 0; /* placeholder for block length */
    buffer[17] = 0;

    /* loop to retry for blocks that do not compress enough */
    input_length = block_length;
    compressed_length = 0;

    while(1)
    {
        compress_level = fp->is_uncompressed ? 0 : Z_DEFAULT_COMPRESSION;
        zs.zalloc = NULL;
        zs.zfree  = NULL;
        zs.next_in   = fp->uncompressed_block;
        zs.avail_in  = input_length;
        zs.next_out  = (void*)&buffer[BLOCK_HEADER_LENGTH];
        zs.avail_out = buffer_size - BLOCK_HEADER_LENGTH - BLOCK_FOOTER_LENGTH;

        status = deflateInit2(&zs, compress_level, Z_DEFLATED,
                              GZIP_WINDOW_BITS, Z_DEFAULT_MEM_LEVEL,
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
        compressed_length += BLOCK_HEADER_LENGTH + BLOCK_FOOTER_LENGTH;

        if(compressed_length > MAX_BLOCK_SIZE)
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

    status = inflateInit2(&zs, GZIP_WINDOW_BITS);

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
******************************************************************************/

static int bamHeaderCheck(const char *header)
{
    return (header[0] == GZIP_ID1 &&
            header[1] == (char) GZIP_ID2 &&
            header[2] == Z_DEFLATED &&
            (header[3] & FLG_FEXTRA) != 0 &&
            unpackInt16((const unsigned char*)&header[10]) == BGZF_XLEN &&
            header[12] == BGZF_ID1 &&
            header[13] == BGZF_ID2 &&
            unpackInt16((const unsigned char*)&header[14]) == BGZF_LEN);
}




/* @funcstatic bamCacheFree ***************************************************
**
** Free BAM cache data
**
** @param [u] fp [AjPSeqBamBgzf] BGZ file object
** @return [void]
**
******************************************************************************/

static void bamCacheFree(AjPSeqBamBgzf fp)
{
    void **keyarray = NULL;
    void **valarray = NULL;
    ajint i;

    if(fp->open_mode != 'r')
        return;

    if(!fp->cache)
        return;

    ajTableToarrayKeysValues(fp->cache, &keyarray, &valarray);

    for(i = 0; keyarray[i]; i++)
    {
	AJFREE(keyarray[i]);		/* the key   */
	AJFREE(valarray[i]);		/* the value */
    }

    AJFREE(keyarray);
    AJFREE(valarray);
    ajTableFree(&fp->cache);

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
	memcpy(fp->uncompressed_block, p->block, MAX_BLOCK_SIZE);
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
******************************************************************************/

static void bamCacheBlock(AjPSeqBamBgzf fp, int size)
{
    BamPCache p;
    BamPCache oldp;

    if(MAX_BLOCK_SIZE >= fp->cache_size)
        return;

    if((ajint) ajTableGetLength(fp->cache) * MAX_BLOCK_SIZE > fp->cache_size)
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
    p->block = malloc(MAX_BLOCK_SIZE);
    memcpy(p->block, fp->uncompressed_block, MAX_BLOCK_SIZE);
    oldp = ajTablePut(fp->cache, &fp->block_address, p);

    if(oldp)
    {
        AJFREE(oldp->block);
        AJFREE(oldp);
    }

    return;
}




/* @funcstatic bamReadBlock ****************************************************
**
** Read the next block of data from a BGZ file
**
** @param [u] fp [AjPSeqBamBgzf] BGZ file handler
** @return [int] Zeo on success
**
******************************************************************************/

static int bamReadBlock(AjPSeqBamBgzf fp)
{
    char header[BLOCK_HEADER_LENGTH];
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
        bamReportError(fp, "invalid block header");
        return -1;
    }

    block_length = unpackInt16((unsigned char*)&header[16]) + 1;
    compressed_block = (char*) fp->compressed_block;
    memcpy(compressed_block, header, BLOCK_HEADER_LENGTH);
    remaining = block_length - BLOCK_HEADER_LENGTH;
    count = fread(&compressed_block[BLOCK_HEADER_LENGTH], 1,
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
******************************************************************************/

int ajSeqBamBgzfRead(AjPSeqBamBgzf fp, void* data, int length)
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
                ajErr("bamReadBlock failed");
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




/* @funcstatic flush_block ***************************************************
**
** Flush block to output file
**
** @param [u] fp [AjPSeqBamBgzf] Output file
** @return [int] 0 on success, -1 on failure
******************************************************************************/

static int flush_block(AjPSeqBamBgzf fp)
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




/* @func ajSeqBamBgzfWrite *****************************************************
**
** Write length bytes from data to the file.
**
** @param [u] fp [AjPSeqBamBgzf] BAM file
** @param [r] data [const void*] data buffer
** @param [r] length [int] Length of data buffer to be written
** @return [int] Returns the number of bytes written.
**               Returns -1 on error.
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
            if(flush_block(fp) != 0)
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
******************************************************************************/

int ajSeqBamBgzfClose(AjPSeqBamBgzf fp)
{
    int block_length;

    if(fp->open_mode == 'w')
    {
        if(flush_block(fp) != 0)
            return -1;

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
        return -1;

    /* Ubuntu warns if retval of fread is not tested */
    if(!fread(buf, 1, 28, fp->file))
    {
    }
    
    fseek(fp->file, offset, SEEK_SET);

    return (memcmp(magic, buf, 28) == 0) ? 1 : 0;
}




/* @funcstatic bamDestroyHeaderHash *******************************************
**
** Free the hash table in a BAM header object
**
** @param [u] header [AjPSeqBamHeader] Header object
** @return [void]
**
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
******************************************************************************/

static void bamHeaderFree(AjPList *header)
{
    BamPHeaderLine hline = NULL;

    while(ajListGetLength(*header))
    {
        ajListPop(*header, (void*)hline);
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
******************************************************************************/

static void bamHeaderLineFree(BamPHeaderLine *Phline)
{
    BamPHeaderLine hline = *Phline;
    AjPList tags = hline->tags;
    BamPHeaderTag htag = NULL;

    while(ajListGetLength(tags))
    {
        ajListPop(tags, (void*)htag);
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

    tbl = ajTableNew(512);

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
**  mimics the behaviour of getline, except it returns pointer to the
**  next chunk of the text or NULL if everything has been read.
**
**  The lineptr should be freed by the caller. The newline character is
**  stripped.
**
** @param [u] lineptr [char**] Buffer
** @param [u] n [size_t*] Buffer current size, can be extended
** @param [r] text [const char*] Text starting at current position
** @return [const char*] Next chunk of text, NULL when all is done.
**
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




/* @funcstatic bamHeaderLineParse *********************************************
**
** Parse a header line and creates a header line object
**
** @param [r] headerLine [const char*] Header line
** @return [BamPHeaderLine] Header line object
**
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
    
    hline = malloc(sizeof(BamOHeaderLine));
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
            return 0;
        }
    }

    return 1;
}




/* @funcstatic bamHeaderLineHasTag ********************************************
**
** Looks for a tag key and returns the tag and value as a
** BamPheaderTag object if found
**
** @param [r] hline [const BamPHeaderLine] Header line
** @param [r] key [const char*] Key to look for
** @return [BamPHeaderTag] Header tag if found, NULL i fnot
**
******************************************************************************/

static BamPHeaderTag bamHeaderLineHasTag(const BamPHeaderLine hline,
                                         const char *key)
{
    AjPList tags = hline->tags;
    BamPHeaderTag tag;
    AjIList iter = NULL;

    iter = ajListIterNewread(tags);

    while(!ajListIterDone(iter))
    {
        tag = ajListIterGet(iter);

        if(tag->key[0]==key[0] && tag->key[1]==key[1])
            return tag;
    }

    return NULL;
}




/* @funcstatic bamTagExists ***************************************************
**
** Look for a tag "XY" in a predefined const char *[] array.
**
** @param [r] tag [const char*] query tag name
** @param [r] tags [const char**] array of tage names
** @return [int] tag number on success, -1 on failure
**
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
** @param [r] value_from [const char*] first charecter of value
** @param [r] value_to [const char*] last character of value
** @return [BamPHeaderTag] Header tag object
**
******************************************************************************/

static BamPHeaderTag bamTagNew(const char *name, const char *value_from,
                             const char *value_to)
{
    BamPHeaderTag tag = NULL;
    int len;

    tag = malloc(sizeof(BamOHeaderTag));
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





/* @funcstatic bamGetAux *******************************************************
**
** Get auxilliary data tag from a BAM record
**
** @param [r] b [const AjPSeqBam] BAM record
** @param [r] tag [const char[2]] Tag name
** @return [unsigned char*] Tag value
**
******************************************************************************/

static unsigned char* bamGetAux(const AjPSeqBam b, const char tag[2])
{
    unsigned char *s;
    int y;
    int x;

    y = tag[0]<<8 | tag[1];
    
    s = bam1_aux(b);

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
