#ifdef __cplusplus
extern "C"
{
#endif

#ifndef ajdan_h
#define ajdan_h

#include "ajax.h"




/* @data AjPMelt **************************************************************
**
** Melting values for di-nucleotides
**
** @alias AjSMelt
** @alias AjOMelt
**
** @attr pair [AjPStr] Di-nucleotide
** @attr enthalpy [float] Enthalpy
** @attr entropy [float] Entropy
** @attr energy [float] Free energy
** @attr Padding [char[4]] Padding
**
*/

typedef struct AjSMelt
{
    AjPStr pair;
    float enthalpy;
    float entropy;
    float energy;
    char Padding[4];
} AjOMelt;

#define AjPMelt AjOMelt*



/*
** Prototype definitions
*/

void  ajMeltExit(void);
void  ajMeltInit(AjBool isdna, ajint savesize);
float ajMeltEnergy(const AjPStr strand, ajint len,
		   ajint shift, AjBool isDNA,
		   AjBool maySave, float *enthalpy, float *entropy);
float ajMeltEnergy2(const char *strand, ajint pos, ajint len,
		    AjBool isDNA,
		    float *enthalpy, float *entropy,
		    float **saveentr, float **saveenth, float **saveener);
float ajMeltTemp(const AjPStr strand, ajint len, ajint shift, float saltconc,
	   float DNAconc, AjBool isDNA);
float ajMeltTempSave(const char *strand, ajint pos, ajint len, float saltconc,
	    float DNAconc, AjBool isDNA,
	    float **saveentr, float **saveenth, float **saveener);
float ajMeltGC(const AjPStr strand, ajint len);
float ajMeltTempProd(float gc, float saltconc, ajint len);
float ajAnneal(float tmprimer, float tmproduct);

/*
** End of prototype definitions
*/

__deprecated float ajTm(const AjPStr strand, ajint len,
                        ajint shift, float saltconc,
                        float DNAconc, AjBool isDNA);
__deprecated float ajTm2(const char *strand, ajint pos,
                         ajint len, float saltconc,
                         float DNAconc, AjBool isDNA,
                         float **saveentr, float **saveenth, float **saveener);
__deprecated float ajProdTm(float gc, float saltconc, ajint len);


#endif

#ifdef __cplusplus
}
#endif
