AC_DEFUN([CHECK_GENERAL],
#
# Handle general setup e.g. documentation directory
#
[AC_MSG_CHECKING(if docroot is given)
AC_ARG_WITH([docroot],
    [AS_HELP_STRING([--with-docroot=DIR],
        [root directory path of documentation (defaults to none)])],
[if test "$withval" != no ; then
  AC_MSG_RESULT(yes)
  CPPFLAGS="$CPPFLAGS -DDOC_ROOT=\\\"$withval\\\""
fi], [
AC_MSG_RESULT(no)
])
]


# GCC profiling
[AC_MSG_CHECKING(if gcc profiling is selected)
AC_ARG_WITH([gccprofile],
    [AS_HELP_STRING([--with-gccprofile], [selects profiling])],
[if test "$withval" != no ; then
  AC_MSG_RESULT(yes)
  CFLAGS="$CFLAGS -g -pg"
  LDFLAGS="$LDFLAGS -pg"
fi], [
AC_MSG_RESULT(no)
])

]

# GOV coverage
[AC_MSG_CHECKING(if gcov coverage is selected)
AC_ARG_WITH([gcov],
    [AS_HELP_STRING([--with-gcov], [selects profiling])],
[if test "$withval" != no ; then
  AC_MSG_RESULT(yes)
  CFLAGS="$CFLAGS -fprofile-arcs -ftest-coverage"
fi], [
AC_MSG_RESULT(no)
])

]
)

