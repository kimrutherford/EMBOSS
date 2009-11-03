/* @source splitsource application
**
** Split a sequence into source sequences
**
** @author Copyright (C) Gary Williams (gwilliam@hgmp.mrc.ac.uk)
** @Modified: Rewritten for more intuitive overlaps (ableasby@hgmp.mrc.ac.uk)
** @Modified: Split into 2 applications (pmr@ebi.ac.uk)
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



static void splitsource_write(AjPSeqout seqout, AjPSeq subseq,
                              const AjPStr desc);
static void splitsource_AddSubSeqFeat(AjPFeattable ftable, const AjPFeature gf,
                                      const AjPSeq oldseq);
static void splitsource_ProcessChunk(AjPSeqout seqout, const AjPSeq seq,
                                     const AjPFeature gf,
                                     const AjPStr name,  AjBool feature);
static void splitsource_MakeSubSeqName(AjPStr * name_ptr, const AjPSeq seq,
                                       ajuint count,
                                       const  AjPFeature gf);




/* @prog splitsource **********************************************************
**
** Split a sequence into (overlapping) smaller sequences
**
******************************************************************************/

int main(int argc, char **argv)
{

    AjPSeqall seqall;
    AjPSeqout seqout;
    AjPSeq seq;
    AjBool feature;
    AjPStr outseq_name = ajStrNew();

    AjIList iter = NULL;
    AjPFeattable old_feattable = NULL;
    AjPFeature gf = NULL;
    const AjPStr type = NULL;
    AjPStr origid = NULL;
    ajuint count = 0;

    embInit("splitsource", argc, argv);

    seqout  = ajAcdGetSeqoutall("outseq");
    seqall  = ajAcdGetSeqall("sequence");
    feature = ajAcdGetBoolean("feature");

    while(ajSeqallNext(seqall, &seq))
    {
	ajSeqTrim(seq);

        old_feattable = ajSeqGetFeatCopy(seq);
        iter = ajListIterNewread(old_feattable->Features);

        while(!ajListIterDone(iter))
        {
            gf = ajListIterGet(iter);
            type = ajFeatGetType(gf);

            if (ajStrMatchC(type, "source"))
            {
                splitsource_MakeSubSeqName(&outseq_name, seq,
                                           ++count, gf);
                splitsource_ProcessChunk(seqout,seq,
                                         gf,
                                         outseq_name, feature);
             }
        }
        ajFeattableDel(&old_feattable);
        ajListIterDel(&iter);
    }

    ajSeqoutClose(seqout);
    ajSeqallDel(&seqall);
    ajSeqDel(&seq);
    ajSeqoutDel(&seqout);

    ajStrDel(&origid);
    ajStrDel(&outseq_name);

    embExit();

    return 0;
}




/* @funcstatic splitsource_write  *********************************************
**
** Write out split sequence
**
** @param [u] default_seqout [AjPSeqout] Output object
** @param [u] subseq [AjPSeq] sequence to write
** @param [r] desc [const AjPStr] Description
** @return [void]
** @@
******************************************************************************/

static void splitsource_write(AjPSeqout default_seqout,
                           AjPSeq subseq, const AjPStr desc)
{
  /* set the description of the subsequence */
  ajSeqAssignDescS(subseq, desc);

  /* set the type of the subsequence */
  ajSeqType(subseq);

  ajSeqoutWriteSeq(default_seqout, subseq);

  return;
}

/* @funcstatic splitsource_MakeSubSeqName *************************************
**
** Undocumented
**
** @param [w] name_ptr [AjPStr*] Undocumented
** @param [r] seq [const AjPSeq] Undocumented
** @param [r] count [ajuint] Undocumented
** @param [r] gf [const AjPFeature] Undocumented
**
******************************************************************************/

static void splitsource_MakeSubSeqName (AjPStr * name_ptr,
                                        const AjPSeq seq, ajuint count,
                                        const AjPFeature gf)
{
  AjPStr value = ajStrNew();
  ajuint start;
  ajuint end;

  start = ajFeatGetStart(gf)-1;
  end = ajFeatGetEnd(gf)-1;

  /* create a nice name for the subsequence */
  ajStrAssignS(name_ptr, ajSeqGetNameS(seq));
  ajStrAppendC(name_ptr, "_");
  ajStrFromUint(&value, count);
  ajStrAppendS(name_ptr, value);
  ajStrAppendC(name_ptr, "_");
  ajStrFromUint(&value, ajSeqGetBegin(seq)+start);
  ajStrAppendS(name_ptr, value);
  ajStrAppendC(name_ptr, "-");
  ajStrFromUint(&value, ajSeqGetBegin(seq)+end);
  ajStrAppendS(name_ptr, value);

  ajStrDel(&value);
}

/* @funcstatic splitsource_ProcessChunk ***************************************
**
** Undocumented
**
** @param [u] seqout [AjPSeqout] Undocumented
** @param [r] seq [const AjPSeq] Undocumented
** @param [r] gf [const AjPFeature] Undocumented
** @param [r] name [const AjPStr] Undocumented
** @param [r] feature [AjBool] Undocumented
**
******************************************************************************/

static void splitsource_ProcessChunk (AjPSeqout seqout, const AjPSeq seq,
                                      const AjPFeature gf, const AjPStr name,
                                      AjBool feature)
{
  AjPStr str = ajStrNew();

  AjPFeattable new_feattable = NULL;
  AjPSeq subseq = ajSeqNew ();
  ajuint start;
  ajuint end;
  AjPStr desc = ajStrNew();
  AjPStr tagval = ajStrNew();
  const char *srctagnames[] = {"organism", "strain", "clone",
                            NULL};
  ajint i;

  start = ajFeatGetStart(gf)-1;
  end = ajFeatGetEnd(gf)-1;

  ajDebug("splitsource_ProcessChunk %d..%d '%S' %B\n",
	  start, end, name, feature);

  new_feattable = ajFeattableNew(name);
  subseq->Fttable = new_feattable;
  ajFeattableSetNuc(new_feattable);

  ajStrAssignSubC(&str,ajSeqGetSeqC(seq),start,end);
  ajSeqAssignSeqS(subseq,str);
  if (feature)
    splitsource_AddSubSeqFeat(subseq->Fttable,gf,seq);
  ajSeqAssignNameS(subseq, name);


  for(i=0;srctagnames[i];i++)
  {
      if(ajFeatGetTagC(gf, srctagnames[i], 1, &tagval))
          ajFmtPrintAppS(&desc, "%s=\"%S\" ", srctagnames[i], tagval);
  }
  ajStrAppendS(&desc, ajSeqGetDescS(seq));
  

  splitsource_write(seqout,subseq,desc);

  ajStrDel(&str);
  ajSeqDel(&subseq);
  ajStrDel(&desc);

  return;
}



/* @funcstatic splitsource_AddSubSeqFeat **************************************
**
** Undocumented
**
** @param [u] ftable [AjPFeattable] Undocumented
** @param [r] gfsrc [const AjPFeature] Undocumented
** @param [r] oldseq [const AjPSeq] Undocumented
**
******************************************************************************/

static void splitsource_AddSubSeqFeat(AjPFeattable ftable,
                                      const AjPFeature gfsrc,
                                      const AjPSeq oldseq)
{
  AjPFeattable old_feattable = NULL;
  AjIList iter = NULL;
  ajuint start;
  ajuint end;

  start = ajFeatGetStart(gfsrc)-1;
  end = ajFeatGetEnd(gfsrc)-1;

  old_feattable = ajSeqGetFeatCopy(oldseq);
  iter = ajListIterNewread(old_feattable->Features);

  while(!ajListIterDone(iter)) {
    AjPFeature gf = ajListIterGet(iter);

    AjPFeature copy = NULL;


    if (((ajFeatGetEnd(gf) < start + 1) &&
        (gf->End2 == 0 || gf->End2 < start + 1)) ||
        ((ajFeatGetStart(gf) > end + 1) &&
        (gf->Start2 == 0 || gf->Start2 > end + 1)))
    {
        continue;
    }

    copy = ajFeatCopy(gf);
    copy->Start = copy->Start - start;
    copy->End = copy->End - start;

    if (copy->Start2 > 0)
      copy->Start2 = copy->Start2 - start;

    if (copy->End2 > 0)
      copy->End2 = copy->End2 - start;

    ajFeatTrimOffRange (copy, 0, 1, end - start + 1, AJTRUE, AJTRUE);

    ajFeattableAdd(ftable, copy);
  }
  ajFeattableDel(&old_feattable);
  ajListIterDel(&iter);
}
