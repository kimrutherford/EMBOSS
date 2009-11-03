#!/bin/csh -f

foreach x (*.acd)
  echo $x:r
   if(! -e $x:r.save) then
      echo "saved in $x:r.save"
     cp $x $x:r.save
   endif
   acdpretty $x:r
   mv $x:r.acdpretty $x
end
