/* @source Ensembl Software functions
**
** @author Copyright (C) 1999 Ensembl Developers
** @author Copyright (C) 2006 Michael K. Schuster
** @modified 2009 by Alan Bleasby for incorporation into EMBOSS core
** @modified $Date: 2011/07/06 21:58:55 $ by $Author: mks $
** @version $Revision: 1.7 $
** @@
**
** This library is free software; you can redistribute it and/or
** modify it under the terms of the GNU Library General Public
** License as published by the Free Software Foundation; either
** version 2 of the License, or (at your option) any later version.
**
** This library is distributed in the hope that it will be useful,
** but WITHOUT ANY WARRANTY; without even the implied warranty of
** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
** Library General Public License for more details.
**
** You should have received a copy of the GNU Library General Public
** License along with this library; if not, write to the
** Free Software Foundation, Inc., 59 Temple Place - Suite 330,
** Boston, MA  02111-1307, USA.
******************************************************************************/

/* ==================================================================== */
/* ========================== include files =========================== */
/* ==================================================================== */

#include "enssoftware.h"




/* ==================================================================== */
/* ============================ constants ============================= */
/* ==================================================================== */

/* @const ensSoftwareVersion **************************************************
**
** This is the software version of the AJAX Ensembl library.
** Interoperation is only guaranteed if software and database versions match.
**
******************************************************************************/

const char* ensSoftwareVersion = "62";




/* ==================================================================== */
/* ======================== global variables ========================== */
/* ==================================================================== */




/* ==================================================================== */
/* ========================== private data ============================ */
/* ==================================================================== */




/* ==================================================================== */
/* ======================== private constants ========================= */
/* ==================================================================== */




/* ==================================================================== */
/* ======================== private variables ========================= */
/* ==================================================================== */




/* ==================================================================== */
/* ======================== private functions ========================= */
/* ==================================================================== */




/* ==================================================================== */
/* ===================== All functions by section ===================== */
/* ==================================================================== */




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
** @cc CVS Revision: 1.8
** @cc CVS Tag: branch-ensembl-62
**
******************************************************************************/




/* @section element retrieval *************************************************
**
** Functions for returning elements of the Ensembl Software internals.
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
** Get the version element of the Ensembl Software.
**
** @cc Bio::EnsEMBL::ApiVersion::software_version
** @return [const char*] Ensembl Software Version or NULL
** @@
******************************************************************************/

const char* ensSoftwareGetVersion(void)
{
    return ensSoftwareVersion;
}
