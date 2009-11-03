/* Copyright (c) 2002 Robert B. Russell 
**  EMBL, Meyerhofstrasse 1, 69917 Heidelberg, Germany 
**    Email: russell@embl.de
**
**  This program is free software; you can redistribute it and/or modify
**  it under the terms of the GNU General Public License as published by
**  the Free Software Foundation; either version 2 of the License, or
**  (at your option) any later version.
**
**  This program is distributed in the hope that it will be useful,
**  but WITHOUT ANY WARRANTY; without even the implied warranty of
**  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
**  GNU General Public License for more details.
**
**  You should have received a copy of the GNU General Public License
**  along with this program; if not, write to the Free Software
**  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA.
**
** Modified for EMBOSS by Rodrigo Lopez <rls@ebi.ac.uk>
** Remodified for EMBOSS by ajb
**
*/


#include "emboss.h"
#include <math.h>


#define NCAAs "A_CDEFGHI_KLMN_PQRST_VW_Y_"
#define NCHEPTAD 7

struct hept_pref
{
    float **m;		     /* 20 x 7 amino acid heptad weights */
    float smallest;	     /* Smallest of the above */
    ajint n;		     /* statistical fitting data (weighted) */
    struct fit_dat *f;
};

struct fit_dat
{
    ajint win;				/* Window length */
    float m_cc, sd_cc;			/* mean/sd for coiled-coils */
    float m_g,  sd_g;			/* mean/sd for globular */
    float sc;				/* scaling factor */
    ajint w;			        /* 1= weighted, 0=un-weighted */
};


static struct hept_pref *newcoils_read_matrix(AjPFile inf);



static void newcoils_pred_coils(AjPFile outf, const char *seq,
		       const char *ident, const AjPStr str,
		       const struct hept_pref *h, ajint win, ajint which,
		       ajint weighted, ajint fasta, float min_P, ajint *t,
		       ajint *tc, ajint min_segs);




/* @prog newcoils *************************************************************
**
** Predicts coils protein secondary structure
**
******************************************************************************/

int main(ajint argc, char **argv)
{

    AjPFile datafile;
    AjPFile outf = NULL;
    AjPSeqall seqall;
    AjPSeq ajseq = NULL;
    ajint i;
    ajint verb;
    ajint window;
    ajint pt;
    ajint which;
    ajint weighted;
    ajint t  = 0;
    ajint tc = 0;
    ajint mode;
    ajint min_seg;
    const AjPStr seqdes;

    float min_P;

    struct hept_pref *h;


    embInit("newcoils",argc,argv);

    window   = ajAcdGetInt("window");
    weighted = ajAcdGetInt("weighted");
    verb     = ajAcdGetInt("verb");
    mode     = ajAcdGetInt("mode");
    min_P    = ajAcdGetFloat("minp");
    min_seg  = ajAcdGetInt("minseg");
    outf     = ajAcdGetOutfile("outfile");
    datafile = ajAcdGetDatafile("datafile");
    seqall   = ajAcdGetSeqall("sequence");

    ajseq = ajSeqNew();

    h = newcoils_read_matrix(datafile);

    if(verb)
    {
	for(i=0; i<strlen(NCAAs); ++i)
	    if(NCAAs[i] != '_')
	    {
		pt = (int)(NCAAs[i]-'A');
		ajFmtPrintF(outf,"AA %c %4.2f %4.2f %4.2f %4.2f %4.2f %4.2f "
			    "%4.2f\n",NCAAs[i],h->m[pt][0],h->m[pt][1],
			    h->m[pt][2],h->m[pt][3],h->m[pt][4],
			    h->m[pt][5],h->m[pt][6]);
	    }

	for(i=0; i<h->n; ++i)
	    ajFmtPrintF(outf,"Window %4d %1d %f %f %f %f %f\n",h->f[i].win,
			h->f[i].w,h->f[i].m_cc,h->f[i].sd_cc,h->f[i].m_g,
			h->f[i].sd_g,h->f[i].sc);
    }

    /* See if there is a file for the chosen window length/weight scheme */
    which = -1;
    for(i=0; i<h->n; ++i)
    {
	if((h->f[i].win == window) && (h->f[i].w == weighted))
	{				/* match */
	    if(verb)
		ajFmtPrintF(outf,"Found fitting data for win %4d w %d\n",
			    window,weighted);
	    which = i;
	}
    }

    while(ajSeqallNext(seqall, &ajseq))
    {

	seqdes = ajSeqGetDescS(ajseq);
	newcoils_pred_coils(outf,ajSeqGetSeqC(ajseq),ajSeqGetNameC(ajseq),
			    seqdes,h,window,
			    which,weighted,mode,min_P,&t,&tc,min_seg); 

    }

    if (outf)
	ajFileClose(&outf);

    ajSeqDel(&ajseq);

    embExit();
    return 0;
}




/* @funcstatic newcoils_pred_coils ********************************************
**
** Undocumented
**
** @param [u] outf [AjPFile] Undocumented
** @param [r] seq [const char*] Undocumented
** @param [r] ident [const char*] Undocumented
** @param [r] str [const AjPStr] Undocumented
** @param [r] h [const struct hept_pref *] Undocumented
** @param [r] win [ajint] Undocumented
** @param [r] which [ajint] Undocumented
** @param [r] weighted [ajint] Undocumented
** @param [r] mode [ajint] Undocumented
** @param [r] min_P [float] Undocumented
** @param [w] t [ajint*] Undocumented
** @param [w] tc [ajint*] Undocumented
** @param [r] min_seg [ajint] Undocumented
** @return [void]
******************************************************************************/
static void newcoils_pred_coils(AjPFile outf, const char *seq,
				const char *ident, const AjPStr str,
				const struct hept_pref *h,
				ajint win, ajint which,
				ajint weighted,ajint mode,
				float min_P, ajint *t,
				ajint *tc, ajint min_seg)
{
    ajint i;
    ajint j;
    ajint coillen = 0;
    ajint len;
    ajint pos;
    ajint aa_pt;
    ajint total_coil_segments;
    ajint are_there_coils;

    float actual_win;
    float this_score;
    float Gg;
    float Gcc;
    float power;
    float t1;
    float t2;
    float t3;
    float t4;
    float *score;
    float *P;

    char *hept_seq;
	
    len = strlen(seq);

    AJCNEW(score,len);
    AJCNEW(P,len);
    AJCNEW(hept_seq,len);


    /*	printf("Sequence is %s length is %d\n",seq,len); */
    for(i=0; i<len; ++i)
    {
	P[i] = 0.0;
	score[i] = 0.0;
	hept_seq[i] = 'x';
    }

    for(i=0; i<(len-win+1); ++i)
    {
	this_score = 1.0;
	actual_win = 0.0;
	for(j=0; ((j<win) && ((i+j)<len)); ++j)
	{
	    aa_pt = (int)(seq[i+j]-'A');
	    if((aa_pt>=0) && (aa_pt<26) && (NCAAs[aa_pt]!='_'))
	    {
		pos = j%NCHEPTAD; /* Position in the heptad?  pos modulus 7 */
		if(weighted && (pos==0 || pos==3))
		    power = 2.5;
		else
		    power = 1.0;

		actual_win += power;
		if(h->m[aa_pt][pos]!=-1)
		    this_score *= pow(h->m[aa_pt][pos],power);
		else
		    this_score *= pow(h->smallest,power);
	    }
	}

	if(actual_win>0)
	    this_score = pow(this_score,(1/(float)actual_win));
	else
	    this_score = 0;

	for(j=0; ((j<win) && ((i+j)<len)); ++j)
	{
	    aa_pt = (int)(seq[i+j]-'A');
	    if((aa_pt>=0) && (aa_pt<26) && (NCAAs[aa_pt]!='_'))
	    {
		pos = j%NCHEPTAD; /* Position in the heptad?  pos modulus 7 */
		if(this_score>score[i+j])
		{
		    score[i+j]    = this_score;
		    hept_seq[i+j] = 'a'+pos;
		}
	    }
	}
    }


    if(mode==1)
	ajFmtPrintF(outf,">%s %S\n",ident,str);

    are_there_coils = 0;
    total_coil_segments = 0;
    for(i=0; i<len; ++i)
    {
	/* Calculate P */
	t1  = 1/(h->f[which].sd_cc);
	t2  = (score[i]-(h->f[which].m_cc))/h->f[which].sd_cc;
	t3  = fabs(t2);
	t4  = pow(t3,2);
	t4  = t3*t3;
	Gcc = t1 * exp(-0.5*t4);

	t1 = 1/(h->f[which].sd_g);
	t2 = (score[i]-(h->f[which].m_g))/h->f[which].sd_g;
	t3 = fabs(t2);
	t4 = pow(t3,2);
	t4 = t3 * t3;
	Gg = t1 * exp(-0.5*t4);

	P[i] = Gcc/(h->f[which].sc*Gg+Gcc);

	if(P[i] >= min_P)
	{
	    ++coillen;
	    are_there_coils = 1;
	    if((i==0) || (P[i-1]<min_P))
		total_coil_segments++;

	    (*tc)++; 
	}
	(*t)++;

	if(mode==1)
	{
	    if(P[i] >= min_P)
		ajFmtPrintF(outf,"x");
	    else
		ajFmtPrintF(outf,"%c",seq[i]);

	    if(((i+1)%60)==0)
		ajFmtPrintF(outf,"\n");
	}
	else if(mode == 0)
	    ajFmtPrintF(outf,"%4d %c %c %7.3f %7.3f (%-7.3f %7.3f)\n",i+1,
			seq[i],hept_seq[i],score[i],P[i],Gcc,Gg);
    }

    if(mode==1)
	ajFmtPrintF(outf,"\n");

    if((mode==2) && (are_there_coils==1) && (total_coil_segments >= min_seg))
    {
	if(total_coil_segments==1)
	    ajFmtPrintF(outf,"Pred %4d coil segment length:%d : %s %S\n",
			total_coil_segments,coillen,ident,str);
	else
	    ajFmtPrintF(outf,"Pred %4d coil segments Total length:%d: %s %S\n",
			total_coil_segments,coillen,ident,str);
    }

    AJFREE(P);
    AJFREE(score);
    AJFREE(hept_seq);

    return;
}




/* @funcstatic newcoils_read_matrix *******************************************
**
** Reads the matrix and stores in a hept_pref structure
**
** @param [u] inf [AjPFile] matrix input file
** @return [struct hept_pref*] Matrix data for heptad preference
******************************************************************************/
static struct hept_pref* newcoils_read_matrix(AjPFile inf)
{
    ajint i;
    ajint j;
    ajint pt;
    ajint aa_len;
    ajint win;

    float m_g;
    float sd_g;
    float m_cc;
    float sd_cc;
    float sc;
    float hept[NCHEPTAD];

    AjPStr buff;
    const char   *pbuff;
    
    struct hept_pref *h;


    buff = ajStrNew();

    aa_len = strlen(NCAAs);

    AJNEW(h);
    AJCNEW(h->m,aa_len);

    for(i=0; i<aa_len; ++i)
    {
	AJCNEW(h->m[i],NCHEPTAD);
	for(j=0; j<NCHEPTAD; ++j)
	    h->m[i][j] = -1;
    }

    AJNEW(h->f);

    h->n = 0;
    h->smallest = 1.0;

    while(ajFileReadLine(inf,&buff))
    {
	pbuff = ajStrGetPtr(buff);
	if(*pbuff != '%')
	{
	    if((strncmp(pbuff,"uw ",3)==0) || (strncmp(pbuff,"w ",2)==0))
	    {
		i = h->n;
		if(strncmp(pbuff,"uw ",3)==0)
		    h->f[i].w = 0;
		else
		    h->f[i].w = 1;

		ajFmtScanS(buff,"%*s %d %f %f %f %f %f",&win,&m_cc,
			   &sd_cc,&m_g,&sd_g,&sc);

		h->f[i].win   = win;
		h->f[i].m_cc  = (float)m_cc; 
		h->f[i].sd_cc = (float)sd_cc;
		h->f[i].m_g   = (float)m_g;
		h->f[i].sd_g  = (float)sd_g;
		h->f[i].sc    = (float)sc;
		h->n++;

		AJCRESIZE(h->f,(h->n)+1);
		
		if((h->n)>=9)
		    ajFatal("Too many window parms in matrix file\n");

	    }
	    else if(*pbuff>='A' && *pbuff<='Z')
	    {
		/* AA data */
		pt = (int)(pbuff[0]-'A');
		if(h->m[pt][0]==-1)
		{
		    ajFmtScanS(buff,"%*s%f%f%f%f%f%f%f",&hept[0],
			       &hept[1],&hept[2],&hept[3],&hept[4],
			       &hept[5],&hept[6]);

		    for(i=0; i<NCHEPTAD; ++i)
		    {
			h->m[pt][i] = (float)hept[i];
			if(h->m[pt][i]>0)
			{
			    if(h->m[pt][i]<h->smallest)
				h->smallest = h->m[pt][i];
			}
			else
			    h->m[pt][i]=-1; /* Don't permit zero values */
		    }

		}
		else
		    ajWarn("multiple entries for AA %c in matrix file\n",
			   *pbuff);
	    }
	    else
	    {
		ajWarn("strange characters in matrix file\n");
		ajWarn("Ignoring line: %S\n",buff);
	    }
	}
    }


    ajStrDel(&buff);

    return h;
}
