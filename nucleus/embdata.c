/* @source embdata ************************************************************
**
** General routines for data files
**
** @author Copyright (c) 1999 Mark Faller
** @version $Revision: 1.22 $
** @modified $Date: 2012/12/07 10:23:28 $ by $Author: rice $
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

#include "embdata.h"
#include "ajlib.h"
#include "ajlist.h"
#include "ajtable.h"
#include "ajfileio.h"




/*
** Routines for getting the data into the data structure. The data structure
** consists of a list of tables. This means the routine can read any amount
** of data from a file. It is up to the developer to know the order of the
** tables in the list and what each refers to
*/




static AjBool dataListNextLine(AjPFile pfile, const char *commentLine,
                               AjPStr * line);




/* @func embDataListDel *******************************************************
**
** Deletes the tables of data list. Calls ajTablestrFree for each table in the
** list, and then calls ajListFree to free the actual list.
**
** @param [w] data [AjPList*] is the list of data tables to delete
** @return [void]
**
**
** @release 1.0.0
** @@
******************************************************************************/

void embDataListDel(AjPList* data)
{
   AjIList iter;
   AjPTable table;

   iter = ajListIterNewread(*data);

   while(!ajListIterDone(iter))
   {
      table = ajListIterGet(iter);
      ajTablestrFree(&table);
   }

   ajListIterDel(&iter);
   ajListFree(data);

   return;
}




/* @funcstatic dataListNextLine ***********************************************
**
** Private function to read in the next line of data from the file. It is
** called from embDataListRead.
**
** @param [u] pfile [AjPFile] file pointer to the data file
** @param [r] commentLine [const char *] the character(s) used to describe the
**        start of a comment line in the data file
** @param [w] line [AjPStr *] Buffer to hold the current line
** @return [AjBool] returns AjTrue if found another line of input otherwise
**         returns AjFalse
**
**
** @release 1.0.0
** @@
******************************************************************************/

static AjBool dataListNextLine(AjPFile pfile, const char *commentLine,
			       AjPStr * line)
{
   ajlong i;
   AjBool test;

   test = ajReadlineTrim(pfile, line);

   while(test)
   {
      i = ajStrFindC(*line, commentLine);

      if(i!=0)
	  break;

      test = ajReadlineTrim(pfile, line);
   }

   if(test)
       return ajTrue;

   return ajFalse;
}




/* @func embDataListRead ******************************************************
**
** General routine for reading in data from a file. The keys and values of
** each table are stored as AjPStr.
**
** @param [w] data [AjPList] is the list of data tables.
** @param [u] pfile [AjPFile] pointer to the data file
** @return [void]
**
**
** @release 2.9.0
** @@
******************************************************************************/

void embDataListRead(AjPList data, AjPFile pfile)
{
    AjPStr line = NULL;
    AjPStrTok tokens = NULL;
    char whiteSpace[]  = " \t\n\r";
    char commentLine[] = "#";
    char endOfData[]   = "//";
    AjPStr key;
    AjPStr copyKey;
    AjPStr value;
    AjPTable table;			/* stored in the list */
    AjIList iter = NULL;
    AjPTable ptable;
    AjPStr tmp;

    tmp  = ajStrNew();
    line = ajStrNew();


    while(dataListNextLine(pfile, commentLine, &line))
    {
	ajStrTokenDel(&tokens);
	tokens = ajStrTokenNewC(line, whiteSpace);

	/* the first token is the key for the row */
	key = ajStrNew();
	ajStrTokenNextParse(tokens, &key);

	if(!ajStrGetLen(key))
	{
	    ajFmtError("Error, did not pick up first key");
	    ajFatal("Error, did not pick up first key");
	}

	while(1)
	{
	    /*
	     ** while there are more tokens generate new table in list and
	     ** add (key,value)
	     */
	    value = NULL;

	    if(ajStrTokenNextParse(tokens, &value))
	    {
		table = ajTablestrNewCase(350);
		copyKey = ajStrNewRef(key);
		ajTablePut(table, copyKey, value);
		ajListPushAppend(data, table);
	    }

	    else break;
	}
	ajStrDel(&value);

	while(dataListNextLine(pfile, commentLine, &line))
	{
	    /*
	     ** for rest of data iterate for each table in list adding
	     ** (key,value) to each
	     */
	    ajStrTokenDel(&tokens);
	    tokens = ajStrTokenNewC(line, whiteSpace);
	    ajStrTokenNextParse(tokens, &key);

	    /* check for end of data block*/
	    if(! ajStrCmpC(key, endOfData))
		break;

	    iter = ajListIterNewread(data);

	    while(!ajListIterDone(iter))
	    {
		ptable = ajListIterGet(iter);
		copyKey = ajStrNewRef(key);

		if(!ajStrTokenNextParse(tokens, &tmp))
                    break;

		value = ajStrNewRef(tmp);
		ajTablePut(ptable, copyKey, value);
	    }

	    ajListIterDel(&iter);
	}
    }

    ajStrDel(&tmp);
    ajStrDel(&line);
    ajStrTokenDel(&tokens);
    ajListIterDel(&iter);
    ajStrDel(&key);

    return;
}




/* @func embDataListGetTables *************************************************
**
** Returns a list of data tables as requested. The data must already have been
** read in and stored as a list of tables. An unsigned integer is used to
** request tables. The first table has a value of 1, the second a value of 2,
** the third a value of 4, the fourth a value of 8 etc. For example a value
** of 10 would request the second and fourth tables from the list in that
** order. Only returns a list of pointers to the data. It does not copy the
** tables.
**
** @param [r] fullList [const AjPList] The list containing all the tables
**                                     of data
** @param [w] returnList [AjPList] The new list containing just the tables
**        requested
** @param [r] required [ajuint] used to request tables. A value of 1
**        requests the first table, a value of 16 requests the fifth table,
**        a value of 14 returns the second third and fourth tables in the
**        original list.
** @return [void]
**
**
** @release 1.0.0
** @@
******************************************************************************/

void embDataListGetTables(const AjPList fullList, AjPList returnList,
			   ajuint required)
{
   AjIList iter;
   AjPTable table;

   iter = ajListIterNewread(fullList);

   while(!ajListIterDone(iter))
   {
      table = ajListIterGet(iter);

      if(required & 1)
          ajListPushAppend(returnList, table);

      required >>= 1;
   }

   ajListIterDel(&iter);

   return;
}




/* @func embDataListGetTable **************************************************
**
** Returns a single table of data from the list of data tables. The data must
** already have been read in and stored as a list of tables. An unsigned
** integer is used to request a table. The first table in the list has a
** value of 1, the second a value of 2, the third a value of 4, the fourth a
** value of 8 etc. For example a value of 64 would request the seventh data
** table in the list. When looking for which table to return the position of
** the lowest set bit in the value determines which table is returned i.e.
** a value of 66 would request the second table (not the seventh)
**
** @param [r] fullList [const AjPList] The list containing all the tables
**                                     of data
** @param [r] required [ajuint] used to request a table. A value of 1
**        requests the first table, a value of 16 requests the fifth table,
**        a value of 14 returns the second table in the original list.
** @return [AjPTable] the data table. Key and value are stored as AjPStrs
**
**
** @release 1.0.0
** @@
******************************************************************************/

AjPTable embDataListGetTable(const AjPList fullList, ajuint required)
{
   AjIList iter;
   AjPTable returnTable = NULL;

   iter = ajListIterNewread(fullList);

   while(!ajListIterDone(iter))
   {
      returnTable = ajListIterGet(iter);

      if(required & 1)
	  break;

      required >>= 1;
   }


   ajListIterDel(&iter);

   return returnTable;
}
