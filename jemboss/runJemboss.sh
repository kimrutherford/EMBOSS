#!/bin/sh 
#
# 
#
JEMBOSS_HOME=.
export JEMBOSS_HOME
LIB=$JEMBOSS_HOME/lib
export LIB

CLASSPATH=${JEMBOSS_HOME}:$LIB/JembossPrintAlignment.jar
CLASSPATH=${CLASSPATH}:$LIB/jakarta-regexp-1.2.jar
CLASSPATH=${CLASSPATH}:$LIB/jalview.jar
CLASSPATH=${CLASSPATH}:$LIB/mail.jar
CLASSPATH=${CLASSPATH}:$LIB/activation.jar
CLASSPATH=${CLASSPATH}:$LIB/axis/axis.jar
CLASSPATH=${CLASSPATH}:$LIB/axis/jaxrpc.jar
CLASSPATH=${CLASSPATH}:$LIB/axis/saaj.jar
CLASSPATH=${CLASSPATH}:$LIB/axis/commons-logging.jar
CLASSPATH=${CLASSPATH}:$LIB/axis/commons-discovery.jar
CLASSPATH=${CLASSPATH}:$LIB/axis/wsdl4j.jar
CLASSPATH=${CLASSPATH}:$LIB/grout.jar
CLASSPATH=${CLASSPATH}:$LIB/client.jar
export CLASSPATH

#
#For csh: 
#setenv CLASSPATH $LIB/JembossPrintAlignment.jar:$LIB/jakarta-regexp-1.2.jar
#setenv CLASSPATH ${CLASSPATH}:$LIB/jnet.jar:$LIB/jcert.jar:$LIB/jsse.jar
#setenv CLASSPATH ${CLASSPATH}:$LIB/axis/axis.jar:$LIB/axis/jaxrpc.jar:$LIB/axis/saaj.jar
#setenv CLASSPATH ${CLASSPATH}:$LIB/axis/commons-logging.jar:$LIB/axis/commons-discovery.jar
#setenv CLASSPATH ${CLASSPATH}:$LIB/grout.jar:$LIB/client.jar:$LIB/jalview.jar:.
#

#
#Add local to run as 'standalone':
#java org.emboss.jemboss.Jemboss local &
#
java org.emboss.jemboss.Jemboss &
