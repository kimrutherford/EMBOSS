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
# last changed: 11/08/03
#
#

######################## Functions ########################


getJavaHomePath()
{
  JAVA_HOME_TMP=${JAVA_HOME_TMP-`which java 2>/dev/null`} 

  if [ ! -f "$JAVA_HOME_TMP" ]; then
     if [ -d /usr/java/j2sdk1.4.2 ]; then
       JAVA_HOME_TMP=/usr/java/j2sdk1.4.2
     elif [ -d /usr/local/java/j2sdk1.4.2 ]; then
       JAVA_HOME_TMP=/usr/local/java/j2sdk1.4.2
     else
       JAVA_HOME_TMP=0
     fi
  else
    JAVA_HOME_TMP=`dirname $JAVA_HOME_TMP`
    JAVA_HOME_TMP=`dirname $JAVA_HOME_TMP`
  fi
}


getClustalWPath()
{
  CLUSTALW=${CLUSTALW-`which clustalw 2>/dev/null`}

  echo
  echo "-------------------------- ClustalW --------------------------"
  echo
  echo "To use emma (EMBOSS interface to ClustalW) Jemboss needs to"
  echo "know the path to the clustalw binary."
  echo
  if (test "$CLUSTALW" != ""); then
    CLUSTALW=`dirname $CLUSTALW`
    echo "Enter the path to the clustalw or press return to use the"
    echo "default [$CLUSTALW]:"
    read CLUSTALW_TMP
    if (test "$CLUSTALW_TMP" != ""); then
      CLUSTALW="$CLUSTALW_TMP"
    fi
  else
    echo "Enter the path to clustalw or press return to set"
    echo "this later in jemboss.properties"
    read CLUSTALW
  fi
}


getPrimerPath()
{
  PRIMER3=${PRIMER3-`which primer3_core 2>/dev/null`}
                                                                                
  echo
  echo "-------------------------- Primer3  --------------------------"
  echo
  echo "To use eprimer3 (EMBOSS interface primer3 from the Whitehead"
  echo "Institute) Jemboss needs to know the path to the primer3_core"
  echo "binary."
  echo
  if (test "$PRIMER3" != ""); then
    PRIMER3=`dirname $PRIMER3`
    echo "Enter the path to the primer3_core or press return to use the"
    echo "default [$PRIMER3]:"
    read PRIMER3_TMP
    if (test "$PRIMER3_TMP" != ""); then
      PRIMER3="$PRIMER3_TMP"
    fi
  else
    echo "Enter the path to primer3_core or press return to set"
    echo "this later in jemboss.properties"
    read PRIMER3
  fi
}

setDataDirectory()
{

  JEMBOSS_CLASS=$1/org/emboss/jemboss/server/
  AUTH=$2
  DATADIR=$3

  OLDPATH="/tmp/SOAP/emboss"

  if [ $AUTH = "y" ]; then
    JEM_CLASS="$JEMBOSS_CLASS/JembossAuthServer.java"
    if test -f "$JEM_CLASS.orig" && (test ! -z "$JEM_CLASS.orig");then
      mv $JEM_CLASS.orig $JEM_CLASS
    fi
    mv $JEM_CLASS $JEM_CLASS.orig
    sed "s|$OLDPATH|$DATADIR|" $JEM_CLASS.orig > $JEM_CLASS
    
    JEM_CLASS="$JEMBOSS_CLASS/JembossFileAuthServer.java"
    if test -f "$JEM_CLASS.orig" && test ! -z "$JEM_CLASS.orig";then
      mv $JEM_CLASS.orig $JEM_CLASS
    fi
    mv $JEM_CLASS $JEM_CLASS.orig
    sed "s|$OLDPATH|$DATADIR|" $JEM_CLASS.orig > $JEM_CLASS
#   echo "sed 's|$OLDPATH|$DATADIR|' $JEM_CLASS.old > $JEM_CLASS"
  else
    JEM_CLASS="$JEMBOSS_CLASS/JembossServer.java"
    if test -f "$JEM_CLASS.orig" && test ! -z "$JEM_CLASS.orig";then
      mv $JEM_CLASS.orig $JEM_CLASS
    fi
    mv $JEM_CLASS $JEM_CLASS.orig
    sed "s|$OLDPATH|$DATADIR|" $JEM_CLASS.orig > $JEM_CLASS

    JEM_CLASS="$JEMBOSS_CLASS/JembossFileServer.java"
    if test -f "$JEM_CLASS.orig" && test ! -z "$JEM_CLASS.orig";then
      mv $JEM_CLASS.orig $JEM_CLASS
    fi
    mv $JEM_CLASS $JEM_CLASS.orig
    sed "s|$OLDPATH|$DATADIR|" $JEM_CLASS.orig > $JEM_CLASS
  fi

}


embassy_install()
{
  
  EMBOSS_DOWNLOAD=$1
  RECORD=$2
  PLATFORM=$3
  EMBOSS_INSTALL=$4
  USER_CONFIG=$5

  echo
  echo "--------------------------------------------------------------"
  echo
  echo "EMBASSY packages can optionally be installed along with"
  echo "the EMBOSS applications, see:"
  echo "http://emboss.sourceforge.net/apps/release/version/embassy"
  echo "where 'version' corresponds to the EMBOSS version e.g. 4.0"
  echo
  echo "--------------------------------------------------------------"
  echo
  echo "Install EMBASSY packages (y,n) [y]?"
  read EMBASSY

  if [ "$EMBASSY" = "" ]; then
    EMBASSY="y"
  fi

  echo "$EMBASSY" >> $RECORD
  
  if [ $EMBASSY = "y" ]; then

    if [ ! -d $EMBOSS_DOWNLOAD/embassy ]; then
      mkdir $EMBOSS_DOWNLOAD/embassy
    fi
    echo
    echo "To install EMBASSY Packages:"
    echo "(1) Download these from ftp://emboss.open-bio.org/pub/EMBOSS/"
    echo "(2) And unpack (gunzip and untar) them in: "
    echo "$EMBOSS_DOWNLOAD/embassy"
    echo "(3) *before* pressing return to continue!"
    read BLANK

    cd $EMBOSS_DOWNLOAD/embassy

    echo
    echo "Install all packages that are downloaded in"
    echo "$EMBOSS_DOWNLOAD/embassy "
    echo "(if no prompts will be given for each package to install)"
    echo "(y,n) [y]?"
    
    read ALL
    if [ "$ALL" = "" ]; then
      ALL="y"
    fi

    for dir in *
    do
      if [ $ALL = "y" ]; then
        INST="y"
      else
        echo "Install $dir (y,n) [y]?"
        read INST
        if [ "$INST" = "" ]; then
          INST="y"
        fi
      fi
      if [ $INST = "y" ]; then

        EMBASSY_INST="$EMBOSS_DOWNLOAD/embassy/$dir"
        if [ -f "$EMBASSY_INST/configure" ]; then
          echo
          echo "  ******** $dir will be configured and installed  ******** "
          echo
          cd $EMBASSY_INST
          ./configure --with-thread=$PLATFORM \
              --prefix=$EMBOSS_INSTALL $USER_CONFIG
          make
          make install
        else
          echo
          echo "Did not install $dir cannot find"
          echo "$EMBASSY_INST/configure"
          echo
        fi
      fi
    done

  fi

}

ssl_print_notes()
{

 KEYSTOREFILE=$1
 TOMCAT_ROOT=$2
 PORT=$3
 NUM=2

 echo
 if [ -f $JAVA_HOME/jre/lib/security/java.security ]; then

   SECURITY=`sed -n  -e 's|^security.provider.\([0-9]\)=com.sun.net.ssl.internal.ssl.Provider|security|p' $JAVA_HOME/jre/lib/security/java.security`


   if [ $SECURITY = "security" ]; then 
     NUM=1 
   else
     echo "1) EDIT "$JAVA_HOME/jre/lib/security/java.security
     echo "   adding/changing the provider line (usually provider 2 or 3):"
     echo "   security.provider.2=com.sun.net.ssl.internal.ssl.Provider"
   fi
 else
   if [ "$MACOSX" != "y" ]; then
     echo "1) EDIT the java.security file "
     echo "   adding/changing the provider line (usually provider 2 or 3):"
     echo "   security.provider.2=com.sun.net.ssl.internal.ssl.Provider"
   fi
 fi

 echo "$NUM) COPY & PASTE THE FOLLOWING INTO "
 echo "   $TOMCAT_ROOT/conf/server.xml"
 echo 
 echo

 if [ -d "$TOMCAT_ROOT/shared/classes" ]; then

   TCVERSION=`sed -n -e 's|\(.*\)Running The Tomcat 4\(.*\)|4|p' $TOMCAT_ROOT/RUNNING.txt`

   if [ "$TCVERSION" != "4" ]; then 
#tomcat 5.x

     TCVERSION=`sed -n -e 's|\(.*\)Running The Tomcat 5.5\(.*\)|5|p' $TOMCAT_ROOT/RUNNING.txt`
    
     if [ "$TCVERSION" != "5" ]; then
       echo
       echo '    <!-- Define a SSL Coyote HTTP/1.1 Connector on port '$PORT' -->'
       echo '    <Connector className="org.apache.coyote.tomcat5.CoyoteConnector"'
       echo '               port="'$PORT'" minProcessors="5" maxProcessors="75"'
       echo '               enableLookups="false"'
       echo '               acceptCount="10" debug="0" scheme="https" secure="true"'
       echo '               useURIValidationHack="false">'
       echo '      <Factory className="org.apache.coyote.tomcat5.CoyoteServerSocketFactory"'
       echo '           keystoreFile="'$KEYSTOREFILE'" keystorePass="'$PASSWD'"'
       echo '           clientAuth="false" protocol="TLS"/>'
       echo '    </Connector>'
       echo
     else
#tomcat 5.5
       echo
       echo '    <!-- Define a SSL Coyote HTTP/1.1 Connector on port '$PORT' -->'
       echo '    <Connector port="'$PORT'" minProcessors="5" maxProcessors="75"'
       echo '               enableLookups="false"'
       echo '               acceptCount="10" debug="0" scheme="https" secure="true"'
       echo '               useURIValidationHack="false"'
       echo '               keystoreFile="'$KEYSTOREFILE'" keystorePass="'$PASSWD'"'
       echo '               clientAuth="false" sslProtocol="TLS"/>'
       echo
     fi
   else
#tomcat 4.1.x
     echo
     echo '    <!-- Define a SSL Coyote HTTP/1.1 Connector on port '$PORT' -->'
     echo '    <Connector className="org.apache.coyote.tomcat4.CoyoteConnector"'
     echo '               port="'$PORT'" minProcessors="5" maxProcessors="75"'
     echo '               enableLookups="false"'
     echo '               acceptCount="10" debug="0" scheme="https" secure="true"'
     echo '               useURIValidationHack="false">'
     echo '      <Factory className="org.apache.coyote.tomcat4.CoyoteServerSocketFactory"'
     echo '           keystoreFile="'$KEYSTOREFILE'" keystorePass="'$PASSWD'"'
     echo '           clientAuth="false" protocol="TLS"/>'
     echo '    </Connector>'
     echo
   fi
 else
#tomcat 4.0.x
   echo '   <!-- Define an SSL HTTP/1.1 Connector on port '$PORT' -->'
   echo '   <Connector className="org.apache.catalina.connector.http.HttpConnector"'
   echo '           port="'$PORT'" minProcessors="5" maxProcessors="75"'
   echo '           enableLookups="true"'
   echo '           acceptCount="10" debug="0" scheme="https" secure="true">'
   echo '   <Factory className="org.apache.catalina.net.SSLServerSocketFactory"'
   echo '           keystoreFile="'$KEYSTOREFILE'" keystorePass="'$PASSWD'"'
   echo '           clientAuth="false" protocol="TLS"/>'
   echo '   </Connector>'  
   echo 
 fi

}

ssl_create_keystore()
{

  HOST=$1
  JEMBOSS_RES=$2
  KEYSTORE=$3
  ALIAS=$4
  PASSWD=$5
  VALID=$6

  keytool -genkey -alias $ALIAS -dname "CN=$HOST, \
      OU=Jemboss, O=HGMP-RC, L=CAMBRIDGE, S=CAMBRIDGE, C=UK" -keyalg RSA \
      -keypass $PASSWD -storepass $PASSWD -keystore $JEMBOSS_RES/$KEYSTORE.keystore -validity $VALID

  keytool -export -alias $ALIAS -storepass $PASSWD -file $JEMBOSS_RES/$KEYSTORE.cer \
      -keystore $JEMBOSS_RES/$KEYSTORE.keystore

}

ssl_import()
{
  FILE=$1
  KEYSTORE=$2
  PASSWD=$3

  keytool -import -v -trustcacerts -alias tomcat -file $FILE -keystore \
            $KEYSTORE -keypass $PASSWD -storepass $PASSWD -noprompt

}


make_jemboss_properties()
{

  EMBOSS_INSTALL=$1
  URL=$2
  AUTH=$3
  SSL=$4
  PORT=$5
  EMBOSS_URL=$6
  CLUSTALW=$7
  PRIMER3=$8

  EMBOSSPATH=/usr/bin/:/bin
  export EMBOSSPATH

  if (test "$CLUSTALW" != ""); then
    EMBOSSPATH=${EMBOSSPATH}:$CLUSTALW
  else
    EMBOSSPATH=${EMBOSSPATH}:/packages/clustal/
  fi

  if (test "$PRIMER3" != ""); then
    EMBOSSPATH=${EMBOSSPATH}:$PRIMER3
  else
    EMBOSSPATH=${EMBOSSPATH}:/packages/primer3/bin
  fi
 
  if [ $SSL = "y" ]; then
     URL=https://$URL:$PORT
  else
     URL=http://$URL:$PORT
  fi
  
  JEMBOSS_PROPERTIES=$EMBOSS_INSTALL/share/EMBOSS/jemboss/resources/jemboss.properties

  mv $JEMBOSS_PROPERTIES $JEMBOSS_PROPERTIES.orig
  touch $JEMBOSS_PROPERTIES

  if [ $AUTH = "y" ]; then
    echo "user.auth=true" > $JEMBOSS_PROPERTIES
  else
    echo "user.auth=false" > $JEMBOSS_PROPERTIES
  fi
  echo "jemboss.server=true" >> $JEMBOSS_PROPERTIES
  echo "server.public=$URL/axis/services" \
                                      >> $JEMBOSS_PROPERTIES

  echo "server.private=$URL/axis/services" \
                                       >> $JEMBOSS_PROPERTIES

  if [ $AUTH = "y" ]; then
   echo "service.public=JembossAuthServer" >> $JEMBOSS_PROPERTIES
   echo "service.private=JembossAuthServer" >> $JEMBOSS_PROPERTIES
  else
   echo "service.public=JembossServer" >> $JEMBOSS_PROPERTIES
   echo "service.private=JembossServer" >> $JEMBOSS_PROPERTIES
  fi

  echo "plplot=$EMBOSS_INSTALL/share/EMBOSS/" >> $JEMBOSS_PROPERTIES
  echo "embossData=$EMBOSS_INSTALL/share/EMBOSS/data/" >> $JEMBOSS_PROPERTIES
  echo "embossBin=$EMBOSS_INSTALL/bin/" >> $JEMBOSS_PROPERTIES
  echo "embossPath=$EMBOSSPATH" >> $JEMBOSS_PROPERTIES

# echo "embossPath=/usr/bin/:/bin:$CLUSTALW:$PRIMER3:/packages/clustal/:/packages/primer3/bin:" \
#                                                    >> $JEMBOSS_PROPERTIES
  echo "acdDirToParse=$EMBOSS_INSTALL/share/EMBOSS/acd/" >> $JEMBOSS_PROPERTIES
  echo "embossURL=$EMBOSS_URL" >> $JEMBOSS_PROPERTIES
  cp $JEMBOSS_PROPERTIES $JEMBOSS_PROPERTIES.bak

  echo
  echo "Changed $EMBOSS_INSTALL/share/EMBOSS/jemboss/resources/jemboss.properties"
  echo "to reflect this installation (original in jemboss.properties.orig)"
  echo

}


deploy_axis_services()
{

  JEMBOSS_LIB=$1
  AXIS=$1/axis
  CLASSPATH=$AXIS/axis.jar::$AXIS/jaxrpc.jar:$AXIS/saaj.jar:$AXIS/commons-logging.jar:
  CLASSPATH=${CLASSPATH}:$AXIS/commons-discovery.jar:$AXIS/wsdl4j.jar:$AXIS/servlet.jar
#  CLASSPATH=${CLASSPATH}:$JEMBOSS_LIB/jnet.jar:$JEMBOSS_LIB/jsse.jar:$JEMBOSS_LIB/jcert.jar

  PROXY_OFF="-DproxySet=false -DproxyHost= -DproxyPort= -Dhttp.proxyHost= -Dhttp.proxyPort= -Dhttps.proxyHost= -Dhttps.proxyPort= "
  SERVICE=$2
  URL=$3
  URL2=$4
  JAVAHOME=$5
  OPT_PROP1=$6
  OPT_PROP2=$7

  echo
# echo "$JAVAHOME/bin/java -classpath $CLASSPATH $OPT_PROP1 $OPT_PROP2 \\ "
# echo " org.apache.axis.client.AdminClient -l$URL/axis/services JembossServer.wsdd"
  echo

  $JAVAHOME/bin/java -classpath $CLASSPATH $OPT_PROP1 $OPT_PROP2 $PROXY_OFF \
        org.apache.axis.client.AdminClient \
        -l$URL/axis/services JembossServer.wsdd

  echo "#!/bin/csh " > deploy.csh
  echo "$JAVAHOME/bin/java -classpath $CLASSPATH $OPT_PROP1 $OPT_PROP2 $PROXY_OFF org.apache.axis.client.AdminClient -l$URL/axis/services JembossServer.wsdd" >> deploy.csh
  echo "" >> deploy.csh
  echo 'if ($status != 0) then' >> deploy.csh
  echo "  $JAVAHOME/bin/java -classpath $CLASSPATH $OPT_PROP1 $OPT_PROP2 $PROXY_OFF org.apache.axis.client.AdminClient -l$URL2/axis/services JembossServer.wsdd" >> deploy.csh
  echo "endif" >> deploy.csh
  chmod u+x deploy.csh
}

deploy_auth_services()
{

  JEMBOSS_LIB=$1
  CLASSPATH=$JEMBOSS_LIB/soap.jar:$JEMBOSS_LIB/activation.jar:$JEMBOSS_LIB/mail.jar
#  CLASSPATH=${CLASSPATH}:$JEMBOSS_LIB/jnet.jar:$JEMBOSS_LIB/jsse.jar:$JEMBOSS_LIB/jcert.jar

  SERVICE=$2
  URL=$3
  JAVAHOME=$4
  OPT_PROP1=$5
  OPT_PROP2=$6

  echo
  echo "Deploying $SERVICE "
  echo "$JAVAHOME/bin/java -classpath $CLASSPATH $OPT_PROP1 $OPT_PROP2 org.apache.soap.server.ServiceManagerClient  $URL/soap/servlet/rpcrouter deploy $SERVICE"
  echo

  $JAVAHOME/bin/java -classpath $CLASSPATH $OPT_PROP1 $OPT_PROP2 \
        org.apache.soap.server.ServiceManagerClient \
        $URL/soap/servlet/rpcrouter deploy $SERVICE

}


output_auth_xml()
{
  AUTH=$2
  if [ $AUTH = "y" ]; then
    JEM_CLASS="org.emboss.jemboss.server.JembossAuthServer"
    FIL_CLASS="org.emboss.jemboss.server.JembossFileAuthServer"
    ID="JembossAuthServer"
  else
    JEM_CLASS="org.emboss.jemboss.server.JembossServer"
    FIL_CLASS="org.emboss.jemboss.server.JembossFileServer"
    ID="JembossServer"
  fi

  XML_FILE=$1
  echo '<deployment xmlns="http://xml.apache.org/axis/wsdd/"' > $XML_FILE
  echo '            xmlns:java="http://xml.apache.org/axis/wsdd/providers/java">' >> $XML_FILE
  echo "  <service name=\"$ID\" provider=\"java:RPC\">" >> $XML_FILE
  echo "    <parameter name=\"className\" value=\"$JEM_CLASS\"/>" >> $XML_FILE
  echo '    <parameter name="allowedMethods" value="*"/>' >> $XML_FILE
  echo '  </service>' >> $XML_FILE
  echo '  <service name="EmbreoFile" provider="java:RPC">' >> $XML_FILE
  echo "    <parameter name=\"className\" value=\"$FIL_CLASS\"/>" >> $XML_FILE
  echo '    <parameter name="allowedMethods" value="*"/>' >> $XML_FILE
  echo '  </service>' >> $XML_FILE
  echo '</deployment>' >> $XML_FILE

}

#
# Find the likely location for the png & gd libs
get_libs()
{
  USER_CONFIG=$1

  include_lib_dirs="
/usr/local
/opt/freeware
/usr/freeware"

  if (test -f /usr/include/png.h) && (test -f /usr/include/gd.h); then
     USER_CONFIG="default"
  else
    for lib_dir in `echo  "$include_lib_dirs"` ;
    do
      if (test -f $lib_dir/include/png.h) && (test -f $lib_dir/include/gd.h); then
        USER_CONFIG="--with-pngdriver=$lib_dir"
      fi  
    done
  fi
}

check_libs()
{
  USER_CONFIG=$1
  PLATFORM=$2

  if [ "$USER_CONFIG" = "" ]; then
    DIR="/usr"
  else
    DIR=`echo $USER_CONFIG | sed -n -e 's|\(.*\)--with-pngdriver=\([^ ]*\)\(.*\)|\2|p'`
  fi

  lib_dirs="
$DIR/lib
$DIR/lib32
$DIR/lib64"

  echo
  echo "Inspecting $DIR"
 
# test for libpng
  WARN="true"
  if (test ! -f $DIR/include/png.h ); then
    WARN="true"
  else
    for lib_dir in `echo  "$lib_dirs"` ; 
    do
      echo "checking $lib_dir"
      if (test -f $lib_dir/libpng.a) || (test -f $lib_dir/libpng.so); then
         WARN="false"
         echo "...found libpng in $lib_dir"
         break
      elif (test -f $lib_dir/libpng.dylib); then
         WARN="false"
         echo "...found libpng in $lib_dir"
         break
      fi                     
    done
  fi

  if (test $WARN = "true"); then
    echo
    echo "------------------------- WARNING ----------------------------"
    echo
    echo "The script has detected that $DIR/include/png.h"
    echo "does not exist!"
    echo
    echo "Download libpng from"
    echo "      http://www.libpng.org/pub/png/libpng.html"
    echo "      http://libpng.sourceforge.net/"
    echo
    echo "For details see the EMBOSS admin guide:"
    echo "http://emboss.sourceforge.net/admin/"
    echo
    echo "To exit use Control C or press return to continue."
    echo
    echo "--------------------------------------------------------------"
    read REPLY
  fi

# test for gd
  WARN="true"
  if (test ! -f $DIR/include/gd.h ); then
    WARN="true"
  else
    for lib_dir in `echo  "$lib_dirs"` ;
    do
      echo "checking $lib_dir"
      if (test -f $lib_dir/libgd.a) || (test -f $lib_dir/libgd.so); then
         WARN="false"
         echo "...found gd in $lib_dir"
         break
      elif (test -f $lib_dir/libgd.dylib); then
         WARN="false"
         echo "...found libpng in $lib_dir"
         break
      fi
    done
  fi

  if (test $WARN = "true"); then
    echo
    echo "------------------------- WARNING ----------------------------"
    echo
    echo "The script has detected that $DIR/include/gd.h"
    echo "does not exist"
    echo
    echo "Download gd from"
    echo "       http://www.boutell.com/gd/"
    echo
    echo "For details see the EMBOSS admin guide:"
    echo "http://emboss.sourceforge.net/admin/"
    echo
    echo "To exit use Control C or press return to continue."
    echo
    echo "--------------------------------------------------------------"
    read REPLY
  fi 

# test for zlib which can be either in /usr/lib or $DIR/lib

  lib_dirs="
$DIR/lib
$DIR/lib32
$DIR/lib64
/usr/lib
/usr/local/lib"

  WARN="true"
  for lib_dir in `echo  "$lib_dirs"` ;
  do
    echo "checking $lib_dir"
    if (test -f $lib_dir/libz.a) || (test -f $lib_dir/libz.dylib); then
       WARN="false"
       echo "...found zlib in $lib_dir"
       break
    fi
  done

  if (test $WARN = "true"); then
    echo
    echo "------------------------- WARNING ----------------------------"
    echo
    echo "The script has detected that zlib is not installed in /usr"
    if( (test $PLATFORM = "macos") || (test $PLATFORM = "linux") ); then
      echo "or /usr/local"
    fi
    echo
    echo "Download zlib from"
    echo "       http://www.info-zip.org/pub/infozip/zlib/"
    echo
    echo "For details see the EMBOSS admin guide:"
    echo "http://emboss.sourceforge.net/admin/"
    echo
    echo "To exit use Control C or press return to continue."
    echo
    echo "--------------------------------------------------------------"
    read REPLY
  fi
}


clear
echo
echo "--------------------------------------------------------------"
echo "         EMBOSS and Jemboss Server installation script"
echo "--------------------------------------------------------------"
echo " "
echo "Note: any default values are given in square brackets []. "
echo " "
echo "There are two types of installation see details at: "
echo "http://emboss.sourceforge.net/Jemboss/install/setup.html"
echo " "
echo "(1) CLIENT-SERVER"
echo "(2) STANDALONE"
echo "Enter type of installation [1] :"
read INSTALL_TYPE

if (test "$INSTALL_TYPE" != "1") && (test "$INSTALL_TYPE" != "2"); then
  INSTALL_TYPE="1"
fi
clear

echo
echo "--------------------------------------------------------------"
echo "         EMBOSS and Jemboss Server installation script"
echo "--------------------------------------------------------------"
echo
echo "*** This script needs to be run with permissions to be able"
echo "*** to install EMBOSS in the required directories. This may"
echo "*** be best done as root or as a tomcat user."
echo
echo "Before running this script you should download the latest:"
echo
echo "(1) EMBOSS release (contains Jemboss) ftp://emboss.open-bio.org/pub/EMBOSS/"


if [ $INSTALL_TYPE = "1" ]; then
  echo "(2) Tomcat release http://jakarta.apache.org/site/binindex.html"
  echo "(3) Apache AXIS (SOAP) release   http://xml.apache.org/axis/"
fi
  
echo
echo "Has the above been downloaded (y/n)? "
read DOWNLOADED

if (test "$DOWNLOADED" != "y") && (test "$DOWNLOADED" != "Y"); then
  exit 1
fi


RECORD="install.record"
if [ -f "$RECORD" ]; then
  mv $RECORD $RECORD.old
fi

echo "$DOWNLOADED" > $RECORD
 
PLATTMP=`uname`

case $PLATTMP in
  Linux)
    PLATTMP="1"
    ;;
  AIX)
    PLATTMP="2"
    ;;
  IRIX)
    PLATTMP="3"
    ;;
  HP-UX)
    PLATTMP="4"
    ;;
  SunOS)
    PLATTMP="5"
    ;;
  Darwin)
    PLATTMP="6"
    ;;
  OSF1)
    PLATTMP="7"
    ;;
  FreeBSD)
    PLATTMP="8"
    ;;
  *)
    PLATTMP="1"
    ;;
esac


echo 
echo "Select the platform that your Jemboss server will be"
echo "run on from 1-8 [$PLATTMP]:"
echo "(1)  linux"
echo "(2)  aix"
echo "(3)  irix"
echo "(4)  hp-ux"
echo "(5)  solaris"
echo "(6)  macosX"
echo "(7)  OSF"
echo "(8)  FreeBSD"
read PLAT

if [ "$PLAT" = "" ]; then
  PLAT=$PLATTMP
fi

echo "$PLAT" >> $RECORD

AUTH_TYPE_TMP=1
AIX="n"
MACOSX="n"
if [ "$PLAT" = "1" ]; then
  PLATFORM="linux"
  AUTH_TYPE_TMP=3
elif [ "$PLAT" = "2" ]; then
  PLATFORM="aix"
  AIX="y"
  AUTH_TYPE_TMP=4
elif [ "$PLAT" = "3" ]; then
  PLATFORM="irix"
  AUTH_TYPE_TMP=1
elif [ "$PLAT" = "4" ]; then
  PLATFORM="hpux"
  AUTH_TYPE_TMP=5
elif [ "$PLAT" = "5" ]; then
  PLATFORM="solaris"
  AUTH_TYPE_TMP=6
elif [ "$PLAT" = "6" ]; then
  PLATFORM="macos"
  MACOSX="y"
  AUTH_TYPE_TMP=3
elif [ "$PLAT" = "7" ]; then
  PLATFORM="osf"
  AUTH_TYPE_TMP=7
elif [ "$PLAT" = "8" ]; then
  PLATFORM="freebsd"
  AUTH_TYPE_TMP=7
else
  echo "Platform not selected from 1-8."
  exit 1
fi

SSL="y"
if [ $INSTALL_TYPE = "1" ]; then
#
# localhost name
#
  echo
  echo "The IP address is needed by Jemboss to access"
  echo "the Tomcat web server."
  echo "Enter IP of server machine [localhost]:"
  read LOCALHOST

  if [ "$LOCALHOST" = "" ]; then
    LOCALHOST=localhost
  fi

  echo "$LOCALHOST" >> $RECORD
#
# SSL
#
  echo
  echo "Enter if you want the Jemboss server to use data"
  echo "encryption (https/SSL) (y,n) [y]?"
  read SSL

  echo "$SSL" >> $RECORD

  JSSE_HOME=""

  if (test "$SSL" = "y") || (test "$SSL" = ""); then
    PORT=8443
    SSL="y"
  else
    PORT=8080
  fi

#
# PORT
#
  USER_PORT=""
  echo
  echo "Enter port number [$PORT]:"
  read USER_PORT

  if [ "$USER_PORT" = "" ]; then
    if [ $SSL = "y" ]; then
      PORT=8443
    else
      PORT=8080
    fi
  else
    PORT=$USER_PORT
  fi

  echo "$PORT" >> $RECORD

fi
echo

#
# JAVA_HOME
#
getJavaHomePath
JAVA_HOME=$JAVA_HOME_TMP
if [ "$JAVA_HOME" != "0" ]; then
  echo "Enter java (1.4 or above) location [$JAVA_HOME_TMP]: "
  read JAVA_HOME

  if [ "$JAVA_HOME" = "" ]; then 
    JAVA_HOME=$JAVA_HOME_TMP
  fi
fi

while [ ! -f "$JAVA_HOME/bin/javac" ]
do
  echo "Enter java (1.3 or above) location (/usr/java/jdk1.3.1/): "
  read JAVA_HOME
done
echo
echo "$JAVA_HOME" >> $RECORD

#
# add java bin to path
#
PATH=$JAVA_HOME/bin/:$PATH ; export PATH

#
#
# JNI location for linux/solaris/AIX/SGI/HP-UX
#
#  
#
JAVA_INCLUDE=$JAVA_HOME/include/
JAVA_INCLUDE_OS=$JAVA_INCLUDE
if [ -d $JAVA_INCLUDE/linux ]; then
  JAVA_INCLUDE_OS=${JAVA_INCLUDE}/linux
elif [ -d $JAVA_INCLUDE/solaris ]; then
  JAVA_INCLUDE_OS=${JAVA_INCLUDE}/solaris
elif [ -d $JAVA_INCLUDE/irix ]; then
  JAVA_INCLUDE_OS=${JAVA_INCLUDE}/irix
elif [ -d $JAVA_INCLUDE/hp-ux ]; then
  JAVA_INCLUDE_OS=${JAVA_INCLUDE}/hp-ux
elif [ -d $JAVA_INCLUDE/alpha ]; then
  JAVA_INCLUDE_OS=${JAVA_INCLUDE}/alpha
elif [ -d $JAVA_INCLUDE/freebsd ]; then
  JAVA_INCLUDE_OS=${JAVA_INCLUDE}/freebsd
elif [ -d $JAVA_INCLUDE ]; then
  JAVA_INCLUDE_OS=${JAVA_INCLUDE}
else
  echo "Enter java include/Header directory location (containing jni.h)? "
  read JAVA_INCLUDE
  JAVA_INCLUDE_OS=$JAVA_INCLUDE
  if [ -d $JAVA_INCLUDE/linux ]; then
    JAVA_INCLUDE_OS=${JAVA_INCLUDE}/linux
  elif [ -d $JAVA_INCLUDE/solaris ]; then
    JAVA_INCLUDE_OS=${JAVA_INCLUDE}/solaris
  elif [ -d $JAVA_INCLUDE/irix ]; then
    JAVA_INCLUDE_OS=${JAVA_INCLUDE}/irix
  elif [ -d $JAVA_INCLUDE ]; then
    JAVA_INCLUDE_OS=${JAVA_INCLUDE}
  else
    echo "Problems finding java include libraries!"
    exit 1
  fi

  echo "$JAVA_INCLUDE" >> $RECORD
fi

#
# EMBOSS_DOWNLOAD
#
EMBOSS_DOWNLOAD_TMP=`pwd`
EMBOSS_DOWNLOAD_TMP=`dirname $EMBOSS_DOWNLOAD_TMP`
EMBOSS_DOWNLOAD_TMP=`dirname $EMBOSS_DOWNLOAD_TMP`

echo "Enter EMBOSS download directory"
echo "[$EMBOSS_DOWNLOAD_TMP]: "
read EMBOSS_DOWNLOAD

if [ "$EMBOSS_DOWNLOAD" = "" ]; then
  EMBOSS_DOWNLOAD=$EMBOSS_DOWNLOAD_TMP
fi

while [ ! -d "$EMBOSS_DOWNLOAD/ajax" ]
do
  echo "Enter EMBOSS download directory (e.g. /usr/emboss/EMBOSS-2.x.x): "
  read EMBOSS_DOWNLOAD
done
echo
echo "$EMBOSS_DOWNLOAD" >> $RECORD

echo "Enter where EMBOSS should be installed [/usr/local/emboss]: "
read EMBOSS_INSTALL

if [ "$EMBOSS_INSTALL" = "" ]; then
  EMBOSS_INSTALL=/usr/local/emboss
fi

if [ -d "$EMBOSS_INSTALL/share/EMBOSS/jemboss" ]; then
  echo 
  echo "Jemboss has already be installed to: "
  echo "$EMBOSS_INSTALL/share/EMBOSS/jemboss "
  echo "It is recommended this is removed before continuing."
  echo "To continue press return."
  read BLANK
fi
echo
echo "$EMBOSS_INSTALL" >> $RECORD

if [ $INSTALL_TYPE = "1" ]; then
  echo "Enter URL for emboss documentation for application "
  echo "[http://emboss.sourceforge.net/]:"
  read EMBOSS_URL

  echo "$EMBOSS_URL" >> $RECORD
fi

if [ "$EMBOSS_URL" = "" ]; then
  EMBOSS_URL="http://emboss.sourceforge.net/"
fi
echo

#
# set JSSE_HOME to the EMBOSS install dir
#
JSSE_HOME=$EMBOSS_INSTALL/share/EMBOSS/jemboss
JEMBOSS_SERVER_AUTH=""
AUTH=y

if [ $INSTALL_TYPE = "1" ]; then
  echo "Do you want Jemboss to use unix authorisation (y/n) [y]? "
  read AUTH

  echo "$AUTH" >> $RECORD
else
  AUTH="n"
fi

if [ "$AUTH" = "" ]; then
  AUTH="y"
fi
echo

if [ "$AUTH" = "y" ]; then

  echo "#include <stdio.h>" > dummy.c
  echo 'int main(){ printf("%d",getuid()); }' >> dummy.c
  if (cc dummy.c -o dummy >/dev/null 2>&1); then
    UUIDTMP=`./dummy`
    CC="cc"; 
  else
    gcc dummy.c -o dummy >/dev/null 2>&1
    UUIDTMP=`./dummy`
    CC="gcc";
  fi
  rm -f dummy.c dummy

  if (test "$UUIDTMP" = "") || (test "$UUIDTMP" = "0"); then
    UUIDTMP="506"
  fi

  echo "Provide the UID of the account (non-privileged) to run Tomcat,"
  echo "it has to be greater than 100 [$UUIDTMP]:"
  read UUID

  echo "$UUID" >> $RECORD

  if [ "$UUID" = "" ]; then
    UUID="$UUIDTMP"
  fi

  CC="$CC -DTOMCAT_UID=$UUID "; export CC

  echo
  echo "Unix Authentication Method, see:"
  echo "http://emboss.sourceforge.net/Jemboss/install/authentication.html"
  echo
  echo "(1) shadow      (3) PAM         (5) HP-UX shadow"
  echo "(2) no shadow   (4) AIX shadow  (6) Re-entrant shadow"
  echo "(7) Re-entrant no shadow"  
  echo 
  echo "Type of unix password method being used "
  echo "(select 1, 2, 3, 4, 5, 6 or 7 )[$AUTH_TYPE_TMP]"
  read AUTH_TYPE
  
  if [ "$AUTH_TYPE" = "" ]; then
     AUTH_TYPE="$AUTH_TYPE_TMP"
  fi

  echo "$AUTH_TYPE" >> $RECORD

  if [ "$AUTH_TYPE" = "1" ]; then
    JEMBOSS_SERVER_AUTH=" --with-auth=shadow"
  elif [ "$AUTH_TYPE" = "2" ]; then
    JEMBOSS_SERVER_AUTH=" --with-auth=noshadow"
  elif [ "$AUTH_TYPE" = "3" ]; then
    JEMBOSS_SERVER_AUTH=" --with-auth=pam"
  elif [ "$AUTH_TYPE" = "4" ]; then
    JEMBOSS_SERVER_AUTH=" --with-auth=aixshadow"
  elif [ "$AUTH_TYPE" = "5" ]; then
    JEMBOSS_SERVER_AUTH=" --with-auth=hpuxshadow"
  elif [ "$AUTH_TYPE" = "6" ]; then
    JEMBOSS_SERVER_AUTH=" --with-auth=rshadow"
  elif [ "$AUTH_TYPE" = "7" ]; then
    JEMBOSS_SERVER_AUTH=" --with-auth=rnoshadow"
  else
    JEMBOSS_SERVER_AUTH=" --with-auth=shadow"
  fi
fi
echo
 
if [ $INSTALL_TYPE = "1" ]; then
#
#
# SOAP data directory store
#

  echo "Define the directory you want to store the results in"
  echo "[/tmp/SOAP/emboss]"
  read DATADIR
  echo "$DATADIR" >> $RECORD

  if [ "$DATADIR" != "" ]; then
    setDataDirectory $EMBOSS_DOWNLOAD/jemboss $AUTH $DATADIR
  else
    setDataDirectory $EMBOSS_DOWNLOAD/jemboss $AUTH /tmp/SOAP/emboss
  fi
  echo

#
#
# Tomcat
#
  TOMCAT_ROOT=0

  while [ ! -d "$TOMCAT_ROOT/webapps" ]
  do
    echo "Enter Tomcat root directory (e.g. /usr/local/jakarta-tomcat-4.x.x)"
    read TOMCAT_ROOT
  done
  echo "$TOMCAT_ROOT" >> $RECORD
  echo

  if [ -d "$TOMCAT_ROOT/webapps/axis/WEB-INF/classes/org" ]; then
    echo
    echo "It looks like an installation has already been carried out in: "
    echo "$TOMCAT_ROOT/webapps/axis/WEB-INF/classes/ "
    echo "It is recommended that tomcat is removed and a fresh copy of tomcat used."
    echo "This is likely to fail if you continue."
    read BLANK
  fi

  if [ -d "$TOMCAT_ROOT/webapps/axis/WEB-INF/classes/resources" ]; then
    echo
    echo "It looks like an installation has already been carried out in: "
    echo "$TOMCAT_ROOT/webapps/axis/WEB-INF/classes/ "
    echo "It is recommended that tomcat is removed and a fresh copy of tomcat used."
    echo "This is likely to fail if you continue."
    read BLANK
  fi

#
# Apache AXIS (SOAP)
#
  SOAP_ROOT=0

  while [ ! -d "$SOAP_ROOT/webapps/axis" ]
  do
    echo "Enter Apache AXIS (SOAP) root directory (e.g. /usr/local/axis-xx)"
    read SOAP_ROOT
  done
  echo "$SOAP_ROOT" >> $RECORD
  echo
  cp -R $SOAP_ROOT/webapps/axis $TOMCAT_ROOT/webapps
  
  #already have commons-logging.jar in $TOMCAT_ROOT/server/lib/
  rm -f $TOMCAT_ROOT/webapps/axis/WEB-INF/lib/commons-logging.jar
fi

#
# Configuration options
#

USER_CONFIG=""
get_libs $USER_CONFIG

if [ "$USER_CONFIG" = "" ]; then
  echo
  echo "--------------------------------------------------------------"
  echo
  echo "The libraries for EMBOSS (libpng and gd) do not appear to"
  echo "be in /usr. It may be necessary to use the configuration"
  echo "flag --with-pngdriver to specify their location"
  echo
  echo "For details see the EMBOSS admin guide:"
  echo "http://emboss.sourceforge.net/admin/"
  echo
  echo "Enter any other EMBOSS configuration options (e.g. --with-pngdriver=pathname)"
  echo "or press return to leave blank:"
  read USER_CONFIG
elif [ "$USER_CONFIG" = "default" ]; then
  echo
  echo "--------------------------------------------------------------"
  echo
  echo "The libraries for EMBOSS (libpng and gd) appear to be in /usr,"
  echo "if these are the correct libraries then there should be no need"
  echo "to add any configuration options."
  echo
  echo "Enter any other EMBOSS configuration options (e.g. --with-pngdriver=pathname)"
  echo "or press return to leave blank:"
  read USER_CONFIG
else
  echo "Enter any other EMBOSS configuration options or press return to"
  echo "use default [$USER_CONFIG]:"
  read USER_CONFIG_TMP
  if [ "$USER_CONFIG_TMP" != "" ]; then
    USER_CONFIG="$USER_CONFIG_TMP"
  fi
fi

check_libs "$USER_CONFIG" $PLATFORM

echo "$USER_CONFIG" >> $RECORD
#
#
if [ "$AIX" = "y" ]; then
  if [ "$AUTH" = "y" ]; then
    CC="xlc_r -DTOMCAT_UID=$UUID "; export CC
  else
    CC=xlc_r; export CC
  fi
fi

echo
echo "  ******** EMBOSS will be configured with this information  ******** "
echo 
printf "%s\n" "./configure --with-java=$JAVA_INCLUDE \\"
printf "%s\n" "            --with-javaos=$JAVA_INCLUDE_OS \\"
printf "%s\n" "            --with-thread=$PLATFORM \\"
printf "%s\n" "            --prefix=$EMBOSS_INSTALL \\"
printf "%s\n" "           $JEMBOSS_SERVER_AUTH $USER_CONFIG"
echo

WORK_DIR=`pwd`
cd $EMBOSS_DOWNLOAD

./configure --with-java=$JAVA_INCLUDE \
            --with-javaos=$JAVA_INCLUDE_OS \
            --with-thread=$PLATFORM \
            --prefix=$EMBOSS_INSTALL $JEMBOSS_SERVER_AUTH $USER_CONFIG

make

echo
echo "  ******* EMBOSS with Jemboss will be installed in $EMBOSS_INSTALL ******* "
echo
sleep 2

make install

#
#
# Config EMBASSY
#
embassy_install $EMBOSS_DOWNLOAD $RECORD $PLATFORM $EMBOSS_INSTALL $USER_CONFIG


#
#
# Get clustalw and primer3_core path
#
getClustalWPath
getPrimerPath

#
#
#


#cd $EMBOSS_INSTALL/share/EMBOSS/jemboss
JEMBOSS=$EMBOSS_INSTALL/share/EMBOSS/jemboss

#---------------------------------------------------------------------------------
# Exit for standalone installations
#---------------------------------------------------------------------------------
if [ $INSTALL_TYPE = "2" ]; then
  echo
  echo "To run Jemboss:"
  echo "cd $JEMBOSS"
  echo "./runJemboss.sh"
  echo
  exit 0
fi


#
# create wossname.jar
#
PATH=$EMBOSS_INSTALL/bin:$PATH
export PATH
$EMBOSS_INSTALL/bin/wossname -colon -gui -outf wossname.out -auto
$JAVA_HOME/bin/jar cvf $JEMBOSS/resources/wossname.jar wossname.out

#
# create resources.jar archive of the scoring matrix
#
cd $EMBOSS_INSTALL/share/EMBOSS/data
$JAVA_HOME/bin/jar cvf $JEMBOSS/resources/resources.jar EPAM* EBLOSUM* ENUC*


if [ "$MACOSX" = "y" ]; then
  cd $EMBOSS_INSTALL/lib/
  ln -s libajax.dylib libajax.jnilib
# ln -s $EMBOSS_INSTALL/lib/libajax.dylib $EMBOSS_INSTALL/lib/libajax.jnilib
# cp $EMBOSS_INSTALL/lib/libajax.[0-9].dylib /System/Library/Frameworks/JavaVM.framework/Libraries/libajax.jnilib
fi

cd $WORK_DIR

if [ "$PLATFORM" = "hpux" ]; then
  ln -s $EMBOSS_INSTALL/lib/libajax.sl $EMBOSS_INSTALL/lib/libajax.so
fi



make_jemboss_properties $EMBOSS_INSTALL $LOCALHOST $AUTH $SSL $PORT $EMBOSS_URL $CLUSTALW $PRIMER3

#
#
# Tomcat scripts
#
#

rm -f tomstart

echo
echo "#!/bin/csh " > tomstart
echo "setenv JAVA_HOME $JAVA_HOME" >> tomstart

if [ "$SSL" = "y" ]; then
  echo "setenv JSSE_HOME $JSSE_HOME" >> tomstart
fi
 
if [ "$AIX" = "y" ]; then
 echo "setenv LIBPATH /usr/lib/threads:/usr/lib:/lib:$EMBOSS_INSTALL/lib" >> tomstart
 cp $EMBOSS_DOWNLOAD/ajax/.libs/libajax.so.0 $EMBOSS_INSTALL/lib
 ln -s $EMBOSS_INSTALL/lib/libajax.so.0 $EMBOSS_INSTALL/lib/libajax.so
else
 echo "setenv JAVA_OPTS \"-Djava.library.path=$EMBOSS_INSTALL/lib\"" >> tomstart
 echo "setenv LD_LIBRARY_PATH $EMBOSS_INSTALL/lib" >> tomstart
fi

if [ "$MACOSX" = "y" ]; then
 echo "setenv DYLD_LIBRARY_PATH $EMBOSS_INSTALL/lib" >> tomstart
fi

if [ "$PLATFORM" = "hpux" ]; then
 echo "setenv SHLIB_PATH $EMBOSS_INSTALL/lib" >> tomstart
fi


if [ "$AUTH_TYPE" = "3" ]; then
  if [ -f "/lib64/libpam.so" ]; then
    echo "setenv LD_PRELOAD /lib64/libpam.so" >> tomstart
  elif [ -f "/usr/lib64/libpam.so" ]; then
    echo "setenv LD_PRELOAD /usr/lib64/libpam.so" >> tomstart
  elif [ -f "/lib/libpam.so" ]; then
    echo "setenv LD_PRELOAD /lib/libpam.so" >> tomstart
  elif [ -f "/usr/lib/libpam.so" ]; then
    echo "setenv LD_PRELOAD /usr/lib/libpam.so" >> tomstart
  elif [ -f "/usr/lib/libpam.dylib" ]; then
    echo "setenv LD_PRELOAD /usr/lib/libpam.dylib" >> tomstart
  else
    echo
    echo "WARNING: don't know what to set LD_PRELOAD to"
    echo "edit LD_PRELOAD in tomstart script!"
    echo "setenv LD_PRELOAD /usr/lib/libpam.so" >> tomstart
    echo
  fi
fi

echo 'set path=($path '"$JAVA_HOME/bin)"  >> tomstart
echo "rehash"  >> tomstart
echo "$TOMCAT_ROOT/bin/startup.sh"  >> tomstart


rm -f tomstop

echo
echo "#!/bin/csh " > tomstop
echo "setenv JAVA_HOME $JAVA_HOME" >> tomstop
echo "setenv LD_LIBRARY_PATH $EMBOSS_INSTALL/lib" >> tomstop
#if [ "$AUTH_TYPE" = "3" ]; then
#  echo "setenv LD_PRELOAD /lib/libpam.so" >> tomstop
#fi
echo 'set path=($path '"$JAVA_HOME/bin)"  >> tomstop
echo "rehash"  >> tomstop
echo "$TOMCAT_ROOT/bin/shutdown.sh"  >> tomstop


chmod u+x tomstart
chmod u+x tomstop


#
#
# Run Jemboss script
#
#

RUNFILE=$JEMBOSS/runJemboss.sh

  sed "s|^java |$JAVA_HOME/bin/java |" $RUNFILE > $RUNFILE.new
  rm -f $RUNFILE
  mv $RUNFILE.new $RUNFILE
  

#
# Add classes to Tomcat path
#

  cp $JEMBOSS/lib/mail.jar $TOMCAT_ROOT/webapps/axis/WEB-INF/lib
  cp $JEMBOSS/lib/activation.jar $TOMCAT_ROOT/webapps/axis/WEB-INF/lib

  mv $JEMBOSS/org $TOMCAT_ROOT/webapps/axis/WEB-INF/classes/org
  cp -R $JEMBOSS/resources $TOMCAT_ROOT/webapps/axis/WEB-INF/classes/
  
  cp -R $EMBOSS_DOWNLOAD/jemboss/lib/axis $JEMBOSS/lib
  
# Ensure that the native library is not loaded more than once.
# To avoid place classes that load native libraries outside of the
# web app, and ensure that the loadLibrary() call is executed only once
# during the lifetime of a particular JVM.
  cd $JEMBOSS;
  mkdir tmp;
  cd tmp;
  jar -xvf ../lib/client.jar org/emboss/jemboss/parser/Ajax.class org/emboss/jemboss/parser/AjaxUtil.class
  jar -cvf ../lib/ajax.jar org
  jar -xvf ../lib/client.jar
  rm org/emboss/jemboss/parser/Ajax.class org/emboss/jemboss/parser/AjaxUtil.class
  jar -cvf ../lib/jemboss.jar .
  cd ..;
  rm -rf tmp;
  cp lib/jemboss.jar $TOMCAT_ROOT/webapps/axis/WEB-INF/lib/

  if [ -d "$TOMCAT_ROOT/shared/lib" ]; then
  #tomcat 4.1.x and 5.5.x
    cp lib/ajax.jar $TOMCAT_ROOT/shared/lib/
  elif [ -d "$TOMCAT_ROOT/lib" ]; then
  #tomcat 4.1.x and 5.5.x
    cp lib/ajax.jar $TOMCAT_ROOT/lib/
  else
    echo "WARNING: no $TOMCAT_ROOT/lib or $TOMCAT_ROOT/shared/lib found"
    echo "ajax.jar not added to Tomcat shared libraries folder"
  fi

  cd $WORK_DIR


#
#
# Create XML deployment descriptor files
#

output_auth_xml JembossServer.wsdd $AUTH

if [ "$SSL" != "y" ]; then

  echo
  echo "Tomcat XML deployment descriptors have been created for"
  echo "the Jemboss Server. Would you like an automatic deployment"
  echo "of the Jemboss web services to be tried (y/n) [y]?"
  read DEPLOYSERVICE

  if (test "$DEPLOYSERVICE" = "y") || (test "$DEPLOYSERVICE" = ""); then
    echo
    echo "Please wait, starting tomcat......."
    ./tomstart

    sleep 25
    deploy_axis_services $JEMBOSS/lib JembossServer.wsdd http://localhost:$PORT/ http://$LOCALHOST:$PORT/ $JAVA_HOME "" ""
  fi

else

  echo
  echo "--------------------------------------------------------------"
  echo
  echo "Client and server certificates need to be generated for the"
  echo "secure (https) connection. These are then imported into"
  echo "keystores. The keystores act as databases for security the"
  echo "certificates."
  echo  
  echo "For details see:"
  echo "http://emboss.sourceforge.net/Jemboss/install/ssl.html"
  echo
  echo "--------------------------------------------------------------"
  echo 
  PASSWD=""
  while [ "$PASSWD" = "" ]
  do
    echo "Provide a password (must be at least 6 characters): "
    read PASSWD
  done

  echo
  echo "Provide the validity period for these certificates, i.e. the"
  echo "number of days before they expire and new ones need to be made [90]:"
  read VALID
  echo

  if [ "$VALID" = "" ]; then
    VALID=90
  fi

  echo "Please wait, creating certificates......."
  ssl_create_keystore $LOCALHOST $JEMBOSS/resources "server" "tomcat-sv" $PASSWD $VALID
  ssl_create_keystore "Client" $JEMBOSS/resources "client" "tomcat-cl" $PASSWD $VALID

  ssl_import $JEMBOSS/resources/server.cer $JEMBOSS/resources/client.keystore $PASSWD
  ssl_import $JEMBOSS/resources/client.cer $JEMBOSS/resources/server.keystore $PASSWD

  cd $JEMBOSS/resources
  $JAVA_HOME/bin/jar cvf client.jar client.keystore
  cd $WORK_DIR

  echo
  echo "Tomcat XML deployment descriptors have been created for the Jemboss Server."
  echo "Would you like an automatic deployment of these to be tried (y/n) [y]?"
  read DEPLOYSERVICE

  if (test "$DEPLOYSERVICE" = "y") || (test "$DEPLOYSERVICE" = ""); then
    ssl_print_notes $JEMBOSS/resources/server.keystore $TOMCAT_ROOT $PORT
    echo

    VAL=0
    while [ $VAL != "y" ] 
    do
      echo "To continue you must have changed the above file(s)!"
      echo "Have the above files been edited (y/n)?"
      read VAL
    done

    echo
    echo "Please wait, starting tomcat......."
    ./tomstart

    sleep 45
    OPT_PROP1="-Djava.protocol.handler.pkgs=com.sun.net.ssl.internal.www.protocol"
    OPT_PROP2="-Djavax.net.ssl.trustStore=$JEMBOSS/resources/client.keystore"

    deploy_axis_services $JEMBOSS/lib JembossServer.wsdd https://localhost:$PORT/ https://$LOCALHOST:$PORT $JAVA_HOME $OPT_PROP1 $OPT_PROP2
  fi
fi

#
# Change jemboss.properties to reflect server location
# $EMBOSS_INSTALL/share/EMBOSS/jemboss/resources/jemboss.properties
#
#

echo
echo "--------------------------------------------------------------"
echo "--------------------------------------------------------------"

if [ "$AUTH" = "y" ]; then

  if [ $SSL = "y" ]; then
     URL=https://$LOCALHOST:$PORT
  else
     URL=http://$LOCALHOST:$PORT
  fi

  echo 
  echo "Tomcat should be running and the Jemboss web services deployed!"
  echo "(see $URL/axis/)"
  echo
  echo "It is *very* important to now:"
  echo "1. As root:"
  echo "   chown root $EMBOSS_INSTALL/bin/jembossctl"
  echo "   chmod u+s $EMBOSS_INSTALL/bin/jembossctl"
  echo "2. Ensure that tomcat is running as the non-privileged user,"
  echo "   with the same UID (i.e. $UUID) that was given to this script"
  echo "   (and NOT as root!)."
  echo "3. Use the tomstop & tomstart scripts in this directory"
  echo "   to stop & start tomcat."
  echo 
else
  echo
  echo "Note: Tomcat may still be running!"
  echo "Use the tomstop & tomstart scripts to stop & start tomcat."
  echo
fi

if [ ! -d "$DATADIR" ]; then
  echo "Create the user results directory (and ensure this is world read/write-able): "
  echo "   mkdir $DATADIR"
  echo
fi
echo "Try running Jemboss with the script:"
echo "   $JEMBOSS/runJemboss.sh"
echo
echo "To create a web launch page see:"
echo "http://emboss.sourceforge.net/Jemboss/install/deploy.html"
echo

chmod a+x $JEMBOSS/runJemboss.sh
chmod u+x $JEMBOSS/utils/*sh

exit 0;


