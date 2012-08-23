#!/opt/perl/bin/perl
#

use strict;
use warnings;

use Time::HiRes qw/time sleep/;

use lib "/export/m1/jianqiang.liu/perllib";

use jqliuLog;

my $priority = shift || 2;

my $lcnt = 0;
infoOut(" ... 0 ...");
my $t1 = time();
my $t2 = $t1;
while (<>) {
    chomp;
    next if !$_;

    ++$lcnt;
    if ($lcnt % 10000 == 0) {
        my $t3 = time();
        infoOut("  ... $lcnt " . sprintf("%.2f", $t3 - $t2) . " ...");
        $t2 = time();
    }
    my $url = $_;
    print "push_url\t$priority\thtml\tguhi\t0\t$url\n";
    #sleep(0.1) if ($lcnt % 1000 == 0);
}
my $t4 = time();
infoOut(" ... $lcnt " . sprintf("%.2f", $t4 - $t1));


