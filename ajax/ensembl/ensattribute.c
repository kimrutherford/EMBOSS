/******************************************************************************
** @source Ensembl Attribute functions.
**
** @author Copyright (C) 1999 Ensembl Developers
** @author Copyright (C) 2006 Michael K. Schuster
** @modified 2009 by Alan Bleasby for incorporation into EMBOSS core
** @version $Revision: 1.3 $
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

#include "ensattribute.h"
#include "ensgene.h"
#include "enstranscript.h"
#include "enstranslation.h"




/* ==================================================================== */
/* ========================== private data ============================ */
/* ==================================================================== */




/* ==================================================================== */
/* ======================== private functions ========================= */
/* ==================================================================== */

static AjBool attributeAdaptorFetchAllBySQL(EnsPDatabaseadaptor adaptor,
                                            const AjPStr statement,
                                            AjPList attributes);




/* @filesection ensattribute **************************************************
**
** @nam1rule ens Function belongs to the Ensembl library
**
******************************************************************************/




/* @datasection [EnsPAttribute] Attribute *************************************
**
** Functions for manipulating Ensembl Attribute objects
**
** @cc Bio::EnsEMBL::Attribute CVS Revision: 1.7
**
** @nam2rule Attribute
**
******************************************************************************/




/* @section constructors ******************************************************
**
** All constructors return a new Ensembl Attribute by pointer.
** It is the responsibility of the user to first destroy any previous
** Attribute. The target pointer does not need to be initialised to
** NULL, but it is good programming practice to do so anyway.
**
** @fdata [EnsPAttribute]
** @fnote None
**
** @nam3rule New Constructor
** @nam4rule NewObj Constructor with existing object
** @nam4rule NewRef Constructor by incrementing the reference counter
**
** @argrule Obj object [EnsPAttribute] Ensembl Attribute
** @argrule Ref object [EnsPAttribute] Ensembl Attribute
**
** @valrule * [EnsPAttribute] Ensembl Attribute
**
** @fcategory new
******************************************************************************/




/* @func ensAttributeNew ******************************************************
**
** Default Ensembl Attribute constructor.
**
** @cc Bio::EnsEMBL::Attribute::new
** @param [u] code [AjPStr] Code
** @param [u] name [AjPStr] Name
** @param [u] description [AjPStr] Description
** @param [u] value [AjPStr] Value
**
** @return [EnsPAttribute] Ensembl Attribute or NULL
** @@
******************************************************************************/

EnsPAttribute ensAttributeNew(AjPStr code, AjPStr name, AjPStr description,
                              AjPStr value)
{
    EnsPAttribute attribute = NULL;
    
    AJNEW0(attribute);
    
    if(code)
	attribute->Code = ajStrNewRef(code);
    
    if(name)
	attribute->Name = ajStrNewRef(name);
    
    if(description)
	attribute->Description = ajStrNewRef(description);
    
    if(value)
	attribute->Value = ajStrNewRef(value);
    
    attribute->Use = 1;
    
    return attribute;
}




/* @func ensAttributeNewObj ***************************************************
**
** Object-based constructor function, which returns an independent object.
**
** @param [r] object [const EnsPAttribute] Ensembl Attribute
**
** @return [EnsPAttribute] Ensembl Attribute or NULL
** @@
******************************************************************************/

EnsPAttribute ensAttributeNewObj(const EnsPAttribute object)
{
    EnsPAttribute attribute = NULL;
    
    AJNEW0(attribute);
    
    if(object->Code)
        attribute->Code = ajStrNewRef(object->Code);
    
    if(object->Name)
        attribute->Name = ajStrNewRef(object->Name);
    
    if(object->Description)
        attribute->Description = ajStrNewRef(object->Description);
    
    if(object->Value)
        attribute->Value = ajStrNewRef(object->Value);
    
    attribute->Use = 1;
    
    return attribute;
}




/* @func ensAttributeNewRef ***************************************************
**
** Ensembl Object referencing function, which returns a pointer to the
** Ensembl Object passed in and increases its reference count.
**
** @param [u] feature [EnsPAttribute] Ensembl Attribute
**
** @return [EnsPAttribute] Ensembl Attribute or NULL
** @@
******************************************************************************/

EnsPAttribute ensAttributeNewRef(EnsPAttribute attribute)
{
    if(!attribute)
	return NULL;
    
    attribute->Use++;
    
    return attribute;
}




/* @section destructors *******************************************************
**
** Destruction destroys all internal data structures and frees the
** memory allocated for the Ensembl Attributes.
**
** @fdata [EnsPAttribute]
** @fnote None
**
** @nam3rule Del Destroy (free) an Attribute object
**
** @argrule * Pattribute [EnsPAttribute*] Attribute object address
**
** @valrule * [void]
**
** @fcategory delete
******************************************************************************/




/* @func ensAttributeDel ******************************************************
**
** Default destructor for an Ensembl Attribute.
**
** @param [d] Pattribute [EnsPAttribute*] Ensembl Attribute address
**
** @return [void]
** @@
******************************************************************************/

void ensAttributeDel(EnsPAttribute* Pattribute)
{
    EnsPAttribute pthis = NULL;
    
    if(!Pattribute)
        return;
    
    if(!*Pattribute)
        return;

    pthis = *Pattribute;
    
    pthis->Use--;
    
    if(pthis->Use)
    {
	*Pattribute = NULL;
	
	return;
    }
    
    ajStrDel(&pthis->Code);
    
    ajStrDel(&pthis->Name);
    
    ajStrDel(&pthis->Description);
    
    ajStrDel(&pthis->Value);
    
    AJFREE(pthis);

    *Pattribute = NULL;
    
    return;
}




/* @section element retrieval *************************************************
**
** Functions for returning elements of an Ensembl Attribute object.
**
** @fdata [EnsPAttribute]
** @fnote None
**
** @nam3rule Get Return Attribute attribute(s)
** @nam4rule GetCode Return the code
** @nam4rule GetName Return the name
** @nam4rule GetDescription Return the description
** @nam4rule GetValue Return the value
**
** @argrule * attribute [const EnsPAttribute] Attribute
**
** @valrule Code [AjPStr] Code
** @valrule Name [AjPStr] Name
** @valrule Description [AjPStr] Description
** @valrule Value [AjPStr] Value
**
** @fcategory use
******************************************************************************/




/* @func ensAttributeGetCode **************************************************
**
** Get the code element of an Ensembl Attribute.
**
** @cc Bio::EnsEMBL::Attribute::code
** @param [r] attribute [const EnsPAttribute] Ensembl Attribute
**
** @return [AjPStr] Code
** @@
******************************************************************************/

AjPStr ensAttributeGetCode(const EnsPAttribute attribute)
{
    if(!attribute)
        return NULL;
    
    return attribute->Code;
}




/* @func ensAttributeGetName **************************************************
**
** Get the name element of an Ensembl Attribute.
**
** @cc Bio::EnsEMBL::Attribute::name
** @param [r] attribute [const EnsPAttribute] Ensembl Attribute
**
** @return [AjPStr] Name
** @@
******************************************************************************/

AjPStr ensAttributeGetName(const EnsPAttribute attribute)
{
    if(!attribute)
        return NULL;
    
    return attribute->Name;
}




/* @func ensAttributeGetDescription *******************************************
**
** Get the description element of an Ensembl Attribute.
**
** @cc Bio::EnsEMBL::Attribute::description
** @param [r] attribute [const EnsPAttribute] Ensembl Attribute
**
** @return [AjPStr] Description
** @@
******************************************************************************/

AjPStr ensAttributeGetDescription(const EnsPAttribute attribute)
{
    if(!attribute)
        return NULL;
    
    return attribute->Description;
}




/* @func ensAttributeGetValue *************************************************
**
** Get the value element of an Ensembl Attribute.
**
** @cc Bio::EnsEMBL::Attribute::value
** @param [r] attribute [const EnsPAttribute] Ensembl Attribute
**
** @return [AjPStr] Value
** @@
******************************************************************************/

AjPStr ensAttributeGetValue(const EnsPAttribute attribute)
{
    if(!attribute)
        return NULL;
    
    return attribute->Value;
}




/* @func ensAttributeGetMemSize ***********************************************
**
** Get the memory size in bytes of an Ensembl Attribute.
**
** @param [r] attribute [const EnsPAttribute] Ensembl Attribute
**
** @return [ajuint] Memory size
** @@
******************************************************************************/

ajuint ensAttributeGetMemSize(const EnsPAttribute attribute)
{
    ajuint size = 0;
    
    if(!attribute)
	return 0;
    
    size += (ajuint) sizeof (EnsOAttribute);
    
    if(attribute->Code)
    {
	size += (ajuint) sizeof (AjOStr);
	
	size += ajStrGetRes(attribute->Code);
    }
    
    if(attribute->Name)
    {
	size += (ajuint) sizeof (AjOStr);
	
	size += ajStrGetRes(attribute->Name);
    }
    
    if(attribute->Description)
    {
	size += (ajuint) sizeof (AjOStr);
	
	size += ajStrGetRes(attribute->Description);
    }
    
    if(attribute->Value)
    {
	size += (ajuint) sizeof (AjOStr);
	
	size += ajStrGetRes(attribute->Value);
    }
    
    return size;
}




/* @section debugging *********************************************************
**
** Functions for reporting of an Ensembl Attribute object.
**
** @fdata [EnsPAttribute]
** @nam3rule Trace Report Ensembl Attribute elements to debug file
**
** @argrule Trace attribute [const EnsPAttribute] Ensembl Attribute
** @argrule Trace level [ajuint] Indentation level
**
** @valrule * [AjBool] ajTrue upon success, ajFalse otherwise
**
** @fcategory misc
******************************************************************************/




/* @func ensAttributeTrace ****************************************************
**
** Trace an Ensembl Attribute.
**
** @param [r] attribute [const EnsPAttribute] Ensembl Attribute
** @param [r] level [ajuint] Indentation level
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
** @@
******************************************************************************/

AjBool ensAttributeTrace(const EnsPAttribute attribute, ajuint level)
{
    AjPStr indent = NULL;
    
    if(!attribute)
	return ajFalse;
    
    indent = ajStrNew();
    
    ajStrAppendCountK(&indent, ' ', level * 2);
    
    ajDebug("%SensAttributeTrace %p\n"
	    "%S  Code '%S'\n"
	    "%S  Name '%S'\n"
	    "%S  Description '%S'\n"
	    "%S  Value '%S'\n"
	    "%S  Use %u\n",
	    indent, attribute,
	    indent, attribute->Code,
	    indent, attribute->Name,
	    indent, attribute->Description,
	    indent, attribute->Value,
	    indent, attribute->Use);
    
    ajStrDel(&indent);
    
    return ajTrue;
}




/* @datasection [EnsPAttributeadaptor] Attribute Adaptor **********************
**
** Functions for manipulating Ensembl Attribute Adaptor objects
**
** @cc Bio::EnsEMBL::DBSQL::Attributeadaptor CVS Revision: 1.23
**
** @nam2rule Attributeadaptor
**
******************************************************************************/




/* @funcstatic attributeAdaptorFetchAllBySQL **********************************
**
** Run a SQL statement against an Ensembl Database Adaptor and consolidate the
** results into an AJAX List of Ensembl Attributes.
**
** The caller is responsible for deleting the Ensembl Attributes before
** deleting the AJAX List.
**
** @param [r] adaptor [EnsPDatabaseadaptor] Ensembl Database Adaptor
** @param [r] statement [const AjPStr] SQL statement
** @param [u] attributes [AjPList] AJAX List of Ensembl Attributes
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
** @@
******************************************************************************/

static AjBool attributeAdaptorFetchAllBySQL(EnsPDatabaseadaptor adaptor,
                                            const AjPStr statement,
                                            AjPList attributes)
{
    AjPSqlstatement sqls = NULL;
    AjISqlrow sqli       = NULL;
    AjPSqlrow sqlr       = NULL;
    
    AjPStr code        = NULL;
    AjPStr name        = NULL;
    AjPStr description = NULL;
    AjPStr value       = NULL;
    
    EnsPAttribute attribute = NULL;
    
    if(!adaptor)
	return ajFalse;
    
    if(!statement)
	return ajFalse;
    
    if(!attributes)
	return ajFalse;
    
    sqls = ensDatabaseadaptorSqlstatementNew(adaptor, statement);
    
    sqli = ajSqlrowiterNew(sqls);
    
    while(!ajSqlrowiterDone(sqli))
    {
	code        = ajStrNew();
	name        = ajStrNew();
	description = ajStrNew();
	value       = ajStrNew();
	
        sqlr = ajSqlrowiterGet(sqli);
	
        ajSqlcolumnToStr(sqlr, &code);
        ajSqlcolumnToStr(sqlr, &name);
	ajSqlcolumnToStr(sqlr, &description);
	ajSqlcolumnToStr(sqlr, &value);
	
	attribute = ensAttributeNew(code, name, description, value);
	
        ajListPushAppend(attributes, (void *) attribute);
	
	ajStrDel(&code);
	ajStrDel(&name);
	ajStrDel(&description);
	ajStrDel(&value);
    }
    
    ajSqlrowiterDel(&sqli);
    
    ajSqlstatementDel(&sqls);
    
    return ajTrue;
}




/* @section object retrieval **************************************************
**
** Functions for retrieving Ensembl Attribute objects from an
** Ensembl Core database.
**
** @fdata [EnsPAttributeadaptor]
** @fnote None
**
** @nam3rule Fetch Retrieve Ensembl Attribute object(s)
** @nam4rule FetchAll Retrieve all Ensembl Attribute objects
** @nam5rule FetchAllBy Retrieve all Ensembl Attribute objects
**                      matching a criterion
** @nam4rule FetchBy Retrieve one Ensembl Attribute object
**                   matching a criterion
**
** @argrule * adaptor [const EnsPAttributeadaptor] Ensembl Attribute Adaptor
** @argrule FetchAll attributes [AjPList] AJAX List of Ensembl Attribute
**                                        objects
**
** @valrule * [AjBool] ajTrue upon success, ajFalse otherwise
**
** @fcategory use
******************************************************************************/




/* @func ensAttributeadaptorFetchAllByGene ************************************
**
** Fetch all Ensembl Attributes via an Ensembl Gene.
**
** The caller is responsible for deleting the Ensembl Attributes before
** deleting the AJAX List.
**
** @cc Bio::EnsEMBL::DBSQL::Attributeadaptor::AUTOLOAD
** @cc Bio::EnsEMBL::DBSQL::Attributeadaptor::fetch_all_by_
** @param [r] adaptor [EnsPDatabaseadaptor] Ensembl Database Adaptor
** @param [r] gene [const EnsPGene] Ensembl Gene
** @param [r] code [const AjPStr] Ensembl Attribute code
** @param [u] attributes [AjPList] AJAX List of Ensembl Attributes
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
** @@
******************************************************************************/

/*
** FIXME: Although the Attribute adaptor now implements another SQL constraint
** for an attrib_type.code, the Bio::EnsEMBL::Gene::get_all_Attributes method
** does not use it.
** It gets all Attributes first and then uses the Perl grep function to return
** only the Attributes of a particular code. This should be changed in the
** Perl API.
*/

AjBool ensAttributeadaptorFetchAllByGene(EnsPDatabaseadaptor adaptor,
                                         const EnsPGene gene,
                                         const AjPStr code,
                                         AjPList attributes)
{
    char *txtcode = NULL;
    
    AjPStr statement = NULL;
    
    if(!adaptor)
	return ajFalse;
    
    if(!gene)
	return ajFalse;
    
    if(!attributes)
	return ajFalse;
    
    statement = ajFmtStr("SELECT "
			 "attrib_type.code, "
			 "attrib_type.name, "
			 "attrib_type.description, "
			 "gene_attrib.value "
			 "FROM "
			 "attrib_type, "
			 "gene_attrib "
			 "WHERE "
			 "attrib_type.attrib_type_id = "
			 "gene_attrib.attrib_type_id "
			 "AND "
			 "gene_attrib.gene_id = %u",
			 ensGeneGetIdentifier(gene));
    
    if(code && ajStrGetLen(code))
    {
	ensDatabaseadaptorEscapeC(adaptor, &txtcode, code);
	
	ajFmtPrintAppS(&statement, " AND attrib_type.code = '%s'", txtcode);
	
	ajCharDel(&txtcode);
    }
    
    attributeAdaptorFetchAllBySQL(adaptor, statement, attributes);
    
    ajStrDel(&statement);
    
    return ajTrue;
}




/* @func ensAttributeadaptorFetchAllBySeqregion *******************************
**
** Fetch all Ensembl Attributes via an Ensembl Sequence Region.
**
** The caller is responsible for deleting the Ensembl Attributes before
** deleting the AJAX List.
**
** @cc Bio::EnsEMBL::DBSQL::Attributeadaptor::AUTOLOAD
** @cc Bio::EnsEMBL::DBSQL::Attributeadaptor::fetch_all_by_
** @param [r] adaptor [EnsPDatabaseadaptor] Ensembl Database Adaptor.
** @param [r] sr [const EnsPSeqregion] Ensembl Sequence Region.
** @param [r] code [const AjPStr] Ensembl Attribute code.
** @param [u] attributes [AjPList] AJAX List of Ensembl Attributes.
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise.
** @@
******************************************************************************/

AjBool ensAttributeadaptorFetchAllBySeqregion(EnsPDatabaseadaptor adaptor,
                                              const EnsPSeqregion sr,
                                              const AjPStr code,
                                              AjPList attributes)
{
    char *txtcode = NULL;
    
    AjPStr statement = NULL;
    
    if(!adaptor)
	return ajFalse;
    
    if(!sr)
	return ajFalse;
    
    if(!attributes)
	return ajFalse;
    
    statement = ajFmtStr("SELECT "
			 "attrib_type.code, "
			 "attrib_type.name, "
			 "attrib_type.description, "
			 "seq_region_attrib.value "
			 "FROM "
			 "attrib_type, "
			 "seq_region_attrib "
			 "WHERE "
			 "attrib_type.attrib_type_id = "
			 "seq_region_attrib.attrib_type_id "
			 "AND "
			 "seq_region_attrib.seq_region_id = %u",
			 ensSeqregionGetIdentifier(sr));
    
    if(code && ajStrGetLen(code))
    {
	ensDatabaseadaptorEscapeC(adaptor, &txtcode, code);
	
	ajFmtPrintAppS(&statement, " AND attrib_type.code = '%s'", txtcode);
	
	ajCharDel(&txtcode);
    }
    
    attributeAdaptorFetchAllBySQL(adaptor, statement, attributes);
    
    ajStrDel(&statement);
    
    return ajTrue;
}




/* @func ensAttributeadaptorFetchAllBySlice ***********************************
**
** Fetch all Ensembl Attributes via an Ensembl Slice.
**
** The caller is responsible for deleting the Ensembl Attributes before
** deleting the AJAX List.
**
** @cc Bio::EnsEMBL::DBSQL::Attributeadaptor::AUTOLOAD
** @cc Bio::EnsEMBL::DBSQL::Attributeadaptor::fetch_all_by_
** @param [r] adaptor [EnsPDatabaseadaptor] Ensembl Database Adaptor
** @param [r] slice [const EnsPSlice] Ensembl Slice
** @param [r] code [const AjPStr] Ensembl Attribute code
** @param [u] attributes [AjPList] AJAX List of Ensembl Attributes
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
** @@
******************************************************************************/

/*
** FIXME: Although the Attribute adaptor now implements another SQL constraint
** for an attrib_type.code, the Bio::EnsEMBL::Slice::get_all_Attributes method
** does not use it.
** It gets all Attributes first and then uses the Perl grep function to return
** only the Attributes of a particular code. This should be changed in the
** Perl API.
*/

AjBool ensAttributeadaptorFetchAllBySlice(EnsPDatabaseadaptor adaptor,
                                          const EnsPSlice slice,
                                          const AjPStr code,
                                          AjPList attributes)
{
    EnsPSeqregion sr = NULL;
    
    if(!adaptor)
	return ajFalse;
    
    if(!slice)
	return ajFalse;
    
    if(!attributes)
	return ajFalse;
    
    sr = ensSliceGetSeqregion(slice);
    
    if(!sr)
    {
	ajDebug("ensAttributeadaptorFetchAllBySlice cannot get Attributes "
		"for a Slice without a Sequence Region.\n");
	
	return ajFalse;
    }
    
    return ensAttributeadaptorFetchAllBySeqregion(adaptor,
						  sr,
						  code,
						  attributes);
}




/* @func ensAttributeadaptorFetchAllByTranscript ******************************
**
** Fetch all Ensembl Attributes via an Ensembl Transcript.
**
** The caller is responsible for deleting the Ensembl Attributes before
** deleting the AJAX List.
**
** @cc Bio::EnsEMBL::DBSQL::Attributeadaptor::AUTOLOAD
** @cc Bio::EnsEMBL::DBSQL::Attributeadaptor::fetch_all_by_
** @param [r] adaptor [EnsPDatabaseadaptor] Ensembl Database Adaptor
** @param [r] transcript [const EnsPTranscript] Ensembl Transcript
** @param [r] code [const AjPStr] Ensembl Attribute code
** @param [u] attributes [AjPList] AJAX List of Ensembl Attributes
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
** @@
******************************************************************************/

AjBool ensAttributeadaptorFetchAllByTranscript(EnsPDatabaseadaptor adaptor,
                                               const EnsPTranscript transcript,
                                               const AjPStr code,
                                               AjPList attributes)
{
    char *txtcode = NULL;
    
    AjPStr statement = NULL;
    
    if(!adaptor)
	return ajFalse;
    
    if(!transcript)
	return ajFalse;
    
    if(!attributes)
	return ajFalse;
    
    statement = ajFmtStr("SELECT "
			 "attrib_type.code, "
			 "attrib_type.name, "
			 "attrib_type.description, "
			 "transcript_attrib.value "
			 "FROM "
			 "attrib_type, "
			 "transcript_attrib "
			 "WHERE "
			 "attrib_type.attrib_type_id = "
			 "transcript_attrib.attrib_type_id "
			 "AND "
			 "transcript_attrib.transcript_id = %u",
			 ensTranscriptGetIdentifier(transcript));
    
    if(code && ajStrGetLen(code))
    {
	ensDatabaseadaptorEscapeC(adaptor, &txtcode, code);
	
	ajFmtPrintAppS(&statement, " AND attrib_type.code = '%s'", txtcode);
	
	ajCharDel(&txtcode);
    }
    
    attributeAdaptorFetchAllBySQL(adaptor, statement, attributes);
    
    ajStrDel(&statement);
    
    return ajTrue;
}




/* @func ensAttributeadaptorFetchAllByTranslation *****************************
**
** Fetch all Ensembl Attributes via an Ensembl Translation.
**
** The caller is responsible for deleting the Ensembl Attributes before
** deleting the AJAX List.
**
** @cc Bio::EnsEMBL::DBSQL::Attributeadaptor::AUTOLOAD
** @cc Bio::EnsEMBL::DBSQL::Attributeadaptor::fetch_all_by_
** @param [r] adaptor [EnsPDatabaseadaptor] Ensembl Database Adaptor
** @param [r] translation [const EnsPTranslation] Ensembl Translation
** @param [r] code [const AjPStr] Ensembl Attribute code
** @param [u] attributes [AjPList] AJAX List of Ensembl Attributes
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
** @@
******************************************************************************/

AjBool ensAttributeadaptorFetchAllByTranslation(
    EnsPDatabaseadaptor adaptor,
    const EnsPTranslation translation,
    const AjPStr code,
    AjPList attributes)
{
    char *txtcode = NULL;
    
    AjPStr statement = NULL;
    
    if(!adaptor)
	return ajFalse;
    
    if(!translation)
	return ajFalse;
    
    if(!attributes)
	return ajFalse;
    
    statement = ajFmtStr("SELECT "
			 "attrib_type.code, "
			 "attrib_type.name, "
			 "attrib_type.description, "
			 "translation_attrib.value "
			 "FROM "
			 "attrib_type, "
			 "translation_attrib "
			 "WHERE "
			 "attrib_type.attrib_type_id = "
			 "translation_attrib.attrib_type_id "
			 "AND "
			 "translation_attrib.translation_id = %u",
			 ensTranslationGetIdentifier(translation));
    
    if(code && ajStrGetLen(code))
    {
	ensDatabaseadaptorEscapeC(adaptor, &txtcode, code);
	
	ajFmtPrintAppS(&statement, " AND attrib_type.code = '%s'", txtcode);
	
	ajCharDel(&txtcode);
    }
    
    attributeAdaptorFetchAllBySQL(adaptor, statement, attributes);
    
    ajStrDel(&statement);
    
    return ajTrue;
}
