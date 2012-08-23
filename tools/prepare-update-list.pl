#!/opt/perl/bin/perl
#

use strict;
use warnings;

my $priority = shift || 2;

while (<>) {
    chomp;
    next if !$_;

    my $url = $_;
    print "push_url\t$priority\thtml\tfuhu\t1\t$url\n";
}


