#!/bin/csh

foreach embosshome (`embossversion -full -auto| grep '^BaseDirectory'`)
#  echo "$embosshome"
end

foreach embossinst (`embossversion -full -auto| grep '^InstallDirectory'`)
#  echo "$embossinst"
end

setenv EMBOSS_EDAM $embosshome/emboss/data/EDAM.obo

foreach x ( $embossinst/share/EMBOSS/acd/*.acd )
  set y = $x:t
  acdvalid $y:r |& $embosshome/scripts/dovalidclean.pl $y:r
end
