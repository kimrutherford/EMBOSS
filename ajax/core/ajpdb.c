/****************************************************************************
**
** @source ajpdb.c 
**
** AJAX low-level functions for handling protein structural data.  
** For use with the Atom, Chain and Pdb objects defined in ajpdb.h
** Also for use with Hetent, Het, Vdwres, Vdwall, Cmap and Pdbtosp objects
** (also defined in ajpdb.h).
** Includes functions for reading and writing ccf (clean coordinate file)
** format.
** 
** @author Copyright (C) 2004 Jon Ison (jison@hgmp.mrc.ac.uk) 
** @version 1.0 
** @@
** 
** This library is free software; you can redistribute it and/or
** modify it under the terms of the GNU Library General Public
** License as published by the Free Software Foundation; either
** version 2 of the License, or (at your option) any later version.
** 
** This library is distributed in the hope that it will be useful,
** but WITHOUT ANY WARRANTY; without even the implied warranty of
** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
** Library General Public License for more details.
** 
** You should have received a copy of the GNU Library General Public
** License along with this library; if not, write to the
** Free Software Foundation, Inc., 59 Temple Place - Suite 330,
** Boston, MA  02111-1307, USA.
****************************************************************************/

/* ======================================================================= */
/* ============================ include files ============================ */
/* ======================================================================= */

#include "ajax.h"





/* ======================================================================= */
/* ============================ private data ============================= */
/* ======================================================================= */

#define CMAP_MODE_I   1
#define CMAP_MODE_C   2

static   AjPStr cmapStrline       = NULL;   /* Line of text     */
static   AjPStr cmapStrtemp_id    = NULL;   /* Temp. protein id */
static   AjPStr cmapStrtemp_domid = NULL;   /* Temp. domain id  */
static   AjPStr cmapStrtemp_ligid = NULL;   /* Temp. ligand id  */
static   AjPStr cmapStrdesc       = NULL;   /* Ligand description,  SITES output
					       only */
static   AjPStr cmapStrtype       = NULL;   /* Type of contact  */
static   AjPStr cmapStrtmpstr     = NULL;   /* Housekeeping */



/* ======================================================================= */
/* ================= Prototypes for private functions ==================== */
/* ======================================================================= */

static ajint  pdbSortPdbtospPdb(const void *ptr1, const void *ptr2);




/* ======================================================================= */
/* ========================== private functions ========================== */
/* ======================================================================= */




/* @funcstatic pdbSortPdbtospPdb **********************************************
**
** Function to sort Pdbtosp objects by Pdb element. Usually called by 
** ajPdbtospReadAllNew.
**
** @param [r] ptr1  [const void*] Pointer to AjOPdbtosp object 1
** @param [r] ptr2  [const void*] Pointer to AjOPdbtosp object 2
**
** @return [ajint] -1 if Pdb1 should sort before Pdb2,
**                 +1 if the Pdb2 should sort first. 
**                  0 if they are identical in length and content. 
** @@
****************************************************************************/

static ajint pdbSortPdbtospPdb(const void *ptr1, const void *ptr2)
{
    const AjPPdbtosp p = NULL;
    const AjPPdbtosp q = NULL;

    p = (*(AjPPdbtosp const *)ptr1);
    q = (*(AjPPdbtosp const *)ptr2);
    
    return ajStrCmpS(p->Pdb, q->Pdb);
}





/* ======================================================================= */
/* =========================== constructors ============================== */
/* ======================================================================= */




/* @section Constructors ****************************************************
**
** All constructors return a pointer to a new instance. It is the 
** responsibility of the user to first destroy any previous instance. The 
** target pointer does not need to be initialised to NULL, but it is good 
** programming practice to do so anyway.
**
****************************************************************************/




/* @func ajPdbtospReadAllRawNew ********************************************
**
** Reads the swissprot:pdb equivalence table available at URL (1)
**  (1) http://www.expasy.ch/cgi-bin/lists?pdbtosp.txt
** and returns the data as a list of Pdbtosp objects. 
**
** @param [u] inf [AjPFile] Input file  
**
** @return [AjPList] List of Pdbtosp objects. 
** @@
**
****************************************************************************/

AjPList       ajPdbtospReadAllRawNew(AjPFile inf)
{
    AjPList    ret     =NULL;   /* List of Pdbtosp objects to return */
    AjPPdbtosp tmp     =NULL;   /* Temp. pointer to Pdbtosp object */
    AjPStr     pdb     =NULL;   /* PDB identifier */
    AjPStr     spr     =NULL;   /* Swissprot identifier */
    AjPStr     acc     =NULL;   /* Accession number */
    AjPStr     line    =NULL;   /* Line from file */
    AjPStr     token   =NULL;   /* Token from line */
    const AjPStr subtoken=NULL;   /* Token from line */
    AjPList    acclist =NULL;   /* List of accession numbers */
    AjPList    sprlist =NULL;   /* List of swissprot identifiers */
    ajint      n       =0;      /* No. of accession numbers for current pdb 
				   code */
    AjBool     ok      =ajFalse;/* True if "____  _" has been found and we 
				   can start parsing */
    AjBool     done_1st=ajFalse;/* True if the first line of data has been 
				   parsed*/

    

    /* Memory allocation */
    line    = ajStrNew();
    token   = ajStrNew();
    subtoken= ajStrNew();
    pdb     = ajStrNew();
    acclist = ajListstrNew();
    sprlist = ajListstrNew();
    ret     = ajListNew();
    


    /* Read lines from file */
    while(ajReadlineTrim(inf, &line))
    {
	if(ajStrPrefixC(line, "____  _"))
	{
	    ok=ajTrue;
	    continue;
	}
	
	
	if(!ok)
	    continue;

	if(ajStrMatchC(line, ""))
	    break; 
	
	

	/* Read in pdb code first.  Then tokenise by ':', discard the 
	   first token, then tokenise the second token by ',', parsing 
	   out the swissprot codes and accession numbers from the 
	   subtokens */


	/* Make sure this is a line containing the pdb code */
	if((ajStrFindC(line, ":")!=-1))
	{
	    if(done_1st)
	    {
		tmp = ajPdbtospNew(0);
		ajStrAssignS(&tmp->Pdb, pdb);
		tmp->n = n;
		ajListToarray(acclist, (void ***) &tmp->Acc);
		ajListToarray(sprlist, (void ***) &tmp->Spr);
		ajListPushAppend(ret, (void *)tmp);
		
		
		ajListstrFree(&acclist);
		ajListstrFree(&sprlist);		
		acclist = ajListstrNew();
		sprlist = ajListstrNew();

		n=0;
	    }	

	    ajFmtScanS(line, "%S", &pdb);

	    ajStrParseC(line, ":");
	    ajStrAssignS(&token, ajStrParseC(NULL, ":"));

	    done_1st=ajTrue;
	}
	else 
	{
	    ajStrAssignS(&token, line);
	}
	

	spr  = ajStrNew();
	acc  = ajStrNew();
	ajFmtScanS(token, "%S (%S", &spr, &acc);
	

	if(ajStrSuffixC(acc, "),"))
	{
	    ajStrCutEnd(&acc, 2);
	}
	else
       	    ajStrCutEnd(&acc,1);
	
	ajListstrPushAppend(acclist, acc);
	ajListstrPushAppend(sprlist, spr);
	n++;

	ajStrParseC(token, ",");

	while((subtoken=ajStrParseC(NULL, ",")))
	{
	    spr  = ajStrNew();
	    acc  = ajStrNew();

	    ajFmtScanS(subtoken, "%S (%S", &spr, &acc); 

	    if(ajStrSuffixC(acc, "),"))
	    {
		ajStrCutEnd(&acc,2);
	    }
	    else
		ajStrCutEnd(&acc,1);


	    ajListstrPushAppend(acclist, acc);
	    ajListstrPushAppend(sprlist, spr);
	    n++;
	}
    }	

    /* Data for last pdb code ! */
    tmp = ajPdbtospNew(0);
    ajStrAssignS(&tmp->Pdb, pdb);
    tmp->n = n;


    ajListToarray(acclist, (void ***) &tmp->Acc);
    ajListToarray(sprlist, (void ***) &tmp->Spr);	  
    ajListPushAppend(ret, (void *)tmp);
    ajListstrFree(&acclist);
    ajListstrFree(&sprlist);		



    /* Tidy up */
    ajStrDel(&line);
    ajStrDel(&token);
    ajStrDel(&pdb);
    ajListstrFree(&acclist);	
    ajListstrFree(&sprlist);	


    return ret;
}




/* @func ajPdbtospReadNew **************************************************
**
** Read a Pdbtosp object from a file in embl-like format (see documentation  
** for DOMAINATRIX "pdbtosp" application).
**
** @param [u] inf [AjPFile] Input file stream
** @param [r] entry [const AjPStr] Pdb id
**
** @return [AjPPdbtosp] True on success
** @@
****************************************************************************/

AjPPdbtosp  ajPdbtospReadNew(AjPFile inf, const AjPStr entry) 
{
    AjPPdbtosp ret = NULL;
    
    ret = ajPdbtospReadCNew(inf,ajStrGetPtr(entry));

    return ret;
}




/* @func ajPdbtospReadCNew *************************************************
**
** Read a Pdbtosp object from a file in embl-like format.  Memory for the
** object is allocated.
**
** @param [u] inf   [AjPFile] Input file stream
** @param [r] entry [const char*]   Pdb id
**
** @return [AjPPdbtosp] True on success
** @@
****************************************************************************/

AjPPdbtosp ajPdbtospReadCNew(AjPFile inf, const char *entry)
{
    AjPPdbtosp ret = NULL;

    AjPStr line   = NULL;
    AjPStr tentry = NULL;	
    AjPStr pdb    = NULL;	
    AjBool ok            = ajFalse;
    ajint  n             = 0;
    ajint  i             = 0;
    

    line    = ajStrNew();
    tentry  = ajStrNew();
    pdb     = ajStrNew();

    ajStrAssignC(&tentry,entry);
    ajStrFmtUpper(&tentry);
    
    while((ok=ajReadlineTrim(inf,&line)))
    {
	if(!ajStrPrefixC(line,"EN   "))
	    continue;

	ajFmtScanS(line, "%*S %S", &pdb);

	if(ajStrMatchWildS(pdb,tentry))
	    break;
    }

    if(!ok)
    {
        ajStrDel(&line);
        ajStrDel(&tentry);
        ajStrDel(&pdb);

	return NULL;
    }

    while(ok && !ajStrPrefixC(line,"//"))
    {
	if(ajStrPrefixC(line,"XX"))
	{
	    ok = ajReadlineTrim(inf,&line);
	    continue;
	}
	else if(ajStrPrefixC(line,"NE"))
	{
	    ajFmtScanS(line, "%*S %d", &n);
	    (ret) = ajPdbtospNew(n);
	    ajStrAssignS(&(ret)->Pdb, pdb);
	}
	else if(ajStrPrefixC(line,"IN"))
	{
	    ajFmtScanS(line, "%*S %S %*S %S", &(ret)->Spr[i],
		       &(ret)->Acc[i]);
	    i++;
	}
	
	ok = ajReadlineTrim(inf,&line);
    }

    ajStrDel(&line);
    ajStrDel(&tentry);
    ajStrDel(&pdb);

    return ret;
}




/* @func ajPdbtospReadAllNew ***********************************************
**
** Read all the Pdbtosp objects in a file in embl-like format (see 
** documentation for DOMAINATRIX "pdbtosp" application) and writes a list of 
** these objects. It then sorts the list by PDB id.
**
** @param [u] inf [AjPFile] Input file stream
**
** @return [AjPList] List of Pdbtosp objects.
** @@
****************************************************************************/

AjPList  ajPdbtospReadAllNew(AjPFile inf)
{
    AjPList ret = NULL;
    AjPPdbtosp ptr = NULL;
    
    /* Check args and allocate list if necessary */
    if(!inf)
	return NULL;

    if(!(ret))
	ret = ajListNew();
    

    while((ptr = ajPdbtospReadCNew(inf, "*")))
	ajListPush(ret, (void *) ptr);

    ajListSort(ret, pdbSortPdbtospPdb);
    
    return ret;
}




/* @func ajCmapReadINew ****************************************************
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
** @@
****************************************************************************/

 AjPCmap  ajCmapReadINew(AjPFile inf, ajint chn, ajint mod)
{
    AjPCmap ret = NULL;
    
    if(!(ret=ajCmapReadNew(inf, CMAP_MODE_I, chn, mod)))
	return NULL;

    return ret;
}




/* @func ajCmapReadCNew ****************************************************
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
** @@
****************************************************************************/

AjPCmap ajCmapReadCNew(AjPFile inf, char chn, ajint mod)
{
    AjPCmap ret = NULL;

    if(!(ret = ajCmapReadNew(inf, CMAP_MODE_C, (ajint)chn, mod)))
	return NULL;

    return ret;
}




/* @func ajCmapReadAllNew ***************************************************
**
** Read every Cmap object from a file in CON format (see 
** documentation for DOMAINATRIX "contacts" application) and returns a list
** of these objects. 
** 
** @param [u] inf     [AjPFile]  Input file stream
**
** @return [AjPList]   List of Cmap objects.
*** @@
****************************************************************************/

AjPList ajCmapReadAllNew(AjPFile inf)
{
  AjPList ret  = NULL;
  AjPCmap cmap = NULL;

  ret = ajListNew();

  while((cmap = ajCmapReadNew(inf, CMAP_MODE_I, 0, 0)))
    ajListPushAppend(ret, cmap);

  return ret;
}




/* @func ajCmapReadNew *****************************************************
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
** @@
****************************************************************************/

AjPCmap ajCmapReadNew(AjPFile inf, ajint mode, ajint chn, ajint mod)
	       
{	
    AjPCmap  ret = NULL;
    const AjPStr   token       = NULL;   /* For parsing      */
        
    ajint    smcon     = 0;      /* No. of SM contacts       */	
    ajint    licon     = 0;      /* No. of LI contacts       */	
    ajint    x         = 0;      /* No. of first residue making contact */
    ajint    y         = 0;      /* No. of second residue making contact */
    ajint    md        = -1;     /* Model number   */
    ajint    cn1       = -1;     /* Chain number 1 */
    ajint    cn2       = -1;     /* Chain number 2 */
    char     id1       = -1;     /* Chain id 1     */
    char     id2       = -1;     /* Chain id 2     */
    ajint    nres1     = 0;      /* No. of residues in domain / chain 1 */
    ajint    nres2     = 0;      /* No. of residues in domain / chain 2 */
    AjPStr   seq1      = NULL;   /* Sequence 1 */
    AjPStr   seq2      = NULL;   /* Sequence 2 */
           
    AjBool   idok      = ajFalse; /* If the required chain has been found */

    ajint     en;                /* Entry number. */
    ajint     ns;                /* No. of sites, SITES output only */
    ajint     sn;                /* Site number, SITES output only */
    
    /* Check args */	
    if(!inf)
    {	
	ajWarn("Invalid args to ajCmapReadNew");	
	return NULL;
    }


    /* Convert '_' chain identifiers to '.' if necessary */
    if(mode==CMAP_MODE_C)
	if(chn=='_')
	    chn='.';



    
    /* Initialise strings */
    if(!cmapStrline)
    {
	cmapStrline       = ajStrNew();
	cmapStrtemp_id    = ajStrNew();
	cmapStrtemp_domid = ajStrNew();
	cmapStrtemp_ligid = ajStrNew();
	cmapStrdesc       = ajStrNew();
	cmapStrtmpstr     = ajStrNew();
    }
    

    /* Start of main loop */
    while((ajReadlineTrim(inf, &cmapStrline)))
    {
        /* // */
	if(ajStrPrefixC(cmapStrline, "//"))
	{
        /* If the delimiter between entries is found and ret is non-NULL, i.e.
           has been allocated, the function should return. */
	  ajStrDel(&seq1);
	  ajStrDel(&seq2);

	  return ret;	
	}


        /* SI */
	else if(ajStrPrefixC(cmapStrline, "SI"))
	{ 
	  token = ajStrParseC(cmapStrline, ";");
	  ajFmtScanS(token, "%*s %*s %d", &sn);

	  token = ajStrParseC(NULL, ";");
	  ajFmtScanS(token, "%*s %d", &ns);
        }
	/* EN */
	else if(ajStrPrefixC(cmapStrline, "EN"))
	{
	    ajFmtScanS(cmapStrline, "%*s %*c%d", &en);
	}
	
	    
	/* TY */
	else if(ajStrPrefixC(cmapStrline, "TY"))
	{
	    ajFmtScanS(cmapStrline, "%*s %S", &cmapStrtype);
	    ajStrSetClear(&seq1);
	    ajStrSetClear(&seq2);
	    id1 = '.';
	    id2 = '.';
	    cn1=0;
	    cn2=0;
	    nres1=0;
	    nres2=0;
	}

	/* EX, NE records are not parsed */

	/* ID */
	else if(ajStrPrefixC(cmapStrline, "ID"))
	{
	    token = ajStrParseC(cmapStrline, ";");
	    ajFmtScanS(token, "%*s %*s %S", &cmapStrtemp_id);
	    token = ajStrParseC(NULL, ";");
	    ajFmtScanS(token, "%*s %S", &cmapStrtemp_domid);
	    token = ajStrParseC(NULL, ";");
	    ajFmtScanS(token, "%*s %S", &cmapStrtemp_ligid);
	}

	/* DE records are not parsed (SITES output) */
	else if(ajStrPrefixC(cmapStrline, "DE"))
	{
            ajStrAssignSubS(&cmapStrdesc, cmapStrline, 4, -1);
	}

	/* CN */
	else if(ajStrPrefixC(cmapStrline, "CN"))
	{
	    token = ajStrParseC(cmapStrline, ";");
	    /* ajFmtScanS(token, "%*s %*s %d", &md);
            if(md == '.')
	       md = 0;	
	       */

	    ajFmtScanS(token, "%*s %*s %S", &cmapStrtmpstr);

	    if(ajStrMatchC(cmapStrtmpstr, "."))
		md = 0;	
	    else
		ajFmtScanS(cmapStrtmpstr, "%d", &md);
	    
	    token = ajStrParseC(NULL, ";");
	    ajFmtScanS(token, "%*s %d", &cn1);

	    token = ajStrParseC(NULL, ";");
	    ajFmtScanS(token, "%*s %d", &cn2);

	    token = ajStrParseC(NULL, ";");
	    ajFmtScanS(token, "%*s %c", &id1);

	    token = ajStrParseC(NULL, ";");
	    ajFmtScanS(token, "%*s %c", &id2);

	    token = ajStrParseC(NULL, ";");
	    /* ajFmtScanS(token, "%*s %d", &nres1);
	    if(nres1 == '.')
	       nres1 = 0; */

	    ajFmtScanS(token, "%*s %S", &cmapStrtmpstr);

	    if(ajStrMatchC(cmapStrtmpstr, "."))
		nres1 = 0;	
	    else
		ajFmtScanS(cmapStrtmpstr, "%d", &nres1);


	    token = ajStrParseC(NULL, ";");
	    /* ajFmtScanS(token, "%*s %d", &nres2);
	       if((char)nres2 == '.')
	       nres2 = 0; */

	    ajFmtScanS(token, "%*s %S", &cmapStrtmpstr);

	    if(ajStrMatchC(cmapStrtmpstr, "."))
		nres2 = 0;	
	    else
		ajFmtScanS(cmapStrtmpstr, "%d", &nres2);

	}

	/* S1 */
	else if(ajStrPrefixC(cmapStrline, "S1"))
	{    
	    while(ajReadlineTrim(inf,&cmapStrline) &&
                  !ajStrPrefixC(cmapStrline,"XX"))
		ajStrAppendC(&seq1,ajStrGetPtr(cmapStrline));

	    ajStrRemoveWhite(&seq1);
	}

	/* S2 */
	else if(ajStrPrefixC(cmapStrline, "S2"))
	{    
	    while(ajReadlineTrim(inf,&cmapStrline) &&
                  !ajStrPrefixC(cmapStrline,"XX"))
		ajStrAppendC(&seq2,ajStrGetPtr(cmapStrline));

	    ajStrRemoveWhite(&seq2);
	}
	/* NC */	    
	else if((ajStrPrefixC(cmapStrline, "NC")) && 
		((md==mod) || ((chn==0)&&(mod==0)&&(mode==CMAP_MODE_I))))
	{
	    token = ajStrParseC(cmapStrline, ";");
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

	    if( ((cn1==chn)&&(mode==CMAP_MODE_I))                       ||
		((chn==0)&&(mod==0)&&(mode==CMAP_MODE_I))               ||
		((toupper((int)id1)==toupper(chn))&&(mode==CMAP_MODE_C))||
		((toupper((int)id1)=='.') && (toupper(chn)!='.') &&
		(mode==CMAP_MODE_C))
		)
	    {
		idok=ajTrue;
		
		/* Allocate contact map and write values */
		if(ajStrMatchC(cmapStrtype, "INTER"))
		{
		    if(nres1>nres2)
			(ret) = ajCmapNew(nres1);
		    else
			(ret) = ajCmapNew(nres2);
		}
		else
		    (ret) = ajCmapNew(nres1);

		ajStrAssignS(&(ret)->Id, cmapStrtemp_id);
		ajStrAssignS(&(ret)->Domid, cmapStrtemp_domid);
		ajStrAssignS(&(ret)->Ligid, cmapStrtemp_ligid);
		
		if(ajStrMatchC(cmapStrtype, "INTRA"))
		{
		    ret->Type = ajINTRA;
		    (ret)->Ncon = smcon;
		}
		else if(ajStrMatchC(cmapStrtype, "INTER"))
		{
		    ret->Type = ajINTER;
		    (ret)->Ncon = smcon;
		}
		else if(ajStrMatchC(cmapStrtype, "LIGAND"))
		  {
		    ret->Type = ajLIGAND;
		    (ret)->Ncon = licon;
		    ret->ns = ns;
		    ret->sn = sn;
		    ajStrAssignS(&ret->Desc, cmapStrdesc);
		}
		else
		    ajFatal("Unrecognised contact type");

		ret->Chn1  = cn1;
		ret->Chn2  = cn2;
		ret->Chid1 = id1;
		ret->Chid2 = id2;
		ret->Nres1 = nres1;
		ret->Nres2 = nres2;
		ret->en = en;

		
		ajStrAssignS(&ret->Seq1, seq1);
		ajStrAssignS(&ret->Seq2, seq2);		
	    }
	}

	/* SM */
	else if((ajStrPrefixC(cmapStrline, "SM")) && 
		((md==mod) || ((chn==0)&&(mod==0)&&(mode==CMAP_MODE_I)))
		&& (idok))
	{
	    ajFmtScanS(cmapStrline, "%*s %*s %d %*c %*s %d", &x, &y);

	    /* Check residue number is in range */
	    if((x>(ret)->Dim) || (y>(ret)->Dim))
		ajFatal("Fatal attempt to write bad data in "
			"ajCmapReadNew\nFile: %S (%S)\nx: %d y:%d\n",
			ajFileGetNameS(inf), cmapStrtemp_id, x, y);
	    
	    /* Enter '1' in matrix to indicate contact */
	    ajUint2dPut(&(ret)->Mat, x-1, y-1, 1);
	    ajUint2dPut(&(ret)->Mat, y-1, x-1, 1);
	}

	/* LI */
	else if((ajStrPrefixC(cmapStrline, "LI")) && 
		((md==mod) || ((chn==0)&&(mod==0)&&(mode==CMAP_MODE_I)))
		&& (idok))
	{
	    ajFmtScanS(cmapStrline, "%*s %*s %d", &x);

	    /* Check residue number is in range */
	    if((x>(ret)->Dim))
		ajFatal("Fatal attempt to write bad data in "
			"ajCmapReadNew\nFile: %S (%S)\nx: %d\n",
			ajFileGetNameS(inf), cmapStrtemp_id, x);
	    
	    /* Enter '1' in matrix to indicate contact.  For ligand contacts, 
	       the first row / column only is used. */
	    ajUint2dPut(&(ret)->Mat, x-1, 0, 1);
	    ajUint2dPut(&(ret)->Mat, 0, x-1, 1);
	}
    }


    ajStrDel(&seq1);
    ajStrDel(&seq2);
    return ret;	
}




/* @func ajVdwallReadNew ***************************************************
**
** Read a Vdwall object from a file in embl-like format (see documentation
** for the EMBASSY DOMAINATRIX package).
** 
** @param [u] inf     [AjPFile]  Input file stream
**
** @return [AjPVdwall] Pointer to Vdwall object.
** @category new [AjPVdwall] Vdwall constructor from reading file in embl-like
**              format (see documentation for the EMBASSY DOMAINATRIX package).
** @@
****************************************************************************/

AjPVdwall  ajVdwallReadNew(AjPFile inf)
{
    AjPVdwall ret = NULL;
    AjPStr line = NULL;   /* Line of text */
    ajint nres  = 0;      /* No. residues */
    ajint natm  = 0;      /* No. atoms */
    ajint rcnt  = 0;      /* Residue count */
    ajint acnt  = 0;      /* Atom count */
    char id1    = '\0';             /* Residue 1 char id code */
    AjPStr id3  = NULL;   /* Residue 3 char id code */
    
    
    /* Allocate strings */
    line = ajStrNew();
    id3  = ajStrNew();


    /* Start of main loop */
    while((ajReadlineTrim(inf, &line)))
    {
	/* Parse NR line */
	if(ajStrPrefixC(line, "NR"))
	{	
	    ajFmtScanS(line, "%*s %d", &nres);
		
	    /* Allocate Vdwall object */
	    (ret) = ajVdwallNew(nres);
		
	}
	/* Parse residue id 3 char */
	else if(ajStrPrefixC(line, "AA"))
	{	
	    rcnt++;
	    acnt = 0;
	    ajFmtScanS(line, "%*s %S", &id3);
	}
	/* Parse residue id 1 char */
	else if(ajStrPrefixC(line, "ID"))
	    ajFmtScanS(line, "%*s %c", &id1);
	/* Parse number of atoms */
	else if(ajStrPrefixC(line, "NN"))
	{
	    ajFmtScanS(line, "%*s %d", &natm);
	    
	    /* Allocate next Vdwres object */
	    (ret)->Res[rcnt-1]=ajVdwresNew(natm);
	    
	    /* Write members of Vdwres object */
	    (ret)->Res[rcnt-1]->Id1 = id1;
	    ajStrAssignS(&(ret)->Res[rcnt-1]->Id3, id3);
	    
	}
	/* Parse atom line */
	else if(ajStrPrefixC(line, "AT"))
	{
	    acnt++;
	    ajFmtScanS(line, "%*s %S %*c %f", 
		       &(ret)->Res[rcnt-1]->Atm[acnt-1], 
		       &(ret)->Res[rcnt-1]->Rad[acnt-1]);	
	}
    }	


    ajStrDel(&line);
    ajStrDel(&id3);
    
    return ret;
}




/* @func ajHetReadNew ******************************************************
**
** Read heterogen dictionary, the Het object is allocated.
** 
** @param [u] inf [AjPFile]    Pointer to Het file
** 
** @return [AjPHet] Het object.
** @category new [AjPHet] Het constructor from reading dictionary of
**                         heterogen groups in clean format (see documentation
**                         for the EMBASSY DOMAINATRIX package).
** @@
****************************************************************************/

AjPHet  ajHetReadNew(AjPFile inf)
{
    AjPHet hetdic   = NULL;
    AjPStr line     = NULL;		/* current line */
    AjPHetent entry = NULL;		/* current entry */
    AjPList list    = NULL;		/* List of entries */
    AjPStr temp     = NULL;		/* Temporary string */
  
  
    /*Check args */
    if((!inf))
    {
	ajWarn("Bad args passed to ajHetReadNew\n");

	return NULL;
    }

    /* Create Het object if necessary */
    if(!(hetdic))
	hetdic = ajHetNew(0);
    
    /* Create string and list objects */
  
    line = ajStrNew();
    temp = ajStrNew();
    list = ajListNew();
  
    /* Read lines from file */
    while(ajReadlineTrim(inf, &line))
    {
	if(ajStrPrefixC(line, "ID   "))
	{
	    entry=ajHetentNew();
	    ajFmtScanS(line, "%*s %S", &entry->abv);
	}	
	else if(ajStrPrefixC(line, "DE   "))
	{  	/* NEED TO ACCOUNT FOR MULTIPLE LINES */
	    ajStrAssignSubS(&temp, line, 5, -1);
	    if(ajStrGetLen(entry->ful))
		ajStrAppendS(&entry->ful, temp);
	    else
		ajStrAssignS(&entry->ful, temp);
	}	
	else if(ajStrPrefixC(line, "SY   "))
	{
	    /*	  ajFmtScanS(line, "%*s %S", &entry->syn); */
	    ajStrAssignSubS(&temp, line, 5, -1);
	    if(ajStrGetLen(entry->syn))
		ajStrAppendS(&entry->syn, temp);
	    else
		ajStrAssignS(&entry->syn, temp);
	}
	else if(ajStrPrefixC(line, "NN   "))
	    ajFmtScanS(line, "%*s %S", &entry->cnt);
	else if(ajStrPrefixC(line, "//"))
	    ajListPush(list, (AjPHetent) entry);
    }

    (hetdic)->n=ajListToarray(list, (void ***) &((hetdic)->entries));
  
    ajStrDel(&line);
    ajStrDel(&temp);
    ajListFree(&list);

    return hetdic;
}




/* @func ajHetReadRawNew ***************************************************
**
** Reads a dictionary of heterogen groups available at 
** http://pdb.rutgers.edu/het_dictionary.txt and writes a Het object.
**
** @param [u] inf [AjPFile]    Pointer to dictionary
**
** @return [AjPHet] True on success
** @category new [AjPHet] Het constructor from reading dictionary of
**                        heterogen groups in raw format.
** @@
****************************************************************************/

AjPHet ajHetReadRawNew(AjPFile inf)
{
    AjPHet ret       = NULL;
    AjPStr line      = NULL;  /* A line from the file */
    AjPHetent entry  = NULL;  /* The current entry */
    AjPHetent tmp    = NULL;  /* Temp. pointer */
    AjPList list     = NULL;  /* List of entries */
    ajint het_cnt    = 0;     /* Count of number of HET records in file */
    ajint formul_cnt = 0;     /* Count of number of FORMUL records in file */
    

    /* Check arg's */
    if((!inf))
    {
	ajWarn("Bad args passed to ajHetReadRawNew\n");
	return NULL;
    }
    
    /* Create strings etc */
    line = ajStrNew();
    list = ajListNew();

    
    /* Read lines from file */
    while(ajReadlineTrim(inf, &line))
    {
	if(ajStrPrefixC(line,"HET "))
	{
	    het_cnt++;
	    
	    entry=ajHetentNew();
	    ajFmtScanS(line, "%*s %S", &entry->abv);
	}
	else if(ajStrPrefixC(line,"HETNAM"))
	{
	    ajStrAppendC(&entry->ful, &line->Ptr[15]);
	}
	else if(ajStrPrefixC(line,"HETSYN"))
	{
	    ajStrAppendC(&entry->syn, &line->Ptr[15]);
	}
	else if(ajStrPrefixC(line,"FORMUL"))
	{
	    formul_cnt++;

	    /* In cases where HETSYN or FORMUL were not
	       specified, assign a value of '.' */
	    if(MAJSTRGETLEN(entry->ful)==0)
		ajStrAssignC(&entry->ful, ".");

	    if(MAJSTRGETLEN(entry->syn)==0)
		ajStrAssignC(&entry->syn, ".");
	    

	    /* Push entry onto list */
	    ajListPush(list, (AjPHetent) entry);
	}
    }

    if(het_cnt != formul_cnt)
    {	
	while(ajListPop(list, (void **) &tmp))
	    ajHetentDel(&tmp);
	
	ajListFree(&list);	    
	ajStrDel(&line);

	ajFatal("Fatal discrepancy in count of HET and FORMUL records\n");
    }	
    
    ret = ajHetNew(0);
    ret->n = ajListToarray(list, (void ***) &((ret)->entries));
    
   
    ajStrDel(&line);
    ajListFree(&list);

    return ret;
}




/* @func ajPdbReadFirstModelNew ********************************************
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
** @@
****************************************************************************/

AjPPdb ajPdbReadFirstModelNew(AjPFile inf) 
{
    return( (AjPPdb) ajPdbReadNew(inf, 0));
}




/* @func ajPdbReadAllModelsNew **********************************************
**
** Reads a clean coordinate file (see documentation for DOMAINATRIX "pdbparse" 
** application) and writes a filled Pdb object.  Data for all models is read.
**
** @param [u] inf  [AjPFile] Pointer to clean coordinate file
**
** @return [AjPPdb] Pointer to Pdb object.
** @category new [AjPPdb] Pdb constructor from reading ccf format file.
** @@
****************************************************************************/

AjPPdb ajPdbReadAllModelsNew(AjPFile inf)
{
    return( (AjPPdb) ajPdbReadNew(inf, 1));
}




/* @func ajPdbReadNew *******************************************************
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
** @@
****************************************************************************/

AjPPdb ajPdbReadNew(AjPFile inf, ajint mode) 
{
    AjPPdb ret = NULL;
    
    ajint nmod = 0;
    ajint ncha = 0;
    ajint ngrp = 0;
    ajint nc   = 0;
    ajint mod  = 0;
    ajint chn  = 0;
    ajint gpn  = 0;


    float reso = 0.0;

    AjPStr line      = NULL;
    AjPStr token     = NULL;
    AjPStr idstr     = NULL;
    AjPStr destr     = NULL;
    AjPStr osstr     = NULL;
    AjPStr xstr      = NULL;
    AjPStrTok handle = NULL;
    
    AjPAtom    atom     = NULL;
    AjPResidue residue  = NULL;
    ajint      rn_last = -100000;
    ajint      mn_last = -100000;

    AjBool     fixReadAtoms = ajTrue;

    /* Initialise strings */
    line  = ajStrNew();
    token = ajStrNew();
    idstr = ajStrNew();
    destr = ajStrNew();
    osstr = ajStrNew();
    xstr  = ajStrNew();

    /* Start of main loop */
    while(ajReadlineTrim(inf,&line))
    {
	if(ajStrPrefixC(line,"XX"))
	    continue;

	/* Parse ID */
	else if(ajStrPrefixC(line,"ID"))
	{
	    ajStrTokenAssignC(&handle,line," \n\t\r");
	    ajStrTokenNextParse(&handle,&token);
	    ajStrTokenNextParse(&handle,&idstr);
	    continue;
	}

	
	/* Parse number of chains */
	else if(ajStrPrefixC(line,"CN"))
	{
	    ajStrTokenAssignC(&handle,line," []\n\t\r");
	    ajStrTokenNextParse(&handle,&token);
	    ajStrTokenNextParse(&handle,&token);
	    ajStrToInt(token,&nc);
	    continue;
	}
	

	/* Parse description text */
	else if(ajStrPrefixC(line,"DE"))
	{
	    ajStrTokenAssignC(&handle, line, " ");
	    ajStrTokenNextParse(&handle, &token);
	    /* 'DE' */
	    ajStrTokenNextParseC(&handle, "\n\r", &token);

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
	else if(ajStrPrefixC(line,"OS"))
	{
	    ajStrTokenAssignC(&handle, line, " ");
	    ajStrTokenNextParse(&handle, &token);
	    /* 'OS' */
	    ajStrTokenNextParseC(&handle, "\n\r", &token);

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
	else if(ajStrPrefixC(line,"EX"))
	{
	    ajStrTokenAssignC(&handle,line," ;\n\t\r");
	    ajStrTokenNextParse(&handle,&token);
	    ajStrTokenNextParse(&handle,&token);

	    ajStrTokenNextParse(&handle,&xstr); /* method */
	    ajStrTokenNextParse(&handle,&token);

	    ajStrTokenNextParse(&handle,&token); /* reso */
	    ajStrToFloat(token,&reso);
	    ajStrTokenNextParse(&handle,&token);
	    ajStrTokenNextParse(&handle,&token); /* nmod */
	    ajStrToInt(token,&nmod);
	    ajStrTokenNextParse(&handle,&token);

	    ajStrTokenNextParse(&handle,&token); /* ncha */
	    ajStrToInt(token,&ncha);

	    ajStrTokenNextParse(&handle,&token); /* nlig */
	    ajStrToInt(token,&ngrp);

	    ret = ajPdbNew(ncha);

	    ajStrAssignS(&(ret)->Pdb,idstr);
	    ajStrAssignS(&(ret)->Compnd,destr);
	    ajStrAssignS(&(ret)->Source,osstr);

	    if(ajStrMatchC(xstr,"xray"))
		(ret)->Method = ajXRAY;
	    else
		(ret)->Method = ajNMR;

	    (ret)->Reso = reso;

	    /* 
	     **   0==Read first model only. Number of models is hard-coded to 1
	     **   as only the data for the first model is read in. 
	     **   1==Read all models. 
	     */

	    if(mode == 0)
		(ret)->Nmod = 1;
	    else if(mode == 1)
		(ret)->Nmod = nmod;
	    else
		ajFatal("Unrecognised mode in ajPdbReadNew");
	    
	    (ret)->Nchn = ncha;
	    (ret)->Ngp  = ngrp;

	    continue;
	}
	

	/* Parse information line */
	else if(ajStrPrefixC(line,"IN"))
	{
	    ajStrTokenAssignC(&handle,line," ;\n\t\r");
	    ajStrTokenNextParse(&handle,&token);
	    ajStrTokenNextParse(&handle,&token);

	    /* id value */
	    ajStrTokenNextParse(&handle,&token); 
	    (ret)->Chains[nc-1]->Id=*ajStrGetPtr(token);
	    ajStrTokenNextParse(&handle,&token);

	    /* residues */
	    ajStrTokenNextParse(&handle,&token); 
	    ajStrToInt(token,&(ret)->Chains[nc-1]->Nres);
	    ajStrTokenNextParse(&handle,&token);

	    /* hetatm */
	    ajStrTokenNextParse(&handle,&token); 
	    ajStrToInt(token,&(ret)->Chains[nc-1]->Nlig);

	    /* helices */
	    ajStrTokenNextParse(&handle,&token); 
	    ajStrToInt(token,&(ret)->Chains[nc-1]->numHelices);

	    /* strands */
	    ajStrTokenNextParse(&handle,&token); 
	    ajStrToInt(token,&(ret)->Chains[nc-1]->numStrands);

	    /* sheets */
	    /*
	    ajStrTokenNextParse(&handle,&token); 
	    ajStrToInt(token,&(ret)->Chains[nc-1]->numSheets);
	    */
	    /* turns */
	    /*
	    ajStrTokenNextParse(&handle,&token); 
	    ajStrToInt(token,&(ret)->Chains[nc-1]->numTurns);
	    */
	    continue;
	}
  

	/* Parse sequence line */
	else if(ajStrPrefixC(line,"SQ"))
	{
	    while(ajReadlineTrim(inf,&line) && !ajStrPrefixC(line,"XX"))
		ajStrAppendC(&(ret)->Chains[nc-1]->Seq,ajStrGetPtr(line));

	    ajStrRemoveWhite(&(ret)->Chains[nc-1]->Seq);
	    continue;
	}


	/* Parse atom line */
	else if(fixReadAtoms && ajStrPrefixC(line,"AT"))
	{
	    mod = chn = gpn = 0;
	    
	    /* Skip AT record */
	    ajStrTokenAssignC(&handle,line," \t\n\r");
	    ajStrTokenNextParse(&handle,&token);

	    /* Model number. 0==Read first model only */
	    ajStrTokenNextParse(&handle,&token);
	    ajStrToInt(token,&mod);

	    if((mode == 0) && (mod!=1))
            {
		/* break; */
		/* Discard remaining AT lines */
		while(ajReadlineTrim(inf,&line) && ajStrPrefixC(line,"AT"));
            }

	    /* Chain number */
	    ajStrTokenNextParse(&handle,&token);
	    ajStrToInt(token,&chn);

	    /* Group number */
	    ajStrTokenNextParse(&handle,&token);
	    ajStrToInt(token,&gpn);
	    
	    /* Allocate object */
	    /* AJNEW0(atom); */
	    atom = ajAtomNew();

	    atom->Mod = mod;
	    atom->Chn = chn;
	    atom->Gpn = gpn;
	    
	    /* Residue number */
	    ajStrTokenNextParse(&handle,&token);
	    ajStrToInt(token,&atom->Idx);

	    /* Residue number string */
	    ajStrTokenNextParse(&handle,&token);
	    ajStrAssignS(&atom->Pdb,token);

	    /* Residue id, 1 char */
	    ajStrTokenNextParse(&handle,&token);
	    atom->Id1 = *ajStrGetPtr(token);
	    
	    /* Residue id, 3 char */
	    ajStrTokenNextParse(&handle,&token);
	    ajStrAssignS(&atom->Id3,token);

	    /* Atom type */
	    ajStrTokenNextParse(&handle,&token);
	    atom->Type = *ajStrGetPtr(token);
	    
	    /* Atom identifier */
	    ajStrTokenNextParse(&handle,&token);
	    ajStrAssignS(&atom->Atm,token);

	    /* X coordinate */
	    ajStrTokenNextParse(&handle,&token);
	    ajStrToFloat(token,&atom->X);

	    /* Y coordinate */
	    ajStrTokenNextParse(&handle,&token);
	    ajStrToFloat(token,&atom->Y);

	    /* Z coordinate */
	    ajStrTokenNextParse(&handle,&token);
	    ajStrToFloat(token,&atom->Z);

	    /* Occupancy */
	    ajStrTokenNextParse(&handle,&token);
	    ajStrToFloat(token,&atom->O);

	    /* B value thermal factor.  */
	    ajStrTokenNextParse(&handle,&token);
	    ajStrToFloat(token,&atom->B);


	    /* 
	     ** Push atom onto appropriate list. 
	     ** Check for coordinates for water or groups that could not
	     ** be uniquely assigned to a chain
	     */
	    if(chn==0)
	    {
		/* Heterogen */
		if(atom->Type == 'H')
		    ajListPushAppend((ret)->Groups,(void *)atom);
		else if(atom->Type == 'W')
		    ajListPushAppend((ret)->Water,(void *)atom);
		else
		    ajFatal("Unexpected parse error in "
			    "ajPdbReadFirstModelNew");
	    }
	    else
	      {
		ajListPushAppend((ret)->Chains[chn-1]->Atoms,(void *)atom);
	      }
	    continue;
	}
	
	/* Parse residue line */
	else if(ajStrPrefixC(line,"RE"))
	{
	    mod = chn = 0;
	    
	    /* Skip RE record */
	    ajStrTokenAssignC(&handle,line," \t\n\r");
	    ajStrTokenNextParse(&handle,&token);

	    /* Model number. 0==Read first model only */
	    ajStrTokenNextParse(&handle,&token);
	    ajStrToInt(token,&mod);

	    if((mode == 0) && (mod!=1))
	      {
		/* break;*/
		/* Discard remaining RE lines */
		while(ajReadlineTrim(inf,&line) && ajStrPrefixC(line,"RE"));
	      }

	    /* Chain number */
	    ajStrTokenNextParse(&handle,&token);
	    ajStrToInt(token,&chn);

	    /* Allocate object */
	    /* AJNEW0(residue); */
	    residue = ajResidueNew();

	    residue->Mod = mod;
	    residue->Chn = chn;
	    
	    /* Residue number */
	    ajStrTokenNextParse(&handle,&token);
	    ajStrToInt(token,&residue->Idx);

	    /* Residue number (original string) */
	    ajStrTokenNextParse(&handle,&token);
	    ajStrAssignS(&residue->Pdb,token);

	    /* Residue id, 1 char */
	    ajStrTokenNextParse(&handle,&token);
	    residue->Id1 = *ajStrGetPtr(token);
	    
	    /* Residue id, 3 char */
	    ajStrTokenNextParse(&handle,&token);
	    ajStrAssignS(&residue->Id3,token);

	    /* Element serial number (PDB elements) */
	    ajStrTokenNextParse(&handle,&token);
	    ajStrToInt(token,&residue->eNum);
	    
	    /* Element identifier  (PDB elements) */
	    ajStrTokenNextParse(&handle,&token);
	    ajStrAssignS(&residue->eId,token);

	    /* Element type (PDB elements) */
	    ajStrTokenNextParse(&handle,&token);
	    residue->eType = *ajStrGetPtr(token);

	    /* Class of helix  (PDB elements) */ 
	    ajStrTokenNextParse(&handle,&token);
	    ajStrToInt(token,&residue->eClass);

	    /* Number of the element (stride) */
	    ajStrTokenNextParse(&handle,&token);
	    ajStrToInt(token,&residue->eStrideNum);

	    /* Element type (stride) */
	    ajStrTokenNextParse(&handle,&token);
	    residue->eStrideType = *ajStrGetPtr(token);

	    /* Phi angle */
	    ajStrTokenNextParse(&handle,&token);
            ajStrToFloat(token,&residue->Phi);

	    /* Psi angle */
            ajStrTokenNextParse(&handle,&token);
            ajStrToFloat(token,&residue->Psi);

	    /* Residue solvent accessible area.  */
            ajStrTokenNextParse(&handle,&token);
            ajStrToFloat(token,&residue->Area);

	    /* Absolute accessibility, all atoms.  */
            ajStrTokenNextParse(&handle,&token);
            ajStrToFloat(token,&residue->all_abs);

	    /* Relative accessibility, all atoms. */
            ajStrTokenNextParse(&handle,&token);
            ajStrToFloat(token,&residue->all_rel);
	    
	    /* Absolute accessibility, atoms in side chain.  */
            ajStrTokenNextParse(&handle,&token);
            ajStrToFloat(token,&residue->side_abs);

	    /* Relative accessibility, atoms in side chain.  */
            ajStrTokenNextParse(&handle,&token);
            ajStrToFloat(token,&residue->side_rel);

	    /* Absolute accessibility, atoms in main chain. */
            ajStrTokenNextParse(&handle,&token);
            ajStrToFloat(token,&residue->main_abs);

	    /* Relative accessibility, atoms in main chain. */
            ajStrTokenNextParse(&handle,&token);
            ajStrToFloat(token,&residue->main_rel);

	    /* Absolute accessibility, non-polar atoms. */
            ajStrTokenNextParse(&handle,&token);
            ajStrToFloat(token,&residue->npol_abs);

	    /* Relative accessibility, non-polar atoms. */
            ajStrTokenNextParse(&handle,&token);
            ajStrToFloat(token,&residue->npol_rel);

	    /* Absolute accessibility, polar atoms. */
            ajStrTokenNextParse(&handle,&token);
            ajStrToFloat(token,&residue->pol_abs);

	    /* Relative accessibility, polar atoms. */
            ajStrTokenNextParse(&handle,&token);
            ajStrToFloat(token,&residue->pol_rel);

	    ajListPushAppend((ret)->Chains[chn-1]->Residues,(void *)residue);  

	    continue;
	}
	/* Parse coordinate line */
	else if(ajStrPrefixC(line,"CO"))
	{
	    mod = chn = gpn = 0;
	    
	    ajStrTokenAssignC(&handle,line," \t\n\r");
	    ajStrTokenNextParse(&handle,&token);

	    ajStrTokenNextParse(&handle,&token);
	    ajStrToInt(token,&mod);

	    ajStrTokenNextParse(&handle,&token);
	    ajStrToInt(token,&chn);

	    ajStrTokenNextParse(&handle,&token);
	    ajStrToInt(token,&gpn);
	    
	    /* AJNEW0(atom); */
	    atom = ajAtomNew();
	    
	    atom->Mod = mod;
	    atom->Chn = chn;
	    atom->Gpn = gpn;
	    
	    ajStrTokenNextParse(&handle,&token);
	    atom->Type = ajStrGetCharFirst(token);
	    
	    ajStrTokenNextParse(&handle,&token);
	    ajStrToInt(token,&atom->Idx);

	    ajStrTokenNextParse(&handle,&token);
	    ajStrAssignS(&atom->Pdb,token);

	    /* Residue object */
	    if(atom->Type=='P')
	    {
		/* New model */
		if(atom->Mod != mn_last)
		{
		    rn_last = -100000;
		    mn_last = atom->Mod;
		}
		/* New residue */
		if(atom->Idx != rn_last)
		{
		    residue = ajResidueNew();

		    residue->Mod     = atom->Mod;
		    residue->Chn     = atom->Chn;
		    residue->Idx     = atom->Idx;
		    ajStrAssignS(&residue->Pdb, atom->Pdb);
		}	
	    }	

	    ajStrTokenNextParse(&handle,&token);
	    residue->eType = *ajStrGetPtr(token);

	    ajStrTokenNextParse(&handle,&token);
	    ajStrToInt(token,&residue->eNum);

	    ajStrTokenNextParse(&handle,&token);
	    ajStrAssignS(&residue->eId,token);
	    
	    ajStrTokenNextParse(&handle,&token);
	    ajStrToInt(token,&residue->eClass);

	    ajStrTokenNextParse(&handle,&token);
	    residue->eStrideType = *ajStrGetPtr(token);

	    ajStrTokenNextParse(&handle,&token);
	    ajStrToInt(token,&residue->eStrideNum);

	    ajStrTokenNextParse(&handle,&token);
	    atom->Id1 = *ajStrGetPtr(token);
	    residue->Id1  = atom->Id1;
	    
	    ajStrTokenNextParse(&handle,&token);
	    ajStrAssignS(&atom->Id3,token);
	    ajStrAssignS(&residue->Id3, atom->Id3);

	    ajStrTokenNextParse(&handle,&token);
	    ajStrAssignS(&atom->Atm,token);

	    ajStrTokenNextParse(&handle,&token);
	    ajStrToFloat(token,&atom->X);

	    ajStrTokenNextParse(&handle,&token);
	    ajStrToFloat(token,&atom->Y);

	    ajStrTokenNextParse(&handle,&token);
	    ajStrToFloat(token,&atom->Z);

	    ajStrTokenNextParse(&handle,&token);
	    ajStrToFloat(token,&atom->O);

	    ajStrTokenNextParse(&handle,&token);
	    ajStrToFloat(token,&atom->B);

	    ajStrTokenNextParse(&handle,&token);
            ajStrToFloat(token,&residue->Phi);

            ajStrTokenNextParse(&handle,&token);
            ajStrToFloat(token,&residue->Psi);

            ajStrTokenNextParse(&handle,&token);
            ajStrToFloat(token,&residue->Area);

            ajStrTokenNextParse(&handle,&token);
            ajStrToFloat(token,&residue->all_abs);

            ajStrTokenNextParse(&handle,&token);
            ajStrToFloat(token,&residue->all_rel);

            ajStrTokenNextParse(&handle,&token);
            ajStrToFloat(token,&residue->side_abs);

            ajStrTokenNextParse(&handle,&token);
            ajStrToFloat(token,&residue->side_rel);

            ajStrTokenNextParse(&handle,&token);
            ajStrToFloat(token,&residue->main_abs);

            ajStrTokenNextParse(&handle,&token);
            ajStrToFloat(token,&residue->main_rel);

            ajStrTokenNextParse(&handle,&token);
            ajStrToFloat(token,&residue->npol_abs);

            ajStrTokenNextParse(&handle,&token);
            ajStrToFloat(token,&residue->npol_rel);

            ajStrTokenNextParse(&handle,&token);
            ajStrToFloat(token,&residue->pol_abs);

            ajStrTokenNextParse(&handle,&token);
            ajStrToFloat(token,&residue->pol_rel);


	    /* Check for coordinates for water or groups that could not
	       be uniquely assigned to a chain */
	    if(chn==0)
	    {
		/* Heterogen */
		if(atom->Type == 'H')
		    ajListPushAppend((ret)->Groups,(void *)atom);
		else if(atom->Type == 'W')
		    ajListPushAppend((ret)->Water,(void *)atom);
		else
		    ajFatal("Unexpected parse error in ajPdbRead");
	    }
	    else
		ajListPushAppend((ret)->Chains[chn-1]->Atoms,(void *)atom);

	    
	    ajListPushAppend((ret)->Chains[chn-1]->Residues,(void *)residue);
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
** @@
****************************************************************************/

AjPPdb ajPdbReadoldNew(AjPFile inf)
{
    AjPPdb ret = NULL;
    
    ajint nmod = 0;
    ajint ncha = 0;
    ajint ngrp = 0;
    ajint nc   = 0;
    ajint mod  = 0;
    ajint chn  = 0;
    ajint gpn  = 0;

    float reso = 0.0;

    AjPStr line      = NULL;
    AjPStr token     = NULL;
    AjPStr idstr     = NULL;
    AjPStr destr     = NULL;
    AjPStr osstr     = NULL;
    AjPStr xstr      = NULL;
    AjPStrTok handle = NULL;
    
    AjPAtom atom     = NULL;
    AjPResidue res     = NULL;
    ajint      rn_last = -100000;
    ajint      mn_last = -100000;


    /* Initialise strings */
    line  = ajStrNew();
    token = ajStrNew();
    idstr = ajStrNew();
    destr = ajStrNew();
    osstr = ajStrNew();
    xstr  = ajStrNew();

    /* Start of main application loop */
    while(ajReadlineTrim(inf,&line))
    {
	if(ajStrPrefixC(line,"XX"))
	    continue;

	/* Parse ID */
	if(ajStrPrefixC(line,"ID"))
	{
	    ajStrTokenAssignC(&handle,line," \n\t\r");
	    ajStrTokenNextParse(&handle,&token);
	    ajStrTokenNextParse(&handle,&idstr);
	    continue;
	}

	
	/* Parse number of chains */
	if(ajStrPrefixC(line,"CN"))
	{
	    ajStrTokenAssignC(&handle,line," []\n\t\r");
	    ajStrTokenNextParse(&handle,&token);
	    ajStrTokenNextParse(&handle,&token);
	    ajStrToInt(token,&nc);
	    continue;
	}
	

	/* Parse description text */
	if(ajStrPrefixC(line,"DE"))
	{
	    ajStrTokenAssignC(&handle, line, " ");
	    ajStrTokenNextParse(&handle, &token);
	    /* 'DE' */
	    ajStrTokenNextParseC(&handle, "\n\r", &token);

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
	if(ajStrPrefixC(line,"OS"))
	{
	    ajStrTokenAssignC(&handle, line, " ");
	    ajStrTokenNextParse(&handle, &token);
	    /* 'OS' */
	    ajStrTokenNextParseC(&handle, "\n\r", &token);

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
	if(ajStrPrefixC(line,"EX"))
	{
	    ajStrTokenAssignC(&handle,line," ;\n\t\r");
	    ajStrTokenNextParse(&handle,&token);
	    ajStrTokenNextParse(&handle,&token);

	    ajStrTokenNextParse(&handle,&xstr); /* method */
	    ajStrTokenNextParse(&handle,&token);

	    ajStrTokenNextParse(&handle,&token); /* reso */
	    ajStrToFloat(token,&reso);
	    ajStrTokenNextParse(&handle,&token);
	    ajStrTokenNextParse(&handle,&token); /* nmod */
	    ajStrToInt(token,&nmod);
	    ajStrTokenNextParse(&handle,&token);

	    ajStrTokenNextParse(&handle,&token); /* nchn */
	    ajStrToInt(token,&ncha);

	    ajStrTokenNextParse(&handle,&token); /* nlig */
	    ajStrToInt(token,&ngrp);

	    ret = ajPdbNew(ncha);

	    ajStrAssignS(&(ret)->Pdb,idstr);
	    ajStrAssignS(&(ret)->Compnd,destr);
	    ajStrAssignS(&(ret)->Source,osstr);

	    if(ajStrMatchC(xstr,"xray"))
		(ret)->Method = ajXRAY;
	    else
		(ret)->Method = ajNMR;

	    (ret)->Reso = reso;
	    (ret)->Nmod = nmod;
	    (ret)->Nchn = ncha;
	    (ret)->Ngp  = ngrp;
	}
	

	/* Parse information line */
	if(ajStrPrefixC(line,"IN"))
	{
	    ajStrTokenAssignC(&handle,line," ;\n\t\r");
	    ajStrTokenNextParse(&handle,&token);
	    ajStrTokenNextParse(&handle,&token);
	    ajStrTokenNextParse(&handle,&token); /* id value */
	    (ret)->Chains[nc-1]->Id=*ajStrGetPtr(token);
	    ajStrTokenNextParse(&handle,&token);
	    ajStrTokenNextParse(&handle,&token); /* residues */
	    ajStrToInt(token,&(ret)->Chains[nc-1]->Nres);
	    ajStrTokenNextParse(&handle,&token);
	    /* hetatm */
	    ajStrTokenNextParse(&handle,&token); 
	    ajStrToInt(token,&(ret)->Chains[nc-1]->Nlig);
	    /* helices */
	    ajStrTokenNextParse(&handle,&token); 
	    ajStrToInt(token,&(ret)->Chains[nc-1]->numHelices);
	    /* strands */
	    ajStrTokenNextParse(&handle,&token); 
	    ajStrToInt(token,&(ret)->Chains[nc-1]->numStrands);
	    /* sheets */
	    /*
	    ajStrTokenNextParse(&handle,&token); 
	    ajStrToInt(token,&(ret)->Chains[nc-1]->numSheets);
	    */
	    /* turns */
	    /*
	    ajStrTokenNextParse(&handle,&token); 
	    ajStrToInt(token,&(ret)->Chains[nc-1]->numTurns);
	    */
	    continue;
	}
  

	/* Parse sequence line */
	if(ajStrPrefixC(line,"SQ"))
	{
	    while(ajReadlineTrim(inf,&line) && !ajStrPrefixC(line,"XX"))
		ajStrAppendC(&(ret)->Chains[nc-1]->Seq,ajStrGetPtr(line));
	    ajStrRemoveWhite(&(ret)->Chains[nc-1]->Seq);

	    continue;
	}


	/* Parse coordinate line */
	if(ajStrPrefixC(line,"CO"))
	{
	    mod = chn = gpn = 0;
	    
	    ajStrTokenAssignC(&handle,line," \t\n\r");
	    ajStrTokenNextParse(&handle,&token);

	    ajStrTokenNextParse(&handle,&token);
	    ajStrToInt(token,&mod);

	    ajStrTokenNextParse(&handle,&token);
	    ajStrToInt(token,&chn);

	    ajStrTokenNextParse(&handle,&token);
	    ajStrToInt(token,&gpn);
	    
	    /* AJNEW0(atom); */
	    atom = ajAtomNew();
	    
	    atom->Mod = mod;
	    atom->Chn = chn;
	    atom->Gpn = gpn;
	    
	    ajStrTokenNextParse(&handle,&token);
	    atom->Type = *ajStrGetPtr(token);
	    
	    ajStrTokenNextParse(&handle,&token);
	    ajStrToInt(token,&atom->Idx);

	    ajStrTokenNextParse(&handle,&token);
	    ajStrAssignS(&atom->Pdb,token);

	    
	    /* Residue object */
	    if(atom->Type=='P')
	    {
		/* New model */
		if(atom->Mod != mn_last)
		{
		    rn_last = -100000;
		    mn_last = atom->Mod;
		}

		/* New residue */
		if(atom->Idx != rn_last)
		{
		    res = ajResidueNew();

		    res->Mod     = atom->Mod;
		    res->Chn     = atom->Chn;
		    res->Idx     = atom->Idx;
		    ajStrAssignS(&res->Pdb, atom->Pdb);
		}	
	    }	

	    ajStrTokenNextParse(&handle,&token);
	    res->eType = *ajStrGetPtr(token);

	    ajStrTokenNextParse(&handle,&token);
	    ajStrToInt(token,&res->eNum);

	    ajStrTokenNextParse(&handle,&token);
	    ajStrAssignS(&res->eId,token);
	    
	    ajStrTokenNextParse(&handle,&token);
	    ajStrToInt(token,&res->eClass);

	    ajStrTokenNextParse(&handle,&token);
	    res->eStrideType = *ajStrGetPtr(token);

	    ajStrTokenNextParse(&handle,&token);
	    ajStrToInt(token,&res->eStrideNum);

	    ajStrTokenNextParse(&handle,&token);
	    atom->Id1 = *ajStrGetPtr(token);
	    res->Id1  = atom->Id1;
	    
	    ajStrTokenNextParse(&handle,&token);
	    ajStrAssignS(&atom->Id3,token);
	    ajStrAssignS(&res->Id3, atom->Id3);

	    ajStrTokenNextParse(&handle,&token);
	    ajStrAssignS(&atom->Atm,token);

	    ajStrTokenNextParse(&handle,&token);
	    ajStrToFloat(token,&atom->X);

	    ajStrTokenNextParse(&handle,&token);
	    ajStrToFloat(token,&atom->Y);

	    ajStrTokenNextParse(&handle,&token);
	    ajStrToFloat(token,&atom->Z);

	    ajStrTokenNextParse(&handle,&token);
	    ajStrToFloat(token,&atom->O);

	    ajStrTokenNextParse(&handle,&token);
	    ajStrToFloat(token,&atom->B);

	    ajStrTokenNextParse(&handle,&token);
            ajStrToFloat(token,&res->Phi);

            ajStrTokenNextParse(&handle,&token);
            ajStrToFloat(token,&res->Psi);

            ajStrTokenNextParse(&handle,&token);
            ajStrToFloat(token,&res->Area);

            ajStrTokenNextParse(&handle,&token);
            ajStrToFloat(token,&res->all_abs);

            ajStrTokenNextParse(&handle,&token);
            ajStrToFloat(token,&res->all_rel);

            ajStrTokenNextParse(&handle,&token);
            ajStrToFloat(token,&res->side_abs);

            ajStrTokenNextParse(&handle,&token);
            ajStrToFloat(token,&res->side_rel);

            ajStrTokenNextParse(&handle,&token);
            ajStrToFloat(token,&res->main_abs);

            ajStrTokenNextParse(&handle,&token);
            ajStrToFloat(token,&res->main_rel);

            ajStrTokenNextParse(&handle,&token);
            ajStrToFloat(token,&res->npol_abs);

            ajStrTokenNextParse(&handle,&token);
            ajStrToFloat(token,&res->npol_rel);

            ajStrTokenNextParse(&handle,&token);
            ajStrToFloat(token,&res->pol_abs);

            ajStrTokenNextParse(&handle,&token);
            ajStrToFloat(token,&res->pol_rel);


	    /* Check for coordinates for water or groups that could not
	       be uniquely assigned to a chain */
	    if(chn==0)
	    {
		/* Heterogen */
		if(atom->Type == 'H')
		    ajListPushAppend((ret)->Groups,(void *)atom);
		else if(atom->Type == 'W')
		    ajListPushAppend((ret)->Water,(void *)atom);
		else
		    ajFatal("Unexpected parse error in ajPdbRead");
	    }
	    else
		ajListPushAppend((ret)->Chains[chn-1]->Atoms,(void *)atom);

	    
	    ajListPushAppend((ret)->Chains[chn-1]->Residues,(void *)res);
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
** @@
****************************************************************************/

AjPPdb ajPdbReadoldFirstModelNew(AjPFile inf) 
{
    AjPPdb ret = NULL;
    
    ajint nmod = 0;
    ajint ncha = 0;
    ajint ngrp = 0;
    ajint nc   = 0;
    ajint mod  = 0;
    ajint chn  = 0;
    ajint gpn  = 0;

    float reso = 0.0;

    AjPStr line      = NULL;
    AjPStr token     = NULL;
    AjPStr idstr     = NULL;
    AjPStr destr     = NULL;
    AjPStr osstr     = NULL;
    AjPStr xstr      = NULL;
    AjPStrTok handle = NULL;
    
    AjPAtom atom     = NULL;
    AjPResidue res     = NULL;
    ajint      rn_last = -100000;
    ajint      mn_last = -100000;

    /* Initialise strings */
    line  = ajStrNew();
    token = ajStrNew();
    idstr = ajStrNew();
    destr = ajStrNew();
    osstr = ajStrNew();
    xstr  = ajStrNew();

    /* Start of main application loop */
    while(ajReadlineTrim(inf,&line))
    {
	if(ajStrPrefixC(line,"XX"))
	    continue;

	/* Parse ID */
	if(ajStrPrefixC(line,"ID"))
	{
	    ajStrTokenAssignC(&handle,line," \n\t\r");
	    ajStrTokenNextParse(&handle,&token);
	    ajStrTokenNextParse(&handle,&idstr);
	    continue;
	}

	
	/* Parse number of chains */
	if(ajStrPrefixC(line,"CN"))
	{
	    ajStrTokenAssignC(&handle,line," []\n\t\r");
	    ajStrTokenNextParse(&handle,&token);
	    ajStrTokenNextParse(&handle,&token);
	    ajStrToInt(token,&nc);
	    continue;
	}
	

	/* Parse description text */
	if(ajStrPrefixC(line,"DE"))
	{
	    ajStrTokenAssignC(&handle, line, " ");
	    ajStrTokenNextParse(&handle, &token);
	    /* 'DE' */
	    ajStrTokenNextParseC(&handle, "\n\r", &token);

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
	if(ajStrPrefixC(line,"OS"))
	{
	    ajStrTokenAssignC(&handle, line, " ");
	    ajStrTokenNextParse(&handle, &token);
	    /* 'OS' */
	    ajStrTokenNextParseC(&handle, "\n\r", &token);

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
	if(ajStrPrefixC(line,"EX"))
	{
	    ajStrTokenAssignC(&handle,line," ;\n\t\r");
	    ajStrTokenNextParse(&handle,&token);
	    ajStrTokenNextParse(&handle,&token);

	    ajStrTokenNextParse(&handle,&xstr); /* method */
	    ajStrTokenNextParse(&handle,&token);

	    ajStrTokenNextParse(&handle,&token); /* reso */
	    ajStrToFloat(token,&reso);
	    ajStrTokenNextParse(&handle,&token);
	    ajStrTokenNextParse(&handle,&token); /* nmod */
	    ajStrToInt(token,&nmod);
	    ajStrTokenNextParse(&handle,&token);

	    ajStrTokenNextParse(&handle,&token); /* ncha */
	    ajStrToInt(token,&ncha);

	    ajStrTokenNextParse(&handle,&token); /* nlig */
	    ajStrToInt(token,&ngrp);

	    ret = ajPdbNew(ncha);

	    ajStrAssignS(&(ret)->Pdb,idstr);
	    ajStrAssignS(&(ret)->Compnd,destr);
	    ajStrAssignS(&(ret)->Source,osstr);

	    if(ajStrMatchC(xstr,"xray"))
		(ret)->Method = ajXRAY;
	    else
		(ret)->Method = ajNMR;

	    (ret)->Reso = reso;
	    /* (ret)->Nmod = nmod; */

	    /*
	    ** Number of models is hard-coded to 1 as only the 
	    **  data for the first model is read in
	    */
	    (ret)->Nmod = 1;
	    (ret)->Nchn = ncha;
	    (ret)->Ngp  = ngrp;
	}
	

	/* Parse information line */
	if(ajStrPrefixC(line,"IN"))
	{
	    ajStrTokenAssignC(&handle,line," ;\n\t\r");
	    ajStrTokenNextParse(&handle,&token);
	    ajStrTokenNextParse(&handle,&token);
	    ajStrTokenNextParse(&handle,&token); /* id value */
	    (ret)->Chains[nc-1]->Id=*ajStrGetPtr(token);
	    ajStrTokenNextParse(&handle,&token);
	    ajStrTokenNextParse(&handle,&token); /* residues */
	    ajStrToInt(token,&(ret)->Chains[nc-1]->Nres);
	    ajStrTokenNextParse(&handle,&token);
	    /* hetatm */
	    ajStrTokenNextParse(&handle,&token); 
	    ajStrToInt(token,&(ret)->Chains[nc-1]->Nlig);
	    /* helices */
	    ajStrTokenNextParse(&handle,&token); 
	    ajStrToInt(token,&(ret)->Chains[nc-1]->numHelices);
	    /* strands */
	    ajStrTokenNextParse(&handle,&token); 
	    ajStrToInt(token,&(ret)->Chains[nc-1]->numStrands);
	    /* sheets */
	    /*
	    ajStrTokenNextParse(&handle,&token); 
	    ajStrToInt(token,&(ret)->Chains[nc-1]->numSheets);
	    */
	    /* turns */
	    /*
	    ajStrTokenNextParse(&handle,&token); 
	    ajStrToInt(token,&(ret)->Chains[nc-1]->numTurns);
	    */

	    continue;
	}
  

	/* Parse sequence line */
	if(ajStrPrefixC(line,"SQ"))
	{
	    while(ajReadlineTrim(inf,&line) && !ajStrPrefixC(line,"XX"))
		ajStrAppendC(&(ret)->Chains[nc-1]->Seq,ajStrGetPtr(line));

	    ajStrRemoveWhite(&(ret)->Chains[nc-1]->Seq);
	    continue;
	}


	/* Parse coordinate line */
	if(ajStrPrefixC(line,"CO"))
	{
	    mod = chn = gpn = 0;
	    
	    ajStrTokenAssignC(&handle,line," \t\n\r");
	    ajStrTokenNextParse(&handle,&token);

	    ajStrTokenNextParse(&handle,&token);
	    ajStrToInt(token,&mod);

	    if(mod!=1)
		break;

	    ajStrTokenNextParse(&handle,&token);
	    ajStrToInt(token,&chn);

	    ajStrTokenNextParse(&handle,&token);
	    ajStrToInt(token,&gpn);
	    
	    /* AJNEW0(atom); */
	    atom = ajAtomNew();

	    atom->Mod = mod;
	    atom->Chn = chn;
	    atom->Gpn = gpn;
	    

	    ajStrTokenNextParse(&handle,&token);
	    atom->Type = *ajStrGetPtr(token);
	    
	    ajStrTokenNextParse(&handle,&token);
	    ajStrToInt(token,&atom->Idx);

	    ajStrTokenNextParse(&handle,&token);
	    ajStrAssignS(&atom->Pdb,token);

	    /* Residue object */
	    if(atom->Type=='P')
	    {
		/* New model */
		if(atom->Mod != mn_last)
		{
		    rn_last = -100000;
		    mn_last = atom->Mod;
		}

		/* New residue */
		if(atom->Idx != rn_last)
		{
		    res = ajResidueNew();

		    res->Mod     = atom->Mod;
		    res->Chn     = atom->Chn;
		    res->Idx     = atom->Idx;
		    ajStrAssignS(&res->Pdb, atom->Pdb);
		}	
	    }


	    ajStrTokenNextParse(&handle,&token);
	    res->eType = *ajStrGetPtr(token);

	    ajStrTokenNextParse(&handle,&token);
	    ajStrToInt(token,&res->eNum);

	    ajStrTokenNextParse(&handle,&token);
	    ajStrAssignS(&res->eId,token);
	    
	    ajStrTokenNextParse(&handle,&token);
	    ajStrToInt(token,&res->eClass);

	    ajStrTokenNextParse(&handle,&token);
	    res->eStrideType = *ajStrGetPtr(token);

	    ajStrTokenNextParse(&handle,&token);
	    ajStrToInt(token,&res->eStrideNum);

	    ajStrTokenNextParse(&handle,&token);
	    atom->Id1 = *ajStrGetPtr(token);
	    
	    ajStrTokenNextParse(&handle,&token);
	    ajStrAssignS(&atom->Id3,token);
	    ajStrAssignS(&res->Id3, atom->Id3);

	    ajStrTokenNextParse(&handle,&token);
	    ajStrAssignS(&atom->Atm,token);

	    ajStrTokenNextParse(&handle,&token);
	    ajStrToFloat(token,&atom->X);

	    ajStrTokenNextParse(&handle,&token);
	    ajStrToFloat(token,&atom->Y);

	    ajStrTokenNextParse(&handle,&token);
	    ajStrToFloat(token,&atom->Z);

	    ajStrTokenNextParse(&handle,&token);
	    ajStrToFloat(token,&atom->O);

	    ajStrTokenNextParse(&handle,&token);
	    ajStrToFloat(token,&atom->B);

	    ajStrTokenNextParse(&handle,&token);
            ajStrToFloat(token,&res->Phi);

            ajStrTokenNextParse(&handle,&token);
            ajStrToFloat(token,&res->Psi);

            ajStrTokenNextParse(&handle,&token);
            ajStrToFloat(token,&res->Area);

            ajStrTokenNextParse(&handle,&token);
            ajStrToFloat(token,&res->all_abs);

            ajStrTokenNextParse(&handle,&token);
            ajStrToFloat(token,&res->all_rel);

            ajStrTokenNextParse(&handle,&token);
            ajStrToFloat(token,&res->side_abs);

            ajStrTokenNextParse(&handle,&token);
            ajStrToFloat(token,&res->side_rel);

            ajStrTokenNextParse(&handle,&token);
            ajStrToFloat(token,&res->main_abs);

            ajStrTokenNextParse(&handle,&token);
            ajStrToFloat(token,&res->main_rel);

            ajStrTokenNextParse(&handle,&token);
            ajStrToFloat(token,&res->npol_abs);

            ajStrTokenNextParse(&handle,&token);
            ajStrToFloat(token,&res->npol_rel);

            ajStrTokenNextParse(&handle,&token);
            ajStrToFloat(token,&res->pol_abs);

            ajStrTokenNextParse(&handle,&token);
            ajStrToFloat(token,&res->pol_rel);

	    /* Check for coordinates for water or groups that could not
	    ** be uniquely assigned to a chain
	    */
	    if(chn==0)
	    {
		/* Heterogen */
		if(atom->Type == 'H')
		    ajListPushAppend((ret)->Groups,(void *)atom);
		else if(atom->Type == 'W')
		    ajListPushAppend((ret)->Water,(void *)atom);
		else
		    ajFatal("Unexpected parse error in "
			    "ajPdbReadFirstModelNew");
	    }
	    else
		ajListPushAppend((ret)->Chains[chn-1]->Atoms,(void *)atom);

	    ajListPushAppend((ret)->Chains[chn-1]->Residues,(void *)res);
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




/* @func ajAtomNew *******************************************************
**
** Atom object constructor.
** This is normally called by the ajChainNew function.
**
** @return [AjPAtom] Pointer to an atom object
** @category new [AjPAtom] Default Atom constructor.
** @@
****************************************************************************/

AjPAtom ajAtomNew(void)
{
    AjPAtom ret = NULL;

    AJNEW0(ret);
    
    ret->Id3   = ajStrNew();
    ret->Atm   = ajStrNew();
    ret->Pdb   = ajStrNew();
/*    ret->eId   = ajStrNew(); */

    ret->Id1   = '.';
/*    ret->eType = '.'; 
    ajStrAssignC(&ret->eId, ".");
    ret->eStrideType = '.'; */
    

    return ret;
}




/* @func ajResidueNew *****************************************************
**
** Residue object constructor.
** This is normally called by the ajChainNew function.
**
** @return [AjPResidue] Pointer to a Residue object
** @category new [AjPResidue] Default Residue constructor.
** @@
****************************************************************************/

AjPResidue ajResidueNew(void)
{
    AjPResidue ret = NULL;
    
    AJNEW0(ret);
    
    ret->Pdb   = ajStrNew();
    ret->Id3   = ajStrNew();
    ret->eId   = ajStrNew();

    ret->Id1   = '.';
    ret->eType = '.';
    ajStrAssignC(&ret->eId, ".");
    ret->eStrideType = '.';
    
    return ret;
}




/* @func ajChainNew *********************************************************
**
** Chain object constructor. 
** This is normally called by the ajPdbNew function
**
** @return [AjPChain] Pointer to a chain object
** @@
** @category new [AjPChain] Default Chain constructor.
****************************************************************************/

AjPChain ajChainNew(void)
{
    AjPChain ret = NULL;
  
    AJNEW0(ret);

    ret->Seq      = ajStrNewC("");
    ret->Atoms    = ajListNew();
    ret->Residues = ajListNew();

    return ret;
}




/* @func ajPdbNew ***********************************************************
**
** Pdb object constructor. Fore-knowledge of the number of chains 
** is required. This is normally called by the functions that read PDB 
** files or clean coordinate files (see embpdb.c & embpdbraw.c).
**
** @param [r] n [ajint] Number of chains in this pdb file
**
** @return [AjPPdb] Pointer to a pdb object
** @category new [AjPPdb] Default Pdb constructor.
** @@
****************************************************************************/

AjPPdb ajPdbNew(ajint n)
{
    AjPPdb ret = NULL;
    ajint i;
    
    AJNEW0(ret);
  

    ret->Pdb    = ajStrNew();
    ret->Compnd = ajStrNew();
    ret->Source = ajStrNew();
    ret->Groups = ajListNew();
    ret->Water  = ajListNew();
    ret->gpid   = ajChararrNew();
    

    if(n)
    {	
	AJCNEW0(ret->Chains,n);
	for(i=0;i<n;++i)
	    ret->Chains[i] = ajChainNew();
    }

    return ret;
}




/* @func ajHetentNew ********************************************************
**
** Hetent object constructor. 
**
** @return [AjPHetent] Pointer to a Hetent object
** @category new [AjPHetent] Default Hetent constructor.
** @@
****************************************************************************/

AjPHetent ajHetentNew(void)
{
    AjPHetent ret = NULL;
    
    AJNEW0(ret);
    
    /* Create strings */
    ret->abv = ajStrNew();
    ret->syn = ajStrNew();
    ret->ful = ajStrNew();
    
    return ret;
}




/* @func ajHetNew ***********************************************************
**
** Het object constructor. 
**
** @param [r] n [ajint] Number of entries in dictionary.
** 
** @return [AjPHet] Pointer to a Het object
** @category new [AjPHet] Default Het constructor.
** @@
****************************************************************************/

AjPHet ajHetNew(ajint n)
{
    ajint i    = 0;
    AjPHet ret = NULL;
    
    AJNEW0(ret);

    if(n)
    {
	ret->n = n;
	AJCNEW0(ret->entries, n);
	for(i=0;i<n;i++)
	    ret->entries[i]=ajHetentNew();
    }
    else
    {
	ret->n = 0;
	ret->entries = NULL;
    }
    

    return ret;
}




/* @func ajVdwallNew ********************************************************
**
** Vdwall object constructor. This is normally called by the ajVdwallReadNew
** function. Fore-knowledge of the number of residues is required.
**
** @param  [r] n [ajint]  Number of residues
**
** @return [AjPVdwall] Pointer to a Vdwall object
** @category new [AjPVdwall] Default Vdwall constructor.
** @@
****************************************************************************/

AjPVdwall ajVdwallNew(ajint n)
{
    AjPVdwall ret = NULL;
    
    AJNEW0(ret);

    ret->N = n;

    if(n)
	AJCNEW0(ret->Res, n);


    return ret;
}




/* @func ajVdwresNew ********************************************************
**
** Vdwres object constructor. This is normally called by the ajVdwallReadNew
** function. Fore-knowledge of the number of atoms is required.
**
** @param  [r] n [ajint]  Number of atoms
**
** @return [AjPVdwres] Pointer to a Vdwres object
** @category new [AjPVdwres] Default Vdwres constructor.
** @@
****************************************************************************/

AjPVdwres  ajVdwresNew(ajint n)
{
    ajint x;
    AjPVdwres ret = NULL;
    
    AJNEW0(ret);

    ret->Id3 = ajStrNew();    
    ret->N   = n;

    if(n)
    {
	AJCNEW0(ret->Atm, n);

	for(x=0;x<n;++x)
	    ret->Atm[x]=ajStrNew();

	AJCNEW0(ret->Rad, n);
    }

    return ret;
}




/* @func ajCmapNew **********************************************************
**
** Cmap object constructor. This is normally called by the ajCmapReadNew
** function. Fore-knowledge of the dimension (number of residues) for the 
** contact map is required.
**
** @param  [r] n [ajint]   Dimension of contact map
**
** @return [AjPCmap] Pointer to a Cmap object
** 
** @category new [AjPCmap] Default Cmap constructor.
** @@
****************************************************************************/

AjPCmap ajCmapNew(ajint n)
{
    AjPCmap ret = NULL;
    ajint z = 0;
    

    AJNEW0(ret);

    ret->Id    = ajStrNew();    	
    ret->Domid = ajStrNew();    	
    ret->Ligid = ajStrNew();    	
    ret->Seq1  = ajStrNew();
    ret->Seq2  = ajStrNew();
    ret->Desc  = ajStrNew();
    ret->Chid1 = '.';
    ret->Chid2 = '.';
       

    if(n)
    {
	/* Create the SQUARE contact map */
	ret->Mat = ajUint2dNewRes((ajint)n);

	for(z=0;z<n;++z)
	    ajUint2dPut(&ret->Mat, z, n-1, (ajint) 0);
    }

    ret->Dim  = n;
    ret->Ncon = 0;

    return ret;
}




/* @func ajPdbtospNew *******************************************************
**
** Pdbtosp object constructor. Fore-knowledge of the number of entries is 
** required. This is normally called by the ajPdbtospReadCNew / 
** ajPdbtospReadNew functions.
**
** @param [r] n [ajint] Number of entries
**
** @return [AjPPdbtosp] Pointer to a Pdbtosp object
** @category new [AjPPdbtosp] Default Pdbtosp constructor.
** @@
****************************************************************************/

AjPPdbtosp ajPdbtospNew(ajint n)
{

    AjPPdbtosp ret = NULL;
    ajint i;

    AJNEW0(ret);

    ret->Pdb  = ajStrNew();

    if(n)
    {
	AJCNEW0(ret->Acc,n);
	AJCNEW0(ret->Spr,n);

	for(i=0; i<n; i++)
	{
	    ret->Acc[i]=ajStrNew();
	    ret->Spr[i]=ajStrNew();
	}
    }

    ret->n = n;

    return ret;
}




/* ======================================================================= */
/* =========================== destructors =============================== */
/* ======================================================================= */




/* @section Destructors *****************************************************
**
** All destructor functions receive the address of the instance to be
** deleted.  The original pointer is set to NULL so is ready for re-use.
**
****************************************************************************/




/* @func ajAtomDel **********************************************************
**
** Destructor for atom object.
**
** @param [d] ptr [AjPAtom*] Atom object pointer
**
** @return [void]
** @category delete [AjPAtom] Default Atom destructor.
** @@
****************************************************************************/

void ajAtomDel(AjPAtom *ptr)
{
    AjPAtom pthis;

    pthis = *ptr;

    if(!ptr || !pthis)
	return;

    ajStrDel(&pthis->Id3);
    ajStrDel(&pthis->Atm);
    ajStrDel(&pthis->Pdb);
/*    ajStrDel(&pthis->eId); */

    AJFREE(pthis);
    (*ptr) = NULL;

    return;
}




/* @func ajResidueDel **********************************************************
**
** Destructor for residue object.
**
** @param [d] ptr [AjPResidue*] Residue object pointer
**
** @return [void]
** @category delete [AjPResidue] Default Residue destructor.
** @@
****************************************************************************/

void ajResidueDel(AjPResidue *ptr)
{
    AjPResidue pthis;

    pthis = *ptr;

    if(!ptr || !pthis)
	return;

    ajStrDel(&pthis->Pdb);
    ajStrDel(&pthis->Id3);
    ajStrDel(&pthis->eId);

    AJFREE(pthis);
    (*ptr) = NULL;

    return;
}




/* @func ajChainDel *********************************************************
**
** Destructor for chain object.
**
** @param [d] ptr [AjPChain*] Chain object pointer
**
** @return [void]
** @category delete [AjPChain] Default Chain destructor.
** @@
****************************************************************************/

void ajChainDel(AjPChain *ptr)
{
    AjPChain pthis;
    AjPAtom    atm = NULL;
    AjPResidue res = NULL;

    pthis = *ptr;

    if(!ptr || !pthis)
	return;
    
    while(ajListPop(pthis->Atoms,(void **)&atm))
	ajAtomDel(&atm);

    while(ajListPop(pthis->Residues,(void **)&res))
	ajResidueDel(&res);

    ajStrDel(&pthis->Seq);
    ajListFree(&pthis->Atoms);
    ajListFree(&pthis->Residues);

    AJFREE(pthis);
    (*ptr) = NULL;

    return;
}




/* @func ajPdbDel ***********************************************************
**
** Destructor for pdb object.
**
** @param [d] ptr [AjPPdb*] Pdb object pointer
**
** @return [void]
** @category delete [AjPPdb] Default Pdb destructor.
** @@
****************************************************************************/

void ajPdbDel(AjPPdb *ptr)
{
    AjPPdb pthis;
    AjPAtom atm = NULL;

    ajint nc = 0;
    ajint i  = 0;

    pthis = *ptr;

    if(!pthis || !ptr)
	return;
    
    nc = pthis->Nchn;

    ajStrDel(&pthis->Pdb);
    ajStrDel(&pthis->Compnd);
    ajStrDel(&pthis->Source);

    ajChararrDel(&pthis->gpid);
    
    
    while(ajListPop(pthis->Water,(void **)&atm))
	ajAtomDel(&atm);

    ajListFree(&pthis->Water);

    while(ajListPop(pthis->Groups,(void **)&atm))
	ajAtomDel(&atm);

    ajListFree(&pthis->Groups);

    for(i=0;i<nc;++i)
	ajChainDel(&pthis->Chains[i]);

    AJFREE(pthis->Chains);

    AJFREE(pthis);
    (*ptr) = NULL;

    return;
}




/* @func ajHetentDel ********************************************************
**
** Destructor for Hetent object.
**
** @param [d] ptr [AjPHetent*] Hetent object pointer
**
** @return [void]
** @category delete [AjPHetent] Default Hetent destructor.
** @@
****************************************************************************/

void ajHetentDel(AjPHetent *ptr)
{
    /* Check arg's */
    if(*ptr==NULL)
	return;

   
    if((*ptr)->abv)
	ajStrDel( &((*ptr)->abv)); 

    if((*ptr)->syn)
	ajStrDel( &((*ptr)->syn)); 

    if((*ptr)->ful)
	ajStrDel( &((*ptr)->ful)); 

    AJFREE(*ptr);
    *ptr = NULL;
    
    return;
}




/* @func ajHetDel ***********************************************************
**
** Destructor for Het object.
**
** @param [d] ptr [AjPHet*] Het object pointer
**
** @return [void]
** @category delete [AjPHet] Default Het destructor.
** @@
****************************************************************************/

void ajHetDel(AjPHet *ptr)
{
    ajint i = 0;
    
    /* Check arg's */
    if(*ptr==NULL)
	return;

    if((*ptr)->entries)
    {
        for(i=0;i<(*ptr)->n;i++)
	{
	    if((*ptr)->entries[i])
		ajHetentDel(&((*ptr)->entries[i]));
	}	
	
	AJFREE((*ptr)->entries);
    }

    AJFREE(*ptr);
    *ptr = NULL;

    return;
}




/* @func ajVdwallDel ********************************************************
**
** Destructor for Vdwall object.
**
** @param [d] ptr [AjPVdwall*] Vdwall object pointer
**
** @return [void]
** @category delete [AjPVdwall] Default Vdwall destructor.
** @@
****************************************************************************/

void ajVdwallDel(AjPVdwall *ptr)
{
    ajint x = 0;
    
    for(x=0;x<(*ptr)->N; ++x)
	ajVdwresDel(&(*ptr)->Res[x]);
    
    AJFREE((*ptr)->Res);
    AJFREE(*ptr);    
    *ptr = NULL;

    return;
}




/* @func ajVdwresDel ********************************************************
**
** Destructor for Vdwres object.
**
** @param [d] ptr [AjPVdwres*] Vdwres object pointer
**
** @return [void]
** @category delete [AjPVdwres] Default Vdwres destructor.
** @@
****************************************************************************/

void ajVdwresDel(AjPVdwres *ptr)
{
    ajint x = 0;
    
    ajStrDel(&(*ptr)->Id3);
    
    for(x=0;x<(*ptr)->N; ++x)
	ajStrDel(&(*ptr)->Atm[x]);
    
    AJFREE((*ptr)->Atm);
    AJFREE((*ptr)->Rad);
    AJFREE(*ptr);    
    *ptr = NULL;

    return;
}




/* @func ajCmapDel **********************************************************
**
** Destructor for Cmap object.
**
** @param [d] ptr [AjPCmap*] Cmap object pointer
**
** @return [void]
** @category delete [AjPCmap] Default Cmap destructor.
** @@
****************************************************************************/

void ajCmapDel(AjPCmap *ptr)
{
    if(!ptr)
	return;

    if(!(*ptr))
	return;

    if((*ptr)->Id)
	ajStrDel(&(*ptr)->Id);

    if((*ptr)->Domid)
	ajStrDel(&(*ptr)->Domid);

    if((*ptr)->Ligid)
	ajStrDel(&(*ptr)->Ligid);

    if((*ptr)->Seq1)
	ajStrDel(&(*ptr)->Seq1);

    if((*ptr)->Seq2)
	ajStrDel(&(*ptr)->Seq2);

    if((*ptr)->Desc)
        ajStrDel(&(*ptr)->Desc);

    if((*ptr)->Mat)
	ajUint2dDel(&(*ptr)->Mat);

    if((*ptr))
	AJFREE(*ptr);    

    *ptr = NULL;

    return;
}




/* @func ajPdbtospDel *******************************************************
**
** Destructor for Pdbtosp object.
**
** @param [d] ptr [AjPPdbtosp*] Pdbtosp object pointer
**
** @return [void]
** @category delete [AjPPdbtosp] Default Pdbtosp destructor.
** @@
****************************************************************************/

void ajPdbtospDel(AjPPdbtosp *ptr)
{
    AjPPdbtosp pthis;
    ajint i;

    pthis = *ptr;

    if(!pthis || !ptr)
	return;

    ajStrDel(&pthis->Pdb);

    if(pthis->n)
    {
	for(i=0; i<pthis->n; i++)
	{
	    ajStrDel(&pthis->Acc[i]);
	    ajStrDel(&pthis->Spr[i]);
	}

	AJFREE(pthis->Acc);
	AJFREE(pthis->Spr);
    }

    AJFREE(pthis);
    (*ptr) = NULL;

    return;
}




/* ======================================================================= */
/* ============================ Assignments ============================== */
/* ======================================================================= */




/* @section Assignments *****************************************************
**
** These functions overwrite the instance provided as the first argument
** A NULL value is always acceptable so these functions are often used to
** create a new instance by assignment.
**
****************************************************************************/




/* @func ajAtomCopy *********************************************************
**
** Copies the data from an Atom object to an Atom object; the new object
** is created if needed. 
** 
** IMPORTANT - THIS DOES NOT COPY THE eNum & eType ELEMENTS, WHICH ARE SET 
** TO ZERO and '.' INSTEAD.
** 
** @param [w] to   [AjPAtom*]  Atom object pointer
** @param [r] from [const AjPAtom]   Atom object pointer
**
** @return [AjBool] True on success
** @@
****************************************************************************/

AjBool ajAtomCopy(AjPAtom *to, const AjPAtom from)
{
    if(!to)
    {
	ajWarn("Bad arg (NULL) passed to ajAtomCopy");
	return ajFalse;
    }

    if(!(*to))
	*to = ajAtomNew();

    (*to)->Mod   = from->Mod;
    (*to)->Chn   = from->Chn;    
    (*to)->Gpn   = from->Gpn;
    (*to)->Idx   = from->Idx;
    ajStrAssignS(&((*to)->Pdb), from->Pdb);
    (*to)->Id1   = from->Id1;
    ajStrAssignS(&((*to)->Id3), from->Id3);
    (*to)->Type  = from->Type;
    ajStrAssignS(&((*to)->Atm), from->Atm);
    (*to)->X     = from->X;
    (*to)->Y     = from->Y;
    (*to)->Z     = from->Z;
    (*to)->O     = from->O;
    (*to)->B     = from->B;

    return ajTrue;
}




/* @func ajResidueCopy ******************************************************
**
** Copies the data from a Residue object to an Residue object; the new object
** is created if needed. 
** 
** IMPORTANT - THIS DOES NOT COPY THE eNum & eType ELEMENTS, WHICH ARE SET 
** TO ZERO and '.' INSTEAD.
** 
** @param [w] to   [AjPResidue*]  Residue object pointer
** @param [r] from [const AjPResidue]   Residue object pointer
**
** @return [AjBool] True on success
** @@
****************************************************************************/

AjBool ajResidueCopy(AjPResidue *to, const AjPResidue from)
{
    if(!to)
    {
	ajWarn("Bad arg (NULL) passed to ajResidueCopy");

	return ajFalse;
    }

    if(!(*to))
	*to = ajResidueNew();

    (*to)->Mod   = from->Mod;
    (*to)->Chn   = from->Chn;    
    (*to)->Idx   = from->Idx;
    ajStrAssignS(&((*to)->Pdb), from->Pdb);
    (*to)->Id1   = from->Id1;
    ajStrAssignS(&((*to)->Id3), from->Id3);
    (*to)->Phi     = from->Phi;
    (*to)->Psi     = from->Psi;
    (*to)->Area    = from->Area;

    (*to)->eNum  = from->eNum; 
    ajStrAssignS(&((*to)->eId), from->eId);
    (*to)->eType = from->eType;
    (*to)->eClass= from->eClass; 
    (*to)->eStrideNum  = from->eStrideNum; 
    (*to)->eStrideType = from->eStrideType; 

    (*to)->all_abs  = from->all_abs; 
    (*to)->all_rel  = from->all_rel; 
    (*to)->side_abs = from->side_abs; 
    (*to)->side_rel = from->side_rel; 
    (*to)->main_abs = from->main_abs;     
    (*to)->main_rel = from->main_rel; 
    (*to)->npol_abs = from->npol_abs; 
    (*to)->npol_rel = from->npol_rel; 
    (*to)->pol_abs  = from->pol_abs; 
    (*to)->pol_rel  = from->pol_rel; 


    return ajTrue;
}




/* @func ajAtomListCopy *****************************************************
**
** Read a list of Atom structures and writes a copy of the list.  The 
** data are copied and the list is created if necessary.
** 
** @param [w] to       [AjPList *] List of Atom objects to write
** @param [r] from     [const AjPList]   List of Atom objects to read
**
** @return [AjBool] True if list was copied ok.
** @@
****************************************************************************/

AjBool ajAtomListCopy(AjPList *to, const AjPList from)
{
/* AjPList ret  = NULL; */
   AjIList iter = NULL;
   AjPAtom hit  = NULL;
   AjPAtom new  = NULL;

   /* Check arg's */
   if(!from || !to)
   {
       ajWarn("Bad arg's passed to ajAtomListCopy\n");
       return ajFalse;
   }
    
   /* Allocate the new list, if necessary */
   if(!(*to))
       *to=ajListNew();
    
   /* Initialise the iterator */
   iter=ajListIterNewread(from);
    
   /* Iterate through the list of Atom objects */
   while((hit=(AjPAtom)ajListIterGet(iter)))
   {
       new=ajAtomNew();
	
       ajAtomCopy(&new, hit);

       /* Push scophit onto list */
       ajListPushAppend(*to, new);
   }


   ajListIterDel(&iter);

   return ajTrue; 
}




/* @func ajResidueListCopy **************************************************
**
** Read a list of Residue structures and writes a copy of the list.  The 
** data are copied and the list is created if necessary.
** 
** @param [w] to       [AjPList *] List of Residue objects to write
** @param [r] from     [const AjPList]   List of Residue objects to read
**
** @return [AjBool] True if list was copied ok.
** @@
****************************************************************************/

AjBool ajResidueListCopy(AjPList *to, const AjPList from)
{
/* AjPList ret  = NULL; */
   AjIList iter = NULL;
   AjPResidue hit  = NULL;
   AjPResidue new  = NULL;

   /* Check arg's */
   if(!from || !to)
   {
       ajWarn("Bad arg's passed to ajResidueListCopy\n");
       return ajFalse;
   }
    
   /* Allocate the new list, if necessary */
   if(!(*to))
       *to=ajListNew();
    
   /* Initialise the iterator */
   iter=ajListIterNewread(from);
    
   /* Iterate through the list of Atom objects */
   while((hit=(AjPResidue)ajListIterGet(iter)))
   {
       new=ajResidueNew();
	
       ajResidueCopy(&new, hit);

       /* Push scophit onto list */
       ajListPushAppend(*to, new);
   }


   ajListIterDel(&iter);
   return ajTrue; 
}




/* @func ajPdbCopy **********************************************************
**
** Copies data from one Pdb object to another; the new object is 
** always created. 
** 
** 
** @param [w] to   [AjPPdb*] Pdb object pointer
** @param [r] from [const AjPPdb]  Pdb object pointer
**
** @return [AjBool] True on success
** @@
****************************************************************************/

AjBool ajPdbCopy(AjPPdb *to, const AjPPdb from)
{
    ajint x = 0;
    

    if(!from)
    {
	ajWarn("NULL arg passed to ajPdbCopy");

	return ajFalse;
    }

    if((*to))
    {
	ajWarn("Pointer passed to ajPdbCopy should be NULL but isn't !");

	return ajFalse;
    }	


    /* Copy data in Pdb object */
    (*to) = ajPdbNew(from->Nchn);
    ajStrAssignS(&((*to)->Pdb), from->Pdb);
    ajStrAssignS(&((*to)->Compnd), from->Compnd);
    ajStrAssignS(&((*to)->Source), from->Source);
    (*to)->Method = from->Method;
    (*to)->Reso   = from->Reso;
    (*to)->Nmod   = from->Nmod;    
    (*to)->Nchn   = from->Nchn;    
    (*to)->Ngp    = from->Ngp;

    for(x=0;x<from->Ngp;x++)
	ajChararrPut(&((*to)->gpid), x, ajChararrGet(from->gpid, x));
       
    ajListFree(&((*to)->Groups));
    ajListFree(&((*to)->Water));

/*    (*to)->Groups = ajAtomListCopy(from->Groups);
    (*to)->Water  = ajAtomListCopy(from->Water); */

    if(!ajAtomListCopy(&(*to)->Groups, from->Groups))
	ajFatal("Error copying Groups list");
    
    if(!ajAtomListCopy(&(*to)->Water, from->Water))
	ajFatal("Error copying Water list");
    
    /* Copy data in Chain objects */
    for(x=0;x<from->Nchn;x++)
    {
	ajListFree(&((*to)->Chains[x]->Atoms));
	
	(*to)->Chains[x]->Id         = from->Chains[x]->Id;
	(*to)->Chains[x]->Nres       = from->Chains[x]->Nres;
	(*to)->Chains[x]->Nlig       = from->Chains[x]->Nlig;
	(*to)->Chains[x]->numHelices = from->Chains[x]->numHelices;
	(*to)->Chains[x]->numStrands = from->Chains[x]->numStrands;
	ajStrAssignS(&((*to)->Chains[x]->Seq), from->Chains[x]->Seq);
/*	(*to)->Chains[x]->Atoms = ajAtomListCopy(from->Chains[x]->Atoms); */
	if(!ajAtomListCopy(&(*to)->Chains[x]->Atoms, from->Chains[x]->Atoms))
	    ajFatal("Error copying Atoms list");	    
	if(!ajResidueListCopy(&(*to)->Chains[x]->Residues,
                              from->Chains[x]->Residues))
	    ajFatal("Error copying Residues list");	    
    }
    

    return ajTrue;
}




/* ======================================================================= */
/* ============================= Modifiers =============================== */
/* ======================================================================= */




/* @section Modifiers *******************************************************
**
** These functions use the contents of an instance and update them.
**
****************************************************************************/




/* ======================================================================= */
/* ========================== Operators ===================================*/
/* ======================================================================= */




/* @section Operators *******************************************************
**
** These functions use the contents of an instance but do not make any 
** changes.
**
****************************************************************************/




/* @func ajResidueSSEnv *******************************************************
**
** Assigns secondary structure environment of a residue
**
** @param  [r] res [const AjPResidue]    Residue object
** @param  [w] SEnv [char*] Character for the Secondary structure environment
** @param  [w] flog [AjPFile] Log file
** @return [AjBool] ajTrue on success
** @@
******************************************************************************/

AjBool   ajResidueSSEnv(const AjPResidue res, char *SEnv, AjPFile flog)
{
    *SEnv='\0';
    ajFmtPrintF(flog, "R:%c-%d S:%c A:%.2f\n", res->Id1, res->Idx, 
		res->eStrideType, res->side_rel);
  
    if(res->eStrideType == 'H' ||
       res->eStrideType == 'G')
	*SEnv='H';
    else if(res->eStrideType == 'E' ||
	    res->eStrideType == 'B' ||
	    res->eStrideType == 'b')
	*SEnv='S';
    else if(res->eStrideType == 'T' ||
	    res->eStrideType == 'C' ||
	    res->eStrideType == 'I')
	*SEnv='C';
    /*If no stride assignment, get pdb assignment*/
    else if(res->eStrideType == '.')
    {
	if(res->eType == 'H')
	    *SEnv='H';
	else if(res->eType == 'E')
	    *SEnv='S';
	else if(res->eType == 'C' ||
		res->eType == 'T')
	    *SEnv='C';
	else if(res->eType == '.')
	{
	    ajFmtPrintF(flog, "SEnv unknown for residue %d\n", res->Idx);
	    /* *SEnv='C';  */
	    *SEnv='\0';  
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
** @param [r] res [const AjPResidue] Residue object
** @param [r] SEnv [char]          Secondary structure environment code
** @param [w] OEnv [AjPStr*]       Character for the overall environment class 
** @param  [w] flog [AjPFile] Log file
** @return [ajint] Number of environments
** @@
******************************************************************************/

ajint ajResidueEnv1(const AjPResidue res, char SEnv, AjPStr *OEnv, AjPFile flog)
{
    if(SEnv=='\0')
    {
	ajStrSetClear(OEnv);

	return 0;
    }
  
    if((res->side_rel <= 15) && (SEnv == 'H'))
	ajStrAssignC(OEnv,"AA");
    else if((res->side_rel <= 15) && (SEnv == 'S'))
	ajStrAssignC(OEnv,"AB");
    else if((res->side_rel <= 15) && (SEnv == 'C'))
	ajStrAssignC(OEnv,"AC");
    else if((res->side_rel > 15) && (res->side_rel <= 30) && (SEnv == 'H'))
	ajStrAssignC(OEnv,"AD"); 
    else if((res->side_rel > 15) && (res->side_rel <= 30) && (SEnv == 'S'))
	ajStrAssignC(OEnv,"AE");
    else if((res->side_rel > 15) && (res->side_rel <= 30) && (SEnv == 'C'))
	ajStrAssignC(OEnv,"AF");
    else if((res->side_rel > 30) && (res->side_rel <= 45) && (SEnv == 'H'))
	ajStrAssignC(OEnv,"AG");
    else if((res->side_rel > 30) && (res->side_rel <= 45) && (SEnv == 'S'))
	ajStrAssignC(OEnv,"AH");
    else if((res->side_rel > 30) && (res->side_rel <= 45) && (SEnv == 'C'))
	ajStrAssignC(OEnv,"AI");
    else if((res->side_rel > 45) && (res->side_rel <= 60) && (SEnv == 'H'))
	ajStrAssignC(OEnv,"AJ");
    else if((res->side_rel > 45) && (res->side_rel <= 60) && (SEnv == 'S'))
	ajStrAssignC(OEnv,"AK");
    else if((res->side_rel > 45) && (res->side_rel <= 60) && (SEnv == 'C'))
	ajStrAssignC(OEnv,"AL");
    else if((res->side_rel > 60) && (res->side_rel <= 75) && (SEnv == 'H'))
	ajStrAssignC(OEnv,"AM");
    else if((res->side_rel > 60) && (res->side_rel <= 75) && (SEnv == 'S'))
	ajStrAssignC(OEnv,"AN");  
    else if((res->side_rel > 60) && (res->side_rel <= 75) && (SEnv == 'C'))
	ajStrAssignC(OEnv,"AO"); 
    else if((res->side_rel > 75) && (res->side_rel <= 90) && (SEnv == 'H'))
	ajStrAssignC(OEnv,"AP");
    else if((res->side_rel > 75) && (res->side_rel <= 90) && (SEnv == 'S'))
	ajStrAssignC(OEnv,"AQ");
    else if((res->side_rel > 75) && (res->side_rel <= 90) && (SEnv == 'C'))
	ajStrAssignC(OEnv,"AR");
    else if((res->side_rel > 90) && (SEnv == 'H'))
	ajStrAssignC(OEnv,"AS");  
    else if((res->side_rel > 90) && (SEnv == 'S'))
	ajStrAssignC(OEnv,"AT");
    else if((res->side_rel > 90) && (SEnv == 'C'))
	ajStrAssignC(OEnv,"AU");
    else
    {
	ajStrSetClear(OEnv); 
	ajFmtPrintF(flog, "OEnv unassigned for residue %d\n", res->Idx); 
	return 0;
    }

    /*The total number of environments*/
    return 21;
}




/* @func ajResidueEnv2 ********************************************************
**
** Assigns environment based on side chain accessibility and secondary 
** structure.  Assigns environment of "*" as default.
**
** @param [r] res [const AjPResidue] Residue object
** @param [r] SEnv [char]          Secondary structure environment code
** @param [w] OEnv [AjPStr*]       Character for the overall environment class 
** @param  [w] flog [AjPFile] Log file
** @return [ajint] Number of environments
** @@
******************************************************************************/
ajint ajResidueEnv2(const AjPResidue res, char SEnv, AjPStr *OEnv, AjPFile flog)
{
    float BLimit=40; /* Upper limit for the relative solvent
			accessible area for a buried residue */
    float PBLimit=114; /* Upper limit for the relative solvent
  			  accessible area for a Partially buried
  			  residue */
  
    float HLimit=45; /* Upper limit for the fraction polar measure of
			a Hydrophobic residue */
    float MPLimit=67; /* Upper limit for the fraction polar measure of
  			 a Moderately polar residue */
  
    AjPStr   BEnv=NULL;

    if(!res)
    {
	ajWarn("No residue to ajResidueEnv");

	return 0;
    }
  
    ajStrSetClear(OEnv);
    BEnv=ajStrNew();

    ajFmtPrintF(flog, "R:%c-%d S:%c A:%.2f f:%.2f\n",
		res->Id1, res->Idx, res->eType,
		res->side_rel, res->pol_rel);
  
    /*Assign the basic classes*/

    /*Buried, Hydrophobic*/
    if((res->side_rel <= BLimit) &&
       (res->pol_rel <= HLimit))
	ajStrAssignC(&BEnv, "B1");  
    /*buried, moderately polar*/
    else if((res->side_rel <= BLimit) &&
	    (res->pol_rel > HLimit) &&
	    (res->pol_rel <= MPLimit)) 
	ajStrAssignC(&BEnv, "B2");
    /*buried, polar*/
    else if((res->side_rel <= BLimit) &&
	    (res->pol_rel > MPLimit))
	ajStrAssignC(&BEnv, "B3");
    /*Partially buried, moderately Polar*/
    else if((res->side_rel > BLimit) &&
	    (res->side_rel <= PBLimit) &&
	    (res->pol_rel <= MPLimit))
	ajStrAssignC(&BEnv, "P1");
    /*Partially buried, Polar*/
    else if((res->side_rel > BLimit) &&
	    (res->side_rel <= PBLimit) &&
	    (res->pol_rel > MPLimit))
	ajStrAssignC(&BEnv, "P2");
    /*Exposed*/
    else if(res->side_rel > PBLimit)
	ajStrAssignC(&BEnv, "E");
    else
    {
	ajStrSetClear(OEnv); 
	ajFmtPrintF(flog, "BEnv unassigned for residue %d\n", res->Idx);
	ajStrDel(&BEnv);
	return 0;
    }
  
    /*Assign overall environment class*/
    if((ajStrMatchC(BEnv, "B1")) && (SEnv == 'H'))
	ajStrAssignC(OEnv,"AA");
    else if((ajStrMatchC(BEnv, "B1")) && (SEnv == 'S'))
	ajStrAssignC(OEnv,"AB");
    else if((ajStrMatchC(BEnv, "B1")) && (SEnv == 'C'))
	ajStrAssignC(OEnv,"AC");
    else if((ajStrMatchC(BEnv, "B2")) && (SEnv == 'H'))
	ajStrAssignC(OEnv,"AD");
    else if((ajStrMatchC(BEnv, "B2")) && (SEnv == 'S'))
	ajStrAssignC(OEnv,"AE");
    else if((ajStrMatchC(BEnv, "B2")) && (SEnv == 'C'))
	ajStrAssignC(OEnv,"AF");
    else if((ajStrMatchC(BEnv, "B3")) && (SEnv == 'H'))
	ajStrAssignC(OEnv,"AG");
    else if((ajStrMatchC(BEnv, "B3")) && (SEnv == 'S'))
	ajStrAssignC(OEnv,"AH");
    else if((ajStrMatchC(BEnv, "B3")) && (SEnv == 'C'))
	ajStrAssignC(OEnv,"AI");
    else if((ajStrMatchC(BEnv, "P1")) && (SEnv == 'H'))
	ajStrAssignC(OEnv,"AJ");
    else if((ajStrMatchC(BEnv, "P1")) && (SEnv == 'S'))
	ajStrAssignC(OEnv,"AK");
    else if((ajStrMatchC(BEnv, "P1")) && (SEnv == 'C'))
	ajStrAssignC(OEnv,"AL");
    else if((ajStrMatchC(BEnv, "P2")) && (SEnv == 'H'))
	ajStrAssignC(OEnv,"AM");
    else if((ajStrMatchC(BEnv, "P2")) && (SEnv == 'S'))
	ajStrAssignC(OEnv,"AN");
    else if((ajStrMatchC(BEnv, "P2")) && (SEnv == 'C'))
	ajStrAssignC(OEnv,"AO");
    else if((ajStrMatchC(BEnv, "E")) && (SEnv == 'H'))
	ajStrAssignC(OEnv,"AP");
    else if((ajStrMatchC(BEnv, "E")) && (SEnv == 'S'))
	ajStrAssignC(OEnv,"AQ");
    else if((ajStrMatchC(BEnv, "E")) && (SEnv == 'C'))
	ajStrAssignC(OEnv,"AR");
    else
    {
	ajStrSetClear(OEnv); 
	ajFmtPrintF(flog, "OEnv unassigned for residue %d\n", res->Idx);
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
** @param [r] res [const AjPResidue] Residue object
** @param [r] SEnv [char]          Secondary structure environment code
** @param [w] OEnv [AjPStr*]       Character for the overall environment class 
** @param  [w] flog [AjPFile] Log file
** @return [ajint] Number of environments
** @@
******************************************************************************/
ajint ajResidueEnv3(const AjPResidue res, char SEnv, AjPStr *OEnv, AjPFile flog)
{
    float BLimit=5; /* Upper limit for the relative solvent accessible
		       area for a buried residue */
    float PBLimit=25; /* Upper limit for the relative solvent
  			 accessible area for a Partially buried residue */
  
    float PolLimit1=10;
    float PolLimit2=30;
    float PolLimit3=50;
    float PolLimit4=80;

  
    AjPStr   BEnv=NULL;

    if(!res)
    {
	ajWarn("No res to ajResidueEnv");

	return 0;
    }
  
    ajStrSetClear(OEnv);
    BEnv=ajStrNew();

    ajFmtPrintF(flog, "R:%c-%d S:%c A:%.2f f:%.2f\n",
		res->Id1, res->Idx, res->eType,
		res->side_rel, res->pol_rel);
  
    /*Assign the basic classes*/

    /*Buried, Hydrophobic*/
    if((res->side_rel <= BLimit) &&
       (res->pol_rel <= PolLimit1))
	ajStrAssignC(&BEnv, "B1");  
    /*buried, moderately polar*/
    else if((res->side_rel <= BLimit) &&
	    (res->pol_rel > PolLimit1) &&
	    (res->pol_rel <= PolLimit2)) 
	ajStrAssignC(&BEnv, "B2");
    else if((res->side_rel <= BLimit) &&
	    (res->pol_rel > PolLimit2) &&
	    (res->pol_rel <= PolLimit3)) 
	ajStrAssignC(&BEnv, "B3");
    else if((res->side_rel <= BLimit) &&
	    (res->pol_rel > PolLimit3) &&
	    (res->pol_rel <= PolLimit4)) 
	ajStrAssignC(&BEnv, "B4");
    else if((res->side_rel <= BLimit) &&
	    (res->pol_rel > PolLimit4)) 
	ajStrAssignC(&BEnv, "B5");
    /*Partially buried, moderately Polar*/
    else if((res->side_rel > BLimit) &&
	    (res->side_rel <= PBLimit) &&
	    (res->pol_rel <= PolLimit3))
	ajStrAssignC(&BEnv, "P1");
    /*Partially buried, Polar*/
    else if((res->side_rel > BLimit) &&
	    (res->side_rel <= PBLimit) &&
	    (res->pol_rel > PolLimit3))
	ajStrAssignC(&BEnv, "P2");
    /*Exposed*/
    else if(res->side_rel > PBLimit)
	ajStrAssignC(&BEnv, "E");
    else
    {
	ajStrSetClear(OEnv); 
	ajFmtPrintF(flog, "BEnv unassigned for residue %d\n", res->Idx);

	ajStrDel(&BEnv);
	return 0;
    }
  
    /*Assign overall environment class*/
    if((ajStrMatchC(BEnv, "B1")) && (SEnv == 'H'))
	ajStrAssignC(OEnv,"AA");
    else if((ajStrMatchC(BEnv, "B1")) && (SEnv == 'S'))
	ajStrAssignC(OEnv,"AB");
    else if((ajStrMatchC(BEnv, "B1")) && (SEnv == 'C'))
	ajStrAssignC(OEnv,"AC");
    else if((ajStrMatchC(BEnv, "B2")) && (SEnv == 'H'))
	ajStrAssignC(OEnv,"AD");
    else if((ajStrMatchC(BEnv, "B2")) && (SEnv == 'S'))
	ajStrAssignC(OEnv,"AE");
    else if((ajStrMatchC(BEnv, "B2")) && (SEnv == 'C'))
	ajStrAssignC(OEnv,"AF");
    else if((ajStrMatchC(BEnv, "B3")) && (SEnv == 'H'))
	ajStrAssignC(OEnv,"AG");
    else if((ajStrMatchC(BEnv, "B3")) && (SEnv == 'S'))
	ajStrAssignC(OEnv,"AH");
    else if((ajStrMatchC(BEnv, "B3")) && (SEnv == 'C'))
	ajStrAssignC(OEnv,"AI");
    else if((ajStrMatchC(BEnv, "B4")) && (SEnv == 'H'))
	ajStrAssignC(OEnv,"AJ");
    else if((ajStrMatchC(BEnv, "B4")) && (SEnv == 'S'))
	ajStrAssignC(OEnv,"AK");
    else if((ajStrMatchC(BEnv, "B4")) && (SEnv == 'C'))
	ajStrAssignC(OEnv,"AL"); 
    else if((ajStrMatchC(BEnv, "B5")) && (SEnv == 'H'))
	ajStrAssignC(OEnv,"AM");
    else if((ajStrMatchC(BEnv, "B5")) && (SEnv == 'S'))
	ajStrAssignC(OEnv,"AN");
    else if((ajStrMatchC(BEnv, "B5")) && (SEnv == 'C'))
	ajStrAssignC(OEnv,"AO");
    else if((ajStrMatchC(BEnv, "P1")) && (SEnv == 'H'))
	ajStrAssignC(OEnv,"AP");
    else if((ajStrMatchC(BEnv, "P1")) && (SEnv == 'S'))
	ajStrAssignC(OEnv,"AQ");
    else if((ajStrMatchC(BEnv, "P1")) && (SEnv == 'C'))
	ajStrAssignC(OEnv,"AR");
    else if((ajStrMatchC(BEnv, "P2")) && (SEnv == 'H'))
	ajStrAssignC(OEnv,"AS");
    else if((ajStrMatchC(BEnv, "P2")) && (SEnv == 'S'))
	ajStrAssignC(OEnv,"AT");
    else if((ajStrMatchC(BEnv, "P2")) && (SEnv == 'C'))
	ajStrAssignC(OEnv,"AU");
    else if((ajStrMatchC(BEnv, "E")) && (SEnv == 'H'))
	ajStrAssignC(OEnv,"AV");
    else if((ajStrMatchC(BEnv, "E")) && (SEnv == 'S'))
	ajStrAssignC(OEnv,"AW");
    else if((ajStrMatchC(BEnv, "E")) && (SEnv == 'C'))
	ajStrAssignC(OEnv,"AX");
    else
    {
	ajStrSetClear(OEnv); 
	ajFmtPrintF(flog, "OEnv unassigned for residue %d\n", res->Idx);

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
** @param [r] res [const AjPResidue] Residue object
** @param [r] SEnv [char]          Secondary structure environment code
** @param [w] OEnv [AjPStr*]       Character for the overall environment class 
** @param  [w] flog [AjPFile] Log file
** @return [ajint] Number of environments
** @@
******************************************************************************/

ajint ajResidueEnv4(const AjPResidue res, char SEnv, AjPStr *OEnv, AjPFile flog)
{
    ajStrSetClear(OEnv);

    if(SEnv=='\0')
    {
	ajStrSetClear(OEnv);

	return 0;
    }
  
    if((res->side_rel <= 5) && (SEnv == 'H'))
	ajStrAssignC(OEnv,"AA");
    else if((res->side_rel <= 5) && (SEnv == 'S'))
	ajStrAssignC(OEnv,"AB");
    else if((res->side_rel <= 5) && (SEnv == 'C'))
	ajStrAssignC(OEnv,"AC");
    else if((res->side_rel > 5) && (res->side_rel <= 25) && (SEnv == 'H'))
	ajStrAssignC(OEnv,"AD");
    else if((res->side_rel > 5) && (res->side_rel <= 25) && (SEnv == 'S'))
	ajStrAssignC(OEnv,"AE");
    else if((res->side_rel > 5) && (res->side_rel <= 25) && (SEnv == 'C'))
	ajStrAssignC(OEnv,"AF");
    else if((res->side_rel > 25) && (SEnv == 'H'))
	ajStrAssignC(OEnv,"AG");
    else if((res->side_rel > 25) && (SEnv == 'S'))
	ajStrAssignC(OEnv,"AH");
    else if((res->side_rel > 25) && (SEnv == 'C'))
	ajStrAssignC(OEnv,"AI");
    else
    {
	ajStrSetClear(OEnv);
	ajFmtPrintF(flog, "OEnv unassigned for residue %d\n", res->Idx); 

	return 0;
    }

    return 9;

}




/* @func ajResidueEnv5 ********************************************************
** Assigns environment based on side chain accessibility and secondary 
** structure.  Assigns environment of "*" as default.
** @param [r] res [const AjPResidue] Residue object
** @param [r] SEnv [char]          Secondary structure environment code
** @param [w] OEnv [AjPStr*]       Character for the overall environment class 
** @param  [w] flog [AjPFile] Log file
** @return [ajint] Number of environments
** @@
******************************************************************************/

ajint ajResidueEnv5(const AjPResidue res, char SEnv, AjPStr *OEnv, AjPFile flog)
{
    float BLimit=5; /* Upper limit for the relative solvent accessible
		       area for a buried residue */ /* */
    float PBLimit=25; /* Upper limit for the relative solvent
  			 accessible area for a Partially buried residue */
  
    float PolLimit1=10;
    float PolLimit2=30;
    float PolLimit3=50;
    float PolLimit4=80;

  
    AjPStr   BEnv=NULL;

    if(!res)
    {
	ajWarn("No residue to ajResidueEnv");

	return 0;
    }
  
    ajStrSetClear(OEnv);
    BEnv=ajStrNew();

    ajFmtPrintF(flog, "R:%c-%d S:%c A:%.2f f:%.2f\n",
		res->Id1, res->Idx, res->eType,
		res->side_rel, res->pol_rel);
  
    /*Assign the basic classes*/

    /*Buried*/
    if(res->side_rel <= BLimit)
	ajStrAssignC(&BEnv, "B");  
    /*partially buried, hydrophobic*/
    else if((res->side_rel > BLimit) &&
	    (res->side_rel <= PBLimit) &&
	    (res->pol_rel <= PolLimit1))
	ajStrAssignC(&BEnv, "P1");  
    /*partially buried, moderately polar*/
    else if((res->side_rel > BLimit) &&
	    (res->side_rel <= PBLimit) &&
	    (res->pol_rel > PolLimit1) &&
	    (res->pol_rel <= PolLimit2)) 
	ajStrAssignC(&BEnv, "P2");
    else if((res->side_rel > BLimit) &&
	    (res->side_rel <= PBLimit) &&
	    (res->pol_rel > PolLimit2) &&
	    (res->pol_rel <= PolLimit3)) 
	ajStrAssignC(&BEnv, "P3");
    else if((res->side_rel > BLimit) &&
	    (res->side_rel <= PBLimit) &&
	    (res->pol_rel > PolLimit3) &&
	    (res->pol_rel <= PolLimit4)) 
	ajStrAssignC(&BEnv, "P4");
    /*partially buried, polar*/
    else if((res->side_rel > BLimit) &&
	    (res->side_rel <= PBLimit) &&
	    (res->pol_rel > PolLimit4)) 
	ajStrAssignC(&BEnv, "P5");
    /*Exposed, moderately Polar*/
    else if((res->side_rel > PBLimit) &&
	    (res->pol_rel <= PolLimit3))
	ajStrAssignC(&BEnv, "E1");
    /*Exposed, Polar*/
    else if((res->side_rel > PBLimit) &&
	    (res->pol_rel > PolLimit3))
	ajStrAssignC(&BEnv, "E2");
    else
    {
	ajStrSetClear(OEnv); 
	ajFmtPrintF(flog, "BEnv unassigned for residue %d\n", res->Idx);

	ajStrDel(&BEnv);

	return 0;
    }
  
    /*Assign overall environment class*/
    if((ajStrMatchC(BEnv, "B")) && (SEnv == 'H'))
	ajStrAssignC(OEnv,"AA");
    else if((ajStrMatchC(BEnv, "B")) && (SEnv == 'S'))
	ajStrAssignC(OEnv,"AB");
    else if((ajStrMatchC(BEnv, "B")) && (SEnv == 'C'))
	ajStrAssignC(OEnv,"AC");
    else if((ajStrMatchC(BEnv, "P1")) && (SEnv == 'H'))
	ajStrAssignC(OEnv,"AD");
    else if((ajStrMatchC(BEnv, "P1")) && (SEnv == 'S'))
	ajStrAssignC(OEnv,"AE");
    else if((ajStrMatchC(BEnv, "P1")) && (SEnv == 'C'))
	ajStrAssignC(OEnv,"AF");
    else if((ajStrMatchC(BEnv, "P2")) && (SEnv == 'H'))
	ajStrAssignC(OEnv,"AG");
    else if((ajStrMatchC(BEnv, "P2")) && (SEnv == 'S'))
	ajStrAssignC(OEnv,"AH");
    else if((ajStrMatchC(BEnv, "P2")) && (SEnv == 'C'))
	ajStrAssignC(OEnv,"AI");
    else if((ajStrMatchC(BEnv, "P3")) && (SEnv == 'H'))
	ajStrAssignC(OEnv,"AJ");
    else if((ajStrMatchC(BEnv, "P3")) && (SEnv == 'S'))
	ajStrAssignC(OEnv,"AK");
    else if((ajStrMatchC(BEnv, "P3")) && (SEnv == 'C'))
	ajStrAssignC(OEnv,"AL"); 
    else if((ajStrMatchC(BEnv, "P4")) && (SEnv == 'H'))
	ajStrAssignC(OEnv,"AM");
    else if((ajStrMatchC(BEnv, "P4")) && (SEnv == 'S'))
	ajStrAssignC(OEnv,"AN");
    else if((ajStrMatchC(BEnv, "P4")) && (SEnv == 'C'))
	ajStrAssignC(OEnv,"AO");
    else if((ajStrMatchC(BEnv, "P5")) && (SEnv == 'H'))
	ajStrAssignC(OEnv,"AP");
    else if((ajStrMatchC(BEnv, "P5")) && (SEnv == 'S'))
	ajStrAssignC(OEnv,"AQ");
    else if((ajStrMatchC(BEnv, "P5")) && (SEnv == 'C'))
	ajStrAssignC(OEnv,"AR");
    else if((ajStrMatchC(BEnv, "E1")) && (SEnv == 'H'))
	ajStrAssignC(OEnv,"AS");
    else if((ajStrMatchC(BEnv, "E1")) && (SEnv == 'S'))
	ajStrAssignC(OEnv,"AT");
    else if((ajStrMatchC(BEnv, "E1")) && (SEnv == 'C'))
	ajStrAssignC(OEnv,"AU");
    else if((ajStrMatchC(BEnv, "E2")) && (SEnv == 'H'))
	ajStrAssignC(OEnv,"AV");
    else if((ajStrMatchC(BEnv, "E2")) && (SEnv == 'S'))
	ajStrAssignC(OEnv,"AW");
    else if((ajStrMatchC(BEnv, "E2")) && (SEnv == 'C'))
	ajStrAssignC(OEnv,"AX");
    else
    {
	ajStrSetClear(OEnv); 
	ajFmtPrintF(flog, "OEnv unassigned for residue %d\n", res->Idx);

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
** @param [r] res [const AjPResidue] Residue object
** @param [r] SEnv [char]          Secondary structure environment code
** @param [w] OEnv [AjPStr*]       Character for the overall environment class 
** @param  [w] flog [AjPFile] Log file
** @return [ajint] Number of environments
** @@
******************************************************************************/

ajint ajResidueEnv6(const AjPResidue res, char SEnv, AjPStr *OEnv, AjPFile flog)
{
    float BLimit=5; /* Upper limit for the relative solvent accessible
		       area for a buried residue */
    float PBLimit=25; /* Upper limit for the relative solvent
  			 accessible area for a Partially buried residue */
  
    float PolLimit1=10;
    float PolLimit2=30;
    float PolLimit3=50;
    float PolLimit4=80;

  
    AjPStr   BEnv=NULL;

    if(!res)
    {
	ajWarn("No residue to ajResidueEnv");

	return 0;
    }
  
    ajStrSetClear(OEnv);
    BEnv=ajStrNew();

    ajFmtPrintF(flog, "R:%c-%d S:%c A:%.2f f:%.2f\n",
		res->Id1, res->Idx, res->eType,
		res->side_rel, res->pol_rel);
  
    /*Assign the basic classes*/

    /*Buried*/
    if(res->side_rel <= BLimit)
	ajStrAssignC(&BEnv, "B");  
    /*Partially buried, hydrophobic*/
    else if((res->side_rel > BLimit) &&
	    (res->side_rel <= PBLimit) &&
	    (res->pol_rel <= PolLimit3))
	ajStrAssignC(&BEnv, "P1");
    /*Partially buried, Polar*/
    else if((res->side_rel > BLimit) &&
	    (res->side_rel <= PBLimit) &&
	    (res->pol_rel > PolLimit3))
	ajStrAssignC(&BEnv, "P2");
    /*partially buried, hydrophobic*/
    else if((res->side_rel > PBLimit) &&
	    (res->pol_rel <= PolLimit1))
	ajStrAssignC(&BEnv, "E1");  
    /*partially buried, moderately polar*/
    else if((res->side_rel > PBLimit) &&
	    (res->pol_rel > PolLimit1) &&
	    (res->pol_rel <= PolLimit2)) 
	ajStrAssignC(&BEnv, "E2");
    else if((res->side_rel > PBLimit) &&
	    (res->pol_rel > PolLimit2) &&
	    (res->pol_rel <= PolLimit3)) 
	ajStrAssignC(&BEnv, "E3");
    else if((res->side_rel > PBLimit) &&
	    (res->pol_rel > PolLimit3) &&
	    (res->pol_rel <= PolLimit4)) 
	ajStrAssignC(&BEnv, "E4");
    /*partially buried, polar*/
    else if((res->side_rel > PBLimit) &&
	    (res->pol_rel > PolLimit4)) 
	ajStrAssignC(&BEnv, "E5");
    else
    {
	ajStrSetClear(OEnv); 
	ajFmtPrintF(flog, "BEnv unassigned for residue %d\n", res->Idx);
	ajStrDel(&BEnv);

	return 0;
    }
  
    /*Assign overall environment class*/
    if((ajStrMatchC(BEnv, "B")) && (SEnv == 'H'))
	ajStrAssignC(OEnv,"AA");
    else if((ajStrMatchC(BEnv, "B")) && (SEnv == 'S'))
	ajStrAssignC(OEnv,"AB");
    else if((ajStrMatchC(BEnv, "B")) && (SEnv == 'C'))
	ajStrAssignC(OEnv,"AC");
    else if((ajStrMatchC(BEnv, "P1")) && (SEnv == 'H'))
	ajStrAssignC(OEnv,"AD");
    else if((ajStrMatchC(BEnv, "P1")) && (SEnv == 'S'))
	ajStrAssignC(OEnv,"AE");
    else if((ajStrMatchC(BEnv, "P1")) && (SEnv == 'C'))
	ajStrAssignC(OEnv,"AF");
    else if((ajStrMatchC(BEnv, "P2")) && (SEnv == 'H'))
	ajStrAssignC(OEnv,"AG");
    else if((ajStrMatchC(BEnv, "P2")) && (SEnv == 'S'))
	ajStrAssignC(OEnv,"AH");
    else if((ajStrMatchC(BEnv, "P2")) && (SEnv == 'C'))
	ajStrAssignC(OEnv,"AI");
    else if((ajStrMatchC(BEnv, "E1")) && (SEnv == 'H'))
	ajStrAssignC(OEnv,"AJ");
    else if((ajStrMatchC(BEnv, "E1")) && (SEnv == 'S'))
	ajStrAssignC(OEnv,"AK");
    else if((ajStrMatchC(BEnv, "E1")) && (SEnv == 'C'))
	ajStrAssignC(OEnv,"AL"); 
    else if((ajStrMatchC(BEnv, "E2")) && (SEnv == 'H'))
	ajStrAssignC(OEnv,"AM");
    else if((ajStrMatchC(BEnv, "E2")) && (SEnv == 'S'))
	ajStrAssignC(OEnv,"AN");
    else if((ajStrMatchC(BEnv, "E2")) && (SEnv == 'C'))
	ajStrAssignC(OEnv,"AO");
    else if((ajStrMatchC(BEnv, "E3")) && (SEnv == 'H'))
	ajStrAssignC(OEnv,"AP");
    else if((ajStrMatchC(BEnv, "E3")) && (SEnv == 'S'))
	ajStrAssignC(OEnv,"AQ");
    else if((ajStrMatchC(BEnv, "E3")) && (SEnv == 'C'))
	ajStrAssignC(OEnv,"AR");
    else if((ajStrMatchC(BEnv, "E4")) && (SEnv == 'H'))
	ajStrAssignC(OEnv,"AS");
    else if((ajStrMatchC(BEnv, "E4")) && (SEnv == 'S'))
	ajStrAssignC(OEnv,"AT");
    else if((ajStrMatchC(BEnv, "E4")) && (SEnv == 'C'))
	ajStrAssignC(OEnv,"AU");
    else if((ajStrMatchC(BEnv, "E5")) && (SEnv == 'H'))
	ajStrAssignC(OEnv,"AV");
    else if((ajStrMatchC(BEnv, "E5")) && (SEnv == 'S'))
	ajStrAssignC(OEnv,"AW");
    else if((ajStrMatchC(BEnv, "E5")) && (SEnv == 'C'))
	ajStrAssignC(OEnv,"AX");
    else
    {
	ajStrSetClear(OEnv); 
	ajFmtPrintF(flog, "OEnv unassigned for residue %d\n", res->Idx);
	ajStrDel(&BEnv);

	return 0;
    }

    ajStrDel(&BEnv);

    return 24;
}




/* @func ajResidueEnv7 *******************************************************
**
** Assigns environment based on side chain accessibility and secondary 
** structure.  Assigns environment of "*" as default.
**
** @param [r] res [const AjPResidue] Residue object
** @param [r] SEnv [char]          Secondary structure environment code
** @param [w] OEnv [AjPStr*]       Character for the overall environment class 
** @param  [w] flog [AjPFile] Log file
** @return [ajint] Number of environments
** @@
******************************************************************************/

ajint ajResidueEnv7(const AjPResidue res, char SEnv, AjPStr *OEnv, AjPFile flog)
{
    float BLimit=5; /* Upper limit for the relative solvent accessible
		       area for a buried residue */
    float PBLimit=25; /* Upper limit for the relative solvent
  			 accessible area for a Partially buried residue */
  
    float PolLimit1=10;
    float PolLimit2=50;
    float PolLimit3=90;
  
    AjPStr   BEnv=NULL;

    if(!res)
    {
	ajWarn("No residue to ajResidueEnv");
	ajStrDel(&BEnv);

	return 0;
    }
  
    ajStrSetClear(OEnv);
    BEnv=ajStrNew();

    ajFmtPrintF(flog, "R:%c-%d S:%c A:%.2f f:%.2f\n",
		res->Id1, res->Idx, res->eType,
		res->side_rel, res->pol_rel);
  
    /*Assign the basic classes*/

    /*Buried*/
    if((res->side_rel <= BLimit) &&
       (res->pol_rel <=PolLimit1))
	ajStrAssignC(&BEnv, "B1");  
    else if((res->side_rel <= BLimit) &&
	    (res->pol_rel > PolLimit1) &&
	    (res->pol_rel <= PolLimit3))
	ajStrAssignC(&BEnv, "B2"); 
    else if((res->side_rel <= BLimit) &&
	    (res->pol_rel > PolLimit3))
	ajStrAssignC(&BEnv, "B3");
    else if((res->side_rel > BLimit) &&
	    (res->side_rel <= PBLimit))
	ajStrAssignC(&BEnv, "P");
    else if((res->side_rel > PBLimit) &&
	    (res->pol_rel <= PolLimit1))
	ajStrAssignC(&BEnv, "E1");
    else if((res->side_rel > PBLimit) &&
	    (res->pol_rel > PolLimit1) &&
	    (res->pol_rel <= PolLimit2))
	ajStrAssignC(&BEnv, "E2");
    else if((res->side_rel > PBLimit) &&
	    (res->pol_rel > PolLimit2) &&
	    (res->pol_rel <= PolLimit3))
	ajStrAssignC(&BEnv, "E3");
    else if((res->side_rel > PBLimit) && (res->pol_rel > PolLimit3))
	ajStrAssignC(&BEnv, "E4");
    else
    {
	ajStrSetClear(OEnv); 
	ajFmtPrintF(flog, "BEnv unassigned for residue %d\n", res->Idx);

	ajStrDel(&BEnv);

	return 0;
    }
  
    /*Assign overall environment class*/
    if((ajStrMatchC(BEnv, "B1")) && (SEnv == 'H'))
	ajStrAssignC(OEnv,"AA");
    else if((ajStrMatchC(BEnv, "B1")) && (SEnv == 'S'))
	ajStrAssignC(OEnv,"AB");
    else if((ajStrMatchC(BEnv, "B1")) && (SEnv == 'C'))
	ajStrAssignC(OEnv,"AC");
    else if((ajStrMatchC(BEnv, "B2")) && (SEnv == 'H'))
	ajStrAssignC(OEnv,"AD");
    else if((ajStrMatchC(BEnv, "B2")) && (SEnv == 'S'))
	ajStrAssignC(OEnv,"AE");
    else if((ajStrMatchC(BEnv, "B2")) && (SEnv == 'C'))
	ajStrAssignC(OEnv,"AF");
    else if((ajStrMatchC(BEnv, "B3")) && (SEnv == 'H'))
	ajStrAssignC(OEnv,"AG");
    else if((ajStrMatchC(BEnv, "B3")) && (SEnv == 'S'))
	ajStrAssignC(OEnv,"AH");
    else if((ajStrMatchC(BEnv, "B3")) && (SEnv == 'C'))
	ajStrAssignC(OEnv,"AI");
    else if((ajStrMatchC(BEnv, "P")) && (SEnv == 'H'))
	ajStrAssignC(OEnv,"AJ");
    else if((ajStrMatchC(BEnv, "P")) && (SEnv == 'S'))
	ajStrAssignC(OEnv,"AK");
    else if((ajStrMatchC(BEnv, "P")) && (SEnv == 'C'))
	ajStrAssignC(OEnv,"AL"); 
    else if((ajStrMatchC(BEnv, "E1")) && (SEnv == 'H'))
	ajStrAssignC(OEnv,"AM");
    else if((ajStrMatchC(BEnv, "E1")) && (SEnv == 'S'))
	ajStrAssignC(OEnv,"AN");
    else if((ajStrMatchC(BEnv, "E1")) && (SEnv == 'C'))
	ajStrAssignC(OEnv,"AO");
    else if((ajStrMatchC(BEnv, "E2")) && (SEnv == 'H'))
	ajStrAssignC(OEnv,"AP");
    else if((ajStrMatchC(BEnv, "E2")) && (SEnv == 'S'))
	ajStrAssignC(OEnv,"AQ");
    else if((ajStrMatchC(BEnv, "E2")) && (SEnv == 'C'))
	ajStrAssignC(OEnv,"AR");
    else if((ajStrMatchC(BEnv, "E3")) && (SEnv == 'H'))
	ajStrAssignC(OEnv,"AS");
    else if((ajStrMatchC(BEnv, "E3")) && (SEnv == 'S'))
	ajStrAssignC(OEnv,"AT");
    else if((ajStrMatchC(BEnv, "E3")) && (SEnv == 'C'))
	ajStrAssignC(OEnv,"AU");
    else if((ajStrMatchC(BEnv, "E4")) && (SEnv == 'H'))
	ajStrAssignC(OEnv,"AV");
    else if((ajStrMatchC(BEnv, "E4")) && (SEnv == 'S'))
	ajStrAssignC(OEnv,"AW");
    else if((ajStrMatchC(BEnv, "E4")) && (SEnv == 'C'))
	ajStrAssignC(OEnv,"AX");
    else
    {
	ajStrSetClear(OEnv); 
	ajFmtPrintF(flog, "OEnv unassigned for residue %d\n", res->Idx);
	ajStrDel(&BEnv);

	return 0;
    }

    ajStrDel(&BEnv);

    return 24;

}




/* @func ajResidueEnv8 *******************************************************
**
** Assigns environment based only of side chain accessibility and secondary 
** structure.  Assigns environment of "*" as default.
**
** @param [r] res [const AjPResidue] Residue object
** @param [r] SEnv [char]          Secondary structure environment code
** @param [w] OEnv [AjPStr*]       Character for the overall environment class 
** @param  [w] flog [AjPFile] Log file
** @return [ajint] Number of environments
** @@
******************************************************************************/
ajint ajResidueEnv8(const AjPResidue res, char SEnv, AjPStr *OEnv, AjPFile flog)
{
    ajStrSetClear(OEnv);

    if(SEnv=='\0')
    {
	ajStrSetClear(OEnv);

	return 0;
    }
  
    if((res->pol_rel <= 15) && (SEnv == 'H'))
	ajStrAssignC(OEnv,"AA");
    else if((res->pol_rel <= 15) && (SEnv == 'S'))
	ajStrAssignC(OEnv,"AB");
    else if((res->pol_rel <= 15) && (SEnv == 'C'))
	ajStrAssignC(OEnv,"AC");
    else if((res->pol_rel > 15) && (res->pol_rel <= 30) && (SEnv == 'H'))
	ajStrAssignC(OEnv,"AD");
    else if((res->pol_rel > 15) && (res->pol_rel <= 30) && (SEnv == 'S'))
	ajStrAssignC(OEnv,"AE");
    else if((res->pol_rel > 15) && (res->pol_rel <= 30) && (SEnv == 'C'))
	ajStrAssignC(OEnv,"AF");
    else if((res->pol_rel > 30) && (res->pol_rel <= 45) && (SEnv == 'H'))
	ajStrAssignC(OEnv,"AG");
    else if((res->pol_rel > 30) && (res->pol_rel <= 45) && (SEnv == 'S'))
	ajStrAssignC(OEnv,"AH");
    else if((res->pol_rel > 30) && (res->pol_rel <= 45) && (SEnv == 'C'))
	ajStrAssignC(OEnv,"AI");
    else if((res->pol_rel > 45) && (res->pol_rel <= 60) && (SEnv == 'H'))
	ajStrAssignC(OEnv,"AJ");
    else if((res->pol_rel > 45) && (res->pol_rel <= 60) && (SEnv == 'S'))
	ajStrAssignC(OEnv,"AK");
    else if((res->pol_rel > 45) && (res->pol_rel <= 60) && (SEnv == 'C'))
	ajStrAssignC(OEnv,"AL");
    else if((res->pol_rel > 60) && (res->pol_rel <= 75) && (SEnv == 'H'))
	ajStrAssignC(OEnv,"AM");
    else if((res->pol_rel > 60) && (res->pol_rel <= 75) && (SEnv == 'S'))
	ajStrAssignC(OEnv,"AN");  
    else if((res->pol_rel > 60) && (res->pol_rel <= 75) && (SEnv == 'C'))
	ajStrAssignC(OEnv,"AO"); 
    else if((res->pol_rel > 75) && (res->pol_rel <= 90) && (SEnv == 'H'))
	ajStrAssignC(OEnv,"AP");
    else if((res->pol_rel > 75) && (res->pol_rel <= 90) && (SEnv == 'S'))
	ajStrAssignC(OEnv,"AQ");
    else if((res->pol_rel > 75) && (res->pol_rel <= 90) && (SEnv == 'C'))
	ajStrAssignC(OEnv,"AR");
    else if((res->pol_rel > 90) && (SEnv == 'H'))
	ajStrAssignC(OEnv,"AS");  
    else if((res->pol_rel > 90)&& (SEnv == 'S'))
	ajStrAssignC(OEnv,"AT");
    else if((res->pol_rel > 90)&& (SEnv == 'C'))
	ajStrAssignC(OEnv,"AU");
    else
    {
	ajStrSetClear(OEnv); 
	ajFmtPrintF(flog, "OEnv unassigned for residue %d\n", res->Idx); 

	return 0;
    }

    return 21;
}




/* @func ajResidueEnv9 ********************************************************
**
** Assigns environment based only of side chain accessibility and secondary 
** structure.  Assigns environment of "*" as default.
**
** @param [r] res [const AjPResidue] Residue object
** @param [r] SEnv [char]          Secondary structure environment code
** @param [w] OEnv [AjPStr*]       Character for the overall environment class 
** @param  [w] flog [AjPFile] Log file
** @return [ajint] Number of environments
** @@
******************************************************************************/

ajint ajResidueEnv9(const AjPResidue res, char SEnv, AjPStr *OEnv, AjPFile flog)
{
    ajStrSetClear(OEnv);

    if(SEnv=='\0')
    {
	ajStrSetClear(OEnv);

	return 0;
    }
  
    if((res->pol_rel <= 5) && (SEnv == 'H'))
	ajStrAssignC(OEnv,"AA");
    else if((res->pol_rel <= 5) && (SEnv == 'S'))
	ajStrAssignC(OEnv,"AB");
    else if((res->pol_rel <= 5) && (SEnv == 'C'))
	ajStrAssignC(OEnv,"AC");
    else if((res->pol_rel > 5) && (res->pol_rel <= 25) && (SEnv == 'H'))
	ajStrAssignC(OEnv,"AD");
    else if((res->pol_rel > 5) && (res->pol_rel <= 25) && (SEnv == 'S'))
	ajStrAssignC(OEnv,"AE");
    else if((res->pol_rel > 5) && (res->pol_rel <= 25) && (SEnv == 'C'))
	ajStrAssignC(OEnv,"AF");
    else if((res->pol_rel > 25) && (SEnv == 'H'))
	ajStrAssignC(OEnv,"AG");
    else if((res->pol_rel > 25) && (SEnv == 'S'))
	ajStrAssignC(OEnv,"AH");
    else if((res->pol_rel > 25) && (SEnv == 'C'))
	ajStrAssignC(OEnv,"AI");
    else
    {
	ajStrSetClear(OEnv);
	ajFmtPrintF(flog, "OEnv unassigned for residue %d\n", res->Idx); 
	
	return 0;
    }

    return 9;
}




/* @func ajResidueEnv10 *******************************************************
**
** Assigns environment based on side chain accessibility and secondary 
** structure.  Assigns environment of "*" as default.
**
** @param [r] res [const AjPResidue] Residue object
** @param [r] SEnv [char]          Secondary structure environment code
** @param [w] OEnv [AjPStr*]       Character for the overall environment class 
** @param  [w] flog [AjPFile] Log file
** @return [ajint] Number of environments
** @@
******************************************************************************/

ajint ajResidueEnv10(const AjPResidue res, char SEnv, AjPStr *OEnv,
                     AjPFile flog)
{
    float BLimit=5; /* Upper limit for the relative solvent accessible
		       area for a buried residue */
    float PBLimit=25; /* Upper limit for the relative solvent
  			 accessible area for a Partially buried residue */
  
    float HLimit=5; /* Upper limit for the fraction polar measure of a
		       Hydrophobic residue */
    float MPLimit=25; /* Upper limit for the fraction polar measure of
  			 a Moderately polar residue */
  
    AjPStr   BEnv=NULL;

    if(!res)
    {
	ajWarn("No residue to ajResidueEnv");

	return 0;
    }
  
    ajStrSetClear(OEnv);
    BEnv=ajStrNew();

    ajFmtPrintF(flog, "R:%c-%d S:%c A:%.2f f:%.2f\n",
		res->Id1, res->Idx, res->eType,
		res->side_rel, res->pol_rel);
  
    /*Assign the basic classes*/
  
    /*Buried, Hydrophobic*/
    if((res->side_rel <= BLimit) &&
       (res->pol_rel <= HLimit))
	ajStrAssignC(&BEnv, "B1");  
    /*buried, moderately polar*/
    else if((res->side_rel <= BLimit) &&
	    (res->pol_rel > HLimit) &&
	    (res->pol_rel <= MPLimit)) 
	ajStrAssignC(&BEnv, "B2");
    /*buried, polar*/
    else if((res->side_rel <= BLimit) &&
	    (res->pol_rel > MPLimit))
	ajStrAssignC(&BEnv, "B3");
    /*Partially buried, moderately hydrophobic*/
    else if((res->side_rel > BLimit) &&
	    (res->side_rel <= PBLimit) &&
	    (res->pol_rel <= HLimit))
	ajStrAssignC(&BEnv, "P1");
    /*Partially buried, moderately polar*/
    else if((res->side_rel > BLimit) &&
	    (res->side_rel <= PBLimit) &&
	    (res->pol_rel > HLimit) &&
	    (res->pol_rel <= MPLimit))
	ajStrAssignC(&BEnv, "P2");
    /*Partially buried, polar*/
    else if((res->side_rel > BLimit) &&
	    (res->side_rel <= PBLimit) && (res->pol_rel > MPLimit))
	ajStrAssignC(&BEnv, "P3");
    /*Exposed, moderately polar*/
    else if((res->side_rel > PBLimit) &&
	    (res->pol_rel <= MPLimit))
	ajStrAssignC(&BEnv, "E1");
    /*Exposed, polar*/
    else if((res->side_rel > PBLimit) &&
	    (res->pol_rel > MPLimit))
	ajStrAssignC(&BEnv, "E2");
    else
    {
	 ajStrSetClear(OEnv);
	ajFmtPrintF(flog, "BEnv unassigned for residue %d\n", res->Idx);
	
	ajStrDel(&BEnv);

	return 0;
    }
  
    /*Assign overall environment class*/
    if((ajStrMatchC(BEnv, "B1")) && (SEnv == 'H'))
	ajStrAssignC(OEnv,"AA");
    else if((ajStrMatchC(BEnv, "B1")) && (SEnv == 'S'))
	ajStrAssignC(OEnv,"AB");
    else if((ajStrMatchC(BEnv, "B1")) && (SEnv == 'C'))
	ajStrAssignC(OEnv,"AC");
    else if((ajStrMatchC(BEnv, "B2")) && (SEnv == 'H'))
	ajStrAssignC(OEnv,"AD");
    else if((ajStrMatchC(BEnv, "B2")) && (SEnv == 'S'))
	ajStrAssignC(OEnv,"AE");
    else if((ajStrMatchC(BEnv, "B2")) && (SEnv == 'C'))
	ajStrAssignC(OEnv,"AF");
    else if((ajStrMatchC(BEnv, "B3")) && (SEnv == 'H'))
	ajStrAssignC(OEnv,"AG");
    else if((ajStrMatchC(BEnv, "B3")) && (SEnv == 'S'))
	ajStrAssignC(OEnv,"AH");
    else if((ajStrMatchC(BEnv, "B3")) && (SEnv == 'C'))
	ajStrAssignC(OEnv,"AI");
    else if((ajStrMatchC(BEnv, "P1")) && (SEnv == 'H'))
	ajStrAssignC(OEnv,"AJ");
    else if((ajStrMatchC(BEnv, "P1")) && (SEnv == 'S'))
	ajStrAssignC(OEnv,"AK");
    else if((ajStrMatchC(BEnv, "P1")) && (SEnv == 'C'))
	ajStrAssignC(OEnv,"AL"); 
    else if((ajStrMatchC(BEnv, "P2")) && (SEnv == 'H'))
	ajStrAssignC(OEnv,"AM");
    else if((ajStrMatchC(BEnv, "P2")) && (SEnv == 'S'))
	ajStrAssignC(OEnv,"AN");
    else if((ajStrMatchC(BEnv, "P2")) && (SEnv == 'C'))
	ajStrAssignC(OEnv,"AO");
    else if((ajStrMatchC(BEnv, "P3")) && (SEnv == 'H'))
	ajStrAssignC(OEnv,"AP");
    else if((ajStrMatchC(BEnv, "P3")) && (SEnv == 'S'))
	ajStrAssignC(OEnv,"AQ");
    else if((ajStrMatchC(BEnv, "P3")) && (SEnv == 'C'))
	ajStrAssignC(OEnv,"AR");
    else if((ajStrMatchC(BEnv, "E1")) && (SEnv == 'H'))
	ajStrAssignC(OEnv,"AS");
    else if((ajStrMatchC(BEnv, "E1")) && (SEnv == 'S'))
	ajStrAssignC(OEnv,"AT");
    else if((ajStrMatchC(BEnv, "E1")) && (SEnv == 'C'))
	ajStrAssignC(OEnv,"AU");
    else if((ajStrMatchC(BEnv, "E2")) && (SEnv == 'H'))
	ajStrAssignC(OEnv,"AV");
    else if((ajStrMatchC(BEnv, "E2")) && (SEnv == 'S'))
	ajStrAssignC(OEnv,"AW");
    else if((ajStrMatchC(BEnv, "E2")) && (SEnv == 'C'))
	ajStrAssignC(OEnv,"AX");
    else
    {
 	ajStrSetClear(OEnv); 
	ajFmtPrintF(flog, "OEnv unassigned for residue %d\n", res->Idx);
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
** @param [r] res [const AjPResidue] Residue object
** @param [r] SEnv [char]          Secondary structure environment code
** @param [w] OEnv [AjPStr*]       Character for the overall environment class 
** @param  [w] flog [AjPFile] Log file
** @return [ajint] Number of environments
** @@
******************************************************************************/

ajint ajResidueEnv11(const AjPResidue res, char SEnv, AjPStr *OEnv,
                     AjPFile flog)
{
    float BLimit=5; /*Upper limit for the relative solvent accessible
		      area for a buried residue*/
    float PBLimit=25; /*Upper limit for the relative solvent
			accessible area for a Partially buried
			residue*/

    float PolLimit1=10;
    float PolLimit2=30;
    float PolLimit3=50;
    float PolLimit4=70;
    float PolLimit5=90;

  
    AjPStr   BEnv=NULL;

    if(!res)
    {
	ajWarn("No residue to ajResidueEnv");

	return 0;
    }
  
    ajStrSetClear(OEnv);
    BEnv=ajStrNew();

    ajFmtPrintF(flog, "R:%c-%d S:%c A:%.2f f:%.2f\n",
		res->Id1, res->Idx, res->eType,
		res->side_rel, res->pol_rel);
  
    /*Assign the basic classes*/
  
    if((res->side_rel <= BLimit) &&
       (res->pol_rel <= PolLimit1))
	ajStrAssignC(&BEnv, "B1");    
    else if((res->side_rel <= BLimit) &&
	    (res->pol_rel > PolLimit1) &&
	    (res->pol_rel <= PolLimit2)) 
	ajStrAssignC(&BEnv, "B2");
    else if((res->side_rel <= BLimit) &&
	    (res->pol_rel > PolLimit2) &&
	    (res->pol_rel <= PolLimit3)) 
	ajStrAssignC(&BEnv, "B3");
    else if((res->side_rel <= BLimit) &&
	    (res->pol_rel > PolLimit3) &&
	    (res->pol_rel <= PolLimit4)) 
	ajStrAssignC(&BEnv, "B4");
    else if((res->side_rel <= BLimit) &&
	    (res->pol_rel > PolLimit4) &&
	    (res->pol_rel <= PolLimit5)) 
	ajStrAssignC(&BEnv, "B5");
    else if((res->side_rel <= BLimit) &&
	    (res->pol_rel > PolLimit5)) 
	ajStrAssignC(&BEnv, "B6");
    else if((res->side_rel > BLimit) &&
	    (res->side_rel <= PBLimit))
	ajStrAssignC(&BEnv, "P");
    else if(res->side_rel > PBLimit)
	ajStrAssignC(&BEnv, "E");
    else
    {
	ajStrSetClear(OEnv); 
	ajFmtPrintF(flog, "BEnv unassigned for residue %d\n", res->Idx);
	ajStrDel(&BEnv);

	return 0;
    }
  
    /*Assign overall environment class*/
    if((ajStrMatchC(BEnv, "B1")) && (SEnv == 'H'))
	ajStrAssignC(OEnv,"AA");
    else if((ajStrMatchC(BEnv, "B1")) && (SEnv == 'S'))
	ajStrAssignC(OEnv,"AB");
    else if((ajStrMatchC(BEnv, "B1")) && (SEnv == 'C'))
	ajStrAssignC(OEnv,"AC");
    else if((ajStrMatchC(BEnv, "B2")) && (SEnv == 'H'))
	ajStrAssignC(OEnv,"AD");
    else if((ajStrMatchC(BEnv, "B2")) && (SEnv == 'S'))
	ajStrAssignC(OEnv,"AE");
    else if((ajStrMatchC(BEnv, "B2")) && (SEnv == 'C'))
	ajStrAssignC(OEnv,"AF");
    else if((ajStrMatchC(BEnv, "B3")) && (SEnv == 'H'))
	ajStrAssignC(OEnv,"AG");
    else if((ajStrMatchC(BEnv, "B3")) && (SEnv == 'S'))
	ajStrAssignC(OEnv,"AH");
    else if((ajStrMatchC(BEnv, "B3")) && (SEnv == 'C'))
	ajStrAssignC(OEnv,"AI");
    else if((ajStrMatchC(BEnv, "B4")) && (SEnv == 'H'))
	ajStrAssignC(OEnv,"AJ");
    else if((ajStrMatchC(BEnv, "B4")) && (SEnv == 'S'))
	ajStrAssignC(OEnv,"AK");
    else if((ajStrMatchC(BEnv, "B4")) && (SEnv == 'C'))
	ajStrAssignC(OEnv,"AL"); 
    else if((ajStrMatchC(BEnv, "B5")) && (SEnv == 'H'))
	ajStrAssignC(OEnv,"AM");
    else if((ajStrMatchC(BEnv, "B5")) && (SEnv == 'S'))
	ajStrAssignC(OEnv,"AN");
    else if((ajStrMatchC(BEnv, "B5")) && (SEnv == 'C'))
	ajStrAssignC(OEnv,"AO");
    else if((ajStrMatchC(BEnv, "B6")) && (SEnv == 'H'))
	ajStrAssignC(OEnv,"AP");
    else if((ajStrMatchC(BEnv, "B6")) && (SEnv == 'S'))
	ajStrAssignC(OEnv,"AQ");
    else if((ajStrMatchC(BEnv, "B6")) && (SEnv == 'C'))
	ajStrAssignC(OEnv,"AR");
    else if((ajStrMatchC(BEnv, "P")) && (SEnv == 'H'))
	ajStrAssignC(OEnv,"AS");
    else if((ajStrMatchC(BEnv, "P")) && (SEnv == 'S'))
	ajStrAssignC(OEnv,"AT");
    else if((ajStrMatchC(BEnv, "P")) && (SEnv == 'C'))
	ajStrAssignC(OEnv,"AU");
    else if((ajStrMatchC(BEnv, "E")) && (SEnv == 'H'))
	ajStrAssignC(OEnv,"AV");
    else if((ajStrMatchC(BEnv, "E")) && (SEnv == 'S'))
	ajStrAssignC(OEnv,"AW");
    else if((ajStrMatchC(BEnv, "E")) && (SEnv == 'C'))
	ajStrAssignC(OEnv,"AX");
    else
    {
	ajStrSetClear(OEnv);
	ajFmtPrintF(flog, "OEnv unassigned for residue %d\n", res->Idx);

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
** @param [r] res [const AjPResidue] Residue object
** @param [r] SEnv [char]          Secondary structure environment code
** @param [w] OEnv [AjPStr*]       Character for the overall environment class 
** @param  [w] flog [AjPFile] Log file
** @return [ajint] Number of environments
** @@
******************************************************************************/

ajint ajResidueEnv12(const AjPResidue res, char SEnv, AjPStr *OEnv,
                     AjPFile flog)
{
    float BLimit=5; /* Upper limit for the relative solvent accessible
		       area for a buried residue */
    float PBLimit=25; /* Upper limit for the relative solvent
  			 accessible area for a Partially buried residue */
  
    float PolLimit1=10;
    float PolLimit2=30;
    float PolLimit3=50;
    float PolLimit4=70;
    float PolLimit5=90;

  
    AjPStr   BEnv=NULL;

    if(!res)
    {
	ajWarn("No residue to ajResidueEnv");

	return 0;
    }
  
    ajStrSetClear(OEnv);
    BEnv=ajStrNew();

    ajFmtPrintF(flog, "R:%c-%d S:%c A:%.2f f:%.2f\n", res->Id1, res->Idx,
                res->eType, res->side_rel, res->pol_rel);
  
    /*Assign the basic classes*/
  
    if((res->side_rel <= BLimit))
	ajStrAssignC(&BEnv, "B");    
    else if((res->side_rel > BLimit) &&
	    (res->side_rel <= PBLimit) &&
	    (res->pol_rel <= PolLimit1))
	ajStrAssignC(&BEnv, "P1");
    else if((res->side_rel > BLimit) &&
	    (res->side_rel <= PBLimit) &&
	    (res->pol_rel > PolLimit1) &&
	    (res->pol_rel <= PolLimit2))
	ajStrAssignC(&BEnv, "P2");
    else if((res->side_rel > BLimit) &&
	    (res->side_rel <= PBLimit) &&
	    (res->pol_rel > PolLimit2) &&
	    (res->pol_rel <= PolLimit3))
	ajStrAssignC(&BEnv, "P3");
    else if((res->side_rel > BLimit) &&
	    (res->side_rel <= PBLimit) &&
	    (res->pol_rel > PolLimit3) &&
	    (res->pol_rel <= PolLimit4))
	ajStrAssignC(&BEnv, "P4");
    else if((res->side_rel > BLimit) &&
	    (res->side_rel <= PBLimit) &&
	    (res->pol_rel > PolLimit4) &&
	    (res->pol_rel <= PolLimit5))
	ajStrAssignC(&BEnv, "P5");
    else if((res->side_rel > BLimit) &&
	    (res->side_rel <= PBLimit) &&
	    (res->pol_rel > PolLimit5))
	ajStrAssignC(&BEnv, "P6");
    else if(res->side_rel > PBLimit)
	ajStrAssignC(&BEnv, "E"); 
    else
    {
	ajStrSetClear(OEnv);
	ajFmtPrintF(flog, "BEnv unassigned for residue %d\n", res->Idx);
	ajStrDel(&BEnv);

	return 0;
    }

    
    /*Assign overall environment class*/
    if((ajStrMatchC(BEnv, "B")) && (SEnv == 'H'))
	ajStrAssignC(OEnv,"AA");
    else if((ajStrMatchC(BEnv, "B")) && (SEnv == 'S'))
	ajStrAssignC(OEnv,"AB");
    else if((ajStrMatchC(BEnv, "B")) && (SEnv == 'C'))
	ajStrAssignC(OEnv,"AC");
    else if((ajStrMatchC(BEnv, "P1")) && (SEnv == 'H'))
	ajStrAssignC(OEnv,"AD");
    else if((ajStrMatchC(BEnv, "P1")) && (SEnv == 'S'))
	ajStrAssignC(OEnv,"AE");
    else if((ajStrMatchC(BEnv, "P1")) && (SEnv == 'C'))
	ajStrAssignC(OEnv,"AF");
    else if((ajStrMatchC(BEnv, "P2")) && (SEnv == 'H'))
	ajStrAssignC(OEnv,"AG");
    else if((ajStrMatchC(BEnv, "P2")) && (SEnv == 'S'))
	ajStrAssignC(OEnv,"AH");
    else if((ajStrMatchC(BEnv, "P2")) && (SEnv == 'C'))
	ajStrAssignC(OEnv,"AI");
    else if((ajStrMatchC(BEnv, "P3")) && (SEnv == 'H'))
	ajStrAssignC(OEnv,"AJ");
    else if((ajStrMatchC(BEnv, "P3")) && (SEnv == 'S'))
	ajStrAssignC(OEnv,"AK");
    else if((ajStrMatchC(BEnv, "P3")) && (SEnv == 'C'))
	ajStrAssignC(OEnv,"AL"); 
    else if((ajStrMatchC(BEnv, "P4")) && (SEnv == 'H'))
	ajStrAssignC(OEnv,"AM");
    else if((ajStrMatchC(BEnv, "P4")) && (SEnv == 'S'))
	ajStrAssignC(OEnv,"AN");
    else if((ajStrMatchC(BEnv, "P4")) && (SEnv == 'C'))
	ajStrAssignC(OEnv,"AO");
    else if((ajStrMatchC(BEnv, "P5")) && (SEnv == 'H'))
	ajStrAssignC(OEnv,"AP");
    else if((ajStrMatchC(BEnv, "P5")) && (SEnv == 'S'))
	ajStrAssignC(OEnv,"AQ");
    else if((ajStrMatchC(BEnv, "P5")) && (SEnv == 'C'))
	ajStrAssignC(OEnv,"AR");
    else if((ajStrMatchC(BEnv, "P6")) && (SEnv == 'H'))
	ajStrAssignC(OEnv,"AS");
    else if((ajStrMatchC(BEnv, "P6")) && (SEnv == 'S'))
	ajStrAssignC(OEnv,"AT");
    else if((ajStrMatchC(BEnv, "P6")) && (SEnv == 'C'))
	ajStrAssignC(OEnv,"AU");
    else if((ajStrMatchC(BEnv, "E")) && (SEnv == 'H'))
	ajStrAssignC(OEnv,"AV");
    else if((ajStrMatchC(BEnv, "E")) && (SEnv == 'S'))
	ajStrAssignC(OEnv,"AW");
    else if((ajStrMatchC(BEnv, "E")) && (SEnv == 'C'))
	ajStrAssignC(OEnv,"AX");
    else
    {
	ajStrSetClear(OEnv); 
	ajFmtPrintF(flog, "OEnv unassigned for residue %d\n", res->Idx);
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
** @param [r] res [const AjPResidue] Residue object
** @param [r] SEnv [char]          Secondary structure environment code
** @param [w] OEnv [AjPStr*]       Character for the overall environment class 
** @param  [w] flog [AjPFile] Log file
** @return [ajint] Number of environments
** @@
******************************************************************************/

ajint ajResidueEnv13(const AjPResidue res, char SEnv, AjPStr *OEnv,
                     AjPFile flog)
{
    float BLimit=5; /* Upper limit for the relative solvent accessible
		       area for a buried residue */
    float PBLimit=25; /* Upper limit for the relative solvent
  			 accessible area for a Partially buried residue */
  
    float PolLimit1=10;
    float PolLimit2=30;
    float PolLimit3=50;
    float PolLimit4=70;
    float PolLimit5=90;

    AjPStr   BEnv=NULL;

    if(!res)
    {
	ajWarn("No residue to ajResidueEnv");

	return 0;
    }
  
    ajStrSetClear(OEnv);
    BEnv=ajStrNew();

    ajFmtPrintF(flog, "R:%c-%d S:%c A:%.2f f:%.2f\n",
		res->Id1, res->Idx, res->eType,
		res->side_rel, res->pol_rel);
  
    /*Assign the basic classes*/
    if((res->side_rel <= BLimit))
	ajStrAssignC(&BEnv, "B");    
    else if((res->side_rel > BLimit) &&
	    (res->side_rel <= PBLimit))
	ajStrAssignC(&BEnv, "P");
    else if((res->side_rel > PBLimit) &&
	    (res->pol_rel <= PolLimit1))
	ajStrAssignC(&BEnv, "E1");
    else if((res->side_rel > PBLimit) &&
	    (res->pol_rel > PolLimit1) &&
	    (res->pol_rel <= PolLimit2))
	ajStrAssignC(&BEnv, "E2");
    else if((res->side_rel > PBLimit) &&
	    (res->pol_rel > PolLimit2) &&
	    (res->pol_rel <= PolLimit3))
	ajStrAssignC(&BEnv, "E3");
    else if((res->side_rel > PBLimit) &&
	    (res->pol_rel > PolLimit3) &&
	    (res->pol_rel <= PolLimit4))
	ajStrAssignC(&BEnv, "E4"); 
    else if((res->side_rel > PBLimit) &&
	    (res->pol_rel > PolLimit4) &&
	    (res->pol_rel <= PolLimit5))
	ajStrAssignC(&BEnv, "E5"); 
    else if((res->side_rel > PBLimit) &&
	    (res->pol_rel > PolLimit5))
	ajStrAssignC(&BEnv, "E6");
    else
    {
 	ajStrSetClear(OEnv); 
	ajFmtPrintF(flog, "BEnv unassigned for residue %d\n", res->Idx);
	ajStrDel(&BEnv);

	return 0;
    }

    
    /*Assign overall environment class*/
    if((ajStrMatchC(BEnv, "B")) && (SEnv == 'H'))
	ajStrAssignC(OEnv,"AA");
    else if((ajStrMatchC(BEnv, "B")) && (SEnv == 'S'))
	ajStrAssignC(OEnv,"AB");
    else if((ajStrMatchC(BEnv, "B")) && (SEnv == 'C'))
	ajStrAssignC(OEnv,"AC");
    else if((ajStrMatchC(BEnv, "P")) && (SEnv == 'H'))
	ajStrAssignC(OEnv,"AD");
    else if((ajStrMatchC(BEnv, "P")) && (SEnv == 'S'))
	ajStrAssignC(OEnv,"AE");
    else if((ajStrMatchC(BEnv, "P")) && (SEnv == 'C'))
	ajStrAssignC(OEnv,"AF");
    else if((ajStrMatchC(BEnv, "E1")) && (SEnv == 'H'))
	ajStrAssignC(OEnv,"AG");
    else if((ajStrMatchC(BEnv, "E1")) && (SEnv == 'S'))
	ajStrAssignC(OEnv,"AH");
    else if((ajStrMatchC(BEnv, "E1")) && (SEnv == 'C'))
	ajStrAssignC(OEnv,"AI");
    else if((ajStrMatchC(BEnv, "E2")) && (SEnv == 'H'))
	ajStrAssignC(OEnv,"AJ");
    else if((ajStrMatchC(BEnv, "E2")) && (SEnv == 'S'))
	ajStrAssignC(OEnv,"AK");
    else if((ajStrMatchC(BEnv, "E2")) && (SEnv == 'C'))
	ajStrAssignC(OEnv,"AL"); 
    else if((ajStrMatchC(BEnv, "E3")) && (SEnv == 'H'))
	ajStrAssignC(OEnv,"AM");
    else if((ajStrMatchC(BEnv, "E3")) && (SEnv == 'S'))
	ajStrAssignC(OEnv,"AN");
    else if((ajStrMatchC(BEnv, "E3")) && (SEnv == 'C'))
	ajStrAssignC(OEnv,"AO");
    else if((ajStrMatchC(BEnv, "E4")) && (SEnv == 'H'))
	ajStrAssignC(OEnv,"AP");
    else if((ajStrMatchC(BEnv, "E4")) && (SEnv == 'S'))
	ajStrAssignC(OEnv,"AQ");
    else if((ajStrMatchC(BEnv, "E4")) && (SEnv == 'C'))
	ajStrAssignC(OEnv,"AR");
    else if((ajStrMatchC(BEnv, "E5")) && (SEnv == 'H'))
	ajStrAssignC(OEnv,"AS");
    else if((ajStrMatchC(BEnv, "E5")) && (SEnv == 'S'))
	ajStrAssignC(OEnv,"AT");
    else if((ajStrMatchC(BEnv, "E5")) && (SEnv == 'C'))
	ajStrAssignC(OEnv,"AU");
    else if((ajStrMatchC(BEnv, "E6")) && (SEnv == 'H'))
	ajStrAssignC(OEnv,"AV");
    else if((ajStrMatchC(BEnv, "E6")) && (SEnv == 'S'))
	ajStrAssignC(OEnv,"AW");
    else if((ajStrMatchC(BEnv, "E6")) && (SEnv == 'C'))
	ajStrAssignC(OEnv,"AX");
    else
    {
	ajStrSetClear(OEnv); 
	ajFmtPrintF(flog, "OEnv unassigned for residue %d\n", res->Idx);
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
** @param [r] res [const AjPResidue] Residue object
** @param [r] SEnv [char]          Secondary structure environment code
** @param [w] OEnv [AjPStr*]       Character for the overall environment class 
** @param  [w] flog [AjPFile] Log file
** @return [ajint] Number of environments
** @@
******************************************************************************/

ajint ajResidueEnv14(const AjPResidue res, char SEnv, AjPStr *OEnv,
                     AjPFile flog)
{
    float PBLimit=25; /* Upper limit for the relative solvent
  			accessible area for a Partially buried residue */
  
    float PolLimit1=5;
    float PolLimit2=15;
    float PolLimit3=25;
    float PolLimit4=40;
    float PolLimit5=60;
    float PolLimit6=80;
 

    AjPStr   BEnv=NULL;

    if(!res)
    {
	ajWarn("No residue to ajResidueEnv");

	return 0;
    }
  
    ajStrSetClear(OEnv);
    BEnv=ajStrNew();

    ajFmtPrintF(flog, "R:%c-%d S:%c A:%.2f f:%.2f\n",
		res->Id1, res->Idx, res->eType,
		res->side_rel, res->pol_rel);

    if((res->side_rel <= PBLimit) &&
       (res->pol_rel <= PolLimit1))
	ajStrAssignC(&BEnv, "B1");
    else if((res->side_rel <= PBLimit) &&
	    (res->pol_rel > PolLimit1) &&
	    (res->pol_rel <= PolLimit2))
	ajStrAssignC(&BEnv, "B2");
    else if((res->side_rel <= PBLimit) &&
	    (res->pol_rel > PolLimit2) &&
	    (res->pol_rel <= PolLimit3))
	ajStrAssignC(&BEnv, "B3");
    else if((res->side_rel <= PBLimit) &&
	    (res->pol_rel > PolLimit3) &&
	    (res->pol_rel <= PolLimit4))
	ajStrAssignC(&BEnv, "B4");
    else if((res->side_rel <= PBLimit) &&
	    (res->pol_rel > PolLimit4) &&
	    (res->pol_rel <= PolLimit5))
	ajStrAssignC(&BEnv, "B5");
    else if((res->side_rel <= PBLimit) &&
	    (res->pol_rel > PolLimit5) &&
	    (res->pol_rel <= PolLimit6))
	ajStrAssignC(&BEnv, "B6");
    else if((res->side_rel <= PBLimit) &&
	    (res->pol_rel > PolLimit6))
	ajStrAssignC(&BEnv, "B7");
    else if((res->side_rel > PBLimit))
	ajStrAssignC(&BEnv, "E");
    else
    {
 	ajStrSetClear(OEnv);
	ajFmtPrintF(flog, "BEnv unassigned for residue %d\n", res->Idx);
	ajStrDel(&BEnv);

	return 0;
    }

    /*Assign overall environment class*/
    if((ajStrMatchC(BEnv, "B1")) && (SEnv == 'H'))
	ajStrAssignC(OEnv,"AA");
    else if((ajStrMatchC(BEnv, "B1")) && (SEnv == 'S'))
	ajStrAssignC(OEnv,"AB");
    else if((ajStrMatchC(BEnv, "B1")) && (SEnv == 'C'))
	ajStrAssignC(OEnv,"AC");
    else if((ajStrMatchC(BEnv, "B2")) && (SEnv == 'H'))
	ajStrAssignC(OEnv,"AD");
    else if((ajStrMatchC(BEnv, "B2")) && (SEnv == 'S'))
	ajStrAssignC(OEnv,"AE");
    else if((ajStrMatchC(BEnv, "B2")) && (SEnv == 'C'))
	ajStrAssignC(OEnv,"AF");
    else if((ajStrMatchC(BEnv, "B3")) && (SEnv == 'H'))
	ajStrAssignC(OEnv,"AG");
    else if((ajStrMatchC(BEnv, "B3")) && (SEnv == 'S'))
	ajStrAssignC(OEnv,"AH");
    else if((ajStrMatchC(BEnv, "B3")) && (SEnv == 'C'))
	ajStrAssignC(OEnv,"AI");
    else if((ajStrMatchC(BEnv, "B4")) && (SEnv == 'H'))
	ajStrAssignC(OEnv,"AJ");
    else if((ajStrMatchC(BEnv, "B4")) && (SEnv == 'S'))
	ajStrAssignC(OEnv,"AK");
    else if((ajStrMatchC(BEnv, "B4")) && (SEnv == 'C'))
	ajStrAssignC(OEnv,"AL"); 
    else if((ajStrMatchC(BEnv, "B5")) && (SEnv == 'H'))
	ajStrAssignC(OEnv,"AM");
    else if((ajStrMatchC(BEnv, "B5")) && (SEnv == 'S'))
	ajStrAssignC(OEnv,"AN");
    else if((ajStrMatchC(BEnv, "B5")) && (SEnv == 'C'))
	ajStrAssignC(OEnv,"AO");
    else if((ajStrMatchC(BEnv, "B6")) && (SEnv == 'H'))
	ajStrAssignC(OEnv,"AP");
    else if((ajStrMatchC(BEnv, "B6")) && (SEnv == 'S'))
	ajStrAssignC(OEnv,"AQ");
    else if((ajStrMatchC(BEnv, "B6")) && (SEnv == 'C'))
	ajStrAssignC(OEnv,"AR");
    else if((ajStrMatchC(BEnv, "B7")) && (SEnv == 'H'))
	ajStrAssignC(OEnv,"AS");
    else if((ajStrMatchC(BEnv, "B7")) && (SEnv == 'S'))
	ajStrAssignC(OEnv,"AT");
    else if((ajStrMatchC(BEnv, "B7")) && (SEnv == 'C'))
	ajStrAssignC(OEnv,"AU");
    else if((ajStrMatchC(BEnv, "E")) && (SEnv == 'H'))
	ajStrAssignC(OEnv,"AV");
    else if((ajStrMatchC(BEnv, "E")) && (SEnv == 'S'))
	ajStrAssignC(OEnv,"AW");
    else if((ajStrMatchC(BEnv, "E")) && (SEnv == 'C'))
	ajStrAssignC(OEnv,"AX");
    else
    {
	ajStrSetClear(OEnv); 
	ajFmtPrintF(flog, "OEnv unassigned for residue %d\n", res->Idx);
	ajStrDel(&BEnv);

	return 0;
    }

    ajStrDel(&BEnv);

    return 24;
}




/* @func ajResidueEnv15 ******************************************************
**
** Assigns environment based on side chain accessibility and secondary 
** structure.   Assigns environment of "*" as default.
**
** @param [r] res [const AjPResidue] Residue object
** @param [r] SEnv [char]          Secondary structure environment code
** @param [w] OEnv [AjPStr*]       Character for the overall environment class 
** @param  [w] flog [AjPFile] Log file
** @return [ajint] Number of environments
** @@
******************************************************************************/

ajint ajResidueEnv15(const AjPResidue res, char SEnv, AjPStr *OEnv,
                     AjPFile flog)
{
    float PBLimit=25;			/* Upper limit for the
  					   relative solvent accessible
  					   area for a Partially buried
  					   residue */
  
    float PolLimit1=5;
    float PolLimit2=15;
    float PolLimit3=25;
    float PolLimit4=35;
    float PolLimit5=45;
    float PolLimit6=75;
 

    AjPStr   BEnv=NULL;

    if(!res)
    {
	ajWarn("No residue to ajResidueEnv");

	return 0;
    }
  
    ajStrSetClear(OEnv);
    BEnv=ajStrNew();

    ajFmtPrintF(flog, "R:%c-%d S:%c A:%.2f f:%.2f\n",
		res->Id1, res->Idx, res->eType,
		res->side_rel, res->pol_rel);

    if((res->side_rel <= PBLimit) && (res->pol_rel <= PolLimit1))
	ajStrAssignC(&BEnv, "B1");
    else if((res->side_rel <= PBLimit) &&
	    (res->pol_rel > PolLimit1) &&
	    (res->pol_rel <= PolLimit2))
	ajStrAssignC(&BEnv, "B2");
    else if((res->side_rel <= PBLimit) &&
	    (res->pol_rel > PolLimit2) &&
	    (res->pol_rel <= PolLimit3))
	ajStrAssignC(&BEnv, "B3");
    else if((res->side_rel <= PBLimit) &&
	    (res->pol_rel > PolLimit3) &&
	    (res->pol_rel <= PolLimit4))
	ajStrAssignC(&BEnv, "B4");
    else if((res->side_rel <= PBLimit) &&
	    (res->pol_rel > PolLimit4) &&
	    (res->pol_rel <= PolLimit5))
	ajStrAssignC(&BEnv, "B5");
    else if((res->side_rel <= PBLimit) &&
	    (res->pol_rel > PolLimit5) &&
	    (res->pol_rel <= PolLimit6))
	ajStrAssignC(&BEnv, "B6");
    else if((res->side_rel <= PBLimit) &&
	    (res->pol_rel > PolLimit6))
	ajStrAssignC(&BEnv, "B7");
    else if((res->side_rel > PBLimit))
	ajStrAssignC(&BEnv, "E");
    else
    {
	ajStrSetClear(OEnv); 
	ajFmtPrintF(flog, "BEnv unassigned for residue %d\n", res->Idx);
	ajStrDel(&BEnv);

	return 0;
    }

    /*Assign overall environment class*/
    if((ajStrMatchC(BEnv, "B1")) && (SEnv == 'H'))
	ajStrAssignC(OEnv,"AA");
    else if((ajStrMatchC(BEnv, "B1")) && (SEnv == 'S'))
	ajStrAssignC(OEnv,"AB");
    else if((ajStrMatchC(BEnv, "B1")) && (SEnv == 'C'))
	ajStrAssignC(OEnv,"AC");
    else if((ajStrMatchC(BEnv, "B2")) && (SEnv == 'H'))
	ajStrAssignC(OEnv,"AD");
    else if((ajStrMatchC(BEnv, "B2")) && (SEnv == 'S'))
	ajStrAssignC(OEnv,"AE");
    else if((ajStrMatchC(BEnv, "B2")) && (SEnv == 'C'))
	ajStrAssignC(OEnv,"AF");
    else if((ajStrMatchC(BEnv, "B3")) && (SEnv == 'H'))
	ajStrAssignC(OEnv,"AG");
    else if((ajStrMatchC(BEnv, "B3")) && (SEnv == 'S'))
	ajStrAssignC(OEnv,"AH");
    else if((ajStrMatchC(BEnv, "B3")) && (SEnv == 'C'))
	ajStrAssignC(OEnv,"AI");
    else if((ajStrMatchC(BEnv, "B4")) && (SEnv == 'H'))
	ajStrAssignC(OEnv,"AJ");
    else if((ajStrMatchC(BEnv, "B4")) && (SEnv == 'S'))
	ajStrAssignC(OEnv,"AK");
    else if((ajStrMatchC(BEnv, "B4")) && (SEnv == 'C'))
	ajStrAssignC(OEnv,"AL"); 
    else if((ajStrMatchC(BEnv, "B5")) && (SEnv == 'H'))
	ajStrAssignC(OEnv,"AM");
    else if((ajStrMatchC(BEnv, "B5")) && (SEnv == 'S'))
	ajStrAssignC(OEnv,"AN");
    else if((ajStrMatchC(BEnv, "B5")) && (SEnv == 'C'))
	ajStrAssignC(OEnv,"AO");
    else if((ajStrMatchC(BEnv, "B6")) && (SEnv == 'H'))
	ajStrAssignC(OEnv,"AP");
    else if((ajStrMatchC(BEnv, "B6")) && (SEnv == 'S'))
	ajStrAssignC(OEnv,"AQ");
    else if((ajStrMatchC(BEnv, "B6")) && (SEnv == 'C'))
	ajStrAssignC(OEnv,"AR");
    else if((ajStrMatchC(BEnv, "B7")) && (SEnv == 'H'))
	ajStrAssignC(OEnv,"AS");
    else if((ajStrMatchC(BEnv, "B7")) && (SEnv == 'S'))
	ajStrAssignC(OEnv,"AT");
    else if((ajStrMatchC(BEnv, "B7")) && (SEnv == 'C'))
	ajStrAssignC(OEnv,"AU");
    else if((ajStrMatchC(BEnv, "E")) && (SEnv == 'H'))
	ajStrAssignC(OEnv,"AV");
    else if((ajStrMatchC(BEnv, "E")) && (SEnv == 'S'))
	ajStrAssignC(OEnv,"AW");
    else if((ajStrMatchC(BEnv, "E")) && (SEnv == 'C'))
	ajStrAssignC(OEnv,"AX");
    else
    {
	ajStrSetClear(OEnv); 
	ajFmtPrintF(flog, "OEnv unassigned for residue %d\n", res->Idx);
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
** @param [r] res [const AjPResidue] Residue object
** @param [r] SEnv [char]          Secondary structure environment code
** @param [w] OEnv [AjPStr*]       Character for the overall environment class 
** @param  [w] flog [AjPFile] Log file
** @return [ajint] Number of environments
** @@
******************************************************************************/
ajint ajResidueEnv16(const AjPResidue res, char SEnv, AjPStr *OEnv,
                     AjPFile flog)
{
    float BLimit=5; /* Upper limit for the relative solvent accessible
		       area for a buried residue */
  
    float PolLimit1=5;
    float PolLimit2=15;
    float PolLimit3=25;
    float PolLimit4=35;
    float PolLimit5=45;
    float PolLimit6=75;
 

    AjPStr   BEnv=NULL;

    if(!res)
    {
	ajWarn("No residue to ajResidueEnv");

	return 0;
    }
  
    ajStrSetClear(OEnv);
    BEnv=ajStrNew();

    ajFmtPrintF(flog, "R:%c-%d S:%c A:%.2f f:%.2f\n",
		res->Id1, res->Idx, 
		res->eType, res->side_rel, res->pol_rel);

    if((res->side_rel <= BLimit) &&
       (res->pol_rel <= PolLimit1))
	ajStrAssignC(&BEnv, "B1");
    else if((res->side_rel <= BLimit) &&
	    (res->pol_rel > PolLimit1) &&
	    (res->pol_rel <= PolLimit2))
	ajStrAssignC(&BEnv, "B2");
    else if((res->side_rel <= BLimit) &&
	    (res->pol_rel > PolLimit2) &&
	    (res->pol_rel <= PolLimit3))
	ajStrAssignC(&BEnv, "B3");
    else if((res->side_rel <= BLimit) &&
	    (res->pol_rel > PolLimit3) &&
	    (res->pol_rel <= PolLimit4))
	ajStrAssignC(&BEnv, "B4");
    else if((res->side_rel <= BLimit) &&
	    (res->pol_rel > PolLimit4) &&
	    (res->pol_rel <= PolLimit5))
	ajStrAssignC(&BEnv, "B5");
    else if((res->side_rel <= BLimit) &&
	    (res->pol_rel > PolLimit5) &&
	    (res->pol_rel <= PolLimit6))
	ajStrAssignC(&BEnv, "B6");
    else if((res->side_rel <= BLimit) &&
	    (res->pol_rel > PolLimit6))
	ajStrAssignC(&BEnv, "B7");
    else if((res->side_rel > BLimit))
	ajStrAssignC(&BEnv, "E");
    else
    {
	ajStrSetClear(OEnv); 
	ajFmtPrintF(flog, "BEnv unassigned for residue %d\n", res->Idx);
	ajStrDel(&BEnv);

	return 0;
    }

    /*Assign overall environment class*/
    if((ajStrMatchC(BEnv, "B1")) && (SEnv == 'H'))
	ajStrAssignC(OEnv,"AA");
    else if((ajStrMatchC(BEnv, "B1")) && (SEnv == 'S'))
	ajStrAssignC(OEnv,"AB");
    else if((ajStrMatchC(BEnv, "B1")) && (SEnv == 'C'))
	ajStrAssignC(OEnv,"AC");
    else if((ajStrMatchC(BEnv, "B2")) && (SEnv == 'H'))
	ajStrAssignC(OEnv,"AD");
    else if((ajStrMatchC(BEnv, "B2")) && (SEnv == 'S'))
	ajStrAssignC(OEnv,"AE");
    else if((ajStrMatchC(BEnv, "B2")) && (SEnv == 'C'))
	ajStrAssignC(OEnv,"AF");
    else if((ajStrMatchC(BEnv, "B3")) && (SEnv == 'H'))
	ajStrAssignC(OEnv,"AG");
    else if((ajStrMatchC(BEnv, "B3")) && (SEnv == 'S'))
	ajStrAssignC(OEnv,"AH");
    else if((ajStrMatchC(BEnv, "B3")) && (SEnv == 'C'))
	ajStrAssignC(OEnv,"AI");
    else if((ajStrMatchC(BEnv, "B4")) && (SEnv == 'H'))
	ajStrAssignC(OEnv,"AJ");
    else if((ajStrMatchC(BEnv, "B4")) && (SEnv == 'S'))
	ajStrAssignC(OEnv,"AK");
    else if((ajStrMatchC(BEnv, "B4")) && (SEnv == 'C'))
	ajStrAssignC(OEnv,"AL"); 
    else if((ajStrMatchC(BEnv, "B5")) && (SEnv == 'H'))
	ajStrAssignC(OEnv,"AM");
    else if((ajStrMatchC(BEnv, "B5")) && (SEnv == 'S'))
	ajStrAssignC(OEnv,"AN");
    else if((ajStrMatchC(BEnv, "B5")) && (SEnv == 'C'))
	ajStrAssignC(OEnv,"AO");
    else if((ajStrMatchC(BEnv, "B6")) && (SEnv == 'H'))
	ajStrAssignC(OEnv,"AP");
    else if((ajStrMatchC(BEnv, "B6")) && (SEnv == 'S'))
	ajStrAssignC(OEnv,"AQ");
    else if((ajStrMatchC(BEnv, "B6")) && (SEnv == 'C'))
	ajStrAssignC(OEnv,"AR");
    else if((ajStrMatchC(BEnv, "B7")) && (SEnv == 'H'))
	ajStrAssignC(OEnv,"AS");
    else if((ajStrMatchC(BEnv, "B7")) && (SEnv == 'S'))
	ajStrAssignC(OEnv,"AT");
    else if((ajStrMatchC(BEnv, "B7")) && (SEnv == 'C'))
	ajStrAssignC(OEnv,"AU");
    else if((ajStrMatchC(BEnv, "E")) && (SEnv == 'H'))
	ajStrAssignC(OEnv,"AV");
    else if((ajStrMatchC(BEnv, "E")) && (SEnv == 'S'))
	ajStrAssignC(OEnv,"AW");
    else if((ajStrMatchC(BEnv, "E")) && (SEnv == 'C'))
	ajStrAssignC(OEnv,"AX");
    else
    {
	ajStrSetClear(OEnv); 
	ajFmtPrintF(flog, "OEnv unassigned for residue %d\n", res->Idx);
	ajStrDel(&BEnv);

	return 0;
    }


    ajStrDel(&BEnv);

    return 24;
}




/* @func ajPdbGetEStrideType ************************************************
**
** Reads a Pdb object and writes a string with the secondary structure. The
** string that is written is the same length as the full-length chain 
** (regardless of whether coordinates for a residue were available or not) 
** therefore it can be indexed into using residue numbers.  The string is 
** allocated if necessary.  If secondary structure assignment was not available
** for a residue a '.' is given in the string.
**
** @param [r] obj [const AjPPdb]  Pdb object
** @param [r] chn [ajint]   Chain number
** @param [w] EStrideType [AjPStr *] String to hold secondary structure
**
** @return [ajint] Length (residues) of array that was written or -1 (for 
**                 an error)
** @@
****************************************************************************/

ajint  ajPdbGetEStrideType(const AjPPdb obj, ajint chn, AjPStr *EStrideType)
{
    AjPResidue tmp    = NULL;
    AjIList    iter   = NULL;
    ajint      idx    = 0;
    

    if(!obj || !EStrideType || (chn<1))
    {
	ajWarn("Bad args passed to ajPdbGetEStrideType");

	return -1;
    }

    if(chn > obj->Nchn)
    {
	ajWarn("chn arg in ajPdbGetEStrideType exceeds no. chains");

	return -1;
    }
    else 
	idx = chn-1;
    
    /* +1 is for the NULL */
    if(!(*EStrideType))
	*EStrideType = ajStrNewRes(obj->Chains[idx]->Nres+1);
    else
    {
	ajStrDel(EStrideType);
	*EStrideType = ajStrNewRes(obj->Chains[idx]->Nres+1);
    }
    
    /* Set all positions to . */
    ajStrAppendCountK(EStrideType, '.', obj->Chains[idx]->Nres);   

    iter=ajListIterNewread(obj->Chains[idx]->Residues); 

    while((tmp=(AjPResidue)ajListIterGet(iter)))
	(*EStrideType)->Ptr[tmp->Idx-1] = tmp->eStrideType;
    

    ajListIterDel(&iter);

    return obj->Chains[idx]->Nres;
}




/* ======================================================================= */
/* ============================== Casts ===================================*/
/* ======================================================================= */




/* @section Casts ***********************************************************
**
** These functions examine the contents of an instance and return some
** derived information. Some of them provide access to the internal
** components of an instance. They are provided for programming convenience
** but should be used with caution.
**
****************************************************************************/




/* ======================================================================= */
/* =========================== Reporters ==================================*/
/* ======================================================================= */




/* @section Reporters *******************************************************
**
** These functions return the contents of an instance but do not make any 
** changes.
**
****************************************************************************/




/* @func ajPdbChnidToNum ****************************************************
**
** Finds the chain number for a given chain identifier in a pdb structure
**
** @param [r] id  [char]    Chain identifier
** @param [r] pdb [const AjPPdb]  Pdb object
** @param [w] chn [ajint *] Chain number
**
** @return [AjBool] True on success
** @@
****************************************************************************/

AjBool ajPdbChnidToNum(char id, const AjPPdb pdb, ajint *chn)
{
    ajint a;
    
    for(a=0;a<pdb->Nchn;a++)
    {
	if(toupper((int)pdb->Chains[a]->Id) == toupper((int)id))
	{
	    *chn=a+1;

	    return ajTrue;
	}

	/*
	** Cope with chain id's of ' ' (which might be given as '.' in 
	**the Pdb object)
	*/
	if((id==' ')&&(pdb->Chains[a]->Id=='.'))
	{
	    *chn=a+1;

	    return ajTrue;
	}
    }
    
    /*
    ** A '.' may be given as the id for domains comprising more than one
    ** chain
    */
    if(id=='.')
    {
	*chn=1;

	return ajTrue;
    }
	
    return ajFalse;
}




/* @func ajPdbtospArrFindPdbid **********************************************
**
** Performs a binary search for a PDB code over an array of Pdbtosp
** structures (which of course must first have been sorted). This is a 
** case-insensitive search.
**
** @param [r] arr [AjPPdbtosp const *] Array of AjOPdbtosp objects
** @param [r] siz [ajint] Size of array
** @param [r] id  [const AjPStr]      Search term
**
** @return [ajint] Index of first Pdbtosp object found with an PDB code
** matching id, or -1 if id is not found.
** @@
****************************************************************************/

ajint ajPdbtospArrFindPdbid(AjPPdbtosp const *arr, ajint siz, const AjPStr id)
{
    int l;
    int m;
    int h;
    int c;


    l = 0;
    h = siz-1;

    while(l<=h)
    {
        m=(l+h)>>1;

        if((c=ajStrCmpCaseS(id, arr[m]->Pdb)) < 0) 
	    h=m-1;
        else if(c>0) 
	    l=m+1;
        else 
	    return m;
    }

    return -1;
}




/* ======================================================================= */
/* ========================== Input & Output ============================= */
/* ======================================================================= */




/* @section Input & output **************************************************
**
** These functions are used for formatted input and output to file.    
**
****************************************************************************/




/* @func ajPdbWriteAll *****************************************************
**
** Writes a clean coordinate file for a protein.
**
** @param [w] outf [AjPFile] Output file stream
** @param [r] obj  [const AjPPdb]  Pdb object
**
** @return [AjBool] True on success
** @category output [AjPPdb] Writes a ccf-format file for a protein.
** @@
****************************************************************************/

AjBool ajPdbWriteAll(AjPFile outf, const AjPPdb obj)
{
    ajint x         = 0;
    ajint y         = 0;
    AjIList iter    = NULL;
    AjPAtom tmp     = NULL;
    AjPResidue tmpr = NULL;
    AjPSeqout outseq;

    /* Write the header information */

    ajFmtPrintF(outf, "%-5s%S\n", "ID", obj->Pdb);
    ajFmtPrintF(outf, "XX\n");

    ajFmtPrintSplit(outf,obj->Compnd,"DE   ",75," \t\r\n");
    ajFmtPrintF(outf, "XX\n");

    ajFmtPrintSplit(outf,obj->Source,"OS   ",75," \t\r\n");
    ajFmtPrintF(outf, "XX\n");

    ajFmtPrintF(outf, "%-5sMETHOD ", "EX");

    if(obj->Method == ajXRAY)
	ajFmtPrintF(outf, "xray; ");	
    else
	ajFmtPrintF(outf, "nmr_or_model; ");		
    ajFmtPrintF(outf, "RESO %.2f; NMOD %d; NCHN %d; NGRP %d;\n", obj->Reso,
		obj->Nmod, obj->Nchn, obj->Ngp);


    /* Write chain-specific information */
    for(x=0;x<obj->Nchn;x++)
    { 
	ajFmtPrintF(outf, "XX\n");	
	ajFmtPrintF(outf, "%-5s[%d]\n", 
		    "CN", 
		    x+1);	
	ajFmtPrintF(outf, "XX\n");	
 
	ajFmtPrintF(outf, "%-5s", "IN");

	if(obj->Chains[x]->Id == ' ')
	    ajFmtPrintF(outf, "ID %c; ", '.');
	else
	    ajFmtPrintF(outf, "ID %c; ", obj->Chains[x]->Id);

	
	/*
	ajFmtPrintF(outf, "NR %d; NL %d; NH %d; NE %d; NS %d; NT %d;\n", 
		    obj->Chains[x]->Nres,
		    obj->Chains[x]->Nlig,
		    obj->Chains[x]->numHelices, 
		    obj->Chains[x]->numStrands, 
		    obj->Chains[x]->numSheets, 
		    obj->Chains[x]->numTurns);
		    */

	ajFmtPrintF(outf, "NR %d; NL %d; NH %d; NE %d;\n", 
		    obj->Chains[x]->Nres,
		    obj->Chains[x]->Nlig,
		    obj->Chains[x]->numHelices, 
		    obj->Chains[x]->numStrands);

	/*
	ajFmtPrintF(outf, "%-5sID %c; NR %d; NH %d; NW %d;\n", 
		    "IN", 
		    obj->Chains[x]->Id,
		    obj->Chains[x]->Nres,
		    obj->Chains[x]->Nhet,
		    obj->Chains[x]->Nwat);
		    */
	ajFmtPrintF(outf, "XX\n");	
	outseq = ajSeqoutNewFile(outf);
	ajSeqoutDumpSwisslike(outseq, obj->Chains[x]->Seq, "SQ");
	ajSeqoutDel(&outseq);
    }
    ajFmtPrintF(outf, "XX\n");	

    /* printf("NCHN: %d   NMOD: %d\n", obj->Nchn, obj->Nmod); */
    
    

    /* Write RESIDUES list */
    for(x=1;x<=obj->Nmod;x++)
    {
	for(y=0;y<obj->Nchn;y++)
	{
	    iter=ajListIterNewread(obj->Chains[y]->Residues);

	    while(!ajListIterDone(iter))
	    {
		tmpr=(AjPResidue)ajListIterGet(iter);

		if(tmpr->Mod>x)
		    break;
		else if(tmpr->Mod!=x)
		    continue;
		else	
		{
		    ajFmtPrintF(outf, "%-5s%-5d%-5d%-5d%-6S%-2c%-6S", 
				"RE", 
				tmpr->Mod, 
				tmpr->Chn, 
				tmpr->Idx, 
				tmpr->Pdb,
				tmpr->Id1, 
				tmpr->Id3);
		    
			
		    if(tmpr->eNum != 0)
			ajFmtPrintF(outf, "%-5d", tmpr->eNum);
		    else
			ajFmtPrintF(outf, "%-5c", '.');

		    ajFmtPrintF(outf, "%-5S%-5c", tmpr->eId, tmpr->eType);

		    if(tmpr->eType == 'H')
			ajFmtPrintF(outf, "%-5d", tmpr->eClass);
		    else
			ajFmtPrintF(outf, "%-5c", '.');
		    
		    if(tmpr->eStrideNum != 0)
			ajFmtPrintF(outf, "%-5d", tmpr->eStrideNum);
		    else
			ajFmtPrintF(outf, "%-5c", '.');

		    ajFmtPrintF(outf, "%-5c", tmpr->eStrideType);


		    ajFmtPrintF(outf, "%8.2f%8.2f%8.2f%8.2f%8.2f"
				"%8.2f%8.2f%8.2f%8.2f%8.2f%8.2f%8.2f"
				"%8.2f\n", 
				tmpr->Phi,
				tmpr->Psi,
				tmpr->Area, 
				tmpr->all_abs, 
				tmpr->all_rel, 
				tmpr->side_abs, 
				tmpr->side_rel, 
				tmpr->main_abs, 
				tmpr->main_rel, 
				tmpr->npol_abs, 
				tmpr->npol_rel, 
				tmpr->pol_abs, 
				tmpr->pol_rel);
		}
	    }

	    ajListIterDel(&iter);			
	}
    }


    /* Write ATOMS list */
    for(x=1;x<=obj->Nmod;x++)
    {
	for(y=0;y<obj->Nchn;y++)
	{
	    /* Print out chain-specific coordinates */
	    iter=ajListIterNewread(obj->Chains[y]->Atoms);

	    while(!ajListIterDone(iter))
	    {
		tmp=(AjPAtom)ajListIterGet(iter);

		if(tmp->Mod>x)
		    break;
		else if(tmp->Mod!=x)
		    continue;
		else	
		{
		    if(tmp->Type=='H')
			ajFmtPrintF(outf, "%-5s%-5d%-5d%-5d%-5c%-6S%-2c%-6S"
                                    "%-2c%-6S"
				    "%9.3f%9.3f%9.3f%8.2f%8.2f\n", 
				    "AT", 
				    tmp->Mod, 
				    tmp->Chn, 
				    tmp->Gpn, 
				    '.',
				    tmp->Pdb, 
				    tmp->Id1,
				    tmp->Id3,
				    tmp->Type, 
				    tmp->Atm, 
				    tmp->X, 
				    tmp->Y, 
				    tmp->Z,
				    tmp->O,
				    tmp->B);
		    else
		    {
			ajFmtPrintF(outf, "%-5s%-5d%-5d%-5c%-5d%-6S%-2c%-6S"
                                    "%-2c%-6S"
				    "%9.3f%9.3f%9.3f%8.2f%8.2f\n", 
				    "AT", 
				    tmp->Mod, 
				    tmp->Chn, 
				    '.', 
				    tmp->Idx, 
				    tmp->Pdb, 
				    tmp->Id1,
				    tmp->Id3,
				    tmp->Type, 
				    tmp->Atm, 
				    tmp->X, 
				    tmp->Y, 
				    tmp->Z,
				    tmp->O,
				    tmp->B);
		    }
		}
	    }

	    ajListIterDel(&iter);			
	}

	/* Print out group-specific coordinates for this model */
	iter=ajListIterNewread(obj->Groups);

	while(!ajListIterDone(iter))
	{
	    tmp=(AjPAtom)ajListIterGet(iter);

	    if(tmp->Mod>x)
		break;
	    else if(tmp->Mod!=x)
		continue;
	    else	
	    {
		ajFmtPrintF(outf, "%-5s%-5d%-5c%-5d%-5c%-6S%-2c%-6S%-2c%-6S"
			    "%9.3f%9.3f%9.3f%8.2f%8.2f\n", 
			    "AT", 
			    tmp->Mod, 
			    '.', 
			    tmp->Gpn, 
			    '.',
			    tmp->Pdb,
			    tmp->Id1,
			    tmp->Id3,
			    tmp->Type, 
			    tmp->Atm, 
			    tmp->X, 
			    tmp->Y, 
			    tmp->Z,
			    tmp->O,
			    tmp->B);
	    }
	}

	ajListIterDel(&iter);			


	/* Print out water-specific coordinates for this model */
	iter=ajListIterNewread(obj->Water);

	while(!ajListIterDone(iter))
	{
	    tmp=(AjPAtom)ajListIterGet(iter);

	    if(tmp->Mod>x)
		break;
	    else if(tmp->Mod!=x)
		continue;
	    else	
	    {
		ajFmtPrintF(outf, "%-5s%-5d%-5c%-5c%-5c%-6S%-2c%-6S%-2c%-6S"
			    "%9.3f%9.3f%9.3f%8.2f%8.2f\n",
			    "AT", 
			    tmp->Mod, 
			    '.', 
			    '.', 
			    '.',
			    tmp->Pdb, 
			    tmp->Id1,
			    tmp->Id3,
			    tmp->Type, 
			    tmp->Atm, 
			    tmp->X, 
			    tmp->Y, 
			    tmp->Z,
			    tmp->O,
			    tmp->B);
	    }
	}

	ajListIterDel(&iter);			
    }

    ajFmtPrintF(outf, "//\n");    


    return ajTrue;
}




/* @func ajPdbWriteSegment *************************************************
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
** @@
** 
****************************************************************************/

AjBool ajPdbWriteSegment(AjPFile outf, const AjPPdb pdb, const AjPStr segment, 
			  char chnid, const AjPStr domain, AjPFile errf)
{
    ajint chn;
    ajint start     = 0;
    ajint end       = 0;
    char  id;
    
    AjIList    iter        = NULL;
    AjPAtom    atm         = NULL;
    AjPResidue res         = NULL;
    AjBool     found_start = ajFalse;
    AjBool     found_end   = ajFalse;    

    AjPSeqout outseq;
   
    /* Check for unknown or zero-length chain */
    if(!ajPdbChnidToNum(chnid, pdb, &chn))
    {
	ajWarn("Chain incompatibility error in "
	       "ajPbdWriteSegment");			
		
	ajFmtPrintF(errf, "//\n%S\nERROR Chain incompatibility "
		    "error in ajPbdWriteDomain\n", domain);

	return ajFalse;
    }
    else if(pdb->Chains[chn-1]->Nres==0)
    {		
	ajWarn("Chain length zero");			
	    
	ajFmtPrintF(errf, "//\n%S\nERROR Chain length zero\n", 
		    domain);

	return ajFalse;
    }
    
    /* Check if segment exists in this chain */
    if((start = ajStrFindS(pdb->Chains[chn-1]->Seq, segment)) == -1)
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

    if(pdb->Method == ajXRAY)
	ajFmtPrintF(outf, "xray; ");	
    else
	ajFmtPrintF(outf, "nmr_or_model; ");		
    /* The NCHN and NMOD are hard-coded to 1 for domain files */
    ajFmtPrintF(outf, "RESO %.2f; NMOD 1; NCHN 1; NGRP 0;\n", 
		pdb->Reso);
    

    id = pdb->Chains[chn-1]->Id;

    if(id == ' ')
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
    iter = ajListIterNewread(pdb->Chains[chn-1]->Atoms);
    


    /* Iterate through the list of residues */
    while((res=(AjPResidue)ajListIterGet(iter)))
    {
	if(res->Mod!=1)
	    break;

	if(!found_start)
	{
	    if(res->Idx == start)
		found_start = ajTrue;	
	    else		
		continue;
	}	

	if(!found_end)
	{
	    if(res->Idx == end)
		found_end = ajTrue;     
	}
	/*  The end position has been found, and the current residue is 
	 ** not the final residue.
	 */
	else if(res->Idx != end && found_end)
	    break;
	    
	    
	/* Print out coordinate line */
	ajFmtPrintF(outf, "%-5s%-5d%-5d%-5d%-6S%-2c%-6S", 
		    "RE", 
		    res->Mod, 
		    1, 		/* chn number is always given as 1 */
		    res->Idx-start+1, 
		    res->Pdb,
		    res->Id1, 
		    res->Id3);

	if(res->eNum != 0)
	    ajFmtPrintF(outf, "%-5d", res->eNum);
	else
	    ajFmtPrintF(outf, "%-5c", '.');

	ajFmtPrintF(outf, "%-5S%-5c", res->eId, res->eType);
	
	if(res->eType == 'H')
	    ajFmtPrintF(outf, "%-5d", res->eClass);
	else
	    ajFmtPrintF(outf, "%-5c", '.');
	
	if(res->eStrideNum != 0)
	    ajFmtPrintF(outf, "%-5d", res->eStrideNum);
	else
	    ajFmtPrintF(outf, "%-5c", '.');

	ajFmtPrintF(outf, "%-5c", res->eStrideType);
	
	
	ajFmtPrintF(outf, "%8.2f%8.2f%8.2f%8.2f%8.2f"
		    "%8.2f%8.2f%8.2f%8.2f%8.2f%8.2f%8.2f"
		    "%8.2f\n", 
		    res->Phi,
		    res->Psi,
		    res->Area, 
		    res->all_abs, 
		    res->all_rel, 
		    res->side_abs, 
		    res->side_rel, 
		    res->main_abs, 
		    res->main_rel, 
		    res->npol_abs, 
		    res->npol_rel, 
		    res->pol_abs, 
		    res->pol_rel);
	
		    
	/* Assign pointer for this chain */
	/* res2 = res;   but it's never used */
    }
    
    ajListIterDel(&iter);			
    

    
    /* Iterate through the list of atoms */
    while((atm=(AjPAtom)ajListIterGet(iter)))
    {
	if(atm->Mod!=1)
	    break;

	if(atm->Type!='P')
	    continue;

	if(!found_start)
	{
	    if(atm->Idx == start)
		found_start = ajTrue;	
	    else		
		continue;
	}	

	if(!found_end)
	{
	    if(atm->Idx == end)
		found_end = ajTrue;     
	}
	/*  The end position has been found, and the current atom no longer
	 ** belongs to this final residue.
	 */
	else if(atm->Idx != end && found_end)
	    break;
	    
	    
	/* Print out coordinate line */
	ajFmtPrintF(outf, "%-5s%-5d%-5d%-5c%-5d%-6S%-2c%-6S%-2c%-6S"
		    "%9.3f%9.3f%9.3f%8.2f%8.2f\n"
		    "AT", 
		    atm->Mod, /* It will always be 1 */
		    1, 	      /* chn number is always given as 1 */ 
		    '.', 
		    atm->Idx-start+1, 
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




/* @func ajHetWrite ********************************************************
**
** Writes the contents of a Het object to file. 
**
** @param [w] outf    [AjPFile]   Output file
** @param [r] obj     [const AjPHet]    Het object
** @param [r] dogrep  [AjBool]    Flag (True if we are to write 'cnt'
**                                element of the Het object to file).
**
** @return [AjBool] True on success
** @category output [AjPHet] Write Het object to file in clean
** @@
****************************************************************************/

AjBool ajHetWrite(AjPFile outf, const AjPHet obj, AjBool dogrep)
{
    ajint i = 0;
    
    /* Check arg's */
    if(!outf || !obj)
	return ajFalse;
    
    
    for(i=0;i<obj->n; i++)
    {
	ajFmtPrintF(outf, "ID   %S\n", obj->entries[i]->abv);
	ajFmtPrintSplit(outf, obj->entries[i]->ful, "DE   ", 70, " \t\n\r");
	ajFmtPrintSplit(outf, obj->entries[i]->syn, "SY   ", 70, " \t\n\r");

	if(dogrep)
	    ajFmtPrintF(outf, "NN   %d\n", obj->entries[i]->cnt);

	ajFmtPrintF(outf, "//\n");
    }

    return ajTrue;
}




/* @func ajPdbtospWrite ****************************************************
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
** @@
**
****************************************************************************/

AjBool   ajPdbtospWrite(AjPFile outf, const AjPList list)
{
    AjIList    iter = NULL;
    AjPPdbtosp tmp  = NULL;
    ajint        x  = 0;
    

    if(!outf || !list)
    {
	ajWarn("Bad args passed to ajPdbtospWrte");

	return ajFalse;
    }
    
    iter = ajListIterNewread(list);
    
    while((tmp=(AjPPdbtosp)ajListIterGet(iter)))
    {
	ajFmtPrintF(outf, "%-5s%S\nXX\n%-5s%d\nXX\n", 
		    "EN", tmp->Pdb, "NE", tmp->n);	

	for(x=0; x<tmp->n; x++)
	    ajFmtPrintF(outf, "%-5s%S ID; %S ACC;\n", 
			"IN", tmp->Spr[x], tmp->Acc[x]);

	ajFmtPrintF(outf, "XX\n//\n");
    }

    ajListIterDel(&iter);

    return ajTrue;
}




/* @func ajCmapWrite *****************************************************
**
** Write a Cmap object to file in CON format (see documentation for 
** DOMAINATRIX "contacts" application). 
** 
** @param [u] outf    [AjPFile]  Output file stream.
** @param [r] cmap    [const AjPCmap]  Cmap object pointer.
**
** @return [AjBool] True on success (object was written successfully)
** @category output [AjPCmap] Write Cmap object to file in CON format.
** @@
****************************************************************************/

AjBool   ajCmapWrite(AjPFile outf, const AjPCmap cmap)
{
    ajint x = 0;
    ajint y = 0;
    AjPStr Id=NULL;
    AjPStr Domid=NULL;
    AjPStr Ligid=NULL;
    AjPStr res1=NULL;
    AjPStr res2=NULL;
    AjPSeqout outseq;

    Id    = ajStrNew();
    Domid = ajStrNew();
    Ligid = ajStrNew();
    res1 = ajStrNew();
    res2 = ajStrNew();
    

    if(!cmap || !outf)
    {
	ajStrDel(&Id);
	ajStrDel(&Domid);
	ajStrDel(&Ligid);
	ajStrDel(&res1);
	ajStrDel(&res2);
	
	return ajFalse;
    }
    
    
    /* EN */
    ajFmtPrintF(outf, "%-5s[%d]\n", "EN", cmap->en);
    ajFmtPrintF(outf, "XX\n");  

    /* ID */
    if(MAJSTRGETLEN(cmap->Id))
	ajStrAssignS(&Id, cmap->Id);
    else
	ajStrAssignC(&Id, ".");

    if(MAJSTRGETLEN(cmap->Domid))
	ajStrAssignS(&Domid, cmap->Domid);
    else
	ajStrAssignC(&Domid, ".");

    if(MAJSTRGETLEN(cmap->Ligid))
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
    if((cmap->Type ==  ajINTRA)||(cmap->Type ==  ajLIGAND))
	ajFmtPrintF(outf, "%-5sMO .; CN1 %d; CN2 .; ID1 %c; ID2 .; "
		    "NRES1 %d; NRES2 .\n",
		    "CN", 
		    cmap->Chn1, 
		    cmap->Chid1, 
		    cmap->Nres1);
    else if(cmap->Type ==  ajINTER)
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
    if(MAJSTRGETLEN(cmap->Seq1))
    {
	outseq = ajSeqoutNewFile(outf);
	ajSeqoutDumpSwisslike(outseq, cmap->Seq1, "S1");
	ajSeqoutDel(&outseq);
	ajFmtPrintF(outf, "XX\n");	
    }
    

    /* S2 */
    if(cmap->Type ==  ajINTER)
    {
	if(MAJSTRGETLEN(cmap->Seq2))
	{
	    outseq = ajSeqoutNewFile(outf);
	    ajSeqoutDumpSwisslike(outseq, cmap->Seq2, "S2");
	    ajSeqoutDel(&outseq);
	    ajFmtPrintF(outf, "XX\n");	
	}
    }
    
    /* NC */
    if((cmap->Type ==  ajINTRA)||(cmap->Type ==  ajINTER))
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
    if(cmap->Type ==  ajINTRA)    
    {
	for(x=0; x<cmap->Nres1; x++)
	    for(y=x+1; y<cmap->Nres1; y++)
	    {	
		if((ajUint2dGet(cmap->Mat, x, y)==1))
		{
		    /* Assign residue id. */
		    if(!ajResidueToTriplet(ajStrGetCharPos(cmap->Seq1, x),
                                           &res1))
			ajFatal("Index out of range in ajCmapWrite");
		
		    if(!ajResidueToTriplet(ajStrGetCharPos(cmap->Seq1, y),
                                           &res2))
			ajFatal("Index out of range in ajCmapWrite");

		    /* Print out the contact. */
		    ajFmtPrintF(outf, "%-5s%S %d ; %S %d\n", "SM", res1, x+1, 
				res2, y+1);	
		}
	    }
    }
    else if(cmap->Type ==  ajINTER)    
    {
	for(x=0; x<cmap->Nres1; x++)
	    for(y=x+1; y<cmap->Nres2; y++)
	    {	
		if((ajUint2dGet(cmap->Mat, x, y)==1))
		{
		    /* Assign residue id. */
		    if(!ajResidueToTriplet(ajStrGetCharPos(cmap->Seq1, x),
                                           &res1))
			ajFatal("Index out of range in ajCmapWrite");
		
		    if(!ajResidueToTriplet(ajStrGetCharPos(cmap->Seq2, y),
                                           &res2))
			ajFatal("Index out of range in ajCmapWrite");

		    /* Print out the contact. */
		    ajFmtPrintF(outf, "%-5s%S %d ; %S %d\n", "SM", res1, x+1, 
				res2, y+1);	
		}
	    }
    }
    else if(cmap->Type ==  ajLIGAND)    
    {
	for(x=0; x<cmap->Nres1; x++)
	    if((ajUint2dGet(cmap->Mat, 0, x)==1))
	    {
		/* Assign residue id. */
		if(!ajResidueToTriplet(ajStrGetCharPos(cmap->Seq1, x), &res1))
		    ajFatal("Index out of range in ajCmapWrite");
		
		/* Print out the contact. */
		ajFmtPrintF(outf, "%-5s%S %d\n", "LI", res1, x+1);
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
** @@
******************************************************************************/

void ajPdbExit(void)
{
    ajStrDel(&cmapStrline);
    ajStrDel(&cmapStrtemp_id);
    ajStrDel(&cmapStrtemp_domid);
    ajStrDel(&cmapStrtemp_ligid);
    ajStrDel(&cmapStrtype);
    ajStrDel(&cmapStrdesc);
    ajStrDel(&cmapStrtmpstr);

    return;
}




/* ======================================================================= */
/* ======================== Miscellaneous =================================*/
/* ======================================================================= */




/* @section Miscellaneous ***************************************************
**
** These functions may have diverse functions that do not fit into the other
** categories. 
**
****************************************************************************/
