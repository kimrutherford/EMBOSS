/****************************************************************************
**
** @source embdmx.h
** 
** @source Algorithms for some of the DOMAINATRIX EMBASSY applications. 
** For Scophit and Scopalign objects defined in ajxyz.h
** The functionality will eventually be subsumed by other AJAX and NUCLEUS 
** libraries. 
** 
** @author: Copyright (C) 2004 Ranjeeva Ranasinghe (rranasin@hgmp.mrc.ac.uk)
** @author: Copyright (C) 2004 Jon Ison (jison@hgmp.mrc.ac.uk) 
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

#ifdef __cplusplus
extern "C"
{
#endif

#ifndef embdmx_h
#define embdmx_h

/* @data EmbPDmxNrseq *********************************************************
**
** NUCLEUS data structure for non-redundant sequence testing
**

** @attr Seq [AjPSeq] Sequence to be tested for redundancy
** @attr Garbage [AjBool] True if sequence is to be ignored
** @attr Padding [char[4]] Padding to alignment boundary
** @@
******************************************************************************/

typedef struct EmbSDmxNrseq {
  AjPSeq Seq;
  AjBool Garbage;
  char Padding[4];
} EmbODmxNrseq;
#define EmbPDmxNrseq EmbODmxNrseq*





/*****************************************************************************/
/* These functions are NOT YET DOCUMENTED in the appropriate data structures */
/*****************************************************************************/



/*
** Prototype definitions
*/

void          embDmxNrseqDel(EmbPDmxNrseq* Pnrseq);

EmbPDmxNrseq  embDmxNrseqNew(const AjPSeq seq);

AjPList       embDmxScophitReadAllFasta(AjPFile in);

AjBool        embDmxHitlistToScophits(const AjPList in, 
				      AjPList out);

AjBool        embDmxScophitsToHitlist(const AjPList in, 
				      EmbPHitlist *out, 
				      AjIList *iter);

AjBool        embDmxScophitToHit(EmbPHit *to, 
				 const AjPScophit from);

AjBool        embDmxScophitsAccToHitlist(const AjPList in, 
					 EmbPHitlist *out,   
					 AjIList *iter);

AjBool        embDmxHitsWrite(AjPFile outf,
			      EmbPHitlist hits, 
			      ajint maxhits);

AjBool        embDmxScopToScophit(const AjPScop source, 
				  AjPScophit* target);

AjBool        embDmxScopalgToScop(const AjPScopalg align,
				  AjPScop const *scop_arr, 
				  ajint scop_dim, 
				  AjPList* list);

AjBool        embDmxScophitsOverlap(const AjPScophit h1,
				    const AjPScophit h2, 
				    ajuint n);

AjBool        embDmxScophitsOverlapAcc(const AjPScophit h1,
				       const AjPScophit h2, 
				       ajuint n);

AjPScophit    embDmxScophitMerge(const AjPScophit hit1, 
				 const AjPScophit hit2);

AjBool        embDmxScophitMergeInsertThis(const AjPList list,
					   AjPScophit hit1, 
					   AjPScophit hit2,
					   AjIList iter);

AjBool        embDmxScophitMergeInsertThisTarget(const AjPList list, 
						 AjPScophit hit1, 
						 AjPScophit hit2,  
						 AjIList iter);

AjBool        embDmxScophitMergeInsertThisTargetBoth(const AjPList list, 
						     AjPScophit hit1, 
						     AjPScophit hit2,  
						     AjIList iter);

AjBool        embDmxScophitMergeInsertOther(AjPList list,
					    AjPScophit hit1, 
					    AjPScophit hit2);

AjBool        embDmxScophitMergeInsertOtherTargetBoth(AjPList list, 
						      AjPScophit hit1, 
						      AjPScophit hit2);

AjBool        embDmxScophitMergeInsertOtherTarget(AjPList list, 
						  AjPScophit hit1, 
						  AjPScophit hit2);

AjBool        embDmxSeqNR(const AjPList input, 
			  AjPUint *keep, 
			  ajint *nset,
			  const AjPMatrixf matrix,
			  float gapopen, 
			  float gapextend,
			  float thresh, 
			  AjBool CheckGarbage);

AjBool        embDmxSeqNRRange(const AjPList input, 
			       AjPUint *keep, 
			       ajint *nset,
			       const AjPMatrixf matrix, 
			       float gapopen, 
			       float gapextend,
			       float thresh1, 
			       float thresh2, 
			       AjBool CheckGarbage);

AjBool        embDmxSeqCompall(const AjPList input, 
			       AjPFloat2d *scores, 
			       const AjPMatrixf matrix, 
			       float gapopen, 
			       float gapextend);

/*
** End of prototype definitions
*/

#endif

#ifdef __cplusplus
}
#endif
