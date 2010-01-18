#ifdef __cplusplus
extern "C"
{
#endif

#ifndef ajrange_h
#define ajrange_h




/* @data AjPRange *************************************************************
**
** Data structure for AJAX sequence range specifications
**
** @alias AjORange
** @alias AjSRange
**
** @attr start [ajuint*] From positions
** @attr end [ajuint*] End positions
** @attr text [AjPStr*] Associated text for each range
** @attr n [ajuint] Number of ranges
** @attr Padding [char[4]] Padding to alignment boundary
** @@
******************************************************************************/


typedef struct AjSRange
{
    ajuint *start;
    ajuint *end;
    AjPStr *text;
    ajuint n;
    char Padding[4];
} AjORange;
#define AjPRange AjORange*




/*
** Prototype definitions
*/

AjBool    ajRangeSetOffset (AjPRange thys, ajuint begin);
AjBool    ajRangeElementSet(AjPRange thys, ajuint element,
                            ajuint start, ajuint end);
AjBool    ajRangeIsWhole(const AjPRange thys, const AjPSeq s);
void      ajRangeDel(AjPRange *thys);
AjPRange  ajRangeNewI(ajuint n);
AjPRange  ajRangeNewFilename(const AjPStr name);
AjPRange  ajRangeNewFilenameLimits(const AjPStr name,
                                   ajuint imin, ajuint imax,
                                   ajuint minsize, ajuint size);
AjPRange  ajRangeNewRange(const AjPRange src);
AjPRange  ajRangeNewString(const AjPStr s);
AjPRange  ajRangeNewStringLimits(const AjPStr str, ajuint imin, ajuint imax,
                                 ajuint minsize, ajuint size);
ajuint    ajRangeGetSize(const AjPRange thys);
AjBool    ajRangeIsOrdered (const AjPRange thys);
ajuint	  ajRangeCountOverlaps (const AjPRange thys, ajuint pos, ajuint length);
ajuint    ajRangeElementTypeOverlap(const AjPRange thys, ajuint element,
                                    ajuint pos, ajuint length);
AjBool    ajRangeSeqExtract (const AjPRange thys, AjPSeq seq);
AjPSeq    ajRangeSeqExtractPep (const AjPRange thys, AjPSeq seq,
                                const AjPTrn trntable, ajint frame);
AjBool    ajRangeSeqExtractList (const AjPRange thys,
				 const AjPSeq seq, AjPList outliststr);
AjBool    ajRangeSeqMask (const AjPRange thys,
			  const AjPStr maskchar, AjPSeq seq);
AjBool    ajRangeSeqStuff (const AjPRange thys, AjPSeq seq);
AjBool    ajRangeSeqStuffPep (const AjPRange thys, AjPSeq seq,
                              ajint frame);
AjBool    ajRangeSeqToLower (const AjPRange thys, AjPSeq seq);
AjBool    ajRangeStrExtract (const AjPRange thys,
			     const AjPStr instr, AjPStr *outstr);
AjBool    ajRangeStrExtractList (const AjPRange thys,
				 const AjPStr instr, AjPList outliststr);
AjBool    ajRangeStrMask (const AjPRange thys,
			  const AjPStr maskchar, AjPStr *str);
AjBool    ajRangeStrStuff (const AjPRange thys,
			   const AjPStr instr, AjPStr *outstr);
AjBool    ajRangeStrToLower (const AjPRange thys, AjPStr *str);
AjBool    ajRangeElementGetText(const AjPRange thys, ajuint element,
                                AjPStr * text);
AjBool    ajRangeElementGetValues(const AjPRange thys, ajuint element,
                                  ajuint *start, ajuint *end);

/*
** End of prototype definitions
*/

__deprecated AjPRange  ajRangeCopy(const AjPRange src);
__deprecated AjPRange  ajRangeGet(const AjPStr s);
__deprecated AjPRange  ajRangeGetLimits(const AjPStr str,
                                        ajuint imin, ajuint imax,
                                        ajuint minsize, ajuint size);
__deprecated AjPRange  ajRangeFile(const AjPStr name);
__deprecated AjPRange  ajRangeFileLimits(const AjPStr name,
                                         ajuint imin, ajuint imax,
                                         ajuint minsize, ajuint size);
__deprecated ajuint    ajRangeNumber(const AjPRange thys);
__deprecated AjBool    ajRangeValues(const AjPRange thys, ajuint element,
                                     ajuint *start, ajuint *end);
__deprecated AjBool    ajRangeBegin (AjPRange thys, ajuint begin);
__deprecated AjBool    ajRangeChange(AjPRange thys, ajuint element,
                                     ajuint start, ajuint end);
__deprecated AjBool    ajRangeText(const AjPRange thys, ajuint element,
                                   AjPStr * text);
__deprecated ajuint	  ajRangeOverlaps (const AjPRange thys,
                                           ajuint pos, ajuint length);
__deprecated ajuint	  ajRangeOverlapSingle (ajuint start, ajuint end,
				ajuint pos, ajuint length);
__deprecated AjBool    ajRangeOrdered (const AjPRange thys);
__deprecated AjBool    ajRangeDefault(const AjPRange thys, const AjPSeq s);

#endif

#ifdef __cplusplus
}
#endif
