/* @include ajmart ************************************************************
**
** AJAX Mart (database) functions
**
** These functions control all aspects of AJAX sequence database access
**
** @author Copyright (C) 2009 Alan Bleasby
** @version $Revision: 1.17 $
** @modified Nov 23 ajb First version
** @modified $Date: 2012/04/26 17:36:15 $ by $Author: mks $
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

#ifndef AJMART_H
#define AJMART_H


/* ========================================================================= */
/* ============================= include files ============================= */
/* ========================================================================= */

#include "ajdefine.h"
#include "ajdom.h"
#include "ajseqdata.h"

AJ_BEGIN_DECLS




/* ========================================================================= */
/* =============================== constants =============================== */
/* ========================================================================= */




#define MART_CACHE_RD 1
#define MART_CACHE_WT 2
#define MART_CACHE_WTIFEMPTY 4




/* ========================================================================= */
/* ============================== public data ============================== */
/* ========================================================================= */




/* @data AjPMartLoc ******************************************************
**
** Ajax BioMart locations structure.
**
** Holds lists of RegistryDBPointer, MartDBLocation & MartURLLocation
** locations. Each list consists of string tables.
**
** AjPMartLoc is implemented as a pointer to a C data structure.
**
** @alias AjPPMartLoc
** @alias AjSMartLoc
** @alias AjOMartLoc
**
** @attr Registries [AjPTable*] Tables of registry entries
** @attr Databases [AjPTable*] Tables of database entries
** @attr Urls [AjPTable*] Tables of URL entries
** @attr Reg_read [AjPList] List of registry tables used only
**                          during XML reading and subsequently
**                          converted to the table array above.
** @attr Data_read [AjPList] List of database tables (see Reg_read)
** @attr Url_read [AjPList] List of URL tables (see Reg_read)
** @attr Nregistries [ajuint] Number of registry entries
** @attr Ndatabases [ajuint] Number of database entries
** @attr Nurls [ajuint] Number of url entries
** @attr Depth [ajuint] Depth of XML when reading
** @@
******************************************************************************/

typedef struct AjSMartLoc
{
    AjPTable *Registries;
    AjPTable *Databases;
    AjPTable *Urls;
    AjPList Reg_read;
    AjPList Data_read;
    AjPList Url_read;
    ajuint Nregistries;
    ajuint Ndatabases;
    ajuint Nurls;
    ajuint Depth;
} AjOMartLoc;

#define AjPMartLoc AjOMartLoc*
typedef AjPMartLoc* AjPPMartLoc;




/* @data AjPMartDataset *******************************************************
**
** Ajax BioMart dataset information
**
** AjPMartDataset is implemented as a pointer to a C data structure.
**
** @alias AjPPMartDataset
** @alias AjSMartDataset
** @alias AjOMartDataset
**
** @attr Sets [AjPTable*] Datasets
** @attr Set_read [AjPList] List used for XML / tab-sep processing
** @attr Nsets [ajuint] Number of datasets
** @attr Depth [ajuint] Depth for XML processing
** @@
******************************************************************************/

typedef struct AjSMartDataset
{
    AjPTable *Sets;
    AjPList Set_read;
    ajuint Nsets;
    ajuint Depth;
} AjOMartDataset;

#define AjPMartDataset AjOMartDataset*
typedef AjPMartDataset* AjPPMartDataset;




/* @data AjPMartAttribute *****************************************************
**
** Ajax BioMart attribute information
**
** AjPMartAttribute is implemented as a pointer to a C data structure.
**
** @alias AjPPMartAttribute
** @alias AjSMartAttribute
** @alias AjOMartAttribute
**
** @attr Attributes [AjPTable*] Attributes
** @attr Att_read [AjPList] List used for XML / tab-sep processing
** @attr Natts [ajuint] Number of attributes
** @attr Depth [ajuint] Depth for XML processing
** @@
******************************************************************************/

typedef struct AjSMartAttribute
{
    AjPTable *Attributes;
    AjPList Att_read;
    ajuint Natts;
    ajuint Depth;
} AjOMartAttribute;

#define AjPMartAttribute AjOMartAttribute*
typedef AjPMartAttribute* AjPPMartAttribute;




/* @data AjPMartFilter ********************************************************
**
** Ajax BioMart filter information
**
** AjPMartFilter is implemented as a pointer to a C data structure.
**
** @alias AjPPMartFilter
** @alias AjSMartFilter
** @alias AjOMartFilter
**
** @attr Filters [AjPTable*] Filterss
** @attr Filter_read [AjPList] List used for XML / tab-sep processing
** @attr Nfilters [ajuint] Number of filters
** @attr Depth [ajuint] Depth for XML processing
** @@
******************************************************************************/

typedef struct AjSMartFilter
{
    AjPTable *Filters;
    AjPList Filter_read;
    ajuint Nfilters;
    ajuint Depth;
} AjOMartFilter;

#define AjPMartFilter AjOMartFilter*
typedef AjPMartFilter* AjPPMartFilter;




/* @data AjPMartdsinfo ********************************************************
**
** Biomart martservice object for holding query dataset info pre conversion
** to an XML query string
**
** @alias AjSMartdsinfo
** @alias AjOMartdsinfo
**
** @attr Name [AjPStr] Name of dataset
** @attr Interface [AjPStr] Interface of dataset
** @attr Attributes [AjPList] Attributes
** @attr Filters [AjPList] Filters
** @@
******************************************************************************/

typedef struct AjSMartdsinfo
{
    AjPStr Name;
    AjPStr Interface;
    AjPList Attributes;
    AjPList Filters;
} AjOMartdsinfo;

#define AjPMartdsinfo AjOMartdsinfo*




/* @data AjPMartqinfo *********************************************************
**
** Biomart martservice object for holding query info pre conversion
** to an XML query string
**
** @alias AjSMartqinfo
** @alias AjOMartqinfo
**
** @attr Hvirtualschema [AjPStr] Header virtualSchemaName
** @attr Hversion [AjPStr] Header datasetConfigVersion
** @attr Hformatter [AjPStr] Header output formatter type
** @attr Hcount [AjBool] Header count
** @attr Hheader [AjBool] Header flag for printing column names
** @attr Hstamp [AjBool] Header flag for completionStamp
** @attr Hunique [AjBool] Header flag for unique rows
** @attr Dsets [AjPMartdsinfo*] Mart attributes
** @attr Dnsets [ajuint] Number of datasets
** @attr Verify [AjBool] Check attnames and filtnames
** @@
******************************************************************************/

typedef struct AjSMartqinfo
{
    AjPStr Hvirtualschema;
    AjPStr Hversion;
    AjPStr Hformatter;
    AjBool Hcount;
    AjBool Hheader;
    AjBool Hstamp;
    AjBool Hunique;
    AjPMartdsinfo *Dsets;
    ajuint Dnsets;
    AjBool Verify;
} AjOMartqinfo;

#define AjPMartqinfo AjOMartqinfo*




/* @data AjPMartquery *********************************************************
**
** Biomart martservice query object
**
** @alias AjSMartquery
** @alias AjOMartquery
**
** @attr Reghost [AjPStr] Registry host
** @attr Regpath [AjPStr] Registry path to mart service on Reghost
** @attr Marthost [AjPStr] Host containing desired mart (selected from registry)
** @attr Martpath [AjPStr] Path to mart service on Marthost
** @attr Cachedir [AjPStr] Cache directory
** @attr Query [AjPStr] Mart query
** @attr Loc [AjPMartLoc] Mart registry information
** @attr Mart [AjPStr] Mart name
** @attr Dsname [AjPStr] Dataset name
** @attr Dataset [AjPMartDataset] Mart datasets
** @attr Atts [AjPMartAttribute] Mart attributes
** @attr Filters [AjPMartFilter] Mart filters
** @attr Config [AjPDomDocument] Configuration Information
** @attr Regport [ajuint] Registry host URL port
** @attr Martport [ajuint] Mart host URL port
** @attr Cacheflag [ajuint] Control operation of cache
** @attr Padding [ajuint] Padding to alignment boundary
** @@
******************************************************************************/

typedef struct AjSMartquery
{
    AjPStr Reghost;
    AjPStr Regpath;
    AjPStr Marthost;
    AjPStr Martpath;
    AjPStr Cachedir;
    AjPStr Query;
    AjPMartLoc Loc;
    AjPStr Mart;
    AjPStr Dsname;
    AjPMartDataset Dataset;
    AjPMartAttribute Atts;
    AjPMartFilter Filters;
    AjPDomDocument Config;
    ajuint Regport;
    ajuint Martport;
    ajuint Cacheflag;
    ajuint Padding;
} AjOMartquery;

#define AjPMartquery AjOMartquery*




/* ========================================================================= */
/* =========================== public functions ============================ */
/* ========================================================================= */



/*
** Prototype definitions
*/

AjPMartLoc ajMartLocNew(void);
void ajMartLocDel(AjPMartLoc *thys);

AjPMartFilter ajMartFilterNew(void);
void ajMartFilterDel(AjPMartFilter *thys);
    
AjPMartAttribute ajMartAttributeNew(void);
void ajMartAttributeDel(AjPMartAttribute *thys);
    
AjPMartDataset ajMartDatasetNew(void);
void ajMartDatasetDel(AjPMartDataset *thys);
    
AjPMartquery ajMartqueryNew(void);
void ajMartqueryDel(AjPMartquery *thys);

AjPMartqinfo ajMartQinfoNew(ajuint n);
void ajMartQinfoDel(AjPMartqinfo *thys);

AjPMartdsinfo ajMartDsinfoNew(void);
void ajMartDsinfoDel(AjPMartdsinfo *thys);


AjBool ajMartGetRegistry(AjPSeqin seqin);
AjBool ajMartregistryParse(AjPSeqin seqin);

AjBool ajMartGetDatasets(AjPSeqin seqin, const AjPStr mart);
AjBool ajMartdatasetParse(AjPSeqin seqin);

AjBool ajMartGetAttributes(AjPSeqin seqin, const AjPStr dataset);
AjBool ajMartattributesParse(AjPSeqin seqin);
AjBool ajMartGetAttributesSchema(AjPSeqin seqin, const AjPStr dataset,
                                 const AjPStr schema);
AjBool ajMartGetAttributesRetry(AjPSeqin seqin, const AjPStr dataset);
    

AjBool ajMartGetFilters(AjPSeqin seqin, const AjPStr dataset);
AjBool ajMartfiltersParse(AjPSeqin seqin);
AjBool ajMartGetFiltersSchema(AjPSeqin seqin, const AjPStr dataset,
                              const AjPStr schema);
AjBool ajMartGetFiltersRetry(AjPSeqin seqin, const AjPStr dataset);
    

AjBool ajMartFilterMatch(AjPTable atab, const AjPMartFilter filt);
AjBool ajMartAssociate(AjPSeqin seqin);

void ajMartMakeQueryXml(const AjPMartqinfo qinfo, AjPSeqin seqin);

void ajMartFixMart(AjPSeqin seqin);
void ajMartFixRegistry(AjPSeqin seqin);

AjBool ajMartCheckQinfo(AjPSeqin seqin, AjPMartqinfo qinfo);

AjBool ajMartSendQuery(AjPSeqin seqin);

const AjPStr ajMartGetMarthost(const AjPSeqin seqin);
const AjPStr ajMartGetMartpath(const AjPSeqin seqin);
ajuint ajMartGetMartport(const AjPSeqin seqin);

const AjPStr ajMartGetReghost(const AjPSeqin seqin);
const AjPStr ajMartGetRegpath(const AjPSeqin seqin);
ajuint ajMartGetRegport(const AjPSeqin seqin);

AjBool ajMartSetMarthostC(AjPSeqin seqin, const char *marthost);
AjBool ajMartSetMartpathC(AjPSeqin seqin, const char *martpath);
AjBool ajMartSetMarthostS(AjPSeqin seqin, const AjPStr marthost);
AjBool ajMartSetMartpathS(AjPSeqin seqin, const AjPStr martpath);
AjBool ajMartSetMartport(AjPSeqin seqin, ajuint martport);

AjBool ajMartSetReghostC(AjPSeqin seqin, const char *reghost);
AjBool ajMartSetRegpathC(AjPSeqin seqin, const char *regpath);
AjBool ajMartSetReghostS(AjPSeqin seqin, const AjPStr reghost);
AjBool ajMartSetRegpathS(AjPSeqin seqin, const AjPStr regpath);
AjBool ajMartSetRegport(AjPSeqin seqin, ajuint regport);

AjPMartquery ajMartGetMartqueryPtr(const AjPSeqin seqin);
void ajMartAttachMartquery(AjPSeqin seqin, AjPMartquery mq);
void ajMartquerySeqinFree(AjPSeqin seqin);

char *ajStrtokQuotR(const char *srcstr, const char *delimstr,
                    const char *quotstr, const char **ptrptr,
                    AjPStr *buf);

AjBool ajMartParseParameters(AjPMartqinfo qinfo, const AjPStr atts,
                             const AjPStr filts, ajuint idx);

void ajMartSetQuerySchemaC(AjPMartqinfo qinfo, const char *schema);
void ajMartSetQueryVersionC(AjPMartqinfo qinfo, const char *version);
void ajMartSetQueryFormatC(AjPMartqinfo qinfo, const char *format);
void ajMartSetQueryCount(AjPMartqinfo qinfo, AjBool count);
void ajMartSetQueryHeader(AjPMartqinfo qinfo, AjBool header);
void ajMartSetQueryUnique(AjPMartqinfo qinfo, AjBool unique);
void ajMartSetQueryStamp(AjPMartqinfo qinfo, AjBool stamp);
void ajMartSetQueryVerify(AjPMartqinfo qinfo, AjBool verify);

void ajMartSetQueryDatasetName(AjPMartqinfo qinfo, const AjPStr name,
                               ajuint idx);
void ajMartSetQueryDatasetInterfaceC(AjPMartqinfo qinfo, const char *iface,
                                     ajuint idx);

AjBool ajMartGetConfiguration(AjPSeqin seqin, const AjPStr dataset);
AjBool ajMartconfigurationParse(AjPSeqin seqin);
AjBool ajMartattributesPageSort(AjPSeqin seqin);

AjBool ajMartNameIsNucC(const char *name);
AjBool ajMartNameIsProtC(const char *name);
AjBool ajMartTableNameIsNuc(const AjPTable t);
AjBool ajMartTableNameIsProt(const AjPTable t);

AjPStr *ajMartCheckHeader(AjPSeqin seqin, AjPMartqinfo qinfo);
void    ajMartDecodeHname(const AjPStr dir, AjPStr *host, AjPStr *path,
                          ajuint *port);


/*
** End of prototype definitions
*/

AJ_END_DECLS

#endif  /* !AJMART_H */
