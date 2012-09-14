dnl                                            -*- Autoconf -*-
##### http://autoconf-archive.cryp.to/ax_lib_postgresql.html
#
# SYNOPSIS
#
#   AX_LIB_POSTGRESQL([MINIMUM-VERSION])
#
# DESCRIPTION
#
#   This macro provides tests of availability of PostgreSQL 'libpq'
#   library of particular version or newer.
#
#   AX_LIB_POSTGRESQL macro takes only one argument which is optional.
#   If there is no required version passed, then macro does not run
#   version test.
#
#   The --with-postgresql option takes one of three possible values:
#
#   no - do not check for PostgreSQL client library
#
#   yes - do check for PostgreSQL library in standard locations
#   (pg_config should be in the PATH)
#
#   path - complete path to pg_config utility, use this option if
#   pg_config can't be found in the PATH
#
#   This macro calls:
#
#     AC_SUBST([POSTGRESQL_CFLAGS])
#     AC_SUBST([POSTGRESQL_CPPFLAGS])
#     AC_SUBST([POSTGRESQL_LDFLAGS])
#     AC_SUBST([POSTGRESQL_VERSION])
#
#   And sets:
#
#     HAVE_POSTGRESQL
#
# LAST MODIFICATION
#
#   2006-07-16
#   2010-05-14 MKS: Added POSTGRESQL_CPPFLAGS
#   2011-06-21 AJB: Added workaround for Fedora pg_config oddity
#   2011-08-01 MKS: Changed PG_CONFIG to POSTGRESQL_CONFIG
#                   Made test constructs more portable
#
# COPYLEFT
#
#   Copyright (c) 2006 Mateusz Loskot <mateusz@loskot.net>
#
#   Copying and distribution of this file, with or without
#   modification, are permitted in any medium without royalty provided
#   the copyright notice and this notice are preserved.

AC_DEFUN([AX_LIB_POSTGRESQL],
[
  POSTGRESQL_CFLAGS=""
  POSTGRESQL_CPPFLAGS=""
  POSTGRESQL_LDFLAGS=""
  POSTGRESQL_CONFIG=""
  POSTGRESQL_VERSION=""

  AC_ARG_WITH([postgresql],
  [AS_HELP_STRING([--with-postgresql@<:=@ARG@:>@],
  [use PostgreSQL library @<:@default=yes@:>@, optionally specify path to pg_config])],
  [
    AS_IF([test "x${withval}" = "xno"],
    [want_postgresql="no"],
    [test "x${withval}" = "xyes"],
    [want_postgresql="yes"],
    [
      want_postgresql="yes"
      POSTGRESQL_CONFIG="${withval}"
    ])
  ],
  [want_postgresql="yes"])

  dnl
  dnl Check PostgreSQL libraries (libpq)
  dnl

  AS_IF([test "x${want_postgresql}" = "xyes"],
  [
    AS_IF([test -z "${POSTGRESQL_CONFIG}" -o test],
    [AC_PATH_PROG([POSTGRESQL_CONFIG], [pg_config], [no])])

    AS_IF([test "x${POSTGRESQL_CONFIG}" != "xno"],
    [
      AC_MSG_CHECKING([for PostgreSQL libraries])

      POSTGRESQL_CFLAGS="-I`${POSTGRESQL_CONFIG} --includedir`"
      POSTGRESQL_CPPFLAGS="-I`${POSTGRESQL_CONFIG} --includedir`"
      POSTGRESQL_LDFLAGS="-L`${POSTGRESQL_CONFIG} --libdir` -lpq"

      POSTGRESQL_VERSION=`${POSTGRESQL_CONFIG} --version | sed -e 's#PostgreSQL ##'`

      dnl It isn't enough to just test for pg_config as Fedora
      dnl provides it in the postgresql RPM even though postgresql-devel may
      dnl not be installed

      EMBCPPFLAGS="${CPPFLAGS}"
      EMBLDFLAGS="${LDFLAGS}"

      CPPFLAGS="${POSTGRESQL_CPPFLAGS} ${EMBCPPFLAGS}"
      LDFLAGS="${POSTGRESQL_LDFLAGS} ${EMBLDFLAGS}"

      AC_LINK_IFELSE([AC_LANG_PROGRAM([[#include <stdio.h>
                                        #include "libpq-fe.h"]],
                                      [[PQconnectdb(NULL)]])],
        [havepostgresql="yes"],
        [havepostgresql="no"])

      CPPFLAGS="${EMBCPPFLAGS}"
      LDFLAGS="${EMBLDFLAGS}"

      AS_IF([test "x${havepostgresql}" = "xyes"],
      [
        AC_DEFINE([HAVE_POSTGRESQL], [1],
        [Define to 1 if PostgreSQL libraries are available.])
        found_postgresql="yes"
        AC_MSG_RESULT([yes])
      ],
      [
        POSTGRESQL_CFLAGS=""
        POSTGRESQL_CPPFLAGS=""
        POSTGRESQL_LDFLAGS=""
        found_postgresql="no"
        AC_MSG_RESULT([no])
      ])
    ],
    [
      found_postgresql="no"
    ])
  ])

  dnl
  dnl Check if required version of PostgreSQL is available
  dnl

  postgresql_version_req=ifelse([$1], [], [], [$1])

  AS_IF([test "x${found_postgresql}" = "xyes" -a -n "${postgresql_version_req}"],
  [
    AC_MSG_CHECKING([if PostgreSQL version is >= ${postgresql_version_req}])

    dnl Decompose required version string of PostgreSQL
    dnl and calculate its number representation

    postgresql_version_req_major=`expr ${postgresql_version_req} : '\([[0-9]]*\)'`
    postgresql_version_req_minor=`expr ${postgresql_version_req} : '[[0-9]]*\.\([[0-9]]*\)'`
    postgresql_version_req_micro=`expr ${postgresql_version_req} : '[[0-9]]*\.[[0-9]]*\.\([[0-9]]*\)'`

    AS_IF([test "x${postgresql_version_req_micro}" = "x"],
    [postgresql_version_req_micro="0"])

    postgresql_version_req_number=`expr ${postgresql_version_req_major} \* 1000000 \
                                  \+ ${postgresql_version_req_minor} \* 1000 \
                                  \+ ${postgresql_version_req_micro}`

    dnl Decompose version string of installed PostgreSQL
    dnl and calculate its number representation

    postgresql_version_major=`expr ${POSTGRESQL_VERSION} : '\([[0-9]]*\)'`
    postgresql_version_minor=`expr ${POSTGRESQL_VERSION} : '[[0-9]]*\.\([[0-9]]*\)'`
    postgresql_version_micro=`expr ${POSTGRESQL_VERSION} : '[[0-9]]*\.[[0-9]]*\.\([[0-9]]*\)'`

    AS_IF([test "x${postgresql_version_micro}" = "x"],
      [postgresql_version_micro="0"])

    postgresql_version_number=`expr ${postgresql_version_major} \* 1000000 \
                              \+ ${postgresql_version_minor} \* 1000 \
                              \+ ${postgresql_version_micro}`

    postgresql_version_check=`expr ${postgresql_version_number} \>\= ${postgresql_version_req_number}`

    AS_IF([test "x${postgresql_version_check}" = "x1"],
    [AC_MSG_RESULT([yes])],
    [AC_MSG_RESULT([no])])
  ])

  AC_SUBST([POSTGRESQL_CFLAGS])
  AC_SUBST([POSTGRESQL_CPPFLAGS])
  AC_SUBST([POSTGRESQL_LDFLAGS])
  AC_SUBST([POSTGRESQL_VERSION])
])
