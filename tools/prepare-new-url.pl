#!/opt/perl/bin/perl
#

use strict;
use warnings;

while (<>) {
    chomp;

    next if !$_;
    my ($priority, $crawl_type, $max_crawl_level, $url, $crawl_level, $anchor_text, $parent_url_md5) = split /\t/, $_, -1;

    # priority, want_type, crawl_type, max_crawl_level, url,
    # condition, crawl_tag, crawl_level, anchor_text, parent_url_md5
    print "push_url\t$priority\thtml\t$crawl_type\t$max_crawl_level\t$url\t\t\t$crawl_level\t$anchor_text\t$parent_url_md5\n";
}


