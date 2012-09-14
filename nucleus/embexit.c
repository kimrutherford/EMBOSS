/* @source embexit ************************************************************
**
** General routines for exiting on success or failure
**
** @author Copyright (c) 1999 Peter Rice
** @version $Revision: 1.25 $
** @modified $Date: 2011/10/18 14:24:24 $ by $Author: rice $
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

#include "embexit.h"

/* any include with an Exit function */

#include "ajutil.h"
#include "embsig.h"
#include "embdbi.h"
#include "embgroup.h"
#include "embindex.h"
#include "embword.h"
#include "embpatlist.h"

#include "ajtextdb.h"
#include "ajseqdb.h"
#include "ajfeatdb.h"
#include "ajobodb.h"
#include "ajassemdb.h"
#include "ajrefseqdb.h"
#include "ajtaxdb.h"
#include "ajurldb.h"
#include "ajvardb.h"
#include "ajresourcedb.h"

#include "ajgraph.h"
#include "ajacd.h"




/* @func embExit **************************************************************
**
** Cleans up as necessary, and calls ajExit
**
** @return [void]
**
** @release 1.0.0
** @@
******************************************************************************/

__noreturn void embExit (void)
{
    ajUtilLoginfo();
    embSigExit();
    embDbiExit();
    embGrpExit();
    embIndexExit();
    embWordExit();
    embPatlistExit();

    ajTextdbExit();
    ajSeqDbExit();
    ajFeatdbExit();
    ajObodbExit();
    ajAssemdbExit();
    ajRefseqdbExit();
    ajTaxdbExit();
    ajUrldbExit();
    ajVardbExit();
    ajResourcedbExit();

    ajGraphicsExit();
    ajAcdExit(ajFalse);

    ajExit();
}




/* @func embExitBad ***********************************************************
**
** Cleans up as necessary, and calls ajExitBad
**
** @return [void]
**
** @release 4.0.0
** @@
******************************************************************************/

__noreturn void embExitBad (void)
{
    ajExitBad();
}
