/* @source distmat application
**
**   Calculates the evolutionary distance matrix for a set of
** aligned sequences. Measures the pairwise evolutionary
** distances between aligned sequences. Distances are expressed
** as substitutions per 100 bases or a.a.'s.
**
** Methods to correct for multiple substitutions at a site:
**  Nucleic Acid- Kimura's 2 parameter, Tajima-Nei, Jin-Nei
**                Gamma distance or Tamura methods.
**  Protein     - Kimura method.
**  Nucleic Acid or Protein - Jukes-Cantor method
**
**
** @author Copyright (C) Tim Carver (tcarver@hgmp.mrc.ac.uk)
** @@
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
#include <math.h>




static AjPFloat2d distmat_calc_match(char* const * seqcharptr,
				     ajint len, ajint nseqs,
				     AjBool ambig, AjBool nuc,
				     AjPFloat2d* gap);
static AjPFloat2d distmat_uncorrected(const AjPFloat2d match,
				      const AjPFloat2d gap,
				      float gapwt, ajint len, ajint nseqs);

/* correction methods for multiple subst. */
static AjPFloat2d distmat_JukesCantor(const AjPFloat2d match,
				      const AjPFloat2d gap,
				      float gapwt, ajint mlen, ajint nseqs,
				      AjBool nuc);
static AjPFloat2d distmat_Kimura(char* const * seqcharptr,
				 ajint len, ajint nseqs);
static AjPFloat2d distmat_KimuraProt(char* const * seqcharptr,
				     ajint mlen,
				     ajint nseqs);
static AjPFloat2d distmat_Tamura(char* const * seqcharptr,
				 ajint len, ajint nseqs);
static AjPFloat2d distmat_TajimaNei(char* const * seqcharptr,
				    const AjPFloat2d match,
				    ajint mlen, ajint nseqs);
static AjPFloat2d distmat_JinNei(char* const * seqcharptr,
				 ajint len, ajint nseqs,
				 AjBool calc_a, float var_a);

/* output routine and misc routines */
static void distmat_outputDist(AjPFile outf, ajint nseqs,
			       const AjPSeqset seqset, const AjPFloat2d match,
			       float gapwt, ajint method, AjBool ambig,
			       AjBool nuc, ajint posn, ajint incr);
static float distmat_checkambigProt(ajint t1, ajint t2);
static void distmat_checkSubs(ajint t1, ajint t2, ajint* trans, ajint* tranv);
static void distmat_checkRY(ajint t1, ajint t2, ajint* trans, ajint* tranv);

static char** distmat_getSeq(const AjPSeqset seqset, ajint nseqs, ajint mlen,
			     ajint incr, ajint posn, ajint* len);




int main (int argc, char * argv[])
{
    ajint nseqs;
    ajuint mlen;
    ajint len;
    ajint i;
    ajint method;
    ajint incr;
    ajint posn;

    float gapwt;
    float var_a;

    const char  *p;
    char **seqcharptr;

    AjPSeqset seqset = NULL;
    AjPFloat2d match = NULL;
    AjPFloat2d matchDist = NULL;
    AjPFloat2d gap = NULL;
    AjPStr methodlist = NULL;
    AjPStr methodlist2 = NULL;
    AjPFile outf = NULL;

    AjBool nuc = ajFalse;
    AjBool ambig;
    AjBool calc_a;


    embInit("distmat", argc, argv);

    seqset = ajAcdGetSeqset("sequence");

    if(ajSeqsetIsNuc(seqset))		/* nucleic acid seq */
	nuc = ajTrue;
    else if( ajSeqsetIsProt(seqset))
	nuc = ajFalse;
    else
	embExit();


    outf   = ajAcdGetOutfile("outfile"); /* output filename  */
    ambig  = ajAcdGetBoolean("ambiguous");
    gapwt  = ajAcdGetFloat("gapweight");
    if(nuc)
    {
	methodlist = ajAcdGetListSingle("nucmethod");
        methodlist2 = ajAcdGetListSingle("protmethod");
    }
    else
    {
        methodlist2 = ajAcdGetListSingle("nucmethod");
	methodlist = ajAcdGetListSingle("protmethod");
    }
    posn   = ajAcdGetInt("position");
    calc_a = ajAcdGetBoolean("calculatea");
    var_a  = ajAcdGetFloat("parametera");


    incr = 1;				/* codons to analyse */
    if(posn >= 1 && posn <= 3 && nuc)
    {
	incr = 3;
	posn--;
    }
    else if(posn == 123)
	posn = 0;
    else if(posn == 23 || posn == 13 || posn != 12)
	ajFatal("Choose base positions 1, 2, 3, 12, or 123");

    ajStrToInt(methodlist, &method);

    nseqs = ajSeqsetGetSize(seqset);
    if(nseqs<2)
	ajFatal("Insufficient sequences (%d) to create a matrix",nseqs);

    mlen = ajSeqsetGetLen(seqset);
    for(i=0;i<nseqs;++i)	      /* check seqs are same length */
    {
	p = ajSeqsetGetseqSeqC(seqset,i);
	if(strlen(p)!=mlen)
	    ajWarn("Sequence lengths are not equal!");
	ajSeqsetFmtUpper(seqset);
    }

    seqcharptr = distmat_getSeq(seqset,nseqs,mlen,incr,posn,&len);

    /* look at pairs of seqs for matches */
    if(method == 0 || method == 1 || method == 4 )
	match = distmat_calc_match(seqcharptr,len,nseqs,ambig,nuc,
				   &gap);


    /* No correction made for multiple subst. */
    if(method == 0)
	matchDist = distmat_uncorrected(match,gap,gapwt,len,nseqs);

    /* adjust for multiple substs */
    else if(method == 1)		/* for nucl. & prot. */
	matchDist = distmat_JukesCantor(match,gap,gapwt,len,nseqs,nuc);
    else if(method == 2)
	if(nuc)
	    matchDist = distmat_Kimura(seqcharptr,len,nseqs);
	else
	    matchDist = distmat_KimuraProt(seqcharptr,mlen,nseqs);
    else if(method == 3)		/* for nucl. */
	matchDist = distmat_Tamura(seqcharptr,len,nseqs);
    else if(method == 4)
	matchDist = distmat_TajimaNei(seqcharptr,match,mlen,nseqs);
    else if(method == 5)
	matchDist = distmat_JinNei(seqcharptr,len,nseqs,calc_a,var_a);

    distmat_outputDist(outf,nseqs,seqset,matchDist,gapwt,
		       method,ambig,nuc,posn,incr);

    /* free allocated memory */
    for(i=0;i<nseqs;i++)
	ajCharDel(&seqcharptr[i]);
    AJFREE(seqcharptr);

    ajFloat2dDel(&gap);
    ajFloat2dDel(&match);
    ajFloat2dDel(&matchDist);

    ajSeqsetDel(&seqset);
    ajFileClose(&outf);
    ajStrDel(&methodlist);
    ajStrDel(&methodlist2);

    embExit();

    return 0;
}




/* @funcstatic distmat_Tamura *************************************************
**
** Tamura distance - nucleic acids only.
**
** K Tamura, Mol. Biol. Evol. 1992, 9, 678.
**
** @param [r] seqcharptr [char* const *] Array of sequences as C strings
** @param [r] len [ajint] Length
** @param [r] nseqs [ajint] Number of sequences
** @return [AjPFloat2d] corrected distance matrix
**
******************************************************************************/

static AjPFloat2d distmat_Tamura(char* const * seqcharptr,
				 ajint len, ajint nseqs)
{
    ajint i;
    ajint j;
    ajint k;

    ajint t1;
    ajint t2;
    ajint m;
    ajint trans;
    ajint tranv;
    ajint ival;
    
    float P;
    float Q;
    float D;
    float C;
    float geecee;
    float cgap;
    float X1;
    float X2;

    AjPInt2d Ptrans = NULL;
    AjPInt2d Qtranv = NULL;
    AjPInt2d score  = NULL;

    AjPFloat2d matDist = NULL;
    AjPFloat2d gap     = NULL;
    AjPFloat2d GC      = NULL;

    Ptrans  = ajInt2dNew();
    Qtranv  = ajInt2dNew();
    score   = ajInt2dNew();

    matDist = ajFloat2dNew();
    GC      = ajFloat2dNew();
    gap     = ajFloat2dNew();


    /* initialise array */
    for(i=0;i<nseqs;++i)
    {
	for(j=0;j<nseqs;++j)
	{
	    ajFloat2dPut(&matDist,i,j,0.);
	    ajFloat2dPut(&GC,i,j,0.);
	    ajFloat2dPut(&gap,i,j,0.);
	    ajInt2dPut(&Ptrans,i,j,0);
	    ajInt2dPut(&Qtranv,i,j,0);
	    ajInt2dPut(&score,i,j,0);
	}
    }

    /*
    ** calc GC content for each seq for each pair
    ** of seq - ignoring gap posns in both seqs
    */
    for(i=0;i<nseqs;++i)
    {
	for(j=0; j<nseqs; j++)
	{
	    for(k=0;k<len;++k)
	    {
		cgap = ajFloat2dGet(gap,i,j);
		geecee = ajFloat2dGet(GC,i,j);

		t1 = toupper((int) seqcharptr[j][k]);
		t2 = toupper((int) seqcharptr[i][k]);


		if(strchr("-NXWRYMKBVDH",t1) || strchr("-NXWRYMKBVDH",t2))
		    ++cgap;
		else if(strchr("GCS",t2))
		    ++geecee;

		ajFloat2dPut(&GC,i,j,geecee);
		ajFloat2dPut(&gap,i,j,cgap);
	    }
	}
    }

    /* fraction GC content */
    for(i=0;i<nseqs;++i)
    {
	for(j=0; j<nseqs; j++)
	{
	    cgap = ajFloat2dGet(gap,i,j);
	    geecee = ajFloat2dGet(GC,i,j)/((float)len-cgap);
	    ajFloat2dPut(&GC,i,j,geecee);
	}
    }

    /* calc transition & transversion subst.'s */
    for(i=0;i<nseqs;i++)
    {
	for(j=i+1;j<nseqs;j++)
	{
	    for(k=0; k< len; k++)
	    {
		t1 = toupper((int) seqcharptr[i][k]);
		t2 = toupper((int) seqcharptr[j][k]);
		if(!strchr("-",t2) && !strchr("-",t1))
		{
		    trans = ajInt2dGet(Ptrans,i,j);
		    tranv = ajInt2dGet(Qtranv,i,j);

		    distmat_checkSubs(t1,t2,&trans,&tranv);
		    ajInt2dPut(&Ptrans,i,j,trans);
		    ajInt2dPut(&Qtranv,i,j,tranv);
		    m = ajInt2dGet(score,i,j)+1;
		    ajInt2dPut(&score,i,j,m);
		}
	    }
	}
    }


    /* calc distance matrix */
    for(i=0;i<nseqs;i++)
    {
	for(j=i+1;j<nseqs;j++)
	{
	    C = ajFloat2dGet(GC,j,i)+ajFloat2dGet(GC,i,j);
	    C = C - (2*ajFloat2dGet(GC,j,i)*ajFloat2dGet(GC,i,j));
            ival = ajInt2dGet(Ptrans,i,j);
	    X1 = (float) ival;
            ival = ajInt2dGet(score,i,j);
	    X2 = (float) ival;
	    P = X1/X2;
	    
            ival = ajInt2dGet(Qtranv,i,j);
	    X1= (float) ival;
            ival = ajInt2dGet(score,i,j);
	    X2 = (float) ival;
	    Q = X1/X2;

	    if(P != 0.)
		P = P/C;
	    D = (float) (-(C*log(1-P-Q)) - (0.5*(1-C)*log(1-2*Q)));
	    ajFloat2dPut(&matDist,i,j,D);
	}
    }

    ajInt2dDel(&Ptrans);
    ajInt2dDel(&Qtranv);
    ajInt2dDel(&score);
    ajFloat2dDel(&GC);
    ajFloat2dDel(&gap);

    return matDist;
}




/* @funcstatic distmat_Kimura *************************************************
**
** Kimura 2-parameter distance - nucleic acid only.
**
** M Kimura, J. Mol. Evol., 1980, 16, 111.
**
** @param [r] seqcharptr [char* const *] Array of sequences as C strings
** @param [r] len [ajint] Length
** @param [r] nseqs [ajint] Number of sequences
** @return [AjPFloat2d] corrected distance matrix
**
******************************************************************************/

static AjPFloat2d distmat_Kimura(char* const * seqcharptr,
				 ajint len, ajint nseqs)
{
    ajint i;
    ajint j;
    ajint k;

    ajint t1;
    ajint t2;

    float P;
    float Q;
    float D;

    float X1;
    float X2;

    ajint m;
    ajint trans;
    ajint tranv;
    ajint ival;
    
    AjPFloat2d matDist = NULL;
    AjPInt2d Ptrans    = NULL;
    AjPInt2d Qtranv    = NULL;
    AjPInt2d match     = NULL;



    matDist = ajFloat2dNew();
    Ptrans  = ajInt2dNew();
    Qtranv  = ajInt2dNew();
    match   = ajInt2dNew();


    /* initialise array */
    for(i=0;i<nseqs;++i)
    {
	for(j=i+1;j<nseqs;++j)
	{
	    ajFloat2dPut(&matDist,i,j,0.);
	    ajInt2dPut(&Ptrans,i,j,0);
	    ajInt2dPut(&Qtranv,i,j,0);
	    ajInt2dPut(&match,i,j,0);
	}
    }

    /* calc transition & transversion subst.'s */
    for(i=0;i<nseqs;i++)
    {
	for(j=i+1;j<nseqs;j++)
	{
	    for(k=0; k< len; k++)
	    {
		t1 = toupper((int) seqcharptr[i][k]);
		t2 = toupper((int) seqcharptr[j][k]);

		if(!strchr("-",t2) && !strchr("-",t1))
		{
		    trans = ajInt2dGet(Ptrans,i,j);
		    tranv = ajInt2dGet(Qtranv,i,j);

		    distmat_checkSubs(t1,t2,&trans,&tranv);
		    distmat_checkRY(t1,t2,&trans,&tranv);
		    ajInt2dPut(&Ptrans,i,j,trans);
		    ajInt2dPut(&Qtranv,i,j,tranv);
		    m = ajInt2dGet(match,i,j)+1;
		    ajInt2dPut(&match,i,j,m);
		}
	    }
	}
    }


    /* calc distance matrix */
    for(i=0;i<nseqs;i++)
    {
	for(j=i+1;j<nseqs;j++)
	{
            ival = ajInt2dGet(Ptrans,i,j);
	    X1 = (float) ival;
            ival = ajInt2dGet(match,i,j);
	    X2 = (float) ival;
	    P = X1/X2;

            ival = ajInt2dGet(Qtranv,i,j);
	    X1 = (float) ival;
            ival = ajInt2dGet(match,i,j);
	    X2 = (float) ival;
	    Q = X1/X2;

	    D = (float) (-0.5*log((1-(2*P)-Q)*sqrt(1-(2*Q))));

	    ajFloat2dPut(&matDist,i,j,D);
	}
    }

    ajInt2dDel(&Ptrans);
    ajInt2dDel(&Qtranv);
    ajInt2dDel(&match);

    return matDist;
}




/* @funcstatic distmat_KimuraProt *********************************************
**
** Kimura protein distance
**
** @param [r] seqcharptr [char* const *] Array of sequences as C strings
** @param [r] mlen [ajint] Length
** @param [r] nseqs [ajint] Number of sequences
** @return [AjPFloat2d] corrected distance matrix
**
******************************************************************************/

static AjPFloat2d distmat_KimuraProt(char* const * seqcharptr, ajint mlen,
				     ajint nseqs)
{
    ajint i;
    ajint j;
    ajint k;
    ajint mi;
    ajint ival;
    
    char m1;
    char m2;

    float D;
    float m;

    float X2;

    AjPFloat2d matDist = NULL;
    AjPFloat2d match   = NULL;
    AjPInt2d scored    = NULL;



    matDist = ajFloat2dNew();
    match   = ajFloat2dNew();
    scored  = ajInt2dNew();


    /* initialise array */
    for(i=0;i<nseqs;++i)
    {
	for(j=i+1;j<nseqs;++j)
	{
	    ajFloat2dPut(&matDist,i,j,0.);
	    ajFloat2dPut(&match,i,j,0);
	    ajInt2dPut(&scored,i,j,0);
	}
    }

    /* calc matches */
    for(i=0;i<nseqs;i++)
    {
	for(j=i+1;j<nseqs;j++)
	{
	    for(k=0; k< mlen; k++)
	    {
		m1 = seqcharptr[i][k];
		m2 = seqcharptr[j][k];
		if(m1 != '-' && m2 != '-')
		{

		    m = ajFloat2dGet(match,i,j)+
			distmat_checkambigProt(toupper((int) m1),
					       toupper((int) m2));
		    ajFloat2dPut(&match,i,j,m);
		    mi = ajInt2dGet(scored,i,j)+1;
		    ajInt2dPut(&scored,i,j,mi);
		}
	    }
	}
    }


    /* calc distance matrix */
    for(i=0;i<nseqs;i++)
    {
	for(j=i+1;j<nseqs;j++)
	{
            ival = ajInt2dGet(scored,i,j);
	    X2 = (float) ival;
	    D = (float) (1.-(ajFloat2dGet(match,i,j)/X2));
	    D = (float) (-log(1-D-(0.2*D*D)));
	    ajFloat2dPut(&matDist,i,j,D);
	}
    }

    ajInt2dDel(&scored);
    ajFloat2dDel(&match);

    return matDist;
}




/* @funcstatic distmat_calc_match *********************************************
**
** Sum the no. of matches between each pair of sequence in an
** alignment.
**
** @param [r] seqcharptr [char* const *] Array of sequences as C strings
** @param [r] len [ajint] Length
** @param [r] nseqs [ajint] Number of sequences
** @param [r] ambig [AjBool] Ambiguity codes
** @param [r] nuc [AjBool] Nucleotide
** @param [u] gap [AjPFloat2d*] Gaps
** @return [AjPFloat2d] corrected distance matrix
**
******************************************************************************/

static AjPFloat2d distmat_calc_match(char* const * seqcharptr,
				     ajint len, ajint nseqs,
				     AjBool ambig, AjBool nuc, AjPFloat2d* gap)
{

    ajint i;
    ajint j;
    ajint k;

    char m1;
    char m2;

    float m;
    AjPFloat2d match = NULL;



    match = ajFloat2dNew();
    *gap  = ajFloat2dNew();


    /* initialise arrays */
    for(i=0;i<nseqs;++i)
    {
	for(j=0;j<nseqs;++j)
	{
	    ajFloat2dPut(&match,i,j,0);
	    ajFloat2dPut(gap,i,j,0);
	}
    }


    for(i=0;i<nseqs;i++)
	for(j=i+1;j<nseqs;j++)
	    for(k=0; k< len; k++)
	    {
		m1 = seqcharptr[i][k];
		m2 = seqcharptr[j][k];
		if(ambig && nuc)	/* using -ambiguous */
		{
		    m = ajBaseAlphacharCompare(m1,m2);
		    m = ajFloat2dGet(match,i,j)+m;
		    ajFloat2dPut(&match,i,j,m);
		}
		else if( ambig && !nuc && m1 != '-' && m2 != '-')
		{
		    m = distmat_checkambigProt(toupper((int) m1),
					       toupper((int) m2));
		    m = ajFloat2dGet(match,i,j)+m;
		    ajFloat2dPut(&match,i,j,m);
		}
		else if( m2 == m1 && m1 != '-' )
		{
		    m = ajFloat2dGet(match,i,j)+1;
		    ajFloat2dPut(&match,i,j,m);
		}

		if( m1 == '-' || m2 == '-' ) /* gap in seq */
		{
		    m = ajFloat2dGet(*gap,i,j)+1;
		    ajFloat2dPut(gap,i,j,m);
		}
	    }

    return match;
}




/* @funcstatic distmat_uncorrected ********************************************
**
** No correction for multiple substitutions is used in the calculation
** of the distance matrix.
**
**        D = p-distance = 1 - (matches/(posns_scored + gaps*gap_penalty))
**
** @param [r] match [const AjPFloat2d] Matches
** @param [r] gap [const AjPFloat2d] Gaps
** @param [r] gapwt [float] Gap weight
** @param [r] len [ajint] Length
** @param [r] nseqs [ajint] Number of sequences
** @return [AjPFloat2d] uncorrected distance matrix D
**
******************************************************************************/

static AjPFloat2d distmat_uncorrected(const AjPFloat2d match,
				      const AjPFloat2d gap,
				      float gapwt, ajint len, ajint nseqs)
{
    ajint i;
    ajint j;

    float m;
    float g;
    float D;

    AjPFloat2d matchUn = NULL;


    matchUn = ajFloat2dNew();
    for(i=0;i<nseqs;++i)
        for(j=0;j<nseqs;++j)
            ajFloat2dPut(&matchUn,i,j,0);

    for(i=0;i<nseqs;i++)
	for(j=i+1;j<nseqs;j++)
	{
	    m = ajFloat2dGet(match,i,j); /* no. matches */
	    g = ajFloat2dGet(gap,i,j);	/* no. gaps    */

	    D = 1 - (m/((float)len-g+(g*gapwt)));

	    ajFloat2dPut(&matchUn,i,j,D);
	}

    return matchUn;
}




/* @funcstatic distmat_TajimaNei **********************************************
**
** Tajima-Nei correction used for multiple substitutions in the calc
** of the distance matrix. Nucleic acids only.
**
**  D = p-distance = 1 - (matches/(posns_scored + gaps)
**
**  distance = -b * ln(1-D/b)
**
** Tajima and Nei, Mol. Biol. Evol. 1984, 1, 269.
**
** @param [r] seqcharptr [char* const *] Array of sequences as C strings
** @param [r] match [const AjPFloat2d] Matches
** @param [r] mlen [ajint] Length
** @param [r] nseqs [ajint] Number of sequences
** @return [AjPFloat2d] corrected distance matrix
**
******************************************************************************/

static AjPFloat2d distmat_TajimaNei(char* const * seqcharptr,
				    const AjPFloat2d match,
				    ajint mlen, ajint nseqs)
{

    ajint i;
    ajint j;
    ajint l;
    ajint ti;
    ajint tj;

    ajint bs;
    ajint bs1;
    ajint pair;
    ajint val;
    ajint ival;
    
    AjPInt2d len   = NULL;
    AjPInt3d pfreq = NULL;
    AjPInt3d cbase = NULL;

    float fi;
    float fj;
    float fij;
    float fij2;
    float ci1;
    float ci2;
    float cj1;
    float cj2;
    float slen;
    float h;
    float m;
    float D;
    float b;

    float X1;

    AjPFloat2d matchTN = NULL;


    len   = ajInt2dNew();		/* scored length for pairs */
    pfreq = ajInt3dNew();		/* pair freq between seq's */
    cbase = ajInt3dNew();		/* no. of bases */
    matchTN = ajFloat2dNew();



    for(i=0;i<nseqs;++i)
	for(j=0;j<nseqs;++j)
	{
	    ajFloat2dPut(&matchTN,i,j,0);
	    ajInt2dPut(&len,i,j,0);
	    for(pair=0;pair<6;pair++)
		ajInt3dPut(&pfreq,i,j,pair,0);

	    for(bs=0;bs<4;bs++)
		ajInt3dPut(&cbase,i,j,bs,0);
	}

    /* calc content of each seq  - ignoring gaps */
    for(i=0;i<nseqs;i++)
	for(j=0;j<nseqs;j++)
	    for(l=0; l< mlen; l++)
	    {
		ti = toupper((int) seqcharptr[i][l]);

		if(!strchr("-NXWMKBVDH",ti) )
		{
		    tj = toupper((int) seqcharptr[j][l]);

		    if(!strchr("-NXWMKBVDH",tj))
		    {
                        ival = ajInt2dGet(len,i,j)+1;
			slen = (float) ival;
			ajInt2dPut(&len,i,j,(ajint)slen);
			if(strchr("G",ti))
			{
			    val = ajInt3dGet(cbase,i,j,3)+1;
			    ajInt3dPut(&cbase,i,j,3,val);
			}
			else if(strchr("C",ti))
			{
			    val = ajInt3dGet(cbase,i,j,2)+1;
			    ajInt3dPut(&cbase,i,j,2,val);
			}
			else if(strchr("T",ti))
			{
			    val = ajInt3dGet(cbase,i,j,1)+1;
			    ajInt3dPut(&cbase,i,j,1,val);
			}
			else if(strchr("A",ti))
			{
			    val = ajInt3dGet(cbase,i,j,0)+1;
			    ajInt3dPut(&cbase,i,j,0,val);
			}
		    }

		    if(ti != tj)
		    {

			if((strchr("A",ti) && strchr("T",tj)) ||
			   (strchr("T",ti) && strchr("A",tj)) )
			{		/* AT pair */
			    val = ajInt3dGet(pfreq,i,j,0)+1;
			    ajInt3dPut(&pfreq,i,j,0,val);
			}
			else if((strchr("A",ti) && strchr("C",tj)) ||
				(strchr("C",ti) && strchr("A",tj)) )
			{		/* AC pair */
			    val = ajInt3dGet(pfreq,i,j,1)+1;
			    ajInt3dPut(&pfreq,i,j,1,val);
			}
			else if((strchr("A",ti) && strchr("G",tj)) ||
				(strchr("G",ti) && strchr("A",tj)) )
			{		/* AG pair */
			    val = ajInt3dGet(pfreq,i,j,2)+1;
			    ajInt3dPut(&pfreq,i,j,2,val);
			}
			else if((strchr("T",ti) && strchr("C",tj)) ||
				(strchr("C",ti) && strchr("T",tj)) )
			{		/* TC pair */
			    val = ajInt3dGet(pfreq,i,j,3)+1;
			    ajInt3dPut(&pfreq,i,j,3,val);
			}
			else if((strchr("T",ti) && strchr("G",tj)) ||
				(strchr("G",ti) && strchr("T",tj)) )
			{		/* TG pair */
			    val = ajInt3dGet(pfreq,i,j,4)+1;
			    ajInt3dPut(&pfreq,i,j,4,val);
			}
			else if((strchr("C",ti) && strchr("G",tj)) ||
				(strchr("G",ti) && strchr("C",tj)) )
			{		/* CG pair */
			    val = ajInt3dGet(pfreq,i,j,5)+1;
			    ajInt3dPut(&pfreq,i,j,5,val);
			}
		    }
		}
	    }
    

    /* calc distance matrix */
    for(i=0;i<nseqs;i++)
	for(j=i+1;j<nseqs;j++)
	{
            ival = ajInt2dGet(len,i,j);
	    slen = (float) ival;

	    fij2 = 0.;
	    for(bs=0;bs<4;bs++)
	    {
                ival = ajInt3dGet(cbase,i,j,bs);
		fi  = (float) ival;
                ival = ajInt3dGet(cbase,j,i,bs);
		fj  = (float) ival;
		fij = 0.;
		if(fi != 0. && fj != 0.)
		    fij = (fi+fj)/((float)2.*slen);
		fij2 += fij*fij;
	    }

	    pair = 0;
	    h = 0.;
	    for(bs=0;bs<3;bs++)
	    {
		for(bs1=bs+1;bs1<4;bs1++)
		{
                    ival = ajInt3dGet(pfreq,i,j,pair);
		    X1 = (float) ival;
		    fij = X1/slen;
                    ival = ajInt3dGet(cbase,j,i,bs);
		    ci1 = (float) ival;
                    ival = ajInt3dGet(cbase,i,j,bs);
		    cj1 = (float) ival;
                    ival = ajInt3dGet(cbase,j,i,bs1);
		    ci2 = (float) ival;
                    ival = ajInt3dGet(cbase,i,j,bs1);
		    cj2 = (float) ival;

		    if(fij !=0.)
			h += ((float)0.5*fij*fij)/((ci1+cj1)/((float)2.*slen) *
					    (ci2+cj2)/((float)2.*slen));

		    pair++;
		}
	    }

	    m = ajFloat2dGet(match,i,j); /* no. matches */
	    D = (float)1. - m/slen;
	    b = (float)0.5*(1-fij2+((D*D)/h));

	    ajFloat2dPut(&matchTN,i,j, (float) (-b*log(1.-(D/b))) );
	}


    ajInt2dDel(&len);
    ajInt3dDel(&cbase);
    ajInt3dDel(&pfreq);

    return matchTN;
}




/* @funcstatic distmat_JinNei *************************************************
**
**  Nucleic acids only.
**
**  Jin and Nei, Mol. Biol. Evol. 82, 7, 1990.
**
** @param [r] seqcharptr [char* const *] Array of sequences as C strings
** @param [r] mlen [ajint] Length
** @param [r] nseqs [ajint] Number of sequences
** @param [r] calc_a [AjBool] Calculation
** @param [r] var_a [float] Variable
** @return [AjPFloat2d] corrected distance matrix
******************************************************************************/

static AjPFloat2d distmat_JinNei(char* const * seqcharptr,
				 ajint mlen, ajint nseqs,
				 AjBool calc_a, float var_a)
{
    ajint i;
    ajint j;
    ajint k;

    ajint t1;
    ajint t2;
    ajint trans;
    ajint tranv;
    ajint slen = 0;

    float xlen = 0.0;
    float av;
    float var;
    float dist;
    float P;
    float Q;

    float X1;
    ajint ti;
    
    AjPFloat2d matDist = NULL;
    AjPFloat2d cval    = NULL;
    AjPFloat2d avL     = NULL;

    AjPInt2d Ptrans = NULL;
    AjPInt2d Qtranv = NULL;
    AjPInt2d len    = NULL;


    matDist = ajFloat2dNew();
    cval    = ajFloat2dNew();
    avL     = ajFloat2dNew();
    Ptrans  = ajInt2dNew();
    Qtranv  = ajInt2dNew();
    len     = ajInt2dNew();

    /* initialise array */
    for(i=0;i<nseqs;++i)
	for(j=i+1;j<nseqs;++j)
	{
	    ajFloat2dPut(&matDist,i,j,0.);
	    ajInt2dPut(&Ptrans,i,j,0);
	    ajInt2dPut(&Qtranv,i,j,0);
	    ajInt2dPut(&len,i,j,0);
	    ajFloat2dPut(&avL,i,j,0);
	}


    /* calc transition & transversion subst.'s */
    for(i=0;i<nseqs;i++)
	for(j=i+1;j<nseqs;j++)
	{
	    av = ajFloat2dGet(avL,i,j);
	    for(k=0; k< mlen; k++)
	    {
		t1 = toupper((int) seqcharptr[i][k]);
		t2 = toupper((int) seqcharptr[j][k]);
		if(!strchr("-",t1) && !strchr("-",t2))
		{
		    slen = ajInt2dGet(len,i,j)+1;
		    ajInt2dPut(&len,i,j,slen);

		    trans = 0;
		    tranv = 0;
		    distmat_checkSubs(t1,t2,&trans,&tranv);
		    distmat_checkRY(t1,t2,&trans,&tranv);
		    av+= ((float)trans + ((float)2.*(float)tranv));

		    trans+= ajInt2dGet(Ptrans,i,j);
		    tranv+= ajInt2dGet(Qtranv,i,j);

		    ajInt2dPut(&Ptrans,i,j,trans);
		    ajInt2dPut(&Qtranv,i,j,tranv);
		}
	    }
	    xlen = (float) slen;
	    ajFloat2dPut(&avL,i,j,av/xlen);
	}


    if(calc_a)		/* calc inverse of coeff of variance */
	for(i=0;i<nseqs;i++)
	    for(j=i+1;j<nseqs;j++)
	    {
		ti = ajInt2dGet(len,i,j);
		xlen = (float) ti;

/* This makes no sense - av gets overwritten */
/*
		av = (float)( ajInt2dGet(Ptrans,i,j)+
			     (2*ajInt2dGet(Qtranv,i,j)) )/xlen;
*/

		av = ajFloat2dGet(avL,i,j);
		var = 0.;

		for(k=0; k< mlen; k++)
		{
		    t1 = toupper((int) seqcharptr[i][k]);
		    t2 = toupper((int) seqcharptr[j][k]);
		    if(!strchr("-",t1) && !strchr("-",t2))
		    {
			trans = 0;
			tranv = 0;
			distmat_checkSubs(t1,t2,&trans,&tranv);
			distmat_checkRY(t1,t2,&trans,&tranv);

			var+= (av-(float)(trans+(2*tranv)))*
			    (av-(float)(trans+(2*tranv)));
		    }
		}
		var = var/xlen;
		ajFloat2dPut(&cval,i,j,(av*av)/var);
	    }


    /* calc the dist matrix */
    for(i=0;i<nseqs;i++)
	for(j=i+1;j<nseqs;j++)
	{
	    ti = ajInt2dGet(len,i,j);
	    xlen = (float) ti;
	    
	    ti = ajInt2dGet(Ptrans,i,j);
	    X1 = (float) ti;
	    
	    P = X1/xlen;

	    ti = ajInt2dGet(Qtranv,i,j);
	    X1 = (float) ti;
	    
	    Q = X1/xlen;

	    if(calc_a)
		var = ajFloat2dGet(cval,i,j);
	    else
		var = var_a;

	    dist = (float) (0.5*var*
                            (pow(1.-(2*P)-Q,-1./var) +
                             (0.5*pow(1.-(2*Q),-1./var)) - 1.5 ));

	    ajFloat2dPut(&matDist,i,j,dist);
	}

    ajFloat2dDel(&cval);
    ajFloat2dDel(&avL);
    ajInt2dDel(&Ptrans);
    ajInt2dDel(&Qtranv);
    ajInt2dDel(&len);

    return matDist;
}




/* @funcstatic distmat_JukesCantor ********************************************
**
** Use the Jukes-Cantor method to correct for multiple substitutions in
** the calculation of the distance matrix.
**
**        D = p-distance = 1 - (matches/(posns_scored + gaps*gap_penalty))
**
** distance = -b * ln(1-D/b)        b = 3/4    nucleic acid
**                                    = 19/20  protein
**
** "Phylogenetic Inference", Swoffrod, Olsen, Waddell and Hillis,
** in Mol. Systematics, 2nd ed, 1996, Ch 11. Derived from "Evolution
** of Protein Molecules", Jukes & Cantor, in Mammalian Prot. Metab.,
** III, 1969, pp. 21-132.
**
** @param [r] match [const AjPFloat2d] Matches
** @param [r] gap [const AjPFloat2d] Gaps
** @param [r] gapwt [float] Gap weight
** @param [r] mlen [ajint] Length
** @param [r] nseqs [ajint] Number of sequences
** @param [r] nuc [AjBool] Nucleotide
** @return [AjPFloat2d] corrected distance matrix
**
******************************************************************************/

static AjPFloat2d distmat_JukesCantor(const AjPFloat2d match,
				      const AjPFloat2d gap,
				      float gapwt, ajint mlen, ajint nseqs,
				      AjBool nuc)
{
    ajint i;
    ajint j;

    float m;
    float g;
    float b;
    float D;
    float val;
    
    AjPFloat2d matchJC = NULL;


    b = (float)(19./20.);
    if(nuc)
	b = (float) (3./4.);

    matchJC = ajFloat2dNew();
    for(j=0;j<nseqs;++j)
        for(i=0;i<nseqs;++i)
            ajFloat2dPut(&matchJC,j,i,0);


    for(j=0;j<nseqs;j++)
	for(i=j;i<nseqs;i++)
	{
	    m = ajFloat2dGet(match,j,i); /* no. matches */
	    g = ajFloat2dGet(gap,j,i);	/* no. gaps    */

	    D = 1 - (m/((float)mlen-g+(g*gapwt)));

            val = (float) log((double)1.0 - (double)(D/b));
            val *= -b;

            ajFloat2dPut(&matchJC,j,i,val);
	}


    return matchJC;
}




/* @funcstatic distmat_checkRY ************************************************
**
** Check substitutions (not found by checkSubs) involving abiguity codes
** R (A or G) & Y (C or T) for transitions & transversions.
**
** @param [r] t1 [ajint] Transition score
** @param [r] t2 [ajint] Transversion score
** @param [w] trans [ajint*] Transitions
** @param [w] tranv [ajint*] Transversions
** @return [void]
**
******************************************************************************/

static void distmat_checkRY(ajint t1, ajint t2, ajint* trans, ajint* tranv)
{

    if(strchr("R",t1))
    {
	if(strchr("AGR",t2))		/* transitions */
	    ++*trans;
	else if(strchr("CTUY",t2))	/* transversion */
	    ++*tranv;
    }
    else if(strchr("AG",t1))
    {
	if(strchr("R",t2))		/* transitions */
	    ++*trans;
	else if(strchr("Y",t2))		/* transversion */
	    ++*tranv;
    }
    else if(strchr("Y",t1))
    {
	if(strchr("CTUY",t2))		/* transitions */
	    ++*trans;
	else if(strchr("AGR",t2))	/* transversion */
	    ++*tranv;
    }
    else if(strchr("CTU",t1))
    {
	if(strchr("Y",t2))		/* transitions */
	    ++*trans;
	else if(strchr("R",t2))		/* transversion */
	    ++*tranv;
    }

    return;
}




/* @funcstatic distmat_checkSubs **********************************************
**
** Check substitutions for transitions & transversions (ignores
** ambiguity codes).
**
** @param [r] t1 [ajint] Transition score
** @param [r] t2 [ajint] Transversion score
** @param [w] trans [ajint*] Transitions
** @param [w] tranv [ajint*] Transversions
** @return [void]
**
******************************************************************************/

static void distmat_checkSubs(ajint t1, ajint t2, ajint* trans, ajint* tranv)
{
    if(strchr("A",t1))
    {
	if(strchr("G",t2))		/* transitions */
	    ++*trans;
	else if(strchr("CTU",t2))	/* transversion */
	    ++*tranv;
    }
    else if(strchr("G",t1))
    {
	if(strchr("A",t2))		/* transitions */
	    ++*trans;
	else if(strchr("CTU",t2))	/* transversion */
	    ++*tranv;
    }
    else if(strchr("C",t1))
    {
	if(strchr("TU",t2))		/* transitions */
	    ++*trans;
	else if(strchr("AG",t2))	/* transversion */
	    ++*tranv;
    }
    else if(strchr("T",t1))
    {
	if(strchr("CU",t2))		/* transitions */
	    ++*trans;
	else if(strchr("AG",t2))	/* transversion */
	    ++*tranv;
    }
    else if(strchr("U",t1))
    {
	if(strchr("TC",t2))		/* transitions */
	    ++*trans;
	else if(strchr("AG",t2))	/* transversion */
	    ++*tranv;
    }

    return;
}




/* @funcstatic distmat_checkambigProt *****************************************
**
** Check amino acid ambiguity codes  to estimate the distance score.
**
** @param [r] t1 [ajint] Transition score
** @param [r] t2 [ajint] Transversion score
** @return [float] Estimated distance score
**
******************************************************************************/

static float distmat_checkambigProt(ajint t1, ajint t2)
{
    float n;

    n = (float)0.;

    if( !strchr("X",t1) && t1 == t2 )
	n = (float)1.0;
    else if(((strchr("B",t1) && strchr("DN",t2)) ||
              (strchr("B",t2) && strchr("DN",t1))) )
	n = (float)0.5;
    else if(((strchr("Z",t1) && strchr("EQ",t2)) ||
              (strchr("Z",t2) && strchr("EQ",t1))) )
	n = (float)0.5;
    else if(((strchr("J",t1) && strchr("IL",t2)) ||
              (strchr("J",t2) && strchr("IL",t1))) )
	n = (float)0.5;
    else if( strchr("X",t1) && strchr("X",t2) )
	n = (float)0.0025;
    else if( strchr("X",t1) || strchr("X",t2) )
	n = (float)0.05;

    return n;
}




/* @funcstatic distmat_getSeq *************************************************
**
** Get the part of the sequences that the distances are calculated from.
** i.e. codon positions 1, 2, 3 or 1 & 2.
**
** @param [r] seqset [const AjPSeqset] Sequence set object
** @param [r] nseqs [ajint] Number of sequences
** @param [r] mlen [ajint] Length
** @param [r] incr [ajint] Increment
** @param [r] posn [ajint] Position
** @param [w] len [ajint*] length of longest sequence
** @return [char**] Sequences as an array of C strings
**
******************************************************************************/

static char** distmat_getSeq(const AjPSeqset seqset, ajint nseqs, ajint mlen,
			     ajint incr, ajint posn, ajint* len)
{
    ajint i;
    ajint j;
    ajint count;

    AjBool onetwo = ajFalse;
    const char*  pseqset;
    char** pseq;


    /* positions 1 & 2 used to score distances */
    if(posn == 12)
    {
	onetwo = ajTrue;
	posn   = 0;
	incr   = 3;
    }

    *len = 0;
    for(j=posn;j<mlen;j+=incr)
    {
	*len+=1;
	if(onetwo) *len+=1;
    }

    AJCNEW(pseq,nseqs);
    for(i=0;i<nseqs;i++)		/* get seq as char* */
    {
	pseqset =  ajSeqsetGetseqSeqC(seqset,i);
	pseq[i] = ajCharNewRes(*len);

	count = 0;
	for(j=posn;j<mlen;j+=incr)
	{
	    strncpy(pseq[i]+count,pseqset+j,1);
	    count++;
	    if(onetwo)
	    {
		strncpy(pseq[i]+count,pseqset+j+1,1);
		count++;
	    }
	}
	ajDebug("SEQ %d: %s",i,pseq[i]);
    }

    return pseq;
}




/* @funcstatic distmat_outputDist *********************************************
**
** Output the distance matrix
**
** @param [u] outf [AjPFile] Output file
** @param [r] nseqs [ajint] Number of sequences
** @param [r] seqset [const AjPSeqset] Sequence set object
** @param [r] match [const AjPFloat2d] Matches
** @param [r] gapwt [float] Gap weight
** @param [r] method [ajint] Method
** @param [r] ambig [AjBool] Ambiguities
** @param [r] nuc [AjBool] Nucleotide
** @param [r] posn [ajint] Position
** @param [r] incr [ajint] Incvrement
** @return [void]
**
******************************************************************************/

static void distmat_outputDist(AjPFile outf, ajint nseqs,
			       const AjPSeqset seqset,
			       const AjPFloat2d match,
			       float gapwt, ajint method,
			       AjBool ambig, AjBool nuc, ajint posn,
			       ajint incr)
{
    ajint i;
    ajint j;
    float D;

    if(posn == 0)
    {
	if(incr ==3)
	    posn = posn+1;
	else
	    posn = 123;
    }
    else if(posn == 1 || posn == 2)
      	posn++;

    /* print title and parameters */
    ajFmtPrintF(outf,"Distance Matrix\n---------------\n\n");
    if(method == 0)
	ajFmtPrintF(outf,"Uncorrected for Multiple Substitutions\n");
    else if(method == 1)
	ajFmtPrintF(outf,"Using the Jukes-Cantor correction method\n");
    else if(method == 2)
	ajFmtPrintF(outf,"Using the Kimura correction method\n");
    else if(method == 3)
	ajFmtPrintF(outf,"Using the Tamura correction method\n");
    else if(method == 4)
	ajFmtPrintF(outf,"Using the Tajima-Nei correction method\n");
    else if(method == 5)
	ajFmtPrintF(outf,"Using the Jin-Nei correction method\n");

    if(ambig)
	ajFmtPrintF(outf,"Using ambiguity codes\n");
    if(nuc)
	ajFmtPrintF(outf,"Using base positions %d in the codon\n",posn);
    ajFmtPrintF(outf,"Gap weighting is %f\n\n",gapwt);

    /* print matrix */

    for(j=0;j<nseqs;j++)
	ajFmtPrintF(outf,"\t    %d",j+1);
    ajFmtPrintF(outf,"\n");

    /* Output distance matrix */
    for(j=0;j<nseqs;j++)
    {
	ajFmtPrintF(outf,"\t");
	for(i=0;i<nseqs;i++)
	    if(i >= j)
	    {
		if(i==j)
		    D = 0.;
		else
		    D=ajFloat2dGet(match,j,i);

		D=D*(float)100.;
		if(D < 10.)
		    ajFmtPrintF(outf,"  %.2f\t",D);
		else if(D < 100.)
		    ajFmtPrintF(outf," %.2f\t",D);
		else
		    ajFmtPrintF(outf,"%.2f\t",D);
	    }
	    else
		ajFmtPrintF(outf,"\t");

	ajFmtPrintF(outf,"\t%S %d",ajSeqsetGetseqNameS(seqset,j),j+1);
	ajFmtPrintF(outf,"\n");
    }

    return;
}
