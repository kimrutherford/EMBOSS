/* @include ajdan *************************************************************
**
** AJAX nucleic acid functions
**
** @author Copyright (C) 1999 Alan Bleasby
** @version $Revision: 1.14 $
** @modified Feb 25 ajb First version
** @modified $Date: 2011/10/18 14:23:40 $ by $Author: rice $
** @@
**
** This library is free software; you can redistribute it and/or
** modify it under the terms of the GNU Lesser General Public
** License as published by the Free Software Foundation; either
** version 2.1 of the License, or (at your option) any later version.
**
** This library is distributed in the hope that it will be useful,
** but WITHOUT ANY WARRANTY; without even the implied warranty of
** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
** Lesser General Public License for more details.
**
** You should have received a copy of the GNU Lesser General Public
** License along with this library; if not, write to the Free Software
** Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,
** MA  02110-1301,  USA.
**
******************************************************************************/

#ifndef AJDAN_H
#define AJDAN_H

/* ========================================================================= */
/* ============================= include files ============================= */
/* ========================================================================= */

#include "ajdefine.h"
#include "ajstr.h"

AJ_BEGIN_DECLS




/* ========================================================================= */
/* =============================== constants =============================== */
/* ========================================================================= */




/* ========================================================================= */
/* ============================== public data ============================== */
/* ========================================================================= */




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
** @@
******************************************************************************/

typedef struct AjSMelt
{
    AjPStr pair;
    float enthalpy;
    float entropy;
    float energy;
    char Padding[4];
} AjOMelt;

#define AjPMelt AjOMelt*




/* ========================================================================= */
/* =========================== public functions ============================ */
/* ========================================================================= */




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




#ifdef AJ_COMPILE_DEPRECATED_BOOK
#endif /* AJ_COMPILE_DEPRECATED_BOOK */

#ifdef AJ_COMPILE_DEPRECATED

__deprecated float ajTm(const AjPStr strand, ajint len,
                        ajint shift, float saltconc,
                        float DNAconc, AjBool isDNA);
__deprecated float ajTm2(const char *strand, ajint pos,
                         ajint len, float saltconc,
                         float DNAconc, AjBool isDNA,
                         float **saveentr, float **saveenth, float **saveener);
__deprecated float ajProdTm(float gc, float saltconc, ajint len);

#endif /* AJ_COMPILE_DEPRECATED */




AJ_END_DECLS

#endif /* !AJDAN_H */
