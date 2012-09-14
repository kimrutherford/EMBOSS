/* @source ajhttp *************************************************************
**
** AJAX HTTP (database) functions
**
** These functions control all aspects of AJAX http access
** via SEND/GET/POST protocols
**
** @author Copyright (C) 2010 Alan Bleasby
** @version $Revision: 1.24 $
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


#include "ajlib.h"

#include "ajhttp.h"
#include "ajsys.h"
#include "ajfile.h"
#include "ajreg.h"
#include "ajutil.h"
#include "ajnam.h"
#include "ajfileio.h"

#include <limits.h>
#include <stdarg.h>
#include <sys/types.h>
#ifndef WIN32
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include <netdb.h>

#include <dirent.h>
#include <unistd.h>
#else
#include <winsock2.h>
#include <ws2tcpip.h>
#endif
#include <errno.h>
#include <signal.h>




static FILE* httpSend(const AjPStr dbname,
                      AjOSysSocket sock,
                      const AjPStr host, ajint iport,
                      const AjPStr proxyauth, const AjPStr proxycreds,
                      const AjPStr get);




/* @func ajHttpGetProxyinfo ***************************************************
**
** Returns a proxy definition (if any). Any proxy string given as an
** argument overrides any EMBOSS_PROXY definition. This
** allows strings from DB definitions to override such an envvar.
**
** @param [r] dbproxy [const AjPStr] Primary proxy string (if any)
** @param [w] proxyport [ajint*] Proxy port
** @param [w] proxyname [AjPStr*] Proxy name
** @param [w] proxyauth [AjPStr*] Proxy authentication type (if any)
** @param [w] proxycreds [AjPStr*] Proxy auth credentials (if any)
** @return [AjBool] ajTrue if a proxy was defined
**
** @release 6.4.0
** @@
******************************************************************************/

AjBool ajHttpGetProxyinfo(const AjPStr dbproxy, ajint* proxyport,
                          AjPStr* proxyname, AjPStr* proxyauth,
                          AjPStr* proxycreds)
{
    AjPStr proxy     = NULL;
    AjPStr portStr   = NULL;
    const char *p    = NULL;
    const char *q    = NULL;
    AjPStrTok handle = NULL;

    AjPStr serv  = NULL;
    AjPStr token = NULL;
    AjBool ret = ajFalse;
    

    ajStrDel(proxyname);
    *proxyport = 0;
    ajStrDel(proxyauth);
    ajStrDel(proxycreds);

    
    ajNamGetValueC("proxy", &proxy);

    if(ajStrGetLen(dbproxy))
	ajStrAssignS(&proxy, dbproxy);


    token = ajStrNew();
    
    ajStrTokenAssignC(&handle, proxy, " \n\r\t,");
    ret = ajStrTokenNextParse(&handle, &token);
    if(!ret || ajStrMatchC(token, ":"))
    {
        ajStrDel(&proxy);
        ajStrDel(&token);
        ajStrTokenDel(&handle);

        return ajFalse;
    }


    serv    = ajStrNew();
    portStr = ajStrNew();

    ajStrAssignS(&serv, token);

    if(ajStrMatchC(serv, ":"))
	ajStrAssignClear(&serv);


    
    p = ajStrGetPtr(serv);
    if((q = strchr(p, (int)':')) && p != q)
    {
        ajStrAssignSubC(proxyname,p,0,q-p-1);
        ajStrAssignC(&portStr,q+1);
    }
    
    
    if(!ajStrGetLen(*proxyname) || !ajStrGetLen(portStr))
    {
        ajStrTokenDel(&handle);
        ajStrDel(&token);
        ajStrDel(&serv);
        ajStrDel(&proxy);
        ajStrDel(&portStr);
        
        return ajFalse;
    }

    ajStrToInt(portStr, proxyport);

    ajStrDel(&portStr);
    ajStrDel(&serv);

    /* Check for authentication type */

    ret = ajStrTokenNextParse(&handle, &token);
    if(!ret)
    {
        ajStrTokenDel(&handle);
        ajStrDel(&token);
        ajStrDel(&proxy);
        
        return ajTrue;
    }
        
    ajStrAssignS(proxyauth, token);
    
    /* Check for authentication credentials */

    ret = ajStrTokenNextParse(&handle, &token);
    if(!ret)
    {
        ajWarn("ajHttpGetProxyinfo: No credentials specified in proxy "
               "definition (%S)",proxy);
        ajStrTokenDel(&handle);
        ajStrDel(&token);
        ajStrDel(&proxy);
        
        return ajFalse;
    }
    
    ajStrAssignS(proxycreds,token);
    
    ajStrTokenDel(&handle);
    ajStrDel(&token);
    ajStrDel(&proxy);
    
    return ajTrue;
}




/* @func ajHttpGetVersion *****************************************************
**
** Returns the HTTP version. Any supplied version takes precedence over
** an EMBOSS_HTTPVERSION definition so allowing DB entries to
** override such a setting.
**
** @param [r] version [const AjPStr] Version or NULL (or zero-length string)
** @param [w] httpver [AjPStr*] HTTP version
** @return [AjBool] ajTrue if a version was defined
**
** @release 6.4.0
** @@
******************************************************************************/

AjBool ajHttpGetVersion(const AjPStr version, AjPStr* httpver)
{
    ajNamGetValueC("httpversion", httpver);
    ajDebug("httpver getValueC '%S'\n", *httpver);

    if(ajStrGetLen(version))
	ajStrAssignS(httpver, version);

    ajDebug("httpver after qry '%S'\n", *httpver);

    if(!ajStrGetLen(*httpver))
    {
	ajStrAssignC(httpver, "1.1");

	return ajFalse;
    }

    if(!ajStrIsFloat(*httpver))
    {
	ajWarn("Invalid HTTPVERSION '%S', reset to 1.1", *httpver);
	ajStrAssignC(httpver, "1.1");

	return ajFalse;
    }

    ajDebug("httpver final '%S'\n", *httpver);

    return ajTrue;
}




/* @func ajHttpOpen ***********************************************************
**
** Opens an HTTP connection
**
** @param [r] dbname [const AjPStr] Database name (for error reporting)
** @param [r] host [const AjPStr] Host name
** @param [r] iport [ajint] Port
** @param [r] get [const AjPStr] GET string
** @param [u] Psock [AjPSysSocket] Socket returned to caller
** @return [FILE*] Open file on success, NULL on failure
**
** @release 6.4.0
** @@
******************************************************************************/

FILE* ajHttpOpen(const AjPStr dbname, const AjPStr host, ajint iport,
                 const AjPStr get, AjPSysSocket Psock)
{
    FILE* fp;
    struct addrinfo hints;
    struct addrinfo *add = NULL;
    struct addrinfo *addinit = NULL;

    AjPStr portstr = NULL;

    const char *phost = NULL;
    const char *pport = NULL;

    AjOSysSocket sock = *Psock;

    AjPStr errstr = NULL;
    int ret;

    phost = ajStrGetPtr(host);
    ajDebug("ajHttpOpen db: '%S' host '%S' port: %u get: '%S'\n",
	    dbname, host, iport, get);

    memset(&hints,0,sizeof(hints));

    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;

    portstr = ajStrNew();
    ajFmtPrintS(&portstr,"%d",iport);
    pport =  ajStrGetPtr(portstr);

    ret = getaddrinfo(phost, pport, &hints, &addinit);

    ajStrDel(&portstr);
    
    if(ret)
    {
	ajErr("[%s] Failed to find host '%S' for database '%S'",
	      gai_strerror(ret), host, dbname);

	return NULL;
    }

    sock.sock = AJBADSOCK;
    
    for(add = addinit; add; add = add->ai_next)
    {
        sock.sock = ajSysFuncSocket(add->ai_family, add->ai_socktype,
                                    add->ai_protocol);

        if(sock.sock == AJBADSOCK)
            continue;

        if(connect(sock.sock, add->ai_addr, add->ai_addrlen))
        {
            ajSysSocketclose(sock);
            sock.sock = AJBADSOCK;
            continue;
        }

        break;
    }

    freeaddrinfo(addinit);

    if(sock.sock == AJBADSOCK)
    {
	ajDebug("Socket connect failed\n");
	ajFmtPrintS(&errstr, "socket connect failed for database '%S'",
		    dbname);
	ajErr("%S", errstr);
	perror(ajStrGetPtr(errstr));
	ajStrDel(&errstr);

	return NULL;
    }

    fp = httpSend(dbname, sock, host, iport, NULL, NULL, get);

    return fp;
}




/* @func ajHttpOpenProxy ******************************************************
**
** Opens an HTTP connection via a proxy
**
** @param [r] dbname [const AjPStr] Databse name (for error reporting)
** @param [r] proxyname [const AjPStr] Proxy name
** @param [r] proxyport [ajint] Proxy port
** @param [r] proxyauth [const AjPStr] Proxy auth type (if any)
** @param [r] proxycreds [const AjPStr] Proxy auth credentials (if any)
** @param [r] host [const AjPStr] Host name
** @param [r] iport [ajint] Port
** @param [r] get [const AjPStr] GET string
** @param [u] Psock [AjPSysSocket] Socket returned to caller
** @return [FILE*] Open file on success, NULL on failure
**
** @release 6.4.0
** @@
******************************************************************************/

FILE* ajHttpOpenProxy(const AjPStr dbname, const AjPStr proxyname,
                      ajint proxyport, const AjPStr proxyauth,
                      const AjPStr proxycreds, const AjPStr host,
                      ajint iport, const AjPStr get, AjPSysSocket Psock)
{
    FILE* fp;
    struct addrinfo hints;
    struct addrinfo *add;
    struct addrinfo *addinit;

    AjPStr portstr = NULL;

    const char *phost = NULL;
    const char *pport = NULL;

    AjOSysSocket sock = *Psock;
    
    AjPStr errstr = NULL;
    int ret;

    phost = ajStrGetPtr(proxyname);
    ajDebug("ajHttpOpenProxy db: '%S' host '%s' get: '%S'\n",
	    dbname, phost, get);

    memset(&hints,0,sizeof(hints));

    hints.ai_socktype = SOCK_STREAM;

    portstr = ajStrNew();
    ajFmtPrintS(&portstr,"%d",proxyport);
    pport =  ajStrGetPtr(portstr);
    
    ret = getaddrinfo(phost, pport, &hints, &addinit);


    
    if(ret)
    {
	ajErr("[%s] Failed to find host '%S' for service '%S'",
	      gai_strerror(ret), proxyname, portstr);
        ajStrDel(&portstr);

	return NULL;
    }

    ajStrDel(&portstr);

    sock.sock = AJBADSOCK;
    
    for(add = addinit; add; add = add->ai_next)
    {
        sock.sock = ajSysFuncSocket(add->ai_family, add->ai_socktype,
                                    add->ai_protocol);

        if(sock.sock == AJBADSOCK)
            continue;

        if(connect(sock.sock, add->ai_addr, add->ai_addrlen))
        {
            ajSysSocketclose(sock);
            sock.sock = AJBADSOCK;
            continue;
        }

        break;
    }

    freeaddrinfo(addinit);
    
    if(sock.sock == AJBADSOCK)
    {
	ajDebug("Socket connect failed\n");
	ajFmtPrintS(&errstr, "socket connect failed for database '%S'",
		    dbname);
	ajErr("%S", errstr);
	perror(ajStrGetPtr(errstr));
	ajStrDel(&errstr);

	return NULL;
    }

    fp = httpSend(dbname, sock, host, iport, proxyauth, proxycreds, get);

    return fp;
}




/* @funcstatic httpSend *******************************************************
**
** Send HTTP GET request to an open socket
**
** @param [r] dbname [const AjPStr] Database name (for error reporting)
** @param [u] sock [AjOSysSocket] Socket structure
** @param [r] host [const AjPStr] Host name for Host header line
** @param [r] iport [ajint] Port for Host header line
** @param [r] proxyauth [const AjPStr] Proxy auth type (if any)
** @param [r] proxycreds [const AjPStr] Proxy auth credentials (if any)
** @param [r] get [const AjPStr] GET string
** @return [FILE*] Open file on success, NULL on failure
**
** @release 6.4.0
** @@
******************************************************************************/

static FILE* httpSend(const AjPStr dbname,
                      AjOSysSocket sock,
                      const AjPStr host, ajint iport,
                      const AjPStr proxyauth, const AjPStr proxycreds,
                      const AjPStr get)
{
    FILE* fp       = NULL;
    AjPStr gethead = NULL;
    AjPStr cred = NULL;
    
    ajint isendlen;

    ajDebug("httpSend: Sending to socket\n");

    gethead = ajStrNew();


    isendlen = send(sock.sock, ajStrGetPtr(get), ajStrGetLen(get), 0);

    if(isendlen < 0 || isendlen != (ajint) ajStrGetLen(get))
	ajErr("send failure, expected %u bytes returned %d : %s\n",
	      ajStrGetLen(get), isendlen, ajMessGetSysmessageC());

    ajDebug("sending: '%S'\n", get);
    if(isendlen < 0)
            ajDebug("send for GET errno %d msg '%s'\n",
                    errno, ajMessGetSysmessageC());

    if(ajStrGetLen(proxyauth))
    {
        if(!ajStrMatchCaseC(proxyauth,"Basic"))
            ajErr("Only 'Basic' proxy authentication currently implemented,\n"
                  "no 'Digest' or 'NTLM' [%S]",proxyauth);

        cred = ajStrNew();
        
        ajUtilBase64EncodeC(&cred, ajStrGetLen(proxycreds),
                            (const unsigned char *) ajStrGetPtr(proxycreds));

        ajFmtPrintS(&gethead,"Proxy-Authorization: Basic %S\r\n",cred);

        isendlen =  send(sock.sock, ajStrGetPtr(gethead),
                         ajStrGetLen(gethead), 0);

        if(isendlen < 0 || isendlen != (ajint) ajStrGetLen(gethead))
            ajErr("send failure, expected %u bytes returned %d : %s\n",
                  ajStrGetLen(gethead), isendlen, ajMessGetSysmessageC());
        ajDebug("sending: '%S'\n", gethead);
        if(isendlen < 0)
            ajDebug("send for host errno %d msg '%s'\n",
                    errno, ajMessGetSysmessageC());

        ajStrDel(&cred);
    }



    ajFmtPrintS(&gethead, "User-Agent: EMBOSS/%S (%S)\r\n",
                ajNamValueVersion(), ajNamValueSystem());
    isendlen = send(sock.sock, ajStrGetPtr(gethead), ajStrGetLen(gethead), 0);

    if(isendlen < 0 || isendlen != (ajint) ajStrGetLen(gethead))
	ajErr("send failure, expected %u bytes returned %d : %s\n",
	      ajStrGetLen(gethead), isendlen, ajMessGetSysmessageC());
    ajDebug("sending: '%S'\n", gethead);

    ajFmtPrintS(&gethead, "Host: %S:%d\r\n", host, iport);
    isendlen =  send(sock.sock, ajStrGetPtr(gethead), ajStrGetLen(gethead), 0);

    if(isendlen < 0 || isendlen != (ajint) ajStrGetLen(gethead))
	ajErr("send failure, expected %u bytes returned %d : %s\n",
	      ajStrGetLen(gethead), isendlen, ajMessGetSysmessageC());
    ajDebug("sending: '%S'\n", gethead);
    if(isendlen < 0)
            ajDebug("send for host errno %d msg '%s'\n",
                    errno, ajMessGetSysmessageC());

    if(ajStrFindC(get,"HTTP/1.1") != -1)
    {
        ajFmtPrintS(&gethead, "Connection: close\r\n");
        isendlen =  send(sock.sock, ajStrGetPtr(gethead),ajStrGetLen(gethead),
                         0);

        if(isendlen < 0 || isendlen != (ajint) ajStrGetLen(gethead))
            ajErr("send failure, expected %u bytes returned %d : %s\n",
                  ajStrGetLen(gethead), isendlen, ajMessGetSysmessageC());

        ajDebug("sending: '%S'\n", gethead);
        if(isendlen < 0)
            ajDebug("send for host errno %d msg '%s'\n",
                    errno, ajMessGetSysmessageC());
    }

    ajFmtPrintS(&gethead, "\r\n");
    isendlen =  send(sock.sock, ajStrGetPtr(gethead), ajStrGetLen(gethead), 0);

    if(isendlen < 0 || isendlen != (ajint) ajStrGetLen(gethead))
	ajErr("send failure, expected %u bytes returned %d : %s\n",
	      ajStrGetLen(gethead), isendlen, ajMessGetSysmessageC());
    ajDebug("sending: '%S'\n", gethead);
    if(isendlen < 0)
        ajDebug("send for blankline errno %d msg '%s'\n",
                errno, ajMessGetSysmessageC());

    ajStrDel(&gethead);


    fp = ajSysFdFromSocket(sock, "r");
    
    if(!fp)
    {
	ajDebug("httpSend socket open failed\n");
	ajErr("httpSend: socket open failed for database '%S'", dbname);

	return NULL;
    }

    return fp;
}




/* @func ajHttpUrlrefNew ******************************************************
**
** Initialise a URL components object
**
** @return [AjPUrlref] URL Components
**
** @release 6.4.0
******************************************************************************/

AjPUrlref ajHttpUrlrefNew(void)
{
    AjPUrlref ret = NULL;

    AJNEW0(ret);
    
    ret->Method   = ajStrNew();
    ret->Host     = ajStrNew();
    ret->Port     = ajStrNew();
    ret->Absolute = ajStrNew();
    ret->Relative = ajStrNew();
    ret->Fragment = ajStrNew();
    ret->Username = ajStrNew();
    ret->Password = ajStrNew();

    return ret;
}




/* @func ajHttpUrlrefDel ******************************************************
**
** Delete URL components object
**
** @param [u] thys [AjPUrlref*] URL components object
** @return [void]
**
** @release 6.4.0
******************************************************************************/

void ajHttpUrlrefDel(AjPUrlref *thys)
{
    AjPUrlref pthis = NULL;

    if(!thys)
        return;

    if(!*thys)
        return;

    pthis = *thys;

    ajStrDel(&pthis->Method);
    ajStrDel(&pthis->Host);
    ajStrDel(&pthis->Port);
    ajStrDel(&pthis->Absolute);
    ajStrDel(&pthis->Relative);
    ajStrDel(&pthis->Fragment);
    ajStrDel(&pthis->Username);
    ajStrDel(&pthis->Password);

    AJFREE(pthis);
    
    *thys = NULL;
    
    return;
}




/* @func ajHttpUrlrefParseC ***************************************************
**
** Parse an IPV4/6 URL into its components
**
** @param [u] parts [AjPUrlref*] URL components object
** @param [u] url [const char*] URL
** @return [void]
**
** @release 6.4.0
******************************************************************************/

void ajHttpUrlrefParseC(AjPUrlref *parts, const char *url)
{
    char *ucopy = NULL;
    char *p = NULL;
    char *post = NULL;

    char *dest = NULL;
    char *src  = NULL;
    
    AjPUrlref comp = NULL;
    
    char *pmethod = NULL;
    char *phost   = NULL;
    char *pabs    = NULL;
    char *prel    = NULL;
    
    if(!parts || !url)
        return;
    
    if(!*parts)
        return;
    
    ucopy = ajCharNewC(url);

    post = ucopy;
    comp = *parts;
    
    /* Get any fragment */
    if ((p = strchr(ucopy, '#')))
    {
	*p++ = '\0';
        ajStrAssignC(&comp->Fragment,p);
    }

    if((p = strchr(ucopy, ' ')))
        *p++ = '\0';


    for(p = ucopy; *p; ++p)
    {
	if (isspace((int) *p))
        {
            dest = p;
            src  = p+1;
            
	    while ((*dest++ = *src++));

	    p = p-1;
	}

	if (*p == '/' || *p == '#' || *p == '?')
	    break;

	if (*p == ':')
        {
		*p = '\0';
                pmethod = post;
		post = p+1;

                if(ajCharPrefixCaseC(pmethod,"URL"))
                    pmethod = NULL;
                else
                    break;
	}
    }

    p = post;

    if(*p == '/')
    {
	if(p[1] == '/')
        {
	    phost = p+2;		/* There is a host   */
	    *p = '\0';
	    p = strchr(phost,'/');	/* Find end of host */

	    if(p)
            {
	        *p=0;			/* and terminate it */
	        pabs = p+1;		/* Path found */
	    }
	}
        else
	    pabs = p+1;		/* Path found but not host */
    }
    else
        prel = (*post) ? post : NULL;


    if(pmethod)
        ajStrAssignC(&comp->Method,pmethod);

    if(phost)
        ajStrAssignC(&comp->Host,phost);

    if(pabs)
        ajStrAssignC(&comp->Absolute,pabs);

    if(prel)
        ajStrAssignC(&comp->Relative,prel);

    AJFREE(ucopy);

    return;
}




/* @func ajHttpUrlrefSplitPort ************************************************
**
** Separate any port from a host specification (IPV4/6)
**
** @param [u] urli [AjPUrlref] URL components object
** @return [void]
**
** @release 6.4.0
******************************************************************************/

void ajHttpUrlrefSplitPort(AjPUrlref urli)
{
    const char *p   = NULL;
    const char *end = NULL;
    const char *start = NULL;
    
    ajint len;

    if(!(len = ajStrGetLen(urli->Host)))
        return;

    end = (start = ajStrGetPtr(urli->Host)) + len - 1;

    p = end;

    if(!isdigit((int) *p))
        return;

    while(isdigit((int) *p) && p != start)
        --p;

    if(p == start)
        return;

    if(*p != ':')
        return;
    
    ajStrAssignC(&urli->Port,p+1);

    ajStrAssignSubC(&urli->Host,start,0,p-start-1);
    
    return;
}




/* @func ajHttpUrlrefParseS ***************************************************
**
** Parse an IPV4/6 URL into its components
**
** @param [u] parts [AjPUrlref*] URL components object
** @param [r] surl [const AjPStr] URL
** @return [void]
**
** @release 6.5.0
******************************************************************************/

void ajHttpUrlrefParseS(AjPUrlref *parts, const AjPStr surl)
{
    ajHttpUrlrefParseC(parts, MAJSTRGETPTR(surl));
}




/* @func ajHttpUrlrefSplitUsername ********************************************
**
** Separate any username[:password] from a host specification (IPV4/6)
**
** @param [u] urli [AjPUrlref] URL components object
** @return [void]
**
** @release 6.4.0
******************************************************************************/

void ajHttpUrlrefSplitUsername(AjPUrlref urli)
{
    const char *p   = NULL;
    const char *end = NULL;
    AjPStr userpass = NULL;
    AjPStr host = NULL;
    
    ajint len;

    if(!ajStrGetLen(urli->Host))
        return;

    if(!(end = strchr(ajStrGetPtr(urli->Host), (int)'@')))
        return;

    p = ajStrGetPtr(urli->Host);
    len = end - p;

    if(!len)
        return;
    
    userpass = ajStrNew();
    ajStrAssignSubC(&userpass, p, 0, end - p - 1);
    
    host = ajStrNew();
    ajStrAssignC(&host,end + 1);
    ajStrAssignS(&urli->Host,host);

    

    if(!(end = strchr(ajStrGetPtr(userpass), (int)':')))
    {
        ajStrAssignS(&urli->Username,userpass);
        ajStrDel(&userpass);
        ajStrDel(&host);

        return;
    }

    p = ajStrGetPtr(userpass);
    len = end - p;

    if(!len)
        ajWarn("ajHttpUrlrefSplitUsername: Missing username in URL [%S@%S]",
               userpass,host);
    else
        ajStrAssignSubC(&urli->Username,p,0,len - 1);

    ajStrAssignC(&urli->Password, end + 1);

    ajStrDel(&userpass);
    ajStrDel(&host);

    return;
}




/* @func ajHttpQueryUrl *******************************************************
**
** Returns the components of a URL (IPV4/6)
** An equivalent for seqHttpUrl().
**
** @param [r] qry [const AjPQuery] Database query
** @param [w] iport [ajint*] Port
** @param [w] host [AjPStr*] Host name
** @param [w] urlget [AjPStr*] URL for the HTTP header GET
** @return [AjBool] ajTrue if the URL was parsed
**
** @release 6.4.0
** @@
******************************************************************************/

AjBool ajHttpQueryUrl(const AjPQuery qry, ajint* iport, AjPStr* host,
                      AjPStr* urlget)
{
    const AjPStr url = NULL;
    AjPUrlref uo = NULL;
    
    url = qry->DbUrl;
    if(!url)
    {
	ajErr("no URL defined for database %S", qry->DbName);

	return ajFalse;
    }

    uo = ajHttpUrlrefNew();
    
    ajHttpUrlrefParseC(&uo, ajStrGetPtr(url));
    ajHttpUrlrefSplitPort(uo);
    
    ajStrAssignS(host,uo->Host);
    ajFmtPrintS(urlget,"/%S",uo->Absolute);

    if(ajStrGetLen(uo->Port))
        ajStrToInt(uo->Port,iport);

    ajHttpUrlrefDel(&uo);

    return ajTrue;
}




/* @func ajHttpUrlDeconstruct *************************************************
**
** Deconstruct a URL (IPV4/6)
**
** @param [r] url [const AjPStr] url
** @param [w] iport [ajint*] Port
** @param [w] host [AjPStr*] Host name
** @param [w] urlget [AjPStr*] URL for the HTTP header GET
** @return [void]
**
** @release 6.4.0
** @@
******************************************************************************/

void ajHttpUrlDeconstruct(const AjPStr url, ajint* iport, AjPStr* host,
                          AjPStr* urlget)
{
    AjPUrlref uo = NULL;
    
    uo = ajHttpUrlrefNew();
    
    ajHttpUrlrefParseC(&uo, ajStrGetPtr(url));
    ajHttpUrlrefSplitPort(uo);
    
    ajStrAssignS(host,uo->Host);
    ajFmtPrintS(urlget,"/%S",uo->Absolute);

    if(ajStrGetLen(uo->Port))
        ajStrToInt(uo->Port,iport);

    ajHttpUrlrefDel(&uo);

    return;
}




/* @func ajHttpRedirect *******************************************************
**
** Reads the header of http response in given buffer buff,
** if it includes a redirection response updates the host, port and get
** parameters using the 'Location' header
**
** @param [u] buff [AjPFilebuff] file buffer
** @param [w] host [AjPStr*] Host name
** @param [w] port [ajint*] Port
** @param [w] path [AjPStr*] part of URL after port number
** @param [w] httpcode [ajuint*] HTTP protocol return code
** @return [AjBool] returns true if the header includes a redirection response
**
** @release 6.4.0
** @@
******************************************************************************/

AjBool ajHttpRedirect(AjPFilebuff buff, AjPStr* host, ajint* port, AjPStr* path,
                      ajuint *httpcode)
{

    AjPRegexp httpexp  = NULL;
    AjPRegexp nullexp  = NULL;
    AjPRegexp redirexp = NULL;

    AjPStr codestr  = NULL;
    AjPStr newurl   = NULL;
    AjPStr newhost  = NULL;
    AjPStr currline = NULL;

    AjBool isheader = ajFalse;
    AjBool ret = ajFalse;

    httpexp  = ajRegCompC("^HTTP/\\S+\\s+(\\d+)");

    if(!buff->Size)
	return 0;

    ajBuffreadLine(buff, &currline);

    ajDebug("ajHttpRedirect: First line: '%S'\n", currline);

    if(ajRegExec(httpexp, currline))
    {
        isheader = ajTrue;
        ajRegSubI(httpexp, 1, &codestr);
        ajStrToUint(codestr, httpcode);
        ajDebug("Header: codestr '%S' code '%u'\n", codestr, *httpcode);
        ajStrDel(&codestr);
    }

    if(isheader)
    {
        if(*httpcode == 301 || *httpcode == 302 || *httpcode==307)
        {
            redirexp = ajRegCompC("^Location: (\\S+)");
            nullexp  = ajRegCompC("^\r?\n?$");

            while( ajBuffreadLine(buff, &currline) &&
                  !ajRegExec(nullexp, currline))
            {
        	ajDebug("ajHttpRedirect: header line: '%S'\n", currline);

        	if(ajRegExec(redirexp, currline))
                {
                    ajRegSubI(redirexp, 1, &newurl);
                    ajHttpUrlDeconstruct(newurl, port, &newhost, path);

                    if(ajStrGetLen(newhost))
                	ajStrAssignS(host, newhost);

                    ajStrDel(&newurl);
                    ajStrDel(&newhost);
                    ret = ajTrue;
                    break;
                }
            }

            ajRegFree(&redirexp);
            ajRegFree(&nullexp);
        }
    }

    if(!ret)
	ajFilebuffReset(buff);

    ajRegFree(&httpexp);
    ajStrDel(&currline);

    return ret;
}




/* @func ajHttpRead ***********************************************************
**
** Reads the header of http response in given buffer buff,
** if it includes a redirection response updates the host, port and get
** parameters using the 'Location' header
**
** @param [r] dbhttpver [const AjPStr] DB http version
** @param [r] dbname [const AjPStr] DB name
** @param [r] dbproxy [const AjPStr] DB proxy
** @param [r] host [const AjPStr] Host name
** @param [r] port [ajint] Port
** @param [r] dbpath [const AjPStr] part of URL after port number
** @return [AjPFilebuff] http response
**
** @release 6.4.0
** @@
******************************************************************************/

AjPFilebuff ajHttpRead(const AjPStr dbhttpver, const AjPStr dbname,
                       const AjPStr dbproxy, const AjPStr host,
                       ajint port, const AjPStr dbpath)
{
    return ajHttpReadPos(dbhttpver, dbname, dbproxy, host, port, dbpath, 0L);
}




/* @func ajHttpReadPos ********************************************************
**
** Reads the header of http response in given buffer buff,
** if it includes a redirection response updates the host, port and get
** parameters using the 'Location' header
**
** @param [r] dbhttpver [const AjPStr] DB http version
** @param [r] dbname [const AjPStr] DB name
** @param [r] dbproxy [const AjPStr] DB proxy
** @param [r] host [const AjPStr] Host name
** @param [r] port [ajint] Port
** @param [r] dbpath [const AjPStr] part of URL after port number
** @param [r] fpos [ajlong] File start offset
** @return [AjPFilebuff] http response
**
** @release 6.4.0
** @@
******************************************************************************/

AjPFilebuff ajHttpReadPos(const AjPStr dbhttpver, const AjPStr dbname,
                          const AjPStr dbproxy, const AjPStr host,
                          ajint port, const AjPStr dbpath, ajlong fpos)
{
    AjPStr get       = NULL;
    AjPStr httpver   = NULL;
    AjPStr proxyname = NULL;
    AjPStr proxyauth = NULL;
    AjPStr proxycred = NULL;
    AjPStr newhost   = NULL;
    AjPStr path      = NULL;
    AjPFilebuff buff = NULL;
    FILE *fp         = NULL;

    AjOSysSocket sock;
    AjOSysTimeout timo;
    ajint proxyport = 0;
    ajuint httpcode = 0;

    httpver   = ajStrNew();
    proxyname = ajStrNew();
    proxyauth = ajStrNew();
    proxycred = ajStrNew();
    get       = ajStrNew();
    newhost   = ajStrNew();

    ajDebug("ajHttpRead db: '%S' host '%S' port: %u dbpath: '%S'\n",
	    dbname, host, port, dbpath);

    ajStrAssignS(&newhost,host);
    ajStrAssignS(&path, dbpath);


    ajHttpGetVersion(dbhttpver, &httpver);

    ajHttpGetProxyinfo(dbproxy, &proxyport, &proxyname, &proxyauth, &proxycred);


    while (buff==NULL || ajHttpRedirect(buff, &newhost, &port, &path,
                                        &httpcode))
    {
	if(buff) /* means buff includes http redirect response*/
	    ajFilebuffDel(&buff);

	if(ajStrGetCharFirst(path)!='/')
	    ajStrInsertK(&path, 0, '/');

	if(ajStrGetLen(proxyname))
	{
	    ajFmtPrintS(&get, "GET http://%S:%d%S HTTP/%S\r\n",
	                host, port, path, httpver);
	    fp = ajHttpOpenProxy(dbname, proxyname, proxyport, proxyauth,
	                         proxycred, newhost, port, get, &sock);
	}
	else
	{
	    ajFmtPrintS(&get, "GET %S HTTP/%S\r\n", path, httpver);
	    if(fpos)
                ajFmtPrintAppS(&get, "Range: bytes=%Ld-\r\n", fpos);

	    fp = ajHttpOpen(dbname, newhost, port, get, &sock);
	}

	if(!fp)
	{
	    ajErr("ajHttpRead: cannot open HTTP connection '%S'\n", get);
	    buff = NULL;
	    break;
	}

	buff = ajFilebuffNewFromCfile(fp);

	if(!buff)
	{
	    ajErr("ajHttpRead: socket buffer attach failed for host '%S'"
		    ", HTTP get command was '%S'", host, get);
	    break;
	}

	timo.seconds = 180;
	ajSysTimeoutSet(&timo);
	ajFilebuffLoadAll(buff);
	ajSysTimeoutUnset(&timo);
    }

    if(httpcode >= 400)
    {
        ajFilebuffDel(&buff);
    }

    ajStrDel(&get);
    ajStrDel(&httpver);
    ajStrDel(&proxyname);
    ajStrDel(&newhost);
    ajStrDel(&path);

    return buff;
}
