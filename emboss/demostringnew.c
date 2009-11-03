/* @source
**
** Illustrates use of the AJAX string library.
** 
** @author Copyright (C) Jon Ison (jison@ebi.ac.uk)
** @@
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
******************************************************************************/

#include "emboss.h"








    AjPStr    str0  = NULL;      
    AjPStr    str1  = NULL;      
    AjPStr    str2  = NULL;      
    AjPStr    str3  = NULL;        
    AjPStr    str4  = NULL;        
    AjPStr    str5  = NULL;        

    char*     txt0  = NULL;      
    char*     txt1  = NULL;      
    char*     txt2  = NULL;      
    char*     txt3  = NULL;      
    char*     txt4  = NULL;      
    char*     txt5  = NULL;      



static void demostringnew_msg(const char * msg);



/* @prog demostringnew ********************************************************
**
** Illustrates use of the AJAX string library.
**
******************************************************************************/

int main(int argc, char **argv)
{
    /*         
    ** All pointers set to NULL for safety. 
    ** Variables names and initialisation values aligned for clarity.
    */

    AjBool    boo   = ajFalse;
    ajint     n1    = 0;
    ajint     n2    = 0;
    ajlong    l1    = 0;     /* long int */
    float     f1    = 0.0;
    double    d1    = 0.0;   /* there is no long double */
    size_t    size  = 100;   /* Reserved memory size.  Could be any value you know in advance. */


    embInit("demostringnew", argc, argv);
    
    demostringnew_msg("/* Starting string values */");
    

    /* Functions with the prefix ajStr are for manipulating EMBOSS strings.
       Functions with the prefix ajChar are for manipulating C-type (char*) string 
       See filesection and datasection sections in ajstr.c */


    /*
    ** String constructor functions 
    ** See "@section constructors" in ajstr.c
    */

    /* Construct a new string with no starting value or reserved size.
       There is no equivlent function for C-type (char*) strings */
    str0  = ajStrNew ();

    /* Construct a new string with a reserved size but no starting value */
    txt1 = ajCharNewRes(size);  
    str1 = ajStrNewRes (size);    

    /* Construct a new C-type (char*) string with a starting value ... */
    txt2 = ajCharNewC ("Starting value");   /* ... copied from a C-type (char*) string */
    str2 = ajStrNewC  (txt2);                /* ... copied from a C-type (char*) string */
    txt3 = ajCharNewS (str2);                /* ... copied from a string */
    str3 = ajStrNewS  (str2);                /* ... copied from a string */

    /* Construct a new string with a reserved size and starting value ... */
    txt4 = ajCharNewResC("Starting value, reserved size", size);  /* ... copied from a C-type (char*) string)*/
    str4 = ajStrNewResC (txt4, size);                              /* ... copied from a C-type (char*) string */
    /* or str4 = ajStrNewResLenC(txt4, size, strlen(txt4)); to specify string length */
    txt5 = ajCharNewResS(str4, size);                              /* ... copied from a string */
    str5 = ajStrNewResS (str4, size);                              /* ... copied from a string */

    demostringnew_msg("/* After string constructor functions */");    





    /*
    ** String destructor functions 
    ** See "@section destructors" in ajstr.c) 
    */
    
    /* Destruct a string */
    ajCharDel(&txt1);
    ajCharDel(&txt2);
    ajCharDel(&txt3);
    ajCharDel(&txt4);
    ajCharDel(&txt5);
    ajStrDel (&str0);
    ajStrDel (&str1);
    ajStrDel (&str3);
    ajStrDel (&str5);

    /* str2 & str4 still in memory */
    demostringnew_msg("/* After string destructor functions */");    





    /*
    ** String (de)referencing functions 
    ** See "@section destructors" in ajstr.c)
    */

    str0 = ajStrNewRef(str2);
    /* or ajStrAssignRef(&str0, str2); */
    demostringnew_msg("/* After string reference */");    

    ajStrDelStatic(&str0);
    demostringnew_msg("/* After string dereference */");    





    /*
    ** String assignment functions 
    ** See "@section assignment" in ajstr.c)
    */

    /* Still only str2 & str4 in memory */

    /* Assign a string value using ... */
    ajStrAssignC(&str1, "Assigned value");     /* ... a C-type (char*) string */
    /*    or ajStrAssignLenC(&str1, "Assigned value", strlen("Assigned value")); to specify string length. */
    ajStrAssignS(&str3, str1);                 /* ... a string                */
    ajStrAssignK(&str5, 'A');                  /* ... a character             */
    demostringnew_msg("/* After string assignment 1 */");    

    ajStrAssignSubC(&str1, "Assigned value", 0, 11);
    ajStrAssignSubS(&str3, str1, 0, 9);
    demostringnew_msg("/* After string assignment 2 */");    

    /* The assignment functions allocate memory if necessary so str1, str3 and str5 will be created for you.  It's bad practice to use this mechanism however because it's not obvious the string has been allocated (and needs freeing).  Much cleaner to call the construct (ajStrNew) explicitly. */



    /* Assign a string with a reserved size and value using ... */
    ajStrAssignResC(&str1, size, "Assigned value, reserved size");  /* ... a C-type (char*) string */
    ajStrAssignResS(&str3, size, str1);                             /* ... a string                */
    demostringnew_msg("/* After string assignment 3 */");    



    /* Assign a string value only if the string is empty using ... */
    str0 = ajStrNew();
    ajStrAssignEmptyC(&str0, "New value if string was empty");    /* ... a C-type (char*) string */ 
    ajStrAssignEmptyS(&str1, str0);                               /* ... a string                */
    demostringnew_msg("/* After string assignment 4 */");    

    /* Now str0-5 in memory.  The above code is for illustrative purposes: it's much cleaner to put all the constructors / destructors at the top / bottom of the code where possible. */


    /* Assign all strings intuitive values */
    txt0 = ajCharNewResC("TEXT 0", 100);   
    txt1 = ajCharNewResC("TEXT 1", 100);   
    txt2 = ajCharNewResC("Text 2", 100);   
    txt3 = ajCharNewResC("Text 3", 100);   
    txt4 = ajCharNewResC("Text 4", 100);   
    txt5 = ajCharNewResC("Text 5", 100);
    ajStrAssignC(&str0, "STRING 0");   
    ajStrAssignC(&str1, "STRING 1");   
    ajStrAssignC(&str2, "String 2");   
    ajStrAssignC(&str3, "String 3");   
    ajStrAssignC(&str4, "String 4 WITHSOMETEXTINABLOCK");   
    ajStrAssignC(&str5, "String 5 WITHSOMETEXTINABLOCK");   
    demostringnew_msg("/* After string assignment 5 */");    





    /*
    ** String formatting functions 
    ** See "@section formatting" in ajstr.c
    */
    ajCharFmtLower(txt0);
    ajCharFmtLower(txt1);
    ajStrFmtLower(&str0);
    ajStrFmtLowerSub(&str1, 0, 2);

    ajCharFmtUpper(txt2);
    ajCharFmtUpper(txt3);
    ajStrFmtUpper(&str2);
    ajStrFmtUpperSub(&str3, 0, 2);
    demostringnew_msg("/* After string formatting 1 */");    


    ajStrFmtTitle(&str0);
    ajStrFmtQuote(&str1);
    ajStrFmtBlock(&str4, 3);
    demostringnew_msg("/* After string formatting 2 */");    


    /* See also ajStrFmtWrap, ajStrFmtWrapLeft
       ... these need checking. */





    /*
    ** String conversion functions 
    ** See "@section datatype to string conversion" in ajstr.c
    */
    n1 = n2 = l1 = 1;
    f1 = d1 = 0.5;
    ajStrFromBool( &str0, boo);
    ajStrFromInt(&str1, n1);
    ajStrFromLong(&str2, l1);
    ajStrFromFloat(&str3, f1, 5);
    ajStrFromDouble(&str4, d1, 5);
    ajStrFromDoubleExp(&str5, d1, 5);
    demostringnew_msg("/* After datatype to string conversion */");    


    /*
    ** String conversion functions 
    ** See "@section string to datatype conversion" in ajstr.c
    */
    ajStrToBool(str0, &boo);
    ajStrToInt(str1, &n1);
    ajStrToLong(str2, &l1);
    ajStrToDouble(str4, &d1);
    ajUser("/* After string to datatype conversion */\n"
	   "boo (from str0): %B\nn1 (from str1): %d\nl1 (from str2): %d", 
	   boo, n1, l1);
    ajFmtPrint("f1 (from str3): %f\nd1 (from str4): %f\n\n\n\n", f1, d1);
    /* Check ajUser ... doesn't support %f */
    /* See also  ajStrToHex */





    /* Assign all strings new values */
    strcpy(txt0, "Text String");
    strcpy(txt1, "TEXT STRING");
    strcpy(txt2, "Text*");
    strcpy(txt3, "Text");
    strcpy(txt4, "Text String 4");
    strcpy(txt5, "Text String 5");

    ajStrAssignC(&str0, "String");   
    ajStrAssignC(&str1, "STRING");   
    ajStrAssignC(&str2, "String*");   
    ajStrAssignC(&str3, "*String");   
    ajStrAssignC(&str4, "String 4");   
    ajStrAssignC(&str5, "String 5");   
    demostringnew_msg("/* After resetting strings */");    





    /*
    ** String comparison functions 
    ** See "@section comparison" in ajstr.c
    */
    ajUser("/* String comparison functions */");
    boo = ajCharMatchC(txt0, txt1);
    ajUser("ajCharMatchC(txt0 txt1); == %B", boo);
    boo = ajCharMatchCaseC(txt0, txt1);
    ajUser("ajCharMatchCaseC(txt0 txt1); == %B", boo);
    boo = ajCharMatchC(txt0, txt2);
    ajUser("ajCharMatchC(txt0,txt2); == %B", boo);
    boo = ajCharMatchWildC(txt0, txt2);
    ajUser("ajCharMatchWildC(txt0,txt2); == %B", boo);
    boo = ajCharMatchWildS(txt0, str2);
    ajUser("ajCharMatchWildS(txt0,str2); == %B", boo);
    /* See also ajCharMatchWildNextC, ajCharMatchWildWordC
       ... these need checking & documentation updated. */

    boo = ajCharPrefixC(txt0, txt3);
    ajUser("ajCharPrefixC(txt0, txt3); == %B", boo);
    boo = ajCharPrefixS(txt0, str0);
    ajUser("ajCharPrefixS(txt0, str0); == %B", boo);
    boo = ajCharPrefixCaseC(txt5, txt1);
    ajUser("ajCharPrefixCaseC(txt5, txt1); == %B", boo);
    boo = ajCharPrefixCaseC(txt1, txt5);
    ajUser("ajCharPrefixCaseC(txt1, txt5); == %B", boo);
    boo = ajCharPrefixCaseS(txt0, str0);
    ajUser("ajCharPrefixCaseS(txt0, str0); == %B", boo);
    boo = ajCharSuffixC(txt0, txt3);
    ajUser("ajCharSuffixC(txt0, txt3); === %B", boo);
    boo = ajCharSuffixS(txt0, str0);
    ajUser("ajCharSuffixS(txt0, str0); == %B", boo);

    /* See also ajCharSuffixCaseC, ajCharSuffixCaseC, ajCharSuffixCaseS, ajCharSuffixCaseS
       ... these need checking. */
    boo =  ajStrMatchC (str0, txt0);
    ajUser("ajStrMatchC (str0, txt0); == %B", boo);
    boo = ajStrMatchS(str0, str1);
    ajUser("ajStrMatchS(str0, str1); == %B", boo);
    boo = ajStrMatchCaseC(str0, txt0);
    ajUser("ajStrMatchCaseC(str0, txt0); == %B", boo);
    boo = ajStrMatchCaseS(str0, str0);
    ajUser("ajStrMatchCaseS(str0, str0); == %B", boo);


    /*
    ajUser("== %B", boo);

    boo = ajStrMatchWildC(str2, const char* text);
     ajStrMatchWildS  (const AjPStr thys, const AjPStr wild);
     ajStrMatchWildWordC (const AjPStr str, const char* text);
     ajStrMatchWildWordS (const AjPStr str, const AjPStr text);
     ajStrPrefixC(const AjPStr str, const char* txt2);
     ajStrPrefixS(const AjPStr str, const AjPStr str2);
     ajStrPrefixCaseC (const AjPStr str, const char* pref);
     ajStrPrefixCaseS (const AjPStr str, const AjPStr pref);
     ajStrSuffixC (const AjPStr thys, const char* suff);
     ajStrSuffixS (const AjPStr thys, const AjPStr suff);
    */





    /**************************************************************************/
    /* String substitution functions (See "@section substitution" in ajstr.c) */
    /**************************************************************************/

    /*
AjBool     ajStrExchangeCC(AjPStr* Pstr, const char* txt, const char* txtnew);
AjBool     ajStrExchangeCS(AjPStr* Pstr, const char* txt,
                           const AjPStr strnew);
AjBool     ajStrExchangeKK(AjPStr* Pstr, char chr, char chrnew);
AjBool     ajStrExchangeSC(AjPStr* Pstr, const AjPStr str,
                           const char* txtnew);
AjBool     ajStrExchangeSS(AjPStr* Pstr, const AjPStr str,
                           const AjPStr strnew);
AjBool     ajStrExchangeSetCC(AjPStr* Pstr, const char* oldc,
                              const char* newc);
AjBool     ajStrExchangeSetSS(AjPStr* Pstr, const AjPStr str,
                            const AjPStr strnew);
AjBool     ajStrRandom(AjPStr *s);
AjBool     ajStrReverse(AjPStr* Pstr);
    */


    embExit();
    return 0;
}


/* @funcstatic demostringnew_msg **********************************************
**
** Prints string and text details to the user
**
** @param [r] msg [const char*] Message prefix
** @return [void]
******************************************************************************/

static void demostringnew_msg(const char * msg)
 {
   ajUser("%s\nstr0: %S\nstr1: %S\nstr2: %S\nstr3: %S\nstr4: %S\nstr5: %S\n"
	  "txt0: %s\ntxt1: %s\ntxt2: %s\ntxt3: %s\ntxt4: %s\ntxt5: %s\n\n\n",
	  msg, str0, str1, str2, str3, str4, str5,
	  txt0, txt1, txt2, txt3, txt4, txt5);
   
   return;
}






           
