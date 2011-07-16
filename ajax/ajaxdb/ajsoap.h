#ifdef __cplusplus
extern "C"
{
#endif

#ifndef ajsoap_h
#define ajsoap_h

#ifdef HAVE_AXIS2C
#include <axiom.h>
#include <axis2_util.h>
#include <axiom_soap.h>
#include <axis2_client.h>
#include <axutil_qname.h>


/*
 * default end-points and name-spaces for EBI web-services
 */

#define WSDBFETCH_NS "http://www.ebi.ac.uk/ws/services/WSDbfetchDoclit"
#define WSDBFETCH_EP "http://www.ebi.ac.uk/ws/services/WSDbfetchDoclit"
#define EBEYE_NS     "http://www.ebi.ac.uk/EBISearchService"
#define EBEYE_EP     "http://www.ebi.ac.uk/ebisearch/service.ebi"

#define UNRECOGNISEDXML "cannot parse webservices results"





axiom_node_t* 	    ajSoapAxis2Call(axis2_svc_client_t *client,
              	                    const axutil_env_t *env,
              	                    axiom_node_t *payload);

AjBool              ajSoapAxis2Error(axiom_node_t *fault,
				     const axutil_env_t *env);


axis2_svc_client_t* ajSoapAxis2GetClient(const axutil_env_t *env,
                                         const axis2_char_t *address);

axutil_env_t*       ajSoapAxis2GetEnv(void);


#endif


#endif




#ifdef __cplusplus
}
#endif
