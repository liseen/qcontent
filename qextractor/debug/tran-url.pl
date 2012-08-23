#!/usr/bin/env perl

use strict;
use warnings;
#use Smart::Comments;

use Getopt::Long;
use TokyoTyrant;

my $server = "localhost:9850";

GetOptions("server=s"  => \$server
        );

# connect to the server
my ($s_host, $s_port) = split /:/, $server;
# create the object
my $rdb = TokyoTyrant::RDB->new();

if(!$rdb->open($s_host, $s_port)){
    my $ecode = $rdb->ecode();
    printf STDERR ("open error: %s\n", $rdb->errmsg($ecode));
}

# traverse records
$rdb->iterinit();
while(defined(my $key = $rdb->iternext())){
    my $value = $rdb->get($key);
    my ($st, $time) = split /:/, $value;
    print "$key\t$st\t$time\n";
}


# close the connection
if(!$rdb->close()){
    my $ecode = $rdb->ecode();
    printf STDERR ("close error: %s\n", $rdb->errmsg($ecode));
}


