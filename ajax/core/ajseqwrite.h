#ifdef __cplusplus
extern "C"
{
#endif

#ifndef ajseqwrite_h
#define ajseqwrite_h




/* @data AjPSeqout ************************************************************
**
** Ajax Sequence Output object.
**
** Holds definition of sequence output.
**
** @alias AjSSeqout
** @alias AjOSeqout
**
** @attr Name [AjPStr] Name (ID)
** @attr Acc [AjPStr] Accession number (primary only)
** @attr Sv [AjPStr] SeqVersion number
** @attr Gi [AjPStr] GI NCBI version number
** @attr Desc [AjPStr] One-line description
** @attr Tax [AjPStr] Main taxonomy (species)
** @attr Taxid [AjPStr] Main taxonomy (species) id in NCBI taxonomy
** @attr Organelle [AjPStr] Organelle taxonomy
** @attr Type [AjPStr] Type N or P
** @attr Outputtype [AjPStr] Output sequence known type
** @attr Molecule [AjPStr] Molecule type
** @attr Class [AjPStr] Class of entry
** @attr Division [AjPStr] Database division
** @attr Evidence [AjPStr] Experimental evidence (e.g. from UniProt)
** @attr Db [AjPStr] Database name from input name
** @attr Setdb [AjPStr] Database name from input command line
** @attr Setoutdb [AjPStr] Database name from command line
** @attr Full [AjPStr] Full name
** @attr Date [AjPSeqDate] Dates
** @attr Fulldesc [AjPSeqDesc] Dates
** @attr Doc [AjPStr] Obsolete - see TextPtr
** @attr Usa [AjPStr] USA for re-reading
** @attr Ufo [AjPStr] UFO for re-reading
** @attr Fttable [AjPFeattable] Feature table
** @attr Ftquery [AjPFeattabOut] Feature table output
** @attr FtFormat [AjPStr] Feature output format (if not in UFO)
** @attr FtFilename [AjPStr] Feature output filename (if not in UFO)
** @attr Informatstr [AjPStr] Input format
** @attr Formatstr [AjPStr] Output format
** @attr EType [AjEnum] unused, obsolete
** @attr Format [AjEnum] Output format index
** @attr Filename [AjPStr] Output filename (if not in USA)
** @attr Directory [AjPStr] Output firectory
** @attr Entryname [AjPStr] Entry name
** @attr Acclist [AjPList] Secondary accessions
** @attr Keylist [AjPList] Keyword list
** @attr Taxlist [AjPList] Taxonomy list
** @attr Genelist [AjPList] Gene list
** @attr Reflist [AjPList] References (citations)
** @attr Cmtlist [AjPList] Comment block list
** @attr Xreflist [AjPList] Database cross reference list
** @attr Seq [AjPStr] The sequence
** @attr File [AjPFile] Output file
** @attr Knownfile [AjPFile] Already open output file (we don't close this one)
** @attr Extension [AjPStr] File extension
** @attr Savelist [AjPList] Previous sequences saved for later output
**                          (e.g. MSF format)
** @attr Accuracy [float*] Accuracy values (one per base) from base calling
** @attr Cleanup [(void*)] Function to write remaining lines on closing
** @attr Rev [AjBool] true: to be reverse-complemented
** @attr Circular [AjBool] true: circular nucleotide molecule
** @attr Single [AjBool] If true, single sequence in each file (-ossingle)
** @attr Features [AjBool] If true, save features with sequence or in file
** @attr Qualsize [ajuint] Size of Accuracy array
** @attr Count [ajint] Number of sequences
** @attr Offset [ajint] offset from start
**
** @new ajSeqoutNew Default constructor
** @delete ajSeqoutDel Default destructor
** @modify ajSeqoutUsa Resets using a new USA
** @modify ajSeqoutClear Resets ready for reuse.
** @modify ajSeqoutOpen If the file is not yet open, calls seqoutUsaProcess
** @cast ajSeqoutCheckGcg Calculates the GCG checksum for a sequence set.
** @modify ajSeqWrite Master sequence output routine
** @modify ajSeqsetWrite Master sequence set output routine
** @modify ajSeqFileNewOut Opens an output file for sequence writing.
** @other AjPSeq Sequences
** @attr Padding [char[4]] Padding to alignment boundary
** @@
******************************************************************************/

typedef struct AjSSeqout {
  AjPStr Name;
  AjPStr Acc;
  AjPStr Sv;
  AjPStr Gi;
  AjPStr Desc;
  AjPStr Tax;
  AjPStr Taxid;
  AjPStr Organelle;
  AjPStr Type;
  AjPStr Outputtype;
  AjPStr Molecule;
  AjPStr Class;
  AjPStr Division;
  AjPStr Evidence;
  AjPStr Db;
  AjPStr Setdb;
  AjPStr Setoutdb;
  AjPStr Full;
  AjPSeqDate Date;
  AjPSeqDesc Fulldesc;
  AjPStr Doc;
  AjPStr Usa;
  AjPStr Ufo;
  AjPFeattable Fttable;
  AjPFeattabOut Ftquery;
  AjPStr FtFormat;
  AjPStr FtFilename;
  AjPStr Informatstr;
  AjPStr Formatstr;
  AjEnum EType;
  AjEnum Format;
  AjPStr Filename;
  AjPStr Directory;
  AjPStr Entryname;
  AjPList Acclist;
  AjPList Keylist;
  AjPList Taxlist;
  AjPList Genelist;
  AjPList Reflist;
  AjPList Cmtlist;
  AjPList Xreflist;
  AjPStr Seq;
  AjPFile File;
  AjPFile Knownfile;
  AjPStr Extension;
  AjPList Savelist;
  float* Accuracy;
  void (*Cleanup) (AjPFile filethys);
  AjBool Rev;
  AjBool Circular;
  AjBool Single;
  AjBool Features;
  ajuint Qualsize;
  ajint Count;
  ajint Offset;
  char Padding[4];
} AjOSeqout;

#define AjPSeqout AjOSeqout*




/*
** Prototype definitions
*/

AjBool       ajSeqoutWriteSeq (AjPSeqout outseq, const AjPSeq seq);
AjBool       ajSeqoutOpenFilename (AjPSeqout seqout, const AjPStr name);
ajint        ajSeqoutGetCheckgcg (const AjPSeqout outseq);
const AjPStr ajSeqoutGetFilename(const AjPSeqout thys);
void         ajSeqoutClear (AjPSeqout thys);
void         ajSeqoutGetBasecount(const AjPSeqout seqout, ajuint* bases);
AjBool       ajSeqoutSetNameDefaultC(AjPSeqout thys,
				     AjBool multi, const char* txt);
AjBool       ajSeqoutSetNameDefaultS(AjPSeqout thys,
				     AjBool multi, const AjPStr str);
void         ajSeqoutDel (AjPSeqout* thys);
AjBool       ajSeqoutstrGetFormatDefault (AjPStr* Pformat);
AjBool       ajSeqoutstrIsFormatExists(const AjPStr format);
AjBool       ajSeqoutstrIsFormatSingle (const AjPStr format);
AjPSeqout    ajSeqoutNew (void);
AjPSeqout    ajSeqoutNewFile (AjPFile file);
AjPSeqout    ajSeqoutNewFormatC(const char* txt);
AjPSeqout    ajSeqoutNewFormatS(const AjPStr str);
AjBool       ajSeqoutOpen (AjPSeqout thys);
AjBool       ajSeqoutSetFormatC (AjPSeqout thys, const char* format);
AjBool       ajSeqoutSetFormatS (AjPSeqout thys, const AjPStr format);
void         ajSeqoutTrace (const AjPSeqout seq);
void         ajSeqoutPrintFormat (AjPFile outf, AjBool full);
void         ajSeqoutPrintbookFormat (AjPFile outf);
void         ajSeqoutPrinthtmlFormat (AjPFile outf);
void         ajSeqoutPrintwikiFormat (AjPFile outf);
void         ajSeqoutClearUsa (AjPSeqout thys, const AjPStr Usa);
AjBool       ajSeqoutWriteSet (AjPSeqout seqout, const AjPSeqset seq);
void         ajSeqoutClose(AjPSeqout outseq);
void         ajSeqoutFlush(AjPSeqout seqout);
void         ajSeqoutExit(void);
void         ajSeqoutReset(AjPSeqout seqout);

void         ajSeqoutDumpSwisslike(AjPSeqout outseq,const AjPStr seq,
				   const char *prefix);

/*
** End of prototype definitions
*/

__deprecated void         ajSeqoutCount(const AjPSeqout seqout, ajuint* b);
__deprecated void         ajSeqAllWrite (AjPSeqout outseq, const AjPSeq seq);
__deprecated AjBool       ajSeqFileNewOut (AjPSeqout seqout,
					   const AjPStr name);
__deprecated AjPSeqout    ajSeqoutNewF (AjPFile file);
__deprecated void         ajSeqWrite (AjPSeqout seqout, const AjPSeq seq);
__deprecated void         ajSeqsetWrite(AjPSeqout outseq, const AjPSeqset seq);
__deprecated void         ajSeqWriteClose (AjPSeqout outseq);
__deprecated AjBool       ajSeqOutFormatSingle (AjPStr format);
__deprecated AjBool       ajSeqOutSetFormat (AjPSeqout thys,
					     const AjPStr format);
__deprecated AjBool       ajSeqOutSetFormatC (AjPSeqout thys,
					      const char* format);
__deprecated AjBool       ajSeqOutFormatDefault (AjPStr* pformat);

__deprecated void         ajSeqoutUsa (AjPSeqout* pthis, const AjPStr Usa);

__deprecated void         ajSeqPrintOutFormat (AjPFile outf, AjBool full);
__deprecated AjBool       ajSeqFindOutFormat(const AjPStr format,
					     ajint* iformat);
__deprecated void         ajSeqWriteExit(void);
__deprecated ajint        ajSeqoutCheckGcg (const AjPSeqout outseq);

__deprecated void         ajSeqoutDefName(AjPSeqout thys,
					  const AjPStr setname, AjBool multi);
__deprecated void         ajSeqWriteXyz(AjPFile outf,
					const AjPStr seq, const char *prefix);
__deprecated void         ajSssWriteXyz(AjPFile outf,
					const AjPStr seq, const char *prefix);
#endif

#ifdef __cplusplus
}
#endif
