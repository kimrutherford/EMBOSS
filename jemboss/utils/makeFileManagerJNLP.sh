#!/bin/sh
#
# This program is free software; you can redistribute it and/or
# modify it under the terms of the GNU General Public License
# as published by the Free Software Foundation; either version 2
# of the License, or (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program; if not, write to the Free Software
# Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
#
#  @author: Copyright (C) Tim Carver
#
#
# Install EMBOSS & Jemboss
# last changed: 15/10/02
#
#

######################## Functions ########################

getJavaHomePath()
{
  JAVA_HOME_TMP=${JAVA_HOME_TMP-`which java 2>/dev/null`}
#  JAVA_HOME_TMP=`which java`

  if [ ! -f "$JAVA_HOME_TMP" ]; then
     if [ -d /usr/java/j2sdk1.4.1 ]; then
       JAVA_HOME_TMP=/usr/java/j2sdk1.4.1
     elif [ -d /usr/local/java/j2sdk1.4.1 ]; then
       JAVA_HOME_TMP=/usr/local/java/j2sdk1.4.1
     else
       JAVA_HOME_TMP=0
     fi
  else
    JAVA_HOME_TMP=`dirname $JAVA_HOME_TMP`
    JAVA_HOME_TMP=`dirname $JAVA_HOME_TMP`
  fi
}

echo
echo '*** Run this script from the installed jemboss utils directory.'
echo '*** If you are using SSL the script will use the client.keystore'
echo '*** in the $JEMBOSS/resources directory to create client.jar'
echo '*** which is wrapped with the FileManager client in FileManager.jar.'
echo '*** Press any key to continue.'
read KEY

cd ..
#CWPWD=$PWD
CWPWD=`pwd`
cd ..
CWPWD2=`pwd`
cd $CWPWD

while [ ! -d "$CWPWD/resources" ]
do
  echo
  echo "Enter the installed jemboss directory "
  echo "[/usr/local/emboss/share/EMBOSS/jemboss]:"
  read TMP
  if [ $TMP != "" ]; then
    CWPWD=$TMP
  fi
done

#
#

if [ ! -d "jnlp_fm" ]; then
  mkdir jnlp_fm
fi

getJavaHomePath
JAVA_HOME=$JAVA_HOME_TMP
while [ ! -f "$JAVA_HOME/bin/keytool" ]
do
  echo "Enter java (1.3 or above) location [/usr/java/jdk1.3.1/]: "
  read JAVA_HOME
  if [ "$JAVA_HOME" = "" ]; then
    JAVA_HOME="/usr/java/jdk1.3.1/"
  fi
done

PATH=$PATH:$JAVA_HOME/bin/ ; export PATH

cd $CWPWD

#
# Wrap client.keystore for JNLP 

if [ -f "resources/client.keystore" ]; then
  echo
  echo "Create client.jar to contain client.keystore."
  cd resources
  jar cf client.jar client.keystore
  cd ..
else
  echo
  echo "*** WARNING! If you are using an SSL Jemboss server then"
  echo "*** this will not work as the script has not found the"
  echo "*** client keystore file."
fi

#
# Create FileManager jar file

jar cf FileManager.jar images/* \
        resources/resources.jar \
        resources/version resources/jemboss.properties resources/readme.html \
        resources/*html 

if [ -f "resources/client.jar" ]; then
  jar uf FileManager.jar resources/client.jar
fi

mv FileManager.jar jnlp_fm
cp lib/*jar jnlp_fm
cp lib/axis/*jar jnlp_fm
cp images/Jemboss_logo_large.gif jnlp_fm
cp utils/template.html jnlp_fm/index.html
cd jnlp_fm
rm mail.jar activation.jar wsdl4j.jar servlet.jar log4j-1.2.8.jar axis-ant.jar

echo
echo
echo "The following information is used by keytool to"
echo "create a key store...."
echo
echo "What is your first and last name [Unknown]?"
read NAME
echo "What is the name of your organizational unit [Unknown]?"
read ORGU
echo "What is the name of your organization [Unknown]?"
read ORG
echo "What is the name of your City or Locality [Unknown]?"
read LOC
echo "What is the name of your State or Province [Unknown]?"
read STATE
echo "What is the two-letter country code for this unit [Unknown]?"
read CODE

echo "Give a key password (at least 6 characters):"
read KEYPASS
echo "Give a store password (at least 6 characters):"
read STOREPASS

#
# create a keystore file

keytool -genkey -alias signFiles -dname "CN=$NAME, \
        OU=$ORGU, O=$ORG, L=$LOC, S=$STATE, C=$CODE" \
        -keypass $KEYPASS -storepass $STOREPASS -keystore jembossstore 

#
# sign each of the jar files

echo
echo
echo "Each of the jar files will now be signed...."
echo
for i in *.jar; do 
  if [ $i != "jalview.jar" ]; then
    echo "Signing $i"
    jarsigner -keystore jembossstore -storepass $STOREPASS -keypass $KEYPASS \
              -signedjar s$i $i signFiles 
  fi
done;

#
# create a jnlp template file

JNLP="FileManager.jnlp"
if [ -f "$JNLP" ]; then
  echo "$JNLP exists. Enter a new JNLP file name: "
  read JNLP
fi

echo '<?xml version="1.0" encoding="UTF-8"?>'            > $JNLP
echo '<jnlp'                                            >> $JNLP
echo '        spec="1.0+"'                              >> $JNLP
echo '        codebase="http://EDIT"'                   >> $JNLP
echo '        href="'$JNLP'">'                          >> $JNLP 
echo '         <information>'                           >> $JNLP  
echo '           <title>Jemboss File manager</title>'   >> $JNLP  
echo '           <vendor>HGMP-RC</vendor> '             >> $JNLP  
echo '           <homepage href="http://www.uk.embnet.org/Software/EMBOSS/Jemboss/"/>' \
                                                        >> $JNLP  
echo '           <description>Jemboss</description>'    >> $JNLP  
echo '           <description kind="short">File Manager.' \
                                                        >> $JNLP  
echo '           </description>'                        >> $JNLP 
echo '           <icon href="Jemboss_logo_large.gif"/>' >> $JNLP 
echo '           <offline-allowed/>'                    >> $JNLP 
echo '         </information>'                          >> $JNLP 
echo '         <security>'                              >> $JNLP 
echo '           <all-permissions/>'                    >> $JNLP 
echo '         </security>'                             >> $JNLP 
echo '         <resources>'                             >> $JNLP 
echo '           <j2se version="1.3+"/>'                >> $JNLP 

echo '             <jar href="'sFileManager.jar'"/>'    >> $JNLP
for i in s*.jar; do
  if [ $i != "sFileManager.jar" ]; then
    if [ $i != "saaj.jar" ]; then
      if [ $i != "servlet.jar" ]; then
         echo '             <jar href="'$i'"/>'          >> $JNLP
      fi
    fi
  fi
done;

echo '         </resources>'                            >> $JNLP
echo '         <application-desc main-class="org.emboss.jemboss.FileManager"/>' \
                                                        >> $JNLP
echo '       </jnlp>'                                   >> $JNLP
 
#
#
 
echo
echo
echo "*** The signed jar files, index.html and $JNLP have been"
echo "*** created in the directory $CWPWD/jnlp_fm."
echo "*** "
echo "*** Please edit the 'codebase' line in $JNLP."
echo "*** Also, edit the 'Click here' line in index.html to point"
echo "*** href at $JNLP."
echo "*** The 'jnlp' directory will then need to be added to your HTTP"
echo "*** server configuration file or moved into the www data"
echo "*** directories."
echo "*** "
echo "*** For your http server to recognise the jnlp application, the"
echo "*** following line needs to be added to the mime.types file:"
echo "*** application/x-java-jnlp-file jnlp"
echo

