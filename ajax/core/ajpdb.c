/* @source ajpdb **************************************************************
**
** AJAX low-level functions for handling protein structural data.
** For use with the Atom, Chain and Pdb objects defined in ajpdb.h
** Also for use with Hetent, Het, Vdwres, Vdwall, Cmap and Pdbtosp objects
** (also defined in ajpdb.h).
** Includes functions for reading and writing ccf (clean coordinate file)
** format.
** 
** @author Copyright (C) 2004 Jon Ison (jison@hgmp.mrc.ac.uk) 
** @version $Revision: 1.58 $ 
** @modified $Date: 2012/12/07 10:16:44 $ by $Author: rice $
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




/* ======================================================================= */
/* ============================ include files ============================ */
/* ======================================================================= */

#include "ajlib.h"

#include "ajpdb.h"
#include "ajbase.h"
#include "ajfileio.h"
#include "ajseqwrite.h"

#include <limits.h>
#include <string.h>




/* ======================================================================= */
/* ============================ private data ============================= */
/* ======================================================================= */

#define CMAP_MODE_I 1
#define CMAP_MODE_C 2

static AjPStr pdbGStrline = NULL;       /* Line of text     */
static AjPStr pdbGStrtemp_id = NULL;    /* Temp. protein id */
static AjPStr pdbGStrtemp_domid = NULL; /* Temp. domain id  */
static AjPStr pdbGStrtemp_ligid = NULL; /* Temp. ligand id  */
static AjPStr pdbGStrdesc = NULL;       /* Ligand description, SITES output
                                         * only */
static AjPStr pdbGStrtype = NULL;       /* Type of contact  */
static AjPStr pdbGStrtmpstr = NULL;     /* Housekeeping */




/* ======================================================================= */
/* ================= Prototypes for private functions ==================== */
/* ======================================================================= */

static ajint pdbSortPdbtospPdb(const void *item1, const void *item2);




/* ======================================================================= */
/* ========================== private functions ========================== */
/* ======================================================================= */




/* @funcstatic pdbSortPdbtospPdb **********************************************
**
** Function to compare AJAX PDB to Swiss-Prot objects by Pdb member.
** Usually called by ajPdbtospReadAllNew.
**
** @param [r] item1 [const void*] AJAX PDB to Swiss-Prot address 1
** @param [r] item2 [const void*] AJAX PDB to Swiss-Prot address 2
** @see ajListSort
**
** @return [ajint] -1 if Pdb1 should sort before Pdb2,
**                 +1 if the Pdb2 should sort first.
**                  0 if they are identical in length and content.
**
** @release 2.9.0
** @@
******************************************************************************/

static ajint pdbSortPdbtospPdb(const void *item1, const void *item2)
{
    AjPPdbtosp pdbtosp1 = *(AjOPdbtosp *const *) item1;
    AjPPdbtosp pdbtosp2 = *(AjOPdbtosp *const *) item2;

    return ajStrCmpS(pdbtosp1->Pdb, pdbtosp2->Pdb);
}




/* ======================================================================= */
/* =========================== constructors ============================== */
/* ======================================================================= */




/* @section Constructors ******************************************************
**
** All constructors return a pointer to a new instance. It is the
** responsibility of the user to first destroy any previous instance. The
** target pointer does not need to be initialised to NULL, but it is good
** programming practice to do so anyway.
**
******************************************************************************/




/* @func ajPdbtospReadAllRawNew ***********************************************
**
** Reads the swissprot:pdb equivalence table available at URL (1)
**  (1) http://www.expasy.ch/cgi-bin/lists?pdbtosp.txt
** and returns the data as a list of Pdbtosp objects.
**
** @param [u] inf [AjPFile] Input file
**
** @return [AjPList] AJAX List of AJAX PDB to Swiss-Prot (AjPPdbtosp) objects
**
** @release 2.9.0
** @@
**
******************************************************************************/

AjPList ajPdbtospReadAllRawNew(AjPFile inf)
{
    AjPList list = NULL;        /* List of Pdbtosp objects to return */
    AjPPdbtosp pdbtosp = NULL;  /* Temp. pointer to Pdbtosp object */
    AjPStr pdb = NULL;          /* PDB identifier */
    AjPStr spr = NULL;          /* Swissprot identifier */
    AjPStr acc = NULL;          /* Accession number */
    AjPStr line = NULL;         /* Line from file */
    AjPStr token = NULL;        /* Token from line */
    const AjPStr subtoken = NULL;       /* Token from line */
    AjPList acclist = NULL;     /* List of accession numbers */
    AjPList sprlist = NULL;     /* List of swissprot identifiers */
    ajint n = 0;                /* No. of accession numbers for current pdb
                                 * code */
    AjBool ok = ajFalse;        /* True if "____  _" has been found and we
                                 * can start parsing */
    AjBool done_1st = ajFalse;  /* True if the first line of data has been
                                 * parsed */


    /* Memory allocation */
    line = ajStrNew();
    token = ajStrNew();
    subtoken = ajStrNew();
    pdb = ajStrNew();
    acclist = ajListstrNew();
    sprlist = ajListstrNew();
    list = ajListNew();


    /* Read lines from file */
    while (ajReadlineTrim(inf, &line))
    {
        if (ajStrPrefixC(line, "____  _"))
        {
            ok = ajTrue;
            continue;
        }


        if (!ok)
            continue;

        if (ajStrMatchC(line, ""))
            break;


        /* Read in pdb code first.  Then tokenise by ':', discard the first
         * token, then tokenise the second token by ',', parsing out the
         * swissprot codes and accession numbers from the subtokens */


        /* Make sure this is a line containing the pdb code */
        if ((ajStrFindC(line, ":") != -1))
        {
            if (done_1st)
            {
                pdbtosp = ajPdbtospNew(0);
                ajStrAssignS(&pdbtosp->Pdb, pdb);
                pdbtosp->Number = n;
                ajListToarray(acclist, (void ***) &pdbtosp->Acc);
                ajListToarray(sprlist, (void ***) &pdbtosp->Spr);
                ajListPushAppend(list, (void *) pdbtosp);

                ajListstrFree(&acclist);
                ajListstrFree(&sprlist);
                acclist = ajListstrNew();
                sprlist = ajListstrNew();

                n = 0;
            }

            ajFmtScanS(line, "%S", &pdb);

            ajStrParseC(line, ":");
            ajStrAssignS(&token, ajStrParseC(NULL, ":"));

            done_1st = ajTrue;
        }
        else
        {
            ajStrAssignS(&token, line);
        }

        spr = ajStrNew();
        acc = ajStrNew();
        ajFmtScanS(token, "%S (%S", &spr, &acc);

        if (ajStrSuffixC(acc, "),"))
            ajStrCutEnd(&acc, 2);
        else
            ajStrCutEnd(&acc, 1);

        ajListstrPushAppend(acclist, acc);
        ajListstrPushAppend(sprlist, spr);
        n++;

        ajStrParseC(token, ",");

        while ((subtoken = ajStrParseC(NULL, ",")))
        {
            spr = ajStrNew();
            acc = ajStrNew();

            ajFmtScanS(subtoken, "%S (%S", &spr, &acc);

            if (ajStrSuffixC(acc, "),"))
                ajStrCutEnd(&acc, 2);
            else
                ajStrCutEnd(&acc, 1);

            ajListstrPushAppend(acclist, acc);
            ajListstrPushAppend(sprlist, spr);
            n++;
        }
    }

    /* Data for last pdb code ! */
    pdbtosp = ajPdbtospNew(0);
    ajStrAssignS(&pdbtosp->Pdb, pdb);
    pdbtosp->Number = n;

    ajListToarray(acclist, (void ***) &pdbtosp->Acc);
    ajListToarray(sprlist, (void ***) &pdbtosp->Spr);
    ajListPushAppend(list, (void *) pdbtosp);
    ajListstrFree(&acclist);
    ajListstrFree(&sprlist);

    ajStrDel(&line);
    ajStrDel(&token);
    ajStrDel(&pdb);
    ajListstrFree(&acclist);
    ajListstrFree(&sprlist);

    return list;
}




/* @func ajPdbtospReadNew *****************************************************
**
** Read a Pdbtosp object from a file in embl-like format (see documentation
** for DOMAINATRIX "pdbtosp" application).
**
** @param [u] inf [AjPFile] Input file stream
** @param [r] entry [const AjPStr] Pdb id
**
** @return [AjPPdbtosp] True on success
**
** @release 2.9.0
** @@
******************************************************************************/

AjPPdbtosp ajPdbtospReadNew(AjPFile inf, const AjPStr entry)
{
    return ajPdbtospReadCNew(inf, ajStrGetPtr(entry));
}




/* @func ajPdbtospReadCNew ****************************************************
**
** Read a Pdbtosp object from a file in embl-like format.  Memory for the
** object is allocated.
**
** @param [u] inf   [AjPFile] Input file stream
** @param [r] entry [const char*]   Pdb id
**
** @return [AjPPdbtosp] True on success
**
** @release 2.9.0
** @@
******************************************************************************/

AjPPdbtosp ajPdbtospReadCNew(AjPFile inf, const char *entry)
{
    AjPPdbtosp pdbtosp = NULL;

    AjPStr line = NULL;
    AjPStr tentry = NULL;
    AjPStr pdb = NULL;
    AjBool ok = ajFalse;
    ajint n = 0;
    ajint i = 0;

    line = ajStrNew();
    tentry = ajStrNew();
    pdb = ajStrNew();

    ajStrAssignC(&tentry, entry);
    ajStrFmtUpper(&tentry);

    while ((ok = ajReadlineTrim(inf, &line)))
    {
        if (!ajStrPrefixC(line, "EN   "))
            continue;

        ajFmtScanS(line, "%*S %S", &pdb);

        if (ajStrMatchWildS(pdb, tentry))
            break;
    }

    if (!ok)
    {
        ajStrDel(&line);
        ajStrDel(&tentry);
        ajStrDel(&pdb);

        return NULL;
    }

    while (ok && !ajStrPrefixC(line, "//"))
    {
        if (ajStrPrefixC(line, "XX"))
        {
            ok = ajReadlineTrim(inf, &line);
            continue;
        }
        else if (ajStrPrefixC(line, "NE"))
        {
            ajFmtScanS(line, "%*S %d", &n);
            pdbtosp = ajPdbtospNew(n);
            ajStrAssignS(&pdbtosp->Pdb, pdb);
        }
        else if (ajStrPrefixC(line, "IN"))
        {
            ajFmtScanS(line, "%*S %S %*S %S", &pdbtosp->Spr[i],
                       &pdbtosp->Acc[i]);
            i++;
        }

        ok = ajReadlineTrim(inf, &line);
    }

    ajStrDel(&line);
    ajStrDel(&tentry);
    ajStrDel(&pdb);

    return pdbtosp;
}




/* @func ajPdbtospReadAllNew **************************************************
**
** Read all the Pdbtosp objects in a file in embl-like format (see
** documentation for DOMAINATRIX "pdbtosp" application) and writes a list of
** these objects. It then sorts the list by PDB id.
**
** @param [u] inf [AjPFile] Input file stream
**
** @return [AjPList] List of Pdbtosp objects.
**
** @release 2.9.0
** @@
******************************************************************************/

AjPList ajPdbtospReadAllNew(AjPFile inf)
{
    AjPList list = NULL;

    AjPPdbtosp pdbtosp = NULL;

    /* Check args and allocate list if necessary */
    if (!inf)
        return NULL;

    list = ajListNew();

    while ((pdbtosp = ajPdbtospReadCNew(inf, "*")))
        ajListPush(list, (void *) pdbtosp);

    ajListSort(list, &pdbSortPdbtospPdb);

    return list;
}




/* @func ajCmapReadINew *******************************************************
**
** Read a Cmap object from a file in CON  format (see
** documentation for DOMAINATRIX "contacts" application). Takes the chain
** identifier as an integer. If the arguments mod and chn are both 0, the
** function will read the next Cmap in the file.
**
** @param [u] inf     [AjPFile]  Input file stream
** @param [r] chn     [ajint]    Chain number
** @param [r] mod     [ajint]    Model number
**
** @return [AjPCmap] Pointer to new Cmap object.
** @category new [AjPCmap] Cmap constructor from reading file in CON
**           format (see documentation for the EMBASSY DOMAINATRIX package).
**
** @release 2.9.0
** @@
******************************************************************************/

AjPCmap ajCmapReadINew(AjPFile inf, ajint chn, ajint mod)
{
    AjPCmap cmap = NULL;

    if (!(cmap = ajCmapReadNew(inf, CMAP_MODE_I, chn, mod)))
        return NULL;

    return cmap;
}




/* @func ajCmapReadCNew *******************************************************
**
** Read a Cmap object from a file in CON format (see
** documentation for DOMAINATRIX "contacts" application). Takes the chain
** identifier as a character.
**
** @param [u] inf     [AjPFile]  Input file stream
** @param [r] chn     [char]     Chain number
** @param [r] mod     [ajint]    Model number
**
** @return [AjPCmap]   Pointer to new Cmap object.
** @category new [AjPCmap] Cmap constructor from reading file in CON
**              format (see documentation for the EMBASSY DOMAINATRIX package).
**
** @release 2.9.0
** @@
******************************************************************************/

AjPCmap ajCmapReadCNew(AjPFile inf, char chn, ajint mod)
{
    AjPCmap cmap = NULL;

    if (!(cmap = ajCmapReadNew(inf, CMAP_MODE_C, (ajint) chn, mod)))
        return NULL;

    return cmap;
}




/* @func ajCmapReadAllNew *****************************************************
**
** Read every Cmap object from a file in CON format (see
** documentation for DOMAINATRIX "contacts" application) and returns a list
** of these objects.
**
** @param [u] inf     [AjPFile]  Input file stream
**
** @return [AjPList]   List of Cmap objects.
*** @@
**
** @release 3.0.0
******************************************************************************/

AjPList ajCmapReadAllNew(AjPFile inf)
{
    AjPList list = NULL;
    AjPCmap cmap = NULL;

    list = ajListNew();

    while ((cmap = ajCmapReadNew(inf, CMAP_MODE_I, 0, 0)))
        ajListPushAppend(list, cmap);

    return list;
}




/* @func ajCmapReadNew ********************************************************
**
** Read a Cmap object from a file in CON format (see
** documentation for DOMAINATRIX "contacts" application). This is not
** usually called by the user, who uses ajCmapReadINew or ajCmapReadCNew
** instead.  If mode==CMAP_MODE_I, chn==0 and  mod==0, the function will
** read the next Cmap in the file.
**
** @param [u] inf     [AjPFile]  Input file stream.
** @param [r] mode    [ajint]    Mode, either CMAP_MODE_I (treat chn arg as
**                               an integer) or CMAP_MODE_C (treat chn arg as
**                               a character).
** @param [r] chn     [ajint]    Chain identifier / number.
** @param [r] mod     [ajint]    Model number.
**
** @return [AjPCmap] True on success (an object read)
** @category new [AjPCmap] Cmap constructor from reading file in CON
**              format (see documentation for the EMBASSY DOMAINATRIX package).
**
** @release 2.9.0
** @@
******************************************************************************/

AjPCmap ajCmapReadNew(AjPFile inf, ajint mode, ajint chn, ajint mod)

{
    AjPCmap cmap = NULL;
    const AjPStr token = NULL;  /* For parsing      */

    ajint smcon = 0;            /* No. of SM contacts       */
    ajint licon = 0;            /* No. of LI contacts       */
    ajint x = 0;                /* No. of first residue making contact */
    ajint y = 0;                /* No. of second residue making contact */
    ajint md = -1;              /* Model number   */
    ajint cn1 = -1;             /* Chain number 1 */
    ajint cn2 = -1;             /* Chain number 2 */
    char id1 = -1;              /* Chain id 1     */
    char id2 = -1;              /* Chain id 2     */
    ajint nres1 = 0;            /* No. of residues in domain / chain 1 */
    ajint nres2 = 0;            /* No. of residues in domain / chain 2 */
    AjPStr seq1 = NULL;         /* Sequence 1 */
    AjPStr seq2 = NULL;         /* Sequence 2 */

    AjBool idok = ajFalse;      /* If the required chain has been found */

    ajint en;                   /* Entry number. */
    ajint ns;                   /* No. of sites, SITES output only */
    ajint sn;                   /* Site number, SITES output only */

    /* Check args */
    if (!inf)
    {
        ajWarn("Invalid args to ajCmapReadNew");
        return NULL;
    }

    /* Convert '_' chain identifiers to '.' if necessary */
    if (mode == CMAP_MODE_C)
        if (chn == '_')
            chn = '.';

    /* Initialise strings */
    if (!pdbGStrline)
    {
        pdbGStrline = ajStrNew();
        pdbGStrtemp_id = ajStrNew();
        pdbGStrtemp_domid = ajStrNew();
        pdbGStrtemp_ligid = ajStrNew();
        pdbGStrdesc = ajStrNew();
        pdbGStrtmpstr = ajStrNew();
    }

    /* Start of main loop */
    while ((ajReadlineTrim(inf, &pdbGStrline)))
    {
        /* // */
        if (ajStrPrefixC(pdbGStrline, "//"))
        {
            /* If the delimiter between entries is found and cmap is
             * non-NULL, i.e. has been allocated, the function should return. */
            ajStrDel(&seq1);
            ajStrDel(&seq2);

            return cmap;
        }
        
        /* SI */
        else if (ajStrPrefixC(pdbGStrline, "SI"))
        {
            token = ajStrParseC(pdbGStrline, ";");
            ajFmtScanS(token, "%*s %*s %d", &sn);

            token = ajStrParseC(NULL, ";");
            ajFmtScanS(token, "%*s %d", &ns);
        }
        
        /* EN */
        else if (ajStrPrefixC(pdbGStrline, "EN"))
        {
            ajFmtScanS(pdbGStrline, "%*s %*c%d", &en);
        }
        
        /* TY */
        else if (ajStrPrefixC(pdbGStrline, "TY"))
        {
            ajFmtScanS(pdbGStrline, "%*s %S", &pdbGStrtype);
            ajStrSetClear(&seq1);
            ajStrSetClear(&seq2);
            id1 = '.';
            id2 = '.';
            cn1 = 0;
            cn2 = 0;
            nres1 = 0;
            nres2 = 0;
        }
        
        /* EX, NE records are not parsed */
        
        /* ID */
        else if (ajStrPrefixC(pdbGStrline, "ID"))
        {
            token = ajStrParseC(pdbGStrline, ";");
            ajFmtScanS(token, "%*s %*s %S", &pdbGStrtemp_id);
            token = ajStrParseC(NULL, ";");
            ajFmtScanS(token, "%*s %S", &pdbGStrtemp_domid);
            token = ajStrParseC(NULL, ";");
            ajFmtScanS(token, "%*s %S", &pdbGStrtemp_ligid);
        }
        
        /* DE records are not parsed (SITES output) */
        else if (ajStrPrefixC(pdbGStrline, "DE"))
        {
            ajStrAssignSubS(&pdbGStrdesc, pdbGStrline, 4, -1);
        }
        
        /* CN */
        else if (ajStrPrefixC(pdbGStrline, "CN"))
        {
            token = ajStrParseC(pdbGStrline, ";");
#if AJFALSE
            ajFmtScanS(token, "%*s %*s %d", &md);
            if (md == '.')
                md = 0;
#endif
            ajFmtScanS(token, "%*s %*s %S", &pdbGStrtmpstr);

            if (ajStrMatchC(pdbGStrtmpstr, "."))
                md = 0;
            else
                ajFmtScanS(pdbGStrtmpstr, "%d", &md);

            token = ajStrParseC(NULL, ";");
            ajFmtScanS(token, "%*s %d", &cn1);

            token = ajStrParseC(NULL, ";");
            ajFmtScanS(token, "%*s %d", &cn2);

            token = ajStrParseC(NULL, ";");
            ajFmtScanS(token, "%*s %c", &id1);

            token = ajStrParseC(NULL, ";");
            ajFmtScanS(token, "%*s %c", &id2);

            token = ajStrParseC(NULL, ";");
#if AJFALSE
            ajFmtScanS(token, "%*s %d", &nres1);
            if (nres1 == '.')
                nres1 = 0;
#endif

            ajFmtScanS(token, "%*s %S", &pdbGStrtmpstr);

            if (ajStrMatchC(pdbGStrtmpstr, "."))
                nres1 = 0;
            else
                ajFmtScanS(pdbGStrtmpstr, "%d", &nres1);


            token = ajStrParseC(NULL, ";");
#if AJFALSE
            ajFmtScanS(token, "%*s %d", &nres2);
            if ((char) nres2 == '.')
                nres2 = 0;
#endif
            ajFmtScanS(token, "%*s %S", &pdbGStrtmpstr);

            if (ajStrMatchC(pdbGStrtmpstr, "."))
                nres2 = 0;
            else
                ajFmtScanS(pdbGStrtmpstr, "%d", &nres2);
        }

        /* S1 */
        else if (ajStrPrefixC(pdbGStrline, "S1"))
        {
            while (ajReadlineTrim(inf, &pdbGStrline) &&
                   !ajStrPrefixC(pdbGStrline, "XX"))
                ajStrAppendC(&seq1, ajStrGetPtr(pdbGStrline));

            ajStrRemoveWhite(&seq1);
        }

        /* S2 */
        else if (ajStrPrefixC(pdbGStrline, "S2"))
        {
            while (ajReadlineTrim(inf, &pdbGStrline) &&
                   !ajStrPrefixC(pdbGStrline, "XX"))
                ajStrAppendC(&seq2, ajStrGetPtr(pdbGStrline));

            ajStrRemoveWhite(&seq2);
        }

        /* NC */
        else if ((ajStrPrefixC(pdbGStrline, "NC")) &&
                 ((md == mod) || ((chn == 0) && (mod == 0) && (mode == CMAP_MODE_I))))
        {
            token = ajStrParseC(pdbGStrline, ";");
            ajFmtScanS(token, "%*s %*s %d", &smcon);

            token = ajStrParseC(NULL, ";");
            ajFmtScanS(token, "%*s %d", &licon);

            /*
            ** The fourth conditional is to capture those few domains which
            ** are made up from more than one chain.  For these, the chain
            ** character passed in might be an A or a B (e.g. the character
            ** extracted from the scop domain code) whereas the chain id given
            ** in the contact map file will be a '.' - because of how
            ** scopparse copes with these cases. (A '.' is also in the contact
            ** maps for where a chain id was not specified in the original
            ** pdb file).
            */

            if (((cn1 == chn) && (mode == CMAP_MODE_I)) ||
                ((chn == 0) && (mod == 0) && (mode == CMAP_MODE_I)) ||
                ((toupper((int) id1) == toupper(chn)) &&
                 (mode == CMAP_MODE_C)) ||
                ((toupper((int) id1) == '.') && (toupper(chn) != '.') &&
                 (mode == CMAP_MODE_C))
                )
            {
                idok = ajTrue;

                /* Allocate contact map and write values */
                if (ajStrMatchC(pdbGStrtype, "INTER"))
                {
                    if (nres1 > nres2)
                        cmap = ajCmapNew(nres1);
                    else
                        cmap = ajCmapNew(nres2);
                }
                else
                    cmap = ajCmapNew(nres1);

                ajStrAssignS(&cmap->Id, pdbGStrtemp_id);
                ajStrAssignS(&cmap->Domid, pdbGStrtemp_domid);
                ajStrAssignS(&cmap->Ligid, pdbGStrtemp_ligid);

                if (ajStrMatchC(pdbGStrtype, "INTRA"))
                {
                    cmap->Type = ajECmapTypeIntra;
                    cmap->Ncon = smcon;
                }
                else if (ajStrMatchC(pdbGStrtype, "INTER"))
                {
                    cmap->Type = ajECmapTypeInter;
                    cmap->Ncon = smcon;
                }
                else if (ajStrMatchC(pdbGStrtype, "LIGAND"))
                {
                    cmap->Type = ajECmapTypeLigand;
                    cmap->Ncon = licon;
                    cmap->ns = ns;
                    cmap->sn = sn;
                    ajStrAssignS(&cmap->Desc, pdbGStrdesc);
                }
                else
                    ajFatal("Unrecognised contact type");

                cmap->Chn1 = cn1;
                cmap->Chn2 = cn2;
                cmap->Chid1 = id1;
                cmap->Chid2 = id2;
                cmap->Nres1 = nres1;
                cmap->Nres2 = nres2;
                cmap->en = en;

                ajStrAssignS(&cmap->Seq1, seq1);
                ajStrAssignS(&cmap->Seq2, seq2);
            }
        }

        /* SM */
        else if ((ajStrPrefixC(pdbGStrline, "SM")) &&
                 ((md == mod) ||
                  ((chn == 0) && (mod == 0) && (mode == CMAP_MODE_I)))
                 && (idok))
        {
            ajFmtScanS(pdbGStrline, "%*s %*s %d %*c %*s %d", &x, &y);

            /* Check residue number is in range */
            if ((x > cmap->Dim) || (y > cmap->Dim))
                ajFatal("Fatal attempt to write bad data in "
                        "ajCmapReadNew\nFile: %S (%S)\nx: %d y:%d\n",
                        ajFileGetPrintnameS(inf), pdbGStrtemp_id, x, y);

            /* Enter '1' in matrix to indicate contact */
            ajUint2dPut(&cmap->Mat, x - 1, y - 1, 1);
            ajUint2dPut(&cmap->Mat, y - 1, x - 1, 1);
        }

        /* LI */
        else if ((ajStrPrefixC(pdbGStrline, "LI")) &&
                 ((md == mod) ||
                  ((chn == 0) && (mod == 0) && (mode == CMAP_MODE_I)))
                 && (idok))
        {
            ajFmtScanS(pdbGStrline, "%*s %*s %d", &x);

            /* Check residue number is in range */
            if ((x > cmap->Dim))
                ajFatal("Fatal attempt to write bad data in "
                        "ajCmapReadNew\nFile: %S (%S)\nx: %d\n",
                        ajFileGetPrintnameS(inf), pdbGStrtemp_id, x);

            /* Enter '1' in matrix to indicate contact. For ligand contacts,
             * the first row / column only is used. */
            ajUint2dPut(&cmap->Mat, x - 1, 0, 1);
            ajUint2dPut(&cmap->Mat, 0, x - 1, 1);
        }
    }

    ajStrDel(&seq1);
    ajStrDel(&seq2);

    return cmap;
}




/* @func ajVdwallReadNew ******************************************************
**
** Read a Vdwall object from a file in embl-like format (see documentation
** for the EMBASSY DOMAINATRIX package).
**
** @param [u] inf     [AjPFile]  Input file stream
**
** @return [AjPVdwall] Pointer to Vdwall object.
** @category new [AjPVdwall] Vdwall constructor from reading file in embl-like
**              format (see documentation for the EMBASSY DOMAINATRIX package).
**
** @release 2.9.0
** @@
******************************************************************************/

AjPVdwall ajVdwallReadNew(AjPFile inf)
{
    AjPVdwall vdwall = NULL;
    AjPStr line = NULL;         /* Line of text */
    ajint nres = 0;             /* No. residues */
    ajint natm = 0;             /* No. atoms */
    ajint rcnt = 0;             /* Residue count */
    ajint acnt = 0;             /* Atom count */
    char id1 = '\0';            /* Residue 1 char id code */
    AjPStr id3 = NULL;          /* Residue 3 char id code */


    /* Allocate strings */
    line = ajStrNew();
    id3 = ajStrNew();

    /* Start of main loop */
    while ((ajReadlineTrim(inf, &line)))
    {
        /* Parse NR line */
        if (ajStrPrefixC(line, "NR"))
        {
            ajFmtScanS(line, "%*s %d", &nres);

            /* Allocate Vdwall object */
            vdwall = ajVdwallNew(nres);

        }
        /* Parse residue id 3 char */
        else if (ajStrPrefixC(line, "AA"))
        {
            rcnt++;
            acnt = 0;
            ajFmtScanS(line, "%*s %S", &id3);
        }
        /* Parse residue id 1 char */
        else if (ajStrPrefixC(line, "ID"))
            ajFmtScanS(line, "%*s %c", &id1);
        /* Parse number of atoms */
        else if (ajStrPrefixC(line, "NN"))
        {
            ajFmtScanS(line, "%*s %d", &natm);

            /* Allocate next Vdwres object */
            vdwall->Res[rcnt - 1] = ajVdwresNew(natm);

            /* Write members of Vdwres object */
            vdwall->Res[rcnt - 1]->Id1 = id1;
            ajStrAssignS(&vdwall->Res[rcnt - 1]->Id3, id3);

        }
        /* Parse atom line */
        else if (ajStrPrefixC(line, "AT"))
        {
            acnt++;
            ajFmtScanS(line, "%*s %S %*c %f",
                       &vdwall->Res[rcnt - 1]->Atm[acnt - 1],
                       &vdwall->Res[rcnt - 1]->Rad[acnt - 1]);
        }
    }

    ajStrDel(&line);
    ajStrDel(&id3);

    return vdwall;
}




/* @func ajHetReadNew *********************************************************
**
** Read heterogen dictionary, the Het object is allocated.
**
** @param [u] inf [AjPFile]    Pointer to Het file
**
** @return [AjPHet] Het object.
** @category new [AjPHet] Het constructor from reading dictionary of
**                         heterogen groups in clean format (see documentation
**                         for the EMBASSY DOMAINATRIX package).
**
** @release 2.9.0
** @@
******************************************************************************/

AjPHet ajHetReadNew(AjPFile inf)
{
    AjPHet het = NULL;
    AjPHetent hetent = NULL;    /* current entry */
    AjPList list = NULL;        /* AJAX List of AJAX Heterogen Entry objects */
    AjPStr line = NULL;         /* current line */
    AjPStr temp = NULL;         /* Temporary string */

    /* Check args */
    if (!inf)
    {
        ajWarn("Bad args passed to ajHetReadNew\n");

        return NULL;
    }

    /* Create Het object if necessary */
    if (!het)
        het = ajHetNew(0);

    /* Create string and list objects */

    line = ajStrNew();
    temp = ajStrNew();
    list = ajListNew();

    /* Read lines from file */
    while (ajReadlineTrim(inf, &line))
    {
        if (ajStrPrefixC(line, "ID   "))
        {
            hetent = ajHetentNew();
            ajFmtScanS(line, "%*s %S", &hetent->abv);
        }
        else if (ajStrPrefixC(line, "DE   "))
        {                       /* NEED TO ACCOUNT FOR MULTIPLE LINES */
            ajStrAssignSubS(&temp, line, 5, -1);
            if (ajStrGetLen(hetent->ful))
                ajStrAppendS(&hetent->ful, temp);
            else
                ajStrAssignS(&hetent->ful, temp);
        }
        else if (ajStrPrefixC(line, "SY   "))
        {
#if AJFALSE
            ajFmtScanS(line, "%*s %S", &hetent->syn);
#endif
            ajStrAssignSubS(&temp, line, 5, -1);
            if (ajStrGetLen(hetent->syn))
                ajStrAppendS(&hetent->syn, temp);
            else
                ajStrAssignS(&hetent->syn, temp);
        }
        else if (ajStrPrefixC(line, "NN   "))
            ajFmtScanS(line, "%*s %S", &hetent->cnt);
        else if (ajStrPrefixC(line, "//"))
            ajListPush(list, (AjPHetent) hetent);
    }

    het->Number = (ajuint) ajListToarray(list, (void ***) &het->Entries);

    ajStrDel(&line);
    ajStrDel(&temp);

    ajListFree(&list);

    return het;
}




/* @func ajHetReadRawNew ******************************************************
**
** Reads a dictionary of heterogen groups available at
** http://pdb.rutgers.edu/het_dictionary.txt and writes a Het object.
**
** @param [u] inf [AjPFile]    Pointer to dictionary
**
** @return [AjPHet] True on success
** @category new [AjPHet] Het constructor from reading dictionary of
**                        heterogen groups in raw format.
**
** @release 2.9.0
** @@
******************************************************************************/

AjPHet ajHetReadRawNew(AjPFile inf)
{
    AjPHet het = NULL;
    AjPStr line = NULL;         /* A line from the file */
    AjPHetent entry = NULL;     /* The current entry */
    AjPHetent tmp = NULL;       /* Temp. pointer */
    AjPList list = NULL;        /* List of entries */
    ajint het_cnt = 0;          /* Count of number of HET records in file */
    ajint formul_cnt = 0;       /* Count of number of FORMUL records in file */

    /* Check arg's */
    if (!inf)
    {
        ajWarn("Bad args passed to ajHetReadRawNew\n");
        return NULL;
    }

    /* Create strings etc */
    line = ajStrNew();
    list = ajListNew();

    /* Read lines from file */
    while (ajReadlineTrim(inf, &line))
    {
        if (ajStrPrefixC(line, "HET "))
        {
            het_cnt++;

            entry = ajHetentNew();
            ajFmtScanS(line, "%*s %S", &entry->abv);
        }
        else if (ajStrPrefixC(line, "HETNAM"))
        {
            ajStrAppendC(&entry->ful, &line->Ptr[15]);
        }
        else if (ajStrPrefixC(line, "HETSYN"))
        {
            ajStrAppendC(&entry->syn, &line->Ptr[15]);
        }
        else if (ajStrPrefixC(line, "FORMUL"))
        {
            formul_cnt++;

            /* In cases where HETSYN or FORMUL were not specified, assign a
             * value of '.' */
            if (MAJSTRGETLEN(entry->ful) == 0)
                ajStrAssignC(&entry->ful, ".");

            if (MAJSTRGETLEN(entry->syn) == 0)
                ajStrAssignC(&entry->syn, ".");


            /* Push entry onto list */
            ajListPush(list, (AjPHetent) entry);
        }
    }

    if (het_cnt != formul_cnt)
    {
        while (ajListPop(list, (void **) &tmp))
            ajHetentDel(&tmp);

        ajListFree(&list);
        ajStrDel(&line);

        ajFatal("Fatal discrepancy in count of HET and FORMUL records\n");
    }

    het = ajHetNew(0);
    het->Number = (ajuint) ajListToarray(list, (void ***) &het->Entries);

    ajStrDel(&line);

    ajListFree(&list);

    return het;
}




/* @func ajPdbReadFirstModelNew ***********************************************
**
** Reads a clean coordinate file file (see documentation for DOMAINATRIX
** "pdbparse" application) and writes a filled Pdb object. Data for the first
** model only is read in.
**
** @param [u] inf  [AjPFile] Pointer to clean coordinate file
**
** @return [AjPPdb] Pointer to Pdb object.
** @category new [AjPPdb] Pdb constructor from reading ccf format file
**                         (retrieve data for 1st model only).
**
** @release 2.9.0
** @@
******************************************************************************/

AjPPdb ajPdbReadFirstModelNew(AjPFile inf)
{
    return (AjPPdb) ajPdbReadNew(inf, 0);
}




/* @func ajPdbReadAllModelsNew ************************************************
**
** Reads a clean coordinate file (see documentation for DOMAINATRIX "pdbparse"
** application) and writes a filled Pdb object.  Data for all models is read.
**
** @param [u] inf  [AjPFile] Pointer to clean coordinate file
**
** @return [AjPPdb] Pointer to Pdb object.
** @category new [AjPPdb] Pdb constructor from reading ccf format file.
**
** @release 3.0.0
** @@
******************************************************************************/

AjPPdb ajPdbReadAllModelsNew(AjPFile inf)
{
    return (AjPPdb) ajPdbReadNew(inf, 1);
}




/* @func ajPdbReadNew *********************************************************
**
** Reads a clean coordinate file file (see documentation for DOMAINATRIX
** "pdbparse" application) and writes a filled Pdb object. Data for the first
** model only is read in.
**
** @param [u] inf  [AjPFile] Pointer to clean coordinate file
** @param [r] mode [ajint] Mode. 0==Read first model only. 1==Read all models.
**
** @return [AjPPdb] Pointer to Pdb object.
** @category new [AjPPdb] Pdb constructor from reading ccf format file
**                         (retrieve data for 1st model only).
**
** @release 2.9.0
** @@
******************************************************************************/

AjPPdb ajPdbReadNew(AjPFile inf, ajint mode)
{
    AjPPdb pdb = NULL;

    ajint nmod = 0;
    ajint ncha = 0;
    ajint ngrp = 0;
    ajint nc = 0;
    ajint mod = 0;
    ajint chn = 0;
    ajint gpn = 0;

    float reso = 0.0F;

    AjPStr line = NULL;
    AjPStr token = NULL;
    AjPStr idstr = NULL;
    AjPStr destr = NULL;
    AjPStr osstr = NULL;
    AjPStr xstr = NULL;
    AjPStrTok handle = NULL;

    AjPAtom atom = NULL;
    AjPResidue residue = NULL;
    ajuint rn_last = UINT_MAX;
    ajuint mn_last = UINT_MAX;

    AjBool fixReadAtoms = ajTrue;

    /* Initialise strings */
    line = ajStrNew();
    token = ajStrNew();
    idstr = ajStrNew();
    destr = ajStrNew();
    osstr = ajStrNew();
    xstr = ajStrNew();

    /* Start of main loop */
    while (ajReadlineTrim(inf, &line))
    {
        if (ajStrPrefixC(line, "XX"))
            continue;

        /* Parse ID */
        else if (ajStrPrefixC(line, "ID"))
        {
            ajStrTokenAssignC(&handle, line, " \n\t\r");
            ajStrTokenNextParse(handle, &token);
            ajStrTokenNextParse(handle, &idstr);
            continue;
        }

        /* Parse number of chains */
        else if (ajStrPrefixC(line, "CN"))
        {
            ajStrTokenAssignC(&handle, line, " []\n\t\r");
            ajStrTokenNextParse(handle, &token);
            ajStrTokenNextParse(handle, &token);
            ajStrToInt(token, &nc);
            continue;
        }

        /* Parse description text */
        else if (ajStrPrefixC(line, "DE"))
        {
            ajStrTokenAssignC(&handle, line, " ");
            ajStrTokenNextParse(handle, &token);
            /* 'DE' */
            ajStrTokenNextParseC(handle, "\n\r", &token);

            /* desc */
            if (ajStrGetLen(destr))
            {
                ajStrAppendC(&destr, " ");
                ajStrAppendS(&destr, token);
            }
            else
                ajStrAssignS(&destr, token);
            continue;
        }

        /* Parse source text */
        else if (ajStrPrefixC(line, "OS"))
        {
            ajStrTokenAssignC(&handle, line, " ");
            ajStrTokenNextParse(handle, &token);
            /* 'OS' */
            ajStrTokenNextParseC(handle, "\n\r", &token);

            /* source */
            if (ajStrGetLen(osstr))
            {
                ajStrAppendC(&osstr, " ");
                ajStrAppendS(&osstr, token);
            }
            else
                ajStrAssignS(&osstr, token);
            continue;
        }

        /* Parse experimental line */
        else if (ajStrPrefixC(line, "EX"))
        {
            ajStrTokenAssignC(&handle, line, " ;\n\t\r");
            ajStrTokenNextParse(handle, &token);
            ajStrTokenNextParse(handle, &token);

            ajStrTokenNextParse(handle, &xstr);        /* method */
            ajStrTokenNextParse(handle, &token);

            ajStrTokenNextParse(handle, &token);       /* reso */
            ajStrToFloat(token, &reso);
            ajStrTokenNextParse(handle, &token);
            ajStrTokenNextParse(handle, &token);       /* nmod */
            ajStrToInt(token, &nmod);
            ajStrTokenNextParse(handle, &token);

            ajStrTokenNextParse(handle, &token);       /* ncha */
            ajStrToInt(token, &ncha);

            ajStrTokenNextParse(handle, &token);       /* nlig */
            ajStrToInt(token, &ngrp);

            pdb = ajPdbNew(ncha);

            ajStrAssignS(&pdb->Pdb, idstr);
            ajStrAssignS(&pdb->Compnd, destr);
            ajStrAssignS(&pdb->Source, osstr);

            if (ajStrMatchC(xstr, "xray"))
                pdb->Method = ajEPdbMethodXray;
            else
                pdb->Method = ajEPdbMethodNmr;

            pdb->Reso = reso;

            /*
            ** 0 == Read first model only. Number of models is hard-coded to 1
            ** as only the data for the first model is read in.
            ** 1 == Read all models.
            */

            if (mode == 0)
                pdb->Nmod = 1;
            else if (mode == 1)
                pdb->Nmod = nmod;
            else
                ajFatal("Unrecognised mode in ajPdbReadNew");

            pdb->Nchn = ncha;
            pdb->Ngp = ngrp;

            continue;
        }

        /* Parse information line */
        else if (ajStrPrefixC(line, "IN"))
        {
            ajStrTokenAssignC(&handle, line, " ;\n\t\r");
            ajStrTokenNextParse(handle, &token);
            ajStrTokenNextParse(handle, &token);

            /* id value */
            ajStrTokenNextParse(handle, &token);
            pdb->Chains[nc - 1]->Id = *ajStrGetPtr(token);
            ajStrTokenNextParse(handle, &token);

            /* residues */
            ajStrTokenNextParse(handle, &token);
            ajStrToUint(token, &pdb->Chains[nc - 1]->Nres);
            ajStrTokenNextParse(handle, &token);

            /* hetatm */
            ajStrTokenNextParse(handle, &token);
            ajStrToUint(token, &pdb->Chains[nc - 1]->Nlig);

            /* helices */
            ajStrTokenNextParse(handle, &token);
            ajStrToInt(token, &pdb->Chains[nc - 1]->numHelices);

            /* strands */
            ajStrTokenNextParse(handle, &token);
            ajStrToInt(token, &pdb->Chains[nc - 1]->numStrands);

            /* sheets */
#if AJFALSE
            ajStrTokenNextParse(handle, &token);
            ajStrToInt(token, &pdb->Chains[nc - 1]->numSheets);
#endif
            /* turns */
#if AJFALSE
            ajStrTokenNextParse(handle, &token);
            ajStrToInt(token, &pdb->Chains[nc - 1]->numTurns);
#endif
            continue;
        }

        /* Parse sequence line */
        else if (ajStrPrefixC(line, "SQ"))
        {
            while (ajReadlineTrim(inf, &line) && !ajStrPrefixC(line, "XX"))
                ajStrAppendC(&pdb->Chains[nc - 1]->Seq, ajStrGetPtr(line));

            ajStrRemoveWhite(&pdb->Chains[nc - 1]->Seq);
            continue;
        }

        /* Parse atom line */
        else if (fixReadAtoms && ajStrPrefixC(line, "AT"))
        {
            mod = chn = gpn = 0;

            /* Skip AT record */
            ajStrTokenAssignC(&handle, line, " \t\n\r");
            ajStrTokenNextParse(handle, &token);

            /* Model number. 0 == Read first model only */
            ajStrTokenNextParse(handle, &token);
            ajStrToInt(token, &mod);

            if ((mode == 0) && (mod != 1))
            {
                /* break; */
                /* Discard remaining AT lines */
                while (ajReadlineTrim(inf, &line) && ajStrPrefixC(line, "AT"));
            }

            /* Chain number */
            ajStrTokenNextParse(handle, &token);
            ajStrToInt(token, &chn);

            /* Group number */
            ajStrTokenNextParse(handle, &token);
            ajStrToInt(token, &gpn);

            /* Allocate object */
            atom = ajAtomNew();

            atom->Mod = mod;
            atom->Chn = chn;
            atom->Gpn = gpn;

            /* Residue number */
            ajStrTokenNextParse(handle, &token);
            ajStrToUint(token, &atom->Idx);

            /* Residue number string */
            ajStrTokenNextParse(handle, &token);
            ajStrAssignS(&atom->Pdb, token);

            /* Residue id, 1 char */
            ajStrTokenNextParse(handle, &token);
            atom->Id1 = *ajStrGetPtr(token);

            /* Residue id, 3 char */
            ajStrTokenNextParse(handle, &token);
            ajStrAssignS(&atom->Id3, token);

            /* Atom type */
            ajStrTokenNextParse(handle, &token);
            atom->Type = *ajStrGetPtr(token);

            /* Atom identifier */
            ajStrTokenNextParse(handle, &token);
            ajStrAssignS(&atom->Atm, token);

            /* X coordinate */
            ajStrTokenNextParse(handle, &token);
            ajStrToFloat(token, &atom->X);

            /* Y coordinate */
            ajStrTokenNextParse(handle, &token);
            ajStrToFloat(token, &atom->Y);

            /* Z coordinate */
            ajStrTokenNextParse(handle, &token);
            ajStrToFloat(token, &atom->Z);

            /* Occupancy */
            ajStrTokenNextParse(handle, &token);
            ajStrToFloat(token, &atom->O);

            /* B value thermal factor.  */
            ajStrTokenNextParse(handle, &token);
            ajStrToFloat(token, &atom->B);

            /*
            ** Push atom onto appropriate list.
            ** Check for coordinates for water or groups that could not
            ** be uniquely assigned to a chain
            */
            if (chn == 0)
            {
                /* Heterogen */
                if (atom->Type == 'H')
                    ajListPushAppend(pdb->Groups, (void *) atom);
                else if (atom->Type == 'W')
                    ajListPushAppend(pdb->Water, (void *) atom);
                else
                    ajFatal("Unexpected parse error in "
                            "ajPdbReadFirstModelNew");
            }
            else
            {
                ajListPushAppend(pdb->Chains[chn - 1]->Atoms, (void *) atom);
            }
            continue;
        }

        /* Parse residue line */
        else if (ajStrPrefixC(line, "RE"))
        {
            mod = chn = 0;

            /* Skip RE record */
            ajStrTokenAssignC(&handle, line, " \t\n\r");
            ajStrTokenNextParse(handle, &token);

            /* Model number. 0 == Read first model only */
            ajStrTokenNextParse(handle, &token);
            ajStrToInt(token, &mod);

            if ((mode == 0) && (mod != 1))
            {
                /* break; */
                /* Discard remaining RE lines */
                while (ajReadlineTrim(inf, &line) && ajStrPrefixC(line, "RE"));
            }

            /* Chain number */
            ajStrTokenNextParse(handle, &token);
            ajStrToInt(token, &chn);

            /* Allocate object */
            residue = ajResidueNew();

            residue->Mod = mod;
            residue->Chn = chn;

            /* Residue number */
            ajStrTokenNextParse(handle, &token);
            ajStrToUint(token, &residue->Idx);

            /* Residue number (original string) */
            ajStrTokenNextParse(handle, &token);
            ajStrAssignS(&residue->Pdb, token);

            /* Residue id, 1 char */
            ajStrTokenNextParse(handle, &token);
            residue->Id1 = *ajStrGetPtr(token);

            /* Residue id, 3 char */
            ajStrTokenNextParse(handle, &token);
            ajStrAssignS(&residue->Id3, token);

            /* Element serial number (PDB elements) */
            ajStrTokenNextParse(handle, &token);
            ajStrToInt(token, &residue->eNum);

            /* Element identifier  (PDB elements) */
            ajStrTokenNextParse(handle, &token);
            ajStrAssignS(&residue->eId, token);

            /* Element type (PDB elements) */
            ajStrTokenNextParse(handle, &token);
            residue->eType = *ajStrGetPtr(token);

            /* Class of helix  (PDB elements) */
            ajStrTokenNextParse(handle, &token);
            ajStrToInt(token, &residue->eClass);

            /* Number of the element (stride) */
            ajStrTokenNextParse(handle, &token);
            ajStrToInt(token, &residue->eStrideNum);

            /* Element type (stride) */
            ajStrTokenNextParse(handle, &token);
            residue->eStrideType = *ajStrGetPtr(token);

            /* Phi angle */
            ajStrTokenNextParse(handle, &token);
            ajStrToFloat(token, &residue->Phi);

            /* Psi angle */
            ajStrTokenNextParse(handle, &token);
            ajStrToFloat(token, &residue->Psi);

            /* Residue solvent accessible area.  */
            ajStrTokenNextParse(handle, &token);
            ajStrToFloat(token, &residue->Area);

            /* Absolute accessibility, all atoms.  */
            ajStrTokenNextParse(handle, &token);
            ajStrToFloat(token, &residue->all_abs);

            /* Relative accessibility, all atoms. */
            ajStrTokenNextParse(handle, &token);
            ajStrToFloat(token, &residue->all_rel);

            /* Absolute accessibility, atoms in side chain.  */
            ajStrTokenNextParse(handle, &token);
            ajStrToFloat(token, &residue->side_abs);

            /* Relative accessibility, atoms in side chain.  */
            ajStrTokenNextParse(handle, &token);
            ajStrToFloat(token, &residue->side_rel);

            /* Absolute accessibility, atoms in main chain. */
            ajStrTokenNextParse(handle, &token);
            ajStrToFloat(token, &residue->main_abs);

            /* Relative accessibility, atoms in main chain. */
            ajStrTokenNextParse(handle, &token);
            ajStrToFloat(token, &residue->main_rel);

            /* Absolute accessibility, non-polar atoms. */
            ajStrTokenNextParse(handle, &token);
            ajStrToFloat(token, &residue->npol_abs);

            /* Relative accessibility, non-polar atoms. */
            ajStrTokenNextParse(handle, &token);
            ajStrToFloat(token, &residue->npol_rel);

            /* Absolute accessibility, polar atoms. */
            ajStrTokenNextParse(handle, &token);
            ajStrToFloat(token, &residue->pol_abs);

            /* Relative accessibility, polar atoms. */
            ajStrTokenNextParse(handle, &token);
            ajStrToFloat(token, &residue->pol_rel);

            ajListPushAppend(pdb->Chains[chn - 1]->Residues, (void *) residue);

            continue;
        }

        /* Parse coordinate line */
        else if (ajStrPrefixC(line, "CO"))
        {
            mod = chn = gpn = 0;

            ajStrTokenAssignC(&handle, line, " \t\n\r");
            ajStrTokenNextParse(handle, &token);

            ajStrTokenNextParse(handle, &token);
            ajStrToInt(token, &mod);

            ajStrTokenNextParse(handle, &token);
            ajStrToInt(token, &chn);

            ajStrTokenNextParse(handle, &token);
            ajStrToInt(token, &gpn);

            atom = ajAtomNew();

            atom->Mod = mod;
            atom->Chn = chn;
            atom->Gpn = gpn;

            ajStrTokenNextParse(handle, &token);
            atom->Type = ajStrGetCharFirst(token);

            ajStrTokenNextParse(handle, &token);
            ajStrToUint(token, &atom->Idx);

            ajStrTokenNextParse(handle, &token);
            ajStrAssignS(&atom->Pdb, token);

            /* Residue object */
            if (atom->Type == 'P')
            {
                /* New model */
                if (atom->Mod != mn_last)
                {
                    rn_last = UINT_MAX;
                    mn_last = atom->Mod;
                }
                /* New residue */
                if (atom->Idx != rn_last)
                {
                    residue = ajResidueNew();

                    residue->Mod = atom->Mod;
                    residue->Chn = atom->Chn;
                    residue->Idx = atom->Idx;
                    ajStrAssignS(&residue->Pdb, atom->Pdb);
                }
            }

            ajStrTokenNextParse(handle, &token);
            residue->eType = *ajStrGetPtr(token);

            ajStrTokenNextParse(handle, &token);
            ajStrToInt(token, &residue->eNum);

            ajStrTokenNextParse(handle, &token);
            ajStrAssignS(&residue->eId, token);

            ajStrTokenNextParse(handle, &token);
            ajStrToInt(token, &residue->eClass);

            ajStrTokenNextParse(handle, &token);
            residue->eStrideType = *ajStrGetPtr(token);

            ajStrTokenNextParse(handle, &token);
            ajStrToInt(token, &residue->eStrideNum);

            ajStrTokenNextParse(handle, &token);
            atom->Id1 = *ajStrGetPtr(token);
            residue->Id1 = atom->Id1;

            ajStrTokenNextParse(handle, &token);
            ajStrAssignS(&atom->Id3, token);
            ajStrAssignS(&residue->Id3, atom->Id3);

            ajStrTokenNextParse(handle, &token);
            ajStrAssignS(&atom->Atm, token);

            ajStrTokenNextParse(handle, &token);
            ajStrToFloat(token, &atom->X);

            ajStrTokenNextParse(handle, &token);
            ajStrToFloat(token, &atom->Y);

            ajStrTokenNextParse(handle, &token);
            ajStrToFloat(token, &atom->Z);

            ajStrTokenNextParse(handle, &token);
            ajStrToFloat(token, &atom->O);

            ajStrTokenNextParse(handle, &token);
            ajStrToFloat(token, &atom->B);

            ajStrTokenNextParse(handle, &token);
            ajStrToFloat(token, &residue->Phi);

            ajStrTokenNextParse(handle, &token);
            ajStrToFloat(token, &residue->Psi);

            ajStrTokenNextParse(handle, &token);
            ajStrToFloat(token, &residue->Area);

            ajStrTokenNextParse(handle, &token);
            ajStrToFloat(token, &residue->all_abs);

            ajStrTokenNextParse(handle, &token);
            ajStrToFloat(token, &residue->all_rel);

            ajStrTokenNextParse(handle, &token);
            ajStrToFloat(token, &residue->side_abs);

            ajStrTokenNextParse(handle, &token);
            ajStrToFloat(token, &residue->side_rel);

            ajStrTokenNextParse(handle, &token);
            ajStrToFloat(token, &residue->main_abs);

            ajStrTokenNextParse(handle, &token);
            ajStrToFloat(token, &residue->main_rel);

            ajStrTokenNextParse(handle, &token);
            ajStrToFloat(token, &residue->npol_abs);

            ajStrTokenNextParse(handle, &token);
            ajStrToFloat(token, &residue->npol_rel);

            ajStrTokenNextParse(handle, &token);
            ajStrToFloat(token, &residue->pol_abs);

            ajStrTokenNextParse(handle, &token);
            ajStrToFloat(token, &residue->pol_rel);


            /* Check for coordinates for water or groups that could not be
             * uniquely assigned to a chain */
            if (chn == 0)
            {
                /* Heterogen */
                if (atom->Type == 'H')
                    ajListPushAppend(pdb->Groups, (void *) atom);
                else if (atom->Type == 'W')
                    ajListPushAppend(pdb->Water, (void *) atom);
                else
                    ajFatal("Unexpected parse error in ajPdbRead");
            }
            else
                ajListPushAppend(pdb->Chains[chn - 1]->Atoms, (void *) atom);

            ajListPushAppend(pdb->Chains[chn - 1]->Residues, (void *) residue);
        }
    }
    /* End of main application loop */

    ajStrTokenDel(&handle);
    ajStrDel(&line);
    ajStrDel(&token);
    ajStrDel(&idstr);
    ajStrDel(&destr);
    ajStrDel(&osstr);
    ajStrDel(&xstr);

    return pdb;
}




/* @func ajPdbReadoldNew ******************************************************
**
** Reads a clean coordinate file (see documentation for DOMAINATRIX "pdbparse"
** application) lacking residue-level description in RE records and writes a
** filled Pdb object.
**
** @param [u] inf  [AjPFile] Pointer to clean coordinate file
**
** @return [AjPPdb] Pointer to Pdb object.
** @category new [AjPPdb] Pdb constructor from reading ccf format file.
**
** @release 3.0.0
** @@
******************************************************************************/

AjPPdb ajPdbReadoldNew(AjPFile inf)
{
    AjPPdb pdb = NULL;

    ajint nmod = 0;
    ajint ncha = 0;
    ajint ngrp = 0;
    ajint nc = 0;
    ajint mod = 0;
    ajint chn = 0;
    ajint gpn = 0;

    float reso = 0.0F;

    AjPStr line = NULL;
    AjPStr token = NULL;
    AjPStr idstr = NULL;
    AjPStr destr = NULL;
    AjPStr osstr = NULL;
    AjPStr xstr = NULL;
    AjPStrTok handle = NULL;

    AjPAtom atom = NULL;
    AjPResidue residue = NULL;
    ajuint rn_last = UINT_MAX;
    ajuint mn_last = UINT_MAX;


    /* Initialise strings */
    line = ajStrNew();
    token = ajStrNew();
    idstr = ajStrNew();
    destr = ajStrNew();
    osstr = ajStrNew();
    xstr = ajStrNew();

    /* Start of main application loop */
    while (ajReadlineTrim(inf, &line))
    {
        if (ajStrPrefixC(line, "XX"))
            continue;

        /* Parse ID */
        if (ajStrPrefixC(line, "ID"))
        {
            ajStrTokenAssignC(&handle, line, " \n\t\r");
            ajStrTokenNextParse(handle, &token);
            ajStrTokenNextParse(handle, &idstr);
            continue;
        }


        /* Parse number of chains */
        if (ajStrPrefixC(line, "CN"))
        {
            ajStrTokenAssignC(&handle, line, " []\n\t\r");
            ajStrTokenNextParse(handle, &token);
            ajStrTokenNextParse(handle, &token);
            ajStrToInt(token, &nc);
            continue;
        }


        /* Parse description text */
        if (ajStrPrefixC(line, "DE"))
        {
            ajStrTokenAssignC(&handle, line, " ");
            ajStrTokenNextParse(handle, &token);
            /* 'DE' */
            ajStrTokenNextParseC(handle, "\n\r", &token);

            /* desc */
            if (ajStrGetLen(destr))
            {
                ajStrAppendC(&destr, " ");
                ajStrAppendS(&destr, token);
            }
            else
                ajStrAssignS(&destr, token);
            continue;
        }


        /* Parse source text */
        if (ajStrPrefixC(line, "OS"))
        {
            ajStrTokenAssignC(&handle, line, " ");
            ajStrTokenNextParse(handle, &token);
            /* 'OS' */
            ajStrTokenNextParseC(handle, "\n\r", &token);

            /* source */
            if (ajStrGetLen(osstr))
            {
                ajStrAppendC(&osstr, " ");
                ajStrAppendS(&osstr, token);
            }
            else
                ajStrAssignS(&osstr, token);

            continue;
        }


        /* Parse experimental line */
        if (ajStrPrefixC(line, "EX"))
        {
            ajStrTokenAssignC(&handle, line, " ;\n\t\r");
            ajStrTokenNextParse(handle, &token);
            ajStrTokenNextParse(handle, &token);

            ajStrTokenNextParse(handle, &xstr);        /* method */
            ajStrTokenNextParse(handle, &token);

            ajStrTokenNextParse(handle, &token);       /* reso */
            ajStrToFloat(token, &reso);
            ajStrTokenNextParse(handle, &token);
            ajStrTokenNextParse(handle, &token);       /* nmod */
            ajStrToInt(token, &nmod);
            ajStrTokenNextParse(handle, &token);

            ajStrTokenNextParse(handle, &token);       /* nchn */
            ajStrToInt(token, &ncha);

            ajStrTokenNextParse(handle, &token);       /* nlig */
            ajStrToInt(token, &ngrp);

            pdb = ajPdbNew(ncha);

            ajStrAssignS(&pdb->Pdb, idstr);
            ajStrAssignS(&pdb->Compnd, destr);
            ajStrAssignS(&pdb->Source, osstr);

            if (ajStrMatchC(xstr, "xray"))
                pdb->Method = ajEPdbMethodXray;
            else
                pdb->Method = ajEPdbMethodNmr;

            pdb->Reso = reso;
            pdb->Nmod = nmod;
            pdb->Nchn = ncha;
            pdb->Ngp = ngrp;
        }


        /* Parse information line */
        if (ajStrPrefixC(line, "IN"))
        {
            ajStrTokenAssignC(&handle, line, " ;\n\t\r");
            ajStrTokenNextParse(handle, &token);
            ajStrTokenNextParse(handle, &token);
            ajStrTokenNextParse(handle, &token);       /* id value */
            pdb->Chains[nc - 1]->Id = *ajStrGetPtr(token);
            ajStrTokenNextParse(handle, &token);
            ajStrTokenNextParse(handle, &token);       /* residues */
            ajStrToUint(token, &pdb->Chains[nc - 1]->Nres);
            ajStrTokenNextParse(handle, &token);
            /* hetatm */
            ajStrTokenNextParse(handle, &token);
            ajStrToUint(token, &pdb->Chains[nc - 1]->Nlig);
            /* helices */
            ajStrTokenNextParse(handle, &token);
            ajStrToInt(token, &pdb->Chains[nc - 1]->numHelices);
            /* strands */
            ajStrTokenNextParse(handle, &token);
            ajStrToInt(token, &pdb->Chains[nc - 1]->numStrands);
            /* sheets */
#if AJFALSE
            ajStrTokenNextParse(handle, &token);
            ajStrToInt(token, &pdb->Chains[nc - 1]->numSheets);
#endif
            /* turns */
#if AJFALSE
            ajStrTokenNextParse(handle, &token);
            ajStrToInt(token, &pdb->Chains[nc - 1]->numTurns);
#endif
            continue;
        }


        /* Parse sequence line */
        if (ajStrPrefixC(line, "SQ"))
        {
            while (ajReadlineTrim(inf, &line) && !ajStrPrefixC(line, "XX"))
                ajStrAppendC(&pdb->Chains[nc - 1]->Seq, ajStrGetPtr(line));
            ajStrRemoveWhite(&pdb->Chains[nc - 1]->Seq);

            continue;
        }


        /* Parse coordinate line */
        if (ajStrPrefixC(line, "CO"))
        {
            mod = chn = gpn = 0;

            ajStrTokenAssignC(&handle, line, " \t\n\r");
            ajStrTokenNextParse(handle, &token);

            ajStrTokenNextParse(handle, &token);
            ajStrToInt(token, &mod);

            ajStrTokenNextParse(handle, &token);
            ajStrToInt(token, &chn);

            ajStrTokenNextParse(handle, &token);
            ajStrToInt(token, &gpn);

            atom = ajAtomNew();

            atom->Mod = mod;
            atom->Chn = chn;
            atom->Gpn = gpn;

            ajStrTokenNextParse(handle, &token);
            atom->Type = *ajStrGetPtr(token);

            ajStrTokenNextParse(handle, &token);
            ajStrToUint(token, &atom->Idx);

            ajStrTokenNextParse(handle, &token);
            ajStrAssignS(&atom->Pdb, token);


            /* Residue object */
            if (atom->Type == 'P')
            {
                /* New model */
                if (atom->Mod != mn_last)
                {
                    rn_last = UINT_MAX;
                    mn_last = atom->Mod;
                }

                /* New residue */
                if (atom->Idx != rn_last)
                {
                    residue = ajResidueNew();

                    residue->Mod = atom->Mod;
                    residue->Chn = atom->Chn;
                    residue->Idx = atom->Idx;
                    ajStrAssignS(&residue->Pdb, atom->Pdb);
                }
            }

            ajStrTokenNextParse(handle, &token);
            residue->eType = *ajStrGetPtr(token);

            ajStrTokenNextParse(handle, &token);
            ajStrToInt(token, &residue->eNum);

            ajStrTokenNextParse(handle, &token);
            ajStrAssignS(&residue->eId, token);

            ajStrTokenNextParse(handle, &token);
            ajStrToInt(token, &residue->eClass);

            ajStrTokenNextParse(handle, &token);
            residue->eStrideType = *ajStrGetPtr(token);

            ajStrTokenNextParse(handle, &token);
            ajStrToInt(token, &residue->eStrideNum);

            ajStrTokenNextParse(handle, &token);
            atom->Id1 = *ajStrGetPtr(token);
            residue->Id1 = atom->Id1;

            ajStrTokenNextParse(handle, &token);
            ajStrAssignS(&atom->Id3, token);
            ajStrAssignS(&residue->Id3, atom->Id3);

            ajStrTokenNextParse(handle, &token);
            ajStrAssignS(&atom->Atm, token);

            ajStrTokenNextParse(handle, &token);
            ajStrToFloat(token, &atom->X);

            ajStrTokenNextParse(handle, &token);
            ajStrToFloat(token, &atom->Y);

            ajStrTokenNextParse(handle, &token);
            ajStrToFloat(token, &atom->Z);

            ajStrTokenNextParse(handle, &token);
            ajStrToFloat(token, &atom->O);

            ajStrTokenNextParse(handle, &token);
            ajStrToFloat(token, &atom->B);

            ajStrTokenNextParse(handle, &token);
            ajStrToFloat(token, &residue->Phi);

            ajStrTokenNextParse(handle, &token);
            ajStrToFloat(token, &residue->Psi);

            ajStrTokenNextParse(handle, &token);
            ajStrToFloat(token, &residue->Area);

            ajStrTokenNextParse(handle, &token);
            ajStrToFloat(token, &residue->all_abs);

            ajStrTokenNextParse(handle, &token);
            ajStrToFloat(token, &residue->all_rel);

            ajStrTokenNextParse(handle, &token);
            ajStrToFloat(token, &residue->side_abs);

            ajStrTokenNextParse(handle, &token);
            ajStrToFloat(token, &residue->side_rel);

            ajStrTokenNextParse(handle, &token);
            ajStrToFloat(token, &residue->main_abs);

            ajStrTokenNextParse(handle, &token);
            ajStrToFloat(token, &residue->main_rel);

            ajStrTokenNextParse(handle, &token);
            ajStrToFloat(token, &residue->npol_abs);

            ajStrTokenNextParse(handle, &token);
            ajStrToFloat(token, &residue->npol_rel);

            ajStrTokenNextParse(handle, &token);
            ajStrToFloat(token, &residue->pol_abs);

            ajStrTokenNextParse(handle, &token);
            ajStrToFloat(token, &residue->pol_rel);


            /* Check for coordinates for water or groups that could not be
             * uniquely assigned to a chain */
            if (chn == 0)
            {
                /* Heterogen */
                if (atom->Type == 'H')
                    ajListPushAppend(pdb->Groups, (void *) atom);
                else if (atom->Type == 'W')
                    ajListPushAppend(pdb->Water, (void *) atom);
                else
                    ajFatal("Unexpected parse error in ajPdbRead");
            }
            else
                ajListPushAppend(pdb->Chains[chn - 1]->Atoms, (void *) atom);


            ajListPushAppend(pdb->Chains[chn - 1]->Residues, (void *) residue);
        }
    }
    /* End of main application loop */



    ajStrTokenDel(&handle);
    ajStrDel(&line);
    ajStrDel(&token);
    ajStrDel(&idstr);
    ajStrDel(&destr);
    ajStrDel(&osstr);
    ajStrDel(&xstr);

    return pdb;
}




/* @func ajPdbReadoldFirstModelNew ********************************************
**
** Reads a clean coordinate file file (see documentation for DOMAINATRIX
** "pdbparse" application) lacking residue-level description in RE records and
** writes a filled Pdb object. Data for the first model only is read in.
**
** @param [u] inf  [AjPFile] Pointer to clean coordinate file
**
** @return [AjPPdb] Pointer to Pdb object.
** @category new [AjPPdb] Pdb constructor from reading ccf format file
**                         (retrieve data for 1st model only).
**
** @release 3.0.0
** @@
******************************************************************************/

AjPPdb ajPdbReadoldFirstModelNew(AjPFile inf)
{
    AjPPdb ret = NULL;

    ajint nmod = 0;
    ajint ncha = 0;
    ajint ngrp = 0;
    ajint nc = 0;
    ajint mod = 0;
    ajint chn = 0;
    ajint gpn = 0;

    float reso = 0.0F;

    AjPStr line = NULL;
    AjPStr token = NULL;
    AjPStr idstr = NULL;
    AjPStr destr = NULL;
    AjPStr osstr = NULL;
    AjPStr xstr = NULL;
    AjPStrTok handle = NULL;

    AjPAtom atom = NULL;
    AjPResidue res = NULL;
    ajuint rn_last = UINT_MAX;
    ajuint mn_last = UINT_MAX;

    /* Initialise strings */
    line = ajStrNew();
    token = ajStrNew();
    idstr = ajStrNew();
    destr = ajStrNew();
    osstr = ajStrNew();
    xstr = ajStrNew();

    /* Start of main application loop */
    while (ajReadlineTrim(inf, &line))
    {
        if (ajStrPrefixC(line, "XX"))
            continue;

        /* Parse ID */
        if (ajStrPrefixC(line, "ID"))
        {
            ajStrTokenAssignC(&handle, line, " \n\t\r");
            ajStrTokenNextParse(handle, &token);
            ajStrTokenNextParse(handle, &idstr);
            continue;
        }


        /* Parse number of chains */
        if (ajStrPrefixC(line, "CN"))
        {
            ajStrTokenAssignC(&handle, line, " []\n\t\r");
            ajStrTokenNextParse(handle, &token);
            ajStrTokenNextParse(handle, &token);
            ajStrToInt(token, &nc);
            continue;
        }


        /* Parse description text */
        if (ajStrPrefixC(line, "DE"))
        {
            ajStrTokenAssignC(&handle, line, " ");
            ajStrTokenNextParse(handle, &token);
            /* 'DE' */
            ajStrTokenNextParseC(handle, "\n\r", &token);

            /* desc */
            if (ajStrGetLen(destr))
            {
                ajStrAppendC(&destr, " ");
                ajStrAppendS(&destr, token);
            }
            else
                ajStrAssignS(&destr, token);

            continue;
        }


        /* Parse source text */
        if (ajStrPrefixC(line, "OS"))
        {
            ajStrTokenAssignC(&handle, line, " ");
            ajStrTokenNextParse(handle, &token);
            /* 'OS' */
            ajStrTokenNextParseC(handle, "\n\r", &token);

            /* source */
            if (ajStrGetLen(osstr))
            {
                ajStrAppendC(&osstr, " ");
                ajStrAppendS(&osstr, token);
            }
            else
                ajStrAssignS(&osstr, token);
            continue;
        }


        /* Parse experimental line */
        if (ajStrPrefixC(line, "EX"))
        {
            ajStrTokenAssignC(&handle, line, " ;\n\t\r");
            ajStrTokenNextParse(handle, &token);
            ajStrTokenNextParse(handle, &token);

            ajStrTokenNextParse(handle, &xstr);        /* method */
            ajStrTokenNextParse(handle, &token);

            ajStrTokenNextParse(handle, &token);       /* reso */
            ajStrToFloat(token, &reso);
            ajStrTokenNextParse(handle, &token);
            ajStrTokenNextParse(handle, &token);       /* nmod */
            ajStrToInt(token, &nmod);
            ajStrTokenNextParse(handle, &token);

            ajStrTokenNextParse(handle, &token);       /* ncha */
            ajStrToInt(token, &ncha);

            ajStrTokenNextParse(handle, &token);       /* nlig */
            ajStrToInt(token, &ngrp);

            ret = ajPdbNew(ncha);

            ajStrAssignS(&(ret)->Pdb, idstr);
            ajStrAssignS(&(ret)->Compnd, destr);
            ajStrAssignS(&(ret)->Source, osstr);

            if (ajStrMatchC(xstr, "xray"))
                (ret)->Method = ajEPdbMethodXray;
            else
                (ret)->Method = ajEPdbMethodNmr;

            (ret)->Reso = reso;
            /* (ret)->Nmod = nmod; */

            /*
            ** Number of models is hard-coded to 1 as only the
            **  data for the first model is read in
            */
            (ret)->Nmod = 1;
            (ret)->Nchn = ncha;
            (ret)->Ngp = ngrp;
        }


        /* Parse information line */
        if (ajStrPrefixC(line, "IN"))
        {
            ajStrTokenAssignC(&handle, line, " ;\n\t\r");
            ajStrTokenNextParse(handle, &token);
            ajStrTokenNextParse(handle, &token);
            ajStrTokenNextParse(handle, &token);       /* id value */
            (ret)->Chains[nc - 1]->Id = *ajStrGetPtr(token);
            ajStrTokenNextParse(handle, &token);
            ajStrTokenNextParse(handle, &token);       /* residues */
            ajStrToUint(token, &(ret)->Chains[nc - 1]->Nres);
            ajStrTokenNextParse(handle, &token);
            /* hetatm */
            ajStrTokenNextParse(handle, &token);
            ajStrToUint(token, &(ret)->Chains[nc - 1]->Nlig);
            /* helices */
            ajStrTokenNextParse(handle, &token);
            ajStrToInt(token, &(ret)->Chains[nc - 1]->numHelices);
            /* strands */
            ajStrTokenNextParse(handle, &token);
            ajStrToInt(token, &(ret)->Chains[nc - 1]->numStrands);
            /* sheets */
#if AJFALSE
            ajStrTokenNextParse(handle, &token);
            ajStrToInt(token, &(ret)->Chains[nc - 1]->numSheets);
#endif
            /* turns */
#if AJFALSE
            ajStrTokenNextParse(handle, &token);
            ajStrToInt(token, &(ret)->Chains[nc - 1]->numTurns);
#endif

            continue;
        }


        /* Parse sequence line */
        if (ajStrPrefixC(line, "SQ"))
        {
            while (ajReadlineTrim(inf, &line) && !ajStrPrefixC(line, "XX"))
                ajStrAppendC(&(ret)->Chains[nc - 1]->Seq, ajStrGetPtr(line));

            ajStrRemoveWhite(&(ret)->Chains[nc - 1]->Seq);
            continue;
        }


        /* Parse coordinate line */
        if (ajStrPrefixC(line, "CO"))
        {
            mod = chn = gpn = 0;

            ajStrTokenAssignC(&handle, line, " \t\n\r");
            ajStrTokenNextParse(handle, &token);

            ajStrTokenNextParse(handle, &token);
            ajStrToInt(token, &mod);

            if (mod != 1)
                break;

            ajStrTokenNextParse(handle, &token);
            ajStrToInt(token, &chn);

            ajStrTokenNextParse(handle, &token);
            ajStrToInt(token, &gpn);

            atom = ajAtomNew();

            atom->Mod = mod;
            atom->Chn = chn;
            atom->Gpn = gpn;


            ajStrTokenNextParse(handle, &token);
            atom->Type = *ajStrGetPtr(token);

            ajStrTokenNextParse(handle, &token);
            ajStrToUint(token, &atom->Idx);

            ajStrTokenNextParse(handle, &token);
            ajStrAssignS(&atom->Pdb, token);

            /* Residue object */
            if (atom->Type == 'P')
            {
                /* New model */
                if (atom->Mod != mn_last)
                {
                    rn_last = UINT_MAX;
                    mn_last = atom->Mod;
                }

                /* New residue */
                if (atom->Idx != rn_last)
                {
                    res = ajResidueNew();

                    res->Mod = atom->Mod;
                    res->Chn = atom->Chn;
                    res->Idx = atom->Idx;
                    ajStrAssignS(&res->Pdb, atom->Pdb);
                }
            }


            ajStrTokenNextParse(handle, &token);
            res->eType = *ajStrGetPtr(token);

            ajStrTokenNextParse(handle, &token);
            ajStrToInt(token, &res->eNum);

            ajStrTokenNextParse(handle, &token);
            ajStrAssignS(&res->eId, token);

            ajStrTokenNextParse(handle, &token);
            ajStrToInt(token, &res->eClass);

            ajStrTokenNextParse(handle, &token);
            res->eStrideType = *ajStrGetPtr(token);

            ajStrTokenNextParse(handle, &token);
            ajStrToInt(token, &res->eStrideNum);

            ajStrTokenNextParse(handle, &token);
            atom->Id1 = *ajStrGetPtr(token);

            ajStrTokenNextParse(handle, &token);
            ajStrAssignS(&atom->Id3, token);
            ajStrAssignS(&res->Id3, atom->Id3);

            ajStrTokenNextParse(handle, &token);
            ajStrAssignS(&atom->Atm, token);

            ajStrTokenNextParse(handle, &token);
            ajStrToFloat(token, &atom->X);

            ajStrTokenNextParse(handle, &token);
            ajStrToFloat(token, &atom->Y);

            ajStrTokenNextParse(handle, &token);
            ajStrToFloat(token, &atom->Z);

            ajStrTokenNextParse(handle, &token);
            ajStrToFloat(token, &atom->O);

            ajStrTokenNextParse(handle, &token);
            ajStrToFloat(token, &atom->B);

            ajStrTokenNextParse(handle, &token);
            ajStrToFloat(token, &res->Phi);

            ajStrTokenNextParse(handle, &token);
            ajStrToFloat(token, &res->Psi);

            ajStrTokenNextParse(handle, &token);
            ajStrToFloat(token, &res->Area);

            ajStrTokenNextParse(handle, &token);
            ajStrToFloat(token, &res->all_abs);

            ajStrTokenNextParse(handle, &token);
            ajStrToFloat(token, &res->all_rel);

            ajStrTokenNextParse(handle, &token);
            ajStrToFloat(token, &res->side_abs);

            ajStrTokenNextParse(handle, &token);
            ajStrToFloat(token, &res->side_rel);

            ajStrTokenNextParse(handle, &token);
            ajStrToFloat(token, &res->main_abs);

            ajStrTokenNextParse(handle, &token);
            ajStrToFloat(token, &res->main_rel);

            ajStrTokenNextParse(handle, &token);
            ajStrToFloat(token, &res->npol_abs);

            ajStrTokenNextParse(handle, &token);
            ajStrToFloat(token, &res->npol_rel);

            ajStrTokenNextParse(handle, &token);
            ajStrToFloat(token, &res->pol_abs);

            ajStrTokenNextParse(handle, &token);
            ajStrToFloat(token, &res->pol_rel);

            /* Check for coordinates for water or groups that could not * be
             * uniquely assigned to a chain */
            if (chn == 0)
            {
                /* Heterogen */
                if (atom->Type == 'H')
                    ajListPushAppend((ret)->Groups, (void *) atom);
                else if (atom->Type == 'W')
                    ajListPushAppend((ret)->Water, (void *) atom);
                else
                    ajFatal("Unexpected parse error in "
                            "ajPdbReadFirstModelNew");
            }
            else
                ajListPushAppend((ret)->Chains[chn - 1]->Atoms, (void *) atom);

            ajListPushAppend((ret)->Chains[chn - 1]->Residues, (void *) res);
        }
    }
    /* End of main application loop */


    ajStrTokenDel(&handle);
    ajStrDel(&line);
    ajStrDel(&token);
    ajStrDel(&idstr);
    ajStrDel(&destr);
    ajStrDel(&osstr);
    ajStrDel(&xstr);

    return ret;
}




/* @func ajAtomNew ************************************************************
**
** Atom object constructor.
** This is normally called by the ajChainNew function.
**
** @return [AjPAtom] Pointer to an atom object
** @category new [AjPAtom] Default Atom constructor.
**
** @release 1.8.0
** @@
******************************************************************************/

AjPAtom ajAtomNew(void)
{
    AjPAtom atom = NULL;

    AJNEW0(atom);

    atom->Id3 = ajStrNew();
    atom->Atm = ajStrNew();
    atom->Pdb = ajStrNew();
#if AJFALSE
    atom->eId = ajStrNew();
#endif
    atom->Id1 = '.';
#if AJFALSE
    atom->eType = '.';
    ajStrAssignC(&atom->eId, ".");
    atom->eStrideType = '.';
#endif

    return atom;
}




/* @func ajResidueNew *********************************************************
**
** Residue object constructor.
** This is normally called by the ajChainNew function.
**
** @return [AjPResidue] Pointer to a Residue object
** @category new [AjPResidue] Default Residue constructor.
**
** @release 3.0.0
** @@
******************************************************************************/

AjPResidue ajResidueNew(void)
{
    AjPResidue residue = NULL;

    AJNEW0(residue);

    residue->Pdb = ajStrNew();
    residue->Id3 = ajStrNew();
    residue->eId = ajStrNew();

    residue->Id1 = '.';
    residue->eType = '.';
    ajStrAssignC(&residue->eId, ".");
    residue->eStrideType = '.';

    return residue;
}




/* @func ajChainNew ***********************************************************
**
** Chain object constructor.
** This is normally called by the ajPdbNew function
**
** @return [AjPChain] Pointer to a chain object
**
** @release 1.8.0
** @@
** @category new [AjPChain] Default Chain constructor.
******************************************************************************/

AjPChain ajChainNew(void)
{
    AjPChain chain = NULL;

    AJNEW0(chain);

    chain->Seq = ajStrNewC("");
    chain->Atoms = ajListNew();
    chain->Residues = ajListNew();

    return chain;
}




/* @func ajPdbNew *************************************************************
**
** Pdb object constructor. Fore-knowledge of the number of chains
** is required. This is normally called by the functions that read PDB
** files or clean coordinate files (see embpdb.c & embpdbraw.c).
**
** @param [r] n [ajuint] Number of chains in this pdb file
**
** @return [AjPPdb] Pointer to a pdb object
** @category new [AjPPdb] Default Pdb constructor.
**
** @release 1.8.0
** @@
******************************************************************************/

AjPPdb ajPdbNew(ajuint n)
{
    ajuint i = 0U;

    AjPPdb pdb = NULL;

    AJNEW0(pdb);

    pdb->Pdb = ajStrNew();
    pdb->Compnd = ajStrNew();
    pdb->Source = ajStrNew();
    pdb->Groups = ajListNew();
    pdb->Water = ajListNew();
    pdb->gpid = ajChararrNew();

    if (n)
    {
        AJCNEW0(pdb->Chains, n);

        for (i = 0U; i < n; ++i)
            pdb->Chains[i] = ajChainNew();
    }

    return pdb;
}




/* @func ajHetentNew **********************************************************
**
** Hetent object constructor.
**
** @return [AjPHetent] Pointer to a Hetent object
** @category new [AjPHetent] Default Hetent constructor.
**
** @release 2.9.0
** @@
******************************************************************************/

AjPHetent ajHetentNew(void)
{
    AjPHetent hetent = NULL;

    AJNEW0(hetent);

    hetent->abv = ajStrNew();
    hetent->syn = ajStrNew();
    hetent->ful = ajStrNew();

    return hetent;
}




/* @func ajHetNew *************************************************************
**
** Het object constructor.
**
** @param [r] n [ajuint] Number of entries in dictionary.
**
** @return [AjPHet] Pointer to a Het object
** @category new [AjPHet] Default Het constructor.
**
** @release 2.9.0
** @@
******************************************************************************/

AjPHet ajHetNew(ajuint n)
{
    ajuint i = 0U;

    AjPHet het = NULL;

    AJNEW0(het);

    if (n)
    {
        AJCNEW0(het->Entries, n);

        het->Number = n;

        for (i = 0U; i < n; i++)
            het->Entries[i] = ajHetentNew();
    }
    else
    {
        het->Number = 0U;
        het->Entries = NULL;
    }

    return het;
}




/* @func ajVdwallNew **********************************************************
**
** Vdwall object constructor. This is normally called by the ajVdwallReadNew
** function. Fore-knowledge of the number of residues is required.
**
** @param  [r] n [ajuint] Number of residues
**
** @return [AjPVdwall] Pointer to a Vdwall object
** @category new [AjPVdwall] Default Vdwall constructor.
**
** @release 2.9.0
** @@
******************************************************************************/

AjPVdwall ajVdwallNew(ajuint n)
{
    AjPVdwall vdwall = NULL;

    AJNEW0(vdwall);

    vdwall->N = n;

    if (n)
        AJCNEW0(vdwall->Res, n);

    return vdwall;
}




/* @func ajVdwresNew **********************************************************
**
** Vdwres object constructor. This is normally called by the ajVdwallReadNew
** function. Fore-knowledge of the number of atoms is required.
**
** @param  [r] n [ajuint] Number of atoms
**
** @return [AjPVdwres] Pointer to a Vdwres object
** @category new [AjPVdwres] Default Vdwres constructor.
**
** @release 2.9.0
** @@
******************************************************************************/

AjPVdwres ajVdwresNew(ajuint n)
{
    ajuint i = 0U;

    AjPVdwres vdwres = NULL;

    AJNEW0(vdwres);

    vdwres->Id3 = ajStrNew();
    vdwres->N = n;

    if (n)
    {
        AJCNEW0(vdwres->Atm, n);

        for (i = 0U; i < n; i++)
            vdwres->Atm[i] = ajStrNew();

        AJCNEW0(vdwres->Rad, n);
    }

    return vdwres;
}




/* @func ajCmapNew ************************************************************
**
** Cmap object constructor. This is normally called by the ajCmapReadNew
** function. Fore-knowledge of the dimension (number of residues) for the
** contact map is required.
**
** @param  [r] n [ajuint] Dimension of contact map
**
** @return [AjPCmap] Pointer to a Cmap object
**
** @category new [AjPCmap] Default Cmap constructor.
**
** @release 2.9.0
** @@
******************************************************************************/

AjPCmap ajCmapNew(ajuint n)
{
    ajuint i = 0;
    
    AjPCmap cmap = NULL;

    AJNEW0(cmap);

    cmap->Id = ajStrNew();
    cmap->Domid = ajStrNew();
    cmap->Ligid = ajStrNew();
    cmap->Seq1 = ajStrNew();
    cmap->Seq2 = ajStrNew();
    cmap->Desc = ajStrNew();
    cmap->Chid1 = '.';
    cmap->Chid2 = '.';

    if (n)
    {
        /* Create the SQUARE contact map */
        cmap->Mat = ajUint2dNewRes((ajint) n);

        for (i = 0U; i < n; i++)
            ajUint2dPut(&cmap->Mat, i, n - 1, 0U);
    }

    cmap->Dim = n;
    cmap->Ncon = 0;

    return cmap;
}




/* @func ajPdbtospNew *********************************************************
**
** Pdbtosp object constructor. Fore-knowledge of the number of entries is
** required. This is normally called by the ajPdbtospReadCNew /
** ajPdbtospReadNew functions.
**
** @param [r] n [ajuint] Number of entries
**
** @return [AjPPdbtosp] Pointer to a Pdbtosp object
** @category new [AjPPdbtosp] Default Pdbtosp constructor.
**
** @release 2.9.0
** @@
******************************************************************************/

AjPPdbtosp ajPdbtospNew(ajuint n)
{
    ajuint i = 0U;

    AjPPdbtosp pdbtosp = NULL;

    AJNEW0(pdbtosp);

    pdbtosp->Pdb = ajStrNew();

    if (n)
    {
        AJCNEW0(pdbtosp->Acc, n);
        AJCNEW0(pdbtosp->Spr, n);

        for (i = 0U; i < n; i++)
        {
            pdbtosp->Acc[i] = ajStrNew();
            pdbtosp->Spr[i] = ajStrNew();
        }
    }

    pdbtosp->Number = n;

    return pdbtosp;
}




/* ======================================================================= */
/* =========================== destructors =============================== */
/* ======================================================================= */




/* @section Destructors *******************************************************
**
** All destructor functions receive the address of the instance to be
** deleted.  The original pointer is set to NULL so is ready for re-use.
**
******************************************************************************/




/* @func ajAtomDel ************************************************************
**
** Destructor for an AJAX Atom object.
**
** @param [d] Patom [AjPAtom*] AJAX Atom address
**
** @return [void]
** @category delete [AjPAtom] Default Atom destructor
**
** @release 1.8.0
** @@
******************************************************************************/

void ajAtomDel(AjPAtom *Patom)
{
    AjPAtom atom = NULL;

    if (!Patom || !*Patom)
        return;

    atom = *Patom;

    ajStrDel(&atom->Id3);
    ajStrDel(&atom->Atm);
    ajStrDel(&atom->Pdb);
#if AJFALSE
    ajStrDel(&atom->eId);
#endif
    AJFREE(atom);

    *Patom = NULL;

    return;
}




/* @func ajResidueDel *********************************************************
**
** Destructor for an AJAX Residue object.
**
** @param [d] Presidue [AjPResidue*] AJAX Residue address
**
** @return [void]
** @category delete [AjPResidue] Default AJAX Residue destructor
**
** @release 3.0.0
** @@
******************************************************************************/

void ajResidueDel(AjPResidue *Presidue)
{
    AjPResidue residue = NULL;

    if (!Presidue || !*Presidue)
        return;

    residue = *Presidue;

    ajStrDel(&residue->Pdb);
    ajStrDel(&residue->Id3);
    ajStrDel(&residue->eId);

    AJFREE(residue);

    *Presidue = NULL;

    return;
}




/* @func ajChainDel ***********************************************************
**
** Destructor for an AJAX Chain object.
**
** @param [d] Pchain [AjPChain*] AJAX Chain address
**
** @return [void]
** @category delete [AjPChain] Default AJAX Chain destructor.
**
** @release 1.8.0
** @@
******************************************************************************/

void ajChainDel(AjPChain *Pchain)
{
    AjPAtom atom = NULL;

    AjPChain chain = NULL;

    AjPResidue residue = NULL;

    if (!Pchain || !*Pchain)
        return;

    chain = *Pchain;

    while (ajListPop(chain->Atoms, (void **) &atom))
        ajAtomDel(&atom);

    ajListFree(&chain->Atoms);

    while (ajListPop(chain->Residues, (void **) &residue))
        ajResidueDel(&residue);

    ajListFree(&chain->Residues);

    ajStrDel(&chain->Seq);

    AJFREE(chain);

    *Pchain = NULL;

    return;
}




/* @func ajPdbDel *************************************************************
**
** Destructor for an AJAX PDB object.
**
** @param [d] Ppdb [AjPPdb*] AJAX PDB address
**
** @return [void]
** @category delete [AjPPdb] Default AJAX PDB destructor.
**
** @release 1.8.0
** @@
******************************************************************************/

void ajPdbDel(AjPPdb *Ppdb)
{
    ajuint i = 0U;

    AjPAtom atom = NULL;

    AjPPdb pdb = NULL;

    if (!Ppdb || !*Ppdb)
        return;

    pdb = *Ppdb;

    ajStrDel(&pdb->Pdb);
    ajStrDel(&pdb->Compnd);
    ajStrDel(&pdb->Source);

    ajChararrDel(&pdb->gpid);

    while (ajListPop(pdb->Water, (void **) &atom))
        ajAtomDel(&atom);

    ajListFree(&pdb->Water);

    while (ajListPop(pdb->Groups, (void **) &atom))
        ajAtomDel(&atom);

    ajListFree(&pdb->Groups);

    for (i = 0U; i < pdb->Nchn; i++)
        ajChainDel(&pdb->Chains[i]);

    AJFREE(pdb->Chains);

    AJFREE(pdb);

    *Ppdb = NULL;

    return;
}




/* @func ajHetentDel **********************************************************
**
** Destructor for an AJAX Hetent object.
**
** @param [d] Phetent [AjPHetent*] AJAX Hetent address
**
** @return [void]
** @category delete [AjPHetent] Default AJAX Hetent destructor.
**
** @release 2.9.0
** @@
******************************************************************************/

void ajHetentDel(AjPHetent *Phetent)
{
    AjPHetent hetent = NULL;

    if (!Phetent || !*Phetent)
        return;

    hetent = *Phetent;

    ajStrDel(&hetent->abv);
    ajStrDel(&hetent->syn);
    ajStrDel(&hetent->ful);

    AJFREE(hetent);

    *Phetent = NULL;

    return;
}




/* @func ajHetDel *************************************************************
**
** Destructor for an AJAX Het object.
**
** @param [d] Phet [AjPHet*] AJAX Het address
**
** @return [void]
** @category delete [AjPHet] Default AJAX Het destructor.
**
** @release 2.9.0
** @@
******************************************************************************/

void ajHetDel(AjPHet *Phet)
{
    ajuint i = 0U;

    AjPHet het = NULL;

    if (!Phet || !*Phet)
        return;

    het = *Phet;

    if (het->Entries)
    {
        for (i = 0U; i < het->Number; i++)
            ajHetentDel(&het->Entries[i]);

        AJFREE(het->Entries);
    }

    AJFREE(het);

    *Phet = NULL;

    return;
}




/* @func ajVdwallDel **********************************************************
**
** Destructor for an AJAX Vdwall object.
**
** @param [d] Pvdwall [AjPVdwall*] AJAX Vdwall address
**
** @return [void]
** @category delete [AjPVdwall] Default AJAX Vdwall destructor.
**
** @release 2.9.0
** @@
******************************************************************************/

void ajVdwallDel(AjPVdwall *Pvdwall)
{
    ajuint i = 0U;

    AjPVdwall vdwall = NULL;
    
    if(!Pvdwall || !*Pvdwall)
        return;

    vdwall = *Pvdwall;

    for (i = 0U; i < vdwall->N; i++)
        ajVdwresDel(&vdwall->Res[i]);

    AJFREE(vdwall->Res);
    AJFREE(vdwall);
    
    *Pvdwall = NULL;

    return;
}




/* @func ajVdwresDel **********************************************************
**
** Destructor for an AJAX Vdwres object.
**
** @param [d] Pvdwres [AjPVdwres*] AJAX Vdwres address
**
** @return [void]
** @category delete [AjPVdwres] Default AJAX Vdwres destructor.
**
** @release 2.9.0
** @@
******************************************************************************/

void ajVdwresDel(AjPVdwres *Pvdwres)
{
    ajuint i = 0U;

    AjPVdwres vdwres = NULL;
    
    if(!Pvdwres || !*Pvdwres)
        return;
    
    vdwres = *Pvdwres;
    
    ajStrDel(&vdwres->Id3);

    for (i = 0U; i < vdwres->N; i++)
        ajStrDel(&vdwres->Atm[i]);

    AJFREE(vdwres->Atm);
    AJFREE(vdwres->Rad);
    AJFREE(vdwres);
    
    *Pvdwres = NULL;

    return;
}




/* @func ajCmapDel ************************************************************
**
** Destructor for an AJAX Cmap object.
**
** @param [d] Pcmap [AjPCmap*] AJAX Cmap address
**
** @return [void]
** @category delete [AjPCmap] Default AJAX Cmap destructor.
**
** @release 2.9.0
** @@
******************************************************************************/

void ajCmapDel(AjPCmap *Pcmap)
{
    AjPCmap cmap = NULL;
    
    if (!Pcmap || !*Pcmap)
        return;

    cmap = *Pcmap;

    ajStrDel(&cmap->Id);
    ajStrDel(&cmap->Domid);
    ajStrDel(&cmap->Ligid);
    ajStrDel(&cmap->Seq1);
    ajStrDel(&cmap->Seq2);
    ajStrDel(&cmap->Desc);

    ajUint2dDel(&cmap->Mat);

    AJFREE(cmap);

    *Pcmap = NULL;

    return;
}




/* @func ajPdbtospDel *********************************************************
**
** Destructor for an AJAX Pdbtosp object.
**
** @param [d] Ppdbtosp [AjPPdbtosp*] AJAX Pdbtosp address
**
** @return [void]
** @category delete [AjPPdbtosp] Default AJAX Pdbtosp destructor.
**
** @release 2.9.0
** @@
******************************************************************************/

void ajPdbtospDel(AjPPdbtosp *Ppdbtosp)
{
    ajuint i = 0U;

    AjPPdbtosp pdbtosp = NULL;

    if (!Ppdbtosp || !*Ppdbtosp)
        return;

    pdbtosp = *Ppdbtosp;

    ajStrDel(&pdbtosp->Pdb);

    if (pdbtosp->Number)
    {
        for (i = 0U; i < pdbtosp->Number; i++)
        {
            ajStrDel(&pdbtosp->Acc[i]);
            ajStrDel(&pdbtosp->Spr[i]);
        }

        AJFREE(pdbtosp->Acc);
        AJFREE(pdbtosp->Spr);
    }

    AJFREE(pdbtosp);
    
    *Ppdbtosp = NULL;

    return;
}




/* ======================================================================= */
/* ============================ Assignments ============================== */
/* ======================================================================= */




/* @section Assignments *******************************************************
**
** These functions overwrite the instance provided as the first argument
** A NULL value is always acceptable so these functions are often used to
** create a new instance by assignment.
**
******************************************************************************/




/* @func ajAtomCopy ***********************************************************
**
** Copies the data from an AJAX Atom object to an AJAX Atom object;
** the new object is created if needed.
**
** IMPORTANT - THIS DOES NOT COPY THE eNum & eType ELEMENTS, WHICH ARE SET
** TO ZERO and '.' INSTEAD.
**
** @param [w] Pto  [AjPAtom*] AJAX Atom address
** @param [r] from [const AjPAtom] AJAX Atom
**
** @return [AjBool] True on success
**
** @release 2.9.0
** @@
******************************************************************************/

AjBool ajAtomCopy(AjPAtom *Pto, const AjPAtom from)
{
    if (!Pto)
    {
        ajWarn("Bad arg (NULL) passed to ajAtomCopy");
        return ajFalse;
    }

    if (!*Pto)
        *Pto = ajAtomNew();

    (*Pto)->Mod = from->Mod;
    (*Pto)->Chn = from->Chn;
    (*Pto)->Gpn = from->Gpn;
    (*Pto)->Idx = from->Idx;
    ajStrAssignS(&((*Pto)->Pdb), from->Pdb);
    (*Pto)->Id1 = from->Id1;
    ajStrAssignS(&((*Pto)->Id3), from->Id3);
    (*Pto)->Type = from->Type;
    ajStrAssignS(&((*Pto)->Atm), from->Atm);
    (*Pto)->X = from->X;
    (*Pto)->Y = from->Y;
    (*Pto)->Z = from->Z;
    (*Pto)->O = from->O;
    (*Pto)->B = from->B;

    return ajTrue;
}




/* @func ajResidueCopy ********************************************************
**
** Copies the data from a Residue object to an Residue object; the new object
** is created if needed.
**
** IMPORTANT - THIS DOES NOT COPY THE eNum & eType ELEMENTS, WHICH ARE SET
** TO ZERO and '.' INSTEAD.
**
** @param [w] Pto  [AjPResidue*] AJAX Residue address
** @param [r] from [const AjPResidue] AJAX Residue
**
** @return [AjBool] True on success
**
** @release 3.0.0
** @@
******************************************************************************/

AjBool ajResidueCopy(AjPResidue *Pto, const AjPResidue from)
{
    if (!Pto)
    {
        ajWarn("Bad arg (NULL) passed to ajResidueCopy");

        return ajFalse;
    }

    if (!*Pto)
        *Pto = ajResidueNew();

    (*Pto)->Mod = from->Mod;
    (*Pto)->Chn = from->Chn;
    (*Pto)->Idx = from->Idx;
    ajStrAssignS(&((*Pto)->Pdb), from->Pdb);
    (*Pto)->Id1 = from->Id1;
    ajStrAssignS(&((*Pto)->Id3), from->Id3);
    (*Pto)->Phi = from->Phi;
    (*Pto)->Psi = from->Psi;
    (*Pto)->Area = from->Area;

    (*Pto)->eNum = from->eNum;
    ajStrAssignS(&((*Pto)->eId), from->eId);
    (*Pto)->eType = from->eType;
    (*Pto)->eClass = from->eClass;
    (*Pto)->eStrideNum = from->eStrideNum;
    (*Pto)->eStrideType = from->eStrideType;

    (*Pto)->all_abs = from->all_abs;
    (*Pto)->all_rel = from->all_rel;
    (*Pto)->side_abs = from->side_abs;
    (*Pto)->side_rel = from->side_rel;
    (*Pto)->main_abs = from->main_abs;
    (*Pto)->main_rel = from->main_rel;
    (*Pto)->npol_abs = from->npol_abs;
    (*Pto)->npol_rel = from->npol_rel;
    (*Pto)->pol_abs = from->pol_abs;
    (*Pto)->pol_rel = from->pol_rel;

    return ajTrue;
}




/* @func ajAtomListCopy *******************************************************
**
** Read a list of Atom structures and writes a copy of the list.  The
** data are copied and the list is created if necessary.
**
** @param [w] Pto      [AjPList*] AJAX List of AJAX Atom objects to write
** @param [r] from     [const AjPList]   List of Atom objects to read
**
** @return [AjBool] True if list was copied ok.
**
** @release 2.9.0
** @@
******************************************************************************/

AjBool ajAtomListCopy(AjPList *Pto, const AjPList from)
{
    AjIList iter = NULL;
    AjPAtom hit = NULL;
    AjPAtom new = NULL;

    /* Check arg's */
    if (!from || !Pto)
    {
        ajWarn("Bad arg's passed to ajAtomListCopy\n");
        return ajFalse;
    }

    /* Allocate the new list, if necessary */
    if (!*Pto)
        *Pto = ajListNew();

    /* Initialise the iterator */
    iter = ajListIterNewread(from);

    /* Iterate through the list of Atom objects */
    while ((hit = (AjPAtom) ajListIterGet(iter)))
    {
        new = ajAtomNew();

        ajAtomCopy(&new, hit);

        /* Push scophit onto list */
        ajListPushAppend(*Pto, new);
    }

    ajListIterDel(&iter);

    return ajTrue;
}




/* @func ajResidueListCopy ****************************************************
**
** Read a list of Residue structures and writes a copy of the list.  The
** data are copied and the list is created if necessary.
**
** @param [w] to       [AjPList *] List of Residue objects to write
** @param [r] from     [const AjPList]   List of Residue objects to read
**
** @return [AjBool] True if list was copied ok.
**
** @release 3.0.0
** @@
******************************************************************************/

AjBool ajResidueListCopy(AjPList *to, const AjPList from)
{
    AjIList iter = NULL;
    AjPResidue hit = NULL;
    AjPResidue new = NULL;

    /* Check arg's */
    if (!from || !to)
    {
        ajWarn("Bad arg's passed to ajResidueListCopy\n");
        return ajFalse;
    }

    /* Allocate the new list, if necessary */
    if (!(*to))
        *to = ajListNew();

    /* Initialise the iterator */
    iter = ajListIterNewread(from);

    /* Iterate through the list of Atom objects */
    while ((hit = (AjPResidue) ajListIterGet(iter)))
    {
        new = ajResidueNew();

        ajResidueCopy(&new, hit);

        /* Push scophit onto list */
        ajListPushAppend(*to, new);
    }

    ajListIterDel(&iter);
    return ajTrue;
}




/* @func ajPdbCopy ************************************************************
**
** Copies data from one AHAX PDB object to another; the new object is
** always created.
**
**
** @param [w] Pto  [AjPPdb*] AJAX PDB address
** @param [r] from [const AjPPdb] AJAX PDB
**
** @return [AjBool] True on success
**
** @release 2.9.0
** @@
******************************************************************************/

AjBool ajPdbCopy(AjPPdb *Pto, const AjPPdb from)
{
    ajuint i = 0U;

    if (!from)
    {
        ajWarn("NULL arg passed to ajPdbCopy");

        return ajFalse;
    }

    if ((*Pto))
    {
        ajWarn("Pointer passed to ajPdbCopy should be NULL but isn't !");

        return ajFalse;
    }

    /* Copy data in Pdb object */
    (*Pto) = ajPdbNew(from->Nchn);
    ajStrAssignS(&((*Pto)->Pdb), from->Pdb);
    ajStrAssignS(&((*Pto)->Compnd), from->Compnd);
    ajStrAssignS(&((*Pto)->Source), from->Source);
    (*Pto)->Method = from->Method;
    (*Pto)->Reso = from->Reso;
    (*Pto)->Nmod = from->Nmod;
    (*Pto)->Nchn = from->Nchn;
    (*Pto)->Ngp = from->Ngp;

    for (i = 0U; i < from->Ngp; i++)
        ajChararrPut(&((*Pto)->gpid), i, ajChararrGet(from->gpid, i));

    ajListFree(&((*Pto)->Groups));
    ajListFree(&((*Pto)->Water));

#if AJFALSE
    (*Pto)->Groups = ajAtomListCopy(from->Groups);
    (*Pto)->Water = ajAtomListCopy(from->Water);
#endif
    if (!ajAtomListCopy(&(*Pto)->Groups, from->Groups))
        ajFatal("Error copying Groups list");

    if (!ajAtomListCopy(&(*Pto)->Water, from->Water))
        ajFatal("Error copying Water list");

    /* Copy data in Chain objects */
    for (i = 0; i < from->Nchn; i++)
    {
        ajListFree(&((*Pto)->Chains[i]->Atoms));

        (*Pto)->Chains[i]->Id = from->Chains[i]->Id;
        (*Pto)->Chains[i]->Nres = from->Chains[i]->Nres;
        (*Pto)->Chains[i]->Nlig = from->Chains[i]->Nlig;
        (*Pto)->Chains[i]->numHelices = from->Chains[i]->numHelices;
        (*Pto)->Chains[i]->numStrands = from->Chains[i]->numStrands;
        ajStrAssignS(&((*Pto)->Chains[i]->Seq), from->Chains[i]->Seq);
#if AJFALSE
        (*Pto)->Chains[i]->Atoms = ajAtomListCopy(from->Chains[i]->Atoms);
#endif
        if (!ajAtomListCopy(&(*Pto)->Chains[i]->Atoms, from->Chains[i]->Atoms))
            ajFatal("Error copying Atoms list");
        if (!ajResidueListCopy(&(*Pto)->Chains[i]->Residues,
                               from->Chains[i]->Residues))
            ajFatal("Error copying Residues list");
    }

    return ajTrue;
}




/* ======================================================================= */
/* ============================= Modifiers =============================== */
/* ======================================================================= */




/* @section Modifiers *********************************************************
**
** These functions use the contents of an instance and update them.
**
******************************************************************************/




/* ======================================================================= */
/* ========================== Operators ===================================*/
/* ======================================================================= */




/* @section Operators *********************************************************
**
** These functions use the contents of an instance but do not make any
** changes.
**
******************************************************************************/




/* @func ajResidueSSEnv *******************************************************
**
** Assigns secondary structure environment of an AJAX Residue
**
** @param  [r] residue [const AjPResidue] AJAX Residue
** @param  [w] SEnv [char*] Character for the Secondary structure environment
** @param  [w] flog [AjPFile] Log file
** @return [AjBool] ajTrue on success
**
** @release 3.0.0
** @@
******************************************************************************/

AjBool ajResidueSSEnv(const AjPResidue residue, char *SEnv, AjPFile flog)
{
    *SEnv = '\0';
    ajFmtPrintF(flog, "R:%c-%d S:%c A:%.2f\n", residue->Id1, residue->Idx,
                residue->eStrideType, residue->side_rel);

    if (residue->eStrideType == 'H' ||
        residue->eStrideType == 'G')
        *SEnv = 'H';
    else if (residue->eStrideType == 'E' ||
             residue->eStrideType == 'B' ||
             residue->eStrideType == 'b')
        *SEnv = 'S';
    else if (residue->eStrideType == 'T' ||
             residue->eStrideType == 'C' ||
             residue->eStrideType == 'I')
        *SEnv = 'C';
    /* If no stride assignment, get pdb assignment */
    else if (residue->eStrideType == '.')
    {
        if (residue->eType == 'H')
            *SEnv = 'H';
        else if (residue->eType == 'E')
            *SEnv = 'S';
        else if (residue->eType == 'C' ||
                 residue->eType == 'T')
            *SEnv = 'C';
        else if (residue->eType == '.')
        {
            ajFmtPrintF(flog, "SEnv unknown for residue %d\n", residue->Idx);
            /* *SEnv='C'; */
            *SEnv = '\0';
            return ajFalse;
        }
    }

    return ajTrue;
}




/* @func ajResidueEnv1 ********************************************************
**
** Assigns environment based only of side chain accessibility and secondary
** structure.  Assigns environment of "*" as default.
**
** @param [r] residue [const AjPResidue] AJAX Residue
** @param [r] SEnv [char]          Secondary structure environment code
** @param [w] OEnv [AjPStr*]       Character for the overall environment class
** @param  [w] flog [AjPFile] Log file
** @return [ajint] Number of environments
**
** @release 3.0.0
** @@
******************************************************************************/

ajint ajResidueEnv1(const AjPResidue residue, char SEnv, AjPStr *OEnv,
                    AjPFile flog)
{
    if (SEnv == '\0')
    {
        ajStrSetClear(OEnv);

        return 0;
    }

    if ((residue->side_rel <= 15) && (SEnv == 'H'))
        ajStrAssignC(OEnv, "AA");
    else if ((residue->side_rel <= 15) && (SEnv == 'S'))
        ajStrAssignC(OEnv, "AB");
    else if ((residue->side_rel <= 15) && (SEnv == 'C'))
        ajStrAssignC(OEnv, "AC");
    else if ((residue->side_rel > 15) &&
             (residue->side_rel <= 30) && (SEnv == 'H'))
        ajStrAssignC(OEnv, "AD");
    else if ((residue->side_rel > 15) &&
             (residue->side_rel <= 30) && (SEnv == 'S'))
        ajStrAssignC(OEnv, "AE");
    else if ((residue->side_rel > 15) &&
             (residue->side_rel <= 30) && (SEnv == 'C'))
        ajStrAssignC(OEnv, "AF");
    else if ((residue->side_rel > 30) &&
             (residue->side_rel <= 45) && (SEnv == 'H'))
        ajStrAssignC(OEnv, "AG");
    else if ((residue->side_rel > 30) &&
             (residue->side_rel <= 45) && (SEnv == 'S'))
        ajStrAssignC(OEnv, "AH");
    else if ((residue->side_rel > 30) &&
             (residue->side_rel <= 45) && (SEnv == 'C'))
        ajStrAssignC(OEnv, "AI");
    else if ((residue->side_rel > 45) &&
             (residue->side_rel <= 60) && (SEnv == 'H'))
        ajStrAssignC(OEnv, "AJ");
    else if ((residue->side_rel > 45) &&
             (residue->side_rel <= 60) && (SEnv == 'S'))
        ajStrAssignC(OEnv, "AK");
    else if ((residue->side_rel > 45) &&
             (residue->side_rel <= 60) && (SEnv == 'C'))
        ajStrAssignC(OEnv, "AL");
    else if ((residue->side_rel > 60) &&
             (residue->side_rel <= 75) && (SEnv == 'H'))
        ajStrAssignC(OEnv, "AM");
    else if ((residue->side_rel > 60) &&
             (residue->side_rel <= 75) && (SEnv == 'S'))
        ajStrAssignC(OEnv, "AN");
    else if ((residue->side_rel > 60) &&
             (residue->side_rel <= 75) && (SEnv == 'C'))
        ajStrAssignC(OEnv, "AO");
    else if ((residue->side_rel > 75) &&
             (residue->side_rel <= 90) && (SEnv == 'H'))
        ajStrAssignC(OEnv, "AP");
    else if ((residue->side_rel > 75) &&
             (residue->side_rel <= 90) && (SEnv == 'S'))
        ajStrAssignC(OEnv, "AQ");
    else if ((residue->side_rel > 75) &&
             (residue->side_rel <= 90) && (SEnv == 'C'))
        ajStrAssignC(OEnv, "AR");
    else if ((residue->side_rel > 90) && (SEnv == 'H'))
        ajStrAssignC(OEnv, "AS");
    else if ((residue->side_rel > 90) && (SEnv == 'S'))
        ajStrAssignC(OEnv, "AT");
    else if ((residue->side_rel > 90) && (SEnv == 'C'))
        ajStrAssignC(OEnv, "AU");
    else
    {
        ajStrSetClear(OEnv);
        ajFmtPrintF(flog, "OEnv unassigned for residue %d\n", residue->Idx);
        return 0;
    }

    /* The total number of environments */
    return 21;
}




/* @func ajResidueEnv2 ********************************************************
**
** Assigns environment based on side chain accessibility and secondary
** structure.  Assigns environment of "*" as default.
**
** @param [r] residue [const AjPResidue] AJAX Residue
** @param [r] SEnv [char] Secondary structure environment code
** @param [w] OEnv [AjPStr*] Character for the overall environment class
** @param  [w] flog [AjPFile] Log file
** @return [ajint] Number of environments
**
** @release 3.0.0
** @@
******************************************************************************/

ajint ajResidueEnv2(const AjPResidue residue, char SEnv, AjPStr *OEnv,
                    AjPFile flog)
{
    float BLimit = 40.0F;       /* Upper limit for the relative solvent
                                 * accessible area for a buried residue */
    float PBLimit = 114.0F;     /* Upper limit for the relative solvent
                                 * accessible area for a Partially buried
                                 * residue */

    float HLimit = 45.0F;       /* Upper limit for the fraction polar measure
                                 * of a Hydrophobic residue */
    float MPLimit = 67.0F;      /* Upper limit for the fraction polar measure
                                 * of a Moderately polar residue */

    AjPStr BEnv = NULL;

    if (!residue)
    {
        ajWarn("No residue to ajResidueEnv");

        return 0;
    }

    ajStrSetClear(OEnv);
    BEnv = ajStrNew();

    ajFmtPrintF(flog, "R:%c-%d S:%c A:%.2f f:%.2f\n",
                residue->Id1, residue->Idx, residue->eType,
                residue->side_rel, residue->pol_rel);

    /* Assign the basic classes */

    /* Buried, Hydrophobic */
    if ((residue->side_rel <= BLimit) &&
        (residue->pol_rel <= HLimit))
        ajStrAssignC(&BEnv, "B1");
    /* buried, moderately polar */
    else if ((residue->side_rel <= BLimit) &&
             (residue->pol_rel > HLimit) &&
             (residue->pol_rel <= MPLimit))
        ajStrAssignC(&BEnv, "B2");
    /* buried, polar */
    else if ((residue->side_rel <= BLimit) &&
             (residue->pol_rel > MPLimit))
        ajStrAssignC(&BEnv, "B3");
    /* Partially buried, moderately Polar */
    else if ((residue->side_rel > BLimit) &&
             (residue->side_rel <= PBLimit) &&
             (residue->pol_rel <= MPLimit))
        ajStrAssignC(&BEnv, "P1");
    /* Partially buried, Polar */
    else if ((residue->side_rel > BLimit) &&
             (residue->side_rel <= PBLimit) &&
             (residue->pol_rel > MPLimit))
        ajStrAssignC(&BEnv, "P2");
    /* Exposed */
    else if (residue->side_rel > PBLimit)
        ajStrAssignC(&BEnv, "E");
    else
    {
        ajStrSetClear(OEnv);
        ajFmtPrintF(flog, "BEnv unassigned for residue %d\n", residue->Idx);
        ajStrDel(&BEnv);
        return 0;
    }

    /* Assign overall environment class */
    if ((ajStrMatchC(BEnv, "B1")) && (SEnv == 'H'))
        ajStrAssignC(OEnv, "AA");
    else if ((ajStrMatchC(BEnv, "B1")) && (SEnv == 'S'))
        ajStrAssignC(OEnv, "AB");
    else if ((ajStrMatchC(BEnv, "B1")) && (SEnv == 'C'))
        ajStrAssignC(OEnv, "AC");
    else if ((ajStrMatchC(BEnv, "B2")) && (SEnv == 'H'))
        ajStrAssignC(OEnv, "AD");
    else if ((ajStrMatchC(BEnv, "B2")) && (SEnv == 'S'))
        ajStrAssignC(OEnv, "AE");
    else if ((ajStrMatchC(BEnv, "B2")) && (SEnv == 'C'))
        ajStrAssignC(OEnv, "AF");
    else if ((ajStrMatchC(BEnv, "B3")) && (SEnv == 'H'))
        ajStrAssignC(OEnv, "AG");
    else if ((ajStrMatchC(BEnv, "B3")) && (SEnv == 'S'))
        ajStrAssignC(OEnv, "AH");
    else if ((ajStrMatchC(BEnv, "B3")) && (SEnv == 'C'))
        ajStrAssignC(OEnv, "AI");
    else if ((ajStrMatchC(BEnv, "P1")) && (SEnv == 'H'))
        ajStrAssignC(OEnv, "AJ");
    else if ((ajStrMatchC(BEnv, "P1")) && (SEnv == 'S'))
        ajStrAssignC(OEnv, "AK");
    else if ((ajStrMatchC(BEnv, "P1")) && (SEnv == 'C'))
        ajStrAssignC(OEnv, "AL");
    else if ((ajStrMatchC(BEnv, "P2")) && (SEnv == 'H'))
        ajStrAssignC(OEnv, "AM");
    else if ((ajStrMatchC(BEnv, "P2")) && (SEnv == 'S'))
        ajStrAssignC(OEnv, "AN");
    else if ((ajStrMatchC(BEnv, "P2")) && (SEnv == 'C'))
        ajStrAssignC(OEnv, "AO");
    else if ((ajStrMatchC(BEnv, "E")) && (SEnv == 'H'))
        ajStrAssignC(OEnv, "AP");
    else if ((ajStrMatchC(BEnv, "E")) && (SEnv == 'S'))
        ajStrAssignC(OEnv, "AQ");
    else if ((ajStrMatchC(BEnv, "E")) && (SEnv == 'C'))
        ajStrAssignC(OEnv, "AR");
    else
    {
        ajStrSetClear(OEnv);
        ajFmtPrintF(flog, "OEnv unassigned for residue %d\n", residue->Idx);
        ajStrDel(&BEnv);

        return 0;
    }

    ajStrDel(&BEnv);

    return 18;

}




/* @func ajResidueEnv3 ********************************************************
**
** Assigns environment based on side chain accessibility and secondary
** structure.  Assigns environment of "*" as default.
**
** @param [r] residue [const AjPResidue] AJAX Residue
** @param [r] SEnv [char] Secondary structure environment code
** @param [w] OEnv [AjPStr*] Character for the overall environment class
** @param  [w] flog [AjPFile] Log file
** @return [ajint] Number of environments
**
** @release 3.0.0
** @@
******************************************************************************/

ajint ajResidueEnv3(const AjPResidue residue, char SEnv, AjPStr *OEnv,
                    AjPFile flog)
{
    float BLimit = 5.0F;        /* Upper limit for the relative solvent
                                 * accessible area for a buried residue */
    float PBLimit = 25.0F;      /* Upper limit for the relative solvent
                                 * accessible area for a Partially buried
                                 * residue */

    float PolLimit1 = 10.0F;
    float PolLimit2 = 30.0F;
    float PolLimit3 = 50.0F;
    float PolLimit4 = 80.0F;

    AjPStr BEnv = NULL;

    if (!residue)
    {
        ajWarn("No residue to ajResidueEnv");

        return 0;
    }

    ajStrSetClear(OEnv);
    BEnv = ajStrNew();

    ajFmtPrintF(flog, "R:%c-%d S:%c A:%.2f f:%.2f\n",
                residue->Id1, residue->Idx, residue->eType,
                residue->side_rel, residue->pol_rel);

    /* Assign the basic classes */

    /* Buried, Hydrophobic */
    if ((residue->side_rel <= BLimit) &&
        (residue->pol_rel <= PolLimit1))
        ajStrAssignC(&BEnv, "B1");
    /* buried, moderately polar */
    else if ((residue->side_rel <= BLimit) &&
             (residue->pol_rel > PolLimit1) &&
             (residue->pol_rel <= PolLimit2))
        ajStrAssignC(&BEnv, "B2");
    else if ((residue->side_rel <= BLimit) &&
             (residue->pol_rel > PolLimit2) &&
             (residue->pol_rel <= PolLimit3))
        ajStrAssignC(&BEnv, "B3");
    else if ((residue->side_rel <= BLimit) &&
             (residue->pol_rel > PolLimit3) &&
             (residue->pol_rel <= PolLimit4))
        ajStrAssignC(&BEnv, "B4");
    else if ((residue->side_rel <= BLimit) &&
             (residue->pol_rel > PolLimit4))
        ajStrAssignC(&BEnv, "B5");
    /* Partially buried, moderately Polar */
    else if ((residue->side_rel > BLimit) &&
             (residue->side_rel <= PBLimit) &&
             (residue->pol_rel <= PolLimit3))
        ajStrAssignC(&BEnv, "P1");
    /* Partially buried, Polar */
    else if ((residue->side_rel > BLimit) &&
             (residue->side_rel <= PBLimit) &&
             (residue->pol_rel > PolLimit3))
        ajStrAssignC(&BEnv, "P2");
    /* Exposed */
    else if (residue->side_rel > PBLimit)
        ajStrAssignC(&BEnv, "E");
    else
    {
        ajStrSetClear(OEnv);
        ajFmtPrintF(flog, "BEnv unassigned for residue %d\n", residue->Idx);

        ajStrDel(&BEnv);
        return 0;
    }

    /* Assign overall environment class */
    if ((ajStrMatchC(BEnv, "B1")) && (SEnv == 'H'))
        ajStrAssignC(OEnv, "AA");
    else if ((ajStrMatchC(BEnv, "B1")) && (SEnv == 'S'))
        ajStrAssignC(OEnv, "AB");
    else if ((ajStrMatchC(BEnv, "B1")) && (SEnv == 'C'))
        ajStrAssignC(OEnv, "AC");
    else if ((ajStrMatchC(BEnv, "B2")) && (SEnv == 'H'))
        ajStrAssignC(OEnv, "AD");
    else if ((ajStrMatchC(BEnv, "B2")) && (SEnv == 'S'))
        ajStrAssignC(OEnv, "AE");
    else if ((ajStrMatchC(BEnv, "B2")) && (SEnv == 'C'))
        ajStrAssignC(OEnv, "AF");
    else if ((ajStrMatchC(BEnv, "B3")) && (SEnv == 'H'))
        ajStrAssignC(OEnv, "AG");
    else if ((ajStrMatchC(BEnv, "B3")) && (SEnv == 'S'))
        ajStrAssignC(OEnv, "AH");
    else if ((ajStrMatchC(BEnv, "B3")) && (SEnv == 'C'))
        ajStrAssignC(OEnv, "AI");
    else if ((ajStrMatchC(BEnv, "B4")) && (SEnv == 'H'))
        ajStrAssignC(OEnv, "AJ");
    else if ((ajStrMatchC(BEnv, "B4")) && (SEnv == 'S'))
        ajStrAssignC(OEnv, "AK");
    else if ((ajStrMatchC(BEnv, "B4")) && (SEnv == 'C'))
        ajStrAssignC(OEnv, "AL");
    else if ((ajStrMatchC(BEnv, "B5")) && (SEnv == 'H'))
        ajStrAssignC(OEnv, "AM");
    else if ((ajStrMatchC(BEnv, "B5")) && (SEnv == 'S'))
        ajStrAssignC(OEnv, "AN");
    else if ((ajStrMatchC(BEnv, "B5")) && (SEnv == 'C'))
        ajStrAssignC(OEnv, "AO");
    else if ((ajStrMatchC(BEnv, "P1")) && (SEnv == 'H'))
        ajStrAssignC(OEnv, "AP");
    else if ((ajStrMatchC(BEnv, "P1")) && (SEnv == 'S'))
        ajStrAssignC(OEnv, "AQ");
    else if ((ajStrMatchC(BEnv, "P1")) && (SEnv == 'C'))
        ajStrAssignC(OEnv, "AR");
    else if ((ajStrMatchC(BEnv, "P2")) && (SEnv == 'H'))
        ajStrAssignC(OEnv, "AS");
    else if ((ajStrMatchC(BEnv, "P2")) && (SEnv == 'S'))
        ajStrAssignC(OEnv, "AT");
    else if ((ajStrMatchC(BEnv, "P2")) && (SEnv == 'C'))
        ajStrAssignC(OEnv, "AU");
    else if ((ajStrMatchC(BEnv, "E")) && (SEnv == 'H'))
        ajStrAssignC(OEnv, "AV");
    else if ((ajStrMatchC(BEnv, "E")) && (SEnv == 'S'))
        ajStrAssignC(OEnv, "AW");
    else if ((ajStrMatchC(BEnv, "E")) && (SEnv == 'C'))
        ajStrAssignC(OEnv, "AX");
    else
    {
        ajStrSetClear(OEnv);
        ajFmtPrintF(flog, "OEnv unassigned for residue %d\n", residue->Idx);

        ajStrDel(&BEnv);

        return 0;
    }

    ajStrDel(&BEnv);

    return 24;
}




/* @func ajResidueEnv4 ********************************************************
**
** Assigns environment based only of side chain accessibility and secondary
** structure.  Assigns environment of "*" as default.
**
** @param [r] residue [const AjPResidue] AJAX Residue
** @param [r] SEnv [char] Secondary structure environment code
** @param [w] OEnv [AjPStr*] Character for the overall environment class
** @param  [w] flog [AjPFile] Log file
** @return [ajint] Number of environments
**
** @release 3.0.0
** @@
******************************************************************************/

ajint ajResidueEnv4(const AjPResidue residue, char SEnv, AjPStr *OEnv,
                    AjPFile flog)
{
    ajStrSetClear(OEnv);

    if (SEnv == '\0')
    {
        ajStrSetClear(OEnv);

        return 0;
    }

    if ((residue->side_rel <= 5) && (SEnv == 'H'))
        ajStrAssignC(OEnv, "AA");
    else if ((residue->side_rel <= 5) && (SEnv == 'S'))
        ajStrAssignC(OEnv, "AB");
    else if ((residue->side_rel <= 5) && (SEnv == 'C'))
        ajStrAssignC(OEnv, "AC");
    else if ((residue->side_rel > 5) &&
             (residue->side_rel <= 25) && (SEnv == 'H'))
        ajStrAssignC(OEnv, "AD");
    else if ((residue->side_rel > 5) &&
             (residue->side_rel <= 25) && (SEnv == 'S'))
        ajStrAssignC(OEnv, "AE");
    else if ((residue->side_rel > 5) &&
             (residue->side_rel <= 25) && (SEnv == 'C'))
        ajStrAssignC(OEnv, "AF");
    else if ((residue->side_rel > 25) && (SEnv == 'H'))
        ajStrAssignC(OEnv, "AG");
    else if ((residue->side_rel > 25) && (SEnv == 'S'))
        ajStrAssignC(OEnv, "AH");
    else if ((residue->side_rel > 25) && (SEnv == 'C'))
        ajStrAssignC(OEnv, "AI");
    else
    {
        ajStrSetClear(OEnv);
        ajFmtPrintF(flog, "OEnv unassigned for residue %d\n", residue->Idx);

        return 0;
    }

    return 9;

}




/* @func ajResidueEnv5 ********************************************************
** Assigns environment based on side chain accessibility and secondary
** structure.  Assigns environment of "*" as default.
** @param [r] residue [const AjPResidue] AJAX Residue
** @param [r] SEnv [char]          Secondary structure environment code
** @param [w] OEnv [AjPStr*]       Character for the overall environment class
** @param  [w] flog [AjPFile] Log file
** @return [ajint] Number of environments
**
** @release 3.0.0
** @@
******************************************************************************/

ajint ajResidueEnv5(const AjPResidue residue, char SEnv, AjPStr *OEnv,
                    AjPFile flog)
{
    float BLimit = 5.0F;        /* Upper limit for the relative solvent
                                 * accessible area for a buried residue */
    float PBLimit = 25.0F;      /* Upper limit for the relative solvent
                                 * accessible area for a Partially buried
                                 * residue */

    float PolLimit1 = 10.0F;
    float PolLimit2 = 30.0F;
    float PolLimit3 = 50.0F;
    float PolLimit4 = 80.0F;

    AjPStr BEnv = NULL;

    if (!residue)
    {
        ajWarn("No residue to ajResidueEnv");

        return 0;
    }

    ajStrSetClear(OEnv);
    BEnv = ajStrNew();

    ajFmtPrintF(flog, "R:%c-%d S:%c A:%.2f f:%.2f\n",
                residue->Id1, residue->Idx, residue->eType,
                residue->side_rel, residue->pol_rel);

    /* Assign the basic classes */

    /* Buried */
    if (residue->side_rel <= BLimit)
        ajStrAssignC(&BEnv, "B");
    /* partially buried, hydrophobic */
    else if ((residue->side_rel > BLimit) &&
             (residue->side_rel <= PBLimit) &&
             (residue->pol_rel <= PolLimit1))
        ajStrAssignC(&BEnv, "P1");
    /* partially buried, moderately polar */
    else if ((residue->side_rel > BLimit) &&
             (residue->side_rel <= PBLimit) &&
             (residue->pol_rel > PolLimit1) &&
             (residue->pol_rel <= PolLimit2))
        ajStrAssignC(&BEnv, "P2");
    else if ((residue->side_rel > BLimit) &&
             (residue->side_rel <= PBLimit) &&
             (residue->pol_rel > PolLimit2) &&
             (residue->pol_rel <= PolLimit3))
        ajStrAssignC(&BEnv, "P3");
    else if ((residue->side_rel > BLimit) &&
             (residue->side_rel <= PBLimit) &&
             (residue->pol_rel > PolLimit3) &&
             (residue->pol_rel <= PolLimit4))
        ajStrAssignC(&BEnv, "P4");
    /* partially buried, polar */
    else if ((residue->side_rel > BLimit) &&
             (residue->side_rel <= PBLimit) &&
             (residue->pol_rel > PolLimit4))
        ajStrAssignC(&BEnv, "P5");
    /* Exposed, moderately Polar */
    else if ((residue->side_rel > PBLimit) &&
             (residue->pol_rel <= PolLimit3))
        ajStrAssignC(&BEnv, "E1");
    /* Exposed, Polar */
    else if ((residue->side_rel > PBLimit) &&
             (residue->pol_rel > PolLimit3))
        ajStrAssignC(&BEnv, "E2");
    else
    {
        ajStrSetClear(OEnv);
        ajFmtPrintF(flog, "BEnv unassigned for residue %d\n", residue->Idx);

        ajStrDel(&BEnv);

        return 0;
    }

    /* Assign overall environment class */
    if ((ajStrMatchC(BEnv, "B")) && (SEnv == 'H'))
        ajStrAssignC(OEnv, "AA");
    else if ((ajStrMatchC(BEnv, "B")) && (SEnv == 'S'))
        ajStrAssignC(OEnv, "AB");
    else if ((ajStrMatchC(BEnv, "B")) && (SEnv == 'C'))
        ajStrAssignC(OEnv, "AC");
    else if ((ajStrMatchC(BEnv, "P1")) && (SEnv == 'H'))
        ajStrAssignC(OEnv, "AD");
    else if ((ajStrMatchC(BEnv, "P1")) && (SEnv == 'S'))
        ajStrAssignC(OEnv, "AE");
    else if ((ajStrMatchC(BEnv, "P1")) && (SEnv == 'C'))
        ajStrAssignC(OEnv, "AF");
    else if ((ajStrMatchC(BEnv, "P2")) && (SEnv == 'H'))
        ajStrAssignC(OEnv, "AG");
    else if ((ajStrMatchC(BEnv, "P2")) && (SEnv == 'S'))
        ajStrAssignC(OEnv, "AH");
    else if ((ajStrMatchC(BEnv, "P2")) && (SEnv == 'C'))
        ajStrAssignC(OEnv, "AI");
    else if ((ajStrMatchC(BEnv, "P3")) && (SEnv == 'H'))
        ajStrAssignC(OEnv, "AJ");
    else if ((ajStrMatchC(BEnv, "P3")) && (SEnv == 'S'))
        ajStrAssignC(OEnv, "AK");
    else if ((ajStrMatchC(BEnv, "P3")) && (SEnv == 'C'))
        ajStrAssignC(OEnv, "AL");
    else if ((ajStrMatchC(BEnv, "P4")) && (SEnv == 'H'))
        ajStrAssignC(OEnv, "AM");
    else if ((ajStrMatchC(BEnv, "P4")) && (SEnv == 'S'))
        ajStrAssignC(OEnv, "AN");
    else if ((ajStrMatchC(BEnv, "P4")) && (SEnv == 'C'))
        ajStrAssignC(OEnv, "AO");
    else if ((ajStrMatchC(BEnv, "P5")) && (SEnv == 'H'))
        ajStrAssignC(OEnv, "AP");
    else if ((ajStrMatchC(BEnv, "P5")) && (SEnv == 'S'))
        ajStrAssignC(OEnv, "AQ");
    else if ((ajStrMatchC(BEnv, "P5")) && (SEnv == 'C'))
        ajStrAssignC(OEnv, "AR");
    else if ((ajStrMatchC(BEnv, "E1")) && (SEnv == 'H'))
        ajStrAssignC(OEnv, "AS");
    else if ((ajStrMatchC(BEnv, "E1")) && (SEnv == 'S'))
        ajStrAssignC(OEnv, "AT");
    else if ((ajStrMatchC(BEnv, "E1")) && (SEnv == 'C'))
        ajStrAssignC(OEnv, "AU");
    else if ((ajStrMatchC(BEnv, "E2")) && (SEnv == 'H'))
        ajStrAssignC(OEnv, "AV");
    else if ((ajStrMatchC(BEnv, "E2")) && (SEnv == 'S'))
        ajStrAssignC(OEnv, "AW");
    else if ((ajStrMatchC(BEnv, "E2")) && (SEnv == 'C'))
        ajStrAssignC(OEnv, "AX");
    else
    {
        ajStrSetClear(OEnv);
        ajFmtPrintF(flog, "OEnv unassigned for residue %d\n", residue->Idx);

        ajStrDel(&BEnv);
        return 0;
    }

    ajStrDel(&BEnv);

    return 24;
}




/* @func ajResidueEnv6 ********************************************************
**
** Assigns environment based on side chain accessibility and secondary
** structure.  Assigns environment of "*" as default.
**
** @param [r] residue [const AjPResidue] AJAX Residue
** @param [r] SEnv [char] Secondary structure environment code
** @param [w] OEnv [AjPStr*] Character for the overall environment class
** @param  [w] flog [AjPFile] Log file
** @return [ajint] Number of environments
**
** @release 3.0.0
** @@
******************************************************************************/

ajint ajResidueEnv6(const AjPResidue residue, char SEnv, AjPStr *OEnv,
                    AjPFile flog)
{
    float BLimit = 5.0F;        /* Upper limit for the relative solvent
                                 * accessible area for a buried residue */
    float PBLimit = 25.0F;      /* Upper limit for the relative solvent
                                 * accessible area for a Partially buried
                                 * residue */

    float PolLimit1 = 10.0F;
    float PolLimit2 = 30.0F;
    float PolLimit3 = 50.0F;
    float PolLimit4 = 80.0F;

    AjPStr BEnv = NULL;

    if (!residue)
    {
        ajWarn("No residue to ajResidueEnv");

        return 0;
    }

    ajStrSetClear(OEnv);
    BEnv = ajStrNew();

    ajFmtPrintF(flog, "R:%c-%d S:%c A:%.2f f:%.2f\n",
                residue->Id1, residue->Idx, residue->eType,
                residue->side_rel, residue->pol_rel);

    /* Assign the basic classes */

    /* Buried */
    if (residue->side_rel <= BLimit)
        ajStrAssignC(&BEnv, "B");
    /* Partially buried, hydrophobic */
    else if ((residue->side_rel > BLimit) &&
             (residue->side_rel <= PBLimit) &&
             (residue->pol_rel <= PolLimit3))
        ajStrAssignC(&BEnv, "P1");
    /* Partially buried, Polar */
    else if ((residue->side_rel > BLimit) &&
             (residue->side_rel <= PBLimit) &&
             (residue->pol_rel > PolLimit3))
        ajStrAssignC(&BEnv, "P2");
    /* partially buried, hydrophobic */
    else if ((residue->side_rel > PBLimit) &&
             (residue->pol_rel <= PolLimit1))
        ajStrAssignC(&BEnv, "E1");
    /* partially buried, moderately polar */
    else if ((residue->side_rel > PBLimit) &&
             (residue->pol_rel > PolLimit1) &&
             (residue->pol_rel <= PolLimit2))
        ajStrAssignC(&BEnv, "E2");
    else if ((residue->side_rel > PBLimit) &&
             (residue->pol_rel > PolLimit2) &&
             (residue->pol_rel <= PolLimit3))
        ajStrAssignC(&BEnv, "E3");
    else if ((residue->side_rel > PBLimit) &&
             (residue->pol_rel > PolLimit3) &&
             (residue->pol_rel <= PolLimit4))
        ajStrAssignC(&BEnv, "E4");
    /* partially buried, polar */
    else if ((residue->side_rel > PBLimit) &&
             (residue->pol_rel > PolLimit4))
        ajStrAssignC(&BEnv, "E5");
    else
    {
        ajStrSetClear(OEnv);
        ajFmtPrintF(flog, "BEnv unassigned for residue %d\n", residue->Idx);
        ajStrDel(&BEnv);

        return 0;
    }

    /* Assign overall environment class */
    if ((ajStrMatchC(BEnv, "B")) && (SEnv == 'H'))
        ajStrAssignC(OEnv, "AA");
    else if ((ajStrMatchC(BEnv, "B")) && (SEnv == 'S'))
        ajStrAssignC(OEnv, "AB");
    else if ((ajStrMatchC(BEnv, "B")) && (SEnv == 'C'))
        ajStrAssignC(OEnv, "AC");
    else if ((ajStrMatchC(BEnv, "P1")) && (SEnv == 'H'))
        ajStrAssignC(OEnv, "AD");
    else if ((ajStrMatchC(BEnv, "P1")) && (SEnv == 'S'))
        ajStrAssignC(OEnv, "AE");
    else if ((ajStrMatchC(BEnv, "P1")) && (SEnv == 'C'))
        ajStrAssignC(OEnv, "AF");
    else if ((ajStrMatchC(BEnv, "P2")) && (SEnv == 'H'))
        ajStrAssignC(OEnv, "AG");
    else if ((ajStrMatchC(BEnv, "P2")) && (SEnv == 'S'))
        ajStrAssignC(OEnv, "AH");
    else if ((ajStrMatchC(BEnv, "P2")) && (SEnv == 'C'))
        ajStrAssignC(OEnv, "AI");
    else if ((ajStrMatchC(BEnv, "E1")) && (SEnv == 'H'))
        ajStrAssignC(OEnv, "AJ");
    else if ((ajStrMatchC(BEnv, "E1")) && (SEnv == 'S'))
        ajStrAssignC(OEnv, "AK");
    else if ((ajStrMatchC(BEnv, "E1")) && (SEnv == 'C'))
        ajStrAssignC(OEnv, "AL");
    else if ((ajStrMatchC(BEnv, "E2")) && (SEnv == 'H'))
        ajStrAssignC(OEnv, "AM");
    else if ((ajStrMatchC(BEnv, "E2")) && (SEnv == 'S'))
        ajStrAssignC(OEnv, "AN");
    else if ((ajStrMatchC(BEnv, "E2")) && (SEnv == 'C'))
        ajStrAssignC(OEnv, "AO");
    else if ((ajStrMatchC(BEnv, "E3")) && (SEnv == 'H'))
        ajStrAssignC(OEnv, "AP");
    else if ((ajStrMatchC(BEnv, "E3")) && (SEnv == 'S'))
        ajStrAssignC(OEnv, "AQ");
    else if ((ajStrMatchC(BEnv, "E3")) && (SEnv == 'C'))
        ajStrAssignC(OEnv, "AR");
    else if ((ajStrMatchC(BEnv, "E4")) && (SEnv == 'H'))
        ajStrAssignC(OEnv, "AS");
    else if ((ajStrMatchC(BEnv, "E4")) && (SEnv == 'S'))
        ajStrAssignC(OEnv, "AT");
    else if ((ajStrMatchC(BEnv, "E4")) && (SEnv == 'C'))
        ajStrAssignC(OEnv, "AU");
    else if ((ajStrMatchC(BEnv, "E5")) && (SEnv == 'H'))
        ajStrAssignC(OEnv, "AV");
    else if ((ajStrMatchC(BEnv, "E5")) && (SEnv == 'S'))
        ajStrAssignC(OEnv, "AW");
    else if ((ajStrMatchC(BEnv, "E5")) && (SEnv == 'C'))
        ajStrAssignC(OEnv, "AX");
    else
    {
        ajStrSetClear(OEnv);
        ajFmtPrintF(flog, "OEnv unassigned for residue %d\n", residue->Idx);
        ajStrDel(&BEnv);

        return 0;
    }

    ajStrDel(&BEnv);

    return 24;
}




/* @func ajResidueEnv7 ********************************************************
**
** Assigns environment based on side chain accessibility and secondary
** structure.  Assigns environment of "*" as default.
**
** @param [r] residue [const AjPResidue] AJAX Residue
** @param [r] SEnv [char] Secondary structure environment code
** @param [w] OEnv [AjPStr*] Character for the overall environment class
** @param  [w] flog [AjPFile] Log file
** @return [ajint] Number of environments
**
** @release 3.0.0
** @@
******************************************************************************/

ajint ajResidueEnv7(const AjPResidue residue, char SEnv, AjPStr *OEnv,
                    AjPFile flog)
{
    float BLimit = 5.0F;        /* Upper limit for the relative solvent
                                 * accessible area for a buried residue */
    float PBLimit = 25.0F;      /* Upper limit for the relative solvent
                                 * accessible area for a Partially buried
                                 * residue */

    float PolLimit1 = 10.0F;
    float PolLimit2 = 50.0F;
    float PolLimit3 = 90.0F;

    AjPStr BEnv = NULL;

    if (!residue)
    {
        ajWarn("No residue to ajResidueEnv");
        ajStrDel(&BEnv);

        return 0;
    }

    ajStrSetClear(OEnv);
    BEnv = ajStrNew();

    ajFmtPrintF(flog, "R:%c-%d S:%c A:%.2f f:%.2f\n",
                residue->Id1, residue->Idx, residue->eType,
                residue->side_rel, residue->pol_rel);

    /* Assign the basic classes */

    /* Buried */
    if ((residue->side_rel <= BLimit) &&
        (residue->pol_rel <= PolLimit1))
        ajStrAssignC(&BEnv, "B1");
    else if ((residue->side_rel <= BLimit) &&
             (residue->pol_rel > PolLimit1) &&
             (residue->pol_rel <= PolLimit3))
        ajStrAssignC(&BEnv, "B2");
    else if ((residue->side_rel <= BLimit) &&
             (residue->pol_rel > PolLimit3))
        ajStrAssignC(&BEnv, "B3");
    else if ((residue->side_rel > BLimit) &&
             (residue->side_rel <= PBLimit))
        ajStrAssignC(&BEnv, "P");
    else if ((residue->side_rel > PBLimit) &&
             (residue->pol_rel <= PolLimit1))
        ajStrAssignC(&BEnv, "E1");
    else if ((residue->side_rel > PBLimit) &&
             (residue->pol_rel > PolLimit1) &&
             (residue->pol_rel <= PolLimit2))
        ajStrAssignC(&BEnv, "E2");
    else if ((residue->side_rel > PBLimit) &&
             (residue->pol_rel > PolLimit2) &&
             (residue->pol_rel <= PolLimit3))
        ajStrAssignC(&BEnv, "E3");
    else if ((residue->side_rel > PBLimit) && (residue->pol_rel > PolLimit3))
        ajStrAssignC(&BEnv, "E4");
    else
    {
        ajStrSetClear(OEnv);
        ajFmtPrintF(flog, "BEnv unassigned for residue %d\n", residue->Idx);

        ajStrDel(&BEnv);

        return 0;
    }

    /* Assign overall environment class */
    if ((ajStrMatchC(BEnv, "B1")) && (SEnv == 'H'))
        ajStrAssignC(OEnv, "AA");
    else if ((ajStrMatchC(BEnv, "B1")) && (SEnv == 'S'))
        ajStrAssignC(OEnv, "AB");
    else if ((ajStrMatchC(BEnv, "B1")) && (SEnv == 'C'))
        ajStrAssignC(OEnv, "AC");
    else if ((ajStrMatchC(BEnv, "B2")) && (SEnv == 'H'))
        ajStrAssignC(OEnv, "AD");
    else if ((ajStrMatchC(BEnv, "B2")) && (SEnv == 'S'))
        ajStrAssignC(OEnv, "AE");
    else if ((ajStrMatchC(BEnv, "B2")) && (SEnv == 'C'))
        ajStrAssignC(OEnv, "AF");
    else if ((ajStrMatchC(BEnv, "B3")) && (SEnv == 'H'))
        ajStrAssignC(OEnv, "AG");
    else if ((ajStrMatchC(BEnv, "B3")) && (SEnv == 'S'))
        ajStrAssignC(OEnv, "AH");
    else if ((ajStrMatchC(BEnv, "B3")) && (SEnv == 'C'))
        ajStrAssignC(OEnv, "AI");
    else if ((ajStrMatchC(BEnv, "P")) && (SEnv == 'H'))
        ajStrAssignC(OEnv, "AJ");
    else if ((ajStrMatchC(BEnv, "P")) && (SEnv == 'S'))
        ajStrAssignC(OEnv, "AK");
    else if ((ajStrMatchC(BEnv, "P")) && (SEnv == 'C'))
        ajStrAssignC(OEnv, "AL");
    else if ((ajStrMatchC(BEnv, "E1")) && (SEnv == 'H'))
        ajStrAssignC(OEnv, "AM");
    else if ((ajStrMatchC(BEnv, "E1")) && (SEnv == 'S'))
        ajStrAssignC(OEnv, "AN");
    else if ((ajStrMatchC(BEnv, "E1")) && (SEnv == 'C'))
        ajStrAssignC(OEnv, "AO");
    else if ((ajStrMatchC(BEnv, "E2")) && (SEnv == 'H'))
        ajStrAssignC(OEnv, "AP");
    else if ((ajStrMatchC(BEnv, "E2")) && (SEnv == 'S'))
        ajStrAssignC(OEnv, "AQ");
    else if ((ajStrMatchC(BEnv, "E2")) && (SEnv == 'C'))
        ajStrAssignC(OEnv, "AR");
    else if ((ajStrMatchC(BEnv, "E3")) && (SEnv == 'H'))
        ajStrAssignC(OEnv, "AS");
    else if ((ajStrMatchC(BEnv, "E3")) && (SEnv == 'S'))
        ajStrAssignC(OEnv, "AT");
    else if ((ajStrMatchC(BEnv, "E3")) && (SEnv == 'C'))
        ajStrAssignC(OEnv, "AU");
    else if ((ajStrMatchC(BEnv, "E4")) && (SEnv == 'H'))
        ajStrAssignC(OEnv, "AV");
    else if ((ajStrMatchC(BEnv, "E4")) && (SEnv == 'S'))
        ajStrAssignC(OEnv, "AW");
    else if ((ajStrMatchC(BEnv, "E4")) && (SEnv == 'C'))
        ajStrAssignC(OEnv, "AX");
    else
    {
        ajStrSetClear(OEnv);
        ajFmtPrintF(flog, "OEnv unassigned for residue %d\n", residue->Idx);
        ajStrDel(&BEnv);

        return 0;
    }

    ajStrDel(&BEnv);

    return 24;
}




/* @func ajResidueEnv8 ********************************************************
**
** Assigns environment based only of side chain accessibility and secondary
** structure.  Assigns environment of "*" as default.
**
** @param [r] residue [const AjPResidue] AJAX Residue
** @param [r] SEnv [char] Secondary structure environment code
** @param [w] OEnv [AjPStr*] Character for the overall environment class
** @param  [w] flog [AjPFile] Log file
** @return [ajint] Number of environments
**
** @release 3.0.0
** @@
******************************************************************************/

ajint ajResidueEnv8(const AjPResidue residue, char SEnv, AjPStr *OEnv,
                    AjPFile flog)
{
    ajStrSetClear(OEnv);

    if (SEnv == '\0')
    {
        ajStrSetClear(OEnv);

        return 0;
    }

    if ((residue->pol_rel <= 15) && (SEnv == 'H'))
        ajStrAssignC(OEnv, "AA");
    else if ((residue->pol_rel <= 15) && (SEnv == 'S'))
        ajStrAssignC(OEnv, "AB");
    else if ((residue->pol_rel <= 15) && (SEnv == 'C'))
        ajStrAssignC(OEnv, "AC");
    else if ((residue->pol_rel > 15) && (residue->pol_rel <= 30) && (SEnv == 'H'))
        ajStrAssignC(OEnv, "AD");
    else if ((residue->pol_rel > 15) && (residue->pol_rel <= 30) && (SEnv == 'S'))
        ajStrAssignC(OEnv, "AE");
    else if ((residue->pol_rel > 15) && (residue->pol_rel <= 30) && (SEnv == 'C'))
        ajStrAssignC(OEnv, "AF");
    else if ((residue->pol_rel > 30) && (residue->pol_rel <= 45) && (SEnv == 'H'))
        ajStrAssignC(OEnv, "AG");
    else if ((residue->pol_rel > 30) && (residue->pol_rel <= 45) && (SEnv == 'S'))
        ajStrAssignC(OEnv, "AH");
    else if ((residue->pol_rel > 30) && (residue->pol_rel <= 45) && (SEnv == 'C'))
        ajStrAssignC(OEnv, "AI");
    else if ((residue->pol_rel > 45) && (residue->pol_rel <= 60) && (SEnv == 'H'))
        ajStrAssignC(OEnv, "AJ");
    else if ((residue->pol_rel > 45) && (residue->pol_rel <= 60) && (SEnv == 'S'))
        ajStrAssignC(OEnv, "AK");
    else if ((residue->pol_rel > 45) && (residue->pol_rel <= 60) && (SEnv == 'C'))
        ajStrAssignC(OEnv, "AL");
    else if ((residue->pol_rel > 60) && (residue->pol_rel <= 75) && (SEnv == 'H'))
        ajStrAssignC(OEnv, "AM");
    else if ((residue->pol_rel > 60) && (residue->pol_rel <= 75) && (SEnv == 'S'))
        ajStrAssignC(OEnv, "AN");
    else if ((residue->pol_rel > 60) && (residue->pol_rel <= 75) && (SEnv == 'C'))
        ajStrAssignC(OEnv, "AO");
    else if ((residue->pol_rel > 75) && (residue->pol_rel <= 90) && (SEnv == 'H'))
        ajStrAssignC(OEnv, "AP");
    else if ((residue->pol_rel > 75) && (residue->pol_rel <= 90) && (SEnv == 'S'))
        ajStrAssignC(OEnv, "AQ");
    else if ((residue->pol_rel > 75) && (residue->pol_rel <= 90) && (SEnv == 'C'))
        ajStrAssignC(OEnv, "AR");
    else if ((residue->pol_rel > 90) && (SEnv == 'H'))
        ajStrAssignC(OEnv, "AS");
    else if ((residue->pol_rel > 90) && (SEnv == 'S'))
        ajStrAssignC(OEnv, "AT");
    else if ((residue->pol_rel > 90) && (SEnv == 'C'))
        ajStrAssignC(OEnv, "AU");
    else
    {
        ajStrSetClear(OEnv);
        ajFmtPrintF(flog, "OEnv unassigned for residue %d\n", residue->Idx);

        return 0;
    }

    return 21;
}




/* @func ajResidueEnv9 ********************************************************
**
** Assigns environment based only of side chain accessibility and secondary
** structure.  Assigns environment of "*" as default.
**
** @param [r] residue [const AjPResidue] AJAX Residue
** @param [r] SEnv [char] Secondary structure environment code
** @param [w] OEnv [AjPStr*] Character for the overall environment class
** @param  [w] flog [AjPFile] Log file
** @return [ajint] Number of environments
**
** @release 3.0.0
** @@
******************************************************************************/

ajint ajResidueEnv9(const AjPResidue residue, char SEnv, AjPStr *OEnv,
                    AjPFile flog)
{
    ajStrSetClear(OEnv);

    if (SEnv == '\0')
    {
        ajStrSetClear(OEnv);

        return 0;
    }

    if ((residue->pol_rel <= 5) && (SEnv == 'H'))
        ajStrAssignC(OEnv, "AA");
    else if ((residue->pol_rel <= 5) && (SEnv == 'S'))
        ajStrAssignC(OEnv, "AB");
    else if ((residue->pol_rel <= 5) && (SEnv == 'C'))
        ajStrAssignC(OEnv, "AC");
    else if ((residue->pol_rel > 5) && (residue->pol_rel <= 25) && (SEnv == 'H'))
        ajStrAssignC(OEnv, "AD");
    else if ((residue->pol_rel > 5) && (residue->pol_rel <= 25) && (SEnv == 'S'))
        ajStrAssignC(OEnv, "AE");
    else if ((residue->pol_rel > 5) && (residue->pol_rel <= 25) && (SEnv == 'C'))
        ajStrAssignC(OEnv, "AF");
    else if ((residue->pol_rel > 25) && (SEnv == 'H'))
        ajStrAssignC(OEnv, "AG");
    else if ((residue->pol_rel > 25) && (SEnv == 'S'))
        ajStrAssignC(OEnv, "AH");
    else if ((residue->pol_rel > 25) && (SEnv == 'C'))
        ajStrAssignC(OEnv, "AI");
    else
    {
        ajStrSetClear(OEnv);
        ajFmtPrintF(flog, "OEnv unassigned for residue %d\n", residue->Idx);

        return 0;
    }

    return 9;
}




/* @func ajResidueEnv10 *******************************************************
**
** Assigns environment based on side chain accessibility and secondary
** structure.  Assigns environment of "*" as default.
**
** @param [r] residue [const AjPResidue] AJAX Residue
** @param [r] SEnv [char] Secondary structure environment code
** @param [w] OEnv [AjPStr*] Character for the overall environment class
** @param  [w] flog [AjPFile] Log file
** @return [ajint] Number of environments
**
** @release 3.0.0
** @@
******************************************************************************/

ajint ajResidueEnv10(const AjPResidue residue, char SEnv, AjPStr *OEnv,
                     AjPFile flog)
{
    float BLimit = 5.0F;        /* Upper limit for the relative solvent
                                 * accessible area for a buried residue */
    float PBLimit = 25.0F;      /* Upper limit for the relative solvent
                                 * accessible area for a Partially buried
                                 * residue */

    float HLimit = 5.0F;        /* Upper limit for the fraction polar measure
                                 * of a Hydrophobic residue */
    float MPLimit = 25.0F;      /* Upper limit for the fraction polar measure
                                 * of a Moderately polar residue */
    
    AjPStr BEnv = NULL;

    if (!residue)
    {
        ajWarn("No residue to ajResidueEnv");

        return 0;
    }

    ajStrSetClear(OEnv);
    BEnv = ajStrNew();

    ajFmtPrintF(flog, "R:%c-%d S:%c A:%.2f f:%.2f\n",
                residue->Id1, residue->Idx, residue->eType,
                residue->side_rel, residue->pol_rel);

    /* Assign the basic classes */

    /* Buried, Hydrophobic */
    if ((residue->side_rel <= BLimit) &&
        (residue->pol_rel <= HLimit))
        ajStrAssignC(&BEnv, "B1");
    /* buried, moderately polar */
    else if ((residue->side_rel <= BLimit) &&
             (residue->pol_rel > HLimit) &&
             (residue->pol_rel <= MPLimit))
        ajStrAssignC(&BEnv, "B2");
    /* buried, polar */
    else if ((residue->side_rel <= BLimit) &&
             (residue->pol_rel > MPLimit))
        ajStrAssignC(&BEnv, "B3");
    /* Partially buried, moderately hydrophobic */
    else if ((residue->side_rel > BLimit) &&
             (residue->side_rel <= PBLimit) &&
             (residue->pol_rel <= HLimit))
        ajStrAssignC(&BEnv, "P1");
    /* Partially buried, moderately polar */
    else if ((residue->side_rel > BLimit) &&
             (residue->side_rel <= PBLimit) &&
             (residue->pol_rel > HLimit) &&
             (residue->pol_rel <= MPLimit))
        ajStrAssignC(&BEnv, "P2");
    /* Partially buried, polar */
    else if ((residue->side_rel > BLimit) &&
             (residue->side_rel <= PBLimit) && (residue->pol_rel > MPLimit))
        ajStrAssignC(&BEnv, "P3");
    /* Exposed, moderately polar */
    else if ((residue->side_rel > PBLimit) &&
             (residue->pol_rel <= MPLimit))
        ajStrAssignC(&BEnv, "E1");
    /* Exposed, polar */
    else if ((residue->side_rel > PBLimit) &&
             (residue->pol_rel > MPLimit))
        ajStrAssignC(&BEnv, "E2");
    else
    {
        ajStrSetClear(OEnv);
        ajFmtPrintF(flog, "BEnv unassigned for residue %d\n", residue->Idx);

        ajStrDel(&BEnv);

        return 0;
    }

    /* Assign overall environment class */
    if ((ajStrMatchC(BEnv, "B1")) && (SEnv == 'H'))
        ajStrAssignC(OEnv, "AA");
    else if ((ajStrMatchC(BEnv, "B1")) && (SEnv == 'S'))
        ajStrAssignC(OEnv, "AB");
    else if ((ajStrMatchC(BEnv, "B1")) && (SEnv == 'C'))
        ajStrAssignC(OEnv, "AC");
    else if ((ajStrMatchC(BEnv, "B2")) && (SEnv == 'H'))
        ajStrAssignC(OEnv, "AD");
    else if ((ajStrMatchC(BEnv, "B2")) && (SEnv == 'S'))
        ajStrAssignC(OEnv, "AE");
    else if ((ajStrMatchC(BEnv, "B2")) && (SEnv == 'C'))
        ajStrAssignC(OEnv, "AF");
    else if ((ajStrMatchC(BEnv, "B3")) && (SEnv == 'H'))
        ajStrAssignC(OEnv, "AG");
    else if ((ajStrMatchC(BEnv, "B3")) && (SEnv == 'S'))
        ajStrAssignC(OEnv, "AH");
    else if ((ajStrMatchC(BEnv, "B3")) && (SEnv == 'C'))
        ajStrAssignC(OEnv, "AI");
    else if ((ajStrMatchC(BEnv, "P1")) && (SEnv == 'H'))
        ajStrAssignC(OEnv, "AJ");
    else if ((ajStrMatchC(BEnv, "P1")) && (SEnv == 'S'))
        ajStrAssignC(OEnv, "AK");
    else if ((ajStrMatchC(BEnv, "P1")) && (SEnv == 'C'))
        ajStrAssignC(OEnv, "AL");
    else if ((ajStrMatchC(BEnv, "P2")) && (SEnv == 'H'))
        ajStrAssignC(OEnv, "AM");
    else if ((ajStrMatchC(BEnv, "P2")) && (SEnv == 'S'))
        ajStrAssignC(OEnv, "AN");
    else if ((ajStrMatchC(BEnv, "P2")) && (SEnv == 'C'))
        ajStrAssignC(OEnv, "AO");
    else if ((ajStrMatchC(BEnv, "P3")) && (SEnv == 'H'))
        ajStrAssignC(OEnv, "AP");
    else if ((ajStrMatchC(BEnv, "P3")) && (SEnv == 'S'))
        ajStrAssignC(OEnv, "AQ");
    else if ((ajStrMatchC(BEnv, "P3")) && (SEnv == 'C'))
        ajStrAssignC(OEnv, "AR");
    else if ((ajStrMatchC(BEnv, "E1")) && (SEnv == 'H'))
        ajStrAssignC(OEnv, "AS");
    else if ((ajStrMatchC(BEnv, "E1")) && (SEnv == 'S'))
        ajStrAssignC(OEnv, "AT");
    else if ((ajStrMatchC(BEnv, "E1")) && (SEnv == 'C'))
        ajStrAssignC(OEnv, "AU");
    else if ((ajStrMatchC(BEnv, "E2")) && (SEnv == 'H'))
        ajStrAssignC(OEnv, "AV");
    else if ((ajStrMatchC(BEnv, "E2")) && (SEnv == 'S'))
        ajStrAssignC(OEnv, "AW");
    else if ((ajStrMatchC(BEnv, "E2")) && (SEnv == 'C'))
        ajStrAssignC(OEnv, "AX");
    else
    {
        ajStrSetClear(OEnv);
        ajFmtPrintF(flog, "OEnv unassigned for residue %d\n", residue->Idx);
        ajStrDel(&BEnv);

        return 0;
    }

    ajStrDel(&BEnv);

    return 24;
}




/* @func ajResidueEnv11 *******************************************************
**
** Assigns environment based on side chain accessibility and secondary
** structure.  Assigns environment of "*" as default.
**
** @param [r] residue [const AjPResidue] AJAX Residue
** @param [r] SEnv [char] Secondary structure environment code
** @param [w] OEnv [AjPStr*] Character for the overall environment class
** @param  [w] flog [AjPFile] Log file
** @return [ajint] Number of environments
**
** @release 3.0.0
** @@
******************************************************************************/

ajint ajResidueEnv11(const AjPResidue residue, char SEnv, AjPStr *OEnv,
                     AjPFile flog)
{
    float BLimit = 5.0F;        /* Upper limit for the relative solvent
                                 * accessible area for a buried residue */
    float PBLimit = 25.0F;      /* Upper limit for the relative solvent
                                 * accessible area for a Partially buried
                                 * residue */

    float PolLimit1 = 10.0F;
    float PolLimit2 = 30.0F;
    float PolLimit3 = 50.0F;
    float PolLimit4 = 70.0F;
    float PolLimit5 = 90.0F;

    AjPStr BEnv = NULL;

    if (!residue)
    {
        ajWarn("No residue to ajResidueEnv");

        return 0;
    }

    ajStrSetClear(OEnv);
    BEnv = ajStrNew();

    ajFmtPrintF(flog, "R:%c-%d S:%c A:%.2f f:%.2f\n",
                residue->Id1, residue->Idx, residue->eType,
                residue->side_rel, residue->pol_rel);

    /* Assign the basic classes */

    if ((residue->side_rel <= BLimit) &&
        (residue->pol_rel <= PolLimit1))
        ajStrAssignC(&BEnv, "B1");
    else if ((residue->side_rel <= BLimit) &&
             (residue->pol_rel > PolLimit1) &&
             (residue->pol_rel <= PolLimit2))
        ajStrAssignC(&BEnv, "B2");
    else if ((residue->side_rel <= BLimit) &&
             (residue->pol_rel > PolLimit2) &&
             (residue->pol_rel <= PolLimit3))
        ajStrAssignC(&BEnv, "B3");
    else if ((residue->side_rel <= BLimit) &&
             (residue->pol_rel > PolLimit3) &&
             (residue->pol_rel <= PolLimit4))
        ajStrAssignC(&BEnv, "B4");
    else if ((residue->side_rel <= BLimit) &&
             (residue->pol_rel > PolLimit4) &&
             (residue->pol_rel <= PolLimit5))
        ajStrAssignC(&BEnv, "B5");
    else if ((residue->side_rel <= BLimit) &&
             (residue->pol_rel > PolLimit5))
        ajStrAssignC(&BEnv, "B6");
    else if ((residue->side_rel > BLimit) &&
             (residue->side_rel <= PBLimit))
        ajStrAssignC(&BEnv, "P");
    else if (residue->side_rel > PBLimit)
        ajStrAssignC(&BEnv, "E");
    else
    {
        ajStrSetClear(OEnv);
        ajFmtPrintF(flog, "BEnv unassigned for residue %d\n", residue->Idx);
        ajStrDel(&BEnv);

        return 0;
    }

    /* Assign overall environment class */
    if ((ajStrMatchC(BEnv, "B1")) && (SEnv == 'H'))
        ajStrAssignC(OEnv, "AA");
    else if ((ajStrMatchC(BEnv, "B1")) && (SEnv == 'S'))
        ajStrAssignC(OEnv, "AB");
    else if ((ajStrMatchC(BEnv, "B1")) && (SEnv == 'C'))
        ajStrAssignC(OEnv, "AC");
    else if ((ajStrMatchC(BEnv, "B2")) && (SEnv == 'H'))
        ajStrAssignC(OEnv, "AD");
    else if ((ajStrMatchC(BEnv, "B2")) && (SEnv == 'S'))
        ajStrAssignC(OEnv, "AE");
    else if ((ajStrMatchC(BEnv, "B2")) && (SEnv == 'C'))
        ajStrAssignC(OEnv, "AF");
    else if ((ajStrMatchC(BEnv, "B3")) && (SEnv == 'H'))
        ajStrAssignC(OEnv, "AG");
    else if ((ajStrMatchC(BEnv, "B3")) && (SEnv == 'S'))
        ajStrAssignC(OEnv, "AH");
    else if ((ajStrMatchC(BEnv, "B3")) && (SEnv == 'C'))
        ajStrAssignC(OEnv, "AI");
    else if ((ajStrMatchC(BEnv, "B4")) && (SEnv == 'H'))
        ajStrAssignC(OEnv, "AJ");
    else if ((ajStrMatchC(BEnv, "B4")) && (SEnv == 'S'))
        ajStrAssignC(OEnv, "AK");
    else if ((ajStrMatchC(BEnv, "B4")) && (SEnv == 'C'))
        ajStrAssignC(OEnv, "AL");
    else if ((ajStrMatchC(BEnv, "B5")) && (SEnv == 'H'))
        ajStrAssignC(OEnv, "AM");
    else if ((ajStrMatchC(BEnv, "B5")) && (SEnv == 'S'))
        ajStrAssignC(OEnv, "AN");
    else if ((ajStrMatchC(BEnv, "B5")) && (SEnv == 'C'))
        ajStrAssignC(OEnv, "AO");
    else if ((ajStrMatchC(BEnv, "B6")) && (SEnv == 'H'))
        ajStrAssignC(OEnv, "AP");
    else if ((ajStrMatchC(BEnv, "B6")) && (SEnv == 'S'))
        ajStrAssignC(OEnv, "AQ");
    else if ((ajStrMatchC(BEnv, "B6")) && (SEnv == 'C'))
        ajStrAssignC(OEnv, "AR");
    else if ((ajStrMatchC(BEnv, "P")) && (SEnv == 'H'))
        ajStrAssignC(OEnv, "AS");
    else if ((ajStrMatchC(BEnv, "P")) && (SEnv == 'S'))
        ajStrAssignC(OEnv, "AT");
    else if ((ajStrMatchC(BEnv, "P")) && (SEnv == 'C'))
        ajStrAssignC(OEnv, "AU");
    else if ((ajStrMatchC(BEnv, "E")) && (SEnv == 'H'))
        ajStrAssignC(OEnv, "AV");
    else if ((ajStrMatchC(BEnv, "E")) && (SEnv == 'S'))
        ajStrAssignC(OEnv, "AW");
    else if ((ajStrMatchC(BEnv, "E")) && (SEnv == 'C'))
        ajStrAssignC(OEnv, "AX");
    else
    {
        ajStrSetClear(OEnv);
        ajFmtPrintF(flog, "OEnv unassigned for residue %d\n", residue->Idx);

        ajStrDel(&BEnv);
        return 0;
    }

    ajStrDel(&BEnv);

    return 24;
}




/* @func ajResidueEnv12 *******************************************************
**
** Assigns environment based on side chain accessibility and secondary
** structure.  Assigns environment of "*" as default.
**
** @param [r] residue [const AjPResidue] AJAX Residue
** @param [r] SEnv [char] Secondary structure environment code
** @param [w] OEnv [AjPStr*] Character for the overall environment class
** @param  [w] flog [AjPFile] Log file
** @return [ajint] Number of environments
**
** @release 3.0.0
** @@
******************************************************************************/

ajint ajResidueEnv12(const AjPResidue residue, char SEnv, AjPStr *OEnv,
                     AjPFile flog)
{
    float BLimit = 5.0F;        /* Upper limit for the relative solvent
                                 * accessible area for a buried residue */
    float PBLimit = 25.0F;      /* Upper limit for the relative solvent
                                 * accessible area for a Partially buried
                                 * residue */

    float PolLimit1 = 10.0F;
    float PolLimit2 = 30.0F;
    float PolLimit3 = 50.0F;
    float PolLimit4 = 70.0F;
    float PolLimit5 = 90.0F;

    AjPStr BEnv = NULL;

    if (!residue)
    {
        ajWarn("No residue to ajResidueEnv");

        return 0;
    }

    ajStrSetClear(OEnv);
    BEnv = ajStrNew();

    ajFmtPrintF(flog, "R:%c-%d S:%c A:%.2f f:%.2f\n", residue->Id1, residue->Idx,
                residue->eType, residue->side_rel, residue->pol_rel);

    /* Assign the basic classes */

    if ((residue->side_rel <= BLimit))
        ajStrAssignC(&BEnv, "B");
    else if ((residue->side_rel > BLimit) &&
             (residue->side_rel <= PBLimit) &&
             (residue->pol_rel <= PolLimit1))
        ajStrAssignC(&BEnv, "P1");
    else if ((residue->side_rel > BLimit) &&
             (residue->side_rel <= PBLimit) &&
             (residue->pol_rel > PolLimit1) &&
             (residue->pol_rel <= PolLimit2))
        ajStrAssignC(&BEnv, "P2");
    else if ((residue->side_rel > BLimit) &&
             (residue->side_rel <= PBLimit) &&
             (residue->pol_rel > PolLimit2) &&
             (residue->pol_rel <= PolLimit3))
        ajStrAssignC(&BEnv, "P3");
    else if ((residue->side_rel > BLimit) &&
             (residue->side_rel <= PBLimit) &&
             (residue->pol_rel > PolLimit3) &&
             (residue->pol_rel <= PolLimit4))
        ajStrAssignC(&BEnv, "P4");
    else if ((residue->side_rel > BLimit) &&
             (residue->side_rel <= PBLimit) &&
             (residue->pol_rel > PolLimit4) &&
             (residue->pol_rel <= PolLimit5))
        ajStrAssignC(&BEnv, "P5");
    else if ((residue->side_rel > BLimit) &&
             (residue->side_rel <= PBLimit) &&
             (residue->pol_rel > PolLimit5))
        ajStrAssignC(&BEnv, "P6");
    else if (residue->side_rel > PBLimit)
        ajStrAssignC(&BEnv, "E");
    else
    {
        ajStrSetClear(OEnv);
        ajFmtPrintF(flog, "BEnv unassigned for residue %d\n", residue->Idx);
        ajStrDel(&BEnv);

        return 0;
    }

    /* Assign overall environment class */
    if ((ajStrMatchC(BEnv, "B")) && (SEnv == 'H'))
        ajStrAssignC(OEnv, "AA");
    else if ((ajStrMatchC(BEnv, "B")) && (SEnv == 'S'))
        ajStrAssignC(OEnv, "AB");
    else if ((ajStrMatchC(BEnv, "B")) && (SEnv == 'C'))
        ajStrAssignC(OEnv, "AC");
    else if ((ajStrMatchC(BEnv, "P1")) && (SEnv == 'H'))
        ajStrAssignC(OEnv, "AD");
    else if ((ajStrMatchC(BEnv, "P1")) && (SEnv == 'S'))
        ajStrAssignC(OEnv, "AE");
    else if ((ajStrMatchC(BEnv, "P1")) && (SEnv == 'C'))
        ajStrAssignC(OEnv, "AF");
    else if ((ajStrMatchC(BEnv, "P2")) && (SEnv == 'H'))
        ajStrAssignC(OEnv, "AG");
    else if ((ajStrMatchC(BEnv, "P2")) && (SEnv == 'S'))
        ajStrAssignC(OEnv, "AH");
    else if ((ajStrMatchC(BEnv, "P2")) && (SEnv == 'C'))
        ajStrAssignC(OEnv, "AI");
    else if ((ajStrMatchC(BEnv, "P3")) && (SEnv == 'H'))
        ajStrAssignC(OEnv, "AJ");
    else if ((ajStrMatchC(BEnv, "P3")) && (SEnv == 'S'))
        ajStrAssignC(OEnv, "AK");
    else if ((ajStrMatchC(BEnv, "P3")) && (SEnv == 'C'))
        ajStrAssignC(OEnv, "AL");
    else if ((ajStrMatchC(BEnv, "P4")) && (SEnv == 'H'))
        ajStrAssignC(OEnv, "AM");
    else if ((ajStrMatchC(BEnv, "P4")) && (SEnv == 'S'))
        ajStrAssignC(OEnv, "AN");
    else if ((ajStrMatchC(BEnv, "P4")) && (SEnv == 'C'))
        ajStrAssignC(OEnv, "AO");
    else if ((ajStrMatchC(BEnv, "P5")) && (SEnv == 'H'))
        ajStrAssignC(OEnv, "AP");
    else if ((ajStrMatchC(BEnv, "P5")) && (SEnv == 'S'))
        ajStrAssignC(OEnv, "AQ");
    else if ((ajStrMatchC(BEnv, "P5")) && (SEnv == 'C'))
        ajStrAssignC(OEnv, "AR");
    else if ((ajStrMatchC(BEnv, "P6")) && (SEnv == 'H'))
        ajStrAssignC(OEnv, "AS");
    else if ((ajStrMatchC(BEnv, "P6")) && (SEnv == 'S'))
        ajStrAssignC(OEnv, "AT");
    else if ((ajStrMatchC(BEnv, "P6")) && (SEnv == 'C'))
        ajStrAssignC(OEnv, "AU");
    else if ((ajStrMatchC(BEnv, "E")) && (SEnv == 'H'))
        ajStrAssignC(OEnv, "AV");
    else if ((ajStrMatchC(BEnv, "E")) && (SEnv == 'S'))
        ajStrAssignC(OEnv, "AW");
    else if ((ajStrMatchC(BEnv, "E")) && (SEnv == 'C'))
        ajStrAssignC(OEnv, "AX");
    else
    {
        ajStrSetClear(OEnv);
        ajFmtPrintF(flog, "OEnv unassigned for residue %d\n", residue->Idx);
        ajStrDel(&BEnv);

        return 0;
    }

    ajStrDel(&BEnv);

    return 24;
}




/* @func ajResidueEnv13 *******************************************************
**
** Assigns environment based on side chain accessibility and secondary
** structure.  Assigns environment of "*" as default.
**
** @param [r] residue [const AjPResidue] AJAX Residue
** @param [r] SEnv [char] Secondary structure environment code
** @param [w] OEnv [AjPStr*] Character for the overall environment class
** @param  [w] flog [AjPFile] Log file
** @return [ajint] Number of environments
**
** @release 3.0.0
** @@
******************************************************************************/

ajint ajResidueEnv13(const AjPResidue residue, char SEnv, AjPStr *OEnv,
                     AjPFile flog)
{
    float BLimit = 5.0F;        /* Upper limit for the relative solvent
                                 * accessible area for a buried residue */
    float PBLimit = 25.0F;      /* Upper limit for the relative solvent
                                 * accessible area for a Partially buried
                                 * residue */

    float PolLimit1 = 10.0F;
    float PolLimit2 = 30.0F;
    float PolLimit3 = 50.0F;
    float PolLimit4 = 70.0F;
    float PolLimit5 = 90.0F;

    AjPStr BEnv = NULL;

    if (!residue)
    {
        ajWarn("No residue to ajResidueEnv");

        return 0;
    }

    ajStrSetClear(OEnv);
    BEnv = ajStrNew();

    ajFmtPrintF(flog, "R:%c-%d S:%c A:%.2f f:%.2f\n",
                residue->Id1, residue->Idx, residue->eType,
                residue->side_rel, residue->pol_rel);

    /* Assign the basic classes */
    if ((residue->side_rel <= BLimit))
        ajStrAssignC(&BEnv, "B");
    else if ((residue->side_rel > BLimit) &&
             (residue->side_rel <= PBLimit))
        ajStrAssignC(&BEnv, "P");
    else if ((residue->side_rel > PBLimit) &&
             (residue->pol_rel <= PolLimit1))
        ajStrAssignC(&BEnv, "E1");
    else if ((residue->side_rel > PBLimit) &&
             (residue->pol_rel > PolLimit1) &&
             (residue->pol_rel <= PolLimit2))
        ajStrAssignC(&BEnv, "E2");
    else if ((residue->side_rel > PBLimit) &&
             (residue->pol_rel > PolLimit2) &&
             (residue->pol_rel <= PolLimit3))
        ajStrAssignC(&BEnv, "E3");
    else if ((residue->side_rel > PBLimit) &&
             (residue->pol_rel > PolLimit3) &&
             (residue->pol_rel <= PolLimit4))
        ajStrAssignC(&BEnv, "E4");
    else if ((residue->side_rel > PBLimit) &&
             (residue->pol_rel > PolLimit4) &&
             (residue->pol_rel <= PolLimit5))
        ajStrAssignC(&BEnv, "E5");
    else if ((residue->side_rel > PBLimit) &&
             (residue->pol_rel > PolLimit5))
        ajStrAssignC(&BEnv, "E6");
    else
    {
        ajStrSetClear(OEnv);
        ajFmtPrintF(flog, "BEnv unassigned for residue %d\n", residue->Idx);
        ajStrDel(&BEnv);

        return 0;
    }

    /* Assign overall environment class */
    if ((ajStrMatchC(BEnv, "B")) && (SEnv == 'H'))
        ajStrAssignC(OEnv, "AA");
    else if ((ajStrMatchC(BEnv, "B")) && (SEnv == 'S'))
        ajStrAssignC(OEnv, "AB");
    else if ((ajStrMatchC(BEnv, "B")) && (SEnv == 'C'))
        ajStrAssignC(OEnv, "AC");
    else if ((ajStrMatchC(BEnv, "P")) && (SEnv == 'H'))
        ajStrAssignC(OEnv, "AD");
    else if ((ajStrMatchC(BEnv, "P")) && (SEnv == 'S'))
        ajStrAssignC(OEnv, "AE");
    else if ((ajStrMatchC(BEnv, "P")) && (SEnv == 'C'))
        ajStrAssignC(OEnv, "AF");
    else if ((ajStrMatchC(BEnv, "E1")) && (SEnv == 'H'))
        ajStrAssignC(OEnv, "AG");
    else if ((ajStrMatchC(BEnv, "E1")) && (SEnv == 'S'))
        ajStrAssignC(OEnv, "AH");
    else if ((ajStrMatchC(BEnv, "E1")) && (SEnv == 'C'))
        ajStrAssignC(OEnv, "AI");
    else if ((ajStrMatchC(BEnv, "E2")) && (SEnv == 'H'))
        ajStrAssignC(OEnv, "AJ");
    else if ((ajStrMatchC(BEnv, "E2")) && (SEnv == 'S'))
        ajStrAssignC(OEnv, "AK");
    else if ((ajStrMatchC(BEnv, "E2")) && (SEnv == 'C'))
        ajStrAssignC(OEnv, "AL");
    else if ((ajStrMatchC(BEnv, "E3")) && (SEnv == 'H'))
        ajStrAssignC(OEnv, "AM");
    else if ((ajStrMatchC(BEnv, "E3")) && (SEnv == 'S'))
        ajStrAssignC(OEnv, "AN");
    else if ((ajStrMatchC(BEnv, "E3")) && (SEnv == 'C'))
        ajStrAssignC(OEnv, "AO");
    else if ((ajStrMatchC(BEnv, "E4")) && (SEnv == 'H'))
        ajStrAssignC(OEnv, "AP");
    else if ((ajStrMatchC(BEnv, "E4")) && (SEnv == 'S'))
        ajStrAssignC(OEnv, "AQ");
    else if ((ajStrMatchC(BEnv, "E4")) && (SEnv == 'C'))
        ajStrAssignC(OEnv, "AR");
    else if ((ajStrMatchC(BEnv, "E5")) && (SEnv == 'H'))
        ajStrAssignC(OEnv, "AS");
    else if ((ajStrMatchC(BEnv, "E5")) && (SEnv == 'S'))
        ajStrAssignC(OEnv, "AT");
    else if ((ajStrMatchC(BEnv, "E5")) && (SEnv == 'C'))
        ajStrAssignC(OEnv, "AU");
    else if ((ajStrMatchC(BEnv, "E6")) && (SEnv == 'H'))
        ajStrAssignC(OEnv, "AV");
    else if ((ajStrMatchC(BEnv, "E6")) && (SEnv == 'S'))
        ajStrAssignC(OEnv, "AW");
    else if ((ajStrMatchC(BEnv, "E6")) && (SEnv == 'C'))
        ajStrAssignC(OEnv, "AX");
    else
    {
        ajStrSetClear(OEnv);
        ajFmtPrintF(flog, "OEnv unassigned for residue %d\n", residue->Idx);
        ajStrDel(&BEnv);

        return 0;
    }

    ajStrDel(&BEnv);

    return 24;
}




/* @func ajResidueEnv14 *******************************************************
**
** Assigns environment based on side chain accessibility and secondary
** structure.  Assigns environment of "*" as default.
**
** @param [r] residue [const AjPResidue] AJAX Residue
** @param [r] SEnv [char] Secondary structure environment code
** @param [w] OEnv [AjPStr*] Character for the overall environment class
** @param  [w] flog [AjPFile] Log file
** @return [ajint] Number of environments
**
** @release 3.0.0
** @@
******************************************************************************/

ajint ajResidueEnv14(const AjPResidue residue, char SEnv, AjPStr *OEnv,
                     AjPFile flog)
{
    float PBLimit = 25.0F;      /* Upper limit for the relative solvent
                                 * accessible area for a Partially buried
                                 * residue */

    float PolLimit1 = 5.0F;
    float PolLimit2 = 15.0F;
    float PolLimit3 = 25.0F;
    float PolLimit4 = 40.0F;
    float PolLimit5 = 60.0F;
    float PolLimit6 = 80.0F;

    AjPStr BEnv = NULL;

    if (!residue)
    {
        ajWarn("No residue to ajResidueEnv");

        return 0;
    }

    ajStrSetClear(OEnv);
    BEnv = ajStrNew();

    ajFmtPrintF(flog, "R:%c-%d S:%c A:%.2f f:%.2f\n",
                residue->Id1, residue->Idx, residue->eType,
                residue->side_rel, residue->pol_rel);

    if ((residue->side_rel <= PBLimit) &&
        (residue->pol_rel <= PolLimit1))
        ajStrAssignC(&BEnv, "B1");
    else if ((residue->side_rel <= PBLimit) &&
             (residue->pol_rel > PolLimit1) &&
             (residue->pol_rel <= PolLimit2))
        ajStrAssignC(&BEnv, "B2");
    else if ((residue->side_rel <= PBLimit) &&
             (residue->pol_rel > PolLimit2) &&
             (residue->pol_rel <= PolLimit3))
        ajStrAssignC(&BEnv, "B3");
    else if ((residue->side_rel <= PBLimit) &&
             (residue->pol_rel > PolLimit3) &&
             (residue->pol_rel <= PolLimit4))
        ajStrAssignC(&BEnv, "B4");
    else if ((residue->side_rel <= PBLimit) &&
             (residue->pol_rel > PolLimit4) &&
             (residue->pol_rel <= PolLimit5))
        ajStrAssignC(&BEnv, "B5");
    else if ((residue->side_rel <= PBLimit) &&
             (residue->pol_rel > PolLimit5) &&
             (residue->pol_rel <= PolLimit6))
        ajStrAssignC(&BEnv, "B6");
    else if ((residue->side_rel <= PBLimit) &&
             (residue->pol_rel > PolLimit6))
        ajStrAssignC(&BEnv, "B7");
    else if ((residue->side_rel > PBLimit))
        ajStrAssignC(&BEnv, "E");
    else
    {
        ajStrSetClear(OEnv);
        ajFmtPrintF(flog, "BEnv unassigned for residue %d\n", residue->Idx);
        ajStrDel(&BEnv);

        return 0;
    }

    /* Assign overall environment class */
    if ((ajStrMatchC(BEnv, "B1")) && (SEnv == 'H'))
        ajStrAssignC(OEnv, "AA");
    else if ((ajStrMatchC(BEnv, "B1")) && (SEnv == 'S'))
        ajStrAssignC(OEnv, "AB");
    else if ((ajStrMatchC(BEnv, "B1")) && (SEnv == 'C'))
        ajStrAssignC(OEnv, "AC");
    else if ((ajStrMatchC(BEnv, "B2")) && (SEnv == 'H'))
        ajStrAssignC(OEnv, "AD");
    else if ((ajStrMatchC(BEnv, "B2")) && (SEnv == 'S'))
        ajStrAssignC(OEnv, "AE");
    else if ((ajStrMatchC(BEnv, "B2")) && (SEnv == 'C'))
        ajStrAssignC(OEnv, "AF");
    else if ((ajStrMatchC(BEnv, "B3")) && (SEnv == 'H'))
        ajStrAssignC(OEnv, "AG");
    else if ((ajStrMatchC(BEnv, "B3")) && (SEnv == 'S'))
        ajStrAssignC(OEnv, "AH");
    else if ((ajStrMatchC(BEnv, "B3")) && (SEnv == 'C'))
        ajStrAssignC(OEnv, "AI");
    else if ((ajStrMatchC(BEnv, "B4")) && (SEnv == 'H'))
        ajStrAssignC(OEnv, "AJ");
    else if ((ajStrMatchC(BEnv, "B4")) && (SEnv == 'S'))
        ajStrAssignC(OEnv, "AK");
    else if ((ajStrMatchC(BEnv, "B4")) && (SEnv == 'C'))
        ajStrAssignC(OEnv, "AL");
    else if ((ajStrMatchC(BEnv, "B5")) && (SEnv == 'H'))
        ajStrAssignC(OEnv, "AM");
    else if ((ajStrMatchC(BEnv, "B5")) && (SEnv == 'S'))
        ajStrAssignC(OEnv, "AN");
    else if ((ajStrMatchC(BEnv, "B5")) && (SEnv == 'C'))
        ajStrAssignC(OEnv, "AO");
    else if ((ajStrMatchC(BEnv, "B6")) && (SEnv == 'H'))
        ajStrAssignC(OEnv, "AP");
    else if ((ajStrMatchC(BEnv, "B6")) && (SEnv == 'S'))
        ajStrAssignC(OEnv, "AQ");
    else if ((ajStrMatchC(BEnv, "B6")) && (SEnv == 'C'))
        ajStrAssignC(OEnv, "AR");
    else if ((ajStrMatchC(BEnv, "B7")) && (SEnv == 'H'))
        ajStrAssignC(OEnv, "AS");
    else if ((ajStrMatchC(BEnv, "B7")) && (SEnv == 'S'))
        ajStrAssignC(OEnv, "AT");
    else if ((ajStrMatchC(BEnv, "B7")) && (SEnv == 'C'))
        ajStrAssignC(OEnv, "AU");
    else if ((ajStrMatchC(BEnv, "E")) && (SEnv == 'H'))
        ajStrAssignC(OEnv, "AV");
    else if ((ajStrMatchC(BEnv, "E")) && (SEnv == 'S'))
        ajStrAssignC(OEnv, "AW");
    else if ((ajStrMatchC(BEnv, "E")) && (SEnv == 'C'))
        ajStrAssignC(OEnv, "AX");
    else
    {
        ajStrSetClear(OEnv);
        ajFmtPrintF(flog, "OEnv unassigned for residue %d\n", residue->Idx);
        ajStrDel(&BEnv);

        return 0;
    }

    ajStrDel(&BEnv);

    return 24;
}




/* @func ajResidueEnv15 *******************************************************
**
** Assigns environment based on side chain accessibility and secondary
** structure.   Assigns environment of "*" as default.
**
** @param [r] residue [const AjPResidue] AJAX Residue
** @param [r] SEnv [char] Secondary structure environment code
** @param [w] OEnv [AjPStr*] Character for the overall environment class
** @param  [w] flog [AjPFile] Log file
** @return [ajint] Number of environments
**
** @release 3.0.0
** @@
******************************************************************************/

ajint ajResidueEnv15(const AjPResidue residue, char SEnv, AjPStr *OEnv,
                     AjPFile flog)
{
    float PBLimit = 25.0F;      /* Upper limit for the relative solvent
                                 * accessible area for a Partially buried
                                 * residue */

    float PolLimit1 = 5.0F;
    float PolLimit2 = 15.0F;
    float PolLimit3 = 25.0F;
    float PolLimit4 = 35.0F;
    float PolLimit5 = 45.0F;
    float PolLimit6 = 75.0F;

    AjPStr BEnv = NULL;

    if (!residue)
    {
        ajWarn("No residue to ajResidueEnv");

        return 0;
    }

    ajStrSetClear(OEnv);
    BEnv = ajStrNew();

    ajFmtPrintF(flog, "R:%c-%d S:%c A:%.2f f:%.2f\n",
                residue->Id1, residue->Idx, residue->eType,
                residue->side_rel, residue->pol_rel);

    if ((residue->side_rel <= PBLimit) && (residue->pol_rel <= PolLimit1))
        ajStrAssignC(&BEnv, "B1");
    else if ((residue->side_rel <= PBLimit) &&
             (residue->pol_rel > PolLimit1) &&
             (residue->pol_rel <= PolLimit2))
        ajStrAssignC(&BEnv, "B2");
    else if ((residue->side_rel <= PBLimit) &&
             (residue->pol_rel > PolLimit2) &&
             (residue->pol_rel <= PolLimit3))
        ajStrAssignC(&BEnv, "B3");
    else if ((residue->side_rel <= PBLimit) &&
             (residue->pol_rel > PolLimit3) &&
             (residue->pol_rel <= PolLimit4))
        ajStrAssignC(&BEnv, "B4");
    else if ((residue->side_rel <= PBLimit) &&
             (residue->pol_rel > PolLimit4) &&
             (residue->pol_rel <= PolLimit5))
        ajStrAssignC(&BEnv, "B5");
    else if ((residue->side_rel <= PBLimit) &&
             (residue->pol_rel > PolLimit5) &&
             (residue->pol_rel <= PolLimit6))
        ajStrAssignC(&BEnv, "B6");
    else if ((residue->side_rel <= PBLimit) &&
             (residue->pol_rel > PolLimit6))
        ajStrAssignC(&BEnv, "B7");
    else if ((residue->side_rel > PBLimit))
        ajStrAssignC(&BEnv, "E");
    else
    {
        ajStrSetClear(OEnv);
        ajFmtPrintF(flog, "BEnv unassigned for residue %d\n", residue->Idx);
        ajStrDel(&BEnv);

        return 0;
    }

    /* Assign overall environment class */
    if ((ajStrMatchC(BEnv, "B1")) && (SEnv == 'H'))
        ajStrAssignC(OEnv, "AA");
    else if ((ajStrMatchC(BEnv, "B1")) && (SEnv == 'S'))
        ajStrAssignC(OEnv, "AB");
    else if ((ajStrMatchC(BEnv, "B1")) && (SEnv == 'C'))
        ajStrAssignC(OEnv, "AC");
    else if ((ajStrMatchC(BEnv, "B2")) && (SEnv == 'H'))
        ajStrAssignC(OEnv, "AD");
    else if ((ajStrMatchC(BEnv, "B2")) && (SEnv == 'S'))
        ajStrAssignC(OEnv, "AE");
    else if ((ajStrMatchC(BEnv, "B2")) && (SEnv == 'C'))
        ajStrAssignC(OEnv, "AF");
    else if ((ajStrMatchC(BEnv, "B3")) && (SEnv == 'H'))
        ajStrAssignC(OEnv, "AG");
    else if ((ajStrMatchC(BEnv, "B3")) && (SEnv == 'S'))
        ajStrAssignC(OEnv, "AH");
    else if ((ajStrMatchC(BEnv, "B3")) && (SEnv == 'C'))
        ajStrAssignC(OEnv, "AI");
    else if ((ajStrMatchC(BEnv, "B4")) && (SEnv == 'H'))
        ajStrAssignC(OEnv, "AJ");
    else if ((ajStrMatchC(BEnv, "B4")) && (SEnv == 'S'))
        ajStrAssignC(OEnv, "AK");
    else if ((ajStrMatchC(BEnv, "B4")) && (SEnv == 'C'))
        ajStrAssignC(OEnv, "AL");
    else if ((ajStrMatchC(BEnv, "B5")) && (SEnv == 'H'))
        ajStrAssignC(OEnv, "AM");
    else if ((ajStrMatchC(BEnv, "B5")) && (SEnv == 'S'))
        ajStrAssignC(OEnv, "AN");
    else if ((ajStrMatchC(BEnv, "B5")) && (SEnv == 'C'))
        ajStrAssignC(OEnv, "AO");
    else if ((ajStrMatchC(BEnv, "B6")) && (SEnv == 'H'))
        ajStrAssignC(OEnv, "AP");
    else if ((ajStrMatchC(BEnv, "B6")) && (SEnv == 'S'))
        ajStrAssignC(OEnv, "AQ");
    else if ((ajStrMatchC(BEnv, "B6")) && (SEnv == 'C'))
        ajStrAssignC(OEnv, "AR");
    else if ((ajStrMatchC(BEnv, "B7")) && (SEnv == 'H'))
        ajStrAssignC(OEnv, "AS");
    else if ((ajStrMatchC(BEnv, "B7")) && (SEnv == 'S'))
        ajStrAssignC(OEnv, "AT");
    else if ((ajStrMatchC(BEnv, "B7")) && (SEnv == 'C'))
        ajStrAssignC(OEnv, "AU");
    else if ((ajStrMatchC(BEnv, "E")) && (SEnv == 'H'))
        ajStrAssignC(OEnv, "AV");
    else if ((ajStrMatchC(BEnv, "E")) && (SEnv == 'S'))
        ajStrAssignC(OEnv, "AW");
    else if ((ajStrMatchC(BEnv, "E")) && (SEnv == 'C'))
        ajStrAssignC(OEnv, "AX");
    else
    {
        ajStrSetClear(OEnv);
        ajFmtPrintF(flog, "OEnv unassigned for residue %d\n", residue->Idx);
        ajStrDel(&BEnv);

        return 0;
    }

    ajStrDel(&BEnv);

    return 24;
}




/* @func ajResidueEnv16 *******************************************************
**
** Assigns environment based on side chain accessibility and secondary
** structure.  Assigns environment of "*" as default.
**
** @param [r] residue [const AjPResidue] AJAX Residue
** @param [r] SEnv [char] Secondary structure environment code
** @param [w] OEnv [AjPStr*] Character for the overall environment class
** @param  [w] flog [AjPFile] Log file
** @return [ajint] Number of environments
**
** @release 3.0.0
** @@
******************************************************************************/

ajint ajResidueEnv16(const AjPResidue residue, char SEnv, AjPStr *OEnv,
                     AjPFile flog)
{
    float BLimit = 5.0F;        /* Upper limit for the relative solvent
                                 * accessible area for a buried residue */

    float PolLimit1 = 5.0F;
    float PolLimit2 = 15.0F;
    float PolLimit3 = 25.0F;
    float PolLimit4 = 35.0F;
    float PolLimit5 = 45.0F;
    float PolLimit6 = 75.0F;

    AjPStr BEnv = NULL;

    if (!residue)
    {
        ajWarn("No residue to ajResidueEnv");

        return 0;
    }

    ajStrSetClear(OEnv);
    BEnv = ajStrNew();

    ajFmtPrintF(flog, "R:%c-%d S:%c A:%.2f f:%.2f\n",
                residue->Id1, residue->Idx,
                residue->eType, residue->side_rel, residue->pol_rel);

    if ((residue->side_rel <= BLimit) &&
        (residue->pol_rel <= PolLimit1))
        ajStrAssignC(&BEnv, "B1");
    else if ((residue->side_rel <= BLimit) &&
             (residue->pol_rel > PolLimit1) &&
             (residue->pol_rel <= PolLimit2))
        ajStrAssignC(&BEnv, "B2");
    else if ((residue->side_rel <= BLimit) &&
             (residue->pol_rel > PolLimit2) &&
             (residue->pol_rel <= PolLimit3))
        ajStrAssignC(&BEnv, "B3");
    else if ((residue->side_rel <= BLimit) &&
             (residue->pol_rel > PolLimit3) &&
             (residue->pol_rel <= PolLimit4))
        ajStrAssignC(&BEnv, "B4");
    else if ((residue->side_rel <= BLimit) &&
             (residue->pol_rel > PolLimit4) &&
             (residue->pol_rel <= PolLimit5))
        ajStrAssignC(&BEnv, "B5");
    else if ((residue->side_rel <= BLimit) &&
             (residue->pol_rel > PolLimit5) &&
             (residue->pol_rel <= PolLimit6))
        ajStrAssignC(&BEnv, "B6");
    else if ((residue->side_rel <= BLimit) &&
             (residue->pol_rel > PolLimit6))
        ajStrAssignC(&BEnv, "B7");
    else if ((residue->side_rel > BLimit))
        ajStrAssignC(&BEnv, "E");
    else
    {
        ajStrSetClear(OEnv);
        ajFmtPrintF(flog, "BEnv unassigned for residue %d\n", residue->Idx);
        ajStrDel(&BEnv);

        return 0;
    }

    /* Assign overall environment class */
    if ((ajStrMatchC(BEnv, "B1")) && (SEnv == 'H'))
        ajStrAssignC(OEnv, "AA");
    else if ((ajStrMatchC(BEnv, "B1")) && (SEnv == 'S'))
        ajStrAssignC(OEnv, "AB");
    else if ((ajStrMatchC(BEnv, "B1")) && (SEnv == 'C'))
        ajStrAssignC(OEnv, "AC");
    else if ((ajStrMatchC(BEnv, "B2")) && (SEnv == 'H'))
        ajStrAssignC(OEnv, "AD");
    else if ((ajStrMatchC(BEnv, "B2")) && (SEnv == 'S'))
        ajStrAssignC(OEnv, "AE");
    else if ((ajStrMatchC(BEnv, "B2")) && (SEnv == 'C'))
        ajStrAssignC(OEnv, "AF");
    else if ((ajStrMatchC(BEnv, "B3")) && (SEnv == 'H'))
        ajStrAssignC(OEnv, "AG");
    else if ((ajStrMatchC(BEnv, "B3")) && (SEnv == 'S'))
        ajStrAssignC(OEnv, "AH");
    else if ((ajStrMatchC(BEnv, "B3")) && (SEnv == 'C'))
        ajStrAssignC(OEnv, "AI");
    else if ((ajStrMatchC(BEnv, "B4")) && (SEnv == 'H'))
        ajStrAssignC(OEnv, "AJ");
    else if ((ajStrMatchC(BEnv, "B4")) && (SEnv == 'S'))
        ajStrAssignC(OEnv, "AK");
    else if ((ajStrMatchC(BEnv, "B4")) && (SEnv == 'C'))
        ajStrAssignC(OEnv, "AL");
    else if ((ajStrMatchC(BEnv, "B5")) && (SEnv == 'H'))
        ajStrAssignC(OEnv, "AM");
    else if ((ajStrMatchC(BEnv, "B5")) && (SEnv == 'S'))
        ajStrAssignC(OEnv, "AN");
    else if ((ajStrMatchC(BEnv, "B5")) && (SEnv == 'C'))
        ajStrAssignC(OEnv, "AO");
    else if ((ajStrMatchC(BEnv, "B6")) && (SEnv == 'H'))
        ajStrAssignC(OEnv, "AP");
    else if ((ajStrMatchC(BEnv, "B6")) && (SEnv == 'S'))
        ajStrAssignC(OEnv, "AQ");
    else if ((ajStrMatchC(BEnv, "B6")) && (SEnv == 'C'))
        ajStrAssignC(OEnv, "AR");
    else if ((ajStrMatchC(BEnv, "B7")) && (SEnv == 'H'))
        ajStrAssignC(OEnv, "AS");
    else if ((ajStrMatchC(BEnv, "B7")) && (SEnv == 'S'))
        ajStrAssignC(OEnv, "AT");
    else if ((ajStrMatchC(BEnv, "B7")) && (SEnv == 'C'))
        ajStrAssignC(OEnv, "AU");
    else if ((ajStrMatchC(BEnv, "E")) && (SEnv == 'H'))
        ajStrAssignC(OEnv, "AV");
    else if ((ajStrMatchC(BEnv, "E")) && (SEnv == 'S'))
        ajStrAssignC(OEnv, "AW");
    else if ((ajStrMatchC(BEnv, "E")) && (SEnv == 'C'))
        ajStrAssignC(OEnv, "AX");
    else
    {
        ajStrSetClear(OEnv);
        ajFmtPrintF(flog, "OEnv unassigned for residue %d\n", residue->Idx);
        ajStrDel(&BEnv);

        return 0;
    }

    ajStrDel(&BEnv);

    return 24;
}




/* @func ajPdbGetEStrideType **************************************************
**
** Reads a Pdb object and writes a string with the secondary structure. The
** string that is written is the same length as the full-length chain
** (regardless of whether coordinates for a residue were available or not)
** therefore it can be indexed into using residue numbers.  The string is
** allocated if necessary.  If secondary structure assignment was not available
** for a residue a '.' is given in the string.
**
** @param [r] pdb [const AjPPdb] AJAX PDB object
** @param [r] chn [ajuint]   Chain number
** @param [w] EStrideType [AjPStr *] String to hold secondary structure
**
** @return [ajint] Length (residues) of array that was written or -1 (for
**                 an error)
**
** @release 2.9.0
** @@
******************************************************************************/

ajint ajPdbGetEStrideType(const AjPPdb pdb, ajuint chn, AjPStr *EStrideType)
{
    AjPResidue residue = NULL;
    AjIList iter = NULL;
    ajuint idx = 0U;

    if (!pdb || !EStrideType || (chn < 1))
    {
        ajWarn("Bad args passed to ajPdbGetEStrideType");

        return -1;
    }

    if (chn > pdb->Nchn)
    {
        ajWarn("chn arg in ajPdbGetEStrideType exceeds no. chains");

        return -1;
    }
    else
        idx = chn - 1U;

    /* +1 is for the NULL */
    if (!*EStrideType)
        *EStrideType = ajStrNewRes(pdb->Chains[idx]->Nres + 1U);
    else
    {
        ajStrDel(EStrideType);
        *EStrideType = ajStrNewRes(pdb->Chains[idx]->Nres + 1U);
    }

    /* Set all positions to . */
    ajStrAppendCountK(EStrideType, '.', pdb->Chains[idx]->Nres);

    iter = ajListIterNewread(pdb->Chains[idx]->Residues);

    while ((residue = (AjPResidue) ajListIterGet(iter)))
        (*EStrideType)->Ptr[residue->Idx - 1] = residue->eStrideType;

    ajListIterDel(&iter);

    return pdb->Chains[idx]->Nres;
}




/* ======================================================================= */
/* ============================== Casts ===================================*/
/* ======================================================================= */




/* @section Casts *************************************************************
**
** These functions examine the contents of an instance and return some
** derived information. Some of them provide access to the internal
** components of an instance. They are provided for programming convenience
** but should be used with caution.
**
******************************************************************************/




/* ======================================================================= */
/* =========================== Reporters ==================================*/
/* ======================================================================= */




/* @section Reporters *********************************************************
**
** These functions return the contents of an instance but do not make any
** changes.
**
******************************************************************************/




/* @func ajPdbChnidToNum ******************************************************
**
** Finds the chain number for a given chain identifier in a pdb structure
**
** @param [r] identifier [char] Chain identifier
** @param [r] pdb [const AjPPdb] Pdb object
** @param [w] chn [ajuint*] Chain number
**
** @return [AjBool] True on success
**
** @release 2.9.0
** @@
******************************************************************************/

AjBool ajPdbChnidToNum(char identifier, const AjPPdb pdb, ajuint *chn)
{
    ajuint i = 0U;

    for (i = 0U; i < pdb->Nchn; i++)
    {
        if (toupper((int) pdb->Chains[i]->Id) == toupper((int) identifier))
        {
            *chn = i + 1;

            return ajTrue;
        }

        /*
        ** Cope with chain id's of ' ' (which might be given as '.' in
        ** the Pdb object)
        */
        if ((identifier == ' ') && (pdb->Chains[i]->Id == '.'))
        {
            *chn = i + 1;

            return ajTrue;
        }
    }

    /*
    ** A '.' may be given as the id for domains comprising more than one
    ** chain
    */
    if (identifier == '.')
    {
        *chn = 1;

        return ajTrue;
    }

    return ajFalse;
}




/* @func ajPdbtospArrFindPdbid ************************************************
**
** Performs a binary search for a PDB code over an array of Pdbtosp
** structures (which of course must first have been sorted). This is a
** case-insensitive search.
**
** @param [r] array [AjPPdbtosp const *] Array of AjOPdbtosp objects
** @param [r] size [ajint] Size of array
** @param [r] identifier [const AjPStr] Search term
**
** @return [ajint] Index of first Pdbtosp object found with an PDB code
** matching id, or -1 if id is not found.
**
** @release 2.9.0
** @@
******************************************************************************/

ajint ajPdbtospArrFindPdbid(AjPPdbtosp const *array, ajint size,
                            const AjPStr identifier)
{
    ajint l = 0;
    ajint m = 0;
    ajint h = size - 1;
    ajint c = 0;

    while (l <= h)
    {
        m = (l + h) >> 1;

        if ((c = ajStrCmpCaseS(identifier, array[m]->Pdb)) < 0)
            h = m - 1;
        else if (c > 0)
            l = m + 1;
        else
            return m;
    }

    return -1;
}




/* ======================================================================= */
/* ========================== Input & Output ============================= */
/* ======================================================================= */




/* @section Input & output ****************************************************
**
** These functions are used for formatted input and output to file.
**
******************************************************************************/




/* @func ajPdbWriteAll ********************************************************
**
** Writes a clean coordinate file for a protein.
**
** @param [w] outf [AjPFile] Output file stream
** @param [r] pdb  [const AjPPdb] AJAX PDB object
**
** @return [AjBool] True on success
** @category output [AjPPdb] Writes a ccf-format file for a protein.
**
** @release 1.9.0
** @@
******************************************************************************/

AjBool ajPdbWriteAll(AjPFile outf, const AjPPdb pdb)
{
    ajuint i = 0U;
    ajuint j = 0U;
    AjIList iter = NULL;
    AjPAtom atom = NULL;
    AjPResidue residue = NULL;
    AjPSeqout outseq = NULL;

    /* Write the header information */

    ajFmtPrintF(outf, "%-5s%S\n", "ID", pdb->Pdb);
    ajFmtPrintF(outf, "XX\n");

    ajFmtPrintSplit(outf, pdb->Compnd, "DE   ", 75, " \t\r\n");
    ajFmtPrintF(outf, "XX\n");

    ajFmtPrintSplit(outf, pdb->Source, "OS   ", 75, " \t\r\n");
    ajFmtPrintF(outf, "XX\n");

    ajFmtPrintF(outf, "%-5sMETHOD ", "EX");

    if (pdb->Method == ajEPdbMethodXray)
        ajFmtPrintF(outf, "xray; ");
    else
        ajFmtPrintF(outf, "nmr_or_model; ");
    ajFmtPrintF(outf, "RESO %.2f; NMOD %d; NCHN %d; NGRP %d;\n", pdb->Reso,
                pdb->Nmod, pdb->Nchn, pdb->Ngp);

    /* Write chain-specific information */
    for (i = 0U; i < pdb->Nchn; i++)
    {
        ajFmtPrintF(outf, "XX\n");
        ajFmtPrintF(outf, "%-5s[%d]\n",
                    "CN",
                    i + 1);
        ajFmtPrintF(outf, "XX\n");

        ajFmtPrintF(outf, "%-5s", "IN");

        if (pdb->Chains[i]->Id == ' ')
            ajFmtPrintF(outf, "ID %c; ", '.');
        else
            ajFmtPrintF(outf, "ID %c; ", pdb->Chains[i]->Id);

#if AJFALSE
        ajFmtPrintF(outf, "NR %d; NL %d; NH %d; NE %d; NS %d; NT %d;\n",
                    pdb->Chains[i]->Nres,
                    pdb->Chains[i]->Nlig,
                    pdb->Chains[i]->numHelices,
                    pdb->Chains[i]->numStrands,
                    pdb->Chains[i]->numSheets,
                    pdb->Chains[i]->numTurns);
#endif
        ajFmtPrintF(outf, "NR %d; NL %d; NH %d; NE %d;\n",
                    pdb->Chains[i]->Nres,
                    pdb->Chains[i]->Nlig,
                    pdb->Chains[i]->numHelices,
                    pdb->Chains[i]->numStrands);
#if AJFALSE
        ajFmtPrintF(outf, "%-5sID %c; NR %d; NH %d; NW %d;\n",
                    "IN",
                    pdb->Chains[i]->Id,
                    pdb->Chains[i]->Nres,
                    pdb->Chains[i]->Nhet,
                    pdb->Chains[i]->Nwat);
#endif
        ajFmtPrintF(outf, "XX\n");
        outseq = ajSeqoutNewFile(outf);
        ajSeqoutDumpSwisslike(outseq, pdb->Chains[i]->Seq, "SQ");
        ajSeqoutDel(&outseq);
    }
    ajFmtPrintF(outf, "XX\n");

/*  printf("NCHN: %d   NMOD: %d\n", pdb->Nchn, pdb->Nmod); */

    /* Write RESIDUES list */
    for (i = 1U; i <= pdb->Nmod; i++)
    {
        for (j = 0U; j < pdb->Nchn; j++)
        {
            iter = ajListIterNewread(pdb->Chains[j]->Residues);

            while (!ajListIterDone(iter))
            {
                residue = (AjPResidue) ajListIterGet(iter);

                if (residue->Mod > i)
                    break;
                else if (residue->Mod != i)
                    continue;
                else
                {
                    ajFmtPrintF(outf, "%-5s%-5d%-5d%-5d%-6S%-2c%-6S",
                                "RE",
                                residue->Mod,
                                residue->Chn,
                                residue->Idx,
                                residue->Pdb,
                                residue->Id1,
                                residue->Id3);

                    if (residue->eNum != 0)
                        ajFmtPrintF(outf, "%-5d", residue->eNum);
                    else
                        ajFmtPrintF(outf, "%-5c", '.');

                    ajFmtPrintF(outf, "%-5S%-5c", residue->eId, residue->eType);

                    if (residue->eType == 'H')
                        ajFmtPrintF(outf, "%-5d", residue->eClass);
                    else
                        ajFmtPrintF(outf, "%-5c", '.');

                    if (residue->eStrideNum != 0)
                        ajFmtPrintF(outf, "%-5d", residue->eStrideNum);
                    else
                        ajFmtPrintF(outf, "%-5c", '.');

                    ajFmtPrintF(outf, "%-5c", residue->eStrideType);

                    ajFmtPrintF(outf, "%8.2f%8.2f%8.2f%8.2f%8.2f"
                                "%8.2f%8.2f%8.2f%8.2f%8.2f%8.2f%8.2f"
                                "%8.2f\n",
                                residue->Phi,
                                residue->Psi,
                                residue->Area,
                                residue->all_abs,
                                residue->all_rel,
                                residue->side_abs,
                                residue->side_rel,
                                residue->main_abs,
                                residue->main_rel,
                                residue->npol_abs,
                                residue->npol_rel,
                                residue->pol_abs,
                                residue->pol_rel);
                }
            }

            ajListIterDel(&iter);
        }
    }

    /* Write ATOMS list */
    for (i = 1U; i <= pdb->Nmod; i++)
    {
        for (j = 0U; j < pdb->Nchn; j++)
        {
            /* Print out chain-specific coordinates */
            iter = ajListIterNewread(pdb->Chains[j]->Atoms);

            while (!ajListIterDone(iter))
            {
                atom = (AjPAtom) ajListIterGet(iter);

                if (atom->Mod > i)
                    break;
                else if (atom->Mod != i)
                    continue;
                else
                {
                    if (atom->Type == 'H')
                        ajFmtPrintF(outf, "%-5s%-5d%-5d%-5d%-5c%-6S%-2c%-6S"
                                    "%-2c%-6S"
                                    "%9.3f%9.3f%9.3f%8.2f%8.2f\n",
                                    "AT",
                                    atom->Mod,
                                    atom->Chn,
                                    atom->Gpn,
                                    '.',
                                    atom->Pdb,
                                    atom->Id1,
                                    atom->Id3,
                                    atom->Type,
                                    atom->Atm,
                                    atom->X,
                                    atom->Y,
                                    atom->Z,
                                    atom->O,
                                    atom->B);
                    else
                    {
                        ajFmtPrintF(outf, "%-5s%-5d%-5d%-5c%-5d%-6S%-2c%-6S"
                                    "%-2c%-6S"
                                    "%9.3f%9.3f%9.3f%8.2f%8.2f\n",
                                    "AT",
                                    atom->Mod,
                                    atom->Chn,
                                    '.',
                                    atom->Idx,
                                    atom->Pdb,
                                    atom->Id1,
                                    atom->Id3,
                                    atom->Type,
                                    atom->Atm,
                                    atom->X,
                                    atom->Y,
                                    atom->Z,
                                    atom->O,
                                    atom->B);
                    }
                }
            }

            ajListIterDel(&iter);
        }

        /* Print out group-specific coordinates for this model */
        iter = ajListIterNewread(pdb->Groups);

        while (!ajListIterDone(iter))
        {
            atom = (AjPAtom) ajListIterGet(iter);

            if (atom->Mod > i)
                break;
            else if (atom->Mod != i)
                continue;
            else
            {
                ajFmtPrintF(outf, "%-5s%-5d%-5c%-5d%-5c%-6S%-2c%-6S%-2c%-6S"
                            "%9.3f%9.3f%9.3f%8.2f%8.2f\n",
                            "AT",
                            atom->Mod,
                            '.',
                            atom->Gpn,
                            '.',
                            atom->Pdb,
                            atom->Id1,
                            atom->Id3,
                            atom->Type,
                            atom->Atm,
                            atom->X,
                            atom->Y,
                            atom->Z,
                            atom->O,
                            atom->B);
            }
        }

        ajListIterDel(&iter);

        /* Print out water-specific coordinates for this model */
        iter = ajListIterNewread(pdb->Water);

        while (!ajListIterDone(iter))
        {
            atom = (AjPAtom) ajListIterGet(iter);

            if (atom->Mod > i)
                break;
            else if (atom->Mod != i)
                continue;
            else
            {
                ajFmtPrintF(outf, "%-5s%-5d%-5c%-5c%-5c%-6S%-2c%-6S%-2c%-6S"
                            "%9.3f%9.3f%9.3f%8.2f%8.2f\n",
                            "AT",
                            atom->Mod,
                            '.',
                            '.',
                            '.',
                            atom->Pdb,
                            atom->Id1,
                            atom->Id3,
                            atom->Type,
                            atom->Atm,
                            atom->X,
                            atom->Y,
                            atom->Z,
                            atom->O,
                            atom->B);
            }
        }

        ajListIterDel(&iter);
    }

    ajFmtPrintF(outf, "//\n");

    return ajTrue;
}




/* @func ajPdbWriteSegment ****************************************************
**
** Writes a clean coordinate file for a segment, e.g. a domain. The segment
** corresponds to a sequence that is passed to the function.
** In the clean coordinate file, the coordinates are presented as belonging
** to a single chain.  Coordinates for heterogens are NOT written to file.
**
** @param [w] outf    [AjPFile] Output file stream
** @param [r] pdb     [const AjPPdb]  Pdb object
** @param [r] segment [const AjPStr]  Sequence of segment to print out.
** @param [r] chnid   [char]    Chain id of segment
** @param [r] domain  [const AjPStr]  Domain code for segment
** @param [w] errf    [AjPFile] Output file stream for error messages
**
** @return [AjBool] True on success
**
** @release 3.0.0
** @@
**
******************************************************************************/

AjBool ajPdbWriteSegment(AjPFile outf, const AjPPdb pdb, const AjPStr segment,
                         char chnid, const AjPStr domain, AjPFile errf)
{
    ajuint chn = 0U;
    ajlong start = 0L;
    ajlong end = 0L;
    char id;

    AjIList iter = NULL;
    AjPAtom atm = NULL;
    AjPResidue residue = NULL;
    AjBool found_start = ajFalse;
    AjBool found_end = ajFalse;

    AjPSeqout outseq = NULL;

    /* Check for unknown or zero-length chain */
    if (!ajPdbChnidToNum(chnid, pdb, &chn))
    {
        ajWarn("Chain incompatibility error in "
               "ajPbdWriteSegment");

        ajFmtPrintF(errf, "//\n%S\nERROR Chain incompatibility "
                    "error in ajPbdWriteDomain\n", domain);

        return ajFalse;
    }
    else if (pdb->Chains[chn - 1]->Nres == 0)
    {
        ajWarn("Chain length zero");

        ajFmtPrintF(errf, "//\n%S\nERROR Chain length zero\n",
                    domain);

        return ajFalse;
    }

    /* Check if segment exists in this chain */
    if ((start = ajStrFindS(pdb->Chains[chn - 1]->Seq, segment)) == -1)
    {
        ajWarn("Domain not found in ajPbdWriteSegment");
        ajFmtPrintF(errf, "//\n%S\nERROR Domain not found "
                    "in ajPbdWriteSegment\n", domain);

        return ajFalse;
    }
    else
    {
        /* Residue numbers start at 1 ! */
        start++;
        end = start + MAJSTRGETLEN(segment) - 1;
    }

    /* Write header info. to domain coordinate file */
    ajFmtPrintF(outf, "%-5s%S\n", "ID", domain);
    ajFmtPrintF(outf, "XX\n");
    ajFmtPrintF(outf, "%-5sCo-ordinates for domain %S\n",
                "DE", domain);
    ajFmtPrintF(outf, "XX\n");
    ajFmtPrintF(outf, "%-5sDomain defined from sequence segment\n",
                "OS");
    ajFmtPrintF(outf, "XX\n");
    ajFmtPrintF(outf, "%-5sMETHOD ", "EX");

    if (pdb->Method == ajEPdbMethodXray)
        ajFmtPrintF(outf, "xray; ");
    else
        ajFmtPrintF(outf, "nmr_or_model; ");
    /* The NCHN and NMOD are hard-coded to 1 for domain files */
    ajFmtPrintF(outf, "RESO %.2f; NMOD 1; NCHN 1; NGRP 0;\n",
                pdb->Reso);

    id = pdb->Chains[chn - 1]->Id;

    if (id == ' ')
        id = '.';

    /* Write sequence to domain coordinate file */
    ajFmtPrintF(outf, "XX\n");
    ajFmtPrintF(outf, "%-5s[1]\n", "CN");
    ajFmtPrintF(outf, "XX\n");

    ajFmtPrintF(outf, "%-5sID %c; NR %d; NL 0; NH 0; NE 0;\n",
                "IN",
                id,
                MAJSTRGETLEN(segment));
    ajFmtPrintF(outf, "XX\n");
    outseq = ajSeqoutNewFile(outf);
    ajSeqoutDumpSwisslike(outseq, segment, "SQ");
    ajSeqoutDel(&outseq);
    ajFmtPrintF(outf, "XX\n");

    /* Write coordinates list to domain coordinate file */
    ajPdbChnidToNum(chnid, pdb, &chn);

    /* Initialise the iterator */
    iter = ajListIterNewread(pdb->Chains[chn - 1]->Atoms);

    /* Iterate through the list of residues */
    while ((residue = (AjPResidue) ajListIterGet(iter)))
    {
        if (residue->Mod != 1)
            break;

        if (!found_start)
        {
            if (residue->Idx == start)
                found_start = ajTrue;
            else
                continue;
        }

        if (!found_end)
        {
            if (residue->Idx == end)
                found_end = ajTrue;
        }
        /*
        ** The end position has been found, and the current residue is
        ** not the final residue.
        */
        else if (residue->Idx != end && found_end)
            break;


        /* Print out coordinate line */
        ajFmtPrintF(outf, "%-5s%-5d%-5d%-5d%-6S%-2c%-6S",
                    "RE",
                    residue->Mod,
                    1,          /* chn number is always given as 1 */
                    residue->Idx - start + 1,
                    residue->Pdb,
                    residue->Id1,
                    residue->Id3);

        if (residue->eNum != 0)
            ajFmtPrintF(outf, "%-5d", residue->eNum);
        else
            ajFmtPrintF(outf, "%-5c", '.');

        ajFmtPrintF(outf, "%-5S%-5c", residue->eId, residue->eType);

        if (residue->eType == 'H')
            ajFmtPrintF(outf, "%-5d", residue->eClass);
        else
            ajFmtPrintF(outf, "%-5c", '.');

        if (residue->eStrideNum != 0)
            ajFmtPrintF(outf, "%-5d", residue->eStrideNum);
        else
            ajFmtPrintF(outf, "%-5c", '.');

        ajFmtPrintF(outf, "%-5c", residue->eStrideType);

        ajFmtPrintF(outf, "%8.2f%8.2f%8.2f%8.2f%8.2f"
                    "%8.2f%8.2f%8.2f%8.2f%8.2f%8.2f%8.2f"
                    "%8.2f\n",
                    residue->Phi,
                    residue->Psi,
                    residue->Area,
                    residue->all_abs,
                    residue->all_rel,
                    residue->side_abs,
                    residue->side_rel,
                    residue->main_abs,
                    residue->main_rel,
                    residue->npol_abs,
                    residue->npol_rel,
                    residue->pol_abs,
                    residue->pol_rel);

        /* Assign pointer for this chain */
        /* residue2 = residue;   but it's never used */
    }

    ajListIterDel(&iter);

    /* Iterate through the list of atoms */
    while ((atm = (AjPAtom) ajListIterGet(iter)))
    {
        if (atm->Mod != 1)
            break;

        if (atm->Type != 'P')
            continue;

        if (!found_start)
        {
            if (atm->Idx == start)
                found_start = ajTrue;
            else
                continue;
        }

        if (!found_end)
        {
            if (atm->Idx == end)
                found_end = ajTrue;
        }
        /*
        ** The end position has been found, and the current atom no longer
        ** belongs to this final residue.
        */
        else if (atm->Idx != end && found_end)
            break;

        /* Print out coordinate line */
        ajFmtPrintF(outf, "%-5s%-5d%-5d%-5c%-5d%-6S%-2c%-6S%-2c%-6S"
                    "%9.3f%9.3f%9.3f%8.2f%8.2f\n"
                    "AT",
                    atm->Mod,   /* It will always be 1 */
                    1,          /* chn number is always given as 1 */
                    '.',
                    atm->Idx - start + 1,
                    atm->Pdb,
                    atm->Id1,
                    atm->Id3,
                    atm->Type,
                    atm->Atm,
                    atm->X,
                    atm->Y,
                    atm->Z,
                    atm->O,
                    atm->B);

        /* Assign pointer for this chain */
        /* atm2 = atm; but it's never used */
    }

    ajListIterDel(&iter);

    /* Write last line in file */
    ajFmtPrintF(outf, "//\n");

    return ajTrue;
}




/* @func ajHetWrite ***********************************************************
**
** Writes the contents of a Het object to file.
**
** @param [w] outf    [AjPFile]   Output file
** @param [r] het     [const AjPHet] AJAX Het
** @param [r] dogrep  [AjBool]    Flag (True if we are to write 'cnt'
**                                element of the Het object to file).
**
** @return [AjBool] True on success
** @category output [AjPHet] Write Het object to file in clean
**
** @release 2.9.0
** @@
******************************************************************************/

AjBool ajHetWrite(AjPFile outf, const AjPHet het, AjBool dogrep)
{
    ajuint i = 0U;

    /* Check arg's */
    if (!outf || !het)
        return ajFalse;

    for (i = 0U; i < het->Number; i++)
    {
        ajFmtPrintF(outf, "ID   %S\n", het->Entries[i]->abv);
        ajFmtPrintSplit(outf, het->Entries[i]->ful, "DE   ", 70, " \t\n\r");
        ajFmtPrintSplit(outf, het->Entries[i]->syn, "SY   ", 70, " \t\n\r");

        if (dogrep)
            ajFmtPrintF(outf, "NN   %d\n", het->Entries[i]->cnt);

        ajFmtPrintF(outf, "//\n");
    }

    return ajTrue;
}




/* @func ajPdbtospWrite *******************************************************
**
** Reads a list of Pdbtosp objects, e.g. taken from the swissprot:pdb
** equivalence table available at URL:
**  (1) http://www.expasy.ch/cgi-bin/lists?pdbtosp.txt)
** and writes the list out to file in embl-like format (see
** documentation for DOMAINATRIX "pdbtosp" application).
**
** @param [w] outf  [AjPFile] Output file
** @param [r] list  [const AjPList] List of Pdbtosp objects
**
** @return [AjBool] True of file was written ok.
**
** @release 2.9.0
** @@
******************************************************************************/

AjBool ajPdbtospWrite(AjPFile outf, const AjPList list)
{
    ajuint i = 0;
    
    AjIList iter = NULL;
    AjPPdbtosp pdbtosp = NULL;

    if (!outf || !list)
    {
        ajWarn("Bad args passed to ajPdbtospWrte");

        return ajFalse;
    }

    iter = ajListIterNewread(list);

    while ((pdbtosp = (AjPPdbtosp) ajListIterGet(iter)))
    {
        ajFmtPrintF(outf, "%-5s%S\nXX\n%-5s%u\nXX\n",
                    "EN", pdbtosp->Pdb, "NE", pdbtosp->Number);

        for (i = 0U; i < pdbtosp->Number; i++)
            ajFmtPrintF(outf, "%-5s%S ID; %S ACC;\n",
                        "IN", pdbtosp->Spr[i], pdbtosp->Acc[i]);

        ajFmtPrintF(outf, "XX\n//\n");
    }

    ajListIterDel(&iter);

    return ajTrue;
}




/* @func ajCmapWrite **********************************************************
**
** Write a Cmap object to file in CON format (see documentation for
** DOMAINATRIX "contacts" application).
**
** @param [u] outf    [AjPFile]  Output file stream.
** @param [r] cmap    [const AjPCmap]  Cmap object pointer.
**
** @return [AjBool] True on success (object was written successfully)
** @category output [AjPCmap] Write Cmap object to file in CON format.
**
** @release 3.0.0
** @@
******************************************************************************/

AjBool ajCmapWrite(AjPFile outf, const AjPCmap cmap)
{
    ajint x = 0;
    ajint y = 0;
    AjPStr Id = NULL;
    AjPStr Domid = NULL;
    AjPStr Ligid = NULL;
    AjPStr res1 = NULL;
    AjPStr res2 = NULL;
    AjPSeqout outseq = NULL;

    if (!cmap || !outf)
        return ajFalse;
    
    Id = ajStrNew();
    Domid = ajStrNew();
    Ligid = ajStrNew();
    res1 = ajStrNew();
    res2 = ajStrNew();

    /* EN */
    ajFmtPrintF(outf, "%-5s[%d]\n", "EN", cmap->en);
    ajFmtPrintF(outf, "XX\n");

    /* ID */
    if (MAJSTRGETLEN(cmap->Id))
        ajStrAssignS(&Id, cmap->Id);
    else
        ajStrAssignC(&Id, ".");

    if (MAJSTRGETLEN(cmap->Domid))
        ajStrAssignS(&Domid, cmap->Domid);
    else
        ajStrAssignC(&Domid, ".");

    if (MAJSTRGETLEN(cmap->Ligid))
        ajStrAssignS(&Ligid, cmap->Ligid);
    else
        ajStrAssignC(&Ligid, ".");

    ajFmtPrintF(outf, "%-5sPDB %S; DOM %S; LIG %S;\n",
                "ID",
                Id, Domid, Ligid);
    ajFmtPrintF(outf, "XX\n");

    /* DE */
    ajFmtPrintF(outf, "DE   %S\n", cmap->Desc);
    ajFmtPrintF(outf, "XX\n");

    /* SI */
    ajFmtPrintF(outf, "%-5sSN %d; NS %d\n", "SI", cmap->sn, cmap->ns);
    ajFmtPrintF(outf, "XX\n");

    /* CN */
    if ((cmap->Type == ajECmapTypeIntra) || (cmap->Type == ajECmapTypeLigand))
        ajFmtPrintF(outf, "%-5sMO .; CN1 %d; CN2 .; ID1 %c; ID2 .; "
                    "NRES1 %d; NRES2 .\n",
                    "CN",
                    cmap->Chn1,
                    cmap->Chid1,
                    cmap->Nres1);
    else if (cmap->Type == ajECmapTypeInter)
        ajFmtPrintF(outf, "%-5sMO .; CN1 %d; CN2 %d; ID1 %c; ID2 %c; "
                    "NRES1 %d; NRES2 %d\n",
                    "CN",
                    cmap->Chn1,
                    cmap->Chn2,
                    cmap->Chid1,
                    cmap->Chid2,
                    cmap->Nres1,
                    cmap->Nres2);
    else
        ajFatal("cmap type not known in ajCmapWrite");


    /* S1 */
    if (MAJSTRGETLEN(cmap->Seq1))
    {
        outseq = ajSeqoutNewFile(outf);
        ajSeqoutDumpSwisslike(outseq, cmap->Seq1, "S1");
        ajSeqoutDel(&outseq);
        ajFmtPrintF(outf, "XX\n");
    }

    /* S2 */
    if (cmap->Type == ajECmapTypeInter)
    {
        if (MAJSTRGETLEN(cmap->Seq2))
        {
            outseq = ajSeqoutNewFile(outf);
            ajSeqoutDumpSwisslike(outseq, cmap->Seq2, "S2");
            ajSeqoutDel(&outseq);
            ajFmtPrintF(outf, "XX\n");
        }
    }

    /* NC */
    if ((cmap->Type == ajECmapTypeIntra) || (cmap->Type == ajECmapTypeInter))
    {
        ajFmtPrintF(outf, "%-5sSM %d; LI .\n", "NC", cmap->Ncon);
        ajFmtPrintF(outf, "XX\n");
    }
    else
    {
        ajFmtPrintF(outf, "%-5sSM .; LI %d\n", "NC", cmap->Ncon);
        ajFmtPrintF(outf, "XX\n");
    }

    /* SM or LI */
    if (cmap->Type == ajECmapTypeIntra)
    {
        for (x = 0; x < cmap->Nres1; x++)
            for (y = x + 1; y < cmap->Nres1; y++)
            {
                if ((ajUint2dGet(cmap->Mat, x, y) == 1))
                {
                    /* Assign residue id. */
                    if (!ajResidueToTriplet(ajStrGetCharPos(cmap->Seq1, x),
                                            &res1))
                        ajFatal("Index out of range in ajCmapWrite");

                    if (!ajResidueToTriplet(ajStrGetCharPos(cmap->Seq1, y),
                                            &res2))
                        ajFatal("Index out of range in ajCmapWrite");

                    /* Print out the contact. */
                    ajFmtPrintF(outf, "%-5s%S %d ; %S %d\n", "SM", res1, x + 1,
                                res2, y + 1);
                }
            }
    }
    else if (cmap->Type == ajECmapTypeInter)
    {
        for (x = 0; x < cmap->Nres1; x++)
            for (y = x + 1; y < cmap->Nres2; y++)
            {
                if ((ajUint2dGet(cmap->Mat, x, y) == 1))
                {
                    /* Assign residue id. */
                    if (!ajResidueToTriplet(ajStrGetCharPos(cmap->Seq1, x),
                                            &res1))
                        ajFatal("Index out of range in ajCmapWrite");

                    if (!ajResidueToTriplet(ajStrGetCharPos(cmap->Seq2, y),
                                            &res2))
                        ajFatal("Index out of range in ajCmapWrite");

                    /* Print out the contact. */
                    ajFmtPrintF(outf, "%-5s%S %d ; %S %d\n", "SM", res1, x + 1,
                                res2, y + 1);
                }
            }
    }
    else if (cmap->Type == ajECmapTypeLigand)
    {
        for (x = 0; x < cmap->Nres1; x++)
            if ((ajUint2dGet(cmap->Mat, 0, x) == 1))
            {
                /* Assign residue id. */
                if (!ajResidueToTriplet(ajStrGetCharPos(cmap->Seq1, x), &res1))
                    ajFatal("Index out of range in ajCmapWrite");

                /* Print out the contact. */
                ajFmtPrintF(outf, "%-5s%S %d\n", "LI", res1, x + 1);
            }
    }

    ajFmtPrintF(outf, "//\n");

    ajStrDel(&Id);
    ajStrDel(&Domid);
    ajStrDel(&Ligid);
    ajStrDel(&res1);
    ajStrDel(&res2);

    return ajTrue;
}




/* @func ajPdbExit ************************************************************
**
** Cleanup of Pdb function internals.
**
** @return [void]
**
** @release 6.1.0
** @@
******************************************************************************/

void ajPdbExit(void)
{
    ajStrDel(&pdbGStrline);
    ajStrDel(&pdbGStrtemp_id);
    ajStrDel(&pdbGStrtemp_domid);
    ajStrDel(&pdbGStrtemp_ligid);
    ajStrDel(&pdbGStrtype);
    ajStrDel(&pdbGStrdesc);
    ajStrDel(&pdbGStrtmpstr);

    return;
}




/* ======================================================================= */
/* ======================== Miscellaneous =================================*/
/* ======================================================================= */




/* @section Miscellaneous *****************************************************
**
** These functions may have diverse functions that do not fit into the other
** categories.
**
******************************************************************************/
