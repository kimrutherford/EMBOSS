#ifdef __cplusplus
extern "C"
{
#endif

#ifndef embdata_h
#define embdata_h



/*
** Prototype definitions
*/

void     embDataListDel(AjPList *data);
AjPTable embDataListGetTable(const AjPList fullList, ajuint required);
void     embDataListGetTables(const AjPList fullList, AjPList returnList,
			      ajuint required);
void     embDataListRead(AjPList data, AjPFile file);

/*
** End of prototype definitions
*/

#endif

#ifdef __cplusplus
}
#endif
