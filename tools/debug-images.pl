#!/usr/bin/env perl

use strict;
use warnings;
#use Smart::Comments;

use Getopt::Long;
use TokyoTyrant;
use Digest::MD5 qw(md5 md5_hex);

my $host = "localhost";
my $port = 9870;

GetOptions("host=s"  => \$host,
           "port=i" => \$port,
        );

# create the object
my $rdb = TokyoTyrant::RDBTBL->new();
# connect to the server
if(!$rdb->open($host, $port)){
    my $ecode = $rdb->ecode();
    warn ("open error: %s\n", $rdb->errmsg($ecode));
    exit 1;
}

while (<>) {
    chomp;
    my $key = $_;
    next if !$key;

    if ($key =~ /^http:/) {
        $key = md5_hex($key);
    }
    # search for records
    my $rcols = $rdb->get($key);
    ### $rcols;
    my $url = $rcols->{url} || '';
    my $title = $rcols->{title} || '';
    my $raw_title = $rcols->{raw_title} || '';
    my $content = $rcols->{content} || '';
    my $content_confidence = $rcols->{content_confidence} || 0;
    my $list_confidence = $rcols->{list_confidence} || 0;

    next if $content_confidence < 90;
    next if $list_confidence >= 10;
    next if length($content) < 500;
    next if $title eq $raw_title;

    my $images = $rcols->{images} || '';
    my @imgs = split /\n/, $images;
    my $i = 1;
    for my $img (@imgs) {
        next if !$img;
        next if $img =~ /\b(?:logo|taobao|daohang|guanggao|print)/;
        print "$key\t$url\t$i\t$img\n";
        ++$i;
    }
}

# close the connection
if(!$rdb->close()){
    my $ecode = $rdb->ecode();
    printf STDERR ("close error: %s\n", $rdb->errmsg($ecode));
}



