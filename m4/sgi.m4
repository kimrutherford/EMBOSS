AC_DEFUN([CHECK_SGI],
#
# Handle SGI compiler flags
#
[AC_MSG_CHECKING(for sgiabi)
AC_ARG_WITH([sgiabi],
    [AS_HELP_STRING([--with-sgiabi=@<:@ARG@:>@],
        [SGI compiler flags @<:@default=no@:>@])],
[if test "$withval" != no ; then
  AC_MSG_RESULT(yes)

  if test "`uname`" = "IRIX64" || test "`uname`" = "IRIX" ; then

    if test "$withval" = n32m3 ; then
      CFLAGS="-n32 -mips3 $CFLAGS"
      LD="/usr/bin/ld -n32 -mips3 -IPA -L/usr/lib32"
	if test -d /usr/freeware ; then
          LDFLAGS="-L/usr/freeware/lib32 $LDFLAGS"
        fi
    fi

    if test "$withval" = n32m4 ; then
      CFLAGS="-n32 -mips4 $CFLAGS"
      LD="/usr/bin/ld -n32 -mips4 -IPA -L/usr/lib32"
	if test -d /usr/freeware ; then
          LDFLAGS="-L/usr/freeware/lib32 $LDFLAGS"
        fi
    fi

    if test "$withval" = 64m3 ; then
      CFLAGS="-64 -mips3 $CFLAGS"
      LD="/usr/bin/ld -64 -mips3 -IPA -L/usr/lib64"
	if test -d /usr/freeware ; then
          LDFLAGS="-L/usr/freeware/lib64 $LDFLAGS"
        fi
    fi

    if test "$withval" = 64m4 ; then
      CFLAGS="-64 -mips4 $CFLAGS"
      LD="/usr/bin/ld -64 -mips4 -IPA -L/usr/lib64"
	if test -d /usr/freeware ; then
          LDFLAGS="-L/usr/freeware/lib64 $LDFLAGS"
        fi
    fi

  fi


fi], [
AC_MSG_RESULT(no)
])
]
)
