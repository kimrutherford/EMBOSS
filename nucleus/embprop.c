/* @source embprop.c
**
** Residue/sequence properties
** @author Copyright (c) 1999 Alan Bleasby
** @version 1.0
** @modified 24 Nov 1999 - GWW - Added embPropProtGaps and embPropProt1to3
** @modified 1 Sept 2000 - GWW - Added embPropTransition embPropTranversion
** @modified 4 July 2001 - DMAM - Modified embPropAminoRead embPropCalcMolwt
** @modified 4 July 2001 - DMAM - Added embPropCalcMolwtMod
** @modified 1 July 2008 - JISON - Added embPropGet* functions
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
#include <stdio.h>
#include <math.h>
#include <string.h>
#include <ctype.h>

char dayhoffstr[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZ";

float dayhoff[] =
{
    (float) 8.6, (float) 0.0, (float) 2.9, (float) 5.5, (float) 6.0,
    (float) 3.6, (float) 8.4, (float) 2.0, (float) 4.5, (float) 0.0,
    (float) 6.6, (float) 7.4, (float) 1.7, (float) 4.3, (float) 0.0,
    (float) 5.2, (float) 3.9, (float) 4.9, (float) 7.0, (float) 6.1,
    (float) 0.0, (float) 6.6, (float) 1.3, (float) 0.9, (float) 3.4,
    (float) 0.0
};



#define PROPENZTRYPSIN 0
#define PROPENZLYSC    1
#define PROPENZARGC    2
#define PROPENZASPN    3
#define PROPENZV8B     4
#define PROPENZV8P     5
#define PROPENZCHYMOT  6
#define PROPENZCNBR    7

#define RAG_MINPEPLEN 3

#define AMINODATFILE "Eamino.dat"


/* static AjBool propInit = 0;*/

static char propPurines[]     = "agrAGR";
static char propPyrimidines[] = "ctuyCTUY";




static ajint propFragCompare(const void *a, const void *b);




/* @func embPropEaminoRead ****************************************************
**
** Read amino acid properties from Eamino.dat
**
** @param [u] mfptr [AjPFile] Input file object
** @return [EmbPPropAmino*] array of amino acid properties
** @@
******************************************************************************/

EmbPPropAmino* embPropEaminoRead(AjPFile mfptr)
{
    AjPStr  line  = NULL;
    AjPStr  token = NULL;
    
    AjBool firstline;
    
    const char *p;
    ajuint i;
    ajint  n;
    
    EmbPPropAmino *ret;

    line  = ajStrNew();
    token = ajStrNew();
    
    firstline = ajTrue;

    AJCNEW0(ret,EMBPROPSIZE);

    for(i=0; i < EMBPROPSIZE; ++i)
	AJNEW0(ret[i]);
    
    while(ajReadline(mfptr, &line))
    {
	ajStrRemoveWhiteExcess(&line);
	p = ajStrGetPtr(line);

	if(*p=='#' || *p=='!' || !*p)
	    continue;

	if(firstline)
	{
	    if(!ajStrPrefixC(line,"aa"))
		ajFatal("Incorrect (old?) format amino data file");

	    firstline = ajFalse;
	    continue;
	}

	ajFmtScanS(line,"%S",&token);
	ajStrFmtUpper(&token);

	if(ajStrGetLen(token) != 1)
	    ajFatal("Amino file line doesn't begin with a single character");

	i = ajBasecodeToInt((ajint) *ajStrGetPtr(token));

	if(i == 27)
	    ajFatal("Amino file line doesn't begin with a single A->Z (%S)",
		    line);

	n = ajFmtScanS(line,"%*s%d%d%d%d%d%d%f%d%d%d",
		       &ret[i]->tiny,
		       &ret[i]->sm_all,
		       &ret[i]->aliphatic,
		       &ret[i]->aromatic,
		       &ret[i]->nonpolar,
		       &ret[i]->polar,
		       &ret[i]->charge,
		       &ret[i]->pve,
		       &ret[i]->nve,
		       &ret[i]->extcoeff);
	if(n!= 10)
	    ajFatal("Only %d columns in amino file - expected %d",n+1,11);
    }

    ajStrDel(&line);
    ajStrDel(&token);

    return ret;
}




/* @func embPropGetProperties **************************************************
**
** Returns a string containing a list of defined properties
**
** @param [r] prop [const EmbPPropAmino] Input properties object
** @param [w] Pstr [AjPStr*] String of properties separated by commas
** @return [AjBool] True if properties are defined
** @@
******************************************************************************/

AjBool embPropGetProperties(const EmbPPropAmino prop, AjPStr* Pstr)
{
    ajStrAssignC(Pstr, "");

    if(prop->tiny)
        ajStrAppendC(Pstr, "tiny,");

    if(prop->sm_all)
        ajStrAppendC(Pstr, "small,");

    if(prop->aliphatic)
        ajStrAppendC(Pstr, "aliphatic,");

    if(prop->aromatic)
        ajStrAppendC(Pstr, "aromatic,");

    if(prop->polar)
        ajStrAppendC(Pstr, "polar,");

    if(prop->nonpolar)
        ajStrAppendC(Pstr, "nonpolar,");

    ajStrTrimEndC(Pstr, ",");

    if(!ajStrGetLen(*Pstr))
        return ajFalse;

    return ajTrue;
}




/* @func embPropEmolwtRead ****************************************************
**
** Read molecular weights from Emolwt.dat
**
** @param [u] mfptr [AjPFile] Input file object
** @return [EmbPPropMolwt*] array of amino acid molecular weights
** @@
******************************************************************************/

EmbPPropMolwt* embPropEmolwtRead(AjPFile mfptr)
{
    AjPStr  line  = NULL;
    AjPStr  token = NULL;
    
    AjBool firstline;
    
    const char *p;
    ajuint i;
    ajint  n;
    
    EmbPPropMolwt *ret;

    line  = ajStrNew();
    token = ajStrNew();
    
    firstline = ajTrue;

    AJCNEW0(ret,EMBPROPSIZE+2);

    for(i=0; i < EMBPROPSIZE+2; ++i)
	AJNEW0(ret[i]);
    
    while(ajReadline(mfptr, &line))
    {
	ajStrRemoveWhiteExcess(&line);
	p = ajStrGetPtr(line);

	if(*p=='#' || *p=='!' || !*p)
	    continue;

	if(firstline)
	{
	    if(!ajStrPrefixC(line,"Mol"))
		ajFatal("Incorrect format molwt file: '%S'", line);

	    firstline = ajFalse;
	    continue;
	}

	ajFmtScanS(line,"%S",&token);
	ajStrFmtUpper(&token);

	if(ajStrGetLen(token) != 1)
	{
	    if(ajStrPrefixC(token,"HYDROGEN"))
	    {
		if(ajFmtScanS(line,"%*s%lf%lf",
			      &ret[EMBPROPHINDEX]->average,
			      &ret[EMBPROPHINDEX]->mono) != 2)
		    ajFatal("Bad format hydrogen data line");
	    }
	    else if(ajStrPrefixC(token,"OXYGEN"))
	    {
		if(ajFmtScanS(line,"%*s%lf%lf",
			      &ret[EMBPROPOINDEX]->average,
			      &ret[EMBPROPOINDEX]->mono) != 2)
		    ajFatal("Bad format oxygen data line");
	    }
	    else if(ajStrPrefixC(token,"WATER"))
	    {
		if(ajFmtScanS(line,"%*s%lf%lf",
			      &ret[EMBPROPWINDEX]->average,
			      &ret[EMBPROPWINDEX]->mono) != 2)
		    ajFatal("Bad format water data line");
	    }
	    else
		ajFatal("Unknown molwt token %S",token);

	    continue;
	}


	i = ajBasecodeToInt((ajint) *ajStrGetPtr(token));

	if(i == 27)
	    ajFatal("Molwt file line doesn't begin with a single A->Z (%S)",
		    line);

	n = ajFmtScanS(line,"%*s%lf%lf",
		       &ret[i]->average,
		       &ret[i]->mono);
	if(n != 2)
	    ajFatal("Only %d columns in amino file - expected %d",n,3);
    }

    ajStrDel(&line);
    ajStrDel(&token);

    return ret;
}




/* @func embPropMolwtGetMolwt *************************************************
**
** Return charge value
**
** @param [r] prop [const EmbPPropMolwt] Input mOlecular weights object
** @return [float] charge
** @@
******************************************************************************/

float embPropMolwtGetMolwt(const EmbPPropMolwt prop)
{
    float ret;

    ret = (float) prop->average; /* satisfy VC++ */
    
    return ret;
}




/* @func embPropGetCharge *****************************************************
**
** Return charge value
**
** @param [r] prop [const EmbPPropAmino] Input properties object
** @return [float] charge
** @@
******************************************************************************/

float embPropGetCharge(const EmbPPropAmino prop)
{
    return prop->charge;
}




/* @func embPropGetTiny *****************************************************
**
** Return tiny value
**
** @param [r] prop [const EmbPPropAmino] Input properties object
** @return [ajint] tiny
** @@
******************************************************************************/

ajint embPropGetTiny(const EmbPPropAmino prop)
{
    return prop->tiny;
}




/* @func embPropGetSmall *****************************************************
**
** Return small value
**
** @param [r] prop [const EmbPPropAmino] Input properties object
** @return [ajint] small
** @@
******************************************************************************/

ajint embPropGetSmall(const EmbPPropAmino prop)
{
    return prop->sm_all;
}




/* @func embPropGetAliphatic **************************************************
**
** Return aliphatic value
**
** @param [r] prop [const EmbPPropAmino] Input properties object
** @return [ajint] aliphatic
** @@
******************************************************************************/

ajint embPropGetAliphatic(const EmbPPropAmino prop)
{
    return prop->aliphatic;
}




/* @func embPropGetAromatic ***************************************************
**
** Return aromatic value
**
** @param [r] prop [const EmbPPropAmino] Input properties object
** @return [ajint] aromatic
** @@
******************************************************************************/

ajint embPropGetAromatic(const EmbPPropAmino prop)
{
    return prop->aromatic;
}




/* @func embPropGetNonpolar ***************************************************
**
** Return nonpolar value
**
** @param [r] prop [const EmbPPropAmino] Input properties object
** @return [ajint] nonpolar
** @@
******************************************************************************/

ajint embPropGetNonpolar(const EmbPPropAmino prop)
{
    return prop->nonpolar;
}




/* @func embPropGetPolar *****************************************************
**
** Return polar value
**
** @param [r] prop [const EmbPPropAmino] Input properties object
** @return [ajint] polar
** @@
******************************************************************************/

ajint embPropGetPolar(const EmbPPropAmino prop)
{
    return prop->polar;
}




/* @func embPropGetPve *****************************************************
**
** Return pve value
**
** @param [r] prop [const EmbPPropAmino] Input properties object
** @return [ajint] pve
** @@
******************************************************************************/

ajint embPropGetPve(const EmbPPropAmino prop)
{
    return prop->pve;
}




/* @func embPropGetNve *****************************************************
**
** Return nve value
**
** @param [r] prop [const EmbPPropAmino] Input properties object
** @return [ajint] nve
** @@
******************************************************************************/

ajint embPropGetNve(const EmbPPropAmino prop)
{
    return prop->nve;
}




/* @func embPropGetExtcoeff ***************************************************
**
** Return extcoeff value
**
** @param [r] prop [const EmbPPropAmino] Input properties object
** @return [ajint] extcoeff
** @@
******************************************************************************/

ajint embPropGetExtcoeff(const EmbPPropAmino prop)
{
    return prop->extcoeff;
}




/* @func embPropCalcMolwt  ****************************************************
**
** Calculate the molecular weight of a protein sequence
** This is a shell around embPropCalcMolwtMod using water as the modifier.
**
** @param [r] s [const char *] sequence
** @param [r] start [ajint] start position
** @param [r] end [ajint] end position
** @param [r] mwdata [EmbPPropMolwt const *] molecular weight data
** @param [r] mono [AjBool] true for monoisotopic values
**
** @return [double] molecular weight
** @@
******************************************************************************/

double embPropCalcMolwt(const char *s, ajint start, ajint end,
			EmbPPropMolwt const *mwdata, AjBool mono)
{
    double nmass = 0.;
    double cmass = 0.;

    nmass = (mono) ? mwdata[EMBPROPHINDEX]->mono :
	mwdata[EMBPROPHINDEX]->average;

    cmass = (mono) ? mwdata[EMBPROPOINDEX]->mono + nmass :
	mwdata[EMBPROPOINDEX]->average + nmass;
    

    return embPropCalcMolwtMod(s,start,end,mwdata,mono,nmass,cmass);
}




/* @func embPropCalcMolwtMod  *************************************************
**
** Calculate the molecular weight of a protein sequence
** with chemically modified termini
**
** @param [r] s [const char *] sequence
** @param [r] start [ajint] start position
** @param [r] end [ajint] end position
** @param [r] mwdata [EmbPPropMolwt const *] molecular weight data
** @param [r] mono [AjBool] true for monoisotopic values
** @param [r] nmass [double] mass of the N-terminal group
** @param [r] cmass [double] mass of the C-terminal group
**
** @return [double] molecular weight
** @@
******************************************************************************/

double embPropCalcMolwtMod(const char *s, ajint start, ajint end,
			   EmbPPropMolwt const *mwdata, AjBool mono,
			   double nmass, double cmass)
{
    const char *p;
    double sum;
    ajint i;
    ajint len;
    ajint idx;
    double mw = 0.;
    
    len = end - start + 1;

    p = s + start;
    sum = 0.0;

    for(i=0;i<len;++i)
    {
	idx = ajBasecodeToInt(toupper((ajint)p[i]));
	mw = (mono) ? mwdata[idx]->mono : mwdata[idx]->average;
	
	sum += mw;
    }
    
    return sum + nmass + cmass;
}




/* @func embPropCalcMolextcoeff*********************************************
**
** Calculate the molecular extinction coefficient of a protein sequence
**
** @param [r] s [const char *] sequence
** @param [r] start [ajint] start position
** @param [r] end [ajint] end position
** @param [r] cystine [AjBool] Treat C residues as cystine pairs
** @param [r] aadata [EmbPPropAmino const *] amino acid data
**
** @return [double] molar extinction coefficient
** @@
******************************************************************************/

double embPropCalcMolextcoeff(const char *s, ajint start, ajint end,
			      AjBool cystine, EmbPPropAmino const *aadata)
{

    const char *p;
    char aa;
    double sum;
    ajint i;
    ajint len;
    ajuint havecystine = 0;

    len = end-start+1;
    
    p   = s+start;
    sum = 0.0;

    for(i=0;i<len;++i)
    {
        aa = toupper((ajint)p[i]);
        if(aa == 'C')
        {
            if(!cystine) continue;
            havecystine++;
            if(havecystine % 2)
                continue;
        }
        sum += (double) aadata[ajBasecodeToInt(aa)]->extcoeff;
    }

    return sum;
}




/* @func embPropCharToThree  **************************************************
**
** Return 3 letter amino acid code A=Ala B=Asx C=Cys etc
**
** @param [r] c [char] integer code
**
** @return [const char*] three letter amino acid code
** @@
******************************************************************************/

const char* embPropCharToThree(char c)
{
    return embPropIntToThree(ajBasecodeToInt(c));
}




/* @func embPropIntToThree  ***************************************************
**
** Return 3 letter amino acid code 0=Ala 1=Asx 2=Cys etc
**
** @param [r] c [ajint] integer code
**
** @return [const char*] three letter amino acid code
** @@
******************************************************************************/

const char* embPropIntToThree(ajint c)
{
    static const char *tab[]=
    {
	"Ala","Asx","Cys","Asp","Glu","Phe","Gly","His","Ile","---","Lys",
	"Leu","Met","Asn","---","Pro","Gln","Arg","Ser","Thr","---",
	"Val","Trp","Xaa","Tyr","Glx"
    };

    return tab[c];
}




/* @func embPropCalcFragments  ************************************************
**
** Read amino acd properties
**
** @param [r] s [const char *] sequence
** @param [r] n [ajint] "enzyme" number
** @param [w] l [AjPList *] list to push hits to
** @param [w] pa [AjPList *] list to push partial hits to
** @param [r] unfavoured [AjBool] allow unfavoured cuts
** @param [r] overlap [AjBool] show overlapping partials
** @param [r] allpartials [AjBool] show all possible partials
** @param [w] ncomp [ajint *] number of complete digest fragments
** @param [w] npart [ajint *] number of partial digest fragments
** @param [w] rname [AjPStr *] name of reagent
** @param [r] nterm [AjBool] nterm ragging
** @param [r] cterm [AjBool] cterm ragging
** @param [r] dorag [AjBool] true if ragging
** @param [r] mwdata [EmbPPropMolwt const *] molecular weight data
** @param [r] mono [AjBool] true for monoisotopic weights
**
** @return [void]
** @@
******************************************************************************/

void embPropCalcFragments(const char *s, ajint n,
			  AjPList *l, AjPList *pa,
			  AjBool unfavoured, AjBool overlap,
			  AjBool allpartials, ajint *ncomp, ajint *npart,
			  AjPStr *rname, AjBool nterm, AjBool cterm,
			  AjBool dorag, EmbPPropMolwt const *mwdata,
			  AjBool mono)
{
    static const char *PROPENZReagent[]=
    {
	"Trypsin","Lys-C","Arg-C","Asp-N","V8-bicarb","V8-phosph",
	"Chymotrypsin","CNBr"
    };

    static const char *PROPENZSite[]=
    {
	"KR","K","R","D","E","DE","FYWLM","M"
    };

    static const char *PROPENZAminoCarboxyl[]=
    {
	"CC","C","C","N","C","CC","CCCCC","C"
    };

    static const char *PROPENZUnfavoured[]=
    {
	"KRIFLP","P","P","","KREP","P","P",""
    };


    ajint i;
    ajint j;
    ajint lim;
    ajint len;
    AjPList t;
    EmbPPropFrag fr;
    ajint *begsa = NULL;
    ajint *endsa = NULL;
    double molwt;
    double *molwtsa = NULL;
    AjBool *afrag   = NULL;
    ajint mark;
    ajint bwp;
    ajint ewp;
    ajint *ival;
    ajint defcnt;

    ajint it;
    ajint st = 0;
    ajint mt = 0;
    ajint et = 0;

    ajStrAssignC(rname,PROPENZReagent[n]);
    defcnt = 0;
    len = (ajint) strlen(s);

    t = ajListNew();			/* Temporary list */


    /* First get all potential cut points */
    for(i=0;i<len;++i)
    {
	if(!strchr(PROPENZSite[n],s[i]))
	    continue;

	if(len==i+1)
	    continue;

	if(strchr(PROPENZUnfavoured[n],s[i+1])
	   && !unfavoured)
	    continue;

	AJNEW0(ival);
	*ival = i;
	ajListPushAppend(t,(void *)ival);
	++defcnt;
    }

    if(defcnt)
    {
	AJCNEW(begsa,(defcnt+1));
	AJCNEW(endsa,(defcnt+1));
	AJCNEW(molwtsa,(defcnt+1));
	AJCNEW(afrag,(defcnt+1));
    }

    for(i=0;i<defcnt;++i)  /* Pop them into a temporary array 	 */
    {
	ajListPop(t,(void **)&ival);
	endsa[i] = *ival;
	AJFREE(ival);
    }


    mark = 0;

    for(i=0;i<defcnt;++i)  /* Work out true starts, ends and molwts */
    {
	bwp = mark;
	ewp = endsa[i];

	if(strchr(PROPENZAminoCarboxyl[n],'N'))
	    --ewp;

	molwt=embPropCalcMolwt(s,bwp,ewp,mwdata,mono);

	if(n==PROPENZCNBR)
	    molwt -= (17.045 + 31.095);

	begsa[i]   = mark;
	endsa[i]   = ewp;
	molwtsa[i] = molwt;
	afrag[i]   = ajFalse;
	mark       = ewp+1;
    }

    if(defcnt)		   /* Special treatment for last fragment   */
    {
	molwt = embPropCalcMolwt(s,mark,len-1,mwdata,mono);
	if(n==PROPENZCNBR)
	    molwt -= (17.045 + 31.095);
	begsa[i]   = mark;
	endsa[i]   = len-1;
	molwtsa[i] = molwt;
	afrag[i]   = ajFalse;
	++defcnt;
    }

    /* Push the hits */
    for(i=0;i<defcnt;++i)
    {
	if(dorag)
	{
	    st = begsa[i];
	    et = endsa[i];

	    for(it=st+RAG_MINPEPLEN-1; it < et; ++it)
	    {
		AJNEW0(fr);
		fr->start = st;
		fr->end   = it;
		fr->molwt = embPropCalcMolwt(s,st,it,mwdata,mono);

		if(n == PROPENZCNBR)
		    fr->molwt -= (17.045 + 31.095);

		fr->isfrag = ajTrue;
		ajListPush(*l,(void *)fr);
	    }
	}
	
	AJNEW0(fr);
	fr->start  = begsa[i];
	fr->end    = endsa[i];
	fr->molwt  = molwtsa[i];
	fr->isfrag = afrag[i];
	ajListPush(*l,(void *) fr);

	if(dorag && nterm)
	    for(it=st+1; it < et-RAG_MINPEPLEN+2; ++it)
	    {
		AJNEW0(fr);
		fr->start = it;
		fr->end   = et;
		fr->molwt = embPropCalcMolwt(s,it,et,mwdata,mono);

		if(n == PROPENZCNBR)
		    fr->molwt -= (17.045 + 31.095);

		fr->isfrag = ajTrue;
		ajListPush(*l,(void *)fr);
	    }
    }

    if(!dorag)
	ajListSort(*l,propFragCompare);
    *ncomp = defcnt;


    /* Now deal with overlaps */
    *npart = 0;

    lim = defcnt -1;

    if(overlap && !allpartials)
    {
	for(i=0;i<lim;++i)
	{
	    if(dorag)
	    {
		st = begsa[i];
		mt = endsa[i];
		et = endsa[i+1];

		if(cterm)
		    for(it=mt+1; it < et; ++it)
		    {
			AJNEW0(fr);
			fr->start = st;
			fr->end   = it;
			fr->molwt = embPropCalcMolwt(s,st,it,mwdata,mono);

			if(n == PROPENZCNBR)
			    fr->molwt -= (17.045 + 31.095);

			fr->isfrag = ajTrue;
			ajListPush(*l,(void *)fr);
		    }
	    }

	    AJNEW0(fr);
	    fr->isfrag = ajTrue;
	    fr->molwt = embPropCalcMolwt(s,begsa[i],endsa[i+1],mwdata,mono);
	    if(n==PROPENZCNBR)
		fr->molwt -= (17.045 + 31.095);
	    fr->start = begsa[i];
	    fr->end   = endsa[i+1];
	    ajListPush(*pa,(void *)fr);
	    ++(*npart);

	    if(dorag && nterm)
		for(it=st+1; it<mt; ++it)
		{
		    AJNEW0(fr);
		    fr->start = it;
		    fr->end   = et;
		    fr->molwt = embPropCalcMolwt(s,it,et,mwdata,mono);

		    if(n == PROPENZCNBR)
			fr->molwt -= (17.045 + 31.095);

		    fr->isfrag = ajTrue;
		    ajListPush(*l,(void *)fr);
		}
	}

	if(*npart)			/* Remove complete sequence */
	{
	    --(*npart);
	    ajListPop(*pa,(void **)&fr);
	}

	if(!dorag)
	    ajListSort(*pa,propFragCompare);
    }

    if(allpartials)
    {
        lim = defcnt;

	for(i=0;i<lim;++i)
	    for(j=i+1;j<lim;++j)
	    {
		AJNEW0(fr);
		fr->isfrag = ajTrue;
		fr->molwt = embPropCalcMolwt(s,begsa[i],endsa[j],mwdata,mono);

		if(n==PROPENZCNBR)
		    fr->molwt -= (17.045 + 31.095);

		fr->start = begsa[i];
		fr->end   = endsa[j];
		ajListPush(*pa,(void *)fr);
		++(*npart);
	    }

	if(*npart)			/* Remove complete sequence */
	{
	    --(*npart);
	    ajListPop(*pa,(void **)&fr);
	}

	if(!dorag)
	    ajListSort(*pa,propFragCompare);
    }

    if(defcnt)
    {
	AJFREE(molwtsa);
	AJFREE(endsa);
	AJFREE(begsa);
	AJFREE(afrag);
    }

    ajListFree(&t);

    return;
}




/* @funcstatic propFragCompare  ***********************************************
**
** compare two molecular weight AjPFrag list elements for sorting
**
** @param [r] a [const void*] First element
** @param [r] b [const void*] Second element
**
** @return [ajint] 0=equal +ve=(a greater than b) -ve=(a less than b)
** @@
******************************************************************************/

static ajint propFragCompare(const void *a, const void *b)
{
    return (ajint)((*(EmbPPropFrag const *)b)->molwt -
		   (*(EmbPPropFrag const *)a)->molwt);
}




/* @func embPropProtGaps ******************************************************
**
** Creates a string of a protein sequence which has been padded out with
** two spaces after every residue to aid aligning a translation under a
** nucleic sequence
**
** @param [u] seq [AjPSeq] protein sequence to add spaces into
** @param [r] pad [ajint] number of spaces to insert at the start of the result
** @return [AjPStr] New string with the padded sequence
** @@
******************************************************************************/

AjPStr embPropProtGaps(AjPSeq seq, ajint pad)
{
    const char *p;
    AjPStr temp;
    ajint i;

    temp = ajStrNewRes(ajSeqGetLen(seq)*3 + pad+1);

    /* put any required padding spaces at the start */
    for(i=0; i<pad; i++)
	ajStrAppendC(&temp, " ");


    for(p=ajSeqGetSeqC(seq); *p; p++)
    {
	ajStrAppendK(&temp, *p);
	ajStrAppendC(&temp, "  ");
    }

    return temp;
}




/* @func embPropProt1to3 ******************************************************
**
** Creates a a 3-letter sequence protein string from single-letter sequence
** EMBOSS is bad at reading 3-letter sequences, but this may be useful
** when displaying translations.
**
** @param [u] seq [AjPSeq] protein sequence to convert to 3-letter codes
** @param [r] pad [ajint] number of spaces to insert at the start of the result
** @return [AjPStr] string containing 3-letter protein sequence
** @@
******************************************************************************/

AjPStr embPropProt1to3(AjPSeq seq, ajint pad)
{
    const char *p;
    const char *p3;
    AjPStr temp;
    ajint i;

    temp = ajStrNewRes(ajSeqGetLen(seq)*3 + pad+1);

    /* put any required padding spaces at the start */
    for(i=0; i<pad; i++)
	ajStrAppendC(&temp, " ");


    for(p=ajSeqGetSeqC(seq); *p; p++)
    {
	if(*p == '*')
	    ajStrAppendC(&temp, "***");
	else if(*p == '.')
	    ajStrAppendC(&temp, "...");
	else if(*p == '-')
	    ajStrAppendC(&temp, "---");
	else if(!isalpha((ajint)*p))
	    ajStrAppendC(&temp, "???");
	else
	{
	    p3 = embPropCharToThree(*p);
	    ajStrAppendK(&temp, *p3);
	    ajStrAppendK(&temp, *(p3+1));
	    ajStrAppendK(&temp, *(p3+2));
	}
    }

    return temp;
}




/* @func embPropProt1to3Rev ****************************************************
**
** Creates a a 3-letter sequence protein string from single-letter sequence
** in the reverse direction.
** EMBOSS is bad at reading 3-letter sequences, but this may be useful
** when displaying translations.
**
** @param [u] seq [AjPSeq] protein sequence to convert to 3-letter codes
** @param [r] pad [ajint] number of characters to skip at the start
**                        of the result
** @return [AjPStr] string containing 3-letter protein sequence
** @@
******************************************************************************/

AjPStr embPropProt1to3Rev(AjPSeq seq, ajint pad)
{
    const char *p;
    const char *p3;
    AjPStr temp;
    ajint i=0;

    temp = ajStrNewRes(ajSeqGetLen(seq)*3 + pad+1);

    for(p=ajSeqGetSeqC(seq); *p; p++)
    {
	if(*p == '*')
	    ajStrAppendC(&temp, "***");
	else if(*p == '.')
	    ajStrAppendC(&temp, "...");
	else if(*p == '-')
	    ajStrAppendC(&temp, "---");
	else if(!isalpha((ajint)*p))
	    ajStrAppendC(&temp, "???");
	else
	{
            p3 = embPropCharToThree(*p);

            if(i++)
            {
                ajStrAppendK(&temp, *(p3+2));
                ajStrAppendK(&temp, *(p3+1));
                ajStrAppendK(&temp, *p3);
            }
            else
            {
                if(pad >= 2) 
                    ajStrAppendK(&temp, *(p3+2));

                if(pad >= 1) 
                    ajStrAppendK(&temp, *(p3+1));

                ajStrAppendK(&temp, *p3);
            }            
        }
    }

    return temp;
}




/* @func embPropPurine ********************************************************
**
** Returns ajTrue if the input base is a Purine.
** Returns ajFalse if it is a Pyrimidine or it is ambiguous.
**
** @param [r] base [char] base
** @return [AjBool] return ajTrue if this is a Purine
** @@
******************************************************************************/

AjBool embPropPurine(char base)
{
    if(strchr(propPurines, (ajint)base))
	return ajTrue;

    return ajFalse;
}




/* @func embPropPyrimidine ****************************************************
**
** Returns ajTrue if the input base is a Pyrimidine.
** Returns ajFalse if it is a Purine or it is ambiguous.
**
** @param [r] base [char] base
** @return [AjBool] return ajTrue if this is a Pyrimidine
** @@
******************************************************************************/

AjBool embPropPyrimidine(char base)
{
    if(strchr(propPyrimidines, (ajint)base))
	return ajTrue;

    return ajFalse;
}




/* @func embPropTransversion **************************************************
**
** Returns ajTrue if the input two bases have undergone a tranversion.
** (Pyrimidine to Purine, or vice versa)
** Returns ajFalse if this is not a transversion or it can not be determined
** (e.g. no change A->A, transition C->T, unknown A->N)
**
** @param [r] base1 [char] first base
** @param [r] base2 [char] second base
** @return [AjBool] return ajTrue if this is a transversion
** @@
******************************************************************************/

AjBool embPropTransversion(char base1, char base2)
{
    AjBool bu1;
    AjBool bu2;
    AjBool by1;
    AjBool by2;

    bu1 = embPropPurine(base1);
    bu2 = embPropPurine(base2);

    by1 = embPropPyrimidine(base1);
    by2 = embPropPyrimidine(base2);

    ajDebug("base1 py = %b, pu = %b", bu1, by1);
    ajDebug("base2 py = %b, pu = %b", bu2, by2);


    /* not a purine or a pyrimidine - ambiguous - return ajFalse */
    if(!bu1 && !by1)
	return ajFalse;

    if(!bu2 && !by2)
	return ajFalse;

    ajDebug("embPropTransversion result = %d", (bu1 != bu2));

    return (bu1 != bu2);
}




/* @func embPropTransition ****************************************************
**
** Returns ajTrue if the input two bases have undergone a transition.
** (Pyrimidine to Pyrimidine, or Purine to Purine)
** Returns ajFalse if this is not a transition or it can not be determined
** (e.g. no change A->A, transversion A->T, unknown A->N)
**
** @param [r] base1 [char] first base
** @param [r] base2 [char] second base
** @return [AjBool] return ajTrue if this is a transition
** @@
******************************************************************************/

AjBool embPropTransition(char base1, char base2)
{
    AjBool bu1;
    AjBool bu2;
    AjBool by1;
    AjBool by2;

    bu1 = embPropPurine(base1);
    bu2 = embPropPurine(base2);

    by1 = embPropPyrimidine(base1);
    by2 = embPropPyrimidine(base2);

    ajDebug("base1 py = %b, pu = %b", bu1, by1);
    ajDebug("base2 py = %b, pu = %b", bu2, by2);

    /* not a purine or a pyrimidine - ambiguous - return ajFalse */
    if(!bu1 && !by1)
	return ajFalse;

    if(!bu2 && !by2)
	return ajFalse;

    /* no change - return ajFalse */
    if(tolower((int)base1) == tolower((int)base2))
	return ajFalse;

    /* U to T is not a transition */
    if(tolower((int)base1) == 't' && tolower((int)base2) == 'u')
	return ajFalse;

    if(tolower((int)base1) == 'u' && tolower((int)base2) == 't')
	return ajFalse;

    /* C to Y, T to Y, A to R, G to R - ambiguous - not a transition */
    if(bu1 && tolower((int)base2) == 'r')
	return ajFalse;

    if(bu2 && tolower((int)base1) == 'r')
	return ajFalse;

    if(by1 && tolower((int)base2) == 'y')
	return ajFalse;

    if(by2 && tolower((int)base1) == 'y')
	return ajFalse;

    ajDebug("embPropTransition result = %b", (bu1 == bu2));

    return (bu1 == bu2);
}




/* @func embPropFixF *********************************************************
**
** Fix for missing properties data in a float array
**
** @param [u] matrix [float[]] Matrix
** @param [r] missing [float] Missing data value
** @return [void]
******************************************************************************/

void embPropFixF(float matrix[], float missing)
{
    ajint i;

    float mtot = 0.0;
    float dtot = 0.0;

    for(i=0;i<26;i++)
    {
	if(matrix[i] == missing)
	{
	    switch (i)
	    {
	    case 1:			/* B: D + N */
		matrix[i] = ((matrix[3] * dayhoff[3]) +
			     (matrix[13] * dayhoff[13])) /
				 (dayhoff[3] + dayhoff[13]);
		ajDebug("Missing %d '%c' %f %f => %f\n",
			i, dayhoffstr[i], matrix[3], matrix[13], matrix[i]);
		break;
	    case 9:			/* J: I + L */
		matrix[i] = ((matrix[8] * dayhoff[8]) +
			     (matrix[11] * dayhoff[11])) /
				 (dayhoff[8] + dayhoff[11]);
		ajDebug("Missing %d '%c' %f %f => %f\n",
			i, dayhoffstr[i], matrix[8], matrix[11], matrix[i]);
		break;
	    case 25:			/* Z: E + Q */
		matrix[i] = ((matrix[4] * dayhoff[4]) +
			     (matrix[16] * dayhoff[16])) /
				 (dayhoff[4] + dayhoff[16]);
		ajDebug("Missing %d '%c' %f %f => %f\n",
			i, dayhoffstr[i], matrix[4], matrix[16], matrix[i]);
		break;
	    default:
		ajDebug("Missing %d '%c' unknown\n", i, dayhoffstr[i]);
		break;
	    }
	}
	else
	{
	    if(dayhoff[i] > 0.0)
	    {
		dtot += dayhoff[i];
		mtot += matrix[i] * dayhoff[i];
	    }
	}
    }

    mtot /= dtot;

    for(i=0;i<26;i++)
	if(matrix[i] == missing) /* X:average O,U:X */
	{
	    matrix[i] = mtot;
	    ajDebug("Missing %d '%c' unknown %f\n",
		    i, dayhoffstr[i], matrix[i]);
	}

    return;
}




/* @func embPropNormalF ********************************************************
**
** Normalize data values in a float array to have mean 0.0 and
** standard deviation 1.0
**
** Assume the data values represent all values for a population
** (e.g. values for all standard amino acids) and use
** new value = (old value - mean) / standard deviation
**
** @param [u] matrix [float[]] Matrix
** @param [r] missing [float] Missing data value
** @return [void]
******************************************************************************/

void embPropNormalF(float matrix[], float missing)
{
    ajuint i;

    double count = 0.0;
    double total = 0.0;
    double sumsq = 0.0;
    double sigma = 0.0;
    double mean = 0.0;
    const char* alphabet = "ABCDEFGHIJKLMNOPQRSTUVWXYZ";

    for(i=0;i<26;i++)
	if(matrix[i] != missing)
	{
            count += 1.0;
            total += matrix[i];
            sumsq += matrix[i] * matrix[i];
        }

    if(!count)
        return;

    sigma = sqrt(count*sumsq - total*total)/count;
    mean = total/count;

    ajDebug("matrix normalize mean: %.3f sigma: %.3f\n", mean, sigma);

    for(i=0;i<26;i++)
	if(matrix[i] != missing)
        {
            ajDebug("matrix[%u] %c %.3f", i, alphabet[i], matrix[i]);
            matrix[i] = (float) ((matrix[i] - mean) / sigma);
            ajDebug(" => %.3f\n", matrix[i]);
        }

    return;
}




/* @func embPropAminoDel ******************************************************
**
** Delete array of amino acid properties
**
** @param [w] thys [EmbPPropAmino**] amino acid properties
** @return [void]
******************************************************************************/

void embPropAminoDel(EmbPPropAmino **thys)
{
    EmbPPropAmino *pthis = NULL;
    ajuint i;
    
    pthis = *thys;

    for(i=0; i < EMBPROPSIZE; ++i)
	AJFREE(pthis[i]);

    AJFREE(pthis);

    *thys = NULL;

    return;
}




/* @func embPropMolwtDel ******************************************************
**
** Delete array of molwts
**
** @param [w] thys [EmbPPropMolwt**] molwts
** @return [void]
******************************************************************************/

void embPropMolwtDel(EmbPPropMolwt **thys)
{
    EmbPPropMolwt *pthis = NULL;
    ajuint i;
    
    pthis = *thys;

    for(i=0; i < EMBPROPSIZE + 2; ++i)
	AJFREE(pthis[i]);

    AJFREE(pthis);

    *thys = NULL;

    return;
}
