/* @include ajsoap ************************************************************
**
** support for webservices access using Axis2C library
**
** @author Copyright (c) 2009 Mahmut Uludag
** @version $Revision: 1.3 $
** @modified $Date: 2012/07/15 20:24:49 $ by $Author: rice $
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

#ifndef AJSOAP_H
#define AJSOAP_H

/* ========================================================================= */
/* ============================= include files ============================= */
/* ========================================================================= */

#include "ajdefine.h"

#ifdef HAVE_AXIS2C
#include <axiom.h>
#include <axis2_util.h>
#include <axiom_soap.h>
#include <axis2_client.h>
#include <axutil_qname.h>

AJ_BEGIN_DECLS





/* ========================================================================= */
/* =============================== constants =============================== */
/* ========================================================================= */




/*
 * default end-points and name-spaces for EBI web-services
 */

#define WSDBFETCH_NS "http://www.ebi.ac.uk/ws/services/WSDbfetchDoclit"
#define WSDBFETCH_EP "http://www.ebi.ac.uk/ws/services/WSDbfetchDoclit"
#define EBEYE_NS     "http://www.ebi.ac.uk/EBISearchService"
#define EBEYE_EP     "http://www.ebi.ac.uk/ebisearch/service.ebi"

#define UNRECOGNISEDXML "cannot parse webservices results"





/* ========================================================================= */
/* ============================== public data ============================== */
/* ========================================================================= */




/* ========================================================================= */
/* =========================== public functions ============================ */
/* ========================================================================= */



/*
** Prototype definitions
*/

axiom_node_t* 	    ajSoapAxis2Call(axis2_svc_client_t *client,
              	                    const axutil_env_t *env,
              	                    axiom_node_t *payload);

AjBool              ajSoapAxis2Error(axiom_node_t *fault,
				     const axutil_env_t *env);


axis2_svc_client_t* ajSoapAxis2GetClient(const axutil_env_t *env,
                                         const axis2_char_t *address);

axutil_env_t*       ajSoapAxis2GetEnv(void);

/*
** End of prototype definitions
*/

AJ_END_DECLS

#endif  /* HAVE_AXIS2C */
#endif  /* !AJSOAP_H */

