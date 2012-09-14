/* @source ajdmx **************************************************************
**
** AJAX library code for some of the DOMAINATRIX EMBASSY applications.
** For use with the Scophit and Scopalign objects.  The code is disparate
** including low-level functions and algorithms.  The functionality will
** eventually be subsumed by other AJAX and NUCLEUS libraries.
**
** @author Copyright (C) 2004 Ranjeeva Ranasinghe (rranasin@hgmp.mrc.ac.uk)
** @author Copyright (C) 2004 Jon Ison (jison@hgmp.mrc.ac.uk)
** @version $Revision: 1.40 $
** @modified $Date: 2012/07/02 16:47:42 $ by $Author: rice $
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

#include "ajdmx.h"
#include "ajarr.h"
#include "ajmath.h"
#include "ajfileio.h"
#include "ajseqwrite.h"

#include <math.h>




/* ========================================================================= */
/* =============================== constants =============================== */
/* ========================================================================= */




/* ========================================================================= */
/* =========================== global variables ============================ */
/* ========================================================================= */




/* ========================================================================= */
/* ============================= private data ============================== */
/* ========================================================================= */




/* ========================================================================= */
/* =========================== private constants =========================== */
/* ========================================================================= */




/* ========================================================================= */
/* =========================== private variables =========================== */
/* ========================================================================= */




static AjPStr dmxGStrline    = NULL; /* Line of text */
static AjPStr dmxGStrtype    = NULL;
static AjPStr dmxGStrclass   = NULL;
static AjPStr dmxGStrfold    = NULL;
static AjPStr dmxGStrsuper   = NULL;
static AjPStr dmxGStrfamily  = NULL;
static AjPStr dmxGStrarch    = NULL;
static AjPStr dmxGStrtopo    = NULL;
static AjPStr dmxGStrpostsim = NULL; /* Post-similar line */
static AjPStr dmxGStrposttmp = NULL; /* Temp. storage for post-similar line */

static AjPStr dmxGStrposisim = NULL; /* Positions line */
static AjPStr dmxGStrpositmp = NULL; /* Temp. storage for Positions line */
static AjPStr dmxGStrseq1    = NULL;
static AjPStr dmxGStrcodetmp = NULL; /* Id code of sequence */




/* ========================================================================= */
/* =========================== private functions =========================== */
/* ========================================================================= */




static void dmxTraceScophit(const AjPScophit scophit, const char *title);




/* ========================================================================= */
/* ======================= All functions by section ======================== */
/* ========================================================================= */




/* @section Constructors ******************************************************
**
** All constructors return a pointer to a new instance. It is the
** responsibility of the user to first destroy any previous instance. The
** target pointer does not need to be initialised to NULL, but it is good
** programming practice to do so anyway.
**
******************************************************************************/




/* @func ajDmxScophitNew ******************************************************
**
** Constructor for an AJAX SCOP Hit object.
**
** @return [AjPScophit] AJAX SCOP Hit object
**
** @release 2.9.0
** @@
******************************************************************************/

AjPScophit ajDmxScophitNew(void)
{
    AjPScophit scophit = NULL;

    AJNEW0(scophit);

    scophit->Class        = ajStrNew();
    scophit->Architecture = ajStrNew();
    scophit->Topology     = ajStrNew();
    scophit->Fold         = ajStrNew();
    scophit->Superfamily  = ajStrNew();
    scophit->Family       = ajStrNew();
    scophit->Seq          = ajStrNew();
    scophit->Acc          = ajStrNew();
    scophit->Spr          = ajStrNew();
    scophit->Dom          = ajStrNew();
    scophit->Typeobj      = ajStrNew();
    scophit->Typesbj      = ajStrNew();
    scophit->Model        = ajStrNew();
    scophit->Alg          = ajStrNew();
    scophit->Group        = ajStrNew();
    scophit->Start        = 0U;
    scophit->End          = 0U;
    scophit->Rank         = 0U;
    scophit->Score        = 0.0F;
    scophit->Sunid_Family = 0U;
    scophit->Eval         = 0.0F;
    scophit->Pval         = 0.0F;
    scophit->Target       = ajFalse;
    scophit->Target2      = ajFalse;
    scophit->Priority     = ajFalse;

    return scophit;
}




/* @func ajDmxScopalgNew ******************************************************
**
** Constructor for an AJAX SCOP Alignment object.
** This is normally called by the ajDmxScopalgRead function.
** Fore-knowledge of the number of sequences is required.
**
** @param [r] n [ajuint] Number of sequences
**
** @return [AjPScopalg] AJAX SCOP Alignment object
**
** @release 2.9.0
** @@
******************************************************************************/

AjPScopalg ajDmxScopalgNew(ajuint n)
{
    ajuint i = 0U;

    AjPScopalg scopalg = NULL;

    AJNEW0(scopalg);

    scopalg->Class        = ajStrNew();
    scopalg->Architecture = ajStrNew();
    scopalg->Topology     = ajStrNew();
    scopalg->Fold         = ajStrNew();
    scopalg->Superfamily  = ajStrNew();
    scopalg->Family       = ajStrNew();
    scopalg->Architecture = ajStrNew();
    scopalg->Topology     = ajStrNew();
    scopalg->Post_similar = ajStrNew();
    scopalg->Positions    = ajStrNew();
    scopalg->Width        = 0;
    scopalg->Number       = n;

    if(n)
    {
        AJCNEW0(scopalg->Codes, n);

        for(i = 0U; i < n; i++)
            scopalg->Codes[i] = ajStrNew();

        AJCNEW0(scopalg->Seqs, n);

        for(i = 0U; i < n; i++)
            scopalg->Seqs[i] = ajStrNew();
    }

    return scopalg;
}




/* @func ajDmxScopalgRead *****************************************************
**
** Read an AJAX SCOP Alignment object from an AJAX File.
**
** @param [u] inf [AjPFile] Input AJAX File
** @param [w] Pscopalg [AjPScopalg*] AJAX SCOP Alignment address
**
** @return [AjBool] True if the file contained any data, even an empty
** alignment. False if the file did not contain a 'TY' record, which is
** taken to indicate a domain alignment file.
**
** @release 2.9.0
** @@
******************************************************************************/

AjBool ajDmxScopalgRead(AjPFile inf, AjPScopalg *Pscopalg)
{
    AjBool ok = ajFalse;    /* True if the file contained 'TY' record. */

    AjBool done_1st_blk = ajFalse; /* Flag for whether we've read first
                                      block of sequences */

    ajuint i    = 0U;  /* Loop counter */
    ajuint j    = 0U;  /* Loop counter */

    ajint cnt   = 0;   /* Temp. counter of sequence */
    ajuint nseq = 0U;  /* No. of sequences in alignment */
    ajint Sunid = 0;   /* SCOP Sunid for family */
    ajint ntok  = 0;   /*
                        * No. string tokens in sequence line from alignment.
                        * Sequence start and end may or may not be present,
                        * therefore ntok is either 2 or 4:
                        * (2)  (ACC       SEQ    ) or
                        * (4)  (ACC start SEQ end)
                        */

    AjPList list_seqs  = NULL;     /* List of sequences */
    AjPList list_codes = NULL;     /* List of codes */
    AjPStr  *arr_seqs  = NULL;     /* Array of sequences */
    AjPStr  seq        = NULL;
    AjPStr  code       = NULL;     /* Id code of sequence */

    /* Check args */
    if(!inf)
        return ajFalse;

    if(!Pscopalg)
        return ajFalse;

    /* Allocate strings */
    /* Only initialise strings if this is called for the first time */
    if(!dmxGStrline)
    {
        dmxGStrtype    = ajStrNew();
        dmxGStrclass   = ajStrNew();
        dmxGStrfold    = ajStrNew();
        dmxGStrsuper   = ajStrNew();
        dmxGStrfamily  = ajStrNew();
        dmxGStrarch    = ajStrNew();
        dmxGStrtopo    = ajStrNew();
        dmxGStrline    = ajStrNew();
        dmxGStrpostsim = ajStrNew();
        dmxGStrposttmp = ajStrNew();
        dmxGStrposisim = ajStrNew();
        dmxGStrpositmp = ajStrNew();
        dmxGStrcodetmp = ajStrNew();
        dmxGStrseq1    = ajStrNew();
    }

    /* Create new lists */
    list_seqs  = ajListstrNew();
    list_codes = ajListstrNew();

    /* Read the rest of the file */
    while(ajReadlineTrim(inf, &dmxGStrline))
    {
        if(ajStrPrefixC(dmxGStrline, "# TY"))
        {
            ok = ajTrue;
            ajStrAssignC(&dmxGStrtype, ajStrGetPtr(dmxGStrline) + 5);
            ajStrRemoveWhiteExcess(&dmxGStrtype);
        }
        else if(ajStrPrefixC(dmxGStrline, "# SI"))
        {
            ajFmtScanS(dmxGStrline, "%*s %*s %d", &Sunid);
        }
        else if(ajStrPrefixC(dmxGStrline, "# CL"))
        {
            ajStrAssignC(&dmxGStrclass, ajStrGetPtr(dmxGStrline) + 5);
            ajStrRemoveWhiteExcess(&dmxGStrclass);
        }
        else if(ajStrPrefixC(dmxGStrline, "# FO"))
        {
            ajStrAssignC(&dmxGStrfold, ajStrGetPtr(dmxGStrline) + 5);

            while((ajReadlineTrim(inf, &dmxGStrline)))
            {
                if(ajStrPrefixC(dmxGStrline, "# XX"))
                    break;

                ajStrAppendC(&dmxGStrfold, ajStrGetPtr(dmxGStrline) + 5);
            }

            ajStrRemoveWhiteExcess(&dmxGStrfold);
        }
        else if(ajStrPrefixC(dmxGStrline, "# SF"))
        {
            ajStrAssignC(&dmxGStrsuper, ajStrGetPtr(dmxGStrline) + 5);

            while((ajReadlineTrim(inf, &dmxGStrline)))
            {
                if(ajStrPrefixC(dmxGStrline, "# XX"))
                    break;

                ajStrAppendC(&dmxGStrsuper, ajStrGetPtr(dmxGStrline) + 5);
            }

            ajStrRemoveWhiteExcess(&dmxGStrsuper);
        }
        else if(ajStrPrefixC(dmxGStrline, "# FA"))
        {
            ajStrAssignC(&dmxGStrfamily, ajStrGetPtr(dmxGStrline) + 5);

            while((ajReadlineTrim(inf, &dmxGStrline)))
            {
                if(ajStrPrefixC(dmxGStrline, "# XX"))
                    break;

                ajStrAppendC(&dmxGStrfamily, ajStrGetPtr(dmxGStrline) + 5);
            }

            ajStrRemoveWhiteExcess(&dmxGStrfamily);
        }
        else if(ajStrPrefixC(dmxGStrline, "# AR"))
        {
            ajStrAssignC(&dmxGStrarch, ajStrGetPtr(dmxGStrline) + 5);

            while((ajReadlineTrim(inf, &dmxGStrline)))
            {
                if(ajStrPrefixC(dmxGStrline, "# XX"))
                    break;

                ajStrAppendC(&dmxGStrarch, ajStrGetPtr(dmxGStrline) + 5);
            }

            ajStrRemoveWhiteExcess(&dmxGStrarch);
        }
        else if(ajStrPrefixC(dmxGStrline, "# TP"))
        {
            ajStrAssignC(&dmxGStrtopo, ajStrGetPtr(dmxGStrline) + 5);

            while((ajReadlineTrim(inf, &dmxGStrline)))
            {
                if(ajStrPrefixC(dmxGStrline, "# XX"))
                    break;

                ajStrAppendC(&dmxGStrtopo, ajStrGetPtr(dmxGStrline) + 5);
            }

            ajStrRemoveWhiteExcess(&dmxGStrtopo);
        }
        else if(ajStrPrefixC(dmxGStrline, "# XX"))
            continue;
        else if (ajStrPrefixC(dmxGStrline, "# Post_similar"))
        {
            /* Parse post_similar line */
            ajFmtScanS(dmxGStrline, "%*s %*s %S", &dmxGStrposttmp);

            if(done_1st_blk == ajTrue)
                ajStrAppendS(&dmxGStrpostsim, dmxGStrposttmp);
            else
                ajStrAssignS(&dmxGStrpostsim, dmxGStrposttmp);

            continue;
        }
        else if (ajStrPrefixC(dmxGStrline, "# Positions"))
        {
            /* Parse Positions line */
            ajFmtScanS(dmxGStrline, "%*s %*s %S", &dmxGStrpositmp);

            if(done_1st_blk == ajTrue)
                ajStrAppendS(&dmxGStrposisim, dmxGStrpositmp);
            else
                ajStrAssignS(&dmxGStrposisim, dmxGStrpositmp);

            continue;
        }
        /* Ignore any other line beginning with '#' which are
           taken to be comments, e.g. 'Number' lines */
        else if((ajStrPrefixC(dmxGStrline ,"#")))
            continue;
        /* ajFileReadLine will have trimmed the tailing \n */
        else if(ajStrGetCharPos(dmxGStrline, 1) == '\0')
        {
            /*
            ** The first blank line therefore the first block of sequences
            ** has been done
            */

            if(!ok)
            {
                ajWarn("ajDmxScopalgRead but file was not identified as "
                       "being a domain alignment file");
                return ajFalse;
            }

            done_1st_blk = ajTrue;
            j++;

            if(j == 1)
                ajListstrToarray(list_seqs, &arr_seqs);

            cnt = 0;
            continue;
        }
        else
        {
            /* Line of sequence */
            if(!ok)
            {
                ajWarn("ajDmxScopalgRead but file was not identified as "
                       "being a domain alignment file");
                return ajFalse;
            }

            /* Parse a line of sequence */
            if(done_1st_blk == ajTrue)
            {
                /* already read in the first block of sequences */
                if(ntok == 4)
                    ajFmtScanS(dmxGStrline, "%*s %*s %S", &dmxGStrseq1);
                else if(ntok == 2)
                    ajFmtScanS(dmxGStrline, "%*s %S", &dmxGStrseq1);
                else
                    ajFatal("ajDmxScopalgRead could not parse alignment");

                ajStrAppendS(&arr_seqs[cnt], dmxGStrseq1);
                cnt++;
                continue;
            }
            else
            {
                /* It is a sequence line from the first block */
                nseq++;
                seq = ajStrNew();
                code = ajStrNew();

                if(((ntok = ajStrParseCountC(dmxGStrline, " ")) == 4))
                    ajFmtScanS(dmxGStrline, "%S %*s %S", &code, &seq);
                else if(ntok == 2)
                    ajFmtScanS(dmxGStrline, "%S %S", &code, &seq);
                else
                    ajFatal("ajDmxScopalgRead could not parse alignment");

                /* Push strings onto lists */
                ajListstrPushAppend(list_seqs, seq);
                ajListstrPushAppend(list_codes, code);
                continue;
            }
        }
    }

    if(!ok)
    {
        ajWarn("ajDmxScopalgRead but file was not identified as being "
               "a domain alignment file");

        return ajFalse;
    }

    /*
    ** Cope for cases where alignment is in one block only,
    ** i.e. there were no empty lines:
    **
    ** XX
    ** # Number               10        20        30        40        50
    ** d1bsna1      1 QDLDEARAMEAKRKAEEHISSSHGDVDYAQASAELAKAIAQLRVIELTKK 50
    ** d1e79h1      1 DMLDLGAAKANLEKAQSELLGAADEATRAEIQIRIEANEALVKAL----- 43
    ** # Post_similar 111111111111111111111111111111111111111111111-----
    */
    if(!done_1st_blk && nseq)
        ajListstrToarray(list_seqs, &arr_seqs);

    ajStrDel(&dmxGStrseq1);

    if(!nseq)
        ajWarn("No sequences in alignment !\n");

    /* Allocate memory for Scopalg structure */
    *Pscopalg = ajDmxScopalgNew(nseq);

    /* Assign domain records */
    if(ajStrMatchC(dmxGStrtype, "SCOP"))
        (*Pscopalg)->Type = ajEDomainTypeSCOP;
    else if(ajStrMatchC(dmxGStrtype, "CATH"))
        (*Pscopalg)->Type = ajEDomainTypeCATH;

    ajStrAssignS(&(*Pscopalg)->Class, dmxGStrclass);
    ajStrAssignS(&(*Pscopalg)->Architecture, dmxGStrarch);
    ajStrAssignS(&(*Pscopalg)->Topology, dmxGStrtopo);
    ajStrAssignS(&(*Pscopalg)->Fold, dmxGStrfold);
    ajStrAssignS(&(*Pscopalg)->Superfamily, dmxGStrsuper);
    ajStrAssignS(&(*Pscopalg)->Family, dmxGStrfamily);
    (*Pscopalg)->Sunid_Family = Sunid;

    if(nseq)
    {
        /* Assign sequences and free memory */
        for(i = 0U; i < nseq; i++)
        {
            ajStrAssignS(&(*Pscopalg)->Seqs[i], arr_seqs[i]);
            ajStrDel(&arr_seqs[i]);
        }
        AJFREE(arr_seqs);

        /* Assign width */
        (*Pscopalg)->Width = ajStrGetLen((*Pscopalg)->Seqs[0]);

        for(i = 0U; ajListstrPop(list_codes, &dmxGStrcodetmp); i++)
        {
            ajStrAssignS(&(*Pscopalg)->Codes[i], dmxGStrcodetmp);
            ajStrDel(&dmxGStrcodetmp);
        }

        /* Assign Post_similar line */
        ajStrAssignS(&(*Pscopalg)->Post_similar, dmxGStrpostsim);

        /* Assign Positions line */
        ajStrAssignS(&(*Pscopalg)->Positions, dmxGStrposisim);
    }
    else
        ajWarn("ajDmxScopalgRead called but no sequences found.");

    ajListstrFree(&list_seqs);
    ajListstrFree(&list_codes);

    return ajTrue;
}




/* @section Structure Destructors *********************************************
**
** All destructor functions receive the address of the instance to be
** deleted.  The original pointer is set to NULL so is ready for re-use.
**
******************************************************************************/




/* @func ajDmxScophitDel ******************************************************
**
** Destructor for an AJAX SCOP Hit object.
**
** @param [w] Pscophit [AjPScophit*] AJAX Scophit address
**
** @return [void]
**
** @release 2.9.0
** @@
******************************************************************************/

void ajDmxScophitDel(AjPScophit *Pscophit)
{
    AjPScophit scophit = NULL;

    if(!Pscophit || !*Pscophit)
        return;

    scophit = *Pscophit;

    ajStrDel(&scophit->Class);
    ajStrDel(&scophit->Architecture);
    ajStrDel(&scophit->Topology);
    ajStrDel(&scophit->Fold);
    ajStrDel(&scophit->Superfamily);
    ajStrDel(&scophit->Family);
    ajStrDel(&scophit->Seq);
    ajStrDel(&scophit->Acc);
    ajStrDel(&scophit->Spr);
    ajStrDel(&scophit->Dom);
    ajStrDel(&scophit->Typeobj);
    ajStrDel(&scophit->Typesbj);
    ajStrDel(&scophit->Model);
    ajStrDel(&scophit->Alg);
    ajStrDel(&scophit->Group);

    AJFREE(scophit);

    *Pscophit = NULL;

    return;
}




/* @func ajDmxScophitDelWrap **************************************************
**
** Wrapper to destructor for an AJAX SCOP Hit object for use with generic
** functions.
**
** @param [d] Pitem [void**] AJAX SCOP Hit address
** @see ajListPurge
**
** @return [void]
**
** @release 2.9.0
** @@
******************************************************************************/

void ajDmxScophitDelWrap(void **Pitem)
{
    ajDmxScophitDel((AjPScophit *) Pitem);

    return;
}




/* @func ajDmxScopalgDel ******************************************************
**
** Destructor for an AJAX SCOP Alignment object.
**
** @param [d] Pscopalg [AjPScopalg*] AJAX SCOP Alignment address
**
** @return [void]
**
** @release 2.9.0
** @@
******************************************************************************/

void ajDmxScopalgDel(AjPScopalg *Pscopalg)
{
    ajuint i = 0U;

    AjPScopalg scopalg = NULL;

    if(!Pscopalg || !*Pscopalg)
        return;

    scopalg = *Pscopalg;

    ajStrDel(&scopalg->Class);
    ajStrDel(&scopalg->Architecture);
    ajStrDel(&scopalg->Topology);
    ajStrDel(&scopalg->Fold);
    ajStrDel(&scopalg->Superfamily);
    ajStrDel(&scopalg->Family);
    ajStrDel(&scopalg->Architecture);
    ajStrDel(&scopalg->Topology);
    ajStrDel(&scopalg->Post_similar);
    ajStrDel(&scopalg->Positions);

    for(i = 0U; i < scopalg->Number; i++)
    {
        ajStrDel(&scopalg->Codes[i]);
        ajStrDel(&scopalg->Seqs[i]);
    }

    AJFREE(scopalg->Codes);
    AJFREE(scopalg->Seqs);

    AJFREE(scopalg);

    *Pscopalg = NULL;

    return;
}




/* @section Assignments *******************************************************
**
** These functions overwrite the instance provided as the first argument
** A NULL value is always acceptable so these functions are often used to
** create a new instance by assignment.
**
******************************************************************************/




/* @func ajDmxScophitListCopy *************************************************
**
** Read an AJAX List of AJAX SCOP Hit objects and return a pointer to a
** duplicate of the AJAX List.
**
** @param [r] scophits [const AjPList] AJAX List of AJAX SCOP Hit objects
**
** @return [AjPList] True on success (list was duplicated ok)
**
** @release 2.9.0
** @@
**
** Should modify this eventually to fit "standard" method for assignment
** functions, i.e. pass in the pointer as the first argument
**
******************************************************************************/

AjPList ajDmxScophitListCopy(const AjPList scophits)
{
    AjPList list = NULL;
    AjIList iter = NULL;

    AjPScophit oldscophit = NULL;
    AjPScophit newscophit = NULL;

    /* Check arg's */
    if(!scophits)
    {
        ajWarn("Bad arg's passed to ajDmxScophitListCopy\n");
        return NULL;
    }

    /* Allocate the new list */
    list = ajListNew();

    /* Initialise the iterator */
    iter = ajListIterNewread(scophits);

    /* Iterate through the list of Scophit objects */
    while((oldscophit = (AjPScophit)ajListIterGet(iter)))
    {
        newscophit = ajDmxScophitNew();

        ajDmxScophitCopy(&newscophit, oldscophit);

        /* Push scophit onto list */
        ajListPushAppend(list, newscophit);
    }

    ajListIterDel(&iter);

    return list;
}




/* @func ajDmxScophitCopy *****************************************************
**
** Copies the contents from one AJAX SCOP Hit object to another.
**
** @param [w] Pto  [AjPScophit*] AJAX SCOP Hit address
** @param [r] from [const AjPScophit] AJAX SCOP Hit
**
** @return [AjBool] True if copy was successful.
**
** @release 2.9.0
** @@
******************************************************************************/

AjBool ajDmxScophitCopy(AjPScophit *Pto, const AjPScophit from)
{
    if(!Pto || !*Pto)
        return ajFalse;

    if(!from)
        return ajFalse;

    (*Pto)->Type = from->Type;
    ajStrAssignS(&(*Pto)->Class, from->Class);
    ajStrAssignS(&(*Pto)->Architecture, from->Architecture);
    ajStrAssignS(&(*Pto)->Topology, from->Topology);
    ajStrAssignS(&(*Pto)->Fold, from->Fold);
    ajStrAssignS(&(*Pto)->Superfamily, from->Superfamily);
    ajStrAssignS(&(*Pto)->Family, from->Family);
    ajStrAssignS(&(*Pto)->Seq, from->Seq);
    ajStrAssignS(&(*Pto)->Acc, from->Acc);
    ajStrAssignS(&(*Pto)->Spr, from->Spr);
    ajStrAssignS(&(*Pto)->Dom, from->Dom);
    ajStrAssignS(&(*Pto)->Typeobj, from->Typeobj);
    ajStrAssignS(&(*Pto)->Typesbj, from->Typesbj);
    ajStrAssignS(&(*Pto)->Model, from->Model);
    ajStrAssignS(&(*Pto)->Alg, from->Alg);
    ajStrAssignS(&(*Pto)->Group, from->Group);
    (*Pto)->Start        = from->Start;
    (*Pto)->End          = from->End;
    (*Pto)->Rank         = from->Rank;
    (*Pto)->Score        = from->Score;
    (*Pto)->Eval         = from->Eval;
    (*Pto)->Pval         = from->Pval;
    (*Pto)->Target       = from->Target;
    (*Pto)->Target2      = from->Target2;
    (*Pto)->Priority     = from->Priority;
    (*Pto)->Sunid_Family = from->Sunid_Family;

    return ajTrue;
}




/* @section Modifiers *********************************************************
**
** These functions use the contents of an instance and update them.
**
******************************************************************************/




/* @func ajDmxScophitTargetLowPriority ****************************************
**
** Sets the Target element of an AJAX SCOP Hit object to ajTrue if its
** Priority is low.
**
** @param [u] Pscophit [AjPScophit*] AJAX SCOP Hit address
**
** @return [AjBool] True on success. False otherwise.
**
** @release 2.9.0
** @@
******************************************************************************/

AjBool ajDmxScophitTargetLowPriority(AjPScophit *Pscophit)
{
    if(!Pscophit || !*Pscophit)
    {
        ajWarn("Bad arg's passed to ajDmxScophitTargetLowPriority\n");
        return ajFalse;
    }

    if((*Pscophit)->Priority == ajFalse)
        (*Pscophit)->Target = ajTrue;

    return ajTrue;
}




/* @func ajDmxScophitTarget2 **************************************************
**
** Sets the Target2 element of an AJAX SCOP Hit object to ajTrue.
**
** @param [u] Pscophit [AjPScophit*] AJAX SCOP Hit address
**
** @return [AjBool] True on success. False otherwise.
**
** @release 2.9.0
** @@
******************************************************************************/

AjBool ajDmxScophitTarget2(AjPScophit *Pscophit)
{
    if(!Pscophit || !*Pscophit)
    {
        ajWarn("Bad arg's passed to ajDmxScophitTarget2\n");
        return ajFalse;
    }

    (*Pscophit)->Target2 = ajTrue;

    return ajTrue;
}




/* @func ajDmxScophitTarget ***************************************************
**
** Sets the Target element of an AJAX SCOP Hit object to ajTrue.
**
** @param [u] Pscophit [AjPScophit*] AJAX SCOP Hit address
**
** @return [AjBool] True on success. False otherwise.
**
** @release 2.9.0
** @@
******************************************************************************/

AjBool ajDmxScophitTarget(AjPScophit *Pscophit)
{
    if(!Pscophit || !*Pscophit)
    {
        ajWarn("Bad arg's passed to ajDmxScophitTarget\n");
        return ajFalse;
    }

    (*Pscophit)->Target = ajTrue;

    return ajTrue;
}




/* @section Operators *********************************************************
**
** These functions use the contents of an instance but do not make any
** changes.
**
******************************************************************************/




/* @func ajDmxScophitCheckTarget **********************************************
**
** Checks to see if the Target element of an AJAX SCOP Hit object equals
** ajTrue.
**
** @param [r] scophit [const AjPScophit] AJAX SCOP Hit
**
** @return [AjBool] Returns ajTrue if the Target element of the AAX SCOP Hit
** object equals ajTrue, returns ajFalse otherwise.
**
** @release 2.9.0
** @@
******************************************************************************/

AjBool ajDmxScophitCheckTarget(const AjPScophit scophit)
{
    return scophit->Target;
}




/* @section Casts *************************************************************
**
** These functions examine the contents of an instance and return some
** derived information. Some of them provide access to the internal
** components of an instance. They are provided for programming convenience
** but should be used with caution.
**
******************************************************************************/




/* @section Reporters *********************************************************
**
** These functions return the contents of an instance but do not make any
** changes.
**
******************************************************************************/




/* @func ajDmxScophitCompScore ************************************************
**
** Function to compare AJAX SCOP Hit objects by score member.
** Usually called by ajListSort.
**
** @param [r] item1 [const void*] AJAX SCOP Hit address 1
** @param [r] item2 [const void*] AJAX SCOP Hit address 2
** @see ajListSort
**
** @return [ajint] 1 if score1 < score2, 0 if score1 == score2, else -1.
**
** @release 2.9.0
** @@
******************************************************************************/

ajint ajDmxScophitCompScore(const void *item1, const void *item2)
{
    AjPScophit scophit1 = *(AjOScophit *const *) item1;
    AjPScophit scophit2 = *(AjOScophit *const *) item2;

    if(scophit1->Score < scophit2->Score)
        return -1;
    else if(E_FPEQ(scophit1->Score, scophit2->Score, U_FEPS))
        return 0;

    return 1;
}




/* @func ajDmxScophitCompPval *************************************************
**
** Function to compare AJAX SCOP Hit objects by Pval member.
** Usually called by ajListSort.
**
** @param [r] item1 [const void*] AJAX SCOP Hit address 1
** @param [r] item2 [const void*] AJAX SCOP Hit address 2
** @see ajListSort
**
** @return [ajint] 1 if Pval1 > Pval2, 0 if Pval1 == Pval2, else -1.
**
** @release 2.9.0
** @@
******************************************************************************/

ajint ajDmxScophitCompPval(const void *item1, const void *item2)
{
    AjPScophit scophit1 = *(AjOScophit *const *) item1;
    AjPScophit scophit2 = *(AjOScophit *const *) item2;

    if(scophit1->Pval < scophit2->Pval)
        return -1;
    else if(E_FPEQ(scophit1->Pval, scophit2->Pval, U_FEPS))
        return 0;

    return 1;
}




/* @func ajDmxScophitCompAcc **************************************************
**
** Function to compare AJAX SCOP Hit objects by Acc member.
** Usually called by ajListSort.
**
** @param [r] item1 [const void*] AJAX SCOP Hit address 1
** @param [r] item2 [const void*] AJAX SCOP Hit address 2
** @see ajListSort
**
** @return [ajint] -1 if Acc1 should sort before Acc2,
**                 +1 if the Acc2 should sort first.
**                  0 if they are identical in length and content.
**
** @release 2.9.0
** @@
******************************************************************************/

ajint ajDmxScophitCompAcc(const void *item1, const void *item2)
{
    AjPScophit scophit1 = *(AjOScophit *const *) item1;
    AjPScophit scophit2 = *(AjOScophit *const *) item2;

    dmxTraceScophit(scophit1, "CompAcc scophit1");
    dmxTraceScophit(scophit2, "CompAcc scophit2");

    return ajStrCmpS(scophit1->Acc, scophit2->Acc);
}




/* @func ajDmxScophitCompSunid ************************************************
**
** Function to compare AJAX SCOP Hit objects by Sunid_Family member.
** Usually called by ajListSort.
**
** @param [r] item1 [const void*] AJAX SCOP Hit address 1
** @param [r] item2 [const void*] AJAX SCOP Hit address 2
** @see ajListSort
**
** @return [ajint] -1 if Sunid_Family1 < Sunid_Family2, +1 if the
** Sunid_Family2 should sort first. 0 if they are identical.
**
** @release 2.9.0
** @@
******************************************************************************/

ajint ajDmxScophitCompSunid(const void *item1, const void *item2)
{
    AjPScophit scophit1 = *(AjOScophit *const *) item1;
    AjPScophit scophit2 = *(AjOScophit *const *) item2;

    if(scophit1->Sunid_Family < scophit2->Sunid_Family)
        return -1;
    else if(scophit1->Sunid_Family == scophit2->Sunid_Family)
        return 0;

    return 1;
}




/* @func ajDmxScophitCompSpr **************************************************
**
** Function to compare AJAX SCOP Hit objects by Spr member.
** Usually called by ajListSort.
**
** @param [r] item1 [const void*] AJAX SCOP Hit address 1
** @param [r] item2 [const void*] AJAX SCOP Hit address 2
** @see ajListSort
**
** @return [ajint] -1 if Spr1 should sort before Spr2,
**                 +1 if the Spr2 should sort first.
**                  0 if they are identical in length and content.
**
** @release 2.9.0
** @@
******************************************************************************/

ajint ajDmxScophitCompSpr(const void *item1, const void *item2)
{
    AjPScophit scophit1 = *(AjOScophit *const *) item1;
    AjPScophit scophit2 = *(AjOScophit *const *) item2;

    return ajStrCmpS(scophit1->Spr, scophit2->Spr);
}




/* @func ajDmxScophitCompEnd **************************************************
**
** Function to compare AJAX SCOP Hit objects by End member.
** Usually called by ajListSort.
**
** @param [r] item1 [const void*] AJAX SCOP Hit address 1
** @param [r] item2 [const void*] AJAX SCOP Hit address 2
** @see ajListSort
**
** @return [ajint] -1 if End1 should sort before End2, +1 if the End2
** should sort first. 0 if they are identical.
**
** @release 2.9.0
** @@
******************************************************************************/

ajint ajDmxScophitCompEnd(const void *item1, const void *item2)
{
    AjPScophit scophit1 = *(AjOScophit *const *) item1;
    AjPScophit scophit2 = *(AjOScophit *const *) item2;

    dmxTraceScophit(scophit1, "CompEnd scophit1");
    dmxTraceScophit(scophit2, "CompEnd scophit2");

    if(scophit1->End < scophit2->End)
        return -1;
    else if(scophit1->End == scophit2->End)
        return 0;

    return 1;
}




/* @func ajDmxScophitCompStart ************************************************
**
** Function to compare AJAX SCOP Hit objects by Start member.
** Usually called by ajListSort.
**
** @param [r] item1 [const void*] AJAX SCOP Hit address 1
** @param [r] item2 [const void*] AJAX SCOP Hit address 2
** @see ajListSort
**
** @return [ajint] -1 if Start1 should sort before Start2, +1 if the Start2
** should sort first. 0 if they are identical.
**
** @release 2.9.0
** @@
******************************************************************************/

ajint ajDmxScophitCompStart(const void *item1, const void *item2)
{
    AjPScophit scophit1 = *(AjOScophit *const *) item1;
    AjPScophit scophit2 = *(AjOScophit *const *) item2;

    dmxTraceScophit(scophit1, "CompStart scophit1");
    dmxTraceScophit(scophit2, "CompStart scophit2");

    if(scophit1->Start < scophit2->Start)
        return -1;
    else if(scophit1->Start == scophit2->Start)
        return 0;

    return 1;
}




/* @func ajDmxScophitCompFam **************************************************
**
** Function to compare AJAX SCOP Hit objects by Family member.
** Usually called by ajListSort.
**
** @param [r] item1 [const void*] AJAX SCOP Hit address 1
** @param [r] item2 [const void*] AJAX SCOP Hit address 2
** @see ajListSort
**
** @return [ajint] -1 if Family1 should sort before Family2, +1 if the
** Family2 should sort first. 0 if they are identical.
**
** @release 2.9.0
** @@
******************************************************************************/

ajint ajDmxScophitCompFam(const void *item1, const void *item2)
{
    AjPScophit scophit1 = *(AjOScophit *const *) item1;
    AjPScophit scophit2 = *(AjOScophit *const *) item2;

    return ajStrCmpS(scophit1->Family, scophit2->Family);
}




/* @func ajDmxScophitCompSfam *************************************************
**
** Function to compare AJAX SCOP Hit objects by Superfamily member.
** Usually called by ajListSort.
**
** @param [r] item1 [const void*] AJAX SCOP Hit address 1
** @param [r] item2 [const void*] AJAX SCOP Hit address 2
** @see ajListSort
**
** @return [ajint] -1 if Superfamily1 should sort before Superfamily2, +1 if
** the Superfamily2 should sort first. 0 if they are identical.
**
** @release 2.9.0
** @@
******************************************************************************/

ajint ajDmxScophitCompSfam(const void *item1, const void *item2)
{
    AjPScophit scophit1 = *(AjOScophit *const *) item1;
    AjPScophit scophit2 = *(AjOScophit *const *) item2;

    return ajStrCmpS(scophit1->Superfamily, scophit2->Superfamily);
}




/* @func ajDmxScophitCompClass ************************************************
**
** Function to compare AJAX SCOP Hit objects by Class member.
** Usually called by ajListSort.
**
** @param [r] item1 [const void*] AJAX SCOP Hit address 1
** @param [r] item2 [const void*] AJAX SCOP Hit address 2
** @see ajListSort
**
** @return [ajint] -1 if Class1 should sort before Class2, +1 if the Class2
** should sort first. 0 if they are identical.
**
** @release 3.0.0
** @@
******************************************************************************/

ajint ajDmxScophitCompClass(const void *item1, const void *item2)
{
    AjPScophit scophit1 = *(AjOScophit *const *) item1;
    AjPScophit scophit2 = *(AjOScophit *const *) item2;

    return ajStrCmpS(scophit1->Class, scophit2->Class);
}




/* @func ajDmxScophitCompFold *************************************************
**
** Function to compare AJAX SCOP Hit objects by Fold member.
** Usually called by ajListSort.
**
** @param [r] item1 [const void*] AJAX SCOP Hit address 1
** @param [r] item2 [const void*] AJAX SCOP Hit address 2
** @see ajListSort
**
** @return [ajint] -1 if Fold1 should sort before Fold2, +1 if the Fold2
** should sort first. 0 if they are identical.
**
** @release 2.9.0
** @@
******************************************************************************/

ajint ajDmxScophitCompFold(const void *item1, const void *item2)
{
    AjPScophit scophit1 = *(AjOScophit *const *) item1;
    AjPScophit scophit2 = *(AjOScophit *const *) item2;

    return ajStrCmpS(scophit1->Fold, scophit2->Fold);
}




/* ======================================================================= */
/* ========================== Input & Output ============================= */
/* ======================================================================= */




/* @func ajDmxScopalgGetseqs **************************************************
**
** Read an AJAX SCOP Alignment object and write an array of AJAX Strong objects
** containing the sequences without gaps.
**
** @param [r] scopalg [const AjPScopalg] AJAX SCOP Alignment
** @param [w] array [AjPStr**] Array of AJAX String objects
**
** @return [ajuint] Number of sequences read
**
** @release 2.9.0
** @@
******************************************************************************/

ajuint ajDmxScopalgGetseqs(const AjPScopalg scopalg, AjPStr **array)
{
    ajuint i = 0U;

    if(!scopalg || !array)
    {
        ajWarn("Null args passed to ajDmxScopalgGetseqs");
        return 0;
    }

    *array = (AjPStr *) AJCALLOC0(scopalg->Number, sizeof (AjPStr));

    for(i = 0U; i < scopalg->Number; ++i)
    {
        (*array)[i] = ajStrNew();

        ajStrAssignS(&((*array)[i]), scopalg->Seqs[i]);

        ajStrRemoveGap(&((*array)[i]));

    }

    return scopalg->Number;
}




/* @func ajDmxScophitsWrite ***************************************************
**
** Write contents of an AJAX List of AJAX SCOP Hit objects to an AJAX File in
** EMBL-like format.
** Text for Class, Architecture, Topology, Fold, Superfamily and Family
** is only written if the text is available.
**
** @param [w] outf [AjPFile] Output file stream
** @param [r] scophits [const AjPList] AJAX List of AJAX SCOP Hit objects
**
** @return [AjBool] True on success
**
** @release 2.9.0
** @@
******************************************************************************/

AjBool ajDmxScophitsWrite(AjPFile outf, const AjPList scophits)
{
    AjIList iter = NULL;

    AjPScophit scophit = NULL;

    AjPSeqout outseq = NULL;

    iter = ajListIterNewread(scophits);

    while((scophit = (AjPScophit) ajListIterGet(iter)))
    {
        if(!scophit)
            return ajFalse;

        if(scophit->Type == ajEDomainTypeSCOP)
            ajFmtPrintF(outf, "TY   SCOP\nXX\n");
        else if(scophit->Type == ajEDomainTypeCATH)
            ajFmtPrintF(outf, "TY   CATH\nXX\n");

        if(MAJSTRGETLEN(scophit->Dom))
        {
            ajFmtPrintF(outf, "%-5s%S\n", "DO", scophit->Dom);
            ajFmtPrintF(outf, "XX\n");
        }

        if(MAJSTRGETLEN(scophit->Class))
        {
            ajFmtPrintF(outf, "CL   %S\n", scophit->Class);
            ajFmtPrintF(outf, "XX\n");
        }

        if(MAJSTRGETLEN(scophit->Architecture))
        {
            ajFmtPrintF(outf, "AR   %S\n", scophit->Architecture);
            ajFmtPrintF(outf, "XX\n");
        }

        if(MAJSTRGETLEN(scophit->Topology))
        {
            ajFmtPrintF(outf, "TP   %S\n", scophit->Topology);
            ajFmtPrintF(outf, "XX\n");
        }

        if(MAJSTRGETLEN(scophit->Fold))
        {
            ajFmtPrintSplit(outf, scophit->Fold, "FO   ", 75, " \t\n\r");
            ajFmtPrintF(outf, "XX\n");

#if AJFALSE
            ajFmtPrintSplit(outf, scophit->Fold, "XX\nFO   ", 75,
                            " \t\n\r");
            ajFmtPrintF(outf, "XX\n");
#endif /* AJFALSE */
        }

        if(MAJSTRGETLEN(scophit->Superfamily))
        {
            ajFmtPrintSplit(outf, scophit->Superfamily, "SF   ", 75,
                            " \t\n\r");
            ajFmtPrintF(outf, "XX\n");

#if AJFALSE
            ajFmtPrintSplit(outf, scophit->Superfamily, "XX\nSF   ", 75,
                            " \t\n\r");
            ajFmtPrintF(outf, "XX\n");
#endif /* AJFALSE */
        }

        if(MAJSTRGETLEN(scophit->Family))
        {
            ajFmtPrintSplit(outf, scophit->Family,"FA   ", 75, " \t\n\r");
            ajFmtPrintF(outf, "XX\n");

#if AJFALSE
            ajFmtPrintSplit(outf, scophit->Family, "XX\nFA   ", 75,
                            " \t\n\r");
            ajFmtPrintF(outf, "XX\n");
#endif
        }

        if(MAJSTRGETLEN(scophit->Family))
        {
            ajFmtPrintF(outf, "XX\nSI   %d\n", scophit->Sunid_Family);
            ajFmtPrintF(outf, "XX\n");
        }

#if AJFALSE
        if(MAJSTRGETLEN(scophit->Typeobj))
            ajFmtPrintF(outf, "%-5s%S\n", "TY", scophit->Typeobj);
#endif /* AJFALSE */
        ajFmtPrintF(outf, "XX\n");
        ajFmtPrintF(outf, "%-5s%.5f\n", "SC", scophit->Score);
        ajFmtPrintF(outf, "XX\n");

        ajFmtPrintF(outf, "%-5s%.3e\n", "PV", scophit->Pval);
        ajFmtPrintF(outf, "XX\n");

        ajFmtPrintF(outf, "%-5s%.3e\n", "EV", scophit->Eval);
        ajFmtPrintF(outf, "XX\n");

        if(MAJSTRGETLEN(scophit->Group))
        {
            ajFmtPrintF(outf, "%-5s%S\n", "GP", scophit->Group);
            ajFmtPrintF(outf, "XX\n");
        }

        ajFmtPrintF(outf, "%-5s%S\n", "AC", scophit->Acc);
        ajFmtPrintF(outf, "XX\n");

        if(MAJSTRGETLEN(scophit->Spr))
        {
            ajFmtPrintF(outf, "%-5s%S\n", "SP", scophit->Spr);
            ajFmtPrintF(outf, "XX\n");
        }

        ajFmtPrintF(outf, "%-5s%d START; %d END;\n", "RA",
                    scophit->Start, scophit->End);
        ajFmtPrintF(outf, "XX\n");
        outseq = ajSeqoutNewFile(outf);
        ajSeqoutDumpSwisslike(outseq, scophit->Seq, "SQ");
        ajSeqoutDel(&outseq);
        ajFmtPrintF(outf, "XX\n");

        ajFmtPrintF(outf, "//\n");
    }

    ajListIterDel(&iter);

    return ajTrue;
}




/* @func ajDmxScophitsWriteFasta **********************************************
**
** Write contents of an AJAX List of AJAX SCOP Hit objects to an AJAX File
** in DHF format.
** Text for Class, Archhitecture, Topology, Fold, Superfamily and Family
** is only written if the text is available.
**
** @param [w] outf [AjPFile] AJAX Output File stream
** @param [r] scophits [const AjPList] AJAX List of AJAX SCOP Hit objects
**
** @return [AjBool] True on success
**
** @release 3.0.0
** @@
******************************************************************************/

AjBool ajDmxScophitsWriteFasta(AjPFile outf, const AjPList scophits)
{

    AjIList iter = NULL;

    AjPScophit scophit = NULL;

    iter = ajListIterNewread(scophits);

    while((scophit = (AjPScophit) ajListIterGet(iter)))
    {
        if(!scophit)
            return ajFalse;

        ajFmtPrintF(outf, "> ");

        if(MAJSTRGETLEN(scophit->Acc))
            ajFmtPrintF(outf, "%S^", scophit->Acc);
        else
            ajFmtPrintF(outf, ".^");

        if(MAJSTRGETLEN(scophit->Spr))
            ajFmtPrintF(outf, "%S^", scophit->Spr);
        else
            ajFmtPrintF(outf, ".^");

        ajFmtPrintF(outf, "%d^%d^", scophit->Start, scophit->End);

        if((scophit->Type == ajEDomainTypeSCOP))
            ajFmtPrintF(outf, "SCOP^");
        else if ((scophit->Type == ajEDomainTypeCATH))
            ajFmtPrintF(outf, "CATH^");
        else
            ajFmtPrintF(outf, ".^");

        if(MAJSTRGETLEN(scophit->Dom))
            ajFmtPrintF(outf, "%S^", scophit->Dom);
        else
            ajFmtPrintF(outf, ".^");

        ajFmtPrintF(outf,"%d^", scophit->Sunid_Family);

        if(MAJSTRGETLEN(scophit->Class))
            ajFmtPrintF(outf,"%S^",scophit->Class);
        else
            ajFmtPrintF(outf, ".^");

        if(MAJSTRGETLEN(scophit->Architecture))
            ajFmtPrintF(outf,"%S^",scophit->Architecture);
        else
            ajFmtPrintF(outf, ".^");

        if(MAJSTRGETLEN(scophit->Topology))
            ajFmtPrintF(outf,"%S^",scophit->Topology);
        else
            ajFmtPrintF(outf, ".^");

        if(MAJSTRGETLEN(scophit->Fold))
            ajFmtPrintF(outf,"%S^",scophit->Fold);
        else
            ajFmtPrintF(outf, ".^");

        if(MAJSTRGETLEN(scophit->Superfamily))
            ajFmtPrintF(outf,"%S^",scophit->Superfamily);
        else
            ajFmtPrintF(outf, ".^");

        if(MAJSTRGETLEN(scophit->Family))
            ajFmtPrintF(outf,"%S^",scophit->Family);
        else
            ajFmtPrintF(outf, ".^");

        if(MAJSTRGETLEN(scophit->Model))
            ajFmtPrintF(outf, "%S^", scophit->Model);
        else
            ajFmtPrintF(outf, ".^");

        ajFmtPrintF(outf, "%.2f^", scophit->Score);

        ajFmtPrintF(outf, "%.3e^", scophit->Pval);

        ajFmtPrintF(outf, "%.3e", scophit->Eval);

        ajFmtPrintF(outf, "\n");
        ajFmtPrintF(outf, "%S\n", scophit->Seq);

    }

    ajListIterDel(&iter);

    return ajTrue;
}




/* @func ajDmxScophitReadFasta ************************************************
**
** Read an AJAX SCOP Hit object from a file in extended FASTA format
** (see documentation for the DOMAINATRIX "seqsearch" application).
**
** @param [u] inf [AjPFile] AJAX Input file stream
**
** @return [AjPScophit] AJAX SCOP Hit object, or NULL if the file was not in
** extended FASTA (DHF) format (indicated by a token count of the the lines
** beginning with '>').
**
** @release 3.0.0
** @@
******************************************************************************/

AjPScophit ajDmxScophitReadFasta(AjPFile inf)
{
    AjPScophit scophit  = NULL;    /* Current hit */
    AjBool    donefirst = ajFalse; /* First '>' line has been read */
    ajint     ntok      = 0;       /* No. tokens in a line */
    const AjPStr token  = NULL;
    AjPStr    line      = NULL;    /* Line of text */
    AjPStr    subline   = NULL;
    AjPStr    type      = NULL;

    /* Allocate strings */
    line     = ajStrNew();
    subline  = ajStrNew();
    type     = ajStrNew();

    while((ajReadlineTrim(inf, &line)))
    {
        if(ajStrPrefixC(line, ">"))
        {
            /* Process the last hit */
            if(donefirst)
            {
                ajStrRemoveWhite(&scophit->Seq);
                ajStrDel(&line);
                ajStrDel(&subline);
                ajStrDel(&type);

                return scophit;
            }
            else
                scophit = ajDmxScophitNew();

            /* Check line has correct no. of tokens and allocate Hit */
            ajStrAssignSubS(&subline, line, 1, -1);

            if( (ntok=ajStrParseCountC(subline, "^")) != 17)
            {
                ajWarn("Wrong no. (%d) of tokens for a DHF file on line %S\n",
                       ntok, line);
                ajStrDel(&line);
                ajStrDel(&subline);
                ajDmxScophitDel(&scophit);
                ajStrDel(&type);
                return NULL;
            }

            /* Acc */
            token = ajStrParseC(subline, "^");
            ajStrAssignS(&scophit->Acc, token);
            ajStrTrimWhite(&scophit->Acc);

            if(ajStrMatchC(scophit->Acc, "."))
                ajStrSetClear(&scophit->Acc);

            /* Spr */
            token = ajStrParseC(NULL, "^");
            ajStrAssignS(&scophit->Spr, token);

            if(ajStrMatchC(scophit->Spr, "."))
                ajStrSetClear(&scophit->Spr);

            /* Start */
            token = ajStrParseC(NULL, "^");
            ajFmtScanS(token, "%d", &scophit->Start);

            /* End */
            token = ajStrParseC(NULL, "^");
            ajFmtScanS(token, "%d", &scophit->End);

            /* Type */
            token = ajStrParseC(NULL, "^");
            ajStrAssignS(&type, token);

            if(ajStrMatchC(type, "SCOP"))
                scophit->Type = ajEDomainTypeSCOP;
            else if(ajStrMatchC(type, "CATH"))
                scophit->Type = ajEDomainTypeCATH;

            /* Dom */
            token = ajStrParseC(NULL, "^");
            ajStrAssignS(&scophit->Dom, token);

            if(ajStrMatchC(scophit->Dom, "."))
                ajStrSetClear(&scophit->Dom);

            /* Domain identifier */
            token = ajStrParseC(NULL, "^");
            ajFmtScanS(token, "%d", &scophit->Sunid_Family);

            token = ajStrParseC(NULL, "^");
            ajStrAssignS(&scophit->Class, token);

            if(ajStrMatchC(scophit->Class, "."))
                ajStrSetClear(&scophit->Class);

            token = ajStrParseC(NULL, "^");
            ajStrAssignS(&scophit->Architecture, token);

            if(ajStrMatchC(scophit->Architecture, "."))
                ajStrSetClear(&scophit->Architecture);

            token = ajStrParseC(NULL, "^");
            ajStrAssignS(&scophit->Topology, token);
            if(ajStrMatchC(scophit->Topology, "."))
                ajStrSetClear(&scophit->Topology);

            token = ajStrParseC(NULL, "^");
            ajStrAssignS(&scophit->Fold, token);

            if(ajStrMatchC(scophit->Fold, "."))
                ajStrSetClear(&scophit->Fold);

            token = ajStrParseC(NULL, "^");
            ajStrAssignS(&scophit->Superfamily, token);

            if(ajStrMatchC(scophit->Superfamily, "."))
                ajStrSetClear(&scophit->Superfamily);

            token = ajStrParseC(NULL, "^");
            ajStrAssignS(&scophit->Family, token);

            if(ajStrMatchC(scophit->Family, "."))
                ajStrSetClear(&scophit->Family);

            token = ajStrParseC(NULL, "^");
            ajStrAssignS(&scophit->Model, token);

            if(ajStrMatchC(scophit->Model, "."))
                ajStrSetClear(&scophit->Model);

            token = ajStrParseC(NULL, "^");
            ajFmtScanS(token, "%f", &scophit->Score);

            token = ajStrParseC(NULL, "^");
            ajFmtScanS(token, "%f", &scophit->Pval);

            token = ajStrParseC(NULL, "^");
            ajFmtScanS(token, "%f", &scophit->Eval);

            donefirst = ajTrue;
        }
        else
        {
            if(scophit)
                ajStrAppendS(&scophit->Seq, line);
        }
    }

    /* EOF therefore process last hit */
    if(donefirst)
    {
        ajStrRemoveWhite(&scophit->Seq);
        ajStrDel(&line);
        ajStrDel(&subline);
        ajStrDel(&type);
        return scophit;
    }

    ajStrDel(&line);
    ajStrDel(&subline);
    ajStrDel(&type);
    ajDmxScophitDel(&scophit);

    return NULL;
}




/* @func ajDmxScopalgWrite ****************************************************
**
** Write an AJAX SCOP Alignment object to an AJAX FIle in EMBOSS simple
** multiple sequence format (same as that used by clustal) annotated with
** domain classification as below (records are for SCOP domains in this
** example):
**
**
** # TY   SCOP
** # XX
** # CL   Alpha and beta proteins (a+b)
** # XX
** # FO   Phospholipase D/nuclease
** # XX
** # SF   Phospholipase D/nuclease
** # XX
** # FA   Phospholipase D
** # XX
** # SI   64391
** # XX
** d1f0ia1    1 AATPHLDAVEQTLRQVSPGLEGDVWERTSGNKLDGSAADPSDWLLQTP-GCWGDDKC    50
** d1f0ia2    1 -----------------------------NVPV---------IAVG-GLG---VGIK    15
**
** d1f0ia1   51 A-------------------------------D-RVGTKRLLAKMTENIGNATRTVD    75
** d1f0ia2   16 DVDPKSTFRPDLPTASDTKCVVGLHDNTNADRDYDTV-NPEESALRALVASAKGHIE    65
**
**
**
** @param [r] scopalg [const AjPScopalg] AJAX SCOP Alignment
** @param [u] outf [AjPFile] AJAX Output file stream
**
** @return [AjBool] True on success (an alignment was written)
**
** @release 2.9.0
** @@
******************************************************************************/

AjBool ajDmxScopalgWrite(const AjPScopalg scopalg, AjPFile outf)
{
    ajuint i = 0U;
    ajuint j = 0U;

    ajint tmp_wid  = 0;     /* Temp. variable for width */
    ajint code_wid = 0;     /* Max. code width +1 */
    ajint seq_wid  = 0;     /* Width of alignment rounded up to nearest 60 */
    ajuint nblk    = 0U;    /* Number of blocks of alignment in output */

    AjPStr tmp_seq = NULL;  /* Temp. variable for sequence */
    AjPStr nogap   = NULL;  /* Temp. variable for sequence w/o gaps */

    ajint  len_nogap = 0;   /* Length of no_gap */
    ajint pos      = 0;     /* House-keeping */

    ajint start    = 0;     /* Start position of sequence fragment wrt full
                               length alignment */
    ajuint end     = 0;     /* End position of sequence fragment wrt full
                               length alignment */
    AjPInt    idx  = NULL;  /* Index */

    idx = ajIntNewRes(scopalg->Number);

    for(i = 0U; i < scopalg->Number; i++)
        ajIntPut(&idx, scopalg->Number, 1);

    /* Write SCOP classification records to file */
    if(scopalg->Type == ajEDomainTypeSCOP)
    {
        ajFmtPrintF(outf,"# TY   SCOP\n# XX\n");
        ajFmtPrintF(outf,"# CL   %S\n# XX\n", scopalg->Class);

        ajFmtPrintSplit(outf, scopalg->Fold, "# FO   ", 75, " \t\n\r");
        ajFmtPrintF(outf, "# XX\n");
        ajFmtPrintSplit(outf, scopalg->Superfamily, "# SF   ", 75, " \t\n\r");
        ajFmtPrintF(outf, "# XX\n");
        ajFmtPrintSplit(outf,scopalg->Family, "# FA   ", 75, " \t\n\r");
        ajFmtPrintF(outf, "# XX\n");

#if AJFALSE
        ajFmtPrintSplit(outf,scopalg->Fold, "\nXX\n# FO   ", 75,
                        " \t\n\r");
        ajFmtPrintSplit(outf,scopalg->Superfamily, "# XX\n# SF   ", 75,
                        " \t\n\r");
        ajFmtPrintSplit(outf,scopalg->Family, "# XX\n# FA   ", 75,
                        " \t\n\r");
        ajFmtPrintF(outf,"# XX\n");
#endif /* AJFALSE */

        ajFmtPrintF(outf, "# SI   %d\n# XX", scopalg->Sunid_Family);
    }
    else
    {
        ajFmtPrintF(outf, "# TY   CATH\n# XX\n");
        ajFmtPrintF(outf, "# CL   %S\n# XX\n", scopalg->Class);

        ajFmtPrintSplit(outf, scopalg->Architecture, "# AR   ", 75, " \t\n\r");
        ajFmtPrintF(outf, "# XX\n");
        ajFmtPrintSplit(outf, scopalg->Topology, "# TP   ", 75, " \t\n\r");
        ajFmtPrintF(outf, "# XX\n");
        ajFmtPrintSplit(outf, scopalg->Superfamily, "# SF   ", 75, " \t\n\r");
        ajFmtPrintF(outf, "# XX\n");

#if AJFALSE
        ajFmtPrintSplit(outf, scopalg->Architecture,
                        "\nXX\n# AR   ", 75, " \t\n\r");
        ajFmtPrintSplit(outf, scopalg->Topology, "# XX\n# TP   ", 75,
                        " \t\n\r");
        ajFmtPrintSplit(outf, scopalg->Superfamily, "# XX\n# SF   ", 75,
                        " \t\n\r");
        ajFmtPrintF(outf, "# XX\n");
#endif /* AJFALSE */

        ajFmtPrintF(outf, "# SI   %d\n# XX", scopalg->Sunid_Family);
    }

    /* Find max. width of code, and add 1 to it for 1 whitespace */
    for(i = 0U; i < scopalg->Number; i++)
        if((tmp_wid = MAJSTRGETLEN(scopalg->Codes[i])) > code_wid)
            code_wid = tmp_wid;
    code_wid++;

    /* Calculate no. of blocks in alignment */
    seq_wid = ajRound(scopalg->Width, 50);
    nblk    = (ajint) (seq_wid / 50);


    /* Print out sequence in blocks */
    for(i = 0U; i < nblk; i++)
    {
        start = i * 50;
        end   = start + 49;

        if(end>=scopalg->Width)
            end = scopalg->Width - 1;

        ajFmtPrintF(outf, "\n");
        for(j = 0U; j < scopalg->Number; j++)
        {
            ajStrAssignSubS(&tmp_seq, scopalg->Seqs[j], start, end);
            ajStrAssignS(&nogap, tmp_seq);
            /* Remove gap characters */
            ajStrRemoveSetC(&nogap, " -");
            len_nogap = MAJSTRGETLEN(nogap);

            pos = ajIntGet(idx, j);

            ajFmtPrintF(outf, "%*S%7d %-50S%7d\n",
                        code_wid,
                        pos,
                        scopalg->Codes[j],
                        tmp_seq,
                        pos + len_nogap - 1);

            ajIntPut(&idx, j, pos + len_nogap);
        }
    }

    ajIntDel(&idx);

    return ajTrue;
}




/* @func ajDmxScopalgWriteClustal *********************************************
**
** Write an AJAX SCOP Alignment object to an AJAX File in CLUSTAL format
** (just the alignment without the domain classification information).
**
** @param [r] scopalg [const AjPScopalg] AJAX SCOP Alignment
** @param [u] outf [AjPFile] AJAX Outfile file
**
** @return [AjBool] True on success (a file has been written)
**
** @release 2.9.0
** @@
******************************************************************************/

AjBool ajDmxScopalgWriteClustal(const AjPScopalg scopalg, AjPFile outf)
{
    ajuint i = 0U;

    if(!scopalg)
    {
        ajWarn("Null args passed to ajDmxScopalgWriteClustal");
        return ajFalse;
    }

    /* remove i from the print statement before committing */
    ajFmtPrintF(outf, "CLUSTALW\n\n");
    ajFmtPrintF(outf, "\n");

    for(i = 0U; i < scopalg->Number; ++i)
        ajFmtPrintF(outf, "%S_%d   %S\n", scopalg->Codes[i], i,
                    scopalg->Seqs[i]);

    ajFmtPrintF(outf, "\n");
    ajFmtPrintF(outf, "\n");

    return ajTrue;
}




/* @func ajDmxScopalgWriteClustal2 ********************************************
**
** Write an AJAX SCOP Alignment object to an AJAX File in CLUSTAL format
** (just the alignment without the domain classification information).
**
** @param [r] scopalg [const AjPScopalg] AJAX SCOP Alignment
** @param [u] outf [AjPFile] AJAX Outfile file
**
** @return [AjBool] True on success (a file has been written)
**
** @release 2.9.0
** @@
******************************************************************************/

AjBool ajDmxScopalgWriteClustal2(const AjPScopalg scopalg, AjPFile outf)
{
    ajuint i = 0U;

    if(!scopalg)
    {
        ajWarn("Null args passed to ajDmxScopalgWriteClustal2");
        return ajFalse;
    }

    /* remove i from the print statement before committing */
    ajFmtPrintF(outf, "\n");

    for(i = 0; i < scopalg->Number; i++)
        ajFmtPrintF(outf, "%S_%d   %S\n", scopalg->Codes[i], i,
                    scopalg->Seqs[i]);

    ajFmtPrintF(outf,"\n");

    return ajTrue;
}





/* @func ajDmxScopalgWriteFasta ***********************************************
**
** Write an AJAX SCOP Alignment object to an AJAX File in FASTA format
** (just the alignment without the domain classification information).
**
** @param [r] scopalg [const AjPScopalg] AJAX SCOP Alignment
** @param [u] outf [AjPFile] AJAX Outfile file
**
** @return [AjBool] True on success (a file has been written)
**
** @release 2.9.0
** @@
******************************************************************************/

AjBool ajDmxScopalgWriteFasta(const AjPScopalg scopalg, AjPFile outf)
{
    ajuint i = 0U;

    if(!scopalg)
    {
        ajWarn("Null args passed to ajDmxScopalgWriteFasta");
        return ajFalse;
    }

#if AJFALSE
    /* remove i from the print statement before committing */
    ajFmtPrintF(outf, "CLUSTALW\n\n");
    ajFmtPrintF(outf, "\n");
#endif /* AJFALSE */

    for(i = 0U; i < scopalg->Number; i++)
        ajFmtPrintF(outf, ">%S_%d\n%S\n", scopalg->Codes[i], i,
                    scopalg->Seqs[i]);

    ajFmtPrintF(outf,"\n");
    ajFmtPrintF(outf,"\n");

    return ajTrue;
}




/* @section Miscellaneous *****************************************************
**
** These functions may have diverse functions that do not fit into the other
** categories.
**
******************************************************************************/




/* @func ajDmxScopSeqFromSunid ************************************************
**
** Write a sequence corresponding to an AJAX SCOP domain given a Sunid for the
** domain. The sequence is taken from one of an AJAX List of AJAX SCOP objects
** that is provided. The SWISS-Prot sequence is taken in priority over the PDB
** sequence.
**
** @param [r] identifier [ajint] Search term
** @param [w] seq [AjPStr*] Result sequence
** @param [r] scops [const AjPList] Sorted AJAX List of AJAX SCOP objects
**
** @return [AjBool] True if a SWISS-Prot identifier code was found for the
**                  PDB code.
**
** @release 2.9.0
** @@
******************************************************************************/

AjBool ajDmxScopSeqFromSunid(ajint identifier, AjPStr *seq,
                             const AjPList scops)
{
    ajuint dim = 0U;      /* Size of array */
    ajint  idx = 0;       /* Index into array for the Pdb code */

    AjPScop *array = NULL;  /* Array derived from list */

    if(!identifier || !scops)
    {
        ajWarn("Bad args passed to ajDmxScopSeqFromSunid");
        return ajFalse;
    }

    dim = (ajuint) ajListToarray(scops, (void ***) &array);

    if(!dim)
    {
        ajWarn("Empty list passed to ajDmxScopSeqFromSunid");
        return ajFalse;
    }

    if((idx = ajScopArrFindSunid(array, dim, identifier)) == -1)
    {
        AJFREE(array);
        return ajFalse;
    }

    /* swissprot sequence has priority */
    if((ajStrGetLen(array[idx]->SeqSpr)) == 0)
        ajStrAssignS(seq, array[idx]->SeqPdb);
    else
        ajStrAssignS(seq, array[idx]->SeqSpr);

    AJFREE(array);

    return ajTrue;
}




/* @func ajDmxExit ************************************************************
**
** Cleanup of Dmx function internals.
**
** @return [void]
**
** @release 6.1.0
** @@
******************************************************************************/

void ajDmxExit(void)
{
    ajStrDel(&dmxGStrline);
    ajStrDel(&dmxGStrtype);
    ajStrDel(&dmxGStrclass);
    ajStrDel(&dmxGStrfold);
    ajStrDel(&dmxGStrsuper);
    ajStrDel(&dmxGStrfamily);
    ajStrDel(&dmxGStrarch);
    ajStrDel(&dmxGStrtopo);
    ajStrDel(&dmxGStrpostsim);
    ajStrDel(&dmxGStrposttmp);
    ajStrDel(&dmxGStrposisim);
    ajStrDel(&dmxGStrpositmp);
    ajStrDel(&dmxGStrseq1);
    ajStrDel(&dmxGStrcodetmp);

    return;
}




/* @func ajDmxDummyFunction ***************************************************
**
** Dummy function to catch all unused functions defined in the ajdmx
** source file.
**
** @return [void]
**
**
** @release 2.9.0
******************************************************************************/

void ajDmxDummyFunction(void)
{
    return;
}




/* @funcstatic dmxTraceScophit ************************************************
**
** Reports internals of a SCOPhit object
**
** @param [r] scophit [const AjPScophit] AJAX SCOP Hit object
** @param [r] title [const char*] title
** @return [void]
**
** @release 4.1.0
******************************************************************************/

static void dmxTraceScophit(const AjPScophit scophit, const char *title)
{
    ajDebug("SCOPhit trace: %s\n", title);
    ajDebug("Type: %d\n", scophit->Type);
    ajDebug("Acc: '%S'\n", scophit->Acc);
    ajDebug("Start: %d\n", scophit->Start);
    ajDebug("End: %d\n", scophit->End);

    return;
}
