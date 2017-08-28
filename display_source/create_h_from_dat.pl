#!/usr/bin/perl -w

open( INFILE, $ARGV[0] ) || die $!;

print "enum {\n";

while( <INFILE> ) {
    @parts = split;
    $name = $parts[0];
    $name =~ s/"//g;
    print "\ti$name,\n";
}

print "}\n";

close INFILE;
