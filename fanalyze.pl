#!/usr/local/bin/perl -w
#
# $Header$
#
# NAME
#   fanalyze.pl [-wrt] [-rt] [-calls] [-cg] [fperf.dat file]
#
# DESCRIPTION
#   this script analyzed the data contained in an fperf data file
#   and outputs some statistics.
#
# AUTHOR
#   jeff murphy
#

use strict;

print "fanalyze v1.0 (c) 1999 Nickel City Software\n\n";

my ($opt_wrt, $opt_rt, $opt_calls, $opt_cg) = (0,0,0,0);

my ($f);
while($_ = shift @ARGV) {
	if(/-wrt/i) {
		$opt_wrt = 1;
	} 
	elsif(/-rt/i) {
		$opt_rt = 1;
	}
	elsif(/-calls/i) {
		$opt_calls = 1;
	} 
	elsif(/-cg/i) {
		$opt_cg = 1;
	}
	else {
		$f = $_;
	}
}

die "usage: $0 [fperf data file]\n" unless defined($f);
open(FD, $f) || die "can't open file $f: $!";

# search for beginning of "RT func stats" section.

while(<FD>) {
  chomp;
  last if /^SECTION-RT:$/;
}

# read in "RT func stats" section

my @rts;

while(<FD>) {
	chomp;
	last if /^SECTION-(.*):$/;
	my (@a) = split(/\t/);
	my (%c);
	foreach (@a) {
		my (@b) = split(/=/);
		$c{$b[0]} = $b[1];
	}
	push @rts, \%c;
}

print "read ".@rts." data elements from RTS section.\n";

my (@cg);

while(<FD>) {
	chomp;
	last if /^SECTION-(.*):$/;
	my (@a) = split(/\t/);
	my (%c);
	foreach (@a) {
		my (@b) = split(/=/);
		if($#b == 0) {
			$c{'type'} = $_;
		} else {
			$c{$b[0]} = $b[1];
		}
	}
	push @cg, \%c;
}

close(FD);


print "read ".@cg." data elements from CG section.\n";

if($opt_wrt == 1) {
	print "\nfunctions reverse sorted by weighted run time:\n\n";
	
	foreach (sort 
		 {($b->{'avg'}*$b->{'calls'}) <=> ($a->{'avg'}*$a->{'calls'})} 
		 @rts) {
		print $_->{'avg'}." (".($_->{'calls'}*$_->{'avg'}).") ".$_->{'f'}."\n";
	}
}

if($opt_rt == 1) {
	print "\nfunctions reverse sorted by run time:\n\n";

	foreach (sort {$b->{'avg'} <=> $a->{'avg'}} @rts) {
		print $_->{'avg'}." (".$_->{'calls'}.") ".$_->{'f'}."\n";
	}
}

if($opt_calls == 1) {
	print "\nfunctions reverse sorted by number of calls:\n\n";
	
	foreach (sort {$b->{'calls'} <=> $a->{'calls'}} @rts) {
		print $_->{'calls'}." (".$_->{'avg'}.") ".$_->{'f'}."\n";
	}
}

if($opt_cg == 1) {
	print "\ncall graph:\n";
	foreach (@cg) {
		print $_->{'type'}."\n";
	}
}


exit 0;

