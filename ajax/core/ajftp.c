/* @source ajftp **************************************************************
**
** AJAX FTP (database) functions
**
** These functions control all aspects of AJAX ftp access
**
** @author Copyright (C) 2010 Alan Bleasby
** @version $Revision: 1.5 $
** @modified $Date: 2012/08/21 10:55:30 $ by $Author: rice $
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

#include "ajftp.h"
#include "ajsys.h"
#include "ajfile.h"
#include "ajreg.h"
#include "ajutil.h"
#include "ajnam.h"
#include "ajfileio.h"
#include "ajhttp.h"

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



static FILE* ftpSend(const AjPStr dbname,
                     AjOSysSocket sock,
                     const AjPStr get,
                     ajlong fpos);
static AjBool ftpResponse(AjPFile ftpfile, AjPStr *Presponse);




/* @func ajFtpOpen ************************************************************
**
** Opens an FTP connection
**
** @param [r] dbname [const AjPStr] Database name (for error reporting)
** @param [r] host [const AjPStr] Host name
** @param [r] iport [ajint] Port
** @param [r] get [const AjPStr] GET string
** @param [r] fpos [ajlong] Start position offset
** @param [u] Psock [AjPSysSocket] Socket returned to caller
** @return [FILE*] Open file on success, NULL on failure
**
** @release 6.5.0
** @@
******************************************************************************/

FILE* ajFtpOpen(const AjPStr dbname, const AjPStr host, ajint iport,
                const AjPStr get, ajlong fpos, AjPSysSocket Psock)
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
    ajDebug("ajFtpOpen db: '%S' host '%S' port: %u get: '%S'\n",
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
	ajFmtPrintS(&errstr, "socket connect failed for database '%S': %s",
		    dbname, strerror(errno));
	ajErr("%S", errstr);
	ajStrDel(&errstr);

	return NULL;
    }


/*
** read welcome messages and other responses all with \r\n at the end of line
220- first line
220- middle line(s)
220 last line has no -
*/

    fp = ftpSend(dbname, sock, get, fpos);

    return fp;
}




/* @funcstatic ftpSend ********************************************************
**
** Send FTP request to an open socket
**
** @param [r] dbname [const AjPStr] Database name (for error reporting)
** @param [u] sock [AjOSysSocket] Socket structure
** @param [r] get [const AjPStr] GET string
** @param [r] fpos [ajlong] Start position offset
** @return [FILE*] Open file on success, NULL on failure
**
** @release 6.4.0
** @@
******************************************************************************/

static FILE* ftpSend(const AjPStr dbname,
                     AjOSysSocket sock,
                     const AjPStr get,
                     ajlong fpos)
{
    FILE* fp       = NULL;
    AjPStr ftpcmd = NULL;
    AjPStr ftpresponse = NULL;
    AjPStr pasvresponse = NULL;
    ajuint ia=0, ib=0, ic=0, id=0;
    ajuint ip1=0, ip2=0;
    ajuint iport=21;
    ajint ipos;
    ajint iscan;
    AjPStr hoststr = NULL;
    AjPStr portstr = NULL;
    AjPStr errstr = NULL;
    struct addrinfo hints;
    struct addrinfo *add = NULL;
    struct addrinfo *addinit = NULL;
    AjOSysSocket filesock;
    int ret;
    AjPFile ftpfile = NULL;

/*    AjPStr cred = NULL;*/
    
    ajint isendlen;

    ajDebug("ftpSend: Sending to socket '%S'\n", get);

    ftpcmd = ajStrNew();

/*
    if(ajStrGetLen(proxyauth))
    {
        if(!ajStrMatchCaseC(proxyauth,"Basic"))
            ajErr("Only 'Basic' proxy authentication currently implemented,\n"
                  "no 'Digest' or 'NTLM' [%S]",proxyauth);

        cred = ajStrNew();
        
        ajUtilBase64EncodeC(&cred, ajStrGetLen(proxycreds),
                            (const unsigned char *) ajStrGetPtr(proxycreds));

        ajFmtPrintS(&ftpcmd,"Proxy-Authorization: Basic %S\r\n",cred);

        isendlen =  send(sock.sock, ajStrGetPtr(ftpcmd),
                         ajStrGetLen(ftpcmd), 0);

        if(isendlen < 0 || isendlen != (ajint) ajStrGetLen(ftpcmd))
            ajErr("send failure, expected %u bytes returned %d : %s",
                  ajStrGetLen(ftpcmd), isendlen, ajMessGetSysmessageC());
        ajDebug("sending: '%S'\n", ftpcmd);
        if(isendlen < 0)
            ajDebug("send for host errno %d msg '%s'\n",
                    errno, ajMessGetSysmessageC());

        ajStrDel(&cred);
    }
*/

    ftpfile = ajFileNewFromCfile(ajSysFdFromSocket(sock, "r"));

    ftpResponse(ftpfile, &ftpresponse);

    ajFmtPrintS(&ftpcmd, "USER anonymous\r\n");
    isendlen =  send(sock.sock, ajStrGetPtr(ftpcmd),
                     ajStrGetLen(ftpcmd), 0);
    if(isendlen < 0 || isendlen != (ajint) ajStrGetLen(ftpcmd))
	ajErr("send failure, expected %u bytes returned %d : %s",
	      ajStrGetLen(ftpcmd), isendlen, ajMessGetSysmessageC());
    ajDebug("sending: '%S'\n", ftpcmd);
    ftpResponse(ftpfile, &ftpresponse);

    ajFmtPrintS(&ftpcmd, "PASS emboss-bug@emboss.open-bio.org\r\n");
    isendlen =  send(sock.sock, ajStrGetPtr(ftpcmd),
                     ajStrGetLen(ftpcmd), 0);
    if(isendlen < 0 || isendlen != (ajint) ajStrGetLen(ftpcmd))
	ajErr("send failure, expected %u bytes returned %d : %s",
	      ajStrGetLen(ftpcmd), isendlen, ajMessGetSysmessageC());
    ajDebug("sending: '%S'\n", ftpcmd);
    ftpResponse(ftpfile, &ftpresponse);


    ajFmtPrintS(&ftpcmd, "TYPE I\r\n");
    isendlen =  send(sock.sock, ajStrGetPtr(ftpcmd),
                     ajStrGetLen(ftpcmd), 0);
    if(isendlen < 0 || isendlen != (ajint) ajStrGetLen(ftpcmd))
	ajErr("send failure, expected %u bytes returned %d : %s",
	      ajStrGetLen(ftpcmd), isendlen, ajMessGetSysmessageC());
    ajDebug("sending: '%S'\n", ftpcmd);
    ftpResponse(ftpfile, &ftpresponse);

    ajFmtPrintS(&ftpcmd, "PASV\r\n");
    isendlen =  send(sock.sock, ajStrGetPtr(ftpcmd),
                     ajStrGetLen(ftpcmd), 0);
    if(isendlen < 0 || isendlen != (ajint) ajStrGetLen(ftpcmd))
	ajErr("send failure, expected %u bytes returned %d : %s",
	      ajStrGetLen(ftpcmd), isendlen, ajMessGetSysmessageC());
    ajDebug("sending: '%S'\n", ftpcmd);
    ftpResponse(ftpfile, &pasvresponse);

    ajFmtPrintS(&ftpcmd, "SIZE %S\r\n", get);
    isendlen =  send(sock.sock, ajStrGetPtr(ftpcmd),
                     ajStrGetLen(ftpcmd), 0);
    if(isendlen < 0 || isendlen != (ajint) ajStrGetLen(ftpcmd))
	ajErr("send failure, expected %u bytes returned %d : %s",
	      ajStrGetLen(ftpcmd), isendlen, ajMessGetSysmessageC());
    ajDebug("sending: '%S'\n", ftpcmd);
    ftpResponse(ftpfile, &ftpresponse);
    if(ajStrGetCharFirst(ftpresponse) != '2')
    {
        ajDebug("FTP failure: File not found\nResponse: %S",
               ftpresponse);
        return NULL;
    }

    if(fpos)
    {
        ajFmtPrintS(&ftpcmd, "REST %Ld\r\n", fpos);
        isendlen =  send(sock.sock, ajStrGetPtr(ftpcmd),
                         ajStrGetLen(ftpcmd), 0);
        if(isendlen < 0 || isendlen != (ajint) ajStrGetLen(ftpcmd))
            ajErr("send failure, expected %u bytes returned %d : %s",
                  ajStrGetLen(ftpcmd), isendlen, ajMessGetSysmessageC());
        ajDebug("sending: '%S'\n", ftpcmd);
        ftpResponse(ftpfile, &ftpresponse);
    }

    ajFmtPrintS(&ftpcmd, "RETR %S\r\n", get);
    isendlen =  send(sock.sock, ajStrGetPtr(ftpcmd),
                     ajStrGetLen(ftpcmd), 0);
    if(isendlen < 0 || isendlen != (ajint) ajStrGetLen(ftpcmd))
	ajErr("send failure, expected %u bytes returned %d : %s",
	      ajStrGetLen(ftpcmd), isendlen, ajMessGetSysmessageC());
    ajDebug("sending: '%S'\n", ftpcmd);
    /*ftpResponse(sock, &ftpresponse);*/

    ipos = (ajint) ajStrFindAnyK(pasvresponse, '(');
    if(ipos < 4)
        return NULL;

    if(ajStrPrefixC(pasvresponse, "227 "))
    {
        ajStrCutStart(&pasvresponse, ipos);
        iscan = ajFmtScanS(pasvresponse, "(%u,%u,%u,%u,%u,%u)",
                           &ia, &ib, &ic, &id, &ip1, &ip2);
        
        iport = ip1*256+ip2;
        if(iscan != 6)
        {
            ajWarn("FTP failure: Passive mode unexpected reply\nResponse: %S",
               pasvresponse);
            return NULL;
        }
    }
    else if(ajStrPrefixC(pasvresponse, "228 "))
    {
        ajWarn("FTP failure: Long passive mode not supported\nResponse: %S",
               pasvresponse);
        return NULL;
    }
    else if(ajStrPrefixC(pasvresponse, "229 "))
    {
        ajWarn("FTP failure: Extended passive mode not supported\nResponse: %S",
               pasvresponse);
        return NULL;
    }

    ajFmtPrintS(&hoststr, "%u.%u.%u.%u", ia, ib, ic, id);
    ajFmtPrintS(&portstr, "%u", iport);

    ajDebug("PASV mode %S:%S '%S'\n", hoststr, portstr, pasvresponse);

    memset(&hints,0,sizeof(hints));

    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    ret = getaddrinfo(MAJSTRGETPTR(hoststr), MAJSTRGETPTR(portstr),
                      &hints, &addinit);

    if(ret)
    {
	ajErr("[%s] Failed to find host '%S' for database '%S'",
	      gai_strerror(ret), hoststr, dbname);

        ajStrDel(&hoststr);
        ajStrDel(&portstr);

	return NULL;
    }

    ajStrDel(&hoststr);
    ajStrDel(&portstr);

    filesock.sock = AJBADSOCK;
    
    for(add = addinit; add; add = add->ai_next)
    {
        filesock.sock = ajSysFuncSocket(add->ai_family, add->ai_socktype,
                                        add->ai_protocol);

        if(filesock.sock == AJBADSOCK)
            continue;

        if(connect(filesock.sock, add->ai_addr, add->ai_addrlen))
        {
            ajSysSocketclose(filesock);
            filesock.sock = AJBADSOCK;
            continue;
        }

        break;
    }

    freeaddrinfo(addinit);

    if(filesock.sock == AJBADSOCK)
    {
	ajDebug("Socket connect failed\n");
	ajFmtPrintS(&errstr, "socket connect failed for database '%S': %s",
		    dbname, strerror(errno));
	ajErr("%S", errstr);
	ajStrDel(&errstr);

	return NULL;
    }


    /*
    ** need to check response for the IP address and port to use for
    ** the passive connection
    227 Entering Passive Mode (10,42,5,11,146,64)
    10.42.5.11:(146.46)
    */

/*    ajFmtPrintS(&ftpcmd, "PASV\r\n");*/

/*    ajFmtPrintS(&ftpcmd, "REST %Ld\r\n", 0L);*/

    ajStrDel(&ftpcmd);

    /*ajFileClose(&ftpfile);*/

    fp = ajSysFdFromSocket(filesock, "r");
    
    if(!fp)
    {
	ajDebug("ftpSend socket open failed\n");
	ajErr("ftpSend: socket open failed for database '%S'", dbname);

	return NULL;
    }

    ajStrDel(&ftpresponse);
    ajStrDel(&pasvresponse);

    ajFileClose(&ftpfile);

    return fp;
}




/* @func ajFtpRead ************************************************************
**
** Reads the header of ftp response in given buffer buff,
**
** @param [r] dbname [const AjPStr] DB name
** @param [r] host [const AjPStr] Host name
** @param [r] port [ajint] Port
** @param [r] fpos [ajlong] Start position offset
** @param [r] dbpath [const AjPStr] part of URL after port number
** @return [AjPFilebuff] http response
**
** @release 6.5.0
** @@
******************************************************************************/

AjPFilebuff ajFtpRead(const AjPStr dbname,
                      const AjPStr host,
                      ajint port, ajlong fpos, const AjPStr dbpath)
{
    AjPStr get       = NULL;
    AjPStr newhost   = NULL;
    AjPStr path      = NULL;
    AjPFilebuff buff = NULL;
    FILE* fp         = NULL;

    AjOSysSocket sock;
    AjOSysTimeout timo;

    get       = ajStrNew();
    newhost   = ajStrNew();

    ajDebug("ajFtpRead db: '%S' host '%S' port: %u dbpath: '%S'\n",
	    dbname, host, port, dbpath);

    ajStrAssignS(&newhost,host);
    ajStrAssignS(&path, dbpath);


    if(ajStrGetCharFirst(path)!='/')
        ajStrInsertK(&path, 0, '/');

    /*ajFmtPrintS(&get, "GET %S HTTP/%S\r\n", path, httpver);*/
        
    fp = ajFtpOpen(dbname, newhost, port, path, fpos, &sock);

    if(!fp)
    {
        if(port == 21)
            ajErr("Cannot open FTP connection 'ftp://%S%S'",
                  host, dbpath);
        else
            ajErr("Cannot open FTP connection 'ftp://%S:%d%S'",
                  host, port, dbpath);
        buff = NULL;
        return NULL;
    }

    buff = ajFilebuffNewFromCfile(fp);

    if(!buff)
    {
        ajErr("ajFtpRead: socket buffer attach failed for host '%S'"
              ", FTP command was '%S'", host, get);
        return NULL;
    }

    ajDebug("ajFtpRead created file %S:%d%S\n", host, port, dbpath);
    timo.seconds = 180;
    ajSysTimeoutSet(&timo);
    ajSysTimeoutUnset(&timo);

    ajStrDel(&get);
    ajStrDel(&newhost);
    ajStrDel(&path);

    return buff;
}




/* @funcstatic ftpResponse ****************************************************
**
** Read the response from the FTP server
**
** @param [u] ftpfile [AjPFile] File to read from open socket
** @param [u] Presponse [AjPStr*] Response from server
** @return [AjBool] True on success
** @@
******************************************************************************/

static AjBool ftpResponse(AjPFile ftpfile, AjPStr* Presponse)
{
    AjBool done = ajFalse;
    AjPStr ftpline = NULL;

    ajStrAssignC(Presponse, "");

    while(!done)
    {
        if(!ajReadline(ftpfile, &ftpline))
            return ajFalse;

        ajDebug("FTP read line: '%S'\n", ftpline);
        ajStrAppendS(Presponse, ftpline);

        if(ajStrGetCharPos(ftpline, 3) != '-')
            done = ajTrue;
    }
    ajDebug("FTP success\n");

    ajStrDel(&ftpline);

    return ajTrue;
}
