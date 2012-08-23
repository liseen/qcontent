#!/usr/bin/env perl

use strict;
use warnings;
use Smart::Comments;

use Getopt::Long;
use TokyoTyrant;

my $host = "localhost";
my $port = 9860;

GetOptions("host=s"  => \$host,
           "port=i" => \$port,
        );

my $key = shift or
    die "no md5 key gived\n";

# create the object
my $rdb = TokyoTyrant::RDB->new();

# connect to the server
if(!$rdb->open($host, $port)){
    my $ecode = $rdb->ecode();
    printf STDERR ("open error: %s\n", $rdb->errmsg($ecode));
}

my $value = $rdb->get("$key");
### $value

# close the connection
if(!$rdb->close()){
    my $ecode = $rdb->ecode();
    printf STDERR ("close error: %s\n", $rdb->errmsg($ecode));
}

__END__
# traverse records
$rdb->iterinit();
while(defined(my $key = $rdb->iternext())){
    my $value = $rdb->get($key);
    print "$key\t$value\n";
}


