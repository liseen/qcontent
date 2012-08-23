#!/bin/env perl

use strict;
use warnings;

use File::Slurp;

my $file_a = shift;
my $file_b = shift;

my @lines_a = read_file($file_a);
my @lines_b = read_file($file_b);

for my $i (0..$#lines_a) {
    chomp $lines_a[$i];
    chomp $lines_b[$i];
    if ($lines_a[$i] ne $lines_b[$i]) {
        print $lines_a[$i] . "\t" . $lines_b[$i] . "\n";
    }
}
