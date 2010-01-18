/* @source edialign application
**
** Local multiple alignment
**
** @author Burkhard Morgenstern and Said Abdeddaim
** @modified Alan Bleasby (ajb@ebi.ac.uk) EMBOSS port based on ACD
**  from Guy Bottu
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
#include <math.h>


#define	edmin(a,b)	((a)<(b)?(a):(b))
#define	edmax(a,b)	((a)>(b)?(a):(b))

#define	TAILLE_MAX_LIGNE_FICHIER	10000

#define PAPER_WIDTH      80 
#define MLINE 1000 
#define MAX_REGEX 1000 
#define NAME_LEN 1000   
#define SEQ_NAME_LEN 12   
#define MAX_SEQNUM 10000
#define MAX_ITNUM 3 
#define MAX_INPUT_LINE 10000 
#define MIN_MOT_WGT 0.1 
#define MAX_CSC 10 

/* Default parameters */

#define BETA              0
#define WEB               0
#define OVERLAP_THRESHOLD 35
#define MIN_DIA           1
#define MAX_DIA          40
#define MATNAME          "BLOSUM"     
#define WEAK_WGT_TYPE_THR      0.5 
#define STRONG_WGT_TYPE_THR    0.75 




#define INT16 short int
#ifdef __alpha
#define INT32 int
#else
#define INT32 long int
#endif
#define REAL32 float
#define REAL64 double

/*
#ifndef MAC
#define Malloc malloc
#define Calloc calloc
#define Realloc realloc
#define Free free
#endif
*/

#define STATUS 0x10000002


/* Globals */
FILE *fp_dia, *fp_dpa, *fp_frg , *fp_mot ; 
struct multi_frag *anchor_frg ;

ajint col_score = 0; 
ajint char_num[ MAX_REGEX ] ;
char *mot_char[ MAX_REGEX ] ; 
ajint regex_len , mot_len = 0 ; 


clock_t beg_pa , end_pa , beg_ali , end_ali , beg_ts , end_ts ;
float time_diff_pa , time_diff_ali , perc_pa_time , time_diff_srt ; 
float total_pa_time = 0 ; 


float mot_factor , mot_offset_factor , max_mot_offset ; 

ajint wgt_type_plot = 0 , motifs = 0 ; 
ajint bubblesort = 0 , cd_gobics = 0 ; 
ajint nas = 0 , ref_seq = 0;
ajuint i_max ; 
ajint speed_optimized = 0 ; 
ajint online = 0 ; 
ajint time_stamps = 0 ; 
ajint break1 = 0 ; 
ajint break2 = 0 ; 
ajint wgt_print = 0 ; 
ajint wgt_print_x = 0 ; 
short max_itnum = MAX_ITNUM ; 
ajint quali_num = 1 ; 
ajint wgt_plot = 0 ; 
ajint self_comparison = 0;
short exclude_frg = 0; 
ajint ***gl_exclude_list ;
ajint max_sim_score = -2000 ; 
ajint sf_mat = 0 ; 
char nuc1, nuc2, nuc3 ;
short crick_strand = 0;
ajint gl_frg_count = 0; 
ajint dna_speed = 0;
char pst_name[NAME_LEN];
ajint cont_it = 1 , wgt_type = 0  ;
ajint mask = 0, strict = 0 , textual_alignment = 1;
char prn[ NAME_LEN ] ;
ajint redundant, print_max_nd = 1;
ajint lmax = MAX_DIA;
char **arguments;
ajint  pr_av_nd = 0, pr_av_max_nd ;
char input_line[ NAME_LEN ];
char input_parameters[ NAME_LEN ];
ajint print_status = 0 ;
char clust_sim[NAME_LEN] ;
float tot_weight = 0, av_len;
ajint anchors = 0;
ajint pa_only = 0;
ajint dia_num = 0;
ajint max_dia_num = 0;
float av_dia_num = 0;
float av_max_dia_num = 0;
ajint afc_file = 0;
ajint afc_filex = 0;
ajint dia_pa_file = 0;
ajint frag_file = 0;
ajuint argnum;
ajint standard_out = 0;
ajint plot_num = 4 ;
ajint default_name = 1;
ajint fasta_file = 0;
ajint cw_file = 0; 
ajint msf_file = 0;
char *upg_str = NULL;
ajint dcount = 0;


ajint **shift = NULL; 
ajint   thr_sim_score = 4 ;

char **seq = NULL;
/*char *seq[MAX_SEQNUM];*/   /* sequences */


char *newseq[MAX_SEQNUM];   /* sequences */
ajint sim_score[21][21];  /* similarity matrix */
float av_sim_score_pep ;
float av_sim_score_nuc ;
float **glob_sim = NULL;        /* overall similarity between any two sequences */
float **wgt_prot = NULL ;      /* `weight' of diagonals */
float **wgt_dna = NULL  ;      /* `weight' of diagonals */
float **wgt_trans = NULL;      /* `weight' of diagonals */
float **min_weight = NULL;      /* `weight' of diagonals */
ajint min_dia = MIN_DIA ;             /* minimum length of diagonals */
ajint max_dia = MAX_DIA ;  /* maximum length of diagonals */
ajint iter_cond_prob = 0;
ajint *seqlen;                /* lengths of sequences */

char **full_name = NULL;
/*char *full_name[MAX_SEQNUM] ;*/


float **pair_score = NULL;
short **cont_it_p = NULL; 
float score;
ajint maxlen;              /* maximum length of sequences */
ajuint seqnum;              /* number of sequences */
ajint *num_dia_bf = NULL;   /* num_dia_bf[ istep ] = number of diagonals from
                            all pairwise alignments BEFORE FILTER
                            PROCEDURE in iteration step `istep' */     
ajint *num_dia_af = NULL;   /* num_dia_af[istep] = number of diagonals from
                            all pairwise alignments AFTER FILTER 
                            PROCEDURE in iteration step `it' */     
ajint num_dia_anc;         /* number of diagonals definde by anchored 
                            regions */
ajint num_all_it_dia = 0;  /* total number of diagonals in multiple alignment 
                            in all iteration steps */
float weight_sum_bf;     /* sum of weights  of diagonals in multiple 
                            alignment before filter procedure */  
float weight_sum_af;     /* sum of weights  of diagonals in multiple 
                            alignment after fliter procedure*/
float threshold = 0.0 ;  /* threshold T */
ajuint num_dia_p;           /* number of diagonals in pairwise alignment */ 
ajint long_output = 0;     /* if long_output = 1, a log-file is produced.  */   
ajint frg_mult_file = 0 ; 
ajint frg_mult_file_v = 0 ; 
ajint overlap_weights = 1 ;  
ajint ow_force = 0 ;
ajint anc_num = 0;          /* number of anchored regions 
                            (specified in file *.anc) */
ajint par_count;           /* number of parameters       */
float pairalignsum;      /* sum of weights in pairwise alignment */ 
ajint pairalignlen;        /* sum of aligned residues in pairwise alignment */
char amino_acid[22];
ajint istep;  
struct multi_frag         /* pointer to first diagonal in multiple alignment */
      *this_it_dia;       /* in current iteration step */  
struct multi_frag         /* pointer to first diagonal in multiple alignment */
      *all_it_dia = NULL; /* in all iteration step */
struct multi_frag *end_dia;  
                         /* pointer to last diagonal in multiple alignment */

char par_dir[NAME_LEN];

char **seq_name = NULL;

/*char *seq_name[MAX_SEQNUM];*/

char mat_name[NAME_LEN];     /* name of file containing similarity matrix */
char mat_name_p[NAME_LEN];
char anc_name[NAME_LEN];  /* anchored regions */
char seq_file[NAME_LEN];
char input_name[NAME_LEN];
char tmp_str[NAME_LEN];
char output_name[NAME_LEN];
char printname[NAME_LEN];
char gl_mot_regex[MAX_REGEX] ; 

char *par_file;

short **mot_pos ;       /* positions of pre-defined motifs */ 

ajint **amino = NULL;           /* amino acid residues in protein sequences or 
                          translated DNA sequences, respective */

ajint **amino_c = NULL;         /* amino acid residues on crick strand */ 
 


ajint ***open_pos;           /* open_pos[i][j][p] = 1, if the p-th residue of 
                          sequence i is not yet directly (by one diagonal) 
                          aligned with any residue of sequence j and 
                          open_pos[i][j][r] = 0 otherwise. So, at the
                          beginning of the first iteration step, all values 
                          are 1. In the subsequent iteration steps,
                          only those parts of the sequence are considered,  
                          that are not yet aligned. */     

  
struct multi_frag *pair_dia;   /* diagonals in pairwise alignemnt */


double **tp400_prot =NULL;    /* propability distribution for sums of similarity
                       socores in diagonals occurring in comparison matrix
                       (by random experiments and approximation  */

double **tp400_dna =NULL;    /* propability distribution for sums of similarity
                       socores in diagonals occurring in comparison matrix
                       (by random experiments and approximation  */

double **tp400_trans =NULL;    /* propability distribution for sums of similarity
                       socores in diagonals occurring in comparison matrix
                       (by random experiments and approximation  */


char dia_pa_name[NAME_LEN];
char frag_file_name[NAME_LEN];
char mot_file_name[NAME_LEN];

ajint lgs_option = 0; 
float sf_mat_thr = 0;



/* @datastatic edialignPositionSet ********************************************
**
** Dialign positionset structure
**
** @attr pos [ajint*] Positions array
** @attr nbr [ajint] Size of position array
** @attr Padding [char[4]] Padding to alignment boundary
******************************************************************************/

typedef struct
{
    ajint *pos;
    ajint nbr;
    char Padding[4];
} edialignPositionSet;



/* @datastatic edialignSequence ***********************************************
**
** Dialign sequence structure
**
** @attr longueur       [ajint]   Length
** @attr Padding        [char[4]] Padding to alignment boundary
** @attr aligSetNbr     [ajint*]  Numbers of sets
** @attr predAligSetPos [ajint*]  Predicted alignment set positions
** @attr succAligSetPos [ajint*]  Successful alignment set positions
******************************************************************************/


typedef struct {
    ajint longueur;
    char  Padding[4];
    ajint *aligSetNbr;
    ajint *predAligSetPos;
    ajint *succAligSetPos;
} edialignSequence;




/* @datastatic edialignCLOSURE ************************************************
**
** Dialign closure structure
**
** @attr seq [edialignSequence*] Sequences
** @attr seqNbr [ajint] Numbers of sequences
** @attr maxLong [ajint] Maximum long
** @attr aligSet [edialignPositionSet*] Alignment sets
** @attr nbrAligSets [ajint] Number of alignment sets
** @attr oldNbrAligSets [ajint] Old number of alignment sets
** @attr predFrontier [ajint**] Predicted boundaries
** @attr succFrontier [ajint**] Successful boundaries
** @attr topolog [ajint*] Topologies
** @attr gauche1 [ajint*] Left end in 1
** @attr gauche2 [ajint*] Left end in 2
** @attr droite1 [ajint*] Right end in 1
** @attr droite2 [ajint*] Right end in 2
** @attr pos_ [ajint**] Positions
******************************************************************************/


typedef struct {

    edialignSequence *seq;
    ajint seqNbr;
    ajint maxLong;

    edialignPositionSet *aligSet;
    ajint nbrAligSets;
    ajint oldNbrAligSets;

    ajint **predFrontier;
    ajint **succFrontier;

    ajint *topolog;
    ajint *gauche1;
    ajint *gauche2;
    ajint *droite1;
    ajint *droite2;
    ajint **pos_;

} edialignCLOSURE;




/* 
   fragments in function `pairalign' 
   
   b1, b2:    begin of the diagonal
   ext:       length of the diagonal
   weight:    weight of the diagonal
   prec:      preceding diagonal in dot matrix
   last:      last diagonal ending in the same column 
   sum:       sum of weights accumulated  
   cs:        crick strand 
   trans:     translation
*/ 

struct pair_frag
{
    ajint b1;
    ajint b2;
    ajint ext;
    float weight;
    struct pair_frag *prec;
    struct pair_frag *last;
    float sum;
    short trans;
    short cs;
};




/*
   fragments outside function `pairalign' 
   
   b[0], b[1]:  begin of the diagonal
   s[0], s[1]:  sequences, to which diagonal belongs
   ext:         length of the diagonal
   weight:      individual weight of the diagonal
   ow:          overlap weight of the diagonal
   sel:         1, if accepted in filter proces, 0 else
   trans:       translation
   cs:          crick strand 
   Padding:     padding to alignment boundary
   it:          iteration step 
   *next:       next diagonal 
   */

struct multi_frag
{
    ajint b[2];
    ajint s[2];
    ajint ext;
    ajint it;
    float weight;
    float ow;
    short sel;
    short trans;
    short cs;
    short Padding;
    struct multi_frag *next;
    struct multi_frag *pred;
};




struct leaf
{
    ajint s1;
    ajint s2;
    ajint clade;
};




struct seq_pair
{
    ajint s1;
    ajint s2;
    float weight;
};      




struct subtree
{
    ajint member_num;
    ajint valid;
    ajint *member;
    char *name;
    float depth;
    char Padding[4];
};         




char DEBUG=0;
edialignCLOSURE *gabiosclos;         /* closure data structure for GABIOS-LIB */



static void **edialign_callouer_mat(size_t t_elt, size_t nb_lig,
				    size_t nb_col);
static void *edialign_allouer(size_t taille);
static void *edialign_reallouer(void *pointeur, size_t taille);
static void edialign_liberer(void *pointeur);
static void edialign_liberer_mat(void **pointeur, size_t nb_lig);
static void edialign_realloc_closure(edialignCLOSURE *clos);
static void edialign_free_closure(edialignCLOSURE *clos);
static void edialign_freeAligGraphClosure(edialignCLOSURE *clos);
static void **edialign_recallouer_mat(void **pointeur, size_t t_elt,
				      size_t anc_nb_lig, 
				      size_t nb_lig, size_t nb_col);
static void edialign_desinit_seq(edialignCLOSURE *clos);
static void edialign_erreur(const char *message);
static ajint edialign_word_count(char *str);
static void edialign_rel_wgt_calc(ajint l1, ajint l2, float **rel_wgt);
static void edialign_wgt_prnt_prot(void);
static ajint edialign_mini2(ajint a, ajint b);
static ajuint edialign_minu2(ajint a, ajint b);
static ajint edialign_mini3(ajint a, ajint b, ajint c);
static float edialign_mot_dist_factor(ajint offset , float parameter);
static float edialign_maxf2(float a, float b);
static void edialign_wgt_prnt(void);
static void edialign_regex_parse(char *mot_regex);
static void edialign_seq_parse(char *mot_regex_unused);
static void edialign_seq_shift(void);
static void edialign_matrix_read(FILE *fp_mat);
static void edialign_mem_alloc(void);
static ajint edialign_multi_anc_read(char *file_name);
static void edialign_exclude_frg_read( char *file_name , int ***exclude_list);
static void edialign_tp400_read(ajint w_type, double **pr_ptr);
static edialignCLOSURE *edialign_newAligGraphClosure(ajint nbreseq,
						     ajint *longseq,
						     ajint nbreancr,
						     ajint **ancrages);
static ajint edialign_translate(char c1, char c2 ,char c3, ajint seqno,
				ajint pos);

static char edialign_invert(char c1);
static void edialign_ow_bubble_sort( int number , struct multi_frag *dp );
static void edialign_frag_sort(ajint number , struct multi_frag *dp ,
			       ajint olw ); 
static void edialign_filter(ajint *number, struct multi_frag *diagonal);
static void edialign_para_print( char *s_f, FILE *fpi );

static float edialign_frag_chain(ajint n1, ajint n2, FILE *fp1, FILE *fp_m,
				 ajuint *number);
static void edialign_ow_add( struct multi_frag *sm1 , struct multi_frag *sm2 );
static void edialign_print_log(struct multi_frag *d,FILE *fp_l,FILE *fp_fs);
static void edialign_print_fragments(struct multi_frag *d , FILE *fp_ff2 );
static void edialign_throw_out( float *weight_sum );
static void edialign_sel_test(void);
static void edialign_av_tree_print(void);
static void edialign_subst_mat( char *file_name, int fragno ,
			       struct multi_frag *frg );
static void edialign_ali_arrange(ajint ifragno , struct multi_frag *d,
				 FILE *fp, AjPSeqout seqout, FILE *fp3 ,
				 FILE *fp4 ,
				 FILE *fp_col_score,AjBool isprot);
static void edialign_bubble_sort(ajint number, struct multi_frag *dp);




#if 0
static void regex_format_complain(void);
#endif



/* @prog edialign *************************************************************
**
** Local multiple alignment
**
******************************************************************************/

int main(int argc, char **argv)
{
    ajuint k;
    ajint dia_counter;
    ajint tmpi1;
    ajint tmpi2 ;

    struct multi_frag *current_dia;  
    struct multi_frag *diagonal1;  
    struct multi_frag *diagonal2;  

    /* pointers to diagonals in multiple alignment */ 

    char str[NAME_LEN], dist_name[NAME_LEN]; 
    char par_str[NAME_LEN];  
    char *char_ptr;
    char prn2[NAME_LEN];
    char logname[NAME_LEN];
    char fsm_name[NAME_LEN];
    char dia_name[NAME_LEN];
    char csc_name[NAME_LEN];
    char itname[NAME_LEN], itname2[NAME_LEN], itname3[NAME_LEN];
    char itname4[NAME_LEN];
    char dialign_dir[NAME_LEN];

    ajuint i;
    ajuint j;
    ajuint n;
    ajint len;
    
    ajuint hv;

    ajint ii;

    FILE *fp_ali = NULL;
/*    FILE *fp2 = NULL;*/
    FILE *fp3 = NULL;
    FILE *fp4 = NULL;
    FILE *fp_log = NULL;
    FILE *fp_fsm = NULL;
    FILE *fp_st;
    FILE *fp_csc = NULL; 
    FILE *fp_matrix = NULL;	       /* file containing similarity matrix */

    AjPFile matfp = NULL;
    AjPSeqset seqset = NULL;
    const AjPSeq pseq;
    const AjPStr sname = NULL;
    AjPStr nucmode;
    AjBool revcomp;
    AjPStr overlapw;
    AjPStr linkage;
    AjPFile outfile = NULL;
    AjBool isprot;
    ajuint s_len;
    char s_str[SEQ_NAME_LEN+3];
    AjPSeqout seqout = NULL;
    AjPStr tnstr = NULL;
    
    struct multi_frag *next_dia;

    embInit("edialign", argc, argv);

    tnstr = ajStrNew();

    seqset    = ajAcdGetSeqset("sequences");
    nucmode   = ajAcdGetListSingle("nucmode");
    revcomp   = ajAcdGetBoolean("revcomp");
    overlapw  = ajAcdGetSelectSingle("overlapw");
    linkage   = ajAcdGetListSingle("linkage");
    lmax      = ajAcdGetInt("maxfragl");
    dna_speed = !!ajAcdGetBoolean("fragmat");
    
    thr_sim_score  = ajAcdGetInt("fragsim");
    iter_cond_prob = !!ajAcdGetBoolean("itscore");
    
    threshold = ajAcdGetFloat("threshold");
    mask      = !!ajAcdGetBoolean("mask");
    plot_num  = ajAcdGetInt("starnum");

    quali_num = !ajAcdGetBoolean("dostars");

    outfile   = ajAcdGetOutfile("outfile");
    seqout    = ajAcdGetSeqoutall("outseq");

    ajSeqsetFmtUpper(seqset);
    n = ajSeqsetGetSize(seqset);

    max_dia = lmax;
    
    AJCNEW0(seq,n);
    AJCNEW0(seq_name,n);
    AJCNEW0(full_name,n);
    AJCNEW0(seqlen,n+1);

    isprot = ajFalse;
    
    for(i=0;i<n;++i)
    {
	pseq = ajSeqsetGetseqSeq(seqset,i);
	len = ajSeqGetLen(pseq);
	seqlen[i] = len;
	if(ajSeqIsProt(pseq))
	    isprot = ajTrue;

	AJCNEW(seq[i],len+2);		/* room to shift */
	strcpy(seq[i],ajSeqGetSeqC(pseq));
	
	sname = ajSeqsetGetseqNameS(seqset,i);
	len = ajStrGetLen(sname);
	AJCNEW(seq_name[i],SEQ_NAME_LEN+3);
	AJCNEW(full_name[i],len+1);

	s_len = ajStrGetLen(sname);
	for(j=0; j < SEQ_NAME_LEN; ++j)  
	    if(j < s_len) 
		s_str[j] =  *(ajStrGetPtr(sname) + j);
	    else 
		s_str[j] =  ' ';
	
	s_str[ SEQ_NAME_LEN ] = '\0';




	strcpy(seq_name[i],s_str);
	strcpy(full_name[i],ajStrGetPtr(sname));
    }

    seqnum = n;
    
    if(!isprot)
    {
	if(ajStrMatchC(nucmode,"n"))
	    wgt_type = 1;
	else if(ajStrMatchC(nucmode,"nt"))
	    wgt_type = 2;
	else if(ajStrMatchC(nucmode,"ma"))
	    wgt_type = 3;
    }
    

    crick_strand = !!revcomp;

    if(ajStrMatchC(overlapw,"yes"))
	overlap_weights = 0;
    else if(ajStrMatchC(overlapw,"no"))
	ow_force = 1;

    if(ajStrMatchC(linkage,"max"))
	strcpy(clust_sim,"max");
    else if(ajStrMatchC(linkage,"min"))
	strcpy(clust_sim,"min");
    else if(ajStrMatchC(linkage,"UPGMA"))
	strcpy(clust_sim, "av");    

    matfp = ajDatafileNewInNameC("edialignmat");

    if(!matfp)
	ajFatal("edialignmat matrix file not found\n");


    strcpy(mat_name,MATNAME);

 
    par_file = (char *) calloc((size_t) NAME_LEN , sizeof(char) );


    if( time_stamps ) 
	beg_ali = clock() ;

    strcpy ( dialign_dir , "DIALIGN2_DIR" );

/*
    if ((par_file = getenv(dialign_dir)) == NULL)
    {
	printf("\n \n \n    Please set the environmentvariable "
	       "DIALIGN2_DIR \n");
	printf("    as described in the README file \n"); 
	embExitBad();
    }
*/
    argnum = argc;
/*
    strcpy( par_dir , par_file );
*/


/*
    if(argc == 1)
    {
	printf("\n    usage: %s [ options ] <seq_file> \n\n", argv[0] );
	printf("    <seq_file> contains input sequences in FASTA format.\n"); 
	printf("    Per default, sequences are assumed to be protein "
	       "sequences.\n" ) ;
	printf("    For DNA alignment, please use one of these options: \n\n");
	printf("     -n    DNA sequences; similarity calculated at the "
	       "nucleotide level \n\n"); 
	printf("     -nt   DNA sequences; similarity calculated at the "
	       "peptide level\n");
	printf("           (by translation using the genetic code) \n\n");
	printf("     -lgs  long genomic sequences: Both nucleotide and "
	       "peptide\n");
	printf("           similarities calculated \n\n");  
	printf("    Many more options are available, please consult the \n");
	printf("    DIALIGN USER_GUIDE that should come with the DIALIGN "
	       "package.\n");
	printf("    For more information on DIALIGN, please visit the "
	       "DIALIGN\n"); 
	printf("    home page at BiBiServ (Bielefeld Bioinformatic "
	       "Server): \n\n") ;
	printf("        http://bibiserv.techfak.uni-bielefeld.de/"
	       "dialign/ \n\n");    
	embExitBad() ;
    }
*/

    arguments = ( char ** ) calloc( argnum , sizeof ( char * ) );

    for( i = 0 ; i < argnum ; i++ )
    {
	arguments[i] = ( char *)  calloc( NAME_LEN , sizeof (char) );
	strcpy( arguments[i] , argv[i] );
    }
 


    sname = ajSeqsetGetFilename(seqset);

    strcpy( input_name , ajStrGetPtr(sname));
  
    threshold = 0.0 ;


/*
    edialign_para_read( argnum , arguments );
*/

    if( ( textual_alignment == 0 ) && ( col_score == 1 ) )
    { 
	printf("\n\n   Option -csc makes sense only if \"textual alignment\"");
	printf(" is produced. \n");
	printf("   This can be enforced with option -ta \n\n");
	printf("   program terminated \n\n\n");
	embExitBad() ;
    } 


    if( cd_gobics )
    {
	strcpy( input_line , "program parameters:  " ) ; 
	for( i = 1 ; i < ( argnum -1 ) ; i++ ) {
	    strcat( input_line , argv[i] );
	    strcat( input_line , " " );
	}
    }
    else
    {
	strcpy( input_line , "program call:  " ) ; 
	for( i = 0 ; i < argnum ; i++ ) {
	    strcat( input_line , argv[i] );
	    strcat( input_line , " " );
	}
    }


    if ( wgt_type > 0 )  
	strict = 1 ; 

    strcpy( seq_file , input_name );

    if(
       ( ! strcmp( input_name + strlen( input_name ) - 4 , ".seq" ) )
       || ( ! strcmp( input_name + strlen( input_name ) - 3 , ".fa" ) )
       || ( ! strcmp( input_name + strlen( input_name ) - 6 , ".fasta" ) )
       )
	if( ( char_ptr = strrchr(input_name,'.') ) != NULL)
	    *char_ptr = '\0';


    strcpy( anc_name , input_name );
    strcat( anc_name , ".anc" );
/*
    seqnum = edialign_seq_read( seq_file , seq , seq_name , full_name ) ;
*/
    if ( motifs )
	edialign_regex_parse( gl_mot_regex ) ; 


    if( ( seqnum == 2 ) && ( iter_cond_prob == 0 ) ) 
	max_itnum = 1 ; 

 
    if(  ( ow_force == 0 ) && ( seqnum > OVERLAP_THRESHOLD )  )
	overlap_weights = 0;
    if( seqnum == 2 )
	overlap_weights = 0;

    if( seqnum < 2 )
    { 
	if( cd_gobics )
	{
	    printf("\n\n         Something is wrong with your sequence file. "
		   "Maybe you entered a\n");
	    printf("         MS WORD or RFT file or your file contains only "
		   "one single sequence.\n");
	    printf("         Please note that our server only accepts plain "
		   "text files. \n\n");  
	    printf("         For more information, please consult our online "
		   "manual \n");
	    printf("         at the CHAOS/DIALIGN home page:\n\n");  
	    printf("             http://dialign.gobics.de/"
		   "chaos-dialign-manual");
	}

	else
	{
	    ajFatal("This application requires more than one sequence\n");
/*
	    printf("\n\n         Your sequence file containes only a single "
		   "sequence.\n");
	    printf("         Please make sure your input file contains at "
		   "least two sequences.\n\n");
	    printf("         For more information, please consult the online "
		   "manual \n");
	    printf("         at the DIALIGN home page: \n\n");
	    printf("             http://bibiserv.techfak.uni-bielefeld.de/"
		   "dialign/manual.html ");
*/
	}
/*


	printf("\n       \n \n \n \n");
	embExitBad();
*/
    }

    maxlen = 0;

  

    if( (pair_score = (float **) calloc( seqnum , sizeof(float *) )) == NULL)
    {       
	printf(" problems with memory allocation for `pair_score' !  \n \n");
	embExitBad();
    }

    for(i=0;i<seqnum;i++)
	if( (pair_score[i] = (float *) calloc( seqnum ,
					      sizeof(float) )) == NULL)
	{       
	    printf(" problems with memory allocation for "
		   "`pair_score' !  \n \n");
	    embExitBad();
	}

    if(( cont_it_p = (short **) calloc( seqnum , sizeof( short *))) == NULL )
    {
	printf(" problems with memory allocation for `cont_it_p ' !  \n \n");
	embExitBad();
    }

    for( i = 0 ; i < seqnum ; i++ )  
	if( (cont_it_p[i] = (short *) calloc( seqnum ,
					     sizeof(short) )) == NULL)
	{   
	    printf(" problems with memory allocation for "
		   "`cont_it_p' !  \n \n");
	    embExitBad();
	}

    for( i = 0 ; i < seqnum ; i++ )
	for( j = 0 ; j < seqnum ; j++ )
	    cont_it_p[i][j] = 1 ; 



    for( i = 0 ; i < seqnum ; i++ )
    {
	av_len = av_len + seqlen[i];

	if( seqlen[i] == 0 )
	{
	    printf("\n \n \n                       WARNING: \n \n");
	    printf("          Sequence %d contains no residues.\n",i+1);
	    printf("          Please inspect the sequence file.\n \n ");
	    printf("\n \n          Program terminated \n \n \n " );     

	    embExitBad();
	}
 
	if(maxlen < seqlen[i])
	    maxlen = seqlen[i];
    }


    av_len = av_len / seqnum;

    if ( motifs )
	edialign_seq_parse( gl_mot_regex ) ; 
  
    edialign_seq_shift();


    if( (glob_sim = 
	 (float **) calloc( seqnum , sizeof(float*))) == NULL) 
    {
	printf("Problems with memory allocation for glob_sim\n"); 
	embExitBad(); 
    } 

    for(i=0;i<seqnum;i++)
    {

	if( (glob_sim[i] = 
	     (float *) calloc( seqnum , sizeof(float))) == NULL) 
	{ 
	    printf("Problems with memory allocation for glob_sim \n"); 
	    embExitBad(); 
	} 
 
    }

    strcpy(par_str,"sdfsdf");

/*
    if( argc > 1 )
    {
	strcpy(str,par_dir);
	strcat(str,"/");
	strcat(str,mat_name);
	strcpy(mat_name_p,str);
   
	if( (fp_matrix = fopen(mat_name_p, "r")) == NULL)
	{


	    printf("\n\n Cannot find the file %s \n\n", mat_name );
	    printf(" Make sure the environment variable DIALIGN2_DIR "
		   "points\n");
	    printf(" to a directory containing the files \n\n");
	    printf("   BLOSUM \n   tp400_dna\n   tp400_prot \n   "
		   "tp400_trans \n\n" );
	    printf(" These files should be contained in the DIALIGN "
		   "package \n\n\n" ) ;
	    embExitBad() ;




	    printf("\n \n \n \n              ATTENTION ! \n \n");
	    printf("\n   There is no similarity matrix `%s'. \n", mat_name);
	    printf("   in the directory \n \n");
	    printf("           %s\n \n", par_dir);
	    embExitBad();
	}
    }
*/

/*
** This section had to be used if the VC++ /MT libs were used instead of
** the /MD ones
    ajStrAssignS(&tnstr,ajFileGetNameS(matfp));
    ajFileClose(&matfp);

    fp_matrix = fopen(ajStrGetPtr(tnstr),"rb");
*/

    fp_matrix = ajFileGetFileptr(matfp);

    if( wgt_type != 1 )
	edialign_matrix_read( fp_matrix );

    ajFileClose(&matfp);

    edialign_mem_alloc(  );


    if( wgt_type != 1 )  
	if( (amino = (int **) calloc( seqnum , sizeof(int *) ) ) == NULL)
	{
	    printf(" problems with memory allocation");
	    printf(" for `amino' !  \n \n");
	    embExitBad();
	}

    if( wgt_type != 1 )
	for( i = 0 ; i < seqnum ; i++ ) 
	    if((amino[i] = (int *) calloc((seqlen[i]+5), sizeof(int))) == NULL)
	    {
		printf(" problems with memory allocation");
		printf(" for `amino[%d]' !  \n \n", i);
		embExitBad();
	    }

 
    if( crick_strand )
    { 
	if( (amino_c = (int **) calloc( seqnum , sizeof(int *) ) ) == NULL)
	{
	    printf(" problems with memory allocation");
	    printf(" for `amino_c' !  \n \n");
	    embExitBad();
	}

	for( i = 0 ; i < seqnum ; i++ )
	    if( (amino_c[i] = (int *) calloc((seqlen[i]+5 ),
					     sizeof(int) ) ) == NULL)
	    {
		printf(" problems with memory allocation");
		printf(" for `amino_c[%d]' !  \n \n", i);
		embExitBad();
	    }
    }


    /******************************************************  
     *                                                     *      
     *  read file, that contains data of anchored regions  *
     *                                                     *      
     ******************************************************/  



    if( anchors )
    {
	edialign_multi_anc_read( input_name );
    }

    if( exclude_frg )
    { 
	if((gl_exclude_list = (int ***) calloc(seqnum,
					    sizeof(int **) )) == NULL)
	{
	    printf(" problems with memory allocation for "
		   "'exclude_list' \n \n");
	    embExitBad();
	} 

	for(i = 0 ; i < seqnum ; i++ ) 
	    if( ( gl_exclude_list[ i ] =
		 (int **) calloc( seqnum , sizeof(int *) )) == NULL)
	    {
		printf(" problems with memory allocation for "
		       "'gl_exclude_list' \n \n");
		embExitBad();
	    } 
  
	for(i = 0 ; i < seqnum ; i++ ) 
	    for(j = 0 ; j < seqnum ; j++ ) 
		if( ( gl_exclude_list[ i ][ j ]  =
		     (int *) calloc( seqlen[ i ] + 1 , sizeof(int) )) == NULL)
		{
		    printf(" problems with memory allocation for "
			   "'gl_exclude_list' \n \n");
		    embExitBad();
		} 

	edialign_exclude_frg_read ( input_name , gl_exclude_list ) ;
    }


    if( wgt_type == 0 )
	edialign_tp400_read( 0 , tp400_prot);
    if( wgt_type % 2 )
	edialign_tp400_read( 1 , tp400_dna );
    if( wgt_type > 1 )
	edialign_tp400_read( 2 , tp400_trans );

    /****************************\
     *                            * 
     *    Name of output files    *  
     *                            * 
     \****************************/

    if( default_name )
    {
	strcpy( printname , input_name);
	strcpy( prn , printname);
    } 
    else
    {  
	strcpy( printname , output_name );
	strcpy( prn , printname);
    }
    

    strcpy(prn2 , prn); 
  
    if( default_name )
	strcat(prn,".ali");

    strcat(prn2,".fa");  
    


    strcpy(logname,printname);
    strcat(logname,".log");

    strcpy(fsm_name , printname);
    strcat(fsm_name,".fsm");

    if( print_status )
    {
	strcpy( pst_name , printname );
	strcat( pst_name,".sta");
    }    

    if( afc_file )
    {
	strcpy( dia_name , printname );  
	strcat( dia_name , ".afc" );
	fp_dia = fopen( dia_name , "w" );
	fprintf(fp_dia,"\n #  %s \n\n  seq_len: " , input_line );
	for( i = 0 ; i < seqnum ; i++ )
	    fprintf(fp_dia,"  %d ", seqlen[i] );
	fprintf(fp_dia,"\n\n");

    }

    if( col_score )
    { 
	strcpy( csc_name , printname );  
	strcat( csc_name , ".csc" );
	fp_csc = fopen( csc_name , "w" );
    }

    if( dia_pa_file )
    {
	strcpy( dia_pa_name , printname );  
	strcat( dia_pa_name , ".fop" );

	fp_dpa = fopen( dia_pa_name , "w" );


	fprintf(fp_dpa,"\n #  %s \n\n  seq_len: " , input_line );
	for( i = 0 ; i < seqnum ; i++ ) 
	    fprintf(fp_dpa,"  %d ", seqlen[i] ); 
	fprintf(fp_dpa,"\n\n");
	fclose( fp_dpa ) ;
    }


    if( motifs )
    {
	strcpy( mot_file_name , printname );  
	strcat( mot_file_name , ".mot" );
	fp_mot = fopen( mot_file_name , "w" );
      
	fprintf(fp_mot,"\n #  %s \n\n   " , input_line );
	fprintf(fp_mot," motif: %s \n\n", gl_mot_regex ); 
	fprintf(fp_mot," max offset for motifs = %d \n\n",
		(int) max_mot_offset ); 
	fprintf(fp_mot," the following fragments contain the motif: \n\n" ); 
	fprintf(fp_mot,"   seq1 seq2    beg1 beg1 len    wgt" ); 
	fprintf(fp_mot,"   # mot    mot_wgt  \n\n" ); 
    }


    if( frag_file )
    {
	strcpy( frag_file_name , printname );  
	strcat( frag_file_name , ".frg" );
	fp_frg = fopen( frag_file_name , "w" );
      
	fprintf(fp_frg,"\n #  %s \n\n  seq_len: " , input_line );
	for( i = 0 ; i < seqnum ; i++ )
	    fprintf(fp_frg,"  %d ", seqlen[i] );
	fprintf(fp_frg,"\n  sequences: " );
	for( i = 0 ; i < seqnum ; i++ )
	    fprintf(fp_frg,"  %s ", seq_name[i] );

	fprintf(fp_frg ,"\n\n");
    }



    gabiosclos = edialign_newAligGraphClosure(seqnum, seqlen, 0, NULL);

    if( (open_pos = (int *** ) calloc( seqnum , sizeof(int **))) == NULL)
    {
	printf("Problems with memory allocation for open_pos\n"); 
	embExitBad();
    }

    for(i=0;i<seqnum;i++)
    {
        if( (open_pos[i] = 
	     (int ** ) calloc( seqnum , sizeof(int *))) == NULL)
	{ 
	    printf("Problems with memory allocation for open_pos\n"); 
	    embExitBad();
	}
    }


    for(i=0;i<seqnum;i++)
	for(j=0;j<seqnum;j++)
	{
	    if( (open_pos[i][j] = 
		 (int * ) calloc( ( seqlen[i]+2) , sizeof(int) ) ) == NULL)
	    { 
		printf("Problems with memory allocation for open_pos\n"); 
		embExitBad();
	    }
	}

    for( i = 0 ; i <seqnum ; i++)
	for( j = 0 ; j <seqnum ; j++)
	    for( ii = 1 ; ii <= seqlen[i] ; ii++)
		open_pos[i][j][ii] = 1;


    /**************************************
     *                                     *
     *      definition of  `amino'         *       
     *                                     *
     **************************************/




    if( wgt_type > 1 ) 
	for(hv=0;hv<seqnum;hv++)
	    for(ii=1;ii<=seqlen[hv]-2;ii++)
	    {
		if(edialign_translate(seq[hv][ii],seq[hv][ii+1],
				      seq[hv][ii+2],hv,
				      ii ) == -1)
		    embExitBad();


		amino[hv][ii] = edialign_translate(seq[hv][ii],
						   seq[hv][ii+1],
						   seq[hv][ii+2],hv,ii);
   
		if( crick_strand )
		{ 
		    nuc1 = edialign_invert( seq[hv][ii+2] );
		    nuc2 = edialign_invert( seq[hv][ii+1] );
		    nuc3 = edialign_invert( seq[hv][ii] );
 
		    amino_c[hv][ii] = edialign_translate( nuc1 , nuc2 , nuc3 ,
							hv , ii);
		}
	    }


    if( wgt_type == 0 ) 
	for(hv=0;hv<seqnum;hv++)
	    for(ii=1;ii<=seqlen[hv];ii++)
	    {
		if( seq[hv][ii] == 'C' ) amino[hv][ii] = 1;           
		if( seq[hv][ii] == 'S' ) amino[hv][ii] = 2;           
		if( seq[hv][ii] == 'T' ) amino[hv][ii] = 3;           
		if( seq[hv][ii] == 'P' ) amino[hv][ii] = 4;           
		if( seq[hv][ii] == 'A' ) amino[hv][ii] = 5;           
		if( seq[hv][ii] == 'G' ) amino[hv][ii] = 6;           
		if( seq[hv][ii] == 'N' ) amino[hv][ii] = 7;           
		if( seq[hv][ii] == 'D' ) amino[hv][ii] = 8;           
		if( seq[hv][ii] == 'E' ) amino[hv][ii] = 9;           
		if( seq[hv][ii] == 'Q' ) amino[hv][ii] = 10;           
		if( seq[hv][ii] == 'H' ) amino[hv][ii] = 11;           
		if( seq[hv][ii] == 'R' ) amino[hv][ii] = 12;           
		if( seq[hv][ii] == 'K' ) amino[hv][ii] = 13;           
		if( seq[hv][ii] == 'M' ) amino[hv][ii] = 14;           
		if( seq[hv][ii] == 'I' ) amino[hv][ii] = 15;           
		if( seq[hv][ii] == 'L' ) amino[hv][ii] = 16;           
		if( seq[hv][ii] == 'V' ) amino[hv][ii] = 17;           
		if( seq[hv][ii] == 'F' ) amino[hv][ii] = 18;           
		if( seq[hv][ii] == 'Y' ) amino[hv][ii] = 19;           
		if( seq[hv][ii] == 'W' ) amino[hv][ii] = 20;           
	    }


     
    amino_acid[0] = 'X';           
    amino_acid[1] = 'C';           
    amino_acid[2] = 'S';           
    amino_acid[3] = 'T';           
    amino_acid[4] = 'P';           
    amino_acid[5] = 'A';           
    amino_acid[6] = 'G';           
    amino_acid[7] = 'N';           
    amino_acid[8] = 'D';           
    amino_acid[9] = 'E';           
    amino_acid[10] = 'Q';           
    amino_acid[11] = 'H';           
    amino_acid[12] = 'R';           
    amino_acid[13] = 'K';           
    amino_acid[14] = 'M';           
    amino_acid[15] = 'I';           
    amino_acid[16] = 'L';           
    amino_acid[17] = 'V';           
    amino_acid[18] = 'F';           
    amino_acid[19] = 'Y';           
    amino_acid[20] = 'W';



    num_dia_anc = anc_num * (seqnum-1);


    if ( anchors )
    {
	if( time_stamps )
	    beg_ts = clock() ;

	if ( nas == 0 ) 
	{
	    if( bubblesort ) 
		edialign_bubble_sort ( anc_num , anchor_frg ) ;  
	    else
		edialign_frag_sort ( anc_num , anchor_frg , 0 ) ;
	}

	if( time_stamps)
	{
	    end_ts = clock() ;
	    time_diff_srt = (float) ( end_ts - beg_ts ) / CLOCKS_PER_SEC ;
	    if( time_stamps )
		printf (" for anc: time_diff_srt = %f \n", time_diff_srt );
	}


	edialign_filter( &anc_num , anchor_frg);
	/*  embExitBad() ; 
	 */
    }


    if(long_output)
    {
	fp_log = fopen(logname,"w");
	fprintf(fp_log,"\n #  %s \n\n   " , input_line );
    }

    if(frg_mult_file)
    {
	fp_fsm = fopen(fsm_name,"w");
	fprintf(fp_fsm,"\n #  %s \n\n" , input_line );
    }





    if( 
       (num_dia_bf = (int *) calloc( ( max_itnum + 1 )  ,  sizeof(int)))
       == NULL
       )
    {
	printf(" problems with memory allocation for `num_dia_bf' !  \n \n");
	embExitBad();
    }


    if( 
       (num_dia_af = (int *) calloc( ( max_itnum + 1 )  ,  sizeof(int)))
       == NULL
       )
    {
	printf(" problems with memory allocation for `num_dia_af' !  \n \n");
	embExitBad();
    }


    all_it_dia = (struct multi_frag *) calloc(1, sizeof(struct multi_frag));
    current_dia = all_it_dia;


    strcpy(itname,printname);
    strcpy(itname2,printname);
    strcpy(itname3,printname);
    strcpy(itname4,printname);
    sprintf(str,".ali");
  
    if( default_name )
	strcat(itname,str);
  
    sprintf(str,".fa");
    strcat(itname2,str);


    if( msf_file )
	strcat(itname3,".ms");
  
  
    if( cw_file )
	strcat(itname4,".cw");
  
    if( textual_alignment )
    {
/*
	ajStrAssignS(&tnstr,ajFileGetNameS(outfile));
	ajFileClose(&outfile);

	fp_ali = fopen(ajStrGetPtr(tnstr),"wb");
*/
	fp_ali = ajFileGetFileptr(outfile);
	
	/* fp_ali = fopen(itname,"w"); */
    }
    
 
    if( standard_out )
	fp_ali = stdout;

    /* Sequence output A */
    fasta_file = 1;

/*
    if( textual_alignment )
        if(fasta_file)
	    fp2 = fopen(itname2,"w");
*/

    if(msf_file)
	fp3 = fopen(itname3,"w");

    if(cw_file)
	fp4 = fopen(itname4,"w");

    if( textual_alignment )
	edialign_para_print(seq_file , fp_ali);


    /***************************\
     *                           * 
     *      ITERATION START      *   
     *                           * 
     \***************************/

    istep = 0 ; 
    while( ( cont_it == 1 ) && ( istep < max_itnum ) ) 
    {
	cont_it = 0 ;
	istep++ ; 

	/* printf("\n  istep = %d \n", istep ); */


	this_it_dia = current_dia;
 
	strcpy(itname,printname);
	strcpy(itname2,printname);
	strcpy(itname3,printname);
	strcpy(itname4,printname);
	sprintf(str,".ali");

        
	if( default_name )
	    strcat(itname,str); 

	sprintf(str,".fa");
	strcat(itname2,str); 

	if( msf_file )
	    strcat(itname3,".ms"); 
   
    
	if( cw_file )
	    strcat(itname4,".cw"); 
  
	weight_sum_af = 0;
	num_dia_bf[ istep ] = 0;
 
	if( time_stamps ) 
	    beg_pa = clock(); 


	if( ref_seq == 0 ) 
	    i_max = seqnum ; 
	else
	    i_max = 1 ; 

	for(i = 0 ; i < i_max ; i++)
	{
	    for(j = i + 1 ; j < seqnum ; j++)
	    {
		/****************************************\
		 *                                        * 
		 *          PAIRWISE  ALIGNMENT           *
		 *                                        * 
		 \****************************************/

                if( cont_it_p[ i ][ j] )
		{

		    /*
		       printf("\n out of frc it %d : wgt 20 = %f \n", istep ,
		       wgt_dna[ 20 ][ 20 ] ) ;
		       */
		    score = edialign_frag_chain(i,j,fp_ali,fp_mot,&num_dia_p);
                }
                else
		{
		    score = 0 ; 
		    num_dia_p = 0 ; 
                }

                if( istep == 1 )
		{
                    pair_score[j][i] = score;  
                    pair_score[i][j] = score;  
		}


                for(k=0;k<num_dia_p;k++)
		{
                    *current_dia = pair_dia[k];

                    current_dia->next 
			= (struct multi_frag *) calloc(1,
						   sizeof(struct multi_frag));
                    end_dia = current_dia;   
                    current_dia = current_dia->next;
                    current_dia->pred = end_dia; 
		}

                num_dia_bf[ istep ] = num_dia_bf[ istep ] + num_dia_p;

                for(hv=0; hv<num_dia_p;hv++)
		    weight_sum_af = weight_sum_af + (pair_dia[hv]).weight;


		if(num_dia_p)
		    free(pair_dia);

	    } /*    for(j = i+1 ; j<seqnum ; j++) */

	} /*   for(i = 0 ; i<seqnum ; i++) */


	if( time_stamps )
	{ 
	    end_pa = clock(); 

	    time_diff_pa = (float) ( end_pa - beg_pa ) / CLOCKS_PER_SEC ; 
	    if( time_stamps )
		printf (" time_diff_pa = %f \n", time_diff_pa ); 
	    total_pa_time = total_pa_time + time_diff_pa;
	}




	if( break1 )
	{
	    printf("\n  break1\n");
	    embExitBad() ;
	}


	/*
	   if( pa_only )
	   {
	   printf("\n\n istep = %d, pa finished - exit \n\n", istep );       
	   embExitBad();
	   }
	   */

	if(overlap_weights)
	{
	    diagonal1 = this_it_dia;
	    dia_counter = 0;

	    if( diagonal1 != NULL )   
		while( diagonal1->next != NULL )   
		{
		    dia_counter++;
		    if( print_status )
			if( ( dia_counter % 100 ) == 0 )
			{                
			    fp_st = fopen( pst_name ,"w");

			    fprintf(fp_st," dsd  %s \n", input_line);
			    fprintf(fp_st,"\n\n\n    Status of the program "
				    "run:\n");  
			    fprintf(fp_st,"    =========================="
				    "\n\n");  
			    if( seqnum > 2 ) { 
				fprintf(fp_st,"      iteration step %d in ",
					istep); 
				fprintf(fp_st,"multiple alignment\n" );
			    }
			    fprintf(fp_st,"      calculating overlap weight "
				    "for diagonals\n");
			    fprintf(fp_st,"      current diagonal = %d\n\n",
				    dia_counter );
			    fprintf(fp_st,"      total number of"); 
			    fprintf(fp_st," diagonals: %d\n\n\n\n",
				    num_dia_bf[ istep ]);
			    fclose(fp_st);
			}

		    diagonal2 = diagonal1->next;

		    while(diagonal2->next != NULL) 
		    {
			if( diagonal1->trans == diagonal2->trans ) 
			    edialign_ow_add(diagonal1 , diagonal2); 
			diagonal2 = diagonal2->next;        
		    }
		    diagonal1 = diagonal1->next;        
		}

	    if(bubblesort)   
		edialign_ow_bubble_sort(num_dia_bf[istep],this_it_dia); 
	    else 
		edialign_frag_sort(num_dia_bf[istep],this_it_dia,
				   overlap_weights); 
	}
	else				/* no overlap_weights */
	{
	    beg_ts = clock() ; 

	    if( bubblesort ) 
		edialign_bubble_sort( num_dia_bf[ istep ] , this_it_dia );
	    else 
		edialign_frag_sort(num_dia_bf[istep],this_it_dia,
				   overlap_weights);

	    end_ts = clock() ; 
	    time_diff_srt = (float) ( end_ts - beg_ts ) / CLOCKS_PER_SEC ;
	    if( time_stamps )
		printf (" time_diff_srt = %f \n", time_diff_srt );
	}


	num_dia_af[ istep ] = num_dia_bf[ istep ];
	weight_sum_bf = weight_sum_af;

	pairalignsum = 0;
	pairalignlen = 0;


	edialign_filter( num_dia_af + istep , this_it_dia); 
	num_all_it_dia = num_all_it_dia + num_dia_af[ istep ];


	/*
	   if( pa_only == 0 )
	   {
	   printf("\n\n istep = %d, filter finished - exit \n\n", istep );
	   embExitBad();
	   }
	*/


	weight_sum_af = 0;
       
	edialign_print_log( this_it_dia , fp_log , fp_fsm );

	if( frag_file )
	    edialign_print_fragments( this_it_dia , fp_frg );

	edialign_throw_out( &weight_sum_af );

	edialign_sel_test( );

	threshold = threshold ;

	if( break2 )
	{
	    printf("\n  break2\n");
	    embExitBad() ;
	}


    } /* while ( cond_it == 1 ) */  


    /***************************\
     *                           * 
     *       ITERATION END       *   
     *                           * 
     \***************************/

    strcpy( dist_name , printname);
    strcat(dist_name , ".dst");

    if ( ref_seq == 0 ) 
	edialign_av_tree_print();

    if( standard_out )
        fp_ali = stdout;
      
    if(sf_mat)
    {
	edialign_subst_mat(input_name , num_all_it_dia , all_it_dia);
    }


    if( textual_alignment )
	edialign_ali_arrange(num_all_it_dia,all_it_dia,fp_ali,seqout,fp3,fp4,
		    fp_csc,isprot);


    if(long_output) 
    {
	/*     fprintf(fp_log "\n\n thr = %f , lmax = %d , speed = %f  */  
	fprintf(fp_log, "\n\n    total sum of weights: %f \n\n\n", tot_weight);
	fclose(fp_log);
    }



/*
    if( argnum == 1 )
    {
	printf("\n     Program terminated normally\n");
	printf("     Results are contained in file `%s' \n \n \n", itname);
    }
*/

    av_dia_num = (float) (2 * dia_num);
    av_dia_num = av_dia_num / ( seqnum * ( seqnum - 1) ) ;

    av_max_dia_num = (float) (2 * max_dia_num);
    av_max_dia_num = av_max_dia_num / ( seqnum * ( seqnum - 1) ) ;



    tmpi1 = (ajint) av_dia_num ;
    tmpi2 = (ajint) av_max_dia_num ;

    if(pr_av_nd)
	printf(" %d ", tmpi1 );

    if(pr_av_max_nd)
	printf(" %d ", tmpi2 );



    if(pr_av_nd)
	fprintf(fp_ali, "    %d fragments considered for alignment \n",
		tmpi1 );

    if(pr_av_max_nd)
	fprintf(fp_ali, "    %d fragments simultaneously stored \n\n", tmpi2 );

    if( textual_alignment )
    {
/*	fclose(fp_ali); */
	ajFileClose(&outfile);
    }
    


    if( time_stamps )
    { 
	end_ali = clock() ; 
	time_diff_ali = (float) ( end_ali - beg_ali ) / CLOCKS_PER_SEC ;

	perc_pa_time = total_pa_time / time_diff_ali * 100 ; 
	printf (" time_diff_ali = %f \n", time_diff_ali ); 
	printf (" total_pa_time = %f \n", total_pa_time );
	printf (" corresponds to %f percent \n\n", perc_pa_time );
    } 


    for(i=0;i<seqnum;i++)
    {
        for(j=0;j<seqnum;j++)
        {
            if(gl_exclude_list)
                AJFREE(gl_exclude_list[i][j]);
            if(open_pos)
                AJFREE(open_pos[i][j]);
        }

        if(gl_exclude_list) AJFREE(gl_exclude_list[i]);
        if(open_pos) AJFREE(open_pos[i]);
        if(pair_score) AJFREE(pair_score[i]);
        if(cont_it_p) AJFREE(cont_it_p[i]);
        if(glob_sim) AJFREE(glob_sim[i]);
        if(amino) AJFREE(amino[i]);
        if(amino_c) AJFREE(amino_c[i]);
    }
    AJFREE(gl_exclude_list);
    AJFREE(open_pos);
    AJFREE(pair_score);
    AJFREE(cont_it_p);
    AJFREE(glob_sim);
    AJFREE(amino);
    AJFREE(amino_c);

    ajStrDel(&tnstr);
    ajSeqsetDel(&seqset);
    ajSeqoutDel(&seqout);
    ajStrDel(&nucmode);
    ajStrDel(&overlapw);
    ajStrDel(&linkage);
    ajFileClose(&outfile);

    for(ii=1;ii<max_dia+1;ii++)
    {
        if(tp400_prot)AJFREE(tp400_prot[ii]);
        if(tp400_dna)AJFREE(tp400_dna[ii]);
        if(tp400_trans)AJFREE(tp400_trans[ii]);
        if(wgt_prot)AJFREE(wgt_prot[ii]);
        if(wgt_dna)AJFREE(wgt_dna[ii]);
        if(wgt_trans)AJFREE(wgt_trans[ii]);
        if(min_weight)AJFREE(min_weight[ii]);
    }
    
    AJFREE(tp400_prot);
    AJFREE(tp400_dna);
    AJFREE(tp400_trans);
    AJFREE(wgt_prot);
    AJFREE(wgt_dna);
    AJFREE(wgt_trans);
    AJFREE(min_weight);

    for(i=0;i<n;i++)
    {
        if(seq) AJFREE(seq[i]);
        if(seq_name) AJFREE(seq_name[i]);
        if(full_name) AJFREE(full_name[i]);
    }
    
    AJFREE(seq);
    AJFREE(seq_name);
    AJFREE(full_name);
    AJFREE(seqlen);

    if(arguments)
    {
        for(i=0;i<argnum;i++)
            AJFREE(arguments[i]);
        AJFREE(arguments);
    }

    AJFREE(par_file);
    AJFREE(upg_str);
    AJFREE(num_dia_bf);
    AJFREE(num_dia_af);
    while(all_it_dia)
    {
        next_dia = all_it_dia->next;
        AJFREE(all_it_dia);
        all_it_dia = next_dia;
    }

    edialign_freeAligGraphClosure(gabiosclos);

    embExit();

    return 0;
} /* main */




/* @funcstatic edialign_computeClosure ***************************************
**
** edialign_computeClosure
**
** @param [w] clos [edialignCLOSURE*] Closure
** @return [void]
*****************************************************************************/

static void edialign_computeClosure(edialignCLOSURE *clos)
{
    ajint **Succ;
    ajint **Pred;
    ajint *NSucc;
    ajint *NPred;
    ajint *npred;
    ajint nsucc;
    ajint ni;
    ajint nj;
    ajint s;
    ajint top;
    ajint bottom;
    ajint n0;
    ajint p;
    ajint n;
    ajint i;
    ajint k;
    ajint pos_n;
    ajint x;

    Succ = (ajint **) edialign_callouer_mat(sizeof(ajint), clos->nbrAligSets+2,
				   clos->seqNbr);
    Pred = (ajint **) edialign_callouer_mat(sizeof(int), clos->nbrAligSets+2,
				   clos->seqNbr);
    NSucc = (ajint *) edialign_allouer((clos->nbrAligSets+2) * sizeof(ajint));
    NPred = (ajint *) edialign_allouer((clos->nbrAligSets+2) * sizeof(ajint));
    npred = (ajint *) edialign_allouer((clos->nbrAligSets+2) * sizeof(ajint));
    clos->topolog = (ajint *) edialign_allouer(sizeof(int));

    /* C A L C U L des Succ[n][x] et NPred[n] */

    for(n=1; n <= clos->nbrAligSets; n++)
	NPred[n] = 0;

    for(n=1; n <= clos->nbrAligSets; n++)
    {
	nsucc = 0;
	for(x=0; x < clos->seqNbr; x++)
	    if(clos->aligSet[n].pos[x] > 0)
	    {
		pos_n = clos->aligSet[n].pos[x];

		for(i=pos_n+1; i <= clos->seq[x].longueur &&
		     clos->seq[x].aligSetNbr[i] == 0; i++)
		    clos->seq[x].predAligSetPos[i] = pos_n;

		if(i <= clos->seq[x].longueur)
		{
		    clos->seq[x].predAligSetPos[i] = pos_n;
		    if(clos->aligSet[clos->seq[x].aligSetNbr[i]].nbr > 0)
		    {
			n0 = Succ[n][nsucc] = clos->seq[x].aligSetNbr[i];
			clos->aligSet[n0].nbr = - clos->aligSet[n0].nbr;
			nsucc++;
		    }
		}

		for(i=pos_n-1; i > 0 && clos->seq[x].aligSetNbr[i] == 0; i--)
		    clos->seq[x].succAligSetPos[i] = pos_n;

		if(i > 0) 	
		    clos->seq[x].succAligSetPos[i] = pos_n;
	    }

	for(p=0; p < nsucc; p++)
	{
	    n0 = Succ[n][p];
	    Pred[n0][NPred[n0]] = n;
	    NPred[n0]++;

	    clos->aligSet[n0].nbr = - clos->aligSet[n0].nbr;
	}
	NSucc[n] = nsucc;
    }

    /* C A L C U L de clos->topolog */
    
    clos->topolog = (ajint *) edialign_reallouer(clos->topolog,
						 (clos->nbrAligSets+2) *
						 sizeof(ajint));
    
    bottom = top = 0;
    
    for(n=1; n <= clos->nbrAligSets; n++)
    {
	npred[n] = NPred[n];
	if (npred[n] == 0)
	{
	    top++;
	    clos->topolog[top] = n;
	}
    }
    
    
    while( bottom != top)
    {
	bottom++;
	ni = clos->topolog[bottom];
	for(s=0; s < NSucc[ni]; s++)
	{
	    nj = Succ[ni][s];
	    npred[nj]--;
	    if(npred[nj] == 0)
	    {
		top++;
		clos->topolog[top] = nj;
	    }
	}
    }
    
    
    for(x=0; x < clos->seqNbr; x++)
    {
	clos->predFrontier[0][x] = 0;
	clos->succFrontier[clos->nbrAligSets+1][x] = clos->seq[x].longueur+1;
    }
    
    
    for(k=1; k <= clos->nbrAligSets; k++)
    {
	n0 = clos->topolog[k];
	for(x=0; x < clos->seqNbr; x++)
	{
	    if(clos->aligSet[n0].pos[x] > 0)
		clos->predFrontier[n0][x] = clos->aligSet[n0].pos[x];
	    else
		for(p=0, clos->predFrontier[n0][x]=0; p < NPred[n0]; p++)
		{
		    n = Pred[n0][p];
		    if (clos->predFrontier[n][x] > clos->predFrontier[n0][x]) 
			clos->predFrontier[n0][x] = clos->predFrontier[n][x];
		}
	}
    }
    
    
    for(k=clos->nbrAligSets; k > 0; k--)
    {
	n0 = clos->topolog[k];
	for(x=0; x < clos->seqNbr; x++)
	{
	    if(clos->aligSet[n0].pos[x] > 0)
		clos->succFrontier[n0][x] = clos->aligSet[n0].pos[x];
	    else
		for(p=0, clos->succFrontier[n0][x]=clos->seq[x].longueur+1; 
		     p < NSucc[n0]; p++)
		{
		    n = Succ[n0][p];
		    if(clos->succFrontier[n][x] < clos->succFrontier[n0][x]) 
			clos->succFrontier[n0][x] = clos->succFrontier[n][x];
		}
	}
    }
    
    edialign_liberer(npred);
    edialign_liberer(NPred);
    edialign_liberer(NSucc); 
    edialign_liberer_mat((void **) Pred, clos->nbrAligSets+2); 
    edialign_liberer_mat((void **) Succ, clos->nbrAligSets+2); 
    edialign_liberer(clos->topolog);

    return;
}




/* @funcstatic edialign_moveAligSet ******************************************
**
** edialign_moveAligSet
**
** @param [w] clos [edialignCLOSURE*] Closure
** @param [r] n1 [ajint] Undocumented
** @param [r] n2 [ajint] Undocumented
** @return [void]
*****************************************************************************/

static void edialign_moveAligSet(edialignCLOSURE *clos, ajint n1, ajint n2)
{
    ajint x;
    ajint k;

    for(x=0; x < clos->seqNbr; x++)
    {
	k = clos->aligSet[n1].pos[x] = clos->aligSet[n2].pos[x];
	if(k > 0)
	    clos->seq[x].aligSetNbr[k] = n1;

	clos->predFrontier[n1][x] = clos->predFrontier[n2][x];
	clos->succFrontier[n1][x] = clos->succFrontier[n2][x];
    }
    
    clos->aligSet[n1].nbr = clos->aligSet[n2].nbr;

    return;
}




/* @funcstatic edialign_read_closure ****************************************
**
** edialign_read_closure
**
** @param [w] clos [edialignCLOSURE*] Closure
** @param [r] nbreancr [ajint] Undocumented
** @param [w] ancrages [ajint**] Undocumented
** @return [void]
*****************************************************************************/

static void edialign_read_closure(edialignCLOSURE *clos, ajint nbreancr,
				  ajint **ancrages)
{
    ajint x;
    ajint ind;
    ajint n;

    for(n=0; n < nbreancr; n++)
    {
	clos->nbrAligSets++;
	edialign_realloc_closure(clos);

	clos->aligSet[clos->nbrAligSets].nbr = 0;
	for(x=0; x < clos->seqNbr; x++)
	{
	    ind = clos->aligSet[clos->nbrAligSets].pos[x] = ancrages[n][x];
	    if(ind > 0)
	    {
		clos->aligSet[clos->nbrAligSets].nbr++;
		clos->seq[x].aligSetNbr[ind] = clos->nbrAligSets;
	    }
	}
    }

    edialign_computeClosure(clos);
}




/* @funcstatic edialign_init_closure ****************************************
**
** edialign_init_closure
**
** @param [w] clos [edialignCLOSURE*] Closure
** @param [r] nbreancr [ajint] Undocumented
** @param [w] ancrages [ajint**] Undocumented
** @return [void]
*****************************************************************************/

static void edialign_init_closure(edialignCLOSURE *clos, ajint nbreancr,
				  ajint **ancrages)
{
    ajint x;
    ajint i;
    ajint *longsequ;

    longsequ = (ajint *) edialign_allouer(clos->seqNbr * sizeof(ajint));

    for(x=0; x < clos->seqNbr; x++)
    {
	longsequ[x] = clos->seq[x].longueur;
	for(i=1; i <= clos->seq[x].longueur; i++)
	    clos->seq[x].aligSetNbr[i] = clos->seq[x].succAligSetPos[i] 
		= clos->seq[x].predAligSetPos[i] = 0;
    }

    clos->nbrAligSets = 0;

    if(nbreancr > 0)
	edialign_read_closure(clos, nbreancr, ancrages);

    for(x=0; x < clos->seqNbr; x++)
	clos->seq[x].longueur = longsequ[x];

    edialign_liberer(longsequ);

    return;
}




/* @funcstatic edialign_alloc_closure ****************************************
**
** edialign_alloc_closure
**
** @param [w] clos [edialignCLOSURE*] Closure
** @return [void]
*****************************************************************************/

static void edialign_alloc_closure(edialignCLOSURE *clos)
{
    ajlong na;
    ajint x;

    /* sera re'alloue' */
    clos->predFrontier = (ajint **) edialign_callouer_mat(sizeof(int),
							  clos->maxLong+2,
							  clos->seqNbr+1);

    /* sera re'alloue' */
    clos->succFrontier = (ajint **) edialign_callouer_mat(sizeof(int),
							  clos->maxLong+2,
							  clos->seqNbr+1);

    /* sera re'alloue' */
    clos->aligSet =
	(edialignPositionSet *) edialign_allouer((clos->maxLong+2) *
						 sizeof(edialignPositionSet));

    for(na=0; na <= clos->maxLong+1; na++)
	clos->aligSet[na].pos = (ajint *) edialign_allouer(clos->seqNbr *
							   sizeof(ajint));

    clos->oldNbrAligSets = clos->maxLong;

    for (x=0; x < clos->seqNbr; x++)
    {
	clos->seq[x].aligSetNbr = (ajint *) edialign_allouer((clos->seq[x].
							      longueur+2)*
							     sizeof(ajint));
	clos->seq[x].predAligSetPos = (ajint *) 
	    edialign_allouer((clos->seq[x].longueur+2)*sizeof(ajint));
	clos->seq[x].succAligSetPos = (ajint *) 
	    edialign_allouer((clos->seq[x].longueur+2)*sizeof(ajint));
    }

    clos->gauche1 = (ajint *) edialign_allouer(clos->seqNbr * sizeof(ajint));
    clos->gauche2 = (ajint *) edialign_allouer(clos->seqNbr * sizeof(ajint));
    clos->droite1 = (ajint *) edialign_allouer(clos->seqNbr * sizeof(ajint));
    clos->droite2 = (ajint *) edialign_allouer(clos->seqNbr * sizeof(ajint));
    clos->pos_ = (ajint **) edialign_callouer_mat(sizeof(ajint), clos->seqNbr,
					 clos->seqNbr);

    return;
}




/* @funcstatic edialign_free_closure ****************************************
**
** edialign_free_closure. Unused.
**
** @param [w] clos [edialignCLOSURE*] Closure
** @return [void]
*****************************************************************************/

static void edialign_free_closure(edialignCLOSURE *clos)
{
    ajlong na;
    ajint x;

    edialign_liberer(clos->gauche1);
    edialign_liberer(clos->gauche2);
    edialign_liberer(clos->droite1);
    edialign_liberer(clos->droite2); 
    edialign_liberer_mat((void **) clos->pos_, clos->seqNbr); 

    edialign_liberer_mat((void **) clos->succFrontier,clos->oldNbrAligSets+2); 
    edialign_liberer_mat((void **) clos->predFrontier,clos->oldNbrAligSets+2);

    for(x=0; x < clos->seqNbr; x++)
    {
	edialign_liberer(clos->seq[x].aligSetNbr); 
	edialign_liberer(clos->seq[x].predAligSetPos); 
	edialign_liberer(clos->seq[x].succAligSetPos); 
    }

    for(na=0; na <= clos->oldNbrAligSets+1; na++)
    {
	edialign_liberer(clos->aligSet[na].pos); 
    }
    edialign_liberer(clos->aligSet);

    return;
}




/* @funcstatic edialign_realloc_closure **************************************
**
** edialign_realloc_closure
**
** @param [w] clos [edialignCLOSURE*] Closure
** @return [void]
*****************************************************************************/

static void edialign_realloc_closure(edialignCLOSURE *clos)
{
    ajint na;

    if(clos->nbrAligSets > clos->oldNbrAligSets)
    {
	clos->predFrontier = (ajint **)
	    edialign_recallouer_mat((void **) clos->predFrontier,
				    sizeof(ajint), 
				    clos->oldNbrAligSets+2,
				    clos->nbrAligSets+2,
			   clos->seqNbr+1);

	clos->succFrontier = (ajint **)
	    edialign_recallouer_mat((void **) clos->succFrontier,
				    sizeof(ajint), 
				    clos->oldNbrAligSets+2,
				    clos->nbrAligSets+2,
				    clos->seqNbr+1);

	clos->aligSet = (edialignPositionSet *)
	    edialign_reallouer(clos->aligSet, (clos->nbrAligSets+2) *
		      sizeof(edialignPositionSet));

	for(na=clos->oldNbrAligSets+2; na <= clos->nbrAligSets+1; na++)
	{
	    clos->aligSet[na].pos = (ajint *)
		edialign_allouer(clos->seqNbr * sizeof(ajint));
	}
	clos->oldNbrAligSets = clos->nbrAligSets;
    }

    return;
}




#if 0
/* @funcstatic edialign_print_aligSets **************************************
**
** edialign_print_aligSets. Unused.
**
** @param [w] clos [edialignCLOSURE*] Closure
** @param [r] nseq [ajint] Undocumented
** @param [r] i [ajint] Undocumented
** @return [ajint] Undocumented
*****************************************************************************/

static ajint edialign_print_aligSets(edialignCLOSURE *clos,
				     ajint nseq, ajint i)
{
    ajint n;
    ajint ng;
    ajint nd;
    ajint k;
    ajint x;

    n = ng = nd = clos->seq[nseq].aligSetNbr[i];

    if(ng == 0)
    {
	k = clos->seq[nseq].predAligSetPos[i];
	if (k > 0)
	    ng = clos->seq[nseq].aligSetNbr[k];
	k = clos->seq[nseq].succAligSetPos[i];
	if (k > 0)
	    nd = clos->seq[nseq].aligSetNbr[k];
    }

    printf("echelle %d: ", n);
    if(n != 0)	
	for(x=0; x < clos->seqNbr; x++) 
	    printf("%d ", clos->aligSet[n].pos[x]);

    printf("\nfrontiere clos->gauche %d: ", ng);
    if(ng != 0)
	for(x=0; x < clos->seqNbr; x++) 
	    printf("%d ", clos->predFrontier[ng][x]);

    printf("\nfrontiere clos->droite %d: ", nd);
    if(nd != 0)
	for (x=0; x < clos->seqNbr; x++) 
	    printf("%d ", clos->succFrontier[nd][x]);

    printf("\n");

    return 0;
}
#endif




/* @funcstatic edialign_init_seq **************************************
**
** edialign_init_seq
**
** @param [w] clos [edialignCLOSURE*] Undocumented
** @param [r] nbreseq [ajint] Undocumented
** @param [w] longseq [ajint*] Undocumented
** @return [void]
*****************************************************************************/

static void edialign_init_seq(edialignCLOSURE *clos,
			      ajint nbreseq, ajint *longseq)
{
    ajint x;

    clos->seqNbr = nbreseq;

    clos->seq =
	(edialignSequence *) edialign_allouer(clos->seqNbr *
					      sizeof(edialignSequence));

    for (x=clos->maxLong=0; x < clos->seqNbr; x++)
    {
	clos->seq[x].longueur = longseq[x];
	if (clos->maxLong < longseq[x])
	    clos->maxLong = longseq[x];
    }

    return;
}




/* @funcstatic edialign_desinit_seq **************************************
**
** edialign_desinit_seq. Unused.
**
** @param [w] clos [edialignCLOSURE*] Undocumented
** @return [void]
*****************************************************************************/

static void edialign_desinit_seq(edialignCLOSURE *clos)
{

    edialign_liberer(clos->seq);

    return;
}




/* @funcstatic edialign_newAligGraphClosure **********************************
**
** edialign_newAligGraphClosure
**
** @param [r] nbreseq [ajint] Undocumented
** @param [w] longseq [ajint*] Undocumented
** @param [r] nbreancr [ajint] Undocumented
** @param [w] ancrages [ajint**] Undocumented
** @return [edialignCLOSURE*] Undocumented
*****************************************************************************/

static edialignCLOSURE* edialign_newAligGraphClosure(ajint nbreseq,
						     ajint *longseq,
						     ajint nbreancr,
						     ajint **ancrages)
{

    edialignCLOSURE *clos =
	(edialignCLOSURE *) edialign_allouer(sizeof(edialignCLOSURE));

    edialign_init_seq(clos, nbreseq, longseq);

    edialign_alloc_closure(clos);		/* utilise clos->maxLong */

    edialign_init_closure(clos, nbreancr, ancrages); 

    return clos;
}




/* @funcstatic edialign_freeAligGraphClosure **********************************
**
** edialign_freeAligGraphClosure. Unused.
**
** @param [w] clos [edialignCLOSURE*] Undocumented
** @return [void]
*****************************************************************************/

static void edialign_freeAligGraphClosure(edialignCLOSURE *clos)
{
    edialign_free_closure(clos);

    edialign_desinit_seq(clos);

    edialign_liberer(clos);

    return;
}




/* @funcstatic edialign_addAlignedPositions **********************************
**
** edialign_addAlignedPositions
**
** @param [w] clos [edialignCLOSURE*] Undocumented
** @param [r] seq1 [ajint] Undocumented
** @param [r] i [ajint] Undocumented
** @param [r] seq2 [ajint] Undocumented
** @param [r] j [ajint] Undocumented
** @return [ajint] Undocumented
*****************************************************************************/

static ajint edialign_addAlignedPositions(edialignCLOSURE *clos,
					  ajint seq1, ajint i,
					  ajint seq2, ajint j)
{
    ajint n;
    ajint n1;
    ajint n2;
    ajint ng1;
    ajint ng2;
    ajint nd1;
    ajint nd2;
    ajint nn;
    ajint k;
    ajint x;
    ajint y;

    n1 = ng1 = nd1 = clos->seq[seq1].aligSetNbr[i];
    n2 = ng2 = nd2 = clos->seq[seq2].aligSetNbr[j];

    if(n1 == 0 || n2 == 0 || n1 != n2)
    {
	if(ng1 == 0)
	{
	    k = clos->seq[seq1].predAligSetPos[i];
	    if(k > 0)
		ng1 = clos->seq[seq1].aligSetNbr[k];
	    k = clos->seq[seq1].succAligSetPos[i];
	    if(k > 0)
		nd1 = clos->seq[seq1].aligSetNbr[k];
	}

	if(ng2 == 0)
	{
	    k = clos->seq[seq2].predAligSetPos[j];
	    if(k > 0)
		ng2 = clos->seq[seq2].aligSetNbr[k];
	    k = clos->seq[seq2].succAligSetPos[j];
	    if(k > 0)
		nd2 = clos->seq[seq2].aligSetNbr[k];
	}

	if(ng1 == 0)
	    for(x=0; x < clos->seqNbr; x++)
		clos->gauche1[x] = 0;
	else
	    for(x=0; x < clos->seqNbr; x++)
		clos->gauche1[x] = clos->predFrontier[ng1][x];

	if(nd1 == 0)
	    for(x=0; x < clos->seqNbr; x++)
		clos->droite1[x] = clos->seq[x].longueur + 1;
	else
	    for(x=0; x < clos->seqNbr; x++)
		clos->droite1[x] = clos->succFrontier[nd1][x];

	if(ng2 == 0)
	    for(x=0; x < clos->seqNbr; x++)
		clos->gauche2[x] = 0;
	else
	    for(x=0; x < clos->seqNbr; x++)
		clos->gauche2[x] = clos->predFrontier[ng2][x];

	if(nd2 == 0)
	    for(x=0; x < clos->seqNbr; x++)
		clos->droite2[x] = clos->seq[x].longueur + 1;
	else
	    for(x=0; x < clos->seqNbr; x++)
		clos->droite2[x] = clos->succFrontier[nd2][x];

	clos->gauche1[seq1] = clos->droite1[seq1] = i;
	clos->gauche2[seq2] = clos->droite2[seq2] = j;

	nn = clos->nbrAligSets + 1;

	for(x=0; x < clos->seqNbr; x++)	
	{
	    clos->aligSet[nn].pos[x] = 0;
	    if(n1 > 0 && clos->aligSet[n1].pos[x] > 0)
		clos->aligSet[nn].pos[x] = clos->aligSet[n1].pos[x];
	    else
	    {
		if(n2 > 0 && clos->aligSet[n2].pos[x] > 0)
		       clos->aligSet[nn].pos[x] = clos->aligSet[n2].pos[x];
	    }

	    if (clos->aligSet[nn].pos[x] == 0)
	    {
		clos->predFrontier[nn][x] = edmax(clos->gauche1[x],
						clos->gauche2[x]);
		clos->succFrontier[nn][x] = edmin(clos->droite1[x],
						clos->droite2[x]);
	    }
	    else
		clos->predFrontier[nn][x] = clos->succFrontier[nn][x] =
		    clos->aligSet[nn].pos[x];
	}

	clos->predFrontier[nn][seq1] = clos->succFrontier[nn][seq1] =
	    clos->aligSet[nn].pos[seq1] = i;

	clos->predFrontier[nn][seq2] = clos->succFrontier[nn][seq2] =
	    clos->aligSet[nn].pos[seq2] = j;


	for(x=clos->aligSet[nn].nbr=0; x < clos->seqNbr; x++)
	    if(clos->aligSet[nn].pos[x] > 0)
	    {
		k = clos->aligSet[nn].pos[x]; 
		clos->seq[x].aligSetNbr[k] = nn;
		clos->aligSet[nn].nbr++;
	    }

	for(x=0; x < clos->seqNbr; x++)
	    if(clos->droite1[x] != clos->droite2[x])
	    /* => la front. clos->gauche peut changer */
		for(y=0; y < clos->seqNbr; y++)
		{
		    clos->pos_[x][y] = 0;
		    k = clos->succFrontier[nn][x];
		    if(k == clos->aligSet[nn].pos[x]) 	
			k = clos->seq[x].succAligSetPos[k];
		    if(k <= clos->seq[x].longueur)
			while(k > 0)
			{
			    n = clos->seq[x].aligSetNbr[k];
			    if(clos->predFrontier[n][y] <
			       clos->predFrontier[nn][y])
			    {
				clos->pos_[x][y] = k;
				k = clos->seq[x].succAligSetPos[k];
			    }
			    else
				k = 0;
			}
		}

	for(x=0; x < clos->seqNbr; x++)
	    if(clos->droite1[x] != clos->droite2[x])  
		/* => la front. gauche peut changer */
		for(y=0; y < clos->seqNbr; y++)
		{
		    k = clos->succFrontier[nn][x];
		    if(k == clos->aligSet[nn].pos[x]) 
			k = clos->seq[x].succAligSetPos[k];
		    if(clos->pos_[x][y] > 0)
			while(k > 0 && k <= clos->pos_[x][y])
			{
			    n = clos->seq[x].aligSetNbr[k];
			    clos->predFrontier[n][y] =
				clos->predFrontier[nn][y];
			    k = clos->seq[x].succAligSetPos[k];
			}
		}

	for(x=0; x < clos->seqNbr; x++)
	    if(clos->gauche1[x] != clos->gauche2[x])  
		/* => la front. droite peut changer */
		for(y=0; y < clos->seqNbr; y++)
		{
		    clos->pos_[x][y] = 0;
		    k = clos->predFrontier[nn][x];
		    if(k > 0 && k == clos->aligSet[nn].pos[x])
			k = clos->seq[x].predAligSetPos[k];
		    while(k > 0)
		    {
			n = clos->seq[x].aligSetNbr[k];
			if(clos->succFrontier[n][y] >
			   clos->succFrontier[nn][y])
			{
			    clos->pos_[x][y] = k;
			    k = clos->seq[x].predAligSetPos[k];
			}
			else
			    k = 0;
		    }
		}

	for(x=0; x < clos->seqNbr; x++)
	    if(clos->gauche1[x] != clos->gauche2[x])
		/* => la front. clos->droite peut changer */
		for(y=0; y < clos->seqNbr; y++)
		{
		    k = clos->predFrontier[nn][x];
		    if(k > 0 && k == clos->aligSet[nn].pos[x])
			k = clos->seq[x].predAligSetPos[k];
		    if(clos->pos_[x][y] > 0)
			while(k >= clos->pos_[x][y])
			{
			    n = clos->seq[x].aligSetNbr[k];
			    clos->succFrontier[n][y] =
				clos->succFrontier[nn][y];
			    k = clos->seq[x].predAligSetPos[k];
			}
		}

	if(n1 == 0)
	{
	    for(k=i-1; k > 0 && clos->seq[seq1].aligSetNbr[k] == 0; k--)
		clos->seq[seq1].succAligSetPos[k] = i;
	    if(k > 0) 	
		clos->seq[seq1].succAligSetPos[k] = i;
	    for(k=i+1; k <= clos->seq[seq1].longueur 
		 && clos->seq[seq1].aligSetNbr[k] == 0; k++)
		clos->seq[seq1].predAligSetPos[k] = i;
	    if(k <= clos->seq[seq1].longueur) 	
		clos->seq[seq1].predAligSetPos[k] = i;
	}

	if(n2 == 0)
	{
	    for(k=j-1; k > 0 && clos->seq[seq2].aligSetNbr[k] == 0; k--)
		clos->seq[seq2].succAligSetPos[k] = j;
	    if(k > 0) 	
		clos->seq[seq2].succAligSetPos[k] = j;
	    for(k=j+1; k <= clos->seq[seq2].longueur 
		 && clos->seq[seq2].aligSetNbr[k] == 0; k++)
		clos->seq[seq2].predAligSetPos[k] = j;
	    if(k <= clos->seq[seq2].longueur) 	
		clos->seq[seq2].predAligSetPos[k] = j;
	}


	if (n1 > n2)
	{
	    n = n1;
	    n1 = n2;
	    n2 = n;
	}

	if (n2 == 0)	
	{
	    clos->nbrAligSets++;

	    edialign_realloc_closure(clos);
	}
	else
	{ 
	    if(n1 == 0)
	    {
		edialign_moveAligSet(clos, n2, nn);
	    }
	    else  
	    {
		edialign_moveAligSet(clos, n1, nn);

		if(n2 < clos->nbrAligSets)
		    edialign_moveAligSet(clos, n2, clos->nbrAligSets);
		clos->nbrAligSets--;

		edialign_realloc_closure(clos);
	    }
	}
    }

    return 0;
}




/* @funcstatic edialign_path ************************************************
**
** edialign_path
**
** @param [w] clos [edialignCLOSURE*] Undocumented
** @param [r] x [ajint] Undocumented
** @param [r] i [ajint] Undocumented
** @param [r] y [ajint] Undocumented
** @param [r] j [ajint] Undocumented
** @return [ajint] Undocumented
*****************************************************************************/

static ajint edialign_path(edialignCLOSURE *clos,
			   ajint x, ajint i, ajint y, ajint j)
{
    ajint n2;
    ajint k;

    if(x == y)
	return(i <= j);

    n2 = clos->seq[y].aligSetNbr[j];

    if(n2 == 0) 
    {
	k = clos->seq[y].predAligSetPos[j];
	if(k > 0)
	    n2 = clos->seq[y].aligSetNbr[k];
    }

    if(n2 == 0)
	return(0);

    return(i <= clos->predFrontier[n2][x]);
}




/* @funcstatic edialign_alignedPositions ************************************
**
** edialign_alignedPositions
**
** @param [w] clos [edialignCLOSURE*] Undocumented
** @param [r] x [ajint] Undocumented
** @param [r] i [ajint] Undocumented
** @param [r] y [ajint] Undocumented
** @param [r] j [ajint] Undocumented
** @return [ajint] Undocumented
*****************************************************************************/

static ajint edialign_alignedPositions(edialignCLOSURE *clos,
				       ajint x, ajint i,
				       ajint y, ajint j)
{

    return (x == y && i == j) || (clos->seq[x].aligSetNbr[i] != 0 &&
				  clos->seq[x].aligSetNbr[i] ==
				  clos->seq[y].aligSetNbr[j]);
}




/* @funcstatic edialign_alignablePositions ************************************
**
** edialign_alignablePositions
**
** @param [w] clos [edialignCLOSURE*] Undocumented
** @param [r] x [ajint] Undocumented
** @param [r] i [ajint] Undocumented
** @param [r] y [ajint] Undocumented
** @param [r] j [ajint] Undocumented
** @return [ajint] Undocumented
*****************************************************************************/

static ajint edialign_alignablePositions(edialignCLOSURE *clos,
					 ajint x, ajint i,
					 ajint y, ajint j)
{

    if(edialign_path(clos, x, i, y, j))
	return(edialign_path(clos, y, j, x, i));

    return(!edialign_path(clos, y, j, x, i));
}




/* @funcstatic edialign_addAlignedSegments **********************************
**
** edialign_addAlignedSegments
**
** @param [w] clos [edialignCLOSURE*] Undocumented
** @param [r] x [ajint] Undocumented
** @param [r] i [ajint] Undocumented
** @param [r] y [ajint] Undocumented
** @param [r] j [ajint] Undocumented
** @param [r] l [ajint] Undocumented
** @return [ajint] Undocumented
*****************************************************************************/

static ajint edialign_addAlignedSegments(edialignCLOSURE *clos,
					 ajint x, ajint i,
					 ajint y, ajint j, ajint l)
{
    ajint k;

    for(k=0; k < l; i++, j++, k++)
	edialign_addAlignedPositions(clos, x, i, y, j);

    return 0;
}




/* @funcstatic edialign_alignableSegments **********************************
**
** edialign_alignableSegments
**
** @param [w] clos [edialignCLOSURE*] Undocumented
** @param [r] x [ajint] Undocumented
** @param [r] i [ajint] Undocumented
** @param [r] y [ajint] Undocumented
** @param [r] j [ajint] Undocumented
** @param [r] l [ajint] Undocumented
** @return [ajint] Undocumented
*****************************************************************************/

static ajint edialign_alignableSegments(edialignCLOSURE *clos,
					ajint x, ajint i,
					ajint y, ajint j, ajint l)
{
    ajint k;

    for(k=0; k < l && edialign_alignablePositions(clos, x, i, y, j); i++, j++,
	k++);

    return(k==l);
}




#if 0
/* @funcstatic edialign_alignedSegments **********************************
**
** edialign_alignedSegments. Unused.
**
** @param [w] clos [edialignCLOSURE*] Undocumented
** @param [r] x [ajint] Undocumented
** @param [r] i [ajint] Undocumented
** @param [r] y [ajint] Undocumented
** @param [r] j [ajint] Undocumented
** @param [r] l [ajint] Undocumented
** @return [ajint] Undocumented
*****************************************************************************/

static ajint edialign_alignedSegments(edialignCLOSURE *clos,
				      ajint x, ajint i, ajint y,
				      ajint j, ajint l)
{
    ajint k;

    for(k=0; k < l && edialign_alignedPositions(clos, x, i, y, j); i++, j++,
	k++);

    return(k==l);
}
#endif




 /* on suppose que x!=y */

/* @funcstatic edialign_predFrontier **********************************
**
** edialign_predFrontier
**
** @param [w] clos [edialignCLOSURE*] Undocumented
** @param [r] x [ajint] Undocumented
** @param [r] i [ajint] Undocumented
** @param [r] y [ajint] Undocumented
** @return [ajint] Undocumented
*****************************************************************************/

static ajint edialign_predFrontier(edialignCLOSURE *clos,
				   ajint x, ajint i, ajint y)
{
    ajint n;
    ajint k;

    n = clos->seq[x].aligSetNbr[i];

    if(n == 0)
    {
	k = clos->seq[x].predAligSetPos[i];
	if(k > 0)
	    n = clos->seq[x].aligSetNbr[k];
    }

    if(n > 0)
	return(clos->predFrontier[n][y]);

    return(0);
}




 /* on suppose que x!=y */

/* @funcstatic edialign_succFrontier ****************************************
**
** edialign_succFrontier
**
** @param [w] clos [edialignCLOSURE*] Undocumented
** @param [r] x [ajint] Undocumented
** @param [r] i [ajint] Undocumented
** @param [r] y [ajint] Undocumented
** @return [ajint] Undocumented
*****************************************************************************/

static ajint edialign_succFrontier(edialignCLOSURE *clos,
				   ajint x, ajint i, ajint y)
{
    ajint n;
    ajint k;

    n = clos->seq[x].aligSetNbr[i];

    if(n == 0)
    {
	k = clos->seq[x].succAligSetPos[i];
	if(k > 0)
	    n = clos->seq[x].aligSetNbr[k];
    }

    if(n > 0)
	return(clos->succFrontier[n][y]);

    return(clos->seq[y].longueur+1);
}




/* @funcstatic edialign_anchor_check ****************************************
**
** edialign_anchor_check
**
** @param [r] s1 [ajint] Undocumented
** @param [r] s2 [ajint] Undocumented
** @param [r] b1 [ajint] Undocumented
** @param [r] b2 [ajint] Undocumented
** @param [r] l [ajint] Undocumented
** @param [r] scr [float] Undocumented
** @return [void]
*****************************************************************************/

static void edialign_anchor_check(ajint s1, ajint s2, ajint b1, ajint b2,
				  ajint l, float scr)
{
    (void) scr;				/* make it used */
    if(  
       ( s1 < 1 ) || 
       ( s1 > (ajint) seqnum ) 
       )
    { 
	ajFatal(" \n\n  wrong sequence # %d in anchoring file\n\n"
		"  data set consists only of %u sequences \n\n",
		s1,seqnum);
    }  

    if(  
       ( s2 < 1 ) || 
       ( s2 > (ajint) seqnum )  
       )
    { 
	ajFatal(" \n\n  wrong sequence # %d in anchoring file\n\n"
		"  data set consists only of %u sequences \n\n",
		s2,seqnum );
    }


    if( s1 == s2 )
    { 
	ajFatal("\n strange data in anchoring file:\n"
		" sequence # %d anchored with itself.\n\n", s1 );
    }
 


    /*
       if(  
       ( b1 < 1 ) || 
       ( b1 + l - 1 > seqlen[ s1 - 1 ] )  
       ) { 
       printf(" \n\n anchor # %d starts", anc_num + 1 ) ;
       printf(" at position %d in sequence %d and has a length of %d.\n",
       b1, s1, l ) ;
       printf(" This does not fit into sequence # %d " , s1 );
       printf(" (sequence length = %d) \n\n", seqlen[ s1 - 1 ] ) ; 
       printf("  PROGRAM TERMINATED \n\n" ) ;
       embExitBad() ; 
       } 
       */

    if( 
       ( b1 < 1 ) ||
       ( b1 + l - 1 > seqlen[ s1 - 1 ] )
       )
    {
	ajFatal(" \n\n  anchor # %d starts"
		" at position %d in sequence %d\n "
		" and is %d residues in length.\n"
		"  However, sequence %d"
		" is only %d residues in length \n\n",
		anc_num+1,b1,s1,l,s1,seqlen[s1-1]);
    }

    if( 
       ( b2 < 1 ) ||
       ( b2 + l - 1 > seqlen[ s2 - 1 ] )
       )
    {
	ajFatal(" \n\n  anchor # %d starts"
		" at position %d in sequence %d\n "
		" and is %d residues in length.\n"
		"  However, sequence %d"
		" is only %d residues in length \n\n",
		anc_num+1,b2,s2,l,s2,seqlen[s2 - 1]);
    }

    return;
}




/* @funcstatic edialign_multi_anc_read **************************************
**
** edialign_multi_anc_read
**
** @param [u] file_name [char*] File name
** @return [ajint] Undocumented
*****************************************************************************/

static ajint edialign_multi_anc_read(char *file_name)
{
    char anc_file_name[NAME_LEN];
    FILE *fp;
    struct multi_frag *current_frg ;
    char line[10000];
    ajint len;
    ajint beg1;
    ajint beg2;
    ajint seq1;
    ajint seq2;
    float wgt; 

    strcpy( anc_file_name , file_name );
    strcat( anc_file_name , ".anc" );

    if((fp = fopen( anc_file_name, "r")) == NULL)
	edialign_erreur("\n\n cannot find file with anchor points \n\n\n");

    if(( anchor_frg = ( struct multi_frag * ) calloc( 1 ,
					       sizeof( struct multi_frag ) ))
       == NULL) {
	printf(" problems with memory allocation "
	       "for `anchor fragments' !  \n \n");
	embExitBad();
    }

    current_frg = anchor_frg ; 


    while(fgets(line , MLINE , fp ) != NULL )
    {

	if(edialign_word_count( line ) == 6 )
	{   
	    sscanf(line,"%d %d %d %d %d %f ", &seq1 , &seq2 , &beg1, &beg2 ,
		   &len , &wgt );

	    edialign_anchor_check( seq1 , seq2 , beg1, beg2 , len , wgt ) ; 
 
	    seq1 = seq1 - 1 ; 
	    seq2 = seq2 - 1 ; 

	    current_frg->s[0] = seq1 ;
	    current_frg->s[1] = seq2 ;
	    current_frg->b[0] = beg1 ;
	    current_frg->b[1] = beg2 ;
	    current_frg->ext  = len ;
	    current_frg->weight  = wgt;


	    current_frg->next = (struct multi_frag *)
		calloc( 1 , sizeof(struct multi_frag) );

	    current_frg = current_frg->next;
	    anc_num++; 
	}
	else
	{ 
	    if(edialign_word_count( line ) != 0 )
	    {
		ajFatal("\n\n  Anchor file has wrong format. "
			"\n  Each line must contain 6 numbers! \n"
			"\n  Anchor file contains line \n\n"
			"         %s \n", line);
	    } 
	}
    }

    return 0;
}




/* @funcstatic edialign_frag_chain ******************************************
**
** edialign_frag_chain
**
** @param [r] n1 [ajint] Undocumented
** @param [r] n2 [ajint] Undocumented
** @param [u] fp1 [FILE*] Undocumented
** @param [u] fp_m [FILE*] Undocumented
** @param [u] number [ajuint*] Undocumented
** @return [float] Undocumented
*****************************************************************************/

static float edialign_frag_chain(ajint n1, ajint n2, FILE *fp1, FILE *fp_m,
				 ajuint *number)
{
    /* pairwise alignment */ 


    /* `i'  denotes positions in the 1. sequence ( seq[n1] ),
       `j'  denotes positions in the 2. sequence ( seq[n2] )  */


    ajint mot_match[ MAX_DIA * 3 ]; 

    ajint mot_match_num;

    float mot_wgt_sum;
    float this_mot_wgt ;

    float thr;	    /* threshold for the weight of fragments starting 
		       at a given point (i,j). For any new pair (i,j), 
		       thr = 0. However, if a fragment with positive weight w 
		       is found starting at (i,j), thr is defined to be w
		       and any further fragment starting at (i,j) is
		       taken into consideration, only if its weight excedes 
		       thr. This is, because it is not meaningful to consider
		       a fragment containing another fragment with 
		       higher weight. */ 
    ajint i;
    ajint j;
    ajint k;
    ajint diff1;
    ajint diff2;
    ajint hv;
    ajint numsubseq; 
    ajint ende2;       /* denote the last position considered in the 2nd 
		       sequence. Coincides with seqlen[n2], respectively, 
		       exept if nucleotide diagonals are translated into
		       peptide diagonals. In this case, 
		       ende2 = seqlen[n2]-2  */     
    ajint start_a ;   /* diagonals begining at a position (i,j) are only
		       considered if the similarity-value at (i,j)
		       exceeds a certain threshold, respectively if
		       seq[n1][i] = seq[n2][j]. In this case the value
		       of `start_a' is 1, otherwise the value is 0  */   
    ajint start_pep   = 0;
    ajint start_pep_c = 0;
    ajint start_dna   = 0;
    ajint start_dna1;
    ajint trpl_start;
    ajint trans;
    ajint crick_wgt = 0;
    ajint match_p;
    ajint match_p_c;
    ajint match_d;
    ajint kmaxloc; /* maximum length of diagonals starting at a given 
		      position (i,j) of the dot matrix. 
		      kmaxloc = min{ max_dia,seqlen[n1]-i+1 ,seqlen[n2]-j+1} */
    ajint lmax_real ;
    ajint mnum = 0;		      /* number of current diagonal */
    ajint *ub_int;      
    ajint *lb_int;     
    ajint limit;		      /* min { ub_int[i] ; ende2 }  */
    ajint bound_test; /* = 1 , if diagonal under consideration is consistent
		         with ub_int and lb_int. 
		         = 0 , if not.   */
    ajint max_nd = 0;
    ajint new_region = 0;
    ajint current_nd = 0;
    short accepted;
    char ch;
    
    float total_sum;
    float wgt_k_match;

    float thr2;
    float mot_wgt;

    struct pair_frag **diap; /* diap[i] = pointer to last diagonal ending 
				in the (i-1)-th column  */
    struct pair_frag **prec_vec; /* prec_vec[j] = pointer to diagonal with 
                                    maximum sum of weights accumulated 
                                    at a given position (i,j)  */
    struct pair_frag *current_dia, *hp, *cp, *cd;  

    FILE *fp_st;
    FILE *nd_fp;


    (void) fp_m;			/* make it used */

    /*
       printf( "\n  in frag_chain: iter = %d wgt_type = %d  \n\n", istep ,
       wgt_type );  
       printf( "\n  in frag_chain: iter = %d wgt_dna 20 = %f \n\n", istep ,
       wgt_dna[ 20 ][ 20 ] );  
       */

    if(print_status)
	if(seqnum > 20)
	{
	    fp_st = fopen( pst_name ,"w");

	    fprintf(fp_st,"\n\n\n    Status of the program run:\n");
	    fprintf(fp_st,"    ==========================\n\n");
	    fprintf(fp_st,"      %s \n\n", input_line);
	    fprintf(fp_st,"      iteration step %d in multiple alignment\n\n",
		    istep );
	    fprintf(fp_st,"      aligning seq %d /", n1 + 1 );
	    fprintf(fp_st," seq %d\n", n2 + 1);
	    fprintf(fp_st,"      total number of");
	    fprintf(fp_st," sequences: %d\n\n", seqnum);
	    fprintf(fp_st,"\n\n\n" );

	    fclose(fp_st);
	}




    if((ub_int = (int *) calloc( ( seqlen[n1] + 3 ) , sizeof(int) ) ) == NULL)
    {
	printf("problems with memory allocation for ub_int!  \n \n");
	embExitBad();
    }

    if((lb_int = (int *) calloc( (seqlen[n1]+3) , sizeof(int) ) ) == NULL)
    {
	printf("problems with memory allocation for lb_int!  \n \n");
	embExitBad();
    }
 
    if((prec_vec = (struct pair_frag **) 
	  calloc( (seqlen[n2]+3) , sizeof(struct pair_frag *) ) ) == NULL)
    {
	printf("problems with memory allocation for prec_vec!  \n \n");
	embExitBad();
    }

    if( 
       (diap = (struct pair_frag **) calloc( (seqlen[n1] + 3) , 
					    sizeof(struct pair_frag *) ))  ==
       NULL 
       )
    {
	printf("\n \n \n      ATTENTION: \n \n \n");
	printf("      problems with memory allocation\n");
	printf("      for diagonals! \n");
	embExitBad();
    }   


    for(i = 1; i<= seqlen[n1]; i++)
	diap[i] = NULL;


    if( (diap[0] = (struct pair_frag *) 
	  calloc( 1 , sizeof(struct pair_frag) ) ) == NULL)
    {
	printf("problems with memory allocation for diap!  \n \n");
	embExitBad();
    }

    for( j = 1 ; j< seqlen[n2]+3 ; j++ )
	prec_vec[j] = diap[0];


    if( dia_pa_file )
	fp_dpa = fopen( dia_pa_name , "a" );

    ende2 = seqlen[n2];

    /* Calculation of rel_weight  */


    if( iter_cond_prob == 0 )
    { 
	if( wgt_type == 0 )
	    edialign_rel_wgt_calc( seqlen[n1] , seqlen[n2] , wgt_prot );
	if( wgt_type % 2 )
	    edialign_rel_wgt_calc( seqlen[n1] , seqlen[n2] , wgt_dna);
	if( wgt_type > 1 )
	    edialign_rel_wgt_calc( seqlen[n1] , seqlen[n2] , wgt_trans);

	if( istep == 1 ) 
	    if( wgt_print || wgt_print_x ) { 
		edialign_wgt_prnt( ) ; 
		if( wgt_print_x ) 
		    embExitBad() ;
	    }

    } /* if( iter_cond_prob == 0 ) */  




    for( hv = 1 ; hv <= seqlen[ n1 ] ; hv++ )
    {
	lb_int[ hv ] = edialign_predFrontier( gabiosclos , n1 , hv , n2 );
	ub_int[ hv ] = edialign_succFrontier( gabiosclos , n1 , hv , n2 );
	if (lb_int[ hv ] != ub_int[ hv ])
	{
	    lb_int[ hv ]++;
	    ub_int[ hv ]--;
	}
    }

    mnum = 0; 

    if( iter_cond_prob || ( istep == 1 ) )
	new_region = 1;

    /* DP START */

    for( i = 1 ; i <= seqlen[n1] ; i++ )
    {

	if( open_pos[n1][n2][i] ) 
	{
	    if( new_region )
	    {

		diff2 = ( edialign_succFrontier(gabiosclos, n1, i , n2) 
			 - edialign_predFrontier(gabiosclos, n1, i , n2) -1 );

		if ( diff2 < 0 )
		    diff2 = 0;

		diff1 = ( edialign_succFrontier(gabiosclos, n2, lb_int[i] , n1)  
			 - edialign_predFrontier(gabiosclos, n2, lb_int[i] , n1)
			 -1 ) ;
		if ( diff1 < 0 )
		    diff1 = 0;

		/*
		   printf(" new region, i = %d diff = %d , %d \n", i, diff1 ,
		   diff2  ); 
		   */


		if( iter_cond_prob )
		    if( ( diff1 > 0 ) && ( diff2 > 0 ) ) {
			if( wgt_type == 0 )
			    edialign_rel_wgt_calc( diff1 , diff2 , wgt_prot );
			if( wgt_type % 2 )
			    edialign_rel_wgt_calc( diff1 , diff2 , wgt_dna );
			if( wgt_type > 1 )
			    edialign_rel_wgt_calc( diff1 , diff2 , wgt_trans );
		    }
 
	    }

	    limit = edialign_mini2( ub_int[i] , ende2 );   
	    for( j = lb_int[i] ; j <= limit ; j++ )
	    {

		if( wgt_type != 1 )
		    start_pep = ( sim_score[ amino[n1][i] ][ amino[n2][j] ] 
				 >=  thr_sim_score );

		if( crick_strand )
		    start_pep_c = (sim_score[amino_c[n1][i]][amino_c[n2][j]]
				   >=  thr_sim_score );

		if( wgt_type % 2 )
		{
		    if( strict )
			start_dna = ( (seq[n1][i] == seq[n2][j]) && 
				     ( seq[n1][i] == 'A' || 
				      seq[n1][i] == 'C' ||
				      seq[n1][i] == 'T' || 
				      seq[n1][i] == 'G' ||
				      seq[n1][i] == 'U'  ) );
  
		    else
			start_dna = (seq[n1][i] == seq[n2][j]);
 
		    if( dna_speed ) 
			if( ( i < seqlen[n1] ) && ( j < limit ) )
			{
			    if( strict )
				start_dna1 = ((seq[n1][i + 1] == seq[n2][j+1])
					      &&
					      (seq[n1][ i + 1 ] == 'A' ||
					       seq[n1][ i + 1 ] == 'C' ||
					       seq[n1][ i + 1 ] == 'T' ||
					       seq[n1][ i + 1 ] == 'G' ||
					       seq[n1][ i + 1 ] == 'U'  ) );
 
			    else
				start_dna1 = (seq[n1][i+1] == seq[n2][j+1]);
			    start_dna = start_dna * start_dna1 ;
			}

		}

		if( wgt_type != 1 )
		    start_a = start_pep ;
		else 
		    start_a = start_dna ; 
              
		if( wgt_type == 3 ) 
		    start_a =  start_pep + start_dna ; 

		if( crick_strand )
		    start_a =  start_a + start_pep_c ;

		if( self_comparison ) 
		    if( i == j ) 
			start_a = 0 ; 

		if( exclude_frg ) 
		    if( j == gl_exclude_list[ n1 ][ n2 ][ i ] ) 
			start_a = 0 ; 

		if( start_a )
		{

		    /*match = 0;*/
		    match_d = 0;
		    match_p = 0;
		    match_p_c = 0;
		    thr = 0;
		    /*
		       start_count++ ;
		       */
		    bound_test = 1;

		    if( wgt_type > 1 ) 
			lmax_real = lmax * 3 ;
		    else
			lmax_real = lmax ;

		    kmaxloc = 
			edialign_mini3(lmax_real , seqlen[n1]-i+1 ,
				       seqlen[n2]-j+1 ); 

		    if( motifs )
		    {   
			for( k = 1 ; k <= kmaxloc ; k++ )  
			    if( ( mot_pos[ n1 ][ i + k - 1 ] == 1 ) &&  
			       ( mot_pos[ n2 ][ j + k - 1 ] == 1 ) ) { 
				mot_match[ k ] = 1 ;
				/*     printf(" match in  %d  %d  %d \n",
				       i, j, k ); */  
			    }  
			    else 
				mot_match[ k ] = 0 ; 
		    }
 
		    /*******************\
		     *                   *
		     *  fragments start  *
		     *                   *
		     \*******************/

		    k = 1; 
		    mot_match_num = 0; 
		    mot_wgt_sum = 0 ; 
 
		    while( ( k <= kmaxloc ) && start_a )
		    {
			if( motifs )
			{
			    if((( i- j)*(i-j)) <
			       (max_mot_offset * max_mot_offset)) 
				if( k >= mot_len )  
				    if( mot_match[ k - mot_len + 1] )
				    {
					/*mot_offset = ( i - j ) ;*/
					this_mot_wgt = edialign_mot_dist_factor
					    ((i-j),
					     mot_offset_factor); 

					/*
					   printf("  i - j = %d , tmw = %f \n",
					   i - j , this_mot_wgt );
					   */
					mot_wgt_sum = mot_wgt_sum +
					    this_mot_wgt;
					mot_match_num++ ; 

				    }
			}

			if( open_pos[n1][n2][ i + k - 1 ] ) 
			{
			    bound_test = bound_test *
				( j + k - 1 >= lb_int[ i + k - 1 ] );
			    bound_test = bound_test *
				( j + k - 1 <= ub_int[ i + k - 1 ] );

			    trpl_start = 0;

			    if( wgt_type < 2 ) 
				trans = 0 ;
			    else
				trans = 1 ;
 
			    if( start_pep || 
			       ( crick_strand && start_pep_c ) 
			       )
				if((wgt_type > 1 ) && ((k % 3) == 1))
				{ 
				    trpl_start = 1 ;
  
				    trpl_start = trpl_start *
					( j + k >= lb_int[ i + k ] );
				    trpl_start = trpl_start *
					( j + k <= ub_int[ i + k ] );
				    trpl_start = trpl_start *
					open_pos[ n1 ][ n2 ][ i + k ] ;


				    trpl_start = trpl_start *
					( j + k + 1 >= lb_int[ i + k + 1 ] );
				    trpl_start = trpl_start *
					( j + k + 1 <= ub_int[ i + k + 1 ] );
				    trpl_start = trpl_start *
					open_pos[ n1 ][ n2 ][ i + k + 1 ] ;
				}


 
			    if( 
			       bound_test && 
			       ( ( wgt_type != 2 ) || trpl_start )
			       )
			    {
				if( start_pep )
				    if( 
				       ( wgt_type == 0 ) || 
				       ( ( wgt_type > 1) && trpl_start ) 
				       ) 
					match_p = match_p
					    + sim_score[amino[n1][i+k-1]]
						[amino[n2][j+k-1]];

				if( crick_strand )
				    if( start_pep_c )
					if(
					   ( wgt_type == 0 ) ||
					   ( ( wgt_type > 1) && trpl_start )
					   )
					    match_p_c = match_p_c
						+ sim_score[amino_c[n1][i+k-1]]
						    [amino_c[n2][j+k-1]];


				if( start_dna )
				    if( wgt_type % 2 ) 
					if( !strict ||
					   (seq[n1][i+k-1] == 'A' || 
					    seq[n1][i+k-1] == 'C' ||
					    seq[n1][i+k-1] == 'T' || 
					    seq[n1][i+k-1] == 'G' ||
					    seq[n1][i+k-1] == 'U'  )) 
					    match_d = match_d + 
						(seq[n1][i+k-1] ==
						 seq[n2][j+k-1] );

				wgt_k_match = 0;


				if( wgt_type == 0 )
				    wgt_k_match = wgt_prot[ k ][ match_p ]; 
				if( wgt_type == 1 )
				    wgt_k_match = wgt_dna[ k ][ match_d ]; 

				if( wgt_type > 1 )
				{ 
				    if( start_pep ) 
					wgt_k_match = wgt_trans[(k+2) / 3]
					    [match_p]; 

				    if( crick_strand ) 
					if( start_pep_c ) {
					    if( wgt_trans[ ( k + 2 ) / 3 ]
					       [ match_p_c ] > wgt_k_match )
					    { 
						wgt_k_match = wgt_trans[(k+2) /
							   3 ][ match_p_c ] ; 
						crick_wgt = 1 ;
					    }
					    else
						crick_wgt = 0 ;
					}
				}

				if( start_dna ) 
				    if( wgt_type == 3 )
					if( k <= lmax )
					    if( wgt_dna[ k ][ match_d ] 
					       > wgt_k_match ) {
						wgt_k_match = wgt_dna[k]
						    [match_d];   
						trans = 0 ;
					    }



				if( wgt_type == 0 )
				    if( match_p <= ( k * av_sim_score_pep ) ) 
					start_pep = 0;
                            
				if( wgt_type == 1 )
				    if( match_d <= ( k * av_sim_score_nuc ) )
					start_dna = 0;

				if( start_pep ) 
				    if( wgt_type > 1 )
					if((match_p * 3 ) <=
					   (k * av_sim_score_pep)) 
					    start_pep = 0;

				if( start_pep_c )
				    if( wgt_type > 1 )
					if((match_p_c * 3 ) <=
					   (k * av_sim_score_pep))
					    start_pep_c = 0;


				if( wgt_type != 1 )
				    start_a = start_pep ;
				else
				    start_a = start_dna ;

				if( wgt_type == 3 )
				    start_a =  start_pep + start_dna ;

				if( crick_strand )
				    start_a = start_a + start_pep_c ;

				if( exclude_frg ) 
				    if(gl_exclude_list[ n1 ][ n2 ][ i + k ] == j +
				       k )
					start_a = 0 ; 
 

				if( motifs )
				    if( mot_wgt_sum > 0 ) {
					fprintf( fp_mot , "  %4d %4d  ",
						n1 + 1, n2 + 1);
					fprintf( fp_mot , "  %4d %4d %3d  ",
						i, j, k );  
					fprintf( fp_mot , "  %5.2f ",
						wgt_k_match );  
					mot_wgt = mot_wgt_sum * mot_factor ; 
					wgt_k_match = wgt_k_match + mot_wgt ;
					fprintf( fp_mot , "  %2d ",
						mot_match_num ); 
					fprintf( fp_mot , "     %5.2f \n",
						wgt_k_match ); 
				    }
 
				/* 
				   if( wgt_k_match > 0 )
				   printf(" k = %d min_dia = %d "
				   "wgt_k_match = %f thr = %f \n",
				   k, min_dia , wgt_k_match , thr );
				   */
				if( k >= min_dia )  
				    if( wgt_k_match > thr  )
				    {   
					if( (current_dia = (struct pair_frag *)
					     calloc(1 ,
						    sizeof(struct pair_frag) ))
					   == NULL )
					{ 
					    printf("\n \n \n      ATTENTION:"
						   " \n \n \n");
					    printf("      too many diagonals "
						   "in\n");
					    printf("      pairwise alignment "
						   "of");
					    printf(" sequences\n");
					    printf("      %s  and  ",
						   seq_name[n1]);
					    printf("%s\n \n \n \n",
						   seq_name[n2]);

					    fprintf(fp1,"\n \n      ATTENTION:"
						    "\n \n");
					    fprintf(fp1,"      too many "
						    "diagonals\n");
					    fprintf(fp1,"      in pairwise "
						    "alignment");
					    fprintf(fp1," of sequences\n");
					    fprintf(fp1,"      %s  and  ",
						    seq_name[n1]);
					    fprintf(fp1,"%s\n \n \n \n",
						    seq_name[n2]);

					    embExitBad();
					} 
  

 
					current_dia->b1 = i;
					current_dia->b2 = j;
					current_dia->ext = k + 2 * trans ;
					current_dia->weight = wgt_k_match ;
					current_dia->trans = trans ;
					current_dia->cs = crick_wgt ;
					current_dia->sum  = current_dia->weight
					    + (prec_vec[j])->sum ;
					current_dia->prec = prec_vec[j];
					current_dia->last 
					    = diap[ i + (current_dia->ext)];
					diap[i+(current_dia->ext)] =
					    current_dia;

					mnum++; 

					if(print_max_nd)
					{
					    current_nd++ ;
					    if( current_nd > max_nd )
						max_nd = current_nd;
					}

					dia_num++;
					if(afc_file)
					{    
					    fprintf(fp_dia,"FRG %d ",
						    dia_num ); 
					    fprintf(fp_dia,"name: %s ",
						    seq_name[ n1 ]); 
					    fprintf(fp_dia," %s ",
						    seq_name[ n2 ]); 
					    if( seqnum > 2 )
					    { 
						fprintf(fp_dia,"  seq: %d",
							n1 + 1 ); 
						fprintf(fp_dia," %d" ,
							n2 + 1 ); 
					    }
					    fprintf(fp_dia,"  beg: %d %d",
						    i, j ); 
					    fprintf(fp_dia,"  len: %d",
						    current_dia->ext );
					    fprintf(fp_dia,"  wgt: %6.3f",
						    current_dia->weight );

					    /*
					       if( BETA )
					       if( iter_cond_prob )
					       { 
					       fprintf(fp_dia,"   d1 = %d "
					       "d2 = %d ", diff1, diff2 ); 
					       }
					       */

					    fprintf(fp_dia,"  it = %d ",
						    istep ); 
					    if( ( wgt_type == 3 ) ||
					       crick_strand )
					    {
						if( current_dia->trans )
						    fprintf(fp_dia," P-frg" );
						else
						    fprintf(fp_dia," N-frg" );
					    }
					    
					    fprintf(fp_dia,"\n");     
					    if( afc_filex ) {
						fprintf(fp_dia,"SEG1 "); 
						for(hv = 0;hv <
						    current_dia->ext ; hv++)
						{
						    ch = seq[n1][ i + hv ]; 
						    fprintf(fp_dia,"%c" , ch);
						}
						fprintf(fp_dia ,"\n");  
						fprintf(fp_dia,"SEG2 "); 
						for(hv = 0;hv <
						    current_dia->ext ; hv++)
						{
						    ch = seq[n2][ j + hv ] ; 
						    fprintf(fp_dia,"%c" , ch);
						}
						fprintf(fp_dia ,"\n\n");  
					    }
					}

					if( ! redundant )
					{
					    thr2 = edialign_maxf2(thr,
						      (current_dia->weight));
					    thr = thr2 ;
					}  
                                    
				    } /*   if( wgt[k][match] > thr  )  */
			    } /*   if ( bound_test )              */
			} /*   if( open_pos ...  )  */
			k++;
		    } /*   while( ( k <= kmaxloc ) && start_a ) */
		} /*   if( start_a )                    */
	    } /*   for(j=lb_int[i];j<=limit;j++)   */
	    new_region = 0;     
	} /*   if( open_pos )                     */
	else
	    if( iter_cond_prob )

		new_region = 1;


	if(print_status)
	    if((( seqlen[n1] + seqlen[n2] ) > 1000))
		if( ! ( i % 100 ) )
		{
		    fp_st = fopen( pst_name ,"w");

		    fprintf(fp_st,"\n\n\n    Status of the program run:\n");
		    fprintf(fp_st,"    ==========================\n\n");
		    fprintf(fp_st,"      %s \n\n", input_line);
		    if( seqnum > 2 )
		    {
			fprintf(fp_st,"      iteration step %d in" , istep ); 
			fprintf(fp_st," multiple alignment\n\n" );
		    } 
		    if( seqnum > 2 )
		    {
			fprintf(fp_st,"      aligning seq %d /", n1 + 1 );
			fprintf(fp_st," seq %d\n", n2 + 1);
			fprintf(fp_st,"      total number of");
			fprintf(fp_st," sequences: %d\n\n", seqnum);
		    }
		    fprintf(fp_st,"      current position in");
		    fprintf(fp_st," sequence %d:  %8d\n", n1 + 1, i);
		    fprintf(fp_st,"      length of seq %d:", n1 + 1 );
 
		    fprintf(fp_st,"                 %8d\n\n", seqlen[n1]);


		    /*
		       if( iter_cond_prob || ( istep == 1 ) )
		       {
		       if( open_pos[n1][n2][i] )
		       {
		       fprintf(fp_st,"      diff1 = %d \n", diff1 );
		       fprintf(fp_st,"      diff2 = %d \n", diff2 );
		       }
		       else
		       fprintf(fp_st,"      position already aligned");
		       }
		       */

		    fprintf(fp_st,"\n\n\n" );

		    fclose(fp_st);
		}



	cp = diap[ i + 1 ];
	hp = NULL;     
	accepted = 0; 


	while( cp != NULL  )
	{
	    j = cp->b2 + cp->ext;
	    if( (prec_vec[j])->sum < cp->sum )
	    {
		prec_vec[j] = cp;
		accepted = 1;

		hp = cp; 
		cp = cp->last;
	    }
	    else
	    {
		cp = cp->last;

		if( accepted )
		{
		    free( hp->last );

   
		    hp->last = cp;
		} 
		else
		{ 
		    free( diap[ i + 1 ] );
		    diap[ i + 1 ] = cp;
		}  

		current_nd--;

	    }
	}

	for( hv=2 ; hv < ( seqlen[n2] + 3 ) ; hv++ )
	    if( (prec_vec[hv])->sum < (prec_vec[hv-1])->sum )
		prec_vec[hv] = prec_vec[hv-1];



    } /*   for(i= ... )                   */
    /*
       printf (" start_count = %d \n ", start_count );
       */
    if( pr_av_max_nd )
	if( istep == 1 )
	{
	    if( ( nd_fp = fopen("nd_file","a")) == NULL)    
	    {
		printf("\n\n  nd_fp could not be opened \n\n" );
		embExitBad();
	    }     
   
	    fprintf(nd_fp, " %2d/%2d %8d  %8d \n",n1+1,n2+1,mnum,max_nd);
	    fclose( nd_fp );
	}
   

    numsubseq = 0;		   /* counts diagonals in alignment */

    current_dia = prec_vec[ seqlen[n2] + 1 ];
    cd = current_dia;

   
    total_sum = cd->sum;
     

    while( cd->prec != NULL )
    {
	numsubseq++;
	cd = cd->prec;
    }



    if(numsubseq)
    {
	hv = numsubseq - 1;

	if ( 
	    ( 
	     pair_dia = (struct multi_frag *) 
	     calloc( ( numsubseq + 1 ) , sizeof(struct multi_frag)))  == NULL)
	{
	    printf("problems with memory allocation for `pair_dia'!  \n \n");
	    embExitBad();
	}


	while(hv>=0)
	{
	    if( dia_pa_file )
	    {
		fprintf(fp_dpa, " %3d) " , ++dcount );
		if( seqnum > 2 )
		    fprintf(fp_dpa, "seq: %3d %3d " , n1 + 1, n2 + 1);
		fprintf(fp_dpa, " beg: %6d %6d ", current_dia->b1,
			current_dia->b2);
		fprintf(fp_dpa, " len: %2d ", current_dia->ext );
		fprintf(fp_dpa, " weight: %5.2f ", current_dia->weight ); 
		fprintf(fp_dpa, " it: %d ", istep ); 
		if( ( wgt_type == 3 ) || crick_strand )
		{
		    if( current_dia->trans )
			fprintf(fp_dpa," P-frg" );
		    else
			fprintf(fp_dpa," N-frg" );
		}
		
		if( current_dia->trans )
		    if( crick_strand)
		    { 
			if( current_dia->cs )  
			    fprintf(fp_dpa," crick " ) ;  
			else 
			    fprintf(fp_dpa," watson " ) ;   
		    }
		fprintf(fp_dpa,"\n");
	    }


	    (pair_dia[hv]).b[0] = current_dia->b1;
	    (pair_dia[hv]).b[1] = current_dia->b2;
	    (pair_dia[hv]).s[0] = n1;
	    (pair_dia[hv]).s[1] = n2;
	    (pair_dia[hv]).sel = 1;
	    (pair_dia[hv]).ext = current_dia->ext;
	    (pair_dia[hv]).weight = current_dia->weight;
	    (pair_dia[hv]).ow = current_dia->weight;
	    (pair_dia[hv]).trans = current_dia->trans ;
	    if( crick_strand )
	    {
		(pair_dia[hv]).cs = current_dia->cs ;
	    }
	    (pair_dia[hv]).it = istep; 
	    hv--;
	    current_dia = current_dia->prec;
	}
	/*     if( dia_pa_file )
	       fprintf(fp_dpa, " \n" );
	       */     


	/*    modified in LGI-VITRY 
	      if( iter_cond_prob )
	      */

	cont_it = 1 ;   

    } /* if(numsubseq) */

            

    *number = numsubseq;


    if( long_output )
    {
	printf("Seq. %3d -%3d: ", n1+1, n2+1);
	printf("T = %2.2f,", threshold); 
	printf(" %3d D. in alignment,", *number);
	printf("%6d D. in matrix", mnum);
	printf("\n");
    } 

 

    for( hv=0 ; hv < seqlen[n1]+3 ; hv++ )
    {
	current_dia = diap[hv];
     
	while( current_dia != NULL )
	{
	    hp = current_dia;
	    current_dia = current_dia->last;
	    free(hp);
	}
    }    


    if( istep == 1 )
    {
	max_dia_num = max_dia_num + max_nd ;
    }
 
    free(diap);

    free(ub_int);
    free(lb_int);
    free(prec_vec);
    if(  dia_pa_file )
    {
	fclose( fp_dpa );
    }

    return( total_sum);
}




#if 0
/* @funcstatic edialign_num_test ******************************************
**
** edialign_num_test
**
** @param [u] cp [char*] Undocumented
** @return [ajint] Undocumented
*****************************************************************************/

static ajint edialign_num_test( char *cp )
{ 
    ajint result = 1;
    ajint i;
    char *strng;
    ajint len;
    
    strng = cp;

    len = strlen(strng);
    
    for(i = 0 ; i < len ; i++ )
	if( ! isdigit(strng[i]) && ( strng[i] != '.' ) )
        {
	    result = 0;
	    /*     printf("\n %c is no digit !!!\n", strng[i]);   */
        } 
         
    return result ;    
}
#endif




/* @funcstatic edialign_minf2 ******************************************
**
** edialign_minf2
**
** @param [r] a [float] Undocumented
** @param [r] b [float] Undocumented
** @return [float] Undocumented
*****************************************************************************/

static float edialign_minf2(float a, float b)
{
    if (a<b)
	return a;
 
    return b;
}




/* @funcstatic edialign_maxf2 ******************************************
**
** edialign_maxf2
**
** @param [r] a [float] Undocumented
** @param [r] b [float] Undocumented
** @return [float] Undocumented
*****************************************************************************/

static float edialign_maxf2(float a, float b)
{
    if (a>b)
	return a;

    return b;
}




/* @funcstatic edialign_mini2 ******************************************
**
** edialign_mini2
**
** @param [r] a [ajint] Undocumented
** @param [r] b [ajint] Undocumented
** @return [ajint] Undocumented
*****************************************************************************/

static ajint edialign_mini2(ajint a, ajint b)
{
    if(a<b)
	return a;

    return b;
}




/* @funcstatic edialign_minu2 ******************************************
**
** edialign_mini2
**
** @param [r] a [ajint] Undocumented
** @param [r] b [ajint] Undocumented
** @return [ajuint] Undocumented
*****************************************************************************/

static ajuint edialign_minu2(ajint a, ajint b)
{
    ajuint ia;
    ajuint ib;

    if(a<0)
	ia = 0;
    else
	ia = a;

    if(b<0)
	ib = 0;
    else
	ib = b;

    if(a<b)
	return ia;

    return ib;
}




/* @funcstatic edialign_maxi2 ******************************************
**
** edialign_maxi2
**
** @param [r] a [ajint] Undocumented
** @param [r] b [ajint] Undocumented
** @return [ajint] Undocumented
*****************************************************************************/

static ajint edialign_maxi2(ajint a, ajint b)
{
    if (a>b)
	return a;

    return b;
}




/* @funcstatic edialign_mini3 ************************************************
**
** edialign_mini3
**
** @param [r] a [ajint] Undocumented
** @param [r] b [ajint] Undocumented
** @param [r] c [ajint] Undocumented
** @return [ajint] Undocumented
*****************************************************************************/

static ajint edialign_mini3(ajint a, ajint b, ajint c)
{
    return edialign_mini2(a, edialign_mini2(b,c));
}




#if 0
/* @funcstatic edialign_minf ************************************************
**
** edialign_minf
**
** @param [u] a [float*] Undocumented
** @param [r] b [float] Undocumented
** @return [void]
*****************************************************************************/

static void edialign_minf(float *a, float b)
{
    if (*a > b)
	*a = b;

    return;
}
#endif




/* @funcstatic edialign_mini ************************************************
**
** edialign_mini
**
** @param [u] a [ajint*] Undocumented
** @param [r] b [ajint] Undocumented
** @return [void]
*****************************************************************************/

static void edialign_mini(ajint *a, ajint b)
{
    if (*a > b)
	*a = b;

    return;
}




#if 0
/* @funcstatic edialign_maxf ************************************************
**
** edialign_maxf. Unused.
**
** @param [u] a [float*] Undocumented
** @param [r] b [float] Undocumented
** @return [void]
*****************************************************************************/

static void edialign_maxf(float *a, float b)
{
    if (*a < b)
	*a = b;

    return;
}
#endif




/* @funcstatic edialign_maxi ************************************************
**
** edialign_maxi
**
** @param [u] a [ajint*] Undocumented
** @param [r] b [ajint] Undocumented
** @return [void]
*****************************************************************************/

static void edialign_maxi(ajint *a, ajint b)
{
    if (*a < b)
	*a = b;

    return;
}




/* @funcstatic edialign_maxu ************************************************
**
** edialign_maxu
**
** @param [u] a [ajuint*] Undocumented
** @param [r] b [ajuint] Undocumented
** @return [void]
*****************************************************************************/

static void edialign_maxu(ajuint *a, ajuint b)
{
    if (*a < b)
	*a = b;

    return;
}




/* @funcstatic edialign_invert **********************************************
**
** edialign_invert
**
** @param [r] c1 [char] Undocumented
** @return [char] Undocumented
*****************************************************************************/

static char edialign_invert ( char c1 )
{
    char c2 = c1;

    if(c1 == 'T')
	c2 = 'A' ; 
    if(c1 == 'A')
	c2 = 'T' ;
    if(c1 == 'C')
	c2 = 'G' ;
    if(c1 == 'G')
	c2 = 'C' ;
  
    return( c2 );
}




/* @funcstatic edialign_translate *******************************************
**
** edialign_translate
**
** @param [r] c1 [char] Undocumented
** @param [r] c2 [char] Undocumented
** @param [r] c3 [char] Undocumented
** @param [r] seqno [ajint] Undocumented
** @param [r] pos [ajint] Undocumented
** @return [ajint] Undocumented
*****************************************************************************/

static ajint edialign_translate(char c1, char c2 ,char c3, ajint seqno,
				ajint pos)
{
    /* translation of triplets into amino acids */
  

    ajint  amac = 0;				/* resulting amino acid */

    (void) seqno;			/* make it used */
    (void) pos;				/* make it used */

    if(c1 == 'T')
    {
	if(c2 == 'T')
	{
	    if(c3 == 'T') amac = 18;
	    if(c3 == 'C') amac = 18;
	    if(c3 == 'A') amac = 16;
	    if(c3 == 'G') amac = 16;
	}
	if(c2 == 'C') amac = 2;
	if(c2 == 'A')
	{
	    if(c3 == 'T') amac = 19;
	    if(c3 == 'C') amac = 19;
	    if(c3 == 'A') amac = 0;     /* stop codon */
	    if(c3 == 'G') amac = 0;
	}
	if(c2 == 'G')
	{
	    if(c3 == 'T') amac = 1;
	    if(c3 == 'C') amac = 1;
	    if(c3 == 'A') amac = 20;
      
	    if(c3 == 'G') amac = 20;
	}
    }

    if(c1 == 'C')
    {
	if(c2 == 'T') amac = 16;
	if(c2 == 'C') amac = 4;
	if(c2 == 'A')
	{
	    if(c3 == 'T') amac = 11;
	    if(c3 == 'C') amac = 11;
	    if(c3 == 'A') amac = 10;
	    if(c3 == 'G') amac = 10;
	}
	if(c2 == 'G') amac = 12;
    }

    if(c1 == 'A')
    {
	if(c2 == 'T')
	{
	    if(c3 == 'T') amac = 15;
	    if(c3 == 'C') amac = 15;
	    if(c3 == 'A') amac = 15;
	    if(c3 == 'G') amac = 14;
	}
	if(c2 == 'C') amac = 3;
	if(c2 == 'A')
	{
	    if(c3 == 'T') amac = 7;
	    if(c3 == 'C') amac = 7;
	    if(c3 == 'A') amac = 13;
	    if(c3 == 'G') amac = 13;
	}
	if(c2 == 'G')
	{
	    if(c3 == 'T') amac = 2;
	    if(c3 == 'C') amac = 2;
	    if(c3 == 'A') amac = 12;
	    if(c3 == 'G') amac = 12;
	}
    }

    if(c1 == 'G')
    {
	if(c2 == 'T') amac = 17;
	if(c2 == 'C') amac = 5;
	if(c2 == 'A')
	{
	    if(c3 == 'T') amac = 8;
	    if(c3 == 'C') amac = 8;
	    if(c3 == 'A') amac = 9;
	    if(c3 == 'G') amac = 9;
	}
	if(c2 == 'G') amac = 6;
    }


    if( 
       ( c1 != 'A'  &&  c1 != 'T'  &&  c1 != 'G'  &&  c1 != 'C' )  ||
       ( c2 != 'A'  &&  c2 != 'T'  &&  c2 != 'G'  &&  c2 != 'C' )  ||
       ( c3 != 'A'  &&  c3 != 'T'  &&  c3 != 'G'  &&  c3 != 'C' )  
       ) 
	return( 0 );

    return( amac );
} /*  translate */




#if 0
/* @funcstatic edialign_int_test ********************************************
**
** edialign_int_test. Unused.
**
** @param [r] f [float] Undocumented
** @return [ajint] Undocumented
*****************************************************************************/

static ajint edialign_int_test(float f)
{
    ajint i = f;

    if(i == f) 
	return (1);
  
    return (0);
}
#endif




/* @funcstatic edialign_change ***********************************************
**
** edialign_change
**
** @param [u] a [struct multi_frag*] Undocumented
** @param [u] b [struct multi_frag*] Undocumented
** @return [void]
*****************************************************************************/

static void edialign_change(struct multi_frag *a, struct multi_frag *b)
{
    struct multi_frag c;
    struct multi_frag *an;
    struct multi_frag *bn;
  
    c = *a;
    an = a->next;
    bn = b->next;
  
    *a = *b;
    *b = c;
  
    a->next = an;
    b->next = bn;

    return;
}




#if 0
/* @funcstatic edialign_pair_change *****************************************
**
** edialign_pair_change. Unused.
**
** @param [u] a [struct seq_pair*] Undocumented
** @param [u] b [struct seq_pair*] Undocumented
** @return [void]
*****************************************************************************/

static void edialign_pair_change(struct seq_pair *a, struct seq_pair *b)
{
    struct seq_pair c;
  
    c = *a;
    *a = *b;
    *b = c;

    return;
}
#endif




/* @funcstatic edialign_ow_bubble_sort **************************************
**
** edialign_ow_bubble_sort
**
** @param [r] number [int] Undocumented
** @param [u] dp [struct multi_frag*] Undocumented
** @return [void]
*****************************************************************************/

static void edialign_ow_bubble_sort( int number , struct multi_frag *dp )
{ 
    /* sorting diagonals in multiple alignment according to their
       overlap weights */
  
    struct multi_frag *hp;
    ajint hv1;
    ajint hv2;
  
    FILE *fp_st;

    for( hv1 = 1 ; hv1 < number ; hv1++ )
    { 
	hp = dp;
      
	if( print_status )
	    if( ( hv1 % 100 ) == 0 )
	    {         
		fp_st = fopen( pst_name ,"w");
	    
		fprintf(fp_st,"\n\n\n    Status of the program run:\n");  
		fprintf(fp_st,"    ==========================\n\n");  
		fprintf(fp_st,"      %s \n\n", input_line);
		fprintf(fp_st,"      iteration step %d in multiple "
			"alignment\n", istep );
		fprintf(fp_st,"      overlap weight sorting of diagonals\n");
		fprintf(fp_st,"      current diagonal = %d\n\n", hv1 );
		fprintf(fp_st,"      total number of"); 
		fprintf(fp_st," diagonals: %d\n\n\n\n", number);
		fclose(fp_st);
	    }
      
      
	for( hv2 = hv1 ; hv2 < number ; hv2++ )
	{
	    if( hp->ow < (hp->next)->ow )
		edialign_change( hp , hp->next );
            hp = hp->next;
	} 
    }

    return;
} /*  ow_bubble_sort */




/* @funcstatic edialign_bubble_sort **************************************
**
** edialign_bubble_sort
**
** @param [r] number [ajint] Undocumented
** @param [u] dp [struct multi_frag*] Undocumented
** @return [void]
*****************************************************************************/

static void edialign_bubble_sort(ajint number , struct multi_frag *dp )
{ 
    /* sorting diagonals in multiple alignment according to their
       individual weights */
  
    struct multi_frag *hp;
    int hv1, hv2;
    FILE *fp_st;

    for( hv1 = 1 ; hv1 < number ; hv1++ )
    { 
	hp = dp;
      
	if( print_status )
	    if( ( hv1 % 100 ) == 0 )
	    {   
		fp_st = fopen( pst_name ,"w");
	    
		fprintf(fp_st,"\n\n\n    Status of the program run:\n");  
		fprintf(fp_st,"    ==========================\n\n");  
		fprintf(fp_st,"      %s \n\n", input_line);
		fprintf(fp_st,"      iteration step %d\n", istep );
		fprintf(fp_st,"      ind. weight sorting of diagonals\n");
		fprintf(fp_st,"      current diagonal = %d\n\n", hv1 );
		fprintf(fp_st,"      total number of"); 
		fprintf(fp_st," diagonals: %d\n\n\n\n", number);
		fclose(fp_st);
	    }
      
      
	for( hv2 = hv1 ; hv2 < number ; hv2++ )
	{ 
	    if( hp->weight < (hp->next)->weight )
		edialign_change( hp , hp->next );
	    hp = hp->next;
	}  
    }

    return;
} /*  bubble_sort */




/* @funcstatic edialign_change_struct_el ************************************
**
** edialign_change_struct_el
**
** @param [u] a [struct multi_frag**] Undocumented
** @param [r] l [ajint] Undocumented
** @param [r] r [ajint] Undocumented
** @return [void]
*****************************************************************************/

static void edialign_change_struct_el(struct multi_frag **a, ajint l, ajint r)
{
    struct multi_frag *dummy;

    dummy = a[l];
    a[l]  = a[r];
    a[r]  = dummy;  

    return;
}




/* @funcstatic edialign_change_first ***************************************
**
** edialign_change_first
**
** @param [u] a [struct multi_frag*] Undocumented
** @param [u] b [struct multi_frag*] Undocumented
** @return [void]
*****************************************************************************/

static void edialign_change_first(struct multi_frag *a, struct multi_frag *b)
{
    struct multi_frag c;
    struct multi_frag *an;
    struct multi_frag *bn;

    if(a==b)
    {
	/* Change the first list-element with the second one
	 ** (old first-el.).
	 */
	c  = *a;
	an = a->next;
	bn = a->next->next;

	*a = *(a->next);
	a->next = bn;
	
	*an = c;
	an->next = a;
    }
    else    /* Change the new first list-el. with the old first-el. */
    {
        /* Make a copy of the new first-listelement a. */
	c  = *a;
        /* Make a copy of the pointer at the second-el. */ 
	an = a->next;
        /* Make a copy of the pointer old first-el. shows at. */ 
	bn = b->next->next;
	
	/* Write the value of the old first-el. on the place of the
	 ** new first-el.
	 */ 
	*a      = *(b->next);

        /* Bend his "next" pointer at the next el. of the old first-el. */ 
	a->next = bn;
	
        /* Write the value of the new fist-el. on the place of the
	 ** old first-el.
	 */ 
	*(b->next)    = c;

        /* Bend his "next" pointer at the next el. of the new first-el. */ 
	b->next->next = an;

	b->next       = a;    
    }

    return;
}




/* @funcstatic edialign_quicksort_ow ***************************************
**
** edialign_quicksort_ow
**
** @param [u] array [struct multi_frag**] Undocumented
** @param [r] left [ajint] Undocumented
** @param [r] right [ajint] Undocumented
** @return [void]
*****************************************************************************/

static void edialign_quicksort_ow(struct multi_frag **array, ajint left,
				  ajint right)
{
    ajint l = left;
    ajint r = right;
    struct multi_frag *element;

    element = array[(left+right)/2];

    do
    {
	while(array[l]->ow > element->ow)
	    l++;
	while(element->ow  > array[r]->ow)
	    r--;
	  
	if(l < r)  edialign_change_struct_el(array,l,r);
	if(l <= r) {l++; r--;}
    }while(l<=r);
  
    if(left < r)
	edialign_quicksort_ow(array, left, r);
    if(l < right)
	edialign_quicksort_ow(array, l, right);


    return;
} /*edialign_quicksort_ow*/




/* @funcstatic edialign_quicksort_weight ************************************
**
** edialign_quicksort_weight
**
** @param [u] array [struct multi_frag**] Undocumented
** @param [r] left [ajint] Undocumented
** @param [r] right [ajint] Undocumented
** @return [void]
*****************************************************************************/

static void edialign_quicksort_weight(struct multi_frag **array, ajint left,
				      ajint right)
{
    ajint l = left;
    ajint r = right;
    struct multi_frag *element;

    element = array[(left+right)/2];
  
    do
    {
	while(array[l]->weight > element->weight)  l++;
	while(element->weight  > array[r]->weight) r--;
      
	if(l < r)  edialign_change_struct_el(array,l,r);
	if(l <= r) {l++; r--;}
    }while(l<=r);
  
    if(left < r)
	edialign_quicksort_weight(array, left, r);
    if(l < right)
	edialign_quicksort_weight(array, l, right);


    return;
}




/* @funcstatic edialign_assemble_list ***************************************
**
** edialign_assemble_list
**
** @param [u] array [struct multi_frag**] Undocumented
** @param [u] dp [struct multi_frag*] Undocumented
** @param [r] number [ajint] Undocumented
** @return [void]
*****************************************************************************/

static void edialign_assemble_list(struct multi_frag **array,
				   struct multi_frag *dp, ajint number)
{
    ajint i;
    ajint idx = 0;

    for(i = 0; i< number-1; i++)
    {
	if(dp==array[i])
	    idx = i;
	array[i]->next = array[i+1];
    }
  
    array[number-1]->next = 0;
    if(dp==array[number-1])
	idx = number-1;
  
    if(idx!=0)
	edialign_change_first(array[0],array[idx-1]);

    return;
}




/* @funcstatic edialign_frag_sort ***************************************
**
** edialign_frag_sort
**
** @param [r] number [ajint] Undocumented
** @param [u] dp [struct multi_frag*] Undocumented
** @param [r] olw [ajint] Undocumented
** @return [void]
*****************************************************************************/

static void edialign_frag_sort(ajint number , struct multi_frag *dp ,
			       ajint olw ) 
{
    ajint i=1;

    struct multi_frag **array = NULL;
    if((array = (struct multi_frag**)calloc(number+1,
					    sizeof(struct multi_frag*)))==0)
    {
	ajFatal(" problems with memory allocation for `all_clades'\n \n");
    }
  
    array[0] = dp;
    while(array[i-1]->next)
    {array[i] = array[i-1]->next; i++;}

    if( olw )
	edialign_quicksort_ow(array,0,number);
    else 
	edialign_quicksort_weight(array,0,number);
  
  
    edialign_assemble_list(array, dp, number+1);

    AJFREE(array);
    return;
}




/* @funcstatic edialign_ow_add ***************************************
**
** edialign_ow_add
**
** @param [u] sm1 [struct multi_frag*] Undocumented
** @param [u] sm2 [struct multi_frag*] Undocumented
** @return [void]
*****************************************************************************/

static void edialign_ow_add( struct multi_frag *sm1 , struct multi_frag *sm2 )
{
    /* increasing the overlap weights of two diagonals, if they
       have any overlap */

    ajint trans;
    ajint i;
    ajint j;
    ajint k;
    ajint s1;
    ajint s2;
    ajint b1;
    ajint b2;
    ajint conslen;
    ajint dif;
    ajint match;
    float add_wgt;

    trans = sm1->trans;

    for(i=0;i<2;i++)
	for(j=0;j<2;j++)
	    if( sm1->s[i] == sm2->s[j] )
		if( sm1->s[j] != sm2->s[i] )
		    if( sm1->b[i] < sm2->b[j] + sm2->ext && 
		       sm2->b[j] < sm1->b[i] + sm1->ext )
		    {
			conslen = edialign_mini2( sm1->b[i] + sm1->ext,
						 sm2->b[j] +
					sm2->ext)
			    - edialign_maxi2( sm1->b[i] , sm2->b[j] );        
			if( 
			   ( trans == 0 ) ||
			   ( ( conslen % 3 ) == 0 )
			   )    { 
 
			    s1 = sm1->s[(i+1)%2];
			    s2 = sm2->s[(j+1)%2];
     
			    b1 = sm1->b[(i+1)%2];
			    dif = sm2->b[j] - sm1->b[i];
			    if (dif > 0)
				b1 = b1 + dif;

			    b2 = sm2->b[(j+1)%2];
			    dif = sm1->b[i] - sm2->b[j];
			    if (dif > 0)
				b2 = b2 + dif;

			    match = 0;

			    for( k = 0 ; k < conslen ; k++ )
			    {
				if( 
				   ( wgt_type == 0 ) ||
				   ( trans && ( ( k % 3 ) == 0 ) ) 
				   ) 
				    match = match
					+ sim_score[amino[s1][b1+k]][amino[s2]
								 [b2+k]];
				else
				    match = match + ( seq[ s1 ][ b1 + k ] ==
						     seq[ s2 ][ b2 + k ] );
			    }


			    if( wgt_type == 0 )
				add_wgt = wgt_prot[ conslen ][ match ];
			    else
				if( trans )
				    add_wgt = wgt_trans[ conslen / 3 ][match];
				else
				    add_wgt = wgt_dna[ conslen ][ match ] ;   

			    sm1->ow = sm1->ow + add_wgt ; 
			    sm2->ow = sm2->ow + add_wgt ; 

			}
		    }     

    return;
}




/* @funcstatic edialign_seq_shift ********************************************
**
** edialign_seq_shift
**
** @return [void]
*****************************************************************************/

static void edialign_seq_shift(void)
{
    ajuint i;
    ajint hv;
  
    for(i = 0 ; i < seqnum ; i++)
	for(hv = seqlen[i]+1 ; hv > 0 ; hv--)
	    seq[i][hv] = seq[i][hv-1];

    return;
}

     


/* @funcstatic edialign_filter ********************************************
**
** edialign_filter
**
** @param [u] number [ajint*] Undocumented
** @param [u] diagonal [struct multi_frag*] Undocumented
** @return [void]
*****************************************************************************/

static void edialign_filter(ajint *number, struct multi_frag *diagonal)
{
    /* checks diagonals one by one, if they are consistent with the 
       diagonals already included into the alignment. If a new diagonal 
       is consistent, it is included into the alignment and the frontiers 
       in clos (when GABIOS is used) are changed accordingly */

    ajuint i;
    ajuint j;
    ajint hv;
    ajint ab[2];
    ajint as[2];
    ajint aext;
    ajint nv;
    float awgt ; 

    ajint test;  /* = 1 if current diagonal consistent; = 0 otherwise */
    ajint number_bf;	      /* number of diagonals before filter */ 

    FILE *fp_st;
    FILE *fp_cap = NULL;

    struct multi_frag *dia; 
    char cap_file_name[ NAME_LEN ] ;
 
    if( ( istep > 0 ) && ( iter_cond_prob == 0 ) )  
	for( i = 0 ; i < seqnum ; i++ )
	    for( j = 0 ; j < seqnum ; j++ )
		cont_it_p[i][j] = 0 ;

    dia = diagonal;
    number_bf = *number;

    if( ( istep == 0 ) && anchors && ( seqnum > 2 ) )
    {
	strcpy( cap_file_name , input_name );
	strcat( cap_file_name , ".cap" );
	fp_cap = fopen( cap_file_name ,"w");
    }
   

    for(nv = 0 ; nv < number_bf ; nv++ )
    {
	ab[0] = dia->b[0]; /* begin of n-th diagonal in 1. sequence */
	ab[1] = dia->b[1]; /* begin of n-th diagonal in 2. sequence */
	as[0] = dia->s[0];	    /* 1. sequence of n-th diagonal */
	as[1] = dia->s[1];	    /* 2. sequence of n-th diagonal */
	aext  = dia->ext;		/* length of n-th diagonal */
	awgt  = dia->weight;		/* length of n-th diagonal */
	/*ae[0] = ab[0] + aext - 1;*/ /* end of n-th diagonal in 1. sequence */
	/*ae[1] = ab[1]+aext-1;*/ /* end of n-th diagonal in 2. sequence */


	if( print_status )
	    if( ( ( nv + 1 ) % 10 ) == 0 )
	    {  
		fp_st = fopen( pst_name ,"w");

		fprintf(fp_st,"\n\n\n    Status of the program run:\n");  
		fprintf(fp_st,"    ==========================\n\n");  
		fprintf(fp_st,"      %s \n\n", input_line);
		fprintf(fp_st,"      iteration step %d \n", istep );
		fprintf(fp_st,"      checking diagonal %d for ", nv + 1);
		fprintf(fp_st,"consistency\n\n      total number of"); 
		fprintf(fp_st," diagonals = %d \n\n\n\n", number_bf);
		fclose(fp_st);
	    }

	test = edialign_alignableSegments(gabiosclos,
					  as[0], ab[0], as[1], ab[1],
					  aext);
       
	if(test) /* i.e current diagonal consistent with the diagonals
		    already included into the alignment */
	{

	    edialign_addAlignedSegments(gabiosclos, as[0], ab[0], as[1], ab[1],
					aext);
  
	    if( istep )  
		for(hv=0;hv<aext;hv++)
		    for(i=0;i<2;i++) 
		    {
			j = (i+1)%2;
			open_pos[ as[i] ][ as[j] ][ ab[i]+hv ] = 0;
		    }
   
	    dia->sel = 1;   
	    glob_sim[ as[0] ][ as[1] ] =  
		glob_sim[ as[0] ][ as[1] ] + dia->weight;

	    if( istep )
		tot_weight = tot_weight + dia->weight;
   

	} /* if test, i.e. current diagonal consistent */   
	else				/* no consistency */
	{
	    (*number)--;
	    dia->sel = 0;   
	    cont_it_p[ as[0] ][ as[1] ] = 1 ; 
	}

	if( ( istep == 0 ) && anchors && ( seqnum > 2 ) )
	{
	    fprintf( fp_cap, " anchor %d %d %d %d %d %f " , as[0] + 1,
		    as[1] + 1 , ab[0], ab[1], aext , awgt);
	    if( dia->sel == 0 )
		fprintf( fp_cap , " inconsistent ");
	    fprintf( fp_cap , "\n");
	}


	dia = dia->next;

    } /*  for(hv = 0 ; hv < number_bf ; hv++ )  */
                    
    if( ( istep == 0 ) && anchors && ( seqnum > 2 ) ) 
	fclose(  fp_cap  ) ;

    return;
}




/* @funcstatic edialign_sel_test ********************************************
**
** edialign_sel_test
**
** @return [void]
*****************************************************************************/

static void edialign_sel_test(void)
{
    ajint hv;
    struct multi_frag *hp;   

    hp = this_it_dia;   

    for( hv = 0 ; hv < num_dia_af[ istep ] ; hv++ )
    {
        if( hp->sel == 0 )   
	{  
            ajFatal("\n\n\n sel[%d] = %d \n", hv, hp->sel);
	}
        hp = hp->next;
    }

    return;
}




/* @funcstatic edialign_throw_out ********************************************
**
** edialign_throw_out
**
** @param [u] weight_sum [float*] Undocumented
** @return [void]
*****************************************************************************/

static void edialign_throw_out( float *weight_sum )
{
    ajint nc;
    short consist_found = 0; 
  
    struct multi_frag *cp = NULL;		/* current diagonal */
    struct multi_frag *hp = NULL;		/* predecedor of cp */ 
    
/*    hp = ( struct multi_frag *) calloc( 1 , sizeof( struct multi_frag ) ); */
    cp = this_it_dia;
    hp = NULL;
    *weight_sum = 0;


    for( nc = 0 ; nc <  num_dia_bf[ istep ] ; nc++ )
    {
        if( cp->sel )
	{
            *weight_sum = *weight_sum + cp->weight;
            consist_found = 1;

            hp = cp;
            cp = cp->next;  
	} 
        else
	{
            cp = cp->next;
            if( consist_found ) 
	    {
                free(hp->next);
                hp->next = cp;              
	    } 
            else
	    {
                free( this_it_dia);
                this_it_dia = cp;
	    }
	}
    }

    return;
}




/* @funcstatic edialign_new_shift ********************************************
**
** shifts the elements of sequence s starting with  position p 
** for dif elements to the right
**
** @param [r] s [ajint] Undocumented
** @param [r] p [ajint] Undocumented
** @param [r] dif [ajint] Undocumented
** @return [void]
*****************************************************************************/

static void edialign_new_shift(ajint s, ajint p, ajint dif)
{
    ajint hv;
    ajint shift_dif; /* length of a gap (if existing) between position hv
                        and position hv+1. In case of gaps, the function
                        `new_shift' diminishs the lengths of the gaps instead
                        of shifting further sequence elements to the right  */

    for(hv=p ; ( hv<seqlen[s]+1 ) && (dif>0) ; hv++)
    {
        shift_dif = shift[s][hv+1] - shift[s][hv] - 1;
        shift[s][hv] = shift[s][hv] + dif;
        dif = dif -  shift_dif;
    }

    return;
}




/* @funcstatic edialign_wgt_type_count **************************************
**
** edialign_wgt_type_count
**
** @param [r] num [ajint] Undocumented
** @param [r] e_len [ajint] Undocumented
** @param [u] plus_cnt [ajint*] Undocumented
** @param [u] minus_cnt [ajint*] Undocumented
** @param [u] nuc_cnt [ajint*] Undocumented
** @param [u] frg_inv [ajint*] Undocumented
** @param [u] dia [struct multi_frag*] Undocumented
** @return [void]
*****************************************************************************/

static void edialign_wgt_type_count(ajint num , ajint e_len, ajint *plus_cnt,
				    ajint *minus_cnt, ajint *nuc_cnt ,
				    ajint *frg_inv, struct multi_frag *dia)
{
    ajint i;
    ajint dc;
    ajint pc;
    ajint s1;
    ajint pos;
    
    (void) e_len;			/* make it used */

    for( dc = 0 ; dc < num ; dc++ )
    {

        for( pc = 0 ; pc < dia->ext ; pc++ )
	{
            i  = dia->b[0] + pc;
            s1 = dia->s[0];
            pos = shift[s1][i];
            if ( dia->trans )
		if ( dia->cs )
		    minus_cnt[ pos ] = minus_cnt[ pos ] + 1 ;
		else
		    plus_cnt[ pos ] = plus_cnt[ pos ] + 1 ;
            else {
		nuc_cnt[ pos ] = nuc_cnt[ pos ] + 1 ;
            }
            frg_inv[ pos ] = frg_inv[ pos ] + 1 ;
	}
        dia = dia->next;
    }

    return;
}




/* @funcstatic edialign_plot_calc **************************************
**
** edialign_plot_calc
**
** @param [r] num [ajint] Undocumented
** @param [r] e_len [ajint] Undocumented
** @param [u] w_count [float*] Undocumented
** @param [u] pl [float*] Undocumented
** @param [u] dia [struct multi_frag*] Undocumented
** @param [u] fp_csc [FILE*] Undocumented
** @return [void]
*****************************************************************************/

static void edialign_plot_calc(ajint num , ajint e_len, float *w_count,
			       float *pl, struct multi_frag *dia ,
			       FILE *fp_csc )
{
    ajint i;
    ajint dc;
    ajint pc;
    ajint s1;
    ajint pos;
    float max_weight = 0;	 /* maximum value of `weight_count' */
    float shrink;
    float shrink_csc;
    float hsc;
   
    for( dc = 0 ; dc < num ; dc++ )
    {
  
        for( pc = 0 ; pc < dia->ext ; pc++ )
	{
            i  = dia->b[0] + pc;
            s1 = dia->s[0];
            pos = shift[s1][i]; 
            w_count[ pos ] = w_count[ pos ] + dia->weight;  
	}   
        dia = dia->next;
    }
  
  
    for( i = 0 ; i <= e_len ; i++ )
	if( max_weight < w_count[i] )
	    max_weight = w_count[i];
  
  
    if( max_weight )
    {
        shrink = plot_num / max_weight;
        shrink_csc = MAX_CSC / max_weight;
  
        for( i = 0 ; i <= e_len ; i++ )
	    pl[i] = w_count[i] * shrink;

        if( col_score )
	{
	    printf(" e_len = %d \n\n", e_len) ; 
	    for( i = 0 ; i <= e_len ; i++ )
	    {
		hsc = w_count[i] * shrink_csc ;
		fprintf( fp_csc , "%5.1f\t0\n", hsc ) ;
	    }
        }
    }
    else
    { 
	for( i = 0 ; i <= e_len ; i++ )
	    pl[i] = 0 ; 

	printf(" e_len = %d \n\n", e_len) ; 
	printf(" no max weight\n\n"); 
    }

    return;
}  




/* @funcstatic edialign_av_tree_print **************************************
**
** edialign_av_tree_print
**
** @return [void]
*****************************************************************************/

static void edialign_av_tree_print(void)
{
    ajuint i;
    ajuint j;
    ajint k;
    ajuint connect;
    ajint max_pair[2];
    ajuint m1;
    ajuint m2;
    struct subtree *all_clades = NULL;
    double **clade_similarity = NULL;
    double new_similarity = 0.; 
    double max_sim; 
    char *string = NULL;
    char l_name[2][20];   
    float branch_len[2];
    float depth; 

    if( (all_clades = (struct subtree *) 
	 calloc( seqnum , sizeof( struct subtree ) )) == NULL)
    {
        ajFatal(" problems with memory allocation for `all_clades'\n \n");
    }


    if( (clade_similarity = (double **) 
         calloc( seqnum , sizeof( double* ) )) == NULL)
	embExitBad();

    for(i = 0 ; i < seqnum ; i++ )
	if( (clade_similarity[i] = (double *) 
	     calloc( seqnum , sizeof( double ) )) == NULL)
	    embExitBad();

    if( (string = (char *) 
	 calloc( seqnum * 100 , sizeof(char) )) == NULL)
    {
        printf(" problems with memory allocation for `string'\n \n");
        embExitBad();
    }




    for(i = 0 ; i < seqnum ; i++ ) 
    { 
	if( (all_clades[i].member = (int *) 
             calloc( seqnum , sizeof( int ) )) == NULL)
	{    
	    printf(" problems with memory allocation for `all_clades'\n \n");
	    embExitBad();
	}


	if( (all_clades[i].name = (char *) 
             calloc( seqnum * 100 , sizeof( char ) )) == NULL)
	{    
	    printf(" problems with memory allocation for `all_clades'\n \n");
	    embExitBad();
	}
         
	strcpy( all_clades[i].name , seq_name[i] );
	all_clades[i].member_num = 1;
	all_clades[i].member[0] = i;
	all_clades[i].valid = 1;
	all_clades[i].depth = 0;
    } 



    for(i = 0 ; i < seqnum ; i++ ) 
	for(j = i + 1 ; j < seqnum ; j++ ) 
	{
	    clade_similarity[i][j] =  glob_sim[i][j];
	    clade_similarity[j][i] =  glob_sim[i][j];
	} 


    for(connect = 1 ; connect < seqnum ; connect++)
    {
        max_sim = - 1;

   

        for(i = 0 ; i < seqnum ; i++ ) 
	    for(j = 0 ; j < seqnum ; j++ ) 
		if( i != j )
		    if( all_clades[i].valid && all_clades[j].valid )    
			if( clade_similarity[i][j] > max_sim )
			{
			    max_sim =  clade_similarity[i][j];
			    max_pair[0] = i;  
			    max_pair[1] = j;  
			}  

  
        depth = (float) (1 / ( max_sim + 1 )); 

	{
            m1 = max_pair[0];  
            m2 = max_pair[1];  

            for( i = 0 ; i < seqnum ; i++ )
		if( all_clades[i].valid )        
		    if( i != m1 )
			if( i != m2 )      
			{
			    if( ! strcmp(clust_sim , "av") )
				new_similarity = 
				    ( 
				     clade_similarity[i][m1] *
				     all_clades[m1].member_num + 
				     clade_similarity[i][m2] *
				     all_clades[m2].member_num  
				     ) /
					 ( all_clades[m1].member_num +
					  all_clades[m2].member_num );
 
			    if( ! strcmp(clust_sim , "max") )
				new_similarity = 
				    edialign_maxf2(
					   (float) clade_similarity[i][m1] ,
					   (float) clade_similarity[i][m2] );
		    
			    if( ! strcmp(clust_sim , "min") )
				new_similarity =
				  edialign_minf2(
					   (float) clade_similarity[i][m1],
					   (float) clade_similarity[i][m2]);


			    clade_similarity[i][m1] = new_similarity;
			    clade_similarity[m1][i] = new_similarity;
			}


            all_clades[m2].valid = 0;

            for(k = 0 ; k <  all_clades[m2].member_num  ; k++)
		all_clades[m1].member[ all_clades[m1].member_num + k ] = 
		    all_clades[m2].member[ k ] ;

            all_clades[m1].member_num = 
		all_clades[m1].member_num + all_clades[m2].member_num;


            for(k = 0 ; k < 2 ; k++)
	    {
                branch_len[k] = depth - all_clades[ max_pair[k] ].depth;
                sprintf( l_name[k],":%f", branch_len[k]);
	    } 


            all_clades[m1].depth = depth;

       
            strcpy(string,"(");
            strcat(string, all_clades[m1].name); 
            strcat(string,l_name[0]); 
	    /*            strcat(string,",\n");   */ 
            strcat(string, all_clades[m2].name); 
            strcat(string,l_name[1]); 
            strcat(string,")");

            strcpy( all_clades[m1].name , string ); 
	}
    }


    strcat(string, ";"); 

    i = strlen( string ) + 2;

    if( (upg_str = (char *) calloc( i , sizeof(char) )) == NULL)
    {
        printf(" problems with memory allocation for `upg_str'\n \n");
        embExitBad();
    }

    for(i = 0 ; i <= strlen( string ) ; i++ )
	upg_str[i] = string[i] ;

    for(i=0; i < seqnum; i++)
    {
        AJFREE(all_clades[i].member);
        AJFREE(all_clades[i].name);
        AJFREE(clade_similarity[i]);
    }
    AJFREE(all_clades);
    AJFREE(clade_similarity);
    
    AJFREE(string);
    return;
}




/* @funcstatic edialign_print_log **************************************
**
** edialign_print_log
**
** @param [u] d [struct multi_frag*] Undocumented
** @param [u] fp_l [FILE*] Undocumented
** @param [u] fp_fs [FILE*] Undocumented
** @return [void]
*****************************************************************************/

static void edialign_print_log(struct multi_frag *d,FILE *fp_l,FILE *fp_fs)
{
    ajuint i;
    ajuint j;
    ajint pv;
    ajint percent;
    ajint this_frag_trans;
    ajint frg_count = 0;
    struct multi_frag *diagonal;
    char hc;

    if(long_output)
    {
        fprintf(fp_l," \n \n  Iteration %d:\n", istep );

        if( istep < 10 ) 
	    fprintf(fp_l,"  ------------");
        else
	    fprintf(fp_l,"  -------------");
    } 


    for(i= 0 ; i<seqnum ; i++)
	for(j= i+1 ; j<seqnum; j++)
	{
	    if(long_output) {
		if( seqnum > 2 ) {
		    fprintf(fp_l, "\n \n \n \n  Pairwise alignment ");
		    fprintf(fp_l, "%d/%d", i + 1, j + 1); 
		    fprintf(fp_l, " (%s / %s) \n" ,seq_name[i],seq_name[j] );
		    fprintf(fp_l, "  =========================");
		    fprintf(fp_l, "===================== ");
		}
		fprintf(fp_l, " \n \n \n");
	    }

	    pairalignsum = 0;
	    pairalignlen = 0;

	    diagonal = d;
	    while(diagonal != NULL)
	    {
		frg_count++ ;
		if( diagonal->s[0] == (ajint)i && diagonal->s[1] == (ajint)j)
		{
		    if(diagonal->sel)
		    {
			if(long_output)
			{
			    fprintf(fp_l,"   *");
			    fprintf(fp_l," (%3d,", diagonal->b[0]);
			}

			pairalignsum = pairalignsum + diagonal->weight;
			pairalignlen = pairalignlen + diagonal->ext;
		    }
		    else
			if(long_output)
			    fprintf(fp_l,"     (%3d,", diagonal->b[0]);

		    if(long_output)
		    {
			fprintf(fp_l,"%3d)  ", diagonal->b[1]);
			fprintf(fp_l," wgt:%7.3f ", diagonal->weight);
			if(seqnum > 2) 
			    if(overlap_weights)
				fprintf(fp_l," olw:%7.3f ", diagonal->ow);
			fprintf(fp_l,"len: %2d", diagonal->ext);
			if( ( wgt_type == 3 ) || crick_strand )
			{
			    if( diagonal->trans )
				fprintf(fp_l,"  P-frg" );
			    else
				fprintf(fp_l,"  N-frg" );
			}

			if( diagonal->trans )
			    if( crick_strand )
			    {
				if( diagonal->cs )
				    fprintf(fp_l,", CRICK strand " );
				else
				    fprintf(fp_l,", WATSON strand " );
			    }

		    }

		    if( frg_mult_file_v )
		    {
			fprintf(fp_fs,"FRG %d ", frg_count ); 
			fprintf(fp_fs,"name: %s %s ",
				seq_name[i] , seq_name[j] ) ;  
 
			fprintf(fp_fs,"seq: %d %d ", i + 1 , j + 1 ) ;  
			fprintf(fp_fs,"beg: %d %d ", diagonal->b[0],
				diagonal->b[1]); 
			fprintf(fp_fs,"len: %d ", diagonal->ext);

			fprintf(fp_fs,"wgt:%7.3f ", diagonal->weight);
			if(diagonal->sel) 
			    fprintf(fp_fs," CONS  "); 
			else   
			    fprintf(fp_fs," NON-CONS ");
			fprintf(fp_fs,"\n") ; 
			fprintf(fp_fs,"SEG1   ");
			for(pv = 0 ; pv < diagonal->ext ; pv ++)
			    fprintf(fp_fs,"%c", seq[i][diagonal->b[0] + pv]);
			fprintf(fp_fs,"\n"); 

			fprintf(fp_fs,"SEG2   ");
			for(pv = 0 ; pv < diagonal->ext ; pv ++)
			    fprintf(fp_fs,"%c", seq[j][diagonal->b[1] + pv]);
			fprintf(fp_fs,"\n"); 
			fprintf(fp_fs,"\n"); 
		    }

		    if( frg_mult_file & ! frg_mult_file_v )
		    { 
			if( diagonal->sel )
			{
			    fprintf(fp_fs," %d %d ", i + 1 , j + 1 ) ;  
			    fprintf(fp_fs," %d %d ", diagonal->b[0],
				    diagonal->b[1]); 
			    fprintf(fp_fs," %d \n", diagonal->ext);
			} 
		    }                

		    if(long_output)
		    {
			fprintf(fp_l,"\n");
 
			if( 
			   wgt_type == 2 || 
			   ( ( wgt_type == 3 ) && diagonal->trans ) 
			   )
			    this_frag_trans = 1;
			else
			    this_frag_trans = 0;
                           
			if( this_frag_trans ) 
			{
			    fprintf(fp_l,"\n           ");
			    for(pv = 0 ; pv < diagonal->ext ; pv ++)
			    { 
				hc = amino_acid[ amino[i][ diagonal->b[0] +
							  pv - 1 ] ] ;
				if( crick_strand )
				    if( diagonal->cs )
					hc = amino_acid[amino_c[i]
							[diagonal->b[0] +
							 pv - 1 ] ] ;
                          
				if( ( pv % 3 ) == 0 )
				    fprintf(fp_l,"/");
				if( ( pv % 3 ) == 1 )
				    fprintf(fp_l,"%c", hc ) ; 
				if( ( pv % 3 ) == 2 )
				    fprintf(fp_l,"\\");

			    } 
			}

			fprintf(fp_l,"\n           "); 
			for(pv = 0 ; pv < diagonal->ext ; pv ++)
			    fprintf(fp_l,"%c", seq[i][ diagonal->b[0] + pv ] );
			fprintf(fp_l,"\n"); 


			fprintf(fp_l,"           ");
			for(pv = 0 ; pv < diagonal->ext ; pv ++)
			    fprintf(fp_l,"%c", seq[j][ diagonal->b[1] + pv ] );


			if( this_frag_trans )
			{
			    fprintf(fp_l,"\n           ");
			    for(pv = 0 ; pv < diagonal->ext ; pv ++)
			    {
				hc = amino_acid[ amino[j][ diagonal->b[1] +
							  pv - 1 ]
						] ;
				if( crick_strand )
				    if( diagonal->cs )
					hc = amino_acid[ amino_c[j]
							[ diagonal->b[1] +
							 pv - 1 ] ] ;

				if( ( pv % 3 ) == 0 )
				    fprintf(fp_l,"\\");
				if( ( pv % 3 ) == 1 )
				    fprintf(fp_l,"%c", hc ) ;
				if( ( pv % 3 ) == 2 )
				    fprintf(fp_l,"/");

			    }
			}

			fprintf(fp_l,"\n \n");
		    }   
		} /*  if( diagonal->s[0] == i && diagonal->s[1] == j)  */

		diagonal = diagonal->next;

	    } /*  while(diagonal != NULL) */

	    percent = pairalignlen*100/edialign_mini2(seqlen[i],seqlen[j]);

	    if(long_output)
	    {
		fprintf(fp_l,"\n      Sum of diagonal scores: %f\n",
			pairalignsum);
		fprintf(fp_l,"      Aligned residues: %d\n", pairalignlen);
		fprintf(fp_l,"      (%d percent of the shorter", percent);
		fprintf(fp_l," sequence aligned)\n");
	    }
	} /* for(i = 0     ; i < seqnum ; i++)
	     for(j = i + 1 ; j < seqnum ; j++)  */

    return;
}




/* @funcstatic edialign_word_count ******************************************
**
** edialign_word_count
**
** @param [u] str [char*] Undocumented
** @return [ajint] Undocumented
*****************************************************************************/

static ajint edialign_word_count( char *str )
{

    short word = 0 ; 
    ajuint i ; 
    ajint word_len = 0 ; 

    for( i = 0 ; i < strlen( str ) - 1  ; i++ )
    { 
	if( ( str[i] != ' ' ) && ( str[i] != '\t' ) )
	{  
	    if( ! word )
	    { 
		word_len++ ; 
		word = 1 ; 
	    }
	}
	else 
	    word = 0 ; 

    }

    return( word_len ) ; 
} 




/* @funcstatic edialign_exclude_frg_read *************************************
**
** edialign_exclude_frg_read
**
** @param [u] file_name [char*] Undocumented
** @param [u] exclude_list [int***] Undocumented
** @return [void]
*****************************************************************************/

static void edialign_exclude_frg_read( char *file_name , int ***exclude_list)
{
    char exclude_file_name[ NAME_LEN ];
    FILE *fp;
    char line[ 10000 ];
    ajint i;
    ajint len;
    ajint beg1;
    ajint beg2;
    ajuint seq1;
    ajuint seq2; 

    strcpy( exclude_file_name , file_name );
    strcat( exclude_file_name , ".xfr" );

    if( (fp = fopen( exclude_file_name, "r")) == NULL)
	edialign_erreur("\n\n cannot find file with excluded fragments \n\n");

  

    while( fgets( line , MLINE , fp ) != NULL )
    {
	if( strlen( line ) > 4 )
	{   
	    sscanf(line,"%d %d %d %d %d", &seq1, &seq2, &beg1, &beg2 , &len);

	    if( seq1 > seqnum )
	    {
		printf ("\n\n exclueded fragment makes no sense!\n\n");
		printf (" wrong sequence no %d in fragment\n\n", seq1 );
		printf ("%d %d %d %d %d \n\n ", seq1, seq2, beg1, beg2 , len);
		embExitBad() ;
	    }
 
	    if( seq2 > seqnum )
	    {
		printf ("\n\n    excluded fragment makes no sense!\n\n");
		printf ("    wrong sequence no %d in fragment\n\n", seq2 );
		printf ("    %d %d %d %d %d \n\n", seq1, seq2, beg1, beg2,
			len );
		embExitBad() ;
	    }

	    /*
	       seq1 = seq1 - 1; 
	       seq2 = seq2 - 1;
	       */

	    if( beg1 + len > seqlen[ seq1 - 1 ] + 1 ){
		printf ("\n\n    excluded fragment makes no sense!\n");
		printf ("    fragment");
		printf ("     \" %d %d %d %d %d \"\n", seq1, seq2, beg1,
			beg2 , len );
		printf ("    doesn't fit into sequence %d:\n", seq1 );
		printf ("    sequence %d has length =  %d\n\n", seq1 ,
			seqlen[ seq1 - 1 ] );
		embExitBad() ;
	    }


 
	    for( i = 0 ; i < len ; i++ )
	    {  
		exclude_list[ seq1 - 1 ][ seq2 - 1 ][ beg1 + i ] = beg2 + i ;
	    }
	}
    }

    return;
}




#if 0
/* @funcstatic edialign_ws_remove *************************************
**
** edialign_ws_remove
**
** @param [u] str [char*] Undocumented
=============================
** @return [void]
*****************************************************************************/

static void edialign_ws_remove( char *str )
{
    ajint pv = 0 ;

    while( ( str[ pv ] == ' ' ) || ( str[ pv ] == '\t' ) )
	pv++;

    strcpy( str , str + pv );

    return;
}




/* @funcstatic edialign_n_clean *************************************
**
** edialign_n_clean
**
** @param [u] str [char*] Undocumented
** @return [void]
*****************************************************************************/

static void edialign_n_clean( char *str )
{
    ajint pv = 0 ;
    char *char_ptr ;

    while( ( str[ pv ] == ' ' ) || 
	  ( str[ pv ] == '\t' ) || 
	  ( str[ pv ] == '>' ) )
	pv++;

    strcpy( str , str + pv ) ;

    if( ( char_ptr = strchr( str ,' ') ) != NULL)
	*char_ptr = '\0';
    if( ( char_ptr = strchr( str ,'\t') ) != NULL)
	*char_ptr = '\0';
    if( ( char_ptr = strchr( str ,'\n') ) != NULL)
	*char_ptr = '\0';

    return;
}




/* @funcstatic edialign_fasta_test *************************************
**
** edialign_fasta_test
**
** @param [u] seq_file [char*] Undocumented
** @return [void]
*****************************************************************************/

static void edialign_fasta_test( char *seq_file )
{
    ajint test = 1;
    FILE *fp;

    char line[ MAX_INPUT_LINE ] ;

    if( (fp = fopen( seq_file , "r")) == NULL)
    { 
	printf("\n\n Cannot find sequence file %s \n\n\n", seq_file );
	embExitBad() ;
    }

    while( test )
    {
	fgets( line , MAX_INPUT_LINE , fp );

	edialign_ws_remove( line );

	if( line[0] != '\n' )
	{
	    if( line[0] == '>' ) 
		test = 0;
	    else
		edialign_erreur("\n\n  file not in FASTA format  \n\n");
	}
    }
  
    fclose( fp );

    return;
}
#endif




#if 0
/* @funcstatic edialign_seq_read ********************************************
**
** edialign_seq_read. unused.
**
** @param [u] seq_file [char*] Undocumented
** @param [u] sq [char* [MAX_SEQNUM]] Undocumented
** @param [u] sqn [char**] Undocumented
** @param [u] fsqn [char**] Undocumented
** @return [ajint] Undocumented
*****************************************************************************/

static ajint edialign_seq_read(char *seq_file, char *sq[MAX_SEQNUM] ,
			       char **sqn , char **fsqn)
{
    char line[ MAX_INPUT_LINE ] ;

    ajint  sn, i, k , crc ;
    ajint j = 0;
    FILE *fp;
    ajint max_char[ MAX_SEQNUM ] ;

    if( (fp = fopen( seq_file , "r")) == NULL)
    { 
	printf("\n\n Cannot find sequence file %s \n\n\n", seq_file );
	embExitBad() ;
    }
    edialign_fasta_test( seq_file );

    sn = -1 ;
    while( fgets( line , MAX_INPUT_LINE , fp ) != NULL )
    {
	edialign_ws_remove( line );

	if( line[0] == '>' )
	{
	    sn++;

   
	    edialign_n_clean( line );


	    fsqn[ sn ] = ( char * ) calloc(strlen(line) + 3 , sizeof(char)); 

	    strcpy( fsqn[ sn ] , line ) ; 


	    max_char[ sn ] = 0;
	    sqn[ sn ]  = (char *) calloc( SEQ_NAME_LEN + 3 , sizeof(char));

	    for( crc = 0 ; crc < SEQ_NAME_LEN ; crc++ )  
		if( crc < strlen(line) ) 
		    sqn[ sn ][ crc ] =  line[ crc ] ;
		else 
		    sqn[ sn ][ crc ] =  ' ';

	    sqn[ sn ][ SEQ_NAME_LEN ] = '\0';  

       

	}

 
	else  
	    max_char[ sn ] = max_char[ sn ] + strlen( line ) - 1 ; 
    }

    for( i = 0 ; i <= sn ; i++ )
    {
	sq[ i ]  = ( char * ) calloc( max_char[ i ] + 1 , sizeof ( char ) );
    }

    if( (seqlen = (int *) calloc( ( sn + 1 ) , sizeof(int) )) == NULL)
	edialign_erreur("\n problems with memory allocation for `seqlen' \n");


    fclose( fp );


    /******************************************/

    if( self_comparison == 1 )
    {
	if( sn != 0 ) {
	    printf("\n\n With option \"self comparison\" input file "
		   "must contain one single sequence \n\n" ); 
	    embExitBad() ;
	}

	sq[ 1 ]  = ( char * ) calloc( max_char[ 0 ] + 1 , sizeof ( char ) );

	sqn[ 1 ]  = ( char * ) calloc( strlen( line ) + 3 , sizeof ( char ) );
	strcpy( sqn[ 1 ] , sqn[ 0 ] ) ;
    }

    /******************************************/


    if( (fp = fopen( seq_file , "r")) == NULL) 
	edialign_erreur("\n\n no seq file \n\n");
 
    sn = -1 ;
    while( fgets( line , MAX_INPUT_LINE , fp ) != NULL )
    {
	edialign_ws_remove( line ); 
	if( line[0] == '>' )
	{
	    sn++;
	    j = 0;
	}
	else
	    for( k = 0 ; k < strlen( line )  ; k++ )
		if( 
		   (( line[ k ] >= 65 ) && ( line[ k ] <= 90 )) || 
		   (( line[ k ] >= 97 ) && ( line[ k ] <= 122 )) 
		   ) 
		    sq[ sn ][ j++ ] = toupper( line[ k ] ) ; 
    }
  
    sn++; 

    for( i = 0 ; i <  sn ; i++ )
    {
	seqlen[ i ] = strlen ( sq[ i ] ) ;
    }

    if( self_comparison )
    {
	seqlen[ 1 ] = seqlen[ 0 ] ;  
	for( i = 0 ; i <=  seqlen[ 0 ] ; i++ ) 
	    sq[ 1 ][ i ] = sq[ 0 ][ i ] ;    
	sn++; 
    }

    fclose( fp );

    return( sn );
}
#endif




/* @funcstatic edialign_matrix_read *****************************************
**
** edialign_matrix_read
**
** @param [u] fp_mat [FILE*] Undocumented
** @return [void]
*****************************************************************************/

static void edialign_matrix_read( FILE *fp_mat )
{
    ajint i, j;
    char line[MLINE], dummy[MLINE];
 
    fgets( line , MLINE , fp_mat );
    fgets( line , MLINE , fp_mat );

    for( i = 1 ; i <= 20 ; i++ )
    {
	for(j=i;j<=20;j++)
	{
	    fscanf( fp_mat , "%d" , &sim_score[i][j]);
	    sim_score[j][i] = sim_score[i][j];  
	    if ( sim_score[i][j] > max_sim_score )
		max_sim_score = sim_score[i][j] ;
	}

	fscanf( fp_mat, "%s\n", dummy);
    }


/*    fclose(fp_mat); */

    for( i = 0 ; i <= 20 ; i++ )
    {
	sim_score[i][0] = 0 ;
	sim_score[0][i] = 0 ;
    }

    /*
       sim_score[0][0] = max_sim_score ;
       */
    return;
}




/* @funcstatic edialign_tp400_read *****************************************
**
** edialign_tp400_read
**
** @param [r] w_type [ajint] Undocumented
** @param [u] pr_ptr [double**] Undocumented
** @return [void]
*****************************************************************************/

static void edialign_tp400_read( ajint w_type , double **pr_ptr )
{  
 
    /* reads probabilities from file */
    /* w_type = 0 (protein), 1 (dna w/o transl.), 2 (dna with transl.) */  

    char line[MLINE];
/*
    char file_name[MLINE];
    char suffix[10];
*/
    char str[MLINE] ;
    ajint sum, len;
    double pr;
    AjPFile etpfile = NULL;
    AjPStr tnstr = NULL;
    
    FILE *fp;
 

    tnstr = ajStrNew();
    

    if ( w_type == 0 )
    {
	etpfile = ajDatafileNewInNameC("tp400_prot");
/*	strcpy( suffix , "prot" );*/
    }

    else if ( w_type == 1 )
    {
	etpfile = ajDatafileNewInNameC("tp400_dna");
/*	strcpy( suffix , "dna" );*/
    }  

    else if ( w_type == 2 )
    {
	etpfile = ajDatafileNewInNameC("tp400_trans");
/*	strcpy( suffix , "trans" );*/
    }
 
/*
    strcpy( file_name , par_dir ); 
    strcat( file_name , "/tp400_" );
    strcat( file_name , suffix );


    if ( ( fp = fopen( file_name , "r" ) ) == NULL )
    { 
	printf("\n\n Cannot find the file %s \n\n", file_name );    
	printf(" Make sure the environment variable DIALIGN2_DIR points\n");
	printf(" to a directory containing the files \n\n");
	printf("   BLOSUM \n   tp400_dna\n   tp400_prot \n   tp400_trans "
	       "\n\n" );
	printf(" These files should be contained in the DIALIGN package "
	       "\n\n\n" ) ;
	embExitBad() ;
    }
*/


/*
    ajStrAssignS(&tnstr,ajFileGetNameS(etpfile));
    ajFileClose(&etpfile);    
    fp = fopen(ajStrGetPtr(tnstr),"rb");
*/
    fp = ajFileGetFileptr(etpfile);

    if ( fgets( line , MLINE , fp ) == NULL ) 
	ajFatal("\n\n problem with tp400 file \n\n");
    else
	if( w_type % 2 )  
	  av_sim_score_nuc = (float) atof( line );
	else
	  av_sim_score_pep = (float) atof( line );
     

    while( fgets( line , MLINE , fp ) != NULL )
    {
	sscanf(line,"%d %d %s", &len, &sum, str  );

	pr = atof(str);
	pr_ptr[len][sum] = pr;

    }



    ajStrDel(&tnstr);
/*    fclose(fp); */
    ajFileClose(&etpfile);    
    

    return;
}




/* @funcstatic edialign_subst_mat *****************************************
**
** edialign_subst_mat
**
** @param [u] file_name [char*] Undocumented
** @param [r] fragno [int] Undocumented
** @param [u] frg [struct multi_frag*] Undocumented
** @return [void]
*****************************************************************************/

static void edialign_subst_mat( char *file_name, int fragno ,
			       struct multi_frag *frg )
{
    ajint ii;
    ajuint i;
    ajuint j;
    ajuint s0, s1;
    ajint frg_count ; 
    short a0 , a1 ; 
    ajint ****sbsmt ;
    struct multi_frag *frag ; 
    char mat_file_name[ NAME_LEN ] ;
    FILE *fp_mat; 


    if( ( sbsmt = (int **** ) calloc( seqnum , sizeof(int ***))) == NULL)
    { 
	printf("Problems with memory allocation for sbsmt\n");
	embExitBad();
    }

    for( i = 0 ; i < seqnum ; i++ ) 
	if( ( sbsmt[i] = (int *** ) calloc( seqnum , sizeof(int **))) == NULL)
	{ 
	    printf("Problems with memory allocation for sbsmt\n");
	    embExitBad();
	}

    for( i = 0 ; i < seqnum ; i++ )
	for( j = 0 ; j < seqnum ; j++ ) 
	    if((sbsmt[i][j] = (int ** ) calloc(21,sizeof(int*))) == NULL)
	    { 
		printf("Problems with memory allocation for sbsmt\n");
		embExitBad();
	    }

    for( i = 0 ; i < seqnum ; i++ )
	for( j = 0 ; j < seqnum ; j++ ) 
	    for( a0 = 0 ; a0 < 21 ; a0++ ) 
		if((sbsmt[i][j][a0] = (int *) calloc(21,sizeof(int))) == NULL)
		{ 
		    printf("Problems with memory allocation for sbsmt\n");
		    embExitBad();
		}

    for( i = 0 ; i <seqnum ; i++ )
	for( j = 0 ; j <seqnum ; j++ )
	    for( a0 = 0 ; a0 <= 20 ; a0++ )
		for( a1 = 0 ; a1 <= 20 ; a1++ ) 
		    sbsmt[ i ][ j ][ a0 ][ a1 ] = 0 ;


    strcpy( mat_file_name , file_name );
    strcat( mat_file_name , ".mat" );

    fp_mat = fopen( mat_file_name, "w") ;



    frag = frg ;  

    for( frg_count = 0 ; frg_count < fragno ; frg_count++ )
    {
	if( frag->weight > sf_mat_thr )
	    for( ii = 0 ; ii < frag->ext ; ii++ )
	    {
		a0 = amino[ frag->s[0] ][ frag->b[0] + ii ] ; 
		a1 = amino[ frag->s[1] ][ frag->b[1] + ii ] ; 
		s0 = frag->s[0] ; 
		s1 = frag->s[1] ;
		sbsmt[ s0 ][ s1 ][ a0 ][ a1 ]++ ;
		sbsmt[ s1 ][ s0 ][ a1 ][ a0 ]++ ;

	    }
	frag = frag->next ;  
    }


    fprintf( fp_mat, "taxanumber: %d ;\n", seqnum) ;
    fprintf( fp_mat, "description: DIALIGN alignment ;\n" ) ;
    fprintf( fp_mat, "description: %s;\n", input_line ) ;


    for( i = 0 ; i < seqnum ; i++ ) 
	fprintf( fp_mat, "taxon: %.3d  name: %s  ;\n", i + 1 , full_name[i] ) ;


    for( s0 = 0 ; s0 < seqnum ; s0++ )
	for( s1 = s0 + 1  ; s1 < seqnum ; s1++ ) 
	    for ( a0 = 1 ; a0 <= 20 ; a0++ ) 
		for( a1 = 1 ; a1 < 21 ; a1++ )
		{    
		    fprintf( fp_mat, "pair: %.3d %.3d ", s0 + 1, s1 + 1 );  
		    fprintf( fp_mat, " acids: %c%c ", amino_acid[a0] ,
			    amino_acid[a1] );  
		    fprintf( fp_mat, " number: %d ;\n", sbsmt[s0][s1][a0][a1]);
		}

    return;
}




/* @funcstatic edialign_print_fragments **************************************
**
** edialign_print_fragments
**
** @param [u] d [struct multi_frag*] Undocumented
** @param [u] fp_ff2 [FILE*] Undocumented
** @return [void]
*****************************************************************************/

static void edialign_print_fragments(struct multi_frag *d , FILE *fp_ff2 )
{
    struct multi_frag *fragment ;

    fragment = d;
    while( fragment != NULL )
    {
	if( fragment->it )
	{
	    gl_frg_count++ ;
	    fprintf( fp_ff2, "%6d) ", gl_frg_count );
	    fprintf( fp_ff2, "seq: %3d %3d  ", fragment->s[0] + 1 ,
		    fragment->s[1] + 1 );
	    fprintf( fp_ff2, "beg: %7d %7d ", fragment->b[0] ,
		    fragment->b[1] );
	    fprintf( fp_ff2, "len: %3d ", fragment->ext  );
	    fprintf( fp_ff2, "wgt: %6.2f ", fragment->weight  );
	    fprintf( fp_ff2, "olw: %6.2f ", fragment->ow );

	    fprintf( fp_ff2, "it: %d ", fragment->it  );
	    if( fragment->sel )
		fprintf( fp_ff2, "cons   " );
	    else 
		fprintf( fp_ff2, "incons " );

	    if( ( wgt_type == 3 ) || crick_strand )
	    { 
		if( fragment->trans )
		    fprintf( fp_ff2, " P-frg" );
		else
		    fprintf( fp_ff2, " N-frg" );
		if( fragment->trans )
		    if( crick_strand )
		    {
			if( fragment->cs )
			    fprintf( fp_ff2, " -" );
			else
			    fprintf( fp_ff2, " +" );
		    }
	    } 


	    fprintf( fp_ff2, "\n" );
	}
	fragment = fragment->next ;
    }

    return;
}




#if 0
/* @funcstatic edialign_weight_print ****************************************
**
** edialign_weight_print
**
** @param [u] wgt [float**] Undocumented
** @return [void]
*****************************************************************************/

static void edialign_weight_print( float **wgt )
{
    ajint l, s ; 
    FILE *fp;

    fp = fopen("weight_table","w");
 

    fprintf(fp,"  len1 = %d, len2 = %d\n\n",seqlen[0], seqlen[1] );
    fprintf(fp,"  \n   %s \n\n", input_line );
    for( l = 1 ; l <= max_dia ; l++ )
	for( s = 0 ; s <= l * max_sim_score ; s++ )
	    fprintf(fp," %d %d %7.8f \n", l, s, wgt[l][s] );

    fclose(fp);

    return;
}
#endif





/* @funcstatic edialign_ali_arrange ****************************************
**
** edialign_ali_arrange
**
** @param [r] ifragno [ajint] Undocumented
** @param [u] d [struct multi_frag*] Undocumented
** @param [u] fp [FILE*] Undocumented
** @param [u] seqout [AjPSeqout] Undocumented
** @param [u] fp3 [FILE*] Undocumented
** @param [u] fp4 [FILE*] Undocumented
** @param [u] fp_col_score [FILE*] Undocumented
** @param [r] isprot [AjBool] Undocumented
** @return [void]
*****************************************************************************/

static void edialign_ali_arrange(ajint ifragno , struct multi_frag *d,
				 FILE *fp, AjPSeqout seqout, FILE *fp3 ,
				 FILE *fp4 ,
				 FILE *fp_col_score, AjBool isprot)
{
    ajint block_no;
    ajuint char_no ;
    ajint shift_cond;
    ajuint endlen;
    ajuint hv;
    ajuint i, j, p;
    ajint ii;
    ajint pn, k, l, lc;
    ajuint max_p;
    ajuint sv, s1, s2;
    ajint b1, b2, e, dif, lv, add, msf_lines;
    AjPSeq eseq = NULL;
    
    char sim_char;
    float weak_wgt_type_thr = WEAK_WGT_TYPE_THR ; 
    float strong_wgt_type_thr = STRONG_WGT_TYPE_THR  ; 
    float frac_plus, frac_minus, frac_nuc, f_inv ;
 
    char **endseq = NULL;
    char **hseq = NULL;
    char *clear_seq = NULL;
    float *weight_count = NULL;
    ajint *plus_count = NULL;
    ajint *minus_count = NULL;
    ajint *nuc_count = NULL;
    ajint *frg_involved = NULL;
    float *plot = NULL;  /* plot[i] = sum of weights of fragments involved at
		     position i normalized such that the maximum value */
 
    char gap_char = '-';
    char ambi_char = ' ';
    ajint *begin = NULL, *end = NULL, *b_len = NULL, *first_pos = NULL;
    ajint pl_int ;
    ajint b_size;				/* size of fragments */
    struct multi_frag *fragments = NULL;
    struct multi_frag *dia = NULL; 
    ajint **inv_shift = NULL;     
    ajint char_per_line; /* number of residues per line in output file */
    char aligned; 
    ajuint fragno = ifragno;

    char_per_line = ( ( PAPER_WIDTH - 18 ) / 11) * 10;

    dia = d;

    if((endseq = (char **) calloc( seqnum , sizeof(char *))) == NULL)
    {
	printf(" problems with memory allocation for `endseq' !  \n \n");
	embExitBad();
    }

    if((hseq = (char **) calloc(seqnum , sizeof(char *))) == NULL)
    {
	printf(" problems with memory allocation for `hseq' !  \n \n");
	embExitBad();
    }

    if((begin = (int *) calloc( seqnum , sizeof(int))) == NULL)
    {
	printf(" problems with memory allocation for `begin' !  \n \n");
	embExitBad();
    }

    if((end = (int *) calloc(seqnum , sizeof(int))) == NULL)
    {
	printf(" problems with memory allocation for `end' !  \n \n");
	embExitBad();
    }

    if((b_len = (int *) calloc(seqnum , sizeof(int))) == NULL)
    {
	printf(" problems with memory allocation for `b_len' !  \n \n");
	embExitBad();
    }

    if((first_pos = (int *) calloc(seqnum , sizeof(int))) == NULL)
    {
	printf(" problems with memory allocation for `first_pos' !  \n \n");
	embExitBad();
    }

    if((shift = (int **) calloc(seqnum , sizeof(int *))) == NULL) 
    {
	printf("not enough memory available for `shift' !!!!\n");   
	fprintf(fp,"not enough memory available for `shift' !\n");   
	embExitBad(); 
    }

    for(hv=0 ; hv<seqnum ; hv++)
	if((shift[hv] = (int *) calloc((seqlen[hv]+2),sizeof(int))) == NULL) 
	{
	    printf("not enough memory available for `shift' !!!!\n");   
	    fprintf(fp,"not enough memory available for `shift' !\n");   
	    embExitBad(); 
	}


    if( ifragno >= 0 )
    {

	for(hv=0;hv<seqnum;hv++)
	{
	    begin[hv] = seqlen[hv];
	    end[hv] = 1;
	}


	if( fragno > 0 )
	    if((fragments = calloc(fragno,sizeof(struct multi_frag))) == NULL)
	    {
		printf("not enough memory available for fragments!\n");   
		fprintf(fp,"not enough memory available for fragments!\n");   
		embExitBad();
	    } 

	for( hv = 1 ; hv <= fragno ; hv++)
	{
	    fragments[hv-1] = *dia;
	    dia = dia->next;
	}

	for( hv = 0 ; hv < fragno ; hv++ )
	    for( j = 0 ; j < 2 ; j++ )
	    {
		edialign_mini( &begin[ fragments[hv].s[j] ] ,
			      fragments[hv].b[j] );
		edialign_maxi( &end[ fragments[hv].s[j] ] ,
			      fragments[hv].b[j] +
		     fragments[hv].ext );
	    }

	for(hv=0;hv<seqnum;hv++)
	{
	    begin[hv] = 1;
	    end[hv] = seqlen[hv]+1; 
	}  
      
	b_size = 0;

	for(i=0;i<seqnum;i++)
	{
	    b_len[i] = end[i] - begin[i];
	    edialign_maxi(&b_size,b_len[i]);
	}  

	for(i=0;i<seqnum;i++)
	    for(hv=0;hv<(ajuint) b_len[i];hv++)
		shift[i][ begin[i]+hv ] = hv;

	shift_cond = 1;
    
	while(shift_cond)
	{
	    shift_cond = 0;
 
	    for( hv = 0 ; hv < fragno ; hv++ )
		for(j=0;j<2;j++)
		{
		    k = (j+1)%2;
		    s1 = fragments[hv].s[j]; 
		    s2 = fragments[hv].s[k]; 
		    b1 = fragments[hv].b[j]; 
		    b2 = fragments[hv].b[k]; 
		    e = fragments[hv].ext; 

		    for(l = e-1;l>=0;l--)
		    {
			dif =  shift[s2][b2+l] - shift[s1][b1+l]; 
			if (dif > 0 )
			{
			    edialign_new_shift(s1,b1+l,dif);
			    shift_cond = 1;
			}   
		    }
		}
	} /*  while (shift_cond)  */







	endlen = 0;

	for(hv=0;hv<seqnum;hv++)
	    edialign_maxu(&endlen,shift[hv][ end[hv]-1 ] + 1);  

	for(hv=0;hv<seqnum;hv++)
	    if( (endseq[hv] = calloc(endlen+1, sizeof(char) )) == NULL )
	    {
		printf(" not enough memory available for printing results!\n");
		fprintf(fp," not enough memory available");   
		fprintf(fp," for printing results!\n");   
		embExitBad();
	    }
 

	if( (inv_shift = (int **) calloc( seqnum , sizeof(int *) )) == NULL ) 
	{
	    printf("not enough memory available for `inv_shift' !!!!\n");   
	    fprintf(fp,"not enough memory available for `inv_shift' !\n");   
	    embExitBad(); 
	}

	for(hv=0 ; hv<seqnum ; hv++)
	    if( (inv_shift[hv] = (int *) calloc( (endlen+2) , sizeof(int) )) 
	       == NULL ) 
	    {
		printf("not enough memory available for `inv_shift' !!!!\n");
		fprintf(fp,"not enough memory available for `inv_shift' !\n");
		embExitBad(); 
	    }

	if( (clear_seq = (char *) calloc( (endlen+1) , sizeof(char) )) == NULL)
	{
	    printf(" problems with memory allocation for `clear_seq' !  "
		   "\n \n");
	    embExitBad();
	}

	if( (weight_count = 
             (float *) calloc( ( endlen + 2 ) , sizeof(float) )) == NULL)
	{
	    printf(" problems with memory allocation for `weight_count' "
		   "!\n \n");
	    embExitBad();
	}

	if( (plot = (float *) calloc( ( endlen + 2 ) ,sizeof(float) )) == NULL)
	{
	    printf(" problems with memory allocation for `plot' ! \n \n");
	    embExitBad();
	}

	if( (plus_count = 
             (int *) calloc( ( endlen + 2 ) , sizeof( int ) )) == NULL)
	{
	    printf(" problems with memory allocation for `plus_count' !\n \n");
	    embExitBad();
	}

	if( (minus_count = 
             (int *) calloc( ( endlen + 2 ) , sizeof( int ) )) == NULL)
	{
	    printf(" problems with memory allocation for `minus_count' "
		   "!\n \n");
	    embExitBad();
	}

	if( (nuc_count = 
             (int *) calloc( ( endlen + 2 ) , sizeof( int ) )) == NULL)
	{
	    printf(" problems with memory allocation for `nuc_count' !\n \n");
	    embExitBad();
	}

	if( (frg_involved = 
             (int *) calloc( ( endlen + 2 ) , sizeof( int ) )) == NULL)
	{
	    printf(" problems with memory allocation for `frg_involved ' "
		   "!\n \n");
	    embExitBad();
	}

   

	for(hv=0 ; hv<seqnum ; hv++)
	    for(ii=1 ; ii <= seqlen[hv] ; ii++)
		inv_shift[hv][ shift[hv][ii] ] = ii;

	for(hv=0;hv<seqnum;hv++)
	    if( (hseq[hv] = calloc( (maxlen+1), sizeof(char) )) == NULL )
	    {
		printf("not enough memory available for printing results! \n");
		fprintf(fp,"not enough memory available");   
		fprintf(fp," for printing results! \n");   
		embExitBad();
	    }
	/*
	   printf("endlen = %d \n\n", endlen); 
	   */ 

	for(hv=0;hv<seqnum;hv++)
	    for(i=0;i<endlen;i++)
		endseq[hv][i] = gap_char;

	for(hv=0;hv<seqnum;hv++)
	    for(i=begin[hv];i<(ajuint) end[hv];i++)
		hseq[hv][i] = tolower(seq[hv][i]);

	for( hv = 0 ; hv < fragno ; hv++ )
	    for(k=0;k<2;k++)
		for(ii = fragments[hv].b[k] ;  ii < fragments[hv].b[k] +
		    fragments[hv].ext ; ii++)
		    hseq[ fragments[hv].s[k]][ii] = seq[fragments[hv].s[k]][ii];

	for(hv=0;hv<seqnum;hv++)
	    for(ii = begin[hv] ; ii < end[hv] ; ii++)
		endseq[hv][ shift[hv][ii] ] = hseq[hv][ii];

	for(i=0;i<endlen;i++)
	    clear_seq[i] = ' ';






	for(p=0;p<endlen;p++)
	{
	    s1 = 0;
	    while( 
                  ( endseq[s1][p] == tolower( endseq[s1][p] ) )
		  && (s1 < (seqnum - 1) ) /* no capital letter */
		  )
		s1++;

	    if(s1 < (seqnum - 1) )
	    {
		for(s2 = s1+1 ; s2 < seqnum ; s2++)   
		{
		    if( endseq[s2][p] != tolower( endseq[s2][p] ) )
			/* endseq[s2][p] capital letter */ 
		    {
			aligned = edialign_alignedPositions(gabiosclos,s1,
							    inv_shift[s1][p],
							    s2,
						   edialign_succFrontier(gabiosclos,
									 s1,
							    inv_shift[s1][p],
							    s2));

			if (!aligned)
			    /* i.e.endseq[s1][p] not aligned with end
			       seq[s2][p]*/ 
			    clear_seq[p] =ambi_char;
		    }
		}
	    }
	}


	if( mask )
	    for(sv = 0 ; sv < seqnum ; sv++)
		for(hv = 0 ; hv < endlen ; hv++ )
		    if( endseq[sv][hv] != gap_char )
			if( endseq[sv][hv] == tolower( endseq[sv][hv] ) )
			    endseq[sv][hv] = '*' ;


	if( col_score )
	{
	    fprintf(fp_col_score , "# 1 %d \n" , endlen  );
	    fprintf(fp_col_score,"# %s \n", upg_str);
	}

	edialign_plot_calc( num_all_it_dia , endlen , weight_count , plot ,
			   all_it_dia , fp_col_score);

	edialign_wgt_type_count( num_all_it_dia , endlen , plus_count,
				minus_count, nuc_count , frg_involved,
				all_it_dia );
      

	lc = (endlen-1)/char_per_line;
	for(hv=0;hv<seqnum;hv++)
	    first_pos[hv] = begin[hv] ;


	for( k = 0 ; k <= lc ; k++ )
	{
	    for( hv = 0 ; hv < seqnum ; hv++ )
	    { 
		fprintf(fp, "%s", seq_name[hv] );

		fprintf(fp,"%8d  ", first_pos[hv]);


		for(i=0;i<edialign_minu2(char_per_line,endlen-k*char_per_line);
		    i++)
		{
		    if(!(i%10))fprintf(fp, " ");
		    fprintf(fp, "%c",endseq[hv][k*char_per_line+i]);
		    if(endseq[hv][k*char_per_line+i] != gap_char)
			first_pos[hv]++;
		}
		fprintf(fp, " \n");
	    }

	    fprintf(fp,"         ");
	    for( i = 0 ; i < edialign_minu2(char_per_line , endlen-k*
					    char_per_line )
		; i++ )
	    {
		if(!(i%10))fprintf(fp, " ");
		fprintf(fp, "%c",clear_seq[k*char_per_line+i]);
	    }

	    if( plot_num )
		fprintf(fp, " \n");


      
	    if( quali_num == 0 )  
		for( pn = 0 ; pn < plot_num ; pn ++ ) 
		{ 
		    fprintf(fp,"                      ");
		    for(i=0;i<edialign_minu2(char_per_line,endlen-k*
					     char_per_line);i++)
		    {
			if( !(i%10) )fprintf(fp, " ");
			if( plot[ k*char_per_line + i ]  >  pn )
			    fprintf(fp, "*");
			else
			    fprintf(fp, " ");
		    }
		    fprintf(fp, " \n");

		    if( plot_num == 1 )  
			fprintf(fp, " \n");
		}


	    if( quali_num )
	    {
		for( i = 0 ; i < SEQ_NAME_LEN ; i++ )
		{ 
		    fprintf(fp," ");
		}
            
		fprintf(fp,"          ");
		for( i = 0 ; i < edialign_minu2(char_per_line,endlen-k*
						char_per_line);
		    i++ )
		{ 
		    if( !(i%10) )fprintf(fp, " ");
		    pl_int = (ajint) (9 * plot[ k * char_per_line + i ] /
				      plot_num);
		    fprintf(fp, "%d", pl_int );
		} 
		fprintf(fp, " \n");
	    } 

/***********************************************************************
	      
	      fprintf(fp, " \n");
	      if( wgt_type > 1 )
	      {
	      for( i = 0 ; i < SEQ_NAME_LEN ; i++ )
	      { 
	      fprintf(fp," ");
	      }
	      
	      fprintf(fp,"  plus    ");
	      for( i = 0 ; i < edialign_mini2( char_per_line , endlen-k*
	      char_per_line );
	      i++ ) { 
	      if( !(i%10) )fprintf(fp, " ");
	      fprintf(fp, "%d", plus_count[ k * char_per_line + i ] );
	      } 
	      fprintf(fp, " \n");
	      } 
	      
	      if( wgt_type > 1 ) {
	      for( i = 0 ; i < SEQ_NAME_LEN ; i++ ) { 
	      fprintf(fp," ");
	      }
	      
	      fprintf(fp,"  minus   ");
	      for( i = 0 ; i < edialign_mini2( char_per_line ,endlen-k*
	      char_per_line);
	      i++ ) { 
	      if( !(i%10) )fprintf(fp, " ");
	      fprintf(fp, "%d", minus_count[ k * char_per_line + i ] );
	      } 
	      fprintf(fp, " \n");
	      } 
	      
	      if( wgt_type > 1 ) {
	      for( i = 0 ; i < SEQ_NAME_LEN ; i++ ) { 
	      fprintf(fp," ");
	      }
	      
	      fprintf(fp,"  nuc     ");
	      for( i = 0 ; i < edialign_mini2( char_per_line , endlen-k*
	      char_per_line );
	      i++ ) { 
	      if( !(i%10) )fprintf(fp, " ");
	      fprintf(fp, "%d", nuc_count[ k * char_per_line + i ] );
	      } 
	      fprintf(fp, " \n");
	      fprintf(fp, " \n");
	      } 
	      
************************************************************************/

	    if( wgt_type_plot ) 
		if( wgt_type == 3 )
		{

		    fprintf(fp,"sim. level");

		    for( i = 0 ; i < SEQ_NAME_LEN ; i++ )
		    { 
			fprintf(fp," ");
		    }
            
		    for(i=0; i < edialign_minu2(char_per_line,endlen-k*
						char_per_line);
			i++ )
		    { 
			if( !(i%10) )fprintf(fp, " ");
			sim_char = '.' ; 

			if( frg_involved[ k * char_per_line + i ] ) {   

			  f_inv = (float) frg_involved[ k *
							char_per_line + i] ; 
			    frac_plus =  plus_count[ k * char_per_line + i ] /
				f_inv ;
			    frac_minus =  minus_count[k * char_per_line + i] /
				f_inv ;
			    frac_nuc =  nuc_count[ k * char_per_line + i ] /
				f_inv ;

			    if ( frac_plus > weak_wgt_type_thr )
			    {
				if( crick_strand ) 
				    sim_char = 'f' ;
				else  
				    sim_char = 'p' ;
			    }
			    if ( frac_plus > strong_wgt_type_thr )
			    {
				if( crick_strand ) 
				    sim_char = 'F' ; 
				else 
				    sim_char = 'P' ;
			    }
			    if ( frac_minus > weak_wgt_type_thr )
				sim_char = 'r' ; 
			    if ( frac_minus > strong_wgt_type_thr )
				sim_char = 'R' ; 

			    if ( frac_nuc > weak_wgt_type_thr )
				sim_char = 'n' ; 
			    if ( frac_nuc > strong_wgt_type_thr )
				sim_char = 'N' ; 

			}  
			fprintf(fp, "%c", sim_char );
		    } 
		    fprintf(fp, " \n");
		    fprintf(fp, " \n");
		} 

       
       

	   
	    fprintf(fp, " \n");
   
	} /*   for(k=0;k<=lc;k++)  */
    

	if( fasta_file )
	{ 

	    for(sv=0;sv<seqnum;++sv)
	    {
		eseq = ajSeqNewRes(endlen+1);
		ajSeqAssignNameC(eseq,seq_name[sv]);
		ajSeqAssignSeqC(eseq,endseq[sv]);
		if(isprot)
		    ajSeqSetProt(eseq);
		else
		    ajSeqSetNuc(eseq);
		ajSeqoutWriteSeq(seqout,eseq);
		ajSeqDel(&eseq);
	    }
	    
/*
	    for(sv = 0 ; sv < seqnum ; sv++ )
	    {
		fprintf(fp2,">%s", full_name[sv]);
		for(i = 0 ; i < endlen ; i++)
		{
		    if( ! ( i % 50 ) )   
			fprintf(fp2,"\n"); 
		    fprintf(fp2,"%c", endseq[sv][i]);  
		}
                    
		fprintf(fp2,"\n ");         
		if( sv < ( seqnum - 1 ) )
		    fprintf(fp2,"\n");             
	    }
*/
            ajSeqoutClose(seqout);
	}     
   
       
	if( cw_file )
	{   
	    block_no = 0;
 
	    fprintf(fp4,"DIALIGN 2.1 multiple sequence alignment \n\n");
	    fprintf(fp4,"// \n\n\n");
          
	    while( block_no * 60 < (ajint) endlen )
	    {
		char_no = edialign_minu2( 60 ,  ( endlen - block_no * 60 ) ) ;
		for( sv = 0 ; sv < seqnum ; sv++ )
		{
		    fprintf(fp4,"%s ", seq_name[sv] );
		    for( i = 0 ; i < char_no ; i++)
			fprintf(fp4,"%c", endseq[sv][ block_no * 60 + i ] );
		    fprintf(fp4,"\n");
		}
		fprintf(fp4,"\n\n");
		block_no++; 
	    } 


	}
	 

	if( msf_file )
	{ 
	    msf_lines = endlen / 50;
	    if(endlen % 50)
		msf_lines = msf_lines + 1;
          

	    fprintf(fp3,"DIALIGN 2\n\n\n");
	    fprintf(fp3,"   MSF: %d \n\n", endlen);

	    for( sv = 0 ; sv < seqnum ; sv++ )
		fprintf(fp3," Name: %s    Len: %d \n", seq_name[sv],
			seqlen[sv] );
	    fprintf(fp3,"\n// \n\n");

	    for(lv = 0 ; lv < msf_lines ; lv++ )
	    {
		add = lv * 50;
		max_p = edialign_mini2( endlen - add , 50 );
           
		for( sv = 0 ; sv < seqnum ; sv++ )
		{
		    fprintf(fp3, "%s", seq_name[sv] );
		    for(i=0 ; i < 4 ; i++ )
			fprintf(fp3, " "); 
               
		    for(i = 0 ; i < max_p ; i++)
		    {
			if( !(i%10) )fprintf(fp3, " ");
			if(  endseq[sv][add + i] == '-' )
			    fprintf(fp3,".");
			else
			    fprintf(fp3,"%c", endseq[sv][add + i]);
		    } 
		    fprintf(fp3,"\n"); 
		}
		fprintf(fp3,"\n\n");
	    } 

	}


	if( ( seqnum > 2 ) && ( ref_seq == 0 ) )  
	{ 
	    fprintf(fp,"\n \n \n   Sequence tree:\n");
	    fprintf(fp,"   ==============\n\n");

	    if( ! strcmp( clust_sim , "av" ) )
		fprintf(fp,"Tree constructed using UPGMA ");
	    fprintf(fp,"based on DIALIGN fragment weight scores");

	    if( ! strcmp( clust_sim , "max" ) )
		fprintf(fp,"Tree constructed using maximum linkage "
			"clustering");


	    if( ! strcmp( clust_sim , "min" ) )
		fprintf(fp,"Tree constructed using minimum linkage "
			"clustering");


	    fprintf(fp,"\n \n%s", upg_str);
	}

	fprintf(fp,"\n \n \n");
           
	for(hv=0;hv<seqnum;hv++)
	    AJFREE(hseq[hv]);  

	for(hv=0;hv<seqnum;hv++)
	    AJFREE(endseq[hv]);

	if( fragno > 0 )
	    AJFREE( fragments );

	AJFREE(plot);

	AJFREE(weight_count);


    } /* for(bc=0;bc<1;bc++) */


    for(hv=0;hv<seqnum;hv++)
    {
	free(shift[hv]);  
	free(inv_shift[hv]);  
    }
    
    AJFREE(endseq);
    AJFREE (hseq);
    AJFREE(begin);
    AJFREE(end);
    AJFREE(b_len);
    AJFREE(first_pos);

    AJFREE(inv_shift);
    AJFREE(shift);
    AJFREE(frg_involved);
    AJFREE(nuc_count);
    AJFREE(minus_count);
    AJFREE(plus_count);
    AJFREE(weight_count);
    AJFREE(plot);
    AJFREE(clear_seq);
        
    
    return;
}




/* @funcstatic edialign_para_print ****************************************
**
** edialign_para_print
**
** @param [u] s_f [char*] Undocumented
** @param [u] fpi [FILE*] Undocumented
** @return [void]
*****************************************************************************/

static void edialign_para_print( char *s_f, FILE *fpi )
{
    ajuint hv;
   
    (void) s_f;				/* make it used */

    {
	if(cd_gobics)
	{
            fprintf(fpi," \n                          CHAOS / DIALIGN  \n");
            fprintf(fpi,"                          ***************\n \n");
 
            if( BETA )
		fprintf(fpi,"                           beta version\n\n"); 

            fprintf(fpi,"          Program code written by \n");
            fprintf(fpi,"          Burkhard Morgenstern, Said Abdeddaim and "
		    "Michael Brudno \n\n");
            fprintf(fpi,"             e-mail contact: ");
            fprintf(fpi,"dialign (at) gobics (dot) de \n \n");
            fprintf(fpi,"          Published research assisted");
            fprintf(fpi," by CHAOS / DIALIGN should cite:  \n \n");
            fprintf(fpi,"             Michael Brudno et al.");
            fprintf(fpi," (2003)\n");
            fprintf(fpi,"             \"Fast and sensitive multiple "
		    "alignment");
            fprintf(fpi," of large genomic sequences\" \n"); 
            fprintf(fpi,"             BMC Bioinformatics 4:66 \n");
            fprintf(fpi,"             http://www.biomedcentral.com/1471-"
		    "2105/4/66 \n\n");
	}
	else
	{
            fprintf(fpi," \n                           DIALIGN 2.2.1 \n");
            fprintf(fpi,"                           *************\n \n");
 
            if( BETA )
		fprintf(fpi,"                           beta version\n\n"); 

            fprintf(fpi,"          Program code written by Burkhard");
            fprintf(fpi," Morgenstern and Said Abdeddaim \n");
            fprintf(fpi,"             e-mail contact: ");
            fprintf(fpi,"dialign (at) gobics (dot) de \n \n");
            fprintf(fpi,"          Published research assisted");
            fprintf(fpi," by DIALIGN 2 should cite:  \n \n");
            fprintf(fpi,"             Burkhard Morgenstern");
            fprintf(fpi," (1999).\n");
          
            fprintf(fpi,"             DIALIGN 2: improvement of the");
            fprintf(fpi," segment-to-segment\n             approach");
            fprintf(fpi," to multiple sequence alignment.\n");
            fprintf(fpi,"             Bioinformatics 15,");
            fprintf(fpi," 211 - 218. \n\n");
	}

	fprintf(fpi,"          For more information, please visit");
	fprintf(fpi," the DIALIGN home page at \n\n             ");
	fprintf(fpi,"http://bibiserv.techfak.uni-bielefeld.de/dialign/");
	fprintf(fpi," \n \n");

	fprintf(fpi,"         ***************************************"
		"*********************\n \n");
    } 

    if( online )
    { 
	fprintf(fpi,"\n\n    The following options have been used: \n\n") ;
	fprintf(fpi,"     - sequences are");
	if( wgt_type == 0 ) 
	    fprintf(fpi," protein sequences \n");     
	if( wgt_type == 1 ) 
	    fprintf(fpi," nucleic acid sequences without translation "
		    "option\n");     
	if( wgt_type == 2 ) 
	    fprintf(fpi," nucleic acid sequences with translation option\n");
	if( speed_optimized ) 
	    fprintf(fpi,"     - speed optimized,"); 
	fprintf(fpi," see user guide for details \n"); 
	if( anchors )
	    fprintf(fpi,"     - anchor points used\n" ); 
	fprintf(fpi,"\n"); 
    }
    else
	fprintf(fpi,"\n\n   %s \n\n", input_line );

    fprintf(fpi," \n");

    fprintf(fpi,"   Aligned sequences:          length:\n");
    fprintf(fpi,"   ==================          =======\n \n");
       
    for(hv=0;hv<seqnum;hv++)
    {
	fprintf(fpi, " %3d) ", hv + 1 );
	fprintf(fpi, "%s", seq_name[hv] );
	fprintf(fpi, "         %9d\n",seqlen[hv]);
    }
   
   

    fprintf(fpi, "\n   Average seq. length:" );
    fprintf(fpi, "      %9.1f \n", av_len );

    fprintf(fpi,"\n\n   Please note that only upper-case letters are");
    fprintf(fpi," considered to be aligned. \n");

    fprintf(fpi,"\n\n   Alignment (DIALIGN format):\n");
    fprintf(fpi,"   ===========================\n \n");

    return;
}




#if 0
/* @funcstatic edialign_para_read ****************************************
**
** edialign_para_read
**
** @param [r] num [int] Undocumented
** @param [u] arg [char**] Undocumented
** @return [void]
*****************************************************************************/

static void edialign_para_read( int num , char ** arg )      
{
    ajint an = 1;

    
    while( an < num - 1 )
    { 

        if( 
	   strcmp( arg[an] , "-afc")  && /* create file containing 
					    ALL fragments considered 
					    for alignment */
	   strcmp( arg[an] , "-afc_v") && /* like -afc with fragments
					     explicitly printed */
	   strcmp( arg[an] , "-b1")   && /* break */ 
	   strcmp( arg[an] , "-b2")   && /* break */ 
	   strcmp( arg[an] , "-bs")   && /* bubble sort */ 
	   strcmp( arg[an] , "-csc") && /* column score output */ 
	   strcmp( arg[an] , "-cs")   && /* crick strand */ 
	   strcmp( arg[an] , "-cw")   && /* additional output file
					    in clustalw format */
	   strcmp( arg[an] , "-d1w")  && /* old weight fkt */

	   strcmp( arg[an] , "-ds")   &&    
	   strcmp( arg[an] , "-fa")   && /* separate file with 
					    alignment in fasta format */
	   strcmp( arg[an] , "-ff")   && /* fragment file  */
	   strcmp( arg[an] , "-fn")   && /* name of output file  */
	   strcmp( arg[an] , "-fop")  && /* create file containing 
					    fragments selected for 
					    optimal pairwise alignment */
	   strcmp( arg[an] , "-fsm")  && /* create file containing 
					    consistent fragments in 
					    multiple alignment (in 
					    format needed for -xfr ) */
	   strcmp( arg[an] , "-fsmv") && /* same as -fsm but verbose */ 
	   strcmp( arg[an] , "-cd_gobics") && /* chaos + dialign @ gobics */
	   strcmp( arg[an] , "-lgs_t")   && /* genomic sequences, transl. */
	   strcmp( arg[an] , "-istep")   && /* max iteration steps */ 
	   strcmp( arg[an] , "-it")   && /* iteration */ 
	   strcmp( arg[an] , "-iw")   && /* ind. weights */ 
	   strcmp( arg[an] , "-lgs")  && /* genomic sequences  */
	   strcmp( arg[an] , "-lgsx")  && /* genomic sequences, accurate +
					     textual alignment   */
	   strcmp( arg[an] , "-lmax") && /* max. length of diag. */  
	   strcmp( arg[an] , "-lo")   && /* long output */ 
	   strcmp( arg[an] , "-ma")   && /* mixed weights */
	   strcmp( arg[an] , "-anc")  && /* anchor regions */ 
	   strcmp( arg[an] , "-mask") && 
	   strcmp( arg[an] , "-mat")  && /* calc. subst. freq. matrix */
	   strcmp( arg[an] , "-mat_thr")  && /* thr for sbst. fr. mat. */
	   strcmp( arg[an] , "-max_link") && /* max. linkage clustering */ 
	   strcmp( arg[an] , "-min_link") && /* min. linkage clustering */ 
	   strcmp( arg[an] , "-mot")  && /* motifs considered */ 
	   strcmp( arg[an] , "-msf")  && /* separate file with 
					    alignment in msf format */
	   strcmp( arg[an] , "-n")    && /* DNA/RNA sequences */
	   strcmp( arg[an] , "-nas")    && /* no anchor sorting */
	   strcmp( arg[an] , "-nt")   && /* DNA/RNA sequences with 
					    translation option */
	   strcmp( arg[an] , "-nta")  && /* no textual alignment */ 
	   strcmp( arg[an] , "-o")    && /* optimized  */
	   strcmp( arg[an] , "-online")    && /* online */
	   strcmp( arg[an] , "-ow")   && /* overlap weights */ 
	   strcmp( arg[an] , "-pamnd") && /* print av. max. number of frg. */
	   strcmp( arg[an] , "-pand") && /* print av. number of diag. */
	   strcmp( arg[an] , "-pao")  && /* pairw. alignments only */
	   strcmp( arg[an] , "-ref_seq")  && /* seq_2, ... , seq_n 
						aligned to seq_1 */
	   strcmp( arg[an] , "-stars")&& /* maximum number of stars under 
				    alignment indicating relative similarity*/
	   strcmp( arg[an] , "-pst")  && /* print status */
	   strcmp( arg[an] , "-sc")  &&	/* self comparison */
	   strcmp( arg[an] , "-smin")  &&  
	   strcmp( arg[an] , "-stdo")  && /* standard output */
	   strcmp( arg[an] , "-ta")  &&	/* textual alignment*/
	   strcmp( arg[an] , "-thr") &&	/* threshold */
	   strcmp( arg[an] , "-ts") &&	/* time stamps */
	   strcmp( arg[an] , "-wgtpr") && /* weight print */
	   strcmp( arg[an] , "-wgtprx") && /* weight print */
	   strcmp( arg[an] , "-wtp") &&	/* weight type plot */
	   strcmp( arg[an] , "-xfr")	/* excluded fragments */

	   )
	{
            printf("\n \n   Arguments in command line make no sense! \n \n");
            printf("\n   Unknown option %s \n \n \n \n",  arg[an] );
            embExitBad();
	}  

        if( !strcmp( arg[an] , "-afc") )
	    afc_file = 1;

        if( !strcmp( arg[an] , "-afc_v") ) { 
	    afc_file = 1;
	    afc_filex = 1 ;
        }

        if( !strcmp( arg[an] , "-b1") )
	    break1 = 1;

        if( !strcmp( arg[an] , "-b2") )
	    break2 = 1;

        if( !strcmp( arg[an] , "-bs") )
	    bubblesort = 1;
        
        if( !strcmp( arg[an] , "-csc") )
	    col_score = 1;

        if( !strcmp( arg[an] , "-cd_gobics") )
	    cd_gobics = 1;

        if( !strcmp( arg[an] , "-cs") )
	    crick_strand = 1;

        if( !strcmp( arg[an] , "-cw") )
	    cw_file = 1;
	
        if( !strcmp( arg[an] , "-ds") )
	    dna_speed = 1 ;

        if( !strcmp( arg[an] , "-fa") )
	    fasta_file = 1;

        if( !strcmp( arg[an] , "-ff") )
	    frag_file = 1;

        if( !strcmp( arg[an] , "-fop") )
	    dia_pa_file = 1;

        if( !strcmp( arg[an] , "-fsm") )
	    frg_mult_file = 1;

        if( !strcmp( arg[an] , "-fsmv") ) { 
	    frg_mult_file = 1;
	    frg_mult_file_v = 1;
        }

        if( !strcmp( arg[an] , "-it") )
	    iter_cond_prob = 1;

        if( !strcmp( arg[an] , "-iw") )
	    overlap_weights = 0;

        if( !strcmp( arg[an] , "-lgs") ) {
	    wgt_type = 3 ;
	    /*          iter_cond_prob = 1 ; 
	     */ 
	    threshold = 2.0 ;
	    lmax = 30 ;
	    thr_sim_score = 8 ;
	    strict = 1 ;
	    textual_alignment = 0 ;
	    /* dia_pa_file = 1; */ 
	    frag_file = 1 ;
	    dna_speed = 1 ;
	    crick_strand = 1 ;
	    lgs_option = 1 ;
	    print_status = 1 ; 
        }

        if( !strcmp( arg[an] , "-lgs_t") ) {
	    wgt_type = 2 ;
	    iter_cond_prob = 1 ;
	    threshold = 0.0 ;
	    lmax = 30 ;
	    thr_sim_score = 8 ;
	    strict = 1 ;
	    textual_alignment = 0 ;
	    dia_pa_file = 1;
	    frag_file = 1 ;
	    dna_speed = 1 ;
	    print_status = 1 ; 
        }

        if( !strcmp( arg[an] , "-lgsx") ) {
	    wgt_type = 3 ;
	    iter_cond_prob = 1 ;
	    strict = 1 ;
	    frag_file = 1 ;
	    crick_strand = 1 ;
	    lgs_option = 1 ;
	    print_status = 1 ; 
        }

        if( !strcmp( arg[an] , "-lo") )
	    long_output = 1;

        if( !strcmp( arg[an] , "-ma") ) {
	    wgt_type = 3;
        }

        if( !strcmp( arg[an] , "-anc") )
	    anchors = 1;

        if( !strcmp( arg[an] , "-mask") )
	    mask = 1;

        if( !strcmp( arg[an] , "-max_link") )
	    strcpy (clust_sim , "max" );

        if( !strcmp( arg[an] , "-min_link") )
	    strcpy (clust_sim , "min" );

        if( !strcmp( arg[an] , "-msf") )
	    msf_file = 1;

        if( !strcmp( arg[an] , "-n") ) {
	    wgt_type = 1;
        }

        if( !strcmp( arg[an] , "-nas") ) {
	    nas = 1;
        }

        if( !strcmp( arg[an] , "-nt") )
	    wgt_type = 2;

        if( !strcmp( arg[an] , "-nta") )
	    textual_alignment = 0;

        if( !strcmp( arg[an] , "-o") )
	{
            speed_optimized = 1 ; 
            threshold = 0.5 ;
            lmax = 30 ;
            thr_sim_score = 8 ;
	}

        if( !strcmp( arg[an] , "-ow") )
	    ow_force = 1;

        if( !strcmp( arg[an] , "-pao") )
	    pa_only = 1;

        if( !strcmp( arg[an] , "-pamnd") )
	    pr_av_max_nd = 1;

        if( !strcmp( arg[an] , "-pand") )
	    pr_av_nd = 1;

        if( !strcmp( arg[an] , "-pst") )
	    print_status = 1;

        if( !strcmp( arg[an] , "-red") )
	    redundant = 1;

        if( !strcmp( arg[an] , "-mat") )
	    sf_mat = 1;

        if( !strcmp( arg[an] , "-online") )
	    online = 1;
 
        if( !strcmp( arg[an] , "-ref_seq") )
	    ref_seq = 1;
 
        if( !strcmp( arg[an] , "-sc") )
	    self_comparison = 1;

        if( !strcmp( arg[an] , "-stdo") )
	    standard_out = 1;

        if( !strcmp( arg[an] , "-strict") )
	    strict = 1;
 
        if( !strcmp( arg[an] , "-ta") )
	    textual_alignment = 1 ;

        if( !strcmp( arg[an] , "-ts") )
	    time_stamps = 1 ;

        if( !strcmp( arg[an] , "-wgtpr") )
	    wgt_print = 1 ;

        if( !strcmp( arg[an] , "-wgtprx") )  
	    wgt_print_x = 1 ;

        if( !strcmp( arg[an] , "-wtp") )  
	    wgt_type_plot = 1 ;

        if( !strcmp( arg[an] , "-xfr") )
	    exclude_frg = 1 ;


 
 
	/********************************************************************/


        if( !strcmp( arg[an] , "-fn") )
	{
	    if( an + 2 < num )  
	    { 
		strcpy( output_name , arg[++an] );
		default_name = 0;
	    } 
	    else
	    {
		printf("\n \n   Arguments in command line don't make sense! "
		       "\n");
		printf("   (Name of output file not properly specified) "
		       "\n \n");
		embExitBad();
	    }
	}
	



	/********************************************************************/


        if( !strcmp( arg[an] , "-istep") )
	{
	    if( ( an + 2 < num ) && edialign_num_test( arg[an + 1] ) )  
		max_itnum = atoi( arg[++an] ); 
	    else
	    {
		printf("\n \n   Arguments in command line don't make "
		       "sense! \n");
		printf("   (max_itnum not properly specified) \n \n");
		embExitBad();
	    }
	}
	


	/********************************************************************/


        if( !strcmp( arg[an] , "-lmax") )
	{
	    if( ( an + 2 < num ) && edialign_num_test( arg[an + 1] ) )  
		lmax = atoi( arg[++an] ); 
	    else
	    {
		printf("\n \n   Arguments in command line don't make "
		       "sense! \n");
		printf("   (lmax not properly specified) \n \n");
		embExitBad();
	    }
	}
	


        /********************************************************************/


        if( !strcmp( arg[an] , "-stars") )
	{
	    if( ( an + 2 < num ) && edialign_num_test( arg[an + 1] ) )
	    { 
		plot_num = atoi( arg[++an] ); 
		quali_num = 0 ; 
	    }
	    else
	    {
		printf("\n \n   Arguments in command line don't make "
		       "sense! \n");
		printf("   (Number of \"*\" characters not properly "
		       "specified) \n \n");
		embExitBad();
	    }
	}
	


	/********************************************************************/


        if( !strcmp( arg[an] , "-smin") )
	{
	    if( (an + 2 < num) && edialign_num_test( arg[an + 1] ) )  
		thr_sim_score = atoi( arg[++an] );
	    else
	    {
		printf("\n \n   Arguments in command line don't make "
		       "sense! \n");
		printf("   (Speed not properly specified) \n \n");
		embExitBad();
	    }
	}
	


        /********************************************************************/


        if( !strcmp( arg[an] , "-thr") )
	{
	    if( (an + 2 < num) && edialign_num_test( arg[an + 1] ) )  
	    {
		threshold = atof( arg[++an] );
	    }
	    else
	    {
		printf("\n \n   Arguments in command line don't make "
		       "sense! \n");
		printf("   (Threshod not properly specified) \n \n");
		embExitBad();
	    }
	}
	


	/********************************************************************/


        if( !strcmp( arg[an] , "-mat_thr") )
	{
	    if( (an + 2 < num) && edialign_num_test( arg[an + 1] ) )  
	    {
		sf_mat_thr = atof( arg[++an] );
	    }
	    else
	    {
		printf("\n \n   Arguments in command line don't "
		       "make sense! \n");
		printf("   (subst. mat. threshod not properly specified) "
		       "\n \n");
		embExitBad();
	    }  
	}
	


	/********************************************************************/


        if( !strcmp( arg[an] , "-mot") )
	{
	    if(  ( an + 4 < num )            && 
	       edialign_num_test( arg[ an + 2 ] )   && 
	       edialign_num_test( arg[ an + 3 ] ) 
	       )  
	    {
		motifs = 1 ;  
		strcpy( mot_regex , arg[++an] );
		mot_factor = atof( arg[++an] ) ;
		mot_offset_factor = atof( arg[++an] ) ;
		regex_len = strlen( mot_regex ) ;
	    }
	    else
		regex_format_complain();
	}
	

        /********************************************************************/

        an++;
    } 

    return;
}
#endif




/* @funcstatic edialign_erreur ****************************************
**
** edialign_erreur
**
** @param [u] message [const char*] Undocumented
** @return [void]
*****************************************************************************/

static void edialign_erreur(const char *message)
{
    ajFatal("%s\n", message);
}




/* @funcstatic edialign_allouer ****************************************
**
** edialign_allouer
**
** @param [r] taille [size_t] Undocumented
** @return [void*] Undocumented
*****************************************************************************/

static void* edialign_allouer(size_t taille)
{
    void *pointeur;

    if (taille == 0)
	taille = 1;

    pointeur = (void *) malloc(taille);

    if (pointeur==NULL)
 	edialign_erreur("out of memory");

    return(pointeur);
}




/* @funcstatic edialign_reallouer ****************************************
**
** edialign_reallouer
**
** @param [u] pointeur [void*] Undocumented
** @param [r] taille [size_t] Undocumented
** @return [void*] Undocumented
*****************************************************************************/

static void* edialign_reallouer(void *pointeur, size_t taille)
{

    pointeur = (void *) realloc(pointeur, taille);

    if (pointeur==NULL)
  	edialign_erreur("out of memory");

    return(pointeur);
}




/* @funcstatic edialign_liberer ****************************************
**
** edialign_liberer
**
** @param [d] pointeur [void*] Undocumented
** @return [void]
*****************************************************************************/

static void edialign_liberer(void *pointeur)
{

    free(pointeur);

    return;
}




/* @funcstatic edialign_callouer_mat ****************************************
**
** edialign_callouer_mat
**
** @param [r] t_elt [size_t] Undocumented
** @param [r] nb_lig [size_t] Undocumented
** @param [r] nb_col [size_t] Undocumented
** @return [void**] Undocumented
*****************************************************************************/

static void** edialign_callouer_mat(size_t t_elt, size_t nb_lig, size_t nb_col)
{
    void **pointeur;
    ajint i;
    ajint imax = nb_lig;

    pointeur = (void **) edialign_allouer(nb_lig * sizeof(void *));

    for (i=0; i < imax; i++)
	pointeur[i] = (void *) edialign_allouer(nb_col * t_elt);

    return(pointeur);
}




/* @funcstatic edialign_recallouer_mat **************************************
**
** edialign_recallouer_mat
**
** @param [u] pointeur [void**] Undocumented
** @param [r] t_elt [size_t] Undocumented
** @param [r] anc_nb_lig [size_t] Undocumented
** @param [r] nb_lig [size_t] Undocumented
** @param [r] nb_col [size_t] Undocumented
** @return [void**] Undocumented
*****************************************************************************/

static void** edialign_recallouer_mat(void **pointeur, size_t t_elt,
				      size_t anc_nb_lig, 
				      size_t nb_lig, size_t nb_col)
{
    size_t i;

    if (anc_nb_lig == nb_lig)
	return(pointeur);

    for (i=nb_lig; i < anc_nb_lig; i++)
	edialign_liberer(pointeur[i]);

    pointeur = (void **) edialign_reallouer(pointeur, nb_lig * sizeof(void *));

    for (i=anc_nb_lig; i < nb_lig; i++)
	pointeur[i] = (void *) edialign_allouer(nb_col * t_elt);

    return(pointeur);
}




#if 0
/* @funcstatic edialign_recallouer_mat2 **************************************
**
** edialign_recallouer_mat2
**
** @param [u] pointeur [void**] Undocumented
** @param [r] t_elt [size_t] Undocumented
** @param [r] anc_nb_lig [size_t] Undocumented
** @param [r] nb_lig [size_t] Undocumented
** @param [r] nb_col [size_t] Undocumented
** @return [void**] Undocumented
*****************************************************************************/

static void** edialign_recallouer_mat2(void **pointeur, size_t t_elt, 
				       size_t anc_nb_lig, size_t nb_lig,
				       size_t nb_col)
{
    ajint i;

    for (i=nb_lig; i < anc_nb_lig; i++)
	edialign_liberer(pointeur[i]);

    pointeur = (void **) edialign_reallouer(pointeur, nb_lig * sizeof(void *));

    for (i=0; i < edmin(anc_nb_lig, nb_lig); i++)
	pointeur[i] = (void *) edialign_reallouer(pointeur[i], nb_col * t_elt);

    for (i=anc_nb_lig; i < nb_lig; i++)
	pointeur[i] = (void *) edialign_allouer(nb_col * t_elt);

    return(pointeur);
}
#endif




/* @funcstatic edialign_liberer_mat **************************************
**
** edialign_liberer_mat
**
** @param [d] pointeur [void**] Undocumented
** @param [r] nb_lig [size_t] Undocumented
** @return [void]
*****************************************************************************/

static void edialign_liberer_mat(void **pointeur, size_t nb_lig)
{
    size_t i;

    for (i=0; i < nb_lig; i++)
	edialign_liberer(pointeur[i]);

    edialign_liberer(pointeur);

    return;
}




#if 0
/* @funcstatic edialign_ouvrir **************************************
**
** edialign_ouvrir. unused
**
** @param [u] nomfich [char*] Undocumented
** @param [u] mode [char*] Undocumented
** @return [FILE*] Undocumented
*****************************************************************************/

static FILE* edialign_ouvrir(char *nomfich, char *mode)
{
    FILE *f;

    if ((f = fopen(nomfich, mode)) == NULL) 
    {
 	printf("fopen(\"%s\",\"%s\"): ", nomfich, mode);
	edialign_erreur("enable to open file");
    }

    return f;
}




/* @funcstatic edialign_fermer **************************************
**
** edialign_fermer. unused
**
** @param [d] f [FILE*] Undocumented
** @return [void]
*****************************************************************************/

static void edialign_fermer(FILE *f)
{
    if (fclose(f) == EOF)
  	edialign_erreur("enable to close file");

    return;
}




/* @funcstatic edialign_fcopie **************************************
**
** edialign_fcopie. unused
**
** @param [u] fdestination [FILE*] Undocumented
** @param [u] fsource [FILE*] Undocumented
** @return [void]
*****************************************************************************/

static void edialign_fcopie(FILE *fdestination, FILE *fsource)
{
    char line[TAILLE_MAX_LIGNE_FICHIER];

    while (fgets(line, TAILLE_MAX_LIGNE_FICHIER, fsource) != NULL)
 	fputs(line, fdestination);
 
    fflush(fdestination);
    return;
}




/* @funcstatic edialign_strmin **************************************
**
** edialign_strmin. unused
**
** @param [u] p [char*] Undocumented
** @return [void]
*****************************************************************************/

static void edialign_strmin(char *p)
{
    char c;

    for (; (c=*p); p++)
	*p = tolower(c);

    return;
}




/* @funcstatic edialign_strmax **************************************
**
** edialign_strmax. unused
**
** @param [u] p [char*] Undocumented
** @return [void]
*****************************************************************************/

static void edialign_strmax(char *p)
{
    char c;

    for (; (c=*p); p++)
	*p = toupper(c);

    return;
}
#endif




/* @funcstatic edialign_regex_complain **************************************
**
** edialign_regex_complain
**
** @param [r] regex [const char*] Undocumented
** @return [void]
*****************************************************************************/

__noreturn static void edialign_regex_complain( const char *regex )
{
    printf("\n   bracket structure in regular expression makes no sense \n");
    printf("\n          %s  \n\n", regex) ; 
    printf("   program terminated\n\n"); 
    embExitBad();  
}




/* @funcstatic edialign_struc_check **************************************
**
** edialign_struc_check
**
** @param [u] regex [char*] Undocumented
** @return [void]
*****************************************************************************/

static void edialign_struc_check( char *regex )
{
    ajint p;
    ajint bracket_count = 0;

    for( p = 0 ; p < MAX_REGEX; p++ )
    { 
	char_num[ p ] = 0 ;
    }

    for( p = 0 ; p < regex_len ; p++ )
    {

	if( regex[ p ] == '[' ) 
	    bracket_count++ ; 
       
	if( ( regex[ p ] != '[' ) && ( regex[ p ] != ']' ) )
	{ 
	    char_num[ mot_len ]++ ;  
	    regex[ p ] = toupper( regex[ p ] ) ; 
	}

	if( regex[ p ] == ']' ) 
	    bracket_count-- ; 

	if( ( regex[ p ] == ']' ) || ( bracket_count == 0 ) )  
	    mot_len++ ; 


	if( ( bracket_count < 0 ) || ( bracket_count > 1 ) )   
	    edialign_regex_complain( regex ) ; 


    }

    if( bracket_count != 0 ) 
	edialign_regex_complain( regex ) ; 

    return;
}




/* @funcstatic edialign_regex_parse *****************************************
**
** edialign_regex_parse
**
** @param [u] mot_regex [char*] Undocumented
** @return [void]
*****************************************************************************/

static void edialign_regex_parse(char *mot_regex)
{
    ajuint i;
    ajint p;
    ajint mp = 0; 
    ajint in_bracket = 0;  
    ajint char_c = 0 ; 


    if((mot_pos = ( short  ** ) calloc( seqnum , sizeof( short *) ) ) == NULL)
    {
	printf(" problems with memory allocation");
	printf(" for `mot_pos' !  \n \n");
	embExitBad();
    }

    for(i = 0; i < seqnum; i++)
	if((mot_pos[i] = ( short *) calloc((seqlen[i] + 2),
					   sizeof(short))) == NULL)
	{
	    printf(" problems with memory allocation");
	    printf(" for `mot_pos[%d]' !  \n \n", i);
	    embExitBad();
	}




    edialign_struc_check( mot_regex ); 

    /*
       printf("  \n  regex_len = %d\n", regex_len) ; 
       printf("  mot_len = %d\n", mot_len) ; 
       printf("\n"); 
       
       for( p = 0 ; p < mot_len ; p++ ) {
       printf("  %d ", char_num[ p ] );
       }
       printf("\n\n");  
       */
 
    for( p = 0 ; p < mot_len ; p++ )
    {
	mot_char[p] = (char *) calloc(char_num[p], sizeof(char));
    }


    /* PROBLEM */ 


    for( p = 0 ; p < regex_len ; p++ )
    {
	if( mot_regex[ p ] == '[' )
	{ 
	    in_bracket = 1 ; 
	}

	if( mot_regex[ p ] == ']' )
	{  
	    in_bracket = 0 ; 
	    char_c = 0 ; 
	    mp++ ;
	}

	if( ( mot_regex[ p ] != '[' ) && ( mot_regex[ p ] != ']' ) )
	{ /* char */ 
	    if( in_bracket )
	    {
		mot_char[ mp ][ char_c ] = mot_regex[ p ] ; 
		char_c++;
	    }
	    else
	    {	       /* not in bracket */                   
		char_c = 0 ; 
		mot_char[ mp ][ 0 ] = mot_regex[ p ] ; 
		mp++ ; 
	    }
	}
    }

    /*
       for( mp = 0 ; mp < mot_len ; mp++ ) {
       printf("  position %d   ", mp + 1 ); 
       for( p = 0 ; p < char_num[ mp ] ; p++ ) {
       printf(" %c ", mot_char[ mp ][ p ]  ) ;
       }
       printf("\n"); 
       }
       */ 

    return;
}



 
/* @funcstatic edialign_seq_parse *****************************************
**
** edialign_seq_parse
**
** @param [u] mot_regex_unused [char*] Undocumented
** @return [void]
*****************************************************************************/

static void edialign_seq_parse(char *mot_regex_unused)
{ 
    ajuint sn;
    ajint ok; 
    ajint sp;
    ajint rp;
    ajint hv;
    ajint match = 0;

    (void) mot_regex_unused;		/* make it used */

    max_mot_offset = (float)
      (sqrt(-log(0.1) *  10 / mot_factor) * mot_offset_factor); 


    for(sn = 0 ; sn < seqnum ; sn++)
	for(sp = 0 ; sp < ( seqlen[ sn ] - mot_len + 1 ) ; sp++)
	{ 
	    ok = 1 ;
	    rp = 0 ;  
	    while( ok && ( rp < mot_len))
	    {
		if(mot_char[ rp ][ 0 ]  != 'X')
		{ 
		    match = 0 ;  
		    for(hv = 0 ; hv < char_num[ rp ] ; hv++)
		    { 
			if(mot_char[rp][hv] == seq[sn][sp + rp])
			{ 
			    match = 1 ; 
			}
		    }
		}
		ok = match;
		rp++;    
	    }

	    if( ok )
	    { 
		printf( " motif in seq %d at pos %d  \n",sn + 1 ,sp + 1);
		mot_pos[ sn ][ sp + 1 ] = 1 ;  
	    }
	    else 
		mot_pos[ sn ][ sp + 1 ] = 0 ;  
	}


    printf("\n") ; 

    /*
       for( sn = 0 ; sn < seqnum ; sn++ ) { 
       printf("     %s \n", seq[ sn ] ) ;
       printf("     "); 
       for( i = 1 ; i <= seqlen[ sn ] ; i++ ) { 
       
       
       if( mot_pos[ sn ][ i ]  ) 
       printf("*");
       else 
       printf(" ");
       }
       printf("\n\n" ) ; 
       }
       printf("\n" ) ; 
       */ 

    return;
}




#if 0
/* @funcstatic edialign_regex_format_complain *******************************
**
** edialign_regex_format_complain
**
** @return [void]
*****************************************************************************/

static void edialign_regex_format_complain(void)
{ 
    printf("\n \n   Arguments in command line don't make sense! \n");
    printf("   (Motifs not properly specified) \n \n");
    printf("   With the motif-search option, the program call is:\n\n");
    printf("      ./dialign2-2 [para] -mot <regex> <fct1> <fct2> ");
    printf("[para] <seq> \n\n");
    printf("   where \n      <regex>  is a regular expression,");
    printf(" e.g. \"AT[CG]XT\",\n");
    printf("      <fct1>    is a weighting factor \n");
    printf("      <fct2>    is a weighting factor \n");
    printf("      <seq>    is the input sequence file and \n");
    printf("      [para]   are (optional)");
    printf(" additional program parameters\n\n" );
    embExitBad();
}
#endif



 
/* @funcstatic edialign_mot_dist_factor *************************************
**
** edialign_mot_dist_factor
**
** @param [r] offset [ajint] Undocumented
** @param [r] parameter [float] Undocumented
** @return [float] Undocumented
*****************************************************************************/

static float edialign_mot_dist_factor(ajint offset , float parameter)
{
    float mdf , parameter2;
    ajint offset2 ; 

    offset2 = offset * offset ;
    parameter2 = parameter * parameter ;

    /* factor1 = (float) offset2 / (parameter2 * 10); */
    mdf = (float) (exp(-(offset2) / (parameter2 * 10))); 

    return mdf ;
}




/* @funcstatic edialign_rel_wgt_calc *************************************
**
** edialign_rel_wgt_calc
**
** @param [r] l1 [ajint] Undocumented
** @param [r] l2 [ajint] Undocumented
** @param [u] rel_wgt [float**] Undocumented
** @return [void]
*****************************************************************************/

static void edialign_rel_wgt_calc(ajint l1, ajint l2, float **rel_wgt)
{
    ajint l;
    ajint m;
    ajint mss = 0;
    float ent;
    float factor;
    float l1f;
    float l2f;
    float av_sim_score = 0.;
    double t_pr;
    double pr400;
    double **tpr = NULL;

    /*
       printf(" it %d, rel_wgt_calc: len = %d , %d \n", istep , l1 , l2 );
       */

    if( rel_wgt == wgt_prot )
    {
	tpr = tp400_prot ;
	mss = max_sim_score ;
	av_sim_score = av_sim_score_pep ; 
    } 
    
    if( rel_wgt == wgt_dna )
    {
	tpr = tp400_dna ;
	mss = 1 ;
	av_sim_score = av_sim_score_nuc ; 
    }
      
    if( rel_wgt == wgt_trans )
    {
	tpr = tp400_trans ;
	mss = max_sim_score ;
	av_sim_score = av_sim_score_pep ; 
    }
      
  

    l1f = (float) l1;
    l2f = (float) l2;

    factor = ( l1f * l2f ) / (float) 400.00;


    for( l = 1 ; l <= max_dia; l++ )
	for( m = 0 ; m <= l * mss ; m++ )
	{
	    rel_wgt[l][m] = 0;


	    if( tpr[l][m] )
		if( m > av_sim_score * l )

		{
		    pr400 = tpr[l][m];

		    if( pr400 > 0.0000000001 )
			t_pr = 1 - pow( 1 - pr400 , factor );
		    else
			t_pr = pr400 * factor;

		    ent = 0;

		    if(t_pr)
		      ent = (float) -log( t_pr );

		    if( ent > threshold )
			rel_wgt[l][m] = ent;
		}
	} 

    return;
}




/* @funcstatic edialign_wgt_prnt_prot *************************************
**
** edialign_wgt_prnt_prot
**
** @return [void]
*****************************************************************************/

static void edialign_wgt_prnt_prot(void)
{
    ajint  i;
    ajint j; 

    printf(" \n\n  weight scores for PROTEIN fragments\n\n" ); 
    printf("  sequence lengths = %d , %d \n\n", seqlen[0] , seqlen[1] ) ;  
    for( i = 1 ; i <= max_dia ; i++ ) {
	for( j = 0 ; j <= ( i * 15 ) ; j++ ) 
	    printf(" %3d %3d %f  \n", i , j , wgt_prot[ i ][ j ] );
    }

    return;
}




/* @funcstatic edialign_wgt_prnt_dna *************************************
**
** edialign_wgt_prnt_dna
**
** @return [void]
*****************************************************************************/

static void edialign_wgt_prnt_dna(void)
{
    ajint i;
    ajint j;
 
    printf(" \n\n  weight scores for NON-TRANSLATED DNA fragments\n\n" ); 
    printf("  sequence lengths = %d , %d \n\n", seqlen[0] , seqlen[1] ) ;  
    for( i = 1 ; i <= max_dia ; i++ )
    {
	for( j = 0 ; j <= i ; j++ ) 
	    printf(" %3d %3d %f  \n", i , j , wgt_dna[ i ][ j ] );
    }

    return;
}




/* @funcstatic edialign_wgt_prnt_trans *************************************
**
** edialign_wgt_prnt_trans
**
** @return [void]
*****************************************************************************/

static void edialign_wgt_prnt_trans(void)
{
    ajint i;
    ajint j; 

    printf(" \n\n  weight scores for TRANSLATED DNA fragments\n\n" ); 
    printf("  sequence lengths = %d , %d \n\n", seqlen[0] , seqlen[1] ) ;  
    for( i = 1 ; i <= max_dia ; i++ ) {
	for( j = 0 ; j <= ( i * 15 ) ; j++ ) 
	    printf(" %3d %3d %f  \n", i , j , wgt_trans[ i ][ j ] );
    }

    return;
}




/* @funcstatic edialign_wgt_prnt *************************************
**
** edialign_wgt_prnt
**
** @return [void]
*****************************************************************************/

static void edialign_wgt_prnt(void)
{
    if (wgt_type == 0 )  
	edialign_wgt_prnt_prot( );

    if (wgt_type % 2 )  
	edialign_wgt_prnt_dna( );

    if (wgt_type > 1 )  
	edialign_wgt_prnt_trans( );

    return;
}



/* @funcstatic edialign_mem_alloc *************************************
**
** edialign_mem_alloc
**
** @return [void]
*****************************************************************************/

static void edialign_mem_alloc(void)
{
    /* allocates memory for `tp400_xxx', `wgt_xxx' */ 

    ajint i;
 
    if( wgt_type == 0 )
    {
	if( (tp400_prot = (double **) calloc((max_dia + 1),sizeof(double*))) 
	   == NULL)
	{ 
	    printf(" problems with memory allocation for `tp400_prot' !  "
		   "\n \n");
	    embExitBad();
	}

	if( ( wgt_prot = (float **) calloc( (max_dia+1) , sizeof(float*) ))
	   == NULL)
	{
	    printf(" problems with memory allocation for `weights' !  \n \n");
	    embExitBad();
	}
    }

    if( wgt_type % 2 )
    { 
	if( (tp400_dna = (double **) calloc( ( max_dia + 1 ) ,
					    sizeof(double*)))
	   == NULL)
	{
	    printf(" problems with memory allocation for `tp400_dna' !  "
		   "\n \n");
	    embExitBad();
	}

	if( ( wgt_dna = (float **) calloc( (max_dia+1) , sizeof(float*) ))
	   == NULL)
	{
	    printf(" problems with memory allocation for `weights' !  \n \n");
	    embExitBad();
	}
    }

 
    if( wgt_type > 1 )
    {
	if( (tp400_trans = (double **) calloc( ( max_dia + 1 ) ,
					      sizeof(double*) ))
	   == NULL)
	{
	    printf(" problems with memory allocation for `tp400_trans' !  "
		   "\n \n");
	    embExitBad();
	}
 
	if( ( wgt_trans = (float **) calloc( (max_dia+1) , sizeof(float*) ))
	   == NULL)
	{
	    printf(" problems with memory allocation for `weights' !  \n \n");
	    embExitBad();
	}
    }
  
    for( i = 1 ; i <= max_dia ; i++ )
    {
   
 
	if( wgt_type == 0 )
	{
	    if( (tp400_prot[i] = 
		 (double *) calloc(((i + 1) * max_sim_score),sizeof(double))) 
	       == NULL)
	    { 
		printf(" problems with memory allocation for `tp400_prot' !  "
		       "\n \n");
		embExitBad();
	    }

	    if( (wgt_prot[i] =
		 (float *) calloc( ((i+1) * max_sim_score ) , sizeof(float) ))
	       == NULL)
	    {
		printf(" problems with memory allocation for `weights'!\n\n");
		embExitBad();
	    }
	}
  

	if( wgt_type % 2 )
	{
	    if( (tp400_dna[i] =
		 (double *) calloc( ((i + 1) ) , sizeof(double) ))
	       == NULL)
	    {
		printf(" problems with memory allocation for `tp400_dna' !"
		       "\n \n");
		embExitBad();
	    }

	    if( (wgt_dna[i] =
		 (float *) calloc( ((i+1) ) , sizeof(float) ))
	       == NULL)
	    {
		printf(" problems with memory allocation for `weights'!\n\n");
		embExitBad();
	    }
	}


	if( wgt_type > 1 )
	{
	    if( (tp400_trans[i] =
		 (double *) calloc(((i + 1) * max_sim_score),sizeof(double)))
	       == NULL)
	    {
		printf(" problems with memory allocation for `tp400_trans' "
		       "%d !  \n \n", i);
		embExitBad();
	    }

	    if( (wgt_trans[i] =
		 (float *) calloc( ((i+1) * max_sim_score ) , sizeof(float) ))
	       == NULL)
	    {
		printf(" problems with memory allocation for `weights'!\n\n");
		embExitBad();
	    }
	}
    }     

    return;
}
