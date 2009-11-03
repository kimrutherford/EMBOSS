dnl @synopsis CHECK_AMD64()
dnl
dnl This macro tests whether a linux processor is an x86_64 and, if so,
dnl sets CFLAGS to contain -D__amd64__
dnl @author: Alan Bleasby.
dnl

AC_DEFUN([CHECK_AMD64],
#
#
#
[AC_MSG_CHECKING([if Linux x86_64])


if test "`uname -a | grep Linux`"; then
  if test "`uname -a | grep x86_64`"; then
    CFLAGS="${CFLAGS} -D__amd64__"
    AC_MSG_RESULT(yes)
  else
    AC_MSG_RESULT(no)
  fi
else
 AC_MSG_RESULT(no)
fi

])
