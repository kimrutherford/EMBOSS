#!/bin/csh -f

set myapp = $argv[1]

set myapp = "/data/pmr/profemboss/emboss/$myapp"

# -b suppresses explanation of the profile table entries

set statfile = "gmon.out"
if (-e "gmon.sum") then
    set statfile = "gmon.sum"
endif

gprof -b $myapp $statfile
