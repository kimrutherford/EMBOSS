/* @source ajfileio ***********************************************************
**
** AJAX file routines
**
** @author Copyright (C) 1999 Peter Rice
** @version $Revision: 1.28 $
** @modified Peter Rice pmr@ebi.ac.uk I/O file functions from ajfile.c
** @modified $Date: 2012/03/13 13:29:28 $ by $Author: rice $
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

#include "ajfileio.h"
#include "ajutil.h"
#include "ajfile.h"

#include <string.h>
#include <errno.h>


static void   filebuffLineAdd(AjPFilebuff thys, const AjPStr line);




/* @filesection ajfile ********************************************************
**
** @nam1rule aj Function belongs to the AJAX library.
**
*/




/* @datasection [AjPFile] File object *****************************************
**
** Function is for manipulating input files and returns or takes at least
** one AjPFile argument.
** 
*/




/* @section file line read operations
**
** @fdata [AjPFile]
**
** These functions read data directly from a file using system functions.
** Integer data is by default assumed to be stored as little-endian
** so that binary files with integers are portable across systems
**
** @nam2rule Readline
** @nam3rule Trim     Remove trailing newline and linefeed characters
** @nam3rule Append   Append to existing buffer
** @suffix   Pos      Return file position after read
**
** @argrule * file [AjPFile] Input file object
** @argrule * Pdest [AjPStr*] Buffer (expanded automatically) for results
** @argrule Pos Ppos [ajlong*] File position after read
**
** @valrule * [AjBool] True on success
**
** @fcategory input
*/




/* @func ajReadline ***********************************************************
**
** Read a line from a file.
**
** @param [u] file [AjPFile] Input file.
** @param [w] Pdest [AjPStr*] Buffer to hold the current line.
** @return [AjBool] ajTrue on success.
** @category modify [AjPFile] Reads a record from a file
**
** @release 6.0.0
** @@
******************************************************************************/

AjBool ajReadline(AjPFile file, AjPStr* Pdest)
{
    ajlong fpos = 0;

    return ajReadlinePos(file, Pdest, &fpos);
}




/* @func ajReadlineAppend *****************************************************
**
** Reads a record from a file and appends it to the user supplied buffer.
**
** @param [u] file [AjPFile] Input file.
** @param [u] Pdest [AjPStr*] Buffer to hold results.
** @return [AjBool] ajTrue on success.
**
** @release 6.0.0
** @@
******************************************************************************/

AjBool ajReadlineAppend(AjPFile file, AjPStr* Pdest)
{
    static AjPStr locbuff = 0;
    AjBool ok;

    if(!locbuff)
	locbuff = ajStrNewRes(512);

    ok = ajReadline(file, &locbuff);

    if(ok)
	ajStrAppendS(Pdest, locbuff);

    ajStrDel(&locbuff);

    return ok;
}




/* @func ajReadlinePos ********************************************************
**
** Reads a line from a file.
**
** @param [u] file [AjPFile] Input file.
** @param [w] Pdest [AjPStr*] Buffer to hold the current line.
** @param [w] Ppos [ajlong*] File position before the read.
** @return [AjBool] ajTrue on success.
** @category modify [AjPFile] Reads a record from a file
**
** @release 6.0.0
** @@
******************************************************************************/

AjBool ajReadlinePos(AjPFile file, AjPStr* Pdest, ajlong* Ppos)
{
    const char *cp;
    char *buff;
    ajint isize;
    ajint ilen;
    ajint jlen;
    ajint ipos;
    ajuint buffsize;
    const char* pnewline = NULL;
#ifndef __ppc__
    size_t iread;
#endif
    
    MAJSTRDEL(Pdest);

    if(file->Buffsize)
        buffsize = file->Buffsize;
    else
        buffsize = ajFileValueBuffsize();

    if(!file->Buff)
      ajStrAssignResC(&file->Buff, buffsize, "");
    else if(buffsize > MAJSTRGETRES(file->Buff))
      ajStrSetRes(&file->Buff, buffsize);

    if(MAJSTRGETUSE(file->Buff) == 1)
      buff = MAJSTRGETPTR(file->Buff);
    else
      buff  = ajStrGetuniquePtr(&file->Buff);

    isize = MAJSTRGETRES(file->Buff);
    ilen  = 0;
    ipos  = 0;
    
    if(!file->fp)
	ajWarn("ajReadlinePos file not found");
    
    *Ppos = file->Filepos;

    while(buff)
    {
	if(file->End)
	{
	    ajStrAssignClear(Pdest);
	    ajDebug("at EOF: File already read to end %F\n", file);

	    return ajFalse;
	}
	

#ifndef __ppc__
        if(file->Readblock)
        {
            if(file->Blockpos >= file->Blocklen)
            {
                iread = fread(file->Readblock,
                              1, file->Blocksize,
                              file->fp);

                if(!iread && ferror(file->fp))
                    ajFatal("fread failed with error:%d '%s'",
                            ferror(file->fp), strerror(ferror(file->fp)));

                file->Blockpos = 0;
                file->Blocklen = iread;
                file->Readblock[iread] = '\0';
                /*ajDebug("++ fread %u Ppos:%Ld\n", iread, *Ppos);*/
             }

            if(file->Blockpos < file->Blocklen)
            {

                /* we know we have something in Readblock to process */

                pnewline = strchr(&file->Readblock[file->Blockpos], '\n');

                if(pnewline)
                    jlen = pnewline - &file->Readblock[file->Blockpos] + 1;
                else
                    jlen = file->Blocklen - file->Blockpos;

                /*ajDebug("ipos:%d jlen:%d pnewline:%p "
                          "Readblock:%p blockpos:%d blocklen:%d\n",
                          ipos, jlen, pnewline, file->Readblock,
                          file->Blockpos, file->Blocklen);*/
                memmove(&buff[ipos], &file->Readblock[file->Blockpos], jlen);
                buff[ipos+jlen]='\0';
                cp = &buff[ipos];
                file->Blockpos += jlen;
            }
            else
            {
                jlen = 0;
                cp = NULL;
            }
        }
        else
        {
            cp = fgets(&buff[ipos], isize, file->fp);
            jlen = strlen(&buff[ipos]);
        }
        
#else
	cp = ajSysFuncFgets(&buff[ipos], isize, file->fp);
	jlen = strlen(&buff[ipos]);
#endif

        if(!cp && !ipos)
	{
	    if(feof(file->fp))
	    {
		file->End = ajTrue;
		ajStrAssignClear(Pdest);
		ajDebug("EOF ajFileGetsL file %F\n", file);

		return ajFalse;
	    }
	    else
            {
		ajFatal("Error reading from file '%S' ferror:%d %d: %s\n",
                        ajFileGetNameS(file),
                        ferror(file->fp),
                        errno, strerror(errno));
            }
	}

	ilen += jlen;
        file->Filepos += jlen;

	/*
	 ** We need to read again if:
	 ** We have read the entire buffer
	 ** and we don't have a newline at the end
	 ** (must be careful about that - we may just have read enough)
	 */

	if(((file->Readblock && !pnewline) ||(jlen == (isize-1))) &&
	   (buff[ilen-1] != '\n'))
	{
            MAJSTRSETVALIDLEN(&file->Buff, ilen); /* fix before resizing! */
	    ajStrSetResRound(&file->Buff, ilen+buffsize+1);
	    /*ajDebug("more to do: jlen: %d ipos: %d isize: %d ilen: %d "
		    "Size: %d\n",
		    jlen, ipos, isize, ilen, ajStrGetRes(file->Buff));*/
	    ipos += jlen;
	    buff = ajStrGetuniquePtr(&file->Buff);
	    isize = ajStrGetRes(file->Buff) - ipos;
	    /*ajDebug("expand to: ipos: %d isize: %d Size: %d\n",
              ipos, isize, ajStrGetRes(file>Buff));*/
	}
	else
	{
            buff = NULL;
        }
    }
    
    MAJSTRSETVALIDLEN(&file->Buff, ilen);
    if (ajStrGetCharLast(file->Buff) != '\n')
    {
	/*ajDebug("Appending missing newline to '%S'\n", file->Buff);*/
	ajStrAppendK(&file->Buff, '\n');
    }
    ajStrAssignRef(Pdest, file->Buff);

/*
  if(file->Readblock)
        ajDebug("ajFileGetsL done blocklen:%d blockpos:%d readlen:%u\n",
                file->Blocklen, file->Blockpos, ajStrGetLen(file->Buff));
*/
    return ajTrue;
}




/* @func ajReadlineTrim *******************************************************
**
** Reads a line from a file and removes any trailing newline.
**
** @param [u] file [AjPFile] Input file.
** @param [w] Pdest [AjPStr*] Buffer to hold the current line.
** @return [AjBool] ajTrue on success.
** @category modify [AjPFile] Reads a record from a file and removes
**                            newline characters
**
** @release 6.0.0
** @@
******************************************************************************/

AjBool ajReadlineTrim(AjPFile file, AjPStr* Pdest)
{
    ajlong fpos=0;

    return ajReadlineTrimPos(file, Pdest, &fpos);
}




/* @func ajReadlineTrimPos ****************************************************
**
** Reads a line from a file and removes any trailing newline.
**
** @param [u] file [AjPFile] Input file.
** @param [w] Pdest [AjPStr*] Buffer to hold the current line.
** @param [w] Ppos [ajlong*] File position before the read.
** @return [AjBool] ajTrue on success.
** @category modify [AjPFile] Reads a record from a file and removes
**                            newline characters
**
** @release 6.0.0
** @@
******************************************************************************/

AjBool ajReadlineTrimPos(AjPFile file, AjPStr* Pdest, ajlong* Ppos)
{
    AjBool ok;

    ok = ajReadlinePos(file, Pdest, Ppos);

    if(!ok)
	return ajFalse;

    MAJSTRDEL(Pdest);

    /* trim any trailing newline */


    /*ajDebug("Remove carriage-return characters from PC-style files\n");*/
    if(ajStrGetCharLast(file->Buff) == '\n')
	ajStrCutEnd(&file->Buff, 1);

    /* PC files have \r\n Macintosh files have just \r : this fixes both */
    if(ajStrGetCharLast(file->Buff) == '\r')
	ajStrCutEnd(&file->Buff, 1);

    ajStrAssignRef(Pdest, file->Buff);

    return ajTrue;
}




/* @section file binary read operations
**
** @fdata [AjPFile]
**
** These functions read data directly from a file using system functions.
** Integer data is by default assumed to be stored as little-endian
** so that binary files with integers are portable across systems
**
** @nam2rule Readbin       
** @nam3rule Binary Binary read into a buffer
** @nam3rule Char Binary read of a character string
** @nam4rule CharTrim Trim trailing space from a character string
** @nam3rule Int Binary read of an integer
** @nam3rule Int2 Binary read of a 2 byte integer
** @nam3rule Int4 Binary read of a 4 byte integer
** @nam3rule Int8 Binary read of an 8 byte integer
** @nam3rule Uint Binary read of an unsigned integer
** @nam3rule Uint2 Binary read of a 2 byte unsigned integer
** @nam3rule Uint4 Binary read of a 4 byte unsigned integer
** @nam3rule Uint8 Binary read of an 8 byte unsigned integer
** @nam3rule Str Binary read of a string
** @nam4rule StrTrim Trim trailing space from a string
** @suffix Endian Data in file is big-endian
** @suffix Local Data in file  is in local endian-ness
**
** @argrule Readbin file [AjPFile] File object
** @argrule Binary count [size_t] Number of elements to read
** @argrule Binary size [size_t] Size of each element
** @argrule Binary buffer [void*] Buffer for binary read
** @argrule Char size [size_t] Size of each element
** @argrule Char buffer [char*] Buffer for binary read
** @argrule Str size [size_t] Size of each element
** @argrule Str Pstr [AjPStr*] Buffer for binary read
**
** @argrule Int Pi [ajint*] Integer value
** @argrule Int2 Pi2 [ajshort*] Integer 2 byte value
** @argrule Int4 Pi4 [ajint*] Integer 4 byte value
** @argrule Int8 Pi8 [ajlong*] Integer 8 byte value
** @argrule Uint Pu [ajuint*] Unsigned integer value
** @argrule Uint2 Pu2 [ajushort*] Unsigned integer 2 byte value
** @argrule Uint4 Pu4 [ajuint*] Unsigned integer 4 byte value
** @argrule Uint8 Pu8 [ajulong*] Unsigned integer 8 byte value
**
** @valrule * [size_t] Number of elements read. Zero for failure.
**
** @fcategory input
**
******************************************************************************/




/* @func ajReadbinBinary ******************************************************
**
** Binary read from an input file object using the C 'fread' function.
**
** @param [u] file [AjPFile] Input file.
** @param [r] count [size_t] Number of elements to read.
** @param [r] size [size_t] Number of bytes per element.
** @param [w] buffer [void*] Buffer for output.
** @return [size_t] Return value from 'fread'
**
** @release 6.0.0
** @@
******************************************************************************/

size_t ajReadbinBinary(AjPFile file, size_t count,size_t size,
		   void* buffer)
{
    return fread(buffer, size, count, file->fp);
}




/* @func ajReadbinChar ********************************************************
**
** Reads a character string from a file
**
** @param [u] file [AjPFile] File object.
** @param [r] size[size_t] Number of bytes to read from index file.
** @param [w] buffer[char*] Buffer to read into
** @return [size_t] Number of bytes read.
**
** @release 6.0.0
** @@
******************************************************************************/

size_t ajReadbinChar(AjPFile file, size_t size, 
                     char* buffer)
{
    size_t ret;

    ret = fread(buffer, 1, size, file->fp);

    return ret;
}




/* @func ajReadbinCharTrim ****************************************************
**
** Reads a character string from a file and trims trailing spaces
**
** @param [u] file [AjPFile] File object.
** @param [r] size[size_t] Number of bytes to read from index file.
** @param [w] buffer[char*] Buffer to read into
** @return [size_t] Number of bytes read.
**
** @release 6.0.0
** @@
******************************************************************************/

size_t ajReadbinCharTrim(AjPFile file, size_t size,
                             char* buffer)
{
    size_t ret;
    char* sp;

    ret = fread(buffer, 1, size, file->fp);

    buffer[size] = '\0';
    sp = &buffer[strlen(buffer)];

    while(sp > buffer)
    {
        sp--;

	if(*sp != ' ')
	    break;

	*sp = '\0';
    }

    return ret;
}




/* @func ajReadbinInt *********************************************************
**
** Binary read of an unsigned integer from an input file object using
** the C 'fread' function. Converts from little-endian.
**
** @param [u] file [AjPFile] Input file.
** @param [w] Pi [ajint*] Integer value
** @return [size_t] Number of bytes read.
**
** @release 6.0.0
** @@
******************************************************************************/

size_t ajReadbinInt(AjPFile file, ajint *Pi)
{
    size_t ret;

    ret = fread(Pi, 4, 1, file->fp);

#ifdef WORDS_BIGENDIAN
    ajByteRevLen4(Pi);
#endif
    
    return ret;
}




/* @func ajReadbinIntEndian ***************************************************
**
** Binary read of an unsigned integer from an input file object using
** the C 'fread' function. Converts from big-endian.
**
** @param [u] file [AjPFile] Input file.
** @param [w] Pi [ajint*] Integer value
** @return [size_t] Number of bytes read.
**
** @release 6.0.0
** @@
******************************************************************************/

size_t ajReadbinIntEndian(AjPFile file, ajint *Pi)
{
    size_t ret;

    
    ret = fread(Pi, 4, 1, file->fp);

#ifndef WORDS_BIGENDIAN
    ajByteRevLen4(Pi);
#endif

    return ret;
}




/* @func ajReadbinIntLocal ****************************************************
**
** Binary read of an unsigned integer from an input file object using
** the C 'fread' function. No conversion (assumes integer was written
** on the same system).
**
** @param [u] file [AjPFile] Input file.
** @param [w] Pi [ajint*] Integer value
** @return [size_t] Number of bytes read.
**
** @release 6.0.0
** @@
******************************************************************************/

size_t ajReadbinIntLocal(AjPFile file, ajint *Pi)
{
    size_t ret;

    ret = fread(Pi, 4, 1, file->fp);

    return ret;
}




/* @func ajReadbinInt2 ********************************************************
**
** Binary read of a 2 byte integer from an input file object using
** the C 'fread' function. Converts from little-endian.
**
** @param [u] file [AjPFile] Input file.
** @param [w] Pi2 [ajshort*] Integer value
** @return [size_t] Number of bytes read.
**
** @release 6.0.0
** @@
******************************************************************************/

size_t ajReadbinInt2(AjPFile file, ajshort *Pi2)
{
    size_t ret;

    ret = fread(Pi2, 2, 1, file->fp);

#ifdef WORDS_BIGENDIAN
    ajByteRevLen2(Pi2);
#endif
    
    return ret;
}




/* @func ajReadbinInt2Endian **************************************************
**
** Binary read of an unsigned integer from an input file object using
** the C 'fread' function. Converts from big-endian.
**
** @param [u] file [AjPFile] Input file.
** @param [w] Pi2 [ajshort*] Integer value
** @return [size_t] Number of bytes read.
**
** @release 6.0.0
** @@
******************************************************************************/

size_t ajReadbinInt2Endian(AjPFile file, ajshort *Pi2)
{
    size_t ret;

    ret = fread(Pi2, 2, 1, file->fp);

#ifndef WORDS_BIGENDIAN
    ajByteRevLen2(Pi2);
#endif
    
    return ret;
}




/* @func ajReadbinInt2Local ***************************************************
**
** Binary read of an unsigned integer from an input file object using
** the C 'fread' function. No conversion (assumes integer was written
** on the same system).
**
** @param [u] file [AjPFile] Input file.
** @param [w] Pi2 [ajshort*] Integer value
** @return [size_t] Number of bytes read.
**
** @release 6.0.0
** @@
******************************************************************************/

size_t ajReadbinInt2Local(AjPFile file, ajshort *Pi2)
{
    size_t ret;

    ret = fread(Pi2, 2, 1, file->fp);

    return ret;
}




/* @func ajReadbinInt4 ********************************************************
**
** Binary read of a 4 byte integer from an input file object using
** the C 'fread' function.
**
** @param [u] file [AjPFile] Input file.
** @param [w] Pi4 [ajint*] Integer value
** @return [size_t] Number of bytes read.
**
** @release 6.0.0
** @@
******************************************************************************/

size_t ajReadbinInt4(AjPFile file, ajint *Pi4)
{
    size_t ret;

    ret = fread(Pi4, 4, 1, file->fp);

#ifdef WORDS_BIGENDIAN
    ajByteRevLen4(Pi4);
#endif
    
    return ret;
}




/* @func ajReadbinInt4Endian **************************************************
**
** Binary read of an unsigned integer from an input file object using
** the C 'fread' function. Converts from big-endian.
**
** @param [u] file [AjPFile] Input file.
** @param [w] Pi4 [ajint*] Integer value
** @return [size_t] Number of bytes read.
**
** @release 6.0.0
** @@
******************************************************************************/

size_t ajReadbinInt4Endian(AjPFile file, ajint *Pi4)
{
    size_t ret;

    ret = fread(Pi4, 4, 1, file->fp);

#ifndef WORDS_BIGENDIAN
    ajByteRevLen4(Pi4);
#endif

    return ret;
}




/* @func ajReadbinInt4Local ***************************************************
**
** Binary read of an unsigned integer from an input file object using
** the C 'fread' function. No conversion (assumes integer was written
** on the same system).
**
** @param [u] file [AjPFile] Input file.
** @param [w] Pi4 [ajint*] Integer value
** @return [size_t] Number of bytes read.
**
** @release 6.0.0
** @@
******************************************************************************/

size_t ajReadbinInt4Local(AjPFile file, ajint *Pi4)
{
    size_t ret;

    ret = fread(Pi4, 4, 1, file->fp);

    return ret;
}




/* @func ajReadbinInt8 ********************************************************
**
** Binary read of an 8 byte integer from an input file object using
** the C 'fread' function. Converts from little-endian.
**
** @param [u] file [AjPFile] Input file.
** @param [w] Pi8 [ajlong*] Integer value
** @return [size_t] Number of bytes read.
**
** @release 6.0.0
** @@
******************************************************************************/

size_t ajReadbinInt8(AjPFile file, ajlong *Pi8)
{
    long ret;

    ret = fread(Pi8, 8, 1, file->fp);

#ifdef WORDS_BIGENDIAN
    ajByteRevLen8(Pi8);
#endif
    
    return ret;
}




/* @func ajReadbinInt8Endian **************************************************
**
** Binary read of an unsigned integer from an input file object using
** the C 'fread' function. Converts from big-endian.
**
** @param [u] file [AjPFile] Input file.
** @param [w] Pi8 [ajlong*] Integer value
** @return [size_t] Number of bytes read.
**
** @release 6.0.0
** @@
******************************************************************************/

size_t ajReadbinInt8Endian(AjPFile file, ajlong *Pi8)
{
    size_t ret;

    ret = fread(Pi8, 8, 1, file->fp);

#ifndef WORDS_BIGENDIAN
    ajByteRevLen8(Pi8);
#endif
    
    return ret;
}




/* @func ajReadbinInt8Local ***************************************************
**
** Binary read of an unsigned integer from an input file object using
** the C 'fread' function. No conversion (assumes integer was written
** on the same system).
**
** @param [u] file [AjPFile] Input file.
** @param [w] Pi8 [ajlong*] Integer value
** @return [size_t] Number of bytes read.
**
** @release 6.0.0
** @@
******************************************************************************/

size_t ajReadbinInt8Local(AjPFile file, ajlong *Pi8)
{
    size_t ret;

    ret = fread(Pi8, 8, 1, file->fp);

    return ret;
}




/* @func ajReadbinStr *********************************************************
**
** Reads a string from a file
**
** @param [u] file [AjPFile] File object.
** @param [r] size [size_t] Number of bytes to read from index file.
** @param [w] Pstr [AjPStr*] Buffer to read into
** @return [size_t] Number of bytes read.
**
** @release 6.5.0
** @@
******************************************************************************/

size_t ajReadbinStr(AjPFile file, size_t size, 
                    AjPStr* Pstr)
{
    size_t ret;
    char* cp;

    ajStrSetRes(Pstr, size+1);
    cp = ajStrGetuniquePtr(Pstr);

    ret = fread(cp, 1, size, file->fp);
    cp[size] = '\0';

    ajStrSetValid(Pstr);

    return ret;
}




/* @func ajReadbinStrTrim *****************************************************
**
** Reads a character string from a file and trims trailing spaces
**
** @param [u] file [AjPFile] File object.
** @param [r] size [size_t] Number of bytes to read from index file.
** @param [w] Pstr [AjPStr*] Buffer to read into
** @return [size_t] Number of bytes read.
**
** @release 6.5.0
** @@
******************************************************************************/

size_t ajReadbinStrTrim(AjPFile file, size_t size,
                        AjPStr* Pstr)
{
    size_t ret;
    char* cp;

    ajStrSetRes(Pstr, size+1);
    cp = ajStrGetuniquePtr(Pstr);

    ret = fread(cp, 1, size, file->fp);
    cp[size] = '\0';

    ajStrSetValid(Pstr);

    ajStrTrimEndC(Pstr, " ");

    return ret;
}




/* @func ajReadbinUint ********************************************************
**
** Binary read of an unsigned integer from an input file object using
** the C 'fread' function.
**
** @param [u] file [AjPFile] Input file.
** @param [w] Pu [ajuint*] Unsigned integer value
** @return [size_t] Number of bytes read.
**
** @release 6.0.0
** @@
******************************************************************************/

size_t ajReadbinUint(AjPFile file, ajuint *Pu)
{
    size_t ret;
#ifdef WORDS_BIGENDIAN
    ajint val2;
#endif

    ret = fread(Pu, 4, 1, file->fp);

#ifdef WORDS_BIGENDIAN
    val2 = *Pu;
    ajByteRevLen4(&val2);
    *Pu = val2;
#endif
    
    return ret;
}




/* @func ajReadbinUintEndian **************************************************
**
** Binary read of an unsigned integer from an input file object using
** the C 'fread' function. Converts from big-endian.
**
** @param [u] file [AjPFile] Input file.
** @param [w] Pu [ajuint*] Unsigned integer value
** @return [size_t] Number of bytes read.
**
** @release 6.0.0
** @@
******************************************************************************/

size_t ajReadbinUintEndian(AjPFile file, ajuint *Pu)
{
    ajuint ret;
#ifndef WORDS_BIGENDIAN
    ajint val2;
#endif

    ret = fread(Pu, 4, 1, file->fp);

#ifndef WORDS_BIGENDIAN
    val2 = *Pu;
    ajByteRevLen4(&val2);
    *Pu = val2;
#endif
    
    return ret;
}




/* @func ajReadbinUintLocal ***************************************************
**
** Binary read of an unsigned integer from an input file object using
** the C 'fread' function. No conversion (assumes integer was written
** on the same system).
**
** @param [u] file [AjPFile] Input file.
** @param [w] Pu [ajuint*] Unsigned integer value
** @return [size_t] Number of bytes read.
**
** @release 6.0.0
** @@
******************************************************************************/

size_t ajReadbinUintLocal(AjPFile file, ajuint *Pu)
{
    ajuint ret;

    ret = fread(Pu, 4, 1, file->fp);
    
    return ret;
}




/* @func ajReadbinUint2 *******************************************************
**
** Binary read of an unsigned integer from an input file object using
** the C 'fread' function. Converts from little-endian.
**
** @param [u] file [AjPFile] Input file.
** @param [w] Pu2 [ajushort*] Unsigned integer value
** @return [size_t] Number of bytes read.
**
** @release 6.0.0
** @@
******************************************************************************/

size_t ajReadbinUint2(AjPFile file, ajushort *Pu2)
{
    size_t ret;
#ifdef WORDS_BIGENDIAN
    ajshort val2;
#endif

    ret = fread(Pu2, 2, 1, file->fp);

#ifdef WORDS_BIGENDIAN
    val2 = *Pu2;
    ajByteRevLen2(&val2);
    *Pu2 = val2;
#endif
    
    return ret;
}




/* @func ajReadbinUint2Endian *************************************************
**
** Binary read of an unsigned integer from an input file object using
** the C 'fread' function. Converts from big-endian.
**
** @param [u] file [AjPFile] Input file.
** @param [w] Pu2 [ajushort*] Unsigned integer value
** @return [size_t] Number of bytes read.
**
** @release 6.0.0
** @@
******************************************************************************/

size_t ajReadbinUint2Endian(AjPFile file, ajushort *Pu2)
{
    size_t ret;
#ifndef WORDS_BIGENDIAN
    ajshort val2;
#endif

    ret = fread(Pu2, 2, 1, file->fp);

#ifndef WORDS_BIGENDIAN
    val2 = *Pu2;
    ajByteRevLen2(&val2);
    *Pu2 = val2;
#endif
    
    return ret;
}




/* @func ajReadbinUint2Local **************************************************
**
** Binary read of an unsigned integer from an input file object using
** the C 'fread' function. No conversion (assumes integer was written
** on the same system).
**
** @param [u] file [AjPFile] Input file.
** @param [w] Pu2 [ajushort*] Unsigned integer value
** @return [size_t] Number of bytes read.
**
** @release 6.0.0
** @@
******************************************************************************/

size_t ajReadbinUint2Local(AjPFile file, ajushort *Pu2)
{
    size_t ret;

    ret = fread(Pu2, 2, 1, file->fp);

    return ret;
}




/* @func ajReadbinUint4 *******************************************************
**
** Binary read of an unsigned integer from an input file object using
** the C 'fread' function. Converts from little-endian.
**
** @param [u] file [AjPFile] Input file.
** @param [w] Pu4 [ajuint*] Unsigned integer value
** @return [size_t] Number of bytes read.
**
** @release 6.0.0
** @@
******************************************************************************/

size_t ajReadbinUint4(AjPFile file, ajuint *Pu4)
{
    size_t ret;
#ifdef WORDS_BIGENDIAN
    ajint val2;
#endif

    ret = fread(Pu4, 4, 1, file->fp);

#ifdef WORDS_BIGENDIAN
    val2 = *Pu4;
    ajByteRevLen4(&val2);
    *Pu4 = val2;
#endif
    
    return ret;
}




/* @func ajReadbinUint4Endian *************************************************
**
** Binary read of an unsigned integer from an input file object using
** the C 'fread' function. Converts from a big-endian.
**
** @param [u] file [AjPFile] Input file.
** @param [w] Pu4 [ajuint*] Unsigned integer value
** @return [size_t] Number of bytes read.
**
** @release 6.0.0
** @@
******************************************************************************/

size_t ajReadbinUint4Endian(AjPFile file, ajuint *Pu4)
{
    size_t ret;
#ifndef WORDS_BIGENDIAN
    ajint val2;
#endif

    ret = fread(Pu4, 4, 1, file->fp);

#ifndef WORDS_BIGENDIAN
    val2 = *Pu4;
    ajByteRevLen4(&val2);
    *Pu4 = val2;
#endif
    
    return ret;
}




/* @func ajReadbinUint4Local **************************************************
**
** Binary read of an unsigned integer from an input file object using
** the C 'fread' function. No conversion (assumes integer was written
** on the same system).
**
** @param [u] file [AjPFile] Input file.
** @param [w] Pu4 [ajuint*] Unsigned integer value
** @return [size_t] Number of bytes read.
**
** @release 6.0.0
** @@
******************************************************************************/

size_t ajReadbinUint4Local(AjPFile file, ajuint *Pu4)
{
    size_t ret;

    ret = fread(Pu4, 4, 1, file->fp);

    return ret;
}




/* @func ajReadbinUint8 *******************************************************
**
** Binary read of an unsigned integer from an input file object using
** the C 'fread' function. Converts from little-endian.
**
** @param [u] file [AjPFile] Input file.
** @param [w] Pu8 [ajulong*] Unsigned long integer value
** @return [size_t] Number of bytes read.
**
** @release 6.0.0
** @@
******************************************************************************/

size_t ajReadbinUint8(AjPFile file, ajulong *Pu8)
{
    size_t ret;
#ifdef WORDS_BIGENDIAN
    ajlong val2;
#endif

    ret = fread(Pu8, 8, 1, file->fp);

#ifdef WORDS_BIGENDIAN
    val2 = *Pu8;
    ajByteRevLen8(&val2);
    *Pu8 = val2;
#endif
    
    return ret;
}




/* @func ajReadbinUint8Endian *************************************************
**
** Binary read of an unsigned integer from an input file object using
** the C 'fread' function. Converts from big-endian.
**
** @param [u] file [AjPFile] Input file.
** @param [w] Pu8 [ajulong*] Unsigned long integer value
** @return [size_t] Number of bytes read.
**
** @release 6.0.0
** @@
******************************************************************************/

size_t ajReadbinUint8Endian(AjPFile file, ajulong *Pu8)
{
    size_t ret;
#ifndef WORDS_BIGENDIAN
    ajlong val2;
#endif

    ret = fread(Pu8, 8, 1, file->fp);

#ifndef WORDS_BIGENDIAN
    val2 = (ajlong) *Pu8;
    ajByteRevLen8(&val2);
    *Pu8 = val2;
#endif

    return ret;
}




/* @func ajReadbinUint8Local **************************************************
**
** Binary read of an unsigned integer from an input file object using
** the C 'fread' function. No conversion (assumes integer was written
** on the same system).
**
** @param [u] file [AjPFile] Input file.
** @param [w] Pu8 [ajulong*] Unsigned long integer value
** @return [size_t] Number of bytes read.
**
** @release 6.0.0
** @@
******************************************************************************/

size_t ajReadbinUint8Local(AjPFile file, ajulong *Pu8)
{
    size_t ret;

    ret = fread(Pu8, 8, 1, file->fp);

    return ret;
}




/* @datasection [AjPFile] File object *****************************************
**
** Function is for manipulating files open for output and returns or
** takes at least one AjSFile argument.
** 
*/




/* @section file binary write operations
**
** @fdata [AjPFile]
**
** These functions write data directly to a file using system functions
**
** @nam2rule Writebin Write binary data to a file
** @nam3rule Binary Binary write from a buffer
** @nam3rule Byte Write a single byte (character)
** @nam3rule Char Write a C character string
** @nam3rule Int Binary read of an integer
** @nam3rule Int2 Binary read of a 2 byte integer
** @nam3rule Int4 Binary read of a 4 byte integer
** @nam3rule Int8 Binary read of an 8 byte integer
** @nam3rule Uint2 Binary read of a 2 byte unsigned integer
** @nam3rule Uint4 Binary read of a 4 byte unsigned integer
** @nam3rule Uint8 Binary read of an 8 byte unsigned integer
** @nam3rule Newline Write newline character(s)
** @nam3rule Str Write a string object
**
** @argrule Writebin file [AjPFile] Output file object
** @argrule Binary count [size_t] Number of elements to write
** @argrule Binary size [size_t] Size of each element
** @argrule Binary buffer [const void*] Buffer for binary write
** @argrule Byte ch [char] Byte to be written
** @argrule Char txt [const char*] Character string (length passed separately)
** @argrule Char len [size_t] Character string length to be written
** @argrule Int i [ajint] Integer value
** @argrule Int2 i2 [ajshort] Integer 2 byte value
** @argrule Int4 i4 [ajint] Integer 4 byte value
** @argrule Int8 i8 [ajlong] Integer 8 byte value
** @argrule Uint2 u2 [ajushort] Integer 2 byte unsigned value
** @argrule Uint4 u4 [ajuint] Integer 4 byte unsigned value
** @argrule Uint8 u8 [ajulong] Integer 8 byte unsigned value
** @argrule Uint u [ajuint] Unsigned integer value
** @argrule Str str [const AjPStr] String (length passed separately)
** @argrule Str len [size_t] String length to be written
**
** @valrule * [size_t] Number of elements written
**
** @fcategory output
**
******************************************************************************/




/* @func ajWritebinBinary *****************************************************
**
** Binary write to an output file object using the C 'fwrite' function.
**
** @param [u] file [AjPFile] Output file.
** @param [r] count [size_t] Number of elements to write.
** @param [r] size [size_t] Number of bytes per element.
** @param [r] buffer [const void*] Buffer for output.
** @return [size_t] Return value from 'fwrite'
**
** @release 6.0.0
** @@
******************************************************************************/

size_t ajWritebinBinary(AjPFile file, size_t count,
		   size_t size, const void* buffer)
{
    return fwrite(buffer, size, count, file->fp);
}




/* @func ajWritebinByte *******************************************************
**
** Writes a single byte to a binary file
**
** @param [u] file [AjPFile] Output file
** @param [r] ch [char] Character
** @return [size_t] Return value from fwrite
**
** @release 6.0.0
** @@
******************************************************************************/

size_t ajWritebinByte(AjPFile file, char ch)
{
    return fwrite(&ch, 1, 1, file->fp);
}




/* @func ajWritebinChar *******************************************************
**
** Writes a text string to a binary file
**
** @param [u] file [AjPFile] Output file
** @param [r] txt [const char*] Text string
** @param [r] len [size_t] Length (padded) to write to the file
** @return [size_t] Return value from fwrite
**
** @release 6.0.0
** @@
******************************************************************************/

size_t ajWritebinChar(AjPFile file, const char* txt, size_t len)
{
    size_t ret;
    size_t i;
    size_t j;
    
    i = strlen(txt) + 1;

    if(i >= len)
        return fwrite(txt, len, 1, file->fp);

    ret = fwrite(txt, i, 1, file->fp);

    j = len-i;

    for(i=0;i<j;i++)
        fwrite("", 1, 1, file->fp);

    return ret;
}




/* @func ajWritebinInt2 *******************************************************
**
** Writes a 2 byte integer to a binary file, with the correct byte orientation
**
** @param [u] file [AjPFile] Output file
** @param [r] i2 [ajshort] Integer
** @return [size_t] Return value from fwrite
**
** @release 6.0.0
** @@
******************************************************************************/

size_t ajWritebinInt2(AjPFile file, ajshort i2)
{
#ifdef WORDS_BIGENDIAN
    short j;

    j = i2;
    ajByteRevLen2(&j);

    return fwrite(&j, 2, 1, file->fp);
#else
    return fwrite(&i2, 2, 1, file->fp);
#endif

}




/* @func ajWritebinInt4 *******************************************************
**
** Writes a 4 byte integer to a binary file, with the correct byte orientation
**
** @param [u] file [AjPFile] Output file
** @param [r] i4 [ajint] Integer
** @return [size_t] Return value from fwrite
**
** @release 6.0.0
** @@
******************************************************************************/

size_t ajWritebinInt4(AjPFile file, ajint i4)
{
#ifdef WORDS_BIGENDIAN
    ajint j;

    j = i4;

    ajByteRevLen4(&j);

    return fwrite(&j, 4, 1, file->fp);
#else
    return fwrite(&i4, 4, 1, file->fp);
#endif
}




/* @func ajWritebinInt8 *******************************************************
**
** Writes an 8 byte long to a binary file, with the correct byte orientation
**
** @param [u] file [AjPFile] Output file
** @param [r] i8 [ajlong] Integer
** @return [size_t] Return value from fwrite
**
** @release 6.0.0
** @@
******************************************************************************/

size_t ajWritebinInt8(AjPFile file, ajlong i8)
{
#ifdef WORDS_BIGENDIAN
    ajlong j;

    j = i8;

    ajByteRevLen8(&j);

    return fwrite(&j, 8, 1, file->fp);
#else
    return fwrite(&i8, 8, 1, file->fp);
#endif
}




/* @func ajWritebinNewline ****************************************************
**
** Writes newline character(s) to a file
**
** @param [u] file [AjPFile] Output file
** @return [size_t] Return value from fwrite
**
** @release 6.2.0
******************************************************************************/

size_t ajWritebinNewline(AjPFile file)
{
    size_t ret;

#ifdef WIN32
    /*ret = fwrite("\r\n", 1, 2, file->fp);*/
    ret = fwrite("\n", 1, 1, file->fp);
#else
    ret = fwrite("\n", 1, 1, file->fp);
#endif

    return ret;
}




/* @func ajWritebinStr ********************************************************
**
** Writes a string to a binary file
**
** @param [u] file [AjPFile] Output file
** @param [r] str [const AjPStr] String
** @param [r] len [size_t] Length (padded) to use in the file
** @return [size_t] Return value from fwrite
**
** @release 6.0.0
** @@
******************************************************************************/

size_t ajWritebinStr(AjPFile file, const AjPStr str, size_t len)
{
    size_t ret;
    ajuint ilen;
    ajuint i;
    ajuint j;
    ajuint k;

    ilen = 1+ajStrGetLen(str);

    if(ilen >= len)
        return fwrite(ajStrGetPtr(str), len, 1, file->fp);

    ret = fwrite(ajStrGetPtr(str), ilen, 1, file->fp);

    j = len - ilen;

    k = 20;

    for(i=0;i<j;i+=20)
    {
        if((j-i) < 20)
            k = j-i;
        fwrite("\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0",
               k, 1, file->fp);
    }

    return ret;
}




/* @func ajWritebinUint2 ******************************************************
**
** Writes a 2 byte unsigned integer to a binary file, with the correct
** byte orientation
**
** @param [u] file [AjPFile] Output file
** @param [r] u2 [ajushort] Unsigned integer
** @return [size_t] Return value from fwrite
**
** @release 6.0.0
** @@
******************************************************************************/

size_t ajWritebinUint2(AjPFile file, ajushort u2)
{
#ifdef WORDS_BIGENDIAN
    unsigned short j;

    j = u2;

    ajByteRevLen2u(&j);

    return fwrite(&j, 2, 1, file->fp);
#else
    return fwrite(&u2, 2, 1, file->fp);
#endif

}




/* @func ajWritebinUint4 ******************************************************
**
** Writes a 4 byte unsigned integer to a binary file, with the correct
** byte orientation
**
** @param [u] file [AjPFile] Output file
** @param [r] u4 [ajuint] Unsigned integer
** @return [size_t] Return value from fwrite
**
** @release 6.5.0
** @@
******************************************************************************/

size_t ajWritebinUint4(AjPFile file, ajuint u4)
{
#ifdef WORDS_BIGENDIAN
    ajuint j;

    j = u4;

    ajByteRevLen4u(&j);

    return fwrite(&j, 4, 1, file->fp);
#else
    return fwrite(&u4, 4, 1, file->fp);
#endif
}




/* @func ajWritebinUint8 ******************************************************
**
** Writes an 8 byte unsigned long to a binary file, with the correct
** byte orientation
**
** @param [u] file [AjPFile] Output file
** @param [r] u8 [ajulong] Unsigned integer
** @return [size_t] Return value from fwrite
**
** @release 6.5.0
** @@
******************************************************************************/

size_t ajWritebinUint8(AjPFile file, ajulong u8)
{
#ifdef WORDS_BIGENDIAN
    ajulong j;

    j = u8;

    ajByteRevLen8u(&j);

    return fwrite(&j, 8, 1, file->fp);
#else
    return fwrite(&u8, 8, 1, file->fp);
#endif
}




/* @section file line write operations
**
** @fdata [AjPFile]
**
** These functions write data directly to a file using system functions.
** Integer data is by default assumed to be stored as little-endian
** so that binary files with integers are portable across systems
**
** @nam2rule Writeline Write a string to a file
** @nam3rule Space   Write a string to a file with a leading space
** @suffix Newline   Write a string to a file with a trailing newline
**
** @argrule * file [AjPFile] Output file
** @argrule * line [const AjPStr] Output line with trailing newline
**
** @valrule * [AjBool] True on success
**
** @fcategory output
**
******************************************************************************/




/* @func ajWriteline **********************************************************
**
** Writes a string to a file, including any newline characters
**
** @param [u] file [AjPFile] Output file
** @param [r] line [const AjPStr] String to be written
** @return [AjBool] True on success
**
** @release 6.0.0
******************************************************************************/

AjBool ajWriteline(AjPFile file, const AjPStr line)
{
    if(!fwrite(MAJSTRGETPTR(line), MAJSTRGETLEN(line), 1, file->fp))
        return ajFalse;

    return ajTrue;
}




/* @func ajWritelineNewline ***************************************************
**
** Writes a string to a file, including any newline characters
**
** @param [u] file [AjPFile] Output file
** @param [r] line [const AjPStr] String to be written
** @return [AjBool] True on success
**
** @release 6.2.0
******************************************************************************/

AjBool ajWritelineNewline(AjPFile file, const AjPStr line)
{
    if(!fwrite(MAJSTRGETPTR(line), 1, MAJSTRGETLEN(line), file->fp))
        return ajFalse;

#ifdef WIN32
    /*if(!fwrite("\r\n", 1, 2, file->fp))
      return ajFalse;*/
    if(!fwrite("\n", 1, 1, file->fp))
        return ajFalse;
#else
    if(!fwrite("\n", 1, 1, file->fp))
        return ajFalse;
#endif

    return ajTrue;
}




/* @func ajWritelineSpace *****************************************************
**
** Writes a string to a file, with a leading space
**
** @param [u] file [AjPFile] Output file
** @param [r] line [const AjPStr] String to be written
** @return [AjBool] True on success
**
** @release 6.2.0
******************************************************************************/

AjBool ajWritelineSpace(AjPFile file, const AjPStr line)
{
    if(!fwrite(" ", 1, 1, file->fp))
        return ajFalse;

    if(!fwrite(MAJSTRGETPTR(line), MAJSTRGETLEN(line), 1, file->fp))
        return ajFalse;

    return ajTrue;
}




/* @datasection [AjPFilebuff] Buffered file object *****************************
**
** Function is for manipulating buffered input files and returns or
** takes at least one AjPFilebuff argument.
** 
*/




/* @section buffered file line read operations
**
** @fdata [AjPFilebuff]
**
** These functions read data directly from a file using system functions.
** Integer data is by default assumed to be stored as little-endian
** so that binary files with integers are portable across systems
**
** @nam2rule Buffread
** @nam3rule Line     Read next line from buffer
** @nam4rule Trim     Remove trailing newline and linefeed characters
** @suffix   Pos      Return file position after read
** @suffix   Store    Append line to store buffer
**
** @argrule * buff [AjPFilebuff] Input buffered file object
** @argrule Line Pdest [AjPStr*] Buffer (expanded automatically) for results
** @argrule Pos Ppos [ajlong*] File position after read
** @argrule Store dostore [AjBool] If true, use store buffer
** @argrule Store Pstore [AjPStr*] Buffer to store text from file
**
** @valrule * [AjBool] True on success
**
** @fcategory input
*/




/* @func ajBuffreadLine *******************************************************
**
** Reads a line from a buffered file. If the buffer has data, reads from the
** buffer. If the buffer is exhausted, reads from the file. If the file is
** exhausted, sets end of file and returns. If end of file was already set,
** looks for another file to open.
**
** @param [u] buff [AjPFilebuff] Buffered input file.
** @param [w] Pdest [AjPStr*] Buffer to hold results.
** @return [AjBool] ajTrue if data was read.
**
** @release 6.0.0
** @@
******************************************************************************/

AjBool ajBuffreadLine(AjPFilebuff buff, AjPStr* Pdest)
{
    ajlong fpos = 0;

    return ajBuffreadLinePos(buff, Pdest, &fpos);
}




/* @func ajBuffreadLinePos ****************************************************
**
** Reads a line from a buffered file. If the buffer has data, reads from the
** buffer. If the buffer is exhausted, reads from the file. If the file is
** exhausted, sets end of file and returns. If end of file was already set,
** looks for another file to open.
**
** @param [u] buff [AjPFilebuff] Buffered input file.
** @param [w] Pdest [AjPStr*] Buffer to hold results.
** @param [w] Ppos [ajlong*] File position before the read.
** @return [AjBool] ajTrue if data was read.
**
** @release 6.0.0
** @@
******************************************************************************/

AjBool ajBuffreadLinePos(AjPFilebuff buff, AjPStr* Pdest, ajlong* Ppos)
{
    AjBool ok;
    
    /* read from the buffer if it is not empty */
    
    *Ppos = 0;
    
    if(buff->Pos < buff->Size)
    {
	ajStrAssignS(Pdest, buff->Curr->Line);
	*Ppos = buff->Curr->Fpos;
	buff->Prev = buff->Curr;
	buff->Curr = buff->Curr->Next;
	buff->Pos++;

	return ajTrue;
    }
    
    /* file has been closed */
    if(!buff->File->Handle)
	return ajFalse;
    
    /* buffer used up - try reading from the file */
    
    ok = ajReadlinePos(buff->File, Pdest, &buff->Fpos);
    
    if(!ok)
    {
	if(buff->File->End)
	{
	    if(buff->Size)
	    {
		/* we have data in the buffer - fail */
		ajDebug("End of file - data in buffer - return ajFalse\n");
		return ajFalse;
	    }
	    else
	    {
		/* buffer clear - try another file */
		if(ajFileReopenNext(buff->File))
		{
		    /* OK - read the new file */
		    ok = ajBuffreadLinePos(buff, Pdest, Ppos);
		    ajDebug("End of file - trying next file ok: %B "
			    "fpos: %Ld %Ld\n",
			    ok, *Ppos, buff->Fpos);

		    return ok;
		}
		else
		{
		    /* no new file, fail again */
		    ajDebug("End of file - no new file to read - "
			    "return ajFalse\n");

		    return ajFalse;
		}
	    }
	}
	else
        {
	    ajWarn("Error reading from file '%S'",
		    ajFileGetNameS(buff->File));
            return ajFalse;
        }
    }
    
    if(buff->Nobuff)
    {
	*Ppos = buff->Fpos;
	/*ajDebug("ajBuffreadLinePos unbuffered fpos: %Ld\n", *Ppos);*/

	return ajTrue;
    }
    
    filebuffLineAdd(buff, *Pdest);
    *Ppos = buff->Fpos;
    
    return ajTrue;
}




/* @func ajBuffreadLinePosStore ***********************************************
**
** Reads a line from a buffered file. Also appends the line to
** a given string if the append flag is true. A double NULL character
** is added afterwards. If the buffer has data, reads from the
** buffer. If the buffer is exhausted, reads from the file. If the file is
** exhausted, sets end of file and returns. If end of file was already set,
** looks for another file to open.
**
** @param [u] buff [AjPFilebuff] Buffered input file.
** @param [w] Pdest [AjPStr*] Buffer to hold results.
** @param [w] Ppos [ajlong*] File position before the read.
** @param [r] dostore [AjBool] append if true
** @param [w] Pstore [AjPStr*] string to append to
** @return [AjBool] ajTrue if data was read.
** @category modify [AjPFilebuff] Reads a line from a buffered file
**                                with append.
**
** @release 6.0.0
** @@
******************************************************************************/

AjBool ajBuffreadLinePosStore(AjPFilebuff buff, AjPStr* Pdest, ajlong* Ppos,
                              AjBool dostore, AjPStr *Pstore)
{
    AjBool ret;

    ret =  ajBuffreadLinePos(buff, Pdest, Ppos);

    if(dostore && ret)
    {
	ajDebug("ajBuffreadLinePosStore:\n%S", *Pdest);
	ajStrAppendS(Pstore,*Pdest);
    }

    return ret;
}




/* @func ajBuffreadLineStore **************************************************
**
** Reads a line from a buffered file. Also appends the line to
** a given string if the append flag is true. A double NULL character
** is added afterwards. If the buffer has data, reads from the
** buffer. If the buffer is exhausted, reads from the file. If the file is
** exhausted, sets end of file and returns. If end of file was already set,
** looks for another file to open.
**
** @param [u] buff [AjPFilebuff] Buffered input file.
** @param [w] Pdest [AjPStr*] Buffer to hold results.
** @param [r] dostore [AjBool] append if true
** @param [w] Pstore[AjPStr*] string to append to
** @return [AjBool] ajTrue if data was read.
** @category modify [AjPFilebuff] Reads a line from a buffered file
**                                with append.
**
** @release 6.0.0
** @@
******************************************************************************/

AjBool ajBuffreadLineStore(AjPFilebuff buff, AjPStr* Pdest,
                           AjBool dostore, AjPStr *Pstore)
{
    ajlong fpos = 0;
    AjBool ret;

    ret =  ajBuffreadLinePos(buff, Pdest, &fpos);

    if(dostore && ret)
    {
	ajDebug("ajBuffreadLineStore: '%S'", *Pdest);
	ajStrAppendS(Pstore,*Pdest);
    }

    return ret;
}




/* @func ajBuffreadLineTrim ***************************************************
**
** Reads a line from a buffered file. If the buffer has data, reads from the
** buffer. If the buffer is exhausted, reads from the file. If the file is
** exhausted, sets end of file and returns. If end of file was already set,
** looks for another file to open.
**
** @param [u] buff [AjPFilebuff] Buffered input file.
** @param [w] Pdest [AjPStr*] Buffer to hold results.
** @return [AjBool] ajTrue if data was read.
** @category modify [AjPFilebuff] Reads a line from a buffered file.
**
** @release 6.0.0
** @@
******************************************************************************/

AjBool ajBuffreadLineTrim(AjPFilebuff buff, AjPStr* Pdest)
{
    AjBool ret;
    ajlong fpos = 0;

    ret = ajBuffreadLinePos(buff, Pdest, &fpos);
    
    /* trim any trailing newline */

    /*ajDebug("Remove carriage-return characters from PC-style files\n");*/
    if(ajStrGetCharLast(*Pdest) == '\n')
	ajStrCutEnd(Pdest, 1);

    /* PC files have \r\n Macintosh files have just \r : this fixes both */
    if(ajStrGetCharLast(*Pdest) == '\r')
	ajStrCutEnd(Pdest, 1);

    return ret;
}




/* @funcstatic filebuffLineAdd ************************************************
**
** Appends a line to a buffer.
**
** @param [u] buff [AjPFilebuff] File buffer
** @param [r] line [const AjPStr] Line
** @return [void]
**
** @release 6.0.0
******************************************************************************/

static void filebuffLineAdd(AjPFilebuff buff, const AjPStr line)
{
    /* ajDebug("fileBuffLineAdd '%S'\n", line);*/
    if(buff->Freelines)
    {
	if(!buff->Lines)
	{
	    /* Need to set first line in list */
	    buff->Lines = buff->Freelines;
	}
	else
	    buff->Last->Next = buff->Freelines;

	buff->Last = buff->Freelines;
	buff->Freelines = buff->Freelines->Next;

	if(!buff->Freelines)
	{
	    /* Free list now empty */
	    buff->Freelast = NULL;
	}
    }
    else
    {
	/* No Free list, make a new string */
	if(!buff->Lines)
	    buff->Lines = AJNEW0(buff->Last);
	else
	    buff->Last = AJNEW0(buff->Last->Next);
    }
    
    ajStrAssignS(&buff->Last->Line, line);
    buff->Prev = buff->Curr;
    buff->Curr = buff->Last;
    buff->Last->Next = NULL;
    buff->Last->Fpos = buff->Fpos;
    buff->Pos++;
    buff->Size++;
        
    return;
}




#ifdef AJ_COMPILE_DEPRECATED_BOOK
#endif




#ifdef AJ_COMPILE_DEPRECATED
/* @obsolete ajFileGets
** @rename ajReadline
*/
__deprecated AjBool ajFileGets(AjPFile thys, AjPStr* pdest)
{
    return ajReadline(thys, pdest);
}




/* @obsolete ajFileReadAppend
** @rename ajReadlineAppend
*/
__deprecated AjBool ajFileReadAppend(AjPFile thys, AjPStr* pbuff)
{
    return ajReadlineAppend(thys, pbuff);
}




/* @obsolete ajFileGetsL
** @rename ajReadlinePos
*/
__deprecated AjBool ajFileGetsL(AjPFile thys, AjPStr* pdest, ajlong* fpos)
{
    return ajReadlinePos(thys, pdest, fpos);
}




/* @obsolete ajFileGetsTrim
** @rename ajReadlineTrim
*/
__deprecated AjBool ajFileGetsTrim(AjPFile thys, AjPStr* pdest)
{
    ajlong fpos=0;

    return ajReadlineTrimPos(thys, pdest, &fpos);
}




/* @obsolete ajFileReadLine
** @rename ajReadlineTrim
*/
__deprecated AjBool ajFileReadLine(AjPFile thys, AjPStr* pdest)
{
    ajlong fpos=0;

    return ajReadlineTrimPos(thys, pdest, &fpos);
}




/* @obsolete ajFileGetsTrimL
** @rename ajReadlineTrimPos
*/
__deprecated AjBool ajFileGetsTrimL(AjPFile thys, AjPStr* pdest, ajlong* fpos)
{
    return ajReadlineTrimPos(thys, pdest, fpos);
}




/* @obsolete ajFileRead
** @replace ajReadbinBinary (1,2,3,4/4,3,2,1)
*/
__deprecated size_t ajFileRead(void* ptr, size_t element_size, size_t count,
		  AjPFile file)
{
    return ajReadbinBinary(file, count, element_size, ptr);
}




/* @obsolete ajFileOutHeader
** @remove Not used
*/
__deprecated void ajFileOutHeader(AjPFile file)
{
    ajFmtPrintF(file, "Standard output header ...\n");

    return;
}




/* @obsolete ajFileWrite
** @replace ajWritebinBinary (1,2,3,4/1,4,3,2)
*/
__deprecated size_t ajFileWrite(AjPFile file, const void* ptr,
		   size_t element_size, size_t count)
{
    return ajWritebinBinary(file,count,element_size,ptr);
}




/* @obsolete ajFileWriteByte
** @rename ajWritebinByte
*/
__deprecated ajint ajFileWriteByte(AjPFile thys, char ch)
{
    return ajWritebinByte(thys, ch);
}




/* @obsolete ajFileWriteInt2
** @rename ajWritebinInt2
*/
__deprecated ajint ajFileWriteInt2(AjPFile thys, ajshort i)
{
    return ajWritebinInt2(thys, i);
}




/* @obsolete ajFileWriteInt4
** @rename ajWritebinInt4
*/
__deprecated ajint ajFileWriteInt4(AjPFile thys, ajint i)
{
    return ajWritebinInt4(thys, i);
}




/* @obsolete ajFileWriteStr
** @rename ajWritebinStr
*/
__deprecated ajint ajFileWriteStr(AjPFile thys, const AjPStr str, ajuint len)
{
    return ajWritebinStr(thys, str, len);
}




/* @obsolete ajFileBuffGet
** @rename ajBuffreadLine
*/
__deprecated AjBool ajFileBuffGet(AjPFilebuff thys, AjPStr* pdest)
{
    return ajBuffreadLine(thys, pdest);
}




/* @obsolete ajFileBuffGetL
** @rename ajBuffreadLinePos
*/
__deprecated AjBool ajFileBuffGetL(AjPFilebuff buff, AjPStr* pdest,
                                   ajlong* fpos)
{
    return ajBuffreadLinePos(buff, pdest, fpos);
}




/* @obsolete ajFileBuffGetStoreL
** @rename ajBuffreadLinePosStore
*/
__deprecated AjBool ajFileBuffGetStoreL(AjPFilebuff thys, AjPStr* pdest,
			   ajlong* fpos,
			   AjBool store, AjPStr *astr)
{
    return ajBuffreadLinePosStore(thys, pdest, fpos, store, astr);
}




/* @obsolete ajFileBuffGetStore
** @rename ajBuffreadLineStore
*/
__deprecated AjBool ajFileBuffGetStore(AjPFilebuff thys, AjPStr* pdest,
			  AjBool store, AjPStr *astr)
{
    return ajBuffreadLineStore(thys, pdest, store, astr);
}




/* @obsolete ajFileBuffGetTrim
** @rename ajBuffreadLineTrim
*/
__deprecated AjBool ajFileBuffGetTrim(AjPFilebuff thys, AjPStr* pdest)
{
    return ajBuffreadLineTrim(thys, pdest);
}
#endif
