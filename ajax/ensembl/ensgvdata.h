#ifdef __cplusplus
extern "C"
{
#endif

#ifndef ensgvdata_h
#define ensgvdata_h

#include "ensfeature.h"




/* #data EnsPGvsampleadaptor **************************************************
**
** Ensembl Genetic Variation Sample Adaptor.
** Defined as an alias in EnsPBaseadaptor
**
** #alias EnsPBaseadaptor
** ##
******************************************************************************/

#define EnsPGvsampleadaptor EnsPBaseadaptor




/* EnsEGvsampleDisplay ********************************************************
**
** Ensembl Genetic Variation Sample Display enumeration.
**
******************************************************************************/

typedef enum EnsOGvsampleDisplay
{
    ensEGvsampleDisplayNULL,
    ensEGvsampleDisplayReference,
    ensEGvsampleDisplayDefault,
    ensEGvsampleDisplayDisplayable,
    ensEGvsampleDisplayUndisplayable
} EnsEGvsampleDisplay;




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
** @attr Display [EnsEGvsampleDisplay] Display
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
    EnsEGvsampleDisplay Display;
    ajuint Size;
} EnsOGvsample;

#define EnsPGvsample EnsOGvsample*




/* #data EnsPGvindividualadaptor **********************************************
**
** Ensembl Genetic Variation Individual Adaptor.
** Defined as an alias in EnsPBaseadaptor
**
** #alias EnsPBaseadaptor
** ##
******************************************************************************/

#define EnsPGvindividualadaptor EnsPBaseadaptor




/* EnsEGvindividualGender *****************************************************
**
** Ensembl Genetic Variation Individual Gender enumeration.
**
******************************************************************************/

typedef enum EnsOGvindividualGender
{
    ensEGvindividualGenderNULL,
    ensEGvindividualGenderMale,
    ensEGvindividualGenderFemale,
    ensEGvindividualGenderUnknown
} EnsEGvindividualGender;




/* EnsEGvindividualType *******************************************************
**
** Ensembl Genetic Variation Individual Type enumeration.
**
******************************************************************************/

typedef enum EnsOGvindividualType
{
    ensEGvindividualTypeNULL,
    ensEGvindividualTypeFullyInbred,
    ensEGvindividualTypePartlyInbred,
    ensEGvindividualTypeOutbred,
    ensEGvindividualTypeMutant,
    ensEGvindividualTypeUnknown
} EnsEGvindividualType;




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
** @attr Gender [EnsEGvindividualGender] Gender
** @attr Type [EnsEGvindividualType] Type
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
    EnsEGvindividualGender Gender;
    EnsEGvindividualType Type;
    AjPStr Description;
} EnsOGvindividual;

#define EnsPGvindividual EnsOGvindividual*




/* #data EnsPGvpopulationadaptor **********************************************
**
** Ensembl Genetic Variation Population Adaptor.
** Defined as an alias in EnsPBaseadaptor
**
** #alias EnsPBaseadaptor
** ##
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




#endif /* ensgvdata_h */

#ifdef __cplusplus
}
#endif
