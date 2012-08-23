#!/bin/env perl

use strict;
use warnings;

my %dels;

while (<>) {
    my (undef, $host, $url, undef) = split /\t/, $_, 4;
    next if !$url;

    $url =~ s/\d{3,}/*/gs;
    my @blocks = split /[&?\/]/, $url;
    push @blocks, $url;
    for my $b (@blocks) {
        next if !$b || length($b) < 10;
        my $k = $host . "\t" .$b;
        ++$dels{$k};
    }
}

while (my ($k, $v) = each %dels) {
    if ($v > 10) {
        print "$k\t$v\n";
    }
}
