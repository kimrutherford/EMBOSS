/******************************************************************************
** @source NUCLEUS EST alignment functions
**
** @author Copyright (C) 1996 Richard Mott
** @author Copyright (C) 1998 Peter Rice revised for EMBOSS
** @version 4.0
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

/* ==================================================================== */
/* ========================== include files =========================== */
/* ==================================================================== */

#include "emboss.h"
#include "embest.h"
#include <time.h>
#include <string.h>
#include <stdio.h>




/* @datastatic EstPKeyValue ***************************************************
**
** Key value data structure
**
** @alias EstSKeyValue
** @alias EstOKeyValue
**
** @attr key [float] key score
** @attr value [ajint] value
** @@
******************************************************************************/

typedef struct EstSKeyValue
{
    float key;
    ajint value;
} EstOKeyValue;

#define EstPKeyValue EstOKeyValue*




/* @datastatic EstPCoord ******************************************************
**
** Coordinates data structure
**
** @alias EstSCoord
** @alias EstOCoord
**
** @attr left [ajint] left end
** @attr right [ajint] right end 
** @@
******************************************************************************/

typedef struct EstSCoord
{
    ajint left;
    ajint right;
} EstOCoord;

#define EstPCoord EstOCoord*




/* @datastatic EstPSavePair ***************************************************
**
** Save pairwise matches
**
** @alias EstSSavePair
** @alias EstOSavePair
**
** @attr col [ajint] Column number
** @attr row [ajint] Row number
** @@
******************************************************************************/

typedef struct EstSSavePair
{
    ajint col;
    ajint row;
} EstOSavePair;
#define EstPSavePair EstOSavePair*


static EstPSavePair rpair  = NULL;
static ajint rpairs        = 0;
static ajint rpair_size    = 0;
static ajint rpairs_sorted = 0;

static ajint lsimmat[256][256];
static AjBool verbose;
static AjBool debug;
static float estRand3(ajint *idum);
static ajint estPairRemember(ajint col, ajint row);
static ajint estSavePairCmp(const void *a, const void *b);
static void  estPairInit(ajint max_bytes);
static void  estPairFree(void);
static ajint estDoNotForget(ajint col, ajint row);
static ajint estAlignMidpt(const AjPSeq est, const AjPSeq genome,
			   ajint gap_penalty, ajint intron_penalty,
			    ajint splice_penalty,
			    const AjPSeq splice_sites,
			    ajint middle, ajint *gleft, ajint *gright);

static EmbPEstAlign estAlignRecursive(const AjPSeq est, const AjPSeq genome,
				      ajint match, ajint mismatch,
				      ajint gap_penalty, ajint intron_penalty,
				      ajint splice_penalty,
				      const AjPSeq splice_sites,
				      float max_area);

static void  estWriteMsp(AjPFile ofile, ajint *matches,
			 ajint *len, ajint *tsub,
			 const AjPSeq genome, ajint gsub, ajint gpos,
			 const AjPSeq est,
			 ajint esub, ajint epos, ajint reverse, ajint gapped);

static const char* estShuffleSeq(char *s, ajint *seed);




/* @func embEstSetDebug *******************************************************
**
** Sets debugging calls on
**
** @return [void]
******************************************************************************/

void embEstSetDebug(void)
{
    debug = ajTrue;

    return;
}




/* @func embEstSetVerbose *****************************************************
**
** Sets verbose debugging calls on
**
** @return [void]
******************************************************************************/

void embEstSetVerbose(void)
{
    verbose = ajTrue;

    return;
}




/* @func  embEstGetSeed *******************************************************
**
** Returns a seed for the random number generator, using the system clock.
**
** @return [ajint] seed.
** @@
******************************************************************************/

ajint embEstGetSeed(void)
{
    time_t *tloc;
    ajint seed;

    tloc = NULL;

    seed = ((ajint)time(tloc))% 100000;

    ajDebug("! seed = %d\n", seed);

    return seed;
}




/* @func embEstMatInit ********************************************************
**
** Comparison matrix initialisation.
**
** @param [r] match [ajint] Match code
** @param [r] mismatch [ajint] Mismatch penalty
** @param [r] gap [ajint] Gap penalty
** @param [r] neutral [ajint] Score for ambiguous base positions.
** @param [r] pad_char [char] Pad character for gaps in input sequences
**
** @return [void]
** @@
******************************************************************************/

void embEstMatInit(ajint match, ajint mismatch, ajint gap,
		   ajint neutral, char pad_char)
{
    ajint c1;
    ajint c2;

    for(c1=0;c1<256;c1++)
	for(c2=0;c2<256;c2++)
	{
	    if( c1 == c2 )
	    {
		if( c1 != '*' && c1 != 'n' &&  c1 != 'N' && c1 != '-' )
		    lsimmat[c1][c2] = match;
		else
		    lsimmat[c1][c2] = 0;
	    }
	    else
	    {
		if( c1 == pad_char || c2 == pad_char )
		    lsimmat[c1][c2] = lsimmat[c2][c1] = -gap;
		else if( c1 == 'n' || c2 == 'n' || c1 == 'N' || c2 == 'N' )
		    lsimmat[c1][c2] = lsimmat[c2][c1] = neutral;
		else
		    lsimmat[c1][c2] = lsimmat[c2][c1] = -mismatch;
	    }
	}

    for(c1=0;c1<256;c1++)
    {
	c2 = tolower(c1);
	lsimmat[c1][c2] = lsimmat[c1][c1];
	lsimmat[c2][c1] = lsimmat[c1][c1];
    }

    return;
}




/* @func embEstFindSpliceSites ************************************************
**
** Finds all putative DONOR and ACCEPTOR splice sites in the genomic sequence.
**
** Returns a sequence object whose "dna" should be interpreted as an
** array indicating what kind (if any) of splice site can be found at
** each sequence position.
**
**     DONOR    sites are NNGTNN last position in exon
**
**     ACCEPTOR sites are NAGN last position in intron
**
**     if forward==1 then search fot GT/AG
**     else               search for CT/AC
**
** @param [r] genome [const AjPSeq] Genomic sequence
** @param [r] forward [ajint] Boolean. 1 = forward direction
**
** @return [AjPSeq] Sequence of bitmask codes for splice sites.
** @@
******************************************************************************/

AjPSeq embEstFindSpliceSites(const AjPSeq genome, ajint forward )
{
    AjPSeq sites;
    ajint pos;
    ajint genomelen;
    const char *s;
    char *sitestr;

    sites     = ajSeqNew();
    genomelen = ajSeqGetLen(genome);
    s         = ajSeqGetSeqC(genome);
    sitestr   = ajSeqGetSeqCopyC( genome );


    for(pos=0;pos<genomelen;pos++)
	sitestr[pos] = NOT_A_SITE;

    if( forward )
    {
	/* gene is in forward direction -splice consensus is gt/ag */
	for(pos=1;pos<genomelen-2;pos++)
	{
	    /* last position in exon */
	    if( tolower((ajint) s[pos]) == 'g' &&
	       tolower((ajint) s[pos+1]) == 't' ) /* donor */
		sitestr[pos-1] = ajSysCastItoc((ajuint) sitestr[pos-1] |
					   (ajuint) DONOR);

	    /* last position in intron */
	    if( tolower((ajint) s[pos]) == 'a' &&
	       tolower((ajint) s[pos+1]) == 'g' ) /* acceptor */
		sitestr[pos+1]  = ajSysCastItoc((ajuint) sitestr[pos+1] |
					    (ajuint) ACCEPTOR);
	}

	ajSeqAssignNameC(sites, "forward"); /* so that other functions know */
    }
    else
    {
	/* gene is on reverse strand so splice consensus looks like ct/ac */
	for(pos=1;pos<genomelen-2;pos++)
	{
	    /* last position in exon */
	    if( tolower((ajint) s[pos]) == 'c' &&
	       tolower((ajint) s[pos+1]) == 't' ) /* donor */
		sitestr[pos-1] = ajSysCastItoc((ajuint) sitestr[pos-1] |
					   (ajuint) DONOR);

	    /* last position in intron */
	    if( tolower((ajint) s[pos]) == 'a' &&
	       tolower((ajint) s[pos+1]) == 'c' ) /* acceptor */
		sitestr[pos+1] = ajSysCastItoc((ajuint) sitestr[pos+1] |
					   (ajuint) ACCEPTOR);
	}

	ajSeqAssignNameC(sites,"reverse");  /* so that other functions know */
    }

    ajSeqAssignSeqC(sites, sitestr);
    AJFREE(sitestr);

    return sites;
}




/* @func embEstShuffleSeq *****************************************************
**
** Shuffle the sequence.
**
** @param [u] seq [AjPSeq] Original sequence
** @param [r] in_place [ajint] Boolean 1=shuffle in place
** @param [u] seed [ajint*] Random number seed.
**
** @return [AjPSeq] shuffled sequence.
** @@
******************************************************************************/

AjPSeq embEstShuffleSeq( AjPSeq seq, ajint in_place, ajint *seed )
{
    AjPSeq shuffled;
    AjPStr shufflestr;

    if(!in_place)
	shuffled = ajSeqNewSeq(seq);
    else
	shuffled = seq;

    shufflestr = ajSeqGetSeqCopyS(shuffled);

    estShuffleSeq(ajStrGetuniquePtr(&shufflestr), seed);
    ajSeqAssignSeqS(shuffled, shufflestr);

    return shuffled;
}




/* @funcstatic estShuffleSeq **************************************************
**
** in-place shuffle of a string
**
** @param [u] s [char*] String
** @param [u] seed [ajint*] Seed
**
** @return [const char*] shuffled string.
** @@
******************************************************************************/

static const char* estShuffleSeq( char *s, ajint *seed )
{
    EstPKeyValue tmp;
    ajint n;
    ajint len;

    len = (ajint) strlen(s);

    AJCNEW(tmp, len);

    for(n=0;n<len;n++)
    {
	tmp[n].key = estRand3(seed);
	tmp[n].value = s[n];
    }

    for(n=0;n<len;n++)
	s[n] = ajSysCastItoc(tmp[n].value);

    AJFREE(tmp);

    return s;
}




/* @funcstatic estRand3 *******************************************************
**
** Random number generator.
**
** @param [w] idum [ajint*] Seed
**
** @return [float] Random flaoting point number.
** @@
******************************************************************************/

static float estRand3(ajint *idum)
{
    static ajint inext,inextp;
    static long ma[56];
    static ajint iff = 0;
    long mj;
    long mk;
    ajint i;
    ajint ii;
    ajint k;
    float ZZ;

    static ajint MBIG  = 1000000000;
    static ajint MSEED = 161803398;
    static ajint MZ    = 0;

    ajint FAC;

    FAC = (ajint) (((float)1.0/(float)MBIG));

    if(*idum < 0 || iff == 0)
    {
	iff = 1;
	mj  = MSEED-(*idum < 0 ? -*idum : *idum);
	mj %= MBIG;
	ma[55] = mj;
	mk = 1;

	for(i=1;i<=54;i++)
	{
	    ii = (21*i) % 55;
	    ma[ii] = mk;
	    mk     = mj-mk;

	    if(mk < MZ)
		mk += MBIG;

	    mj = ma[ii];
	}

	for(k=1;k<=4;k++)
	    for(i=1;i<=55;i++)
	    {
		ma[i] -= ma[1+(i+30) % 55];

		if(ma[i] < MZ)
                    ma[i] += MBIG;
	    }

	inext  = 0;
	inextp = 31;
	*idum  = 1;
    }

    if(++inext == 56)
	inext=1;

    if(++inextp == 56)
	inextp=1;

    mj=ma[inext]-ma[inextp];

    if(mj < MZ)
	mj += MBIG;
    ma[inext] = mj;

    ZZ = (float) (mj*FAC);
    ZZ = (ZZ < (float)0.0 ? -ZZ : ZZ );
    ZZ = (ZZ > (float)1.0 ? ZZ-(ajint)ZZ : ZZ);

    return ZZ;
}




/* @func embEstFreeAlign ******************************************************
**
** Free a genomic EST alignment structure
**
** @param [d] ge [EmbPEstAlign*] Genomic EST alignment data structure
**
** @return [void]
** @@
******************************************************************************/

void embEstFreeAlign(EmbPEstAlign *ge)
{
    if( *ge )
    {
	if( (*ge)->align_path )
	    AJFREE( (*ge)->align_path );

	AJFREE(*ge);
    }

    return;
}




/* @func embEstPrintAlign *****************************************************
**
** Print the alignment
**
** @param [u] ofile [AjPFile] Output file
** @param [r] genome [const AjPSeq] Genomic sequence
** @param [r] est [const AjPSeq] EST sequence
** @param [r] ge [const EmbPEstAlign] Genomic EST alignment
** @param [r] width [ajint] Output width (in bases)
**
** @return [void]
** @@
******************************************************************************/

void embEstPrintAlign(AjPFile ofile, const AjPSeq genome, const AjPSeq est,
		      const EmbPEstAlign ge, ajint width )
{
    ajint gpos;
    ajint epos;
    ajint pos;
    ajint len;
    ajint i;
    ajint j;
    ajint max;
    ajint m;
    char *gbuf;
    char *ebuf;
    char *sbuf;
    const char *genomeseq;
    const char *estseq;

    ajint *gcoord;
    ajint *ecoord;
    ajint namelen;

    char format[256];


    genomeseq = ajSeqGetSeqC(genome);
    estseq = ajSeqGetSeqC(est);

    namelen = ajStrGetLen(ajSeqGetNameS(genome)) >
        ajStrGetLen(ajSeqGetNameS(est)) ?
	ajStrGetLen(ajSeqGetNameS(genome)) : ajStrGetLen(ajSeqGetNameS(est)) ;


    sprintf(format, "%%%dS %%6d ", namelen );

    if(ofile)
    {
	ajFmtPrintF(ofile, "\n");
	len = ajSeqGetLen(genome) + ajSeqGetLen(est) + 1;

	AJCNEW(gbuf,len);
	AJCNEW(ebuf,len);
	AJCNEW(sbuf,len);

	AJCNEW(gcoord,len);
	AJCNEW(ecoord,len);

	gpos = ge->gstart;
	epos = ge->estart;
	len = 0;

	for(pos=0;pos<ge->len;pos++)
	{
	    ajint way = ge->align_path[pos];

	    if( way == DIAGONAL  )
	    {
		/* diagonal */
		gcoord[len] = gpos;
		ecoord[len] = epos;
		gbuf[len] = genomeseq[gpos++];
		ebuf[len] = estseq[epos++];
		m = lsimmat[(ajint)gbuf[len]][(ajint)ebuf[len]];

		/*
		** MATHOG, the triple form promotes char to
		** arithmetic type, which
		** generates warnings as it might be able
		** to overflow the char type.  This is
		** equivalent but doesn't trigger any compiler noise
		** sbuf[len] = (char) ( m > 0 ? '|' : ' ' );
		*/

		if(m>0)
		    sbuf[len] = '|';
		else
		    sbuf[len] = ' ';

		len++;
	    }
	    else if(way == DELETE_EST)
	    {
		gcoord[len] = gpos;
		ecoord[len] = epos;
		gbuf[len] = '-';
		ebuf[len] = estseq[epos++];
		sbuf[len] = ' ';
		len++;
	    }
	    else if( way == DELETE_GENOME )
	    {
		gcoord[len] = gpos;
		ecoord[len] = epos;
		gbuf[len] = genomeseq[gpos++];
		ebuf[len] = '-';
		sbuf[len] = ' ';
		len++;
	    }
	    else if( way <= INTRON )
	    {
		/*
		** want enough space to print the first 5 and last 5
		** bases of the intron, plus a string containing the
		** intron length
		*/
		ajint intron_width;
		ajint half_width;
		ajint g;
		char number[30];
		ajint numlen;

		intron_width = ge->align_path[pos+1];
		g = gpos-1;
		half_width = intron_width > 10 ? 5 : intron_width/2;


		sprintf(number," %d ", intron_width );
		numlen = (ajint) strlen(number);

		for(j=len;j<len+half_width;j++)
		{
		    g++;
		    gcoord[j] = gpos-1;
		    ecoord[j] = epos-1;
		    gbuf[j] = ajSysCastItoc(tolower((ajint) genomeseq[g]));
		    ebuf[j] = '.';

		    if(way == FORWARD_SPLICED_INTRON)
			sbuf[j] = '>';
		    else if(way == REVERSE_SPLICED_INTRON)
			sbuf[j] = '<';
		    else
			sbuf[j] = '?';
		}

		len = j;

		for(j=len;j<len+numlen;j++)
		{
		    gcoord[j] = gpos-1;
		    ecoord[j] = epos-1;
		    gbuf[j] = '.';
		    ebuf[j] = '.';
		    sbuf[j] = number[j-len];
		}

		len = j;
		g = gpos + intron_width - half_width-1;

		for(j=len;j<len+half_width;j++)
		{
		    g++;
		    gcoord[j] = gpos-1;
		    ecoord[j] = epos-1;
		    gbuf[j] = ajSysCastItoc(tolower((ajint) genomeseq[g]));
		    ebuf[j] = '.';

		    if(way == FORWARD_SPLICED_INTRON)
			sbuf[j] = '>';
		    else if(way == REVERSE_SPLICED_INTRON)
			sbuf[j] = '<';
		    else
			sbuf[j] = '?';
		}

		gpos += ge->align_path[++pos];
		len = j;
	    }
	}

	for(i=0;i<len;i+=width)
	{
	    max = ( i+width > len ? len : i+width );

	    ajFmtPrintF(ofile, format, ajSeqGetNameS(genome), gcoord[i]+1 );

	    for(j=i;j<max;j++)
		ajFmtPrintF(ofile, "%c",  gbuf[j]);

	    ajFmtPrintF(ofile," %6d\n", gcoord[j-1]+1 );

	    for(j=0;j<namelen+8;j++)
		ajFmtPrintF(ofile, " ");

	    for(j=i;j<max;j++)
		ajFmtPrintF(ofile,"%c", sbuf[j]);

	    ajFmtPrintF(ofile,  "\n");
	    ajFmtPrintF(ofile, format, ajSeqGetNameS(est), ecoord[i]+1 );

	    for(j=i;j<max;j++)
		ajFmtPrintF(ofile, "%c", ebuf[j]);

	    ajFmtPrintF(ofile," %6d\n\n", ecoord[j-1]+1 );
	}

	ajFmtPrintF( ofile, "\nAlignment Score: %d\n", ge->score );

	AJFREE(gbuf);
	AJFREE(ebuf);
	AJFREE(sbuf);
	AJFREE(gcoord);
	AJFREE(ecoord);
    }

    return;
}




/* @func embEstAlignNonRecursive **********************************************
**
** Modified Smith-Waterman/Needleman to align an EST or mRNA to a Genomic
** sequence, allowing for introns.
**
** The recursion is
**
**     {  S[gpos-1][epos]   - gap_penalty
**
**     {  S[gpos-1][epos-1] + D[gpos][epos]
**
**     S[gpos][epos] = max {  S[gpos][epos-1]   - gap_penalty
**
**     {  C[epos]           - intron_penalty
**
**     {  0 (optional, only if ! needleman )
**
**     C[epos] = max{ S[gpos][epos], C[epos] }
**
**     S[gpos][epos] is the score of the best path to the cell gpos, epos
**     C[epos] is the score of the best path to the column epos
**
**
** @param [r] est [const AjPSeq] Sequence of EST
** @param [r] genome [const AjPSeq] Sequence of genomic region
** @param [r] gap_penalty [ajint] Gap penalty
** @param [r] intron_penalty [ajint] Intron penalty
** @param [r] splice_penalty [ajint] Splice site penalty
** @param [r] splice_sites [const AjPSeq] Marked splice sites.
**     The intron_penalty may be modified to splice_penalty if splice_sites is
**     non-null and there are DONOR and ACCEPTOR sites at the start and
**     end of the intron.
** @param [r] backtrack [ajint] Boolean.
**     If backtrack is 0 then only the start and end points and the score
**     are computed, and no path matrix is allocated.
** @param [r] needleman [ajint] Boolean 1 = global alignment
**                                      0 = local alignment
** @param [r] init_path [ajint] Type of initialization for the path.
**     If init_path  is DIAGONAL then the boundary conditions are adjusted
**     so that the optimal path enters the cell (0,0) diagonally. Otherwise
**     it enters from the left (ie as a deletion in the EST)
**
** @return [EmbPEstAlign] Resulting genomic EST alignment
** @@
******************************************************************************/

EmbPEstAlign embEstAlignNonRecursive(const AjPSeq est, const AjPSeq genome,
				     ajint gap_penalty, ajint intron_penalty,
				     ajint splice_penalty,
				     const AjPSeq splice_sites,
				     ajint backtrack, ajint needleman,
				     ajint init_path)
{
    AjPSeq gdup = NULL;
    AjPSeq edup = NULL;
    const char* splice_sites_str;
    unsigned char **ppath = NULL;
    unsigned char *path   = NULL;
    ajint *score1;
    ajint *score2;
    ajint *s1;
    ajint *s2;
    ajint *s3;
    ajint *best_intron_score;
    ajint *best_intron_coord;
    ajint e_len_pack;
    ajint gpos;
    ajint epos;
    ajint glen;
    ajint elen;
    ajint emax = -1;
    ajint gmax = -1;
    ajint max_score = 0;
    ajint diagonal;
    ajint delete_genome;
    ajint delete_est;
    ajint intron;
    const char *gseq;
    const char *eseq;
    char g;
    ajint max;
    ajint total = 0;
    ajint p;
    ajint pos;
    ajint *temp_path = NULL;
    ajint is_acceptor;
    EmbPEstAlign ge;
    EstPCoord start1 = NULL;
    EstPCoord start2 = NULL;
    EstPCoord t1     = NULL;
    EstPCoord t2     = NULL;
    EstPCoord t3     = NULL;

    EstPCoord best_intron_start = NULL;
    EstOCoord best_start;
    ajint splice_type = 0;

    unsigned char direction;
    unsigned char diagonal_path[4]      = { 1, 4, 16, 64 };
    unsigned char delete_est_path[4]    = { 2, 8, 32, 128 };
    unsigned char delete_genome_path[4] = { 3, 12, 48, 192 };
    unsigned char mask[4]               = { 3, 12, 48, 192 };

    /*
     ** path is encoded as 2 bits per cell:
     **
     ** 00 intron
     ** 10 diagonal
     ** 01 delete_est
     ** 11 delete_genome
     ** the backtrack path, packed 4 cells per byte
     */

    char dbgmsg[512] = "<undefined>\n";

    best_start.right = 0;
    best_start.left  = 0;

    glen = ajSeqGetLen(genome);
    elen = ajSeqGetLen(est);
    splice_sites_str = ajSeqGetSeqC(splice_sites);
    e_len_pack = elen/4+1;

    if(debug)
    {
	ajDebug("embEstAlignNonRecursive\n");
	ajDebug("   backtrack:%d needleman:%d, init_path:%d\n",
		backtrack, needleman, init_path);
    }

    AJNEW0(ge);

    if(backtrack)
    {
	AJCNEW(ppath, glen);

	for(gpos=0;gpos<glen;gpos++)
	    AJCNEW(ppath[gpos], e_len_pack);

	AJCNEW(temp_path,  glen+elen);
    }
    else
    {
	AJCNEW(start1,elen+1);
	AJCNEW(start2, elen+1);
	AJCNEW(best_intron_start, elen);

	t1 = start1+1;
	t2 = start2+1;
    }

    AJCNEW(score1, ajSeqGetLen(est)+1);
    AJCNEW(score2, ajSeqGetLen(est)+1);

    s1 = score1+1;
    s2 = score2+1;

    AJCNEW(best_intron_coord, elen+1);
    AJCNEW(best_intron_score, elen+1);

    gdup = ajSeqNewSeq(genome);
    edup = ajSeqNewSeq(est);
    ajSeqFmtLower(gdup);
    ajSeqFmtLower(edup);
    gseq = ajSeqGetSeqC(gdup);
    eseq = ajSeqGetSeqC(edup);

    if(!backtrack)
    {
	/* initialise the boundaries for the start points */
	for(epos=0;epos<elen;epos++)
	{
	    t1[epos].left = 0;
	    t1[epos].right = epos;
	    t2[epos].left = 0;	  /* try initializing t2 explicitly */
	    t2[epos].right = epos; /* otherwise it gets missed on first pass */
	    best_intron_start[epos] = t1[epos];
	}
    }

    if(needleman)
	for(epos=0;epos<elen;epos++)
	{
	    s1[epos] = MINUS_INFINITY;
	    best_intron_score[epos] = MINUS_INFINITY;
	}

    for(gpos=0;gpos<glen;gpos++) /* loop thru GENOME sequence */
    {
	s3 = s1;
	s1 = s2;
	s2 = s3;

	if((gpos % 1000) == 0)
	    ajDebug(" genome base %d\n", gpos);

	g = gseq[gpos];

	if( backtrack )
	    path = ppath[gpos];
	else
	{
	    t3 = t1; t1 = t2; t2 = t3;
	    t2[-1].left = gpos;	       /* set start1[0] to (gpos,0) */
	    t1[-1].left = gpos;	       /* set start1[0] to (gpos,0) */
	    t1[-1].right = 0;
	}

	if( splice_sites && (splice_sites_str[gpos] & ACCEPTOR ) )
	    is_acceptor = 1; /* gpos is last base of putative intron */
	else
	    is_acceptor = 0;

	/* initialisation */

	if( needleman )
	{
	    if(init_path == DIAGONAL || gpos > 0)
		s1[-1] = MINUS_INFINITY;
	    else
		s1[-1] = 0;
	}
	else
	    s1[-1] = 0;

	for(epos=0;epos<elen;epos++) /* loop thru EST sequence */
	{
	    /* align est and genome */

	    diagonal = s2[epos-1] + lsimmat[(ajint)g][(ajint)eseq[epos]];

	    /* single deletion in est */

	    delete_est = s1[epos-1] - gap_penalty;

	    /* single deletion in genome */

	    delete_genome = s2[epos] - gap_penalty;

	    /*
	    ** intron in genome, possibly modified by
	    ** donor-acceptor splice sites
	    */

	    if(is_acceptor &&
	       (splice_sites_str[best_intron_coord[epos]] & DONOR))
		intron = best_intron_score[epos] - splice_penalty;
	    else
		intron = best_intron_score[epos] - intron_penalty;

	    if(delete_est > delete_genome)
		max = delete_est;
	    else
		max = delete_genome;

	    if(diagonal > max)
		max = diagonal;

	    if(intron > max)
		max = intron;

	    if(needleman || max > 0)	/* save this score */
	    {
		if(max == diagonal)	/* match extension */
		{
		    s1[epos] = diagonal;

		    if(backtrack)
			path[epos/4] =  ajSysCastItouc((ajuint) path[epos/4] |
				      	   (ajuint) diagonal_path[epos%4]);
		    else
		    {
			if( t2[epos-1].left == -1 ) /* SW start */
			{
			    t1[epos].left = gpos;
			    t1[epos].right = epos;
			    if(debug && t1[epos].left == 10126)
				sprintf(dbgmsg,
					"t1[%d].left = gpos:%d\n",
					epos, gpos);
			}
			else
			{
			    /* continue previous match */
			    t1[epos] = t2[epos-1];

			    if(debug && t1[epos].left == 10126)
				sprintf(dbgmsg,
					"t1[%d] = t2[epos-1] "
					"left:%d right:%d gpos: %d(a)\n",
					epos, t1[epos].left, t1[epos].right,
					gpos);
			}
		    }
		}
		else if( max == delete_est ) /* (continue) gap in EST */
		{
		    s1[epos] = delete_est;

		    if( backtrack )
		    {			/* <mod> */
			path[epos/4]  =  ajSysCastItouc((ajuint) path[epos/4] |
					    (ajuint) delete_est_path[epos%4]);
		    }
		    else
		    {
			t1[epos] = t1[epos-1];

			if(debug && t1[epos].left == 10126)
			    sprintf(dbgmsg,
				    "t1[%d] = t2[epos-1] left:%d (b)\n",
				    epos, t1[epos].left);
		    }
		}
		else if( max == delete_genome )
		{
		    /* (continue) gap in GENOME */
		    s1[epos] = delete_genome;

		    if( backtrack )
		    {			/* <mod> */
			path[epos/4] = ajSysCastItouc((ajuint) path[epos/4] |
					 (ajuint) delete_genome_path[epos%4]);
		    }
		    else
		    {
			t1[epos] = t2[epos];

			if(debug && t1[epos].left == 10126)
			    sprintf(dbgmsg,
				    "t1[%d] = t2[epos] left:%d\n",
				    epos, t1[epos].left);
		    }
		}
		else
		{
		    /* Intron */
		    s1[epos] = intron;

		    if(!backtrack)
			t1[epos] = best_intron_start[epos];
		}
	    }
	    else
	    {
		/* not worth saving (SW with score < 0 ) */
		s1[epos] = 0;

		if(!backtrack)
		{
		    t1[epos].left = -1;
		    t1[epos].right = -1;
		}
	    }


	    if( best_intron_score[epos] < s1[epos] )
	    {
		/* if( intron > 0 ) */ /* will only need to store
		   if this path is positive */
		if( backtrack )
		    if( estDoNotForget(epos,
				       best_intron_coord[epos]) == 0 )
			/*
			** store the previous path just
			** in case we need it
			*/

		    {
			/*
			** error - stack ran out of memory. Clean up
			** and return NULL
			*/
			ajErr("stack ran out of memory, returning NULL");

			AJFREE(score1);
			AJFREE(score2);
			/* AJFREE(eseq); */ /* copy of pointer from edup */
			/* AJFREE(gseq); */ /* copy of pointer from gdup */
			AJFREE(best_intron_score);
			AJFREE(best_intron_coord);
			AJFREE(temp_path);

			for(gpos=0;gpos<glen;gpos++)
			    AJFREE(ppath[gpos]);

			AJFREE(ppath);
			estPairFree();
			AJFREE(ge);

			ajSeqDel(&gdup);
			ajSeqDel(&edup);

			return NULL;
		    }

		best_intron_score[epos] = s1[epos];
		best_intron_coord[epos] = gpos;

		if(!backtrack)
		    best_intron_start[epos] = t1[epos];
	    }

	    if(!needleman && max_score < s1[epos])
	    {
		max_score = s1[epos];
		emax = epos;
		gmax = gpos;

		if(!backtrack)
		{
		    best_start = t1[epos];

		    if(verbose)
			ajDebug("max_score: %d best_start = t1[%d] "
				"left:%d right:%d\n",
				max_score, epos, best_start.left,
				best_start.right);
		    if(verbose)
			ajDebug("t1 from :%s\n", dbgmsg);
		}
	    }
	}
    }

    /* back track */

    if( needleman )
    {
	ge->gstop = glen-1;
	ge->estop = elen-1;
	ge->score = s1[ge->estop];
    }
    else
    {
	ge->gstop = gmax;
	ge->estop = emax;
	ge->score = max_score;
    }

    if(backtrack)
    {
	pos = 0;

	epos = ge->estop;
	gpos = ge->gstop;
	total = 0;

	/* determine the type of spliced intron (forward or reversed) */

	if(splice_sites)
	{
	    if( ! strcmp( ajSeqGetNameC(splice_sites), "forward") )
		splice_type = FORWARD_SPLICED_INTRON;
	    else if( ! strcmp( ajSeqGetNameC(splice_sites), "reverse") )
		splice_type = REVERSE_SPLICED_INTRON;
	    else
	    {
		ajUser("splice_sites '%s'", ajSeqGetNameC(splice_sites));
		splice_type = INTRON; /* This is really an error - splice_sites
					 MUST have a direction */
	    }
	}

	while(( needleman || total < max_score) && epos >= 0 && gpos >= 0 )
	{
	    direction = ajSysCastItouc(((ajuint)ppath[gpos][epos/4] &
				    (ajuint)mask[epos%4] ) >> (2*(epos%4)));
	    temp_path[pos++] = direction;

	    if((ajuint) direction == INTRON ) /* intron */
	    {
		ajint gpos1;

		if( gpos-best_intron_coord[epos]  <= 0 )
		{
		    if( verbose )
			ajWarn("NEGATIVE intron gpos: %d %d\n",
			       gpos, gpos-best_intron_coord[epos] );
		    gpos1 = estPairRemember(epos, gpos );
		}
		else
		    gpos1 = best_intron_coord[epos];

		if( splice_sites && (splice_sites_str[gpos] & ACCEPTOR ) &&
		   ( splice_sites_str[gpos1] & DONOR ) )
		{
		    total -= splice_penalty;
		    temp_path[pos-1] = splice_type; /* make note that this
						       is a proper intron */
		}
		else
		    total -= intron_penalty;

		temp_path[pos++] = gpos-gpos1; /* intron this far */
		gpos = gpos1;
	    }
	    else if((ajuint) direction == DIAGONAL ) /* diagonal */
	    {
		total += lsimmat[(ajint)gseq[gpos]][(ajint)eseq[epos]];
		epos--;
		gpos--;
	    }
	    else if((ajuint) direction == DELETE_EST ) /* delete_est */
	    {
		total -= gap_penalty;
		epos--;
	    }
	    else			/* delete_genome */
	    {
		total -= gap_penalty;
		gpos--;
	    }
	}

	gpos++;
	epos++;


	ge->gstart = gpos;
	ge->estart = epos;
	ge->len    = pos;

	if(debug)
	    ajDebug("gstart = gpos (a) : %d\n", ge->gstart);

	AJCNEW(ge->align_path, ge->len);

	/* reverse the ge so it starts at the beginning of the sequences */

	for(p=0;p<ge->len;p++)
	{
	    if( temp_path[p] > INTRON ) /* can be INTRON or
					   FORWARD_SPLICED_INTRON or
					   REVERSE_SPLICED_INTRON */
		ge->align_path[pos-p-1] = temp_path[p];
	    else
	    {
		ge->align_path[pos-p-2] = temp_path[p];
		ge->align_path[pos-p-1] = temp_path[p+1];
		p++;
	    }
	}
    }
    else
    {
	ge->gstart = best_start.left;
	ge->estart = best_start.right;

	if(debug)
	    ajDebug("gstart = best_start.left : %d\n", ge->gstart);
    }

    AJFREE(score1);
    AJFREE(score2);

    AJFREE(best_intron_score);
    AJFREE(best_intron_coord);
    ajSeqDel(&gdup);
    ajSeqDel(&edup);

    if( backtrack )
    {
	AJFREE(temp_path);

	for(gpos=0;gpos<glen;gpos++)
	    AJFREE(ppath[gpos]);

	AJFREE(ppath);
	estPairFree();
    }
    else
    {
	AJFREE(start1);
	AJFREE(start2);
	AJFREE(best_intron_start);
    }

    if(debug)
	ajDebug("RETURN: embEstAlignNonRecursive "
		"score %d total: %d gstart %d estart %d "
		"gstop %d estop %d\n",
		ge->score, total, ge->gstart, ge->estart,
		ge->gstop, ge->estop );

    return ge;
}




/* @func embEstAlignLinearSpace ***********************************************
**
** Align EST sequence to genomic in linear space
**
** @param [r] est [const AjPSeq] Sequence of EST
** @param [r] genome [const AjPSeq] Sequence of genomic region
** @param [r] match [ajint] Match score
** @param [r] mismatch [ajint] Mismatch penalty (positive)
** @param [r] gap_penalty [ajint] Gap penalty
** @param [r] intron_penalty [ajint] Intron penalty
** @param [r] splice_penalty [ajint] Splice site penalty
** @param [r] splice_sites [const AjPSeq] Marked splice sites.
**     The intron_penalty may be modified to splice_penalty if splice_sites is
**     non-null and there are DONOR and ACCEPTOR sites at the start and
**     end of the intron.
** @param [r] megabytes [float] Maximum memory allowed in Mbytes for
**        alignment by standard methods.
**
** @return [EmbPEstAlign] Genomic EST alignment
** @@
******************************************************************************/

EmbPEstAlign embEstAlignLinearSpace( const AjPSeq est, const AjPSeq genome,
				    ajint match, ajint mismatch,
				    ajint gap_penalty,
				    ajint intron_penalty,
				    ajint splice_penalty,
				    const AjPSeq splice_sites,
				    float megabytes )
{
    EmbPEstAlign ge;
    EmbPEstAlign rge;
    AjPSeq genome_subseq;
    AjPSeq est_subseq;
    AjPSeq splice_subseq;
    float area;
    float max_area;
    ajint glen;
    ajint elen;

    elen = ajSeqGetLen(est);
    glen = ajSeqGetLen(genome);

    max_area = megabytes*(float)1.0e6;

    if(debug)
	ajDebug("embEstAlignLinearSpace\n");

    estPairInit((ajint)((float)1.0e6*megabytes) );

    area = ((float)glen+(float)1.0)
	*((float)elen+(float)1.0)
	    /(float)4;	 /* divide by 4 as we pack 4 cells per byte */

    if(debug)
	ajDebug("area %.6f max_area %.6f\n", area/1000000.0,
		max_area/1000000.0);

    /* sequences small enough to align by standard methods ? */

    if( area <= max_area )
    {
	if(debug)
	    ajDebug("using non-recursive alignment %d %d   %.6f %.6f\n",
		    glen, elen,
		    area/1000000.0, max_area/1000000.0);

	return embEstAlignNonRecursive(est, genome,
				       gap_penalty, intron_penalty,
				       splice_penalty, splice_sites,
				       1, 0, DIAGONAL);
    }

    /*
    ** need to recursively split
    **
    ** first do a Smith-Waterman without backtracking to find
    ** the start and end of the alignment
    */

    ge = embEstAlignNonRecursive(est, genome,
				 gap_penalty, intron_penalty,
				 splice_penalty, splice_sites,
				 0, 0, DIAGONAL);

    /* extract subsequences corresponding to the aligned regions */

    if(debug)
	ajDebug("sw alignment score %d gstart %d estart %d "
		"gstop %d estop %d\n", ge->score, ge->gstart,
		ge->estart, ge->gstop, ge->estop );

    genome_subseq = ajSeqNewSeq(genome);
    est_subseq = ajSeqNewSeq(est);
    ajSeqSetRange(genome_subseq, ge->gstart+1, ge->gstop+1);
    ajSeqSetRange(est_subseq, ge->estart+1, ge->estop+1);
    ajSeqTrim(genome_subseq);
    ajSeqTrim(est_subseq);

    if( splice_sites )
    {
	splice_subseq = ajSeqNewSeq(splice_sites);
	ajSeqSetRange(splice_subseq, ge->gstart+1, ge->gstop+1 );
	ajSeqTrim(splice_subseq);
    }
    else
	splice_subseq = NULL;

    /* recursively do the alignment */

    rge = estAlignRecursive(est_subseq, genome_subseq, match,
			    mismatch, gap_penalty, intron_penalty,
			    splice_penalty, splice_subseq, max_area);

    ge->len = rge->len;
    ge->align_path = rge->align_path;

    AJFREE(rge);
    ajSeqDel(&genome_subseq);
    ajSeqDel(&est_subseq);
    ajSeqDel(&splice_subseq);

    if(debug)
	ajDebug("RETURN: embEstAlignLinearSpace "
		"score %d gstart %d estart %d "
		"gstop %d estop %d\n",
		ge->score, ge->gstart, ge->estart,
		ge->gstop, ge->estop );

    return ge;
}




/* @funcstatic estAlignRecursive **********************************************
**
** Modified Smith-Waterman/Needleman to align an EST or mRNA to a Genomic
**     sequence, allowing for introns
**
** @param [r] est [const AjPSeq] Sequence of EST
** @param [r] genome [const AjPSeq] Sequence of genomic region
** @param [r] match [ajint] Match score
** @param [r] mismatch [ajint] Mismatch penalty (positive)
** @param [r] gap_penalty [ajint] Gap penalty
** @param [r] intron_penalty [ajint] Intron penalty
** @param [r] splice_penalty [ajint] Splice site penalty
** @param [r] splice_sites [const AjPSeq] Marked splice sites.
**     The intron_penalty may be modified to splice_penalty if splice_sites is
**     non-null and there are DONOR and ACCEPTOR sites at the start and
**     end of the intron.
** @param [r] max_area [float] Maximum memory available for alignment
**            by standard method (allowing 4 bases per byte).
**            Otherwise sequences are split and aligned recursively.
**
** @return [EmbPEstAlign] Resulting genomic EST alignment
** @@
******************************************************************************/

static EmbPEstAlign estAlignRecursive( const AjPSeq est,const  AjPSeq genome,
				      ajint match, ajint mismatch,
				      ajint gap_penalty, ajint intron_penalty,
				      ajint splice_penalty,
				      const AjPSeq splice_sites,
				      float max_area)
{
    ajint middle;
    ajint gleft;
    ajint gright;
    ajint score;
    ajint i;
    ajint j;
    AjPSeq left_splice  = NULL;
    AjPSeq right_splice = NULL;
    AjPSeq left_genome;
    AjPSeq right_genome;
    AjPSeq left_est;
    AjPSeq right_est;
    EmbPEstAlign left_ge;
    EmbPEstAlign right_ge;
    EmbPEstAlign ge;
    float area;
    ajint split_on_del;
    ajint glen;
    ajint elen;

    if(debug)
	ajDebug("estAlignRecursive\n");

    glen = ajSeqGetLen(genome);
    elen = ajSeqGetLen(est);

    area = ((float)glen+(float)1.0)
	*((float)elen+(float)1.0)
	    /(float)4;	 /* divide by 4 as we pack 4 cells per byte */

    if(debug)
	ajDebug("area %.6f max_area %.6f\n", area/1000000.0,
		max_area/1000000.0);

    /* sequences small enough to align by standard methods */

    if( area <= max_area )
    {
	if(debug)
	    ajDebug("using non-recursive alignment %d %d   %.6f %.6f\n",
		    ajSeqGetLen(genome), ajSeqGetLen(est),
		    area/1000000.0, max_area/1000000.0);

	ge = embEstAlignNonRecursive( est, genome,
				     gap_penalty, intron_penalty,
				     splice_penalty, splice_sites,
				     1, 1, DIAGONAL );

	if(ge != NULL)
	{
	    /* success */
	    if(debug)
		ajDebug("RETURN: estAlignRecursive success returns "
			"ge gstart:%d estart:%d gstop:%d estop:%d\n",
			ge->gstart, ge->estart, ge->gstop, ge->estop);
	    return ge;
	}
	else		    /* failure because we ran out of memory */
	{
	    if(debug)
		ajDebug("Stack memory overflow ... splitting\n");
	}
    }
    /* need to recursively split */

    if(debug)
	ajDebug("splitting genome and est\n");

    middle = ajSeqGetLen(est)/2;

    score = estAlignMidpt( est, genome, gap_penalty,
			  intron_penalty, splice_penalty, splice_sites,
			  middle, &gleft, &gright );
    if(debug)
	ajDebug("score %d middle %d gleft %d gright %d\n",
		score, middle, gleft, gright );

    split_on_del = (gleft == gright);


    /* split genome */

    left_genome = ajSeqNewSeq(genome);
    right_genome = ajSeqNewSeq(genome);
    ajSeqSetRange(left_genome,  1, gleft+1 );
    ajSeqSetRange(right_genome, gright+1, ajSeqGetLen(genome));
    ajSeqTrim(left_genome);
    ajSeqTrim(right_genome);

    if( splice_sites )
    {
	left_splice = ajSeqNewSeq(splice_sites);
	right_splice = ajSeqNewSeq(splice_sites);
	ajSeqSetRange(left_splice,  1, gleft+1 );
	ajSeqSetRange(right_splice, gright+1, ajSeqGetLen(genome));
	ajSeqTrim(left_splice);
	ajSeqTrim(right_splice);
    }

    /* split est */

    left_est = ajSeqNewSeq(est);
    right_est = ajSeqNewSeq(est);
    ajSeqSetRange(left_est,  1, middle+1 );
    ajSeqSetRange(right_est, middle+2, ajSeqGetLen(est));
    ajSeqTrim(left_est);
    ajSeqTrim(right_est);

    /* align left and right parts separately */

    if(verbose)
	ajDebug("LEFT\n");

    left_ge = estAlignRecursive(left_est, left_genome, match, mismatch,
				gap_penalty, intron_penalty,
				splice_penalty, left_splice, max_area);

    if(verbose)
	ajDebug("RIGHT\n");

    right_ge = estAlignRecursive( right_est, right_genome, match,
				 mismatch, gap_penalty, intron_penalty,
				 splice_penalty, right_splice, max_area);


    /* merge the alignments */

    AJNEW0(ge);
    ge->score = left_ge->score + right_ge->score;
    ge->gstart = 0;
    ge->estart = 0;
    ge->gstop = ajSeqGetLen(genome)-1;
    ge->estop = ajSeqGetLen(est)-1;

    ge->len = left_ge->len+right_ge->len;
    AJCNEW(ge->align_path, ge->len);

    for(i=0,j=0;j<left_ge->len;i++,j++)
	ge->align_path[i] = left_ge->align_path[j];

    if(split_on_del)			/* merge on an est deletion */
    {
	if(debug)
	    ajDebug("split_on_del split at deletion at %d\n", i);
	ge->align_path[i++] = DELETE_EST;

	for(j=1;j<right_ge->len;i++,j++) /* omit first symbol on
					    right-hand alignment */
	    ge->align_path[i] = right_ge->align_path[j];
    }
    else
	for(j=0;j<right_ge->len;i++,j++)
	    ge->align_path[i] = right_ge->align_path[j];

    ajSeqDel(&left_est);
    ajSeqDel(&right_est);
    ajSeqDel(&left_genome);
    ajSeqDel(&right_genome);

    if(splice_sites)
    {
	ajSeqDel(&left_splice);
	ajSeqDel(&right_splice);
    }

    embEstFreeAlign(&left_ge);
    embEstFreeAlign(&right_ge );

    if(debug)
	ajDebug("RETURN: estAlignRecursive at end returns "
		"ge gstart:%d estart:%d gstop:%d estop:%d\n",
		ge->gstart, ge->estart, ge->gstop, ge->estop);
    return ge;
}




/* @funcstatic estAlignMidpt **************************************************
**
** Modified Needleman-Wunsch to align an EST or mRNA to a Genomic
** sequence, allowing for introns. The recursion is
**
**     {  S[gpos-1][epos]   - gap_penalty
**
**     {  S[gpos-1][epos-1] + D[gpos][epos]
**
**     S[gpos][epos] = max {  S[gpos][epos-1]   - gap_penalty
**
**     {  C[epos]           - intron_penalty
**
**     C[epos] = max{ S[gpos][epos], C[epos] }
**
**     S[gpos][epos] is the score of the best path to the cell gpos, epos
**     C[epos] is the score of the best path to the column epos
**
**     The intron_penalty may be modified to splice_penalty if splice_sites is
**     non-null and there are DONOR and ACCEPTOR sites at the start and
**     end of the intron.
**
**     NB: IMPORTANT:
**
**     The input sequences are assumed to be subsequences chosen so
**     that they align end-to end, with NO end gaps. Call
**     non_recursive_est_to_genome() to get the initial max scoring
**     segment and chop up the sequences appropriately.
**
**     The return value is the alignment score.
**
**     If the alignment crosses middle by a est deletion (ie horizontally) then
**              gleft == gright
**
** @param [r] est [const AjPSeq] Sequence of EST
** @param [r] genome [const AjPSeq] Sequence of genomic region
** @param [r] gap_penalty [ajint] Gap penalty
** @param [r] intron_penalty [ajint] Intron penalty
** @param [r] splice_penalty [ajint] Splice site penalty
** @param [r] splice_sites [const AjPSeq] Marked splice sites.
**     The intron_penalty may be modified to splice_penalty if splice_sites is
**     non-null and there are DONOR and ACCEPTOR sites at the start and
**     end of the intron.
** @param [r] middle [ajint] Sequence mid point position.
**     This Function does not compute the path, instead it finds the
**     genome coordinates where the best path crosses epos=middle, so this
**     should be called recursively to generate the complete alignment in
**     linear space.
** @param [w] gleft [ajint*] genome left coordinate at the crossing point.
**     If the alignment crosses middle in a diagonal fashion then
**              gleft+1 == gright
** @param [w] gright [ajint*] genome right coordinate at the crossing point.
**     If the alignment crosses middle in a diagonal fashion then
**              gleft+1 == gright
**
** @return [ajint] alignment score
** @@
******************************************************************************/

static ajint estAlignMidpt( const AjPSeq est, const AjPSeq genome,
			   ajint gap_penalty,
			   ajint intron_penalty,
			   ajint splice_penalty, const AjPSeq splice_sites,
			   ajint middle, ajint *gleft, ajint *gright )
{
    AjPSeq gdup = NULL;
    AjPSeq edup = NULL;
    ajint *score1;
    ajint *score2;
    ajint *s1;
    ajint *s2;
    ajint *s3;
    ajint *best_intron_score;
    ajint *best_intron_coord;
    ajint gpos;
    ajint epos;
    ajint glen;
    ajint elen;
    ajint score;
    ajint diagonal;
    ajint delete_genome;
    ajint delete_est;
    ajint intron;
    const char *gseq;
    const char *eseq;
    char g;
    ajint max;
    ajint is_acceptor;
    EstPCoord m1;
    EstPCoord m2;
    EstPCoord m3;
    EstPCoord midpt1;
    EstPCoord midpt2;
    EstPCoord best_intron_midpt;
    const char *splice_sites_str;

    splice_sites_str = ajSeqGetSeqC(splice_sites);

    AJCNEW(score1, ajSeqGetLen(est)+1);
    AJCNEW(score2, ajSeqGetLen(est)+1);

    s1 = score1+1;
    s2 = score2+1;


    AJCNEW(midpt1, ajSeqGetLen(est)+1);
    AJCNEW(midpt2, ajSeqGetLen(est)+1);

    m1 = midpt1+1;
    m2 = midpt2+1;

    AJCNEW(best_intron_coord, ajSeqGetLen(est)+1);
    AJCNEW(best_intron_score, ajSeqGetLen(est)+1);
    AJCNEW(best_intron_midpt, ajSeqGetLen(est)+1);


    gdup = ajSeqNewSeq(genome);
    edup = ajSeqNewSeq(est);
    ajSeqFmtLower(gdup);
    ajSeqFmtLower(edup);
    gseq = ajSeqGetSeqC(gdup);
    eseq = ajSeqGetSeqC(edup);
    glen = ajSeqGetLen(genome);
    elen = ajSeqGetLen(est);

    middle++;


    /* initialise the boundary: We want the alignment to start at [0,0] */

    for(epos=0;epos<elen;epos++)
    {
	s1[epos] = MINUS_INFINITY;
	best_intron_score[epos] = MINUS_INFINITY;
    }

    for(gpos=0;gpos<glen;gpos++)
    {
	s3 = s1; s1 = s2; s2 = s3;
	m3 = m1; m1 = m2; m2 = m3;

	g = gseq[gpos];

	if(splice_sites && ( splice_sites_str[gpos] & ACCEPTOR ))
	    is_acceptor = 1; /* gpos is last base of putative intron */
	else
	    is_acceptor = 0;


	/* boundary conditions */

	s1[-1] = MINUS_INFINITY;

	/* the meat */

	for(epos=0;epos<elen;epos++)
	{
	    /* align est and genome */

	    diagonal = s2[epos-1] + lsimmat[(ajint)g][(ajint)eseq[epos]];

	    /* single deletion in est */

	    delete_est = s1[epos-1] - gap_penalty;

	    /* single deletion in genome */

	    delete_genome = s2[epos] - gap_penalty;

	    /*
	    ** intron in genome, possibly modified by
	    ** donor-acceptor splice sites
	    */

	    if(is_acceptor &&
	       (splice_sites_str[best_intron_coord[epos]] & DONOR ))
		intron = best_intron_score[epos] - splice_penalty;
	    else
		intron = best_intron_score[epos] - intron_penalty;

	    if(delete_est > delete_genome)
		max = delete_est;
	    else
		max = delete_genome;

	    if(diagonal > max)
		max = diagonal;

	    if(intron > max)
		max = intron;

	    if(max == diagonal)
	    {
		s1[epos] = diagonal;

		if(epos == middle)
		{
		    m1[epos].left = gpos-1;
		    m1[epos].right = gpos;
		}
		else
		    m1[epos] = m2[epos-1];
	    }
	    else if(max == delete_est)
	    {
		s1[epos] = delete_est;

		if( epos == middle )
		{
		    m1[epos].left = gpos;
		    m1[epos].right = gpos;
		}
		else
		    m1[epos] = m1[epos-1];
	    }
	    else if(max == delete_genome)
	    {
		s1[epos] = delete_genome;
		m1[epos] = m2[epos];
	    }
	    else			/* intron */
	    {
		s1[epos] = intron;
		m1[epos] = best_intron_midpt[epos];
	    }

	    if(best_intron_score[epos] < s1[epos])
	    {
		best_intron_score[epos] = s1[epos];
		best_intron_coord[epos] = gpos;
		best_intron_midpt[epos] = m1[epos];
	    }
	}
    }

    *gleft  = m1[ajSeqGetLen(est)-1].left;
    *gright = m1[ajSeqGetLen(est)-1].right;
    score   = s1[ajSeqGetLen(est)-1];

    if(debug)
	ajDebug("midpt score %d middle %d gleft %d gright %d "
		"est %d genome %d\n",
		score, middle-1, *gleft, *gright, ajSeqGetLen(est),
		ajSeqGetLen(genome));

    AJFREE(score1);
    AJFREE(score2);
    AJFREE(midpt1);
    AJFREE(midpt2);
    AJFREE(best_intron_score);
    AJFREE(best_intron_coord);
    AJFREE(best_intron_midpt);
    ajSeqDel(&gdup);
    ajSeqDel(&edup);

    return score;
}




/* @funcstatic estPairRemember ************************************************
**
** Recall saved pair values for row and column
**
** @param [r] col [ajint] Current column
** @param [r] row [ajint] Current row
**
** @return [ajint] Row number
** @@
******************************************************************************/

static ajint estPairRemember( ajint col, ajint row )
{
    EstOSavePair rp;
    ajint left;
    ajint right;
    ajint middle;
    ajint d;
    ajint bad;

    if(!rpairs_sorted)
    {
	qsort(rpair, rpairs, sizeof(EstOSavePair), estSavePairCmp);
	rpairs_sorted = 1;
    }

    rp.col = col;
    rp.row = row;

    left = 0;
    right = rpairs-1;

    if(debug)
	ajDebug("estPairRemember left: %d right: %d rp rp.col rp.row\n",
		left, right, rp, rp.col, rp.row);

    /*
    ** MATHOG, changed flow somewhat, added "bad" variable, inverted some
    ** tests ( PLEASE CHECK THEM!  I did this because the original version
    ** had two ways to drop into the failure code, but one of them was only
    ** evident after careful reading of the code.
    */

    if((estSavePairCmp(&rpair[left],&rp ) > 0 ) ||
       (estSavePairCmp(&rpair[right],&rp ) < 0 ))
	bad = 1;      /*MATHOG, preceding test's logic was inverted */
    else
    {
	bad = 0;
	while(right-left > 1)
	{
	    /* binary check for row/col */
	    middle = (left+right)/2;
	    d = estSavePairCmp( &rpair[middle], &rp );

	    if( d < 0 )
		left = middle;
	    else if( d >= 0 )
		right = middle;
	}

	if(debug)
	    ajDebug("col %d row %d right: col %d row %d left: col %d row %d\n",
		    col, row, rpair[right].col, rpair[right].row,
		    rpair[left].col,
		    rpair[left].row );

	/*
	** any of these fail indicates failure
	** MATHOG, next test's logic was inverted
	*/
	if( estSavePairCmp( &rpair[right], &rp ) < 0 ||
	   rpair[left].col != col ||
	   rpair[left].row >= row )
	{
	    if(debug)
	    {
		ajDebug("estPairRemember => bad2\n");
		ajDebug("estSavePairCmp( %d+%d, %d+%d) %d\n",
			rpair[right].col, rpair[right].row,
			rp.col, rp.row,
			estSavePairCmp( &rpair[right], &rp ));
		ajDebug("rpair[left].col %d %d\n", rpair[left].col, col);
		ajDebug("rpair[left].row %d %d\n", rpair[left].row, row);
	    }

	    bad = 2;
	}
    }

    /* error - this should never happen  */

    if(bad != 0)
	ajFatal("ERROR in estPairRemember() "
		"left: %d (%d %d) right: %d (%d %d) "
		"col: %d row: %d, bad: %d\n",
		left, rpair[left].col, rpair[left].row, right,
		rpair[right].col, rpair[right].row, col, row, bad);

    return rpair[left].row;
}




/* @funcstatic estSavePairCmp *************************************************
**
** Compare two EstPSavePair values. Return the column difference, or if
** the columns are the same, return the row difference.
**
** A value of zero means the two RPAIRS are identical.
**
** @param [r] a [const void*] First value
** @param [r] b [const void*] Second value
**
** @return [ajint] difference.
** @@
******************************************************************************/

static ajint estSavePairCmp( const void *a, const void *b )
{
    const EstPSavePair A;
    const EstPSavePair B;
    ajint n;

    A = (const EstPSavePair)a;
    B = (const EstPSavePair)b;
    n = A->col - B->col;

    if(n == 0)
	n = A->row - B->row;

    return n;
}




/* @funcstatic estPairInit ****************************************************
**
** Initialise the rpair settings
**
** @param [r] max_bytes [ajint] Maximum memory size (bytes)
**
** @return [void]
** @@
******************************************************************************/

static void estPairInit(ajint max_bytes)
{
    ajint maxpairs = max_bytes/sizeof(EstOSavePair);
    estPairFree();

    rpair_size = maxpairs;
 
    AJCNEW0(rpair, rpair_size);

    return;
}




/* @funcstatic estPairFree ****************************************************
**
** Free the rpairs data structure
**
** @return [void]
** @@
******************************************************************************/

static void estPairFree(void)
{
    if(debug)
	ajDebug("estPairFree: rpairs: %d rpair: %x\n", rpairs, rpair);

    if(rpair)
	AJFREE(rpair);

    rpair = NULL;
    rpair_size = 0;
    rpairs = 0;
    rpairs_sorted = 0;

    return;
}




/* @funcstatic estDoNotForget *************************************************
**
** Saving rpairs row and column values.
**
** @param [r] col [ajint] Current column
** @param [r] row [ajint] Current row
**
** @return [ajint] o upon error.
** @@
******************************************************************************/

static ajint estDoNotForget( ajint col, ajint row )
{
    if( rpairs >= rpair_size )
    {
	rpair_size = (rpairs == 0 ? 10000 : 2*rpairs);

	ajDebug("Rpair resize: %d to %d\n", rpairs, rpair_size);
	AJCRESIZE(rpair, rpair_size);

	if (!rpair)
	{
	    ajDie("Memory limit exceeded in allocating space for rpairs");

	    return 0;		     /* failure - ran out of memory */
	}

	if(verbose)
	{
	    ajDebug("rpairs %d allocated rpair_size %d rpair: %x\n",
		    rpairs, rpair_size, rpair);
	    ajDebug("test rpair[0] %x col %d row %d\n",
		    &rpair[0], rpair[0].col, rpair[0].row);
	    ajDebug("test rpair[%d] %x col %d row %d\n",
		    rpairs, &rpair[rpairs],
		    rpair[rpairs].col, rpair[rpairs].row);
	    ajDebug("test rpair[%d] %x col %d row %d\n",
		    rpair_size-1, &rpair[rpair_size-1],
		    rpair[rpair_size-1].col, rpair[rpair_size-1].row);
	}
    }

    rpair[rpairs].col = col;
    rpair[rpairs].row = row;

    rpairs++;
    rpairs_sorted = 0;

    return 1;				/* success */
}




/* @func embEstOutBlastStyle **************************************************
**
** output in blast style.
**
** @param [u] blast [AjPFile] Output file
** @param [r] genome [const AjPSeq] Genomic sequence
** @param [r] est [const AjPSeq] EST sequence
** @param [r] ge [const EmbPEstAlign] Genomic EST alignment
** @param [r] gap_penalty [ajint] Gap penalty
** @param [r] intron_penalty [ajint] Intron penalty
** @param [r] splice_penalty [ajint] Splice site penalty
** @param [r] gapped [ajint] Boolean. 1 = write a gapped alignment
** @param [r] reverse [ajint] Boolean. 1 = reverse alignment
**
** @return [void]
** @@
******************************************************************************/

void embEstOutBlastStyle(AjPFile blast, const AjPSeq genome, const AjPSeq est,
			 const EmbPEstAlign ge,
			 ajint gap_penalty, ajint intron_penalty,
			 ajint splice_penalty, ajint gapped, ajint reverse)
{

    ajint gsub;
    ajint gpos;
    ajint esub;
    ajint epos;
    ajint tsub;
    ajint p;
    ajint matches = 0;
    ajint len     = 0;
    ajint m;
    ajint total_matches = 0;
    ajint total_len     = 0;
    float percent;
    const char *genomestr;
    const char *eststr;
    ajint goff;
    ajint eoff;

    if(verbose)
	ajDebug("debugging set to %d\n", debug);

    genomestr = ajSeqGetSeqC(genome);
    eststr = ajSeqGetSeqC(est);

    gsub = gpos = ge->gstart;
    esub = epos = ge->estart;
    goff = ajSeqGetOffset(genome);
    eoff = ajSeqGetOffset(est);

    if(debug)
	ajDebug("blast_style_output: gsub %d esub %d\n", gsub, esub);

    if( blast )
    {
	tsub = 0;

	for(p=0;p<ge->len;p++)
	    if(ge->align_path[p] <= INTRON)
	    {
		estWriteMsp(blast, &matches, &len, &tsub, genome, gsub, gpos,
			    est, esub, epos, reverse, gapped);
		if(gapped)
		{
		    ajDebug("Intron: path:%d gpos:%d gstart:%d\n",
			    ge->align_path[p], gpos, ge->gstart);

		    if(ge->align_path[p] == INTRON)
		    {
			ajFmtPrintF(blast,
				    "?Intron  %5d %5.1f %5d %5d %-12S\n",
				    -intron_penalty, (float) 0.0, goff+gpos+1,
				    goff+gpos+ge->align_path[p+1],
				    ajSeqGetNameS(genome));
		    }
		    else		/* proper intron */
		    {
			if( ge->align_path[p] == FORWARD_SPLICED_INTRON )
			    ajFmtPrintF( blast,
					"+Intron  %5d %5.1f %5d %5d %-12s\n",
					-splice_penalty, (float) 0.0,
					goff+gpos+1,
					goff+gpos+ge->align_path[p+1],
					ajSeqGetNameC(genome) );
			else
			    ajFmtPrintF( blast,
					"-Intron  %5d %5.1f %5d %5d %-12s\n",
					-splice_penalty, (float) 0.0,
					goff+gpos+1,
					goff+gpos+ge->align_path[p+1],
					ajSeqGetNameC(genome) );

		    }
		}

		gpos += ge->align_path[++p];
		esub = epos;
		gsub = gpos;
	    }
	    else if(ge->align_path[p] == DIAGONAL)
	    {
		m = lsimmat[(ajint)genomestr[(ajint)gpos]]
		    [(ajint)eststr[(ajint)epos]];
		tsub += m;

		if(m > 0)
		{
		    matches++;
		    total_matches++;
		}

		len++;
		total_len++;
		gpos++;
		epos++;
	    }
	    else if(ge->align_path[p] == DELETE_EST)
	    {
		if(gapped)
		{
		    tsub -= gap_penalty;
		    epos++;
		    len++;
		    total_len++;
		}
		else
		{
		    estWriteMsp(blast, &matches, &len, &tsub, genome, gsub,
				gpos, est, esub, epos, reverse, gapped);
		    epos++;
		    esub = epos;
		    gsub = gpos;
		}
	    }
	    else if(ge->align_path[(ajint)p] == DELETE_GENOME)
	    {
		if(gapped)
		{
		    tsub -= gap_penalty;
		    gpos++;
		    total_len++;
		    len++;
		}
		else
		{
		    estWriteMsp(blast, &matches, &len, &tsub, genome, gsub,
				gpos, est, esub, epos, reverse, gapped);
		    gpos++;
		    esub = epos;
		    gsub = gpos;
		}
	    }

	estWriteMsp(blast, &matches, &len, &tsub, genome, gsub, gpos, est,
		    esub, epos, reverse, gapped);

	if(gapped)
	{
	    if(total_len > 0)
		percent = (total_matches/(float)(total_len))*(float)100.0;
	    else
		percent = (float) 0.0;

	    if(reverse)
		ajFmtPrintF( blast,
			    "\nSpan     %5d %5.1f %5d %5d %-12S %5d "
			    "%5d %-12S  %S\n",
			    ge->score, percent, goff+ge->gstart+1,
			    goff+ge->gstop+1,
			    ajSeqGetNameS(genome),
			    eoff+ajSeqGetLen(est)-ge->estart,
			    eoff+ajSeqGetLen(est)-ge->estop,
			    ajSeqGetNameS(est), ajSeqGetDescS(est) );
	    else
		ajFmtPrintF( blast,
			    "\nSpan     %5d %5.1f %5d %5d %-12S "
			    "%5d %5d %-12S  %S\n",
			    ge->score, percent, goff+ge->gstart+1,
			    goff+ge->gstop+1,
			    ajSeqGetNameS(genome), eoff+ge->estart+1,
			    eoff+ge->estop+1,
			    ajSeqGetNameS(est), ajSeqGetDescS(est) );
	}

    }

    return;
}




/* @funcstatic estWriteMsp ****************************************************
**
** write out the MSP (maximally scoring pair).
**
** @param [u] ofile [AjPFile] Output file
** @param [w] matches [ajint*] Number of matches found
** @param [w] len [ajint*] Length of alignment
** @param [w] tsub [ajint*] Score
** @param [r] genome [const AjPSeq] Genomic sequence
** @param [r] gsub [ajint] Genomic start position
** @param [r] gpos [ajint] Genomic end position
** @param [r] est [const AjPSeq] EST sequence
** @param [r] esub [ajint] EST start position
** @param [r] epos [ajint] EST end position
** @param [r] reverse [ajint] Boolean 1=reverse the EST sequence
** @param [r] gapped [ajint] Boolean 1=full gapped alignment
**                         0=display ungapped segment
**
** @return [void]
** @@
******************************************************************************/

static void estWriteMsp(AjPFile ofile, ajint *matches,
			ajint *len, ajint *tsub,
			const AjPSeq genome, ajint gsub, ajint gpos,
			const AjPSeq est,
			ajint esub, ajint epos, ajint reverse, ajint gapped)
{
    float percent;
    ajint goff;
    ajint eoff;

    goff = ajSeqGetOffset(genome);
    eoff = ajSeqGetOffset(est);

    if( *len > 0 )
	percent = (*matches/(float)(*len))*(float)100.0;
    else
	percent = (float) 0.0;

    if(percent > 0)
    {
	if( gapped )
	    ajFmtPrintF( ofile, "Exon     " );
	else
	    ajFmtPrintF( ofile, "Segment  " );

	if(reverse)
	    ajFmtPrintF(ofile, "%5d %5.1f %5d %5d %-12S %5d %5d %-12S  %S\n",
			*tsub, percent, gsub+1, gpos, ajSeqGetNameS(genome),
			eoff+ajSeqGetLen(est)-esub, 
			eoff+ajSeqGetLen(est)-epos+1,
			ajSeqGetNameS(est), ajSeqGetDescS(est) );
	else
	    ajFmtPrintF(ofile, "%5d %5.1f %5d %5d %-12S %5d %5d %-12S  %S\n",
			*tsub, percent, goff+gsub+1, goff+gpos,
			ajSeqGetNameS(genome),
			eoff+esub+1, eoff+epos, ajSeqGetNameS(est),
			ajSeqGetDescS(est) );
    }

    *matches = *len = *tsub = 0;

    return;
}
