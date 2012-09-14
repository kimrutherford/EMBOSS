/* @include embest ************************************************************
**
** NUCLEUS EST alignment functions
**
** @author Copyright (C) 1996 Richard Mott
** @author Copyright (C) 1998 Peter Rice revised for EMBOSS
** @version $Revision: 1.13 $
** @@
**
** This library is free software; you can redistribute it and/or
** modify it under the terms of the GNU Lesser General Public
** License as published by the Free Software Foundation; either
** version 2.1 of the License, or (at your option) any later version.
**
** This library is distributed in the hope that it will be useful,
** but WITHOUT ANY WARRANTY; without even the implied warranty of
** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
** Lesser General Public License for more details.
**
** You should have received a copy of the GNU Lesser General Public
** License along with this library; if not, write to the Free Software
** Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,
** MA  02110-1301,  USA.
**
******************************************************************************/

#ifndef EMBEST_H
#define EMBEST_H

/* ========================================================================= */
/* ============================= include files ============================= */
/* ========================================================================= */

#include "ajdefine.h"
#include "ajseqdata.h"

AJ_BEGIN_DECLS




/* ========================================================================= */
/* =============================== constants =============================== */
/* ========================================================================= */




/* Definition of the padding-character in CAF */

#define padding_char '-'




/* ========================================================================= */
/* ============================== public data ============================== */
/* ========================================================================= */




/* @data hash_list ************************************************************
**
** NUCLEUS internal data structure for est2genome EMBOSS application
** to maintain internal hash lists.
**
** @attr name [char*] Name
** @attr offset [unsigned long] Offset
** @attr text_offset [unsigned long] Text offset
** @attr next [struct hash_list*] Next in list
** @@
******************************************************************************/

typedef struct hash_list
{
  char *name;
  unsigned long offset;
  unsigned long text_offset;
  struct hash_list *next;
}
HASH_LIST;




/* @enum directions ***********************************************************
**
** Path matrix cell type and direction
**
** @value INTRON Intron
** @value DIAGONAL Diagonal in path
** @value DELETE_EST Delete in EST sequence
** @value DELETE_GENOME Delete in genome sequence
** @value FORWARD_SPLICED_INTRON Forward spliced intron
** @value REVERSE_SPLICED_INTRON Reverse spliced intron
******************************************************************************/

typedef enum { INTRON=0, DIAGONAL=1, DELETE_EST=2, DELETE_GENOME=3,
	       FORWARD_SPLICED_INTRON=-1, REVERSE_SPLICED_INTRON=-2
} directions;




/* @enum donor_acceptor *******************************************************
**
** Donors and acceptors for slice site junctions
**
** @value NOT_A_SITE Not a splice site
** @value DONOR      Donor site
** @value ACCEPTOR   Acceptor site
******************************************************************************/

typedef enum { NOT_A_SITE=1, DONOR=2, ACCEPTOR=4 } donor_acceptor;




/* @data EmbPEstAlign *********************************************************
**
** NUCLEUS data structure for EST alignments (originally for est2genome)
**
** @attr gstart [ajint] Genomic start
** @attr estart [ajint] EST start
** @attr gstop [ajint] Genomic stop
** @attr estop [ajint] EST stop
** @attr score [ajint] Score
** @attr len [ajint] Length
** @attr align_path [ajint*] Path
** @@
******************************************************************************/

typedef struct EmbSEstAlign
{
  ajint gstart;
  ajint estart;
  ajint gstop;
  ajint estop;
  ajint score;
  ajint len;
  ajint *align_path;
} EmbOEstAlign;
#define EmbPEstAlign EmbOEstAlign*




enum base_types /* just defines a, c, g, t as 0-3, for indexing purposes. */
{
  base_a, base_c, base_g, base_t, base_n, base_i, base_o, nucleotides, anybase
};
/* Definitions for nucleotides */



#define MINUS_INFINITY -10000000




/* ========================================================================= */
/* =========================== public functions ============================ */
/* ========================================================================= */



/*
** Prototype definitions
*/

EmbPEstAlign embEstAlignNonRecursive ( const AjPSeq est, const AjPSeq genome,
				       ajint gap_penalty, ajint intron_penalty,
				       ajint splice_penalty,
				       const AjPSeq splice_sites,
				       ajint backtrack, ajint needleman,
				       ajint init_path );

EmbPEstAlign embEstAlignLinearSpace ( const AjPSeq est, const AjPSeq genome,
				      ajint match, ajint mismatch,
				      ajint gap_penalty, ajint intron_penalty,
				      ajint splice_penalty,
				      const AjPSeq splice_sites,
				      float max_area );

AjPSeq       embEstFindSpliceSites( const AjPSeq genome, ajint direction );
void         embEstFreeAlign( EmbPEstAlign *ge );
ajint        embEstGetSeed (void);
void         embEstMatInit (ajint match, ajint mismatch, ajint gap,
			    ajint neutral, char pad_char);
void         embEstOutBlastStyle ( AjPFile ofile,
				  const AjPSeq genome, const AjPSeq est,
				  const EmbPEstAlign ge, ajint gap_penalty,
				  ajint intron_penalty,
				  ajint splice_penalty,
				  ajint gapped, ajint reverse  );

void         embEstPrintAlign( AjPFile ofile,
			      const AjPSeq genome, const AjPSeq est,
			      const EmbPEstAlign ge, ajint width );
void         embEstSetDebug (void);
void         embEstSetVerbose (void);
AjPSeq       embEstShuffleSeq( AjPSeq seq, ajint in_place, ajint *seed );

/*
** End of prototype definitions
*/

AJ_END_DECLS

#endif  /* !EMBEST_H */
