/****************************************************************************
**
** @source embdmx.c
** 
** Algorithms for some of the DOMAINATRIX EMBASSY applications. 
** For use with the Scophit and Scopalign objects.  
** The functionality will eventually be subsumed by other AJAX and NUCLEUS 
** libraries. 
** 
** @author Copyright (C) 2004 Ranjeeva Ranasinghe (rranasin@hgmp.mrc.ac.uk)
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

#include "emboss.h"





/* ======================================================================= */
/* ============================ private data ============================= */
/* ======================================================================= */





/* ======================================================================= */
/* ================= Prototypes for private functions ==================== */
/* ======================================================================= */





/* ======================================================================= */
/* ========================== private functions ========================== */
/* ======================================================================= */





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

/* @func embDmxNrseqNew *******************************************************
**
** Creates an empty non redundant sequence object
**
** @param [r] seq [const AjPSeq] Sequence object
** @return [EmbPDmxNrseq] New non-redundant sequence object
******************************************************************************/

EmbPDmxNrseq embDmxNrseqNew(const AjPSeq seq)
{
    EmbPDmxNrseq ret;

    AJNEW0(ret);
    ret->Seq = ajSeqNewSeq(seq);

    return ret;
}




/* @func embDmxScophitsToHitlist *********************************************
**
** Reads from a list of Scophit objects and writes a Hitlist object 
** with the next block of hits with identical SCOP classification. If the 
** iterator passed in is NULL it will read from the start of the list, 
** otherwise it will read from the current position. Memory for the Hitlist
** will be allocated if necessary and must be freed by the user.
** 
** @param [r] in      [const AjPList]     List of pointers to Scophit objects
** @param [w] out     [EmbPHitlist*] Pointer to Hitlist object
** @param [u] iter    [AjIList*]    Pointer to iterator for list.
**
** @return [AjBool] True on success (lists were processed ok)
** @@
****************************************************************************/

AjBool embDmxScophitsToHitlist(const AjPList in,
			       EmbPHitlist *out, AjIList *iter)
{
    AjPScophit scoptmp = NULL;        /* Temp. pointer to Scophit object */
    EmbPHit tmp      = NULL;           /* Temp. pointer to Hit object */
    AjPList list    = NULL;           /* Temp. list of Hit objects */
    AjBool do_fam   = ajFalse;
    AjBool do_sfam  = ajFalse;
    AjBool do_fold  = ajFalse;
    AjBool do_class = ajFalse;
    AjPStr fam      = NULL;
    AjPStr sfam     = NULL;
    AjPStr fold     = NULL;
    AjPStr class    = NULL;
    ajint Sunid_Family = 0;
    ajint type      = 0;
    
    /* Check args and allocate memory */
    if(!in || !iter)
    {
	ajWarn("NULL arg passed to embDmxScophitsToHitlist");

	return ajFalse;
    }


    /*
    ** If the iterator passed in is NULL it will read from the start of the 
    ** list, otherwise it will read from the current position.
    */
    if(!(*iter))
	*iter=ajListIterNewread(in);


    if(!((scoptmp=(AjPScophit)ajListIterGet(*iter))))
    {
/*	ajWarn("Empty list in embDmxScophitsToHitlist"); */
	ajListIterDel(iter);	
	return ajFalse;
    }

    if(!(*out))
	*out = embHitlistNew(0);
    
    fam   = ajStrNew();
    sfam  = ajStrNew();
    fold  = ajStrNew();
    class = ajStrNew();

    list = ajListNew();

    Sunid_Family=scoptmp->Sunid_Family;
    type = scoptmp->Type;
    
    
    if(scoptmp->Class)
    {
	do_class = ajTrue;
	ajStrAssignS(&class, scoptmp->Class);
    }

    if(scoptmp->Fold)
    {
	do_fold= ajTrue;
	ajStrAssignS(&fold, scoptmp->Fold);
    }

    if(scoptmp->Superfamily)
    {
	do_sfam = ajTrue;
	ajStrAssignS(&sfam, scoptmp->Superfamily);
    }

    if(scoptmp->Family)
    {
	do_fam = ajTrue;
	ajStrAssignS(&fam, scoptmp->Family);
    }

    embDmxScophitToHit(&tmp, scoptmp);
    ajListPush(list, (EmbPHit) tmp);
    tmp = NULL;
        

    while((scoptmp=(AjPScophit)ajListIterGet(*iter)))
    {
	/*
	** The ajListIterGetBack(*iter); return the
	** iterator to the correct position for the 
	** next read
	*/
	if(do_class)
	    if(!ajStrMatchS(scoptmp->Class, class))
	    {
		ajListIterGetBack(*iter);
		break;
	    }
	
	if(do_fold)
	    if(!ajStrMatchS(scoptmp->Fold, fold))
	    {
		ajListIterGetBack(*iter);
		break;
	    }
	
	if(do_sfam)
	    if(!ajStrMatchS(scoptmp->Superfamily, sfam))
	    {
		ajListIterGetBack(*iter);
		break;
	    }
	
	if(do_fam)
	    if(!ajStrMatchS(scoptmp->Family, fam))
	    {
		ajListIterGetBack(*iter);
		break;
	    }
	
	
	embDmxScophitToHit(&tmp, scoptmp);
	ajListPush(list, (EmbPHit) tmp);
	tmp = NULL;
    }

    ajStrAssignS(&(*out)->Class, class);
    ajStrAssignS(&(*out)->Fold, fold);
    ajStrAssignS(&(*out)->Superfamily, sfam);
    ajStrAssignS(&(*out)->Family, fam);
    (*out)->Sunid_Family = Sunid_Family;
    (*out)->Type = type;
    
    /* Copy temp. list to Hitlist */
    (*out)->N = ajListToarray(list, (void ***) &((*out)->hits));

    ajStrDel(&fam);
    ajStrDel(&sfam);
    ajStrDel(&fold);
    ajStrDel(&class);
    ajListFree(&list);	    

    return ajTrue;
}




/* @func embDmxScophitToHit *************************************************
**
** Copies the contents from a Scophit to a Hit object. Creates the Hit object
** if necessary.
**
** @param [w] to   [EmbPHit*] Hit object pointer 
** @param [r] from [const AjPScophit] Scophit object 
**
** @return [AjBool] True if copy was successful.
** @@
****************************************************************************/

AjBool embDmxScophitToHit(EmbPHit *to, const AjPScophit from)
{
    if(!from)
    {
	ajWarn("NULL arg passed to embDmxScophitToHit");

	return ajFalse;
    }
    
    if(!(*to))
	*to = embHitNew();

    ajStrAssignS(&(*to)->Seq, from->Seq);
    (*to)->Start = from->Start;
    (*to)->End   = from->End;
    ajStrAssignS(&(*to)->Acc, from->Acc);
    ajStrAssignS(&(*to)->Spr, from->Spr);
    ajStrAssignS(&(*to)->Dom, from->Dom);
    ajStrAssignS(&(*to)->Typeobj, from->Typeobj);
    ajStrAssignS(&(*to)->Typesbj, from->Typesbj);
    ajStrAssignS(&(*to)->Model, from->Model);
    ajStrAssignS(&(*to)->Alg, from->Alg);
    ajStrAssignS(&(*to)->Group, from->Group);
    (*to)->Rank  = from->Rank;
    (*to)->Score = from->Score;
    (*to)->Eval  = from->Eval;
    (*to)->Pval  = from->Pval;
    (*to)->Target   = from->Target;
    (*to)->Target2  = from->Target2;
    (*to)->Priority = from->Priority;

    return ajTrue;
}




/* @func embDmxScophitsAccToHitlist *****************************************
**
** Reads from a list of Scophit objects and writes a Hitlist object 
** with the next block of hits with identical SCOP classification. A Hit is 
** only written to the Hitlist if an accession number is given.  Also, only 
** one of any pair of duplicate hits (overlapping hits with
** identical accession) 
** will be written to the Hitlist. An 'overlap' is defined as a shared region 
** of 10 or more residues.
** To check for these the list is first be sorted by Accession number.
** 
** If the iterator passed in is NULL it will read from the start of the list, 
** otherwise it will read from the current position. Memory for the Hitlist
** will be allocated if necessary and must be freed by the user.
** 
** @param [r] in      [const AjPList]     List of pointers to Scophit objects
** @param [w] out     [EmbPHitlist*] Pointer to Hitlist object
** @param [u] iter    [AjIList*]    Pointer to iterator for list.
**
** @return [AjBool] True on success (lists were processed ok)
** @@
****************************************************************************/

AjBool embDmxScophitsAccToHitlist(const AjPList in,
				  EmbPHitlist *out, AjIList *iter)
{
    AjPScophit scoptmp = NULL;        /* Temp. pointer to Scophit object */

    EmbPHit tmp   = NULL;            /* Temp. pointer to Hit object */
    AjPList list = NULL;           /* Temp. list of Hit objects */

    AjBool do_fam   = ajFalse;
    AjBool do_sfam  = ajFalse;
    AjBool do_fold  = ajFalse;
    AjBool do_class = ajFalse;

    AjPStr fam   = NULL;
    AjPStr sfam  = NULL;
    AjPStr fold  = NULL;
    AjPStr class = NULL;
    ajint Sunid_Family = 0;
    ajint type   =0;
    
    /* Check args and allocate memory */
    if(!in || !iter)
    {
	ajWarn("NULL arg passed to embDmxScophitsAccToHitlist");

	return ajFalse;
    }


    /*
    ** If the iterator passed in is NULL it will read from the start of the 
    ** list, otherwise it will read from the current position.
    */
    if(!(*iter))
	*iter=ajListIterNewread(in);


    if(!((scoptmp=(AjPScophit)ajListIterGet(*iter))))
    {
	ajWarn("Empty list in embDmxScophitsToHitlist");
	ajListIterDel(iter);	

	return ajFalse;
    }

    /*
    ** Find the first Scophit which has an accession number 
    ** if necessary
    */
    if((ajStrMatchC(scoptmp->Acc,"Not_available")) ||
       (MAJSTRGETLEN(scoptmp->Acc)==0))
    {
	while((scoptmp=(AjPScophit)ajListIterGet(*iter)))
	    if((ajStrMatchC(scoptmp->Acc,"Not_available") == ajFalse) &&
	       (MAJSTRGETLEN(scoptmp->Acc)!=0))
		break;

	if(!scoptmp)
	{
	    ajWarn("List with no Scophits with Acc in "
		   "embDmxScophitsAccToHitlist");
	    ajListIterDel(iter);	

	    return ajFalse;
	}
    }
    

    if(!(*out))
	*out = embHitlistNew(0);
    
    fam   = ajStrNew();
    sfam  = ajStrNew();
    fold  = ajStrNew();
    class = ajStrNew();

    list = ajListNew();

    
    Sunid_Family=scoptmp->Sunid_Family;
    type=scoptmp->Type;
    
    
    if(scoptmp->Class)
    {
	do_class = ajTrue;
	ajStrAssignS(&class, scoptmp->Class);
    }

    if(scoptmp->Fold)
    {
	do_fold= ajTrue;
	ajStrAssignS(&fold, scoptmp->Fold);
    }

    if(scoptmp->Superfamily)
    {
	do_sfam = ajTrue;
	ajStrAssignS(&sfam, scoptmp->Superfamily);
    }

    if(scoptmp->Family)
    {
	do_fam = ajTrue;
	ajStrAssignS(&fam, scoptmp->Family);
    }

    /* Only want to push the hit if it is not targetted */
    if(!scoptmp->Target2)
    {
	embDmxScophitToHit(&tmp, scoptmp);
	ajListPush(list, (EmbPHit) tmp);
	tmp = NULL;
    }
    
        

    while((scoptmp=(AjPScophit)ajListIterGet(*iter)))
    {
	if(do_class)
	    if(!ajStrMatchS(scoptmp->Class, class))
		break;

	if(do_fold)
	    if(!ajStrMatchS(scoptmp->Fold, fold))
		break;

	if(do_sfam)
	    if(!ajStrMatchS(scoptmp->Superfamily, sfam))
		break;

	if(do_fam)
	    if(!ajStrMatchS(scoptmp->Family, fam))
		break;
	
	if((ajStrMatchC(scoptmp->Acc,"Not_available")) ||
	   (MAJSTRGETLEN(scoptmp->Acc)==0))
	    continue;

	if(scoptmp->Target2)
	    continue;
		
	embDmxScophitToHit(&tmp, scoptmp);
	ajListPush(list, (EmbPHit) tmp);
	tmp=NULL;

	continue;
    }

    ajStrAssignS(&(*out)->Class, class);
    ajStrAssignS(&(*out)->Fold, fold);
    ajStrAssignS(&(*out)->Superfamily, sfam);
    ajStrAssignS(&(*out)->Family, fam);
    (*out)->Sunid_Family = Sunid_Family;
    (*out)->Type = type;
        

    /* Copy temp. list to Hitlist */
    (*out)->N = ajListToarray(list, (void ***) &((*out)->hits));

    /* Tidy up and return */
    ajStrDel(&fam);
    ajStrDel(&sfam);
    ajStrDel(&fold);
    ajStrDel(&class);
    ajListFree(&list);	    

    return ajTrue;
}




/* @func embDmxHitsWrite ****************************************************
 ** Writes a list of AjOHit objects to an output file. This is intended for 
 ** displaying the results from scans of a model against a protein sequence
 ** database. Output in a sigplot compatible format.
 **
 ** @param [u] outf    [AjPFile]     Output file stream
 ** @param [u] hits    [EmbPHitlist]  Hitlist objects with hits from scan
 ** @param [r] maxhits [ajint]       Max. hits to write.
 **
 ** @return [AjBool] True if file was written
 ** @@
 ***************************************************************************/

AjBool embDmxHitsWrite(AjPFile outf, EmbPHitlist hits, ajint maxhits)
{
    ajint  x  = 0;

    
    AjPList tmplist = NULL;
    AjPList outlist = NULL; /* rank-ordered list of hits for output */
    AjIList iter    = NULL;
    AjPScophit hit  = NULL;
    
    

    /*Check args*/
    if(!outf || !hits)
        return ajFalse;

    tmplist=ajListNew();
    outlist=ajListNew();
    
    
    /* Push hits onto tmplist */
    ajListPushAppend(tmplist, hits);
    embDmxHitlistToScophits(tmplist, outlist);
    ajListSort(outlist, ajDmxScophitCompPval);
    

    /*Print header info*/
    ajFmtPrintF(outf, "DE   Results of %S search\nXX\n",hits->Model);

    /*Print SCOP classification records of signature */
    ajFmtPrintF(outf,"CL   %S",hits->Class);

    ajFmtPrintSplit(outf,hits->Fold,"FO   ",75," \t\n\r");
    ajFmtPrintF(outf, "XX\n");
    ajFmtPrintSplit(outf,hits->Superfamily,"SF   ",75," \t\n\r");
    ajFmtPrintF(outf, "XX\n");
    ajFmtPrintSplit(outf,hits->Family,"FA   ",75," \t\n\r");
    ajFmtPrintF(outf, "XX\n");

    /*
    ajFmtPrintSplit(outf,hits->Fold,"\nXX\nFO   ",75," \t\n\r");
    ajFmtPrintSplit(outf,hits->Superfamily,"XX\nSF   ",75," \t\n\r");
    ajFmtPrintSplit(outf,hits->Family,"XX\nFA   ",75," \t\n\r");
    */

    ajFmtPrintF(outf,"XX\nSI   %d\n", hits->Sunid_Family);
    ajFmtPrintF(outf,"XX\n");
    

    iter=ajListIterNewread(outlist);

    while((hit=(AjPScophit) ajListIterGet(iter)))
    {
/*Fix for Typeobj 
	if(cnt==maxhits)
	    break; */

	if(x==maxhits)
	    break;
	
	ajFmtPrintF(outf,"HI  %-6d%-10S%-5d%-5d%-15S%-10S%-10S%-10.2f"
		    "%.3e %.3e\n",
                    x+1, hit->Acc, 
                    hit->Start+1, hit->End+1,
                    hit->Group, 
                    hit->Typeobj, hit->Typesbj, 
                    hit->Score, hit->Pval, hit->Eval); 

/* Without Typeobj 
	ajFmtPrintF(outf,"HI  %-6d%-10S%-5d%-5d%-15S%-10.2f"
		    "%.3e %.3e\n",
                    x+1, hit->Acc, 
                    hit->Start+1, hit->End+1,
                    hit->Group, 
                    hit->Score, hit->Pval, hit->Eval); */

	ajDmxScophitDel(&hit);

/*
** CORRECTION
** if(ajStrMatchC(hit->Typeobj, "FALSE"))
**    cnt++;
*/

	x++;
    }
    
    ajListIterDel(&iter);
    ajListFree(&outlist);
    ajListFree(&tmplist);
    
    /*Print tail info*/
    ajFmtPrintF(outf, "XX\n//\n");   
    
    /*Clean up and return*/ 
    return ajTrue;
}




/* @func embDmxScopToScophit ************************************************
**
** Writes a Scophit structure with the common information in a Scop
** structure. The swissprot sequence is taken in preference to the pdb 
** sequence.
**
** @param [r] source  [const AjPScop]       The Scop object to convert
** @param [w] target  [AjPScophit*]   Destination of the the scophit 
**                                    structure to write to. 
**
** @return [AjBool] ajTrue on the success of creating a Scophit structure. 
** @@
****************************************************************************/

AjBool embDmxScopToScophit(const AjPScop source, AjPScophit* target)
{

    if(!source || !target)
    {
	ajWarn("bad args passed to embDmxScopToScophit\n");

	return ajFalse;
    }
    

    ajStrAssignS(&(*target)->Class,source->Class);
    ajStrAssignS(&(*target)->Fold,source->Fold);
    ajStrAssignS(&(*target)->Superfamily,source->Superfamily);
    ajStrAssignS(&(*target)->Family,source->Family);
    (*target)->Sunid_Family = source->Sunid_Family;
    
    /* The swissprot sequence was not available */
    if(ajStrGetLen(source->SeqSpr)==0)
    {
	ajStrAssignS(&(*target)->Seq,source->SeqPdb);
	(*target)->Start = 0;
	(*target)->End   = 0;
	ajStrAssignC(&(*target)->Acc,"Not_available");
	ajStrAssignC(&(*target)->Spr,"Not_available");
    }
    else
    {
	ajStrAssignS(&(*target)->Seq,source->SeqSpr);
	(*target)->Start = source->Startd;
	(*target)->End   = source->Endd;
	ajStrAssignS(&(*target)->Acc,source->Acc);
	ajStrAssignS(&(*target)->Spr,source->Spr);
    }

    ajStrAssignS(&(*target)->Dom,source->Entry);
    
    return ajTrue;
}




/* @func embDmxScopalgToScop ************************************************
**
** Takes a Scopalg object (scop alignment) and an array of Scop objects
** taken from, e.g. a scop classification file.
** Extracts the scop domain codes from the alignment and compiles a list of 
** corresponding Scop objects from the scop classification file.
**
** @param [r] align     [const AjPScopalg]  Contains a seed alignment.
** @param [r] scop_arr  [AjPScop const*]    Array of AjPScop objects
** @param [r] scop_dim  [ajint]       Size of array
** @param [w] list      [AjPList*]    List of Scop objects.
** 
** @return [AjBool] A populated list has been returned
**                  (a file has been written)
** @@
****************************************************************************/

AjBool embDmxScopalgToScop(const AjPScopalg align, AjPScop const *scop_arr, 
			   ajint scop_dim, AjPList* list)
{
    AjPStr entry_up = NULL;  /* Current entry, upper case */
    ajint idx = 0;           /* Index into array for the Pdb code */
    ajuint i   = 0;           /* Simple loop counter */


    entry_up  = ajStrNew();
    

    /* check for bad arguments */
    if(!align)
    {
        ajWarn("Bad args passed to embDmxScopalgToScop");
	ajStrDel(&entry_up);

        return ajFalse;
    }



    /*
    ** write to list the scop structures matching a particular
    ** family of domains
    */
    for(i=0;i<align->N;i++)
    {
	ajStrAssignS(&entry_up, align->Codes[i]);
	ajStrFmtUpper(&entry_up);
	
	
        if((idx = ajScopArrFindScopid(scop_arr,scop_dim,entry_up))==-1)
        {
	    ajStrDel(&entry_up);

	    return ajFalse;
	}
	else
	{
	    /* DIAGNOSTICS
	    ajFmtPrint("Pushing %d (%S)\n", scop_arr[idx]->Sunid_Family, 
		       scop_arr[idx]->Acc); */
	    
            ajListPushAppend(*list,(void*)scop_arr[idx]);
	}
	
    }

    ajStrDel(&entry_up);

    return ajTrue;
}




/* @func embDmxScophitsOverlapAcc *******************************************
**
** Checks for overlap and identical accession numbers between two hits.
**
** @param [r] h1  [const AjPScophit]  Pointer to hit object 1
** @param [r] h2  [const AjPScophit]  Pointer to hit object 2
** @param [r] n   [ajuint]       Threshold number of residues for overlap
**
** @return [AjBool] True if the overlap between the sequences is at least as 
** long as the threshold. False otherwise.
** @@
****************************************************************************/

AjBool embDmxScophitsOverlapAcc(const AjPScophit h1, const AjPScophit h2,
				ajuint n)
{
    if((MAJSTRGETLEN(h1->Seq)<n) || (MAJSTRGETLEN(h2->Seq)<n))
    {
	ajWarn("Sequence length smaller than overlap limit in "
	       "embDmxScophitsOverlapAcc ... checking for string "
	       "match instead");

	if(((ajStrFindS(h1->Seq, h2->Seq)!=-1) ||
	    (ajStrFindS(h2->Seq, h1->Seq)!=-1)) &&
	   (ajStrMatchS(h1->Acc, h2->Acc)))
	    return ajTrue;
	else
	    return ajFalse;
    }

    if( ((((h1->End - h2->Start + 1)>=n) && (h2->Start >= h1->Start)) ||
	 (((h2->End - h1->Start + 1)>=n) && (h1->Start >= h2->Start)))  &&
       (ajStrMatchS(h1->Acc, h2->Acc)))
	return ajTrue;

    return ajFalse;
}




/* @func embDmxScophitsOverlap **********************************************
**
** Checks for overlap between two hits.
**
** @param [r] h1  [const AjPScophit]     Pointer to hit object 1
** @param [r] h2  [const AjPScophit]     Pointer to hit object 2
** @param [r] n   [ajuint]          Threshold number of residues for overlap
**
** @return [AjBool] True if the overlap between the sequences is at least as 
**                  long as the threshold. False otherwise.
** @@
****************************************************************************/

AjBool embDmxScophitsOverlap(const AjPScophit h1, const AjPScophit h2,
			     ajuint n)
{
    if((MAJSTRGETLEN(h1->Seq)<n) || (MAJSTRGETLEN(h2->Seq)<n))
    {
	ajWarn("Sequence length smaller than overlap limit in "
	       "embDmxScophitsOverlap ... checking for string match instead");

	if((ajStrFindS(h1->Seq, h2->Seq)!=-1) ||
	   (ajStrFindS(h2->Seq, h1->Seq)!=-1))
	    return ajTrue;
	else
	    return ajFalse;
    }
    
    if( (((h1->End - h2->Start + 1)>=n) && (h2->Start >= h1->Start)) ||
       (((h2->End - h1->Start + 1)>=n) && (h1->Start >= h2->Start)))
	return ajTrue;

    return ajFalse;
}




/* @func embDmxScophitMerge *************************************************
**
** Creates a new Scophit object which corresponds to a merging of the two 
** sequences from the Scophit objects hit1 and hit2. 
**
** The Typeobj of the merged hit is set.  The merged hit is classified 
** as follows :
** If hit1 or hit2 is a SEED, the merged hit is classified as a SEED.
** Otherwise, if hit1 or hit2 is HIT, the merged hit is clsasified as a HIT.
** If hit1 and hit2 are both OTHER, the merged hit remains classified as 
** OTHER.
** 
** @param [r] hit1     [const AjPScophit]  Scophit 1
** @param [r] hit2     [const AjPScophit]  Scophit 2
**
** @return [AjPScophit] Pointer to Scophit object.
** @@
****************************************************************************/

AjPScophit embDmxScophitMerge(const AjPScophit hit1, const AjPScophit hit2)
{
    AjPScophit ret;
    ajuint start = 0;    /* Start of N-terminal-most sequence */
    ajuint end   = 0;    /* End of N-terminal-most sequence */
    AjPStr temp = NULL;
    

    /* Check args */
    if(!hit1 || !hit2)
    {
	ajWarn("Bad arg's passed to AjPScophitMerge");
	return NULL;
    }

    if(!ajStrMatchS(hit1->Acc, hit2->Acc))
    {
	ajWarn("Merge attempted on 2 hits with different accession numbers");
	return NULL;
    }

    if((hit1->Type != hit2->Type))
    {
	ajWarn("Merge attempted on 2 hits of different domain type");
	return NULL;
    }

    /* Allocate memory */
    ret = ajDmxScophitNew();
    temp = ajStrNew();
    
    ajStrAssignS(&(ret->Acc), hit1->Acc);
    ajStrAssignS(&(ret->Spr), hit1->Spr);
    ajStrAssignS(&(ret->Dom), hit1->Dom);
    ret->Type = hit1->Type;
    
        
    if(ajStrMatchS(hit1->Class, hit2->Class))
    {
	ajStrAssignS(&(ret->Class), hit1->Class);

	if(ajStrMatchS(hit1->Fold, hit2->Fold))
	{
	    ajStrAssignS(&(ret->Fold), hit1->Fold);

	    if(ajStrMatchS(hit1->Superfamily, hit2->Superfamily))
	    {
		ajStrAssignS(&(ret->Superfamily), hit1->Superfamily);

		if(ajStrMatchS(hit1->Family, hit2->Family))
		    ajStrAssignS(&(ret->Family), hit1->Family);
	    }
	}
    }
    

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


    /*
    ** Assign the C-terminus of the sequence of the new hit     
    ** if necessary
    */
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


    /* Classify the merged hit */
    if(ajStrMatchC(hit1->Typeobj, "SEED") ||
       ajStrMatchC(hit1->Typeobj, "SEED"))
	ajStrAssignC(&(ret->Typeobj), "SEED");
    else if(ajStrMatchC(hit1->Typeobj, "HIT") ||
	    ajStrMatchC(hit1->Typeobj, "HIT"))
	ajStrAssignC(&(ret->Typeobj), "HIT");
    else
	ajStrAssignC(&(ret->Typeobj), "OTHER");


    if(ajStrMatchS(hit1->Model, hit2->Model))
	ajStrAssignS(&ret->Model, hit1->Model);
    

    if(hit1->Sunid_Family == hit2->Sunid_Family)
	ret->Sunid_Family = hit1->Sunid_Family;
    
    /* All other elements of structure are left as NULL / o / ajFalse */
        
    ajStrDel(&temp);

    return ret;
}




/* @func embDmxScophitMergeInsertOther **************************************
**
** Creates a new Scophit object which corresponds to a merging of two Scophit
** objects hit1 and hit2. Appends the new Scophit onto a list. Target
** hit1 and hit2 for removal (set the Target element to ajTrue).
** 
** @param [u] list   [AjPList]     List of Scophit objects
** @param [d] hit1   [AjPScophit]  Scophit object 1
** @param [d] hit2   [AjPScophit]  Scophit object 2
**
** @return [AjBool] True on success.
** @@
****************************************************************************/

AjBool embDmxScophitMergeInsertOther(AjPList list, AjPScophit hit1,
				    AjPScophit hit2)
{
    AjPScophit new;

    /* Check args */
    if(!hit1 || !hit2 || !list)
	return ajFalse;
    

    new = embDmxScophitMerge(hit1, hit2);
    ajDmxScophitTarget(&hit1);
    ajDmxScophitTarget(&hit2);
    ajListPushAppend(list, (void *) new);
    
    return ajTrue;
}




/* @func embDmxScophitMergeInsertOtherTarget ********************************
**
** Creates a new Scophit object which corresponds to a merging of two Scophit
** objects hit1 and hit2. Appends the new Scophit onto a list. Target
** hit1 and hit2 for removal (set the Target element to ajTrue).
** 
** @param [u] list   [AjPList]     List of Scophit objects
** @param [d] hit1   [AjPScophit]  Scophit object 1
** @param [d] hit2   [AjPScophit]  Scophit object 2
**
** @return [AjBool] True on success.
** @@
****************************************************************************/

AjBool embDmxScophitMergeInsertOtherTarget(AjPList list, AjPScophit hit1,
					   AjPScophit hit2)
{
    AjPScophit new;

    /* Check args */
    if(!hit1 || !hit2 || !list)
	return ajFalse;
    

    new = embDmxScophitMerge(hit1, hit2);
    ajDmxScophitTarget(&new);
    ajDmxScophitTarget(&hit1);
    ajDmxScophitTarget(&hit2);
    ajListPushAppend(list, (void *) new);
    
    return ajTrue;
}




/* @func embDmxScophitMergeInsertOtherTargetBoth ****************************
**
** Creates a new Scophit object which corresponds to a merging of two Scophit
** objects hit1 and hit2. Appends the new Scophit onto a list. Target
** hit1 and hit2 for removal (set the Target element to ajTrue).
** 
** @param [u] list   [AjPList]     List of Scophit objects
** @param [d] hit1   [AjPScophit]  Scophit object 1
** @param [d] hit2   [AjPScophit]  Scophit object 2
**
** @return [AjBool] True on success.
** @@
****************************************************************************/

AjBool embDmxScophitMergeInsertOtherTargetBoth(AjPList list, AjPScophit hit1,
					      AjPScophit hit2)
{
    AjPScophit new;

    /* Check args */
    if(!hit1 || !hit2 || !list)
	return ajFalse;
    

    new = embDmxScophitMerge(hit1, hit2);
    ajDmxScophitTarget(&new);
    ajDmxScophitTarget(&hit1);
    ajDmxScophitTarget(&hit2);
    ajDmxScophitTarget2(&new);
    ajDmxScophitTarget2(&hit1);
    ajDmxScophitTarget2(&hit2);
    ajListPushAppend(list, (void *) new);
    
    return ajTrue;
}




/* @func embDmxScophitMergeInsertThis ***************************************
**
** Creates a new Scophit object which corresponds to a merging of two Scophit
** objects hit1 and hit2. Insert the new Scophit immediately after hit2. 
** Target hit1 and hit2 for removal (set the Target element to ajTrue).
** 
** @param [r] list   [const AjPList]     List of Scophit objects
** @param [d] hit1   [AjPScophit]  Scophit object 1
** @param [d] hit2   [AjPScophit]  Scophit object 2
** @param [u] iter   [AjIList]     List iterator
**
** @return [AjBool] True on success.
** @@
****************************************************************************/

AjBool embDmxScophitMergeInsertThis(const AjPList list, AjPScophit hit1, 
				   AjPScophit hit2,  AjIList iter)
{
    AjPScophit new;

    /* Check args */
    if(!hit1 || !hit2 || !list || !iter)
	return ajFalse;
    

    new = embDmxScophitMerge(hit1, hit2);
    ajDmxScophitTarget(&hit1);
    ajDmxScophitTarget(&hit2);
    ajListIterInsert(iter, (void *) new);
    
    return ajTrue;
}




/* @func embDmxScophitMergeInsertThisTarget *********************************
**
** Creates a new Scophit object which corresponds to a merging of two Scophit
** objects hit1 and hit2. Insert the new Scophit immediately after hit2. 
** Target hit1, hit2 and the new Scophit for removal
** (set the Target element to ajTrue).
** 
** @param [r] list   [const AjPList]     List of Scophit objects
** @param [d] hit1   [AjPScophit]  Scophit object 1
** @param [d] hit2   [AjPScophit]  Scophit object 2
** @param [u] iter   [AjIList]     List iterator
**
** @return [AjBool] True on success.
** @@
****************************************************************************/

AjBool embDmxScophitMergeInsertThisTarget(const AjPList list,
					  AjPScophit hit1, 
					  AjPScophit hit2,  AjIList iter)
{
    AjPScophit new;

    /* Check args */
    if(!hit1 || !hit2 || !list || !iter)
	return ajFalse;
    

    new = embDmxScophitMerge(hit1, hit2);
    ajDmxScophitTarget(&new);
    ajDmxScophitTarget(&hit1);
    ajDmxScophitTarget(&hit2);
    ajListIterInsert(iter, (void *) new);
    
    return ajTrue;
}




/* @func embDmxScophitMergeInsertThisTargetBoth  ****************************
**
** Creates a new Scophit object which corresponds to a merging of two Scophit
** objects hit1 and hit2. Insert the new Scophit immediately after hit2. 
** Target hit1, hit2 and the new Scophit for removal (both the Target and 
** Target2 elements are set to ajTrue).
** 
** 
** @param [r] list   [const AjPList]     List of Scophit objects
** @param [d] hit1   [AjPScophit]  Scophit object 1
** @param [d] hit2   [AjPScophit]  Scophit object 2
** @param [u] iter   [AjIList]     List iterator
**
** @return [AjBool] True on success.
** @@
****************************************************************************/

AjBool embDmxScophitMergeInsertThisTargetBoth(const AjPList list,
					      AjPScophit hit1, 
					      AjPScophit hit2,
					      AjIList iter)
{
    AjPScophit new;

    /* Check args */
    if(!hit1 || !hit2 || !list || !iter)
	return ajFalse;
    

    new = embDmxScophitMerge(hit1, hit2);
    ajDmxScophitTarget(&new);
    ajDmxScophitTarget(&hit1);
    ajDmxScophitTarget(&hit2);
    ajDmxScophitTarget2(&new);
    ajDmxScophitTarget2(&hit1);
    ajDmxScophitTarget2(&hit2);
    ajListIterInsert(iter, (void *) new);
    
    return ajTrue;
}




/* @func embDmxSeqNR ********************************************************
**
** Reads a list of AjPSeq's and writes an array describing the redundancy in
** the list. Elements in the array correspond to sequences in the list, i.e.
** the array[0] corresponds to the first sequence in the list.
**
** Sequences are classed as redundant (0 in the array, i.e. they are possibly
** to be discarded later) if they exceed a threshold (%) level of sequence
** similarity to any other in the set (the shortest sequence of the current
** pair will be discarded).  If, however, the CheckGarbage argument
** is set ON (True) then the the sequence that is *not* garbage is marked 
** up as redundant. A sequence is "garbage" if the Garbage element of the 
** data structure is set.  If the CheckGarbage argument is set OFF (False) 
** the shortest sequence is marked as redundant as normal.
** 
** The set output will always contain at least 1 sequence.
** 
** @param [r] input  [const AjPList]    List of EmbPDmxNrseq objects
** @param [w] keep   [AjPUint*]    0: rejected (redundant) sequence, 1: the 
                                  sequence was retained
** @param [w] nset   [ajint*]     Number of non-garbage sequences in nr set 
** @param [r] matrix    [const AjPMatrixf] Residue substitution matrix
** @param [r] gapopen   [float]      Gap insertion penalty
** @param [r] gapextend [float]      Gap extension penalty
** @param [r] thresh    [float]      Threshold residue id. for "redundancy"
** @param [r] CheckGarbage [AjBool]  If True, when two sequences are compared
** and deemed redundant, then the Sequence that is *not* garbage is marked 
** up as redundant. A sequence is "garbage" if the Garbage element of the 
** data structure is set.  If False, the shortest sequence is marked as 
** redundant as normal.
**
** @return [AjBool] ajTrue on success
** @@
****************************************************************************/

AjBool embDmxSeqNR(const AjPList input, AjPUint *keep, ajint *nset,
		      const AjPMatrixf matrix, float gapopen, float gapextend,
		      float thresh, AjBool CheckGarbage)
{
    ajint start1  = 0;	  /* Start of seq 1, passed as arg but not used */
    ajint start2  = 0;	  /* Start of seq 2, passed as arg but not used */
    ajint maxarr  = 300;  /* Initial size for matrix */
    ajint len;
    ajint x;		  /* Counter for seq 1 */
    ajint y;		  /* Counter for seq 2 */
    ajint nin;		  /* Number of sequences in input list */
    ajint *compass;

    const char  *p;
    const char  *q;

    AjFloatArray *sub;
    float id   = 0.;	  /* Passed as arg but not used here */
    float sim  = 0.;
    float idx  = 0.;	  /* Passed as arg but not used here */
    float simx = 0.;	  /* Passed as arg but not used here */
    float *path;

    AjPStr m = NULL;	  /* Passed as arg but not used here */
    AjPStr n = NULL;	  /* Passed as arg but not used here */

    EmbPDmxNrseq *inseqs = NULL;	 /* Array containing input sequences */
    AjPUint     lens    = NULL;	 /* 1: Lengths of sequences* in input list */
    AjPFloat2d  scores  = NULL;
    AjPSeqCvt   cvt     = 0;
    AjBool      show    = ajFalse; /* Passed as arg but not used here */


    /* Intitialise some variables */
    AJCNEW(path, maxarr);
    AJCNEW(compass, maxarr);
    m = ajStrNew();
    n = ajStrNew();
    gapopen   = ajRoundFloat(gapopen,8);
    gapextend = ajRoundFloat(gapextend,8);
    sub = ajMatrixfGetMatrix(matrix);
    cvt = ajMatrixfGetCvt(matrix);

    /* Convert the AjPList to an array of AjPseq */
    if(!(nin=ajListToarray(input,(void ***)&inseqs)))
    {
	ajWarn("Zero sized list of sequences passed into SeqsetNR");
	AJFREE(compass);
	AJFREE(path);
	ajStrDel(&m);
	ajStrDel(&n);

	return ajFalse;
    }


    /* Create an ajint array to hold lengths of sequences */
    lens = ajUintNewRes(nin);

    for(x=0; x<nin; x++)
	ajUintPut(&lens,x,ajSeqGetLen(inseqs[x]->Seq));


    /* Set the keep array elements to 1 */
    for(x=0;x<nin;x++)
	ajUintPut(keep,x,1);


    /* Create a 2d float array to hold the similarity scores */
    scores = ajFloat2dNew();

    /* Start of main application loop */
    for(x=0; x<nin; x++)
    {
	for(y=x+1; y<nin; y++)
	{
	    /* DIAGNOSTICS 
	       ajFmtPrint("x=%d y=%d\nComparing\n%S\nto\n%S\n\n", 
		       x, y, inseqs[x]->Seq->Seq, inseqs[y]->Seq->Seq);*/
	    
	    

	    /* Process w/o alignment identical sequences */
	    if(ajStrMatchS(inseqs[x]->Seq->Seq, inseqs[y]->Seq->Seq))
	    {
/*  DIAGNOSTICS		printf("Score=%f\n", 100.0);  */
		
		ajFloat2dPut(&scores,x,y,(float)100.0);
		continue;
	    }


	    /* Intitialise variables for use by alignment functions */
	    len = ajUintGet(lens,x)*ajUintGet(lens,y);

	    if(len>maxarr)
	    {
		AJCRESIZE(path,len);
		AJCRESIZE(compass,len);
		maxarr=len;
	    }

	    p = ajSeqGetSeqC(inseqs[x]->Seq);
	    q = ajSeqGetSeqC(inseqs[y]->Seq);

	    ajStrAssignC(&m,"");
	    ajStrAssignC(&n,"");


	    /* Check that no sequence length is 0 */
	    if((ajUintGet(lens,x)==0)||(ajUintGet(lens,y)==0))
	    {
		ajWarn("Zero length sequence in SeqsetNR");
		AJFREE(compass);
		AJFREE(path);
		ajStrDel(&m);
		ajStrDel(&n);
		ajFloat2dDel(&scores);
		ajUintDel(&lens);
		AJFREE(inseqs);

		return ajFalse;
	    }


	    /* Call alignment functions */
	    embAlignPathCalc(p,q,ajUintGet(lens,x),ajUintGet(lens,y), gapopen,
			     gapextend,path,sub,cvt,compass,show);

	    embAlignWalkNWMatrix(path,inseqs[x]->Seq,inseqs[y]->Seq,&m,&n,
				 ajUintGet(lens,x),ajUintGet(lens,y),
				 &start1,&start2,gapopen,gapextend,compass);

	    embAlignCalcSimilarity(m,n,sub,cvt,ajUintGet(lens,x),
				   ajUintGet(lens,y),&id,&sim,&idx, &simx);


	    /* Write array with score*/
            /* DIAGNOSTICS	   printf("Score=%f\n", sim);  */
	    ajFloat2dPut(&scores,x,y,sim);
	}
    }


    /* DIAGNOSTIC 
    for(x=0; x<nin; x++)
    {
	for(y=x+1; y<nin; y++)	
	{
	    ajFmtPrint("%d:%d : %f\n", x+1, y+1, ajFloat2dGet(scores,x,y));
        }
    }
    */
    


    /* Write the keep array as appropriate */
    for(x=0; x<nin; x++)
    {
	if(!ajUintGet(*keep,x))
	    continue;

	for(y=x+1; y<nin; y++)
	{
	    if(!ajUintGet(*keep,y))
		continue;

	    if(ajFloat2dGet(scores,x,y) >= thresh)
	    {
		/* If both are garbage, set on length as usual */
		if(CheckGarbage && inseqs[x]->Garbage && inseqs[y]->Garbage)
		{
		    if(ajUintGet(lens,x) < ajUintGet(lens,y))
			ajUintPut(keep,x,0);
		    else	
			ajUintPut(keep,y,0);
		}

		/*
                ** If just one is garbage, set non-garbage sequence as
                ** redundant
                */
		else if(CheckGarbage &&
			((inseqs[x]->Garbage) || (inseqs[y]->Garbage)))
		{
		    if(inseqs[x]->Garbage)
			ajUintPut(keep,y,0);
		    else
			ajUintPut(keep,x,0);
		}

		/* Otherwise set on length as usual */
		else
		{
		    if(ajUintGet(lens,x) < ajUintGet(lens,y))
			ajUintPut(keep,x,0);
		    else
			ajUintPut(keep,y,0);
		}
	    }
	}
    }

    for(x=0; x<nin; x++)
	if((!inseqs[x]->Garbage))
	    if(ajUintGet(*keep,x))
		(*nset)++;
    

    /* Keep first sequence in case all have been processed out */
    if(*nset == 0)
    {
	for(x=0; x<nin; x++)
	{
	    if((!inseqs[x]->Garbage))
	    {	
		ajUintPut(keep,x,1);
		*nset = 1;
		break;
	    }
	}

	if(*nset == 0)
	    ajWarn("nset == 0 after processing");
    }
    
/*
** DIAGNOSTIC
**  for(x=0; x<nin; x++)
**      for(y=x+1; y<nin; y++)
**          ajFmtPrint("x=%d y=%d\nComparing\n%S\nto\n%S\n\n", 
**              x, y, inseqs[x]->Seq->Seq, inseqs[y]->Seq->Seq);
*/

    AJFREE(compass);
    AJFREE(path);
    ajStrDel(&m);
    ajStrDel(&n);
    ajFloat2dDel(&scores);
    ajUintDel(&lens);
    AJFREE(inseqs);

    return ajTrue;
}




/* @func embDmxSeqNRRange****************************************************
**
** Reads a list of AjPSeq's and writes an array describing the redundancy in
** the list. Elements in the array correspond to sequences in the list, i.e.
** the array[0] corresponds to the first sequence in the list.
**
** Sequences are classed as redundant (0 in the array, i.e. they are possibly
** to be discarded later) if they lie outside a range of threshold (%) 
** sequence similarity to others in the set (the shortest sequence of the 
** current pair will be discarded).  If, however, the CheckGarbage argument
** is set ON (True) then the the sequence that is *not* garbage is marked 
** up as redundant. A sequence is "garbage" if the Garbage element of the 
** data structure is set.  If the CheckGarbage argument is set OFF (False) 
** the shortest sequence is marked as redundant as normal.
**
** @param [r] input  [const AjPList]    List of EmbPDmxNrseq objects
** @param [w] keep   [AjPUint*]    0: rejected (redundant) sequence, 1: the
                                  sequence was retained
** @param [w] nset   [ajint*]     Number of non-garbage sequences in nr set.
** @param [r] matrix    [const AjPMatrixf] Residue substitution matrix
** @param [r] gapopen   [float]      Gap insertion penalty
** @param [r] gapextend [float]      Gap extension penalty
** @param [r] threshlow    [float]    Threshold lower limit
** @param [r] threshup    [float]    Threshold upper limit
** @param [r] CheckGarbage [AjBool]  If True, when two sequences are compared
** and deemed redundant, then the Sequence that is *not* garbage is marked 
** up as redundant. A sequence is "garbage" if the Garbage element of the 
** data structure is set.  If False, the shortest sequence is marked as 
** redundant as normal.
** 
**
** @return [AjBool] ajTrue on success
** @@
****************************************************************************/

AjBool embDmxSeqNRRange(const AjPList input, AjPUint *keep, ajint *nset,
		      const AjPMatrixf matrix, float gapopen, float gapextend,
		      float threshlow, float threshup, AjBool CheckGarbage)
{
    ajint start1 = 0;	/* Start of seq 1, passed as arg but not used */
    ajint start2 = 0;	/* Start of seq 2, passed as arg but not used */
    ajint maxarr = 300;	/* Initial size for matrix */
    ajint len;
    ajint x;		/* Counter for seq 1 */
    ajint y;		/* Counter for seq 2 */
    ajint nin;		/* Number of sequences in input list */
    ajint *compass;

    const char  *p;
    const char  *q;

    float **sub;
    float id   = 0.;	/* Passed as arg but not used here */
    float sim  = 0.;
    float idx  = 0.;	/* Passed as arg but not used here */
    float simx = 0.;	/* Passed as arg but not used here */
    float *path;

    AjPStr m = NULL;	/* Passed as arg but not used here */
    AjPStr n = NULL;	/* Passed as arg but not used here */

    EmbPDmxNrseq *inseqs = NULL;	/* Array containing input sequences */
    AjPUint lens    = NULL;	/* 1: Lengths of sequences* in input list */
    AjPFloat2d  scores = NULL;
    AjPSeqCvt cvt = 0;
    AjBool show = ajFalse;	/* Passed as arg but not used here */



    /* Intitialise some variables */
    AJCNEW(path, maxarr);
    AJCNEW(compass, maxarr);
    m = ajStrNew();
    n = ajStrNew();
    gapopen   = ajRoundFloat(gapopen,8);
    gapextend = ajRoundFloat(gapextend,8);
    sub = ajMatrixfGetMatrix(matrix);
    cvt = ajMatrixfGetCvt(matrix);


    /* Convert the AjPList to an array of AjPseq */
    if(!(nin=ajListToarray(input,(void ***)&inseqs)))
    {
	ajWarn("Zero sized list of sequences passed into SeqsetNR");
	AJFREE(compass);
	AJFREE(path);
	ajStrDel(&m);
	ajStrDel(&n);

	return ajFalse;
    }


    /* Create an ajint array to hold lengths of sequences */
    lens = ajUintNewRes(nin);

    for(x=0; x<nin; x++)
	ajUintPut(&lens,x,ajSeqGetLen(inseqs[x]->Seq));


    /* Set the keep array elements to 1 */
    for(x=0;x<nin;x++)
	ajUintPut(keep,x,1);


    /* Create a 2d float array to hold the similarity scores */
    scores = ajFloat2dNew();


    /* Start of main application loop */
    for(x=0; x<nin; x++)
    {
	for(y=x+1; y<nin; y++)
	{
	    /* Process w/o alignment identical sequences */
	    if(ajStrMatchS(inseqs[x]->Seq->Seq, inseqs[y]->Seq->Seq))
	    {
		ajFloat2dPut(&scores,x,y,(float)100.0);
		continue;
	    }


	    /* Intitialise variables for use by alignment functions */
	    len = ajUintGet(lens,x)*ajUintGet(lens,y);

	    if(len>maxarr)
	    {
		AJCRESIZE(path,len);
		AJCRESIZE(compass,len);
		maxarr=len;
	    }

	    p = ajSeqGetSeqC(inseqs[x]->Seq);
	    q = ajSeqGetSeqC(inseqs[y]->Seq);

	    ajStrAssignC(&m,"");
	    ajStrAssignC(&n,"");


	    /* Check that no sequence length is 0 */
	    if((ajUintGet(lens,x)==0)||(ajUintGet(lens,y)==0))
	    {
		ajWarn("Zero length sequence in SeqsetNR");
		AJFREE(compass);
		AJFREE(path);
		ajStrDel(&m);
		ajStrDel(&n);
		ajFloat2dDel(&scores);
		ajUintDel(&lens);
		AJFREE(inseqs);

		return ajFalse;
	    }


	    /* Call alignment functions */
	    embAlignPathCalc(p,q,ajUintGet(lens,x),ajUintGet(lens,y), gapopen,
			     gapextend,path,sub,cvt,compass,show);

	    embAlignWalkNWMatrix(path,inseqs[x]->Seq,inseqs[y]->Seq,&m,&n,
				 ajUintGet(lens,x),ajUintGet(lens,y),
				 &start1,&start2,gapopen,gapextend,compass);

	    embAlignCalcSimilarity(m,n,sub,cvt,ajUintGet(lens,x),
				   ajUintGet(lens,y),&id,&sim,&idx, &simx);


	    /* Write array with score */
	    ajFloat2dPut(&scores,x,y,sim);
	}
    }


    /* Write the keep array as appropriate, first check the upper limit */
    for(x=0; x<nin; x++)
    {
	if(!ajUintGet(*keep,x))
	    continue;

	for(y=x+1; y<nin; y++)
	{
	    if(!ajUintGet(*keep,y))
		continue;

/*	    if(ajFloat2dGet(scores,x,y) >= threshup) */
	    
	    if((ajFloat2dGet(scores,x,y) <= threshup) &&
	       (ajFloat2dGet(scores,x,y) >= threshlow))

	    {
		/* If both are garbage, set on length as usual */
		if(CheckGarbage && inseqs[x]->Garbage && inseqs[y]->Garbage)
		{
		    if(ajUintGet(lens,x) < ajUintGet(lens,y))
			ajUintPut(keep,x,0);
		    else	
			ajUintPut(keep,y,0);
		}

		/*
                ** If just one is garbage, set non-garbage sequence as
                ** redundant
                */
		else if(CheckGarbage && ((inseqs[x]->Garbage) ||
                                         (inseqs[y]->Garbage)))
		{
		    if(inseqs[x]->Garbage)
			ajUintPut(keep,y,0);
		    else
			ajUintPut(keep,x,0);
		}
		/* Otherwise set on length as usual */
		else
		{
		    if(ajUintGet(lens,x) < ajUintGet(lens,y))
			ajUintPut(keep,x,0);
		    else
			ajUintPut(keep,y,0);
		}
	    }
	}
    }


    /* Now check the lower limit */
    /*
    for(x=0; x<nin; x++)
    {
	if(!ajUintGet(*keep,x))
	    continue;

	ok = ajFalse;

	for(y=x+1; y<nin; y++)
	{
	    if(!ajUintGet(*keep,y))
		continue;

	    if(ajFloat2dGet(scores,x,y) >= threshlow)
	    {
		ok = ajTrue;
		break;
	    }
	}

	if(!ok)
	    ajUintPut(keep,x,0);
    }
    */

    for(x=0; x<nin; x++)
	if((!inseqs[x]->Garbage))
	    if(ajUintGet(*keep,x))
		(*nset)++;



    /* Keep first sequence in case all have been processed out */
    if(*nset == 0)
    {
	for(x=0; x<nin; x++)
	{
	    if((!inseqs[x]->Garbage))
	    {	
		ajUintPut(keep,x,1);
		*nset = 1;
		break;
	    }
	}

	if(*nset == 0)
	    ajWarn("nset == 0 after processing");
    }
    
    /* Tidy up */
    AJFREE(compass);
    AJFREE(path);
    ajStrDel(&m);
    ajStrDel(&n);
    ajFloat2dDel(&scores);
    ajUintDel(&lens);
    AJFREE(inseqs);

    return ajTrue;
}




/* @func embDmxSeqCompall ***************************************************
**
** Reads a list of AjPSeq's and writes an array of sequence similarity values
** for an all-versus-all comparison of the sequences.  The rows and columns 
** in the array correspond to the order of the sequences in the list.
**
** 
** @param [r] input  [const AjPList]    List of ajPSeq's 
** @param [w] scores [AjPFloat2d*] Sequence similarity values
** @param [r] matrix    [const AjPMatrixf] Residue substitution matrix
** @param [r] gapopen   [float]      Gap insertion penalty
** @param [r] gapextend [float]      Gap extension penalty
**
** @return [AjBool] ajTrue on success
** @@
****************************************************************************/

AjBool embDmxSeqCompall(const AjPList input, AjPFloat2d *scores, 
		   const AjPMatrixf matrix, float gapopen, float gapextend)
{
    ajint start1  = 0;	  /* Start of seq 1, passed as arg but not used */
    ajint start2  = 0;	  /* Start of seq 2, passed as arg but not used */
    ajint maxarr  = 300;  /* Initial size for matrix */
    ajint len;
    ajint x;		  /* Counter for seq 1 */
    ajint y;		  /* Counter for seq 2 */
    ajint nin;		  /* Number of sequences in input list */
    ajint *compass;

    const char  *p;
    const char  *q;

    AjFloatArray *sub;
    float id   = 0.;	  /* Passed as arg but not used here */
    float sim  = 0.;
    float idx  = 0.;	  /* Passed as arg but not used here */
    float simx = 0.;	  /* Passed as arg but not used here */
    float *path;

    AjPStr m = NULL;	  /* Passed as arg but not used here */
    AjPStr n = NULL;	  /* Passed as arg but not used here */

    AjPSeq      *inseqs = NULL;	 /* Array containing input sequences */
    AjPUint     lens    = NULL;	 /* 1: Lengths of sequences* in input list */
    AjPSeqCvt   cvt     = 0;
    AjBool      show    = ajFalse; /* Passed as arg but not used here */


    /* Intitialise some variables */
    AJCNEW(path, maxarr);
    AJCNEW(compass, maxarr);
    m = ajStrNew();
    n = ajStrNew();
    gapopen   = ajRoundFloat(gapopen,8);
    gapextend = ajRoundFloat(gapextend,8);
    sub = ajMatrixfGetMatrix(matrix);
    cvt = ajMatrixfGetCvt(matrix);

    /* Convert the AjPList to an array of AjPseq */
    if(!(nin=ajListToarray(input,(void ***)&inseqs)))
    {
	ajWarn("Zero sized list of sequences passed into embDmxSeqCompall");
	AJFREE(compass);
	AJFREE(path);
	ajStrDel(&m);
	ajStrDel(&n);

	return ajFalse;
    }


    /* Create an ajint array to hold lengths of sequences */
    lens = ajUintNewRes(nin);

    for(x=0; x<nin; x++)
	ajUintPut(&lens,x,ajSeqGetLen(inseqs[x]));


    /* Create a 2d float array to hold the similarity scores */
    *scores = ajFloat2dNew();

    /* Start of main application loop */
    for(x=0; x<nin; x++)
    {
	for(y=x+1; y<nin; y++)
	{
	    /* DIAGNOSTICS 
	       ajFmtPrint("x=%d y=%d\nComparing\n%S\nto\n%S\n\n", 
		       x, y, inseqs[x]->Seq, inseqs[y]->Seq);*/
	    
	    

	    /* Process w/o alignment identical sequences */
	    if(ajStrMatchS(inseqs[x]->Seq, inseqs[y]->Seq))
	    {
/*  DIAGNOSTICS		printf("Score=%f\n", 100.0);  */
		
		ajFloat2dPut(scores,x,y,(float)100.0);
		continue;
	    }


	    /* Intitialise variables for use by alignment functions */
	    len = ajUintGet(lens,x)*ajUintGet(lens,y);

	    if(len>maxarr)
	    {
		AJCRESIZE(path,len);
		AJCRESIZE(compass,len);
		maxarr=len;
	    }

	    p = ajSeqGetSeqC(inseqs[x]);
	    q = ajSeqGetSeqC(inseqs[y]);

	    ajStrAssignC(&m,"");
	    ajStrAssignC(&n,"");


	    /* Check that no sequence length is 0 */
	    if((ajUintGet(lens,x)==0)||(ajUintGet(lens,y)==0))
	    {
		ajWarn("Zero length sequence in embDmxSeqCompall");
		AJFREE(compass);
		AJFREE(path);
		ajStrDel(&m);
		ajStrDel(&n);
		ajFloat2dDel(scores);
		ajUintDel(&lens);
		AJFREE(inseqs);

		return ajFalse;
	    }


	    /* Call alignment functions */
	    embAlignPathCalc(p,q,ajUintGet(lens,x),ajUintGet(lens,y), gapopen,
			     gapextend,path,sub,cvt,compass,show);

	    embAlignWalkNWMatrix(path,inseqs[x],inseqs[y],&m,&n,
				 ajUintGet(lens,x),ajUintGet(lens,y),
				 &start1,&start2,gapopen,gapextend,compass);

	    embAlignCalcSimilarity(m,n,sub,cvt,ajUintGet(lens,x),
				   ajUintGet(lens,y),&id,&sim,&idx, &simx);


	    /* Write array with score*/
            /* DIAGNOSTICS	   printf("Score=%f\n", sim);  */
            ajFloat2dPut(scores,x,y,sim);
	}
    }


    /* DIAGNOSTIC 
    for(x=0; x<nin; x++)
    {
	for(y=x+1; y<nin; y++)	
	{
	    ajFmtPrint("%d:%d : %f\n", x+1, y+1, ajFloat2dGet(*scores,x,y));
        }
    }
    */
    

/*
    for(x=0; x<nin; x++)
    {
	for(y=x+1; y<nin; y++)
	{
            ajFmtPrint("x=%d y=%d\nComparing\n%S\nto\n%S\n\n", 
		       x, y, inseqs[x]->Seq, inseqs[y]->Seq);
	}
    }
*/  


    AJFREE(compass);
    AJFREE(path);
    ajStrDel(&m);
    ajStrDel(&n);
    ajUintDel(&lens);
    AJFREE(inseqs);

    return ajTrue;
}




/* @func embDmxScophitReadAllFasta ********************************************
**
** Reads a DHF file and returns a list of Scophit objects. 
** Parsing routine is identical to embHitlistReadFasta.
** 
** @param [u] inf      [AjPFile]  DHF file.
**
** @return [AjPList] List of Scophit object pointers, or NULL (error).
** @@
******************************************************************************/

AjPList  embDmxScophitReadAllFasta(AjPFile inf)
{
    AjPScophit hit       = NULL;    /* Current hit.                     */
    AjPList    tmplist   = NULL;    /* Temp. list of hits               */
    AjBool     donefirst = ajFalse; /* Read first code line.            */
    ajint     ntok       = 0;       /* No. tokens in a line.            */
    const AjPStr token   = NULL;
    AjPStr    line       = NULL;    /* Line of text.                    */
    AjPStr    subline    = NULL;
    AjBool    ok         = ajFalse;
    AjBool    parseok    = ajFalse;
    AjPStr    type       = NULL;
    

    /* Allocate strings */
    line     = ajStrNew();
    subline  = ajStrNew();
    tmplist  = ajListNew();
    type     = ajStrNew();
    

    while((ok = ajReadlineTrim(inf,&line)))
    {
	if(ajStrPrefixC(line,">"))
	{
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
		ajFatal("Incorrect no. (%d) of tokens on line %S\n", ntok,
                        line);
	    else
	    {
		parseok = ajTrue;
		hit     = ajDmxScophitNew();
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
	    ajFmtScanS(token, "%d", &hit->Start);

	    /* End */
	    token = ajStrParseC(NULL, "^");
	    ajFmtScanS(token, "%d", &hit->End);
	    
	    /* Type */
	    token = ajStrParseC(NULL, "^");
	    ajStrAssignS(&type, token);

	    if(ajStrMatchC(type, "SCOP"))
		hit->Type = ajSCOP;
	    else if(ajStrMatchC(type, "CATH"))
		hit->Type = ajCATH;

	    /* Dom */
	    token = ajStrParseC(NULL, "^");
	    ajStrAssignS(&hit->Dom, token);

	    if(ajStrMatchC(hit->Dom, "."))
		ajStrSetClear(&hit->Dom);

	    /* Read domain identifier */
	    token = ajStrParseC(NULL, "^");
	    ajFmtScanS(token, "%d", &hit->Sunid_Family);
	    

	    token = ajStrParseC(NULL, "^");
	    ajStrAssignS(&hit->Class, token);

	    if(ajStrMatchC(hit->Class, "."))
		ajStrSetClear(&hit->Class);	

	    token = ajStrParseC(NULL, "^");
	    ajStrAssignS(&hit->Architecture, token);

	    if(ajStrMatchC(hit->Architecture, "."))
		ajStrSetClear(&hit->Architecture);

	    token = ajStrParseC(NULL, "^");
	    ajStrAssignS(&hit->Topology, token);

	    if(ajStrMatchC(hit->Topology, "."))
		ajStrSetClear(&hit->Topology);

	    token = ajStrParseC(NULL, "^");
	    ajStrAssignS(&hit->Fold, token);

	    if(ajStrMatchC(hit->Fold, "."))
		ajStrSetClear(&hit->Fold);

	    token = ajStrParseC(NULL, "^");
	    ajStrAssignS(&hit->Superfamily, token);

	    if(ajStrMatchC(hit->Superfamily, "."))
		ajStrSetClear(&hit->Superfamily);

	    token = ajStrParseC(NULL, "^");
	    ajStrAssignS(&hit->Family, token);

	    if(ajStrMatchC(hit->Family, "."))
		ajStrSetClear(&hit->Family);

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
	    ajStrAppendS(&hit->Seq, line);
    }


    /* EOF therefore process last hit */
    if((!ok) && (parseok))
    {
	ajStrRemoveWhite(&hit->Seq);
	ajListPushAppend(tmplist, hit);
    }


    /* Tidy up */
    ajStrDel(&line);
    ajStrDel(&subline);
    ajStrDel(&type);
    
    
    /* File read error */
    return tmplist;
}




/* @func embDmxHitlistToScophits ********************************************
**
** Read from a list of Hitlist structures and writes a list of Scophit 
** structures.
** 
** @param [r] in      [const AjPList]  List of pointers to Hitlist structures
** @param [w] out     [AjPList] List of Scophit structures
**
** @return [AjBool] True on success (lists were processed ok)
** @@
****************************************************************************/

AjBool embDmxHitlistToScophits(const AjPList in, AjPList out)
{
    AjPScophit scophit = NULL;   /* Pointer to Scophit object */
    EmbPHitlist hitlist = NULL;   /* Pointer to Hitlist object */
    AjIList iter = NULL;         /* List iterator */
    ajuint x      = 0;            /* Loop counter */


    /* Check args */
    if(!in)
    {
	ajWarn("Null arg passed to embDmxHitlistToScophits");

	return ajFalse;
    }

    /* Create list iterator and new list */
    iter = ajListIterNewread(in);	
    

    /* Iterate through the list of Hitlist pointers */
    while((hitlist=(EmbPHitlist)ajListIterGet(iter)))
    {
	/* Loop for each hit in hitlist structure */
	for(x=0; x<hitlist->N; ++x)
	{
	    /* Create a new scophit structure */
	    scophit = ajDmxScophitNew();
	    

	    /* Assign scop classification records from hitlist structure */
	    scophit->Type = hitlist->Type;
	    ajStrAssignS(&scophit->Class, hitlist->Class);
	    ajStrAssignS(&scophit->Fold, hitlist->Fold);
	    ajStrAssignS(&scophit->Superfamily, hitlist->Superfamily);
	    ajStrAssignS(&scophit->Family, hitlist->Family);
	    scophit->Sunid_Family = hitlist->Sunid_Family;
	    scophit->Priority = hitlist->Priority;
	    
	    /* Assign records from hit structure */
	    ajStrAssignS(&scophit->Seq, hitlist->hits[x]->Seq);
	    ajStrAssignS(&scophit->Acc, hitlist->hits[x]->Acc);
	    ajStrAssignS(&scophit->Spr, hitlist->hits[x]->Spr);
	    ajStrAssignS(&scophit->Dom, hitlist->hits[x]->Dom);
	    ajStrAssignS(&scophit->Typeobj, hitlist->hits[x]->Typeobj);
	    ajStrAssignS(&scophit->Typesbj, hitlist->hits[x]->Typesbj);
	    ajStrAssignS(&scophit->Model, hitlist->hits[x]->Model);
	    ajStrAssignS(&scophit->Alg, hitlist->hits[x]->Alg);
	    ajStrAssignS(&scophit->Group, hitlist->hits[x]->Group);
	    scophit->Start = hitlist->hits[x]->Start;
	    scophit->End = hitlist->hits[x]->End;
	    scophit->Rank = hitlist->hits[x]->Rank;
	    scophit->Score = hitlist->hits[x]->Score;
	    scophit->Eval = hitlist->hits[x]->Eval;
	    scophit->Pval = hitlist->hits[x]->Pval;
	    
           	     
	    /* Push scophit onto list */
	    ajListPushAppend(out,scophit);
	}
    }	
    
    ajListIterDel(&iter);	

    return ajTrue;
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


/* @func embDmxNrseqDel *******************************************************
**
** Creates an empty non redundant sequence object
**
** @param [d] Pnrseq [EmbPDmxNrseq*] Non-redundant sequence object
** @return [void]
******************************************************************************/

void embDmxNrseqDel(EmbPDmxNrseq* Pnrseq)
{
    EmbPDmxNrseq nrseq;

    if(!Pnrseq)
        return;

    nrseq = *Pnrseq;

    if(!nrseq)
        return;
    
    ajSeqDel(&nrseq->Seq);
    AJFREE(*Pnrseq);

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





/* ======================================================================= */
/* ========================== Input & Output ============================= */
/* ======================================================================= */

/* @section Input & output **************************************************
**
** These functions are used for formatted input and output to file.    
**
****************************************************************************/





/* ======================================================================= */
/* ======================== Miscellaneous =================================*/
/* ======================================================================= */
/* @section Miscellaneous ***************************************************
**
** These functions may have diverse functions that do not fit into the other
** categories. 
**
****************************************************************************/
