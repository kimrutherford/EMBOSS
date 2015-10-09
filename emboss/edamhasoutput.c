/** @source edamhasoutput
**
** Find EDAM ontology terms by has_output relationship
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




/* @prog edamhasoutput ********************************************************
**
** Find EDAM ontology 'data' (identifier) terms by keyword
**
******************************************************************************/

int main(int argc, char **argv)
{
    /* Variable declarations */
    AjPStr   query;
    AjPStr  *namespace = NULL;
    AjPOutfile  outfile = NULL;    
    AjBool sensitive = ajFalse;
    AjBool subclasses = ajFalse;
    AjBool obsolete = ajTrue;

    AjPObo obo = NULL;
    AjPObo oborelate = NULL;
    AjPObo obotest = NULL;
    AjPOboin oboin = NULL;
    AjPOboin oboinrelate = NULL;

    AjPList obolist = ajListNew();

    AjPStr oboqry = NULL;
    AjPStr oboqryrelate = NULL;
    AjPStr qrystr = NULL;
    AjPTable obotable = NULL;
    AjPTable nstable = NULL;
    AjPTable foundtable = NULL;
    const AjPStr name = NULL;

    AjPStrTok handle = NULL;
    ajuint i;
    ajuint imax = 3;
    
    const char* fields[] = {"id", "acc", "nam", "des"};
    
    /* ACD processing */
    embInit("edamhasoutput", argc, argv);

    query     = ajAcdGetString("identifier");
    namespace = ajAcdGetList("namespace");
    outfile   = ajAcdGetOutobo("outfile");
    sensitive = ajAcdGetBoolean("sensitive");
    subclasses = ajAcdGetBoolean("subclasses");
    obsolete = ajAcdGetBoolean("obsolete");
    
    oboin = ajOboinNew();
    obo = ajOboNew();
    oboinrelate = ajOboinNew();
    oborelate = ajOboNew();

    foundtable = ajTablestrNew(600);
    obotable = ajTablestrNew(600);
    nstable = ajTablestrNew(6);

    if(sensitive)
        imax++;

    for(i=0; namespace[i]; i++)
    {
        ajTablePut(nstable, namespace[i], (void*) 1);
    }
 
    handle = ajStrTokenNewC(query, ",");
    while(ajStrTokenNextParse(&handle, &qrystr))
    {
        for(i=0;i<imax;i++)
        {
            ajFmtPrintS(&oboqry, "edam-%s:%S", fields[i], qrystr);

            ajOboinQryS(oboin, oboqry);

            while(ajOboinRead(oboin, obo))
            {
                if(!obsolete && ajOboIsObsolete(obo))
                    continue;

                ajListPushAppend(obolist, ajOboNewObo(obo));
                if(subclasses)
                    ajOboGetTree(obo, obolist);

                ajDebug("%S '%S' %Lu\n",
                       qrystr, obo->Id, ajListGetLength(obolist));
                while(ajListGetLength(obolist))
                {
                    ajListPop(obolist, (void**) &obotest);
                    if(!obsolete && ajOboIsObsolete(obotest))
                    {
                        ajOboDel(&obotest);
                        continue;
                    }

                    if(!ajTableMatchS(obotable, obotest->Id))
                    {
                        ajFmtPrintS(&oboqryrelate, "edam-hasout:%S",
                                    obotest->Id);
                        ajOboinQryS(oboinrelate, oboqryrelate);
                        while(ajOboinRead(oboinrelate, oborelate))
                        {
                            if(!obsolete && ajOboIsObsolete(oborelate))
                                continue;

                            name = ajOboGetNamespace(oborelate);
                            if(ajTableMatchS(nstable, name))
                            {
                                if(!ajTableMatchS(foundtable, oborelate->Id))
                                {
                                    ajObooutWrite(outfile, oborelate);
                                    ajTablePut(foundtable,
                                               ajStrNewS(oborelate->Id),
                                               (void *) 1);
                                }
                            }
                        }
                    }
                    ajOboDel(&obotest);
                }
            }
        }
    }

    /* Memory clean-up and exit */

    ajListFree(&obolist);

    ajStrTokenDel(&handle);
    ajStrDel(&qrystr);
    ajTablestrFreeKey(&obotable);
    ajTablestrFreeKey(&foundtable);
    ajTableFree(&nstable);

    ajStrDel(&query);
    for(i=0; namespace[i]; i++)
        ajStrDel(&namespace[i]);
    AJFREE(namespace);

    ajStrDel(&oboqry);
    ajStrDel(&oboqryrelate);
    ajOboDel(&obo);
    ajOboDel(&oborelate);
    ajOboinDel(&oboin);
    ajOboinDel(&oboinrelate);

    ajOutfileClose(&outfile);
    
    embExit();

    return 0;
}


/* ==================================================================== */
/* ============================ functions ============================= */
/* ==================================================================== */

