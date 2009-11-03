#ifdef __cplusplus
extern "C"
{
#endif

#ifndef embcons_h
#define embcons_h




/*
** Prototype definitions
*/

void embConsCalc (const AjPSeqset seqset, const AjPMatrix cmpmatrix,
	ajint nseqs, ajint mlen,float fplural, float setcase,
	ajint identity, AjBool gaps, AjPStr *cons);

/*
** End of prototype definitions
*/

#endif

#ifdef __cplusplus
}
#endif
