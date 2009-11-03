#ifdef __cplusplus
extern "C"
{
#endif

#ifndef embshow_h
#define embshow_h


/********* Descriptor object types *****************/

enum ShowEValtype
{
    SH_SEQ,
    SH_BLANK,
    SH_TICK,
    SH_TICKNUM,
    SH_COMP,
    SH_TRAN,
    SH_RE,
    SH_FT,
    SH_NOTE
};




/* @data EmbPShow *************************************************************
**
** NUCLEUS data structure for EmbPShow object for a sequence
**
** @attr list [AjPList] list of EmbPShowInfo structures
**
** @cc information about the sequence
**
** @attr seq [const AjPSeq] the sequence
** @attr nucleic [AjBool] ajTrue = the sequence is nucleic
** @attr offset [ajuint] offset to start numbering at
** @attr start [ajuint] sequence position to start printing at
** @attr end [ajuint] sequence position to stop printing at
**
** @cc information about the page layout
**
** @attr width [ajuint] width of sequence to display on each line
** @attr length [ajuint] length of a page (0 = indefinite)
** @attr margin [ajuint] margin for numbers
** @attr html [AjBool] ajTrue = format page for HTML
** @@
******************************************************************************/

typedef struct EmbSShow {
  AjPList list;
  const AjPSeq seq;
  AjBool nucleic;
  ajuint offset;
  ajuint start;
  ajuint end;
  ajuint width;
  ajuint length;
  ajuint margin;
  AjBool html;
} EmbOShow;
#define EmbPShow EmbOShow*




/* @data EmbPShowInfo *********************************************************
**
**
** The sequence and associated things to show are held in an ordered list
** of type EmbPShowInfo. This list is held in the structure EmbPShow.
**
** The things to show are displayed around the sequence in the order that
** they are held on the list.
**
** EmbPShowInfo holds the descriptor (one of EmbPShowBlank, EmbPShowTicks,
** EmbPShowSeq, EmbPShowComp, etc.) and the type of the descriptor (one of
** SH_BLANK, SH_TICKS, SH_SEQ, SH_COMP, etc.  )
**
** Each descriptor (EmbPShowSeq, EmbPShowBlank, EmbPShowTicks, etc.) holds
** information that could be useful in displaying its type of information.
**
** So, for example:
**
** EmbPShow could have a list of:
** ----------------------------
**
** EmbPShowInfo->type=SH_BLANK
**    |       ->info=EmbPShowBlank
**    |
** EmbPShowInfo->type=SH_TICKS
**    |       ->info=EmbPShowTicks
**    |
** EmbPShowInfo->type=SH_SEQ
**    |       ->info=EmbPShowSeq
**    |
** EmbPShowInfo->type=SH_COMP
**    |       ->info=EmbPShowComp
**    |
** EmbPShowInfo->type=etc.
**    |       ->info=etc.
**    |
**   etc.
**
** @attr info [void*] Information descriptor (set of available descriptors)
** @attr type [ajint] Type of information (enumerated list)
** @attr Padding [char[4]] Padding to alignment boundary
** @@
******************************************************************************/

typedef struct EmbSShowInfo {
  void * info;
  ajint type;
  char Padding[4];
} EmbOShowInfo;
#define EmbPShowInfo EmbOShowInfo*




/* @data EmbPShowSeq **********************************************************
**
** NUCLEUS data structure for sequence information, type = SH_SEQ
**
** @attr number [AjBool] ajTrue = number the sequence
** @attr threeletter [AjBool] ajTrue = display proteins in three letter code
** @attr upperrange [const AjPRange] range of sequence to uppercase
** @attr highlight [const AjPRange] range of sequence to colour in HTML
** @@
******************************************************************************/

typedef struct EmbSShowSeq {
  AjBool number;
  AjBool threeletter;
  const AjPRange upperrange;
  const AjPRange highlight;
} EmbOShowSeq;
#define EmbPShowSeq EmbOShowSeq*

/* blank line information, type = SH_BLANK */





/* @data EmbPShowBlank ********************************************************
**
** NUCLEUS data structure for  blank line information, type = SH_BLANK
**
** @attr dummy [AjBool] Dummy attribute - no specific information needed
**                      AJNEW0() falls over if 0 bytes are allocated, so
**                      put in this dummy to pad the structure out
** @@
******************************************************************************/

typedef struct EmbSShowBlank {
  AjBool dummy;
} EmbOShowBlank;
#define EmbPShowBlank EmbOShowBlank*




/* @data EmbPShowTicks ********************************************************
**
** NUCLEUS data structure for tick line information, type = SH_TICK
**
** @attr dummy [AjBool] Dummy attribute - no specific information needed
**                      AJNEW0() falls over if 0 bytes are allocated, so
**                      put in this dummy to pad the structure out
** @@
******************************************************************************/

typedef struct EmbSShowTicks {
  AjBool dummy;
} EmbOShowTicks;
#define EmbPShowTicks EmbOShowTicks*




/* @data EmbPShowTicknum ******************************************************
**
** NUCLEUS data structure for tick number line information, type = SH_TICKNUM
**
** @attr dummy [AjBool] Dummy attribute - no specific information needed
**                      AJNEW0() falls over if 0 bytes are allocated, so
**                      put in this dummy to pad the structure out
** @@
******************************************************************************/

typedef struct EmbSShowTicknum {
  AjBool dummy;
} EmbOShowTicknum;
#define EmbPShowTicknum EmbOShowTicknum*




/* @data EmbPShowTran *********************************************************
**
** NUCLEUS data structure for translation information, type = SH_TRAN
**
** @attr transeq [AjPSeq] Copy of our stored translation
** @attr trnTable [const AjPTrn] translation table
** @attr frame [ajint] 1,2,3,-1,-2 or -3 = frame to translate
** @attr threeletter [AjBool] ajTrue = display in three letter code
** @attr number [AjBool] ajTrue = number the translation
** @attr tranpos [ajuint] store of translation position for numbering
** @attr regions [const AjPRange] only translate in these regions,
**                                NULL = do all
** @attr orfminsize [ajuint] minimum size of ORF to display
** @attr lcinterorf [AjBool] ajTrue = put the inter-orf regions in lower case
** @attr firstorf [AjBool] ajTrue = beginning of the seq is a possible ORF
** @attr lastorf [AjBool] ajTrue = end of the seq is a possible ORF
** @attr showframe [AjBool] ajTrue = write the frame number
** @attr Padding [char[4]] Padding to alignment boundary
** @@
******************************************************************************/

typedef struct EmbSShowTran {
  AjPSeq transeq;
  const AjPTrn trnTable;
  ajint frame;
  AjBool threeletter;
  AjBool number;
  ajuint tranpos;
  const AjPRange regions;
  ajuint orfminsize;
  AjBool lcinterorf;
  AjBool firstorf;
  AjBool lastorf;
  AjBool showframe;
  char   Padding[4];
} EmbOShowTran;
#define EmbPShowTran EmbOShowTran*




/* @data EmbPShowComp *********************************************************
**
** NUCLEUS data structure for sequence complement information, type = SH_COMP
**
** @attr number [AjBool] ajTrue = number the complement
** @@
******************************************************************************/

typedef struct EmbSShowComp {
  AjBool number;
} EmbOShowComp;
#define EmbPShowComp EmbOShowComp*




/* @data EmbPShowRE ***********************************************************
**
** NUCLEUS data structure for RE cut site information, type = SH_RE
**
** @attr sense [ajint]  1 or -1 = sense to display
** @attr flat [AjBool] ajTrue = display in flat format with recognition sites
** @attr matches [AjPList] list of AjPMatmatch matches
** @attr plasmid [AjBool] ajTrue = Circular (plasmid) sequence. Needed so
**                        that when we display sequences we can decide whether
**                        to show cuts that go past the origin in either
**                        direction
** @attr hits [ajuint]  number of hits in list
** @attr sitelist [AjPList] list of EmbSShowREsite
** @@
******************************************************************************/

typedef struct EmbSShowRE {
  ajint sense;
  AjBool flat;
  AjPList matches;
  AjBool plasmid;
  ajuint hits;
  AjPList sitelist;
} EmbOShowRE;
#define EmbPShowRE EmbOShowRE*




/* @data EmbPShowFT ***********************************************************
**
** NUCLEUS data structure for  Feature information, type = SH_FT
**
** @attr feat [AjPFeattable] Feature table
** @@
******************************************************************************/

typedef struct EmbSShowFT {
  AjPFeattable feat;
} EmbOShowFT;
#define EmbPShowFT EmbOShowFT*




/* @data EmbPShowNote *********************************************************
**
** NUCLEUS data structure for annotation information, type = SH_NOTE
**
** @attr regions [const AjPRange] regions to annotate, NULL = no regions
** @@
******************************************************************************/

typedef struct EmbSShowNote {
  const AjPRange regions;
} EmbOShowNote;
#define EmbPShowNote EmbOShowNote*




/********* assorted structures ***********/

/* @data EmbPShowREsite *******************************************************
**
** NUCLEUS data structure for Restriction Enzyme cut site position list node
**
** @attr name [AjPStr] name of Restriction Enzyme
** @attr pos [ajint] cut site position
** @attr Padding [char[4]] Padding to alignment boundary
** @@
******************************************************************************/

typedef struct EmbSShowREsite {
  AjPStr name;
  ajint pos;
  char Padding[4];
} EmbOShowREsite;
#define EmbPShowREsite EmbOShowREsite*




/*
** Prototype definitions
*/

EmbPShow embShowNew (const AjPSeq seq, ajint begin, ajint end, ajint width,
		     ajint length, ajint margin, AjBool html, ajint offset);
void     embShowDel (EmbPShow* pthis);

void     embShowAddSeq (EmbPShow thys, AjBool number, AjBool threeletter,
			const AjPRange upperrange, const AjPRange colour);
void     embShowAddBlank (EmbPShow thys);
void     embShowAddTicks (EmbPShow thys);
void     embShowAddTicknum (EmbPShow thys);
void     embShowAddComp (EmbPShow thys, AjBool number);
void     embShowAddTran (EmbPShow thys, const AjPTrn trnTable, ajint frame,
			 AjBool threeletter, AjBool number,
			 const AjPRange regions,
			 ajint orfminsize, AjBool lcinterorf,
			 AjBool firstorf, AjBool lastorf, AjBool showframe);
void     embShowAddRE (EmbPShow thys, ajint sense, const AjPList restrictlist,
		       AjBool plasmid, AjBool flat);
void     embShowAddFT (EmbPShow thys, const AjPFeattable feat);
void     embShowAddNote (EmbPShow thys, const AjPRange regions);
void     embShowPrint (AjPFile out, const EmbPShow thys);
void     embShowUpperRange (AjPStr *line,
			    const AjPRange upperrange, ajuint pos);
void     embShowColourRange (AjPStr *line,
			     const AjPRange colour, ajuint pos);

/*
** End of prototype definitions
*/

#endif

#ifdef __cplusplus
}
#endif


