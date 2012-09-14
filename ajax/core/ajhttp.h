/* @include ajhttp ************************************************************
**
** AJAX HTTP (database) functions
**
** These functions control all aspects of AJAX http access
** via SEND/GET/POST protocols
**
** @author Copyright (C) 2010 Alan Bleasby
** @version $Revision: 1.16 $
** @modified $Date: 2011/11/23 09:56:06 $ by $Author: rice $
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

#ifndef AJHTTP_H
#define AJHTTP_H

/* ========================================================================= */
/* ============================= include files ============================= */
/* ========================================================================= */

#include "ajdefine.h"
#include "ajstr.h"
#include "ajfile.h"
#include "ajsys.h"
#include "ajquery.h"

AJ_BEGIN_DECLS




/* ========================================================================= */
/* =============================== constants =============================== */
/* ========================================================================= */




/* ========================================================================= */
/* ============================== public data ============================== */
/* ========================================================================= */




/* @data AjPUrlref ************************************************************
**
** Structure to hold parts of a URL
**
** @alias AjSUrlref
** @alias AjOUrlref
**
** @attr Method [AjPStr] Method of access (e.g. http)
** @attr Host [AjPStr] host
** @attr Port [AjPStr] Port
** @attr Absolute [AjPStr] Absolute path
** @attr Relative [AjPStr] Relative path
** @attr Fragment [AjPStr] Fragment/section
** @attr Username [AjPStr] Username
** @attr Password [AjPStr] Password
** @@
******************************************************************************/

typedef struct AjSUrlref
{
    AjPStr Method;
    AjPStr Host;
    AjPStr Port;
    AjPStr Absolute;
    AjPStr Relative;
    AjPStr Fragment;
    AjPStr Username;
    AjPStr Password;
} AjOUrlref;

#define AjPUrlref AjOUrlref*




/* ========================================================================= */
/* =========================== public functions ============================ */
/* ========================================================================= */




/*
** Prototype definitions
*/

AjBool ajHttpGetProxyinfo(const AjPStr dbproxy, ajint* proxyport,
                          AjPStr* proxyname, AjPStr* proxyauth,
                          AjPStr* proxycreds);
AjBool ajHttpGetVersion(const AjPStr version, AjPStr* httpver);
FILE*  ajHttpOpen(const AjPStr dbname, const AjPStr host, ajint iport,
                  const AjPStr get, AjPSysSocket Psock);
FILE*  ajHttpOpenProxy(const AjPStr dbname, const AjPStr proxyname,
                       ajint proxyport, const AjPStr proxyauth,
                       const AjPStr proxycreds, const AjPStr host,
                       ajint iport, const AjPStr get, AjPSysSocket Psock);
AjPFilebuff ajHttpRead(const AjPStr dbhttpver, const AjPStr dbname,
                       const AjPStr dbproxy, const AjPStr host,
                       ajint port, const AjPStr dbpath);
AjPFilebuff ajHttpReadPos(const AjPStr dbhttpver, const AjPStr dbname,
                          const AjPStr dbproxy, const AjPStr host,
                          ajint port, const AjPStr dbpath, ajlong fpos);
AjBool ajHttpRedirect(AjPFilebuff buff, AjPStr* host, ajint* port,
                      AjPStr* get, ajuint *httpcode);

AjPUrlref ajHttpUrlrefNew(void);
void   ajHttpUrlrefDel(AjPUrlref *thys);
void   ajHttpUrlrefParseC(AjPUrlref *parts, const char *url);
void   ajHttpUrlrefParseS(AjPUrlref *parts, const AjPStr surl);
void   ajHttpUrlrefSplitPort(AjPUrlref urli);
void   ajHttpUrlrefSplitUsername(AjPUrlref urli);

AjBool ajHttpQueryUrl(const AjPQuery qry, ajint* iport, AjPStr* host,
                      AjPStr* urlget);

void   ajHttpUrlDeconstruct(const AjPStr url, ajint* iport, AjPStr* host,
                            AjPStr* urlget);

/*
** End of prototype definitions
*/




AJ_END_DECLS

#endif /* !AJHTTP_H */
