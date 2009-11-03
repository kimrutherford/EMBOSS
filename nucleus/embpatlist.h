#ifdef __cplusplus
extern "C"
{
#endif

#ifndef embpatlist_h
#define embpatlist_h

/*
** Prototype definitions
*/

void   embPatlistSeqSearch   (AjPFeattable ftable, const AjPSeq seq,
			      AjPPatlistSeq plist, AjBool reverse);
void   embPatlistRegexSearch (AjPFeattable ftable, const AjPSeq seq,
			      AjPPatlistRegex plist, AjBool reverse);
void   embPatternRegexSearch (AjPFeattable ftable, const AjPSeq seq,
			      const AjPPatternRegex pat, AjBool reverse);
void   embPatternSeqSearch   (AjPFeattable ftable, const AjPSeq seq,
			      const AjPPatternSeq pat, AjBool reverse);
AjBool embPatternSeqCompile  (AjPPatternSeq pat);

void   embPatlistExit        (void);

/*
** End of prototype definitions
*/

#endif

#ifdef __cplusplus
}
#endif
