/* @source Ensembl Utility functions
**
** @author Copyright (C) 1999 Ensembl Developers
** @author Copyright (C) 2006 Michael K. Schuster
** @modified 2009 by Alan Bleasby for incorporation into EMBOSS core
** @modified $Date: 2011/03/29 15:21:32 $ by $Author: uludag $
** @version $Revision: 1.20 $
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
******************************************************************************/

/* ==================================================================== */
/* ========================== include files =========================== */
/* ==================================================================== */

#include "ensregistry.h"
#include "enstranslation.h"
#include "ensutility.h"




/* ==================================================================== */
/* ============================ constants ============================= */
/* ==================================================================== */




/* ==================================================================== */
/* ======================== global variables ========================== */
/* ==================================================================== */




/* ==================================================================== */
/* ========================== private data ============================ */
/* ==================================================================== */




/* ==================================================================== */
/* ======================== private constants ========================= */
/* ==================================================================== */




/* ==================================================================== */
/* ======================== private variables ========================= */
/* ==================================================================== */

/* #varstatic utilityInit *****************************************************
**
** Private boolean variable to ascertain that ensInit has been called once
** and only once.
**
******************************************************************************/

static AjBool utilityInit = AJFALSE;




/* ==================================================================== */
/* ======================== private functions ========================= */
/* ==================================================================== */




/* ==================================================================== */
/* ===================== All functions by section ===================== */
/* ==================================================================== */




/* @filesection ensutility ****************************************************
**
** @nam1rule ens Function belongs to the Ensembl library
**
******************************************************************************/




/* @datasection [none] Internals **********************************************
**
** Functions to control Ensembl internals
**
******************************************************************************/




/* @section Initialise ********************************************************
**
** @fdata [none]
**
** @nam2rule Init Initialise Ensembl internals
**
** @valrule * [void]
**
** @fcategory internals
******************************************************************************/




/* @func ensInit **************************************************************
**
** Initialises Ensembl internals.
**
** @return [void]
** @@
******************************************************************************/

void ensInit(void)
{
    if(utilityInit)
        return;

    ensRegistryInit();

    ensTranslationInit();

    utilityInit = ajTrue;

    return;
}




/* @section Exit **************************************************************
**
** @fdata [none]
**
** @nam2rule Exit Clear Ensembl internals
**
** @valrule * [void]
**
** @fcategory internals
******************************************************************************/




/* @func ensExit **************************************************************
**
** Clears Ensembl internals.
**
** @return [void]
** @@
******************************************************************************/

void ensExit(void)
{
    if(!utilityInit)
        return;

    ensRegistryExit();

    ensTranslationExit();

    utilityInit = ajFalse;

    return;
}




/* @section miscellaneous *****************************************************
**
** Miscellaneous functions with no specific Ensembl datatype
**
** @fdata [none]
**
** @nam2rule Trace    Debug output
** @nam3rule Query    AJAX Query
** @nam3rule Seqin    AJAX Sequence Input
** @nam3rule Seqdesc  Sequence Description
** @nam3rule Seq      AJAX Sequence
** @nam3rule Textin   AJAX Text Input
**
** @argrule Query     qry      [const AjPQuery]    AJAX Query
** @argrule Seq       seq      [const AjPSeq]      AJAX Sequence
** @argrule Seqdesc   seqdesc  [const AjPSeqDesc]  AJAX Sequence Description
** @argrule Seqin     seqin    [const AjPSeqin]    AJAX Sequence Input
** @argrule Textin    textin   [const AjPTextin]   AJAX Text Input
** @argrule Trace     level    [ajuint]            Indentation level
**
** @valrule * [AjBool] True on success
**
** @fcategory misc
******************************************************************************/




/* @func ensTraceQuery ********************************************************
**
** Trace an AJAX Query object.
**
** @param [r] qry [const AjPQuery] AJAX Query
** @param [r] level [ajuint] Indentation level
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
** @@
******************************************************************************/

AjBool ensTraceQuery(const AjPQuery qry, ajuint level)
{
    AjPStr indent = NULL;
    AjIList iter = NULL;
    AjPQueryField field = NULL;

    if(!qry)
        return ajFalse;

    indent = ajStrNew();

    ajStrAppendCountK(&indent, ' ', level * 2);

    ajDebug("%SensTraceQuery %p\n"
            "%S  SvrName      '%S'\n"
            "%S  DbName       '%S'\n"
            "%S  DbAlias      '%S'\n"
            "%S  DbType       '%S'\n"
            "%S  QueryFields   %p\n"
            "%S  ResultsList   %p\n"
            "%S  ResultsTable  %p\n"
            "%S  Method       '%S'\n"
            "%S  Qlinks       '%S'\n"
            "%S  Formatstr    '%S'\n"
            "%S  IndexDir     '%S'\n"
            "%S  Directory    '%S'\n"
            "%S  Filename     '%S'\n"
            "%S  Exclude      '%S'\n"
            "%S  DbFields     '%S'\n"
            "%S  DbFilter     '%S'\n"
            "%S  DbReturn     '%S'\n"
            "%S  DbIdentifier '%S'\n"
            "%S  DbAccession  '%S'\n"
            "%S  DbUrl        '%S'\n"
            "%S  DbProxy      '%S'\n"
            "%S  DbHttpVer    '%S'\n"
            "%S  ServerVer    '%S'\n"
            "%S  Field        '%S'\n"
            "%S  QryString    '%S'\n"
            "%S  Application  '%S'\n"
            "%S  Fpos          %Ld\n"
            "%S  TextAccess    %p\n"
            "%S  Access        %p\n"
            "%S  QryData       %p\n"
            "%S  QueryType     %d\n"
            "%S  DataType      %d\n"
            "%S  QryDone      '%B'\n"
            "%S  SetServer    '%B'\n"
            "%S  SetDatabase  '%B'\n"
            "%S  SetQuery     '%B'\n"
            "%S  Wild         '%B'\n"
            "%S  CaseId       '%B'\n"
            "%S  HasAcc       '%B'\n"
            "%S  CountEntries  %u\n"
            "%S  TotalEntries  %u\n",
            indent, qry,
            indent, qry->SvrName,
            indent, qry->DbName,
            indent, qry->DbAlias,
            indent, qry->DbType,
            indent, qry->QueryFields,
            indent, qry->ResultsList,
            indent, qry->ResultsTable,
            indent, qry->Method,
            indent, qry->Qlinks,
            indent, qry->Formatstr,
            indent, qry->IndexDir,
            indent, qry->Directory,
            indent, qry->Filename,
            indent, qry->Exclude,
            indent, qry->DbFields,
            indent, qry->DbFilter,
            indent, qry->DbReturn,
            indent, qry->DbIdentifier,
            indent, qry->DbAccession,
            indent, qry->DbUrl,
            indent, qry->DbProxy,
            indent, qry->DbHttpVer,
            indent, qry->ServerVer,
            indent, qry->Field,
            indent, qry->QryString,
            indent, qry->Application,
            indent, qry->Fpos,
            indent, qry->TextAccess,
            indent, qry->Access,
            indent, qry->QryData,
            indent, qry->QueryType,
            indent, qry->DataType,
            indent, qry->QryDone,
            indent, qry->SetServer,
            indent, qry->SetDatabase,
            indent, qry->SetQuery,
            indent, qry->Wild,
            indent, qry->CaseId,
            indent, qry->HasAcc,
            indent, qry->CountEntries,
            indent, qry->TotalEntries);

    ajDebug("%S  AJAX List %p of AJAX Query Field objects:\n",
            indent,
            qry->QueryFields);

    iter = ajListIterNewread(qry->QueryFields);
    while(!ajListIterDone(iter))
    {
        field = (AjPQueryField) ajListIterGet(iter);
        ajDebug("%S    %10.10S   '%S'\n",
                indent, field->Field, field->Wildquery);
    }
    ajListIterDel(&iter);

    ajStrDel(&indent);

    return ajTrue;
}




/* @func ensTraceSeq **********************************************************
**
** Trace an AJAX Sequence object.
**
** @param [r] seq [const AjPSeq] AJAX Sequence
** @param [r] level [ajuint] Indentation level
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
** @@
******************************************************************************/

AjBool ensTraceSeq(const AjPSeq seq, ajuint level)
{
    AjPStr indent = NULL;

    if(!seq)
        return ajFalse;

    indent = ajStrNew();

    ajStrAppendCountK(&indent, ' ', level * 2);

    ajDebug("%SensTraceSeq %p\n"
            "%S  Name      '%S'\n"
            "%S  Acc       '%S'\n"
            "%S  Sv        '%S'\n"
            "%S  Gi        '%S'\n"
            "%S  Tax       '%S'\n"
            "%S  Taxid     '%S'\n"
            "%S  Organelle '%S'\n"
            "%S  Type      '%S'\n"
            "%S  Molecule  '%S'\n"
            "%S  Class     '%S'\n"
            "%S  Division  '%S'\n"
            "%S  Evidence  '%S'\n"
            "%S  Db        '%S'\n"
            "%S  Setdb     '%S'\n"
            "%S  Full      '%S'\n"
            "%S  Date       %p\n"
            "%S  Desc      '%S'\n"
            "%S  Fulldesc   %p\n"
            "%S  Doc       '%S'\n"
            "%S  Usa       '%S'\n"
            "%S  Ufo       '%S'\n"
            "%S  Formatstr '%S'\n"
            "%S  Filename  '%S'\n"
            "%S  Entryname '%S'\n"
            "%S  TextPtr   '%S'\n"
            "%S  Acclist    %p\n"
            "%S  Keylist    %p\n"
            "%S  Taxlist    %p\n"
            "%S  Genelist   %p\n"
            "%S  Reflist    %p\n"
            "%S  Cmtlist    %p\n"
            "%S  Xreflist   %p\n"
            "%S  Seq        %p\n"
            "%S  Fttable    %p\n"
            "%S  Accuracy   %p\n"
            "%S  Fpos       %Ld\n"
            "%S  Rev       '%B'\n"
            "%S  Reversed  '%B'\n"
            "%S  Trimmed   '%B'\n"
            "%S  Circular  '%B'\n"
            "%S  Begin      %d\n"
            "%S  End        %d\n"
            "%S  Offset     %u\n"
            "%S  Offend     %u\n"
            "%S  Qualsize   %u\n"
            "%S  Weight     %f\n"
            "%S  Format     %d\n"
            "%S  Etype      %d\n",
            indent, seq,
            indent, seq->Name,
            indent, seq->Acc,
            indent, seq->Sv,
            indent, seq->Gi,
            indent, seq->Tax,
            indent, seq->Taxid,
            indent, seq->Organelle,
            indent, seq->Type,
            indent, seq->Molecule,
            indent, seq->Type,
            indent, seq->Division,
            indent, seq->Evidence,
            indent, seq->Db,
            indent, seq->Setdb,
            indent, seq->Full,
            indent, seq->Date,
            indent, seq->Desc,
            indent, seq->Fulldesc,
            indent, seq->Doc,
            indent, seq->Usa,
            indent, seq->Ufo,
            indent, seq->Formatstr,
            indent, seq->Filename,
            indent, seq->Entryname,
            indent, seq->TextPtr,
            indent, seq->Acclist,
            indent, seq->Keylist,
            indent, seq->Taxlist,
            indent, seq->Genelist,
            indent, seq->Reflist,
            indent, seq->Cmtlist,
            indent, seq->Xreflist,
            indent, seq->Seq,
            indent, seq->Fttable,
            indent, seq->Accuracy,
            indent, seq->Fpos,
            indent, seq->Rev,
            indent, seq->Reversed,
            indent, seq->Trimmed,
            indent, seq->Circular,
            indent, seq->Begin,
            indent, seq->End,
            indent, seq->Offset,
            indent, seq->Offend,
            indent, seq->Qualsize,
            indent, seq->Weight,
            indent, seq->Format,
            indent, seq->EType);

    ajStrDel(&indent);

    return ajTrue;
}




/* @func ensTraceSeqdesc ******************************************************
**
** Trace an AJAX Sequence Description object.
**
** @param [r] seqdesc [const AjPSeqDesc] AJAX Sequence Description
** @param [r] level [ajuint] Indentation level
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
** @@
******************************************************************************/

AjBool ensTraceSeqdesc(const AjPSeqDesc seqdesc, ajuint level)
{
    AjIList iter = NULL;

    AjPStr indent = NULL;
    AjPStr value = NULL;

    if(!seqdesc)
        return ajFalse;

    indent = ajStrNew();

    ajStrAppendCountK(&indent, ' ', level * 2);

    ajDebug("%SensTraceSeqdesc %p\n"
            "%S  Name      '%S'\n"
            "%S  Short      %p\n"
            "%S  EC         %p\n"
            "%S  AltNames   %p\n"
            "%S  SubNames   %p\n"
            "%S  Includes   %p\n"
            "%S  Contains   %p\n"
            "%S  Precursor '%B'\n"
            "%S  Fragments  %u\n",
            indent, seqdesc,
            indent, seqdesc->Name,
            indent, seqdesc->Short,
            indent, seqdesc->EC,
            indent, seqdesc->AltNames,
            indent, seqdesc->SubNames,
            indent, seqdesc->Includes,
            indent, seqdesc->Contains,
            indent, seqdesc->Precursor,
            indent, seqdesc->Fragments);

    /* Trace the AJAX List of AJAX String short names. */

    if(seqdesc->Short)
    {
        ajDebug("%S    AJAX List of AJAX String short names:\n", indent);

        iter = ajListIterNew(seqdesc->Short);

        while(!ajListIterDone(iter))
        {
            value = (AjPStr) ajListIterGet(iter);

            ajDebug("%S      '%S'\n", indent, value);
        }

        ajListIterDel(&iter);
    }

    /* Trace the AJAX List of AJAX String EC numbers. */

    if(seqdesc->EC)
    {
        ajDebug("%S    AJAX List of AJAX String EC numbers:\n", indent);

        iter = ajListIterNew(seqdesc->EC);

        while(!ajListIterDone(iter))
        {
            value = (AjPStr) ajListIterGet(iter);

            ajDebug("%S      '%S'\n", indent, value);
        }

        ajListIterDel(&iter);
    }

    ajStrDel(&indent);

    return ajTrue;
}




/* @func ensTraceSeqin ********************************************************
**
** Trace an AJAX Sequence Input object.
**
** @param [r] seqin [const AjPSeqin] AJAX Sequence Input
** @param [r] level [ajuint] Indentation level
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
** @@
******************************************************************************/

AjBool ensTraceSeqin(const AjPSeqin seqin, ajuint level)
{
    AjPStr indent = NULL;

    indent = ajStrNew();

    ajStrAppendCountK(&indent, ' ', level * 2);

    ajDebug("%SensTraceSeqin %p\n"
            "%S  Textin        %p\n"
            "%S  Name         '%S'\n"
            "%S  Acc          '%S'\n"
            "%S  Inputtype    '%S'\n"
            "%S  Type         '%S'\n"
            "%S  Full         '%S'\n"
            "%S  Date         '%S'\n"
            "%S  Desc         '%S'\n"
            "%S  Doc          '%S'\n"
            "%S  Inseq        '%S'\n"
            "%S  DbSequence   '%S'\n"
            "%S  Usalist       %p\n"
            "%S  Begin         %d\n"
            "%S  End           %d\n"
            "%S  Ufo          '%S'\n"
            "%S  Fttable       %p\n"
            "%S  Ftquery       %p\n"
            "%S  Entryname    '%S'\n"
            "%S  Features     '%B'\n"
            "%S  IsNuc        '%B'\n"
            "%S  IsProt       '%B'\n"
            "%S  multiset     '%B'\n"
            "%S  multidone    '%B'\n"
            "%S  Lower        '%B'\n"
            "%S  Upper        '%B'\n"
            "%S  Rev          '%B'\n"
            "%S  SeqData       %p\n",
            indent, seqin,
            indent, seqin->Input,
            indent, seqin->Name,
            indent, seqin->Acc,
            indent, seqin->Inputtype,
            indent, seqin->Type,
            indent, seqin->Full,
            indent, seqin->Date,
            indent, seqin->Desc,
            indent, seqin->Doc,
            indent, seqin->Inseq,
            indent, seqin->DbSequence,
            indent, seqin->Usalist,
            indent, seqin->Begin,
            indent, seqin->End,
            indent, seqin->Ufo,
            indent, seqin->Fttable,
            indent, seqin->Ftquery,
            indent, seqin->Entryname,
            indent, seqin->Features,
            indent, seqin->IsNuc,
            indent, seqin->IsProt,
            indent, seqin->Multiset,
            indent, seqin->Multidone,
            indent, seqin->Lower,
            indent, seqin->Upper,
            indent, seqin->Rev,
            indent, seqin->SeqData);

    ajStrDel(&indent);

    ensTraceTextin(seqin->Input, level + 1);

    return ajTrue;
}




/* @func ensTraceTextin *******************************************************
**
** Trace an AJAX Text Input object.
**
** @param [r] textin [const AjPTextin] AJAX Text Input
** @param [r] level [ajuint] Indentation level
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
** @@
******************************************************************************/

AjBool ensTraceTextin(const AjPTextin textin, ajuint level)
{
    AjPStr indent = NULL;

    if(!textin)
        return ajFalse;

    indent = ajStrNew();

    ajStrAppendCountK(&indent, ' ', level * 2);

    ajDebug("%SensTraceTextin  %p\n"
            "%S  Db           '%S'\n"
            "%S  Qry          '%S'\n"
            "%S  Formatstr    '%S'\n"
            "%S  Filename     '%S'\n"
            "%S  List          %p\n"
            "%S  Filebuff      %p\n"
            "%S  Fpos          %Ld\n"
            "%S  Query         %p\n"
            "%S  TextData      %p\n"
            "%S  Search       '%B'\n"
            "%S  Single       '%B'\n"
            "%S  Multi        '%B'\n"
            "%S  CaseId       '%B'\n"
            "%S  Text         '%B'\n"
            "%S  ChunkEntries '%B'\n"
            "%S  Count         %u\n"
            "%S  Filecount     %u\n"
            "%S  Entrycount    %u\n"
            "%S  Records       %u\n"
            "%S  Format        %d\n",
            indent, textin,
            indent, textin->Db,
            indent, textin->Qry,
            indent, textin->Formatstr,
            indent, textin->Filename,
            indent, textin->List,
            indent, textin->Filebuff,
            indent, textin->Fpos,
            indent, textin->Query,
            indent, textin->TextData,
            indent, textin->Search,
            indent, textin->Single,
            indent, textin->Multi,
            indent, textin->CaseId,
            indent, textin->Text,
            indent, textin->ChunkEntries,
            indent, textin->Count,
            indent, textin->Filecount,
            indent, textin->Entrycount,
            indent, textin->Records,
            indent, textin->Format);

    ajStrDel(&indent);

    ensTraceQuery(textin->Query, level + 1);

    return ajTrue;
}
