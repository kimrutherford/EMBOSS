#!/usr/bin/perl -w

# Uses the EFUNC database to check for unused functions by module

open (MODS, "getz -lv '[efunc-mod:*]'|")
  || die "unable to list modules in EFUNC";

while (<MODS>) {
  if (!(/^\s+(\S+)\s+(\d+)/)) {
    print "Bad format returned by modules list:\n";
    print;
    next;
  }
  $mod = $1;
# $num = $2;
  open (AJAXMODS, "getz -c '[efunc-lib:ajax] & [efunc-mod:$mod]'|")
    || die "Cannot check modules in AJAX for $mod";
  $amods = <AJAXMODS>;
  close AJAXMODS;
  if (!defined($amods)) {
    $anum = 0;
#   print "$mod not in AJAX\n";
    next;
  }
  else {
    $anum = int($amods);
    print "$mod used in AJAX $anum times\n";
  }

####################################
# check for functions in this module
####################################

  open (AJAXFUN, "getz '[efunc-mod:$mod] ! [efunc-mod:$mod] < efunc_up'|")
	|| die "unable to list unused function names of module $mod";

  while (<AJAXFUN>) {
    /^EFUNC:(\S+)/;
    $fun = $1;
    print "Unused function $mod/$fun\n";
  }
 close AJAXFUN;

}
close MODS;
