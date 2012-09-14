/* @include ajftp ************************************************************
**
** AJAX FTP (database) functions
**
** These functions control all aspects of AJAX ftp access
** via SEND/GET/POST protocols
**
** @author Copyright (C) 2010 Alan Bleasby
** @version $Revision: 1.2 $
** @modified $Date: 2011/11/17 13:04:31 $ by $Author: rice $
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

#ifndef AJFTP_H
#define AJFTP_H

/* ========================================================================= */
/* ============================= include files ============================= */
/* ========================================================================= */

#include "ajdefine.h"
#include "ajstr.h"
#include "ajfile.h"
#include "ajsys.h"
#include "ajquery.h"
#include "ajhttp.h"

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

FILE*  ajFtpOpen(const AjPStr dbname, const AjPStr host, ajint iport,
                 const AjPStr get, ajlong fpos, AjPSysSocket Psock);

AjPFilebuff ajFtpRead(const AjPStr dbname,
                      const AjPStr host,
                      ajint port,
                      ajlong fpos,
                      const AjPStr dbpath);


/*
** End of prototype definitions
*/




AJ_END_DECLS

#endif /* !AJFTP_H */
