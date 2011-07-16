#!/bin/csh -f

cd ~/devemboss

cd test/qa

# Create the example files by running qatest and saving all the results

 ../../scripts/qatest.pl -kk >&! qatest.out

# Run wossname to list all programs.

../../scripts/autodoc.pl 
