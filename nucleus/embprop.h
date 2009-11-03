#ifdef __cplusplus
extern "C"
{
#endif

#ifndef embprop_h
#define embprop_h

#define EMBPROPSIZE 28
#define EMBPROPMOLWT      0
#define EMBEMBPROPTINY    1
#define EMBPROPSMALL      2
#define EMBPROPALIPHATIC  3
#define EMBPROPAROMATIC   4
#define EMBPROPNONPOLAR   5
#define EMBPROPPOLAR      6
#define EMBPROPCHARGE     7
#define EMBPROPPOSITIVE   8
#define EMBPROPNEGATIVE   9
#define EMBPROPABSORBANCE 10

/* define monoisotopic masses for common N- and C- terminal modifications */
#define EMBPROPMSTN_H       1.00782
#define EMBPROPMSTN_FORMYL 29.00274
#define EMBPROPMSTN_ACETYL 43.01839

#define EMBPROPMSTC_OH     17.00274
#define EMBPROPMSTC_AMIDE  16.01872



#ifndef WIN32
extern double *EmbPropTable[];
#else
#ifdef NUCLEUS_EXPORTS
__declspec(dllexport) double *EmbPropTable[];
#else
__declspec(dllimport) double *EmbPropTable[];
#endif
#endif




typedef struct EmbSPropFrag	/* Enzyme digestion structure */
{
    ajint     start;
    ajint     end;
    double    molwt;
    AjBool    isfrag;
    char      Padding[4];
} EmbOPropFrag;
#define EmbPPropFrag EmbOPropFrag*




/*
** Prototype definitions
*/

void    embPropAminoRead (AjPFile fp);
/* void    embPropAminoRead (void); */
void 	embPropCalcFragments (const char *s, ajint n,
			      AjPList *l, AjPList *pa,
			      AjBool unfavoured, AjBool overlap,
			      AjBool allpartials, ajint *ncomp, ajint *npart,
			      AjPStr *rname, AjBool nterm, AjBool cterm,
			      AjBool dorag);
double  embPropCalcMolextcoeff(const char *s, ajint start, ajint end);
double  embPropCalcMolwt (const char *s, ajint start, ajint end);
/* new method for chemically modified ends */
double  embPropCalcMolwtMod (const char *s, ajint start, ajint end,
			     double nmass, double cmass);
const char*   embPropCharToThree (char c);
void    embPropExit(void);
void    embPropFixF(float matrix[], float missing);
const char*   embPropIntToThree (ajint c);
AjPStr  embPropProtGaps (AjPSeq seq, ajint pad);
AjPStr  embPropProt1to3 (AjPSeq seq, ajint pad);
AjBool  embPropPurine (char base);
AjBool  embPropPyrimidine (char base);
AjBool  embPropTransversion (char base1, char base2);
AjBool  embPropTransition (char base1, char base2);

/*
** End of prototype definitions
*/

#endif

#ifdef __cplusplus
}
#endif
