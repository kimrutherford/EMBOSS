/******************************************************************************
** @source AJAX matrices functions
**
** @version 1.0
** @author Copyright (C) 2003 Alan Bleasby
** @author Copyright (C) 2003 Peter Rice
** @@
** @modified Copyright (C) 2003 Jon Ison. Rewritten for string matrix labels
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
******************************************************************************/

#include "ajax.h"




/* @func ajMatrixNew **********************************************************
**
** Creates a new, zero matrix from an array of strings and a matrix name. If 
** the matrix is a residue substitution matrix then each string would be a 
** defined sequence character.
**
** The matrix comparison value table Matrix is created and initialised
** with zeroes.
**
** @param [r] codes [const AjPPStr] Matrix labels, e.g. valid sequence
**                                  character codes
** @param [r] n [ajint] Number of labels
** @param [r] filename [const AjPStr] Matrix filename
** @return [AjPMatrix] New matrix, or NULL if codes, n or filename are 0.
** @@
******************************************************************************/

AjPMatrix ajMatrixNew(const AjPPStr codes, ajint n, const AjPStr filename)
{
    ajint     i   = 0;
    AjPMatrix ret = NULL;
    ajint nsize;

    if((!n) || (!codes) || (!filename))
	return NULL;

    nsize = n + 1;

    AJNEW0(ret);

    ajStrAssignS(&ret->Name, filename);

    AJCNEW0(ret->Codes, n);
    for(i=0; i<n; i++)
	ret->Codes[i] = ajStrNew();
    
    for(i=0; i<n; i++)
	ajStrAssignS(&ret->Codes[i], codes[i]);

    ret->Size = nsize;

    AJCNEW0(ret->Matrix, nsize);
    for(i=0; i<nsize; i++)
	AJCNEW0(ret->Matrix[i], nsize);
    ret->Cvt = ajSeqcvtNewStr(codes, n);

    return ret;
}




/* @func ajMatrixNewAsym ******************************************************
**
** Creates a new, zero asymmetrical matrix from two array of strings and a 
** matrix name. If the matrix is a substitution matrix then each string would 
** be a defined code, e.g. sequence character.
**
** The matrix comparison value table Matrix is created and initialised
** with zeroes.
**
** @param [r] codes [const AjPPStr] Matrix column labels, e.g. valid sequence
**                                  character codes
** @param [r] n [ajint] Number of column labels
** @param [r] rcodes [const AjPPStr] Matrix row labels, e.g. valid sequence
**                                  character codes
** @param [r] rn [ajint] Number of row labels
** @param [r] filename [const AjPStr] Matrix filename
** @return [AjPMatrix] New matrix, or NULL if codes, n or filename are 0.
** @@
******************************************************************************/
AjPMatrix ajMatrixNewAsym(const AjPPStr codes, ajint n, 
			  const AjPPStr rcodes, ajint rn, 
			  const AjPStr filename)
{
    ajint     i   = 0;
    AjPMatrix ret = NULL;
    ajint nsize;
    ajint rnsize;

    if((!n) || (!codes) || (!filename))
	return NULL;

    nsize  = n + 1;
    rnsize = rn + 1;

    AJNEW0(ret);

    ajStrAssignS(&ret->Name, filename);

    AJCNEW0(ret->Codes, n);
    for(i=0; i<n; i++)
	ret->Codes[i] = ajStrNew();
    
    for(i=0; i<n; i++)
	ajStrAssignS(&ret->Codes[i], codes[i]);

    ret->Size  = nsize;
    
    AJCNEW0(ret->CodesRow, rn);
    for(i=0; i<rn; i++)
	ret->CodesRow[i] = ajStrNew();
    
    for(i=0; i<rn; i++)
	ajStrAssignS(&ret->CodesRow[i], rcodes[i]);

    ret->SizeRow = rnsize;
    
    AJCNEW0(ret->Matrix, rnsize);
    for(i=0; i<rnsize; i++)
	AJCNEW0(ret->Matrix[i], nsize);

    ret->Cvt = ajSeqcvtNewStrAsym(codes, n, rcodes, rn);

    return ret;
}





/* @func ajMatrixfNew *********************************************************
**
** Creates a new, zero matrix from an array of strings and a matrix name. If 
** the matrix is a residue substitution matrix then each string would be a 
** defined sequence character.
**
** The matrix comparison value table Matrix is created and initialised
** with zeroes.
**
** @param [r] codes [const AjPPStr] Matrix labels,
**                                 e.g. valid sequence char codes
** @param [r] n [ajint] Number of labels
** @param [r] filename [const AjPStr] Matrix filename
** @return [AjPMatrixf] New matrix, or NULL if codes, n or filename are 0.
** @@
******************************************************************************/

AjPMatrixf ajMatrixfNew(const AjPPStr codes, ajint n, const AjPStr filename)
{
    ajint i = 0;
    AjPMatrixf ret = 0;
    ajint nsize;
 
    if((!n) || (!codes) || (!filename))
	return NULL;

    nsize = n + 1;

    AJNEW0(ret);

    ajStrAssignS(&ret->Name, filename);

    AJCNEW0(ret->Codes, n);
    for(i=0; i<n; i++)
	ret->Codes[i] = ajStrNew();

    for(i=0; i<n; i++)
	ajStrAssignS(&ret->Codes[i], codes[i]);

    ret->Size = nsize;

    AJCNEW0(ret->Matrixf, nsize);
    for(i=0; i<nsize; i++)
	AJCNEW0(ret->Matrixf[i], nsize);
    ret->Cvt = ajSeqcvtNewStr(codes, n);

    return ret;
}





/* @func ajMatrixfNewAsym *****************************************************
**
** Creates a new, zero asymmetrical matrix from an array of strings and a 
** matrix name. If the matrix is a residue substitution matrix then each 
** string would be a defined sequence character.
**
** The matrix comparison value table Matrix is created and initialised
** with zeroes.
**
** @param [r] codes [const AjPPStr] Matrix labels,
**                                 e.g. valid sequence char codes
** @param [r] n [ajint] Number of labels
** @param [r] rcodes [const AjPPStr] Matrix row labels, e.g. valid sequence
**                                  character codes.
** @param [r] rn [ajint] Number of row labels
** @param [r] filename [const AjPStr] Matrix filename
** @return [AjPMatrixf] New matrix, or NULL if codes, n or filename are 0.
** @@
******************************************************************************/
AjPMatrixf ajMatrixfNewAsym(const AjPPStr codes, ajint n, 
			    const AjPPStr rcodes, ajint rn, 
			    const AjPStr filename)
{
    ajint i = 0;
    AjPMatrixf ret = 0;
    ajint nsize;
    ajint rnsize;
 
    if((!n) || (!codes) || (!filename))
	return NULL;

    nsize = n + 1;
    rnsize = rn + 1;

    AJNEW0(ret);

    ajStrAssignS(&ret->Name, filename);

    AJCNEW0(ret->Codes, n);
    for(i=0; i<n; i++)
	ret->Codes[i] = ajStrNew();

    for(i=0; i<n; i++)
	ajStrAssignS(&ret->Codes[i], codes[i]);

    ret->Size = nsize;


    AJCNEW0(ret->CodesRow, rn);
    for(i=0; i<rn; i++)
	ret->CodesRow[i] = ajStrNew();

    for(i=0; i<rn; i++)
	ajStrAssignS(&ret->CodesRow[i], rcodes[i]);

    ret->SizeRow = rnsize;


    AJCNEW0(ret->Matrixf, rnsize);
    for(i=0; i<rnsize; i++)
	AJCNEW0(ret->Matrixf[i], nsize);

    ret->Cvt = ajSeqcvtNewStrAsym(codes, n, rcodes, rn);

    return ret;
}







/* @func ajMatrixfDel *********************************************************
**
** Delete a float matrix
**
** @param [w] thys [AjPMatrixf*] Matrix to delete
** @return [void]
** @@
******************************************************************************/

void ajMatrixfDel(AjPMatrixf *thys)
{
    ajint isize = 0;
    ajint jsize = 0;
    ajint rsize = 0;
    ajint ssize = 0;
    ajint i     = 0;


    if(!*thys || !thys)
	return;
    
    isize = (*thys)->Size;
    jsize = isize - 1;
    rsize = (*thys)->SizeRow;
    ssize = rsize - 1;

    

    for(i=0; i<jsize; ++i)
	ajStrDel(&(*thys)->Codes[i]);
    AJFREE((*thys)->Codes);

    for(i=0; i<ssize; ++i)
	ajStrDel(&(*thys)->CodesRow[i]);
    AJFREE((*thys)->CodesRow);

    ajStrDel(&(*thys)->Name);

    for(i=0; i<rsize; ++i)
	AJFREE((*thys)->Matrixf[i]);
    AJFREE((*thys)->Matrixf);

    ajSeqcvtDel(&(*thys)->Cvt);
    AJFREE(*thys);

    return;
}




/* @func ajMatrixDel **********************************************************
**
** Delete an integer matrix
**
** @param [w] thys [AjPMatrix*] Matrix to delete
** @return [void]
** @@
******************************************************************************/

void ajMatrixDel(AjPMatrix *thys)
{
    ajint isize = 0;
    ajint jsize = 0;
    ajint rsize = 0;
    ajint ssize = 0;
    ajint i     = 0;


    if(!*thys || !thys)
	return;

    isize = (*thys)->Size;
    jsize = isize - 1;
    rsize = (*thys)->SizeRow;
    ssize = rsize - 1;
    

    for(i=0; i<jsize; ++i)
	ajStrDel(&(*thys)->Codes[i]);
    AJFREE((*thys)->Codes);

    for(i=0; i<ssize; ++i)
	ajStrDel(&(*thys)->CodesRow[i]);
    AJFREE((*thys)->CodesRow);

    ajStrDel(&(*thys)->Name);

    for(i=0; i<rsize; ++i)
	AJFREE((*thys)->Matrix[i]);
    AJFREE((*thys)->Matrix);

    ajSeqcvtDel(&(*thys)->Cvt);
    AJFREE(*thys);

    return;
}




/* @func ajMatrixArray ********************************************************
**
** Returns the comparison matrix as an array of integer arrays.
** Sequence characters are indexed in this array using the internal
** Sequence Conversion table in the matrix (see ajMatrixCvt)
**
** @param [r] thys [const AjPMatrix] Matrix object
** @return [AjIntArray*] array of integer arrays for comparison values.
** @@
******************************************************************************/

AjIntArray* ajMatrixArray(const AjPMatrix thys)
{
    if(thys)
	return thys->Matrix;
    else
	return NULL;
}




/* @func ajMatrixfArray *******************************************************
**
** Returns the comparison matrix as an array of float arrays.
** Sequence characters are indexed in this array using the internal
** Sequence Conversion table in the matrix (see ajMatrixCvt)
**
** @param [r] thys [const AjPMatrixf] Float Matrix object
** @return [AjFloatArray*] array of float arrays for comparison values.
** @@
******************************************************************************/

AjFloatArray* ajMatrixfArray(const AjPMatrixf thys)
{
    if(thys)
	return thys->Matrixf;
    else
	return NULL;
}




/* @func ajMatrixSize *********************************************************
**
** Returns the comparison matrix size.
**
** @param [r] thys [const AjPMatrix] Matrix object
** @return [ajint] .
** @@
******************************************************************************/

ajint ajMatrixSize(const AjPMatrix thys)
{
    if(thys)
	return thys->Size;
    else
	return 0;
}




/* @func ajMatrixfSize ********************************************************
**
** Returns the comparison matrix size.
**
** @param [r] thys [const AjPMatrixf] Matrix object
** @return [ajint] .
** @@
******************************************************************************/

ajint ajMatrixfSize(const AjPMatrixf thys)
{
    if(thys)
	return thys->Size;
    else
	return 0;
}




/* @func ajMatrixCvt **********************************************************
**
** Returns the sequence character conversion table for a matrix.
** This table converts any character defined in the matrix to a
** positive integer, and any other character is converted to zero.
**
** @param [r] thys [const AjPMatrix] Matrix object
** @return [AjPSeqCvt] sequence character conversion table
** @@
******************************************************************************/

AjPSeqCvt ajMatrixCvt(const AjPMatrix thys)
{
    if(thys)
	return thys->Cvt;
    else
	return NULL;
}




/* @func ajMatrixfCvt *********************************************************
**
** Returns the sequence character conversion table for a matrix.
** This table converts any character defined in the matrix to a
** positive integer, and any other character is converted to zero.
**
** @param [r] thys [const AjPMatrixf] Float Matrix object
** @return [AjPSeqCvt] sequence character conversion table
** @@
******************************************************************************/

AjPSeqCvt ajMatrixfCvt(const AjPMatrixf thys)
{
    if(thys)
	return thys->Cvt;
    else
	return NULL;
}




/* @func ajMatrixChar *********************************************************
**
** Returns the sequence character conversion table for a matrix.
** This table converts any string defined in the matrix to a
** positive integer, and any other string is converted to zero.
**
** @param [r] thys [const AjPMatrix] Matrix object
** @param [r] i [ajint] Character index
** @param [w] label [AjPStr *] Matrix label, e.g. sequence character code
** @return [void] 
** @@
******************************************************************************/

void ajMatrixChar(const AjPMatrix thys, ajint i, AjPStr *label)
{
    if(!thys)
    {
	ajStrAssignC(label, "?");
	return;
    }

    if(i >= thys->Size)
    {
	ajStrAssignC(label, "?");
	return;
    }

    if(i < 0) 
    {
	ajStrAssignC(label, "?");
	return;
    }

    ajStrAssignS(label, thys->Codes[i]);

    return;
}




/* @func ajMatrixfChar ********************************************************
**
** Returns the sequence character conversion table for a matrix.
** This table converts any character defined in the matrix to a
** positive integer, and any other character is converted to zero.
**
** @param [r] thys [const AjPMatrixf] Matrix object
** @param [r] i [ajint] Character index
** @param [w] label [AjPStr *] Matrix label, e.g. sequence character code
** @return [void] 
** @@
******************************************************************************/

void ajMatrixfChar(const AjPMatrixf thys, ajint i, AjPStr *label)
{
    if(!thys)
    {
	ajStrAssignC(label, "?");
	return;
    }

    if(i >= thys->Size) 
    {	
	ajStrAssignC(label, "?");
	return;
    }
    
    if(i < 0)
    {
	ajStrAssignC(label, "?");
	return;
    }

    ajStrAssignS(label, thys->Codes[i]);

    return;
}




/* @func ajMatrixName *********************************************************
**
** Returns the name of a matrix object, usually the filename from
** which it was read.
**
** @param [r] thys [const AjPMatrix] Matrix object
** @return [const AjPStr] The name, a pointer to the internal name.
** @@
******************************************************************************/

const AjPStr ajMatrixName(const AjPMatrix thys)
{
    static AjPStr emptystr = NULL;

    if(!thys)
    {
	if (!emptystr)
	    emptystr = ajStrNewC("");
	return emptystr;
    }

    return thys->Name;
}




/* @func ajMatrixfName ********************************************************
**
** Returns the name of a matrix object, usually the filename from
** which it was read.
**
** @param [r] thys [const AjPMatrixf] Matrix object
** @return [const AjPStr] The name, a pointer to the internal name.
** @@
******************************************************************************/

const AjPStr ajMatrixfName(const AjPMatrixf thys)
{
    static AjPStr emptystr = NULL;

    if(!thys)
    {
	emptystr = ajStrNewC("");
	return emptystr;
    }

    return thys->Name;
}


/* @func ajMatrixRead *********************************************************
**
** Constructs a comparison matrix from a given local data file
**
** @param [w] pthis [AjPMatrix*] New Matrix object.
** @param [r] filename [const AjPStr] Input filename
** @return [AjBool] ajTrue on success.
** @@
******************************************************************************/

AjBool ajMatrixRead(AjPMatrix* pthis, const AjPStr filename)
{
    AjPStr buffer = NULL;
    const AjPStr tok    = NULL;

    AjPStr firststring  = NULL;
    AjPStr *orderstring = NULL;

    AjPFile file    = NULL;
    AjBool first    = ajTrue;
    AjPMatrix  thys = NULL;
    const char *ptr = NULL;
    ajint **matrix  = NULL;

    ajint minval = -1;
    ajint i      = 0;
    ajint l      = 0;
    ajint k      = 0;
    ajint cols   = 0;
    ajint rows   = 0;

    ajint *templine = NULL;

    AjPList rlabel_list = NULL;
    AjPStr  *rlabel_arr  = NULL;

#ifndef WIN32
    static const char *delimstr = " :\t\n";
#else
    static const char *delimstr = " :\t\n\r";
#endif

    rlabel_list = ajListNew();
    

    firststring = ajStrNew();
    
    ajFileDataNew(filename,&file);
    
    if(!file)
    {
	ajStrDel(&firststring);
	ajListFree(&rlabel_list);
	return ajFalse;
    }
    
    /* Read row labels */
    while(ajFileGets(file,&buffer))
    {
	ptr = ajStrGetPtr(buffer);
#ifndef WIN32
	if(*ptr != '#' && *ptr != '\n')
#else
	if(*ptr != '#' && *ptr != '\n' && *ptr != '\r')
#endif
	{
	    if(first)
		first = ajFalse;
	    else
	    {	
		ajFmtScanC(ptr, "%S", &firststring);
		ajListPushAppend(rlabel_list, firststring);
		firststring = ajStrNew();	
	    }
	}
    }
    first = ajTrue;
    ajStrDel(&firststring);
    rows = ajListToarray(rlabel_list, (void ***) &rlabel_arr);
    ajFileSeek(file, 0, 0);


    while(ajFileGets(file,&buffer))
    {
	ajStrRemoveWhiteExcess(&buffer);
	ptr = ajStrGetPtr(buffer);
	if(*ptr && *ptr != '#')
	{				
	    if(first)
	    {
		cols = ajStrParseCountC(buffer,delimstr);
		AJCNEW0(orderstring, cols);
		for(i=0; i<cols; i++)   
		    orderstring[i] = ajStrNew();
		
		tok = ajStrParseC(buffer, " :\t\n");
		ajStrAssignS(&orderstring[l++], tok);
		while((tok = ajStrParseC(NULL, " :\t\n")))
		    ajStrAssignS(&orderstring[l++], tok);

		first = ajFalse;

		thys = *pthis = ajMatrixNewAsym(orderstring, cols, 
						rlabel_arr, rows, 
						filename);
		matrix = thys->Matrix;
	    }
	    else
	    {
		ajFmtScanC(ptr, "%S", &firststring);
		
		/* JISON 19/7/4
		   k = ajSeqcvtGetCodeK(thys->Cvt, ajStrGetCharFirst(firststring)); */
		k = ajSeqcvtGetCodeS(thys->Cvt, firststring);

		/* 
		 ** cols+1 is used below because 2nd and subsequent lines have 
		 ** one more string in them (the residue label) 
		 */
		templine = ajArrIntLine(buffer,delimstr,2,cols+1);
		
		for(i=0; i<cols; i++)   
		{
		    if(templine[i] < minval) 
			minval = templine[i];

		    /* JISON 19/7/4
		    matrix[k][ajSeqcvtGetCodeK(thys->Cvt,
					ajStrGetCharFirst(orderstring[i]))] 
					    = templine[i]; */
		    matrix[k][ajSeqcvtGetCodeAsymS(thys->Cvt,
					       orderstring[i])] 
						   = templine[i];
		}
		AJFREE(templine);
	    }
	}
    }
    ajDebug("fill rest with minimum value %d\n", minval);
    

    ajFileClose(&file);
    ajStrDel(&buffer);

    for(i=0; i<cols; i++)   
	ajStrDel(&orderstring[i]);
    AJFREE(orderstring);
        
    
    ajDebug("read matrix file %S\n", filename);
    
    ajStrDel(&firststring);    

    for(i=0; i<rows; i++)   
	ajStrDel(&rlabel_arr[i]);
    AJFREE(rlabel_arr);
    ajListFree(&rlabel_list);

    return ajTrue;
}




/* @func ajMatrixfRead ********************************************************
**
** Constructs a comparison matrix from a given local data file
**
** @param [w] pthis [AjPMatrixf*] New Float Matrix object.
** @param [r] filename [const AjPStr] Input filename
** @return [AjBool] ajTrue on success.
** @@
******************************************************************************/

AjBool ajMatrixfRead(AjPMatrixf* pthis, const AjPStr filename)
{
    AjPStr *orderstring = NULL;
    AjPStr buffer       = NULL;
    AjPStr firststring  = NULL;
    AjPStr reststring   = NULL;
    const AjPStr tok    = NULL;

    ajint len  = 0;
    ajint i    = 0;
    ajint l    = 0;
    ajint k    = 0;
    ajint cols = 0;
    ajint rows   = 0;
    
    const char *ptr = NULL;

    AjPMatrixf thys = NULL;
    AjPFile file    = NULL;
    AjBool  first   = ajTrue;

    float **matrix  = NULL;
    float *templine = NULL;
    float minval    = -1.0;

    AjPList rlabel_list = NULL;
    AjPStr  *rlabel_arr  = NULL;
#ifndef WIN32
    static const char *delimstr = " :\t\n";
#else
    static const char *delimstr = " :\t\n\r";
#endif



    rlabel_list = ajListNew();
    

    
    firststring = ajStrNew();
    reststring  = ajStrNew();

    ajFileDataNew(filename,&file);
    
    if(!file)
    {
	ajStrDel(&firststring);
	ajStrDel(&reststring);
	return ajFalse;
    }
    

    /* Read row labels */
    while(ajFileGets(file,&buffer))
    {
	ptr = ajStrGetPtr(buffer);
#ifndef WIN32
	if(*ptr != '#' && *ptr != '\n')
#else
	if(*ptr != '#' && *ptr != '\n' && *ptr != '\r')
#endif
	{	
	    if(first)
		first = ajFalse;
	    else
	    {
		ajFmtScanC(ptr, "%S", &firststring);
		ajListPushAppend(rlabel_list, firststring);
		firststring = ajStrNew();
	    }
	}
    }
    first = ajTrue;
    ajStrDel(&firststring);
    rows = ajListToarray(rlabel_list, (void ***) &rlabel_arr);
    ajFileSeek(file, 0, 0);


    while(ajFileGets(file,&buffer))
    {
	ajStrRemoveWhiteExcess(&buffer);
	ptr = ajStrGetPtr(buffer);
	if(*ptr && *ptr != '#')
	{				
	    if(first)
	    {
		cols = ajStrParseCountC(buffer,delimstr);
		AJCNEW0(orderstring, cols);
		for(i=0; i<cols; i++)   
		    orderstring[i] = ajStrNew();

		tok = ajStrParseC(buffer, " :\t\n");
		ajStrAssignS(&orderstring[l++], tok);
		while((tok = ajStrParseC(NULL, " :\t\n")))
		    ajStrAssignS(&orderstring[l++], tok);

		first = ajFalse;

		thys = *pthis = ajMatrixfNewAsym(orderstring, cols, 
						 rlabel_arr, rows, 
						 filename);
		matrix = thys->Matrixf;
	    }
	    else
	    {
		ajFmtScanC(ptr, "%S", &firststring);
		/* JISON 19/7/4 
		   k = ajSeqcvtGetCodeK(thys->Cvt, ajStrGetCharFirst(firststring)); */
		k = ajSeqcvtGetCodeS(thys->Cvt, firststring); 

		len = MAJSTRGETLEN(firststring);
		ajStrAssignSubC(&reststring, ptr, len, -1);

		/* 
		** Must discard the first string (label) and use 
		** reststring otherwise ajArrFloatLine would fail (it 
		** cannot convert a string to a float)
		**   
		** Use cols,1,cols in below because although 2nd and 
		** subsequent lines have one more string in them (the
		** residue label in the 1st column) we've discarded that
		** from the string that's passed
		*/
		templine = ajArrFloatLine(reststring,delimstr,1,cols);
		
		for(i=0; i<cols; i++)  
		{
		    if(templine[i] < minval) 
			minval = templine[i];
		    /* JISON 19/7/4
		    matrix[k][ajSeqcvtGetCodeK(thys->Cvt,
					ajStrGetCharFirst(orderstring[i]))] 
					    = templine[i]; */

		    matrix[k][ajSeqcvtGetCodeAsymS(thys->Cvt,
					       orderstring[i])] 
						   = templine[i];
		}
		AJFREE(templine);
	    }
	}
    }
    ajDebug("fill rest with minimum value %d\n", minval);
    

    ajFileClose(&file);
    ajStrDel(&buffer);

    for(i=0; i<cols; i++)   
	ajStrDel(&orderstring[i]);
    AJFREE(orderstring);


    ajDebug("read matrix file %S\n", filename);
    
    ajStrDel(&firststring);
    ajStrDel(&reststring);

   for(i=0; i<rows; i++)   
	ajStrDel(&rlabel_arr[i]);
    AJFREE(rlabel_arr);
    ajListFree(&rlabel_list);

    return ajTrue;
}








/* @func ajMatrixSeqNum *******************************************************
**
** Converts a sequence top index numbers using the matrix's
** internal conversion table. Sequence characters not defined
** in the matrix are converted to zero.
**
** @param [r] thys [const AjPMatrix] Matrix object
** @param [r] seq [const AjPSeq] Sequence object
** @param [w] numseq [AjPStr*] Index code version of the sequence
** @return [AjBool] ajTrue on success.
** @@
******************************************************************************/

AjBool ajMatrixSeqNum(const AjPMatrix thys, const AjPSeq seq, AjPStr* numseq)
{
    return ajSeqConvertNum(seq, thys->Cvt, numseq);
}




/* @func ajMatrixfSeqNum ******************************************************
**
** Converts a sequence to index numbers using the matrix's
** internal conversion table. Sequence characters not defined
** in the matrix are converted to zero.
**
** @param [r] thys [const AjPMatrixf] Float Matrix object
** @param [r] seq [const AjPSeq] Sequence object
** @param [w] numseq [AjPStr*] Index code version of the sequence
** @return [AjBool] ajTrue on success.
** @@
******************************************************************************/

AjBool ajMatrixfSeqNum(const AjPMatrixf thys, const AjPSeq seq, AjPStr* numseq)
{
    return ajSeqConvertNum(seq, thys->Cvt, numseq);
}

/* @func ajMatrixGetCodes *****************************************************
**
** Returns the character codes for each offset in the matrix
**
** @param [r] thys [const AjPMatrix] Matrix object
** @return [AjPStr] Matrix codes
******************************************************************************/

AjPStr ajMatrixGetCodes(const AjPMatrix thys)
{
    AjPStr ret = NULL;
    ajint i;
    ajint maxcode;

    ret = ajStrNewRes(thys->Size);
    maxcode = thys->Size - 1;
    for (i=0;i<maxcode;i++)
	ajStrAppendK(&ret, ajStrGetCharFirst(thys->Codes[i]));
    return ret;
}

/* @func ajMatrixfGetCodes ****************************************************
**
** Returns the character codes for each offset in the matrix
**
** @param [r] thys [const AjPMatrixf] Matrix object
** @return [AjPStr] Matrix codes
******************************************************************************/

AjPStr ajMatrixfGetCodes(const AjPMatrixf thys)
{
    AjPStr ret = NULL;
    ajint i;
    ajint maxcode;

    ret = ajStrNewRes(thys->Size + 1);
    maxcode = thys->Size - 1;
    for (i=0;i<maxcode;i++)
	ajStrAppendK(&ret, ajStrGetCharFirst(thys->Codes[i]));
    return ret;
}
