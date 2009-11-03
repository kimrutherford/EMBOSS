/* @source emma application
**
** EMBOSS interface to clustal
** @author Copyright (C) Mark Faller (mfaller@hgmp.mrc.ac.uk)
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

static AjPStr emma_getUniqueFileName(void);




/* @prog emma *****************************************************************
**
** Multiple alignment program - interface to ClustalW program
**
******************************************************************************/

int main(int argc, char **argv, char **env)
{

    AjPSeqall seqall = NULL;
    AjPFile dend_outfile = NULL;
    AjPStr tmp_dendfilename = NULL;
    AjPFile tmp_dendfile = NULL;

    AjPStr tmp_aln_outfile = NULL;
    AjPSeqset seqset = NULL;
    AjPSeqout seqout = NULL;
    AjPSeqin  seqin  = NULL;

    AjBool only_dend;
    AjBool are_prot = ajFalse;
    AjBool do_slow;
    AjBool use_dend;
    AjPFile dend_file = NULL;
    AjPStr dend_filename = NULL;

    ajint ktup;
    ajint gapw;
    ajint topdiags;
    ajint window;
    AjBool nopercent;

    AjPStr pw_matrix = NULL;
    AjPStr pw_dna_matrix  = NULL;
    AjPFile pairwise_matrix = NULL;
    float pw_gapc;
    float pw_gapv;

    AjPStr pwmstr = NULL;
    char   pwmc   = '\0';
    AjPStr pwdstr = NULL;
    char   pwdc   = '\0';

    AjPStr m1str = NULL;
    AjPStr m2str = NULL;
    char   m1c   = '\0';
    char   m2c   = '\0';

    AjPStr matrix = NULL;
    AjPStr dna_matrix = NULL;
    AjPFile ma_matrix = NULL;
    float gapc;
    float gapv;
    AjBool endgaps;
    AjBool norgap;
    AjBool nohgap;
    ajint gap_dist;
    ajint maxdiv;
    AjPStr hgapres = NULL;


    AjPSeqout fil_file = NULL;
    AjPSeq seq = NULL;

    const char* prog_default = "clustalw";
    AjPStr cmd = NULL;
    AjPStr tmp = NULL;
    AjPStr tmpFilename;
    AjPStr line = NULL;
    ajint nb = 0;


    /* get all the parameters */

    embInit("emma", argc, argv);

    pwmstr = ajStrNew();
    pwdstr = ajStrNew();
    m1str  = ajStrNew();
    m2str  = ajStrNew();


    seqall = ajAcdGetSeqall("sequence");
    seqout = ajAcdGetSeqoutset("outseq");

    dend_outfile = ajAcdGetOutfile("dendoutfile");

    only_dend = ajAcdGetToggle("onlydend");
    use_dend  = ajAcdGetToggle("dend");
    dend_file = ajAcdGetInfile("dendfile");
    if (dend_file)
	ajStrAssignS(&dend_filename, ajFileGetNameS(dend_file));
    ajFileClose(&dend_file);

    do_slow = ajAcdGetToggle("slow");

    ktup      = ajAcdGetInt("ktup");
    gapw      = ajAcdGetInt("gapw");
    topdiags  = ajAcdGetInt("topdiags");
    window    = ajAcdGetInt("window");
    nopercent = ajAcdGetBoolean("nopercent");

    pw_matrix = ajAcdGetListSingle("pwmatrix");
    pwmc = ajStrGetCharFirst(pw_matrix);

    if(pwmc=='b')
	ajStrAssignC(&pwmstr,"blosum");
    else if(pwmc=='p')
	ajStrAssignC(&pwmstr,"pam");
    else if(pwmc=='g')
	ajStrAssignC(&pwmstr,"gonnet");
    else if(pwmc=='i')
	ajStrAssignC(&pwmstr,"id");
    else if(pwmc=='o')
	ajStrAssignC(&pwmstr,"own");


    pw_dna_matrix = ajAcdGetListSingle("pwdnamatrix");
    pwdc = ajStrGetCharFirst(pw_dna_matrix);

    if(pwdc=='i')
	ajStrAssignC(&pwdstr,"iub");
    else if(pwdc=='c')
	ajStrAssignC(&pwdstr,"clustalw");
    else if(pwdc=='o')
	ajStrAssignC(&pwdstr,"own");

    pairwise_matrix = ajAcdGetInfile("pairwisedatafile");

    pw_gapc = ajAcdGetFloat( "pwgapopen");
    pw_gapv = ajAcdGetFloat( "pwgapextend");

    matrix = ajAcdGetListSingle( "matrix");
    m1c = ajStrGetCharFirst(matrix);

    if(m1c=='b')
	ajStrAssignC(&m1str,"blosum");
    else if(m1c=='p')
	ajStrAssignC(&m1str,"pam");
    else if(m1c=='g')
	ajStrAssignC(&m1str,"gonnet");
    else if(m1c=='i')
	ajStrAssignC(&m1str,"id");
    else if(m1c=='o')
	ajStrAssignC(&m1str,"own");


    dna_matrix = ajAcdGetListSingle( "dnamatrix");
    m2c = ajStrGetCharFirst(dna_matrix);

    if(m2c=='i')
	ajStrAssignC(&m2str,"iub");
    else if(m2c=='c')
	ajStrAssignC(&m2str,"clustalw");
    else if(m2c=='o')
	ajStrAssignC(&m2str,"own");


    ma_matrix = ajAcdGetInfile("mamatrixfile");
    gapc      = ajAcdGetFloat("gapopen");
    gapv      = ajAcdGetFloat("gapextend");
    endgaps   = ajAcdGetBoolean("endgaps");
    norgap    = ajAcdGetBoolean("norgap");
    nohgap    = ajAcdGetBoolean("nohgap");
    gap_dist  = ajAcdGetInt("gapdist");
    hgapres   = ajAcdGetString("hgapres");
    maxdiv    = ajAcdGetInt("maxdiv");

    tmp = ajStrNewC("fasta");

    /*
    ** Start by writing sequences into a unique temporary file
    ** get file pointer to unique file
    */


    fil_file = ajSeqoutNew();
    tmpFilename = emma_getUniqueFileName();
    if(!ajSeqoutOpenFilename( fil_file, tmpFilename))
	embExitBad();

    /* Set output format to fasta */
    ajSeqoutSetFormatS( fil_file, tmp);

    while(ajSeqallNext(seqall, &seq))
    {
        /*
        **  Check sequences are all of the same type
        **  Still to be done
        **  Write out sequences
        */
	if (!nb)
	    are_prot  = ajSeqIsProt(seq);
        ajSeqoutWriteSeq(fil_file, seq);
	++nb;
    }
    ajSeqoutClose(fil_file);

    if(nb < 2)
	ajFatal("Multiple alignments need at least two sequences");

    /* Generate clustalw command line */
    if(!ajNamGetValueC("clustalw", &cmd))
      cmd = ajStrNewC(prog_default);

    /* add tmp file containing sequences */
    ajStrAppendC(&cmd, " -infile=");
    ajStrAppendC(&cmd, ajStrGetPtr( tmpFilename));

    /* add out file name */
    tmp_aln_outfile = emma_getUniqueFileName();
    ajStrAppendC(&cmd, " -outfile=");
    ajStrAppendS(&cmd, tmp_aln_outfile);


    /* calculating just the nj tree or doing full alignment */
    if(only_dend)
        ajStrAppendC(&cmd, " -tree");
    else
        if(!use_dend)
	    ajStrAppendC(&cmd, " -align");

    /* Set sequence type from information from acd file */
    if(are_prot)
        ajStrAppendC(&cmd, " -type=protein");
    else
        ajStrAppendC(&cmd, " -type=dna");


    /*
    **  set output to MSF format - will read in this file later and output
    **  user requested format
    */
    ajStrAppendC(&cmd, " -output=");
    ajStrAppendC(&cmd, "gcg");

    /* If going to do pairwise alignment */
    if(!use_dend)
    {
        /* add fast pairwise alignments*/
        if(!do_slow)
        {
            ajStrAppendC(&cmd, " -quicktree");
            ajStrAppendC(&cmd, " -ktuple=");
            ajStrFromInt(&tmp, ktup);
            ajStrAppendS(&cmd, tmp);
            ajStrAppendC(&cmd, " -window=");
            ajStrFromInt(&tmp, window);
            ajStrAppendS(&cmd, tmp);
            if(nopercent)
                ajStrAppendC(&cmd, " -score=percent");
            else
                ajStrAppendC(&cmd, " -score=absolute");
            ajStrAppendC(&cmd, " -topdiags=");
            ajStrFromInt(&tmp, topdiags);
            ajStrAppendS(&cmd, tmp);
            ajStrAppendC(&cmd, " -pairgap=");
            ajStrFromInt(&tmp, gapw);
            ajStrAppendS(&cmd, tmp);
        }
        else
        {
            if(pairwise_matrix)
            {
		if(are_prot)
		    ajStrAppendC(&cmd, " -pwmatrix=");
		else
		    ajStrAppendC(&cmd, " -pwdnamatrix=");
		ajStrAppendS(&cmd, ajFileGetNameS(pairwise_matrix));
            }
            else
            {
		if(are_prot)
		{
		    ajStrAppendC(&cmd, " -pwmatrix=");
		    ajStrAppendS(&cmd, pwmstr);
		}
		else
		{
		    ajStrAppendC(&cmd, " -pwdnamatrix=");
		    ajStrAppendS(&cmd, pwdstr);
		}
            }
            ajStrAppendC(&cmd, " -pwgapopen=");
            ajStrFromFloat(&tmp, pw_gapc, 3);
            ajStrAppendS(&cmd, tmp);
            ajStrAppendC(&cmd, " -pwgapext=");
            ajStrFromFloat(&tmp, pw_gapv, 3);
            ajStrAppendS(&cmd, tmp);
        }
    }

    /* Multiple alignments */

    /* using existing tree or generating new tree? */
    if(use_dend)
    {
        ajStrAppendC(&cmd, " -usetree=");
        ajStrAppendS(&cmd, dend_filename);
    }
    else
    {
	/* use tmp file to hold dend file, will read back in later */
	tmp_dendfilename = emma_getUniqueFileName();
        ajStrAppendC(&cmd, " -newtree=");
        ajStrAppendS(&cmd, tmp_dendfilename);
    }

    if(ma_matrix)
    {
	if(are_prot)
	    ajStrAppendC(&cmd, " -matrix=");
	else
	    ajStrAppendC(&cmd, " -pwmatrix=");
	ajStrAppendS(&cmd, ajFileGetNameS(ma_matrix));
    }
    else
    {
	if(are_prot)
	{
	    ajStrAppendC(&cmd, " -matrix=");
	    ajStrAppendS(&cmd, m1str);
	}
	else
	{
	    ajStrAppendC(&cmd, " -dnamatrix=");
	    ajStrAppendS(&cmd, m2str);
	}
    }

    ajStrAppendC(&cmd, " -gapopen=");
    ajStrFromFloat(&tmp, gapc, 3);
    ajStrAppendS(&cmd, tmp);
    ajStrAppendC(&cmd, " -gapext=");
    ajStrFromFloat(&tmp, gapv, 3);
    ajStrAppendS(&cmd, tmp);
    ajStrAppendC(&cmd, " -gapdist=");
    ajStrFromInt(&tmp, gap_dist);
    ajStrAppendS(&cmd, tmp);
    ajStrAppendC(&cmd, " -hgapresidues=");
    ajStrAppendS(&cmd, hgapres);

    if(!endgaps)
	ajStrAppendC(&cmd, " -endgaps");

    if(norgap)
	ajStrAppendC(&cmd, " -nopgap");

    if(nohgap)
	ajStrAppendC(&cmd, " -nohgap");

    ajStrAppendC(&cmd, " -maxdiv=");
    ajStrFromInt(&tmp, maxdiv);
    ajStrAppendS(&cmd, tmp);


    /*  run clustalw */

/*    ajFmtError("..%s..\n\n", ajStrGetPtr( cmd)); */
    ajDebug("Executing '%S'\n", cmd);
#ifndef WIN32
    ajSysSystemEnv(cmd, env);
#else
    if(system(ajStrGetPtr(cmd)) == -1)
    {
        fprintf(stderr,"Command: %s\n",ajStrGetPtr(cmd));
	ajFatal("clustalw execution failure");
    }

#endif

    /* produce alignment file only if one was produced */
    if(!only_dend)
    {
	/* read in tmp alignment output file to output through EMBOSS output */

	seqin = ajSeqinNew();
	/*
	**  add the Usa format to the start of the filename to tell EMBOSS
	**  format of file
	*/
	ajStrInsertC(&tmp_aln_outfile, 0, "msf::");
	ajSeqinUsa(&seqin, tmp_aln_outfile);
	seqset = ajSeqsetNew();
	if(ajSeqsetRead(seqset, seqin))
	{
	    ajSeqoutWriteSet(seqout, seqset);


	    ajSeqoutClose(seqout);
	    ajSeqinDel(&seqin);

	    /* remove the Usa from the start of the string */
	    ajStrCutStart(&tmp_aln_outfile, 5);
	}
	else
	    ajFmtError("Problem writing out EMBOSS alignment file\n");
    }


    /* read in new tmp dend file (if produced) to output through EMBOSS */
    if(tmp_dendfilename!=NULL)
    {
	tmp_dendfile = ajFileNewInNameS( tmp_dendfilename);

	if(tmp_dendfile!=NULL){
	while(ajReadlineTrim(tmp_dendfile, &line))
	    ajFmtPrintF(dend_outfile, "%s\n", ajStrGetPtr( line));

	ajFileClose(&tmp_dendfile);
	ajSysFileUnlink(tmp_dendfilename);
    }
    }


    ajSysFileUnlink( tmpFilename);

    if(!only_dend)
	ajSysFileUnlink(tmp_aln_outfile);

    ajStrDel(&pw_matrix);
    ajStrDel(&matrix);
    ajStrDel(&pw_dna_matrix);
    ajStrDel(&dna_matrix);
    ajStrDel(&tmp_dendfilename);
    ajStrDel(&dend_filename);
    ajStrDel(&tmp_aln_outfile);
    ajStrDel(&pwmstr);
    ajStrDel(&pwdstr);
    ajStrDel(&m1str);
    ajStrDel(&m2str);
    ajStrDel(&hgapres);
    ajStrDel(&cmd);
    ajStrDel(&tmp);
    ajStrDel(&tmpFilename);
    ajStrDel(&line);

    ajFileClose(&dend_outfile);
    ajFileClose(&tmp_dendfile);
    ajFileClose(&dend_file);
    ajFileClose(&pairwise_matrix);
    ajFileClose(&ma_matrix);

    ajSeqallDel(&seqall);
    ajSeqsetDel(&seqset);
    ajSeqDel(&seq);
    ajSeqoutDel(&seqout);
    ajSeqoutDel(&fil_file);
    ajSeqinDel(&seqin);

    embExit();

    return 0;
}




/* @funcstatic emma_getUniqueFileName *****************************************
**
** routine to return a name of a unique file; the  unique file name is the
** process id
**
** @return [AjPStr] Undocumented
** @@
******************************************************************************/

static AjPStr emma_getUniqueFileName(void)
{
    static char ext[2] = "A";
    AjPStr filename    = NULL;

    /*
    ** A kludge to make filenames greater than 5 characters. This
    ** sometimes, but by no means always, helped bypass a memory
    ** allocation bug in versions of clustalw before 1.83.
    ** You should update your clustalw.
    */
    ajFmtPrintS(&filename, "%08d%s",getpid(), ext);

    if(++ext[0] > 'Z')
	ext[0] = 'A';


    return filename;
}
