/* @Source acdrelations application
**
** Add relations: attribute to ACD files
** Any existing relations: attributes are replaced.
**
** @author: Copyright (C) Jon Ison (jison@ebi.ac.uk)
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
** 
*******************************************************************************
** 
**  ACDRELATIONS documentation
**  See http://wwww.emboss.org
**  
**  Please cite the authors and EMBOSS.
**
**  Email jison@ebi.ac.uk.
**  
**  acdrelations reads ACD files and writes exact copies but (depending on the
**  mode it is run in) with relations: attributes either modified (mode 1) or
**  added (mode 2).
**  Mode 1
**  relations: lines in this format:
**  relations: "/edam/data/0001773 Tool-specific parameter"
**  are replaced with this format:
**  relations: "EDAM:0001773 data Tool-specific parameter"
**
**
**  Mode 2
**  acdrelations reads ACD files and writes exact copies but with a relations:
**  attribute added to each data definition.  Any existing relations: attributes
**  are replaced.  Text for relations: is from the EDAM ontology.
**
**  acdrelations parameters:
**  knowntypes.standards      (EMBOSS data file)
**  edamtoacd.dat             (acdrelations data file)
**  ACD file input directory
**  ACD file output directory
**
**  knowntypes.standards and edamtoacd.dat define the relations: value.
**  Values given in knowntypes.standards take precedence over edamtoacd.dat
** 
**  knowntypes.standards is the standard EMBOSS data file of known types which is annotated with EDAM terms.
**  An excerpt:
**  aaindex_data                        | file           |  "EDAM: Amino acid index"                               | AAINDEX entry
**  aaindex_database                    | file           |  "EDAM: AAindex database"                               | AAINDEX database
**  abi_trace                           | file           |  "EDAM: ABI sequencing trace"                           | ABI sequencing trace
**
**  edamtoacd.dat is a data file defining terms from the EDAM ontology to use
**  as values for relations: attribute in ACD files, for different combinations
**  of ACD datatypes / attributes.
**  For example: 
**  #
**  align          | "EDAM: Sequence alignment data"                             |
**  align          | "EDAM: Nucleotide sequence alignment data"                  | type:"nucleotide"
**  align          | "EDAM: Protein sequence alignment data"                     | type:"protein"
**  align          | "EDAM: Multiple sequence alignment data"                    | minseqs:"3"
**  align          | "EDAM: Pairwise sequence alignment data"                    | minseqs:"2"; maxseqs:"2"
**  align          | "EDAM: Multiple protein sequence alignment data"            | type:"protein"; minseqs:"3"
**  align          | "EDAM: Multiple nucleotide sequence alignment data"         | type:"nucleotide"; minseqs:"3"
**  align          | "EDAM: Pairwise nucleotide sequence alignment data"         | type:"nucleotide"; minseqs:"2"; maxseqs:"2"
**  align          | "EDAM: Pairwise protein sequence alignment data"            | type:"protein"; minseqs:"2"; maxseqs:"2"
**  array          | "EDAM: Generic float array"                                 | 
**  #
**  arraychar      | "EDAM: Generic character array"                             | 
**  #
**  arrayword      | "EDAM: Generic string array"                                | 
**  #
**
** 
**  Meaning of columns is:
**  ACD datatype | Attributes that must be specified | Value of relation: attribute | Note
**
**  IMPORTANT
**  For acdrelations to work correctly, edamtoacd.dat must be correctly formatted and laid out:
**  1. Lines for a given datatype *must* be given in order of number of attributes in the third column (lowest number first)
**  2. The first line for each datatype gives the default value of the relations: attribute and should have no attributes defined.
**  3. The attribute:value pairs must be delimited by ';'.
** 
******************************************************************************/

#include "emboss.h"





/******************************************************************************
**
** DATA STRUCTURES
**
******************************************************************************/




/* @datastatic PEdamdat *******************************************************
**
** edamdat object
** Holds a single line from the data file edamtoacd.dat
**
** @alias SEdamdat
** @alias OEdamdat
**
** @attr acdtype [AjPStr]  ACD datatype, e.g. "align"
** @attr acdattr [AjPStr*] Array of ACD attribute:value strings e.g. minseqs:"3"
**                         Strings must contain no whitespace!
** @attr edam    [AjPStr]  Text to be given after relations: attribute,
**                         e.g. "EDAM: Multiple sequence alignment data"
**                         if all relations in acdattr are defined for acdtype.
** @attr n       [ajint]   Size of acdattr array
** @attr Padding [ajint] Padding to alignment boundaty
******************************************************************************/
typedef struct SEdamdat
{
  AjPStr  acdtype;
  AjPStr* acdattr;
  AjPStr  edam;   
  ajint   n;       
  ajint Padding;
} OEdamdat;
#define PEdamdat OEdamdat*




/* @datastatic PEdam **********************************************************
**
** edam object
** Holds all lines from the data file edamtoacd.dat
**
** @alias SEdam
** @alias OEdam
**
** @attr dat    [PEdamdat*] Array of PEdamdat objects 
** @attr n      [ajint]     Size of dat array
** @attr Padding [ajint] Padding to alignment boundaty
******************************************************************************/
typedef struct SEdam
{
  PEdamdat *dat; 
  ajint  n;      
  ajint Padding;
} OEdam;
#define PEdam OEdam*




/* @datastatic PKtypedat ******************************************************
**
** ktypedat object
** Holds a single line from the data file knowntypes.standard
**
** @alias SKtypedat
** @alias OKtypedat
**
** @attr acdtype [AjPStr]  ACD datatype, e.g. "align"
** @attr ktype   [AjPStr]  Value of knowntype: attribute: no whitespace!
** @attr edam    [AjPStr]  Text to be given after relations: attribute,
**                         e.g. "EDAM: Multiple sequence alignment data"
**                         if ktype is defined for acdtype.
******************************************************************************/ 
typedef struct SKtypedat
{
  AjPStr acdtype;
  AjPStr ktype;  
  AjPStr edam;   
} OKtypedat;
#define PKtypedat OKtypedat*




/* @datastatic PKtype *********************************************************
**
** ktype object
** Holds all lines from the data file knowntypes.standard
**
** @alias SKtype
** @alias OKtype
**
** @attr dat [PKtypedat*]  Array of PKtypedat objects  
** @attr n   [ajint]       Size of dat array
** @attr Padding [ajint] Padding to alignment boundaty
******************************************************************************/ 
typedef struct SKtype
{
  PKtypedat *dat;   
  ajint n;         
  ajint Padding;
} OKtype;
#define PKtype OKtype*





/******************************************************************************
**
** PROTOTYPES  
**
******************************************************************************/

static void acdrelations_writerelations
            (AjPFile  outf, 
	     AjPStr   acdtype,
	     AjPStr  *strarr, 
             ajint    n,
	     PEdam  P,
	     PKtype T);

static void acdrelations_readdatfile
            (AjPFile inf, 
	     PEdam *P);

static void acdrelations_readtypefile
            (AjPFile inf, 
	     PKtype *P);

static void acdrelations_procacdfile1
            (AjPFile inf, 
	     AjPFile outf);

static void acdrelations_procacdfile2
            (AjPFile inf, 
	     AjPFile outf, 
	     PEdam P,
             PKtype T);


static PEdamdat  acdrelations_EdamdatNew(void);
static PEdam     acdrelations_EdamNew(void);

static void      acdrelations_EdamdatDel(PEdamdat *P);
static void      acdrelations_EdamDel(PEdam *P);

static PKtypedat acdrelations_KtypedatNew(void);
static PKtype    acdrelations_KtypeNew(void);

static void      acdrelations_KtypedatDel(PKtypedat *P);
static void      acdrelations_KtypeDel(PKtype *P);




/* @prog acdrelations ********************************************************
**
** Add relations: attribute to ACD files. 
** 
*****************************************************************************/

int main(ajint argc, char **argv)
{
  /* Variable declarations */
  AjPFile   inf_edam   = NULL;  /* Name of EDAM data (input) file   */
  AjPFile   acdoutf    = NULL;  /* Name of ACD (output) file        */
  AjPStr    mode       = NULL;
    
  ajint     modei;
  AjPList   acdinlist  = NULL;  /* List of ACD file names (input)   */
  AjPFile   acdinf     = NULL;  /* Name of ACD (input) file         */
  AjPStr    acdname    = NULL;  /* Name of current acd file         */
  AjPDirout acdoutdir  = NULL;  /* Directory for ACD files (output) */

  AjPFile   inf_ktype  = NULL;  /* Name of knowntypes.standard file */
  
  PEdam   edam         = NULL;  /* EDAM relations data              */
  PKtype  ktype        = NULL;  /* Data from knowntype.standard     */


  
  /* Read data from acd. */
/*  embInitP("acdrelations",argc,argv,"MYEMBOSS"); */
  embInit("acdrelations",argc,argv);
    
  /* ACD data handling */
  mode = ajAcdGetListSingle("mode");
  inf_edam   = ajAcdGetDatafile("inedamfile");
  inf_ktype  = ajAcdGetInfile("intypefile");
  acdinlist  = ajAcdGetDirlist("indir");  
  acdoutdir  = ajAcdGetOutdir("outdir");

  ajStrToInt(mode, &modei);
    
  /* Read data file */  
  if(modei==2)
  {
      edam  = acdrelations_EdamNew();
      ktype = acdrelations_KtypeNew();
      acdrelations_readdatfile(inf_edam, &edam);
      acdrelations_readtypefile(inf_ktype, &ktype);
  }
  

  /*  Main application loop. Process each ACD file in turn.  */
  while(ajListPop(acdinlist,(void **)&acdname))
  {
      if(!(acdinf = ajFileNewInNameS(acdname)))   
          ajFatal("Cannot open input ACD file %S\n", acdname);
      
      ajFilenameTrimPath(&acdname);
            
      if(!(acdoutf = ajFileNewOutNameDirS(acdname, acdoutdir)))
          ajFatal("Cannot open output ACD file %S\n", acdname);

      if(modei==1)
          acdrelations_procacdfile1(acdinf, acdoutf);
      else if(modei==2)
          acdrelations_procacdfile2(acdinf, acdoutf, edam, ktype);
      else
          ajFatal("Unknown mode");
      
      ajFileClose(&acdinf);
      ajFileClose(&acdoutf);
  }
  
  /* Clean up and exit */
  ajStrDel(&mode);

  if(modei==2)
  {
      ajFileClose(&inf_edam);
      ajFileClose(&inf_ktype);
      acdrelations_EdamDel(&edam);
      acdrelations_KtypeDel(&ktype);
  }
  
  ajListFree(&acdinlist);
  ajDiroutDel(&acdoutdir);
  


  ajExit();
  return 0;
}




/******************************************************************************
**
** FUNCTIONS  
**
******************************************************************************/




/* @funcstatic acdrelations_readdatfile ***************************************
**
** Read the data file edamtoacd.dat and write edam object.
**
** @param [u] inf [AjPFile] Handle on edamtoacd.dat 
** @param [u] P   [PEdam*]  edam object to write
** @return [void] 
** @@
******************************************************************************/

static void acdrelations_readdatfile
            (AjPFile inf, 
	     PEdam *P)
{
  AjPStr  line           = NULL;
  const AjPStr  tok      = NULL;
  const AjPStr  subtok   = NULL;
  AjPStr  strtmp         = NULL;
  AjPList strlist        = NULL;

  AjPStr  acdtype        = NULL;
  AjPStr  relations      = NULL;

  PEdamdat dattmp        = NULL;
  AjPList  datlist       = NULL;
  
  if(!P)
    ajFatal("Null arg error 1 in acdrelations_readdatfile");
  if(!inf)
    ajFatal("Null arg error 3 in acdrelations_readdatfile");  


  /* Allocate memory */
  line      = ajStrNew();
  acdtype   = ajStrNew();
  relations = ajStrNew();
  datlist   = ajListNew();

  /* Read data from file */
  while(ajReadline(inf,&line))
    {
      /* Discard comment lines */
      if(ajStrPrefixC(line,"#")) 
	continue;

      
      
      /* Tokenise line, delimited by '|'.
         Parse first token (ACD datatype ) */
      ajStrAssignS(&acdtype, ajStrParseC(line, "|")); 
      
      /* Parse second token (EDAM relations: value ) */
      ajStrAssignS(&relations, ajStrParseC(NULL, "|")); 

      /* Parse third token (attribute:value strings block) */
      tok = ajStrParseC(NULL, "|");
        

      /* Create new string list */
      strlist = ajListstrNew();

      /* Tokenise third token itself into tokens delimited by ' ' (space)
         Parse tokens (individual attribute:value strings)*/
      if((subtok=ajStrParseC(tok, ";")))
      {
          strtmp = ajStrNew();
          ajStrAssignS(&strtmp, subtok);
          ajStrRemoveWhite(&strtmp);
          ajListstrPushAppend(strlist, strtmp);
              
          while((subtok=ajStrParseC(NULL, ";")))
          {
              strtmp = ajStrNew();
              ajStrAssignS(&strtmp, subtok);
              ajStrRemoveWhite(&strtmp);
              ajListstrPushAppend(strlist, strtmp);
          }
      }
      
      /* Write PEdamdat structure & push onto list */
      dattmp = acdrelations_EdamdatNew();
      ajStrRemoveWhite(&acdtype);
      ajStrAssignS(&dattmp->acdtype, acdtype);
      ajStrAssignS(&dattmp->edam, relations);
      dattmp->n = (ajuint) ajListstrToarray(strlist, &dattmp->acdattr);
      ajListPushAppend(datlist, dattmp);
      
      /* Clear nodes (but not strings) from string list */
      ajListstrFree(&strlist);
    }
  

  /* Write PEdam structure */
  ((*P)->n) = (ajuint) ajListToarray(datlist, (void***) &((*P)->dat));

  /* Free memory */
  ajStrDel(&line);
  ajStrDel(&acdtype);
  ajStrDel(&relations);
  ajListFree(&datlist);

  return;
}





/* @funcstatic acdrelations_readtypefile **************************************
**
** Read the data file knowntypes.standard and write ktype object
**
** @param [u] inf [AjPFile]  Handle on knowntypes.standard
** @param [u] T   [PKtype*]  ktype object to write
** @return [void] 
** @@
******************************************************************************/

static void acdrelations_readtypefile
            (AjPFile inf, 
	     PKtype *T)
{
    AjPStr     line    = NULL;
    PKtypedat  dattmp  = NULL;
    AjPList    datlist = NULL;
    
    if(!T)
        ajFatal("Null arg error 1 in acdrelations_readtypefile");
    if(!inf)
        ajFatal("Null arg error 3 in acdrelations_readtypefile");


    /* Allocate memory */
    line           = ajStrNew();
    datlist        = ajListNew();

    
    /* Read data from file */
    while(ajReadline(inf,&line))
    {
        /* Discard comment lines */
        if(ajStrPrefixC(line,"#")) 
            continue;

        /* Create object for holding line */
        dattmp = acdrelations_KtypedatNew();
        
        /* Tokenise line delimited by '|'
           Parse first token (value of knowntype: attribute) */
        ajStrAssignS(&dattmp->ktype, ajStrParseC(line, "|"));
        ajStrRemoveSetC(&dattmp->ktype, "_");
        ajStrRemoveWhite(&dattmp->ktype);
        
        /* Parse second token (ACD datatype) */
        ajStrAssignS(&dattmp->acdtype, ajStrParseC(NULL, "|"));

        /* Parse third token (EDAM relations: value ) */
        ajStrAssignS(&dattmp->edam, ajStrParseC(NULL, "|"));

        /* Push line onto list */
        ajListPushAppend(datlist, dattmp);
    }
    

    /* Write PKtype structure */
    ((*T)->n) = (ajuint) ajListToarray(datlist, (void***) &((*T)->dat));
  
    
    /* Free memory */
    ajStrDel(&line);
    ajListFree(&datlist);

    return;
}





/* @funcstatic acdrelations_procacdfile1 ***************************************
**
** Process ACD file and write new ACD file with relations: attributes in new
** format (mode '1' operation).
**
** @param [u] inf  [AjPFile] ACD input file
** @param [u] outf [AjPFile] ACD output file
** @return [void] 
** @@
******************************************************************************/
static void acdrelations_procacdfile1
            (AjPFile inf, 
	     AjPFile outf)
{
  AjPStr    line      = NULL;
  AjPStr    tmp       = NULL;
  AjPStr    tok       = NULL;
  AjPStr    id        = NULL;
  AjPStr    name      = NULL;
  AjPStr    namespace = NULL;
  AjPStrTok parse     = NULL;
  ajint     pos;
  
  
  /* Allocate memory */
  line        = ajStrNew();
  tmp         = ajStrNew();
  tok         = ajStrNew();
  id          = ajStrNew();
  name        = ajStrNew();
  namespace   = ajStrNew();

  
  /*  Read next line */
  while(ajReadline(inf,&line))
  {
      ajFmtScanS(line, "%S", &tok);
      
      /* Write reformatted relations: line */
      if(ajStrMatchC(tok, "relations:"))
      {
          /* Tokenise line segment (from after "relations:") by '"', ' ' and '/' 
             Discard unwanted tokens */
          pos = (ajuint) ajStrFindAnyK(line, ':');
          ajStrAssignC(&tmp, line->Ptr+pos);
          
          parse = ajStrTokenNewC(tmp, "\" /");
          ajStrTokenNextParse(&parse, &tok);
          ajStrTokenNextParse(&parse, &tok); 
          
          /* Get namespace and id tokens */
          ajStrTokenNextParse(&parse, &namespace);
          ajStrTokenNextParse(&parse, &id);           

          /* Get rest of string (name)
             Strip whitespace at start and trailing " from name. */
          ajStrTokenRestParse(&parse, &name);
          ajStrRemoveWhiteExcess(&name);
          ajStrRemoveSetC(&name, "\"\n");
          ajStrTokenDel(&parse);
                        
          /* Position of last '/' and last '"' characters respectively. */
          ajFmtPrintF(outf, "    relations: \"EDAM:%S %S %S\"\n", id, namespace, name);
      }

      /* Write other lines out as-is */
      else
          ajFmtPrintF(outf, "%S", line);          
  }


  /* Free memory */
  ajStrDel(&line);
  ajStrDel(&tmp);
  ajStrDel(&tok);
  ajStrDel(&id);
  ajStrDel(&name);
  ajStrDel(&namespace);

  return;
}




/* @funcstatic acdrelations_procacdfile2 ***************************************
**
** Process ACD file and write new ACD file with new relations: attributes
** added (replaced if necessary) using mode '2' operation.
**
** @param [u] inf  [AjPFile] ACD input file
** @param [u] outf [AjPFile] ACD output file
** @param [u] P    [PEdam]   edam object
** @param [u] T    [PKtype]  ktype object
** @return [void] 
** @@
******************************************************************************/
static void acdrelations_procacdfile2
            (AjPFile inf, 
	     AjPFile outf, 
	     PEdam P,
    	     PKtype T)
{
  AjPStr   line     = NULL;
  AjPStr   tok      = NULL;
  AjPStr   acdtype  = NULL;  
  AjPStr   strtmp   = NULL;
  AjPList  strlist  = NULL;
  AjPStr  *strarr   = NULL;
  ajint    nstr     = 0;
  

  /* Allocate memory */
  line        = ajStrNew();
  tok         = ajStrNew();
  acdtype     = ajStrNew();  
  strlist     = ajListstrNew();

  
  /*  Read next line */
  while(ajReadline(inf,&line))
    {
      ajFmtScanS(line, "%S", &tok);
      
      /* Write application definition or section definition out as-is */
      if(ajStrMatchC(tok, "application:")  ||
	 ajStrMatchC(tok, "section:"))
      {
	  ajFmtPrintF(outf, "%S", line);
	  while(ajReadline(inf,&line))
          {
	      ajFmtPrintF(outf, "%S", line);
	      ajFmtScanS(line, "%S", &tok);
	      if(ajStrMatchC(tok, "]"))
                  break;
          }
      }
      /* Write variables, endsection definitions and comments out as-is */
      else if(ajStrMatchC(tok, "variable:")   ||
	      ajStrMatchC(tok, "endsection:") ||
              ajStrMatchC(tok, "#"))
          ajFmtPrintF(outf, "%S", line);

      /* Write out blank lines as-is */
      else if (!ajFmtScanS(line, "%S", &tok))
          ajFmtPrintF(outf, "%S", line);
      /* Process data definition */
      else
          /* First line of data definition */
      {
          /* Process and write datatype line */
          ajFmtPrintF(outf, "%S", line);
          ajFmtScanS(line, "%S", &acdtype);
	  ajStrRemoveSetC(&acdtype, ":");

          /* Process subsequent (attribute) lines */
	  while(ajReadline(inf,&line))
	    {
	      strtmp = ajStrNew();
	      ajStrAssignS(&strtmp, line);
	      ajStrRemoveWhite(&strtmp);
	      ajListstrPushAppend(strlist, strtmp);

	      ajFmtScanS(line, "%S", &tok);

              /* Reached end of data definition */
              if(ajStrMatchC(tok, "]"))
              {
                  nstr = (ajuint) ajListstrToarray(strlist, &strarr);
                  
                  /* Write relations: line */
                  acdrelations_writerelations(outf, acdtype, strarr, nstr, P, T);

                  AJFREE(strarr);
                  ajListstrFreeData(&strlist);
		  strlist = ajListstrNew();
		  
		  ajFmtPrintF(outf, "%S", line);
		  break;
		}
              /* Ignore existing relations: lines */
              else if(ajStrMatchC(tok, "relations:"))
                  continue;
              
	      ajFmtPrintF(outf, "%S", line);
	    }
	}
    }


  /* Free memory */
  ajStrDel(&line);
  ajStrDel(&tok);
  ajStrDel(&acdtype);
  ajListstrFreeData(&strlist);

  return;
}





/* @funcstatic acdrelations_writerelations ************************************
**
** Writes relations: attribute for an ACD data definition
** The relations: values given in knowntypes.standard have highest precedence,
** then the values given in edamtoacd.dat
** Attribute values for a given datatype in edamtoacd.dat are in order of
** increasing precedence, i.e. the last line is highest and the relations:
** value will be used if all conditions are met.
**
** @param [u] outf    [AjPFile] ACD output file
** @param [u] acdtype [AjPStr] ACD datatype, e.g. "align"
** @param [u] strarr  [AjPStr*] All ACD attribute lines (whitespace removed)
**                              for the the current ACD data item (of type
**                              acdtype).  One line per array element.
** @param [r] n       [ajint]   Size of strarr
** @param [u] P       [PEdam]   edam object to write
** @param [u] T       [PKtype]  ktype object to read
** @return [void] 
** @@
******************************************************************************/
static void acdrelations_writerelations
            (AjPFile outf, 
	     AjPStr  acdtype,
	     AjPStr *strarr, 
             ajint   n,
	     PEdam   P,
    	     PKtype  T)
{
    ajint  i         = 0;
    ajint  j         = 0;
    ajint  k         = 0;
    ajint  nmatch    = 0;
    AjPStr relations = NULL;
    AjPStr ktype     = NULL;     /* Value of knowntype: attribute */
    AjBool done      = ajFalse;
    AjBool donetype  = ajFalse;
    AjPStr tmpstr    = NULL;
    
    
    if(!outf || !acdtype || !strarr || !n || !P)
        ajFatal("NULL args passed to acdrelations_writerelations");

    /* Memory allocation */
    relations = ajStrNew();
    ktype     = ajStrNew();
    tmpstr    = ajStrNew();

    
    /* Loop through all lines in edamtoacd.dat */
    for(i=0; i<P->n ;i++)
    {
        /* Found matching datatype */
        if(ajStrMatchS(acdtype, P->dat[i]->acdtype))
        {
            /* Copy first relations: string defined for this datatype (default) */
            ajStrAssignS(&relations, P->dat[i]->edam);
            done = ajTrue;
            i++;

            /* Check next line in edamtoacd.dat */
            for( ; i<P->n; i++)
            {
                /* Datatype still matches */
                if(ajStrMatchS(acdtype, P->dat[i]->acdtype))
                {
                    /* Loop through all required attributes for this datatype */
                    for(nmatch=0, j=0; j<P->dat[i]->n; j++)
                    {
                        /* Loop through all attribute lines for the data defininition */
                        for(k=0; k<n; k++)
                            if(ajStrMatchS(P->dat[i]->acdattr[j], strarr[k]))
                            {
                                nmatch++;
                                break;
                            }
                    }
                    /* All attribute values match */
                    if(nmatch == P->dat[i]->n)
                        ajStrAssignS(&relations, P->dat[i]->edam);
                    /* Should never happen */
                    else if (nmatch > P->dat[i]->n)
                        ajFatal("Terminal weirdness in acdrelations_writerelations");
                }
                else
                    break;
            }
            break;            
        }
    }

    /* Check for match of knowntype: attribute against knowntypes.standard.
       These have higher precedence than the rules defined in edamtoacd.dat */
    for(donetype=ajFalse, i=0; i<n; i++)
    {
        if(ajStrPrefixC(strarr[i], "knowntype:"))
        {
            
            for(j=0;j<T->n; j++)
            {
                /* No check is made on the "Type" column in knowntypes.standard
                   as these are not proper ACD datatype names
                   To check these add
                   if(ajStrMatchS(acdtype, T->dat[j]->acdtype)) */
                
                ajFmtPrintS(&tmpstr, "knowntype:\"%S\"", T->dat[j]->ktype);

                    if(ajStrMatchS(tmpstr, strarr[i]))
                    {
                        ajStrAssignS(&relations, T->dat[j]->edam);
                        donetype=ajTrue;
                        break;
                    }
            }
            if(donetype)
                break;
        }
        else
            continue;
    }
    
    
    if(!done)
        ajFatal("No matching datatype (%S) in acdrelations_writerelations", acdtype);
    
    
    /* Write relations: attribute line to file */
    ajFmtPrintF(outf, "    relations:%S\n", relations);


    /* Free memory */
    ajStrDel(&relations);
    ajStrDel(&ktype);
    ajStrDel(&tmpstr);
    
    return;
}





/* @funcstatic acdrelations_EdamdatNew ****************************************
**
** edamdat constructor
** The array is NOT allocated 
**
** @return [PEdamdat] New object
** @@
******************************************************************************/
static PEdamdat  acdrelations_EdamdatNew(void)
{
  PEdamdat ret;

  AJNEW0(ret);
  ret->acdtype  = ajStrNew();
  ret->edam     = ajStrNew();
  ret->acdattr  = NULL;
  ret->n        = 0;

  return ret;
}





/* @funcstatic acdrelations_EdamNew *******************************************
**
** edam constructor
** The array is NOT allocated 
**
** @return [PEdam] New object
** @@
******************************************************************************/
static PEdam     acdrelations_EdamNew(void)
{
  PEdam ret;

  AJNEW0(ret);
  
  ret->dat = NULL;
  ret->n = 0;

  return ret;
}





/* @funcstatic acdrelations_EdamdatDel ****************************************
**
** edamdat destructor
**
** @param [d] P       [PEdamdat*]  edamdat object to delete
** @return [void] 
** @@
******************************************************************************/
static void        acdrelations_EdamdatDel(PEdamdat *P)
{
  int i;

  if(!P)
    ajFatal("Null arg error 1 in acdrelations_EdamdatDel");
  else if(!(*P))
    ajFatal("Null arg error 2 in acdrelations_EdamdatDel");

  ajStrDel(&(*P)->acdtype);
  ajStrDel(&(*P)->edam);
  
  if((*P)->n)
    {
        for(i=0;i<(*P)->n;i++)
            ajStrDel(&(*P)->acdattr[i]);
        AJFREE((*P)->acdattr);
    }

  AJFREE(*P);
  *P=NULL;

  return;
}





/* @funcstatic acdrelations_EdamDel *******************************************
**
** edam destructor
**
** @param [d] P       [PEdam*]  edam object to delete
** @return [void] 
** @@
******************************************************************************/
static void        acdrelations_EdamDel(PEdam *P)
{
  int i;

  if(!P)
    ajFatal("Null arg error 1 in acdrelations_EdamDel");
  else if(!(*P))
    ajFatal("Null arg error 2 in acdrelations_EdamDel");

  if((*P)->n)
    {
        for(i=0;i<(*P)->n;i++)
            acdrelations_EdamdatDel(&(*P)->dat[i]);
        AJFREE((*P)->dat);
    }

  AJFREE(*P);
  *P=NULL;

  return;
}





/* @funcstatic acdrelations_KtypedatNew ***************************************
**
** ktypedat constructor
**
** @return [PKtypedat] New object
** @@
******************************************************************************/

static PKtypedat  acdrelations_KtypedatNew(void)
{
  PKtypedat ret;

  AJNEW0(ret);
  ret->acdtype = ajStrNew();
  ret->edam    = ajStrNew();
  ret->ktype   = ajStrNew();

  return ret;
}





/* @funcstatic acdrelations_KtypeNew ******************************************
**
** ktype constructor
**
** @return [PKtype] New object
** @@
******************************************************************************/
static PKtype     acdrelations_KtypeNew(void)
{
  PKtype ret;

  AJNEW0(ret);
  
  ret->dat = NULL;
  ret->n = 0;

  return ret;
}





/* @funcstatic acdrelations_KtypedatDel ***************************************
**
** ktypedat destructor
**
** @param [d] P       [PKtypedat*] ktypedat object to delete
** @return [void] 
** @@
******************************************************************************/
static void        acdrelations_KtypedatDel(PKtypedat *P)
{
  if(!P)
    ajFatal("Null arg error 1 in acdrelations_KtypedatDel");
  else if(!(*P))
    ajFatal("Null arg error 2 in acdrelations_KtypedatDel");

  ajStrDel(&(*P)->acdtype);
  ajStrDel(&(*P)->edam);
  ajStrDel(&(*P)->ktype);

  AJFREE(*P);
  *P=NULL;

  return;
}





/* @funcstatic acdrelations_KtypeDel ******************************************
**
** ktype destructor
**
** @param [d] P       [PKtype*] ktype object to delete
** @return [void] 
** @@
******************************************************************************/
static void        acdrelations_KtypeDel(PKtype *P)
{
  int i;

  if(!P)
    ajFatal("Null arg error 1 in acdrelations_KtypeDel");
  else if(!(*P))
    ajFatal("Null arg error 2 in acdrelations_KtypeDel");

  if((*P)->n)
    {
        for(i=0;i<(*P)->n;i++)
            acdrelations_KtypedatDel(&(*P)->dat[i]);
        AJFREE((*P)->dat);
    }

  AJFREE(*P);
  *P=NULL;

  return;
}
