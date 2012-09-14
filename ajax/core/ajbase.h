/* @include ajbase ************************************************************
**
** AJAX IUB base nucleic acid functions
**
** @author Copyright (C) 1999 Alan Bleasby
** @version $Revision: 1.15 $
** @modified Feb 28 ajb First version
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

#ifndef AJBASE_H
#define AJBASE_H

/* ========================================================================= */
/* ============================= include files ============================= */
/* ========================================================================= */

#include "ajdefine.h"
#include "ajstr.h"

#include <ctype.h>

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

const AjPStr  ajBaseGetCodes(ajint ibase);
const AjPStr  ajBaseGetMnemonic(ajint base);
char          ajBaseAlphacharComp(char base);
void          ajBaseExit(void);
float         ajBaseAlphaCompare(ajint base1, ajint base2);

ajint         ajBaseAlphaToBin(ajint c);
char          ajBaseBinToAlpha(ajint c);

char          ajBaseAlphacharToBin(char c);
float         ajBaseAlphacharCompare(char c, char c2);
AjBool        ajBaseFromDoublet(const AjPStr nuc2, char* Pc);

ajint         ajBasecodeToInt(ajint c);
ajint         ajBasecodeFromInt(ajint n);

ajint         ajResidueAlphaToBin(ajint c);
char          ajResidueBinToAlpha(ajint c);
const AjPStr  ajResidueGetCodes(ajint ires);
const AjPStr  ajResidueGetMnemonic(ajint base);
AjBool        ajResidueFromTriplet(const AjPStr aa3, char *Pc);
AjBool        ajResidueToTriplet(char c, AjPStr *Paa3);
AjBool        ajBaseExistsBin(ajint base);
AjBool        ajBaseExistsChar(char c);
AjBool        ajResidueExistsBin(ajint base);
AjBool        ajResidueExistsChar(char c);

/*
** End of prototype definitions
*/




#ifdef AJ_COMPILE_DEPRECATED_BOOK
#endif /* AJ_COMPILE_DEPRECATED_BOOK */

#ifdef AJ_COMPILE_DEPRECATED

__deprecated AjBool        ajBaseAa1ToAa3(char aa1, AjPStr *aa3);
__deprecated AjBool        ajBaseAa3ToAa1(char *aa1, const AjPStr aa3);
__deprecated const AjPStr  ajBaseCodes(ajint ibase);
__deprecated char          ajSeqBaseComp(char base);
__deprecated ajint         ajAZToInt(ajint c);
__deprecated ajint         ajAZToBin(ajint c);
__deprecated char          ajAZToBinC(char c);
__deprecated char          ajBinToAZ(ajint c);
__deprecated ajint         ajIntToAZ(ajint n);
__deprecated char          ajBaseComp(char base);
__deprecated float         ajBaseProb(ajint base1, ajint base2);

#endif /* AJ_COMPILE_DEPRECATED */




AJ_END_DECLS

#endif /* !AJBASE_H */
