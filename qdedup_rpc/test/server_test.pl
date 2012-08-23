#!/usr/bin/env perl

use strict;
use warnings;
use IO::Socket::INET;

use Test::More;
use base qw(Test::Class);

sub initial : Test(setup) {
}

sub end : Test(teardown) {
    print "End one test\n";
}

sub test_server_2:Test(3) {
    system("rm -rf ./dedup_mmap_file");
    my $re = system("../dedup_server/dedup_server &");
    system("ls");

    sleep 1;
    ok ($re == 0);
    my @stat_re = stat("dedup_mmap_file");

    ok ($stat_re[7] == 100*1024*1024, "default file size"); # 100M

    my $socket = IO::Socket::INET->new(PeerAddr => "localhost", 
                                       PeerPort => 8881,
                                       Proto  => 'udp');
    ok ($socket, "not empty socket");
    system("pkill -9 dedup_server");
    system("rm -rf ./dedup_mmap_file");
}

sub test_server_3:Test(3) {
    system("rm -rf ./dedup_mmap_test");

    my $re = system("../dedup_server/dedup_server -f dedup_mmap_test -s 30 -p 8895&");
    system("ls");

    sleep 1;

    ok ($re == 0);
    my @stat_re = stat("./dedup_mmap_test");
    ok ($stat_re[7] == 30*1024*1024, "30M"); # 30M

    my $socket = IO::Socket::INET->new(PeerAddr => "localhost", 
                                       PeerPort => 8895,
                                       Proto  => 'udp');
    ok ($socket, "not empty socket");

    system("pkill -9 dedup_server");
    system("rm -rf ./dedup_mmap_test");
}

sub test_multi_server:Test(4) {
    system("rm -rf ./dedup_mmap_test_1 ./dedup_mmap_test_2");

    my $re = system("../dedup_server/dedup_server -f dedup_mmap_test_1 -s 30 -p 8895&");
    ok ($re == 0);

    $re = system("../dedup_server/dedup_server -f dedup_mmap_test_2 -s 30 -p 8995&");
    ok ($re == 0);

    my $socket = IO::Socket::INET->new(PeerAddr => "localhost", 
                                       PeerPort => 8895,
                                       Proto  => 'udp');
    ok ($socket, "not empty first server");

    my $socket2 = IO::Socket::INET->new(PeerAddr => "localhost", 
                                       PeerPort => 8995,
                                       Proto  => 'udp');
    ok ($socket, "not empty second server");

    system("pkill -9 dedup_server");
    system("rm -rf ./dedup_mmap_test_*");
}

sub test_server:Test(1) {
    my $re = system('../dedup_server/dedup_server -h');
    ok ($re == 0, "server help");
}

Test::Class->runtests();
