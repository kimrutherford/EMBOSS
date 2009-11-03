#ifdef __cplusplus
extern "C"
{
#endif

/*
* This set of routines was written primarily for the compseq program.
* Feel free to use it for anything you want.
*
* compseq counts the frequency of n-mers (or words) in a sequence.
*
* The easiest way to do this was to make a big ajulong array
* to hold the counts of each type of word.
*
* I needed a way of converting a sequence word into an integer so that I
* could increment the appropriate element of the array.
* (embNmerNuc2int and embNmerProt2int)
*
* I also needed a way of converting an integer back to a short sequence
* so that I could display the word.
* (embNmerInt2nuc and embNmerInt2prot)
*
* embNmerGetNoElements returns the number of elements required to store the
* results.
* In other words it is the number of possible words of size n.
*
* Gary Williams
*
*/

#ifndef embnmer_h
#define embnmer_h




/*
** Prototype definitions
*/

ajulong embNmerNuc2int (const char *seq, ajint wordsize, ajint offset,
			AjBool *otherflag);
ajint   embNmerInt2nuc (AjPStr *seq, ajint wordsize,
			ajulong value);
ajulong embNmerProt2int (const char *seq, ajint wordsize, ajint offset,
			 AjBool *otherflag, AjBool ignorebz);
ajint   embNmerInt2prot (AjPStr *seq, ajint wordsize,
			ajulong value, AjBool ignorebz);
AjBool  embNmerGetNoElements (ajulong *no_elements,
			      ajint word, AjBool seqisnuc,
			      AjBool ignorebz);

/*
** End of prototype definitions
*/

#endif

#ifdef __cplusplus
}
#endif
