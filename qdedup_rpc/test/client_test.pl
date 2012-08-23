#!/usr/bin/env perl

use strict;
use warnings;

use Test::More;
use base qw(Test::Class);

sub initial : Test(setup) {
}

sub end : Test(teardown) {
    print "End one test\n";
}

sub test_client_startup:Test(2) {
    my $re = system('../dedup_client/dedup_client -h');
    ok ($re == 0, "client help");

    $re = system('../dedup_client/dedup_client');
    ok ($re != 0, "client failed");
}

sub test_client_run:Test( ) {
    system("rm -rf ./dedup_mmap_file");

    my $re = system("../dedup_server/dedup_server -D");
    ok ($re == 0);
    
    $re = system("cat 50_normal | ../dedup_client/dedup_client -S localhost:8881 -o output");

    ok ($re == 0);
    open FH, "output.out" or die "cannot open sout";
    my @files = <FH>;
    my $count = @files;
   
    open FH2, "output.del" or die "cannot open serr";
    my @files2 = <FH2>;
    my $count2 = @files2;

    open FH3, "output.bad_list" or die "cannot open serr";
    my @files3 = <FH3>;
    my $count3 = @files3;

    ok ($count == 11, "11 passed");
    ok ($count2 == 0, "0 deduped");
    ok ($count3 == 39, "39 bad list");
    close FH;
    close FH2;
    close FH3;

    ## test 2
    $re = system("cat 50_normal | ../dedup_client/dedup_client -S localhost:8881 -o output");

    ok ($re == 0);

   open FH, "output.out" or die "cannot open sout";
    my @files = <FH>;
    my $count = @files;
   
    open FH2, "output.del" or die "cannot open serr";
    my @files2 = <FH2>;
    my $count2 = @files2;

    open FH3, "output.bad_list" or die "cannot open serr";
    my @files3 = <FH3>;
    my $count3 = @files3;

    ok ($count == 2, "2 passed");
    ok ($count2 == 9, "9 deduped");
    ok ($count3 == 39, "39 bad list");
    close FH;
    close FH2;
    close FH3;

    system("pkill -2 dedup_server");
    system("rm -rf ./dedup_mmap_file");
}


sub test_client_run_close:Test:Test( ) {
    system("rm -rf ./dedup_mmap_file");

    my $re = system("../dedup_server/dedup_server -D");
    ok ($re == 0);
    
    $re = system("cat 50_normal | ../dedup_client/dedup_client -S localhost:8881 -o output");

    ok ($re == 0);
    open FH, "output.out" or die "cannot open sout";
    my @files = <FH>;
    my $count = @files;
   
    open FH2, "output.del" or die "cannot open serr";
    my @files2 = <FH2>;
    my $count2 = @files2;

    open FH3, "output.bad_list" or die "cannot open serr";
    my @files3 = <FH3>;
    my $count3 = @files3;

    ok ($count == 11, "11 passed");
    ok ($count2 == 0, "0 deduped");
    ok ($count3 == 39, "39 bad list");
    close FH;
    close FH2;
    close FH3;

    system("pkill -2 dedup_server");
    system("../dedup_server/dedup_server -D");
    ## test 2
    $re = system("cat 50_normal | ../dedup_client/dedup_client -S localhost:8881 -o output");

    ok ($re == 0);

    open FH, "output.out" or die "cannot open sout";
    my @files = <FH>;
    my $count = @files;
   
    open FH2, "output.del" or die "cannot open serr";
    my @files2 = <FH2>;
    my $count2 = @files2;

    open FH3, "output.bad_list" or die "cannot open serr";
    my @files3 = <FH3>;
    my $count3 = @files3;

    ok ($count == 2, "2 passed");
    ok ($count2 == 9, "9 deduped");
    ok ($count3 == 39, "39 bad list");
    close FH;
    close FH2;
    close FH3;

    system("pkill -2 dedup_server");
    system("rm -rf ./dedup_mmap_file");
}

sub test_multi_server:Test() {
    system("rm -rf ./dedup_mmap_test_1 ./dedup_mmap_test_2");

    my $re = system("../dedup_server/dedup_server -f dedup_mmap_test_1 -s 30 -p 8895 -D");
    ok ($re == 0);

    $re = system("../dedup_server/dedup_server -f dedup_mmap_test_2 -s 30 -p 8995 -D");
    ok ($re == 0);
    
    $re = system("cat 50_normal | ../dedup_client/dedup_client -S localhost:8895,localhost:8995 -o output");

    ok ($re == 0);
    open FH, "output.out" or die "cannot open sout";
    my @files = <FH>;
    my $count = @files;
   
    open FH2, "output.del" or die "cannot open serr";
    my @files2 = <FH2>;
    my $count2 = @files2;

    open FH3, "output.bad_list" or die "cannot open serr";
    my @files3 = <FH3>;
    my $count3 = @files3;

    ok ($count == 11, "11 passed");
    ok ($count2 == 0, "0 deduped");
    ok ($count3 == 39, "39 bad list");
    close FH;
    close FH2;
    close FH3;

    ## test 2
    $re = system("cat 50_normal | ../dedup_client/dedup_client -S localhost:8895,localhost:8995 -o output");

    ok ($re == 0);

    open FH, "output.out" or die "cannot open sout";
    my @files = <FH>;
    my $count = @files;
   
    open FH2, "output.del" or die "cannot open serr";
    my @files2 = <FH2>;
    my $count2 = @files2;

    open FH3, "output.bad_list" or die "cannot open serr";
    my @files3 = <FH3>;
    my $count3 = @files3;

    ok ($count == 2, "2 passed");
    ok ($count2 == 9, "9 deduped");
    ok ($count3 == 39, "39 bad list");
    close FH;
    close FH2;
    close FH3;

    system("pkill -9 dedup_server");
    system("rm -rf ./dedup_mmap_test_*");

}

Test::Class->runtests();
