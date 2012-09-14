/* @include ajvar *************************************************************
**
** AJAX variation functions
**
** These functions control all aspects of AJAX variation
** parsing and include simple utilities.
**
** @author Copyright (C) 2010 Peter Rice
** @version $Revision: 1.7 $
** @modified Oct 5 pmr First version
** @modified $Date: 2012/07/02 17:37:28 $ by $Author: rice $
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

#ifndef AJVAR_H
#define AJVAR_H

/* ========================================================================= */
/* ============================= include files ============================= */
/* ========================================================================= */

#include "ajdefine.h"
#include "ajvardata.h"

AJ_BEGIN_DECLS




/* ========================================================================= */
/* =============================== constants =============================== */
/* ========================================================================= */




/* ========================================================================= */
/* ============================== public data ============================== */
/* ========================================================================= */




/* ========================================================================= */
/* =========================== public functions ============================ */
/* ========================================================================= */




/*
** Prototype definitions
*/

AjPVar       ajVarNew(void);
void         ajVarDel(AjPVar *Pvar);
void         ajVarClear(AjPVar var);
void         ajVarReset(AjPVar var);

AjPVarData   ajVardataNew(void);
void         ajVardataDel(AjPVarData *Pvar);
void         ajVardataClear(AjPVarData var);

AjPVarField  ajVarfieldNew(void);
void         ajVarfieldDel(AjPVarField *Pvar);
void         ajVarfieldClear(AjPVarField var);
const char*  ajVarfieldGetNumber(const AjPVarField var);
const char*  ajVarfieldGetType(const AjPVarField var);

AjPVarHeader ajVarheaderNew(void);
void         ajVarheaderDel(AjPVarHeader *Pvar);
void         ajVarheaderClear(AjPVarHeader var);

AjPVarSample ajVarsampleNew(void);
void         ajVarsampleDel(AjPVarSample *Pvar);
void         ajVarsampleClear(AjPVarSample var);

const AjPStr ajVarGetDb(const AjPVar var);
const AjPStr ajVarGetId(const AjPVar var);
const char*  ajVarGetQryC(const AjPVar var);
const AjPStr ajVarGetQryS(const AjPVar var);

void         ajVarExit(void);

/*
** End of prototype definitions
*/




AJ_END_DECLS

#endif /* !AJVAR_H */
