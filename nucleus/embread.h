#ifdef __cplusplus
extern "C"
{
#endif

#ifndef embread_h
#define embread_h

#define AJREADAMINO 28




/*
** Prototype definitions
*/

AjBool embReadAminoDataDoubleC (const char *s, double **a, double fill);
AjBool embReadAminoDataFloatC  (const char *s, float **a, float fill);
AjBool embReadAminoDataIntC    (const char *s, ajint **a, ajint fill);

/*
** End of prototype definitions
*/

#endif

#ifdef __cplusplus
}
#endif
