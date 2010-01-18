/* @source notseq  application
**
** Excludes a set of sequences and writes out the remaining ones
**
** @author Copyright (C) Gary Williams
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




static void notseq_readfile(const AjPStr exclude, AjPStr *pattern);




/* @prog notseq ***************************************************************
**
** Excludes a set of sequences and writes out the remaining ones
**
******************************************************************************/

int main(int argc, char **argv)
{

    AjPSeqall seqall;
    AjPSeqout seqout;
    AjPSeqout junkout;
    AjPSeq seq = NULL;
    AjPStr exclude = NULL;
    AjPStr pattern = NULL;
    AjPStr name = NULL;
    AjPStr acc  = NULL;

    embInit("notseq", argc, argv);

    seqout  = ajAcdGetSeqoutall("outseq");
    junkout = ajAcdGetSeqoutall("junkoutseq");
    seqall  = ajAcdGetSeqall("sequence");
    exclude = ajAcdGetString("exclude");

    notseq_readfile(exclude, &pattern);

    while(ajSeqallNext(seqall, &seq))
    {
	ajStrAssignS(&name, ajSeqGetNameS(seq));
	ajStrAssignS(&acc, ajSeqGetAccS(seq));

	if(embMiscMatchPatternDelimC(name, pattern, ",;") ||
           embMiscMatchPatternDelimC(acc, pattern, ",;"))
	    ajSeqoutWriteSeq(junkout, seq);
	else
	    /* no match, so not excluded */
	    ajSeqoutWriteSeq(seqout, seq);

	ajStrSetClear(&name);
	ajStrSetClear(&acc);
    }

    ajSeqoutClose(seqout);
    ajSeqoutClose(junkout);

    ajSeqallDel(&seqall);
    ajSeqDel(&seq);
    ajSeqoutDel(&seqout);
    ajSeqoutDel(&junkout);
    ajStrDel(&exclude);
    ajStrDel(&pattern);
    ajStrDel(&name);
    ajStrDel(&acc);

    embExit();

    return 0;
}




/* @funcstatic notseq_readfile ************************************************
**
** If the list of names starts with a '@', open that file, read in
** the list of names and replaces the input string with the names
**
** Else simply copy the exclude list
**
** @param [r] exclude [const AjPStr] names to search for or 'file'
** @param [w] pattern [AjPStr*] names to search for or 'file'
** @return [void]
** @@
******************************************************************************/

static void notseq_readfile(const AjPStr exclude, AjPStr *pattern)
{
    AjPFile file = NULL;
    AjPStr line;
    AjPStr filename = NULL;
    const char *p = NULL;

    if(ajStrFindC(exclude, "@") != 0)
    {
	ajStrAssignS(pattern, exclude);
    }
    else
    {
	ajStrAssignS(&filename, exclude);
        ajStrTrimC(&filename, "@");       /* remove the @ */
        file = ajFileNewInNameS(filename);
        if(file == NULL)
            ajFatal("Cannot open the file of sequence names: '%S'", filename);

        /* blank off the file name and replace with the sequence names */
        ajStrSetClear(pattern);
        line = ajStrNew();
        while(ajReadlineTrim(file, &line))
        {
            p = ajStrGetPtr(line);

            if(!*p || *p == '#' || *p == '!')
		continue;

            ajStrAppendS(pattern, line);
            ajStrAppendC(pattern, ",");
        }
        ajStrDel(&line);
        ajStrDel(&filename);

        ajFileClose(&file);
    }

    return;
}
