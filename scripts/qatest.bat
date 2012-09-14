@echo off
echo QA tests starting ...

rem set AXIS2C_HOME=c:\cygwin\home\emboss\membossdev\memboss-dev\win32build\axis2c
rem set EMBOSS_AXIS2C_HOME=c:\cygwin\home\emboss\membossdev\memboss-dev\win32buildaxis2c
perl c:\mEMBOSS\scripts\qatest.pl -without=srs -noembassy
echo 
echo QA tests completed

