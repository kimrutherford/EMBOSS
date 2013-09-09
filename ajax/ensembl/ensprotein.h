/* @include ensprotein ********************************************************
**
** Ensembl Protein Feature functions
**
** @author Copyright (C) 1999 Ensembl Developers
** @author Copyright (C) 2006 Michael K. Schuster
** @version $Revision: 1.7 $
** @modified 2009 by Alan Bleasby for incorporation into EMBOSS core
** @modified $Date: 2012/08/05 11:22:27 $ by $Author: mks $
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

#ifndef ENSPROTEIN_H
#define ENSPROTEIN_H

/* ========================================================================= */
/* ============================= include files ============================= */
/* ========================================================================= */

#include "ensfeature.h"

AJ_BEGIN_DECLS




/* ========================================================================= */
/* =============================== constants =============================== */
/* ========================================================================= */

/* @const EnsPProteinfeatureadaptor *******************************************
**
** Ensembl Protein Feature Adaptor.
** Defined as an alias in EnsPBaseadaptor.
**
** #alias EnsPBaseadaptor
**
** #cc Bio::EnsEMBL::DBSQL::ProteinFeatureAdaptor
** ##
******************************************************************************/

#define EnsPProteinfeatureadaptor EnsPBaseadaptor




/* ========================================================================= */
/* ============================== public data ============================== */
/* ========================================================================= */

/* @data EnsPProteinfeature ***************************************************
**
** Ensembl Protein Feature.
**
** @alias EnsSProteinfeature
** @alias EnsOProteinfeature
**
** @attr Use [ajuint] Use counter
** @cc Bio::EnsEMBL::Storable
** @attr Identifier [ajuint] SQL database-internal identifier
** @attr Adaptor [EnsPProteinfeatureadaptor] Ensembl Protein Feature Adaptor
** @cc Bio::EnsEMBL::Featurepair
** @attr Featurepair [EnsPFeaturepair] Ensembl Feature Pair
** @cc Bio::EnsEMBL::Proteinfeature
** @attr Accession [AjPStr] (InterPro) Accession
** @attr Description [AjPStr] (InterPro) Description
** @@
******************************************************************************/

typedef struct EnsSProteinfeature
{
    ajuint Use;
    ajuint Identifier;
    EnsPProteinfeatureadaptor Adaptor;
    EnsPFeaturepair Featurepair;
    AjPStr Accession;
    AjPStr Description;
} EnsOProteinfeature;

#define EnsPProteinfeature EnsOProteinfeature*




/* ========================================================================= */
/* =========================== public functions ============================ */
/* ========================================================================= */

/*
** Prototype definitions
*/

/* Ensembl Protein Feature */

EnsPProteinfeature ensProteinfeatureNewCpy(const EnsPProteinfeature object);

EnsPProteinfeature ensProteinfeatureNewIni(
    EnsPProteinfeatureadaptor pfa,
    ajuint identifier,
    EnsPFeaturepair fp,
    AjPStr accession,
    AjPStr description);

EnsPProteinfeature ensProteinfeatureNewRef(EnsPProteinfeature pf);

void ensProteinfeatureDel(EnsPProteinfeature *Ppf);

AjPStr ensProteinfeatureGetAccession(
    const EnsPProteinfeature pf);

EnsPProteinfeatureadaptor ensProteinfeatureGetAdaptor(
    const EnsPProteinfeature pf);

AjPStr ensProteinfeatureGetDescription(
    const EnsPProteinfeature pf);

EnsPFeaturepair ensProteinfeatureGetFeaturepair(
    const EnsPProteinfeature pf);

ajuint ensProteinfeatureGetIdentifier(
    const EnsPProteinfeature pf);

AjBool ensProteinfeatureSetAccession(EnsPProteinfeature pf,
                                     AjPStr accession);

AjBool ensProteinfeatureSetAdaptor(EnsPProteinfeature pf,
                                   EnsPProteinfeatureadaptor pfa);

AjBool ensProteinfeatureSetDescription(EnsPProteinfeature pf,
                                       AjPStr description);

AjBool ensProteinfeatureSetFeaturepair(EnsPProteinfeature pf,
                                       EnsPFeaturepair fp);

AjBool ensProteinfeatureSetIdentifier(EnsPProteinfeature pf,
                                      ajuint identifier);

AjBool ensProteinfeatureTrace(const EnsPProteinfeature pf, ajuint level);

size_t ensProteinfeatureCalculateMemsize(const EnsPProteinfeature pf);

/* Ensembl Protein Feature Adaptor */

EnsPProteinfeatureadaptor ensRegistryGetProteinfeatureadaptor(
    EnsPDatabaseadaptor dba);

EnsPProteinfeatureadaptor ensProteinfeatureadaptorNew(
    EnsPDatabaseadaptor dba);

void ensProteinfeatureadaptorDel(EnsPProteinfeatureadaptor *Ppfa);

EnsPBaseadaptor ensProteinfeatureadaptorGetBaseadaptor(
    EnsPProteinfeatureadaptor pfa);

EnsPDatabaseadaptor ensProteinfeatureadaptorGetDatabaseadaptor(
    EnsPProteinfeatureadaptor pfa);

AjBool ensProteinfeatureadaptorFetchAllbyTranslationidentifier(
    EnsPProteinfeatureadaptor pfa,
    ajuint tlid,
    AjPList pfs);

AjBool ensProteinfeatureadaptorFetchByIdentifier(
    EnsPProteinfeatureadaptor pfa,
    ajuint identifier,
    EnsPProteinfeature *Ppf);

/*
** End of prototype definitions
*/




AJ_END_DECLS

#endif /* !ENSPROTEIN_H */
