/* @source ajutil *************************************************************
**
**AJAX utility functions
**
** @author Copyright (C) 1998 Ian Longden
** @author Copyright (C) 1998 Peter Rice
** @version $Revision: 1.66 $
** @modified $Date: 2012/03/28 21:11:23 $ by $Author: mks $
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

#include "ajlib.h"

#include "ajutil.h"

/* include anything with an Exit function */

#include "ajindex.h"
#include "ajsql.h"
#include "ajmatrices.h"
#include "ajtree.h"
#include "ajdmx.h"
#include "ajdomain.h"
#include "ajquery.h"
#include "ajtext.h"
#include "ajfeat.h"
#include "ajseq.h"
#include "ajobo.h"
#include "ajassem.h"
#include "ajrefseq.h"
#include "ajtax.h"
#include "ajurl.h"
#include "ajvar.h"
#include "ajresource.h"
#include "ajphylo.h"
#include "ajnexus.h"
#include "ajalign.h"
#include "ajreport.h"
#include "ajnam.h"
#include "ajsys.h"
#include "ajcall.h"
#include "ajbase.h"
#include "ajcod.h"
#include "ajtranslate.h"
#include "ajdan.h"
#include "ajtime.h"
#include "ajreg.h"
#include "ajarr.h"
#include "ajfiledata.h"
#include "ajfile.h"
#include "ajlist.h"
#include "ajtable.h"
#include "ajstr.h"
#include "ajmem.h"

#include <stdarg.h>
#ifdef WIN32
#include "win32.h"
#include <winsock2.h>
#include <lmcons.h> /* for UNLEN */
#else /* !WIN32 */
#include <pwd.h>
#include <unistd.h>
#endif /* WIN32 */



static AjBool utilBigendian;
static ajint utilBigendCalled = 0;

static AjBool utilIsBase64(char c);
static char   utilBase64Encode(unsigned char u);
static unsigned char utilBase64Decode(char c);




/* @filesection ajutil ********************************************************
**
** @nam1rule aj Function belongs to the AJAX library.
**
******************************************************************************/




/* @datasection [none] Exit functions *****************************************
**
** @nam2rule Exit Functions for exiting cleanly
**
******************************************************************************/




/* @section exit  *************************************************************
**
** Functions for exiting cleanly.
**
** @fdata [none]
**
** @nam3rule  Abort  Exits without flushing any files.
** @nam3rule  Bad    Calls 'exit' with an unsuccessful code (EXIT_FAILURE
**                   defined in stdlib.h).
**
** @valrule   *  [void] All functions do not return
**
** @fcategory misc
**
******************************************************************************/




/* @func ajExit ***************************************************************
**
** Calls 'exit' with a successful code (zero), but first calls ajReset to
** call memory clean up and debug reporting functions.
**
** No cleanup or reporting routines are called. Simply crashes.
**
** @return [void]
**
** @release 1.0.0
** @@
******************************************************************************/

__noreturn void ajExit(void)
{
    ajReset();
    exit(EXIT_SUCCESS);
}




/* @func ajExitAbort **********************************************************
**
** Exits without flushing any files. Needed for exit from, for example,
** a failed system call (ajFileNewInPipe, and so on) where the parent
** process has open output files, and the child process needs to exit
** without affecting them. Failure to exit this way can mean the output
** buffer is flushed twice.
**
** Calls '_exit' with an unsuccessful code (EXIT_FAILURE defined in stdlib.h).
**
** No cleanup or reporting routines are called. Simply crashes.
**
** @return [void]
**
** @release 3.0.0
** @@
******************************************************************************/

__noreturn void  ajExitAbort(void)
{
    _exit(EXIT_FAILURE);
}




/* @func ajExitBad ************************************************************
**
** Calls 'exit' with an unsuccessful code (EXIT_FAILURE defined in stdlib.h).
**
** No cleanup or reporting routines are called. Simply crashes.
**
** @return [void]
**
** @release 1.0.0
** @@
******************************************************************************/

__noreturn void  ajExitBad(void)
{
    exit(EXIT_FAILURE);
}




/* @datasection [none] Memory cleanup functions *******************************
**
** @nam2rule  Reset  Resets internal memory and returns.
**
******************************************************************************/




/* @section reset *************************************************************
**
** Functions for memory cleanup
**
** @fdata [none]
**
**
** @valrule   *  [void] No return value
**
** @fcategory misc
**
******************************************************************************/




/* @func ajReset **************************************************************
**
** Cleans up all internal memory by calling cleanup routines which
** can report on resource usage etc.
**
** Intended to be called at the end of processing by exit functions.
**
** @return [void]
**
** @release 5.0.0
** @@
******************************************************************************/

void ajReset(void)
{
#ifdef WIN32
    WSACleanup();
#endif /* WIN32 */
    ajDebug("\nFinal Summary\n=============\n\n");
    ajBtreeExit();
    ajSqlExit();
    ajMatrixExit();
    ajTreeExit();
    ajPdbExit();
    ajDmxExit();
    ajDomainExit();
    ajQueryExit();
    ajTextExit();
    ajFeatExit();
    ajSeqExit();
    ajOboExit();
    ajAssemExit();
    ajRefseqExit();
    ajTaxExit();
    ajUrlExit();
    ajVarExit();
    ajResourceExit();
    ajPhyloExit();
    ajNexusExit();
    ajAlignExit();
    ajReportExit();
    ajNamExit();
    ajSysExit();
    ajCallExit();
    ajBaseExit();
    ajCodExit();
    ajTrnExit();
    ajMeltExit();
    ajTimeExit();
    ajRegExit();
    ajArrExit();
    ajDatafileExit();
    ajFileExit();
    ajListExit();
    ajMessExit();
    ajTableExit();
    ajStrExit();
    ajMemExit();
    ajMessExitDebug();     /* clears data for ajDebug - do this last!!!  */

    return;
}




/* @datasection [none] Byte manipulation functions ****************************
**
** @nam2rule  Byte          Manipulate a byte of data.
**
******************************************************************************/




/* @section byte manipulation functions  **************************************
**
** Functions for manipulating bytes.
**
** @fdata [none]
**
** @nam3rule  Rev       Reverse the byte order.
** @nam4rule  RevLen2      Reverse the byte order in a 2 byte integer.
** @nam4rule  RevLen4      Reverse the byte order in a 4 byte integer.
** @nam4rule  RevLen8      Reverse the byte order in a 8 byte integer.
** @nam4rule  RevLen2u      Reverse the byte order in a 2 byte unsigned integer.
** @nam4rule  RevLen4u      Reverse the byte order in a 4 byte unsigned integer.
** @nam4rule  RevLen8u      Reverse the byte order in a 8 byte unsigned integer.
** @nam4rule  RevInt    Reverse the byte order in an integer.
** @nam4rule  RevShort  Reverse the byte order in a short integer.
** @nam4rule  RevLong   Reverse the byte order in a long.
** @nam4rule  RevUint    Reverse the byte order in an unsigned integer.
** @nam4rule  RevUlong   Reverse the byte order in an unsigned long.
**
** @argrule   RevLen2  sval [ajshort*] Short to be reversed
** @argrule   RevLen4  ival [ajint*] Integer to be reversed
** @argrule   RevLen8  lval [ajlong*] Long integer to be reversed
** @argrule   RevLen2u  sval [ajushort*] Unsigned short to be reversed
** @argrule   RevLen4u  ival [ajuint*] Unsigned integer to be reversed
** @argrule   RevLen8u  lval [ajulong*] Unsigned long integer to be reversed
** @argrule   RevShort sval [ajshort*] Short to be reversed
** @argrule   RevInt   ival [ajint*] Integer to be reversed
** @argrule   RevLong  lval [ajlong*] Long integer to be reversed
** @argrule   RevUint  ival [ajuint*] Unsigned integer to be reversed
** @argrule   RevUlong lval [ajulong*] Unsigned long integer to be reversed
**
** @valrule   *  [void]
**
** @fcategory misc
**
******************************************************************************/




/* @func ajByteRevInt *********************************************************
**
** Reverses the byte order in an integer.
**
** @param [u] ival [ajint*] Integer in wrong byte order.
**                        Returned in correct order.
** @return [void]
**
** @release 5.0.0
** @@
******************************************************************************/

void ajByteRevInt(ajint* ival)
{
    union lbytes
    {
        char chars[8];
        ajint i;
    } data, revdata;

    char* cs;
    char* cd;
    ajuint i;

    data.i = *ival;
    cs     = data.chars;
    cd     = &revdata.chars[sizeof(ajint)-1];

    for(i=0; i < sizeof(ajint); i++)
    {
        *cd = *cs++;
        --cd;
    }

    *ival = revdata.i;

    return;
}




/* @func ajByteRevLen2 ********************************************************
**
** Reverses the byte order in a 2 byte integer.
**
** Intended for cases where the number of bytes is known, for
** example when reading a binary file.
**
** @param [u] sval [ajshort*] Short integer in wrong byte order.
**                          Returned in correct order.
** @return [void]
**
** @release 5.0.0
** @@
******************************************************************************/

void ajByteRevLen2(ajshort* sval)
{
    union lbytes
    {
        char chars[2];
        ajshort s;
    } data, revdata;

    char* cs;
    char* cd;
    ajint i;

    data.s = *sval;
    cs     = data.chars;
    cd     = &revdata.chars[1];

    for(i=0; i < 2; i++)
    {
        *cd = *cs++;
        --cd;
    }

    *sval = revdata.s;

    return;
}




/* @func ajByteRevLen2u *******************************************************
**
** Reverses the byte order in a 2 byte integer.
**
** Intended for cases where the number of bytes is known, for
** example when reading a binary file.
**
** @param [u] sval [ajushort*] Short integer in wrong byte order.
**                          Returned in correct order.
** @return [void]
**
** @release 6.2.0
** @@
******************************************************************************/

void ajByteRevLen2u(ajushort* sval)
{
    union lbytes
    {
        char chars[2];
        ajushort s;
    } data, revdata;

    char* cs;
    char* cd;
    ajint i;

    data.s = *sval;
    cs     = data.chars;
    cd     = &revdata.chars[1];

    for(i=0; i < 2; i++)
    {
        *cd = *cs++;
        --cd;
    }

    *sval = revdata.s;

    return;
}




/* @func ajByteRevLen4 ********************************************************
**
** Reverses the byte order in a 4 byte integer.
**
** Intended for cases where the number of bytes is known, for
** example when reading a binary file.
**
** @param [u] ival [ajint*] Integer in wrong byte order.
**                        Returned in correct order.
** @return [void]
**
** @release 5.0.0
** @@
******************************************************************************/

void ajByteRevLen4(ajint* ival)
{
    union lbytes
    {
        char chars[4];
        ajint i;
    } data, revdata;

    char* cs;
    char* cd;
    ajint i;

    data.i = *ival;
    cs     = data.chars;
    cd     = &revdata.chars[3];

    for(i=0; i < 4; i++)
    {
        *cd = *cs++;
        --cd;
    }

    *ival = revdata.i;

    return;
}




/* @func ajByteRevLen4u *******************************************************
**
** Reverses the byte order in a 4 byte integer.
**
** Intended for cases where the number of bytes is known, for
** example when reading a binary file.
**
** @param [u] ival [ajuint*] Integer in wrong byte order.
**                        Returned in correct order.
** @return [void]
**
** @release 6.2.0
** @@
******************************************************************************/

void ajByteRevLen4u(ajuint* ival)
{
    union lbytes
    {
        char chars[4];
        ajuint i;
    } data, revdata;

    char* cs;
    char* cd;
    ajint i;

    data.i = *ival;
    cs     = data.chars;
    cd     = &revdata.chars[3];

    for(i=0; i < 4; i++)
    {
        *cd = *cs++;
        --cd;
    }

    *ival = revdata.i;

    return;
}




/* @func ajByteRevLen8 ********************************************************
**
** Reverses the byte order in an 8 byte long.
**
** Intended for cases where the number of bytes is known, for
** example when reading a binary file.
**
** @param [u] lval [ajlong*] Integer in wrong byte order.
**                           Returned in correct order.
** @return [void]
**
** @release 5.0.0
** @@
******************************************************************************/

void ajByteRevLen8(ajlong* lval)
{
    union lbytes
    {
        char chars[8];
        ajlong l;
    } data, revdata;

    char* cs;
    char* cd;
    ajint i;

    data.l = *lval;
    cs     = data.chars;
    cd     = &revdata.chars[7];

    for(i=0; i < 8; i++)
    {
        *cd = *cs++;
        --cd;
    }

    *lval = revdata.l;

    return;
}




/* @func ajByteRevLen8u *******************************************************
**
** Reverses the byte order in an 8 byte long.
**
** Intended for cases where the number of bytes is known, for
** example when reading a binary file.
**
** @param [u] lval [ajulong*] Integer in wrong byte order.
**                           Returned in correct order.
** @return [void]
**
** @release 6.2.0
** @@
******************************************************************************/

void ajByteRevLen8u(ajulong* lval)
{
    union lbytes
    {
        char chars[8];
        ajulong l;
    } data, revdata;

    char* cs;
    char* cd;
    ajint i;

    data.l = *lval;
    cs     = data.chars;
    cd     = &revdata.chars[7];

    for(i=0; i < 8; i++)
    {
        *cd = *cs++;
        --cd;
    }

    *lval = revdata.l;

    return;
}




/* @func ajByteRevLong ********************************************************
**
** Reverses the byte order in a long.
**
** @param [u] lval [ajlong*] Integer in wrong byte order.
**                           Returned in correct order.
** @return [void]
**
** @release 5.0.0
** @@
******************************************************************************/

void ajByteRevLong(ajlong* lval)
{
    union lbytes
    {
        char chars[8];
        ajlong l;
    } data, revdata;

    char* cs;
    char* cd;
    ajuint i;

    data.l = *lval;
    cs     = data.chars;
    cd     = &revdata.chars[sizeof(ajlong)-1];

    for(i=0; i < sizeof(ajlong); i++)
    {
        *cd = *cs++;
        --cd;
    }

    *lval = revdata.l;

    return;
}




/* @func ajByteRevShort *******************************************************
**
** Reverses the byte order in a short integer.
**
** @param [u] sval [ajshort*] Short integer in wrong byte order.
**                          Returned in correct order.
** @return [void]
**
** @release 5.0.0
** @@
******************************************************************************/

void ajByteRevShort(ajshort* sval)
{
    union lbytes
    {
        char chars[8];
        short s;
    } data, revdata;

    char* cs;
    char* cd;
    ajuint i;

    data.s = *sval;
    cs     = data.chars;
    cd     = &revdata.chars[sizeof(ajshort)-1];

    for(i=0; i < sizeof(ajshort); i++)
    {
        *cd = *cs++;
        --cd;
    }

    *sval = revdata.s;

    return;
}




/* @func ajByteRevUint ********************************************************
**
** Reverses the byte order in an unsigned integer.
**
** @param [u] ival [ajuint*] Unsigned integer in wrong byte order.
**                        Returned in correct order.
** @return [void]
**
** @release 5.0.0
** @@
******************************************************************************/

void ajByteRevUint(ajuint* ival)
{
    union lbytes
    {
        char chars[8];
        ajuint i;
    } data, revdata;

    char* cs;
    char* cd;
    ajuint i;

    data.i = *ival;
    cs     = data.chars;
    cd     = &revdata.chars[sizeof(ajuint)-1];

    for(i=0; i < sizeof(ajuint); i++)
    {
        *cd = *cs++;
        --cd;
    }

    *ival = revdata.i;

    return;
}




/* @func ajByteRevUlong *******************************************************
**
** Reverses the byte order in an unsigned long.
**
** @param [u] lval [ajulong*] Integer in wrong byte order.
**                           Returned in correct order.
** @return [void]
**
** @release 6.4.0
** @@
******************************************************************************/

void ajByteRevUlong(ajulong* lval)
{
    union lbytes
    {
        char chars[8];
        ajulong l;
    } data, revdata;

    char* cs;
    char* cd;
    ajuint i;

    data.l = *lval;
    cs     = data.chars;
    cd     = &revdata.chars[sizeof(ajulong)-1];

    for(i=0; i < sizeof(ajulong); i++)
    {
        *cd = *cs++;
        --cd;
    }

    *lval = revdata.l;

    return;
}




/* @datasection [none]  Miscellaneous utility functions ***********************
**
** Miscellaneous utility functions.
**
** @nam2rule  Util           Miscellaneous utility functions.
**
******************************************************************************/




/* @section Miscellaneous utility functions ***********************************
**
** Miscellaneous utility functions.
**
** @fdata [none]
**
** @nam3rule  Base64     Base-64 encode/decode functions
** @nam4rule  Decode     Base-64 decode
** @nam4rule  Encode     Base-64 encode
** @nam3rule  Catch      Dummy function to be called in special cases so
**                       it can be used when debugging in GDB.
** @nam3rule  Get        Retrieve system information
** @nam4rule  GetBigendian  Tests whether the host system uses big endian
**                          byte order.
** @nam4rule  GetUid        Returns the user's userid.
**
** @nam3rule  Loginfo    If a log file is in use, writes run details to
**                       end of file.
** @suffix C Character string input
**
** @argrule   GetUid  Puid [AjPStr*] User's userid
** @argrule   Decode  Pdest [AjPStr*] Decoded string
** @argrule   Decode  src [const char*] Encoded input string
** @argrule   Encode  Pdest [AjPStr*] Decoded string
** @argrule   Encode  size [size_t] Length of input string
** @argrule   Encode  src [const unsigned char*] Input string
**
** @valrule   *  [void]
** @valrule   *Decode  [size_t] Length of encoded string
** @valrule   *Encode  [AjBool] True if operation was successful.
** @valrule   *Get  [AjBool] True if operation was successful.
**
** @fcategory misc
**
******************************************************************************/




/* @func ajUtilBase64DecodeC **************************************************
**
** Decode a base 64 string
**
** @param [w] Pdest [AjPStr*] Decoded string
** @param [r] src [const char*] source base64 string
** @return [size_t] Length of decoded string (zero if decode error)
**
** @release 6.4.0
** @@
******************************************************************************/

size_t ajUtilBase64DecodeC(AjPStr *Pdest, const char *src)
{
    unsigned char *buf = NULL;
    size_t srclen;
    size_t i,j;
    char c1 = 'A';
    char c2 = 'A';
    char c3 = 'A';
    char c4 = 'A';
    unsigned char b1 = 0;
    unsigned char b2 = 0;
    unsigned char b3 = 0;
    unsigned char b4 = 0;
    
    if(!src)
        return 0;

    if(!*src)
        return 0;
    
    if(!Pdest)
        return 0;

    if(!*Pdest)
        *Pdest = ajStrNew();

    ajStrSetClear(Pdest);
    
    srclen = strlen(src);
    
    buf = (unsigned char *) ajCharNewRes(srclen);

    /* Ignore non-base64 chars */
    for(i = 0, j = 0; src[i]; ++i)
        if(utilIsBase64(src[i]))
            buf[j++] = src[i];

    for(i = 0; i < j; i += 4)
    {
        c1 = buf[i];

        if(i+1 < j)
            c2 = buf[i+1];
        
        if(i+2 < j)
            c3 = buf[i+2];

        if(i+3 < j)
            c4 = buf[i+3];
    
      
      b1 = utilBase64Decode(c1);
      b2 = utilBase64Decode(c2);
      b3 = utilBase64Decode(c3);
      b4 = utilBase64Decode(c4);
      
      ajStrAppendK(Pdest, ((b1<<2)|(b2>>4)));
      
      if(c3 != '=')
          ajStrAppendK(Pdest, (((b2&0xf)<<4)|(b3>>2)));
      
      if(c4 != '=')
          ajStrAppendK(Pdest, (((b3&0x3)<<6)|b4));	
    }
    
    AJFREE(buf);
    
    return ajStrGetLen(*Pdest);
  
}




/* @func ajUtilBase64EncodeC **************************************************
**
** Decode a base 64 string
**
** @param [u] Pdest [AjPStr*] Encoded string
** @param [r] size [size_t] Size of data to encode
** @param [r] src [const unsigned char *] source data
** @return [AjBool] True on success
**
** @release 6.4.0
** @@
******************************************************************************/

AjBool ajUtilBase64EncodeC(AjPStr *Pdest, size_t size, const unsigned char *src)
{
    size_t i;
    unsigned char b1 = 0;
    unsigned char b2 = 0;
    unsigned char b3 = 0;
    unsigned char b4 = 0;
    unsigned char b5 = 0;
    unsigned char b6 = 0;
    unsigned char b7 = 0;    

    if(!src)
        return ajFalse;

    if(!Pdest)
        return ajFalse;

    if(!*Pdest)
        return ajFalse;
    
    if(!size)
        size = strlen((const char *)src);
    

    for(i = 0; i < size; i += 3)
    {
        b1 = src[i];
      
        if(i+1 < size)
            b2 = src[i+1];
      
        if(i+2 < size)
            b3 = src[i+2];
      
        b4 = b1 >> 2;
        b5 = ((b1 & 0x3) << 4) | (b2 >> 4);
        b6 = ((b2 & 0xf) << 2) | (b3 >> 6);
        b7 = b3 & 0x3f;

        ajStrAppendK(Pdest, utilBase64Encode(b4));
        ajStrAppendK(Pdest, utilBase64Encode(b5));
      
        if(i+1 < size)
            ajStrAppendK(Pdest, utilBase64Encode(b6));
        else
            ajStrAppendK(Pdest, '=');
      
        if(i+2 < size)
            ajStrAppendK(Pdest, utilBase64Encode(b7));
        else
            ajStrAppendK(Pdest, '=');
    }

    return ajTrue;
}




/* @funcstatic utilIsBase64 ***************************************************
**
** Test for valid base 64 character
**
** @param [r] c [char] Character
** @return [AjBool] True if valid base64 character
**
** @release 6.4.0
** @@
******************************************************************************/

static AjBool utilIsBase64(char c)
{

    if((c >= 'A' && c <= 'Z') || (c >= 'a' && c <= 'z') ||
       (c >= '0' && c <= '9') || (c == '+')             ||
       (c == '/')             || (c == '='))
        return ajTrue;
  
    return ajFalse;
}




/* @funcstatic utilBase64Encode ***********************************************
**
** Encode one byte to base 64
**
** @param [r] u [unsigned char] Character
** @return [char] Encoded character
**
** @release 6.4.0
** @@
******************************************************************************/

static char utilBase64Encode(unsigned char u)
{

    if(u < 26)
        return 'A' + u;

    if(u < 52)
        return 'a' + (u-26);

    if(u < 62)
        return '0' + (u-52);

    if(u == 62)
        return '+';
  
    return '/';
}




/* @funcstatic utilBase64Decode ***********************************************
**
** Decode one byte from base 64
**
** @param [r] c [char] Character
** @return [unsigned char] Decoded character
**
** @release 6.4.0
** @@
******************************************************************************/

static unsigned char utilBase64Decode(char c)
{
  
    if(c >= 'A' && c <= 'Z')
        return(c - 'A');

    if(c >= 'a' && c <= 'z')
        return(c - 'a' + 26);

    if(c >= '0' && c <= '9')
        return(c - '0' + 52);

    if(c == '+')
        return 62;
  
    return 63;
}




/* @func ajUtilCatch **********************************************************
**
** Dummy function to be called in special cases so it can be used when
** debugging in GDB.
**
** To use, simply put a call to ajUtilCatch() into your code, and use
** "break ajUtilCatch" in gdb to get a traceback.
**
** @return [void]
**
** @release 2.5.0
** @@
******************************************************************************/

void ajUtilCatch(void)
{
    static ajint calls = 0;

    calls = calls + 1;

    return;
}




/* @func ajUtilGetBigendian ***************************************************
**
** Tests whether the host system uses big endian byte order.
**
** @return [AjBool] ajTrue if host is big endian.
**
** @release 5.0.0
** @@
******************************************************************************/

AjBool ajUtilGetBigendian(void)
{
    static union lbytes
    {
        char chars[sizeof(ajint)];
        ajint i;
    } data;

    if(!utilBigendCalled)
    {
        utilBigendCalled = 1;
        data.i           = 0;
        data.chars[0]    = '\1';

        if(data.i == 1)
            utilBigendian = ajFalse;
        else
            utilBigendian = ajTrue;
    }

    return utilBigendian;
}




/* @func ajUtilGetUid *********************************************************
**
** Returns the user's userid
**
** @param [w] Puid [AjPStr*] String to return result
** @return [AjBool] ajTrue on success
**
** @release 5.0.0
** @@
******************************************************************************/

AjBool ajUtilGetUid(AjPStr* Puid)
{
#ifndef WIN32
    ajint uid;
    struct passwd* pwd;

    ajDebug("ajUtilUid\n");

    uid = getuid();

    if(!uid)
    {
        ajStrAssignClear(Puid);

        return ajFalse;
    }

    ajDebug("  uid: %d\n", uid);
    pwd = getpwuid(uid);

    if(!pwd)
    {
        ajStrAssignClear(Puid);

        return ajFalse;
    }

    ajDebug("  pwd: '%s'\n", pwd->pw_name);

    ajStrAssignC(Puid, pwd->pw_name);

    return ajTrue;

#else /* WIN32 */
    char nameBuf[UNLEN+1];
    DWORD nameLen = UNLEN+1;

    ajDebug("ajUtilUid\n");

    if (GetUserName(nameBuf, &nameLen))
    {
        ajDebug("  pwd: '%s'\n", nameBuf);
        ajStrAssignC(Puid, nameBuf);

        return ajTrue;
    }

    ajStrAssignC(Puid, "");

    return ajFalse;
#endif /* !WIN32 */
}




/* @func ajUtilLoginfo ********************************************************
**
** If a log file is in use, writes run details to end of file.
**
** @return [void]
**
** @release 5.0.0
** @@
******************************************************************************/

void ajUtilLoginfo(void)
{
    AjPFile logf;
    AjPStr logfname = NULL;
    AjPStr uids    = NULL;
    AjPTime today = NULL;
    double walltime;
    double cputime;

    today = ajTimeNewTodayFmt("log");

    if(ajNamGetValueC("logfile", &logfname))
    {
        logf = ajFileNewOutappendNameS(logfname);

        if(!logf)
            return;

        walltime = ajTimeDiff(ajTimeRefToday(),today);
        cputime = ajClockSeconds();

        if(walltime < cputime)
            walltime = cputime; /* avoid reporting 0.0 if cpu time appears */

        ajUtilGetUid(&uids),
            ajFmtPrintF(logf, "%S\t%S\t%D\t%.1f\t%.1f\n",
                        ajUtilGetProgram(),
                        uids,
                        today,
                        cputime, walltime);
        ajStrDel(&uids);
        ajStrDel(&logfname);
        ajFileClose(&logf);
    }


    ajTimeDel(&today);

    return;
}




/* @section provenance ********************************************************
**
** Functions providing information about the run-time environment
**
** @fdata [none]
**
** @nam3rule Get Return data as a string
** @nam4rule GetCmdline Return the full commandline equivalent
** @nam4rule GetInputs Return the full commandline equivalent
** @nam4rule GetProgram Return the program name
**
** @valrule * [const AjPStr]
** @fcategory misc
**
******************************************************************************/




/* @func ajUtilGetCmdline *****************************************************
**
** Returns the original command line as qualifiers and values with newline
** delimiters
**
** @return [const AjPStr] Commandline with newlines between qualifiers
** and parameters
**
** @release 6.2.0
******************************************************************************/

const AjPStr ajUtilGetCmdline (void)
{
    return acdArgSave;
}




/* @func ajUtilGetInputs ******************************************************
**
** Returns the user non-default inputs in commandline form
**
** @return [const AjPStr] Commandline with newlines between qualifiers
** and parameters
**
** @release 6.2.0
******************************************************************************/

const AjPStr ajUtilGetInputs (void)
{
    return acdInputSave;
}




/* @func ajUtilGetProgram *****************************************************
**
** Returns the application (program) name from the ACD definition.
**
** @return [const AjPStr] Program name
**
** @release 6.2.0
** @@
******************************************************************************/

const AjPStr ajUtilGetProgram(void)
{
    return acdProgram;
}




#ifdef AJ_COMPILE_DEPRECATED_BOOK
#endif /* AJ_COMPILE_DEPRECATED_BOOK */





#ifdef AJ_COMPILE_DEPRECATED
/* @obsolete ajUtilRevInt
** @rename ajByteRevInt
*/

__deprecated void ajUtilRevInt(ajint* ival)
{
    ajByteRevInt(ival);

    return;
}




/* @obsolete ajUtilRev2
** @rename ajByteRev2
*/

__deprecated void ajUtilRev2(ajshort* sval)
{
    ajByteRevLen2(sval);

    return;
}




/* @obsolete ajUtilRev4
** @rename ajByteRevLen4
*/

__deprecated void ajUtilRev4(ajint* ival)
{
    ajByteRevLen4(ival);

    return;
}




/* @obsolete ajUtilRev8
** @rename ajByteRevLen8
*/

__deprecated void ajUtilRev8(ajlong* lval)
{
    ajByteRevLen8(lval);
}




/* @obsolete ajUtilRevLong
** @rename ajByteRevLong
*/

__deprecated void ajUtilRevLong(ajlong* lval)
{
    ajByteRevLong(lval);

    return;
}




/* @obsolete ajUtilRevShort
** @rename ajByteRevShort
*/

__deprecated void ajUtilRevShort(ajshort* sval)
{
    ajByteRevShort(sval);
}




/* @obsolete ajUtilRevUint
** @rename ajByteRevUint
*/

__deprecated void ajUtilRevUint(ajuint* ival)
{
    ajByteRevUint(ival);
}




/* @obsolete ajUtilBigendian
** @rename ajUtilGetBigendian
*/
__deprecated AjBool ajUtilBigendian(void)
{
    return ajUtilGetBigendian();
}




/* @obsolete ajUtilUid
** @rename ajUtilGetUid
*/

__deprecated AjBool ajUtilUid(AjPStr* dest)
{
    return ajUtilGetUid(dest);
}




/* @obsolete ajLogInfo
** @rename ajUtilLoginfo
*/

__deprecated void ajLogInfo(void)
{
    ajUtilLoginfo();

    return;
}




/* @obsolete ajAcdGetCmdline
** @rename ajUtilGetCmdline
*/
__deprecated const AjPStr ajAcdGetCmdline (void)
{
    return ajUtilGetCmdline();
}




/* @obsolete ajAcdGetInputs
** @rename ajUtilGetInputs
*/

__deprecated const AjPStr ajAcdGetInputs (void)
{
    return ajUtilGetInputs();
}




/* @obsolete ajAcdProgramS
** @remove Use ajAcdGetProgram
*/

__deprecated void  ajAcdProgramS(AjPStr* pgm)
{
    ajStrAssignS(pgm, acdProgram);
    return;
}




/* @obsolete ajAcdGetProgram
** @rename ajUtilGetProgram
*/
__deprecated const AjPStr ajAcdGetProgram(void)
{
    return ajUtilGetProgram();
}




/* @obsolete ajAcdProgram
** @remove Use ajAcdGetProgram
*/

__deprecated const char*  ajAcdProgram(void)
{
    return ajStrGetPtr(acdProgram);
}

#endif /* AJ_COMPILE_DEPRECATED */
