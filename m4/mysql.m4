dnl                                            -*- Autoconf -*-
##### http://autoconf-archive.cryp.to/ax_lib_mysql.html
#
# SYNOPSIS
#
#   AX_LIB_MYSQL([MINIMUM-VERSION])
#
# DESCRIPTION
#
#   This macro provides tests of availability of MySQL 'libmysqlclient'
#   library of particular version or newer.
#
#   AX_LIB_MYSQL macro takes only one argument which is optional.
#   If there is no required version passed, then macro does not run
#   version test.
#
#   The --with-mysql option takes one of three possible values:
#
#   no - do not check for MySQL client library
#
#   yes - do check for MySQL library in standard locations
#   (mysql_config should be in the PATH)
#
#   path - complete path to mysql_config utility, use this option if
#   mysql_config can't be found in the PATH
#
#   This macro calls:
#
#     AC_SUBST([MYSQL_CFLAGS])
#     AC_SUBST([MYSQL_CPPFLAGS])
#     AC_SUBST([MYSQL_LDFLAGS])
#     AC_SUBST([MYSQL_VERSION])
#
#   And sets:
#
#     HAVE_MYSQL
#
# LAST MODIFICATION
#
#   2006-07-16
#   2007-01-09 MKS: mysql_config --cflags may set gcc -fomit-frame-pointers,
#                   which prevents gdb from displaying stack traces.
#                   Changed mysql_config --cflags to mysql_config --include
#   2009-09-23 AJB: Checking for availability of both, include files and
#                   library files.
#   2010-06-14 MKS: Added MYSQL_CPPFLAGS
#   2011-08-01 MKS: Made test constructs more portable
#
# COPYLEFT
#
#   Copyright (c) 2006 Mateusz Loskot <mateusz@loskot.net>
#
#   Copying and distribution of this file, with or without
#   modification, are permitted in any medium without royalty provided
#   the copyright notice and this notice are preserved.

AC_DEFUN([AX_LIB_MYSQL],
[
  MYSQL_CFLAGS=""
  MYSQL_CPPFLAGS=""
  MYSQL_LDFLAGS=""
  MYSQL_CONFIG=""
  MYSQL_VERSION=""

  AC_ARG_WITH([mysql],
  [AS_HELP_STRING([--with-mysql@<:@=ARG@:>@],
  [use MySQL client library @<:@default=yes@:>@, optionally specify path to mysql_config])],
  [
    AS_IF([test "x${withval}" = "xno"],
    [want_mysql="no"],
    [test "x${withval}" = "xyes"],
    [want_mysql="yes"],
    [
      want_mysql="yes"
      MYSQL_CONFIG="${withval}"
    ])
  ],
  [want_mysql="yes"])

  dnl
  dnl Check MySQL libraries (libmysqlclient)
  dnl

  AS_IF([test "x${want_mysql}" = "xyes"],
  [
    AS_IF([test -z "${MYSQL_CONFIG}" -o test],
    [AC_PATH_PROG([MYSQL_CONFIG], [mysql_config], [no])])

    AS_IF([test "x${MYSQL_CONFIG}" != "xno"],
    [
      AC_MSG_CHECKING([for MySQL libraries])

      MYSQL_CFLAGS="`${MYSQL_CONFIG} --cflags`"
      MYSQL_CPPFLAGS="`${MYSQL_CONFIG} --include`"
      MYSQL_LDFLAGS="`${MYSQL_CONFIG} --libs`"

      MYSQL_VERSION=`${MYSQL_CONFIG} --version`

      dnl It isn't enough to just test for mysql_config as Fedora
      dnl provides it in the mysql RPM even though mysql-devel may
      dnl not be installed

      EMBCPPFLAGS="${CPPFLAGS}"
      EMBLDFLAGS="${LDFLAGS}"

      CPPFLAGS="${MYSQL_CPPFLAGS} ${EMBCPPFLAGS}"
      LDFLAGS="${MYSQL_LDFLAGS} ${EMBLDFLAGS}"

      AC_LINK_IFELSE([AC_LANG_PROGRAM([[#include <stdio.h>
                                        #include "mysql.h"]],
                                      [[mysql_info(NULL)]])],
        [havemysql="yes"],
        [havemysql="no"])

      CPPFLAGS="${EMBCPPFLAGS}"
      LDFLAGS="${EMBLDFLAGS}"

      AS_IF([test "x${havemysql}" = "xyes"],
      [
        AC_DEFINE([HAVE_MYSQL], [1],
        [Define to 1 if MySQL libraries are available.])
        found_mysql="yes"
        AC_MSG_RESULT([yes])
      ],
      [
        MYSQL_CFLAGS=""
        MYSQL_CPPFLAGS=""
        MYSQL_LDFLAGS=""
        found_mysql="no"
        AC_MSG_RESULT([no])
      ])
    ],
    [
      found_mysql="no"
    ])
  ])

  dnl
  dnl Check if required version of MySQL is available
  dnl

  mysql_version_req=ifelse([$1], [], [], [$1])

  AS_IF([test "x${found_mysql}" = "xyes" -a -n "${mysql_version_req}"],
  [
    AC_MSG_CHECKING([if MySQL version is >= ${mysql_version_req}])

    dnl Decompose required version string of MySQL
    dnl and calculate its number representation

    mysql_version_req_major=`expr ${mysql_version_req} : '\([[0-9]]*\)'`
    mysql_version_req_minor=`expr ${mysql_version_req} : '[[0-9]]*\.\([[0-9]]*\)'`
    mysql_version_req_micro=`expr ${mysql_version_req} : '[[0-9]]*\.[[0-9]]*\.\([[0-9]]*\)'`

    AS_IF([test "x${mysql_version_req_micro}" = "x"],
    [mysql_version_req_micro="0"])

    mysql_version_req_number=`expr ${mysql_version_req_major} \* 1000000 \
                             \+ ${mysql_version_req_minor} \* 1000 \
                             \+ ${mysql_version_req_micro}`

    dnl Decompose version string of installed MySQL
    dnl and calculate its number representation

    mysql_version_major=`expr ${MYSQL_VERSION} : '\([[0-9]]*\)'`
    mysql_version_minor=`expr ${MYSQL_VERSION} : '[[0-9]]*\.\([[0-9]]*\)'`
    mysql_version_micro=`expr ${MYSQL_VERSION} : '[[0-9]]*\.[[0-9]]*\.\([[0-9]]*\)'`

    AS_IF([test "x${mysql_version_micro}" = "x"],
    [mysql_version_micro="0"])

    mysql_version_number=`expr ${mysql_version_major} \* 1000000 \
                         \+ ${mysql_version_minor} \* 1000 \
                         \+ ${mysql_version_micro}`

    mysql_version_check=`expr ${mysql_version_number} \>\= ${mysql_version_req_number}`

    AS_IF([test "x${mysql_version_check}" = "x1"],
    [AC_MSG_RESULT([yes])],
    [AC_MSG_RESULT([no])])
  ])

  AC_SUBST([MYSQL_CFLAGS])
  AC_SUBST([MYSQL_CPPFLAGS])
  AC_SUBST([MYSQL_LDFLAGS])
  AC_SUBST([MYSQL_VERSION])
])
