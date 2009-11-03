/****************************************************************************
** 
** @source ajdomain.c 
** 
** AJAX low-level functions for handling protein domain data.  
** For use with Scop and Cath objects defined in ajdomain.h
** Includes functions for reading SCOP and CATH parsable files and for 
** reading and writing dcf (domain classification file) format.
**
** @author Copyright (C) 2004 Jon Ison (jison@hgmp.mrc.ac.uk)
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
****************************************************************************/





/* ======================================================================= */
/* ============================ include files ============================ */
/* ======================================================================= */

#include "ajax.h"





/* ======================================================================= */
/* ============================ private data ============================= */
/* ======================================================================= */

/* @datastatic AjPScopcla *****************************************************
**
** local Scopcla object.
**
** Holds scop database data from raw file (dir.cla.scop.txt from SCOP authors)
**
** AjPScopcla is implemented as a pointer to a C data structure.
**
**
**
** @alias AjSScopcla
** @alias AjOScopcla
**
**
**
** @attr Entry [AjPStr]      Domain identifer code. 
** @attr Pdb [AjPStr]        Corresponding pdb identifier code.
** @attr Sccs [AjPStr]       Scop compact classification string.
** @attr Class [ajint]       SCOP sunid for class 
** @attr Fold[ajint]         SCOP sunid for fold 
** @attr Superfamily [ajint] SCOP sunid for superfamily 
** @attr Family [ajint]      SCOP sunid for family 
** @attr Domain [ajint]      SCOP sunid for domain   
** @attr Source [ajint]      SCOP sunid for species 
** @attr Domdat [ajint]      SCOP sunid for domain data
** @attr N [ajint]           No. chains from which this domain is comprised 
** @attr Chain [char*]       Chain identifiers 
** @attr Start [AjPStr*]     PDB residue number of first residue in domain 
** @attr End [AjPStr*]       PDB residue number of last residue in domain 
**
** @@
****************************************************************************/

typedef struct AjSScopcla
{
    AjPStr Entry;
    AjPStr Pdb;
    AjPStr Sccs;

    ajint  Class;
    ajint  Fold;
    ajint  Superfamily;
    ajint  Family;  
    ajint  Domain;
    ajint  Source;
    ajint  Domdat;
        
    ajint    N;
    char   *Chain;
    AjPStr *Start;
    AjPStr *End;
} AjOScopcla,*AjPScopcla;





/* @datastatic AjPScopdes *****************************************************
**
** AJAX Scopdes object.
**
** Holds SCOP database data from raw file (dir.des.scop.txt from SCOP authors)
**
** AjPScopdes is implemented as a pointer to a C data structure.
**
**
**
** @alias AjSScopdes
** @alias AjOScopdes
**
**
**
** @attr Type [AjPStr]  Type of node, either 'px' (domain data), 'cl' (class),
**                      'cf' (fold), 'sf' (superfamily), 'fa' (family), 'dm' 
**                      (domain) or 'sp' (species).
** @attr Sccs [AjPStr]  Scop compact classification string.
** @attr Entry [AjPStr] Domain identifer code (or '-' if Type!='px').
** @attr Desc [AjPStr]  Description in english of the node.
** @attr Sunid [ajint]  SCOP sunid for node.
** @attr Padding [char[4]]  Padding to alignment boundary
**
** @@
****************************************************************************/

typedef struct AjSScopdes
{
    AjPStr Type;
    AjPStr Sccs;
    AjPStr Entry;
    AjPStr Desc;
    ajint  Sunid;
    char Padding[4];
} AjOScopdes;
#define AjPScopdes AjOScopdes*





/* @datastatic AjPCathDom *****************************************************
**
** AJAX CathDom object
**
** Holds CATH database data from domlist.v2.4. This file only contains
** domain information for proteins that have 2 or more domains. 
**
** AjPCathDom is implemented as a pointer to a C data structure.
**
**
**
**  @alias AjSCathDom
**  @alias AjOCathDom
**
**
**
** @attr DomainID [AjPStr] ID for protein containing 2 or more domains
** @attr Start [AjPStr*]   PDB residue number of first residue in segment
** @attr End [AjPStr*]      PDB residue number of last residue in segment
** @attr NSegment [ajint]  No. of chain segments domain is comprised of
** @attr Padding [char[4]]  Padding to alignment boundary
**
**  @@
****************************************************************************/

typedef struct AjSCathDom 
{
   AjPStr DomainID; 
   AjPStr *Start;
   AjPStr *End;          
   ajint  NSegment;
   char Padding[4];
} AjOCathDom;
#define AjPCathDom AjOCathDom*





/* @datastatic AjPCathName ****************************************************
**
** AJAX CathName object
**
** Holds CATH database data from CAT.names.all.v2.4. This file contains
** a description of each level in the CATH hierarchy. 
**
** AjPCathName is implemented as a pointer to a C data structure.
**
**
** 
**  @alias AjSCathName
**  @alias AjOCathName
**
**
**
** @attr Id [AjPStr]   Classification Id 
** @attr Desc [AjPStr] Description of level in CATH hierarchy
**  
**  @@
****************************************************************************/

typedef struct AjSCathName
{
    AjPStr Id;
    AjPStr Desc;          
} AjOCathName;
#define AjPCathName AjOCathName*





/* ======================================================================= */
/* ================= Prototypes for private functions ==================== */
/* ======================================================================= */

static AjPScopcla    domainScopclaNew(ajint chains);
static void          domainScopclaDel(AjPScopcla *thys);
static AjPScopcla    domainScopclaRead(AjPFile inf, const AjPStr entry);
static AjPScopcla    domainScopclaReadC(AjPFile inf, const char *entry);

static AjPScopdes    domainScopdesNew(void);
static void          domainScopdesDel(AjPScopdes *ptr);
static AjPScopdes    domainScopdesRead(AjPFile inf, const AjPStr entry);
static AjPScopdes    domainScopdesReadC(AjPFile inf, const char *entry);
static ajint         domainScopdesBinSearch(ajint id, AjPScopdes const *arr,
					    ajint siz);

static ajint         domainScopdesCompSunid(const void *scop1,
					    const void *scop2);

static ajint         domainCathNameBinSearch(const AjPStr id,
					     AjPCathName const *arr,
					     ajint siz);
static ajint         domainCathDomBinSearch(const AjPStr id,
					    AjPCathDom const *arr,
					    ajint siz);
static ajint         domainSortDomainID(const void *DomID1,
					const void *DomID2);
static ajint         domainSortNameId(const void *cath1,
				      const void *cath2);
static void          domainCathNameDel(AjPCathName *ptr);
static AjPCathName   domainCathNameNew(void);
static void          domainCathDomDel(AjPCathDom *ptr);
static AjPCathDom    domainCathDomNew(ajint nsegments);


/* ======================================================================= */
/* ========================== private functions ========================== */
/* ======================================================================= */

/* @funcstatic domainScopclaNew ***********************************************
**
** Scopcla object constructor. Fore-knowledge of the number of chains is 
** required. This is normally called by the domainScopclaReadC / 
** domainScopclaRead functions.
**
** @param [r] chains [ajint] Number of chains
**
** @return [AjPScopcla] Pointer to a scopcla object
** @@
****************************************************************************/

static AjPScopcla domainScopclaNew(ajint chains)
{
    AjPScopcla ret = NULL;
    ajint i;

    AJNEW0(ret);

    ret->Entry = ajStrNew();
    ret->Pdb   = ajStrNew();
    ret->Sccs  = ajStrNew();

    if(chains)
    {
	ret->Chain = ajCharNewRes(chains);
	AJCNEW0(ret->Start,chains);
	AJCNEW0(ret->End,chains);
	for(i=0; i<chains; i++)
	{
	    ret->Start[i]=ajStrNew();
	    ret->End[i]=ajStrNew();
	}
    }

    ret->N = chains;

    return ret;
}





/* @funcstatic domainScopdesNew ***********************************************
**
** Scopdes object constructor.
**
** This is normally called by the domainScopdesReadC / domainScopdesRead
** functions.
**
** @return [AjPScopdes] Pointer to a scopdes object
** @@
****************************************************************************/

static AjPScopdes domainScopdesNew(void)
{
    AjPScopdes ret = NULL;

    AJNEW0(ret);

    ret->Type      = ajStrNew();
    ret->Sccs      = ajStrNew();
    ret->Entry     = ajStrNew();
    ret->Desc      = ajStrNew();
    
    return ret;
}





/* @funcstatic domainScopclaRead **********************************************
**
** Read a Scopcla object for a given SCOP domain from the SCOP parsable 
** file (dir.cla.scop.txt).
**
** @param [u] inf   [AjPFile]      Input file stream
** @param [r] entry [const AjPStr]       id
** @return [AjPScopcla] Scopcla object
** @category new [AjPScopcla] Read a Scopcla object for a given SCOP domain
** @@
****************************************************************************/

static AjPScopcla domainScopclaRead(AjPFile inf, const AjPStr entry)
{
    return domainScopclaReadC(inf,ajStrGetPtr(entry));
}




/* @funcstatic domainScopdesRead **********************************************
**
** Read a Scopdes object for a given SCOP domain from the SCOP parsable 
** file (dir.des.scop.txt).
**
** @param [u] inf   [AjPFile]      Input file stream.
** @param [r] entry [const AjPStr]       CATH id of domain.
**
** @return [AjPScopdes] Scopdes object
** @category new [AjPScopdes] Read a Scopdes object for a given SCOP domain
** @@
****************************************************************************/

static AjPScopdes domainScopdesRead(AjPFile inf, const AjPStr entry)
{
    return domainScopdesReadC(inf,ajStrGetPtr(entry));
}





/* @funcstatic domainScopdesReadC *********************************************
**
** Read a Scopdes object for a given SCOP domain from the SCOP parsable 
** file (dir.des.scop.txt).
**
** @param [u] inf   [AjPFile]      Input file stream.
** @param [r] entry [const char*]        SCOP id of domain.
**
** @return [AjPScopdes] Scopdes object
** @category new [AjPScopdes] Read a Scopdes object for a given SCOP domain
** @@
****************************************************************************/

static AjPScopdes domainScopdesReadC(AjPFile inf, const char *entry)
{
    AjPScopdes ret = NULL;
    static AjPStr line     = NULL;   /* Line from file */
    static AjPStr sunidstr = NULL;   /* sunid as string */
    static AjPStr tentry   = NULL;
    static AjPStr tmp      = NULL;
    static AjPRegexp rexp  = NULL;

    AjBool ok = ajFalse;
    
    
    /* Only initialise strings if this is called for the first time */
    if(!line)
    {    
	line     = ajStrNew();
	tentry   = ajStrNew();
	sunidstr = ajStrNew();
	tmp      = ajStrNew();

	rexp  = ajRegCompC(
	    "^([^ \t]+)[ \t]+([^ \t]+)[ \t]+([^ \t]+)[ \t]+([^ \t]+)[ \t]+");
    }
    

    /* Read up to the correcty entry (line) */
    ajStrAssignC(&tentry,entry);
    ajStrFmtUpper(&tentry);
    
    while((ok=ajFileReadLine(inf,&line)))
    {
	if((ajFmtScanS(line, "%S", &sunidstr)==0))
	    return NULL;

	/* Ignore comment lines */
	if(*(line->Ptr) == '#')
	    continue;
	
	if(ajStrMatchWildS(sunidstr,tentry))
	    break;
    }
    
    if(!ok)
	return NULL;

    ret = domainScopdesNew();
    
    if((ajFmtScanS(line, "%d %S %S %S", &ret->Sunid,&ret->Type,
		   &ret->Sccs, &ret->Entry)!=4))
    {
	domainScopdesDel(&ret);
	return NULL;
    }

    /* Tokenise the line by ' ' and discard the first 4 strings */
    
    if(!ajRegExec(rexp,line))
    {
	ajFmtPrint("-->  %S\n", line);
	ajFatal("File read error in domainScopdesReadC");
    }

    
    ajRegSubI(rexp,1,&tmp);
    ajRegSubI(rexp,2,&tmp);
    ajRegSubI(rexp,3,&tmp);
    ajRegSubI(rexp,4,&tmp);
    ajRegPost(rexp,&ret->Desc);
    ajStrRemoveWhiteExcess(&ret->Desc);

    return ret;
}





/* @funcstatic domainScopclaReadC *********************************************
**
** Read a Scopcla object for a given SCOP domain from the SCOP parsable 
** file (dir.des.scop.txt).
**
** @param [u] inf   [AjPFile]      Input file stream
** @param [r] entry [const char*]  SCOP domain id
** @return [AjPScopcla] Scopcla object
** @category new [AjPScopcla] Read a Scopcla object for a given SCOP domain
** @@
****************************************************************************/

static AjPScopcla domainScopclaReadC(AjPFile inf, const char *entry)
{
    AjPScopcla ret = NULL;
    static AjPStr line   = NULL;
    static AjPStr scopid = NULL;  /* SCOP code */
    static AjPStr pdbid  = NULL;  /* PDB code */
    static AjPStr chains = NULL;  /* Chain data */
    static AjPStr sccs   = NULL;  /* Scop compact classification string */
    static AjPStr class  = NULL;  /* Classification containing all
					 SCOP sunid's  */
    static AjPStr tentry = NULL;
    static AjPStr token  = NULL;
    static AjPStr str    = NULL;

    static AjPRegexp exp = NULL;

    AjPStrTok handle  = NULL;
    AjPStrTok bhandle = NULL;
    AjBool ok         = ajFalse;    

    char c   = ' ';
    const char *p  = NULL;
    ajint n  = 0;
    ajint i  = 0;
    ajint from;
    ajint to;


    /* Only initialise strings if this is called for the first time */
    if(!line)
    {    
	line    = ajStrNew();
	scopid  = ajStrNew();
	pdbid   = ajStrNew();
	chains  = ajStrNew();
	sccs    = ajStrNew();
	tentry  = ajStrNew();
	token   = ajStrNew();
	str     = ajStrNew();
	
	exp   = ajRegCompC("^([0-9]+)([A-Za-z]+)[-]([0-9]+)");
    }
    

    /* Read up to the correcty entry (line) */
    ajStrAssignC(&tentry,entry);
    ajStrFmtUpper(&tentry);
    
    while((ok=ajFileReadLine(inf,&line)))
    {
	if((ajFmtScanS(line, "%S", &scopid)==0))
	    return NULL;

	/* Ignore comment lines */
	if(*scopid->Ptr == '#')
	    continue;
		
	if(ajStrMatchWildS(scopid,tentry))
	    break;
    }
    
    if(!ok)
	return NULL;


    if((ajFmtScanS(line, "%*S %S %S %S %*d %S", &pdbid,&chains, &sccs,
		   &class)!=4))
	return NULL;

    /* Count chains and allocate Scopcla object */
    n = ajStrParseCountC(chains,",");
    ret = domainScopclaNew(n);

    ajStrFmtUpper(&scopid);
    ajStrAssignS(&ret->Entry,scopid);

    ajStrFmtUpper(&pdbid);
    ajStrAssignS(&ret->Pdb,pdbid);

    ajStrFmtUpper(&sccs);
    ajStrAssignS(&ret->Sccs,sccs);

    handle = ajStrTokenNewC(chains,",");
    for(i=0;i<n;++i)
    {
	ajStrTokenNextParse(&handle,&token);
	    	    
	p = ajStrGetPtr(token);
	if(sscanf(p,"%d-%d",&from,&to)==2)
	{
	    ret->Chain[i]='.';
	    ajFmtPrintS(&ret->Start[i],"%d",from);
	    ajFmtPrintS(&ret->End[i],"%d",to);
	}
	else if(sscanf(p,"%c:%d-%d",&c,&from,&to)==3)
	{
	    ajFmtPrintS(&ret->Start[i],"%d",from);
	    ajFmtPrintS(&ret->End[i],"%d",to);
	    ret->Chain[i]=c;
	}
	else if(ajStrGetCharPos(token,1)==':')
	{
	    ajStrAssignC(&ret->Start[i],".");
	    ajStrAssignC(&ret->End[i],".");
	    ret->Chain[i]=*ajStrGetPtr(token);
	}
	else if(ajRegExec(exp,token))
	{
	    ajRegSubI(exp,1,&str);
	    ajStrAssignS(&ret->Start[i],str);
	    ajRegSubI(exp,2,&str);
	    ret->Chain[i] = *ajStrGetPtr(str);
	    ajRegSubI(exp,3,&str);
	    ajStrAssignS(&ret->End[i],str);
	}
	else if(ajStrGetCharFirst(token)=='-')
	{
	    ret->Chain[i]='.';
	    ajStrAssignC(&ret->Start[i],".");
	    ajStrAssignC(&ret->End[i],".");
	}
	else
	    ajFatal("Unparseable chain line [%S]\n",chains);
    }
    ajStrTokenDel(&handle);
	      
	      
    /* Read SCOP sunid's from classification string */
    bhandle = ajStrTokenNewC(class,",\n");
    while(ajStrTokenNextParse(&bhandle,&token))
    {
	if(ajStrPrefixC(token,"cl"))
	    ajFmtScanS(token, "cl=%d", &ret->Class);
	else if(ajStrPrefixC(token,"cf"))
	    ajFmtScanS(token, "cf=%d", &ret->Fold);
	else if(ajStrPrefixC(token,"sf"))
	    ajFmtScanS(token, "sf=%d", &ret->Superfamily);
	else if(ajStrPrefixC(token,"fa"))
	    ajFmtScanS(token, "fa=%d", &ret->Family);
	else if(ajStrPrefixC(token,"dm"))
	    ajFmtScanS(token, "dm=%d", &ret->Domain);
	else if(ajStrPrefixC(token,"sp"))
	    ajFmtScanS(token, "sp=%d", &ret->Source);
	else if(ajStrPrefixC(token,"px"))
	    ajFmtScanS(token, "px=%d", &ret->Domdat);
    }
    ajStrTokenDel(&bhandle);

    return ret;
}	





/* @funcstatic domainScopclaDel ***********************************************
**
** Destructor for scopcla object.
**
** @param [d] thys [AjPScopcla*] Scopcla object pointer
** @return [void]
** @category delete [AjPScopcla] Default destructor
** @@
****************************************************************************/

static void domainScopclaDel(AjPScopcla *thys)
{
    AjPScopcla pthis;
    ajint i;

    pthis = *thys;

    if(!pthis || !thys)
	return;

    ajStrDel(&pthis->Entry);
    ajStrDel(&pthis->Pdb);
    ajStrDel(&pthis->Sccs);

    if(pthis->N)
    {
	for(i=0; i<pthis->N; i++)
	{
	    ajStrDel(&pthis->Start[i]);
	    ajStrDel(&pthis->End[i]);
	}
	AJFREE(pthis->Start);
	AJFREE(pthis->End);
	AJFREE(pthis->Chain);
    }

    AJFREE(pthis);
    (*thys)=NULL;

    return;
}





/* @funcstatic domainScopdesBinSearch *****************************************
**
** Performs a binary search for a Sunid over an array of Scopdes objects 
** structures (which of course must first have been sorted, e.g. by using 
** domainScopdesCompSunid).
**
** @param [r] id  [ajint]        Search value of Sunid
** @param [r] arr [AjPScopdes const *] Array of Scopdes objects
** @param [r] siz [ajint]        Size of array
**
** @return [ajint] Index of first Scopdes object found with a Sunid 
**                 element matching id, or -1 if id is not found.
** @@
****************************************************************************/

static ajint domainScopdesBinSearch(ajint id, AjPScopdes const *arr, ajint siz)
{
    int l;
    int m;
    int h;
    

    l = 0;
    h = siz-1;
    while(l<=h)
    {
        m = (l+h)>>1;

	if(id < arr[m]->Sunid)
	    h = m-1;
	else if(id > arr[m]->Sunid)
	    l = m+1;
	else
	    return m;
    }

    return -1;
}





/* @funcstatic domainScopdesDel ***********************************************
**
** Scopdes object destructor.
**
** @param [d] ptr [AjPScopdes *] Scopdes object pointer
**
** @return [void] 
** @category delete [AjPScopdes] Default destructor
** @@
****************************************************************************/

static void domainScopdesDel(AjPScopdes *ptr)
{
    /* Check arg's */
    if(*ptr==NULL)
	return;
    
    if((*ptr)->Type)
	ajStrDel( &((*ptr)->Type)); 

    if((*ptr)->Sccs)
	ajStrDel( &((*ptr)->Sccs)); 

    if((*ptr)->Entry)
	ajStrDel( &((*ptr)->Entry)); 

    if((*ptr)->Desc)
	ajStrDel( &((*ptr)->Desc)); 

    AJFREE(*ptr);
    *ptr = NULL;
    
    return;
    
}





/* @funcstatic domainScopdesCompSunid *****************************************
**
** Function to sort Scopdes objects by Sunid element.
**
** @param [r] scop1  [const void*] Pointer to Scopdes object 1
** @param [r] scop2  [const void*] Pointer to Scopdes object 2
**
** @return [ajint] -1 if Sunid1 should sort before Sunid2, +1 if the Sunid2 
** should sort first. 0 if they are identical in value.
** @@
****************************************************************************/

static ajint domainScopdesCompSunid(const void *scop1, const void *scop2)
{
    const AjPScopdes p = NULL;
    const AjPScopdes q = NULL;

    p = (*  (AjPScopdes const *)scop1);
    q = (*  (AjPScopdes const *)scop2);
    
    if(p->Sunid < q->Sunid)
	return -1;
    else if(p->Sunid == q->Sunid)
	return 0;

    return 1;
}



/* @funcstatic domainCathNameBinSearch ****************************************
**
** Performs a binary search for a domain code over an array of CathName
** structures (which of course must first have been sorted). This is a 
** case-insensitive search.
**
** @param [r] id  [const AjPStr]       Search term
** @param [r] arr [AjPCathName const *] Array of CathName objects
** @param [r] siz [ajint]        Size of array
**
** @return [ajint] Index of first CathName object found with an CATH Id code
** matching id, or -1 if id is not found.
** @@
****************************************************************************/
static ajint domainCathNameBinSearch(const AjPStr id, AjPCathName const *arr,
				     ajint siz)
{
    int l;
    int m;
    int h;
    int c;


    l=0;
    h=siz-1;
    while(l<=h)
    {
        m=(l+h)>>1;

        if((c=ajStrCmpCaseS(id, arr[m]->Id)) < 0) 
	    h=m-1;
        else if(c>0) 
	    l=m+1;
        else 
	    return m;
    }
    return -1;
}




/* @funcstatic domainCathDomBinSearch *****************************************
**
** Performs a binary search for a domain code over an array of CathDom
** structures (which of course must first have been sorted). This is a 
** case-insensitive search.
**
** @param [r] id  [const AjPStr]       Search term
** @param [r] arr [AjPCathDom const *] Array of AjPCathDom objects
** @param [r] siz [ajint]        Size of array
**
** @return [ajint] Index of first AjPCathDom object found with an domain code
** matching id, or -1 if id is not found.
** @@
****************************************************************************/
static ajint domainCathDomBinSearch(const AjPStr id, AjPCathDom const *arr,
				    ajint siz)
{
    int l;
    int m;
    int h;
    int c;


    l=0;
    h=siz-1;
    while(l<=h)
    {
        m=(l+h)>>1;

        if((c=ajStrCmpCaseS(id, arr[m]->DomainID)) < 0) 
	    h=m-1;
        else if(c>0) 
	    l=m+1;
        else 
	    return m;
    }
    return -1;
}







/* @funcstatic domainCathDomNew ***********************************************
**
** CathDom object constructor. Fore-knowledge of the number of segments is 
** required.
**
** @param [r] nsegments [ajint] Number of segments
**
** @return [AjPCathDom] Pointer to a CathDom object
** @@
****************************************************************************/
static AjPCathDom domainCathDomNew(ajint nsegments)
{
    AjPCathDom ret = NULL;
    ajint x;    

    AJNEW0(ret);
    
    ret->DomainID = ajStrNew();
    
    if(nsegments > 0)
    {
	AJCNEW0(ret->Start, nsegments);
	AJCNEW0(ret->End, nsegments);
	
	for(x=0; x<nsegments; x++)
	{
	    ret->Start[x] = ajStrNew();
	    ret->End[x]   = ajStrNew();
	}
    }
    else
    {
	ret->Start = NULL;
	ret->End   = NULL;
    }


    ret->NSegment = nsegments;

    return ret;
}





/* @funcstatic domainCathDomDel ***********************************************
**
** Destructor for CathDom object. 
**
** @param [w] ptr [AjPCathDom*] Cathdom object
**
** @return [void] 
** @@
****************************************************************************/
static void domainCathDomDel(AjPCathDom *ptr)
{
    AjPCathDom pthis = *ptr;
    
    ajint x;
    
    if(!pthis || !ptr)      /*(pthis==NULL)||(ptr==NULL)*/
	return;
 
    ajStrDel(&pthis->DomainID);
    
    if(pthis->NSegment)
    {
	for(x=0; x<pthis->NSegment; x++)
	{
	    ajStrDel(&pthis->Start[x]);
	    ajStrDel(&pthis->End[x]);
	}
	AJFREE(pthis->Start);
	AJFREE(pthis->End);
    
    }
    
    AJFREE(pthis);
    pthis=NULL;
    
    return;
}
  




/* @funcstatic domainCathNameNew **********************************************
**
** CathName object constructor.
**
** @return [AjPCathName] Pointer to a CathName object
** @@
****************************************************************************/
static AjPCathName domainCathNameNew(void)
{
    AjPCathName ret = NULL;
    
    AJNEW0(ret);
    
    
    ret->Id	     = ajStrNew();
    ret->Desc	     = ajStrNew();

    return ret;
}





/* @funcstatic domainCathNameDel **********************************************
**
** Destructor for CathName object. 
**
** @param [w] ptr [AjPCathName *] CathName object pointer
** @return [void] 
** @@
****************************************************************************/
static void domainCathNameDel(AjPCathName *ptr)
{
    AjPCathName pthis = *ptr;
    
    if(!pthis || !ptr)     /*(pthis==NULL)||(ptr==NULL)*/
  return;
  
    ajStrDel(&pthis->Id);
    ajStrDel(&pthis->Desc);
    
    AJFREE(pthis);
    pthis=NULL;
    
    return;
}
 
 
 
 
    
/* @funcstatic domainSortNameId ***********************************************
**
** Function to sort CathName objects by Id element.
**
** @param [r] cath1  [const void*] Pointer to AjOCathName object 1
** @param [r] cath2  [const void*] Pointer to AjOCathName object 2
**
** @return [ajint] -1 if Id1 should sort before Id2, +1 if the Id2 
** should sort first. 0 if they are identical in value.
** @@
****************************************************************************/
static ajint domainSortNameId(const void *cath1, const void *cath2)
{
    const AjPCathName p = NULL;
    const AjPCathName q = NULL;

    p = (*(AjPCathName const *)cath1);
    q = (*(AjPCathName const *)cath2);
    
    return ajStrCmpS(p->Id, q->Id);
}


 
  
   
/* @funcstatic domainSortDomainID *********************************************
**
** Function to sort CathDom objects by DomainID element.
**
** @param [r] DomID1  [const void*] Pointer to CathDom object 1
** @param [r] DomID2  [const void*] Pointer to CathDom object 2
**
** @return [ajint] -1 if DomID1 should sort before DomID2, +1 if the DomID2 
** should sort first. 0 if they are identical in value.
** @@
****************************************************************************/
static ajint domainSortDomainID(const void *DomID1, const void *DomID2)
{
    const AjPCathDom p = NULL;
    const AjPCathDom q = NULL;

    p = (*(AjPCathDom const *)DomID1);
    q = (*(AjPCathDom const *)DomID2);
    
    return ajStrCmpS(p->DomainID, q->DomainID);

}




/* ======================================================================= */
/* =========================== constructors ============================== */
/* ======================================================================= */

/* @section Constructors ****************************************************
**
** These constructors return a pointer to a new instance of an object.
**
****************************************************************************/


/* @func ajCathReadAllNew **************************************************
**
** Reads a DCF file (domain classification file) for CATH domains. This file
** is in DCF format (see documentation for DOMAINATRIX "cathparse"
** application) and creates a list of Cath objects for the entire content.
** 
** @param [u] inf  [AjPFile]     Pointer to CATH classification file
** 
** @return [AjPList] List of Cath objects. 
** @@
****************************************************************************/

AjPList  ajCathReadAllNew(AjPFile inf)
{
    AjPList ret = NULL;
    
    AjPCath cath_object = NULL;

    /* Check arg's */
    if((!inf))
    {
	ajWarn("Bad args passed to ajCathReadAllNew\n");
	return NULL;
    }

    ret = ajListNew();
    

    while((cath_object=ajCathReadCNew(inf, "*")))
	ajListPushAppend(ret, cath_object);
  
    return ret;
}




/* @func ajCathReadAllRawNew ***********************************************
**
** Reads the CATH parsable files (dir.cla.scop.txt & dir.des.scop.txt) and 
** writes a list of Cath objects.
**
** @param [u] cathf   [AjPFile] Cath class file
** @param [u] domf    [AjPFile] Cath domain file
** @param [u] namesf  [AjPFile] Output file
** @param [u] logf    [AjPFile] Log file
**
** @return [AjPList] List of Scop objects.
** @@
****************************************************************************/

AjPList   ajCathReadAllRawNew(AjPFile cathf, AjPFile domf, AjPFile namesf, 
			       AjPFile logf)
{ 
    AjPList ret = NULL;
    AjPStr CathNameLine    = NULL;  /* String used to hold line from namesf*/
    AjPStr CathDomLine     = NULL;  /* String used to hold line from domf  */
    AjPStr CathListLine    = NULL;  /* String used to hold line from cathf */
    AjPStr tmpDomainID     = NULL;  /* temp ptr to string to hold DomainId 
				       minus "0".                          */
    AjPStr tmpStringDomPtr = NULL;  /* temp ptr to string to hold DXX as a 
				       string.                             */
    AjPStr tmpNumDomPtr    = NULL;  /* temp ptr to string to hold number of 
				       domains as a string.                */ 
    ajint  tmpDomInt    = 0;        /* temp ptr to string to hold number of
				       domains as int.                     */  
    AjPStr tmpDomIDNumDom  = NULL;  /* temp ptr to string to hold DomainId
				       plus domain number before CathDom 
				       object created.                     */
    ajint  tmpNSegment     = 0;     /* temp ptr to string to hold NSegment
				       before CathDom object created.      */
    AjPStr NDomAsString    = NULL;  /* temp ptr to string to hold (d) 
				       number of domains (appended onto 
				       tmpDomainID).                       */
    AjPStr tmpNumString    = NULL;  /* temp string used to search CathName
				       objects. */
    AjPStr tmpNumString1   = NULL;  /* temp string used to search CathName 
				       objects - first part. */
    AjPStr tmpNumString2   = NULL;  /* temp string used to search CathName 
				       objects - second part. */

    AjPStr Search_DomainIDPtr = NULL;  /* temp search string used to search 
					  CathDomList. */
    const AjPStr StrTokPtr  = NULL;
    
    
    AjPCathDom *CathDomArray   = NULL; /* Array to hold sorted CathDomList  */
    AjPCathName *CathNameArray = NULL; /* Array to hold sorted CathNameList */
    
    /* Initialise Integers */   
    ajint idxCathDom = 0;    /* Index to CathDomList array */
    ajint dimCathDom = 0;    /* Dimension of CathDomList array */  
    ajint idxCathName= 0;    /* Index to CathNameList array */
    ajint dimCathName= 0;    /* Dimension of CathNameList array */  
    
    
    ajint intC  = 0;  /* Class number as int */ 
    ajint intA  = 0;  /* Architecture number as int */
    ajint intT  = 0;  /* Topology number as int */
    ajint intH  = 0;  /* Homologous Superfamily number as int */
    ajint intF  = 0;  /* Family number as int */
    ajint intNI = 0;  /* Near Identical family number as int */
    ajint intI  = 0;  /* Identical family number as int */  
    ajint d;          /*Declare int for looping through domains*/   
    ajint s;          /*Declare int for looping through segments*/ 
    ajint single_seg = 1;   /* Number of segments when no match founf in 
			      CathDomList */
    
    
    AjPCathName CathNamePtr = NULL;  /* Reusable CathName object pointer */
    AjPCathDom  CathDomPtr  = NULL;  /* Reusable CathDom object pointer */
    AjPCath CathPtr         = NULL;   /* Reusable CathList object pointer */
    
    
    AjPList  CathNameList =NULL; /* List containing ptrs to CathName objects */
    AjPList  CathDomList  =NULL; /* List containing ptrs to CathDom objects */
    
    
    AjPStrTok handle = NULL;
    AjPStr    tmptok = NULL;
    
    
    /* Intitialise strings */
    tmptok          = ajStrNew();
    CathNameLine    = ajStrNew();
    CathDomLine     = ajStrNew();
    CathListLine    = ajStrNew();
    tmpDomainID     = ajStrNew();  
    tmpStringDomPtr = ajStrNew();
    tmpNumDomPtr    = ajStrNew();
    tmpDomIDNumDom  = ajStrNew();
    NDomAsString    = ajStrNew();
    tmpNumString    = ajStrNew();
    tmpNumString1    = ajStrNew();
    tmpNumString2    = ajStrNew();
    Search_DomainIDPtr = ajStrNew(); 
    
    ret = ajListNew();    

    
    /* Create list for AjSCathName structures (CathNameList) to hold
       data from lines in CAT.names.all.v2.4  */
    
    
    CathNameList = ajListNew();
    CathDomList = ajListNew();
    
    
    /* Read all the lines in CAT.names.all.v2.4 and populate
       CathNameList */
    /* 1. Need a loop to read through every line in cathf 
       ... while(ajFileReadLine)
       {
	   2. Create a CathName structure ... CathNameNew
	       3. Extract data from line and write data structure   
		   4. Push the CathName pointer onto CathNameList ... 
       }
       */
    
    while(ajFileReadLine(namesf, &CathNameLine))
    {
    	CathNamePtr = domainCathNameNew();
	
	/*1st token is classification index e.g 0002.0160 */
	handle = ajStrTokenNewC(CathNameLine, " \t");
	ajStrTokenNextParseC(&handle, " \t", &(CathNamePtr)->Id);

	
	/*2nd token is domain code and should be discarded */
	ajStrTokenNextParseC(&handle, " \t", &tmptok);

	
	/*3rd token is classification text */
	ajStrTokenRestParse(&handle, &(CathNamePtr)->Desc);
	if(CathNamePtr->Desc->Ptr[0]==':')
	    ajStrCutStart(&(CathNamePtr)->Desc, 1);

	ajStrTokenDel(&handle);
	

	/*
	StrTokPtr = ajStrParseWhite(CathNameLine);		
	ajStrAssignS(&(CathNamePtr)->Id, StrTokPtr);
	StrTokPtr = ajStrParseWhite(NULL);		
	ajStrTokenRestParse(NULL, &StrTokPtr);
	ajStrAssignS(&(CathNamePtr)->Desc, StrTokPtr);
	if(CathNamePtr->Desc->Ptr[0]==':')
	    ajStrCutStart(&(CathNamePtr)->Desc, 1);
	    */

	/*
	ajFmtScanS(CathNameLine, "%S", &(CathNamePtr)->Id); 
	ajStrAssignSubS(&(CathNamePtr)->Desc, CathNameLine, 28, -1); 
	ajStrTrimWhite(&(CathNamePtr)->Desc);
	if(CathNamePtr->Desc->Ptr[0]==':')
	    ajStrCutStart(&(CathNamePtr)->Desc, 1);
	    */
	
	/* Push pointer to CathName object onto list*/
	ajListPush(CathNameList, CathNamePtr);
    } 
    
    
    
    /* Sort the list by cath classification number, AjPStr Id */

    /* Sort list using domainSortNameId function */    
    ajListSort(CathNameList, domainSortNameId); 
    /* make list into array and get array size - dimCathName */
    dimCathName = ajListToarray(CathNameList, (void ***) &CathNameArray); 
    
    
    /* We now have a list that we can do a binary search over */
    
    
    /* Create list of AjSCathDom structures (CathDomList) to hold 
       data from lines in domlist.v2.4 */
    
    
    while(ajFileReadLine(domf, &CathDomLine))
    {	
	/*1st token is DomainID e.g 1cuk00*/
	/* ajStrTok goes through each element of line*/
	StrTokPtr = ajStrParseWhite(CathDomLine);		
        
	/* Remove last num from string (0). Assign StrTokPtr to temp ptr 
	   tmpDomainID e.g. 1cuk0 */
	ajStrAssignSubS(&tmpDomainID, StrTokPtr, 0,4); 
	

	/*
	ajFmtPrint("tmpDomainID : %S\n",tmpDomainID );
	fflush(stdout);
	*/

	StrTokPtr = ajStrParseWhite(NULL);	/*2nd token is no. of domains*/
	/* Assign value of StrTokPtr (no. of domains) to tmpStringDomPtr */ 
	ajStrAssignS(&tmpStringDomPtr, StrTokPtr); 
	/* Remove first character (index 0 = the letter D) from 
	   tmpStringDomPtr and give to tmpDomInt */
	ajStrAssignSubS(&tmpNumDomPtr, tmpStringDomPtr, 1,2); 
	ajStrToInt(tmpNumDomPtr, &tmpDomInt);
	

	/*
	ajFmtPrint("tmpDomInt : %d\n",tmpDomInt );
	fflush(stdout);
	*/

	/*error-ajStrToInt(tmpDomIntPtr, &(tmpNumDomPtr)); */  
	/* Number of domains expressed as Int and assigned to tmpDomInt*/
	
	/*3rd token is no. of fragments, don't need this*/ 
	ajStrParseWhite(NULL);			
	
	
	for (d=1; d<=tmpDomInt; d++)	/* For each domain */
	{
	    /* Get the number of segments */
	    StrTokPtr = ajStrParseWhite(NULL); /* Token= no. of segments*/

/*	    ajFmtPrint("StrTokPtr : %S\n", StrTokPtr);
	    fflush(stdout); */
	    

	    /*Convert string containing no. of segs to int */
	    ajStrToInt(StrTokPtr, &(tmpNSegment)); 
	    
	    /* Create CathDom object giving tmpNSegment as argument */
	    CathDomPtr = domainCathDomNew(tmpNSegment); 
	    
	    
	    /* Converts value of d to a string */	
	    ajStrFromInt(&NDomAsString, d); 
	    if(d>1)
		ajStrCutEnd(&tmpDomainID, 1);
	    
		
	    /* Append no. of domains as a string (NDomAsString) onto end 
	       of tmpDomainID */
	    ajStrAppendS(&tmpDomainID, NDomAsString); 
	    /* Assign tmpDomainID to DomainID element in CathDom object */
	    ajStrAssignS(&(CathDomPtr->DomainID), tmpDomainID); 
	    
	    
	    
	    for(s=0; s < CathDomPtr->NSegment; s++) /* For each segment */
	    {
		/* get Start and End residue numbers for each segment */ 
		/* nth (starting at token no. 5) token is Chain of
                   starting residue*/
		ajStrParseWhite(NULL);		
		/* (n+1)th token is start of segment res number */
		StrTokPtr = ajStrParseWhite(NULL); 
		ajStrAssignS(&(CathDomPtr->Start[s]), StrTokPtr);
		    
		/* Assign Start res no. to Start element in AjPCathDom */
		/*error- ajStrToInt(StrTokPtr, &(CathDomPtr->Start[s]));*/ 
		/* (n+2)th token is "-" */
		ajStrParseWhite(NULL);		
		/* (n+3)th token is Chain of ending residue*/
		ajStrParseWhite(NULL);		
		/* (n+4)th token is end of segment res number */
		StrTokPtr = ajStrParseWhite(NULL); 
		ajStrAssignS(&(CathDomPtr->End[s]), StrTokPtr);
		    
		/* Assign End res no. to Start element in AjPCathDom */
		/*error-   ajStrToInt(StrTokPtr, &(CathDomPtr->End[s]));*/ 
		ajStrParseWhite(NULL);		/* (n+5)th token is "-" */

	    }

	    
	    
	    /* Read all the lines in domlist.v2.4 and populate CathDomList */
	    /* Push pointer to CathDom object onto list*/
	    ajListPush(CathDomList, CathDomPtr); 
	}
    }
    
    
    
    /* Sort the list by domain code (ajListSort by DomainId) 
       We now have a list that we can do a binary search over */
    
    /* Sort list using domainSortDomainID function */
    ajListSort(CathDomList, domainSortDomainID); 
    /* make list into array and get array size - dimCathDom */
    dimCathDom = ajListToarray(CathDomList, (void ***) &CathDomArray); 
    
    
    /* Start of main application loop */
    /* while there is a line to read from caths.list.v2.4, 
       read a line into a string ... ajFileReadLine*/ 
    while(ajFileReadLine(cathf, &CathListLine))
    {
	/* Extract DomainId from string and write to temp. variable 
	   - Search_DomainIDPtr */
		    
	/* DomainID held in temp string */
	ajFmtScanS(CathListLine, "%S", &(Search_DomainIDPtr)); 
		    
		    
	/* Binary search of Search_StringPtr against DomainID element 
	   in CathDomList */
		    
	/* Binary search of Search_DomainIDPtr over array of 
	   CathDom objects */
	idxCathDom = domainCathDomBinSearch(Search_DomainIDPtr, 
					    CathDomArray, dimCathDom); 
	/* sorted by AjPStr Id */
	if(idxCathDom != -1		/*match found*/)
	{
	    /* Extract number of segments  */
	    /* Extract number of segments from CathDom and assign 
	       to tmpNSegment */
	    (tmpNSegment) = (CathDomArray[idxCathDom]->NSegment); 
	    						   

	    /* Allocate the Cath object, AjXyzCathNew */
	    
	    /* Create Cath object giving tmpNSegment as argument */
	    CathPtr = ajCathNew(tmpNSegment); 


	    
	    /* Assign DomainId from CathDom to Cath objects */ 	   
	    ajStrAssignS(&(CathPtr->DomainID), 
		      CathDomArray[idxCathDom]->DomainID); 
	    /* Assign number of segments to NSegment element in Cath object */
	    (CathPtr->NSegment) = (CathDomArray[idxCathDom]->NSegment); 
	    
	   
	    /* Write the number of segments and start and end points */
	    /*
	    printf("Number of segments for CathPtr : %d\n", 
	    CathPtr->NSegment);
	    printf("Number of segments for CathDomPtr : %d\n\n", 
	    CathDomPtr->NSegment);
            */
	    for(s=0; s<(CathPtr->NSegment); s++) /* For each segment */
	    {
		/* get Start and End residue numbers for each segment */ 

		/* Assign value of start from CathDom to CathPtr */
		ajStrAssignS(&(CathPtr->Start[s]), 
			  CathDomArray[idxCathDom]->Start[s]); 
		/* Assign value of end from CathDom to CathPtr */
		ajStrAssignS(&(CathPtr->End[s]), 
			  CathDomArray[idxCathDom]->End[s]); 
			
	    }      
	}
	/* no match found => only one domain in protein */
	else				
	{
	    /* Presume that domain contains a single segment, 
	       single_seg = 1*/

	    /* Allocate the Cath object, AjXyzCathNew */
	    
	    /* Create Cath object giving tmpNSegment as argument */
	    CathPtr = ajCathNew(single_seg); 
	    
	    /* Assign DomainId from Search_DomainIDPtr */
	    ajStrAssignS(&(CathPtr->DomainID), Search_DomainIDPtr); 
	    
	    /* Assign the number of segments as 1 */
	    /* Assign number of segments to NSegment element in Cath 
	       object from single_seg */
	    (CathPtr->NSegment) = (single_seg); 
	               
		  
	    /* get Start and End residue numbers for each segment */ 
	    ajStrAssignC(&(CathPtr->Start[0]), ".");
	    ajStrAssignC(&(CathPtr->End[0]), ".");

	    /* Assign value of start to "." */ 
	    /*urrggh! ((CathPtr->Start[0]) = ".");*/
	    /* Assign value of end to "." */ 		       
	    /*yeek!	  ((CathPtr->End[0]) = ".");*/
	    
	}


	/* Extract Pdb code from DomainId */
	ajStrAssignSubS(&(CathPtr->Pdb), CathPtr->DomainID, 0,3);
	
	/* Extract chain identifer from DomainId */
	CathPtr->Chain=ajStrGetCharPos(CathPtr->DomainID, 4);
		    
	/* ajStrChar char from string */
	/* error-ajStrAssignSubS(&(CathPtr->Chain), CathPtr->DomainID, 4,4);*/
	
        /* Extract length of domain from string */
	/* Take the 9th element of line and assign to Length in Cath object */
	ajFmtScanS(CathListLine, "%*S %*d %*d %*d %*d %*d %*d %*d %d", 
		   &(CathPtr->Length)); 

	
        /* Extract ajint Class_Id, Arch_Id, Topology_Id, Superfamily_Id, 
	   NIFamily_Id, Family_Id, IFamily_Id from string and write into
	   AjPCath */
	ajFmtScanS(CathListLine, "%*S %d %d %d %d %d %d %d", 
		   &intC, &intA, &intT, &intH, &intF, &intNI, &intI);
	
	(CathPtr->Class_Id)       = (intC); 
	(CathPtr->Arch_Id)        = (intA);
	(CathPtr->Topology_Id)   = (intT);
	(CathPtr->Superfamily_Id) = (intH);
	(CathPtr->Family_Id)      = (intF);
	(CathPtr->NIFamily_Id)    = (intNI);
	(CathPtr->IFamily_Id)        = (intI);
	          
	    
	/* Construct number string for SUPERFAMILY from Class_Id, Arch_Id, 
	   Topology_Id, 
	   Superfamily_Id and store in temp. variable (format X.XX.XX.XX) */ 
	    
	/* Make string containg CATH id numbers */ 
	ajFmtPrintS(&tmpNumString, "%d.%d.%d.%d", intC, intA, intT, intH); 

	/* Binary search using temp. variable in AjSCathName */
	
	ajFmtPrintF(logf, "%S\n", tmpNumString);
	
	/* Binary search of tmpNumString over array of CathName objects */
        idxCathName = domainCathNameBinSearch(tmpNumString, CathNameArray, 
					   dimCathName); 
	
	if ( idxCathName != -1)		/*match found*/
        {
	    /* Extract Superfamily string and write into AJPCath */
            ajStrAssignS(&(CathPtr->Superfamily),
		      (CathNameArray[idxCathName]->Desc)); 
        }
        else				/*no match found*/
        {
            /* Write Superfamily string as a '.'  */
	    ajStrAssignC(&(CathPtr->Superfamily), ".");
	    
	    /*error	    (CathPtr->Superfamily) = "."); */
        }               
        
	
	/* Construct number string for TOPOLOGY from Class_Id, Arch_Id, Topology_Id, 
	   and store in temp. variable (format X.XX.XX) */ 

	/* Make string containg CAT id numbers */
        ajFmtPrintS(&tmpNumString, "%d.%d.%d", intC, intA, intT); 
	
	/* Binary search using temp. variable in AjSCathName */
	ajFmtPrintF(logf, "%S\n", tmpNumString);
	/* Binary search of tmpNumString over array of CathName objects */
        idxCathName = domainCathNameBinSearch(tmpNumString, CathNameArray, 
					   dimCathName); 
        
	if ( idxCathName != -1)		/*match found*/
        {
	    /* Extract Topology string  and write into AJPCath*/
	    ajStrAssignS(&(CathPtr->Topology), CathNameArray[idxCathName]->Desc);
        }
        else				/*no match found*/
        {
            /* Write topology as a '.'  */
	    ajStrAssignC(&(CathPtr->Topology), ".");
	    /*error	    CathPtr->Topology) = "."); */
        }                         


	/* Construct number string for ARCHITECTURE from Class_Id, Arch_Id, 
	   and store in temp. variable */ 
	/* Class and Architecture numbers in domlist.v2.4 are in format 
	   XXXX.XXXX */
	/*
	if(intC < 10)
	    ajFmtPrintS(&tmpNumString1, "000%d", intC); 
	else
	    ajFatal("MIKE GIVE A ERROR MESSAGE");
	if(intA < 10)
	    ajFmtPrintS(&tmpNumString2, "000%d", intA); 
	else if(intA < 100)
	    ajFmtPrintS(&tmpNumString2, "00%d", intA); 
	else if(intA < 1000)
	    ajFmtPrintS(&tmpNumString2, "0%d", intA); 
	else if(intA < 10000)
	    ajFmtPrintS(&tmpNumString2, "%d", intA); 
	else
	    ajFatal("MIKE GIVE A ERROR MESSAGE");
	ajFmtPrintS(&tmpNumString, "%S.%S", tmpNumString1, tmpNumString2);
	*/
	ajFmtPrintS(&tmpNumString, "%04d.%04d", intC, intA);
	

	/* Make string containg CA id numbers */
	/*	ajFmtPrintS(&tmpNumString, "000%d.%d", intC, intA);*/ 
	
	/* Binary search using temp. variable in AjSCathName */
	ajFmtPrintF(logf, "%S\n", tmpNumString);
	/* Binary search of tmpNumString over array of CathName objects */
        idxCathName = domainCathNameBinSearch(tmpNumString, CathNameArray, 
					   dimCathName); 
	
        
	if ( idxCathName != -1 )	/*match found*/
        {
	    /* Extract Architecture string and write into AJPCath*/
	    ajStrAssignS(&(CathPtr->Architecture), 
		      CathNameArray[idxCathName]->Desc);
        }
        else				/*no match found*/
        {
            /* Write architecture as a '.' */
	    ajStrAssignC(&(CathPtr->Architecture), ".");
	    /*error	    (CathPtr->Architecture) = "."); */
        }    


	/* Construct number string for CLASS from Class_Id and store in 
	   temp. variable */ 
	if(intC < 10)
	    ajFmtPrintS(&tmpNumString, "%04d", intC); 
	else
	    ajFatal("MIKE GIVE A ERROR MESSAGE");




	/* Binary search using temp. variable in AjSCathName */
	ajFmtPrintF(logf, "%S\n", tmpNumString);
	/* Binary search of tmpNumString over array of CathName objects */
        idxCathName = domainCathNameBinSearch(tmpNumString, CathNameArray, 
					   dimCathName); 
        
	
	if ( idxCathName != -1)		/*match found*/
        {
	    /* Extract Class string and write into AJPCath*/
	    ajStrAssignS(&(CathPtr->Class), CathNameArray[idxCathName]->Desc);
        }
        else				/*no match found*/
        {
            /* Write class as a '.' */
	    ajStrAssignC(&(CathPtr->Class), ".");
	    /*error	    (CathPtr->Class) = "."); */
        }    

	/* Push the Cath object onto list */
	ajListPushAppend(ret, CathPtr);

    } /* End of main application loop */
    
    
    /* Free the memory for the list and nodes in 
       list of AjSCathName structures (ajListFree) */
    while(ajListPop(CathNameList, (void **) &CathNamePtr))
	domainCathNameDel(&CathNamePtr);
    ajListFree(&CathNameList);
    
    /* Free the memory for the list and nodes in 
       list of AjSCathDom structures (ajListFree)  */
    while(ajListPop(CathDomList, (void **) &CathDomPtr))
	domainCathDomDel(&CathDomPtr);
    ajListFree(&CathDomList);
    

    
    /* Tidy up */
    ajStrDel(&tmptok);
    ajStrDel(&CathNameLine);
    ajStrDel(&CathDomLine);
    ajStrDel(&CathListLine);
    ajStrDel(&tmpStringDomPtr);
    ajStrDel(&tmpNumDomPtr);
    ajStrDel(&tmpDomainID);
    ajStrDel(&tmpDomIDNumDom);
    ajStrDel(&tmpNumString);
    ajStrDel(&tmpNumString1);
    ajStrDel(&tmpNumString2);
    AJFREE(CathDomArray);
    AJFREE(CathNameArray);
    ajStrDel(&Search_DomainIDPtr);
    ajStrDel(&NDomAsString);
    


    return ret;
}





/* @func ajCathReadCNew ****************************************************
**
** Read a Cath object from a DCF file (domain classification file) in DCF 
** format (see documentation for 
** DOMAINATRIX "cathparse" application).
**
** @param [u] inf   [AjPFile]  Input file stream
** @param [r] entry [const char*] CATH id of entry to retrieve (or "*" for next
**                             domain in file).
**
** @return [AjPCath] Cath object.
** @category new [AjPCath] Cath constructor from reading dcf format file.
** @@
****************************************************************************/

 AjPCath ajCathReadCNew(AjPFile inf, const char *entry)
{
    AjPCath ret = NULL;
    
    static AjPRegexp exp1  = NULL;
    static AjPRegexp exp2  = NULL;
    static AjPStr domainID = NULL;
    static AjPStr pdb      = NULL;
    static AjPStr class    = NULL;
    
    static AjPStr architecture = NULL;
    static AjPStr topology     = NULL;
    static AjPStr superfamily  = NULL;

    static AjPStr line   = NULL; 	
    static AjPStr str    = NULL;
    static AjPStr stmp   = NULL;
    static AjPStr tentry = NULL;

    static AjPStr Acc     = NULL;         
    static AjPStr Spr     = NULL;          
    static AjPStr SeqPdb  = NULL;	
    static AjPStr SeqSpr  = NULL;	
    
    AjBool ok = ajFalse; 
    
    ajint idx = 0;     
    ajint n   = 0;                   
    ajint  Length;		/* No. of residues in domain */

    ajint  Startd;      /* Start of sequence relative to full length 
			    swissprot sequence */
    ajint  Endd;        /* End of sequence relative to full length 
			    swissprot sequence */    

    /* Only initialise strings if this is called for the first time */
    if(!line)
    {
	domainID        = ajStrNew();
	pdb             = ajStrNew();
	class           = ajStrNew();
	architecture    = ajStrNew();
	topology        = ajStrNew();
	superfamily     = ajStrNew();
	line		= ajStrNew();
	str		= ajStrNew();
	tentry          = ajStrNew();
	exp1    = ajRegCompC("^([^ \t\r\n]+)[ \t\n\r]+");
	exp2    = ajRegCompC(
		   "^([A-Za-z0-9.]+)[ ]*[^ \t\r\n]+[ ]*([0-9.-]+)[ ]*"
		     "[^ \t\r\n]+[ ]*([0-9.-]+)");
	Acc    = ajStrNew();
	Spr     = ajStrNew();
    }
    SeqSpr  = ajStrNew();
    SeqPdb  = ajStrNew();

    
    ajStrAssignC(&tentry,entry);    
    ajStrFmtUpper(&tentry);	   
    
    while((ok=ajFileReadLine(inf,&line)))
    {
	if(!ajStrPrefixC(line,"ID   "))
	    continue;
	
	ajFmtScanS(line, "%*S %S", &stmp);
	
	if(ajStrMatchWildS(stmp,tentry))
	    break;
    }

    
    if(!ok)
	return NULL;
    
    
    while(ok && !ajStrPrefixC(line,"//"))
    {
	if(ajStrPrefixC(line,"XX"))
	{
	    ok = ajFileReadLine(inf,&line);
	    continue;
	}
	
	ajRegExec(exp1,line);
	ajRegPost(exp1,&str);
	

	if(ajStrPrefixC(line,"ID"))
	    ajStrAssignS(&domainID,str);
	else if(ajStrPrefixC(line,"EN"))
	    ajStrAssignS(&pdb,str);
	else if(ajStrPrefixC(line,"CL"))
	    ajStrAssignS(&class,str);
	else if(ajStrPrefixC(line,"AR"))
	{
	    ajStrAssignS(&architecture,str);
	    while(ajFileReadLine(inf,&line))
	    {
		if(ajStrPrefixC(line,"XX"))
		    break;
		ajStrAppendC(&architecture,ajStrGetPtr(line)+3);
	    }
	    ajStrRemoveWhiteExcess(&architecture);
	}
	else if(ajStrPrefixC(line,"TP"))
	{
	    ajStrAssignS(&topology,str);
	    while(ajFileReadLine(inf,&line))
	    {
		if(ajStrPrefixC(line,"XX"))
		    break;
		ajStrAppendC(&topology,ajStrGetPtr(line)+3);
	    }
	    ajStrRemoveWhiteExcess(&topology);
	}
	else if(ajStrPrefixC(line,"SF"))
	{
	    ajStrAssignS(&superfamily,str);
	    while(ajFileReadLine(inf,&line))
	    {
		if(ajStrPrefixC(line,"XX"))
		    break;
		ajStrAppendC(&superfamily,ajStrGetPtr(line)+3);
	    }
	    ajStrRemoveWhiteExcess(&superfamily);
	}
	else if(ajStrPrefixC(line,"NR"))  
	{
	    ajFmtScanS(line, "%*S %d", &Length);
	}
	else if(ajStrPrefixC(line,"NC"))
	{
	    ajStrToInt(str,&n);
	    (ret) = ajCathNew(n);
	    ajStrAssignS(&(ret)->DomainID,domainID);
	    ajStrAssignS(&(ret)->Pdb,pdb);
	    ajStrAssignS(&(ret)->Class,class);
	    ajStrAssignS(&(ret)->Architecture,architecture);
	    ajStrAssignS(&(ret)->Topology, topology);
	    ajStrAssignS(&(ret)->Superfamily,superfamily);
	    ret->Length = Length;
	    ajStrAssignS(&(ret)->Acc,Acc);
	    ajStrAssignS(&(ret)->Spr,Spr);
	    ajStrAssignS(&(ret)->SeqPdb,SeqPdb);
	    ajStrAssignS(&(ret)->SeqSpr,SeqSpr);
	}
	else if(ajStrPrefixC(line,"CN"))
	    ajFmtScanS(line, "%*S %*c%d", &idx);
	else if(ajStrPrefixC(line,"CH"))
	{
	    if(!ajRegExec(exp2,str))
		return NULL;
	    ajRegSubI(exp2,1,&stmp);
	    (ret)->Chain = *ajStrGetPtr(stmp);
	    ajRegSubI(exp2,2,&str);
	    ajStrAssignC(&(ret)->Start[idx-1],ajStrGetPtr(str)); 

	    ajRegSubI(exp2,3,&str);
	    ajStrAssignC(&(ret)->End[idx-1],ajStrGetPtr(str)); 

	}
	/* Sequence from pdb file */
	else if(ajStrPrefixC(line,"DS"))
	{
	    while((ok=ajFileReadLine(inf,&line)) && !ajStrPrefixC(line,"XX"))
		ajStrAppendC(&SeqPdb,ajStrGetPtr(line));
	    ajStrRemoveWhite(&SeqPdb);
	    continue;
	}
	/* Sequence from swissprot */
	else if(ajStrPrefixC(line,"SQ"))
	{
	    while((ok=ajFileReadLine(inf,&line)) && !ajStrPrefixC(line,"XX"))
		ajStrAppendC(&SeqSpr,ajStrGetPtr(line));
	    ajStrRemoveWhite(&SeqSpr);
	    continue;
	}
	
	/* Accession number */
	else if(ajStrPrefixC(line,"AC"))
	    ajFmtScanS(line, "%*s %S", &Acc);
	/* Swissprot code */
	else if(ajStrPrefixC(line,"SF"))
	    ajFmtScanS(line, "%*s %S", &Spr);
	/* Start and end relative to swissprot sequence */
	else if(ajStrPrefixC(line,"RA"))
	    ajFmtScanS(line, "%*s %d %*s %d", &Startd, &Endd);
	
	ok = ajFileReadLine(inf,&line);
    }
 
    ajStrDel(&SeqSpr);
    ajStrDel(&SeqPdb);

    return ret;
}




/* @func ajCathReadNew *****************************************************
**
** Read a Cath object from a DCF file (domain classification file) which is
** in DCF format (see documentation for 
** DOMAINATRIX "cathparse" application).
**
** @param [u] inf   [AjPFile]  Input file stream
** @param [r] entry [const AjPStr] CATH id of entry to retrieve
**                             (or "*" for next domain in file).
**
** @return [AjPCath] Cath object.
** @category new [AjPCath] Cath constructor from reading dcf format file.
** @@
****************************************************************************/

 AjPCath ajCathReadNew(AjPFile inf, const AjPStr entry)
{
    AjPCath ret = NULL;

    if((ret = ajCathReadCNew(inf, entry->Ptr)))
	return ret;
    else 
	return NULL;
}




/* @func ajDomainReadAllNew **************************************************
**
** Reads the DCF file (domain classification file; see documentation 
** for DOMAINATRIX "scopparse" application) and creates a list of Domain 
** objects for the entire content.
** 
** @param [u] inf  [AjPFile]     Pointer to DOMAIN classification file
** 
** @return [AjPList] List of domain objects or NULL (file read problem).
** @@
****************************************************************************/

AjPList  ajDomainReadAllNew(AjPFile inf)
{
    AjPList ret = NULL;
    
    AjPDomain domain_object = NULL;
    ajint     type = 0;


    /* Check arg's */
    if((!inf))
    {
	ajWarn("Bad args passed to ajDomainReadAllNew\n");
	return NULL;
    }

    
    type = ajDomainDCFType(inf);

    if((type != ajSCOP) && (type != ajCATH))
    {
	ajWarn("Could not ascertain domain type in ajDomainReadNew");
	return NULL;
    }


    ret = ajListNew();
    
    if(type == ajSCOP)
	while((domain_object->Scop=ajScopReadCNew(inf, "*")))
	    ajListPushAppend(ret, domain_object);
    else
	while((domain_object->Cath=ajCathReadCNew(inf, "*")))
	    ajListPushAppend(ret, domain_object);

    return ret;
}




/* @func ajScopReadAllNew **************************************************
**
** Reads the DCF file (domain classification file) of SCOP domains. The DCF
** file is in DCF format (see documentation 
** for DOMAINATRIX "scopparse" application). The function creates a list of 
** cop objects for the entire content.
** 
** @param [u] inf  [AjPFile]     Pointer to SCOP classification file
** 
** @return [AjPList] List of scop objects or NULL (file read problem).
** @@
****************************************************************************/

AjPList  ajScopReadAllNew(AjPFile inf)
{
    AjPList ret = NULL;
    
    AjPScop scop_object = NULL;

    /* Check arg's */
    if((!inf))
    {
	ajWarn("Bad args passed to ajScopReadAllNew\n");
	return NULL;
    }

    ret = ajListNew();
    

    while((scop_object=ajScopReadCNew(inf, "*")))
	ajListPushAppend(ret, scop_object);
  
    return ret;
}




/* @func ajScopReadAllRawNew ***********************************************
**
** Reads the SCOP parsable files (dir.cla.scop.txt & dir.des.scop.txt) and 
** creates a list of Scop objects.
**
** @param [u] claf      [AjPFile] Scop class file
** @param [u] desf      [AjPFile] Scop description file
** @param [r] omit      [AjBool]  True if domains of 1 chain only are output
**
** @return [AjPList] List of Scop objects.
** @@
****************************************************************************/

AjPList   ajScopReadAllRawNew(AjPFile claf, AjPFile desf, AjBool omit)
{
    AjPScopcla cla=NULL;   
    AjPScopdes des=NULL;  
    AjPScopdes *desarr=NULL;
    AjPScop tmp       = NULL;
    
    AjPList  clalist=NULL;
    AjPList  deslist=NULL;
    AjPList      ret=NULL;    

    AjBool   nooutput=ajFalse;
    char     chn;
    
    
    ajint  dim=0;  /* Dimension of array */
    ajint  idx=0;  /* Index into array */
    ajint  i=0;
    

    clalist = ajListNew();
    deslist = ajListNew();
        ret = ajListNew();


    


    /* Read the dir.cla.scop.txt file */ 
    while((cla = domainScopclaReadC(claf, "*")))
    {
	ajListPushAppend(clalist, cla);
/*	ajFmtPrint(" %d ", cla->Domdat); */
    }
    
    
    
    /* Read the dir.des.scop.txt file, sort the list by Sunid
       and convert to an array */
    while((des = domainScopdesReadC(desf, "*")))
    {
	ajListPush(deslist, des);
/*	ajFmtPrint("%d\n", des->Sunid); */
    }
    

    ajListSort(deslist, domainScopdesCompSunid);
    dim=ajListToarray(deslist, (void ***) &desarr);
    

    while(ajListPop(clalist, (void **)&cla))
    {
	if(omit)
	{
	    if(cla->N > 1)
	    {
		chn=cla->Chain[0];
		for(nooutput=ajFalse, i=1;i<cla->N;i++)
		    if(chn != cla->Chain[i])
		    {
			nooutput=ajTrue;
			break;
		    }
		if(nooutput)
		    continue;
	    }
	}
	
	tmp = ajScopNew(cla->N);
	ajStrAssignS(&tmp->Entry, cla->Entry);
	ajStrAssignS(&tmp->Pdb, cla->Pdb);

	tmp->Sunid_Class       = cla->Class;
	tmp->Sunid_Fold        = cla->Fold;
	tmp->Sunid_Superfamily = cla->Superfamily;
	tmp->Sunid_Family      = cla->Family;
	tmp->Sunid_Domain      = cla->Domain;
	tmp->Sunid_Source      = cla->Source;
	tmp->Sunid_Domdat      = cla->Domdat;


	idx = domainScopdesBinSearch(cla->Class,  desarr, dim);
	ajStrAssignS(&tmp->Class, desarr[idx]->Desc);

	idx = domainScopdesBinSearch(cla->Fold,  desarr, dim);
	ajStrAssignS(&tmp->Fold, desarr[idx]->Desc);

	idx = domainScopdesBinSearch(cla->Superfamily,  desarr, dim);
	ajStrAssignS(&tmp->Superfamily, desarr[idx]->Desc);

	idx = domainScopdesBinSearch(cla->Family,  desarr, dim);
	ajStrAssignS(&tmp->Family, desarr[idx]->Desc);

	idx = domainScopdesBinSearch(cla->Domain,  desarr, dim);
	ajStrAssignS(&tmp->Domain, desarr[idx]->Desc);

	idx = domainScopdesBinSearch(cla->Source,  desarr, dim);
	ajStrAssignS(&tmp->Source, desarr[idx]->Desc);

	for(i=0;i<cla->N;++i)
	{
	    tmp->Chain[i] = cla->Chain[i];
	    ajStrAssignS(&tmp->Start[i], cla->Start[i]);
	    ajStrAssignS(&tmp->End[i], cla->End[i]);	    
	}
	

	ajListPushAppend(ret, tmp);
	

	domainScopclaDel(&cla);
    
    }

    while(ajListPop(deslist, (void **)&des))
	domainScopdesDel(&des);
    
    /* Tidy up */
    AJFREE(desarr);
    ajListFree(&clalist);
    ajListFree(&deslist);

    
    return ret;
}




/* @func ajCathNew **********************************************************
**
** Cath object constructor. Fore-knowledge of the number of chain segments
** the domain is comprised of is required. 
**
** @param [r] n [ajint] No. of chain segments
** 
** @return [AjPCath] Pointer to a Cath object
** @category new [AjPCath] Cath default constructor.
**
****************************************************************************/

AjPCath ajCathNew(ajint n)
{
    AjPCath ret = NULL;
    ajint x = 0;
    
    AJNEW0(ret);

    
    ret->DomainID     = ajStrNew();
    ret->Pdb          = ajStrNew();  
    ret->Class        = ajStrNew();  
    ret->Architecture = ajStrNew();  
    ret->Topology     = ajStrNew();  
    ret->Superfamily  = ajStrNew();  
    ret->Acc         = ajStrNew();
    ret->Spr         = ajStrNew();
    ret->SeqPdb      = ajStrNew();
    ret->SeqSpr      = ajStrNew();
    
    if(n > 0)
    {
	AJCNEW0(ret->Start, n);
	AJCNEW0(ret->End, n);
	
	for(x=0; x<n; x++)
	{
	    ret->Start[x] = ajStrNew();
	    ret->End[x]   = ajStrNew();
	}
    }
    
    ret->NSegment = n;
    
    return ret;
}



/* @func ajDomainNew ********************************************************
**
** Domain object constructor. Fore-knowledge of the number of chains (SCOP
** domains) or chain segments (CATH domains) the domain is comprised of is 
** required. This is normally called by the ajDomainReadCNew / 
** ajDomainReadNew functions.
**
** @param [r] n [ajint] Number of chains (SCOP) or chain segments (CATH)
** @param [r] type [ajint]  Either ajSCOP (1) or ajCATH (2)
**
** @return [AjPDomain] Pointer to a Domain object
** @category new [AjPDomain] Domain default constructor.
** @@
****************************************************************************/

AjPDomain ajDomainNew(ajint n, ajint type)
{
    AjPDomain ret = NULL;

    
    if((type != ajSCOP) && (type != ajCATH))
    {
	ajWarn("Bad args passed to ajDomainNew");
	return NULL;
    }
    
    AJNEW0(ret);

    if(type == ajSCOP)
    {
	ret->Scop = ajScopNew(n);
	ret->Type = ajSCOP;
    }
    else
    {
	ret->Cath = ajCathNew(n);
	ret->Type = ajCATH;
    }
    

    return ret;
}



/* @func ajScopNew **********************************************************
**
** Scop object constructor. Fore-knowledge of the number of chains is 
** required. This is normally called by the ajScopReadCNew / ajScopReadNew
** functions.
**
** @param [r] chains [ajint] Number of chains
**
** @return [AjPScop] Pointer to a Scop object
** @category new [AjPScop] Scop default constructor.
** @@
****************************************************************************/

AjPScop ajScopNew(ajint chains)
{

    AjPScop ret = NULL;
    ajint i;

    AJNEW0(ret);

    ret->Entry       = ajStrNew();
    ret->Pdb         = ajStrNew();
    ret->Class       = ajStrNew();
    ret->Fold        = ajStrNew();
    ret->Superfamily = ajStrNew();
    ret->Family      = ajStrNew();  
    ret->Domain      = ajStrNew();  
    ret->Source      = ajStrNew();
    ret->Acc         = ajStrNew();
    ret->Spr         = ajStrNew();
    ret->SeqPdb      = ajStrNew();
    ret->SeqSpr      = ajStrNew();
    ret->Sse         = ajStrNew();
    ret->Sss         = ajStrNew();

    if(chains)
    {
	ret->Chain=ajCharNewRes(chains);
	AJCNEW0(ret->Start,chains);
	AJCNEW0(ret->End,chains);
	for(i=0; i<chains; i++)
	{
	    ret->Start[i]=ajStrNew();
	    ret->End[i]=ajStrNew();
	}
    }

    ret->N = chains;

    return ret;
}



/* @func ajDomainReadNew ****************************************************
**
** Read a Domain object from a DCF file (domain classification file - see 
** documentation for DOMAINATRIX "scopparse" application).
**
** @param [u] inf   [AjPFile] Input file stream.
** @param [r] entry [const AjPStr]  Domain id of domain to read (or "*" for 
**                   next domain in file).
**
** @return [AjPDomain] Domain object. 
** @category new [AjPDomain] Domain constructor from reading dcf file.
** @@
****************************************************************************/

AjPDomain ajDomainReadNew(AjPFile inf, const AjPStr entry)
{
    AjPDomain ret  = NULL;
    ajint     type = 0;
    
    type = ajDomainDCFType(inf);

    if((type != ajSCOP) && (type != ajCATH))
    {
	ajWarn("Could not ascertain domain type in ajDomainReadNew");
	return NULL;
    }

    AJNEW0(ret);

    if(type == ajSCOP)
	ret->Scop = ajScopReadCNew(inf,ajStrGetPtr(entry));
    else
	ret->Cath = ajCathReadCNew(inf,ajStrGetPtr(entry));
    
    return ret;
}




/* @func ajScopReadNew *****************************************************
**
** Read a Scop object from a DCF file (domain classification file). The DCF
** file is in DCF format (see documentation for 
** DOMAINATRIX "scopparse" application).
**
** @param [u] inf   [AjPFile] Input file stream.
** @param [r] entry [const AjPStr]  SCOP id of domain to read (or "*" for next 
**                            domain in file).
**
** @return [AjPScop] Scop object. 
** @category new [AjPScop] Scop constructor from reading dcf format file.
** @@
****************************************************************************/

AjPScop ajScopReadNew(AjPFile inf, const AjPStr entry)
{
    AjPScop ret = NULL;
    
    ret = ajScopReadCNew(inf,ajStrGetPtr(entry));
    
    return ret;
}


/* @func ajDomainReadCNew ***************************************************
**
** Read a Domain object from a DCF (domain classification file; see 
** documentation for DOMAINATRIX "scopparse" application). 
**
** @param [u] inf   [AjPFile]  Input file stream
** @param [r] entry [const char*]    Domain id of domain to parse
** @param [r] dtype [ajint]    Type of domain (ajSCOP or ajCATH) in the DCF
** file, or '0' if not sure.
**
** @return [AjPDomain] Domain object or NULL (file read problem).
** @category new [AjPDomain] Domain constructor from reading dcf file.
** @@
****************************************************************************/

AjPDomain ajDomainReadCNew(AjPFile inf, const char *entry, ajint dtype)
{
    AjPDomain ret  = NULL;
    ajint     type = 0;
    
    if(!dtype)
	type = ajDomainDCFType(inf);
    else
	type = dtype;
    
    if((type != ajSCOP) && (type != ajCATH))
    {
	ajWarn("Could not ascertain domain type in ajDomainReadCNew");
	return NULL;
    }

    AJNEW0(ret);
    ret->Type = type;
    

    if(ret->Type == ajSCOP)
	ret->Scop = ajScopReadCNew(inf,entry);
    else
	ret->Cath = ajCathReadCNew(inf,entry);
    
    if(!ret->Scop && !ret->Cath)
    {
	AJFREE(ret);
	return NULL;
    }
    
    return ret;
}



/* @func ajScopReadCNew ****************************************************
**
** Read a Scop object from a DCF file (domain classification file) in DCF
** format (see documentation for 
** DOMAINATRIX "scopparse" application).
**
** @param [u] inf   [AjPFile]  Input file stream
** @param [r] entry [const char*]    SCOP id of domain to parse
**
** @return [AjPScop] Scop object or NULL (file read problem).
** @category new [AjPScop] Scop constructor from reading dcf format file.
** @@
****************************************************************************/

AjPScop ajScopReadCNew(AjPFile inf, const char *entry)
{
    AjPScop ret = NULL;
    
    static AjPRegexp exp1 = NULL;
    static AjPRegexp exp2 = NULL;
    static AjPStr line    = NULL;
    static AjPStr str     = NULL;
    static AjPStr xentry  = NULL;
    static AjPStr source  = NULL;
    static AjPStr class   = NULL;
    static AjPStr fold    = NULL;
    static AjPStr super   = NULL;
    static AjPStr family  = NULL;
    static AjPStr domain  = NULL;
    static AjPStr pdb     = NULL;
    static AjPStr tentry  = NULL;
    static AjPStr stmp    = NULL;
    static AjPStr Acc     = NULL;         
    static AjPStr Spr     = NULL;          
    static AjPStr SeqPdb  = NULL;	
    static AjPStr SeqSpr  = NULL;	
    static AjPStr sse     =NULL;
    static AjPStr sss     =NULL;

    AjBool ok             = ajFalse;
    
    const char *p;
    ajint idx = 0;
    ajint n   = 0;
    ajint  Startd;      /* Start of sequence relative to full length 
			    swissprot sequence */
    ajint  Endd;        /* End of sequence relative to full length 
			    swissprot sequence */

    ajint  Sunid_Class;         /* SCOP sunid for class */
    ajint  Sunid_Fold;          /* SCOP sunid for fold */
    ajint  Sunid_Superfamily;   /* SCOP sunid for superfamily */
    ajint  Sunid_Family;        /* SCOP sunid for family */
    ajint  Sunid_Domain;        /* SCOP sunid for domain */  
    ajint  Sunid_Source;        /* SCOP sunid for species */
    ajint  Sunid_Domdat;        /* SCOP sunid for domain data */


    /* Only initialise strings if this is called for the first time */
    if(!line)
    {
	str     = ajStrNew();
	xentry  = ajStrNew();
	pdb     = ajStrNew();
	source  = ajStrNew();
	class   = ajStrNew();
	fold    = ajStrNew();
	super   = ajStrNew();
	family  = ajStrNew();
	domain  = ajStrNew();
	line    = ajStrNew();
	tentry  = ajStrNew();
	stmp    = ajStrNew();
	Acc    = ajStrNew();
	Spr     = ajStrNew();
	sse     = ajStrNew();
	exp1    = ajRegCompC("^([^ \t\r\n]+)[ \t\n\r]+");
	exp2    = ajRegCompC("^([A-Za-z0-9.]+)[ ]*[^ \t\r\n]+[ ]*"
			     "([0-9.-]+)[ ]*"
			     "[^ \t\r\n]+[ ]*([0-9.-]+)");
    }
    
    SeqSpr  = ajStrNew();
    SeqPdb  = ajStrNew();
    sss     = ajStrNew();


    
    ajStrAssignC(&tentry,entry);
    ajStrFmtUpper(&tentry);
    
    while((ok=ajFileReadLine(inf,&line)))
    {
	if(!ajStrPrefixC(line,"ID   "))
	    continue;
	
	if(!ajRegExec(exp1,line))
	    return NULL;
	ajRegPost(exp1,&stmp);
	if(ajStrMatchWildS(stmp,tentry))
	    break;
    }

    
    if(!ok)
    {
	ajRegFree(&exp1);
	ajRegFree(&exp2);
	ajStrDel(&line);
	ajStrDel(&str);
	ajStrDel(&xentry);
	ajStrDel(&source);
	ajStrDel(&class);
	ajStrDel(&fold);
	ajStrDel(&super);
	ajStrDel(&family);
	ajStrDel(&domain);
	ajStrDel(&pdb);
	ajStrDel(&tentry);
	ajStrDel(&stmp);
	ajStrDel(&Acc);
	ajStrDel(&Spr);
	ajStrDel(&SeqPdb);
	ajStrDel(&SeqSpr);
	ajStrDel(&sse);
	ajStrDel(&sss);
	return NULL;
    }    
    
    while(ok && (!ajStrPrefixC(line,"//")))
    {
	if(ajStrPrefixC(line,"XX"))
	{
	    ok = ajFileReadLine(inf,&line);
	    continue;
	}
	/* Empty line */
	if(!(MAJSTRGETLEN(line)))
	{
	    ok = ajFileReadLine(inf,&line);
	    continue;
	}
   
	ajRegExec(exp1,line);
	ajRegPost(exp1,&str);

	if(ajStrPrefixC(line,"ID"))
	    ajStrAssignS(&xentry,str);
	else if(ajStrPrefixC(line,"EN"))
	    ajStrAssignS(&pdb,str);
	else if(ajStrPrefixC(line,"OS"))
	    ajStrAssignS(&source,str);
	else if(ajStrPrefixC(line,"CL"))
	    ajStrAssignS(&class,str);
	else if(ajStrPrefixC(line,"FO"))
	{
	    ajStrAssignS(&fold,str);
	    while((ok=ajFileReadLine(inf,&line)))
	    {
		if(ajStrPrefixC(line,"XX"))
		    break;
		ajStrAppendC(&fold,ajStrGetPtr(line)+3);
	    }
	    ajStrRemoveWhiteExcess(&fold);
	}
	else if(ajStrPrefixC(line,"SF"))
	{
	    ajStrAssignS(&super,str);
	    while((ok=ajFileReadLine(inf,&line)))
	    {
		if(ajStrPrefixC(line,"XX"))
		    break;
		ajStrAppendC(&super,ajStrGetPtr(line)+3);
	    }
	    ajStrRemoveWhiteExcess(&super);
	}
	else if(ajStrPrefixC(line,"FA"))
	{
	    ajStrAssignS(&family,str);
	    while((ok=ajFileReadLine(inf,&line)))
	    {
		if(ajStrPrefixC(line,"XX"))
		    break;
		ajStrAppendC(&family,ajStrGetPtr(line)+3);
	    }
	    ajStrRemoveWhiteExcess(&family);
	}
	else if(ajStrPrefixC(line,"DO"))
	{
	    ajStrAssignS(&domain,str);
	    while((ok=ajFileReadLine(inf,&line)))
	    {
		if(ajStrPrefixC(line,"XX"))
		    break;
		ajStrAppendC(&domain,ajStrGetPtr(line)+3);
	    }
	    ajStrRemoveWhiteExcess(&domain);
	}
	else if(ajStrPrefixC(line,"NC"))
	{
	    ajStrToInt(str,&n);
	    (ret) = ajScopNew(n);
	    ajStrAssignS(&(ret)->Entry,xentry);
	    ajStrAssignS(&(ret)->Pdb,pdb);
	    ajStrAssignS(&(ret)->Source,source);
	    ajStrAssignS(&(ret)->Class,class);
	    ajStrAssignS(&(ret)->Fold,fold);
	    ajStrAssignS(&(ret)->Domain,domain);
	    ajStrAssignS(&(ret)->Superfamily,super);
	    ajStrAssignS(&(ret)->Family,family);
	    ajStrAssignS(&(ret)->Acc,Acc);
	    ajStrAssignS(&(ret)->Spr,Spr);
	    ajStrAssignS(&(ret)->Sse,sse);
            ajStrAssignS(&(ret)->Sss,sss);
	    ajStrAssignS(&(ret)->SeqPdb,SeqPdb);
	    ajStrAssignS(&(ret)->SeqSpr,SeqSpr);
	    (ret)->Sunid_Class = Sunid_Class;
	    (ret)->Sunid_Fold = Sunid_Fold;
	    (ret)->Sunid_Superfamily = Sunid_Superfamily;
	    (ret)->Sunid_Family = Sunid_Family;
	    (ret)->Sunid_Domain = Sunid_Domain;
	    (ret)->Sunid_Source = Sunid_Source;
	    (ret)->Sunid_Domdat = Sunid_Domdat;
	    (ret)->Startd       = Startd ;
	    (ret)->Endd         = Endd;
	}
	else if(ajStrPrefixC(line,"CN"))
	{
	    p = ajStrGetPtr(str);
	    sscanf(p,"[%d]",&idx);
	}
	else if(ajStrPrefixC(line,"CH"))
	{
	    if(!ajRegExec(exp2,str))
		return NULL;
	    ajRegSubI(exp2,1,&stmp);
	    (ret)->Chain[idx-1] = *ajStrGetPtr(stmp);
	    ajRegSubI(exp2,2,&str);
	    ajStrAssignC(&(ret)->Start[idx-1],ajStrGetPtr(str)); 

	    ajRegSubI(exp2,3,&str);
	    ajStrAssignC(&(ret)->End[idx-1],ajStrGetPtr(str)); 

	}
	/* Sequence from pdb file */
	else if(ajStrPrefixC(line,"DS"))
	{
	    while((ok=ajFileReadLine(inf,&line)) && !ajStrPrefixC(line,"XX"))
		ajStrAppendC(&SeqPdb,ajStrGetPtr(line));
	    ajStrRemoveWhite(&SeqPdb);
	    continue;
	}
	/* Sequence from swissprot */
	else if(ajStrPrefixC(line,"SQ"))
	{
	    while((ok=ajFileReadLine(inf,&line)) && !ajStrPrefixC(line,"XX"))
		ajStrAppendC(&SeqSpr,ajStrGetPtr(line));
	    ajStrRemoveWhite(&SeqSpr);
	    continue;
	}
	
	/* Accession number */
	else if(ajStrPrefixC(line,"AC"))
	    ajFmtScanS(line, "%*s %S", &Acc);

	/* Swissprot code */
	else if(ajStrPrefixC(line,"SF"))
	    ajFmtScanS(line, "%*s %S", &Spr);

	/* SSE string */
	else if(ajStrPrefixC(line,"SE"))
	    ajFmtScanS(line, "%*s %S", &sse);

        /* SSS string */
	else if(ajStrPrefixC(line,"SS"))
	{
	    while((ok=ajFileReadLine(inf,&line)) && !ajStrPrefixC(line,"XX"))
		ajStrAppendC(&sss,ajStrGetPtr(line));
	    ajStrRemoveWhite(&sss);
	    continue;
	}

	/* Start and end relative to swissprot sequence */
	else if(ajStrPrefixC(line,"RA"))
	    ajFmtScanS(line, "%*s %d %*s %d", &Startd, &Endd);
	/* Sunid of domain data */
	else if(ajStrPrefixC(line,"SI"))
	    ajFmtScanS(line, "%*s %d %*s %d %*s %d %*s "
		       "%d %*s %d %*s %d %*s %d", 
		       &Sunid_Class, &Sunid_Fold, &Sunid_Superfamily,
		       &Sunid_Family, 
		       &Sunid_Domain, &Sunid_Source, &Sunid_Domdat);
	
	ok = ajFileReadLine(inf,&line);
    }
 
    ajStrDel(&SeqSpr);
    ajStrDel(&SeqPdb);
    ajStrDel(&sss);
    
    return ret;
}









/* ======================================================================= */
/* =========================== destructors =============================== */
/* ======================================================================= */

/* @section Structure Destructors *******************************************
**
** These destructors functions receive the address of the instance to be
** deleted.  The original pointer is set to NULL so is ready for re-use.
**
****************************************************************************/


/* @func ajDomainDel ********************************************************
**
** Destructor for domain object.
**
** @param [d] ptr [AjPDomain*] Domain object pointer
**
** @return [void]
** @category delete [AjPDomain] Default Domain destructor.
** @@
****************************************************************************/

void ajDomainDel(AjPDomain *ptr)
{
    AjPDomain pthis;


    pthis = *ptr;

    if(!pthis || !ptr)
	return;

    if(pthis->Type == ajSCOP)
	ajScopDel(&pthis->Scop);
    else
	ajCathDel(&pthis->Cath);

    AJFREE(pthis);
    (*ptr) = NULL;

    return;
}





/* @func ajScopDel **********************************************************
**
** Destructor for scop object.
**
** @param [d] ptr [AjPScop*] Scop object pointer
**
** @return [void]
** @category delete [AjPScop] Default Scop destructor.
** @@
****************************************************************************/

void ajScopDel(AjPScop *ptr)
{
    AjPScop pthis;
    ajint i;

    pthis = *ptr;

    if(!pthis || !ptr)
	return;

    ajStrDel(&pthis->Entry);
    ajStrDel(&pthis->Pdb);
    ajStrDel(&pthis->Class);
    ajStrDel(&pthis->Fold);
    ajStrDel(&pthis->Superfamily);
    ajStrDel(&pthis->Family);
    ajStrDel(&pthis->Domain);
    ajStrDel(&pthis->Source);
    ajStrDel(&pthis->Acc);
    ajStrDel(&pthis->Spr);
    ajStrDel(&pthis->SeqPdb);
    ajStrDel(&pthis->SeqSpr);
    ajStrDel(&pthis->Sse);
    ajStrDel(&pthis->Sss);

    if(pthis->N)
    {
	for(i=0; i<pthis->N; i++)
	{
	    ajStrDel(&pthis->Start[i]);
	    ajStrDel(&pthis->End[i]);
	}
	AJFREE(pthis->Start);
	AJFREE(pthis->End);
	AJFREE(pthis->Chain);
    }

    AJFREE(pthis);
    (*ptr) = NULL;

    return;
}





/* @func ajCathDel **********************************************************
**
** Destructor for Cath object. Fore-knowledge of the number of chain segments
** domain is comprised of is required. 
**
** @param [d] ptr [AjPCath *] Cath object pointer
**
** @return [void]
** @category delete [AjPCath] Default Cath destructor.
**
****************************************************************************/ 

void ajCathDel(AjPCath *ptr)
{
    AjPCath pthis;
    ajint x;

    pthis = *ptr;
    
    if(!pthis || !ptr)     /* (pthis==NULL)||(ptr==NULL) */
	return;
  
    ajStrDel(&pthis->DomainID);     
    ajStrDel(&pthis->Pdb);         
    ajStrDel(&pthis->Class);        
    ajStrDel(&pthis->Architecture); 
    ajStrDel(&pthis->Topology);      
    ajStrDel(&pthis->Superfamily);   
    ajStrDel(&pthis->Acc);
    ajStrDel(&pthis->Spr);
    ajStrDel(&pthis->SeqPdb);
    ajStrDel(&pthis->SeqSpr);


    if(pthis->NSegment)
    {
	for(x=0; x<pthis->NSegment; x++)
	{
	    ajStrDel(&pthis->Start[x]);
	    ajStrDel(&pthis->End[x]);    
	}
    
	AJFREE(pthis->Start);
	AJFREE(pthis->End);
    }
    AJFREE(pthis);

    pthis  = NULL;
    
    (*ptr) = NULL;
    
    return;
} 
 




/* ======================================================================= */
/* ============================ Assignments ============================== */
/* ======================================================================= */

/* @section Assignments *****************************************************
**
** These functions overwrite the instance provided as the first argument
** A NULL value is always acceptable so these functions are often used to
** create a new instance by assignment.
**
****************************************************************************/


/* @func ajDomainCopy *********************************************************
**
** Copies the contents from one Domain object to another.
**
** @param [w] to   [AjPDomain*] Domain object pointer 
** @param [r] from [const AjPDomain]  Domain object 
**
** @return [AjBool] True if copy was successful.
** @category assign [AjPDomain] Replicates a Domain object.
** @@
****************************************************************************/
AjBool ajDomainCopy(AjPDomain *to, const AjPDomain from)
{
    ajint n = 0;
    
    /* Check args */
    if(!from)
	return ajFalse;

    if((*to))
	ajDomainDel(to);
    
    if(from->Type == ajSCOP)
	n = from->Scop->N;
    else
	n = from->Cath->NSegment;

    (*to) = ajDomainNew(n, from->Type);
    (*to)->Type = from->Type;
    

    if(from->Type == ajSCOP)
	ajScopCopy(&((*to)->Scop), from->Scop);
    else
	ajCathCopy(&((*to)->Cath), from->Cath);
    

    return ajTrue;
}



/* @func ajCathCopy *********************************************************
**
** Copies the contents from one Cath object to another.
**
** @param [w] to   [AjPCath*] Cath object pointer 
** @param [r] from [const AjPCath]  Cath object 
**
** @return [AjBool] True if copy was successful.
** @category assign [AjPCath] Replicates a Cath object.
** @@
****************************************************************************/

AjBool ajCathCopy(AjPCath *to, const AjPCath from)
{
    ajint x = 0;
    
    /* Check args */
    if(!from)
	return ajFalse;

    if(!(*to))
	(*to) = ajCathNew(from->NSegment);
    

    ajStrAssignS(&(*to)->DomainID, from->DomainID);
    ajStrAssignS(&(*to)->Pdb, from->Pdb);
    ajStrAssignS(&(*to)->Class, from->Class);
    ajStrAssignS(&(*to)->Architecture, from->Architecture);
    ajStrAssignS(&(*to)->Topology, from->Topology);
    ajStrAssignS(&(*to)->Superfamily, from->Superfamily);

    (*to)->Length = from->Length;
    (*to)->Chain  = from->Chain;

    for(x=0; x<from->NSegment; x++)
    {
	ajStrAssignS(&(*to)->Start[x], from->Start[x]);	
	ajStrAssignS(&(*to)->End[x], from->End[x]);	
    }
    
    ajStrAssignS(&(*to)->Acc, from->Acc);
    ajStrAssignS(&(*to)->Spr, from->Spr);
    ajStrAssignS(&(*to)->SeqPdb, from->SeqPdb);
    ajStrAssignS(&(*to)->SeqSpr, from->SeqSpr);
    (*to)->Startd = from->Startd;
    (*to)->Endd   = from->Endd;

    (*to)->Class_Id       = from->Class_Id;
    (*to)->Arch_Id        = from->Arch_Id;
    (*to)->Topology_Id     = from->Topology_Id;
    (*to)->Superfamily_Id = from->Superfamily_Id;
    (*to)->Family_Id      = from->Family_Id;
    (*to)->NIFamily_Id    = from->NIFamily_Id;
    (*to)->IFamily_Id     = from->IFamily_Id;

    return ajTrue;
}





/* @func ajScopCopy *********************************************************
**
** Copies the contents from one Scop object to another.
**
** @param [w] to   [AjPScop*] Scop object pointer 
** @param [r] from [const AjPScop]  Scop object 
**
** @return [AjBool] True if copy was successful.
** @category assign [AjPScop] Replicates a Scop object.
** @@
****************************************************************************/

AjBool ajScopCopy(AjPScop *to, const AjPScop from)
{
    ajint x = 0;
    
    /* Check args */
    if(!from)
	return ajFalse;

    if(!(*to))
	(*to) = ajScopNew(from->N);
    

    ajStrAssignS(&(*to)->Entry, from->Entry);
    ajStrAssignS(&(*to)->Pdb, from->Pdb);
    ajStrAssignS(&(*to)->Class, from->Class);
    ajStrAssignS(&(*to)->Fold, from->Fold);
    ajStrAssignS(&(*to)->Superfamily, from->Superfamily);
    ajStrAssignS(&(*to)->Family, from->Family);
    ajStrAssignS(&(*to)->Domain, from->Domain);
    ajStrAssignS(&(*to)->Source, from->Source);
    ajStrAssignS(&(*to)->Sse, from->Sse);
    ajStrAssignS(&(*to)->Sss, from->Sss);

    for(x=0; x<from->N; x++)
    {
	(*to)->Chain[x] = from->Chain[x];
	ajStrAssignS(&(*to)->Start[x], from->Start[x]);	
	ajStrAssignS(&(*to)->End[x], from->End[x]);	
    }
    
    ajStrAssignS(&(*to)->Acc, from->Acc);
    ajStrAssignS(&(*to)->Spr, from->Spr);
    ajStrAssignS(&(*to)->SeqPdb, from->SeqPdb);
    ajStrAssignS(&(*to)->SeqSpr, from->SeqSpr);
    (*to)->Startd = from->Startd;
    (*to)->Endd   = from->Endd;

    (*to)->Sunid_Class = from->Sunid_Class;
    (*to)->Sunid_Fold  = from->Sunid_Fold;
    (*to)->Sunid_Superfamily = from->Sunid_Superfamily;
    (*to)->Sunid_Family = from->Sunid_Family;
    (*to)->Sunid_Domain = from->Sunid_Domain;
    (*to)->Sunid_Source = from->Sunid_Source;
    (*to)->Sunid_Domdat = from->Sunid_Domdat;

    (*to)->Score = from->Score;

    return ajTrue;
}





/* ======================================================================= */
/* ============================= Modifiers =============================== */
/* ======================================================================= */

/* @section Modifiers *******************************************************
**
** These functions use the contents of an instance and update them.
**
****************************************************************************/





/* ======================================================================= */
/* ========================== Operators ===================================*/
/* ======================================================================= */

/* @section Operators *******************************************************
**
** These functions use the contents of an instance but do not make any 
** changes.
**
****************************************************************************/





/* @func ajScopMatchSunid ***************************************************
**
** Function to sort Scop objects by Sunid_Family.
**
** @param [r] entry1  [const void*] Pointer to Scop object 1
** @param [r] entry2  [const void*] Pointer to Scop object 2
**
** @return [ajint] -1 if Start1 should sort before Start2, +1 if the Start2 
** should sort first. 0 if they are identical.
** @category use [AjPScop] Sort Scop objects by Sunid_Family element.
** @@
****************************************************************************/
ajint ajScopMatchSunid(const void *entry1, const void *entry2)
{
    const AjPScop p = NULL;
    const AjPScop q = NULL;

    p = (*(AjPScop const *)entry1);
    q = (*(AjPScop const *)entry2);
   

    if(p->Sunid_Family < q->Sunid_Family)
        return -1;
    else if(p->Sunid_Family == q->Sunid_Family)
        return 0;

    return 1;
}





/* @func ajScopMatchScopid **************************************************
**
** Function to sort Scop objects by Entry element. 
**
** @param [r] hit1  [const void*] Pointer to Scop object 1
** @param [r] hit2  [const void*] Pointer to Scop object 2
**
** @return [ajint] -1 if Entry1 should sort before Entry2, +1 if the Entry2 
** should sort first. 0 if they are identical in length and content. 
** @category use [AjPScop] Sort Scop objects by Entry element.
** @@
****************************************************************************/

ajint ajScopMatchScopid(const void *hit1, const void *hit2)
{
    const AjPScop p = NULL;
    const AjPScop q = NULL;

    p = (*(AjPScop const *)hit1);
    q = (*(AjPScop const *)hit2);
    
    return ajStrCmpS(p->Entry, q->Entry);
}





/* @func ajScopMatchPdbId ***************************************************
**
** Function to sort Scop objects by Pdb element. 
**
** @param [r] hit1  [const void*] Pointer to Scop object 1
** @param [r] hit2  [const void*] Pointer to Scop object 2
**
** @return [ajint] -1 if Pdb1 should sort before Pdb2, +1 if the Pdb2 
** should sort first. 0 if they are identical in length and content. 
** @category use [AjPScop] Sort Scop objects by Pdb element.
** @@
****************************************************************************/

ajint ajScopMatchPdbId(const void *hit1, const void *hit2)
{
    const AjPScop p = NULL;
    const AjPScop q = NULL;

    p = (*(AjPScop const *)hit1);
    q = (*(AjPScop const *)hit2);
    
    return ajStrCmpS(p->Pdb, q->Pdb);
}





/* @func ajCathMatchPdbId ***************************************************
**
** Function to sort Cath objects by Pdb element. 
**
** @param [r] hit1  [const void*] Pointer to Cath object 1
** @param [r] hit2  [const void*] Pointer to Cath object 2
**
** @return [ajint] -1 if Pdb1 should sort before Pdb2, +1 if the Pdb2 
** should sort first. 0 if they are identical in length and content. 
** @category use [AjPScop] Sort Cath objects by Pdb element.
** @@
****************************************************************************/

ajint ajCathMatchPdbId(const void *hit1, const void *hit2)
{
    const AjPCath p = NULL;
    const AjPCath q = NULL;

    p = (*(AjPCath const *)hit1);
    q = (*(AjPCath const *)hit2);
    
    return ajStrCmpS(p->Pdb, q->Pdb);
}





/* ======================================================================= */
/* ============================== Casts ===================================*/
/* ======================================================================= */

/* @section Casts ***********************************************************
**
** These functions examine the contents of an instance and return some
** derived information. Some of them provide access to the internal
** components of an instance. 
**
****************************************************************************/





/* ======================================================================= */
/* =========================== Reporters ==================================*/
/* ======================================================================= */

/* @section Reporters *******************************************************
**
** These functions return the contents of an instance but do not make any 
** changes.
**
****************************************************************************/


/* @func ajDomainGetId ******************************************************
**
** Returns domain id, either DomainID element (Cath object) or Entry 
** (Scop object).
**
** @param [r] obj  [const AjPDomain] Domain object
**
** @return [AjPStr] Or NULL on error.
** @@
****************************************************************************/
AjPStr ajDomainGetId(const AjPDomain obj)
{
    if(!obj)
    {
	ajWarn("Bad args to ajDomainGetId");
	return NULL;
    }
    
    if(obj->Type == ajSCOP)
	return(obj->Scop->Entry);
    else if(obj->Type == ajCATH)
	return(obj->Cath->DomainID);
    else
	ajWarn("Domain type not resolved in ajDomainGetId");
    return NULL;
}		

/* @func ajDomainGetSeqPdb **************************************************
**
** Returns the pdb sequence from a Domain object.
**
** @param [r] obj  [const AjPDomain] Domain object
**
** @return [AjPStr] Or NULL on error.
** @@
****************************************************************************/
AjPStr ajDomainGetSeqPdb(const AjPDomain obj)
{
    if(!obj)
    {
	ajWarn("Bad args to ajDomainGetSeqPdb");
	return NULL;
    }
    
    if(obj->Type == ajSCOP)
	return(obj->Scop->SeqPdb);
    else if(obj->Type == ajCATH)
	return(obj->Cath->SeqPdb);
    else
	ajWarn("Domain type not resolved in ajDomainGetSeqPdb");
    return NULL;
}		


/* @func ajDomainGetSeqSpr **************************************************
**
** Returns the swissprot sequence from a Domain object.
**
** @param [r] obj  [const AjPDomain] Domain object
**
** @return [AjPStr] Or NULL on error.
** @@
****************************************************************************/
AjPStr ajDomainGetSeqSpr(const AjPDomain obj)
{
    if(!obj)
    {
	ajWarn("Bad args to ajDomainGetSeqSpr");
	return NULL;
    }
    
    if(obj->Type == ajSCOP)
	return(obj->Scop->SeqSpr);
    else if(obj->Type == ajCATH)
	return(obj->Cath->SeqSpr);
    else
	ajWarn("Domain type not resolved in ajDomainGetSeqSpr");
    return NULL;
}	


/* @func ajDomainGetPdb *****************************************************
**
** Returns the Pdb element from a Domain object.
**
** @param [r] obj  [const AjPDomain] Domain object
**
** @return [AjPStr] Or NULL on error.
** @@
****************************************************************************/
AjPStr ajDomainGetPdb(const AjPDomain obj)
{
    if(!obj)
    {
	ajWarn("Bad args to ajDomainGetSeqPdb");
	return NULL;
    }
    
    if(obj->Type == ajSCOP)
	return(obj->Scop->Pdb);
    else if(obj->Type == ajCATH)
	return(obj->Cath->Pdb);
    else
	ajWarn("Domain type not resolved in ajDomainGetSeqPdb");
    return NULL;
}	





/* @func ajDomainGetAcc *****************************************************
**
** Returns the Acc element from a Domain object.
**
** @param [r] obj  [const AjPDomain] Domain object
**
** @return [AjPStr] Or NULL on error.
** @@
****************************************************************************/
AjPStr ajDomainGetAcc(const AjPDomain obj)
{
    if(!obj)
    {
	ajWarn("Bad args to ajDomainGetSeqPdb");
	return NULL;
    }
    
    if(obj->Type == ajSCOP)
	return(obj->Scop->Acc);
    else if(obj->Type == ajCATH)
	return(obj->Cath->Acc);
    else
	ajWarn("Domain type not resolved in ajDomainGetSeqPdb");
    return NULL;
}	





/* @func ajDomainGetSpr *****************************************************
**
** Returns the Spr element from a Domain object.
**
** @param [r] obj  [const AjPDomain] Domain object
**
** @return [AjPStr] Or NULL on error.
** @@
****************************************************************************/
AjPStr ajDomainGetSpr(const AjPDomain obj)
{
    if(!obj)
    {
	ajWarn("Bad args to ajDomainGetSeqPdb");
	return NULL;
    }
    
    if(obj->Type == ajSCOP)
	return(obj->Scop->Spr);
    else if(obj->Type == ajCATH)
	return(obj->Cath->Spr);
    else
	ajWarn("Domain type not resolved in ajDomainGetSeqPdb");
    return NULL;
}	





/* @func ajDomainGetN *******************************************************
**
** Returns no. chains or chain segments in a domain, either NSegment element
** (Cath domains) or N element (Scop domains).
**
** @param [r] obj  [const AjPDomain] Domain object
**
** @return [ajint] Or -1 on error.
** @@
****************************************************************************/
ajint ajDomainGetN(const AjPDomain obj)
{
    if(!obj)
    {
	ajWarn("Bad args to ajDomainGetSeqPdb");
	return -1;
    }
    
    if(obj->Type == ajSCOP)
	return(obj->Scop->N);
    else if(obj->Type == ajCATH)
	return(obj->Cath->NSegment);
    else
	ajWarn("Domain type not resolved in ajDomainGetSeqPdb");
    return -1;
}	




/* @func ajScopArrFindScopid ************************************************
**
** Performs a binary search for a SCOP domain id over an array of Scop
** structures (which of course must first have been sorted). This is a 
** case-insensitive search.
**
** @param [r] arr [AjPScop const *]    Array of AjPScop objects
** @param [r] siz [ajint]       Size of array
** @param [r] id  [const AjPStr]      Search term
**
** @return [ajint] Index of first Scop object found with a SCOP domain id
** matching id, or -1 if id is not found.
** @category use [AjPScop*] Binary search for Entry element over array of
**                         Scop objects.
** @@
****************************************************************************/
ajint ajScopArrFindScopid(AjPScop const *arr, ajint siz, const AjPStr id)

{
    int l;
    int m;
    int h;
    int c;


    l = 0;
    h = siz-1;
    while(l<=h)
    {
        m = (l+h)>>1;

        if((c = ajStrCmpCaseS(id, arr[m]->Entry)) < 0) 
	    h = m-1;
        else if(c>0) 
	    l = m+1;
        else 
	    return m;
    }

    return -1;
}





/* @func ajScopArrFindSunid *************************************************
**
** Performs a binary search for a SCOP sunid over an array of Scop
** objects (which of course must first have been sorted). This is a 
** case-insensitive search.
**
** @param [r] arr [AjPScop const *]    Array of Scop objects
** @param [r] siz [ajint]       Size of array
** @param [r] id  [ajint]       Search term
**
** @return [ajint] Index of first Scop object found with an PDB code
** matching id, or -1 if id is not found.
** @category use [AjPScop*] Binary search for Sunid_Family element
**                         over array of Scop objects.
** @@
****************************************************************************/

ajint ajScopArrFindSunid(AjPScop const *arr, ajint siz, ajint id)
{
    int l;
    int m;
    int h;

    l = 0;
    h = siz-1;
    while(l<=h)
    {
        m=(l+h)>>1;
        
        if(id < arr[m]->Sunid_Family)
            h=m-1;
        else if(id > arr[m]->Sunid_Family)
            l=m+1;
        else 
            return m;
    }

    return -1;
}





/* @func ajScopArrFindPdbid *************************************************
**
** Performs a binary search for a SCOP domain id over an array of Scop
** objects (which of course must first have been sorted). This is a 
** case-insensitive search.
**
** @param [r] arr [AjPScop const*]    Array of AjPScop objects
** @param [r] siz [ajint]       Size of array
** @param [r] id  [const AjPStr]      Search term
**
** @return [ajint] Index of first Scop object found with a PDB code
** matching id, or -1 if id is not found.
** @category use [AjPScop*] Binary search for Pdb element over array
**                         of Scop objects.
** @@
****************************************************************************/

ajint ajScopArrFindPdbid(AjPScop const *arr, ajint siz, const AjPStr id)
{
    int l;
    int m;
    int h;
    int c;


    l = 0;
    h = siz-1;
    while(l<=h)
    {
        m = (l+h)>>1;

        if((c=ajStrCmpCaseS(id, arr[m]->Pdb)) < 0) 
	    h = m-1;
        else if(c>0) 
	    l = m+1;
        else 
	    return m;
    }

    return -1;
}





/* @func ajCathArrFindPdbid ************************************************
**
** Performs a binary search for a CATH domain id over an array of Cath
** structures (which of course must first have been sorted). This is a 
** case-insensitive search.
**
** @param [r] arr [AjPCath const*]    Array of AjPCath objects
** @param [r] siz [ajint]       Size of array
** @param [r] id  [const AjPStr]      Search term
**
** @return [ajint] Index of first Cath object found with a PDB code
** matching id, or -1 if id is not found.
** @category use [AjPCath*] Binary search for Pdb element over array
**                         of Cath objects.
** @@
****************************************************************************/

ajint ajCathArrFindPdbid(AjPCath const *arr, ajint siz, const AjPStr id)
{
    int l;
    int m;
    int h;
    int c;


    l = 0;
    h = siz-1;
    while(l<=h)
    {
        m=(l+h)>>1;

        if((c=ajStrCmpCaseS(id, arr[m]->Pdb)) < 0) 
	    h = m-1;
        else if(c>0) 
	    l = m+1;
        else 
	    return m;
    }

    return -1;
}





/* ======================================================================= */
/* ========================== Input & Output ============================= */
/* ======================================================================= */

/* @section Input and Output ************************************************
**
** These functions use the contents of an instance but do not make any 
** changes.
**
****************************************************************************/


/* @func ajPdbWriteDomain **************************************************
**
** Writes a clean coordinate file for a SCOP domain. Where coordinates for 
** multiple models (e.g. NMR structures) are given, data for model 1 are 
** written.
** In the clean file, the coordinates are presented as belonging to a single 
** chain regardless of how many chains the domain comprised.
** Coordinates for heterogens are NOT written to file.
**
** @param [u] outf [AjPFile] Output file stream
** @param [r] pdb  [const AjPPdb]  Pdb object
** @param [r] scop [const AjPScop] Scop object
** @param [u] errf [AjPFile] Output file stream for error messages
**
** @return [AjBool] True on success
** @@
** 
****************************************************************************/
AjBool ajPdbWriteDomain(AjPFile outf, const AjPPdb pdb,
			 const AjPScop scop, AjPFile errf)
{
    /*
    ** rn_mod is a modifier to the residue number to give correct residue
    ** numbering for the domain
    */
    ajint z;
    ajint chn;
    ajint start   = 0;
    ajint end     = 0;
    ajint finalrn = 0;
    ajint rn_mod  = 0;  
    ajint last_rn = 0;  
    ajint this_rn;
    char  id;
    
    AjPStr tmpseq = NULL;   
    AjPStr seq    = NULL;   
    AjPStr tmpstr = NULL;
        
    AjBool      found_start = ajFalse;
    AjBool      found_end   = ajFalse;
    AjBool      nostart     = ajFalse;
    AjBool      noend       = ajFalse;
    AjIList     iter        = NULL;
    AjPAtom     atm         = NULL;
    AjPAtom     atm2        = NULL;
    AjPResidue  res         = NULL;
    AjPResidue  res2        = NULL;
/*    AjPResidue *resarr      = NULL; */
    AjPSeqout outseq;



    if(!errf || !scop || !pdb || !outf)
      ajFatal("Bad args passed to ajPdbWriteDomain");


    /* Intitialise strings */
    seq    = ajStrNew();
    tmpseq = ajStrNew();
    tmpstr = ajStrNew();
    



    /* Check for unknown or zero-length chains */
    for(z=0;z<scop->N;z++)
	if(!ajPdbChnidToNum(scop->Chain[z], pdb, &chn))
	{
	    ajWarn("Chain incompatibility error in "
		   "ajPdbWriteDomain");			
		
	    ajFmtPrintF(errf, "//\n%S\nERROR Chain incompatibility "
			"error in ajPdbWriteDomain\n", scop->Entry);
	    ajStrDel(&seq);
	    ajStrDel(&tmpseq);
	    ajStrDel(&tmpstr);
	    return ajFalse;
	}
	else if(pdb->Chains[chn-1]->Nres==0)
	{		
	    ajWarn("Chain length zero");			
	    
	    ajFmtPrintF(errf, "//\n%S\nERROR Chain length zero\n", 
			scop->Entry);
	    ajStrDel(&seq);
	    ajStrDel(&tmpseq);
	    ajStrDel(&tmpstr);
	    return ajFalse;
	}
    


    /* Write header info. to domain coordinate file */
    ajFmtPrintF(outf, "%-5s%S\n", "ID", scop->Entry);
    ajFmtPrintF(outf, "XX\n");
    ajFmtPrintF(outf, "%-5sCo-ordinates for SCOP domain %S\n", 
		"DE", scop->Entry);
    ajFmtPrintF(outf, "XX\n");
    ajFmtPrintF(outf, "%-5sSee Escop.dat for domain classification\n", 
		"OS");
    ajFmtPrintF(outf, "XX\n");
    ajFmtPrintF(outf, "%-5sMETHOD ", "EX");
    if(pdb->Method == ajXRAY)
	ajFmtPrintF(outf, "xray; ");	
    else
	ajFmtPrintF(outf, "nmr_or_model; ");		
    ajFmtPrintF(outf, "RESO %.2f; NMOD 1; NCHN 1; NGRP 0;\n", 
		pdb->Reso);
    
    /* The NCHN and NMOD are hard-coded to 1 for domain files */
    


    /* Start of main application loop */
    /* Print out data up to co-ordinates list */
    for(z=0;
	z<scop->N;
	z++,found_start=ajFalse, found_end=ajFalse, 
	nostart=ajFalse, noend=ajFalse, last_rn=0)
    {	
	/*
	** Unknown or Zero sized chains have already been checked for
	** so no additional checking is needed here
	*/
	ajPdbChnidToNum(scop->Chain[z], pdb, &chn);
	

	/* Initialise the iterator */
	iter=ajListIterNewread(pdb->Chains[chn-1]->Residues);


	/*
	** If start of domain is unspecified 
	** then assign start to first residue in chain
	*/
	if(!ajStrCmpC(scop->Start[z], "."))
	{
	    nostart     = ajTrue;
	    start       = 1;
	    found_start = ajTrue;	
	}
		

	/*
	** If end of domain is unspecified 
	** then assign end to last residue in chain
	*/
	if(!ajStrCmpC(scop->End[z], "."))
	{
	    noend = ajTrue;
	    end=pdb->Chains[chn-1]->Nres;
	    found_end=ajTrue;	
	}
		

	/* Find start and end of domains in chain */
	if(!found_start || !found_end)
	{
	    /* Iterate through the list of atoms */
	    while((res=(AjPResidue)ajListIterGet(iter)))
	    {
		/* 
		 ** Hard-coded to work on model 1
		 ** Break if model no. !=1
		 */
		if(res->Mod!=1 || (found_start && found_end))
		    break; 

		/* if(res->Type!='P' || res->Mod!=1 
		   || (found_start && found_end))
		    break; */


		/* If we are onto a new residue */
		this_rn=res->Idx;
		if(this_rn!=last_rn)
		{
		    last_rn=this_rn;

		    /*
		    ** The start position was specified, but has not 
		    ** been found yet
		    */
		    if(!found_start && !nostart)		
		    {
			ajStrAssignS(&tmpstr, scop->Start[z]);
			ajStrAppendK(&tmpstr, '*');
			
			/* Start position found  */
		        /*if(!ajStrCmpCaseS(res->Pdb, scop->Start[z])) */
			if(ajStrMatchWildS(res->Pdb, tmpstr))
			{
			    if(!ajStrMatchS(res->Pdb, scop->Start[z]))
			    {
				ajWarn("Domain start found by wildcard "
				       "match only "
				       "in ajPdbWriteDomain");
				ajFmtPrintF(errf, "//\n%S\nERROR Domain "
					    "start found "
					    "by wildcard match only in "
					    "ajPdbWriteDomain\n",
					    scop->Entry);
			    }
			    
			    start=res->Idx;
			    found_start=ajTrue;	
			}
			else	
			    continue;
		    }




		    /*
		    ** The end position was specified, but has not 
		    ** been found yet
		    */
		    if(!found_end && !noend)		
		    {
			ajStrAssignS(&tmpstr, scop->End[z]);
			ajStrAppendK(&tmpstr, '*');

			/* End position found */
			/*if(!ajStrCmpCaseS(res->Pdb, scop->End[z])) */
			if(ajStrMatchWildS(res->Pdb, tmpstr))
			{
			    if(!ajStrMatchS(res->Pdb, scop->End[z]))
			    {
				ajWarn("Domain end found by wildcard "
				       "match only "
				       "in ajPdbWriteDomain");
				ajFmtPrintF(errf, "//\n%S\nERROR Domain end "
					    "found "
					    "by wildcard match only in "
					    "ajPdbWriteDomain\n",
					    scop->Entry);
			    }

			    end = res->Idx;
			    found_end = ajTrue;       
			    break;
			}
		    }	
		}
	    }
	}

	
	/* Diagnostics if start position was not found */
	if(!found_start)		
	{
	    ajStrDel(&seq);
	    ajStrDel(&tmpseq);
	    ajStrDel(&tmpstr);
	    ajListIterDel(&iter);	
	    ajWarn("Domain start not found in ajPdbWriteDomain");
	    ajFmtPrintF(errf, "//\n%S\nERROR Domain start not found "
			"in in ajPdbWriteDomain\n", scop->Entry);
	    return ajFalse;
	}
	

	/* Diagnostics if end position was not found */
	if(!found_end)		
	{
	    ajStrDel(&seq);
	    ajStrDel(&tmpseq);
	    ajStrDel(&tmpstr);
	    ajListIterDel(&iter);	
	    ajWarn("Domain end not found in ajPdbWriteDomain");
	    ajFmtPrintF(errf, "//\n%S\nERROR Domain end not found "
			"in ajPdbWriteDomain\n", scop->Entry);
	    return ajFalse;
	}
	

	/* Write <seq> string here */
	ajStrAssignSubS(&tmpseq, pdb->Chains[chn-1]->Seq, start-1, end-1);
	ajStrAppendS(&seq, tmpseq);



	/* Free the iterator */
	ajListIterDel(&iter);	
    }
    /* End of main application loop */
    

    /*
    ** If the domain was composed of more than once chain then a '.' is
    ** given as the chain identifier
    */
    if(scop->N > 1)
	id = '.';
    else
    {
	id = pdb->Chains[chn-1]->Id;
	if(id == ' ')
	    id = '.';
    }

    
    /* Write sequence to domain coordinate file */
    ajFmtPrintF(outf, "XX\n");	
    ajFmtPrintF(outf, "%-5s[1]\n", "CN");	
    ajFmtPrintF(outf, "XX\n");	
    /*
    ajFmtPrintF(outf, "%-5sID %c; NR %d; NL 0; NH %d; NE %d; NS %d; NT %d;\n", 
		"IN", 
		id,
		ajStrGetLen(seq),
		pdb->Chains[chn-1]->numHelices, 
		pdb->Chains[chn-1]->numStrands, 
		pdb->Chains[chn-1]->numSheets, 
		pdb->Chains[chn-1]->numTurns);
		*/
    ajFmtPrintF(outf, "%-5sID %c; NRES %d; NL 0; NH %d; NE %d;\n", 
		"IN", 
		id,
		ajStrGetLen(seq),
		pdb->Chains[chn-1]->numHelices, 
		pdb->Chains[chn-1]->numStrands);
    ajFmtPrintF(outf, "XX\n");

    outseq = ajSeqoutNewFile(outf);
    ajSeqoutDumpSwisslike(outseq, seq, "SQ");
    ajSeqoutDel(&outseq);

    ajFmtPrintF(outf, "XX\n");	


    
    /* Write residue list to domain coordinate file */        
    for(nostart=ajFalse, noend=ajFalse, 
	z=0; z<scop->N;
	z++,found_start=ajFalse, found_end=ajFalse)
    {
	/*
	** Unknown or Zero length chains have already been checked for
	** so no additional checking is needed here
	*/
	ajPdbChnidToNum(scop->Chain[z], pdb, &chn);
	
	/* Initialise the iterator */
	iter = ajListIterNewread(pdb->Chains[chn-1]->Residues);


	/* Increment res. counter from last chain if appropriate */
	if(noend)
	    rn_mod += res2->Idx;
	else	 
	    rn_mod += finalrn;

	
	/* Check whether start and end of domain are specified */
	if(!ajStrCmpC(scop->Start[z], "."))
	    nostart = ajTrue;
	else
	    nostart=ajFalse;
	
	if(!ajStrCmpC(scop->End[z], "."))
	    noend = ajTrue;
	else 
	    noend = ajFalse;
	

	/* Iterate through the list of residues */
	while((res=(AjPResidue)ajListIterGet(iter)))
	{
	    /* Break if model no. !=1 */
	    if(res->Mod!=1)
		break;

	    /* The start position has not been found yet */
	    if(!found_start)
	    {
		/* Start position was specified */
		if(!nostart)
		{
		    ajStrAssignS(&tmpstr, scop->Start[z]);
		    ajStrAppendK(&tmpstr, '*');

		    /* Start position found */
		    /*if(!ajStrCmpCaseS(res->Pdb, scop->Start[z])) */
		    if(ajStrMatchWildS(res->Pdb, tmpstr))		    
		    {
			if(!ajStrMatchS(res->Pdb, scop->Start[z]))
			{
			    ajWarn("Domain start found by wildcard match only "
				   "in ajPdbWriteDomain");
			    ajFmtPrintF(errf, "//\n%S\nERROR Domain "
					"start found "
					"by wildcard match only in "
					"ajPdbWriteDomain\n", scop->Entry);
			}
			    

			rn_mod -= res->Idx-1;
			found_start = ajTrue;	
		    }
		    else	
			continue;
		}
		else	
		    found_start=ajTrue;	
	    }	

	    
	    /*
	     ** The end position was specified, but has not 
	     ** been found yet
	     */
	    if(!found_end && !noend)
	    {
		ajStrAssignS(&tmpstr, scop->End[z]);
		ajStrAppendK(&tmpstr, '*');

		/* End position found */
		/*if(!ajStrCmpCaseS(res->Pdb, scop->End[z])) */
		if(ajStrMatchWildS(res->Pdb, tmpstr))
		{
		    if(!ajStrMatchS(res->Pdb, scop->End[z]))
		    {
			ajWarn("Domain end found by wildcard match only "
			       "in ajPdbWriteDomain");
			ajFmtPrintF(errf, "//\n%S\nERROR Domain end found "
				    "by wildcard match only in "
				    "ajPdbWriteDomain\n", scop->Entry);
		    }

		    found_end = ajTrue;     
		    finalrn   = res->Idx;
		}
	    }	
	    /*
	    ** The end position was specified and has been found, and
	    ** the current atom no longer belongs to this final residue
	    */
	    else if(res->Idx != finalrn && !noend)
		break;
	    
	    ajFmtPrintF(outf, "%-5s%-5d%-5d%-5d%-6S%-2c%-6S", 
			"RE", 
			res->Mod, /* It will always be 1 */
		        1, /* chn number is always given as 1 */
			res->Idx+rn_mod, 
			res->Pdb,
			res->Id1, 
			res->Id3);
	    
			
	    if(res->eNum != 0)
		ajFmtPrintF(outf, "%-5d", res->eNum);
	    else
		ajFmtPrintF(outf, "%-5c", '.');
	    ajFmtPrintF(outf, "%-5S%-5c", res->eId, res->eType);
	    
	    if(res->eType == 'H')
		ajFmtPrintF(outf, "%-5d", res->eClass);
	    else
		ajFmtPrintF(outf, "%-5c", '.');
	    
	    if(res->eStrideNum != 0)
		ajFmtPrintF(outf, "%-5d", res->eStrideNum);
	    else
		ajFmtPrintF(outf, "%-5c", '.');
	    ajFmtPrintF(outf, "%-5c", res->eStrideType);
	    

	    ajFmtPrintF(outf, "%8.2f%8.2f%8.2f%8.2f%8.2f"
			"%8.2f%8.2f%8.2f%8.2f%8.2f%8.2f%8.2f"
			"%8.2f\n", 
			res->Phi,
			res->Psi,
			res->Area, 
			res->all_abs, 
			res->all_rel, 
			res->side_abs, 
			res->side_rel, 
			res->main_abs, 
			res->main_rel, 
			res->npol_abs, 
			res->npol_rel, 
			res->pol_abs, 
			res->pol_rel);

	    /* Assign pointer for this chain */
	    res2 = res;
	}

	ajListIterDel(&iter);			
    } 	


    /* Write atom list to domain coordinate file */        
    for(nostart=ajFalse, noend=ajFalse, 
	z=0; z<scop->N;
	z++,found_start=ajFalse, found_end=ajFalse)
    {
	/*
	** Unknown or Zero length chains have already been checked for
	** so no additional checking is needed here
	*/
	ajPdbChnidToNum(scop->Chain[z], pdb, &chn);
	
/*	if(resarr)
	    AJFREE(resarr);
	ajListToarray(pdb->Chains[chn-1]->Residues, (void ***) &resarr);  */
	

	/* Initialise the iterator */
	iter = ajListIterNewread(pdb->Chains[chn-1]->Atoms);


	/* Increment res. counter from last chain if appropriate */
	if(noend)
	    rn_mod += atm2->Idx;
	else	 
	    rn_mod += finalrn;

	
	/* Check whether start and end of domain are specified */
	if(!ajStrCmpC(scop->Start[z], "."))
	    nostart = ajTrue;
	else
	    nostart=ajFalse;
	
	if(!ajStrCmpC(scop->End[z], "."))
	    noend = ajTrue;
	else 
	    noend = ajFalse;
	

	/* Iterate through the list of atoms */
	while((atm=(AjPAtom)ajListIterGet(iter)))
	{
	    /*
	    ** Continue if a non-protein atom is found or break if
	    ** model no. !=1
	    */
	    if(atm->Mod!=1)
		break;
	    if(atm->Type!='P')
		continue;


	    /*	if(atm->Mod!=1 || atm->Type!='P')
		break; */


	    
	    /* The start position has not been found yet */
	    if(!found_start)
	    {
		/* Start position was specified */
		if(!nostart)
		{
		    ajStrAssignS(&tmpstr, scop->Start[z]);
		    ajStrAppendK(&tmpstr, '*');

		    /* Start position found */
		    /*if(!ajStrCmpCaseS(atm->Pdb, scop->Start[z])) */
		    if(ajStrMatchWildS(atm->Pdb, tmpstr))      
/*		    if(ajStrMatchWildS(resarr[atm->Idx-1]->Pdb, tmpstr))	 */
		    {
			if(!ajStrMatchS(atm->Pdb, scop->Start[z])) 
			/* if(!ajStrMatchS(resarr[atm->Idx-1]->Pdb, scop->Start[z])) */
			{
			    ajWarn("Domain start found by wildcard match only "
				   "in ajPdbWriteDomain");
			    ajFmtPrintF(errf, "//\n%S\nERROR Domain "
					"start found "
					"by wildcard match only in "
					"ajPdbWriteDomain\n", scop->Entry);
			}
			    

			rn_mod -= atm->Idx-1;
			found_start = ajTrue;	
		    }
		    else	
			continue;
		}
		else	
		    found_start=ajTrue;	
	    }	


	    /*
	    ** The end position was specified, but has not 
	    ** been found yet
	    */
	    if(!found_end && !noend)
	    {
		ajStrAssignS(&tmpstr, scop->End[z]);
		ajStrAppendK(&tmpstr, '*');

		/* End position found */
		/*if(!ajStrCmpCaseS(atm->Pdb, scop->End[z])) */
		 if(ajStrMatchWildS(atm->Pdb, tmpstr)) 
		     /* if(ajStrMatchWildS(resarr[atm->Idx-1]->Pdb, tmpstr)) */
		{
		     if(!ajStrMatchS(atm->Pdb, scop->End[z]))  
        	 /* if(!ajStrMatchS(resarr[atm->Idx-1]->Pdb, scop->End[z])) */
		    {
			ajWarn("Domain end found by wildcard match only "
			       "in ajPdbWriteDomain");
			ajFmtPrintF(errf, "//\n%S\nERROR Domain end found "
				    "by wildcard match only in "
				    "ajPdbWriteDomain\n", scop->Entry);
		    }

		    found_end = ajTrue;     
		    finalrn   = atm->Idx;
		}
	    }	
	    /*
	    ** The end position was specified and has been found, and
	    ** the current atom no longer belongs to this final residue
	    */
	    else if(atm->Idx != finalrn && !noend)
		break;
	    


	    /* Print out coordinate line */
	    ajFmtPrintF(outf, "%-5s%-5d%-5d%-5c%-5d%-6S%-2c%-6S%-2c%-6S"
			"%9.3f%9.3f%9.3f%8.2f%8.2f\n", 
			"AT", 
			atm->Mod, /* It will always be 1 */
			atm->Chn, /* chn number is always given as 1 */
			'.', 
			atm->Idx+rn_mod, 
			atm->Pdb, 
			atm->Id1,
			atm->Id3,
			atm->Type, 
			atm->Atm, 
			atm->X, 
			atm->Y, 
			atm->Z,
			atm->O,
			atm->B);
	    
	    /* Assign pointer for this chain */
	    atm2 = atm;
	}

	ajListIterDel(&iter);			
    } 	



    /* Write last line in file */
    ajFmtPrintF(outf, "//\n");    
    

    /* Tidy up */
/*    if(resarr)
	AJFREE(resarr); */
    ajStrDel(&seq);
    ajStrDel(&tmpseq);
    ajStrDel(&tmpstr);    



    return ajTrue;
}







/* @func ajCathWrite *******************************************************
**
** Write contents of a Cath object to an output DCF file (domain 
** classification file) in DCF format (see documentation for DOMAINATRIX
** "cathparse" application).
** 
** @param [u] outf [AjPFile] Output file stream
** @param [r] obj  [const AjPCath] Cath object
**
** @return [AjBool] True if file was written ok.
** @@
****************************************************************************/
     
AjBool ajCathWrite(AjPFile outf, const AjPCath obj)  
{
    
    ajint i;
    AjPStr tmp;
    AjPSeqout outseq;

    /* Check args */
    if(!outf || !obj)
    {
	ajWarn("Bad args passed to ajCathWrite");
	return ajFalse;
    }
    


    tmp = ajStrNew();

    ajStrAssignS(&tmp, obj->DomainID);
    ajStrFmtUpper(&tmp);
    ajFmtPrintF(outf,"ID   %S\nXX\n",tmp);
    
    ajStrAssignS(&tmp, obj->Pdb);
    ajStrFmtUpper(&tmp);
    ajFmtPrintF(outf,"EN   %S\nXX\n",tmp);
    ajFmtPrintF(outf,"TY   CATH\nXX\n");    
    ajFmtPrintF(outf,"CI   %d CL; %d AR; %d TP; %d SF; %d FA; %d NI;"
		"%d IF;\nXX\n",
		obj->Class_Id, obj->Arch_Id, obj->Topology_Id,
		obj->Superfamily_Id, obj->Family_Id, obj->NIFamily_Id,
		obj->IFamily_Id);

    ajFmtPrintF(outf,"CL   %S\n",obj->Class);
    ajFmtPrintF(outf, "XX\n");
    ajFmtPrintSplit(outf,obj->Architecture,"AR   ",75," \t\n\r");
    ajFmtPrintF(outf, "XX\n");
    ajFmtPrintSplit(outf,obj->Topology,"TP   ",75," \t\n\r");
    ajFmtPrintF(outf, "XX\n");
    ajFmtPrintSplit(outf,obj->Superfamily,"SF   ",75," \t\n\r");
    ajFmtPrintF(outf, "XX\n");

    if(ajStrGetLen(obj->SeqPdb))
    {
	ajFmtPrintF(outf,"XX\n");
	outseq = ajSeqoutNewFile(outf);
	ajSeqoutDumpSwisslike(outseq, obj->SeqPdb, "DS");
	ajSeqoutDel(&outseq);
    }	

    ajFmtPrintF(outf,"NR   %d\n",obj->Length);

    /*
    ajFmtPrintF(outf,"CL   %S",obj->Class);
    ajFmtPrintSplit(outf,obj->Architecture,"\nXX\nAR   ",75," \t\n\r");
    ajFmtPrintSplit(outf,obj->Topology,"XX\nTP   ",75," \t\n\r");
    ajFmtPrintSplit(outf,obj->Superfamily,"XX\nSF   ",75," \t\n\r");
    ajFmtPrintF(outf,"XX\nNR   %d\n",obj->Length);
    */


    if(ajStrGetLen(obj->Acc))
	ajFmtPrintF(outf,"XX\nAC   %S\n",obj->Acc);    

    if(ajStrGetLen(obj->Spr))
	ajFmtPrintF(outf,"XX\nSP   %S\n",obj->Spr);

    if(ajStrGetLen(obj->SeqSpr))
    {
	ajFmtPrintF(outf, "XX\n%-5s%d START; %d END;\n", "RA", obj->Startd,
		    obj->Endd);
	ajFmtPrintF(outf, "XX\n");	
	outseq = ajSeqoutNewFile(outf);
	ajSeqoutDumpSwisslike(outseq, obj->SeqSpr, "SQ");
	ajSeqoutDel(&outseq);
    }
    

    ajFmtPrintF(outf,"XX\nNC   %d\n",obj->NSegment);
    
    for(i=0;i<obj->NSegment;++i)
    {
	ajFmtPrintF(outf,"XX\nCN   [%d]\n",i+1);
	
	ajFmtPrintF(outf,"XX\nCH   %c CHAIN; %S START; %S END;\n",
		    obj->Chain,
		    obj->Start[i],
		    obj->End[i]);
	
    }

    ajFmtPrintF(outf,"//\n");


    ajStrDel(&tmp);
    
    return ajTrue;
}    


/* @func ajDomainWrite ******************************************************
**
** Write contents of a Domain object to an output file in DCF format
** (see documentation for DOMAINATRIX "scopparse" application).
**
** @param [u] outf [AjPFile] Output file stream
** @param [r] obj  [const AjPDomain] Domain object
**
** @return [AjBool] True if file was written ok.
** @@
****************************************************************************/

AjBool ajDomainWrite(AjPFile outf, const AjPDomain obj)
{


    if(!outf || !obj)
    {
	ajWarn("Bad args passed to ajDomainWrite");
	return ajFalse;
    }

    if(obj->Type == ajSCOP)
	ajScopWrite(outf, obj->Scop);
    else 
    	ajCathWrite(outf, obj->Cath);

    return ajTrue;
}



 



/* @func ajScopWrite *******************************************************
**
** Write contents of a Scop object to a DCF file (domain classification 
** file).  The DCF file uses DCF format (see documentation for DOMAINATRIX
** "scopparse" application).
**
** @param [u] outf [AjPFile] Output file stream
** @param [r] obj  [const AjPScop] Scop object
**
** @return [AjBool] True if file was written ok.
** @@
****************************************************************************/

AjBool ajScopWrite(AjPFile outf, const AjPScop obj)
{
    ajint i;
    AjPSeqout outseq;

    if(!outf || !obj)
    {
	ajWarn("Bad args passed to ajScopWrite");
	return ajFalse;
    }
    

    ajFmtPrintF(outf,"ID   %S\nXX\n",obj->Entry);
    ajFmtPrintF(outf,"EN   %S\nXX\n",obj->Pdb);
    ajFmtPrintF(outf,"TY   SCOP\nXX\n");
    ajFmtPrintF(outf,"SI   %d CL; %d FO; %d SF; %d FA; %d DO; %d SO; "
		"%d DD;\nXX\n",
		obj->Sunid_Class,obj->Sunid_Fold, obj->Sunid_Superfamily,
		obj->Sunid_Family,obj->Sunid_Domain, obj->Sunid_Source,
		obj->Sunid_Domdat);

    ajFmtPrintF(outf,"CL   %S\n",obj->Class);
    ajFmtPrintF(outf, "XX\n");
    ajFmtPrintSplit(outf,obj->Fold,"FO   ",75," \t\n\r");
    ajFmtPrintF(outf, "XX\n");
    ajFmtPrintSplit(outf,obj->Superfamily,"SF   ",75," \t\n\r");
    ajFmtPrintF(outf, "XX\n");
    ajFmtPrintSplit(outf,obj->Family,"FA   ",75," \t\n\r");
    ajFmtPrintF(outf, "XX\n");
    ajFmtPrintSplit(outf,obj->Domain,"DO   ",75," \t\n\r");;
    ajFmtPrintF(outf,"XX\nOS   %S\n",obj->Source);

    if(ajStrGetLen(obj->SeqPdb))
    {
	ajFmtPrintF(outf,"XX\n");
	outseq = ajSeqoutNewFile(outf);
	ajSeqoutDumpSwisslike(outseq, obj->SeqPdb, "DS");		
	ajSeqoutDel(&outseq);
    }	

    if(ajStrGetLen(obj->Acc))
	ajFmtPrintF(outf,"XX\nAC   %S\n",obj->Acc);    

    if(ajStrGetLen(obj->Spr))
	ajFmtPrintF(outf,"XX\nSP   %S\n",obj->Spr);

    if(ajStrGetLen(obj->SeqSpr))
    {
	ajFmtPrintF(outf, "XX\n%-5s%d START; %d END;\n", "RA", obj->Startd,
		    obj->Endd);
	ajFmtPrintF(outf, "XX\n");	
	outseq = ajSeqoutNewFile(outf);
	ajSeqoutDumpSwisslike(outseq, obj->SeqSpr, "SQ");
	ajSeqoutDel(&outseq);
    }
    
    
    if(ajStrGetLen(obj->Sse))
        ajFmtPrintF(outf,"XX\nSE   %S\n",obj->Sse); 

    if(ajStrGetLen(obj->Sss))
	ajFmtPrintF(outf,"XX\n");
    
    if(ajStrGetLen(obj->Sss))
    {
	outseq = ajSeqoutNewFile(outf);
	ajSeqoutDumpSwisslike(outseq, obj->Sss, "SS");
	ajSeqoutDel(&outseq);
    }

    ajFmtPrintF(outf,"XX\nNC   %d\n",obj->N);

    for(i=0;i<obj->N;++i)
    {
	ajFmtPrintF(outf,"XX\nCN   [%d]\n",i+1);
	ajFmtPrintF(outf,"XX\nCH   %c CHAIN; %S START; %S END;\n",
		    obj->Chain[i],
		    obj->Start[i],
		    obj->End[i]);
    }
    ajFmtPrintF(outf,"//\n");
    
    return ajTrue;
}





/* ======================================================================= */
/* ======================== Miscellaneous =================================*/
/* ======================================================================= */

/* @section Miscellaneous ***************************************************
**
** These functions may have diverse functions that do not fit into the other
** categories. 
**
****************************************************************************/




/* @func ajDomainDCFType ****************************************************
**
** Reads a DCF file (domain classification file; see documentation for 
** DOMAINATRIX "scopparse" application) and assertains the type of domains 
** (ajSCOP or ajCATH) within.
** 
**
** @param [u] inf   [AjPFile]  Input file stream
**
** @return [ajint] ajSCOP, ajCATH or -1 (error) as appropriate.
** @@
****************************************************************************/

ajint ajDomainDCFType(AjPFile inf)
{
    ajlong  offset      = 0;
    static  AjPStr line = NULL;
    static  AjPStr tmp  = NULL;	


    /* Only initialise strings if this is called for the first time */
    if(!line)
    {
	line    = ajStrNew();
	tmp     = ajStrNew();
    }
    
    offset = ajFileTell(inf);
    
    while((ajFileReadLine(inf,&line)))
    {
	if(!ajStrPrefixC(line,"TY   "))
	    continue;
	
	ajFmtScanS(line, "%*S %S", &tmp);
	if(ajStrMatchC(tmp, "SCOP"))
	{
	    ajFileSeek(inf, offset, 0);
	    return ajSCOP;
	}
	
	else if(ajStrMatchC(tmp, "CATH"))
	{
	    ajFileSeek(inf, offset, 0);
	    return ajCATH;
	}
	else 
	{
	    ajWarn("Serious error: Unknown domain type in DCF file");
	    return -1;
	}
    }

    
    return -1;
}





/* @func ajDomainDummyFunction ************************************************
**
** Dummy function to catch all unused functions defined in the ajdomain
** source file.
**
** @return [void]
**
******************************************************************************/

void ajDomainDummyFunction(void)
{
    AjPStr str=NULL;
    AjPFile file=NULL;

    domainScopclaRead(file, str);
    domainScopdesRead(file, str);

    return;
}
