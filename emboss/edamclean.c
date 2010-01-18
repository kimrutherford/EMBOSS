 /* @Source edamclean application
 **
 ** Validate and fix EDAM OBO ontology
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
 **  EDAMCLEAN documentation
 **  See http://wwww.emboss.org
 **  
 **  Please cite the authors and EMBOSS.
 **
 **  Email jison@ebi.ac.uk.
 **
 **
 **  edamclean reads EDAM (OBO format file), validates the file syntax, writes a
 **  report from parsing and (optionally) fixes the term numbering and relations.
 **  It has optional modes of operation:
 **  1. Report only
 **  2. Renumber terms
 **  3. Fix relations
 **
 **  1. Report only
 **  Write an informative report from parsing but do not change the file.
 **  The following checks are performed:
 **  i.    All ids in the file are unique
 **  ii.   All term names within a namespace are unique
 **  iii.  All values after namespace: are valid (see below).
 **  iv.   All field names are valid; either a standard OBO field, a relation or a token that must be ignored (see below).
 **  v.    All terms have the following fields in the order specified (optional fields are in parenthesis):
           id, name, namespace, def, (comment), (synonym), is_a
 **  vi.   Terms in specific namespaces have all mandatory relations defined and do not have disallowed relations. See "Rules" below.
 **  vii.  End-points (term names) of all relations exist. See "Rules" below.
 **  viii. All id: lines have the format:   id: EDAM:0000000 
 **  ix.   All def: lines have the format:  def: "Some text." [EDAM:EBI "EMBRACE definition"]
 **  x.    All relation lines have the format:  RelationName: EDAM:0000000 ! Term name
 **  xi.   All comment: values are *not* in quotes ("")
 **  xii.  All synonym: values are *not* in quotes ("")
 **
 **  2. Renumber terms
 **  Write a report as above.
 **  Renumber all terms so that they have unique ids, starting with EDAM:0000000 
 **  for the first term in the file and increasing by 1 thereon.
 **
 **  3. Fix relations
 **  Write a report as above.  If no errors reported, correct term ids used in all relations fields.  
 ** 
 **
 **  edamclean parameters:
 **  Name of OBO format file (input)
 **  Name of OBO format file (output)
 **  Name of report file (output)
 **  Boolean (whether to fix the output file)
 ** 
 **
 ** Standard OBO fields
 **   id:
 **   name:
 **   namespace:
 **   def:
 **   comment:
 **   synonym:
 **
 ** Relations
 **   is_a
 **   has_part
 **   is_part_of
 **   concerns
 **   is_concern_of
 **   has_input
 **   is_input_of
 **   has_output
 **   is_output_of
 **   has_source
 **   is_source_of
 **   has_identifier
 **   is_identifier_of
 **   has_attribute
 **   is_attribute_of
 **
 ** Namespace
 **   edam_term
 **   edam_entity
 **   edam_field
 **   edam_function
 **   edam_resource
 **   edam_datatype
 **   edam_identifier 
 **
 **
 ** Tokens to ignore
 ** Lines beginning with the following tokens are not parsed and are preserved as-is in the output:
 **   !
 **   format-version
 **   date
 **   data-version
 **
 ** 
 ** Rules
 ** Rules for which term types (rules in a namespace) may or must be related to which other term types are described under "Rules by term type" in the EDAM on-line documentation.  See http://www.ebi.ac.uk/~jison/edam.html#6.1.
 ** 
 ** 
 ** Notes
 ** Typedef definitions are *not* validated and are preserved as-is in the output.
 **
 ** Important!
 ** 1. The program should not be run in modes 2 or 3 (ie. generate an EDAM output file) until all  reported problems (from mode 1) have been fixed by hand - *except* "Non-unique id" errors!  Results are undefined otherwise.
 ** 2. All [Term] definitions in the input file *must* appear before the first [Typedef] definition - terms appearing after are *not* validated fully. 
 ******************************************************************************/

 #include "emboss.h"





 /******************************************************************************
 **
 ** GLOBAL VARIABLES
 **
 ******************************************************************************/
 #define NFIELDS 6
 char *FIELDS[NFIELDS] =
 {
     "id:", 
     "name:", 
     "namespace:", 
     "def:", 
     "comment:", 
     "synonym:"
 };


 #define NRELATIONS 15
 char *RELATIONS[NRELATIONS] =
 {
     "is_a:",  
     "has_part:",
     "is_part_of:",
     "concerns:",
     "is_concern_of:",
     "has_input:",
     "is_input_of:",
     "has_output:",
     "is_output_of:",
     "has_source:",
     "is_source_of:",
     "has_identifier:",
     "is_identifier_of:",
     "has_attribute:",
     "is_attribute_of:"
 };

 #define NNAMESPACES 7
 char *NAMESPACES[NNAMESPACES] =
 {
     "edam_term", 
     "edam_entity", 
     "edam_field", 
     "edam_function", 
     "edam_resource", 
     "edam_datatype", 
     "edam_identifier"
 };

 enum _namespace
 {
     edam_term, 
     edam_entity, 
     edam_field, 
     edam_function, 
     edam_resource, 
     edam_datatype, 
     edam_identifier
 };

 #define NOTHER 12
 char *OTHER[NOTHER] =
 {
     "!", 
     "format-version:", 
     "date:", 
     "data-version:",
     "subsetdef:",
     "[Term]",
     "[Typedef]",
     "inverse_of:", 
     "is_anti_symmetric:", 
     "is_cyclic:", 
     "is_transitive:",
     "\0"   /* NULL string to allow empty lines */
 };





 /******************************************************************************
 **
 ** DATA STRUCTURES
 **
 ******************************************************************************/

 /* @datastatic PTerm *******************************************************
 **
 ** Term object
 ** Holds name and identifier of a single EDAM term
 **
 ** @alias STerm
 ** @alias OTerm
 **
 ** @attr name [AjPStr]  Name of term
 ** @attr id   [AjPStr]  Id of term
 ** @attr line [ajint]   Line number of name: field for the term
 ******************************************************************************/
 typedef struct STerm
 {
     AjPStr  name;
     AjPStr  id;
     ajint   line;
 } OTerm;
 #define PTerm OTerm*



 /* @datastatic PNamespace *****************************************************
 **
 ** Namespace object
 ** Holds name and array of terms for an EDAM namespace.
 ** Only one copy of the terms is kept in memory (list holds pointers only)
 ** 
 ** @alias SNamespace
 ** @alias ONamespace
 **
 ** @attr name  [AjPStr]     Name of namespace
 ** @attr terms [PTerm*]  Array of terms
 ** @attr list  [AjPList]    List of terms
 ** @attr n     [ajint]      Size of array / list
 ******************************************************************************/
 typedef struct SNamespace
 {
     AjPStr   name;
     PTerm *terms;
     AjPList  list;
     ajint    n;
 } ONamespace;
 #define PNamespace ONamespace*




 /******************************************************************************
 **
 ** PROTOTYPES  
 **
 ******************************************************************************/
 PTerm       ajTermNew(void);
 PNamespace  ajNamespaceNew(void);
 void        ajTermDel(PTerm *P);
 void        ajNamespaceDel(PNamespace *P);
 AjPStr      FindTerm(ajint namespace, AjPStr termname, PNamespace *namespaces);




 /******************************************************************************
 **
 ** FUNCTIONS  
 **
 ******************************************************************************/


 /* @funcstatic ajTermNew ***************************************************
 **
 ** Term constructor
 **
 ** @return [PTerm] New object
 ** @@
 ******************************************************************************/
 PTerm  ajTermNew(void)
 {
   PTerm ret;

   AJNEW0(ret);
   ret->name  = ajStrNew();
   ret->id    = ajStrNew();
   ret->line  = 0;

   return ret;
 }

 /* @funcstatic ajNamespaceNew ***************************************************
 **
 ** Namespace constructor
 ** The array is *not* allocated.
 ** 
 ** @return [PNamespace] New object
 ** @@
 ******************************************************************************/
 PNamespace  ajNamespaceNew(void)
 {
   PNamespace ret;

   AJNEW0(ret);
   ret->name  = ajStrNew();
   ret->terms = NULL;
   ret->list  = ajListstrNew();
   ret->n     = 0;

   return ret;
 }


 /* @funcstatic ajTermDel ***************************************************
 **
 ** Term destructor
 **
 ** @param [r] P       [PTerm*]  Term object to delete
 ** @return [void] 
 ** @@
 ******************************************************************************/
 void ajTermDel(PTerm *P)
 {
   if(!P)
     ajFatal("Null arg error 1 in ajTermDel");
   else if(!(*P))
     ajFatal("Null arg error 2 in ajTermDel");

   ajStrDel(&(*P)->name);
   ajStrDel(&(*P)->id);

   AJFREE(*P);
   *P=NULL;

   return;
 }

 /* @funcstatic ajNamespaceDel ***************************************************
 **
 ** Namespace destructor
 **
 ** @param [r] P       [PNamespace*]  Namespace object to delete
 ** @return [void] 
 ** @@
 ******************************************************************************/
 void ajNamespaceDel(PNamespace *P)
 {
   int i;

   if(!P)
     ajFatal("Null arg error 1 in ajNamespaceDel");
   else if(!(*P))
     ajFatal("Null arg error 2 in ajNamespaceDel");

   ajStrDel(&(*P)->name);

   if((*P)->n)
     {
         for(i=0;i<(*P)->n;i++)
             ajTermDel(&(*P)->terms[i]);
         AJFREE((*P)->terms);
     }
   ajListstrFree(&(*P)->list);

   AJFREE(*P);
   *P=NULL;

   return;
 }


 /* @funcstatic FindTerm ***************************************************
 **
 ** Finds a term within a namespace and returns its index in the namespace array.
 ** Returns NULL if term is not found
 ** 
 ** @param [r] namespace    [ajint]  Namespace as integer
 ** @param [r] termname     [AjPStr] Name of term
 ** @param [r] namespaces   [AjPStr] Array of namespace objects
 ** @return [AjPStr] 
 ** @@
 ******************************************************************************/
AjPStr       FindTerm(ajint namespace, AjPStr termname, PNamespace *namespaces)
 {
     ajint x;

     if(!termname || !namespaces)
         ajFatal("Bad args to FindTerm");

     for(x=0; x<namespaces[namespace]->n; x++)
         if(ajStrMatchS(termname, namespaces[namespace]->terms[x]->name))
             return namespaces[namespace]->terms[x]->id;
     return NULL;
 }


 /* @prog edamclean ********************************************************
 **
 ** Validate and fix EDAM OBO ontology
 ** 
 *****************************************************************************/

 int main(ajint argc, char **argv)
 {
   /* Variable declarations */
   AjPFile    inf_edam        = NULL;    /* Name of EDAM (input) file      */
   AjPFile    ouf_edam        = NULL;    /* Name of EDAM (output) file     */
   AjPFile    ouf_log         = NULL;    /* Name of report (output) file   */
   AjPStr     mode            = NULL;    /* Mode of operation */

   AjPList    list_tmp        = NULL;    /* Temporary list                          */
   AjPStr    *fields          = NULL;    /* Array of valid tokens for first word in line */
   ajint      nfields         = 0;       /* Size of fields array                       */
   AjPStr    *ids             = NULL;    /* Array of all ids in file */  
   ajint      nids            = 0;       /* Size of ids */
   AjPStr     id              = NULL;    /* ID of a term */

   AjPStr     line            = NULL;    /* A line from the input file                   */
   ajint      linecnt         = 0;       /* Line number of line                          */
   ajint      termcnt         = 0;       /* Count of term definitions                    */
   AjPStr     tok             = NULL;    /* A token from line                            */
   AjBool     done            = ajFalse; /* Housekeeping  */
   ajint      x               = 0;       /* Housekeeping  */
   ajint      y               = 0;       /* Housekeeping  */
   ajint      z               = 0;       /* Housekeeping  */
   ajint      idx             = 0;       /* Housekeeping  */
   AjPStr     name            = NULL;    /* Name of a term */
   AjPStr     namespace       = NULL;    /* Namespace of a term */
   AjPStr     relation        = NULL;    /* Relationship name, e.g. "is_a" */
   AjPStr     tmp_name        = NULL;    /* Temp. name of a term */
   AjPStr     tmp_id          = NULL;    /* Temp. id of a term */
   AjPStr     tmp_str         = NULL;    /* Temp. string */
   PTerm      tmp_term        = NULL;    /* Temp. term pointer */
   ajint      tmp_line        = 0;       /* Temp. line number */  
   PNamespace namespaces[NNAMESPACES];   /* Array of namespace objects */

   AjBool    done_first      = ajFalse;  /* Housekeeping ... read first term */
   AjBool    first           = ajFalse;  /* Housekeeping ... on first term */
   AjBool    found_id               = ajFalse;
   AjBool    found_typedef        = ajFalse;  /* Found first [Typedef] line */
      
   AjBool    found_name             = ajFalse;
   AjBool    found_namespace        = ajFalse;
   AjBool    found_def              = ajFalse;
   AjBool    found_comment          = ajFalse;
   AjBool    found_synonym          = ajFalse;
   AjBool    found_isa              = ajFalse;
   AjBool    found_concerns         = ajFalse;
   AjBool    found_is_concern_of    = ajFalse;
   AjBool    found_has_input        = ajFalse;
   AjBool    found_is_input_of      = ajFalse;
   AjBool    found_has_output       = ajFalse;
   AjBool    found_is_output_of     = ajFalse;
   AjBool    found_has_source       = ajFalse;
   AjBool    found_is_source_of     = ajFalse;
   AjBool    found_has_identifier   = ajFalse;
   AjBool    found_is_identifier_of = ajFalse;
   AjBool    found_has_attribute    = ajFalse;
   AjBool    found_is_attribute_of  = ajFalse;
   AjBool    found_has_part         = ajFalse;
   AjBool    found_is_part_of       = ajFalse;







   /* Read data from acd */
   embInit("edamclean", argc, argv);


   /* ACD data handling */
   inf_edam  = ajAcdGetInfile("edamin");
   ouf_edam  = ajAcdGetOutfile("edamout");
   ouf_log   = ajAcdGetOutfile("log");
   mode      = ajAcdGetSelectSingle("mode");

   ajFmtPrint("MODE : %S\n", mode);
   
   

   /* Memory allocation */
   line       = ajStrNew();
   tok        = ajStrNew();
   name       = ajStrNew();
   namespace  = ajStrNew();
   relation   = ajStrNew();
   tmp_name = ajStrNew();
   tmp_id   = ajStrNew();

   for(x=0; x<NNAMESPACES; x++)
   {
       namespaces[x] = ajNamespaceNew();
       ajStrAssignC(&(namespaces[x]->name), NAMESPACES[x]);
   }



   /*  Check for valid first tokens */      
   /* First, write array of valid tokens for first word in line */
   list_tmp = ajListstrNew();
   for(x=0; x<NFIELDS; x++)
   {
       tmp_str = ajStrNew();
       ajStrAssignC(&tmp_str, FIELDS[x]);
       ajListstrPushAppend(list_tmp, tmp_str);
   }

   for(x=0; x<NRELATIONS; x++)
   {
       tmp_str = ajStrNew();
       ajStrAssignC(&tmp_str, RELATIONS[x]);
       ajListstrPushAppend(list_tmp, tmp_str);
   }

   for(x=0; x<NOTHER; x++)
   {
       tmp_str = ajStrNew();
       ajStrAssignC(&tmp_str, OTHER[x]);
       ajListstrPushAppend(list_tmp, tmp_str);
   }

   nfields = ajListstrToarray(list_tmp, &fields);
   ajListstrFree(&list_tmp);  

   ajFmtPrintF(ouf_log, "1. FIRST TOKEN IN LINES\n");          
   for(linecnt=0; ajReadline(inf_edam, &line); linecnt++)
   {
       ajStrAssignClear(&tok); 
       ajFmtScanS(line, "%S", &tok);
       for(x=0, done=ajFalse;x<nfields;x++)
           if(ajStrMatchS(tok, fields[x]))
           {
               done = ajTrue;
               break;
           }
       if(!done)
           ajFmtPrintF(ouf_log, "Line %6d : Invalid 1st token: %S (%S)\n", linecnt+1, tok, line);          
   }
   ajFmtPrintF(ouf_log, "\n\n");
   ajFileSeek(inf_edam, 0, 0);    /* Rewind file */



   /*  Check for valid namespace: values */
   ajFmtPrintF(ouf_log, "2. NAMESPACE VALUES\n");          
   for(linecnt=0; ajReadline(inf_edam, &line); linecnt++)
   {
       if(ajStrPrefixC(line, "namespace:"))
       {
           ajStrAssignClear(&tok);
           ajFmtScanS(line, "%*s %S", &tok);

           for(x=0, done=ajFalse;x<NNAMESPACES;x++)
               if(ajStrMatchC(tok, NAMESPACES[x]))
               {
                   done = ajTrue;
                   break;
               }
           if(!done)
               ajFmtPrintF(ouf_log, "Line %6d : Invalid namespace: %S (%S)\n", linecnt+1, tok, line);
       }

   }
   ajFmtPrintF(ouf_log, "\n\n");
   ajFileSeek(inf_edam, 0, 0);    /* Rewind file */



   /*  Check for valid comment: and synonym: values */
   ajFmtPrintF(ouf_log, "3. COMMENT / SYNONYM VALUES\n");          
   for(linecnt=0; ajReadline(inf_edam, &line); linecnt++)
   {
       if(ajStrPrefixC(line, "comment:") || ajStrPrefixC(line, "synonym:"))
           if(ajStrFindAnyK(line, '\"') != -1)
               ajFmtPrintF(ouf_log, "Line %6d : Invalid quote in line (%S)\n", linecnt+1, line);          
   }
   ajFmtPrintF(ouf_log, "\n\n");
   ajFileSeek(inf_edam, 0, 0);    /* Rewind file */



   /*  Check for id: line format (also build list of term ids) */
   list_tmp = ajListstrNew();
   ajFmtPrintF(ouf_log, "4. id: LINE FORMAT\n");          
   for(linecnt=0; ajReadline(inf_edam, &line); linecnt++)
   {
       /* Stop checking once first [Typedef] line is found. */ 
       if(ajStrPrefixC(line, "[Typedef]"))
           break;
       
       if(ajStrPrefixC(line, "id:"))
       {
           if(ajStrCalcCountC(line, ":")!=2)
               ajFmtPrintF(ouf_log, "Line %6d : Invalid id: line format - wrong number of colon (:) (%S) \n", linecnt+1, line);
           else
           {
               ajStrParseC(line, ":");
               ajStrAssignS(&tok, ajStrParseC(NULL, ":"));
               ajStrRemoveWhite(&tok);
               if(!ajStrMatchC(tok, "EDAM"))
                   ajFmtPrintF(ouf_log, "Line %6d : Invalid id: line format - no 'EDAM' token (%S)\n", linecnt+1, line);          
               ajStrAssignS(&tok, ajStrParseC(NULL, ":"));
               ajStrRemoveWhite(&tok);
               if(ajStrGetLen(tok) != 7)
                   ajFmtPrintF(ouf_log, "Line %6d : Invalid id: line format - id number wrong (%S)\n", linecnt+1, line);          
               ajListstrPushAppend(list_tmp, tok);
           }
       }
   }
   nids = ajListstrToarray(list_tmp, &ids);
   ajListstrFree(&list_tmp);
   ajFmtPrintF(ouf_log, "\n\n");
   ajFileSeek(inf_edam, 0, 0);    /* Rewind file */



   /*  Check for def: line format */
   ajFmtPrintF(ouf_log, "5. def: LINE FORMAT\n");          
   for(linecnt=0; ajReadline(inf_edam, &line); linecnt++)
   {
       /* Stop checking once first [Typedef] line is found. */ 
       if(ajStrPrefixC(line, "[Typedef]"))
           break;

       if(ajStrPrefixC(line, "def:"))
       {
           if(ajStrCalcCountC(line, ":[")!=3)
               ajFmtPrintF(ouf_log, "Line %6d : Invalid def: line format - wrong number of colon ':' or open-bracket '[' (%S)\n", linecnt+1, line);
           else
           {
               ajStrParseC(line, ":[");
               ajStrAssignS(&tok, ajStrParseC(NULL, ":["));
               /* Check for 2 double quotes */
               if(ajStrCalcCountC(tok, "\"")!=2)
                   ajFmtPrintF(ouf_log, "Line %6d : Invalid def: line format - wrong number of double quotes (%S)\n", linecnt+1, line);          
               /* Careful - different tokeniser used (no colon) ! */ 
               ajStrAssignClear(&tok);
               ajStrAssignS(&tok, ajStrParseC(NULL, "["));   
               ajStrRemoveLastNewline(&tok);
               ajStrRemoveWhiteExcess(&tok);
                           
               /* Check for line suffix */
               if(!ajStrMatchC(tok, "EDAM:EBI \"EMBRACE definition\"]"))
                   ajFmtPrintF(ouf_log, "Line %6d : Invalid def: line format - invalid suffix (%S)\n", linecnt+1, line);
           }
       }
   }
   ajFmtPrintF(ouf_log, "\n\n");
   ajFileSeek(inf_edam, 0, 0);    /* Rewind file */



     /*  Check for relations line format */
   ajFmtPrintF(ouf_log, "6. RELATIONS LINE FORMAT\n");          
   for(linecnt=0; ajReadline(inf_edam, &line); linecnt++)
   {
       for(x=0; x<NRELATIONS; x++)
           if(ajStrPrefixC(line, RELATIONS[x]))
           {
             if(ajStrCalcCountC(line, "!:")!=3)
                 ajFmtPrintF(ouf_log, "Line %6d : Invalid relations line format (%S)\n", linecnt+1, line);
             else
             {

                 ajStrParseC(line, ":!");
                 ajStrAssignS(&tok, ajStrParseC(NULL, ":!"));
                 ajStrRemoveWhite(&tok);
                 /* Check for "EDAM" */
                 if(!ajStrMatchC(tok, "EDAM"))
                     ajFmtPrintF(ouf_log, "Line %6d : Invalid relations line format (%S)\n", linecnt+1, line);
                 /* Check for 7 digit number */
                 ajStrAssignS(&tok, ajStrParseC(NULL, ":!"));
                 ajStrRemoveWhite(&tok);
                 if(ajStrGetLen(tok) != 7)
                     ajFmtPrintF(ouf_log, "Line %6d : Invalid relations line format (%S)\n", linecnt+1, line);          
                 /* Check for non-NULL terminal comment */
                 ajStrAssignS(&tok, ajStrParseC(NULL, ":!"));   
                 if(ajStrGetLen(tok) == 0)
                     ajFmtPrintF(ouf_log, "Line %6d : Invalid relations line format (%S)\n", linecnt+1, line);
                 break;
             }
           }
   }
   ajFmtPrintF(ouf_log, "\n\n");
   ajFileSeek(inf_edam, 0, 0);    /* Rewind file */



   /*  Check for unique ids */
   ajFmtPrintF(ouf_log, "7. UNIQUE IDS\n");          
   for(linecnt=0; ajReadline(inf_edam, &line); linecnt++)
   {
       /* Stop checking once first [Typedef] line is found. */ 
       if(ajStrPrefixC(line, "[Typedef]"))
           break;
       
       if(ajStrPrefixC(line, "id:"))
       {
           ajStrParseC(line, ":");
           ajStrParseC(NULL, ":");
           ajStrAssignClear(&tok);
           ajStrAssignS(&tok, ajStrParseC(NULL, ":"));

           for(x=0, y=0; x<nids; x++)
               if(ajStrMatchS(tok, ids[x]))
               {
                   y++;
                   if(y>1)
                   {
                       ajFmtPrintF(ouf_log, "Line %6d : Non-unique id: %S (%S)\n", linecnt+1, tok, line);
                       break;
                   }
               }
       }
   }
   ajFmtPrintF(ouf_log, "\n\n");
   ajFileSeek(inf_edam, 0, 0);    /* Rewind file */



   /*  Check for mandatory fields / field order */
   ajFmtPrintF(ouf_log, "8. STANDARD MANDATORY FIELDS AND FIELD ORDER\n");          
   for(first = ajTrue, done_first=ajFalse, linecnt=0; ajReadline(inf_edam, &line); linecnt++)
   {
 /*      id, name, namespace, def, (comment), (synonym), is_a */

       /* Stop checking once first [Typedef] line is found. */ 
       if(ajStrPrefixC(line, "[Typedef]"))
           break;

       if(ajStrPrefixC(line, "[Term]"))
       {
           /* Process previous term */
           if(done_first)
           {
               if(!found_id) 
                   ajFmtPrintF(ouf_log, "Line %6d : No id: field in term\n", tmp_line);
               if(!found_name)
                   ajFmtPrintF(ouf_log, "Line %6d : No name: field in term\n", tmp_line);
               if(!found_namespace)
                   ajFmtPrintF(ouf_log, "Line %6d : No namespace: field in term\n", tmp_line);
               if(!found_def)
                   ajFmtPrintF(ouf_log, "Line %6d : No def: field in term\n", tmp_line);
               /* No is_a needed for first term in file */
               if((!found_isa) && (!first))
                   ajFmtPrintF(ouf_log, "Line %6d : No is_a: field in term\n", tmp_line);

               first = ajFalse;
               
               
               /* edam_entity */
               if(ajStrMatchC(namespace, NAMESPACES[1]))
               {
                   if(found_concerns || found_has_input || found_is_input_of || found_has_output || found_is_output_of || found_has_source || found_is_source_of || found_is_identifier_of || found_is_attribute_of)
                       ajFmtPrintF(ouf_log, "Line %6d : Relation not allowed for term in this namespace\n", tmp_line);   

               }
               /* edam_field */
               else if(ajStrMatchC(namespace, NAMESPACES[2]))
               {
                   if(!found_concerns) 
                       ajFmtPrintF(ouf_log, "Line %6d : No concerns: relation in term\n", tmp_line);

                   if(found_is_concern_of || found_has_input || found_is_input_of || found_has_output || found_is_output_of || found_has_source || found_is_source_of || found_has_identifier || found_is_identifier_of || found_has_attribute || found_is_attribute_of || found_has_part || found_is_part_of)
                       ajFmtPrintF(ouf_log, "Line %6d : Relation not allowed for term in this namespace\n", tmp_line);   
               }
               /* edam_function */
               else if(ajStrMatchC(namespace, NAMESPACES[3]))
               {
                   if(!found_is_concern_of) 
                       ajFmtPrintF(ouf_log, "Line %6d : No is_concern_of: relation in term\n", tmp_line);

                   if(found_concerns || found_is_input_of || found_is_output_of || found_has_source || found_is_source_of || found_has_identifier || found_is_identifier_of || found_has_attribute || found_is_attribute_of || found_has_part || found_is_part_of)
                       ajFmtPrintF(ouf_log, "Line %6d : Relation not allowed for term in this namespace\n", tmp_line);   
               }
               /* edam_resource */
               else if(ajStrMatchC(namespace, NAMESPACES[4]))
               {
                   if(!found_is_source_of) 
                       ajFmtPrintF(ouf_log, "Line %6d : No is_source_of: relation in term\n", tmp_line);

                   if(found_concerns || found_has_input || found_is_input_of || found_has_output || found_is_output_of || found_has_source || found_is_identifier_of || found_has_attribute || found_is_attribute_of)
                       ajFmtPrintF(ouf_log, "Line %6d : Relation not allowed for term in this namespace\n", tmp_line);   
               }
               /* edam_datatype */
               else if(ajStrMatchC(namespace, NAMESPACES[5]))
               {
                   if(found_concerns || found_is_concern_of || found_has_input || found_has_output || found_is_source_of || found_has_attribute)
                       ajFmtPrintF(ouf_log, "Line %6d : Relation not allowed for term in this namespace\n", tmp_line);   
               }
               /* edam_identifier */
               else if(ajStrMatchC(namespace, NAMESPACES[6]))
               {
                   if(!found_is_identifier_of) 
                       ajFmtPrintF(ouf_log, "Line %6d : No is_identifier_of: relation in term\n", tmp_line);

                   if(found_concerns || found_is_concern_of || found_has_input || found_has_output || found_is_source_of || found_has_identifier || found_has_attribute || found_is_attribute_of || found_has_part || found_is_part_of)
                       ajFmtPrintF(ouf_log, "Line %6d : Relation not allowed for term in this namespace\n", tmp_line);   
               }
           }


           tmp_line        = linecnt+1;

           found_id               = ajFalse;
           found_name             = ajFalse;
           found_namespace        = ajFalse;
           found_def              = ajFalse;
           found_comment          = ajFalse;
           found_synonym          = ajFalse;
           found_isa              = ajFalse;
           found_concerns         = ajFalse;
           found_is_concern_of    = ajFalse;
           found_has_input        = ajFalse;
           found_is_input_of      = ajFalse;
           found_has_output       = ajFalse;
           found_is_output_of     = ajFalse;
           found_has_source       = ajFalse;
           found_is_source_of     = ajFalse;
           found_has_identifier   = ajFalse;
           found_is_identifier_of = ajFalse;
           found_has_attribute    = ajFalse;
           found_is_attribute_of  = ajFalse;
           found_has_part         = ajFalse;
           found_is_part_of       = ajFalse;
          
          done_first = ajTrue;
      }
      else if(ajStrPrefixC(line, "id:"))
      {
          found_id = ajTrue;
          if(found_name || found_namespace || found_def || found_comment || found_synonym || found_isa)
              ajFmtPrintF(ouf_log, "Line %6d : id: field in wrong order (%S)\n", linecnt+1, line);
      }

      else if(ajStrPrefixC(line, "name:"))
      {
          found_name = ajTrue;
          if( (!found_id) || found_namespace || found_def || found_comment || found_synonym || found_isa)
              ajFmtPrintF(ouf_log, "Line %6d : name: field in wrong order (%S)\n", linecnt+1, line);
      }
      else if(ajStrPrefixC(line, "namespace:"))
      {
          found_namespace = ajTrue;
          ajFmtScanS(line, "%*s %S", &namespace);
                    
          if((!found_id) || (!found_name) || found_def || found_comment || found_synonym || found_isa)
              ajFmtPrintF(ouf_log, "Line %6d : namespace: field in wrong order (%S)\n", linecnt+1, line);
      }
      else if(ajStrPrefixC(line, "def:"))
      {
          found_def = ajTrue;
          if((!found_id) || (!found_name) || (!found_namespace) || found_comment || found_synonym || found_isa)
              ajFmtPrintF(ouf_log, "Line %6d : def: field in wrong order (%S)\n", linecnt+1, line);
      }
      else if(ajStrPrefixC(line, "comment:"))
      {
          found_comment = ajTrue;
          if((!found_id) || (!found_name) || (!found_namespace) || (!found_def) || found_synonym || found_isa)
              ajFmtPrintF(ouf_log, "Line %6d : comment: field in wrong order (%S)\n", linecnt+1, line);
      }
      else if(ajStrPrefixC(line, "synonym:"))
      {
          found_synonym = ajTrue;
          if((!found_id) || (!found_name) || (!found_namespace) || (!found_def) || found_isa)
              ajFmtPrintF(ouf_log, "Line %6d : synonym: field in wrong order (%S)\n", linecnt+1, line);
      }
      else if(ajStrPrefixC(line, "is_a:"))
      {
          found_isa = ajTrue;
          if((!found_id) || (!found_name) || (!found_namespace) || (!found_def))
              ajFmtPrintF(ouf_log, "Line %6d : is_a: field in wrong order (%S)\n", linecnt+1, line);
      }
      else if(ajStrPrefixC(line, "concerns:"))
           found_concerns         = ajTrue;
      else if(ajStrPrefixC(line, "is_concern_of:"))
          found_is_concern_of    = ajTrue;
      else if(ajStrPrefixC(line, "has_input:"))
          found_has_input        = ajTrue;
      else if(ajStrPrefixC(line, "is_input_of:"))
          found_is_input_of      = ajTrue;
      else if(ajStrPrefixC(line, "has_output:"))
          found_has_output       = ajTrue;
      else if(ajStrPrefixC(line, "is_output_of:"))
          found_is_output_of     = ajTrue;
      else if(ajStrPrefixC(line, "has_source:"))
          found_has_source       = ajTrue;
      else if(ajStrPrefixC(line, "is_source_of:"))
          found_is_source_of     = ajTrue;
      else if(ajStrPrefixC(line, "has_identifier:"))
          found_has_identifier   = ajTrue;
      else if(ajStrPrefixC(line, "is_identifier_of:"))
          found_is_identifier_of = ajTrue;
      else if(ajStrPrefixC(line, "has_attribute:"))
          found_has_attribute    = ajTrue;
      else if(ajStrPrefixC(line, "is_attribute_of:"))
          found_is_attribute_of  = ajTrue;
      else if(ajStrPrefixC(line, "has_part:"))
          found_has_part         = ajTrue;
      else if(ajStrPrefixC(line, "is_part_of:"))
          found_is_part_of       = ajTrue;
           

  }
  ajFmtPrintF(ouf_log, "\n\n");
  ajFileSeek(inf_edam, 0, 0);    /* Rewind file */
  
  

  /*  Check for unique names within each namespace */
  ajFmtPrintF(ouf_log, "9. UNIQUE NAMES WITHIN EACH NAMESPACE\n");          
  for(linecnt=0; ajReadline(inf_edam, &line); linecnt++)
  {
      /* First build the arrays of names in each namespace */
      if(ajStrPrefixC(line, "[Term]"))
          tmp_term = ajTermNew();
      else if(ajStrPrefixC(line, "name:"))
      {
          ajStrParseC(line, ":");
          ajStrAssignS(&tmp_term->name, ajStrParseC(NULL, ":"));
          ajStrRemoveWhiteExcess(&tmp_term->name);
          tmp_term->line = linecnt;
      }
      else if(ajStrPrefixC(line, "id:"))
      {
          ajStrParseC(line, ":");
          ajStrParseC(NULL, ":");
          ajStrAssignS(&tmp_term->id, ajStrParseC(NULL, ":"));
          ajStrRemoveWhiteExcess(&tmp_term->id);          
      }
      else if(ajStrPrefixC(line, "namespace:"))
      {
          ajFmtScanS(line, "%*s %S", &namespace);
          for(x=0; x<NNAMESPACES; x++)
              if(ajStrMatchC(namespace, NAMESPACES[x]))
                  ajListPushAppend(namespaces[x]->list, tmp_term);
      }
  }
          
  for(x=0; x<NNAMESPACES; x++)
      namespaces[x]->n = ajListToarray(namespaces[x]->list, (void***) &(namespaces[x]->terms));


          
  for(x=0; x<NNAMESPACES; x++)
  {
      ajFmtPrintF(ouf_log, "9.%d %s\n", x+1, NAMESPACES[x]);          
      for(y=0; y<namespaces[x]->n; y++)
          for(z=0; z<namespaces[x]->n; z++)
              if((y!=z) && (ajStrMatchS(namespaces[x]->terms[y]->name, namespaces[x]->terms[z]->name)))
              {
                  ajFmtPrintF(ouf_log, "Line %6d : Non-unique name: %S (First used on line %d)\n",
                              namespaces[x]->terms[y]->line,
                              namespaces[x]->terms[y]->name,
                              namespaces[x]->terms[z]->line);
                  break;
              }
  }
  ajFmtPrintF(ouf_log, "\n\n");
  ajFileSeek(inf_edam, 0, 0);    /* Rewind file */
  
  
  /*  Check for unique names within each namespace */
  ajFmtPrintF(ouf_log, "10. VALID END-POINTS OF RELATIONS\n");          
  for(linecnt=0; ajReadline(inf_edam, &line); linecnt++)
  {
      if(ajStrPrefixC(line, "namespace:"))
      {
          /* Identify index of this namespace */
          ajFmtScanS(line, "%*s %S", &namespace);
          for(idx=0; idx<NNAMESPACES; idx++)
              if(ajStrMatchS(namespace, namespaces[idx]->name))
                  break;
          if(idx==NNAMESPACES)
              ajFatal("namespace not found - cannot recover");
/*          if(!ajStrMatchC(mode, "Fix relations"))
            continue; */
      }
      ajStrAssignClear(&tok);
      ajFmtScanS(line, "%S", &tok);
      for(x=0, done=ajFalse; x<NRELATIONS; x++)
          if(ajStrMatchC(tok, RELATIONS[x]))
          {
              done=ajTrue;
              break;
          }
      /* Relation line */
      if(done)
      {
          if(ajStrCalcCountC(line, ":!")!=3)
          {
              ajFmtPrintF(ouf_log, "Line %6d : Cannot check end-point (Invalid relations line format) (%S)\n", linecnt+1, line);
              if(ajStrMatchC(mode, "Fix relations"))
                  ajFatal("Unable to recover from invalid relations line format");
          }
          
          else
          {
              ajStrAssignS(&relation, ajStrParseC(line, ":!"));
              ajStrParseC(NULL, ":!");
              ajStrParseC(NULL, ":!");
              ajStrAssignS(&tmp_name, ajStrParseC(NULL, ":!"));             
              ajStrRemoveWhiteExcess(&tmp_name);
              
/*              if(ajStrMatchC(tmp_name, "Protein secondary database"))
                ajFmtPrint("Protein secondary database ...\n"); */
              
                  
              if(ajStrPrefixC(line, "is_a:"))
              {
                  if(!(id=FindTerm(idx, tmp_name, namespaces)))
                      if(!(id=FindTerm(edam_term, tmp_name, namespaces)))   /* Must also check children of root node where start/end-points of is_a relation are in different namespaces */
                      {
                          if(idx==edam_identifier) /* Must also check in "edam_datatype" namespace for identifiers */
                          {
                              if(!(id=FindTerm(edam_datatype, tmp_name, namespaces)))  
                                  ajFmtPrintF(ouf_log, "Line %6d : End-point term of relation does not exist (%S)\n",
                                              linecnt+1, line);
                          }
                          else
                              ajFmtPrintF(ouf_log, "Line %6d : End-point term of relation does not exist (%S)\n",
                                          linecnt+1, line);
                      }
                  
                  
/*                  if(ajStrMatchC(tmp_name, "Protein secondary database"))
                  {
                      if(id!=-1)
                          ajFmtPrint("... found !!!!\n");
                      else
                          ajFmtPrint("... not found !!!!\n");
                          } */
              }
              else if(ajStrPrefixC(line, "has_part:") || ajStrPrefixC(line, "is_part_of:"))
              {
                  if(!(id=FindTerm(idx, tmp_name, namespaces)))
                      ajFmtPrintF(ouf_log, "Line %6d : End-point term of relation does not exist (%S)\n",
                                  linecnt+1, line);
              }
              else if (ajStrPrefixC(line, "concerns:"))
              {
                  if(!(id=FindTerm(edam_entity, tmp_name, namespaces)))
                      if(!(id=FindTerm(edam_function, tmp_name, namespaces)))
                          if(!(id=FindTerm(edam_resource, tmp_name, namespaces)))
                              ajFmtPrintF(ouf_log, "Line %6d : End-point term of relation does not exist (%S)\n",
                                          linecnt+1, line);
              }
              else if (ajStrPrefixC(line, "is_concern_of:"))
              {
                  if(!(id=FindTerm(edam_field, tmp_name, namespaces)))
                      ajFmtPrintF(ouf_log, "Line %6d : End-point term of relation does not exist (%S)\n",
                                  linecnt+1, line);
              }
              else if (ajStrPrefixC(line, "has_input:") || ajStrPrefixC(line, "has_output:") || ajStrPrefixC(line, "has_attribute:") || ajStrPrefixC(line, "is_source_of:"))
              {
                  if(!(id=FindTerm(edam_datatype, tmp_name, namespaces)))
                      ajFmtPrintF(ouf_log, "Line %6d : End-point term of relation does not exist (%S)\n",
                                  linecnt+1, line);
              }
              else if (ajStrPrefixC(line, "is_input_of:") || ajStrPrefixC(line, "is_output_of:"))
              {
                  if(!(id=FindTerm(edam_function, tmp_name, namespaces)))
                      ajFmtPrintF(ouf_log, "Line %6d : End-point term of relation does not exist (%S)\n",
                                  linecnt+1, line);
              }
              else if (ajStrPrefixC(line, "has_source:"))
              {
                  if(!(id=FindTerm(edam_resource, tmp_name, namespaces)))
                      ajFmtPrintF(ouf_log, "Line %6d : End-point term of relation does not exist (%S)\n",
                                  linecnt+1, line);
              }
              else if (ajStrPrefixC(line, "has_identifier:"))
              {
                  if(!(id=FindTerm(edam_identifier, tmp_name, namespaces)))
                      ajFmtPrintF(ouf_log, "Line %6d : End-point term of relation does not exist (%S)\n",
                                  linecnt+1, line);
              }
              else if (ajStrPrefixC(line, "is_identifier_of:"))
              {
                  if(!(id=FindTerm(edam_entity, tmp_name, namespaces)))
                      if(!(id=FindTerm(edam_resource, tmp_name, namespaces)))
                          if(!(id=FindTerm(edam_datatype, tmp_name, namespaces)))
                              ajFmtPrintF(ouf_log, "Line %6d : End-point term of relation does not exist (%S)\n",
                                          linecnt+1, line);
              }
              else if (ajStrPrefixC(line,  "is_attribute_of:"))
              {
                  if(!(id=FindTerm(edam_entity, tmp_name, namespaces)))
                      ajFmtPrintF(ouf_log, "Line %6d : End-point term of relation does not exist (%S)\n",
                                  linecnt+1, line);
              }
              else
                  ajFatal("Unknown relation");

              /*
              if(ajStrMatchC(tmp_name, "Protein secondary database"))
              {
                  if(id!=NULL)
                     ajFmtPrint("... found !\n");
                  else
                      ajFmtPrint("... not found !\n");
              }
              */              
              
              /* Relation line ... write to output file */
              if(ajStrMatchC(mode, "Fix relations"))
              {
                  if(id != NULL)
                      ajFmtPrintF(ouf_edam, "%S: EDAM:%S ! %S\n", relation, id, tmp_name);
                  else
                      ajFatal("Could not find end-point term of relation (or recover) : %S", tmp_name);
              }
          }
      }
      /* Not a relation line ... write to output file */
      else if(ajStrMatchC(mode, "Fix relations"))
          ajFmtPrintF(ouf_edam, "%S", line);
      
  
  }
  ajFmtPrintF(ouf_log, "\n\n");
  ajFileSeek(inf_edam, 0, 0);    /* Rewind file */


  /* Renumber term ids */
  if(ajStrMatchC(mode, "Renumber terms"))
  {
      for(termcnt=0, found_typedef = ajFalse; ajReadline(inf_edam, &line); )
      {
          if(ajStrPrefixC(line, "[Term]"))
              termcnt++;
          else if(ajStrPrefixC(line, "[Typedef]"))
              found_typedef = ajTrue;
                 
          if(ajStrPrefixC(line, "id:") && (!found_typedef))
              ajFmtPrintF(ouf_edam, "id: EDAM:%07d\n", termcnt);
          else
              ajFmtPrintF(ouf_edam, "%S", line);
      }
  }

      
  
  /* Clean up and exit */
/*
  for(x=0; x<nfields; x++)
      ajStrDel(&fields[x]);
  AJFREE(fields);

  for(x=0; x<nids; x++)
      ajStrDel(&ids[x]);
  AJFREE(ids);

  
  ajFileClose(&inf_edam);
  ajFileClose(&ouf_edam);
  ajFileClose(&ouf_log);
  ajStrDel(&line);
  ajStrDel(&tok);
  ajStrDel(&name);
  ajStrDel(&namespace);
  ajStrDel(&relation);
  ajStrDel(&tmp_name);
  ajStrDel(&tmp_id);

  
  for(x=0; x<NNAMESPACES; x++)
      ajNamespaceDel(&namespaces[x]);
*/  
  ajExit();
  return 0;
}
