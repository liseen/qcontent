package QContent::Extractor::Test;

use warnings;
use strict;
#use Smart::Comments;
use IPC::Run3;
use Test::Base -Base;

use File::Spec;
use FindBin;
#use lib "$FindBin::Bin/../lib";

our @EXPORT = qw(run_tests run_test);

filters(qw/trim chomp/);

sub run_test ($) {
    my $block = shift;

    my $url = $block->url;

    my $vdom_file = $url;
    $vdom_file =~ s/^http:\/\///gs;
    $vdom_file =~ s/[\W]/_/gs;
    $vdom_file .= ".vdom";

    my $vdom_path = File::Spec->rel2abs($vdom_file, "$FindBin::Bin/../vdom/");
    my $cmd = File::Spec->rel2abs('extract', "$FindBin::Bin/../../") . " " . $vdom_path;
    ### $cmd

    my $out;
    run3($cmd, \undef, \$out, \undef);

    my $name = $block->name . ' :' . $block->seq_num();

    is($?, 0, "$name run ./extract tt/vdom/$vdom_file");
    my %rets;
    for my $ret (split /\n+--- /, $out) {
        my ($k, $v) = split /\n/, $ret, 2;
        next if !$k;
        if (!defined $v) {
            $v = '';
        }
        $rets{$k} = $v;
    }

    ### %rets
    #is($rets{raw_title}, $block->raw_title, $block->name . ' ' . 'raw_title');
    is($rets{list_confidence}, $block->list_confidence, $name . '  list_confidence' . " $url");
    is($rets{publish_time}, $block->publish_time, $name . ' publish_time' . " $url");
    is($rets{content_confidence}, $block->content_confidence, $name . ' content_confidence' . " $url");
    is($rets{title}, $block->title, $name . ' title' . " $url");
    is($rets{content}, $block->content, $name . ' content' . " $url");
    is($rets{images}, $block->images, $name . ' images' . " $url");
    #is($ret{}, $block->, $block->name . ' ' . '');
};

sub run_tests() {
    for my $block (Test::Base::blocks()) {
        run_test($block);
    }
}

1;
