#!/usr/bin/perl

sub do_cmd_EMBOSS {
    local ($_) = @_;
    join('', "<b>EMBOSS</b>", $_);
}

sub do_cmd_URL {
    local ($_) = @_;
    local ($href);
    $href = &missing_braces unless
	s/$next_pair_pr_rx/$href=$2;''/eo;
    join('', " <a href=\"$href\">$href</a>", $_);
}

sub do_cmd_filename {
    local ($_) = @_;
    local ($fname);
    $fname = &missing_braces unless
	s/$next_pair_pr_rx/$fname=$2;''/eo;
    join('', " <pre>$fname</pre>", $_);
}

sub do_cmd_progname {
    local ($_) = @_;
    local ($pname);
    $pname = &missing_braces unless
	s/$next_pair_pr_rx/$pname=$2;''/eo;
    join('', " <i>$pname</i>", $_);
}

sub do_cmd_ilcomm {
    local ($_) = @_;
    local ($cname);
    $cname = &missing_braces unless
	s/$next_pair_pr_rx/$cname=$2;''/eo;
    join('', " <pre>$cname</pre>", $_);
}


1;
