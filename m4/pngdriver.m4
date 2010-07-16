dnl @synopsis CHECK_PNGDRIVER()
dnl
dnl This macro searches for an installed png/gd/zlib library. If nothing
dnl was specified when calling configure, it searches first in /usr/local
dnl and then in /usr. If the --with-pngdriver=DIR is specified, it will try
dnl to find it in DIR/include/zlib.h and DIR/lib/libz.a. If --without-pngdriver
dnl is specified, the library is not searched at all.
dnl
dnl It defines the symbol PLD_png if the librarys are found. You should
dnl use autoheader to include a definition for this symbol in a config.h
dnl file.
dnl
dnl Sources files should then use something like
dnl
dnl   #ifdef PLD_png
dnl   #include <zlib.h>
dnl   #endif /* PLD_png */
dnl
dnl @author Ian Longden <il@sanger.ac.uk>
dnl Modified: Alan Bleasby. Corrected library order
dnl

AC_DEFUN([CHECK_PNGDRIVER],
#
# Handle user hints
#
[AC_MSG_CHECKING(if png driver is wanted)
AC_ARG_WITH([pngdriver],
    [AS_HELP_STRING([--with-pngdriver=@<:@DIR@:>@],
        [root directory path of png/gd/zlib installation (defaults to /usr)])]
    [AS_HELP_STRING([--without-pngdriver],
        [to disable pngdriver usage completely])],
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
# Locate png/gd/zlib, if wanted
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

	  ICCHECK=0
	  if test "`uname`" = "SunOS"; then
	        AC_CHECK_LIB(iconv, libiconv_close, ICCHECK=1, ICCHECK=0, -L${ALT_HOME}/lib -liconv)
	if test $ICCHECK = "1" ; then
	        LDFLAGS="${LDFLAGS} -L${ALT_HOME}/lib -liconv"
	fi
	    LDFLAGS="$LDFLAGS -R$ALT_HOME/lib"
          fi





#
# Check for zlib in ALT_HOME
#
        AC_CHECK_LIB(z, inflateEnd, CHECK=1, CHECK=0, -L${ALT_HOME}/lib -lz)
#

#
# Check for png
#
	if test $CHECK = "1" ; then
	  AC_CHECK_LIB(png, png_destroy_read_struct, CHECK=1, CHECK=0 , -L${ALT_HOME}/lib -lz)
	fi
	



#
# Check for gd
#
	if test $CHECK = "1"; then
	  AC_CHECK_LIB(gd, gdImageCreateFromPng, CHECK=1, CHECK=0 , -L${ALT_HOME}/lib -lgd -lpng -lz -lm)
          if test $CHECK = "0"; then
		echo need to upgrade gd for png driver for plplot
	  fi
	fi
#
# If everything found okay then proceed to include png driver in config.
#
	if test $CHECK = "1" ; then
	  LIBS="$LIBS -lgd -lpng -lz -lm"

	  if test $ICCHECK = "1" ; then
		  LIBS="$LIBS -liconv"
	  fi
        
	  if test "`uname`" = "SunOS"; then
	    LDFLAGS="$LDFLAGS -R$ALT_HOME/lib"
          fi

	  AC_DEFINE(PLD_png)
	  AM_CONDITIONAL(AMPNG, true)
	  echo PNG libraries found
	    if test $ALT_HOME = "/usr" ; then
		  LDFLAGS="$ALT_LDFLAGS"
		  CPPFLAGS="$ALT_CPPFLAGS"
	    fi
	else
#
# If not okay then reset FLAGS.
#
  	  AM_CONDITIONAL(AMPNG, false)
	  LDFLAGS="$ALT_LDFLAGS"
	  CPPFLAGS="$ALT_CPPFLAGS"
	  echo No png driver will be made due to librarys missing/old.
	fi
#       echo PNG STUFF FOLLOWS!!!
#       echo CHECK = $CHECK
#       echo LIBS = $LIBS
#       echo LDFLAGS = $LDFLAGS
#       echo CPPFLAGS = $CPPFLAGS


else
        if test $withval != "no"; then
		echo "Directory $ALT_HOME does not exist"
		exit 0
        fi
fi
])
