#ifdef __cplusplus
extern "C"
{
#endif

#ifndef ajhttp_h
#define ajhttp_h




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




/*
** Prototype definitions
*/


AjBool ajHttpGetProxyinfo(const AjPStr dbproxy, ajint* proxyport,
                          AjPStr* proxyname, AjPStr* proxyauth,
                          AjPStr* proxycreds);
AjBool ajHttpGetVersion(const AjPStr version, AjPStr* httpver);
FILE*  ajHttpOpen(const AjPStr dbname, const AjPStr host, ajint iport,
                  const AjPStr get, struct AJSOCKET *Psock);
FILE*  ajHttpOpenProxy(const AjPStr dbname, const AjPStr proxyname,
                       ajint proxyport, const AjPStr proxyauth,
                       const AjPStr proxycreds, const AjPStr host,
                       ajint iport, const AjPStr get, struct AJSOCKET *Psock);
AjPFilebuff ajHttpRead(const AjPStr dbhttpver, const AjPStr dbname,
                       const AjPStr dbproxy, const AjPStr host,
                       ajint port, const AjPStr dbpath);
AjBool ajHttpRedirect(AjPFilebuff buff, AjPStr* host, ajint* port,
		      AjPStr* get);

AjPUrlref ajHttpUrlrefNew(void);
void   ajHttpUrlrefDel(AjPUrlref *thys);
void   ajHttpUrlrefParseC(AjPUrlref *parts, const char *url);
void   ajHttpUrlrefSplitPort(AjPUrlref urli);
void   ajHttpUrlrefSplitUsername(AjPUrlref urli);

AjBool ajHttpQueryUrl(const AjPQuery qry, ajint* iport, AjPStr* host,
                      AjPStr* urlget);

void   ajHttpUrlDeconstruct(const AjPStr url, ajint* iport, AjPStr* host,
                          AjPStr* urlget);

/*
** End of prototype definitions
*/

#endif

#ifdef __cplusplus
}
#endif
