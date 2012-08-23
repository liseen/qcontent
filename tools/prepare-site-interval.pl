#!/opt/perl/bin/perl
#

use strict;
use warnings;

my $interval = shift
    or die "No interval specified!";

while (<>) {
    chomp;
    next if !$_;

    my $site = $_;
    print "set_site_interval\t$site\t$interval\n";
}


