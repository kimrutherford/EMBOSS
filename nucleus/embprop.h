#ifdef __cplusplus
extern "C"
{
#endif

#ifndef embprop_h
#define embprop_h

#define EMBPROPSIZE 29
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

#define EMBPROPHINDEX 26
#define EMBPROPOINDEX 27
#define EMBPROPWINDEX 28

/* define monoisotopic masses for common N- and C- terminal modifications
**
**Values from Expasy
**
**http://www.expasy.org/tools/findmod/findmod_masses.html
*/

#define EMBPROPMSTN_H       1.00783
#define EMBPROPMSTN_FORMYL 29.01823
#define EMBPROPMSTN_ACETYL 43.04013

#define EMBPROPMSTC_OH     17.00274
#define EMBPROPMSTC_AMIDE  16.01804




/* @data EmbPPropAmino ********************************************************
**
** Amino acid properties
**
** @attr tiny [ajint] tiny
** @attr sm_all [ajint] small
** @attr aliphatic [ajint] aliphatic
** @attr aromatic [ajint] aromatic
** @attr nonpolar [ajint] non-polar
** @attr polar [ajint] polar
** @attr charge [float] charge
** @attr pve [ajint] positive
** @attr nve [ajint] negative
** @attr extcoeff [ajint] extinction coefficient
** @attr Padding [char[4]] padding to alignment boundary
** @@
******************************************************************************/

typedef struct EmbSPropAmino
{
    ajint     tiny;
    ajint     sm_all;
    ajint     aliphatic;
    ajint     aromatic;
    ajint     nonpolar;
    ajint     polar;
    float     charge;
    ajint     pve;
    ajint     nve;
    ajint     extcoeff;
    char      Padding[4];
} EmbOPropAmino;
#define EmbPPropAmino EmbOPropAmino*




/* @data EmbPPropMolwt ********************************************************
**
** Molecular weights
**
** @attr average [double] average molwt
** @attr mono [double] monoisotopic molwt
** @@
******************************************************************************/

typedef struct EmbSPropMolwt
{
    double average;
    double mono;
} EmbOPropMolwt;
#define EmbPPropMolwt EmbOPropMolwt*




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

EmbPPropAmino *embPropEaminoRead(AjPFile fp);
EmbPPropMolwt *embPropEmolwtRead(AjPFile fp);
void          embPropAminoDel(EmbPPropAmino **thys);
void          embPropMolwtDel(EmbPPropMolwt **thys);

float embPropMolwtGetMolwt(const EmbPPropMolwt prop);
float embPropGetCharge(const EmbPPropAmino prop);
AjBool embPropGetProperties(const EmbPPropAmino prop, AjPStr* Pstr);
ajint embPropGetTiny(const EmbPPropAmino prop);
ajint embPropGetSmall(const EmbPPropAmino prop);
ajint embPropGetAliphatic(const EmbPPropAmino prop);
ajint embPropGetAromatic(const EmbPPropAmino prop);
ajint embPropGetNonpolar(const EmbPPropAmino prop);
ajint embPropGetPolar(const EmbPPropAmino prop);
ajint embPropGetPve(const EmbPPropAmino prop);
ajint embPropGetNve(const EmbPPropAmino prop);
ajint embPropGetExtcoeff(const EmbPPropAmino prop);



/* void    embPropAminoRead (void); */
void 	embPropCalcFragments (const char *s, ajint n,
			      AjPList *l, AjPList *pa,
			      AjBool unfavoured, AjBool overlap,
			      AjBool allpartials, ajint *ncomp, ajint *npart,
			      AjPStr *rname, AjBool nterm, AjBool cterm,
			      AjBool dorag, EmbPPropMolwt const  *mwdata,
			      AjBool mono);
double  embPropCalcMolextcoeff(const char *s, ajint start, ajint end,
			       AjBool cystine, EmbPPropAmino const *aadata);
double  embPropCalcMolwt (const char *s, ajint start, ajint end,
			  EmbPPropMolwt const *mwdata, AjBool mono);
/* new method for chemically modified ends */
double  embPropCalcMolwtMod (const char *s, ajint start, ajint end,
			     EmbPPropMolwt const *mwdata, AjBool mono,
			     double nmass, double cmass);
const char*   embPropCharToThree (char c);

void    embPropFixF(float matrix[], float missing);
const char*   embPropIntToThree (ajint c);
void    embPropNormalF(float matrix[], float missing);
AjPStr  embPropProtGaps (AjPSeq seq, ajint pad);
AjPStr  embPropProt1to3 (AjPSeq seq, ajint pad);
AjPStr  embPropProt1to3Rev (AjPSeq seq, ajint pad);
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
