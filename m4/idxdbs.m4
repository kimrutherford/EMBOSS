AC_DEFUN([CHECK_IDXDBS],
[
AC_MSG_CHECKING(for EMBOSS pre-indexed databases)


if test -f ./emboss/index/edam.xac; then
AC_MSG_RESULT(yes)
else
AC_MSG_RESULT(no)
echo ""
echo "Pre-indexed edam, taxon + drcat databases not found."
echo "Please download them from within this directory using:"
echo "    rsync -av rsync://emboss.open-bio.org/EMBOSS/ ."
echo "and then repeat the configure step."
exit 1
fi
])
