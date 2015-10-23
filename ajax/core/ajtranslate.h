/* @include ajtranslate *******************************************************
**
** AJAX translate functions
**
** These functions control all aspects of sequence translation
**
** These functions do not translate to the 'ambiguity' residues
** 'B' (Asn or Asp) and 'Z' (Glu or Gln). So the codons:
** RAC, RAT, RAY, RAU which could code for 'B' return 'X'
** and SAA, SAG, SAR which could code for 'Z' return 'X'.
**
** This translation table doesn't have the doubly ambiguous
** codons set up:
** YTR - L
** MGR - R
** YUR - L
**
** This should be attended to at some time.
**
** @author Copyright (C) 1999 Gary Williams
** @version $Revision: 1.24 $
** @modified Feb 15 1999 GWW First version
** @modified April 19 1999 GWW Second version using NCBI's GC tables
** @modified April 18 2000 GWW Reorganised many of the routines
** @modified 2001-2011 Peter Rice
** @modified $Date: 2013/02/07 10:32:50 $ by $Author: rice $
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

#ifndef AJTRANSLATE_H
#define AJTRANSLATE_H

/* ========================================================================= */
/* ============================= include files ============================= */
/* ========================================================================= */

#include "ajdefine.h"
#include "ajstr.h"
#include "ajfile.h"
#include "ajseq.h"

AJ_BEGIN_DECLS




/* ========================================================================= */
/* =============================== constants =============================== */
/* ========================================================================= */




/* ========================================================================= */
/* ============================== public data ============================== */
/* ========================================================================= */




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
** @use ajTrnDangleC Translates the last 1 or two bases of a sequence
**                   in a char* text
** @use ajTrnDangleS Translates the last 1 or two bases of a sequence
**                     in a AjStr
** @use ajTrnSeqOrig Translating a sequence
** @cast ajTrnGetTitle Returns description of the translation table
** @cast ajTrnGetFilename Returns file name the translation table was read from
** @use ajTrnCodonstrTypeS Checks whether the input codon is a Start codon,
**                     a Stop codon or something else
** @use ajTrnCodonstrTypeC Checks whether a const char* codon is
**                      a Start codon, a Stop codon or something else
**
** @attr FileName [AjPStr] name of file that held the data
** @attr Title [AjPStr] title of data read from file
** @attr GC [char[16][16][16]] genetic codon table
** @attr Starts [char[16][16][16]] initiation site table
**
** @alias AjSTrn
** @alias AjOTrn
** @@
******************************************************************************/

typedef struct AjSTrn
{
    AjPStr FileName;
    AjPStr Title;
    char GC[16][16][16];
    char Starts[16][16][16];
} AjOTrn;

#define AjPTrn AjOTrn*




/* ========================================================================= */
/* =========================== public functions ============================ */
/* ========================================================================= */




/*
** Prototype definitions
*/

void          ajTrnDel(AjPTrn* pthis);
void          ajTrnExit(void);
AjPTrn        ajTrnNew(const AjPStr trnFileName);
AjPTrn        ajTrnNewI(ajint trnFileNameInt);
AjPTrn        ajTrnNewC(const char *trnFileName);
void          ajTrnReadFile(AjPTrn trnObj, AjPFile trnFile);
AjPSeq        ajTrnNewPep(const AjPSeq nucleicSeq, ajint frame);
char          ajTrnCodonS(const AjPTrn trnObj, const AjPStr codon);
char          ajTrnCodonRevS(const AjPTrn trnObj, const AjPStr codon);
char          ajTrnCodonC(const AjPTrn trnObj, const char *codon);
char          ajTrnCodonRevC(const AjPTrn trnObj, const char *codon);
char          ajTrnCodonK(const AjPTrn trnObj, const char *codon);
char          ajTrnRevCodonK(const AjPTrn trnObj, const char *codon);
void          ajTrnRevStr(const AjPTrn trnObj, const AjPStr str,
                          AjPStr *pep);
void          ajTrnSeqC(const AjPTrn trnObj, const char *str, ajint len,
                        AjPStr *pep);
void          ajTrnSeqInitC(const AjPTrn trnObj, const char *str, ajint len,
                            AjPStr *pep);
void          ajTrnSeqRevC(const AjPTrn trnObj, const char *str, ajint len,
                           AjPStr *pep);
void          ajTrnSeqRevinitC(const AjPTrn trnObj, const char *str, ajint len,
                               AjPStr *pep);
void          ajTrnSeqAltRevC(const AjPTrn trnObj, const char *str, ajint len,
                              AjPStr *pep);
void          ajTrnSeqAltRevinitC(const AjPTrn trnObj, const char *str,
                                  ajint len, AjPStr *pep);
void          ajTrnSeqS(const AjPTrn trnObj, const AjPStr str,
                        AjPStr *pep);
void          ajTrnSeqAltRevS(const AjPTrn trnObj, const AjPStr str,
                              AjPStr *pep);
void          ajTrnSeqSeq(const AjPTrn trnObj, const AjPSeq seq,
                          AjPStr *pep);
void          ajTrnSeqRevSeq(const AjPTrn trnObj, const AjPSeq seq,
                             AjPStr *pep);
void          ajTrnSeqAltRevSeq(const AjPTrn trnObj, const AjPSeq seq,
                                AjPStr *pep);
void          ajTrnSeqFrameC(const AjPTrn trnObj, const char *seq, ajint len,
                             ajint frame, AjPStr *pep);
void          ajTrnSeqFrameInitC(const AjPTrn trnObj, const char *seq,
                                 ajint len, ajint frame, AjPStr *pep);
const AjPStr  ajTrnName(ajint trnFileNameInt);
void          ajTrnSeqFrameS(const AjPTrn trnObj, const AjPStr seq,
                             ajint frame, AjPStr *pep);
void          ajTrnSeqFrameInitS(const AjPTrn trnObj, const AjPStr seq,
                                 ajint frame, AjPStr *pep);
void          ajTrnSeqFrameSeq(const AjPTrn trnObj, const AjPSeq seq,
                               ajint frame, AjPStr *pep);
void          ajTrnSeqFrameSeqInit(const AjPTrn trnObj, const AjPSeq seq,
                                   ajint frame, AjPStr *pep);
ajint         ajTrnSeqDangleC(const AjPTrn trnObj, const char *seq,
                              ajint frame, AjPStr *pep);
ajint         ajTrnSeqDangleS(const AjPTrn trnObj, const AjPStr seq,
                              ajint frame, AjPStr *pep);
AjPSeq        ajTrnSeqFramePep(const AjPTrn trnObj, const AjPSeq seq,
                               ajint frame);
AjPStr        ajTrnGetTitle(const AjPTrn thys);
AjPStr        ajTrnGetFilename(const AjPTrn thys);
ajint         ajTrnCodonstrTypeC(const AjPTrn trnObj,
                                 const char *codon, char *aa);
ajint         ajTrnCodonstrTypeS(const AjPTrn trnObj,
                                 const AjPStr codon, char *aa);
AjPSeq        ajTrnSeqInit(const AjPTrn trnObj, const AjPSeq seq,
                           ajint frame);
AjPSeq        ajTrnSeqOrig(const AjPTrn trnObj, const AjPSeq seq,
                           ajint frame);

/*
** End of prototype definitions
*/





#ifdef AJ_COMPILE_DEPRECATED_BOOK
#endif /* AJ_COMPILE_DEPRECATED_BOOK */

#ifdef AJ_COMPILE_DEPRECATED

__deprecated ajint         ajTrnStartStop(const AjPTrn trnObj,
                                          const AjPStr codon, char *aa);
__deprecated ajint         ajTrnStartStopC(const AjPTrn trnObj,
                                           const char *codon, char *aa);
__deprecated AjPStr        ajTrnGetFileName(const AjPTrn thys);
__deprecated const  AjPStr ajTrnCodon(const AjPTrn trnObj, const AjPStr codon);
__deprecated const  AjPStr ajTrnRevCodon(const AjPTrn trnObj,
                                         const AjPStr codon);
__deprecated void    ajTrnC(const AjPTrn trnObj, const char *str, ajint len,
                            AjPStr *pep);
__deprecated void    ajTrnRevC(const AjPTrn trnObj, const char *str,
                               ajint len, AjPStr *pep);
__deprecated void    ajTrnAltRevC(const AjPTrn trnObj, const char *str,
                                  ajint len, AjPStr *pep);
__deprecated void    ajTrnStr(const AjPTrn trnObj, const AjPStr str,
                              AjPStr *pep);
__deprecated void    ajTrnAltRevStr(const AjPTrn trnObj, const AjPStr str,
                                    AjPStr *pep);
__deprecated void    ajTrnSeq(const AjPTrn trnObj, const AjPSeq seq,
                              AjPStr *pep);
__deprecated void    ajTrnRevSeq(const AjPTrn trnObj, const AjPSeq seq,
                                 AjPStr *pep);
__deprecated void    ajTrnAltRevSeq(const AjPTrn trnObj, const AjPSeq seq,
                                    AjPStr *pep);
__deprecated void    ajTrnCFrame(const AjPTrn trnObj, const char *seq,
                                 ajint len, ajint frame, AjPStr *pep);
__deprecated void    ajTrnStrFrame(const AjPTrn trnObj, const AjPStr seq,
                                   ajint frame, AjPStr *pep);
__deprecated void    ajTrnSeqFrame(const AjPTrn trnObj, const AjPSeq seq,
                                   ajint frame, AjPStr *pep);
__deprecated ajint   ajTrnCDangle(const AjPTrn trnObj, const char *seq,
                                  ajint len, ajint frame, AjPStr *pep);
__deprecated ajint   ajTrnStrDangle(const AjPTrn trnObj, const AjPStr seq,
                                    ajint frame, AjPStr *pep);

#endif /* AJ_COMPILE_DEPRECATED */




AJ_END_DECLS

#endif /* !AJTRANSLATE_H */
