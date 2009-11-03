/****************************************************************************
**
** @source embsig.c
**
** Data structures and algorithms for use with sparse sequence signatures.
** For use with the the Hit, Hitlist, Sigpos and Signature objects. 
** 
** Copyright (c) 2004 Jon Ison
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
****************************************************************************/

#include "emboss.h"
#include <math.h>




/* ======================================================================= */
/* ============================ private data ============================= */
/* ======================================================================= */

/* @datastatic EmbPHitidx *****************************************************
**
** Nucleus Hitidx object.
**
** Holds data for an indexing Hit and Hitlist objects
**
** EmbPHitidx is implemented as a pointer to a C data structure.
**
** @alias EmbSHitidx
** @alias EmbOHitidx
**
** @attr Id [AjPStr] Identifier
** @attr hptr [EmbPHit] Pointer to EmbPHit structure
** @attr lptr [EmbPHitlist] Pointer to EmbPHitlist structure 
** @@
****************************************************************************/
typedef struct EmbSHitidx
{  
    AjPStr      Id;
    EmbPHit      hptr;
    EmbPHitlist  lptr;
}EmbOHitidx;
#define EmbPHitidx EmbOHitidx*


/* @datastatic EmbPSigcell ****************************************************
**
** Nucleus Sigcell object.
**
** Holds data for a cell of a path matrix for a signature:sequence alignment.
**
** EmbPSigcell is implemented as a pointer to a C data structure.
**
** @alias EmbSSigcell
** @alias EmbOSigcell
**
** @attr val [float] Value for this cell
** @attr prev [ajint] Index in path matrix of prev. highest value
** @attr visited [AjBool] ajTrue if this cell has been visited
** @@
****************************************************************************/
typedef struct EmbSSigcell
{
    float  val;
    ajint  prev;
    AjBool visited;
} EmbOSigcell;
#define EmbPSigcell EmbOSigcell*





/* ======================================================================= */
/* ================= Prototypes for private functions ==================== */
/* ======================================================================= */

static EmbPHitidx   embHitidxNew(void);

static void         embHitidxDel(EmbPHitidx *pthis);

static ajint        embHitidxBinSearch(const AjPStr id,
				EmbPHitidx const *arr, 
				ajint siz);

static ajint        embHitidxMatchId(const void *hit1, const void *hit2);


static AjBool       embHitlistReadFam(AjPFile scopf, 
			       const AjPStr fam,
			       const AjPStr sfam, 
			       const AjPStr fold, 
			       const AjPStr klass,
			       AjPList* list);

static AjBool       embHitlistReadSfam(AjPFile scopf,
				const AjPStr fam, 
				const AjPStr sfam, 
				const AjPStr fold, 
				const AjPStr klass,
				AjPList* list);

static AjBool       embHitlistReadFold(AjPFile scopf,
				const AjPStr fam, 
				const AjPStr fold, 
				const AjPStr klass,
				AjPList* list);

static AjBool       embHitlistReadFamFasta(AjPFile scopf, 
				    const AjPStr fam, 
				    const AjPStr sfam, 
				    const AjPStr fold, 
				    const AjPStr klass, 
				    AjPList* list);

static AjBool       embHitlistReadSfamFasta(AjPFile scopf, 
				     const AjPStr fam, 
				     const AjPStr sfam, 
				     const AjPStr fold, 
				     const AjPStr klass,
				     AjPList* list);

static AjBool       embHitlistReadFoldFasta(AjPFile scopf, 
				     const AjPStr fam,
				     const AjPStr fold,
				     const AjPStr klass,
				     AjPList* list);




/* ======================================================================= */
/* ========================== private functions ========================== */
/* ======================================================================= */
/* @func embSigdatNew *******************************************************
**
** Sigdat object constructor. This is normally called by the 
** embSignatureReadNew function. Fore-knowledge of the number of empirical 
** residues and gaps is required.
** Important: Functions which manipulate the Sigdat object rely on data in 
** the gap arrays (gsiz and grfq) being filled in order of increasing gap 
** size.
**
** @param [r] nres [ajuint]   Number of emprical residues / environments.
** @param [r] ngap [ajuint]   Number of emprical gaps.
** 
** @return [EmbPSigdat] Pointer to a Sigdat object
** @@
****************************************************************************/

EmbPSigdat embSigdatNew(ajuint nres, ajuint ngap)
{
    EmbPSigdat ret = NULL;
    ajuint x=0;
    

    AJNEW0(ret);
    ret->nres = nres;
    ret->nenv = nres;
    ret->ngap = ngap;

    if(ngap)
    {
	ret->gsiz = ajUintNewL((ajint) ngap);
	ret->gfrq = ajUintNewL((ajint) ngap);
	ajUintPut(&ret->gsiz, ngap-1, (ajint)0);
	ajUintPut(&ret->gfrq, ngap-1, (ajint)0);
    }
    else
    {
	ret->gsiz = ajUintNew();
	ret->gfrq = ajUintNew();
	ajUintPut(&ret->gsiz, 0, (ajint)0);
	ajUintPut(&ret->gfrq, 0, (ajint)0);
    }

    if(nres)
    {
	ret->rids = ajChararrNewL((ajint) nres);
	ret->rfrq = ajUintNewL((ajint) nres);
        ajUintPut(&ret->rfrq, nres-1, (ajint)0);
	ajChararrPut(&ret->rids, nres-1, (char)' ');

	AJCNEW0(ret->eids, nres);
	for(x=0;x<nres;x++)
	    ret->eids[x]=ajStrNew();
	ret->efrq = ajUintNewL((ajint) nres);
        ajUintPut(&ret->efrq, nres-1, (ajint)0);
    }
    else
    {
	ret->rids = ajChararrNew();
	ret->rfrq = ajUintNew();
	ajUintPut(&ret->rfrq, 0, (ajint)0);
	ajChararrPut(&ret->rids, 0, (char)' ');

	/* ret->eids is *NOT* allocated in this case. */
	ret->efrq = ajUintNew();
	ajUintPut(&ret->efrq, 0, (ajint)0);
    }
    
    return ret;
}




/* @func embSigposNew *******************************************************
**
** Sigpos object constructor. This is normally called by the
** embSignatureCompile function. Fore-knowledge of the number of permissible
** gaps is required.
**
** @param [r] ngap [ajuint]   Number of permissible gaps.
** 
** @return [EmbPSigpos] Pointer to a Sigpos object
** @@
****************************************************************************/

EmbPSigpos embSigposNew(ajuint ngap)
{
    EmbPSigpos ret = NULL;
    
    AJNEW0(ret);
    ret->ngaps = ngap;
    
    /* Create arrays */
    AJCNEW0(ret->gsiz, ngap);
    AJCNEW0(ret->gpen, ngap);
    AJCNEW0(ret->subs, 26);
        
    return ret;
}




/* @func embSigposDel *******************************************************
**
** Destructor for Sigpos object.
**
** @param [w] pthis [EmbPSigpos*] Sigpos object pointer
**
** @return [void]
** @@
****************************************************************************/

void embSigposDel(EmbPSigpos *pthis)
{
    AJFREE((*pthis)->gsiz);
    AJFREE((*pthis)->gpen);
    AJFREE((*pthis)->subs);

    AJFREE(*pthis); 
    *pthis = NULL;
    
    return; 
}





/* @func embSigdatDel *******************************************************
**
** Destructor for Sigdat object.
**
** @param [w] pthis [EmbPSigdat*] Sigdat object pointer
**
** @return [void]
** @@
****************************************************************************/

void embSigdatDel(EmbPSigdat *pthis)
{
    ajuint x=0;
    
    ajUintDel(&(*pthis)->gsiz);
    ajUintDel(&(*pthis)->gfrq);
    ajUintDel(&(*pthis)->rfrq);
    ajChararrDel(&(*pthis)->rids);

    if((*pthis)->eids)
    {
	for(x=0; x<(*pthis)->nres; x++)
	    ajStrDel(&((*pthis)->eids[x]));
	AJFREE((*pthis)->eids);    
    }
    ajUintDel(&(*pthis)->efrq);

    AJFREE(*pthis);    
    *pthis = NULL;

    return; 
}




/* @funcstatic embHitidxNew ***************************************************
**
** Hitidx object constructor. This is normally called by the 
** embHitlistClassify function.
**
** @return [EmbPHitidx] Pointer to a Hitidx object
** @@
****************************************************************************/

static EmbPHitidx embHitidxNew(void)
{
    EmbPHitidx ret  =NULL;

    AJNEW0(ret);

    ret->Id         = ajStrNew();
    ret->hptr       = NULL;
    ret->lptr       = NULL;
    
    return ret;
}




/* @funcstatic embHitidxDel **************************************************
**
** Destructor for Hitidx object.
**
** @param [w] pthis [EmbPHitidx*] Hitidx object pointer
**
** @return [void]
** @@
****************************************************************************/

static void embHitidxDel(EmbPHitidx *pthis)
{
    ajStrDel(&(*pthis)->Id);

    AJFREE(*pthis);
    *pthis = NULL;
    
    return;
}




/* @funcstatic embHitidxBinSearch *********************************************
**
** Performs a binary search for an accession number over an array of Hitidx
** structures (which of course must first have been sorted). This is a 
** case-insensitive search.
**
** @param [r] id  [const AjPStr]       Search term
** @param [r] arr [EmbPHitidx const *]  Array of EmbOHitidx objects
** @param [r] siz [ajint]        Size of array
**
** @return [ajint] Index of first Hitidx object found with an Id element 
** matching id, or -1 if id is not found.
** @@
****************************************************************************/

static ajint embHitidxBinSearch(const AjPStr id, EmbPHitidx const *arr,
				ajint siz)
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

        if((c=ajStrCmpCaseS(id, arr[m]->Id)) < 0) 
	    h = m-1;
        else if(c>0) 
	    l = m+1;
        else 
	    return m;
    }

    return -1;
}




/* @funcstatic embHitidxMatchId ***********************************************
**
** Function to sort Hitidx objects by Id element. Usually called by 
** embHitlistClassify.
**
** @param [r] hit1  [const void*] Pointer to Hitidx object 1
** @param [r] hit2  [const void*] Pointer to Hitidx object 2
**
** @return [ajint] -1 if Id1 should sort before Id2, +1 if the Id2 should sort 
** first. 0 if they are identical in length and content. 
** @@
****************************************************************************/

static ajint embHitidxMatchId(const void *hit1, const void *hit2)
{
    const EmbPHitidx p = NULL;
    const EmbPHitidx q = NULL;

    p = (*(EmbPHitidx const *)hit1);
    q = (*(EmbPHitidx const *)hit2);
    
    return ajStrCmpS(p->Id, q->Id);

}




/* @funcstatic embHitlistReadFam **********************************************
**
** Reads a domain families file (see documentation for the EMBASSY 
** DOMAINATRIX package), selects the entries with the specified family, 
** and create a list of Hitlist structures.  Only the first familiy in the 
** scop families file matching the specified classification is read (the file
** should not normally contain duplicate families).
**
** @param [u] scopf     [AjPFile]    The scop families file.
** @param [r] fam       [const AjPStr]     Family
** @param [r] sfam      [const AjPStr]     Superfamily
** @param [r] fold      [const AjPStr]     Fold
** @param [r] klass     [const AjPStr]     Class
** @param [w] list      [AjPList*]   A list of hitlist structures.
** 
** @return [AjBool] True on success (a file has been written)
** @@
****************************************************************************/

static AjBool embHitlistReadFam(AjPFile scopf,
				const AjPStr fam, const AjPStr sfam,
				const AjPStr fold, 
				const AjPStr klass, AjPList* list)
{
    EmbPHitlist hitlist = NULL; 
    AjBool done        = ajFalse;
    const AjPStr class       = NULL;

    class = klass;

    /*
    ** if family is specified then the other fields also have to be
    ** specified. check that the other fields are populated
    */
    if(!fam || !sfam || !fold || !class)
    {
	ajWarn("Bad arguments passed to embHitlistReadFam\n");
	return ajFalse;
    }
    

    while((hitlist=embHitlistRead(scopf)))
    {
	if(ajStrMatchS(fam,hitlist->Family) &&
	   ajStrMatchS(sfam,hitlist->Superfamily) &&
	   ajStrMatchS(fold,hitlist->Fold) &&
	   ajStrMatchS(class,hitlist->Class))
	{ 
	    ajListPushAppend(*list,hitlist);
	    done=ajTrue;
	    break;
	}
	else
	    embHitlistDel(&hitlist);
    }
    
    if(done)
	return ajTrue;

    return ajFalse;
}





/* @funcstatic embHitlistReadSfam *********************************************
**
** Reads a domain families file (see documentation for the EMBASSY 
** DOMAINATRIX package), selects the entries with the specified 
** superfamily, and create a list of Hitlist structures.
**
** @param [u] scopf     [AjPFile]      The scop families file.
** @param [r] fam       [const AjPStr]       Family
** @param [r] sfam      [const AjPStr]       Superfamily
** @param [r] fold      [const AjPStr]       Fold
** @param [r] klass     [const AjPStr]       Class
** @param [w] list      [AjPList*]     A list of hitlist structures.
** 
** @return [AjBool] True on success (a file has been written)
** @@
****************************************************************************/

static AjBool embHitlistReadSfam(AjPFile scopf,
				 const AjPStr fam, const AjPStr sfam,
				 const AjPStr fold, const AjPStr klass,
				 AjPList* list)
{
    EmbPHitlist hitlist = NULL; 
    AjBool done  = ajFalse;
    const AjPStr class = NULL;

    class = klass;
    
    /*
    ** if family is specified then the other fields also have to be
    ** specified.  check that the other fields are populated
    */
    if(!sfam || !fold || !class)
    {
	ajWarn("Bad arguments passed to embHitlistReadSfam\n");
	return ajFalse;
    }
    
    
    while((hitlist=embHitlistRead(scopf)))
	if(ajStrMatchS(fam,hitlist->Superfamily) &&
	   ajStrMatchS(fold,hitlist->Fold) &&
	   ajStrMatchS(class,hitlist->Class))
	{
	    ajListPushAppend(*list,hitlist);
	    done=ajTrue;
	}
	else
	    embHitlistDel(&hitlist);

    if(done)
	return ajTrue;

    return ajFalse;
}





/* @funcstatic embHitlistReadFold *********************************************
**
** Reads a domain families file (see documentation for the EMBASSY 
** DOMAINATRIX package), selects the entries with the specified 
** fold, and create a list of Hitlist structures.
**
** @param [u] scopf     [AjPFile]      The scop families file.
** @param [r] fam       [const AjPStr]       Family
** @param [r] fold      [const AjPStr]       Fold
** @param [r] klass     [const AjPStr]       Class
** @param [w] list      [AjPList*]     A list of hitlist structures.
** @return [AjBool] ajTrue on success
** @@
****************************************************************************/

static AjBool embHitlistReadFold(AjPFile scopf,
				 const AjPStr fam,
				 const AjPStr fold, const AjPStr klass,
				 AjPList* list)
{
    EmbPHitlist hitlist = NULL; 
    AjBool done  = ajFalse;
    const AjPStr class = NULL;

    class = klass;
    
    /*
    ** if family is specified then the other fields also have to be
    ** specified.  check that the other fields are populated
    */ 
    if(!fold || !class)
    {
	ajWarn("Bad arguments passed to embHitlistReadFold\n");
	return ajFalse;
    }
    
    while((hitlist = embHitlistRead(scopf)))
	if(ajStrMatchS(fam,hitlist->Fold) &&
	   ajStrMatchS(class,hitlist->Class))
	{
	    ajListPushAppend(*list,hitlist);
	    done=ajTrue;
	}
	else
	    embHitlistDel(&hitlist);

    if(done)
	return ajTrue;

    return ajFalse;	
}





/* @funcstatic embHitlistReadFamFasta *****************************************
**
** Reads a domain families file in extended FASTA format (see 
** documentation for the EMBASSY 
** DOMAINATRIX package), selects the entries with the specified family, 
** and create a list of Hitlist structures.  Only the first familiy in the 
** scop families file matching the specified classification is read (the file
** should not normally contain duplicate families).
**
** @param [u] scopf     [AjPFile]    The scop families file.
** @param [r] fam       [const AjPStr]     Family
** @param [r] sfam      [const AjPStr]     Superfamily
** @param [r] fold      [const AjPStr]     Fold
** @param [r] klass     [const AjPStr]     Class
** @param [w] list      [AjPList*]   A list of hitlist structures.
** 
** @return [AjBool] True on success (a file has been written)
** @@
****************************************************************************/

static AjBool embHitlistReadFamFasta(AjPFile scopf,
				     const AjPStr fam, const AjPStr sfam,
				     const AjPStr fold, 
				     const AjPStr klass, AjPList* list)
{
    EmbPHitlist hitlist = NULL; 
    AjBool done        = ajFalse;
    const AjPStr class       = NULL;

    class = klass;

    /*
    ** if family is specified then the other fields also have to be
    ** specified. check that the other fields are populated
    */
    if(!fam || !sfam || !fold || !class)
    {
	ajWarn("Bad arguments passed to embHitlistReadFamFasta\n");
	return ajFalse;
    }
    

    while((hitlist=embHitlistReadFasta(scopf)))
    {
	if(ajStrMatchS(fam,hitlist->Family) &&
	   ajStrMatchS(sfam,hitlist->Superfamily) &&
	   ajStrMatchS(fold,hitlist->Fold) &&
	   ajStrMatchS(class,hitlist->Class))
	{ 
	    ajListPushAppend(*list,hitlist);
	    done=ajTrue;
	    break;
	}
	else
	    embHitlistDel(&hitlist);
    }
    
    if(done)
	return ajTrue;

    return ajFalse;
}





/* @funcstatic embHitlistReadSfamFasta ****************************************
**
** Reads a domain families file in extended FASTA format (see documentation 
** for the EMBASSY 
** DOMAINATRIX package), selects the entries with the specified 
** superfamily, and create a list of Hitlist structures.
**
** @param [u] scopf     [AjPFile]      The scop families file.
** @param [r] fam       [const AjPStr]       Family
** @param [r] sfam      [const AjPStr]       Superfamily
** @param [r] fold      [const AjPStr]       Fold
** @param [r] klass     [const AjPStr]       Class
** @param [w] list      [AjPList*]     A list of hitlist structures.
** 
** @return [AjBool] True on success (a file has been written)
** @@
****************************************************************************/

static AjBool embHitlistReadSfamFasta(AjPFile scopf, const AjPStr fam,
				      const AjPStr sfam,
				      const AjPStr fold, const AjPStr klass,
				      AjPList* list)
{
    EmbPHitlist hitlist = NULL; 
    AjBool done  = ajFalse;
    const AjPStr class = NULL;

    class = klass;
    
    /*
    ** if family is specified then the other fields also have to be 
   ** specified.  check that the other fields are populated
    */
    if(!sfam || !fold || !class)
    {
	ajWarn("Bad arguments passed to embHitlistReadSfamFasta\n");
	return ajFalse;
    }
    
    
    while((hitlist=embHitlistReadFasta(scopf)))
	if(ajStrMatchS(fam,hitlist->Superfamily) &&
	   ajStrMatchS(fold,hitlist->Fold) &&
	   ajStrMatchS(class,hitlist->Class))
	{
	    ajListPushAppend(*list,hitlist);
	    done=ajTrue;
	}
	else
	    embHitlistDel(&hitlist);

    if(done)
	return ajTrue;

    return ajFalse;
}





/* @funcstatic embHitlistReadFoldFasta ****************************************
**
** Reads a domain families file in extended FASTA format (see 
** documentation for the EMBASSY 
** DOMAINATRIX package), selects the entries with the specified 
** fold, and create a list of Hitlist structures.
**
** @param [u] scopf     [AjPFile]      The scop families file.
** @param [r] fam       [const AjPStr]       Family
** @param [r] fold      [const AjPStr]       Fold
** @param [r] klass     [const AjPStr]       Class
** @param [w] list      [AjPList*]     A list of hitlist structures.
** @return [AjBool] ajTrue on success
** @@
****************************************************************************/

static AjBool embHitlistReadFoldFasta(AjPFile scopf, const AjPStr fam,
				      const AjPStr fold, const AjPStr klass,
				      AjPList* list)
{
    EmbPHitlist hitlist = NULL; 
    AjBool done  = ajFalse;
    const AjPStr class = NULL;

    class = klass;
    
    /*
    ** if family is specified then the other fields also have to be
    ** specified.  check that the other fields are populated
    */ 
    if(!fold || !class)
    {
	ajWarn("Bad arguments passed to embHitlistReadFoldFasta\n");
	return ajFalse;
    }
    
    while((hitlist = embHitlistReadFasta(scopf)))
	if(ajStrMatchS(fam,hitlist->Fold) &&
	   ajStrMatchS(class,hitlist->Class))
	{
	    ajListPushAppend(*list,hitlist);
	    done=ajTrue;
	}
	else
	    embHitlistDel(&hitlist);

    if(done)
	return ajTrue;

    return ajFalse;	
}










/* ======================================================================= */
/* =========================== constructors ============================== */
/* ======================================================================= */

/* @section Constructors ****************************************************
**
** All constructors return a pointer to a new instance. It is the 
** responsibility of the user to first destroy any previous instance. The 
** target pointer does not need to be initialised to NULL, but it is good 
** programming practice to do so anyway.
**
****************************************************************************/

/* @func embHitlistNew ******************************************************
**
** Hitlist object constructor. This is normally called by the embHitlistRead
** function. Fore-knowledge of the number of hits is required.
**
** @param [r] n [ajuint] Number of hits
** 
** @return [EmbPHitlist] Pointer to a hitlist object
** @@
****************************************************************************/

EmbPHitlist embHitlistNew(ajuint n)
{
    EmbPHitlist ret = NULL;
    ajuint i = 0;
    

    AJNEW0(ret);
    ret->Class       = ajStrNew();
    ret->Architecture = ajStrNew();
    ret->Topology     = ajStrNew();
    ret->Fold        = ajStrNew();
    ret->Superfamily = ajStrNew();
    ret->Family      = ajStrNew();
    ret->Model       = ajStrNew();
    ret->Priority    = ajFalse;
    
    ret->N=n;

    if(n)
    {
	AJCNEW0(ret->hits,n);
	for(i=0;i<n;++i)
	    ret->hits[i] = embHitNew();
    }	

    return ret;
}





/* @func embHitNew **********************************************************
**
** Hit object constructor. This is normally called by the embHitlistNew
** function.
**
** @return [EmbPHit] Pointer to a hit object
** @@
****************************************************************************/

EmbPHit embHitNew(void)
{
    EmbPHit ret = NULL;

    AJNEW0(ret);

    ret->Seq       = ajStrNew();
    ret->Acc       = ajStrNew();
    ret->Spr       = ajStrNew();
    ret->Dom       = ajStrNew();
    ret->Typeobj   = ajStrNew();
    ret->Typesbj   = ajStrNew();
    ret->Model     = ajStrNew();
    ret->Alg       = ajStrNew();
    ret->Group     = ajStrNew();
    ret->Start     = 0;
    ret->End       = 0;
    ret->Rank      = 0;
    ret->Score     = 0;    
    ret->Eval      = 0;
    ret->Pval      = 0;
    ret->Target    = ajFalse;
    ret->Target2   = ajFalse;
    ret->Priority  = ajFalse;

    return ret;
}







/* @func embSignatureNew ****************************************************
**
** Signature object constructor. This is normally called by the
** embSignatureReadNew function. Fore-knowledge of the number of signature 
** positions is required.
**
** @param [r] n [ajuint]   Number of signature positions
** 
** @return [EmbPSignature] Pointer to a Signature object
** @@
****************************************************************************/

EmbPSignature embSignatureNew(ajuint n)
{
    EmbPSignature ret = NULL;


    AJNEW0(ret);
    ret->Class        = ajStrNew();
    ret->Architecture = ajStrNew();
    ret->Topology     = ajStrNew();
    ret->Fold         = ajStrNew();
    ret->Superfamily  = ajStrNew();
    ret->Family       = ajStrNew();

    ret->Id           = ajStrNew();
    ret->Domid        = ajStrNew();
    ret->Ligid        = ajStrNew();
    ret->Desc         = ajStrNew();

    ret->npos = n;

    /* Create arrays of pointers to Sigdat & Sigpos structures */
    if(n)
    {
	ret->dat = AJCALLOC0(n, sizeof(EmbPSigdat));
	ret->pos = AJCALLOC0(n, sizeof(EmbPSigpos));
    }
    
    return ret;
}





/* ======================================================================= */
/* =========================== destructors =============================== */
/* ======================================================================= */

/* @section Structure Destructors *******************************************
**
** All destructor functions receive the address of the instance to be
** deleted.  The original pointer is set to NULL so is ready for re-use.
**
****************************************************************************/

/* @func embHitlistDel ******************************************************
**
** Destructor for hitlist object.
**
** @param [w] ptr [EmbPHitlist*] Hitlist object pointer
**
** @return [void]
** @@
****************************************************************************/

void embHitlistDel(EmbPHitlist *ptr)
{
    ajuint x = 0;  /* Counter */

    if(!(*ptr))
    {
	ajWarn("Null pointer passed to embHitlistDel");
	return;
    }
    
    if((*ptr)->Class)
	ajStrDel(&(*ptr)->Class);
    if((*ptr)->Architecture)
	ajStrDel(&(*ptr)->Architecture);
    if((*ptr)->Topology)
	ajStrDel(&(*ptr)->Topology);
    if((*ptr)->Fold)
	ajStrDel(&(*ptr)->Fold);
    if((*ptr)->Superfamily)
	ajStrDel(&(*ptr)->Superfamily);
    if((*ptr)->Family)
	ajStrDel(&(*ptr)->Family);
    if((*ptr)->Model)
	ajStrDel(&(*ptr)->Model);
    
    for(x=0;x<(*ptr)->N; x++)
	if((*ptr)->hits[x])
	    embHitDel(&(*ptr)->hits[x]);

    if((*ptr)->hits)
	AJFREE((*ptr)->hits);
    
    if(*ptr)
	AJFREE(*ptr);
    
    *ptr = NULL;
    
    return;
}





/* @func embHitDel **********************************************************
**
** Destructor for hit object.
**
** @param [w] ptr [EmbPHit*] Hit object pointer
**
** @return [void]
** @@
****************************************************************************/

void embHitDel(EmbPHit *ptr)
{
    ajStrDel(&(*ptr)->Seq);
    ajStrDel(&(*ptr)->Acc);
    ajStrDel(&(*ptr)->Spr);
    ajStrDel(&(*ptr)->Dom);
    ajStrDel(&(*ptr)->Typeobj);
    ajStrDel(&(*ptr)->Typesbj);
    ajStrDel(&(*ptr)->Model);
    ajStrDel(&(*ptr)->Alg);
    ajStrDel(&(*ptr)->Group);

    AJFREE(*ptr);
    *ptr = NULL;
    
    return;
}





/* @func embSignatureDel ****************************************************
**
** Destructor for Signature object.
**
** @param [w] ptr [EmbPSignature*] Signature object pointer
**
** @return [void]
** @@
****************************************************************************/

void embSignatureDel(EmbPSignature *ptr)
{
    ajuint x = 0;
    
    if(!(*ptr))
	return;
    
    if((*ptr)->dat)
	for(x=0;x<(*ptr)->npos; ++x)
	    if((*ptr)->dat[x])
		embSigdatDel(&(*ptr)->dat[x]);

    if((*ptr)->pos)
	for(x=0;x<(*ptr)->npos; ++x)
	    if((*ptr)->pos[x])
		embSigposDel(&(*ptr)->pos[x]);

    if((*ptr)->Class)
	ajStrDel(&(*ptr)->Class);
    if((*ptr)->Architecture)
	ajStrDel(&(*ptr)->Architecture);
    if((*ptr)->Topology)
	ajStrDel(&(*ptr)->Topology);
    if((*ptr)->Fold)
	ajStrDel(&(*ptr)->Fold);
    if((*ptr)->Superfamily)
	ajStrDel(&(*ptr)->Superfamily);
    if((*ptr)->Family)
	ajStrDel(&(*ptr)->Family);

    if((*ptr)->Id)
	ajStrDel(&(*ptr)->Id);
    if((*ptr)->Domid)
	ajStrDel(&(*ptr)->Domid);
    if((*ptr)->Ligid)
	ajStrDel(&(*ptr)->Ligid);
    if((*ptr)->Desc)
	ajStrDel(&(*ptr)->Desc);


    if((*ptr)->dat)
	AJFREE((*ptr)->dat);

    if((*ptr)->pos)
	AJFREE((*ptr)->pos);

    AJFREE(*ptr);    
    *ptr = NULL;

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


/* @func embHitMerge ********************************************************
**
** Creates a new Hit object which corresponds to a merging of the two 
** sequences from the Hit objects hit1 and hit2. 
**
** 
** @param [r] hit1     [const EmbPHit]  Hit 1
** @param [r] hit2     [const EmbPHit]  Hit 2
**
** @return [EmbPHit] Pointer to Hit object.
** @@
****************************************************************************/

EmbPHit embHitMerge(const EmbPHit hit1, const EmbPHit hit2)
{
    EmbPHit ret;
    ajuint start = 0;      /* Start of N-terminal-most sequence */
    ajuint end   = 0;      /* End of N-terminal-most sequence */
    AjPStr temp = NULL;
    
    /* Check args */
    if(!hit1 || !hit2)
    {
	ajWarn("Bad arg's passed to EmbPHitMerge");
	return NULL;
    }

    if(!ajStrMatchS(hit1->Acc, hit2->Acc))
    {
	ajWarn("Merge attempted on 2 hits with different accession numbers");
	return NULL;
    }

    /* Allocate memory */
    ret  = embHitNew();
    temp = ajStrNew();
    

    ajStrAssignS(&(ret->Acc), hit1->Acc);
    ajStrAssignS(&(ret->Spr), hit1->Spr);
    ajStrAssignS(&(ret->Dom), hit1->Dom);
        


    /*
    ** Copy the N-terminal most sequence to our new sequence 
    ** and assign start point of new hit
    */
    if(hit1->Start <= hit2->Start)
    {
	ajStrAssignS(&(ret->Seq), hit1->Seq);
	ret->Start = hit1->Start;
	end   = hit1->End;
	start = hit2->Start;
    }	
    else
    {
	ajStrAssignS(&(ret->Seq), hit2->Seq);
    	ret->Start = hit2->Start;
	end   = hit2->End;
	start = hit1->Start;
    }
    

    /* Assign end point of new hit */
    if(hit1->End >= hit2->End)
	ret->End = hit1->End;
    else
    	ret->End = hit2->End;


    /* Assign the C-terminus of the sequence of the new hit     
       if necessary */
    if(hit2->End > end)
    {
	ajStrAssignSubS(&temp, hit2->Seq, end-start+1, -1);
	ajStrAppendS(&(ret->Seq),temp);
    }
    else if(hit1->End > end)
    {
	ajStrAssignSubS(&temp, hit1->Seq, end-start+1, -1);
	ajStrAppendS(&(ret->Seq),temp);
    }


    /* Classify the merged hit 
       The Typeobj of the merged hit is set.  The merged hit is classified 
       as follows :
       If hit1 or hit2 is a SEED, the merged hit is classified as a SEED.
       Otherwise, if hit1 or hit2 is HIT, the merged hit is clsasified as a HIT.
       If hit1 and hit2 are both OTHER, the merged hit remains classified as 
       OTHER. */
    /*
    if(ajStrMatchC(hit1->Typeobj, "SEED") ||
       ajStrMatchC(hit1->Typeobj, "SEED"))
	ajStrAssignC(&(ret->Typeobj), "SEED");
    else if(ajStrMatchC(hit1->Typeobj, "HIT") ||
	    ajStrMatchC(hit1->Typeobj, "HIT"))
	ajStrAssignC(&(ret->Typeobj), "HIT");
    else
	ajStrAssignC(&(ret->Typeobj), "OTHER");
	*/

    if(ajStrMatchS(hit1->Model, hit2->Model))
	ajStrAssignS(&ret->Model, hit1->Model);
    

    
    /* All other elements of structure are left as NULL / o / ajFalse */
        
    ajStrDel(&temp);

    return ret;
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


/* @func embHitlistMatchFold ************************************************
**
** Function to sort Hitlist object by Fold element. 
**
** @param [r] hit1  [const void*] Pointer to Hitlist object 1
** @param [r] hit2  [const void*] Pointer to Hitlist object 2
**
** @return [ajint] -1 if Fold1 should sort before Fold2, +1 if the Fold2 
** should sort first. 0 if they are identical.
** @@
****************************************************************************/

ajint embHitlistMatchFold(const void *hit1, const void *hit2)
{
    EmbPHitlist p = NULL;
    EmbPHitlist q = NULL;

    p = (*(EmbPHitlist const *)hit1);
    q = (*(EmbPHitlist const *)hit2);
    
    return ajStrCmpS(p->Fold, q->Fold);
}





/* @func embMatchScore ******************************************************
**
** Function to sort Hit objects by score record. Usually called by 
** ajListSort.
**
** @param [r] hit1  [const void*] Pointer to Hit object 1
** @param [r] hit2  [const void*] Pointer to Hit object 2
**
** @return [ajint] 1 if score1<score2, 0 if score1==score2, else -1.
** @@
****************************************************************************/

ajint embMatchScore(const void *hit1, const void *hit2)
{
    EmbPHit p = NULL;
    EmbPHit q = NULL;

    p = (*(EmbPHit const *)hit1);
    q = (*(EmbPHit const *)hit2);
    
    if(p->Score < q->Score)
	return -1;
    else if (p->Score == q->Score)
	return 0;

    return 1;
}



/* @func embMatchinvScore ***************************************************
**
** Function to sort Hit objects by score record. Usually called by 
** ajListSort.  The sorting order is inverted - i.e. it returns -1 if score1 
** > score2 (as opposed to embMatchScore).
**
** @param [r] hit1  [const void*] Pointer to Hit object 1
** @param [r] hit2  [const void*] Pointer to Hit object 2
**
** @return [ajint] 1 if score1<score2, 0 if score1==score2, else -1.
** @@
****************************************************************************/

ajint embMatchinvScore(const void *hit1, const void *hit2)
{
    EmbPHit p = NULL;
    EmbPHit q = NULL;

    p = (*(EmbPHit const *)hit1);
    q = (*(EmbPHit const *)hit2);
    
    if(p->Score > q->Score)
	return -1;
    else if (p->Score == q->Score)
	return 0;

    return 1;
}






/* @func embMatchLigid ********************************************************
**
** Function to sort Hit objects by Ligid record (referenced via Sig element).
**
** @param [r] hit1  [const void*] Pointer to Hit object 1
** @param [r] hit2  [const void*] Pointer to Hit object 2
**
** @return [ajint] -1 if Ligid1 should sort before Ligid2, +1 if the Ligid2 
** should sort first. 0 if they are identical.
** @@
******************************************************************************/

ajint embMatchLigid(const void *hit1, const void *hit2)
{
    EmbPHit p = NULL;
    EmbPHit q = NULL;

    p = (*(EmbPHit const *)hit1);
    q = (*(EmbPHit const *)hit2);
    
    return ajStrCmpS(p->Sig->Ligid, q->Sig->Ligid);
}




/* @func embMatchSN ******************************************************
**
** Function to sort Hit objects by sn element within Sig element. Usually
** called by ajListSort.
**
** @param [r] hit1  [const void*] Pointer to Hit object 1
** @param [r] hit2  [const void*] Pointer to Hit object 2
**
** @return [ajint] 1 if sn1<sn2, 0 if sn1==sn2, else -1.
** @@
****************************************************************************/

ajint embMatchSN(const void *hit1, const void *hit2)
{
    EmbPHit p = NULL;
    EmbPHit q = NULL;

    p = (*(EmbPHit const *)hit1);
    q = (*(EmbPHit const *)hit2);
    
    if(p->Sig->sn < q->Sig->sn)
	return -1;
    else if (p->Sig->sn == q->Sig->sn)
	return 0;

    return 1;
}


/* ======================================================================= */
/* ============================== Casts ===================================*/
/* ======================================================================= */

/* @section Casts ***********************************************************
**
** These functions examine the contents of an instance and return some
** derived information. Some of them provide access to the internal
** components of an instance. They are provided for programming convenience
** but should be used with caution.
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

/* @func embHitsOverlap *****************************************************
**
** Checks for overlap between two hits.
**
** @param [r] hit1  [const EmbPHit]     Pointer to hit object 1
** @param [r] hit2  [const EmbPHit]     Pointer to hit object 2
** @param [r] n     [ajuint]      Threshold number of residues for overlap
**
** @return [AjBool] True if the overlap between the sequences is at least as 
** long as the threshold. False otherwise.
** @@
****************************************************************************/

AjBool embHitsOverlap(const EmbPHit hit1, const EmbPHit hit2, ajuint n)
{
    ajuint len1 = 0;
    ajuint len2 = 0;

    ajDebug("embHitsOverlap n:%u\n",
	    n);

    if((MAJSTRGETLEN(hit1->Seq) && (MAJSTRGETLEN(hit2->Seq))))
	if((MAJSTRGETLEN(hit1->Seq)<n) || (MAJSTRGETLEN(hit2->Seq)<n))
	{
	    ajWarn("Sequence length smaller than overlap limit in "
		   "embHitsOverlap ... checking for string match instead");
	    ajDebug("At least one sequence length %u, %u smaller than %u\n",
		    ajStrGetLen(hit1->Seq), ajStrGetLen(hit2->Seq) ,n);

	    if((ajStrFindS(hit1->Seq, hit2->Seq)!=-1) ||
	       (ajStrFindS(hit2->Seq, hit1->Seq)!=-1))
		return ajTrue;

	    ajDebug("No string overlap in sequences\n");
	    return ajFalse;
	}

    ajDebug("Test range hit1 %u..%u hit2 %u..%u\n",
	    hit1->Start, hit1->End,
	    hit2->Start, hit2->End);

    if(hit1->End > hit2->Start)
	len1 = hit1->End - hit2->Start + 1;
    if(hit2->End > hit1->Start)
	len2 = hit2->End - hit2->Start + 1;

    if( ((len1>=n) && 
	 (hit2->Start >= hit1->Start)) ||
       ((len2>=n) && 
	(hit1->Start >= hit2->Start)))
	return ajTrue;

    ajDebug("No overlap of at least %u\n",
	    hit1->Start, hit1->End,
	    hit2->Start, hit2->End);
	    
     return ajFalse;
}




/* ======================================================================= */
/* ========================== Input & Output ============================= */
/* ======================================================================= */

/* @section Input & output **************************************************
**
** These functions are used for formatted input and output to file.    
**
****************************************************************************/

/* @func embHitReadFasta ****************************************************
**
** Read a hit object from a file in extended FASTA format 
** (see documentation for the DOMAINATRIX "seqsearch" application). 
** 
** @param [u] inf      [AjPFile] Input file stream
**
** @return [EmbPHit] Hit object, or NULL if the file was not in extended 
** FASTA (DHF) format (indicated by a token count of the the lines 
** beginning with '>').
** @@
****************************************************************************/

EmbPHit embHitReadFasta(AjPFile inf) 
{
    EmbPHit    hit       = NULL;    /* Current hit */
    AjBool    donefirst = ajFalse; /* First '>' line has been read */
    ajint     ntok      = 0;       /* No. tokens in a line */
    const AjPStr token  = NULL;
    AjPStr    line      = NULL;    /* Line of text */
    AjPStr    subline   = NULL;


    /* Allocate strings */
    line     = ajStrNew();
    subline  = ajStrNew();


    while((ajFileReadLine(inf,&line)))
    {
	if(ajStrPrefixC(line,">"))
	{
	    /* Process the last hit */
	    if(donefirst)
	    {
		ajStrRemoveWhite(&hit->Seq);
		ajStrDel(&line);
		ajStrDel(&subline);
		return hit;
	    }	
/*	    else
		hit = embHitNew(); */

	    /* Check line has correct no. of tokens and allocate Hit */
	    ajStrAssignSubS(&subline, line, 1, -1);
	    if( (ntok=ajStrParseCountC(subline, "^")) != 17)
	    {
		ajWarn("Wrong no. (%d) of tokens for a DHF file on line %S\n",
		       ntok, line);
		ajStrDel(&line);
		ajStrDel(&subline);
		embHitDel(&hit);
		return NULL;
	    }
	    else
	    {
		hit = embHitNew();
	    }
	    	    
	    /* Acc */
	    token = ajStrParseC(subline, "^");
	    ajStrAssignS(&hit->Acc, token);
	    ajStrTrimWhite(&hit->Acc); 
	    if(ajStrMatchC(hit->Acc, "."))
		ajStrSetClear(&hit->Acc);

	    /* Spr */
	    token = ajStrParseC(NULL, "^");
	    ajStrAssignS(&hit->Spr, token);
	    if(ajStrMatchC(hit->Spr, "."))
		ajStrSetClear(&hit->Spr);

	    /* Start */
	    token = ajStrParseC(NULL, "^");
	    ajFmtScanS(token, "%u", &hit->Start);

	    /* End */
	    token = ajStrParseC(NULL, "^");
	    ajFmtScanS(token, "%u", &hit->End);
	    
	    /* Disregard domain type */
	    token = ajStrParseC(NULL, "^");

	    /* Dom */
	    token = ajStrParseC(NULL, "^");
	    ajStrAssignS(&hit->Dom, token);
	    if(ajStrMatchC(hit->Dom, "."))
		ajStrSetClear(&hit->Dom);

	    /* Disregard domain identifier - a change of domain identifier indicates 
	       a new block of hits in a file with multiple hitlists, but we only 
	       want a single hit so don't need this. */
	    token = ajStrParseC(NULL, "^");
	    token = ajStrParseC(NULL, "^");
	    token = ajStrParseC(NULL, "^");
	    token = ajStrParseC(NULL, "^");
	    token = ajStrParseC(NULL, "^");
	    token = ajStrParseC(NULL, "^");
	    token = ajStrParseC(NULL, "^");

	    token = ajStrParseC(NULL, "^");
	    ajStrAssignS(&hit->Model, token);
	    if(ajStrMatchC(hit->Model, "."))
		ajStrSetClear(&hit->Model);

	    token = ajStrParseC(NULL, "^");
	    ajFmtScanS(token, "%f", &hit->Score);
	    
	    token = ajStrParseC(NULL, "^");
	    ajFmtScanS(token, "%f", &hit->Pval);

	    token = ajStrParseC(NULL, "^");
	    ajFmtScanS(token, "%f", &hit->Eval);

	    donefirst = ajTrue;
	}
	else
	{
	    if(hit)
		ajStrAppendS(&hit->Seq, line);
	}
    }

    /* EOF therefore process last hit */
    if(donefirst)
    {
	ajStrRemoveWhite(&hit->Seq);
	ajStrDel(&line);
	ajStrDel(&subline);
	return hit;
    }
    

    /* Tidy up */
    ajStrDel(&line);
    ajStrDel(&subline);
    return NULL;
}




/* @func embHitlistRead *****************************************************
**
** Read a hitlist object from a file in embl-like format (see documentation
** for the DOMAINATRIX "seqsearch" application). 
** 
** @param [u] inf      [AjPFile] Input file stream
**
** @return [EmbPHitlist] Hitlist object
** @@
****************************************************************************/

EmbPHitlist embHitlistRead(AjPFile inf) 
{
    EmbPHitlist ret = NULL;

    AjPStr    type     = NULL;
    AjPStr line   = NULL;   /* Line of text */
    AjPStr class  = NULL;
    AjPStr arch   = NULL;
    AjPStr top    = NULL;
    AjPStr fold   = NULL;
    AjPStr super  = NULL;
    AjPStr family = NULL;
    AjBool   ok   = ajFalse;
    ajuint    n    = 0;              /* Number of current sequence */
    ajuint    nset = 0;              /* Number in set */
    ajuint  Sunid_Family = 0;        /* SCOP sunid for family */



    /* Allocate strings */
    class   = ajStrNew();
    arch    = ajStrNew();
    top     = ajStrNew();
    fold    = ajStrNew();
    super   = ajStrNew();
    family  = ajStrNew();
    line    = ajStrNew();
    type    = ajStrNew();
    

    
    /* Read first line */
    ok = ajFileReadLine(inf,&line);

    /* '//' is the delimiter for block of hits (in case the file 
       contains multiple hitlists). */

    while(ok && !ajStrPrefixC(line,"//"))
    {
	if(ajStrPrefixC(line,"XX"))
	{
	    ok = ajFileReadLine(inf,&line);
	    continue;
	}
	else if(ajStrPrefixC(line,"TY"))
	{
	    ajFmtScanS(line, "%*s %S", &type);
	}
	else if(ajStrPrefixC(line,"SI"))
	{
	    ajFmtScanS(line, "%*s %u", &Sunid_Family);
	}
	else if(ajStrPrefixC(line,"CL"))
	{
	    ajStrAssignC(&class,ajStrGetPtr(line)+3);
	    ajStrRemoveWhiteExcess(&class);
	}
	else if(ajStrPrefixC(line,"AR"))
	{
	    ajStrAssignC(&arch,ajStrGetPtr(line)+3);
	    ajStrRemoveWhiteExcess(&arch);
	}
	else if(ajStrPrefixC(line,"TP"))
	{
	    ajStrAssignC(&top,ajStrGetPtr(line)+3);
	    ajStrRemoveWhiteExcess(&top);
	}
	else if(ajStrPrefixC(line,"FO"))
	{
	    ajStrAssignC(&fold,ajStrGetPtr(line)+3);
	    while((ok = ajFileReadLine(inf,&line)))
	    {
		if(ajStrPrefixC(line,"XX"))
		    break;
		ajStrAppendC(&fold,ajStrGetPtr(line)+3);
	    }
	    ajStrRemoveWhiteExcess(&fold);
	}
	else if(ajStrPrefixC(line,"SF"))
	{
	    ajStrAssignC(&super,ajStrGetPtr(line)+3);
	    while((ok = ajFileReadLine(inf,&line)))
	    {
		if(ajStrPrefixC(line,"XX"))
		    break;
		ajStrAppendC(&super,ajStrGetPtr(line)+3);
	    }
	    ajStrRemoveWhiteExcess(&super);
	}
	else if(ajStrPrefixC(line,"FA"))
	{
	    ajStrAssignC(&family,ajStrGetPtr(line)+3);
	    while((ok = ajFileReadLine(inf,&line)))
	    {
		if(ajStrPrefixC(line,"XX"))
		    break;
		ajStrAppendC(&family,ajStrGetPtr(line)+3);
	    }
	    ajStrRemoveWhiteExcess(&family);
	}
	else if(ajStrPrefixC(line,"NS"))
	{
	    ajFmtScanS(line, "NS %u", &nset);


	    /* Create hitlist structure */
	    (ret)=embHitlistNew(nset);
	    (ret)->N=nset;
	    ajStrAssignS(&(ret)->Class, class);
	    ajStrAssignS(&(ret)->Architecture, arch);
	    ajStrAssignS(&(ret)->Topology, top);
	    ajStrAssignS(&(ret)->Fold, fold);
	    ajStrAssignS(&(ret)->Superfamily, super);
	    ajStrAssignS(&(ret)->Family, family);
	    (ret)->Sunid_Family = Sunid_Family;
	    if(ajStrMatchC(type, "SCOP"))
		(ret)->Type = ajSCOP;
	    else if(ajStrMatchC(type, "CATH"))
		(ret)->Type = ajCATH;
	}
	else if(ajStrPrefixC(line,"NN"))
	{
	    /* Increment hit counter */
	    n++;
	    
	    /* Safety check */
	    if(n>nset)
		ajFatal("Dangerous error in input file caught "
			"in embHitlistRead.\n Email jison@hgmp.mrc.ac.uk");
	}
	else if(ajStrPrefixC(line,"SC"))
	{
	    ajFmtScanS(line, "%*s %f", &(ret)->hits[n-1]->Score);
	}
	else if(ajStrPrefixC(line,"PV"))
	{
	    ajFmtScanS(line, "%*s %f", &(ret)->hits[n-1]->Pval);
	}
	else if(ajStrPrefixC(line,"EV"))
	{
	    ajFmtScanS(line, "%*s %f", &(ret)->hits[n-1]->Eval);
	}
	else if(ajStrPrefixC(line,"AC"))
	{
	    ajStrAssignC(&(ret)->hits[n-1]->Acc,ajStrGetPtr(line)+3);
	    ajStrRemoveWhiteExcess(&(ret)->hits[n-1]->Acc);
	}
	else if(ajStrPrefixC(line,"SP"))
	{
	    ajStrAssignC(&(ret)->hits[n-1]->Spr,ajStrGetPtr(line)+3);
	    ajStrRemoveWhiteExcess(&(ret)->hits[n-1]->Spr);
	}
	else if(ajStrPrefixC(line,"DO"))
	{
	    ajStrAssignC(&(ret)->hits[n-1]->Dom,ajStrGetPtr(line)+3);
	    ajStrRemoveWhiteExcess(&(ret)->hits[n-1]->Dom);
	}
/*	else if(ajStrPrefixC(line,"TY"))
	{
	    ajStrAssignC(&(ret)->hits[n-1]->Typeobj,ajStrGetPtr(line)+3);	
	    ajStrRemoveWhiteExcess(&(ret)->hits[n-1]->Typeobj);		
	}  */
	else if(ajStrPrefixC(line,"MO"))
	{
	    ajStrAssignC(&(ret)->hits[n-1]->Model,ajStrGetPtr(line)+3);	
	    ajStrRemoveWhiteExcess(&(ret)->hits[n-1]->Model);		
	}
	else if(ajStrPrefixC(line,"RA"))
	    ajFmtScanS(line, "%*s %u %*s %u", &(ret)->hits[n-1]->Start,
		       &(ret)->hits[n-1]->End);
	else if(ajStrPrefixC(line,"GP"))
	    ajFmtScanS(line, "%*s %S", &(ret)->hits[n-1]->Group);
	else if(ajStrPrefixC(line,"SQ"))
	{
	    while((ok=ajFileReadLine(inf,&line)) && !ajStrPrefixC(line,"XX"))
		ajStrAppendC(&(ret)->hits[n-1]->Seq,ajStrGetPtr(line));
	    ajStrRemoveWhite(&(ret)->hits[n-1]->Seq);
	    continue;
	}
	
	ok = ajFileReadLine(inf,&line);
    }


    ajStrDel(&line);
    ajStrDel(&class);
    ajStrDel(&arch);
    ajStrDel(&top);
    ajStrDel(&fold);
    ajStrDel(&super);
    ajStrDel(&family);
    ajStrDel(&type);
    
    return ret;
}





/* @func embHitlistReadFasta ************************************************
**
** Read a hitlist object from a file in extended FASTA format 
** (see documentation for the DOMAINATRIX "seqsearch" application). 
** 
** @param [u] inf      [AjPFile] Input file stream
**
** @return [EmbPHitlist] Hitlist object
** @@
****************************************************************************/

EmbPHitlist embHitlistReadFasta(AjPFile inf) 
{
    EmbPHitlist hitlist   = NULL;
    EmbPHit     hit       = NULL;    /* Current hit.                     */
    AjPList    tmplist   = NULL;    /* Temp. list of hits               */       
    AjBool     donefirst = ajFalse; /* Read first code line.            */
    AjBool     doneseq   = ajFalse; /* Read at least one sequence line. */
    ajuint     this_id    = 0;       /* Domain id of last hit.           */
    ajuint     last_id    = 0;       /* Domain id of this hit.           */
    ajint     ntok       = 0;       /* No. tokens in a line.            */
    const AjPStr token   = NULL;
    AjPStr    line       = NULL;    /* Line of text.                    */
    AjPStr    subline    = NULL;
    AjBool    ok         = ajFalse;
    AjBool    parseok    = ajFalse;
    AjPStr    type       = NULL;
    
    ajlong    fpos       = 0;    
    ajlong    fpos_noseq = 0;    


    /* Allocate strings */
    line     = ajStrNew();
    subline  = ajStrNew();
    tmplist  = ajListNew();
    type     = ajStrNew();
    

    while((ok = ajFileReadLine(inf,&line)))
    {
	if(ajStrPrefixC(line,">"))
	{
	    /* This line added so that it can process files with no sequence 
	       info. correctly */
	    fpos_noseq = ajFileTell(inf); 


	    /* Process the last hit */
	    if(donefirst)
	    {
		if(MAJSTRGETLEN(hit->Seq))
		    ajStrRemoveWhite(&hit->Seq);
		ajListPushAppend(tmplist, hit);
	    }
	    
	    /* Check line has correct no. of tokens and allocate Hit */
	    ajStrAssignSubS(&subline, line, 1, -1);
	    if( (ntok=ajStrParseCountC(subline, "^")) != 17)
		ajFatal("Incorrect no. (%d) of tokens on line %S\n",
			ntok, line);
	    else
	    {
		parseok = ajTrue;
		hit     = embHitNew();
	    }
	    
	    /* Acc */
	    token = ajStrParseC(subline, "^");
	    ajStrAssignS(&hit->Acc, token);
	    ajStrTrimWhite(&hit->Acc); 
	    if(ajStrMatchC(hit->Acc, "."))
		ajStrSetClear(&hit->Acc);

	    /* Spr */
	    token = ajStrParseC(NULL, "^");
	    ajStrAssignS(&hit->Spr, token);
	    if(ajStrMatchC(hit->Spr, "."))
		ajStrSetClear(&hit->Spr);

	    /* Start */
	    token = ajStrParseC(NULL, "^");
	    ajFmtScanS(token, "%u", &hit->Start);

	    /* End */
	    token = ajStrParseC(NULL, "^");
	    ajFmtScanS(token, "%u", &hit->End);
	    
	    /* Type */
	    token = ajStrParseC(NULL, "^");
	    ajStrAssignS(&type, token);

	    /* Dom */
	    token = ajStrParseC(NULL, "^");
	    ajStrAssignS(&hit->Dom, token);
	    if(ajStrMatchC(hit->Dom, "."))
		ajStrSetClear(&hit->Dom);

	    /* Read domain identifier - a change of domain identifier indicates 
	       a new block of hits in a file with multiple hitlists. */
	    token = ajStrParseC(NULL, "^");
	    ajFmtScanS(token, "%u", &this_id);


	    /* Domain identifier differs therefore we have come to the end of 
	       the list */
	    if((this_id != last_id) && (donefirst))
	    {
		/* Delete the hit we've just read in ... which is of the wrong family */
		embHitDel(&hit);
		
		hitlist->N = ajListToarray(tmplist, (void ***)&hitlist->hits);
		ajStrDel(&line);
		ajStrDel(&subline);
		ajStrDel(&type);
		ajListFree(&tmplist);

		if(doneseq)
		    ajFileSeek(inf, fpos, 0);
		else
		    ajFileSeek(inf, fpos_noseq, 0);
		return hitlist;
	    }
	    else
	    {
		if((!donefirst))
		{
		    hitlist = embHitlistNew(0);
		    hitlist->Sunid_Family = this_id;
		    if(ajStrMatchC(type, "SCOP"))
			hitlist->Type = ajSCOP;
		    else if(ajStrMatchC(type, "CATH"))
			hitlist->Type = ajCATH;
		}	
		last_id = this_id;
	    }	    
	    if(donefirst)
	    {
		token = ajStrParseC(NULL, "^");
		token = ajStrParseC(NULL, "^");
		token = ajStrParseC(NULL, "^");
		token = ajStrParseC(NULL, "^");
		token = ajStrParseC(NULL, "^");
		token = ajStrParseC(NULL, "^");
	    }
	    else 
	    {
		token = ajStrParseC(NULL, "^");
		ajStrAssignS(&hitlist->Class, token);
		if(ajStrMatchC(hitlist->Class, "."))
		    ajStrSetClear(&hitlist->Class);	

		token = ajStrParseC(NULL, "^");
		ajStrAssignS(&hitlist->Architecture, token);
		if(ajStrMatchC(hitlist->Architecture, "."))
		    ajStrSetClear(&hitlist->Architecture);

		token = ajStrParseC(NULL, "^");
		ajStrAssignS(&hitlist->Topology, token);
		if(ajStrMatchC(hitlist->Topology, "."))
		    ajStrSetClear(&hitlist->Topology);

		token = ajStrParseC(NULL, "^");
		ajStrAssignS(&hitlist->Fold, token);
		if(ajStrMatchC(hitlist->Fold, "."))
		    ajStrSetClear(&hitlist->Fold);

		token = ajStrParseC(NULL, "^");
		ajStrAssignS(&hitlist->Superfamily, token);
		if(ajStrMatchC(hitlist->Superfamily, "."))
		    ajStrSetClear(&hitlist->Superfamily);

		token = ajStrParseC(NULL, "^");
		ajStrAssignS(&hitlist->Family, token);
		if(ajStrMatchC(hitlist->Family, "."))
		    ajStrSetClear(&hitlist->Family);
	    }

	    token = ajStrParseC(NULL, "^");
	    ajStrAssignS(&hit->Model, token);
	    if(ajStrMatchC(hit->Model, "."))
		ajStrSetClear(&hit->Model);

	    token = ajStrParseC(NULL, "^");
	    ajFmtScanS(token, "%f", &hit->Score);
	    
	    token = ajStrParseC(NULL, "^");
	    ajFmtScanS(token, "%f", &hit->Pval);

	    token = ajStrParseC(NULL, "^");
	    ajFmtScanS(token, "%f", &hit->Eval);

	    donefirst = ajTrue;

	}
	else
	{
	    ajStrAppendS(&hit->Seq, line);
	    doneseq=ajTrue;
	    fpos = ajFileTell(inf);
	}
    }

    /* EOF therefore process last hit */
    if((!ok) && (parseok))
    {
	ajStrRemoveWhite(&hit->Seq);
	ajListPushAppend(tmplist, hit);
	hitlist->N = ajListToarray(tmplist, (void ***)&hitlist->hits);
	ajStrDel(&subline);
	ajStrDel(&line);
	ajStrDel(&type);
	ajListFree(&tmplist);
	return hitlist;
    }


    /* Tidy up */
    ajStrDel(&line);
    ajStrDel(&subline);
    ajStrDel(&type);
    ajListFree(&tmplist);
    
    
    /* File read error */
    return NULL;
}






/* @func embHitlistReadNode *************************************************
**
** Reads a scop families file (see documentation for the EMBASSY 
** DOMAINATRIX package) and writes a list of Hitlist objects containing 
** all domains matching the scop classification provided.
**
** @param [u] inf    [AjPFile]   File containing multiple Hitlist objects
** @param [r] fam    [const AjPStr]    Family.
** @param [r] sfam   [const AjPStr]    Superfamily.
** @param [r] fold   [const AjPStr]    Fold.
** @param [r] klass  [const AjPStr]    Class.
** 
** @return [AjPList] List of Hitlist objects or NULL.
** @@
****************************************************************************/

AjPList embHitlistReadNode(AjPFile inf,
			   const AjPStr fam, const AjPStr sfam, 
			   const AjPStr fold, const AjPStr klass)
{
    AjPList ret = NULL;
    const AjPStr class   = NULL;

    class = klass;

    if(!inf)
	ajFatal("NULL arg passed to embHitlistReadNode");

    /* Allocate the list if it does not already exist */

    (ret) = ajListNew();

    
    /*
    ** if family is specified then the other fields
    ** also have to be specified.
    */
    if(fam)
    {
	if(!sfam || !fold || !class)
	{
	    ajWarn("Bad arguments passed to embHitlistReadNode\n");
		ajListFree(&(ret));
	    return NULL;
	}
	else
	{
	    if((embHitlistReadFam(inf,fam,sfam,fold,class,&ret)))
		return ret;
	    else
	    {
		    ajListFree(&(ret));
		return NULL;
	    }
	}
    }
    /*
    ** if superfamily is specified then the other fields
    ** also have to be specified.
    */
    else if(sfam)
    {
	if(!fold || !class)
	{
	    ajWarn("Bad arguments passed to embHitlistReadNode\n");
		ajListFree(&(ret));
	    return NULL;
	}
	else
	{
	    if((embHitlistReadSfam(inf,fam,sfam,fold,class,&ret)))
		return ret;
	    else
	    {
		    ajListFree(&(ret));
		return NULL;
	    }
	}	   
    }
    /*
    ** if fold is specified then the other fields also have
    ** to be specified.
    */
    else if(fold)
    {
	if(!class)
	{
	    ajWarn("Bad arguments passed to embHitlistReadNode\n");
		ajListFree(&(ret));
	    return NULL;
	}
	else
	{
	    if((embHitlistReadFold(inf,fam,fold,class,&ret)))
		return ret;
	    else
	    {
		    ajListFree(&(ret));
		return NULL;
	    }
	}
    } 

    ajWarn("Bad arguments passed to embHitlistReadNode\n");
	ajListFree(&(ret));

    return ret;
}




/* @func embHitlistReadNodeFasta ********************************************
**
** Reads a domain families file (see documentation for the EMBASSY 
** DOMAINATRIX package) and writes a list of Hitlist objects containing 
** all domains matching the domain classification provided.
**
** @param [u] inf    [AjPFile]   File containing multiple Hitlist objects
** @param [r] fam    [const AjPStr]    Family.
** @param [r] sfam   [const AjPStr]    Superfamily.
** @param [r] fold   [const AjPStr]    Fold.
** @param [r] klass  [const AjPStr]    Class.
** 
** @return [AjPList] List of Hitlist objects or NULL.
** @@
****************************************************************************/

AjPList embHitlistReadNodeFasta(AjPFile inf, 
				const AjPStr fam, 
				const AjPStr sfam, 
				const AjPStr fold, 
				const AjPStr klass)
{
    AjPList ret = NULL;
    const AjPStr class   = NULL;

    class = klass;

    if(!inf)
	ajFatal("NULL arg passed to embHitlistReadNodeFasta");

    /* Allocate the list if it does not already exist */

    (ret) = ajListNew();

    
    /*
    ** if family is specified then the other fields
    ** also have to be specified.
    */
    if(fam)
    {
	if(!sfam || !fold || !class)
	{
	    ajWarn("Bad arguments passed to embHitlistReadNodeFasta\n");
		ajListFree(&(ret));
	    return NULL;
	}
	else
	{
	    if((embHitlistReadFamFasta(inf,fam,sfam,fold,class,&ret)))
		return ret;
	    else
	    {
		    ajListFree(&(ret));
		return NULL;
	    }
	}
    }
    /*
    ** if superfamily is specified then the other fields
    ** also have to be specified.
    */
    else if(sfam)
    {
	if(!fold || !class)
	{
	    ajWarn("Bad arguments passed to embHitlistReadNodeFasta\n");
		ajListFree(&(ret));
	    return NULL;
	}
	else
	{
	    if((embHitlistReadSfamFasta(inf,fam,sfam,fold,class,&ret)))
		return ret;
	    else
	    {
		    ajListFree(&(ret));
		return NULL;
	    }
	}	   
    }
    /*
    ** if fold is specified then the other fields also have
    ** to be specified.
    */
    else if(fold)
    {
	if(!class)
	{
	    ajWarn("Bad arguments passed to embHitlistReadNodeFasta\n");
		ajListFree(&(ret));
	    return NULL;
	}
	else
	{
	    if((embHitlistReadFoldFasta(inf,fam,fold,class,&ret)))
		return ret;
	    else
	    {
		    ajListFree(&(ret));
		return NULL;
	    }
	}
    } 

    ajWarn("Bad arguments passed to embHitlistReadNodeFasta\n");
	ajListFree(&(ret));

    return ret;
}




/* @func embHitlistWrite ****************************************************
**
** Write contents of a Hitlist object to an output file in embl-like format
** (see documentation for the DOMAINATRIX "seqsearch" application).
** Text for Class, Fold, Superfamily and Family is only written if the text
** is available.
** 
** @param [u] outf [AjPFile] Output file stream
** @param [r] obj [const EmbPHitlist] Hitlist object
**
** @return [AjBool] True on success
** @@
****************************************************************************/

AjBool embHitlistWrite(AjPFile outf, const EmbPHitlist obj)
{
    ajuint x = 0;  /* Counter */
    AjPSeqout outseq;

    if(!obj)
	return ajFalse;


    if((obj->Type == ajSCOP))  
	ajFmtPrintF(outf, "TY   SCOP\nXX\n");
    else if ((obj->Type == ajCATH))
	ajFmtPrintF(outf, "TY   CATH\nXX\n");


    if(MAJSTRGETLEN(obj->Class))
	ajFmtPrintF(outf,"CL   %S\n",obj->Class);

    if(MAJSTRGETLEN(obj->Architecture))
	ajFmtPrintF(outf,"AR   %S\n",obj->Architecture);

    if(MAJSTRGETLEN(obj->Topology))
	ajFmtPrintF(outf,"TP   %S\n",obj->Topology);

    if(MAJSTRGETLEN(obj->Fold))
	ajFmtPrintSplit(outf,obj->Fold,"XX\nFO   ",75," \t\n\r");

    if(MAJSTRGETLEN(obj->Superfamily))
	ajFmtPrintSplit(outf,obj->Superfamily,"XX\nSF   ",75," \t\n\r");

    if(MAJSTRGETLEN(obj->Family))
	ajFmtPrintSplit(outf,obj->Family,"XX\nFA   ",75," \t\n\r");
    
    if( (MAJSTRGETLEN(obj->Class))       ||
       (MAJSTRGETLEN(obj->Architecture)) || 
	(MAJSTRGETLEN(obj->Topology))    ||
       (MAJSTRGETLEN(obj->Fold))         ||
	(MAJSTRGETLEN(obj->Superfamily)) ||
       (MAJSTRGETLEN(obj->Family)))
	ajFmtPrintF(outf,"XX\nSI   %u\n", obj->Sunid_Family);

    ajFmtPrintF(outf,"XX\nNS   %u\nXX\n",obj->N);

    for(x=0;x<obj->N;x++)
    {
	ajFmtPrintF(outf, "%-5s[%u]\nXX\n", "NN", x+1);
	if(MAJSTRGETLEN(obj->hits[x]->Model))
	{
	    ajFmtPrintF(outf, "%-5s%S\n", "MO", obj->hits[x]->Model);
	    ajFmtPrintF(outf, "XX\n");
	}
	
/*	if(MAJSTRGETLEN(obj->hits[x]->Typeobj))
	    ajFmtPrintF(outf, "%-5s%S\n", "TY", obj->hits[x]->Typeobj);
	ajFmtPrintF(outf, "XX\n"); */

	ajFmtPrintF(outf, "%-5s%.2f\n", "SC", obj->hits[x]->Score);
	ajFmtPrintF(outf, "XX\n");

	ajFmtPrintF(outf, "%-5s%.3e\n", "PV", obj->hits[x]->Pval);
	ajFmtPrintF(outf, "XX\n");

	ajFmtPrintF(outf, "%-5s%.3e\n", "EV", obj->hits[x]->Eval);
	ajFmtPrintF(outf, "XX\n");

	if(MAJSTRGETLEN(obj->hits[x]->Group))
	{
	    ajFmtPrintF(outf, "%-5s%S\n", "GP", obj->hits[x]->Group);
	    ajFmtPrintF(outf, "XX\n");
	}

	ajFmtPrintF(outf, "%-5s%S\n", "AC", obj->hits[x]->Acc);
	ajFmtPrintF(outf, "XX\n");

	if(MAJSTRGETLEN(obj->hits[x]->Spr))
	{
	    ajFmtPrintF(outf, "%-5s%S\n", "SP", obj->hits[x]->Spr);
	    ajFmtPrintF(outf, "XX\n");
	}
	
	if(MAJSTRGETLEN(obj->hits[x]->Dom))
	{
	    ajFmtPrintF(outf, "%-5s%S\n", "DO", obj->hits[x]->Dom);
	    ajFmtPrintF(outf, "XX\n");
	}
	
	ajFmtPrintF(outf, "%-5s%u START; %u END;\n", "RA",
		    obj->hits[x]->Start, obj->hits[x]->End);
	ajFmtPrintF(outf, "XX\n");
	outseq = ajSeqoutNewFile(outf);
	ajSeqoutDumpSwisslike(outseq, obj->hits[x]->Seq, "SQ");
	ajSeqoutDel(&outseq);
	ajFmtPrintF(outf, "XX\n");
    }
    ajFmtPrintF(outf, "//\n");

    return ajTrue;
}




/* @func embHitlistWriteSubset **********************************************
**
** Write contents of a Hitlist object to an output file in embl-like format
** (see documentation for the DOMAINATRIX "seqsearch" application).
** Only those hits are written for which a 1 is given in the corresponding
** position in array of integers.
** Text for Class, Architecture, Topology, Fold, Superfamily and Family is 
** only written if the text is available.
** 
** @param [u] outf  [AjPFile]    Output file stream
** @param [r] obj   [const EmbPHitlist] Hitlist object
** @param [r] ok    [const AjPUint]     Whether hits are to be printed or not
**
** @return [AjBool] True on success
** @@
****************************************************************************/

AjBool embHitlistWriteSubset(AjPFile outf, 
			     const EmbPHitlist obj, const AjPUint ok)
{
    ajuint x    = 0;  /* Counter */
    ajuint y    = 0;  /* Counter */
    ajuint nset = 0;  /* No. in set to be printed out */
    AjPSeqout outseq;

    if(!obj)
	return ajFalse;


    if((obj->Type == ajSCOP))  
	ajFmtPrintF(outf, "TY   SCOP\nXX\n");
    else if ((obj->Type == ajCATH))
	ajFmtPrintF(outf, "TY   CATH\nXX\n");

    if(MAJSTRGETLEN(obj->Class))
	ajFmtPrintF(outf,"CL   %S\n",obj->Class);

    if(MAJSTRGETLEN(obj->Architecture))
	ajFmtPrintF(outf,"AR   %S\n",obj->Architecture);

    if(MAJSTRGETLEN(obj->Topology))
	ajFmtPrintF(outf,"TP   %S\n",obj->Topology);

    if(MAJSTRGETLEN(obj->Fold))
	ajFmtPrintSplit(outf,obj->Fold,"XX\nFO   ",75," \t\n\r");

    if(MAJSTRGETLEN(obj->Superfamily))
	ajFmtPrintSplit(outf,obj->Superfamily,"XX\nSF   ",75," \t\n\r");

    if(MAJSTRGETLEN(obj->Family))
	ajFmtPrintSplit(outf,obj->Family,"XX\nFA   ",75," \t\n\r");

    if(MAJSTRGETLEN(obj->Family))
	ajFmtPrintF(outf,"XX\nSI   %u\n", obj->Sunid_Family);


    for(nset=0, x=0;x<obj->N;x++)
	if(ajUintGet(ok, x) == 1)
	    nset++;
	    
    ajFmtPrintF(outf,"XX\nNS   %u\nXX\n",nset);

    for(x=0;x<obj->N;x++)
    { 
	if(ajUintGet(ok, x) == 1)
	{
	    y++;

	    ajFmtPrintF(outf, "%-5s[%u]\nXX\n", "NN", y);
	    if(MAJSTRGETLEN(obj->hits[x]->Model))
	    {
		ajFmtPrintF(outf, "%-5s%S\n", "MO", obj->hits[x]->Model);
		ajFmtPrintF(outf, "XX\n");
	    }
	    
/*	    if(MAJSTRGETLEN(obj->hits[x]->Typeobj))
		ajFmtPrintF(outf, "%-5s%S\n", "TY", obj->hits[x]->Typeobj);
	    ajFmtPrintF(outf, "XX\n"); */

	    ajFmtPrintF(outf, "%-5s%.2f\n", "SC", obj->hits[x]->Score);
	    ajFmtPrintF(outf, "XX\n");

	    ajFmtPrintF(outf, "%-5s%.3e\n", "PV", obj->hits[x]->Pval);
	    ajFmtPrintF(outf, "XX\n");

	    ajFmtPrintF(outf, "%-5s%.3e\n", "EV", obj->hits[x]->Eval);
	    ajFmtPrintF(outf, "XX\n");

	    if(MAJSTRGETLEN(obj->hits[x]->Group))
	    {
		ajFmtPrintF(outf, "%-5s%S\n", "GP", obj->hits[x]->Group);
		ajFmtPrintF(outf, "XX\n");
	    }
	    
	    ajFmtPrintF(outf, "%-5s%S\n", "AC", obj->hits[x]->Acc);
	    ajFmtPrintF(outf, "XX\n");
	    if(MAJSTRGETLEN(obj->hits[x]->Spr))
	    {
		ajFmtPrintF(outf, "%-5s%S\n", "SP", obj->hits[x]->Spr);
		ajFmtPrintF(outf, "XX\n");
	    }

	    if(MAJSTRGETLEN(obj->hits[x]->Dom))
	    {
		ajFmtPrintF(outf, "%-5s%S\n", "DO", obj->hits[x]->Dom);
		ajFmtPrintF(outf, "XX\n");
	    }

	    ajFmtPrintF(outf, "%-5s%u START; %u END;\n", "RA",
			obj->hits[x]->Start, obj->hits[x]->End);
	    ajFmtPrintF(outf, "XX\n");
	    outseq = ajSeqoutNewFile(outf);
	    ajSeqoutDumpSwisslike(outseq, obj->hits[x]->Seq, "SQ");
	    ajSeqoutDel(&outseq);
	    ajFmtPrintF(outf, "XX\n");
	}
    }
    ajFmtPrintF(outf, "//\n");
	
    return ajTrue;
}




/* @func embHitlistWriteFasta ***********************************************
**
** Write contents of a Hitlist object to an output file in embl-like format
** (see documentation for the DOMAINATRIX "seqsearch" application).
** Text for Class, Fold, Superfamily and Family is only written if the text
** is available.
** 
** @param [u] outf [AjPFile] Output file stream
** @param [r] obj [const EmbPHitlist] Hitlist object
**
** @return [AjBool] True on success
** @@
****************************************************************************/

AjBool embHitlistWriteFasta(AjPFile outf, const  EmbPHitlist obj)
{
    ajuint x = 0;  /* Counter */
    
    if(!obj)
	return ajFalse;

    for(x=0;x<obj->N;x++)
    {
	ajFmtPrintF(outf, "> ");
	
	if(MAJSTRGETLEN(obj->hits[x]->Acc))
	    ajFmtPrintF(outf, "%S^", obj->hits[x]->Acc);
	else
	    ajFmtPrintF(outf, ".^");

	if(MAJSTRGETLEN(obj->hits[x]->Spr))
	    ajFmtPrintF(outf, "%S^", obj->hits[x]->Spr);
	else
	    ajFmtPrintF(outf, ".^");

	ajFmtPrintF(outf, "%u^%u^", obj->hits[x]->Start, obj->hits[x]->End);

	if((obj->Type == ajSCOP))  
	    ajFmtPrintF(outf, "SCOP^");
	else if ((obj->Type == ajCATH))
	    ajFmtPrintF(outf, "CATH^");
	else
	    ajFmtPrintF(outf, ".^");
	
	if(MAJSTRGETLEN(obj->hits[x]->Dom))
	    ajFmtPrintF(outf, "%S^", obj->hits[x]->Dom);
	else
	    ajFmtPrintF(outf, ".^");

	ajFmtPrintF(outf,"%u^", obj->Sunid_Family);

	if(MAJSTRGETLEN(obj->Class))
	    ajFmtPrintF(outf,"%S^",obj->Class);
	else
	    ajFmtPrintF(outf, ".^");

	if(MAJSTRGETLEN(obj->Architecture))
	    ajFmtPrintF(outf,"%S^",obj->Architecture);
	else
	    ajFmtPrintF(outf, ".^");

	if(MAJSTRGETLEN(obj->Topology))
	    ajFmtPrintF(outf,"%S^",obj->Topology);
	else
	    ajFmtPrintF(outf, ".^");

	if(MAJSTRGETLEN(obj->Fold))
	    ajFmtPrintF(outf,"%S^",obj->Fold);
	else
	    ajFmtPrintF(outf, ".^");

	if(MAJSTRGETLEN(obj->Superfamily))
	    ajFmtPrintF(outf,"%S^",obj->Superfamily);
	else
	    ajFmtPrintF(outf, ".^");

	if(MAJSTRGETLEN(obj->Family))
	    ajFmtPrintF(outf,"%S^",obj->Family);
	else
	    ajFmtPrintF(outf, ".^");

	if(MAJSTRGETLEN(obj->hits[x]->Model))
	    ajFmtPrintF(outf, "%S^", obj->hits[x]->Model);
	else
	    ajFmtPrintF(outf, ".^");

	ajFmtPrintF(outf, "%.2f^", obj->hits[x]->Score);

	ajFmtPrintF(outf, "%.3e^", obj->hits[x]->Pval);

	ajFmtPrintF(outf, "%.3e", obj->hits[x]->Eval);

	ajFmtPrintF(outf, "\n");
	ajFmtPrintF(outf, "%S\n", obj->hits[x]->Seq);
    }
    

    return ajTrue;
}





/* @func embHitlistWriteSubsetFasta *****************************************
**
** Write contents of a Hitlist object to an output file in embl-like format
** (see documentation for the DOMAINATRIX "seqsearch" application).
** Only those hits are written for which a 1 is given in the corresponding
** position in array of integers.
** Text for Class, Architecture, Topology, Fold, Superfamily and Family is 
** only written if the text is available.
** 
** @param [u] outf  [AjPFile]    Output file stream
** @param [r] obj   [const EmbPHitlist] Hitlist object
** @param [r] ok    [const AjPUint]     Whether hits are to be printed or not
**
** @return [AjBool] True on success
** @@
****************************************************************************/

AjBool embHitlistWriteSubsetFasta(AjPFile outf, 
				  const EmbPHitlist obj, const AjPUint ok)
{
    ajuint x    = 0;  /* Counter */

    if(!obj)
	return ajFalse;

    for(x=0;x<obj->N;x++)
    { 
	if(ajUintGet(ok, x) == 1)
	{
	    ajFmtPrintF(outf, "> ");
	
	    if(MAJSTRGETLEN(obj->hits[x]->Acc))
		ajFmtPrintF(outf, "%S^", obj->hits[x]->Acc);
	    else
		ajFmtPrintF(outf, ".^");
	    if(MAJSTRGETLEN(obj->hits[x]->Spr))
		ajFmtPrintF(outf, "%S^", obj->hits[x]->Spr);
	    else
		ajFmtPrintF(outf, ".^");
	    ajFmtPrintF(outf, "%u^%u^",
			obj->hits[x]->Start, obj->hits[x]->End);

	    if((obj->Type == ajSCOP))  
		ajFmtPrintF(outf, "SCOP^");
	    else if ((obj->Type == ajCATH))
		ajFmtPrintF(outf, "CATH^");
	    else
		ajFmtPrintF(outf, ".^");

	    if(MAJSTRGETLEN(obj->hits[x]->Dom))
		ajFmtPrintF(outf, "%S^", obj->hits[x]->Dom);
	    else
		ajFmtPrintF(outf, ".^");

	    ajFmtPrintF(outf,"%u^", obj->Sunid_Family);
	    if(MAJSTRGETLEN(obj->Class))
		ajFmtPrintF(outf,"%S^",obj->Class);
	    else
		ajFmtPrintF(outf, ".^");
	    if(MAJSTRGETLEN(obj->Architecture))
		ajFmtPrintF(outf,"%S^",obj->Architecture);
	    else
		ajFmtPrintF(outf, ".^");
	    if(MAJSTRGETLEN(obj->Topology))
		ajFmtPrintF(outf,"%S^",obj->Topology);
	    else
		ajFmtPrintF(outf, ".^");
	    if(MAJSTRGETLEN(obj->Fold))
		ajFmtPrintF(outf,"%S^",obj->Fold);
	    else
		ajFmtPrintF(outf, ".^");
	    if(MAJSTRGETLEN(obj->Superfamily))
		ajFmtPrintF(outf,"%S^",obj->Superfamily);
	    else
		ajFmtPrintF(outf, ".^");
	    if(MAJSTRGETLEN(obj->Family))
		ajFmtPrintF(outf,"%S^",obj->Family);
	    else
		ajFmtPrintF(outf, ".^");
	    if(MAJSTRGETLEN(obj->Model))
		ajFmtPrintF(outf, "%S^", obj->Model);
	    else
		ajFmtPrintF(outf, ".^");
	    ajFmtPrintF(outf, "%.2f^", obj->hits[x]->Score);
	    ajFmtPrintF(outf, "%.3e^", obj->hits[x]->Pval);
	    ajFmtPrintF(outf, "%.3e", obj->hits[x]->Eval);

	    ajFmtPrintF(outf, "\n");
	    ajFmtPrintF(outf, "%S\n", obj->hits[x]->Seq);
	}
    }
    
    return ajTrue;
}






/* @func embHitlistWriteHitFasta **********************************************
**
** Write contents of one Hitlist object to an output file in embl-like format
** (see documentation for the DOMAINATRIX "seqsearch" application).
** Text for Class, Fold, Superfamily and Family is only written if the text
** is available.
** 
** @param [u] outf [AjPFile] Output file stream
** @param [r] n [ajuint] Number of hit
** @param [r] obj [const EmbPHitlist] Hitlist object
**
** @return [AjBool] True on success
** @@
****************************************************************************/

AjBool        embHitlistWriteHitFasta(AjPFile outf, 
				      ajuint n, 
				      const EmbPHitlist obj)
{
    if(!obj)
	return ajFalse;
    if(n >= obj->N)
	return ajFalse;
    
    ajFmtPrintF(outf, "> ");
    
    if(MAJSTRGETLEN(obj->hits[n]->Acc))
	ajFmtPrintF(outf, "%S^", obj->hits[n]->Acc);
    else
	ajFmtPrintF(outf, ".^");
    
    if(MAJSTRGETLEN(obj->hits[n]->Spr))
	ajFmtPrintF(outf, "%S^", obj->hits[n]->Spr);
    else
	ajFmtPrintF(outf, ".^");
    
    ajFmtPrintF(outf, "%u^%u^", obj->hits[n]->Start, obj->hits[n]->End);
    
    if((obj->Type == ajSCOP))  
	ajFmtPrintF(outf, "SCOP^");
    else if ((obj->Type == ajCATH))
	ajFmtPrintF(outf, "CATH^");
    else
	ajFmtPrintF(outf, ".^");
    
    if(MAJSTRGETLEN(obj->hits[n]->Dom))
	ajFmtPrintF(outf, "%S^", obj->hits[n]->Dom);
    else
	ajFmtPrintF(outf, ".^");
    
    ajFmtPrintF(outf,"%u^", obj->Sunid_Family);
    
    if(MAJSTRGETLEN(obj->Class))
	ajFmtPrintF(outf,"%S^",obj->Class);
    else
	ajFmtPrintF(outf, ".^");
    
    if(MAJSTRGETLEN(obj->Architecture))
	ajFmtPrintF(outf,"%S^",obj->Architecture);
    else
	ajFmtPrintF(outf, ".^");
    
    if(MAJSTRGETLEN(obj->Topology))
	ajFmtPrintF(outf,"%S^",obj->Topology);
    else
	ajFmtPrintF(outf, ".^");
    
    if(MAJSTRGETLEN(obj->Fold))
	ajFmtPrintF(outf,"%S^",obj->Fold);
    else
	ajFmtPrintF(outf, ".^");
    
    if(MAJSTRGETLEN(obj->Superfamily))
	ajFmtPrintF(outf,"%S^",obj->Superfamily);
    else
	ajFmtPrintF(outf, ".^");
    
    if(MAJSTRGETLEN(obj->Family))
	ajFmtPrintF(outf,"%S^",obj->Family);
    else
	ajFmtPrintF(outf, ".^");    

    if(MAJSTRGETLEN(obj->hits[n]->Model))
	ajFmtPrintF(outf, "%S^", obj->hits[n]->Model);
    else
	ajFmtPrintF(outf, ".^");
    
    ajFmtPrintF(outf, "%.2f^", obj->hits[n]->Score);
    
    ajFmtPrintF(outf, "%.3e^", obj->hits[n]->Pval);
    
    ajFmtPrintF(outf, "%.3e", obj->hits[n]->Eval);
    
    ajFmtPrintF(outf, "\n");
    ajFmtPrintF(outf, "%S\n", obj->hits[n]->Seq);

    return ajTrue;
}





/* @func embSignatureReadNew ************************************************
**
** Read a Signature object from a file in embl-like format (see documentation
** for the DOMAINATRIX "sigscan" application).
**
** @param [u] inf [AjPFile] Input file stream
**
** @return [EmbPSignature] Signature object
** @@
****************************************************************************/

EmbPSignature embSignatureReadNew(AjPFile inf)
{
    EmbPSignature ret = NULL;
    
    static AjPStr type   = NULL;
    static AjPStr typesig= NULL;
    static AjPStr line   = NULL;
    static AjPStr class  = NULL;
    static AjPStr arch   = NULL;
    static AjPStr top    = NULL;
    static AjPStr fold   = NULL;
    static AjPStr super  = NULL;
    static AjPStr family = NULL;
    ajuint  Sunid_Family;        /* SCOP sunid for family */

    static AjPStr id     = NULL;
    static AjPStr domid  = NULL;
    static AjPStr ligid  = NULL;
    static AjPStr desc   = NULL;

    AjBool ok   = ajFalse;
    ajuint  npos = 0;   /* No. signature positions */
    ajuint  i    = 0;   /* Loop counter */
    ajuint  n    = 0;   /* Counter of signature positions */
    ajuint  nres = 0;   /* No. residues for a sig. position */
    ajuint  ngap = 0;   /* No. gaps for a sig. position */
    ajuint  wsiz = 0;   /* Windows size for a sig. position */
    ajuint  v1   = 0;
    ajuint  v2   = 0;
    char   c1   = '\0';
    static AjPStr env  = NULL;
    const AjPStr token = NULL;   /* For parsing      */
    
    /* Signature of type ajLIGAND only */
    ajuint     ns;
    ajuint     sn;
    ajuint     np;
    ajuint     pn;
    ajuint     minpatch;
    ajuint     maxgap;
    
    /* CHECK ARG'S */
    if(!inf)
	return NULL;
    

    /* Only initialise strings if this is called for the first time */
    if(!line)
    {
	class   = ajStrNew();
	arch    = ajStrNew();
	top     = ajStrNew();
	fold    = ajStrNew();
	super   = ajStrNew();
	family  = ajStrNew();
	line    = ajStrNew();
	id      = ajStrNew();
	domid   = ajStrNew();
	ligid   = ajStrNew();
	desc    = ajStrNew();
	type    = ajStrNew();
	typesig = ajStrNew();
	env     = ajStrNew();
    }


    /* Read first line */
    ok=ajFileReadLine(inf,&line);

    while(ok && !ajStrPrefixC(line,"//"))
    {
      /* Records for signatures of type ajLIGAND only */
      /* IS */
      	if(ajStrPrefixC(line,"IS"))
	{
	    token = ajStrParseC(line, ";");
	    ajFmtScanS(line, "%*s %*s %u", &sn);

	    token = ajStrParseC(NULL, ";");
	    ajFmtScanS(token, "%*s %u", &ns);
	  }

	/* IP */
      	else if(ajStrPrefixC(line,"IP"))
	  {
	    token = ajStrParseC(line, ";");
	    ajFmtScanS(line, "%*s %*s %u", &pn);

	    token = ajStrParseC(NULL, ";");
	    ajFmtScanS(token, "%*s %u", &np);

	    token = ajStrParseC(NULL, ";");
	    ajFmtScanS(token, "%*s %u", &minpatch);

	    token = ajStrParseC(NULL, ";");
	    ajFmtScanS(token, "%*s %u", &maxgap);
	  }

	/* DE */
      	else if(ajStrPrefixC(line,"DE"))
	  {
	    ajFmtScanS(line, "%*s%S", &desc);
	  }

	/* ID */
      	else if(ajStrPrefixC(line,"ID"))
	  {
	    token = ajStrParseC(line, ";");
	    ajFmtScanS(line, "%*s %*s %S", &id);
	    token = ajStrParseC(NULL, ";");
	    ajFmtScanS(token, "%*s %S", &domid);
	    token = ajStrParseC(NULL, ";");
	    ajFmtScanS(token, "%*s %S", &ligid);
	  }



	if(ajStrPrefixC(line,"TY"))
	{
	    ajFmtScanS(line, "%*s %S", &type);
	}
	else if(ajStrPrefixC(line,"TS"))
	{
	    ajFmtScanS(line, "%*s %S", &typesig);
	}
	else if(ajStrPrefixC(line,"XX"))
	{
	    ok = ajFileReadLine(inf,&line);
	    continue;
	}
	else if(ajStrPrefixC(line,"SI"))
	{
	    ajFmtScanS(line, "%*s %u", &Sunid_Family);
	}
	else if(ajStrPrefixC(line,"CL"))
	{
	    ajStrAssignC(&class,ajStrGetPtr(line)+3);
	    ajStrRemoveWhiteExcess(&class);
	}
	else if(ajStrPrefixC(line,"AR"))
	{
	    ajStrAssignC(&arch,ajStrGetPtr(line)+3);
	    ajStrRemoveWhiteExcess(&arch);
	}
	else if(ajStrPrefixC(line,"TP"))
	{
	    ajStrAssignC(&top,ajStrGetPtr(line)+3);
	    ajStrRemoveWhiteExcess(&top);
	}
	else if(ajStrPrefixC(line,"FO"))
	{
	    ajStrAssignC(&fold,ajStrGetPtr(line)+3);
	    while((ok = ajFileReadLine(inf,&line)))
	    {
		if(ajStrPrefixC(line,"XX"))
		    break;
		ajStrAppendC(&fold,ajStrGetPtr(line)+3);
	    }
	    ajStrRemoveWhiteExcess(&fold);
	}
	else if(ajStrPrefixC(line,"SF"))
	{
	    ajStrAssignC(&super,ajStrGetPtr(line)+3);
	    while((ok = ajFileReadLine(inf,&line)))
	    {
		if(ajStrPrefixC(line,"XX"))
		    break;
		ajStrAppendC(&super,ajStrGetPtr(line)+3);
	    }
	    ajStrRemoveWhiteExcess(&super);
	}
	else if(ajStrPrefixC(line,"FA"))
	{
	    ajStrAssignC(&family,ajStrGetPtr(line)+3);
	    while((ok = ajFileReadLine(inf,&line)))
	    {
		if(ajStrPrefixC(line,"XX"))
		    break;
		ajStrAppendC(&family,ajStrGetPtr(line)+3);
	    }
	    ajStrRemoveWhiteExcess(&family);
	}
	else if(ajStrPrefixC(line,"NP"))
	{
	    ajFmtScanS(line, "NP %u", &npos);

	    /* Create signature structure */
	    (ret)=embSignatureNew(npos);

	    if(ajStrMatchC(type, "SCOP"))
		(ret)->Type = ajSCOP;
	    else if(ajStrMatchC(type, "CATH"))
		(ret)->Type = ajCATH;
	    else if(ajStrMatchC(type, "LIGAND"))
		(ret)->Type = ajLIGAND;

	    if(ajStrMatchC(typesig, "1D"))
		(ret)->Typesig = aj1D;
	    else if(ajStrMatchC(typesig, "3D"))
		(ret)->Typesig = aj3D;

	    ajStrAssignS(&(ret)->Class, class);
	    ajStrAssignS(&(ret)->Architecture, arch);
	    ajStrAssignS(&(ret)->Topology, top);
	    ajStrAssignS(&(ret)->Fold, fold);
	    ajStrAssignS(&(ret)->Superfamily, super);
	    ajStrAssignS(&(ret)->Family, family);
	    (ret)->Sunid_Family = Sunid_Family;	

	    /* ajLIGAND only */
	    ajStrAssignS(&(ret)->Id, id);
	    ajStrAssignS(&(ret)->Domid, domid);
	    ajStrAssignS(&(ret)->Ligid, ligid);
	    ajStrAssignS(&(ret)->Desc, desc);
	    ret->ns = ns;
	    ret->sn = sn;
	    ret->np = np;
	    ret->pn = pn;
	    ret->minpatch = minpatch;
	    ret->maxgap   = maxgap;

	}
	else if(ajStrPrefixC(line,"NN"))
	{
	    /* Increment position counter */
	    n++;

	    /* Safety check */
	    if(n>npos)
		ajFatal("Dangerous error in input file: "
			"n (%u) > npos (%u). Caught in "
			"embSignatureReadNew.\n"
			" Email jison@ebi.ac.uk", n, npos);
	}
	else if(ajStrPrefixC(line,"IN"))
	    {
		ajFmtScanS(line, "%*s %*s %u %*c %*s %u %*c %*s %u", 
			   &nres, &ngap, &wsiz);
	
		/* Create Sigdat structures and fill some elements */
		(ret)->dat[n-1]=embSigdatNew(nres, ngap);

		(ret)->dat[n-1]->wsiz=wsiz;

		/* Skip 'XX' line */
		if(!(ok = ajFileReadLine(inf,&line)))
		    break;

		/* Read in residue data */
		for(i=0; i<(ret)->dat[n-1]->nres; i++)
		{
		    if(!(ok = ajFileReadLine(inf,&line)))
			break;

		    if(ret->Typesig == aj1D)
		    {
			ajFmtScanS(line, "%*s %c %*c %u", &c1,&v2);
			ajChararrPut(&(ret)->dat[n-1]->rids,i,c1);
			ajUintPut(&(ret)->dat[n-1]->rfrq,i,v2);
		    }
		    else if(ret->Typesig == aj3D)
		    {
			ajFmtScanS(line, "%*s %S %*c %u", &env,&v2);
			ajStrAssignS(&(ret)->dat[n-1]->eids[i], env);
			ajUintPut(&(ret)->dat[n-1]->efrq,i,v2);
		    }
		    else
		    {
			/* This code block identical to above */
			ajWarn("Signature type (1D or 3D) not known in embSignatureWrite. Presuming 1D");
			ajFmtScanS(line, "%*s %c %*c %u", &c1,&v2);
			ajChararrPut(&(ret)->dat[n-1]->rids,i,c1);
			ajUintPut(&(ret)->dat[n-1]->rfrq,i,v2);
		    }
		}
		if(!ok)
		    break;
	       

		/* Skip 'XX' line */
		if(!(ok = ajFileReadLine(inf,&line)))
		    break;

		/* Read in gap data */
		for(i=0; i<(ret)->dat[n-1]->ngap; i++)
		{
		    if(!(ok = ajFileReadLine(inf,&line)))
			break;
		    ajFmtScanS(line, "%*s %u %*c %u", &v1,&v2);
		    ajUintPut(&(ret)->dat[n-1]->gsiz,i,v1);
		    ajUintPut(&(ret)->dat[n-1]->gfrq,i,v2);
		}
		if(!ok)
		    break;
	    }

	ok = ajFileReadLine(inf,&line);
    }
    
    if(!ok)
	return NULL;

    return ret;
}




/* @func embSignatureWrite **************************************************
**
** Write contents of a Signature object to an output file in embl-like 
** format (see documentation for the DOMAINATRIX "sigscan" application). 
**
** @param [w] outf [AjPFile]       Output file stream
** @param [r] obj  [const EmbPSignature]  Signature object
**
** @return [AjBool] ajTrue on success
** @@
****************************************************************************/
AjBool embSignatureWrite(AjPFile outf, const EmbPSignature obj)
{ 
    ajuint i;
    ajuint j;

    if(!outf || !obj)
	return ajFalse;

    if((obj->Type == ajSCOP))  
	ajFmtPrintF(outf, "TY   SCOP\nXX\n");
    else if ((obj->Type == ajCATH))
	ajFmtPrintF(outf, "TY   CATH\nXX\n");
    else if ((obj->Type == ajLIGAND))
	ajFmtPrintF(outf, "TY   LIGAND\nXX\n");


    if((obj->Typesig == aj1D))  
	ajFmtPrintF(outf, "TS   1D\nXX\n");
    else if ((obj->Typesig == aj3D))
	ajFmtPrintF(outf, "TS   3D\nXX\n");
    else
    {
	ajWarn("Signature type (1D or 3D) not known in embSignatureWrite. Presuming 1D");
	ajFmtPrintF(outf, "TS   1D\nXX\n");
    }
    
    if(MAJSTRGETLEN(obj->Class))
    {	ajFmtPrintF(outf,"CL   %S\n",obj->Class);
	ajFmtPrintF(outf, "XX\n");
    }
    if(MAJSTRGETLEN(obj->Architecture))
    {	ajFmtPrintF(outf,"AR   %S\n",obj->Architecture);
	ajFmtPrintF(outf, "XX\n");
    }
    if(MAJSTRGETLEN(obj->Topology))
    {	ajFmtPrintF(outf,"TP   %S\n",obj->Topology);
	ajFmtPrintF(outf, "XX\n");
    }
    if(MAJSTRGETLEN(obj->Fold))
    {
	ajFmtPrintSplit(outf,obj->Fold,"FO   ",75," \t\n\r");
	ajFmtPrintF(outf, "XX\n");
    }
    if(MAJSTRGETLEN(obj->Superfamily))
    {
	ajFmtPrintSplit(outf,obj->Superfamily,"SF   ",75," \t\n\r");
	ajFmtPrintF(outf, "XX\n");
    }
    if(MAJSTRGETLEN(obj->Family))
    {
	ajFmtPrintSplit(outf,obj->Family,"FA   ",75," \t\n\r");
	ajFmtPrintF(outf, "XX\n");
    }

    /*
    if(MAJSTRGETLEN(obj->Class))
	ajFmtPrintF(outf,"CL   %S",obj->Class);
    if(MAJSTRGETLEN(obj->Architecture))
	ajFmtPrintSplit(outf,obj->Architecture,"\nXX\nAR   ",75," \t\n\r");
    if(MAJSTRGETLEN(obj->Topology))
	ajFmtPrintSplit(outf,obj->Topology,"\nXX\nTP   ",75," \t\n\r");
    if(MAJSTRGETLEN(obj->Fold))
	ajFmtPrintSplit(outf,obj->Fold,"\nXX\nFO   ",75," \t\n\r");
    if(MAJSTRGETLEN(obj->Superfamily))
	ajFmtPrintSplit(outf,obj->Superfamily,"XX\nSF   ",75," \t\n\r");
    if(MAJSTRGETLEN(obj->Family))
	ajFmtPrintSplit(outf,obj->Family,"XX\nFA   ",75," \t\n\r");
	*/

    if(obj->Sunid_Family)
	ajFmtPrintF(outf,"SI   %u\nXX\n", obj->Sunid_Family);
	




    /* Signatures of type ajLIGAND only */
    if(obj->Type == ajLIGAND)
    {
      ajFmtPrintF(outf, "%-5sPDB %S; DOM %S; LIG %S;\n", 
		  "ID", 
		  obj->Id, 
		  obj->Domid,
		  obj->Ligid);
      ajFmtPrintF(outf, "XX\n"); 

      ajFmtPrintF(outf, "%-5s%S\n", 
		  "DE", 
		  obj->Desc);
      ajFmtPrintF(outf, "XX\n");   

      ajFmtPrintF(outf, "%-5sSN %u; NS %u\n", 
		  "IS", 
		  obj->sn, 
		  obj->ns);
      ajFmtPrintF(outf, "XX\n");   

      ajFmtPrintF(outf, "%-5sPN %u; NP %u; MP %u; MG %u\n", 
		  "IP", 
		  obj->pn, 
		  obj->np, 
		  obj->minpatch, 
		  obj->maxgap);
      ajFmtPrintF(outf, "XX\n");   
    }

    ajFmtPrintF(outf,"NP   %u\n",obj->npos);
    for(i=0;i<obj->npos;++i)
    {
	ajFmtPrintF(outf,"XX\nNN   [%u]\n",i+1);



	if(obj->Typesig == aj1D)
	{
	    ajFmtPrintF(outf,"XX\nIN   NRES %u ; NGAP %u ; WSIZ %u\nXX\n",
			obj->dat[i]->nres, obj->dat[i]->ngap,
			obj->dat[i]->wsiz);
	    for(j=0;j<obj->dat[i]->nres;++j)
		ajFmtPrintF(outf,"AA   %c ; %u\n",
			    (char)  ajChararrGet(obj->dat[i]->rids, j),
			    (ajint) ajUintGet(obj->dat[i]->rfrq, j));
	}
	else if (obj->Typesig == aj3D)
	{
	    ajFmtPrintF(outf,"XX\nIN   NRES %u ; NGAP %u ; WSIZ %u\nXX\n",
			obj->dat[i]->nenv, obj->dat[i]->ngap,
			obj->dat[i]->wsiz);
	    for(j=0;j<obj->dat[i]->nenv;++j)
	    {
		ajFmtPrintF(outf,"AA   %S ; %u\n",
			    obj->dat[i]->eids[j], 
			    (ajint) ajUintGet(obj->dat[i]->efrq, j));
	    }
	}
	else
	{
	    ajWarn("Type of signature (1D or 3D) unknown in embSignatureWrite."
		   " Presuming 1D.");
	    /* This code block identical to above */
	    ajFmtPrintF(outf,"XX\nIN   NRES %u ; NGAP %u ; WSIZ %u\nXX\n",
			obj->dat[i]->nres, obj->dat[i]->ngap,
			obj->dat[i]->wsiz);
	    for(j=0;j<obj->dat[i]->nres;++j)
		ajFmtPrintF(outf,"AA   %c ; %u\n",
			    (char)  ajChararrGet(obj->dat[i]->rids, j),
			    (ajint) ajUintGet(obj->dat[i]->rfrq, j));
	}
	

	ajFmtPrintF(outf,"XX\n");
	for(j=0;j<obj->dat[i]->ngap;++j)
	    ajFmtPrintF(outf,"GA   %u ; %u\n",
			(ajint) ajUintGet(obj->dat[i]->gsiz, j),
			(ajint) ajUintGet(obj->dat[i]->gfrq, j));
    }
    ajFmtPrintF(outf,"//\n");
    
    return ajTrue;
}







/* @func embSignatureHitsRead ***********************************************
**
** Reads a signature hits file, allocates a Hitlist object and writes it 
** with hits from a signature hits file (see documentation for the 
** DOMAINATRIX "sigscan" application). In other words, this function reads
** the results of a scan of a signature against a protein sequence database.  
**
** @param [u] inf  [AjPFile]      Input file stream
**
** @return [EmbPHitlist] Hitlist object that was allocated.
** @@
****************************************************************************/

EmbPHitlist embSignatureHitsRead(AjPFile inf)
{
    AjPList list        = NULL;
    EmbPHitlist ret      = NULL;
    ajuint  Sunid_Family = 0;
    AjBool ok           = ajTrue;
    EmbPHit tmphit       = NULL;
    

    AjPStr class  = NULL;
    AjPStr arch  = NULL;
    AjPStr top  = NULL;
    AjPStr fold   = NULL;
    AjPStr super  = NULL;
    AjPStr family = NULL;
    AjPStr line   = NULL;
    AjPStr type   = NULL;

    
    if(!inf)
    {
	ajWarn("NULL file pointer passed to embSignatureHitsRead");
	return NULL;
    }
    

    list   = ajListNew();
    class  = ajStrNew();
    arch   = ajStrNew();
    top    = ajStrNew();
    fold   = ajStrNew();
    super  = ajStrNew();
    family = ajStrNew();
    line   = ajStrNew();
    type    = ajStrNew();
    
    
    while(ok && ajFileReadLine(inf,&line))
    {
	if(ajStrPrefixC(line,"TY"))
	{
	    ajFmtScanS(line, "%*s %S", &type);
	}
	else if(ajStrPrefixC(line,"SI"))
	{
	    ajFmtScanS(line, "%*s %u", &Sunid_Family);
	}
	else if(ajStrPrefixC(line,"CL"))
	{
	    ajStrAssignC(&class,ajStrGetPtr(line)+3);
	    ajStrRemoveWhiteExcess(&class);
	}
	else if(ajStrPrefixC(line,"AR"))
	{
	    ajStrAssignC(&arch,ajStrGetPtr(line)+3);
	    ajStrRemoveWhiteExcess(&arch);
	}
	else if(ajStrPrefixC(line,"TP"))
	{
	    ajStrAssignC(&top,ajStrGetPtr(line)+3);
	    ajStrRemoveWhiteExcess(&top);
	}
	else if(ajStrPrefixC(line,"FO"))
	{
	    ajStrAssignC(&fold,ajStrGetPtr(line)+3);
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
	    ajStrAssignC(&super,ajStrGetPtr(line)+3);
	    while((ok = ajFileReadLine(inf,&line)))
	    {
		if(ajStrPrefixC(line,"XX"))
		    break;
		ajStrAppendC(&super,ajStrGetPtr(line)+3);
	    }
	    ajStrRemoveWhiteExcess(&super);
	}
	else if(ajStrPrefixC(line,"FA"))
	{
	    ajStrAssignC(&family,ajStrGetPtr(line)+3);
	    while((ok = ajFileReadLine(inf,&line)))
	    {
		if(ajStrPrefixC(line,"XX"))
		    break;
		ajStrAppendC(&family,ajStrGetPtr(line)+3);
	    }
	    ajStrRemoveWhiteExcess(&family);
	}
	else if(ajStrPrefixC(line,"HI"))
	{
	    tmphit=embHitNew();

	    ajFmtScanS(line, "%*s %*d %S %u %u %S %S %S %f %f %f", 
		       &tmphit->Acc, 
		       &tmphit->Start, 
		       &tmphit->End, 
		       &tmphit->Group, 
		       &tmphit->Typeobj, 
		       &tmphit->Typesbj, 
		       &tmphit->Score, 
		       &tmphit->Pval, 
		       &tmphit->Eval); 

/* Without Typeobj 
	    ajFmtScanS(line, "%*s %*d %S %u %u %S %f %f %f", 
		       &tmphit->Acc, 
		       &tmphit->Start, 
		       &tmphit->End, 
		       &tmphit->Group, 
		       &tmphit->Score, 
		       &tmphit->Pval, 
		       &tmphit->Eval);  */

	    ajListPush(list, (void *)tmphit);
	}
    }

    ret = embHitlistNew(ajListGetLength(list));
    ajStrAssignS(&ret->Class, class);
    ajStrAssignS(&ret->Architecture, arch);
    ajStrAssignS(&ret->Topology, top);
    ajStrAssignS(&ret->Fold, fold);
    ajStrAssignS(&ret->Superfamily, super);
    ajStrAssignS(&ret->Family, family);
    ret->Sunid_Family = Sunid_Family;
    if(ajStrMatchC(type, "SCOP"))
	(ret)->Type = ajSCOP;
    else if(ajStrMatchC(type, "CATH"))
	(ret)->Type = ajCATH;
    else if(ajStrMatchC(type, "LIGAND"))
      (ret)->Type = ajLIGAND;
    
    ret->N=ajListToarray(list, (void ***)&(ret->hits));
    

    ajListFree(&list);
    ajStrDel(&class);
    ajStrDel(&arch);
    ajStrDel(&top);
    ajStrDel(&fold);
    ajStrDel(&super);
    ajStrDel(&family);
    ajStrDel(&line);
    ajStrDel(&type);
    
    return ret;
}





/* @func embSignatureHitsWrite **********************************************
**
** Writes a list of AjOHit objects to an output file (see documentation
** for the DOMAINATRIX "sigscan" application). This is intended for 
** displaying the results from scans of a signature against a protein sequence
** database.  The Hitlist must have first been classified by a call to 
** embHitlistClassify.  Hits up to the first user-specified number of false
** hits are written.
**
** @param [u] outf    [AjPFile]      Output file stream
** @param [r] sig     [const EmbPSignature] Signature object
** @param [r] hitlist [const EmbPHitlist]   Hitlist objects with hits from scan
** @param [r] n       [ajuint]        Max. no. false hits to output
**
** @return [AjBool] True if file was written
** @@
****************************************************************************/

AjBool embSignatureHitsWrite(AjPFile outf, const EmbPSignature sig, 
			     const EmbPHitlist hitlist, ajuint n)
{
    ajuint  x  = 0;
    ajuint  nf = 0;
    
    
    /* Check args */
    if(!outf || !hitlist || !sig)
	return ajFalse;

    
    /* Print header info */
    ajFmtPrintF(outf, "DE   Results of signature search\nXX\n");


    /* Print SCOP classification records of signature */
    if((sig->Type == ajSCOP))  
	ajFmtPrintF(outf, "TY   SCOP\nXX\n");
    else if ((sig->Type == ajCATH))
	ajFmtPrintF(outf, "TY   CATH\nXX\n");
    else if ((sig->Type == ajLIGAND))
	ajFmtPrintF(outf, "TY   LIGAND\nXX\n");
    if(MAJSTRGETLEN(sig->Class))
	ajFmtPrintF(outf,"CL   %S",sig->Class);
    if(MAJSTRGETLEN(sig->Architecture))    
	ajFmtPrintSplit(outf,sig->Architecture,"\nXX\nAR   ",75," \t\n\r");
    if(MAJSTRGETLEN(sig->Topology)) 
	ajFmtPrintSplit(outf,sig->Topology,"\nXX\nTP   ",75," \t\n\r");
    if(MAJSTRGETLEN(sig->Fold)) 
	ajFmtPrintSplit(outf,sig->Fold,"\nXX\nFO   ",75," \t\n\r");
    if(MAJSTRGETLEN(sig->Superfamily))
	ajFmtPrintSplit(outf,sig->Superfamily,"XX\nSF   ",75," \t\n\r");
    if(MAJSTRGETLEN(sig->Family)) 
	ajFmtPrintSplit(outf,sig->Family,"XX\nFA   ",75," \t\n\r");
    ajFmtPrintF(outf,"XX\nSI   %u\n", sig->Sunid_Family);
    ajFmtPrintF(outf,"XX\n");
    
    
    /* Loop through list and print out data */
    for(x=0;x<hitlist->N; x++)
    {
	if(ajStrMatchC(hitlist->hits[x]->Typeobj, "FALSE"))
	    nf++; 
	if(nf>n)
	    break;  

/* Without Typeobj 
	if(x>n)
	    break; */
	


	if(MAJSTRGETLEN(hitlist->hits[x]->Acc))
	    ajFmtPrintF(outf, "HI  %-6d%-10S%-5d%-5d%-15S%-10S%-10S"
			"%-7.1f%.3e %.3e\n", 
			x+1, hitlist->hits[x]->Acc, 
			hitlist->hits[x]->Start+1, hitlist->hits[x]->End+1,
			hitlist->hits[x]->Group, 
			hitlist->hits[x]->Typeobj, hitlist->hits[x]->Typesbj, 
			hitlist->hits[x]->Score, hitlist->hits[x]->Pval,
			hitlist->hits[x]->Eval);
	else
	    ajFmtPrintF(outf, "HI  %-6d%-10S%-5d%-5d%-15S%-10S%-10S"
			"%-7.1f%.3e %.3e\n", 
			x+1, hitlist->hits[x]->Spr, 
			hitlist->hits[x]->Start+1, hitlist->hits[x]->End+1,
			hitlist->hits[x]->Group, 
			hitlist->hits[x]->Typeobj, hitlist->hits[x]->Typesbj, 
			hitlist->hits[x]->Score, hitlist->hits[x]->Pval,
			hitlist->hits[x]->Eval);

	/* Without Typeobj 
	if(MAJSTRGETLEN(hitlist->hits[x]->Acc))
	    ajFmtPrintF(outf, "HI  %-6d%-10S%-5d%-5d%-15S"
			"%-7.1f%.3e %.3e\n", 
			x+1, hitlist->hits[x]->Acc, 
			hitlist->hits[x]->Start+1, hitlist->hits[x]->End+1,
			hitlist->hits[x]->Group, 
			hitlist->hits[x]->Score, hitlist->hits[x]->Pval,
			hitlist->hits[x]->Eval);
	else
	    ajFmtPrintF(outf, "HI  %-6d%-10S%-5d%-5d%-15S"
			"%-7.1f%.3e %.3e\n", 
			x+1, hitlist->hits[x]->Spr, 
			hitlist->hits[x]->Start+1, hitlist->hits[x]->End+1,
			hitlist->hits[x]->Group, 
			hitlist->hits[x]->Score, hitlist->hits[x]->Pval,
			hitlist->hits[x]->Eval);
			*/

    }
    

    ajFmtPrintF(outf, "XX\n//\n");

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



/* @func embHitlistClassify *************************************************
**
** Classifies a list of signature-sequence hits (held in a Hitlist object) 
** according to list of target sequences (a list of Hitlist objects).
** 
**
** The following classification of hits is taken from the documentation
** for the DOMAINATRIX "sigscan" application :
** Definition of classes of hit 
** The primary classification is an objective definition of the hit and has 
** one of the following values:
** SEED - the sequence was included in the original alignment from which the 
** signature was generated.
** HIT - A protein which was detected by psiblast  (see psiblasts.c) to 
** be a homologue to at least one of the proteins in the family from which 
** the signature was derived. Such proteins are identified by the 'HIT' 
** record in the scop families file.
** OTHER - A true member of the family but not a homologue as detected by 
** psi-blast. Such proteins may have been found from the literature and 
** manually added to the scop families file or may have been detected by the 
** EMBOSS program swissparse (see swissparse.c). They are identified in the 
** scop families file by the 'OTHER' record.
** CROSS - A protein which is homologous to a protein of the same fold,
** but differnt family, of the proteins from which the signature was
** derived.
** FALSE - A homologue to a protein with a different fold to the family
** of the signature.
** UNKNOWN - The protein is not known to be CROSS, FALSE or a true hit (a 
** SEED, HIT or OTHER).
** The secondary classification is provided for convenience and a value as 
** follows:
** Hits of SEED, HIT and OTHER classification are all listed as TRUE.
** Hits of CROSS, FALSE or UNKNOWN objective
** classification are listed as CROSS, 
** FALSE or UNKNOWN respectively.
**
** The Group element is copied from the target sequence for 'TRUE' objective
** hits, whereas 'NOT_APPLICABLE' is given for other types of hit.
**
** The subjective column allows for hand-annotation of the hits files so that 
** proteins of UNKNOWN objective classification can re-classified by a human 
** expert as TRUE, FALSE, CROSS or otherwise
** left as UNKNOWN for the purpose of 
** generating signature performance plots with the EMBOSS application sigplot.
**
** Writes the Group, Typeobj (primary classification) & Typesbj (secondary
** classification) elements depending on how the SCOP classification 
** records of the Hit object and target sequence in question compare.
**
** @param [u] hitlist [EmbPHitlist] Hitlist object with hits
** @param [r] targets [const AjPList]   List of AjOHitlist objects with targets
** @param [r] thresh  [ajuint]       Minimum length (residues) of overlap 
** required for two hits with the same code to be counted as the same hit.
**
** @return [AjBool] True on success, else False
** @@
****************************************************************************/

AjBool embHitlistClassify(EmbPHitlist hitlist, const AjPList targets,
			  ajuint thresh)
{  
    /*
    ** A list of Hitidx structures is derived from the list of AjOHitlist 
    ** objects to allow rapid searching for a given protein accession number
    */
    AjIList itert   = NULL;	/* List iterator for targets */
    EmbPHitlist ptrt = NULL;	/* Pointer for targets (hitlist structure) */
    EmbPHitidx ptri  = NULL;	/* Pointer for index (Hitidx structure) */

    EmbPHitidx *idxarr = NULL;	/* Array of Hitidx structures */
    AjPList idxlist   = NULL;	/* List of Hitidx structures */

    ajint idxsiz = 0;		/* No.target sequences */
    ajint pos    = 0;		/* Position of a matching code in Hitidx 
					  structure */
    ajint tpos = 0;		/* Temp. position counter */
    ajuint x    = 0;		/* Loop counter */

    AjPStr tmpstr = NULL;
    
    EmbPHitlist  lptr;
    EmbPHit      hitx;

    
    /* Check args */
    if(!hitlist || (!targets))
    {
	ajWarn("NULL args passed to embHitlistClassify\n");
	return ajFalse;
    }
    


    /* Create list & list iterator & other memory */
    itert   = ajListIterNewread(targets);
    idxlist = ajListNew();
    tmpstr  = ajStrNew();
    

    /* Loop through list of targets filling list of Hitidx structures */
    while((ptrt=(EmbPHitlist)ajListIterGet(itert)))
    {
	/* Write Hitidx structure */
	for(x=0;x<ptrt->N;x++)
	{
	    ptri = embHitidxNew();
	    ptri->hptr=ptrt->hits[x];
	    ptri->lptr=ptrt;

	    hitx = ptrt->hits[x];

	    if(MAJSTRGETLEN(hitx->Acc))
		ajStrAssignS(&ptri->Id, hitx->Acc);
	    else if(MAJSTRGETLEN(hitx->Spr))
		ajStrAssignS(&ptri->Id, hitx->Spr);
	    else if(MAJSTRGETLEN(hitx->Dom))
		ajStrAssignS(&ptri->Id, hitx->Dom);
	    else 
		ajFatal("None of Acc, Spr or Dom set in embHitlistClassify");
	    
	    ajListPush(idxlist,(EmbPHitidx) ptri);
	}
    }

    
    /* Order the list of Hitidx structures by Id and transform 
       into an array */
    ajListSort(idxlist, embHitidxMatchId);
    idxsiz = ajListToarray(idxlist, (void ***) &idxarr);
        

    /* Loop through list of hits */
    for(x=0; x<hitlist->N; x++)
    {
	hitx = hitlist->hits[x];
	if((MAJSTRGETLEN(hitx->Acc)))
	    pos=embHitidxBinSearch(hitx->Acc, idxarr, idxsiz);
	else if((MAJSTRGETLEN(hitx->Spr)))
	    pos=embHitidxBinSearch(hitx->Spr, idxarr, idxsiz);
	else if((MAJSTRGETLEN(hitx->Dom)))
	    pos=embHitidxBinSearch(hitx->Dom, idxarr, idxsiz);
	else 
	    ajFatal("None of Acc, Spr or Dom are set in embHitlistClassify");

	if(pos!=-1)
	{
	    /*
	    ** Id was found
	    ** The list may contain multiple entries for the same Id, so 
	    ** search the current position and then up the list for other 
	    ** matching strings
	    */
	    tpos=pos;

	    ajDebug("\nAcc '%S' Spr '%S' Dom '%S' found at pos:%d\n",
		    hitx->Acc, hitx->Spr, hitx->Dom, pos);
		    

	    if(MAJSTRGETLEN(hitx->Acc))
		ajStrAssignS(&tmpstr, hitx->Acc);
	    else if(MAJSTRGETLEN(hitx->Spr))
		ajStrAssignS(&tmpstr, hitx->Spr);
	    else if(MAJSTRGETLEN(hitx->Dom))
		ajStrAssignS(&tmpstr, hitx->Dom);
	    else 
		ajFatal("None of Acc, Spr or Dom set in embHitlistClassify");

	    while(ajStrMatchCaseS(idxarr[tpos]->Id, tmpstr))
	    {
		ajDebug("Found '%S' at %d\n",
			idxarr[tpos]->Id, tpos);

		if(embHitsOverlap(idxarr[tpos]->hptr, 
				    hitx, thresh))
		{
		    lptr = idxarr[tpos]->lptr;

		    ajDebug("Hits overlap [%d] '%S'\n",
			    tpos, idxarr[tpos]->Id);
		    ajDebug("Family '%S' '%S'\n",
			    lptr->Family, hitlist->Family);
		    ajDebug("Superfamily '%S' '%S'\n",
			    lptr->Superfamily,
			    hitlist->Superfamily);
		    ajDebug("Fold '%S' '%S'\n",
			    lptr->Fold, hitlist->Fold);
		    ajDebug("Class '%S' '%S'\n",
			    lptr->Class, hitlist->Class);

/*		    if( (idxarr[tpos]->lptr)->Sunid_Family ==
		       hitlist->Sunid_Family) */

		    /* All SCOP nodes are identical */
		    if((ajStrMatchCaseS(lptr->Family, hitlist->Family)) &&
		       (ajStrMatchCaseS(lptr->Superfamily,
					hitlist->Superfamily))          &&
		       (ajStrMatchCaseS(lptr->Fold, hitlist->Fold))     &&
		       (ajStrMatchCaseS(lptr->Class, hitlist->Class)))
		    {

/*			ajStrAssignS(&hitlist->hits[x]->Typeobj, 
				  (idxarr[tpos]->hptr)->Typeobj); */

			ajStrAssignC(&hitx->Typeobj, 
				  "TRUE");
			ajStrAssignC(&hitx->Typesbj, 
				  "TRUE");
			ajStrAssignS(&hitx->Group, 
				  (idxarr[tpos]->hptr)->Group);
		    }
		    else if((ajStrMatchCaseS(lptr->Fold,  hitlist->Fold))   &&
			    (ajStrMatchCaseS(lptr->Class, hitlist->Class)))
			/* SCOP folds are identical */
		    {
			ajStrAssignC(&hitx->Typeobj, "CROSS");
			ajStrAssignC(&hitx->Typesbj, "CROSS");

			ajStrAssignC(&hitx->Group, "NOT_APPLICABLE");
		    }
		    else
			/* SCOP folds are different */
		    {
			ajStrAssignC(&hitx->Typeobj, "FALSE");
			ajStrAssignC(&hitx->Typesbj, "FALSE");
			ajStrAssignC(&hitx->Group, "NOT_APPLICABLE");
		    }
		}
		else
		{

		    ajDebug("Hits do not overlap '%S'\n",
			    idxarr[tpos]->Id);

		    /*
		    ** Id was found but there was no overlap so set 
		    ** classification to UNKNOWN, but only if it has 
		    ** not already been set
		    */
		    if((!ajStrMatchC(hitx->Typesbj, "TRUE")) &&
		       (!ajStrMatchC(hitx->Typesbj, "CROSS")) &&
		       (!ajStrMatchC(hitx->Typesbj, "FALSE")))
		    {
			ajStrAssignC(&hitx->Typeobj, "UNKNOWN");
			ajStrAssignC(&hitx->Typesbj, "UNKNOWN");
			ajStrAssignC(&hitx->Group, "NOT_APPLICABLE");
		    }
		}
		ajDebug("from top hit[%d] obj '%S' sbj '%S' Group '%S'\n",
			x,
			hitx->Typeobj,
			hitx->Typesbj,
			hitx->Group);

		tpos--;	
		if(tpos<0) 
		    break;
	    }	    
				    

	    /* Search down the list */
	    tpos = pos+1; 

	    ajDebug("Search down the list from %d\n",
		    tpos);

	    if(MAJSTRGETLEN(hitx->Acc))
		ajStrAssignS(&tmpstr, hitx->Acc);
	    else if(MAJSTRGETLEN(hitx->Spr))
		ajStrAssignS(&tmpstr, hitx->Spr);
	    else if(MAJSTRGETLEN(hitx->Dom))
		ajStrAssignS(&tmpstr, hitx->Dom);
	    else 
		ajFatal("None of Acc, Spr or Dom set in embHitlistClassify");

	    if(tpos<idxsiz) 
		while(ajStrMatchCaseS(idxarr[tpos]->Id, tmpstr))
		{
		    ajDebug("Found '%S' at %d\n",
			    idxarr[tpos]->Id, tpos);

		    if(embHitsOverlap(idxarr[tpos]->hptr, 
					hitx, thresh))
		    {	
			lptr = idxarr[tpos]->lptr;

			ajDebug("Hits overlap '%S'\n",
				idxarr[tpos]->Id);
			ajDebug("Family '%S' '%S'\n",
				lptr->Family, hitlist->Family);
			ajDebug("Superfamily '%S' '%S'\n",
				lptr->Superfamily,
				hitlist->Superfamily);
			ajDebug("Fold '%S' '%S'\n",
				lptr->Fold, hitlist->Fold);
			ajDebug("Class '%S' '%S'\n",
			    lptr->Class, hitlist->Class);

			/*
			   SCOP family is identical 
			   if( (idxarr[tpos]->lptr)->Sunid_Family ==
			   hitlist->Sunid_Family)  */

			    /* All SCOP nodes are identical */
			if((ajStrMatchCaseS(lptr->Family, hitlist->Family)) &&
			   (ajStrMatchCaseS(lptr->Superfamily,
					    hitlist->Superfamily))          &&
			   (ajStrMatchCaseS(lptr->Fold, hitlist->Fold))     &&
			   (ajStrMatchCaseS(lptr->Class, hitlist->Class)))
			{
			    /*	ajStrAssignS(&hitx->Typeobj, 
					    (idxarr[tpos]->hptr)->Typeobj); */

			    ajStrAssignC(&hitx->Typeobj, "TRUE");
			    ajStrAssignC(&hitx->Typesbj, "TRUE");
			    ajStrAssignS(&hitx->Group, 
				      (idxarr[tpos]->hptr)->Group);
			}
			else if((ajStrMatchCaseS(lptr->Fold,hitlist->Fold)) &&
				(ajStrMatchCaseS(lptr->Class, hitlist->Class)))
			    /* SCOP fold is identical */
			{	
			    ajStrAssignC(&hitx->Typeobj, "CROSS");
			    ajStrAssignC(&hitx->Typesbj, "CROSS");
			    ajStrAssignC(&hitx->Group,
				      "NOT_APPLICABLE");
			}
			else
			    /* SCOP folds are different */
			{
			    ajStrAssignC(&hitx->Typeobj, "FALSE");
			    ajStrAssignC(&hitx->Typesbj, "FALSE");
			    ajStrAssignC(&hitx->Group,
				      "NOT_APPLICABLE");
			}
		    }
  		    else
		    {
			ajDebug("Hits do not overlap [%d] '%S'\n",
				tpos, idxarr[tpos]->Id);

			/*
			** Id was found but there was no overlap so set 
			** classification to UNKNOWN, but only if it has 
			** not already been set
			*/
			if((!ajStrMatchC(hitx->Typesbj, "TRUE")) &&
			   (!ajStrMatchC(hitx->Typesbj, "CROSS")) &&
			   (!ajStrMatchC(hitx->Typesbj, "FALSE")))
			{
			    ajStrAssignC(&hitx->Typeobj, "UNKNOWN");
			    ajStrAssignC(&hitx->Typesbj, "UNKNOWN");
			    ajStrAssignC(&hitx->Group,
				      "NOT_APPLICABLE");
			}
		    }
		    tpos++;	
		    if(tpos==idxsiz) 
			break;
		}
	}
	else
	{
	    ajDebug("id '%S' not found hit[%d] obj '%S' sbj '%S' Group '%S'\n",
		    tmpstr, x,
		    hitx->Typeobj,
		    hitx->Typesbj,
		    hitx->Group);
	    /* Id was NOT found so set classification to UNKNOWN */
	    ajStrAssignC(&hitx->Typeobj, "UNKNOWN");
	    ajStrAssignC(&hitx->Typesbj, "UNKNOWN");
	    ajStrAssignC(&hitx->Group, "NOT_APPLICABLE");
	}
	ajDebug("final '%S' hit[%d] obj '%S' sbj '%S' Group '%S'\n",
		tmpstr, x,
		hitx->Typeobj,
		hitx->Typesbj,
		hitx->Group);
    }
    

    while(ajListPop(idxlist, (void **) &ptri))
	embHitidxDel(&ptri);	
    ajListFree(&idxlist);
    AJFREE(idxarr);
    ajListIterDel(&itert);
    ajStrDel(&tmpstr);

    return ajTrue;
}




/* @func embSignatureCompile ************************************************
**
** Function to compile a signature: calls embSigposNew to allocate an array
** of AjOSigpos objects within an AjOSignature object, and then writes this
** array. The signature must first have been allocated by using the 
** embSignatureNew function.
**
** @param [w] S      [EmbPSignature*] Signature object
** @param [r] gapo   [float]         Gap opening penalty
** @param [r] gape   [float]         Gap extension penalty
** @param [r] matrix [const AjPMatrixf]    Residue substitution matrix
**
** @return [AjBool] True if array was written succesfully.
** @@
****************************************************************************/

AjBool embSignatureCompile(EmbPSignature *S, float gapo, float gape, 
			   const AjPMatrixf matrix)
{
    AjPSeqCvt cvt   = NULL;   /* Conversion array for AjPMatrixf */
    float     **sub = NULL;   /* Substitution matrix from AjPMatrixf */
    ajuint     x     = 0;      
    ajuint     y     = 0;      
    ajuint     z     = 0;      
    AjBool    *tgap = NULL;   /* Temporary array of gap sizes. A cell 
			         == True if a gap is permissible */
    float     *tpen = NULL;   /* Temporary array of gap penalties */
    ajuint     dim   = 0;      /* Dimension of tgap & tpen arrays */
    float     pen   = 0.0;    /* Gap penalty */
    ajuint     ngap  = 0;      /* No. of gaps */
    ajint     divi   = 0;      /* Used in calculating residue match values */
     
    /* CHECK ARGS */
    if(!(*S) || !((*S)->dat) || !((*S)->pos) || !matrix)
	return ajFalse;
    

    /* INITIALISE SUBSTITUTION MATRIX */
    sub  = ajMatrixfArray(matrix);
    cvt  = ajMatrixfCvt(matrix);    

    

    /* LOOP FOR EACH SIGNATURE POSITION */
    for(x=0; x<(*S)->npos; x++)
    {
	/*
	** ALLOCATE TEMP. ARRAY OF GAP SIZES, OF SIZE == 
	** LARGEST GAP + WINDOW SIZE.  NOTE IT IS ESSENTIAL 
	** THAT THE GAP DATA IS GIVEN IN ORDER OF INCREASING
	** GAP SIZE IN THE STRUCTURE
	*/
	/*      (*S)->dat[x]->gsiz[(*S)->dat[x]->ngap - 1] */
	dim = (ajUintGet((*S)->dat[x]->gsiz, (*S)->dat[x]->ngap - 1))
	    + (*S)->dat[x]->wsiz + 1;
	AJCNEW0(tgap, dim);
	AJCNEW0(tpen, dim);
	
	/* FILL TEMP ARRAYS */
	for(y=0; y<(*S)->dat[x]->ngap; y++)
	{
	    /* GAP NOT EXTENDED BY WINDOW */
	    tgap[(ajUintGet((*S)->dat[x]->gsiz, y))]=ajTrue;
	    tpen[(ajUintGet((*S)->dat[x]->gsiz, y))]=0;
	    

	    /* GAP IS EXTENDED BY WINDOW */
	    for(z=1; z<=(*S)->dat[x]->wsiz; z++)
	    {
		pen=gapo+gape*(z-1);
		
		/* A penalty has been assigned for this gap distance before */
		if(tgap[(ajUintGet((*S)->dat[x]->gsiz, y))+z])
		{
		    /* Write the new penalty value if it is lower than the 
		       existing one */
		    if( pen < tpen[(ajUintGet((*S)->dat[x]->gsiz, y))+z])
			tpen[(ajUintGet((*S)->dat[x]->gsiz, y))+z]=pen;
		}
		/* We have not assigned a penalty to this gap distance before */
		else
		{
		    tpen[(ajUintGet((*S)->dat[x]->gsiz, y))+z]=pen;
		    tgap[(ajUintGet((*S)->dat[x]->gsiz, y))+z]=ajTrue;
		}
		
		
		/* A penalty has been assigned for this gap distance before */
		if( ajUintGet((*S)->dat[x]->gsiz, y) >= z)
		{
		    /* Write the new penalty value if it is lower than the 
		       existing one */
		    if(tgap[ajUintGet((*S)->dat[x]->gsiz, y)-z])
		    {
			if(pen < tpen[(ajUintGet((*S)->dat[x]->gsiz, y))-z])
			    tpen[(ajUintGet((*S)->dat[x]->gsiz, y))-z]=pen;
		    }
		    /*
		    ** We have not assigned a penalty to this gap
		    ** distance before
		    */
		    else
		    { 
			tpen[(ajUintGet((*S)->dat[x]->gsiz, y))-z]=pen;
			tgap[(ajUintGet((*S)->dat[x]->gsiz, y))-z]=ajTrue;
		    }
		}
	    }
	}
	

	/* ALLOCATE ARRAY OF Sigpos OBJECTS */
	for(ngap=0, y=0; y<dim; y++)
	    if(tgap[y])
		ngap++;
	(*S)->pos[x] = embSigposNew(ngap);



	/*ASIGN THE GAP DATA  */
	for(ngap=0, y=0; y<dim; y++)
	    if(tgap[y])
	    {
		(*S)->pos[x]->gsiz[ngap]=y;
		(*S)->pos[x]->gpen[ngap]=tpen[y];
		ngap++;
	    }

	/* CALCULATE RESIDUE MATCH VALUES */
	if( (*S)->Typesig==aj1D)
	{
	    for(z=0;z<26; z++)
	    {
		for(divi=0, y=0; y<(*S)->dat[x]->nres; y++)
		{
		    divi+=(ajUintGet((*S)->dat[x]->rfrq, y));
		
		    (*S)->pos[x]->subs[z] += 
			(ajUintGet((*S)->dat[x]->rfrq, y)) * 
			    sub[ajSeqcvtGetCodeK(cvt,(char)((ajint)'A'+z))]
				[ajSeqcvtGetCodeK(cvt, ajChararrGet((*S)->dat[x]->rids,
							     y))];
		}
		(*S)->pos[x]->subs[z] /= divi;
	    }
	}
	else if( (*S)->Typesig==aj3D)
	{
	    for(z=0;z<26; z++)
	    {
		for(divi=0, y=0; y<(*S)->dat[x]->nenv; y++)
		{
		    divi+=(ajUintGet((*S)->dat[x]->efrq, y));
		
		    /* Environments are rows and residue identities are columns. */
		    (*S)->pos[x]->subs[z] += 
			(ajUintGet((*S)->dat[x]->efrq, y)) * 
			    sub[ajSeqcvtGetCodeS(cvt, (*S)->dat[x]->eids[y])]
				[ajSeqcvtGetCodeK(cvt,(char)((ajint)'A'+z))];

		}
		(*S)->pos[x]->subs[z] /= divi;
	    }
	}
	else
	{
	    ajWarn("Signature type (1D or 3D) not known in embSignatureCompile. Presuming 1D");
	    
	    /* This code block identical to above */
	    for(z=0;z<26; z++)
	    {
		for(divi=0, y=0; y<(*S)->dat[x]->nres; y++)
		{
		    divi+=(ajUintGet((*S)->dat[x]->rfrq, y));
		
		    (*S)->pos[x]->subs[z] += 
			(ajUintGet((*S)->dat[x]->rfrq, y)) * 
			    sub[ajSeqcvtGetCodeK(cvt,(char)((ajint)'A'+z))]
				[ajSeqcvtGetCodeK(cvt, ajChararrGet((*S)->dat[x]->rids,
							     y))];
		}
		(*S)->pos[x]->subs[z] /= divi;
	    }
	}
	
	       
		
	/* FREE tgap & tpen ARRAYS */
	AJFREE(tgap);
	AJFREE(tpen);
    }
        
    return ajTrue;
}





/* @func embSignatureAlignSeq ***********************************************
**
** Performs an alignment of a signature to a protein sequence. The signature
** must have first been compiled by calling the embSignatureCompile 
** function.
** A Hit object is written.
**
** @param [r] S      [const EmbPSignature] Signature object
** @param [r] seq    [const AjPSeq]       Protein sequence
** @param [w] hit    [EmbPHit*]      Hit object pointer
** @param [r] nterm  [ajuint]        N-terminal matching option
**
** @return [AjBool] True if a signature-sequence alignment was successful and 
** the Hit object was written.  Returns False if there was an internal error, 
** bad arg's etc. or in cases where a sequence is rejected because of 
** N-terminal matching options). 
** @@
****************************************************************************/

AjBool embSignatureAlignSeq(const EmbPSignature S, const AjPSeq seq,
			    EmbPHit *hit, 
			    ajuint nterm)
{
    const AjPStr  P     = NULL; 
    ajuint  gidx   = 0;	  /*Index into gap array */
    ajuint  glast  = 0;	  /*Index of last gap to try */
    ajint  nres   = 0;	  /*No. of residues in protein */
    ajuint  nresm1 = 0;	  /*== nres-1 */
    static EmbPSigcell path = NULL;  /*Path matrix as 1D array */

    static ajint savedim = 0;		/* dimension of path */
    static ajint savenres = 0;		/* dimension of alg and p */
    ajint dim =0;         /*Dimension of 1D path matrix == nres 
				   * S->npos */
    static char *p = NULL;  /*Protein sequence */
    ajint  start   = 0;	    /* Index into path matrix of first position 
			    ** in the previous row to grow an alignment 
			    ** from
			    */
    ajuint startp = 0;	  /*Index into protein sequence for this 
				    position */
    ajint stop  = 0;	  /*Index into path matrix of last position in 
				    previous row to grow an alignment from */
    ajint this  = 0;	  /*Index into path matrix for current row */
    ajint last  = 0;	  /*Index into path matrix for last row */
    ajint thisp = 0;	  /*Index into protein sequence for current row */
    ajint lastp = 0;	  /*Index into protein sequence for last row */
    ajuint sidx  = 0;	  /*Index into signature */
    float val   = 0;	  /*Value for signature position:residue match */
    float mval  = 0;	  /*Max. value of matches of last signature 
		 	    position:protein sequence */
    ajint max  = 0;	  /*Index into path matrix for cell with mval */
    ajint maxp = 0;      /*Index into protein sequence for path matrix 
				    cell with mval */
    static char *alg = NULL;   /*String for alignment */
    ajint cnt;                 /*A loop counter */
    ajint mlen = 0;            /*Min. possible length of the alignment of the 
				    signature */
    float score=0;             /*Score for alignment */
    double td;
    
    /* CHECK ARGS AND CREATE STRINGS */
    if(!S || !seq || !hit)
	return ajFalse;
    

    P = ajSeqGetSeqS(seq);
    /*Check protein sequence contains alphabetic characters only */
    if(!ajStrIsAlpha(P))
	return ajFalse;


    /* INITIALISE VARIABLES */
    nres   = ajStrGetLen(P);    /* No. columns in path matrix */
    if(nres<1) return ajFalse;

    nresm1 = nres-1;         /* Index of last column in path matrix */
    dim = nres * S->npos;


    /* ALLOCATE MEMORY */
    /*First time the function is called */
    if(!path)
    {
	/* CREATE PATH MATRIX */
	AJCNEW(path, dim);
	savedim = dim;

	/* CREATE ALIGNMENT AND PROTEIN SEQUENCE STRINGS */
	alg = AJALLOC((nres*sizeof(char))+1);
	p = AJALLOC((nres*sizeof(char))+1);
	savenres = nres;
    }	
    else 
    {
	/* CREATE PATH MATRIX */
	if(dim > savedim)
	{
	    AJCRESIZE(path, dim);
	}
	/* CREATE ALIGNMENT AND PROTEIN SEQUENCE STRINGS */
	if((nres) > savenres)
	{
	    AJCRESIZE(alg, nres+1);
	    AJCRESIZE(p, nres+1);
	    savenres = nres;
	}
    }



    /*
    ** INITIALISE PATH MATRIX
    ** Only necessary to initialise <try> element to ajFalse
    */
    for(cnt=0;cnt<dim;cnt++)
	path[cnt].visited = ajFalse;
    
    


    /* COPY SEQUENCE AND CONVERT TO UPPER CASE, OVERWRITE ALIGNMENT STRING */
    strcpy(p, ajStrGetPtr(P));
    ajCharFmtUpper(p);
    for(cnt=0;cnt<nres;cnt++)
	alg[cnt] = '-';
    alg[cnt] = '\0';
    
    switch(nterm)
    {
    case 1:
	/*
	** The first position 
	** can be aligned anywhere in the protein sequence, so long
	** as there is sufficient space to align the rest of the 
	** signature (this is fast, but might not be ideal, e.g. for
        ** detection of fragments.).  Note that gap distance for 
	** first signature position is ignored. Note the function 
	** will return if the whole of the signature can not
	** be aligned
	** This is the RECOMMENDED option
	*/
	
	/*
	** Find last gap to try for first sig. position and return an 
	** error if first sig. position cannot be fitted
	*/
	mlen = 1;   /*For first signature position */
	for(sidx=1;sidx<S->npos;sidx++)
	    mlen+=(1+S->pos[sidx]->gsiz[0]);
	start = startp = 0;
	stop  = nres-mlen;
	if(stop<0)
	    return ajFalse;    
	
	/*
	** Assign path matrix for row 0. 'this' is index into both path
	** matrix and protein sequence in this case.  There is no gap 
	** penalty for the first position.
	** Assign indices into path matrix of start and stop positions for
	** row 0.  
	** Assign index into protein sequence for start position.
	*/
	for(this=0;this<=stop;this++)
	{
	    path[this].val = S->pos[0]->subs[(ajint) ((ajint)p[this] -
						      (ajint)'A')];
	    path[this].prev    = 0;
	    path[this].visited = ajTrue;
	}
	break;
	
    case 2:
	/*
	** The first position 
	** can be aligned anywhere in the protein sequence (this is
	** slower, but means that, e.g. high scoring alignments that
	** are lacking C-terminal signature positions, will not be 
	** discarded.
	*/
	
	for(this=0;this<nres;this++)
	{
	    path[this].val=S->pos[0]->subs[(ajint) ((ajint)p[this] -
						    (ajint)'A')];
	    path[this].prev=0;
	    path[this].visited=ajTrue;
	}
	start = startp = 0;
	stop  = nresm1;
	break;
	
    case 3:
	/*
	** Use empirical gaps only, rather than allowing the 
	** first signature positions to be aligned to anywhere
	** within the sequence
	*/
	
	for(glast=S->pos[0]->ngaps; glast>0; glast--)
	    if(S->pos[0]->gsiz[glast-1]<nresm1)
		break;
	if(glast==0)
	    return ajFalse;
	
	glast--;
	
	for(gidx=0; gidx<=glast; ++gidx)
	{	
	    this=S->pos[0]->gsiz[gidx];
	    path[this].val=S->pos[0]->subs[(ajint) ((ajint)p[this] -
						    (ajint)'A')];
	    path[this].prev    = 0;
	    path[this].visited = ajTrue;
	}
	startp = start = S->pos[0]->gsiz[0];
	stop=S->pos[0]->gsiz[gidx-1];
	break;
	
    default:
	ajFatal("Bad nterm value for embSignatureAlignSeq. "
		"This should never happen.\n");
	break;
    }
    
    
    /*
    ** Assign path matrix for other rows
    ** Loop for each signature position, beginning at row 1
    */
    for(sidx=1;sidx<S->npos;sidx++)
    {
	/*Loop for permissible region of previous row */
	for(last=start, lastp=startp; last<=stop; last++, lastp++)
	{
	    if(path[last].visited==ajFalse)
		continue;

	    /*Loop for each permissible gap in current row */
	    for(gidx=0;gidx<S->pos[sidx]->ngaps;gidx++)
	    {
		if((thisp=lastp+S->pos[sidx]->gsiz[gidx]+1)>(ajint)nresm1)
		    break;

		this = last+nres+S->pos[sidx]->gsiz[gidx]+1;
		val  = path[last].val +
		    S->pos[sidx]->subs[(ajint) (p[thisp] - (ajint)'A')] -
			S->pos[sidx]->gpen[gidx];
		

		if((path[this].visited==ajTrue)&&(val > path[this].val))
		{
		    path[this].val  = val;
		    path[this].prev = last;
		    continue;
		}				
		/*The cell hasn't been visited before so give it a score */
		if(path[this].visited==ajFalse)
		{
		    path[this].val     = val;
		    path[this].prev    = last;
		    path[this].visited = ajTrue;
		    continue;
		}	
	    }
	}
    	
	/* We cannot accomodate the next position */
	if((startp+=(1+S->pos[sidx]->gsiz[0]))>=nresm1)
	    break;
	start += (nres+1+S->pos[sidx]->gsiz[0]);
	/*last gives (index into last position tried)+1  because
	  of loop increment.  */
	
	stop=this;
    }


    /*
    ** Find index into protein sequence and number of signature position 
    ** (row) corresponding to the last cell in the path matrix which was
    ** assigned
    */
    td = floor((double)(this/nres));
    sidx = (ajuint) td;

    thisp = this - (sidx * nres);
    


    /*
    ** Find maximal value in this row ... give mval a silly value
    ** so it is assigned at least once
    */
    for(mval=-1000000 ; thisp>=0; this--, thisp--)
    {
	if(path[this].visited==ajFalse)
	    continue;
	if(path[this].val > mval)
	{
	    mval = path[this].val;
	    max  = this;
	    maxp = thisp;
	}
    }


    /*Assign score for alignment */
    score = mval; 
    score /= S->npos;


    /* Backtrack through matrix */
    alg[maxp] = '*';

    for(this=path[max].prev; sidx>0; this=path[this].prev)
    {
	td = floor((double)(this/nres));
	sidx = (ajuint) td;
	thisp= this - (sidx * nres);
	alg[thisp] = '*';
    }
    

    /* Write hit structure */
    if(!(*hit))
	*hit = embHitNew();
    
    ajStrAssignC(&(*hit)->Model, "SPARSE");    
    ajStrAssignC(&(*hit)->Alg, alg);
    ajStrAssignS(&(*hit)->Seq, P);
    (*hit)->Start=thisp;
    (*hit)->End=maxp;
    ajStrAssignS(&(*hit)->Acc, ajSeqGetAccS(seq));
    if(!MAJSTRGETLEN((*hit)->Acc))
	ajStrAssignS(&(*hit)->Acc, ajSeqGetNameS(seq));
    if(!MAJSTRGETLEN((*hit)->Acc))
	ajWarn("Could not find an accession number or name for a sequence"
	       " in embSignatureAlignSeq");
    (*hit)->Score=score;
    
    return ajTrue;
}





/* @func embSignatureAlignSeqall ********************************************
**
** Aligns a signature to a set of sequences and writes a Hitlist object with 
** the results. The top-scoring <n> hits are written. The signature must have 
** first been compiled by calling the embSignatureCompile function.
** Memory for an Hitlist object must be allocated beforehand by using the 
** Hitlist constructor with an arg. of 0.
**
** @param [r] sig      [const EmbPSignature] Signature object
** @param [u] db       [AjPSeqall]    Protein sequences
** @param [r] n        [ajuint]        Max. number of top-scoring hits to store
** @param [w] hitlist  [EmbPHitlist*]  Hitlist object pointer
** @param [r] nterm    [ajuint]        N-terminal matching option
**
** @return [AjBool] True if Hitlist object was written succesfully.
** @@
****************************************************************************/

AjBool embSignatureAlignSeqall(const EmbPSignature sig, AjPSeqall db,
			       ajuint n, 
			       EmbPHitlist *hitlist, ajuint nterm)
{
    ajint   nhits    = 0;        /* Number of hits written to Hitlist object*/
    ajuint   hitcnt   = 0;        /* Counter of number of hits */
    EmbPHit  hit      = NULL;	 /* The current hit */    
    EmbPHit  ptr      = NULL;	 /* Temp. pointer to hit structure */    
    AjPSeq  seq      = NULL;     /* The current protein sequence from db */ 
    AjPList listhits = NULL;     /* Temp. list of hits */

    /* Check args */
    if(!sig || !db || !hitlist)
    {
	ajWarn("NULL arg passed to embSignatureAlignSeqall");
	return ajFalse;
    }
    

    /* Memory allocation */
    listhits = ajListNew();
    /*    seq = ajSeqNew();    */


    /* Initialise Hitlist object with SCOP records from Signature */
    (*hitlist)->Type = sig->Type;
    (*hitlist)->Sunid_Family = sig->Sunid_Family;
    ajStrAssignS(&(*hitlist)->Class, sig->Class);
    ajStrAssignS(&(*hitlist)->Architecture, sig->Architecture);
    ajStrAssignS(&(*hitlist)->Topology, sig->Topology);
    ajStrAssignS(&(*hitlist)->Fold, sig->Fold);
    ajStrAssignS(&(*hitlist)->Superfamily, sig->Superfamily);
    ajStrAssignS(&(*hitlist)->Family, sig->Family);

        
    /* Search the database */
    while(ajSeqallNext(db,&seq))
    {
	/* Allocate memory for hit */
	hit=embHitNew();
	

	if(!embSignatureAlignSeq(sig, seq, &hit, nterm))
	{	
	    embHitDel(&hit);
	    continue;
	}
	else
	    hitcnt++;
	

	/* Push hit onto list */
	ajListPush(listhits,(EmbPHit) hit);
	

	if(hitcnt>n)
	{	
	    /* Sort list according to score, highest first */
	    ajListSort(listhits, embMatchinvScore);
	 

	    /* Pop the hit (lowest scoring) from the bottom of the list */
	    ajListPopLast(listhits, (void *) &ptr);
	    embHitDel(&ptr);
	}
    }
    

    /* Sort list according to score, highest first */
    ajListSort(listhits, embMatchinvScore);


    /* Convert list to array within Hitlist object */
    nhits=ajListToarray(listhits, (void ***)  &(*hitlist)->hits);
    (*hitlist)->N = nhits;
    

    ajListFree(&listhits);
    ajSeqDel(&seq);

    return ajTrue;
}




