#include "emboss.h"




static void ajtest_kim (const AjPStr seqout_name, const AjPSeq subseq);




/* @prog ajtest ***************************************************************
**
** testing, and subject to frequent change
**
******************************************************************************/

int main(int argc, char **argv)
{
    AjPSeqset seqset;
    AjPSeqall seqall;
    AjPSeq seq;
    ajint i = 0;
    AjPStr kimout = NULL;
    AjPStr dir = NULL;

    embInit("ajtest", argc, argv);

    seqall = ajAcdGetSeqall ("sequence");
    seqset = ajAcdGetSeqset ("bsequence");
    dir = ajAcdGetOutdirName("outdir");

    while(ajSeqallNext (seqall, &seq))
    {
	ajFmtPrintS(&kimout, "kim%d.out", ++i);
	ajtest_kim (kimout, seq);
    }

    ajSeqDel(&seq);
    ajSeqallDel(&seqall);
    ajSeqsetDel(&seqset);
    ajStrDel(&kimout);
    ajStrDel(&dir);

    embExit();

    return 0;
}




/* @funcstatic ajtest_kim *****************************************************
**
** Test for Kim Rutherford's reported problem
**
** @param [r] seqout_name [const AjPStr] Seqout name
** @param [r] subseq [const AjPSeq] Subsequence
** @return [void]
** @@
******************************************************************************/

static void ajtest_kim (const AjPStr seqout_name, const AjPSeq subseq)
{
    AjPFile seqout_file = ajFileNewOutNameS(seqout_name);
    AjPSeqout named_seqout = ajSeqoutNewFile(seqout_file);

    AjPStr format_str = ajStrNew();

    ajStrAssignC(&format_str, "embl");

    ajSeqoutSetFormatS(named_seqout, format_str);

    ajSeqoutWriteSeq(named_seqout, subseq);

    ajSeqoutDel(&named_seqout);
    ajFileClose(&seqout_file);
    ajStrDel(&format_str);

    return;
}
