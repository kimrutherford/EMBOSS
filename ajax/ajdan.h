#ifdef __cplusplus
extern "C"
{
#endif

#ifndef ajdan_h
#define ajdan_h

#include "ajax.h"

typedef struct AjMelt AjMelt;
struct AjMelt
{
    AjPStr pair;
    float enthalpy;
    float entropy;
    float energy;
    char Padding[4];
};




/*
** Prototype definitions
*/

void  ajMeltExit(void);
void  ajMeltInit(AjBool isdna, ajint savesize);
float ajProbScore(const AjPStr seq1, const AjPStr seq2, ajint len);
float ajMeltEnergy(const AjPStr strand, ajint len,
		   ajint shift, AjBool isDNA,
		   AjBool maySave, float *enthalpy, float *entropy);
float ajMeltEnergy2(const char *strand, ajint pos, ajint len,
		    AjBool isDNA,
		    float *enthalpy, float *entropy,
		    float **saveentr, float **saveenth, float **saveener);
float ajTm(const AjPStr strand, ajint len, ajint shift, float saltconc,
	   float DNAconc, AjBool isDNA);
float ajTm2(const char *strand, ajint pos, ajint len, float saltconc,
	    float DNAconc, AjBool isDNA,
	    float **saveentr, float **saveenth, float **saveener);
float ajMeltGC(const AjPStr strand, ajint len);
float ajProdTm(float gc, float saltconc, ajint len);
float ajAnneal(float tmprimer, float tmproduct);

/*
** End of prototype definitions
*/

#endif

#ifdef __cplusplus
}
#endif
