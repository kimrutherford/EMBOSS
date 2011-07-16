#ifdef __cplusplus
extern "C"
{
#endif

#ifndef embgroup_h
#define embgroup_h




/* @data EmbPGroupProg ********************************************************
**
** Hold details of programs (names and documentation) and the package
** they belong to.
**
** @alias EmbOGroupProg
** @alias EmbSGroupProg
**
** @attr name [AjPStr] Name of group or of program
** @attr doc [AjPStr] Documentation for this program
** @attr keywords [AjPStr] keywords for this program
** @attr package [AjPStr] EMBASSY package, empty for main package
** @attr groups [AjPList] List of group(s) this program belongs to.
** @attr acdtopics [AjPList] List of relation topic attributes
** @attr acdoperations [AjPList] List of relation operation attributes
** @attr acdinputs [AjPList] List of relation input attributes
** @attr acdoutputs [AjPList] List of relation output attributes
** @attr acdparams [AjPList] List of relation parameter attributes
** @@
******************************************************************************/

typedef struct EmbSGroupProg {
    AjPStr name;
    AjPStr doc;
    AjPStr keywords;
    AjPStr package;
    AjPList groups;
    AjPList acdtopics;
    AjPList acdoperations;
    AjPList acdinputs;
    AjPList acdoutputs;
    AjPList acdparams;
} EmbOGroupProg;
#define EmbPGroupProg EmbOGroupProg*




/* @data EmbPGroupRelation ****************************************************
**
** Hold details of relation attributes and the qualifiers they appear in.
**
** @alias EmbOGroupRelation
** @alias EmbSGroupRelation
**
** @attr type [AjPStr] Type of qualifier
** @attr qual [AjPStr] Name of qualifier
** @attr acdgroup [AjPStr] ACD group for qualifier type
** @attr id [AjPStr] EDAM term id
** @attr namespace [AjPStr] EDAM namespace
** @attr name [AjPStr] EDAM term name
** @@
******************************************************************************/

typedef struct EmbSGroupRelation {
    AjPStr type;
    AjPStr qual;
    AjPStr acdgroup;
    AjPStr id;
    AjPStr namespace;
    AjPStr name;
} EmbOGroupRelation;
#define EmbPGroupRelation EmbOGroupRelation*




/* @data EmbPGroupTop *********************************************************
**
** This serves as both a node in a list of names of groups which each hold
** a list of details of programs (names and documentation) and also
** it is a node in a list of the details of programs (names and documentation).
**
** Using the same structure for both is a bit confusing, but it simplifies
** some of the routines which search and output the lists of gnodes of
** groups and program data.
**
** GROUP LIST 	nodes point to	PROGRAM LISTS
** ----------			-------------
**
** group gnode 	-> program gnode - program gnode - program gnode - etc.
**     |
** group gnode 	-> program gnode - program gnode - program gnode - etc.
**     |
**    etc.
**
**
** The layout of the 'alpha' list of alphabetic listing of applications is
** a bit different - instead of applications being grouped, as in 'glist'
** above, the applications all come under one major group and each
** application holds a list of the groups it belongs to:
**
** ALPHA LIST
** ----------
**
** group gnode -> program gnode -> group gnode - group gnode - etc.
** 		   |
** 	       program gnode -> group gnode - group gnode - group gnode - etc.
** 		   |
** 	       program gnode -> group gnode - group gnode - group gnode - etc.
** 		   |
** 	       program gnode -> group gnode - group gnode - group gnode - etc.
** 		   |
** 	       program gnode -> group gnode - group gnode - group gnode - etc.
**                    |
**                   etc.
**
**
** @alias EmbOGroup
** @alias EmbSGroup
**
** @attr name [AjPStr] name of group or of program
** @attr doc [AjPStr] documentation for this program (used by list of programs)
** @attr progs [AjPList] list of programs in this group (used by groups list)
** @@
******************************************************************************/

typedef struct EmbSGroupTop {
    AjPStr name;
    AjPStr doc;
    AjPList progs;
} EmbOGroupTop;
#define EmbPGroupTop EmbOGroupTop*




/*
** Prototype definitions
*/

ajint         embGrpCompareTwoGnodes(const void * a, const void * b);
ajint         embGrpCompareTwoPnodes(const void * a, const void * b);
void          embGrpExit(void);
AjBool        embGrpGetEmbassy(const AjPStr appname, AjPStr* embassyname);
void          embGrpGetProgGroups(AjPList glist, AjPList alpha,
				  char * const env[],
				  AjBool emboss, AjBool embassy,
				  const AjPStr embassyname,
				  AjBool explode, AjBool colon,
				  AjBool gui);
void          embGrpGroupsListDel(AjPList *groupslist);
void          embGrpProgsListDel(AjPList *progslist);
void          embGrpRelationsListDel(AjPList *relslist);
void          embGrpProgDel(EmbPGroupProg *Pgl);
void          embGrpKeySearchProgs(AjPList newlist, const AjPList glist,
				   const AjPStr key, AjBool all);
void          embGrpSearchProgsEdam(AjPList newlist, const AjPList glist,
                                    const AjPStr key, const char* namespace,
                                    AjBool sensitive, AjBool subclasses,
                                    AjBool obsolete);
void          embGrpKeySearchSeeAlso(AjPList newlist,
				     AjPList *appgroups, AjPStr* package,
				     const AjPList alpha, const AjPList glist,
				     const AjPStr key);
EmbPGroupTop  embGrpMakeNewGnode(const AjPStr name);
EmbPGroupProg embGrpMakeNewPnode(const AjPStr name, const AjPStr doc,
				 const AjPStr keywords, const AjPStr package);
void          embGrpOutputGroupsList(AjPFile outfile,
				     const AjPList groupslist,
				     AjBool showprogs, AjBool html,
				     AjBool showkey,
				     const AjPStr package);
void          embGrpOutputProgsList(AjPFile outfile,  const AjPList progslist,
				    AjBool html, AjBool showkey,
				    const AjPStr package);
void          embGrpSortGroupsList(AjPList groupslist);
void          embGrpSortProgsList(AjPList progslist);
void          embGrpGroupMakeUnique(AjPList list);
void          embGrpProgsMakeUnique(AjPList list);

/*
** End of prototype definitions
*/

#endif

#ifdef __cplusplus
}
#endif
