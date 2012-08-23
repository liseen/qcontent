#!/usr/bin/env perl

use strict;
#use warnings;

use Getopt::Long;
use TokyoTyrant;

my $begin_time = time  - 1 * 24 * 60 * 60;
my $server = "localhost:19851";

GetOptions("server=s"  => \$server
        );

my $limit = shift;

my ($s_host, $s_port) = split /:/, $server;

# create the object
my $rdb = TokyoTyrant::RDBTBL->new();
# connect to the server
if(!$rdb->open($s_host, $s_port)){
    my $ecode = $rdb->ecode();
    warn ("open error: %s\n", $rdb->errmsg($ecode));
    exit 1;
}

# search for records
my $qry = TokyoTyrant::RDBQRY->new($rdb);

$qry->addcond("download_time", $qry->QCNUMGE, $begin_time);
$qry->setlimit($limit) if $limit;

my $res = $qry->search();
foreach my $rkey (@$res){
    my $rcols = $rdb->get($rkey);
    my @fields;
    my $url = $rkey;
    next if $url =~ /\n/;

    push @fields, $rcols->{download_time};
    push @fields, $rcols->{host};
    push @fields, $url;
    push @fields, $rcols->{url_md5};
    push @fields, $rcols->{parent_url_md5};
    push @fields, $rcols->{crawl_level};
    push @fields, $rcols->{loading_time};
    my $anchor = $rcols->{anchor_text};
    $anchor =~ s/[\t\r\n]/ /gs;
    push @fields, $anchor;
    my $title = $rcols->{title};
    $title =~ s/[\t\r\n]/ /gs;
    push @fields, $title;
    my $content = $rcols->{content};
    next if length($content) < 100;
    $content =~ s/[\t\r\n]/ /gs;
    push @fields, $content;

    print join "\t", @fields;
    print "\n";
}

# close the connection
if(!$rdb->close()){
    my $ecode = $rdb->ecode();
    printf STDERR ("close error: %s\n", $rdb->errmsg($ecode));
}





