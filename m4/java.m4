dnl @synopsis CHECK_JAVA()
dnl
dnl Need to specify --with-java and --with-javaos
dnl @author Alan Bleasby (ableasby@hgmp.mrc.ac.uk)
dnl

AC_DEFUN([CHECK_JAVA],
#
# Handle user hints
#
[AC_MSG_CHECKING(if java include directory given)
AC_ARG_WITH(java,
[  --with-java=DIR         root directory path of java installation
  --without-java          to disable java],
[if test "$withval" != no ; then
  AC_MSG_RESULT(yes)
  JALT_HOME="$withval"
  if test -d $withval ; then
	echo "Java directory $withval exists"
  else
	echo "Java directory $withval does not exist"
	exit 0
  fi

  JALT_HOME="${JALT_HOME} -DHAVE_JAVA"
  AC_CHECK_PROG(havejavac,javac,"yes","no")
  if test "${havejavac}" != yes ; then
  echo "Error: Either JAVA not installed or 'javac' not in your PATH"
  exit 1
  fi
  if test -f /etc/debian_release || test -f /etc/debian_version ; then
     CFLAGS="$CFLAGS -DDEBIAN"
  fi
  JAVA_OK=yes
  AC_SUBST(JAVA_OK)
else
  JAVA_OK=no
  AC_SUBST(JAVA_OK)
  AC_MSG_RESULT(no)

fi], [

AC_MSG_RESULT(no)

])

if test -n "${JALT_HOME}"
then
	CFLAGS="$CFLAGS -I${JALT_HOME}"
fi

])



AC_DEFUN([CHECK_JAVAOS],
#
# Handle user hints
#
[AC_MSG_CHECKING(if java OS include directory given)
AC_ARG_WITH(javaos,
[  --with-javaos=DIR       root directory path of java installation include OS],
[if test "$withval" != no ; then
  AC_MSG_RESULT(yes)
	  if test -d $withval ; then
		echo "Javaos directory $withval exists"
	  else
		echo "Javaos directory $withval does not exist"
		exit 0
	  fi
	  ALT_HOMEOS="$withval"
else
  AC_MSG_RESULT(no)
fi], [
AC_MSG_RESULT(no)
	if test "$JAVA_OK" = "yes" ; then
		echo "Error: --with-java=dir unspecified"
		exit 0
	fi
])

if test -n "${ALT_HOMEOS}"
then
	CFLAGS="$CFLAGS -I${ALT_HOMEOS}"
fi


])



AC_DEFUN([CHECK_AUTH],
#
# Handle user authorisation
#
[AC_MSG_CHECKING(if any authorisation type is given)
AC_ARG_WITH(auth,
[  --with-auth=AUTHTYPE [   defaults PAM] ],
[if test "$withval" != no ; then
  AC_MSG_RESULT(yes)

	  ALT_AUTHTYPE="-D${withval}"

	  if test "$withval" = "yes" ; then
		ALT_AUTHTYPE="-DPAM"
	  fi
	  if test "$withval" = "pam" ; then
		ALT_AUTHTYPE="-DPAM"
	  fi
	  if test "$withval" = "shadow" ; then
		ALT_AUTHTYPE="-DN_SHADOW"
	  fi
	  if test "$withval" = "rshadow" ; then
		ALT_AUTHTYPE="-DR_SHADOW"
	  fi
	  if test "$withval" = "noshadow" ; then
		ALT_AUTHTYPE="-DNO_SHADOW"
	  fi
	  if test "$withval" = "rnoshadow" ; then
		ALT_AUTHTYPE="-DRNO_SHADOW"
	  fi
	  if test "$withval" = "aixshadow" ; then
		ALT_AUTHTYPE="-DAIX_SHADOW"
	  fi
	  if test "$withval" = "hpuxshadow" ; then
		ALT_AUTHTYPE="-DHPUX_SHADOW"
	  fi
if test "`uname`" != "IRIX64" && test "`uname`" != "IRIX" ; then
	  AC_CHECK_LIB(crypt, main, LDFLAGS="$LDFLAGS -lcrypt",LDFLAGS="$LDFLAGS")
fi
	  AC_CHECK_LIB(pam, main, LDFLAGS="$LDFLAGS -lpam",LDFLAGS="$LDFLAGS")
else
  AC_MSG_RESULT(no)
fi], [
AC_MSG_RESULT(no)
#	if test "$JAVA_OK" = "yes" ; then
#		echo "Error: --with-java=dir unspecified"
#		exit 0
#	fi
])

if test -n "${ALT_AUTHTYPE}"
then
	CFLAGS="$CFLAGS ${ALT_AUTHTYPE}"
else
	CFLAGS="$CFLAGS -DNO_AUTH"
fi


])















AC_DEFUN([CHECK_THREADS],
#
# Handle jemboss threading options
#
[AC_MSG_CHECKING(if any threading type is given)
AC_ARG_WITH(thread,
[  --with-thread=TYPE [     thread type [default=linux]] ],
[if test "$withval" != no ; then
  AC_MSG_RESULT(yes)

	  ALT_THREADTYPE="-D${withval}"

	  if test "$withval" = "yes" ; then
	        CFLAGS="$CFLAGS -D_REENTRANT" 
		LDFLAGS="$LDFLAGS -lpthread"
		LIBS="$LIBS -lpthread"
	  fi

	  if test "$withval" = "freebsd" ; then
	        CFLAGS="$CFLAGS -D_THREAD_SAFE -pthread" 
		LDFLAGS="$LDFLAGS"
		LIBS="$LIBS -lc_r"
	  fi

	  if test "$withval" = "linux" ; then
	        CFLAGS="$CFLAGS -D_REENTRANT" 
		LDFLAGS="$LDFLAGS -lpthread"
		LIBS="$LIBS -lpthread"
	  fi

	  if test "$withval" = "solaris" ; then
	        CFLAGS="$CFLAGS -D_POSIX_C_SOURCE=199506L" 
		LDFLAGS="$LDFLAGS -lpthread"
		LIBS="$LIBS -lpthread"
	  fi
	  if test "$withval" = "macos" ; then
	        CFLAGS="$CFLAGS"
		LDFLAGS="$LDFLAGS -lpthread"
		LIBS="$LIBS -lpthread"
	  fi
	  if test "$withval" = "hpux" ; then
	        CFLAGS="$CFLAGS -Ae +z -DNATIVE -D_POSIX_C_SOURCE=199506L" 
		LDFLAGS="$LDFLAGS -lpthread"
		LIBS="$LIBS -lpthread"
	  fi
	  if test "$withval" = "irix" ; then
	        CFLAGS="$CFLAGS" 
		LDFLAGS="$LDFLAGS -lpthread"
		LIBS="$LIBS -lpthread"
	  fi
	  if test "$withval" = "aix" ; then
	        CFLAGS="$CFLAGS -D_REENTRANT" 
		LDFLAGS="$LDFLAGS -lpthread"
		LIBS="$LIBS -lpthread"
	  fi
	  if test "$withval" = "osf" ; then
	        CFLAGS="$CFLAGS -D_REENTRANT -D_OSF_SOURCE" 
		LDFLAGS="$LDFLAGS -lpthread"
		LIBS="$LIBS -lpthread"
	  fi
else
  AC_MSG_RESULT(no)
fi], [
AC_MSG_RESULT(no)
])
])
