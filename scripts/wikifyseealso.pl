#!/usr/bin/perl -w

print "{|\n";

while (<>) {
    if(/<a href=[^>]+>([^<>]+)</) {
	$name = $1;
	$cname = ucfirst($name);
	print "|-\n";
	print "|[[:Appdoc:$cname|$name]]\n";
	print "|\{\{:Appinc:Short$cname\}\}\n";
    }
}
print "|}\n";



