#!/usr/bin/perl -w
use strict;

# Example perl script
print "HTTP/1.x 200 OK\n";
print "Content-Type: text/html; charset=UTF-8;\n\n";
print "Today is ".`date`;
