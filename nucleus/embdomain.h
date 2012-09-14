/* @include embdomain *********************************************************
**
** Algorithms for handling protein domain data.
** For use with Scop and Cath objects defined in ajdomain.h
** 
** @author Copyright (c) 2004 Jon Ison (jison@hgmp.mrc.ac.uk) 
** @version $Revision: 1.7 $
** @modified $Date: 2011/10/18 14:24:24 $ by $Author: rice $
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
****************************************************************************/

#ifndef EMBDOMAIN_H
#define EMBDOMAIN_H



/* ========================================================================= */
/* ============================= include files ============================= */
/* ========================================================================= */

#include "ajdefine.h"
#include "ajdomain.h"
#include "ajstr.h"
#include "ajlist.h"

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

/* ======================================================================= */
/* ========================== Scop object ================================ */
/* ======================================================================= */

AjPStr   embScopToPdbid(const AjPStr scop, AjPStr *pdb);
AjBool   embScopToSp(const AjPStr scop, AjPStr *spr, const AjPList list);
AjBool   embScopToAcc(const AjPStr scop, AjPStr *acc, const AjPList list);

/*
** End of prototype definitions
*/

AJ_END_DECLS

#endif  /* !EMBDOMAIN_H */















