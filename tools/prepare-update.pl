#!/opt/perl/bin/perl
#

use strict;
use warnings;

my $priority = shift || 1;

while (<>) {
    chomp;
    next if !$_;

    my $url = $_;
    print "push_url\t$priority\thtml\tguhi\t0\t$url\n";
}


