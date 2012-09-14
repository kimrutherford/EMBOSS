#!/bin/csh

echo "# QA test commands" >! qatest.cmd
setenv EMBOSS_ACDCOMMANDLINELOG ../qatest.cmd

../../scripts/qatest.pl -without=srs -without=iprscan

unsetenv EMBOSS_ACDCOMMANDLINELOG
