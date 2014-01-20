#!/usr/bin/perl -n

if (/\/\*\*\s*(.*)/s) {
    $_ = $1;
    $print = 1;
}
if ($print) {
    if (/^(.*?)\*\//) {
	$_ = $1; 
	$print = 0;
    }
    print;
}
