/* @source coderet application
**
** Retrieves CDS, mRNA and translations from feature tables
**
** @author Copyright (C) Alan Bleasby (ableasby@hgmp.mrc.ac.uk)
** Last modified by Jon Ison Thu Jun 29 08:26:08 BST 2006
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




static void coderet_put_seq(const AjPSeq seq, const AjPStr strseq,
			    ajint n, const char *name,
			    ajint type, AjPSeqout seqout);

static void coderet_put_rest(const AjPSeq seq, const AjPStr copyseq,
			     const char* name, AjPSeqout seqout);



/* @prog coderet **************************************************************
**
** Extract CDS, mRNA and translations from feature tables
**
******************************************************************************/

int main(int argc, char **argv)
{
    AjPSeqall seqall      = NULL;
    AjPSeq seq            = NULL;
    AjPSeqout seqoutcds   = NULL;
    AjPSeqout seqoutmrna  = NULL;
    AjPSeqout seqoutprot  = NULL;
    AjPSeqout seqoutrest  = NULL;
    AjPFile logf     = NULL;

    ajint icds =0;
    ajint imrna =0;
    ajint irest =0;
    ajint itran =0;

    AjPStr cds  = NULL;
    AjPStr trnseq  = NULL;
    AjPStr mrna = NULL;
    AjPStr usa  = NULL;
    AjPStr copyseq  = NULL;

    AjBool ret = ajFalse;
    const AjPFeature gf = NULL;
    const AjPFeattable feat = NULL;
    AjIList iter = NULL;

    embInit("coderet",argc,argv);

    seqall  = ajAcdGetSeqall("seqall");

    seqoutcds  = ajAcdGetSeqoutall("cdsoutseq");
    seqoutmrna = ajAcdGetSeqoutall("mrnaoutseq");
    seqoutrest = ajAcdGetSeqoutall("restoutseq");
    seqoutprot = ajAcdGetSeqoutall("translationoutseq");
    logf = ajAcdGetOutfile("outfile");

    cds  = ajStrNew();
    mrna = ajStrNew();
    usa  = ajStrNew();


    /*
    **  Must get this so that embedded references in the same database
    **  can be resolved
    */
    ajStrAssignS(&usa,ajSeqallGetUsa(seqall));

    if(seqoutcds)
	ajFmtPrintF(logf, "   CDS");

    if(seqoutmrna)
	ajFmtPrintF(logf, "  mRNA");

    if(seqoutrest)
	ajFmtPrintF(logf, " non-c");

    if(seqoutprot)
	ajFmtPrintF(logf, " Trans");

    ajFmtPrintF(logf, " Total Sequence\n");
    if(seqoutcds)
	ajFmtPrintF(logf, " =====");

    if(seqoutmrna)
	ajFmtPrintF(logf, " =====");

    if(seqoutrest)
	ajFmtPrintF(logf, " =====");

    if(seqoutprot)
	ajFmtPrintF(logf, " =====");

    ajFmtPrintF(logf, " ===== ========\n");

    while(ajSeqallNext(seqall,&seq))
    {
        icds = 0;
        imrna = 0;
        itran = 0;
        irest = 0;
        feat = ajSeqGetFeat(seq);
        if(seqoutrest)
        {
            copyseq = ajSeqGetSeqCopyS(seq);
            ajStrFmtUpper(&copyseq);
        }
        
        iter = ajListIterNewread(feat->Features);
        while(!ajListIterDone(iter))
        {
            gf = ajListIterGet(iter);
            if(ajFeatIsChild(gf))
                continue;
            if(seqoutcds && ajFeatTypeMatchC(gf, "CDS"))
            {
                icds++;
                ret = ajFeatGetSeq(gf, feat, seq, &cds);
		if(!ret)
		{
		    ajWarn("Cannot extract %S\n",ajSeqGetNameS(seq));
		    continue;
		}
		coderet_put_seq(seq,cds,icds,"cds",0,seqoutcds);
	    }

            if(seqoutmrna && ajFeatTypeMatchC(gf, "mRNA"))
            {
                imrna++;
                ret = ajFeatGetSeq(gf, feat, seq, &mrna);
                if(!ret)
                {
                    ajWarn("Cannot extract %s",ajSeqGetNameC(seq));
                    continue;
                }
		coderet_put_seq(seq,mrna,imrna,"mrna",0,seqoutmrna);
	    }


            if(seqoutrest)          /* set feature to lowercase */
            {

                if(ajFeatTypeMatchC(gf, "CDS") ||
                   ajFeatTypeMatchC(gf, "mRNA") ||
                   ajFeatTypeMatchC(gf, "exon"))
                {
                    irest++;
                    ajFeatLocMark(gf, feat, &copyseq);
                }

            }

            if(seqoutprot && ajFeatTypeMatchC(gf, "CDS"))
            {
                if(ajFeatGetTranslation(gf, &trnseq))
                {
                    itran++;
                    coderet_put_seq(seq, trnseq, itran,"pro",1,seqoutprot);
                }
            }
        }
        ajListIterDel(&iter);
        
        if(seqoutrest)
            coderet_put_rest(seq,copyseq,"noncoding",seqoutrest);

        if(seqoutcds)
            ajFmtPrintF(logf, "%6d", icds);
	if(seqoutmrna)
            ajFmtPrintF(logf, "%6d", imrna);
	if(seqoutrest)
            ajFmtPrintF(logf, "%6d", irest);
	if(seqoutprot)
            ajFmtPrintF(logf, "%6d", itran);

        ajFmtPrintF(logf, "%6d %s\n",
                    icds+imrna+irest+itran, ajSeqGetNameC(seq));
    }


    if(seqoutcds)
	ajSeqoutClose(seqoutcds);
    if(seqoutmrna)
	ajSeqoutClose(seqoutmrna);
    if(seqoutrest)
	ajSeqoutClose(seqoutrest);
    if(seqoutprot)
	ajSeqoutClose(seqoutprot);


    ajStrDel(&cds);
    ajStrDel(&mrna);
    ajStrDel(&usa);
    ajStrDel(&copyseq);
    ajSeqallDel(&seqall);
    ajSeqDel(&seq);
    ajStrDel(&trnseq);
    ajSeqoutDel(&seqoutcds);
    ajSeqoutDel(&seqoutmrna);
    ajSeqoutDel(&seqoutprot);
    ajSeqoutDel(&seqoutrest);
    ajFileClose(&logf);

    embExit();

    return 0;
}




/* @funcstatic coderet_put_seq ************************************************
**
** Undocumented.
**
** @param [r] seq [const AjPSeq] Undocumented
** @param [r] strseq [const AjPStr] Undocumented
** @param [r] n [ajint] Undocumented
** @param [r] name [const char*] Undocumented
** @param [r] type [ajint] Undocumented
** @param [u] seqout [AjPSeqout] Undocumented
** @@
******************************************************************************/

static void coderet_put_seq(const AjPSeq seq, const AjPStr strseq,
			    ajint n, const char *name,
			    ajint type, AjPSeqout seqout)
{
    AjPSeq nseq = NULL;
    AjPStr fn   = NULL;

    fn = ajStrNew();


    ajFmtPrintS(&fn,"%S_%s_%d",ajSeqGetAccS(seq),name,n);
    ajStrFmtLower(&fn);

    nseq = ajSeqNewRes(ajStrGetLen(strseq));
    ajSeqAssignNameS(nseq, fn);
    ajSeqAssignEntryS(nseq, fn);

    if(!type)
	ajSeqSetNuc(nseq);
    else
	ajSeqSetProt(nseq);

    ajSeqAssignSeqS(nseq,strseq);


    ajSeqoutWriteSeq(seqout,nseq);


    ajSeqDel(&nseq);
    ajStrDel(&fn);

    return;
}




/* @funcstatic coderet_put_rest ***********************************************
**
** Undocumented.
**
** @param [r] seq [const AjPSeq] Undocumented
** @param [r] copyseq [const AjPStr] Undocumented
** @param [r] name [const char*] Undocumented
** @param [u] seqout [AjPSeqout] Undocumented
** @@
******************************************************************************/

static void coderet_put_rest(const AjPSeq seq, const AjPStr copyseq,
			     const char* name, AjPSeqout seqout)
{
    AjPSeq nseq = NULL;
    const AjPStr strseq = NULL;
    AjPStr subseq = NULL;
    AjPStr fn   = NULL;

    const char* cp = ajStrGetPtr(copyseq);
    ajint i = 0;
    ajint j = 0;
    AjBool isup;

    fn = ajStrNew();
    isup = ajFalse;
    strseq = ajSeqGetSeqS(seq);

    while(*cp)
    {
	if(islower(*cp))
	{
	    if(isup)
	    {
		ajFmtPrintS(&fn,"%S_%s_%d",ajSeqGetAccS(seq),name,i+1);
		ajStrFmtLower(&fn);
		nseq = ajSeqNewRes(j-i);
		ajSeqAssignNameS(nseq, fn);
		ajSeqAssignEntryS(nseq, fn);
		ajSeqSetNuc(nseq);
		ajStrAssignSubS(&subseq, strseq, i, j-1);
		ajSeqAssignSeqS(nseq,subseq);
		ajSeqoutWriteSeq(seqout,nseq);
		ajSeqDel(&nseq);
		isup = ajFalse;
	    }
	}
	else
	{
	    if(!isup)
	    {
		i = j;
		isup = ajTrue;
	    }
	}
	cp++;
	j++;
    }

    if(isup)
    {
	ajFmtPrintS(&fn,"%S_%s_%d",ajSeqGetAccS(seq),name,i+1);
	ajStrFmtLower(&fn);

	nseq = ajSeqNewRes(j-i);
	ajSeqAssignNameS(nseq, fn);
	ajSeqAssignEntryS(nseq, fn);

	ajSeqSetNuc(nseq);

	ajStrAssignSubS(&subseq, strseq, i, j);
	ajSeqAssignSeqS(nseq,subseq);


	ajSeqoutWriteSeq(seqout,nseq);

	ajSeqDel(&nseq);
    }

    ajStrDel(&fn);
    ajStrDel(&subseq);

    return;
}
