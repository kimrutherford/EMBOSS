#ifdef __cplusplus
extern "C"
{
#endif

#ifndef ensgvdata_h
#define ensgvdata_h

#include "ensfeature.h"




/* @data EnsPGvsampleadaptor **************************************************
**
** Ensembl Genetic Variation Sample Adaptor.
**
** @alias EnsPBaseadaptor
** @@
******************************************************************************/

#define EnsPGvsampleadaptor EnsPBaseadaptor




/******************************************************************************
**
** Ensembl Genetic Variation Sample Display enumeration.
**
******************************************************************************/

enum EnsEGvsampleDisplay
{
    ensEGvsampleDisplayNULL,
    ensEGvsampleDisplayReference,
    ensEGvsampleDisplayDefault,
    ensEGvsampleDisplayDisplayable,
    ensEGvsampleDisplayUndisplayable
};




/* @data EnsPGvsample *********************************************************
**
** Ensembl Genetic Variation Sample.
**
** @alias EnsSGvsample
** @alias EnsOGvsample
**
** @attr Use [ajuint] Use counter
** @cc Bio::EnsEMBL::Storable
** @attr Identifier [ajuint] SQL database-internal identifier
** @attr Adaptor [EnsPGvsampleadaptor] Ensembl Genetic Variation
**                                     Sample Adaptor
** @cc Bio::EnsEMBL::Variation::Sample
** @attr Name [AjPStr] Name
** @attr Description [AjPStr] Description
** @attr Display [AjEnum] Display
** @attr Size [ajuint] Size
** @@
******************************************************************************/

typedef struct EnsSGvsample
{
    ajuint Use;
    ajuint Identifier;
    EnsPGvsampleadaptor Adaptor;
    AjPStr Name;
    AjPStr Description;
    AjEnum Display;
    ajuint Size;
} EnsOGvsample;

#define EnsPGvsample EnsOGvsample*




/* @data EnsPGvindividualadaptor **********************************************
**
** Ensembl Genetic Variation Individual Adaptor.
**
** @alias EnsPBaseadaptor
** @@
******************************************************************************/

#define EnsPGvindividualadaptor EnsPBaseadaptor




/******************************************************************************
**
** Ensembl Genetic Variation Individual Gender enumeration.
**
******************************************************************************/

enum EnsEGvindividualGender
{
    ensEGvindividualGenderNULL,
    ensEGvindividualGenderMale,
    ensEGvindividualGenderFemale,
    ensEGvindividualGenderUnknown
};




/******************************************************************************
**
** Ensembl Genetic Variation Individual Type enumeration.
**
******************************************************************************/

enum EnsEGvindividualType
{
    ensEGvindividualTypeNULL,
    ensEGvindividualTypeFullyInbred,
    ensEGvindividualTypePartlyInbred,
    ensEGvindividualTypeOutbred,
    ensEGvindividualTypeMutant,
    ensEGvindividualTypeUnknown
};




/* @data EnsPGvindividual *****************************************************
**
** Ensembl Genetic Variation Individual.
**
** @alias EnsSGvindividual
** @alias EnsOGvindividual
**
** @attr Use [ajuint] Use counter
** @cc Bio::EnsEMBL::Storable
** @attr Identifier [ajuint] SQL database-internal identifier
** @attr Adaptor [EnsPGvindividualadaptor] Ensembl Genetic Variation
**                                         Individual Adaptor
** @cc Bio::EnsEMBL::Variation::Sample
** @attr Gvsample [EnsPGvsample] Sample
** @cc Bio::EnsEMBL::Variation::Individual
** @attr Father [struct EnsSGvindividual*] Father Individual
** @attr Mother [struct EnsSGvindividual*] Mother Individual
** @attr Gender [AjEnum] Gender
** @attr Type [AjEnum] Type
** @attr Description [AjPStr] Description
** @@
******************************************************************************/

typedef struct EnsSGvindividual
{
    ajuint Use;
    ajuint Identifier;
    EnsPGvindividualadaptor Adaptor;
    EnsPGvsample Gvsample;
    struct EnsSGvindividual* Father;
    struct EnsSGvindividual* Mother;
    AjEnum Gender;
    AjEnum Type;
    AjPStr Description;
} EnsOGvindividual;

#define EnsPGvindividual EnsOGvindividual*




/* @data EnsPGvpopulationadaptor **********************************************
**
** Ensembl Genetic Variation Population Adaptor.
**
** @alias EnsPBaseadaptor
** @@
******************************************************************************/

#define EnsPGvpopulationadaptor EnsPBaseadaptor




/* @data EnsPGvpopulation *****************************************************
**
** Ensembl Genetic Variation Population.
**
** @alias EnsSGvpopulation
** @alias EnsOGvpopulation
**
** @attr Use [ajuint] Use counter
** @cc Bio::EnsEMBL::Storable
** @attr Identifier [ajuint] SQL database-internal identifier
** @attr Adaptor [EnsPGvpopulationadaptor] Ensembl Genetic Variation
**                                         Population Adaptor
** @cc Bio::EnsEMBL::Variation::Sample
** @attr Gvsample [EnsPGvsample] Sample
** @cc Bio::EnsEMBL::Variation::Population
** @attr SubPopulations [AjPList] AJAX List of Ensembl Genetic Variation
**                                (Sub-) Populations
** @@
******************************************************************************/

typedef struct EnsSGvpopulation
{
    ajuint Use;
    ajuint Identifier;
    EnsPGvpopulationadaptor Adaptor;
    EnsPGvsample Gvsample;
    AjPList SubPopulations;
} EnsOGvpopulation;

#define EnsPGvpopulation EnsOGvpopulation*




#endif

#ifdef __cplusplus
}
#endif
