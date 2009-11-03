#ifdef __cplusplus
extern "C"
{
#endif

#ifndef embmat_h
#define embmat_h

#define PRINTS_MAT "PRINTS/prints.mat"

typedef ajuint *PMAT_INT[26];




/* @data EmbPMatPrints ********************************************************
**
** NUCLEUS data structure for PRINTS protein fingerprints
**
** @attr cod [AjPStr] gc line
** @attr acc [AjPStr] gx line
** @attr tit [AjPStr] gt line
** @attr len [ajuint*] Lengths of motifs
** @attr thresh [ajuint*] % of maximum score for matrix
** @attr max [ajuint*] Maximum score for matrix
** @attr matrix [PMAT_INT*] Matrices
** @attr n [ajuint] Number of motifs in fingerprint
** @attr Padding [char[4]] Padding to alignment boundary
** @@
******************************************************************************/

typedef struct EmbSMatPrints
{
    AjPStr cod;
    AjPStr acc;
    AjPStr tit;
    ajuint    *len;
    ajuint    *thresh;
    ajuint    *max;
    PMAT_INT  *matrix;
    ajuint    n;
    char      Padding[4];
} EmbOMatPrint;
#define EmbPMatPrints EmbOMatPrint*




/* @data EmbPMatMatch *********************************************************
**
** NUCLEUS data structure for sequence matrix matches
**
** @attr seqname [AjPStr] Sequence name
** @attr cod [AjPStr] Matrix name
** @attr acc [AjPStr] Matrix accession number
** @attr tit [AjPStr] Matrix title
** @attr pat [AjPStr] Pattern
** @attr n [ajuint] Number of motifs in fingerprint
** @attr len [ajuint] Lengths of motifs
** @attr thresh [ajuint] % of maximum score for matrix
** @attr max [ajuint] Maximum score for matrix
** @attr element [ajuint] Number of matching element
** @attr start [ajuint] Start of match
** @attr end [ajuint] End of match
** @attr score [ajuint] Score of match
** @attr hpe [ajuint] Hits per element (so far)
** @attr hpm [ajuint] Hits per motif (so far)
** @attr all [AjBool] Can be set if all elements match
** @attr ordered [AjBool] Can be set if "all" and in order
** @attr forward [AjBool] on forward strand
** @attr mm [ajuint] Number of mismatches
** @attr cut1 [ajint] Undocumented
** @attr cut2 [ajint] Undocumented
** @attr cut3 [ajint] Undocumented
** @attr cut4 [ajint] Undocumented
** @attr circ12 [AjBool] Circular for cut1 and/or cut2
** @attr circ34 [AjBool] Circular for cut3 and/or cut4
** @attr iso [AjPStr] Holds names of isoschizomers
** @@
******************************************************************************/

typedef struct EmbSMatMatch
{
    AjPStr seqname;
    AjPStr cod;
    AjPStr acc;
    AjPStr tit;
    AjPStr pat;
    ajuint  n;
    ajuint  len;
    ajuint  thresh;
    ajuint  max;
    ajuint  element;
    ajuint  start;
    ajuint  end;
    ajuint  score;
    ajuint  hpe;
    ajuint  hpm;
    AjBool all;
    AjBool ordered;
    AjBool forward;
    ajuint  mm;
    ajint  cut1;
    ajint  cut2;
    ajint  cut3;
    ajint  cut4;
    AjBool circ12;
    AjBool circ34;
    AjPStr iso;
} EmbOMatMatch;
#define EmbPMatMatch EmbOMatMatch*




/*
** Prototype definitions
*/

void   embMatMatchDel (EmbPMatMatch *s);
void   embMatPrintsInit (AjPFile *fp);
void   embMatProtDelInt (EmbPMatPrints *s);
EmbPMatPrints embMatProtReadInt (AjPFile fp);
ajuint    embMatProtScanInt (const AjPStr s, const AjPStr n,
			    const EmbPMatPrints m, AjPList *l,
			    AjBool *all, AjBool *ordered, AjBool overlap);

/*
** End of prototype definitions
*/

#endif

#ifdef __cplusplus
}
#endif
