#!/usr/bin/perl -w
chdir("..");
system("ls org/emboss/jemboss/*java org/emboss/jemboss/*/*java org/emboss/jemboss/*/*/*java > x");
open(IN, "x"); 
my $java; 
while($line = <IN>)
{
 chomp($line);
 $line =~ /(.*)(\.java)/;
 $java = $java." ";
 $java = $java.$1;
 $java = $java."  \\\n";
}
system("rm -rf x");


print $java;
