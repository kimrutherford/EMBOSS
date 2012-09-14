/* @source ajdomain ***********************************************************
**
** AJAX low-level functions for handling protein domain data.
** For use with Scop and Cath objects defined in ajdomain.h
** Includes functions for reading SCOP and CATH parsable files and for
** reading and writing dcf (domain classification file) format.
**
** @author Copyright (C) 2004 Jon Ison (jison@hgmp.mrc.ac.uk)
** @version $Revision: 1.49 $
** @modified $Date: 2012/07/02 16:48:15 $ by $Author: rice $
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




/* ========================================================================= */
/* ============================= include files ============================= */
/* ========================================================================= */

#include "ajlib.h"

#include "ajdomain.h"
#include "ajlist.h"
#include "ajfileio.h"

#include <string.h>
#include <ctype.h>
#include <limits.h>




/* ========================================================================= */
/* =============================== constants =============================== */
/* ========================================================================= */




/* ========================================================================= */
/* =========================== global variables ============================ */
/* ========================================================================= */




/* ========================================================================= */
/* ============================= private data ============================== */
/* ========================================================================= */




/* @datastatic AjPScopcla *****************************************************
**
** Private Scopcla object.
**
** Holds scop database data from raw file (dir.cla.scop.txt from SCOP authors)
**
** AjPScopcla is implemented as a pointer to a C data structure.
**
** @alias AjSScopcla
** @alias AjOScopcla
**
** @attr Entry [AjPStr]      Domain identifier code.
** @attr Pdb [AjPStr]        Corresponding pdb identifier code.
** @attr Sccs [AjPStr]       Scop compact classification string.
** @attr Class [ajint]       SCOP sunid for class
** @attr Fold[ajint]         SCOP sunid for fold
** @attr Superfamily [ajint] SCOP sunid for superfamily
** @attr Family [ajint]      SCOP sunid for family
** @attr Domain [ajint]      SCOP sunid for domain
** @attr Source [ajint]      SCOP sunid for species
** @attr Domdat [ajint]      SCOP sunid for domain data
** @attr Number [ajuint]
** Number of chains from which this domain is comprised
** @attr Chain [char*]       Chain identifiers
** @attr Start [AjPStr*]     PDB residue number of first residue in domain
** @attr End [AjPStr*]       PDB residue number of last residue in domain
**
** @@
******************************************************************************/

typedef struct AjSScopcla
{
    AjPStr Entry;
    AjPStr Pdb;
    AjPStr Sccs;
    ajint Class;
    ajint Fold;
    ajint Superfamily;
    ajint Family;
    ajint Domain;
    ajint Source;
    ajint Domdat;
    ajuint Number;
    char *Chain;
    AjPStr *Start;
    AjPStr *End;
} AjOScopcla, *AjPScopcla;




/* @datastatic AjPScopdes *****************************************************
**
** AJAX Scopdes object.
**
** Holds SCOP database data from raw file (dir.des.scop.txt from SCOP authors)
**
** AjPScopdes is implemented as a pointer to a C data structure.
**
** @alias AjSScopdes
** @alias AjOScopdes
**
** @attr Type [AjPStr]  Type of node, either 'px' (domain data), 'cl' (class),
**                      'cf' (fold), 'sf' (superfamily), 'fa' (family), 'dm'
**                      (domain) or 'sp' (species).
** @attr Sccs [AjPStr]  Scop compact classification string.
** @attr Entry [AjPStr] Domain identifier code (or '-' if Type!='px').
** @attr Desc [AjPStr]  Description in English of the node.
** @attr Sunid [ajint]  SCOP sunid for node.
** @attr Padding [char[4]]  Padding to alignment boundary
**
** @@
******************************************************************************/

typedef struct AjSScopdes
{
    AjPStr Type;
    AjPStr Sccs;
    AjPStr Entry;
    AjPStr Desc;
    ajint  Sunid;
    char   Padding[4];
} AjOScopdes;

#define AjPScopdes AjOScopdes*




/* @datastatic AjPCathDom *****************************************************
**
** AJAX CathDom object
**
** Holds CATH database data from domlist.v2.4. This file only contains
** domain information for proteins that have 2 or more domains.
**
** AjPCathDom is implemented as a pointer to a C data structure.
**
** @alias AjSCathDom
** @alias AjOCathDom
**
** @attr DomainID [AjPStr] ID for protein containing 2 or more domains
** @attr Start [AjPStr*]   PDB residue number of first residue in segment
** @attr End [AjPStr*]     PDB residue number of last residue in segment
** @attr NSegment [ajuint] Number of chain segments domain is comprised of
** @attr Padding [char[4]] Padding to alignment boundary
**
** @@
******************************************************************************/

typedef struct AjSCathDom
{
    AjPStr DomainID;
    AjPStr *Start;
    AjPStr *End;
    ajuint NSegment;
    char Padding[4];
} AjOCathDom;

#define AjPCathDom AjOCathDom*




/* @datastatic AjPCathName ****************************************************
**
** AJAX CathName object
**
** Holds CATH database data from CAT.names.all.v2.4. This file contains
** a description of each level in the CATH hierarchy.
**
** AjPCathName is implemented as a pointer to a C data structure.
**
** @alias AjSCathName
** @alias AjOCathName
**
** @attr Id [AjPStr] Classification Id
** @attr Desc [AjPStr] Description of level in CATH hierarchy
**
** @@
******************************************************************************/

typedef struct AjSCathName
{
    AjPStr Id;
    AjPStr Desc;
} AjOCathName;

#define AjPCathName AjOCathName*




/* ========================================================================= */
/* =========================== private constants =========================== */
/* ========================================================================= */




/* ========================================================================= */
/* =========================== private variables =========================== */
/* ========================================================================= */




static AjPStr domainGStrline     = NULL;   /* Line from file */
static AjPStr domainGStrsunidstr = NULL;   /* sunid as string */
static AjPStr domainGStrtentry   = NULL;
static AjPStr domainGStrtmp      = NULL;
static AjPStr domainGStrscopid   = NULL;  /* SCOP code */
static AjPStr domainGStrpdbid    = NULL;  /* PDB code */
static AjPStr domainGStrchains   = NULL;  /* Chain data */
static AjPStr domainGStrsccs     = NULL;  /* Scop compact classification */
static AjPStr domainGStrclass    = NULL;  /* Classification containing all
                                             SCOP sunid's  */
static AjPStr domainGStrtoken    = NULL;
static AjPStr domainGStrstr      = NULL;

static AjPRegexp domainGRegexp   = NULL;
static AjPRegexp domainGRegrexp  = NULL;




/* ========================================================================= */
/* =========================== private functions =========================== */
/* ========================================================================= */




static AjPScopcla    domainScopclaNew(ajuint chains);
static void          domainScopclaDel(AjPScopcla *Pscopcla);
static AjPScopcla    domainScopclaRead(AjPFile inf, const AjPStr entry);
static AjPScopcla    domainScopclaReadC(AjPFile inf, const char *entry);

static AjPScopdes    domainScopdesNew(void);
static void          domainScopdesDel(AjPScopdes *Pscopdes);
static AjPScopdes    domainScopdesRead(AjPFile inf, const AjPStr entry);
static AjPScopdes    domainScopdesReadC(AjPFile inf, const char *entry);
static ajint         domainScopdesBinSearch(ajint sunid,
                                            AjPScopdes const *array,
                                            ajint size);

static ajint         domainScopdesCompSunid(const void *item1,
                                            const void *item2);

static ajint         domainCathNameBinSearch(const AjPStr identifier,
                                             AjPCathName const *array,
                                             ajint size);
static ajint         domainCathDomBinSearch(const AjPStr identifier,
                                            AjPCathDom const *array,
                                            ajint size);
static ajint         domainSortCathDomDomainID(const void *item1,
                                               const void *item2);
static ajint         domainSortCathNameId(const void *item1,
                                          const void *item2);
static void          domainCathNameDel(AjPCathName *Pcathname);
static AjPCathName   domainCathNameNew(void);
static void          domainCathDomDel(AjPCathDom *Pcathdom);
static AjPCathDom    domainCathDomNew(ajuint nsegments);




/* ========================================================================= */
/* ======================= All functions by section ======================== */
/* ========================================================================= */




/* @funcstatic domainScopclaNew ***********************************************
**
** Scopcla object constructor. Fore-knowledge of the number of chains is
** required. This is normally called by the domainScopclaReadC /
** domainScopclaRead functions.
**
** @param [r] chains [ajuint] Number of chains
**
** @return [AjPScopcla] Pointer to a scopcla object
**
** @release 2.9.0
** @@
******************************************************************************/

static AjPScopcla domainScopclaNew(ajuint chains)
{
    ajuint i = 0U;

    AjPScopcla scopcla = NULL;

    AJNEW0(scopcla);

    scopcla->Entry = ajStrNew();
    scopcla->Pdb   = ajStrNew();
    scopcla->Sccs  = ajStrNew();

    if(chains)
    {
        scopcla->Chain = ajCharNewRes(chains);

        AJCNEW0(scopcla->Start, chains);
        AJCNEW0(scopcla->End,   chains);

        for(i = 0U; i < chains; i++)
        {
            scopcla->Start[i] = ajStrNew();
            scopcla->End[i]   = ajStrNew();
        }
    }

    scopcla->Number = chains;

    return scopcla;
}




/* @funcstatic domainScopdesNew ***********************************************
**
** Scopdes object constructor.
**
** This is normally called by the domainScopdesReadC / domainScopdesRead
** functions.
**
** @return [AjPScopdes] Pointer to a scopdes object
**
** @release 2.9.0
** @@
******************************************************************************/

static AjPScopdes domainScopdesNew(void)
{
    AjPScopdes scopdes = NULL;

    AJNEW0(scopdes);

    scopdes->Type  = ajStrNew();
    scopdes->Sccs  = ajStrNew();
    scopdes->Entry = ajStrNew();
    scopdes->Desc  = ajStrNew();

    return scopdes;
}




/* @funcstatic domainScopclaRead **********************************************
**
** Read a Scopcla object for a given SCOP domain from the SCOP parsable
** file (dir.cla.scop.txt).
**
** @param [u] inf [AjPFile] AJAX Input file stream
** @param [r] entry [const AjPStr] Identifier
** @return [AjPScopcla] Scopcla object
** @category new [AjPScopcla] Read a Scopcla object for a given SCOP domain
**
** @release 2.9.0
** @@
******************************************************************************/

static AjPScopcla domainScopclaRead(AjPFile inf, const AjPStr entry)
{
    return domainScopclaReadC(inf, ajStrGetPtr(entry));
}




/* @funcstatic domainScopdesRead **********************************************
**
** Read a Scopdes object for a given SCOP domain from the SCOP parsable
** file (dir.des.scop.txt).
**
** @param [u] inf   [AjPFile] AJAX Input file stream.
** @param [r] entry [const AjPStr] CATH identifier of domain.
**
** @return [AjPScopdes] Scopdes object
** @category new [AjPScopdes] Read a Scopdes object for a given SCOP domain
**
** @release 2.9.0
** @@
******************************************************************************/

static AjPScopdes domainScopdesRead(AjPFile inf, const AjPStr entry)
{
    return domainScopdesReadC(inf, ajStrGetPtr(entry));
}




/* @funcstatic domainScopdesReadC *********************************************
**
** Read a Scopdes object for a given SCOP domain from the SCOP parsable
** file (dir.des.scop.txt).
**
** @param [u] inf   [AjPFile] AJAX Input file stream.
** @param [r] entry [const char*] SCOP identifier of domain.
**
** @return [AjPScopdes] Scopdes object
** @category new [AjPScopdes] Read a Scopdes object for a given SCOP domain
**
** @release 2.9.0
** @@
******************************************************************************/

static AjPScopdes domainScopdesReadC(AjPFile inf, const char *entry)
{
    AjPScopdes scopdes = NULL;
    AjBool ok = ajFalse;
    static AjBool called = AJFALSE;

    /* Only initialise strings if this is called for the first time */
    if(!called)
    {
        called = ajTrue;

        if(!domainGStrline)
            domainGStrline = ajStrNew();

        if(!domainGStrtentry)
            domainGStrtentry = ajStrNew();

        domainGStrsunidstr = ajStrNew();
        domainGStrtmp      = ajStrNew();

        domainGRegrexp  = ajRegCompC(
            "^([^ \t]+)[ \t]+([^ \t]+)[ \t]+([^ \t]+)[ \t]+([^ \t]+)[ \t]+");
    }


    /* Read up to the correct entry (line) */
    ajStrAssignC(&domainGStrtentry, entry);
    ajStrFmtUpper(&domainGStrtentry);

    while((ok = ajReadlineTrim(inf, &domainGStrline)))
    {
        if((ajFmtScanS(domainGStrline, "%S", &domainGStrsunidstr) == 0))
            return NULL;

        /* Ignore comment lines */
        if(*(domainGStrline->Ptr) == '#')
            continue;

        if(ajStrMatchWildS(domainGStrsunidstr, domainGStrtentry))
            break;
    }

    if(!ok)
        return NULL;

    scopdes = domainScopdesNew();

    if((ajFmtScanS(domainGStrline, "%d %S %S %S", &scopdes->Sunid,
                   &scopdes->Type, &scopdes->Sccs, &scopdes->Entry) != 4))
    {
        domainScopdesDel(&scopdes);
        return NULL;
    }

    /* Tokenise the line by ' ' and discard the first 4 strings */

    if(!ajRegExec(domainGRegrexp, domainGStrline))
    {
        ajFmtPrint("-->  %S\n", domainGStrline);
        ajFatal("File read error in domainScopdesReadC");
    }

    ajRegSubI(domainGRegrexp, 1, &domainGStrtmp);
    ajRegSubI(domainGRegrexp, 2, &domainGStrtmp);
    ajRegSubI(domainGRegrexp, 3, &domainGStrtmp);
    ajRegSubI(domainGRegrexp, 4, &domainGStrtmp);

    ajRegPost(domainGRegrexp, &scopdes->Desc);

    ajStrRemoveWhiteExcess(&scopdes->Desc);

    return scopdes;
}




/* @funcstatic domainScopclaReadC *********************************************
**
** Read a Scopcla object for a given SCOP domain from the SCOP parsable
** file (dir.des.scop.txt).
**
** @param [u] inf   [AjPFile] AJAX Input file stream
** @param [r] entry [const char*] SCOP domain identifier
** @return [AjPScopcla] Scopcla object
** @category new [AjPScopcla] Read a Scopcla object for a given SCOP domain
**
** @release 2.9.0
** @@
******************************************************************************/

static AjPScopcla domainScopclaReadC(AjPFile inf, const char *entry)
{
    AjPScopcla scopcla = NULL;

    AjPStrTok handle  = NULL;
    AjPStrTok bhandle = NULL;
    AjBool ok         = ajFalse;

    char c   = ' ';
    const char *p = NULL;
    ajuint n  = 0U;
    ajuint i  = 0U;
    ajint from;
    ajint to;
    static AjBool called = AJFALSE;

    /* Only initialise strings if this is called for the first time */
    if(!called)
    {
        called = ajTrue;
        domainGStrline    = ajStrNew();
        domainGStrscopid  = ajStrNew();
        domainGStrpdbid   = ajStrNew();
        domainGStrchains  = ajStrNew();
        domainGStrsccs    = ajStrNew();
        domainGStrtentry  = ajStrNew();
        domainGStrtoken   = ajStrNew();
        domainGStrstr     = ajStrNew();
        domainGStrclass   = ajStrNew();

        domainGRegexp   = ajRegCompC("^([0-9]+)([A-Za-z]+)[-]([0-9]+)");
    }


    /* Read up to the correct entry (line) */
    ajStrAssignC(&domainGStrtentry, entry);
    ajStrFmtUpper(&domainGStrtentry);

    while((ok = ajReadlineTrim(inf, &domainGStrline)))
    {
        if((ajFmtScanS(domainGStrline, "%S", &domainGStrscopid) == 0))
            return NULL;

        /* Ignore comment lines */
        if(ajStrGetCharFirst(domainGStrscopid) == '#')
            continue;

        if(ajStrMatchWildS(domainGStrscopid, domainGStrtentry))
            break;
    }

    if(!ok)
        return NULL;

    if((ajFmtScanS(domainGStrline, "%*S %S %S %S %*d %S",
                   &domainGStrpdbid, &domainGStrchains, &domainGStrsccs,
                   &domainGStrclass) != 4))
        return NULL;

    /* Count chains and allocate Scopcla object */
    n = ajStrParseCountC(domainGStrchains, ",");
    scopcla = domainScopclaNew(n);

    ajStrFmtUpper(&domainGStrscopid);
    ajStrAssignS(&scopcla->Entry, domainGStrscopid);

    ajStrFmtUpper(&domainGStrpdbid);
    ajStrAssignS(&scopcla->Pdb, domainGStrpdbid);

    ajStrFmtUpper(&domainGStrsccs);
    ajStrAssignS(&scopcla->Sccs, domainGStrsccs);

    handle = ajStrTokenNewC(domainGStrchains, ",");

    for(i = 0U; i < n; i++)
    {
        ajStrTokenNextParse(&handle, &domainGStrtoken);
        p = ajStrGetPtr(domainGStrtoken);

        if(sscanf(p, "%d-%d", &from, &to) == 2)
        {
            scopcla->Chain[i] = '.';
            ajFmtPrintS(&scopcla->Start[i], "%d", from);
            ajFmtPrintS(&scopcla->End[i], "%d", to);
        }
        else if(sscanf(p, "%c:%d-%d", &c, &from, &to) == 3)
        {
            ajFmtPrintS(&scopcla->Start[i], "%d", from);
            ajFmtPrintS(&scopcla->End[i], "%d", to);
            scopcla->Chain[i] = c;
        }
        else if(ajStrGetCharPos(domainGStrtoken, 1) == ':')
        {
            ajStrAssignC(&scopcla->Start[i], ".");
            ajStrAssignC(&scopcla->End[i], ".");
            scopcla->Chain[i] = *ajStrGetPtr(domainGStrtoken);
        }
        else if(ajRegExec(domainGRegexp, domainGStrtoken))
        {
            ajRegSubI(domainGRegexp, 1, &domainGStrstr);
            ajStrAssignS(&scopcla->Start[i], domainGStrstr);
            ajRegSubI(domainGRegexp, 2, &domainGStrstr);
            scopcla->Chain[i] = *ajStrGetPtr(domainGStrstr);
            ajRegSubI(domainGRegexp, 3, &domainGStrstr);
            ajStrAssignS(&scopcla->End[i], domainGStrstr);
        }
        else if(ajStrGetCharFirst(domainGStrtoken) == '-')
        {
            scopcla->Chain[i] = '.';
            ajStrAssignC(&scopcla->Start[i], ".");
            ajStrAssignC(&scopcla->End[i],   ".");
        }
        else
            ajFatal("Unparseable chain line [%S]\n", domainGStrchains);
    }

    ajStrTokenDel(&handle);


    /* Read SCOP sunid's from classification string */
    bhandle = ajStrTokenNewC(domainGStrclass, ",\n");

    while(ajStrTokenNextParse(&bhandle, &domainGStrtoken))
    {
        if(ajStrPrefixC(domainGStrtoken, "cl"))
            ajFmtScanS(domainGStrtoken, "cl=%d", &scopcla->Class);
        else if(ajStrPrefixC(domainGStrtoken, "cf"))
            ajFmtScanS(domainGStrtoken, "cf=%d", &scopcla->Fold);
        else if(ajStrPrefixC(domainGStrtoken, "sf"))
            ajFmtScanS(domainGStrtoken, "sf=%d", &scopcla->Superfamily);
        else if(ajStrPrefixC(domainGStrtoken, "fa"))
            ajFmtScanS(domainGStrtoken, "fa=%d", &scopcla->Family);
        else if(ajStrPrefixC(domainGStrtoken, "dm"))
            ajFmtScanS(domainGStrtoken, "dm=%d", &scopcla->Domain);
        else if(ajStrPrefixC(domainGStrtoken, "sp"))
            ajFmtScanS(domainGStrtoken, "sp=%d", &scopcla->Source);
        else if(ajStrPrefixC(domainGStrtoken, "px"))
            ajFmtScanS(domainGStrtoken, "px=%d", &scopcla->Domdat);
    }

    ajStrTokenDel(&bhandle);

    return scopcla;
}




/* @funcstatic domainScopclaDel ***********************************************
**
** Destructor for scopcla object.
**
** @param [d] Pscopcla [AjPScopcla*] Scopcla address
** @return [void]
** @category delete [AjPScopcla] Default destructor
**
** @release 2.9.0
** @@
******************************************************************************/

static void domainScopclaDel(AjPScopcla *Pscopcla)
{
    ajuint i = 0U;

    AjPScopcla scopcla = NULL;

    scopcla = *Pscopcla;

    if(!Pscopcla || !scopcla)
        return;

    ajStrDel(&scopcla->Entry);
    ajStrDel(&scopcla->Pdb);
    ajStrDel(&scopcla->Sccs);

    if(scopcla->Number)
    {
        for(i = 0U; i < scopcla->Number; i++)
        {
            ajStrDel(&scopcla->Start[i]);
            ajStrDel(&scopcla->End[i]);
        }

        AJFREE(scopcla->Start);
        AJFREE(scopcla->End);
        AJFREE(scopcla->Chain);
    }

    AJFREE(scopcla);

    *Pscopcla = NULL;

    return;
}




/* @funcstatic domainScopdesBinSearch *****************************************
**
** Performs a binary search for a Sunid over an array of Scopdes objects
** structures (which of course must first have been sorted, e.g. by using
** domainScopdesCompSunid).
**
** @param [r] sunid [ajint] Search value of Sunid
** @param [r] array [AjPScopdes const*] Array of Scopdes objects
** @param [r] size [ajint] Size of array
**
** @return [ajint] Index of first Scopdes object found with a Sunid
**                 element matching id, or -1 if identifier is not found.
**
** @release 2.9.0
** @@
******************************************************************************/

static ajint domainScopdesBinSearch(ajint sunid, AjPScopdes const *array,
                                    ajint size)
{
    ajint l = 0;
    ajint m = 0;
    ajint h = size - 1;

    while(l <= h)
    {
        m = (l + h) >> 1;

        if(sunid < array[m]->Sunid)
            h = m - 1;
        else if(sunid > array[m]->Sunid)
            l = m + 1;
        else
            return m;
    }

    return -1;
}




/* @funcstatic domainScopdesDel ***********************************************
**
** Scopdes object destructor.
**
** @param [d] Pscopdes [AjPScopdes*] Scopdes address
**
** @return [void]
** @category delete [AjPScopdes] Default destructor
**
** @release 2.9.0
** @@
******************************************************************************/

static void domainScopdesDel(AjPScopdes *Pscopdes)
{
    if((Pscopdes == NULL) || (*Pscopdes == NULL))
        return;

    if((*Pscopdes)->Type)
        ajStrDel(&((*Pscopdes)->Type));

    if((*Pscopdes)->Sccs)
        ajStrDel(&((*Pscopdes)->Sccs));

    if((*Pscopdes)->Entry)
        ajStrDel(&((*Pscopdes)->Entry));

    if((*Pscopdes)->Desc)
        ajStrDel(&((*Pscopdes)->Desc));

    AJFREE(*Pscopdes);

    *Pscopdes = NULL;

    return;
}




/* @funcstatic domainScopdesCompSunid *****************************************
**
** Function to comapre Scopdes objects by Sunid element.
**
** @param [r] item1 [const void*] Scopdes address 1
** @param [r] item2 [const void*] Scopdes address 2
** @see ajListSort
**
** @return [ajint] -1 if Sunid1 should sort before Sunid2, +1 if the Sunid2
** should sort first. 0 if they are identical in value.
**
** @release 2.9.0
** @@
******************************************************************************/

static ajint domainScopdesCompSunid(const void *item1, const void *item2)
{
    AjPScopdes scopdes1 = *(AjOScopdes *const *) item1;
    AjPScopdes scopdes2 = *(AjOScopdes *const *) item2;

    if(scopdes1->Sunid < scopdes2->Sunid)
        return -1;
    else if(scopdes1->Sunid == scopdes2->Sunid)
        return 0;

    return 1;
}




/* @funcstatic domainCathNameBinSearch ****************************************
**
** Performs a binary search for a domain code over an array of CathName
** structures (which of course must first have been sorted). This is a
** case-insensitive search.
**
** @param [r] identifier [const AjPStr] Search term
** @param [r] array [AjPCathName const*] Array of CathName objects
** @param [r] size [ajint] Size of array
**
** @return [ajint] Index of first CathName object found with an CATH Id code
** matching identifier, or -1 if identifier is not found.
**
** @release 2.9.0
** @@
******************************************************************************/

static ajint domainCathNameBinSearch(const AjPStr identifier,
                                     AjPCathName const *array,
                                     ajint size)
{
    ajint l = 0;
    ajint m = 0;
    ajint h = size - 1;
    ajint c = 0;

    while(l <= h)
    {
        m = (l + h) >> 1;

        if((c = ajStrCmpCaseS(identifier, array[m]->Id)) < 0)
            h = m - 1;
        else if(c > 0)
            l = m + 1;
        else
            return m;
    }

    return -1;
}




/* @funcstatic domainCathDomBinSearch *****************************************
**
** Performs a binary search for a domain code over an array of CathDom
** structures (which of course must first have been sorted). This is a
** case-insensitive search.
**
** @param [r] identifier [const AjPStr] Search term
** @param [r] array [AjPCathDom const*] Array of AjPCathDom objects
** @param [r] size [ajint] Size of array
**
** @return [ajint] Index of first AjPCathDom object found with an domain code
** matching identifier, or -1 if identifier is not found.
**
** @release 2.9.0
** @@
******************************************************************************/

static ajint domainCathDomBinSearch(const AjPStr identifier,
                                    AjPCathDom const *array,
                                    ajint size)
{
    ajint l = 0;
    ajint m = 0;
    ajint h = size - 1;
    ajint c = 0;

    while(l <= h)
    {
        m = (l + h) >> 1;

        if((c = ajStrCmpCaseS(identifier, array[m]->DomainID)) < 0)
            h = m - 1;
        else if(c > 0)
            l = m + 1;
        else
            return m;
    }

    return -1;
}




/* @funcstatic domainCathDomNew ***********************************************
**
** CathDom object constructor. Fore-knowledge of the number of segments is
** required.
**
** @param [r] nsegments [ajuint] Number of segments
**
** @return [AjPCathDom] Pointer to a CathDom object
**
** @release 2.9.0
** @@
******************************************************************************/

static AjPCathDom domainCathDomNew(ajuint nsegments)
{
    ajuint i = 0U;

    AjPCathDom cathdom = NULL;

    AJNEW0(cathdom);

    cathdom->DomainID = ajStrNew();

    if(nsegments > 0)
    {
        AJCNEW0(cathdom->Start, nsegments);
        AJCNEW0(cathdom->End,   nsegments);

        for(i = 0U; i < nsegments; i++)
        {
            cathdom->Start[i] = ajStrNew();
            cathdom->End[i]   = ajStrNew();
        }
    }
    else
    {
        cathdom->Start = NULL;
        cathdom->End   = NULL;
    }

    cathdom->NSegment = nsegments;

    return cathdom;
}




/* @funcstatic domainCathDomDel ***********************************************
**
** Destructor for CathDom object.
**
** @param [w] Pcathdom [AjPCathDom*] Cathdom address
**
** @return [void]
**
** @release 2.9.0
** @@
******************************************************************************/

static void domainCathDomDel(AjPCathDom *Pcathdom)
{
    ajuint i = 0U;

    AjPCathDom cathdom = NULL;

    if((Pcathdom == NULL) || (*Pcathdom == NULL))
        return;

    cathdom = *Pcathdom;

    ajStrDel(&cathdom->DomainID);

    if(cathdom->NSegment)
    {
        for(i = 0U; i < cathdom->NSegment; i++)
        {
            ajStrDel(&cathdom->Start[i]);
            ajStrDel(&cathdom->End[i]);
        }

        AJFREE(cathdom->Start);
        AJFREE(cathdom->End);
    }

    AJFREE(cathdom);

    *Pcathdom = NULL;

    return;
}




/* @funcstatic domainCathNameNew **********************************************
**
** CATH Name object constructor.
**
** @return [AjPCathName] CATH Name object
**
** @release 2.9.0
** @@
******************************************************************************/

static AjPCathName domainCathNameNew(void)
{
    AjPCathName cathname = NULL;

    AJNEW0(cathname);

    cathname->Id   = ajStrNew();
    cathname->Desc = ajStrNew();

    return cathname;
}




/* @funcstatic domainCathNameDel **********************************************
**
** Destructor for a CATH Name object.
**
** @param [d] Pcathname [AjPCathName*] CATH Name address
** @return [void]
**
** @release 2.9.0
** @@
******************************************************************************/

static void domainCathNameDel(AjPCathName *Pcathname)
{
    AjPCathName cathname = NULL;

    if((Pcathname == NULL) || (*Pcathname == NULL))
        return;

    cathname = *Pcathname;

    ajStrDel(&cathname->Id);
    ajStrDel(&cathname->Desc);

    AJFREE(cathname);

    *Pcathname = NULL;

    return;
}




/* @funcstatic domainSortCathNameId *******************************************
**
** Function to compare CATH Name objects by Id element.
**
** @param [r] item1 [const void*] CATH Name address 1
** @param [r] item2 [const void*] CATH Name address 2
** @see ajListSort
**
** @return [ajint] -1 if Id1 should sort before Id2, +1 if the Id2
** should sort first. 0 if they are identical in value.
**
** @release 2.9.0
** @@
******************************************************************************/

static ajint domainSortCathNameId(const void *item1, const void *item2)
{
    AjPCathName cathname1 = *(AjOCathName *const *) item1;
    AjPCathName cathname2 = *(AjOCathName *const *) item2;

    return ajStrCmpS(cathname1->Id, cathname2->Id);
}




/* @funcstatic domainSortCathDomDomainID **************************************
**
** Function to sort CathDom objects by DomainID element.
**
** @param [r] item1 [const void*] CATH Domain address 1
** @param [r] item2 [const void*] CATH Domain address 2
** @see ajListSort
**
** @return [ajint] -1 if DomainID1 should sort before DomainID2, +1 if the
** DomainID2 should sort first. 0 if they are identical in value.
**
** @release 2.9.0
** @@
******************************************************************************/

static ajint domainSortCathDomDomainID(const void *item1, const void *item2)
{
    AjPCathDom cathdom1 = *(AjOCathDom *const *) item1;
    AjPCathDom cathdom2 = *(AjOCathDom *const *) item2;

    return ajStrCmpS(cathdom1->DomainID, cathdom2->DomainID);
}




/* @section Constructors ******************************************************
**
** These constructors return a pointer to a new instance of an object.
**
******************************************************************************/




/* @func ajCathReadAllNew *****************************************************
**
** Reads a DCF file (domain classification file) for CATH domains. This file
** is in DCF format (see documentation for DOMAINATRIX "cathparse"
** application) and creates an AJAX List of AJAX CATH objects for the
** entire content.
**
** @param [u] inf [AjPFile] Pointer to CATH classification file
**
** @return [AjPList] AJAX List of AJAX CATH objects
**
** @release 2.9.0
** @@
******************************************************************************/

AjPList ajCathReadAllNew(AjPFile inf)
{
    AjPList caths = NULL;

    AjPCath cath = NULL;

    /* Check arg's */
    if((!inf))
    {
        ajWarn("Bad args passed to ajCathReadAllNew\n");
        return NULL;
    }

    caths = ajListNew();

    while((cath = ajCathReadCNew(inf, "*")))
        ajListPushAppend(caths, cath);

    return caths;
}




/* @func ajCathReadAllRawNew **************************************************
**
** Reads the CATH parsable files (dir.cla.scop.txt & dir.des.scop.txt) and
** writes an AJAX List of AJAX CATH objects.
**
** @param [u] cathf [AjPFile] Cath class file
** @param [u] domf [AjPFile] Cath domain file
** @param [u] namesf [AjPFile] Output file
** @param [u] flog [AjPFile] Log file
**
** @return [AjPList] AJAX List of AJAX CATH objects
**
** @release 2.9.0
** @@
******************************************************************************/

AjPList ajCathReadAllRawNew(AjPFile cathf, AjPFile domf, AjPFile namesf,
                            AjPFile flog)
{
    AjPList caths = NULL;

    AjPStr CathNameLine    = NULL;  /* String used to hold line from namesf */
    AjPStr CathDomLine     = NULL;  /* String used to hold line from domf   */
    AjPStr CathListLine    = NULL;  /* String used to hold line from cathf  */
    AjPStr tmpDomainID     = NULL;  /* temp ptr to string to hold DomainId
                                       minus "0".                           */
    AjPStr tmpStringDomPtr = NULL;  /* temp ptr to string to hold DXX as a
                                       string.                              */
    AjPStr tmpNumDomPtr    = NULL;  /* temp ptr to string to hold number of
                                       domains as a string.                 */
    ajuint tmpDomInt       = 0U;    /* temp ptr to string to hold number of
                                       domains as int.                      */
    AjPStr tmpDomIDNumDom  = NULL;  /* temp ptr to string to hold DomainId
                                       plus domain number before CathDom
                                       object created.                      */
    ajint  tmpNSegment     = 0;     /* temp ptr to string to hold NSegment
                                       before CathDom object created.       */
    AjPStr NDomAsString    = NULL;  /* temp ptr to string to hold (d)
                                       number of domains (appended onto
                                       tmpDomainID).                        */
    AjPStr tmpNumString    = NULL;  /* temp string used to search CathName
                                       objects. */
    AjPStr tmpNumString1   = NULL;  /* temp string used to search CathName
                                       objects - first part. */
    AjPStr tmpNumString2   = NULL;  /* temp string used to search CathName
                                       objects - second part. */

    AjPStr Search_DomainIDPtr = NULL;  /* temp search string used to search
                                          CathDomList. */
    const AjPStr StrTokPtr  = NULL;

    AjPCathDom  *CathDomArray  = NULL; /* Array to hold sorted CathDomList  */
    AjPCathName *CathNameArray = NULL; /* Array to hold sorted CathNameList */

    /* Initialise Integers */
    ajint idxCathDom = 0;    /* Index to CathDomList array */
    ajint dimCathDom = 0;    /* Dimension of CathDomList array */
    ajint idxCathName= 0;    /* Index to CathNameList array */
    ajint dimCathName= 0;    /* Dimension of CathNameList array */

    ajint intC  = 0;  /* Class number as int */
    ajint intA  = 0;  /* Architecture number as int */
    ajint intT  = 0;  /* Topology number as int */
    ajint intH  = 0;  /* Homologous Superfamily number as int */
    ajint intF  = 0;  /* Family number as int */
    ajint intNI = 0;  /* Near Identical family number as int */
    ajint intI  = 0;  /* Identical family number as int */
    ajuint d    = 0U; /* Declare int for looping through domains */
    ajuint s    = 0U; /* Declare int for looping through segments */
    ajint single_seg = 1;   /* Number of segments when no match found in
                               CathDomList */

    AjPCathName CathNamePtr = NULL;  /* Reusable CathName object pointer */
    AjPCathDom  CathDomPtr  = NULL;  /* Reusable CathDom object pointer */
    AjPCath CathPtr         = NULL;   /* Reusable CathList object pointer */

    AjPList  CathNameList = NULL; /* List containing ptrs to CathName objects */
    AjPList  CathDomList  = NULL; /* List containing ptrs to CathDom objects */

    AjPStrTok handle = NULL;
    AjPStr    tmptok = NULL;

    /* Initialise strings */
    tmptok             = ajStrNew();
    CathNameLine       = ajStrNew();
    CathDomLine        = ajStrNew();
    CathListLine       = ajStrNew();
    tmpDomainID        = ajStrNew();
    tmpStringDomPtr    = ajStrNew();
    tmpNumDomPtr       = ajStrNew();
    tmpDomIDNumDom     = ajStrNew();
    NDomAsString       = ajStrNew();
    tmpNumString       = ajStrNew();
    tmpNumString1      = ajStrNew();
    tmpNumString2      = ajStrNew();
    Search_DomainIDPtr = ajStrNew();

    caths = ajListNew();

    /* Create list for AjSCathName structures (CathNameList) to hold
       data from lines in CAT.names.all.v2.4  */

    CathNameList = ajListNew();
    CathDomList = ajListNew();

    /* Read all the lines in CAT.names.all.v2.4 and populate
    ** CathNameList
    ** 1. Need a loop to read through every line in cathf
    ** ... while(ajFileReadLine)
    ** {
    ** 2. Create a CathName structure ... CathNameNew
    ** 3. Extract data from line and write data structure
    ** 4. Push the CathName pointer onto CathNameList ...
    ** }
    */

    while(ajReadlineTrim(namesf, &CathNameLine))
    {
        CathNamePtr = domainCathNameNew();

        /*1st token is classification index e.g 0002.0160 */
        handle = ajStrTokenNewC(CathNameLine, " \t");
        ajStrTokenNextParseC(&handle, " \t", &(CathNamePtr)->Id);


        /*2nd token is domain code and should be discarded */
        ajStrTokenNextParseC(&handle, " \t", &tmptok);


        /*3rd token is classification text */
        ajStrTokenRestParse(&handle, &(CathNamePtr)->Desc);

        if(CathNamePtr->Desc->Ptr[0] == ':')
            ajStrCutStart(&(CathNamePtr)->Desc, 1);

        ajStrTokenDel(&handle);

#if 0
        /* AJB: This block can be deleted? */
        StrTokPtr = ajStrParseWhite(CathNameLine);
        ajStrAssignS(&(CathNamePtr)->Id, StrTokPtr);
        StrTokPtr = ajStrParseWhite(NULL);
        ajStrTokenRestParse(NULL, &StrTokPtr);
        ajStrAssignS(&(CathNamePtr)->Desc, StrTokPtr);

        if(CathNamePtr->Desc->Ptr[0] == ':')
            ajStrCutStart(&(CathNamePtr)->Desc, 1);

        ajFmtScanS(CathNameLine, "%S", &(CathNamePtr)->Id);
        ajStrAssignSubS(&(CathNamePtr)->Desc, CathNameLine, 28, -1);
        ajStrTrimWhite(&(CathNamePtr)->Desc);

        if(CathNamePtr->Desc->Ptr[0] == ':')
            ajStrCutStart(&(CathNamePtr)->Desc, 1);
#endif

        /* Push pointer to CathName object onto list */
        ajListPush(CathNameList, CathNamePtr);
    }



    /* Sort the list by cath classification number, AjPStr Id */

    /* Sort list using domainSortCathNameId function */
    ajListSort(CathNameList, &domainSortCathNameId);

    /* make list into array and get array size - dimCathName */
    dimCathName = (ajuint) ajListToarray(CathNameList,
					 (void ***) &CathNameArray);


    /* We now have a list that we can do a binary search over */


    /* Create list of AjSCathDom structures (CathDomList) to hold
       data from lines in domlist.v2.4 */


    while(ajReadlineTrim(domf, &CathDomLine))
    {
        /*
        ** 1st token is DomainID e.g 1cuk00
        ** ajStrTok goes through each element of line
        */
        StrTokPtr = ajStrParseWhite(CathDomLine);

        /*
        ** Remove last num from string (0). Assign StrTokPtr to temp ptr
        ** tmpDomainID e.g. 1cuk0
        */
        ajStrAssignSubS(&tmpDomainID, StrTokPtr, 0, 4);


        /*
          ajFmtPrint("tmpDomainID : %S\n", tmpDomainID );
          fflush(stdout);
        */

        StrTokPtr = ajStrParseWhite(NULL);      /*2nd token is no. of domains */
        /* Assign value of StrTokPtr (no. of domains) to tmpStringDomPtr */
        ajStrAssignS(&tmpStringDomPtr, StrTokPtr);
        /*
        ** Remove first character (index 0 = the letter D) from
        ** tmpStringDomPtr and give to tmpDomInt
        */
        ajStrAssignSubS(&tmpNumDomPtr, tmpStringDomPtr, 1, 2);
        ajStrToUint(tmpNumDomPtr, &tmpDomInt);


        /*
        ** ajFmtPrint("tmpDomInt : %u\n", tmpDomInt );
        ** fflush(stdout);
        */

        /*
        ** error-ajStrToInt(tmpDomIntPtr, &(tmpNumDomPtr));
        ** Number of domains expressed as Int and assigned to tmpDomInt
        */

        /*3rd token is no. of fragments, don't need this */
        ajStrParseWhite(NULL);


        for (d = 1; d <= tmpDomInt; d++)    /* For each domain */
        {
            /* Get the number of segments */
            StrTokPtr = ajStrParseWhite(NULL); /* Token= no. of segments */
            /*
            ** ajFmtPrint("StrTokPtr : %S\n", StrTokPtr);
            ** fflush(stdout);
            */


            /*Convert string containing no. of segments to int */
            ajStrToInt(StrTokPtr, &(tmpNSegment));

            /* Create CathDom object giving tmpNSegment as argument */
            CathDomPtr = domainCathDomNew(tmpNSegment);


            /* Converts value of d to a string */
            ajStrFromInt(&NDomAsString, d);

            if(d > 1)
                ajStrCutEnd(&tmpDomainID, 1);


            /*
            ** Append no of domains as a string (NDomAsString) onto tmpDomainID
            */
            ajStrAppendS(&tmpDomainID, NDomAsString);

            /* Assign tmpDomainID to DomainID element in CathDom object */
            ajStrAssignS(&(CathDomPtr->DomainID), tmpDomainID);

            for(s = 0U; s < CathDomPtr->NSegment; s++) /* For each segment */
            {
                /*
                ** get Start and End residue numbers for each segment
                ** nth (starting at token no. 5) token is Chain of
                ** starting residue
                */
                ajStrParseWhite(NULL);

                /* (n + 1)th token is start of segment res number */
                StrTokPtr = ajStrParseWhite(NULL);
                ajStrAssignS(&(CathDomPtr->Start[s]), StrTokPtr);

                /*
                ** Assign Start res no. to Start element in AjPCathDom
                **error- ajStrToInt(StrTokPtr, &(CathDomPtr->Start[s]));
                ** (n+2)th token is "-"
                */
                ajStrParseWhite(NULL);

                /* (n+3)th token is Chain of ending residue */
                ajStrParseWhite(NULL);

                /* (n+4)th token is end of segment res number */
                StrTokPtr = ajStrParseWhite(NULL);
                ajStrAssignS(&(CathDomPtr->End[s]), StrTokPtr);

                /*
                ** Assign End res no. to Start element in AjPCathDom
                ** error-   ajStrToInt(StrTokPtr, &(CathDomPtr->End[s]));*/
                ajStrParseWhite(NULL);          /* (n+5)th token is "-" */
            }

            /*
            ** Read all the lines in domlist.v2.4 and populate CathDomList
            ** Push pointer to CathDom object onto list
            */
            ajListPush(CathDomList, CathDomPtr);
        }
    }


    /*
    ** Sort the list by domain code (ajListSort by DomainId)
    ** We now have a list that we can do a binary search over
    */

    /* Sort list using domainSortCathDomDomainID function */
    ajListSort(CathDomList, &domainSortCathDomDomainID);

    /* make list into array and get array size - dimCathDom */
    dimCathDom = (ajuint) ajListToarray(CathDomList,
					(void ***) &CathDomArray);


    /*
    ** Start of main application loop
    ** while there is a line to read from caths.list.v2.4,
    ** read a line into a string ... ajFileReadLine
    */
    while(ajReadlineTrim(cathf, &CathListLine))
    {
        /*
        ** Extract DomainId from string and write to temp. variable
        ** - Search_DomainIDPtr
        */

        /* DomainID held in temp string */
        ajFmtScanS(CathListLine, "%S", &(Search_DomainIDPtr));


        /*
        ** Binary search of Search_StringPtr against DomainID element
        ** in CathDomList
        */

        /*
        ** Binary search of Search_DomainIDPtr over array of
        ** CathDom objects
        */
        idxCathDom = domainCathDomBinSearch(Search_DomainIDPtr,
                                            CathDomArray, dimCathDom);
        /* sorted by AjPStr Id */
        if(idxCathDom != -1             /*match found */)
        {
            /*
            ** Extract number of segments from CathDom and assign
            ** to tmpNSegment
            */
            (tmpNSegment) = (CathDomArray[idxCathDom]->NSegment);


            /* Allocate the Cath object, AjXyzCathNew */

            /* Create Cath object giving tmpNSegment as argument */
            CathPtr = ajCathNew(tmpNSegment);



            /* Assign DomainId from CathDom to Cath objects */
            ajStrAssignS(&(CathPtr->DomainID),
                         CathDomArray[idxCathDom]->DomainID);
            /* Assign number of segments to NSegment element in Cath object */
            (CathPtr->NSegment) = (CathDomArray[idxCathDom]->NSegment);


            /*
            ** Write the number of segments and start and end points
            **
            ** printf("Number of segments for CathPtr : %d\n",
            ** CathPtr->NSegment);
            ** printf("Number of segments for CathDomPtr : %d\n\n",
            ** CathDomPtr->NSegment);
            */
            for(s = 0U; s < CathPtr->NSegment; s++) /* For each segment */
            {
                /* get Start and End residue numbers for each segment */

                /* Assign value of start from CathDom to CathPtr */
                ajStrAssignS(&(CathPtr->Start[s]),
                             CathDomArray[idxCathDom]->Start[s]);
                /* Assign value of end from CathDom to CathPtr */
                ajStrAssignS(&(CathPtr->End[s]),
                             CathDomArray[idxCathDom]->End[s]);

            }
        }
        /* no match found => only one domain in protein */
        else
        {
            /*
            ** Presume that domain contains a single segment,
            ** single_seg = 1
            */

            /* Allocate the Cath object, AjXyzCathNew */

            /* Create Cath object giving tmpNSegment as argument */
            CathPtr = ajCathNew(single_seg);

            /* Assign DomainId from Search_DomainIDPtr */
            ajStrAssignS(&(CathPtr->DomainID), Search_DomainIDPtr);

            /*
            ** Assign the number of segments as 1
            ** Assign number of segments to NSegment element in Cath
            ** object from single_seg
            */
            (CathPtr->NSegment) = (single_seg);


            /* get Start and End residue numbers for each segment */
            ajStrAssignC(&(CathPtr->Start[0]), ".");
            ajStrAssignC(&(CathPtr->End[0]), ".");
        }


        /* Extract Pdb code from DomainId */
        ajStrAssignSubS(&(CathPtr->Pdb), CathPtr->DomainID, 0, 3);

        /* Extract chain identifier from DomainId */
        CathPtr->Chain = ajStrGetCharPos(CathPtr->DomainID, 4);

        /*
        ** ajStrChar char from string
        ** error-ajStrAssignSubS(&(CathPtr->Chain), CathPtr->DomainID, 4, 4);
        */

        /*
        ** Extract length of domain from string
        ** Take the 9th element of line and assign to Length in Cath object
        */
        ajFmtScanS(CathListLine, "%*S %*d %*d %*d %*d %*d %*d %*d %d",
                   &(CathPtr->Length));


        /*
        ** Extract ajint Class_Id, Arch_Id, Topology_Id, Superfamily_Id,
        ** NIFamily_Id, Family_Id, IFamily_Id from string and write into
        ** AjPCath
        */
        ajFmtScanS(CathListLine, "%*S %d %d %d %d %d %d %d",
                   &intC, &intA, &intT, &intH, &intF, &intNI, &intI);

        (CathPtr->Class_Id)       = (intC);
        (CathPtr->Arch_Id)        = (intA);
        (CathPtr->Topology_Id)   = (intT);
        (CathPtr->Superfamily_Id) = (intH);
        (CathPtr->Family_Id)      = (intF);
        (CathPtr->NIFamily_Id)    = (intNI);
        (CathPtr->IFamily_Id)        = (intI);


        /*
        ** Construct number string for SUPERFAMILY from Class_Id, Arch_Id,
        ** Topology_Id,
        ** Superfamily_Id and store in temp. variable (format X.XX.XX.XX)
        */

        /* Make string containing CATH id numbers */
        ajFmtPrintS(&tmpNumString, "%d.%d.%d.%d", intC, intA, intT, intH);

        /* Binary search using temp. variable in AjSCathName */

        ajFmtPrintF(flog, "%S\n", tmpNumString);

        /* Binary search of tmpNumString over array of CathName objects */
        idxCathName = domainCathNameBinSearch(tmpNumString, CathNameArray,
                                              dimCathName);

        if ( idxCathName != -1)         /*match found */
        {
            /* Extract Superfamily string and write into AJPCath */
            ajStrAssignS(&(CathPtr->Superfamily),
                         (CathNameArray[idxCathName]->Desc));
        }
        else                            /*no match found */
        {
            /* Write Superfamily string as a '.'  */
            ajStrAssignC(&(CathPtr->Superfamily), ".");

            /*error         (CathPtr->Superfamily) = "."); */
        }


        /*
        ** Construct number string for TOPOLOGY from Class_Id,
        ** Arch_Id, Topology_Id,
        ** and store in temp. variable (format X.XX.XX)
        */

        /* Make string containing CAT id numbers */
        ajFmtPrintS(&tmpNumString, "%d.%d.%d", intC, intA, intT);

        /* Binary search using temp. variable in AjSCathName */
        ajFmtPrintF(flog, "%S\n", tmpNumString);
        /* Binary search of tmpNumString over array of CathName objects */
        idxCathName = domainCathNameBinSearch(tmpNumString, CathNameArray,
                                              dimCathName);

        if ( idxCathName != -1)         /* match found */
        {
            /* Extract Topology string  and write into AJPCath */
            ajStrAssignS(&(CathPtr->Topology), CathNameArray[idxCathName]->Desc);
        }
        else                            /* no match found */
        {
            /* Write topology as a '.'  */
            ajStrAssignC(&(CathPtr->Topology), ".");
            /*error         CathPtr->Topology) = "."); */
        }


        /*
        ** Construct number string for ARCHITECTURE from Class_Id, Arch_Id,
        ** and store in temp. variable
        ** Class and Architecture numbers in domlist.v2.4 are in format
        ** XXXX.XXXX
        */

#if 0
        /* AJB: Not keen on the ajFatal comments. Can this be deleted? */

        /*
          if(intC < 10)
          ajFmtPrintS(&tmpNumString1, "000%d", intC);
          else
          ajFatal("MIKE GIVE A ERROR MESSAGE");
          if(intA < 10)
          ajFmtPrintS(&tmpNumString2, "000%d", intA);
          else if(intA < 100)
          ajFmtPrintS(&tmpNumString2, "00%d", intA);
          else if(intA < 1000)
          ajFmtPrintS(&tmpNumString2, "0%d", intA);
          else if(intA < 10000)
          ajFmtPrintS(&tmpNumString2, "%d", intA);
          else
          ajFatal("MIKE GIVE A ERROR MESSAGE");
          ajFmtPrintS(&tmpNumString, "%S.%S", tmpNumString1, tmpNumString2);
        */
#endif

        ajFmtPrintS(&tmpNumString, "%04d.%04d", intC, intA);


        /*
        ** Make string containing CA id numbers
        **      ajFmtPrintS(&tmpNumString, "000%d.%d", intC, intA);
        */

        /* Binary search using temp. variable in AjSCathName */
        ajFmtPrintF(flog, "%S\n", tmpNumString);
        /* Binary search of tmpNumString over array of CathName objects */
        idxCathName = domainCathNameBinSearch(tmpNumString, CathNameArray,
                                              dimCathName);


        if ( idxCathName != -1 )        /* match found */
        {
            /* Extract Architecture string and write into AJPCath */
            ajStrAssignS(&(CathPtr->Architecture),
                         CathNameArray[idxCathName]->Desc);
        }
        else                            /* no match found */
        {
            /* Write architecture as a '.' */
            ajStrAssignC(&(CathPtr->Architecture), ".");
            /*error         (CathPtr->Architecture) = "."); */
        }


        /*
        ** Construct number string for CLASS from Class_Id and store in
        **temp. variable
        */
        if(intC < 10)
            ajFmtPrintS(&tmpNumString, "%04d", intC);
        else
            ajFatal("MIKE GIVE A ERROR MESSAGE");




        /* Binary search using temp. variable in AjSCathName */
        ajFmtPrintF(flog, "%S\n", tmpNumString);
        /* Binary search of tmpNumString over array of CathName objects */
        idxCathName = domainCathNameBinSearch(tmpNumString, CathNameArray,
                                              dimCathName);


        if ( idxCathName != -1)         /* match found */
        {
            /* Extract Class string and write into AJPCath */
            ajStrAssignS(&(CathPtr->Class), CathNameArray[idxCathName]->Desc);
        }
        else                            /* no match found */
        {
            /* Write class as a '.' */
            ajStrAssignC(&(CathPtr->Class), ".");
            /*error         (CathPtr->Class) = "."); */
        }

        /* Push the Cath object onto list */
        ajListPushAppend(caths, CathPtr);

    } /* End of main loop */


    /*
    ** Free the memory for the list and nodes in
    ** list of AjSCathName structures (ajListFree)
    */
    while(ajListPop(CathNameList, (void **) &CathNamePtr))
        domainCathNameDel(&CathNamePtr);
    ajListFree(&CathNameList);

    /*
    ** Free the memory for the list and nodes in
    ** list of AjSCathDom structures (ajListFree)
    */
    while(ajListPop(CathDomList, (void **) &CathDomPtr))
        domainCathDomDel(&CathDomPtr);

    ajListFree(&CathDomList);

    /* Tidy up */
    ajStrDel(&tmptok);
    ajStrDel(&CathNameLine);
    ajStrDel(&CathDomLine);
    ajStrDel(&CathListLine);
    ajStrDel(&tmpStringDomPtr);
    ajStrDel(&tmpNumDomPtr);
    ajStrDel(&tmpDomainID);
    ajStrDel(&tmpDomIDNumDom);
    ajStrDel(&tmpNumString);
    ajStrDel(&tmpNumString1);
    ajStrDel(&tmpNumString2);
    AJFREE(CathDomArray);
    AJFREE(CathNameArray);
    ajStrDel(&Search_DomainIDPtr);
    ajStrDel(&NDomAsString);

    return caths;
}




/* @func ajCathReadCNew *******************************************************
**
** Read a Cath object from a DCF file (domain classification file) in DCF
** format (see documentation for
** DOMAINATRIX "cathparse" application).
**
** @param [u] inf   [AjPFile] AJAX Input file stream
** @param [r] entry [const char*] CATH id of entry to retrieve (or "*" for next
**                                domain in file).
**
** @return [AjPCath] Cath object.
** @category new [AjPCath] Cath constructor from reading dcf format file.
**
** @release 2.9.0
** @@
******************************************************************************/

AjPCath ajCathReadCNew(AjPFile inf, const char *entry)
{
    AjPCath cath = NULL;

    static AjPRegexp exp1  = NULL;
    static AjPRegexp exp2  = NULL;
    static AjPStr domainID = NULL;
    static AjPStr pdb      = NULL;
    static AjPStr class    = NULL;

    static AjPStr architecture = NULL;
    static AjPStr topology     = NULL;
    static AjPStr superfamily  = NULL;

    static AjPStr line   = NULL;
    static AjPStr str    = NULL;
    static AjPStr stmp   = NULL;
    static AjPStr tentry = NULL;

    static AjPStr Acc     = NULL;
    static AjPStr Spr     = NULL;
    static AjPStr SeqPdb  = NULL;
    static AjPStr SeqSpr  = NULL;

    AjBool ok = ajFalse;

    ajint idx = 0;
    ajint n   = 0;
    ajint  Length;      /* No. of residues in domain */

    ajint  Startd;      /* Start of sequence relative to full length
                           swissprot sequence */
    ajint  Endd;        /* End of sequence relative to full length
                           swissprot sequence */

    /* Only initialise strings if this is called for the first time */
    if(!line)
    {
        domainID        = ajStrNew();
        pdb             = ajStrNew();
        class           = ajStrNew();
        architecture    = ajStrNew();
        topology        = ajStrNew();
        superfamily     = ajStrNew();
        line            = ajStrNew();
        str             = ajStrNew();
        tentry          = ajStrNew();
        exp1    = ajRegCompC("^([^ \t\r\n]+)[ \t\n\r]+");
        exp2    = ajRegCompC(
            "^([A-Za-z0-9.]+)[ ]*[^ \t\r\n]+[ ]*([0-9.-]+)[ ]*"
            "[^ \t\r\n]+[ ]*([0-9.-]+)");
        Acc    = ajStrNew();
        Spr     = ajStrNew();
    }

    SeqSpr  = ajStrNew();
    SeqPdb  = ajStrNew();


    ajStrAssignC(&tentry, entry);
    ajStrFmtUpper(&tentry);

    while((ok = ajReadlineTrim(inf, &line)))
    {
        if(!ajStrPrefixC(line, "ID   "))
            continue;

        ajFmtScanS(line, "%*S %S", &stmp);

        if(ajStrMatchWildS(stmp, tentry))
            break;
    }


    if(!ok)
        return NULL;


    while(ok && !ajStrPrefixC(line, "//"))
    {
        if(ajStrPrefixC(line, "XX"))
        {
            ok = ajReadlineTrim(inf, &line);
            continue;
        }

        ajRegExec(exp1, line);
        ajRegPost(exp1, &str);


        if(ajStrPrefixC(line, "ID"))
            ajStrAssignS(&domainID, str);
        else if(ajStrPrefixC(line, "EN"))
            ajStrAssignS(&pdb, str);
        else if(ajStrPrefixC(line, "CL"))
            ajStrAssignS(&class, str);
        else if(ajStrPrefixC(line, "AR"))
        {
            ajStrAssignS(&architecture, str);

            while(ajReadlineTrim(inf, &line))
            {
                if(ajStrPrefixC(line, "XX"))
                    break;

                ajStrAppendC(&architecture, ajStrGetPtr(line) + 3);
            }

            ajStrRemoveWhiteExcess(&architecture);
        }
        else if(ajStrPrefixC(line, "TP"))
        {
            ajStrAssignS(&topology, str);

            while(ajReadlineTrim(inf, &line))
            {
                if(ajStrPrefixC(line, "XX"))
                    break;

                ajStrAppendC(&topology, ajStrGetPtr(line) + 3);
            }

            ajStrRemoveWhiteExcess(&topology);
        }
        else if(ajStrPrefixC(line, "SF"))
        {
            ajStrAssignS(&superfamily, str);

            while(ajReadlineTrim(inf, &line))
            {
                if(ajStrPrefixC(line, "XX"))
                    break;
                ajStrAppendC(&superfamily, ajStrGetPtr(line) + 3);
            }

            ajStrRemoveWhiteExcess(&superfamily);
        }
        else if(ajStrPrefixC(line, "NR"))
            ajFmtScanS(line, "%*S %d", &Length);
        else if(ajStrPrefixC(line, "NC"))
        {
            ajStrToInt(str, &n);
            cath = ajCathNew(n);
            ajStrAssignS(&cath->DomainID, domainID);
            ajStrAssignS(&cath->Pdb, pdb);
            ajStrAssignS(&cath->Class, class);
            ajStrAssignS(&cath->Architecture, architecture);
            ajStrAssignS(&cath->Topology, topology);
            ajStrAssignS(&cath->Superfamily, superfamily);
            cath->Length = Length;
            ajStrAssignS(&cath->Acc, Acc);
            ajStrAssignS(&cath->Spr, Spr);
            ajStrAssignS(&cath->SeqPdb, SeqPdb);
            ajStrAssignS(&cath->SeqSpr, SeqSpr);
        }
        else if(ajStrPrefixC(line, "CN"))
            ajFmtScanS(line, "%*S %*c%d", &idx);
        else if(ajStrPrefixC(line, "CH"))
        {
            if(!ajRegExec(exp2, str))
                return NULL;

            ajRegSubI(exp2, 1, &stmp);
            cath->Chain = *ajStrGetPtr(stmp);
            ajRegSubI(exp2, 2, &str);
            ajStrAssignC(&cath->Start[idx - 1], ajStrGetPtr(str));

            ajRegSubI(exp2, 3, &str);
            ajStrAssignC(&cath->End[idx - 1], ajStrGetPtr(str));

        }
        /* Sequence from pdb file */
        else if(ajStrPrefixC(line, "DS"))
        {
            while((ok = ajReadlineTrim(inf, &line)) && !ajStrPrefixC(line, "XX"))
                ajStrAppendC(&SeqPdb, ajStrGetPtr(line));

            ajStrRemoveWhite(&SeqPdb);
            continue;
        }
        /* Sequence from swissprot */
        else if(ajStrPrefixC(line, "SQ"))
        {
            while((ok = ajReadlineTrim(inf, &line)) && !ajStrPrefixC(line, "XX"))
                ajStrAppendC(&SeqSpr, ajStrGetPtr(line));

            ajStrRemoveWhite(&SeqSpr);
            continue;
        }

        /* Accession number */
        else if(ajStrPrefixC(line, "AC"))
            ajFmtScanS(line, "%*s %S", &Acc);
        /* Swissprot code */
        else if(ajStrPrefixC(line, "SF"))
            ajFmtScanS(line, "%*s %S", &Spr);
        /* Start and end relative to swissprot sequence */
        else if(ajStrPrefixC(line, "RA"))
            ajFmtScanS(line, "%*s %d %*s %d", &Startd, &Endd);

        ok = ajReadlineTrim(inf, &line);
    }

    ajStrDel(&SeqSpr);
    ajStrDel(&SeqPdb);

    return cath;
}




/* @func ajCathReadNew ********************************************************
**
** Read a Cath object from a DCF file (domain classification file) which is
** in DCF format (see documentation for
** DOMAINATRIX "cathparse" application).
**
** @param [u] inf   [AjPFile]  Input file stream
** @param [r] entry [const AjPStr] CATH id of entry to retrieve
**                             (or "*" for next domain in file).
**
** @return [AjPCath] Cath object.
** @category new [AjPCath] Cath constructor from reading dcf format file.
**
** @release 2.9.0
** @@
******************************************************************************/

AjPCath ajCathReadNew(AjPFile inf, const AjPStr entry)
{
    AjPCath cath = NULL;

    if((cath = ajCathReadCNew(inf, entry->Ptr)))
        return cath;

    return NULL;
}




/* @func ajDomainReadAllNew ***************************************************
**
** Reads the DCF file (domain classification file; see documentation
** for DOMAINATRIX "scopparse" application) and creates a list of Domain
** objects for the entire content.
**
** @param [u] inf [AjPFile] Pointer to DOMAIN classification file
**
** @return [AjPList] AJAX List of AJAX Domain objects or NULL
** (file read problem)
**
** @release 3.0.0
** @@
******************************************************************************/

AjPList ajDomainReadAllNew(AjPFile inf)
{
    AjPList domains = NULL;

    AjPDomain domain = NULL;

    AjEDomainType type = ajEDomainTypeNULL;

    /* Check arg's */
    if((!inf))
    {
        ajWarn("Bad args passed to ajDomainReadAllNew\n");
        return NULL;
    }

    type = ajDomainDCFType(inf);

    if((type != ajEDomainTypeSCOP) && (type != ajEDomainTypeCATH))
    {
        ajWarn("Could not ascertain domain type in ajDomainReadNew");
        return NULL;
    }

    domains = ajListNew();

    if(type == ajEDomainTypeSCOP)
        while((domain->Scop = ajScopReadCNew(inf, "*")))
            ajListPushAppend(domains, domain);
    else
        while((domain->Cath = ajCathReadCNew(inf, "*")))
            ajListPushAppend(domains, domain);

    return domains;
}




/* @func ajScopReadAllNew *****************************************************
**
** Reads the DCF file (domain classification file) of SCOP domains. The DCF
** file is in DCF format (see documentation
** for DOMAINATRIX "scopparse" application). The function creates a list of
** cop objects for the entire content.
**
** @param [u] inf  [AjPFile] Pointer to SCOP classification file
**
** @return [AjPList] List of scop objects or NULL (file read problem).
**
** @release 2.9.0
** @@
******************************************************************************/

AjPList ajScopReadAllNew(AjPFile inf)
{
    AjPList scops = NULL;

    AjPScop scop = NULL;

    /* Check arg's */
    if((!inf))
    {
        ajWarn("Bad args passed to ajScopReadAllNew\n");
        return NULL;
    }

    scops = ajListNew();

    while((scop = ajScopReadCNew(inf, "*")))
        ajListPushAppend(scops, scop);

    return scops;
}




/* @func ajScopReadAllRawNew **************************************************
**
** Reads the SCOP parsable files (dir.cla.scop.txt & dir.des.scop.txt) and
** creates a list of Scop objects.
**
** @param [u] claf [AjPFile] Scop class file
** @param [u] desf [AjPFile] Scop description file
** @param [r] omit [AjBool]  True if domains of 1 chain only are output
**
** @return [AjPList] List of Scop objects.
**
** @release 2.9.0
** @@
******************************************************************************/

AjPList ajScopReadAllRawNew(AjPFile claf, AjPFile desf, AjBool omit)
{
    AjPScopcla scopcla = NULL;

    AjPScopdes scopdes = NULL;
    AjPScopdes *desarr = NULL;

    AjPScop scop = NULL;

    AjPList  clalist = NULL;
    AjPList  deslist = NULL;
    AjPList    scops = NULL;

    AjBool   nooutput = ajFalse;
    char     chn;


    ajint  dim = 0;  /* Dimension of array */
    ajint  idx = 0;  /* Index into array */
    ajuint i = 0U;

    clalist = ajListNew();
    deslist = ajListNew();
    scops = ajListNew();


    /* Read the dir.cla.scop.txt file */
    while((scopcla = domainScopclaReadC(claf, "*")))
    {
        ajListPushAppend(clalist, scopcla);
/*      ajFmtPrint(" %d ", scopcla->Domdat); */
    }


    /* Read the dir.des.scop.txt file, sort the list by Sunid
       and convert to an array */
    while((scopdes = domainScopdesReadC(desf, "*")))
    {
        ajListPush(deslist, scopdes);
/*      ajFmtPrint("%d\n", scopdes->Sunid); */
    }


    ajListSort(deslist, &domainScopdesCompSunid);
    dim = (ajuint) ajListToarray(deslist, (void ***) &desarr);


    while(ajListPop(clalist, (void **) &scopcla))
    {
        if(omit)
        {
            if(scopcla->Number > 1)
            {
                chn = scopcla->Chain[0];
                for(nooutput = ajFalse, i = 1; i < scopcla->Number; i++)
                    if(chn != scopcla->Chain[i])
                    {
                        nooutput = ajTrue;
                        break;
                    }
                if(nooutput)
                    continue;
            }
        }

        scop = ajScopNew(scopcla->Number);
        ajStrAssignS(&scop->Entry, scopcla->Entry);
        ajStrAssignS(&scop->Pdb, scopcla->Pdb);

        scop->Sunid_Class       = scopcla->Class;
        scop->Sunid_Fold        = scopcla->Fold;
        scop->Sunid_Superfamily = scopcla->Superfamily;
        scop->Sunid_Family      = scopcla->Family;
        scop->Sunid_Domain      = scopcla->Domain;
        scop->Sunid_Source      = scopcla->Source;
        scop->Sunid_Domdat      = scopcla->Domdat;


        idx = domainScopdesBinSearch(scopcla->Class,  desarr, dim);
        ajStrAssignS(&scop->Class, desarr[idx]->Desc);

        idx = domainScopdesBinSearch(scopcla->Fold,  desarr, dim);
        ajStrAssignS(&scop->Fold, desarr[idx]->Desc);

        idx = domainScopdesBinSearch(scopcla->Superfamily,  desarr, dim);
        ajStrAssignS(&scop->Superfamily, desarr[idx]->Desc);

        idx = domainScopdesBinSearch(scopcla->Family,  desarr, dim);
        ajStrAssignS(&scop->Family, desarr[idx]->Desc);

        idx = domainScopdesBinSearch(scopcla->Domain,  desarr, dim);
        ajStrAssignS(&scop->Domain, desarr[idx]->Desc);

        idx = domainScopdesBinSearch(scopcla->Source,  desarr, dim);
        ajStrAssignS(&scop->Source, desarr[idx]->Desc);

        for(i = 0U; i < scopcla->Number; i++)
        {
            scop->Chain[i] = scopcla->Chain[i];
            ajStrAssignS(&scop->Start[i], scopcla->Start[i]);
            ajStrAssignS(&scop->End[i],   scopcla->End[i]);
        }

        ajListPushAppend(scops, scop);

        domainScopclaDel(&scopcla);
    }

    while(ajListPop(deslist, (void **)&scopdes))
        domainScopdesDel(&scopdes);

    /* Tidy up */
    AJFREE(desarr);
    ajListFree(&clalist);
    ajListFree(&deslist);

    return scops;
}




/* @func ajCathNew ************************************************************
**
** Cath object constructor. Fore-knowledge of the number of chain segments
** the domain is comprised of is required.
**
** @param [r] n [ajuint] No. of chain segments
**
** @return [AjPCath] Pointer to a Cath object
** @category new [AjPCath] Cath default constructor.
**
**
** @release 2.9.0
******************************************************************************/

AjPCath ajCathNew(ajuint n)
{
    ajuint i = 0U;

    AjPCath cath = NULL;

    AJNEW0(cath);

    cath->DomainID     = ajStrNew();
    cath->Pdb          = ajStrNew();
    cath->Class        = ajStrNew();
    cath->Architecture = ajStrNew();
    cath->Topology     = ajStrNew();
    cath->Superfamily  = ajStrNew();
    cath->Acc         = ajStrNew();
    cath->Spr         = ajStrNew();
    cath->SeqPdb      = ajStrNew();
    cath->SeqSpr      = ajStrNew();

    if(n > 0)
    {
        AJCNEW0(cath->Start, n);
        AJCNEW0(cath->End, n);

        for(i = 0U; i < n; i++)
        {
            cath->Start[i] = ajStrNew();
            cath->End[i]   = ajStrNew();
        }
    }

    cath->NSegment = n;

    return cath;
}




/* @func ajDomainNew **********************************************************
**
** Domain object constructor. Fore-knowledge of the number of chains (SCOP
** domains) or chain segments (CATH domains) the domain is comprised of is
** required. This is normally called by the ajDomainReadCNew /
** ajDomainReadNew functions.
**
** @param [r] n [ajuint] Number of chains (SCOP) or chain segments (CATH)
** @param [u] type [AjEDomainType] AJAX Domain Type enuimeration
**
** @return [AjPDomain] Pointer to a Domain object
** @category new [AjPDomain] Domain default constructor.
**
** @release 3.0.0
** @@
******************************************************************************/

AjPDomain ajDomainNew(ajuint n, AjEDomainType type)
{
    AjPDomain domain = NULL;

    if((type != ajEDomainTypeSCOP) && (type != ajEDomainTypeCATH))
    {
        ajWarn("Bad args passed to ajDomainNew");
        return NULL;
    }

    AJNEW0(domain);

    if(type == ajEDomainTypeSCOP)
    {
        domain->Scop = ajScopNew(n);
        domain->Type = ajEDomainTypeSCOP;
    }
    else
    {
        domain->Cath = ajCathNew(n);
        domain->Type = ajEDomainTypeCATH;
    }

    return domain;
}




/* @func ajScopNew ************************************************************
**
** Scop object constructor. Fore-knowledge of the number of chains is
** required. This is normally called by the ajScopReadCNew / ajScopReadNew
** functions.
**
** @param [r] chains [ajuint] Number of chains
**
** @return [AjPScop] Pointer to a Scop object
** @category new [AjPScop] Scop default constructor.
**
** @release 1.8.0
** @@
******************************************************************************/

AjPScop ajScopNew(ajuint chains)
{
    ajuint i = 0U;

    AjPScop scop = NULL;

    AJNEW0(scop);

    scop->Entry       = ajStrNew();
    scop->Pdb         = ajStrNew();
    scop->Class       = ajStrNew();
    scop->Fold        = ajStrNew();
    scop->Superfamily = ajStrNew();
    scop->Family      = ajStrNew();
    scop->Domain      = ajStrNew();
    scop->Source      = ajStrNew();
    scop->Acc         = ajStrNew();
    scop->Spr         = ajStrNew();
    scop->SeqPdb      = ajStrNew();
    scop->SeqSpr      = ajStrNew();
    scop->Sse         = ajStrNew();
    scop->Sss         = ajStrNew();

    if(chains)
    {
        scop->Chain = ajCharNewRes(chains);
        AJCNEW0(scop->Start, chains);
        AJCNEW0(scop->End,   chains);

        for(i = 0U; i < chains; i++)
        {
            scop->Start[i] = ajStrNew();
            scop->End[i]   = ajStrNew();
        }
    }

    scop->Number = chains;

    return scop;
}




/* @func ajDomainReadNew ******************************************************
**
** Read a Domain object from a DCF file (domain classification file - see
** documentation for DOMAINATRIX "scopparse" application).
**
** @param [u] inf   [AjPFile] Input file stream.
** @param [r] entry [const AjPStr] Domain id of domain to read (or "*" for
**                                 next domain in file).
**
** @return [AjPDomain] Domain object.
** @category new [AjPDomain] Domain constructor from reading dcf file.
**
** @release 3.0.0
** @@
******************************************************************************/

AjPDomain ajDomainReadNew(AjPFile inf, const AjPStr entry)
{
    AjEDomainType type = ajEDomainTypeNULL;

    AjPDomain domain = NULL;

    type = ajDomainDCFType(inf);

    if((type != ajEDomainTypeSCOP) && (type != ajEDomainTypeCATH))
    {
        ajWarn("Could not ascertain domain type in ajDomainReadNew");
        return NULL;
    }

    AJNEW0(domain);

    if(type == ajEDomainTypeSCOP)
        domain->Scop = ajScopReadCNew(inf, ajStrGetPtr(entry));
    else
        domain->Cath = ajCathReadCNew(inf, ajStrGetPtr(entry));

    return domain;
}




/* @func ajScopReadNew ********************************************************
**
** Read a Scop object from a DCF file (domain classification file). The DCF
** file is in DCF format (see documentation for
** DOMAINATRIX "scopparse" application).
**
** @param [u] inf   [AjPFile] Input file stream.
** @param [r] entry [const AjPStr] SCOP id of domain to read (or "*" for next
**                                 domain in file).
**
** @return [AjPScop] Scop object.
** @category new [AjPScop] Scop constructor from reading dcf format file.
**
** @release 2.9.0
** @@
******************************************************************************/

AjPScop ajScopReadNew(AjPFile inf, const AjPStr entry)
{
    return ajScopReadCNew(inf, ajStrGetPtr(entry));
}




/* @func ajDomainReadCNew *****************************************************
**
** Read a Domain object from a DCF (domain classification file; see
** documentation for DOMAINATRIX "scopparse" application).
**
** @param [u] inf   [AjPFile] Input file stream
** @param [r] entry [const char*] Domain id of domain to parse
** @param [u] dtype [AjEDomainType] AJAX Domain Type enumeration in the DCF
** file, or ajEDomainTypeNULL if not sure.
**
** @return [AjPDomain] Domain object or NULL (file read problem).
** @category new [AjPDomain] Domain constructor from reading dcf file.
**
** @release 3.0.0
** @@
******************************************************************************/

AjPDomain ajDomainReadCNew(AjPFile inf, const char *entry,
                           AjEDomainType dtype)
{
    AjEDomainType type = ajEDomainTypeNULL;

    AjPDomain domain = NULL;

    if(!dtype)
        type = ajDomainDCFType(inf);
    else
        type = dtype;

    if((type != ajEDomainTypeSCOP) && (type != ajEDomainTypeCATH))
    {
        ajWarn("Could not ascertain domain type in ajDomainReadCNew");
        return NULL;
    }

    AJNEW0(domain);

    domain->Type = type;

    if(domain->Type == ajEDomainTypeSCOP)
        domain->Scop = ajScopReadCNew(inf, entry);
    else
        domain->Cath = ajCathReadCNew(inf, entry);

    if(!domain->Scop && !domain->Cath)
    {
        AJFREE(domain);
        return NULL;
    }

    return domain;
}




/* @func ajScopReadCNew *******************************************************
**
** Read a Scop object from a DCF file (domain classification file) in DCF
** format (see documentation for
** DOMAINATRIX "scopparse" application).
**
** @param [u] inf   [AjPFile] Input file stream
** @param [r] entry [const char*] SCOP id of domain to parse
**
** @return [AjPScop] Scop object or NULL (file read problem).
** @category new [AjPScop] Scop constructor from reading dcf format file.
**
** @release 2.9.0
** @@
******************************************************************************/

AjPScop ajScopReadCNew(AjPFile inf, const char *entry)
{
    AjPScop scop = NULL;

    static AjPRegexp exp1 = NULL;
    static AjPRegexp exp2 = NULL;
    static AjPStr line    = NULL;
    static AjPStr str     = NULL;
    static AjPStr xentry  = NULL;
    static AjPStr source  = NULL;
    static AjPStr class   = NULL;
    static AjPStr fold    = NULL;
    static AjPStr super   = NULL;
    static AjPStr family  = NULL;
    static AjPStr domain  = NULL;
    static AjPStr pdb     = NULL;
    static AjPStr tentry  = NULL;
    static AjPStr stmp    = NULL;
    static AjPStr Acc     = NULL;
    static AjPStr Spr     = NULL;
    static AjPStr SeqPdb  = NULL;
    static AjPStr SeqSpr  = NULL;
    static AjPStr sse     = NULL;
    static AjPStr sss     = NULL;

    AjBool ok             = ajFalse;

    const char *p;
    ajint idx = 0;
    ajint n   = 0;
    ajint Startd;      /* Start of sequence relative to full length
                          swissprot sequence */
    ajint Endd;        /* End of sequence relative to full length
                          swissprot sequence */

    ajint Sunid_Class;         /* SCOP sunid for class */
    ajint Sunid_Fold;          /* SCOP sunid for fold */
    ajint Sunid_Superfamily;   /* SCOP sunid for superfamily */
    ajint Sunid_Family;        /* SCOP sunid for family */
    ajint Sunid_Domain;        /* SCOP sunid for domain */
    ajint Sunid_Source;        /* SCOP sunid for species */
    ajint Sunid_Domdat;        /* SCOP sunid for domain data */


    /* Only initialise strings if this is called for the first time */
    if(!line)
    {
        str     = ajStrNew();
        xentry  = ajStrNew();
        pdb     = ajStrNew();
        source  = ajStrNew();
        class   = ajStrNew();
        fold    = ajStrNew();
        super   = ajStrNew();
        family  = ajStrNew();
        domain  = ajStrNew();
        line    = ajStrNew();
        tentry  = ajStrNew();
        stmp    = ajStrNew();
        Acc     = ajStrNew();
        Spr     = ajStrNew();
        sse     = ajStrNew();
        exp1    = ajRegCompC("^([^ \t\r\n]+)[ \t\n\r]+");
        exp2    = ajRegCompC("^([A-Za-z0-9.]+)[ ]*[^ \t\r\n]+[ ]*"
                             "([0-9.-]+)[ ]*"
                             "[^ \t\r\n]+[ ]*([0-9.-]+)");
    }

    SeqSpr  = ajStrNew();
    SeqPdb  = ajStrNew();
    sss     = ajStrNew();

    ajStrAssignC(&tentry, entry);
    ajStrFmtUpper(&tentry);

    while((ok = ajReadlineTrim(inf, &line)))
    {
        if(!ajStrPrefixC(line, "ID   "))
            continue;

        if(!ajRegExec(exp1, line))
            return NULL;

        ajRegPost(exp1, &stmp);

        if(ajStrMatchWildS(stmp, tentry))
            break;
    }

    if(!ok)
    {
        ajRegFree(&exp1);
        ajRegFree(&exp2);
        ajStrDel(&line);
        ajStrDel(&str);
        ajStrDel(&xentry);
        ajStrDel(&source);
        ajStrDel(&class);
        ajStrDel(&fold);
        ajStrDel(&super);
        ajStrDel(&family);
        ajStrDel(&domain);
        ajStrDel(&pdb);
        ajStrDel(&tentry);
        ajStrDel(&stmp);
        ajStrDel(&Acc);
        ajStrDel(&Spr);
        ajStrDel(&SeqPdb);
        ajStrDel(&SeqSpr);
        ajStrDel(&sse);
        ajStrDel(&sss);
        return NULL;
    }

    while(ok && (!ajStrPrefixC(line, "//")))
    {
        if(ajStrPrefixC(line, "XX"))
        {
            ok = ajReadlineTrim(inf, &line);
            continue;
        }

        /* Empty line */
        if(!(MAJSTRGETLEN(line)))
        {
            ok = ajReadlineTrim(inf, &line);
            continue;
        }

        ajRegExec(exp1, line);
        ajRegPost(exp1, &str);

        if(ajStrPrefixC(line, "ID"))
            ajStrAssignS(&xentry, str);
        else if(ajStrPrefixC(line, "EN"))
            ajStrAssignS(&pdb, str);
        else if(ajStrPrefixC(line, "OS"))
            ajStrAssignS(&source, str);
        else if(ajStrPrefixC(line, "CL"))
            ajStrAssignS(&class, str);
        else if(ajStrPrefixC(line, "FO"))
        {
            ajStrAssignS(&fold, str);
            while((ok = ajReadlineTrim(inf, &line)))
            {
                if(ajStrPrefixC(line, "XX"))
                    break;
                ajStrAppendC(&fold, ajStrGetPtr(line) + 3);
            }
            ajStrRemoveWhiteExcess(&fold);
        }
        else if(ajStrPrefixC(line, "SF"))
        {
            ajStrAssignS(&super, str);
            while((ok = ajReadlineTrim(inf, &line)))
            {
                if(ajStrPrefixC(line, "XX"))
                    break;
                ajStrAppendC(&super, ajStrGetPtr(line) + 3);
            }
            ajStrRemoveWhiteExcess(&super);
        }
        else if(ajStrPrefixC(line, "FA"))
        {
            ajStrAssignS(&family, str);
            while((ok = ajReadlineTrim(inf, &line)))
            {
                if(ajStrPrefixC(line, "XX"))
                    break;
                ajStrAppendC(&family, ajStrGetPtr(line) + 3);
            }
            ajStrRemoveWhiteExcess(&family);
        }
        else if(ajStrPrefixC(line, "DO"))
        {
            ajStrAssignS(&domain, str);
            while((ok = ajReadlineTrim(inf, &line)))
            {
                if(ajStrPrefixC(line, "XX"))
                    break;
                ajStrAppendC(&domain, ajStrGetPtr(line) + 3);
            }
            ajStrRemoveWhiteExcess(&domain);
        }
        else if(ajStrPrefixC(line, "NC"))
        {
            ajStrToInt(str, &n);
            scop = ajScopNew(n);
            ajStrAssignS(&scop->Entry, xentry);
            ajStrAssignS(&scop->Pdb, pdb);
            ajStrAssignS(&scop->Source, source);
            ajStrAssignS(&scop->Class, class);
            ajStrAssignS(&scop->Fold, fold);
            ajStrAssignS(&scop->Domain, domain);
            ajStrAssignS(&scop->Superfamily, super);
            ajStrAssignS(&scop->Family, family);
            ajStrAssignS(&scop->Acc, Acc);
            ajStrAssignS(&scop->Spr, Spr);
            ajStrAssignS(&scop->Sse, sse);
            ajStrAssignS(&scop->Sss, sss);
            ajStrAssignS(&scop->SeqPdb, SeqPdb);
            ajStrAssignS(&scop->SeqSpr, SeqSpr);
            scop->Sunid_Class = Sunid_Class;
            scop->Sunid_Fold = Sunid_Fold;
            scop->Sunid_Superfamily = Sunid_Superfamily;
            scop->Sunid_Family = Sunid_Family;
            scop->Sunid_Domain = Sunid_Domain;
            scop->Sunid_Source = Sunid_Source;
            scop->Sunid_Domdat = Sunid_Domdat;
            scop->Startd       = Startd ;
            scop->Endd         = Endd;
        }
        else if(ajStrPrefixC(line, "CN"))
        {
            p = ajStrGetPtr(str);
            sscanf(p, "[%d]", &idx);
        }
        else if(ajStrPrefixC(line, "CH"))
        {
            if(!ajRegExec(exp2, str))
                return NULL;

            ajRegSubI(exp2, 1, &stmp);
            scop->Chain[idx - 1] = *ajStrGetPtr(stmp);
            ajRegSubI(exp2, 2, &str);
            ajStrAssignC(&(scop)->Start[idx - 1], ajStrGetPtr(str));

            ajRegSubI(exp2, 3, &str);
            ajStrAssignC(&(scop)->End[idx - 1], ajStrGetPtr(str));

        }
        /* Sequence from pdb file */
        else if(ajStrPrefixC(line, "DS"))
        {
            while((ok = ajReadlineTrim(inf, &line)) && !ajStrPrefixC(line, "XX"))
                ajStrAppendC(&SeqPdb, ajStrGetPtr(line));

            ajStrRemoveWhite(&SeqPdb);
            continue;
        }
        /* Sequence from swissprot */
        else if(ajStrPrefixC(line, "SQ"))
        {
            while((ok = ajReadlineTrim(inf, &line)) && !ajStrPrefixC(line, "XX"))
                ajStrAppendC(&SeqSpr, ajStrGetPtr(line));

            ajStrRemoveWhite(&SeqSpr);
            continue;
        }

        /* Accession number */
        else if(ajStrPrefixC(line, "AC"))
            ajFmtScanS(line, "%*s %S", &Acc);

        /* Swissprot code */
        else if(ajStrPrefixC(line, "SF"))
            ajFmtScanS(line, "%*s %S", &Spr);

        /* SSE string */
        else if(ajStrPrefixC(line, "SE"))
            ajFmtScanS(line, "%*s %S", &sse);

        /* SSS string */
        else if(ajStrPrefixC(line, "SS"))
        {
            while((ok = ajReadlineTrim(inf, &line)) && !ajStrPrefixC(line, "XX"))
                ajStrAppendC(&sss, ajStrGetPtr(line));

            ajStrRemoveWhite(&sss);
            continue;
        }

        /* Start and end relative to swissprot sequence */
        else if(ajStrPrefixC(line, "RA"))
            ajFmtScanS(line, "%*s %d %*s %d", &Startd, &Endd);
        /* Sunid of domain data */
        else if(ajStrPrefixC(line, "SI"))
            ajFmtScanS(line, "%*s %d %*s %d %*s %d %*s "
                       "%d %*s %d %*s %d %*s %d",
                       &Sunid_Class, &Sunid_Fold, &Sunid_Superfamily,
                       &Sunid_Family,
                       &Sunid_Domain, &Sunid_Source, &Sunid_Domdat);

        ok = ajReadlineTrim(inf, &line);
    }

    ajStrDel(&SeqSpr);
    ajStrDel(&SeqPdb);
    ajStrDel(&sss);

    return scop;
}




/* @section Structure Destructors *********************************************
**
** These destructors functions receive the address of the instance to be
** deleted.  The original pointer is set to NULL so is ready for re-use.
**
******************************************************************************/




/* @func ajDomainDel **********************************************************
**
** Destructor for an AJAX Domain object.
**
** @param [d] Pdomain [AjPDomain*] AJAX Domain address
**
** @return [void]
** @category delete [AjPDomain] Default AJAX Domain destructor.
**
** @release 3.0.0
** @@
******************************************************************************/

void ajDomainDel(AjPDomain *Pdomain)
{
    AjPDomain domain = NULL;

    if(!Pdomain || !*Pdomain)
        return;

    domain = *Pdomain;

    if(domain->Type == ajEDomainTypeSCOP)
        ajScopDel(&domain->Scop);
    else
        ajCathDel(&domain->Cath);

    AJFREE(domain);

    *Pdomain = NULL;

    return;
}




/* @func ajScopDel ************************************************************
**
** Destructor for an AJAX SCOP object.
**
** @param [d] Pscop [AjPScop*] AJAX SCOP address
**
** @return [void]
** @category delete [AjPScop] Default AJAX SCOP destructor.
**
** @release 1.8.0
** @@
******************************************************************************/

void ajScopDel(AjPScop *Pscop)
{
    ajuint i = 0U;

    AjPScop scop = NULL;

    if(!Pscop || !*Pscop)
        return;

    scop = *Pscop;

    ajStrDel(&scop->Entry);
    ajStrDel(&scop->Pdb);
    ajStrDel(&scop->Class);
    ajStrDel(&scop->Fold);
    ajStrDel(&scop->Superfamily);
    ajStrDel(&scop->Family);
    ajStrDel(&scop->Domain);
    ajStrDel(&scop->Source);
    ajStrDel(&scop->Acc);
    ajStrDel(&scop->Spr);
    ajStrDel(&scop->SeqPdb);
    ajStrDel(&scop->SeqSpr);
    ajStrDel(&scop->Sse);
    ajStrDel(&scop->Sss);

    if(scop->Number)
    {
        for(i = 0U; i < scop->Number; i++)
        {
            ajStrDel(&scop->Start[i]);
            ajStrDel(&scop->End[i]);
        }

        AJFREE(scop->Start);
        AJFREE(scop->End);
        AJFREE(scop->Chain);
    }

    AJFREE(scop);

    *Pscop = NULL;

    return;
}




/* @func ajCathDel ************************************************************
**
** Destructor for an AJAX CATH object.
**
** @param [d] Pcath [AjPCath*] AJAX CATH address
**
** @return [void]
** @category delete [AjPCath] Default AJAX CATH destructor.
**
**
** @release 2.9.0
******************************************************************************/

void ajCathDel(AjPCath *Pcath)
{
    ajuint i = 0U;

    AjPCath cath = NULL;

    if(!Pcath || !*Pcath)
        return;

    cath = *Pcath;

    ajStrDel(&cath->DomainID);
    ajStrDel(&cath->Pdb);
    ajStrDel(&cath->Class);
    ajStrDel(&cath->Architecture);
    ajStrDel(&cath->Topology);
    ajStrDel(&cath->Superfamily);
    ajStrDel(&cath->Acc);
    ajStrDel(&cath->Spr);
    ajStrDel(&cath->SeqPdb);
    ajStrDel(&cath->SeqSpr);

    if(cath->NSegment)
    {
        for(i = 0U; i < cath->NSegment; i++)
        {
            ajStrDel(&cath->Start[i]);
            ajStrDel(&cath->End[i]);
        }

        AJFREE(cath->Start);
        AJFREE(cath->End);
    }

    AJFREE(cath);

    *Pcath = NULL;

    return;
}




/* @section Assignments *******************************************************
**
** These functions overwrite the instance provided as the first argument
** A NULL value is always acceptable so these functions are often used to
** create a new instance by assignment.
**
******************************************************************************/




/* @func ajDomainCopy *********************************************************
**
** Copies the contents from one AJAX Domain object to another.
**
** @param [w] Pto  [AjPDomain*] AJAX Domain address
** @param [r] from [const AjPDomain] AJAX Domain
**
** @return [AjBool] True if copy was successful.
** @category assign [AjPDomain] Replicates an AJAX Domain object.
**
** @release 3.0.0
** @@
******************************************************************************/

AjBool ajDomainCopy(AjPDomain *Pto, const AjPDomain from)
{
    ajuint n = 0U;

    /* Check args */
    if(!from)
        return ajFalse;

    if((*Pto))
        ajDomainDel(Pto);

    if(from->Type == ajEDomainTypeSCOP)
        n = from->Scop->Number;
    else
        n = from->Cath->NSegment;

    (*Pto) = ajDomainNew(n, from->Type);

    (*Pto)->Type = from->Type;

    if(from->Type == ajEDomainTypeSCOP)
        ajScopCopy(&((*Pto)->Scop), from->Scop);
    else
        ajCathCopy(&((*Pto)->Cath), from->Cath);

    return ajTrue;
}




/* @func ajCathCopy ***********************************************************
**
** Copies the contents from one AJAX CATH object to another.
**
** @param [w] Pto  [AjPCath*] AJAX Cath address
** @param [r] from [const AjPCath] AJAX Cath
**
** @return [AjBool] True if copy was successful.
** @category assign [AjPCath] Replicates an AJAX CATH object.
**
** @release 3.0.0
** @@
******************************************************************************/

AjBool ajCathCopy(AjPCath *Pto, const AjPCath from)
{
    ajuint i = 0U;

    /* Check args */
    if(!from)
        return ajFalse;

    if(!*Pto)
        *Pto = ajCathNew(from->NSegment);

    ajStrAssignS(&(*Pto)->DomainID, from->DomainID);
    ajStrAssignS(&(*Pto)->Pdb, from->Pdb);
    ajStrAssignS(&(*Pto)->Class, from->Class);
    ajStrAssignS(&(*Pto)->Architecture, from->Architecture);
    ajStrAssignS(&(*Pto)->Topology, from->Topology);
    ajStrAssignS(&(*Pto)->Superfamily, from->Superfamily);

    (*Pto)->Length = from->Length;
    (*Pto)->Chain  = from->Chain;

    for(i = 0U; i < from->NSegment; i++)
    {
        ajStrAssignS(&(*Pto)->Start[i], from->Start[i]);
        ajStrAssignS(&(*Pto)->End[i],   from->End[i]);
    }

    ajStrAssignS(&(*Pto)->Acc,    from->Acc);
    ajStrAssignS(&(*Pto)->Spr,    from->Spr);
    ajStrAssignS(&(*Pto)->SeqPdb, from->SeqPdb);
    ajStrAssignS(&(*Pto)->SeqSpr, from->SeqSpr);

    (*Pto)->Startd         = from->Startd;
    (*Pto)->Endd           = from->Endd;
    (*Pto)->Class_Id       = from->Class_Id;
    (*Pto)->Arch_Id        = from->Arch_Id;
    (*Pto)->Topology_Id    = from->Topology_Id;
    (*Pto)->Superfamily_Id = from->Superfamily_Id;
    (*Pto)->Family_Id      = from->Family_Id;
    (*Pto)->NIFamily_Id    = from->NIFamily_Id;
    (*Pto)->IFamily_Id     = from->IFamily_Id;

    return ajTrue;
}




/* @func ajScopCopy ***********************************************************
**
** Copies the contents from one AJAX SCOP object to another.
**
** @param [w] Pto  [AjPScop*] AJAX SCOP address
** @param [r] from [const AjPScop] AJAX SCOP
**
** @return [AjBool] True if copy was successful.
** @category assign [AjPScop] Replicates an AJAX SCOP object.
**
** @release 2.9.0
** @@
******************************************************************************/

AjBool ajScopCopy(AjPScop *Pto, const AjPScop from)
{
    ajuint i = 0U;

    /* Check args */
    if(!from)
        return ajFalse;

    if(!*Pto)
        *Pto = ajScopNew(from->Number);

    ajStrAssignS(&(*Pto)->Entry,       from->Entry);
    ajStrAssignS(&(*Pto)->Pdb,         from->Pdb);
    ajStrAssignS(&(*Pto)->Class,       from->Class);
    ajStrAssignS(&(*Pto)->Fold,        from->Fold);
    ajStrAssignS(&(*Pto)->Superfamily, from->Superfamily);
    ajStrAssignS(&(*Pto)->Family,      from->Family);
    ajStrAssignS(&(*Pto)->Domain,      from->Domain);
    ajStrAssignS(&(*Pto)->Source,      from->Source);
    ajStrAssignS(&(*Pto)->Sse,         from->Sse);
    ajStrAssignS(&(*Pto)->Sss,         from->Sss);

    for(i = 0U; i < from->Number; i++)
    {
        (*Pto)->Chain[i] = from->Chain[i];
        ajStrAssignS(&(*Pto)->Start[i], from->Start[i]);
        ajStrAssignS(&(*Pto)->End[i],   from->End[i]);
    }

    ajStrAssignS(&(*Pto)->Acc,    from->Acc);
    ajStrAssignS(&(*Pto)->Spr,    from->Spr);
    ajStrAssignS(&(*Pto)->SeqPdb, from->SeqPdb);
    ajStrAssignS(&(*Pto)->SeqSpr, from->SeqSpr);

    (*Pto)->Startd            = from->Startd;
    (*Pto)->Endd              = from->Endd;
    (*Pto)->Sunid_Class       = from->Sunid_Class;
    (*Pto)->Sunid_Fold        = from->Sunid_Fold;
    (*Pto)->Sunid_Superfamily = from->Sunid_Superfamily;
    (*Pto)->Sunid_Family      = from->Sunid_Family;
    (*Pto)->Sunid_Domain      = from->Sunid_Domain;
    (*Pto)->Sunid_Source      = from->Sunid_Source;
    (*Pto)->Sunid_Domdat      = from->Sunid_Domdat;

    (*Pto)->Score = from->Score;

    return ajTrue;
}




/* @section Modifiers *********************************************************
**
** These functions use the contents of an instance and update them.
**
******************************************************************************/





/* @section Operators *********************************************************
**
** These functions use the contents of an instance but do not make any
** changes.
**
******************************************************************************/




/* @func ajScopMatchSunid *****************************************************
**
** Function to compare AJAX SCOP objects by Sunid_Family member.
**
** @param [r] item1 [const void*] AJAX SCOP address 1
** @param [r] item2 [const void*] AJAX SCOP address 2
** @see ajListSort
**
** @return [ajint] -1 if Start1 should sort before Start2, +1 if the Start2
** should sort first. 0 if they are identical.
** @category use [AjPScop] Sort Scop objects by Sunid_Family member.
**
** @release 2.9.0
** @@
******************************************************************************/

ajint ajScopMatchSunid(const void *item1, const void *item2)
{
    AjPScop scop1 = *(AjOScop *const *) item1;
    AjPScop scop2 = *(AjOScop *const *) item2;

    if(scop1->Sunid_Family < scop2->Sunid_Family)
        return -1;
    else if(scop1->Sunid_Family == scop2->Sunid_Family)
        return 0;

    return 1;
}




/* @func ajScopMatchScopid ****************************************************
**
** Function to comapre AJAX SCOP objects by Entry member.
**
** @param [r] item1 [const void*] AJAX SCOP address 1
** @param [r] item2 [const void*] AJAX SCOP address 2
** @see ajListSort
**
** @return [ajint] -1 if Entry1 should sort before Entry2, +1 if the Entry2
** should sort first. 0 if they are identical in length and content.
** @category use [AjPScop] Sort Scop objects by Entry member.
**
** @release 2.9.0
** @@
******************************************************************************/

ajint ajScopMatchScopid(const void *item1, const void *item2)
{
    AjPScop scop1 = *(AjOScop *const *) item1;
    AjPScop scop2 = *(AjOScop *const *) item2;

    return ajStrCmpS(scop1->Entry, scop2->Entry);
}




/* @func ajScopMatchPdbId *****************************************************
**
** Function to compare AJAX SCOP objects by Pdb member.
**
** @param [r] item1 [const void*] AJAX SCOP address 1
** @param [r] item2 [const void*] AJAX SCOP address 2
** @see ajListSort
**
** @return [ajint] -1 if Pdb1 should sort before Pdb2, +1 if the Pdb2
** should sort first. 0 if they are identical in length and content.
** @category use [AjPScop] Sort Scop objects by Pdb member.
**
** @release 2.9.0
** @@
******************************************************************************/

ajint ajScopMatchPdbId(const void *item1, const void *item2)
{
    AjPScop scop1 = *(AjOScop *const *) item1;
    AjPScop scop2 = *(AjOScop *const *) item2;

    return ajStrCmpS(scop1->Pdb, scop2->Pdb);
}




/* @func ajCathMatchPdbId *****************************************************
**
** Function to compare AJAX CATH objects by Pdb member.
**
** @param [r] item1 [const void*] AJAX CATH address 1
** @param [r] item2 [const void*] AJAX CATH address 2
** @see ajListSort
**
** @return [ajint] -1 if Pdb1 should sort before Pdb2, +1 if the Pdb2
** should sort first. 0 if they are identical in length and content.
** @category use [AjPScop] Sort AJAX CATH objects by Pdb member.
**
** @release 2.9.0
** @@
******************************************************************************/

ajint ajCathMatchPdbId(const void *item1, const void *item2)
{
    AjPCath cath1 = *(AjOCath *const *) item1;
    AjPCath cath2 = *(AjOCath *const *) item2;

    return ajStrCmpS(cath1->Pdb, cath2->Pdb);
}




/* @section Casts *************************************************************
**
** These functions examine the contents of an instance and return some
** derived information. Some of them provide access to the internal
** components of an instance.
**
******************************************************************************/




/* @section Reporters *********************************************************
**
** These functions return the contents of an instance but do not make any
** changes.
**
******************************************************************************/




/* @func ajDomainGetId ********************************************************
**
** Returns domain id, either DomainID element (Cath object) or Entry
** (Scop object).
**
** @param [r] domain [const AjPDomain] AJAX Domain
**
** @return [AjPStr] DomainID, Entry or NULL on error
**
** @release 3.0.0
** @@
******************************************************************************/

AjPStr ajDomainGetId(const AjPDomain domain)
{
    if(!domain)
    {
        ajWarn("Bad args to ajDomainGetId");
        return NULL;
    }

    if(domain->Type == ajEDomainTypeSCOP)
        return(domain->Scop->Entry);
    else if(domain->Type == ajEDomainTypeCATH)
        return(domain->Cath->DomainID);
    else
        ajWarn("Domain type not resolved in ajDomainGetId");

    return NULL;
}




/* @func ajDomainGetSeqPdb ****************************************************
**
** Returns the pdb sequence from an AJAX Domain object.
**
** @param [r] domain [const AjPDomain] AJAX Domain
**
** @return [AjPStr] PDB Sequence or NULL on error
**
** @release 3.0.0
** @@
******************************************************************************/

AjPStr ajDomainGetSeqPdb(const AjPDomain domain)
{
    if(!domain)
    {
        ajWarn("Bad args to ajDomainGetSeqPdb");
        return NULL;
    }

    if(domain->Type == ajEDomainTypeSCOP)
        return(domain->Scop->SeqPdb);
    else if(domain->Type == ajEDomainTypeCATH)
        return(domain->Cath->SeqPdb);
    else
        ajWarn("Domain type not resolved in ajDomainGetSeqPdb");

    return NULL;
}




/* @func ajDomainGetSeqSpr ****************************************************
**
** Returns the swissprot sequence from an AJAX Domain object.
**
** @param [r] domain [const AjPDomain] AJAX Domain
**
** @return [AjPStr] Swiss-Prot sequence or NULL on error
**
** @release 3.0.0
** @@
******************************************************************************/

AjPStr ajDomainGetSeqSpr(const AjPDomain domain)
{
    if(!domain)
    {
        ajWarn("Bad args to ajDomainGetSeqSpr");
        return NULL;
    }

    if(domain->Type == ajEDomainTypeSCOP)
        return(domain->Scop->SeqSpr);
    else if(domain->Type == ajEDomainTypeCATH)
        return(domain->Cath->SeqSpr);
    else
        ajWarn("Domain type not resolved in ajDomainGetSeqSpr");

    return NULL;
}




/* @func ajDomainGetPdb *******************************************************
**
** Returns the Pdb element from an AJAX Domain object.
**
** @param [r] domain [const AjPDomain] AJAX Domain
**
** @return [AjPStr] Pdb or NULL on error.
**
** @release 3.0.0
** @@
******************************************************************************/

AjPStr ajDomainGetPdb(const AjPDomain domain)
{
    if(!domain)
    {
        ajWarn("Bad args to ajDomainGetSeqPdb");
        return NULL;
    }

    if(domain->Type == ajEDomainTypeSCOP)
        return(domain->Scop->Pdb);
    else if(domain->Type == ajEDomainTypeCATH)
        return(domain->Cath->Pdb);
    else
        ajWarn("Domain type not resolved in ajDomainGetSeqPdb");

    return NULL;
}




/* @func ajDomainGetAcc *******************************************************
**
** Returns the Acc element from a Domain object.
**
** @param [r] domain [const AjPDomain] AJAX Domain
**
** @return [AjPStr] Acc or NULL on error
**
** @release 3.0.0
** @@
******************************************************************************/

AjPStr ajDomainGetAcc(const AjPDomain domain)
{
    if(!domain)
    {
        ajWarn("Bad args to ajDomainGetSeqPdb");
        return NULL;
    }

    if(domain->Type == ajEDomainTypeSCOP)
        return(domain->Scop->Acc);
    else if(domain->Type == ajEDomainTypeCATH)
        return(domain->Cath->Acc);
    else
        ajWarn("Domain type not resolved in ajDomainGetSeqPdb");

    return NULL;
}




/* @func ajDomainGetSpr *******************************************************
**
** Returns the Spr element from an AJAX Domain object.
**
** @param [r] domain [const AjPDomain] AJAX Domain
**
** @return [AjPStr] Spr or NULL on error
**
** @release 3.0.0
** @@
******************************************************************************/

AjPStr ajDomainGetSpr(const AjPDomain domain)
{
    if(!domain)
    {
        ajWarn("Bad args to ajDomainGetSeqPdb");
        return NULL;
    }

    if(domain->Type == ajEDomainTypeSCOP)
        return(domain->Scop->Spr);
    else if(domain->Type == ajEDomainTypeCATH)
        return(domain->Cath->Spr);
    else
        ajWarn("Domain type not resolved in ajDomainGetSeqPdb");

    return NULL;
}




/* @func ajDomainGetN *********************************************************
**
** Returns no. chains or chain segments in a domain, either NSegment element
** (Cath domains) or N element (Scop domains).
**
** @param [r] domain [const AjPDomain] AJAX Domain
**
** @return [ajint] Or -1 on error.
**
** @release 3.0.0
** @@
******************************************************************************/

ajint ajDomainGetN(const AjPDomain domain)
{
    if(!domain)
    {
        ajWarn("Bad args to ajDomainGetSeqPdb");
        return -1;
    }

    if(domain->Type == ajEDomainTypeSCOP)
        return(domain->Scop->Number);
    else if(domain->Type == ajEDomainTypeCATH)
        return(domain->Cath->NSegment);
    else
        ajWarn("Domain type not resolved in ajDomainGetSeqPdb");

    return -1;
}




/* @func ajScopArrFindScopid **************************************************
**
** Performs a binary search for a SCOP domain id over an array of Scop
** structures (which of course must first have been sorted). This is a
** case-insensitive search.
**
** @param [r] array [AjPScop const *] Array of AJAX SCOP objects
** @param [r] size [ajint] Size of array
** @param [r] identifier [const AjPStr] Search term
**
** @return [ajint] Index of first Scop object found with a SCOP domain id
** matching id, or -1 if id is not found.
** @category use [AjPScop*] Binary search for Entry element over array of
**                         Scop objects.
**
** @release 2.9.0
** @@
******************************************************************************/

ajint ajScopArrFindScopid(AjPScop const *array, ajint size,
                          const AjPStr identifier)

{
    ajint l = 0;
    ajint m = 0;
    ajint h = size - 1;
    ajint c = 0;

    while(l <= h)
    {
        m = (l + h) >> 1;

        if((c = ajStrCmpCaseS(identifier, array[m]->Entry)) < 0)
            h = m - 1;
        else if(c > 0)
            l = m + 1;
        else
            return m;
    }

    return -1;
}




/* @func ajScopArrFindSunid ***************************************************
**
** Performs a binary search for a SCOP sunid over an array of Scop
** objects (which of course must first have been sorted). This is a
** case-insensitive search.
**
** @param [r] array [AjPScop const *] Array of AJAX SCOP objects
** @param [r] size [ajint] Size of array
** @param [r] identifier [ajint] Search term
**
** @return [ajint] Index of first Scop object found with an PDB code
** matching id, or -1 if id is not found.
** @category use [AjPScop*] Binary search for Sunid_Family element
**                         over array of Scop objects.
**
** @release 2.9.0
** @@
******************************************************************************/

ajint ajScopArrFindSunid(AjPScop const *array, ajint size, ajint identifier)
{
    ajint l = 0;
    ajint m = 0;
    ajint h = size - 1;

    while(l <= h)
    {
        m = (l + h) >> 1;

        if(identifier < array[m]->Sunid_Family)
            h = m - 1;
        else if(identifier > array[m]->Sunid_Family)
            l = m + 1;
        else
            return m;
    }

    return -1;
}




/* @func ajScopArrFindPdbid ***************************************************
**
** Performs a binary search for a SCOP domain id over an array of Scop
** objects (which of course must first have been sorted). This is a
** case-insensitive search.
**
** @param [r] array [AjPScop const*] Array of AJAX SCOP objects
** @param [r] size [ajint] Size of array
** @param [r] identifier [const AjPStr] Search term
**
** @return [ajint] Index of first Scop object found with a PDB code
** matching id, or -1 if id is not found.
** @category use [AjPScop*] Binary search for Pdb element over array
**                         of Scop objects.
**
** @release 2.9.0
** @@
******************************************************************************/

ajint ajScopArrFindPdbid(AjPScop const *array, ajint size,
                         const AjPStr identifier)
{
    ajint l = 0;
    ajint m = 0;
    ajint h = size - 1;
    ajint c = 0;

    while(l <= h)
    {
        m = (l + h) >> 1;

        if((c = ajStrCmpCaseS(identifier, array[m]->Pdb)) < 0)
            h = m - 1;
        else if(c > 0)
            l = m + 1;
        else
            return m;
    }

    return -1;
}




/* @func ajCathArrFindPdbid ***************************************************
**
** Performs a binary search for a CATH domain id over an array of Cath
** structures (which of course must first have been sorted). This is a
** case-insensitive search.
**
** @param [r] array [AjPCath const*] Array of AjPCath objects
** @param [r] size [ajint] Size of array
** @param [r] identifier [const AjPStr] Search term
**
** @return [ajint] Index of first Cath object found with a PDB code
** matching id, or -1 if id is not found.
** @category use [AjPCath*] Binary search for Pdb element over array
**                         of Cath objects.
**
** @release 2.9.0
** @@
******************************************************************************/

ajint ajCathArrFindPdbid(AjPCath const *array, ajint size,
                         const AjPStr identifier)
{
    ajint l = 0;
    ajint m = 0;
    ajint h = size - 1;
    ajint c = 0;

    while(l <= h)
    {
        m= (l + h) >> 1;

        if((c = ajStrCmpCaseS(identifier, array[m]->Pdb)) < 0)
            h = m - 1;
        else if(c > 0)
            l = m + 1;
        else
            return m;
    }

    return -1;
}




/* @section Input and Output **************************************************
**
** These functions use the contents of an instance but do not make any
** changes.
**
******************************************************************************/




/* @func ajPdbWriteDomain *****************************************************
**
** Writes a clean coordinate file for a SCOP domain. Where coordinates for
** multiple models (e.g. NMR structures) are given, data for model 1 are
** written.
** In the clean file, the coordinates are presented as belonging to a single
** chain regardless of how many chains the domain comprised.
** Coordinates for heterogens are NOT written to file.
**
** @param [u] outf [AjPFile] Output file stream
** @param [r] pdb  [const AjPPdb]  Pdb object
** @param [r] scop [const AjPScop] Scop object
** @param [u] errf [AjPFile] Output file stream for error messages
**
** @return [AjBool] True on success
**
** @release 1.9.0
** @@
**
******************************************************************************/

AjBool ajPdbWriteDomain(AjPFile outf, const AjPPdb pdb,
                        const AjPScop scop, AjPFile errf)
{
    /*
    ** rn_mod is a modifier to the residue number to give correct residue
    ** numbering for the domain
    */
    ajuint i      = 0U;
    ajuint chn    = 0U;
    ajint start   = 0;
    ajint end     = 0;
    ajuint finalrn = 0U;
    ajint rn_mod  = 0;
    ajint last_rn = 0;
    ajint this_rn = 0;
    char  id      = '\0';

    AjPStr tmpseq = NULL;
    AjPStr seq    = NULL;
    AjPStr tmpstr = NULL;

    AjBool      found_start = ajFalse;
    AjBool      found_end   = ajFalse;
    AjBool      nostart     = ajFalse;
    AjBool      noend       = ajFalse;
    AjIList     iter        = NULL;
    AjPAtom     atom1       = NULL;
    AjPAtom     atom2       = NULL;
    AjPResidue  residue1    = NULL;
    AjPResidue  residue2    = NULL;
/*  AjPResidue *resarr      = NULL; */
    AjPSeqout   outseq      = NULL;

    if(!errf || !scop || !pdb || !outf)
        ajFatal("Bad args passed to ajPdbWriteDomain");

    /* Initialise strings */
    seq    = ajStrNew();
    tmpseq = ajStrNew();
    tmpstr = ajStrNew();

    /* Check for unknown or zero-length chains */
    for(i = 0U; i < scop->Number; i++)
        if(!ajPdbChnidToNum(scop->Chain[i], pdb, &chn))
        {
            ajWarn("Chain incompatibility error in "
                   "ajPdbWriteDomain");

            ajFmtPrintF(errf, "//\n%S\nERROR Chain incompatibility "
                        "error in ajPdbWriteDomain\n", scop->Entry);
            ajStrDel(&seq);
            ajStrDel(&tmpseq);
            ajStrDel(&tmpstr);

            return ajFalse;
        }
        else if(pdb->Chains[chn - 1]->Nres == 0)
        {
            ajWarn("Chain length zero");

            ajFmtPrintF(errf, "//\n%S\nERROR Chain length zero\n",
                        scop->Entry);
            ajStrDel(&seq);
            ajStrDel(&tmpseq);
            ajStrDel(&tmpstr);

            return ajFalse;
        }



    /* Write header info. to domain coordinate file */
    ajFmtPrintF(outf, "%-5s%S\n", "ID", scop->Entry);
    ajFmtPrintF(outf, "XX\n");
    ajFmtPrintF(outf, "%-5sCo-ordinates for SCOP domain %S\n",
                "DE", scop->Entry);
    ajFmtPrintF(outf, "XX\n");
    ajFmtPrintF(outf, "%-5sSee Escop.dat for domain classification\n",
                "OS");
    ajFmtPrintF(outf, "XX\n");
    ajFmtPrintF(outf, "%-5sMETHOD ", "EX");

    if(pdb->Method == ajEPdbMethodXray)
        ajFmtPrintF(outf, "xray; ");
    else
        ajFmtPrintF(outf, "nmr_or_model; ");

    ajFmtPrintF(outf, "RESO %.2f; NMOD 1; NCHN 1; NGRP 0;\n",
                pdb->Reso);

    /* The NCHN and NMOD are hard-coded to 1 for domain files */



    /* Start of main application loop */
    /* Print out data up to coordinates list */
    for(i = 0U;
        i < scop->Number;
        i++, found_start = ajFalse, found_end = ajFalse,
            nostart = ajFalse, noend = ajFalse, last_rn = 0)
    {
        /*
        ** Unknown or Zero sized chains have already been checked for
        ** so no additional checking is needed here
        */
        ajPdbChnidToNum(scop->Chain[i], pdb, &chn);


        /* Initialise the iterator */
        iter = ajListIterNewread(pdb->Chains[chn - 1]->Residues);


        /*
        ** If start of domain is unspecified
        ** then assign start to first residue in chain
        */
        if(!ajStrCmpC(scop->Start[i], "."))
        {
            nostart     = ajTrue;
            start       = 1;
            found_start = ajTrue;
        }


        /*
        ** If end of domain is unspecified
        ** then assign end to last residue in chain
        */
        if(!ajStrCmpC(scop->End[i], "."))
        {
            noend = ajTrue;
            end = pdb->Chains[chn - 1]->Nres;
            found_end = ajTrue;
        }


        /* Find start and end of domains in chain */
        if(!found_start || !found_end)
        {
            /* Iterate through the list of atoms */
            while((residue1 = (AjPResidue) ajListIterGet(iter)))
            {
                /*
                ** Hard-coded to work on model 1
                ** Break if model no. != 1
                */
                if(residue1->Mod != 1 || (found_start && found_end))
                    break;

                /* if(residue1->Type != 'P' || residue1->Mod != 1
                   || (found_start && found_end))
                   break; */


                /* If we are onto a new residue */
                this_rn = residue1->Idx;

                if(this_rn != last_rn)
                {
                    last_rn = this_rn;

                    /*
                    ** The start position was specified, but has not
                    ** been found yet
                    */
                    if(!found_start && !nostart)
                    {
                        ajStrAssignS(&tmpstr, scop->Start[i]);
                        ajStrAppendK(&tmpstr, '*');

                        /* Start position found  */
                        /* if(!ajStrCmpCaseS(residue1->Pdb, scop->Start[i])) */
                        if(ajStrMatchWildS(residue1->Pdb, tmpstr))
                        {
                            if(!ajStrMatchS(residue1->Pdb, scop->Start[i]))
                            {
                                ajWarn("Domain start found by wildcard "
                                       "match only "
                                       "in ajPdbWriteDomain");
                                ajFmtPrintF(errf, "//\n%S\nERROR Domain "
                                            "start found "
                                            "by wildcard match only in "
                                            "ajPdbWriteDomain\n",
                                            scop->Entry);
                            }

                            start = residue1->Idx;
                            found_start = ajTrue;
                        }
                        else
                            continue;
                    }


                    /*
                    ** The end position was specified, but has not
                    ** been found yet
                    */
                    if(!found_end && !noend)
                    {
                        ajStrAssignS(&tmpstr, scop->End[i]);
                        ajStrAppendK(&tmpstr, '*');

                        /* End position found */
                        /* if(!ajStrCmpCaseS(residue1->Pdb, scop->End[i])) */
                        if(ajStrMatchWildS(residue1->Pdb, tmpstr))
                        {
                            if(!ajStrMatchS(residue1->Pdb, scop->End[i]))
                            {
                                ajWarn("Domain end found by wildcard "
                                       "match only "
                                       "in ajPdbWriteDomain");
                                ajFmtPrintF(errf, "//\n%S\nERROR Domain end "
                                            "found "
                                            "by wildcard match only in "
                                            "ajPdbWriteDomain\n",
                                            scop->Entry);
                            }

                            end = residue1->Idx;
                            found_end = ajTrue;
                            break;
                        }
                    }
                }
            }
        }


        /* Diagnostics if start position was not found */
        if(!found_start)
        {
            ajStrDel(&seq);
            ajStrDel(&tmpseq);
            ajStrDel(&tmpstr);
            ajListIterDel(&iter);
            ajWarn("Domain start not found in ajPdbWriteDomain");
            ajFmtPrintF(errf, "//\n%S\nERROR Domain start not found "
                        "in in ajPdbWriteDomain\n", scop->Entry);

            return ajFalse;
        }


        /* Diagnostics if end position was not found */
        if(!found_end)
        {
            ajStrDel(&seq);
            ajStrDel(&tmpseq);
            ajStrDel(&tmpstr);
            ajListIterDel(&iter);
            ajWarn("Domain end not found in ajPdbWriteDomain");
            ajFmtPrintF(errf, "//\n%S\nERROR Domain end not found "
                        "in ajPdbWriteDomain\n", scop->Entry);

            return ajFalse;
        }


        /* Write <seq> string here */
        ajStrAssignSubS(&tmpseq, pdb->Chains[chn - 1]->Seq, start - 1, end - 1);
        ajStrAppendS(&seq, tmpseq);



        /* Free the iterator */
        ajListIterDel(&iter);
    }
    /* End of main application loop */


    /*
    ** If the domain was composed of more than once chain then a '.' is
    ** given as the chain identifier
    */
    if(scop->Number > 1)
        id = '.';
    else
    {
        id = pdb->Chains[chn - 1]->Id;
        if(id == ' ')
            id = '.';
    }


    /* Write sequence to domain coordinate file */
    ajFmtPrintF(outf, "XX\n");
    ajFmtPrintF(outf, "%-5s[1]\n", "CN");
    ajFmtPrintF(outf, "XX\n");
    /*
      ajFmtPrintF(outf, "%-5sID %c; NR %d; NL 0; NH %d; NE %d; NS %d; NT %d;\n",
      "IN",
      id,
      ajStrGetLen(seq),
      pdb->Chains[chn - 1]->numHelices,
      pdb->Chains[chn - 1]->numStrands,
      pdb->Chains[chn - 1]->numSheets,
      pdb->Chains[chn - 1]->numTurns);
    */
    ajFmtPrintF(outf, "%-5sID %c; NRES %d; NL 0; NH %d; NE %d;\n",
                "IN",
                id,
                ajStrGetLen(seq),
                pdb->Chains[chn - 1]->numHelices,
                pdb->Chains[chn - 1]->numStrands);
    ajFmtPrintF(outf, "XX\n");

    outseq = ajSeqoutNewFile(outf);
    ajSeqoutDumpSwisslike(outseq, seq, "SQ");
    ajSeqoutDel(&outseq);

    ajFmtPrintF(outf, "XX\n");



    /* Write residue list to domain coordinate file */
    for(nostart = ajFalse, noend = ajFalse, i = 0U;
        i < scop->Number;
        i++, found_start = ajFalse, found_end = ajFalse)
    {
        /*
        ** Unknown or Zero length chains have already been checked for
        ** so no additional checking is needed here
        */
        ajPdbChnidToNum(scop->Chain[i], pdb, &chn);

        /* Initialise the iterator */
        iter = ajListIterNewread(pdb->Chains[chn - 1]->Residues);


        /* Increment res. counter from last chain if appropriate */
        if(noend)
            rn_mod += residue2->Idx;
        else
            rn_mod += finalrn;


        /* Check whether start and end of domain are specified */
        if(!ajStrCmpC(scop->Start[i], "."))
            nostart = ajTrue;
        else
            nostart = ajFalse;

        if(!ajStrCmpC(scop->End[i], "."))
            noend = ajTrue;
        else
            noend = ajFalse;


        /* Iterate through the list of residues */
        while((residue1 = (AjPResidue) ajListIterGet(iter)))
        {
            /* Break if model no. != 1 */
            if(residue1->Mod != 1)
                break;

            /* The start position has not been found yet */
            if(!found_start)
            {
                /* Start position was specified */
                if(!nostart)
                {
                    ajStrAssignS(&tmpstr, scop->Start[i]);
                    ajStrAppendK(&tmpstr, '*');

                    /* Start position found */
                    /* if(!ajStrCmpCaseS(residue1->Pdb, scop->Start[i])) */
                    if(ajStrMatchWildS(residue1->Pdb, tmpstr))
                    {
                        if(!ajStrMatchS(residue1->Pdb, scop->Start[i]))
                        {
                            ajWarn("Domain start found by wildcard match only "
                                   "in ajPdbWriteDomain");
                            ajFmtPrintF(errf, "//\n%S\nERROR Domain "
                                        "start found "
                                        "by wildcard match only in "
                                        "ajPdbWriteDomain\n", scop->Entry);
                        }


                        rn_mod -= residue1->Idx-1;
                        found_start = ajTrue;
                    }
                    else
                        continue;
                }
                else
                    found_start = ajTrue;
            }


            /*
            ** The end position was specified, but has not
            ** been found yet
            */
            if(!found_end && !noend)
            {
                ajStrAssignS(&tmpstr, scop->End[i]);
                ajStrAppendK(&tmpstr, '*');

                /* End position found */
                /* if(!ajStrCmpCaseS(residue1->Pdb, scop->End[i])) */
                if(ajStrMatchWildS(residue1->Pdb, tmpstr))
                {
                    if(!ajStrMatchS(residue1->Pdb, scop->End[i]))
                    {
                        ajWarn("Domain end found by wildcard match only "
                               "in ajPdbWriteDomain");
                        ajFmtPrintF(errf, "//\n%S\nERROR Domain end found "
                                    "by wildcard match only in "
                                    "ajPdbWriteDomain\n", scop->Entry);
                    }

                    found_end = ajTrue;
                    finalrn   = residue1->Idx;
                }
            }
            /*
            ** The end position was specified and has been found, and
            ** the current atom no longer belongs to this final residue
            */
            else if(residue1->Idx != finalrn && !noend)
                break;

            ajFmtPrintF(outf, "%-5s%-5d%-5d%-5d%-6S%-2c%-6S",
                        "RE",
                        residue1->Mod, /* It will always be 1 */
                        1, /* chn number is always given as 1 */
                        residue1->Idx+rn_mod,
                        residue1->Pdb,
                        residue1->Id1,
                        residue1->Id3);


            if(residue1->eNum != 0)
                ajFmtPrintF(outf, "%-5d", residue1->eNum);
            else
                ajFmtPrintF(outf, "%-5c", '.');
            ajFmtPrintF(outf, "%-5S%-5c", residue1->eId, residue1->eType);

            if(residue1->eType == 'H')
                ajFmtPrintF(outf, "%-5d", residue1->eClass);
            else
                ajFmtPrintF(outf, "%-5c", '.');

            if(residue1->eStrideNum != 0)
                ajFmtPrintF(outf, "%-5d", residue1->eStrideNum);
            else
                ajFmtPrintF(outf, "%-5c", '.');
            ajFmtPrintF(outf, "%-5c", residue1->eStrideType);


            ajFmtPrintF(outf, "%8.2f%8.2f%8.2f%8.2f%8.2f"
                        "%8.2f%8.2f%8.2f%8.2f%8.2f%8.2f%8.2f"
                        "%8.2f\n",
                        residue1->Phi,
                        residue1->Psi,
                        residue1->Area,
                        residue1->all_abs,
                        residue1->all_rel,
                        residue1->side_abs,
                        residue1->side_rel,
                        residue1->main_abs,
                        residue1->main_rel,
                        residue1->npol_abs,
                        residue1->npol_rel,
                        residue1->pol_abs,
                        residue1->pol_rel);

            /* Assign pointer for this chain */
            residue2 = residue1;
        }

        ajListIterDel(&iter);
    }


    /* Write atom list to domain coordinate file */
    for(nostart = ajFalse, noend = ajFalse, i = 0U;
        i < scop->Number;
        i++, found_start = ajFalse, found_end = ajFalse)
    {
        /*
        ** Unknown or Zero length chains have already been checked for
        ** so no additional checking is needed here
        */
        ajPdbChnidToNum(scop->Chain[i], pdb, &chn);
/*
  if(resarr)
  AJFREE(resarr);
  ajListToarray(pdb->Chains[chn-1]->Residues, (void ***) &resarr);
*/


        /* Initialise the iterator */
        iter = ajListIterNewread(pdb->Chains[chn-1]->Atoms);


        /* Increment res. counter from last chain if appropriate */
        if(noend)
            rn_mod += atom2->Idx;
        else
            rn_mod += finalrn;


        /* Check whether start and end of domain are specified */
        if(!ajStrCmpC(scop->Start[i], "."))
            nostart = ajTrue;
        else
            nostart = ajFalse;

        if(!ajStrCmpC(scop->End[i], "."))
            noend = ajTrue;
        else
            noend = ajFalse;


        /* Iterate through the list of atoms */
        while((atom1 = (AjPAtom) ajListIterGet(iter)))
        {
            /*
            ** Continue if a non-protein atom is found or break if
            ** model no. != 1
            */
            if(atom1->Mod != 1)
                break;

            if(atom1->Type != 'P')
                continue;


            /*  if(atom1->Mod != 1 || atom1->Type != 'P')
                break; */



            /* The start position has not been found yet */
            if(!found_start)
            {
                /* Start position was specified */
                if(!nostart)
                {
                    ajStrAssignS(&tmpstr, scop->Start[i]);
                    ajStrAppendK(&tmpstr, '*');

                    /* Start position found */
/*                  if(!ajStrCmpCaseS(atom1->Pdb, scop->Start[i])) */
                    if(ajStrMatchWildS(atom1->Pdb, tmpstr))
/*                  if(ajStrMatchWildS(resarr[atom1->Idx - 1]->Pdb, tmpstr)) */
                    {
                        if(!ajStrMatchS(atom1->Pdb, scop->Start[i]))
                            /*
                              if(!ajStrMatchS(resarr[atom1->Idx - 1]->Pdb,
                              scop->Start[i]))
                            */
                        {
                            ajWarn("Domain start found by wildcard match only "
                                   "in ajPdbWriteDomain");
                            ajFmtPrintF(errf, "//\n%S\nERROR Domain "
                                        "start found "
                                        "by wildcard match only in "
                                        "ajPdbWriteDomain\n", scop->Entry);
                        }


                        rn_mod -= atom1->Idx-1;
                        found_start = ajTrue;
                    }
                    else
                        continue;
                }
                else
                    found_start = ajTrue;
            }


            /*
            ** The end position was specified, but has not
            ** been found yet
            */
            if(!found_end && !noend)
            {
                ajStrAssignS(&tmpstr, scop->End[i]);
                ajStrAppendK(&tmpstr, '*');

                /* End position found */
                /* if(!ajStrCmpCaseS(atom1->Pdb, scop->End[i])) */
                if(ajStrMatchWildS(atom1->Pdb, tmpstr))
                    /* if(ajStrMatchWildS(resarr[atom1->Idx - 1]->Pdb, tmpstr)) */
                {
                    if(!ajStrMatchS(atom1->Pdb, scop->End[i]))
                        /* if(!ajStrMatchS(resarr[atom1->Idx - 1]->Pdb, scop->End[i])) */
                    {
                        ajWarn("Domain end found by wildcard match only "
                               "in ajPdbWriteDomain");
                        ajFmtPrintF(errf, "//\n%S\nERROR Domain end found "
                                    "by wildcard match only in "
                                    "ajPdbWriteDomain\n", scop->Entry);
                    }

                    found_end = ajTrue;
                    finalrn   = atom1->Idx;
                }
            }
            /*
            ** The end position was specified and has been found, and
            ** the current atom no longer belongs to this final residue
            */
            else if(atom1->Idx != finalrn && !noend)
                break;



            /* Print out coordinate line */
            ajFmtPrintF(outf, "%-5s%-5d%-5d%-5c%-5d%-6S%-2c%-6S%-2c%-6S"
                        "%9.3f%9.3f%9.3f%8.2f%8.2f\n",
                        "AT",
                        atom1->Mod, /* It will always be 1 */
                        atom1->Chn, /* chn number is always given as 1 */
                        '.',
                        atom1->Idx+rn_mod,
                        atom1->Pdb,
                        atom1->Id1,
                        atom1->Id3,
                        atom1->Type,
                        atom1->Atm,
                        atom1->X,
                        atom1->Y,
                        atom1->Z,
                        atom1->O,
                        atom1->B);

            /* Assign pointer for this chain */
            atom2 = atom1;
        }

        ajListIterDel(&iter);
    }



    /* Write last line in file */
    ajFmtPrintF(outf, "//\n");


    /* Tidy up */
/*
  if(resarr)
  AJFREE(resarr);
*/
    ajStrDel(&seq);
    ajStrDel(&tmpseq);
    ajStrDel(&tmpstr);

    return ajTrue;
}




/* @func ajCathWrite **********************************************************
**
** Write contents of an AJAX CATH object to an output DCF file (domain
** classification file) in DCF format (see documentation for DOMAINATRIX
** "cathparse" application).
**
** @param [u] outf [AjPFile] AJAX Output file stream
** @param [r] cath [const AjPCath] AJAX CATH
**
** @return [AjBool] True if file was written ok.
**
** @release 2.9.0
** @@
******************************************************************************/

AjBool ajCathWrite(AjPFile outf, const AjPCath cath)
{
    ajuint i = 0U;

    AjPStr tmp = NULL;

    AjPSeqout outseq = NULL;

    /* Check args */
    if(!outf || !cath)
    {
        ajWarn("Bad args passed to ajCathWrite");
        return ajFalse;
    }

    tmp = ajStrNew();

    ajStrAssignS(&tmp, cath->DomainID);
    ajStrFmtUpper(&tmp);
    ajFmtPrintF(outf, "ID   %S\nXX\n", tmp);

    ajStrAssignS(&tmp, cath->Pdb);
    ajStrFmtUpper(&tmp);
    ajFmtPrintF(outf, "EN   %S\nXX\n", tmp);
    ajFmtPrintF(outf, "TY   CATH\nXX\n");
    ajFmtPrintF(outf, "CI   %d CL; %d AR; %d TP; %d SF; %d FA; %d NI;"
                "%d IF;\nXX\n",
                cath->Class_Id, cath->Arch_Id, cath->Topology_Id,
                cath->Superfamily_Id, cath->Family_Id, cath->NIFamily_Id,
                cath->IFamily_Id);

    ajFmtPrintF(outf, "CL   %S\n", cath->Class);
    ajFmtPrintF(outf, "XX\n");
    ajFmtPrintSplit(outf, cath->Architecture, "AR   ", 75, " \t\n\r");
    ajFmtPrintF(outf, "XX\n");
    ajFmtPrintSplit(outf, cath->Topology, "TP   ", 75, " \t\n\r");
    ajFmtPrintF(outf, "XX\n");
    ajFmtPrintSplit(outf, cath->Superfamily, "SF   ", 75, " \t\n\r");
    ajFmtPrintF(outf, "XX\n");

    if(ajStrGetLen(cath->SeqPdb))
    {
        ajFmtPrintF(outf, "XX\n");
        outseq = ajSeqoutNewFile(outf);
        ajSeqoutDumpSwisslike(outseq, cath->SeqPdb, "DS");
        ajSeqoutDel(&outseq);
    }

    ajFmtPrintF(outf, "NR   %d\n", cath->Length);

    /*
      ajFmtPrintF(outf, "CL   %S", cath->Class);
      ajFmtPrintSplit(outf, cath->Architecture, "\nXX\nAR   ", 75, " \t\n\r");
      ajFmtPrintSplit(outf, cath->Topology, "XX\nTP   ", 75, " \t\n\r");
      ajFmtPrintSplit(outf, cath->Superfamily, "XX\nSF   ", 75, " \t\n\r");
      ajFmtPrintF(outf, "XX\nNR   %d\n", cath->Length);
    */


    if(ajStrGetLen(cath->Acc))
        ajFmtPrintF(outf, "XX\nAC   %S\n", cath->Acc);

    if(ajStrGetLen(cath->Spr))
        ajFmtPrintF(outf, "XX\nSP   %S\n", cath->Spr);

    if(ajStrGetLen(cath->SeqSpr))
    {
        ajFmtPrintF(outf, "XX\n%-5s%d START; %d END;\n", "RA", cath->Startd,
                    cath->Endd);
        ajFmtPrintF(outf, "XX\n");
        outseq = ajSeqoutNewFile(outf);
        ajSeqoutDumpSwisslike(outseq, cath->SeqSpr, "SQ");
        ajSeqoutDel(&outseq);
    }


    ajFmtPrintF(outf, "XX\nNC   %d\n", cath->NSegment);

    for(i = 0U; i < cath->NSegment; i++)
    {
        ajFmtPrintF(outf, "XX\nCN   [%d]\n", i + 1);

        ajFmtPrintF(outf, "XX\nCH   %c CHAIN; %S START; %S END;\n",
                    cath->Chain,
                    cath->Start[i],
                    cath->End[i]);
    }

    ajFmtPrintF(outf, "//\n");

    ajStrDel(&tmp);

    return ajTrue;
}




/* @func ajDomainWrite ********************************************************
**
** Write contents of an AJAX Domain object to an output file in DCF format
** (see documentation for DOMAINATRIX "scopparse" application).
**
** @param [u] outf [AjPFile] Output file stream
** @param [r] domain [const AjPDomain] AJAX Domain
**
** @return [AjBool] True if file was written ok.
**
** @release 3.0.0
** @@
******************************************************************************/

AjBool ajDomainWrite(AjPFile outf, const AjPDomain domain)
{
    if(!outf || !domain)
    {
        ajWarn("Bad args passed to ajDomainWrite");
        return ajFalse;
    }

    if(domain->Type == ajEDomainTypeSCOP)
        ajScopWrite(outf, domain->Scop);
    else
        ajCathWrite(outf, domain->Cath);

    return ajTrue;
}




/* @func ajScopWrite **********************************************************
**
** Write contents of an AJAX SCOP object to a DCF file (domain classification
** file).  The DCF file uses DCF format (see documentation for DOMAINATRIX
** "scopparse" application).
**
** @param [u] outf [AjPFile] Output file stream
** @param [r] scop [const AjPScop] AJAX SCOP
**
** @return [AjBool] True if file was written ok.
**
** @release 1.8.0
** @@
******************************************************************************/

AjBool ajScopWrite(AjPFile outf, const AjPScop scop)
{
    ajuint i = 0U;

    AjPSeqout outseq = NULL;

    if(!outf || !scop)
    {
        ajWarn("Bad args passed to ajScopWrite");

        return ajFalse;
    }


    ajFmtPrintF(outf, "ID   %S\nXX\n", scop->Entry);
    ajFmtPrintF(outf, "EN   %S\nXX\n", scop->Pdb);
    ajFmtPrintF(outf, "TY   SCOP\nXX\n");
    ajFmtPrintF(outf, "SI   %d CL; %d FO; %d SF; %d FA; %d DO; %d SO; "
                "%d DD;\nXX\n",
                scop->Sunid_Class, scop->Sunid_Fold, scop->Sunid_Superfamily,
                scop->Sunid_Family, scop->Sunid_Domain, scop->Sunid_Source,
                scop->Sunid_Domdat);

    ajFmtPrintF(outf, "CL   %S\n", scop->Class);
    ajFmtPrintF(outf, "XX\n");
    ajFmtPrintSplit(outf, scop->Fold, "FO   ", 75, " \t\n\r");
    ajFmtPrintF(outf, "XX\n");
    ajFmtPrintSplit(outf, scop->Superfamily, "SF   ", 75, " \t\n\r");
    ajFmtPrintF(outf, "XX\n");
    ajFmtPrintSplit(outf, scop->Family, "FA   ", 75, " \t\n\r");
    ajFmtPrintF(outf, "XX\n");
    ajFmtPrintSplit(outf, scop->Domain, "DO   ", 75, " \t\n\r");;
    ajFmtPrintF(outf, "XX\nOS   %S\n", scop->Source);

    if(ajStrGetLen(scop->SeqPdb))
    {
        ajFmtPrintF(outf, "XX\n");
        outseq = ajSeqoutNewFile(outf);
        ajSeqoutDumpSwisslike(outseq, scop->SeqPdb, "DS");
        ajSeqoutDel(&outseq);
    }

    if(ajStrGetLen(scop->Acc))
        ajFmtPrintF(outf, "XX\nAC   %S\n", scop->Acc);

    if(ajStrGetLen(scop->Spr))
        ajFmtPrintF(outf, "XX\nSP   %S\n", scop->Spr);

    if(ajStrGetLen(scop->SeqSpr))
    {
        ajFmtPrintF(outf, "XX\n%-5s%d START; %d END;\n", "RA", scop->Startd,
                    scop->Endd);
        ajFmtPrintF(outf, "XX\n");
        outseq = ajSeqoutNewFile(outf);
        ajSeqoutDumpSwisslike(outseq, scop->SeqSpr, "SQ");
        ajSeqoutDel(&outseq);
    }

    if(ajStrGetLen(scop->Sse))
        ajFmtPrintF(outf, "XX\nSE   %S\n", scop->Sse);

    if(ajStrGetLen(scop->Sss))
        ajFmtPrintF(outf, "XX\n");

    if(ajStrGetLen(scop->Sss))
    {
        outseq = ajSeqoutNewFile(outf);
        ajSeqoutDumpSwisslike(outseq, scop->Sss, "SS");
        ajSeqoutDel(&outseq);
    }

    ajFmtPrintF(outf, "XX\nNC   %d\n", scop->Number);

    for(i = 0U; i < scop->Number; i++)
    {
        ajFmtPrintF(outf, "XX\nCN   [%u]\n", i + 1);
        ajFmtPrintF(outf, "XX\nCH   %c CHAIN; %S START; %S END;\n",
                    scop->Chain[i],
                    scop->Start[i],
                    scop->End[i]);
    }
    ajFmtPrintF(outf, "//\n");

    return ajTrue;
}




/* @section Miscellaneous *****************************************************
**
** These functions may have diverse functions that do not fit into the other
** categories.
**
******************************************************************************/




/* @func ajDomainDCFType ******************************************************
**
** Reads a DCF file (domain classification file; see documentation for
** DOMAINATRIX "scopparse" application) and ascertains the type of domains
** (ajEDomainTypeSCOP or ajEDomainTypeCATH) within.
**
**
** @param [u] inf [AjPFile] Input file stream
**
** @return [AjEDomainType] AJAX Domain Type enumeration ajEDomainTypeSCOP,
** ajEDomainTypeCATH or ajEDomainTypeNULL (error) as appropriate.
**
** @release 3.0.0
** @@
******************************************************************************/

AjEDomainType ajDomainDCFType(AjPFile inf)
{
    ajlong offset = 0;

    AjPStr line = NULL;
    AjPStr tmp  = NULL;


    /* Only initialise strings if this is called for the first time */
    if(!line)
    {
        line = ajStrNew();
        tmp  = ajStrNew();
    }

    offset = ajFileResetPos(inf);

    while((ajReadlineTrim(inf, &line)))
    {
        if(!ajStrPrefixC(line, "TY   "))
            continue;

        ajFmtScanS(line, "%*S %S", &tmp);

        if(ajStrMatchC(tmp, "SCOP"))
        {
            ajFileSeek(inf, offset, 0);
            ajStrDel(&line);
            ajStrDel(&tmp);
            return ajEDomainTypeSCOP;
        }

        else if(ajStrMatchC(tmp, "CATH"))
        {
            ajFileSeek(inf, offset, 0);
            ajStrDel(&line);
            ajStrDel(&tmp);
            return ajEDomainTypeCATH;
        }
        else
        {
            ajWarn("Serious error: Unknown domain type in DCF file");
            ajStrDel(&line);
            ajStrDel(&tmp);
            return ajEDomainTypeNULL;
        }
    }

    ajStrDel(&line);
    ajStrDel(&tmp);

    return ajEDomainTypeNULL;
}




/* @func ajDomainExit *********************************************************
**
** Cleanup of Domain function internals.
**
** @return [void]
**
** @release 6.1.0
** @@
******************************************************************************/

void ajDomainExit(void)
{
    ajStrDel(&domainGStrline);
    ajStrDel(&domainGStrsunidstr);
    ajStrDel(&domainGStrtentry);
    ajStrDel(&domainGStrtmp);
    ajStrDel(&domainGStrscopid);
    ajStrDel(&domainGStrpdbid);
    ajStrDel(&domainGStrchains);
    ajStrDel(&domainGStrsccs);
    ajStrDel(&domainGStrclass);
    ajStrDel(&domainGStrtoken);
    ajStrDel(&domainGStrstr);

    ajRegFree(&domainGRegexp);
    ajRegFree(&domainGRegrexp);

    return;
}




/* @func ajDomainDummyFunction ************************************************
**
** Dummy function to catch all unused functions defined in the ajdomain
** source file.
**
** @return [void]
**
**
** @release 2.9.0
******************************************************************************/

void ajDomainDummyFunction(void)
{
    AjPStr str = NULL;
    AjPFile file = NULL;

    domainScopclaRead(file, str);
    domainScopdesRead(file, str);

    return;
}
