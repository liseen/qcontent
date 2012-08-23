#!/opt/perl/bin/perl
#

use strict;
use warnings;

my $priority = shift || 3;

while (<>) {
    chomp;
    next if !$_;
    my $line = $_;
    my ($md5, $download_time, $order, $img_url) = split /\t/, $line;
    my $crawl_tag = $md5 . "_" . $download_time . "_" . $order;
    print "push_url\t$priority\tthumb\tgihi\t0\t$img_url\t\t$crawl_tag\n";
}


