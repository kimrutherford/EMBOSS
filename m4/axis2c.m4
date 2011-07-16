dnl @synopsis CHECK_AXIS2C()
dnl
dnl This macro searches for installed axis2c libraries only
dnl if the --with-axis2c[=dir] switch is given. With no
dnl directory specified it will default to /usr
dnl

AC_DEFUN([CHECK_AXIS2C],
[
AC_MSG_CHECKING([whether to look for axis2c support])

AC_ARG_WITH([axis2c],[AS_HELP_STRING([--with-axis2c=@<:@DIR@:>@],
            [root directory of axis2c installation])],
            [], [with_axis2c=no])

AXIS2C_CPPFLAGS=""
AXIS2C_LDFLAGS=""

found_axis2c="no"

AS_IF([test "x$with_axis2c" != xno],
  [
    AC_MSG_RESULT([yes])
    if test "x$with_axis2c" != "xyes" ; then
        ALT_HOME=$with_axis2c
        if test ! -d "${ALT_HOME}" ; then
          AC_MSG_ERROR([No such axis2c directory (${ALT_HOME})])
        fi
	 export PKG_CONFIG_PATH="$PKG_CONFIG_PATH:$ALT_HOME/lib/pkgconfig"
    else
        ALT_HOME="/usr"
    fi

    PKG_CHECK_MODULES([AXIS2C],[axis2c],
    [
        AXIS2C_CPPFLAGS=${AXIS2C_CFLAGS}
        AXIS2C_LDFLAGS=${AXIS2C_LIBS}
    ],
    [
        echo pkg-config axis2c detection failed
    ])


    A2CTMPLIBS="${LIBS}"

    LIBS="${AXIS2C_LDFLAGS} ${LIBS}"
    AC_CHECK_LIB([axis2_engine], [axis2_svc_client_create],
    [
       AC_DEFINE([HAVE_AXIS2C],[1],[Define to 1 if using axis2c libraries])
       AC_DEFINE_UNQUOTED([AXIS2C_LOC],["${ALT_HOME}"],[AXIS2C location])
       found_axis2c="yes"
       AXIS2C_LDFLAGS="${AXIS2C_LDFLAGS} -laxis2_engine -lneethi -laxis2_http_sender -laxis2_http_receiver -laxis2_http_common -laxis2_axiom -laxis2_parser -lguththila -laxutil"
    ],
    [
       AC_MSG_RESULT([no])
       found_axis2c="no"
    ])

    LIBS="${A2CTMPLIBS}"

  ],

  [
      AC_MSG_RESULT([no])
      found_axis2c="no"
  ])

# AM_CONDITIONAL([USEAXIS2C], [test "$found_axis2c" = "yes"])

AC_SUBST([AXIS2C_CPPFLAGS])
AC_SUBST([AXIS2C_LDFLAGS])
])
