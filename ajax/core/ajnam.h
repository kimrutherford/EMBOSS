/* @include ajnam *************************************************************
**
** AJAX nam functions
**
** Creates a hash table of initial values and allow access to this
** via the routines ajNamDatabase and ajNamGetValueS.
**
** @author Copyright (C) 1998 Ian Longden
** @version $Revision: 1.49 $
** @modified 2000-2011 Peter Rice
** @modified $Date: 2013/01/24 15:31:46 $ by $Author: rice $
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

#ifndef AJNAM_H
#define AJNAM_H

/* ========================================================================= */
/* ============================= include files ============================= */
/* ========================================================================= */

#include "ajdefine.h"
#include "ajfile.h"
#include "ajquerydata.h"

AJ_BEGIN_DECLS




/* ========================================================================= */
/* =============================== constants =============================== */
/* ========================================================================= */




#define AJMETHOD_ENTRY 1
#define AJMETHOD_QUERY 2
#define AJMETHOD_ALL   4




/* ========================================================================= */
/* ============================== public data ============================== */
/* ========================================================================= */




/*
** Prototype definitions
*/

AjBool ajNamDatabase(const AjPStr name);
AjBool ajNamAliasDatabase(AjPStr *Pname);
AjBool ajNamServer(const AjPStr name);
AjBool ajNamAliasServer(AjPStr *Pname, const AjPStr server);
AjBool ajNamDatabaseServer(const AjPStr name, const AjPStr server);
AjPList ajNamDbGetAttrlist(const AjPStr name);
AjPList ajNamDbGetAttrlistSvr(const AjPStr name, const AjPStr svrname);
AjBool ajNamDbGetAttrC(const AjPStr name, const char *attribute,
                       AjPStr *value);
AjBool ajNamDbGetAttrS(const AjPStr name, const AjPStr attribute,
                       AjPStr *value);
AjBool ajNamDbDetails(const AjPStr name, AjPStr* type,
                      AjBool* id, AjBool* qry, AjBool* all,
                      AjPStr* comment, AjPStr* release,
                      AjPStr* methods, AjPStr* defined);
AjBool ajNamDbDetailsSvr(const AjPStr name,const AjPStr svrname,  AjPStr* type,
                      AjBool* id, AjBool* qry, AjBool* all,
                      AjPStr* comment, AjPStr* release,
                      AjPStr* methods, AjPStr* defined);
AjBool ajNamSvrGetAttrC(const AjPStr name, const char *attribute,
                        AjPStr *value);
AjBool ajNamSvrGetAttrS(const AjPStr name, const AjPStr attribute,
                        AjPStr *value);
ajuint ajNamDbGetAttrSpecialC(const AjPStr name, const char *attribute,
                              AjPStr* value);
ajuint ajNamDbGetAttrSpecialS(const AjPStr name, const AjPStr attribute,
                              AjPStr* value);
ajuint ajNamSvrGetAttrSpecialC(const AjPStr name, const char *attribute,
                               AjPStr* value);
ajuint ajNamSvrGetAttrSpecialS(const AjPStr name, const AjPStr attribute,
                               AjPStr* value);
AjPList ajNamSvrGetAttrlist(const AjPStr name);
AjBool ajNamSvrGetdbAttrC(const AjPStr name, const AjPStr dbname,
                          const char *attribute,
                          AjPStr *value);
AjBool ajNamSvrGetdbAttrS(const AjPStr name, const AjPStr dbname,
                          const AjPStr attribute,
                          AjPStr *value);
ajuint ajNamSvrGetdbAttrSpecialC(const AjPStr name, const AjPStr dbname,
                                 const char *attribute,
                                 AjPStr *value);
ajuint ajNamSvrGetdbAttrSpecialS(const AjPStr name, const AjPStr dbname,
                                 const AjPStr attribute,
                                 AjPStr *value);
AjBool ajNamSvrDetails(const AjPStr name, AjPStr* type, AjPStr *scope,
                       AjBool* id, AjBool* qry, AjBool* all,
                       AjPStr* comment, AjPStr *version,
                       AjPStr* methods, AjPStr* defined,
                       AjPStr* cachedirectory, AjPStr* cachefile,
                       AjPStr* url);
AjBool ajNamDbGetDbalias(const AjPStr dbname, AjPStr* dbalias);
AjBool ajNamDbGetDbaliasTest(const AjPStr dbname, AjPStr* dbalias);
AjBool ajNamDbGetIndexdir(const AjPStr dbname, AjPStr* indexdir);
AjBool ajNamDbGetType(const AjPStr dbname, ajuint *itype);
AjBool ajNamDbGetUrl(const AjPStr dbname, AjPStr* url);
AjBool ajNamDbData(AjPQuery qry, ajuint argc, ...);
AjBool ajNamDbQuery(AjPQuery qry);
AjBool ajNamFileQuery(AjPQuery qry);
AjBool ajNamQuerySetDbalias(AjPQuery qry);
const char* ajNamQueryGetDatatypeC(const AjPQuery qry);
const AjPStr ajNamQueryGetUrl(const AjPQuery qry);
AjBool ajNamDbTest(const AjPStr dbname);

AjBool ajNamSvrGetDbalias(const AjPStr svrname, AjPStr* svralias);
AjBool ajNamSvrGetUrl(const AjPStr svrname, AjPStr* url);
AjBool ajNamSvrData(AjPQuery qry, ajuint argc, ...);
AjBool ajNamSvrQuery(AjPQuery qry);
AjBool ajNamSvrTest(const AjPStr svrname);
ajuint ajNamSvrCount(const AjPStr svrname);
void   ajNamSvrListFindAliases(const AjPStr server, const AjPStr dbname,
                               AjPList dbnames);
void   ajNamSvrListListAliases(const AjPStr server, AjPList dbnames);
void   ajNamSvrListListDatabases(const AjPStr server, AjPList dbnames);

void   ajNamDebugOrigin(void);
void   ajNamDebugServers(void);
void   ajNamDebugDatabases(void);
void   ajNamDebugResources(void);
void   ajNamDebugVariables(void);
void   ajNamDebugAliases(void);

void   ajNamExit(void);

AjBool ajNamGetAliasC(const char* name, AjPStr* value);
AjBool ajNamGetAliasS(const AjPStr namestr, AjPStr* value);

AjBool ajNamGetenvC(const char* name, AjPStr* value);
AjBool ajNamGetenvS(const AjPStr name, AjPStr* value);
AjBool ajNamGetValueC(const char *name, AjPStr* value);
AjBool ajNamGetValueS(const AjPStr name, AjPStr* value);
void   ajNamInit(const char* prefix);
AjBool ajNamIsDbname(const AjPStr name);
void   ajNamListFindAliases(const AjPStr dbname, AjPList dbnames);
void   ajNamListListAliases(AjPList dbnames);
void   ajNamListListDatabases(AjPList dbnames);
void   ajNamListListResources(AjPList dbnames);
void   ajNamListListServers(AjPList svrnames);
void   ajNamPrintSvrAttr(AjPFile outf, AjBool full);
void   ajNamPrintDbAttr(AjPFile outf, AjBool full);
void   ajNamPrintRsAttr(AjPFile outf, AjBool full);

AjBool ajNamResolve(AjPStr* name);

const AjPStr ajNamValueRootdir(void);
const AjPStr ajNamValueBasedir(void);
const AjPStr ajNamValueInstalldir(void);
const AjPStr ajNamValuePackage(void);
const AjPStr ajNamValueSystem(void);
const AjPStr ajNamValueVersion(void);

AjBool ajNamRsAttrValueS(const AjPStr name, const AjPStr attribute,
                         AjPStr *value);
AjBool ajNamRsAttrValueC(const char *name, const char *attribute,
                         AjPStr *value);
AjBool ajNamRsListValue(const AjPStr name, AjPStr *value);
AjBool ajNamSetControl(const char* optionName);

/*
** End of prototype definitions
*/




#ifdef WIN32
#ifdef AJAXDLL_EXPORTS
__declspec(dllexport) const char* EMBOSSWINROOT_ENVVAR;
#else
__declspec(dllimport) const char* EMBOSSWINROOT_ENVVAR;
#endif
#endif




#ifdef AJ_COMPILE_DEPRECATED_BOOK
#endif /* AJ_COMPILE_DEPRECATED_BOOK */

#ifdef AJ_COMPILE_DEPRECATED

__deprecated AjBool ajNamRsAttrValue(const AjPStr name, const AjPStr attribute,
                                     AjPStr *value);
__deprecated AjBool ajNamGetenv(const AjPStr name, AjPStr* value);
__deprecated AjBool ajNamGetValue(const AjPStr name, AjPStr* value);
__deprecated AjBool ajNamRootInstall(AjPStr* root);
__deprecated AjBool ajNamRoot(AjPStr* root);
__deprecated AjBool ajNamRootBase(AjPStr* rootbase);
__deprecated AjBool ajNamRootPack(AjPStr* pack);
__deprecated AjBool ajNamRootVersion(AjPStr* version);

#endif /* AJ_COMPILE_DEPRECATED */




AJ_END_DECLS

#endif /* !AJNAM_H */
