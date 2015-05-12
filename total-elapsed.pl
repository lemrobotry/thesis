#! /usr/bin/perl -W

use strict;

my $total_elapsed = 0.0;

while (<>) {
    if (m|^<total-time>.*<elapsed>(.*)</elapsed>|) {
	$total_elapsed += $1;
    }
}

print $total_elapsed, "\n";
