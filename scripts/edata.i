#~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
#    $RCSfile: edata.i,v $
#    $Revision: 1.2 $
#    $Date: 2002/08/29 10:05:41 $
#    $Author: rice $
#
#~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

# emboss data object indexing
# based on SRS cfunc.i

$EDATA_DB=$Library:[EDATA group:$MISC_LIBS 
  format:$EDATA_FORMAT maxNameLen:80 iFile:edata
  files:{
    $LibFile:edata
  }
  links:{
    $Link:[$EDATA_DB to:$EDATA_DB  token:ref_data toField:$DF_ID
      fromName:EDATA_UP toName:EDATA_DOWN]
  }
]

$EDATA_FORMAT=$LibFormat:[fileType:$EDATA_FILE syntax:$EDATA_SYNTAX 
  printFormat:table3 fields:{
    $Field:[$DF_ALL]
    $Field:[$DF_ID code:id index:id indexToken:id]
    $Field:[$DF_Alias code:alias index:str indexToken:alias]
    $Field:[$DF_Description code:des index:str indexToken:des ]
    $Field:[$DF_Constructor code:new index:str indexToken:new ]
    $Field:[$DF_Destructor code:del index:str indexToken:del ]
    $Field:[$DF_Assignment code:assign index:str indexToken:assign]
    $Field:[$DF_Modifier code:mod index:str indexToken:modify]
    $Field:[$DF_Operator code:use index:str indexToken:use]
    $Field:[$DF_Cast code:cast index:str indexToken:cast]
    $Field:[$DF_Use code:use index:str indexToken:use]
    $Field:[$DF_Other code:other index:str indexToken:other]
    $Field:[$DF_Attribute code:attr index:str indexToken:attr]
    $Field:[$DF_Body code:body index:str indexToken:btext]
  }
]

$EDATA_SYNTAX=$Syntax:[file:"SRSSITE:edata.is" ignore:" \t"]
$EDATA_FILE=$FileType:[typeName:dat maxline:500]
       
$DF_Constructor=$SrsField:[Constructor short:new]
$DF_Destructor=$SrsField:[Destructor short:del]
$DF_Assignment=$SrsField:[Assignment short:ass]
$DF_Modifier=$SrsField:[Modifier short:mod]
$DF_Operator=$SrsField:[Operator short:set]
$DF_Cast=$SrsField:[Cast short:cas]
$DF_Use=$SrsField:[Example short:use]
$DF_Alias=$SrsField:[Alias short:ali]
$DF_Attribute=$SrsField:[Attr short:att]







