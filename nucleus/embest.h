#ifdef __cplusplus
extern "C"
{
#endif

#ifndef embest_h
#define embest_h

/* Definition of the padding-character in CAF */

#define padding_char '-'




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




typedef enum { INTRON=0, DIAGONAL=1, DELETE_EST=2, DELETE_GENOME=3,
	       FORWARD_SPLICED_INTRON=-1, REVERSE_SPLICED_INTRON=-2
} directions;
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

#endif

#ifdef __cplusplus
}
#endif
