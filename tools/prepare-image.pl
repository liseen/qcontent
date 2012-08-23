#!/opt/perl/bin/perl
#

use strict;
use warnings;

my $priority = shift || 3;

while (<>) {
    chomp;
    next if !$_;

    my $url = $_;
    print "push_url\t$priority\timage\tgihi\t0\t$url\n";
}


