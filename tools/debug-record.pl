#!/usr/bin/env perl

use strict;
use warnings;
use Smart::Comments;

use Getopt::Long;
use TokyoTyrant;
use Digest::MD5 qw(md5 md5_hex);

my $host = "localhost";
my $port = 9870;

GetOptions("host=s"  => \$host,
           "port=i" => \$port,
        );

my $key = shift or die "no key gived!";
if ($key =~ /^http/) {
    $key = md5_hex($key);
}

# create the object
my $rdb = TokyoTyrant::RDBTBL->new();
# connect to the server
if(!$rdb->open($host, $port)){
    my $ecode = $rdb->ecode();
    warn ("open error: %s\n", $rdb->errmsg($ecode));
    exit 1;
}

# search for records
my $rcols = $rdb->get($key);
### $rcols;

# close the connection
if(!$rdb->close()){
    my $ecode = $rdb->ecode();
    printf STDERR ("close error: %s\n", $rdb->errmsg($ecode));
}





