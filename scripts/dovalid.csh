#!/bin/csh

foreach x ( ~/local/share/EMBOSS/acd/*.acd )
  set y = $x:t
  acdvalid $y:r |& ~/cvsemboss/scripts/dovalidclean.pl $y:r
end
