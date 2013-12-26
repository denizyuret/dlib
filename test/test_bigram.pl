#!/usr/bin/perl -w
use strict;
my %cnt;
while(<>) {
    my $prev = "<s>";
    for my $next (split) {
	$cnt{$prev}{$next}++;
	$prev = $next;
    }
}

for my $a (keys %cnt) {
    for my $b (keys %{$cnt{$a}}) {
	print "$a\t$b\t$cnt{$a}{$b}\n";
    }
}
