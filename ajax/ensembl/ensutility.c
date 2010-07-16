/******************************************************************************
** @source Ensembl Utility functions
**
** @author Copyright (C) 1999 Ensembl Developers
** @author Copyright (C) 2006 Michael K. Schuster
** @modified 2009 by Alan Bleasby for incorporation into EMBOSS core
** @version $Revision: 1.6 $
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

#include "ensutility.h"




/* ==================================================================== */
/* ========================== private data ============================ */
/* ==================================================================== */

/* utilityInit ****************************************************************
**
** Private boolean variable to ascertain that ensInit has been called once
** and only once.
**
******************************************************************************/

static AjBool utilityInit = AJFALSE;




/* ==================================================================== */
/* ======================== private functions ========================= */
/* ==================================================================== */




/* @filesection ensutility ****************************************************
**
** @nam1rule ens Function belongs to the Ensembl library
**
******************************************************************************/




/* @datasection [none] Internals **********************************************
**
** Functions control Ensembl internals
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
**
******************************************************************************/




/* @func ensInit **************************************************************
**
** Initialises the Ensembl library.
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
**
******************************************************************************/




/* @func ensExit **************************************************************
**
** Frees the Ensembl library.
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

    return;
}




/* @func ensSeqinTrace ********************************************************
**
** Trace an AJAX Sequence Input object.
**
** @param [r] seqin [const AjPSeqin] AJAX Sequence Input
** @param [r] level [ajuint] Indentation level
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
** @@
******************************************************************************/

AjBool ensSeqinTrace(const AjPSeqin seqin, ajuint level)
{
    AjPStr indent = NULL;

    indent = ajStrNew();

    ajStrAppendCountK(&indent, ' ', level * 2);

    ajDebug("%SensSeqinTrace %p\n"
            "%S  Name      '%S'\n"
            "%S  Acc       '%S'\n"
            "%S  Inputtype '%S'\n"
            "%S  Type      '%S'\n"
            "%S  Db        '%S'\n"
            "%S  Full      '%S'\n"
            "%S  Date      '%S'\n"
            "%S  Desc      '%S'\n"
            "%S  Doc       '%S'\n"
            "%S  Inseq     '%S'\n"
            "%S  Begin      %d\n"
            "%S  End        %d\n"
            "%S  List       %p\n"
            "%S  Usa       '%S'\n"
            "%S  Ufo       '%S'\n"
            "%S  Fttable    %p\n"
            "%S  Ftquery    %p\n"
            "%S  Formatstr '%S'\n"
            "%S  Filename  '%S'\n"
            "%S  Entryname '%S'\n"
            "%S  Filebuff   %p\n"
            "%S  Search    '%B'\n"
            "%S  Single    '%B'\n"
            "%S  CaseId    '%B'\n"
            "%S  Features  '%B'\n"
            "%S  IsNuc     '%B'\n"
            "%S  IsProt    '%B'\n"
            "%S  multi     '%B'\n"
            "%S  multiset  '%B'\n"
            "%S  multidone '%B'\n"
            "%S  Lower     '%B'\n"
            "%S  Upper     '%B'\n"
            "%S  Text      '%B'\n"
            "%S  Count      %d\n"
            "%S  Filecount  %d\n"
            "%S  Fileseqs   %d\n"
            "%S  Rev       '%B'\n"
            "%S  Fpos       %Ld\n"
            "%S  Query      %p\n"
            "%S  Data       %p\n"
            "%S  Format     %d\n"
            "%S  Records    %u\n",
            indent, seqin,
            indent, seqin->Name,
            indent, seqin->Acc,
            indent, seqin->Inputtype,
            indent, seqin->Type,
            indent, seqin->Db,
            indent, seqin->Full,
            indent, seqin->Date,
            indent, seqin->Desc,
            indent, seqin->Doc,
            indent, seqin->Inseq,
            indent, seqin->Begin,
            indent, seqin->End,
            indent, seqin->List,
            indent, seqin->Usa,
            indent, seqin->Ufo,
            indent, seqin->Fttable,
            indent, seqin->Ftquery,
            indent, seqin->Formatstr,
            indent, seqin->Filename,
            indent, seqin->Entryname,
            indent, seqin->Filebuff,
            indent, seqin->Search,
            indent, seqin->Single,
            indent, seqin->CaseId,
            indent, seqin->Features,
            indent, seqin->IsNuc,
            indent, seqin->IsProt,
            indent, seqin->multi,
            indent, seqin->multiset,
            indent, seqin->multidone,
            indent, seqin->Lower,
            indent, seqin->Upper,
            indent, seqin->Text,
            indent, seqin->Count,
            indent, seqin->Filecount,
            indent, seqin->Fileseqs,
            indent, seqin->Rev,
            indent, seqin->Fpos,
            indent, seqin->Query,
            indent, seqin->Data,
            indent, seqin->Format,
            indent, seqin->Records);

    ajStrDel(&indent);

    ensSeqQueryTrace(seqin->Query, level + 1);

    return ajTrue;
}




/* @func ensSeqQueryTrace *****************************************************
**
** Trace an AJAX Sequence Query object.
**
** @param [r] seqquery [const AjPSeqQuery] AJAX Sequence Query
** @param [r] level [ajuint] Indentation level
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
** @@
******************************************************************************/

AjBool ensSeqQueryTrace(const AjPSeqQuery seqquery, ajuint level)
{
    AjPStr indent = NULL;

    indent = ajStrNew();

    ajStrAppendCountK(&indent, ' ', level * 2);

    ajDebug("%SensSeqQueryTrace %p\n"
            "%S  DbName       '%S'\n"
            "%S  DbAlias      '%S'\n"
            "%S  DbType       '%S'\n"
            "%S  Id           '%S'\n"
            "%S  Acc          '%S'\n"
            "%S  Des          '%S'\n"
            "%S  Key          '%S'\n"
            "%S  Org          '%S'\n"
            "%S  Sv           '%S'\n"
            "%S  Gi           '%S'\n"
            "%S  CaseId       '%B'\n"
            "%S  HasAcc       '%B'\n"
            "%S  Method       '%S'\n"
            "%S  Formatstr    '%S'\n"
            "%S  IndexDir     '%S'\n"
            "%S  Directory    '%S'\n"
            "%S  Filename     '%S'\n"
            "%S  Exclude      '%S'\n"
            "%S  DbFields     '%S'\n"
            "%S  DbFilter     '%S'\n"
            "%S  DbProxy      '%S'\n"
            "%S  DbHttpVer    '%S'\n"
            "%S  DbIdentifier '%S'\n"
            "%S  DbAccession  '%S'\n"
            "%S  DbSequence   '%S'\n"
            "%S  DbReturn     '%S'\n"
            "%S  Field        '%S'\n"
            "%S  QryString    '%S'\n"
            "%S  Application  '%S'\n"
            "%S  Fpos          %Ld\n"
            "%S  Type          %d\n"
            "%S  QryDone      '%B'\n"
            "%S  Access        %p\n"
            "%S  QryData       %p\n"
            "%S  Wild         '%B'\n",
            indent, seqquery,
            indent, seqquery->DbName,
            indent, seqquery->DbAlias,
            indent, seqquery->DbType,
            indent, seqquery->Id,
            indent, seqquery->Acc,
            indent, seqquery->Des,
            indent, seqquery->Key,
            indent, seqquery->Org,
            indent, seqquery->Sv,
            indent, seqquery->Gi,
            indent, seqquery->CaseId,
            indent, seqquery->HasAcc,
            indent, seqquery->Method,
            indent, seqquery->Formatstr,
            indent, seqquery->IndexDir,
            indent, seqquery->Directory,
            indent, seqquery->Filename,
            indent, seqquery->Exclude,
            indent, seqquery->DbFields,
            indent, seqquery->DbFilter,
            indent, seqquery->DbProxy,
            indent, seqquery->DbHttpVer,
            indent, seqquery->DbIdentifier,
            indent, seqquery->DbAccession,
            indent, seqquery->DbSequence,
            indent, seqquery->DbReturn,
            indent, seqquery->Field,
            indent, seqquery->QryString,
            indent, seqquery->Application,
            indent, seqquery->Fpos,
            indent, seqquery->Type,
            indent, seqquery->QryDone,
            indent, seqquery->Access,
            indent, seqquery->QryData,
            indent, seqquery->Wild);

    ajStrDel(&indent);

    return ajTrue;
}




/* @func ensSeqDescTrace ******************************************************
**
** Trace an AJAX Sequence Description object.
**
** @param [r] seqdesc [const AjPSeqDesc] AJAX Sequence Description
** @param [r] level [ajuint] Indentation level
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
** @@
******************************************************************************/

AjBool ensSeqDescTrace(const AjPSeqDesc seqdesc, ajuint level)
{
    AjIList iter = NULL;

    AjPStr indent = NULL;
    AjPStr value = NULL;

    if(!seqdesc)
        return ajFalse;

    indent = ajStrNew();

    ajStrAppendCountK(&indent, ' ', level * 2);

    ajDebug("%SensSeqDescTrace %p\n"
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




/* @func ensSeqTrace **********************************************************
**
** Trace an AJAX Sequence object.
**
** @param [r] seq [const AjPSeq] AJAX Sequence
** @param [r] level [ajuint] Indentation level
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
** @@
******************************************************************************/

AjBool ensSeqTrace(const AjPSeq seq, ajuint level)
{
    AjPStr indent = NULL;

    if(!seq)
        return ajFalse;

    indent = ajStrNew();

    ajStrAppendCountK(&indent, ' ', level * 2);

    ajDebug("%SensSeqTrace %p\n"
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
            "%S  Fpos       %Lu\n"
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
