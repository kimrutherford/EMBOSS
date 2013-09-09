/** @source goname
**
** Find gene ontology terms by identifier
**
** @author Copyright (C) 2010 Jon Ison / EMBOSS
** @version 1  First version</replaceable>
** @modified July 2010  Jon Ison First version</replaceable>
** @@
**
** This program is free software; you can redistribute it and/or
** modify it under the terms of the GNU General Public License
** as published by the Free Software Foundation; either version 2
** of the License, or (at your option) any later version.
** 
** This program is distributed in the hope that it will be useful,
** but WITHOUT ANY WARRANTY; without even the implied warranty of
** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
** GNU General Public License for more details.
** 
** You should have received a copy of the GNU General Public License
** along with this program; if not, write to the Free Software
** Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
********************************************************************/


/* ==================================================================== */
/* ========================== include files =========================== */
/* ==================================================================== */

#include "emboss.h"

/* Inclusion of system and local header files goes here */



/* ==================================================================== */
/* ============================ constants ============================= */
/* ==================================================================== */

/* #define and enum statements go here */



/* ==================================================================== */
/* ======================== global variables ========================== */
/* ==================================================================== */

/* Global variables definitions go here */



/* ==================================================================== */
/* ============================== data ================================ */
/* ==================================================================== */

/* Definition of datatypes go here */



/* ==================================================================== */
/* ==================== function prototypes =========================== */
/* ==================================================================== */

/* Function prototypes for public (external) functions go here */




/* @prog goname ***************************************************************
**
** Find GO ontology 'data' (identifier) terms by keyword
**
******************************************************************************/

int main(int argc, char **argv)
{
    /* Variable declarations */
    AjPStr   query;
    AjPStr  *namespace = NULL;
    AjPOutfile  outfile = NULL;    
    AjBool subclasses = ajTrue;
    AjBool obsolete = ajFalse;

    AjPObo obo = NULL;
    AjPObo obotest = NULL;
    AjPOboin oboin = NULL;

    AjPList obolist = NULL;

    AjPStr oboqry = NULL;
    AjPStr qrystr = NULL;
    AjPTable foundtable = NULL;
    AjPTable nstable = NULL;
    const AjPStr name = NULL;

    AjPStrTok handle = NULL;
    ajint i;
    ajuint ifound = 0;
    ajuint ikeep = 0;

    char wordchars[] =
        "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz"
        "0123456789*?";

    /* ACD processing */
    embInit("goname", argc, argv);

    query     = ajAcdGetString("query");
    namespace = ajAcdGetList("namespace");
    outfile   = ajAcdGetOutobo("outfile");
    subclasses = ajAcdGetBoolean("subclasses");
    obsolete = ajAcdGetBoolean("obsolete");
    
    /* Application logic */
    /* Check GO ontology (gene_ontology.*.obo) is installed indexed.
       Loop through queryable fields
        :Return list of GO ids with fields matching keyword(s)
       Merge lists of matching entries
       Write output file */

    oboin = ajOboinNew();
    obo = ajOboNew();

    obolist = ajListNew();

    foundtable = ajTablestrNew(600);
    nstable = ajTablestrNew(6);

    for(i=0; namespace[i]; i++)
    {
        ajTablePut(nstable, namespace[i], (void*) 1);
    }
 
    handle = ajStrTokenNewC(query, ",");
    while(ajStrTokenNextParse(handle, &qrystr))
    {
        if(ajStrIsCharsetC(qrystr, wordchars))
            ajFmtPrintS(&oboqry, "go-nam:%S", qrystr);
        else
            ajFmtPrintS(&oboqry, "go-acc:%S", qrystr);
        ajOboinQryS(oboin, oboqry);

        while(ajOboinRead(oboin, obo))
        {
            if(!obsolete && ajOboIsObsolete(obo))
                continue;

            ajListPushAppend(obolist, ajOboNewObo(obo));
            if(subclasses)
                ajOboGetTree(obo, obolist);

            while(ajListGetLength(obolist))
            {
                ajListPop(obolist, (void**) &obotest);
                if(!obsolete && ajOboIsObsolete(obotest))
                {
                    ajOboDel(&obotest);
                    continue;
                }

                ifound++;
                name = ajOboGetNamespace(obotest);

                if(ajTableMatchS(nstable, name))
                {
                    if(!ajTableMatchS(foundtable, obotest->Id))
                    {
                        ajObooutWrite(outfile, obotest);
                        ajTablePut(foundtable, ajStrNewS(obotest->Id),
                                   (void *) 1);
                        ikeep++;
                    }
                }
                ajOboDel(&obotest);
            }
        }
    }

    if(!ifound)
        ajErr("No matching terms");
    else if(!ikeep)
        ajErr("No terms in requested namespace");

    /* Memory clean-up and exit */

    ajOboDel(&obo);
    ajOboinDel(&oboin);

    ajListFree(&obolist);

    ajStrTokenDel(&handle);

    ajStrDel(&query);
    ajStrDel(&oboqry);
    ajStrDel(&qrystr);

    ajTablestrFreeKey(&foundtable);
    ajTableFree(&nstable);

    for(i=0; namespace[i]; i++)
        ajStrDel(&namespace[i]);
    AJFREE(namespace);

    ajOutfileClose(&outfile);
    
    embExit();

    return 0;
}


/* ==================================================================== */
/* ============================ functions ============================= */
/* ==================================================================== */

