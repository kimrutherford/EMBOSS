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
** @new ajRangeNewI Default constructor for range objects
** @new ajRangeGet Create a range object from a string
** @new ajRangeFile Create a range object from a file
** @delete ajRangeDel Default destructor for range objects
** @modify ajRangeChange Set the values of a start and end in a range element
** @modify ajRangeBegin Sets the range values relative to the Begin value
** @use ajRangeNumber Return the number of ranges in a range object
** @use ajRangeValues Return (as parameters) start and end values in a range
** @use ajRangeText Return (as parameters) text value of a range
** @use ajRangeStrExtractList PushApp substrings defined by range onto list
** @use ajRangeStrExtract Extract substrings defined by range
** @use ajRangeStrStuff The opposite of ajRangeStrExtract
** @use ajRangeStrMask Mask the range in a String
** @use ajRangeStrToLower Change to lower-case the range in a String
** @use ajRangeOverlaps Detect overlaps of a set of ranges to a seq region
** @use ajRangeOrdered Test if ranges are in ascending non-overlapping order
** @use ajRangeDefault Test if the default range has been set
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

AjBool    ajRangeBegin (AjPRange thys, ajuint begin);
AjBool    ajRangeChange(AjPRange thys, ajuint element,
			ajuint start, ajuint end);
AjPRange  ajRangeCopy(const AjPRange src);
AjBool    ajRangeDefault(const AjPRange thys, const AjPSeq s);
void      ajRangeDel(AjPRange *thys);
AjPRange  ajRangeFile(const AjPStr name);
AjPRange  ajRangeFileLimits(const AjPStr name, ajuint imin, ajuint imax,
			   ajuint minsize, ajuint size);
AjPRange  ajRangeGet(const AjPStr s);
AjPRange  ajRangeGetLimits(const AjPStr str, ajuint imin, ajuint imax,
			   ajuint minsize, ajuint size);
AjPRange  ajRangeNewI(ajuint n);
ajuint    ajRangeNumber(const AjPRange thys);
AjBool    ajRangeOrdered (const AjPRange thys);
ajuint	  ajRangeOverlaps (const AjPRange thys, ajuint pos, ajuint length);
ajuint	  ajRangeOverlapSingle (ajuint start, ajuint end,
				ajuint pos, ajuint length);
AjBool    ajRangeSeqExtract (const AjPRange thys, AjPSeq seq);
AjBool    ajRangeSeqExtractList (const AjPRange thys,
				 const AjPSeq seq, AjPList outliststr);
AjBool    ajRangeSeqMask (const AjPRange thys,
			  const AjPStr maskchar, AjPSeq seq);
AjBool    ajRangeSeqStuff (const AjPRange thys, AjPSeq seq);
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
AjBool    ajRangeText(const AjPRange thys, ajuint element, AjPStr * text);
AjBool    ajRangeValues(const AjPRange thys, ajuint element,
			ajuint *start, ajuint *end);

/*
** End of prototype definitions
*/

#endif

#ifdef __cplusplus
}
#endif
