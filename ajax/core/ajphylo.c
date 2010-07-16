/******************************************************************************
** @source AJAX Phylogeny functions
** These functions create and control linked lists.
**
** @author Copyright (C) 2003 Peter Rice
** @version 1.0
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

#include "ajax.h"


static AjPRegexp phyloRegDistFloat = NULL;
static AjPRegexp phyloRegFreqFloat = NULL;
static AjPRegexp phyloRegFreqInt   = NULL;
static AjPRegexp phyloRegFreqIndiv = NULL;
static AjPRegexp phyloRegPropProp = NULL;
static AjPRegexp phyloRegStateState  = NULL;
static AjPRegexp phyloRegStateInt    = NULL;
static AjPRegexp phyloRegStateChar   = NULL;
static AjPRegexp phyloRegTreeTree   = NULL;
static AjPRegexp phyloRegTreeRoot   = NULL;
static AjPRegexp phyloRegTreeUnroot = NULL;
static AjPRegexp phyloRegTreeMulti  = NULL;
static AjPRegexp phyloRegTreeQuart  = NULL;
static AjPRegexp phyloRegTreeLength  = NULL;




/* @func ajPhyloDistNew ******************************************************
**
** Constructor for AjPPhyloDist
**
** @return [AjPPhyloDist] AjPPhyloDist object
******************************************************************************/

AjPPhyloDist ajPhyloDistNew(void)
{
    AjPPhyloDist ret = NULL;

    AJNEW0(ret);

    return ret;
}




/* @func ajPhyloFreqNew ******************************************************
**
** Constructor for AjPPhyloFreq
**
** @return [AjPPhyloFreq] AjPPhyloFreq object
******************************************************************************/

AjPPhyloFreq ajPhyloFreqNew(void)
{
    AjPPhyloFreq ret = NULL;

    AJNEW0(ret);

    return ret;
}




/* @func ajPhyloPropNew ******************************************************
**
** Constructor for AjPPhyloProp
**
** @return [AjPPhyloProp] AjPPhyloProp object
******************************************************************************/

AjPPhyloProp ajPhyloPropNew(void)
{
    AjPPhyloProp ret = NULL;

    AJNEW0(ret);

    return ret;
}




/* @func ajPhyloStateNew *****************************************************
**
** Constructor for AjPPhyloState
**
** @return [AjPPhyloState] AjPPhyloState object
******************************************************************************/

AjPPhyloState ajPhyloStateNew(void)
{
    AjPPhyloState ret = NULL;

    AJNEW0(ret);

    return ret;
}




/* @func ajPhyloTreeNew ******************************************************
**
** Constructor for AjPPhyloTree
**
** @return [AjPPhyloTree] AjPPhyloTree object
******************************************************************************/

AjPPhyloTree ajPhyloTreeNew(void)
{
    AjPPhyloTree ret = NULL;

    AJNEW0(ret);

    return ret;
}




/* @func ajPhyloDistDel ******************************************************
**
** Destructor for AjPPhyloDist
**
** @param [d] pthis [AjPPhyloDist*] Distance object
** @return [void]
******************************************************************************/

void ajPhyloDistDel(AjPPhyloDist* pthis)
{
    AjPPhyloDist thys;
    ajint i;

    if(!pthis)
        return;

    thys = *pthis;

    if(!thys)
        return;

    if(thys->Names)
    {
	for(i=0; i < thys->Size; i++)
	    ajStrDel(&thys->Names[i]);

	AJFREE(thys->Names);
    }

    AJFREE(thys->Data);
    AJFREE(thys->Replicates);

    AJFREE(*pthis);

    return;
}




/* @func ajPhyloFreqDel ******************************************************
**
** Destructor for AjPPhyloFreq
**
** @param [d] pthis [AjPPhyloFreq*] frequency object
** @return [void]
******************************************************************************/

void ajPhyloFreqDel(AjPPhyloFreq* pthis)
{
    AjPPhyloFreq thys;
    ajint i;

    if(!pthis)
        return;

    thys = *pthis;

    if(!thys)
        return;

    if(thys->Names)
    {
	for(i=0; i < thys->Size; i++)
	    ajStrDel(&thys->Names[i]);

	AJFREE(thys->Names);
    }

    AJFREE(thys->Locus);
    AJFREE(thys->Allele);
    AJFREE(thys->Species);
    AJFREE(thys->Individuals);
    AJFREE(thys->Data);

    AJFREE(*pthis);

    return;
}




/* @func ajPhyloPropDel *******************************************************
**
** Destructor for AjPPhyloProp
**
** @param [d] pthis [AjPPhyloProp*] Properties object
** @return [void]
******************************************************************************/

void ajPhyloPropDel(AjPPhyloProp* pthis)
{
    AjPPhyloProp thys;
    ajint i;

    if(!pthis)
        return;

    thys = *pthis;

    if(!thys)
        return;

    if(thys->Str)
	for(i=0; i < thys->Size; i++)
	    ajStrDel(&thys->Str[i]);

    AJFREE(*pthis);

    return;
}




/* @func ajPhyloStateDel ******************************************************
**
** Destructor for AjPPhyloState
**
** @param [d] pthis [AjPPhyloState*] Discrete states object
** @return [void]
******************************************************************************/

void ajPhyloStateDel(AjPPhyloState* pthis)
{
    AjPPhyloState thys;
    ajint i;

    if(!pthis)
        return;

    thys = *pthis;

    if(!thys)
        return;

    if(thys->Names)
    {
	for(i=0; i < thys->Size; i++)
	    ajStrDel(&thys->Names[i]);

 	AJFREE(thys->Names);
   }

    if(thys->Str)
    {
	for(i=0; i < thys->Size; i++)
	    ajStrDel(&thys->Str[i]);

	AJFREE(thys->Str);
    }

    ajStrDel(&thys->Characters);
    AJFREE(*pthis);

    return;

}




/* @func ajPhyloStateDelarray *************************************************
**
** Destructor for AjPPhyloState array
**
** @param [d] pthis [AjPPhyloState**] States object array
** @return [void]
******************************************************************************/

void ajPhyloStateDelarray(AjPPhyloState** pthis)
{
    ajuint i = 0;

    if(!pthis)
        return;

    if(!*pthis)
        return;

    while((*pthis)[i])
    {
	ajPhyloStateDel(&(*pthis)[i]);
	i++;
    }

    AJFREE(*pthis);

    return;
}




/* @func ajPhyloTreeDel *******************************************************
**
** Destructor for AjPPhyloTree
**
** @param [d] pthis [AjPPhyloTree*] Trees object
** @return [void]
******************************************************************************/

void ajPhyloTreeDel(AjPPhyloTree* pthis)
{
    AjPPhyloTree thys;

    if(!pthis)
        return;

    thys = *pthis;

    if(!thys)
        return;

    ajStrDel(&thys->Tree);

    AJFREE(*pthis);

    return;
}




/* @func ajPhyloTreeDelarray **************************************************
**
** Destructor for AjPPhyloTree array
**
** @param [d] pthis [AjPPhyloTree**] Trees object array
** @return [void]
******************************************************************************/

void ajPhyloTreeDelarray(AjPPhyloTree** pthis)
{
    ajuint i = 0;

    if(!pthis)
        return;

    if(!*pthis)
        return;

    while((*pthis)[i])
    {
	ajPhyloTreeDel(&(*pthis)[i]);
	i++;
    }

    AJFREE(*pthis);

    return;
}




/* @func ajPhyloDistRead ******************************************************
**
** Reads phylogenetic distance matrix from a file
**
** @param [r] filename [const AjPStr] input filename
** @param [r] size [ajint] Number of rows in distance matrix
** @param [r] missing [AjBool] Missing values (replicates zero) allowed
** @return [AjPPhyloDist*] Phylogenetic distances object on success
**                        NULL on failure
******************************************************************************/

AjPPhyloDist* ajPhyloDistRead(const AjPStr filename, ajint size,
			      AjBool missing)
{
    AjPPhyloDist* ret   = NULL;
    AjPPhyloDist dist   = NULL;
    AjPFile distfile   = NULL;
    AjPList distlist  = NULL;
    AjPStr rdline      = NULL;
    ajint count;
    ajint irow = 0 ;
    ajint icol = 0;
    ajint i = 0;
    ajint j = 0;
    ajint k = 0;
    ajint ncells;
    float dval;
    ajint ival;
    ajint ipos = 0;
    ajint iend;
    ajint idiag;
    ajint jpos;
    AjBool done     = ajFalse;
    AjPStr tmpstr   = NULL;
    AjPStr tmpval   = NULL;
    AjBool lowertri = ajFalse;
    AjBool uppertri = ajFalse;
    AjPStr name = NULL;
    AjPStr rest = NULL;
    ajuint imaxlen;
    ajuint linenum = 0;
    
    if(!phyloRegDistFloat)
	phyloRegDistFloat =
	    ajRegCompC("^\\s*(-?[0-9]+[.]?[0-9]*)\\s+(([0-9]+)[^0-9.])?");

    distlist = ajListNew();
    distfile = ajFileNewInNameS(filename);

    if(!distfile)
	return NULL;

    done = ajTrue;
    while(ajReadline(distfile, &rdline))
    {
        linenum++;

        if(ajStrIsWhite(rdline))
            continue;

	if(done) {
            ajStrTrimWhite(&rdline);

            if(!ajStrToInt(rdline, &count))
            {
                ajErr("Distance file '%S' line %u bad header record '%S'",
                      filename, linenum, rdline);

                return NULL;
            }
            ajDebug("DistRead: row count: %d\n", count);

            if(size && (count != size))
            {
                ajErr("Distance file '%S' line %u "
                      "found %d expected %d rows '%S'",
                      filename, linenum, count, size);

                return NULL;
            }

	    irow = -1;
	    icol = 0;

	    i = 0;
	    done = ajFalse;

	    dist = ajPhyloDistNew();
	    dist->Size = count;
	    AJCNEW0(dist->Names, count);
	    ncells = count*count;
	    AJCNEW0(dist->Data,ncells);
	    AJCNEW0(dist->Replicates,ncells);

            continue;
	}

	/*ajDebug("DistRead line: %S", rdline); */

	if(!i || !ajRegExec(phyloRegDistFloat, rdline))
	{
	    if(!irow)		     /* how long was that first row */
	    {
		if(!icol)		/* empty - lower triangular */
		{
		    lowertri = ajTrue;
		    ajDebug("DistRead: ++Lower-Triangular form++\n");
		}
		else if(icol == (count - 1)) /* one short - upper triangular */
		{
		    uppertri = ajTrue;
		    ajDebug("DistRead: ++Upper-Triangular form++\n");

		    /* now we need to move along one */

		    for(j=count-1; j; j--)
		    {
			dist->Data[j] = dist->Data[j-1];
			dist->Replicates[j] = dist->Replicates[j-1];
		    }
		}
		else if(icol != count)	/* something wrong */
		{
		    ajErr("Distance file %S: line %u Row '%S' has %d values",
			  filename, linenum, dist->Names[irow], icol);
		}
	    }
	    else if(irow > 0) /* later rows - we know what to expect */
	    {
		if(lowertri)
		{
		    /*ajDebug("DistRead Lower irow: %d i: %d\n",
			    irow, i);*/
		    if(icol != irow)
			ajErr("Distance file %S: line %u Row '%S' has %d values"
			      " in lower-triangular format",
			      filename, linenum, dist->Names[irow], icol);
		}
		else if(uppertri)
		{
		    /*ajDebug("DistRead Upper irow: %d i: %d\n",
			    irow, i);*/
		    if(i)
			ajErr("Distance file %S: line %u Row '%S' has %d values"
			      " in upper-triangular format",
			      filename, linenum,
                              dist->Names[irow], icol - irow - 1);
		}
		else
		{
		    /*ajDebug("DistRead Square irow: %d i: %d\n",
			    irow, i);*/
		    if(icol != count)
			ajErr("Distance file %S: line %u Row '%S' has %d values"
			      " in square format",
			      filename, linenum, dist->Names[irow], icol);
		}
	    }

	    irow++;

	    if(irow > count)
	    {
		ajErr("Distances file line %u read beyond %d rows",
                      linenum, count);

		return NULL;
	    }
            ajStrExtractWord(rdline, &rest, &name);

            if(ajStrGetLen(name) >= 10)
                imaxlen = 10;
            else
                imaxlen = ajStrGetLen(name);
	    ajStrAssignSubS(&dist->Names[irow], rdline, 0, imaxlen-1);
                    
            ajStrCutStart(&rdline, imaxlen);
	    ajStrTrimWhiteEnd(&dist->Names[irow]);
	    ajDebug("DistRead name [%d] '%S' i: %d\n",
		    irow, dist->Names[irow], i);

	    if(uppertri)
	    {
		icol = irow + 1;
		i = irow + 1;
	    }
	    else
	    {
		i = 0;
		icol = 0;
	    }
	}

	while(ajRegExec(phyloRegDistFloat, rdline))
	{
	    ajRegSubI(phyloRegDistFloat, 1, &tmpval);
	    ajStrToFloat(tmpval, &dval);
	    ajRegSubI(phyloRegDistFloat, 3, &tmpval);

	    if(ajStrGetLen(tmpval))
	    {
		ajStrToInt(tmpval, &ival);

		if(ival)
		    dist->HasReplicates = ajTrue;
		else
		{
		    if(!missing)
		    {
			ajErr("Distances file %S: line %u "
                              "Zero (%S) replicates for %S",
			      filename, linenum, tmpval, dist->Names[irow]);

			return NULL;
		    }

		    dist->HasMissing = ajTrue;
		}
		/*ajDebug("DistRead row %2d [%d] %.3f %d\n",
			irow, i, dval, ival);*/
	    }
	    else
	    {
		ival = 1;
		/*ajDebug("DistRead row %2d [%d] %.3f .\n",
			irow, i, dval);*/
	    }
            if(dval < 0.0)
            {
                if(!missing)
                {
                    ajErr("Distances file %S: line %u "
                          "negative (%f) value for %S",
                          filename, linenum, dval, dist->Names[irow]);

                    return NULL;
                }

                dist->HasMissing = ajTrue;

                ival = 0;
                dval = 0.0;
            }

	    ipos = irow*count + i;
	    dist->Data[ipos] = dval;
	    dist->Replicates[ipos] = ival;
	    i++;
	    icol++;

	    if(i == count)
		i = 0;
	    ajRegPost(phyloRegDistFloat, &tmpstr);
	    ajStrAssignS(&rdline, tmpstr);
	    /*ajDebug("irow: %d icol: %d\n", irow, icol);*/
	}

	if(irow == (count-1))
	{
	    if(lowertri) /* fill in the rest */
	    {
		for(k=0; k < count; k++)
		{
		    idiag = k*count + k;	/* diagonal */
		    dist->Data[idiag] = 0.0;
		    dist->Replicates[idiag] = 1;
		    iend = k*count+count;	/* end of row */
		    ipos = idiag + 1;
		    jpos = ipos + count - 1;

		    while(ipos < iend)
		    {
			dist->Data[ipos] = dist->Data[jpos];
			dist->Replicates[ipos] = dist->Replicates[jpos];
			ipos++;
			jpos += count;
		    }
		}
                done = ajTrue;
	    }
	    else if(uppertri)			/* fill in the rest */
	    {
		for(k=0; k < count; k++)
		{
		    idiag = k*count + k;	/* diagonal */
		    dist->Data[idiag] = 0.0;
		    dist->Replicates[idiag] = 1;
		    ipos = k*count;		/* start of row*/
		    jpos = k;

		    while(ipos < idiag)
		    {
			dist->Data[ipos] = dist->Data[jpos];
			dist->Replicates[ipos] = dist->Replicates[jpos];
			ipos++;
			jpos += count;
		    }
		}
                done = ajTrue;
	    }
            else if(icol == count)
            {
                done = ajTrue;
            }
        
	    if(done)
            {
                ajListPushAppend(distlist, dist);
                dist = NULL;
            }
	}
    }

    if(irow != (count-1))
    {
	ajErr("Distances file found %d rows, expected %d", irow, count);

	return NULL;
    }

    ajDebug("Distances file '%S' has %u distance matrices\n",
	    filename, ajListGetLength(distlist));
    ajFileClose(&distfile);
    ajListToarray(distlist, (void***) &ret);
    /*ret = (AjPPhyloTree*) trees;*/
    ajListFree(&distlist);

    for(i=0; ret[i]; i++)
	ajPhyloDistTrace(ret[i]);

    ajStrDel(&name);
    ajStrDel(&rest);

    return ret;
}




/* @func ajPhyloDistTrace *****************************************************
**
** Reports phylogenetic distance data to the debug file
**
** @param [r] thys [const AjPPhyloDist] Phylogenetic frequencies object
** @return [void]
******************************************************************************/

void ajPhyloDistTrace(const AjPPhyloDist thys)
{
    ajint i;
    ajint j;
    ajint jend;

    ajDebug("ajPhyloDistTrace\n");
    ajDebug("================\n");

    ajDebug("  Count: %d HasReplicates: %B HasMissing: %B\n",
	    thys->Size, thys->HasReplicates, thys->HasMissing);

    jend = 0;
    j    = 0;

    ajDebug("%-10.10s", "Name");

    for(i=0; i < thys->Size; i++)
	ajDebug(" %6d ++", (i+1));

    ajDebug("\n");

    ajDebug("==========");

    for(i=0; i < thys->Size; i++)
	ajDebug(" ====== ==");

    ajDebug("\n");

    for(i=0; i < thys->Size; i++)
    {
	jend += thys->Size;
	ajDebug("%-10.10S", thys->Names[i]);

	while(j < jend)
	{
	    ajDebug(" %6.3f %2d",
		    thys->Data[j], thys->Replicates[j]);
	    j++;
	}
	ajDebug("\n");
    }

    return;
}




/* @func ajPhyloFreqRead ******************************************************
**
** Reads phylogenetic frequencies from a file
**
** @param [r] filename [const AjPStr] input filename
** @param [r] contchar [AjBool] Continuous character data expected
** @param [r] genedata [AjBool] Gene frequency data expected
** @param [r] indiv [AjBool] Multiple individuals for one name
**                           only for continuous character data.
** @return [AjPPhyloFreq] Phylogenetic frequencies object on success
**                        NULL on failure
******************************************************************************/

AjPPhyloFreq ajPhyloFreqRead(const AjPStr filename,
			     AjBool contchar, AjBool genedata, AjBool indiv)
{
    AjPPhyloFreq ret   = NULL;

    AjPFile freqfile = NULL;
    AjPStr rdline    = NULL;
    AjPStr tmpline   = NULL;
    AjPStr tmpstr    = NULL;
    AjPStr tmpval    = NULL;
    ajint count;
    ajint len;
    ajint ncells;
    ajint nfreq = 0;
    ajint i;
    ajint j;
    ajint k;
    ajint jnew;
    ajint jold;
    ajint misspos;
    ajint irow;
    ajint icol;
    ajint ipos = 0;
    ajint jpos = 0;
    float jtot;
    float jrest;
    AjBool done    = ajFalse;
    AjBool alldata = ajFalse;

    ajint totfreq;
    ajint totsp;
    ajint isp;

    float dval;
    float** pvals = NULL;

    (void) indiv;			/* make it used */

    if(!phyloRegFreqFloat)
	phyloRegFreqFloat = ajRegCompC("^\\s*([-]?[0-9]+[.]?[0-9]*)\\s+");

    if(!phyloRegFreqInt)
	phyloRegFreqInt = ajRegCompC("^\\s*([1-9][0-9]*)(\\s+[0-9\\s]*)$");

    if(!phyloRegFreqIndiv)
	phyloRegFreqIndiv =
	    ajRegCompC("^(\\S.........)\\s+([1-9][0-9]*)\\s*$");

    freqfile = ajFileNewInNameS(filename);

    if(!freqfile)
	return NULL;

    ajReadline(freqfile, &rdline);

    /* process header */

    if(!ajRegExec(phyloRegFreqInt, rdline))
    {
	ajErr("Frequencies file %S: Bad header line '%S'",
	      filename, rdline);

	return NULL;
    }

    ajRegSubI(phyloRegFreqInt, 1, &tmpval);
    ajStrToInt(tmpval, &count);
    ajRegSubI(phyloRegFreqInt, 2, &tmpstr);

    if(!ajRegExec(phyloRegFreqInt, tmpstr))
    {
	ajErr("Frequencies file %S: Bad header line '%S'",
	      filename, rdline);
	return NULL;
    }

    ajRegSubI(phyloRegFreqInt, 1, &tmpval);
    ajStrToInt(tmpval, &len);
 
    ret = ajPhyloFreqNew();
    ret->Size = count;
    ret->Loci = len;
    AJCNEW0(ret->Names,count);
    AJCNEW0(ret->Allele,len);

    /* process alleles counts, if any */

    ajReadline(freqfile, &rdline);

    if(ajRegExec(phyloRegFreqInt, rdline))	/* allele counts */
    {
	if(contchar)
	{
	    ajErr("Frequencies file %S: Has gene frequency data",
		  filename);

	    return NULL;
	}

	ajStrAssignS(&tmpline, rdline);

	for(i=0; i < len; i++)
	{
	    if(!ajRegExec(phyloRegFreqInt, rdline))
	    {
		ajErr("Frequencies file %S: Bad allele header,"
		      " expected %d allele counts, found %d",
		      filename, len, i);

		return NULL;
	    }

	    ajRegSubI(phyloRegFreqInt, 1, &tmpval);
	    ajStrToInt(tmpval, &ret->Allele[i]);
	    ajRegSubI(phyloRegFreqInt, 2, &tmpstr);
	    ajStrAssignS(&rdline, tmpstr);

	    nfreq += ret->Allele[i];	/* assume all values */
	}

	if(ajRegExec(phyloRegFreqInt, rdline))
	{
	    ajErr("Frequencies file %S: Bad allele header,"
		  " more than expected %d allele counts",
		  filename, len);

	    return NULL;
	}

	ret->Len = nfreq;
	AJCNEW0(ret->Locus,nfreq);
	jpos = 0;

	for(i=0; i < len; i++)
	    for(j=0; j < ret->Allele[i]; j++)
		ret->Locus[jpos++] = i;

	ncells = count*nfreq;
	AJCNEW0(ret->Data,ncells);
	ajReadline(freqfile, &rdline);
    }
    else
    {
	if(genedata)
	{
	    ajErr("Frequencies file %S: Has continuous character data",
		  filename);

	    return NULL;
	}

	ret->ContChar = ajTrue;
	alldata = ajTrue;
	nfreq = len;
	ncells = count*nfreq;
	ret->Len = nfreq;
	AJCNEW0(ret->Data,ncells);
	AJCNEW0(ret->Locus,nfreq);

	for(i=0; i < len; i++)
	{
	    ret->Locus[i] = i;
	    ret->Allele[i] = 2;
	}

	if(ajRegExec(phyloRegFreqIndiv, rdline))
	{
	    ret->Within = ajTrue;
	    AJCNEW0(ret->Individuals, count);
	    AJCNEW0(pvals, count);
	}
    }

    irow = -1;
    icol = 0;
    i    = 0;
    done = ajFalse;

    if(ret->Within)
    {
	totfreq = 0;
	totsp   = 0;

	while(!done)
	{
	    if(ajRegExec(phyloRegFreqIndiv, rdline))
	    {
		irow++;

		if(irow && icol != nfreq)
		    ajErr("Frequencies file %S: Read %d values"
			  " for '%S', expected %d",
			  filename, icol, ret->Names[irow], nfreq);

		ajRegSubI(phyloRegFreqIndiv, 1, &ret->Names[irow]);
		ajStrTrimWhiteEnd(&ret->Names[irow]);
		ajRegSubI(phyloRegFreqIndiv, 2, &tmpval);
		ajStrToInt(tmpval, &ret->Individuals[irow]);
		nfreq = ret->Individuals[irow]*len;
		totfreq += nfreq;
		totsp += ret->Individuals[irow];
		AJCNEW0(pvals[irow],ret->Individuals[irow]);
		icol = 0;
	    }
	    
	    while(ajRegExec(phyloRegFreqFloat, rdline))
	    {
		ajRegSubI(phyloRegFreqFloat, 1, &tmpval);
		ajStrToFloat(tmpval, &dval);
		pvals[irow][icol] = dval;
		ajRegPost(phyloRegFreqFloat, &tmpstr);
		ajStrAssignS(&rdline, tmpstr);
		icol++;
		ipos++;
	    }

	    if(!ajReadline(freqfile, &rdline))
		break;
	}

	if(irow != (count-1))
	{
	    ajErr("Frequencies file %S: found %d rows, expected %d",
		  filename, irow, count);

	    return NULL;
	}

	AJCNEW0(ret->Data, totfreq);
	AJCNEW0(ret->Species, totsp);
	isp  = 0;
	ipos = 0;

	for(i=0; i < count; i++)
	{
	    for(j=0; j < ret->Individuals[i]; j++)
		ret->Species[isp++] = i;

	    nfreq = ret->Individuals[i]*len;

	    for(j=0; j < nfreq; j++)
		ret->Data[ipos++] = pvals[i][j];

	    AJFREE(pvals[i]);
	}

	ajDebug("FreqRead: wrote %d values\n", ipos);
	AJFREE(pvals);
	ajPhyloFreqTrace(ret);

	return ret;
    }

    /* We read, for each 'species', a 10-character name and 'nfreq' values */

    while(!done)
    {
	/*ajDebug("FreqRead line: %S", rdline);*/
	if(!ajRegExec(phyloRegFreqFloat, rdline))
	{
	    irow++;

	    if(irow == 1)
	    {
		if(icol == nfreq)
		    alldata = ajTrue;
		else if(!ret->ContChar && icol == (nfreq - len))
		    alldata = ajFalse;
		else
		{
		    if(ret->ContChar)
			ajErr("Frequencies file %S: Read %d values,"
			      " expected %d",
			      filename, icol, nfreq);
		    else
			ajErr("Frequencies file %S: Read %d values,"
			      " expected %d or %d",
			      filename, icol, nfreq, (nfreq - len));

		    return NULL;
		}
	    }

	    if(irow > count)
	    {
		ajErr("Frequencies file read beyond %d rows", count);

		return NULL;
	    }

	    if(irow)
	    {
		if(alldata)
		{
		    if(icol != nfreq)
		    {
			ajErr("Frequencies file %S:"
			      " read only %d rows for %S",
			      filename, icol, ret->Names[irow-1]);

			return NULL;
		    }
		}
		else
		{
		    if(icol != (nfreq - len))
		    {
			ajErr("Frequencies file %S:"
			      " read only %d rows for %S",
			      filename, icol, ret->Names[irow-1]);

			return NULL;
		    }
		}
		
	    }

	    ajStrAssignSubS(&ret->Names[irow], rdline, 0, 9);
	    ajStrCutStart(&rdline, 10);
	    ajStrTrimWhiteEnd(&ret->Names[irow]);
	    ajDebug("FreqRead name [%d] '%S' i: %d\n",
		    irow, ret->Names[irow], i);
	    icol = 0;
	    ipos = irow * nfreq;
	}

	while(ajRegExec(phyloRegFreqFloat, rdline))
	{
	    ajRegSubI(phyloRegFreqFloat, 1, &tmpval);
	    ajStrToFloat(tmpval, &dval);
	    ret->Data[ipos] = dval;
	    ajRegPost(phyloRegFreqFloat, &tmpstr);
	    ajStrAssignS(&rdline, tmpstr);
	    icol++;
	    ipos++;
	}

	if(!ajReadline(freqfile, &rdline))
	    break;
    }

    if(irow != (count-1))
    {
	ajErr("Frequencies file %S: found %d rows, expected %d",
	      filename, irow, count);

	return NULL;
    }

    if(!alldata)		 /* fill in the missing frequencies */
    {
	for(i=0; i < count; i++)	/* for each row (Name) */
	{
	    misspos = i*nfreq + nfreq - 1;
	    jnew = misspos - 1;
	    jold = misspos - ret->Loci;

	    for(j=ret->Loci; j; j--)
	    {
		jtot = 0.0;

		for(k=1; k < ret->Allele[j-1]; k++)
		{
		    jtot += ret->Data[jold];
		    ret->Data[jnew--] = ret->Data[jold--];
		}
		jrest = (float) 1.0 - jtot;
		ret->Data[misspos] = jrest;
		misspos -= ret->Allele[j-1];
		jnew--;
	    }
	}
    }

    ajPhyloFreqTrace(ret);

    ajStrDel(&tmpline);
    ajStrDel(&tmpval);
    ajStrDel(&tmpstr);
    ajStrDel(&rdline);

    ajFileClose(&freqfile);

    return ret;
}




/* @func ajPhyloFreqTrace *****************************************************
**
** Reports phylogenetic frequencies data to the debug file
**
** @param [r] thys [const AjPPhyloFreq] Phylogenetic frequencies object
** @return [void]
******************************************************************************/

void ajPhyloFreqTrace(const AjPPhyloFreq thys)
{
    ajint i;
    ajint j;
    ajint k;
    ajint ipos;
    ajint isp;

    ajDebug("ajPhyloFreqTrace\n");
    ajDebug("================\n");

    ajDebug("  Count: %d Len: %d Loci: %d ContChar: %B Within: %B\n",
	    thys->Size, thys->Len, thys->Loci,
	    thys->ContChar, thys->Within);

    if(thys->ContChar)			/* no alleles */
    {
	ajDebug("  Continuous frequency data\n");
	ajDebug("  -------------------------\n");
	ajDebug("%-10.10s", "Name");

	for(j=0; j < thys->Len; j++)
	    ajDebug(" %7d", 1+thys->Locus[j]);

	ajDebug("\n");

	ajDebug("==========");

	for(j=0; j < thys->Len; j++)
	    ajDebug(" =======");

	ajDebug("\n");

	if(thys->Species)
	{
	    isp  = 0;
	    ipos = 0;

	    for(i=0; i < thys->Size; i++)
	    {
		for(k=0; k < thys->Individuals[i]; k++)
		{
		    if(k)
			ajDebug("sp.%3d %3d", thys->Species[isp]+1, k+1);
		    else
			ajDebug("%-10.10S", thys->Names[i]);

		    for(j=0; j < thys->Len; j++)
			ajDebug(" %7.3f", thys->Data[ipos++]);

		    ajDebug("\n");
		    isp++;
		}
	    }
	}
	else
	{
	    for(i=0; i < thys->Size; i++)
	    {
		ajDebug("%-10.10S", thys->Names[i]);
		ipos = i*thys->Len;

		for(j=0; j < thys->Len; j++)
		    ajDebug(" %7.3f", thys->Data[ipos++]);

		ajDebug("\n");
	    }
	}
    }
    else
    {
	ajDebug("  Allele frequency data\n");
	ajDebug("  ---------------------\n");

	for(j=0; j < thys->Loci; j++)
	    ajDebug("Locus %d Alleles %d\n", j+1, thys->Allele[j]);

	ajDebug("\n");

	ajDebug("%-10.10s", "Name");

	for(j=0; j < thys->Len; j++)
	    ajDebug(" Loc. %2d", 1+thys->Locus[j]);

	ajDebug("\n");

	ajDebug("==========");

	for(j=0; j < thys->Len; j++)
	    ajDebug(" =======");

	ajDebug("\n");

	for(i=0; i < thys->Size; i++)
	{
	    ajDebug("%-10.10S", thys->Names[i]);
	    ipos = i*thys->Len;

	    for(j=0; j < thys->Len; j++)
		ajDebug(" %7.3f", thys->Data[ipos++]);

	    ajDebug("\n");
	}
    }

    return;
}




/* @func ajPhyloPropRead ******************************************************
**
** Reads phylogenetic properties (weights, factors, ancestral states)
** from a file
**
** @param [r] filename [const AjPStr] input filename
** @param [r] propchars [const AjPStr] Valid property characters
** @param [r] len [ajint] Length of properties string
** @param [r] size [ajint] Number of property sets expected
**                         If zero, read first only or whatever the file says
** @return [AjPPhyloProp] Phylogenetic properties object on success
**                        NULL on failure
******************************************************************************/

AjPPhyloProp ajPhyloPropRead(const AjPStr filename, const AjPStr propchars,
			     ajint len, ajint size)
{
    AjPPhyloProp ret  = NULL;
    AjPFile propfile  = NULL;
    AjPList proplist  = NULL;
    AjBool propok     = ajFalse;
    AjPStr token      = NULL;
    void ** props     = NULL;
    ajint i;
    ajint ilen;
    const char* cp;
    AjPStr rdline  = NULL;
    AjPStr proppat = NULL;
    AjPStr propstr = NULL;
    ajint count;
    ajint dosize;

    ret = ajPhyloPropNew();

    if(ajStrMatchC(propchars, ""))
	ajFmtPrintS(&proppat, "\\S+");
    else
	ajFmtPrintS(&proppat, "[%S]+", propchars);

    ajRegFree(&phyloRegPropProp);
	
    phyloRegPropProp = ajRegComp(proppat);

    ajDebug("ajPhyloPropRead '%S' propchars: '%S' len: %d size: %d\n",
	    filename, propchars, len, size);
    proplist = ajListstrNew();
    dosize = size;

    if (size)
	count = size;
    else
	count = 1;

    propfile = ajFileNewInNameS(filename);

    if(!propfile)		/* read the filename string as data */
    {
	if (size > 1)
	{
	    ajErr("Bad properties string '%S':"
			  " valid filename required to read %d sets",
			  filename, size);

	    return NULL;
	}
	ajStrAssignS(&rdline, filename);

	/*ajStrFmtUpper(&rdline);*/ /* keep to catch names without '.' */
	if(!ajRegExec(phyloRegPropProp, rdline))
	    return NULL;

	ajRegSubI(phyloRegPropProp, 0, &token);

	if(!ajStrMatchS(rdline, token))
	    return NULL;

	dosize = 1;
	ilen = ajStrGetLen(token);

	if(ilen != len)
	{
	    ajErr("Bad properties string (not valid filename) '%S':"
		  " read %d properties, expected %d",
		  filename, ilen, len);

	    return NULL;
	}

	AJCNEW0(ret->Str, 2);
	ajStrAssignS(&ret->Str[0], token);

	ret->Size = 1;
	ret->Len = ilen;
    }

    else				/* read data from the file */
    {
	i = 0;

	while (!dosize || i < count)
	{
	    propstr = ajStrNewRes(len+1);
	    propok = ajFalse;
	    ilen = 0;

	    while(!propok && ajReadlineTrim(propfile, &rdline))
	    {
		ajStrFmtUpper(&rdline);
		cp = ajStrGetPtr(rdline);

		while(cp && ajRegExecC(phyloRegPropProp, cp))
		{
		    ajRegSubI(phyloRegPropProp, 0, &token);
		    ajStrAppendS(&propstr, token);
		    ilen += ajStrGetLen(token);
		    ajRegPostC(phyloRegPropProp, &cp);
		}

		if(ilen == len)
		    propok = ajTrue;
		else if(ilen > len)
		    ajErr("Bad properties file '%S':"
			  " read %d properties, expected %d",
			  filename, ilen, len);
	    }

	    if(!propok)
	    {
		if (ilen)
		    ajErr("End of properties file '%S':"
			  " after %d properties, expected %d",
			  filename, ilen, len);
		else if (size)
		    ajErr("End of properties file '%S':"
			  " after %d sets, expected %d",
			  filename, i, size);
		else if (ajFileIsEof(propfile))
		    break;
	    }

	    ajListstrPushAppend(proplist, propstr);
	    i++;
	}

	ajFileClose(&propfile);
	ajListToarray(proplist, (void***) &props);
	ret->Str = (AjPStr*) props;

	ret->Size = ajListGetLength(proplist);;
	ret->Len = len;
    }

    ajListFree(&proplist);
    ajStrDel(&proppat);
    ajStrDel(&token);
    ajStrDel(&rdline);

    ajPhyloPropTrace(ret);

    return ret;
}




/* @func ajPhyloPropGetSize ***************************************************
**
** Returns size (number of property strings)
**
** @param [r] thys [const AjPPhyloProp] Properties object
** @return [ajint] Number of property strings
******************************************************************************/

ajint ajPhyloPropGetSize(const AjPPhyloProp thys)
{
    if(!thys)
	return 0;

    return thys->Size;
}




/* @func ajPhyloPropTrace *****************************************************
**
** Reports phylogenetic property data to the debug file
**
** @param [r] thys [const AjPPhyloProp] Phylogenetic frequencies object
** @return [void]
******************************************************************************/

void ajPhyloPropTrace(const AjPPhyloProp thys)
{
    ajint i;

    ajDebug("ajPhyloPropTrace\n");
    ajDebug("================\n");

    ajDebug("  Len: %d  Size: %d IsWeight: %B IsFactor: %B\n",
	    thys->Len, thys->Size, thys->IsWeight, thys->IsFactor);

    for(i=0; i < thys->Size; i++)
    {
	ajDebug("%3d: '%S'",i,  thys->Str[i]);
	ajDebug("\n");
    }

    return;
}




/* @func ajPhyloStateRead *****************************************************
**
** Reads phylogenetic discrete states from a file
**
** @param [r] filename [const AjPStr] input filename
** @param [r] statechars [const AjPStr] Valid state characters
** @return [AjPPhyloState*] Phylogenetic discrete states object on success
**                        NULL on failure
******************************************************************************/

AjPPhyloState* ajPhyloStateRead(const AjPStr filename, const AjPStr statechars)
{
    AjPPhyloState* ret  = NULL;
    AjPPhyloState state = NULL;

    AjPFile statefile = NULL;
    AjPList statelist = NULL;
    AjPStr rdline     = NULL;
    AjPStr tmpstr     = NULL;
    AjPStr tmpval     = NULL;
    AjPStr charpat    = NULL;
    AjPStr token      = NULL;

    ajint size;
    ajint len;
    ajint count;
    ajint i;
    ajint j;
    ajint ilen;
    const char* cp;
    void **states = NULL;

    if(!phyloRegStateInt)
	phyloRegStateInt =
	    ajRegCompC("^\\s+([1-9][0-9]*)\\s+([1-9][0-9]*)"
		       "\\s+([1-9][0-9]*)?\\s*$");

    if(!phyloRegStateState)
	phyloRegStateState = ajRegCompC("^(\\S.........)\\s*(\\S.*)$");

    if(ajStrMatchC(statechars, ""))
	ajFmtPrintS(&charpat, "\\S+");
    else
	ajFmtPrintS(&charpat, "[%S]+", statechars);

    ajRegFree(&phyloRegStateChar);
    phyloRegStateChar = ajRegComp(charpat);

    statelist = ajListNew();

    statefile = ajFileNewInNameS(filename);

    if(!statefile)
	return NULL;

    while(ajReadline(statefile, &rdline))
    {
	
	if(!ajRegExec(phyloRegStateInt, rdline))
	{
	    ajErr("Discrete states file %S: Bad header line '%S'",
		  filename, rdline);

	    return NULL;
	}
	
	ajRegSubI(phyloRegStateInt, 1, &tmpval);
	ajStrToInt(tmpval, &size);
	
	ajRegSubI(phyloRegStateInt, 2, &tmpval);
	ajStrToInt(tmpval, &len);
	
	ajRegSubI(phyloRegStateInt, 3, &tmpval);
	if(ajStrGetLen(tmpval))
	    ajStrToInt(tmpval, &count);
	else
	    count = 1;
	
	state = ajPhyloStateNew();
	state->Size = size;
	state->Len = len;
	state->Count = count;
	AJCNEW0(state->Names,size);
	AJCNEW0(state->Str,size);
	
	/* process alleles counts, if any */
	
	ilen = 0;
	i    = 0;

	while(ajReadline(statefile, &rdline))
	{
	    if(ilen == 0 && ajRegExec(phyloRegStateState, rdline))
	    {
		ajRegSubI(phyloRegStateState, 1, &state->Names[i]);
		ajStrTrimWhiteEnd(&state->Names[i]);
		ajRegSubI(phyloRegStateState, 2, &tmpstr);
		ajStrAssignS(&rdline, tmpstr);
	    }
	    
	    ajStrFmtUpper(&rdline);
	    cp = ajStrGetPtr(rdline);
	    j = 0;

	    while(cp && ajRegExecC(phyloRegStateChar, cp))
	    {
		ajRegSubI(phyloRegStateChar, 0, &token);
		ajRegPre(phyloRegStateChar, &tmpstr);
		ajStrAppendS(&state->Str[i], token);
		ilen += ajStrGetLen(token);
		ajRegPostC(phyloRegStateChar, &cp);
		if(!j++ && !ajStrIsWhite(tmpstr))
		{
		    ajStrRemoveWhite(&tmpstr);
		    ajErr("found bad state character(s) '%S' for '%S', "
                          "expected [%S]",
			  tmpstr, state->Names[i], statechars);
		}
		if(*cp && !isspace(*cp))
		    ajErr("found bad state character '%c' for '%S', "
                          "expected [%S]",
			  *cp, state->Names[i], statechars);
	    }

	    if(ilen == len)
	    {
		ilen = 0;
		i++;
	    }
	    else if(ilen > len)
	    {
		ajErr("Bad discrete states file b '%S':"
		      " read %d states for '%S', expected %d",
		      filename, ilen, state->Names[i], len);

		return NULL;
	    }
	}
	
	if(i != size)
	{
	    ajErr("Bad discrete states file '%S':"
		  " read %d species, expected %d",
		  filename, i, len);

	    return NULL;
	}

	ajListPushAppend(statelist, state);
    }

    ajFileClose(&statefile);
    ajListToarray(statelist, (void***) &states);
    ret = (AjPPhyloState*) states;

    ajListFree(&statelist);
    ajStrDel(&tmpval);
    ajStrDel(&tmpstr);
    ajStrDel(&rdline);
    ajStrDel(&charpat);
    ajStrDel(&token);

    return ret;
}




/* @func ajPhyloStateTrace ****************************************************
**
** Reports phylogenetic discrete state data to the debug file
**
** @param [r] thys [const AjPPhyloState] Phylogenetic discrete states object
** @return [void]
******************************************************************************/

void ajPhyloStateTrace(const AjPPhyloState thys)
{
    ajint i;

    ajDebug("ajPhyloStateTrace\n");
    ajDebug("=================\n");

    ajDebug("  Len: %d  Size: %d Count: %d Characters: '%S'\n",
	    thys->Len, thys->Size, thys->Count, thys->Characters);

    for(i=0; i < thys->Size; i++)
	ajDebug("%S: %S\n", thys->Names[i], thys->Str[i]);

    return;
}




/* @func ajPhyloTreeRead ******************************************************
**
** Reads phylogenetic trees from a file.
**
** For background see the NESCent Hackathon Wiki
** https://www.nescent.org/wg_phyloinformatics/Main_Page
**
** Programs using or producing trees include:
**
** Editors in http://bioinfo.unice.fr/biodiv/Tree_editors.html
**
** Packages PHYLIP, GARLI, MrBayes, PROTML, TREE-PUZZLE
** PAUP
**
** Tree formats include:
**
** Newick (New Hampshire) is a simple string with parentheses
** see http://evolution.genetics.washington.edu/phylip/newicktree.html
** and http://evolution.genetics.washington.edu/phylip/newick_doc.html
** Newick format is used by PHYLIP, GARLI, MrBayes, PROTML, TREE-PUZZLE
** In PAUP the format is called "phylip"
** Phylip includes weights for multiple input trees as a final comment
** containing only a floating point number e.g. [0.5];
**
** Nexus files use Translate to define numbers, and then have a
** Newick string using the numbers, or simply define the Newick string
** with names. The branch lengths appear as in Newick.
** http://workshop.molecularevolution.org/resources/fileformats/tree_formats.php
** Test files (including malformed ones) are available from
** https://www.nescent.org/wg_phyloinformatics/Supporting_NEXUS
**
** nhx (New Hampshire Extended) includes annotation using specially formatted
** comments after the name:length in the form [&&NHX:code=value;code=value]
** where codes are documented in
** http://www.phylosoft.org/forester/NHX.html
** according to some documentation, ATV and FORESTER can read NHX with the []
** removed but this is deprecated (naturally, as this would be a valid Newick
** tree with proper comments).
** NHX also bans the use of empty nodes so names are required.
** Apparently PHYLIP can read trees with no names.
**
** svggraph is an output format only
** (or at least, no sign of anything reading it)
**
** tabtree is an ASCII text representation used for output only by
** BioPerl::TreeIO
**
** lintree is a program output format. The program is available from
** http://www.bio.psu.edu/People/Faculty/Nei/Lab/lintre/lintre.unix.tar.Z
**
** phyloXML is an effort to define an XML format, still in progress
** http://www.phyloxml.org/
**
** TGF treegraph format
** http://www.math.uni-bonn.de/people/jmueller/extra/treegraph/docu.pdf
**
** TGF TreeDyn format ... not the same as treegraph
** http://www.treedyn.org/
**
** @param [r] filename [const AjPStr] input filename
** @param [r] size [ajint] Number of trees expected
**                         If zero, read all trees
** @return [AjPPhyloTree*] Phylogenetic tree object array on success
**                        NULL on failure
******************************************************************************/

AjPPhyloTree* ajPhyloTreeRead(const AjPStr filename, ajint size)
{
    AjPPhyloTree* ret = NULL;
    AjPPhyloTree tree = NULL;
    /*AjPTree mytree = NULL;*/             /* only for tracing the tree */
    AjPFile treefile  = NULL;
    AjPList treelist  = NULL;
    AjBool treeok;
    ajint i;
    ajint count;
    ajint headcount = 0;
    const char* cp;
    AjPStr rdline    = NULL;
    AjPStr token     = NULL;
    AjPStr treecopy  = NULL;
    AjPStr pretoken  = NULL;
    AjPStr posttoken = NULL;

    
    if(!phyloRegTreeTree)		/* tree definition */
	phyloRegTreeTree = ajRegCompC("\\S+");
    if(!phyloRegTreeRoot)		/* (a,b) node to be trimmed */
	phyloRegTreeRoot = ajRegCompC("^(.+)(\\([^\\)]+\\))");
    if(!phyloRegTreeUnroot)	     /* remaining unrooted (a,b,c); */
	phyloRegTreeUnroot = ajRegCompC("^\\([^,]*,[^,]*,.*\\);$");
    if(!phyloRegTreeMulti)	 /* multifurcating (a,b,c) anywhere */
	phyloRegTreeMulti = ajRegCompC("^\\([^,]*,[^,]*,.*\\)$");
    if(!phyloRegTreeQuart)		 /* unrooted quartet ((a,b),(c,d)); */
	phyloRegTreeQuart = ajRegCompC("^\\(\\.*\\)\\);$");
    if(!phyloRegTreeLength)	 /* unrooted quartet ((a,b),(c,d)); */
	phyloRegTreeLength = ajRegCompC(":[0-9][0-9.]*");

    treelist = ajListNew();
    count = size;

    if(count < 1)
	count = 1;

    treefile = ajFileNewInNameS(filename);

    if(!treefile)
	return NULL;

    if(treefile)
    {
        treeok = ajFalse;
	i = 0;

	while(!treeok)
	{
	    ajDebug("ajPhyloTreeRead i: %d count: %d size: %d\n",
		    i, count, size);
	    tree = ajPhyloTreeNew();

	    while(ajReadlineTrim(treefile, &rdline))
	    {
		if(!i && !ajStrGetLen(tree->Tree))
		{
		    if(ajStrToInt(rdline, &headcount))
		    {
			ajDebug("ajPhyloTreeRead count: %d\n", headcount);
			if(size)
			{
			    if(size < headcount)
				ajWarn("Tree file '%S' has %d trees,"
				       " expected %d",
				       filename, headcount, size);
			    if(size > headcount)
				ajErr("Tree file '%S' has %d trees,"
				      " required %d",
				      filename, headcount, size);
			}

			ajReadlineTrim(treefile, &rdline);
			count = headcount;
		    }
		}

		cp = ajStrGetPtr(rdline);
		/*ajDebug("ajPhyloTreeRead rdline '%S'\n", rdline);*/

		while(cp && ajRegExecC(phyloRegTreeTree, cp))
		{
		    if(!size && !headcount && !ajStrGetLen(tree->Tree))
		    {
			count++;
			ajDebug("ajPhyloTreeRead count++ %d\n", count);
		    }

		    ajRegSubI(phyloRegTreeTree, 0, &token);
		    ajStrAppendS(&tree->Tree, token);
		    ajDebug("ajPhyloTreeRead token '%S'\n", token);
		    ajRegPostC(phyloRegTreeTree, &cp);
		}

		if(ajStrGetCharLast(tree->Tree) == ';')
		    break;
		ajDebug("ajPhyloTreeRead processing tree->Tree '%S'\n",
			tree->Tree);
	    }

	    if(ajStrGetLen(tree->Tree) && ajStrGetCharLast(tree->Tree) == ';')
	    {
		ajDebug("ajPhyloTreeRead tree done tree->Tree '%S'\n",
			tree->Tree);
		tree->Size = 1 + ajStrCalcCountK(tree->Tree, ',');
		tree->BaseBifurcated = ajTrue;

		if(ajRegExec(phyloRegTreeQuart, tree->Tree))
		{
		    tree->BaseBifurcated = ajFalse; /* but rooted for phylip */
		    tree->BaseQuartet = ajTrue;
		}

		if(ajRegExec(phyloRegTreeLength, tree->Tree))
		{
		    tree->HasLengths = ajTrue;
		}

		tree->Multifurcated = ajFalse;
		ajStrAssignS(&treecopy, tree->Tree);

		while(ajRegExec(phyloRegTreeRoot, treecopy))
		{
		    ajRegSubI(phyloRegTreeRoot, 1, &pretoken);
		    ajRegSubI(phyloRegTreeRoot, 2, &token);
		    ajRegPost(phyloRegTreeRoot, &posttoken);
		    ajDebug("ajPhyloTreeRead root match '%S': '%S'\n",
			    treecopy, token);

		    if(ajRegExec(phyloRegTreeMulti, token))
		    {
			tree->Multifurcated = ajTrue;
			ajDebug("ajPhyloTree multifurcated node '%S'\n",
				token);
		    }

		    ajStrAssignS(&treecopy, pretoken);
		    ajStrAppendS(&treecopy, posttoken);
		}

		if(ajRegExec(phyloRegTreeUnroot, treecopy))
		    tree->BaseBifurcated = ajFalse;
		ajDebug("ajPhyloTreeRead tree '%S' rooted: %B "
			"basetrifurcated: %B"
			" treecopy: '%S'\n",
			tree->Tree, tree->BaseBifurcated,
			tree->BaseTrifurcated, treecopy);
		ajListPushAppend(treelist, tree);
		tree = NULL;

		i++;

		if(i == headcount)
		    treeok = ajTrue;
		else if(size && size == i)
		    treeok = ajTrue;
	    }
	    else
	    {
		if(ajFileIsEof(treefile))
		    treeok = ajTrue;
		else
		{
		    ajErr("Tree file %S: Unexpected end of file",
			  filename);

		    return NULL;
		}
	    }
	}

	if(size && size != i)
	{
	    ajErr("Tree file '%S' has %d trees,"
		  " required %d",
		  filename, i, size);

	    return NULL;
	}

	ajDebug("Tree file '%S' has %d (%d) trees,"
		" required %d\n",
		  filename, i, ajListGetLength(treelist), size);
        ajFileClose(&treefile);
	ajListToarray(treelist, (void***) &ret);
	/*ret = (AjPPhyloTree*) trees;*/
	ajListFree(&treelist);
    }

    /* this code block fails to clean up the temporary tree data */
    /*
    for(i=0;ret[i];i++)
    {
	mytree = ajTreeNewNewick(ret[i]->Tree);
	ajTreeTrace(mytree);
	ajTreeFree(&mytree);
    }
    */

    ajStrDel(&rdline);
    ajStrDel(&token);
    ajStrDel(&treecopy);
    ajStrDel(&pretoken);
    ajStrDel(&posttoken);
    ajPhyloTreeDel(&tree);

    return ret;
}




/* @func ajPhyloTreeTrace ****************************************************
**
** Reports phylogenetic discrete state data to the debug file
**
** @param [r] thys [const AjPPhyloTree] Phylogenetic discrete states object
** @return [void]
******************************************************************************/

void ajPhyloTreeTrace(const AjPPhyloTree thys)
{
    ajDebug("ajPhyloTreeTrace\n");
    ajDebug("================\n");

    ajDebug("  Multifurcated: %B BaseTrifurcated: %B BaseBifurcated: %B"
	    " BaseQuartet: %B Tree: '%S'\n",
	    thys->Multifurcated, thys->BaseTrifurcated,
	    thys->BaseBifurcated, thys->BaseQuartet,
	    thys->Tree);

    return;
}




/* @func ajPhyloExit ********************************************************
**
** Cleans up phylogenetic data internal memory
**
** @return [void]
** @@
******************************************************************************/

void ajPhyloExit(void)
{
    ajRegFree(&phyloRegDistFloat);
    ajRegFree(&phyloRegFreqFloat);
    ajRegFree(&phyloRegFreqInt);
    ajRegFree(&phyloRegFreqIndiv);
    ajRegFree(&phyloRegPropProp);
    ajRegFree(&phyloRegStateState);
    ajRegFree(&phyloRegStateInt);
    ajRegFree(&phyloRegStateChar);
    ajRegFree(&phyloRegTreeTree);
    ajRegFree(&phyloRegTreeRoot);
    ajRegFree(&phyloRegTreeUnroot);
    ajRegFree(&phyloRegTreeMulti);
    ajRegFree(&phyloRegTreeQuart);
    ajRegFree(&phyloRegTreeLength);

    return;
}
