AC_DEFUN([CHECK_GROUT],
#
# Handle GROUT flags
#
[AC_MSG_CHECKING(for gdome2)
AC_ARG_WITH(grout,
[  --with-grout [           Grout graphics output alternative [default=no]] ],
[if test "$withval" != no ; then
  AC_MSG_RESULT(yes)
  CPPFLAGS="$CPPFLAGS -DGROUT `gdome-config --cflags`"
  LDFLAGS="$LDFLAGS `gdome-config --libs`"
  if test "`uname`" = "SunOS"; then
      LDFLAGS="$LDFLAGS -R$withval/lib"
  fi

fi], [
AC_MSG_RESULT(no)
])
]
)
