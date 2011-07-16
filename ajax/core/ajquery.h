#ifdef __cplusplus
extern "C"
{
#endif

#ifndef ajquery_h
#define ajquery_h

/*
** Prototype definitions
*/

AjPQuery      ajQueryNew(AjEDataType datatype);
void          ajQueryDel(AjPQuery* pthis);
void          ajQueryTrace(const AjPQuery thys);
void          ajQueryClear(AjPQuery thys);
const AjPStr  ajQuerystrParseFormat(AjPStr *Pqry, AjPTextin textin,
                                    AjBool findformat(const AjPStr format,
                                                      ajint *iformat));
AjBool        ajQuerystrParseListfile(AjPStr *Pqry);
AjBool        ajQuerystrParseRange(AjPStr *Pqry,
                                   ajint *Pbegin, ajint *Pend, AjBool *Prev);
AjBool        ajQuerystrParseRead(AjPStr *Pqry,
                                  AjPTextin textin,
                                  AjBool findformat(const AjPStr format,
                                                    ajint *iformat),
                                  AjBool *Pnontext);
void          ajQueryStarclear(AjPQuery thys);
AjBool        ajQueryKnownFieldC(const AjPQuery qry, const char* fieldtxt);
AjBool        ajQueryKnownFieldS(const AjPQuery qry, const AjPStr field);
AjBool        ajQueryAddFieldAndC(AjPQuery qry, const char* fieldtxt,
                                  const char* wildquerytxt);
AjBool        ajQueryAddFieldAndS(AjPQuery qry, const AjPStr field,
                                  const AjPStr wildquery);
AjBool        ajQueryAddFieldElseC(AjPQuery qry, const char* fieldtxt,
                                   const char* wildquerytxt);
AjBool        ajQueryAddFieldElseS(AjPQuery qry, const AjPStr field,
                                   const AjPStr wildquery);
AjBool        ajQueryAddFieldEorC(AjPQuery qry, const char* fieldtxt,
                                  const char* wildquerytxt);
AjBool        ajQueryAddFieldEorS(AjPQuery qry, const AjPStr field,
                                  const AjPStr wildquery);
AjBool        ajQueryAddFieldNotC(AjPQuery qry, const char* fieldtxt,
                                  const char* wildquerytxt);
AjBool        ajQueryAddFieldNotS(AjPQuery qry, const AjPStr field,
                                  const AjPStr wildquery);
AjBool        ajQueryAddFieldOrC(AjPQuery qry, const char* fieldtxt,
                                 const char* wildquerytxt);
AjBool        ajQueryAddFieldOrS(AjPQuery qry, const AjPStr field,
                                 const AjPStr wildquery);
const AjPList ajQueryGetallFields(const AjPQuery query);
const char*   ajQueryGetDatatype(const AjPQuery query);
const AjPStr  ajQueryGetFormat(const AjPQuery query);
const AjPStr  ajQueryGetId(const AjPQuery query);
AjBool        ajQueryGetQuery(const AjPQuery query, AjPStr* Pdest);
AjBool        ajQueryIsSet(const AjPQuery thys);
AjBool        ajQuerySetWild(AjPQuery thys);

AjPQueryField ajQueryfieldNewC(const char* fieldtxt,
                               const char* wildtxt,
                               AjEQryLink oper);
AjPQueryField ajQueryfieldNewS(const AjPStr field,
                               const AjPStr wild,
                               AjEQryLink oper);
void          ajQueryfieldDel(AjPQueryField *Pthis);
void          ajQuerylistTrace(const AjPList list);

void          ajQueryExit(void);

/*
** End of prototype definitions
*/

#endif

#ifdef __cplusplus
}
#endif
