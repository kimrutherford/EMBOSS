dnl                                            -*- Autoconf -*-
dnl @synopsis CHECK_JAVA()
dnl
dnl Need to specify --with-java and --with-javaos
dnl @author Alan Bleasby
dnl
dnl This macro calls:
dnl
dnl   AC_SUBST([JAVA_CFLAGS])
dnl   AC_SUBST([JAVA_CPPFLAGS])
dnl   AC_SUBST([JAVA_LDFLAGS])
dnl
dnl   AM_CONDITIONAL([JAVA_BUILD], ...)
dnl
dnl And sets:
dnl
dnl   AC_DEFINE([HAVE_JAVA], ...)
dnl
dnl   AC_PATH_PROG([ANT], ...)
dnl   AC_PATH_PROG([JAR], ...)
dnl   AC_PATH_PROG([JAVA], ...)
dnl   AC_PATH_PROG([JAVAC], ...)

AC_DEFUN([CHECK_JAVA],
[
  JAVA_CFLAGS=""
  JAVA_CPPFLAGS=""
  JAVA_LDFLAGS=""

  have_java="yes"
  auth_java=""

  AC_MSG_CHECKING([for Java JNI])

  AC_ARG_WITH([java],
  [AS_HELP_STRING([--with-java@<:@=ARG@:>@],
  [root directory path of Java installation])],
  [
    AC_MSG_RESULT([${withval}])
    AS_IF([test "x${withval}" = "xno"], [have_java="no"])
  ],
  [
    AC_MSG_RESULT([no])
    have_java="no"
  ])

  AS_IF([test "x${have_java}" = "xyes"],
  [
    # If specified, the Java JNI include directory has to exist.
    AS_IF([test -d ${with_java}],
    [AS_VAR_SET([JAVA_CPPFLAGS], ["-I${withval}"])],
    [
      have_java="no"
      AC_MSG_ERROR([Java include directory ${withval} does not exist])
    ])
  ])

  AC_MSG_CHECKING([for Java JNI OS])

  AC_ARG_WITH([javaos],
  [AS_HELP_STRING([--with-javaos@<:@=ARG@:>@],
  [root directory path of Java OS include])],
  [
    AC_MSG_RESULT([${withval}])

    AS_IF([test "x${withval}" != "xno"],
    [
      # If specified, the Java JNI OS include directory has to exist.
      AS_IF([test "x${have_java}" = "xyes" && test -d ${withval}],
      [AS_VAR_APPEND([JAVA_CPPFLAGS], [" -I${withval}"])],
      [
        have_java="no"
        AC_MSG_ERROR([Java OS include directory ${withval} does not exist])
      ])
    ])
  ],
  [
    AC_MSG_RESULT([no])
  ])

  # Authorisation type

  AC_MSG_CHECKING([for authorisation type])

  AC_ARG_WITH([auth],
  [AS_HELP_STRING([--with-auth@<:@=ARG@:>@],
  [authorisation mechanism for Jemboss server @<:@default=PAM@:>@])],
  [
    AS_IF([test "x${withval}" != "xno"],
    [
      AC_MSG_RESULT([yes])

      AS_CASE([${withval}],
      [yes],
      [
        auth_java="PAM"
        AC_CHECK_LIB([pam], [main],
        [AS_VAR_APPEND([JAVA_LDFLAGS], [" -lpam"])])
      ],
      [pam],
      [
        auth_java="PAM"
        AC_CHECK_LIB([pam], [main],
        [AS_VAR_APPEND([JAVA_LDFLAGS], [" -lpam"])])
      ],
      [shadow],
      [
        auth_java="N_SHADOW"
        AC_CHECK_LIB([crypy], [main],
        [AS_VAR_APPEND([JAVA_LDFLAGS], [" -lcrypt"])])
      ],
      [rshadow],
      [
        auth_java="R_SHADOW"
        AC_CHECK_LIB([crypy], [main],
        [AS_VAR_APPEND([JAVA_LDFLAGS], [" -lcrypt"])])
      ],
      [noshadow],
      [auth_java="NO_SHADOW"],
      [rnoshadow],
      [auth_java="RNO_SHADOW"],
      [aixshadow],
      [auth_java="AIX_SHADOW"],
      [hpuxshadow],
      [auth_java="HPUX_SHADOW"])
    ],
    [AC_MSG_RESULT([no])])
  ],
  [AC_MSG_RESULT([no])])

  AS_IF([test -n "${auth_java}"],
  [AS_VAR_APPEND([JAVA_CPPFLAGS], [" -D${auth_java}"])],
  [AS_VAR_APPEND([JAVA_CPPFLAGS], [" -DNO_AUTH"])])

  # Threading type

  AC_MSG_CHECKING([for threading type])

  AC_ARG_WITH([thread],
  [AS_HELP_STRING([--with-thread@<:@=ARG@:>@],
  [thread type @<:@default=linux@:>@])],
  [
    AS_IF([test "x${withval}" != "xno"],
    [
      AC_MSG_RESULT([yes])

      AS_CASE([${withval}],
      [yes],
      [
        AS_VAR_APPEND([JAVA_CPPFLAGS], [" -D_REENTRANT"])
        AS_VAR_APPEND([JAVA_LDFLAGS], [" -lpthread"])
        # AS_VAR_APPEND([LIBS], [" -lpthread"])
      ],
      [freebsd],
      [
        AS_VAR_APPEND([JAVA_CPPFLAGS], [" -D_THREAD_SAFE"])
        AS_VAR_APPEND([JAVA_LDFLAGS], [" -pthread"])
        # AS_VAR_APPEND([LIBS], [" -lc_r"])
      ],
      [linux],
      [
        AS_VAR_APPEND([JAVA_CPPFLAGS], [" -D_REENTRANT"])
        AS_VAR_APPEND([JAVA_LDFLAGS], [" -lpthread"])
        # AS_VAR_APPEND([LIBS], [" -lpthread"])
      ],
      [solaris],
      [
        AS_VAR_APPEND([JAVA_CPPFLAGS], [" -D_POSIX_C_SOURCE=199506L"])
        AS_VAR_APPEND([JAVA_LDFLAGS], [" -lpthread"])
        # AS_VAR_APPEND([LIBS], [" -lpthread"])
      ],
      [macos],
      [
        # AS_VAR_APPEND([JAVA_CPPFLAGS], [""])
        # AS_VAR_APPEND([JAVA_LDFLAGS], [" -lpthread"])
        # AS_VAR_APPEND([LIBS], [" -lpthread"])
      ],
      [hpux],
      [
        AS_VAR_APPEND([JAVA_CFLAGS], [" -Ae +z"])
        AS_VAR_APPEND([JAVA CPPFLAGS], [" -DNATIVE -D_POSIX_C_SOURCE=199506L"])
        AS_VAR_APPEND([JAVA_LDFLAGS], [" -lpthread"])
        # AS_VAR_APPEND([LIBS], [" -lpthread"])
      ],
      [irix],
      [
        # AS_VAR_APPEND([JAVA_CFLAGS], [""])
        AS_VAR_APPEND([JAVA_LDFLAGS], [" -lpthread"])
        AS_VAR_APPEND([LIBS], [" -lpthread"])
      ],
      [aix],
      [
        AS_VAR_APPEND([JAVA_CPPFLAGS], [" -D_REENTRANT"])
        AS_VAR_APPEND([JAVA_LDFLAGS], [" -lpthread"])
        AS_VAR_APPEND([LIBS], [" -lpthread"])
      ],
      [osf],
      [
        AS_VAR_APPEND([JAVA_CPPFLAGS], [" -D_REENTRANT -D_OSF_SOURCE"])
        AS_VAR_APPEND([JAVA_LDFLAGS], [" -lpthread"])
        AS_VAR_APPEND([LIBS], [" -lpthread"])
      ])
    ],
    [AC_MSG_RESULT([no])])
  ],
  [AC_MSG_RESULT([no])])

  # Test for programs ant, jar, java and javac.

  AS_IF([test "x${have_java}" = "xyes"],
  [
    AC_PATH_PROG([ANT], [ant], [no])
    AS_IF([test "x${ANT}" = "xno"], [have_java="no"])

    AC_PATH_PROG([JAR], [jar], [no])
    AS_IF([test "x${JAR}" = "xno"], [have_java="no"])

    AC_PATH_PROG([JAVA], [java], [no])
    AS_IF([test "x${JAVA}" = "xno"], [have_java="no"])

    AC_PATH_PROG([JAVAC], [javac], [no])
    AS_IF([test "x${JAVAC}" = "xno"], [have_java="no"])
  ])

  AS_IF([test "x${have_java}" = "xyes"],
  [
    AC_DEFINE([HAVE_JAVA], [1],
    [Define to 1 if the Java Native Interface (JNI) is available.])

    ### FIXME: Append -DDEBIAN for the moment.
    # Debian uses PAM service "ssh" instead of "login", see ajjava.c
    # This could use AC_DEFINE() if no better option was avialable.
    # Ultimately, this should be configurable via server configuration
    # files.
    AS_IF([test -f "/etc/debian_release" || test -f /etc/debian_version],
    [AS_VAR_APPEND([JAVA_CPPFLAGS], [" -DDEBIAN"])])
  ])

  AC_ARG_VAR([ANT], [Path to the Apache Ant make tool])
  AC_ARG_VAR([JAR], [Path to the Java archive tool])
  AC_ARG_VAR([JAVA], [Path to the Java application launcher])
  AC_ARG_VAR([JAVAC], [Path to the Java compiler])

  AC_SUBST([JAVA_CFLAGS])
  AC_SUBST([JAVA_CPPFLAGS])
  AC_SUBST([JAVA_LDFLAGS])

  AM_CONDITIONAL([JAVA_BUILD], [test "x${have_java}" = "xyes"])
])
