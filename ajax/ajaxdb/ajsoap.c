/* @source ajsoap *************************************************************
**
** support for webservices access using Axis2C library
**
** @author Copyright (c) 2009 Mahmut Uludag
** @version $Revision: 1.8 $
** @modified $Date: 2011/10/18 14:23:39 $ by $Author: rice $
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

#include "ajsoap.h"
#include "ajlib.h"
#include "ajnam.h"
#include "ajutil.h"




#ifdef HAVE_AXIS2C

#define AXIS_NS "http://xml.apache.org/axis/"

static char axis2C_HOME[] = AXIS2C_LOC;




/* @func ajSoapAxis2Call ******************************************************
**
** Makes an axis2 webservices call and logs/reports its raw results
**
** @param [u] client [axis2_svc_client_t*] axis2 client obj
** @param [r] env [const axutil_env_t*] axis2 environment obj
** @param [u] payload [axiom_node_t*] webservices input
** @return [axiom_node_t*] axis2 OM object, webservices output
**
** @release 6.4.0
** @@
******************************************************************************/

axiom_node_t* ajSoapAxis2Call(axis2_svc_client_t *client,
                              const axutil_env_t *env,
                              axiom_node_t *payload)
{
    axiom_node_t *ret_node = NULL;
    axis2_char_t * om_str  = NULL;

    if (ajDebugOn())
    {
	om_str = axiom_node_to_string(payload, env);

	if (om_str)
	{
	    ajDebug("\nSending OM : %s\n", om_str);
	    AXIS2_FREE(env->allocator, om_str);
	    om_str = NULL;
	}
    }

    /* Send request */
    ret_node = axis2_svc_client_send_receive(client, env, payload);

    if (ret_node)
    {
	if(ajDebugOn())
	{
	    om_str = axiom_node_to_string(ret_node, env);

	    if (om_str)
		ajDebug("\nReceived OM : %s\n", om_str);

	    AXIS2_FREE(env->allocator, om_str);
	    om_str = NULL;
	}
    }
    else
    {
	ajDebug("seqAxis2wsCall: webservice call failed: %s\n",
	                AXIS2_ERROR_GET_MESSAGE(env->error));
	ajErr("webservice call FAILED: %s\n",
		AXIS2_ERROR_GET_MESSAGE(env->error));
    }

    return ret_node;
}




/* @func ajSoapAxis2GetClient *************************************************
**
** Returns an axis2 client object for web services calls
**
** @param [r] env [const axutil_env_t*] axis2 environment
** @param [r] address [const axis2_char_t*] webservices endpoint
** @return [axis2_svc_client_t*] axis2 service client
**
** @release 6.4.0
** @@
******************************************************************************/

axis2_svc_client_t* ajSoapAxis2GetClient(const axutil_env_t *env,
                                         const axis2_char_t *address)
{
    AjPStr home = NULL;

    axis2_endpoint_ref_t* endpoint_ref = NULL;
    axis2_options_t* options           = NULL;
    axis2_svc_client_t* svc_client     = NULL;

    options      = axis2_options_create(env);

    endpoint_ref = axis2_endpoint_ref_create(env, address);

    axis2_options_set_soap_version(options, env, AXIOM_SOAP11);
    axis2_options_set_to(options, env, endpoint_ref);

    /* Set up axis2c location.
    ** This is either from:
    ** EMBOSS_AXIS2C_HOME: Used by mEMBOSS as axis2c files are
    **               provided by the setup and there could
    **               be confusion with an existing axis2c
    **               installation otherwise.
    ** axis2C_HOME: The location specified during the
    **              EMBOSS configuration.
    **
    ** The client uses this information to pick up the
    ** libraries, modules and axis2.xml file
    */

    if(!ajNamGetValueC("AXIS2C_HOME",&home))
        home = ajStrNewC(axis2C_HOME);

    /* Create service client */
    svc_client = axis2_svc_client_create(env, ajStrGetPtr(home));

    if (!svc_client)
    {
	ajErr("Error creating webservice client: %s",
	      AXIS2_ERROR_GET_MESSAGE(env->error));
	return NULL;
    }

    axis2_svc_client_set_options(svc_client, env, options);
    axis2_svc_client_engage_module(svc_client, env, AXIS2_MODULE_ADDRESSING);

    ajStrDel(&home);

    return svc_client;
}




/* @func ajSoapAxis2Error *****************************************************
**
** Processes a webservices fault object received after a webservices call,
** and generates an ajax error unless it is wsdbfetch no-entry-found exception.
**
** @param [u] fault [axiom_node_t*] fault object
**                                  received after a webservices call
** @param [r] env [const axutil_env_t*] axis2 environment obj
** @return [AjBool] returns true if the fault object is recognised
**
** @release 6.4.0
** @@
******************************************************************************/

AjBool ajSoapAxis2Error(axiom_node_t *fault, const axutil_env_t *env)
{
    axis2_char_t* faultcode = NULL;
    axis2_char_t* faultmsg  = NULL;
    axis2_char_t* exception = NULL;
    axiom_element_t* parent = NULL;
    axiom_element_t* child  = NULL;
    axiom_node_t* node      = NULL;
    axutil_qname_t* qname   = NULL;

    parent = axiom_node_get_data_element(fault, env);

    if (!parent)
	return ajFalse;

    /* fault->faultcode */
    qname = axutil_qname_create(env, "faultcode", NULL, "");
    child = axiom_element_get_first_child_with_qname(parent, env, qname,
                                                     fault, &node);
    if (child)
	faultcode = axiom_element_get_text(child, env, node);

    axutil_qname_free(qname, env);

    /* fault->faultstring */
    qname = axutil_qname_create(env, "faultstring", NULL, "");
    child = axiom_element_get_first_child_with_qname(parent, env, qname,
                                                     fault, &node);
    if (child)
	faultmsg = axiom_element_get_text(child, env, node);

    axutil_qname_free(qname, env);

    /* fault->detail */
    qname = axutil_qname_create(env, "detail", NULL, "");
    parent = axiom_element_get_first_child_with_qname(parent, env, qname,
                                                      fault, &node);
    axutil_qname_free(qname, env);

    if(!parent)
	return ajFalse;

    /* fault->detail->exceptionName */
    qname = axutil_qname_create(env, "exceptionName", AXIS_NS , "");
    child = axiom_element_get_first_child_with_qname(parent, env, qname,
                                                     node, &node);
    if(child)
	exception = axiom_element_get_text(child, env, node);


    if(!strstr(exception, "DbfNoEntryFoundException"))
	ajErr("webservices error: %s, %s %s", faultcode, faultmsg, exception);

    axutil_qname_free(qname, env);

    return ajTrue;
}




/* @func ajSoapAxis2GetEnv ****************************************************
**
** Returns an axis2 environment object for webservices calls
**
** @return [axutil_env_t*] axis2 environment object
**
** @release 6.4.0
** @@
******************************************************************************/

axutil_env_t* ajSoapAxis2GetEnv(void)
{
    axutil_env_t* env = NULL;
    AjPStr ax2log     = NULL;

    ax2log = ajStrNew();

    /* If debug mode is on, an extra log-file with .ax2log extension
     * is created in current working directory. Otherwise axis2 log
     * messages are not recorded */

    ajFmtPrintS(&ax2log, "%s%S.ax2log", CURRENT_DIR, ajUtilGetProgram());

    if(ajDebugOn())
	env = axutil_env_create_all(ajStrGetPtr(ax2log),
	                            AXIS2_LOG_LEVEL_DEBUG);
    else
	env = axutil_env_create_all(_BLACKHOLE,
	                            AXIS2_LOG_LEVEL_ERROR);

    ajStrDel(&ax2log);

    return env;
}


#endif
