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




static AjBool  seqABIReadInt4(AjPFile fp,ajlong *i4);
static AjBool  seqABIReadFloat4(AjPFile fp,float* f4);
static AjBool  seqABIReadInt2(AjPFile fp, ajshort *i2);
static AjBool  seqABIGetFlag(AjPFile fp, ajlong flagLabel,
			     ajlong flagInstance, ajlong word, ajlong* val);
static AjBool  seqABIGetFlagF(AjPFile fp, ajlong flagLabel,
			      ajlong flagInstance, ajlong word,float* val);
static AjBool  seqABIGetFlagW(AjPFile fp, ajlong flagLabel,
			      ajlong word, ajshort* val);
static ajshort seqABIBaseIdx(char B);





/* @func ajSeqABITest *********************************************************
**
** Test file type is ABI format - look for 'ABIF' flag (which may be in one
** of 2 places).
**
** @param [u] fp [AjPFile] ABI format file
** @return [AjBool] ajTrue on success
** @@
******************************************************************************/

AjBool ajSeqABITest(AjPFile fp)
{
    char pabi[5];
    pabi[4] = '\0';

    ajDebug("ajSeqABITest file %F end: %B\n", fp, fp->End);

    if (fp->End && ajFileIsStdin(fp))
    {
	ajDebug("EOF: ajSeqABITest already at end file %F\n", fp);

	return ajFalse;
    }

    if(ajFileSeek(fp,0,SEEK_SET) >= 0)
	if(ajReadbinBinary(fp,1,4,(void *)pabi))
	{
	    ajDebug("ajSeqABITest was at '%s'\n", pabi);

	    if(ajCharPrefixC(pabi,"ABIF"))
		return ajTrue;
	}

    if(ajFileSeek(fp,26,SEEK_SET) >= 0)
    {
	ajDebug("ajSeqABITest seek to pos 26\n");

	if(ajReadbinBinary(fp,1,4,(void*)pabi))
	{
	    ajDebug("ajSeqABITest seek to '%s'\n", pabi);

	    if(ajCharPrefixC(pabi,"ABIF"))
		return ajTrue;
	}
    }

    return ajFalse;
}




/* @func ajSeqABIReadConfid ***************************************************
**
** Read in confidence values from an ABI trace file.
**
** @param [u] fp [AjPFile] ABI format input file
** @param [r] pconO [ajlong] PCON offset in an ABI file
** @param [r] numBases [ajlong] number of bases
** @param [w] Pqual [float *] array of confidence values
** @return [AjBool] ajTrue on success
** @@
******************************************************************************/

AjBool ajSeqABIReadConfid(AjPFile fp,ajlong pconO,ajlong numBases,
                          float * Pqual)
{
    ajint i;
    char qv;

    ajDebug("ajSeqABIReadConfid pcon0 %Ld numBases %Ld\n", pconO, numBases);

    ajFileSeek(fp,pconO,SEEK_SET);

    for (i=0;i<(ajint)numBases;i++)
    {
	ajReadbinBinary(fp,1,1,&qv);
        Pqual[i] = (float)(int)qv;
    }

    return ajTrue;
}




/* @func ajSeqABIReadSeq ******************************************************
**
** Read in a sequence from an ABI trace file.
**
** @param [u] fp [AjPFile] ABI format input file
** @param [r] baseO [ajlong] PBAS offset in an ABI file
** @param [r] numBases [ajlong] number of bases
** @param [w] nseq [AjPStr*] read sequence
** @return [AjBool] ajTrue on success
** @@
******************************************************************************/

AjBool ajSeqABIReadSeq(AjPFile fp,ajlong baseO,ajlong numBases,
		       AjPStr* nseq)
{
    ajint i;
    char pseq;

    ajDebug("ajSeqABIReadSeq base0 %Ld numBases %Ld\n", baseO, numBases);

    ajFileSeek(fp,baseO,SEEK_SET);

    for (i=0;i<(ajint)numBases;i++)
    {
	ajReadbinBinary(fp,1,1,&pseq);
	ajStrAppendK(nseq,pseq);
    }

    return ajTrue;
}




/* @func ajSeqABIMachineName **************************************************
**
** Get the name of the machine used to obtain an ABI trace file.
**
** @param [u] fp [AjPFile] ABI format file
** @param [w] machine [AjPStr*] machine name
** @return [AjBool] ajTrue on success
** @@
******************************************************************************/

AjBool ajSeqABIMachineName(AjPFile fp,AjPStr *machine)
{
    ajlong mchn;
    ajlong MCHNtag;
    unsigned char l;

    MCHNtag = ((ajlong) ((((('M'<<8)+'C')<<8)+'H')<<8)+'N');
    ajDebug("getflag MCHN\n");

    if(seqABIGetFlag(fp,MCHNtag,1,5,&mchn))
    {
	if (ajFileSeek(fp,mchn,SEEK_SET) >= 0)
	{
	    ajReadbinBinary(fp,1,sizeof(char),&l);
	    *machine = ajStrNewRes(l+1);
	    ajReadbinBinary(fp,1,l,(void*)ajStrGetuniquePtr(machine));
	    *(ajStrGetuniquePtr(machine)+l)='\0';
	}
	else
	    return ajFalse;
    }
    else
	return ajFalse;

    return ajTrue;
}




/* @func ajSeqABIGetNData *****************************************************
**
** Find 'DATA' tag and get the number of data points.
**
** @param [u] fp [AjPFile] ABI format file
** @return [ajint] Number of data points in file
** @@
******************************************************************************/

ajint ajSeqABIGetNData(AjPFile fp)
{

    ajlong numPoints;
    ajlong DATAtag;
    ajshort TRACE_INDEX;

    DATAtag = ((ajlong) ((((('D'<<8)+'A')<<8)+'T')<<8)+'A');
    TRACE_INDEX = 9;

    if(!seqABIGetFlag(fp,DATAtag,TRACE_INDEX,3,&numPoints))
	ajFatal("Error - locating DATA tag");

    return (ajint) numPoints;
}




/* @func ajSeqABIGetNBase *****************************************************
**
** Find the 'PBAS' tag in an ABI trace file and get the number of bases.
**
** @param [u] fp [AjPFile] ABI format file
** @return [ajint] Number of bases in file
** @@
******************************************************************************/

ajint ajSeqABIGetNBase(AjPFile fp)
{
    ajlong numBases;
    ajlong BASEtag;

    BASEtag = ((ajlong) ((((('P'<<8)+'B')<<8)+'A')<<8)+'S');

    ajDebug("getflag PBAS 2 NBase\n");
    if(!seqABIGetFlag(fp,BASEtag,2,3,&numBases))
        if(!seqABIGetFlag(fp,BASEtag,1,3,&numBases))
        {
	    ajWarn("ABI file has no PBAS tag for sequence data");
            return 0;
        }

    return (ajint) numBases;
}




/* @func ajSeqABIGetData ******************************************************
**
** Read in the processed trace data from an ABI file.
**
** @param [u] fp [AjPFile] ABI format file
** @param [r] Offset [const ajlong*] data offset in ABI file
** @param [r] numPoints [ajlong] number of data points
** @param [w] trace [AjPInt2d] (4xnumPoints) array of trace data
** @return [void]
** @@
******************************************************************************/

void ajSeqABIGetData(AjPFile fp,const ajlong *Offset,ajlong numPoints,
                     AjPInt2d trace)
{
    ajint i;
    ajint j;
    ajshort traceValue;

    /* Read in data  */
    for (i=0;i<4;i++)
    {
        if (ajFileSeek(fp,Offset[i],SEEK_SET))
	    ajFatal("Error - reading trace");

        for (j=0;j<(ajint)numPoints;j++)
            if (seqABIReadInt2(fp,&traceValue))
                ajInt2dPut(&trace,i,j,(ajint)traceValue);
            else
                ajFatal("Error - reading trace");
    }

    return;
}




/* @func ajSeqABIGetBasePosition **********************************************
**
** Read in the base positions from an ABI file.
**
** @param [u] fp [AjPFile] ABI format file
** @param [r] numBases [ajlong] number of bases to be read
** @param [w] basePositions [AjPShort*] base positions output
** @return [void]
** @@
******************************************************************************/

void ajSeqABIGetBasePosition(AjPFile fp,ajlong numBases,
                             AjPShort* basePositions)
{
    ajint i;
    ajshort bP;

    /* Read in base positions   */
    for (i=0;i<(ajint)numBases;i++)
    {
        if (!seqABIReadInt2(fp,&bP))
	    ajFatal("Error - in finding Base Position");

        ajShortPut(basePositions,i,bP);
    }

    return;
}




/* @func ajSeqABIGetSignal ****************************************************
**
** Read in the signal strength information from an ABI file.
**
** @param [u] fp [AjPFile] ABI format file
** @param [r] fwo_ [ajlong] field order
** @param [w] sigC [ajshort*] average signal strength for C
** @param [w] sigA [ajshort*] average signal strength for A
** @param [w] sigG [ajshort*] average signal strength for G
** @param [w] sigT [ajshort*] average signal strength for T
** @return [void]
** @@
******************************************************************************/

void ajSeqABIGetSignal(AjPFile fp,ajlong fwo_,
		       ajshort *sigC,ajshort *sigA,
		       ajshort *sigG,ajshort *sigT)
{
    ajlong signalO;
    ajshort* base[4];

    ajlong SIGNALtag;

    SIGNALtag    = ((ajlong) ((((('S'<<8)+'/')<<8)+'N')<<8)+'%');
    ajDebug("getflag S/N%%\n");

    /* Get signal strength info */
    if (seqABIGetFlag(fp,SIGNALtag,1,5,&signalO))
    {
        base[0] = sigC;
        base[1] = sigA;
        base[2] = sigG;
        base[3] = sigT;
        if (ajFileSeek(fp, signalO, SEEK_SET) >= 0 &&
            seqABIReadInt2(fp, base[seqABIBaseIdx((char)(fwo_>>24&255))]) &&
            seqABIReadInt2(fp, base[seqABIBaseIdx((char)(fwo_>>16&255))]) &&
            seqABIReadInt2(fp, base[seqABIBaseIdx((char)(fwo_>>8&255))]) &&
            seqABIReadInt2(fp, base[seqABIBaseIdx((char)(fwo_&255))]))
	{
	    /*
	       ajDebug("avg_signal_strength = C:%d A:%d G:%d T:%d\n",sigC,sigA,
			  sigG,sigT);
	    */
	}
    }

    return;
}




/* @func ajSeqABIGetBaseSpace *************************************************
**
** Read in the base spacing from an ABI file.
**
** @param [u] fp [AjPFile] ABI format file
** @return [float] base spacing
** @@
******************************************************************************/

float ajSeqABIGetBaseSpace(AjPFile fp)
{

    float spacing = 0.;
    ajlong SPACINGtag;

    SPACINGtag = ((ajlong) ((((('S'<<8)+'P')<<8)+'A')<<8)+'C');

    seqABIGetFlagF(fp,SPACINGtag,1,5,&spacing);

    return spacing;
}




/* @func ajSeqABIGetBaseOffset ************************************************
**
** Routine to get the 'PBAS' tag offset in an ABI file.
**
** @param [u] fp [AjPFile] ABI format file
** @return [ajint] 'PBAS' tag offset in an ABI file
** @@
******************************************************************************/

ajint ajSeqABIGetBaseOffset(AjPFile fp)
{
    ajlong baseO;
    ajlong BASEtag;

    BASEtag = ((ajlong) ((((('P'<<8)+'B')<<8)+'A')<<8)+'S');
    ajDebug("getflag PBAS 2 offset\n");

    /* Find BASE tag & get offset                                */
    if(!seqABIGetFlag(fp,BASEtag,2,5,&baseO))
        if(!seqABIGetFlag(fp,BASEtag,1,5,&baseO))
        {
	    ajWarn("ABI file has no PBAS flag for Base Offset");
            return 0;
        }

    return (ajint) baseO;
}




/* @func ajSeqABIGetBasePosOffset *********************************************
**
** Routine to get the 'PLOC', base position, tag offset in an ABI file
**
** @param [u] fp [AjPFile] ABI format file
** @return [ajint] base position offset in an ABI file
** @@
******************************************************************************/

ajint ajSeqABIGetBasePosOffset(AjPFile fp)
{
    ajlong basePosO;
    ajlong BASEPOStag;

    BASEPOStag = ((ajlong) ((((('P'<<8)+'L')<<8)+'O')<<8)+'C');
    ajDebug("getflag PLOC 2\n");

    /* Find BASEPOS tag & get base position offset               */
    if (!seqABIGetFlag(fp,BASEPOStag,2,5,&basePosO))
        if (!seqABIGetFlag(fp,BASEPOStag,1,5,&basePosO))
        {
            ajWarn("ABI file has no PLOC flag for Base Pos Offset");
            return 0;
        }

    return (ajint) basePosO;
}




/* @func ajSeqABIGetConfidOffset **********************************************
**
** Routine to get the 'PCON' confidence tag offset in an ABI file.
**
** @param [u] fp [AjPFile] ABI format file
** @return [ajint] 'PCON' tag offset in an ABI file
** @@
******************************************************************************/

ajint ajSeqABIGetConfidOffset(AjPFile fp)
{
    ajlong pconO;
    ajlong PCONtag;

    PCONtag = ((ajlong) ((((('P'<<8)+'C')<<8)+'O')<<8)+'N');
    ajDebug("getflag PCON 2\n");

    /* Find PCON tag & get offset                                */
    if(!seqABIGetFlag(fp,PCONtag,2,5,&pconO))
        if(!seqABIGetFlag(fp,PCONtag,1,5,&pconO))
        {
            /* ajWarn("ABI file has no PCON flag for Confidence Offset");*/
            return 0;
        }

    return (ajint) pconO;
}




/* @func ajSeqABIGetFWO *******************************************************
**
** Routine to get the "FWO" tag, field order ("GATC"), tag.
**
** @param [u] fp [AjPFile] ABI format file
** @return [ajint] field order
** @@
******************************************************************************/

ajint ajSeqABIGetFWO(AjPFile fp)
{

    ajlong fwo_;
    ajlong FWO_tag;

    FWO_tag = ((ajlong) ((((('F'<<8)+'W')<<8)+'O')<<8)+'_');
    ajDebug("getflag FWO_ 2\n");

    /* Find FWO tag */
    if (!seqABIGetFlag(fp,FWO_tag,2,5,&fwo_))
        if (!seqABIGetFlag(fp,FWO_tag,1,5,&fwo_))
        {
	    ajWarn("ABI file has no FWO_ flag for field order");
            return 0;
        }

    return (ajint) fwo_;
}




/* @func ajSeqABIGetPrimerOffset **********************************************
**
** Routine to get the primer offset in an ABI file.
**
** @param [u] fp [AjPFile] ABI format file
** @return [ajint] primer offset
** @@
******************************************************************************/

ajint ajSeqABIGetPrimerOffset(AjPFile fp)
{
    ajshort primerPos;
    ajlong PPOStag;

    PPOStag = ((ajlong) ((((('P'<<8)+'P')<<8)+'O')<<8)+'S');

    /* Find PPOS tag (Primer Position) & get offset              */
    if (!seqABIGetFlagW(fp,PPOStag,6,&primerPos))
    {
        ajWarn("ABI file has no PPOS flag for primer offset");
        return 0;
    }

    return primerPos;
}




/* @func ajSeqABIGetPrimerPosition ********************************************
**
** Routine to get the primer position in an ABI file.
**
** @param [u] fp [AjPFile] ABI format file
** @return [ajint] primer position
** @@
******************************************************************************/

ajint ajSeqABIGetPrimerPosition(AjPFile fp)
{
    ajlong primerPosition;
    ajlong PPOStag;

    PPOStag = ((ajlong) ((((('P'<<8)+'P')<<8)+'O')<<8)+'S');
    ajDebug("getflag PPOS 2\n");

    if (!seqABIGetFlag(fp,PPOStag,2,5,&primerPosition))
    if (!seqABIGetFlag(fp,PPOStag,1,5,&primerPosition))
    {
	ajWarn("ABI file has no PPOS flag for primer position");
        return 0;
    }
    
    /* ppos stored in MBShort of pointer */
    primerPosition = primerPosition>>16;


    return (ajint) primerPosition;
}




/* @func ajSeqABIGetTraceOffset ***********************************************
**
** Get the processed trace data ('DATA' tag) offset in an ABI file.
**
** @param [u] fp [AjPFile] ABI format file
** @param [w] Offset [ajlong *] trace data offset, used in ajSeqABIGetData
** @return [AjBool]  ajTrue on success
** @@
******************************************************************************/

AjBool ajSeqABIGetTraceOffset(AjPFile fp, ajlong *Offset)
{
    ajlong dataxO[4];
    ajlong fwo_;

    /* BYTE[i] is a byte mask for byte i */
    const ajlong BYTE[] = { 0x000000ff };
    ajshort TRACE_INDEX;
    ajlong DATAtag;

    TRACE_INDEX = 9;
    DATAtag     = ((ajlong) ((((('D'<<8)+'A')<<8)+'T')<<8)+'A');

    /* Find FWO tag - Field order "GATC" */
    fwo_ = ajSeqABIGetFWO(fp);

    /* Get data trace offsets            */
    if (!seqABIGetFlag(fp,DATAtag,TRACE_INDEX,
		       5,&dataxO[seqABIBaseIdx((char)(fwo_>>24&BYTE[0]))]))
	return ajFalse;

    if (!seqABIGetFlag(fp,DATAtag,TRACE_INDEX+1,
		       5,&dataxO[seqABIBaseIdx((char)(fwo_>>16&BYTE[0]))]))
	return ajFalse;

    if (!seqABIGetFlag(fp,DATAtag,TRACE_INDEX+2,
		       5,&dataxO[seqABIBaseIdx((char)(fwo_>>8&BYTE[0]))]))
	return ajFalse;

    if (!seqABIGetFlag(fp,DATAtag,TRACE_INDEX+3,
		       5,&dataxO[seqABIBaseIdx((char)(fwo_&BYTE[0]))]))
	return ajFalse;

    Offset[0]=dataxO[seqABIBaseIdx((char)(fwo_>>24&BYTE[0]))];
    Offset[1]=dataxO[seqABIBaseIdx((char)(fwo_>>16&BYTE[0]))];
    Offset[2]=dataxO[seqABIBaseIdx((char)(fwo_>>8&BYTE[0]))];
    Offset[3]=dataxO[seqABIBaseIdx((char)(fwo_&BYTE[0]))];

    return ajTrue;
}




/* @funcstatic seqABIReadInt4  ************************************************
**
** Routine to read 4 bytes from a file and return the integer.
**
** @param [u] fp [AjPFile] ABI format file
** @param [w] i4 [ajlong *] ajlong integer read in from ABI file
** @return [AjBool] true if read successfully
** @@
******************************************************************************/

static AjBool seqABIReadInt4(AjPFile fp,ajlong *i4)
{

    unsigned char buf[sizeof(ajlong)];

    if (ajReadbinBinary(fp,1,4,(void *)buf) != 1)
	return ajFalse;

    *i4 = (ajlong)
        (((ajulong)buf[3]) +
         ((ajulong)buf[2]<<8) +
         ((ajulong)buf[1]<<16) +
         ((ajulong)buf[0]<<24));

    /*ajDebug("seqABIReadInt4 %Ld, %x %x %x %x\n", *i4,
	    (int) buf[0],(int) buf[1],
	    (int) buf[2],(int) buf[3]);*/

    return ajTrue;
}




/* @funcstatic seqABIReadFloat4 ***********************************************
**
** Routine to read 4 bytes from a file and return the float.
**
** @param [u] fp [AjPFile] ABI format file
** @param [w] f4 [float *] float read in from ABI file
** @return [AjBool] true if read successfully
** @@
******************************************************************************/

static AjBool seqABIReadFloat4(AjPFile fp,float* f4)
{

    unsigned char buf[sizeof(ajlong)];
    ajulong res;
    
    if (ajReadbinBinary(fp,1,4,(void *)buf) != 1)
	return ajFalse;

    res = (ajulong)
        (((ajulong)buf[3]) +
         ((ajulong)buf[2]<<8) +
         ((ajulong)buf[1]<<16) +
         ((ajulong)buf[0]<<24));

    *f4 = (float) res;

    return ajTrue;
}




/* @funcstatic seqABIReadInt2 *************************************************
**
** Routine to read 2 bytes from a file and return the short integer.
**
** @param [u] fp [AjPFile] ABI format file
** @param [w] i2 [ajshort *] short integer read in from ABI file
** @return [AjBool] true if read successfully
** @@
******************************************************************************/

static AjBool seqABIReadInt2(AjPFile fp, ajshort *i2)
{
    unsigned char buf[sizeof(ajshort)];

    if (ajReadbinBinary(fp,1,2,(void *)buf) != 1)
	return ajFalse;

    *i2 = (ajshort)
        (((ajushort)buf[1]) +
         ((ajushort)buf[0]<<8));

    return ajTrue;
}




/* @funcstatic seqABIGetFlag **************************************************
**
** Routine to read through an ABI trace file until it reaches a flag
** (flagLabel). If there are multiple flags in the file it will search
** to find the correct instance of that flag (flagInstance).
** It  will then return the *integer* value (val) of the word+1 from
** that flag record.
**
** @param [u] fp [AjPFile] ABI format file
** @param [r] flagLabel [ajlong] flag in the ABI file
** @param [r] flagInstance [ajlong] flag instance in the ABI file
** @param [r] word [ajlong] number of fields to ignore in this record
** @param [w] val [ajlong*] integer value of the word+1
** @return [AjBool] true if read successfully
** @@
******************************************************************************/

static AjBool seqABIGetFlag(AjPFile fp, ajlong flagLabel,
			    ajlong flagInstance, ajlong word, ajlong* val)
{
    ajint     flagNum = -1;
    ajint     i;
    ajlong Label;
    ajlong Instance;
    ajlong indexO;
    ajint INDEX_ENTRY_LENGTH;

    INDEX_ENTRY_LENGTH= 28;

    if(ajFileSeek(fp,26,SEEK_SET) ||
       (!seqABIReadInt4(fp, &indexO))) ajFatal("Error - in finding flag");

    ajDebug("seqABIGetFlag Flag %Lp %Ld %Ld indexO %Ld\n",
            flagLabel, flagInstance, word, indexO);
    do
    {
        flagNum++;

        if (ajFileSeek(fp,indexO+(flagNum*INDEX_ENTRY_LENGTH),SEEK_SET) != 0)
        {
            ajDebug("num %d Flag %Lp %Ld %Ld end of file\n",
                    flagNum, flagLabel, flagInstance, word);
            return ajFalse;
        }

        if (!seqABIReadInt4(fp, &Label))
            return ajFalse;

        if (!seqABIReadInt4(fp, &Instance))
            return ajFalse;

        ajDebug("Label %Lp Instance %Ld\n", Label, Instance);

    } while (!(Label == (ajlong)flagLabel &&
               Instance == (ajlong)flagInstance));

    for (i=2; i<=word; i++)
        if (!seqABIReadInt4(fp, val))
	    return ajFalse;

    ajDebug("OK Flag %Lp %Ld %Ld (%d)\n",
           flagLabel, flagInstance, word, val[0]);
    return ajTrue;

}




/* @funcstatic seqABIGetFlagF *************************************************
**
** Routine to read through an ABI trace file until it reaches a flag
** (flagLabel). If there are multiple flags in the file it will search
** to find the correct instance of that flag (flagInstance).
** It  will then return the *float* value (val) of the word+1 from
** that flag record.
**
** @param [u] fp [AjPFile] ABI format file
** @param [r] flagLabel [ajlong] flag in the ABI file
** @param [r] flagInstance [ajlong] flag instance in the ABI file
** @param [r] word [ajlong] number of fields to ignore in this record
** @param [w] val [float*] integer value of the word+1
** @return [AjBool] true if read successfully
** @@
******************************************************************************/

static AjBool seqABIGetFlagF(AjPFile fp, ajlong flagLabel,
			     ajlong flagInstance, ajlong word,float* val)
{
    ajint     flagNum = -1;
    ajint     i;
    ajlong Label;
    ajlong Instance;
    ajlong indexO;
    ajint INDEX_ENTRY_LENGTH;

    INDEX_ENTRY_LENGTH= 28;

    if(ajFileSeek(fp,26,SEEK_SET) ||
       (!seqABIReadInt4(fp, &indexO))) ajFatal("Error - in finding flag");

    do
    {
        flagNum++;

        if (ajFileSeek(fp,indexO+(flagNum*INDEX_ENTRY_LENGTH),SEEK_SET) != 0)
            return ajFalse;

        if (!seqABIReadInt4(fp, &Label))
            return ajFalse;

        if (!seqABIReadInt4(fp, &Instance))
            return ajFalse;
    } while (!(Label == (ajlong)flagLabel &&
               Instance == (ajlong)flagInstance));

    for (i=2; i<=word; i++)
        if (!seqABIReadFloat4(fp, val))
	    return ajFalse;

    return ajTrue;
}




/* @funcstatic seqABIGetFlagW *************************************************
**
** Routine to read through an ABI trace file until it reaches a flag
** (flagLabel). If there are multiple flags in the file it will search
** to find the correct instance of that flag (flagInstance).
** It  will then return the *short ajint* value (val) of the word+1 from
** that flag record.
**
** @param [u] fp [AjPFile] ABI format file
** @param [r] flagLabel [ajlong] flag in the ABI file
** @param [r] word [ajlong] number of fields to ignore in this record
** @param [w] val [ajshort*] integer value of the word+1
** @return [AjBool] true if read successfully
** @@
******************************************************************************/

static AjBool seqABIGetFlagW(AjPFile fp, ajlong flagLabel,
			     ajlong word, ajshort* val)
{
    ajint     flagNum = -1;
    ajint     i;
    ajlong Label;
    ajlong jval;
    ajlong indexO;
    ajint  INDEX_ENTRY_LENGTH;

    INDEX_ENTRY_LENGTH= 28;

    if(ajFileSeek(fp,26,SEEK_SET) ||
       (!seqABIReadInt4(fp, &indexO))) ajFatal("Error - in finding flag");

    do
    {
        flagNum++;

        if (ajFileSeek(fp, indexO+(flagNum*INDEX_ENTRY_LENGTH), SEEK_SET) != 0)
            return ajFalse;

        if (!seqABIReadInt4(fp, &Label))
            return ajFalse;
    }
    while (Label != (ajlong)flagLabel);


    for (i=2; i<word; i++)
        if (!seqABIReadInt4(fp, &jval))
	    return ajFalse;

    if (!seqABIReadInt2(fp, val))
	return ajFalse;

    return ajTrue;
}




/* @funcstatic seqABIBaseIdx **************************************************
**
** Returns: 0 if C, 1 if A, 2 if G, 3 if anything else
**
** @param [r] B [char] base (C, A, G or T)
** @return [ajshort] 0 if C, 1 if A, 2 if G, 3 if anything else
** @@
******************************************************************************/

static ajshort seqABIBaseIdx(char B)
{
    return ((B)=='C' ? 0 : (B)=='A' ? 1 : (B)=='G' ? 2 : 3);
}




/* @func ajSeqABISampleName ***************************************************
**
** Get the sample name from an ABI trace file.
**
** @param [u] fp [AjPFile] ABI format file
** @param [w] sample [AjPStr*] sample name
** @return [AjBool] true if read successfully
** @@
******************************************************************************/

AjBool ajSeqABISampleName(AjPFile fp, AjPStr *sample)
{
    ajlong mchn;
    ajlong SMPLtag;
    unsigned char l;

    SMPLtag = ((ajlong) ((((('S'<<8)+'M')<<8)+'P')<<8)+'L');
    ajDebug("getflag SMPL\n");

    if((seqABIGetFlag(fp,SMPLtag,1,5,&mchn)) &&
       (ajFileSeek(fp,mchn,SEEK_SET) >= 0))
    {
	ajReadbinBinary(fp,1,sizeof(char),&l);
	*sample = ajStrNewRes(l+1);
	ajReadbinBinary(fp,1,l,(void*)ajStrGetuniquePtr(sample));
	*(ajStrGetuniquePtr(sample)+l)='\0';
    }

    return ajTrue;
}
