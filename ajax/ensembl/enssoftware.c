/* @source enssoftware ********************************************************
**
** Ensembl Software functions
**
** @author Copyright (C) 1999 Ensembl Developers
** @author Copyright (C) 2006 Michael K. Schuster
** @version $Revision: 1.18 $
** @modified 2009 by Alan Bleasby for incorporation into EMBOSS core
** @modified $Date: 2013/02/17 13:04:02 $ by $Author: mks $
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

/* ========================================================================= */
/* ============================= include files ============================= */
/* ========================================================================= */

#include "enssoftware.h"




/* ========================================================================= */
/* =============================== constants =============================== */
/* ========================================================================= */

/* @const ensKSoftwareVersion *************************************************
**
** This is the software version of the AJAX Ensembl library.
** Interoperation is only guaranteed if software and database versions match.
**
******************************************************************************/

const char *ensKSoftwareVersion = "68";




/* ========================================================================= */
/* =========================== global variables ============================ */
/* ========================================================================= */




/* ========================================================================= */
/* ============================= private data ============================== */
/* ========================================================================= */




/* ========================================================================= */
/* =========================== private constants =========================== */
/* ========================================================================= */




/* ========================================================================= */
/* =========================== private variables =========================== */
/* ========================================================================= */




/* ========================================================================= */
/* =========================== private functions =========================== */
/* ========================================================================= */




/* ========================================================================= */
/* ======================= All functions by section ======================== */
/* ========================================================================= */




/* @filesection enssoftware ***************************************************
**
** @nam1rule ens Function belongs to the Ensembl library
** @nam2rule Software Functions for manipulating Ensembl Software
**
******************************************************************************/




/* @datasection [none] Ensembl Software Internals *****************************
**
** Functions to control Ensembl Software Internals
**
** @cc Bio::EnsEMBL::ApiVersion
** @cc CVS Revision: 1.15
** @cc CVS Tag: branch-ensembl-68
**
******************************************************************************/




/* @section member retrieval **************************************************
**
** Functions for returning members of the Ensembl Software internals.
**
** @fdata [none]
**
** @nam3rule Get Return Ensembl Software attribute(s)
** @nam4rule Version Return the version
**
** @valrule Version [const char*] Version or NULL
**
** @fcategory use
******************************************************************************/




/* @func ensSoftwareGetVersion ************************************************
**
** Get the version member of the Ensembl Software.
**
** @cc Bio::EnsEMBL::ApiVersion::software_version
** @return [const char*] Ensembl Software Version or NULL
**
** @release 6.4.0
** @@
******************************************************************************/

const char* ensSoftwareGetVersion(void)
{
    return ensKSoftwareVersion;
}
