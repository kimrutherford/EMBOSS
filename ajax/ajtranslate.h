#ifdef __cplusplus
extern "C"
{
#endif

#ifndef ajtranslate_h
#define ajtranslate_h

#include "ajax.h"




/* @data AjPTrn ***************************************************************
**
** Ajax Sequence translation object.
**
** Holds the Genetic Code specification and information needed to translate
** the sequence and find initiation sites.
**
** @new ajTrnNew Default constructor
** @new ajTrnNewI Default constructor
** @new ajTrnNewC Default constructor
** @delete ajTrnDel Default destructor
** @input ajTrnReadFile Reads a Genetic Code file
** @use ajTrnCodon Translating a codon from an AjPStr
** @use ajTrnRevCodon Reverse complement translating a codon from an AjPStr
** @use ajTrnCodonC Translating a codon from a char* text
** @use ajTrnRevCodonC Translating a codon from a char* text
** @use ajTrnCodonK Translating a codon from a char* to a char
** @use ajTrnRevCodonK Reverse complement translating a codon
**                     from a char* to a char
** @use ajTrnC Translating a sequence from a char* text
** @use ajTrnRevC Reverse complement translating a sequence from a char* text
** @use ajTrnAltRevC (Alt) Reverse complement translating a sequence
**                         from a char* text
** @use ajTrnStr Translating a sequence from a AjPStr
** @use ajTrnRevStr Reverse complement translating a sequence from a AjPStr
** @use ajTrnAltRevStr (Alt) Reverse complement translating a sequence
**                           from a AjPStr
** @use ajTrnSeq Translating a sequence from a AjPSeq
** @use ajTrnRevSeq Reverse complement translating a sequence from a AjPSeq
** @use ajTrnAltRevSeq Reverse complement translating a sequence from a AjPSeq
** @use ajTrnCFrame Translating a sequence from a char* in a frame
** @use ajTrnStrFrame Translating a sequence from a AjPStr in a frame
** @use ajTrnSeqFrame Translating a sequence from a AjPSeq in a frame
** @use ajTrnSeqFramePep Translating a sequence from a AjPSeq in a frame
**                       and returns a new peptide
** @use ajTrnCDangle Translates the last 1 or two bases of a sequence
**                   in a char* text
** @use ajTrnStrDangle Translates the last 1 or two bases of a sequence
**                     in a AjStr
** @use ajTrnSeqOrig Translating a sequence
** @cast ajTrnGetTitle Returns description of the translation table
** @cast ajTrnGetFileName Returns file name the translation table was read from
** @use ajTrnStartStop Checks whether the input codon is a Start codon,
**                     a Stop codon or something else
** @use ajTrnStartStopC Checks whether a const char* codon is
**                      a Start codon, a Stop codon or something else
**
** @attr FileName [AjPStr] name of file that held the data
** @attr Title [AjPStr] title of data read from file
** @attr GC [char[16][16][16]] genetic codon table
** @attr Starts [char[16][16][16]] initiation site table
** @@
******************************************************************************/

typedef struct AjSTrn {
  AjPStr FileName;
  AjPStr Title;
  char GC[16][16][16];
  char Starts[16][16][16];
} AjOTrn;
#define AjPTrn AjOTrn*




/* table to convert character of base to translation array element value */
/*static ajint trnconv[] = {*/
/* characters less than 64 */
/*  14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14,
  14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14,
  14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14,
  14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, */

/*   @  A   B  C   D   E   F  G   H   I   J  K   L  M   N   O */
/*  14, 0, 13, 1, 12, 14, 14, 2, 11, 14, 14, 9, 14, 4, 14, 14,*/

/*   P   Q  R  S  T  U   V  W   X  Y   Z   [   \   ]   ^   _  */
/*  14, 14, 5, 7, 3, 3, 10, 6, 14, 8, 14, 14, 14, 14, 14, 14, */

/*   `  a   b  c   d   e   f  g   h   i   j  k   l  m   n   o */
/*  14, 0, 13, 1, 12, 14, 14, 2, 11, 14, 14, 9, 14, 4, 14, 14,*/

/*   p   q  r  s  t  u   v  w   x  y   z   {   |   }   ~ del  */
/*  14, 14, 5, 7, 3, 3, 10, 6, 14, 8, 14, 14, 14, 14, 14, 14
};*/





/*
** Prototype definitions
*/

void          ajTrnDel (AjPTrn* pthis);
void          ajTrnExit(void);
AjPTrn        ajTrnNew (const AjPStr trnFileName);
AjPTrn        ajTrnNewI (ajint trnFileNameInt);
AjPTrn        ajTrnNewC (const char *trnFileName);
void          ajTrnReadFile (AjPTrn trnObj, AjPFile trnFile);
AjPSeq        ajTrnNewPep(const AjPSeq nucleicSeq, ajint frame);
const  AjPStr ajTrnCodon (const AjPTrn trnObj, const AjPStr codon);
const  AjPStr ajTrnRevCodon (const AjPTrn trnObj, const AjPStr codon);
const  AjPStr ajTrnCodonC (const AjPTrn trnObj, const char *codon);
const  AjPStr ajTrnRevCodonC (const AjPTrn trnObj, const char *codon);
char          ajTrnCodonK (const AjPTrn trnObj, const char *codon);
char          ajTrnRevCodonK (const AjPTrn trnObj, const char *codon);
void          ajTrnC (const AjPTrn trnObj, const char *str, ajint len,
		      AjPStr *pep);
void          ajTrnRevC (const AjPTrn trnObj, const char *str, ajint len,
			 AjPStr *pep);
void          ajTrnAltRevC (const AjPTrn trnObj, const char *str, ajint len,
			    AjPStr *pep);
void          ajTrnStr (const AjPTrn trnObj, const AjPStr str,
			AjPStr *pep);
void          ajTrnRevStr (const AjPTrn trnObj, const AjPStr str,
			   AjPStr *pep);
void          ajTrnAltRevStr (const AjPTrn trnObj, const AjPStr str,
			      AjPStr *pep);
void          ajTrnSeq (const AjPTrn trnObj, const AjPSeq seq,
			AjPStr *pep);
void          ajTrnRevSeq (const AjPTrn trnObj, const AjPSeq seq,
			   AjPStr *pep);
void          ajTrnAltRevSeq (const AjPTrn trnObj, const AjPSeq seq,
			      AjPStr *pep);
void          ajTrnCFrame (const AjPTrn trnObj, const char *seq, ajint len,
			   ajint frame, AjPStr *pep);
const AjPStr  ajTrnName(ajint trnFileNameInt);
void          ajTrnStrFrame (const AjPTrn trnObj, const AjPStr seq,
			     ajint frame, AjPStr *pep);
void          ajTrnSeqFrame (const AjPTrn trnObj, const AjPSeq seq,
			     ajint frame, AjPStr *pep);
AjPSeq        ajTrnSeqFramePep (const AjPTrn trnObj, const AjPSeq seq,
				ajint frame);
ajint         ajTrnCDangle (const AjPTrn trnObj, const char *seq, ajint len,
			    ajint frame, AjPStr *pep);
ajint         ajTrnStrDangle (const AjPTrn trnObj, const AjPStr seq,
			      ajint frame, AjPStr *pep);
AjPStr        ajTrnGetTitle (const AjPTrn thys);
AjPStr        ajTrnGetFileName (const AjPTrn thys);
ajint         ajTrnStartStop (const AjPTrn trnObj,
			      const AjPStr codon, char *aa);
ajint         ajTrnStartStopC (const AjPTrn trnObj,
			       const char *codon, char *aa);
AjPSeq        ajTrnSeqOrig (const AjPTrn trnObj, const AjPSeq seq,
			    ajint frame);

/*
** End of prototype definitions
*/

#endif

#ifdef __cplusplus
}
#endif
