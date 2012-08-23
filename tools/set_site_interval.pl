#!/bin/env perl

use strict;
use warnings;

my $site = shift;

if ($site) {
    my $interval = shift
        or die "No intervel specified!";
    print "set_site_interval\t$site\t$interval\n";
} else {
    while (<>) {
        chomp;
        my ($site, $interval) = split /\t/,$_;
        print "set_site_interval\t$site\t$interval\n";
    }
}

