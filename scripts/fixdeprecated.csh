#!/bin/csh -f

foreach x (*.c)
  echo $x
  if(-e $x.save) then
    echo "already saved"
    cp $x.save $x.saveold
  endif
  fixdeprecated.pl $x:r
end
