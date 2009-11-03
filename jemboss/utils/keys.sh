#!/bin/sh

# Tim carver 
# keys.sh
#
# Creates RSA keystores
#
# Usage: keys.sh directory_to_put_keys hostname key_password
#
# 13 March 02



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
  JEMBOSS_RES=$3

  echo "**********IMPORTING"
  keytool -import -v -trustcacerts -alias tomcat -file $JEMBOSS_RES/$FILE -keystore \
            $JEMBOSS_RES/$KEYSTORE -keypass $PASSWD -storepass $PASSWD -noprompt

}

echo
echo "Enter where to store the keys and certificates:"
read JEMBOSS_RES

echo
echo "Enter your surname:"
read HOST

echo
echo "Enter a password to use to create the keys with"
echo "(at least 6 characters):"
read PASSWD

echo
echo "Provide the validity period for these certificates, i.e. the"
echo "number of days before they expire and new ones need to be made [90]:"
read VALID
echo

if [ "$VALID" = "" ]; then
  VALID=90
fi

#
# create the keystores & export the certificates
#
ssl_create_keystore "server" $JEMBOSS_RES "server" "tomcat-sv" $PASSWD $VALID
ssl_create_keystore "Client" $JEMBOSS_RES "client" "tomcat-cl" $PASSWD $VALID

#
# import certificates into keystores - so server trusts client...
#
ssl_import server.cer client.keystore $JEMBOSS_RES $PASSWD
ssl_import client.cer server.keystore $JEMBOSS_RES $PASSWD

exit 0;


