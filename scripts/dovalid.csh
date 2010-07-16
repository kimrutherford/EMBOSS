#!/bin/csh

setenv EMBOSS_EDAM /homes/pmr/obo/EDAM_beta07.obo

foreach x ( ~/local/share/EMBOSS/acd/*.acd )
  set y = $x:t
  acdvalid $y:r |& ~/devemboss/scripts/dovalidclean.pl $y:r
end
