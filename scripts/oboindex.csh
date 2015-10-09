#!/bin/csh -fv

set db = $argv[1]

cd ~/devemboss

if($db == "edam") then
  dbxedam -dir emboss/data -index emboss/index -file EDAM.obo -auto
else if($db == "chebi") then
  dbxobo -dir emboss/data/OBO -index emboss/index \
    -dbname $db -dbres chebiresource -file $db.obo -auto
else if($db == "swo") then
 dbxobo -dir emboss/data/OBO -index emboss/index \
    -dbname $db -dbres sworesource -file software.obo -auto
else if($db == "eco") then
 dbxobo -dir emboss/data/OBO -index emboss/index \
    -dbname $db -dbres oboresource -file eco.obo -auto
else if($db == "go") then
 dbxobo -dir emboss/data/OBO -index emboss/index \
    -dbname $db -dbres oboresource -file gene_ontology.1_2.obo -auto
else if($db == "pw") then
 dbxobo -dir emboss/data/OBO -index emboss/index \
    -dbname $db -dbres oboresource -file pathway.obo -auto
else
  dbxobo -dir emboss/data/OBO -index emboss/index \
    -dbname $db -dbres oboresource -file $db.obo -auto
endif
