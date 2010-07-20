dnl @synopsis CHECK_HPDF()
dnl
dnl This macro searches for an installed libhpdf (libharu) library. If nothing
dnl was specified when calling configure, it first searches in /usr/local
dnl and then in /usr. If the --with-hpdf=DIR is specified, it will try
dnl to find it in DIR/include and DIR/lib.
dnl
dnl It defines the symbol PLD_pdf if the library is found.
dnl


AC_DEFUN([CHECK_HPDF],
#
# Handle user hints
#
[AC_MSG_CHECKING(whether to look for pdf support)
AC_ARG_WITH([hpdf],
    [AS_HELP_STRING([--with-hpdf=DIR],
        [root directory path of hpdf installation @<:@defaults to /usr@:>@])]
    [AS_HELP_STRING([--without-hpdf],
        [to disable pdf support])],
[if test "$withval" != no ; then
  AC_MSG_RESULT(yes)
  ALT_HOME="$withval"
else
  AC_MSG_RESULT(no)
fi], [
AC_MSG_RESULT(yes)
ALT_HOME=/usr
])


#
# Locate hpdf
#
if test -d "${ALT_HOME}"
then

#
# Keep a copy if it fails
#
	ALT_LDFLAGS="$LDFLAGS"
	ALT_CPPFLAGS="$CPPFLAGS"

#
# Set 
#
        LDFLAGS="${LDFLAGS} -L${ALT_HOME}/lib"
        CPPFLAGS="$CPPFLAGS -I$ALT_HOME/include"

#
# Check for libharu in ALT_HOME
#
        AC_CHECK_LIB(hpdf, HPDF_New, CHECK=1, CHECK=0, -L${ALT_HOME}/lib)
#
#
# If everything found okay then proceed to include png driver in config.
#
	if test $CHECK = "1" ; then
	  LIBS="$LIBS -lhpdf"

	  if test "`uname`" = "SunOS"; then
	    LDFLAGS="$LDFLAGS -R$ALT_HOME/lib"
          fi

	  AC_DEFINE(PLD_pdf)
	  AM_CONDITIONAL(AMPDF, true)
	  echo PDF support found
	    if test $ALT_HOME = "/usr" ; then
		  LDFLAGS="$ALT_LDFLAGS"
		  CPPFLAGS="$ALT_CPPFLAGS"
	    fi
	else
#
# If not okay then reset FLAGS.
#
  	  AM_CONDITIONAL(AMPDF, false)
	  LDFLAGS="$ALT_LDFLAGS"
	  CPPFLAGS="$ALT_CPPFLAGS"
	  echo "No pdf support (libhpdf) found."
	fi

else
        if test $withval != "no"; then
		echo "Directory $ALT_HOME does not exist"
		exit 0
        fi
fi
])
