#!/bin/csh

foreach embossinst (`embossversion -full -auto| grep '^InstallDirectory'`)
#  echo "$embossinst"
end

setenv EMBOSS_TIMETODAY "2007-07-15 12:00:00"
set top = ~/cvsemboss/test
set itop = $embossinst/share/EMBOSS/test

echo "$top"

cd $top

cd swiss
echo "Indexing swiss"
dbiflat -fields "acnum,seqvn,des,keyword,taxon" \
    -dbname SWISSPROT -auto
diff entrynam.idx $itop/swiss/

cd ../swnew
echo "Indexing swnew"
dbiflat -fields "acnum,seqvn,des,keyword,taxon" \
    -dbname SWISSNEW -auto
diff entrynam.idx $itop/swnew/

cd ../embl
echo "Indexing embl"
dbiflat -fields "acnum,seqvn,des,keyword,taxon" \
    -dbname EMBL -idformat embl -auto
diff entrynam.idx $itop/embl/

cd ../genbank
echo "Indexing genbank"
dbiflat -fields "acnum,seqvn,des,keyword,taxon" \
    -dbname GENBANK -idformat gb -filenames '*.seq' -auto
diff entrynam.idx $itop/genbank/

#cd ../gb
#echo "Indexing gb"
#dbigcg -fields "acnum,seqvn,des,keyword,taxon" \
#    -dbname GENBANK -idformat genbank -auto
#diff entrynam.idx $top/gb/

cd ../pir
echo "Indexing pir"
dbigcg -fields "acnum,seqvn,des,keyword,taxon" \
    -dbname PIR -idformat pir -auto
diff entrynam.idx $itop/pir/

cd ../testdb
echo "Indexing testdb"
dbifasta -fields "acnum,des" \
    -dbname TESTDB -idformat idacc -filenames 'testdb.fasta' -auto
diff entrynam.idx $itop/testdb/

#cd ../wormpep
#echo "Indexing wormpep"
#dbifasta -fields "acnum,des" \
#    -dbname WORMPEP -filenames 'wormpep' -auto
#diff entrynam.idx $top/wormpep/

