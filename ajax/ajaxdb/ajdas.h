#ifdef __cplusplus
extern "C"
{
#endif

#ifndef ajdas_h
#define ajdas_h




/* @data AjPDasCoordinate *****************************************************
**
** Description of the namespace/coordinate of a DAS source
**
** @attr uri [AjPStr] the unique URI for a DAS source. For a DAS registration
**                    server it should allow to access more information
**                    about the coordinate/namespace
** @attr taxid [AjPStr] NCBI taxonomy id of the organism, as a number
** @attr source [AjPStr] type of the sources for the coordinate system,
**                      such as chromosome, scaffold, supercontig
** @attr authority [AjPStr] name of the organization that determined
**                          the coordinate system
** @attr test_range [AjPStr] example id with start/stop positions
** @attr version [AjPStr] version of the build upon which
**                        the coordinate system is based
** @@
******************************************************************************/

typedef struct AjSDasCoordinate
{
    AjPStr uri;
    AjPStr taxid;
    AjPStr source;
    AjPStr authority;
    AjPStr test_range;
    AjPStr version;
} AjODasCoordinate;

#define AjPDasCoordinate AjODasCoordinate*




/* @data AjPDasSource *********************************************************
**
** DAS Source
**
** @attr uri [AjPStr] a unique URI for the DAS source
** @attr title [AjPStr] nickname
** @attr description [AjPStr] description
** @attr entry_points_uri [AjPStr] URI for the DAS 'entry_points' command
** @attr sequence_query_uri [AjPStr] URI for the DAS 'sequence' command
** @attr features_query_uri [AjPStr] URI for the DAS 'features' command
** @attr coordinates [AjPList] list of supported coordinates
** @attr sequence [AjBool] whether 'sequence' queries are supported
** @attr stylesheet [AjBool] whether 'stylesheet' queries are supported
** @attr features [AjBool] whether 'features' queries are supported
** @attr entry_points [AjBool] whether 'entry_points' queries are supported
**
** @@
******************************************************************************/

typedef struct AjSDasSource
{
    AjPStr uri;
    AjPStr title;
    AjPStr description;
    AjPStr entry_points_uri;
    AjPStr sequence_query_uri;
    AjPStr features_query_uri;
    AjPList coordinates;
    AjBool sequence;
    AjBool stylesheet;
    AjBool features;
    AjBool entry_points;
} AjODasSource;

#define AjPDasSource AjODasSource*




/* @data AjPDasSegment ********************************************************
**
** DAS Segment
**
** segment elements are returned by DAS 'entry_points' query
**
** @attr id [AjPStr] unique identifier
** @attr type [AjPStr] type of the segment, such as "chromosome" or "scaffold"
** @attr start [ajuint] start position of the segment
** @attr stop [ajuint] stop position of the segment
** @attr orientation [AjPStr] orientation is one of "+" or "-"
**                            and indicates the strandedness of the segment
**                            ("+" if the segment is not intrinsically ordered)
** @attr subparts [AjBool] "yes" indicates that the segment has subparts
** @attr padding [char[4]] Padding to alignment boundary
** @@
******************************************************************************/

typedef struct AjSDasSegment
{
    AjPStr id;
    AjPStr type;
    ajuint start;
    ajuint stop;
    AjPStr orientation;
    AjBool subparts;
    char   padding[4];
} AjODasSegment;

#define AjPDasSegment AjODasSegment*





/* @data AjDASserver **********************************************************
**
** DAS server object
**
** @attr host [AjPStr] host-name
** @attr path [AjPStr] path to DAS service on host
** @attr port [ajuint] URL port
** @attr padding [char[4]] Padding to alignment boundary
** @attr sources [AjPList] list of available DAS sources on the server
** @@
******************************************************************************/

typedef struct AjSDasServer
{
    AjPStr host;
    AjPStr path;
    ajuint port;
    char padding[4];
    AjPList sources;
} AjODasServer;

#define AjPDasServer AjODasServer*




/*
** Prototype definitions
*/


AjPStr       ajDasGetSequenceQueryURI(const AjPQuery qry,
                                      const AjPStr sourceURI);

void         ajDasPrintCachefile(const AjPDasServer server, AjPFile cachef);


void         ajDasSegmentDel(AjPDasSegment* segment);
void         ajDasServerDel(AjPDasServer *thys);
void         ajDasServerGetSources(AjPDasServer server, const AjPStr cmd);
AjPDasServer ajDasServerNew(void);
AjBool       ajDasServerSethostS(AjPDasServer server, const AjPStr host);
AjBool       ajDasServerSetpathS(AjPDasServer server, const AjPStr path);
AjBool       ajDasServerSetport(AjPDasServer server, ajuint port);

AjPStr       ajDasSourceGetDBname(const AjPDasSource source,
				  AjBool titleAndURI);

AjPStr       ajDasTestrangeParse(const AjPStr testrange, AjPStr* id,
                                 ajint* ibegin, ajint* iend);

/* parsing DAS XML outputs */

AjBool       ajDasParseRegistry(AjPFilebuff buff, AjPList sources);
AjBool       ajDasParseEntrypoints(AjPFilebuff buff, AjPList segments);


/*
** End of prototype definitions
*/

#endif

#ifdef __cplusplus
}
#endif
