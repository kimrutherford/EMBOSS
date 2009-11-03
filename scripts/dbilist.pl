#!/usr/bin/perl -w

if (!defined($ARGV[0])) {
  print STDERR "Usage: dbilist.pl <index>\n";
  print STDERR "   where index is entrynam, acnum, seqvn, des, key, tax\n";
  exit ;
}
$index = $ARGV[0];
if ($index eq "entrynam") {$isindex=0}
else {$isindex=1}


print "$index\n";

if ($isindex) {

  open (HIT, "$index.hit") || die "Cannot open $index.hit";

  read (HIT, $hithead, 44, 0);

  ($hitfilesize, $hitrecordcnt, $hitrecordlen, $hitdbname, $hitrelease, $hitdate1, $hitdate2, $hitdate3, $hitdate4) = unpack ("V2vA20A10c4", $hithead);

  print "
hitfilesize $hitfilesize
hitrecordcnt $hitrecordcnt
hitrecordlen $hitrecordlen
htdbname '$hitdbname'
hitrelease '$hitrelease'
hitdate $hitdate1/$hitdate2/$hitdate3$hitdate4
";

  seek (HIT, 300, 0);

  open (TRG, "$index.trg") || die "Cannot open $index.trg";

  read (TRG, $trghead, 44, 0);

  ($trgfilesize, $trgrecordcnt, $trgrecordlen, $trgdbname, $trgrelease, $trgdate1, $trgdate2, $trgdate3, $trgdate4) = unpack ("V2vA20A10c4", $trghead);

  print "
trgfilesize $trgfilesize
trgrecordcnt $trgrecordcnt
trgrecordlen $trgrecordlen
htdbname '$trgdbname'
trgrelease '$trgrelease'
trgdate $trgdate1/$trgdate2/$trgdate3$trgdate4
";

  seek (TRG, 300, 0);

}

open (ENT, "entrynam.idx") || die "Cannot open entrynam.idx";

read (ENT, $enthead, 44, 0);

($entfilesize, $entrecordcnt, $entrecordlen) = unpack ("V2v", $enthead);

print "
entfilesize $entfilesize
entrecordcnt $entrecordcnt
entrecordlen $entrecordlen
";

seek (ENT, 300, 0);

$entidlen = $entrecordlen-10;

if (!$isindex) {
  $i=1;
  while ($i<=$entrecordcnt) {
    $entpos = 300-$entrecordlen+($i*$entrecordlen);
    seek (ENT, $entpos, 0);
    read (ENT, $entrecord, $entrecordlen, 0);
    ($entid,$entref, $entseq, $entfile) = unpack ("A$entidlen"."V2v", $entrecord);
    print "Entry $i pos $entpos file $entfile offsets $entref,$entseq '$entid'\n";
    $i++;
  }
}

else {
  $i=1;
  while ($i<=$trgrecordcnt) {

    seek (TRG, 300-$trgrecordlen+($i*$trgrecordlen), 0);
    read (TRG, $trgrecord, $trgrecordlen, 0);
    ($hitcnt,$hitnum,$token) = unpack ("V2A*", $trgrecord);
    print "\nTRG record $i count:$hitcnt start:$hitnum '$token'\n";

    seek (HIT, 300-$hitrecordlen+($hitnum*$hitrecordlen), 0);
    for ($j=1;$j<=$hitcnt;$j++) {
      read (HIT, $hitrecord, $hitrecordlen, 0);
      ($entrec) = unpack ("V", $hitrecord);

      seek (ENT, 300-$entrecordlen+($entrec*$entrecordlen), 0);
      read (ENT, $entrecord, $entrecordlen, 0);
      ($entid,$entref, $entseq, $entfile) = unpack ("A$entidlen"."V2v", $entrecord);

      $recnum = $hitnum+$j-1;
      print "Hit record $recnum: Entry $entrec file $entfile offsets $entref,$entseq '$entid'\n";
    }
    $i++;
  }

  close HIT;
  close TRG;
}

close ENT;
